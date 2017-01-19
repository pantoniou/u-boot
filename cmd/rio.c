/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 * Authors: Aurelien Jacquiot <a-jacquiot@ti.com>
 *	    WingMan Kwok <w-kwok2@ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

/*
 * RapidIO support
 */
#include <common.h>
#include <command.h>
#include <mapmem.h>
#include <asm/io.h>
#include <watchdog.h>
#include <rio.h>

static struct udevice *rio_dev;

static int do_rio_local_write(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	u32 offset, val;
	int ret;

	offset = simple_strtoul(argv[0], NULL, 16);
	   val = simple_strtoul(argv[1], NULL, 16);

	ret = rio_local_config_write(rio_dev, offset, sizeof(val), val);
	if (ret) {
		printf("cannot perform local write\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_rio_local_read(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	u32 offset, val;
	int ret;

	offset = simple_strtoul(argv[0], NULL, 16);

	ret = rio_local_config_read(rio_dev, offset, sizeof(val), &val);
	if (ret) {
		printf("cannot perform local read\n");
		return CMD_RET_FAILURE;
	}

	/* Display value */
	print_buffer(offset, (const void *)&val, sizeof(val), 1, 0);
	return CMD_RET_SUCCESS;
}

static int do_rio_write(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int portid;
	u16 destid;
	u8 hopcount;
	u32 offset;
	u32 val;
	int ret;

	portid = simple_strtol(argv[0], NULL, 10);
	destid = simple_strtol(argv[1], NULL, 10) & 0xffff;
	hopcount = simple_strtoul(argv[2], NULL, 10) & 0xff;
	offset = simple_strtoul(argv[3], NULL, 16);
	val = simple_strtoul(argv[4], NULL, 16);

	/* Do maintenance write */
	ret = rio_config_write(rio_dev, portid, destid, hopcount,
			       offset, sizeof(val), val);

	if (ret) {
		printf("cannot perform maintenance write\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_rio_read(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int portid;
	u16 destid;
	u8 hopcount;
	u32 offset;
	u32 val;
	int ret;

	portid = simple_strtol(argv[0], NULL, 10);
	destid = simple_strtol(argv[1], NULL, 10) & 0xffff;
	hopcount = simple_strtoul(argv[2], NULL, 10) & 0xff;
	offset = simple_strtoul(argv[3], NULL, 16);

	/* Do maintenance read */
	ret = rio_config_read(rio_dev, portid, destid, hopcount,
			      offset, sizeof(val), &val);

	if (ret) {
		printf("cannot perform maintenance read\n");
		return CMD_RET_FAILURE;
	}

	/* Display value */
	print_buffer(offset, (const void *)&val, sizeof(val), 1, 0);
	return CMD_RET_SUCCESS;
}

static int do_rio_doorbell_rx(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	u32 info = 0;
	int ret;

	if (!argc)
		return CMD_RET_USAGE;

	info = simple_strtoul(argv[0], NULL, 16);

	ret = rio_doorbell_rx(rio_dev, info);
	if (!ret)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

static int do_rio_remove(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	rio_remove(rio_dev);
	rio_dev = NULL;
	return CMD_RET_SUCCESS;
}

static int do_rio_devices(cmd_tbl_t *cmdtp, int flag,
			  int argc, char * const argv[])
{
	struct udevice *dev;
	int i, ret;

	puts("RapidIO uclass entries:\n");
	printf("devnum    device             driver\n");

	for (i = 0, ret = uclass_first_device(UCLASS_RIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		printf("  %d       %s    %s\n",
		       i++, dev->name, dev->driver->name);
	}

	return cmd_process_error(cmdtp, ret);
}

static cmd_tbl_t rio_commands[] = {
	U_BOOT_CMD_MKENT(devices, 0, 1, do_rio_devices, "", ""),
	U_BOOT_CMD_MKENT(remove, 1, 0, do_rio_remove, "", ""),
	U_BOOT_CMD_MKENT(doorbell_rx, 2, 0, do_rio_doorbell_rx, "", ""),
	U_BOOT_CMD_MKENT(r, 5, 1, do_rio_read, "", ""),
	U_BOOT_CMD_MKENT(w, 6, 0, do_rio_write, "", ""),
	U_BOOT_CMD_MKENT(lr, 2, 1, do_rio_local_read, "", ""),
	U_BOOT_CMD_MKENT(lw, 3, 0, do_rio_local_write, "", ""),
};

static int do_rio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *rio_cmd;
	int devnum = 0;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	rio_cmd = find_cmd_tbl(argv[1], rio_commands,
			       ARRAY_SIZE(rio_commands));
	argc -= 2;
	argv += 2;

	if ((!rio_cmd || argc > rio_cmd->maxargs) ||
	    ((strcmp(rio_cmd->name, "devices")) && (argc < 1)))
		return CMD_RET_USAGE;

	if (argc) {
		devnum = simple_strtoul(argv[0], NULL, 10);
		ret = uclass_get_device(UCLASS_RIO, devnum, &rio_dev);
		if (ret)
			return cmd_process_error(cmdtp, ret);
		argc--;
		argv++;
	} else {
		rio_dev = NULL;
		if (rio_cmd->cmd != do_rio_devices)
			return CMD_RET_USAGE;
	}

	ret = rio_cmd->cmd(rio_cmd, flag, argc, argv);

	return cmd_process_error(rio_cmd, ret);
}

U_BOOT_CMD(rio, CONFIG_SYS_MAXARGS, 0, do_rio,
	   "RapidIO sub-system",
	   "devices - show available RapidIO devices\n"
	   "rio remove <devnum> - remove RapidIO device\n"
	   "rio doorbell_rx <devnum> [info] - blocking wait for doorbell info\n"
	   "rio r <devnum> port dst_id hopcount offset - perform config read\n"
	   "rio w <devnum> port dst_id hopcount offset value - perform config write\n"
	   "rio lr <devnum> offset - perform local config read\n"
	   "rio lw <devnum> offset value - perform local config write\n"
);
