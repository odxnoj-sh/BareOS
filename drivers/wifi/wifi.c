#include <wifi_reg.h>
#include <esp32s3_reg.h>
#include <xtensa.h>
#include <kernel.h>
#include <memory.h>
#include <string.h>
#include <net.h>

#define WIFI_STATE_DOWN 0
#define WIFI_STATE_INIT 1
#define WIFI_STATE_UP 2
#define WIFI_STATE_ERROR 3

static int wifi_state = WIFI_STATE_DOWN;
static uint8_t wifi_mac[6];
static struct net_iface *wifi_iface = NULL;
static struct task *wifi_rx_task = NULL;
static struct semaphore wifi_rx_sem;

static struct wifi_dma_desc rx_descs[WIFI_NUM_RX_DESC] __attribute__((aligned(16)));
static struct wifi_dma_desc tx_descs[WIFI_NUM_TX_DESC] __attribute__((aligned(16)));
static uint8_t rx_buffers[WIFI_NUM_RX_DESC][WIFI_DMA_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buffers[WIFI_NUM_TX_DESC][WIFI_DMA_BUF_SIZE] __attribute__((aligned(16)));

static volatile int rx_desc_head = 0;
static volatile int tx_desc_head = 0;
static volatile int tx_desc_tail = 0;

static int wifi_transmit(struct net_iface *iface, struct pbuf *p);

static void wifi_read_mac_address(void) {
    uint32_t mac_low = wifi_reg_read(WIFI_MAC_ADDR0_REG(WIFI_MAC_BASE));
    uint32_t mac_high = wifi_reg_read(WIFI_MAC_ADDR1_REG(WIFI_MAC_BASE));

    wifi_mac[0] = (mac_low >> 24) & 0xFF;
    wifi_mac[1] = (mac_low >> 16) & 0xFF;
    wifi_mac[2] = (mac_low >> 8) & 0xFF;
    wifi_mac[3] = mac_low & 0xFF;
    wifi_mac[4] = (mac_high >> 8) & 0xFF;
    wifi_mac[5] = mac_high & 0xFF;
}

static int wifi_hw_init(void) {
    wifi_reg_write(WIFI_PWR_CLK_EN_REG(WIFI_PWR_BASE), 
                   WIFI_PWR_CLK_EN_WIFI_MAC | WIFI_PWR_CLK_EN_WIFI_BB | WIFI_PWR_CLK_EN_WIFI_RF);

    wifi_reg_write(WIFI_PWR_RST_EN_REG(WIFI_PWR_BASE), 0);
    for (volatile int i = 0; i < 1000; i++);
    wifi_reg_write(WIFI_PWR_RST_EN_REG(WIFI_PWR_BASE), 
                   WIFI_PWR_RST_WIFI_MAC | WIFI_PWR_RST_WIFI_BB | WIFI_PWR_RST_WIFI_RF);
    for (volatile int i = 0; i < 1000; i++);

    wifi_reg_write(WIFI_MAC_INT_ENA_REG(WIFI_MAC_BASE), 0);
    wifi_reg_write(WIFI_MAC_INT_CLR_REG(WIFI_MAC_BASE), 0xFFFFFFFF);

    return 0;
}

static void wifi_dma_init(void) {
    for (int i = 0; i < WIFI_NUM_RX_DESC; i++) {
        rx_descs[i].owner = WIFI_DMA_DESC_OWNER_DMA;
        rx_descs[i].buf_ptr = (uint32_t)&rx_buffers[i][0];
        rx_descs[i].buf_len = WIFI_DMA_BUF_SIZE;
        rx_descs[i].next_desc = (uint32_t)&rx_descs[(i + 1) % WIFI_NUM_RX_DESC];
    }

    for (int i = 0; i < WIFI_NUM_TX_DESC; i++) {
        tx_descs[i].owner = WIFI_DMA_DESC_OWNER_CPU;
        tx_descs[i].buf_ptr = (uint32_t)&tx_buffers[i][0];
        tx_descs[i].buf_len = 0;
        tx_descs[i].next_desc = (uint32_t)&tx_descs[(i + 1) % WIFI_NUM_TX_DESC];
    }

    wifi_reg_write(WIFI_MAC_RX_DMA_DESC_REG(WIFI_MAC_BASE), (uint32_t)&rx_descs[0]);
    wifi_reg_write(WIFI_MAC_TX_DMA_DESC_REG(WIFI_MAC_BASE), (uint32_t)&tx_descs[0]);

    wifi_reg_write(WIFI_MAC_DMA_IN_CONF_REG(WIFI_MAC_BASE), 0x80000001);
    wifi_reg_write(WIFI_MAC_DMA_OUT_CONF_REG(WIFI_MAC_BASE), 0x80000001);
}

static void wifi_rx_task_func(void) {
    while (1) {
        semaphore_wait(&wifi_rx_sem);

        int desc_idx = rx_desc_head;
        struct wifi_dma_desc *desc = &rx_descs[desc_idx];

        if (desc->owner == WIFI_DMA_DESC_OWNER_CPU) {
            uint32_t frame_len = desc->buf_len;
            if (frame_len > 0 && frame_len <= WIFI_DMA_BUF_SIZE) {
                struct pbuf *p = pbuf_alloc(frame_len + ETH_HLEN);
                if (p) {
                    memcpy(p->data, (void *)desc->buf_ptr, frame_len);
                    p->len = frame_len;
                    p->iface = wifi_iface;
                    net_iface_input(wifi_iface, p);
                }
            }
            desc->buf_len = WIFI_DMA_BUF_SIZE;
            desc->owner = WIFI_DMA_DESC_OWNER_DMA;
            rx_desc_head = (rx_desc_head + 1) % WIFI_NUM_RX_DESC;
        }
    }
}

void wifi_isr(void) {
    uint32_t int_raw = wifi_reg_read(WIFI_MAC_INT_RAW_REG(WIFI_MAC_BASE));
    wifi_reg_write(WIFI_MAC_INT_CLR_REG(WIFI_MAC_BASE), int_raw);

    if (int_raw & WIFI_INT_RX_DONE) {
        semaphore_signal(&wifi_rx_sem);
    }
}

int wifi_init(void) {
    if (wifi_state != WIFI_STATE_DOWN) {
        return -1;
    }

    wifi_state = WIFI_STATE_INIT;

    if (wifi_hw_init() != 0) {
        wifi_state = WIFI_STATE_ERROR;
        return -1;
    }

    wifi_read_mac_address();

    wifi_dma_init();

    semaphore_init(&wifi_rx_sem, 0);

    wifi_iface = net_iface_create("wlan0", 0, 0, WIFI_MAX_FRAME_LEN);
    if (!wifi_iface) {
        wifi_state = WIFI_STATE_ERROR;
        return -1;
    }

    memcpy(wifi_iface->mac, wifi_mac, ETH_ALEN);
    wifi_iface->transmit = wifi_transmit;
    wifi_iface->mtu = WIFI_MAX_FRAME_LEN;

    net_iface_up(wifi_iface);

    wifi_rx_task = task_create_user(wifi_rx_task_func, PRIORITY_DEFAULT, 4096);
    if (!wifi_rx_task) {
        wifi_state = WIFI_STATE_ERROR;
        return -1;
    }
    task_wake(wifi_rx_task);

    wifi_reg_write(WIFI_MAC_RXCTRL_REG(WIFI_MAC_BASE), WIFI_MAC_RXCTRL_RX_EN);
    wifi_reg_write(WIFI_MAC_TXCTRL_REG(WIFI_MAC_BASE), WIFI_MAC_TXCTRL_TX_EN);
    wifi_reg_write(WIFI_MAC_INT_ENA_REG(WIFI_MAC_BASE), WIFI_INT_RX_DONE | WIFI_INT_TX_DONE | WIFI_INT_RX_ERR | WIFI_INT_TX_ERR);

    wifi_state = WIFI_STATE_UP;
    return 0;
}

int wifi_transmit(struct net_iface *iface, struct pbuf *p) {
    if (!iface || !p || wifi_state != WIFI_STATE_UP) {
        return -1;
    }

    if (p->len > WIFI_DMA_BUF_SIZE) {
        return -1;
    }

    int desc_idx = tx_desc_head;
    struct wifi_dma_desc *desc = &tx_descs[desc_idx];

    if (desc->owner == WIFI_DMA_DESC_OWNER_DMA) {
        return -1;
    }

    memcpy((void *)desc->buf_ptr, p->data, p->len);
    desc->buf_len = p->len;
    desc->owner = WIFI_DMA_DESC_OWNER_DMA;

    tx_desc_head = (tx_desc_head + 1) % WIFI_NUM_TX_DESC;

    return 0;
}

int wifi_start(void) {
    if (wifi_state != WIFI_STATE_UP) {
        return -1;
    }
    return 0;
}

int wifi_stop(void) {
    if (wifi_state == WIFI_STATE_DOWN) {
        return -1;
    }

    wifi_reg_write(WIFI_MAC_INT_ENA_REG(WIFI_MAC_BASE), 0);
    wifi_reg_write(WIFI_MAC_RXCTRL_REG(WIFI_MAC_BASE), 0);
    wifi_reg_write(WIFI_MAC_TXCTRL_REG(WIFI_MAC_BASE), 0);

    if (wifi_rx_task) {
        task_destroy(wifi_rx_task);
        wifi_rx_task = NULL;
    }

    if (wifi_iface) {
        net_iface_down(wifi_iface);
    }

    wifi_state = WIFI_STATE_DOWN;
    return 0;
}

int wifi_set_ip(uint32_t ip, uint32_t mask, uint32_t gw) {
    if (!wifi_iface) return -1;
    wifi_iface->ip_addr = ip;
    wifi_iface->netmask = mask;
    wifi_iface->gateway = gw;
    return 0;
}

int wifi_get_state(void) {
    return wifi_state;
}

const uint8_t *wifi_get_mac(void) {
    return wifi_mac;
}

int wifi_assoc(const char *ssid, const char *password) {
    (void)ssid;
    (void)password;
    return 0;
}