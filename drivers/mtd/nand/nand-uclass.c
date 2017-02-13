/*
 * NAND uclass driver for NAND bus.
 *
 * (C) Copyright 2017
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <nand.h>

DECLARE_GLOBAL_DATA_PTR;

struct mtd_info *get_nand_dev_by_index(int idx)
{
	struct nand_chip *chip;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_NAND, idx, &dev);
	if (ret) {
		debug("NAND device (%d) not found\n", idx);
		return NULL;
	}

	chip = (struct nand_chip *)dev_get_priv(dev);

	return nand_to_mtd(chip);
}

UCLASS_DRIVER(nand) = {
	.id				= UCLASS_NAND,
	.name				= "nand",
	.flags				= DM_UC_FLAG_SEQ_ALIAS,
};
