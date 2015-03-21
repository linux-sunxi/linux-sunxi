/*
 *    RoboPeak USB LCD Display Linux Driver
 *    
 *    Copyright (C) 2009 - 2013 RoboPeak Team
 *    This file is licensed under the GPL. See LICENSE in the package.
 *
 *    http://www.robopeak.net
 *
 *    Author Shikai Chen
 *
 *   ---------------------------------------------------
 *   Touch Event Handlers
 */


#include "inc/common.h"
#include "inc/touchhandlers.h"
#include "inc/usbhandlers.h"

static struct input_dev * _default_input_dev;
static volatile int _live_flag;

static int _on_create_input_dev(struct input_dev ** inputdev)
{
    *inputdev = input_allocate_device();

    if (!inputdev) {
        return -ENOMEM;
    }

    (*inputdev)->evbit[0] = BIT(EV_SYN) | BIT(EV_KEY) | BIT(EV_ABS);  
    
    (*inputdev)->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);  


    input_set_abs_params((*inputdev), ABS_X, 0, RP_DISP_DEFAULT_WIDTH, 0, 0);
    input_set_abs_params((*inputdev), ABS_Y, 0, RP_DISP_DEFAULT_HEIGHT, 0, 0);
    input_set_abs_params((*inputdev), ABS_PRESSURE, 0, 1, 0, 0);  

    (*inputdev)->name = "RoboPeakUSBDisplayTS";
    (*inputdev)->id.bustype    = BUS_USB;

    return input_register_device((*inputdev));
}

static void _on_release_input_dev(struct input_dev * inputdev)
{
    
    input_unregister_device(inputdev);
}


int __init register_touch_handler(void)
{
    int ret = _on_create_input_dev(&_default_input_dev);
    if (!ret) _live_flag = 1;
    return ret;
}

void unregister_touch_handler(void)
{
    _live_flag = 0;
    _on_release_input_dev(_default_input_dev);
    _default_input_dev = NULL;
}


int touchhandler_on_new_device(struct rpusbdisp_dev * dev)
{
    // singleton design
    return 0;
}

void touchhandler_on_remove_device(struct rpusbdisp_dev * dev)
{
    // singleton design
}


void touchhandler_send_ts_event(struct rpusbdisp_dev * dev, int x, int y, int touch)
{
    if (!_default_input_dev || !_live_flag) return;
    if (touch) {
        input_report_abs(_default_input_dev,ABS_X, x);
        input_report_abs(_default_input_dev,ABS_Y, y);
        input_report_abs(_default_input_dev, ABS_PRESSURE, 1);
        input_report_key(_default_input_dev,BTN_TOUCH, 1);

        input_sync(_default_input_dev);
    } else {
        input_report_abs(_default_input_dev, ABS_PRESSURE, 0);
        input_report_key(_default_input_dev,BTN_TOUCH, 0);
        input_sync(_default_input_dev);        
    }
}