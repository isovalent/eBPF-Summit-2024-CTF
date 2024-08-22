// +build ignore

#include "../../header/vmlinux.h"
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <string.h>

#pragma pack(1)

char __license[] SEC("license") = "GPL";

// TC Return codes
#define TC_ACT_OK 0
#define TC_ACT_SHOT 2

#define ETH_P_IP 0x0800 /* Internet Protocol packet */
#define ETH_HLEN 14     /*Ethernet Header Length */
#define ETH_ALEN 6

#define IP_SRC_OFF (ETH_HLEN + offsetof(struct iphdr, saddr))
#define IP_DST_OFF (ETH_HLEN + offsetof(struct iphdr, daddr))

static inline int swap_udp(struct __sk_buff *skb) {
  // Get pointers to the beginning and end
  void *data_end = (void *)(long)skb->data_end;
  void *data = (void *)(long)skb->data;

  // Point to ethernet header
  struct ethhdr *eth = data;
  __u16 h_proto;
  __u64 nh_off = 0;
  nh_off = sizeof(*eth);

  if (data + nh_off > data_end) {
    return TC_ACT_OK;
  }

  // Get protocol
  h_proto = eth->h_proto;

  // if it is IP continue
  if (h_proto == bpf_htons(ETH_P_IP)) {
    struct iphdr *iph = data + nh_off;

    // Get pointer to IP Header
    if ((void *)(iph + 1) > data_end) {
      return TC_ACT_OK;
    }

    // Check protocol inside IP packet
    if (iph->protocol != IPPROTO_UDP) {
      return TC_ACT_OK;
    }
    __u32 ip_hlen = 0;
    //__u32 poffset = 0;
    //__u32 plength = 0;
    // __u32 ip_total_length = bpf_ntohs(iph->tot_len);

    ip_hlen = iph->ihl << 2;

    if (ip_hlen < sizeof(*iph)) {
      return TC_ACT_OK;
    }
    struct udphdr *udph = data + nh_off + sizeof(*iph);

    if ((void *)(udph + 1) > data_end) {
      return TC_ACT_OK;
    }
    __u16 src_port = bpf_ntohs(udph->source);
    __u16 dst_port = bpf_ntohs(udph->dest);

    // Is the destination port the correct one?
    if (dst_port == 9000) {
      bpf_printk("UDP %d %d", src_port, dst_port);

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

      /* We'll store the mac addresses (L2) */
      __u8 src_mac[ETH_ALEN];
      __u8 dst_mac[ETH_ALEN];
      memcpy(src_mac, eth->h_source, ETH_ALEN);
      memcpy(dst_mac, eth->h_dest, ETH_ALEN);

      /* ip addresses (L3) */
      __be32 src_ip = iph->saddr;
      __be32 dst_ip = iph->daddr;

      /* and source/destination ports (L4) */
      __be16 dest_port = udph->dest;
      __be16 src_port = udph->source;

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

      data[0] = '?';

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
int tc_ingress(struct __sk_buff *skb) { return swap_udp(skb); }

SEC("tc_egress")
int tc_egress_(struct __sk_buff *skb) { return swap_udp(skb); }