/*
 *    RoboPeak USB LCD Display Linux Driver
 *    
 *    Copyright (C) 2009 - 2013 RoboPeak Team
 *    This file is licensed under the GPL. See LICENSE in the package.
 *
 *    http://www.robopeak.net
 *
 *    Author Shikai Chen
 *
 *  ----------------------------------------------------------------------
 *    Common Includes
 *
 */

#ifndef _COMMON_INCLUDE_H
#define _COMMON_INCLUDE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/list.h>
#include <linux/kthread.h> 
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/input.h>

#include "inc/types.h"
#include "inc/drvconf.h"
#include "inc/devconf.h"


#include "inc/protocol.h"

extern int fps;

// object predefine

struct rpusbdisp_dev;

#ifndef err
#define err(format,arg...) printk(KERN_ERR format, ## arg)
#endif

#ifndef info
#define info(format,arg...) printk(KERN_ERR format, ## arg)
#endif


#endif


