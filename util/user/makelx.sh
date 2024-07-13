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
echo "Generate an executable LX program file from source code"
echo "Usage: $SCRIPT_NAME <main source file> [additional source files]..."
echo ""
echo "Usage Notes:"
echo " * File names cannot contain spaces, due to \"make\" command limitations."
echo "   - https://savannah.gnu.org/bugs/?712"

echo ""
echo "Main program source file: '$MAIN_PROGRAM_SOURCE'"

echo "Additional source files for $PROGRAM_NAME:"
echo " - $ADDITIONAL_SOURCES"

MAKE_COMMAND="${MAKE_PROGRAM} SOURCES='${MAIN_PROGRAM_SOURCE}' ${PROGRAM_NAME}_SRC='${ADDITIONAL_SOURCES}'"

echo ""
echo "Command: \"${MAKE_COMMAND}\""
echo "(Placeholder only - functionality not yet implemented)"
