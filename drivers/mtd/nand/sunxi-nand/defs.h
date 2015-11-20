/*
 * defs.h
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

#ifndef _SUNXI_DEFS_H
#define _SUNXI_DEFS_H

#define PREFIX "[MTD][NAND][SUNXI]: "

#ifdef __LINUX__

#include <linux/kernel.h>

#define DBG_INFO(fmt, ...) printk(KERN_INFO PREFIX fmt, ##__VA_ARGS__)
#define ERR_INFO(fmt, ...) printk(KERN_ERR PREFIX fmt, ##__VA_ARGS__)

#else /* !__LINUX__ */

#define DBG_INFO(fmt, ...)
#define ERR_INFO(fmt, ...)

#endif

#endif
