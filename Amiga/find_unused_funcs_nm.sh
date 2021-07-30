#!/bin/bash

EXE=WCS.unstripped
COMPILER_PATH=~/opt/m68k-amigaos_15Jun21/bin/

UNUSED_OR_STATIC_COUNT=0

"$COMPILER_PATH"/m68k-amigaos-objdump -tC "$EXE" | awk '/\.text.*\.part/{next} /\.text/{print $6}' > all_defined_functions.txt

# get the used object files from the linker map file. If they were build from c-Files, then rename them to *.c -> these c-Files where used to build the executable
cat wcs.map | awk '/^LOAD \./{gsub(/\.o/,".c",$2); if (system("test -f " "../"$2)==0) { print $2 }}' >../list_of_used_c_files.txt

# list alle externally referenced functions from the o-files
find . -name "*.o" -exec ~/opt/m68k-amigaos_15Jun21/bin/m68k-amigaos-nm  --undefined-only >>called_functions.txt {} \;


# we look into "tags" to care only for our own functions. Othrewise clib functions would also be considered
# only the c-files that were used to build the executable will tagged
cd ..
ctags -L list_of_used_c_files.txt
cd - >/dev/null

for FUNCTION in $(cat all_defined_functions.txt); do
        if [ $FUNCTION != "main" ]; then        # ignore main(), this should be never called by other source code
                if [ $(grep "^$FUNCTION[^a-zA-Z0-9_\=\[]" ../tags | egrep "[space]*f$" | grep -v "static" | grep -c ".") -ne "0" ]; then     # only functions that are definded in our c-Files are considered
                        if [ $(grep "$FUNCTION" called_functions.txt  | grep -c ".") -eq "0" ]; then
                                echo "--- $FUNCTION() ------------------------------------------"
                                for FILE in $(cat ../list_of_used_c_files.txt); do
                                        grep -nH "$FUNCTION"[^a-zA-Z0-9_\=\[] "../$FILE" # show found functions
                                done
                                ((UNUSED_OR_STATIC_COUNT++))
                        fi
                fi
        fi
done

echo "$UNUSED_OR_STATIC_COUNT functions are either completely unused or could be made static."

