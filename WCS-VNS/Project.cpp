// Project.cpp
// Project code
// Written from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Project.h"
#include "ProjectIO.h"
#include "ProjectDispatch.h"
#include "Requester.h"
#include "Types.h"
#include "AppMem.h"
#include "Application.h"
#include "Log.h"
#include "Database.h"
#include "Notify.h"
#include "Useful.h"
#include "Fenetre.h"
#include "Toolbar.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "SceneViewGUI.h"
#include "ViewGUI.h"
#include "ImportThing.h"
#include "ImportWizGUI.h"
#include "FSSupport.h"
#include "WCSVersion.h"
#include "ImageOutputEvent.h"
#include "ProjectIODetail.h"
#include "ProjUpdateGUI.h"

#define WCS_PROJECT_WCSFRAMES "WCSFrames:"
#define WCS_PROJECT_WCSPROJECTS "WCSProjects:"
#define WCS_PROJECT_WCSCONTENT "WCSContent:"
#define WCS_PROJECT_MUNGTEMP_SIZE 16400

//#define DATABASE_CHUNK_DEBUG
#ifdef DATABASE_CHUNK_DEBUG
static char debugStr[256];
#endif // DATABASE_CHUNK_DEBUG

#ifdef WCS_BUILD_DEMO
// This is a scratch buffer for blitting.
// Thumbnails bigger than 2048 wide are not supported. Duh.
// Here we use it for temporary text message-formatting
extern unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];
#endif // WCS_BUILD_DEMO

#define SETVBUF_BUFFER_SIZE	32768
extern char OverridePrefsFile[512];

// implementations of PROJ_ functions for Windows APIs
//#define PROJ_fopen(a, b) fopen((a) && (a)[0] ? GlobalApp->MainProj->MungPath(a): " ", b)
//#define PROJ_remove(a) remove(GlobalApp->MainProj->MungPath(a))
//#define PROJ_chdir(a) chdir(GlobalApp->MainProj->MungPath(a))
//#define PROJ_rename(a, b) rename(GlobalApp->MainProj->MungPath(a), GlobalApp->MainProj->MungPath(b, 1))
FILE *PROJ_fopen(const char *filename, const char *mode)
{
return(fopen((filename) && (filename)[0] ? GlobalApp->MainProj->MungPath(filename): " ", mode));
} // PROJ_fopen

int PROJ_remove(const char *path)
{
return(remove(GlobalApp->MainProj->MungPath(path)));
} // PROJ_remove

int PROJ_rmdir(const char *path)
{
return(rmdir(GlobalApp->MainProj->MungPath(path)));
} // PROJ_rmdir

int PROJ_chdir(const char *dirname)
{
return(chdir(GlobalApp->MainProj->MungPath(dirname)));
} // PROJ_chdir

int PROJ_rename(const char *oldname, const char *newname)
{
return(rename(GlobalApp->MainProj->MungPath(oldname), GlobalApp->MainProj->MungPath(newname, 1)));
} // PROJ_rename

int PROJ_mkdir(const char *path)
{

// Windows only -> Creates all non-existent directories in path
return(SHCreateDirectoryEx(NULL, GlobalApp->MainProj->MungPath(path), NULL));
// Hmm. SHCreateDirectoryEx is WinMe/Win2k and newer only, and not included in the VC6 headers. Gotta go back for now...
//return(_mkdir(GlobalApp->MainProj->MungPath(path)));
} // PROJ_mkdir

/*===========================================================================*/
/*===========================================================================*/

// ("A1A2A3", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, .66, .35, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
int WinLayout::InitLayout(char *CellIDs, ...)
{
float SpanFrac, CellFrac, SpanOrig, TotalFrac;
int Direction, TotalCells, SpanCells, Terminated = 0;
va_list VarA;

va_start(VarA, CellIDs);

SpanOrig = 0.0f;
TotalCells = 0;
AvgSize = 0.0;
while (!Terminated)
	{
	if (Direction = va_arg(VarA, int))
		{
		TotalFrac = 0.0f;
		SpanCells = 0;
		SpanFrac = (float)va_arg(VarA, double);
		if (SpanOrig + SpanFrac > 1.0)
			{
			SpanFrac = 1.0f - SpanOrig;
			} // if
		if (SpanFrac > 0.0)
			{
			while ((CellFrac = (float)va_arg(VarA, double)) != 0.0)
				{
				if (TotalFrac + CellFrac > 1.0)
					{
					CellFrac = 1.0f - TotalFrac;
					} // if
				if (CellFrac > 0.0)
					{
					if (Direction == WCS_PROJECT_WINLAYOUT_DIRECTION_COL)
						{
						MPanes[TotalCells].OffsetXPct = SpanOrig;
						MPanes[TotalCells].OffsetYPct = TotalFrac;
						MPanes[TotalCells].WidthPct   = SpanFrac;
						MPanes[TotalCells].HeightPct  = CellFrac;
						} // if
					else
						{
						MPanes[TotalCells].OffsetYPct = SpanOrig;
						MPanes[TotalCells].OffsetXPct = TotalFrac;
						MPanes[TotalCells].HeightPct  = SpanFrac;
						MPanes[TotalCells].WidthPct   = CellFrac;
						} // else
					AvgSize += (MPanes[TotalCells].AreaFrac = CellFrac * SpanFrac);
					memcpy(MPanes[TotalCells].PaneID, CellIDs, 2);
					MPanes[TotalCells].PaneID[2] = NULL;
					CellIDs += 2; // advance 2 letters
					TotalFrac += CellFrac;
					TotalCells++;
					SpanCells++;
					} // if
				} // while
			SpanOrig += SpanFrac;
			} // if
		} // if
	else
		{
		Terminated = 1;
		break;
		} // else
	} // while

AvgSize = AvgSize / TotalCells;
return(NumWindows = TotalCells);
} // WinLayout::InitLayout

/*===========================================================================*/

int WinLayout::PaneFromID(char *SearchID)
{
int SearchIdx;

for (SearchIdx = 0; SearchIdx < NumWindows; SearchIdx++)
	{
	if (MPanes[SearchIdx].PaneID[0] == SearchID[0] && MPanes[SearchIdx].PaneID[1] == SearchID[1])
		{
		return(SearchIdx);
		} // if
	} // for
return(-1);
} // WinLayout::PaneFromID

/*===========================================================================*/

int WinLayout::PaneFromXY(short Xc, short Yc)
{
int SearchIdx;

for (SearchIdx = 0; SearchIdx < NumWindows; SearchIdx++)
	{
	// OffsetX, OffsetY, Width, Height
	if (Xc > MPanes[SearchIdx].OffsetX &&
	 Xc < MPanes[SearchIdx].OffsetX + MPanes[SearchIdx].Width &&
	 Yc > MPanes[SearchIdx].OffsetY &&
	 Yc < MPanes[SearchIdx].OffsetY + MPanes[SearchIdx].Height)
		{
		return(SearchIdx);
		} // if
	} // for
return(-1);
} // WinLayout::PaneFromXY

/*===========================================================================*/
/*===========================================================================*/

MatrixLayouts::MatrixLayouts()
{
// A: 1/1 Fullscreen
// B: 2/3 Twothirds
// C: 1/2 Half, (also 4/9)
// D: 1/3 Third
// E: 1/4 Quarter (also 2/9)
// F: 1/6 Sixth
// G: 1/9 Ninth

// WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN
// WCS_PROJECT_WINLAYOUT_DIRECTION_END
// WCS_PROJECT_WINLAYOUT_DIRECTION_COL
// WCS_PROJECT_WINLAYOUT_DIRECTION_ROW

memset(LayoutFlags, 0, sizeof(LayoutFlags));

// ("A1A2A3", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, .66, .35, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[0].InitLayout("a0", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, 1.0, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[1].InitLayout("c0c1", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[2].InitLayout("c0c1", WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[3].InitLayout("c0e1e3", WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);

Matrices[4].InitLayout("c0e2e3", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[5].InitLayout("e0e1e2e3", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[6].InitLayout("e0e1f0f1f2", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .3333, .3333, .34, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[7].InitLayout("d0d1g0g1g2", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .6666, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .3333, .3333, .3333, .34, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);

Matrices[8].InitLayout("b0g0g1g2", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .6666, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .3333, .3333, .3333, .34, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[9].InitLayout("d0f0e2e3", WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .6666, .34, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_COL, .5, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[10].InitLayout("b0f0f1", WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .6666, 1.0, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .34, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
Matrices[11].InitLayout("c0e0f0f1", WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .6666, .6666, .34, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_ROW, .34, .5, .5, WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN, WCS_PROJECT_WINLAYOUT_DIRECTION_END);
} // MatrixLayouts::MatrixLayouts

/*===========================================================================*/

unsigned long MatrixLayouts::TestFlag(char *CellCode, unsigned long FlagToTest)
{
return(LayoutFlags[toupper(CellCode[0]) - 'A'][CellCode[1] - '0'] & FlagToTest);
} // MatrixLayouts::TestFlag

/*===========================================================================*/

void MatrixLayouts::SetFlag(const char *CellCode, unsigned long FlagToSet)
{
LayoutFlags[toupper(CellCode[0]) - 'A'][CellCode[1] - '0'] |= FlagToSet;
} // MatrixLayouts::SetFlag

/*===========================================================================*/

void MatrixLayouts::ClearFlag(const char *CellCode, unsigned long FlagToClear)
{
LayoutFlags[toupper(CellCode[0]) - 'A'][CellCode[1] - '0'] &= ~FlagToClear;
} // MatrixLayouts::ClearFlag

/*===========================================================================*/

int MatrixLayouts::GetPaneFromID(int Layout, char *PaneID)
{
if (Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT)
	{
	return(Current.PaneFromID(PaneID));
	} // if
else
	{
	return(Matrices[Layout].PaneFromID(PaneID));
	} // else
} // MatrixLayouts::GetPaneFromID

/*===========================================================================*/

int MatrixLayouts::GetPaneFromXY(short Xc, short Yc)
{
return(Current.PaneFromXY(Xc, Yc));
} // MatrixLayouts::GetPaneFromXY

/*===========================================================================*/

void MatrixLayouts::SetCurrent(int Layout)
{
int i;
double MatrixWidth, MatrixHeight;
short MatrixOriginX, MatrixOriginY;


if (Layout != -1)
	{
	Current = Matrices[Layout];
	} // if

if (GlobalApp->MainProj)
	{
	MatrixOriginX = (short)GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
	MatrixOriginY = (short)GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);

	MatrixWidth   = (double)GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
	MatrixHeight  = (double)GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
	} // if
for (i = 0; i < Current.NumWindows; i++)
	{
	// only clear flags on matrix change
	if (Layout != -1)
		{
		if (isupper((unsigned char)Current.MPanes[i].PaneID[0]))
			{
			SetFlag(Current.MPanes[i].PaneID, WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
			} // if
		else
			{
			ClearFlag(Current.MPanes[i].PaneID, WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
			} // if
		} // if

	// Calculate pixel coords from percentages
	if (GlobalApp->MainProj)
		{
		Current.MPanes[i].OffsetX   = MatrixOriginX + (short)((MatrixWidth * Current.MPanes[i].OffsetXPct) + .5);		// round up
		Current.MPanes[i].OffsetY   = MatrixOriginY + (short)((MatrixHeight * Current.MPanes[i].OffsetYPct) + .5);		// round up
		Current.MPanes[i].Width  = (short)(MatrixWidth * Current.MPanes[i].WidthPct);		// round down
		Current.MPanes[i].Height = (short)(MatrixHeight * Current.MPanes[i].HeightPct);		// round down
		} // if
	} // for

} // MatrixLayouts::SetCurrent

/*===========================================================================*/
/*===========================================================================*/

void Project::Relayout(void)
{
// Refresh pixel coords
ViewPorts.SetCurrent(-1);

InvalidateRect(GlobalApp->WinSys->RootWin, NULL, 1);

} // Project::Relayout

/*===========================================================================*/

int Project::FindWinID(unsigned long WinID)
{
int Search = -1;

if (FenTrack && FenTrackSize)
	{
	for (Search = 0; Search < FenTrackSize; Search++)
		{
		if (FenTrack[Search].WinID == 0 || FenTrack[Search].WinID == WinID)
			{
			break;
			} // if
		} // for
	} // if

return (Search);

} // Project::FindWinID

/*===========================================================================*/

void Project::InquireWindowCoords(unsigned long WinID, short &X,
 short &Y, short &W, short &H)
{
int Search;

if (FenTrack && FenTrackSize)
	{
	for (Search = 0; Search < FenTrackSize; Search++)
		{
		if (FenTrack[Search].WinID == 0)
			{
			break;
			} // if
		if (FenTrack[Search].WinID == WinID)
			{
			X = FenTrack[Search].X;
			if ((GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("ForceWinPositive")) && (X < 0))
				{
				X = 6;
				} // if
			Y = FenTrack[Search].Y;
			W = FenTrack[Search].W;
			H = FenTrack[Search].H;
			return;
/*
			#ifdef _WIN32
			WinBound.left = FenTrack[Search].X;
			if (FenTrack[Search].Y > 1)
				{
				WinBound.top = FenTrack[Search].Y;
				} // if
			else
				{
				WinBound.top = 2;
				} // else
			WinBound.right = FenTrack[Search].W + FenTrack[Search].X;
			WinBound.bottom = FenTrack[Search].H + FenTrack[Search].Y;
			AdjustWindowRectEx(&WinBound, WS_OVERLAPPEDWINDOW, NULL, WS_EX_TOOLWINDOW);
			X = (short)WinBound.left;
			Y = (short)WinBound.top;
			W = (short)(WinBound.right - WinBound.left);
			H = (short)(WinBound.bottom - WinBound.top);
			return;
			#endif // _WIN32
*/
			} // if
		} // for
	} // if

X = 6;
Y = 44 + (WCS_TOOLBAR_VMARGIN * 4);
W = 500;
H = 300;

return;

} // Project::InquireWindowCoords

/*===========================================================================*/

void Project::SetWindowCoords(unsigned long WinID, short X,
	short Y, short W, short H, ProjWinInfo **FTPtr, unsigned short *FTSizePtr)
{
struct ProjWinInfo *NewTrack, *FT = NULL;
int Search, OldTop = 0, gsmx, gsmy;
unsigned short FTSize;

if (! FTPtr)
	FTPtr = &FenTrack;
if (! FTSizePtr)
	FTSizePtr = &FenTrackSize;

FT = *FTPtr;
FTSize = *FTSizePtr;

//#ifdef WCS_BUILD_FRANK
//char swcdebugmsg[256];
//sprintf(swcdebugmsg, "SetWindowCoords: WinID = %x, X = %d, Y = %d, W = %d, H = %d\n", WinID, X, Y, W, H);
//OutputDebugString(swcdebugmsg);
//#endif // WCS_BUILD_FRANK

gsmx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
gsmy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
assert(!(X > gsmx));
assert(!(Y > gsmy));
if (W > 0)
	{
	assert(!(X < -W));
	assert(!(Y < -H));
	} // if

if (FT && FTSize)
	{
	// Look for an entry with our WinID, and fill in numbers
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == WinID)
			{
			if (Y > 0)
				{
				FT[Search].X = X;
				FT[Search].Y = Y;
				} // if
			if ((W >0) && (H > 0))
				{
				FT[Search].W = W;
				FT[Search].H = H;
				} // if
			return;
			} // if
		} // for
	// Didn't find one, look for an empty one
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == NULL)
			{
			FT[Search].WinID = WinID;
			FT[Search].X = X;
			FT[Search].Y = Y;
			if ((W > 0) && (H > 0))
				{
				FT[Search].W = W;
				FT[Search].H = H;
				} // if
			return;
			} // if
		} // for
	// Didn't find any empty ones either, need to realloc
	OldTop = FTSize;
	} // if

// This will work even if there no FenTrack to start
if (NewTrack = (struct ProjWinInfo *)realloc(FT, (sizeof(struct ProjWinInfo) * (OldTop + WCS_PROJECT_INIT_FENTRACK_INC))))
	{
	FT = NewTrack;
	*FTPtr = FT;
	FTSize += WCS_PROJECT_INIT_FENTRACK_INC;
	*FTSizePtr = FTSize;
	memset(&FT[OldTop], 0, WCS_PROJECT_INIT_FENTRACK_INC * sizeof(struct ProjWinInfo));
	// We happen to know just where an empty slot will be.
	FT[OldTop].WinID = WinID;
	FT[OldTop].X = X;
	FT[OldTop].Y = Y;
	if ((W >0) && (H > 0))
		{
		FT[OldTop].W = W;
		FT[OldTop].H = H;
		} // if
	} // if

return;

} // Project::SetWindowCoords

/*===========================================================================*/

void Project::SetWindowCell(unsigned long WinID, char *WinCell, short X,
 	short Y, short W, short H, ProjWinInfo **FTPtr, unsigned short *FTSizePtr)
{
struct ProjWinInfo *NewTrack, *FT = NULL;
int Search, OldTop = 0;
unsigned short FTSize;

if (! FTPtr)
	FTPtr = &FenTrack;
if (! FTSizePtr)
	FTSizePtr = &FenTrackSize;

FT = *FTPtr;
FTSize = *FTSizePtr;

if (FT && FTSize)
	{
	// Look for an entry with our WinID, and fill in numbers
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == WinID)
			{
			memcpy(FT[Search].PreferredCell, WinCell, 3);
			FT[Search].PreferredCell[2] = NULL;
			if (Y > 0)
				{
				FT[Search].X = X;
				FT[Search].Y = Y;
				} // if
			if ((W >0) && (H > 0))
				{
				FT[Search].W = W;
				FT[Search].H = H;
				} // if
			return;
			} // if
		} // for
	// Didn't find one, look for an empty one
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == NULL)
			{
			FT[Search].WinID = WinID;
			FT[Search].X = X;
			FT[Search].Y = Y;
			memcpy(FT[Search].PreferredCell, WinCell, 3);
			FT[Search].PreferredCell[2] = NULL;
			if ((W > 0) && (H > 0))
				{
				FT[Search].W = W;
				FT[Search].H = H;
				} // if
			return;
			} // if
		} // for
	// Didn't find any empty ones either, need to realloc
	OldTop = FTSize;
	} // if

// This will work even if there no FenTrack to start
if (NewTrack = (struct ProjWinInfo *)realloc(FT, (sizeof(struct ProjWinInfo) * (OldTop + WCS_PROJECT_INIT_FENTRACK_INC))))
	{
	FT = NewTrack;
	*FTPtr = FT;
	FTSize += WCS_PROJECT_INIT_FENTRACK_INC;
	*FTSizePtr = FTSize;
	memset(&FT[OldTop], 0, WCS_PROJECT_INIT_FENTRACK_INC * sizeof(struct ProjWinInfo));
	// We happen to know just where an empty slot will be.
	FT[OldTop].WinID = WinID;
	memcpy(FT[OldTop].PreferredCell, WinCell, 3);
	FT[OldTop].PreferredCell[2] = NULL;
	FT[OldTop].X = X;
	FT[OldTop].Y = Y;
	if ((W >0) && (H > 0))
		{
		FT[OldTop].W = W;
		FT[OldTop].H = H;
		} // if
	} // if

return;

} // Project::SetWindowCell

/*===========================================================================*/

void Project::InquireWindowCell(unsigned long WinID, char *WinCell)
{
int Search;

if (FenTrack && FenTrackSize)
	{
	for (Search = 0; Search < FenTrackSize; Search++)
		{
		if (FenTrack[Search].WinID == 0)
			{
			break;
			} // if
		if (FenTrack[Search].WinID == WinID)
			{
			memcpy(WinCell, FenTrack[Search].PreferredCell, 2);
			WinCell[2] = NULL;
			return;
			} // if
		} // for
	} // if

WinCell[0] = WinCell[1] = WinCell[2] = NULL;

return;

} // Project::InquireWindowCell

/*===========================================================================*/

void Project::InquireWindowFlags(unsigned long WinID, unsigned long &Flags)
{
int Search;

if (FenTrack && FenTrackSize)
	{
	for (Search = 0; Search < FenTrackSize; Search++)
		{
		if (FenTrack[Search].WinID == 0)
			{
			break;
			} // if
		if (FenTrack[Search].WinID == WinID)
			{
			Flags = FenTrack[Search].Flags;
			return;
			} // if
		} // for
	} // if

Flags = 0;

return;

} // Project::InquireWindowFlags

/*===========================================================================*/

void Project::SetWindowFlags(unsigned long WinID, unsigned long Flags, short Operation,
	ProjWinInfo **FTPtr, unsigned short *FTSizePtr)
{
int Search, OldTop = 0;
struct ProjWinInfo *NewTrack, *FT = NULL;
unsigned short FTSize;

if (! FTPtr)
	FTPtr = &FenTrack;
if (! FTSizePtr)
	FTSizePtr = &FenTrackSize;

FT = *FTPtr;
FTSize = *FTSizePtr;

if (FT && FTSize)
	{
	// Look for an entry with our WinID, and fill in numbers
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == WinID)
			{
			if (Operation == WCS_FENTRACK_FLAGS_ENABLE)
				FT[Search].Flags |= Flags;
			else if (Operation == WCS_FENTRACK_FLAGS_DISABLE)
				FT[Search].Flags &= ~(Flags);
			else 
				FT[Search].Flags = Flags;
			return;
			} // if
		} // for
	// Didn't find one, look for an empty one
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == NULL)
			{
			FT[Search].WinID = WinID;
			if (Operation == WCS_FENTRACK_FLAGS_ENABLE)
				FT[Search].Flags |= Flags;
			else if (Operation == WCS_FENTRACK_FLAGS_DISABLE)
				FT[Search].Flags &= ~(Flags);
			else 
				FT[Search].Flags = Flags;
			return;
			} // if
		} // for
	// Didn't find any empty ones either, need to realloc
	OldTop = FTSize;
	} // if

// This will work even if there no FenTrack to start
if (NewTrack = (struct ProjWinInfo *)realloc(FT, (sizeof(struct ProjWinInfo) * (OldTop + WCS_PROJECT_INIT_FENTRACK_INC))))
	{
	FT = NewTrack;
	*FTPtr = FT;
	FTSize += WCS_PROJECT_INIT_FENTRACK_INC;
	*FTSizePtr = FTSize;
	memset(&FT[OldTop], 0, WCS_PROJECT_INIT_FENTRACK_INC * sizeof(struct ProjWinInfo));
	// We happen to know just where an empty slot will be.
	FT[OldTop].WinID = WinID;
	if (Operation == WCS_FENTRACK_FLAGS_ENABLE)
		FT[Search].Flags |= Flags;
	else if (Operation == WCS_FENTRACK_FLAGS_DISABLE)
		FT[Search].Flags ^= Flags;
	else 
		FT[Search].Flags = Flags;
	} // if

return;

} // Project::SetWindowFlags

/*===========================================================================*/
/*===========================================================================*/

ProjPrefsInfo::ProjPrefsInfo()
{
int ConfigOptClear, Ct;

RenderTaskPri = 0;
RenderSize = 0; // -1=size/2, -2=size/4
LoadOnOpen = 0;
OpenWindows = 1;

//ReportMesg[0] = ReportMesg[1] = ReportMesg[2] = ReportMesg[3] = 1;
ProjShowIconTools = ProjShowAnimTools = 1;

PosLonHemisphere = WCS_PROJPREFS_LONCONVENTION_POSWEST;
VertDisplayUnits = WCS_USEFUL_UNIT_METER;
HorDisplayUnits = WCS_USEFUL_UNIT_METER;
AngleDisplayUnits = WCS_PROJPREFS_ANGLEUNITS_DECDEG;
TimeDisplayUnits = WCS_PROJPREFS_TIMEUNITS_SECS;
LatLonSignDisplay = WCS_PROJPREFS_LATLONSIGN_NUMERIC;
SignificantDigits = 8;
LastUTMZone = 10;
DisplayGeoUnitsProjected = 0;
MultiUserMode = 0;
CurrentUserName[0] = CurrentPassword[0] = LastColorSwatch[0] = 0;
NewProjUseTemplates = 0;
GlobalStartupPage = 0;
// RecordMode is currently unused and may be retired eventually
TaskMode = EnabledFilter = AnimatedFilter = ShowDBbyLayer = RecordMode = InteractiveMode = KeyGroupMode = 0;
MaxSAGDBEntries = 6000;
MaxSortedSAGDBEntries = 3000;
GUIConfiguration = 5;
SAGExpanded = 150;
MatrixTaskModeEnabled = 1;
SAGBottomHtPct = 3300;
InteractiveStyle = WCS_INTERACTIVE_STYLE_LIGHTWAVE;
LastUpdateDate = 0;
PaintDefaultsValid = 0;
PaintMode = GradMode = ActiveBrush = Effect = 0;
Tolerance = ForeElev = BackElev = 0.0;
BrushScale = Opacity = 0.0f;
MemoryLimitsEnabled = 1;
GlobalAdvancedEnabled = 0;
VecPolyMemoryLimit = 500;
DEMMemoryLimit = 200;
DBENumFixedColumns = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX; 
for (Ct = 0; Ct < DBENumFixedColumns; ++Ct)
	DBEFixedColumnWidths[Ct] = 0;
DBENumLayerColumns = DBENumAttribColumns = 0;
for (Ct = 0; Ct < WCS_DBEDITGUI_MAX_GRID_COLUMNS; ++Ct)
	{
	DBELayerColumnWidths[Ct] = 0;
	DBEAttribColumnWidths[Ct] = 0;
	} // for
DBEFilterMode = 0;
DBESearchQueryFilterName[0] = DBELayerFilterName[0] = 0;

for (ConfigOptClear = 0; ConfigOptClear < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigOptClear++)
	{
	ConfigOptions[ConfigOptClear][0] = NULL;
	} // for

for (Ct = 0; Ct < WCS_VIEWGUI_VIEWTYPE_MAX; Ct ++)
	{
	for (ConfigOptClear = 0; ConfigOptClear < WCS_VIEWGUI_ENABLE_MAX; ConfigOptClear++)
		{
		ViewEnabled[Ct][ConfigOptClear] = 1;
		} // for
	ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_TERRAIN_TRANS] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_DEMEDGE] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_RENDERPREV] = 
		//ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_VIEW_PLUS_TASK] = 
		//ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_REND_PLUS_TASK] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_GRADREPEAT] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_GRAD_GREY] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_GRAD_PRIMARY] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_CONTOURS] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_SLOPE] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_RELEL] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_FRACTAL] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_ECOSYS] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_OVER_RENDER] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_POLYEDGE] = 
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_LTDREGION] = 0;
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_SAFETITLE] = 0;
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_SAFEACTION] = 0;
	if (Ct > 0)
		ViewEnabled[Ct][WCS_VIEWGUI_ENABLE_ATMOS] = 0;
	ViewContOverlayInt[Ct] = 10.0;
	} // for

} // ProjPrefsInfo::ProjPrefsInfo

/*===========================================================================*/

int ProjPrefsInfo::PublishConfigOpt(char *CO)
{
int Status = 0;
int ConfigLoop;

if (QueryConfigOpt(CO)) return(1); // no need

for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (ConfigOptions[ConfigLoop][0] == NULL)
		{
		strcpy(ConfigOptions[ConfigLoop], CO);
		return(Status = 1);
		} // if
	} // for

return(Status);
} // ProjPrefsInfo::PublishConfigOpt

/*===========================================================================*/

int ProjPrefsInfo::SetConfigOpt(char *CO, char *Val)
{
char *Result = NULL;
int ConfigLoop;

// look for our slot
for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (ConfigOptions[ConfigLoop][0] != NULL)
		{
		if (!strnicmp(ConfigOptions[ConfigLoop], CO, strlen(CO))) // compare w/o data portion
			{
			sprintf(ConfigOptions[ConfigLoop], "%s=%s", CO, Val);
			return(1);
			} // if
		} // if
	} // for

// look for empty slot
for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (ConfigOptions[ConfigLoop][0] == NULL)
		{
		sprintf(ConfigOptions[ConfigLoop], "%s=%s", CO, Val);
		return(1);
		} // if
	} // for

return(0);
} // ProjPrefsInfo::SetConfigOpt

/*===========================================================================*/

char *ProjPrefsInfo::QueryConfigOpt(char *CO)
{
char *Result = NULL, *ConfigStr;
int ConfigLoop, ArgLoop;

for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (ConfigOptions[ConfigLoop][0] != NULL)
		{
		if (!strnicmp(ConfigOptions[ConfigLoop], CO, strlen(CO))) // compare w/o data portion
			{
			ConfigStr = ConfigOptions[ConfigLoop];
			for (ArgLoop = 0; ConfigStr[ArgLoop]; ArgLoop++)
				{
				if ((ConfigStr[ArgLoop] == '=') || (ConfigStr[ArgLoop] == ':'))
					{
					Result = &ConfigStr[ArgLoop + 1];
					return(Result);
					} // if
				} // for
			Result = &ConfigStr[ArgLoop]; // point to NULL at end of string for safe non-zero return
			return(Result);
			} // if
		} // if
	} // for

return(Result);
} // ProjPrefsInfo::QueryConfigOpt

/*===========================================================================*/

bool ProjPrefsInfo::QueryConfigOptTrue(char *CO)
{
char *QResult = NULL;
bool TruthOrFiction = false;

if (QResult = QueryConfigOpt(CO))
	{
	if (tolower(QResult[0]) == 't')
		{
		TruthOrFiction = true;
		} // 
	if (tolower(QResult[0]) == 'y')
		{
		TruthOrFiction = true;
		} // 
	if (QResult[0] == '1')
		{
		TruthOrFiction = true;
		} // 
	if (!stricmp(QResult, "true"))
		{
		TruthOrFiction = true;
		} // 
	} // if

return (TruthOrFiction);

} // ProjPrefsInfo::QueryConfigOptTrue

/*===========================================================================*/

bool ProjPrefsInfo::QueryConfigOptFalse(char *CO)
{
char *QResult = NULL;
bool TruthOrFiction = false;

if (QResult = QueryConfigOpt(CO))
	{
	if (tolower(QResult[0]) == 'f')
		{
		TruthOrFiction = true;
		} // 
	if (tolower(QResult[0]) == 'n')
		{
		TruthOrFiction = true;
		} // 
	if (QResult[0] == '0')
		{
		TruthOrFiction = true;
		} // 
	if (!stricmp(QResult, "false"))
		{
		TruthOrFiction = true;
		} // 
	} // if

return (TruthOrFiction);

} // ProjPrefsInfo::QueryConfigOptFalse

/*===========================================================================*/

void ProjPrefsInfo::RemoveConfigOpt(char *CO)
{
int ConfigLoop;

for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (ConfigOptions[ConfigLoop][0] != NULL)
		{
		if (!strnicmp(ConfigOptions[ConfigLoop], CO, strlen(CO))) // ignore any data value
			{
			ConfigOptions[ConfigLoop][0] = NULL;
			return;
			} // if
		} // if
	} // for
} // ProjPrefsInfo::RemoveConfigOpt

/*===========================================================================*/
/*===========================================================================*/

VectorExportData::VectorExportData()
{

HorScale = 1.0;
VertScale = 1000.0;
HorSize = 4.0;
VertSize = 1.0;
HorGridInterval = 5.0;
VertGridInterval = 100.0;
HorTicInterval = 1.0;
VertTicInterval = 20.0;
HorLabelInterval = 10.0;
VertLabelInterval = 5000.0;
VectorLineWeight = 2.0;
TerrainLineWeight = 1.0;
HorGridWeight = .5;
VertGridWeight = .5;
HorTicWeight = .5;
VertTicWeight = .5;
GraphOutlineWeight = 1.0;
HorTicLength = 4.0;
VertTicLength = 4.0;
HorUnits = 0;
VertUnits = 3;
LayoutUnits = 0;
HorScaleLabel = 1;
VertScaleLabel = 1;
DrawVector = 1;
DrawTerrain = 0;
DrawHorGrid = 1;
DrawVertGrid = 1;
DrawHorTics = 1;
DrawVertTics = 1;
DrawHorLabels = 1;
DrawVertLabels = 1;
DrawGraphOutline = 1;
VectorLineStyle = 0;
TerrainLineStyle = 1;
HorGridStyle = 2;
VertGridStyle = 2;
GraphOutlineStyle = 0;
LaunchIllustrator = 0;
VectorColor[2] = VectorColor[1] = VectorColor[0] = 0;
TerrainColor[2] = TerrainColor[1] = TerrainColor[0] = 128;
HorGridColor[2] = HorGridColor[1] = HorGridColor[0] = 64;
VertGridColor[2] = VertGridColor[1] = VertGridColor[0] = 64;
HorTicColor[2] = HorTicColor[1] = HorTicColor[0] = 0;
VertTicColor[2] = VertTicColor[1] = VertTicColor[0] = 0;
HorLabelColor[2] = HorLabelColor[1] = HorLabelColor[0] = 0;
VertLabelColor[2] = VertLabelColor[1] = VertLabelColor[0] = 0;
GraphOutlineColor[2] = GraphOutlineColor[1] = GraphOutlineColor[0] = 0;
strcpy(PreferredFont, "ArialMT");

} // VectorExportData::VectorExportData

/*===========================================================================*/
/*===========================================================================*/

Project::Project()
{
short Ct;

DL = NULL;
Interactive = NULL;

memset(RootMarkers, 0, sizeof(RootMarkers));

paramfile[0] = framepath[0] = temppath[0] = dbasepath[0] = parampath[0] =
		path[0] = dirname[0] = dbasename[0] =
		backgroundpath[0] = backgroundfile[0] =
		colormappath[0] = colormapfile[0] = projectpath[0] =
		projectname[0] = framefile[0] = 
		cloudpath[0] = cloudfile[0] = wavepath[0] = wavefile[0] =
		deformpath[0] = deformfile[0] = imagepath[0] = sunfile[0] = moonfile[0]
		= sunpath[0] = moonpath[0] = pcprojectpath[0] = pcframespath[0] = altobjectpath[0] = 
		animpath[0] = animfile[0] = savezbufpath[0] = savezbuffile[0] = contentpath[0] = importdatapath[0] =
		LastProject[0][0] = LastProject[1][0] = LastProject[2][0] = LastProject[3][0] = LastProject[4][0] = LastProject[5][0] = 
		UserName[0] = UserEmail[0] = ProjectPassword[0] = AuxImportPath[0] = AuxImportFile[0] = NULL;
ProjectLoaded = 0;
PrefsLoaded = 0;

ParamData[WCS_PARAMETERS_GRIDSIZE] = 10;
ParamData[WCS_PARAMETERS_SENSITIVITY] = 10;
ParamData[WCS_PARAMETERS_GRIDSTYLE] = 0;
ParamData[WCS_PARAMETERS_MOVEMENT] = 0;
ParamData[WCS_PARAMETERS_GBDENS] = 1;
ParamData[WCS_PARAMETERS_AUTODRAW] = 0;
ParamData[WCS_PARAMETERS_ANIMSTART] = 0;
ParamData[WCS_PARAMETERS_ANIMEND] = 0;
ParamData[WCS_PARAMETERS_ANIMSTEP] = 0;
ParamData[WCS_PARAMETERS_COMPASSBDS] = 1;
ParamData[WCS_PARAMETERS_LANDBDS] = 1;
ParamData[WCS_PARAMETERS_BOXBDS] = 0;
ParamData[WCS_PARAMETERS_PROFBDS] = 0;
ParamData[WCS_PARAMETERS_GRIDBDS] = 0;

FenTrackSize = 0;
// Allocate WinInfo array. Must use calloc to use realloc later.
if (FenTrack = (struct ProjWinInfo *)calloc(WCS_PROJECT_INIT_FENTRACK_SIZE, sizeof(struct ProjWinInfo)))
	{
	FenTrackSize = WCS_PROJECT_INIT_FENTRACK_SIZE;
	memset(FenTrack, 0, (WCS_PROJECT_INIT_FENTRACK_SIZE * sizeof(struct ProjWinInfo)));
	} // if
for (Ct = 0; Ct < WCS_MAX_USER_WINDOWCONFIG; Ct ++)
	{
	AltFenTrackSize[Ct] = 0;
	if (AltFenTrack[Ct] = (struct ProjWinInfo *)calloc(WCS_PROJECT_INIT_FENTRACK_SIZE, sizeof(struct ProjWinInfo)))
		{
		AltFenTrackSize[Ct] = WCS_PROJECT_INIT_FENTRACK_SIZE;
		memset(AltFenTrack[Ct], 0, (WCS_PROJECT_INIT_FENTRACK_SIZE * sizeof(struct ProjWinInfo)));
		} // if
	} // for

Imports = NULL;

} // Project::Project

/*===========================================================================*/

Project::~Project()
{
short Ct;

if (DL)
	{
   	DirList_Del(DL);
	DL = NULL;
	} // if

ClearPiers();

for (Ct = 0; Ct < WCS_MAX_USER_WINDOWCONFIG; Ct ++)
	{
	if (AltFenTrack[Ct])
		{
		free(AltFenTrack[Ct]);
		AltFenTrack[Ct] = NULL;
		} // if
	} // for


if (FenTrack)
	{
	free(FenTrack);
	FenTrack = NULL;
	} // if

if (Interactive)
	delete Interactive;
Interactive = NULL;

} // Project::~Project

/*===========================================================================*/

void Project::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = GetRAHostName();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = 0;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	//GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	//GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = projectpath[0] ? projectpath: (char*)"WCSProjects:";
	Prop->Ext = "proj";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	Prop->InterFlags = 0;
	} // if

} // Project::GetRAHostProperties

/*===========================================================================*/

int Project::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	if (Prop->Name && Prop->Name[0] && Prop->Name[0] != '.')
		{
		strncpy(projectname, Prop->Name, 64);
		projectname[63] = 0;
		if (strlen(projectname) < 5 || stricmp(&projectname[strlen(projectname) - 5], ".proj"))
			{
			projectname[58] = 0;
			strcat(projectname, ".proj");
			} // if need extension
		if (Prop->Path && Prop->Path[0])
			{
			strncpy(projectpath, Prop->Path, 255);
			projectpath[255] = 0;
			} // if
		Success = 1;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	//if (Success = ScaleDeleteAnimNodes(Prop))
	//	{
	//	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	//	Changes[1] = NULL;
	//	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	//	} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(Load(Prop, Prop->Path, GlobalApp->AppDB, GlobalApp->AppEffects, 
		GlobalApp->AppImages, NULL, 0xffffffff));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(Save(Prop, Prop->Path, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL, 0xffffffff));
	} // if

return (Success);

} // Project::SetRAHostProperties

/*===========================================================================*/

unsigned long Project::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
//if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
//	{
//	if (GetRAHostAnimated())
//		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
//	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	} // if

Mask &= (Flags);

return (Mask);

} // Project::GetRAFlags

/*===========================================================================*/

short Project::LoadPrefs(char *ProgDir, char *PreferredFile)
{
char filename[256], *RealFileName, TryWCSPrefs = 0;

TryAgain:

if (TryWCSPrefs)
	RealFileName = WCS_PROJECT_PREFS_NAME_WCS;
else
	RealFileName = PreferredFile ? PreferredFile: (char*)WCS_PROJECT_PREFS_NAME;

// support user-supplied prefs filename on command-line
if (OverridePrefsFile[0])
	{
	RealFileName = OverridePrefsFile;
	} // if

strmfp(filename, ProgDir, RealFileName);

if (Load(NULL, filename, GlobalApp->AppDB, GlobalApp->AppEffects, NULL,	// Images needs to be NULL so that they aren't created while loading prefs
	IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Prefs",
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Config",
		WCS_PROJECT_IODETAILTAG_CHUNKID, "UsrConfg",
		WCS_PROJECT_IODETAILTAG_DONE)))
	{
	if (Prefs.LoadOnOpen)
		{
		strmfp(filename, projectpath, projectname);
		Load(NULL, filename, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL, 0xffffffff);
		} // if
	return (1);
	} // if
else if (PreferredFile)
	{
	PreferredFile = NULL;
	goto TryAgain;
	} // else
#ifdef WCS_BUILD_VNS
else if (! TryWCSPrefs)
	{
	TryWCSPrefs = 1;
	goto TryAgain;
	} // else if
#endif // WCS_BUILD_VNS
else
	{
	strmfp(pcprojectpath, ProgDir, "WCSProjects");
	strmfp(pcframespath, ProgDir, "WCSFrames");
	strmfp(contentpath, ProgDir, "Components");
	strcpy(projectpath, "WCSProjects:");
	} // else

return (0);

} // Project::LoadPrefs()

/*===========================================================================*/

short Project::LoadResumeState(char *ProgDir)
{
int Attempt = 0;
char filename[256], PrefsName[256];

TryAgain:

if (Prefs.CurrentUserName[0] && Attempt == 0)
	{
	strcpy(PrefsName, Prefs.CurrentUserName);
	strcat(PrefsName, ".prefs");
	} // if
else if (Attempt < 2)
	{
	strcpy(PrefsName, WCS_PROJECT_PREFS_NAME);
	if (OverridePrefsFile[0]) // support user-supplied command-line arg for prefs file
		{
		strcpy(PrefsName, OverridePrefsFile);
		} // if
	Attempt = 1;
	} // else
#ifdef WCS_BUILD_VNS
else
	{
	strcpy(PrefsName, WCS_PROJECT_PREFS_NAME_WCS);
	} // else
#endif // WCS_BUILD_VNS

// we don't care if the project was already opened before - we now want to load the way it was when WCS closed last
strmfp(filename, ProgDir, PrefsName); //lint !e645
if (Load(NULL, filename, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages))
	{
	if (! Prefs.OpenWindows)
		{
		GlobalApp->MCP->OpenWindows(this);
		} // if not already opened by the project loader
	return (1);
	} // if
else if (Attempt == 0 && Prefs.CurrentUserName[0])
	{
	Attempt = 1;
	goto TryAgain;
	} // else if
#ifdef WCS_BUILD_VNS
else if (Attempt < 2)
	{
	Attempt = 2;
	goto TryAgain;
	} // else if
#endif // WCS_BUILD_VNS

return (0);

} // Project::LoadResumeState()

/*===========================================================================*/

short Project::SavePrefs(char *ProgDir)
{
char filename[256], UserPrefsName[256];

if (OverridePrefsFile[0]) // support user-supplied command-line arg for prefs file
	{
	strmfp(filename, ProgDir, OverridePrefsFile);
	} // if
else
	{
	strmfp(filename, ProgDir, WCS_PROJECT_PREFS_NAME);
	} // else
//if (Save(filename, GlobalApp->AppDB, GlobalApp->ActivePar,
//	IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
//		WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
//		WCS_PROJECT_IODETAILTAG_CHUNKID, "Prefs",
//		WCS_PROJECT_IODETAILTAG_CHUNKID, "Config",
//		WCS_PROJECT_IODETAILTAG_DONE)))
//	return (1);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Saving Prefs file...");
if (Save(NULL, filename, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL))
	{
	if (Prefs.MultiUserMode && Prefs.CurrentUserName[0])
		{
		strcpy(UserPrefsName, Prefs.CurrentUserName);
		strcat(UserPrefsName, ".prefs");
		strmfp(filename, ProgDir, UserPrefsName);
		if (Save(NULL, filename, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL))
			return (1);
		else
			return (0);
		} // if
	return (1);
	} // if

return (0);

} // Project::SavePrefs()

/*===========================================================================*/

int Project::ValidateAssignPaths(void)
{
int ProjectPathInvalid = 0, FramePathInvalid = 0, ContentPathInvalid = 0;
char filename[256];
#ifdef WCS_BUILD_EDSS
int EDSSPass = 0;

RepeatEDSSTry:
#endif // WCS_BUILD_EDSS


ProjectPathInvalid = chdir(UnifyPathGlyphs(pcprojectpath));
FramePathInvalid = chdir(UnifyPathGlyphs(pcframespath));
ContentPathInvalid = chdir(UnifyPathGlyphs(contentpath));
chdir(GlobalApp->GetProgDir());

#ifdef WCS_BUILD_EDSS
if (! EDSSPass && (ProjectPathInvalid || FramePathInvalid || ContentPathInvalid)
	&& Prefs.PrivateQueryConfigOpt("edss_config"))
	{
	if (ProjectPathInvalid)
		strmfp(pcprojectpath, GlobalApp->GetProgDir(), "EDSS");
	if (FramePathInvalid)
		strmfp(pcframespath, GlobalApp->GetProgDir(), "EDSS/Output Imagery");
	if (ContentPathInvalid)
		strmfp(contentpath, GlobalApp->GetProgDir(), "Components");
	EDSSPass = 1;
	goto RepeatEDSSTry;
	} // if
#endif // WCS_BUILD_EDSS

if (ProjectPathInvalid)
	{
	UserMessageOK("Project Master Path", 
"Project Master Path is invalid!\n\
   This path is used by "APP_TLA" to locate your Projects.\n\n\
Click OK and then select the path on your hard drive where your\n\
   Project files and the Project sub-directories are located.\n\
   Project files end in \".proj.\" Normally you should choose the\n\
   \"WCSProjects\" directory inside your WCS directory.\n\n\
The program will not run without this path.\n\n\
For more information about Master Paths see your "APP_TLA" documentation.");
	strmfp(pcprojectpath, GlobalApp->GetProgDir(), "WCSProjects");

	FileReq FR;
	FR.SetDefPath(pcprojectpath);
	FR.SetTitle("Select Project Master Path");
	if (FR.Request(WCS_REQUESTER_FILE_DIRONLY | WCS_REQUESTER_FILE_NOMUNGE))
		{
		pcprojectpath[0] = 0;
		filename[0] = 0;
		BreakFileName((char *)FR.GetNextName(), pcprojectpath, 255, filename, 256);
		} // if
	else
		{
		return (0);
		} // else

	strcpy(projectpath, "WCSProjects:");
	} // if
#ifdef WCS_BUILD_DEMO
else
	{
	FILE *TestWrite = NULL;
	int RedirectedOK = 0;
	if (TestWrite = PROJ_fopen("WCSProjects:testfile.tmp", "w"))
		{
		fclose(TestWrite);
		PROJ_remove("WCSProjects:testfile.tmp");
		} // if
	else
		{
		// Try to identify a good place where the Demo version can create a temporary
		// WCSProjects directory on a read/write filesystem.
		// Try C:/temp/wcsprojects, c:/temp and c:
		if (TestWrite = fopen("c:\\temp\\WCSProjects\\test.tmp", "wb+"))
			{
			fclose(TestWrite);
			remove("c:\\temp\\WCSProjects\\test.tmp");
			strcpy(pcprojectpath, "c:\\temp\\WCSProjects\\");
			RedirectedOK = 1;
			} // if
		if (!RedirectedOK)
			{
			if (TestWrite = fopen("c:\\temp\\test.tmp", "wb+"))
				{
				fclose(TestWrite);
				remove("c:\\temp\\test.tmp");
				if (!mkdir("c:\\temp\\WCSProjects"))
					{
					strcpy(pcprojectpath, "c:\\temp\\WCSProjects\\");
					RedirectedOK = 1;
					} // if
				} // if
			} // if
		if (!RedirectedOK)
			{
			if (TestWrite = fopen("c:\\tmp\\test.tmp", "wb+"))
				{
				fclose(TestWrite);
				remove("c:\\tmp\\test.tmp");
				if (!mkdir("c:\\tmp\\WCSProjects"))
					{
					strcpy(pcprojectpath, "c:\\tmp\\WCSProjects\\");
					RedirectedOK = 1;
					} // if
				} // if
			} // if
		if (!RedirectedOK)
			{
			if (TestWrite = fopen("c:\\test.tmp", "wb+"))
				{
				fclose(TestWrite);
				remove("c:\\test.tmp");
				if (!mkdir("c:\\WCSProjects"))
					{
					strcpy(pcprojectpath, "c:\\WCSProjects\\");
					RedirectedOK = 1;
					} // if
				} // if
			} // if
		if (RedirectedOK)
			{
			sprintf((char *)ThumbNailR, "The "APP_TLA" Demo version has determined it is being\nrun from a non-writable disk (like a CD-ROM).\n\
Therefore, a WCSProjects directory has been created in\n\"%s\"\nto store temporary files while the demo version runs.\n\
You may wish to delete this directory when\nyou are completely finished with the demo.", pcprojectpath);
			UserMessageOK("Demo Version Temporary Directory", (char *)ThumbNailR);
			} // if
		else
			{
			UserMessageOK("Demo Version Temporary Directory", "The "APP_TLA" Demo version has determined it is being\nrun from a non-writable disk (like a CD-ROM).\n\
"APP_TLA" was unable to create a directory to store\ntemporary files while the demo version runs.\n\n\
You may either:\nEnsure your system has a temporary directory\n\
(typically C:\\TEMP for Windows or\nTemporary Items in the System Folder for a Mac)\n\
or\ncopy the contents of the WCSDemo CD\nto your hard drive before running.");
			} // 
		} // else
	} // else

#endif // WCS_BUILD_DEMO

if (ContentPathInvalid)
	{
	UserMessageOK("Content Master Path",
"Content Master Path is invalid!\n\
   This path is used by "APP_TLA" to locate Components for the Component Gallery.\n\n\
Click OK and then select the path on your hard drive where your\n\
   Component sub-directories are located.\n\
   Component sub-directories are named \"Image\", \"LandCover\", \"Water\", etc.\n\
   It will normally be found in the WCS directory.\n\n\
The program will not run without this path.\n\n\
For more information about Master Paths see your "APP_TLA" documentation.");
	strmfp(contentpath, GlobalApp->GetProgDir(), "Components");

	FileReq FR;
	FR.SetDefPath(contentpath);
	FR.SetTitle("Select Content Master Path");
	if (FR.Request(WCS_REQUESTER_FILE_DIRONLY | WCS_REQUESTER_FILE_NOMUNGE))
		{
		contentpath[0] = 0;
		filename[0] = 0;
		BreakFileName((char *)FR.GetNextName(), contentpath, 255, filename, 256);
		} // if
	else
		{
		return (0);
		} // else

	} // if

if (FramePathInvalid)
	{
	// Mac can't handle a requester with 255 or more characters
	UserMessageOK("Frames Master Path",
"Frames Master Path is invalid!\n\
   This path is used by "APP_TLA" to save your output images.\n\n\
Click OK and then select the path on your hard drive where your\n\
   Rendered Image files are to be stored.\n\
   You may choose any existing directory on your system.\n\n\
The program will not run without this path.\n\n\
For more information about Master Paths see your "APP_TLA" documentation.");
	strmfp(pcframespath, GlobalApp->GetProgDir(), "WCSFrames");

	FileReq FR;
	FR.SetDefPath(pcframespath);
	FR.SetTitle("Select Frames Master Path");
	if (FR.Request(WCS_REQUESTER_FILE_DIRONLY | WCS_REQUESTER_FILE_NOMUNGE))
		{
		pcframespath[0] = 0;
		filename[0] = 0;
		BreakFileName((char *)FR.GetNextName(), pcframespath, 255, filename, 256);
		} // if
	else
		{
		return (0);
		} // else

	strcpy(framepath, "WCSFrames:");
	} // if

if (ProjectPathInvalid || FramePathInvalid || ContentPathInvalid)
	{
	//if (! PrefsLoaded)	// removed 10/29/01 by GRH
		{
		ProjectLoaded = 1;
#ifndef WCS_BUILD_DEMO
		SavePrefs(GlobalApp->GetProgDir());
#endif // !WCS_BUILD_DEMO
		ProjectLoaded = 0;
		} // if
	} // if

#ifndef WCS_BUILD_DEMO
if (UserName[0] == 0)
	{
	memset(UserName, 0, sizeof(UserName));	// keep crap from showing up in requester the first time
	GetInputString("Registration:\n\nPlease enter a User Name. This will be used for identifying any stand-alone components you save from "APP_TLA".", "", UserName);
	} // if

if (UserEmail[0] == 0)
	{
	memset(UserEmail, 0, sizeof(UserEmail));	// keep crap from showing up in requester the first time
	GetInputString("Registration:\n\nPlease enter an E-mail address. This will be used for identifying any stand-alone components you save from "APP_TLA".", "", UserEmail);
	} // if
#endif // !WCS_BUILD_DEMO

return (pcprojectpath[0] && pcframespath[0] && contentpath[0]);

} // Project::ValidateAssignPaths

/*===========================================================================*/

void Project::ClearTemplates(void)
{
long CurTemplate;

for (CurTemplate = 0; CurTemplate < WCS_MAX_TEMPLATES; CurTemplate ++)
	ProjectTemplates[CurTemplate].Clear();

} // Project::ClearTemplates

/*===========================================================================*/

struct ProjectIODetail *Project::IODetail(int Flags, ...)
{
va_list VarA;
int Tag, ItemNum = 0, LastItem = -1;
struct ProjectIODetail *Root = NULL, *Current;
struct ChunkIODetail *Detail = NULL, **DetailPtr;

va_start(VarA, Flags);

Tag = va_arg(VarA, int);
if (Tag == WCS_PROJECT_IODETAILTAG_CHUNKID)
	{
	if (Root = ProjectIODetail_New())
		{
		Current = Root;
		Detail = Current->Detail;
		DetailPtr = &Current->Detail;
		Current->Flags = Flags;
		strncpy(Current->ChunkID, va_arg(VarA, char *), 8);
		while (Tag && Tag != WCS_PROJECT_IODETAILTAG_DONE)
			{
			switch (Tag = va_arg(VarA, int))
				{
				case WCS_PROJECT_IODETAILTAG_CHUNKID:
					{
					if (Current->Next = ProjectIODetail_New())
						{
						Current = Current->Next;
						Detail = Current->Detail;
						DetailPtr = &Current->Detail;
						strncpy(Current->ChunkID, va_arg(VarA, char *), 8);
						} // if allocated
					else
						{
						Tag = WCS_PROJECT_IODETAILTAG_DONE;
						} // else bail
					break;
					}
				case WCS_PROJECT_IODETAILTAG_GROUP:
					{
					if (*DetailPtr = ChunkIODetail_New())
						{
						Detail = *DetailPtr;
						DetailPtr = &Detail->Next;
						Detail->Group = (USHORT)va_arg(VarA, int);
						}
					else
						{
						Tag = WCS_PROJECT_IODETAILTAG_DONE;
						} // else out of memory - gotta quit
					break;
					}
				case WCS_PROJECT_IODETAILTAG_NUMITEMS:
					{
					if (Detail)
						{
						Detail->NumItems = (short)va_arg(VarA, int);
						ItemNum = 0;
						LastItem = -1;
						if (Detail->NumItems > 0)
							{
							if (! (Detail->ItemList = (short *)AppMem_Alloc(Detail->NumItems * sizeof (short), APPMEM_CLEAR)))
//							if (! (Detail->ItemList = (short *)AppMem_Alloc(Detail->NumItems * sizeof (short), APPMEM_CLEAR))
//								|| ! (Detail->StyleList = (USHORT *)AppMem_Alloc(Detail->NumItems * sizeof (USHORT), APPMEM_CLEAR)))
								{
								Tag = WCS_PROJECT_IODETAILTAG_DONE;
								} // if ! allocated
							} // if
						} // if
					break;
					}
				case WCS_PROJECT_IODETAILTAG_STYLE:
					{
//					if (Detail && Detail->StyleList && ItemNum < Detail->NumItems)
//						{
//						Detail->StyleList[ItemNum] = (USHORT)va_arg(VarA, int);
//						} // if
					break;
					}
				case WCS_PROJECT_IODETAILTAG_ITEM:
					{
					if (Detail && Detail->ItemList && ItemNum < Detail->NumItems)
						{
						if (ItemNum == LastItem)
							ItemNum ++;
						Detail->ItemList[ItemNum] = (short)va_arg(VarA, int);
						LastItem = ItemNum;
						} // if
					break;
					}
				case WCS_PROJECT_IODETAILTAG_FLAGS:
					{
					if (Detail)
						{
						Detail->Flags |= (ULONG)va_arg(VarA, int);
						} // if a group has been read and detail allocated
					else
						{
						Current->Flags |= (ULONG)va_arg(VarA, int);
						} // else no detail created yet
					break;
					}
				case WCS_PROJECT_IODETAILTAG_USERDATA1:
					{
					if (Detail)
						{
						Detail->UserData1 = va_arg(VarA, void *);
						} // if
					break;
					}
				case WCS_PROJECT_IODETAILTAG_USERDATA2:
					{
					if (Detail)
						{
						Detail->UserData2 = va_arg(VarA, void *);
						} // if
					break;
					}
				case WCS_PROJECT_IODETAILTAG_DONE:
					{
					break;
					}
				default:
					{
					Tag = WCS_PROJECT_IODETAILTAG_DONE;
					break;
					} // default - done
				} // switch
			} // while
		} // if root allocated
	} // if first item is chunk ID - otherwise this is a do-nothing operation

va_end(VarA);

return (Root);

} // Project::IODetail()

/*===========================================================================*/

struct ProjectIODetail *Project::ProjectIODetail_New(void)
{

return ((struct ProjectIODetail *)AppMem_Alloc(sizeof (struct ProjectIODetail), APPMEM_CLEAR));

} // Project::ProjectIODetail_New()

/*===========================================================================*/

struct ChunkIODetail *Project::ChunkIODetail_New(void)
{

return ((struct ChunkIODetail *)AppMem_Alloc(sizeof (struct ChunkIODetail), APPMEM_CLEAR));

} // Project::ChunkIODetail_New()

/*===========================================================================*/

void Project::ProjectIODetail_Del(struct ProjectIODetail *This)
{
struct ProjectIODetail *DeleteThis;
struct ChunkIODetail *DeleteDetail, *Detail;

while (This)
	{
	DeleteThis = This;
	This = This->Next;
	Detail = DeleteThis->Detail;
	while (Detail)
		{
		DeleteDetail = Detail;
		Detail = Detail->Next;
		if (DeleteDetail->ItemList)
			AppMem_Free(DeleteDetail->ItemList, DeleteDetail->NumItems * sizeof (short));
//		if (DeleteDetail->StyleList)
//			AppMem_Free(DeleteDetail->StyleList, DeleteDetail->NumItems * sizeof (USHORT));
		AppMem_Free(DeleteDetail, sizeof (struct ChunkIODetail));
		} // while
	AppMem_Free(DeleteThis, sizeof (struct ProjectIODetail));
	} // while

} // Project::ProjectIODetail_Del()

/*===========================================================================*/

struct ProjectIODetail *Project::ProjectIODetailSearch(struct ProjectIODetail *This, char *Search)
{
/*

struct ProjectIODetail *Found = NULL;

if (Search)
	{
	while (This)
		{
		if (! strnicmp(This->ChunkID, Search, 8))
			{
			Found = This;
			break;
			} // if found
		This = This->Next;
		} // while
	} // if

return (Found);
*/

// implementation is stand-alone C function in ProjectIODetail.cpp
// so it can be used from DEMCore.cpp without Project object available
return(ProjectIODetailSearchStandAlone(This, Search));

} // Project::ProjectIODetailSearch()

/*===========================================================================*/

struct ChunkIODetail *Project::ChunkIODetailSearch(struct ChunkIODetail *This, USHORT Group)
{
struct ChunkIODetail *Found = NULL;

while (This)
	{
	if (Group == This->Group)
		{
		Found = This;
		break;
		} // if found
	This = This->Next;
	} // while

return (Found);

} // Project::ChunkIODetailSearch()

/*===========================================================================*/

short Project::ItemIODetailSearch(struct ChunkIODetail *This, short Item)
{
short Found = -1, ItemNum = 0;

while (ItemNum < This->NumItems)
	{
	if (Item == This->ItemList[ItemNum])
		{
		Found = ItemNum;
		break;
		} // if found
	ItemNum ++;
	} // while

return (Found);

} // Project::IODetailSearchChunk()

/*===========================================================================*/

//static char BCheckerHoze[255];

int Project::Load(RasterAnimHostProperties *fFileSupplied, char *LoadName, Database *DB, EffectsLib *Effects, 
	ImageLib *Images, struct ProjectIODetail *Detail, unsigned long loadcode)
{
FILE *fproject = NULL;
struct ProjectIODetail *LocalDetail;
ULONG ByteOrder, Size, BytesRead = 1, TotalRead = 0;
#ifdef DATABASE_CHUNK_DEBUG
long fTale;
#endif // DATABASE_CHUNK_DEBUG
short ReadError = 0, PartialRead = 0, PathsLoaded = 0, DisplayName = 0,
	NewDBLoaded = 0, EffectsLoaded = 0, ImagesLoaded = 0, LocalPrefsLoaded = 0, V4Warned = 0, ByteFlip;
NotifyTag Changes[3];
char filename[256], Ptrn[32], Title[12], ReadBuf[32];
UBYTE Version, Revision;
time_t StartTime, EndTime, ElapsedTime;

if (! Detail && BrowseInfo)
	BrowseInfo->FreeAll();

if (fFileSupplied)
	{
	fproject = fFileSupplied->fFile;
	ByteFlip = fFileSupplied->ByteFlip;
	strcpy(filename, LoadName);
	} // if
else
	{
	if (LoadName)
		{
		strcpy(filename, LoadName);
		} // if name provided, as in prefs file 
	else
		{
		strcpy(ReadBuf, "Effects");
		LocalDetail = NULL;
		if (Detail && (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
			{
			strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("proj"));
			if (! GetFileNamePtrn(0, "Load Effects", parampath, paramfile, Ptrn, 32))
				goto EndLoad;
			strmfp(filename, parampath, paramfile);
			PartialRead = 1;
			goto ReadyToLoad;
			} // if load effects
		strcpy(ReadBuf, "Images");
		LocalDetail = NULL;
		if (Detail && (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
			{
			strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("proj"));
			if (! GetFileNamePtrn(0, "Load Images", parampath, paramfile, Ptrn, 32))
				goto EndLoad;
			strmfp(filename, parampath, paramfile);
			PartialRead = 1;
			goto ReadyToLoad;
			} // if load images
		strcpy(ReadBuf, "Database");
		LocalDetail = NULL;
		if (Detail && (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
			{
			strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("db")";"WCS_REQUESTER_PARTIALWILD("proj"));
			if (! GetFileNamePtrn(0, "Load Database", dbasepath, dbasename, Ptrn, 32))
				goto EndLoad;
			strmfp(filename, dbasepath, dbasename);
			PartialRead = 1;
			goto ReadyToLoad;
			} // if load database only
		// last resort - load project file
		strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("proj"));
		if (! GetFileNamePtrn(0, "Load Project", projectpath, projectname, Ptrn, 64))
			goto EndLoad;
		strmfp(filename, projectpath, projectname);
		} // else file name not provided - set up extensions and get name from user
	} // else

ReadyToLoad:

if (this == GlobalApp->MainProj)
	{
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

if (! fproject && ((fproject = PROJ_fopen(filename, "rb")) == NULL))
	{
	if (! LoadName)
		UserMessageOK(APP_TLA" Project: Load", "Can't open project file!\nOperation terminated.");
	goto EndLoad;
	} // if open fail 

GetTime(StartTime);	// in Useful.cpp

if (! fFileSupplied)
	{
	fread((char *)ReadBuf, 14, 1, fproject);

	memcpy(Title, &ReadBuf[0], 8);
	memcpy(&Version, &ReadBuf[8], 1);
	memcpy(&Revision, &ReadBuf[9], 1);
	memcpy(&ByteOrder, &ReadBuf[10], 4);

	if (ByteOrder == 0xaabbccdd)
		ByteFlip = 0;
	else if (ByteOrder == 0xddccbbaa)
		ByteFlip = 1;
	else
		{
		UserMessageOK(LoadName, "Unsupported byte order in file!\nOperation terminated.");
		fclose(fproject);
		goto EndLoad;
		} // else

	Title[11] = '\0';
	if (strnicmp(Title, "WCS File", 8))
		{
		UserMessageOK("Load Project", "This is not a supported "APP_TLA" file!\nOperation terminated.");
		fclose(fproject);
		goto EndLoad;
		} // if not V2+ WCS File 
	else if (Version < 2 || Version > WCS_PROJECT_CURRENT_VERSION)	// Version 2 was used for WCS V2 and V3
		{
		fclose(fproject);
		if (! LoadName)
			UserMessageOK("Project: Load", "Unsupported "APP_TLA" File version!\nOperation terminated.");
		goto EndLoad;
		} // else if unsupported version
	else if (Version != WCS_PROJECT_CURRENT_VERSION && PartialRead)	// Version 2 was used for WCS V2 and V3
		{
		fclose(fproject);
		UserMessageOK("Project: Load", "Partial files can only be loaded if saved with the current version of "APP_TLA".");
		goto EndLoad;
		} // else if unsupported version

	TotalRead = BytesRead = 14;
	} // if
else
	{
	Version = fFileSupplied->FileVersion;
	Revision = fFileSupplied->FileRevision;
	ByteFlip = fFileSupplied->ByteFlip;
	} // else

RasterAnimHost::SetActiveLock(1);
if (! Detail)
	GlobalApp->GUIWins->CVG->CloseViewsForIO(1);

// now that we know we've got a valid file we clean up the parts of the application that will be replaced.
// turn off the flag that tells the saver it is OK to save prefs file.
if (! Detail)
	ProjectLoaded = 0;


// need to set some flags to indicate what we're about to clear to expedite
// clearing of related data subsystems
char LocalDatabaseDisposalInProgress, LocalImageDisposalInProgress, LocalEffectsDisposalInProgress;

LocalDetail = NULL;
strcpy(ReadBuf, "Images");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_IMAGES_LOAD_CLEAR))	//lint !e648
		{
		LocalImageDisposalInProgress = GlobalApp->ImageDisposalInProgress;
		GlobalApp->ImageDisposalInProgress = 1;
		} // if load from scratch
	} // if we're gonna load images

LocalDetail = NULL;
strcpy(ReadBuf, "Effects");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_EFFECTS_LOAD_CLEAR))	//lint !e648
		{
		LocalEffectsDisposalInProgress = GlobalApp->EffectsDisposalInProgress;
		GlobalApp->EffectsDisposalInProgress = 1;
		} // if load from scratch
	} // if

LocalDetail = NULL;
strcpy(ReadBuf, "Database");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_DATABASE_LOAD_CLEAR))	//lint !e648
		{
		LocalDatabaseDisposalInProgress = GlobalApp->DatabaseDisposalInProgress;
		GlobalApp->DatabaseDisposalInProgress = 1;
		} // if load from scratch
	} // if


// clear images if necessary
if (! Images)
	Images = GlobalApp->AppImages;
GlobalApp->LoadToImageLib = Images;
LocalDetail = NULL;
strcpy(ReadBuf, "Images");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_IMAGES_LOAD_CLEAR))	//lint !e648
		{
		if (! Detail || (LocalDetail->Flags & WCS_IMAGES_LOAD_CLOSEWINDOWS))
			GlobalApp->MCP->CloseDangerousWindows();
		Images->DeleteAll();
		} // if load from scratch
	} // if we're gonna load images
// clear effects if necessary
if (! Effects)
	Effects = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = Effects;
LocalDetail = NULL;
strcpy(ReadBuf, "Effects");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_EFFECTS_LOAD_CLEAR))	//lint !e648
		{
		if (! Detail || (LocalDetail->Flags & WCS_EFFECTS_LOAD_CLOSEWINDOWS))
			GlobalApp->MCP->CloseDangerousWindows();
		Effects->DeleteAll(TRUE);
		} // if load from scratch
	} // if

if (! Detail)
	{
	ClearTemplates();
	} // if

// clear database if necessary
if (! DB)
	DB = GlobalApp->AppDB;
LocalDetail = NULL;
strcpy(ReadBuf, "Database");
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
	{
	if (! Detail || (LocalDetail->Flags & (unsigned long)WCS_DATABASE_LOAD_CLEAR))	//lint !e648
		{
		// remove links to db if there are any, we're about to destroy the database
		Effects->RemoveDBLinks();
		GlobalApp->MainProj->Interactive->ClearAllPts();
		ClearPiers();
		DB->DestroyAll();
		} // if load from scratch
	} // if

// restore global indicators that dictated we were clearing things, as we're done with that stage now
GlobalApp->DatabaseDisposalInProgress = LocalDatabaseDisposalInProgress;
GlobalApp->ImageDisposalInProgress = LocalImageDisposalInProgress;
GlobalApp->EffectsDisposalInProgress = LocalEffectsDisposalInProgress;

while (BytesRead)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(fproject, (char *)ReadBuf,
		WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
		{
		TotalRead += BytesRead;
		ReadBuf[8] = 0;
		// read block size from file 
		if (BytesRead = ReadBlock(fproject, (char *)&Size,
			WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
			{
			TotalRead += BytesRead;
			BytesRead = 0;
			LocalDetail = NULL;
			if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
				{
				if (! strnicmp(ReadBuf, "Paths", 8))
					{
#ifdef DATABASE_CHUNK_DEBUG
					fTale = ftell(fproject);
					sprintf(debugStr, "Project::Load(Paths: Seek = %u, BytesRead = %u\n", fTale, BytesRead);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
					BytesRead = Paths_Load(fproject, Size, ByteFlip, Images, LocalDetail ? LocalDetail->Detail: NULL);
					if (! (LocalDetail && LocalDetail->Detail))	// keeps certain things from being done if only loading directory list
						PathsLoaded = 1;
					} // if paths
				else if (! strnicmp(ReadBuf, "Password", 8))
					{
					BytesRead = Password_Load(fproject, Size, ByteFlip, LocalDetail);
					} // else if prefs
				else if (! strnicmp(ReadBuf, "Browse", 8))
					{
					if (BrowseInfo)
						BrowseInfo->FreeAll();	// this may be redundant but better to do it twice than not at all
					else
						BrowseInfo = new BrowseData();
					if (BrowseInfo)
						BytesRead = BrowseInfo->Load(fproject, Size, ByteFlip);
					else if (! fseek(fproject, Size, SEEK_CUR))
						BytesRead = Size;
					} // else if browse data
				else if (! strnicmp(ReadBuf, "Config", 8))
					{
					if (! PrefsLoaded)
						BytesRead = GUIConfig_Load(fproject, Size, ByteFlip);
					else if (! fseek(fproject, Size, SEEK_CUR))
						BytesRead = Size;
					} // else if config
				else if (LocalDetail && ! strnicmp(ReadBuf, "UsrConfg", 8))
					{
					if (! PrefsLoaded)
						BytesRead = UserGUIConfig_Load(fproject, Size, ByteFlip);
					else if (! fseek(fproject, Size, SEEK_CUR))
						BytesRead = Size;
					} // else if usrconfig
				#ifdef WCS_BUILD_VNS
				else if (! strnicmp(ReadBuf, "Template", 8))
					{
					BytesRead = Templates_Load(fproject, Size, ByteFlip, DB, Effects, Detail);
					} // else if prefs
				#endif // WCS_BUILD_VNS
				else if (! strnicmp(ReadBuf, "Prefs", 8))
					{
					BytesRead = Prefs_Load(fproject, Size, ByteFlip, LocalDetail);
					PrefsLoaded = LocalPrefsLoaded = 1;
					} // else if prefs
				else if (! strnicmp(ReadBuf, "Params", 8))
					{
					if (! V4Warned)
						{
						if (UserMessageYN(projectname, "This is an old file format no longer supported. Not all components will be loaded from this file. Do you wish to continue to load whatever can be loaded?"))
							{
							if (! fseek(fproject, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						V4Warned = 1;
						} // if
					DisplayName = 1;
					} // else if params
				else if (! strnicmp(ReadBuf, "Images", 8))
					{
					BytesRead = Images->Load(fproject, Size, LocalDetail ? LocalDetail->Detail: NULL);
					ImagesLoaded = 1;
					DisplayName = 1;
					} // else if images
				else if (! strnicmp(ReadBuf, "Effects", 8))
					{
					if (Version < 4)
						{
						if (! V4Warned)
							{
							if (UserMessageYN(projectname, "This is an old file format no longer supported. Not all components will be loaded from this file. Do you wish to continue to load whatever can be loaded?"))
								{
								if (! fseek(fproject, Size, SEEK_CUR))
									BytesRead = Size;
								} // if
							V4Warned = 1;
							} // if
						} // if older file with effects read too early
					else
						{
						BytesRead = Effects->Load(fproject, Size, (LocalDetail ? LocalDetail->Detail: NULL));
						EffectsLoaded = 1;
						DisplayName = 1;
						} // else
					} // else if effects
				else if (! strnicmp(ReadBuf, "ImportOp", 8))
					{
					BytesRead = ImportOps_Load(fproject, Size, ByteFlip, DB, Effects);
					} // else if prefs
				else if (! strnicmp(ReadBuf, "Database", 8))
					{
					if (Version < 4)
						{
						if (! V4Warned)
							{
							if (UserMessageYN(projectname, "This is an old file format no longer supported. Not all components will be loaded from this file. Do you wish to continue to load whatever can be loaded?"))
								{
								if (! fseek(fproject, Size, SEEK_CUR))
									BytesRead = Size;
								} // if
							V4Warned = 1;
							} // if
						} // if older file with effects and database read too early
					else
						{
#ifdef DATABASE_CHUNK_DEBUG
						fTale = ftell(fproject);
						sprintf(debugStr, "Project::Load(Database: Seek = %u\n", fTale);
						OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
						BytesRead = DB->LoadV2(fproject, Size, WCS_DATABASE_STATIC, this, Effects);
						// resolve load linkages with effects that are not attributes on Joes, such as Scenarios.
						if (EffectsLoaded)
							Effects->ResolveDBLoadLinkages(DB);
						NewDBLoaded = 1;
						DisplayName = 1;
						} // else
					} // else if database
				else if (! strnicmp(ReadBuf, "ViewInit", 8))
					{
					BytesRead = GlobalApp->GUIWins->CVG->Load(fproject, Size, ByteFlip);
					} // else if database
				else if (! fseek(fproject, Size, SEEK_CUR))
					BytesRead = Size;
				} // if interested in this chunk
			else if (! fseek(fproject, Size, SEEK_CUR))
				BytesRead = Size;
			TotalRead += BytesRead;
			if (BytesRead != Size)
				{
				ReadError = 1;
				break;
				} // if error
			} // if size block read 
		else
			break;
		} // if tag block read 
	else
		break;
	} // while 

if (! fFileSupplied)
	fclose(fproject);

GetTime(EndTime);	// in Useful.cpp
ElapsedTime = EndTime - StartTime;

// put these back to their default states
GlobalApp->LoadToImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

RasterAnimHost::SetActiveLock(0);
RasterAnimHost::SetActiveRAHost(NULL);

if (! ReadError)
	{
	if (this == GlobalApp->MainProj)
		{
		GlobalApp->UpdateProjectByTime();
		if (NewDBLoaded && (Interactive->GetFloating() && (Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X) == 0.0 && Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y) == 0.0)))
			Interactive->SetFloating(1, DB);
		if (DisplayName)
			{
			GlobalApp->WinSys->UpdateRootTitle(projectname, Prefs.CurrentUserName);
			ProjectLoaded = 1;
			} // if
		if (NewDBLoaded)
			{
			DB->SetFirstActive();
			} // if
		if (PathsLoaded)
			{
			int NamePtr;

			//strncpy(BCheckerHoze, "This is a test of Boundschecker errors!", 12);
			UpdateRecentProjectList();
			NamePtr = (int)(strlen(filename) - 1);
			while (NamePtr > 0 && filename[NamePtr] != ':' && filename[NamePtr] != '\\' && filename[NamePtr] != '/')
				NamePtr --; 
			SetParam(1, WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff, 0, NULL);
			GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_PROJ_LOAD, &filename[NamePtr + 1]);

			if(ElapsedTime > 0)
				{
				char ElapsedTimeMsg[512];
				sprintf(ElapsedTimeMsg, "Elapsed load time: %d seconds.", (unsigned long)ElapsedTime);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ElapsedTimeMsg);
				} // if

			} // if
		if (LocalPrefsLoaded)
			SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, Prefs.GUIConfiguration, NULL);
		if (! Detail)
			Interactive->SetActiveTime(0.0);
		if (! Detail && Prefs.OpenWindows)
			{
			GlobalApp->GUIWins->CVG->ReopenIOComplete(1);
			GlobalApp->MCP->OpenWindows(this);
			} // if load all and open windows
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	if (Detail && (Detail->Flags & WCS_PROJECT_IODETAILFLAG_DESTROY))
		ProjectIODetail_Del(Detail);
	RasterAnimHost::SetActiveRAHost(NULL);
	if (GlobalApp->GUIWins->PUG)
		{ // show Project Update Window if it was created during loading of project
		// asking for page 1 shows it, by default it is created hidden by asking for page 0 
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_PUW, 1);
		} // if
	return (1);
	} // if no error
else
	{
	UserMessageOK("Project: Load", "A chunk size error was detected in loading the file. Not all data was read correctly.");
	} // else

EndLoad:

RasterAnimHost::SetActiveRAHost(NULL);

if (Detail && (Detail->Flags & WCS_PROJECT_IODETAILFLAG_DESTROY))
	ProjectIODetail_Del(Detail);
if (this == GlobalApp->MainProj)
	{
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
return (0);

} // Project::Load

/*===========================================================================*/

ULONG Project::Paths_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, ImageLib *ImgLib, struct ChunkIODetail *Detail)
{
ULONG ItemTag = 0, Tag, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
short ReadOnly = 0, ReadFileDirs = 0, DeleteDirList = 1, LoadPrevProj;
char TempStr[512];

LoadPrevProj = (short)(! LastProject[0][0]);

if (! Detail || ChunkIODetailSearch(Detail, WCS_PROJECT_LOAD_PATHS_FILEDIRS))
	{
	ReadFileDirs = 1;
	} // if

if (Detail && ChunkIODetailSearch(Detail, WCS_PROJECT_LOAD_PATHS_DIRLIST))
	{
	DeleteDirList = 0;
	} // if

while (ItemTag != WCS_PARAM_DONE)
{
// read block descriptor tag from file 
if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
	{
	TotalRead += BytesRead;
	if (ItemTag != WCS_PARAM_DONE)
		{
		// read block size from file 
		if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
			{
			TotalRead += BytesRead;
			BytesRead = 0;
			switch (ItemTag & 0xff)
				{
				case WCS_BLOCKSIZE_CHAR:
					{
					Size = MV.Char[0];
					break;
					}
				case WCS_BLOCKSIZE_SHORT:
					{
					Size = MV.Short[0];
					break;
					}
				case WCS_BLOCKSIZE_LONG:
					{
					Size = MV.Long;
					break;
					}
				} // switch 

			Tag = (ItemTag & 0xffff0000);
			if (! ReadFileDirs && Tag != WCS_PROJECTFILE_PATH_DL_RDONLY &&
				Tag != WCS_PROJECTFILE_PATH_DL_NEW && Tag != WCS_PROJECTFILE_PATH_DL_ADD)
				ItemTag = 0x11110000;	// just some unused value to create a default case

			switch (ItemTag & 0xffff0000)
				{
				case WCS_PROJECTFILE_PATH_PP_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (pcprojectpath[0] == 0)
						{
						strcpy(pcprojectpath, TempStr);
						if (pcprojectpath[strlen(pcprojectpath) - 1] != ':'
							&& pcprojectpath[strlen(pcprojectpath) - 1] != '\\')
							strcat(pcprojectpath, "\\");
						UnifyPathGlyphs(pcprojectpath);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PF_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (pcframespath[0] == 0)
						{
						strcpy(pcframespath, TempStr);
						if (pcframespath[strlen(pcframespath) - 1] != ':'
							&& pcframespath[strlen(pcframespath) - 1] != '\\')
							strcat(pcframespath, "\\");
						UnifyPathGlyphs(pcframespath);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_CN_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (contentpath[0] == 0)
						{
						strcpy(contentpath, TempStr);
						if (contentpath[strlen(contentpath) - 1] != ':'
							&& contentpath[strlen(contentpath) - 1] != '\\')
							strcat(contentpath, "\\");
						UnifyPathGlyphs(contentpath);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV1:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[0], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV2:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[1], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV3:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[2], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV4:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[3], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV5:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[4], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PREV6:
					{
					BytesRead = ReadLongBlock(ffile, (char *)TempStr, Size);
					TempStr[511] = 0;
					if (LoadPrevProj)
						{
						strcpy(LastProject[5], TempStr);
						} // if
					break;
					}
				case WCS_PROJECTFILE_PATH_PJ_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(projectpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_PJ_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[63] = 0;
					strcpy(projectname, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DB_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(dbasepath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DB_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(dbasename, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_PR_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(parampath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_PR_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(paramfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_FR_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(framepath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_FR_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[63] = 0;
					strcpy(framefile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_ZS_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(savezbufpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_ZS_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(savezbuffile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_BG_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(backgroundpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_BG_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(backgroundfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_CM_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(colormappath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_CM_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(colormapfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DR_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(dirname, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_TM_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(temppath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_CL_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(cloudpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_CL_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(cloudfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_WV_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(wavepath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_WV_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(wavefile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DF_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(deformpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DF_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(deformfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_IM_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(imagepath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_SN_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(sunpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_SN_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(sunfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_MN_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(moonpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_MN_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(moonfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_AN_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					strcpy(animpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_AN_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[31] = 0;
					strcpy(animfile, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_AO_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (! altobjectpath[0])
						strcpy(altobjectpath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_ID_PTH:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (! importdatapath[0])
						strcpy(importdatapath, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_UN_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[127] = 0;
					if (! UserName[0])
						strcpy(UserName, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_UE_NME:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[127] = 0;
					if (! UserEmail[0])
						strcpy(UserEmail, TempStr);
					break;
					}
				case WCS_PROJECTFILE_PATH_DL_RDONLY:
					{
					BytesRead = ReadBlock(ffile, (char *)&ReadOnly, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
					break;
					}
				case WCS_PROJECTFILE_PATH_DL_NEW:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (DL && DeleteDirList)
						{
						DirList_Del(DL);
						DL = NULL;
						} // if
					if (! DL)
						DL = DirList_New(TempStr, ReadOnly);
					else if (! DirList_ItemExists(DL, TempStr))
						DirList_Add(DL, TempStr, ReadOnly);
					break;
					}
				case WCS_PROJECTFILE_PATH_DL_ADD:
					{
					BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
					TempStr[255] = 0;
					if (DL)
						{
						if (! DirList_ItemExists(DL, TempStr))
							DirList_Add(DL, TempStr, ReadOnly);
						} // if
					break;
					}
				default:
					{
					if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					break;
					} 
				} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

if (ReadFileDirs)
	{
	if (! sunpath[0])
		strcpy(sunpath, imagepath);
	if (! moonpath[0])
		strcpy(moonpath, imagepath);
	} // if

return (TotalRead);

} // Project::Paths_Load()

/*===========================================================================*/

ULONG Project::GUIConfig_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_GUICONFIG_DIMENSIONS:
						{
						BytesRead = GUIDimensions_Load(ffile, Size, ByteFlip, &FenTrack, &FenTrackSize);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // Project::GUIConfig_Load()

/*===========================================================================*/

ULONG Project::UserGUIConfig_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
short ConfigsToRead, ConfigNumber = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_USERGUICONFIG_NUMCONFIGS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConfigsToRead, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_DIMENSIONS:
						{
						if (ConfigNumber < WCS_MAX_USER_WINDOWCONFIG)
							BytesRead = GUIDimensions_Load(ffile, Size, ByteFlip, &AltFenTrack[ConfigNumber], &AltFenTrackSize[ConfigNumber]);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						ConfigNumber ++;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

} // Project::UserGUIConfig_Load()

/*===========================================================================*/

ULONG Project::GUIDimensions_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, ProjWinInfo **FTPtr, unsigned short *FTSizePtr)
{
unsigned long WinID, Flags=0;
char PreferredCell[4];
short X = 6, Y = 44, W = 500, H = 300;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

PreferredCell[0] = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_GUICONFIG_WINID:
						{
						BytesRead = ReadBlock(ffile, (char *)&WinID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);

						SetWindowCell(WinID, PreferredCell, X, Y, W, H, FTPtr, FTSizePtr);
						SetWindowFlags(WinID, Flags, 0, FTPtr, FTSizePtr);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_PREFERREDCELL:
						{
						BytesRead = ReadBlock(ffile, (char *)PreferredCell, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_FLAGS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Flags, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_TOP:
						{
						BytesRead = ReadBlock(ffile, (char *)&Y, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_LEFT:
						{
						BytesRead = ReadBlock(ffile, (char *)&X, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_WIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&W, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_GUICONFIG_HEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&H, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

} // Project::GUIDimensions_Load()

/*===========================================================================*/

ULONG Project::Templates_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, Database *LoadToDB, EffectsLib *LoadToEffects, struct ProjectIODetail *Detail)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, CurTemplate = 0;
char FileName[512];
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_TEMPLATES_PROJECTTEMPLATE:
						{
						if (CurTemplate < WCS_MAX_TEMPLATES)
							{
							BytesRead = ProjectTemplates[CurTemplate].Load(ffile, Size, ByteFlip);
							CurTemplate ++;
							} // if
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while

for (; CurTemplate < WCS_MAX_TEMPLATES; CurTemplate ++)
	ProjectTemplates[CurTemplate].Clear();

for (CurTemplate = 0; CurTemplate < WCS_MAX_TEMPLATES; CurTemplate ++)
	{
	if (ProjectTemplates[CurTemplate].Enabled || ProjectTemplates[CurTemplate].Embed)
		{
		// clear any IDs already in memory so there is no conflict with the new template as it loads
		GlobalApp->LoadToImageLib->ClearRasterIDs();
		if (! LoadProjectAsTemplate(ProjectTemplates[CurTemplate].PAF.GetPathAndName(FileName), 
			ProjectTemplates[CurTemplate].Embed, LoadToDB, LoadToEffects, Detail))
			return (0);
		} // if
	} // for

// init all IDs so they can be referenced by subsequent project components as they load
GlobalApp->LoadToImageLib->InitRasterIDs();

return (TotalRead);

} // Project::Templates_Load()

/*===========================================================================*/

ULONG Project::ImportOps_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, Database *LoadToDB, EffectsLib *Effects)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
Pier1 **CurPierPtr = &Imports, *FirstPier = NULL;
ImportWizGUI *TempWiz;
union MultiVal MV;

while (*CurPierPtr)
	CurPierPtr = &(*CurPierPtr)->Next;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_IMPORTOPS_PIER1:
						{
						if (*CurPierPtr = new Pier1())
							{
							if ((BytesRead = (*CurPierPtr)->Load(ffile, Size, ByteFlip)) == Size)
								{
								if (! FirstPier)
									FirstPier = *CurPierPtr;
								(*CurPierPtr)->TemplateItem = GlobalApp->TemplateLoadInProgress;
								#ifdef WCS_BUILD_EDSS
								if (AuxImportPath[0] && AuxImportFile[0])
									{
									strcpy((*CurPierPtr)->InDir, AuxImportPath);
									strcpy((*CurPierPtr)->InFile, AuxImportFile);
									strmfp((*CurPierPtr)->LoadName, AuxImportPath, AuxImportFile);
									strcpy((*CurPierPtr)->NameBase, AuxImportFile);
									StripExtension((*CurPierPtr)->NameBase);
									} // if
								#endif // WCS_BUILD_EDSS
								CurPierPtr = &(*CurPierPtr)->Next;
								} // if
							else
								{
								// load failed, delete it before it can do any damage
								delete (*CurPierPtr);
								*CurPierPtr = NULL;
								} // else
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while

if (TotalRead == ReadSize && FirstPier)
	{
	// no errors so go 'head and import items
	if (TempWiz = new ImportWizGUI(LoadToDB, this, Effects, GlobalApp->StatusLog))
		{
		TempWiz->AutoImport(FirstPier);
		delete TempWiz;
		} // if
	} // if

return (TotalRead);

} // Project::ImportOps_Load()

/*===========================================================================*/

int Project::LoadProjectAsTemplate(char *LoadName, int Embed, Database *LoadToDB, EffectsLib *LoadToEffects, struct ProjectIODetail *Detail)
{
FILE *ffile = NULL;
struct ProjectIODetail *LocalDetail;
Project *TempProj;
char Title[12], ReadBuf[32];
UBYTE Version, Revision;
ULONG ByteOrder;
short ByteFlip;
ULONG Size, BytesRead = 1, TotalRead = 0;

GlobalApp->TemplateLoadInProgress = (! Embed);

// temporary project is needed for loading template's master paths
if (TempProj = new Project())
	{
	if (ffile = PROJ_fopen(LoadName, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, ffile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Version, &ReadBuf[8], 1);
		memcpy(&Revision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (ByteOrder == 0xaabbccdd)
			ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			ByteFlip = 1;
		else
			{
			UserMessageOK(LoadName, "Unsupported byte order in Template file!\nOperation terminated.");
			goto LoadError;
			} // else

		Title[11] = '\0';
		if (strnicmp(Title, "WCS File", 8))
			{
			UserMessageOK(LoadName, "This is not a supported "APP_TLA" file for Template!\nOperation terminated.");
			goto LoadError;
			} // if not V2+ WCS File 
		else if (Version < 4 || Version > WCS_PROJECT_CURRENT_VERSION)	// Version 4 was used for WCS V5 and VNS
			{
			UserMessageOK(LoadName, "Unsupported "APP_TLA" File version for Template!\nOperation terminated.");
			goto LoadError;
			} // else if unsupported version

		TotalRead = BytesRead = 14;

		while (BytesRead)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					LocalDetail = NULL;
					if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, ReadBuf)))
						{
						if (! strnicmp(ReadBuf, "Paths", 8))
							{
							BytesRead = TempProj->Paths_Load(ffile, Size, ByteFlip, GlobalApp->LoadToImageLib, LocalDetail ? LocalDetail->Detail: NULL);
							} // if paths
						else if (! strnicmp(ReadBuf, "Images", 8) && GlobalApp->LoadToImageLib)
							{
							BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
							// image paths need to be converted from one set of master paths to the current project's
							GlobalApp->LoadToImageLib->MungPaths(TempProj);
							GlobalApp->LoadToImageLib->UnMungPaths(this);
							} // else if images
						else if (! strnicmp(ReadBuf, "Effects", 8) && GlobalApp->LoadToEffectsLib)
							{
							BytesRead = GlobalApp->LoadToEffectsLib->Load(ffile, Size, (LocalDetail ? LocalDetail->Detail: NULL));
							} // else if effects
						else if (! strnicmp(ReadBuf, "ImportOp", 8) && LoadToDB)
							{
							BytesRead = ImportOps_Load(ffile, Size, ByteFlip, LoadToDB, LoadToEffects);
							} // else if import ops
						else if (! strnicmp(ReadBuf, "Database", 8) && LoadToDB)
							{
							BytesRead = LoadToDB->LoadV2(ffile, Size, WCS_DATABASE_STATIC, this, LoadToEffects);
							} // else if database
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						} // if size block read
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						UserMessageOK(LoadName, "Error loading Template file.\nProject loading terminated.");
						goto LoadError;
						} // if error
					} // if
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 

		fclose(ffile);
		} // if file
	delete TempProj;
	} // if temp project
else
	{
	UserMessageOK(LoadName, "Error creating temporary Project buffer.\nProject loading terminated.");
	goto LoadError;
	} // else

GlobalApp->TemplateLoadInProgress = 0;

return (1);

LoadError:

if (ffile)
	fclose(ffile);

GlobalApp->TemplateLoadInProgress = 0;

return (0);

} // Project::LoadProjectAsTemplate

/*===========================================================================*/

ULONG Project::Prefs_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ProjectIODetail *LocalDetail)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_PREFS_PROJECT:
						{
						if (LocalDetail)
							BytesRead = ProjectPrefs_Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PROJECTFILE_PREFS_INTER:
						{
						BytesRead = InterPrefs_Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // Project::Prefs_Load()

/*===========================================================================*/

ULONG Project::ProjectPrefs_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, Ct, TotalRead = 0, OptNumber = 0, LoadConfigOptions = 1;
union MultiVal MV;
unsigned short FixedCt = 0, LayerCt = 0, AttribCt = 0;
char TempStr[256];
long NumViewEnabled = 0, EnabledCt = 0, CurTemplate = 0;
char CurViewType = 0;
#ifdef WCS_BUILD_VNS
short DummyShort;
#endif // WCS_BUILD_VNS

for (Ct = 0; Ct < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; Ct ++)
	{
	if (Prefs.ConfigOptions[Ct][0])
		{
		LoadConfigOptions = 0;
		break;
		} // if already a string in any field
	} // for

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					/*
					case WCS_PROJECTFILE_PROJPREFS_LOGERR:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ReportMesg[0], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LOGWNG:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ReportMesg[1], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LOGMSG:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ReportMesg[2], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LOGDTA:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ReportMesg[3], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					*/
					case WCS_PROJECTFILE_PROJPREFS_RENDERPRI:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.RenderTaskPri, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_RENDERSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.RenderSize, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LOADONOPEN:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.LoadOnOpen, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_OPENWINDOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.OpenWindows, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_PROJSHOWICONTOOLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ProjShowIconTools, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_PROJSHOWANIMTOOLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ProjShowAnimTools, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_HORIZDISPLAYUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.HorDisplayUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VERTDISPLAYUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.VertDisplayUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_ANGLEDISPLAYUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.AngleDisplayUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_TIMEDISPLAYUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.TimeDisplayUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_POSLONHEMISPHERE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.PosLonHemisphere, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LATLONSIGNDISPLAY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.LatLonSignDisplay, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_SIGNIFICANTDIGITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.SignificantDigits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_TASKMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.TaskMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_ENABLEDFILTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.EnabledFilter, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_ANIMATEDFILTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.AnimatedFilter, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_GUICONFIGURATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.GUIConfiguration, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_SAGEXPANDED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.SAGExpanded, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_MATRIXTASKMODEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.MatrixTaskModeEnabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_MEMORYLIMITSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.MemoryLimitsEnabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_GLOBALADVANCEDENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.GlobalAdvancedEnabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_SAGBOTTOMHTPCT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.SAGBottomHtPct, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_INTERSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.InteractiveStyle, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_LASTUPDATEDATE:
						{
						// <<<>>> TIME64 issue
						// we need to read/write this as 64-bit for future-proofing
						BytesRead = ReadBlock(ffile, (char *)&Prefs.LastUpdateDate, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_SHOWDBBYLAYER:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ShowDBbyLayer, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_MAXSAGDBENTRIES:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.MaxSAGDBEntries, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_MAXSORTEDSAGDBENTRIES:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.MaxSortedSAGDBEntries, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VECPOLYMEMORYLIMIT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.VecPolyMemoryLimit, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DEMMEMORYLIMIT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DEMMemoryLimit, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_RECORDMODE:
						{ // RecordMode is currently unused and may be retired eventually
						BytesRead = ReadBlock(ffile, (char *)&Prefs.RecordMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_INTERACTIVEMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.InteractiveMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_KEYGROUPMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.KeyGroupMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_CONFIGOPTION:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (LoadConfigOptions && OptNumber < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX)
							{
							TempStr[WCS_PROJECT_PREFS_CONFIG_OPTIONS_LEN - 1] = 0;
							strcpy(Prefs.ConfigOptions[OptNumber], TempStr);
							OptNumber ++;
							} // if
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_GEOUNITSPROJECTED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DisplayGeoUnitsProjected, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_GLOBALSTARTUPPAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.GlobalStartupPage, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_PROJECTFILE_PROJPREFS_MULTIUSERMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&DummyShort, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (! Prefs.CurrentUserName[0])
							Prefs.MultiUserMode = DummyShort;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_CURRENTUSERNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						TempStr[63] = 0;
						if (! Prefs.CurrentUserName[0])
							strcpy(Prefs.CurrentUserName, TempStr);
						break;
						}
					#endif // WCS_BUILD_VNS
					case WCS_PROJECTFILE_PROJPREFS_LASTSWATCH:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						TempStr[63] = 0;
						if (! Prefs.LastColorSwatch[0])
							strcpy(Prefs.LastColorSwatch, TempStr);
						break;
						}

					case WCS_PROJECTFILE_PROJPREFS_DBENUMFIXEDCOLUMNS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DBENumFixedColumns, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (Prefs.DBENumFixedColumns > WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX)
							Prefs.DBENumFixedColumns = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBENUMLAYERCOLUMNS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DBENumLayerColumns, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (Prefs.DBENumLayerColumns > WCS_DBEDITGUI_MAX_GRID_COLUMNS)
							Prefs.DBENumLayerColumns = WCS_DBEDITGUI_MAX_GRID_COLUMNS;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBENUMATTRIBCOLUMNS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DBENumAttribColumns, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (Prefs.DBENumAttribColumns > WCS_DBEDITGUI_MAX_GRID_COLUMNS)
							Prefs.DBENumAttribColumns = WCS_DBEDITGUI_MAX_GRID_COLUMNS;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBEFIXEDCOLUMNWIDTHS:
						{
						if (FixedCt < Prefs.DBENumFixedColumns)
							BytesRead = ReadBlock(ffile, (char *)&Prefs.DBEFixedColumnWidths[FixedCt ++], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBELAYERCOLUMNWIDTHS:
						{
						if (LayerCt < Prefs.DBENumLayerColumns)
							BytesRead = ReadBlock(ffile, (char *)&Prefs.DBELayerColumnWidths[LayerCt ++], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBEATTRIBCOLUMNWIDTHS:
						{
						if (AttribCt < Prefs.DBENumAttribColumns)
							BytesRead = ReadBlock(ffile, (char *)&Prefs.DBEAttribColumnWidths[AttribCt ++], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBEFILTERMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.DBEFilterMode, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBESEARCHQUERYFILTERNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						strncpy(Prefs.DBESearchQueryFilterName, TempStr, WCS_EFFECT_MAXNAMELENGTH);
						Prefs.DBESearchQueryFilterName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DBELAYERFILTERNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						strncpy(Prefs.DBELayerFilterName, TempStr, WCS_EFFECT_MAXNAMELENGTH);
						Prefs.DBELayerFilterName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
						break;
						}

					case WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_NUMENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumViewEnabled, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_VIEWTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CurViewType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						EnabledCt = 0;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_ENABLED:
						{
						if (EnabledCt < WCS_VIEWGUI_ENABLE_MAX && EnabledCt < NumViewEnabled)
							BytesRead = ReadBlock(ffile, (char *)&Prefs.ViewEnabled[CurViewType][EnabledCt ++], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_CONTOURINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.ViewContOverlayInt[CurViewType], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_VECPROFEXPORT:
						{
						BytesRead = Prefs.VecExpData.Load(ffile, Size, ByteFlip);
						break;
						}

					#ifdef WCS_BUILD_VNS
					case WCS_PROJECTFILE_PROJPREFS_NEWPROJUSETEMPLATES:
						{
						BytesRead = ReadBlock(ffile, (char *)&Prefs.NewProjUseTemplates, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_PROJPREFS_DEFAULTTEMPLATE:
						{
						if (CurTemplate < WCS_MAX_TEMPLATES)
							{
							BytesRead = Prefs.DefaultTemplates[CurTemplate].Load(ffile, Size, ByteFlip);
							CurTemplate ++;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_VNS

					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

for ( ; CurTemplate < WCS_MAX_TEMPLATES; CurTemplate ++)
	Prefs.DefaultTemplates[CurTemplate].Clear();

return (TotalRead);

} // Project::ProjectPrefs_Load()

/*===========================================================================*/

ULONG Project::Password_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ProjectIODetail *LocalDetail)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
char TestPW[256], TempStr[256];
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_PROJPREFS_CURRENTPASSWORD:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						TempStr[63] = 0;
						strcpy(TestPW, Prefs.CurrentPassword);

						TestPassword:
						if (strcmp(TestPW, TempStr))
							{
							TestPW[0] = 0;
							if (GetInputString("This file is password protected. Please enter the password.", "", TestPW) && TestPW[0])
								{
								goto TestPassword;
								} // if
							else
								{
								UserMessageOK("Project File", "File loading failed due to improper password.");
								BytesRead = 0;
								} // else
							} // if password doesn't match current
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while

return (TotalRead);

} // Project::Password_Load()

/*===========================================================================*/

ULONG VectorExportData::Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
char TempStr[512];
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_VECPROFEXPORT_HORSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorSize, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertSize, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORLABELINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorLabelInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertLabelInterval, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VECLINEWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorLineWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_TERRAINLINEWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainLineWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_GRAPHOUTLINEWT:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphOutlineWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICLEN:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicLength, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICLEN:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicLength, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}

					case WCS_PROJECTFILE_VECPROFEXPORT_HORUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_LAYOUTUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LayoutUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORSCALELABEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorScaleLabel, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTSCALELABEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertScaleLabel, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWVECTOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawVector, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWTERRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORGRID:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawHorGrid, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTGRID:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawVertGrid, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORTICS:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawHorTics, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTTICS:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawVertTics, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORLABELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawHorLabels, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTLABELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawVertLabels, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_DRAWGRAPHOUTLINE:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawGraphOutline, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VECLINEWTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorLineStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_TERRAINLINESTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainLineStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_GRAPHOUTLINESTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphOutlineStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_LAUNCHILLUSTRATOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&LaunchIllustrator, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}

					case WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorGridColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertGridColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorTicColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertTicColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorLabelColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorLabelColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&HorLabelColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertLabelColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertLabelColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertLabelColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphOutlineColor[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphOutlineColor[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphOutlineColor[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_VECPROFEXPORT_PREFERREDFONT:
						{
						BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						TempStr[255] = 0;
						strcpy(PreferredFont, TempStr);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while

return (TotalRead);

} // VectorExportData::Load()

/*===========================================================================*/

ULONG Project::InterPrefs_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
short Count = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PROJECTFILE_INTERPREFS_FOLLOWTERRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->FollowTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_GRIDSAMPLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->GridSample, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (Interactive->GridSample < 50000)
							Interactive->GridSample = 50000;
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_TFXPREVIEW:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->TfxPreview, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_TFXREALTIME:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->TfxRealtime, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_ROTATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Rotate, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PRESERVEXY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.PreserveXY, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_CONSIDERELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ConsiderElev, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PTRELATIVE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.PtRelative, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PTOPERATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.PtOperate, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_TOPOCONFORM:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.TopoConform, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_INSERTSPLINE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.InsertSpline, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_REFCONTROL:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.RefControl, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_HORUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.HorUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						Interactive->VE.SetHUnitScale();
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_VERTUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.VertUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						Interactive->VE.SetVUnitScale();
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_ROTATEAMT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.RotateAmt, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.Smoothing, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_X], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_Y], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_Z], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDFLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.ProjRefCoordsFloating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_INTERPPTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.InterpPts, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_INSERTPTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->VE.InsertPts, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJFRAMERATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->ProjFrameRate, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_DIGITIZEDRAWMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->DigitizeDrawMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_MAPVIEW_POINTSMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Interactive->EditPointsMode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PROJECTFILE_INTERPREFS_SETTINGS_GRIDOVERLAYSIZE:
						{
						BytesRead = Interactive->GridOverlaySize.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // Project::InterPrefs_Load()

/*===========================================================================*/

int Project::Save(RasterAnimHostProperties *fFileSupplied, char *SaveName, Database *DB, EffectsLib *Effects, ImageLib *Images, struct ProjectIODetail *Detail, unsigned long savecode)
{
#ifndef WCS_BUILD_DEMO
FILE *fproject = NULL;
struct ProjectIODetail *LocalDetail;
unsigned long ItemTag, ByteOrder = 0xaabbccdd, TotalWritten = 0;
long BytesWritten, Result;
#ifdef DATABASE_CHUNK_DEBUG
long fTale;
#endif // DATABASE_CHUNK_DEBUG
short PathsSaved = 0, DisplayName = 0;
char filename[512], FileType[12], StrBuf[12], Ptrn[32], NameBackup[64], PathBackup[256];
char Version = WCS_PROJECT_CURRENT_VERSION;
char Revision = WCS_PROJECT_CURRENT_REVISION;
time_t StartTime, EndTime, ElapsedTime;
#endif // !WCS_BUILD_DEMO

#ifdef WCS_BUILD_DEMO

UserMessageDemo("Project files and preferences cannot be saved.");
return (0);

#else // !WCS_BUILD_DEMO
if (fFileSupplied)
	{
	fproject = fFileSupplied->fFile;
	strcpy(filename, fFileSupplied->Path);
	} // if
else
	{
	if (SaveName)
		{
		if (! ProjectLoaded)
			return (0);
		strcpy(filename, SaveName);
		} // if name provided, as in prefs file 
	else
		{
		strcpy(StrBuf, "Database");
		LocalDetail = NULL;
		if (Detail && (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
			{
			if (! stricmp(projectpath, dbasepath))
				strcpy(dbasepath, dirname);
			strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("db"));
			if (! strnicmp(&dbasename[strlen(dbasename) - 5], ".proj", 5))
				{
				dbasename[strlen(dbasename) - 5] = 0;
				strcat(dbasename, ".db");
				} // if trying to save database over project file
			if (! GetFileNamePtrn(1, "Save Database", dbasepath, dbasename, Ptrn, 32))
				return (0);
			strmfp(filename, dbasepath, dbasename);
			if (!FindFileExtension(dbasename))
				{
				strcat(dbasename, ".db");
				strcat(filename, ".db");
				} // if
			goto ReadyToSave;
			} // if load database
		// last resort - save project file
		strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("proj"));
		strcpy(PathBackup, projectpath);
		strcpy(NameBackup, projectname);
	GetNewName:
		if (! GetFileNamePtrn(1, "Save Project", projectpath, projectname, Ptrn, 64))
			return (0);
		if ((! FindFileExtension(projectname)) && strlen(projectname) < (63 - 5))	// 63 = projectname - NULL byte, 5 = extension length
			strcat(projectname, ".proj");
		strmfp(filename, projectpath, projectname);
		if (fproject = PROJ_fopen(filename, "rb"))
			{
			fclose(fproject);
			fproject = NULL;
			if ((Result = UserMessageCustom(projectname, "A Project file of this name already exists. Do you wish to overwrite it?", "Overwrite", "Cancel", "Select New Name", 1)) == 0)
				{
				strcpy(projectpath, PathBackup);
				strcpy(projectname, NameBackup);
				return (0);
				} // if
			if (Result == 2)
				{
				strcpy(projectpath, PathBackup);
				strcpy(projectname, NameBackup);
				goto GetNewName;
				} // if
			} // if
		} // else 
	} // else no file provided

ReadyToSave:

// DIRE WARNING!!
if (Detail && ! strnicmp(&filename[strlen(filename) - 5], ".proj", 5))
	{
	if (! UserMessageOKCAN("WARNING!", "You are about to overwrite your Project file with a partial data set.\nAre you sure you want to?", 1))
		return (0);
	} // if trying to save partial file over project file

if (! fproject && ((fproject = PROJ_fopen(filename, "wb")) == NULL))
	{
	if (! SaveName)
		{
		if (this == GlobalApp->MainProj)
			UserMessageOK(APP_TLA" Project: Save", "Can't open output file!\nOperation terminated.");
		} // if
	return (0);
	} // if open fail 
GetTime(StartTime);	// in Useful.cpp

// if we are going to save the database, give each Joe a unique ID for linking to scenarios
memset(StrBuf, 0, 9);
strcpy(StrBuf, "Database");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	DB->EnumerateUniqueLoadSaveIDs(0);
	} // if

// write header if file pointer not passed to this function
// That is so that if it is a component file, the header is not written twice
if (! fFileSupplied)
	{
	strcpy(FileType, "WCS File");

	// no tags or sizes for first four items: file descriptor, version, revision & byte order
	if ((BytesWritten = WriteBlock(fproject, (char *)FileType,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(fproject, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(fproject, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(fproject, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

memset(StrBuf, 0, 9);
strcpy(StrBuf, "Summary");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Summary_Save(fproject, GlobalApp->AppEffects))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Summary saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save Summary

// password gets saved only if it is a prefs file
if (Prefs.CurrentPassword[0] && strlen(filename) > 6 && ! strnicmp(&filename[strlen(filename) - 6], ".prefs", 6))
	{
	memset(StrBuf, 0, 9);
	strcpy(StrBuf, "Password");
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Password_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Effects saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written
	else
		goto WriteError;
	} // if password

// write browse data chunk only if project is not being saved as a component file
// which already has the browse chunk in it
if (! fFileSupplied)
	{
	// save BrowseData
	if (BrowseInfo)
		{
		strcpy(StrBuf, "Browse");
		if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = BrowseInfo->Save(fproject))
					{
					TotalWritten += BytesWritten;
					fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(fproject, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(fproject, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if Browse Data saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if

memset(StrBuf, 0, 9);
strcpy(StrBuf, "Paths");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Paths_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					PathsSaved = 1;
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Paths saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save paths

memset(StrBuf, 0, 9);
strcpy(StrBuf, "Config");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GUIConfig_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if GUI configuration saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save GUI configuration

memset(StrBuf, 0, 9);
strcpy(StrBuf, "UsrConfg");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = UserGUIConfig_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
				fTale = ftell(fproject);
				sprintf(debugStr, "Project::Save(UsrConfg: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if GUI configuration saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save GUI configuration

memset(StrBuf, 0, 9);
strcpy(StrBuf, "Prefs");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Prefs_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
				fTale = ftell(fproject);
				sprintf(debugStr, "Project::Save(Prefs: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					PathsSaved = 1;
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Paths saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save prefs

#ifdef WCS_BUILD_VNS
memset(StrBuf, 0, 9);
strcpy(StrBuf, "Template");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Templates_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
					fTale = ftell(fproject);
					sprintf(debugStr, "Project::Save(Template: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					PathsSaved = 1;
					} // if wrote size of block 
				else
					goto WriteError;
				} // if template names saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save template names
#endif // WCS_BUILD_VNS

if (Images)
	{
	memset(StrBuf, 0, 9);
	strcpy(StrBuf, "Images");
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
		{
		if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Images->Save(fproject, LocalDetail ? LocalDetail->Detail: NULL))
					{
					TotalWritten += BytesWritten;
					DisplayName = 1;
					fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
					fTale = ftell(fproject);
					sprintf(debugStr, "Project::Save(Images: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
					if (WriteBlock(fproject, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(fproject, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if Effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if save Images
	} // if Images

if (Effects)
	{
	memset(StrBuf, 0, 9);
	strcpy(StrBuf, "Effects");
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
		{
		if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Effects->Save(fproject, LocalDetail ? LocalDetail->Detail: NULL))
					{
					TotalWritten += BytesWritten;
					DisplayName = 1;
					fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
					fTale = ftell(fproject);
					sprintf(debugStr, "Project::Save(Effects: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
					if (WriteBlock(fproject, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(fproject, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if Effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save Effects
	} // if Effects

if (DB)
	{
	memset(StrBuf, 0, 9);
	strcpy(StrBuf, "Database");
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
		{
		if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = DB->SaveV2(fproject, savecode))
					{
					TotalWritten += BytesWritten;
					DisplayName = 1;
					fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
					fTale = ftell(fproject);
					sprintf(debugStr, "Project::Save(Database: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
					if (WriteBlock(fproject, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(fproject, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if Database saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save database
	} // if DB

memset(StrBuf, 0, 9);
strcpy(StrBuf, "ImportOp");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ImportOps_Save(fproject))
				{
				TotalWritten += BytesWritten;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
				fTale = ftell(fproject);
				sprintf(debugStr, "Project::Save(ImportOp: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Import Ops saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save Import Ops

memset(StrBuf, 0, 9);
strcpy(StrBuf, "ViewInit");
LocalDetail = NULL;
if (! Detail || (LocalDetail = ProjectIODetailSearch(Detail, StrBuf)))
	{
	if (BytesWritten = WriteBlock(fproject, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(fproject, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->GUIWins->CVG->Save(fproject))
				{
				TotalWritten += BytesWritten;
				DisplayName = 1;
				fseek(fproject, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
#ifdef DATABASE_CHUNK_DEBUG
				fTale = ftell(fproject);
				sprintf(debugStr, "Project::Save(ViewInit: Seek = %u, BytesWritten = %u\n", fTale, BytesWritten);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (WriteBlock(fproject, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(fproject, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Database saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if save database

if (! fFileSupplied)
	fclose(fproject);

if (Detail && (Detail->Flags & WCS_PROJECT_IODETAILFLAG_DESTROY))
	ProjectIODetail_Del(Detail);

if (PathsSaved)
	{
	int NamePtr;
	NamePtr = (int)(strlen(filename) - 1);
	while (NamePtr >= 0 && filename[NamePtr] != ':' && filename[NamePtr] != '\\' && filename[NamePtr] != '/')
		NamePtr --; 
	UpdateRecentProjectList();
	if (this == GlobalApp->MainProj)
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_PROJ_SAVE, &filename[NamePtr + 1]);
		SetParam(1, WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff, 0, NULL);
		} // if
	} // if

if (DisplayName && this == GlobalApp->MainProj)
	{
	GlobalApp->WinSys->UpdateRootTitle(projectname, Prefs.CurrentUserName);
	} // if
GetTime(EndTime);	// in Useful.cpp
ElapsedTime = EndTime - StartTime;

if(ElapsedTime > 0)
	{
	char ElapsedTimeMsg[512];
	sprintf(ElapsedTimeMsg, "Elapsed save time: %d seconds.", (unsigned long)ElapsedTime);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ElapsedTimeMsg);
	} // if

return (1);

WriteError:

if (! fFileSupplied)
	fclose(fproject);

if (Detail && (Detail->Flags & WCS_PROJECT_IODETAILFLAG_DESTROY))
	ProjectIODetail_Del(Detail);

if (this == GlobalApp->MainProj)
	UserMessageOK("Project: Save", "An error occurred saving the file. Not all data was saved correctly.");

return (0);

#endif // !WCS_BUILD_DEMO

} // Project::Save()

/*===========================================================================*/

ULONG Project::Summary_Save(FILE *ffile, EffectsLib *Effects)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
RenderJob *CurJob;
ImageOutputEvent *CurEvent;
char OutFileName[512];

BytesWritten = fprintf(ffile, "WCSProjects=\"%s\"\n", pcprojectpath);
TotalWritten += BytesWritten;

BytesWritten = fprintf(ffile, "WCSFrames=\"%s\"\n", pcframespath);
TotalWritten += BytesWritten;

BytesWritten = fprintf(ffile, "WCSContent=\"%s\"\n", contentpath);
TotalWritten += BytesWritten;

// other env vars
char *ConfigStr;
int ConfigLoop /*, ArgLoop */;

for (ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (Prefs.ConfigOptions[ConfigLoop][0] != NULL)
		{
		ConfigStr = Prefs.ConfigOptions[ConfigLoop];
		BytesWritten = fprintf(ffile, "%s\n", ConfigStr);
		TotalWritten += BytesWritten;
/*
		for (ArgLoop = 0; ConfigStr[ArgLoop]; ArgLoop++)
			{
			if ((ConfigStr[ArgLoop] == '=') || (ConfigStr[ArgLoop] == ':'))
				{
				Result = &ConfigStr[ArgLoop + 1];
				return(Result);
				} // if
			} // for
*/
		//Result = &ConfigStr[ArgLoop]; // point to NULL at end of string for safe non-zero return
		} // if
	} // for

// blank line
BytesWritten = fprintf(ffile, "\n");
TotalWritten += BytesWritten;


CurJob = (RenderJob *)Effects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB);
while (CurJob)
	{
	if (CurJob->Cam && CurJob->Options)
		{
		BytesWritten = fprintf(ffile, "\"%s\",\"%s\",\"%s\",%01d,%01d,", CurJob->GetName(), CurJob->Cam->GetName(), CurJob->Options->GetName(), CurJob->Priority, CurJob->Enabled);
		TotalWritten += BytesWritten;

		BytesWritten = fprintf(ffile, "%f,%01d,%01d,%01d,%f,%f,%01d,%01d,%01d\n", 
			CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue,
			CurJob->Options->FrameStep,
			(long)(CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5),
			(long)(CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5),
			CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue,
			CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue,
			CurJob->Options->OutputImageWidth,
			CurJob->Options->OutputImageHeight,
			CurJob->Options->RenderImageSegments);
		TotalWritten += BytesWritten;

		for (CurEvent = CurJob->Options->OutputEvents; CurEvent; CurEvent = CurEvent->Next)
			{
			strmfp(OutFileName, CurEvent->PAF.GetPath(), CurEvent->PAF.GetName());
			BytesWritten = fprintf(ffile, "\t\"%s\",%01d,\"%s\",%01d,\"%s\",\"%s\",%01d\n", 
				CurEvent->FileType,
				CurEvent->Enabled,
				OutFileName,
				CurEvent->AutoDigits,
				CurEvent->AutoExtension ? ImageSaverLibrary::GetDefaultExtension(CurEvent->FileType): "",
				CurJob->Options->TempPath.GetPath(),
				CurEvent->BeforePost);
			TotalWritten += BytesWritten;
			} // for
		} // if
	CurJob = (RenderJob *)CurJob->Next;
	} // while

BytesWritten = fprintf(ffile, "\n"); 
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::Summary_Save

/*===========================================================================*/

ULONG Project::Paths_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

// don't write these unless they have values in them so they won't overwrite with 0 when loaded
// This is for templates saved as templates to allow portablity of the template
if (pcprojectpath[0])
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PP_PTH, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(pcprojectpath) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)pcprojectpath)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (pcframespath[0])
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PF_PTH, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(pcframespath) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)pcframespath)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (contentpath[0])
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_CN_PTH, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(contentpath) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)contentpath)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (LastProject[0][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV1, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[0]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (LastProject[1][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV2, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[1]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[1])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (LastProject[2][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV3, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[2]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[2])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (LastProject[3][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV4, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[3]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[3])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (LastProject[4][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV5, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[4]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[4])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (LastProject[5][0])
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PROJECTFILE_PATH_PREV6, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LastProject[5]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)LastProject[5])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PJ_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(projectpath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)projectpath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PJ_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(projectname) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)projectname)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DB_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(dbasepath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)dbasepath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DB_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(dbasename) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)dbasename)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PR_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(parampath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)parampath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_PR_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(paramfile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)paramfile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_FR_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(framepath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)framepath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_FR_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(framefile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)framefile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_ZS_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(savezbufpath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)savezbufpath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
// remove this block for V4
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_ZS_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(savezbuffile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)savezbuffile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DR_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(dirname) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)dirname)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_TM_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(temppath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)temppath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DF_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(deformpath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)deformpath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DF_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(deformfile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)deformfile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_IM_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(imagepath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)imagepath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_AN_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(animpath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)animpath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_AN_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(animfile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)animfile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_AO_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(altobjectpath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)altobjectpath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_ID_PTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(importdatapath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)importdatapath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_UN_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(UserName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)UserName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_UE_NME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(UserEmail) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)UserEmail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (DL)
	{
	int Count = 0;
	short ReadOnly;
	struct DirList *DLItem = DL;

	while (DLItem)
		{
		ItemTag = Count ? WCS_PROJECTFILE_PATH_DL_ADD: WCS_PROJECTFILE_PATH_DL_NEW;

		if (DLItem->Read == '*')
			ReadOnly = 1;
		else
			ReadOnly = 0;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PATH_DL_RDONLY, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&ReadOnly)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, ItemTag, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DLItem->Name) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)DLItem->Name)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		DLItem = DLItem->Next;
		Count ++;
		} // while
	} // if DirList

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::Paths_Save()

/*===========================================================================*/

ULONG Project::GUIConfig_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_PROJECTFILE_GUICONFIG_DIMENSIONS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GUIDimensions_Save(ffile, FenTrack, FenTrackSize))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if GUI dimensions saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;


ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::GUIConfig_Save()

/*===========================================================================*/

ULONG Project::UserGUIConfig_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
short ConfigsToWrite = WCS_MAX_USER_WINDOWCONFIG, ConfigNumber = 0;

// write the number of configurations
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_USERGUICONFIG_NUMCONFIGS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ConfigsToWrite)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

// write each configuration

for (ConfigNumber = 0; ConfigNumber < ConfigsToWrite; ConfigNumber ++)
	{
	ItemTag = WCS_PROJECTFILE_GUICONFIG_DIMENSIONS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GUIDimensions_Save(ffile, AltFenTrack[ConfigNumber], AltFenTrackSize[ConfigNumber]))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if GUI dimensions saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::UserGUIConfig_Save()

/*===========================================================================*/

ULONG Project::Templates_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;

for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (ProjectTemplates[Ct].PAF.GetPath()[0] && ProjectTemplates[Ct].PAF.GetName()[0])
		{
		ItemTag = WCS_PROJECTFILE_TEMPLATES_PROJECTTEMPLATE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ProjectTemplates[Ct].Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if template saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::Templates_Save()

/*===========================================================================*/

ULONG Project::ImportOps_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
Pier1 *CurPier = Imports;

while (CurPier)
	{
	if (! CurPier->TemplateItem)
		{
		ItemTag = WCS_PROJECTFILE_IMPORTOPS_PIER1 + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CurPier->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if pier1 saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	CurPier = CurPier->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::ImportOps_Save()

/*===========================================================================*/

ULONG Project::GUIDimensions_Save(FILE *ffile, ProjWinInfo *FT, unsigned short FTSize)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Search;

if (FT && FTSize)
	{
	for (Search = 0; Search < FTSize; Search++)
		{
		if (FT[Search].WinID == 0)
			{
			break;
			} // if

#ifdef WCS_BUILD_FRANK
		//assert(FT[Search].X >= 0);
#endif // WCS_BUILD_FRANK

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_FLAGS, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&FT[Search].Flags)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_TOP, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&FT[Search].Y)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_LEFT, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&FT[Search].X)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_WIDTH, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&FT[Search].W)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_HEIGHT, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&FT[Search].H)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_PREFERREDCELL, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_CHAR, (char *)FT[Search].PreferredCell)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		// this needs to be last cuz it will trigger SetWindowCell when it is read
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_GUICONFIG_WINID, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&FT[Search].WinID)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		} // for
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Param::GUIDimensions_Save()

/*===========================================================================*/

ULONG Project::Prefs_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_PROJECTFILE_PREFS_PROJECT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ProjectPrefs_Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if GUI dimensions saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PROJECTFILE_PREFS_INTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = InterPrefs_Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if GUI dimensions saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::Prefs_Save()

/*===========================================================================*/

ULONG Project::ProjectPrefs_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0, Ct;
long BytesWritten, NumViewEnabled = WCS_VIEWGUI_ENABLE_MAX, EnabledCt;
char CurViewType;

/*
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LOGERR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ReportMesg[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LOGWNG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ReportMesg[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LOGMSG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ReportMesg[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LOGDTA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ReportMesg[3])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
*/
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_RENDERPRI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.RenderTaskPri)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_RENDERSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.RenderSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LOADONOPEN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.LoadOnOpen)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_OPENWINDOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.OpenWindows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_PROJSHOWICONTOOLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ProjShowIconTools)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_PROJSHOWANIMTOOLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ProjShowAnimTools)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_HORIZDISPLAYUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.HorDisplayUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VERTDISPLAYUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.VertDisplayUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_ANGLEDISPLAYUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.AngleDisplayUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_TIMEDISPLAYUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.TimeDisplayUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_POSLONHEMISPHERE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.PosLonHemisphere)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LATLONSIGNDISPLAY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.LatLonSignDisplay)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_SIGNIFICANTDIGITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.SignificantDigits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_TASKMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.TaskMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_ENABLEDFILTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.EnabledFilter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_ANIMATEDFILTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.AnimatedFilter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_GUICONFIGURATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.GUIConfiguration)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_SAGEXPANDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.SAGExpanded)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_MATRIXTASKMODEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.MatrixTaskModeEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_MEMORYLIMITSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.MemoryLimitsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_GLOBALADVANCEDENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.GlobalAdvancedEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_SAGBOTTOMHTPCT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.SAGBottomHtPct)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_INTERSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.InteractiveStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

// <<<>>> TIME64 issue
// we need to read/write this as 64-bit for future-proofing
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LASTUPDATEDATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.LastUpdateDate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_SHOWDBBYLAYER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.ShowDBbyLayer)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_MAXSAGDBENTRIES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.MaxSAGDBEntries)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_MAXSORTEDSAGDBENTRIES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.MaxSortedSAGDBEntries)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VECPOLYMEMORYLIMIT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.VecPolyMemoryLimit)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DEMMEMORYLIMIT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.DEMMemoryLimit)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

// RecordMode is currently unused and may be retired eventually
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_RECORDMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.RecordMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_INTERACTIVEMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.InteractiveMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_KEYGROUPMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.KeyGroupMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_GEOUNITSPROJECTED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DisplayGeoUnitsProjected)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_GLOBALSTARTUPPAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Prefs.GlobalStartupPage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

#ifdef WCS_BUILD_VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_MULTIUSERMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.MultiUserMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_NEWPROJUSETEMPLATES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.NewProjUseTemplates)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_CURRENTUSERNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.CurrentUserName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Prefs.CurrentUserName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#endif // WCS_BUILD_VNS

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_LASTSWATCH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.LastColorSwatch) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Prefs.LastColorSwatch)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; Ct ++)
	{
	if (Prefs.ConfigOptions[Ct][0])
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_CONFIGOPTION, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.ConfigOptions[Ct]) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)Prefs.ConfigOptions[Ct])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if a string in the field
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBENUMFIXEDCOLUMNS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBENumFixedColumns)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < Prefs.DBENumFixedColumns; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBEFIXEDCOLUMNWIDTHS, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBEFixedColumnWidths[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBENUMLAYERCOLUMNS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBENumLayerColumns)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < Prefs.DBENumLayerColumns; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBELAYERCOLUMNWIDTHS, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBELayerColumnWidths[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBENUMATTRIBCOLUMNS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBENumAttribColumns)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < Prefs.DBENumAttribColumns; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBEATTRIBCOLUMNWIDTHS, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (char *)&Prefs.DBEAttribColumnWidths[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBEFILTERMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Prefs.DBEFilterMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBESEARCHQUERYFILTERNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.DBESearchQueryFilterName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Prefs.DBESearchQueryFilterName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_DBELAYERFILTERNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.DBELayerFilterName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Prefs.DBELayerFilterName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

#ifdef WCS_BUILD_VNS
for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (Prefs.DefaultTemplates[Ct].PAF.GetPath()[0] && Prefs.DefaultTemplates[Ct].PAF.GetName()[0])
		{
		ItemTag = WCS_PROJECTFILE_PROJPREFS_DEFAULTTEMPLATE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Prefs.DefaultTemplates[Ct].Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if anim param saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // for
#endif // WCS_BUILD_VNS

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_NUMENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumViewEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (CurViewType = 0; CurViewType < WCS_VIEWGUI_VIEWTYPE_MAX; CurViewType ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_VIEWTYPE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&CurViewType)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	for (EnabledCt = 0; EnabledCt < NumViewEnabled; EnabledCt ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_ENABLED, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&Prefs.ViewEnabled[CurViewType][EnabledCt])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_VIEWCONTEXT_CONTOURINT, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Prefs.ViewContOverlayInt[CurViewType])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

#ifdef WCS_BUILD_VNS
ItemTag = WCS_PROJECTFILE_PROJPREFS_VECPROFEXPORT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Prefs.VecExpData.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if VectorExportData saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Param::ProjectPrefs_Save()

/*===========================================================================*/

ULONG Project::Password_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_PROJPREFS_CURRENTPASSWORD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prefs.CurrentPassword) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Prefs.CurrentPassword)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Project::Password_Save

/*===========================================================================*/

ULONG VectorExportData::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorGridInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertGridInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorTicInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertTicInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORLABELINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorLabelInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertLabelInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VECLINEWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VectorLineWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_TERRAINLINEWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TerrainLineWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorGridWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertGridWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorTicWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertTicWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_GRAPHOUTLINEWT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&GraphOutlineWeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICLEN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HorTicLength)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICLEN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertTicLength)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_LAYOUTUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LayoutUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORSCALELABEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorScaleLabel)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTSCALELABEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertScaleLabel)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWVECTOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawVector)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWTERRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORGRID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawHorGrid)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTGRID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawVertGrid)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORTICS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawHorTics)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTTICS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawVertTics)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWHORLABELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawHorLabels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWVERTLABELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawVertLabels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_DRAWGRAPHOUTLINE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawGraphOutline)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VECLINEWTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorLineStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_TERRAINLINESTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainLineStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorGridStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertGridStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_GRAPHOUTLINESTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GraphOutlineStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_LAUNCHILLUSTRATOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LaunchIllustrator)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VECCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_TERRAINCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorGridColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorGridColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORGRIDCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorGridColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertGridColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertGridColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTGRIDCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertGridColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorTicColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorTicColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORTICCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorTicColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertTicColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertTicColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTTICCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertTicColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorLabelColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorLabelColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_HORLABELCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HorLabelColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertLabelColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertLabelColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_VERTLABELCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertLabelColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GraphOutlineColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GraphOutlineColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_OUTLINECOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GraphOutlineColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_VECPROFEXPORT_PREFERREDFONT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(PreferredFont) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)PreferredFont)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PROJECTFILE_VECPROFEXPORT_PATHANDFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PAF.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if PathAndFile saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // VectorExportData::Save

/*===========================================================================*/

ULONG Project::InterPrefs_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_FOLLOWTERRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Interactive->FollowTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_TFXPREVIEW, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Interactive->TfxPreview)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_TFXREALTIME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Interactive->TfxRealtime)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_DIGITIZEDRAWMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->DigitizeDrawMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_MAPVIEW_POINTSMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->EditPointsMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_GRIDSAMPLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Interactive->GridSample)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJFRAMERATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->ProjFrameRate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Scale[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Shift[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_ROTATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Rotate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PRESERVEXY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.PreserveXY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.Smooth[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_CONSIDERELEV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.ConsiderElev)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PTRELATIVE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.PtRelative)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PTOPERATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.PtOperate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_TOPOCONFORM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.TopoConform)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_INSERTSPLINE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.InsertSpline)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_REFCONTROL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.RefControl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_HORUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.HorUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_VERTUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.VertUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SCALEAMTZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ScaleAmt[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SHIFTAMTZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ShiftAmt[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_ROTATEAMT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.RotateAmt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_SMOOTHING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.Smoothing)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_ARBREFCOORDZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ArbRefCoord[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_X])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_Y])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Interactive->VE.ProjRefCoord[WCS_INTERVEC_COMP_Z])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_PROJREFCOORDFLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Interactive->VE.ProjRefCoordsFloating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_INTERPPTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Interactive->VE.InterpPts)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PROJECTFILE_INTERPREFS_SETTINGS_INSERTPTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Interactive->VE.InsertPts)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PROJECTFILE_INTERPREFS_SETTINGS_GRIDOVERLAYSIZE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Interactive->GridOverlaySize.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if grid size saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Param::InterPrefs_Save()

/*===========================================================================*/
/*===========================================================================*/

// DirList stuff

struct DirList *Project::DirList_New(char *firstpath, short ReadOnly)
{
struct DirList *DLNew;

if ((DLNew = (struct DirList *)AppMem_Alloc(sizeof (struct DirList), APPMEM_CLEAR)) != NULL)
	{
	strcpy(DLNew->Name, firstpath);
	if (ReadOnly)
		DLNew->Read = '*';
	else
		DLNew->Read = ' ';
	} // if memory OK 

return (DLNew);
 
} // Project::DirList_New() 

/*===========================================================================*/

struct DirList *Project::DirList_Append(struct DirList *DLOld, struct DirList *DLNew)
{
struct DirList *DLCur;

if (DLCur = DLOld)
	{
	while (DLCur->Next)
		DLCur = DLCur->Next;
	DLCur->Next = DLNew;
	} // if
else
	DLOld = DLNew;

// returns head of the list
return (DLOld);

} // Project::DirList_Append

/*===========================================================================*/

struct DirList *Project::DirList_Add(struct DirList *DLOld, char *addpath, short ReadOnly)
{
struct DirList *DLNew;
char ObjPath[256], ObjFile[32];

if (! DLOld)
	{
	if (! addpath)
		{
		FileReq FR;
		FR.SetDefPath("WCSProjects:");
		FR.SetTitle("Object Directory");
		if (FR.Request(WCS_REQUESTER_FILE_DIRONLY))
			{
			ObjPath[0] = 0;
			ObjFile[0] = 0;
			BreakFileName((char *)FR.GetNextName(), ObjPath, 255, ObjFile, 31);
			} // if
		else
			{
			return (NULL);
			} // else
		} // if no path provided 
	return (DirList_New(ObjPath, ReadOnly));
	} // if

while (DLOld->Next)
	{
	DLOld = DLOld->Next;
	} // while 

if (! addpath)
	{
	FileReq FR;
	FR.SetDefPath("WCSProjects:");
	FR.SetTitle("Object Directory");
	if (FR.Request(WCS_REQUESTER_FILE_DIRONLY))
		{
		ObjPath[0] = 0;
		ObjFile[0] = 0;
		BreakFileName((char *)FR.GetNextName(), ObjPath, 255, ObjFile, 31);
		} // if
	else
		{
		return (NULL);
		} // else
	} // if no path provided 
else
	{
	strcpy(ObjPath, addpath);
	} // else path provided 

if ((DLNew = (struct DirList *)AppMem_Alloc(sizeof (struct DirList), APPMEM_CLEAR)) != NULL)
	{
	strcpy(DLNew->Name, ObjPath);
	DLOld->Next = DLNew;
	if (ReadOnly)
		DLNew->Read = '*';
	else
		DLNew->Read = ' ';
	return (DLNew);
	} // if memory OK 

return (NULL);

} // Project::DirList_Add() 

/*===========================================================================*/

struct DirList *Project::DirList_Remove(struct DirList *DLOld, short Item)
{
struct DirList *DLRem, *DLFirst;

if (! DLOld)
	return (NULL);

DLFirst = DLOld;
if (Item > 0)
	{
	DLOld = DirList_Search(DLOld, Item - 1);
	DLRem = DLOld->Next;
	DLOld->Next = DLRem->Next;
	} // if
else
	{
	DLRem = DLOld;
	if (! DLRem->Next)
		{
		return (DLFirst);
		} // only one item in list-can't be deleted 
	DLFirst = DLRem->Next;
	} // else

AppMem_Free(DLRem, sizeof (struct DirList));

return (DLFirst);

} // Project::DirList_Remove() 

/*===========================================================================*/

struct DirList *Project::DirList_Search(struct DirList *DLItem, short Item)
{
short i;

if (! DLItem)
	return (NULL);

for (i=0; i<Item; i++)
	{
	DLItem = DLItem->Next;
	} // for i

return (DLItem);

} // Project::DirList_Search() 

/*===========================================================================*/

short Project::DirList_ItemExists(struct DirList *DLItem, char *Item)
{
short found = 0;

while (DLItem)
	{
	if (! strcmp(DLItem->Name, Item))
		{
		found = 1;
		break;
		}
	DLItem = DLItem->Next;
	} // while 

return (found);

} // Project::DirList_ItemExists() 

/*===========================================================================*/

void Project::DirList_Del(struct DirList *DLDel)
{
struct DirList *DLNext;

while (DLDel)
	{
	DLNext = DLDel->Next;
	AppMem_Free(DLDel, sizeof (struct DirList));
	DLDel = DLNext;
	} // while 

} // Project::DirList_Del() 

/*===========================================================================*/

void Project::DirList_Move(struct DirList *DLOld, short Move, short MoveTo)
{
short i;
struct DirList *DLItem1, *DLItem2;

if (! DLOld)
	return;

if (Move == MoveTo)
	return;

if (MoveTo > Move)
	{
	DLItem1 = DirList_Search(DLOld, Move);
	DLItem2 = DLItem1->Next;
	for (i=Move; i<MoveTo-1; i++)
		{
		swmem(&DLItem1->Read, &DLItem2->Read, 256);
		DLItem1 = DLItem2;
		DLItem2 = DLItem2->Next;
		} // for
	} // if move<moveto 
else
	{
	for (i=Move; i>MoveTo; i--)
		{
		DLItem1 = DirList_Search(DLOld, i - 1);
		DLItem2 = DLItem1->Next;
		swmem(&DLItem1->Read, &DLItem2->Read, 256);
		} // for
	} // else move>moveto 

} // Project::DirList_Move() 

/*===========================================================================*/

struct DirList *Project::DirList_Copy(struct DirList *DLOld)
{
struct DirList *DLCopy, *DLItem;

if (! DLOld)
	return (NULL);

if ((DLCopy = (struct DirList *)AppMem_Alloc(sizeof (struct DirList), APPMEM_CLEAR)) != NULL)
	{
	DLItem = DLCopy;
	while (DLOld)
		{
		strcpy(&DLItem->Read, &DLOld->Read);
		DLOld = DLOld->Next;
		if (DLOld)
			{
			if ((DLItem->Next = (struct DirList *)AppMem_Alloc(sizeof (struct DirList), APPMEM_CLEAR)) != NULL)
				{
				DLItem = DLItem->Next;
				}
			else
				{
				break;
				} // else memory failed 
			} // if another to copy 
		} // while 
	} // if memory OK 

return (DLCopy);

} // Project::DirList_Copy() 

/*===========================================================================*/

struct DirList *Project::DirList_Mung(struct DirList *DLOld)
{
struct DirList *DLCur = DLOld;

while (DLCur)
	{
	strcpy(DLCur->Name, MungPath(DLCur->Name));
	DLCur = DLCur->Next;
	} // while

// returns head of the list
return (DLOld);

} // Project::DirList_Mung

/*===========================================================================*/

struct DirList *Project::DirList_UnMung(struct DirList *DLOld)
{
struct DirList *DLCur = DLOld;

while (DLCur)
	{
	strcpy(DLCur->Name, UnMungPath(DLCur->Name));
	DLCur = DLCur->Next;
	} // while

// returns head of the list
return (DLOld);

} // Project::DirList_UnMung

/*===========================================================================*/
/*===========================================================================*/

// What a hack...
char NewPath[WCS_PROJECT_MUNGTEMP_SIZE], MungTemp[2][WCS_PROJECT_MUNGTEMP_SIZE];

char *Project::MungPath(const char *Name, int WhichName)
{
char Last, PathChanged = 0;
long Letter, NeedSlash = 1;

if (Name)
	{
	strcpy(MungTemp[WhichName], Name);
	} // if
else
	{
	return(NULL);
	} // else

if (MungTemp[WhichName][0])
	{
	// Emulate assigns
	if (!strnicmp(MungTemp[WhichName], WCS_PROJECT_WCSPROJECTS, 12))
		{
		if (pcprojectpath[0])
			{
			strcpy(NewPath, pcprojectpath);
			Last = NewPath[strlen(NewPath) - 1];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			Last = MungTemp[WhichName][12];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			if (NeedSlash)
				{
				strcat(NewPath, "/");
				} // if
			strcat(NewPath, &MungTemp[WhichName][12]);
			strcpy(MungTemp[WhichName], NewPath);
			PathChanged = 1;
			} // if
		} // if
	else if (!strnicmp(MungTemp[WhichName], WCS_PROJECT_WCSFRAMES, 10))
		{
		if (pcframespath[0])
			{
			strcpy(NewPath, pcframespath);
			Last = NewPath[strlen(NewPath) - 1];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			Last = MungTemp[WhichName][10];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			if (NeedSlash)
				{
				strcat(NewPath, "/");
				} // if
			strcat(NewPath, &MungTemp[WhichName][10]);
			strcpy(MungTemp[WhichName], NewPath);
			PathChanged = 1;
			} // if
		} // if
	else if (!strnicmp(MungTemp[WhichName], WCS_PROJECT_WCSCONTENT, 10))
		{
		if (contentpath[0])
			{
			strcpy(NewPath, contentpath);
			Last = NewPath[strlen(NewPath) - 1];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			Last = MungTemp[WhichName][11];
			if ((Last == ':') || (Last == '/') || (Last == '\\'))
				{
				NeedSlash = 0;
				} // if
			if (NeedSlash)
				{
				strcat(NewPath, "/");
				} // if
			strcat(NewPath, &MungTemp[WhichName][11]);
			strcpy(MungTemp[WhichName], NewPath);
			PathChanged = 1;
			} // if
		} // if
	if (PathChanged)
		{
		for (Letter = 0; MungTemp[WhichName][Letter]; Letter++)
			{
 			#ifdef _WIN32
  			if (MungTemp[WhichName][Letter] == '/')
  				{
  				MungTemp[WhichName][Letter] = '\\';
  				} // if

 			#endif // _WIN32
			} // for
		} // if
	} // if

return(MungTemp[WhichName]);
} // Project::MungPath

/*===========================================================================*/

char *Project::UnMungPath(char *Name)
{
int Len;

if (this)
	{
	Len = (int)strlen(pcprojectpath);
	if (Len && (!strnicmp(Name, pcprojectpath, Len)))
		{
		strcpy(MungTemp[0], WCS_PROJECT_WCSPROJECTS);
		if (Name[Len])
			{
			strcat(MungTemp[0], &Name[Len]);
			} // if
		return(MungTemp[0]);
		} // if
	Len = (int)strlen(pcframespath);
	if (Len && (!strnicmp(Name, pcframespath, Len)))
		{

		strcpy(MungTemp[0], WCS_PROJECT_WCSFRAMES);
		if (Name[Len])
			{
			strcat(MungTemp[0], &Name[Len]);
			} // if
		return(MungTemp[0]);
		} // if
	Len = (int)strlen(contentpath);
	if (Len && (!strnicmp(Name, contentpath, Len)))
		{

		strcpy(MungTemp[0], WCS_PROJECT_WCSCONTENT);
		if (Name[Len])
			{
			strcat(MungTemp[0], &Name[Len]);
			} // if
		return(MungTemp[0]);
		} // if
	if (Name)
		{
		strcpy(MungTemp[0], Name);
		} // if
	else
		MungTemp[0][0] = 0;	// with windoze file requester this case can't happen - I think - GRH

	return(MungTemp[0]);
	} // if

return(Name);

} // Project::UnMungPath

/*===========================================================================*/

char *Project::UnMungNulledPath(char *Name)
{
int Len, FullLen;

if (Name)
	{
	for (FullLen = 0; Name[FullLen] || Name[FullLen + 1]; FullLen++)
		{
		// Nothing to do but count.
		} // for
	FullLen += 2; // Two NULLs
	} // if

if (this)
	{
	Len = (int)strlen(pcprojectpath);
	if (Len && (!strnicmp(Name, pcprojectpath, Len)))
		{
		strcpy(MungTemp[0], WCS_PROJECT_WCSPROJECTS);
		if (Name[Len])
			{
			//strcat(MungTemp[0], &Name[Len]);
			memcpy(&MungTemp[0][strlen(MungTemp[0])], &Name[Len], max(0, FullLen - Len));
			} // if
		return(MungTemp[0]);
		} // if
	Len = (int)strlen(pcframespath);
	if (Len && (!strnicmp(Name, pcframespath, Len)))
		{
		strcpy(MungTemp[0], WCS_PROJECT_WCSFRAMES);
		if (Name[Len])
			{
			//strcat(MungTemp[0], &Name[Len]);
			memcpy(&MungTemp[0][strlen(MungTemp[0])], &Name[Len], max(0, FullLen - Len));
			} // if
		return(MungTemp[0]);
		} // if
	Len = (int)strlen(contentpath);
	if (Len && (!strnicmp(Name, contentpath, Len)))
		{
		strcpy(MungTemp[0], WCS_PROJECT_WCSCONTENT);
		if (Name[Len])
			{
			//strcat(MungTemp[0], &Name[Len]);
			memcpy(&MungTemp[0][strlen(MungTemp[0])], &Name[Len], max(0, FullLen - Len));
			} // if
		return(MungTemp[0]);
		} // if
	if (Name)
		{
		//memset(MungTemp[0], 0, WCS_PROJECT_MUNGTEMP_SIZE);
		//strcpy(MungTemp[0], Name);
		strdoublenullcopy(MungTemp[0], Name);
		} // if
	else
		MungTemp[0][0] = 0;	// with windoze file requester this case can't happen - I think - GRH

	return(MungTemp[0]);
	} // if
return(Name);
} // Project::UnMungNulledPath

/*===========================================================================*/

void Project::SetFRDInvalid(short Invalid)
{

//if (GlobalApp->ActivePar)
//	GlobalApp->ActivePar->SetParam(1, WCS_PARAMCLASS_SETTING, WCS_STNG_SHORT, WCS_STNG_FRDINVALID, 0, Invalid, NULL);

} // Project::SetFRDInvalid

/*===========================================================================*/

void Project::ApplicationSetTime(double Time, long Frame, double FrameFraction)
{

// Just in case it's ever needed
// The Application gets its time value from Param, don't need to reset anything here

} // Project::ApplicationSetTime

/*===========================================================================*/

int Project::VerifyProjectLoaded(void)
{

if (! ProjectLoaded)
	{
	if (UserMessageOKCAN("Import Data", "You must create or load a Project before importing data.\nClick OK to create a New Project."))
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PNG, 0);
	return (0);
	} // if

return (1);

} // Project::VerifyProjectLoaded

/*===========================================================================*/

void Project::DeleteFromRecentProjectList(unsigned long projNum)
{
NotifyTag Changes[2];
unsigned long i, ndx;
char TempStr[512];

if ((projNum > 0) && (projNum <= 6))
	{
	ndx = projNum - 1;

	sprintf(TempStr, "Project '%s' not found.  Remove from Recent List?", LastProject[ndx]);

	if (UserMessageYN("Project", TempStr))
		{
		for (i = ndx; i < 5; ++i)
			{
			strcpy(LastProject[i], LastProject[i + 1]);
			} // for
		LastProject[5][0] = 0;

		this->SetParam(1, WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff, 0, NULL);
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if

	} // if

} // Project::DeleteFromRecentProjectList

/*===========================================================================*/

void Project::UpdateRecentProjectList(void)
{
char *TempTempStr;
int Found = 5, Ct;
char TempStr[512];

strmfp(TempStr, projectpath, projectname);
TempTempStr = GlobalApp->MainProj->MungPath(TempStr);

for (Ct = 0; Ct < 5; ++Ct)
	{
	if (! stricmp(TempTempStr, LastProject[Ct]))
		{
		Found = Ct;
		} // if
	} // for

if (Found > 0)
	{
	for (Ct = Found - 1; Ct >= 0; --Ct)
		{
		strcpy(LastProject[Ct + 1], LastProject[Ct]);
		} // for
	strcpy(LastProject[0], TempTempStr);
	} // if

} // Project::UpdateRecentProjectList

/*===========================================================================*/

Pier1 *Project::AddPier(Pier1 *NewPier)
{
Pier1 **CurPierPtr = &Imports;

while (*CurPierPtr)
	{
	CurPierPtr = &(*CurPierPtr)->Next;
	} // while

*CurPierPtr = NewPier;

return (*CurPierPtr);
	
} // Project::AddPier

/*===========================================================================*/

void Project::RemovePier(Pier1 *RemoveMe)
{
Pier1 *CurPier = Imports, *PrevPier = NULL;

while (CurPier)
	{
	if (CurPier == RemoveMe)
		{
		if (! PrevPier)
			Imports = CurPier->Next;
		else
			PrevPier->Next = CurPier->Next;
		CurPier->Next = NULL;
		delete CurPier;
		return;
		} // if
	PrevPier = CurPier;
	CurPier = CurPier->Next;
	} // while

} // Project::RemovePier

/*===========================================================================*/

void Project::ClearPiers(void)
{
Pier1 *DelPier;

while (Imports)
	{
	DelPier = Imports;
	Imports = Imports->Next;
	delete DelPier;
	} // while

} // Project::ClearPiers

/*===========================================================================*/

void Project::SetAuxAutoImportFile(char *AuxPath, char *AuxFile)
{

strcpy(AuxImportPath, AuxPath);
strcpy(AuxImportFile, AuxFile);

} // Project::SetAuxAutoImportFile

/*===========================================================================*/
/*===========================================================================*/

void Template::Clear(void)
{

PAF.SetPathAndName("", "");
Enabled = 0;
Embed = 0;
Remove = 0;

} // Template::Clear

/*===========================================================================*/

void Template::Copy(Template *CopyFrom)
{

PAF.Copy(&PAF, &CopyFrom->PAF);
Enabled = CopyFrom->Enabled;
Embed = CopyFrom->Embed;
Remove = CopyFrom->Remove;

} // Template::Copy

/*===========================================================================*/

ULONG Template::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_TEMPLATE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEMPLATE_EMBED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Embed, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEMPLATE_PATHANDFILE:
						{
						BytesRead = PAF.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // Template::Load

/*===========================================================================*/

ULONG Template::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEMPLATE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEMPLATE_EMBED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Embed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_TEMPLATE_PATHANDFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PAF.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if PAF saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

 return (0L);

} // Template::Save

/*===========================================================================*/
/*===========================================================================*/

NewProject::NewProject(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, ImageLib *ImageSource)
{

ProjHost = ProjSource;
DBHost = DBSource;
EffectsHost = EffectsSource;
ImagesHost = ImageSource;

ProjName.SetPath("WCSProjects:");
CloneName.SetPath("WCSProjects:");
Clone = CloneType = 0;
PlaceInSubDir = 1;
ProjectPathIncludesProjDir = 0;

} // NewProject::NewProject

/*===========================================================================*/

// return 1 if successful
int NewProject::Create(void)
{
char DirName[512], FileName[512], TempStr[512], ProjectName[256], BaseName[256], Ptrn[32], 
	UseExistingDirectory = 0, OverwriteFile = 0;
#ifdef WCS_BUILD_VNS
long Ct, PCt;
#endif // WCS_BUILD_VNS
FILE *ffile;
NotifyTag Changes[3];
Camera *NewCam = NULL, *NewPlanCam = NULL, *NewOverheadCam = NULL;
Light *NewLight = NULL;
GroundEffect *NewGround = NULL;
TerrainParamEffect *NewTerrainPar = NULL;
PlanetOpt *NewPlanetOpt = NULL;
EnvironmentEffect *NewEnv = NULL;
CelestialEffect *NewSun = NULL;
CelestialEffect *NewMoon = NULL;
Sky *NewSky = NULL;
Atmosphere *NewAtmo = NULL;
RenderOpt *NewOpt, *NewPreviewOpt = NULL;
RenderJob *NewJob = NULL;
#ifdef WCS_BUILD_VNS
CoordSys *NewCS = NULL;
Project *TempProj;
#endif // WCS_BUILD_VNS
struct DirList *DLCopy = NULL;

// check output path and file name to see if it exists
if (ProjectPathIncludesProjDir)
	strcpy(DirName, ProjName.GetPath());
else 
	ProjName.GetPathAndName(DirName);
if (DirName[0])
	{
	// check out the directory
	if (strlen(DirName) >= 5 && ! stricmp(&DirName[strlen(DirName) - 5], ".proj"))
		DirName[strlen(DirName) - 5] = 0;
	if (DirName[0])
		{
		if (! PROJ_chdir(DirName))	// chdir returns 0 if successful
			{
			UseExistingDirectory = 2;
			if (ProjHost == GlobalApp->MainProj)
				{
				// ask user what to do if it exists
				sprintf(TempStr, "A directory of the name %s already exists. Do you wish to use it as the new Project Directory?\nIf you answer \"No\" you will be asked to specify the name of a new Directory.", DirName);
				if (! (UseExistingDirectory = UserMessageYNCAN("New Project", TempStr)))
					return (0);
				if (UseExistingDirectory == 1)
					{
					FileReq FR;
					FR.SetDefPath(DirName);
					FR.SetTitle("Select Project Path");
					if (FR.Request(WCS_REQUESTER_FILE_DIRONLY))
						{
						DirName[0] = 0;
						TempStr[0] = 0;
						BreakFileName((char *)FR.GetNextName(), DirName, 255, TempStr, 511);
						if (! DirName[0])
							{
							return(0);
							} // if
						} // if
					else
						{
						return (0);
						} // else

					// we're gonna assume that even if the new directory already exists, they mean to use it.
					if (PROJ_chdir(DirName))
						UseExistingDirectory = 2;
					} // if need new directory
				} // if
			} // if dir exists

		if (PlaceInSubDir)
			strmfp(FileName, DirName, ProjName.GetName());
		else
			strcpy(FileName, DirName);

		// now check the file name
		if (strlen(FileName) >= 5 && stricmp(&FileName[strlen(FileName) - 5], ".proj"))
			strcat(FileName, ".proj");
		BreakFileName(FileName, TempStr, 512, ProjectName, 256);	// this doesn't destroy the full name
		if (ProjHost == GlobalApp->MainProj && (ffile = PROJ_fopen(FileName, "rb")))
			{
			fclose(ffile);
			sprintf(TempStr, "A file of the name %s already exists. Do you wish to overwrite it?\nIf you answer \"No\" you will be asked to specify the name of a new Project file.", FileName);
			if (! (OverwriteFile = UserMessageYNCAN("New Project", TempStr)))
				return (0);
			if (OverwriteFile == 1)
				{
				ProjectName[0] = 0;
				strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("proj"));
				if (! GetFileNamePtrn(1, "Select Project File", DirName, ProjectName, Ptrn, 64) || ! ProjectName[0])
					return (0);
				// we're gonna assume that even if the new file already exists, they mean to overwrite it.
				if (strlen(ProjectName) >= 5 && strlen(ProjectName) <= 63 - 5 && stricmp(&ProjectName[strlen(ProjectName) - 5], ".proj"))
					strcat(ProjectName, ".proj");
				strmfp(FileName, DirName, ProjectName);
				} // if need new directory
			} // if file exists


		// load clone project if needed
		if (! Clone)
			{
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			// clear existing project
			if (ProjHost == GlobalApp->MainProj)
				{
				GlobalApp->GUIWins->CVG->CloseViewsForIO();
				GlobalApp->MCP->CloseDangerousWindows();
				} // ifr
			EffectsHost->DeleteAll(TRUE);
			ImagesHost->DeleteAll();
			ProjHost->Interactive->ClearAllPts();
			ProjHost->ClearPiers();
			ProjHost->ClearTemplates();
			DBHost->DestroyAll();
			if (ProjHost->BrowseInfo)
				ProjHost->BrowseInfo->FreeAll();
			if (ProjHost->DL)
				{
				ProjHost->DirList_Del(ProjHost->DL);
				ProjHost->DL = NULL;
				} // if

			#ifdef WCS_BUILD_VNS
			// set templates
			if (ProjHost->Prefs.NewProjUseTemplates)
				{
				if (TempProj = new Project())
					{
					for (Ct = PCt = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
						{
						// only use enabled templates that have what appears to be a valid path
						if (ProjHost->Prefs.DefaultTemplates[Ct].Enabled)
							{
							if (ProjHost->Prefs.DefaultTemplates[Ct].PAF.GetValidPathAndName(TempStr))
								{
								ProjHost->ProjectTemplates[PCt ++].Copy(&ProjHost->Prefs.DefaultTemplates[Ct]);
								} // if
							} // if this template enabled
						} // for
					for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
						{
						if (ProjHost->ProjectTemplates[Ct].PAF.GetValidPathAndName(TempStr))
							{
							// load master paths and directory list to temporary project
							TempProj->Load(NULL, TempStr, NULL, NULL, NULL,
								TempProj->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
								WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
								WCS_PROJECT_IODETAILTAG_GROUP, WCS_PROJECT_LOAD_PATHS_FILEDIRS,
								WCS_PROJECT_IODETAILTAG_DONE),
								WCS_PROJECT_LOAD_PATHS_FILEDIRS);
							// mung directory names using master paths from template file
							TempProj->DirList_Mung(TempProj->DL);
							// make the dirlist part of the new project
							ProjHost->DL = ProjHost->DirList_Append(ProjHost->DL, TempProj->DL);
							// set back to null state for next paths load
							TempProj->DL = NULL;
							TempProj->pcprojectpath[0] = 0;
							TempProj->pcframespath[0] = 0;
							TempProj->contentpath[0] = 0;
							// load template into new project memory
							ProjHost->LoadProjectAsTemplate(TempStr, FALSE, DBHost, EffectsHost, NULL);
							// clear image IDs so they don't conflict while loading next template
							ImagesHost->ClearRasterIDs();
							} // if
						} // for
					// unmung dir list using current master paths
					ProjHost->DirList_UnMung(ProjHost->DL);
					// init image IDs so they form an unbroken sequence - not really necessary as this will be done
					// when the project is saved anyway - but it seems like good form
					ImagesHost->InitRasterIDs();
					delete TempProj;
					} // if temporary project
				} // if
			#endif // WCS_BUILD_VNS

			// create new components
			strncpy(BaseName, ProjectName, 48);
			if (strlen(BaseName) >= 5 && ! stricmp(&BaseName[strlen(BaseName) - 5], ".proj"))
				BaseName[strlen(BaseName) - 5] = 0;
			BaseName[48] = 0;
			#ifdef WCS_BUILD_VNS
			// create a new default geographic WGS84 CoordSys to banish WCS-BCS to the pit from whence it came
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_COORDSYS, 0, DBHost))
				{
				sprintf(TempStr, "%s Default WGS84 Geographic CoordSys", BaseName);
				NewCS =	(CoordSys *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, TempStr, NULL);
				if (NewCS)
					{ // set to geographic WGS-84
					NewCS->SetSystem("Geographic - WGS 84"); // Geo WGS84
					} // if
				} // if
			#endif // WCS_BUILD_VNS
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, DBHost))
				{
				sprintf(TempStr, "%s Planet Options", BaseName);
				NewPlanetOpt =	(PlanetOpt *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, TempStr, NULL);
				#ifdef WCS_BUILD_VNS
				if (NewPlanetOpt && NewCS)
					{
					NewPlanetOpt->SetCoords(NewCS); // setup the default WGS84 as the Project Default CS in the PlanetOpt
					} // if
				#endif // WCS_BUILD_VNS
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_CAMERA, 0, DBHost))
				{
				sprintf(TempStr, "%s Camera", BaseName);
				NewCam =		(Camera *)				EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, TempStr, NULL);
				sprintf(TempStr, "%s Planimetric", BaseName);
				NewPlanCam = (Camera *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, TempStr, NULL);
				sprintf(TempStr, "%s Overhead", BaseName);
				NewOverheadCam = (Camera *)				EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, TempStr, NULL);
				} // if need cameras
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_LIGHT, 0, DBHost))
				{
				sprintf(TempStr, "%s Sun", BaseName);
				NewLight =		(Light *)				EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_GROUND, 0, DBHost))
				{
				sprintf(TempStr, "%s Ground", BaseName);
				NewGround =		(GroundEffect *)		EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_GROUND, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0, DBHost))
				{
				sprintf(TempStr, "%s Terrain Param", BaseName);
				NewTerrainPar = (TerrainParamEffect *)	EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0, DBHost))
				{
				sprintf(TempStr, "%s Environment", BaseName);
				NewEnv =		(EnvironmentEffect *)	EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_CELESTIAL, 0, DBHost))
				{
				sprintf(TempStr, "%s Sun", BaseName);
				NewSun =		(CelestialEffect *)		EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CELESTIAL, TempStr, NULL);
				sprintf(TempStr, "%s Moon", BaseName);
				NewMoon =		(CelestialEffect *)		EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CELESTIAL, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_SKY, 0, DBHost))
				{
				sprintf(TempStr, "%s Sky", BaseName);
				NewSky =		(Sky *)					EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_SKY, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0, DBHost))
				{
				sprintf(TempStr, "%s Atmosphere", BaseName);
				NewAtmo =		(Atmosphere *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, TempStr, NULL);
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, 0, DBHost))
				{
				sprintf(TempStr, "%s Render Options", BaseName);
				NewOpt =		(RenderOpt *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, TempStr, NULL);
				sprintf(TempStr, "%s Preview Options", BaseName);
				NewPreviewOpt = (RenderOpt *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, TempStr, NULL);
				NewPreviewOpt->RenderDiagnosticData = 1;
				} // if
			if (! EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_RENDERJOB, 0, DBHost))
				{
				sprintf(TempStr, "%s Render Job", BaseName);
				NewJob =		(RenderJob *)			EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDERJOB, TempStr, NULL);
				} // if
			if (NewJob && NewCam && NewOpt)
				{
				NewJob->SetCamera(NewCam);
				NewJob->SetRenderOpt(NewOpt);
				} // if
			// load basic sun from component gallery
			if (NewSun)
				{
				NewSun->LoadComponentFile("WCSContent:Sky/Sun.cel", 0);	// 0 = no messages
				NewSun->Enabled = 0;
				} // if
			// load basic moon from component gallery
			if (NewMoon)
				{
				NewMoon->LoadComponentFile("WCSContent:Sky/Moon.cel", 0);	// 0 = no messages
				NewMoon->Enabled = 0;
				} // if
			if (NewSun && NewLight)
				NewSun->AddLight(NewLight);

			// float the cameras, light and project reference coords
			if (NewCam)
				NewCam->SetFloating(1);
			if (NewOverheadCam)
				{
				NewOverheadCam->CameraType = WCS_EFFECTS_CAMERATYPE_OVERHEAD;
				NewOverheadCam->SetFloating(1);
				} // if
			if (NewPlanCam)
				{
				NewPlanCam->CameraType = WCS_EFFECTS_CAMERATYPE_PLANIMETRIC;
				NewPlanCam->SetFloating(1);
				} // if
			if (NewLight)
				NewLight->SetFloating(1);

			ProjHost->Interactive->SetFloating(1, DBHost);

			// <<<>>>gh if there were a way for the user to specify which options 
			// he wanted we could load other things from the component gallery
			// such as complete environments, skies, atmospheres, render opts
			ProjHost->ProjectLoaded = 1;

			//if (GlobalApp->GUIWins->SAG)
			//	GlobalApp->GUIWins->SAG->UnFreeze();
			} // if no clone
		else if (CloneType)
			{
			if (! CloneName.GetValidPathAndName(TempStr) ||
				! ProjHost->Load(NULL, TempStr, DBHost, EffectsHost, ImagesHost, NULL, 0xffffffff))
				{
				UserMessageOK("New Project", "Unable to load the Project to clone.\nPossible reasons include invalid file path or file access permission denied.");
				return (0);
				} // else
			} // else if need to load a clone project

		// save new project
		// create directory
		if (UseExistingDirectory || ! PROJ_mkdir(DirName))
			{
			// add directory to directory list
			strcpy(ProjHost->dirname, DirName);

			if (ProjHost->DL)
				{
				if (! ProjHost->DirList_ItemExists(ProjHost->DL, DirName))
					ProjHost->DirList_Add(ProjHost->DL, DirName, 0);
				} // if
			else
				ProjHost->DL = ProjHost->DirList_New(DirName, 0);

			// save the file
			BreakFileName(FileName, ProjHost->projectpath, 256, ProjHost->projectname, 64);
#ifndef WCS_BUILD_DEMO
			ProjHost->Save(NULL, FileName, DBHost, EffectsHost, ImagesHost, NULL, 0xffffffff);
#endif // !WCS_BUILD_DEMO
			} // if
		// let the world know
		if (ProjHost == GlobalApp->MainProj)
			{
			GlobalApp->UpdateProjectByTime();
			GlobalApp->WinSys->UpdateRootTitle(ProjHost->projectname, ProjHost->Prefs.CurrentUserName);
			} // if
		ProjHost->ProjectLoaded = 1;
		ProjHost->UpdateRecentProjectList();
		ProjHost->SetParam(1, WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff, 0, NULL);
		if (ProjHost == GlobalApp->MainProj)
			GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_PROJ_LOAD, ProjHost->projectname);
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		return (1);
		} // if
	} // if path and name

UserMessageOK("New Project", "Invalid name! Project cannot be created.\nPlease change the name and try again.");
return (0);

} // NewProject::Create

/*===========================================================================*/
/*===========================================================================*/

#ifdef fopen
#undef fopen
#endif // fopen

static char OriginalPath[1024], BrokenPath[1024], BrokenName[255];
FILE *buffered_fopen(const char *a, const char *b)
{
FILE *Res;
int Success = 0;

if (Res = fopen(a, b))
	{
	Success = 1;
	} // if
else
	{ // try harder on networked paths
	strcpy(OriginalPath, a);
	BreakFileName(OriginalPath, BrokenPath, 1024, BrokenName, 255);
	if (IsNetworkPath(BrokenPath) == 1)
		{ // try to wait out a temporary network glitch
		int RetryCount = 5;
		while (!Res && RetryCount)
			{
			if (Res = fopen(a, b))
				{
				break;
				} // if
			else
				{
				RetryCount--;
				Sleep(50); // kill a bit of time
				} // else
			} // while;
		} // if
	} // else

if (Success)
	{
	setvbuf(Res, NULL, _IOFBF, SETVBUF_BUFFER_SIZE);
	} // if

return(Res);
} // buffered_fopen
