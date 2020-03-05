/*! \file   keepalive.h
    \brief  Keepalive functionality for the IR tower
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
 *  The Original Code is legOS code, released October 2, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): 
 *    Taiichi removed "static" for the function keepaliveSend.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#if defined(_WIN32)
  #include <windows.h>
#endif

#include <lnp-logical.h>
#include "rcx_comm.h"
#include "keepalive.h"

// keepalive settings
//
#define KEEPALIVE_TIMEOUT_S   3
#define KEEPALIVE_TIMEOUT_US  700000
#define KEEPALIVE_BYTE	      0xff

extern int verbose_flag;

//! the keepalive byte (same as tower power on)
const static char keepaliveByte = KEEPALIVE_BYTE;

//! renew keepalive timer because you sent something
void keepaliveRenew()
{
  static struct itimerval it =
  {
    {KEEPALIVE_TIMEOUT_S, KEEPALIVE_TIMEOUT_US},
    {KEEPALIVE_TIMEOUT_S, KEEPALIVE_TIMEOUT_US}};

  setitimer(ITIMER_REAL, &it, NULL);
}

//! send keepalive byte & renew keepalive
void keepaliveSend()
{
  if (verbose_flag)
    fputs("\nKeepAliveSend: keeping the IR tower alive...",stderr);

  if (lnp_logical_write(&keepaliveByte, 1) != 0) {
    rcx_perror("sending keepalive");
    exit(-1);
  }
  keepaliveRenew();
}

//! keepalive signal handler
void keepaliveHandler(int arg)
{
  if (verbose_flag)
    fputs("\nHandler: keeping the IR tower alive...\n",stderr);
  keepaliveSend();
}

//! initialize keepalive
void keepaliveInit(void)
{
  if (verbose_flag) fputs("KeepAlive Init...", stderr);
  signal(SIGALRM, keepaliveHandler);
  keepaliveSend();
}

//! shutdown keepalive
void keepaliveShutdown()
{
  static const struct itimerval it =
  {
    {0, 0},
    {0, 0}};

  setitimer(ITIMER_REAL, &it, 0);
  signal(SIGALRM, SIG_DFL);
}
