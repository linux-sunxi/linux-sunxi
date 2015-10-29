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
 *    Driver Configurations
 */

#ifndef _DRIVER_CONF_H
#define _DRIVER_CONF_H


#define DRIVER_LICENSE_INFO    "GPL"
#define DRIVER_VERSION         "RoboPeak USB Display Driver Version 1.00"

#define RPUSBDISP_MINOR		   300

#define RPUSBDISP_STATUS_QUERY_RETRY_COUNT  4


#define RPUSBDISP_MAX_TRANSFER_SIZE          (PAGE_SIZE*16 - 512)
#define RPUSBDISP_MAX_TRANSFER_TICKETS_COUNT 10

#endif
