// +build ignore

#include "../../header/vmlinux.h"
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <string.h>

#pragma pack(1)
// #include "dns.h"

char __license[] SEC("license") = "GPL";

#define TC_ACT_OK 0
#define ETH_P_IP 0x0800 /* Internet Protocol packet */
#define ETH_HLEN 14     /*Ethernet Header Length */
#define ETH_ALEN 6

#define IP_SRC_OFF (ETH_HLEN + offsetof(struct iphdr, saddr))
#define IP_DST_OFF (ETH_HLEN + offsetof(struct iphdr, daddr))

// static int parse_query(struct __sk_buff *skb, void *query_start,
//                        struct dns_query *q);
// static inline int bpf_strcmplength(char *s1, char *s2, u32 n);

// struct dns_replace {
//   __u8 name[MAX_DNS_NAME_LENGTH]; // This should be a char but code
//   generation
//                                   // between here and Go..
//   __u32 arecord; // This should be a char but code generation between here
//   and
//                  // Go..
// };

// struct {
//   __uint(type, BPF_MAP_TYPE_HASH);
//   __uint(max_entries, 1024);
//   __type(key, char[MAX_DNS_NAME_LENGTH]);
//   __type(value, struct dns_replace);
// } dns_map SEC(".maps");

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

    // if (src_port == 9000 || dst_port == 9000) {
    if (dst_port == 9000) {
      bpf_printk("test");

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

      bpf_printk("FROM: %d...%d", src_ip & 0xff, src_ip >> 24);

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
      dest_port = bpf_htons(9001);
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

      /* And then send it back from wherever it's come from */
      ret = bpf_clone_redirect(skb, skb->ifindex, 0);
      if (ret) {
        bpf_printk("bpf_clone_redirect error: %d", ret);
      }
      /* Since we've handled the packet, drop it */
      return 0;

      /* We modified the packet and redirected it, it can be dropped here */
      // return 2;

      //   // Get the DNS Header
      //   struct dns_hdr *dns_hdr =
      //       data + sizeof(*eth) + sizeof(*iph) + sizeof(*udph);
      //   if ((void *)(dns_hdr + 1) > data_end) {
      //     return 0;
      //   }
      //   // qr == 0 is a query
      //   if (dns_hdr->qr == 0 && dns_hdr->opcode == 0) {
      //     bpf_printk("DNS query transaction id %u",
      //                bpf_ntohs(dns_hdr->transaction_id));
      //   }

      //   // qr == 1 is a response
      //   if (dns_hdr->qr == 1 && dns_hdr->opcode == 0) {
      //     // Read the query
      //     void *query_start = (void *)dns_hdr + sizeof(struct dns_hdr);

      //     struct dns_query q;
      //     int query_length = 0;
      //     query_length = parse_query(skb, query_start, &q);
      //     if (query_length < 1) {
      //       return 0;
      //     }

      //     struct dns_replace *found_name;
      //     // Looking up the domain name in the map
      //     if (sizeof(q.name) != 0) {
      //       found_name = bpf_map_lookup_elem(&dns_map, q.name);
      //       if (found_name > 0) {
      //         bpf_printk("Looks like we've found your name [%s]",
      //                    found_name->name);
      //       }
      //     }
      //     // Read the DNS response
      //     struct dns_response *ar_hdr = data + sizeof(*eth) + sizeof(*iph) +
      //                                   sizeof(*udph) + sizeof(*dns_hdr) +
      //                                   query_length;
      //     if ((void *)(ar_hdr + 1) > data_end) {
      //       return 0;
      //     }

      //     __u32 ip;

      //     __u32 poffset = sizeof(*eth) + sizeof(*iph) + sizeof(*udph) +
      //                     sizeof(*dns_hdr) + query_length + sizeof(*ar_hdr);

      // Load data from the socket buffer, poffset starts at the end of the
      // TCP Header
      //   int ret = bpf_skb_load_bytes(skb, poffset, &ip, sizeof(ip));
      //   if (ret != 0) {
      //     return 0;
      //   }
      //   // bpf_printk("%pI4", &ip);
      //   if (found_name) {
      //     bpf_printk("%pI4 -> %pI4", &ip, &found_name->arecord);
      //     ret = bpf_skb_store_bytes(skb, poffset, &found_name->arecord,
      //                               sizeof(found_name->arecord),
      //                               BPF_F_RECOMPUTE_CSUM);
      //     if (ret != 0) {
      //       return 0;
      //     }
      //   }
    }

    // //Get a pointer to the start of the DNS query
    // void *query_start = (void *)dns_hdr + sizeof(struct dns_hdr);

    // struct dns_query q;
    // int query_length = 0;
    // query_length = parse_query(skb, query_start, &q);
    // if (query_length < 1)
    //     {
    //         return 0;
    //     }
    // //bpf_printk("%u %s %u", query_length, q.name, sizeof(q.name));
    // if (bpf_strcmplength(q.name, "github.com", query_length) == 0) {
    //     bpf_printk("woo");
    // }
  }

  return 0;
}

// // Parse query and return query length
// static int parse_query(struct __sk_buff *skb, void *query_start,
//                        struct dns_query *q) {
//   void *data_end = (void *)(long)skb->data_end;

// #ifdef DEBUG
//   bpf_printk("Parsing query");
// #endif

//   uint16_t i;
//   void *cursor = query_start;
//   int namepos = 0;

//   // Fill dns_query.name with zero bytes
//   // Not doing so will make the verifier complain when dns_query is used as a
//   // key in bpf_map_lookup
//   memset(&q->name[0], 0, sizeof(q->name));
//   // Fill record_type and class with default values to satisfy verifier
//   q->record_type = 0;
//   q->class = 0;

//   // We create a bounded loop of MAX_DNS_NAME_LENGTH (maximum allowed dns
//   name
//   // size). We'll loop through the packet byte by byte until we reach '0' in
//   // order to get the dns query name
//   for (i = 0; i < MAX_DNS_NAME_LENGTH; i++) {

//     // Boundary check of cursor. Verifier requires a +1 here.
//     // Probably because we are advancing the pointer at the end of the loop
//     if (cursor + 1 > data_end) {
// #ifdef DEBUG
//       bpf_printk("Error: boundary exceeded while parsing DNS query name");
// #endif
//       break;
//     }

//     /*
//     #ifdef DEBUG
//     bpf_printk("Cursor contents is %u\n", *(char *)cursor);
//     #endif
//     */

//     // If separator is zero we've reached the end of the domain query
//     if (*(char *)(cursor) == 0) {

//       // We've reached the end of the query name.
//       // This will be followed by 2x 2 bytes: the dns type and dns class.
//       if (cursor + 5 > data_end) {
// #ifdef DEBUG
//         bpf_printk("Error: boundary exceeded while retrieving DNS record type
//         "
//                    "and class");
// #endif
//       } else {
//         q->record_type = bpf_htons(*(uint16_t *)(cursor + 1));
//         q->class = bpf_htons(*(uint16_t *)(cursor + 3));
//       }

//       // Return the bytecount of (namepos + current '0' byte + dns type + dns
//       // class) as the query length.
//       return namepos + 1 + 2 + 2;
//     }

//     // Read and fill data into struct
//     q->name[namepos] = *(char *)(cursor);

//     namepos++;
//     cursor++;
//   }

//   return -1;
// }

// static inline int bpf_strcmplength(char *s1, char *s2, u32 n)
// {
//     for (int i = 0; i < n && i < sizeof(s1) && i < sizeof(s2); i++)
//     {
//         if (s1[i] != s2[i])
//             return s1[i] - s2[i];

//         if (s1[i] == s2[i] == '\0')
//             return 0;
//     }

//     return 0;
// }

// eBPF hooks - This is where the magic happens!
SEC("tc_in")
int tc_ingress(struct __sk_buff *skb) { return read_dns(skb); }

SEC("tc_egress")
int tc_egress_(struct __sk_buff *skb) { return read_dns(skb); }