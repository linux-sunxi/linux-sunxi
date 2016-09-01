#ifndef __AP6210_H__
#define __AP6210_H__

#define AP6210_EMERG(...)	pr_emerg("[ap6210] "__VA_ARGS__)
#define AP6210_ALERT(...)	pr_alert("[ap6210] "__VA_ARGS__)
#define AP6210_CRIT(...)	pr_crit("[ap6210] "__VA_ARGS__)
#define AP6210_ERR(...)		pr_err("[ap6210] "__VA_ARGS__)
#define AP6210_WARN(...)	pr_warn("[ap6210] "__VA_ARGS__)
#define AP6210_NOTICE(...)	pr_notice("[ap6210] "__VA_ARGS__)
#define AP6210_INFO(...)	pr_info("[ap6210] "__VA_ARGS__)
#define AP6210_DEBUG(...)	pr_debug("[ap6210] "__VA_ARGS__)
#define AP6210_DUMP(...)	pr_debug(__VA_ARGS__)
#define AP6210_CONT(...)	pr_cont(__VA_ARGS__)

extern int __init sw_rfkill_init(void);
extern void __exit sw_rfkill_exit(void);

extern int __init ap6210_gpio_wifi_init(void);
extern void __exit ap6210_gpio_wifi_exit(void);


#endif  /* __AP6210_H__ */
