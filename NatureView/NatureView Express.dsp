# Microsoft Developer Studio Project File - Name="NatureView Express" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NatureView Express - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NatureView Express.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NatureView Express.mak" CFG="NatureView Express - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NatureView Express - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NatureView Express - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "NatureView Express - Win32 RelDbg" (based on "Win32 (x86) Application")
!MESSAGE "NatureView Express - Win32 Debug Static" (based on "Win32 (x86) Application")
!MESSAGE "NatureView Express - Win32 Release Static" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NatureView Express - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /D "NVW_BUILD_DIALOGS" /D "NV_CHECK_SIGNATURES" /FR /YX /FD /Zm300 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 OpenThreadsWin32.lib osg.lib osgDB.lib osgGA.lib osgUtil.lib osgText.lib osgProducer.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUseful.lib 3DNDEM.lib 3DNArg.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib advapi32.lib comdlg32.lib COMCTL32.LIB Producer.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC.lib" /out:"Release/NatureViewExpress.exe" /opt:REF
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "NatureView Express - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /D "DEBUG" /D "NVW_BUILD_DIALOGS" /FR /YX /FD /GZ /Zm300 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib osgd.lib osgDBd.lib osgGAd.lib osgUtild.lib osgTextd.lib osgProducerd.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUsefuld.lib 3DNDEMd.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib advapi32.lib comdlg32.lib COMCTL32.LIB Producerd.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"Debug/NatureViewExpress.exe" /pdbtype:sept /FORCE:MULTIPLE
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "NatureView Express - Win32 RelDbg"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NatureView_Express___Win32_RelDbg"
# PROP BASE Intermediate_Dir "NatureView_Express___Win32_RelDbg"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "NatureView_Express___Win32_RelDbg"
# PROP Intermediate_Dir "NatureView_Express___Win32_RelDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /D "NVW_BUILD_DIALOGS" /FR /YX /FD /Zm300 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 osg.lib osgDB.lib osgGA.lib osgUtil.lib osgProducer.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUseful.lib 3DNDEM.lib 3DNArg.lib zlibstat.lib /nologo /subsystem:windows /machine:I386 /out:"Release/NatureViewExpress.exe" /opt:REF
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 OpenThreadsWin32.lib osg.lib osgDB.lib osgGA.lib osgUtil.lib osgText.lib osgProducer.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUseful.lib 3DNDEM.lib 3DNArg.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib advapi32.lib comdlg32.lib COMCTL32.LIB Producer.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"NatureView_Express___Win32_RelDbg/NatureViewExpress.exe" /opt:REF
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "NatureView Express - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "NatureView_Express___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "NatureView_Express___Win32_Debug_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "NatureView_Express___Win32_Debug_Static"
# PROP Intermediate_Dir "NatureView_Express___Win32_Debug_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /D "DEBUG" /FR /YX /FD /GZ /Zm300 /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /D "DEBUG" /FR /YX /FD /GZ /Zm300 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 osgd.lib osgDBd.lib osgGAd.lib osgUtild.lib osgTextd.lib osgProducerd.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUsefuld.lib 3DNDEMd.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib comdlg32.lib Producerd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"Debug/NatureViewExpress.exe" /pdbtype:sept /FORCE:MULTIPLE
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 "Core osg Static.lib" "Core osgDB Static.lib" "Core osgGA Static.lib" "Core osgUtil Static.lib" "Core osgText Static.lib" "Core osgProducer Static.lib" opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUsefuld.lib 3DNDEMd.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib advapi32.lib comdlg32.lib COMCTL32.LIB "OT Static.lib" "Producer Static.lib" /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/NatureViewExpress.exe" /pdbtype:sept /FORCE:MULTIPLE
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "NatureView Express - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NatureView_Express___Win32_Release_Static"
# PROP BASE Intermediate_Dir "NatureView_Express___Win32_Release_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "NatureView_Express___Win32_Release_Static"
# PROP Intermediate_Dir "NatureView_Express___Win32_Release_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /FR /YX /FD /Zm300 /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_LIB" /D "BYTEORDER_LITTLEENDIAN" /FR /YX /FD /Zm300 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 osg.lib osgDB.lib osgGA.lib osgUtil.lib osgText.lib osgProducer.lib opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUseful.lib 3DNDEM.lib 3DNArg.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib comdlg32.lib Producer.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC.lib" /out:"Release/NatureViewExpress.exe" /opt:REF
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 "Core osg Static.lib" "Core osgDB Static.lib" "Core osgGA Static.lib" "Core osgUtil Static.lib" "Core osgText Static.lib" "Core osgProducer Static.lib" opengl32.lib glu32.lib gdi32.lib User32.lib 3DNUsefuld.lib 3DNDEMd.lib zlibstat.lib 3DNReq.lib 3DNCrypt.lib 3DNRand.lib advapi32.lib comdlg32.lib COMCTL32.LIB "OT Static.lib" "Producer Static.lib" /nologo /subsystem:windows /machine:I386 /out:"Release/NatureViewExpress.exe" /opt:REF
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "NatureView Express - Win32 Release"
# Name "NatureView Express - Win32 Debug"
# Name "NatureView Express - Win32 RelDbg"
# Name "NatureView Express - Win32 Debug Static"
# Name "NatureView Express - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BusyWin.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraSupport.cpp
# End Source File
# Begin Source File

SOURCE=.\Category.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateHelpSplashWatermark.cpp
# End Source File
# Begin Source File

SOURCE=.\DecompressZipScene.cpp
# End Source File
# Begin Source File

SOURCE=.\DriveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EventDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\FileAssociation.cpp
# End Source File
# Begin Source File

SOURCE=.\Forest.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameStats.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InternalImage.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MainPopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\miniunz.c
# End Source File
# Begin Source File

SOURCE=.\NavDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Navigation.cpp
# End Source File
# Begin Source File

SOURCE=.\NVAnimObject.cpp
# End Source File
# Begin Source File

SOURCE=.\NVCreateHUD.cpp
# End Source File
# Begin Source File

SOURCE=.\NVELogo256x128.c
# End Source File
# Begin Source File

SOURCE=.\NVEventHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\NVMathSupport.cpp
# End Source File
# Begin Source File

SOURCE=.\NVMiscGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\NVOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\NVParsing.cpp
# End Source File
# Begin Source File

SOURCE=.\NVRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\NVScene.cpp
# End Source File
# Begin Source File

SOURCE=.\NVSigCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\NVTerrain.cpp
# End Source File
# Begin Source File

SOURCE=.\PanoManipulator.cpp
# End Source File
# Begin Source File

SOURCE=.\RedArrow.c
# End Source File
# Begin Source File

SOURCE=.\RedDot.c
# End Source File
# Begin Source File

SOURCE=.\SceneLOD.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptArg.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolTips.cpp
# End Source File
# Begin Source File

SOURCE=.\unzip.c
# End Source File
# Begin Source File

SOURCE=.\Viewer.cpp
# End Source File
# Begin Source File

SOURCE=.\Viewpoints.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BusyWin.h
# End Source File
# Begin Source File

SOURCE=.\CameraSupport.h
# End Source File
# Begin Source File

SOURCE=.\Category.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDefines.h
# End Source File
# Begin Source File

SOURCE=.\CreateHelpSplashWatermark.h
# End Source File
# Begin Source File

SOURCE=.\DecompressZipScene.h
# End Source File
# Begin Source File

SOURCE=.\DriveDlg.h
# End Source File
# Begin Source File

SOURCE=.\EventDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\ExtentsVisitor.h
# End Source File
# Begin Source File

SOURCE=.\FileAssociation.h
# End Source File
# Begin Source File

SOURCE=.\Forest.h
# End Source File
# Begin Source File

SOURCE=.\FrameStats.h
# End Source File
# Begin Source File

SOURCE=.\HelpSheet.h
# End Source File
# Begin Source File

SOURCE=.\IdentityDefines.h
# End Source File
# Begin Source File

SOURCE=.\InternalImage.h
# End Source File
# Begin Source File

SOURCE=.\KeyDefines.h
# End Source File
# Begin Source File

SOURCE=.\MainPopupMenu.h
# End Source File
# Begin Source File

SOURCE=.\NavDlg.h
# End Source File
# Begin Source File

SOURCE=.\Navigation.h
# End Source File
# Begin Source File

SOURCE=.\NVAnimObject.h
# End Source File
# Begin Source File

SOURCE=.\NVEventHandler.h
# End Source File
# Begin Source File

SOURCE=.\NVFolSpecies.h
# End Source File
# Begin Source File

SOURCE=.\NVMathSupport.h
# End Source File
# Begin Source File

SOURCE=.\NVMiscGlobals.h
# End Source File
# Begin Source File

SOURCE=.\NVNodeMasks.h
# End Source File
# Begin Source File

SOURCE=.\NVOverlay.h
# End Source File
# Begin Source File

SOURCE=.\NVParsing.h
# End Source File
# Begin Source File

SOURCE=.\NVRequest.h
# End Source File
# Begin Source File

SOURCE=.\NVScene.h
# End Source File
# Begin Source File

SOURCE=.\NVSigCheck.h
# End Source File
# Begin Source File

SOURCE=.\NVTerrain.h
# End Source File
# Begin Source File

SOURCE=.\NVWatermark.h
# End Source File
# Begin Source File

SOURCE=.\PartialVertexDEM.h
# End Source File
# Begin Source File

SOURCE=.\SceneLOD.h
# End Source File
# Begin Source File

SOURCE=.\SceneLODPublic.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBox.h
# End Source File
# Begin Source File

SOURCE=.\ToolTips.h
# End Source File
# Begin Source File

SOURCE=.\Viewer.h
# End Source File
# Begin Source File

SOURCE=.\Viewpoints.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\3dnmove.ico
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\climb.bmp
# End Source File
# Begin Source File

SOURCE=.\drive.bmp
# End Source File
# Begin Source File

SOURCE=.\enablecat.bmp
# End Source File
# Begin Source File

SOURCE=.\EXEIcons.rc
# End Source File
# Begin Source File

SOURCE=.\exit.bmp
# End Source File
# Begin Source File

SOURCE=.\eyepoint.bmp
# End Source File
# Begin Source File

SOURCE=.\follow.bmp
# End Source File
# Begin Source File

SOURCE=.\goto.bmp
# End Source File
# Begin Source File

SOURCE=.\home.bmp
# End Source File
# Begin Source File

SOURCE=.\mvbk.bmp
# End Source File
# Begin Source File

SOURCE=.\mvfwd.bmp
# End Source File
# Begin Source File

SOURCE=.\mvlt.bmp
# End Source File
# Begin Source File

SOURCE=.\mvrt.bmp
# End Source File
# Begin Source File

SOURCE=.\nextarrow.bmp
# End Source File
# Begin Source File

SOURCE=.\options.bmp
# End Source File
# Begin Source File

SOURCE=.\pause.bmp
# End Source File
# Begin Source File

SOURCE=.\prevarro.bmp
# End Source File
# Begin Source File

SOURCE=.\prevarrow.bmp
# End Source File
# Begin Source File

SOURCE=.\rotate.bmp
# End Source File
# Begin Source File

SOURCE=.\rotdn.bmp
# End Source File
# Begin Source File

SOURCE=.\rotlt.bmp
# End Source File
# Begin Source File

SOURCE=.\rotrt.bmp
# End Source File
# Begin Source File

SOURCE=.\rotup.bmp
# End Source File
# Begin Source File

SOURCE=.\slide.bmp
# End Source File
# Begin Source File

SOURCE=.\stop.bmp
# End Source File
# Begin Source File

SOURCE=.\undo.bmp
# End Source File
# End Group
# End Target
# End Project
