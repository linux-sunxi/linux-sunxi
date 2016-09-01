#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <plat/sys_config.h>
#include <mach/gpio.h>
#include <mach/clock.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <ap6210.h>

static char* wifi_para = "wifi_para";

struct ap6210_gpio_wifi_ops {
	char* mod_name;
	int   wifi_used;
	int   sdio_id;
	int   usb_id;
	int   module_sel;
	int   (*gpio_ctrl)(char* name, int level);	
	void  (*standby)(int in);
	void  (*power)(int mode, int *updown);

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_root;
	struct proc_dir_entry *proc_power;
#endif	/* CONFIG_PROC_FS */
};

static int ap6210_wl_regon = 0;
static int ap6210_bt_regon = 0;

struct ap6210_gpio_wifi_ops ap6210_wifi_select_pm_ops;

char *ap6210_gpio_wifi_get_name(int module_sel) {
	char* mod_name[] = { " ",
			     "bcm40181",   /* 1 - BCM40181(BCM4330)*/
			     "bcm40183",   /* 2 - BCM40183(BCM4330)*/
			     "rtl8723as",  /* 3 - RTL8723AS(RF-SM02B) */
			     "rtl8189es",  /* 4 - RTL8189ES(SM89E00) */
			     "rtl8192cu",  /* 5 - RTL8192CU*/
			     "rtl8188eu",  /* 6 - RTL8188EU*/
			     "ap6210",     /* 7 - AP6210*/
			     "ap6330",     /* 8 - AP6330*/
			     "ap6181",     /* 9 - AP6181*/
			     "rtl8723au",  /* 10 - RTL8723AU */
			   };
			   
	return(mod_name[module_sel]);
}

EXPORT_SYMBOL(ap6210_gpio_wifi_get_name);

static int ap6210_gpio_ctrl(char* name, int level)
{
	int i = 0;	
	int ret = 0;
	int gpio = 0;
	char * gpio_name[2] = {"ap6210_wl_regon", "ap6210_bt_regon"};

	for (i = 0; i < 2; i++) {
		if (strcmp(name, gpio_name[i]) == 0) {
			switch (i)
			{
			case 0: /*ap6210_wl_regon*/
				gpio = ap6210_wl_regon;
				break;
			case 1: /*ap6210_bt_regon*/
				gpio = ap6210_bt_regon;
				break;
			default:
				AP6210_ERR("no matched gpio.\n" );
			}
			break;
		}
	}

	ret = gpio_write_one_pin_value(gpio, level, name);
	
	return 0;
}

static int ap6210_gpio_read(char* name)
{
	int i = 0;	
	int gpio = 0;
	int val = 0;
	char * gpio_name[2] = {"ap6210_wl_regon", "ap6210_bt_regon"};

	for (i = 0; i < 2; i++) {
		if (strcmp(name, gpio_name[i]) == 0) {
			switch (i)
			{
			case 0: /*ap6210_wl_regon*/
				gpio = ap6210_wl_regon;
				break;
			case 1: /*ap6210_bt_regon*/
				gpio = ap6210_bt_regon;
				break;
			default:
				AP6210_ERR("no matched gpio.\n" );
			}
			break;
		}
	}

	val = gpio_read_one_pin_value(gpio, name);
	
	return val;
}


void ap6210_power(int mode, int *updown)
{
	if (mode) {
		if (*updown) {
			ap6210_gpio_ctrl("ap6210_wl_regon", 1);
			mdelay(100);
		} else {
			ap6210_gpio_ctrl("ap6210_wl_regon", 0);
			mdelay(100);
		}
		AP6210_DEBUG("sdio wifi power state: %s\n", *updown ? "on" : "off");
	} else {
		*updown = ap6210_gpio_read("ap6210_wl_regon");
	}

	return;	
}

static void ap6210_cfg_gpio_32k_clkout(int gpio_index)
{
	int ret;    
	struct clk *clk_32k, *parent;    
    
	parent = clk_get(NULL, CLK_SYS_LOSC);	
	clk_32k = clk_get(NULL, CLK_MOD_OUTA);
	ret = clk_set_parent(clk_32k, parent);

	if(ret){
		AP6210_ERR("32k clk_set_parent fail.\n" );
		return;
	}

	ret = clk_set_rate(clk_32k, 32768);
	if(ret){
		AP6210_ERR("32k clk_set_rate fail.\n" );
		return;
	}

	clk_enable(clk_32k);
}
void ap6210_gpio_init(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;
	int ap6210_lpo = 0;

/* CT expected ap6210_lpo as a GPIO */
	ap6210_lpo = gpio_request_ex(wifi_para, "ap6xxx_lpo");
	if (!ap6210_lpo) {
		AP6210_ERR("request lpo gpio failed.\n" );
		return;
	}

	if(ap6210_lpo) {
		AP6210_DEBUG("config 32k clock.\n" );
		ap6210_cfg_gpio_32k_clkout(ap6210_lpo);
	}

	ap6210_wl_regon = gpio_request_ex(wifi_para, "ap6xxx_wl_regon");
	if (!ap6210_wl_regon) {
		AP6210_ERR("request wl_regon gpio failed.\n" );
		return;
	}

	ap6210_bt_regon = gpio_request_ex(wifi_para, "ap6xxx_bt_regon");
	if (!ap6210_bt_regon) {
		AP6210_ERR("request ap6210_bt_regon gpio failed.\n" );
		return;
	}

	ops->gpio_ctrl	= ap6210_gpio_ctrl;
	ops->power = ap6210_power;
}

int ap6210_gpio_wifi_get_mod_type(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;
	if (ops->wifi_used)
		return ops->module_sel;
	else {
		AP6210_ERR("No wifi type selected, please check your config.\n" );
		return 0;
	}
}
EXPORT_SYMBOL(ap6210_gpio_wifi_get_mod_type);

int ap6210_gpio_wifi_gpio_ctrl(char* name, int level)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;	
	if (ops->wifi_used && ops->gpio_ctrl)		
		return ops->gpio_ctrl(name, level);	
	else {		
		AP6210_ERR("No wifi type selected, please check your config.\n" );		
		return -1;	
	}
}
EXPORT_SYMBOL(ap6210_gpio_wifi_gpio_ctrl);

void ap6210_gpio_wifi_power(int on)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;
	int power = on;

	if (ops->wifi_used && ops->power)
		return ops->power(1, &power);
	else {
		AP6210_ERR("No wifi type selected, please check your config.\n" );
		return;
	}
}
EXPORT_SYMBOL(ap6210_gpio_wifi_power);

#ifdef CONFIG_PROC_FS
static int ap6210_gpio_wifi_power_stat(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct ap6210_gpio_wifi_ops *ops = (struct ap6210_gpio_wifi_ops *)data;
	char *p = page;
	int power = 0;

	if (ops->power)
		ops->power(0, &power);

	p += sprintf(p, "%s : power state %s\n", ops->mod_name, power ? "on" : "off");
	return p - page;
}

static int ap6210_gpio_wifi_power_ctrl(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	struct ap6210_gpio_wifi_ops *ops = (struct ap6210_gpio_wifi_ops *)data;
	int power = simple_strtoul(buffer, NULL, 10);
    
	power = power ? 1 : 0;
	if (ops->power)
		ops->power(1, &power);
	else
		AP6210_ERR("No power control for %s\n", ops->mod_name);
	return sizeof(power);	
}

static inline void awwifi_procfs_attach(void)
{
	char proc_rootname[] = "driver/ap6210_gpio_wifi";
	char proc_powername[] = "power";
	
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	ops->proc_root = proc_mkdir(proc_rootname, NULL);
	if (IS_ERR(ops->proc_root))
	{
		AP6210_ERR("failed to create procfs \"%s\".\n", proc_rootname );
	}

	ops->proc_power = create_proc_entry(proc_powername, 0644, ops->proc_root);
	if (IS_ERR(ops->proc_power))
	{
		AP6210_ERR("failed to create procfs \"%s\".\n", proc_powername);
	}
	ops->proc_power->data = ops;
	ops->proc_power->read_proc = ap6210_gpio_wifi_power_stat;
	ops->proc_power->write_proc = ap6210_gpio_wifi_power_ctrl;
}

static inline void awwifi_procfs_remove(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;
	char proc_rootname[] = "driver/ap6210_gpio_wifi";

	remove_proc_entry("power", ops->proc_root);
	remove_proc_entry(proc_rootname, NULL);
}
#else
static inline void awwifi_procfs_attach(void) {}
static inline void awwifi_procfs_remove(void) {}
#endif



static int ap6210_gpio_wifi_get_res(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	if (SCRIPT_PARSER_OK != script_parser_fetch(wifi_para, "wifi_used", &ops->wifi_used, 1)) {
		AP6210_ERR("parse wifi_used failed in script.fex.\n" );
		return -1;
	}
	if (!ops->wifi_used) {
		AP6210_ERR("wifi pm disable in script.fex.\n" );
		return -1;
	}

	if (SCRIPT_PARSER_OK != script_parser_fetch(wifi_para, "wifi_sdc_id", &ops->sdio_id, 1)) {
		AP6210_ERR("parse wifi_sdc_id in script.fex failed.\n" );
		return -1;
	}

	if (SCRIPT_PARSER_OK != script_parser_fetch(wifi_para, "wifi_usbc_id", &ops->usb_id, 1)) {
		AP6210_ERR("parse wifi_sdc_id in script.fex failed.\n" );
		return -1;
	}

	if (SCRIPT_PARSER_OK != script_parser_fetch(wifi_para, "wifi_mod_sel", &ops->module_sel, 1)) {
		AP6210_ERR("parse wifi_sdc_id in script.fex failed.\n" );
		return -1;
	}

	ops->mod_name = ap6210_gpio_wifi_get_name(ops->module_sel);
	
	AP6210_ERR("select wifi %s\n", ops->mod_name);

	return 0;
}

static int __devinit ap6210_gpio_wifi_probe(struct platform_device *pdev)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	switch (ops->module_sel) {
	case 1: /* BCM40181 */
	case 2: /* BCM40183 */
	case 3: /* RTL8723AS */
	case 4: /* RTL8189ES */
	case 5: /* RTL8192CU */
	case 6: /* RTL8188EU */
		AP6210_ERR("Unsupported device.\n");
		break;
	case 7: /* AP6210 */
	case 8: /* AP6330 */
	case 9: /* AP6181 */
		AP6210_ERR("Initializing %s.\n", ops->mod_name);
		ap6210_gpio_init();
		break;
	case 10: /* RTL8723AU */
		AP6210_ERR("Unsupported device.\n");
		break;
	default:
		AP6210_ERR("Unsupported device.\n");
	}

	awwifi_procfs_attach();
	AP6210_DEBUG("wifi gpio attached.\n" );
	return 0;
}

static int __devexit ap6210_gpio_wifi_remove(struct platform_device *pdev)
{
	awwifi_procfs_remove();
	AP6210_DEBUG("wifi gpio released.\n" );
	return 0;
}

#ifdef CONFIG_PM
static int ap6210_gpio_wifi_suspend(struct device *dev)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	if (ops->standby)
		ops->standby(1);
	return 0;
}

static int ap6210_gpio_wifi_resume(struct device *dev)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	if (ops->standby)
		ops->standby(0);
	return 0;
}

static struct dev_pm_ops wifi_dev_pm_ops = {
	.suspend	= ap6210_gpio_wifi_suspend,
	.resume		= ap6210_gpio_wifi_resume,
};
#endif

static struct platform_device ap6210_gpio_wifi_dev = {
	.name           = "ap6210_gpio_wifi",
};

static struct platform_driver ap6210_gpio_wifi_driver = {
	.driver.name    = "ap6210_gpio_wifi",
	.driver.owner   = THIS_MODULE,
#ifdef CONFIG_PM
	.driver.pm      = &wifi_dev_pm_ops,
#endif
	.probe          = ap6210_gpio_wifi_probe,
	.remove         = __devexit_p(ap6210_gpio_wifi_remove),
};

int __init ap6210_gpio_wifi_init(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;

	memset(ops, 0, sizeof(struct ap6210_gpio_wifi_ops));
	ap6210_gpio_wifi_get_res();
	if (!ops->wifi_used)
		return 0;

	platform_device_register(&ap6210_gpio_wifi_dev);
	return platform_driver_register(&ap6210_gpio_wifi_driver);
}

void __exit ap6210_gpio_wifi_exit(void)
{
	struct ap6210_gpio_wifi_ops *ops = &ap6210_wifi_select_pm_ops;
	if (!ops->wifi_used)
		return;

	platform_driver_unregister(&ap6210_gpio_wifi_driver);
	memset(ops, 0, sizeof(struct ap6210_gpio_wifi_ops));
}

/*
module_init(ap6210_gpio_wifi_init);
module_exit(ap6210_gpio_wifi_exit);

MODULE_LICENSE("GPL");
*/