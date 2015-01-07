/*
 * drivers/char/sunxi_mem/sunxi_physmem.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sunxi physical memory allocator driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include <mach/includes.h>
#include "sunxi_physmem_i.h"

#define	BUFFER_PADDR			SW_VE_MEM_BASE
#define	BUFFER_VADDR			BUFFER_PADDR
#define	BUFFER_SIZE			SW_VE_MEM_SIZE

#define	MEMORY_GAP_MIN      0x10000

#define IS_LIST_EMPTY(listh)            ((bool)((listh)->next == listh))
#define LIST_REACH_END(listh, entry)    ((bool)(listh == entry))
#define NEXT_PHYS_ADDR(pnode)           (pnode->phys_addr + pnode->size)

#define SW_VE_MEM_BASE                    (PLAT_PHYS_OFFSET + SZ_64M)
#define SW_VE_MEM_SIZE                    (SZ_64M)

static struct sunxi_mem_allocator	*g_allocator = NULL;
static DEFINE_SPINLOCK(sunxi_memlock);

inline static void __sunxi_init_list_head(struct mem_list * listh)
{
	listh->next = listh->prev = listh;
}

inline static struct mem_list * __sunxi_first_node(struct mem_list * listh)
{
	return listh->next;
}

inline static void __sunxi_remove_node(struct mem_list * pnode)
{
	pnode->prev->next = pnode->next;
	pnode->next->prev = pnode->prev;
	pnode->next = NULL;
	pnode->prev = NULL;
}

inline static void __sunxi_insert_node_before(struct mem_list * pnew_node, struct mem_list * pexisting_node)
{
	pexisting_node->prev->next = pnew_node;
	pnew_node->prev = pexisting_node->prev;
	pnew_node->next = pexisting_node;
	pexisting_node->prev = pnew_node;
}

static u32 sunxi_init(struct sunxi_mem_allocator *this, u32 size, u32 va, u32 pa)
{
	u32	uret = 0;
	struct mem_list * pnode;

	this->normal_size = size;

	this->node_free_listh = this->create_new_node(this, 0, 0, 0);
	if(NULL == this->node_free_listh) {
		uret = __LINE__;
		goto end;
	}
	__sunxi_init_list_head(this->node_free_listh);

	this->free_listh = this->create_new_node(this, 0, 0, 0);
	if(NULL == this->free_listh) {
		uret = __LINE__;
		goto end;
	}

	__sunxi_init_list_head(this->free_listh);

	this->inuse_listh = this->create_new_node(this, 0, 0, 0);
	if (NULL == this->inuse_listh) {
		uret = __LINE__;
		goto end;
	}

	__sunxi_init_list_head(this->inuse_listh);

	pnode = this->create_new_node(this, this->normal_size, va, pa);

	__sunxi_insert_node_before(pnode, __sunxi_first_node(this->free_listh));

end:
	if(0 != uret)
		SXM_ERR("%s err, line %d\n", __func__, uret);

	return uret;
}

static void sunxi_deinit(struct sunxi_mem_allocator *this)
{
	if(NULL != this->inuse_listh) {
		this->free_list(this, &this->inuse_listh);
		this->inuse_listh = NULL;
	}

	if(NULL != this->free_listh) {
		this->free_list(this, &this->free_listh);
		this->free_listh = NULL;
	}

	if(NULL != this->node_free_listh) {
		this->free_list(this, &this->node_free_listh);
		this->node_free_listh = NULL;
	}
}

static struct mem_list * sunxi_create_new_node(struct sunxi_mem_allocator *this,
			u32 size, u32 virt_addr, u32 phys_addr)
{
	struct mem_list * pnode = NULL;

	if((size == 0) || IS_LIST_EMPTY(this->node_free_listh))	{
		pnode = (struct mem_list *)kmalloc(sizeof(*pnode), GFP_ATOMIC);
		if(NULL == pnode) {
			SXM_ERR_FUN_LINE;
			return NULL;
		}
	} else {
		pnode = __sunxi_first_node(this->node_free_listh);
		__sunxi_remove_node(pnode);
	}

	if(pnode != NULL) {
		pnode->virt_addr = virt_addr;
		pnode->phys_addr = phys_addr;
		pnode->size 	= size;
		pnode->next 	= NULL;
		pnode->prev	= NULL;
	}

	return pnode;
}

static bool sunxi_delete_node(struct sunxi_mem_allocator *this, struct mem_list * pnode)
{
	__sunxi_insert_node_before(pnode, __sunxi_first_node(this->node_free_listh));
	return true;
}

static struct mem_list * sunxi_find_free_block(struct sunxi_mem_allocator *this, u32 size)
{
	struct mem_list * pnode = __sunxi_first_node(this->free_listh);

	while(!LIST_REACH_END(this->free_listh, pnode))	{
		if (size <= pnode->size) {
			__sunxi_remove_node(pnode);
			return (pnode);
		}
		pnode = pnode->next;
	}

	return (NULL);
}

static bool sunxi_add_node_to_freelist(struct sunxi_mem_allocator *this, struct mem_list * pnode)
{
	struct mem_list * pnode_temp = __sunxi_first_node(this->free_listh);

	struct mem_list * pnode_prev = NULL;
	struct mem_list * pnode_next = NULL;

	u32 cur_paddr = pnode->phys_addr;
	u32 next_paddr = NEXT_PHYS_ADDR(pnode);

	while(!LIST_REACH_END(this->free_listh, pnode_temp)) {
		if(cur_paddr == NEXT_PHYS_ADDR(pnode_temp))
			pnode_prev = pnode_temp;
		else if(next_paddr == pnode_temp->phys_addr)
			pnode_next = pnode_temp;

		if((pnode_prev == NULL) || (pnode_next == NULL))
			pnode_temp = pnode_temp->next;
		else
			break;
	}

	if(pnode_prev != NULL)	{
		__sunxi_remove_node(pnode_prev);

		pnode->size = pnode->size + pnode_prev->size;
		pnode->virt_addr = pnode_prev->virt_addr;
		pnode->phys_addr = pnode_prev->phys_addr;
		this->delete_node(this, pnode_prev);
	}

	if(pnode_next != NULL) {
		__sunxi_remove_node(pnode_next);

		pnode->size = pnode->size + pnode_next->size;
		this->delete_node(this, pnode_next);
	}

	pnode_temp = __sunxi_first_node(this->free_listh);

	while(!LIST_REACH_END(this->free_listh, pnode_temp)) {
		if(pnode->size <= pnode_temp->size)
			break;
		pnode_temp = pnode_temp->next;
	}

	__sunxi_insert_node_before(pnode, pnode_temp);
	return true;
}

static bool sunxi_free_list(struct sunxi_mem_allocator *this, struct mem_list **pphead)
{
	struct mem_list * pcur;
	struct mem_list * pnext;

	if(*pphead != NULL) {
		pcur = (*pphead)->next;
		while(pcur != *pphead) {
			pnext = pcur->next;
			kfree(pcur);
			pcur = pnext;
		}
		kfree(*pphead);
		*pphead = NULL;
	}

	return true;
}

bool sunxi_allocate(struct sunxi_mem_allocator *this, const u32 size_to_alloc,
		u32* const pvirt_adr, u32* const pphy_adr)
{
	u32 	size;
	struct mem_list *pnode = NULL;

	size = (size_to_alloc + (MEMORY_GAP_MIN - 1)) & (~(MEMORY_GAP_MIN - 1));
	pnode = this->find_free_block(this, size);
	if(pnode == NULL)
		return false;

	if(pnode->size - size >= MEMORY_GAP_MIN) {
		struct mem_list * pnew_node = this->create_new_node(this,
					pnode->size - size,
					pnode->virt_addr + size,
					pnode->phys_addr + size);
		this->add_node_to_freelist(this, pnew_node);

		pnode->size = size;
	}

	__sunxi_insert_node_before(pnode, __sunxi_first_node(this->inuse_listh));
	this->normal_size -= size;

	*pvirt_adr = pnode->virt_addr;
	*pphy_adr = pnode->phys_addr;
	return true;
}

void sunxi_free(struct sunxi_mem_allocator *this, const u32 virtAddr, const u32 physAddr)
{
	u32 dwsize;

	struct mem_list * pnode = __sunxi_first_node(this->inuse_listh);

	while(!LIST_REACH_END(this->inuse_listh, pnode)) {
		if((pnode->virt_addr ==  virtAddr) &&
			(pnode->phys_addr ==  physAddr)) {
			dwsize = pnode->size;
			__sunxi_remove_node(pnode);
			this->add_node_to_freelist(this, pnode);
			this->normal_size += dwsize;
			return;
		}
		pnode = pnode->next;
	}
}

int __init sunxi_mem_allocator_init(void)
{
	u32 	buf_size = BUFFER_SIZE;
	u32 	buf_vaddr = BUFFER_VADDR;
	u32 	buf_paddr = BUFFER_PADDR;

	g_allocator = kmalloc(sizeof(struct sunxi_mem_allocator), GFP_KERNEL);
	if(NULL == g_allocator) {
		SXM_ERR("%s err: out of memory, line %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	g_allocator->init 	= sunxi_init;
	g_allocator->deinit 	= sunxi_deinit;
	g_allocator->allocate 	= sunxi_allocate;
	g_allocator->free 	= sunxi_free;

	g_allocator->add_node_to_freelist = sunxi_add_node_to_freelist;
	g_allocator->delete_node 	= sunxi_delete_node;
	g_allocator->create_new_node 	= sunxi_create_new_node;
	g_allocator->find_free_block 	= sunxi_find_free_block;
	g_allocator->free_list 		= sunxi_free_list;

	if(0 != g_allocator->init(g_allocator, buf_size, buf_vaddr, buf_paddr)) {
		SXM_ERR("%s err, line %d, size 0x%08x, vaddr 0x%08x, paddr 0x%08x\n",
			__func__, __LINE__, buf_size, buf_vaddr, buf_paddr);
		return -ENOMEM;
	}

	SXM_DBG("%s success, line %d\n", __func__, __LINE__);
	return 0;
}
arch_initcall(sunxi_mem_allocator_init);

//bool sunxi_mem_alloc(u32 size, u32* virmem, u32* phymem)
unsigned int sunxi_mem_alloc(unsigned int size)
{
	u32	vtemp = 0, ptemp = 0;
	unsigned long	flags;

	spin_lock_irqsave(&sunxi_memlock, flags);
	if(NULL != g_allocator) {
		if(false == g_allocator->allocate(g_allocator, size, &vtemp, &ptemp)) {
			SXM_ERR("%s err, line %d, allocate failed!\n", __func__, __LINE__);
			ptemp = 0;
		}
	} else
		SXM_ERR("%s err, line %d, g_allocator not initailized yet!\n", __func__, __LINE__);
	spin_unlock_irqrestore(&sunxi_memlock, flags);

	SXM_DBG("%s: size 0x%08x, ret 0x%08x!\n", __func__, size, ptemp);
	return ptemp;
}
EXPORT_SYMBOL(sunxi_mem_alloc);

//void sunxi_mem_free(u32 virmem, u32 phymem)
void sunxi_mem_free(unsigned int phymem)
{
	u32	vtemp = phymem; /* to check */
	unsigned long	flags;

	SXM_DBG("%s: phymem 0x%08x!\n", __func__, phymem);

	spin_lock_irqsave(&sunxi_memlock, flags);
	if(NULL != g_allocator)
		g_allocator->free(g_allocator, vtemp, phymem);
	spin_unlock_irqrestore(&sunxi_memlock, flags);
}
EXPORT_SYMBOL(sunxi_mem_free);

u32 sunxi_mem_get_rest_size(void)
{
	u32 	ret = 0;
	unsigned long	flags;

	spin_lock_irqsave(&sunxi_memlock, flags);
	if(NULL != g_allocator)
		ret = g_allocator->normal_size;
	spin_unlock_irqrestore(&sunxi_memlock, flags);

	return ret;
}
EXPORT_SYMBOL(sunxi_mem_get_rest_size);

