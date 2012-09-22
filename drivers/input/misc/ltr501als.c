/*  Digital Light Sensor driver for linux
 *  by XXX <XXX@allwinnertech.com>
 *
 *  Copyright (C) 2011 AllWinner Technology.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>    
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/input-polldev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <mach/sys_config.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
//#include <linux/pm.h>
//#include <linux/earlysuspend.h>
//#define ALS_SUSPEND_SUPPORTED//not finished yet
#endif

//#include <linux/mutex.h>

//static DEFINE_MUTEX(mymutex);

//#define TWI_SEL 1                //using sysconfig setting
#define ALS_SLAVE_ADDR 0x23      //default

//#define PRINT_CALL_INFO
#define PRINT_SYSFS_INFO

#define ALS_NAME                 "ltr501als"
//#define POLL_INTERVAL_MAX        10000
//#define POLL_INTERVAL            100
#define ABS_LUX                  (ABS_MISC)
#define MIN_LUX                  (0)//----to be modified
#define MAX_LUX                  (100000)//----to be modified
#define INPUT_FUZZ	             (0)
#define INPUT_FLAT	             (0)

#define ALS_INIT_DELAY           (60)//in *10ms
#define ALS_INTERVAL_LONG        (100)//in *10ms
#define ALS_INTERVAL_SHORT       (20)//in *10ms

static unsigned short log_en=0;

#ifdef LOG_ALS
#undef LOG_ALS
#endif
#define LOG_ALS(fmt, args...)     \
        do{                              \
            if(log_en==1)    printk(fmt, ##args);     \
        }while(0)

struct als_struct {//========================>
  
#ifdef ALS_SUSPEND_SUPPORTED
	struct early_suspend early_suspend;
	int suspend_indator;
#endif
  
  int enable;
  int delay;
  
//	spinlock_t          lock; /* syn */
	
  struct input_dev *input_dev;
  
  struct delayed_work  work;
	struct workqueue_struct *queue;
	
  int als_val_report;//new data to be reported
};

static unsigned short twi_addr = ALS_SLAVE_ADDR;
static unsigned short twi_id = 0xff;

static int test_dat=0;
int fdat_init_flag;
int als_val_new;//als tmp data

#define INC_POINT    3//point counts
#define DEC_POINT    5//point counts
int coef_inc[INC_POINT]={6,3,2};//weighting for each point
int coef_dec[DEC_POINT]={5,5,5,5,5};//weighting for each point

#define DELTA_M 5//delta thresh=last_lux*1/DELTA_M
#define N_AVG 5//data count for averaging
int fdat[N_AVG];//data for filtering
int fdat_idx;
int polarity;//-1/0/1
int up_limit;
int low_limit;
int up_thrsh;
int low_thrsh;

static struct i2c_client *this_client=NULL;
static struct als_struct *als_data=NULL;

/*als device iic read and write*/
int als_byte_read(unsigned char reg, unsigned char *value)
{
//  spin_lock_irq(&als_data->lock);
  
	u8 data[2];
	
	int ret;
	struct i2c_msg msg[2];
	
//  mutex_lock(&mymutex);
  
  ret=-1;
  
	//data in byte
	data[0] = reg;
	data[1] = 0xff;
	
  msg[0].addr	= this_client->addr,
  msg[0].flags	= 0,//!I2C_M_RD,
  msg[0].len	= 1,
  msg[0].buf	= data,
  
  msg[1].addr	= this_client->addr,
  msg[1].flags	= I2C_M_RD,
  msg[1].len	= 1,
  msg[1].buf	= data+1,
  
	ret = i2c_transfer(this_client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&this_client->dev, "als failed reading slave 0x%02x\n", this_client->addr);
		return ret;
	}
	
	*value = data[1];
	return 0;
}

int als_byte_write(unsigned char reg, unsigned char value)
{
	u8 data[2];
	struct i2c_msg msg;
	int ret;
	
//  mutex_lock(&mymutex);
	
	data[0] = reg;
	data[1] = value;
	
	msg.addr = this_client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;
	
//  mutex_unlock(&mymutex);
  
	ret = i2c_transfer(this_client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;
	else
	{
		dev_err(&this_client->dev, "als failed writing slave 0x%02x\n", this_client->addr);
	}
	
	return ret;
}

static int als_hw_enable(void)
{
  /*init sensor by iic*/
  int ret;
//  u8 id=0x18;
//  u8 tmp=0xff;
//  u8 offset=0x80;
//  u8 i=0,reg_s=0x80;
  
  als_byte_write(0x80, 0x03);//als enable
  als_byte_write(0x80, 0x00);//als disable
  
  als_byte_write(0x82, 0x7B);
  als_byte_write(0x83, 0x0f);
  als_byte_write(0x84, 0x00);
  als_byte_write(0x85, 0x03);//als rate
  als_byte_write(0x8f, 0x03);
  als_byte_write(0x9e, 0x02);
  
  als_byte_write(0x90, 0x01);
  als_byte_write(0x91, 0x00);
  als_byte_write(0x92, 0x00);
  als_byte_write(0x93, 0x00);
  
  //als threshold
  als_byte_write(0x97, 0x01);
  als_byte_write(0x98, 0x00);
  als_byte_write(0x99, 0x00);
  als_byte_write(0x9a, 0x00);
  
  ret = als_byte_write(0x80, 0x03);
//  ret |= als_byte_write(0x81, 0x03);//ps enable
  
  LOG_ALS("als_hw_enable %s\n",ret==0?"ok":"fail");
  
  return ret;
}

static int als_hw_disable(void)
{
  /*close sensor by iic*/
  als_byte_write(0x80,0x00);//als disable
//  als_byte_write(0x81,0x00);//ps disable
  return 0;
}

#ifdef ALS_SUSPEND_SUPPORTED
static void als_suspend(struct early_suspend *handler)
{
	LOG_ALS(KERN_INFO "als early suspend\n");
	als_data->suspend_indator = 1;
	
	als_hw_disable();
	return;
}

static void als_resume(struct early_suspend *handler)
{
	LOG_ALS(KERN_INFO "als late resume\n");
	als_data->suspend_indator = 0;
  
  als_hw_enable();
	return;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */


static void als_report_abs(void)
{
	input_report_abs(als_data->input_dev, ABS_LUX, als_data->als_val_report);//

	input_sync(als_data->input_dev);
	LOG_ALS("AW_ALS=%d\n",als_data->als_val_report);//
	       ///PS_0x%x/0x%xals_data->data[0], als_data->data[1], als_data->data[2]);
}

static int als_get_val(void)
{
//  int ret=-1;
//	u8 reg_data[4];
//	als_byte_read(ALS_SLAVE_ADDR, 0x8d, reg_data+4);
//	als_byte_read(ALS_SLAVE_ADDR, 0x8e, reg_data+5);
	int div_tmp;
	u8 als_tmp;
	
	int alsval_ch0_lo, alsval_ch0_hi;
	int alsval_ch1_lo, alsval_ch1_hi;
	int luxdata_int;
	int ratio;
	int alsval_ch0, alsval_ch1;
	int ch0_coeff, ch1_coeff;
//becareful of this u8->int
	als_byte_read(0x88, &als_tmp);
	alsval_ch1_lo = als_tmp;
	als_byte_read(0x89, &als_tmp);
	alsval_ch1_hi = als_tmp;
	als_byte_read(0x8a, &als_tmp);
	alsval_ch0_lo = als_tmp;
	als_byte_read(0x8b, &als_tmp);
	alsval_ch0_hi = als_tmp;
	
	alsval_ch1 = (alsval_ch1_hi * 256) + alsval_ch1_lo;
	alsval_ch0 = (alsval_ch0_hi * 256) + alsval_ch0_lo;

	// lux formula
//	ratio = (100 * alsval_ch1)/(alsval_ch1 + alsval_ch0);//<======================may be devided by zero, stupid driver by LITEON
  div_tmp = ( (alsval_ch1 + alsval_ch0)!=0)?(alsval_ch1 + alsval_ch0):1;
  ratio = (100 * alsval_ch1)/div_tmp;
  
	if (ratio < 45)
	{
		ch0_coeff = 17743;
		ch1_coeff = -11059;
	}
	else if ((ratio >= 45) && (ratio < 64))
	{
		ch0_coeff = 37725;
		ch1_coeff = 13363;
	}
	else if ((ratio >= 64) && (ratio < 85))
	{
		ch0_coeff = 16900;
		ch1_coeff = 1690;
	}
	else if (ratio >= 85)
	{
		ch0_coeff = 0;
		ch1_coeff = 0;
	}

	luxdata_int = ((alsval_ch0 * ch0_coeff) - (alsval_ch1 * ch1_coeff))/10000;

  return luxdata_int;
}

static int als_data_init(void)
{
  int ret = -1;
  if(1) {//!als_data
    int i=0;
//    fdat_init_flag = -1;//fdat never read
    polarity  = 0;
    fdat_idx = -1;//fdat never read
    als_val_new = 0;
    
    up_limit = 100000;
    low_limit = 25;
    low_thrsh = 10;
    
    for(i=0;i<N_AVG;i++) {
      fdat[i]=0;
    }
    ret = 0;
  }
  else {
    ret = -1;
    LOG_ALS("als data init fail.\n");
  }
  return ret;
}

//static void shift_fdat_array(int direction, int dat)
//{
//  int i;
//  for(i=0;i<N_AVG-1;i++) {
//    fdat[N_AVG-i]=fdat[N_AVG-i-1];
//  }
//  
//  fdat[0]=als_val_new;
//}

static int avg_data(int *array, int polar)
{
  int i,count,avg=0;
  
  int *coef = (polar==1)?coef_inc:coef_dec;
  
  count = (polar==1)?INC_POINT:DEC_POINT;
  
  for(i=0;i<count;i++) {
//    printk("coef[i]=%d\n",coef[i]);
    avg+=(array[i]/coef[i]);
  }
  
  return avg;
}

static unsigned int als_data_filter(void)
{
  int pol;
  int last_val;
  
  last_val=als_data->als_val_report;
  
  if(fdat_idx<0) {//als read the first value and report it directly
    int i;
    
    fdat[0]=als_val_new;
    als_data->als_val_report=als_val_new;
    for(i=0;i<N_AVG;i++)
      fdat[i]=als_val_new;
    pol=1;
    fdat_idx=0;
	  printk("ALS working... first pol=%d, val=%d\n",pol,als_val_new);
  }
  else if( fdat_idx>=0&&fdat_idx<(pol>=0)?INC_POINT:DEC_POINT ) {
    if(last_val>als_val_new) {//lux decr
      int delta;
      delta=last_val-als_val_new;
      if( delta>=(last_val/DELTA_M) ) {
        pol=-1;
        fdat[fdat_idx]=als_val_new;
        fdat_idx++;
      }
      else {
        int i;
        pol=0;
        fdat_idx=0;
        for(i=0;i<N_AVG;i++)
          fdat[i]=last_val;
      }
	    //printk("pol=%d, idx=%d, als_val_new=%d\n",pol,fdat_idx,als_val_new);
    }
    else if (last_val<als_val_new) {//lux incr
      int delta;
      delta=als_val_new-last_val;
      if( delta>=(last_val/DELTA_M) ) {
        pol=1;
        fdat[fdat_idx]=als_val_new;
        fdat_idx++;
      }
      else {
        int i;
        pol=0;
        fdat_idx=0;
        for(i=0;i<N_AVG;i++)
          fdat[i]=last_val;
      }
	    //printk("pol=%d, idx=%d, als_val_new=%d\n",pol,fdat_idx,als_val_new);
    }
    else {
      int i;
      pol=0;
      fdat_idx=0;
      for(i=0;i<N_AVG;i++)
        fdat[i]=last_val;
    }
  }
  else {
    int i;
    pol=0;
    fdat_idx=0;
    for(i=0;i<N_AVG;i++)
      fdat[i]=last_val;
  }
  
  if(pol!=0) {
    if( fdat_idx==(pol==1?INC_POINT:DEC_POINT) ) {//note this criterion dont-1
      int avg_out=avg_data(fdat,pol);
      als_data->als_val_report=avg_out;
      ////printk("avg dat=%d\n",avg_out);
      fdat_idx=0;
    }
  }
  
  //printk("pol=%d\n",pol);
  return pol;
}

static void als_read_loop(struct work_struct *work)
{
	int polar = 0;//lux changing pol
	int i;
	int interval=ALS_INTERVAL_SHORT;
	
	als_val_new = als_get_val();
	
	als_val_new = ((als_val_new)>MAX_LUX)?MAX_LUX:(als_val_new);
	
	if(als_data->als_val_report < low_thrsh) {
	  if(als_val_new <= low_thrsh) {
	    als_val_new = als_data->als_val_report;
	  }
	}
	
	polar=als_data_filter();
	
	if(polar==0) {//lux stable
	  interval=ALS_INTERVAL_LONG;
	  queue_delayed_work(als_data->queue, &als_data->work, interval);
	  //printk("polar=%d, interval=%d\n",polar,interval);
	}
	else {//lux changed
	  als_report_abs();
	  
	  if(polar==1) interval=ALS_INTERVAL_LONG;//lux incr
	    else interval=ALS_INTERVAL_SHORT;//lux decr
	  
	  queue_delayed_work(als_data->queue, &als_data->work, interval);
	  //printk("polar=%d, interval=%d\n",polar,interval);
	}
	
}

/*
 * I2C init/probing/exit functions__devinit 
 */
static int als_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
  //struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
  struct input_dev *input_dev;//to set input device attribute
  
  int err = 0;
  
  LOG_ALS(KERN_INFO "als driver-->als probe. \n");
  
	als_data = kzalloc(sizeof(struct als_struct), GFP_KERNEL);
	if (!als_data)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	
  /*init the data struct*/
  LOG_ALS("init als_data\n");
  als_data_init();
  
  this_client = client;
  
	INIT_DELAYED_WORK(&als_data->work, als_read_loop);
	als_data->queue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!als_data->queue) {
		err = -ESRCH;
  	LOG_ALS("==als create workquee error==\n");
		goto exit_create_singlethread;
	}
	
  input_dev = input_allocate_device();
  if (!input_dev) {
    dev_err(&client->dev, "als failed to allocate input device\n");
    err = -ENOMEM;
    goto exit_input_dev_alloc_failed;
  }
  
  als_data->input_dev = input_dev;
  
  input_dev->name = ALS_NAME;//set attr
  input_dev->phys = "als";
  input_dev->id.bustype = BUS_HOST;
  input_dev->id.vendor = 0x0001;
  input_dev->id.product = 0x0001;
  input_dev->id.version = 0x0100;
  
  /*register input device struct*/
  err = input_register_device(input_dev);
  if (err) {
    dev_err(&client->dev,
		"als_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
    goto exit_input_register_device_failed;
  }
  
  /*must set input device data type and range,
    or data reported will be filtered by input driver*/
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(ABS_LUX, input_dev->absbit);
  //set range
	input_set_abs_params(input_dev, ABS_LUX, MIN_LUX, MAX_LUX,
	                     INPUT_FUZZ, INPUT_FLAT);
  /*************************/
	
	LOG_ALS("als input_register_device ok: %s. \n",dev_name(&client->dev));
  
#ifdef ALS_SUSPEND_SUPPORTED
	als_data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	als_data->early_suspend.suspend = als_suspend;
	als_data->early_suspend.resume = als_resume;
	register_early_suspend(&als_data->early_suspend);
	als_data->suspend_indator = 0;
#endif
	
  //init a timer at init
  queue_delayed_work(als_data->queue, &als_data->work, ALS_INIT_DELAY);
  
//  spin_lock_init(&als_data->lock);
  
  printk("als probe ok. \n");
  return 0;
  
exit_input_register_device_failed:
  input_free_device(input_dev);
  
exit_input_dev_alloc_failed:
exit_init_hw_failed:
exit_create_singlethread:
	i2c_set_clientdata(client, NULL);
	kfree(&als_data);
	
exit_alloc_data_failed:
  
exit_check_functionality_failed:
  return err;
}


static int __devexit als_remove(struct i2c_client *client)
{
  int result = 0;
  
  LOG_ALS("%s. \n", __func__);
//  data = i2c_get_clientdata(client);
  
  cancel_delayed_work_sync(&als_data->work);
  destroy_workqueue(als_data->queue);
  
  input_unregister_device(als_data->input_dev);
  input_free_device(als_data->input_dev);
  
#ifdef ALS_SUSPEND_SUPPORTED
  unregister_early_suspend(&als_data->early_suspend);
#endif
	
//  msleep(100);
  
  
  if(client==this_client) {  
    i2c_set_clientdata(client, NULL);
  }
  else {
    LOG_ALS("remove als with wrong i2c_client\n");
  }
  
	kfree(als_data);
  
//  aw_ops.free_platform_resource();
  
  return 0;
}
static int als_detect(struct i2c_client *client,
                      struct  i2c_board_info *info)
{
  struct  i2c_adapter *adapter = client->adapter;
  
  LOG_ALS("%s. \n", __func__);
  
  if (adapter->nr == twi_id) {
    const char *type_name = ALS_NAME ;    
    strlcpy(info->type, type_name, I2C_NAME_SIZE);    
    return  0;
  } else {
    return -ENODEV;    
  }
  
}


static const struct i2c_device_id als_id[] = {
	{ ALS_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, als_id);

static const unsigned short normal_i2c[] =
             {ALS_SLAVE_ADDR, I2C_CLIENT_END};

static struct i2c_driver als_i2c_driver = {
  
	.probe = als_probe,
	.remove =  __devexit_p(als_remove),
  .detect = als_detect,
  
	.id_table = als_id,
	
  .class = I2C_CLASS_HWMON,
	.driver = {
	  .name = ALS_NAME,
		.owner = THIS_MODULE,
	},
	
  .address_list = normal_i2c,
};

static int als_fetch_sysconfig_para(void)
{
	int ret = -1;
	int als_used = -1;
	char name[I2C_NAME_SIZE];
	
	script_parser_value_type_t type = SCIRPT_PARSER_VALUE_TYPE_STRING;
	
	if(SCRIPT_PARSER_OK != script_parser_fetch("ls_para", "ls_used", &als_used, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(1 != als_used){
		pr_err("%s: als_unused. \n",  __func__);
		//ret = 1;
		return -1;
	}

	if(SCRIPT_PARSER_OK != script_parser_fetch_ex("ls_para", "ls_name", (int *)(&name), &type, I2C_NAME_SIZE*sizeof(char))){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(strcmp(ALS_NAME, name)){
		pr_err("%s: name %s does not match ALS_NAME. \n", __func__, name);
		pr_err(ALS_NAME);
		//ret = 1;
		return -1;
	}

	if(SCRIPT_PARSER_OK == script_parser_fetch("ls_para", "ls_log", &log_en, sizeof(log_en))){
		log_en=0x01;
		printk("ALS LOG ===> ENABLE\n");
	}
	
//	if(SCRIPT_PARSER_OK != script_parser_fetch("ls_para", "ls_twi_addr", &twi_addr, sizeof(twi_addr))){
//		pr_err("%s: script_parser_fetch twi_addr err. \n", name);
//		goto script_parser_fetch_err;
//	}
	
	if(SCRIPT_PARSER_OK != script_parser_fetch("ls_para", "ls_twi_id", &twi_id, sizeof(twi_id))){
		pr_err("%s: script_parser_fetch twi_id err. \n", name);
		goto script_parser_fetch_err;
	}
	
	LOG_ALS("%s: ls_twi_addr=0x%x, ls_twi_id is %d. \n", __func__, twi_addr, twi_id);
	
	return 0;

script_parser_fetch_err:
	pr_notice("=========script_parser_fetch_err============\n");
	return ret;
}


static int __init als_init(void)
{
  /* register driver */
  int ret = 0;
  
  printk("%s. \n", __func__);
  
  ret = als_fetch_sysconfig_para();
  if(ret < 0){
	  printk("als init fail!\n");
    return -1;
  }
  
	ret = i2c_add_driver(&als_i2c_driver);
	if(ret) {
	  return ret;
	}
	LOG_ALS("i2c_add_driver ret= %d \n", ret);
	
  /*to be added here*/
  ret = als_hw_enable();
  
  return ret;
}

static void __exit als_exit(void)
{
  printk("%s. \n", __func__);
  als_hw_disable();//reset als sensor
  
  i2c_del_driver(&als_i2c_driver);
}

//late_initcall(als_init);
module_init(als_init);
module_exit(als_exit);

MODULE_AUTHOR("zly <zenglingying@allwinnertech.com>");
MODULE_DESCRIPTION("AW Light Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");


