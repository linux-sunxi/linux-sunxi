/*
; 9 - realtek rtl8723as(combo) sdio wifi + bt gpio config
;rtk_rtl8723as_wb_pwr       = port:PH12<1><default><default><0>
;rtk_rtl8723as_wl_dis       = port:PH11<1><default><default><0>
;rtk_rtl8723as_wl_wps       = port:PH09<0><default><default><0>
;rtk_rtl8723as_bt_dis       = port:PB05<1><default><default><0>*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <plat/sys_config.h>

#include "mmc_pm.h"

#define SDIO_MODULE_NAME "RTL8723AS"
#define rtw_msg(...)    do {printk("[RTL8723AS]: "__VA_ARGS__);} while(0)
static int rtl8723as_wl_on = 0;
static int rtl8723as_bt_on = 0;

static int rtk_suspend = 0;

static int rtl8723as_pwr_gpio_ctrl(int level)
{
    struct mmc_pm_ops *ops = &mmc_card_pm_ops;
    int ret = gpio_write_one_pin_value(ops->pio_hdle, level, "rtk_rtl8723as_wb_pwr");

    if (!ret) {
        rtw_msg("%s powered %s\n", SDIO_MODULE_NAME, level ? "up" : "down");
        if (!level) {
            rtl8723as_wl_on = 0;
            rtl8723as_bt_on = 0;
        }
    }

    return ret;
}

struct gpio_per_module_cmd {
    const char *this_module_name;
    const char *other_module_name;
    const char *this_module_dis_pin_name;
    int *this_module_on_status;
    int *other_module_on_status;
};

static int rtl8723as_gpio_ctrl(char* name, int level)
{
    struct mmc_pm_ops *ops = &mmc_card_pm_ops;
    struct gpio_per_module_cmd gpio_cmd[4] = {
        { "wifi", "bluetooth", "rtk_rtl8723as_wl_dis", &rtl8723as_wl_on, &rtl8723as_bt_on },
        { "bluetooth", "wifi", "rtk_rtl8723as_bt_dis", &rtl8723as_bt_on, &rtl8723as_wl_on }
    };
    int i = 0;
    int ret = 0;

    if (strcmp(name, "rtk_rtl8723as_wb_pwr") == 0)
        return rtl8723as_pwr_gpio_ctrl(level);

    for (i=0; i<2; i++) {
        if (strcmp(name, gpio_cmd[i].this_module_dis_pin_name)==0)
            break;
    }

    if (i==2) {
        rtw_msg("No gpio %s for %s module\n", name, SDIO_MODULE_NAME);
        return -1;
    }

    if (!gpio_cmd[i].other_module_on_status) {
        rtw_msg("%s is powered %s by %s\n",
                SDIO_MODULE_NAME,
                level ? "up" : "down",
                *(gpio_cmd[i].this_module_name));
        ret = rtl8723as_pwr_gpio_ctrl(level);
        if (!ret)
            ret = gpio_write_one_pin_value(ops->pio_hdle, level, name);
        if (!ret)
            *(gpio_cmd[i].this_module_on_status) = level;
    } else if (level) {
        rtw_msg("%s is already on by %s\n",
                SDIO_MODULE_NAME,
                gpio_cmd[i].other_module_name);
        ret = gpio_write_one_pin_value(ops->pio_hdle, level, name);
        if (!ret)
            *(gpio_cmd[i].this_module_on_status) = level;
    } else {
        rtw_msg("%s should stay on because of %s\n",
                SDIO_MODULE_NAME,
                gpio_cmd[i].other_module_name);
    }

    return ret;
}

static int rtl8723as_get_gpio_value(char* name)
{
    struct mmc_pm_ops *ops = &mmc_card_pm_ops;
    
    if (strcmp(name, "rtk_rtl8723as_wl_wps")) {
        rtw_msg("No gpio %s for %s\n", name, SDIO_MODULE_NAME);
        return -1;
    }
    
    return gpio_read_one_pin_value(ops->pio_hdle, name);
}

void rtl8723as_power(int mode, int* updown)
{
    if (mode) {
        if (*updown) {
        	rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 1);
        } else {
        	rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 0);
        }
    } else {
        if (rtl8723as_wl_on)
            *updown = 1;
        else
            *updown = 0;
		rtw_msg("sdio wifi power state: %s\n", rtl8723as_wl_on ? "on" : "off");
    }
    return;
}

static void rtl8723as_standby(int instadby)
{
    if (instadby) {
        if (rtl8723as_wl_on) {
            rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 0);
            printk("%s: mmc_pm_suspend.\n", __FUNCTION__);
            rtk_suspend = 1;
        }
    } else {
        if (rtk_suspend) {
            rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 1);
            sunximmc_rescan_card(3, 1);
            printk("%s: mmc_pm_resume.\n", __FUNCTION__);
            rtk_suspend = 0;
        }
    }
}

void rtl8723as_gpio_init(void)
{
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;
	int updown = 1;

	rtl8723as_wl_on = 0;
	rtl8723as_bt_on = 0;
	rtk_suspend	= 0;
	ops->gpio_ctrl	= rtl8723as_gpio_ctrl;
	ops->get_io_val = rtl8723as_get_gpio_value;
	ops->power	= rtl8723as_power;
	ops->standby	= rtl8723as_standby;
	rtl8723as_power(1, &updown);
	sunximmc_rescan_card(ops->sdio_cardid, 1);
	rtw_msg("power up, rescan card.\n");
}

#undef SDIO_MODULE_NAME
