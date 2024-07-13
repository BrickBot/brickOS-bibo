#!/bin/sh

### ==========================================================================
###  $Id: makelx.sh  |  Matthew Sheets
###  FILE: util/user/makelx.sh - script for building user programs
###  MAKE: will both build and install as "makelx" with 755 permissions
###  bibo - another LEGO Mindstorms OS
### --------------------------------------------------------------------------

# Constants
MAKE_PROGRAM=make

# Variable Initialization
SCRIPT_NAME=$(basename $0)
MAIN_PROGRAM_SOURCE=$1
PROGRAM_NAME=$(basename ${1%.*})
shift
ADDITIONAL_SOURCES=$*

# Usage information
echo "Makes an LX program file from source"
echo "Usage: $SCRIPT_NAME <main source file> [additional source files]..."

echo ""
echo "Main program source file: '$MAIN_PROGRAM_SOURCE'"

echo "Additional source files for $PROGRAM_NAME:"
echo " - $ADDITIONAL_SOURCES"

MAKE_COMMAND="${MAKE_PROGRAM} SOURCES='${MAIN_PROGRAM_SOURCE}' ${PROGRAM_NAME}_SRC='${ADDITIONAL_SOURCES}'"

echo ""
echo "Command to execute: ${MAKE_COMMAND}"
echo "(Placeholder only - functionality not yet implemented)"

echo ""
echo "NOTE: make cannot handle spaces in file names"
echo " - https://savannah.gnu.org/bugs/?712"
