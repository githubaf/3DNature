#!/bin/bash
# AF, HGW, 30.Nov.2023
# to be used in directory gc-sections
# shows the size of all parts that where removed by --gcsections
# data.toolchain_ver is ignored in counting unused bytes.
# is called from makefile in post-build-step (Eclipse-> Properties-> C/C++ Build -> Settings -> Build Steps ->Post Build Steps)

set -e # exit on error

if [[ $# -ge 1 ]]; then   # at least 1 parameter
   if  [ -f "$1" ]; then     # if Parameter (gcc-Linker output file) exists
       BUILD="cat $1"
   else
       printf "File '$1' not found!\n"
       exit 1
   fi
else
   BUILD='make all'       # otherwise call make all
fi

# Size is first hex number of objdump-output

printf "BUILD ist $BUILD\n"


$BUILD 2>&1 | grep "removing unused section" | awk '
//{
    File=$8; 
    RemovedElement=$5;

    cmd="m68k-amigaos-objdump -h " File "| grep " RemovedElement;
    while ( ( cmd | getline result ) > 0 ) 
    {
        print result
    }
    close(cmd);
}' | grep -v ".data.toolchain_ver" | awk '
BEGIN{
   Summe=0;
}

//{
   $3=strtonum("0x" $3); 
   Summe=Summe+$3;
   print $2 " " $3 " Bytes, aufsummiert " Summe
}

END{
print "Total: " Summe;
}'

# there are 13 unused-lines. (for instance Minstack...). Some Endian functions are used in external tests
EXPECTED_REMOVALS=13
if [ $(cat $1 | grep "removing unused section" | wc -l) -ne "$EXPECTED_REMOVALS" ]; then
   # "Error:" makes the Eclipse Post-Build Step aborting, otherwise the error is ignored, regardless of any exit code
   echo "Error: Number of unused functions/data has changed! Check sources and modify $0 if this was really intended!"
fi

