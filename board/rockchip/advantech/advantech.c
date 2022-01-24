/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <ram.h>
#include <mmc.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/setup.h>
#include <asm/arch/periph.h>
#include <power/regulator.h>
#include <u-boot/sha256.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <spl.h>
#include <boot_rkimg.h>
#include <version.h>
#include <irq-platform.h>
#include <asm/gpio.h>
#include <asm/arch/grf_rk3399.h>
#include <asm/arch/cru_rk3399.h>
#include <asm/arch/gpio.h>
#include <boot_rkimg.h>

DECLARE_GLOBAL_DATA_PTR;

#define RK3399_CPUID_OFF  0x7
#define RK3399_CPUID_LEN  0x10
#define GRF_BASE	0xff770000
#define PMUGRF_BASE	0xff320000

int board_early_init_f(void)
{
	struct rk3399_grf_regs * const grf = (void *)GRF_BASE;
	struct rk3399_pmugrf_regs *pmugrf = (void *)PMUGRF_BASE;
	struct rockchip_gpio_regs *gpio = NULL;
#ifdef ADV_GRF_IO_VSEL
	grf->io_vsel = ADV_GRF_IO_VSEL;
#endif

#ifdef ENABLE_PCIERST_GPIO
	gpio = (void *)GPIO2_PHYS;
	gpio->swport_ddr |= (1 << 28);//set direction output
	gpio->swport_dr &= ~(1 << 28);//output low
#endif

#ifdef DEBUG2UART_GPIO
	//GPIO1A1
	gpio = (void *)GPIO1_PHYS;
	pmugrf->gpio1a_iomux = 0x3 << 18;//gpio
	gpio->swport_ddr &= ~0x2;//input
	if ((gpio->ext_port&0x2)>>1 == DEBUG2UART_GPIO_ACTIVE) {
		gd->flags |= GD_FLG_DISABLE_CONSOLE;
		//reconfig iomux to defalt gpio
		grf->gpio4c_iomux = 0xf << 22;
	} else {
		gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
		grf->gpio4c_iomux = (0xf << 22) | (0x5 << 6);
	}
#endif

#ifdef DISABLE_MSP430
#if defined(CONFIG_PLAT_ROM5780A3_2G) || defined(CONFIG_PLAT_ROM5780B1_2G)
	gpio = (void *)GPIO0_PHYS;
	pmugrf->gpio0a_iomux = 0x3 << 28; //gpio_a6,wdt_en
	pmugrf->gpio0b_iomux = 0x3 << 18;//gpio_b1,wdt_ping
	gpio->swport_ddr |= (1 << 9 | 1 << 6);//set direction output

	//wdt_en & wdt_png disable
	gpio->swport_dr &= ~((1 << 9) | (1 << 6));
#else
	gpio = (void *)GPIO0_PHYS;
	pmugrf->gpio0a_iomux = 0x3 << 28; //gpio_a6,wdt_en
	pmugrf->gpio0a_iomux = 0x3 << 18;//gpio_a1,wdt_ping
	gpio->swport_ddr |= (1 << 1 | 1 << 6);//set direction output

	//wdt_en & wdt_png disable
	gpio->swport_dr &= ~((1 << 1) | (1 << 6));
#endif
#endif

#ifdef PULLDOWN_MIPI2LVDS_RESET
	gpio = (void *)GPIO1_PHYS;
	pmugrf->gpio1d_iomux = 0x3 << 16; //gpio1_d0
	gpio->swport_ddr |= (1 << 24);
	gpio->swport_dr &= ~(1 << 24);
#endif

#ifdef CONFIG_RESET_PMIC_GPIO
	//if(SYS_LOADER_REBOOT_FLAG == IReadLoaderFlag())
	{
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,1);
		mdelay(5);
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,0);
		mdelay(500);
	}
#endif

	return 0;
}

int rk_board_init(void)
{
	struct udevice *pinctrl;
	int ret;

	/*
	 * The PWM does not have decicated interrupt number in dts and can
	 * not get periph_id by pinctrl framework, so let's init them here.
	 * The PWM2 and PWM3 are for pwm regulators.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		printf("%s: Cannot find pinctrl device\n", __func__);
		goto out;
	}

	/* Enable pwm0 for panel backlight */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM0);
	if (ret) {
		printf("%s PWM0 pinctrl init fail! (ret=%d)\n", __func__, ret);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM1);
	if (ret) {
		printf("%s PWM1 pinctrl init fail!\n", __func__);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM2);
	if (ret) {
		printf("%s PWM2 pinctrl init fail!\n", __func__);
		goto out;
	}

out:
	return 0;
}

static void setup_serial(void)
{
#if CONFIG_IS_ENABLED(ROCKCHIP_EFUSE)
	struct udevice *dev;
	int ret, i;
	u8 cpuid[RK3399_CPUID_LEN];
	u8 low[RK3399_CPUID_LEN/2], high[RK3399_CPUID_LEN/2];
	char cpuid_str[RK3399_CPUID_LEN * 2 + 1];
	u64 serialno;
	char serialno_str[16];

	/* retrieve the device */
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(rockchip_efuse), &dev);
	if (ret) {
		debug("%s: could not find efuse device\n", __func__);
		return;
	}

	/* read the cpu_id range from the efuses */
	ret = misc_read(dev, RK3399_CPUID_OFF, &cpuid, sizeof(cpuid));
	if (ret) {
		debug("%s: reading cpuid from the efuses failed\n",
		      __func__);
		return;
	}

	memset(cpuid_str, 0, sizeof(cpuid_str));
	for (i = 0; i < 16; i++)
		sprintf(&cpuid_str[i * 2], "%02x", cpuid[i]);

	debug("cpuid: %s\n", cpuid_str);

	/*
	 * Mix the cpuid bytes using the same rules as in
	 *   ${linux}/drivers/soc/rockchip/rockchip-cpuinfo.c
	 */
	for (i = 0; i < 8; i++) {
		low[i] = cpuid[1 + (i << 1)];
		high[i] = cpuid[i << 1];
	}

	serialno = crc32_no_comp(0, low, 8);
	serialno |= (u64)crc32_no_comp(serialno, high, 8) << 32;
	snprintf(serialno_str, sizeof(serialno_str), "%llx", serialno);

	env_set("cpuid#", cpuid_str);
	env_set("serial#", serialno_str);
#endif

	return;
}

int rk_board_late_init(void)
{
	struct blk_desc *dev_desc;
	const char *part_name;
	disk_partition_t part_info;
	unsigned char *buf;
	u32 blk_cnt;
	int ret;
	unsigned char version[10];
	u32 valid;
	int sn_len,time_len,info_len;
	unsigned char ori_hwpart;

#ifdef DEBUG2UART_GPIO
	gpio_request(DEBUG2UART_GPIO,"DEBUG2UART_GPIO");
	gpio_direction_input(DEBUG2UART_GPIO);
	if (gpio_get_value(DEBUG2UART_GPIO) == DEBUG2UART_GPIO_ACTIVE && rockchip_get_boot_mode() != BOOT_MODE_RECOVERY)
	{
		gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
		env_set("switch_debug","yes");
	}
	else
		env_set("switch_debug",NULL);
	gpio_free(DEBUG2UART_GPIO);
#endif

#ifdef ENABLE_WDT_GPIO
	gpio_request(ENABLE_WDT_GPIO,"ENABLE_WDT_GPIO");
	gpio_direction_output(ENABLE_WDT_GPIO,1);
	gpio_free(ENABLE_WDT_GPIO);
#endif

#ifdef DISABLE_EDP_VDD_GPIO
	gpio_request(DISABLE_EDP_VDD_GPIO,"DISABLE_EDP_VDD_GPIO");
	gpio_direction_output(DISABLE_EDP_VDD_GPIO,0);
	gpio_free(DISABLE_EDP_VDD_GPIO);
#endif

#ifdef DISABLE_EDP_BKLT_GPIO
	gpio_request(DISABLE_EDP_BKLT_GPIO,"DISABLE_EDP_BKLT_GPIO");
	gpio_direction_output(DISABLE_EDP_BKLT_GPIO,0);
	gpio_free(DISABLE_EDP_BKLT_GPIO);
#endif

#ifdef DISABLE_EDP_PWM_GPIO
	gpio_request(DISABLE_EDP_PWM_GPIO,"DISABLE_EDP_PWM_GPIO");
	gpio_direction_output(DISABLE_EDP_PWM_GPIO,0);
	gpio_free(DISABLE_EDP_PWM_GPIO);
#endif

	memset(version,0,sizeof(version));
	snprintf((char *)version,sizeof(version),"%s",strrchr(PLAIN_VERSION,'V'));
	if(version[0]=='V')
		env_set("swversion",(const char *)version);
	else
		env_set("swversion",NULL);

	/* Get partition info */
	dev_desc = rockchip_get_bootdev();
	if (!dev_desc) {
		printf("%s: dev_desc is NULL!\n", __func__);
		return -ENODEV;
	}

	blk_cnt = DIV_ROUND_UP(512, dev_desc->blksz);
	buf = memalign(ARCH_DMA_MINALIGN, dev_desc->blksz*blk_cnt);
	if (!buf) {
		printf("%s: out of memory!\n", __func__);
		return 0;
	}

	ori_hwpart = dev_desc->hwpart;
	ret = blk_select_hwpart_devnum(IF_TYPE_MMC, dev_desc->devnum, MMC_NUM_BOOT_PARTITION);
	if (ret){
		printf("failed to select boot_part\n");
		goto out;
	}

	ret = blk_dread(dev_desc, 0, blk_cnt, buf);
	if (ret != blk_cnt) {
		printf("%s: failed to read boot_part hdr!\n", __func__);
		blk_select_hwpart_devnum(IF_TYPE_MMC, dev_desc->devnum, ori_hwpart);
		goto out;
	}

	ret = blk_select_hwpart_devnum(IF_TYPE_MMC, dev_desc->devnum, ori_hwpart);
	if (ret){
		printf("failed to select user data part\n");
		goto out;
	}

	valid = is_valid_ethaddr(buf);
	if (valid)
		eth_env_set_enetaddr("ethaddr", buf);
	else {
		part_name = BOARD_INFO_NAME;
		ret = part_get_info_by_name(dev_desc, part_name, &part_info);
		if (ret < 0) {
			printf("%s: failed to get %s part info, ret=%d\n",
				__func__, part_name, ret);
			goto out;
		}

		ret = blk_dread(dev_desc, part_info.start, blk_cnt, buf);
		if (ret != blk_cnt) {
			printf("%s: failed to read boot_part hdr!\n", __func__);
			goto out;
		}

		valid = is_valid_ethaddr(buf);
		if (valid)
			eth_env_set_enetaddr("ethaddr", buf);
		else {
			puts("Skipped eth0addr assignment due to invalid,using default!\n");
			goto out;
		}
	}

	valid = is_valid_ethaddr(&buf[6]);
	if (valid)
		eth_env_set_enetaddr("eth1addr", &buf[6]);
	else
		puts("Skipped eth1addr assignment due to invalid,using default!\n");

	sn_len = buf[12];
	time_len = buf[13+sn_len];
	info_len = buf[14+sn_len+time_len];
	if(sn_len && (sn_len <= 20)) {
		buf[13+sn_len] = '\0';
		env_set("boardsn", (const char *)(buf+13));
		env_set("androidboot.serialno", (const char *)(buf+13));
		if(time_len && (time_len == 8)) {
			buf[14+sn_len+time_len] = '\0';
			env_set("androidboot.factorytime", (const char *)(buf+14+sn_len));
			if(info_len && (info_len <= 0x40)) {
				buf[15+sn_len+time_len+info_len] = '\0';
				env_set("hwversion", (const char *)(buf+15+sn_len+time_len));
			}else
				env_set("hwversion", NULL);
		}else {
			env_set("androidboot.factorytime", NULL);
			env_set("hwversion", NULL);
		}
	} else {
		env_set("boardsn", NULL);
		env_set("androidboot.factorytime", NULL);
		env_set("androidboot.serialno", NULL);
		env_set("hwversion", NULL);
	}

out:
	free(buf);

	return 0;
}

int misc_init_r(void)
{
	setup_serial();

	return 0;
}

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	u64 serial = 0;

	serial_string = env_get("serial#");

	if (serial_string)
		serial = simple_strtoull(serial_string, NULL, 16);

	serialnr->high = (u32)(serial >> 32);
	serialnr->low = (u32)(serial & 0xffffffff);
}
#endif

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xfe800000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.usb2_phyif_utmi_width = 16,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif
