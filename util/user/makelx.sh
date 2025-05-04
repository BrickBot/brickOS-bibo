#!/bin/sh

### ==========================================================================
###  $Id: makelx.sh  |  Matthew Sheets
###  FILE: util/user/makelx.sh - script for building user programs
###  MAKE: will both build and install as "makelx" with 755 permissions
###  bibo - another LEGO MindStorms RCX OS
### --------------------------------------------------------------------------

# IMPORTANT: Do not modify the commented "Variables" placeholder line below
#   without also updating the "install" target in the main Makefile in the source project.
# Installation Variables

# Constants
MAKE_PROGRAM=make

# Variable Initialization
SCRIPT_NAME=$(basename $0)
SCRIPT_DIR=$(realpath -s $(dirname $0))

# Functions
output_usage()
{
  # Output usage information
  echo "Generate an executable LX program file for brickOS-bibo from source code."
  echo "            https://brickbot.github.io/  and  https://github.com/BrickBot/"
  echo "Usage:"
  echo "  $SCRIPT_NAME [args] [Make variables (NAME=VALUE format)] <main source file> [additional source files]..."
  echo ""
  echo "Arguments and Usage Notes:"
  echo "  -k kernel-name, --kernel=kernel-name"
  echo "           Name of the kernel to use; optional if only one kernel exists."
  echo "  File names cannot contain spaces, due to \"make\" command limitations."
  echo "           c.f. https://savannah.gnu.org/bugs/?712"
}

# Locate the Makefile
if [ -f $SCRIPT_DIR/../../Makefile.dist ] ; then
  # We are in the source tree
  MAKEFILE_PATH=$(realpath -s $SCRIPT_DIR/../../Makefile.dist)
elif [ -f $SCRIPT_DIR/../share/$PACKAGE/Makefile ] ; then
  MAKEFILE_PATH=$(realpath -s $SCRIPT_DIR/../share/$PACKAGE/Makefile)
else
  output_usage
  echo "ERROR: Makefile needed by $SCRIPT_NAME not found."
  exit 1
fi

# Process the command-line arguments
if [ "$#" -le 0 ] ; then
  output_usage
else
  # Check for and and process and Make variable arguments
  MAKE_VARS=""
  while echo "$1" | grep -q "=" || echo "$1" | grep -q -G "^-" ; do
    case "$1" in
      -k)
        shift
        KERNEL="$1"
        MAKE_VARS="$MAKE_VARS KERNEL=$KERNEL"
        ;;
      --kernel=*)
        KERNEL="${1#*=}"
        MAKE_VARS="$MAKE_VARS KERNEL=$KERNEL"
        ;;
      *=*)
        MAKE_VARS="$MAKE_VARS '$1'"
        ;;
    esac
    shift
  done
  
  if [ -z "$*" ] ; then
    output_usage
    echo "ERROR: No source file(s) specified."
    exit 1
  fi

  # Generate and execute the make command to build the LX program
  MAKE_COMMAND="${MAKE_PROGRAM} ${MAKE_VARS} SOURCES='$*' --makefile='${MAKEFILE_PATH}'"
  echo "Command: \"${MAKE_COMMAND}\""
  eval "${MAKE_COMMAND}"
fi
