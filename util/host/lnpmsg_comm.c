///////////////////////////////////////////////////////////////////////////////
//
// File: lnp_comm.c
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
//
// Modifications: 10 Feb 2005 - Major revision of original code complete.
//                15 Feb 2005 - Program may now be compiled in either
//                              English or French.
//                12 Mar 2005 - Changed program so it can send a file, line
//                              by line.
//                 26 April 2008 - Matthew Sheets - Split legoshrink code into
//                              a separate library
//
///////////////////////////////////////////////////////////////////////////////


#if defined(_WIN32)
  #include <windows.h>
#else
  #include <termios.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>

#include "lnp.h"
#include "rcx_comm.h"
#include "lnpmsg_comm.h"


// Callback for standard Lego firmware messages
void lnpmsg_message_printf(unsigned char msg) {
    printf("[LEGO ] %4u    (%02Xh)\n", msg, msg);
}

// Callback for Lego remote codes
void lnpmsg_remote_printf(unsigned int code) {
    printf("[RMOTE] %4u   (%04Xh)\n", code, code);
}

// Callback for LNP Integrity messages
void lnpmsg_integrity_printf(const unsigned char * data, unsigned char length) {
    rcx_hexdump("[LNP I]", data, length, TRUE);
}

// Callback for LNP Addressing messages
void lnpmsg_addressing_sniffer_printf(const unsigned char * data,
        unsigned char length, unsigned char src, unsigned char dest)
{
    int destAddr = dest >> 4;
    int destPort = dest & 0x0F;
    int srcAddr  = src  >> 4;
    int srcPort  = src  & 0x0F;
    char prefix[9];
    
    sprintf(prefix, "[%X%X>%X%X]", srcAddr, srcPort, destAddr, destPort);
    rcx_hexdump(prefix, data, length, TRUE);
}


///////////////////////////////////////////////////////////////////////////////
//
// Function: ReceiveMessage
//
// Description: Original documentation for this function stated
//              "receive next LNP message to global array packet[]"
//              This function receives messages from the USB tower and
//              prints them to the screen until the program is exited.
//              Originally, this function sat in a loop receiving messages
//              until the program was closed.  It has been changed to
//              receive a single message.
//
// Parameters: fd - A file descriptor for the USB tower.
//             myAdd - Address of this PC; ignore packets with wrong dest add.
//             myPort - Port of this PC; ignore packets with wrong dest port.
//             verbose - Provide legend for incoming address info.
//             aFile - Name of a file to log the data.  NULL if not logging.
//             match - A pointer to a string that should be compared to the
//                     received message.  Pass NULL for no comparison.
//
// Returns: int - 1 if the received packet matches the provided string
//                "match", false otherwise.
//
// NOTE: match checking functionality has been disabled
//
///////////////////////////////////////////////////////////////////////////////
int ReceiveMessage(tty_t *tty, int timeout, int myAdd, int myPort,
                   int verbose, char *aFile, char *match)
{
  // int status, len, i, j=0, messageStatus;
  unsigned char recv[RCX_BUFFERSIZE + 1];
  // char outputString[LNP_MAX_MSG_LEN + 80]="";        //256 for packet +
                                                  //terminating null.  80 for
                                                  //extra info ie: address
                                                  //static as this fn will
                                                  //be called frequently.

  // rcx_recv_lnp triggers lnp_integrity_byte
  rcx_recv_lnp(tty, recv, RCX_BUFFERSIZE, timeout);

  // if(match != NULL)
    // if(!strcmp(packet, match))
      // return 1;
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// Function: SendFile
//
// Description: This function sends a file over the USB tower, one line of text
//              at a time.  There is a limit of 255 characters per line, any
//              excess will be truncated.  Only integrity communication is
//              currently supported.
//              The robot must send a message requesting each line.
//
// Parameters: fd - A file descriptor for the USB tower.
//             sendFile - The path/name of the file to send.
//             rtrMsg - A string to be matched.  Before sending a line, this
//                      waits to receive a match to this string.  When it does,
//                      the next line in the file is sent.
//             verbose - Does nothing in this function; is used to tell the
//                       ReceiveMessage function if verbose reporting should
//                       be used.
//
// Returns: Nothing.
//
///////////////////////////////////////////////////////////////////////////////
void SendFile(tty_t *tty, int timeout, char *sendFile, char *rtrMsg, int verbose)
{
  int i;
  long fileLen;
  int fileHandle;
  int ncRead;
  int curPos=0;
  char *fileBuffer;

#if defined(_WIN32)
  fileHandle=open(sendFile, O_RDONLY|O_BINARY);  //Try to open the file.
#else
  fileHandle=open(sendFile, O_RDONLY);  //Try to open the file.
#endif

	if(fileHandle == -1)
  {
    fprintf(stderr, STR_OPEN_ERR, sendFile);
    exit(1);
  }

  fileLen=lseek(fileHandle, 0, SEEK_END);     //Determine the size of the file.
  if(fileLen == -1L)
  {
    fprintf(stderr, STR_ACCESS_ERR, sendFile);
    exit(1);
  }

  lseek(fileHandle, 0, SEEK_SET);                //Seek back to start.
  fileBuffer = (char *)malloc(fileLen + 1);      //Allocate enough space for
                                                 //the file plus an end null.

  ncRead = read(fileHandle, fileBuffer, fileLen);//Read in the file and
  if(ncRead != fileLen)                          //check to see if the entire
  {                                              //thing was read in.  If not,
    fprintf(stderr, STR_ACCESS_ERR, sendFile);   //an error has ocurred.
    exit(1);
  }
  fileBuffer[fileLen]=0;                         //Set an end null to be sure.

  close(fileHandle);

  for(i=0; i<ncRead; i++)                        //Replace all carriage
    if(fileBuffer[i]==0xD || fileBuffer[i]==0xA) //returns with null
      fileBuffer[i]=0;                           //characters.

  while(curPos < fileLen)
  {
    while(!ReceiveMessage(tty, timeout, 0, 0, verbose, NULL, rtrMsg));//Wait for the rtr.

    lnp_integrity_printf(&fileBuffer[curPos]);

    while(fileBuffer[curPos] != 0)      //Find next null character.
      curPos++;                         //There could be multiple nulls, so
    while(fileBuffer[curPos] == 0)      //increment to next real character.
      curPos++;
  }

  free(fileBuffer);
  printf("\n\nFile %s has been completely sent.\n", sendFile);
}


///////////////////////////////////////////////////////////////////////////////
//
// Function: LogString
//
// Description: This function writes a string to the specified file.  If it
//              fails to open, close, or write to the file it exits the
//              program.
//
// Parameters: fn      - The file in which to write the string.
//             aString - The string to write.
//
// Returns: Nothing.
//
///////////////////////////////////////////////////////////////////////////////
void LogString(char *fn, char *aString)
{
  int fileHandle;
  int bytesWritten;
  
#if defined(_WIN32)
  fileHandle=open(fn, O_WRONLY|O_CREAT|O_APPEND|O_TEXT, S_IREAD|S_IWRITE);
#else
  fileHandle=open(fn, O_WRONLY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
#endif
    
  if(fileHandle==-1)
  {
    fprintf(stderr, STR_OPEN_ERR, fn);
    exit(1);
  }
  
  bytesWritten = write(fileHandle, aString, strlen(aString));
  if(bytesWritten!=strlen(aString))
  {
    fprintf(stderr, STR_WRITE_ERR, fn);
    close(fileHandle);
    exit(1);
  }
    
  if(close(fileHandle)==-1)
  {
    fprintf(stderr, STR_CLOSE_ERR, fn);
    exit(1);
  }
  
  return;
}  
