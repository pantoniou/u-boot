/*
 * Texas Instruments Keystone RapidIO driver
 * Authors: Aurelien Jacquiot <a-jacquiot@ti.com>
 *	    WingMan Kwok <w-kwok2@ti.com>
 *
 * This is the Rapidio driver for Keystone devices. This is
 * required to support RapidIO functionality on K2HK devices.
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/dma-mapping.h>
#include <asm/io.h>
#include <asm/arch/psc_defs.h>
#include <linux/compat.h>
#include <rio.h>

#include "keystone_rio.h"

#define DRIVER_VER		"v2.0"

#ifdef CONFIG_SOC_K2HK
#define KEYSTONE_RIO_IS_K2	1
#else
#define KEYSTONE_RIO_IS_K2	0
#endif

#define K2_SERDES(p)        ((p)->board_rio_cfg.keystone2_serdes)

static unsigned int rio_dbg;

#define debug_rio(fmt, args...)	if (rio_dbg) printf(fmt, ##args)

/*
 * Main KeyStone RapidIO driver data
 */
struct keystone_rio_data {
	struct udevice		*dev;
	int			riohdid;
	u32			rio_pe_feat;

	u32			ports_registering;
	u32			port_chk_cnt;

	u8			lsu_start;
	u8			lsu_end;
	u8			lsu_free;
	u8			lsu_maint;

	u32			*jtagid_reg;
	u32			*serdes_sts_reg;
	struct keystone_srio_serdes_regs	*serdes_regs;
	struct keystone_rio_regs		*regs;

	struct keystone_rio_car_csr_regs	*car_csr_regs;
	struct keystone_rio_serial_port_regs	*serial_port_regs;
	struct keystone_rio_err_mgmt_regs	*err_mgmt_regs;
	struct keystone_rio_phy_layer_regs	*phy_regs;
	struct keystone_rio_transport_layer_regs *transport_regs;
	struct keystone_rio_pkt_buf_regs	*pkt_buf_regs;
	struct keystone_rio_evt_mgmt_regs	*evt_mgmt_regs;
	struct keystone_rio_port_write_regs	*port_write_regs;
	struct keystone_rio_link_layer_regs	*link_regs;
	struct keystone_rio_fabric_regs		*fabric_regs;
	u32					car_csr_regs_base;
};

DECLARE_GLOBAL_DATA_PTR;

#define krio_write_reg(r, v)		writel(reg_val, reg)
#define krio_read_reg(r)		readl(reg)

#define krio_read(k, r)			readl(&k->regs->r)
#define krio_write(k, r, v)		writel((v), &k->regs->r)

#define krio_car_csr_read(k, r)		readl(&k->car_csr_regs->r)
#define krio_car_csr_write(k, r, v)	writel((v), &k->car_csr_regs->r)

#define krio_car_csr_read_ofs(k, ofs)		\
		readl((void *)k->car_csr_regs_base + ofs)
#define krio_car_csr_write_ofs(k, ofs, v)	\
		writel((v), (void *)(k->car_csr_regs_base + ofs))

#define krio_sp_read(k, r)		readl(&k->serial_port_regs->r)
#define krio_sp_write(k, r, v)		writel((v), &k->serial_port_regs->r)

#define krio_err_read(k, r)		readl(&k->err_mgmt_regs->r)
#define krio_err_write(k, r, v)		writel((v), &k->err_mgmt_regs->r)

#define krio_phy_read(k, r)		readl(&k->phy_regs->r)
#define krio_phy_write(k, r, v)		writel((v), &k->phy_regs->r)

#define krio_tp_read(k, r)		readl(&k->transport_regs->r)
#define krio_tp_write(k, r, v)		writel((v), &k->transport_regs->r)

#define krio_pb_read(k, r)		readl(&k->pkt_buf_regs->r)
#define krio_pb_write(k, r, v)		writel((v), &k->pkt_buf_regs->r)

#define krio_ev_read(k, r)		readl(&k->evt_mgmt_regs->r)
#define krio_ev_write(k, r, v)		writel((v), &k->evt_mgmt_regs->r)

#define krio_pw_read(k, r)		readl(&k->port_write_regs->r)
#define krio_pw_write(k, r, v)		writel((v), &k->port_write_regs->r)

#define krio_lnk_read(k, r)		readl(&k->link_regs->r)
#define krio_lnk_write(k, r, v)		writel((v), &k->link_regs->r)

#define krio_fab_read(k, r)		readl(&k->fabric_regs->r)
#define krio_fab_write(k, r, v)		writel((v), &k->fabric_regs->r)

#define krio_sd_read(k, r)		readl(&k->serdes_regs->r)
#define krio_sd_write(k, r, v)		writel((v), &k->serdes_regs->r)

/*--------------------- Maintenance Request Management  ---------------------*/

static u32 keystone_rio_dio_get_lsu_cc(u32 lsu_id, u8 ltid, u8 *lcb,
				       struct keystone_rio_data *krio_priv)
{
	u32 idx;
	u32 shift;
	u32 value;
	u32 cc;
	/* lSU shadow register status mapping */
	u32 lsu_index[8] = { 0, 9, 15, 20, 24, 33, 39, 44 };

	/* Compute LSU stat index from LSU id and LTID */
	idx   = (lsu_index[lsu_id] + ltid) >> 3;
	shift = ((lsu_index[lsu_id] + ltid) & 0x7) << 2;

	/* Get completion code and context */
	value  = krio_read(krio_priv, lsu_stat_reg[idx]);
	cc     = (value >> (shift + 1)) & 0x7;
	*lcb   = (value >> shift) & 0x1;

	return cc;
}

/**
 * maint_request - Perform a maintenance request
 * @port_id: output port ID of transaction
 * @dest_id: destination ID of target device
 * @hopcount: hopcount for this request
 * @offset: offset in the RapidIO configuration space
 * @buff: dma address of the data on the host
 * @buff_len: length of the data
 * @size: 1 for 16bit, 0 for 8bit ID size
 * @type: packet type
 *
 * Returns %0 on success or %-1 on failure.
 */
static inline int keystone_rio_maint_request(
	int port_id,
	u32 dest_id,
	u8  hopcount,
	u32 offset,
	dma_addr_t buff,
	int buff_len,
	u16 size,
	u16 type,
	struct keystone_rio_data *krio_priv)
{
	unsigned int count;
	unsigned int status = 0;
	unsigned int res    = 0;
	u8           context;
	u8           ltid;

	/* Check is there is space in the LSU shadow reg and that it is free */
	count = 0;
	while (1) {
		status = krio_read(krio_priv, lsu_reg[0].busy_full);
		if (((status & KEYSTONE_RIO_LSU_FULL_MASK) == 0x0) &&
		    ((status & KEYSTONE_RIO_LSU_BUSY_MASK) == 0x0))
			break;
		count++;

		if (count >= KEYSTONE_RIO_TIMEOUT_CNT) {
			debug_rio("RIO: no LSU available, status = 0x%x\n",
				  status);
			res = -1;
			goto out;
		}
		udelay(1);
	}

	/* Get LCB and LTID, LSU reg 6 is already read */
	context = (status >> 4) & 0x1;
	ltid    = status & 0xf;

	/* LSU Reg 0 - MSB of RapidIO address */
	krio_write(krio_priv, lsu_reg[0].addr_msb, 0);

	/* LSU Reg 1 - LSB of destination */
	krio_write(krio_priv, lsu_reg[0].addr_lsb_cfg_ofs, offset);

	/* LSU Reg 2 - source address */
	krio_write(krio_priv, lsu_reg[0].dsp_addr, buff);

	/* LSU Reg 3 - byte count */
	krio_write(krio_priv, lsu_reg[0].dbell_val_byte_cnt, buff_len);

	/* LSU Reg 4 - */
	krio_write(krio_priv, lsu_reg[0].destid,
		   ((port_id << 8)
		    | (KEYSTONE_RIO_LSU_PRIO << 4)
		    | (size ? BIT(10) : 0)
		    | ((u32)dest_id << 16)));

	/* LSU Reg 5 */
	krio_write(krio_priv, lsu_reg[0].dbell_info_fttype,
		   ((hopcount & 0xff) << 8) | (type & 0xff));

	/* Retrieve our completion code */
	count = 0;
	res   = 0;
	while (1) {
		u8 lcb;

		status = keystone_rio_dio_get_lsu_cc(0, ltid, &lcb, krio_priv);
		if (lcb == context)
			break;
		count++;
		if (count >= KEYSTONE_RIO_TIMEOUT_CNT) {
			debug_rio(
				"RIO: timeout %d, ltid = %d, context = %d, lcb = %d, cc = %d\n",
				count, ltid, context, lcb, status);
			res = -2;
			break;
		}
		udelay(1);
	}
out:
	if (res)
		return res;

	if (status)
		debug_rio("RIO: transfer error = 0x%x\n", status);

	switch (status) {
	case KEYSTONE_RIO_LSU_CC_TIMEOUT:
	case KEYSTONE_RIO_LSU_CC_XOFF:
	case KEYSTONE_RIO_LSU_CC_ERROR:
	case KEYSTONE_RIO_LSU_CC_INVALID:
	case KEYSTONE_RIO_LSU_CC_DMA:
		return -3;
	case KEYSTONE_RIO_LSU_CC_RETRY:
		return -4;
	case KEYSTONE_RIO_LSU_CC_CANCELED:
		return -5;
	default:
		break;
	}

	return 0;
}

/*------------------------- RapidIO hw controller setup ---------------------*/

struct keystone_lane_config {
	int start; /* lane start number of the port */
	int end;   /* lane end number of the port */
};

/*
 * Table with the various lanes per port configuration modes:
 * path mode 0: 4 ports in 1x
 * path mode 1: 3 ports in 2x/1x
 * path mode 2: 3 ports in 1x/2x
 * path mode 3: 2 ports in 2x
 * path mode 4: 1 ports in 4x
 */
static struct keystone_lane_config keystone_lane_configs[5][4] = {
	{ {0, 1}, {1, 2},   {2, 3},   {3, 4}   },
	{ {0, 2}, {-1, -1}, {2, 3},   {3, 4}   },
	{ {0, 1}, {1, 2},   {2, 4},   {-1, -1} },
	{ {0, 2}, {-1, -1}, {2, 4},   {-1, -1} },
	{ {0, 4}, {-1, -1}, {-1, -1}, {-1, -1} },
};

/* Retrieve the corresponding lanes bitmask from ports bitmask and path_mode */
static int keystone_rio_get_lane_config(u32 ports, u32 path_mode)
{
	u32 lanes = 0;

	while (ports) {
		u32 lane;
		u32 port = ffs(ports) - 1;

		ports &= ~BIT(port);

		if (keystone_lane_configs[path_mode][port].start == -1)
			return -1;

		for (lane = keystone_lane_configs[path_mode][port].start;
		     lane < keystone_lane_configs[path_mode][port].end;
		     lane++) {
			lanes |= BIT(lane);
		}
	}
	return (int)lanes;
}

/* Serdes Config Begin */
#define reg_fmkr(msb, lsb, val)					\
	(((val) & ((BIT((msb) - (lsb) + 1)) - 1)) << (lsb))

#define reg_finsr(addr, msb, lsb, val)					\
	writel(((readl(addr)						\
		 & ~(((BIT((msb) - (lsb) + 1)) - 1) << (lsb)))		\
		 | reg_fmkr(msb, lsb, val)), (addr))

static void k2_rio_serdes_init_3g(u32 lanes,
				  struct keystone_rio_data *krio_priv)
{
	void __iomem *reg = (void __iomem *)krio_priv->serdes_regs;

	reg_finsr((reg + 0x0000), 31, 24, 0x00);
	reg_finsr((reg + 0x0014),  7,  0, 0x82);
	reg_finsr((reg + 0x0014), 15,  8, 0x82);
	reg_finsr((reg + 0x0060),  7,  0, 0x48);
	reg_finsr((reg + 0x0060), 15,  8, 0x2c);
	reg_finsr((reg + 0x0060), 23, 16, 0x13);
	reg_finsr((reg + 0x0064), 15,  8, 0xc7);
	reg_finsr((reg + 0x0064), 23, 16, 0xc3);
	reg_finsr((reg + 0x0078), 15,  8, 0xc0);

	reg_finsr((reg + 0x0204),  7,  0, 0x80);
	reg_finsr((reg + 0x0204), 31, 24, 0x78);
	reg_finsr((reg + 0x0208),  7,  0, 0x24);
	reg_finsr((reg + 0x0208), 23, 16, 0x01);
	reg_finsr((reg + 0x020c), 31, 24, 0x02);
	reg_finsr((reg + 0x0210), 31, 24, 0x1b);
	reg_finsr((reg + 0x0214),  7,  0, 0x7c);
	reg_finsr((reg + 0x0214), 15,  8, 0x6e);
	reg_finsr((reg + 0x0218),  7,  0, 0xe4);
	reg_finsr((reg + 0x0218), 23, 16, 0x80);
	reg_finsr((reg + 0x0218), 31, 24, 0x7a);
	reg_finsr((reg + 0x022c), 15,  8, 0x08);
	reg_finsr((reg + 0x022c), 23, 16, 0x30);
	reg_finsr((reg + 0x0280),  7,  0, 0x70);
	reg_finsr((reg + 0x0280), 23, 16, 0x70);
	reg_finsr((reg + 0x0284),  7,  0, 0x85);
	reg_finsr((reg + 0x0284), 23, 16, 0x0f);
	reg_finsr((reg + 0x0284), 31, 24, 0x1d);
	reg_finsr((reg + 0x028c), 15,  8, 0x3b);

	reg_finsr((reg + 0x0404),  7,  0, 0x80);
	reg_finsr((reg + 0x0404), 31, 24, 0x78);
	reg_finsr((reg + 0x0408),  7,  0, 0x24);
	reg_finsr((reg + 0x0408), 23, 16, 0x01);
	reg_finsr((reg + 0x040c), 31, 24, 0x02);
	reg_finsr((reg + 0x0410), 31, 24, 0x1b);
	reg_finsr((reg + 0x0414),  7,  0, 0x7c);
	reg_finsr((reg + 0x0414), 15,  8, 0x6e);
	reg_finsr((reg + 0x0418),  7,  0, 0xe4);
	reg_finsr((reg + 0x0418), 23, 16, 0x80);
	reg_finsr((reg + 0x0418), 31, 24, 0x7a);
	reg_finsr((reg + 0x042c), 15,  8, 0x08);
	reg_finsr((reg + 0x042c), 23, 16, 0x30);
	reg_finsr((reg + 0x0480),  7,  0, 0x70);
	reg_finsr((reg + 0x0480), 23, 16, 0x70);
	reg_finsr((reg + 0x0484),  7,  0, 0x85);
	reg_finsr((reg + 0x0484), 23, 16, 0x0f);
	reg_finsr((reg + 0x0484), 31, 24, 0x1d);
	reg_finsr((reg + 0x048c), 15,  8, 0x3b);

	reg_finsr((reg + 0x0604),  7,  0, 0x80);
	reg_finsr((reg + 0x0604), 31, 24, 0x78);
	reg_finsr((reg + 0x0608),  7,  0, 0x24);
	reg_finsr((reg + 0x0608), 23, 16, 0x01);
	reg_finsr((reg + 0x060c), 31, 24, 0x02);
	reg_finsr((reg + 0x0610), 31, 24, 0x1b);
	reg_finsr((reg + 0x0614),  7,  0, 0x7c);
	reg_finsr((reg + 0x0614), 15,  8, 0x6e);
	reg_finsr((reg + 0x0618),  7,  0, 0xe4);
	reg_finsr((reg + 0x0618), 23, 16, 0x80);
	reg_finsr((reg + 0x0618), 31, 24, 0x7a);
	reg_finsr((reg + 0x062c), 15,  8, 0x08);
	reg_finsr((reg + 0x062c), 23, 16, 0x30);
	reg_finsr((reg + 0x0680),  7,  0, 0x70);
	reg_finsr((reg + 0x0680), 23, 16, 0x70);
	reg_finsr((reg + 0x0684),  7,  0, 0x85);
	reg_finsr((reg + 0x0684), 23, 16, 0x0f);
	reg_finsr((reg + 0x0684), 31, 24, 0x1d);
	reg_finsr((reg + 0x068c), 15,  8, 0x3b);

	reg_finsr((reg + 0x0804),  7,  0, 0x80);
	reg_finsr((reg + 0x0804), 31, 24, 0x78);
	reg_finsr((reg + 0x0808),  7,  0, 0x24);
	reg_finsr((reg + 0x0808), 23, 16, 0x01);
	reg_finsr((reg + 0x080c), 31, 24, 0x02);
	reg_finsr((reg + 0x0810), 31, 24, 0x1b);
	reg_finsr((reg + 0x0814),  7,  0, 0x7c);
	reg_finsr((reg + 0x0814), 15,  8, 0x6e);
	reg_finsr((reg + 0x0818),  7,  0, 0xe4);
	reg_finsr((reg + 0x0818), 23, 16, 0x80);
	reg_finsr((reg + 0x0818), 31, 24, 0x7a);
	reg_finsr((reg + 0x082c), 15,  8, 0x08);
	reg_finsr((reg + 0x082c), 23, 16, 0x30);
	reg_finsr((reg + 0x0880),  7,  0, 0x70);
	reg_finsr((reg + 0x0880), 23, 16, 0x70);
	reg_finsr((reg + 0x0884),  7,  0, 0x85);
	reg_finsr((reg + 0x0884), 23, 16, 0x0f);
	reg_finsr((reg + 0x0884), 31, 24, 0x1d);
	reg_finsr((reg + 0x088c), 15,  8, 0x3b);

	reg_finsr((reg + 0x0a00), 15,  8, 0x08);
	reg_finsr((reg + 0x0a08), 23, 16, 0x72);
	reg_finsr((reg + 0x0a08), 31, 24, 0x37);
	reg_finsr((reg + 0x0a30), 15,  8, 0x77);
	reg_finsr((reg + 0x0a30), 23, 16, 0x77);
	reg_finsr((reg + 0x0a84), 15,  8, 0x07);
	reg_finsr((reg + 0x0a94), 31, 24, 0x10);
	reg_finsr((reg + 0x0aa0), 31, 24, 0x81);
	reg_finsr((reg + 0x0abc), 31, 24, 0xff);
	reg_finsr((reg + 0x0ac0),  7,  0, 0x8b);
	reg_finsr((reg + 0x0a48), 15,  8, 0x8c);
	reg_finsr((reg + 0x0a48), 23, 16, 0xfd);
	reg_finsr((reg + 0x0a54),  7,  0, 0x72);
	reg_finsr((reg + 0x0a54), 15,  8, 0xec);
	reg_finsr((reg + 0x0a54), 23, 16, 0x2f);
	reg_finsr((reg + 0x0a58), 15,  8, 0x21);
	reg_finsr((reg + 0x0a58), 23, 16, 0xf9);
	reg_finsr((reg + 0x0a58), 31, 24, 0x00);
	reg_finsr((reg + 0x0a5c),  7,  0, 0x60);
	reg_finsr((reg + 0x0a5c), 15,  8, 0x00);
	reg_finsr((reg + 0x0a5c), 23, 16, 0x04);
	reg_finsr((reg + 0x0a5c), 31, 24, 0x00);
	reg_finsr((reg + 0x0a60),  7,  0, 0x00);
	reg_finsr((reg + 0x0a60), 15,  8, 0x80);
	reg_finsr((reg + 0x0a60), 23, 16, 0x00);
	reg_finsr((reg + 0x0a60), 31, 24, 0x00);
	reg_finsr((reg + 0x0a64),  7,  0, 0x20);
	reg_finsr((reg + 0x0a64), 15,  8, 0x12);
	reg_finsr((reg + 0x0a64), 23, 16, 0x58);
	reg_finsr((reg + 0x0a64), 31, 24, 0x0c);
	reg_finsr((reg + 0x0a68),  7,  0, 0x02);
	reg_finsr((reg + 0x0a68), 15,  8, 0x06);
	reg_finsr((reg + 0x0a68), 23, 16, 0x3b);
	reg_finsr((reg + 0x0a68), 31, 24, 0xe1);
	reg_finsr((reg + 0x0a6c),  7,  0, 0xc1);
	reg_finsr((reg + 0x0a6c), 15,  8, 0x4c);
	reg_finsr((reg + 0x0a6c), 23, 16, 0x07);
	reg_finsr((reg + 0x0a6c), 31, 24, 0xb8);
	reg_finsr((reg + 0x0a70),  7,  0, 0x89);
	reg_finsr((reg + 0x0a70), 15,  8, 0xe9);
	reg_finsr((reg + 0x0a70), 23, 16, 0x02);
	reg_finsr((reg + 0x0a70), 31, 24, 0x3f);
	reg_finsr((reg + 0x0a74),  7,  0, 0x01);
	reg_finsr((reg + 0x0b20), 23, 16, 0x37);
	reg_finsr((reg + 0x0b1c), 31, 24, 0x37);
	reg_finsr((reg + 0x0b20),  7,  0, 0x5d);
	reg_finsr((reg + 0x0000),  7,  0, 0x03);
	reg_finsr((reg + 0x0a00),  7,  0, 0x5f);
}

static void k2_rio_serdes_init_5g(u32 lanes,
				  struct keystone_rio_data *krio_priv)
{
	void __iomem *reg = (void __iomem *)krio_priv->serdes_regs;

	reg_finsr((reg + 0x0000), 31, 24, 0x00);
	reg_finsr((reg + 0x0014),  7,  0, 0x82);
	reg_finsr((reg + 0x0014), 15,  8, 0x82);
	reg_finsr((reg + 0x0060),  7,  0, 0x38);
	reg_finsr((reg + 0x0060), 15,  8, 0x24);
	reg_finsr((reg + 0x0060), 23, 16, 0x14);
	reg_finsr((reg + 0x0064), 15,  8, 0xc7);
	reg_finsr((reg + 0x0064), 23, 16, 0xc3);
	reg_finsr((reg + 0x0078), 15,  8, 0xc0);

	reg_finsr((reg + 0x0204),  7,  0, 0x80);
	reg_finsr((reg + 0x0204), 31, 24, 0x78);
	reg_finsr((reg + 0x0208),  7,  0, 0x26);
	reg_finsr((reg + 0x0208), 23, 16, 0x01);
	reg_finsr((reg + 0x020c), 31, 24, 0x02);
	reg_finsr((reg + 0x0214),  7,  0, 0x38);
	reg_finsr((reg + 0x0214), 15,  8, 0x6f);
	reg_finsr((reg + 0x0218),  7,  0, 0xe4);
	reg_finsr((reg + 0x0218), 23, 16, 0x80);
	reg_finsr((reg + 0x0218), 31, 24, 0x7a);
	reg_finsr((reg + 0x022c), 15,  8, 0x08);
	reg_finsr((reg + 0x022c), 23, 16, 0x30);
	reg_finsr((reg + 0x0280),  7,  0, 0x86);
	reg_finsr((reg + 0x0280), 23, 16, 0x86);
	reg_finsr((reg + 0x0284),  7,  0, 0x85);
	reg_finsr((reg + 0x0284), 23, 16, 0x0f);
	reg_finsr((reg + 0x0284), 31, 24, 0x1d);
	reg_finsr((reg + 0x028c), 15,  8, 0x2c);

	reg_finsr((reg + 0x0404),  7,  0, 0x80);
	reg_finsr((reg + 0x0404), 31, 24, 0x78);
	reg_finsr((reg + 0x0408),  7,  0, 0x26);
	reg_finsr((reg + 0x0408), 23, 16, 0x01);
	reg_finsr((reg + 0x040c), 31, 24, 0x02);
	reg_finsr((reg + 0x0414),  7,  0, 0x38);
	reg_finsr((reg + 0x0414), 15,  8, 0x6f);
	reg_finsr((reg + 0x0418),  7,  0, 0xe4);
	reg_finsr((reg + 0x0418), 23, 16, 0x80);
	reg_finsr((reg + 0x0418), 31, 24, 0x7a);
	reg_finsr((reg + 0x042c), 15,  8, 0x08);
	reg_finsr((reg + 0x042c), 23, 16, 0x30);
	reg_finsr((reg + 0x0480),  7,  0, 0x86);
	reg_finsr((reg + 0x0480), 23, 16, 0x86);
	reg_finsr((reg + 0x0484),  7,  0, 0x85);
	reg_finsr((reg + 0x0484), 23, 16, 0x0f);
	reg_finsr((reg + 0x0484), 31, 24, 0x1d);
	reg_finsr((reg + 0x048c), 15,  8, 0x2c);

	reg_finsr((reg + 0x0604),  7,  0, 0x80);
	reg_finsr((reg + 0x0604), 31, 24, 0x78);
	reg_finsr((reg + 0x0608),  7,  0, 0x26);
	reg_finsr((reg + 0x0608), 23, 16, 0x01);
	reg_finsr((reg + 0x060c), 31, 24, 0x02);
	reg_finsr((reg + 0x0614),  7,  0, 0x38);
	reg_finsr((reg + 0x0614), 15,  8, 0x6f);
	reg_finsr((reg + 0x0618),  7,  0, 0xe4);
	reg_finsr((reg + 0x0618), 23, 16, 0x80);
	reg_finsr((reg + 0x0618), 31, 24, 0x7a);
	reg_finsr((reg + 0x062c), 15,  8, 0x08);
	reg_finsr((reg + 0x062c), 23, 16, 0x30);
	reg_finsr((reg + 0x0680),  7,  0, 0x86);
	reg_finsr((reg + 0x0680), 23, 16, 0x86);
	reg_finsr((reg + 0x0684),  7,  0, 0x85);
	reg_finsr((reg + 0x0684), 23, 16, 0x0f);
	reg_finsr((reg + 0x0684), 31, 24, 0x1d);
	reg_finsr((reg + 0x068c), 15,  8, 0x2c);

	reg_finsr((reg + 0x0804),  7,  0, 0x80);
	reg_finsr((reg + 0x0804), 31, 24, 0x78);
	reg_finsr((reg + 0x0808),  7,  0, 0x26);
	reg_finsr((reg + 0x0808), 23, 16, 0x01);
	reg_finsr((reg + 0x080c), 31, 24, 0x02);
	reg_finsr((reg + 0x0814),  7,  0, 0x38);
	reg_finsr((reg + 0x0814), 15,  8, 0x6f);
	reg_finsr((reg + 0x0818),  7,  0, 0xe4);
	reg_finsr((reg + 0x0818), 23, 16, 0x80);
	reg_finsr((reg + 0x0818), 31, 24, 0x7a);
	reg_finsr((reg + 0x082c), 15,  8, 0x08);
	reg_finsr((reg + 0x082c), 23, 16, 0x30);
	reg_finsr((reg + 0x0880),  7,  0, 0x86);
	reg_finsr((reg + 0x0880), 23, 16, 0x86);
	reg_finsr((reg + 0x0884),  7,  0, 0x85);
	reg_finsr((reg + 0x0884), 23, 16, 0x0f);
	reg_finsr((reg + 0x0884), 31, 24, 0x1d);
	reg_finsr((reg + 0x088c), 15,  8, 0x2c);

	reg_finsr((reg + 0x0a00), 15,  8, 0x80);
	reg_finsr((reg + 0x0a08), 23, 16, 0xd2);
	reg_finsr((reg + 0x0a08), 31, 24, 0x38);
	reg_finsr((reg + 0x0a30), 15,  8, 0x8d);
	reg_finsr((reg + 0x0a30), 23, 16, 0x8d);
	reg_finsr((reg + 0x0a84), 15,  8, 0x07);
	reg_finsr((reg + 0x0a94), 31, 24, 0x10);
	reg_finsr((reg + 0x0aa0), 31, 24, 0x81);
	reg_finsr((reg + 0x0abc), 31, 24, 0xff);
	reg_finsr((reg + 0x0ac0),  7,  0, 0x8b);
	reg_finsr((reg + 0x0a48), 15,  8, 0x8c);
	reg_finsr((reg + 0x0a48), 23, 16, 0xfd);
	reg_finsr((reg + 0x0a54),  7,  0, 0x72);
	reg_finsr((reg + 0x0a54), 15,  8, 0xec);
	reg_finsr((reg + 0x0a54), 23, 16, 0x2f);
	reg_finsr((reg + 0x0a58), 15,  8, 0x21);
	reg_finsr((reg + 0x0a58), 23, 16, 0xf9);
	reg_finsr((reg + 0x0a58), 31, 24, 0x00);
	reg_finsr((reg + 0x0a5c),  7,  0, 0x60);
	reg_finsr((reg + 0x0a5c), 15,  8, 0x00);
	reg_finsr((reg + 0x0a5c), 23, 16, 0x04);
	reg_finsr((reg + 0x0a5c), 31, 24, 0x00);
	reg_finsr((reg + 0x0a60),  7,  0, 0x00);
	reg_finsr((reg + 0x0a60), 15,  8, 0x80);
	reg_finsr((reg + 0x0a60), 23, 16, 0x00);
	reg_finsr((reg + 0x0a60), 31, 24, 0x00);
	reg_finsr((reg + 0x0a64),  7,  0, 0x20);
	reg_finsr((reg + 0x0a64), 15,  8, 0x12);
	reg_finsr((reg + 0x0a64), 23, 16, 0x58);
	reg_finsr((reg + 0x0a64), 31, 24, 0x0c);
	reg_finsr((reg + 0x0a68),  7,  0, 0x02);
	reg_finsr((reg + 0x0a68), 15,  8, 0x06);
	reg_finsr((reg + 0x0a68), 23, 16, 0x3b);
	reg_finsr((reg + 0x0a68), 31, 24, 0xe1);
	reg_finsr((reg + 0x0a6c),  7,  0, 0xc1);
	reg_finsr((reg + 0x0a6c), 15,  8, 0x4c);
	reg_finsr((reg + 0x0a6c), 23, 16, 0x07);
	reg_finsr((reg + 0x0a6c), 31, 24, 0xb8);
	reg_finsr((reg + 0x0a70),  7,  0, 0x89);
	reg_finsr((reg + 0x0a70), 15,  8, 0xe9);
	reg_finsr((reg + 0x0a70), 23, 16, 0x02);
	reg_finsr((reg + 0x0a70), 31, 24, 0x3f);
	reg_finsr((reg + 0x0a74),  7,  0, 0x01);
	reg_finsr((reg + 0x0b20), 23, 16, 0x37);
	reg_finsr((reg + 0x0b1c), 31, 24, 0x37);
	reg_finsr((reg + 0x0b20),  7,  0, 0x5d);
	reg_finsr((reg + 0x0000),  7,  0, 0x03);
	reg_finsr((reg + 0x0a00),  7,  0, 0x5f);
}

static void k2_rio_serdes_lane_enable(u32 lane, u32 rate,
				      struct keystone_rio_data *krio_priv)
{
	void *regs = (void *)krio_priv->serdes_regs;
	u32 val;

	val = readl(regs + 0x200 * (lane + 1) + 0x28);
	val &= ~BIT(29);
	writel(val, regs + 0x200 * (lane + 1) + 0x28);

	switch (rate) {
	case KEYSTONE_RIO_FULL_RATE:
		writel(0xF0C0F0F0, regs + 0x1fe0 + 4 * lane);
		break;
	case KEYSTONE_RIO_HALF_RATE:
		writel(0xF4C0F4F0, regs + 0x1fe0 + 4 * lane);
		break;
	case KEYSTONE_RIO_QUARTER_RATE:
		writel(0xF8C0F8F0, regs + 0x1fe0 + 4 * lane);
		break;
	default:
		return;
	}
}

static int k2_rio_serdes_config(u32 lanes, u32 baud,
				struct keystone_rio_data *krio_priv)
{
	void *regs = (void *)krio_priv->serdes_regs;
	u32 rate;
	u32 val;

	writel(0x00000000, regs + 0x1ff4);

	switch (baud) {
	case KEYSTONE_RIO_BAUD_1_250:
		rate = KEYSTONE_RIO_QUARTER_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_2_500:
		rate = KEYSTONE_RIO_HALF_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_5_000:
		rate = KEYSTONE_RIO_FULL_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_3_125:
		rate = KEYSTONE_RIO_HALF_RATE;
		k2_rio_serdes_init_3g(lanes, krio_priv);
		break;
	default:
		printf("RIO: unsupported baud rate %d\n", baud);
		return -1;
	}

	while (lanes) {
		u32 lane = ffs(lanes) - 1;

		lanes &= ~BIT(lane);

		if (lane >= KEYSTONE_RIO_MAX_PORT)
			return -1;

		k2_rio_serdes_lane_enable(lane, rate, krio_priv);
	}

	writel(0xe0000000, regs + 0x1ff4);

	do {
		val = readl(regs + 0xbf8);
	} while (!(val & BIT(16)));

	return 0;
}

static int k2_rio_serdes_wait_lock(struct keystone_rio_data *krio_priv,
				   u32 lanes)
{
	u32 loop;
	u32 val;
	u32 val_mask;
	void *regs = (void *)krio_priv->serdes_regs;

	val_mask = lanes | (lanes << 8);

	for (loop = 0; loop < 100000; loop++) {
		val = readl(regs + 0x1ff4);
		if ((val & val_mask) == val_mask)
			break;
		udelay(10);
	}

	if (loop == 100000)
		return -1;

	return 0;
}

/* Serdes Config End */

/**
 * keystone_rio_hw_init - Configure a RapidIO controller
 * @mode: serdes configuration
 * @hostid: device id of the host
 */
static void keystone_rio_hw_init(u32 mode, u32 baud,
				 struct keystone_rio_data *krio_priv)
{
	struct keystone_rio_pdata *pdata = dev_get_platdata(krio_priv->dev);
	struct keystone_serdes_config *serdes_config = &pdata->serdes_config;
	u32 val;
	u32 block;

	/* Set sRIO out of reset */
	krio_write(krio_priv, pcr, 0x00000011);

	/* Clear BOOT_COMPLETE bit (allowing write) */
	krio_write(krio_priv, per_set_cntl, 0x00000000);

	/* Enable blocks */
	krio_write(krio_priv, gbl_en, 1);
	for (block = 0; block <= KEYSTONE_RIO_BLK_NUM; block++)
		krio_write(krio_priv, blk[block].enable, 1);

	/* Set control register 1 configuration */
	krio_write(krio_priv, per_set_cntl1, 0x00000000);

	/* Set Control register */
	krio_write(krio_priv, per_set_cntl, serdes_config->cfg_cntl);

	if (pdata->keystone2_serdes) {
		u32 path_mode = pdata->path_mode;
		u32 ports     = pdata->ports;
		int res;

		res = keystone_rio_get_lane_config(ports, path_mode);
		if (res > 0) {
			u32 lanes = (u32)res;

			res = k2_rio_serdes_config(lanes, baud, krio_priv);
		}
	} else {
		u32 port;

		krio_sd_write(krio_priv, pll, serdes_config->serdes_cfg_pll);

		for (port = 0; port < KEYSTONE_RIO_MAX_PORT; port++) {
			krio_sd_write(krio_priv, channel[port].rx,
				      serdes_config->rx_chan_config[port]);

			krio_sd_write(krio_priv, channel[port].tx,
				      serdes_config->tx_chan_config[port]);
		}

		do {
			val = readl(krio_priv->serdes_sts_reg);
		} while ((val & 0x1) != 0x1);
	}

	/* Set prescalar for ip_clk */
	krio_lnk_write(krio_priv, prescalar_srv_clk,
		       serdes_config->prescalar_srv_clk);

	/* Peripheral-specific configuration and capabilities */
	krio_car_csr_write(krio_priv, dev_id, KEYSTONE_RIO_DEV_ID_VAL);
	krio_car_csr_write(krio_priv, dev_info, KEYSTONE_RIO_DEV_INFO_VAL);
	krio_car_csr_write(krio_priv, assembly_id, KEYSTONE_RIO_ID_TI);
	krio_car_csr_write(krio_priv, assembly_info, KEYSTONE_RIO_EXT_FEAT_PTR);

	/* Set host device id */
	krio_car_csr_write(krio_priv, base_dev_id,
			   (krio_priv->riohdid & 0xffff)
			   | ((krio_priv->riohdid & 0xff) << 16));

	krio_priv->rio_pe_feat = RIO_PEF_PROCESSOR
		| RIO_PEF_CTLS
		| KEYSTONE_RIO_PEF_FLOW_CONTROL
		| RIO_PEF_EXT_FEATURES
		| RIO_PEF_ADDR_34
		| RIO_PEF_STD_RT
		| RIO_PEF_INB_DOORBELL
		| RIO_PEF_INB_MBOX;

	krio_car_csr_write(krio_priv, pe_feature, krio_priv->rio_pe_feat);

	krio_car_csr_write(krio_priv, sw_port, KEYSTONE_RIO_MAX_PORT << 8);

	krio_car_csr_write(krio_priv, src_op,
			   (RIO_SRC_OPS_READ
			    | RIO_SRC_OPS_WRITE
			    | RIO_SRC_OPS_STREAM_WRITE
			    | RIO_SRC_OPS_WRITE_RESPONSE
			    | RIO_SRC_OPS_DATA_MSG
			    | RIO_SRC_OPS_DOORBELL
			    | RIO_SRC_OPS_ATOMIC_TST_SWP
			    | RIO_SRC_OPS_ATOMIC_INC
			    | RIO_SRC_OPS_ATOMIC_DEC
			    | RIO_SRC_OPS_ATOMIC_SET
			    | RIO_SRC_OPS_ATOMIC_CLR
			    | RIO_SRC_OPS_PORT_WRITE));

	krio_car_csr_write(krio_priv, dest_op,
			   (RIO_DST_OPS_READ
			    | RIO_DST_OPS_WRITE
			    | RIO_DST_OPS_STREAM_WRITE
			    | RIO_DST_OPS_WRITE_RESPONSE
			    | RIO_DST_OPS_DATA_MSG
			    | RIO_DST_OPS_DOORBELL
			    | RIO_DST_OPS_PORT_WRITE));

	krio_car_csr_write(krio_priv, pe_logical_ctl, RIO_PELL_ADDR_34);

	val = (((KEYSTONE_RIO_SP_HDR_NEXT_BLK_PTR & 0xffff) << 16) |
	       KEYSTONE_RIO_SP_HDR_EP_REC_ID);
	krio_sp_write(krio_priv, sp_maint_blk_hdr, val);

	/* clear high bits of local config space base addr */
	krio_car_csr_write(krio_priv, local_cfg_hbar, 0x00000000);

	/* set local config space base addr */
	krio_car_csr_write(krio_priv, local_cfg_bar, 0x00520000);

	/* Enable HOST & MASTER_ENABLE bits */
	krio_sp_write(krio_priv, sp_gen_ctl, 0xe0000000);

	/* set link timeout value */
	krio_sp_write(krio_priv, sp_link_timeout_ctl, 0x000FFF00);

	/* set response timeout value */
	krio_sp_write(krio_priv, sp_rsp_timeout_ctl, 0x000FFF00);

	/* allows SELF_RESET and PWDN_PORT resets to clear sticky reg bits */
	krio_lnk_write(krio_priv, reg_rst_ctl, 0x00000001);

	/* Set error detection mode */
	/* clear all errors */
	krio_err_write(krio_priv, err_det, 0x00000000);

	/* enable all error detection */
	krio_err_write(krio_priv, err_en, 0x00000000);

	/* set err det block header */
	val = (((KEYSTONE_RIO_ERR_HDR_NEXT_BLK_PTR & 0xffff) << 16) |
	       KEYSTONE_RIO_ERR_EXT_FEAT_ID);
	krio_err_write(krio_priv, err_report_blk_hdr, val);

	/* clear msb of err catptured addr reg */
	krio_err_write(krio_priv, h_addr_capt, 0x00000000);

	/* clear lsb of err catptured addr reg */
	krio_err_write(krio_priv, addr_capt, 0x00000000);

	/* clear err catptured source and dest devID reg */
	krio_err_write(krio_priv, id_capt, 0x00000000);

	/* clear err catptured packet info */
	krio_err_write(krio_priv, ctrl_capt, 0x00000000);

	/* Force all writes to finish */
	val = krio_err_read(krio_priv, ctrl_capt);
}

/**
 * keystone_rio_start - Start RapidIO controller
 */
static void keystone_rio_start(struct keystone_rio_data *krio_priv)
{
	u32 val;

	/* set PEREN bit to enable logical layer data flow */
	val = (KEYSTONE_RIO_PER_EN | KEYSTONE_RIO_PER_FREE);
	krio_write(krio_priv, pcr, val);

	/* set BOOT_COMPLETE bit */
	val = krio_read(krio_priv, per_set_cntl);
	krio_write(krio_priv, per_set_cntl, val | KEYSTONE_RIO_BOOT_COMPLETE);
}

/**
 * keystone_rio_port_status - Return if the port is OK or not
 * @port: index of the port
 *
 * Return %0 if the port is ready or %-EIO on failure.
 */
static int keystone_rio_port_status(int port,
				    struct keystone_rio_data *krio_priv)
{
	unsigned int count, value;
	int solid_ok = 0;

	if (port >= KEYSTONE_RIO_MAX_PORT)
		return -1;

	/* Check port status */
	for (count = 0; count < 300; count++) {
		value = krio_sp_read(krio_priv, sp[port].err_stat);

		if (value & RIO_PORT_N_ERR_STS_PORT_OK) {
			solid_ok++;
			if (solid_ok == 100)
				break;
		} else {
			if (solid_ok) {
				debug_rio(
					"RIO: unstable port %d (solid_ok = %d)\n",
					port, solid_ok);
				return -2;
			}
			solid_ok = 0;
		}
		udelay(20);
	}

	return 0;
}

/**
 * keystone_rio_port_disable - Disable a RapidIO port
 * @port: index of the port to configure
 */
static void keystone_rio_port_disable(u32 port,
				      struct keystone_rio_data *krio_priv)
{
	/* Disable port */
	krio_sp_write(krio_priv, sp[port].ctl, 0x800000);
}

/**
 * keystone_rio_port_init - Configure a RapidIO port
 * @port: index of the port to configure
 * @mode: serdes configuration
 */
static int keystone_rio_port_init(u32 port, u32 path_mode,
				  struct keystone_rio_data *krio_priv)
{
	if (port >= KEYSTONE_RIO_MAX_PORT)
		return -1;

	/* Disable packet forwarding */
	krio_write(krio_priv, pkt_fwd_cntl[port].pf_16b, 0xffffffff);
	krio_write(krio_priv, pkt_fwd_cntl[port].pf_8b, 0x0003ffff);

	/* Silence and discovery timers */
	if ((port == 0) || (port == 2)) {
		krio_phy_write(krio_priv, phy_sp[port].silence_timer,
			       0x20000000);
		krio_phy_write(krio_priv, phy_sp[port].discovery_timer,
			       0x20000000);
	}

	/* Enable port in input and output */
	krio_sp_write(krio_priv, sp[port].ctl, 0x600000);

	/* Program channel allocation to ports (1x, 2x or 4x) */
	krio_phy_write(krio_priv, phy_sp[port].path_ctl, path_mode);

	return 0;
}

/**
 * keystone_rio_port_activate - Start using a RapidIO port
 * @port: index of the port to configure
 */
static int keystone_rio_port_activate(u32 port,
				      struct keystone_rio_data *krio_priv)
{
	/* Cleanup port error status */
	krio_sp_write(krio_priv, sp[port].err_stat,
		      KEYSTONE_RIO_PORT_ERROR_MASK);

	krio_err_write(krio_priv, sp_err[port].det, 0);

	/* Enable promiscuous */
	krio_tp_write(krio_priv, transport_sp[port].control, 0x00309000);

	return 0;
}

/*------------------------ Main driver functions -----------------------*/

/**
 * keystone_rio_config_read - Generate a RIO read maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Location to be read into
 *
 * Returns %0 on success or %-1 on failure.
 */
int keystone_rio_config_read(struct udevice *dev, int portid, u16 destid,
			     u8 hopcount, u32 offset, int len, u32 *val)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);
	struct keystone_rio_pdata *pdata = dev_get_platdata(dev);
	dma_addr_t dma;
	u32 *tbuf;
	int res;

	tbuf = malloc(len);
	if (!tbuf)
		return -1;

	memset(tbuf, 0, len);

	dma = dma_map_single(tbuf, len, DMA_FROM_DEVICE);

	res = keystone_rio_maint_request(portid, destid, hopcount, offset, dma,
					 len, pdata->size,
					 KEYSTONE_RIO_PACKET_TYPE_MAINT_R,
					 krio_priv);

	dma_unmap_single((void *)tbuf, len, dma);

	/* Taking care of byteswap */
	switch (len) {
	case 1:
		*val = *((u8 *)tbuf);
		break;
	case 2:
		*val = ntohs(*((u16 *)tbuf));
		break;
	default:
		*val = ntohl(*((u32 *)tbuf));
		break;
	}

	free(tbuf);

	debug_rio("RIO: %s portid %d destid %d hopcount %d offset 0x%x len %d val 0x%x res %d\n",
		  __func__, portid, destid, hopcount, offset, len, *val,
		res);

	return res;
}

/**
 * keystone_rio_config_write - Generate a RIO write maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Value to be written
 *
 * Returns %0 on success or %-1 on failure.
 */
int keystone_rio_config_write(struct udevice *dev, int portid, u16 destid,
			      u8 hopcount, u32 offset, int len, u32 val)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);
	struct keystone_rio_pdata *pdata = dev_get_platdata(dev);
	u32 *tbuf;
	int res;
	dma_addr_t dma;

	tbuf = malloc(len);
	if (!tbuf)
		return -1;

	memset(tbuf, 0, len);

	/* Taking care of byteswap */
	switch (len) {
	case 1:
		*tbuf = ((u8)val);
		break;
	case 2:
		*tbuf = htons((u16)val);
		break;
	default:
		*tbuf = htonl((u32)val);
		break;
	}

	dma = dma_map_single(tbuf, len, DMA_TO_DEVICE);

	res = keystone_rio_maint_request(portid, destid, hopcount, offset, dma,
					 len, pdata->size,
					 KEYSTONE_RIO_PACKET_TYPE_MAINT_W,
					 krio_priv);

	dma_unmap_single((void *)tbuf, len, dma);

	debug_rio("RIO: %s portid %d destid %d hopcount %d offset 0x%x len %d val 0x%x res %d\n",
		  __func__, portid, destid, hopcount, offset, len, val, res);

	free(tbuf);

	return res;
}

/**
 * keystone_rio_local_config_read - RIO local config space read
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be read into
 *
 * Returns %0 on success or %-1 on failure.
 */
int keystone_rio_local_config_read(struct udevice *dev, u32 offset,
				   int len, u32 *data)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);

	*data = krio_car_csr_read_ofs(krio_priv, offset);

	debug_rio("RIO: %s offset 0x%x data 0x%x\n",
		  __func__, offset, *data);

	return 0;
}

/**
 * keystone_rio_local_config_write - RIO local config space write
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be written
 *
 * Returns %0 on success or %-1 on failure.
 */
int keystone_rio_local_config_write(struct udevice *dev, u32 offset,
				    int len, u32 data)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);

	debug_rio("RIO: %s offset 0x%x data 0x%x\n",
		  __func__, offset, data);

	krio_car_csr_write_ofs(krio_priv, offset, data);
	return 0;
}

/**
 * keystone_rio_doorbell_rx - Blocking wait to receive doorbell info
 * @info: doorbell info to wait on
 *
 * Returns %0 on success or %-1 on failure.
 */
int keystone_rio_doorbell_rx(struct udevice *dev, u16 info)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);
	u32 pending_dbell;
	u16 db_bit_mask, start_db, end_db;
	unsigned int i, received = 0;

	if (info == KEYSTONE_RIO_DBELL_INFO_ANY) {
		start_db = 0;
		end_db = KEYSTONE_RIO_DBELL_NUMBER - 1;
		db_bit_mask = 0xf;
		printf("%s: waiting for dbell any ...\n", __func__);
	} else {
		start_db = (info >> 4) & 0x3;
		end_db = start_db;
		db_bit_mask = BIT(info & 0xf);
		if (start_db >= KEYSTONE_RIO_DBELL_NUMBER) {
			printf("Error: doorbell out of range. Invalid info: 0x%04x\n",
			       info);
			return -1;
		}
		printf("%s: waiting for dbell %u bit %u ...\n",
		       __func__, start_db, info & 0xf);
	}

	while (1) {
		for (i = start_db; i < end_db + 1; i++) {
			pending_dbell = krio_read(krio_priv,
						  doorbell_int[i].status);
			if (pending_dbell) {
				if (pending_dbell & db_bit_mask) {
					received = 1;
					printf("received dbell[%d] status: 0x%08x\n",
					       i, pending_dbell);
				}

				/* Acknowledge the interrupts for
				 * these doorbells
				 */
				krio_write(krio_priv, doorbell_int[i].clear,
					   pending_dbell);
			}
		}

		if (received)
			return 0;

		mdelay(10);
	}
}

/*
 * Platform configuration setup
 */
static int keystone_rio_setup_controller(struct keystone_rio_data *krio_priv)
{
	struct keystone_rio_pdata *pdata = dev_get_platdata(krio_priv->dev);
	u32 ports;
	u32 p;
	u32 mode;
	u32 baud;
	u32 path_mode;
	u32 size = 0;
	int res = 0;
	char str[8];

	size      = pdata->size;
	ports     = pdata->ports;
	baud      = pdata->serdes_baudrate;
	path_mode = pdata->path_mode;

	debug_rio(
		"RIO: size = %d, ports = 0x%x, baud = %d, path_mode = %d\n",
		size, ports, baud, path_mode);

	if (baud > KEYSTONE_RIO_BAUD_5_000) {
		baud = KEYSTONE_RIO_BAUD_5_000;
		printf("RIO: invalid baud rate, forcing it to 5Gbps\n");
	}

	switch (baud) {
	case KEYSTONE_RIO_BAUD_1_250:
		snprintf(str, sizeof(str), "1.25");
		break;
	case KEYSTONE_RIO_BAUD_2_500:
		snprintf(str, sizeof(str), "2.50");
		break;
	case KEYSTONE_RIO_BAUD_3_125:
		snprintf(str, sizeof(str), "3.125");
		break;
	case KEYSTONE_RIO_BAUD_5_000:
		snprintf(str, sizeof(str), "5.00");
		break;
	default:
		return -1;
	}

	debug_rio("RIO: initializing %s Gbps interface with port config %d\n",
		  str, path_mode);

	/* Hardware set up of the controller */
	keystone_rio_hw_init(mode, baud, krio_priv);

	/* Disable all ports */
	for (p = 0; p < KEYSTONE_RIO_MAX_PORT; p++)
		keystone_rio_port_disable(p, krio_priv);

	/* Start the controller */
	keystone_rio_start(krio_priv);

	/* Try to lock K2 SerDes*/
	if (pdata->keystone2_serdes) {
		int lanes = keystone_rio_get_lane_config(ports, path_mode);

		if (lanes > 0) {
			res = k2_rio_serdes_wait_lock(krio_priv, (u32)lanes);
			if (res < 0)
				debug_rio("SerDes for lane mask 0x%x on %s Gbps not locked\n",
					  lanes, str);
		}
	}

	/* Use and check ports status (but only the requested ones) */
	krio_priv->ports_registering = ports;
	while (ports) {
		int status;
		u32 port = ffs(ports) - 1;

		if (port > 32)
			return 0;

		ports &= ~BIT(port);

		res = keystone_rio_port_init(port, path_mode, krio_priv);
		if (res < 0) {
			printf("RIO: initialization of port %d failed\n", p);
			return res;
		}

		/* Start the port */
		keystone_rio_port_activate(port, krio_priv);

		/* Check the port status */
		status = keystone_rio_port_status(port, krio_priv);
		if (status == 0) {
			krio_priv->ports_registering &= ~BIT(port);
			debug_rio("RIO: port RIO%d ready\n", port);
		} else {
			printf("RIO: port %d not ready (status %d)\n",
			       port, status);
		}
	}

	if (krio_priv->ports_registering != 0)
		return -1;

	return res;
}

static int keystone_rio_probe(struct udevice *dev)
{
	struct keystone_rio_data *krio_priv = dev_get_priv(dev);
	struct keystone_rio_pdata *pdata = dev_get_platdata(dev);
	void *regs;

	krio_priv->dev = dev;

	regs = (void *)pdata->boot_cfg_regs_base;
	krio_priv->jtagid_reg     = regs + 0x0018;
	krio_priv->serdes_sts_reg = regs + 0x0154;

	regs = (void *)pdata->serdes_cfg_regs_base;
	krio_priv->serdes_regs = regs;

	regs = (void *)pdata->rio_regs_base;
	krio_priv->regs			= regs;
	krio_priv->car_csr_regs		= regs + 0x0b000;
	krio_priv->serial_port_regs	= regs + 0x0b100;
	krio_priv->err_mgmt_regs	= regs + 0x0c000;
	krio_priv->phy_regs		= regs + 0x1b000;
	krio_priv->transport_regs	= regs + 0x1b300;
	krio_priv->pkt_buf_regs		= regs + 0x1b600;
	krio_priv->evt_mgmt_regs	= regs + 0x1b900;
	krio_priv->port_write_regs	= regs + 0x1ba00;
	krio_priv->link_regs		= regs + 0x1bd00;
	krio_priv->fabric_regs		= regs + 0x1be00;
	krio_priv->car_csr_regs_base	= (u32)regs + 0xb000;

	krio_priv->riohdid = pdata->riohdid;
	krio_priv->lsu_start = 0;
	krio_priv->lsu_end   = 0;

	/* Enable srio clock */
	psc_enable_module(KS2_LPSC_SRIO);

	debug_rio("KeyStone RapidIO driver %s, hdid=%d\n",
		  DRIVER_VER, krio_priv->riohdid);

	/* Setup the sRIO controller */
	return keystone_rio_setup_controller(krio_priv);
}

/**
 * keystone_rio_remove - Shutdown RapidIO subsystem
 */
static int keystone_rio_remove(struct udevice *dev)
{
	/* Power off */
	psc_disable_module(KS2_LPSC_SRIO);
	return 0;
}

static const struct rio_ops keystone_rio_ops = {
	.config_read		= keystone_rio_config_read,
	.config_write		= keystone_rio_config_write,
	.local_config_read	= keystone_rio_local_config_read,
	.local_config_write	= keystone_rio_local_config_write,
	.doorbell_rx		= keystone_rio_doorbell_rx,
};

static int keystone_rio_ofdata_to_platdata(struct udevice *dev)
{
	struct keystone_rio_pdata *pdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	fdt_addr_t addr;
	int i, node = dev->of_offset;

	addr = dev_get_addr_name(dev, "rio");
	if (addr == FDT_ADDR_T_NONE) {
		debug_rio("Can't get the RIO register base address\n");
		return -ENXIO;
	}
	pdata->rio_regs_base = (u32)addr;

	addr = dev_get_addr_name(dev, "boot_config");
	if (addr == FDT_ADDR_T_NONE) {
		debug_rio("Can't get the Boot Conig register base address\n");
		return -ENXIO;
	}
	pdata->boot_cfg_regs_base = (u32)addr;

	addr = dev_get_addr_name(dev, "serdes");
	if (addr == FDT_ADDR_T_NONE) {
		debug_rio("Can't get the Serdes register base address\n");
		return -ENXIO;
	}
	pdata->serdes_cfg_regs_base = (u32)addr;

	if (fdt_get_property(fdt, node, "keystone2-serdes", NULL))
		pdata->keystone2_serdes = KEYSTONE_RIO_IS_K2;

	pdata->riohdid = fdtdec_get_int(fdt, node, "host-id", -1);

	/* defaults to small system size if not specified */
	pdata->size = (u16)fdtdec_get_uint(fdt, node, "dev-id-size", 0);

	/* port(s) to probe defaults to 0x1 if not specified */
	pdata->ports = (u16)fdtdec_get_uint(fdt, node, "ports", 1);

	/* defaults to 5 Gbps if not specified */
	pdata->serdes_baudrate = fdtdec_get_uint(fdt, node, "baudrate",
						 KEYSTONE_RIO_BAUD_5_000);

	/* defaults to 1-4x port if not specified */
	pdata->path_mode = fdtdec_get_uint(fdt, node, "path-mode",
					   KEYSTONE_RIO_MAX_PORTS_PATH_MODE_4);

	if (pdata->keystone2_serdes) {
		/*
		 * K2 sRIO config 0
		 */
		pdata->serdes_config.prescalar_srv_clk = 0x001f;
	} else {
		/*
		 * K1 sRIO config 0: MPY = 5x, div rate = half,
		 * link rate = 3.125 Gbps, mode 1x
		 */

		/* setting control register config */
		pdata->serdes_config.cfg_cntl = 0x0c053860;

		/* SerDes PLL configuration */
		pdata->serdes_config.serdes_cfg_pll = 0x0229;

		/* prescalar_srv_clk */
		pdata->serdes_config.prescalar_srv_clk = 0x001e;

		/* serdes rx_chan_config */
		for (i = 0; i < KEYSTONE_RIO_MAX_PORT; i++)
			pdata->serdes_config.rx_chan_config[i] = 0x00440495;

		/* serdes tx_chan_config */
		for (i = 0; i < KEYSTONE_RIO_MAX_PORT; i++)
			pdata->serdes_config.tx_chan_config[i] = 0x00180795;
	}

	return 0;
}

static const struct udevice_id keystone_rio_ids[] = {
	{ .compatible = "ti,keystone-rapidio" },
	{ }
};

U_BOOT_DRIVER(rio_keystone) = {
	.name				= "rio_keystone",
	.id				= UCLASS_RIO,
	.of_match			= keystone_rio_ids,
	.ofdata_to_platdata		= keystone_rio_ofdata_to_platdata,
	.ops				= &keystone_rio_ops,
	.probe				= keystone_rio_probe,
	.remove				= keystone_rio_remove,
	.priv_auto_alloc_size		= sizeof(struct keystone_rio_data),
	.platdata_auto_alloc_size	= sizeof(struct keystone_rio_pdata),
};
