#!/bin/bash
# AF, HGW, 15.July 2021
# searches for unused functions in the executable compiled with bebbo's gcc for Amiga
# cd into Release or Debug before calling the script!
# We need the preprocessed c-files in that directory, i.e. without disturbing comments or out-defined parts.
# --> add -save-temps to the compiler call to keep thouse preprocessed files!
# The script:
#      1. uses objdump to find all functions in the executable
#      2. for each function found does a grep in all preprocessed c-Files (*.i) an checks, if the function occurs only once, i.e. never called.
#      3 if functions occure more than once but only in one file, they could be made "static".
#
# Remove the funcions by commentingi/defining them out, then call the script again until no unused functions are left. 



COMPILER_PATH=~/opt/m68k-amigaos_15Jun21/bin/
EXE=WCS.unstripped

"$COMPILER_PATH"/m68k-amigaos-objdump -D "$EXE" >"$EXE".objdump

UNUSED_COUNT=0
STATIC_COUNT=0

for FUNCTION in $("$COMPILER_PATH"/m68k-amigaos-objdump -tC "$EXE" | awk '/\.text/{print $6}'); do 
		#((COUNT++))
	      	#echo "$FUNCTION" 

		LINES=$(find . -name "*.i" -exec grep -nH "$FUNCTION" {} \;)

			if [ $(echo "$LINES" | grep -c '.') -eq "1" ]; then    # only one occurence in all preprocessed C-Files? -> unused function
				((UNUSED_COUNT++))
				echo -e "\e[31m$FUNCTION() unused\e[0m"
				echo "$LINES"
				echo "---------------------------------------------------------"
			elif [ $(echo "$LINES" | awk -F: '{print $1}' | sort --unique | grep -c ".") -eq "1" ]; then  # several occurences but all in one file? --> static
				((STATIC_COUNT++))
				echo "$FUNCTION is used only locally in"
				echo "$LINES -> make it static if not already done!"
				echo "---------------------------------------------------------"
			fi
done

echo
echo "$UNUSED_COUNT unused functions found."
echo "$STATIC_COUNT static-candidates found."


                                                                  # more lines (but always same c-file)? -> used only locally -> make it static

