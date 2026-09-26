#include <esp32s3_reg.h>
#include <stdint.h>
#include <stddef.h>

#define PSRAM_BASE 0x3F800000
#define PSRAM_SIZE (8 * 1024 * 1024)

#define SPI1_CMD_REG      (DR_REG_SPI1_BASE + 0x00)
#define SPI1_ADDR_REG     (DR_REG_SPI1_BASE + 0x04)
#define SPI1_CTRL_REG     (DR_REG_SPI1_BASE + 0x08)
#define SPI1_CTRL1_REG    (DR_REG_SPI1_BASE + 0x0C)
#define SPI1_RD_CMD_REG   (DR_REG_SPI1_BASE + 0x10)
#define SPI1_WR_CMD_REG   (DR_REG_SPI1_BASE + 0x14)
#define SPI1_DIN_MODE_REG (DR_REG_SPI1_BASE + 0x18)
#define SPI1_DIN_NUM_REG  (DR_REG_SPI1_BASE + 0x1C)
#define SPI1_DOUT_MODE_REG (DR_REG_SPI1_BASE + 0x20)
#define SPI1_DOUT_NUM_REG (DR_REG_SPI1_BASE + 0x24)
#define SPI1_MISC_REG     (DR_REG_SPI1_BASE + 0x28)
#define SPI1_CACHE_SCTRL_REG (DR_REG_SPI1_BASE + 0x30)
#define SPI1_SRAM_USR_CMD_REG (DR_REG_SPI1_BASE + 0x34)
#define SPI1_SRAM_USR_ADDR_REG (DR_REG_SPI1_BASE + 0x38)
#define SPI1_W0_REG       (DR_REG_SPI1_BASE + 0x58)
#define SPI1_W1_REG       (DR_REG_SPI1_BASE + 0x5C)

#define PSRAM_CMD_READ      0x03
#define PSRAM_CMD_FAST_READ 0x0B
#define PSRAM_CMD_WRITE     0x02
#define PSRAM_CMD_QUAD_READ 0xEB
#define PSRAM_CMD_QUAD_WRITE 0x38

#define PSRAM_CMD_READ_ID   0x9F

#define PSRAM_SIZE_8MB 0x800000

static inline uint32_t reg_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static inline void reg_write(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static void spi1_wait_idle(void) {
    while (reg_read(SPI1_CMD_REG) & (1 << 18)) {
    }
}

static uint8_t psram_read_id(void) {
    reg_write(SPI1_ADDR_REG, 0);
    reg_write(SPI1_CMD_REG, (PSRAM_CMD_READ_ID & 0xFF) | (1 << 31) | (1 << 18) | (1 << 13));
    spi1_wait_idle();
    return reg_read(SPI1_W0_REG) & 0xFF;
}

static void psram_write_cmd(uint8_t cmd, uint32_t addr, uint32_t data) {
    reg_write(SPI1_ADDR_REG, addr & 0xFFFFFF);
    reg_write(SPI1_W0_REG, data);
    reg_write(SPI1_CMD_REG, (cmd & 0xFF) | (1 << 31) | (1 << 18) | (1 << 13));
    spi1_wait_idle();
}

int psram_init(void) {
    reg_write(DR_REG_PCR_BASE + 0x130, 0x00000001);
    reg_write(DR_REG_PCR_BASE + 0x134, 0x00000001);

    reg_write(SPI1_CTRL_REG, 0);
    reg_write(SPI1_CTRL1_REG, 0);
    reg_write(SPI1_MISC_REG, 0);
    reg_write(SPI1_CACHE_SCTRL_REG, 0);

    uint8_t id = psram_read_id();
    if (id != 0x5D && id != 0x26 && id != 0x27) {
        return -1;
    }

    psram_write_cmd(0x66, 0, 0);
    psram_write_cmd(0x99, 0, 0);

    reg_write(SPI1_CACHE_SCTRL_REG, (1 << 31) | (1 << 30) | (1 << 29) | (PSRAM_SIZE_8MB - 1));
    reg_write(SPI1_MISC_REG, (1 << 13) | (1 << 14));

    reg_write(DR_REG_SYSCON_BASE + 0x00, reg_read(DR_REG_SYSCON_BASE + 0x00) | (1 << 26));

    return 0;
}

int psram_read(uint32_t addr, void *buf, size_t len) {
    if (addr >= PSRAM_SIZE) return -1;
    if (len == 0) return 0;

    uint8_t *dst = (uint8_t *)buf;
    size_t remaining = len;
    uint32_t psram_addr = PSRAM_BASE + addr;

    while (remaining > 0) {
        size_t chunk = (remaining > 64) ? 64 : remaining;
        (void)chunk;
        reg_write(SPI1_ADDR_REG, psram_addr & 0xFFFFFF);
        reg_write(SPI1_CMD_REG, (PSRAM_CMD_FAST_READ & 0xFF) | (1 << 31) | (1 << 18) | (1 << 13));
        spi1_wait_idle();

        uint32_t data = reg_read(SPI1_W0_REG);
        size_t copy_len = (remaining < 4) ? remaining : 4;
        for (size_t i = 0; i < copy_len; i++) {
            *dst++ = (data >> (i * 8)) & 0xFF;
        }

        psram_addr += copy_len;
        remaining -= copy_len;
    }

    return 0;
}

int psram_write(uint32_t addr, const void *buf, size_t len) {
    if (addr >= PSRAM_SIZE) return -1;
    if (len == 0) return 0;

    const uint8_t *src = (const uint8_t *)buf;
    size_t remaining = len;
    uint32_t psram_addr = PSRAM_BASE + addr;

    while (remaining > 0) {
        size_t page_offset = psram_addr % 256;
        size_t chunk = 256 - page_offset;
        if (chunk > remaining) chunk = remaining;

        reg_write(SPI1_ADDR_REG, psram_addr & 0xFFFFFF);
        reg_write(SPI1_W0_REG, 0);

        for (size_t i = 0; i < chunk; i += 4) {
            uint32_t word = 0;
            size_t copy = (chunk - i >= 4) ? 4 : chunk - i;
            for (size_t j = 0; j < copy; j++) {
                word |= (uint32_t)src[i + j] << (j * 8);
            }
            reg_write(SPI1_W0_REG + i, word);
        }

        reg_write(SPI1_CMD_REG, (PSRAM_CMD_WRITE & 0xFF) | (1 << 31) | (1 << 18) | (1 << 13));
        spi1_wait_idle();

        psram_addr += chunk;
        src += chunk;
        remaining -= chunk;
    }

    return 0;
}

uint32_t psram_get_size(void) {
    return PSRAM_SIZE;
}