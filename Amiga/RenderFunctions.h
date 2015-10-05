* BitMaps.c                      * ColorBlends.c
  Commands.c                     * DataBase.c
* DataOps.c                        DataOpsGUI.c
* DEM.c                            DiagnosticGUI.c
* DLG.c                            EdDBaseGUI.c
  EdEcoGUI.c                       EditGui.c
  EdMoGUI.c                      * EdPar.c
  EdSetGUI.c                       EvenMoreGUI.c
* GlobeMap.c                     * GlobeMapSupport.c
  GrammarTable.c                   GUI.c
  Images.c                       * InteractiveDraw.c
* InteractiveUtils.c             * InteractiveView.c
* LineSupport.c                  * MakeFaces.c
* Map.c                          * MapExtra.c
  MapGUI.c                       * MapLineObject.c
* MapSupport.c                   * MapTopo.c
* MapTopoObject.c                * MapUtil.c
  MarkovTable.c                  * Memory.c
  MoreGUI.c                      * Params.c
  ParamsGUI.c                      RequesterGUI.c
  RexxSupport.c                  * ScratchPad.c
  ScreenModeGUI.c                * Support.c
  TimeLinesGUI.c                   TLSupportGUI.c
* Tree.c                           Version.c
  VocabTable.c                   * WCS.c                            



GlobeMap.c***************************************************************

globemap()===============================================================
FindTask
SetTaskPri
Close_Render_Window()
Close_Diagnostic_Window()
FixPar()
strcpy
strcat
get_Memory()
User_Message()
memcpy
initmopar()
initpar()
make_window()
Log()
LoadRGB4
SetRast
ModifyIDCMP
AvailMem
sprintf
openbitmaps()
AllocQCmaps()
BuildKeyTable()
LoadForestModels()
BusyWin_New()
time
setvalues()
sqrt
tan
cos
setview()
autoactivate()
strmfp
fopen
InitDEMMap()
fclose
InitVectorMap()
MergeZBufBack()
makesky()
antialias()
InterlaceFields()
savebitmaps()
saveILBM()
User_Message_Def()
getfilename()
SaveZBuf()
memset
Log_ElapsedTime()
Move
ClearScreen
BusyWin_Update()
Open_Diagnostic_Window()
BusyWin_Del()
free_Memory()
freeQCmaps()
FreeKeyTable()
closebitmaps()
UndoPar()

InitDEMMap()=============================================================
strcpy
chdir
strcat
sprintf
Log()
BusyWin_New()
strcmp
strmfp
readDEM()
get_Memory()
User_Message_Def()
LoadImage()
free_Memory()
makerelelfile()
User_Message()
maptopoobject()
BusyWin_Update()
BusyWin_Del()

InitVectorMap()==========================================================
BusyWin_New()
Load_Object()
free_Memory()
memcpy
maplineobject()
freevecarray()
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()

Handle_Render_Window()===================================================
setclipbounds()
QuickFetchEvent()
Close_Render_Window()
LoadRGB4
Set_Diagnostic_Point()
ModifyIDCMP

Close_Render_Window()====================================================
closesharedwindow()

MapTopoObject.c**********************************************************

maptopoobject()==========================================================
sprintf
BusyWin_New()
cos
sqrt
sin
getscrncoords()
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()
abs
setfaceone()
faceone()
renderface()
setfacetwo()
facetwo()
renderface()
writelinefile()
User_Message()
fclose
getfilename()
strmfp
fopen
SetAPen
FadeLine()
EndDrawLine()

renderface()==============================================================
max
srand48
drand48
PointSort2()
MapTopo()
swmem
recurse()

setfaceone()==============================================================
setface()

setfacetwo()==============================================================
setface()

setface()=================================================================

PointSort2()==============================================================
swmem

MapTopo.c*****************************************************************

MapTopo()=================================================================
sqrt
sin
abs
colormap()
ecoset()
Log()
min
max
free_Memory()
get_Memory()
memset
SetAPen
WritePixel
DetailTree()

colormap()================================================================
ecoset()
abs
Log()
colmapavg()

MapUtil.c*****************************************************************

getscrnpt()===============================================================
convertpt()
findposvector()

getscrncoords()===========================================================
getscrnpt()
sqrt

getviewscrncoords()=======================================================
getscrnpt()

convertpt()===============================================================
abs
sin
sqrt

findposvector()===========================================================
sqrt

findangle()===============================================================
atan

findangle2()==============================================================
atan

findangle3()==============================================================
atan

rotate()==================================================================
sqrt

rotate2()=================================================================
sqrt

recurse()=================================================================
polydivide()
recurse()
PointSort2()
abs
MapTopo()

polydivide()==============================================================

autoactivate()============================================================
get_Memory()
memcpy
initinterview()
Log()
sortrenderlist()
free_Memory()
sprintf

sortrenderlist()=========================================================
get_Memory()
Log()
abs
swmem
free_Memory()

SortAltRenderList()======================================================
get_Memory()
Log()
abs
swmem
free_Memory()

setclipbounds()==========================================================

setlineseg()=============================================================

ClipDraw()===============================================================
Move
Draw
swmem

ClipAreaDraw()===========================================================
AreaMove
AreaEnd
ClipPoly4

ClipPoly4()==============================================================
setlineseg()
ClipPolySeg()
AreaMove
AreaDraw
AreaEnd

ClipPolySeg()============================================================
swmem

UTM_LatLon()=============================================================
sqrt
sin
cos
tan
pow

MakeFaces.c**************************************************************

prepfaceone()============================================================

prepfacetwo()============================================================

faceone()================================================================
writeface()

facetwo()================================================================
writeface()

writeface()==============================================================
minmax()
facecompute()

minmax()=================================================================

facecompute()============================================================
cos
findangle()
atan
sqrt
sin
fabs

ColorBlends.c************************************************************

ecoset()=================================================================
SetScreenColor()
sqrt
sin
seashoal()

colmapavg()==============================================================

seashoal()===============================================================
sin
cos
abs

SetScreenColor()=========================================================

MapLineObject.c**********************************************************

maplineobject()==========================================================
getscrncoords()
writelinefile()
fclose
Log
User_Message()
getfilename()
strmfp
sprintf
fopen
User_Message_Def()
strcmp
sqrt
sin
SetAPen
FadeLine()
EndDrawPoint()
EndDrawLine()

LineSupport.c************************************************************

FadeLine()===============================================================

EndDrawLine()============================================================
swmem
WritePixel

EndDrawPoint()===========================================================
WritePixel

writelinefile()==========================================================
fprintf

InitDigPerspective()=====================================================
User_Message_Def()
DBaseObject_New()
strcmp
User_Message()
allocvecarray()

QuitDigPerspective()=====================================================
User_Message_Def()
saveobject()
savedbase()
freevecarray()

PerspectivePoint()=======================================================

VectorToPath()===========================================================
NoLoad_Message()
Load_Object()
User_Message()
CountKeyFrames()
sprintf
GetInputString()
atoi
DeleteKeyFrame()
User_Message_Def()
SetPointer
initmopar()
MakeKeyFrame()
free_Memory()
ClearPointer

PathToVector()===========================================================
NoLoad_Message()
User_Message()
User_Message_Def()
BuildKeyTable()
CountKeyFrames()
strcpy
GetInputString()
strlen
strcat
strnicmp
freevecarray()
DataBase_Copy()
strncpy
allocvecarray()
SetPointer
initmopar()
FreeKeyTable()
ClearPointer
setclipbounds()
outline()
Set_DE_Item()
saveobject()
savedbase()

DiagnosticGUI.c********************************************************

Open_Diagnostic_Window()===============================================
get_Memory()
Set_Param_Menu()
Close_Diagnostic_Window()
User_Message()
DoMethod()
MUI_DoNotiPresFal()

Close_Diagnostic_Window()==============================================
QuitDigPerspective()
DoMethod()
MUI_DisposeObject()
free_Memory()
freeQCmaps()

Handle_Diagnostic_Window()=============================================
QuitDigPerspective()
DoMethod()
allocvecarray()
Close_Diagnostic_Window()

Set_Diagnostic_Point()=================================================
sprintf
DoMethod()

EdPar.c****************************************************************

ExportWave()===========================================================
SetPointer
CountKeyFrames()
get_Memory()
BuildKeyTable()
strcpy
Set_LWM()
fopen
fprintf
fclose
free_Memory()
FreeKeyTable()
ClearPointer
NoLoad_Message()
User_Message()

Set_LWM()==============================================================
ComputeBanking()
findangle()
atan
convertpt()
findposvector()
findangle2()
rotate()

ImportWave()===========================================================
SetPointer
CountKeyFrames()
User_Message_Def()
fopen
fgets
strcmp
fclose
fscanf
get_Memory()
DeleteKeyFrame()
cos
abs
tan
sqrt
MakeKeyFrame()
free_Memory()
ClearPointer
NoLoad_Message()
User_Message()
sprintf
Log

CreateBankKeys()=======================================================
CountKeyFrames()
User_Message()
strcpy
GetInputString()
atoi
SetPointer
BuildKeyTable()
Set_Bank_Key()
FreeKeyTable()
ClearPointer()

Set_Bank_Key()=========================================================
ComputeBanking()
MakeKeyFrame()

ScaleKeys()============================================================
swmem

defaultsettings()======================================================

initmopar()============================================================

initpar()==============================================================

setvalues()============================================================

boundscheck()==========================================================

setecodefault()========================================================

Sort_Eco_Params()======================================================
swmem

FixPar()===============================================================
memcpy
free_Memory()
MergeKeyFrames()

UndoPar()==============================================================
memcpy
free_Memory()
get_Memory()

loadparams()===========================================================
memcpy
strcpy
getparamfile()
strmfp
fopen
Log()
fread
strncmp
User_Message()
User_Message_Def()
free_Memory()
get_Memory()
fseek
strcmp
sprintf
saveparams()
memset

saveparams()============================================================
strcpy
getparamfile()
stcgfe
sprintf
strcmp
strmfp
fopen
Log
User_Message()
fwrite
fopen
fseek
strncmp
fclose
fread

DefaultParams()=======================================================
loadtopo()
DeleteKeyFrame()
sqrt
defaultsettings()
memset
SetParColor()
setecodefault()
SetParEco()
strncpy
free_Memory()
get_Memory()
User_message()

SetParColor()=========================================================
strcpy

SetParEco()===========================================================
strcpy

BitMaps.c*************************************************************

openbitmaps()=========================================================
get_Memory()

savebitmaps()=========================================================
strcpy
strcat
fopen
get_Memory()
free_Memory()
fwrite
open
write
fclose
close

SaveZBuf()============================================================
strcpy
get_Memory()
strcat
open
write
saveILBM()
CheckIFF()
FindIFFChunk()
tell
read
lseek
strncpy
free_Memory()

closebitmaps()========================================================
free_Memory()

allocQCmaps()=========================================================
get_memroy()
freeQCmaps()

freeQCmaps()==========================================================
free_Memory()

saveILBM()============================================================
getfilename()
strmfp
open
close
User_Message()
strcpy
CheckIFF()
read
FindIFFChunk()
tell
lseek
get_Memory()
write
memset
memcpy
free_Memory()
FlushOutputBuff()
CompressRows()
Log

ModFileName()==========================================================
strlen
isdigit
sprintf
stricmp
strcpy
strcat

CompressRows()=========================================================
memcpy
FlushOutputBuff()

FlushOutputBuff()======================================================
write

LoadImage()============================================================
open
strlen
CheckIFF()
FindIFFChunk()
read
close
get_Memory()
memcpy
memset
lseek
stricmp
strcat
free_Memory()
Log()

LoadZBuf()=============================================================
open
CheckIFF()
FindIFFChunk()
read
lseek
close
Log()

CheckIFF()=============================================================
read
strncmp

FindIFFChunk()=========================================================
read
strncmp
lseek

MergeZBufBack()========================================================
strcpy
ModFileName()
strmfp
get_memory()
LoadZBuf()
open
CheckIFF()
FindIFFChunk()
read
lseek
openbitmaps()
LoadImage()
makesky()
close
free_Memory()
User_Message()

InterlaceFields()======================================================
open
lseek
close
read
remove
strcat

GlobeMapSupport.c******************************************************

ComputeBanking()=======================================================

setview()==============================================================
ComputeBanking()
convertpt()
findposvector()
findangle2()
rotate()
cos
sin
sprintf
Log()

setquickview()==========================================================
ComputeBanking()
convertpt()
findposvector()
findangle2()
rotate()
cos
sin

makesky()===============================================================
BusyWin_New()
cos
findangle2
sin
getscrnpt()
convertpt()
findposvector()
rotate()
sprintf
Log()
sqrt
srand48
abs
getskypt()
drand48
BusyWin_Update()

getskypt()==============================================================
atan
sqrt
cos
sin

antialias()=============================================================
abs

Support.c***************************************************************

make_window()===========================================================
OpenWindow

FetchEvent()============================================================
GetMsg
Wait
memcpy
ReplyMsg

QuickFetchEvent()=======================================================
memcpy
ReplyMsg

closesharedwindow()=====================================================
Forbid
stripintuimessages()
ModifyIDCMP
Permit
CloseWindow

stripintuimessages()====================================================
Remove
ReplyMsg

SaveConfig()============================================================
chdir
mkdir
fopen
User_Message()
fprintf
fclose

LoadConfig()============================================================
fopen
User_Message()
loaddbase()
loadparams()
FixPar()
fscanf
Make_DB_Window()
Make_DO_Window()
Make_EP_Window()
Make_DE_Window()
Make_EE_Window()
Make_EC_Window()
Make_EM_Window()
Make_ES_Window()
interactiveview()
Make_EMIA_Window()
Make_EETL_Window()
Make_ECTL_Window()
Make_EMTL_Window()
Make_FM_Window()
Make_LW_Window()
Make_AN_Window()
map()
setclipbounds()
makemap()
ShowView_Map()
ShowPaths_Map()
MapGUI_Update()
Make_EL_Window()
Make_DL_Window()
Make_DC_Window()
Make_DI_Window()
Make_DM_Window()
Make_EMPL_Window()
Make_PJ_Window()
Make_SC_Window()
Make_TS_Window()
Make_PR_Window()
Make_MA_Window()
fclose

SaveProject()==========================================================
strcpy
getfilename()
strmfp
fopen
User_Message()
fprintf
fclose
Log()
User_Message_Def()
savedbase()
saveparams()

LoadProject()==========================================================
strcpy
getfilename()
strmfp
fopen
User_Message()
fgets
strcmp
fclose
fscanf
loaddbase()
Set_DE_List()
Set_DE_Item()
loadparams()
FixPar()
DirList_Del()
DirList_New()
DirList_Add()
UnsetKeyFrame()
DisableKeyButtons()
GetKeyTableValues()
SetAllColorRequester()
Set_EC_List()
Set_EC_Item()
Set_EM_List()
Set_EM_Item()
Set_Radial_Txt()
OpenNewIAGridSize()
Close_EMIA_Window()
Set_EE_List()
Set_EE_Item()
Set_ES_Window()
ClearWindow()
loadmapbase()
valueset()
valuesetalign()
setclipbounds()
makemap()
ShowView_Map()
ShowPaths_Map()
MapGUI_Update()
Log()

LoadDirList()=========================================================
strcpy
getfilename()
strmfp
fopen
User_Message()
fgets
strcmp
fclose
fscanf
DirList_Del()
DirList_New()
DirList_Add()
fclose
Log()

PrintScreen()=========================================================
CreatePort
CreateExtIO
OpenDevice
BusyWin_New()
Delay
SendIO
DoMethod()
AbortIO
WaitIO
GetMsg
Wait
BusyWin_Del()
CloseDevice
DeleteExtIO
DeletePort

Params.c***************************************************************

MergeKeyFrames()=======================================================
memmove
AllocNewKeyArray()
sprintf
User_Message()
memcpy

MakeKeyFrame()=========================================================
SearchKeyFrame()
DeleteKeyFrame()
AllocNewKeyArray()
memove
memset
SetKeyFrame()

AllocNewKeyArray()=====================================================
get_Memory()
User_Message()
memcpy
free_Memory()

DeleteKeyFrame()=======================================================
memove

SearchKeyFrame()=======================================================

SetKeyFrame()==========================================================

UnsetKeyFrame()========================================================
UnsetKeyItem()

UnsetKeyItem()=========================================================

UpdateKeyFrames()======================================================
SetKeyFrame()

BuildKeyTable()========================================================
FreeKeyTable()
get_Memory()
CountKeyFrames()
free_Memory()
SetKeyTableEntry()
SplineAllKeys()

BuildSingleKeyTable()==================================================
CountKeyFrames()
FreeSingleKeyTable()
get_Memory()
free_Memory()
SetKeyTableEntry()
SplineSingleKey()

GetKeyTableValues()====================================================
BuildKeyTable()
FreeKeyTable()

CountKeyFrames()=======================================================

GetActiveKey()=========================================================

GetNextKeyItem()=======================================================
CountKeyFrames()

SetKeyTableEntry()=====================================================

FreeKeyTable()=========================================================
free_Memory()

FreeSingleKeyTable()===================================================
free_Memory()

SplineSingleKey()======================================================
get_Memory()

SplineAllKeys()========================================================
get_Memory()
FreeKeyTable()
User_Message()
DistributeVelocity()

Play_Colors()==========================================================
SetScreen
sprintf
DoMethod()
Delay
CheckInput_ID()

Tree.c*****************************************************************

DetailTree()===========================================================
max
min
SetAPen
free_Memory()
get_Memory()
memset
WritePixel	

LoadForestModels()=====================================================
strcpy
ModFileName()
strmfp
fopen
fscanf
strcmp
fclose
get_Memory()
User_Message()
Log()
free_Memory()

RequesterGUI.c******************************************************

getdbasename()======================================================
DoMethod()
AllocAslRequestTags
AslRequestTags
FreeAslRequest
strcpy

getparamfile()======================================================
DoMethod()
AllocAslRequestTags
AslRequestTags
FreeAslRequest
strcpy

getfilename()=======================================================
DoMethod()
AllocAslRequestTags
AslRequestTags
FreeAslRequest
strcpy

getmultifilename()==================================================
DoMethod()
AllocAslRequestTags
AslRequestTags
FreeAslRequest
strcpy

freemultifilename()=================================================
FreeAslRequest

BusyWin_New()=======================================================
Set_Param_Menu()
get_Memory()
time
DoMethod()
MUI_DoNotiPresFal()
BusyWin_Del()
free_Memory()

BusyWin_Del()=======================================================
DoMethod()
MUI_DisposeObject()
free_Memory()

BusyWin_Update()====================================================
DoMethod()
time
strcpy
ctime
strncmp
sprintf

Log_ElapsedTime()===================================================
time
sprintf
strcat
Log()

KeyButtonFunc()=====================================================

MUI_DoNotifyPressed()===============================================
DoMethod()

MUI_DoNotiPresFal()=================================================
va_start
va_arg
DoMethod()
va_end

Memory.c************************************************************

get_Memory()========================================================
AllocMem
sprintf
Log()

free_Memory()=======================================================
FreeMem

DEM.c***************************************************************

readDEM()===========================================================
open
read
abs
Log()
close
lseek
get_Memory()
FindElMaxMin()
saveDEM()

saveDEM()===========================================================
open
write
close

makerelelfile()=====================================================
enterbox()
strsfn
strcmp
Log()
strmfp
readDEM()
get_Memory()
memcpy
padarray()
computerelel()
strcat
open
FindElMaxMin()
write
close
free_Memory()

padarray()=========================================================
memcpy

enterbox()=========================================================

computerelel()=====================================================
BusyWin_New()
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()

InterpDEM()========================================================
BusyWin_New()
strcpy
User_Message()
strsfn
strcmp
Log()
User_Message_Def()
GetInputString()
strlen
strmfp
readDEM()
free_Memory()
SplineMap()
memcpy
strcat
open
FindElMaxMin()
write
close
DataBase_Copy()
strncpy
Add_DE_NewItem()
memset
saveobject()
CheckInput_ID()
BusyWin_Update()
fopen
makerelelfile()
fclose
BusyWin_Del()

SplineMap()=========================================================
BusyWin_New()max
min
abs
drand48
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()

ExtractDEM()========================================================
BusyWin_New()
strcpy
User_Message()
strmfp
fopen
Log()
Read_USGSHeader()
fclose
Read_USGSProfHeader()
atoi
Set_DM_Data()
get_Memory()
Read_USGSDEMProfile()
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()
free_Memory()
memcpy
stcat
strlen
open
FindElMaxMin()
write
close
strcmp
DataBase_Copy()
strncpy
Add_DE_NewItem()
memset
saveobject()

Read_USGSHeader()====================================================
fgets
Set_DM_HdrData()

Read_USGSProfHeader()================================================
fgets
atoi
Set_DM_ProfData()

Read_USGSDEMProfile()================================================
fascanf

FCvt()===============================================================
isdigit
atof
pow
atoi

FixFlatSpots()=======================================================
get_Memory()
memcpy
swmem
free_Memory()
User_Message()

FindElMaxMin()=======================================================
abs

LoadVistaDEM()=======================================================
fopen
fseek
fread
get_Memory()
fclose
free_Memory()

Database.c***********************************************************

loaddbase()==========================================================
strcpy
getdbasename()
strmfp
fopen
Log()
fgets
strcmp
fclose
User_Message()
fscanf
DataBase_Copy()
DataBase_Del()
DataBase_New()
sprintf
strcat
strmfp
DirList_ItemExists()
DirList_Add()
DirList_New()

makedbase()===========================================================
strcpy
DataBase_New()
savedbase()
DataBase_Del()

savedbase()===========================================================
strcpy
getdbasename()
strmfp
fopen
fclose
FileExists_Message()
Log()
fprintf
fputs
User_Message()
mkdir
User_Message_Def()
DirList_ItemExists()
DirList_Add()
Update_DL_Win()

DataBase_New()=======================================================
get_Memory()
User_Message()

DataBase_Copy()======================================================
DataBase_New()
memcpy
DataBase_Del()
FreeAllVectors()
savedbase()
free_Memory()

loadmapbase()========================================================
freevecarray()
BusyWin_New()
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()
free_Memory()
sprintf
Log()

Load_Object()========================================================
freevecarray()
strmfp
strcat
fopen
Log()
fread
strcmp
User_Message()
allocvecarray()
fclose
fscanf
sprintf

Find_DBObjPts()======================================================
fopen
fread
strcmp
abs
fclose
fscanf

DirList_New()========================================================
get_Memory()
strcpy

DirList_Add()========================================================
strcpy
getfilename()

DirList_Remove()=====================================================
DirList_Search()
free_Memory()

DirList_Search()=====================================================

DirList_ItemEXists()=================================================
strcmp

DirList_Move()=======================================================
DirList_Search()
swmem

DirList_Del()========================================================
free_Memory()

DirList_Copy()=======================================================
get_Memory()
stcpy

DBaseObject_New()====================================================
GetInputString()
strlen
strnicmp
User_Message_Def()
DataBase_Copy()
User_Message()
memcpy
strncpy
strcat
allocvecarray()
setclipbounds()
outline()
DoMethod()

DBaseObject_Add()=====================================================
strcpy
getmultifilename()
User_Message()
strsfn
strcmp
Log()
strlen
strnicmp
User_Message_Def()
strmfp
Find_DBObjPts()
DataBase_Copy()
memcpy
strncpy
strcat
fopen
fclose
Add_DE_NewItem()
Map_DB_Object()
freemultifilename()

MapSupport.c**********************************************************

savemapprefs()========================================================
strmfp
fopen
fprintf
fclose

readmapprefs()========================================================
strmfp
fopen
fgets
strcmp
fclose
fscanf

saveobject()==========================================================
strmfp
fopen
fclose
strcat
Log()
User_Message()
strcpy
strncpy
fwrite
sprintf

loadtopo()============================================================
free_Memory()
strcmp
User_Message()
get_Memory()
BusyWin_New()
BusyWin_Update()
strmfp
strcat
readDEM()
sprintf
Log()
abs
CheckInput_ID()
BusyWin_Del()
sqrt

GetBounds()==========================================================
MapGUI_Message()
SetWindowTitles
MousePtSet()
MapIDCMP_Restore()

valueset()===========================================================
cos

valuesetalign()======================================================

MousePtSet()=========================================================
ModifyIDCMP
SetDrMd
Move
FetchEvent()
Draw
sprintf
LatLonElevScan()
MapGUI_Message()

LatLonElevScan()=====================================================
strcmp
sprintf
MapGUI_Message()

XY_latlon()==========================================================
allocvecarray()
User_Message()

latlon_XY()==========================================================

makebox()============================================================
Move
Draw

outline()============================================================
Load_Object()
latlon_XY()
SetAPen
SetBPen
SetDrPt
abs
setlineseg()
ClipDraw()
WritePixel
DrawEllipse
RectFill

getval()=============================================================
FetchEvent()
Move
Text

FreeAllVectors()=====================================================
User_Message_Def()
saveobject()
freevecarray()

freevecarray()=======================================================
free_Memory()

allocvecarray()======================================================
get_Memory()
memcpy
free_Memory()

SetView_Map()========================================================
MapGUI_Message()
SetWindowTitles
setclipbounds
MousePtSet()
MapIDCMP_Restore()
ShowView_Map()
setcompass()
drawgridview()
constructview()
DrawInterFresh()
Update_EM_Item()

ShowView_Map()=======================================================
SetPointer
SetDrMd
SetWrMsk
azimuthcompute()
ShowCenters_Map()
ClearPointer

SetIAView_Map()======================================================
abs
SetWindowTitles
sqrt
ModifyIDCMP
setclipbounds
SetDrMd
SetWrMsk
setcompass()
ShowCenters_Map()
constructview()
DrawInterFresh()
Update_EM_Item()
azimuthcompute()
FetchEvent()
drawgridpts()
findfocprof()
drawquick()
computequick()
drawfocprof()
drawveryquick()
makeviewcenter()
MapIDCMP_Restore()
drawgridview()

ShowCenters_Map()=====================================================
setlineseg()
ClipDraw()
DrawHaze()
DrawSun()
sin
cos
sqrt
tan

ShowPaths_Map()=======================================================
SetPointer
SetDrMd
SetWrMsk
BuildKeyTable()
setlineseg()
ClipDraw()
FreeKeyTable
ClearPointer

MakeColorMap()========================================================
strcmp
User_Message()
User_Message_Def()
loadtopo()
get_Memory()
SetPointer
swmem
ClearPointer
strcpy
getfilename()
strmfp
saveILBM()
free_Memory()

DrawHaze()============================================================
setlineseg()
ClipDraw()
WritePixel

DrawSun()=============================================================
DrawHaze()
WritePixel
setlineseg()
ClipDraw()

MapExtra.c************************************************************

findarea()============================================================
abs
strcmp
sprintf
strcat
MapGUI_Message()

setorigin()===========================================================
User_Message_Def()
setclipbounds()
MapGUI_Message()
SetWindowTitles
modpoints()
outline()
sprintf
Log()
loadtopo()
maptotopo()

matchpoints()=========================================================
setclipbounds()
MapGUI_Message()
SetWindowTitles
modpoints()
findmouse()
outline()
User_Message()
abs
User_Message_Def()
allocvecarray()
sprintf
Log()
loadtopo()
maptotopo()

modpoints()===========================================================
ModifyIDCMP
markpt()
sprintf
MapGUI_Message()
FetchEvent()
unmarkpt()
SetAPen
Move
Draw
MapIDCMP_Restore()
User_Message_Def()
loadtopo()
maptotopo()
Log()

markpt()==============================================================
ReadPixel
SetAPen
Move
Draw

unmarkpt()============================================================
SetAPen
WritePixel

CloneObject()=========================================================
User_Message_Def()
setclipbounds()
DBaseObject_New()
allocvecarray()
memcpy
outline()

makestream()==========================================================
loadtopo()
setclipbounds()
MapGUI_Message()
SetWindowTitles
SetAPen
MousePtSet()
strcpy
sqrt
abs
memcpy
allocvecarray()
User_message()
outline()
sprintf
free_Memory()
saveobject()
MapIDCMP_Restore()

interpolatepath()===================================================
setclipbounds()
cos
rotate2()
findangle3()
sqrt
sin
sprintf
GetInputString()
atoi
User_Message_Def()
DistributeVelocity()
allocvecarray()
outline()
loadtopo()
maptotopo()
Log()
User_Message()

SetSurface_Map()======================================================
sprintf
MapGUI_Message()
SetWindowTitles
MousePtSet()
latlon_XY()
DoMethod()
MapIDCMP_Restore()

FlatFixer()===========================================================
swmem
strcmp
get_Memory()
memcpy
sprintf
GetInputString()
abs
makemap()
User_Message_Def()
FixFlatSpots()
strmfp
strcat
open
close
FindElMaxMin()
write
Log()
free_Memory()
User_Message()

BuildElevTable()=======================================================
loadtopo()
BuildKeyTable()
get_Memory()
FreeKeyTable()

BuildVelocityTable()===================================================
BuildKeyTable()
cos
get_Memory
FreeKeyTable()

DistributeVelocity()===================================================
get_Memory()
cos
sqrt
free_Memory()

MakeDEM()==============================================================
GetInputString()
atoi
atof
User_Message_Def()
pow2
get_Memory()
DoGauss()
pow
f4
f3
strcpy
getfilename()
strmfp
fopen
fwrite
fclose

InitGauss()===========================================================
sqrt
srand48

DoGauss()=============================================================
drand48

Map.c*****************************************************************

alignmap()============================================================
User_Message()
MapGUI_Message()
SetWindowTitles
MousePtSet()
User_Message_Def()
swmem
SetAPen
Move
Draw
MapIDCMP_Restore()

makemap()=============================================================
User_Message()
DoMethod()
SetPointer
EnsureLoaded()
BusyWin_New()
TempRas_new()
get_Memory()
memset
initpar()
initmopar()
setvalues()
CalcRefine()
BusyWin_Update()
latlon_XY()
max
min
MapRelel_Load()
Set_Eco_Color()
WritePixelLine8
CheckInput_ID()
User_Message_Def()
MapRelel_Free()
free_Memory()
TempRas_Del()
outline()
BusyWin_Del()
ClearPointer

EnsureLoaded()=======================================================
loadtopo()
strcmp
free_Memory()
memcpy
loadtopo()

MapRelel_Load()======================================================
get_Memory()
User_Message()
strmfp
strcat
readDEM()
fopen
fclose
strcpy
makerelelfile()
Log()
free_Memory()

MapRelel_Free()======================================================
free_Memory()

CalcRefine()=========================================================
CheckInput_ID()

shiftmap()===========================================================
MapGUI_Message()
SetWindowTitles
MousePtSet()
MapIDCMP_Restore()

maptotopo()==========================================================
memset
saveobject()

findmouse()==========================================================
setclipbounds()
MapGUI_Message()
SetWindowTitles
ModifyIDCMP
FetchEvent()
MapIDCMP_Restore()
outline()
latlon_XY()
User_Message_Def()
DoMethod()
strcmp
User_Message()

findmulti()=========================================================
Make_DE_Window()
User_Message_Def()
MapGUI_Message()
SetWindowTitles
MousePtSet()
MapIDCMP_Restore()
swmem
setclipbounds()
outline()
latlon_XY()
DoMethod()

addpoints()=========================================================
User_Message_Def()
DBaseObject_New()
strcmp
User_Message()
setclipbounds()
get_Memory()
memcpy
allocvecarray()
outline()
InputTabletPoints()
freevecarray()
free_Memory()
loadtopo()
maptotopo()
sprintf
Log()
SetAPen
Move
Draw
ModifyIDCMP
strcpy
MapGUI_Message()
FetchEvent()
WritePixel
LatLonElevScan()
abs
interpolatepts()
XY_latlon()
MapIDCMP_Restore()

interpolatepts()====================================================
abs
WritePixel

Viewshed_Map()======================================================
User_Message_Def()
Load_Object()
User_Message()
Close_Viewshed_Window()
OpenWindowTags
Log()
get_Memory()
setclipbounds()
SetAPen
RectFill
loadtopo()
sprintf
GetInputString()
atoi
latlon_XY()
max
min
BusyWin_New()
memcpy
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()
memset
SearchViewLine()
WritePixel
makemap()

Close_Viewshed_Window()==============================================
free_Memory()
closesharedwindow()

Handle_Viewshed_Window()=============================================
QuickFetchEvent()
Close_Viewshed_Window()

SearchViewLine()=====================================================
abs

TempRas_New()========================================================
get_Memory()
memcpy
free_Memory()

TempRas_Del()========================================================
free_Memory()

DitherTable_New()====================================================
get_Memory()
drand48

DitherTable_Del()====================================================
free_Memory()

InputTabletPoints()==================================================
CreateMsgPort
CreateExtIO
OpenDevice
User_Message()
SendIO
Delay
AbortIO
WaitIO
MapGUI_Message()
Wait
CheckIO
atoi
sprintf
Log()
sqrt
atan
RotatePt()
CloseDevice
DeleteExtIO
DeleteMsgPort

RotatePt()==========================================================
sqrt
atan
cos
sin

FindCenter()========================================================

ClearWindow()=======================================================
SetAPen
RectFill

Set_Eco_Color()=====================================================
faceone()
abs
Log()

InteractiveView.c***************************************************

interactiveview()===================================================
WindowToFront
DoMethod()
User_Message()
Log()
get_Memory()
openinterview()
SetPointer
initinterview()
User_Message_Def()
closeviewmaps()
ClearPointer
Init_IA_View()
setcompass()
SetWindowTitles
closeinterview()

openinterview()=====================================================
make_window()
WindowLimits
ModifyIDCMP
make_compass()
setclipbounds()

initinterview()=====================================================
strcmp
get_Memory()
stmfp
strcat
readDEM()
Log()
memcpy
computesinglebox()
free_Memory()
sprintf

closeviewmaps()=====================================================
free_Memory()

closeinterview()====================================================
DoMethod()
QuitDigPerspective()
Close_Small_Window()
closesharedwindow()
free_Memory()
LoadRGB4

shaderelief()=======================================================
get_Memory()
User_Message()
SetPointer
BusyWin_New()
constructview()
setvalues()
User_Message_Def()
maptopoobject()
free_Memory()
BusyWin_Update()
InitVectorMap()
BusyWin_Del()
ClearPointer

smallwindow()========================================================
User_Message()
Close_Diagnostic_Window()
ModifyIDCMP
SetDrMd
SetWindowTitles
FetchEvent()
Move
Draw
swmem
get_Memory()
sprintf
make_window()
Log()
SetPointer
memcpy
AllocQCmaps()
initpar()
constructview()
setvalues()
LoadRGB4
strmfp
strcat
readDEM()
strcpy
free_Memory()
strcmp
makerelelfile()
maptopoobject()
ClearPointer
Open_Diagnostic_Window()
QuickFetchEvent()
Close_Small_Window()

Close_Small_Window()===============================================
Close_Diagnostic_Window()
free_Memory()
CloseWindow

Handle_Small_Window()==============================================
QuickFetchEvent()
Close_Small_Window()
LoadRGB4
memcmp
memcpy
Set_EM_Item()
Set_Diagnostic_Point()
ModifyIDCMP
PerspectivePoint()

Handle_InterWind0()================================================
QuickFetchEvent()
Close_EMIA_Window()
LoadRGB4
setclipbounds
Init_IA_View()
DoMethod()
modifycampt()
modifyfocpt()
modifyinteractive()
boundscheck()
constructview()
SetAPen
RectFill
computeview()
drawinterview()
drawquick()
computequick()
drawquick()
drawfocprof()
findfocprof()
drawveryquick()
makeviewcenter()
Update_EM_Item()
constructview()
InitVectorMap()
RefreshWindowFrame
CheckDEMStatus()
OpenNewIAGridSize()
Close_EMIA_Window()
drawgridview()
autocenter()
shaderelief()
smallwindow()
Init_Anim()
MakeMotionKey()
ModifyIDCMP
reversecamcompute()
reversefoccompute()
DrawInterFresh()
setcompass()
drawgridpts()
FetchEvent()
SetPointer
setclipbounds()
SetDrMd
SetWrMsk
ShowCenters_Map()
ClearPointer

Handle_InterWind2()=================================================
QuickFetchEvent()
closesharedwindow()
setclipbounds()
min
SetAPen
RectFill
DrawEllipse
compass()

OpenNewIAGridSize()=================================================
closeviewmaps()
interactiveview()

InteractiveDraw.c***************************************************

drawgridview()======================================================
SetPointer
BusyWin_New()
constructview()
computeview()
SetAPen
RectFill
vgl_makepixmap()
vgl_dumb_setcur()
vgl_dumb_clear()
ScratchRast_new()
drawinterview()
ScratchRast_CornerTurn()
WaitBOVP
BltBitMapRastPort
WaitBlit
ScratchRast_Del()
vgl_freepixmap()
BusyWin_Del()
ClearPointer

drawgridpts()========================================================
SetAPen
computeview()
setlineseg()
ClipDraw()

computeview()========================================================
computesinglebox()
getviewscrncoords()
CheckInput_ID()
BusyWin_Update()
SortAltRenderList()
findfocprof()

makeviewcenter()=====================================================
SetAPen
setlineseg()
ClipDraw()
getviewscrncoords()
abs
tan
sqrt

drawinterview()======================================================
SetAPen
memset
InitArea
AllocRaster
User_Message()
InitTmpRas
SetOPen
abs
vgl_dumb_setcur()
ClipAreaDrawVGL()
ClipAreaDrawRPort()
CheckInput_ID()
BusyWin_Update()
FreeRaster
BNDRYOFF()
setlineseg()
ClipDraw()
vgl_dumb_line()

computequick()======================================================
getviewscrncoords()

DrawInterFresh()====================================================
computequick()
SetAPen
RectFill
findfocprof()
drawgridpts()
drawquick()
drawfocprof()
drawveryquick()
makeviewcenter()

drawveryquick()=====================================================
SetAPen
setlineseg()
ClipDraw()
getviewscrncoords()

drawquick()=========================================================
SetAPen
RectFill
setlineseg()
ClipDraw()

constructview()=====================================================
initmopar()
sqrt
tan
cos
setquickview()

compass()===========================================================
sin
cos
SetAPen
Move
Draw

make_compass()======================================================
make_window()
WindowLimits
ModifyIDCMP
setclipbounds()
SetAPen
DrawEllipse

computesinglebox()==================================================
getviewscrncoords()

drawfocprof()=======================================================
SetAPen
computefocprof()
setlineseg
ClipDraw()

computefocprof()====================================================
getviewscrncoords()

ClipAreaDrawVGL()===================================================
vgl_dumb_setcur()
vgl_dumb_fillpoly_convex()
vgl_dumb_line()

ClipAreaDrawRPort()=================================================
AreaMove
AreaDraw
AreaEnd
ClipPoly4RPort()

ClipPoly4RPort()====================================================
setlineseg()
ClipPolySeg()
AreaMove
AreaDraw
AreaEnd

Play_Motion()=======================================================
setclipbounds()
SetAPen
RectFill
BuildKeyTable()
User_Message()
strcpy
BusyWin_New()
initmopar()
initpar()
setvalues()
sqrt
tan
cos
setquickview()
sprintf
computeview()
BusyWin_Del()
vgl_makepixmap()
vgl_dumb_setcur()
vgl_dumb_clear()
ScratchRast_new()
drawinterview()
ScratchRast_CornerTurn()
WaitBOVP
BltBitMapRastPort
WaitBlit
saveILBM()
ScratchRast_Del()
vgl_freepixmap()
drawgridpts()
findfocprof()
drawquick()
computequick()
drawfocprof()
drawveryquick()
makeviewcenter()
DoMethod()
CheckInput_ID()
Handle_EMIA_Window()
abs
BusyWin_Update()
FreeKeyTable()

InteractiveUtils.c***************************************************

SetIncrements()======================================================

autocenter()=========================================================
setcompass()
setclipbounds()
ShowView_Map()
sprintf
Log()

modifyinteractive()==================================================
Update_InterMap()

modifycampt()========================================================
reversecamcompute()
compass()
Update_InterMap()

modifyfocpt()========================================================
reversefoccompute()
compass
Update_InterMap()

Update_InterMap()====================================================
setclipbounds()
SetDrMd
SetWrMsk
ShowCenters_Map()

azimuthcompute()=====================================================
cos
sqrt
findangle2

reversecamcompute()==================================================
sin
cos
Update_InterMap()

reversefoccompute()==================================================
sin
cos
Update_InterMap()

setcompass()=========================================================
azimuthcompute()
compass()

findfocprof()========================================================
abs

Set_CompassBds()=====================================================
DisableBoundsButtons()
DrawInterFresh()

Set_LandBds()========================================================
DisableBoundsButtons()
DrawInterFresh()

Set_ProfileBds()=====================================================
DisableBoundsButtons()
DrawInterFresh()

Set_BoxBds()=========================================================
DisableBoundsButtons()
DrawInterFresh()

Set_GridBds()========================================================
DisableBoundsButtons()
DrawInterFresh()

DisableBoundsButtons()===============================================
DoMethod()

FixXYZ()=============================================================
ActivateWindow
DoMethod()

Init_IA_View()=======================================================
drawgridview()
constructview()
DrawInterFresh()

Init_IA_Item()=======================================================
drawgridview()
constructview()
DrawInterFresh()

CheckDEMStatus()=====================================================
strcmp

DataOps.c************************************************************

ConvertDEM()=========================================================
User_Message()
sprintf
User_Message_Def()
strlen
stricmp
strcpy
get_Memory()
LoadVistaDEM()
openbitmaps()
LoadImage()
closebitmaps()
free_Memory()
memmove
memcpy
fopen
open
lseek
read
close
fseek
fclose
readDEM()
fgetc
atoi
atof
BusyWin_New()
swmem
CheckInput_ID()
BusyWin_Update()
BusyWin_Del()
free_Memory()
SaveConvertOutput()
Log()
strcat

SaveConvertOutput()===================================================
strcpy
strlen
strcat
strmfp
open
swmem
FindElMaxMin()
write
close
strcmp
DataBase_Copy()
strncpy
Add_DE_NewItem()
memset
saveobject()
get_Memory()
saveILBM()
free_Memory()
SaveZBuf()

DLG.c*****************************************************************

ImportDLG()===========================================================
get_Memory()
User_Message()
strcpy
getfilename()
strmfp
fopen
Log()
fscanf
fseek
UTM_LatLon()
BusyWin_New()
GetInputString()
strncpy()
abs
AssignDLGAttrs()
strcmp
OpenObjFile()
DataBase_Copy()
strcat
saveobject()
Add_DE_NewItem()
ftell
CheckInput_ID()
BusyWin_Update()
fclose
BusyWin_Del()
free_Memory()

ImportDXF()=========================================================
get_Memory()
User_Message()
strcpy
getfilename()
strmfp
fopen
Log()
fseek
BusyWin_New()
fscanf
strcmp
Set_DXFObject()
Add_DE_NewItem()
strcat
saveobject()
CheckInput_ID()
BusyWin_Update()
OpenObjFile()
GetInputString()
BusyWin_Del()
fclose
free_Memory()

OpenObjFile()========================================================
strlen
strcat
strcmp

Set_DXFObject()======================================================
strncpy
strcpy

Set_DLGObject()======================================================
strncpy
strcpy
strcat

AssignDLGAttrs()=====================================================
strcpy
strcat

ImportWDB()==========================================================
get_Memory()
User_Message()
strcpy
getfilename()
strmfp
fopen
Log()
fseek
BusyWin_New()
fread
strcat
saveobject()
Add_DE_NewItem()
CheckInput_ID()
BusyWin_Update()
DataBase_Copy()
OpenObjFile()
Set_DLGObject()
fclose
free_Memory()
BusyWin_Del()

WCS.c*****************************************************************

main()================================================================
OpenLibrary
getcwd
memset
LoadProject()
strcpy
WCS_App_New()
DoMethod()
ModeList_New()
ModeList_Choose()
OpenScreenTags()
ModeList_Del()
DitherTable_New()
PubScreenStatus()
DirList_New()
WCS_App_Startup()
WCS_App_EventLoop()
WCS_App_Del()
SaveProject()
DirList_Del()
DitherTable_Del()
CloseScreen
CloseLibrary
printf
free_Memory
FreeKeyTable

ScratchPad.c*********************************************************

ScratchRast_CornerTurn()=============================================
HK4M()

ScratchRast_New()====================================================
AllocMem
InitRastPort
InitBitMap
AllocRaster
FreeRaster
FreeMem

ScratchRast_Del()====================================================
FreeRaster
FreeMem
