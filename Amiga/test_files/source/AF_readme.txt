BSur.dem         - BigSur.DEM from Amiga VistaPro 3.05, because it has elevation from 0 ... 1122, i.e. covers signed/unsigned int8 and bigger
"63112I   .elev" - from WCS 2.04 WCSProjects:Arizona/GrandCanyon.object
BSur.demZB       - BigSur.DEM from Amiga VistaPro 3.05 converted to ZB with WCS 2.04, BigSur used because it has elevation from 0 ... 1122, i.e. covers signed/unsigned int8 and bigger

n54_e013_3arc_v2.dt1 - DTED file, island of Ruegen, aquired from https://earthexplorer.usgs.gov/ -> Select your Data Sets-> Digital Elevation -> SRTM -> SRTM Void Filled

Überprüfung der Files, 23. Oktober 2023
---------------------------------------
  181270 Oct 20 12:15 '36112.I   .elev'
 1065282 Oct 20 12:15  BSur.DEMAS              # start imagej", File -> Import -> Text Image, Big Sur, 2058x258 OK
  266256 Oct 20 13:37  BSur.DEMF4              # display -endian MSB -depth 32 -size 258x258 -define quantum:format=floating-point -define quantum:scale=8e+01 gray:BSur.DEMF4 # Display OK, Filesize ok
  532512 Oct 20 13:37  BSur.DEMF8              # display -endian MSB -depth 64 -size 258x258 -define quantum:format=floating-point -define quantum:scale=8e+01 gray:BSur.DEMF8 # Display OK, Filesize ok
   66564 Oct 20 13:32  BSur.DEMS1              # display -depth  8 -size 258x258 gray:BSur.DEMS1  # Display OK, Filesize ok
  133128 Oct 20 13:36  BSur.DEMS2              # display -depth 16 -size 258x258 gray:BSur.DEMS2  # Display OK, Filesize ok
  266256 Oct 20 13:36  BSur.DEMS4              # display -depth 32 -size 258x258 gray:BSur.DEMS4  # Display OK, Filesize ok
   66564 Oct 20 13:34  BSur.DEMU1              # display -depth  8 -size 258x258 gray:BSur.DEMU1  # Display OK, Filesize ok
  133128 Oct 20 13:36  BSur.DEMU2              # display -depth 16 -size 258x258 gray:BSur.DEMU2  # Display OK, Filesize ok
  266256 Oct 20 13:36  BSur.DEMU4              # display -depth 16 -size 258x258 gray:BSur.DEMU2  # Display OK, Filesize ok
  266320 Oct 20 12:15  BSur.DEMZB              # tail --bytes $((258*258*4)) BSur.DEMZB | display -endian MSB -depth 32 -size 258x258 -define quantum:format=floating-point -define quantum:scale=8e+01 gray: # Display OK
  123624 Oct 20 12:15  BSurDEMColr.iff         # display BSurDEMColr.iff # Display OK, file BSurDEMColr.iff #ILBM interleaved image, 258 x 258
   68100 Oct 20 12:15  BSurDEMGray.iff         # display BSurDEMGray.iff # FALSCH !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   60793 Oct 20 12:15  BigSur.DEM              # --- original File ---
 1593084 Oct 20 12:15  n54_e013_3arc_v2.ascarr # start imagej", File -> Import -> Text Image, Ruegen, 601x1201 OK
 1454242 Oct 20 12:15  n54_e013_3arc_v2.dt1    # --- Original File ---
  518092 Oct 20 12:15  n54_e013_3arc_v2.iff    # display n54_e013_3arc_v2.iff # Ruegen, Display OK, input is a deep (24-bit) ILBM , # file n54_e013_3arc_v2.iff    #IFF data, ILBM interleaved image, 601 x 1201

