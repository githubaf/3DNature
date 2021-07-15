#!/bin/bash
# AF, HGW, 15.July 2021
# searches for unused functions in the executable compiled with bebbo's gcc for Amiga
# cd into Release or Debug befor calling the script!
# The script:
#      1. disassembles the executable once to a file
#      2. for each function found does a grep in the disassembly an checks, if it occurs only 1, i.e. never called.
#
# Remove the funcions by commenting them out, then call the script again until no unused functions are left. 



COMPILER_PATH=~/opt/m68k-amigaos_15Jun21/bin/
EXE=WCS

"$COMPILER_PATH"/m68k-amigaos-objdump -D "$EXE" >"$EXE".objdump

COUNT=0

for FUNCTION in $("$COMPILER_PATH"/m68k-amigaos-objdump -tC "$EXE" | awk '/\.text/{print $6}'); do 
	if [ $(grep "$FUNCTION" "$EXE".objdump  | wc -l) -eq 1 ]; then 
		((COUNT++))
	      	echo "$FUNCTION" 
	fi
done

echo
echo "$COUNT" unused functions found.
