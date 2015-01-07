/*
 * drivers/char/sunxi_mem/drv.c
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
#include <linux/mm_types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/sunxi_physmem.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <mach/includes.h>
#include "sunxi_physmem_i.h"
#include <linux/sunxi_physmem.h>

static struct cdev 	*g_cdev = NULL;
static dev_t 		g_devid = -1;
static struct class 	*g_class = NULL;
struct device 		*g_dev = NULL;
struct kmem_cache 	*g_pmem_cache = NULL; /* mem cache for struct sunxi_mem_des */

/* flush cache api from cache-v7.S */
extern int flush_dcache_all(void);
extern int flush_clean_user_range2(long start, long end);

/*
 * to sync sunmm_release and sunmm_ioctl, repetitive with SUNMM_LOCK, but any other better way?
 */
static DEFINE_MUTEX(sunmm_mutex);

int sunmm_mmap(struct file *file, struct vm_area_struct * vma)
{
	SXM_DBG("%s, vm_start 0x%08x, vm_end 0x%08x, vm_pgoff 0x%08x, vm_page_prot 0x%08x\n",
		__func__, (u32)vma->vm_start, (u32)vma->vm_end, (u32)vma->vm_pgoff, (u32)vma->vm_page_prot);

	//vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot); /* NOTE: lys commit, for performance, 2012-12-3 */
	//vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
		vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	SXM_DBG("%s success\n", __func__);
	return 0;
}

static int sunmm_open(struct inode *inode, struct file *file)
{
	struct sunxi_mem_data *pdata = NULL;

	pdata = (struct sunxi_mem_data *)kmalloc(sizeof(struct sunxi_mem_data), GFP_KERNEL);
	SUNMM_LOCK_INIT(&pdata->lock);
	INIT_LIST_HEAD(&pdata->list);

	file->private_data = (void *)pdata;
	return 0;
}

static int sunmm_release(struct inode *inode, struct file *file)
{
	u32 	phys_addr = 0;
	struct list_head *p = NULL, *n = NULL;
	struct sunxi_mem_des *pitem = NULL;
	struct sunxi_mem_data *pdata = NULL;
	DEFINE_FLAGS(flags);

	mutex_lock(&sunmm_mutex);

	/* free the buffer if there is any */
	pdata = (struct sunxi_mem_data *)(file->private_data);
	if(NULL != pdata && NULL != g_pmem_cache) {
		SUNMM_LOCK(&pdata->lock, flags);
		list_for_each_safe(p, n, &pdata->list) {
			pitem = list_entry(p, struct sunxi_mem_des, list);
			phys_addr = pitem->phys_addr;
			SXM_INF("%s: get un-freed phys_addr 0x%08x\n", __func__, phys_addr);
			/* remove item from list */
			list_del(&pitem->list);
			SUNMM_UNLOCK(&pdata->lock, flags);

			/* free item */
			kmem_cache_free(g_pmem_cache, (void *)pitem);
			/* free from reserved mem */
			sunxi_mem_free(phys_addr);

			SUNMM_LOCK(&pdata->lock, flags);
		}
		SUNMM_UNLOCK(&pdata->lock, flags);

		kfree(pdata);
		file->private_data = NULL;
	}

	mutex_unlock(&sunmm_mutex);
	return 0;
}

long sunmm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long	ret = 0;
	int 	temp = 0;
	u32 	size_to_alloc = 0;
	u32 	physaddr_to_free = 0;
	u32 	uphysaddr = 0;
	struct sunxi_mem_des *pdes = NULL;
	struct sunxi_mem_data *pdata = NULL;
	struct sunmm_cache_range cache_range = {0};
	DEFINE_FLAGS(flags);

	mutex_lock(&sunmm_mutex);

	pdata = (struct sunxi_mem_data *)file->private_data;

	switch(cmd) {
	case SUNXI_MEM_ALLOC:
		/* default return 0 indicate failed */
		ret = 0;
		/* check para */
		if(NULL == pdata) {
			SXM_ERR("%s err, line %d, private_data is NULL, cmd %d, arg %d\n", __func__,
				__LINE__, cmd, (u32)arg);
			goto end;
		}
		if(copy_from_user(&size_to_alloc, (u32 *)arg, sizeof(u32)) || NULL == g_pmem_cache) {
			SXM_ERR("%s err, line %d, size_to_alloc 0x%08x, g_pmem_cache 0x%08x\n",
				__func__, __LINE__, size_to_alloc, (u32)g_pmem_cache);
			goto end;
		}
		SXM_DBG("%s, SUNXI_MEM_ALLOC - get size_to_alloc 0x%08x\n", __func__, size_to_alloc);

		/* alloc from reserved mem */
		uphysaddr = sunxi_mem_alloc(size_to_alloc);
		if(0 == uphysaddr) {
			SXM_ERR("%s err, line %d, sunxi_mem_alloc failed, size_to_alloc %d\n", __func__, __LINE__, size_to_alloc);
			goto end;
		}
		/* alloc sunxi_mem_des struct */
		pdes = (struct sunxi_mem_des *)kmem_cache_alloc(g_pmem_cache, GFP_KERNEL);
		if(NULL == pdes) {
			SXM_ERR("%s err, line %d, kmem_cache_alloc failed\n", __func__, __LINE__);
			sunxi_mem_free(uphysaddr); /* release buf */
			goto end;
		}

		/* return phys address on success */
		ret = (long)uphysaddr;

		/* add buf to list */
		pdes->phys_addr = uphysaddr;
		pdes->size = size_to_alloc;
		SUNMM_LOCK(&pdata->lock, flags);
		list_add_tail(&pdes->list, &pdata->list);
		SUNMM_UNLOCK(&pdata->lock, flags);
		break;

	case SUNXI_MEM_FREE:
		/* check para */
		if(NULL == pdata) {
			SXM_ERR("%s err, line %d, private_data is NULL, cmd %d, arg %d\n", __func__,
				__LINE__, cmd, (u32)arg);
			ret = -EINVAL;
			goto end;
		}
		if(copy_from_user(&physaddr_to_free, (u32 *)arg, sizeof(u32)) || NULL == g_pmem_cache) {
			SXM_ERR("%s err, line %d, physaddr_to_free %d, g_pmem_cache 0x%08x\n", __func__, __LINE__,
				physaddr_to_free, (u32)g_pmem_cache);
			ret = -EINVAL;
			goto end;
		}
		SXM_DBG("%s, SUNXI_MEM_FREE - get physaddr_to_free 0x%08x\n", __func__, physaddr_to_free);

		/* find the sunxi_mem_des struct */
		{
			bool 	bfind = false;
			struct list_head *p = NULL, *n = NULL;
			struct sunxi_mem_des *pitem = NULL;

			SUNMM_LOCK(&pdata->lock, flags);
			list_for_each_safe(p, n, &pdata->list) {
				pitem = list_entry(p, struct sunxi_mem_des, list);
				if(pitem->phys_addr == physaddr_to_free) {
					/* remove item from list */
					list_del(&pitem->list);
					SUNMM_UNLOCK(&pdata->lock, flags);

					/* free item */
					kmem_cache_free(g_pmem_cache, (void *)pitem);
					/* free from reserved mem */
					sunxi_mem_free(physaddr_to_free);
					bfind = true;
					break;
				}
			}
			if(false == bfind) {
				SUNMM_UNLOCK(&pdata->lock, flags);
				SXM_ERR("%s err, line %d, cannot find the allocated mem 0x%08x\n",
					__func__, __LINE__, physaddr_to_free);
			}
		}
		/* return 0 on success */
		ret = 0;
		break;

	case SUNXI_MEM_GET_REST_SZ:
		ret = (long)sunxi_mem_get_rest_size();
		SXM_DBG("%s, SUNXI_MEM_GET_REST_SZ - ret 0x%08x\n", __func__, (int)ret);
		break;

	case SUNXI_MEM_FLUSH_CACHE:
		if(copy_from_user(&cache_range, (u32 *)arg, sizeof(cache_range))) {
			SXM_ERR("%s err, line %d\n", __func__, __LINE__);
			ret = -EINVAL;
			goto end;
		}
		SXM_DBG("%s, SUNXI_MEM_FLUSH_CACHE, start 0x%08x, end 0x%08x\n", __func__,
			cache_range.start, cache_range.end);
		temp = flush_clean_user_range2(cache_range.start, cache_range.end);
		if(0 != temp) {
			SXM_INF("%s, SUNXI_MEM_FLUSH_CACHE return %d\n", __func__, temp);
		}
		break;

	case SUNXI_MEM_FLUSH_CACHE_ALL:
		SXM_DBG("%s, SUNXI_MEM_FLUSH_CACHE_ALL\n", __func__);
		temp = flush_dcache_all();
		if(0 != temp) {
			SXM_INF("%s, SUNXI_MEM_FLUSH_CACHE_ALL return %d\n", __func__, temp);
		}
		break;

	default:
		SXM_ERR_FUN_LINE;
		ret = -EINVAL;
		goto end;
	}

end:
	mutex_unlock(&sunmm_mutex);
	return ret;
}

static struct file_operations sunxi_mem_fops = {
	.owner		= THIS_MODULE,
	.open		= sunmm_open,
	.release	= sunmm_release,
	.unlocked_ioctl	= sunmm_ioctl,
	.mmap		= sunmm_mmap,
};

static int sunmm_probe(struct platform_device *pdev)
{
	SXM_DBG_FUN_LINE;
	return 0;
}

static int sunmm_remove(struct platform_device *pdev)
{
	SXM_DBG_FUN_LINE;
	return 0;
}

struct platform_device sunxi_mem_device =
{
	.name		= "sunxi_mem",
	.id		= -1,
	//.num_resources  = ARRAY_SIZE(sunmm_resource),
	//.resources	= sunmm_resource,
};

static struct platform_driver sunxi_mem_driver =
{
	.probe		= sunmm_probe,
	.remove		= sunmm_remove,
	.driver		= {
		.name	= "sunxi_mem",
		.owner	= THIS_MODULE,
	},
};

/**
 * mem_cache_ctor - init function for g_pmem_cache
 * @p:	pointer to g_pdes_mgr
 */
static void mem_cache_ctor(void *p)
{
	struct sunxi_mem_des *pdes = (struct sunxi_mem_des *)p;

	memset(pdes, 0, sizeof(struct sunxi_mem_des));
	INIT_LIST_HEAD(&pdes->list);
}

int __init sunmm_module_init(void)
{
	int 	ret = 0;

	SXM_DBG("%s start, line %d\n", __func__, __LINE__);

	/* char device register */
	ret = alloc_chrdev_region(&g_devid, 0, 1, "sunxi_mem");
	if(ret) {
		SXM_ERR_FUN_LINE;
		return ret;
	}
	g_cdev = cdev_alloc();
	if(NULL == g_cdev) {
		SXM_ERR_FUN_LINE;
		goto out1;
	}
	cdev_init(g_cdev, &sunxi_mem_fops);
	g_cdev->owner = THIS_MODULE;
	ret = cdev_add(g_cdev, g_devid, 1);
	if(ret) {
		SXM_ERR_FUN_LINE;
		goto out2;
	}

	/* class create and device register */
	g_class = class_create(THIS_MODULE, "sunxi_mem");
	if(IS_ERR(g_class)) {
		SXM_ERR_FUN_LINE;
		goto out3;
	}
	g_dev = device_create(g_class, NULL, g_devid, NULL, "sunxi_mem");
	if(IS_ERR(g_dev)) {
		SXM_ERR_FUN_LINE;
		goto out4;
	}

	/* platform device register */
	ret = platform_device_register(&sunxi_mem_device);
	if(ret) {
		SXM_ERR_FUN_LINE;
		goto out5;
	}
	ret = platform_driver_register(&sunxi_mem_driver);
	if(ret) {
		SXM_ERR_FUN_LINE;
		goto out6;
	}

	/* alloc mem_des struct pool */
	g_pmem_cache = kmem_cache_create("sunxi_mem_des_cache", sizeof(struct sunxi_mem_des), 0,
					SLAB_HWCACHE_ALIGN, mem_cache_ctor);
	if(NULL == g_pmem_cache) {
		SXM_ERR_FUN_LINE;
		goto out7;
	}

	SXM_DBG("%s success, line %d\n", __func__, __LINE__);
	return ret;

out7:
	platform_driver_unregister(&sunxi_mem_driver);
out6:
	platform_device_unregister(&sunxi_mem_device);
out5:
	device_destroy(g_class, g_devid);
	g_dev = NULL;
out4:
	class_destroy(g_class);
	g_class = NULL;
out3:
	cdev_del(g_cdev);
out2:
	//kfree(g_cdev); /* not need? */
	g_cdev = NULL;
out1:
	unregister_chrdev_region(g_devid, 1);
	g_devid = -1;
	return -EINVAL; /* return err */
}

void __exit sunmm_module_exit(void)
{
	SXM_DBG("%s start\n", __func__);

	/* free mem_des struct pool */
	if(NULL != g_pmem_cache) {
		kmem_cache_destroy(g_pmem_cache);
		g_pmem_cache = NULL;
	}

	/* release platform drv and device */
	platform_driver_unregister(&sunxi_mem_driver);
	platform_device_unregister(&sunxi_mem_device);

	/* release class and  drv and device */
	device_destroy(g_class, g_devid);
	g_dev = NULL;

	class_destroy(g_class);
	g_class = NULL;

	/* release char dev */
	cdev_del(g_cdev);
	//kfree(g_cdev); /* not need? */
	g_cdev = NULL;

	unregister_chrdev_region(g_devid, 1);
	g_devid = -1;

	SXM_DBG("%s end\n", __func__);
}

module_init(sunmm_module_init);
module_exit(sunmm_module_exit);

