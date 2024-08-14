// +build ignore

#include "../../header/vmlinux.h"
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <string.h>

#pragma pack(1)

char __license[] SEC("license") = "GPL";

#define TC_ACT_OK 0
#define TC_ACT_SHOT 2
#define ETH_P_IP 0x0800 /* Internet Protocol packet */
#define ETH_HLEN 14     /*Ethernet Header Length */
#define ETH_ALEN 6

#define IP_SRC_OFF (ETH_HLEN + offsetof(struct iphdr, saddr))
#define IP_DST_OFF (ETH_HLEN + offsetof(struct iphdr, daddr))

static inline int read_dns(struct __sk_buff *skb) {
  void *data_end = (void *)(long)skb->data_end;
  void *data = (void *)(long)skb->data;
  struct ethhdr *eth = data;
  __u16 h_proto;
  __u64 nh_off = 0;
  nh_off = sizeof(*eth);

  if (data + nh_off > data_end) {
    return TC_ACT_OK;
  }

  h_proto = eth->h_proto;

  if (h_proto == bpf_htons(ETH_P_IP)) {
    struct iphdr *iph = data + nh_off;

    if ((void *)(iph + 1) > data_end) {
      return 0;
    }

    if (iph->protocol != IPPROTO_UDP) {
      return 0;
    }
    __u32 ip_hlen = 0;
    //__u32 poffset = 0;
    //__u32 plength = 0;
    // __u32 ip_total_length = bpf_ntohs(iph->tot_len);

    ip_hlen = iph->ihl << 2;

    if (ip_hlen < sizeof(*iph)) {
      return 0;
    }
    struct udphdr *udph = data + nh_off + sizeof(*iph);

    if ((void *)(udph + 1) > data_end) {
      return 0;
    }
    __u16 src_port = bpf_ntohs(udph->source);
    __u16 dst_port = bpf_ntohs(udph->dest);
    bpf_printk("UDP %d %d", src_port, dst_port);

    if (dst_port == 9000) {

      u32 payload_offset = sizeof(*eth) + sizeof(*iph) + sizeof(*udph);
      u32 payload_length = iph->tot_len - (sizeof(*iph) + sizeof(*udph));

      if (payload_length < 3) {
        return TC_ACT_OK;
      }

      uint8_t data[4];
      u32 ret = bpf_skb_load_bytes(skb, payload_offset, data, sizeof(data) - 1);
      if (ret) {
        bpf_printk("error");
        return TC_ACT_OK;
      }
      data[3] = '\0'; // null terminate the string
      bpf_printk("data: %s", data);

      // uint8_t data2[1];
      //  data2[0] = 'A';
      //   if (sizeof(data) != 0) {
      // u32 data_offset = sizeof(*eth) + sizeof(*iph) + sizeof(*udph);

      // ret = bpf_skb_store_bytes(skb, data_offset, &data, sizeof(data),
      //                           BPF_F_RECOMPUTE_CSUM);
      // if (ret) {
      //   bpf_printk("error");
      //   return TC_ACT_OK;
      // }
      // }
      /* We'll store the mac addresses (L2) */
      __u8 src_mac[ETH_ALEN];
      __u8 dst_mac[ETH_ALEN];
      memcpy(src_mac, eth->h_source, ETH_ALEN);
      memcpy(dst_mac, eth->h_dest, ETH_ALEN);

      /* ip addresses (L3) */
      __be32 src_ip = iph->saddr;
      __be32 dst_ip = iph->daddr;

      // bpf_printk("FROM: %d...%d", src_ip & 0xff, src_ip >> 24);

      //   // ignore private ip ranges 10.0.0.0/8, 127.0.0.0/8, 172.16.0.0/12,
      //   // 192.168.0.0/16
      //   if ((src_ip & 0xff) == 10 || ((src_ip & 0xff) == 127) ||
      //       (src_ip & 0xf0ff) == 0x10ac || (src_ip & 0xffff) == 0xa8c0 ||
      //       (dst_ip & 0xff) == 10 || ((dst_ip & 0xff) == 127) ||
      //       (dst_ip & 0xf0ff) == 0x10ac || (dst_ip & 0xffff) == 0xa8c0)
      //     return TC_ACT_UNSPEC;

      /* and source/destination ports (L4) */
      __be16 dest_port = udph->dest;
      __be16 src_port = udph->source;
      // dest_port = bpf_htons(9001);
      /* and then swap them all */

      /* Swap the mac addresses */
      bpf_skb_store_bytes(skb, offsetof(struct ethhdr, h_source), dst_mac,
                          ETH_ALEN, 0);
      bpf_skb_store_bytes(skb, offsetof(struct ethhdr, h_dest), src_mac,
                          ETH_ALEN, 0);

      /* Swap the ip addresses
       * swapping the ips does not require checksum recalculation,
       * but we might want to reduce the TTL to prevent packets infinitely
       * looping between us and another device that does not reduce the TTL */
      bpf_skb_store_bytes(skb,
                          sizeof(struct ethhdr) + offsetof(struct iphdr, saddr),
                          &dst_ip, sizeof(dst_ip), 0);
      bpf_skb_store_bytes(skb,
                          sizeof(struct ethhdr) + offsetof(struct iphdr, daddr),
                          &src_ip, sizeof(src_ip), 0);

      /* Swap the source and destination ports in the udp packet */
      bpf_skb_store_bytes(skb,
                          sizeof(struct ethhdr) + sizeof(struct iphdr) +
                              offsetof(struct udphdr, source),
                          &dest_port, sizeof(dest_port), 0);
      bpf_skb_store_bytes(skb,
                          sizeof(struct ethhdr) + sizeof(struct iphdr) +
                              offsetof(struct udphdr, dest),
                          &src_port, sizeof(src_port), 0);

      u32 data_offset = sizeof(*eth) + sizeof(*iph) + sizeof(*udph);

      data[0] = 'D';
      ret = bpf_skb_store_bytes(skb, data_offset, &data, sizeof(data),
                                BPF_F_RECOMPUTE_CSUM);

      /* And then send it back from wherever it's come from */
      ret = bpf_clone_redirect(skb, skb->ifindex, 0);
      if (ret) {
        bpf_printk("bpf_clone_redirect error: %d", ret);
      }
      /* Since we've handled the packet, drop it */
      return TC_ACT_SHOT;
    }

    return TC_ACT_OK;
  }
  return TC_ACT_OK;
}

// eBPF hooks - This is where the magic happens!
SEC("tc_in")
int tc_ingress(struct __sk_buff *skb) { return read_dns(skb); }

SEC("tc_egress")
int tc_egress_(struct __sk_buff *skb) { return read_dns(skb); }