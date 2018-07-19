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

#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/unaligned.h>

#include <asm/io.h>
#include <linux/gpio.h>
#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <plat/sys_config.h>
#include "gt9xx.h"

struct i2c_client * i2c_connect_client = NULL;

static ssize_t goodix_config_read_proc(struct file *, char __user *, size_t, loff_t *);
static ssize_t goodix_config_write_proc(struct file *, const char __user *, size_t, loff_t *);

static const struct file_operations config_proc_ops = {
    .owner = THIS_MODULE,
    .read = goodix_config_read_proc,
    .write = goodix_config_write_proc,
};

/**
 * goodix_i2c_read - read data from a register of the i2c slave device.
 *
 * @client: i2c device.
 * @reg: the register to read from.
 * @buf: raw write data buffer.
 * @len: length of the buffer to write
 */
static s32 goodix_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
	struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = client->addr;
    msgs[0].len   = GOODIX_ADDR_LENGTH;
    msgs[0].buf   = &buf[0];
    
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = client->addr;
    msgs[1].len   = len - GOODIX_ADDR_LENGTH;
    msgs[1].buf   = &buf[GOODIX_ADDR_LENGTH];

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, msgs, 2);
        if(ret == 2)
			break;
        retries++;
    }
    if((retries >= 5))
    {
        pr_err("I2C Read: 0x%04X, %d bytes failed, errcode: %d! Process reset\n", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
        goodix_reset_guitar(client, 10);  
    }
    return ret;
}

static int goodix_i2c_write(struct i2c_client *client,u8 *buf,s32 len)
{
    struct i2c_msg msg;
    s32 ret = -1;
    s32 retries = 0;

    msg.flags = !I2C_M_RD;
    msg.addr  = client->addr;
    msg.len   = len;
    msg.buf   = buf;

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret == 1)
			break;
		
        retries++;
    }
    if((retries >= 5))
    {
    	pr_err("I2C Write: 0x%04X, %d bytes failed, errcode: %d! Process reset\n", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
        goodix_reset_guitar(client, 10);  
    }
	
    return ret;
}

void goodix_gpio_as_input(u32 p_handler, const char *gpio_name)
{
	gpio_set_one_pin_io_status(p_handler, 0, gpio_name);
}

void goodix_gpio_as_output(u32 p_handler, __u32 value, const char *gpio_name)
{
	gpio_set_one_pin_io_status(p_handler, 1, gpio_name);
	gpio_write_one_pin_value(p_handler, value, gpio_name);

}

void goodix_gpio_as_int(void)
{
	pr_info("%s: config gpio to int mode. \n", __func__);

	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	gpio_int_hdle = gpio_request_ex("ctp_para", "ctp_int_port");
	if(!gpio_int_hdle){
		pr_info("request tp_int_port failed. \n");
		return;
	}
	gpio_get_one_pin_status(gpio_int_hdle, gpio_int_info, "ctp_int_port", 1);

	pr_info("%s, %d: gpio_int_info, port = %d, port_num = %d, funcion = %d\n", __func__, __LINE__,
		gpio_int_info[0].port, gpio_int_info[0].port_num, gpio_int_info[0].mul_sel);

}

static void goodix_clear_penirq(void)
{
	int reg_val;

	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	
	if((reg_val = (reg_val&(1<<(CTP_IRQ_NO))))){
		//pr_info("==clear CTP_IRQ_NO=\n");
		writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	}
	
	return;
}

static int goodix_judge_int_occur(void)
{
	int reg_val;
	int ret = -1;

	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	
	if(reg_val&(1<<(CTP_IRQ_NO))){
		ret = 0;
	}
	
	return ret;
}


void goodix_irq_disable(struct goodix_ts_data *ts)
{
    unsigned long irqflags;
	__u32 reg_val = 0;

    spin_lock_irqsave(&ts->irq_lock, irqflags);
	reg_val = readl(gpio_addr+PIO_INT_CTRL_OFFSET);
	reg_val &= ~(1 << (gpio_int_info[0].port_num));
	writel(reg_val,gpio_addr+PIO_INT_CTRL_OFFSET);
    spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}

void goodix_irq_enable(struct goodix_ts_data *ts)
{
    unsigned long irqflags = 0;
	__u32 reg_val = 0;
    
    spin_lock_irqsave(&ts->irq_lock, irqflags);
    reg_val = readl(gpio_addr+PIO_INT_CTRL_OFFSET);
	reg_val |= (1 << (gpio_int_info[0].port_num));
	writel(reg_val,gpio_addr+PIO_INT_CTRL_OFFSET);
    spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}


void goodix_io_init(u8 addr, int ms)
{       
	goodix_gpio_as_output(gpio_reset_hdle, 0, "ctp_reset");   // begin select I2C slave addr
	msleep(ms);						   // T2: > 10ms

	// HIGH: 0x28/0x29, LOW: 0xBA/0xBB
	goodix_gpio_as_output(gpio_wakeup_hdle, addr == 0x14, "ctp_wakeup");
	
	msleep(2);						   // T3: > 100us
	goodix_gpio_as_output(gpio_reset_hdle, 1, "ctp_reset");
	//goodix_gpio_as_output(gpio_reset_hdle, 0, "ctp_reset");

	msleep(10);						   // T4: > 5ms
	//goodix_gpio_as_output(gpio_reset_hdle, 1, "ctp_reset");    // end select I2C slave addr
	//goodix_int_sync(100);
}

static void goodix_int_sync(s32 ms)
{
    goodix_gpio_as_output(gpio_wakeup_hdle, 0, "ctp_wakeup");
    msleep(ms);
    goodix_gpio_as_int();
}

static void goodix_reset_guitar(struct i2c_client *client, s32 ms)
{
    goodix_gpio_as_output(gpio_reset_hdle, 0, "ctp_reset");   // begin select I2C slave addr
    msleep(ms);                         // T2: > 10ms

	// HIGH: 0x28/0x29, LOW: 0xBA/0xBB
    goodix_gpio_as_output(gpio_wakeup_hdle, client->addr == 0x14, "ctp_wakeup");

    msleep(2);                          // T3: > 100us
    goodix_gpio_as_output(gpio_reset_hdle, 1, "ctp_reset");
    
    msleep(6);                          // T4: > 5ms

    goodix_int_sync(50);  
}

static void goodix_touch_down(struct goodix_ts_data* ts,s32 id,s32 x,s32 y,s32 w)
{
    //pr_info("source data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);

	if(1 == exchange_x_y_flag){
        swap(x, y);
    }

	//pr_info("report data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);

    input_report_key(ts->input_dev, BTN_TOUCH, 1);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(ts->input_dev);

    pr_info("ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
}

static void goodix_touch_up(struct goodix_ts_data* ts, s32 id)
{
    input_report_key(ts->input_dev, BTN_TOUCH, 0);
}

static void goodix_ts_work_func(struct work_struct *work)
{
	struct goodix_ts_data *ts = container_of(work, struct goodix_ts_data, work);

	u8  end_cmd[3] = {GOODIX_READ_COOR_ADDR >> 8, GOODIX_READ_COOR_ADDR & 0xFF, 0};
    u8  point_data[2 + 1 + 8 * GOODIX_MAX_TOUCH + 1]={GOODIX_READ_COOR_ADDR >> 8, GOODIX_READ_COOR_ADDR & 0xFF};
    u8  touch_num = 0;
    u8  finger = 0;
    static u16 pre_touch = 0;
    static u8 pre_key = 0;

    u8  key_value = 0;
    u8* coor_data = NULL;
    int input_x = 0;
    int input_y = 0;
    int input_w = 0;
    int id = 0;
    int i  = 0;
    int ret = -1;
	
    ret = goodix_i2c_read(ts->client, point_data, 12);
    if (ret < 0)
    {
        pr_err("I2C transfer error. errno:%d\n", ret);
        if (ts->use_irq)
        {
            goodix_irq_enable(ts);
        }
        return;
    }
    
    finger = point_data[GOODIX_ADDR_LENGTH];
    if (finger == 0x00)
    {
        if (ts->use_irq)
        {
            goodix_irq_enable(ts);
        }
        return;
    }

    if((finger & 0x80) == 0)
    {
        goto exit_work_func;
    }

    touch_num = finger & 0x0f;
    if (touch_num > GOODIX_MAX_TOUCH)
    {
        goto exit_work_func;
    }

    if (touch_num > 1)
    {
        u8 buf[8 * GOODIX_MAX_TOUCH] = {(GOODIX_READ_COOR_ADDR + 10) >> 8, (GOODIX_READ_COOR_ADDR + 10) & 0xff};

        ret = goodix_i2c_read(ts->client, buf, 2 + 8 * (touch_num - 1)); 
        memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
    }

    pre_key = key_value;

    //pr_info("pre_touch:%02x, finger:%02x\n", pre_touch, finger);


    if (touch_num)
    {
        for (i = 0; i < touch_num; i++)
        {
            coor_data = &point_data[i * 8 + 3];

            id = coor_data[0] & 0x0F;
            input_x  = coor_data[1] | (coor_data[2] << 8);
            input_y  = coor_data[3] | (coor_data[4] << 8);
            input_w  = coor_data[5] | (coor_data[6] << 8);

            goodix_touch_down(ts, id, input_x, input_y, input_w);
        }
    }
    else if (pre_touch)
    {
        pr_info("Touch Release\n");
        goodix_touch_up(ts, 0);
    }

    pre_touch = touch_num;

    input_sync(ts->input_dev);

exit_work_func:
    ret = goodix_i2c_write(ts->client, end_cmd, 3);
    if (ret < 0)
    {
        pr_info("I2C write end_cmd error!\n");
    }
	
    if (ts->use_irq)
    {
        goodix_irq_enable(ts);
    }

	return;
}

static enum hrtimer_restart goodix_ts_timer_handler(struct hrtimer *timer)
{
    struct goodix_ts_data *ts = container_of(timer, struct goodix_ts_data, timer);

    queue_work(ts->ts_workqueue, &ts->work);
    hrtimer_start(&ts->timer, ktime_set(0, (GOODIX_POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
    return HRTIMER_NORESTART;
}

/**
 * goodix_ts_irq_handler - The IRQ handler
 *
 * @irq: interrupt number.
 * @dev_id: private data pointer.
 */
static irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{
	struct goodix_ts_data *ts = dev_id;

    goodix_irq_disable(ts);

	if (!goodix_judge_int_occur()){
		goodix_clear_penirq();
		//pr_info("goodix_ts_irq_handler\n");
		if(!work_pending(&ts->work)){
			queue_work(ts->ts_workqueue, &ts->work);
		}
	}
	else {
		//pr_info("other irq\n");
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}


/**
 * goodix_set_irq_mode - Configure irq to int port.
 *
 * @ts: our goodix_ts_data pointer
 * @major_key: section key
 * @subkey: section subkey
 * @int_mode: int mode
 *
 * Must be called during probe
 */
static int goodix_set_irq_mode(struct goodix_ts_data *ts, char *major_key, 
						char *subkey, ext_int_mode int_mode)
{
	__u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;

	pr_info("Config gpio to int mode\n");

	if (gpio_int_hdle)
		gpio_release(gpio_int_hdle, 2);

	gpio_int_hdle = gpio_request_ex(major_key, subkey);
	if (!gpio_int_hdle) {
		dev_err(&ts->client->dev, "Request ctp_int_port failed.\n");
		return -1;
	}
 
	gpio_get_one_pin_status(gpio_int_hdle, gpio_int_info, subkey, 1);
	pr_info("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__, \
		gpio_int_info[0].port, gpio_int_info[0].port_num);
	
	reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;
	reg_val = readl(gpio_addr + int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (int_mode << (reg_num * 4));
	writel(reg_val, gpio_addr + int_cfg_addr[reg_addr]);

	goodix_clear_penirq();

	reg_val = readl(gpio_addr + PIO_INT_CTRL_OFFSET);
	reg_val |= (1 << (gpio_int_info[0].port_num));
	writel(reg_val, gpio_addr + PIO_INT_CTRL_OFFSET);

	udelay(1);
	
	return 0;
}

/**
 * goodix_read_version - Read goodix touchscreen version
 *
 * @client: the i2c client
 * @version: output buffer containing the version on success
 */
static int goodix_read_version(struct i2c_client *client, u16 *version)
{
	int ret = -1;
    u8 buf[8] = {GOODIX_REG_VERSION >> 8, GOODIX_REG_VERSION & 0xff};

    ret = goodix_i2c_read(client, buf, sizeof(buf));
    if (ret < 0)
    {
        pr_err("GTP read version failed\n");
        return ret;
    }

    if (version)
    {
        *version = (buf[7] << 8) | buf[6];
    }
    if (buf[5] == 0x00)
    {
        pr_info("IC Version: %c%c%c_%02x%02x\n", buf[2], buf[3], buf[4], buf[7], buf[6]);
    }
    else
    {
        pr_info("IC Version: %c%c%c%c_%02x%02x\n", buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
    }
	
    return ret;
}

static int goodix_i2c_read_dbl_check(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
    u8 buf[16] = {0};
    u8 confirm_buf[16] = {0};
    u8 retry = 0;
    
    while (retry++ < 3)
    {
        memset(buf, 0xAA, 16);
        buf[0] = (u8)(addr >> 8);
        buf[1] = (u8)(addr & 0xFF);
        goodix_i2c_read(client, buf, len + 2);
        
        memset(confirm_buf, 0xAB, 16);
        confirm_buf[0] = (u8)(addr >> 8);
        confirm_buf[1] = (u8)(addr & 0xFF);
        goodix_i2c_read(client, confirm_buf, len + 2);
        
        if (!memcmp(buf, confirm_buf, len+2))
        {
            memcpy(rxbuf, confirm_buf+2, len);
            return SUCCESS;
        }
    }    
    pr_err("I2C read 0x%04X, %d bytes, double check failed!\n", addr, len);
    return FAIL;
}


static int  goodix_send_cfg(struct i2c_client *client)
{
    int ret;
    int retry;
    
    pr_info("Driver send config\n");
	
    for (retry = 0; retry < 5; retry++)
    {
        ret = goodix_i2c_write(client, config , GOODIX_CONFIG_MAX_LENGTH + GOODIX_ADDR_LENGTH);
        if (ret > 0)
        {
            break;
        }
    }

    return ret;
}


static int goodix_init_panel(struct goodix_ts_data *ts)
{
	int ret = -1;
	int i;
	u8 opr_buf[16] = {0};
	u8 sensor_id = 0;
	u8 drv_cfg_version;
	u8 check_sum = 0;
	
	u8 cfg_info_group0[] = CTP_CFG_GROUP0;
    u8 cfg_info_group1[] = CTP_CFG_GROUP1;

	u8 *send_cfg_buf[] = {cfg_info_group0,cfg_info_group1,};
    u8 cfg_info_len[] = { CFG_GROUP_LEN(cfg_info_group0),
						  CFG_GROUP_LEN(cfg_info_group1),};
	
    pr_info("Config Groups\' Lengths: %d, %d\n", 
        cfg_info_len[0], cfg_info_len[1]);

	/* check firmware */
    ret = goodix_i2c_read_dbl_check(ts->client, 0x41E4, opr_buf, 1);
    if (SUCCESS == ret)
	{
        if (opr_buf[0] != 0xBE)
        {
            pr_err("Firmware error, no config sent!\n");
            return -1;
        }
    }

	/* get sensor id */
	ret = goodix_i2c_read_dbl_check(ts->client, GOODIX_REG_SENSOR_ID, &sensor_id, 1);
    if (SUCCESS == ret)
    {
        if (sensor_id >= 0x06)
        {
            pr_err("Invalid sensor_id(0x%02X), No Config Sent!\n", sensor_id);
            return -1;
        }
    }
    else
    {
        pr_err("Failed to get sensor_id, No config sent!\n");
        return -1;
    }

	pr_info("Sensor_ID: %d\n", sensor_id);
	pr_info("Get config data from header file.\n");
	
	ts->gtp_cfg_len = cfg_info_len[sensor_id];
    pr_info("Config group%d used,length: %d\n", sensor_id, ts->gtp_cfg_len);
    
    if (ts->gtp_cfg_len < GOODIX_CONFIG_MIN_LENGTH)
    {
        pr_err("Config Group%d is INVALID CONFIG GROUP(Len: %d)! NO Config Sent! You need to check you header file CFG_GROUP section!\n", sensor_id, ts->gtp_cfg_len);
        return -1;
    }

	/* get version */
	ret = goodix_i2c_read_dbl_check(ts->client, GOODIX_REG_CONFIG_DATA, &opr_buf[0], 1);
	if (ret == SUCCESS) 
	{
		pr_info("CFG_GROUP%d Config Version: %d, 0x%02X; IC Config Version: %d, 0x%02X\n", sensor_id, 
                        send_cfg_buf[sensor_id][0], send_cfg_buf[sensor_id][0], opr_buf[0], opr_buf[0]);

		if (opr_buf[0] < 90)
		{
			drv_cfg_version = send_cfg_buf[sensor_id][0]; 
			send_cfg_buf[sensor_id][0] = 0x00;
		}
		else {
			pr_err("Ic fixed config with config version(%d, 0x%02X)\n", opr_buf[0], opr_buf[0]);
			return 0;
		}
	} else {
	    pr_err("Failed to get ic config version!No config sent!\n");
	    return -1;
	}

	memset(&config[GOODIX_ADDR_LENGTH], 0, GOODIX_CONFIG_MAX_LENGTH);
	memcpy(&config[GOODIX_ADDR_LENGTH], send_cfg_buf[sensor_id], ts->gtp_cfg_len);
	
	/* calculate checksum and update cfg */
	check_sum = 0;
    for (i = GOODIX_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
    {
        check_sum += config[i];
    }
    config[ts->gtp_cfg_len] = (~check_sum) + 1;

	if ((ts->abs_x_max == 0) && (ts->abs_y_max == 0))
    {
        ts->abs_x_max = (config[GOODIX_RESOLUTION_LOC + 1] << 8) + config[GOODIX_RESOLUTION_LOC];
        ts->abs_y_max = (config[GOODIX_RESOLUTION_LOC + 3] << 8) + config[GOODIX_RESOLUTION_LOC + 2];
        ts->int_trigger_type = (config[GOODIX_TRIGGER_LOC]) & 0x03; 
    }

	/* send cfg */
	ret = goodix_send_cfg(ts->client);
    if (ret < 0)
    {
        pr_err("Send config error\n");
    }

	check_sum = 0;
	config[GOODIX_ADDR_LENGTH] = drv_cfg_version;
	for (i = GOODIX_ADDR_LENGTH; i < ts->gtp_cfg_len; i++) {
		check_sum += config[i];
	}
	
	config[ts->gtp_cfg_len] = (~check_sum) + 1;

	pr_info("X_MAX: %d, Y_MAX: %d, TRIGGER: 0x%02x\n", ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);

	msleep(10);

	return 0;

}

/**
 * goodix_i2c_test - I2C test function to check if the device answers.
 *
 * @client: the i2c client
 */
static int goodix_i2c_test(struct i2c_client *client)
{
	u8 test[3] = {GOODIX_REG_CONFIG_DATA >> 8, GOODIX_REG_CONFIG_DATA & 0xff};
    u8 retry = 0;
    int ret = -1;
  
    while(retry++ < 5)
    {
        ret = goodix_i2c_read(client, test, 3);
        if (ret > 0)
        {
            return ret;
        }
		
        pr_err("GTP i2c test failed time %d\n",retry);
        msleep(10);
    }
	
    return ret;
}

/**
 * goodix_request_input_dev - Allocate, populate and register the input device
 *
 * @ts: our goodix_ts_data pointer
 *
 * Must be called during probe
 */
static int goodix_request_input_dev(struct goodix_ts_data *ts)
{
	int error;

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		dev_err(&ts->client->dev, "Failed to allocate input device.");
		return -ENOMEM;
	}

	ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) |
				  BIT_MASK(EV_KEY) |
				  BIT_MASK(EV_ABS);

	ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
    __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);

	if (1 == exchange_x_y_flag)
		swap(ts->abs_x_max, ts->abs_y_max);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);

	ts->input_dev->name = "Goodix Capacitive TouchScreen";
	ts->input_dev->phys = "input/ts";
	ts->input_dev->id.bustype = BUS_I2C;
	ts->input_dev->id.vendor = 0x0416;
	ts->input_dev->id.product = 0x1001;
	ts->input_dev->id.version = 10427;

	error = input_register_device(ts->input_dev);
	if (error) {
		dev_err(&ts->client->dev,
			"Failed to register input device: %d", error);
		return error;
	}

	return 0;
}

static int goodix_request_irq(struct goodix_ts_data *ts)
{
	int ret;
	
    ret = goodix_set_irq_mode(ts, "ctp_para", "ctp_int_port", CTP_IRQ_MODE);
	if (0 != ret) {
		dev_err(&ts->client->dev, "Set irq mode failed.");
		return ret;
	}

	ret = request_irq(SW_INT_IRQNO_PIO, goodix_ts_irq_handler, IRQF_SHARED, "gt9xx", ts);
    if (ret != 0){
		pr_err("goodix_probe: request irq failed\n");
		goodix_gpio_as_input(gpio_int_hdle, "ctp_int_port");
        gpio_release(gpio_int_hdle, 2);

		pr_err("start polling mode\n");
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        ts->timer.function = goodix_ts_timer_handler;
        hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
	else {
		goodix_irq_disable(ts);
		ts->use_irq = 1;
	}

	return 0;
}

static int goodix_init_platform_resource(void)
{
	gpio_addr = ioremap(PIO_BASE_ADDRESS, PIO_RANGE_SIZE);
	if (!gpio_addr)
		return -EIO;

	gpio_reset_hdle = gpio_request_ex("ctp_para", "ctp_reset");
	if(!gpio_reset_hdle) {
		pr_err("Request ctp_reset failed.\n");
		return -1;
	}

	gpio_wakeup_hdle = gpio_request_ex("ctp_para", "ctp_wakeup");
	if(!gpio_wakeup_hdle) {
		pr_err("Request ctp_wakeup failed.\n");
		return -1;
	}

	return 0;
}

static int goodix_free_platform_resource(void)
{
	if (gpio_addr)
		iounmap(gpio_addr);

	if (gpio_int_hdle)
		gpio_release(gpio_int_hdle, 2);

	if (gpio_reset_hdle)
		gpio_release(gpio_reset_hdle, 2);

	if (gpio_wakeup_hdle)
		gpio_release(gpio_wakeup_hdle, 2);

	return 0;
}

/**
 * goodix_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:
 *	= 0: success;
 *	< 0: err;
 */
static int goodix_fetch_sysconfig_para(void)
{
	int ret = -1;
	int ctp_used = 0;
	char name[I2C_NAME_SIZE];
	script_parser_value_type_t type = SCRIPT_PARSER_VALUE_TYPE_STRING;

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_used", &ctp_used, 1)) {
		printk(KERN_ERR "*** ctp_used set to 0!\n");
		printk(KERN_ERR "*** If use ctp, please set ctp_used to 1.\n");
		return ret;
	}

	if (SCRIPT_PARSER_OK != script_parser_fetch_ex("ctp_para", "ctp_name", (int *)(&name), &type, sizeof(name)/sizeof(int))) {
		printk(KERN_ERR "Failed to fetch ctp_name.\n");
		return ret;
	}
	printk(KERN_INFO "ctp_name is %s.\n", name);

	if (strcmp(GOODIX_CTP_NAME, name)) {
		printk(KERN_ERR "Name %s does not match GOODIX_CTP_NAME.\n", name);
		return ret;
	}

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_id", &twi_id, sizeof(twi_id)/sizeof(__u32))) {
		printk(KERN_ERR "Failed to fetch ctp_twi_id.\n");
		return ret;
	}
	printk(KERN_INFO "ctp_twi_id is %d.\n", twi_id);

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_addr", &twi_addr, sizeof(twi_addr)/sizeof(__u32))) {
		printk(KERN_ERR "Failed to fetch ctp_twi_addr.\n");
		return ret;
	}
	printk(KERN_INFO "ctp_twi_addr is 0x%hx.\n", twi_addr);

	u_i2c_addr.dirty_addr_buf[0] = twi_addr;
	u_i2c_addr.dirty_addr_buf[1] = I2C_CLIENT_END;

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_x", &screen_max_x, 1)) {
		printk(KERN_ERR "Failed to fetch ctp_screen_max_x.\n");
		return ret;
	}
	printk(KERN_INFO "screen_max_x = %d.\n", screen_max_x);

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_y", &screen_max_y, 1)) {
		printk(KERN_ERR "Failed to fetch ctp_screen_max_y.\n");
		return ret;
	}
	printk(KERN_INFO "screen_max_y = %d.\n", screen_max_y);

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_exchange_x_y_flag", &exchange_x_y_flag, 1)) {
		printk(KERN_ERR "Failed to fetch ctp_exchange_x_y_flag.\n");
		return ret;
	}
	printk(KERN_INFO "exchange_x_y_flag = %d.\n", exchange_x_y_flag);

	return 0;
}

static ssize_t goodix_config_read_proc(struct file *file, char __user *page, size_t size, loff_t *ppos)
{
    char *ptr = page;
    char temp_data[GOODIX_CONFIG_MAX_LENGTH + 2] = {0x80, 0x47};
    int i;
    
    if (*ppos)
    {
        return 0;
    }
    ptr += sprintf(ptr, "==== GT9XX config init value====\n");

    for (i = 0 ; i < GOODIX_CONFIG_MAX_LENGTH ; i++)
    {
        ptr += sprintf(ptr, "0x%02X ", config[i + 2]);

        if (i % 8 == 7)
            ptr += sprintf(ptr, "\n");
    }

    ptr += sprintf(ptr, "\n");

    ptr += sprintf(ptr, "==== GT9XX config real value====\n");
    goodix_i2c_read(i2c_connect_client, temp_data, GOODIX_CONFIG_MAX_LENGTH + 2);
    for (i = 0 ; i < GOODIX_CONFIG_MAX_LENGTH ; i++)
    {
        ptr += sprintf(ptr, "0x%02X ", temp_data[i+2]);

        if (i % 8 == 7)
            ptr += sprintf(ptr, "\n");
    }
    *ppos += ptr - page;
    return (ptr - page);
}


static ssize_t goodix_config_write_proc(struct file *filp, const char __user *buffer, size_t count, loff_t *off)
{
    s32 ret = 0;

    pr_info("write count %d\n", count);

    if (count > GOODIX_CONFIG_MAX_LENGTH)
    {
        pr_err("size not match [%d:%d]\n", GOODIX_CONFIG_MAX_LENGTH, count);
        return -EFAULT;
    }

    if (copy_from_user(&config[2], buffer, count))
    {
        pr_err("copy from user fail\n");
        return -EFAULT;
    }

    ret = goodix_send_cfg(i2c_connect_client);

    if (ret < 0)
    {
        pr_err("send config failed\n");
    }

    return count;
}

static int goodix_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct goodix_ts_data *ts;
	int error;
	u16 version_info;

	dev_dbg(&client->dev, "I2C Address: 0x%02x\n", client->addr);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C check functionality failed.\n");
		return -ENXIO;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	i2c_connect_client = client;
	ts->client = client;
	i2c_set_clientdata(client, ts);

	error = goodix_i2c_test(client);
	if (error < 0) {
		dev_err(&client->dev, "I2C communication failure: %d\n", error);
		return error;
	}

	error = goodix_read_version(client, &version_info);
	if (error < 0) {
		dev_err(&client->dev, "Read version failed.\n");
		return error;
	}

	error = goodix_init_panel(ts);
    if (error < 0)
    {
        pr_err("GTP init panel failed\n");
   		return -ENXIO;
    }

	error = goodix_request_input_dev(ts);
	if (error)
		return error;

	ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if (!ts->ts_workqueue)
    {
        pr_err("Creat workqueue failed\n");
        return -ESRCH;
    }

	spin_lock_init(&ts->irq_lock);    
	INIT_WORK(&ts->work, goodix_ts_work_func);

	error = goodix_request_irq(ts); 
	if (error < 0) {
		dev_err(&client->dev, "request IRQ failed: %d.\n", error);
		return error;
	}

	ts->proc_entry = proc_create(GOODIX_CONFIG_PROC_FILE, 0666, NULL, &config_proc_ops);
    if (ts->proc_entry == NULL)
    {
        pr_err("create_proc_entry %s failed\n", GOODIX_CONFIG_PROC_FILE);
    }

	if (ts->use_irq)
    {
		goodix_irq_enable(ts);
	}

	return 0;
}

static int goodix_ts_remove(struct i2c_client *client)
{
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

	if (ts->use_irq){
		goodix_gpio_as_output(gpio_wakeup_hdle, 0, "ctp_wakeup");
		free_irq(SW_INT_IRQNO_PIO, ts);
	}
	else
    {
        hrtimer_cancel(&ts->timer);
    }

	remove_proc_entry(GOODIX_CONFIG_PROC_FILE, NULL);
	
	cancel_work_sync(&ts->work);
	destroy_workqueue(ts->ts_workqueue);
	input_unregister_device(ts->input_dev);
	i2c_set_clientdata(client, NULL);
	kfree(ts);

	goodix_free_platform_resource();
 
 	return 0;
}

/**
 * goodix_ts_detect - Device detection callback for automatic device creation
 *
 */
static int goodix_ts_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
		pr_err("%s failed\n", __func__);
		return -ENODEV;
	}

	if (twi_id == adapter->nr) {
		printk(KERN_INFO "Detected chip gt9xx at adapter %d, address 0x%02x\n", i2c_adapter_id(adapter), client->addr);
		strlcpy(info->type, GOODIX_CTP_NAME, I2C_NAME_SIZE);
 		return 0;
	} else
		return -ENODEV;
}

static const struct i2c_device_id goodix_ts_id[] = {
	{ GOODIX_CTP_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, goodix_ts_id);

static struct i2c_driver goodix_ts_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = goodix_ts_probe,
	.remove = goodix_ts_remove,
	.id_table = goodix_ts_id,
	.driver = {
		.name = GOODIX_CTP_NAME,
		.owner = THIS_MODULE,
	},
	.address_list = u_i2c_addr.normal_i2c,
};

/**
 * goodix_ts_init - Driver install function
 *
 */
static int __devinit goodix_ts_init(void)
{
	int ret = -1;

	ret = goodix_fetch_sysconfig_para();
	if (ret != 0)
		return ret;

	ret = goodix_init_platform_resource();
	if (ret != 0)
		return ret;

	goodix_io_init(u_i2c_addr.normal_i2c[0], 20);
	
	goodix_ts_driver.detect = goodix_ts_detect;
	
	ret = i2c_add_driver(&goodix_ts_driver);
	
	return ret;
}

/**
 * goodix_ts_exit - Driver uninstall function
 *
 */
static void __exit goodix_ts_exit(void)
{
	printk(KERN_INFO "Driver gt9xx unregistered\n");
	
	i2c_del_driver(&goodix_ts_driver);
	return;
}

module_init(goodix_ts_init);
module_exit(goodix_ts_exit);

MODULE_AUTHOR("Benjamin Tissoires <benjamin.tissoires@gmail.com>");
MODULE_AUTHOR("Bastien Nocera <hadess@hadess.net>");
MODULE_DESCRIPTION("Goodix touchscreen driver");
MODULE_LICENSE("GPL v2");
