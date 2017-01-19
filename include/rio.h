/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 * Authors: Aurelien Jacquiot <a-jacquiot@ti.com>
 *
 * Part of this file is borrowed from Linux rio_regs.h.
 * (C) Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _RIO_H_
#define _RIO_H_

#include <dm.h>

/*
 * In RapidIO, each device has a 16MB configuration space that is
 * accessed via maintenance transactions. Portions of configuration
 * space are standardized and/or reserved.
 */
#define RIO_DEV_ID_CAR			0x00
#define RIO_DEV_INFO_CAR		0x04
#define RIO_ASM_ID_CAR			0x08
#define RIO_ASM_ID_MASK			0xffff0000
#define RIO_ASM_VEN_ID_MASK		0x0000ffff

#define RIO_ASM_INFO_CAR		0x0c
#define RIO_ASM_REV_MASK		0xffff0000
#define RIO_EXT_FTR_PTR_MASK		0x0000ffff

#define RIO_PEF_CAR			0x10
#define RIO_PEF_BRIDGE			0x80000000
#define RIO_PEF_MEMORY			0x40000000
#define RIO_PEF_PROCESSOR		0x20000000
#define RIO_PEF_SWITCH			0x10000000
#define RIO_PEF_INB_MBOX		0x00f00000
#define RIO_PEF_INB_MBOX0		0x00800000
#define RIO_PEF_INB_MBOX1		0x00400000
#define RIO_PEF_INB_MBOX2		0x00200000
#define RIO_PEF_INB_MBOX3		0x00100000
#define RIO_PEF_INB_DOORBELL		0x00080000
#define RIO_PEF_EXT_RT			0x00000200
#define RIO_PEF_STD_RT			0x00000100
#define RIO_PEF_CTLS			0x00000010
#define RIO_PEF_FLOW_CONTROL		0x00000080
#define RIO_PEF_EXT_FEATURES		0x00000008
#define RIO_PEF_ADDR_66			0x00000004
#define RIO_PEF_ADDR_50			0x00000002
#define RIO_PEF_ADDR_34			0x00000001

#define RIO_SWP_INFO_CAR		0x14
#define RIO_SWP_INFO_PORT_TOTAL_MASK	0x0000ff00
#define RIO_SWP_INFO_PORT_NUM_MASK	0x000000ff

#define RIO_SRC_OPS_CAR			0x18
#define RIO_SRC_OPS_READ		0x00008000
#define RIO_SRC_OPS_WRITE		0x00004000
#define RIO_SRC_OPS_STREAM_WRITE	0x00002000
#define RIO_SRC_OPS_WRITE_RESPONSE	0x00001000
#define RIO_SRC_OPS_DATA_MSG		0x00000800
#define RIO_SRC_OPS_DOORBELL		0x00000400
#define RIO_SRC_OPS_ATOMIC_TST_SWP	0x00000100
#define RIO_SRC_OPS_ATOMIC_INC		0x00000080
#define RIO_SRC_OPS_ATOMIC_DEC		0x00000040
#define RIO_SRC_OPS_ATOMIC_SET		0x00000020
#define RIO_SRC_OPS_ATOMIC_CLR		0x00000010
#define RIO_SRC_OPS_PORT_WRITE		0x00000004

#define RIO_DST_OPS_CAR			0x1c
#define RIO_DST_OPS_READ		0x00008000
#define RIO_DST_OPS_WRITE		0x00004000
#define RIO_DST_OPS_STREAM_WRITE	0x00002000
#define RIO_DST_OPS_WRITE_RESPONSE	0x00001000
#define RIO_DST_OPS_DATA_MSG		0x00000800
#define RIO_DST_OPS_DOORBELL		0x00000400
#define RIO_DST_OPS_ATOMIC_TST_SWP	0x00000100
#define RIO_DST_OPS_ATOMIC_INC		0x00000080
#define RIO_DST_OPS_ATOMIC_DEC		0x00000040
#define RIO_DST_OPS_ATOMIC_SET		0x00000020
#define RIO_DST_OPS_ATOMIC_CLR		0x00000010
#define RIO_DST_OPS_PORT_WRITE		0x00000004

#define RIO_OPS_READ			0x00008000
#define RIO_OPS_WRITE			0x00004000
#define RIO_OPS_STREAM_WRITE		0x00002000
#define RIO_OPS_WRITE_RESPONSE		0x00001000
#define RIO_OPS_DATA_MSG		0x00000800
#define RIO_OPS_DOORBELL		0x00000400
#define RIO_OPS_ATOMIC_TST_SWP		0x00000100
#define RIO_OPS_ATOMIC_INC		0x00000080
#define RIO_OPS_ATOMIC_DEC		0x00000040
#define RIO_OPS_ATOMIC_SET		0x00000020
#define RIO_OPS_ATOMIC_CLR		0x00000010
#define RIO_OPS_PORT_WRITE		0x00000004

/* 0x20-0x30 *//* Reserved */

#define RIO_SWITCH_RT_LIMIT		0x34
#define RIO_RT_MAX_DESTID		0x0000ffff

#define RIO_MBOX_CSR			0x40
#define RIO_MBOX0_AVAIL			0x80000000
#define RIO_MBOX0_FULL			0x40000000
#define RIO_MBOX0_EMPTY			0x20000000
#define RIO_MBOX0_BUSY			0x10000000
#define RIO_MBOX0_FAIL			0x08000000
#define RIO_MBOX0_ERROR			0x04000000
#define RIO_MBOX1_AVAIL			0x00800000
#define RIO_MBOX1_FULL			0x00200000
#define RIO_MBOX1_EMPTY			0x00200000
#define RIO_MBOX1_BUSY			0x00100000
#define RIO_MBOX1_FAIL			0x00080000
#define RIO_MBOX1_ERROR			0x00040000
#define RIO_MBOX2_AVAIL			0x00008000
#define RIO_MBOX2_FULL			0x00004000
#define RIO_MBOX2_EMPTY			0x00002000
#define RIO_MBOX2_BUSY			0x00001000
#define RIO_MBOX2_FAIL			0x00000800
#define RIO_MBOX2_ERROR			0x00000400
#define RIO_MBOX3_AVAIL			0x00000080
#define RIO_MBOX3_FULL			0x00000040
#define RIO_MBOX3_EMPTY			0x00000020
#define RIO_MBOX3_BUSY			0x00000010
#define RIO_MBOX3_FAIL			0x00000008
#define RIO_MBOX3_ERROR			0x00000004

#define RIO_WRITE_PORT_CSR		0x44
#define RIO_DOORBELL_CSR		0x44
#define RIO_DOORBELL_AVAIL		0x80000000
#define RIO_DOORBELL_FULL		0x40000000
#define RIO_DOORBELL_EMPTY		0x20000000
#define RIO_DOORBELL_BUSY		0x10000000
#define RIO_DOORBELL_FAILED		0x08000000
#define RIO_DOORBELL_ERROR		0x04000000
#define RIO_WRITE_PORT_AVAILABLE	0x00000080
#define RIO_WRITE_PORT_FULL		0x00000040
#define RIO_WRITE_PORT_EMPTY		0x00000020
#define RIO_WRITE_PORT_BUSY		0x00000010
#define RIO_WRITE_PORT_FAILED		0x00000008
#define RIO_WRITE_PORT_ERROR		0x00000004

/* 0x48 *//* Reserved */

#define RIO_PELL_CTRL_CSR		0x4c
#define RIO_PELL_ADDR_66		0x00000004
#define RIO_PELL_ADDR_50		0x00000002
#define RIO_PELL_ADDR_34		0x00000001

/* 0x50-0x54 *//* Reserved */

#define RIO_LCSH_BA			0x58
#define RIO_LCSL_BA			0x5c

#define RIO_DID_CSR			0x60

/* 0x64 *//* Reserved */

#define RIO_HOST_DID_LOCK_CSR		0x68
#define RIO_COMPONENT_TAG_CSR		0x6c

#define RIO_STD_RTE_CONF_DESTID_SEL_CSR	0x70
#define RIO_STD_RTE_CONF_PORT_SEL_CSR	0x74
#define RIO_STD_RTE_DEFAULT_PORT	0x78

/* 0x7c-0xf8 *//* Reserved */
/* 0x100-0xfff8 *//* [I] Extended Features Space */

#define RIO_PORT_LINK_MAINT_REQ_CSR	0x140
#define RIO_PORT_LINK_MAINT_RESP_CSR	0x144
#define RIO_PORT_LOCAL_ACKID_CSR	0x148
#define RIO_PORT_ERR_STAT_CSR		0x158
#define RIO_PORT_CTRL_CSR		0x15c

#define RIO_PORT_OFFSET			0x20

#define RIO_PORT_CTRL_TYPE_SERIAL	0x00000001
#define RIO_PORT_CTRL_ENUM_BOUNDARY	0x00020000
#define RIO_PORT_CTRL_MCAST_EVT_PART	0x00080000
#define RIO_PORT_CTRL_ERR_CHECK_DIS	0x00100000
#define RIO_PORT_CTRL_INPUT_PORT_EN	0x00200000
#define RIO_PORT_CTRL_OUTPUT_PORT_EN	0x00400000
#define RIO_PORT_CTRL_PORT_DIS		0x00800000

/* 0x10000-0xfffff8 *//* [I] Implementation-defined Space */

/*
 * Extended Features Space is a configuration space area where
 * functionality is mapped into extended feature blocks via a
 * singly linked list of extended feature pointers (EFT_PTR).
 *
 * Each extended feature block can be identified/located in
 * Extended Features Space by walking the extended feature
 * list starting with the Extended Feature Pointer located
 * in the Assembly Information CAR.
 *
 * Extended Feature Blocks (EFBs) are identified with an assigned
 * EFB ID. Extended feature block offsets in the definitions are
 * relative to the offset of the EFB within the  Extended Features
 * Space.
 */

/* Helper macros to parse the Extended Feature Block header */
#define RIO_EFB_PTR_MASK		0xffff0000
#define RIO_EFB_ID_MASK			0x0000ffff
#define RIO_GET_BLOCK_PTR(x)		((x & RIO_EFB_PTR_MASK) >> 16)
#define RIO_GET_BLOCK_ID(x)		(x & RIO_EFB_ID_MASK)

/* Extended Feature Block IDs */
#define RIO_EFB_PAR_EP_ID		0x0001
#define RIO_EFB_PAR_EP_REC_ID		0x0002
#define RIO_EFB_PAR_EP_FREE_ID		0x0003
#define RIO_EFB_SER_EP_ID_V13P		0x0001
#define RIO_EFB_SER_EP_REC_ID_V13P	0x0002
#define RIO_EFB_SER_EP_FREE_ID_V13P	0x0003
#define RIO_EFB_SER_EP_ID		0x0004
#define RIO_EFB_SER_EP_REC_ID		0x0005
#define RIO_EFB_SER_EP_FREE_ID		0x0006
#define RIO_EFB_SER_EP_FREC_ID		0x0009
#define RIO_EFB_ERR_MGMNT		0x0007

/*
 * Physical 8/16 LP-LVDS
 * ID=0x0001, Generic End Point Devices
 * ID=0x0002, Generic End Point Devices, software assisted recovery option
 * ID=0x0003, Generic End Point Free Devices
 *
 * Physical LP-Serial
 * ID=0x0004, Generic End Point Devices
 * ID=0x0005, Generic End Point Devices, software assisted recovery option
 * ID=0x0006, Generic End Point Free Devices
 */
#define RIO_PORT_MNT_HEADER		0x0000
#define RIO_PORT_REQ_CTL_CSR		0x0020
#define RIO_PORT_RSP_CTL_CSR		0x0024 /* 0x0001/0x0002 */
#define RIO_PORT_LINKTO_CTL_CSR		0x0020 /* Serial */
#define RIO_PORT_RSPTO_CTL_CSR		0x0024 /* Serial */
#define RIO_PORT_GEN_CTL_CSR		0x003c
#define RIO_PORT_GEN_HOST		0x80000000
#define RIO_PORT_GEN_MASTER		0x40000000
#define RIO_PORT_GEN_DISCOVERED		0x20000000
#define RIO_PORT_N_MNT_REQ_CSR(x)	(0x0040 + x * 0x20)
#define RIO_PORT_N_MNT_RSP_CSR(x)	(0x0044 + x * 0x20)
#define RIO_PORT_N_MNT_RSP_RVAL		0x80000000
#define RIO_PORT_N_MNT_RSP_ASTAT	0x000003e0
#define RIO_PORT_N_MNT_RSP_LSTAT	0x0000001f
#define RIO_PORT_N_ACK_STS_CSR(x)	(0x0048 + x * 0x20)
#define RIO_PORT_N_ACK_CLEAR		0x80000000
#define RIO_PORT_N_ACK_INBOUND		0x1f000000
#define RIO_PORT_N_ACK_OUTSTAND		0x00001f00
#define RIO_PORT_N_ACK_OUTBOUND		0x0000001f
#define RIO_PORT_N_ERR_STS_CSR(x)	(0x0058 + x * 0x20)
#define RIO_PORT_N_ERR_STS_PW_OUT_ES	0x00010000 /* Output Error-stopped */
#define RIO_PORT_N_ERR_STS_PW_INP_ES	0x00000100 /* Input Error-stopped */
#define RIO_PORT_N_ERR_STS_PW_PEND	0x00000010 /* Port-Write Pending */
#define RIO_PORT_N_ERR_STS_PORT_ERR	0x00000004
#define RIO_PORT_N_ERR_STS_PORT_OK	0x00000002
#define RIO_PORT_N_ERR_STS_PORT_UNINIT	0x00000001
#define RIO_PORT_N_ERR_STS_CLR_MASK	0x07120204
#define RIO_PORT_N_CTL_CSR(x)		(0x005c + x * 0x20)
#define RIO_PORT_N_CTL_PWIDTH		0xc0000000
#define RIO_PORT_N_CTL_PWIDTH_1		0x00000000
#define RIO_PORT_N_CTL_PWIDTH_4		0x40000000
#define RIO_PORT_N_CTL_P_TYP_SER	0x00000001
#define RIO_PORT_N_CTL_LOCKOUT		0x00000002
#define RIO_PORT_N_CTL_EN_RX_SER	0x00200000
#define RIO_PORT_N_CTL_EN_TX_SER	0x00400000
#define RIO_PORT_N_CTL_EN_RX_PAR	0x08000000
#define RIO_PORT_N_CTL_EN_TX_PAR	0x40000000

/*
 * Error Management Extensions (RapidIO 1.3+, Part 8)
 *
 * Extended Features Block ID=0x0007
 */

/* General EM Registers (Common for all Ports) */

#define RIO_EM_EFB_HEADER		0x000
#define RIO_EM_LTL_ERR_DETECT		0x008
#define RIO_EM_LTL_ERR_EN		0x00c
#define RIO_EM_LTL_HIADDR_CAP		0x010
#define RIO_EM_LTL_ADDR_CAP		0x014
#define RIO_EM_LTL_DEVID_CAP		0x018
#define RIO_EM_LTL_CTRL_CAP		0x01c
#define RIO_EM_PW_TGT_DEVID		0x028
#define RIO_EM_PKT_TTL			0x02c

/* Per-Port EM Registers */

#define RIO_EM_PN_ERR_DETECT(x)		(0x040 + x * 0x40)
#define REM_PED_IMPL_SPEC		0x80000000
#define REM_PED_LINK_TO			0x00000001
#define RIO_EM_PN_ERRRATE_EN(x)		(0x044 + x * 0x40)
#define RIO_EM_PN_ATTRIB_CAP(x)		(0x048 + x * 0x40)
#define RIO_EM_PN_PKT_CAP_0(x)		(0x04c + x * 0x40)
#define RIO_EM_PN_PKT_CAP_1(x)		(0x050 + x * 0x40)
#define RIO_EM_PN_PKT_CAP_2(x)		(0x054 + x * 0x40)
#define RIO_EM_PN_PKT_CAP_3(x)		(0x058 + x * 0x40)
#define RIO_EM_PN_ERRRATE(x)		(0x068 + x * 0x40)
#define RIO_EM_PN_ERRRATE_TR(x)		(0x06c + x * 0x40)

/**
 * rio_config_read - Generate a RIO read maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Location to be read into
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_config_read(struct udevice *dev,
		    int  portid,
		    u16  destid,
		    u8   hopcount,
		    u32  offset,
		    int  len,
		    u32 *val);

/**
 * rio_config_write - Generate a RIO write maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Value to be written
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_config_write(struct udevice *dev,
		     int portid,
		     u16 destid,
		     u8  hopcount,
		     u32 offset,
		     int len,
		     u32 val);

/**
 * rio_local_config_read - RIO local config space read
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be read into
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_local_config_read(struct udevice *dev, u32 offset, int len, u32 *data);

/**
 * rio_local_config_write - RIO local config space write
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be written
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_local_config_write(struct udevice *dev, u32 offset, int len, u32 data);

/**
 * rio_doorbell_rx - Blocking wait for receiving doorbell info
 * @info: 16-bit value to wait for (implementation specific)
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_doorbell_rx(struct udevice *dev, u16 info);

/**
 * rio_remove - Remove a RapidIO device
 *
 * Returns %0 on success or %-1 on failure.
 */
int rio_remove(struct udevice *dev);

struct rio_ops {
	int (*config_read)(struct udevice *dev,
			   int portid, u16 destid, u8 hopcount,
			   u32 offset, int len, u32 *val);

	int (*config_write)(struct udevice *dev,
			    int portid, u16 destid, u8 hopcount,
			    u32 offset, int len, u32 val);

	int (*local_config_read)(struct udevice *dev,
				 u32 offset, int len, u32 *data);

	int (*local_config_write)(struct udevice *dev,
				  u32 offset, int len, u32 data);

	int (*doorbell_rx)(struct udevice *dev, u16 info);
};
#endif /* _RIO_H_ */
