#include <net.h>
#include <memory.h>
#include <string.h>
#include <vfs.h>

static struct net_iface *ifaces[NET_MAX_IFACES];
static int num_ifaces = 0;

static struct arp_entry arp_table[NET_MAX_ARP_ENTRIES];
static uint32_t arp_tick = 0;

static struct socket *sockets = NULL;
static int next_port = 1024;

static uint16_t ip_id = 1;

struct pbuf *pbuf_alloc(size_t size) {
    struct pbuf *p = kzalloc(sizeof(struct pbuf));
    if (!p) return NULL;

    size_t total = size + sizeof(struct eth_hdr) + 4;
    p->head = kmalloc(total);
    if (!p->head) {
        kfree(p);
        return NULL;
    }

    p->data = p->head + sizeof(struct eth_hdr) + 4;
    p->len = 0;
    p->cap = size;
    p->ref = 1;
    return p;
}

void pbuf_free(struct pbuf *p) {
    if (!p) return;
    if (p->ref > 0) {
        p->ref--;
        if (p->ref == 0) {
            if (p->head) kfree(p->head);
            kfree(p);
        }
    }
}

struct pbuf *pbuf_prepend(struct pbuf *p, size_t size) {
    if (!p) return NULL;
    ptrdiff_t offset = p->data - p->head;
    if (offset < (ptrdiff_t)size) return NULL;
    p->data -= size;
    p->len += size;
    return p;
}

struct pbuf *pbuf_append(struct pbuf *p, size_t size) {
    if (!p || p->data + p->len + size > p->head + p->cap + sizeof(struct eth_hdr) + 4) return NULL;
    p->len += size;
    return p;
}

void pbuf_trim(struct pbuf *p, size_t new_len) {
    if (!p || new_len > p->len) return;
    p->len = new_len;
}

static int loopback_xmit(struct net_iface *iface, struct pbuf *p);

void net_init(void) {
    struct net_iface *lo = net_iface_create("lo", 0x7F000001, 0xFF000000, 1536);
    if (lo) {
        lo->flags |= IFF_LOOPBACK | IFF_UP | IFF_RUNNING;
        lo->mac[0] = 0;
        lo->mac[1] = 0;
        lo->mac[2] = 0;
        lo->mac[3] = 0;
        lo->mac[4] = 0;
        lo->mac[5] = 0;
        lo->transmit = loopback_xmit;
        net_iface_up(lo);
    }

    for (int i = 0; i < NET_MAX_ARP_ENTRIES; i++) {
        arp_table[i].used = 0;
    }
}

struct net_iface *net_iface_create(const char *name, uint32_t ip, uint32_t mask, uint16_t mtu) {
    if (num_ifaces >= NET_MAX_IFACES) return NULL;

    struct net_iface *iface = kzalloc(sizeof(struct net_iface));
    if (!iface) return NULL;

    strncpy(iface->name, name, 15);
    iface->name[15] = 0;
    iface->ip_addr = ip;
    iface->netmask = mask;
    iface->mtu = mtu;
    iface->flags = 0;

    ifaces[num_ifaces++] = iface;
    return iface;
}

void net_iface_up(struct net_iface *iface) {
    if (!iface) return;
    iface->flags |= IFF_UP | IFF_RUNNING;
}

void net_iface_down(struct net_iface *iface) {
    if (!iface) return;
    iface->flags &= ~(IFF_UP | IFF_RUNNING);
}

struct net_iface *net_iface_get_by_name(const char *name) {
    for (int i = 0; i < num_ifaces; i++) {
        if (strcmp(ifaces[i]->name, name) == 0) return ifaces[i];
    }
    return NULL;
}

struct net_iface *net_iface_get_by_ip(uint32_t ip) {
    for (int i = 0; i < num_ifaces; i++) {
        if (ifaces[i]->ip_addr == ip) return ifaces[i];
    }
    return NULL;
}

int loopback_xmit(struct net_iface *iface, struct pbuf *p) {
    if (!iface || !p) return -1;
    p->iface = iface;
    net_iface_input(iface, p);
    return 0;
}

void net_iface_input(struct net_iface *iface, struct pbuf *p) {
    if (!iface || !p) return;

    struct eth_hdr *eth = (struct eth_hdr *)p->data;
    uint16_t proto = __builtin_bswap16(eth->proto);

    switch (proto) {
        case ETH_P_IP:
            ipv4_input(iface, p);
            break;
        case ETH_P_ARP:
            arp_input(iface, p);
            break;
        default:
            pbuf_free(p);
            break;
    }
}

int net_iface_output(struct net_iface *iface, struct pbuf *p) {
    if (!iface || !p || !iface->transmit) return -1;
    if (!(iface->flags & IFF_UP)) return -1;
    return iface->transmit(iface, p);
}

void net_tick(void) {
    arp_tick++;
    if (arp_tick >= 1000) {
        for (int i = 0; i < NET_MAX_ARP_ENTRIES; i++) {
            if (arp_table[i].used && arp_table[i].expire != 0 && arp_tick > arp_table[i].expire) {
                arp_table[i].used = 0;
            }
        }
        arp_tick = 0;
    }
}

uint16_t checksum16(const uint16_t *data, size_t len) {
    uint32_t sum = 0;
    while (len > 1) {
        sum += *data++;
        len -= 2;
    }
    if (len) sum += *(uint8_t *)data;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

uint16_t ipv4_checksum(const void *data, size_t len) {
    return checksum16(data, len);
}

void ipv4_input(struct net_iface *iface, struct pbuf *p) {
    if (!p || p->len < sizeof(struct ipv4_hdr)) {
        pbuf_free(p);
        return;
    }

    struct ipv4_hdr *iph = (struct ipv4_hdr *)p->data;
    uint8_t ihl = (iph->version_ihl & 0x0F) * 4;
    uint16_t tot_len = __builtin_bswap16(iph->tot_len);

    if (iph->version_ihl >> 4 != IP_VERSION) {
        pbuf_free(p);
        return;
    }
    if (ihl < sizeof(struct ipv4_hdr)) {
        pbuf_free(p);
        return;
    }
    if (tot_len > p->len) {
        pbuf_free(p);
        return;
    }

    uint16_t check = iph->check;
    iph->check = 0;
    if (check != ipv4_checksum(iph, ihl)) {
        pbuf_free(p);
        return;
    }

    uint32_t daddr = __builtin_bswap32(iph->daddr);
    if (daddr != iface->ip_addr && daddr != 0xFFFFFFFF) {
        pbuf_free(p);
        return;
    }

    p->data += ihl;
    p->len -= ihl;

    switch (iph->protocol) {
        case IP_PROTO_ICMP:
            icmp_input(iface, p);
            break;
        case IP_PROTO_UDP:
            udp_input(iface, p);
            break;
        default:
            pbuf_free(p);
            break;
    }
}

int ipv4_output(struct net_iface *iface, uint32_t daddr, uint8_t proto, struct pbuf *p) {
    if (!p || !iface) return -1;

    size_t ihl = sizeof(struct ipv4_hdr);
    if (!pbuf_prepend(p, ihl)) return -1;

    struct ipv4_hdr *iph = (struct ipv4_hdr *)p->data;
    iph->version_ihl = (IP_VERSION << 4) | (IP_IHL & 0x0F);
    iph->tos = 0;
    iph->tot_len = __builtin_bswap16(p->len);
    iph->id = __builtin_bswap16(ip_id++);
    iph->frag_off = __builtin_bswap16(0x4000);
    iph->ttl = IP_TTL;
    iph->protocol = proto;
    iph->check = 0;
    iph->saddr = __builtin_bswap32(iface->ip_addr);
    iph->daddr = __builtin_bswap32(daddr);
    iph->check = ipv4_checksum(iph, ihl);

    if (iface->flags & IFF_LOOPBACK) {
        return loopback_xmit(iface, p);
    }

    return -1;
}

void icmp_input(struct net_iface *iface, struct pbuf *p) {
    if (!p || p->len < sizeof(struct icmp_hdr)) {
        pbuf_free(p);
        return;
    }

    struct icmp_hdr *icmp = (struct icmp_hdr *)p->data;
    uint16_t check = icmp->checksum;
    icmp->checksum = 0;
    if (check != ipv4_checksum(p->data, p->len)) {
        pbuf_free(p);
        return;
    }

    if (icmp->type == ICMP_ECHO_REQUEST) {
        icmp->type = ICMP_ECHO_REPLY;
        icmp->checksum = 0;
        icmp->checksum = ipv4_checksum(p->data, p->len);

        uint32_t saddr = iface->ip_addr;
        struct ipv4_hdr *iph = (struct ipv4_hdr *)(p->data - sizeof(struct ipv4_hdr));
        iph->daddr = iph->saddr;
        iph->saddr = __builtin_bswap32(saddr);
        iph->check = 0;
        iph->check = ipv4_checksum(iph, sizeof(struct ipv4_hdr));

        loopback_xmit(iface, p);
    } else {
        pbuf_free(p);
    }
}

int icmp_send_echo_reply(struct net_iface *iface, struct pbuf *p) {
    (void)iface; (void)p;
    return 0;
}

void arp_input(struct net_iface *iface, struct pbuf *p) {
    if (!p || p->len < sizeof(struct arp_hdr)) {
        pbuf_free(p);
        return;
    }

    struct arp_hdr *arp = (struct arp_hdr *)p->data;
    uint16_t op = __builtin_bswap16(arp->op);
    uint32_t spa = __builtin_bswap32(arp->spa);
    uint32_t tpa = __builtin_bswap32(arp->tpa);

    if (op == ARP_OP_REQUEST && tpa == iface->ip_addr) {
        struct pbuf *reply = pbuf_alloc(sizeof(struct arp_hdr) + sizeof(struct eth_hdr));
        if (!reply) {
            pbuf_free(p);
            return;
        }

        struct eth_hdr *eth = (struct eth_hdr *)reply->data;
        memcpy(eth->dest, arp->sha, ETH_ALEN);
        memcpy(eth->src, iface->mac, ETH_ALEN);
        eth->proto = __builtin_bswap16(ETH_P_ARP);

        struct arp_hdr *rarp = (struct arp_hdr *)(reply->data + sizeof(struct eth_hdr));
        rarp->hrd = __builtin_bswap16(ARP_HRD_ETHER);
        rarp->pro = __builtin_bswap16(ARP_PRO_IP);
        rarp->hln = ETH_ALEN;
        rarp->pln = 4;
        rarp->op = __builtin_bswap16(ARP_OP_REPLY);
        memcpy(rarp->sha, iface->mac, ETH_ALEN);
        rarp->spa = __builtin_bswap32(iface->ip_addr);
        memcpy(rarp->tha, arp->sha, ETH_ALEN);
        rarp->tpa = __builtin_bswap32(spa);

        reply->len = sizeof(struct eth_hdr) + sizeof(struct arp_hdr);
        loopback_xmit(iface, reply);
    }

    if ((op == ARP_OP_REQUEST || op == ARP_OP_REPLY) && spa != 0) {
        for (int i = 0; i < NET_MAX_ARP_ENTRIES; i++) {
            if (!arp_table[i].used || arp_table[i].ip_addr == spa) {
                arp_table[i].ip_addr = spa;
                memcpy(arp_table[i].mac, arp->sha, ETH_ALEN);
                arp_table[i].expire = arp_tick + 300;
                arp_table[i].used = 1;
                break;
            }
        }
    }

    pbuf_free(p);
}

int arp_resolve(struct net_iface *iface, uint32_t ip, uint8_t *mac) {
    (void)iface;
    for (int i = 0; i < NET_MAX_ARP_ENTRIES; i++) {
        if (arp_table[i].used && arp_table[i].ip_addr == ip) {
            memcpy(mac, arp_table[i].mac, ETH_ALEN);
            return 0;
        }
    }
    return -1;
}

void udp_input(struct net_iface *iface, struct pbuf *p) {
    if (!p || p->len < sizeof(struct udp_hdr)) {
        pbuf_free(p);
        return;
    }

    struct udp_hdr *udph = (struct udp_hdr *)p->data;
    uint16_t len = __builtin_bswap16(udph->len);
    (void)__builtin_bswap16(udph->source);
    uint16_t dport = __builtin_bswap16(udph->dest);

    if (len > p->len) {
        pbuf_free(p);
        return;
    }

    p->data += sizeof(struct udp_hdr);
    p->len -= sizeof(struct udp_hdr);

    struct socket *s = sock_find_udp(iface->ip_addr, dport);
    if (!s) {
        pbuf_free(p);
        return;
    }

    mutex_lock(&s->lock);
    p->next = s->rx_queue;
    s->rx_queue = p;
    semaphore_signal(&s->rx_wait);
    mutex_unlock(&s->lock);
}

int udp_output(struct socket *s, struct pbuf *p) {
    if (!s || !p) return -1;

    struct net_iface *iface = net_iface_get_by_ip(s->local_addr);
    if (!iface) return -1;

    if (!pbuf_prepend(p, sizeof(struct udp_hdr))) return -1;

    struct udp_hdr *udph = (struct udp_hdr *)p->data;
    udph->source = __builtin_bswap16(s->local_port);
    udph->dest = __builtin_bswap16(s->remote_port);
    udph->len = __builtin_bswap16(p->len);
    udph->check = 0;
    udph->check = udp_checksum(p, s->local_addr, s->remote_addr);

    return ipv4_output(iface, s->remote_addr, IP_PROTO_UDP, p);
}

uint16_t udp_checksum(struct pbuf *p, uint32_t saddr, uint32_t daddr) {
    uint32_t sum = 0;
    uint16_t *data = (uint16_t *)p->data;
    size_t len = p->len;

    while (len > 1) {
        sum += *data++;
        len -= 2;
    }
    if (len) sum += *(uint8_t *)data;

    sum += (saddr >> 16) & 0xFFFF;
    sum += saddr & 0xFFFF;
    sum += (daddr >> 16) & 0xFFFF;
    sum += daddr & 0xFFFF;
    sum += __builtin_bswap16(IP_PROTO_UDP);
    sum += __builtin_bswap16(p->len);

    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    uint16_t check = ~sum;
    return check == 0 ? 0xFFFF : check;
}

struct socket *sock_create(int domain, int type, int protocol) {
    if (domain != AF_INET || type != SOCK_DGRAM) return NULL;

    struct socket *s = kzalloc(sizeof(struct socket));
    if (!s) return NULL;

    s->domain = domain;
    s->type = type;
    s->protocol = protocol;
    s->local_addr = 0;
    s->local_port = 0;
    s->remote_addr = 0;
    s->remote_port = 0;
    s->bound = 0;
    s->connected = 0;
    s->ref_count = 1;
    mutex_init(&s->lock);
    semaphore_init(&s->rx_wait, 0);

    mutex_lock(&s->lock);
    s->next = sockets;
    sockets = s;
    mutex_unlock(&s->lock);

    return s;
}

int sock_bind(struct socket *s, uint32_t addr, uint16_t port) {
    if (!s) return -1;

    mutex_lock(&s->lock);
    struct socket *cur = sockets;
    while (cur) {
        if (cur != s && cur->bound && cur->local_port == port && cur->local_addr == addr) {
            mutex_unlock(&s->lock);
            return -1;
        }
        cur = cur->next;
    }

    if (port == 0) {
        port = next_port++;
        if (next_port > 65535) next_port = 1024;
    }

    s->local_addr = addr;
    s->local_port = port;
    s->bound = 1;
    mutex_unlock(&s->lock);
    return 0;
}

int sock_connect(struct socket *s, uint32_t addr, uint16_t port) {
    if (!s || !s->bound) return -1;
    s->remote_addr = addr;
    s->remote_port = port;
    s->connected = 1;
    return 0;
}

int sock_sendto(struct socket *s, const void *buf, size_t len, uint32_t addr, uint16_t port) {
    if (!s || len > NET_MAX_PAYLOAD) return -1;

    struct pbuf *p = pbuf_alloc(len);
    if (!p) return -1;

    memcpy(p->data, buf, len);
    p->len = len;

    uint32_t raddr = s->connected ? s->remote_addr : addr;
    uint16_t rport = s->connected ? s->remote_port : port;

    if (!s->connected) {
        s->remote_addr = raddr;
        s->remote_port = rport;
    }

    return udp_output(s, p);
}

int sock_recvfrom(struct socket *s, void *buf, size_t len, uint32_t *addr, uint16_t *port) {
    if (!s) return -1;

    mutex_lock(&s->lock);
    while (!s->rx_queue) {
        semaphore_wait(&s->rx_wait);
    }

    struct pbuf *p = s->rx_queue;
    s->rx_queue = p->next;

    size_t copy = p->len < len ? p->len : len;
    memcpy(buf, p->data, copy);

    if (addr) *addr = 0;
    if (port) *port = 0;

    pbuf_free(p);
    mutex_unlock(&s->lock);
    return copy;
}

int sock_close(struct socket *s) {
    if (!s) return -1;

    mutex_lock(&s->lock);
    s->ref_count--;
    if (s->ref_count > 0) {
        mutex_unlock(&s->lock);
        return 0;
    }

    if (sockets == s) {
        sockets = s->next;
    } else {
        struct socket *cur = sockets;
        while (cur && cur->next != s) cur = cur->next;
        if (cur) cur->next = s->next;
    }

    while (s->rx_queue) {
        struct pbuf *p = s->rx_queue;
        s->rx_queue = p->next;
        pbuf_free(p);
    }

    mutex_unlock(&s->lock);
    kfree(s);
    return 0;
}

struct socket *sock_find_udp(uint32_t laddr, uint16_t lport) {
    for (struct socket *s = sockets; s; s = s->next) {
        if (s->bound && s->local_addr == laddr && s->local_port == lport) {
            return s;
        }
    }
    return NULL;
}

static ssize_t sock_read(struct file *f, void *buf, size_t len);
static ssize_t sock_write(struct file *f, const void *buf, size_t len);
static int sock_close_file(struct file *f);

int sock_alloc_fd(struct socket *s) {
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return -1;

    for (int i = 0; i < 32; i++) {
        if (!current->process->fd_table[i]) {
            struct file *f = kzalloc(sizeof(struct file));
            if (!f) return -1;

            static struct file_operations sock_fops = {
                .read = sock_read,
                .write = sock_write,
                .close = sock_close_file,
            };

            f->fops = &sock_fops;
            f->private_data = s;
            f->ref_count = 1;
            s->fd = i;
            s->proc = current->process;
            current->process->fd_table[i] = f;
            current->process->fd_count = i + 1;
            return i;
        }
    }
    return -1;
}

void sock_free_fd(struct socket *s) {
    if (!s || !s->proc) return;
    if (s->fd >= 0 && s->fd < 32 && s->proc->fd_table[s->fd]) {
        struct file *f = s->proc->fd_table[s->fd];
        s->proc->fd_table[s->fd] = NULL;
        kfree(f);
    }
}

ssize_t sock_read(struct file *f, void *buf, size_t len) {
    struct socket *s = (struct socket *)f->private_data;
    return sock_recvfrom(s, buf, len, NULL, NULL);
}

ssize_t sock_write(struct file *f, const void *buf, size_t len) {
    struct socket *s = (struct socket *)f->private_data;
    return sock_sendto(s, buf, len, 0, 0);
}

int sock_close_file(struct file *f) {
    struct socket *s = (struct socket *)f->private_data;
    sock_close(s);
    return 0;
}

void netstat_print(void) {
    extern void uart_puts(const char *);
    extern void uart_puthex(uint32_t);
    extern void uart_putdec(uint32_t);
    extern void uart_putc(char);

    uart_puts("=== Network Interfaces ===\n");
    for (int i = 0; i < num_ifaces; i++) {
        struct net_iface *iface = ifaces[i];
        uart_puts(iface->name);
        uart_puts(": flags=");
        uart_puthex(iface->flags);
        uart_puts(" mtu=");
        uart_putdec(iface->mtu);
        uart_puts(" ip=");
        uart_puthex(iface->ip_addr);
        uart_puts(" mask=");
        uart_puthex(iface->netmask);
        uart_puts("\n");
    }

    uart_puts("\n=== Sockets ===\n");
    for (struct socket *s = sockets; s; s = s->next) {
        uart_puts("socket: proto=");
        uart_putdec(s->protocol);
        uart_puts(" lport=");
        uart_putdec(s->local_port);
        uart_puts(" rport=");
        uart_putdec(s->remote_port);
        uart_puts(" bound=");
        uart_putdec(s->bound);
        uart_puts(" conn=");
        uart_putdec(s->connected);
        uart_puts("\n");
    }

    uart_puts("\n=== ARP Table ===\n");
    for (int i = 0; i < NET_MAX_ARP_ENTRIES; i++) {
        if (arp_table[i].used) {
            uart_puts("IP=");
            uart_puthex(arp_table[i].ip_addr);
            uart_puts(" MAC=");
            for (int j = 0; j < 6; j++) {
                uart_puthex(arp_table[i].mac[j]);
                if (j < 5) uart_puts(":");
            }
            uart_puts("\n");
        }
    }
}