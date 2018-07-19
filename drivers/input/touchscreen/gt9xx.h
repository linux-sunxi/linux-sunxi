/*
 *  driver for Goodix Touchscreens
 *
 *  Copyright (c) 2014 Red Hat Inc.
 *
 *  This code is based on gt9xx.c authored by bpi
 *
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 of the License.
 */

struct goodix_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	int abs_x_max;
	int abs_y_max;
	unsigned int max_touch_num;
	unsigned int int_trigger_type;
	int gtp_cfg_len;
	uint8_t use_irq;

	spinlock_t	irq_lock;

	struct work_struct  work;
	struct workqueue_struct *ts_workqueue;
	struct proc_dir_entry *proc_entry;
	struct hrtimer timer;
};

// The predefined one is just a sample config, which is not suitable for your tp in most cases.
#define CTP_CFG_GROUP0 {\
		0x50,0x20,0x03,0xE0,0x01,0x05,0x0D,0x00,0x01,0x08,\
        0x28,0x05,0x50,0x32,0x03,0x05,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8C,0x28,0x0C,\
        0x3B,0x3D,0x0C,0x08,0x00,0x00,0x01,0x02,0x03,0x1D,\
        0x00,0x01,0x00,0x00,0x00,0x03,0x64,0x32,0x00,0x00,\
        0x00,0x1E,0x50,0x94,0xD5,0x02,0x07,0x00,0x00,0x04,\
        0x9B,0x21,0x00,0x72,0x28,0x00,0x57,0x31,0x00,0x42,\
        0x3B,0x00,0x35,0x48,0x00,0x35,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,\
        0x16,0x18,0x1A,0x1C,0xFF,0xFF,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0F,\
        0x10,0x12,0x13,0x14,0x16,0x18,0x1C,0x1D,0x1E,0x1F,\
        0x20,0x21,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0xB5,0x01\
    }
    
// TODO: define your config for Sensor_ID == 1 here, if needed
#define CTP_CFG_GROUP1 {\
    }

#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))


#define GOODIX_CTP_NAME			"gt9xx"

#define GOODIX_POLL_TIME			10
#define GOODIX_INT_TRIGGER			1
#define GOODIX_MAX_CONTACTS			10
#define GOODIX_ADDR_LENGTH       	2
#define GOODIX_CONFIG_MAX_LENGTH	240
#define GOODIX_CONFIG_MIN_LENGTH 	186
#define GOODIX_MAX_TOUCH         	5

#define GOODIX_RESOLUTION_LOC		3
#define GOODIX_TRIGGER_LOC			8

#define FAIL                  		0
#define SUCCESS               		1

/* Register defines */
#define GOODIX_READ_COOR_ADDR		0x814E
#define GOODIX_REG_SENSOR_ID     	0x814A
#define GOODIX_REG_CONFIG_DATA		0x8047
#define GOODIX_REG_VERSION			0x8140

/* Base defines for gpio irq */
#define PIO_BASE_ADDRESS	(0x01c20800)
#define PIO_RANGE_SIZE		(0x400)

#define PIO_INT_STAT_OFFSET	(0x214)
#define PIO_INT_CTRL_OFFSET	(0x210)

typedef enum {
     PIO_INT_CFG0_OFFSET = 0x200,
     PIO_INT_CFG1_OFFSET = 0x204,
     PIO_INT_CFG2_OFFSET = 0x208,
     PIO_INT_CFG3_OFFSET = 0x20c,
} int_cfg_offset;

typedef enum {
	POSITIVE_EDGE = 0x0,
	NEGATIVE_EDGE = 0x1,
	HIGH_LEVEL = 0x2,
	LOW_LEVEL = 0x3,
	DOUBLE_EDGE = 0x4
} ext_int_mode;

u8 config[GOODIX_CONFIG_MAX_LENGTH + GOODIX_ADDR_LENGTH]
                = {GOODIX_REG_CONFIG_DATA >> 8, GOODIX_REG_CONFIG_DATA & 0xff};


#define CTP_IRQ_NO		(gpio_int_info[0].port_num)
#define CTP_IRQ_MODE		(NEGATIVE_EDGE)

static void * __iomem gpio_addr = NULL;
static int gpio_int_hdle = 0;
static int gpio_reset_hdle = 0;
static int gpio_wakeup_hdle = 0;
static user_gpio_set_t gpio_int_info[1];
static int	int_cfg_addr[]={PIO_INT_CFG0_OFFSET,PIO_INT_CFG1_OFFSET,
			PIO_INT_CFG2_OFFSET, PIO_INT_CFG3_OFFSET};

/* Addresses to scan */
static union{
       unsigned short dirty_addr_buf[2];
       const unsigned short normal_i2c[2];
} u_i2c_addr = {{0x00},};
static __u32 twi_id;
static __u32 twi_addr;

static int screen_max_x = 0;
static int screen_max_y = 0;
static int exchange_x_y_flag = 0;

#define GOODIX_CONFIG_PROC_FILE     "gt9xx_config"


static void goodix_reset_guitar(struct i2c_client *client, s32 ms);
static void goodix_int_sync(s32 ms);
