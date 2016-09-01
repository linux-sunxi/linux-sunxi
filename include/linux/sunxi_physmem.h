/*
 * include/linux/sunxi_physmem.h
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sunxi physical memory allocator head file
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __SUNXI_PHYSMEM_H
#define __SUNXI_PHYSMEM_H

#define SUNXI_MEM_ALLOC 		1
#define SUNXI_MEM_FREE 			3 /* cannot be 2, which reserved in linux */
#define SUNXI_MEM_GET_REST_SZ 		4
#define SUNXI_MEM_FLUSH_CACHE 		5
#define SUNXI_MEM_FLUSH_CACHE_ALL	6

/* cache range for SUNXI_MEM_FLUSH_CACHE */
struct sunmm_cache_range{
	long 	start;
	long 	end;
};

//bool sunxi_mem_alloc(u32 size, u32* phymem);
u32 sunxi_mem_alloc(u32 size);
void sunxi_mem_free(u32 phymem);
u32 sunxi_mem_get_rest_size(void);

#endif /* __SUNXI_PHYSMEM_H */
