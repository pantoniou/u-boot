/*
 * (C) Copyright 2017
 * Texas Instruments, <www.ti.com>
 *
 * Franklin S Cooper Jr. <fcooper@ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <boot_fit.h>
#include <common.h>
#include <errno.h>
#include <image.h>
#include <libfdt.h>

static ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
	const u32 *cell;
	int len;

	cell = fdt_getprop(fdt, node, prop, &len);
	if (!cell || len != sizeof(*cell))
		return FDT_ERROR;

	return fdt32_to_cpu(*cell);
}

static int fit_select_fdt(const void *fdt, int images, int *fdt_offsetp)
{
	const char *name, *fdt_name;
	int conf, node, fdt_node;
	int len;

	*fdt_offsetp = 0;
	conf = fdt_path_offset(fdt, FIT_CONFS_PATH);
	if (conf < 0) {
		debug("%s: Cannot find /configurations node: %d\n", __func__,
		      conf);
		return -EINVAL;
	}
	for (node = fdt_first_subnode(fdt, conf);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		name = fdt_getprop(fdt, node, "description", &len);
		if (!name) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			printf("%s: Missing FDT description in DTB\n",
			       __func__);
#endif
			return -EINVAL;
		}
		if (board_fit_config_name_match(name))
			continue;

		debug("Selecting config '%s'", name);
		fdt_name = fdt_getprop(fdt, node, FIT_FDT_PROP, &len);
		if (!fdt_name) {
			debug("%s: Cannot find fdt name property: %d\n",
			      __func__, len);
			return -EINVAL;
		}

		debug(", fdt '%s'\n", fdt_name);
		fdt_node = fdt_subnode_offset(fdt, images, fdt_name);
		if (fdt_node < 0) {
			debug("%s: Cannot find fdt node '%s': %d\n",
			      __func__, fdt_name, fdt_node);
			return -EINVAL;
		}

		*fdt_offsetp = fdt_getprop_u32(fdt, fdt_node, "data-offset");
		len = fdt_getprop_u32(fdt, fdt_node, "data-size");
		debug("FIT: Selected '%s'\n", name);

		return len;
	}

#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	printf("No matching DT out of these options:\n");
	for (node = fdt_first_subnode(fdt, conf);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		name = fdt_getprop(fdt, node, "description", &len);
		printf("   %s\n", name);
	}
#endif

	return -ENOENT;
}

int fdt_offset(void *fit)
{
	int fdt_offset, fdt_len;
	int images;

	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		return -1;
	}

	/* Figure out which device tree the board wants to use */
	fdt_len = fit_select_fdt(fit, images, &fdt_offset);

	if (fdt_len < 0)
		return fdt_len;

	return fdt_offset;
}

void *locate_dtb_in_fit(void *fit)
{
	struct image_header *header;
	int size;
	int ret;

	size = fdt_totalsize(fit);
	size = (size + 3) & ~3;

	header = (struct image_header *)fit;

	if (image_get_magic(header) != FDT_MAGIC) {
		debug("No FIT image appended to U-boot\n");
		return NULL;
	}

	ret = fdt_offset(fit);

	if (ret <= 0)
		return NULL;
	else
		return (void *)fit+size+ret;
}
