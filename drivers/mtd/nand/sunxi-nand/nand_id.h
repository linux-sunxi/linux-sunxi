/*
 * nand_id.h
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

#ifndef _SUNXI_NAND_ID_H
#define _SUNXI_NAND_ID_H

struct nand_chip_param {
	unsigned char id[8];
	int id_len;
	int clock_freq; //the highest access frequence of the nand flash chip, based on MHz
	int ecc_mode;   //the Ecc Mode for the nand flash chip, 0: bch-16, 1:bch-28, 2:bch_32
};

struct nand_chip_param *sunxi_get_nand_chip_param(unsigned char mf);

#endif
