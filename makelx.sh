#!/bin/sh

### ==========================================================================
###  $Id: makelx.sh  |  Matthew Sheets
###  FILE: makelx.sh - script for building user programs
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
SCRIPT_EXTENSION=${SCRIPT_NAME##*.}
SCRIPT_DIR=$(realpath -s $(dirname $0))

SOURCE_TREE_MAKEFILE_PATH=$(realpath -s $SCRIPT_DIR/Makefile.lxprog)
INSTALLED_MAKEFILE_PATH=$(realpath -s $SCRIPT_DIR/../$BRICK_ARCH/$SYSCONF_PACKAGESUBDIR/Makefile)

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
if [ -f "$SOURCE_TREE_MAKEFILE_PATH" ] ; then
  # We are in the source tree
  MAKEFILE_PATH="$SOURCE_TREE_MAKEFILE_PATH"
elif [ -f "$INSTALLED_MAKEFILE_PATH" ] ; then
  MAKEFILE_PATH="$INSTALLED_MAKEFILE_PATH"
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
        MAKE_VARS="$MAKE_VARS KERNEL='$KERNEL'"
        ;;
      --kernel=*)
        KERNEL="${1#*=}"
        MAKE_VARS="$MAKE_VARS KERNEL='$KERNEL'"
        ;;
      -t)
        shift
        MAKE_VARS="$MAKE_VARS TTY_NAME='$1'"
        ;;
      --tty=*)
        MAKE_VARS="$MAKE_VARS TTY_NAME='${1#*=}'"
        ;;
      -b)
        shift
        MAKE_VARS="$MAKE_VARS TTY_BAUD='$1'"
        ;;
      --baud=*)
        MAKE_VARS="$MAKE_VARS TTY_BAUD='${1#*=}'"
        ;;
      -o)
        shift
        MAKE_VARS="$MAKE_VARS TTY_TIMEOUT='$1'"
        ;;
      --timeout=*)
        MAKE_VARS="$MAKE_VARS TTY_TIMEOUT='${1#*=}'"
        ;;
      -n)
        shift
        MAKE_VARS="$MAKE_VARS LNP_HOST_ADDR='$1'"
        ;;
      --node=*)
        MAKE_VARS="$MAKE_VARS LNP_HOST_ADDR='${1#*=}'"
        ;;
      -k)
        shift
        MAKE_VARS="$MAKE_VARS LNP_BRICK_ADDR='$1'"
        MAKE_TARGETS="$MAKE_TARGETS set-brick-address"
        ;;
      --brickaddr=*)
        MAKE_VARS="$MAKE_VARS LNP_BRICK_ADDR='${1#*=}'"
        MAKE_TARGETS="$MAKE_TARGETS set-brick-address"
        ;;
      -a)
        shift
        MAKE_VARS="$MAKE_VARS LNP_DEST_ADDR='$1'"
        ;;
      --destaddr=*)
        MAKE_VARS="$MAKE_VARS LNP_DEST_ADDR='${1#*=}'"
        ;;
      -r)
        shift
        MAKE_VARS="$MAKE_VARS LNP_DEST_PORT='$1'"
        ;;
      --destport=*)
        MAKE_VARS="$MAKE_VARS LNP_DEST_PORT='${1#*=}'"
        ;;
      -p)
        shift
        MAKE_VARS="$MAKE_VARS PROG_NUM='$1'"
        ;;
      --program=*)
        MAKE_VARS="$MAKE_VARS PROG_NUM='${1#*=}'"
        ;;
      -f | --firmdl)
        MAKE_TARGETS="$MAKE_TARGETS firmdl"
        ;;
      -l | --download)
        MAKE_TARGETS="$MAKE_TARGETS download"
        ;;
      -x | --execute)
        MAKE_TARGETS="$MAKE_TARGETS execute"
        ;;
      -d | --delete)
        MAKE_TARGETS="$MAKE_TARGETS delete"
        ;;
      -e | --emulate)
        MAKE_TARGETS="$MAKE_TARGETS emulate"
        ;;
      -g | --debug)
        MAKE_TARGETS="$MAKE_TARGETS debug"
        ;;
      -slm | -srm | -sim)
        MAKE_TARGETS="$MAKE_TARGETS message"
        MAKE_VARS="$MAKE_VARS 'LNP_MSG_ARGS=\"$1 \'$2\'\"'"
		shift
        ;;
      -sam)
        MAKE_TARGETS="$MAKE_TARGETS message"
        MAKE_VARS="$MAKE_VARS 'LNP_MSG_ARGS=\"$1 $2 $3 $4 \'$5\'\"'"
		shift ; shift ; shift ; shift
        ;;
      -r | --remote)
        MAKE_TARGETS="$MAKE_TARGETS remote"
        ;;
      -h | --help)
        output_usage
        exit 0
        ;;
      *=*)
        MAKE_VARS="$MAKE_VARS '$1'"
        ;;
    esac
    shift
  done
  
  
  
  if [ -z "$*" ] ; then
    # No file(s) specified
	true ;
  else
    MAKE_FILES="FILES='$*'"
  fi

  # Generate and execute the make command
  MAKE_COMMAND="${MAKE_PROGRAM} ${MAKE_VARS} ${MAKE_FILES} ${MAKE_TARGETS} --makefile='${MAKEFILE_PATH}'"
  echo "Command: \"${MAKE_COMMAND}\""
  eval "${MAKE_COMMAND}"
fi
