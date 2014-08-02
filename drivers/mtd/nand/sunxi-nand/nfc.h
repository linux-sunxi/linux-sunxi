/*
 * nfc.h
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

#ifndef _SUNXI_NAND_NFC_H
#define _SUNXI_NAND_NFC_H

void nfc_read_page1k(uint32_t page_addr, void *buff);
void nfc_write_page1k(uint32_t page_addr, void *buff);

int nfc_first_init(struct mtd_info *mtd);
int nfc_second_init(struct mtd_info *mtd);
void nfc_exit(struct mtd_info *mtd);

#endif
