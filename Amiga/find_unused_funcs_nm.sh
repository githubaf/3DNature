#!/bin/bash

COMPILER_PATH=~/opt/m68k-amigaos_15Jun21/bin/
EXE=WCS.unstripped

"$COMPILER_PATH"/m68k-amigaos-objdump -D "$EXE" >"$EXE".objdump

UNUSED_OR_STATIC_COUNT=0

# list alle external referenced functions from the o-files
find . -name "*.o" -exec ~/opt/m68k-amigaos_15Jun21/bin/m68k-amigaos-nm  --undefined-only >>called_functions.txt {} \;

# get the used objects files from the linker map file. Rename them to *.c -> these c-Files where used to build the executable
cat wcs.map | awk '/^LOAD \./{gsub(/\.o/,".c",$2); print $2}' >../list_of_c_files.txt

# we look into "tags" to care only for our own functions. Othrewise clib functions would also be considered
# only the c-files that were used to build the executable will tagged
cd ..
ctags -L list_of_c_files.txt
cd - >/dev/null

for FUNCTION in $("$COMPILER_PATH"/m68k-amigaos-objdump -tC "$EXE" | awk '/\.text.*\.part/{next} /\.text/{print $6}'); do
	if [ $(grep "$FUNCTION[^a-zA-Z0-9_\=\[]" ../tags | egrep "[space]*f$" | grep -v "static" | grep -c ".") -ne "0" ]; then     # only functions that are definded in our c-Files are considered
		if [ $(grep "$FUNCTION" called_functions.txt  | grep -c ".") -eq "0" ]; then
			#echo "$FUNCTION not externally referenced, i.e unused or static candidate"
			#grep "$FUNCTION[^a-zA-Z0-9_\=\[]" ../tags | egrep "[space]*f$"
			find .. -name "*.c" -exec grep -nH "$FUNCTION" {} \;  # show found functions
			echo "---------------------------------------------"
			((UNUSED_OR_STATIC_COUNT++))
		fi
	fi
done

echo "$UNUSED_OR_STATIC_COUNT functions are either completely unused or could be made static."
