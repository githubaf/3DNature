# Microsoft Developer Studio Project File - Name="3DNReq" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=3DNReq - Win32 ST_D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "3DNReq.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "3DNReq.mak" CFG="3DNReq - Win32 ST_D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "3DNReq - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "3DNReq - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "3DNReq - Win32 MT_D" (based on "Win32 (x86) Static Library")
!MESSAGE "3DNReq - Win32 ST_D" (based on "Win32 (x86) Static Library")
!MESSAGE "3DNReq - Win32 ST_R" (based on "Win32 (x86) Static Library")
!MESSAGE "3DNReq - Win32 MT_R" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "3DNReq - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "3DNReq - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_WIN32" /D "BUILD_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYTEORDER_LITTLEENDIAN" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\3DNReqd.lib"

!ELSEIF  "$(CFG)" == "3DNReq - Win32 MT_D"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "3DNReq___Win32_MT_D"
# PROP BASE Intermediate_Dir "3DNReq___Win32_MT_D"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "3DNReq___Win32_MT_D"
# PROP Intermediate_Dir "3DNReq___Win32_MT_D"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "BUILD_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYTEORDER_LITTLEENDIAN" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "_WIN32" /D "BUILD_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYTEORDER_LITTLEENDIAN" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\3DNReqd.lib"
# ADD LIB32 /nologo /out:"3DNReq___Win32_MT_D\3DNReq_MT_D.lib"

!ELSEIF  "$(CFG)" == "3DNReq - Win32 ST_D"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "3DNReq___Win32_ST_D"
# PROP BASE Intermediate_Dir "3DNReq___Win32_ST_D"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "3DNReq___Win32_ST_D"
# PROP Intermediate_Dir "3DNReq___Win32_ST_D"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "BUILD_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYTEORDER_LITTLEENDIAN" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "_WIN32" /D "BUILD_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYTEORDER_LITTLEENDIAN" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\3DNReqd.lib"
# ADD LIB32 /nologo /out:"3DNReq___Win32_ST_D\3DNReq_ST_D.lib"

!ELSEIF  "$(CFG)" == "3DNReq - Win32 ST_R"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "3DNReq___Win32_ST_R"
# PROP BASE Intermediate_Dir "3DNReq___Win32_ST_R"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DNReq___Win32_ST_R"
# PROP Intermediate_Dir "3DNReq___Win32_ST_R"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"3DNReq___Win32_ST_R\3DNReq_ST_R.lib"

!ELSEIF  "$(CFG)" == "3DNReq - Win32 MT_R"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "3DNReq___Win32_MT_R"
# PROP BASE Intermediate_Dir "3DNReq___Win32_MT_R"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DNReq___Win32_MT_R"
# PROP Intermediate_Dir "3DNReq___Win32_MT_R"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"3DNReq___Win32_MT_R\3DNReq_MT_R.lib"

!ENDIF 

# Begin Target

# Name "3DNReq - Win32 Release"
# Name "3DNReq - Win32 Debug"
# Name "3DNReq - Win32 MT_D"
# Name "3DNReq - Win32 ST_D"
# Name "3DNReq - Win32 ST_R"
# Name "3DNReq - Win32 MT_R"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\RequesterBasic.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
