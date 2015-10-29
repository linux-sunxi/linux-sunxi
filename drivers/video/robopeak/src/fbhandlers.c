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
 *   ---------------------------------------------------
 *    Frame Buffer Handlers
 */

#include "inc/common.h"
#include "inc/fbhandlers.h"
#include "inc/usbhandlers.h"

static struct fb_info * _default_fb;

struct dirty_rect {
    int  left;
    int  top;
    int  right;
    int  bottom;
    atomic_t dirty_flag;
};  


enum {
    DISPLAY_UPDATE_HINT_NONE       = 0,
    DISPLAY_UPDATE_HINT_BITBLT     = 1,
    DISPLAY_UPDATE_HINT_FILLRECT   = 2,
    DISPLAY_UPDATE_HINT_COPYAREA   = 3,

};


struct rpusbdisp_fb_private {
    _u32 pseudo_palette [16];
    struct dirty_rect dirty_rect;
    struct mutex      operation_lock;
    struct rpusbdisp_dev * binded_usbdev;


//lock free area
    atomic_t               unsync_flag;
};



static struct fb_fix_screeninfo _vfb_fix /*__devinitdata*/ = {
	.id =	    "rpusbdisp-fb",
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_TRUECOLOR,
	.accel =	FB_ACCEL_NONE,
    .line_length = RP_DISP_DEFAULT_WIDTH * RP_DISP_DEFAULT_PIXEL_BITS/8,
    .accel =        FB_ACCEL_NONE,
};


static  struct fb_var_screeninfo _var_info /*__devinitdata*/ = {
    .xres = RP_DISP_DEFAULT_WIDTH,
    .yres = RP_DISP_DEFAULT_HEIGHT,
    .xres_virtual = RP_DISP_DEFAULT_WIDTH,
    .yres_virtual = RP_DISP_DEFAULT_HEIGHT,
    .width = RP_DISP_DEFAULT_WIDTH,
    .height = RP_DISP_DEFAULT_HEIGHT,
    .bits_per_pixel =        RP_DISP_DEFAULT_PIXEL_BITS ,
    .red = { 0 , 5 , 0 } ,
    .green = { 5 , 6 , 0 } ,
    .blue = { 11 , 5 , 0 } ,
    .activate = FB_ACTIVATE_NOW,
    .vmode = FB_VMODE_NONINTERLACED,
};


static DEFINE_MUTEX(_mutex_devreg);



static inline struct rpusbdisp_fb_private * _get_fb_private(struct fb_info * info)
{
    return (struct rpusbdisp_fb_private *)info->par;
}


static void _clear_dirty_rect(struct dirty_rect * rect) {
    rect->left = RP_DISP_DEFAULT_WIDTH;
    rect->right = -1;
    rect->top = RP_DISP_DEFAULT_HEIGHT;
    rect->bottom = -1;
    atomic_set(&rect->dirty_flag,0);
}

static void _reset_fb_private(struct rpusbdisp_fb_private * pa) {
    mutex_init(&pa->operation_lock);
    _clear_dirty_rect(&pa->dirty_rect);
    pa->binded_usbdev = NULL;
    atomic_set(&pa->unsync_flag, 1);
}




static  void _display_update( struct fb_info *p, int x, int y, int width, int height, int hint, const void * hint_data)
{

    struct rpusbdisp_fb_private * pa = _get_fb_private(p);

    int clear_dirty = 0;
    mutex_lock(&pa->operation_lock);

    if (!pa->binded_usbdev) goto final;
    // 1. update the dirty rect
    
    if (atomic_dec_and_test(&pa->unsync_flag)) {
        // force the dirty rect to cover the full display area if the display is not synced.
        pa->dirty_rect.left = 0;
        pa->dirty_rect.right = p->var.width-1;
        pa->dirty_rect.top = 0;
        pa->dirty_rect.bottom = p->var.height-1;

        clear_dirty = 1;
    } else {
        
        if (pa->dirty_rect.top > y) pa->dirty_rect.top = y;
        if (pa->dirty_rect.bottom < height+y-1) pa->dirty_rect.bottom = height+y-1;
        if (pa->dirty_rect.left > x) pa->dirty_rect.left = x;
        if (pa->dirty_rect.right < width+x-1) pa->dirty_rect.right = width + x - 1;

    }

    if (pa->dirty_rect.top > pa->dirty_rect.bottom || pa->dirty_rect.left > pa->dirty_rect.right) {
        goto final;
    }
    atomic_set(&pa->dirty_rect.dirty_flag, 1);

    // 2. try to send it
    if (pa->binded_usbdev) {

        switch (hint) {
            case DISPLAY_UPDATE_HINT_FILLRECT:
            {
                const struct fb_fillrect * fillrt = (struct fb_fillrect *)hint_data;
                if (rpusbdisp_usb_try_draw_rect(pa->binded_usbdev, fillrt->dx, fillrt->dy, fillrt->dx + fillrt->width-1, 
                    fillrt->dy + fillrt->height-1, fillrt->color, fillrt->rop==ROP_XOR?RPUSBDISP_OPERATION_XOR:RPUSBDISP_OPERATION_COPY))
                {
                    // data sent, rect the dirty rect
                    _clear_dirty_rect(&pa->dirty_rect);
                  
                        
                }
            }
            break;

            case DISPLAY_UPDATE_HINT_COPYAREA:
            {
                const  struct fb_copyarea * copyarea = (struct fb_copyarea *)hint_data;
                if (rpusbdisp_usb_try_copy_area(pa->binded_usbdev, copyarea->sx, copyarea->sy, copyarea->dx,  copyarea->dy, 
                    copyarea->width, copyarea->height))
                {
                    // data sent, rect the dirty rect
                    _clear_dirty_rect(&pa->dirty_rect);
                       
                }

            }
            break;
            default:
                if (rpusbdisp_usb_try_send_image(pa->binded_usbdev, (const pixel_type_t *)p->fix.smem_start,
                     pa->dirty_rect.left, pa->dirty_rect.top, pa->dirty_rect.right, pa->dirty_rect.bottom, p->fix.line_length/(RP_DISP_DEFAULT_PIXEL_BITS/8),
                     clear_dirty)) {
                    // data sent, rect the dirty rect
                    _clear_dirty_rect(&pa->dirty_rect);

                } 
        }

    }
final:
    mutex_unlock(&pa->operation_lock);

    
}


static  void _display_fillrect ( struct fb_info * p, const  struct fb_fillrect * rect)
{
    sys_fillrect (p, rect);
    _display_update(p, rect->dx, rect->dy, rect->width, rect->height, DISPLAY_UPDATE_HINT_FILLRECT, rect);
}

static  void _display_imageblit ( struct fb_info * p, const  struct fb_image * image)
{

    sys_imageblit (p, image);
    _display_update(p, image->dx, image->dy, image->width, image->height, DISPLAY_UPDATE_HINT_BITBLT, image);
}

static  void _display_copyarea ( struct fb_info * p, const  struct fb_copyarea * area)
{

    sys_copyarea (p, area);
    _display_update(p, area->dx, area->dy, area->width, area->height, DISPLAY_UPDATE_HINT_COPYAREA, area);
}

static ssize_t _display_write ( struct fb_info * p, const  char * buf __user,
                                size_t count, loff_t * ppos)
{       
    int retval;
    retval = fb_sys_write (p, buf, count, ppos);

    _display_update(p, 0, 0, p->var.width, p->var.height, DISPLAY_UPDATE_HINT_NONE, NULL);
    return retval;
}


static int _display_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info)
{
    #define CNVT_TOHW(val, width) ((((val) << (width)) +0x7FFF-(val)) >> 16)
    int ret = 1 ;
    if (info->var.grayscale)
        red = green = blue = ( 19595 * red + 38470 * green +
                               7471 * blue) >> 16 ;
    switch (info->fix.visual) {
    case FB_VISUAL_TRUECOLOR:
        if (regno < 16 ) {
            u32 * pal = info->pseudo_palette;
            u32 value;

            red = CNVT_TOHW (red, info->var.red.length);
            green = CNVT_TOHW (green, info->var.green.length);
            blue = CNVT_TOHW (blue, info->var.blue.length);
            transp = CNVT_TOHW (transp, info->var.transp.length);

            value = (red << info->var.red.offset) |
                    (green << info->var.green.offset) |
                    (blue << info->var.blue.offset) |
                    (transp << info->var.transp.offset);

            pal[regno] = value;
            ret = 0 ;
        }
        break ;
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
    case FB_VISUAL_PSEUDOCOLOR:
         break ;
    }
    return ret;
}


static void _display_defio_handler(struct fb_info *info,
				struct list_head *pagelist) {
    struct page *cur;
    struct fb_deferred_io *fbdefio = info->fbdefio;
    int top = RP_DISP_DEFAULT_HEIGHT, bottom = 0;
    int current_val;
    unsigned long offset;
    unsigned long page_start;

    struct rpusbdisp_fb_private * pa = _get_fb_private(info);
    if (!pa->binded_usbdev) return; //simply ignore it 
    
    list_for_each_entry(cur, &fbdefio->pagelist, lru) {

        // convert page range to dirty box
        page_start = (cur->index<<PAGE_SHIFT);
        
        if (page_start < info->fix.mmio_start && page_start >= info->fix.mmio_start + info->fix.smem_len) {
            continue;
        }
        
        offset = (unsigned long)(page_start - info->fix.mmio_start);
        current_val = offset / info->fix.line_length;
        if (top>current_val) top = current_val;
        current_val = (offset + PAGE_SIZE + info->fix.line_length -1 )/ info->fix.line_length;
        if (bottom<current_val) bottom = current_val;

    }
    if (bottom >= RP_DISP_DEFAULT_HEIGHT) bottom = RP_DISP_DEFAULT_HEIGHT - 1;


    _display_update(info, 0, top, info->var.width, bottom - top + 1, DISPLAY_UPDATE_HINT_NONE, NULL);
}

static  struct fb_ops _display_fbops /*__devinitdata*/ = {
    .owner = THIS_MODULE,
    .fb_read = fb_sys_read,
    .fb_write =     _display_write,
    .fb_fillrect =  _display_fillrect,
    .fb_copyarea =  _display_copyarea,
    .fb_imageblit = _display_imageblit,
    .fb_setcolreg = _display_setcolreg,
};


static void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); /* Clear the ram out, no junk to the user */
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}

static void rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}




#if 0

static struct platform_driver rpusbdisp_fb_driver = {
	.probe		= _rpusbdisp_initial_probe,
	.remove		= __devexit_p(_rpusbdisp_final_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "rpusbdisp-fb",
	},
};
#endif

static int _on_create_new_fb(struct fb_info ** out_fb, struct rpusbdisp_dev *dev)
{
    int ret = -ENOMEM;
    size_t fbmem_size = 0;
    void * fbmem = NULL;
    struct fb_info * fb;
    struct fb_deferred_io *fbdefio;
    
    *out_fb = NULL;
    
   
    fb = framebuffer_alloc(sizeof(struct rpusbdisp_fb_private), NULL/*rpusbdisp_usb_get_devicehandle(dev)*/);
    
    if (!fb) {
        err("Failed to initialize framebuffer device\n");
        goto failed;
    }

    fb->fix = _vfb_fix;
    fb->var = _var_info;


    fb->fbops       = &_display_fbops;
    fb->flags       = FBINFO_DEFAULT | FBINFO_VIRTFB;
    
    fbmem_size = _var_info.yres * _vfb_fix.line_length; // Correct issue with size allocation (too big)
    fbmem =  rvmalloc(fbmem_size);
    if (!fbmem) {

        err("Cannot allocate fb memory.\n");
        goto failed_nofb;
    }


    memset(fbmem, 0, fbmem_size);

    fb->screen_base = (char __iomem *)fbmem;
    fb->fix.smem_start = (unsigned long)fb->screen_base;
    fb->fix.smem_len = fbmem_size;
    
    fb->pseudo_palette = _get_fb_private(fb)->pseudo_palette;
    

    if (fb_alloc_cmap(&fb->cmap, 256, 0)) {
       err("Cannot alloc the cmap.\n");

       goto failed_nocmap;

    }


	// Since kernel 3.5 the fb_deferred_io structure has a second callback (first_io) that must be set to a valid function or NULL.
	// To avoid unexpected crash due to a non initialized function pointer do a kzalloc rather than a kmalloc
	fbdefio = /*kmalloc*/kzalloc(sizeof(struct fb_deferred_io), GFP_KERNEL);

	if (fbdefio) {
                // frame rate is configurable through the fps option during the load operation
		fbdefio->delay = HZ/fps;
		fbdefio->deferred_io = _display_defio_handler;
	} else {
        err("Cannot alloc the fb_deferred_io.\n");

         goto failed_nodefio;

	}

	fb->fbdefio = fbdefio;
	fb_deferred_io_init(fb);


    _reset_fb_private(_get_fb_private(fb));

    // register the framebuffer device
    ret = register_framebuffer(fb);
    if (ret < 0) {
        err("Cannot register the framebuffer device.\n");
        goto failed_on_reg;
    }

    *out_fb = fb;
    return ret;



failed_on_reg:
    kfree(fbdefio);
failed_nodefio:
    fb_dealloc_cmap(&fb->cmap);
failed_nocmap:
    rvfree(fbmem, fbmem_size);
failed_nofb:
    framebuffer_release(fb);
failed:
    return ret;

}

static void _on_release_fb(struct fb_info * fb)
{
    if (!fb) return;

    // del the defio
    fb_deferred_io_cleanup(fb);
    
    unregister_framebuffer(fb);
    kfree(fb->fbdefio);
    fb_dealloc_cmap(&fb->cmap);
    rvfree(fb->screen_base, fb->fix.smem_len);
    framebuffer_release(fb);

    
}


int __init register_fb_handlers(void)
{
    return _on_create_new_fb(&_default_fb, NULL);
}

void unregister_fb_handlers(void)
{
    _on_release_fb(_default_fb);
}


void fbhandler_on_all_transfer_done(struct rpusbdisp_dev * dev)
{
    // we have a chance to flush pending modifications to the display
    struct fb_info * fb = (struct fb_info *)rpusbdisp_usb_get_fbhandle(dev);
    struct rpusbdisp_fb_private * fb_pri;
    
    if (!fb) return;

    fb_pri = _get_fb_private(fb);
    
    if (atomic_read(&fb_pri->dirty_rect.dirty_flag) || atomic_read(&fb_pri->unsync_flag)==1) {
        _display_update(fb, 0, 0, RP_DISP_DEFAULT_WIDTH, RP_DISP_DEFAULT_HEIGHT,   DISPLAY_UPDATE_HINT_NONE, NULL);
    }

}


int fbhandler_on_new_device(struct rpusbdisp_dev * dev)
{
    int ans = -1;
    struct rpusbdisp_fb_private * fb_pri;
    
    mutex_lock(&_mutex_devreg);

    // check whether the default fb has been binded
    fb_pri = _get_fb_private(_default_fb);
    if (!fb_pri->binded_usbdev) {
        mutex_lock(&fb_pri->operation_lock);

        // bind to the default framebuffer ( the only one)
        fb_pri->binded_usbdev = dev;
        rpusbdisp_usb_set_fbhandle(dev, _default_fb);


        ans = 0; 
        mutex_unlock(&fb_pri->operation_lock);
    }

    mutex_unlock(&_mutex_devreg);
    return ans;
}

void fbhandler_on_remove_device(struct rpusbdisp_dev * dev)
{
    // perform unbinding
    struct fb_info * fb = (struct fb_info *)rpusbdisp_usb_get_fbhandle(dev);

    mutex_lock(&_mutex_devreg);
    
    if (fb) {
        struct rpusbdisp_fb_private * fb_pri = _get_fb_private(_default_fb);

        mutex_lock(&fb_pri->operation_lock);

        // unbind them
        fb_pri->binded_usbdev = NULL;
        rpusbdisp_usb_set_fbhandle(dev, NULL);
        
        mutex_unlock(&fb_pri->operation_lock);


        // unregister the fb
        if (fb != _default_fb) {
            _on_release_fb(fb);
        }
    }

    mutex_unlock(&_mutex_devreg);
}

void fbhandler_set_unsync_flag(struct rpusbdisp_dev * dev)
{
    struct fb_info * fb = (struct fb_info *)rpusbdisp_usb_get_fbhandle(dev);
    struct rpusbdisp_fb_private * fb_pri;
    
    if (!fb) return;

    fb_pri = _get_fb_private(_default_fb);

    atomic_set(&fb_pri->unsync_flag, 1);

}
