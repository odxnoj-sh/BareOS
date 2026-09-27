#ifndef WIFI_REG_H
#define WIFI_REG_H

#include <stdint.h>

#define DR_REG_WIFI_BASE 0x60036000
#define DR_REG_WIFI_MAC_BASE 0x60036000
#define DR_REG_WIFI_PWR_BASE 0x60036800

#define WIFI_MAC_BASE DR_REG_WIFI_MAC_BASE
#define WIFI_PWR_BASE DR_REG_WIFI_PWR_BASE

#define WIFI_MAC_INT_RAW_REG(base) ((base) + 0x0000)
#define WIFI_MAC_INT_ST_REG(base) ((base) + 0x0004)
#define WIFI_MAC_INT_ENA_REG(base) ((base) + 0x0008)
#define WIFI_MAC_INT_CLR_REG(base) ((base) + 0x000C)

#define WIFI_MAC_TXCTRL_REG(base) ((base) + 0x0010)
#define WIFI_MAC_TXCTRL1_REG(base) ((base) + 0x0014)
#define WIFI_MAC_RXCTRL_REG(base) ((base) + 0x0020)
#define WIFI_MAC_RXCTRL1_REG(base) ((base) + 0x0024)

#define WIFI_MAC_ADDR0_REG(base) ((base) + 0x0040)
#define WIFI_MAC_ADDR1_REG(base) ((base) + 0x0044)

#define WIFI_MAC_BSSID0_REG(base) ((base) + 0x0048)
#define WIFI_MAC_BSSID1_REG(base) ((base) + 0x004C)

#define WIFI_MAC_TX_STATUS_REG(base) ((base) + 0x0060)
#define WIFI_MAC_RX_STATUS_REG(base) ((base) + 0x0064)

#define WIFI_MAC_RX_FILTER_REG(base) ((base) + 0x0080)
#define WIFI_MAC_TX_QUEUE_REG(base) ((base) + 0x0090)

#define WIFI_MAC_DMA_IN_CONF_REG(base) ((base) + 0x0100)
#define WIFI_MAC_DMA_OUT_CONF_REG(base) ((base) + 0x0110)

#define WIFI_MAC_RX_DMA_DESC_REG(base) ((base) + 0x0120)
#define WIFI_MAC_TX_DMA_DESC_REG(base) ((base) + 0x0130)

#define WIFI_MAC_MAILBOX_REG(base) ((base) + 0x0200)
#define WIFI_MAC_MAILBOX_DATA_REG(base) ((base) + 0x0204)

#define WIFI_MAC_RATE_CTRL_REG(base) ((base) + 0x0300)
#define WIFI_MAC_CRYPTO_CTRL_REG(base) ((base) + 0x0310)

#define WIFI_PWR_CLK_EN_REG(base) ((base) + 0x0000)
#define WIFI_PWR_RST_EN_REG(base) ((base) + 0x0004)
#define WIFI_PWR_CTRL_REG(base) ((base) + 0x0010)

#define WIFI_PWR_CLK_EN_WIFI_MAC (1 << 0)
#define WIFI_PWR_CLK_EN_WIFI_BB (1 << 1)
#define WIFI_PWR_CLK_EN_WIFI_RF (1 << 2)

#define WIFI_PWR_RST_WIFI_MAC (1 << 0)
#define WIFI_PWR_RST_WIFI_BB (1 << 1)
#define WIFI_PWR_RST_WIFI_RF (1 << 2)

#define WIFI_INT_RX_DONE (1 << 0)
#define WIFI_INT_TX_DONE (1 << 1)
#define WIFI_INT_RX_ERR (1 << 2)
#define WIFI_INT_TX_ERR (1 << 3)
#define WIFI_INT_MGMT_RX (1 << 4)
#define WIFI_INT_MGMT_TX (1 << 5)

#define WIFI_MAC_TXCTRL_TX_EN (1 << 0)
#define WIFI_MAC_RXCTRL_RX_EN (1 << 0)
#define WIFI_MAC_RXCTRL_PROMISC (1 << 1)

#define WIFI_DMA_DESC_OWNER_CPU 0
#define WIFI_DMA_DESC_OWNER_DMA 1

#define WIFI_MAX_FRAME_LEN 1536
#define WIFI_DMA_BUF_SIZE 1536

#define WIFI_NUM_RX_DESC 8
#define WIFI_NUM_TX_DESC 8
#define WIFI_NUM_MGMT_DESC 4

struct wifi_dma_desc {
    uint32_t owner;
    uint32_t buf_ptr;
    uint32_t buf_len;
    uint32_t next_desc;
    uint32_t reserved[4];
};

static inline uint32_t wifi_reg_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static inline void wifi_reg_write(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

#endif