#include <wifi_reg.h>
#include <esp32s3_reg.h>
#include <xtensa.h>
#include <kernel.h>
#include <memory.h>
#include <string.h>
#include <net.h>
#include <wifi.h>

#define WIFI_STATE_DOWN 0
#define WIFI_STATE_INIT 1
#define WIFI_STATE_SCANNING 2
#define WIFI_STATE_AUTHENTICATING 3
#define WIFI_STATE_ASSOCIATING 4
#define WIFI_STATE_ASSOCIATED 5
#define WIFI_STATE_FAILED 6
#define WIFI_STATE_ERROR WIFI_STATE_FAILED
#define WIFI_STATE_UP WIFI_STATE_ASSOCIATED

#define WIFI_SEC_OPEN 0
#define WIFI_SEC_WPA_PSK 1

#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAX_PASS_LEN 64

#define WIFI_MGMT_PROBE_REQ 0x40
#define WIFI_MGMT_PROBE_RESP 0x50
#define WIFI_MGMT_AUTH 0xB0
#define WIFI_MGMT_ASSOC_REQ 0x00
#define WIFI_MGMT_ASSOC_RESP 0x10
#define WIFI_MGMT_DISASSOC 0xA0
#define WIFI_MGMT_DEAUTH 0xC0

#define WIFI_AUTH_OPEN 0
#define WIFI_AUTH_SHARED 1

#define WIFI_REASON_UNSPECIFIED 1
#define WIFI_REASON_AUTH_EXPIRE 2
#define WIFI_REASON_AUTH_LEAVE 3
#define WIFI_REASON_ASSOC_EXPIRE 4
#define WIFI_REASON_ASSOC_TOOMANY 5
#define WIFI_REASON_NOT_AUTHED 6
#define WIFI_REASON_NOT_ASSOCED 7

#define WIFI_CAP_ESS (1 << 0)
#define WIFI_CAP_IBSS (1 << 1)
#define WIFI_CAP_PRIVACY (1 << 4)
#define WIFI_CAP_SHORT_PREAMBLE (1 << 5)
#define WIFI_CAP_SHORT_SLOT (1 << 10)

#define WIFI_RATE_1M 0x02
#define WIFI_RATE_2M 0x04
#define WIFI_RATE_5_5M 0x0B
#define WIFI_RATE_11M 0x16
#define WIFI_RATE_6M 0x0C
#define WIFI_RATE_9M 0x12
#define WIFI_RATE_12M 0x18
#define WIFI_RATE_18M 0x24
#define WIFI_RATE_24M 0x30
#define WIFI_RATE_36M 0x48
#define WIFI_RATE_48M 0x60
#define WIFI_RATE_54M 0x6C

#define WIFI_SCAN_TIMEOUT 5000
#define WIFI_AUTH_TIMEOUT 3000
#define WIFI_ASSOC_TIMEOUT 3000
#define WIFI_MAX_RETRIES 3

#define WIFI_MGMT_TX_DESC 3

static int wifi_state = WIFI_STATE_DOWN;
static uint8_t wifi_mac[6];
static struct net_iface *wifi_iface = NULL;
static struct task *wifi_rx_task = NULL;
static struct task *wifi_mgmt_task = NULL;
static struct semaphore wifi_rx_sem;
static struct semaphore wifi_mgmt_sem;

static struct wifi_dma_desc rx_descs[WIFI_NUM_RX_DESC] __attribute__((aligned(16)));
static struct wifi_dma_desc tx_descs[WIFI_NUM_TX_DESC] __attribute__((aligned(16)));
static struct wifi_dma_desc mgmt_descs[WIFI_NUM_MGMT_DESC] __attribute__((aligned(16)));
static uint8_t rx_buffers[WIFI_NUM_RX_DESC][WIFI_DMA_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buffers[WIFI_NUM_TX_DESC][WIFI_DMA_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t mgmt_buffers[WIFI_NUM_MGMT_DESC][WIFI_DMA_BUF_SIZE] __attribute__((aligned(16)));

static volatile int rx_desc_head = 0;
static volatile int tx_desc_head = 0;
static volatile int mgmt_desc_head = 0;

static char wifi_ssid[WIFI_MAX_SSID_LEN + 1] = {0};
static char wifi_pass[WIFI_MAX_PASS_LEN + 1] = {0};
static uint8_t wifi_sec_mode = WIFI_SEC_OPEN;
static uint8_t wifi_bssid[6] = {0};
static uint8_t wifi_channel = 0;
static int wifi_rssi = 0;
static uint16_t wifi_seq_num = 0;
static int wifi_retry_count = 0;
static uint32_t wifi_assoc_deadline = 0;

static int wifi_mgmt_transmit(uint8_t *data, size_t len);
static void wifi_read_mac_address(void);
static void wifi_mgmt_task_func(void);
static void wifi_process_mgmt_frame(uint8_t *data, size_t len);
static int wifi_build_probe_req(uint8_t *buf);
static int wifi_build_auth_req(uint8_t *buf);
static int wifi_build_assoc_req(uint8_t *buf);
static void wifi_parse_probe_resp(uint8_t *data, size_t len);
static void wifi_parse_auth_resp(uint8_t *data, size_t len);
static void wifi_parse_assoc_resp(uint8_t *data, size_t len);
static void wifi_set_state(int new_state);
static void wifi_set_assoc_deadline(int ms);

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

    for (int i = 0; i < WIFI_NUM_MGMT_DESC; i++) {
        mgmt_descs[i].owner = WIFI_DMA_DESC_OWNER_CPU;
        mgmt_descs[i].buf_ptr = (uint32_t)&mgmt_buffers[i][0];
        mgmt_descs[i].buf_len = 0;
        mgmt_descs[i].next_desc = (uint32_t)&mgmt_descs[(i + 1) % WIFI_NUM_MGMT_DESC];
    }

    wifi_reg_write(WIFI_MAC_RX_DMA_DESC_REG(WIFI_MAC_BASE), (uint32_t)&rx_descs[0]);
    wifi_reg_write(WIFI_MAC_TX_DMA_DESC_REG(WIFI_MAC_BASE), (uint32_t)&tx_descs[0]);
    wifi_reg_write(WIFI_MAC_TX_DMA_DESC_REG(WIFI_MAC_BASE) + 0x20, (uint32_t)&mgmt_descs[0]);

    wifi_reg_write(WIFI_MAC_DMA_IN_CONF_REG(WIFI_MAC_BASE), 0x80000001);
    wifi_reg_write(WIFI_MAC_DMA_OUT_CONF_REG(WIFI_MAC_BASE), 0x80000001);
}

static void wifi_set_state(int new_state) {
    wifi_state = new_state;
    if (new_state == WIFI_STATE_FAILED) {
        wifi_assoc_deadline = 0;
        wifi_retry_count = 0;
    }
}

static void wifi_set_assoc_deadline(int ms) {
    wifi_assoc_deadline = cpu_states[0].tick_count + ms;
}

static int wifi_check_deadline(void) {
    return wifi_assoc_deadline && cpu_states[0].tick_count >= wifi_assoc_deadline;
}

static int wifi_mgmt_transmit(uint8_t *data, size_t len) {
    if (wifi_state == WIFI_STATE_DOWN || wifi_state == WIFI_STATE_INIT || wifi_state == WIFI_STATE_FAILED) {
        return -1;
    }
    if (len > WIFI_DMA_BUF_SIZE) {
        return -1;
    }

    struct wifi_dma_desc *desc = &mgmt_descs[mgmt_desc_head];

    if (desc->owner == WIFI_DMA_DESC_OWNER_DMA) {
        return -1;
    }

    memcpy((void *)desc->buf_ptr, data, len);
    desc->buf_len = len;
    desc->owner = WIFI_DMA_DESC_OWNER_DMA;

    mgmt_desc_head = (mgmt_desc_head + 1) % WIFI_NUM_MGMT_DESC;

    return 0;
}

static void wifi_process_mgmt_frame(uint8_t *data, size_t len) {
    if (len < 24) return;

    uint8_t frame_type = data[0] & 0xFC;
    uint8_t *mac_hdr = data;
    uint8_t *mgmt_body = data + 24;
    size_t body_len = len - 24;

    uint8_t *da = mac_hdr + 4;
    uint8_t *sa = mac_hdr + 10;
    (void)(mac_hdr + 16);

    if (memcmp(da, wifi_mac, 6) != 0 && memcmp(da, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) != 0) {
        return;
    }

    switch (frame_type) {
        case WIFI_MGMT_PROBE_RESP:
            if (wifi_state == WIFI_STATE_SCANNING) {
                wifi_parse_probe_resp(mgmt_body, body_len);
            }
            break;
        case WIFI_MGMT_AUTH:
            if (wifi_state == WIFI_STATE_AUTHENTICATING) {
                wifi_parse_auth_resp(mgmt_body, body_len);
            }
            break;
        case WIFI_MGMT_ASSOC_RESP:
            if (wifi_state == WIFI_STATE_ASSOCIATING) {
                wifi_parse_assoc_resp(mgmt_body, body_len);
            }
            break;
        case WIFI_MGMT_DEAUTH:
        case WIFI_MGMT_DISASSOC:
            if (wifi_state == WIFI_STATE_ASSOCIATED && memcmp(sa, wifi_bssid, 6) == 0) {
                wifi_set_state(WIFI_STATE_FAILED);
            }
            break;
        default:
            break;
    }
}

static void wifi_rx_task_func(void) {
    while (1) {
        semaphore_wait(&wifi_rx_sem);

        int desc_idx = rx_desc_head;
        struct wifi_dma_desc *desc = &rx_descs[desc_idx];

        if (desc->owner == WIFI_DMA_DESC_OWNER_CPU) {
            uint32_t frame_len = desc->buf_len;
            uint8_t *frame_data = (uint8_t *)desc->buf_ptr;

            if (frame_len > 0 && frame_len <= WIFI_DMA_BUF_SIZE) {
                uint8_t frame_type = frame_data[0] & 0xFC;

                if (frame_type == WIFI_MGMT_PROBE_RESP || frame_type == WIFI_MGMT_AUTH ||
                    frame_type == WIFI_MGMT_ASSOC_RESP || frame_type == WIFI_MGMT_DEAUTH ||
                    frame_type == WIFI_MGMT_DISASSOC) {
                    wifi_process_mgmt_frame(frame_data, frame_len);
                } else {
                    struct pbuf *p = pbuf_alloc(frame_len + ETH_HLEN);
                    if (p) {
                        memcpy(p->data, frame_data, frame_len);
                        p->len = frame_len;
                        p->iface = wifi_iface;
                        net_iface_input(wifi_iface, p);
                    }
                }
            }
            desc->buf_len = WIFI_DMA_BUF_SIZE;
            desc->owner = WIFI_DMA_DESC_OWNER_DMA;
            rx_desc_head = (rx_desc_head + 1) % WIFI_NUM_RX_DESC;
        }
    }
}

static void wifi_mgmt_task_func(void) {
    while (1) {
        semaphore_wait(&wifi_mgmt_sem);

        if (wifi_state == WIFI_STATE_SCANNING) {
            if (wifi_check_deadline()) {
                if (wifi_rssi == 0) {
                    if (wifi_retry_count < WIFI_MAX_RETRIES) {
                        wifi_retry_count++;
                        wifi_set_assoc_deadline(WIFI_SCAN_TIMEOUT);
                        int ret = wifi_mgmt_transmit((uint8_t *)"scan_retry", 10);
                        (void)ret;
                    } else {
                        wifi_set_state(WIFI_STATE_FAILED);
                    }
                } else {
                    int ret = wifi_mgmt_transmit((uint8_t *)"auth_req", 8);
                    (void)ret;
                    wifi_set_state(WIFI_STATE_AUTHENTICATING);
                    wifi_set_assoc_deadline(WIFI_AUTH_TIMEOUT);
                }
            } else {
                int ret = wifi_build_probe_req((uint8_t *)mgmt_buffers[mgmt_desc_head]);
                if (ret > 0) {
                    struct wifi_dma_desc *desc = &mgmt_descs[mgmt_desc_head];
                    desc->buf_len = ret;
                    desc->owner = WIFI_DMA_DESC_OWNER_DMA;
                    mgmt_desc_head = (mgmt_desc_head + 1) % WIFI_NUM_MGMT_DESC;
                }
            }
        } else if (wifi_state == WIFI_STATE_AUTHENTICATING) {
            if (wifi_check_deadline()) {
                if (wifi_retry_count < WIFI_MAX_RETRIES) {
                    wifi_retry_count++;
                    wifi_set_assoc_deadline(WIFI_AUTH_TIMEOUT);
                    int ret = wifi_build_auth_req((uint8_t *)mgmt_buffers[mgmt_desc_head]);
                    if (ret > 0) {
                        struct wifi_dma_desc *desc = &mgmt_descs[mgmt_desc_head];
                        desc->buf_len = ret;
                        desc->owner = WIFI_DMA_DESC_OWNER_DMA;
                        mgmt_desc_head = (mgmt_desc_head + 1) % WIFI_NUM_MGMT_DESC;
                    }
                } else {
                    wifi_set_state(WIFI_STATE_FAILED);
                }
            }
        } else if (wifi_state == WIFI_STATE_ASSOCIATING) {
            if (wifi_check_deadline()) {
                if (wifi_retry_count < WIFI_MAX_RETRIES) {
                    wifi_retry_count++;
                    wifi_set_assoc_deadline(WIFI_ASSOC_TIMEOUT);
                    int ret = wifi_build_assoc_req((uint8_t *)mgmt_buffers[mgmt_desc_head]);
                    if (ret > 0) {
                        struct wifi_dma_desc *desc = &mgmt_descs[mgmt_desc_head];
                        desc->buf_len = ret;
                        desc->owner = WIFI_DMA_DESC_OWNER_DMA;
                        mgmt_desc_head = (mgmt_desc_head + 1) % WIFI_NUM_MGMT_DESC;
                    }
                } else {
                    wifi_set_state(WIFI_STATE_FAILED);
                }
            }
        }
    }
}

static int wifi_build_probe_req(uint8_t *buf) {
    uint8_t *p = buf;

    p[0] = WIFI_MGMT_PROBE_REQ;
    p[1] = 0;
    p[2] = 0; p[3] = 0;
    memset(p + 4, 0xFF, 6);
    memcpy(p + 10, wifi_mac, 6);
    memset(p + 16, 0xFF, 6);
    p[22] = wifi_seq_num & 0xFF;
    p[23] = (wifi_seq_num >> 8) & 0xFF;
    wifi_seq_num++;

    p += 24;

    *p++ = 0;
    *p++ = (uint8_t)strlen(wifi_ssid);
    memcpy(p, wifi_ssid, strlen(wifi_ssid));
    p += strlen(wifi_ssid);

    *p++ = 1;
    *p++ = 8;
    *p++ = WIFI_RATE_1M | 0x80;
    *p++ = WIFI_RATE_2M | 0x80;
    *p++ = WIFI_RATE_5_5M | 0x80;
    *p++ = WIFI_RATE_11M | 0x80;
    *p++ = WIFI_RATE_6M;
    *p++ = WIFI_RATE_9M;
    *p++ = WIFI_RATE_12M;
    *p++ = WIFI_RATE_18M;

    return p - buf;
}

static int wifi_build_auth_req(uint8_t *buf) {
    uint8_t *p = buf;

    p[0] = WIFI_MGMT_AUTH;
    p[1] = 0;
    p[2] = 0; p[3] = 0;
    memcpy(p + 4, wifi_bssid, 6);
    memcpy(p + 10, wifi_mac, 6);
    memcpy(p + 16, wifi_bssid, 6);
    p[22] = wifi_seq_num & 0xFF;
    p[23] = (wifi_seq_num >> 8) & 0xFF;
    wifi_seq_num++;

    p += 24;

    *p++ = WIFI_AUTH_OPEN;
    *p++ = 0;
    *p++ = 1;
    *p++ = 0;

    return p - buf;
}

static int wifi_build_assoc_req(uint8_t *buf) {
    uint8_t *p = buf;

    p[0] = WIFI_MGMT_ASSOC_REQ;
    p[1] = 0;
    p[2] = 0; p[3] = 0;
    memcpy(p + 4, wifi_bssid, 6);
    memcpy(p + 10, wifi_mac, 6);
    memcpy(p + 16, wifi_bssid, 6);
    p[22] = wifi_seq_num & 0xFF;
    p[23] = (wifi_seq_num >> 8) & 0xFF;
    wifi_seq_num++;

    p += 24;

    *p++ = WIFI_CAP_ESS;
    *p++ = 0;
    *p++ = 0; *p++ = 0;
    *p++ = 0;
    *p++ = (uint8_t)strlen(wifi_ssid);
    memcpy(p, wifi_ssid, strlen(wifi_ssid));
    p += strlen(wifi_ssid);

    *p++ = 1;
    *p++ = 8;
    *p++ = WIFI_RATE_1M | 0x80;
    *p++ = WIFI_RATE_2M | 0x80;
    *p++ = WIFI_RATE_5_5M | 0x80;
    *p++ = WIFI_RATE_11M | 0x80;
    *p++ = WIFI_RATE_6M;
    *p++ = WIFI_RATE_9M;
    *p++ = WIFI_RATE_12M;
    *p++ = WIFI_RATE_18M;
    *p++ = WIFI_RATE_24M;
    *p++ = WIFI_RATE_36M;
    *p++ = WIFI_RATE_48M;
    *p++ = WIFI_RATE_54M;

    return p - buf;
}

static void wifi_parse_probe_resp(uint8_t *data, size_t len) {
    if (len < 12) return;

    uint8_t *p = data;
    p += 8;
    p += 2;
    p += 2;

    while (p - data < (int)len - 2) {
        uint8_t elem_id = *p++;
        uint8_t elem_len = *p++;

        if (elem_id == 0) {
            if (elem_len == strlen(wifi_ssid) && memcmp(p, wifi_ssid, elem_len) == 0) {
                wifi_rssi = -50;
            }
        } else if (elem_id == 3) {
            if (elem_len == 1) {
                wifi_channel = *p;
            }
        }
        p += elem_len;
    }
}

static void wifi_parse_auth_resp(uint8_t *data, size_t len) {
    if (len < 6) return;

    uint16_t auth_alg = data[0] | (data[1] << 8);
    uint16_t auth_seq = data[2] | (data[3] << 8);
    uint16_t status = data[4] | (data[5] << 8);

    if (auth_alg == WIFI_AUTH_OPEN && auth_seq == 2 && status == 0) {
        wifi_retry_count = 0;
        wifi_set_state(WIFI_STATE_ASSOCIATING);
        wifi_set_assoc_deadline(WIFI_ASSOC_TIMEOUT);
        int ret = wifi_build_assoc_req((uint8_t *)mgmt_buffers[mgmt_desc_head]);
        if (ret > 0) {
            struct wifi_dma_desc *desc = &mgmt_descs[mgmt_desc_head];
            desc->buf_len = ret;
            desc->owner = WIFI_DMA_DESC_OWNER_DMA;
            mgmt_desc_head = (mgmt_desc_head + 1) % WIFI_NUM_MGMT_DESC;
        }
    } else {
        wifi_set_state(WIFI_STATE_FAILED);
    }
}

static void wifi_parse_assoc_resp(uint8_t *data, size_t len) {
    if (len < 6) return;

    (void)(data[0] | (data[1] << 8));
    uint16_t status = data[2] | (data[3] << 8);
    (void)(data[4] | (data[5] << 8));

    if (status == 0) {
        wifi_set_state(WIFI_STATE_ASSOCIATED);
        wifi_retry_count = 0;
        wifi_assoc_deadline = 0;
    } else {
        wifi_set_state(WIFI_STATE_FAILED);
    }
}

void wifi_isr(void) {
    uint32_t int_raw = wifi_reg_read(WIFI_MAC_INT_RAW_REG(WIFI_MAC_BASE));
    wifi_reg_write(WIFI_MAC_INT_CLR_REG(WIFI_MAC_BASE), int_raw);

    if (int_raw & WIFI_INT_RX_DONE) {
        semaphore_signal(&wifi_rx_sem);
    }
    if (int_raw & WIFI_INT_MGMT_RX) {
        semaphore_signal(&wifi_mgmt_sem);
    }
}

void wifi_set_credentials(const char *ssid, const char *pass, int sec_mode) {
    if (ssid) {
        strncpy(wifi_ssid, ssid, WIFI_MAX_SSID_LEN);
        wifi_ssid[WIFI_MAX_SSID_LEN] = 0;
    }
    if (pass) {
        strncpy(wifi_pass, pass, WIFI_MAX_PASS_LEN);
        wifi_pass[WIFI_MAX_PASS_LEN] = 0;
    }
    wifi_sec_mode = sec_mode;
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
    semaphore_init(&wifi_mgmt_sem, 0);

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

    wifi_mgmt_task = task_create_user(wifi_mgmt_task_func, PRIORITY_DEFAULT, 4096);
    if (!wifi_mgmt_task) {
        wifi_state = WIFI_STATE_ERROR;
        return -1;
    }
    task_wake(wifi_mgmt_task);

    wifi_reg_write(WIFI_MAC_RXCTRL_REG(WIFI_MAC_BASE), WIFI_MAC_RXCTRL_RX_EN | WIFI_MAC_RXCTRL_PROMISC);
    wifi_reg_write(WIFI_MAC_TXCTRL_REG(WIFI_MAC_BASE), WIFI_MAC_TXCTRL_TX_EN);
    wifi_reg_write(WIFI_MAC_INT_ENA_REG(WIFI_MAC_BASE), WIFI_INT_RX_DONE | WIFI_INT_TX_DONE | WIFI_INT_RX_ERR | WIFI_INT_TX_ERR | WIFI_INT_MGMT_RX | WIFI_INT_MGMT_TX);

    wifi_set_state(WIFI_STATE_UP);
    return 0;
}

int wifi_assoc(const char *ssid, const char *password) {
    if (!ssid || wifi_state != WIFI_STATE_UP) {
        return -1;
    }

    if (wifi_sec_mode != WIFI_SEC_OPEN) {
        return -2;
    }

    wifi_set_credentials(ssid, password, WIFI_SEC_OPEN);

    wifi_retry_count = 0;
    wifi_set_state(WIFI_STATE_SCANNING);
    wifi_set_assoc_deadline(WIFI_SCAN_TIMEOUT);

    semaphore_signal(&wifi_mgmt_sem);

    return 0;
}

int wifi_transmit(struct net_iface *iface, struct pbuf *p) {
    if (!iface || !p || wifi_state != WIFI_STATE_ASSOCIATED) {
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
    if (wifi_state != WIFI_STATE_UP && wifi_state != WIFI_STATE_ASSOCIATED) {
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
    if (wifi_mgmt_task) {
        task_destroy(wifi_mgmt_task);
        wifi_mgmt_task = NULL;
    }

    if (wifi_iface) {
        net_iface_down(wifi_iface);
    }

    wifi_set_state(WIFI_STATE_DOWN);
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

const char *wifi_get_ssid(void) {
    return wifi_ssid;
}

int wifi_get_rssi(void) {
    return wifi_rssi;
}

uint8_t wifi_get_channel(void) {
    return wifi_channel;
}