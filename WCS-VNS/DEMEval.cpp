// DEMEval.cpp
// Code to support 'Freezing' of terraffectors into existing or new copies of DEMs
// Built from scratch and from ragged bits of ViewGUI and LWS/3DS export code
// on Mar 8 2002 by CXH to commemorate Nederland's Frozen Dead Guy Days.
// Copyright 2002, 3D Nature

#include "stdafx.h"
#include "FeatureConfig.h"

#ifdef WCS_DEM_TFX_FREEZE

#include "DEMEval.h"
#include "Application.h"
#include "Useful.h"
#include "Project.h"
#include "DEM.h"
#include "Requester.h"

// sets state and executes all in one
int DEMEval::FreezeDEMTFXUI(DBEditGUI *SelectFromDB, SearchQuery *SelectFromQ)
{
int NewDEMs = 0, DEMCount = 0;
char *DEMPrefixName;
char RequestStr[255];
GeneralEffect *DisableRun;

// no point in asking at this time as we can only do "all" option
/*
switch(UserMessageCustom("Terraffector Freeze", "Freeze All Enabled Objects, Database Selected Objects or Use Active Search Query?", "All", "Selected", "Query"))
	{
	case 0: // Selected
		{
		//UserMessageOK("Terraffector Freeze", "You chose 0.");
		break;
		} // 
	case 1: // All
		{
		//UserMessageOK("Terraffector Freeze", "You chose 1.");
		break;
		} // 
	case 2: // Query
		{
		//UserMessageOK("Terraffector Freeze", "You chose 2.");
		break;
		} // 
	} // switch
*/

DEMPrefixName = NULL;
strcpy(RequestStr, "FRZ");
if(GetInputString("To create new DEMs instead of overwriting old DEMs (old DEMs will be disabled), enter a prefix to use for all newly-created DEMs. Leave blank to overwrite old DEMs.", WCS_REQUESTER_FILESAFE_ONLY, RequestStr))
	{
	if(RequestStr[0])
		{
		NewDEMs = 1;
		DEMPrefixName = RequestStr;
		} // if
	} // if
else
	{
	return(0);
	} // else

if(UserMessageYN("Terraffector Freeze", "Proceed with Freeze?"))
	{
	DEMCount= FreezeDEMTFX(SelectFromDB, SelectFromQ, NewDEMs, DEMPrefixName);
	if(DEMCount)
		{
		if(UserMessageYN("Terraffector Freeze", "Disable all Area Terraffectors and Linear Terraffectors now?"))
			{
			NotifyTag Changes[2];
			Changes[1] = 0;
			for(DisableRun = GlobalApp->AppEffects->RasterTA; DisableRun; DisableRun = DisableRun->Next)
				{
				DisableRun->SetEnabled(0);
				// generate a notify
				Changes[0] = MAKE_ID(DisableRun->GetNotifyClass(), DisableRun->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, DisableRun->GetRAHostRoot());
				} // for
			for(DisableRun = GlobalApp->AppEffects->Terraffector; DisableRun; DisableRun = DisableRun->Next)
				{
				DisableRun->SetEnabled(0);
				// generate a notify
				Changes[0] = MAKE_ID(DisableRun->GetNotifyClass(), DisableRun->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, DisableRun->GetRAHostRoot());
				} // for
			} // if
		} // if
	} // if

return(0);
} // DEMEval::FreezeDEMTFXUI

/*===========================================================================*/

// sets state and executes all in one
int DEMEval::FreezeDEMTFX(DBEditGUI *SelectFromDB, SearchQuery *SelectFromQ, int NewDEMs, char *DEMPrefixName)
{
SelectFromDBSelected = SelectFromDB;
SelectFromQuery = SelectFromQ;
MakeNewDEMs = NewDEMs;
PrefixName[0] = NULL;
if(DEMPrefixName) strncpy(PrefixName, DEMPrefixName, 20);
PrefixName[19] = NULL;

return(Freeze(1));
} // DEMEval::FreezeDEMTFX


/*===========================================================================*/


// executes from pre-set state
int DEMEval::Freeze(int Notify)
{
unsigned long DBFlags = NULL;
int NumFrozen = 0, FreezeCandidates = 0, ClipCount = 0;

if(PrepFreeze())
	{
	// count candidates
	for(Clip = Rend.DBase->GetFirst(DBFlags); Clip ; Clip = Rend.DBase->GetNext(Clip))
		{
		if(Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
			{
			if(Clip->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if(MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
					{
					FreezeCandidates++;
					} // if
				} // if
			} // if
		} // for

	for(Clip = Rend.DBase->GetFirst(DBFlags); Clip && ClipCount < FreezeCandidates; Clip = Rend.DBase->GetNext(Clip))
		{
		if(Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
			{
			if(Clip->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if(MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
					{
					ClipCount++; // whether we succeed or not.
					NumFrozen += FreezeDEM(Notify);
					} // if
				} // if
			} // if
		} // for
	} // if

CleanupFromFreeze();

return(NumFrozen);

} // DEMEval::Freeze

/*===========================================================================*/

int DEMEval::FreezeDEM(int Notify)
{
double ElScale, ElScaleMult;
CoordSys *MyCS = NULL;
JoeCoordSys *MyAttr;
Joe *NewClip = NULL;
BusyWin *BWFD;
int Status = 0;
//unsigned long int LastVertA, LastVertB, BaseVtx, NextBaseVtx;
unsigned long int LonCt, LatCt, Ct;
NotifyTag Changes[3];
char filename[512], justname[100], *FilePath;

if(!(Clip && MyDEM))
	{
	return(0);
	} // if

if ((MyAttr = (JoeCoordSys *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	MyCS = MyAttr->Coord;

Clip->AttemptLoadDEM(0, GlobalApp->MainProj);
if((Terrain = MyDEM->Pristine) && Terrain->LonEntries() && Terrain->LatEntries())
	{
	// Ensure that the point remains within the DEM after whatever precision errors occur during coord transforms
	// so that VertexDataPoint will find a valid elevation
	Terrain->TransferToVerticesLatLon(TRUE);
	ElScale = Terrain->ElScale();
	ElScaleMult = (ElScale / ELSCALE_METERS);

	BWFD = new BusyWin("Freezing DEM", Terrain->LonEntries(), 'BWFD', 0);
	// Process vertices
	for (Ct = LonCt = 0; LonCt < Terrain->LonEntries(); LonCt ++)
		{
		for (LatCt = 0; LatCt < Terrain->LatEntries(); Ct ++, LatCt ++)
			{
			// adjust elevation
			if(GeoClip.PointContained(Terrain->Vertices[Ct].Lat, Terrain->Vertices[Ct].Lon))
				{
				VTD.Lat = Terrain->Vertices[Ct].Lat;
				VTD.Lon = Terrain->Vertices[Ct].Lon;
				InterStash->VertexDataPoint(&Rend, &VTD, &PGD,
				 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
				// now undo exag and transfer Elev back to RawMap
				Terrain->RawMap[Ct] = (float)DEUnCalcExag(VTD.Elev);
				} // if
			} // for

		if (BWFD)
		  	{
			BWFD->Update(LonCt);
			if (BWFD->CheckAbort())
				{
				break;
				} // if
			} // if

		} // for

	if (BWFD) delete BWFD;
	BWFD = NULL;

	// saving code stolen from DEMEditGUI::SaveDEM
	FilePath = Terrain->AttemptFindDEMPath((char *)Clip->FileName(), GlobalApp->MainProj);
	justname[0] = NULL;
	if(MakeNewDEMs && PrefixName[0]) strcpy(justname, PrefixName);
	strcat(justname, (char *)Clip->FileName());
	strmfp(filename, FilePath, justname);
	strcat(filename, ".elev");
	if (Terrain->SaveDEM(filename, GlobalApp->StatusLog))
		{
		//Clip->RecheckBounds();
		//UserMessageOK((char *)Active->FileName(), "DEM has been saved.");
		//Changes[0] = MAKE_ID(Clip->GetNotifyClass(), Clip->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		//Changes[1] = NULL;
		//GlobalApp->AppEx->GenerateNotify(Changes, Clip->GetRAHostRoot());
		Status = 1;

		if(MakeNewDEMs && PrefixName[0])
			{
			Clip->ClearFlags(WCS_JOEFLAG_ACTIVATED); // disable old object

			// add new object to database. I think I'm doing this right...
			NewClip = Rend.DBase->AddDEMToDatabase("Terraffector Freeze", justname, Terrain, MyCS, Rend.ProjectBase, Rend.EffectsBase);

			// notify everyone of what happened
			if(Notify)
				{
				Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
				Changes[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff,	WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				Changes[2] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			} // if
		} // if

	MyDEM->Pristine->FreeRawElevs();
	} // if

return(Status);
} // DEMEval::FreezeDEM

/*===========================================================================*/

int DEMEval::PrepFreeze(void)
{
int Status = 0;

Rend.PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
Rend.EarthLatScaleMeters = LatScale(Rend.PlanetRad);
Rend.EffectsBase = GlobalApp->AppEffects;

InterStash = GlobalApp->MainProj->Interactive;

DefPO = NULL;
if (CurEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled)
			{
			DefPO = (PlanetOpt *)CurEffect;
			}// if
		CurEffect = CurEffect->Next;
		} // while
	} // if

if(!DefPO)
	{
	return(Status); // no enabled planet
	} // if

Rend.ElevDatum = DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
Rend.Exageration = DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
Rend.DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Rend.TexRefLon = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
Rend.TexRefLat = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
Rend.TexRefElev = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
Rend.RefLonScaleMeters = Rend.EarthLatScaleMeters * cos(Rend.TexRefLat * PiOver180);
Rend.TexData.MetersPerDegLat = Rend.EarthLatScaleMeters;
Rend.TexData.Datum = Rend.ElevDatum;
Rend.TexData.Exageration = Rend.Exageration;
Rend.ExagerateElevLines = DefPO->EcoExageration;
Rend.DBase = GlobalApp->AppDB;
Rend.EffectsBase = GlobalApp->AppEffects;
Rend.ProjectBase = GlobalApp->MainProj;
if (GlobalApp->AppEffects->SetTfxGeoClip(&GeoClip, GlobalApp->AppDB, Rend.EarthLatScaleMeters))
	Status = 1;

Clip = NULL;
MyDEM = NULL;

return(Status);
} // DEMEval::PrepFreeze

/*===========================================================================*/

void DEMEval::CleanupFromFreeze(void)
{
} // DEMEval::CleanupFromFreeze


#endif // WCS_DEM_TFX_FREEZE
