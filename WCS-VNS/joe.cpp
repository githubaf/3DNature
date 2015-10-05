// Joe.cpp
// Source for "Joe" database object
// written from DBNotes file on 3/27/95 by Chris "Xenon" Hanson
// Effects and Effect attributes added 5/97 by Gary R. Huber
// Copyright 1995 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Project.h"
#include "Database.h"
#include "Joe.h"
#include "Requester.h"
#include "Points.h"
#include "Useful.h"
#include "AppMem.h"
#include "Layers.h"
#include "MathSupport.h"
#include "EffectsLib.h"
#include "Toolbar.h"
#include "DEM.h"
#include "Conservatory.h"
#include "DBEditGUI.h"
#include "DEMEditGUI.h"
#include "PathTransferGUI.h"
#include "VecProfExportGUI.h"
#include "DEMPaintGUI.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "Lists.h"
#include "FeatureConfig.h"

//#define DATABASE_CHUNK_DEBUG 1
#ifdef DATABASE_CHUNK_DEBUG
static char debugStr[256];
#endif // DATABASE_CHUNK_DEBUG

//#define WCS_OUTPUT_BISECTRIXDETAIL

#define WCS_COMBINATION_V2_V3_MINDIST

NotifyTag DBChangeEventName[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_NAME, 0), 0};
NotifyTag DBPreChangeEventName[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_NAME, 0), 0};
NotifyTag DBChangeEventFlags[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_FLAGS, 0), 0};
NotifyTag DBPreChangeEventFlags[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_FLAGS, 0), 0};

NotifyTag DBChangeEventDrawRGB[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB), 0};
NotifyTag DBChangeEventDrawPEN[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN), 0};
NotifyTag DBChangeEventDrawWIDTH[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH), 0};
NotifyTag DBChangeEventDrawLSTYLE[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE), 0};
NotifyTag DBChangeEventDrawPSTYLE[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE), 0};


NotifyTag DBPreChangeEventDrawRGB[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB), 0};
NotifyTag DBPreChangeEventDrawPEN[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN), 0};
NotifyTag DBPreChangeEventDrawWIDTH[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH), 0};
NotifyTag DBPreChangeEventDrawLSTYLE[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE), 0};
NotifyTag DBPreChangeEventDrawPSTYLE[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE), 0};

NotifyTag DBPreChangeEventPOINTS[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0), 0};
NotifyTag DBChangeEventPOINTS[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0), 0};

/*===========================================================================*/
//
//                        SOMEWHAT DIRE WARNING:
//
/*===========================================================================*/
//
//  Do not create static or auto instances of class Joe. Due to the
// peculiar way in which the name field is dynamically allocated, bad
// things will happen to a non-dynamic Joe object when you try to set
// the name. Just don't do it. Static/auto pointers to Joe objects
// are of course, fine.
//
//  As always, if you (like me) think you really do know what you're
// doing, Carpe Diem and do whatever the heck you want.

/*===========================================================================*/

void *Joe::operator new(size_t MySize, char *MyName)
{
Joe *Me;
char *Bar;
unsigned char StringLen = 0;

if (MyName)
	{
	StringLen = (unsigned char)strlen(MyName);
	} // if

Me = (Joe *)AppMem_Alloc(ROUNDUP(MySize + StringLen + 1, 4), APPMEM_CLEAR);

if (Me)
	{
	memset(Me, 0, (MySize + StringLen));
	Me->NameLen = (unsigned char)(ROUNDUP(MySize + StringLen + 1, 4) - MySize);
	Me->FName = NULL;
	Me->Flags = NULL;

	if (StringLen)
		{
		Bar = ((char *)Me) + MySize;
		strcpy(Bar, MyName);
		for (StringLen = 0; Bar[StringLen]; StringLen++)
			{
			if (Bar[StringLen] == '\n')
				{
				Bar[StringLen] = NULL;
				Me->FName = &Bar[StringLen + 1];
				break;
				} // if
			} // for
		} // if
	} // if

return(Me);

} // Joe::new

/*===========================================================================*/

// Vis4 didn't like it...
#if _MSC_VER >= 1200
void Joe::operator delete(void *pMem, char *MyName)
{

return;

}
#endif // _MSC_VER >= 1200

/*===========================================================================*/

//void Joe::operator delete(void *Me, size_t MySize)
void Joe::operator delete(void *Me)
{
size_t MySize = sizeof(Joe);
Joe *NameDeRef;
int BlockSize;
JoeAttribute *Attr, *NAttr;
LayerStub *LayerKill, *LayerScan;
static NotifyTag JoeDel[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_DELOBJ, 0, 0), 0};
static NotifyTag JoeActive[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0, 0), 0};

if (Me)
	{
	//if (NameDeRef->Name())
	//	{
	//	printf("%s\n", NameDeRef->Name());
	//	} // if
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(JoeDel, Me);
		} // if
	if (GlobalApp->AppDB->ActiveObj == Me)
		{
		GlobalApp->AppDB->ActiveObj = NULL;
		GlobalApp->AppDB->GenerateNotify(JoeActive, NULL);
		} // if
	NameDeRef = (Joe *)Me;
	NameDeRef->FreeRenderData();
	if (NameDeRef->Points())
		{
		GlobalApp->AppDB->MasterPoint.DeAllocate(NameDeRef->Points());
		NameDeRef->Points(NULL);
		} // if
	if (NameDeRef->LayerList)
		{
		LayerKill = NameDeRef->LayerList;
		while(LayerKill)
			{
			LayerScan = LayerKill->NextLayerInObject();
			// changed 12/11/03 by GRH so that database can be purged of unused layers
			// direct deletion replaced by RemoveObjectFromLayer()
			NameDeRef->RemoveObjectFromLayer(TRUE, NULL, LayerKill);
			//delete LayerKill;
			LayerKill = LayerScan;
			} // while
		NameDeRef->LayerList = NULL;
		} // if
	if (NameDeRef->TestFlags(WCS_JOEFLAG_NAMEREPLACED))
		{
		// Need to free un-attached namespace
		// I think these size calcs are right, no way to test 'em.
		BlockSize = (int)(strlen(NameDeRef->FName) + 1);
		BlockSize += (int)(strlen(&NameDeRef->FName[BlockSize]) + 1);
		AppMem_Free(NameDeRef->FName, BlockSize);
		} // if
	// Need to free any Attributes...
	if (GlobalApp->AppEffects) GlobalApp->AppEffects->RemoveFromJoeLists(NameDeRef);
	if (!NameDeRef->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		for (Attr = NameDeRef->AttribInfo.FirstAttrib; Attr; Attr = NAttr)
			{
			NAttr = Attr->NextAttrib;
			BlockSize = (int)Attr->AttribClassSize;
			//AppMem_Free(Attr, BlockSize);
			// Remember, we're calling delete on the base JoeAttribute class, not the derived class,
			// so if we add any fancy dynamic storage to any derived classes, we'll need a
			// virtual destructor...
			delete Attr;
			} // for
		} // if
	AppMem_Free(Me, MySize + NameDeRef->NameLen);
	} // if

} // Joe::delete

/*===========================================================================*/

void Joe::FreeRenderData(void)
{

if (JRendData)
	{
	JRendData->FreeSegmentData();
	JRendData->FreeSplinedJoe();
	delete JRendData;
	JRendData = NULL;
	} // if
	
} // Joe::FreeRenderData

/*===========================================================================*/

void Joe::FreeSegmentData(void)
{

if (JRendData)
	{
	JRendData->FreeSegmentData();
	} // if

} // Joe::FreeSegmentData

/*===========================================================================*/

void JoeRenderData::FreeSegmentData(void)
{

if (SegData[0])
	delete [] SegData[0];
if (SegData[1])
	delete [] SegData[1];
SegData[1] = SegData[0] = NULL;
NumSegs = 0;

} // JoeRenderData::FreeSegmentData

/*===========================================================================*/

void JoeRenderData::FreeSplinedJoe(void)
{

if (SplinedJoe)
	delete SplinedJoe;
SplinedJoe = NULL;

} // JoeRenderData::FreeSplinedJoe

/*===========================================================================*/

JoeSegmentData **JoeRenderData::AddSegmentData(unsigned long NewNumSegs)
{

if (SegData[0] = new JoeSegmentData[NewNumSegs])
	{
	if (SegData[1] = new JoeSegmentData[NewNumSegs])
		{
		NumSegs = NewNumSegs;
		} // if
	else
		FreeSegmentData(); 
	} // if
return (SegData[0] ? SegData: NULL);

} // JoeRenderData::AddSegmentData

/*===========================================================================*/

Joe *JoeRenderData::AddJoeSpline(Database *DBHost, Joe *Original, double AngularDev)
{
CoordSys *MyCoords;

if (SplinedJoe)
	return (SplinedJoe);
	
if (SplinedJoe = new (NULL) Joe)
	{
	SplinedJoe->Copy(SplinedJoe, Original);
	if (SplinedJoe->CopyPoints(SplinedJoe, Original, 0, 0))
		{
		MyCoords = Original->GetCoordSys();
		if (! MyCoords || SplinedJoe->ConvertToDefGeo(MyCoords))
			{
			if (SplinedJoe->Spliney(DBHost, AngularDev))
				return (SplinedJoe);
			} // if
		} // if
	delete SplinedJoe;
	SplinedJoe = NULL;
	} // if
		
return (NULL);
		
} // JoeRenderData::AddJoeSpline

/*===========================================================================*/

bool Joe::Spliney(Database *DBHost, double AngularDev)
{
double LinkAngle, BackDistance, ActiveDistance, NextDistance, InsertInterval, InsertDist,
	S1, S2, S3, h1, h2, h3, h4, D1, D2;
VectorPoint *PLinkActive, *PLinkBackOne, *PLinkAheadOne, *PLinkAheadTwo, *InsertPt, *InsertList;
unsigned long PointsToInsert, CarryOverPoints, NextCarryOver, InsertPtCt, MaxPtsBasedOnDistance;
bool Success = true;

PLinkAheadTwo = PLinkBackOne = NULL;
CarryOverPoints = 0;
ActiveDistance = BackDistance = NextDistance = 0.0;
for (PLinkActive = GetFirstRealPoint(), PLinkAheadOne = GetSecondRealPoint(); PLinkAheadOne; PLinkAheadOne = PLinkAheadTwo)
	{
	PLinkAheadTwo = PLinkAheadOne->Next;
	
	// no insertion necessary if there is only one segment
	if ((CarryOverPoints || PLinkAheadTwo) && ! PLinkActive->SamePoint(PLinkAheadOne))
		{
		// estimate the number of points needed along this segment
		// Find the angle from the back segment to the current segment and from the current segment to the next segment
		if (PLinkAheadTwo && ! PLinkAheadOne->SamePoint(PLinkAheadTwo))
			{
			LinkAngle = fabs(AngleBetweenSegments(PLinkActive, PLinkAheadOne));
			NextDistance = FindDistance(PLinkAheadOne->Latitude, PLinkAheadOne->Longitude, PLinkAheadTwo->Latitude, PLinkAheadTwo->Longitude, EARTHRAD);
			} // if
		else
			{
			LinkAngle = 0.0;
			NextDistance = 0.0;
			} // else
		PointsToInsert = (unsigned long)fabs(LinkAngle / AngularDev);
		NextCarryOver = PointsToInsert / 2;
		PointsToInsert = PointsToInsert - NextCarryOver + CarryOverPoints;
		CarryOverPoints = NextCarryOver;
		if (PointsToInsert > 0)
			{
			if (ActiveDistance == 0.0)
				ActiveDistance = FindDistance(PLinkActive->Latitude, PLinkActive->Longitude, PLinkAheadOne->Latitude, PLinkAheadOne->Longitude, EARTHRAD);
			MaxPtsBasedOnDistance = (unsigned long)(ActiveDistance * 1000.0) + 1;	// distance is in km
			if (PointsToInsert > MaxPtsBasedOnDistance)
				{
				PointsToInsert = MaxPtsBasedOnDistance;
				} // if
			if (InsertList = DBHost->MasterPoint.Allocate(PointsToInsert))
				{
				InsertInterval = 1.0 / (PointsToInsert + 1);
				for (InsertPtCt = 0, InsertPt = InsertList, InsertDist = InsertInterval; InsertPtCt < PointsToInsert; ++InsertPtCt, InsertPt = InsertPt->Next, InsertDist += InsertInterval)
					{
					// common factors
					S1 = InsertDist;
					S2 = S1 * S1;
					S3 = S1 * S2;
					h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
					h2 = -2.0 * S3 + 3.0 * S2;
					h3 = S3 - 2.0 * S2 + S1;
					h4 = S3 - S2;

					// latitude
					D1 = PLinkBackOne ?
						((.5 * (PLinkActive->Latitude - PLinkBackOne->Latitude))
						+ (.5 * (PLinkAheadOne->Latitude - PLinkActive->Latitude)))
						* (2.0 * ActiveDistance / (BackDistance + ActiveDistance)):
						(PLinkAheadOne->Latitude - PLinkActive->Latitude);
					D2 = PLinkAheadTwo ?
						((.5 * (PLinkAheadOne->Latitude - PLinkActive->Latitude))
						+ (.5 * (PLinkAheadTwo->Latitude - PLinkAheadOne->Latitude)))
						* (2.0 * ActiveDistance / (ActiveDistance + NextDistance)):
						(PLinkAheadOne->Latitude - PLinkActive->Latitude);
					InsertPt->Latitude = (PLinkActive->Latitude * h1 + PLinkAheadOne->Latitude * h2 + D1 * h3 + D2 * h4);

					// longitude
					D1 = PLinkBackOne ?
						((.5 * (PLinkActive->Longitude - PLinkBackOne->Longitude))
						+ (.5 * (PLinkAheadOne->Longitude - PLinkActive->Longitude)))
						* (2.0 * ActiveDistance / (BackDistance + ActiveDistance)):
						(PLinkAheadOne->Longitude - PLinkActive->Longitude);
					D2 = PLinkAheadTwo ?
						((.5 * (PLinkAheadOne->Longitude - PLinkActive->Longitude))
						+ (.5 * (PLinkAheadTwo->Longitude - PLinkAheadOne->Longitude)))
						* (2.0 * ActiveDistance / (ActiveDistance + NextDistance)):
						(PLinkAheadOne->Longitude - PLinkActive->Longitude);
					InsertPt->Longitude = (PLinkActive->Longitude * h1 + PLinkAheadOne->Longitude * h2 + D1 * h3 + D2 * h4);

					// elevation
					D1 = PLinkBackOne ?
						((.5 * (PLinkActive->Elevation - PLinkBackOne->Elevation))
						+ (.5 * (PLinkAheadOne->Elevation - PLinkActive->Elevation)))
						* (2.0 * ActiveDistance / (BackDistance + ActiveDistance)):
						(PLinkAheadOne->Elevation - PLinkActive->Elevation);
					D2 = PLinkAheadTwo ?
						((.5 * (PLinkAheadOne->Elevation - PLinkActive->Elevation))
						+ (.5 * (PLinkAheadTwo->Elevation - PLinkAheadOne->Elevation)))
						* (2.0 * ActiveDistance / (ActiveDistance + NextDistance)):
						(PLinkAheadOne->Elevation - PLinkActive->Elevation);
					InsertPt->Elevation = (float)(PLinkActive->Elevation * h1 + PLinkAheadOne->Elevation * h2 + D1 * h3 + D2 * h4);
					if (! InsertPt->Next)
						InsertPt->Next = PLinkAheadOne;
					} // for
				PLinkActive->Next = InsertList;
				NumPoints(NumPoints() + PointsToInsert);
				} // if
			} // if
		} // if
	PLinkBackOne = PLinkActive;
	PLinkActive = PLinkAheadOne;
	BackDistance = ActiveDistance;
	ActiveDistance = NextDistance;
	} // for

return (Success);

} // Joe::Spliney

/*===========================================================================*/

double Joe::AngleBetweenSegments(VectorPoint *PLinkFirst, VectorPoint *PLinkSecond)
{
double TestAngle;
TriangleBoundingBoxVector TBxFrom, TBxTo, AngleFinder;

TBxFrom.SetXY(PLinkFirst->Next, PLinkFirst);
TBxTo.SetXY(PLinkSecond->Next, PLinkSecond);
TestAngle = AngleFinder.FindRelativeAngle(TBxFrom.SetXY(PLinkFirst->Next, PLinkFirst), TBxTo.SetXY(PLinkSecond->Next, PLinkSecond));
TestAngle *= 90.0;
TestAngle -= 180.0;

return (TestAngle);

} // Joe::AngleBetweenSegments

/*===========================================================================*/

JoeSegmentData **Joe::AddSegmentData(unsigned long NewNumSegs)
{
JoeSegmentData **rVal = NULL;

if (JRendData || (JRendData = new JoeRenderData()))
	{
	rVal = JRendData->AddSegmentData(NewNumSegs);
	} // if

return (rVal);

} // Joe::AddSegmentData

/*===========================================================================*/

Joe *Joe::AddJoeSpline(Database *DBHost, double AngularDev)
{
Joe *rVal = NULL;

if (JRendData || (JRendData = new JoeRenderData()))
	{
	rVal = JRendData->AddJoeSpline(DBHost, this, AngularDev);
	} // if

return (rVal);

} // Joe::AddJoeSpline

/*===========================================================================*/

void Joe::JoeInitClear(void)
{

NextJoe = PrevJoe = Parent = NULL;
// Following not needed due to union
//Child = NULL;
Utility = Flags = NULL;
#ifdef WCS_BUILD_VNS
UniqueLoadSaveID = 0;
#endif // WCS_BUILD_VNS
NumPoints(NULL);
LayerList = NULL;
AttribInfo.FirstAttrib = NULL;
AttribInfo.RedGun = AttribInfo.GreenGun = AttribInfo.BlueGun = 0;
AttribInfo.LineStyle = AttribInfo.PointStyle = AttribInfo.LineWidth = 0;
AttribInfo.CanonicalTypeMajor = AttribInfo.CanonicalTypeMinor = 0;
AttribInfo.SecondTypeMajor = AttribInfo.SecondTypeMinor = 0;
NWLat = -DBL_MAX; NWLon = -DBL_MAX;
SELat = DBL_MAX; SELon = DBL_MAX;
Points(NULL);
JRendData = NULL;

} //Joe::JoeInitClear

/*===========================================================================*/

Joe::Joe(size_t Blah, char *Bar)
: RasterAnimHost(NULL)
{

Joe::JoeInitClear();

} //Joe::Joe

/*===========================================================================*/

Joe::Joe(void)
: RasterAnimHost(NULL)
{

JoeInitClear();
//NameLen = 0;
//FName = NULL;

} // Joe::Joe

/*===========================================================================*/

Joe::~Joe()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->DPG;
		GlobalApp->GUIWins->DPG = NULL;
		} // if
	if (GlobalApp->GUIWins->DEM && GlobalApp->GUIWins->DEM->GetActive() == this)
		{
		delete GlobalApp->GUIWins->DEM;
		GlobalApp->GUIWins->DEM = NULL;
		} // if
	if (GlobalApp->GUIWins->VPX && GlobalApp->GUIWins->VPX->GetActive() == this)
		{
		delete GlobalApp->GUIWins->VPX;
		GlobalApp->GUIWins->VPX = NULL;
		} // if
	} // if

} // Joe::~Joe

/*===========================================================================*/

//void Joe::SetName(char *NewName)
//{
//if (NewName)
//	{
//	if (strlen(NewName) < NameLen)
//		{
//		strcpy(((char *)this) + sizeof(Joe), NewName);
//		} // if
//	} //if
//} // Joe::SetName

/*===========================================================================*/

void Joe::SetFlags(unsigned long FlagSet)
{

if (GlobalApp && GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		if (GlobalApp->MainProj)
			{
			if ((FlagSet & WCS_JOEFLAG_ACTIVATED) && (FlagSet & WCS_JOEFLAG_RENDERENABLED) && ((Flags | FlagSet) & WCS_JOEFLAG_ISDEM))
				{
				GlobalApp->MainProj->SetFRDInvalid(1);
				} // if
			if (((Flags | FlagSet) & (WCS_JOEFLAG_ISDEM | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_RENDERENABLED)) && (FlagSet & WCS_JOEFLAG_DEM_NO_SEALEVEL))
				{
				GlobalApp->MainProj->SetFRDInvalid(1);
				} // if
			} // if
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventFlags, this);

		} // if
	} // if
Flags |= FlagSet;

if (GlobalApp && GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventFlags, this);
		} // if
	} // if

} // Joe::SetFlags

/*===========================================================================*/

void Joe::ClearFlags(unsigned long FlagClear)
{

if (GlobalApp && GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventFlags, this);
		} // if
	} // if
if ((Flags & (WCS_JOEFLAG_ISDEM | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_RENDERENABLED)) && (FlagClear & WCS_JOEFLAG_DEM_NO_SEALEVEL))
	{
	GlobalApp->MainProj->SetFRDInvalid(1);
	} // if
Flags &= ~FlagClear;
if (GlobalApp && GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventFlags, this);
		} // if
	} // if

} // Joe::ClearFlags

/*===========================================================================*/

void Joe::SetNewNames(const char *NewName, const char *NewFName)
{
char *NewBlock, *OldBlock;
const char *CurName, *CurFName;
int OldNLen, OldFNLen, BlockSize;

OldBlock = NewBlock = NULL;

if (NewFName || NewName)
	{
	CurName = Name();
	CurFName = FileName();
	if (NewName) CurName = NewName;
	if (NewFName) CurFName = NewFName;
	if (CurName == NULL) CurName = "";
	if (CurFName == NULL) CurFName = "";

	if (TestFlags(WCS_JOEFLAG_NAMEREPLACED))
		{
		// We'll need to free this when we're done.
		OldBlock = FName;
		} // if
	OldNLen  = (int)strlen(CurName);
	OldFNLen = (int)strlen(CurFName);
	BlockSize = 2 + OldNLen + OldFNLen;
	if (NewBlock = (char *)AppMem_Alloc(BlockSize, APPMEM_CLEAR))
		{
		memcpy(&NewBlock[0], CurFName, OldFNLen + 1);
		memcpy(&NewBlock[OldFNLen + 1], CurName, OldNLen + 1);
		FName = NewBlock;
		if (OldBlock)
			{
			// I think these size calcs are right, no way to test it.
			BlockSize = (int)(strlen(OldBlock) + 1);
			BlockSize += (int)(strlen(&OldBlock[BlockSize]) + 1);
			AppMem_Free(OldBlock, BlockSize);
			} // if
		SetFlags(WCS_JOEFLAG_NAMEREPLACED);
		if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
			{
			GlobalApp->AppDB->GenerateNotify(DBChangeEventName, this);
			} // if
		} // if
	} // if

} // Joe::SetNewNames

/*===========================================================================*/

const char * Joe::Name(void)
{
const char *Foo, *Seek;
int Skip;

// If NameReplaced then Name follows after FName in
// seperately allocated block...
if (TestFlags(WCS_JOEFLAG_NAMEREPLACED))
	{
	if (FName)
		{
		// There is no body to this for loop...
		for (Skip = 0; FName[Skip]; Skip++);	//lint !e722
		Seek = &FName[Skip + 1];
		if (Seek[0])
			{
			return(Seek);
			} // if
		else
			{
			return(NULL);
			} // else
		} // if
	} // if
else
	{
	if (Foo = (((char *)this) + sizeof(Joe)))
		{
		if (Foo[0])
			{
			return(Foo);
			} // if
		} // if
	} // else

// if we are a DEM, or we have an external file, use that name
//if (TestFlags(WCS_JOEFLAG_ISDEM) || TestFlags(WCS_JOEFLAG_EXTFILE))
//	{
//	if (Foo = FileName())
//		{
//		if (Foo[0])
//			{
//			return(Foo);
//			} // if
//		} // if
//	} // if

//return("Unnamed");
return(NULL);

} // Joe::Name

/*===========================================================================*/

// The following does not handle updating the pointer to the Joe object
// from its parent. Dangerous! But intentional.
void Joe::UnlinkMe(void) // Only should be called by Database objects
{

if (NextJoe)
	{
	NextJoe->PrevJoe = PrevJoe;
	} // if
if (PrevJoe)
	{
	PrevJoe->NextJoe = NextJoe;
	} // if
PrevJoe = NextJoe = NULL;

} // Joe::UnlinkMe

/*===========================================================================*/

Joe *Joe::LinkMeAfter(Joe *ThisGuy)
{

if (ThisGuy)
	{
	if (ThisGuy->Parent)
		{
		Parent = ThisGuy->Parent;
		PrevJoe = ThisGuy;
		NextJoe = ThisGuy->NextJoe;
		ThisGuy->NextJoe = this;
		if (NextJoe)
			{
			NextJoe->PrevJoe = this;
			} // if
		IncrementUpTree();
		} // if
	} // if

return(NULL);

} // Joe::LinkMeAfter

/*===========================================================================*/

Joe *Joe::LinkMeBefore(Joe *ThisGuy)
{

if (ThisGuy)
	{
	if (ThisGuy->Parent)
		{
		Parent = ThisGuy->Parent;
		NextJoe = ThisGuy;
		PrevJoe = ThisGuy->PrevJoe;
		ThisGuy->PrevJoe = this;
		if (PrevJoe)
				{
				PrevJoe->NextJoe = this;
				} // if
		IncrementUpTree();
		} // if
	} // if

return(NULL);

} // Joe::LinkMeBefore()

/*===========================================================================*/

void Joe::IncrementUpTree(void)
{
Joe *FamilyTree;

for (FamilyTree = Parent; FamilyTree; FamilyTree = FamilyTree->Parent)
	{
	FamilyTree->GroupInfo.NumKids++;
	} // for

} // Joe::IncrementUpTree

/*===========================================================================*/

void Joe::DecrementUpTree(void)
{
Joe *FamilyTree;

for (FamilyTree = Parent; FamilyTree; FamilyTree = FamilyTree->Parent)
	{
	FamilyTree->GroupInfo.NumKids--;
	} // for

} // Joe::DecrementUpTree

/*===========================================================================*/

void Joe::ZeroUpTree(void)
{
Joe *NextPar;

for (NextPar = Parent; NextPar;)
	{
	if (NextPar->NWLat == -10000.0 && NextPar->NWLon == -10000.0 &&
	 NextPar->SELat == 10000.0 && NextPar->SELon == 10000.0)
		{
		return;
		} // if
	NextPar->NWLat = -10000.0;
	NextPar->NWLon = -10000.0;
	NextPar->SELat = 10000.0;
	NextPar->SELon = 10000.0;
	NextPar = NextPar->Parent;
	} // for

} // Joe::ZeroUpTree

/*===========================================================================*/

Joe *Joe::WalkUp(void)
{
Joe *Me;

Me = this;

while(Me)
	{
	if (Me->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		// we're descended of the group tree, walk up and try the nongroup child
		if (Me = Me->Parent)
			{
			if (Me->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Me->GroupInfo.JChild)
					{
					// try the non-group Child
					return(Me->GroupInfo.JChild);
					} // if
				} // if
			} // if
		} // if
	else
		{
		// we're descended from the non-group tree, try the Next pointer
		if (Me = Me->Parent)
			{
			if (Me->NextJoe)
				{
				return(Me->NextJoe);
				} // if
			} // if
		} // else
	} // while

return(NULL);

} // Joe::WalkUp

/*===========================================================================*/

Joe *Joe::WalkUpGroup(void)
{
Joe *Me;

Me = this;

while(Me)
	{
	if (Me = Me->Parent)
		{
		if (Me->NextJoe)
			{
			return(Me->NextJoe);
			} // if
		} // if
	} // while

return(NULL);

} // Joe::WalkUpGroup

/*===========================================================================*/

Joe *Joe::RemoveMe(EffectsLib *Lib)
{

if (Parent)
	{
	//Lib->RemoveFromJoeLists(this);	// removed 9/16/02 - this finds only those instances where there is a Joe attribute linking the Joe to an effect
	if (Lib->RemoveRAHost(this, 1))	// added 9/16/02 - this finds all instances of attachment in the effects library
		return(Parent->RemoveChild(this));
	} // if

return(NULL);

} // Joe::RemoveMe

/*===========================================================================*/

Joe *Joe::RemoveChild(Joe *TheChild)
{
// We do verify that the object is flagged as posessing children,
// so we don't trash the universe by misusing the color guns

if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	return(NULL); // Don't even look at Child pointer, could be meaningless
	} // if

if (TheChild)
	{
	if (TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (TheChild->Parent == this)
			{
			if (GroupInfo.JChild == TheChild) //lint !e1402
				{
				if ((GroupInfo.JChild = TheChild->NextJoe) == NULL)
					{
					// Don't want to do this automatically
					//ClearFlags(WCS_JOEFLAG_HASKIDS);
					} // if
				} // if
			if (GroupInfo.GroupChild == TheChild)
				{
				if ((GroupInfo.GroupChild = TheChild->NextJoe) == NULL)
					{
					// Don't want to do this automatically
					//ClearFlags(WCS_JOEFLAG_HASKIDS);
					} // if
				} // if
			if (TheChild->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				GroupInfo.NumGroups--;
				} // if
			if ((NWLat == TheChild->NWLat) || (NWLon == TheChild->NWLon) ||
			 (SELat == TheChild->SELat) || (SELon == TheChild->SELon))
				{
				// Our bound-extent may be defined partially by the child being
				// removed, clear our bound-extent and those of our parents so
				// the bounds will get recalculated by ReBoundTree()
				TheChild->ZeroUpTree();
				} // if
			TheChild->DecrementUpTree();
			TheChild->UnlinkMe();

			return(TheChild);
			} // if
		} // if
	} // if

return(NULL);

} // Joe::RemoveChild

/*===========================================================================*/

Joe *Joe::AddChild(Joe *TheChild, Joe *AfterChild)
{
Joe *Monte;

if (!TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	if (AttribInfo.FirstAttrib)
		{
		return(NULL); // Can't add kids when attribs are present
		} // if
	else
		{ // need to move to the Group list
		if (!Parent)
			{ // paranoia
			UserMessageOK("Add Error:", "Corruption in parent pointer.");
			return(0);
			} // if
		SetFlags(WCS_JOEFLAG_HASKIDS);
		GroupInfo.JChild = GroupInfo.GroupChild = NULL;
		GroupInfo.NumKids = GroupInfo.NumGroups = 0;
		Monte = Parent;
		Parent->RemoveChild(this); // remove from Child list
		Monte->AddChild(this); // add to GroupChild list
		Parent->GroupInfo.NumGroups++;
		} // else
	} // if

if (TheChild)
	{
	if (TheChild->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		// Note: AfterChild is not supported with group objects
		if (GroupInfo.GroupChild)
			{
			#ifdef WCS_BUILD_V3
			// new tail-append code
			for (Joe *AppendScan = GroupInfo.GroupChild; AppendScan; AppendScan = AppendScan->NextJoe)
				{
				if (AppendScan->NextJoe == NULL)
					{
					// this is the end of the line, append here
					TheChild->PrevJoe = AppendScan;
					TheChild->NextJoe = NULL;
					AppendScan->NextJoe = TheChild;
					break;
					} // if
				} // for
			#else // !WCS_BUILD_V3, must be VNS2 or 1
			// former head-insert code
			TheChild->NextJoe = GroupInfo.GroupChild;
			GroupInfo.GroupChild->PrevJoe = TheChild;
			GroupInfo.GroupChild = TheChild;
			#endif // !WCS_BUILD_V3
			} // if
		else
			{
			GroupInfo.GroupChild = TheChild;
			TheChild->NextJoe = TheChild->PrevJoe = NULL;
			} // else
		GroupInfo.NumGroups++;
		} // if
	else
		{
		if (GroupInfo.JChild)
			{
			//printf(">> %08X\n", this);
			if (AfterChild)
				{
				if (AfterChild->NextJoe)
					{
					AfterChild->NextJoe->PrevJoe = TheChild;
					} // if
				TheChild->PrevJoe = AfterChild;
				TheChild->NextJoe = AfterChild->NextJoe;
				AfterChild->NextJoe = TheChild;
				} // if
			else
				{
				TheChild->NextJoe = GroupInfo.JChild;
				GroupInfo.JChild->PrevJoe = TheChild;
				GroupInfo.JChild = TheChild;
				} // else
			} // if
		else
			{
			//printf("--\n");
			GroupInfo.JChild = TheChild;
			TheChild->NextJoe = TheChild->PrevJoe = NULL;
			} // else
		} // else
	TheChild->Parent = this;
	TheChild->IncrementUpTree();
	} // if valid child

return(TheChild);

} // Joe::AddChild

/*===========================================================================*/

unsigned long Joe::Children(void)
{
unsigned long rVal = 0;

if (Flags & WCS_JOEFLAG_HASKIDS)
	{
	rVal = GroupInfo.NumKids;
	} // if

return(rVal);

} // Joe::Children

/*===========================================================================*/

void Joe::SetRGB(unsigned char Red, unsigned char Green, unsigned char Blue)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawRGB, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.RedGun = Red;
	AttribInfo.GreenGun = Green;
	AttribInfo.BlueGun = Blue;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawRGB, this);
		} // if
	} // if

} // Joe::SetRGB

/*===========================================================================*/

void Joe::SetRed(unsigned char Red)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawRGB, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.RedGun = Red;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawRGB, this);
		} // if
	} // if

} // Joe::SetRed

/*===========================================================================*/

void Joe::SetGreen(unsigned char Green)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawRGB, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.GreenGun = Green;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawRGB, this);
		} // if
	} // if

} // Joe::SetGreen

/*===========================================================================*/

void Joe::SetBlue(unsigned char Blue)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawRGB, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.BlueGun = Blue;
	} // if

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawRGB, this);
		} // if
	} // if

} // Joe::SetBlue

/*===========================================================================*/

void Joe::SetLineWidth(unsigned char NewWidth)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawWIDTH, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.LineWidth = NewWidth;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawWIDTH, this);
		} // if
	} // if

} // Joe::SetLineWidth

/*===========================================================================*/

void Joe::SetLineStyle(unsigned short NewLineStyle)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawLSTYLE, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.LineStyle = NewLineStyle;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawLSTYLE, this);
		} // if
	} // if

} // Joe::SetLineStyle

/*===========================================================================*/

void Joe::SetPointStyle(unsigned char NewPointStyle)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventDrawPSTYLE, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.LineStyle = NewPointStyle;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventDrawPSTYLE, this);
		} // if
	} // if

} // Joe::SetPointStyle

/*===========================================================================*/

unsigned long Joe::GetNumRealPoints(void)
{

#ifdef WCS_JOE_LABELPOINTEXISTS
// first point is the old WCS label point
return (nNumPoints > 1 ? nNumPoints - 1: 0);
#else // WCS_JOE_LABELPOINTEXISTS
return (nNumPoints);
#endif // WCS_JOE_LABELPOINTEXISTS

} // Joe::GetNumRealPoints

/*===========================================================================*/

unsigned long Joe::GetFirstRealPtNum(void)
{

#ifdef WCS_JOE_LABELPOINTEXISTS
// first point is the old WCS label point
return (1);
#else // WCS_JOE_LABELPOINTEXISTS
return (0);
#endif // WCS_JOE_LABELPOINTEXISTS

} // Joe::GetFirstRealPtNum

/*===========================================================================*/

VectorPoint *Joe::GetFirstRealPoint(void)
{

#ifdef WCS_JOE_LABELPOINTEXISTS
// first point is the old WCS label point
return (nPoints ? nPoints->Next: NULL);
#else // WCS_JOE_LABELPOINTEXISTS
return (nPoints);
#endif // WCS_JOE_LABELPOINTEXISTS

} // Joe::GetFirstRealPoint

/*===========================================================================*/

VectorPoint *Joe::GetSecondRealPoint(void)
{

#ifdef WCS_JOE_LABELPOINTEXISTS
// first point is the old WCS label point
return (nPoints && nPoints->Next ? nPoints->Next->Next: NULL);
#else // WCS_JOE_LABELPOINTEXISTS
return (nPoints ? nPoints->Next: NULL);
#endif // WCS_JOE_LABELPOINTEXISTS

} // Joe::GetSecondRealPoint

/*===========================================================================*/

VectorPoint *Joe::TransferPoints(VectorPart *CopyFrom, bool ReplicateOrigin)
{
VectorPoint *CurPt;
VectorNode *CurNode;
unsigned long PtCt, TotalPoints;

TotalPoints = CopyFrom->NumNodes + GetFirstRealPtNum() + (ReplicateOrigin ? 1: 0);
if (Points(GlobalApp->AppDB->MasterPoint.Allocate(TotalPoints)))
	{
	NumPoints(TotalPoints);
	CurPt = Points();
	CurNode = CopyFrom->FirstNode();
	if (GetFirstRealPtNum())
		{
		CurPt->Latitude = CurNode->Lat;
		CurPt->Longitude = CurNode->Lon;
		CurPt->Elevation = (float)CurNode->Elev;
		CurPt = CurPt->Next;
		--TotalPoints;
		} // if
	for (PtCt = 0; PtCt < TotalPoints; ++PtCt, CurPt = CurPt->Next, CurNode = CurNode->NextNode)
		{
		CurPt->Latitude = CurNode->Lat;
		CurPt->Longitude = CurNode->Lon;
		CurPt->Elevation = (float)CurNode->Elev;
		} // for
	} // if

return (Points());

} // Joe::TransferPoints

/*===========================================================================*/

void Joe::RemoveDuplicatePoints(Database *DBHost)
{
VectorPoint *PLink, *LastPoint = NULL;

for (PLink = GetFirstRealPoint(); PLink; PLink = PLink->Next)
	{
	// alter the vector so it doesn't cause a problem ever again
	if (LastPoint && LastPoint->SamePointLatLon(PLink))
		{
		LastPoint->Next = PLink->Next;
		PLink->Next = NULL;
		DBHost->MasterPoint.DeAllocate(PLink);
		PLink = LastPoint;
		NumPoints(NumPoints() - 1);
		continue;
		} // if
	LastPoint = PLink;
	} // for

} // Joe::RemoveDuplicatePoints

/*===========================================================================*/

JoeAttribute *Joe::MatchAttribute(unsigned char MajorAttrib, unsigned char MinorAttrib)
{
JoeAttribute *Search;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
	{
	if ((Search->MajorAttribType == MajorAttrib) && (Search->MinorAttribType == MinorAttrib))
		{
		break;
		} // if
	} // for

return(Search);

} // Joe::MatchAttribute

/*===========================================================================*/

JoeAttribute *Joe::MatchAttributeRange(unsigned char MajorAttrib,
	unsigned char MinorAttribLow, unsigned char MinorAttribHigh, JoeAttribute *Current)
{
JoeAttribute *Search;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

Search = Current ? Current->NextAttrib: AttribInfo.FirstAttrib;

for (; Search; Search = Search->NextAttrib)
	{
	if ((Search->MajorAttribType == MajorAttrib) &&
		(Search->MinorAttribType >= MinorAttribLow) && (Search->MinorAttribType <= MinorAttribHigh))
			{
			break;
			} // if
	} // for

return(Search);

} // Joe::MatchAttributeRange

/*===========================================================================*/

void Joe::AddAttribute(JoeAttribute *Me)
{

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return; // Attributes are void if kids present
	} // if

if (Me)
	{
	Me->NextAttrib = AttribInfo.FirstAttrib;
	AttribInfo.FirstAttrib = Me;
	} // if
} // Joe::AddAttribute

/*===========================================================================*/

void Joe::AddAttributeNotify(JoeAttribute *Me)
{
NotifyTag Changes[2];

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return; // Attributes are void if kids present
	} // if

if (Me)
	{
	Me->NextAttrib = AttribInfo.FirstAttrib;
	AttribInfo.FirstAttrib = Me;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

} // Joe::AddAttributeNotify

/*===========================================================================*/

JoeAttribute *Joe::RemoveAttribute(unsigned char MajorAttrib, unsigned char MinorAttrib)
{
JoeAttribute *Search = NULL, *Found = NULL;;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	if ((AttribInfo.FirstAttrib->MajorAttribType == MajorAttrib) &&
	 (AttribInfo.FirstAttrib->MinorAttribType == MinorAttrib))
		{
		Found = AttribInfo.FirstAttrib;
		AttribInfo.FirstAttrib = AttribInfo.FirstAttrib->NextAttrib;
		return(Found);
		} // if
	else
		{
		for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
			{
			if (Search->NextAttrib)
				{
				if ((Search->NextAttrib->MajorAttribType == MajorAttrib) &&
				 (Search->NextAttrib->MinorAttribType == MinorAttrib))
					{
					Found = Search->NextAttrib;
					Search->NextAttrib = Found->NextAttrib;
					return(Found);
					} // if
				} // if
			else
				{
				return(NULL); // Couldn't find it
				} // else
			} // for
		return(NULL);
		} // else

	} // if
else
	{
	return(NULL);
	} // else

} // Joe::RemoveAttribute

/*===========================================================================*/

JoeAttribute *Joe::RemoveEffectAttribute(unsigned char MajorAttrib, unsigned char MinorAttrib, GeneralEffect *MatchEffect)
{
JoeAttribute *Search = NULL, *Found = NULL;;
NotifyTag Changes[2];

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	if ((AttribInfo.FirstAttrib->MajorAttribType == MajorAttrib) &&
	 (AttribInfo.FirstAttrib->MinorAttribType == MinorAttrib) &&
	 (MatchEffectAttribute(AttribInfo.FirstAttrib, MatchEffect)))
		{
		Found = AttribInfo.FirstAttrib;
		AttribInfo.FirstAttrib = AttribInfo.FirstAttrib->NextAttrib;
		// update joe bounds before notification so editors get correct bounds
		if (MajorAttrib == WCS_JOE_ATTRIB_INTERNAL && MinorAttrib == WCS_JOE_ATTRIB_INTERNAL_COORDSYS && !GlobalApp->DatabaseDisposalInProgress) // avoid rechecking if we're cleaning up whole database
			RecheckBounds();
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		return(Found);
		} // if
	else
		{
		for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
			{
			if (Search->NextAttrib)
				{
				if ((Search->NextAttrib->MajorAttribType == MajorAttrib) &&
				 (Search->NextAttrib->MinorAttribType == MinorAttrib) &&
				 (MatchEffectAttribute(Search->NextAttrib, MatchEffect)))
					{
					Found = Search->NextAttrib;
					Search->NextAttrib = Found->NextAttrib;
					// update joe bounds before notification so editors get correct bounds
					if (MajorAttrib == WCS_JOE_ATTRIB_INTERNAL && MinorAttrib == WCS_JOE_ATTRIB_INTERNAL_COORDSYS)
						if (!GlobalApp->GetTerminate()) // don't bother if we're shutting down anyway
							{
							RecheckBounds();
							} // if
					Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					return(Found);
					} // if
				} // if
			else
				{
				return(NULL); // Couldn't find it
				} // else
			} // for
		return(NULL);
		} // else

	} // if
else
	{
	return(NULL);
	} // else

} // Joe::RemoveEffectAttribute

/*===========================================================================*/

JoeAttribute *Joe::RemoveSpecificAttribute(JoeAttribute *RemoveMe)
{
JoeAttribute *Search = NULL, *Prev = NULL;
NotifyTag Changes[2];

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
		{
		if (Search == RemoveMe)
			{
			if (Prev)
				Prev->NextAttrib = Search->NextAttrib;
			else
				AttribInfo.FirstAttrib = Search->NextAttrib;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			return(Search);
			} // if
		Prev = Search;
		} // for
	} // if

return(NULL);

} // Joe::RemoveSpecificAttribute

/*===========================================================================*/

JoeAttribute *Joe::RemoveSpecificAttributeNoNotify(JoeAttribute *RemoveMe)
{
JoeAttribute *Search = NULL, *Prev = NULL;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
		{
		if (Search == RemoveMe)
			{
			if (Prev)
				Prev->NextAttrib = Search->NextAttrib;
			else
				AttribInfo.FirstAttrib = Search->NextAttrib;
			return(Search);
			} // if
		Prev = Search;
		} // for
	} // if

return(NULL);

} // Joe::RemoveSpecificAttributeNoNotify

/*===========================================================================*/

JoeAttribute *Joe::MatchEffectAttribute(JoeAttribute *Attrib, GeneralEffect *MatchEffect)
{

if (Attrib->MajorAttrib() != WCS_JOE_ATTRIB_INTERNAL)	// this is redundant if used from above but makes the func more generally useful
	return (NULL);

switch (Attrib->MinorAttrib())
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		{
		JoeLake *Attrib2 = (JoeLake *)Attrib;

		if (Attrib2->Lake == (LakeEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LAKE
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		{
		JoeEcosystem *Attrib2 = (JoeEcosystem *)Attrib;

		if (Attrib2->Eco == (EcosystemEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		JoeRasterTA *Attrib2 = (JoeRasterTA *)Attrib;

		if (Attrib2->Terra == (RasterTerraffectorEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		JoeTerraffector *Attrib2 = (JoeTerraffector *)Attrib;

		if (Attrib2->Terra == (TerraffectorEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		{
		JoeShadow *Attrib2 = (JoeShadow *)Attrib;

		if (Attrib2->Shadow == (ShadowEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		{
		JoeFoliage *Attrib2 = (JoeFoliage *)Attrib;

		if (Attrib2->Foliage == (FoliageEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		{
		JoeStream *Attrib2 = (JoeStream *)Attrib;

		if (Attrib2->Stream == (StreamEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_STREAM
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		{
		JoeObject3D *Attrib2 = (JoeObject3D *)Attrib;

		if (Attrib2->Object == (Object3DEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
	case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		{
		JoeTerrainParam *Attrib2 = (JoeTerrainParam *)Attrib;

		if (Attrib2->TerrainPar == (TerrainParamEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		{
		JoeGround *Attrib2 = (JoeGround *)Attrib;

		if (Attrib2->Ground == (GroundEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_GROUND
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		{
		JoeSnow *Attrib2 = (JoeSnow *)Attrib;

		if (Attrib2->SnowBusinessLikeShowBusiness == (SnowEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SNOW
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		{
		JoeEnvironment *Attrib2 = (JoeEnvironment *)Attrib;

		if (Attrib2->Env == (EnvironmentEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		{
		JoeCloud *Attrib2 = (JoeCloud *)Attrib;

		if (Attrib2->Cloud == (CloudEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		{
		JoeWave *Attrib2 = (JoeWave *)Attrib;

		if (Attrib2->Wave == (WaveEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_WAVE
	case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		{
		JoeCmap *Attrib2 = (JoeCmap *)Attrib;

		if (Attrib2->Cmap == (CmapEffect *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CMAP
	case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		{
		JoeThematicMap *Attrib2 = (JoeThematicMap *)Attrib;

		if (Attrib2->Theme == (ThematicMap *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
	case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		{
		JoeCoordSys *Attrib2 = (JoeCoordSys *)Attrib;

		if (Attrib2->Coord == (CoordSys *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		{
		JoeFence *Attrib2 = (JoeFence *)Attrib;

		if (Attrib2->Fnce == (Fence *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FENCE
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		{
		JoeLabel *Attrib2 = (JoeLabel *)Attrib;

		if (Attrib2->Labl == (Label *)MatchEffect)
			return (Attrib);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LABEL
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector
	default:
		break;
	} // switch

return (NULL);

} // Joe::MatchEffectAttribute

/*===========================================================================*/

short Joe::GetEffectPriority(unsigned char MinorAttrib)
{
JoeAttribute *Search = NULL, *Found = NULL;;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(-100); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	if ((AttribInfo.FirstAttrib->MajorAttribType == WCS_JOE_ATTRIB_INTERNAL) &&
	 (AttribInfo.FirstAttrib->MinorAttribType == MinorAttrib))
		{
		Found = AttribInfo.FirstAttrib;
		return(Found->GetEffectPriority());
		} // if
	else
		{
		for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
			{
			if (Search->NextAttrib)
				{
				if ((Search->NextAttrib->MajorAttribType == WCS_JOE_ATTRIB_INTERNAL) &&
				 (Search->NextAttrib->MinorAttribType == MinorAttrib))
					{
					Found = Search->NextAttrib;
					return(Found->GetEffectPriority());
					} // if
				} // if
			else
				{
				return(-100); // Couldn't find it
				} // else
			} // for
		return(-100);
		} // else

	} // if
else
	{
	return(-100);
	} // else

} // Joe::GetEffectPriority

/*===========================================================================*/

short Joe::GetEffectEvalOrder(unsigned char MinorAttrib)
{
JoeAttribute *Search = NULL, *Found = NULL;;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(1); // Attributes are void if kids present
	} // if

if (AttribInfo.FirstAttrib)
	{
	if ((AttribInfo.FirstAttrib->MajorAttribType == WCS_JOE_ATTRIB_INTERNAL) &&
	 (AttribInfo.FirstAttrib->MinorAttribType == MinorAttrib))
			{
			Found = AttribInfo.FirstAttrib;
			return(Found->GetEffectEvalOrder());
			} // if
	else
			{
			for (Search = AttribInfo.FirstAttrib; Search; Search = Search->NextAttrib)
					{
					if (Search->NextAttrib)
							{
							if ((Search->NextAttrib->MajorAttribType == WCS_JOE_ATTRIB_INTERNAL) &&
							 (Search->NextAttrib->MinorAttribType == MinorAttrib))
									{
									Found = Search->NextAttrib;
									return(Found->GetEffectEvalOrder());
									} // if
							} // if
					else
							{
							return(1); // Couldn't find it
							} // else
					} // for
			return(1);
			} // else

	} // if
else
	{
	return(1);
	} // else

} // Joe::GetEffectEvalOrder

/*===========================================================================*/

short JoeAttribute::GetEffectPriority(void)
{

switch (MinorAttribType)
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		{
		JoeLake *Attrib = (JoeLake *)this;

		if (Attrib->Lake)
			return (Attrib->Lake->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LAKE
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		{
		JoeEcosystem *Attrib = (JoeEcosystem *)this;

		if (Attrib->Eco)
			return (Attrib->Eco->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
	case WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION:
		{
		// obsolete
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		JoeRasterTA *Attrib = (JoeRasterTA *)this;

		if (Attrib->Terra)
			return (Attrib->Terra->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		JoeTerraffector *Attrib = (JoeTerraffector *)this;

		if (Attrib->Terra)
			return (Attrib->Terra->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		{
		JoeShadow *Attrib = (JoeShadow *)this;

		if (Attrib->Shadow)
			return (Attrib->Shadow->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		{
		JoeFoliage *Attrib = (JoeFoliage *)this;

		if (Attrib->Foliage)
			return (Attrib->Foliage->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		{
		JoeStream *Attrib = (JoeStream *)this;

		if (Attrib->Stream)
			return (Attrib->Stream->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_STREAM
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		{
		JoeObject3D *Attrib = (JoeObject3D *)this;

		if (Attrib->Object)
			return (Attrib->Object->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
	case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		{
		JoeTerrainParam *Attrib = (JoeTerrainParam *)this;

		if (Attrib->TerrainPar)
			return (Attrib->TerrainPar->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		{
		JoeGround *Attrib = (JoeGround *)this;

		if (Attrib->Ground)
			return (Attrib->Ground->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_GROUND
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		{
		JoeSnow *Attrib = (JoeSnow *)this;

		if (Attrib->SnowBusinessLikeShowBusiness)
			return (Attrib->SnowBusinessLikeShowBusiness->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SNOW
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		{
		JoeEnvironment *Attrib = (JoeEnvironment *)this;

		if (Attrib->Env)
			return (Attrib->Env->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		{
		JoeCloud *Attrib = (JoeCloud *)this;

		if (Attrib->Cloud)
			return (Attrib->Cloud->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		{
		JoeWave *Attrib = (JoeWave *)this;

		if (Attrib->Wave)
			return (Attrib->Wave->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_WAVE
	case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		{
		JoeCmap *Attrib = (JoeCmap *)this;

		if (Attrib->Cmap)
			return (Attrib->Cmap->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CMAP
	case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		{
		JoeThematicMap *Attrib = (JoeThematicMap *)this;

		if (Attrib->Theme)
			return (Attrib->Theme->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
	case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		{
		JoeCoordSys *Attrib = (JoeCoordSys *)this;

		if (Attrib->Coord)
			return (Attrib->Coord->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		{
		JoeFence *Attrib = (JoeFence *)this;

		if (Attrib->Fnce)
			return (Attrib->Fnce->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FENCE
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		{
		JoeLabel *Attrib = (JoeLabel *)this;

		if (Attrib->Labl)
			return (Attrib->Labl->Priority);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LABEL
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector
	default:
		break;
	} // switch

return (-100);

} // JoeAttribute::GetEffectPriority

/*===========================================================================*/

short JoeAttribute::GetEffectEvalOrder(void)
{

switch (MinorAttribType)
	{
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		JoeRasterTA *Attrib = (JoeRasterTA *)this;

		if (Attrib->Terra)
			return (Attrib->Terra->EvalOrder);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		JoeTerraffector *Attrib = (JoeTerraffector *)this;

		if (Attrib->Terra)
			return (Attrib->Terra->EvalOrder);
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
// <<<>>> ADD_NEW_EFFECTS if the effect has a certain evaluation order
	default:
		break;
	} // switch

return (1);

} // JoeAttribute::GetEffectEvalOrder

/*===========================================================================*/

JoeAttribute *Joe::RemoveFirstAttribute(void)
{
JoeAttribute *First = NULL;

if (AttribInfo.FirstAttrib)
	{
	First = AttribInfo.FirstAttrib;
	AttribInfo.FirstAttrib = AttribInfo.FirstAttrib->NextAttrib;
	} // if

return(First);

} // Joe::RemoveFirstAttribute

/*===========================================================================*/

JoeAttribute *Joe::GetFirstAttribute(void)
{
JoeAttribute *First = NULL;

if (AttribInfo.FirstAttrib)
	{
	First = AttribInfo.FirstAttrib;
	} // if

return(First);

} // Joe::GetFirstAttribute

/*===========================================================================*/

LayerStub *Joe::AddObjectToLayer(LayerEntry *MyEntry)
{
LayerStub *LinkStub;

if (MyEntry && Parent)
	{
	if (!(LinkStub = Parent->MatchEntryToStub(MyEntry)))
			{
			// Need to add MasterStub to my parent Joe
			LinkStub = new LayerStub;
			if (LinkStub)
					{
					LinkStub->ObjectReferredTo = Parent;
					LinkStub->ThisObjectsLayer = MyEntry;
					LinkStub->PrevObjectStubSameLayer = NULL;
					LinkStub->NextObjectStubSameLayer = MyEntry->ChainStart;
					MyEntry->ChainStart = LinkStub;
					LinkStub->NextStubSameObject = Parent->LayerList;
					Parent->LayerList = LinkStub;
					} // if
			else
					{
					return(NULL);
					} // else
			} // if
	return(AddObjectToLayer(LinkStub));
	} // if
else
	{
	return(NULL);
	} // else

} // Joe::AddObjectToLayer(LayerEntry)

/*===========================================================================*/

LayerStub *Joe::AddObjectToLayer(LayerStub *MasterStub)
{
LayerStub *Me;

if (MasterStub)
	{
	Me = new LayerStub;
	if (Me)
			{
			Me->ThisObjectsLayer = MasterStub->ThisObjectsLayer;
			Me->ObjectReferredTo = this;

			#ifdef WCS_BUILD_V3
			// new tail-append code
			if (LayerList)
				{ // append at end
				for (LayerStub *AppendScan = LayerList; AppendScan; AppendScan = AppendScan->NextStubSameObject)
					{
					if (AppendScan->NextStubSameObject == NULL)
						{
						// this is the end of the line, append here
						Me->NextStubSameObject = NULL;
						AppendScan->NextStubSameObject = Me;
						break;
						} // if
					} // for
				} // if
			else
				{ // nothing here yet, simpler insert-at-head
				Me->NextStubSameObject = LayerList;
				LayerList = Me;
				} // else

			#else // !WCS_BUILD_V3 must be VNS 2 or 1 or WCS
			// old head-insert code
			Me->NextStubSameObject = LayerList;
			LayerList = Me;
			#endif // !WCS_BUILD_V3

			Me->PrevObjectStubSameLayer = MasterStub;
			if (Me->NextObjectStubSameLayer = MasterStub->NextObjectStubSameLayer)
				{
				Me->NextObjectStubSameLayer->PrevObjectStubSameLayer = Me;
				} // if
			MasterStub->NextObjectStubSameLayer = Me;
			//return(MasterStub);
			// Seems like we need to return "Me" here, really.
			return(Me);
			} // if
	else
			{
			return(NULL);
			} //else
	} // if
else
	{
	return(NULL);
	} // else

} //Joe::AddObjectToLayer(LayerStub)

/*===========================================================================*/

LayerStub *Joe::MatchEntryToStub(LayerEntry *Me)
{
LayerStub *Search;

if (Me)
	{
	for (Search = LayerList; Search; Search = Search->NextStubSameObject)
			{
			if (Search->ThisObjectsLayer == Me)
					{
					return(Search);
					} // if
			} // for
	return(NULL); // Couldn't find yer layer, sorry...
	} // if
else
	{
	return(NULL);
	} // else

} // Joe::MatchEntryToStub

/*===========================================================================*/

int Joe::MatchEntryToChildStubs(LayerEntry *Me)
{
Joe *Walk;

for (Walk = GroupInfo.JChild; Walk; Walk = Walk->NextJoe)
	{
	if (Walk->MatchEntryToStub(Me))
		return (1);
	} // for
for (Walk = GroupInfo.GroupChild; Walk; Walk = Walk->NextJoe)
	{
	if (Walk->MatchEntryToStub(Me))
		return (1);
	} // for

return (0);

} // Joe::MatchEntryToChildStubs

/*===========================================================================*/

LayerStub *Joe::NextLayer(LayerStub *PrevLayer)
{

if (PrevLayer)
	{
	return(PrevLayer->NextLayerInObject());
	} // if

return(NULL);

} // Joe::NextLayer

/*===========================================================================*/

Joe *Joe::RemoveObjectFromLayer(int FullCleanup, LayerEntry *Me, LayerStub *Him)
{
LayerStub *Search = NULL, *Found = NULL;
LayerEntry *TheLayerEntry;
Joe *TheObject;

if (LayerList && (Me || Him))
	{
	if ((LayerList->ThisObjectsLayer == Me) ||
	 (LayerList == Him))
		{
		Found = LayerList;
		LayerList = LayerList->NextStubSameObject;
		} // if
	if (!Found)
		{
		for (Search = LayerList; Search; Search = Search->NextStubSameObject)
			{
			if (Search->NextStubSameObject)
				{
				if ((Search->NextStubSameObject->ThisObjectsLayer == Me) ||
				 (Search->NextStubSameObject == Him))
					{
					Found = Search->NextStubSameObject;
					Search->NextStubSameObject = Found->NextStubSameObject;
					break;
					} // if
				} // if
			} // for
		} // if
	if (Found)
		{
		// Unlink from brethern
		if (TheLayerEntry = Found->ThisObjectsLayer)
			{
			if (TheLayerEntry->ChainStart == Found)
				{
				TheLayerEntry->ChainStart = Found->NextObjectStubSameLayer;
				} // if
			} // if
		// Make sure layerlist is still connected
		if (LayerList == Found)
			{
			LayerList = Found->NextStubSameObject;
			} // if
		if (Found->NextObjectStubSameLayer)
			{
			Found->NextObjectStubSameLayer->PrevObjectStubSameLayer = Found->PrevObjectStubSameLayer;
			} // if
		if (Found->PrevObjectStubSameLayer)
			{
			Found->PrevObjectStubSameLayer->NextObjectStubSameLayer = Found->NextObjectStubSameLayer;
			} // if
		delete Found; // Blam!

		// are there any more Joes attached to the layer in question?
		// is it attached to the parent?
		// does the parent have any children with it? if not, remove from parent
		// if there is only one attachment, is it StaticRoot?
		// if so, then detach the layer from StaticRoot too
		// LayerEntry must be removed outside this function scope where there is access to the LayerTable
		if (TheLayerEntry && FullCleanup)
			{
			// remove from parent
			if (Parent && TheLayerEntry->ChainStart && (Found = Parent->MatchEntryToStub(TheLayerEntry)))
				{
				// test if parent has any children with this layer
				if (! Parent->MatchEntryToChildStubs(TheLayerEntry))
					{
					// Unlink from brethern
					if (Found->ThisObjectsLayer)
						{
						if (Found->ThisObjectsLayer->ChainStart == Found)
							{
							Found->ThisObjectsLayer->ChainStart = Found->NextObjectStubSameLayer;
							} // if
						} // if
					// Make sure layerlist is still connected
					if (Parent->LayerList == Found)
						{
						Parent->LayerList = Found->NextStubSameObject;
						} // if
					else
						{
						for (Search = Parent->LayerList; Search; Search = Search->NextStubSameObject)
							{
							if (Search->NextStubSameObject)
								{
								if (Search->NextStubSameObject == Found)
									{
									Search->NextStubSameObject = Found->NextStubSameObject;
									break;
									} // if
								} // if
							} // for
						} // else
					if (Found->NextObjectStubSameLayer)
						{
						Found->NextObjectStubSameLayer->PrevObjectStubSameLayer = Found->PrevObjectStubSameLayer;
						} // if
					if (Found->PrevObjectStubSameLayer)
						{
						Found->PrevObjectStubSameLayer->NextObjectStubSameLayer = Found->NextObjectStubSameLayer;
						} // if
					delete Found; // Blam!
					} // if
				} // if
			// remove from static root
			if (TheLayerEntry->ChainStart && ! TheLayerEntry->ChainStart->NextObjectStubSameLayer)
				{
				if (TheObject = TheLayerEntry->ChainStart->ObjectReferredTo)
					{
					if (! stricmp(TheObject->GetBestName(), "StaticRoot"))
						{
						Found = TheLayerEntry->ChainStart;
						// Unlink from brethern
						if (Found->ThisObjectsLayer)
							{
							if (Found->ThisObjectsLayer->ChainStart == Found)
								{
								Found->ThisObjectsLayer->ChainStart = Found->NextObjectStubSameLayer;
								} // if
							} // if
						// Make sure layerlist is still connected
						if (TheObject->LayerList == Found)
							{
							TheObject->LayerList = Found->NextStubSameObject;
							} // if
						else
							{
							for (Search = TheObject->LayerList; Search; Search = Search->NextStubSameObject)
								{
								if (Search->NextStubSameObject)
									{
									if (Search->NextStubSameObject == Found)
										{
										Search->NextStubSameObject = Found->NextStubSameObject;
										break;
										} // if
									} // if
								} // for
							} // else
						if (Found->NextObjectStubSameLayer)
							{
							Found->NextObjectStubSameLayer->PrevObjectStubSameLayer = Found->PrevObjectStubSameLayer;
							} // if
						if (Found->PrevObjectStubSameLayer)
							{
							Found->PrevObjectStubSameLayer->NextObjectStubSameLayer = Found->NextObjectStubSameLayer;
							} // if
						delete Found; // Blam!
						} // if
					} // if
				} // if
			} // if
		return(this);
		} // if
	return(NULL); // didn't find that layer, sorry
	} // if
return(NULL);

} // Joe::RemoveObjectFromLayer

/*===========================================================================*/

LayerStub *Joe::MatchLayer(LayerEntry *MatchMe)
{
LayerStub *Search = NULL;

for (Search = LayerList; Search; Search = Search->NextStubSameObject)
	{
	if (Search->ThisObjectsLayer == MatchMe)
		{
		break;
		} // if
	} // for

return (Search);

} // Joe::MatchLayer

/*===========================================================================*/

LayerEntry *Joe::GetMultiPartLayer(void)
{
LayerStub *Search = NULL;

for (Search = LayerList; Search; Search = Search->NextStubSameObject)
	{
	if (Search->ThisObjectsLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		return (Search->ThisObjectsLayer);
		} // if
	} // for

return (NULL);

} // Joe::GetMultiPartLayer

/*===========================================================================*/

//struct TestAttribSpec
//        {
//        unsigned short LineStyle;
//        unsigned char LineWidth, PointStyle, RedGun, GreenGun,
//         BlueGun;
//        JoeAttribute *FirstAttrib;
//        unsigned short CanonicalTypeMajor, CanonicalTypeMinor;
//        } AttribInfo; // Part two of union

/*===========================================================================*/

void *Joe::WriteToFile(FILE *SaveFile, void *FileBuf)
{
LayerStub *LayerLook;
VectorPoint *PointScan;
unsigned char *WriteBuf;
JoeAttribute *AttribCheck;
//struct TestAttribSpec *TAS;
unsigned long PreCalcSize, NonAttribSize, GenCounter, WriteBufSize = 0, LayersToWrite, Out32;
#ifdef DATABASE_CHUNK_DEBUG
static unsigned long joeNum = 0;
//unsigned long rote;
#endif // DATABASE_CHUNK_DEBUG
unsigned short Out16;
unsigned long MaskFlags;
//char EndMark;

if (FileBuf)
	{  // First four bytes contain the size of the buffer
	WriteBufSize = (*  ((long *)FileBuf)) - 4;
	WriteBuf = &((unsigned char *)FileBuf)[4]; // Don't look at it if it hurts...
	} // if

if (SaveFile)
	{
	// First calculate the size of the object...
	//PreCalcSize = 25; // ObjSize, Fixed header stuff, Name stringsize
	// Groups now 16-bytes bigger because the 4 4-byte floats grew by 4 bytes each to 4 8-byte doubles
	// Non-Groups now 16-bytes smaller because the 4 4-byte floats went away
	if (TestFlags(WCS_JOEFLAG_HASKIDS) || TestFlags(WCS_JOEFLAG_ISDEM))
		{
		PreCalcSize = 41; // ObjSize, Fixed header stuff (Flags, bounds), Name stringsize
		} // if
	else
		{
		PreCalcSize = 9; // ObjSize, Flags, Name stringsize (no bounds)
		} // else
	if (Name())
		{
		//printf("Name: %s\n", Name());
		PreCalcSize += (unsigned long)(strlen(Name()) + 1); // Name and NULL
		//printf("%d + ", strlen(Name()) + 1);
		} // if
	//printf("21 + ");
	if (NumPoints())
		{
		PreCalcSize += 5; // Points counter + tag
		PreCalcSize += (NumPoints() * 20); // Point entries
		//printf("%d + ", 5 + (NumPoints * 20));
		} // if
	if (TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (!TestFlags(WCS_JOEFLAG_EXTFILE))
			{
			if (GroupInfo.NumKids + GroupInfo.NumGroups)
				{
				PreCalcSize += 9; // group count, bound count, offset, tag
				//printf("9 + ");
				} // if
			} // if
		} // if
	if (FileName())
		{
		if (GenCounter = (unsigned long)strlen(FileName()))
			{
			//printf("%d + ", GenCounter + 3);
			PreCalcSize += (GenCounter + 3); // string, tag, null
			} // if
		} // if
	#ifdef WCS_BUILD_VNS
	// support for scenarios
	if (UniqueLoadSaveID)
		{
		PreCalcSize += 6; // tag, one-byte size, ID
		} // if
	#endif // WCS_BUILD_VNS

	GenCounter = 0;
	// Don't write layers on group objects
	if (!TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		for (LayerLook = FirstLayer(); LayerLook; GenCounter++)
			{
			LayerLook = NextLayer(LayerLook);
			} // for
		} //if
	LayersToWrite = GenCounter;
	if (GenCounter)
		{
		GenCounter++; // Add one addition unit size for size of counter itself
		if (GlobalApp->AppDB->DBLayers.HowManyLayers() < 256)
			{
			PreCalcSize += GenCounter;  // 8-bit mode
			} // if
		else if (GlobalApp->AppDB->DBLayers.HowManyLayers() < 65536)
			{
			PreCalcSize += (GenCounter * 2); // 16-bit mode
			} // else if
		else
			{
			PreCalcSize += (GenCounter * 4); // 32-bit mode
			} // else
		PreCalcSize += 1; // tag
		} // if

	#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
	GenCounter = 0;
	// We don't write layers or attributes on group objects
	if (!TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		for (LayerLook = FirstLayer(); LayerLook;)
			{
			// only count attrib-flagged layers here
			if (LayerLook->ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
				{
				if (LayerLook->ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
					{
					GenCounter += (1 + 4 + 4 + LayerLook->LAttrib.TextVal.TextBufLen);
					} // if
				else
					{
					GenCounter += (1 + 4 + 8);
					} // else
				} // if
			LayerLook = NextLayer(LayerLook);
			} // for
		} // if
	PreCalcSize += GenCounter;  // attribs
	#endif // WCS_SUPPORT_GENERIC_ATTRIBS

	NonAttribSize = PreCalcSize;
	if (!TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		//printf("9 + ");
		PreCalcSize += 9; // Lineinfo tag and data
		PreCalcSize += (1 + 1 + 8); // CanonType:Tag, Count, CanonAttrib

		NonAttribSize = PreCalcSize;
		// Calculate extended Attribute size
		for (AttribCheck = AttribInfo.FirstAttrib; AttribCheck; AttribCheck = AttribCheck->NextAttrib)
			{
			// We now evaluate this at save-time instead of just reading a variable.
			PreCalcSize += (unsigned long)AttribCheck->GetFileSize();
			} // for

		} // if

	// Check on buffer size
	if (WriteBufSize < NonAttribSize)
		{
		if (FileBuf)
			{ // free old buffer
			AppMem_Free(FileBuf, WriteBufSize + 4);
			FileBuf = NULL;
			} // if
		if (FileBuf = AppMem_Alloc(NonAttribSize + 4, NULL))
			{
			*  ((unsigned long *)FileBuf) = NonAttribSize + 4;
			WriteBuf = (unsigned char *)FileBuf;
			WriteBuf = &WriteBuf[4]; // skip over size counter
			} // if
		else
			{
			return(NULL); // or run lomem code, not yet written
			} // else
		} // if

	//EndMark = WriteBuf[NonAttribSize];

	Out32 = PreCalcSize - 4;
	memcpy(WriteBuf, &Out32, 4); WriteBuf += 4;
	MaskFlags = Flags & ~(WCS_JOEFLAG_NAMEREPLACED | WCS_JOEFLAG_MODIFIED | WCS_JOEFLAG_NEWRETRYCACHE);
	if (!(Flags & WCS_JOEFLAG_ISDEM))
		{
		MaskFlags |= WCS_JOEFLAG_VEC_SPFP_ELEV;
		} // if
	memcpy(WriteBuf, &MaskFlags, 4); WriteBuf += 4; // Flags
	if (TestFlags(WCS_JOEFLAG_HASKIDS) || TestFlags(WCS_JOEFLAG_ISDEM))
		{ // only write bounds for Groups and DEMs
		memcpy(WriteBuf, &NWLat, sizeof(double)); WriteBuf += sizeof(double); // NWLat
		memcpy(WriteBuf, &NWLon, sizeof(double)); WriteBuf += sizeof(double); // NWLon
		memcpy(WriteBuf, &SELat, sizeof(double)); WriteBuf += sizeof(double); // SELat
		memcpy(WriteBuf, &SELon, sizeof(double)); WriteBuf += sizeof(double); // SELon
		} // if
	//printf("\n");

	// NAME (required variable-length field)
	if (Name())
		{
		if (GenCounter = (unsigned long)strlen(Name())) // Name length (includes NULL)
			{
			GenCounter ++;
			} // if
		WriteBuf[0] = (unsigned char)GenCounter; WriteBuf ++;
		if (GenCounter)
			{
			memcpy(WriteBuf, Name(), GenCounter); WriteBuf += GenCounter; // Name
			} // if
		} // if
	else
		{
		WriteBuf[0] = 0; WriteBuf++;  // No name
		} // else

	//printf("NE:%d ", WriteBuf - TempBuf);

	// FILENAME (optional tag field)
	GenCounter = 0;
	if (FileName())
		{
		GenCounter = (unsigned long)strlen(FileName());
		} // if
	if (GenCounter)
		{
		WriteBuf[0] = WCS_JOEFILE_FILENAME; WriteBuf++; // FILENAME tag
		GenCounter++; // Account for NULL
		WriteBuf[0] = (unsigned char)GenCounter; WriteBuf ++; // Length
		memcpy(WriteBuf, FileName(), GenCounter); WriteBuf += GenCounter; // FileName
		} // if

	//printf("FE:%d ", WriteBuf - TempBuf);

	// LAYERSYMBOL (optional tag field)
	GenCounter = 0; // This is important
	if (LayersToWrite)
			{
			LayerLook = FirstLayer();
			if (GlobalApp->AppDB->DBLayers.HowManyLayers() < 256) // I know it's different from above
				{
				WriteBuf[0] = WCS_JOEFILE_LAYERSYM8; //Tag for 8-bit LayerTable
				WriteBuf[1] = (unsigned char)LayersToWrite; // 8-bit count
				WriteBuf += 2;
				for (GenCounter = 0; GenCounter < LayersToWrite; GenCounter++)
					{
					if (LayerLook)
						{
						WriteBuf[0] = (unsigned char)((LayerLook->MyLayer())->GetLayerNum());
						WriteBuf++;
						} // if
					else
						{
						break; // Error, ran out of layers
						} // else
					LayerLook = LayerLook->NextLayerInObject();
					} // for
				} // if
			else if (GlobalApp->AppDB->DBLayers.HowManyLayers() < 65536)
				{
				WriteBuf[0] = WCS_JOEFILE_LAYERSYM16; // 16-bit LayerTable tag
				Out16 = (unsigned short)LayersToWrite;
				memcpy(&WriteBuf[1], &Out16, 2); // 16-bit count
				WriteBuf += 3;
				for (GenCounter = 0; GenCounter < LayersToWrite; GenCounter++)
					{
					if (LayerLook)
						{
						Out16 = (unsigned short)((LayerLook->MyLayer())->GetLayerNum());
						memcpy(WriteBuf, &Out16, 2); // Write Layer entry
						WriteBuf += 2;
						} // if
					else
						{
						break; // Error, ran out of layers
						} // else
					LayerLook = LayerLook->NextLayerInObject();
					} // for
				} // else if
			else // LayersToWrite => 65536
				{
				WriteBuf[0] = WCS_JOEFILE_LAYERSYM32;
				memcpy(&WriteBuf[1], &LayersToWrite, 4);
				WriteBuf += 5;
				for (GenCounter = 0; GenCounter < LayersToWrite; GenCounter++)
					{
					if (LayerLook)
						{
						Out32 = (unsigned short)((LayerLook->MyLayer())->GetLayerNum());
						memcpy(WriteBuf, &Out32, 4); // Write Layer entry
						WriteBuf += 4;
						} // if
					else
						{
						break; // Error, ran out of layers
						} // else
					LayerLook = LayerLook->NextLayerInObject();
					} // for
				} // else
			//printf("LE:%d ", WriteBuf - TempBuf);
			} // if
	#ifdef WCS_BUILD_VNS
	// support for scenarios
	if (UniqueLoadSaveID)
		{
		WriteBuf[0] = WCS_JOEFILE_UNIQUEID; WriteBuf++; // UniqueID tag
		WriteBuf[0] = 4; WriteBuf++; // 4 bytes = size of remaining data (after WCS_JOEFILE_UNIQUEID tag and this size byte)
		Out32 = UniqueLoadSaveID;
		memcpy(WriteBuf, &Out32, 4); WriteBuf += 4; // Group Count;
		//printf("UID:%x ", UniqueLoadSaveID);
		} // if
	#endif // WCS_BUILD_VNS


	// Maybe do Generic Attributes (optional tag field)
	if (GenCounter == LayersToWrite)
	#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
		{
		if (!TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			for (LayerLook = FirstLayer(); LayerLook; LayerLook = LayerLook->NextLayerInObject())
				{
				if (LayerLook->ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
					{
					if (LayerLook->ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
						{
						WriteBuf[0] = WCS_JOEFILE_TEXTLAYERATTRIB;
						Out32 = (unsigned short)((LayerLook->MyLayer())->GetLayerNum());
						memcpy(&WriteBuf[1], &Out32, 4); // Write Layer entry
						memcpy(&WriteBuf[5], &LayerLook->LAttrib.TextVal.TextBufLen, 4); // Write text length
						memcpy(&WriteBuf[9], LayerLook->LAttrib.TextVal.TextBuf, LayerLook->LAttrib.TextVal.TextBufLen); // Write actual text
						WriteBuf += (1 + 4 + 4 + LayerLook->LAttrib.TextVal.TextBufLen);
#ifdef DATABASE_CHUNK_DEBUG
						sprintf(debugStr, "wrote Joe::WriteToFile(#%u - WCS_JOEFILE_TEXTLAYERATTRIB): %u\n", ++joeNum, 1 + 4 + 4 + LayerLook->LAttrib.TextVal.TextBufLen);
						OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
						} // if
					else
						{
						WriteBuf[0] = WCS_JOEFILE_IEEELAYERATTRIB;
						Out32 = (unsigned short)((LayerLook->MyLayer())->GetLayerNum());
						memcpy(&WriteBuf[1], &Out32, 4); // Write Layer entry
						memcpy(&WriteBuf[5], &LayerLook->LAttrib.IEEEVal, 8); // Write actual IEEE double in native order
						WriteBuf += (1 + 4 + 8);
#ifdef DATABASE_CHUNK_DEBUG
						sprintf(debugStr, "wrote Joe::WriteToFile(#%u - WCS_JOEFILE_IEEELAYERATTRIB): %u\n", ++joeNum, 1 + 4 + 8);
						OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
						} // else
					} // if
				} // for
			} // if
		//printf("GA:%d ", WriteBuf - TempBuf);
		} // if

	if (1) //if (GenCounter == LayersToWrite)
	#endif // WCS_SUPPORT_GENERIC_ATTRIBS
		{
		if (TestFlags(WCS_JOEFLAG_HASKIDS)) // BOUND (optional)
			{
			if (!TestFlags(WCS_JOEFLAG_EXTFILE))
				{
				if (GroupInfo.NumKids + GroupInfo.NumGroups)
					{
					WriteBuf[0] = WCS_JOEFILE_BOUND; WriteBuf++; // Bounds tag
					Out32 = GroupInfo.NumGroups;
					memcpy(WriteBuf, &Out32, 4); WriteBuf += 4; // Group Count;
					Out32 = Utility;
					memcpy(WriteBuf, &Out32, 4); WriteBuf += 4; // Offset
					//printf("BE:%d ", WriteBuf - TempBuf);
					} // if
				} // if
			} // if
		else // LINEINFO (optional)
			{
			WriteBuf[0] = WCS_JOEFILE_LINEINFO; // LINEINFO tag
			WriteBuf[1] = AttribInfo.LineWidth;
			WriteBuf[2] = AttribInfo.PointStyle;
			memcpy(&WriteBuf[3], &AttribInfo.LineStyle, 2);
			WriteBuf[5] = AttribInfo.RedGun;
			WriteBuf[6] = AttribInfo.GreenGun;
			WriteBuf[7] = AttribInfo.BlueGun;
			// no more DrawPen, but we must still write a value
			WriteBuf[8] = 0; // AttribInfo.DrawPen;
			WriteBuf += 9;
			//printf("LE:%d ", WriteBuf - TempBuf);

			// ATTRIBINFO (Canonical Types)
			WriteBuf[0] = WCS_JOEFILE_ATTRIBINFO;
			WriteBuf[1] = 2; // Only support two now, could support up to 255
			WriteBuf += 2;
			memcpy(WriteBuf, &AttribInfo.CanonicalTypeMajor, 2); WriteBuf += 2;
			memcpy(WriteBuf, &AttribInfo.CanonicalTypeMinor, 2); WriteBuf += 2;
			memcpy(WriteBuf, &AttribInfo.SecondTypeMajor, 2); WriteBuf += 2;
			memcpy(WriteBuf, &AttribInfo.SecondTypeMinor, 2); WriteBuf += 2;
			} // else

		// POINTS (optional variable tag)
		if (NumPoints())
			{
			WriteBuf[0] = WCS_JOEFILE_POINTS; WriteBuf++; // POINTS tag
			memcpy(WriteBuf, &nNumPoints, 4); WriteBuf += 4; // Point count
			// We assume the count we just wrote was correct.
			for (PointScan = Points(); PointScan; PointScan = PointScan->Next)
				{
				memcpy(WriteBuf, &PointScan->Latitude, 8); WriteBuf += 8;
				memcpy(WriteBuf, &PointScan->Longitude, 8); WriteBuf += 8;
				memcpy(WriteBuf, &PointScan->Elevation, 4); WriteBuf += 4;
				} // for
			//printf("PE:%d ", WriteBuf - TempBuf);
			} // if

		WriteBuf = (unsigned char *)FileBuf;
		if (fwrite(&WriteBuf[4], NonAttribSize, 1, SaveFile))
			{
#ifdef DATABASE_CHUNK_DEBUG
			sprintf(debugStr, "wrote Joe::WriteToFile(#%u [%s/%s] - NonAttribSize): %u\n", ++joeNum, Name(), FName, NonAttribSize);
			OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
			if (!TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				// Write any extended attributes. Not buffered the same way.
				//TAS = (struct TestAttribSpec *)&AttribInfo;
				for (AttribCheck = AttribInfo.FirstAttrib; AttribCheck; AttribCheck = AttribCheck->NextAttrib)
					{
					AttribCheck->WriteToFile(SaveFile);
					} // for
				} // if
			return(FileBuf); // Success!
			} // if
		} // if
	} // if

// Error, clean up
#ifdef DATABASE_CHUNK_DEBUG
sprintf(debugStr, "FAILED Joe::WriteToFile(NonAttribSize\n");
OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
WriteFileCleanUp(FileBuf);

return(NULL);

} // Joe::WriteToFile

/*===========================================================================*/

void Joe::WriteFileCleanUp(void *Buffer)
{
unsigned long *BufSize;

if (Buffer)
	{
	BufSize = (unsigned long *)Buffer;
	AppMem_Free(Buffer, *BufSize);
	} // if

} // Joe::WriteFileCleanUp

/*===========================================================================*/

Joe *Joe::Descend(void)
{
Joe *Drop;

for (Drop = GroupInfo.GroupChild; Drop;)
	{
	if (Drop->GroupInfo.GroupChild)
		{
		Drop = Drop->GroupInfo.GroupChild;
		} // if
	else
		{
		return(Drop);
		} // else
	} // for

return(this);

} // Joe::Descend

/*===========================================================================*/

Joe *Joe::BottomWalkUpGroup(void)
{

if (NextJoe)
	{
	return(NextJoe->Descend());
	} // if
else
	{
	return(Parent);
	} // else

} // Joe::BottomWalkUpGroup

/*===========================================================================*/

void Joe::UpdateGroupBounds()
{
Joe *Walk, *SnipUp, *SnipSide;

SnipUp = Parent;
Parent = NULL;
SnipSide = NextJoe;
NextJoe = NULL;
//const char *WalkName;

//WalkName = GetBestName();

if (GroupInfo.GroupChild)
	{
	for (Walk = this->Descend(); Walk;)
		{
		//WalkName = Walk->GetBestName();
		if (Walk)
			{
			if (Walk->Parent)
				{
				if (Walk->NWLat > Walk->Parent->NWLat)
					{
					Walk->Parent->NWLat = Walk->NWLat;
					} // if
				if (Walk->NWLon > Walk->Parent->NWLon)
					{
					Walk->Parent->NWLon = Walk->NWLon;
					} // if
				if (Walk->SELat < Walk->Parent->SELat)
					{
					Walk->Parent->SELat = Walk->SELat;
					} // if
				if (Walk->SELon < Walk->Parent->SELon)
					{
					Walk->Parent->SELon = Walk->SELon;
					} // if
				} // if
			Walk = Walk->BottomWalkUpGroup();
			} // if
		} // if
	GroupInfo.GroupChild->Parent = this;
	} // if

Parent = SnipUp;
NextJoe = SnipSide;

for (Walk = this; Walk; Walk = Walk->Parent)
	{
	if (Walk->Parent)
		{
		if (Walk->NWLat > Walk->Parent->NWLat)
			{
			Walk->Parent->NWLat = Walk->NWLat;
			} // if
		if (Walk->NWLon > Walk->Parent->NWLon)
			{
			Walk->Parent->NWLon = Walk->NWLon;
			} // if
		if (Walk->SELat < Walk->Parent->SELat)
			{
			Walk->Parent->SELat = Walk->SELat;
			} // if
		if (Walk->SELon < Walk->Parent->SELon)
			{
			Walk->Parent->SELon = Walk->SELon;
			} // if
		} // if
	} // for

} // Joe::UpdateGroupBounds

/*===========================================================================*/

void Joe::SetCanonType(unsigned short Major, unsigned short Minor)
{

if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventName, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.CanonicalTypeMajor = Major;
	AttribInfo.CanonicalTypeMinor = Minor;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventName, this);
		} // if
	} // if

} // Joe::SetCanonType

/*===========================================================================*/

void Joe::SetSecondaryType(unsigned short Major, unsigned short Minor)
{
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBPreChangeEventName, this);
		} // if
	} // if
if (TestFlags(WCS_JOEFLAG_HASKIDS) == NULL)
	{
	AttribInfo.SecondTypeMajor = Major;
	AttribInfo.SecondTypeMinor = Minor;
	} // if
if (GlobalApp->AppDB)
	{
	if (!GlobalApp->AppDB->SuppressNotifiesDuringLoad)
		{
		GlobalApp->AppDB->GenerateNotify(DBChangeEventName, this);
		} // if
	} // if

} // Joe::SetSecondaryType

/*===========================================================================*/

short Joe::AddEffect(GeneralEffect *Effect, int ApplyToAll)
{
JoeAttribute *MyAttr;
GeneralEffect *MyEffect;
NotifyTag Changes[2];
short Success = 0, NewApplyToAll = 0, RemoveIt;

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return (0); // Attributes are void if kids present
	} // if

if (Effect)
	{
	// thematic maps can be duplicated on a Joe, other types of effects can not
	if (MyAttr = MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)Effect->EffectType))
		{
		if ((MyEffect = GetAttributeEffect(MyAttr)) != Effect && MyEffect)
			{
			if (Effect->EffectType != WCS_EFFECTSSUBCLASS_THEMATICMAP)
				{
				if (! ApplyToAll)
					{
					if (TestFlags(WCS_JOEFLAG_ISDEM))
						RemoveIt = UserMessageCustom((char *)GetBestName(), "DEM already has a Component of\n the same type applied to it!\nDo you wish to replace the existing Component?", "Yes", "No", "Yes to All", 0);
					else
						RemoveIt = UserMessageCustom((char *)GetBestName(), "Vector already has a Component of\n the same type applied to it!\nDo you wish to replace the existing Component?", "Yes", "No", "Yes to All", 0);
					NewApplyToAll = RemoveIt == 2;
					} // if
				else if (ApplyToAll < 0)
					{
					if (TestFlags(WCS_JOEFLAG_ISDEM))
						RemoveIt = UserMessageYN((char *)GetBestName(), "DEM already has a Component of\n the same type applied to it!\nDo you wish to replace the existing Component?");
					else
						RemoveIt = UserMessageYN((char *)GetBestName(), "Vector already has a Component of\n the same type applied to it!\nDo you wish to replace the existing Component?");
					} // if
				else
					{
					RemoveIt = 1;
					} // else
				if (RemoveIt)
					{
					// this has been removed by GRH on 1/8/03 because it is dangerous to do for CoordSys attributes 
					// and it duplicates what is done in RemoveFromJoeList() below.
					//if (MyAttr = RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)MyEffect->EffectType, MyEffect))
					//	delete MyAttr;
					//Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					//Changes[1] = NULL;
					//GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					MyEffect->RemoveFromJoeList(this);
					} // if
				} // if
			} // if
		else
			{
			return (1);	// same effect already applied to this Joe
			} // else
		} // if
	if (! MatchAttribute (WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM) && (! MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)Effect->EffectType) || Effect->EffectType == WCS_EFFECTSSUBCLASS_THEMATICMAP))
		{
		switch (Effect->EffectType)
			{
			case WCS_JOE_ATTRIB_INTERNAL_LAKE:
				{
				JoeLake *Me = new JoeLake();

				if (Me)
					{
					Me->Lake = (LakeEffect *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_LAKE
			case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
				{
				JoeEcosystem *Me = new JoeEcosystem();

				if (Me)
					{
					Me->Eco = (EcosystemEffect *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
			case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
				{
				JoeRasterTA *Me = new JoeRasterTA();

				if (Me)
					{
					Me->Terra = (RasterTerraffectorEffect *)Effect;
					AddAttribute((JoeRasterTA *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
			case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
				{
				JoeTerraffector *Me = new JoeTerraffector();

				if (Me)
					{
					Me->Terra = (TerraffectorEffect *)Effect;
					AddAttribute((JoeTerraffector *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
			case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
				{
				JoeShadow *Me = new JoeShadow();

				if (Me)
					{
					Me->Shadow = (ShadowEffect *)Effect;
					AddAttribute((JoeShadow *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
			case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
				{
				JoeFoliage *Me = new JoeFoliage();

				if (Me)
					{
					Me->Foliage = (FoliageEffect *)Effect;
					AddAttribute((JoeFoliage *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
			case WCS_JOE_ATTRIB_INTERNAL_STREAM:
				{
				JoeStream *Me = new JoeStream();

				if (Me)
					{
					Me->Stream = (StreamEffect *)Effect;
					AddAttribute((JoeStream *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_STREAM
			case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
				{
				JoeObject3D *Me = new JoeObject3D();

				if (Me)
					{
					Me->Object = (Object3DEffect *)Effect;
					AddAttribute((JoeObject3D *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
			/*
			case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
				{
				JoeTerrainParam *Me = new JoeTerrainParam();

				if (Me)
					{
					Me->TerrainPar = (TerrainParamEffect *)Effect;
					AddAttribute((JoeTerrainParam *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
			*/
			case WCS_JOE_ATTRIB_INTERNAL_GROUND:
				{
				JoeGround *Me = new JoeGround();

				if (Me)
					{
					Me->Ground = (GroundEffect *)Effect;
					AddAttribute((JoeGround *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_GROUND
			case WCS_JOE_ATTRIB_INTERNAL_SNOW:
				{
				JoeSnow *Me = new JoeSnow();

				if (Me)
					{
					Me->SnowBusinessLikeShowBusiness = (SnowEffect *)Effect;
					AddAttribute((JoeSnow *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_SNOW
			case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
				{
				JoeEnvironment *Me = new JoeEnvironment();

				if (Me)
					{
					Me->Env = (EnvironmentEffect *)Effect;
					AddAttribute((JoeEnvironment *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
			case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
				{
				JoeCloud *Me = new JoeCloud();

				if (Me)
					{
					Me->Cloud = (CloudEffect *)Effect;
					AddAttribute((JoeCloud *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
			case WCS_JOE_ATTRIB_INTERNAL_WAVE:
				{
				JoeWave *Me = new JoeWave();

				if (Me)
					{
					Me->Wave = (WaveEffect *)Effect;
					AddAttribute((JoeWave *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_WAVE
			/* linkage to vectors from Thematic Maps obsolete in VNS 3
			case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
				{
				JoeThematicMap *Me = new JoeThematicMap();

				if (Me)
					{
					Me->Theme = (ThematicMap *)Effect;
					AddAttribute((JoeThematicMap *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
			*/
			case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
				{
				JoeCoordSys *Me = new JoeCoordSys();

				if (Me)
					{
					Me->Coord = (CoordSys *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Me->CoordsJoeList = Effect->AddToJoeList(this);
					ZeroUpTree();
					RecheckBounds();
					GlobalApp->AppDB->BoundUpTree(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
			case WCS_JOE_ATTRIB_INTERNAL_FENCE:
				{
				JoeFence *Me = new JoeFence();

				if (Me)
					{
					Me->Fnce = (Fence *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_FENCE
			case WCS_JOE_ATTRIB_INTERNAL_LABEL:
				{
				JoeLabel *Me = new JoeLabel();

				if (Me)
					{
					Me->Labl = (Label *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_LABEL
			/*
			case WCS_JOE_ATTRIB_INTERNAL_CMAP:
				{
				JoeCmap *Me = new JoeCmap();

				if (Me)
					{
					Me->Cmap = (CmapEffect *)Effect;
					AddAttribute((JoeCmap *)Me);
					Effect->AddToJoeList(this);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_CMAP
			*/
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector
			} // switch
		} // if not already an attribute of this class
	else if (MatchAttribute (WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM) && ! MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)Effect->EffectType))
		{
		switch (Effect->EffectType)
			{
			case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
				{
				JoeCoordSys *Me = new JoeCoordSys();

				if (Me)
					{
					Me->Coord = (CoordSys *)Effect;
					AddAttribute((JoeAttribute *)Me);
					Me->CoordsJoeList = Effect->AddToJoeList(this);
					ZeroUpTree();
					RecheckBounds();
					GlobalApp->AppDB->ReBoundTree(WCS_DATABASE_STATIC);
					Success = 1;
					} // if
				break;
				} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a DEM
			} // switch
		} // if not already an attribute of this class
	} // if

if (Success)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Success && NewApplyToAll ? 2: Success);

} // Joe::AddEffect

/*===========================================================================*/


short Joe::AddCoordSysFastDangerous(CoordSys *Effect, int ApplyToAll)
{
short Success = 0, NewApplyToAll = 0;
//NotifyTag Changes[2];

if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return (0); // Attributes are void if kids present
	} // if

if (Effect)
	{
	JoeCoordSys *Me = new JoeCoordSys();

	if (Me)
		{
		Me->Coord = Effect;
		AddAttribute((JoeAttribute *)Me);
		Me->CoordsJoeList = Effect->FastDangerousAddToJoeList(this);
		ZeroUpTree();
		RecheckBounds();
		GlobalApp->AppDB->BoundUpTree(this);
		Success = 1;
		} // if
	} // if

//if (Success)
//	{
//	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
//	Changes[1] = NULL;
//	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
//	} // if

return (Success && NewApplyToAll ? 2: Success);

} // Joe::AddCoordSysFastDangerous

/*===========================================================================*/

void JoeDEM::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_DEM;
AttribClassSize = sizeof(class JoeDEM);
FileSize = 90;

Pristine = ViewPref = NULL;

MaxFract = 0;
MaxEl = MinEl = 0;
NWAlt = NEAlt = SEAlt = SWAlt = 0;
SumElDif = SumElDifSq = 0.0f;
ElScale = 0.0;
ElDatum = 0.0;

// these are cached copies from the DEM object, for faster access without loading the .ELEV
pLonEntries = pLatEntries = 0;
pElMaxEl = pElMinEl = pNullValue = 0;
pLatStep = pLonStep = 0;
DEMGridNS = DEMGridWE = 0;

} // JoeDEM::InitClear

/*===========================================================================*/

JoeDEM::JoeDEM()
{

InitClear();

} // JoeDEM::JoeDEM

/*===========================================================================*/

unsigned long JoeDEM::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0;

if (Out)
	{
	if (fputc(WCS_JOEFILE_DEMINFOEXTENDV3, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(MaxFract, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&MaxEl, 1, 2, Out);
	ByteWrite += (unsigned long)fwrite(&MinEl, 1, 2, Out);
	ByteWrite += (unsigned long)fwrite(&NWAlt, 1, 2, Out);
	ByteWrite += (unsigned long)fwrite(&NEAlt, 1, 2, Out);
	ByteWrite += (unsigned long)fwrite(&SEAlt, 1, 2, Out);
	ByteWrite += (unsigned long)fwrite(&SWAlt, 1, 2, Out);

	ByteWrite += (unsigned long)fwrite(&SumElDif, 1, 4, Out);
	ByteWrite += (unsigned long)fwrite(&SumElDifSq, 1, 4, Out);

	ByteWrite += (unsigned long)fwrite(&ElScale, 1, 8, Out);
	ByteWrite += (unsigned long)fwrite(&ElDatum, 1, 8, Out); // 38 to here, which is WCS_JOEFILE_DEMINFOEXTEND
	
	// ulongs
	ByteWrite += (unsigned long)fwrite(&pLonEntries, 1, 4, Out);
	ByteWrite += (unsigned long)fwrite(&pLatEntries, 1, 4, Out);
	// floats
	ByteWrite += (unsigned long)fwrite(&pElMaxEl, 1, 4, Out);
	ByteWrite += (unsigned long)fwrite(&pElMinEl, 1, 4, Out);
	ByteWrite += (unsigned long)fwrite(&pNullValue, 1, 4, Out);
	// doubles
	ByteWrite += (unsigned long)fwrite(&pLatStep, 1, 8, Out);
	ByteWrite += (unsigned long)fwrite(&pLonStep, 1, 8, Out);
	ByteWrite += (unsigned long)fwrite(&DEMGridNS, 1, 8, Out);
	ByteWrite += (unsigned long)fwrite(&DEMGridWE, 1, 8, Out); // 90 to here
	} // if

return(ByteWrite);

} // JoeDEM::WriteToFile

/*===========================================================================*/

double JoeDEM::FetchBestElScale(void)
{

if (Pristine)
	return(Pristine->ElScale());
if (ViewPref)
	return(ViewPref->ElScale());
return(1.0);

} // JoeDEM::FetchBestElScale

/*===========================================================================*/


bool JoeDEM::UpdateCachedELEVData(Joe *MyJoe, Project *OpenFrom)
{
if (MyJoe && OpenFrom)
	{
	bool PristineLoaded = false, DoUnload = false;
	if (Pristine)
		{
		PristineLoaded = true;
		DoUnload = false;
		} // if
	else if (MyJoe->AttemptLoadDEM(0, OpenFrom) && Pristine)
		{
		PristineLoaded = true;
		DoUnload = true;
		} // if
	if (PristineLoaded)
		{
		// copy over cached values
		pLonEntries = Pristine->LonEntries();
		pLatEntries = Pristine->LatEntries();
		pElMaxEl = Pristine->MaxEl();
		pElMinEl = Pristine->MinEl();
		pNullValue = Pristine->NullValue();
		pLatStep = Pristine->LatStep();
		pLonStep = Pristine->LonStep();
		Pristine->GetDEMCellSizeMeters(DEMGridNS, DEMGridWE);
		if (DoUnload)
			{
			delete Pristine;
			Pristine = NULL;
			} // if
		return(true);
		} // if
	} // if

return(false);
} // JoeDEM::UpdateCachedELEVData

/*===========================================================================*/

JoeLake::JoeLake()
{

InitClear();

} // JoeLake::JoeLake

/*===========================================================================*/

void JoeLake::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_LAKE;
AttribClassSize = sizeof(class JoeLake);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Lake = NULL;

} // JoeLake::InitClear

/*===========================================================================*/

unsigned long JoeLake::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_LAKE, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Lake->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeLake::WriteToFile

/*===========================================================================*/

JoeEcosystem::JoeEcosystem()
{

InitClear();

} // JoeEcosystem::JoeEcosystem

/*===========================================================================*/

void JoeEcosystem::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM;
AttribClassSize = sizeof(class JoeEcosystem);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Eco = NULL;

} // JoeEcosystem::InitClear

/*===========================================================================*/

unsigned long JoeEcosystem::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Eco->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeEcosystem::WriteToFile

/*===========================================================================*/

JoeRasterTA::JoeRasterTA()
{

InitClear();

} // JoeRasterTA::JoeRasterTA

/*===========================================================================*/

void JoeRasterTA::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR;
AttribClassSize = sizeof(class JoeRasterTA);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Terra = NULL;

} // JoeRasterTA::InitClear

/*===========================================================================*/

unsigned long JoeRasterTA::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Terra->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeRasterTA::WriteToFile

/*===========================================================================*/

JoeTerraffector::JoeTerraffector()
{

InitClear();

} // JoeTerraffector::JoeTerraffector

/*===========================================================================*/

void JoeTerraffector::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR;
AttribClassSize = sizeof(class JoeTerraffector);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Terra = NULL;

} // JoeTerraffector::InitClear

/*===========================================================================*/

unsigned long JoeTerraffector::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Terra->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeTerraffector::WriteToFile

/*===========================================================================*/

JoeShadow::JoeShadow()
{

InitClear();

} // JoeShadow::JoeShadow

/*===========================================================================*/

void JoeShadow::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_SHADOW;
AttribClassSize = sizeof(class JoeShadow);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Shadow = NULL;

} // JoeShadow::InitClear

/*===========================================================================*/

unsigned long JoeShadow::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_SHADOW, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Shadow->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeShadow::WriteToFile

/*===========================================================================*/

JoeFoliage::JoeFoliage()
{

InitClear();

} // JoeFoliage::JoeFoliage

/*===========================================================================*/

void JoeFoliage::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_FOLIAGE;
AttribClassSize = sizeof(class JoeFoliage);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Foliage = NULL;

} // JoeFoliage::InitClear

/*===========================================================================*/

unsigned long JoeFoliage::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_FOLIAGE, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Foliage->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeFoliage::WriteToFile

/*===========================================================================*/

JoeStream::JoeStream()
{

InitClear();

} // JoeStream::JoeStream

/*===========================================================================*/

void JoeStream::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_STREAM;
AttribClassSize = sizeof(class JoeStream);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Stream = NULL;

} // JoeStream::InitClear()

/*===========================================================================*/

unsigned long JoeStream::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_STREAM, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Stream->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeStream::WriteToFile

/*===========================================================================*/

JoeObject3D::JoeObject3D()
{

InitClear();

} // JoeObject3D::JoeObject3D

/*===========================================================================*/

void JoeObject3D::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_OBJECT3D;
AttribClassSize = sizeof(class JoeObject3D);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Object = NULL;

} // JoeObject3D::InitClear()

/*===========================================================================*/

unsigned long JoeObject3D::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_OBJECT3D, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Object->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeObject3D::WriteToFile

/*===========================================================================*/

JoeTerrainParam::JoeTerrainParam()
{

InitClear();

} // JoeTerrainParam::JoeTerrainParam

/*===========================================================================*/

void JoeTerrainParam::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM;
AttribClassSize = sizeof(class JoeTerrainParam);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
TerrainPar = NULL;

} // JoeTerrainParam::InitClear()

/*===========================================================================*/

unsigned long JoeTerrainParam::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(TerrainPar->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeTerrainParam::WriteToFile

/*===========================================================================*/

JoeGround::JoeGround()
{

InitClear();

} // JoeGround::JoeGround

/*===========================================================================*/

void JoeGround::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_GROUND;
AttribClassSize = sizeof(class JoeGround);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Ground = NULL;

} // JoeGround::InitClear()

/*===========================================================================*/

unsigned long JoeGround::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_GROUND, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Ground->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeGround::WriteToFile

/*===========================================================================*/

JoeSnow::JoeSnow()
{

InitClear();

} // JoeSnow::JoeSnow

/*===========================================================================*/

void JoeSnow::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_SNOW;
AttribClassSize = sizeof(class JoeObject3D);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
SnowBusinessLikeShowBusiness = NULL;

} // JoeSnow::InitClear()

/*===========================================================================*/

unsigned long JoeSnow::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_SNOW, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(SnowBusinessLikeShowBusiness->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeSnow::WriteToFile

/*===========================================================================*/

JoeEnvironment::JoeEnvironment()
{

InitClear();

} // JoeEnvironment::JoeEnvironment

/*===========================================================================*/

void JoeEnvironment::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT;
AttribClassSize = sizeof(class JoeEnvironment);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Env = NULL;

} // JoeEnvironment::InitClear()

/*===========================================================================*/

unsigned long JoeEnvironment::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Env->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeEnvironment::WriteToFile

/*===========================================================================*/

JoeCloud::JoeCloud()
{

InitClear();

} // JoeCloud::JoeCloud

/*===========================================================================*/

void JoeCloud::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_CLOUD;
AttribClassSize = sizeof(class JoeCloud);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Cloud = NULL;

} // JoeCloud::InitClear()

/*===========================================================================*/

unsigned long JoeCloud::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_CLOUD, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Cloud->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeCloud::WriteToFile

/*===========================================================================*/

JoeWave::JoeWave()
{

InitClear();

} // JoeWave::JoeWave

/*===========================================================================*/

void JoeWave::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_WAVE;
AttribClassSize = sizeof(class JoeWave);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Wave = NULL;

} // JoeWave::InitClear()

/*===========================================================================*/

unsigned long JoeWave::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_WAVE, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Wave->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeWave::WriteToFile

/*===========================================================================*/

JoeCmap::JoeCmap()
{

InitClear();

} // JoeCmap::JoeCmap

/*===========================================================================*/

void JoeCmap::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_CMAP;
AttribClassSize = sizeof(class JoeCmap);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Cmap = NULL;

} // JoeCmap::InitClear()

/*===========================================================================*/

unsigned long JoeCmap::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_CMAP, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Cmap->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeCmap::WriteToFile

/*===========================================================================*/

JoeThematicMap::JoeThematicMap()
{

InitClear();

} // JoeThematicMap::JoeThematicMap

/*===========================================================================*/

void JoeThematicMap::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP;
AttribClassSize = sizeof(class JoeThematicMap);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Theme = NULL;

} // JoeThematicMap::InitClear

/*===========================================================================*/

unsigned long JoeThematicMap::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Theme->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeThematicMap::WriteToFile

/*===========================================================================*/

JoeCoordSys::JoeCoordSys()
{

InitClear();

} // JoeCoordSys::JoeCoordSys

/*===========================================================================*/

void JoeCoordSys::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_COORDSYS;
AttribClassSize = sizeof(class JoeCoordSys);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Coord = NULL;
CoordsJoeList = NULL;

} // JoeCoordSys::InitClear

/*===========================================================================*/

unsigned long JoeCoordSys::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field;

if (Out)
	{
#ifdef DATABASE_CHUNK_DEBUG
	sprintf(debugStr, "JoeCoordSys::WriteToFile\n");
	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(WCS_JOEFILE_EFFECTINFO (176))\n");
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(FileSizeLong (%u))\n", FileSizeLong);
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(WCS_JOE_ATTRIB_INTERNAL_COORDSYS (45))\n");
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_COORDSYS, Out) != EOF)
		{
		ByteWrite++;
		} // if
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(WCS_JOEFILE_EFFECTINFO_NAME (1))\n");
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(%s, (len = %d))\n", Coord->Name, Size);
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	ByteWrite += (unsigned long)fwrite(Coord->Name, 1, Size, Out);
#ifdef DATABASE_CHUNK_DEBUG
//	sprintf(debugStr, "JoeCoordSys::WriteToFile(WCS_JOEFILE_EFFECTINFO_END (0))\n");
//	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeCoordSys::WriteToFile

/*===========================================================================*/

JoeFence::JoeFence()
{

InitClear();

} // JoeFence::JoeFence

/*===========================================================================*/

void JoeFence::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_FENCE;
AttribClassSize = sizeof(class JoeFence);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Fnce = NULL;

} // JoeFence::InitClear

/*===========================================================================*/

unsigned long JoeFence::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_FENCE, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Fnce->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeFence::WriteToFile

/*===========================================================================*/

JoeLabel::JoeLabel()
{

InitClear();

} // JoeLabel::JoeLabel

/*===========================================================================*/

void JoeLabel::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_LABEL;
AttribClassSize = sizeof(class JoeLabel);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Labl = NULL;

} // JoeLabel::InitClear

/*===========================================================================*/

unsigned long JoeLabel::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_LABEL, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Labl->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);
} // JoeLabel::WriteToFile

// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector - be sure to add constructor(), InitClear() 
// and WriteToFile()

/*===========================================================================*/

JoeAlign3DObj::JoeAlign3DObj()
{

InitClear();

} // JoeAlign3DObj::JoeAlign3DObj

/*===========================================================================*/

void JoeAlign3DObj::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ;
AttribClassSize = sizeof(class JoeAlign3DObj);
FileSize = WCS_EFFECT_MAXNAMELENGTH + 9;
Obj = NULL;

} // JoeAlign3DObj::InitClear

/*===========================================================================*/

unsigned long JoeAlign3DObj::WriteToFile(FILE *Out)
{
unsigned long ByteWrite = 0, FileSizeLong = (unsigned long)(FileSize - 5);
char Size;

if (Out)
	{
	if (fputc(WCS_JOEFILE_EFFECTINFO, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(&FileSizeLong, 1, 4, Out);
	if (fputc(WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ, Out) != EOF)
		{
		ByteWrite++;
		} // if
	if (fputc(WCS_JOEFILE_EFFECTINFO_NAME, Out) != EOF)
		{
		ByteWrite++;
		} // if
	Size = WCS_EFFECT_MAXNAMELENGTH;	// size of name field
	if (fputc(Size, Out) != EOF)
		{
		ByteWrite++;
		} // if
	ByteWrite += (unsigned long)fwrite(Obj->Name, 1, Size, Out);
	if (fputc(WCS_JOEFILE_EFFECTINFO_END, Out) != EOF)
		{
		ByteWrite++;
		} // if
	} // if

return(ByteWrite);

} // JoeAlign3DObj::WriteToFile

/*===========================================================================*/

JoeRAHost::JoeRAHost()
{

InitClear();

} // JoeRAHost::JoeRAHost

/*===========================================================================*/

void JoeRAHost::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_RAHOST;
AttribClassSize = sizeof(class JoeRAHost);
FileSize = 0;
RAHost = NULL;

} // JoeRAHost::InitClear

/*===========================================================================*/

unsigned long JoeRAHost::WriteToFile(FILE *Out)
{

return(0);

} // JoeRAHost::WriteToFile

/*===========================================================================*/

JoeSXQueryItem::JoeSXQueryItem()
{

InitClear();

} // JoeSXQueryItem::JoeSXQueryItem

/*===========================================================================*/

void JoeSXQueryItem::InitClear(void)
{

NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM;
AttribClassSize = sizeof(class JoeSXQueryItem);
FileSize = 0;

} // JoeSXQueryItem::InitClear

/*===========================================================================*/

unsigned long JoeSXQueryItem::WriteToFile(FILE *Out)
{

return(0);

} // JoeSXQueryItem::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

CoordSys *Joe::GetCoordSys(void)
{
JoeCoordSys *MyAttr;
CoordSys *MyCoords;

if (MyAttr = (JoeCoordSys *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

return (MyCoords);

} // Joe::GetCoordSys

/*===========================================================================*/

void Joe::HeadingAndPitch(double Lat, double Lon, double MetersPerDegLat, double &Heading, double &Pitch)
{
double MetersPerDegLon, Rise, Run, PrevHeading, NextHeading, PrevPitch, NextPitch, NearestDist, NewDist, P1Offset, SegLength;
Point2d ZeroPoint, StartSeg, EndSeg;
VertexDEM CurVert, NextVert, PrevVert;
VectorPoint *PrevPoint = NULL, *CurPoint, *NextPoint;
CoordSys *MyCoords;
char UsePoint[2];

if (! TestFlags(WCS_JOEFLAG_ISDEM))
	{
	Heading = Pitch = 0.0;
	if (CurPoint = GetFirstRealPoint())
		{
		MyCoords = GetCoordSys();
		// look for a direct match first
		//CurPoint = GetFirstRealPoint();		// replaced by above
		while (CurPoint)
			{
			NextPoint = CurPoint->Next;
			if (PrevPoint || NextPoint)
				{
				if (CurPoint->ProjToDefDeg(MyCoords, &CurVert))
					{
					MetersPerDegLon = MetersPerDegLat * cos(CurVert.Lat * PiOver180);
					if ((fabs(CurVert.Lat - Lat) * MetersPerDegLat < .01) && (fabs(CurVert.Lon - Lon) * MetersPerDegLon < .01))
						{
						if (PrevPoint)
							{
							Rise = MetersPerDegLat * (CurVert.Lat - PrevVert.Lat);
							Run = -MetersPerDegLon * (CurVert.Lon - PrevVert.Lon);
							PrevHeading = findangle3(Rise, Run) * PiUnder180;
							Run = sqrt(Rise * Rise + Run * Run);
							Rise = -(CurVert.Elev - PrevVert.Elev);
							PrevPitch = findangle3(Run, Rise) * PiUnder180;
							} // if
						if (NextPoint)
							{
							if (NextPoint->ProjToDefDeg(MyCoords, &NextVert))
								{
								Rise = MetersPerDegLat * (NextVert.Lat - CurVert.Lat);
								Run = -MetersPerDegLon * (NextVert.Lon - CurVert.Lon);
								NextHeading = findangle3(Rise, Run) * PiUnder180;
								Run = sqrt(Rise * Rise + Run * Run);
								Rise = -(NextVert.Elev - CurVert.Elev);
								NextPitch = findangle3(Run, Rise) * PiUnder180;
								} // if
							else
								return;	// projection error, might as well quit now
							} // if
						if (PrevPoint && NextPoint)
							{
							if (PrevHeading - NextHeading > 180.0)
								NextHeading += 360.0;
							else if (NextHeading - PrevHeading > 180.0)
								PrevHeading += 180.0;
							Heading = (PrevHeading + NextHeading) * .5;
							Pitch = (PrevPitch + NextPitch) * .5;
							} // if
						else if (PrevPoint)
							{
							Heading = PrevHeading;
							Pitch = PrevPitch;
							} // else if
						else // (NextPoint)
							{
							Heading = NextHeading;
							Pitch = NextPitch;
							} // else
						return;	// we're all done, can't get any better than a direct point match
						} // if
					} // if
				else
					return;	// projection error, might as well quit now
				} // if
			else
				{
				return;
				} // else only one point
			PrevVert.CopyLatLon(&CurVert);
			PrevPoint = CurPoint;
			CurPoint = CurPoint->Next;
			} // while

		// didn't find a direct match so look for nearest segment
		PrevPoint = NULL;
		CurPoint = GetFirstRealPoint();		// skip the label point
		NextPoint = CurPoint->Next;
		NearestDist = FLT_MAX;
		ZeroPoint[0] = 0.0;
		ZeroPoint[1] = 0.0;
		if (CurPoint->ProjToDefDeg(MyCoords, &CurVert))
			{
			StartSeg[0] = MetersPerDegLat * (CurVert.Lat - Lat);
			StartSeg[1] = MetersPerDegLon * (CurVert.Lon - Lon);
			while (CurPoint && NextPoint)
				{
				if (NextPoint->ProjToDefDeg(MyCoords, &NextVert))
					{
					MetersPerDegLon = MetersPerDegLat * cos(CurVert.Lat * PiOver180);
					EndSeg[0] = MetersPerDegLat * (NextVert.Lat - Lat);
					EndSeg[1] = MetersPerDegLon * (NextVert.Lon - Lon);
					if ((NewDist = DistPoint2LineContained(ZeroPoint, StartSeg, EndSeg, UsePoint, P1Offset, SegLength)) < NearestDist)
						{
						NearestDist = NewDist;
						if (UsePoint[1] || (UsePoint[0] && UsePoint[1]) || (UsePoint[0] && ! PrevPoint))
							{
							Rise = MetersPerDegLat * (NextVert.Lat - CurVert.Lat);
							Run = -MetersPerDegLon * (NextVert.Lon - CurVert.Lon);
							Heading = findangle3(Rise, Run) * PiUnder180;
							Run = sqrt(Rise * Rise + Run * Run);
							Rise = -(NextVert.Elev - CurVert.Elev);
							Pitch = findangle3(Run, Rise) * PiUnder180;
							} // if
						else if (UsePoint[0])
							{
							Rise = MetersPerDegLat * (CurVert.Lat - PrevVert.Lat);
							Run = -MetersPerDegLon * (CurVert.Lon - PrevVert.Lon);
							PrevHeading = findangle3(Rise, Run) * PiUnder180;
							Run = sqrt(Rise * Rise + Run * Run);
							Rise = -(CurVert.Elev - PrevVert.Elev);
							PrevPitch = findangle3(Run, Rise) * PiUnder180;

							Rise = MetersPerDegLat * (NextVert.Lat - CurVert.Lat);
							Run = -MetersPerDegLon * (NextVert.Lon - CurVert.Lon);
							NextHeading = findangle3(Rise, Run) * PiUnder180;
							Run = sqrt(Rise * Rise + Run * Run);
							Rise = -(NextVert.Elev - CurVert.Elev);
							NextPitch = findangle3(Run, Rise) * PiUnder180;

							if (PrevHeading - NextHeading > 180.0)
								NextHeading += 360.0;
							else if (NextHeading - PrevHeading > 180.0)
								PrevHeading += 180.0;
							Heading = (PrevHeading + NextHeading) * .5;
							Pitch = (PrevPitch + NextPitch) * .5;
							} // else if
						} // if
					PrevVert.CopyLatLon(&CurVert);
					CurVert.CopyLatLon(&NextVert);
					PrevPoint = CurPoint;
					CurPoint = CurPoint->Next;
					NextPoint = CurPoint->Next;
					StartSeg[0] = EndSeg[0];
					StartSeg[1] = EndSeg[1];
					} // if
				else
					return;	// projection error, might as well quit now
				} // while
			} // if
		else
			return;	// projection error, might as well quit now
		} // if
	} // if

} // Joe::HeadingAndPitch

/*===========================================================================*/

int Joe::CalcBisectrices(double MetersPerDegLat, int ConnectEnds)
{
double CurMetersPerDegLon, CumulativeLength;
long NumSegs, Index, PrevIndex, NextIndex, NextNextIndex, RefIndex, CurSegDataCt;
VectorPoint *PLink, *PrevLink, *NextLink, *NextNextLink, *PLinkLast = NULL;
CoordSys *MyCoords;
struct GeneralLinearEquation BisectrixA, BisectrixB;
float LengthThisSegment;
JoeSegmentData **SegData, SegDataPrev, SegDataCur, SegDataNext, SegDataNextNext;
bool GotOne, GotTwo, Success = true;
VertexDEM Vert;

FreeSegmentData();

// determine how many viable segments there are. Any two identical points will not create a segment between them
for (NumSegs = 0, PLink = GetFirstRealPoint(); PLink; PLink = PLink->Next)
	{
	NextLink = PLink->Next ? PLink->Next: GetFirstRealPoint();
	if (! PLink->SamePointLatLon(NextLink))
		{
		NumSegs ++;
		} // if
	PLinkLast = PLink;
	} // for

// segment data not needed in these cases
if (GetLineStyle() < 4 || ! GetSecondRealPoint() || NumSegs < 2)
	return (AddPointSegData(MetersPerDegLat));

// even linear vectors can loop back to their beginning creating a loop that needs to have its
// end points continuous.
if (PLinkLast && PLinkLast->SamePointLatLon(GetFirstRealPoint()))
	ConnectEnds = 1;
	
MyCoords = GetCoordSys();

// allocate segment data
if (NumSegs)
	{
	if ((SegData = AddSegmentData(NumSegs)) && SegData[0])
		{
		JRendData->ConnectEnds = ConnectEnds;
		// first instance will have the data computed using the lon conversion factor for the current node
		// second instance will have the data computed using the lon conversion factor for the previous node
		NextLink = NULL;
		// loop through segments and set up first set of point data
		for (Index = 0, PLink = GetFirstRealPoint(); PLink && Index < NumSegs; PLink = PLink->Next)
			{
			NextLink = PLink->Next ? PLink->Next: ConnectEnds ? GetFirstRealPoint(): NULL;
			NextIndex = Index < NumSegs - 1 ? Index + 1: 0;
			if (! NextLink || ! PLink->SamePointLatLon(NextLink))
				{
				if (PLink->ProjToDefDeg(MyCoords, &Vert))
					{
					if (Index == 0)
						JRendData->OrigLon = Vert.Lon;
					SegData[0][Index].MyPoint = SegData[1][Index].MyPoint = PLink;
					SegData[0][Index].PtLon = SegData[1][Index].PtLon = Vert.Lon;
					CurMetersPerDegLon = MetersPerDegLat * cos(Vert.Lat * PiOver180);
					SegData[0][Index].MetersPerDegLon = CurMetersPerDegLon;
					SegData[0][Index].PtX = Vert.Lon * CurMetersPerDegLon;
					SegData[0][Index].PtY = Vert.Lat * MetersPerDegLat;
					} // if
				else
					return (0);
				++Index;
				} // if
			} // for
		// loop through segments and set up second set of point data
		for (Index = 0; Index < NumSegs && Success; ++Index)
			{
			NextIndex = Index < NumSegs - 1 ? Index + 1: 0;
			CurMetersPerDegLon = SegData[0][NextIndex].MetersPerDegLon;
			SegData[1][Index].MetersPerDegLon = CurMetersPerDegLon;
			// [1] PtX is calculated relative to the next node
			SegData[1][Index].PtX = (SegData[0][Index].PtLon - SegData[0][NextIndex].PtLon) * CurMetersPerDegLon + SegData[0][NextIndex].PtX;
			SegData[1][Index].PtY = SegData[0][Index].PtY;
			} // for
			
		CumulativeLength = 0.0;
		// loop through segments and set up each point
		for (Index = 0; Index < NumSegs && Success; ++Index)
			{
			PrevIndex = Index - 1;
			if (Index < 0 && ConnectEnds)
				Index = NumSegs - 1;
			PrevLink = PrevIndex >= 0 ? SegData[0][PrevIndex].MyPoint: NULL;
			NextIndex = Index + 1;
			if (NextIndex >= NumSegs)
				{
				if (ConnectEnds)
					NextIndex = 0;
				else
					break;	// last point needs no calculation since it isn't an actual segment
				} // if
			NextLink = NextIndex < NumSegs ? SegData[0][NextIndex].MyPoint: NULL;
			NextNextIndex = NextIndex + 1;
			if (NextNextIndex >= NumSegs && ConnectEnds)
				NextNextIndex = 0;
			NextNextLink = NextNextIndex < NumSegs ? SegData[0][NextNextIndex].MyPoint: NULL;
			
			// calc all the data for current segment which includes the bisectrices for both ends of the segment
			// in both lon factors.
			// Store the point location and bisectrix data for the current segment using the trailing 
			// node's factor in SegData[0].
			// Store the point location and bisectrix data for the current segment using the leading 
			// node's factor in SegData[1].
					
			for (CurSegDataCt = 0; CurSegDataCt < 2; ++CurSegDataCt)
				{
				RefIndex = CurSegDataCt ? NextIndex: Index;
				GotOne = GotTwo = false;
				
				if (PrevLink)
					{
					SegDataPrev.PtX = (SegData[0][PrevIndex].PtLon - SegData[0][RefIndex].PtLon) * SegData[0][RefIndex].MetersPerDegLon + SegData[0][RefIndex].PtX;
					SegDataPrev.PtY = SegData[0][PrevIndex].PtY;
					} // if
				SegDataCur.PtX = (SegData[0][Index].PtLon - SegData[0][RefIndex].PtLon) * SegData[0][RefIndex].MetersPerDegLon + SegData[0][RefIndex].PtX;
				SegDataCur.PtY = SegData[0][Index].PtY;
				SegDataNext.PtX = (SegData[0][NextIndex].PtLon - SegData[0][RefIndex].PtLon) * SegData[0][RefIndex].MetersPerDegLon + SegData[0][RefIndex].PtX;
				SegDataNext.PtY = SegData[0][NextIndex].PtY ;
				if (NextNextLink)
					{
					SegDataNextNext.PtX = (SegData[0][NextNextIndex].PtLon - SegData[0][RefIndex].PtLon) * SegData[0][RefIndex].MetersPerDegLon + SegData[0][RefIndex].PtX;
					SegDataNextNext.PtY = SegData[0][NextNextIndex].PtY;
					} // if
				// trailing bisectrix
					// previous node, current node, leading node
				LengthThisSegment = 0.0f;
				if (PrevLink)
					{
					// calc bisectrix at current node
					if (CalcBisectrix(&SegDataPrev, &SegDataCur, &SegDataNext, &BisectrixA, LengthThisSegment))
						{
						GotOne = true;
						SegData[CurSegDataCt][Index].CumulativeLength = LengthThisSegment;
						} // if
					} // if
				else
					{
					// calc orthogonal at current node
					if (CalcOrthogonal(&SegDataCur, &SegDataNext, 0, &BisectrixA, LengthThisSegment))
						{
						GotOne = true;
						SegData[CurSegDataCt][Index].CumulativeLength = LengthThisSegment;
						} // if
					} // else
					
				// leading bisectrix
					// current node, leading node, nextnext node
				if (NextNextLink)
					{
					// calc bisectrix at next node
					if (CalcBisectrix(&SegDataCur, &SegDataNext, &SegDataNextNext, &BisectrixB, LengthThisSegment))
						GotTwo = true;
					} // if
				else
					{
					// calc orthogonal at next node
					if (CalcOrthogonal(&SegDataCur, &SegDataNext, 1, &BisectrixB, LengthThisSegment))
						GotTwo = true;
					} // else
					
				// calculate intersection
				if (GotOne && GotTwo)
					{
					if (CalcLineIntersection(&BisectrixA, &BisectrixB, SegData[CurSegDataCt][Index].BisectrixData))
						SegData[CurSegDataCt][Index].BisectrixOK = 1;
					else
						{
						SegData[CurSegDataCt][Index].BisectrixData[0] = BisectrixB.CoefA;
						SegData[CurSegDataCt][Index].BisectrixData[1] = BisectrixB.CoefB;
						SegData[CurSegDataCt][Index].BisectrixData[2] = BisectrixB.CoefC;
						} // else
					} // if
				else
					{
					Success = false;
					break;
					} // else
				} // for CurSegDataCt
			CumulativeLength += (SegData[0][Index].CumulativeLength + SegData[1][Index].CumulativeLength) * .5;
			SegData[0][Index].CumulativeLength = SegData[1][Index].CumulativeLength = (float)CumulativeLength;
			} // for Index
			
		#ifdef WCS_OUTPUT_BISECTRIXDETAIL
		char DebugStr[256];
		for (Index = 0; Index < NumSegs; ++Index)
			{
			for (CurSegDataCt = 0; CurSegDataCt < 2; ++CurSegDataCt)
				{ 
				sprintf(DebugStr, "I=%d/%d A=%f B=%f C=%f L=%f X=%f Y=%f Lon=%f MPDGL=%f OK=%d Pt=%p\n", Index, CurSegDataCt, 
					SegData[CurSegDataCt][Index].BisectrixData[0], SegData[CurSegDataCt][Index].BisectrixData[1], SegData[CurSegDataCt][Index].BisectrixData[2],
					SegData[CurSegDataCt][Index].CumulativeLength, SegData[CurSegDataCt][Index].PtX, SegData[CurSegDataCt][Index].PtY, SegData[CurSegDataCt][Index].PtLon, 
					SegData[CurSegDataCt][Index].MetersPerDegLon, SegData[CurSegDataCt][Index].BisectrixOK, SegData[CurSegDataCt][Index].MyPoint);
				OutputDebugStr(DebugStr);
				} // for
			} // for
		#endif // WCS_OUTPUT_BISECTRIXDETAIL
		} // if
	else
		Success = false;
	} // if

return (Success ? 1: 0);

} // Joe::CalcBisectrices

/*===========================================================================*/
/* Replaced by above 7/10/08 GRH
int Joe::CalcBisectrices(double MetersPerDegLat, int ConnectEnds)
{
double CurMetersPerDegLon, OrigLon, BisectrixStash[3];
long NumSegs, Index, NextIndex, PrevIndex, CurSegDataCt;
VectorPoint *PLink, *NextLink, *PLinkLast = NULL;
CoordSys *MyCoords;
struct GeneralLinearEquation BisectrixA, BisectrixB;
float CumulativeLength = 0.0f, LengthThisSegment;
JoeSegmentData **SegData, SegDataPrev, SegDataCur, SegDataNext;
VertexDEM Vert0, Vert1, Vert2;

FreeSegmentData();

// segment data not needed in these cases
if (GetLineStyle() < 4 || ! GetSecondRealPoint())
	return (AddPointSegData());

MyCoords = GetCoordSys();

// determine how many viable segments there are. Any two identical points will not create a segment between them
for (NumSegs = 0, PLink = GetFirstRealPoint(); PLink; PLink = PLink->Next)
	{
	NextLink = PLink->Next ? PLink->Next: GetFirstRealPoint();
	if (! PLink->SamePointLatLon(NextLink))
		{
		NumSegs ++;
		} // if
	PLinkLast = PLink;
	} // for

// even linear vectors can loop back to their beginning creating a loop that needs to have its
// end points continuous.
if (PLinkLast && PLinkLast->SamePointLatLon(GetFirstRealPoint()))
	ConnectEnds = 1;
	
// allocate segment data
if (NumSegs)
	{
	if (SegData = AddSegmentData(NumSegs))
		{
		JRendData->ConnectEnds = ConnectEnds;
		// first instance will have the data computed using the lon conversion factor for the current node
		// second instance will have the data computed using the lon conversion factor for the previous node
		for (CurSegDataCt = 0; CurSegDataCt < 2; ++CurSegDataCt)
			{
			NextLink = NULL;
			
			// loop through segments and set up each point
			for (Index = 0, PLink = GetFirstRealPoint(); PLink && Index < NumSegs; PLink = PLink->Next)
				{
				NextLink = PLink->Next ? PLink->Next: ConnectEnds ? GetFirstRealPoint(): NULL;
				NextIndex = Index < NumSegs - 1 ? Index + 1: 0;
				if (! NextLink || ! PLink->SamePointLatLon(NextLink))
					{
					if (PLink->ProjToDefDeg(MyCoords, &Vert1))
						{
						if (Index == 0)
							JRendData->OrigLon = OrigLon = Vert1.Lon;
						if (CurSegDataCt && NextLink)
							{
							if (NextLink->ProjToDefDeg(MyCoords, &Vert2))
								{
								CurMetersPerDegLon = MetersPerDegLat * cos(Vert2.Lat * PiOver180);
								SegData[CurSegDataCt][NextIndex].PtLon = Vert2.Lon;
								SegData[CurSegDataCt][NextIndex].PtX = Vert2.Lon * CurMetersPerDegLon;
								SegData[CurSegDataCt][NextIndex].PtY = Vert2.Lat * MetersPerDegLat;
								SegData[CurSegDataCt][NextIndex].MyPoint = NextLink;
								SegData[CurSegDataCt][NextIndex].MetersPerDegLon = CurMetersPerDegLon;
								SegData[CurSegDataCt][Index].PtLon = Vert1.Lon;
								SegData[CurSegDataCt][Index].PtX = (Vert1.Lon - Vert2.Lon) * CurMetersPerDegLon + SegData[CurSegDataCt][NextIndex].PtX;
								SegData[CurSegDataCt][Index].PtY = Vert1.Lat * MetersPerDegLat;
								SegData[CurSegDataCt][Index].MyPoint = PLink;
								SegData[CurSegDataCt][Index].MetersPerDegLon = CurMetersPerDegLon;
								} // if
							else
								return (0);
							} // if
						else
							{
							CurMetersPerDegLon = MetersPerDegLat * cos(Vert1.Lat * PiOver180);
							SegData[CurSegDataCt][Index].PtLon = Vert1.Lon;
							SegData[CurSegDataCt][Index].PtX = Vert1.Lon * CurMetersPerDegLon;
							SegData[CurSegDataCt][Index].PtY = Vert1.Lat * MetersPerDegLat;
							SegData[CurSegDataCt][Index].MyPoint = PLink;
							SegData[CurSegDataCt][Index].MetersPerDegLon = CurMetersPerDegLon;
							} // else
						} // if
					else
						return (0);
					++Index;
					} // if
				} // for
		
			// loop through all segments and calculate a bisectrix or orthogonal (for end points)
			for (Index = 0; Index < NumSegs; ++Index)
				{
				NextIndex = Index + 1;
				PrevIndex = Index - 1;
				if (NextIndex >= NumSegs && ConnectEnds)
					NextIndex -= NumSegs;
				if (PrevIndex < 0 && ConnectEnds)
					PrevIndex += NumSegs;
				if (PrevIndex >= 0)
					{
					SegDataPrev.PtX = (SegData[CurSegDataCt][PrevIndex].PtLon - SegData[CurSegDataCt][Index].PtLon) * SegData[CurSegDataCt][Index].MetersPerDegLon + SegData[CurSegDataCt][Index].PtX;
					SegDataPrev.PtY = SegData[CurSegDataCt][PrevIndex].PtY;
					} // if
				if (NextIndex < NumSegs)
					{
					SegDataNext.PtX = (SegData[CurSegDataCt][NextIndex].PtLon - SegData[CurSegDataCt][Index].PtLon) * SegData[CurSegDataCt][Index].MetersPerDegLon + SegData[CurSegDataCt][Index].PtX;
					SegDataNext.PtY = SegData[CurSegDataCt][NextIndex].PtY;
					} // if
				if (PrevIndex >= 0 && NextIndex < NumSegs)
					{
					LengthThisSegment = 0.0f;
					if (CalcBisectrix(&SegDataPrev, &SegData[CurSegDataCt][Index], &SegDataNext, &BisectrixB, LengthThisSegment))
						{
						SegData[CurSegDataCt][Index].BisectrixData[0] = BisectrixB.CoefA;
						SegData[CurSegDataCt][Index].BisectrixData[1] = BisectrixB.CoefB;
						SegData[CurSegDataCt][Index].BisectrixData[2] = BisectrixB.CoefC;
						SegData[CurSegDataCt][Index].CumulativeLength = LengthThisSegment;
						} // if
					else
						return (0);
					} // if
				else if (PrevIndex < 0)
					{
					if (CalcOrthogonal(&SegData[CurSegDataCt][Index], &SegDataNext, 0, &BisectrixB, LengthThisSegment))
						{
						SegData[CurSegDataCt][Index].BisectrixData[0] = BisectrixB.CoefA;
						SegData[CurSegDataCt][Index].BisectrixData[1] = BisectrixB.CoefB;
						SegData[CurSegDataCt][Index].BisectrixData[2] = BisectrixB.CoefC;
						SegData[CurSegDataCt][Index].CumulativeLength = LengthThisSegment;
						} // if
					else
						return (0);
					} // else if first segment of unconnected vector
				else
					{
					if (CalcOrthogonal(&SegDataPrev, &SegData[CurSegDataCt][Index], 1, &BisectrixB, LengthThisSegment))
						{
						SegData[CurSegDataCt][Index].BisectrixData[0] = BisectrixB.CoefA;
						SegData[CurSegDataCt][Index].BisectrixData[1] = BisectrixB.CoefB;
						SegData[CurSegDataCt][Index].BisectrixData[2] = BisectrixB.CoefC;
						SegData[CurSegDataCt][Index].CumulativeLength = LengthThisSegment;
						} // if
					else
						return (0);
					} // else last segment
				} // for Index
				
			// now a loop to fill in the actual intersections
			if (ConnectEnds)
				{
				BisectrixStash[0] = SegData[CurSegDataCt][0].BisectrixData[0];
				BisectrixStash[1] = SegData[CurSegDataCt][0].BisectrixData[1];
				BisectrixStash[2] = SegData[CurSegDataCt][0].BisectrixData[2];
				} // if
			CumulativeLength = 0.0;
			for (Index = 0; Index < NumSegs; ++Index)
				{
				NextIndex = Index + 1;
				if (NextIndex >= NumSegs)
					{
					if (ConnectEnds)
						NextIndex -= NumSegs;
					else
						{
						// having access to the total vector length might come in handy
						// It will be in SegData[NumSegs - 1].CumulativeLength
						SegData[CurSegDataCt][Index].CumulativeLength = CumulativeLength;
						break;
						} // else last point
					} // if
				BisectrixA.CoefA = SegData[CurSegDataCt][Index].BisectrixData[0];
				BisectrixA.CoefB = SegData[CurSegDataCt][Index].BisectrixData[1];
				BisectrixA.CoefC = SegData[CurSegDataCt][Index].BisectrixData[2];
				if (NextIndex == 0)
					{
					BisectrixB.CoefA = BisectrixStash[0];
					BisectrixB.CoefB = BisectrixStash[1];
					BisectrixB.CoefC = BisectrixStash[2];
					} // if
				else
					{
					BisectrixB.CoefA = SegData[CurSegDataCt][NextIndex].BisectrixData[0];
					BisectrixB.CoefB = SegData[CurSegDataCt][NextIndex].BisectrixData[1];
					BisectrixB.CoefC = SegData[CurSegDataCt][NextIndex].BisectrixData[2];
					} // else
					
				if (CalcLineIntersection(&BisectrixA, &BisectrixB, SegData[CurSegDataCt][Index].BisectrixData))
					SegData[CurSegDataCt][Index].BisectrixOK = 1;
				else
					{
					SegData[CurSegDataCt][Index].BisectrixData[0] = BisectrixB.CoefA;
					SegData[CurSegDataCt][Index].BisectrixData[1] = BisectrixB.CoefB;
					SegData[CurSegDataCt][Index].BisectrixData[2] = BisectrixB.CoefC;
					} // else
				CumulativeLength += SegData[CurSegDataCt][Index].CumulativeLength;
				SegData[CurSegDataCt][Index].CumulativeLength = CumulativeLength;
				} // for Index
			} // for CurSegDataCt
		#ifdef WCS_OUTPUT_BISECTRIXDETAIL
		char DebugStr[256];
		for (Index = 0; Index < NumSegs; ++Index)
			{
			for (CurSegDataCt = 0; CurSegDataCt < 2; ++CurSegDataCt)
				{ 
				sprintf(DebugStr, "I=%d/%d A=%f B=%f C=%f L=%f X=%f Y=%f Lon=%f MPDGL=%f OK=%d Pt=%p\n", Index, CurSegDataCt, 
					SegData[CurSegDataCt][Index].BisectrixData[0], SegData[CurSegDataCt][Index].BisectrixData[1], SegData[CurSegDataCt][Index].BisectrixData[2],
					SegData[CurSegDataCt][Index].CumulativeLength, SegData[CurSegDataCt][Index].PtX, SegData[CurSegDataCt][Index].PtY, SegData[CurSegDataCt][Index].PtLon, 
					SegData[CurSegDataCt][Index].MetersPerDegLon, SegData[CurSegDataCt][Index].BisectrixOK, SegData[CurSegDataCt][Index].MyPoint);
				OutputDebugStr(DebugStr);
				} // for
			} // for
		#endif // WCS_OUTPUT_BISECTRIXDETAIL
		return (1);
		} // if
	return (0);
	} // if

return (1);

} // Joe::CalcBisectrices
*/
/*===========================================================================*/

int Joe::CalcBisectrix(JoeSegmentData *VecPt1, JoeSegmentData *VecPt2, JoeSegmentData *VecPt3, struct GeneralLinearEquation *Eq, float &LengthThisSegment)
{
double Len[3], Temp1, Temp2;
VertexDEM Vert;
Point2d Pt1, Pt2, Pt3, Pt4;

Pt1[0] = VecPt1->PtX;
Pt1[1] = VecPt1->PtY;
Pt2[0] = VecPt2->PtX;
Pt2[1] = VecPt2->PtY;
Pt3[0] = VecPt3->PtX;
Pt3[1] = VecPt3->PtY;

Temp1 = Pt1[0] - Pt2[0];
Temp2 = Pt1[1] - Pt2[1];
Len[0] = sqrt(Temp1 * Temp1 + Temp2 * Temp2);
Temp1 = Pt2[0] - Pt3[0];
Temp2 = Pt2[1] - Pt3[1];
Len[1] = sqrt(Temp1 * Temp1 + Temp2 * Temp2);
LengthThisSegment = (float)Len[1];

if (Len[0] > 0.0 && Len[1] > 0.0)
	{
	Len[0] /= (Len[0] + Len[1]);
	Pt4[0] = Len[0] * (Pt3[0] - Pt1[0]) + Pt1[0];
	Pt4[1] = Len[0] * (Pt3[1] - Pt1[1]) + Pt1[1];
	if (fabs(Pt4[0] - Pt2[0]) > .00001 || fabs(Pt4[1] - Pt2[1]) > .00001)
		return (CalcGeneralLinearEquation(Pt2, Pt4, Eq));
	else
		return (CalcLineOrthogonal(Pt1, Pt2, 1, Eq));
	} // if

// one of the segments is 0 length so two of the points must have been the same
return (0);

} // Joe::CalcBisectrix

/*===========================================================================*/

int Joe::CalcOrthogonal(JoeSegmentData *VecPt1, JoeSegmentData *VecPt2, 
	int LastPt, struct GeneralLinearEquation *Eq, float &LengthThisSegment)
{
double Temp1, Temp2;
Point2d Pt1, Pt2;
VertexDEM Vert;

Pt1[0] = VecPt1->PtX;
Pt1[1] = VecPt1->PtY;
Pt2[0] = VecPt2->PtX;
Pt2[1] = VecPt2->PtY;

Temp1 = Pt1[0] - Pt2[0];
Temp2 = Pt1[1] - Pt2[1];
LengthThisSegment = (float)sqrt(Temp1 * Temp1 + Temp2 * Temp2);

return (CalcLineOrthogonal(Pt1, Pt2, LastPt, Eq));

} // Joe::CalcOrthogonal

/*===========================================================================*/

double FindTanAngle(GeneralLinearEquation *Eq1, GeneralLinearEquation *Eq2)
{
double Denom;

Denom = Eq1->CoefA * Eq2->CoefA + Eq1->CoefB * Eq2->CoefB;
if (Denom != 0.0)
	return (Eq2->CoefA * Eq1->CoefB - Eq1->CoefA * Eq2->CoefB) / Denom;
return (0.0);

} // FindTanAngle

/*===========================================================================*/

int Joe::AddPointSegData(double MetersPerDegLat)
{
double CurMetersPerDegLon;
long NumSegs, Index;
VectorPoint *PLink, *NextLink;
CoordSys *MyCoords;
JoeSegmentData **SegData;
bool Success = true;
VertexDEM Vert;

NumSegs = 0;
MyCoords = GetCoordSys();

for (PLink = GetFirstRealPoint(); PLink; PLink = PLink->Next)
	{
	NextLink = PLink->Next;
	if (! NextLink || ! PLink->SamePointLatLon(NextLink))
		{
		++NumSegs;
		} // if
	} // for

if (SegData = AddSegmentData(NumSegs))
	{
	for (Index = 0, PLink = GetFirstRealPoint(); PLink && Index < NumSegs; PLink = PLink->Next)
		{
		NextLink = PLink->Next;
		if (! NextLink || ! PLink->SamePointLatLon(NextLink))
			{
			if (PLink->ProjToDefDeg(MyCoords, &Vert))
				{
				if (Index == 0)
					JRendData->OrigLon = Vert.Lon;
				SegData[0][Index].MyPoint = SegData[1][Index].MyPoint = PLink;
				SegData[0][Index].PtLon = Vert.Lon;
				CurMetersPerDegLon = MetersPerDegLat * cos(Vert.Lat * PiOver180);
				SegData[0][Index].MetersPerDegLon = CurMetersPerDegLon;
				SegData[0][Index].PtX = Vert.Lon * CurMetersPerDegLon;
				SegData[0][Index].PtY = Vert.Lat * MetersPerDegLat;
				} // if
			else
				return (0);
			++Index;
			} // if
		} // for
	} // if
else
	Success = 0;
	
return (Success ? 1: 0);

} // Joe::AddPointSegData

/*===========================================================================*/

// this function replaces that below used in WCS6/VNS 2
// It is used to evaluate terraffectors and streams and textures for any align-to vector textures
// when VectorPolygons are used

#ifdef WCS_COMBINATION_V2_V3_MINDIST

double Joe::MinDistToPoint(VectorNode *CurNode, TfxDetail *SegmentDetail, double MetersPerDegLat, double Exageration, double ElevDatum, 
	int SplineElev, int ConnectEnds, double &Elev, float *DistXYZ, double *Slope)
{
double MinDist, NewDist, MetersPerDegLon, SegLengths[4], CurElev, NextElev, AltDist, AltCumulativeLength, AltElev1, AltElev2;
Point2d Pt1, Pt2, SourcePt, SourcePtMeters, LineIntersection, TempLineIntersection, StartSeg, EndSeg, 
	AltStartSeg, AltEndSeg, AltLineIntersection;
long Index, CurIndex, NextIndex, PrevIndex, SegCt, MaxSegCt, SegIndex, CurSegDataCt, SegDataCt;
bool Interpolate, EquationOK, OffStartEnd, OffEndEnd, OffEnd, SinglePoint, FirstSegment, LastSegment, TestBothEndPoints;
CoordSys *MyCoords;
VectorPoint *CurPoint, *SplinePoints[4], *NextPoint, *SegStartPoint, *SegEndPoint;
JoeSegmentData **SegData;
VertexDEM CurVert;
GeneralLinearEquation SourceBisEq, LineEq, TempEq;

MyCoords = GetCoordSys();
MetersPerDegLon = MetersPerDegLat * cos(CurNode->Lat * PiOver180);
Interpolate = false;
Elev = 0.0;
MinDist = AltDist = FLT_MAX;
if (Slope)
	*Slope = 0.0;
DistXYZ[2] = DistXYZ[1] = DistXYZ[0] = 0.0f;
SplinePoints[3] = SplinePoints[2] = SplinePoints[1] = SplinePoints[0] = NULL;
CurSegDataCt = 0;

if (! (JRendData && JRendData->SegData[0]))
	CalcBisectrices(MetersPerDegLat, ConnectEnds);
if (SegData = JRendData ? JRendData->SegData: NULL)
	{
	// set up the node we're comparing to the vector
	SourcePt[0] = CurNode->Lon;
	SourcePt[1] = CurNode->Lat;
	if (SourcePt[0] - JRendData->OrigLon > 180.0)
		SourcePt[0] -= 360.0;
	if (SourcePt[0] - JRendData->OrigLon < -180.0)
		SourcePt[0] += 360.0;
	SourcePtMeters[0] = SourcePt[0] * MetersPerDegLon;
	SourcePtMeters[1] = SourcePt[1] * MetersPerDegLat;

	// where on the vector are we?
	Index = SegmentDetail->Index;
	CurPoint = SegData[0][Index].MyPoint;
	// leading nodes are at the distal end of the segment
	if (SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_LEADINGNODE))
		CurPoint = SegData[0][++Index].MyPoint;
	NextIndex = Index + 1;
	if (NextIndex >= JRendData->NumSegs && JRendData->ConnectEnds)
		NextIndex = 0;
	NextPoint = NextIndex < JRendData->NumSegs ? SegData[0][NextIndex].MyPoint: NULL;
	
	// conditions for determining evaluation steps. These conditions were added 8/7/08 GRH
	ConnectEnds = (JRendData->ConnectEnds && JRendData->NumSegs > 2);
	OffStartEnd = (! ConnectEnds && (Index == 0 && SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_TRAILINGNODE)));
	OffEndEnd = (! ConnectEnds && (Index == JRendData->NumSegs - 1));
	OffEnd = (OffStartEnd || OffEndEnd);
	SinglePoint = (JRendData->NumSegs < 2 || GetLineStyle() < 4);
	FirstSegment = (! ConnectEnds && Index == 0 && ! OffEnd);
	LastSegment = (! ConnectEnds && Index == JRendData->NumSegs - 2 && ! OffEnd);
	TestBothEndPoints = (FirstSegment && LastSegment);
	// evaluate one point and leave
		// SinglePoint
	// evaluate one point and either the first or last segment
		// OffEnd
	// evaluate one point and the first or last two segments, assuming there are more than two segments
		// FirstSegment || LastSegment
	// evaluate three segments assuming there are four or more segments
		// ! (OffEnd || FirstSegment || LastSegment)

	// there are single nodes at each end of the vector and on the outside of bends
	// The following two tests were in place until 8/7/08 but were found to not evaluate distance to end points
	// sometimes when they needed to be. Replaced 8/7/08 GRH
	//if (SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_SINGLENODE))
	//	{
		// for the single nodes on outside of bends we skip this test
		//if (! NextPoint || JRendData->NumSegs < 2 || GetLineStyle() < 4 || (Index == 0 && ! JRendData->ConnectEnds && SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_TRAILINGNODE)))
		//	{
	if (OffEnd || SinglePoint || FirstSegment || LastSegment)
		{
		if (LastSegment && ! TestBothEndPoints)
			{
			CurIndex = Index + 1;
			CurPoint = SegData[0][CurIndex].MyPoint;
			} // if
		else
			CurIndex = Index;
		// find distance to end point node
		Pt1[0] = SegData[0][CurIndex].PtX;
		Pt1[1] = SegData[0][CurIndex].PtY;
		SourcePtMeters[0] = (SourcePt[0] - SegData[0][CurIndex].PtLon) * SegData[0][CurIndex].MetersPerDegLon + Pt1[0];
		MinDist = DistPoint2Point(Pt1, SourcePtMeters);
		DistXYZ[0] = (float)MinDist;
		Elev = CurPoint->Elevation;
		Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
		DistXYZ[2] = (float)(CurNode->Elev - Elev);
		DistXYZ[1] = (float)SegData[0][CurIndex].CumulativeLength;
		if (TestBothEndPoints)
			{
			CurIndex = Index + 1;
			CurPoint = SegData[0][CurIndex].MyPoint;
			// find distance to end point node
			Pt1[0] = SegData[0][CurIndex].PtX;
			Pt1[1] = SegData[0][CurIndex].PtY;
			SourcePtMeters[0] = (SourcePt[0] - SegData[0][CurIndex].PtLon) * SegData[0][CurIndex].MetersPerDegLon + Pt1[0];
			if ((NewDist = DistPoint2Point(Pt1, SourcePtMeters)) < MinDist)
				{
				MinDist = NewDist;
				DistXYZ[0] = (float)MinDist;
				Elev = CurPoint->Elevation;
				Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
				DistXYZ[2] = (float)(CurNode->Elev - Elev);
				DistXYZ[1] = (float)SegData[0][CurIndex].CumulativeLength;
				} // if
			} // if
		if (SinglePoint)	// prior to 8/7 changes this was an unconditional return
			return (MinDist);
		} // if

	/* segments to test
	OffStartEnd 1
	OffEndEnd 0
	FirstSegment 1, 2
	LastSegment 0, 1
	FirstSegment && LastSegment 1
	! (OffEnd || FirstSegment || LastSegment) 0, 1, 2
	*/
	SegCt = 0;
	MaxSegCt = 3;

	if (OffStartEnd || FirstSegment)
		SegCt = 1;
	if (OffEndEnd)
		MaxSegCt = 1;
	else if (OffStartEnd || LastSegment)
		MaxSegCt = 2;
		
	Interpolate = false;
	for (; SegCt < MaxSegCt; ++SegCt)
		{
		if (SegCt == 0)
			CurIndex = Index - 1;
		else if (SegCt == 1)
			CurIndex = Index;
		else
			CurIndex = Index + 1;
		NextIndex = CurIndex + 1;
		if (CurIndex >= JRendData->NumSegs)
			CurIndex -= JRendData->NumSegs;
		if (CurIndex < 0)
			CurIndex += JRendData->NumSegs;
		if (NextIndex >= JRendData->NumSegs)
			NextIndex -= JRendData->NumSegs;
		if (NextIndex < 0)
			NextIndex += JRendData->NumSegs;
		//if (CurIndex > 0)
		//	CurSegLen = SegData[CurIndex].CumulativeLength - SegData[CurIndex - 1].CumulativeLength;
		//else
		//	CurSegLen = SegData[CurIndex].CumulativeLength;
		CurPoint = SegData[0][CurIndex].MyPoint;
		NextPoint = SegData[0][NextIndex].MyPoint;
		
		// test to see if point is possibly useful
		// create a line from source point (lat/lon) through Bisectrix intersection
		// and see if its intersection with the vector segment is within the segment
		// Try using the longitude conversion for both ends of the segment
		long RefIndex = CurIndex;
		for (CurSegDataCt = 0; CurSegDataCt < 2 && ! Interpolate; ++CurSegDataCt, RefIndex = NextIndex)
			{
			Pt1[0] = SegData[CurSegDataCt][CurIndex].PtX;
			Pt1[1] = SegData[CurSegDataCt][CurIndex].PtY;
			if (! CurSegDataCt)
				Pt2[0] = (SegData[0][NextIndex].PtLon - SegData[0][CurIndex].PtLon) * SegData[0][CurIndex].MetersPerDegLon + SegData[0][CurIndex].PtX;
			else
				Pt2[0] = SegData[0][NextIndex].PtX;
			Pt2[1] = SegData[0][NextIndex].PtY;
			CurElev = CurPoint->Elevation;
			NextElev = NextPoint->Elevation;
			SourcePtMeters[0] = (SourcePt[0] - SegData[0][RefIndex].PtLon) * SegData[0][RefIndex].MetersPerDegLon + SegData[0][RefIndex].PtX;
			
			NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2, TempLineIntersection);
		//if (NewDist <= MinDist)
			{
			//MinDist = NewDist;
			//DistXYZ[0] = (float)MinDist;
			EquationOK = false;
			if (NewDist == 0.0)
				{
				if (Pt1[0] == TempLineIntersection[0] && Pt1[1] == TempLineIntersection[1])
					{
					MinDist = NewDist;
					DistXYZ[0] = (float)MinDist;
					if (CurIndex > 0)
						DistXYZ[1] = (float)SegData[CurSegDataCt][CurIndex - 1].CumulativeLength;
					else
						DistXYZ[1] = 0.0f;
					Elev = ElevDatum + (CurElev - ElevDatum) * Exageration;
					DistXYZ[2] = (float)(CurNode->Elev - Elev);
					return (0.0);
					} // else if
				else if (Pt2[0] == TempLineIntersection[0] && Pt2[1] == TempLineIntersection[1])
					{
					MinDist = NewDist;
					DistXYZ[0] = (float)MinDist;
					DistXYZ[1] = (float)SegData[CurSegDataCt][CurIndex].CumulativeLength;
					Elev = ElevDatum + (NextElev - ElevDatum) * Exageration;
					DistXYZ[2] = (float)(CurNode->Elev - Elev);
					return (0.0);
					} // else if
				} // else
			if (CalcGeneralLinearEquation(Pt1, Pt2, &LineEq))
				{
				if (SegData[CurSegDataCt][CurIndex].BisectrixOK)
					{
					if (CalcGeneralLinearEquation(SourcePtMeters, SegData[CurSegDataCt][CurIndex].BisectrixData, &SourceBisEq))
						{
						EquationOK = true;
						} // if no error
					} // if bisectrices not parallel
				else
					{
					// create line from source pt parallel to bisectrix
					TempEq.CoefA = SegData[CurSegDataCt][CurIndex].BisectrixData[0];
					TempEq.CoefB = SegData[CurSegDataCt][CurIndex].BisectrixData[1];
					TempEq.CoefC = SegData[CurSegDataCt][CurIndex].BisectrixData[2];
					if (CalcParallelLinearEquation(SourcePtMeters, &TempEq, &SourceBisEq))
						{
						EquationOK = true;
						} // if no error
					} // else bisectrices are parallel
				if (EquationOK)
					{
					if (CalcLineIntersection(&SourceBisEq, &LineEq, TempLineIntersection))
						{
						if (PointContainedInSegment(Pt1, Pt2, TempLineIntersection))
							{
							if (NewDist < MinDist)
								{
								MinDist = AltDist = NewDist;
								DistXYZ[0] = (float)MinDist;
								LineIntersection[0] = TempLineIntersection[0];
								LineIntersection[1] = TempLineIntersection[1];
								StartSeg[0] = Pt1[0];
								StartSeg[1] = Pt1[1];
								EndSeg[0] = Pt2[0];
								EndSeg[1] = Pt2[1];
								if (Pt1[0] == LineIntersection[0] && Pt1[1] == LineIntersection[1])
									{
									if (CurIndex > 0)
										DistXYZ[1] = (float)SegData[CurSegDataCt][CurIndex - 1].CumulativeLength;
									else
										DistXYZ[1] = 0.0f;
									Elev = ElevDatum + (CurElev - ElevDatum) * Exageration;
									DistXYZ[2] = (float)(CurNode->Elev - Elev);
									return (MinDist);
									} // if
								else if (Pt2[0] == LineIntersection[0] && Pt2[1] == LineIntersection[1])
									{
									DistXYZ[1] = (float)SegData[CurSegDataCt][CurIndex].CumulativeLength;
									Elev = ElevDatum + (NextElev - ElevDatum) * Exageration;
									DistXYZ[2] = (float)(CurNode->Elev - Elev);
									return (MinDist);
									} // else if
								else
									{
									Interpolate = true;
									SegIndex = CurIndex;
									SegStartPoint = CurPoint;
									SegEndPoint = NextPoint;
									SegDataCt = CurSegDataCt;
									} // else
								} // if
							} // if
						else if (SegCt == 1 && NewDist < MinDist && NewDist < AltDist)
							{
							AltDist = NewDist;
							AltCumulativeLength = CurIndex > 0 ? SegData[CurSegDataCt][CurIndex - 1].CumulativeLength: 0.0;
							AltElev1 = CurElev;
							AltElev2 = NextElev;
							AltStartSeg[0] = Pt1[0];
							AltStartSeg[1] = Pt1[1];
							AltEndSeg[0] = Pt2[0];
							AltEndSeg[1] = Pt2[1];
							AltLineIntersection[0] = TempLineIntersection[0];
							AltLineIntersection[1] = TempLineIntersection[1];
							} // else set up for a fallback position
						} // if lines intersect
					} // if EquationOK
				} // if
			} // if
			} // for RefCt
		} // for
	// section above re-enabled 7/1/08 to allow smooth elevation interpolation lengthwise along vector
	if (Interpolate)
		{
		/*                            |SegIndex
		/*                            |<---- SegLengths[0]--->|
		|<-----	SegLengths[1]-------->|<------- SegLengths[2]------>|<------- SegLengths[3]------>|
		*-----------------------------*-----------------------X-----*-----------------------------*
		SplinePoints[0]			SplinePoints[1]		  		SplinePoints[2]					SplinePoints[3]
		*/
		SegLengths[0] = DistPoint2Point(LineIntersection, StartSeg);
		SegLengths[2] = DistPoint2Point(EndSeg, StartSeg);
		if (SegLengths[0] > SegLengths[2])
			SegLengths[0] = 1.0;
		else if (SegLengths[0] < 0.0)
			SegLengths[0] = 0.0;
		else
			SegLengths[0] /= SegLengths[2];
		PrevIndex = SegIndex - 1;
		NextIndex = SegIndex + 1;
		if (PrevIndex < 0 && JRendData->ConnectEnds)
			PrevIndex += JRendData->NumSegs;
		if (NextIndex >= JRendData->NumSegs && JRendData->ConnectEnds)
			NextIndex -= JRendData->NumSegs;
		if (PrevIndex >= 0)
			{
			SplinePoints[0] = SegData[SegDataCt][PrevIndex].MyPoint;
			if (PrevIndex > 0)
				SegLengths[1] = SegData[SegDataCt][PrevIndex].CumulativeLength - SegData[SegDataCt][PrevIndex - 1].CumulativeLength;
			else
				SegLengths[1] = SegData[SegDataCt][PrevIndex].CumulativeLength;
			if (PrevIndex < SegIndex)
				{
				SegLengths[2] = SegData[SegDataCt][SegIndex].CumulativeLength - SegData[SegDataCt][PrevIndex].CumulativeLength;
				DistXYZ[1] = (float)(SegData[SegDataCt][PrevIndex].CumulativeLength + SegLengths[0] * SegLengths[2]);
				} // if
			else
				{
				SegLengths[2] = SegData[SegDataCt][SegIndex].CumulativeLength;
				DistXYZ[1] = (float)(SegLengths[0] * SegLengths[2]);
				} // else
			} // if
		else
			{
			SplinePoints[0] = NULL;
			SegLengths[1] = 0.0;
			SegLengths[2] = SegData[SegDataCt][SegIndex].CumulativeLength;
			DistXYZ[1] = (float)(SegLengths[0] * SegLengths[2]);
			} // else
		SplinePoints[1] = SegStartPoint;
		SplinePoints[2] = SegEndPoint;
		SplinePoints[3] = NextIndex < JRendData->NumSegs - 1 ? SegData[SegDataCt][NextIndex + 1].MyPoint: NULL;
		if (SplinePoints[3])
			{
			if (NextIndex > SegIndex)
				SegLengths[3] = SegData[SegDataCt][NextIndex].CumulativeLength - SegData[SegDataCt][SegIndex].CumulativeLength;
			else
				SegLengths[3] = SegData[SegDataCt][NextIndex].CumulativeLength;
			} // if
		else
			SegLengths[3] = 0.0;
		if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
			DistXYZ[0] = -DistXYZ[0];
		if (SplineElev)
			{
			Elev = SplineElevation(SplinePoints, SegLengths, Slope);
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			} // else
		else if (Slope)
			{
			Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
				* SegLengths[0];
			*Slope = (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation) / SegLengths[2];
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			} // else if
		else
			{
			Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
				* SegLengths[0];
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			} // else
		} // if
	else if (AltDist < FLT_MAX)
		{
		MinDist = AltDist;
		DistXYZ[0] = (float)AltDist;
		// distance from intersection to start of segment
		NewDist = DistPoint2Point(AltLineIntersection, AltStartSeg);
		// length of segment
		AltDist = DistPoint2Point(AltStartSeg, AltEndSeg);
		// The theory is that if the distance from the intersection to the start of the segment is more than half the 
		// length of the segment then the intersection was on the far end of the segment, otherwise it was off the 
		// near end and towards the vector origin which means it has to be subtracted from the distance of the 
		// start segment.
		if (NewDist <= AltDist * .5)
			{
			DistXYZ[1] = (float)(AltCumulativeLength - NewDist);
			Elev = AltElev1;
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			} // if
		else
			{
			DistXYZ[1] = (float)(AltCumulativeLength + NewDist);
			Elev = AltElev2;
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			} // if
		if (SolveTriangleArea2D(SourcePtMeters, AltStartSeg, AltEndSeg) >= 0)
			DistXYZ[0] = -DistXYZ[0];
		} // else if fallback on closest point found
	else if (MinDist < FLT_MAX)
		{
		if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
			DistXYZ[0] = -DistXYZ[0];
		} // else
//	Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
	DistXYZ[2] = (float)(CurNode->Elev - Elev);
	#ifdef WCS_OUTPUT_BISECTRIXDETAIL
	char DebugStr[256];
	sprintf(DebugStr, "I=%d/%d I=%d [0]=%f [1]=%f SL=%f Lat=%.0f Lon=%.0f\n", Index, SegIndex, Interpolate, 
		DistXYZ[0], DistXYZ[1], SegLengths[0], (CurNode->Lat - 39.999) * 10E6, (CurNode->Lon - 99.999) * 10E6);
	OutputDebugStr(DebugStr);
	#endif // WCS_OUTPUT_BISECTRIXDETAIL
	return (MinDist);
	} // if SegData
	
return (0.0);

} // Joe::MinDistToPoint

/*===========================================================================*/
#else // WCS_COMBINATION_V2_V3_MINDIST

double Joe::MinDistToPoint(VectorNode *CurNode, TfxDetail *SegmentDetail, double MetersPerDegLat, double Exageration, double ElevDatum, 
	int SplineElev, int ConnectEnds, double &Elev, float *DistXYZ, double *Slope)
{
double MinDist, NewDist, MetersPerDegLon, CurMetersPerDegLon, NextMetersPerDegLon, CurSegLen, SegLengths[4];
Point2d Pt1, Pt2, SourcePt, SourcePtMeters, LineIntersection, StartSeg, EndSeg;
long Index, Interpolate;
CoordSys *MyCoords;
VectorPoint *CurPoint, *SplinePoints[4], *LastPoint, *NextPoint;
JoeSegmentData *SegData;
VertexDEM CurVert, NextVert;
//GeneralLinearEquation SourceBisEq, LineEq, TempEq;

MyCoords = GetCoordSys();

MetersPerDegLon = MetersPerDegLat * cos(CurNode->Lat * PiOver180);

Interpolate = 0;
Elev = 0.0;
if (Slope)
	*Slope = 0.0;
DistXYZ[2] = DistXYZ[1] = DistXYZ[0] = 0.0f;
SplinePoints[3] = SplinePoints[2] = SplinePoints[1] = SplinePoints[0] = NULL;
MinDist = FLT_MAX;
LastPoint = NULL;

if (! (JRendData && JRendData->SegData[0]))
	CalcBisectrices(MetersPerDegLat, ConnectEnds);
if (SegData = JRendData ? JRendData->SegData: NULL)
	{
	Index = SegmentDetail->Index;
	CurPoint = SegData[Index].MyPoint;
	if (SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_SINGLENODE) && SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_LEADINGNODE))
		CurPoint = CurPoint->Next;
	NextPoint = Index < SegData->NumSegs - 1 ? SegData[Index + 1].MyPoint: CurPoint->Next;
	SourcePt[0] = CurNode->Lon;
	SourcePt[1] = CurNode->Lat;
	SourcePtMeters[0] = CurNode->Lon * MetersPerDegLon;
	SourcePtMeters[1] = CurNode->Lat * MetersPerDegLat;
	CurMetersPerDegLon = SegData[Index].MetersPerDegLon;
	NextMetersPerDegLon = Index < SegData->NumSegs - 1 ? SegData[Index + 1].MetersPerDegLon: MetersPerDegLat * cos(NextPoint->Latitude * PiOver180);
	// find distance to first end point
	if (CurPoint->ProjToDefDeg(MyCoords, &CurVert))
		{
		// keep it close
		if (SourcePt[0] - CurVert.Lon > 180.0)
			CurVert.Lon += 360.0;
		else if (SourcePt[0] - CurVert.Lon < -180.0)
			CurVert.Lon -= 360.0;
		Pt1[0] = CurMetersPerDegLon * CurVert.Lon;
		Pt1[1] = MetersPerDegLat * CurVert.Lat;
		NewDist = DistPoint2Point(Pt1, SourcePtMeters);
		if (NewDist < MinDist)
			{
			MinDist = NewDist;
			DistXYZ[0] = (float)MinDist;
			Elev = CurVert.Elev;
			} // if
		if (SegmentDetail->FlagCheck(WCS_TFXDETAIL_FLAG_SINGLENODE))// || GetLineStyle() < 4 || ! GetSecondRealPoint())
			{
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			DistXYZ[2] = (float)(CurNode->Elev - Elev);
			return (MinDist);
			} // if
		} // if
	else
		return (0.0);	// projection error, might as well quit now
	if (NextPoint->ProjToDefDeg(MyCoords, &NextVert))
		{
		// keep it close
		if (SourcePt[0] - NextVert.Lon > 180.0)
			NextVert.Lon += 360.0;
		else if (SourcePt[0] - NextVert.Lon < -180.0)
			NextVert.Lon -= 360.0;
		Pt2[0] = NextMetersPerDegLon * NextVert.Lon;
		Pt2[1] = MetersPerDegLat * NextVert.Lat;
		NewDist = DistPoint2Point(Pt2, SourcePtMeters);
		if (NewDist < MinDist)
			{
			MinDist = NewDist;
			DistXYZ[0] = (float)MinDist;
			DistXYZ[1] = (float)(SegData[Index].CumulativeLength);
			Elev = NextVert.Elev;
			} // if
		} // if
	else
		return (0.0);	// projection error, might as well quit now

	// test to see if point is possibly useful
	// create a line from source point (lat/lon) through Bisectrix intersection
	// and see if its intersection with the vector segment is within the segment
	if (Index > 0)
		CurSegLen = SegData[Index].CumulativeLength - SegData[Index - 1].CumulativeLength;
	else
		CurSegLen = SegData[Index].CumulativeLength;
	NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2, LineIntersection);
	if (NewDist < MinDist)
		{
		MinDist = NewDist;
		//DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0 ? (float)-MinDist: (float)MinDist;
		DistXYZ[0] = (float)MinDist;
		Elev = NextVert.Elev;
		// store data for later slope and elevation calc
		Interpolate = 1;
		} // if
	/*
	if (CalcGeneralLinearEquation(Pt1, Pt2, &LineEq))
		{
		if (SegData[Index].BisectrixOK)
			{
			if (CalcGeneralLinearEquation(SourcePtMeters, SegData[Index].BisectrixData, &SourceBisEq))
				{
				if (CalcLineIntersection(&SourceBisEq, &LineEq, LineIntersection))
					{
					if (PointContainedInSegment(Pt1, Pt2, LineIntersection))
						{
						NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2);
						if (NewDist < MinDist)
							{
							MinDist = NewDist;
							//DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0 ? (float)-MinDist: (float)MinDist;
							DistXYZ[0] = (float)MinDist;
							Elev = NextVert.Elev;
							// store data for later slope and elevation calc
							Interpolate = 1;
							} // if
						} // if
					} // if lines intersect
				} // if no error
			} // if bisectrices not parallel
		else
			{
			// create line from source pt parallel to bisectrix
			TempEq.CoefA = SegData[Index].BisectrixData[0];
			TempEq.CoefB = SegData[Index].BisectrixData[1];
			TempEq.CoefC = SegData[Index].BisectrixData[2];
			if (CalcParallelLinearEquation(SourcePtMeters, &TempEq, &SourceBisEq))
				{
				if (CalcLineIntersection(&SourceBisEq, &LineEq, LineIntersection))
					{
					if (PointContainedInSegment(Pt1, Pt2, LineIntersection))
						{
						NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2);
						if (NewDist < MinDist)
							{
							MinDist = NewDist;
							//DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0 ? (float)-MinDist: (float)MinDist;
							DistXYZ[0] = (float)MinDist;
							Elev = NextVert.Elev;
							// store data for later slope and elevation calc
							Interpolate = 1;
							} // if
						} // if
					} // if lines intersect
				} // if no error
			} // else bisectrices are parallel
		} // if
	*/
	if (Interpolate)
		{
		/*
		|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
		*-----------------------------*-----------------------X-----*-----------------------------*
		Point[0]					Point[1]		  				Point[2]					Point[3]
		*/
		if (Index > 0)
			{
			SplinePoints[0] = SegData[Index - 1].MyPoint;
			if (Index > 1)
				SegLengths[1] = SegData[Index - 1].CumulativeLength - SegData[Index - 2].CumulativeLength;
			else
				SegLengths[1] = SegData[Index - 1].CumulativeLength;
			} // if
		else
			{
			SplinePoints[0] = NULL;
			SegLengths[1] = 0.0;
			} // else
		SplinePoints[1] = CurPoint;
		SplinePoints[2] = NextPoint;
		SplinePoints[3] = NextPoint ? NextPoint->Next: NULL;
		SegLengths[0] = DistPoint2Point(LineIntersection, Pt1);
		DistXYZ[1] = (float)(SegData[Index].CumulativeLength + SegLengths[0]);
		SegLengths[0] /= CurSegLen;
		SegLengths[2] = CurSegLen;
		if (SplinePoints[3])
			SegLengths[3] = SegData[Index + 1].CumulativeLength - SegData[Index ].CumulativeLength;
		else
			SegLengths[3] = 0.0;
		StartSeg[0] = Pt1[0];
		StartSeg[1] = Pt1[1];
		EndSeg[0] = Pt2[0];
		EndSeg[1] = Pt2[1];
		if (SplineElev)
			{
			Elev = SplineElevation(SplinePoints, SegLengths, Slope);
			if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
				DistXYZ[0] = -DistXYZ[0];
			} // else
		else if (Slope)
			{
			Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
				* SegLengths[0];
			*Slope = (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation) / SegLengths[2];
			if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
				DistXYZ[0] = -DistXYZ[0];
			} // else if
		else
			{
			Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
				* SegLengths[0];
			if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
				DistXYZ[0] = -DistXYZ[0];
			} // else
		} // if
	else
		{
		if (SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0)
			DistXYZ[0] = -DistXYZ[0];
		} // else
	Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
	DistXYZ[2] = (float)(CurNode->Elev - Elev);
	return (MinDist);
	} // if SegData
	
return (0.0);

} // Joe::MinDistToPoint
#endif // WCS_COMBINATION_V2_V3_MINDIST

/*===========================================================================*/

// this function replaces that below used in V5/VNS 1
// It is used to evaluate terraffectors and streams and textures for any align-to vector textures

double Joe::MinDistToPoint(double Lat, double Lon, double ElevPt, double MetersPerDegLat, double Exageration, double ElevDatum, 
	int SplineLatLon, int SplineElev, int ConnectEnds, int SkipFirstPoint, int &OffEnd, double &Elev, float *DistXYZ, double *Slope)
{
double MinDist, NewDist, /*DistFromPt1, */MetersPerDegLon, CurSegLen, LastSegLen, TotalSegLen, SegLengths[4];
Point2d Pt1, Pt2, SourcePt, SourcePtMeters, LineIntersection, StartSeg, EndSeg;
long Index, Interpolate, FillNextSegLen = 0, FoundMatch = 0, CurSegDataCt;
CoordSys *MyCoords;
VectorPoint *CurPoint, *SplinePoints[4], *LastPoint, *NextPoint;
JoeSegmentData **SegData;
VertexDEM CurVert, NextVert;
GeneralLinearEquation SourceBisEq, LineEq, TempEq;

MyCoords = GetCoordSys();

if ((CurPoint = GetFirstRealPoint()) && CurPoint->ProjToDefDeg(MyCoords, &CurVert))
	MetersPerDegLon = MetersPerDegLat * cos(CurVert.Lat * PiOver180);
else
	MetersPerDegLon = MetersPerDegLat * cos(Lat * PiOver180);
OffEnd = 0;
Interpolate = 0;
Elev = 0.0;
if (Slope)
	*Slope = 0.0;
DistXYZ[2] = DistXYZ[1] = DistXYZ[0] = 0.0f;
SplinePoints[3] = SplinePoints[2] = SplinePoints[1] = SplinePoints[0] = NULL;
TotalSegLen = LastSegLen = 0.0;
MinDist = FLT_MAX;
LastPoint = NULL;
CurSegDataCt = 0;

if (! TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (CurPoint = GetFirstRealPoint())
		{
		SourcePt[0] = Lon;
		SourcePt[1] = Lat;
		SourcePtMeters[0] = Lon * MetersPerDegLon;
		SourcePtMeters[1] = Lat * MetersPerDegLat;
		if (GetLineStyle() < 4 || ! CurPoint->Next)
			{
			for (; CurPoint; CurPoint = CurPoint->Next)
				{
				if (LastPoint && LastPoint->Latitude == CurPoint->Latitude && LastPoint->Longitude == CurPoint->Longitude)
					continue;
				if (CurPoint->ProjToDefDeg(MyCoords, &CurVert))
					{
					// keep it close
					if (SourcePt[0] - CurVert.Lon > 180.0)
						CurVert.Lon += 360.0;
					else if (SourcePt[0] - CurVert.Lon < -180.0)
						CurVert.Lon -= 360.0;
					Pt1[0] = MetersPerDegLon * CurVert.Lon;
					Pt1[1] = MetersPerDegLat * CurVert.Lat;
					NewDist = DistPoint2Point(Pt1, SourcePtMeters);
					if (NewDist < MinDist)
						{
						MinDist = NewDist;
						DistXYZ[0] = (float)MinDist;
						Elev = CurVert.Elev;
						} // if
					} // if
				else
					return (0.0);	// projection error, might as well quit now
				LastPoint = CurPoint;
				} // while
			Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
			DistXYZ[2] = (float)(ElevPt - Elev);
			return (MinDist);
			} // if point style
		else
			{
			if (! (JRendData && JRendData->SegData[0]))
				CalcBisectrices(MetersPerDegLat, ConnectEnds);
			if (SegData = JRendData ? JRendData->SegData: NULL)
				{
				Index = SkipFirstPoint && GetSecondRealPoint() ? 1: 0;
				CurPoint = Index ? GetSecondRealPoint(): GetFirstRealPoint();
				if (Index == 1)
					{
					GetFirstRealPoint()->ProjToDefDeg(MyCoords, &CurVert);
					CurPoint->ProjToDefDeg(MyCoords, &NextVert);
					Pt1[0] = CurVert.Lon;
					Pt1[1] = CurVert.Lat;
					Pt2[0] = NextVert.Lon;
					Pt2[1] = NextVert.Lat;
					if (Pt2[0] - Pt1[0] > 180.0)
						Pt1[0] += 360.0;
					else if (Pt2[0] - Pt1[0] < -180.0)
						Pt1[0] -= 360.0;
					Pt1[0] *= MetersPerDegLon;
					Pt1[1] *= MetersPerDegLat;
					Pt2[0] *= MetersPerDegLon;
					Pt2[1] *= MetersPerDegLat;
					LastSegLen = TotalSegLen = DistPoint2Point(Pt1, Pt2);
					} // if
				for ( ; CurPoint && (CurPoint->Next || ConnectEnds) && (! FoundMatch || FillNextSegLen); CurPoint = CurPoint->Next)
					{
					if (LastPoint && LastPoint->Latitude == CurPoint->Latitude && LastPoint->Longitude == CurPoint->Longitude)
						continue;
					// find distance to first end point
					if (! LastPoint)
						{
						if (CurPoint->ProjToDefDeg(MyCoords, &CurVert))
							{
							// keep it close
							if (SourcePt[0] - CurVert.Lon > 180.0)
								CurVert.Lon += 360.0;
							else if (SourcePt[0] - CurVert.Lon < -180.0)
								CurVert.Lon -= 360.0;
							Pt1[0] = MetersPerDegLon * CurVert.Lon;
							Pt1[1] = MetersPerDegLat * CurVert.Lat;
							if (ComparePoint2d(Pt1, SourcePtMeters, .001))
								{
								OffEnd = 0;
								MinDist = 0.0;
								DistXYZ[0] = (float)MinDist;
								Elev = CurVert.Elev;
								FoundMatch = 1;
								} // if we landed on the vertex
							else
								{
								NewDist = DistPoint2Point(Pt1, SourcePtMeters);
								if (NewDist < MinDist)
									{
									OffEnd = 1;
									MinDist = NewDist;
									DistXYZ[0] = (float)MinDist;
									Elev = CurVert.Elev;
									// Interpolate = 0; already set above
									} // if
								} // else
							} // if
						else
							return (0.0);	// projection error, might as well quit now
						} // if
					// test to see if point is possibly useful
					// create a line from source point (lat/lon) through Bisectrix intersection
					// and see if its intersection with the vector segment is within the segment
					if (! FoundMatch || FillNextSegLen)
						{
						NextPoint = CurPoint->Next ? CurPoint->Next: GetFirstRealPoint();
						if (CurPoint->Latitude == NextPoint->Latitude && CurPoint->Longitude == NextPoint->Longitude)
							continue;
						if (NextPoint->ProjToDefDeg(MyCoords, &NextVert))
							{
							Pt1[0] = CurVert.Lon;
							Pt1[1] = CurVert.Lat;
							Pt2[0] = NextVert.Lon;
							Pt2[1] = NextVert.Lat;
							if (Pt2[0] - Pt1[0] > 180.0)
								Pt1[0] += 360.0;
							else if (Pt2[0] - Pt1[0] < -180.0)
								Pt1[0] -= 360.0;
							if (Pt1[0] - SourcePt[0] > 180.0)
								{
								SourcePt[0] += 360.0;
								SourcePtMeters[0] = Lon * MetersPerDegLon;
								} // if
							else if (Pt1[0] - SourcePt[0] < -180.0)
								{
								SourcePt[0] -= 360.0;
								SourcePtMeters[0] = Lon * MetersPerDegLon;
								} // if
							Pt1[0] *= MetersPerDegLon;
							Pt1[1] *= MetersPerDegLat;
							Pt2[0] *= MetersPerDegLon;
							Pt2[1] *= MetersPerDegLat;
							CurSegLen = DistPoint2Point(Pt1, Pt2);
							if (FillNextSegLen)
								SegLengths[3] = CurSegLen;
							FillNextSegLen = 0;
							if (FoundMatch)
								break;	// we were just here to fill in the next segment length for splining
							if (ComparePoint2d(Pt2, SourcePtMeters, .001))
								{
								FoundMatch = 1;
								OffEnd = 0;
								MinDist = 0.0;
								DistXYZ[0] = (float)MinDist;
								DistXYZ[1] = (float)(TotalSegLen + CurSegLen);
								Elev = NextVert.Elev;
								// store data for later slope and elevation calc
								Interpolate = Slope ? 1: 0;
								if (Interpolate)
									{
									SplineLatLon = 0;
									SplinePoints[0] = LastPoint;
									SplinePoints[1] = CurPoint;
									SplinePoints[2] = NextPoint;
									SplinePoints[3] = NextPoint ? NextPoint->Next: NULL;
									SegLengths[0] = 1.0;
									SegLengths[1] = LastSegLen;
									SegLengths[2] = CurSegLen;
									SegLengths[3] = 0.0;
									StartSeg[0] = Pt1[0];
									StartSeg[1] = Pt1[1];
									EndSeg[0] = Pt2[0];
									EndSeg[1] = Pt2[1];
									FillNextSegLen = 1;
									} // if
								} // if
							/* didn't seem to do anything useful
							else if (CompareLine2Point2d(Pt1, Pt2, SourcePtMeters, .001, DistFromPt1))
								{
								FoundMatch = 1;
								OffEnd = 0;
								MinDist = 0.0;
								DistXYZ[0] = (float)MinDist;
								DistXYZ[1] = (float)(TotalSegLen + DistFromPt1);
								Elev = CurVert.Elev + (NextVert.Elev - CurVert.Elev) * (DistFromPt1 / CurSegLen);
								// store data for later slope and elevation calc
								Interpolate = Slope || SplineLatLon ? 1: 0;
								if (Interpolate)
									{
									SplinePoints[0] = LastPoint;
									SplinePoints[1] = CurPoint;
									SplinePoints[2] = NextPoint;
									SplinePoints[3] = NextPoint ? NextPoint->Next: NULL;
									SegLengths[0] = DistFromPt1 / CurSegLen;
									SegLengths[1] = LastSegLen;
									SegLengths[2] = CurSegLen;
									SegLengths[3] = 0.0;
									StartSeg[0] = Pt1[0];
									StartSeg[1] = Pt1[1];
									EndSeg[0] = Pt2[0];
									EndSeg[1] = Pt2[1];
									FillNextSegLen = 1;
									} // if
								} // else if
							*/
							else
								{
								if (CalcGeneralLinearEquation(Pt1, Pt2, &LineEq))
									{
									if (SegData[CurSegDataCt][Index].BisectrixOK)
										{
										if (CalcGeneralLinearEquation(SourcePtMeters, SegData[CurSegDataCt][Index].BisectrixData, &SourceBisEq))
											{
											if (CalcLineIntersection(&SourceBisEq, &LineEq, LineIntersection))
												{
												if (PointContainedInSegment(Pt1, Pt2, LineIntersection))
													{
													NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2);
													if (NewDist < MinDist)
														{
														OffEnd = 0;
														MinDist = NewDist;
														//DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0 ? (float)-MinDist: (float)MinDist;
														DistXYZ[0] = (float)MinDist;
														Elev = NextVert.Elev;
														// store data for later slope and elevation calc
														Interpolate = 1;
														SplinePoints[0] = LastPoint;
														SplinePoints[1] = CurPoint;
														SplinePoints[2] = NextPoint;
														SplinePoints[3] = NextPoint ? NextPoint->Next: NULL;
														SegLengths[0] = DistPoint2Point(LineIntersection, Pt1);
														DistXYZ[1] = (float)(TotalSegLen + SegLengths[0]);
														SegLengths[0] /= CurSegLen;
														SegLengths[1] = LastSegLen;
														SegLengths[2] = CurSegLen;
														SegLengths[3] = 0.0;
														StartSeg[0] = Pt1[0];
														StartSeg[1] = Pt1[1];
														EndSeg[0] = Pt2[0];
														EndSeg[1] = Pt2[1];
														FillNextSegLen = 1;
														} // if
													} // if
												} // if lines intersect
											} // if no error
										} // if bisectrices not parallel
									else
										{
										// create line from source pt parallel to bisectrix
										TempEq.CoefA = SegData[CurSegDataCt][Index].BisectrixData[0];
										TempEq.CoefB = SegData[CurSegDataCt][Index].BisectrixData[1];
										TempEq.CoefC = SegData[CurSegDataCt][Index].BisectrixData[2];
										if (CalcParallelLinearEquation(SourcePtMeters, &TempEq, &SourceBisEq))
											{
											if (CalcLineIntersection(&SourceBisEq, &LineEq, LineIntersection))
												{
												if (PointContainedInSegment(Pt1, Pt2, LineIntersection))
													{
													NewDist = DistPoint2LineContained(SourcePtMeters, Pt1, Pt2);
													if (NewDist < MinDist)
														{
														OffEnd = 0;
														MinDist = NewDist;
														//DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, Pt1, Pt2) >= 0 ? (float)-MinDist: (float)MinDist;
														DistXYZ[0] = (float)MinDist;
														Elev = NextVert.Elev;
														// store data for later slope and elevation calc
														Interpolate = 1;
														SplinePoints[0] = LastPoint;
														SplinePoints[1] = CurPoint;
														SplinePoints[2] = NextPoint;
														SplinePoints[3] = NextPoint ? NextPoint->Next: NULL;
														SegLengths[0] = DistPoint2Point(LineIntersection, Pt1);
														DistXYZ[1] = (float)(TotalSegLen + SegLengths[0]);
														SegLengths[0] /= CurSegLen;
														SegLengths[1] = LastSegLen;
														SegLengths[2] = CurSegLen;
														SegLengths[3] = 0.0;
														StartSeg[0] = Pt1[0];
														StartSeg[1] = Pt1[1];
														EndSeg[0] = Pt2[0];
														EndSeg[1] = Pt2[1];
														FillNextSegLen = 1;
														} // if
													} // if
												} // if lines intersect
											} // if no error
										} // else bisectrices are parallel
									} // if
								} // else
							// set up for next point
							CurVert.Lon = NextVert.Lon;
							CurVert.Lat = NextVert.Lat;
							CurVert.Elev = NextVert.Elev;
							TotalSegLen += CurSegLen;
							LastSegLen = CurSegLen;
							} // if projected
						else
							return (0.0);	// projection error
						} // if ! FoundMatch
					Index ++;
					LastPoint = CurPoint;
					} // for
				if (! FoundMatch && ! ConnectEnds)
					{
					// test last point distance to see if it is off end
					Pt1[0] = MetersPerDegLon * CurVert.Lon;
					Pt1[1] = MetersPerDegLat * CurVert.Lat;
					NewDist = DistPoint2Point(Pt1, SourcePtMeters);
					if (NewDist < MinDist)
						{
						OffEnd = 1;
						MinDist = NewDist;
						DistXYZ[0] = (float)MinDist;
						Elev = CurVert.Elev;
						if (Slope)
							*Slope = 0.0;
						DistXYZ[1] = (float)TotalSegLen;
						Interpolate = 0;
						} // if
					} // if
				if (Interpolate)
					{
					/*
					|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
					*-----------------------------*-----------------------X-----*-----------------------------*
					Point[0]					Point[1]		  				Point[2]					Point[3]
					*/
					if (FillNextSegLen)
						SplinePoints[3] = NULL;	// segment length didn't get filled because of duplicate points
					if (SplineLatLon)
						{
						// this version splines lat/lon too.
						Elev = SplineLatLonElevation(SplinePoints, SegLengths, Pt1, Slope);
						Pt1[0] *= MetersPerDegLon;
						Pt1[1] *= MetersPerDegLat;
						MinDist = DistPoint2Point(Pt1, SourcePtMeters);
						DistXYZ[0] = SolveTriangleArea2D(SourcePtMeters, StartSeg, Pt1) >= 0 ? (float)-MinDist: (float)MinDist;
						} // if
					else if (SplineElev)
						{
						Elev = SplineElevation(SplinePoints, SegLengths, Slope);
						if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
							DistXYZ[0] = -DistXYZ[0];
						} // else
					else if (Slope)
						{
						Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
							* SegLengths[0];
						*Slope = (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation) / SegLengths[2];
						if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
							DistXYZ[0] = -DistXYZ[0];
						} // else if
					else
						{
						Elev = SplinePoints[1]->Elevation + (SplinePoints[2]->Elevation - SplinePoints[1]->Elevation)
							* SegLengths[0];
						if (SolveTriangleArea2D(SourcePtMeters, StartSeg, EndSeg) >= 0)
							DistXYZ[0] = -DistXYZ[0];
						} // else
					} // if
				Elev = ElevDatum + (Elev - ElevDatum) * Exageration;
				DistXYZ[2] = (float)(ElevPt - Elev);
				return (MinDist);
				} // if SegData
			} // else line style, need segment data
		} // if points
	} // if not DEM

return (0.0);

} // Joe::MinDistToPoint

/*===========================================================================*/

// note that this function does not connect last point back to origin
// used by terraffectors
// It is used to evaluate terraffectors and streams for the non-render situation where there 
// is no initialization process that does setup calculations or raster-based accelleration

double Joe::MinDistToPoint(double Lat, double Lon, double MetersPerDegLat, double *Elev, double *Slope)
{
Point2d ZeroPoint, StartSeg, EndSeg;
VectorPoint *StartPoint, *SplinePoint[4], *PrevPoint;
double MinDist = FLT_MAX, NewDist, lonscale, latscale, P1Offset, SplineDist[4], SegLength;
char UsePoint[2], SetNextDist = 0, CheckNextContained = 0;
CoordSys *MyCoords;
VertexDEM CurVert;

ZeroPoint[0] = 0.0;
ZeroPoint[1] = 0.0;
latscale = MetersPerDegLat;
lonscale = latscale * cos(Lat * PiOver180);

if (! TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (StartPoint = GetFirstRealPoint())
		{
		MyCoords = GetCoordSys();
		//StartPoint = GetFirstRealPoint();		// replaced by above
		if (GetLineStyle() < 4)
			{
			while (StartPoint)
				{
				if (StartPoint->ProjToDefDeg(MyCoords, &CurVert))
					{
					StartSeg[0] = EndSeg[0] = latscale * (CurVert.Lat - Lat);
					StartSeg[1] = EndSeg[1] = lonscale * (CurVert.Lon - Lon);
					NewDist = DistPoint2Line(ZeroPoint, StartSeg, EndSeg);
					if (NewDist < MinDist)
						{
						MinDist = NewDist;
						if (Elev)
							*Elev = CurVert.Elev;
						if (Slope)
							*Slope = 0.0;
						} // if
					StartPoint = StartPoint->Next;
					} // if
				else
					return (0.0);	// projection error, might as well quit now
				} // while
			} // if point style
		else
			{
			if (StartPoint->ProjToDefDeg(MyCoords, &CurVert))
				{
				StartSeg[0] = latscale * (CurVert.Lat - Lat);
				StartSeg[1] = lonscale * (CurVert.Lon - Lon);
				PrevPoint = NULL;
				SplineDist[0] = SplineDist[1] = SplineDist[2] = 0.0;
				if (StartPoint->Next)
					{
					while (StartPoint->Next)
						{
						if (StartPoint->Next->ProjToDefDeg(MyCoords, &CurVert))
							{
							EndSeg[0] = latscale * (CurVert.Lat - Lat);
							EndSeg[1] = lonscale * (CurVert.Lon - Lon);
			//				P1offset = 0.0;		// not needed since always set by DistPoint2LineContained
							if ((NewDist = DistPoint2LineContained(ZeroPoint, StartSeg, EndSeg, UsePoint, P1Offset, SegLength)) <= MinDist)
								{
								if (SetNextDist)
									{
									SplineDist[3] = SegLength;
									SetNextDist = 0;
									} // if
								if (NewDist == MinDist)
									{
									if (CheckNextContained && ! LastTrulyContained(SplinePoint, SplineDist, Lat, Lon, latscale, lonscale))
										{
										SplineDist[1] = SplineDist[2];
										SplineDist[2] = SegLength;
										SetNextDist = 1;

										SplineDist[0] = P1Offset;
										SplinePoint[0] = PrevPoint;
										SplinePoint[1] = StartPoint;
										SplinePoint[2] = StartPoint->Next;
										SplinePoint[3] = SplinePoint[2]->Next;
										} // if
									} // if
								else
									{
									MinDist = NewDist;
									SplineDist[1] = SplineDist[2];
									SplineDist[2] = SegLength;
									SetNextDist = 1;

									SplineDist[0] = P1Offset;
									SplinePoint[0] = PrevPoint;
									SplinePoint[1] = StartPoint;
									SplinePoint[2] = StartPoint->Next;
									SplinePoint[3] = SplinePoint[2]->Next;
									} // else
								if (! UsePoint[0])
									CheckNextContained = 1;
								else
									CheckNextContained = 0;
								/* removed so new SplineElevation() has access to all relevant points
								if (UsePoint[0] && UsePoint[1])
									{
									SplineDist[0] = P1Offset;
									SplinePoint[1] = StartPoint;
									SplinePoint[2] = StartPoint->Next;
									SplinePoint[0] = PrevPoint;
									SplinePoint[3] = SplinePoint[2]->Next;
									} // if
								else if (UsePoint[0])
									{
									SplinePoint[1] = StartPoint;
									SplinePoint[2] = SplinePoint[0] = SplinePoint[3] = NULL;
									} // else if
								else
									{
									SplinePoint[1] = StartPoint->Next;
									SplinePoint[2] = SplinePoint[0] = SplinePoint[3] = NULL;
									} // else if
								*/
								} // if
							else if (SetNextDist || CheckNextContained)
								{
								if (SetNextDist)
									SplineDist[3] = SegLength;
								SetNextDist = 0;
								CheckNextContained = 0;
								} // if
							StartSeg[0] = EndSeg[0];
							StartSeg[1] = EndSeg[1];
							PrevPoint = StartPoint;
							StartPoint = StartPoint->Next;
							} // if
						else
							return (0.0);	// projection error, might as well quit now
						} // while
					if (Elev)
						*Elev = SplineElevation(SplinePoint, SplineDist, Slope, Lat, Lon, latscale, lonscale);
					} // if
				else
					{
					EndSeg[0] = StartSeg[0];
					EndSeg[1] = StartSeg[1];
					MinDist = DistPoint2Line(ZeroPoint, StartSeg, EndSeg);
					SplinePoint[1] = StartPoint;
					SplinePoint[2] = SplinePoint[0] = SplinePoint[3] = NULL;
					if (Elev)
						*Elev = SplineElevation(SplinePoint, SplineDist, Slope);
					} // if
				} // if
			else
				return (0.0);	// projection error, might as well quit now
			} // else line style
		} // if
	} // if not DEM

return (MinDist);	// return value is in meters - deal with it!

} // Joe::MinDistToPoint

/*===========================================================================*/

// Version used by textures for align to vector for area effects.
// Connect to origin if an area effect.

double Joe::MinDistToPoint(double Lat, double Lon, double Elev, double MetersPerDegLat, int ConnectToOrigin, 
	double Datum, double Exag, double &DistX, double &DistY, double &DistZ)
{
double NewDist, lonscale, latscale, P1Offset, SplineDist[4], SegLength, SumSegLength, Angle, TempElev;
CoordSys *MyCoords;
VectorPoint *StartPoint, *SplinePoint[4], *PrevPoint, *NextPoint;
Point2d ZeroPoint, StartSeg, EndSeg;
char UsePoint[2], SetNextDist = 0, CheckNextContained = 0, Sign = 1;
VertexDEM CurVert;

ZeroPoint[0] = 0.0;
ZeroPoint[1] = 0.0;
latscale = MetersPerDegLat;
lonscale = latscale * cos(Lat * PiOver180);

if (! TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (StartPoint = GetFirstRealPoint())
		{
		MyCoords = GetCoordSys();
		DistX = FLT_MAX;
		//StartPoint = GetFirstRealPoint();		// replaced by above
		if (GetLineStyle() < 4)
			{
			DistY = 0.0;
			while (StartPoint)
				{
				if (StartPoint->ProjToDefDeg(MyCoords, &CurVert))
					{
					StartSeg[0] = EndSeg[0] = latscale * (CurVert.Lat - Lat);
					StartSeg[1] = EndSeg[1] = lonscale * (CurVert.Lon - Lon);
					NewDist = DistPoint2Line(ZeroPoint, StartSeg, EndSeg);
					if (NewDist < DistX)
						{
						DistX = NewDist;
						DistZ = Elev - CurVert.Elev;
						} // if
					StartPoint = StartPoint->Next;
					} // if
				else
					{
					DistX = DistY = DistZ = 0.0;
					return (0.0);	// projection error, might as well quit now
					} // else
				} // while
			} // if point style
		else
			{
			if (StartPoint->ProjToDefDeg(MyCoords, &CurVert))
				{
				StartSeg[0] = latscale * (CurVert.Lat - Lat);
				StartSeg[1] = lonscale * (CurVert.Lon - Lon);
				PrevPoint = NULL;
				SplineDist[0] = SplineDist[1] = SplineDist[2] = 0.0;
				SumSegLength = 0.0;
				if (NextPoint = StartPoint->Next)
					{
					while (NextPoint)
						{
						if (NextPoint->ProjToDefDeg(MyCoords, &CurVert))
							{
							EndSeg[0] = latscale * (CurVert.Lat - Lat);
							EndSeg[1] = lonscale * (CurVert.Lon - Lon);
			//				P1offset = 0.0;		// not needed since always set by DistPoint2LineContained
							if ((NewDist = DistPoint2LineContained(ZeroPoint, StartSeg, EndSeg, UsePoint, P1Offset, SegLength)) <= DistX)
								{
								if (SetNextDist)
									{
									SplineDist[3] = SegLength;
									SetNextDist = 0;
									} // if
								if (NewDist == DistX)
									{
									if (CheckNextContained && ! LastTrulyContained(SplinePoint, SplineDist, Lat, Lon, latscale, lonscale))
										{
										SplineDist[1] = SplineDist[2];
										SplineDist[2] = SegLength;
										SetNextDist = 1;

										SplineDist[0] = P1Offset;
										SplinePoint[0] = PrevPoint;
										SplinePoint[1] = StartPoint;
										SplinePoint[2] = NextPoint;
										SplinePoint[3] = (SplinePoint[2]->Next || ! ConnectToOrigin) ? SplinePoint[2]->Next: GetFirstRealPoint();
										DistY = SumSegLength + P1Offset * SegLength;
										// find numeric sign of X
										// NextPoint is valid by definition of the while loop
										// but we don't really know if the point fell beyond the end or start of the segment
										// It doesn't really matter, I suppose, since there won't be a right way to treat
										// end of vector texture sampling anyway. This way probably has more consistency
										// than making all end of line X values positive.
										// We need to find the angle made between two vectors: P(Lat,Lon) - StartPoint, NextPoint - StartPoint
										// P(Lat, Lon) = 0,0
										// V1 = -StartPoint
										// V2 = NextPoint - StartPoint
										// if (FindVectorAngle2D(V1, V2) > Pi)
										//		Sign = 1;
										// else
										//		Sign = -1;
										// Get the vector angle for each of the vectors and subtract one from the other
										// Use findangle3 which considers clockwise rotation to be positive.
										Angle = findangle3(-StartSeg[0], -StartSeg[1]);
										Angle -= findangle3(EndSeg[0] - StartSeg[0], EndSeg[1] - StartSeg[1]);
										if (Angle < 0.0)
											Angle += TwoPi;
										Sign = Angle > Pi ? 1: -1;
										} // if
									} // if
								else
									{
									DistX = NewDist;
									SplineDist[1] = SplineDist[2];
									SplineDist[2] = SegLength;
									SetNextDist = 1;

									SplineDist[0] = P1Offset;
									SplinePoint[0] = PrevPoint;
									SplinePoint[1] = StartPoint;
									SplinePoint[2] = NextPoint;
									SplinePoint[3] = (SplinePoint[2]->Next || ! ConnectToOrigin) ? SplinePoint[2]->Next: GetFirstRealPoint();
									DistY = SumSegLength + P1Offset * SegLength;
									// find numeric sign of X
									Angle = findangle3(-StartSeg[0], -StartSeg[1]);
									Angle -= findangle3(EndSeg[0] - StartSeg[0], EndSeg[1] - StartSeg[1]);
									if (Angle < 0.0)
										Angle += TwoPi;
									Sign = Angle > Pi ? 1: -1;
									} // else
								if (! UsePoint[0])
									CheckNextContained = 1;
								else
									CheckNextContained = 0;
								} // if
							else if (SetNextDist || CheckNextContained)
								{
								if (SetNextDist)
									SplineDist[3] = SegLength;
								SetNextDist = 0;
								CheckNextContained = 0;
								} // if
							SumSegLength += SegLength;
							StartSeg[0] = EndSeg[0];
							StartSeg[1] = EndSeg[1];
							PrevPoint = StartPoint;
							if (StartPoint = StartPoint->Next)
								{
								if (! (NextPoint = StartPoint->Next) && ConnectToOrigin)
									NextPoint = GetFirstRealPoint();
								} // if
							else
								break;
							} // if
						else
							{
							DistX = DistY = DistZ = 0.0;
							return (0.0);	// projection error, might as well quit now
							} // else
						} // while
					TempElev = SplineElevation(SplinePoint, SplineDist, NULL, Lat, Lon, latscale, lonscale);
					TempElev = (TempElev - Datum) * Exag + Datum;
					DistZ = Elev - TempElev;
					} // if
				else
					{
					EndSeg[0] = StartSeg[0];
					EndSeg[1] = StartSeg[1];
					DistX = DistPoint2Line(ZeroPoint, StartSeg, EndSeg);
					SplinePoint[1] = StartPoint;
					SplinePoint[2] = SplinePoint[0] = SplinePoint[3] = NULL;
					TempElev = SplineElevation(SplinePoint, SplineDist, NULL);
					TempElev = (TempElev - Datum) * Exag + Datum;
					DistZ = Elev - TempElev;
					DistY = 0.0;
					} // if
				} // if
			else
				{
				DistX = DistY = DistZ = 0.0;
				return (0.0);	// projection error, might as well quit now
				} // else
			} // else line style
		DistX *= Sign;
		} // if enough points
	else
		{
		DistX = DistY = DistZ = 0.0;
		} // else
	} // if not DEM
else
	{
	DistX = DistY = DistZ = 0.0;
	} // else

return (DistX);	// return value is in meters - deal with it!

} // Joe::MinDistToPoint

/*===========================================================================*/

double Joe::SplineLatLonElevation(VectorPoint **Point, double *Distance, Point2d LatLonPt, double *Slope)
{
double P0, P1, P2, P3, ElDist0, ElDist1, El0, El1, D1, D2, S1, S2, S3, h1, h2, h3, h4, Lt0, Lt1, Lt2, Lt3, Ln0, Ln1, Ln2, Ln3;
CoordSys *MyCoords;
VertexDEM CurVert;

/*
							  |<---- Distance[0] % -->|				
|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
*-----------------------------*-----------------------X-----*-----------------------------*
Point[0]					Point[1]		  				Point[2]					Point[3]

*/

MyCoords = GetCoordSys();

if (Point[1]->ProjToDefDeg(MyCoords, &CurVert))
	{
	P1 = CurVert.Elev;
	Lt1 = CurVert.Lat;
	Ln1 = CurVert.Lon;
	} // if
else
	{
	P1 = Point[1]->Elevation;
	Lt1 = Point[1]->Latitude;
	Ln1 = Point[1]->Longitude;
	} // else

if (Point[2] && Distance[2] > 0.0)
	{
	if (Point[2]->ProjToDefDeg(MyCoords, &CurVert))
		{
		P2 = CurVert.Elev;
		Lt2 = CurVert.Lat;
		Ln2 = CurVert.Lon;
		} // if
	else
		{
		P2 = Point[2]->Elevation;
		Lt2 = Point[2]->Latitude;
		Ln2 = Point[2]->Longitude;
		} // else

	if (Point[0])
		{
		if (Point[0]->ProjToDefDeg(MyCoords, &CurVert))
			{
			P0 = CurVert.Elev;
			Lt0 = CurVert.Lat;
			Ln0 = CurVert.Lon;
			} // if
		else
			{
			P0 = Point[0]->Elevation;
			Lt0 = Point[0]->Latitude;
			Ln0 = Point[0]->Longitude;
			} // else
		} // if
	if (Point[3])
		{
		if (Point[3]->ProjToDefDeg(MyCoords, &CurVert))
			{
			P3 = CurVert.Elev;
			Lt3 = CurVert.Lat;
			Ln3 = CurVert.Lon;
			} // if
		else
			{
			P3 = Point[3]->Elevation;
			Lt3 = Point[3]->Latitude;
			Ln3 = Point[3]->Longitude;
			} // else
		} // if

	// latitude
	D1 = Point[0] ?
		((.5 * (Lt1 - Lt0))
		+ (.5 * (Lt2 - Lt1)))
		* (2.0 * Distance[2] / (Distance[1] + Distance[2])):
		(Lt2 - Lt1);

	D2 = Point[3] ?
		((.5 * (Lt2 - Lt1))
		+ (.5 * (Lt3 - Lt2)))
		* (2.0 * Distance[2] / (Distance[2] + Distance[3])):
		(Lt2 - Lt1);

	S1 = Distance[0];
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	LatLonPt[1] = (Lt1 * h1 + Lt2 * h2 + D1 * h3 + D2 * h4);

	// longitude
	D1 = Point[0] ?
		((.5 * (Ln1 - Ln0))
		+ (.5 * (Ln2 - Ln1)))
		* (2.0 * Distance[2] / (Distance[1] + Distance[2])):
		(Ln2 - Ln1);

	D2 = Point[3] ?
		((.5 * (Ln2 - Ln1))
		+ (.5 * (Ln3 - Ln2)))
		* (2.0 * Distance[2] / (Distance[2] + Distance[3])):
		(Ln2 - Ln1);

	S1 = Distance[0];
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	LatLonPt[0] = (Ln1 * h1 + Ln2 * h2 + D1 * h3 + D2 * h4);

	// slope & elevation
	D1 = Point[0] ?
		((.5 * (P1 - P0))
		+ (.5 * (P2 - P1)))
		* (2.0 * Distance[2] / (Distance[1] + Distance[2])):
		(P2 - P1);

	D2 = Point[3] ?
		((.5 * (P2 - P1))
		+ (.5 * (P3 - P2)))
		* (2.0 * Distance[2] / (Distance[2] + Distance[3])):
		(P2 - P1);

	if (Slope)
		{
		ElDist0 = Distance[0] > .001 ? Distance[0] - .001: 0.0;
		S1 = ElDist0;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El0 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		ElDist1 = Distance[0] < .999 ? Distance[0] + .001: 1.0;
		S1 = ElDist1;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El1 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		*Slope = (El1 - El0) / ((ElDist1 - ElDist0) * Distance[2]);
		} // if
	S1 = Distance[0];
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if

return (P1);

} // Joe::SplineLatLonElevation

/*===========================================================================*/

double Joe::SplineElevation(VectorPoint **Point, double *Distance, double *Slope)
{
double P0, P1, P2, P3, ElDist0, ElDist1, El0, El1, D1, D2, S1, S2, S3, h1, h2, h3, h4;
CoordSys *MyCoords;
VertexDEM CurVert;

/*
							  |<---- Distance[0] % -->|				
|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
*-----------------------------*-----------------------X-----*-----------------------------*
Point[0]					Point[1]		  				Point[2]					Point[3]

*/

MyCoords = GetCoordSys();

if (Point[1]->ProjToDefDeg(MyCoords, &CurVert))
	P1 = CurVert.Elev;
else
	P1 = Point[1]->Elevation;

if (Point[2] && Distance[2] > 0.0)
	{
	if (Point[2]->ProjToDefDeg(MyCoords, &CurVert))
		P2 = CurVert.Elev;
	else
		P2 = Point[2]->Elevation;

	if (Point[0])
		{
		if (Point[0]->ProjToDefDeg(MyCoords, &CurVert))
			P0 = CurVert.Elev;
		else
			P0 = Point[0]->Elevation;
		} // if
	if (Point[3])
		{
		if (Point[3]->ProjToDefDeg(MyCoords, &CurVert))
			P3 = CurVert.Elev;
		else
			P3 = Point[3]->Elevation;
		} // if
	D1 = Point[0] ?
		((.5 * (P1 - P0))
		+ (.5 * (P2 - P1)))
		* (2.0 * Distance[2] / (Distance[1] + Distance[2])):
		(P2 - P1);

	D2 = Point[3] ?
		((.5 * (P2 - P1))
		+ (.5 * (P3 - P2)))
		* (2.0 * Distance[2] / (Distance[2] + Distance[3])):
		(P2 - P1);

	if (Slope)
		{
		ElDist0 = Distance[0] > .001 ? Distance[0] - .001: 0.0;
		S1 = ElDist0;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El0 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		ElDist1 = Distance[0] < .999 ? Distance[0] + .001: 1.0;
		S1 = ElDist1;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El1 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		*Slope = (El1 - El0) / ((ElDist1 - ElDist0) * Distance[2]);
		} // if
	S1 = Distance[0];
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	// slope calculation replaced by above 5/22/02 by GRH
	//if (Slope)
	//	*Slope = 6.0 * ((P1 * (S2 - S1)) + (P2 * (-S2 + S1))) + (D1 * (3.0 * S2 - 4.0 * S1 + 1.0)) + (D2 * (3.0 * S2 - 2.0 * S1));
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if

return (P1);

} // Joe::SplineElevation

/*===========================================================================*/

// This is a more robust but also more energy intensive version that handles points on
// the concave side of a vector without discontinuities along the bisectrix between
// adjacent segments. It appears to develop elevation discontinuities at vertices between two parallel line segments.

double Joe::SplineElevation(VectorPoint **Point, double *Distance, double *Slope, double Lat, double Lon, double latscale, double lonscale)
{
double P0, P1, P2, P3, ElDist0, ElDist1, El0, El1, D1, D2, S1, S2, S3, h1, h2, h3, h4, M[3], B[3];
signed char Projected[2], V[3];
Point2d CartPt[10];
CoordSys *MyCoords;
VertexDEM CurVert;

/*
							  |<---- Distance[0] % -->|				
|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
*-----------------------------*-----------------------X-----*-----------------------------*
Point[0]					Point[1]		  				Point[2]					Point[3]

*/

// the whole difference between this function and the one above is in the recalculation of Distance[0]
// when there are adjacent line segments with non-zero length.

MyCoords = GetCoordSys();

if (Point[1]->ProjToDefDeg(MyCoords, &CurVert))
	P1 = CurVert.Elev;
else
	return (Point[1]->Elevation);

if (Point[2] && Distance[2] > 0.0)
	{
	CartPt[1][0] = latscale * (CurVert.Lat - Lat);
	CartPt[1][1] = lonscale * (CurVert.Lon - Lon);
	if (Point[2]->ProjToDefDeg(MyCoords, &CurVert))
		P2 = CurVert.Elev;
	else
		return (Point[1]->Elevation);
	Projected[0] = Projected[1] = 0;
	CartPt[2][0] = latscale * (CurVert.Lat - Lat);
	CartPt[2][1] = lonscale * (CurVert.Lon - Lon);
	V[1] = V[2] = 0;
	if (fabs(CartPt[2][0] - CartPt[1][0]) > .0000001)
		{
		M[0] = (CartPt[2][1] - CartPt[1][1]) / (CartPt[2][0] - CartPt[1][0]);
		V[0] = 0;
		} // if
	else
		{
		M[0] = (CartPt[2][0] - CartPt[1][0]) / (CartPt[2][1] - CartPt[1][1]);
		V[0] = 1;
		} // else
//	B[0] = 0.0; who cares?
	if (Point[0])
		{
		if (Point[0]->ProjToDefDeg(MyCoords, &CurVert))
			P0 = CurVert.Elev;
		else
			return (Point[1]->Elevation);
		} // if
	if (Point[0] && Distance[1] > 0.0)
		{
		CartPt[0][0] = latscale * (CurVert.Lat - Lat);
		CartPt[0][1] = lonscale * (CurVert.Lon - Lon);
		CartPt[4][0] = CartPt[1][0] + Distance[2] * (CartPt[0][0] - CartPt[1][0]) / Distance[1];
		CartPt[4][1] = CartPt[1][1] + Distance[2] * (CartPt[0][1] - CartPt[1][1]) / Distance[1];
		CartPt[5][0] = (CartPt[2][0] + CartPt[4][0]) * 0.5;  // Optimized out division. Was / 2.0
		CartPt[5][1] = (CartPt[2][1] + CartPt[4][1]) * 0.5;  // Optimized out division. Was / 2.0
		if (fabs(CartPt[1][0] - CartPt[5][0]) > .0000001)
			{	// y = mx + b
			M[1] = (CartPt[1][1] - CartPt[5][1]) / (CartPt[1][0] - CartPt[5][0]);
			if (V[0] || M[0] != M[1])
				{
				Projected[0] = 1;
				B[1] = CartPt[1][1] - M[1] * CartPt[1][0];
				if (V[0])
					{
					CartPt[6][0] = 0.0;
					} // if
				else
					{
					CartPt[6][0] = B[1] / (M[0] - M[1]);
					} // else
				CartPt[6][1] = CartPt[6][0] * M[1] + B[1];
				} // if
			} // if
		else 
			{	// x = my + b
			if (! V[0])
				{
				Projected[0] = 1;
				CartPt[6][0] = CartPt[1][0];
				CartPt[6][1] = CartPt[6][0] * M[0];
				} // if
			} // else if
		} // if
	if (Point[3])
		{
		if (Point[3]->ProjToDefDeg(MyCoords, &CurVert))
			P3 = CurVert.Elev;
		else
			return (Point[1]->Elevation);
		} // if
	if (Point[3] && Distance[3] > 0.0)
		{
		CartPt[3][0] = latscale * (CurVert.Lat - Lat);
		CartPt[3][1] = lonscale * (CurVert.Lon - Lon);
		CartPt[7][0] = CartPt[2][0] + Distance[2] * (CartPt[3][0] - CartPt[2][0]) / Distance[3];
		CartPt[7][1] = CartPt[2][1] + Distance[2] * (CartPt[3][1] - CartPt[2][1]) / Distance[3];
		CartPt[8][0] = (CartPt[1][0] + CartPt[7][0]) * 0.5;  // Optimized out division. Was / 2.0
		CartPt[8][1] = (CartPt[1][1] + CartPt[7][1]) * 0.5;  // Optimized out division. Was / 2.0
		if (fabs(CartPt[2][0] - CartPt[8][0]) > .0000001)
			{
			M[2] = (CartPt[2][1] - CartPt[8][1]) / (CartPt[2][0] - CartPt[8][0]);
			if (V[0] || M[0] != M[2])
				{
				Projected[1] = 1;
				B[2] = CartPt[2][1] - M[2] * CartPt[2][0];
				if (V[0])
					{
					CartPt[9][0] = 0.0;
					} // if
				else
					{
					CartPt[9][0] = B[2] / (M[0] - M[2]);
					} // else
				CartPt[9][1] = CartPt[9][0] * M[2] + B[2];
				} // if
			} // if
		else
			{
			if (! V[0])
				{
				Projected[1] = 1;
				CartPt[9][0] = CartPt[2][0];
				CartPt[9][1] = CartPt[9][0] * M[0];
				} // if
			} // else
		} // if
	if (Projected[0])
		{
		if (! Projected[1])
			{
			if (! V[0])
				{
				if (M[0] != 0.0)
					{
					M[2] = - 1.0 / M[0];
					} // if
				else
					{
					V[2] = 1;	// changed 2/3/98
					} // else
				} // if
			else
				{
				M[2] = 0.0;
				} // else
			if (! V[2])
				{
				B[2] = CartPt[2][1] - M[2] * CartPt[2][0];
				if (V[0])
					{
					CartPt[9][0] = 0.0;
					} // if
				else
					{
					CartPt[9][0] = B[2] / (M[0] - M[2]);
					} // else
				CartPt[9][1] = CartPt[9][0] * M[2] + B[2];
				} // if
			else
				{
				CartPt[9][0] = CartPt[2][0];
				CartPt[9][1] = 0.0;
				} // else
			} // if
		if (fabs(CartPt[6][0] - CartPt[9][0]) > .0000001)
			Distance[0] = (- CartPt[6][0]) / (CartPt[9][0] - CartPt[6][0]);
		else if (fabs(CartPt[6][1] - CartPt[9][1]) > .0000001)
			Distance[0] = (- CartPt[6][1]) / (CartPt[9][1] - CartPt[6][1]);
		else
			Distance[0] = 0.0;
		if (Distance[0] < 0.0)
			Distance[0] = 0.0;
		else if (Distance[0] > 1.0)
			Distance[0] = 1.0;
		} // if
	else if (Projected[1])
		{
		if (! V[0])
			{
			if (M[0] != 0.0)
				{
				M[1] = - 1.0 / M[0];
				} // if
			else
				{
				V[1] = 1;	// changed 2/3/98
				} // else
			} // if
		else
			{
			M[1] = 0.0;
			} // else
		if (! V[1])
			{
			B[1] = CartPt[1][1] - M[1] * CartPt[1][0];
			if (V[0])
				{
				CartPt[6][0] = 0.0;
				} // if
			else
				{
				CartPt[6][0] = B[1] / (M[0] - M[1]);
				} // else
			CartPt[6][1] = CartPt[6][0] * M[1] + B[1];
			} // if
		else
			{
			CartPt[6][0] = CartPt[1][0];
			CartPt[6][1] = 0.0;
			} // else
		if (fabs(CartPt[6][0] - CartPt[9][0]) > .0000001)
			Distance[0] = (- CartPt[6][0]) / (CartPt[9][0] - CartPt[6][0]);
		else if (fabs(CartPt[6][1] - CartPt[9][1]) > .0000001)
			Distance[0] = (- CartPt[6][1]) / (CartPt[9][1] - CartPt[6][1]);
		else
			Distance[0] = 0.0;
		if (Distance[0] < 0.0)
			Distance[0] = 0.0;
		else if (Distance[0] > 1.0)
			Distance[0] = 1.0;
		} // else if

	D1 = Point[0] ?
		((.5 * (P1 - P0))
		+ (.5 * (P2 - P1)))
		* (2.0 * Distance[2] / (Distance[1] + Distance[2])):
		(P2 - P1);

	D2 = Point[3] ?
		((.5 * (P2 - P1))
		+ (.5 * (P3 - P2)))
		* (2.0 * Distance[2] / (Distance[2] + Distance[3])):
		(P2 - P1);

	if (Slope)
		{
		ElDist0 = Distance[0] > .001 ? Distance[0] - .001: 0.0;
		S1 = ElDist0;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El0 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		ElDist1 = Distance[0] < .999 ? Distance[0] + .001: 1.0;
		S1 = ElDist1;
		S2 = S1 * S1;
		S3 = S1 * S2;
		h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
		h2 = -2.0 * S3 + 3.0 * S2;
		h3 = S3 - 2.0 * S2 + S1;
		h4 = S3 - S2;
		El1 = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

		*Slope = (El1 - El0) / ((ElDist1 - ElDist0) * Distance[2]);
		} // if
	S1 = Distance[0];
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	// slope calculation replaced by above 5/22/02 by GRH
	//if (Slope)
	//	*Slope = 6.0 * ((P1 * (S2 - S1)) + (P2 * (-S2 + S1))) + (D1 * (3.0 * S2 - 4.0 * S1 + 1.0)) + (D2 * (3.0 * S2 - 2.0 * S1));
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);

	} // if

return (P1);

} // Joe::SplineElevation

/*===========================================================================*/
/* useful for testing - no splining involved
double Joe::SplineElevation(VectorPoint **Point, double *Distance, double *Slope, double Lat, double Lon, double latscale, double lonscale)
{
double P0, P1, P2, P3, D1, D2, S1, S2, S3, h1, h2, h3, h4, M[3], B[3];
signed char Projected[2], V[3];
Point2d CartPt[10];
CoordSys *MyCoords;
VertexDEM CurVert;

MyCoords = GetCoordSys();

if (Point[1]->ProjToDefDeg(MyCoords, &CurVert))
	P1 = CurVert.Elev;
else
	return (Point[1]->Elevation);

if (Point[2] && Distance[2] > 0.0)
	{
	if (Point[2]->ProjToDefDeg(MyCoords, &CurVert))
		P2 = CurVert.Elev;
	else
		return (Point[1]->Elevation);
	*Slope = (P2 - P1) / Distance[2];
	if (Distance[0] <= 0.0)
		return (P1);
	if (Distance[0] >= 1.0)
		return (P2);
	return (P1 + *Slope * Distance[0] * Distance[2]);
	} // if

return (P1);

} // Joe::SplineElevation
*/
/*===========================================================================*/

// This is used to determine if, on convex sides of vectors, the point is not simply contained
// by the line segment
int Joe::LastTrulyContained(VectorPoint **Point, double *Distance, double Lat, double Lon, double latscale, double lonscale)
{
double M[3], B[3];
signed char Projected, V[3];
Point2d CartPt[10];
CoordSys *MyCoords;
VertexDEM CurVert;

/*
							  |<---- Distance[0] ---->|				
|<-----	Distance[1]	--------->|<------- Distance[2] ------->|<------- Distance[3] ------->|
*-----------------------------*-----------------------X-----*-----------------------------*
Point[0]					Point[1]		  				Point[2]					Point[3]

*/

MyCoords = GetCoordSys();

if (Point[2] && Distance[2] > 0.0)
	{
	Projected = 0;
	if (! Point[1]->ProjToDefDeg(MyCoords, &CurVert))
		return (0);
	CartPt[1][0] = latscale * (CurVert.Lat - Lat);
	CartPt[1][1] = lonscale * (CurVert.Lon - Lon);
	if (! Point[2]->ProjToDefDeg(MyCoords, &CurVert))
		return (0);
	CartPt[2][0] = latscale * (CurVert.Lat - Lat);
	CartPt[2][1] = lonscale * (CurVert.Lon - Lon);
	V[1] = V[2] = 0;
	if (CartPt[2][0] != CartPt[1][0])
		{
		M[0] = (CartPt[2][1] - CartPt[1][1]) / (CartPt[2][0] - CartPt[1][0]);
		V[0] = 0;
		} // if
	else
		{
		M[0] = (CartPt[2][0] - CartPt[1][0]) / (CartPt[2][1] - CartPt[1][1]);
		V[0] = 1;
		} // else
	if (Point[3] && Distance[3] > 0.0)
		{
		if (! Point[3]->ProjToDefDeg(MyCoords, &CurVert))
			return (0);
		CartPt[3][0] = latscale * (CurVert.Lat - Lat);
		CartPt[3][1] = lonscale * (CurVert.Lon - Lon);
		CartPt[7][0] = CartPt[2][0] + Distance[2] * (CartPt[3][0] - CartPt[2][0]) / Distance[3];
		CartPt[7][1] = CartPt[2][1] + Distance[2] * (CartPt[3][1] - CartPt[2][1]) / Distance[3];
		CartPt[8][0] = (CartPt[1][0] + CartPt[7][0]) * 0.5;  // Optimized out division. Was / 2.0
		CartPt[8][1] = (CartPt[1][1] + CartPt[7][1]) * 0.5;  // Optimized out division. Was / 2.0
		if (CartPt[2][0] != CartPt[8][0])
			{
			M[2] = (CartPt[2][1] - CartPt[8][1]) / (CartPt[2][0] - CartPt[8][0]);
			if (V[0] || M[0] != M[2])
				{
				Projected = 1;
				B[2] = CartPt[2][1] - M[2] * CartPt[2][0];
				if (V[0])
					{
					CartPt[9][0] = 0.0;
					} // if
				else
					{
					CartPt[9][0] = B[2] / (M[0] - M[2]);
					} // else
				CartPt[9][1] = CartPt[9][0] * M[2] + B[2];
				} // if
			} // if
		else
			{
			if (! V[0])
				{
				Projected = 1;
				CartPt[9][0] = CartPt[2][0];
				CartPt[9][1] = CartPt[9][0] * M[0];
				} // if
			} // else
		} // if
	if (Projected)
		{
		if (! V[0])
			{
			if (M[0] != 0.0)
				{
				M[1] = - 1.0 / M[0];
				} // if
			else
				{
				V[1] = 1;
				} // else
			} // if
		else
			{
			M[1] = 0.0;
			} // else
		if (! V[1])
			{
			B[1] = CartPt[1][1] - M[1] * CartPt[1][0];
			if (V[0])
				{
				CartPt[6][0] = 0.0;
				} // if
			else
				{
				CartPt[6][0] = B[1] / (M[0] - M[1]);
				} // else
			CartPt[6][1] = CartPt[6][0] * M[1] + B[1];
			} // if
		else
			{
			CartPt[6][0] = CartPt[1][0];
			CartPt[6][1] = 0.0;
			} // else
		if (CartPt[6][0] != CartPt[9][0])
			Distance[0] = (- CartPt[6][0]) / (CartPt[9][0] - CartPt[6][0]);
		else if (CartPt[6][1] != CartPt[9][1])
			Distance[0] = (- CartPt[6][1]) / (CartPt[9][1] - CartPt[6][1]);
		if (Distance[0] > 1.0)
			return (0);
		} // if
	} // if

return (1);

} // Joe::LastTrulyContained

/*===========================================================================*/

short Joe::SimpleContained(double Lat, double Lon)
{
short Contained = 0;
VectorPoint *Pt, *NextPt;
VertexDEM MyVert;

if (Pt = GetFirstRealPoint())
	{
	if ((SELat <= Lat) && (NWLat >= Lat) && (NWLon >= Lon) && (SELon <= Lon))
		{
		MyVert.Lat = Lat;
		MyVert.Lon = Lon;
		if (DefDegToProj(&MyVert)) 
			{
			for (; Pt; Pt = Pt->Next)
				{
				NextPt = Pt->Next;
				if (! NextPt)
					NextPt = GetFirstRealPoint();
				Contained += LineSegsIntersect(MyVert.xyz[1], MyVert.xyz[0], Pt, NextPt);
				} // for
			} // if
		} // if
	} // if

return (Contained % 2);

} // Joe::SimpleContained

/*===========================================================================*/

int Joe::IsJoeContainedInMyGeoBounds(Joe *AmIContained)
{

return ((SELat <= AmIContained->SELat) && (NWLat >= AmIContained->NWLat) && (NWLon >= AmIContained->NWLon) && (SELon <= AmIContained->SELon));

} // Joe::IsJoeContainedInMyGeoBounds

/*===========================================================================*/

// returns -1 for not at all contained, 0 for partially contained and 1 for totally contained
int Joe::CompareGeoBounds(double NorthBnds, double SouthBnds, double WestBnds, double EastBnds)
{

if (SELat >= NorthBnds || NWLat <= SouthBnds || NWLon <= EastBnds || SELon >= WestBnds)
	return (-1);	// completely uncontained

// at least partially contained

if (SouthBnds <= SELat && NorthBnds >= NWLat && EastBnds <= SELon && WestBnds >= NWLon)
	return (1);		// completely contained

return (0);			// partially contained

} // Joe::CompareGeoBounds

/*===========================================================================*/

VectorPoint *Joe::CopyPoints(Joe *CopyTo, Joe *CopyFrom, short DoNotification, short DoBoundsCheck)
{
VectorPoint *PLinkTo, *PLinkFrom;
unsigned long Point;
NotifyTag Changes[2];

if (DoNotification)
	GlobalApp->AppDB->GenerateNotify(DBPreChangeEventPOINTS, this);

if (CopyTo->Points())
	{
	GlobalApp->AppDB->MasterPoint.DeAllocate(CopyTo->Points());
	CopyTo->Points(NULL);
	CopyTo->NumPoints(0);
	} // if

if (CopyFrom->Points() && CopyFrom->NumPoints() > 0)
	{
	if (CopyTo->Points(GlobalApp->AppDB->MasterPoint.Allocate(CopyFrom->NumPoints())))
		{
		CopyTo->NumPoints(CopyFrom->NumPoints());
		PLinkTo = CopyTo->Points();
		PLinkFrom = CopyFrom->Points();
		for (Point = 0; Point < CopyTo->NumPoints() && PLinkTo && PLinkFrom; Point++)
			{
			PLinkTo->Latitude = PLinkFrom->Latitude;
			PLinkTo->Longitude = PLinkFrom->Longitude;
			PLinkTo->Elevation = PLinkFrom->Elevation;
			PLinkTo = PLinkTo->Next;
			PLinkFrom = PLinkFrom->Next;
			} // for

		CopyTo->NWLat = CopyFrom->NWLat;
		CopyTo->NWLon = CopyFrom->NWLon;
		CopyTo->SELat = CopyFrom->SELat;
		CopyTo->SELon = CopyFrom->SELon;
		} // if
	} // if

if (DoBoundsCheck)
	{
	ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
	RecheckBounds();
	ZeroUpTree();
	GlobalApp->AppDB->ReBoundTree(WCS_DATABASE_STATIC);
	} // if
if (DoNotification)
	{
	GlobalApp->AppDB->GenerateNotify(DBChangeEventPOINTS, this);
	Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes);
	} // if

return (CopyTo->Points());

} // CopyPoints

/*===========================================================================*/

void Joe::ConvertPointDatums(CoordSys *ConvertFrom, CoordSys *ConvertTo)
{
VectorPoint *PLink;
VertexDEM VDEM;

for (PLink = Points(); PLink; PLink = PLink->Next)
	{
	VDEM.Lon = PLink->Longitude;
	VDEM.Lat = PLink->Latitude;
	VDEM.Elev = PLink->Elevation;
	ConvertFrom->DegToCart(&VDEM);
	ConvertTo->CartToDeg(&VDEM);
	PLink->Longitude = VDEM.Lon;
	PLink->Latitude = VDEM.Lat;
	} // for

} // Joe::ConvertPointDatums

/*===========================================================================*/

bool Joe::ConvertToDefGeo(CoordSys *MyCoords)
{
bool Success = true;
#ifdef WCS_BUILD_VNS
VectorPoint *CurPt;
VertexDEM VDEM;

if (MyCoords)
	{
	for (CurPt = Points(); CurPt; CurPt = CurPt->Next)
		{
		// translate projected coords to global cartesian and then to default degrees
		VDEM.xyz[0] = CurPt->Longitude;
		VDEM.xyz[1] = CurPt->Latitude;
		VDEM.xyz[2] = CurPt->Elevation;
		if (MyCoords->ProjToDefDeg(&VDEM))
			{
			CurPt->Longitude = VDEM.Lon;
			CurPt->Latitude = VDEM.Lat;
			CurPt->Elevation = (float)VDEM.Elev;
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // for
	} // if
#endif // WCS_BUILD_VNS

return (Success);

} // Joe::ConvertToDefGeo

/*===========================================================================*/

double Joe::AverageElevation(void)
{
double SumEl = 0.0;
long Pt = 0;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM CurVert;

if (Points())
	{
	MyCoords = GetCoordSys();
	for (PLink = GetFirstRealPoint(); PLink; PLink = PLink->Next, Pt ++)
		{
		if (PLink->ProjToDefDeg(MyCoords, &CurVert))
			SumEl += CurVert.Elev;
		else
			return (0.0);
		} // for
	if (Pt > 0)
		return (SumEl / Pt);
	} // if

return (0.0);

} // Joe::AverageElevation

/*===========================================================================*/

// returns length in kilometers

double Joe::GetVecLength(int ConsiderElev)
{
VectorPoint *PLinkA, *PLinkB;
CoordSys *MyCoords;
VertexDEM MyVert;
double Sum = 0.0, HDist, VDist, Radius, LastPtLat, LastPtLon, LastPtElev;

Radius = GlobalApp->AppEffects->GetPlanetRadius();
if (! TestFlags(WCS_JOEFLAG_ISDEM) && GetSecondRealPoint())
	{
	MyCoords = GetCoordSys();

	PLinkA = GetFirstRealPoint();
	if (PLinkA->ProjToDefDeg(MyCoords, &MyVert))
		{
		for (PLinkB = PLinkA->Next; PLinkB; PLinkB = PLinkB->Next)
			{
			LastPtLat = MyVert.Lat;
			LastPtLon = MyVert.Lon;
			LastPtElev = MyVert.Elev;
			if (PLinkB->ProjToDefDeg(MyCoords, &MyVert))
				{
				HDist = FindDistance(LastPtLat, LastPtLon, MyVert.Lat, MyVert.Lon, Radius);
				if (ConsiderElev)
					{
					VDist = MyVert.Elev - LastPtElev;
					HDist = sqrt(HDist * HDist + VDist * VDist);
					} // if
				Sum += HDist;
				} // if
			else
				break;
			PLinkA = PLinkB;
			} // for
		} // if
	} // if

return (Sum);

} // Joe::GetVecLength

/*===========================================================================*/

float Joe::GetElevRange(float &MaxEl, float &MinEl)
{
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM MyVert;
int NotDone = 0;

MaxEl = -10000000.0f;
MinEl = 10000000.0f;

if (PLink = GetFirstRealPoint())
	{
	// see if there is a CoordSys attribute
	MyCoords = GetCoordSys();

	do
		{
		if (PLink->ProjToDefDeg(MyCoords, &MyVert))
			{
			if ((float)MyVert.Elev > MaxEl)
				MaxEl = (float)MyVert.Elev;
			if ((float)MyVert.Elev < MinEl)
				MinEl = (float)MyVert.Elev;
			} // if
		else
			{
			NotDone = 1;
			break;
			}
		PLink = PLink->Next;
		} while (PLink);
	} // if
else
	NotDone = 1;

if (NotDone)
	{
	MaxEl = 0.0f;
	MinEl = 0.0f;
	} // else

return (MaxEl - MinEl);

} // Joe::GetElevRange

/*===========================================================================*/

float Joe::GetMinElev(void)
{
float MaxEl;

if (JRendData || (JRendData = new JoeRenderData()))
	{
	if (JRendData->MinEl == FLT_MAX)
		{
		GetElevRange(MaxEl, JRendData->MinEl);
		} // if
	return (JRendData->MinEl);
	} // if

return (0.0f);

} // Joe::GetMinElev

/*===========================================================================*/

void Joe::GetTrueBounds(double &NWLatitude, double &NWLongitude, double &SELatitude, double &SELongitude)
{

// function used to walk the point list but this is not 
// necessary since bounds are now accurate to double precision
NWLatitude = NWLat;
NWLongitude = NWLon;
SELatitude = SELat;
SELongitude = SELon;

} // Joe::GetTrueBounds

/*===========================================================================*/

void Joe::RecheckBounds(void)
{
double LocalNorth, LocalSouth, LocalWest, LocalEast;
VectorPoint *PLink;
JoeDEM *MyDEM;
CoordSys *MyCoords;
VertexDEM MyVert;
int NotDone = 0;
short LocalElMax, LocalElMin;

if (! TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	// see if there is a CoordSys attribute
	MyCoords = GetCoordSys();

	if (! TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (PLink = GetFirstRealPoint())
			{
			NWLat = -100000.0;
			SELat = 100000.0;
			NWLon = -100000.0;
			SELon = 100000.0;

			do
				{
				#ifdef WCS_BUILD_VNS
				if (PLink->ProjToDefDeg(MyCoords, &MyVert))
					{
					if (MyVert.Lat > NWLat)
						NWLat = MyVert.Lat;
					if (MyVert.Lat < SELat)
						SELat = MyVert.Lat;
					if (MyVert.Lon > NWLon)
						NWLon = MyVert.Lon;
					if (MyVert.Lon < SELon)
						SELon = MyVert.Lon;
					} // if
				else
					{
					NotDone = 1;
					break;
					}
				#else // WCS_BUILD_VNS
				if (PLink->Latitude > NWLat)
					NWLat = PLink->Latitude;
				if (PLink->Latitude < SELat)
					SELat = PLink->Latitude;
				if (PLink->Longitude > NWLon)
					NWLon = PLink->Longitude;
				if (PLink->Longitude < SELon)
					SELon = PLink->Longitude;
				#endif // WCS_BUILD_VNS
				PLink = PLink->Next;
				} while (PLink);
			} // if
		else
			NotDone = 1;

		if (NotDone)
			{
			NWLat = -DBL_MAX; NWLon = -DBL_MAX;	// same values as set in constructor
			SELat = DBL_MAX; SELon = DBL_MAX;
			} // else
		} // if not DEM
	else
		{
		DEM Topo;

		if (Topo.AttemptLoadDEM(this, 1, GlobalApp->MainProj))
			{
			LocalNorth = Topo.Northest();
			LocalSouth = Topo.Southest();
			LocalWest = Topo.Westest();
			LocalEast = Topo.Eastest();
			LocalElMax = (short)Topo.MaxEl();
			LocalElMin = (short)Topo.MinEl();
			} // if
		else
			{
			LocalNorth = NWLat;
			LocalSouth = SELat;
			LocalWest = NWLon;
			LocalEast = SELon;
			if (MyDEM = (JoeDEM *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				LocalElMax = MyDEM->MaxEl;
				LocalElMin = MyDEM->MinEl;
				} // if
			else
				LocalElMax = LocalElMin = 0;
			} // else

		#ifdef WCS_BUILD_VNS
		int VertCt;

		if (MyDEM = (JoeDEM *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
			{
			if (MyCoords)
				{
				double MidWestEast = (LocalWest + LocalEast) * 0.5;
				double sign;
				int SimpleCheck = TRUE;
				NWLat = -100000.0;
				SELat = 100000.0;
				NWLon = -100000.0;
				SELon = 100000.0;
				// see if we're geographic and around +/- 180 degrees
				if ((MyCoords->Method.GCTPMethod == 0) &&
					(LocalWest > 179.0 || LocalWest < -179.0 || LocalEast > 179.0 || LocalEast < - 179.0 || MidWestEast > 179.0 || MidWestEast < - 179.0))
					SimpleCheck = FALSE;
				MyVert.xyz[2] = (LocalElMax + LocalElMin) * .5;
				for (VertCt = 0; VertCt < 9; VertCt ++)
					{
					switch (VertCt)
						{
						case 0:
							{
							MyVert.xyz[0] = LocalWest;
							MyVert.xyz[1] = LocalNorth;
							break;
							} // NW
						case 1:
							{
							MyVert.xyz[0] = LocalEast;
							MyVert.xyz[1] = LocalNorth;
							break;
							} // NE
						case 2:
							{
							MyVert.xyz[0] = LocalEast;
							MyVert.xyz[1] = LocalSouth;
							break;
							} // SE
						case 3:
							{
							MyVert.xyz[0] = LocalWest;
							MyVert.xyz[1] = LocalSouth;
							break;
							} // SW
						case 4:
							{
							MyVert.xyz[0] = MidWestEast;
							MyVert.xyz[1] = (LocalNorth + LocalSouth) * .5;
							break;
							} // Center
						case 5:
							{
							MyVert.xyz[0] = MidWestEast;
							MyVert.xyz[1] = LocalNorth;
							break;
							} // N Center
						case 6:
							{
							MyVert.xyz[0] = MidWestEast;
							MyVert.xyz[1] = LocalSouth;
							break;
							} // S Center
						case 7:
							{
							MyVert.xyz[0] = MidWestEast;
							MyVert.xyz[1] = (LocalNorth + LocalSouth) * .5;
							MyVert.xyz[2] = Topo.MaxEl();
							break;
							} // Center maxel
						case 8:
							{
							MyVert.xyz[0] = MidWestEast;
							MyVert.xyz[1] = (LocalNorth + LocalSouth) * .5;
							MyVert.xyz[2] = Topo.MinEl();
							break;
							} // Center minel
						} // switch
					if (!SimpleCheck)
						sign = Signum(MyVert.xyz[0]);
					if (MyCoords->ProjToDefDeg(&MyVert))
						{
						if (SimpleCheck)
							{
							if (MyVert.Lat > NWLat)
								NWLat = MyVert.Lat;
							if (MyVert.Lat < SELat)
								SELat = MyVert.Lat;
							if (MyVert.Lon > NWLon)
								NWLon = MyVert.Lon;
							if (MyVert.Lon < SELon)
								SELon = MyVert.Lon;
							} // if
						else
							{
							if (MyVert.Lat > NWLat)
								NWLat = MyVert.Lat;
							if (MyVert.Lat < SELat)
								SELat = MyVert.Lat;
							// see if we changed sign by crossing 180
							if (Signum(MyVert.Lon) != sign)
								MyVert.Lon = MyVert.Lon + 360.0 * sign;
							if (MyVert.Lon > NWLon)
								NWLon = MyVert.Lon;
							if (MyVert.Lon < SELon)
								SELon = MyVert.Lon;
							} // else
						} // if
					else
						{
						break;
						}
					} // for
				} // if coord sys
			else
				{
				NWLat = LocalNorth;
				SELat = LocalSouth;
				NWLon = LocalWest;
				SELon = LocalEast;
				} // else default coord sys
			MyDEM->MaxEl = LocalElMax;
			MyDEM->MinEl = LocalElMin;
			} // if DEM attribute
		#else // WCS_BUILD_VNS
		NWLat = LocalNorth;
		SELat = LocalSouth;
		NWLon = LocalWest;
		SELon = LocalEast;
		#endif // WCS_BUILD_VNS
		} // else DEM
	} // if HASKIDS

} // Joe::RecheckBounds

/*===========================================================================*/

int Joe::GetBoundsProjected(CoordSys *Projection, double &NNorthing, double &WEasting, double &SNorthing, double &EEasting, int GISConvention)
{
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM MyVert;
int Geographic = 1, NotDone = 0;

#ifdef WCS_BUILD_VNS
Geographic = ! (Projection && ! Projection->GetGeographic());
#endif // WCS_BUILD_VNS

if (! TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	// see if there is a CoordSys attribute
	MyCoords = GetCoordSys();

	if (! TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (PLink = GetFirstRealPoint())
			{
			if (Geographic)
				{
				NNorthing = -100000.0;
				SNorthing = 100000.0;
				WEasting = -100000.0;
				EEasting = 100000.0;
				} // if
			else
				{
				NNorthing = -FLT_MAX;
				SNorthing = FLT_MAX;
				WEasting = FLT_MAX;
				EEasting = -FLT_MAX;
				} // else

			do
				{
				#ifdef WCS_BUILD_VNS
				if (PLink->ProjToDefDeg(MyCoords, &MyVert))
					{
					if (! Projection)
						{
						if (MyVert.Lat > NNorthing)
							NNorthing = MyVert.Lat;
						if (MyVert.Lat < SNorthing)
							SNorthing = MyVert.Lat;
						if (MyVert.Lon > WEasting)
							WEasting = MyVert.Lon;
						if (MyVert.Lon < EEasting)
							EEasting = MyVert.Lon;
						} // if
					else if (Projection->DefDegToProj(&MyVert))
						{
						if (Geographic)
							{						
							if (MyVert.xyz[1] > NNorthing)
								NNorthing = MyVert.xyz[1];
							if (MyVert.xyz[1] < SNorthing)
								SNorthing = MyVert.xyz[1];
							if (MyVert.xyz[0] > WEasting)
								WEasting = MyVert.xyz[0];
							if (MyVert.xyz[0] < EEasting)
								EEasting = MyVert.xyz[0];
							} // if
						else
							{
							if (MyVert.xyz[1] > NNorthing)
								NNorthing = MyVert.xyz[1];
							if (MyVert.xyz[1] < SNorthing)
								SNorthing = MyVert.xyz[1];
							if (MyVert.xyz[0] < WEasting)
								WEasting = MyVert.xyz[0];
							if (MyVert.xyz[0] > EEasting)
								EEasting = MyVert.xyz[0];
							} // else
						} // else if
					else
						{
						NotDone = 1;
						break;
						} // else
					} // if
				else
					{
					NotDone = 1;
					break;
					} // else
				#else // WCS_BUILD_VNS
				if (PLink->Latitude > NNorthing)
					NNorthing = PLink->Latitude;
				if (PLink->Latitude < SNorthing)
					SNorthing = PLink->Latitude;
				if (PLink->Longitude > WEasting)
					WEasting = PLink->Longitude;
				if (PLink->Longitude < EEasting)
					EEasting = PLink->Longitude;
				#endif // WCS_BUILD_VNS
				PLink = PLink->Next;
				} while (PLink);
			} // if
		else
			NotDone = 1;
		} // if not DEM
	else
		NotDone = 1;
	} // if not children
else
	NotDone = 1;

if (NotDone)
	{
	NNorthing = -DBL_MAX; WEasting = -DBL_MAX;	// same values as set in constructor
	SNorthing = DBL_MAX; EEasting = DBL_MAX;
	return (0);
	} // else
#ifdef WCS_BUILD_VNS
if (GISConvention && Geographic)
	{
	WEasting = -WEasting;
	EEasting = -EEasting;
	} // if
#endif // WCS_BUILD_VNS
return (1);

} // Joe::GetBoundsProjected

/*===========================================================================*/

void Joe::ExaggeratePointElevations(PlanetOpt *DefPlanetOpt)
{
VectorPoint *PLink;

for (PLink = Points(); PLink; PLink = PLink->Next)
	{
	PLink->Elevation = (float)CalcExag((double)PLink->Elevation, DefPlanetOpt);
	} // for

} // Joe::ExaggeratePointElevations

/*===========================================================================*/

int Joe::DefDegToProj(VertexDEM *Vert)
{
CoordSys *MyCoords;

// see if there is a CoordSys attribute
if (MyCoords = GetCoordSys()) 
	{
	if (MyCoords->DefDegToProj(Vert))
		return (1);
	} // if
else
	{
	Vert->xyz[0] = Vert->Lon;
	Vert->xyz[1] = Vert->Lat;
	Vert->xyz[2] = Vert->Elev;
	return (1);
	} // else

return (0);

} // Joe::DefDegToProj

/*===========================================================================*/

void Joe::StretchNorth(double NewNorth)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (NWLat > SELat)
	{
	MyCoords = GetCoordSys();
	Stretch = (NewNorth - SELat) / (NWLat - SELat);
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lat = SELat + (Vert.Lat - SELat) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchNorth

/*===========================================================================*/

void Joe::StretchSouth(double NewSouth)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (NWLat > SELat)
	{
	Stretch = (NWLat - NewSouth) / (NWLat - SELat);
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lat = NWLat - (NWLat - Vert.Lat) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchSouth

/*===========================================================================*/

void Joe::StretchWest(double NewWest)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (NWLon > SELon)
	{
	Stretch = (NewWest - SELon) / (NWLon - SELon);
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lon = SELon + (Vert.Lon - SELon) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchWest

/*===========================================================================*/

void Joe::StretchEast(double NewEast)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (NWLon > SELon)
	{
	Stretch = (NWLon - NewEast) / (NWLon - SELon);
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lon = NWLon - (NWLon - Vert.Lon) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchEast

/*===========================================================================*/

void Joe::StretchLowElev(float MaxEl, float MinEl, float OldMinEl)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (MinEl < MaxEl && MaxEl != OldMinEl)
	{
	Stretch = (MaxEl - MinEl) / (MaxEl - OldMinEl);
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Elev = (float)(MaxEl - (MaxEl - Vert.Elev) * Stretch);
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchLowElev

/*===========================================================================*/

void Joe::StretchHighElev(float MaxEl, float MinEl, float OldMaxEl)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (MinEl < MaxEl && OldMaxEl != MinEl)
	{
	Stretch = (MaxEl - MinEl) / (OldMaxEl - MinEl);
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Elev = (float)(MinEl + (Vert.Elev - MinEl) * Stretch);
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchLowElev

/*===========================================================================*/

void Joe::StretchWidth(double RefLon, double NewWidth, double OldWidth)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (OldWidth > 0.0 && NewWidth > 0.0)
	{
	Stretch = NewWidth / OldWidth;
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lon = RefLon + (Vert.Lon - RefLon) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchWidth

/*===========================================================================*/

void Joe::StretchHeight(double RefLat, double NewHeight, double OldHeight)
{
double Stretch;
VectorPoint *PLink;
CoordSys *MyCoords;
VertexDEM Vert;

if (OldHeight > 0.0 && NewHeight > 0.0)
	{
	Stretch = NewHeight / OldHeight;
	MyCoords = GetCoordSys();
	if (PLink = Points())
		{
		do
			{
			PLink->ProjToDefDeg(MyCoords, &Vert);
			Vert.Lat = RefLat + (Vert.Lat - RefLat) * Stretch;
			PLink->DefDegToProj(MyCoords, &Vert);
			PLink = PLink->Next;
			} while (PLink);
		} // if
	} // if

} // Joe::StretchHeight

/*===========================================================================*/

double Joe::ComputeAreaDegrees(void)
{
double LastLat, LastLon, OriginLat, Area = 0.0;
VectorPoint *Pt, *NextPt;
CoordSys *MyCoords;
VertexDEM MyVert;

// draw first point unconditionally so that point style terraffectors with only one point work
if (Pt = GetFirstRealPoint())
	{
	MyCoords = GetCoordSys();
	if (Pt->ProjToDefDeg(MyCoords, &MyVert))
		{
		OriginLat = MyVert.Lat;
		for ( ; Pt; Pt = Pt->Next)
			{
			LastLat = MyVert.Lat;
			LastLon = MyVert.Lon;
			NextPt = Pt->Next;
			if (! NextPt)
				{
				NextPt = GetFirstRealPoint();
				} // if
			if (NextPt->ProjToDefDeg(MyCoords, &MyVert))
				{
				Area += ((MyVert.Lat - LastLat) * (MyVert.Lon - LastLon) * .5 + (LastLat - OriginLat) * (MyVert.Lon - LastLon));
				} // if
			} // for
		// since our longitudes are bass-ackwards
		Area = -Area;
		} // if
	} // if

return (Area);

} // Joe::ComputeAreaDegrees

/*===========================================================================*/

const char *Joe::GetBestName(void)
{
const char *TempName;

if (TempName = Name())
	return (TempName);
if (TempName = FileName())
	return (TempName);
if (TempName = CanonName())
	return TempName;
if (TempName = SecondaryName())
	return (TempName);
return ("Unnamed");

} // Joe::GetBestName

/*===========================================================================*/

void Joe::Edit(void)
{

GlobalApp->AppDB->SetActiveObj(this);

if (MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM) || TestFlags(WCS_JOEFLAG_ISDEM))
	{
	DONGLE_INLINE_CHECK()
	if (GlobalApp->GUIWins->DEM && GlobalApp->GUIWins->DEM->GetActive() != this)
		{
		delete GlobalApp->GUIWins->DEM;
		GlobalApp->GUIWins->DEM = NULL;
		}
	if (! GlobalApp->GUIWins->DEM)
		GlobalApp->GUIWins->DEM = new DEMEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->MainProj, this);
	if (GlobalApp->GUIWins->DEM)
		{
		if (GlobalApp->GUIWins->DEM->ConstructError)
			{
			delete GlobalApp->GUIWins->DEM;
			GlobalApp->GUIWins->DEM = NULL;
			} // if
		else
			GlobalApp->GUIWins->DEM->Open(GlobalApp->MainProj);
		}
	} // if
else
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_VEG, 0);

} // Joe::Edit

/*===========================================================================*/

long Joe::GetRAHostTypeNumber(void)
{

if (MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM) || TestFlags(WCS_JOEFLAG_ISDEM))
	{	
	return (WCS_RAHOST_OBJTYPE_DEM);
	} // if
else if (TestFlags(WCS_JOEFLAG_ISCONTROL))
	{	
	return (WCS_RAHOST_OBJTYPE_CONTROLPT);
	} // if
else
	{	
	return (WCS_RAHOST_OBJTYPE_VECTOR);
	} // if

} // Joe::GetRAHostTypeNumber

/*===========================================================================*/

char *Joe::GetRAHostTypeString(void)
{

if (MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM) || TestFlags(WCS_JOEFLAG_ISDEM))
	{	
	return ("(DEM)");
	} // if
else if (TestFlags(WCS_JOEFLAG_ISCONTROL))
	{	
	return ("(Control Point)");
	} // if
else if (TestFlags(WCS_JOEFLAG_HASKIDS))
	{	
	return ("(Group)");
	} // if
else
	{	
	return ("(Vector)");
	} // if

} // Joe::GetRAHostTypeString

/*===========================================================================*/

unsigned long Joe::GetRAFlags(unsigned long Mask)
{
JoeAttribute *MyTurn;
unsigned long Flags = 0;
long Type;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
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
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	if (TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (Children())
			{
			Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
			} // if
		} // if
	else
		{
		if (MyTurn = GetFirstAttribute())
			{
			while (MyTurn)
				{
				if (MyTurn->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && MyTurn->MinorAttrib() >= WCS_JOE_ATTRIB_INTERNAL_LAKE
					&& MyTurn->MinorAttrib() < WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP)
					{
					Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
					break;
					} // if
				MyTurn = MyTurn->GetNextAttribute();
				} // while
			} // if
		} // else
	} // if

if ((Type = GetRAHostTypeNumber()) == WCS_RAHOST_OBJTYPE_DEM)
	{	
	Mask &= (WCS_RAHOST_ICONTYPE_DEM | WCS_RAHOST_FLAGBIT_DRAGGABLE |
		WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);
	} // if
else if (Type == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{	
	Mask &= (WCS_RAHOST_ICONTYPE_CONTROLPT | WCS_RAHOST_FLAGBIT_DRAGGABLE |
		WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);
	} // if
else
	{	
	Mask &= (WCS_RAHOST_ICONTYPE_VECTOR | WCS_RAHOST_FLAGBIT_DRAGGABLE |
		WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);
	} // if

return (Mask);

} // Joe::GetRAFlags

/*===========================================================================*/

void Joe::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	if (TestFlags(WCS_JOEFLAG_ISDEM))
		{
		Prop->InterFlags = 0;
		} // if
	else
		{
		Prop->InterFlags = WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY  | WCS_RAHOST_INTERBIT_MOVEZ | WCS_RAHOST_INTERBIT_MOVEELEV | WCS_RAHOST_INTERBIT_SCALEX | WCS_RAHOST_INTERBIT_SCALEY | WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_POINTS;
		} // else
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = (char *)GetBestName();
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
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if

} // Joe::GetRAHostProperties

/*===========================================================================*/

int Joe::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		if (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED)
			SetFlags(WCS_JOEFLAG_ACTIVATED);
		else
			ClearFlags(WCS_JOEFLAG_ACTIVATED);
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	SetNewNames(Prop->Name, NULL);	// this sets the label
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // Joe::SetRAHostProperties

/*===========================================================================*/

char Joe::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	if (DropType == WCS_EFFECTSSUBCLASS_LAKE
		|| DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM
		|| DropType == WCS_EFFECTSSUBCLASS_WAVE
		|| DropType == WCS_EFFECTSSUBCLASS_CLOUD
		|| DropType == WCS_EFFECTSSUBCLASS_ENVIRONMENT
//		|| DropType == WCS_EFFECTSSUBCLASS_CMAP
		|| DropType == WCS_EFFECTSSUBCLASS_ILLUMINATION
		|| DropType == WCS_EFFECTSSUBCLASS_RASTERTA
		|| DropType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
		|| DropType == WCS_EFFECTSSUBCLASS_FOLIAGE
		|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D
		|| DropType == WCS_EFFECTSSUBCLASS_SHADOW
		|| DropType == WCS_EFFECTSSUBCLASS_STREAM
//		|| DropType == WCS_EFFECTSSUBCLASS_TERRAINPARAM
		|| DropType == WCS_EFFECTSSUBCLASS_GROUND
		|| DropType == WCS_EFFECTSSUBCLASS_CAMERA
		|| DropType == WCS_EFFECTSSUBCLASS_THEMATICMAP
		|| DropType == WCS_EFFECTSSUBCLASS_COORDSYS
		|| DropType == WCS_EFFECTSSUBCLASS_FENCE
		|| DropType == WCS_EFFECTSSUBCLASS_LABEL
		|| DropType == WCS_EFFECTSSUBCLASS_SNOW)
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector
		return (1);
	} // if
if (GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_DEM)
	{
	if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a DEM
		return (1);
	} // if
if (GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{
	if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a Control Pts
		return (1);
	} // if

return (0);

} // Joe::GetRAHostDropOK

/*===========================================================================*/

RasterAnimHost *Joe::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
JoeAttribute *Attr = NULL;
GeneralEffect *Effect;

if (! Current)
	Found = 1;
while (Attr = MatchAttributeRange(WCS_JOE_ATTRIB_INTERNAL, WCS_EFFECTSSUBCLASS_LAKE, WCS_MAXIMPLEMENTED_EFFECTS - 1, Attr))
	{
	if (Effect = GetAttributeEffect(Attr))
		{
		if (Found)
			return (Effect);
		if (Current == Effect)
			Found = 1;
		} // if
	} // while

return (NULL);

} // Joe::GetRAHostChild

/*===========================================================================*/

//lint -save -e527
GeneralEffect *Joe::GetAttributeEffect(JoeAttribute *Attr)
{

switch (Attr->MinorAttribType)
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		{
		return (((JoeLake *)Attr)->Lake);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		{
		return (((JoeEcosystem *)Attr)->Eco);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		{
		return (((JoeWave *)Attr)->Wave);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		{
		return (((JoeCloud *)Attr)->Cloud);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		{
		return (((JoeEnvironment *)Attr)->Env);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		{
		return (((JoeCmap *)Attr)->Cmap);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION:
		{
		// obsolete return (((JoeIllumination *)Attr)->Illum);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		return (((JoeRasterTA *)Attr)->Terra);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		return (((JoeTerraffector *)Attr)->Terra);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		{
		return (((JoeFoliage *)Attr)->Foliage);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		{
		return (((JoeObject3D *)Attr)->Object);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		{
		return (((JoeShadow *)Attr)->Shadow);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		{
		return (((JoeStream *)Attr)->Stream);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		{
		return (((JoeTerrainParam *)Attr)->TerrainPar);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		{
		return (((JoeGround *)Attr)->Ground);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		{
		return (((JoeSnow *)Attr)->SnowBusinessLikeShowBusiness);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		{
		return (((JoeCoordSys *)Attr)->Coord);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		{
		return (((JoeThematicMap *)Attr)->Theme);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		{
		return (((JoeFence *)Attr)->Fnce);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		{
		return (((JoeLabel *)Attr)->Labl);
		break;
		} // 
	case WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ:
		{
		return (((JoeAlign3DObj *)Attr)->Obj);
		break;
		} // 
// <<<>>> ADD_NEW_EFFECTS add case if effect can be attached to a Joe
	default:
		break;
	} // switch

return (NULL);

} // Joe::GetAttributeEffect
//lint -restore

/*===========================================================================*/

int Joe::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = -1, Result;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Joe *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Joe *)DropSource->DropSource);
			CopyPoints(this, (Joe *)DropSource->DropSource, 1, 1);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_CAMERA)
	{
	sprintf(QueryStr, "Transfer %s %s path to %s? Path Transfer window will open to accept parameters.", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		CopyCameraPath((Camera *)DropSource->DropSource);	// this takes care of notification
		Success = 1;
		} // if
	} // else if
else if (DropSource->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && DropSource->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	Success = -1;
	if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
		{
		sprintf(QueryStr, "Apply %s %s to %s for position or alignment?", DropSource->Name, DropSource->Type, NameStr);
		if ((Result = UserMessageCustom(NameStr, QueryStr, "Position", "Cancel", "Alignment", 0)) == 1)
			{
			AddEffect((GeneralEffect *)DropSource->DropSource, -1);	// this takes care of notification
			Success = 1;
			} // if position
		else if (Result == 2)
			{
			Success = ((Object3DEffect *)DropSource->DropSource)->SetAlignVec(this);	// takes care of notification
			} // else if alignment
		} // if
	else
		{
		sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			AddEffect((GeneralEffect *)DropSource->DropSource, -1);	// this takes care of notification
			Success = 1;
			} // if
		} // else
	} // else if

return (Success);

} // Joe::ProcessRAHostDragDrop

/*===========================================================================*/

int Joe::RemoveRAHost(RasterAnimHost *RemoveMe)
{
int Removed = 0;
JoeAttribute *Attr = NULL;
GeneralEffect *Effect;

while (Attr = MatchAttributeRange(WCS_JOE_ATTRIB_INTERNAL, WCS_EFFECTSSUBCLASS_LAKE, WCS_MAXIMPLEMENTED_EFFECTS - 1, Attr))
	{
	if ((Effect = GetAttributeEffect(Attr)) == RemoveMe)
		{
		if (Effect->EffectType == WCS_EFFECTSSUBCLASS_COORDSYS)
			{
			Effect->RemoveFromJoeList(((JoeCoordSys *)Attr)->CoordsJoeList);
			Attr = NULL;
			ZeroUpTree();
			//RecheckBounds();
			GlobalApp->AppDB->BoundUpTree(this);
			} // if
		else if (Attr == RemoveSpecificAttribute(Attr))
			{
			delete Attr;
			Attr = NULL;
			Removed = 1;
			Effect->RemoveRAHost(this);
			} // if
		} // if
	} // while
while (Attr = MatchAttributeRange(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ, WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ, Attr))
	{
	if (Effect = ((JoeAlign3DObj *)Attr)->Obj)
		{
		if (Attr == RemoveSpecificAttribute(Attr))
			{
			delete Attr;
			Attr = NULL;
			Removed = 1;
			Effect->RemoveRAHost(this);
			} // if
		} // if
	} // while

return (Removed);

} // Joe::RemoveRAHost

/*===========================================================================*/

void Joe::CopyCameraPath(Camera *PathSource)
{

if (GlobalApp->GUIWins->PTH)
	{
	UserMessageOK("Transfer to Path", "Path Transfer window is in use. Complete the current transfer operation and try again.");
	return;
	}
GlobalApp->GUIWins->PTH = new PathTransferGUI(GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppEffects, PathSource, this);
if (GlobalApp->GUIWins->PTH)
	{
	GlobalApp->GUIWins->PTH->Open(GlobalApp->MainProj);
	}

/*
long NumPts;
double CurTime;
AnimDoubleTime *Lat, *Lon, *Elev;
VectorPoint *PLink, *NewList;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Changes[1] = NULL;

Lat = PathSource->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
Lon = PathSource->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
Elev = PathSource->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);

Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;

CurTime = 0.0;
NumPts = 1;
while (1)	//lint !e716
	{
	Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = CurTime;
	Prop.NewKeyNodeRange[0] = -DBL_MAX;
	Prop.NewKeyNodeRange[1] = DBL_MAX;
	Lat->GetRAHostProperties(&Prop);
	if (Prop.NewKeyNodeRange[1] <= Prop.KeyNodeRange[1])
		break;
	CurTime = Prop.NewKeyNodeRange[1];
	NumPts ++;
	} //while

if (NewList = GlobalApp->AppDB->MasterPoint.Allocate(NumPts + 1))
	{
	PLink = NewList->Next;
	CurTime = 0.0;
	while (PLink)
		{
		PLink->Latitude = Lat->GetValue(0, CurTime);
		PLink->Longitude = Lon->GetValue(0, CurTime);
		PLink->Elevation = (float)Elev->GetValue(0, CurTime);
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = CurTime;
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		Lat->GetRAHostProperties(&Prop);
		CurTime = Prop.NewKeyNodeRange[1];
		PLink = PLink->Next;
		} //while
	NewList->Latitude = NewList->Next->Latitude;
	NewList->Longitude = NewList->Next->Longitude;
	NewList->Elevation = NewList->Next->Elevation;

	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
	GlobalApp->AppDB->GenerateNotify(Changes);
	if (Points)
		{
		GlobalApp->AppDB->MasterPoint.DeAllocate(Points);
		} // if
	Points = NewList;
	NumPoints = NumPts + 1;
	ZeroUpTree();
	RecheckBounds();
	GlobalApp->AppDB->ReBoundTree(WCS_DATABASE_STATIC);
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	GlobalApp->AppDB->GenerateNotify(Changes);
	} // if point array
*/
} // Joe::CopyCameraPath

/*===========================================================================*/

unsigned long Joe::ConformToTopo(void)
{
VectorPoint *Point;
CoordSys *MyCoords;
VertexDEM CurVert;
int Reject;
double TempElev;

if (GetFirstRealPoint())
	{
	MyCoords = GetCoordSys();
	for (Point = Points(); Point; Point = Point->Next)
		{
		if (Point->ProjToDefDeg(MyCoords, &CurVert))
			{
			TempElev = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(CurVert.Lat, CurVert.Lon, Reject);
			if (! Reject)
				{
				CurVert.Elev = TempElev;
				Point->DefDegToProjElev(MyCoords, &CurVert);
				} // if
			} // if
		} // for
	return (NumPoints());
	} // if points exist

return (0);

} // Joe::ConformToTopo

/*===========================================================================*/

bool Joe::MatchEndPoint(Joe *MatcheMe)
{
bool MatchedEnd = false;

if (JRendData && MatcheMe->JRendData)
	{
	if ((JRendData->LastPtLat == MatcheMe->JRendData->LastPtLat && JRendData->LastPtLon == MatcheMe->JRendData->LastPtLon)
		|| (JRendData->FirstPtLat == MatcheMe->JRendData->FirstPtLat && JRendData->FirstPtLon == MatcheMe->JRendData->FirstPtLon)
		|| (JRendData->LastPtLat == MatcheMe->JRendData->FirstPtLat && JRendData->LastPtLon == MatcheMe->JRendData->FirstPtLon)
		|| (JRendData->FirstPtLat == MatcheMe->JRendData->LastPtLat && JRendData->FirstPtLon == MatcheMe->JRendData->LastPtLon))
		{
		MatchedEnd = true;
		} // if
	} // if
	
return (MatchedEnd);

} // Joe::MatchEndPoint

/*===========================================================================*/

JoeRenderData *Joe::SetFirstLastCoords(double FirstLat, double FirstLon, double LastLat, double LastLon)
{

if (JRendData || (JRendData = new JoeRenderData()))
	{
	JRendData->FirstPtLat = FirstLat;
	JRendData->FirstPtLon = FirstLon;
	JRendData->LastPtLat = LastLat;
	JRendData->LastPtLon = LastLon;
	} // if

return (JRendData);

} // Joe::SetFirstLastCoords

/*===========================================================================*/

void Joe::Copy(Joe *CopyTo, Joe *CopyFrom)
{

// note this will not mess with points or attributes, nor change the name or filename
if (! CopyTo->TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	CopyTo->SetLineStyle(CopyFrom->GetLineStyle());
	CopyTo->SetLineWidth(CopyFrom->GetLineWidth());
	CopyTo->SetRGB(CopyFrom->Red(), CopyFrom->Green(), CopyFrom->Blue());
	CopyTo->SetFlags(CopyFrom->GetFlags() & (WCS_JOEFLAG_ISCONTROL | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED));
	} // if

} // Joe::Copy()

/*===========================================================================*/

unsigned long Joe::AttemptLoadDEM(short LoadRelEl, Project *OpenFrom)
{
JoeDEM *MyJD;

if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (MyJD = (JoeDEM *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		if (!MyJD->Pristine)
			{
			MyJD->Pristine = new DEM;
			} // if
		if (MyJD->Pristine)
			{
			return(MyJD->Pristine->AttemptLoadDEM(this, LoadRelEl, OpenFrom));
			} // if
		} // if
	} // if

return(0);

} // Joe::AttemptLoadDEM

/*===========================================================================*/

unsigned long Joe::AttemptLoadDownSampled(long GridSize, Project *OpenFrom)
{
JoeDEM *MyJD;

if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (MyJD = (JoeDEM *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		if (!MyJD->ViewPref)
			{
			MyJD->ViewPref = new DEM;
			} // if
		if (MyJD->ViewPref)
			{
			return(MyJD->ViewPref->AttemptLoadDownSampled(this, GridSize, OpenFrom));
			} // if
		} // if
	} // if

return(0);

} // Joe::AttemptLoadDownSampled

/*===========================================================================*/

int Joe::AttemptLoadBetterDownSampled(int MaxPolys, Project *OpenFrom)
{
JoeDEM *MyJD;

if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (MyJD = (JoeDEM *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		if (!MyJD->ViewPref)
			{
			MyJD->ViewPref = new DEM;
			} // if
		if (MyJD->ViewPref)
			{
			return(MyJD->ViewPref->AttemptLoadBetterDownSampled(this, OpenFrom, MaxPolys));
			} // if
		} // if
	} // if

return(0);

} // Joe::AttemptLoadBetterDownSampled

/*===========================================================================*/

JoeDEM::~JoeDEM()
{

if (Pristine)
	delete Pristine;
if (ViewPref)
	delete ViewPref;

Pristine = NULL;
ViewPref = NULL;

}; // JoeDEM::JoeDEM

/*===========================================================================*/

short Joe::GetClass(void)
{

if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	return(0); // Plain ol' DEM
	} // if
if (TestFlags(WCS_JOEFLAG_ISCONTROL)) return(2);

// All else aside, it's a vector
return(1);

} // Joe::GetClass()

/*===========================================================================*/

unsigned char Joe::GetLineStyleFromString(char *Str)
{
unsigned char Style = 4;

if (stricmp(Str, "Point"))
	Style = 0;
else if (stricmp(Str, "Circle"))
	Style = 1;
else if (stricmp(Str, "Square"))
	Style = 2;
else if (stricmp(Str, "Cross"))
	Style = 3;
else if (stricmp(Str, "Solid"))
	Style = 4;
else if (stricmp(Str, "Dotted"))
	Style = 5;
else if (stricmp(Str, "Dashed"))
	Style = 6;
else if (stricmp(Str, "Broken"))
	Style = 7;

return (Style);

} // Joe::GetLineStyleFromString

/*===========================================================================*/

unsigned long Joe::CountSetNumPoints(void)
{
VectorPoint *PLink;
unsigned long NewNumPts = 0;

NumPoints(0);

for (PLink = Points(); PLink; PLink = PLink->Next)
	{
	NewNumPts ++;
	} // for

return (NumPoints(NewNumPts));

} // Joe::CountSetNumPoints

/*===========================================================================*/

int Joe::SetProperties(JoeProperties *JoeProp)
{
int PropSet = 0;

if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_LINESTYLE)
	{
	SetLineStyle(JoeProp->LineStyle);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_LINEWEIGHT)
	{
	SetLineWidth(JoeProp->LineWeight);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_RED)
	{
	SetRed(JoeProp->Red);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_GREEN)
	{
	SetGreen(JoeProp->Green);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_BLUE)
	{
	SetBlue(JoeProp->Blue);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_ENABLED)
	{
	if (JoeProp->Enabled)
		SetFlags(WCS_JOEFLAG_ACTIVATED);
	else
		ClearFlags(WCS_JOEFLAG_ACTIVATED);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_DRAWENABLED)
	{
	if (JoeProp->DrawEnabled)
		SetFlags(WCS_JOEFLAG_DRAWENABLED);
	else
		ClearFlags(WCS_JOEFLAG_DRAWENABLED);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_RENDERENABLED)
	{
	if (JoeProp->RenderEnabled)
		SetFlags(WCS_JOEFLAG_RENDERENABLED);
	else
		ClearFlags(WCS_JOEFLAG_RENDERENABLED);
	PropSet = 1;
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_SELECTED)
	{
	if (GlobalApp->GUIWins->DBE)
		{
		GlobalApp->GUIWins->DBE->ChangeSelection(this, JoeProp->Selected);
		PropSet = 1;
		} // if
	} // if
if (JoeProp->Mask & WCS_JOEPROPERTIES_MASKBIT_EFFECT)
	{
	if (AddEffect(JoeProp->Effect, -1))
		PropSet = 1;
	} // if

return (PropSet);

} // Joe::SetProperties

/*===========================================================================*/

// Stuff to support Attributes hanging from special Layers

char LayerAttribNameTempCompose[255];

char *Joe::MakeAttributeModifiedName(char *Name, char SymbolAdd)
{
char *TestName;

if (Name)
	{
	if (Name[0] != SymbolAdd)
		{
		LayerAttribNameTempCompose[0] = SymbolAdd;
		strncpy(&LayerAttribNameTempCompose[1], Name, 250);
		LayerAttribNameTempCompose[251] = NULL;
		TestName = LayerAttribNameTempCompose;
		} // if
	else
		{
		strcpy(LayerAttribNameTempCompose, Name);
		// force to correct type if wrong type provided
		LayerAttribNameTempCompose[0] = SymbolAdd;
		LayerAttribNameTempCompose[251] = NULL;
		TestName = LayerAttribNameTempCompose;
		} // else
	return(TestName);
	} // if
return(NULL);

} // Joe::MakeAttributeModifiedName

/*===========================================================================*/

LayerStub *Joe::CheckTextAttributeExistance(char *Name)
{
char *TestName;
LayerEntry *MatchedLayer;

if (Name)
	{
	if (TestName = MakeAttributeModifiedName(Name, WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT))
		{
		// Find matching layer, if it exists
		if (MatchedLayer = GlobalApp->AppDB->DBLayers.MatchLayer(TestName))
			{
			return(MatchLayer(MatchedLayer));
			} // if
		} // if
	} // if
return(NULL);

} // Joe::CheckTextAttributeExistance

/*===========================================================================*/

LayerStub *Joe::CheckIEEEAttributeExistance(char *Name)
{
char *TestName;
LayerEntry *MatchedLayer;

if (Name)
	{
	if (TestName = MakeAttributeModifiedName(Name, WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL))
		{
		// Find matching layer, if it exists
		if (MatchedLayer = GlobalApp->AppDB->DBLayers.MatchLayer(TestName))
			{
			return(MatchLayer(MatchedLayer));
			} // if
		} // if
	} // if
return(NULL);

} // Joe::CheckIEEEAttributeExistance

/*===========================================================================*/

LayerStub *Joe::CheckAttributeExistance(LayerEntry *CheckEntry)
{

return(MatchLayer(CheckEntry));

} // Joe::CheckAttributeExistance

/*===========================================================================*/

char *Joe::GetTextAttributeValue(char *Name)
{
char *rVal = NULL;
LayerStub *LS;

if (LS = CheckTextAttributeExistance(Name))
	{
	rVal = GetTextAttributeValue(LS);
	} // if

return(rVal);

} // Joe::GetTextAttributeValue

/*===========================================================================*/

double Joe::GetIEEEAttributeValue(char *Name)
{
double rVal = 0.0;
LayerStub *LS;

if (LS = CheckIEEEAttributeExistance(Name))
	{
	rVal = GetIEEEAttributeValue(LS);
	} // if

return(rVal);

} // Joe::GetIEEEAttributeValue

/*===========================================================================*/

char *Joe::GetTextAttributeValue(LayerStub *GetFrom)
{
char *rVal = NULL;

if (GetFrom)
	{
	rVal = GetFrom->GetTextAttribVal();
	} // if

return(rVal);

} // Joe::GetTextAttributeValue

/*===========================================================================*/

double Joe::GetIEEEAttributeValue(LayerStub *GetFrom)
{
double rVal = 0.0;

if (GetFrom)
	{
	rVal = GetFrom->GetIEEEAttribVal();
	} // if

return(rVal);

} // Joe::GetIEEEAttributeValue

/*===========================================================================*/

int Joe::GetAttribValueList(JoeAttribLayerData *AttrList)
{
JoeAttribLayerData *CurAttr = AttrList;
LayerStub *GetFrom;
int Found = 0;

while (CurAttr)
	{
	if (CurAttr->AttribName)
		{
		if (GetFrom = CheckTextAttributeExistance(CurAttr->AttribName))
			{
			CurAttr->AttrValueTxt = GetTextAttributeValue(GetFrom);
			Found = 1;
			} // if
		else if (GetFrom = CheckIEEEAttributeExistance(CurAttr->AttribName))
			{
			CurAttr->AttrValueDbl = GetIEEEAttributeValue(GetFrom);
			Found = 1;
			} // else if
		} // if
	CurAttr = CurAttr->Next;
	} // while

return (Found);

} // Joe::GetAttribValueList

/*===========================================================================*/

LayerStub *Joe::AddTextAttribute(char *AttribName, const char *NewText)
{
LayerEntry *MatchedLayer;
char *TestName;

if (AttribName)
	{
	if (TestName = MakeAttributeModifiedName(AttribName, WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT))
		{
		// Find/create matching layer
		if (MatchedLayer = GlobalApp->AppDB->DBLayers.MatchMakeLayer(TestName))
			{
			return(AddTextAttribute(MatchedLayer, NewText));
			} // if
		} // if
	} // if
return(NULL);
} // Joe::AddTextAttribute

/*===========================================================================*/

LayerStub *Joe::AddIEEEAttribute(char *AttribName, double NewIEEE)
{
LayerEntry *MatchedLayer;
char *TestName;

if (AttribName)
	{
	if (TestName = MakeAttributeModifiedName(AttribName, WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL))
		{
		// Find/create matching layer
		if (MatchedLayer = GlobalApp->AppDB->DBLayers.MatchMakeLayer(TestName))
			{
			return(AddIEEEAttribute(MatchedLayer, NewIEEE));
			} // if
		} // if
	} // if

return(NULL);

} // Joe::AddIEEEAttribute

/*===========================================================================*/

LayerStub *Joe::AddTextAttribute(LayerEntry *AttribEntry, const char *NewText)
{
LayerStub *LS;

if (LS = AddObjectToLayer(AttribEntry))
	{
	LS->SetTextAttribVal(NewText);
	return(LS);
	} // if

return(NULL);

} // Joe::AddTextAttribute

/*===========================================================================*/

LayerStub *Joe::AddIEEEAttribute(LayerEntry *AttribEntry, double NewIEEE)
{
LayerStub *LS;

if (LS = AddObjectToLayer(AttribEntry))
	{
	LS->SetIEEEAttribVal(NewIEEE);
	return(LS);
	} // if

return(NULL);

} // Joe::AddIEEEAttribute

/*===========================================================================*/

int Joe::SXQuerySetupForExport(SXQueryAction *AddAction, const char *OutputFilePathSource)
{
JoeSXQueryItem *JoeSX;

// look for JoeSXQueryItem attribute
if (! (JoeSX = (JoeSXQueryItem *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM)))
	{
	if (JoeSX = new JoeSXQueryItem())
		AddAttribute((JoeAttribute *)JoeSX);
	} // if
if (JoeSX)
	{
	return (JoeSX->SXQuerySetupForExport(AddAction, OutputFilePathSource));
	} // if

return (0);

} // Joe::SXQuerySetupForExport

/*===========================================================================*/

void Joe::SXQueryCleanupFromExport(void)
{
JoeSXQueryItem *JoeSX;

// look for JoeSXQueryItem attribute
// clear it and remove it
if (JoeSX = (JoeSXQueryItem *)RemoveAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM))
	{
	JoeSX->SXQueryCleanupFromExport();
	delete JoeSX;
	} // if

} // Joe::SXQueryCleanupFromExport

/*===========================================================================*/

bool Joe::IsSXClickQueryEnabled(void)
{
JoeSXQueryItem *JoeSX;

// look for JoeSXQueryItem attribute
if (JoeSX = (JoeSXQueryItem *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM))
	{
	return (JoeSX->IsClickQueryEnabled());
	} // if

return (0);

} // Joe::IsSXClickQueryEnabled

/*===========================================================================*/

long Joe::GetSXQueryRecordNumber(NameList **FileNamesCreated)
{
JoeSXQueryItem *JoeSX;

// look for JoeSXQueryItem attribute
if (JoeSX = (JoeSXQueryItem *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM))
	{
	return (JoeSX->GetRecordNumber(NULL, this, FileNamesCreated));
	} // if

return (-1);

} // Joe::GetSXQueryRecordNumber

/*===========================================================================*/


int Joe::AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags)
{

if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	PMA->AddPopMenuItem("Paint", "PAINTDEM", 1);
	} // if

return(1);
} // Joe::AddDerivedPopMenus

/*===========================================================================*/

int Joe::HandlePopMenuSelection(void *Action)
{
// only one action known currently --DEM Paint
if (GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->GetActive() != this)
	{
	delete GlobalApp->GUIWins->DPG;
	GlobalApp->GUIWins->DPG = NULL;
	}
if (! GlobalApp->GUIWins->DPG)
	GlobalApp->GUIWins->DPG = new DEMPaintGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->MainProj, this);
if (GlobalApp->GUIWins->DPG)
	{
	if (GlobalApp->GUIWins->DPG->ConstructError)
		{
		delete GlobalApp->GUIWins->DPG;
		GlobalApp->GUIWins->DPG = NULL;
		} // if
	else
		GlobalApp->GUIWins->DPG->Open(GlobalApp->MainProj);
	}
return(1);
} // Joe::HandlePopMenuSelection
