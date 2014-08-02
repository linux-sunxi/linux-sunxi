/*
 * dma.c
 *
 * Copyright (C) 2013 Qiang Yu <yuq825@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mach/dma.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>

#include "defs.h"

static int nanddma_completed_flag = 1;
static DECLARE_WAIT_QUEUE_HEAD(DMA_wait);

static struct sw_dma_client nand_dma_client = {
	.name="NAND_DMA",
};

static void nanddma_buffdone(struct sw_dma_chan * ch, void *buf, int size, enum sw_dma_buffresult result)
{
	nanddma_completed_flag = 1;
	wake_up(&DMA_wait);
	//DBG_INFO("buffer done. nanddma_completed_flag: %d\n", nanddma_completed_flag);
}

static int nanddma_opfn(struct sw_dma_chan * ch,   enum sw_chan_op op_code)
{
	if(op_code == SW_DMAOP_START)
		nanddma_completed_flag = 0;

	//DBG_INFO("buffer opfn: %d, nanddma_completed_flag: %d\n", (int)op_code, nanddma_completed_flag);

	return 0;
}

int dma_nand_request(unsigned int dmatype)
{
	int ch;

	ch = sw_dma_request(DMACH_DNAND, &nand_dma_client, NULL);
	if(ch < 0)
		return ch;

	sw_dma_set_opfn(ch, nanddma_opfn);
	sw_dma_set_buffdone_fn(ch, nanddma_buffdone);

	return ch;
}

int dma_nand_release(int hDma)
{
	return sw_dma_free(hDma, &nand_dma_client);
}


static int dma_set(int hDMA, void *pArg)
{
	sw_dma_setflags(hDMA, SW_DMAF_AUTOSTART);
	return sw_dma_config(hDMA, (struct dma_hw_conf*)pArg);
}


static int dma_enqueue(int hDma, unsigned int buff_addr, size_t len)
{
	static int seq=0;
	__cpuc_flush_dcache_area((void *)buff_addr, len + (1 << 5) * 2 - 2);
	nanddma_completed_flag = 0;
	return sw_dma_enqueue(hDma, (void*)(seq++), buff_addr, len);
}

void dma_nand_config_start(int dma, int rw, unsigned int buff_addr, size_t len)
{
	struct dma_hw_conf nand_hwconf = {
		.xfer_type = DMAXFER_D_BWORD_S_BWORD,
		.hf_irq = SW_DMA_IRQ_FULL,
		.cmbk = 0x7f077f07,
	};

	nand_hwconf.dir = rw + 1;

	if(rw == 0) {
		nand_hwconf.from = 0x01C03030,
		nand_hwconf.address_type = DMAADDRT_D_LN_S_IO,
		nand_hwconf.drqsrc_type = DRQ_TYPE_NAND;
	}
	else {
		nand_hwconf.to = 0x01C03030,
		nand_hwconf.address_type = DMAADDRT_D_IO_S_LN,
		nand_hwconf.drqdst_type = DRQ_TYPE_NAND;
	}

	dma_set(dma, (void*)&nand_hwconf);
	dma_enqueue(dma, buff_addr, len);
}

int dma_nand_wait_finish(void)
{
	wait_event(DMA_wait, nanddma_completed_flag);
    return 0;
}

