/* wcs.rexx
*/

Options FailAt 100

Options Results

Address WCS.1

'PARAMETERS LOAD "WCS:Project/Mars/Red"'
if rc > 0 then say '1 Error was 'WCS.1.LASTERROR

PAR MOT EAR ROT
if rc > 0 then say '2 Error was 'WCS.1.LASTERROR
else say 'Motion value is 'Result

STATUS BLUE
if rc > 0 then say '3 Error was 'WCS.1.LASTERROR

STATUS PURPLE
if rc > 0 then say '4 Error was 'WCS.1.LASTERROR

STATUS
if rc > 0 then say '5 Error was 'WCS.1.LASTERROR

STATUS INQUIRE
if rc > 0 then say '6 Error was 'WCS.1.LASTERROR

STATUS INQUIRE RENDERSTATUS
if rc > 0 then say '6 Error was 'WCS.1.LASTERROR
else say 'Status is: 'Result

DATABASE ENABLED ON
if rc > 0 then say '7 Error was 'WCS.1.LASTERROR

PROJECT QUIT
if rc > 0 then say '8 Error was 'WCS.1.LASTERROR

