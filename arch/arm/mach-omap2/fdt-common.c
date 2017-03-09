/*
 * Copyright 2016-2017 Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>

#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>

#ifdef CONFIG_TI_SECURE_DEVICE

/* Give zero values if not already defined */
#ifndef TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ (0)
#endif
#ifndef CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ
#define CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ (0)
#endif

int ft_hs_disable_rng(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;

	/* Make HW RNG reserved for secure world use */
	path = "/ocp/rng";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}
	ret = fdt_setprop_string(fdt, offs,
				 "status", "disabled");
	if (ret < 0) {
		printf("Could not add status property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}
	return 0;
}

#if (CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE != 0)
int ft_hs_fixup_dram(void *fdt, bd_t *bd)
{
	const char *path, *subpath;
	int offs;
	u32 sec_mem_start = get_sec_mem_start();
	u32 sec_mem_size = CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE;
	fdt64_t temp[2];
	fdt32_t cells;

	/* Delete any original secure_reserved node */
	path = "/reserved-memory/secure_reserved";
	offs = fdt_path_offset(fdt, path);
	if (offs >= 0)
		fdt_del_node(fdt, offs);

	/* Add new secure_reserved node */
	path = "/reserved-memory";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found\n", path);
		path = "/";
		subpath = "reserved-memory";
		offs = fdt_path_offset(fdt, path);
		offs = fdt_add_subnode(fdt, offs, subpath);
		if (offs < 0) {
			printf("Could not create %s%s node.\n", path, subpath);
			return 1;
		}
		path = "/reserved-memory";
		offs = fdt_path_offset(fdt, path);
#ifdef CONFIG_ARMV7_LPAE
		cells = cpu_to_fdt32(2);
#else
		cells = cpu_to_fdt32(1);
#endif
		fdt_setprop(fdt, offs, "#address-cells", &cells, sizeof(cells));
		fdt_setprop(fdt, offs, "#size-cells", &cells, sizeof(cells));
		fdt_setprop(fdt, offs, "ranges", NULL, 0);
	}

	subpath = "secure_reserved";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s%s node.\n", path, subpath);
		return 1;
	}

	temp[0] = cpu_to_fdt64(((u64)sec_mem_start));
	temp[1] = cpu_to_fdt64(((u64)sec_mem_size));
	fdt_setprop_string(fdt, offs, "compatible",
			   "ti,dra7-secure-memory");
	fdt_setprop_string(fdt, offs, "status", "okay");
	fdt_setprop(fdt, offs, "no-map", NULL, 0);
	fdt_setprop(fdt, offs, "reg", temp, sizeof(temp));

	return 0;
}
#else
int ft_hs_fixup_dram(void *fdt, bd_t *bd) { return 0; }
#endif

int ft_hs_add_tee(void *fdt, bd_t *bd)
{
	const char *path, *subpath;
	int offs;

	extern int tee_loaded;
	if (!tee_loaded)
		return 0;

	path = "/";
	offs = fdt_path_offset(fdt, path);

	subpath = "firmware";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s node.\n", subpath);
		return 1;
	}

	subpath = "optee";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s node.\n", subpath);
		return 1;
	}

	fdt_setprop_string(fdt, offs, "compatible", "linaro,optee-tz");
	fdt_setprop_string(fdt, offs, "method", "smc");

	return 0;
}

#endif
