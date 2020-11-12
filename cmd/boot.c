/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/boot_mode.h>

#include <bootretry.h>
#include <cli.h>
#include <dm.h>
#include <environment.h>
#include <errno.h>
#include <i2c.h>

#ifdef CONFIG_CMD_GO

/* Allow ports to override the default behavior */
__attribute__((weak))
unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
				 char * const argv[])
{
	return entry (argc, argv);
}

static int do_go(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong	addr, rc;
	int     rcode = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lX ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_go_exec ((void *)addr, argc - 1, argv + 1);
	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

static int do_reboot_brom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef  DELAY_MSP430_TIMEOUT
	struct udevice *bus;
	struct udevice *dev;
	int ret;
	int addr=0x15;
	uchar byte[2]={0x70,0x17};//ten minutes
	ret = uclass_get_device_by_seq(UCLASS_I2C, 2, &bus);
	if (ret) {
		debug("%s: No bus %d\n", __func__, 2);
	}
	ret = i2c_get_chip(bus,0x29,1,&dev);
	if (ret) {
		debug("%s: get cur dev error\n", __func__);
	}
	ret = dm_i2c_write(dev, addr++, byte, 2);
	if (ret) {
		debug("%s:i2c write error\n", __func__);
	}
	udelay(11000);
#endif

	writel(BOOT_BROM_DOWNLOAD, CONFIG_ROCKCHIP_BOOT_MODE_REG);
	do_reset(NULL, 0, 0, NULL);

	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	go, CONFIG_SYS_MAXARGS, 1,	do_go,
	"start application at address 'addr'",
	"addr [arg ...]\n    - start application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

U_BOOT_CMD(
	rbrom, 1, 0,	do_reboot_brom,
	"Perform RESET of the CPU",
	""
);

#endif

U_BOOT_CMD(
        reset, 2, 0,    do_reset,
        "Perform RESET of the CPU",
        ""
);

U_BOOT_CMD(
        reboot, 2, 0,    do_reset,
        "Perform RESET of the CPU, alias of 'reset'",
        ""
);

#ifdef CONFIG_CMD_POWEROFF
U_BOOT_CMD(
	poweroff, 1, 0,	do_poweroff,
	"Perform POWEROFF of the device",
	""
);
#endif
