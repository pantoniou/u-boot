/*
 * Copyright (c) 2017 Texas Instruments, Inc
 * Authors: Aurelien Jacquiot <a-jacquiot@ti.com>
 *	    WingMan Kwok <w-kwok2@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/list.h>
#include <rio.h>

DECLARE_GLOBAL_DATA_PTR;

UCLASS_DRIVER(rapidio) = {
	.name		= "rapidio",
	.id		= UCLASS_RIO,
};

int rio_config_read(struct udevice *dev, int portid, u16 destid,
		    u8 hopcount, u32 offset, int len, u32 *val)
{
	const struct rio_ops *ops = device_get_ops(dev);

	if (!ops->config_read)
		return -ENOSYS;

	return ops->config_read(dev, portid, destid, hopcount,
				offset, len, val);
}

int rio_config_write(struct udevice *dev, int portid, u16 destid,
		     u8 hopcount, u32 offset, int len, u32 val)
{
	const struct rio_ops *ops = device_get_ops(dev);

	if (!ops->config_write)
		return -ENOSYS;

	return ops->config_write(dev, portid, destid, hopcount,
				 offset, len, val);
}

int rio_local_config_read(struct udevice *dev, u32 offset, int len, u32 *data)
{
	const struct rio_ops *ops = device_get_ops(dev);

	if (!ops->local_config_read)
		return -ENOSYS;

	return ops->local_config_read(dev, offset, len, data);
}

int rio_local_config_write(struct udevice *dev, u32 offset, int len, u32 data)
{
	const struct rio_ops *ops = device_get_ops(dev);

	if (!ops->local_config_write)
		return -ENOSYS;

	return ops->local_config_write(dev, offset, len, data);
}

int rio_doorbell_rx(struct udevice *dev, u16 info)
{
	const struct rio_ops *ops = device_get_ops(dev);

	if (!ops->doorbell_rx)
		return -ENOSYS;

	return ops->doorbell_rx(dev, info);
}

int rio_remove(struct udevice *dev)
{
	return device_remove(dev);
}
