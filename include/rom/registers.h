/*! \file   include/rom/registers.h
    \brief  ROM Interface: RCX registers cached by ROM functions
    \author Markus L. Noga <markus@noga.de>
 */

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License
 *  at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *  the License for the specific language governing rights and
 *  limitations under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

#ifndef __rom_registers_h__
#define __rom_registers_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

extern unsigned char rom_port1_ddr;	//!< ROM shadow of port 1 DDR
extern unsigned char rom_port2_ddr;	//!< ROM shadow of port 2 DDR
extern unsigned char rom_port3_ddr;	//!< ROM shadow of port 3 DDR
extern unsigned char rom_port4_ddr;	//!< ROM shadow of port 4 DDR
extern unsigned char rom_port5_ddr;	//!< ROM shadow of port 5 DDR

/*! Port 6 is connected to both LCD and active sensor output.
   As lcd_refresh() is a ROM call, we need to update this location
   if active sensors are to remain active after a LCD refresh.
 */
extern unsigned char rom_port6_ddr;	//!< ROM shadow of port 6 DDR
extern unsigned char rom_port7_pin;	//!< ROM shadow of port 7 pin

#ifdef  __cplusplus
}
#endif

#endif // __rom_registers_h__
