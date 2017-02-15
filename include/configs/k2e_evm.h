/*
 * Configuration header file for TI's k2e-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2E_EVM_H
#define __CONFIG_K2E_EVM_H

#include <environment/ti/spi.h>

/* Platform type */
#define CONFIG_SOC_K2E

#ifdef CONFIG_TI_SECURE_DEVICE
#define DEFAULT_SEC_BM_BOOT_ENV						\
	"addr_sec_bm=0xc08000\0"					\
	"addr_sec_bm_mkimg=0xc07ffc0\0"					\
	"sec_bm_size=0x1210\0"						\
	"fit_bootfile=fitImage.itb\0"					\
	"update_to_fit=setenv bootfile ${fit_bootfile}\0"		\
	"sec_bm_copy=go ${addr_sec_bm}4 0xc084000 ${sec_bm_size}\0"	\
	"findfdt=setenv fdtfile ${name_fdt}\0"

#define CONFIG_BOOTCOMMAND						\
	"run sec_bm_copy; mon_install ${addr_sec_bm_mkimg};"		\
	"run update_to_fit; run findfdt;"				\
	"dhcp ${loadaddr} ${tftp_root}/${bootfile};"			\
	"run args_all; run args_ramfs; bootm ${loadaddr}#${fdtfile}"
#else
#define DEFAULT_SEC_BM_BOOT_ENV
#endif

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	DEFAULT_SEC_BM_BOOT_ENV						\
	"boot=ubi\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"name_fdt=keystone-k2e-evm.dtb\0"				\
	"name_mon=skern-k2e.bin\0"					\
	"name_ubi=k2e-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2e-evm.gph\0"				\
	"name_fs=arago-console-image-k2e-evm.cpio.gz\0"

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_NAND_DAVINCI
#define CONFIG_KEYSTONE_RBL_NAND
#define CONFIG_KEYSTONE_NAND_MAX_RBL_SIZE	CONFIG_ENV_OFFSET
#define CONFIG_SYS_NAND_MASK_CLE		0x4000
#define CONFIG_SYS_NAND_MASK_ALE		0x2000
#define CONFIG_SYS_NAND_CS			2
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_LARGEPAGE

#include <configs/ti_armv7_keystone2.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE           0x0c100000


#define SPI_MTD_PARTS KEYSTONE_SPI0_MTD_PARTS

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	9
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE

#define CONFIG_DDR_SPD

#endif /* __CONFIG_K2E_EVM_H */
