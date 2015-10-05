# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=DEFG - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to DEFG - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DEFG - Win32 Release" && "$(CFG)" != "DEFG - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "DEFG.mak" CFG="DEFG - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DEFG - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DEFG - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "DEFG - Win32 Debug"
RSC=rc.exe
CPP=xicl.exe

!IF  "$(CFG)" == "DEFG - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\DEFG.exe"

CLEAN : 
	-@erase ".\Release\DEFG.exe"
	-@erase ".\Release\DEFGDiag.obj"
	-@erase ".\Release\DEFGSupport.obj"
	-@erase ".\Release\DEFGSpline.obj"
	-@erase ".\Release\main.obj"
	-@erase ".\Release\DEFG.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/DEFG.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/DEFG.bsc" 
BSC32_SBRS=
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/DEFG.pdb" /machine:I386 /out:"$(OUTDIR)/DEFG.exe" 
LINK32_OBJS= \
	"$(INTDIR)/DEFGDiag.obj" \
	"$(INTDIR)/DEFGSupport.obj" \
	"$(INTDIR)/DEFGSpline.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/DEFG.obj"

"$(OUTDIR)\DEFG.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DEFG - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\DEFG.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\DEFG.exe"
	-@erase ".\Debug\DEFGSpline.obj"
	-@erase ".\Debug\main.obj"
	-@erase ".\Debug\DEFGSupport.obj"
	-@erase ".\Debug\DEFGDiag.obj"
	-@erase ".\Debug\DEFG.obj"
	-@erase ".\Debug\DEFG.ilk"
	-@erase ".\Debug\DEFG.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/DEFG.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/DEFG.bsc" 
BSC32_SBRS=
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/DEFG.pdb" /debug /machine:I386 /out:"$(OUTDIR)/DEFG.exe" 
LINK32_OBJS= \
	"$(INTDIR)/DEFGSpline.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/DEFGSupport.obj" \
	"$(INTDIR)/DEFGDiag.obj" \
	"$(INTDIR)/DEFG.obj"

"$(OUTDIR)\DEFG.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "DEFG - Win32 Release"
# Name "DEFG - Win32 Debug"

!IF  "$(CFG)" == "DEFG - Win32 Release"

!ELSEIF  "$(CFG)" == "DEFG - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\DEFG.cpp
DEP_CPP_DEFG_=\
	".\DEFG.h"\
	".\DEFGSpline.h"\
	".\DEFGSupport.h"\
	

"$(INTDIR)\DEFG.obj" : $(SOURCE) $(DEP_CPP_DEFG_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEFGSpline.cpp

!IF  "$(CFG)" == "DEFG - Win32 Release"

DEP_CPP_DEFGS=\
	".\DEFGSpline.h"\
	

"$(INTDIR)\DEFGSpline.obj" : $(SOURCE) $(DEP_CPP_DEFGS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "DEFG - Win32 Debug"

DEP_CPP_DEFGS=\
	".\DEFGSpline.h"\
	
NODEP_CPP_DEFGS=\
	".\SplineStep"\
	

"$(INTDIR)\DEFGSpline.obj" : $(SOURCE) $(DEP_CPP_DEFGS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.cpp
DEP_CPP_MAIN_=\
	".\DEFG.h"\
	".\DEFGSupport.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEFGDiag.cpp
DEP_CPP_DEFGD=\
	".\DEFG.h"\
	

"$(INTDIR)\DEFGDiag.obj" : $(SOURCE) $(DEP_CPP_DEFGD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEFGSupport.cpp
DEP_CPP_DEFGSU=\
	".\DEFGSupport.h"\
	".\DEFG.h"\
	

"$(INTDIR)\DEFGSupport.obj" : $(SOURCE) $(DEP_CPP_DEFGSU) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
