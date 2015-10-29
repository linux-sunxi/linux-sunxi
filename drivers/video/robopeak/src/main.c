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
 */


#include "inc/common.h"
#include "inc/usbhandlers.h"
#include "inc/fbhandlers.h"
#include "inc/touchhandlers.h"



#if 0
static const struct file_operations lcd_fops = {
        .owner =        THIS_MODULE,
        .read =         lcd_read,
        .write =        lcd_write,
        .open =         lcd_open,
	.unlocked_ioctl = lcd_ioctl,
        .release =      lcd_release,
        .llseek =	 noop_llseek,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver lcd_class = {
        .name =         "usbdisp%d",
        .fops =         &lcd_fops,
        .minor_base =   USBLCD_MINOR,
};
#endif

int fps = 0;
module_param(fps,int,0);
MODULE_PARM_DESC(fps,"Specify the frame rate used to refresh the display (override kernel config)");



static int __init usb_disp_init(void)
{
	int result;

        if (fps == 0) {
	    /* frame rate is not set through the modprobe command. Use the value defined in the .config */
#ifdef CONFIG_RPUSBDISP_FPS
	    fps = CONFIG_RPUSBDISP_FPS;
#else
	    /* Just in case for background compliance. Maybe the Kconfig file of the driver is not integrated */
            fps = 16;
#endif
        }

    do {
	    
        result = register_touch_handler();
	    if (result) {
		    err("touch_handler failed. Error number %d", result);
            break;
        }        

        result = register_fb_handlers();
        if (result) {
            err("fb handler register failed. Error number %d", result);
            break;
        }

        result = register_usb_handlers();
	    if (result) {
		    err("usb_register failed. Error number %d", result);
            break;
        }



    }while(0);

	return result;
}


static void __exit usb_disp_exit(void)
{
    unregister_usb_handlers();
    unregister_fb_handlers();
    unregister_touch_handler();
}

module_init(usb_disp_init);
module_exit(usb_disp_exit);


MODULE_AUTHOR("Shikai Chen <csk@live.com>");
MODULE_DESCRIPTION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE_INFO);

