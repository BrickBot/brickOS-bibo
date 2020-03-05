///////////////////////////////////////////////////////////////////////////////
//
// File: lnp_comm.h
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


#ifndef LNP_COMM_H_INCLUDED
#define LNP_COMM_H_INCLUDED

//#define FRENCH 1
#ifdef FRENCH
  #define STR_RAW              "\nMessage de données non traité:\n"
  #define STR_INTEGRITY        "\n(Intégrité)  %s\n"
  #define STR_VERBOSE          "\nHôte de dest: %d, Port de dest: %d, Hôte de srce: %d, Port de srce: %d\n%s\n"
  #define STR_UNKNOWN          "\n(Type inconnu) %s\n"
  #define STR_BAD_CHECKSUM     "\n  *** Mais Checksum!  Message(s) perdu(s)! ***\n"
  #define STR_SIZE_MISMATCH    "\n  *** Message Size Incorrect!  Message(s) Lost! ***\n"
  #define STR_OPEN_ERR         "Ne pouvait pas ouvrir %s. Le programme termine."
  #define STR_WRITE_ERR        "Ne pouvait pas écrire à %s. Le programme termine."
  #define STR_CLOSE_ERR        "Ne pouvait pas fermer %s. Le programme termine."
  #define STR_ACCESS_ERR       "Erreur dans l'acces %s.  Le programme termine."
  #define STR_HOST_ADD_RANGE   "L'adresse de l'hôte est hors limite %s."
  #define STR_NO_PARAMETER     "Paramètre manquant pour l'option %s."
  #define STR_HOST_PORT_RANGE  "Le port de l'hôte est hors limite %s."
  #define STR_UNRECOGNIZED_OPT "Option inconnue: %s."
  #define STR_FILE_NOT_DELETED "\nInfo de déboggage: Fichier non effacé.\n"
  #define STR_DEFAULT_TTY      "Utilise le tty par défaut connexion peut faillir! Utilisez -h pour les options.\n"
#else
  #define STR_RAW              "\nRaw message data:\n"
  #define STR_INTEGRITY        "\n(Integrity)  %s\n"
  #define STR_VERBOSE          "\nDest Host: %d, Dest Port: %d, Source Host: %d, Source Port: %d\n%s\n"
  #define STR_UNKNOWN          "\n(Unknown Type) %s\n"
  #define STR_BAD_CHECKSUM     "\n  *** Bad Checksum!  Message(s) Lost! ***\n"
  #define STR_SIZE_MISMATCH    "\n  *** Message Size Incorrect!  Message(s) Lost! ***\n"
  #define STR_OPEN_ERR         "Could not open %s.  Program will exit."
  #define STR_WRITE_ERR        "Could not write to %s.  Program will exit."
  #define STR_CLOSE_ERR        "Could not close %s.  Program will exit."
  #define STR_ACCESS_ERR       "Error in accessing %s.  Program will exit."
  #define STR_HOST_ADD_RANGE   "Host address is out of range %s."
  #define STR_NO_PARAMETER     "Parameter missing for %s option."
  #define STR_HOST_PORT_RANGE  "Host port is out of range %s."
  #define STR_UNRECOGNIZED_OPT "Unrecognized option: %s"
  #define STR_FILE_NOT_DELETED "\nDebug Info: File not deleted.\n"
  #define STR_DEFAULT_TTY      "Using default tty...may fail to connect!  Use -h for options.\n"
#endif

#define STR_ADDRESSING         "\n(%d,%d|%d,%d)  %s\n"


//Prototypes
extern void          LogString(char *fileName, char *aPacket);
extern void          lnpmsg_message_printf(unsigned char msg);
extern void          lnpmsg_remote_printf(unsigned int msg);
extern void          lnpmsg_integrity_printf(const unsigned char * data, unsigned char length);
extern void          lnpmsg_addressing_sniffer_printf(const unsigned char * data,
                        unsigned char length, unsigned char src, unsigned char dest);
extern int           ReceiveMessage(tty_t *tty, int timeout, int myAdd, int myPort,
                             int verbose, char *aFile, char *match);
extern void          SendFile(tty_t *tty, int timeout, char *sendFile, char *rtrMsg, int verbose);

#endif /* LNP_COMM_H_INCLUDED */
