/*! \file   include/dirpd.h
    \brief  Interface: direct control of Dual IR Proximity Detector
    \author Mark Falco (falcom@onebox.com)

    This code applies to the Dual IR Proximity Detector  that can be ordered
    from http://www.techno-stuff.com.  You may need to modify the ranges
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
 *  Contributor(s):    Mark Falco (falcom@onebox.com)
 */

#ifndef DIRPD_H
#define DIRPD_H

//0xdec0 - sampled value for object to left
#define DIRPD_LEFT_S 0xd000 
#define DIRPD_LEFT_E 0xe000

//0xb700 - sampled value for object to right
#define DIRPD_RIGHT_S 0xb000
#define DIRPD_RIGHT_E 0xbfff

//0xffc0 - sampled value for object to center
#define DIRPD_CENTER_S 0xf000
#define DIRPD_CENTER_E 0xffff

//0x8d40 - sampled value for no object
#define DIRPD_NONE_S 0x8000
#define DIRPD_NONE_E 0x8fff


//pass in a value obtained from an active sensor

#define DIRPD_LEFT(araw) (araw >= DIRPD_LEFT_S && araw <= DIRPD_LEFT_E)
#define DIRPD_RIGHT(araw) (araw >= DIRPD_RIGHT_S && araw <= DIRPD_RIGHT_E)
#define DIRPD_CENTER(araw) (araw >= DIRPD_CENTER_S && araw <= DIRPD_CENTER_E)
#define DIRPD_NONE(araw) (araw >= DIRPD_NONE_S && araw <= DIRPD_NONE_E)


#endif //DIRPD_H
