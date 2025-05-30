///////////////////////////////////////////////////////////////////////////////
//
// File: lnpmsg.c  [Original File Name: legoshrink.c]
//
// Author: Mike LeSauvage (based on work done by those in the description.)
//
// Description: This program is used to collect information from a Lego
//              robot, print it to the screen, and write it to a log (if
//              desired).
//              This program is derived from ir3, a program written by
//              Pavel Petrovic(ppetrovic@acm.org).  He in turn based it on
//              firmdl3.c and ir.c, the firmware dowloader for the RCX.  His
//              code was tested on brickOS 0.2.6.10.6 and Debian Linux.
//              (http://www.idi.ntnu.no/~petrovic/ir/)
//
//              I modified his program for my purposes, which at the moment,
//              only involve reading messages from the brick.  I documented
//              work that was already implemented, researched and documented
//              the Lego Network Protocol (see lnp_integrity_byte), removed
//              the functions related to sending data, changed the program
//              options, and added address packet support (which was the main
//              reason I started poking around in the first place).
//
//              This program was tested on brickOS 0.2.6.10 on Windows 2000.
//              Since I do not have a serial tower, I don't know if the
//              program will function with one.  I tried to preserve serial
//              related code.  Your mileage may vary.
//
//              As this is code originally derived from code based firmdl3, the
//              Mozilla Public License still applies.
//
//              Reports of success/failure for linux and/or the serial tower
//              would be appreciated.
//
//              Good luck and enjoy!
//
//
//              Mike LeSauvage
//              michael <dot> lesauvage <at> rmc <dot> ca   //This hides me from spam!
//              14 Feb 2004.
//
// Modifications: 10 Feb 2005 - Major revision of original code complete.
//                15 Feb 2005 - Program may now be compiled in either
//                              English or French.
//                12 Mar 2005 - Changed program so it can send a file, line
//                              by line.
//                18 Apr 2009 - Matthew Sheets
//                              Incorporated into BrickOS, reworked to
//                              use existing rcx_comm code, and
//                              added TCP and NCD tty support;
//                              temporarily disabled send/log file support
//                              due to code reuse changes
//
///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <termios.h>
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <lnp-logical.h>
#include <lnp.h>

#include "config.h"
#include "rcx_comm.h"
#include "lnpmsg_comm.h"

// Bitmask the program actions
typedef enum {  do_undefined = 0,
                do_receive = 1,
                do_lego_msg = 2,
                do_remote_msg = 4,
                do_integrity_msg = 8,
                do_addressing_msg = 16,
             } prog_action_t;

///////////////////////////////////////////////////////////////////////////////
//
// Function: main
//
// Description: The main function for the legoshrink program.  The purpose
//              of main is to parse the command line arguments and set up the
//              appropriate environment for receiving packets from the lego
//              robot.
//
// Parameters: argc - The number of command line arguments.
//             argv - An array of pointers to strings of the command line args.
//
// Returns: 0 for normal exit,
//          1 on abnormal termination.
//
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  tty_t tty = { BADFILE, tty_t_undefined };
  prog_action_t progAction = do_receive;
  int addressPC = -1;
  int portPC=-1;
  int verbose=0;
  int baud=RCX_DEFAULT_BAUD;
  int timeout=RCX_DEFAULT_TIMEOUT;
  int overWrite=0;
  char errorMsg[65]="";
  char *logFileName=NULL;
  //char *sendFileName=NULL;
  //char *rtrWord=NULL;
  char *tty_name=NULL;
  char *srcPort=NULL;
  char *destAddr=NULL;
  char *destPort=NULL;
  char *sendMessage=NULL;
  int legoMessage = -1;
  long remoteMessage = -1;

  char *usageString =
      "Usage: lnpmsg [options] [action]\n"
      "\n"
      "Actions:\n"
      "  -slm <message num>  Send Lego standard firmware message (0 - 255).\n"
      "  -srm <message num>  Send Lego remote message (0 - 65,535).\n"
      "  -sim \"<message>\"    Send LNP Integrity message.\n"
      "  -sam <source port> <destination address> <destination port> \"<message>\"\n"
      "                      Send LNP Addressing message; valid addresses & ports: 0-15.\n"
      "   If no send action is specified, the program will run in \"listen\" mode."
      "\n"
      RCX_COMM_OPTIONS
      "  --node=<address>    Set PC host address, valid from 0-15.  Default -1 (any).\n"
//      "  -p <port>        Set PC host port to listen on, valid from 0-15.\n"
//      "                   Use -1 to listen for messages directed to any port(default).\n"
//      "  -l <file>        Log all messages to the specified file.\n"
//      "  -ld <file>       Log messages; first delete named file if it already exists.\n"
//      "  -sf <file> <rtr> Send file.  The file is sent one line at a time (characters\n"
//      "                   in excess of 255 will be truncated).  Before sending each\n"
//      "                   line, lnpmsg waits to receive the rtr word (ready to\n"
//      "                   receive) from the bot.  Only integrity mode is supported.\n"
//      "  --debug          Show raw bytes for all incoming messages.\n"
      "\n"
      RCX_COMM_OPTIONS_INFO
      "\n"
      "Examples: \n" // lnpmsg -t usb -p 6 -l crashlog.txt\n"
//      "          lnpmsg -t usb -sf commands.txt SENDNOW\n"
      "          lnpmsg --tty=tcp:localhost:" DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING " -sim \"Integrity Message\"\n"
      "          lnpmsg -tusb -sam 8 0 15 \"Integrity Message\"\n"
      ;
   
      
  //Parse the command line.
  argc--;
  argv++;

  while(argc > 0 && strlen(errorMsg)==0)
  {
//    if(!strcmp(argv[0], "--debug"))            //Extract debug option.
//      showDebugInfo = 1;
//    else
    if(!strncmp(argv[0], "--node=", 7))            //Extract host address option
    {
        addressPC=atoi(argv[0]+7);
        if(addressPC<0 || addressPC>15)
          sprintf(errorMsg, STR_HOST_ADD_RANGE, "0-15");
    }
    else if(!strcmp(argv[0], "-n"))            //Extract host address option
    {
      if(argc > 1)
      {
        argc--;
        argv++;
        addressPC=atoi(argv[0]);
        if(addressPC<0 || addressPC>15)
          sprintf(errorMsg, STR_HOST_ADD_RANGE, "0-15");
      }
      else
        sprintf(errorMsg, STR_NO_PARAMETER, "-n");
    }
//    else if(!strcmp(argv[0], "-p"))            //Extract host port setting.
//    {
//      if(argc > 1)
//      {
//        argc--;
//        argv++;
//        portPC=atoi(argv[0]);
//        if(portPC<-1 || portPC>15)
//          sprintf(errorMsg, STR_HOST_PORT_RANGE, "(-1)-15");
//      }
//      else
//        sprintf(errorMsg, STR_NO_PARAMETER, "-p");
//    }
//    else if(!strcmp(argv[0], "-l"))           //Extract file option.
//    {
//      if(argc > 1)
//      {
//        argc--;
//        argv++;
//        logFileName=argv[0];
//      }
//      else
//        sprintf(errorMsg, STR_NO_PARAMETER, "-l");
//    }
//    else if(!strcmp(argv[0], "-ld"))          //Extract file option.
//    {                                         //along with the
//      if(argc > 1)                            //overwrite option.
//      {
//        argc--;
//        argv++;
//        logFileName=argv[0];
//        overWrite=1;
//      }
//      else
//        sprintf(errorMsg, STR_NO_PARAMETER, "-ld");
//    }
//    else if(!strcmp(argv[0], "-sf"))        //Extract send file option
//    {                                       //along with its two parameters:
//      if(argc > 2)                          //the file to send and the
//      {                                     //"ready to receive" word.
//        argc--;
//        argv++;
//        sendFileName=argv[0];
//        argc--;
//        argv++;
//        rtrWord=argv[0];
//      }
//      else
//        sprintf(errorMsg, STR_NO_PARAMETER, "-sf");
//    }
    else
    if (!strcmp(argv[0], "-slm"))
    {
      if (argc > 1)
      {
        argc--;
        argv++;
        legoMessage=atoi(argv[0]);
        progAction=do_lego_msg;
        if (legoMessage < 0 || legoMessage > 0xFF)
          sprintf(errorMsg, "Invalid Lego Remote Message: number must be between 0 and 255");
      }
      else
        sprintf(errorMsg, STR_NO_PARAMETER, argv[0]);
    }
    else if (!strcmp(argv[0], "-srm"))
    {
      if (argc > 1)
      {
        argc--;
        argv++;
        remoteMessage=atol(argv[0]);
        progAction=do_remote_msg;
        if (remoteMessage < 0 || remoteMessage > 0xFFFF)
          sprintf(errorMsg, "Invalid Lego Remote Message: number must be between 0 and 65,535");
      }
      else
        sprintf(errorMsg, STR_NO_PARAMETER, argv[0]);
    }
    else if (!strcmp(argv[0], "-sim"))
    {
      if (argc > 1)
      {
        argc--;
        argv++;
        sendMessage=argv[0];
        progAction=do_integrity_msg;
      }
      else
        sprintf(errorMsg, STR_NO_PARAMETER, argv[0]);
    }
    else if (!strcmp(argv[0], "-sam"))
    {
      if (argc > 4)
      {
        argc--;
        argv++;
        srcPort=argv[0];
        argc--;
        argv++;
        destAddr=argv[0];
        argc--;
        argv++;
        destPort=argv[0];
        argc--;
        argv++;
        sendMessage=argv[0];
        progAction=do_addressing_msg;
      }
      else
        sprintf(errorMsg, STR_NO_PARAMETER, argv[0]);
    }
    else if(strncmp(argv[0], "-t", 2) == 0)           //Extract short tty option.
    {
      tty_name = &argv[0][2];
    }
    else if(strncmp(argv[0], "--tty=", 6) == 0)           //Extract long tty option.
    {
      tty_name = &argv[0][6];
    }
    else if(strncmp(argv[0], "-b", 2) == 0)
    {
      baud = atoi(&argv[0][2]);
    }
    else if (strncmp(argv[0], "--baud=", 7) == 0)
    {
      baud = atoi(&argv[0][7]);
    }
    else if (strncmp(argv[0], "-o", 2) == 0)
    {
      timeout = atoi(&argv[0][2]);
    }
    else if (strncmp(argv[0], "--timeout=", 10) == 0)
    {
      timeout = atoi(&argv[0][10]);
    }
    else if(!strcmp(argv[0], "-v") || !strcmp(argv[0], "--verbose")) //Extract verbose option.
    {
      verbose=1;
      rcx_set_debug(1);   // from rcx_comm
    }
    else if(!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help")) //Extract help option.
      sprintf(errorMsg, "Help Information");
    else
      sprintf(errorMsg, STR_UNRECOGNIZED_OPT, argv[0]);

    argc--;
    argv++;
  }

  if(strlen(errorMsg)>0)                    //Produce error messages
  {                                         //(if any), display usage
    fprintf(stderr, "\n%s\n", errorMsg);    //information, and quit.
    fprintf(stderr, "\n%s", usageString);
    exit(1);
  }

  if(logFileName != NULL && overWrite==1)   //If the user specified overWrite
  {                                         //then delete the log file if
    int err;                                //one exists.
    err=remove(logFileName);
    if(err==-1 && verbose)
      fprintf(stderr, STR_FILE_NOT_DELETED);
  }

  rcx_init(&tty, tty_name, baud, timeout, TRUE);
  
  if(BADFILE == tty.fd)
  {
	fprintf(stderr, "Exiting program\n");
    fflush(stderr);
    exit(1);
  }

  lnp_message_set_handler(lnpmsg_message_printf);
  lnp_remote_set_handler(lnpmsg_remote_printf);
  lnp_integrity_set_handler(lnpmsg_integrity_printf);
  lnp_addressing_sniffer_set_handler(lnpmsg_addressing_sniffer_printf);

  switch(progAction)
  {
    case do_receive:
      while(1)
      {
        ReceiveMessage(&tty, timeout, addressPC, portPC, verbose, logFileName, NULL);
        usleep(20000);
      }
      break;
    case do_lego_msg:
      send_msg((unsigned char) legoMessage);
      break;
    case do_remote_msg:
      send_code((unsigned int) remoteMessage);
      break;
    case do_integrity_msg:
      lnp_integrity_puts(sendMessage);
      break;
    case do_addressing_msg:
      lnp_addressing_puts(sendMessage, (atoi(destAddr) << 4) | (atoi(destPort) & 0x0F),
                            atoi(srcPort));
      break;
    default:
      fprintf(stderr, "\n%s", usageString);  // Display help info
  }

  rcx_close(&tty);
  exit(0);
}
