/*
 * dma.h
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

#ifndef _SUNXI_NAND_DMA_H
#define _SUNXI_NAND_DMA_H

int dma_nand_request(unsigned int dmatype);
int dma_nand_release(int hDma);
void dma_nand_config_start(int dma, int rw, unsigned int buff_addr, size_t len);
int dma_nand_wait_finish(void);

#endif

