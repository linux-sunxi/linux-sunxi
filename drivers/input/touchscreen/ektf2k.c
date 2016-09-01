/*
 * Copyright (C) 2013 Paul Kocialkowski
 *
 * Based on ELAN KTF2000 touchscreen driver:
 * Copyright (C) 2010 HTC Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/earlysuspend.h>
#include <plat/sys_config.h>

#include "ctp_platform_ops.h"

#define EKTF2K_NAME		"ektf2k"

/* Data messages */
#define EKTF2K_DATA_HELLO	0x55
#define EKTF2K_DATA_RESET	0x77
#define EKTF2K_DATA_CALIB	0xa8
#define EKTF2K_DATA_REPEAT	0xa6

/* Types for solicited messages */
#define EKTF2K_RESPONSE		0x52
#define EKTF2K_REQUEST		0x53
#define EKTF2K_WRITE		0x54

/* Commands for solicited messages */
#define EKTF2K_FW_VER		0x00
#define EKTF2K_POWER_STATE	0x50
#define EKTF2K_FINGER_STATE	0x51
#define EKTF2K_HEIGHT		0x60
#define EKTF2K_WIDTH		0x63
#define EKTF2K_PACKET_STATE	0x8e
#define EKTF2K_FW_ID		0xF0

/* Commands for unsolicited messages */
#define EKTF2K_NOISE		0x40
#define EKTF2K_REPORT		0x5D
#define EKTF2K_CALIB		0x66

/* Values */
#define EKTF2K_VAL_NOISY	0x41
#define EKTF2K_VAL_PKT_ON	0x00
#define EKTF2K_VAL_PKT_OFF	0x01
#define EKTF2K_VAL_PWR_NORMAL	0x01
#define EKTF2K_VAL_PWR_SLEEP	0x00
#define EKTF2K_VAL_FINGER_ON	0x01
#define EKTF2K_VAL_FINGER_OFF	0x00

#define EKTF2K_CTP_NAME		"ekt3632"
#define EKTF2K_CTP_IRQ_MODE	LOW_LEVEL

struct ektf2k_ctp_data {
	void *__iomem gpio_addr;

	user_gpio_set_t gpio_int_info;
	int gpio_int_cfg[8];
	int gpio_int;

	int gpio_reset;
	int gpio_wakeup;

	int irq;

	int twi_id;
	int twi_addr;

	int screen_max_x;
	int screen_max_y;
	int revert_x_flag;
	int revert_y_flag;
	int exchange_x_y_flag;
};

struct ektf2k_data {
	struct ektf2k_ctp_data *ctp_data;
	struct i2c_client *client;
	struct input_dev *input;

	//struct early_suspend early_suspend;

	int firmware_version;

	int width;
	int height;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ektf2k_early_suspend(struct early_suspend *h);
static void ektf2k_late_resume(struct early_suspend *h);
#endif

/*
 * EKTF2K CTP
 */

static struct ektf2k_ctp_data ektf2k_ctp_data = {
	.gpio_int_cfg = {
		PIO_INT_CFG0_OFFSET, PIO_INT_CFG1_OFFSET,
		PIO_INT_CFG2_OFFSET, PIO_INT_CFG3_OFFSET
	},
	.irq = SW_INT_IRQNO_PIO,
};

static int ektf2k_ctp(struct ektf2k_ctp_data *ctp_data)
{
	script_parser_value_type_t type;
	char ctp_name[I2C_NAME_SIZE];
	int ctp_used;
	int rc;

	rc = script_parser_fetch("ctp_para", "ctp_used", &ctp_used, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_used\n", __func__);
		return -1;
	}

	if (ctp_used != 1) {
		printk(KERN_INFO "%s: CTP not in use\n", __func__);
		return -1;
	}

	type = SCRIPT_PARSER_VALUE_TYPE_STRING;
	memset(&ctp_name, 0, sizeof(ctp_name));

	rc = script_parser_fetch_ex("ctp_para", "ctp_name", (int *) &ctp_name, &type, sizeof(ctp_name) / sizeof(int));
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_name\n", __func__);
		return -1;
	}

	if (strcmp(EKTF2K_CTP_NAME, ctp_name) != 0) {
		printk(KERN_INFO "%s: CTP name %s doesn't match %s\n", __func__, ctp_name, EKTF2K_CTP_NAME);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_twi_id", &ctp_data->twi_id, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_twi_id\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_twi_addr", &ctp_data->twi_addr, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_twi_addr\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_screen_max_x", &ctp_data->screen_max_x, 1);
	if (rc != SCRIPT_PARSER_OK || !ctp_data->screen_max_x) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_screen_max_x\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_screen_max_y", &ctp_data->screen_max_y, 1);
	if (rc != SCRIPT_PARSER_OK || !ctp_data->screen_max_y) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_screen_max_y\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_revert_x_flag", &ctp_data->revert_x_flag, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_revert_x_flag\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_revert_y_flag", &ctp_data->revert_y_flag, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_revert_y_flag\n", __func__);
		return -1;
	}

	rc = script_parser_fetch("ctp_para", "ctp_exchange_x_y_flag", &ctp_data->exchange_x_y_flag, 1);
	if (rc != SCRIPT_PARSER_OK) {
		printk(KERN_ERR "%s: Failed to parse value for key ctp_exchange_x_y_flag\n", __func__);
		return -1;
	}

	printk(KERN_INFO "%s: CTP %s, twi id %d, twi addr 0x%x\n", __func__,
		ctp_name, ctp_data->twi_id, ctp_data->twi_addr);

	return 0;
}

static void ektf2k_ctp_coordinates(struct ektf2k_ctp_data *ctp_data, int *x, int *y,
	int x_max, int y_max)
{
	int t;

	*x = (*x * ctp_data->screen_max_x) / x_max;
	*y = (*y * ctp_data->screen_max_y) / y_max;

	if (ctp_data->revert_x_flag)
		*x = ctp_data->screen_max_x - *x;

	if (ctp_data->revert_y_flag)
		*y = ctp_data->screen_max_y - *y;

	if (ctp_data->exchange_x_y_flag) {
		t = *x;
		*x = *y;
		*y = t;
	}
}

static int ektf2k_ctp_gpio_get_value(struct ektf2k_ctp_data *ctp_data)
{
	void *gpio_int_addr = NULL;
	__u32 reg_val = 0;
	int state;

	gpio_int_addr = (void *) ((int) ctp_data->gpio_addr + PIO_INT_STAT_OFFSET);

	reg_val = readl(gpio_int_addr);
	if (reg_val & (1 << ctp_data->gpio_int_info.port_num))
		state = 1;
	else
		state = 0;

	reg_val = reg_val & (1 << ctp_data->gpio_int_info.port_num);
	writel(reg_val, gpio_int_addr);

	return state;
}

static int ektf2k_ctp_gpio(struct ektf2k_ctp_data *ctp_data)
{
	ctp_data->gpio_addr = ioremap(PIO_BASE_ADDRESS, PIO_RANGE_SIZE);
	if (ctp_data->gpio_addr == NULL)
		return -ENOMEM;

	ctp_data->gpio_reset = gpio_request_ex("ctp_para", "ctp_reset");
	if (!ctp_data->gpio_reset)
		printk(KERN_ERR "%s: Failed to get reset GPIO\n", __func__);

	ctp_data->gpio_wakeup = gpio_request_ex("ctp_para", "ctp_wakeup");
	if (!ctp_data->gpio_wakeup)
		printk(KERN_ERR "%s: Failed to get wakeup GPIO\n", __func__);

	return 0;
}

static int ektf2k_ctp_reset(struct ektf2k_ctp_data *ctp_data)
{
	int rc;

	if (!ctp_data->gpio_reset)
		return 0;

	rc = gpio_write_one_pin_value(ctp_data->gpio_reset, 0, "ctp_reset");
	if (rc != EGPIO_SUCCESS)
		printk(KERN_ERR "%s: Failed to set GPIO to 0\n", __func__);

	mdelay(15);

	rc = gpio_write_one_pin_value(ctp_data->gpio_reset, 1, "ctp_reset");
	if (rc != EGPIO_SUCCESS)
		printk(KERN_ERR "%s: Failed to set GPIO to 1\n", __func__);

	mdelay(15);

	return 0;
}

static int ektf2k_ctp_irq_mux(struct ektf2k_ctp_data *ctp_data)
{
	void *gpio_int_addr = NULL;
	__u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;

	ctp_data->gpio_int = gpio_request_ex("ctp_para", "ctp_int_port");
	if (!ctp_data->gpio_int) {
		printk(KERN_ERR "%s: Failed to request int GPIO\n", __func__);
		return -1;
	}

	gpio_get_one_pin_status(ctp_data->gpio_int, &ctp_data->gpio_int_info, "ctp_int_port", 1);

	reg_num = ctp_data->gpio_int_info.port_num % 8;
	reg_addr = ctp_data->gpio_int_info.port_num / 8;

	gpio_int_addr = (void *) ((int) ctp_data->gpio_addr + ctp_data->gpio_int_cfg[reg_addr]);

	reg_val = readl(gpio_int_addr);
	reg_val &= ~(7 << (reg_num * 4));
	reg_val |= EKTF2K_CTP_IRQ_MODE << (reg_num * 4);
	writel(reg_val, gpio_int_addr);

	ektf2k_ctp_gpio_get_value(ctp_data);

	gpio_int_addr = (void *) ((int) ctp_data->gpio_addr + PIO_INT_CTRL_OFFSET);

	reg_val = readl(gpio_int_addr);
	reg_val |= 1 << (ctp_data->gpio_int_info.port_num);
	writel(reg_val, gpio_int_addr);

	return 0;
}

/*
 * EKTF2K I/O
 */

static int ektf2k_send(struct ektf2k_data *data, char *buffer, int length)
{
	int rc;

	rc = i2c_master_send(data->client, buffer, length);
	if (rc != length) {
		printk(KERN_ERR "%s: Number of sent bytes (%d) doesn't match\n", __func__, rc);
		return -1;
	}

	return 0;
}

static int ektf2k_recv(struct ektf2k_data *data, char *buffer, int length)
{
	int rc;

	rc = i2c_master_recv(data->client, buffer, length);
	if (rc != length) {
		printk(KERN_ERR "%s: Number of recieved bytes (%d) doesn't match\n", __func__, rc);
		return -1;
	}

	return 0;
}

static int ektf2k_transcv(struct ektf2k_data *data, char *buffer, int length)
{
	int rc;

	rc = i2c_master_send(data->client, buffer, length);
	if (rc != length) {
		printk(KERN_ERR "%s: Number of sent bytes (%d) doesn't match\n", __func__, rc);
		return -1;
	}

	msleep(10);

	rc = i2c_master_recv(data->client, buffer, length);
	if (rc != length) {
		printk(KERN_ERR "%s: Number of received bytes (%d) doesn't match\n", __func__, rc);
		return -1;
	}

	if (buffer[0] != EKTF2K_RESPONSE) {
		printk(KERN_ERR "%s: Not a valid response\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * EKTF2K data messages
 */

/*
static int ektf2k_reset(struct ektf2k_data *data)
{
	char buffer[4];
	int rc;

	memset(&buffer, EKTF2K_DATA_RESET, sizeof(buffer));

	rc = ektf2k_send(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0)
		return -1;

	return 0;
}

static int ektf2k_calib(struct ektf2k_data *data)
{
	char buffer[4];
	int rc;

	memset(&buffer, EKTF2K_DATA_CALIB, sizeof(buffer));

	rc = ektf2k_send(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0)
		return -1;

	return 0;
}
*/

static int ektf2k_hello(struct ektf2k_data *data)
{
	char buffer[4];
	int rc;
	int i;

	rc = ektf2k_recv(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0)
		return -1;

	for (i = 0 ; i < sizeof(buffer) ; i++) {
		if (buffer[i] != EKTF2K_DATA_HELLO) {
			printk(KERN_ERR "%s: Received data doesn't match hello\n", __func__);
			return -1;
		}
	}

	return 0;
}

/*
 * EKTF2K solicited messages
 */

static int ektf2k_set_packet_state(struct ektf2k_data *data, int enabled)
{
	char buffer[4] = { EKTF2K_WRITE, EKTF2K_PACKET_STATE, 0x00, 0x01 };
	int rc;

	if (enabled)
		buffer[2] = EKTF2K_VAL_PKT_ON;
	else
		buffer[2] = EKTF2K_VAL_PKT_OFF;

	rc = ektf2k_send(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to set packet state\n", __func__);
		return -1;
	}

	return 0;
}

static int ektf2k_get_packet_state(struct ektf2k_data *data)
{
	char buffer[4] = { EKTF2K_REQUEST, EKTF2K_PACKET_STATE, 0x00, 0x01 };
	int state;
	int rc;

	rc = ektf2k_transcv(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to set power state\n", __func__);
		return -1;
	}

	state = buffer[2];

	if (state == EKTF2K_VAL_PKT_OFF)
		return 0;
	else
		return 1;
}

static int ektf2k_set_power_state(struct ektf2k_data *data, int enabled)
{
	char buffer[4] = { EKTF2K_WRITE, EKTF2K_POWER_STATE, 0x00, 0x01 };
	int rc;

	if (enabled)
		buffer[1] |= (EKTF2K_VAL_PWR_NORMAL << 3);
	else
		buffer[1] |= (EKTF2K_VAL_PWR_SLEEP << 3);

	rc = ektf2k_send(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to set power state\n", __func__);
		return -1;
	}

	return 0;
}

static int ektf2k_get_power_state(struct ektf2k_data *data)
{
	char buffer[4] = { EKTF2K_REQUEST, EKTF2K_POWER_STATE, 0x00, 0x01 };
	int state;
	int rc;

	rc = ektf2k_transcv(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to set power state\n", __func__);
		return -1;
	}

	state = buffer[1] & (1 << 3);

	if (state == EKTF2K_VAL_PWR_SLEEP)
		return 0;
	else
		return 1;
}
/*
static int ektf2k_get_finger_state(struct ektf2k_data *data)
{
	char buffer[4] = { EKTF2K_REQUEST, EKTF2K_FINGER_STATE, 0x00, 0x01 };
	int state;
	int rc;

	rc = ektf2k_transcv(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to set power state\n", __func__);
		return -1;
	}

	state = buffer[2];

	if (state == EKTF2K_VAL_FINGER_OFF)
		return 0;
	else
		return 1;
}
*/
static int ektf2k_get_firmware_infos(struct ektf2k_data *data)
{
	char buffer_fw_ver[4] = { EKTF2K_REQUEST, EKTF2K_FW_VER, 0x00, 0x01 };
	char buffer_width[4] = { EKTF2K_REQUEST, EKTF2K_WIDTH, 0x00, 0x00 };
	char buffer_height[4] = { EKTF2K_REQUEST, EKTF2K_HEIGHT, 0x00, 0x00 };
	int rc;

	rc = ektf2k_transcv(data, (char *) &buffer_fw_ver, sizeof(buffer_fw_ver));
	if (rc < 0 || buffer_fw_ver[1] != EKTF2K_FW_VER) {
		printk(KERN_ERR "%s: Failed to get firmware version\n", __func__);
		return -1;
	}

	data->firmware_version = (buffer_fw_ver[2] << 8) | (buffer_fw_ver[3] & 0xf0);

	msleep(10);

	rc = ektf2k_transcv(data, (char *) &buffer_width, sizeof(buffer_width));
	if (rc < 0 || buffer_width[1] != EKTF2K_WIDTH) {
		printk(KERN_ERR "%s: Failed to get width\n", __func__);
		return -1;
	}

	data->width = ((buffer_width[3] & 0xf0) << 4) | buffer_width[2];

	msleep(10);

	rc = ektf2k_transcv(data, (char *) &buffer_height, sizeof(buffer_height));
	if (rc < 0 || buffer_height[1] != EKTF2K_HEIGHT) {
		printk(KERN_ERR "%s: Failed to get width\n", __func__);
		return -1;
	}

	data->height = ((buffer_height[3] & 0xf0) << 4) | buffer_height[2];

	printk(KERN_INFO "%s: version %x, width %d, height %d\n", __func__,
		data->firmware_version, data->width, data->height);

	return 0;
}

/*
 * EKTF2K unsolicited messages
 */

static int ektf2k_report_coordinates(struct ektf2k_data *data, char *buffer,
	int *x, int *y)
{
	if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0)
		return -1;

	*x = (buffer[0] & 0x0f);
	*x <<= 8;
	*x |= buffer[2];

	*y = (buffer[0] & 0xf0);
	*y <<= 4;
	*y |= buffer[1];

	return 0;
}

static void ektf2k_report(struct ektf2k_data *data, char *buffer)
{
	int count;
	int index, x, y;
	int rc;
	int i;

	count = buffer[1] & 0x07;

	if (count > 0) {
		for (i = 0; i < 5 ; i++) {
			index = 2 + i * 3;
			rc = ektf2k_report_coordinates(data, (char *) &buffer[index], &x, &y);
			if (rc < 0) {
				input_report_key(data->input, BTN_TOUCH, 1);
				continue;
			}

			ektf2k_ctp_coordinates(data->ctp_data, &x, &y, data->width, data->height);

			printk(KERN_ERR "%s: sending input MT event for count %d", __func__, i);

			/*input_report_abs(data->input, ABS_MT_POSITION_X, x);
			input_report_abs(data->input, ABS_MT_POSITION_Y, y);
			input_report_abs(data->input, ABS_MT_TOUCH_MAJOR, 1);*/
			input_report_abs(data->input, ABS_X, x);
			input_report_abs(data->input, ABS_Y, y);
			input_report_abs(data->input, ABS_PRESSURE, 1);
			input_mt_sync(data->input);
			
		}
	} else {
		printk(KERN_ERR "%s: sending input for 0 fingers ", __func__);
		input_report_key(data->input, BTN_TOUCH, 0);
		input_mt_sync(data->input);
	}

	input_sync(data->input);
}

static irqreturn_t ektf2k_irq(int irq, void *device_id)
{
	struct ektf2k_data *data;
	char buffer[25];
	int rc;

	data = (struct ektf2k_data *) device_id;

	ektf2k_ctp_gpio_get_value(data->ctp_data);

	rc = ektf2k_recv(data, (char *) &buffer, sizeof(buffer));
	if (rc < 0)
		return IRQ_HANDLED;

	switch (buffer[0]) {
	case EKTF2K_REPORT:
		ektf2k_report(data, buffer);
		break;
	case EKTF2K_NOISE:
		printk(KERN_INFO "%s: Noise message\n", __func__);

		if (buffer[1] == EKTF2K_VAL_NOISY)
			printk(KERN_INFO "%s: Environment is noisy\n", __func__);
		else
			printk(KERN_INFO "%s: Environment is normal\n", __func__);
		break;
	case EKTF2K_CALIB:
		printk(KERN_INFO "%s: Calibration message\n", __func__);
		break;
	case EKTF2K_DATA_HELLO:
		printk(KERN_INFO "%s: Hello message\n", __func__);
		break;
	}

	return IRQ_HANDLED;
}

/*
 * EKTF2K driver
 */


static int ektf2k_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ektf2k_data *data;
	int rc;

	printk(KERN_INFO "%s()\n", __func__);

	data = kzalloc(sizeof(struct ektf2k_data), GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	data->ctp_data = &ektf2k_ctp_data;
	data->client = client;

	i2c_set_clientdata(client, data);

	rc = ektf2k_ctp_gpio(data->ctp_data);
	if (rc < 0)
		goto error_data_free;

	rc = ektf2k_ctp_irq_mux(data->ctp_data);
	if (rc < 0)
		goto error_data_free;

	rc = ektf2k_ctp_reset(data->ctp_data);
	if (rc < 0)
		goto error_data_free;

	ektf2k_hello(data);

	msleep(10);

	rc = ektf2k_get_firmware_infos(data);
	if (rc < 0)
		goto error_data_free;

	data->input = input_allocate_device();
	if (data->input == NULL)
		goto error_data_free;

	data->input->name = EKTF2K_NAME;
	set_bit(EV_SYN, data->input->evbit);
	set_bit(EV_KEY, data->input->evbit);
	set_bit(EV_ABS, data->input->evbit);
	set_bit(BTN_TOUCH, data->input->keybit);
	set_bit(INPUT_PROP_DIRECT, data->input->propbit);


	input_set_abs_params(data->input, ABS_X, 0, data->ctp_data->screen_max_x, 0, 0);
	input_set_abs_params(data->input, ABS_Y, 0, data->ctp_data->screen_max_y, 0, 0);
	input_set_abs_params(data->input, ABS_PRESSURE, 0, 255, 0, 0);

	rc = input_register_device(data->input);
	if (rc < 0) {
		printk(KERN_ERR "%s: Failed to register input\n", __func__);
		goto error_input_free;
	}

	if (data->ctp_data->irq) {
		client->irq = data->ctp_data->irq;
		rc = request_threaded_irq(client->irq, NULL, ektf2k_irq, IRQF_TRIGGER_LOW | IRQF_ONESHOT, client->name, data);
		if (rc < 0) {
			printk(KERN_ERR "%s: Failed to request IRQ\n", __func__);
			goto error_input_unregister;
		}
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1;
	data->early_suspend.suspend = ektf2k_early_suspend;
	data->early_suspend.resume = ektf2k_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	return 0;

error_input_unregister:
	input_unregister_device(data->input);

error_input_free:
	input_free_device(data->input);

error_data_free:
	kfree(data);

	return -1;
}

static int ektf2k_remove(struct i2c_client *client)
{
	struct ektf2k_data *data;

	printk(KERN_INFO "%s()\n", __func__);

	data = i2c_get_clientdata(client);
	if (data == NULL)
		return -1;

	//if (data->early_suspend.suspend != NULL && data->early_suspend.resume != NULL)
		//unregister_early_suspend(&data->early_suspend);

	if (data->ctp_data->irq)
		free_irq(data->ctp_data->irq, data);

	if (data->ctp_data->gpio_int)
		gpio_release(data->ctp_data->gpio_int, 2);
	if (data->ctp_data->gpio_reset)
		gpio_release(data->ctp_data->gpio_reset, 2);
	if (data->ctp_data->gpio_wakeup)
		gpio_release(data->ctp_data->gpio_wakeup, 2);

	if (data->input != NULL) {
		input_unregister_device(data->input);
		input_free_device(data->input);
	}

	i2c_set_clientdata(client, NULL);

	kfree(data);

	return 0;
}

static int ektf2k_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	if (ektf2k_ctp_data.twi_id == client->adapter->nr) {
		printk(KERN_INFO "%s: Detected %s, adapter %d, addr 0x%x\n", __func__,
			EKTF2K_NAME, i2c_adapter_id(client->adapter), client->addr);
		strlcpy(info->type, EKTF2K_NAME, I2C_NAME_SIZE);
		return 0;
	}

	return -ENODEV;
}

static int ektf2k_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct ektf2k_data *data;

	printk(KERN_INFO "%s()\n", __func__);

	data = i2c_get_clientdata(client);

	disable_irq(client->irq);

	ektf2k_set_packet_state(data, 0);
	ektf2k_set_power_state(data, 0);

	return 0;
}

static int ektf2k_resume(struct i2c_client *client)
{
	struct ektf2k_data *data;
	int enabled;

	printk(KERN_INFO "%s()\n", __func__);

	data = i2c_get_clientdata(client);

	ektf2k_set_power_state(data, 1);
	msleep(10);
	enabled = ektf2k_get_power_state(data);
	if (!enabled)
		printk(KERN_ERR "%s: Failed to enable power\n", __func__);

	ektf2k_set_packet_state(data, 1);
	msleep(10);
	enabled = ektf2k_get_packet_state(data);
	if (!enabled)
		printk(KERN_ERR "%s: Failed to enable packet\n", __func__);

	enable_irq(client->irq);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ektf2k_early_suspend(struct early_suspend *h)
{
	struct ektf2k_data *data;

	data = container_of(h, struct ektf2k_data, early_suspend);
	ektf2k_suspend(data->client, PMSG_SUSPEND);
}

static void ektf2k_late_resume(struct early_suspend *h)
{
	struct ektf2k_data *data;

	data = container_of(h, struct ektf2k_data, early_suspend);
	ektf2k_resume(data->client);
}
#endif

static const struct i2c_device_id ektf2k_id[] = {
	{ EKTF2K_NAME, 0 },
	{ }
};

static unsigned short ektf2k_address_list[2] = { 0 };

static struct i2c_driver ektf2k_driver = {
	.class		= I2C_CLASS_HWMON,
	.probe		= ektf2k_probe,
	.remove		= ektf2k_remove,
	.detect		= ektf2k_detect,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= ektf2k_suspend,
	.resume		= ektf2k_resume,
#endif
	.id_table	= ektf2k_id,
	.driver		= {
		.name = EKTF2K_NAME,
	},
	.address_list	= (const unsigned short *) &ektf2k_address_list,
};

static int __devinit ektf2k_init(void)
{
	int rc;

	printk(KERN_INFO "%s()\n", __func__);

	rc = ektf2k_ctp(&ektf2k_ctp_data);
	if (rc < 0)
		return -ENODEV;

	ektf2k_address_list[0] = ektf2k_ctp_data.twi_addr;
	ektf2k_address_list[1] = I2C_CLIENT_END;

	return i2c_add_driver(&ektf2k_driver);
}

static void __exit ektf2k_exit(void)
{
	printk(KERN_INFO "%s()\n", __func__);

	i2c_del_driver(&ektf2k_driver);
}

module_init(ektf2k_init);
module_exit(ektf2k_exit);

MODULE_AUTHOR("Paul Kocialkowski <contact@paulk.fr>");
MODULE_DESCRIPTION("Elan KTF2K Touchscreen Driver");
MODULE_LICENSE("GPL");
