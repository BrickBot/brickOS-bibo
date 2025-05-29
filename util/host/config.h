/*! \file   include/config.h
    \brief  kernel configuration file
    \author Markus L. Noga <markus@noga.de>
 */

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

#ifndef __config_h__
#define __config_h__

// compilation environment
//
#define CONF_HOST                       //!< Host environment flag

// remote control services
//
#define CONF_RCX_PROTOCOL               //!< RCX protocol handler

// USB device
//
#define DEFAULT_USB_TOWER_NAME_LINUX    "/dev/usb/legousbtower0"
#define DEFAULT_USB_TOWER_NAME_WINDOWS  "\\\\.\\legotower1"

// IR broadcast services
//
#define DEFAULT_IR_SERVER_HOST_NAME "localhost"
#define DEFAULT_IR_SERVER_BROADCAST_PORT 50637
// #define int as string:
//  - https://gcc.gnu.org/onlinedocs/gcc-4.4.7/cpp/Stringification.html
#define xstr(s) str(s)
#define str(s) #s
#define DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING  xstr(DEFAULT_IR_SERVER_BROADCAST_PORT)

// networking services
//
#define CONF_LNP                        //!< link networking protocol
#define CONF_LNP_HOSTADDR 0x8           //!< LNP host address
#define CONF_LNP_HOSTMASK 0xf0          //!< LNP host mask

// properties
//
#define CONF_DEVICE_USER_CONF_FILE "~/.rcx/device.conf"
#define CONF_DEVICE_ETC_CONF_FILE "/etc/rcx/device.conf"

#endif // __config_h__
