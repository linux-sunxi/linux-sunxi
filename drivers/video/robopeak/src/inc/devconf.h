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
 *   Device Configurations
 */

#ifndef _DEVICE_CONF_H
#define _DEVICE_CONF_H

#define RP_DISP_DRIVER_NAME     "rp-usbdisp"

#define RP_DISP_USB_VENDOR_ID   0xFCCF // RP Pseudo vendor id
#define RP_DISP_USB_PRODUCT_ID  0xA001

#define RP_DISP_DEFAULT_HEIGHT      240
#define RP_DISP_DEFAULT_WIDTH       320
#define RP_DISP_DEFAULT_PIXEL_BITS  16
typedef _u16  pixel_type_t; 


#define RP_DISP_FEATURE_RLE_FWVERSION 0x0104

#endif
