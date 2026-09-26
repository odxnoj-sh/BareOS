#include <esp32s3_reg.h>
#include <stdint.h>
#include <stddef.h>

#define FLASH_SECTOR_SIZE 4096
#define FLASH_BLOCK_SIZE 65536
#define FLASH_PAGE_SIZE 256

#define FLASH_BASE_ADDR 0x3F800000

#define SPI0_CMD_REG SPI_MEM_CMD_REG(DR_REG_SPIMEM_BASE)
#define SPI0_ADDR_REG SPI_MEM_ADDR_REG(DR_REG_SPIMEM_BASE)
#define SPI0_CTRL_REG SPI_MEM_CTRL_REG(DR_REG_SPIMEM_BASE)
#define SPI0_CTRL1_REG SPI_MEM_CTRL1_REG(DR_REG_SPIMEM_BASE)
#define SPI0_W0_REG SPI_MEM_SPI_W0_REG(DR_REG_SPIMEM_BASE)

#define CMD_READ 0x03
#define CMD_FAST_READ 0x0B
#define CMD_WRITE_ENABLE 0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_READ_STATUS 0x05
#define CMD_WRITE_STATUS 0x01
#define CMD_SECTOR_ERASE 0x20
#define CMD_BLOCK_ERASE_32K 0x52
#define CMD_BLOCK_ERASE_64K 0xD8
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_ID 0x9F

#define SPI_CMD_VALUE(val) ((val) & 0xFF)
#define SPI_ADDR_VALUE(val) ((val) & 0xFFFFFF)
#define SPI_CTRL_USR (1 << 18)
#define SPI_CTRL_USR_ADDR_VALUE (1 << 13)
#define SPI_CTRL_USR_COMMAND (1 << 31)
#define SPI_CTRL_WR_BIT_ORDER (1 << 26)
#define SPI_CTRL_RD_BIT_ORDER (1 << 25)

static inline uint32_t reg_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static inline void reg_write(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static void spi_wait_idle(void) {
    while (reg_read(SPI0_CMD_REG) & SPI_CTRL_USR) {
    }
}

static void spi_write_enable(void) {
    reg_write(SPI0_CMD_REG, SPI_CMD_VALUE(CMD_WRITE_ENABLE) | SPI_CTRL_USR_COMMAND | SPI_CTRL_USR);
    spi_wait_idle();
}

static uint8_t spi_read_status(void) {
    reg_write(SPI0_CMD_REG, SPI_CMD_VALUE(CMD_READ_STATUS) | SPI_CTRL_USR_COMMAND | SPI_CTRL_USR | SPI_CTRL_USR_ADDR_VALUE);
    spi_wait_idle();
    return reg_read(SPI0_W0_REG) & 0xFF;
}

static void spi_wait_write_done(void) {
    while (spi_read_status() & 0x01) {
    }
}

void flash_init(void) {
    reg_write(SPI0_CTRL1_REG, 0);
}

int flash_read(uint32_t addr, void *buf, size_t len) {
    if (addr >= 16 * 1024 * 1024) return -1;
    if (len == 0) return 0;

    uint8_t *dst = (uint8_t *)buf;
    size_t remaining = len;

    while (remaining > 0) {
        size_t chunk = (remaining > 64) ? 64 : remaining;
        (void)chunk;
        reg_write(SPI0_ADDR_REG, SPI_ADDR_VALUE(addr));
        reg_write(SPI0_CMD_REG, SPI_CMD_VALUE(CMD_FAST_READ) | SPI_CTRL_USR_COMMAND | SPI_CTRL_USR | SPI_CTRL_USR_ADDR_VALUE);
        spi_wait_idle();

        uint32_t data = reg_read(SPI0_W0_REG);
        size_t copy_len = (remaining < 4) ? remaining : 4;
        for (size_t i = 0; i < copy_len; i++) {
            *dst++ = (data >> (i * 8)) & 0xFF;
        }

        addr += copy_len;
        remaining -= copy_len;
    }

    return 0;
}

int flash_write(uint32_t addr, const void *buf, size_t len) {
    if (addr >= 16 * 1024 * 1024) return -1;
    if (len == 0) return 0;

    const uint8_t *src = (const uint8_t *)buf;
    size_t remaining = len;

    while (remaining > 0) {
        size_t page_offset = addr % FLASH_PAGE_SIZE;
        size_t chunk = FLASH_PAGE_SIZE - page_offset;
        if (chunk > remaining) chunk = remaining;

        spi_write_enable();

        reg_write(SPI0_ADDR_REG, SPI_ADDR_VALUE(addr));
        reg_write(SPI0_W0_REG, 0);

        for (size_t i = 0; i < chunk; i += 4) {
            uint32_t word = 0;
            size_t copy = (chunk - i >= 4) ? 4 : chunk - i;
            for (size_t j = 0; j < copy; j++) {
                word |= (uint32_t)src[i + j] << (j * 8);
            }
            reg_write(SPI0_W0_REG + i, word);
        }

        reg_write(SPI0_CMD_REG, SPI_CMD_VALUE(CMD_PAGE_PROGRAM) | SPI_CTRL_USR_COMMAND | SPI_CTRL_USR | SPI_CTRL_USR_ADDR_VALUE);
        spi_wait_idle();
        spi_wait_write_done();

        addr += chunk;
        src += chunk;
        remaining -= chunk;
    }

    return 0;
}

int flash_erase(uint32_t addr, size_t len) {
    if (addr >= 16 * 1024 * 1024) return -1;

    size_t remaining = len;
    uint32_t cur = addr;

    while (remaining > 0) {
        if ((cur % FLASH_SECTOR_SIZE == 0) && (remaining >= FLASH_SECTOR_SIZE)) {
            spi_write_enable();
            reg_write(SPI0_ADDR_REG, SPI_ADDR_VALUE(cur));
            reg_write(SPI0_CMD_REG, SPI_CMD_VALUE(CMD_SECTOR_ERASE) | SPI_CTRL_USR_COMMAND | SPI_CTRL_USR | SPI_CTRL_USR_ADDR_VALUE);
            spi_wait_idle();
            spi_wait_write_done();
            cur += FLASH_SECTOR_SIZE;
            remaining -= FLASH_SECTOR_SIZE;
        } else {
            return -1;
        }
    }

    return 0;
}

uint32_t flash_get_size(void) {
    return 16 * 1024 * 1024;
}