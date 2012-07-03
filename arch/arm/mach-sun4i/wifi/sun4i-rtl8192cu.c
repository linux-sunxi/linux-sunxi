#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <mach/sys_config.h>

extern int sw_usb_disable_hcd(__u32 usbc_no);
extern int sw_usb_enable_hcd(__u32 usbc_no);
static int usb_wifi_host = 2;

static int __init sun4i_enable_rtl8192cu_wifi(void)
{
    int ret = 0;

    ret = script_parser_fetch("usb_wifi_para", "usb_wifi_usbc_num", (int *)&usb_wifi_host, 64);
    if(ret != 0) {
        printk(KERN_ERR "script_parser_fetch usb_wifi_usbc_num failed\n");
        ret = -ENOMEM;
        return ret;
    }

    printk(KERN_INFO "sw_usb_enable_hcd: usbc_num = %d\n", usb_wifi_host);

    sw_usb_enable_hcd(usb_wifi_host);

    return ret;
}

static void __exit sun4i_disable_rtl8192cu_wifi(void)
{
    printk(KERN_INFO "sw_usb_disable_hcd: usbc_num = %d\n", usb_wifi_host);
    sw_usb_disable_hcd(usb_wifi_host);
}


module_init(sun4i_enable_rtl8192cu_wifi);
module_exit(sun4i_disable_rtl8192cu_wifi);

MODULE_DESCRIPTION("Enable RTL8192CU on SUN4I");
MODULE_LICENSE("GPL");

