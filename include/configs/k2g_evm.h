/*
 * Configuration header file for TI's k2g-evm
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2G_EVM_H
#define __CONFIG_K2G_EVM_H

#include <environment/ti/mmc.h>
#include <environment/ti/spi.h>

/* Platform type */
#define CONFIG_SOC_K2G

#define CONFIG_BOARD_LATE_INIT
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_MMC_TI_ARGS						\
	DEFAULT_PMMC_BOOT_ENV						\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	DEFAULT_FIT_TI_ARGS						\
	"boot=mmc\0"							\
	"console=ttyS0,115200n8\0"					\
	"bootpart=0:2\0"						\
	"bootdir=/boot\0"						\
	"rd_spec=-\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"findfdt="\
		"if test $board_name = 66AK2GGP; then " \
			 "setenv name_fdt keystone-k2g-evm.dtb; " \
		"else if test $board_name = 66AK2GIC; then " \
			 "setenv name_fdt keystone-k2g-ice.dtb; " \
		"else if test $name_fdt = undefined; then " \
			"echo WARNING: Could not determine device tree to use;"\
		"fi;fi;fi; setenv fdtfile ${name_fdt}\0" \
	"name_mon=skern-k2g.bin\0"					\
	"name_ubi=k2g-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2g-evm.gph\0"				\
	"init_mmc=run args_all args_mmc\0"				\
	"init_fw_rd_mmc=load mmc ${bootpart} ${rdaddr} "		\
		"${bootdir}/${name_fw_rd}; run set_rd_spec\0"		\
	"soc_variant=k2g\0"						\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${name_fdt}\0"\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_mon_mmc=load mmc ${bootpart} ${addr_mon} ${bootdir}/${name_mon}\0"\
	"name_fs=arago-base-tisdk-image-k2g-evm.cpio\0"

#ifndef CONFIG_TI_SECURE_DEVICE
#define CONFIG_BOOTCOMMAND						\
	"run findfdt; "							\
	"run envboot; "							\
	"run init_${boot}; "						\
	"run get_mon_${boot} run_mon; "					\
	"run set_name_pmmc get_pmmc_${boot} run_pmmc; "			\
	"run get_kern_${boot}; "					\
	"run init_fw_rd_${boot}; "					\
	"run get_fdt_${boot}; "						\
	"run run_kern"
#else
#define CONFIG_BOOTCOMMAND						\
	"run findfdt; "							\
	"run envboot; "							\
	"run run_mon_hs; "						\
	"run init_${boot}; "						\
	"run set_name_pmmc get_pmmc_${boot} run_pmmc; "			\
	"run get_fit_${boot}; "						\
	"bootm ${fit_loadaddr}#${name_fdt}"
#endif

#include <configs/ti_armv7_keystone2.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c080000

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	2
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE
#define CONFIG_PHY_MICREL
#define PHY_ANEG_TIMEOUT	10000 /* PHY needs longer aneg time */

#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_IS_IN_FAT
#define FAT_ENV_INTERFACE		"mmc"
#define FAT_ENV_DEVICE_AND_PART		"0:1"
#define FAT_ENV_FILE			"uboot.env"

#define CONFIG_SF_DEFAULT_BUS		1
#define CONFIG_SF_DEFAULT_CS		0

#ifndef CONFIG_SPL_BUILD
#define CONFIG_CADENCE_QSPI
#define CONFIG_CQSPI_REF_CLK 384000000
#define CONFIG_CQSPI_DECODER 0x0
#define CONFIG_BOUNCE_BUFFER
#endif

#define SPI_MTD_PARTS	KEYSTONE_SPI1_MTD_PARTS

/* NAND */
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_MTD_DEVICE               /* Required for mtdparts */
#define CONFIG_CMD_MTDPARTS

/* NAND: device related configs - Micron MT29F2G16ABAFAWP */
#define CONFIG_SYS_NAND_PAGE_SIZE       2048
#define CONFIG_SYS_NAND_OOBSIZE         224
#define CONFIG_SYS_NAND_BLOCK_SIZE      (128*1024)
#define CONFIG_SYS_NAND_PAGE_COUNT      (CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
/* NAND: driver related configs */
#define TI_GPMC_BASE			KS2_GPMC_BASE
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_BASE		0x30000000
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
#define CONFIG_NAND_OMAP_ELM
#define ELM_BASE			0x021c8000
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME      OMAP_ECC_BCH16_CODE_HW
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS  { 2, 3, 4, 5, 6, 7, 8, 9, \
				  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
				  20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
				  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
				  40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
				  50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
				  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
				  70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
				  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
				  90, 91, 92, 93, 94, 95, 96, 97, 98, 99, \
				  100, 101, 102, 103, 104, 105, 106, 107, 108, \
				  109, 110, 111, 112, 113, 114, 115, 116, 117, \
				  118, 119, 120, 121, 122, 123, 124, 125, 126, \
				  127, 128, 129, 130, 131, 132, 133, 134, 135, \
				  136, 137, 138, 139, 140, 141, 142, 143, 144, \
				  145, 146, 147, 148, 149, 150, 151, 152, 153, \
				  154, 155, 156, 157, 158, 159, 160, 161, 162, \
				  163, 164, 165, 166, 167, 168, 169, 170, 171, \
				  172, 173, 174, 175, 176, 177, 178, 179, 180, \
				  181, 182, 183, 184, 185, 186, 187, 188, 189, \
				  190, 191, 192, 193, 194, 195, 196, 197, 198, \
				  199, 200, 201, 202, 203, 204, 205, 206, 207, \
				  208, 209, \
				}
#define CONFIG_SYS_NAND_ECCSIZE         512
#define CONFIG_SYS_NAND_ECCBYTES        26
#define CONFIG_PHY_TI
#endif /* __CONFIG_K2G_EVM_H */
