/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <spi.h>
#include <spi_flash.h>
#include <version.h>
#include <boot_rkimg.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/bootrom.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/pmu_rk3288.h>
#include <asm/arch/qos_rk3288.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/boot_mode.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <dt-bindings/clock/rk3288-cru.h>

#define GRF_BASE	0xff770000

#ifdef CONFIG_DISPLAY_BOARDINFO
/**
 * Print board information
 */
int checkboard(void)
{
	unsigned int ver=0;

#ifdef CONFIG_DISPLAY_BOARD_ID
	gpio_request(HW_BOARD_ID0,"hw board id0");
	gpio_request(HW_BOARD_ID1,"hw board id1");
	gpio_request(HW_BOARD_ID2,"hw board id2");
	
	gpio_direction_input(HW_BOARD_ID2);
	gpio_direction_input(HW_BOARD_ID1);
	gpio_direction_input(HW_BOARD_ID0);
	ver = gpio_get_value(HW_BOARD_ID2);
	ver = (ver<<1) | gpio_get_value(HW_BOARD_ID1);
	ver = (ver<<1) | gpio_get_value(HW_BOARD_ID0);
#endif
	printf("Board:Advantech %s Board,HW version:%d\n",CONFIG_SYS_CONFIG_NAME,ver);

	return 0;
}
#endif

int adv_board_early_init(void)
{
#if defined DEBUG2UART_GPIO || defined ADV_DP83867_PHY_ID
	struct rk3288_grf * const grf = (void *)GRF_BASE;
#endif
#ifdef RESET_PMIC_GPIO
	uint32_t reg_boot_mode;
#endif

#ifdef DEBUG2UART_GPIO
	gpio_request(DEBUG2UART_GPIO,"DEBUG2UART_GPIO");
	gpio_direction_input(DEBUG2UART_GPIO);
	if (gpio_get_value(DEBUG2UART_GPIO) == DEBUG2UART_GPIO_ACTIVE) {
		gd->flags |= GD_FLG_DISABLE_CONSOLE;
		//reconfig iomux to defalt gpio
		rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_GPIO << GPIO7C7_SHIFT |
		     GPIO7C6_GPIO << GPIO7C6_SHIFT);
	} else {
		gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
		rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
		     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
	}
	gpio_free(DEBUG2UART_GPIO);
#endif

	/* pull disable to fix 83867 phy address*/
#ifdef ADV_DP83867_PHY_ID
	rk_clrsetreg(&grf->gpio1_p[2][3], (3 << 12) | (3 << 4), 0);
#endif

#ifdef RESET_PMIC_GPIO
	reg_boot_mode = readl((void *)CONFIG_ROCKCHIP_BOOT_MODE_REG);
	if(BOOT_NORMAL == reg_boot_mode)
	{
		gpio_request(RESET_PMIC_GPIO,"reset pmic");
		gpio_direction_output(RESET_PMIC_GPIO,0);
		mdelay(5);
		gpio_direction_output(RESET_PMIC_GPIO,1);
		mdelay(500);
	}
#endif

	return 0;
}

#ifdef ADV_MAC_IN_SPI
static int board_info_in_spi(void)
{
    struct spi_flash *flash;
	uchar enetaddr[50];
	u32 valid;
	int sn_len,time_len,info_len;

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!flash)
		return -1;
	if(spi_flash_read(flash, ADV_SPI_MAC_OFFSET, 50, enetaddr)==0) {
		spi_flash_free(flash);
		valid = is_valid_ethaddr(enetaddr);

		if (valid)
			eth_env_set_enetaddr("ethaddr", enetaddr);
		else
			puts("Skipped ethaddr assignment due to invalid,using default!\n");

		sn_len = enetaddr[12];
		time_len = enetaddr[13+sn_len];
		info_len = enetaddr[14+sn_len+time_len];
		if(sn_len && (sn_len != 0xff)) {
			enetaddr[13+sn_len] = '\0';
			env_set("boardsn", (void *)(&enetaddr[13]));
			if(time_len && (time_len != 0xff)) {
				enetaddr[14+sn_len+time_len] = '\0';
				env_set("androidboot.factorytime", (void *)(&enetaddr[14+sn_len]));
				if(info_len && (info_len != 0xff)) {
					enetaddr[15+sn_len+time_len+info_len] = '\0';
					env_set("androidboot.serialno", (void *)(&enetaddr[15+sn_len+time_len]));
				}else
					env_set("androidboot.serialno", NULL);
			}else
				env_set("androidboot.factorytime", NULL);
		} else {
			env_set("boardsn", NULL);
			env_set("androidboot.factorytime", NULL);
			env_set("androidboot.serialno", NULL);
		}
	}
	
	return 0;
}
#endif 

static void board_version_config(void)
{
	unsigned char version[10];

	memset((void *)version,0,sizeof(version));
	snprintf((void *)version,sizeof(version),"%s",strrchr(PLAIN_VERSION,'V'));
	if(version[0]=='V')
		env_set("swversion",(void *)version);
	else
		env_set("swversion",NULL);
}

int rk3288_board_late_init(void)
{
#ifdef ADV_MAC_IN_SPI
	board_info_in_spi();
#endif

	board_version_config();

#ifdef DEBUG2UART_GPIO
	gpio_request(DEBUG2UART_GPIO,"DEBUG2UART_GPIO");
	gpio_direction_input(DEBUG2UART_GPIO);
	if (gpio_get_value(DEBUG2UART_GPIO) == DEBUG2UART_GPIO_ACTIVE)
	{
		env_set("switch_debug","yes");
	}
	else
		env_set("switch_debug",NULL);
	gpio_free(DEBUG2UART_GPIO);
#endif

	return 0;
}
