#!/bin/sh

### ==========================================================================
###  $Id: makelx.sh  |  Matthew Sheets
###  FILE: util/user/makelx.sh - script for building user programs
###  MAKE: will both build and install as "makelx" with 755 permissions
###  bibo - another LEGO Mindstorms OS
### --------------------------------------------------------------------------

echo "Makes an LX program file from source"
echo "Usage: $(basename $0) <main source file> [additional source files]..."

echo ""
echo "(Placeholder only - functionality not yet implemented)"

echo ""
echo "Main source file: '$1'"
echo "Additional source files for $(basename ${1%.*}):"
shift
echo " - $*"

