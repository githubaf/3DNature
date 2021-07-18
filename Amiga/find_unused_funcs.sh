#!/bin/bash
# AF, HGW, 15.July 2021
# searches for unused functions in the executable compiled with bebbo's gcc for Amiga
# cd into Release or Debug before calling the script!
    # to be checked... not working, so ignore  # We need the preprocessed c-files in that directory, i.e. without disturbing comments or out-defined parts.
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

ctags --recurse  # we look into "tags" to care only for our own functions. Othrewise clib functions would also be considered

for FUNCTION in $("$COMPILER_PATH"/m68k-amigaos-objdump -tC "$EXE" | awk '/\.text.*\.part/{next} /\.text/{print $6}'); do # some function are split in objdump as .part0 .part1 etc, skip them
	      	#echo "$FUNCTION"
		if [ $(grep "$FUNCTION[^a-zA-Z0-9_\=\[]" ../tags | grep -c ".") -ne "0" ]; then     # only functions that are definded in our c-Files are considered

		LINES=$(find .. -name "*.c" -exec grep -nH "$FUNCTION[^a-zA-Z0-9_\=\[]" {} \;)  # look into c-Files for that function. 
		                                                               # unfortunatelly we see also prototypes and commented/ifdeffed stuff
		                                                               # using the preprocessed files is even worse, they contain all prototypes from headers.
									       # using the *.s files does not work either. They are unreadable with -flto and do not contain names in function calls :-(

			if [ $(echo "$LINES" | grep -c '.') -eq "1" ]; then    # only one occurence in all preprocessed C-Files? -> unused function
				((UNUSED_COUNT++))
				echo -e "\e[31m$FUNCTION() unused\e[0m"
				echo "$LINES"
				echo "---------------------------------------------------------"
			elif [ $(echo "$LINES" | awk -F: '{print $1}' | sort --unique | grep -c ".") -eq "1" ]; then  # several occurences but all in one file? --> static
				if [ $(echo "$LINES" | grep -i "static" | grep -c ".") -eq "0" ]; then
					((STATIC_COUNT++))
					echo "$FUNCTION() is used only locally in"
					echo "$LINES -> make it static if not already done!"
					echo "---------------------------------------------------------"
				else
					
					echo -e "\e[32m$FUNCTION() is already static.\e[0m"
					echo "---------------------------------------------------------"
				fi

			fi
#		else
#			echo -e "\e[33m$FUNCTION() is not ours\e[0m"
#			echo "---------------------------------------------------------"
	       fi
done

echo
echo "$UNUSED_COUNT unused functions found."
echo "$STATIC_COUNT static-candidates found."

