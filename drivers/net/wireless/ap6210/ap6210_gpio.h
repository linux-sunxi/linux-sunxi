#ifndef __AP6210_GPIO_H__
#define __AP6210_GPIO_H__

extern int ap6210_gpio_wifi_get_mod_type(void);
extern int ap6210_gpio_wifi_gpio_ctrl(char* name, int level);
extern void ap6210_gpio_wifi_power(int on);
extern char *ap6210_gpio_wifi_get_name(int module_sel);

#endif	/* __AP6210_GPIO_H__ */
