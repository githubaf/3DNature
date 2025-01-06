#!/bin/bash

#AF, HGW, 29.Aug.2022
# Perform some checks on the compiled WCS program
# Parameter: executable name


# check if test for CPU >= 68020 works
# is the test-code propably in the executable?
xxd -p $1 | tr -d '\n' | grep "223c00068020" >/dev/null # 68020-Test Code "... move.l #426016,d1 ..."
if [ $? -ne 0 ]; then
 echo "ERROR: Code for CPU-Check not found!"
 exit 1
fi

# seems to be there. Then execute it!
# | tee /dev/stderr prints the vamos output to stderr before sending it to grep, so we will see can always see it

# vamos -C00 -s8 -m8000 $1 2>&1 | tee /dev/stderr | grep "CALL:  108 Alert( alertNum\[d7\]=00068020 )" >/dev/null
vamos -C00 -s8 -m8000 $1 2>&1 | tee /dev/stderr | grep "CALL: .* 108 Alert( alertNum\[d7\]=00068020 )" >/dev/null # vamos writes (exec.library) in new versions
RET=$?
if [ $RET -ne 0 ]; then
 echo "ERROR: Check for 68020 failed!"
 exit 1
else
 echo "OK: Check for 68020 passed"
fi


##################################################

# check if test for FPU works
# is the test-code propably in the executable?
xxd -p $1 | tr -d '\n' | grep "223c0006888108000004" >/dev/null # 68881-Test Code "... move.l #428161,d1 btst #4,d0 ..."
if [ $? -ne 0 ]; then
 echo "ERROR: Code for FPU-Check not found!"
 exit 1
fi

# seems to be there. Then execute it!
# | tee /dev/stderr prints the vamos output to stderr before sending it to grep, so we will see can always see it

# vamos -C20 -s8 -m8000 $1 2>&1 | tee /dev/stderr | grep "CALL:  108 Alert( alertNum\[d7\]=00068881 )" >/dev/null
vamos -C20 -s8 -m8000 $1 2>&1 | tee /dev/stderr | grep "CALL: .* 108 Alert( alertNum\[d7\]=00068881 )" >/dev/null # vamos writes (exec.library) in new versions
if [ $? -ne 0 ]; then
 echo "ERROR: Check for FPU failed!"
 exit 1 
else
 echo "OK: Check for FPU passed"
fi

