/*
 * Linux cfg80211 driver - Android related functions
 *
 * Copyright (C) 1999-2012, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: wl_android.h 367273 2012-11-07 09:58:55Z $
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <wldev_common.h>

/* If any feature uses the Generic Netlink Interface, put it here to enable WL_GENL
 * automatically
 */

/**
 * Android platform dependent functions, feel free to add Android specific functions here
 * (save the macros in dhd). Please do NOT declare functions that are NOT exposed to dhd
 * or cfg, define them as static in wl_android.c
 */

/**
 * wl_android_init will be called from module init function (dhd_module_init now), similarly
 * wl_android_exit will be called from module exit function (dhd_module_cleanup now)
 */
int wl_android_init(void);
int wl_android_exit(void);
void wl_android_post_init(void);
int wl_android_wifi_on(struct net_device *dev);
int wl_android_wifi_off(struct net_device *dev);
int wl_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd);

#define BSSCACHE
#define RSSIAVG
#define RSSIOFFSET
//#define RSSIOFFSET_NEW

#if defined(RSSIAVG)
#define RSSIAVG_LEN 8
#define RSSICACHE_LEN 8

typedef struct wl_rssi_cache {
	struct wl_rssi_cache *next;
	int dirty;
	struct ether_addr BSSID;
	int16 RSSI[RSSIAVG_LEN];
} wl_rssi_cache_t;

typedef struct wl_rssi_cache_ctrl {
	wl_rssi_cache_t *m_cache_head;
} wl_rssi_cache_ctrl_t;

void wl_free_rssi_cache(wl_rssi_cache_ctrl_t *rssi_cache_ctrl);
void wl_delete_dirty_rssi_cache(wl_rssi_cache_ctrl_t *rssi_cache_ctrl);
void wl_reset_rssi_cache(wl_rssi_cache_ctrl_t *rssi_cache_ctrl);
void wl_update_rssi_cache(wl_rssi_cache_ctrl_t *rssi_cache_ctrl, wl_scan_results_t *ss_list);
void wl_update_connected_rssi_cache(wl_rssi_cache_ctrl_t *rssi_cache_ctrl, struct net_device *net);
int16 wl_get_avg_rssi(wl_rssi_cache_ctrl_t *rssi_cache_ctrl, void *addr);
#endif

#if defined(RSSIOFFSET)
#define RSSI_OFFSET	5
#define RSSI_MAX -80
#define RSSI_MIN -94
#define RSSI_INT ((RSSI_MAX-RSSI_MIN)/RSSI_OFFSET)
#define BCM4330_CHIP_ID		0x4330
#define BCM4330B2_CHIP_REV      4
int wl_update_rssi_offset(int rssi);
#endif

#if defined(BSSCACHE)
#define BSSCACHE_LEN	8
#define BSSCACHE_TIME	15000

typedef struct wl_bss_cache {
	struct wl_bss_cache *next;
	int dirty;
	wl_scan_results_t results;
} wl_bss_cache_t;

typedef struct wl_bss_cache_ctrl {
	wl_bss_cache_t *m_cache_head;
	struct timer_list *m_timer;
	int m_timer_expired;
} wl_bss_cache_ctrl_t;

void wl_free_bss_cache(wl_bss_cache_ctrl_t *bss_cache_ctrl);
void wl_delete_dirty_bss_cache(wl_bss_cache_ctrl_t *bss_cache_ctrl);
void wl_reset_bss_cache(wl_bss_cache_ctrl_t *bss_cache_ctrl);
void wl_update_bss_cache(wl_bss_cache_ctrl_t *bss_cache_ctrl, wl_scan_results_t *ss_list);
void wl_run_bss_cache_timer(wl_bss_cache_ctrl_t *bss_cache_ctrl, int kick_off);
void wl_release_bss_cache_ctrl(wl_bss_cache_ctrl_t *bss_cache_ctrl);
void wl_init_bss_cache_ctrl(wl_bss_cache_ctrl_t *bss_cache_ctrl);
#endif

#if defined(CONFIG_WIFI_CONTROL_FUNC)
int wl_android_wifictrl_func_add(void);
void wl_android_wifictrl_func_del(void);
void* wl_android_prealloc(int section, unsigned long size);

int wifi_get_irq_number(unsigned long *irq_flags_ptr);
int wifi_set_power(int on, unsigned long msec);
int wifi_get_mac_addr(unsigned char *buf);
void *wifi_get_country_code(char *ccode);
#endif /* CONFIG_WIFI_CONTROL_FUNC */
