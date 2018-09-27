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
	struct rk3288_grf * const grf = (void *)GRF_BASE;

#ifdef CONFIG_SWITCH_DEBUG_PORT_TO_UART
	gpio_request(DEBUG_SWITCH_GPIO,"debug switch");
	gpio_direction_input(DEBUG_SWITCH_GPIO);
	if (gpio_get_value(DEBUG_SWITCH_GPIO) == DEBUG_SWITCH_GPIO_ACTIVE) {
		gd->flags |= GD_FLG_DISABLE_CONSOLE;
		//reconfig iomux to defalt gpio
		rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_GPIO << GPIO7C7_SHIFT |
		     GPIO7C6_GPIO << GPIO7C6_SHIFT);
	} else {
		rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
		     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
	}
#endif

	/* pull disable to fix 83867 phy address*/
#ifdef CONFIG_DP83867_PHY_ID
	rk_clrsetreg(&grf->gpio1_p[2][3], (3 << 12) | (3 << 4), 0);
#endif

#ifdef CONFIG_RESET_PMIC_GPIO
	if(BOOT_MODE_NORMAL == rockchip_get_boot_mode())
	{
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,1);
		mdelay(5);
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,0);
		while(1);
	}
#endif

	return 0;
}

#ifdef CONFIG_MAC_IN_SPI
static int board_info_in_spi(void)
{
    struct spi_flash *flash;
	uchar enetaddr[6];
	u32 valid;

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!flash)
		return -1;
	if(spi_flash_read(flash, CONFIG_SPI_MAC_OFFSET, 6, enetaddr)==0) {
		spi_flash_free(flash);
		valid = is_valid_ethaddr(enetaddr);

		if (valid)
			eth_env_set_enetaddr("ethaddr", enetaddr);
		else
			puts("Skipped ethaddr assignment due to invalid,using default!\n");
	}
	
	return 0;
}
#endif 

int rk3288_board_late_init(void)
{
#ifdef CONFIG_MAC_IN_SPI
	board_info_in_spi();
#endif

#ifdef CONFIG_SWITCH_DEBUG_PORT_TO_UART
	gpio_request(DEBUG_SWITCH_GPIO,"debug switch");
	gpio_direction_input(DEBUG_SWITCH_GPIO);
	if (gpio_get_value(DEBUG_SWITCH_GPIO) == DEBUG_SWITCH_GPIO_ACTIVE)
		env_set("silent_linux","yes");
	else
		env_set("silent_linux",NULL);
#endif

	return 0;
}
