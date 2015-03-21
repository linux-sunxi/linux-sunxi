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
 *    Definition of Touch Event Handlers
 */

#ifndef _RPUSBDISP_TOUCH_HANDLERS_H
#define _RPUSBDISP_TOUCH_HANDLERS_H


int register_touch_handler(void);
void unregister_touch_handler(void);


int touchhandler_on_new_device(struct rpusbdisp_dev * dev);
void touchhandler_on_remove_device(struct rpusbdisp_dev * dev);

void touchhandler_send_ts_event(struct rpusbdisp_dev * dev, int x, int y, int touch);
#endif

