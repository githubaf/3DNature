// Interactive.cpp
// Code for common Interactive elements
// Created from scratch on 5/14/96 by CXH
// Copyright 1996

#include "stdafx.h"
#include "Interactive.h"
#include "Points.h"
#include "Useful.h"
#include "UsefulMath.h"
#include "Joe.h"
#include "MathSupport.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "Render.h"
#include "DBFilterEvent.h"
#include "Conservatory.h"
#include "DrillDownInfoGUI.h"
#include "DEM.h"
#include <string.h>
#include "Lists.h"

#undef RGB // stupid Microsoft makes macros with hazardously-common names

DiagnosticData::DiagnosticData()
{
int Ct;

for (Ct = 0; Ct < WCS_MAX_DIAGNOSTIC_VALUES; Ct++)
	{
	Value[Ct] = 0.0;
	} // for
for (Ct = 0; Ct < WCS_DIAGNOSTIC_NUMBUFFERS; Ct++)
	{
	ValueValid[Ct] = 0;
	} // for
Object = NULL;
DataRGB[0] = DataRGB[1] = DataRGB[2] = Alpha = 0;
DisplayedBuffer = WCS_DIAGNOSTIC_RGB;
RefLat = RefLon = 0.0;
PixelX = PixelY = PixelZ = MoveX = MoveY = MoveZ = 0;
DimX = DimY = 1;
ViewSource = -1;
ThresholdValid = 0;

} // DiagnosticData::DiagnosticData

/*===========================================================================*/
/*===========================================================================*/

VectorEdit::VectorEdit()
{

Scale[0] = Scale[1] = Scale[2] = Shift[0] = Shift[1] = Shift[2] = Smooth[2] =
	Rotate = PreserveXY = ConsiderElev = 0;
PtRelative = WCS_VECTOR_DISPLAYMODE_ABSOLUTE;
PtOperate = WCS_VECTOR_PTOPERATE_ALLPTS;
InsertSpline = WCS_VECTOR_INSERT_LINEAR;
HorUnits = WCS_VECTOR_HUNITS_DEGREES;
VertUnits = WCS_VECTOR_VUNITS_METERS;
RefControl = WCS_VECTOR_REFERENCE_VECCENTER;
Smooth[0] = Smooth[1] = TopoConform = 1;
InterpPts = 1000;
InsertPts = InsertAfter = OriginVertex = FirstRangePt = LastRangePt = 1;
RestoreNumPts = UndoNumPts = RefNumPts = SelectedNumPts = 0;
ScaleAmt[0] = ScaleAmt[1] = ScaleAmt[2] = 1.0;
ShiftAmt[0] = ShiftAmt[1] = ShiftAmt[2] = 0.0;
ArbRefCoord[0] = ArbRefCoord[1] = ArbRefCoord[2] = 0.0;
ProjRefCoord[0] = ProjRefCoord[1] = ProjRefCoord[2] = 0.0;
RefCoord[0] = RefCoord[1] = RefCoord[2] = 0.0;
RotateAmt = 0.0;
Smoothing = 50.0;
SetHUnitScale();
SetVUnitScale();;
RestorePts = UndoPts = RefPts = NULL;
memset(&SelPts[0], 0, WCS_DXF_MAX_OBJPTS * sizeof (VectorPoint *));
memset(&SelRefPts[0], 0, WCS_DXF_MAX_OBJPTS * sizeof (VectorPoint *));
memset(&SelPtsIndex[0], 0, WCS_DXF_MAX_OBJPTS * sizeof (unsigned short));
Active = NULL;
ActivePoint = NULL;
ProjRefCoordsFloating = 0;

} // VectorEdit::VectorEdit

/*===========================================================================*/

VectorEdit::~VectorEdit()
{

if (UndoPts)
	{
	// deallocate
	} // if
if (RefPts)
	{
	// deallocate
	} // if

} // VectorEdit::~VectorEdit

/*===========================================================================*/
/*===========================================================================*/

InterCommon::InterCommon(Database *DBSource, Project *ProjSource)
{
double RangeDefaults[3];
static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 0};
static NotifyTag AllAppEvents[] = {MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
									MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
									MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff),
									0};

// Map View
/*TopoEnable = VecEnable = EcoEnable = InterEnable = 1;
Align = 0;
Clear = 1;
Dither = 1;
DrawStyle = 0;
ColorRange = */FollowTerrain = 0;
/*RulersEnabled = ControlPtsEnabled = ColorMapsEnabled = 0;
EditMode = EditItem = */EditPointsMode/* = RulerSystem*/ = 0;
GridSample = 50000;
TfxPreview = 0;
TfxRealtime = 0;
ProjFrameRate = 30.0;
ActiveTime = 0.0;
DigitizeDrawMode = 0;

ActiveDEM = NULL;

//GridOverlaySize

GridOverlaySize.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
GridOverlaySize.SetCurValue(0.0);
GridOverlaySize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 10.0;
GridOverlaySize.SetRangeDefaults(RangeDefaults);

DigElevation.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DigElevation.SetCurValue(-1.0);
DigElevation.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = -FLT_MAX;
RangeDefaults[2] = 50.0;
DigElevation.SetRangeDefaults(RangeDefaults);

DigSmooth.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DigSmooth.SetCurValue(-1.0);
RangeDefaults[0] = 100.0;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 1.0;
DigSmooth.SetRangeDefaults(RangeDefaults);
DigSpace.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DigSpace.SetCurValue(-1.0);
DigSpace.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 5.0;
DigSpace.SetRangeDefaults(RangeDefaults);

DigSpeed.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DigSpeed.SetCurValue(-1.0);
DigSpeed.SetMetricType(WCS_ANIMDOUBLE_METRIC_VELOCITY);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 5.0; // meters per second
DigSpeed.SetRangeDefaults(RangeDefaults);

DBHost = DBSource;
ProjHost = ProjSource;
if (DBHost)
	DBHost->RegisterClient(this, AllDBEvents);
if (GlobalApp && GlobalApp->AppEx)
	GlobalApp->AppEx->RegisterClient(this, AllAppEvents);

} // InterCommon::InterCommon

/*===========================================================================*/

InterCommon::~InterCommon()
{

DBHost->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
ClearAllPts();
if (ActiveDEM)
	{
	ActiveDEM->FreeRawElevs();
	delete ActiveDEM;
	} // if
ActiveDEM = NULL;

} // InterCommon::~InterCommon

/*===========================================================================*/

void InterCommon::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[9];
Joe *NewGuy;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
if (Changed = DBHost->MatchNotifyClass(Interested, Changes, 1))
	{
	if (((NewGuy = (Joe *)Activity->ChangeNotify->NotifyData) != VE.Active) || (NewGuy && NewGuy->NumPoints() != VE.RestoreNumPts && VE.RestoreNumPts == 0))
		{
		ClearAllPts();
		if (NewGuy && ! NewGuy->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			SetNewObj(NewGuy);
			} // if
		} // if
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_NEW, 0xff, 0xff);
Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_LOAD, 0xff, 0xff);
Interested[2] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_ADDOBJ, 0xff, 0xff);
Interested[3] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_DELOBJ, 0xff, 0xff);
Interested[4] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_FLAGS, 0xff);
Interested[5] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[6] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[7] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
Interested[8] = NULL;
if (Changed = DBHost->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ActiveDEM)
		{
		delete ActiveDEM;		// frees all DEM arrays too
		ActiveDEM = NULL;
		} // if
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
if (Changed = DBHost->MatchNotifyClass(Interested, Changes, 1))
	{
	if (VE.Active && VE.Active == DBHost->GetActiveObj())
		{
		PointsChanged(VE.Active);
		if (VE.TopoConform && ((Changed & 0x000000ff) == WCS_NOTIFYDBASECHANGE_POINTS_CONFORM))
			ConformToTopo();
		} // if
	} // if

} // InterCommon::HandleNotifyEvent()

/*===========================================================================*/

void InterCommon::SetParam(int Notify, ...)
{
va_list VarA;
unsigned int Change = 0, ParClass, SubClass, Item, Component;
unsigned long ChgFlag = NULL;
ULONG InterNotifyChanges[WCS_MAXINTER_NOTIFY_CHANGES + 1];

va_start(VarA, Notify);

ParClass = va_arg(VarA, int);

while (ParClass && Change < WCS_MAXINTER_NOTIFY_CHANGES)
	{
	if (ParClass > 255)
		{
		InterNotifyChanges[Change] = ParClass;
		SubClass = (ParClass & 0xff0000) >> 16;
		Item = (ParClass & 0xff00) >> 8;
		Component = (ParClass & 0xff);
		ParClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		InterNotifyChanges[Change] = (ParClass << 24) + (SubClass << 16) + (Item << 8) + Component;
		} // else event passed as decomposed list of class, subclass, item and component

	// Do stuff here
	switch (ParClass)
		{
		case WCS_INTERCLASS_CAMVIEW:
			{
			ChgFlag = NULL;
			switch (SubClass)
				{
				case WCS_INTERCAM_SUBCLASS_GRANULARITY:
					{
					switch (Item)
						{
						case WCS_INTERCAM_ITEM_GRIDSAMPLE:
							{
							GridSample = (long)(va_arg(VarA, int));
							break;
							}
						} // switch item
					break;
					} // WCS_INTERCAM_SUBCLASS_GRANULARITY
				case WCS_INTERCAM_SUBCLASS_SETTINGS:
					{
					switch (Item)
						{
						case WCS_INTERCAM_ITEM_SETTINGS_TFXPREVIEW:
							{
							TfxPreview = (char)(va_arg(VarA, int));
							break;
							}
						case WCS_INTERCAM_ITEM_SETTINGS_TFXREALTIME:
							{
							TfxRealtime = (char)(va_arg(VarA, int));
							break;
							}
						} // switch item
					break;
					} // WCS_INTERCAM_SUBCLASS_SETTINGS
				} // switch subclass
			break;
			} // WCS_INTERCLASS_CAMVIEW
		case WCS_INTERCLASS_MAPVIEW:
			{
			ChgFlag = NULL;
			switch (SubClass)
				{
				case WCS_INTERMAP_SUBCLASS_MISC:
					{
					switch (Item)
						{
						case WCS_INTERMAP_ITEM_FOLLOWTERRAIN:
							{
							FollowTerrain = (char)va_arg(VarA, int);
							break;
							}
						case WCS_INTERMAP_ITEM_POINTSMODE:
							{
							EditPointsMode = (unsigned short)va_arg(VarA, int);
							break;
							}
						} // switch
					break;
					} // WCS_INTERMAP_SUBCLASS_MISC
				} // switch subclass
			break;
			} // WCS_INTERCLASS_MAPVIEW
		case WCS_INTERCLASS_VECTOR:
			{
			switch (SubClass)
				{
				case WCS_INTERVEC_SUBCLASS_VALUE:
					{
					unsigned long ValA;
					int ValB;
					switch (Item)
						{
						case WCS_INTERVEC_ITEM_SCALE:
							{
							VE.Scale[Component] = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_SCALEAMOUNT:
							{
							VE.ScaleAmt[Component] = (double)va_arg(VarA, double);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFT:
							{
							VE.Shift[Component] = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTAMOUNT:
							{
							VE.ShiftAmt[Component] = (double)va_arg(VarA, double);
							break;
							}
						case WCS_INTERVEC_ITEM_ROTATE:
							{
							VE.Rotate = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_ROTATEAMOUNT:
							{
							VE.RotateAmt = (double)va_arg(VarA, double);
							break;
							}
						case WCS_INTERVEC_ITEM_PRESERVEXY:
							{
							VE.PreserveXY = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_SMOOTH:
							{
							VE.Smooth[Component] = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_SMOOTHING:
							{
							VE.Smoothing = (double)va_arg(VarA, double);
							break;
							}
						case WCS_INTERVEC_ITEM_CONSIDERELEV:
							{
							VE.ConsiderElev = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_INTERPPTS:
							{
							VE.InterpPts = (long)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTPTS:
							{
							VE.InsertPts = (long)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTAFTER:
							{
							VE.InsertAfter = (long)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_ORIGINVERTEX:
							{
							VE.OriginVertex = (long)va_arg(VarA, int);
							if (VE.RefControl == WCS_VECTOR_REFERENCE_VECORIGIN)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_FIRSTRANGEPT:
							{
							VE.FirstRangePt = (long)va_arg(VarA, int);
							SetPointsSelected();
							if (VE.RefControl == WCS_VECTOR_REFERENCE_SELPTAVG)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_LASTRANGEPT:
							{
							VE.LastRangePt = (long)va_arg(VarA, int);
							SetPointsSelected();
							if (VE.RefControl == WCS_VECTOR_REFERENCE_SELPTAVG)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_PTRELATIVE:
							{
							VE.PtRelative = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_PTOPERATE:
							{
							VE.PtOperate = (short)va_arg(VarA, int);
							SetPointsSelected();
							if (VE.RefControl == WCS_VECTOR_REFERENCE_SELPTAVG)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_TOPOCONFORM:
							{
							VE.TopoConform = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTSPLINE:
							{
							VE.InsertSpline = (short)va_arg(VarA, int);
							break;
							}
						case WCS_INTERVEC_ITEM_REFCONTROL:
							{
							VE.RefControl = (short)va_arg(VarA, int);
							ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_ARBREFCOORD:
							{
							VE.ArbRefCoord[Component] = (double)va_arg(VarA, double);
							if (VE.RefControl == WCS_VECTOR_REFERENCE_ARBCOORD)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_PROJREFCOORD:
							{
							VE.ProjRefCoord[Component] = (double)va_arg(VarA, double);
							if (VE.RefControl == WCS_VECTOR_REFERENCE_PROJCOORD)
								ComputeReferenceCoords();
							GlobalApp->AppEffects->SyncFloaters(GlobalApp->AppDB, GlobalApp->MainProj, 0);
							break;
							}
						case WCS_INTERVEC_ITEM_REFCOORD:
							{
							VE.RefCoord[Component] = (double)va_arg(VarA, double);
							break;
							}
						case WCS_INTERVEC_ITEM_HORUNITS:
							{
							VE.HorUnits = (short)va_arg(VarA, int);
							if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
								VE.PtRelative = WCS_VECTOR_DISPLAYMODE_RELATIVE;
							VE.SetHUnitScale();
							break;
							}
						case WCS_INTERVEC_ITEM_VERTUNITS:
							{
							VE.VertUnits = (short)va_arg(VarA, int);
							VE.SetVUnitScale();
							break;
							}
						case WCS_INTERVEC_ITEM_SETPOINTSELECT:
							{
							UnSelectAllPoints();
							// Fall through to selection work, below.
							//break;
							}
							//lint -fallthrough
						case WCS_INTERVEC_ITEM_ADDPOINTSELECT:	//lint !e616
							{
							ValA = (unsigned long)va_arg(VarA, int);
							ValB = (int)va_arg(VarA, int);
							SetPointSelState(ValA, ValB);
							if (VE.RefControl == WCS_VECTOR_REFERENCE_SELPTAVG)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_TOGGLEPOINTSELECT:
							{
							unsigned long Pt = (unsigned long)va_arg(VarA, int);
							SetPointSelState(Pt, ! GetPointSelState((unsigned short)Pt));
							if (VE.RefControl == WCS_VECTOR_REFERENCE_SELPTAVG)
								ComputeReferenceCoords();
							break;
							}
						case WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT:
							{
							VE.ProjRefCoordsFloating = (short)va_arg(VarA, int);
							SetFloating(VE.ProjRefCoordsFloating, NULL);
							if (VE.ProjRefCoordsFloating)
								GlobalApp->AppEffects->SyncFloaters(GlobalApp->AppDB, GlobalApp->MainProj, 0);
							break;
							}
						default:
							break;
						} // switch item
					break;
					} // WCS_INTERVEC_SUBCLASS_VALUE
				case WCS_INTERVEC_SUBCLASS_OPERATE:
					{
					NotifyTag Immediate[2];
					double Value[3];

					Immediate[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_PREOPERATE, 0, 0);
					Immediate[1] = NULL;
					GenerateNotify(Immediate);
					switch (Item)
						{
						case WCS_INTERVEC_ITEM_SHIFTPTLAT:
							{
							ShiftPointLat(GlobalApp->AppEffects->GetPlanetRadius(), (double)va_arg(VarA, double), TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTPTLON:
							{
							ShiftPointLon(GlobalApp->AppEffects->GetPlanetRadius(), (double)va_arg(VarA, double), TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTPTELEV:
							{
							Value[0] = (double)va_arg(VarA, double);
							ShiftPointElev((float)Value[0]);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTPTLONLAT:
							{
							Value[0] = (double)va_arg(VarA, double);
							Value[1] = (double)va_arg(VarA, double);
							ShiftPointLonLat(GlobalApp->AppEffects->GetPlanetRadius(), Value[0], Value[1], TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV:
							{
							Value[0] = (double)va_arg(VarA, double);
							Value[1] = (double)va_arg(VarA, double);
							Value[2] = (double)va_arg(VarA, double);
							ShiftPointLonLatElev(GlobalApp->AppEffects->GetPlanetRadius(), Value[0], Value[1], Value[2], TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SCALEPTLAT:
							{
							ScalePointLat(GlobalApp->AppEffects->GetPlanetRadius(), (double)va_arg(VarA, double), TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SCALEPTLON:
							{
							ScalePointLon(GlobalApp->AppEffects->GetPlanetRadius(), (double)va_arg(VarA, double), TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_SCALEPTLONLAT:
							{
							Value[0] = (double)va_arg(VarA, double);
							Value[1] = (double)va_arg(VarA, double);
							ScalePointLonLat(GlobalApp->AppEffects->GetPlanetRadius(), Value[0], Value[1], TRUE);
							break;
							}
						case WCS_INTERVEC_ITEM_ROTATEPTDEG:
							{
							RotatePointDegXY(GlobalApp->AppEffects->GetPlanetRadius(), (double)va_arg(VarA, double), TRUE);
							break;
							}
						default:
							break;
						} // switch item
					break;
					} // WCS_INTERVEC_SUBCLASS_OPERATE
				case WCS_INTERVEC_SUBCLASS_BACKUP:
					{
					switch (Item)
						{
						case WCS_INTERVEC_ITEM_BACKUP:
							{
							break;
							}
						case WCS_INTERVEC_ITEM_UNDO:
							{
							UndoPoints(VE.Active);
							break;
							}
						case WCS_INTERVEC_ITEM_FREEZE:
							{
							break;
							}
						case WCS_INTERVEC_ITEM_RESTORE:
							{
							break;
							}
						default:
							break;
						} // switch item
					break;
					} // WCS_INTERCAM_SUBCLASS_VALUE
				} // switch subclass
			break;
			} // WCS_INTERCLASS_VECTOR
		case WCS_INTERCLASS_MISC:
			{
			switch (SubClass)
				{
				case WCS_INTERMISC_SUBCLASS_MISC:
					{
					switch (Item)
						{
						case WCS_INTERMISC_ITEM_PROJFRAMERATE:
							{
							ProjFrameRate = (double)va_arg(VarA, double);
							break;
							} // WCS_INTERMISC_ITEM_PROJFRAMERATE
						case WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE:
							{
							DigitizeDrawMode = (short)va_arg(VarA, int);
							break;
							} // WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE
						default:
							break;
						} // switch
					break;
					} // WCS_INTERMISC_SUBCLASS_MISC
				} // switch
			break;
			} // WCS_INTERCLASS_MISC
		case WCS_INTERCLASS_TIME:
			{
			switch (SubClass)
				{
				case WCS_TIME_SUBCLASS_TIME:
					{
					ActiveTime = (double)va_arg(VarA, double);
					#ifdef WCS_BUILD_DEMO
					if (ActiveTime > 5.0)
						{
						UserMessageDemo("Time cannot be set to greater than 5 seconds.");
						ActiveTime = 5.0;
						} // if
					#endif // WCS_BUILD_DEMO
					break;
					} // time
				case WCS_TIME_SUBCLASS_FRAME:
					{
					ActiveTime = (double)(va_arg(VarA, int) / (ProjFrameRate > 0.0 ? ProjFrameRate: 30.0));
					#ifdef WCS_BUILD_DEMO
					if (ActiveTime > 5.0)
						{
						UserMessageDemo("Time cannot be set to greater than 5 seconds.");
						ActiveTime = 5.0;
						} // if
					#endif // WCS_BUILD_DEMO
					break;
					} // time
				} // switch
			GlobalApp->UpdateProjectByTime();
			break;
			} // time
		} // switch ParClass
	
	ParClass = va_arg(VarA, int);
	Change++;

	} /* while */

va_end(VarA);

InterNotifyChanges[Change] = 0;

if (Notify)
	{
	GenerateNotify(InterNotifyChanges);
	} // if notify clients of changes

} // InterCommon::SetParam


void InterCommon::GetParam(void *Value, ...)
{
va_list VarA;
unsigned int ParClass, SubClass, Item, Component;
double *DblPtr = (double *)Value;
float *FltPtr = (float *)Value;
short *ShtPtr = (short *)Value;
long *LngPtr = (long *)Value;
char *BytePtr = (char *)Value;

va_start(VarA, Value);

ParClass = va_arg(VarA, int);

if (ParClass)
	{
	if (ParClass > 255)
		{
		SubClass = (ParClass & 0xff0000) >> 16;
		Item = (ParClass & 0xff00) >> 8;
		Component = (ParClass & 0xff);
		ParClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		} // else event passed as decomposed list of class, subclass, item and component

	// Do stuff here
	switch (ParClass)
		{
		case WCS_INTERCLASS_CAMVIEW:
			{
			switch (SubClass)
				{
				case WCS_INTERCAM_SUBCLASS_GRANULARITY:
					{
					switch (Item)
						{
						case WCS_INTERCAM_ITEM_GRIDSAMPLE:
							{
							*LngPtr = GridSample;
							break;
							}
						} // switch item
					break;
					} // WCS_INTERCAM_SUBCLASS_GRANULARITY
				case WCS_INTERCAM_SUBCLASS_SETTINGS:
					{
					switch (Item)
						{
						case WCS_INTERCAM_ITEM_SETTINGS_TFXPREVIEW:
							{
							*BytePtr = TfxPreview;
							break;
							}
						case WCS_INTERCAM_ITEM_SETTINGS_TFXREALTIME:
							{
							*BytePtr = TfxRealtime;
							break;
							}
						} // switch item
					break;
					} // WCS_INTERCAM_SUBCLASS_SETTINGS
				} // switch subclass
			break;
			} // WCS_INTERCLASS_CAMVIEW
		case WCS_INTERCLASS_MAPVIEW:
			{
			switch (SubClass)
				{
				case WCS_INTERMAP_SUBCLASS_MISC:
					{
					switch (Item)
						{
						case WCS_INTERMAP_ITEM_FOLLOWTERRAIN:
							{
							*BytePtr = (char)FollowTerrain;
							break;
							}
						case WCS_INTERMAP_ITEM_POINTSMODE:
							{
							*ShtPtr = EditPointsMode;
							break;
							}
						} // switch
					break;
					} // WCS_INTERMAP_SUBCLASS_MISC
				} // switch
			break;
			} // WCS_INTERCLASS_MAPVIEW
		case WCS_INTERCLASS_VECTOR:
			{
			switch (SubClass)
				{
				case WCS_INTERVEC_SUBCLASS_VALUE:
					{
					switch (Item)
						{
						case WCS_INTERVEC_ITEM_SCALE:
							{
							*ShtPtr = VE.Scale[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_SCALEAMOUNT:
							{
							*DblPtr = VE.ScaleAmt[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFT:
							{
							*ShtPtr = VE.Shift[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_SHIFTAMOUNT:
							{
							*DblPtr = VE.ShiftAmt[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_ROTATE:
							{
							*ShtPtr = VE.Rotate;
							break;
							}
						case WCS_INTERVEC_ITEM_ROTATEAMOUNT:
							{
							*DblPtr = VE.RotateAmt;
							break;
							}
						case WCS_INTERVEC_ITEM_PRESERVEXY:
							{
							*ShtPtr = VE.PreserveXY;
							break;
							}
						case WCS_INTERVEC_ITEM_SMOOTH:
							{
							*ShtPtr = VE.Smooth[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_SMOOTHING:
							{
							*DblPtr = VE.Smoothing;
							break;
							}
						case WCS_INTERVEC_ITEM_CONSIDERELEV:
							{
							*ShtPtr = VE.ConsiderElev;
							break;
							}
						case WCS_INTERVEC_ITEM_INTERPPTS:
							{
							*LngPtr = VE.InterpPts;
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTPTS:
							{
							*LngPtr = VE.InsertPts;
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTAFTER:
							{
							*LngPtr = VE.InsertAfter;
							break;
							}
						case WCS_INTERVEC_ITEM_ORIGINVERTEX:
							{
							*LngPtr = VE.OriginVertex;
							break;
							}
						case WCS_INTERVEC_ITEM_FIRSTRANGEPT:
							{
							*LngPtr = VE.FirstRangePt;
							break;
							}
						case WCS_INTERVEC_ITEM_LASTRANGEPT:
							{
							*LngPtr = VE.LastRangePt;
							break;
							}
						case WCS_INTERVEC_ITEM_PTRELATIVE:
							{
							*ShtPtr = VE.PtRelative;
							break;
							}
						case WCS_INTERVEC_ITEM_PTOPERATE:
							{
							*ShtPtr = VE.PtOperate;
							break;
							}
						case WCS_INTERVEC_ITEM_TOPOCONFORM:
							{
							*ShtPtr = VE.TopoConform;
							break;
							}
						case WCS_INTERVEC_ITEM_INSERTSPLINE:
							{
							*ShtPtr = VE.InsertSpline;
							break;
							}
						case WCS_INTERVEC_ITEM_REFCONTROL:
							{
							*ShtPtr = VE.RefControl;
							break;
							}
						case WCS_INTERVEC_ITEM_ARBREFCOORD:
							{
							*DblPtr = VE.ArbRefCoord[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_PROJREFCOORD:
							{
							*DblPtr = VE.ProjRefCoord[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_REFCOORD:
							{
							*DblPtr = VE.RefCoord[Component];
							break;
							}
						case WCS_INTERVEC_ITEM_HORUNITS:
							{
							*ShtPtr = VE.HorUnits;
							break;
							}
						case WCS_INTERVEC_ITEM_VERTUNITS:
							{
							*ShtPtr = VE.VertUnits;
							break;
							}
						case WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT:
							{
							*ShtPtr = VE.ProjRefCoordsFloating;
							break;
							}
						} // switch item
					break;
					} // WCS_INTERVEC_SUBCLASS_VALUE
				} // switch subclass
			break;
			} // WCS_INTERCLASS_VECTOR
		case WCS_INTERCLASS_MISC:
			{
			switch (SubClass)
				{
				case WCS_INTERMISC_SUBCLASS_MISC:
					{
					switch (Item)
						{
						case WCS_INTERMISC_ITEM_PROJFRAMERATE:
							{
							*DblPtr = ProjFrameRate;
							break;
							} // WCS_INTERMISC_ITEM_PROJFRAMERATE
						case WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE:
							{
							*ShtPtr = DigitizeDrawMode;
							break;
							} // WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE
						} // switch
					break;
					} // WCS_INTERMISC_SUBCLASS_MISC
				} // switch
			break;
			} // WCS_INTERCLASS_MISC
		case WCS_INTERCLASS_TIME:
			{
			switch (SubClass)
				{
				case WCS_TIME_SUBCLASS_TIME:
					{
					*DblPtr = ActiveTime;
					break;
					} // time
				case WCS_TIME_SUBCLASS_FRAME:
					{
					*LngPtr = (long)(ActiveTime * (ProjFrameRate > 0.0 ? ProjFrameRate: 30.0) + .5);
					break;
					} // time
				} // switch
//			GlobalApp->UpdateProjectByTime();	// I have no idea why this is here. removed 4/9/00 by GRH
			break;
			} // time
		} // switch ParClass

	} // if

va_end(VarA);

return;

} // InterCommon::GetParam

/*===========================================================================*/

void InterCommon::ClearAllPts(void)
{

if (VE.RestorePts)
	{
	DBHost->MasterPoint.DeAllocate(VE.RestorePts);
	VE.RestorePts = NULL;
	VE.RestoreNumPts = 0;
	} // if
if (VE.UndoPts)
	{
	DBHost->MasterPoint.DeAllocate(VE.UndoPts);
	VE.UndoPts = NULL;
	VE.UndoNumPts = 0;
	} // if
if (VE.RefPts)
	{
	DBHost->MasterPoint.DeAllocate(VE.RefPts);
	VE.RefPts = NULL;
	VE.RefNumPts = 0;
	} // if
VE.SelectedNumPts = 0;
VE.PtOperate = WCS_VECTOR_PTOPERATE_ALLPTS;
VE.FirstRangePt = VE.LastRangePt = VE.InsertAfter = VE.InsertPts = VE.OriginVertex = 1;
VE.InterpPts = 0;
VE.Active = NULL;
VE.ActivePoint = NULL;

} // InterCommon::ClearAllPts

/*===========================================================================*/

void InterCommon::SetNewObj(Joe *NewGuy)
{
VectorPoint *PLinkA, *PLinkB, *PLinkC;
long Ct;

if (VE.Active = NewGuy)
	{
	if (VE.Active->Points())
		{
		if (VE.RestorePts = DBHost->MasterPoint.Allocate(VE.Active->NumPoints()))
			{
			VE.RestoreNumPts = VE.Active->NumPoints();
			if (VE.RefPts = DBHost->MasterPoint.Allocate(VE.Active->NumPoints()))
				{
				VE.RefNumPts = VE.Active->NumPoints();
				for (Ct = 0, PLinkA = VE.Active->Points(), PLinkB = VE.RestorePts, PLinkC = VE.RefPts; PLinkA;
					Ct++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next, PLinkC = PLinkC->Next)
					{
					PLinkB->Latitude = PLinkC->Latitude = PLinkA->Latitude;
					PLinkB->Longitude = PLinkC->Longitude = PLinkA->Longitude;
					PLinkB->Elevation = PLinkC->Elevation = PLinkA->Elevation;
					if (Ct < WCS_DXF_MAX_OBJPTS)
						{
						VE.SelPts[Ct] = PLinkA;
						VE.SelRefPts[Ct] = PLinkC;
						VE.SelPtsIndex[Ct] = (unsigned short)Ct;
						VE.SelectedNumPts++;
						} // if
					} // for
				VE.InterpPts = VE.Active->GetNumRealPoints();
				} // if
			else
				{
				DBHost->MasterPoint.DeAllocate(VE.RestorePts);
				VE.RestorePts = NULL;
				VE.RestoreNumPts = 0;
				} // else
			} // if
		SetActivePoint(VE.Active);
		ComputeReferenceCoords();				
		} // if
	} // if

} // InterCommon::SetNewObj

/*===========================================================================*/

unsigned long InterCommon::GetNumSelectedPoints(void)
{

return(! VE.Active ? 0: (VE.SelectedNumPts == VE.Active->NumPoints() && VE.Active->NumPoints() > 0) ? VE.SelectedNumPts - Joe::GetFirstRealPtNum(): VE.SelectedNumPts);

} // InterCommon::GetNumSelectedPoints

/*===========================================================================*/

void InterCommon::PointsChanged(Joe *Active)
{

if (Active)
	{
	CopyRefToUndo();
	CopyActiveToRef(Active);
	RebuildSelectedList(Active);
	ComputeReferenceCoords();				
	} // if

} // InterCommon::PointsChanged

/*===========================================================================*/

void InterCommon::UndoPoints(Joe *Active)
{

if (VE.UndoPts && Active)
	{
	swmem(&VE.UndoPts, Active->PointsAddr(), sizeof (VectorPoint *));
	swmem(&VE.UndoNumPts, Active->NumPointsAddr(), sizeof (unsigned long));
//	CopyActiveToRef(Active);
//	RebuildSelectedList(Active);
	} // if

} // InterCommon::UndoPoints

/*===========================================================================*/

void InterCommon::RestorePoints(Joe *Active)
{

if (Active)
	{
	CopyRefToUndo();
	CopyRestoreToActive(Active);
//	CopyActiveToRef(Active);
//	RebuildSelectedList(Active);
	} // if

} // InterCommon::RestorePoints

/*===========================================================================*/

int InterCommon::CopyRefToUndo(void)
{
VectorPoint *PLinkA, *PLinkB;

if (VE.UndoNumPts != VE.RefNumPts)
	{
	if (VE.UndoPts)
		{
		DBHost->MasterPoint.DeAllocate(VE.UndoPts);
		VE.UndoPts = NULL;
		VE.UndoNumPts = 0;
		} // if
	} // if
if (VE.RefNumPts > 0)
	{
	if (VE.UndoPts || (VE.UndoPts = DBHost->MasterPoint.Allocate(VE.RefNumPts)))
		{
		VE.UndoNumPts = VE.RefNumPts;
		for (PLinkA = VE.RefPts, PLinkB = VE.UndoPts; PLinkA; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
			{
			PLinkB->Latitude = PLinkA->Latitude;
			PLinkB->Longitude = PLinkA->Longitude;
			PLinkB->Elevation = PLinkA->Elevation;
			} // for
		} // if
	else
		return (0);
	} // if

return (1);

} // InterCommon::CopyRefToUndo

/*===========================================================================*/

int InterCommon::CopyRestoreToActive(Joe *Active)
{
VectorPoint *PLinkA, *PLinkB;

if (VE.RestoreNumPts != Active->NumPoints())
	{
	if (Active->Points())
		{
		DBHost->MasterPoint.DeAllocate(Active->Points());
		Active->Points(NULL);
		Active->NumPoints(0);
		} // if
	} // if
if (VE.RestoreNumPts > 0)
	{
	if (Active->Points() || (Active->Points(DBHost->MasterPoint.Allocate(VE.RestoreNumPts))))
		{
		Active->NumPoints(VE.RestoreNumPts);
		for (PLinkA = VE.RestorePts, PLinkB = Active->Points(); PLinkA; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
			{
			PLinkB->Latitude = PLinkA->Latitude;
			PLinkB->Longitude = PLinkA->Longitude;
			PLinkB->Elevation = PLinkA->Elevation;
			} // for
		} // if
	else
		return (0);
	} // if

return (1);

} // InterCommon::CopyRestoreToActive

/*===========================================================================*/

int InterCommon::CopyActiveToRef(Joe *Active)
{
VectorPoint *PLinkA, *PLinkB;

if (Active->NumPoints() != VE.RefNumPts)
	{
	if (VE.RefPts)
		{
		DBHost->MasterPoint.DeAllocate(VE.RefPts);
		VE.RefPts = NULL;
		VE.RefNumPts = 0;
		} // if
	} // if
if (Active->NumPoints() > 0)
	{
	if (VE.RefPts || (VE.RefPts = DBHost->MasterPoint.Allocate(Active->NumPoints())))
		{
		VE.RefNumPts = Active->NumPoints();
		for (PLinkA = Active->Points(), PLinkB = VE.RefPts; PLinkA; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
			{
			PLinkB->Latitude = PLinkA->Latitude;
			PLinkB->Longitude = PLinkA->Longitude;
			PLinkB->Elevation = PLinkA->Elevation;
			} // for
		} // if
	else
		return (0);
	} // if

return (1);

} // InterCommon::CopyActiveToRef

/*===========================================================================*/

int InterCommon::SetPointSelState(unsigned long Select, int State)
{

if (State)
	{
	return (SelectPoint(VE.Active, Select));
	} // if
else
	{
	return (UnSelectPoint(VE.Active, Select));
	} // else

} // InterCommon::SetPointSelState

/*===========================================================================*/

int InterCommon::SelectPoint(Joe *Active, unsigned long Select)
{
VectorPoint *PLinkA, *PLinkB;
unsigned long InsertPt = 0, Ct = 0, MovePts, Abort = 0;

if (Active && Select < Active->NumPoints())
	{
	if (VE.SelectedNumPts + 1 < WCS_DXF_MAX_OBJPTS)
		{
		while (InsertPt < VE.SelectedNumPts)
			{
			if (VE.SelPtsIndex[InsertPt] == Select)
				{
				Abort = 1;
				break;
				} // if
			if (VE.SelPtsIndex[InsertPt] > Select)
				break;
			InsertPt++; 
			} // while
		if (! Abort)
			{
			MovePts = VE.SelectedNumPts - InsertPt;
			if (MovePts > 0)
				{
				memmove(&VE.SelPts[InsertPt + 1], &VE.SelPts[InsertPt], MovePts * sizeof (VectorPoint *));
				memmove(&VE.SelRefPts[InsertPt + 1], &VE.SelRefPts[InsertPt], MovePts * sizeof (VectorPoint *));
				memmove(&VE.SelPtsIndex[InsertPt + 1], &VE.SelPtsIndex[InsertPt], MovePts * sizeof (unsigned short));
				} // if
			for (Ct = 0, PLinkA = Active->Points(), PLinkB = VE.RefPts; PLinkA && PLinkB && Ct < Select;
				Ct++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next);	//lint !e722
			VE.SelPts[InsertPt] = PLinkA;
			VE.SelRefPts[InsertPt] = PLinkB;
			VE.SelPtsIndex[InsertPt] = (unsigned short)Select;
			VE.SelectedNumPts++;
			return (1);
			} // if
		} // if
	} // if

return (0);

} // InterCommon::SelectPoint

/*===========================================================================*/

int InterCommon::UnSelectPoint(Joe *Active, unsigned long Select)
{
long InsertPt = 0, MovePts, Found = 0;

if (Active && Select < Active->NumPoints())
	{
	if (VE.SelectedNumPts > 0)
		{
		while (InsertPt < (long)VE.SelectedNumPts)
			{
			if (VE.SelPtsIndex[InsertPt] == Select)
				{
				Found = 1;
				break;
				} // if
			InsertPt++; 
			} // while
		if (Found)
			{
			MovePts = VE.SelectedNumPts - InsertPt - 1;
			if (MovePts > 0)
				{
				memmove(&VE.SelPts[InsertPt], &VE.SelPts[InsertPt + 1], MovePts * sizeof (VectorPoint *));
				memmove(&VE.SelRefPts[InsertPt], &VE.SelRefPts[InsertPt + 1], MovePts * sizeof (VectorPoint *));
				memmove(&VE.SelPtsIndex[InsertPt], &VE.SelPtsIndex[InsertPt + 1], MovePts * sizeof (unsigned short));
				} // if
			VE.SelectedNumPts --;
			return (1);
			} // if
		} // if
	} // if

return (0);

} // InterCommon::UnSelectPoint

/*===========================================================================*/

void InterCommon::SelectAllPoints(Joe *Active)
{
VectorPoint *PLinkA, *PLinkB;
unsigned long Ct;

if (Active)
	{
	if (Active->Points())
		{
		if (VE.RefPts)
			{
			VE.SelectedNumPts = 0;
			for (Ct = 0, PLinkA = Active->Points(), PLinkB = VE.RefPts; PLinkB && PLinkA && Ct < WCS_DXF_MAX_OBJPTS;
				Ct++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
				{
				VE.SelPts[Ct] = PLinkA;
				VE.SelRefPts[Ct] = PLinkB;
				VE.SelPtsIndex[Ct] = (unsigned short)Ct;
				VE.SelectedNumPts++;
				} // for
			} // if
		SetActivePoint(Active);
		} // if
	} // if

} // InterCommon::SelectAllPoints

/*===========================================================================*/

void InterCommon::UnSelectAllPoints(void)
{

VE.SelectedNumPts = 0;
VE.ActivePoint = NULL;

} // InterCommon::UnSelectAllPoints

/*===========================================================================*/

void InterCommon::RebuildSelectedList(Joe *Active)
{
VectorPoint *PLinkA, *PLinkB;
unsigned long Ct, IndexCt, Index;

if (Active)
	{
	VE.ActivePoint = NULL;
	for (Ct = 0, IndexCt = 0, Index = VE.SelPtsIndex[0], PLinkA = Active->Points(), PLinkB = VE.RefPts; PLinkA && PLinkB && IndexCt < VE.SelectedNumPts;
		Ct++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
		{
		if (Ct == Index)
			{
			VE.SelPts[IndexCt] = PLinkA;
			VE.SelRefPts[IndexCt] = PLinkB;
			IndexCt++;
			if (IndexCt >= WCS_DXF_MAX_OBJPTS)
				break;
			Index = VE.SelPtsIndex[IndexCt];
			} // if
		} // for
	VE.SelectedNumPts = IndexCt;
	SetActivePoint(Active);
	} // if

} // InterCommon::RebuildSelectedList

/*===========================================================================*/

void InterCommon::SetActivePoint(Joe *Active)
{
VectorPoint *PLinkA;
unsigned long Ct;

if (Active)
	{
	VE.ActivePoint = NULL;
	for (Ct = 0, PLinkA = Active->Points(); PLinkA; Ct++, PLinkA = PLinkA->Next)
		{
		if (Ct == VE.FirstRangePt)
			{
			VE.ActivePoint = PLinkA;
			break;
			} // if
		} // for
	} // if

} // InterCommon::SetActivePoint

/*===========================================================================*/

void InterCommon::SetPointsSelected(void)
{
unsigned long Pt;

if (VE.Active)
	{
	switch (VE.PtOperate)
		{
		case WCS_VECTOR_PTOPERATE_ALLPTS:
			{
			UnSelectAllPoints();
			SelectAllPoints(VE.Active);
			break;
			} // WCS_VECTOR_PTOPERATE_ALLPTS
		case WCS_VECTOR_PTOPERATE_MAPSELECTED:
			{
			// leave point selections as they are
			break;
			} // WCS_VECTOR_PTOPERATE_MAPSELECTED
		case WCS_VECTOR_PTOPERATE_SINGLEPT:
			{
			UnSelectAllPoints();
			if (VE.FirstRangePt >= VE.Active->NumPoints())
				VE.FirstRangePt = VE.Active->GetNumRealPoints() > 0 ? Joe::GetFirstRealPtNum(): 0;
			#ifdef WCS_JOE_LABELPOINTEXISTS
			if (VE.FirstRangePt < 1)
				VE.FirstRangePt = VE.Active->NumPoints() > 0 ? VE.Active->NumPoints() - 1: 0;
			#endif // WCS_JOE_LABELPOINTEXISTS
			VE.LastRangePt = VE.FirstRangePt;
			SelectPoint(VE.Active, VE.FirstRangePt);
			VE.InsertAfter = VE.FirstRangePt > 0 ? VE.FirstRangePt: 1;
			break;
			} // WCS_VECTOR_PTOPERATE_SINGLEPT
		case WCS_VECTOR_PTOPERATE_PTRANGE:
			{
			UnSelectAllPoints();
			if (VE.FirstRangePt >= VE.Active->NumPoints())
				VE.FirstRangePt = VE.Active->GetNumRealPoints() > 0 ? Joe::GetFirstRealPtNum(): 0;
			#ifdef WCS_JOE_LABELPOINTEXISTS
			if (VE.FirstRangePt < 1)
				VE.FirstRangePt = VE.Active->NumPoints() > 0 ? VE.Active->NumPoints() - 1: 0;
			#endif // WCS_JOE_LABELPOINTEXISTS
			if (VE.LastRangePt >= VE.Active->NumPoints())
				VE.LastRangePt = VE.Active->GetNumRealPoints() > 0 ? Joe::GetFirstRealPtNum(): 0;
			#ifdef WCS_JOE_LABELPOINTEXISTS
			if (VE.LastRangePt < 1)
				VE.LastRangePt = VE.Active->NumPoints() > 0 ? VE.Active->NumPoints() - 1: 0;
			#endif // WCS_JOE_LABELPOINTEXISTS
			if (VE.FirstRangePt <= VE.LastRangePt)
				{
				for (Pt = VE.FirstRangePt; Pt <= VE.LastRangePt; Pt++)
					{
					SelectPoint(VE.Active, Pt);
					} // for
				} // if
			else
				{
				for (Pt = VE.LastRangePt; Pt <= VE.FirstRangePt; Pt++)
					{
					SelectPoint(VE.Active, Pt);
					} // for
				} // else
			VE.InsertAfter = VE.FirstRangePt > 0 ? VE.FirstRangePt: Joe::GetFirstRealPtNum();
			break;
			} // WCS_VECTOR_PTOPERATE_PTRANGE
		} // switch
	SetActivePoint(VE.Active);
	} // if

} // InterCommon::SetPointsSelected

/*===========================================================================*/

void InterCommon::GetActivePointCoords(double &Lon, double &Lat, float &Elev)
{

if (VE.ActivePoint)
	{
	Lon = VE.ActivePoint->Longitude;
	Lat = VE.ActivePoint->Latitude;
	Elev = VE.ActivePoint->Elevation;
	} // if
else
	{
	Lon = 0.0;
	Lat = 0.0;
	Elev = 0.0f;
	} // else

} // InterCommon::GetActivePointCoords

/*===========================================================================*/

int InterCommon::GetPointSelState(unsigned long Pt)
{
unsigned long Ct;

if (VE.PtOperate == WCS_VECTOR_PTOPERATE_ALLPTS)
	{
	return (1);
	} // if
else
	{
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		if (VE.SelPtsIndex[Ct] == Pt)
			return (1);
		} // for
	} // else

return (0);

} // InterCommon::GetPointSelState

/*===========================================================================*/

int InterCommon::GetPointSelState(VectorPoint *Pt)
{
unsigned long Ct;

if (VE.PtOperate == WCS_VECTOR_PTOPERATE_ALLPTS)
	{
	return (1);
	} // if
else
	{
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		if (VE.SelPts[Ct] == Pt)
			return (1);
		} // for
	} // else

return (0);

} // InterCommon::GetPointSelState

/*===========================================================================*/

void InterCommon::ChangePointLat(double NewLat)
{
unsigned long Ct;
double LatShift, TempLat;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

if (VE.Active && VE.ActivePoint)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	VE.ActivePoint->ProjToDefDeg(MyCoords, &Vert);
	LatShift = NewLat - Vert.Lat;
	//LatShift = NewLat - VE.ActivePoint->Latitude;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelRefPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempLat = Vert.Lat + LatShift;
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		Vert.Lat = TempLat;
		VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
		//VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude + LatShift;
		} // if
	} // if

} // InterCommon::ChangePointLat

/*===========================================================================*/

void InterCommon::ChangePointLon(double NewLon)
{
unsigned long Ct;
double LonShift, TempLon;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

if (VE.Active && VE.ActivePoint)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	VE.ActivePoint->ProjToDefDeg(MyCoords, &Vert);
	LonShift = NewLon - Vert.Lon;
	//LonShift = NewLon - VE.ActivePoint->Longitude;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelRefPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempLon = Vert.Lon + LonShift;
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		Vert.Lon = TempLon;
		VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
		//VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude + LonShift;
		} // if
	} // if

} // InterCommon::ChangePointLon

/*===========================================================================*/

void InterCommon::ChangePointElev(float NewElev)
{
unsigned long Ct;
float ElevShift, TempElev;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

if (VE.ActivePoint)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	VE.ActivePoint->ProjToDefDeg(MyCoords, &Vert);
	ElevShift = NewElev - (float)Vert.Elev;
	//ElevShift = NewElev - VE.ActivePoint->Elevation;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelRefPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempElev = (float)Vert.Elev + ElevShift;
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		Vert.Elev = TempElev;
		VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
		//VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation + ElevShift;
		} // if
	} // if

} // InterCommon::ChangePointElev

/*===========================================================================*/

void InterCommon::ScaleRotateShift(double PlanetRad, double LocalLonScale, double LocalLatScale, double ElevScale, double RotateDeg,
	double LonShift, double LatShift, float ElevShift, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;
Matx3x3 RotMatx;
struct coords PP;

if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	BuildRotationMatrix(0.0, 0.0, -RotateDeg * PiOver180, RotMatx);
	ConvertDegToDist(PlanetRad, LonShift);
	ConvertDegToDist(PlanetRad, LatShift);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);

		if (VE.Scale[WCS_INTERVEC_COMP_X])
			VE.SelPts[Ct]->Longitude *= LocalLonScale;
		if (VE.Scale[WCS_INTERVEC_COMP_Y] || (VE.Scale[WCS_INTERVEC_COMP_X] && VE.PreserveXY))
			{
			if (VE.PreserveXY)
				VE.SelPts[Ct]->Latitude *= LocalLonScale;
			else
				VE.SelPts[Ct]->Latitude *= LocalLatScale;
			} // if
		if (VE.Scale[WCS_INTERVEC_COMP_Z])
			VE.SelPts[Ct]->Elevation = (float)(VE.SelPts[Ct]->Elevation * ElevScale);

		if (VE.Rotate)
			{
			PP.y = VE.SelPts[Ct]->Latitude;
			PP.x = VE.SelPts[Ct]->Longitude;
			PP.z = VE.SelPts[Ct]->Elevation;
			RotatePoint(&PP, RotMatx);
			VE.SelPts[Ct]->Latitude = PP.y;
			VE.SelPts[Ct]->Longitude = PP.x;
			} // if

		if (VE.Shift[WCS_INTERVEC_COMP_X])
			{
			VE.SelPts[Ct]->Longitude += LonShift;
			} // if
		if (VE.Shift[WCS_INTERVEC_COMP_Y])
			{
			VE.SelPts[Ct]->Latitude += LatShift;
			} // if
		if (VE.Shift[WCS_INTERVEC_COMP_Z])
			VE.SelPts[Ct]->Elevation += ElevShift;

		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ScaleRotateShift

/*===========================================================================*/

void InterCommon::ShiftPointLat(double PlanetRad, double LatShift, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	ConvertDegToDist(PlanetRad, LatShift);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Latitude += LatShift;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ShiftPointLat

/*===========================================================================*/

void InterCommon::ShiftPointLon(double PlanetRad, double LonShift, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	ConvertDegToDist(PlanetRad, LonShift);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Longitude += LonShift;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ShiftPointLon

/*===========================================================================*/

void InterCommon::ShiftPointLonLat(double PlanetRad, double LonShift, double LatShift, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

//if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	ConvertDegToDist(PlanetRad, LonShift);
	ConvertDegToDist(PlanetRad, LatShift);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Longitude += LonShift;
		VE.SelPts[Ct]->Latitude += LatShift;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ShiftPointLonLat

/*===========================================================================*/

void InterCommon::ShiftPointLonLatElev(double PlanetRad, double LonShift, double LatShift, double Elev, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

//if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	ConvertDegToDist(PlanetRad, LonShift);
	ConvertDegToDist(PlanetRad, LatShift);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		//VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		VE.SelPts[Ct]->Elevation = (float)(VE.SelRefPts[Ct]->Elevation + Elev);
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Longitude += LonShift;
		VE.SelPts[Ct]->Latitude += LatShift;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ShiftPointLonLatElev

/*===========================================================================*/

void InterCommon::ShiftPointElev(float ElevShift)
{
unsigned long Ct;
float TempElev;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

if (VE.Active && VE.ActivePoint)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelRefPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempElev = (float)Vert.Elev + ElevShift;
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		Vert.Elev = TempElev;
		VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
		//VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation + ElevShift;
		} // if
	} // if

} // InterCommon::ShiftPointElev

/*===========================================================================*/

void InterCommon::ScalePointLat(double PlanetRad, double LocalLatScale, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

if (VE.ActivePoint && LocalLatScale != 0.0)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Latitude *= LocalLatScale;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ScalePointLat

/*===========================================================================*/

void InterCommon::ScalePointLon(double PlanetRad, double LocalLonScale, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

if (VE.ActivePoint && LocalLonScale != 0.0)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Longitude *= LocalLonScale;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ScalePointLon

/*===========================================================================*/

void InterCommon::ScalePointLonLat(double PlanetRad, double LocalLonScale, double LocalLatScale, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;

if (/*VE.ActivePoint && */LocalLonScale != 0.0 && LocalLatScale != 0.0)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->Latitude = VE.SelRefPts[Ct]->Latitude;
		VE.SelPts[Ct]->Longitude = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Longitude *= LocalLonScale;
		VE.SelPts[Ct]->Latitude *= LocalLatScale;
		ConvertPointFromXYZ(PlanetRad, VE.SelPts[Ct]->Longitude, VE.SelPts[Ct]->Latitude, VE.SelPts[Ct]->Elevation, TRUE);
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::ScalePointLonLat

/*===========================================================================*/

void InterCommon::ScalePointElev(double ElevScale)
{
unsigned long Ct;
float TempElev;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

if (VE.Active && VE.ActivePoint)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelRefPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempElev = (float)(VE.RefCoord[WCS_INTERVEC_COMP_Z] + (Vert.Elev - VE.RefCoord[WCS_INTERVEC_COMP_Z]) * ElevScale);
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		Vert.Elev = TempElev;
		VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
		//VE.SelPts[Ct]->Elevation = (float)(VE.RefCoord[WCS_INTERVEC_COMP_Z] + (VE.SelRefPts[Ct]->Elevation - VE.RefCoord[WCS_INTERVEC_COMP_Z]) * ElevScale);
		} // if
	} // if

} // InterCommon::ScalePointElev

/*===========================================================================*/

void InterCommon::RotatePointDegXY(double PlanetRad, double RotateDeg, short UseDefaultUnits)
{
unsigned long Ct;
short OldHorUnits;
Matx3x3 RotMatx;
struct coords PP;

//if (VE.ActivePoint)
	{
	OldHorUnits = VE.HorUnits;
	if (UseDefaultUnits)
		VE.HorUnits = WCS_VECTOR_HUNITS_KILOMETERS;
	BuildRotationMatrix(0.0, 0.0, -RotateDeg * PiOver180, RotMatx);
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		PP.y = VE.SelRefPts[Ct]->Latitude;
		PP.x = VE.SelRefPts[Ct]->Longitude;
		VE.SelPts[Ct]->Elevation = VE.SelRefPts[Ct]->Elevation;
		ConvertPointToXYZ(PlanetRad, PP.x, PP.y, VE.SelPts[Ct]->Elevation, TRUE);
		PP.z = VE.SelPts[Ct]->Elevation;
		RotatePoint(&PP, RotMatx);
		ConvertPointFromXYZ(PlanetRad, PP.x, PP.y, VE.SelPts[Ct]->Elevation, TRUE);
		VE.SelPts[Ct]->Latitude = PP.y;
		VE.SelPts[Ct]->Longitude = PP.x;
		//if (VE.SelPts[Ct]->Latitude > 90.0)
		//	VE.SelPts[Ct]->Latitude = 90.0;
		//else if (VE.SelPts[Ct]->Latitude < -90.0)
		//	VE.SelPts[Ct]->Latitude = -90.0;
		} // if
	VE.HorUnits = OldHorUnits;
	} // if

} // InterCommon::RotatePointDegXY

/*===========================================================================*/

void InterCommon::ComputeReferenceCoords(void)
{
float MaxEl, MinEl;
unsigned long CoordSet = 0;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

switch (VE.RefControl)
	{
	case WCS_VECTOR_REFERENCE_GEOCOORD:
		{
		VE.RefCoord[WCS_INTERVEC_COMP_X] = 0.0;
		VE.RefCoord[WCS_INTERVEC_COMP_Y] = 0.0;
		VE.RefCoord[WCS_INTERVEC_COMP_Z] = 0.0;
		CoordSet = 1;
		break;
		} // WCS_VECTOR_REFERENCE_GEOCOORD
	case WCS_VECTOR_REFERENCE_PROJCOORD:
		{
		VE.RefCoord[WCS_INTERVEC_COMP_X] = VE.ProjRefCoord[WCS_INTERVEC_COMP_X];
		VE.RefCoord[WCS_INTERVEC_COMP_Y] = VE.ProjRefCoord[WCS_INTERVEC_COMP_Y];
		VE.RefCoord[WCS_INTERVEC_COMP_Z] = VE.ProjRefCoord[WCS_INTERVEC_COMP_Z];
		CoordSet = 1;
		break;
		} // WCS_VECTOR_REFERENCE_PROJCOORD
	case WCS_VECTOR_REFERENCE_ARBCOORD:
		{
		VE.RefCoord[WCS_INTERVEC_COMP_X] = VE.ArbRefCoord[WCS_INTERVEC_COMP_X];
		VE.RefCoord[WCS_INTERVEC_COMP_Y] = VE.ArbRefCoord[WCS_INTERVEC_COMP_Y];
		VE.RefCoord[WCS_INTERVEC_COMP_Z] = VE.ArbRefCoord[WCS_INTERVEC_COMP_Z];
		CoordSet = 1;
		break;
		} // WCS_VECTOR_REFERENCE_ARBCOORD
	case WCS_VECTOR_REFERENCE_VECORIGIN:
		{
		if (VE.Active && VE.Active->GetFirstRealPoint())
			{
			if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				MyCoords = MyAttr->Coord;
			else
				MyCoords = NULL;
			VE.Active->GetFirstRealPoint()->ProjToDefDeg(MyCoords, &Vert);
			VE.RefCoord[WCS_INTERVEC_COMP_X] = Vert.Lon;
			VE.RefCoord[WCS_INTERVEC_COMP_Y] = Vert.Lat;
			VE.RefCoord[WCS_INTERVEC_COMP_Z] = Vert.Elev;
			CoordSet = 1;
			} // if
		break;
		} // WCS_VECTOR_REFERENCE_VECORIGIN
	case WCS_VECTOR_REFERENCE_VECCENTER:
		{
		if (VE.Active && VE.Active->GetFirstRealPoint())
			{
			VE.RefCoord[WCS_INTERVEC_COMP_X] = (VE.Active->NWLon + VE.Active->SELon) * 0.5;
			VE.RefCoord[WCS_INTERVEC_COMP_Y] = (VE.Active->NWLat + VE.Active->SELat) * 0.5;
			VE.Active->GetElevRange(MaxEl, MinEl);
			VE.RefCoord[WCS_INTERVEC_COMP_Z] = (MaxEl + MinEl) * 0.5;
			CoordSet = 1;
			} // if
		break;
		} // WCS_VECTOR_REFERENCE_VECCENTER
	case WCS_VECTOR_REFERENCE_ACTIVEPT:
		{
		if (VE.Active && VE.ActivePoint)
			{
			if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				MyCoords = MyAttr->Coord;
			else
				MyCoords = NULL;
			VE.ActivePoint->ProjToDefDeg(MyCoords, &Vert);
			VE.RefCoord[WCS_INTERVEC_COMP_X] = Vert.Lon;
			VE.RefCoord[WCS_INTERVEC_COMP_Y] = Vert.Lat;
			VE.RefCoord[WCS_INTERVEC_COMP_Z] = Vert.Elev;
			CoordSet = 1;
			} // if
		break;
		} // WCS_VECTOR_REFERENCE_ACTIVEPT
	case WCS_VECTOR_REFERENCE_SELPTAVG:
		{
		if (VE.Active)
			{
			if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				MyCoords = MyAttr->Coord;
			else
				MyCoords = NULL;
			VE.RefCoord[WCS_INTERVEC_COMP_X] = 0.0;
			VE.RefCoord[WCS_INTERVEC_COMP_Y] = 0.0;
			VE.RefCoord[WCS_INTERVEC_COMP_Z] = 0.0;
			for (CoordSet = 0; CoordSet < VE.SelectedNumPts; CoordSet++)
				{
				VE.SelPts[CoordSet]->ProjToDefDeg(MyCoords, &Vert);
				VE.RefCoord[WCS_INTERVEC_COMP_X] += Vert.Lon;
				VE.RefCoord[WCS_INTERVEC_COMP_Y] += Vert.Lat;
				VE.RefCoord[WCS_INTERVEC_COMP_Z] += Vert.Elev;
				} // if
			if (CoordSet)
				{
				VE.RefCoord[WCS_INTERVEC_COMP_X] /= CoordSet;
				VE.RefCoord[WCS_INTERVEC_COMP_Y] /= CoordSet;
				VE.RefCoord[WCS_INTERVEC_COMP_Z] /= CoordSet;
				} // if
			} // if
		break;
		} // WCS_VECTOR_REFERENCE_SELPTAVG
	} // switch

if (! CoordSet)
	{
	VE.RefCoord[WCS_INTERVEC_COMP_X] = 0.0;
	VE.RefCoord[WCS_INTERVEC_COMP_Y] = 0.0;
	VE.RefCoord[WCS_INTERVEC_COMP_Z] = 0.0;
	} // if

} // InterCommon::ComputeReferenceCoords

/*===========================================================================*/

void InterCommon::ConvertPointToXYZ(double PlanetRad, double &X, double &Y, float &Z, short Relative)
{
double Lat;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

// note that X and Y are always supplied in the vector's projection, not necessarily degrees.
// outgoing values will be in units of meters since that is how GlobalApp->AppEffects->GetPlanetRadius() is defined.
if (VE.Active)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	if (MyCoords)
		{
		Vert.xyz[0] = X;
		Vert.xyz[1] = Y;
		Vert.xyz[2] = Z;
		if (MyCoords->ProjToDefDeg(&Vert))
			{
			X = Vert.Lon;
			Y = Lat = Vert.Lat;
			Z = (float)Vert.Elev;
			Lat = Vert.Lat;
			} // if
		else
			return;
		} // if
	else
		Lat = Y;
	// we are all in lat/lon/elev
	if (Relative)
		{
		X -= VE.RefCoord[WCS_INTERVEC_COMP_X];
		Y -= VE.RefCoord[WCS_INTERVEC_COMP_Y];
		Z = (Z - (float)VE.RefCoord[WCS_INTERVEC_COMP_Z]);
		} // if
	if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
		{
		if (Lat > 90.0)
			Lat = 90.0;
		else if (Lat < -90.0)
			Lat = -90.0;
		X = X * PlanetRad * PiOver180 * cos(Lat * PiOver180);// * VE.HUnitScale;
		Y = Y * PlanetRad * PiOver180;// * VE.HUnitScale;
		} // if
	//Z = (float)(Z * VE.VUnitScale);	// vertical unit scale now obsolete
	} // if

} // InterCommon::ConvertPointToXYZ

/*===========================================================================*/

void InterCommon::ConvertPointFromXYZ(double PlanetRad, double &X, double &Y, float &Z, short Relative)
{
double Lat;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;

// incoming values are in meters because that is how GlobalApp->AppEffects->GetPlanetRadius() is defined.
// outgoing values must be in vector's projection, not necessarily degrees.

if (VE.Active)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	// convert XYZ back to degrees, test latitude for range
	//Z = (float)(Z / VE.VUnitScale);	// vertical unit scale now obsolete
	if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
		{
		Y /= (PlanetRad * PiOver180);// * VE.HUnitScale);
		Lat = Relative ? Y + VE.RefCoord[WCS_INTERVEC_COMP_Y]: Y;
		if (Lat > 90.0)
			Lat = 90.0;
		else if (Lat < -90.0)
			Lat = -90.0;
		X /= (PlanetRad * PiOver180 * cos(Lat * PiOver180));// * VE.HUnitScale);
		} // if
	if (Relative)
		{
		Z = Z + (float)VE.RefCoord[WCS_INTERVEC_COMP_Z];
		Y += VE.RefCoord[WCS_INTERVEC_COMP_Y];
		X += VE.RefCoord[WCS_INTERVEC_COMP_X];
		} // if
	if (Y > 90.0)
		Y = 90.0;
	else if (Y < -90.0)
		Y = -90.0;
	// convert back to projection
	if (MyCoords)
		{
		Vert.Lon = X;
		Vert.Lat = Y;
		Vert.Elev = Z;
		if (MyCoords->DefDegToProj(&Vert))
			{
			X = Vert.xyz[0];
			Y = Vert.xyz[1];
			Z = (float)Vert.xyz[2];
			} // if
		} // if
	} // if

} // InterCommon::ConvertPointFromXYZ

/*===========================================================================*/

void VectorEdit::SetHUnitScale(void)
{

switch (HorUnits)
	{
	case WCS_VECTOR_HUNITS_DEGREES:
		{
		HUnitScale = ELSCALE_KILOM / EARTHLATSCALE;
		break;
		} // 
	case WCS_VECTOR_HUNITS_KILOMETERS:
		{
		HUnitScale = 1.0;
		break;
		} // 
	case WCS_VECTOR_HUNITS_METERS:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_METERS;
		break;
		} // 
	case WCS_VECTOR_HUNITS_DECIMETERS:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_DECIM;
		break;
		} // 
	case WCS_VECTOR_HUNITS_CENTIMETERS:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_CENTIM;
		break;
		} // 
	case WCS_VECTOR_HUNITS_MILLIMETERS:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_MILLIM;
		break;
		} // 
	case WCS_VECTOR_HUNITS_MILES:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_MILES;
		break;
		} // 
	case WCS_VECTOR_HUNITS_YARDS:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_YARDS;
		break;
		} // 
	case WCS_VECTOR_HUNITS_FEET:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_FEET;
		break;
		} // 
	case WCS_VECTOR_HUNITS_INCHES:
		{
		HUnitScale = ELSCALE_KILOM / ELSCALE_INCHES;
		break;
		} // 
	} // switch

} // VectorEdit::SetHUnitScale

/*===========================================================================*/

void VectorEdit::SetVUnitScale(void)
{

switch (VertUnits)
	{
	case WCS_VECTOR_VUNITS_KILOMETERS:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_KILOM;
		break;
		} // 
	case WCS_VECTOR_VUNITS_METERS:
		{
		VUnitScale = 1.0;
		break;
		} // 
	case WCS_VECTOR_VUNITS_DECIMETERS:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_DECIM;
		break;
		} // 
	case WCS_VECTOR_VUNITS_CENTIMETERS:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_CENTIM;
		break;
		} // 
	case WCS_VECTOR_VUNITS_MILLIMETERS:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_MILLIM;
		break;
		} // 
	case WCS_VECTOR_VUNITS_MILES:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_MILES;
		break;
		} // 
	case WCS_VECTOR_VUNITS_YARDS:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_YARDS;
		break;
		} // 
	case WCS_VECTOR_VUNITS_FEET:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_FEET;
		break;
		} // 
	case WCS_VECTOR_VUNITS_INCHES:
		{
		VUnitScale = ELSCALE_METERS / ELSCALE_INCHES;
		break;
		} // 
	} // switch

} // VectorEdit::SetVUnitScale

/*===========================================================================*/

void InterCommon::ConvertDegToDist(double PlanetRad, double &Y)
{

if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
	{
	// convert to meters
	Y *= (PlanetRad) * PiOver180;// * VE.HUnitScale);
	} // if

} // InterCommon::ConvertDegToDist

/*===========================================================================*/

void InterCommon::ConvertDegToDist(double PlanetRad, double Lat, double &X)
{

if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
	X *= (PlanetRad * cos(Lat * PiOver180) * PiOver180);// * VE.HUnitScale);

} // InterCommon::ConvertDegToDist

/*===========================================================================*/

void InterCommon::ConvertDistToDeg(double PlanetRad, double &Y)
{

if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
	Y /= (PlanetRad * PiOver180);// * VE.HUnitScale);

} // InterCommon::ConvertDistToDeg

/*===========================================================================*/

void InterCommon::ConvertDistToDeg(double PlanetRad, double Lat, double &X)
{

if (VE.HorUnits != WCS_VECTOR_HUNITS_DEGREES)
	X /= (PlanetRad * cos(Lat * PiOver180) * PiOver180);// * VE.HUnitScale);

} // InterCommon::ConvertDistToDeg

/*===========================================================================*/

void InterCommon::ConvertDistToDegNoOptions(double PlanetRad, double &Y)
{

Y /= (PlanetRad * PiOver180);// * VE.HUnitScale);

} // InterCommon::ConvertDistToDeg

/*===========================================================================*/

void InterCommon::ConvertDistToDegNoOptions(double PlanetRad, double Lat, double &X)
{

X /= (PlanetRad * cos(Lat * PiOver180) * PiOver180);// * VE.HUnitScale);

} // InterCommon::ConvertDistToDeg

/*===========================================================================*/

void InterCommon::ConvertElev(float &Z)
{

//Z = (float)(Z * VE.VUnitScale);	// vertical unit scale now obsolete as is this whole function

} // InterCommon::ConvertElev

/*===========================================================================*/

void InterCommon::UnConvertElev(float &Z)
{

//Z = (float)(Z / VE.VUnitScale);	// vertical unit scale now obsolete as is this whole function

} // InterCommon::UnConvertElev

/*===========================================================================*/

double InterCommon::ElevationPoint(double Lat, double Lon)
{

if (FindAndLoadDEM(Lat, Lon))
	{
	return (ActiveDEM->ElScale() * ActiveDEM->DEMArrayPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon) / ELSCALE_METERS);
	} // if DEM found and loaded

return(0.0);

} // InterCommon::ElevationPoint()

/*===========================================================================*/

double InterCommon::ElevationPointNULLReject(double Lat, double Lon, int &Reject)
{
double Elev = 0.0;

Reject = 1;
if (FindAndLoadDEM(Lat, Lon))
	{
	double temp;

	temp = ActiveDEM->ElScale() / ELSCALE_METERS;
	Elev = ActiveDEM->DEMArrayPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon);
	Reject = 0;
	Elev *= temp;
	} // if DEM found and loaded

return(Elev);

} // InterCommon::ElevationPointNULLReject()

/*===========================================================================*/

/***

int UnloadCount;

double InterCommon::SQElevationPointNULLReject(double Lat, double Lon, int &Reject, Joe **ApprovedList)
{
double Elev;
Joe **Current;

Reject = 0;

// if there's something there, it's already approved
if (ActiveDEM && ActiveDEM->Map() && ActiveDEM->MoJoe)
	{
	if (ActiveDEM->GeographicPointContained(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon))
		{
		Elev = ActiveDEM->DEMArrayPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon);
		if (ActiveDEM->GetNullReject() && fabs(Elev - (double)ActiveDEM->NullValue()) < .1)
			{
			;	// do nothing
			} // if
		else
			return (ActiveDEM->ElScale() * Elev / ELSCALE_METERS);
		} // if DEM found and loaded
	}
else
	{
	if (ActiveDEM)
		{
		delete ActiveDEM;
		UnloadCount++;
		} // if
	ActiveDEM = new DEM;
	} // if

// try to find the right one
if (ActiveDEM)
	{
	Current = ApprovedList;
	do
		{
		if (Lat >= (*Current)->SELat && Lat <= (*Current)->NWLat && Lon >= (*Current)->SELon && Lon <= (*Current)->NWLon)
			{
			if (ActiveDEM->AttemptLoadDEM(*Current, 1, ProjHost))
				{
				if (ActiveDEM->GeographicPointContained(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon))
					{
					Elev = ActiveDEM->DEMArrayPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon);
					if (ActiveDEM->GetNullReject() && fabs(Elev - (double)ActiveDEM->NullValue()) < .1)
						{
						;	// do nothing
						} // if
					else
						return (ActiveDEM->ElScale() * Elev / ELSCALE_METERS);
					} // if DEM found and loaded
				} // if
			} // if
		Current++;	// fetch next approved pointer
		} while (*Current);
	} // if

Reject = 1;
return (NULL);

} // InterCommon::SQElevationPointNULLReject()
***/

// Just see if there's an actual elevation in the ActiveDEM.  Used by DEM Merger
double InterCommon::SQElevationPointNULLReject(double Lat, double Lon, int &Reject, CoordSys *TestSys)
{
double Elev = 0.0;

// if there's something there, it's already approved
Reject = 1;
if (ActiveDEM && ActiveDEM->Map() && ActiveDEM->MoJoe)
	{
	if (ActiveDEM->GeographicPointContained(TestSys, Lat, Lon, TRUE))
		{
		double temp;

		temp = ActiveDEM->ElScale() / ELSCALE_METERS;
		Elev = ActiveDEM->DEMArrayPointExtract2(TestSys, Lat, Lon);
		if (ActiveDEM->NullReject && (Elev == ActiveDEM->NullValue()))
			Reject = 1;
		else
			Reject = 0;
		Elev *= temp;
		} // if
	} // if

return(Elev);

} // InterCommon::SQElevationPointNULLReject()

/*===========================================================================*/

double InterCommon::RelativeElevationPoint(double Lat, double Lon)
{

if (FindAndLoadDEM(Lat, Lon))
	{
	return (ActiveDEM->DEMRelElPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon));
	} // if DEM found and loaded

return(0.0);

} // InterCommon::RelativeElevationPoint()

/*===========================================================================*/

int InterCommon::VertexDataPoint(RenderData *Rend, VertexData *Vert, PolygonData *Poly, unsigned long Flags)
{
double PolySide[2][3], CellSizeNS, CellSizeWE, MeasureLonScale;
int LonSign, LatSign;

if (FindAndLoadDEM(Vert->Lat, Vert->Lon))
	{
	Vert->Displacement = 0.0;
	if (Flags & WCS_VERTEXDATA_FLAG_ELEVATION)
		{
		Vert->Elev = (ActiveDEM->ElScale() * ActiveDEM->DEMArrayPointExtract(Rend->DefCoords, Vert->Lat, Vert->Lon) / ELSCALE_METERS);
		if (ActiveDEM->GetNullReject() && fabs(Vert->Elev - (double)ActiveDEM->NullValue()) < .1)
			return (0);
		if (Rend)
			Vert->Elev = Rend->ElevDatum + (Vert->Elev - Rend->ElevDatum) * Rend->Exageration;
		} // if elevation
	if (Flags & WCS_VERTEXDATA_FLAG_RELEL)
		{
		Vert->RelEl = (float)ActiveDEM->DEMRelElPointExtract(GlobalApp->AppEffects->FetchDefaultCoordSys(), Vert->Lat, Vert->Lon);
		} // if relative elevation

	// apply terraffectors
	if (Flags & WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED)
		{
		Rend->EffectsBase->EvalRasterTAsNoInit(Rend, Vert, 0);
		} // if
	if (Flags & WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED)
		{
		Rend->EffectsBase->EvalRasterTAsNoInit(Rend, Vert, 1);
		} // if
	if (Flags & WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED)
		{
		Rend->EffectsBase->EvalTerraffectorsNoInit(Rend, Vert);
		} // if

	// slope, aspect and normal
	if ((Flags & WCS_VERTEXDATA_FLAG_SLOPE) || (Flags & WCS_VERTEXDATA_FLAG_ASPECT) || (Flags & WCS_VERTEXDATA_FLAG_NORMAL))
		{
		VertexData EastVert, NorthVert, *Vtx[2];
		// find two more vertices to use for calculation
		// DEM LatStep, LonStep may not be in degrees
		ActiveDEM->GetDEMCellSizeMeters(CellSizeNS, CellSizeWE);
		MeasureLonScale = cos(((ActiveDEM->MoJoe->NWLat + ActiveDEM->MoJoe->SELat) * .5) * PiOver180);
		CellSizeNS /= (Rend->EarthLatScaleMeters);
		CellSizeWE /= (Rend->EarthLatScaleMeters * MeasureLonScale);

		if (ActiveDEM->GeographicPointContained(Rend->DefCoords, Vert->Lat, Vert->Lon - CellSizeWE * .25, TRUE))
			{
			EastVert.Lon = Vert->Lon - CellSizeWE * .25;	// .25 is arbitrary - decrease it for a more resolution
			LonSign = 1;
			} // if
		else
			{
			EastVert.Lon = Vert->Lon + CellSizeWE * .25;
			LonSign = -1;
			} // else
		EastVert.Lat = Vert->Lat;
		if (ActiveDEM->GeographicPointContained(Rend->DefCoords, Vert->Lat + CellSizeNS * .25, Vert->Lon, TRUE))
			{
			NorthVert.Lat = Vert->Lat + CellSizeNS * .25;
			LatSign = 1;
			} // if
		else
			{
			NorthVert.Lat = Vert->Lat - CellSizeNS * .25;
			LatSign = -1;
			} // else
		NorthVert.Lon = Vert->Lon;
		EastVert.Elev = (ActiveDEM->ElScale() * ActiveDEM->DEMArrayPointExtract(Rend->DefCoords, EastVert.Lat, EastVert.Lon) / ELSCALE_METERS);
		NorthVert.Elev = (ActiveDEM->ElScale() * ActiveDEM->DEMArrayPointExtract(Rend->DefCoords, NorthVert.Lat, NorthVert.Lon) / ELSCALE_METERS);
		if (Rend)
			{
			EastVert.Elev = Rend->ElevDatum + (EastVert.Elev - Rend->ElevDatum) * Rend->Exageration;
			NorthVert.Elev = Rend->ElevDatum + (NorthVert.Elev - Rend->ElevDatum) * Rend->Exageration;
			} // if

		// apply terraffectors to two vertices
		if (Flags & WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED)
			{
			Rend->EffectsBase->EvalRasterTAsNoInit(Rend, &EastVert, 0);
			Rend->EffectsBase->EvalRasterTAsNoInit(Rend, &NorthVert, 0);
			} // if
		if (Flags & WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED)
			{
			Rend->EffectsBase->EvalRasterTAsNoInit(Rend, &EastVert, 1);
			Rend->EffectsBase->EvalRasterTAsNoInit(Rend, &NorthVert, 1);
			} // if
		if (Flags & WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED)
			{
			Rend->EffectsBase->EvalTerraffectorsNoInit(Rend, &EastVert);
			Rend->EffectsBase->EvalTerraffectorsNoInit(Rend, &NorthVert);
			} // if
		// transform coords
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(Vert);
		Rend->DefCoords->DegToCart(&EastVert);
		Rend->DefCoords->DegToCart(&NorthVert);
		#else // WCS_BUILD_VNS
		Vert->DegToCart(Rend->PlanetRad);
		EastVert.DegToCart(Rend->PlanetRad);
		NorthVert.DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS

		if (LonSign == LatSign)
			{
			Vtx[0] = &EastVert;
			Vtx[1] = &NorthVert;
			} // if
		else
			{
			Vtx[0] = &NorthVert;
			Vtx[1] = &EastVert;
			} // else

		// compute surface normal
		FindPosVector(PolySide[0], Vtx[0]->XYZ, Vert->XYZ);
		FindPosVector(PolySide[1], Vtx[1]->XYZ, Vert->XYZ);
		SurfaceNormal(Poly->Normal, PolySide[1], PolySide[0]);

		// compute slope
		if (Flags & WCS_VERTEXDATA_FLAG_SLOPE)
			{
			Poly->Slope = acos(VectorAngle(Vert->XYZ, Poly->Normal)) / PiOver180;	// in degrees
			if (Poly->Slope < 0.0)
				Poly->Slope = 0.0;
			if (Poly->Slope > 90.0)
				Poly->Slope = 90.0;
			} // if

		// compute aspect in degrees and radians
		if (Flags & WCS_VERTEXDATA_FLAG_ASPECT)
			{
			NorthVert.XYZ[0] = Poly->Normal[0];
			NorthVert.XYZ[1] = Poly->Normal[1];
			NorthVert.XYZ[2] = Poly->Normal[2];
			NorthVert.RotateY(-Vert->Lon);
			NorthVert.RotateX(90.0 - Vert->Lat);
			Poly->Aspect = NorthVert.FindRoughAngleYfromZ();
			} // if
		} // if

	// lakes, streams and waves
	if ((Flags & WCS_VERTEXDATA_FLAG_WATERELEV) || (Flags & WCS_VERTEXDATA_FLAG_WATERDEPTH))
		{
		if (Flags & WCS_VERTEXDATA_FLAG_LAKEAPPLIED)
			{
			Rend->EffectsBase->EvalLakesNoInit(Rend, Vert);
			} // if
		if (Flags & WCS_VERTEXDATA_FLAG_STREAMAPPLIED)
			{
			Rend->EffectsBase->EvalStreamsNoInit(Rend, Vert);
			} // if
		if (Flags & WCS_VERTEXDATA_FLAG_WATERDEPTH)
			{
			Vert->WaterDepth = Vert->WaterElev - Vert->Elev;
			} // if
		if (Flags & WCS_VERTEXDATA_FLAG_WAVEHEIGHT)
			{
			} // if
		} // if
	return (1);
	} // if DEM found and loaded

return (0);

} // InterCommon::VertexDataPoint()

/*===========================================================================*/

void InterCommon::DrillDownPoint(double Lat, double Lon)
{
SearchQuery Query;
Joe *CurJoe;
JoeList *JL = NULL, *CurJL, **CurJLPtr;

CurJLPtr = &JL;

Query.Filters->PassDEM = 0;
Query.Filters->PassDisabled = 0;
Query.Filters->GeoPtContained = 1;
Query.Filters->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(Lat);
Query.Filters->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(Lon);

for (CurJoe = GlobalApp->AppDB->GetFirst(); CurJoe; CurJoe = GlobalApp->AppDB->GetNext(CurJoe))
	{
	if (Query.ApproveJoe(CurJoe))
		{
		// add joe to list
		if (*CurJLPtr = new JoeList)
			{
			CurJL = *CurJLPtr;
			CurJL->Me = CurJoe;
			CurJLPtr = &CurJL->Next;
			} // if
		} // if
	} // for

//if (JL)
	{
	if (GlobalApp->GUIWins->DRL)
		{
		GlobalApp->GUIWins->DRL->SetNewJoeList(JL);
		} // if
	else
		{
		GlobalApp->GUIWins->DRL = new DrillDownInfoGUI(GlobalApp->AppEffects, GlobalApp->AppDB, JL);
		if (GlobalApp->GUIWins->DRL)
			{
			GlobalApp->GUIWins->DRL->Open(GlobalApp->MainProj);
			}
		}

	} // if
/*
// JoeList is displayed and deleted in the DrillDownInfoGUI

CurJL = JL;
while (CurJL)
	{
	UserMessageOK((char *)CurJL->Me->GetBestName(), "Found this vector surrounding the point in question.");
	CurJL = CurJL->Next;
	} // while

while (JL)
	{
	CurJL = JL;
	JL = JL->Next;
	delete CurJL;
	} // while
*/
} // InterCommon::DrillDownPoint

/*===========================================================================*/

DEM *InterCommon::FindAndLoadDEM(double Lat, double Lon)
{
Joe *Clip;
//FILE *DEMFile;

if (ActiveDEM && ActiveDEM->Map())
	{
	if (ActiveDEM->GeographicPointContained(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon, TRUE))
		{
		return (ActiveDEM);
		}
	// its not the right one so lose it
	ActiveDEM->FreeRawElevs();
	} // if
else
	{
	if (ActiveDEM)
		delete ActiveDEM;
	ActiveDEM = new DEM;
	} // if

// try to find the right one

if (ActiveDEM)
	{
	DBHost->ResetGeoClip();
	for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM) && Clip->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			if (Lat >= Clip->SELat && Lat <= Clip->NWLat && Lon >= Clip->SELon && Lon <= Clip->NWLon)
				{
				if (ActiveDEM->AttemptLoadDEM(Clip, 1, ProjHost))
					{
					if (ActiveDEM->GeographicPointContained(GlobalApp->AppEffects->FetchDefaultCoordSys(), Lat, Lon, TRUE))
						return (ActiveDEM);
					} // if file opened
				} // if point in bounds
			} // if DEM
		} // for
	} // if

return (NULL);

} // InterCommon::FindAndLoadDEM()

/*===========================================================================*/

void InterCommon::ConformToTopo(void)
{
double TempElev;
CoordSys *MyCoords;
JoeCoordSys *MyAttr;
unsigned long Ct;
int Reject = 0;
VertexDEM Vert;

if (VE.Active)
	{
	if (MyAttr = (JoeCoordSys *)VE.Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	for (Ct = 0; Ct < VE.SelectedNumPts; Ct++)
		{
		VE.SelPts[Ct]->ProjToDefDeg(MyCoords, &Vert);
		TempElev = ElevationPointNULLReject(Vert.Lat, Vert.Lon, Reject);
		if (! Reject)
			{
			Vert.Elev = (float)TempElev;
			VE.SelPts[Ct]->DefDegToProj(MyCoords, &Vert);
			} // if
		} // for
	} // if

} // InterCommon::ConformToTopo()

/*===========================================================================*/

void InterCommon::DeleteSelectedPoints(void)
{
VectorPoint *NewList, *PLinkA, *PLinkB;
unsigned long Ct, RemainingPts = Joe::GetFirstRealPtNum();

if (DBHost && VE.Active && VE.SelectedNumPts > 0)
	{
	for (Ct = Joe::GetFirstRealPtNum(); Ct < VE.Active->NumPoints(); Ct++)
		{
		if (! GetPointSelState(Ct))
			RemainingPts++;
		} // for
	if (RemainingPts > Joe::GetFirstRealPtNum())
		{
		if (NewList = DBHost->MasterPoint.Allocate(RemainingPts))
			{
			for (PLinkA = VE.Active->GetFirstRealPoint(), PLinkB = NewList->Next, Ct = Joe::GetFirstRealPtNum(); PLinkA && PLinkB;
				PLinkA = PLinkA->Next, Ct++)
				{
				if (! GetPointSelState(Ct))
					{
					PLinkB->Latitude = PLinkA->Latitude;
					PLinkB->Longitude = PLinkA->Longitude;
					PLinkB->Elevation = PLinkA->Elevation;
					PLinkB = PLinkB->Next;
					} // if
				} // for
			#ifdef WCS_JOE_LABELPOINTEXISTS
			if (GetPointSelState((unsigned long)0))
				{
				NewList->Latitude = NewList->Next->Latitude;
				NewList->Longitude = NewList->Next->Longitude;
				NewList->Elevation = NewList->Next->Elevation;
				} // if
			else
				{
				NewList->Latitude = VE.Active->Points()->Latitude;
				NewList->Longitude = VE.Active->Points()->Longitude;
				NewList->Elevation = VE.Active->Points()->Elevation;
				} // else
			#endif // WCS_JOE_LABELPOINTEXISTS
			DBHost->MasterPoint.DeAllocate(VE.Active->Points());
			VE.Active->Points(NewList);
			VE.Active->NumPoints(RemainingPts);
			} // if
		} // if
	else
		{
		DBHost->MasterPoint.DeAllocate(VE.Active->Points());
		VE.Active->Points(NULL);
		VE.Active->NumPoints(0);
		} // else
	} // if

} // InterCommon::DeleteSelectedPoints()

/*===========================================================================*/

void InterCommon::SetActiveTime(double NewTime)
{
NotifyTag Changes[2];

#ifdef WCS_BUILD_DEMO
if (NewTime > 5.0)
	{
	UserMessageDemo("Time cannot be set to greater than 5 seconds.");
	NewTime = 5.0;
	} // if
#endif // WCS_BUILD_DEMO

ActiveTime = NewTime;

#ifdef WCS_BUILD_DEMO
// in case the first version is hacked
if (ActiveTime > 5.0)
	ActiveTime = 5.0;
#endif // WCS_BUILD_DEMO

// do this before notification
GlobalApp->UpdateProjectByTime();
Changes[0] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Changes[1] = NULL;
GenerateNotify(Changes);

} // InterCommon::SetActiveTime

/*===========================================================================*/

void InterCommon::SetActiveTimeAndRate(double NewTime, double NewRate)
{
NotifyTag Changes[2];

#ifdef WCS_BUILD_DEMO
if (NewTime > 5.0)
	{
	UserMessageDemo("Time cannot be set to greater than 5 seconds.");
	NewTime = 5.0;
	} // if
#endif // WCS_BUILD_DEMO

ActiveTime = NewTime;

#ifdef WCS_BUILD_DEMO
// in case the first version is hacked
if (ActiveTime > 5.0)
	ActiveTime = 5.0;
#endif // WCS_BUILD_DEMO

ProjFrameRate = NewRate;
// do this before notification
GlobalApp->UpdateProjectByTime();
Changes[0] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Changes[1] = NULL;
GenerateNotify(Changes);

} // InterCommon::SetActiveTime

/*===========================================================================*/

ULONG InterCommon::Save(FILE *ffile, struct ChunkIODetail *Detail)
{

return(0);

} // InterCommon::Save

/*===========================================================================*/

void InterCommon::SyncFloaters(Database *CurDB)
{

if (VE.ProjRefCoordsFloating)
	{
	SetFloating(1, CurDB);
	} // if

} // InterCommon::SyncFloaters

/*===========================================================================*/

void InterCommon::SetFloating(short NewFloating, Database *CurDB)
{
double North, South, East, West;

if (NewFloating)
	{
	if (CurDB || (CurDB = GlobalApp->AppDB))
		{
		CurDB->GetBounds(North, South, East, West);
		SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Y, (North + South) * 0.5,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_X, (East + West) * 0.5,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Z, 0.0, 0);
		} // if
	VE.ProjRefCoordsFloating = 1;
	} // if enable floating
else
	{
	VE.ProjRefCoordsFloating = 0;
	} // else if unfloat

} // InterCommon::SetFloating
