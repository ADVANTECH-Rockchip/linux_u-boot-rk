/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 Rockchip Electronics Co. Ltd.
 * Author: Finley Xiao <finley.xiao@rock-chips.com>
 */

#ifndef _ASM_ARCH_CRU_RV1126_H
#define _ASM_ARCH_CRU_RV1126_H

#include <common.h>

#define MHz		1000000
#define KHz		1000
#define OSC_HZ		(24 * MHz)

#define APLL_HZ		(1008 * MHz)
#define GPLL_HZ		(1188 * MHz)
#define CPLL_HZ		(500 * MHz)
#define HPLL_HZ		(1400 * MHz)
#define PCLK_PDPMU_HZ	(100 * MHz)
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_KERNEL_BOOT)
#define ACLK_PDBUS_HZ	(396 * MHz)
#else
#define ACLK_PDBUS_HZ	(500 * MHz)
#endif
#define HCLK_PDBUS_HZ	(200 * MHz)
#define PCLK_PDBUS_HZ	(100 * MHz)
#define ACLK_PDPHP_HZ	(300 * MHz)
#define HCLK_PDPHP_HZ	(200 * MHz)
#define HCLK_PDCORE_HZ	(200 * MHz)
#define HCLK_PDAUDIO_HZ	(150 * MHz)
#define CLK_OSC0_DIV_HZ	(32768)
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_KERNEL_BOOT)
#define ACLK_PDVI_HZ	(297 * MHz)
#define CLK_ISP_HZ	(297 * MHz)
#define ACLK_PDISPP_HZ	(297 * MHz)
#define CLK_ISPP_HZ	(237 * MHz)
#define ACLK_VOP_HZ	(300 * MHz)
#define DCLK_VOP_HZ	(65 * MHz)
#endif

/* RV1126 pll id */
enum rv1126_pll_id {
	APLL,
	DPLL,
	CPLL,
	HPLL,
	GPLL,
	PLL_COUNT,
};

struct rv1126_clk_info {
	unsigned long id;
	char *name;
	bool is_cru;
};

/* Private data for the clock driver - used by rockchip_get_cru() */
struct rv1126_pmuclk_priv {
	struct rv1126_pmucru *pmucru;
	ulong gpll_hz;
};

struct rv1126_clk_priv {
	struct rv1126_cru *cru;
	struct rv1126_grf *grf;
	ulong gpll_hz;
	ulong cpll_hz;
	ulong hpll_hz;
	ulong armclk_hz;
	ulong armclk_enter_hz;
	ulong armclk_init_hz;
	bool sync_kernel;
	bool set_armclk_rate;
};

struct rv1126_pll {
	unsigned int con0;
	unsigned int con1;
	unsigned int con2;
	unsigned int con3;
	unsigned int con4;
	unsigned int con5;
	unsigned int con6;
	unsigned int reserved0[1];
};

struct rv1126_pmucru {
	unsigned int pmu_mode;
	unsigned int reserved1[3];
	struct rv1126_pll pll;
	unsigned int offsetcal_status;
	unsigned int reserved2[51];
	unsigned int pmu_clksel_con[14];
	unsigned int reserved3[18];
	unsigned int pmu_clkgate_con[3];
	unsigned int reserved4[29];
	unsigned int pmu_softrst_con[2];
	unsigned int reserved5[14];
	unsigned int pmu_autocs_con[2];
};

check_member(rv1126_pmucru, pmu_autocs_con[1], 0x244);

struct rv1126_cru {
	struct rv1126_pll pll[4];
	unsigned int offsetcal_status[4];
	unsigned int mode;
	unsigned int reserved1[27];
	unsigned int clksel_con[78];
	unsigned int reserved2[18];
	unsigned int clkgate_con[25];
	unsigned int reserved3[7];
	unsigned int softrst_con[15];
	unsigned int reserved4[17];
	unsigned int ssgtbl[32];
	unsigned int glb_cnt_th;
	unsigned int glb_rst_st;
	unsigned int glb_srst_fst;
	unsigned int glb_srst_snd;
	unsigned int glb_rst_con;
	unsigned int reserved5[11];
	unsigned int sdmmc_con[2];
	unsigned int sdio_con[2];
	unsigned int emmc_con[2];
	unsigned int reserved6[2];
	unsigned int gmac_con;
	unsigned int misc[2];
	unsigned int reserved7[45];
	unsigned int autocs_con[26];
};

check_member(rv1126_cru, autocs_con[25], 0x584);

struct pll_rate_table {
	unsigned long rate;
	unsigned int fbdiv;
	unsigned int postdiv1;
	unsigned int refdiv;
	unsigned int postdiv2;
	unsigned int dsmpd;
	unsigned int frac;
};

struct cpu_rate_table {
	unsigned long rate;
	unsigned int aclk_div;
	unsigned int pclk_div;
};

#define RV1126_PMU_MODE			0x0
#define RV1126_PMU_PLL_CON(x)		((x) * 0x4 + 0x10)
#define RV1126_PLL_CON(x)		((x) * 0x4)
#define RV1126_MODE_CON			0x90

enum {
	/* CRU_PMU_CLK_SEL0_CON */
	RTC32K_SEL_SHIFT	= 7,
	RTC32K_SEL_MASK		= 0x3 << RTC32K_SEL_SHIFT,
	RTC32K_SEL_PMUPVTM	= 0,
	RTC32K_SEL_OSC1_32K,
	RTC32K_SEL_OSC0_DIV32K,

	/* CRU_PMU_CLK_SEL1_CON */
	PCLK_PDPMU_DIV_SHIFT	= 0,
	PCLK_PDPMU_DIV_MASK	= 0x1f,

	/* CRU_PMU_CLK_SEL2_CON */
	CLK_I2C0_DIV_SHIFT	= 0,
	CLK_I2C0_DIV_MASK	= 0x7f,

	/* CRU_PMU_CLK_SEL3_CON */
	CLK_I2C2_DIV_SHIFT	= 0,
	CLK_I2C2_DIV_MASK	= 0x7f,

	/* CRU_PMU_CLK_SEL6_CON */
	CLK_PWM1_SEL_SHIFT	= 15,
	CLK_PWM1_SEL_MASK	= 1 << CLK_PWM1_SEL_SHIFT,
	CLK_PWM1_SEL_XIN24M	= 0,
	CLK_PWM1_SEL_GPLL,
	CLK_PWM1_DIV_SHIFT	= 8,
	CLK_PWM1_DIV_MASK	= 0x7f << CLK_PWM1_DIV_SHIFT,
	CLK_PWM0_SEL_SHIFT	= 7,
	CLK_PWM0_SEL_MASK	= 1 << CLK_PWM0_SEL_SHIFT,
	CLK_PWM0_SEL_XIN24M	= 0,
	CLK_PWM0_SEL_GPLL,
	CLK_PWM0_DIV_SHIFT	= 0,
	CLK_PWM0_DIV_MASK	= 0x7f,

	/* CRU_PMU_CLK_SEL9_CON */
	CLK_SPI0_SEL_SHIFT	= 7,
	CLK_SPI0_SEL_MASK	= 1 << CLK_SPI0_SEL_SHIFT,
	CLK_SPI0_SEL_GPLL	= 0,
	CLK_SPI0_SEL_XIN24M,
	CLK_SPI0_DIV_SHIFT	= 0,
	CLK_SPI0_DIV_MASK	= 0x7f,

	/* CRU_PMU_CLK_SEL13_CON */
	CLK_RTC32K_FRAC_NUMERATOR_SHIFT		= 16,
	CLK_RTC32K_FRAC_NUMERATOR_MASK		= 0xffff << 16,
	CLK_RTC32K_FRAC_DENOMINATOR_SHIFT	= 0,
	CLK_RTC32K_FRAC_DENOMINATOR_MASK	= 0xffff,

	/* CRU_CLK_SEL0_CON */
	CORE_HCLK_DIV_SHIFT	= 8,
	CORE_HCLK_DIV_MASK	= 0x1f << CORE_HCLK_DIV_SHIFT,

	/* CRU_CLK_SEL1_CON */
	CORE_ACLK_DIV_SHIFT	= 4,
	CORE_ACLK_DIV_MASK	= 0xf << CORE_ACLK_DIV_SHIFT,
	CORE_DBG_DIV_SHIFT	= 0,
	CORE_DBG_DIV_MASK	= 0x7,

	/* CRU_CLK_SEL2_CON */
	HCLK_PDBUS_SEL_SHIFT	= 15,
	HCLK_PDBUS_SEL_MASK	= 1 << HCLK_PDBUS_SEL_SHIFT,
	HCLK_PDBUS_SEL_GPLL	= 0,
	HCLK_PDBUS_SEL_CPLL,
	HCLK_PDBUS_DIV_SHIFT	= 8,
	HCLK_PDBUS_DIV_MASK	= 0x1f << HCLK_PDBUS_DIV_SHIFT,
	ACLK_PDBUS_SEL_SHIFT	= 6,
	ACLK_PDBUS_SEL_MASK	= 0x3 << ACLK_PDBUS_SEL_SHIFT,
	ACLK_PDBUS_SEL_GPLL	= 0,
	ACLK_PDBUS_SEL_CPLL,
	ACLK_PDBUS_SEL_DPLL,
	ACLK_PDBUS_DIV_SHIFT	= 0,
	ACLK_PDBUS_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL3_CON */
	CLK_SCR1_SEL_SHIFT	= 15,
	CLK_SCR1_SEL_MASK	= 1 << CLK_SCR1_SEL_SHIFT,
	CLK_SCR1_SEL_GPLL	= 0,
	CLK_SCR1_SEL_CPLL,
	CLK_SCR1_DIV_SHIFT	= 8,
	CLK_SCR1_DIV_MASK	= 0x1f << CLK_SCR1_DIV_SHIFT,
	PCLK_PDBUS_SEL_SHIFT	= 7,
	PCLK_PDBUS_SEL_MASK	= 1 << PCLK_PDBUS_SEL_SHIFT,
	PCLK_PDBUS_SEL_GPLL	= 0,
	PCLK_PDBUS_SEL_CPLL,
	PCLK_PDBUS_DIV_SHIFT	= 0,
	PCLK_PDBUS_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL4_CON */
	ACLK_CRYPTO_SEL_SHIFT	= 7,
	ACLK_CRYPTO_SEL_MASK	= 1 << ACLK_CRYPTO_SEL_SHIFT,
	ACLK_CRYPTO_SEL_GPLL	= 0,
	ACLK_CRYPTO_SEL_CPLL,
	ACLK_CRYPTO_DIV_SHIFT	= 0,
	ACLK_CRYPTO_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL5_CON */
	CLK_I2C3_DIV_SHIFT	= 8,
	CLK_I2C3_DIV_MASK	= 0x7f << CLK_I2C3_DIV_SHIFT,
	CLK_I2C1_DIV_SHIFT	= 0,
	CLK_I2C1_DIV_MASK	= 0x7f,

	/* CRU_CLK_SEL6_CON */
	CLK_I2C5_DIV_SHIFT	= 8,
	CLK_I2C5_DIV_MASK	= 0x7f << CLK_I2C5_DIV_SHIFT,
	CLK_I2C4_DIV_SHIFT	= 0,
	CLK_I2C4_DIV_MASK	= 0x7f,

	/* CRU_CLK_SEL7_CON */
	CLK_CRYPTO_PKA_SEL_SHIFT	= 15,
	CLK_CRYPTO_PKA_SEL_MASK		= 1 << CLK_CRYPTO_PKA_SEL_SHIFT,
	CLK_CRYPTO_PKA_SEL_GPLL		= 0,
	CLK_CRYPTO_PKA_SEL_CPLL,
	CLK_CRYPTO_PKA_DIV_SHIFT	= 8,
	CLK_CRYPTO_PKA_DIV_MASK		= 0x1f << CLK_CRYPTO_PKA_DIV_SHIFT,
	CLK_CRYPTO_CORE_SEL_SHIFT	= 7,
	CLK_CRYPTO_CORE_SEL_MASK	= 1 << CLK_CRYPTO_CORE_SEL_SHIFT,
	CLK_CRYPTO_CORE_SEL_GPLL	= 0,
	CLK_CRYPTO_CORE_SEL_CPLL,
	CLK_CRYPTO_CORE_DIV_SHIFT	= 0,
	CLK_CRYPTO_CORE_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL8_CON */
	CLK_SPI1_SEL_SHIFT	= 8,
	CLK_SPI1_SEL_MASK	= 1 << CLK_SPI1_SEL_SHIFT,
	CLK_SPI1_SEL_GPLL	= 0,
	CLK_SPI1_SEL_XIN24M,
	CLK_SPI1_DIV_SHIFT	= 0,
	CLK_SPI1_DIV_MASK	= 0x7f,

	/* CRU_CLK_SEL9_CON */
	CLK_PWM2_SEL_SHIFT	= 15,
	CLK_PWM2_SEL_MASK	= 1 << CLK_PWM2_SEL_SHIFT,
	CLK_PWM2_SEL_XIN24M	= 0,
	CLK_PWM2_SEL_GPLL,
	CLK_PWM2_DIV_SHIFT	= 8,
	CLK_PWM2_DIV_MASK	= 0x7f << CLK_PWM2_DIV_SHIFT,

	/* CRU_CLK_SEL20_CON */
	CLK_SARADC_DIV_SHIFT	= 0,
	CLK_SARADC_DIV_MASK	= 0x7ff,

	/* CRU_CLK_SEL25_CON */
	DCLK_DECOM_SEL_SHIFT	= 15,
	DCLK_DECOM_SEL_MASK	= 1 << DCLK_DECOM_SEL_SHIFT,
	DCLK_DECOM_SEL_GPLL	= 0,
	DCLK_DECOM_SEL_CPLL,
	DCLK_DECOM_DIV_SHIFT	= 8,
	DCLK_DECOM_DIV_MASK	= 0x7f << DCLK_DECOM_DIV_SHIFT,

	/* CRU_CLK_SEL26_CON */
	HCLK_PDAUDIO_DIV_SHIFT	= 0,
	HCLK_PDAUDIO_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL45_CON */
	ACLK_PDVO_SEL_SHIFT	= 7,
	ACLK_PDVO_SEL_MASK	= 1 << ACLK_PDVO_SEL_SHIFT,
	ACLK_PDVO_SEL_GPLL	= 0,
	ACLK_PDVO_SEL_CPLL,
	ACLK_PDVO_DIV_SHIFT	= 0,
	ACLK_PDVO_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL47_CON */
	DCLK_VOP_SEL_SHIFT	= 8,
	DCLK_VOP_SEL_MASK	= 1 << DCLK_VOP_SEL_SHIFT,
	DCLK_VOP_SEL_GPLL	= 0,
	DCLK_VOP_SEL_CPLL,
	DCLK_VOP_DIV_SHIFT	= 0,
	DCLK_VOP_DIV_MASK	= 0xff,

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_KERNEL_BOOT)
	/* CRU_CLK_SEL49_CON */
	ACLK_PDVI_SEL_SHIFT	= 6,
	ACLK_PDVI_SEL_MASK	= 0x3 << ACLK_PDVI_SEL_SHIFT,
	ACLK_PDVI_SEL_CPLL	= 0,
	ACLK_PDVI_SEL_GPLL,
	ACLK_PDVI_SEL_HPLL,
	ACLK_PDVI_DIV_SHIFT	= 0,
	ACLK_PDVI_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL50_CON */
	CLK_ISP_SEL_SHIFT	= 6,
	CLK_ISP_SEL_MASK	= 0x3 << CLK_ISP_SEL_SHIFT,
	CLK_ISP_SEL_GPLL	= 0,
	CLK_ISP_SEL_CPLL,
	CLK_ISP_SEL_HPLL,
	CLK_ISP_DIV_SHIFT	= 0,
	CLK_ISP_DIV_MASK	= 0x1f,
#endif

	/* CRU_CLK_SEL53_CON */
	HCLK_PDPHP_DIV_SHIFT	= 8,
	HCLK_PDPHP_DIV_MASK	= 0x1f << HCLK_PDPHP_DIV_SHIFT,
	ACLK_PDPHP_SEL_SHIFT	= 7,
	ACLK_PDPHP_SEL_MASK	= 1 << ACLK_PDPHP_SEL_SHIFT,
	ACLK_PDPHP_SEL_GPLL	= 0,
	ACLK_PDPHP_SEL_CPLL,
	ACLK_PDPHP_DIV_SHIFT	= 0,
	ACLK_PDPHP_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL57_CON */
	EMMC_SEL_SHIFT	= 14,
	EMMC_SEL_MASK	= 0x3 << EMMC_SEL_SHIFT,
	EMMC_SEL_GPLL	= 0,
	EMMC_SEL_CPLL,
	EMMC_SEL_XIN24M,
	EMMC_DIV_SHIFT	= 0,
	EMMC_DIV_MASK	= 0xff,

	/* CRU_CLK_SEL58_CON */
	SCLK_SFC_SEL_SHIFT	= 15,
	SCLK_SFC_SEL_MASK	= 0x1 << SCLK_SFC_SEL_SHIFT,
	SCLK_SFC_SEL_CPLL	= 0,
	SCLK_SFC_SEL_GPLL,
	SCLK_SFC_DIV_SHIFT	= 0,
	SCLK_SFC_DIV_MASK	= 0xff,

	/* CRU_CLK_SEL59_CON */
	CLK_NANDC_SEL_SHIFT	= 15,
	CLK_NANDC_SEL_MASK	= 0x1 << CLK_NANDC_SEL_SHIFT,
	CLK_NANDC_SEL_GPLL	= 0,
	CLK_NANDC_SEL_CPLL,
	CLK_NANDC_DIV_SHIFT	= 0,
	CLK_NANDC_DIV_MASK	= 0xff,

	/* CRU_CLK_SEL61_CON */
	CLK_GMAC_OUT_SEL_SHIFT	= 15,
	CLK_GMAC_OUT_SEL_MASK	= 0x1 << CLK_GMAC_OUT_SEL_SHIFT,
	CLK_GMAC_OUT_SEL_CPLL	= 0,
	CLK_GMAC_OUT_SEL_GPLL,
	CLK_GMAC_OUT_DIV_SHIFT	= 8,
	CLK_GMAC_OUT_DIV_MASK	= 0x1f << CLK_GMAC_OUT_DIV_SHIFT,

	/* CRU_CLK_SEL63_CON */
	PCLK_GMAC_DIV_SHIFT	= 8,
	PCLK_GMAC_DIV_MASK	= 0x1f << PCLK_GMAC_DIV_SHIFT,
	CLK_GMAC_SRC_SEL_SHIFT	= 7,
	CLK_GMAC_SRC_SEL_MASK	= 0x1 << CLK_GMAC_SRC_SEL_SHIFT,
	CLK_GMAC_SRC_SEL_CPLL	= 0,
	CLK_GMAC_SRC_SEL_GPLL,
	CLK_GMAC_SRC_DIV_SHIFT	= 0,
	CLK_GMAC_SRC_DIV_MASK	= 0x1f << CLK_GMAC_SRC_DIV_SHIFT,

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_KERNEL_BOOT)
	/* CRU_CLK_SEL68_CON */
	ACLK_PDISPP_SEL_SHIFT	= 6,
	ACLK_PDISPP_SEL_MASK	= 0x3 << ACLK_PDISPP_SEL_SHIFT,
	ACLK_PDISPP_SEL_CPLL	= 0,
	ACLK_PDISPP_SEL_GPLL,
	ACLK_PDISPP_SEL_HPLL,
	ACLK_PDISPP_DIV_SHIFT	= 0,
	ACLK_PDISPP_DIV_MASK	= 0x1f,

	/* CRU_CLK_SEL69_CON */
	CLK_ISPP_SEL_SHIFT	= 6,
	CLK_ISPP_SEL_MASK	= 0x3 << CLK_ISPP_SEL_SHIFT,
	CLK_ISPP_SEL_CPLL	= 0,
	CLK_ISPP_SEL_GPLL,
	CLK_ISPP_SEL_HPLL,
	CLK_ISPP_DIV_SHIFT	= 0,
	CLK_ISPP_DIV_MASK	= 0x1f,
#endif

	/* CRU_GMAC_CON */
	GMAC_SRC_M1_SEL_SHIFT	= 5,
	GMAC_SRC_M1_SEL_MASK	= 0x1 << GMAC_SRC_M1_SEL_SHIFT,
	GMAC_SRC_M1_SEL_INT	= 0,
	GMAC_SRC_M1_SEL_EXT,
	GMAC_MODE_SEL_SHIFT	= 4,
	GMAC_MODE_SEL_MASK	= 0x1 << GMAC_MODE_SEL_SHIFT,
	GMAC_RGMII_MODE		= 0,
	GMAC_RMII_MODE,
	RGMII_CLK_SEL_SHIFT	= 2,
	RGMII_CLK_SEL_MASK	= 0x3 << RGMII_CLK_SEL_SHIFT,
	RGMII_CLK_DIV0		= 0,
	RGMII_CLK_DIV1,
	RGMII_CLK_DIV50,
	RGMII_CLK_DIV5,
	RMII_CLK_SEL_SHIFT	= 1,
	RMII_CLK_SEL_MASK	= 0x1 << RMII_CLK_SEL_SHIFT,
	RMII_CLK_DIV20		= 0,
	RMII_CLK_DIV2,
	GMAC_SRC_M0_SEL_SHIFT	= 0,
	GMAC_SRC_M0_SEL_MASK	= 0x1,
	GMAC_SRC_M0_SEL_INT	= 0,
	GMAC_SRC_M0_SEL_EXT,

	/* GRF_IOFUNC_CON1 */
	GMAC_SRC_SEL_SHIFT	= 12,
	GMAC_SRC_SEL_MASK	= 1 << GMAC_SRC_SEL_SHIFT,
	GMAC_SRC_SEL_M0		= 0,
	GMAC_SRC_SEL_M1,
};
#endif
