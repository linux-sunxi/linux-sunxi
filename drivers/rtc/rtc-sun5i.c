/*
 * drivers\rtc\rtc-sun5i.c
 * An I2C driver for the Philips PCF8563 RTC
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * huangxin <huangxin@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <mach/sys_config.h>
#define DRV_VERSION "0.4.3"

/*control&flag status register*/
#define PCF8563_REG_ST1		0x00 /* status */

#define PCF8563_REG_ST2		0x01
/*bit 0:TIE(timer interrupt enable),bit 1:AIE(alarm interrupt enable)*/
# 	define PCF8563_REG_AIE	(3<<0)
/*bit 2:TF(timer flag), bit 3: AF(alarm flag)*/
# 	define PCF8563_REG_AF	(3<<2)

/*second ~ year register*/
#define PCF8563_REG_SC		0x02 /* datetime */
#define PCF8563_REG_MN		0x03
#define PCF8563_REG_HR		0x04
#define PCF8563_REG_DM		0x05
#define PCF8563_REG_DW		0x06
#define PCF8563_REG_MO		0x07
#define PCF8563_REG_YR		0x08

/*alarm function register*/
#define PCF8563_REG_AMN		0x09 /* alarm minute*/
/*
* minute alarm, the minute alarm information coded in BCD format;
* value = 00 to 59.
*/
#	define PCF8563_REG_AMN_COUNT	(0<<0)
/*AE=0, minute alarm is enabled,AE=1; minute alarm is disabled*/
# 	define PCF8563_REG_AMN_AE	(1<<7)

#define PCF8563_REG_AHR		0x0A /* alarm hour*/
/*
* hour alarm, the hour alarm information coded in BCD format;
* value = 00 to 23.
*/
#	define PCF8563_REG_AHR_COUNT	(0<<0)
/*AE=0, hour alarm is enabled,AE=1; hour alarm is disabled*/
# 	define PCF8563_REG_AHR_AE	(1<<7)

#define PCF8563_REG_ADM		0x0B /* alarm day*/
/*
* day alarm, the day alarm information coded in BCD format;
* value = 01 to 31.
*/
#	define PCF8563_REG_ADM_COUNT	(0<<0)
/*AE=0, day alarm is enabled,AE=1; day alarm is disabled*/
# 	define PCF8563_REG_ADM_AE	(1<<7)

#define PCF8563_REG_ADW		0x0C /* alarm weekday*/

#define ALARM_FLAG_BIT      (3)
#define ALARM_INT_BIT       (1)

/*clock out register*/
#define PCF8563_REG_CLKO	0x0D /* clock out */

/*timer function register*/
#define PCF8563_REG_TMRC	0x0E /* timer control */
#define PCF8563_REG_TMR		0x0F /* timer */

#define PCF8563_SC_LV		0x80 /* low voltage */
#define PCF8563_MO_C		0x80 /* century */

#define RTC_NAME	"pcf8563"

#define F25_ALARM
//#define RTC_ALARM_DEBUG

static struct i2c_driver pcf8563_driver;
static __u32 twi_id = 0;

/* Addresses to scan */
static union{
	unsigned short dirty_addr_buf[2];
	const unsigned short normal_i2c[2];
}u_i2c_addr = {{0x00},};

struct pcf8563 {
	struct rtc_device *rtc;
	struct i2c_client *client;
#ifdef F25_ALARM
	struct work_struct work;
#endif
	/*
	* The mutex protects alarm operations, and prevents a race
	* between the enable_irq() in the workqueue and the free_irq()
	* in the remove function.
	*/
	struct mutex mutex;
	/*
	 * The meaning of MO_C bit varies by the chip type.
	 * From PCF8563 datasheet: this bit is toggled when the years
	 * register overflows from 99 to 00
	 *   0 indicates the century is 20xx
	 *   1 indicates the century is 19xx
	 * From RTC8564 datasheet: this bit indicates change of
	 * century. When the year digit data overflows from 99 to 00,
	 * this bit is set. By presetting it to 0 while still in the
	 * 20th century, it will be set in year 2000, ...
	 * There seems no reliable way to know how the system use this
	 * bit.  So let's do it heuristically, assuming we are live in
	 * 1970...2069.
	 */
	 int c_polarity;	/* 0: MO_C=1 means 19xx, otherwise MO_C=1 means 20xx */
};

/**
 * rtc_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:
 *                    = 0; success;
 *                    < 0; err
 */
static int rtc_fetch_sysconfig_para(void)
{
	int ret = -1;
	int device_used = -1;
	__u32 twi_addr = 0;

	char name[I2C_NAME_SIZE];
	script_parser_value_type_t type = SCIRPT_PARSER_VALUE_TYPE_STRING;

	if(SCRIPT_PARSER_OK != script_parser_fetch("rtc_para", "rtc_used", &device_used, 1)){
	                printk("rtc: script_parser_fetch err. \n");
	                goto script_parser_fetch_err;
	}
	if(1 == device_used){
		if(SCRIPT_PARSER_OK != script_parser_fetch_ex("rtc_para", "rtc_name", (int *)(&name), &type, sizeof(name)/sizeof(int))){
			pr_err("%s: script_parser_fetch err. \n", __func__);
			goto script_parser_fetch_err;
		}
		if(strcmp(RTC_NAME, name)){
			pr_err("%s: name %s does not match HV_NAME. \n", __func__, name);
			return ret;
		}
		if(SCRIPT_PARSER_OK != script_parser_fetch("rtc_para", "rtc_twi_addr", &twi_addr, sizeof(twi_addr)/sizeof(__u32))){
			pr_err("%s: script_parser_fetch err. \n", name);
			goto script_parser_fetch_err;
		}
		u_i2c_addr.dirty_addr_buf[0] = twi_addr;
		u_i2c_addr.dirty_addr_buf[1] = I2C_CLIENT_END;

		if(SCRIPT_PARSER_OK != script_parser_fetch("rtc_para", "rtc_twi_id", &twi_id, 1)){
			pr_err("%s: script_parser_fetch err. \n", name);
			goto script_parser_fetch_err;
		}

	}else{
		pr_err("%s: rtc_unused. \n",  __func__);
		ret = -1;
	}
	return 0;

script_parser_fetch_err:
	pr_notice("=========rtc script_parser_fetch_err============\n");
	return ret;
}

/**
 * rtc_detect - Device detection callback for automatic device creation
 * return value:
 *                    = 0; success;
 *                    < 0; err
 */
int rtc_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if(twi_id == adapter->nr)
	{
		pr_info("%s: Detected chip %s at adapter %d, address 0x%02x\n",\
			 __func__, RTC_NAME, i2c_adapter_id(adapter), client->addr);

		strlcpy(info->type, RTC_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		printk("%s,line:%d\n", __func__, __LINE__);
		return -ENODEV;
	}
}

/*
*写时钟步骤：
*	step1:将时间装入发送缓冲区(首地址为50H)中
*	step2:取器件地址
*	step3:取写入寄存器的首地址(从00H开始写)
*	step4:写7个时间信息和2个控制命令
*	step5:写时间
*/
static int pcf8563_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
	int i, err;
	unsigned char buf[9];
	int leap_year = 0;

	/*int tm_year; years from 1900
    *int tm_mon; months since january 0-11
    *the input para tm->tm_year is the offset related 1900;
    */
	leap_year = tm->tm_year + 1900;
	if(leap_year > 2073 || leap_year < 2010) {
		dev_err(&client->dev, "rtc only supports 63（2010～2073） years\n");
		return -EINVAL;
	}
	/*hardware base time:1900, but now set the default start time to 2010*/
	tm->tm_year -= 110;
	/* month is 1..12 in RTC but 0..11 in linux*/
	tm->tm_mon  += 1;

	/*prevent the application seting the error time*/
	if(tm->tm_mon > 12){
		_dev_info(&client->dev, "set time month error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
	       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
	       tm->tm_hour, tm->tm_min, tm->tm_sec);
		switch(tm->tm_mon){
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				if(tm->tm_mday > 31){
					_dev_info(&client->dev, "set time day error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
				       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
				       tm->tm_hour, tm->tm_min, tm->tm_sec);
				}
				if((tm->tm_hour > 24)||(tm->tm_min > 59)||(tm->tm_sec > 59)){
						_dev_info(&client->dev, "set time error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
				       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
				       tm->tm_hour, tm->tm_min, tm->tm_sec);
				}
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if(tm->tm_mday > 30){
					_dev_info(&client->dev, "set time day error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
				       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
				       tm->tm_hour, tm->tm_min, tm->tm_sec);
				}
				if((tm->tm_hour > 24)||(tm->tm_min > 59)||(tm->tm_sec > 59)){
					_dev_info(&client->dev, "set time error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
				       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
				       tm->tm_hour, tm->tm_min, tm->tm_sec);
				}
				break;
			case 2:
				if((leap_year%400==0) || ((leap_year%100!=0) && (leap_year%4==0))) {
					if(tm->tm_mday > 28){
						_dev_info(&client->dev, "set time day error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
				       		tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
				       		tm->tm_hour, tm->tm_min, tm->tm_sec);
					}
					if((tm->tm_hour > 24)||(tm->tm_min > 59)||(tm->tm_sec > 59)){
						_dev_info(&client->dev, "set time error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
					       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
					       tm->tm_hour, tm->tm_min, tm->tm_sec);
					}
				}else{
					if(tm->tm_mday > 29){
						_dev_info(&client->dev, "set time day error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
					       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
					       tm->tm_hour, tm->tm_min, tm->tm_sec);
					}
					if((tm->tm_hour > 24)||(tm->tm_min > 59)||(tm->tm_sec > 59)){
						_dev_info(&client->dev, "set time error:line:%d,%d-%d-%d %d:%d:%d\n",__LINE__,
					       tm->tm_year + 2010, tm->tm_mon, tm->tm_mday,
					       tm->tm_hour, tm->tm_min, tm->tm_sec);
					}

				}
				break;
			default:
				break;
		}
		/*if the set date error,set the default time:2010:01:01:00:00:00*/
		tm->tm_sec  = 0;
		tm->tm_min  = 0;
		tm->tm_hour = 0;
		tm->tm_mday = 1;
		tm->tm_mon  = 1;
		tm->tm_year = 110;// 2010 = 1900 + 110
	}

	/* hours, minutes and seconds */
	buf[PCF8563_REG_SC] = bin2bcd(tm->tm_sec);
	buf[PCF8563_REG_MN] = bin2bcd(tm->tm_min);
	buf[PCF8563_REG_HR] = bin2bcd(tm->tm_hour);

	buf[PCF8563_REG_DM] = bin2bcd(tm->tm_mday);

	/* month, 1 - 12 */
	buf[PCF8563_REG_MO] = bin2bcd(tm->tm_mon);

	/* year and century */
	buf[PCF8563_REG_YR] = bin2bcd(tm->tm_year % 100);
	if (pcf8563->c_polarity ? (tm->tm_year >= 0) : (tm->tm_year < 0))
		buf[PCF8563_REG_MO] |= PCF8563_MO_C;

	//buf[PCF8563_REG_DW] = tm->tm_wday & 0x07;

	/* write register's data */
	for (i = 0; i < 7; i++) {
		unsigned char data[2] = { PCF8563_REG_SC + i,
						buf[PCF8563_REG_SC + i] };

		err = i2c_master_send(client, data, sizeof(data));
		if (err != sizeof(data)) {
			dev_err(&client->dev,
				"%s: err=%d addr=%02x, data=%02x\n",
				__func__, err, data[0], data[1]);
			return -EIO;
		}
	}

	return 0;
}

/*
 * In the routines that deal directly with the pcf8563 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch.
 * 读时钟步骤：
 *		step1:取器件地址
 *		step2:读取时间的首字节地址(从秒开始读)
 *		step3:读七个时间信息
 *		step4:读取时间并放入接收缓冲区中
 */
static int pcf8563_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
	unsigned char buf[13] = { PCF8563_REG_ST1 };
	int ret;
	struct i2c_msg msgs[] = {
		{ client->addr, 0, 1, buf },	/* setup read ptr */
		{ client->addr, I2C_M_RD, 13, buf },	/* read status + date */
	};

	#ifdef RTC_ALARM_DEBUG
	printk("%s,line:%d\n", __func__, __LINE__);
	#endif

	ret = i2c_transfer(client->adapter, msgs, 2);
	/* read registers */
	if (ret != 2) {
		dev_err(&client->dev, "%s: read error,ret:%d\n", __func__,ret);
		return -EIO;
	}

#if	0
	/*no clear why the low voltage detected should be check, noted 2012-1-13 9:40:56 by hx*/
	if (buf[PCF8563_REG_SC] & PCF8563_SC_LV)
		dev_info(&client->dev,
			"low voltage detected, date/time is not reliable.\n");
#endif

	tm->tm_sec = bcd2bin(buf[PCF8563_REG_SC] & 0x7F);
	tm->tm_min = bcd2bin(buf[PCF8563_REG_MN] & 0x7F);
	tm->tm_hour = bcd2bin(buf[PCF8563_REG_HR] & 0x3F); /* rtc hr 0-23 */
	tm->tm_mday = bcd2bin(buf[PCF8563_REG_DM] & 0x3F);
	tm->tm_wday = buf[PCF8563_REG_DW] & 0x07;
	tm->tm_mon = bcd2bin(buf[PCF8563_REG_MO] & 0x1F) - 1; /* month is 1..12 in RTC but 0..11 in linux*/
	tm->tm_year = bcd2bin(buf[PCF8563_REG_YR]);
	if (tm->tm_year < 70)
		tm->tm_year += 110;	/* assume we are in 2010...2079 */
	/* detect the polarity heuristically. see note above. */
	pcf8563->c_polarity = (buf[PCF8563_REG_MO] & PCF8563_MO_C) ?
		(tm->tm_year >= 100) : (tm->tm_year < 100);

	/*in A13,the mon read from rtc hardware is error? so set the datetime again?*/
	#if 0
	if (tm->tm_mon < 0) {
		tm->tm_mon = 1;
		ret = pcf8563_set_datetime(client, tm);
	}
	#endif

	/* the clock can give out invalid datetime, but we cannot return
	 * -EINVAL otherwise hwclock will refuse to set the time on bootup.
	 */
	if (rtc_valid_tm(tm) < 0)
		dev_err(&client->dev, "retrieved date/time is not valid.\n");

	return 0;
}

static int pcf8563_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_get_datetime(to_i2c_client(dev), tm);
}

static int pcf8563_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_set_datetime(to_i2c_client(dev), tm);
}

#ifdef F25_ALARM
int pcf8563_alarm_enable(struct i2c_client *client)
{
	int ret;
	int stat;
    int stat_min, stat_hour, stat_day;

	/*enable the alarm interrupt*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ST2);
    if (ret < 0) {
		goto out;
	}
	stat = ret;

	/*clear alarm flag and enable alarm interrupt*/
    stat &= ~(1<<3);
    stat |=  (1<<1);
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_ST2, stat);
	if (ret < 0) {
		goto out;
	}

	/*enable the minute ae*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	stat_min = ret;
	stat_min &= 0x7f;
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AMN, stat_min);
	if (ret < 0) {
		goto out;
	}

	/*enable the hour ae*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	stat_hour = ret;
	stat_hour &= 0x7f;
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AHR, stat_hour);
	if (ret < 0) {
		goto out;
	}

	/*enable the day ae*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	stat_day = ret;
	stat_day &= 0x7f;
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_ADM, stat_day);
	if (ret < 0) {
		goto out;
	}

#ifdef RTC_ALARM_DEBUG
	/*************************PCF8563_REG_ST2************************/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ST2);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_ST2:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);

	/*************************PCF8563_REG_AMN************************/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_AMN:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);

	/****************PCF8563_REG_AHR**********************/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_AHR:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);

	/****************PCF8563_REG_ADM**********************/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_ADM:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);
#endif
out:
	return ret;
}

int pcf8563_alarm_disable(struct i2c_client *client) {
    int ret;
    int stat;
    int stat_min, stat_hour, stat_day;

   /*clear the alarm counter enable bit and clear the alarm flag big*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ST2);
    if (ret < 0) {
    	printk("%s,%d\n", __func__, __LINE__);
		goto out;
	}
	stat = ret;
	//clear alarm flag and disable alarm interrupt
	stat &= ~(1<<3);
    stat &= ~(1<<1);
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_ST2, stat);
	if (ret < 0) {
		printk("%s,%d\n", __func__, __LINE__);
		goto out;
	}

	/*clear the minute ae and set the minute count to 0*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	stat_min = ret;
	stat_min &= 0x0;
	stat_min |= (1<<7);
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AMN, stat_min);
	if (ret < 0) {
		goto out;
	}

	/*clear the hour ae and set the hour count to 0*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	stat_hour = ret;
	stat_hour &= 0x0;
	stat_hour |= (1<<7);
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AHR, stat_hour);
	if (ret < 0) {
		goto out;
	}

	/*clear the day ae and set the day count to 0*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	stat_day = ret;
	stat_day &= 0x0;
	stat_day |= (1<<7);
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_ADM, stat_day);
	if (ret < 0) {
		goto out;
	}

#ifdef RTC_ALARM_DEBUG
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ST2);
	printk("PCF8563_REG_ST2:%s,line:%d, ret:%d\n", __func__, __LINE__, ret);

	/**********************PCF8563_REG_AMN********************************/
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
	printk("PCF8563_REG_AMN:%s,line:%d, ret:%d\n", __func__, __LINE__, ret);

	/**********************PCF8563_REG_AHR********************************/
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
	printk("PCF8563_REG_AHR%s,line:%d, ret:%d\n", __func__, __LINE__, ret);

	/**********************for debug********************************/
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
	printk("%s,line:%d, ret:%d\n", __func__, __LINE__, ret);
	/**********************for debug********************************/
#endif
out:
	return ret;
}

static irqreturn_t pcf8563_irq_handle(int irq, void *dev_id)
{
	struct pcf8563 *pcf8563 = dev_id;
	//printk("%s,%d,irq:%d\n", __func__, __LINE__, irq);
	(void)schedule_work(&pcf8563->work);
	return IRQ_HANDLED;
}

static void pcf8563_work(struct work_struct *work)
{
	struct pcf8563 *pcf8563 = container_of(work, struct pcf8563, work);
	struct i2c_client *client = pcf8563->client;
	int stat;

	mutex_lock(&pcf8563->mutex);

	stat = i2c_smbus_read_byte_data(client, PCF8563_REG_ST2);
	if (stat < 0) {
		printk("%s,line:%d,stat:%x\n", __func__, __LINE__, stat);
		goto unlock;
	}

	stat &= (1<<3);
	if (stat) {
		//clear alarm flag and disable alarm interrupt
		stat &= ~(1<<3);
    	stat &= ~(1<<1);
		i2c_smbus_write_byte_data(client, PCF8563_REG_ST2, stat);
	 	rtc_update_irq(pcf8563->rtc, 1, RTC_AF | RTC_IRQF);
	} else {
		//printk("%s,line:%d,stat:%x\n", __func__, __LINE__, stat);
	}

unlock:
	mutex_unlock(&pcf8563->mutex);
}

static int pcf8563_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	int ret;
	int stat_min, stat_hour, stat_day, stat_mon, stat_year;
	struct rtc_time *alm_tm = &alrm->time;
	struct i2c_client *client = to_i2c_client(dev);
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);

	mutex_lock(&pcf8563->mutex);

	/*get the minute count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	stat_min = ret;
	alm_tm->tm_min = bcd2bin(stat_min & 0x7F);

	/*get the hour count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	stat_hour = ret;
	alm_tm->tm_hour = bcd2bin(stat_hour & 0x3F);

	/*get the day count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	stat_day = ret;
	alm_tm->tm_hour = bcd2bin(stat_day & 0x3F);

	/*get the month count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_MO);
    if (ret < 0) {
		goto out;
	}
	stat_mon = ret;
	/* month is 1..12 in RTC but 0..11 in linux*/
	alm_tm->tm_mon = bcd2bin(stat_mon & 0x1F) - 1;

	/*get the year count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_YR);
    if (ret < 0) {
		goto out;
	}
	stat_year = ret;
	alm_tm->tm_year = bcd2bin(stat_year & 0xFF);
	if (alm_tm->tm_year < 70) {
		alm_tm->tm_year += 110;	/* assume we are in 2010...2079 */
	}
	ret = 0;

#ifdef RTC_ALARM_DEBUG
	printk("%s: alm_tm is secs=%d, mins=%d, hours=%d,mday=%d, mon=%d, year=%d, wday=%d\n",\
		__func__,alm_tm->tm_sec, alm_tm->tm_min, alm_tm->tm_hour, alm_tm->tm_mday, alm_tm->tm_mon, alm_tm->tm_year, alm_tm->tm_wday);
#endif

out:
	mutex_unlock(&pcf8563->mutex);
	return ret;
	return 0;
}

static int pcf8563_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;
    int ret = 0;
    int stat_min, stat_hour, stat_day;
    int month_day;
    struct rtc_time tm_now;
    unsigned char buf[3];
    struct i2c_client *client = to_i2c_client(dev);
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);

	if (client->irq < 0) {
		return -EINVAL;
	}

	mutex_lock(&pcf8563->mutex);

#ifdef RTC_ALARM_DEBUG
    printk("%s,line:%d:*****************************\n\n",__func__, __LINE__);
    printk("line:%d,%s the alarm time: year:%d, month:%d, day:%d. hour:%d.minute:%d.second:%d\n",\
    __LINE__, __func__, tm->tm_year, tm->tm_mon,\
    	 tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
   	printk("*****************************\n\n");
#endif

    ret = pcf8563_rtc_read_time(dev, &tm_now);

#ifdef RTC_ALARM_DEBUG
    printk("line:%d,%s the current time: year:%d, month:%d, day:%d. hour:%d.minute:%d.second:%d\n",\
    __LINE__, __func__, tm_now.tm_year, tm_now.tm_mon,\
    	 tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
   	printk("*****************************\n\n");
#endif

	ret = pcf8563_alarm_disable(client);
	if (ret < 0) {
		goto out;
	}
	/*adjust the alarm time*/
	if (tm->tm_sec >= 30) {
		tm->tm_min = tm->tm_min + 1;
	}
	if (tm->tm_min >= 60) {
		tm->tm_hour = tm->tm_hour + 1;
		tm->tm_min 	= tm->tm_min - 60;
	}
	if (tm->tm_hour >= 24) {
		tm->tm_mday = tm->tm_mday + 1;
		tm->tm_hour = tm->tm_hour - 24;
	}
	/* month is 0..11 in linux*/
	month_day = rtc_month_days(tm_now.tm_mon, (tm_now.tm_year + 1900));
	if(tm->tm_mday > month_day) {
		tm->tm_mday = tm->tm_mday - month_day;
	}

	if(tm->tm_mday > 31) {
		dev_err(dev, "The time or date can`t set, The day range of 0 to 31\n");
		mutex_unlock(&pcf8563->mutex);
		return -EINVAL;
	}

    buf[0] = bin2bcd(tm->tm_min);
	buf[1] = bin2bcd(tm->tm_hour);
	buf[2] = bin2bcd(tm->tm_mday);

#ifdef RTC_ALARM_DEBUG
/*%x表示按16进制输出;such as int a = 16,%02x:输出10,%03x:输出:010,%04x:输出:0010*/
	printk("%s, line:%d, buf[0]:%02x, buf[1]:%02x, buf[2]:%02x\n", \
		__func__, __LINE__, buf[0], buf[1], buf[2]);
#endif

	/*set the minute count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	stat_min = ret;
	stat_min |= buf[0];
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AMN, stat_min);
	if (ret < 0) {
		goto out;
	}
	/*set the hour count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	stat_hour = ret;
	stat_hour |= buf[1];
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_AHR, stat_hour);
	if (ret < 0) {
		goto out;
	}

	/*set the day count*/
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	stat_day = ret;
	stat_day |= buf[2];
	ret = i2c_smbus_write_byte_data(client, PCF8563_REG_ADM, stat_day);
	if (ret < 0) {
		goto out;
	}

	/* enable or disable alarm */
	if (alrm->enabled) {
		ret = pcf8563_alarm_enable(client);
	} else {
		ret = pcf8563_alarm_disable(client);
	}

	if (ret < 0) {
		goto out;
	}

#ifdef RTC_ALARM_DEBUG
   	printk("...........................\n");
   	printk("debug alarm count!!!\n");
	printk("alarm minute debug PCF8563_REG_AMN\n");
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AMN);
    if (ret < 0) {
		goto out;
	}
	printk("%s,line:%d,PCF8563_REG_AMN:%x\n", __func__, __LINE__, ret);

   	printk("...........................\n");
   	printk("alarm hour debug PCF8563_REG_AHR\n");
 	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_AHR);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_AHR:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);

   	printk("...........................\n");
   	printk("alarm day debug PCF8563_REG_ADM\n");
    ret = i2c_smbus_read_byte_data(client, PCF8563_REG_ADM);
    if (ret < 0) {
		goto out;
	}
	printk("PCF8563_REG_ADM:%s,line:%d,ret:%x\n", __func__, __LINE__, ret);

   	printk("...........................\n");
   	printk("date minute debug PCF8563_REG_MN\n");
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_MN);
    if (ret < 0) {
		goto out;
	}
	printk("%s,line:%d,PCF8563_REG_MN:%x\n", __func__, __LINE__, ret);

   	printk("...........................\n");
   	printk("date hour debug PCF8563_REG_HR\n");
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_HR);
    if (ret < 0) {
		goto out;
	}
	printk("%s,line:%d,PCF8563_REG_HR:%x\n", __func__, __LINE__, ret);

   	printk("...........................\n");
   	printk("date day debug PCF8563_REG_DM\n");
	ret = i2c_smbus_read_byte_data(client, PCF8563_REG_DM);
    if (ret < 0) {
		goto out;
	}
	printk("%s,line:%d,PCF8563_REG_DM:%x\n", __func__, __LINE__, ret);
#endif

out:
	mutex_unlock(&pcf8563->mutex);
	return ret;
}

static int pcf8563_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	int ret = 0;
	struct i2c_client *client = to_i2c_client(dev);

	if (!enabled) {
		ret = pcf8563_alarm_disable(client);
		if (ret < 0) {
			goto out;
		}
	}

	return 0;
out:
	return ret;
}
#endif

static const struct rtc_class_ops pcf8563_rtc_ops = {
	.read_time	= pcf8563_rtc_read_time,
	.set_time	= pcf8563_rtc_set_time,
#ifdef F25_ALARM
	.read_alarm	= pcf8563_read_alarm,
	.set_alarm	= pcf8563_set_alarm,
	.alarm_irq_enable = pcf8563_rtc_alarm_irq_enable,
#endif
};

static int pcf8563_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct pcf8563 *pcf8563;
	int err = 0;
	int ret = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	pcf8563 = kzalloc(sizeof(struct pcf8563), GFP_KERNEL);
	if (!pcf8563)
		return -ENOMEM;

	dev_info(&client->dev, "chip found, driver version " DRV_VERSION "\n");

	pcf8563->client = client;
	i2c_set_clientdata(client, pcf8563);
#ifdef F25_ALARM
	INIT_WORK(&pcf8563->work, pcf8563_work);
#endif
	mutex_init(&pcf8563->mutex);

	pcf8563->rtc = rtc_device_register(pcf8563_driver.driver.name,
				&client->dev, &pcf8563_rtc_ops, THIS_MODULE);

	if (IS_ERR(pcf8563->rtc)) {
		err = PTR_ERR(pcf8563->rtc);
#ifdef F25_ALARM
		goto exit_irq;
#endif
	}

#ifdef RTC_ALARM_DEBUG
	printk("%s, line:%d, client->irq:%d\n", __func__, __LINE__, client->irq);
#endif

	#ifdef F25_ALARM
	if (client->irq >= 0) {
		err = request_irq(client->irq, pcf8563_irq_handle, IRQF_SHARED|IRQF_DISABLED, "pcf8563", pcf8563);
		if (err < 0) {
			dev_err(&client->dev, "pcf8563_probe: request irq failed\n");
			goto exit_kfree;
		}
	}
	#endif

	ret = pcf8563_alarm_disable(client);
	if (ret < 0) {
		goto out;
	}

	return 0;

out:
#ifdef F25_ALARM
exit_irq:
	if (client->irq >= 0) {
		free_irq(client->irq, pcf8563);
	}
#endif

exit_kfree:
	kfree(pcf8563);

	return err;
}

static int pcf8563_remove(struct i2c_client *client)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);

	if (pcf8563->rtc)
		rtc_device_unregister(pcf8563->rtc);

	kfree(pcf8563);

	return 0;
}

static const struct i2c_device_id pcf8563_id[] = {
	{ RTC_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcf8563_id);

static struct i2c_driver pcf8563_driver = {
	.class = I2C_CLASS_HWMON,
	.driver			= {
		.name		= RTC_NAME,
	},
	.probe			= pcf8563_probe,
	.remove			= pcf8563_remove,
	.id_table		= pcf8563_id,
	.address_list	= u_i2c_addr.normal_i2c,
};

static int __init pcf8563_init(void)
{
	if (rtc_fetch_sysconfig_para()) {
		printk("%s,line:%d,err\n\n", __func__,__LINE__);
		return -1;
	}

#ifdef RTC_ALARM_DEBUG
	printk("%s: line:%d,after fetch_sysconfig_para:  normal_i2c: 0x%hx. normal_i2c[1]: 0x%hx \n", \
	__func__, __LINE__, u_i2c_addr.normal_i2c[0], u_i2c_addr.normal_i2c[1]);
#endif

	pcf8563_driver.detect = rtc_detect;

	return i2c_add_driver(&pcf8563_driver);
}

static void __exit pcf8563_exit(void)
{
	i2c_del_driver(&pcf8563_driver);
}

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("allwinner RTC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(pcf8563_init);
module_exit(pcf8563_exit);
