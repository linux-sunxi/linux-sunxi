#include <linux/module.h>
#include <linux/init.h>
#include <linux/rfkill.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <plat/sys_config.h>

#if (defined CONFIG_MMC)
#include "ap6210_gpio.h"
#else
static __inline int ap6210_gpio_wifi_get_mod_type(void)
{
	AP6210_DEBUG("%s : not implemented!\n", __FUNCTION__ );
	return 0;
}
static __inline int ap6210_gpio_wifi_gpio_ctrl(char* name, int level)
{
	AP6210_DEBUG("%s : not implemented!\n", __FUNCTION__ );
	return -1;
}
#endif

#include <ap6210.h>

static const char bt_name[] = "bcm40183";
static struct rfkill *sw_rfkill;
static int bt_used;

static int rfkill_set_power(void *data, bool blocked)
{
	unsigned int mod_sel = ap6210_gpio_wifi_get_mod_type();
    
	AP6210_DEBUG("rfkill set power %s\n", ( blocked ? "blocked" : "unblocked" ));
    
	switch (mod_sel)
	{
        case 2: /* bcm40183 */
		if (!blocked) {
			ap6210_gpio_wifi_gpio_ctrl("bcm40183_bt_regon", 1);
			ap6210_gpio_wifi_gpio_ctrl("bcm40183_bt_rst", 1);
		} else {
			ap6210_gpio_wifi_gpio_ctrl("bcm40183_bt_rst", 0);
			ap6210_gpio_wifi_gpio_ctrl("bcm40183_bt_regon", 0);
		}
		AP6210_ERR("Using %s configuration.\n", ap6210_gpio_wifi_get_name(mod_sel) );
		break;
        case 3: /* realtek rtl8723as */
		if (!blocked) {
			ap6210_gpio_wifi_gpio_ctrl("rtk_rtl8723as_bt_dis", 1);
		} else {
			ap6210_gpio_wifi_gpio_ctrl("rtk_rtl8723as_bt_dis", 0);
		}
		AP6210_ERR("Using %s configuration.\n", ap6210_gpio_wifi_get_name(mod_sel) );
		break;            
        case 7: /* ap6210 */
        case 8: /* ap6330 */
		if (!blocked) {
			ap6210_gpio_wifi_gpio_ctrl("ap6xxx_bt_regon", 1);
		} else {
			ap6210_gpio_wifi_gpio_ctrl("ap6xxx_bt_regon", 0);
		}
		AP6210_ERR("Using %s configuration.\n", ap6210_gpio_wifi_get_name(mod_sel) );
		break;    
	case 10: /* realtek rtl8723au */
		if (!blocked) {
			ap6210_gpio_wifi_gpio_ctrl("rtl8723au_bt", 1);
		} else {
			ap6210_gpio_wifi_gpio_ctrl("rtl8723au_bt", 0);
		}
		AP6210_ERR("Using %s configuration.\n", ap6210_gpio_wifi_get_name(mod_sel) );
		break;
        default:
		AP6210_ERR("no bluetooth module matched.\n" );
	}
    
	msleep(10);
	return 0;
}

static struct rfkill_ops sw_rfkill_ops = {
	.set_block = rfkill_set_power,
};

static int sw_rfkill_probe(struct platform_device *pdev)
{
	int ret = 0;

	sw_rfkill = rfkill_alloc(bt_name, &pdev->dev, 
				 RFKILL_TYPE_BLUETOOTH, &sw_rfkill_ops, NULL);
	if (unlikely(!sw_rfkill)) {
		AP6210_DEBUG("Unable to alocate rfkill structure.\n" );
		return -ENOMEM;
		}

	ret = rfkill_register(sw_rfkill);
	if (unlikely(ret)) {
		AP6210_DEBUG("Unable to register rfkill structure.\n" );
		rfkill_destroy(sw_rfkill);
	}
	AP6210_DEBUG("rfkill structure registered successfully.\n" );
	return ret;
}

static int sw_rfkill_remove(struct platform_device *pdev)
{
	if (likely(sw_rfkill)) {
		rfkill_unregister(sw_rfkill);
		rfkill_destroy(sw_rfkill);
		AP6210_DEBUG("rfkill structure removed successfully.\n" );
	}
	
	return 0;
}

static struct platform_driver sw_rfkill_driver = {
	.probe = sw_rfkill_probe,
	.remove = sw_rfkill_remove,
	.driver = { 
		.name = "sunxi-rfkill",
		.owner = THIS_MODULE,
	},
};

static struct platform_device sw_rfkill_dev = {
	.name = "sunxi-rfkill",
};

int __init sw_rfkill_init(void)
{
	if (SCRIPT_PARSER_OK != script_parser_fetch("bt_para", "bt_used", &bt_used, 1)) {
		AP6210_DEBUG("parse bt_used failed in script.fex.\n" );
		return -1;
	}

	if (!bt_used) {
		AP6210_ERR("bluetooth is disable in script.fex.\n" );
		return 0;
	}

	platform_device_register(&sw_rfkill_dev);
	AP6210_ERR("platform device registered successfully.\n" );
	return platform_driver_register(&sw_rfkill_driver);
}

void __exit sw_rfkill_exit(void)
{
	if (!bt_used) {
		AP6210_ERR("exit no bt used in configuration\n" );
		return ;
	}

	platform_device_unregister(&sw_rfkill_dev);
	platform_driver_unregister(&sw_rfkill_driver);
}

/*
module_init(sw_rfkill_init);
module_exit(sw_rfkill_exit);

MODULE_DESCRIPTION("sunxi-rfkill driver");
MODULE_AUTHOR("Aaron.magic<mgaic@reuuimllatech.com>");
MODULE_LICENSE("GPL");
*/

