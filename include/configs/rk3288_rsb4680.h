/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __RK3288_RSB4680_CONFIG_H
#define __RK3288_RSB4680_CONFIG_H

#include <configs/rk3288_common.h>

#define CONFIG_SYS_MMC_ENV_DEV 0

#define ROCKCHIP_DEVICE_SETTINGS \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

#define CONFIG_BOARD_EARLY_INIT_F

/* mod it to enable console commands.	*/
#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY		0

/* switch debug port to normal uart */
#define CONFIG_DISABLE_CONSOLE
#define DEBUG2UART_GPIO	(2*32+0-8)//(GPIO_BANK2 | GPIO_A0)
#define DEBUG2UART_GPIO_ACTIVE 1

/* reset pmic to reset all system */
#define RESET_PMIC_GPIO	(0*32+10) /* (GPIO_BANK0 | GPIO_B2) */

/* HW board ID */
/*#define CONFIG_DISPLAY_BOARD_ID*/
#ifdef CONFIG_DISPLAY_BOARD_ID
#define HW_BOARD_ID0	(0*32+9)//(GPIO_BANK0 | GPIO_B1)
#define HW_BOARD_ID1	(0*32+8)//(GPIO_BANK0 | GPIO_B0)
#define HW_BOARD_ID2	(0*32+7)//(GPIO_BANK0 | GPIO_A7)
#endif

/* mac in spi*/
#define CONFIG_ENV_OVERWRITE
/*#define CONFIG_MAC_IN_SPI
#define CONFIG_SPI_MAC_OFFSET (896*1024)
#define CONFIG_DP83867_PHY_ID*/
#define FDT_SEQ_MACADDR_FROM_ENV

/*enc in SPI flash*/
#ifndef CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH
#endif
#define CONFIG_SPI_FLASH_WINBOND
#ifndef CONFIG_SPI_FLASH_USE_4K_SECTORS
#define CONFIG_SPI_FLASH_USE_4K_SECTORS
#endif
#ifndef CONFIG_ROCKCHIP_SPI
#define CONFIG_ROCKCHIP_SPI
#endif
#ifdef CONFIG_SF_DEFAULT_BUS
#undef CONFIG_SF_DEFAULT_BUS
#endif
#define CONFIG_SF_DEFAULT_BUS   2
#ifdef CONFIG_SF_DEFAULT_CS
#undef CONFIG_SF_DEFAULT_CS
#endif
#define CONFIG_SF_DEFAULT_CS 0
#ifdef CONFIG_SF_DEFAULT_SPEED
#undef CONFIG_SF_DEFAULT_SPEED
#endif
#define CONFIG_SF_DEFAULT_SPEED 24000000
#ifdef CONFIG_SF_DEFAULT_MODE
#undef CONFIG_SF_DEFAULT_MODE
#endif
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_0
#define CONFIG_ENV_SECT_SIZE	0x10000
#ifdef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SIZE
#endif
#define CONFIG_ENV_SIZE 0x8000
#ifdef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_OFFSET
#endif
#define CONFIG_ENV_OFFSET 0xc0000

#endif
