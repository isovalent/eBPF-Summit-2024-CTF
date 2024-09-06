// +build ignore

#include "../../header/vmlinux.h"
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

#pragma pack(1)
// TC Return codes
#define TC_ACT_OK 0
#define TC_ACT_SHOT 2

#define ETH_P_IP 0x0800 /* Internet Protocol packet */
#define ETH_HLEN 14     /*Ethernet Header Length */
#define ETH_ALEN 6

char __license[] SEC("license") = "GPL";

static inline int redirect_tcp(struct __sk_buff *skb, bool ingress) {
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
    if (iph->protocol != IPPROTO_TCP) {
      return TC_ACT_OK;
    }
    __u32 ip_hlen = 0;

    ip_hlen = iph->ihl << 2;

    if (ip_hlen < sizeof(*iph)) {
      return TC_ACT_OK;
    }
    struct iphdr ip;
    struct tcphdr tcp;

    if (0 != bpf_skb_load_bytes(skb, sizeof(struct ethhdr), &ip,
                                sizeof(struct iphdr))) {
      bpf_printk("bpf_skb_load_bytes iph failed");
      return TC_ACT_OK;
    }

    if (0 != bpf_skb_load_bytes(skb, sizeof(struct ethhdr) + (ip.ihl << 2),
                                &tcp, sizeof(struct tcphdr))) {
      bpf_printk("bpf_skb_load_bytes eth failed");
      return TC_ACT_OK;
    }

    // something doesn't seem right here
    __u16 source = tcp.source;
    __u16 dest = tcp.dest;

    // Print out any traffic we might be interested in
    if (source > 1024 && dest > 1024) {
      bpf_printk("ingress:%s source: %d -> destination %d",
                 ingress ? "true" : "false", source, dest);
    }

    if (ingress) {
      if (dest == 2001) {
        tcp.dest = 2000;
        long ret =
            bpf_skb_store_bytes(skb, sizeof(struct ethhdr) + (ip.ihl << 2),
                                &tcp, sizeof(tcp), BPF_F_RECOMPUTE_CSUM);
        if (ret != 0) {
          bpf_printk("Error writing bytes");
        }
      }
    } else {
      // something is missing here
    }

    // The Rebel engineer left some TCP dump output, might help, might be a read
    // herring
    /*
    15:27:27.195146 IP (tos 0x0, ttl 64, id 64035, offset 0, flags [DF], proto
    TCP (6), length 60) 127.0.0.1.55590 > 127.0.0.1.2001: Flags [S], cksum
    0xfe30 (incorrect -> 0xde8f), seq 1127217463, win 65495, options [mss
    65495,sackOK,TS val 1342243858 ecr 0,nop,wscale 7], length 0 15:27:27.195160
    IP (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 40)
    127.0.0.1.2001 > 127.0.0.1.55590: Flags [R.], cksum 0x946e (correct), seq 0,
    ack 1127217464, win 0, length 0

       15:27:19.192319 IP (tos 0x0, ttl 64, id 57861, offset 0, flags [DF],
    proto TCP (6), length 60) 127.0.0.1.50720 > 127.0.0.1.2001: Flags [S], cksum
    0xfe30 (incorrect -> 0xb3db), seq 1553087697, win 65495, options [mss
    65495,sackOK,TS val 1342235856 ecr 0,nop,wscale 7], length 0 15:27:19.192345
    IP (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 60)
    127.0.0.1.2000 > 127.0.0.1.50720: Flags [S.], cksum 0xfe30 (incorrect ->
    0x7bf5), seq 2242477415, ack 1553087698, win 65483, options [mss
    65495,sackOK,TS val 1342235856 ecr 1342235856,nop,wscale 7], length 0
    15:27:19.192359 IP (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP
    (6), length 40) 127.0.0.1.50720 > 127.0.0.1.2001: Flags [R], cksum 0x4a89
    (incorrect -> 0x4a88), seq 1553087698, win 0, length 0
        */
    return TC_ACT_OK;
  }
  return TC_ACT_OK;
}
// eBPF hooks - This is where the magic happens!
SEC("tc_in")
int tc_ingress(struct __sk_buff *skb) { return redirect_tcp(skb, true); }

SEC("tc_egress")
int tc_egress_(struct __sk_buff *skb) { return redirect_tcp(skb, false); }
