#ifndef NET_NET_H
#define NET_NET_H

#include <stdint.h>
#include <stddef.h>
#include <kernel.h>

#define NET_MAX_PACKET 1536
#define NET_MAX_PAYLOAD 1500
#define NET_MAX_IFACES 8
#define NET_MAX_SOCKETS 32
#define NET_MAX_ARP_ENTRIES 16

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ETH_P_IP 0x0800
#define ETH_P_ARP 0x0806

#define IP_VERSION 4
#define IP_IHL 5
#define IP_TTL 64
#define IP_PROTO_ICMP 1
#define IP_PROTO_UDP 17

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

#define ARP_HRD_ETHER 1
#define ARP_PRO_IP 0x0800
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

#define IFF_UP 0x1
#define IFF_LOOPBACK 0x2
#define IFF_RUNNING 0x4

struct net_iface;

struct pbuf {
    uint8_t *data;
    size_t len;
    size_t cap;
    uint8_t *head;
    struct pbuf *next;
    struct net_iface *iface;
    uint8_t ref;
};

struct net_iface {
    char name[16];
    uint8_t mac[ETH_ALEN];
    uint32_t ip_addr;
    uint32_t netmask;
    uint32_t gateway;
    uint16_t mtu;
    uint8_t flags;
    int (*transmit)(struct net_iface *, struct pbuf *);
    void (*receive)(struct net_iface *, struct pbuf *);
    struct pbuf *rx_queue;
    struct pbuf *tx_queue;
};

struct arp_entry {
    uint32_t ip_addr;
    uint8_t mac[ETH_ALEN];
    uint32_t expire;
    uint8_t used;
};

struct socket {
    int domain;
    int type;
    int protocol;
    int fd;
    struct process *proc;
    uint16_t local_port;
    uint32_t local_addr;
    uint16_t remote_port;
    uint32_t remote_addr;
    uint8_t bound;
    uint8_t connected;
    struct pbuf *rx_queue;
    struct pbuf *tx_queue;
    struct socket *next;
    struct semaphore rx_wait;
    struct mutex lock;
    int ref_count;
};

struct ipv4_hdr {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
} __attribute__((packed));

struct icmp_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
} __attribute__((packed));

struct udp_hdr {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
} __attribute__((packed));

struct eth_hdr {
    uint8_t dest[ETH_ALEN];
    uint8_t src[ETH_ALEN];
    uint16_t proto;
} __attribute__((packed));

struct arp_hdr {
    uint16_t hrd;
    uint16_t pro;
    uint8_t hln;
    uint8_t pln;
    uint16_t op;
    uint8_t sha[ETH_ALEN];
    uint32_t spa;
    uint8_t tha[ETH_ALEN];
    uint32_t tpa;
} __attribute__((packed));

struct pbuf *pbuf_alloc(size_t size);
void pbuf_free(struct pbuf *p);
struct pbuf *pbuf_prepend(struct pbuf *p, size_t size);
struct pbuf *pbuf_append(struct pbuf *p, size_t size);
void pbuf_trim(struct pbuf *p, size_t new_len);

struct net_iface *net_iface_create(const char *name, uint32_t ip, uint32_t mask, uint16_t mtu);
void net_iface_up(struct net_iface *iface);
void net_iface_down(struct net_iface *iface);
struct net_iface *net_iface_get_by_name(const char *name);
struct net_iface *net_iface_get_by_ip(uint32_t ip);
void net_iface_input(struct net_iface *iface, struct pbuf *p);
int net_iface_output(struct net_iface *iface, struct pbuf *p);

void net_init(void);
void net_tick(void);

struct socket *sock_create(int domain, int type, int protocol);
int sock_bind(struct socket *s, uint32_t addr, uint16_t port);
int sock_connect(struct socket *s, uint32_t addr, uint16_t port);
int sock_sendto(struct socket *s, const void *buf, size_t len, uint32_t addr, uint16_t port);
int sock_recvfrom(struct socket *s, void *buf, size_t len, uint32_t *addr, uint16_t *port);
int sock_close(struct socket *s);

void ipv4_input(struct net_iface *iface, struct pbuf *p);
int ipv4_output(struct net_iface *iface, uint32_t daddr, uint8_t proto, struct pbuf *p);
uint16_t ipv4_checksum(const void *data, size_t len);

void icmp_input(struct net_iface *iface, struct pbuf *p);
int icmp_send_echo_reply(struct net_iface *iface, struct pbuf *p);

void arp_input(struct net_iface *iface, struct pbuf *p);
int arp_resolve(struct net_iface *iface, uint32_t ip, uint8_t *mac);

void udp_input(struct net_iface *iface, struct pbuf *p);
int udp_output(struct socket *s, struct pbuf *p);
uint16_t udp_checksum(struct pbuf *p, uint32_t saddr, uint32_t daddr);

struct socket *sock_find_udp(uint32_t laddr, uint16_t lport);
int sock_alloc_fd(struct socket *s);
void sock_free_fd(struct socket *s);

void netstat_print(void);

#endif