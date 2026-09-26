#ifndef ESP32S3_REG_H
#define ESP32S3_REG_H

#define DR_REG_UART_BASE 0x60000000
#define DR_REG_UART1_BASE 0x60001000
#define DR_REG_SPI0_BASE 0x60002000
#define DR_REG_SPI1_BASE 0x60003000
#define DR_REG_SPIMEM_BASE 0x60002000

#define SPI_MEM_CMD_REG(base) ((base) + 0x00)
#define SPI_MEM_ADDR_REG(base) ((base) + 0x04)
#define SPI_MEM_CTRL_REG(base) ((base) + 0x08)
#define SPI_MEM_CTRL1_REG(base) ((base) + 0x0C)
#define SPI_MEM_RD_CMD_REG(base) ((base) + 0x10)
#define SPI_MEM_WR_CMD_REG(base) ((base) + 0x14)
#define SPI_MEM_DIN_MODE_REG(base) ((base) + 0x18)
#define SPI_MEM_DIN_NUM_REG(base) ((base) + 0x1C)
#define SPI_MEM_DOUT_MODE_REG(base) ((base) + 0x20)
#define SPI_MEM_DOUT_NUM_REG(base) ((base) + 0x24)
#define SPI_MEM_MISC_REG(base) ((base) + 0x28)
#define SPI_MEM_TX_CRC_REG(base) ((base) + 0x2C)
#define SPI_MEM_CACHE_SCTRL_REG(base) ((base) + 0x30)
#define SPI_MEM_SRAM_USR_CMD_REG(base) ((base) + 0x34)
#define SPI_MEM_SRAM_USR_ADDR_REG(base) ((base) + 0x38)
#define SPI_MEM_DATA_REG(base) ((base) + 0x40)
#define SPI_MEM_FLASH_SU_REG(base) ((base) + 0x44)
#define SPI_MEM_FLASH_WAITI_REG(base) ((base) + 0x48)
#define SPI_MEM_FLASH_SUS_REG(base) ((base) + 0x4C)
#define SPI_MEM_SPI_W0_REG(base) ((base) + 0x58)
#define SPI_MEM_SPI_W1_REG(base) ((base) + 0x5C)
#define SPI_MEM_SPI_W2_REG(base) ((base) + 0x60)
#define SPI_MEM_SPI_W3_REG(base) ((base) + 0x64)
#define SPI_MEM_SPI_W4_REG(base) ((base) + 0x68)
#define SPI_MEM_SPI_W5_REG(base) ((base) + 0x6C)
#define SPI_MEM_SPI_W6_REG(base) ((base) + 0x70)
#define SPI_MEM_SPI_W7_REG(base) ((base) + 0x74)
#define SPI_MEM_SPI_W8_REG(base) ((base) + 0x78)
#define SPI_MEM_SPI_W9_REG(base) ((base) + 0x7C)
#define SPI_MEM_SPI_W10_REG(base) ((base) + 0x80)
#define SPI_MEM_SPI_W11_REG(base) ((base) + 0x84)
#define SPI_MEM_SPI_W12_REG(base) ((base) + 0x88)
#define SPI_MEM_SPI_W13_REG(base) ((base) + 0x8C)
#define SPI_MEM_SPI_W14_REG(base) ((base) + 0x90)
#define SPI_MEM_SPI_W15_REG(base) ((base) + 0x94)
#define DR_REG_GPIO_BASE 0x60004000
#define DR_REG_GPIO_SD_BASE 0x60004F00
#define DR_REG_FE2_BASE 0x60006000
#define DR_REG_FE_BASE 0x60007000
#define DR_REG_RTCCNTL_BASE 0x60008000
#define DR_REG_IO_MUX_BASE 0x60009000
#define DR_REG_RTC_IO_BASE 0x6000E000
#define DR_REG_I2C_EXT_BASE 0x60013000
#define DR_REG_I2C_BASE 0x60013000
#define DR_REG_TIMERGROUP0_BASE 0x6001F000
#define DR_REG_TIMERGROUP1_BASE 0x60020000
#define DR_REG_SYSCON_BASE 0x600C0000
#define DR_REG_APB_CTRL_BASE 0x600C0000
#define DR_REG_CPU_INTR_BASE 0x600C2000
#define DR_REG_ASSIST_DEBUG_BASE 0x600CE000
#define DR_REG_DMA_BASE 0x600D0000
#define DR_REG_AES_BASE 0x6003A000
#define DR_REG_SHA_BASE 0x6003B000
#define DR_REG_RSA_BASE 0x6003C000
#define DR_REG_ECC_BASE 0x6003D000
#define DR_REG_HMAC_BASE 0x6003E000
#define DR_REG_DS_BASE 0x6003F000
#define DR_REG_SPI2_BASE 0x60024000
#define DR_REG_SPI3_BASE 0x60025000
#define DR_REG_SYSTIMER_BASE 0x60023000
#define DR_REG_LP_TIMER_BASE 0x60030000
#define DR_REG_LP_AON_BASE 0x6002E000
#define DR_REG_PCR_BASE 0x60096000
#define DR_REG_GDMA_BASE 0x60098000

#define UART_FIFO_REG(base) ((base) + 0x00)
#define UART_INT_RAW_REG(base) ((base) + 0x04)
#define UART_INT_ST_REG(base) ((base) + 0x08)
#define UART_INT_ENA_REG(base) ((base) + 0x0C)
#define UART_INT_CLR_REG(base) ((base) + 0x10)
#define UART_CLKDIV_REG(base) ((base) + 0x14)
#define UART_RX_FILT_REG(base) ((base) + 0x18)
#define UART_STATUS_REG(base) ((base) + 0x1C)
#define UART_CONF0_REG(base) ((base) + 0x20)
#define UART_CONF1_REG(base) ((base) + 0x24)
#define UART_FLOW_CONF_REG(base) ((base) + 0x28)
#define UART_SLEEP_CONF0_REG(base) ((base) + 0x2C)
#define UART_SLEEP_CONF1_REG(base) ((base) + 0x30)
#define UART_SWFC_CONF0_REG(base) ((base) + 0x34)
#define UART_SWFC_CONF1_REG(base) ((base) + 0x38)
#define UART_TXBRK_CONF_REG(base) ((base) + 0x3C)
#define UART_IDLE_CONF_REG(base) ((base) + 0x40)
#define UART_RS485_CONF_REG(base) ((base) + 0x44)
#define UART_AT_CMD_PRECNT_REG(base) ((base) + 0x48)
#define UART_AT_CMD_POSTCNT_REG(base) ((base) + 0x4C)
#define UART_AT_CMD_GAPTOUT_REG(base) ((base) + 0x50)
#define UART_AT_CMD_CHAR_REG(base) ((base) + 0x54)
#define UART_MEM_CONF_REG(base) ((base) + 0x58)
#define UART_MEM_TX_STATUS_REG(base) ((base) + 0x5C)
#define UART_MEM_RX_STATUS_REG(base) ((base) + 0x60)
#define UART_POS_REG(base) ((base) + 0x64)
#define UART_DATE_REG(base) ((base) + 0x68)
#define UART_REG_UPDATE_REG(base) ((base) + 0x6C)
#define UART_ID_REG(base) ((base) + 0x70)

#define TIMG_T0CONFIG_REG(base) ((base) + 0x00)
#define TIMG_T0LO_REG(base) ((base) + 0x04)
#define TIMG_T0HI_REG(base) ((base) + 0x08)
#define TIMG_T0UPDATE_REG(base) ((base) + 0x0C)
#define TIMG_T0ALARM_REG(base) ((base) + 0x10)
#define TIMG_T0LOADLO_REG(base) ((base) + 0x14)
#define TIMG_T0LOADHI_REG(base) ((base) + 0x18)
#define TIMG_T0LOAD_REG(base) ((base) + 0x1C)
#define TIMG_WDTWPROTECT_REG(base) ((base) + 0x20)
#define TIMG_WDT_CONFIG0_REG(base) ((base) + 0x24)
#define TIMG_WDT_CONFIG1_REG(base) ((base) + 0x28)
#define TIMG_WDT_CONFIG2_REG(base) ((base) + 0x2C)
#define TIMG_WDT_CONFIG3_REG(base) ((base) + 0x30)
#define TIMG_WDT_CONFIG4_REG(base) ((base) + 0x34)
#define TIMG_WDT_FEED_REG(base) ((base) + 0x38)
#define TIMG_WDT_WRTC_REG(base) ((base) + 0x3C)
#define TIMG_RTCCALICFG_REG(base) ((base) + 0x40)
#define TIMG_RTCCALICFG1_REG(base) ((base) + 0x44)
#define TIMG_INT_ENA_TIMERS_REG(base) ((base) + 0x48)
#define TIMG_INT_RAW_TIMERS_REG(base) ((base) + 0x4C)
#define TIMG_INT_ST_TIMERS_REG(base) ((base) + 0x50)
#define TIMG_INT_CLR_TIMERS_REG(base) ((base) + 0x54)
#define TIMG_LACTCHECK_REG(base) ((base) + 0x58)
#define TIMG_NTIMERS_DATE_REG(base) ((base) + 0x5C)
#define TIMG_REGCLK_REG(base) ((base) + 0x60)

#define GPIO_OUT_REG (DR_REG_GPIO_BASE + 0x00)
#define GPIO_OUT_W1TS_REG (DR_REG_GPIO_BASE + 0x04)
#define GPIO_OUT_W1TC_REG (DR_REG_GPIO_BASE + 0x08)
#define GPIO_ENABLE_REG (DR_REG_GPIO_BASE + 0x10)
#define GPIO_ENABLE_W1TS_REG (DR_REG_GPIO_BASE + 0x14)
#define GPIO_ENABLE_W1TC_REG (DR_REG_GPIO_BASE + 0x18)
#define GPIO_IN_REG (DR_REG_GPIO_BASE + 0x1C)
#define GPIO_STATUS_REG (DR_REG_GPIO_BASE + 0x20)
#define GPIO_STATUS_W1TS_REG (DR_REG_GPIO_BASE + 0x24)
#define GPIO_STATUS_W1TC_REG (DR_REG_GPIO_BASE + 0x28)
#define GPIO_PIN0_REG (DR_REG_GPIO_BASE + 0x3C)
#define GPIO_FUNC0_IN_SEL_CFG_REG (DR_REG_GPIO_BASE + 0x154)
#define GPIO_FUNC0_OUT_SEL_CFG_REG (DR_REG_GPIO_BASE + 0x554)

#define SYSCON_CLK_CONF_REG (DR_REG_SYSCON_BASE + 0x00)
#define SYSCON_APB_FREQ_REG (DR_REG_SYSCON_BASE + 0x0C)

#define INTERRUPT_CORE0_CPU_INT_ENABLE_REG (DR_REG_CPU_INTR_BASE + 0x00)
#define INTERRUPT_CORE0_CPU_INT_TYPE_REG (DR_REG_CPU_INTR_BASE + 0x04)
#define INTERRUPT_CORE0_CPU_INT_PRI_0_REG (DR_REG_CPU_INTR_BASE + 0x10)
#define INTERRUPT_CORE0_CPU_INT_PRI_1_REG (DR_REG_CPU_INTR_BASE + 0x14)
#define INTERRUPT_CORE0_CPU_INT_PRI_2_REG (DR_REG_CPU_INTR_BASE + 0x18)
#define INTERRUPT_CORE0_CPU_INT_PRI_3_REG (DR_REG_CPU_INTR_BASE + 0x1C)
#define INTERRUPT_CORE0_CPU_INT_PRI_4_REG (DR_REG_CPU_INTR_BASE + 0x20)
#define INTERRUPT_CORE0_CPU_INT_PRI_5_REG (DR_REG_CPU_INTR_BASE + 0x24)
#define INTERRUPT_CORE0_CPU_INT_PRI_6_REG (DR_REG_CPU_INTR_BASE + 0x28)
#define INTERRUPT_CORE0_CPU_INT_PRI_7_REG (DR_REG_CPU_INTR_BASE + 0x2C)
#define INTERRUPT_CORE0_CPU_INT_CLEAR_REG (DR_REG_CPU_INTR_BASE + 0x30)
#define INTERRUPT_CORE0_CPU_INT_ENA_W1TS_REG (DR_REG_CPU_INTR_BASE + 0x34)
#define INTERRUPT_CORE0_CPU_INT_ENA_W1TC_REG (DR_REG_CPU_INTR_BASE + 0x38)

#define INTERRUPT_CORE1_CPU_INT_ENABLE_REG (DR_REG_CPU_INTR_BASE + 0x100)
#define INTERRUPT_CORE1_CPU_INT_TYPE_REG (DR_REG_CPU_INTR_BASE + 0x104)
#define INTERRUPT_CORE1_CPU_INT_PRI_0_REG (DR_REG_CPU_INTR_BASE + 0x110)
#define INTERRUPT_CORE1_CPU_INT_PRI_1_REG (DR_REG_CPU_INTR_BASE + 0x114)
#define INTERRUPT_CORE1_CPU_INT_PRI_2_REG (DR_REG_CPU_INTR_BASE + 0x118)
#define INTERRUPT_CORE1_CPU_INT_PRI_3_REG (DR_REG_CPU_INTR_BASE + 0x11C)
#define INTERRUPT_CORE1_CPU_INT_PRI_4_REG (DR_REG_CPU_INTR_BASE + 0x120)
#define INTERRUPT_CORE1_CPU_INT_PRI_5_REG (DR_REG_CPU_INTR_BASE + 0x124)
#define INTERRUPT_CORE1_CPU_INT_PRI_6_REG (DR_REG_CPU_INTR_BASE + 0x128)
#define INTERRUPT_CORE1_CPU_INT_PRI_7_REG (DR_REG_CPU_INTR_BASE + 0x12C)
#define INTERRUPT_CORE1_CPU_INT_CLEAR_REG (DR_REG_CPU_INTR_BASE + 0x130)
#define INTERRUPT_CORE1_CPU_INT_ENA_W1TS_REG (DR_REG_CPU_INTR_BASE + 0x134)
#define INTERRUPT_CORE1_CPU_INT_ENA_W1TC_REG (DR_REG_CPU_INTR_BASE + 0x138)

#define INTERRUPT_SOURCE_PRO_CPU_BASE 0
#define INTERRUPT_SOURCE_APP_CPU_BASE 64

#define PSRAM_CACHE_SRAM_BASE 0x3F800000
#define PSRAM_CACHE_SRAM_SIZE 0x800000

#define FLASH_MMU_TABLE_BASE 0x3FF00000

#endif