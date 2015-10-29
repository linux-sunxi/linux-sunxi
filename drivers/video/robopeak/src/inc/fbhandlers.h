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
 *    Definition of Frame Buffer Handlers
 */

#ifndef _RPUSBDISP_FBHANDLERS_H
#define _RPUSBDISP_FBHANDLERS_H


int register_fb_handlers(void);
void unregister_fb_handlers(void);


int fbhandler_on_new_device(struct rpusbdisp_dev * dev);
void fbhandler_on_remove_device(struct rpusbdisp_dev * dev);
void fbhandler_on_all_transfer_done(struct rpusbdisp_dev * dev);

void fbhandler_set_unsync_flag(struct rpusbdisp_dev * dev);

#endif

