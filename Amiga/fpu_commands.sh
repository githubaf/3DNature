#!/bin/bash
# AF, HGW, selco, 26. April 25
# ###############################

BINARY=$1

m68k-amigaos-objdump -S -M68020 -M68881 $BINARY             |                                                           \
	                grep -E "^ +[0-9,a-f]+:"            | # disassembly lines start with spaces, address and colon  \
                        awk -F'\t' '{print $3}'             | # third tab-separated part is assembly instruction        \
                        grep "^f"                           | # all FPU mnemonics start with "f"                        \
		        awk '{print $1}'                    | # look only at the instruction, i.e. first word           \
                        sort                                |                                                           \
		        uniq -c                             | # count them (need to be sorted before)                   \
			awk '{printf "%10s  (%4d)\n",$2, $1}' # print it more pretty


# Kommas rein, header Opcode,Count rein!
