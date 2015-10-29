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
 *    Definition of USB Driver Handlers
 */

#ifndef _DRIVER_HANDLER_H
#define _DRIVER_HANDLER_H

int register_usb_handlers(void);
void unregister_usb_handlers(void);

int rpusbdisp_usb_get_device_count(void);

struct device * rpusbdisp_usb_get_devicehandle(struct rpusbdisp_dev *);
void rpusbdisp_usb_set_fbhandle(struct rpusbdisp_dev *, void *);
void * rpusbdisp_usb_get_fbhandle(struct rpusbdisp_dev * dev);

void   rpusbdisp_usb_set_touchhandle(struct rpusbdisp_dev * dev, void *);
void * rpusbdisp_usb_get_touchhandle(struct rpusbdisp_dev * dev);

int rpusbdisp_usb_try_send_image(struct rpusbdisp_dev * dev, const pixel_type_t * framebuffer, int x, int y, int right, int bottom, int line_width, int clear_dirty);
int rpusbdisp_usb_try_draw_rect(struct rpusbdisp_dev * dev, int x, int y, int right, int bottom,  pixel_type_t color, int operation);
int rpusbdisp_usb_try_copy_area(struct rpusbdisp_dev * dev, int sx, int sy, int dx, int dy, int width, int height);

#endif