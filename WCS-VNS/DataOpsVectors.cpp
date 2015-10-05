// DataOpsVectors.cpp
// Code for vector data
// Built from v1 dlg.c on 07 Jun 1995 by Chris "Xenon" Hanson
// Original code written by Gary R. Huber, Jamuary, 1994.
// Adapted from DLG.cpp 12/20/99 Frank Weed II
// Copyright 2000 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "Application.h"
#include "Project.h"
#include "AppMem.h"
#include "Database.h"
#include "Joe.h"
#include "Log.h"
#include "Layers.h"
#include "Useful.h"
#include "Palette.h"
#include "Requester.h"
#include "Projection.h"
#include "Types.h"
#include "DB3LayersGUI.h"
#include "DB3OneLayerGUI.h"
#include "DEM.h"
#include "DBEditGUI.h"
#include "EffectsLib.h"
#include "Notify.h"
#include "Interactive.h"
#include "DataOpsDefs.h"
#include "DXF.h"
#include "resource.h"

#include "FeatureConfig.h"

extern NotifyTag DBLoadEvent[];
extern NotifyTag DBPreLoadEvent[];

#ifdef DEBUG
// enable for debugging shapefiles
//#define SHAPE_TRACE
// stop reading shapefile after reading a given number of records
//#define BAILEARLY
#endif // DEBUG

static unsigned long VectApproveHook(Joe *Scrutiny);

/*===========================================================================*/

unsigned long ImportWizGUI::Import(char *FileName, Joe *Level, unsigned char Type)
{
Joe *LoadRoot;
FILE *DLGLoad, *DBFLoad, *Index = NULL;
unsigned long NumLoaded;
char ShortName[255], BaseName[255];
short i;

ShortName[0] = NULL;
BaseName[0] = NULL;
for (i = 0; FileName[i]; i++)
	{
	if ((FileName[i] == ':') || (FileName[i] == '/') ||
	 (FileName[i] == '\\'))
		{
		strcpy(ShortName, "\n");
		strcat(ShortName, &FileName[i + 1]);
		strcpy(BaseName, &FileName[i + 1]);
		} // if
	} // for

NumLoaded = 0;
if (FileName && Level)
	{
	if (Type == WCS_DATABASE_IMPORT_SHAPE)
		{
		DLGLoad = PROJ_fopen(FileName, "rb");
		FileName[strlen(FileName) - 4] = 0;
		strcat(FileName, ".dbf");
		DBFLoad = PROJ_fopen(FileName, "rb");
		FileName[strlen(FileName) - 4] = 0;
		strcat(FileName, ".shx");
		Index = PROJ_fopen(FileName, "rb");
		FileName[strlen(FileName) - 4] = 0;	// for layer name if needed
		} // if
	else
		{
		DLGLoad = PROJ_fopen(FileName, "r");
		DBFLoad = NULL;
		} // else
	if (DLGLoad)
		{
		if (Importing->TestOnly)
			{
			LoadRoot = NULL;	// not relevant in this case, keeps the compiler happy
			switch(Type)
				{
				case WCS_DATABASE_IMPORT_DLG:
					{
					NumLoaded = ImportDLG(DLGLoad, LoadRoot);
					break;
					} // DLG
//				case WCS_DATABASE_IMPORT_MERIDIAN2:
//					{
//					NumLoaded = ImportMeridian2(DLGLoad, LoadRoot);
//					break;
//					} // Meridian2
				case WCS_DATABASE_IMPORT_SHAPE:
					{
					if (Importing->FileLayer)
						strcpy(Importing->LayerName, (const char *)Importing->NameBase);
					NumLoaded = ImportShape(DLGLoad, DBFLoad, Index, LoadRoot, BaseName);
					break;
					} // Shape
				default:
					break;
				} // switch (type)
			} // if
		else if (LoadRoot = new (ShortName) Joe)
			{
			LoadRoot->TemplateItem = GlobalApp->TemplateLoadInProgress;
			if (DBHost->AddJoe(LoadRoot, Level, ProjHost))
				{
				DBHost->GenerateNotify(DBPreLoadEvent);
				switch (Type)
					{
					case WCS_DATABASE_IMPORT_DLG:
						{
						NumLoaded = ImportDLG(DLGLoad, LoadRoot);
						break;
						} // DLG
//					case WCS_DATABASE_IMPORT_MERIDIAN2:
//						{
//						NumLoaded = ImportMeridian2(DLGLoad, LoadRoot);
//						break;
//						} // Meridian2
					case WCS_DATABASE_IMPORT_SHAPE:
						{
						NumLoaded = ImportShape(DLGLoad, DBFLoad, Index, LoadRoot, BaseName);
						break;
						} // WDB
					default:
						break;
					} // switch (Type)
				DBHost->GenerateNotify(DBLoadEvent);
				} // if
			else
				{
				delete LoadRoot;
				LoadRoot = NULL;
				} // else
			} // else if
		if (Index)
			fclose(Index);
		if (DBFLoad)
			fclose(DBFLoad);
		fclose(DLGLoad);
		} // if
	} // if
return(NumLoaded);

} // ImportWizGUI::Import

/*===========================================================================*/

static char VMesg[100];

unsigned long ImportWizGUI::ImportDLG(char *FileName, Joe *Level)
{
Joe *LoadRoot;
FILE *DLGLoad;
unsigned long NumLoaded;
char ShortName[100], *NamePart, *DatePart, *SizePart;

NumLoaded = 0;
NamePart = DatePart = SizePart = NULL;
if (FileName && Level)
	{
	if (DLGLoad = PROJ_fopen(FileName, "r"))
		{
		fgetline(VMesg, 81, DLGLoad);
		VMesg[81] = NULL;
		strupr(VMesg);
		// Make sure DLG appears in the first line/record
		if (strstr(VMesg, "DLG"))
			{
			if (fgetline(VMesg, 81, DLGLoad))
				{
				VMesg[40] = NULL;
				VMesg[51] = NULL;
				VMesg[60] = NULL;
				// Chew up 12 intervening records
				fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad);
				fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad);
				fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad);
				fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad); fgetline(ShortName, 81, DLGLoad);
				fread(&VMesg[61], 20, 1, DLGLoad); // Snag Category name
				VMesg[81] = NULL;
				DatePart = &VMesg[41];
				SizePart = &VMesg[52];
				NamePart = &VMesg[61];
				(void)TrimTrailingSpaces(VMesg);
				(void)TrimTrailingSpaces(DatePart);
				if (DatePart[strlen(DatePart) - 1] == ',')
					{
					DatePart[strlen(DatePart) - 1] = NULL;
					} // if
				SizePart = strtok(SizePart, " ");
				(void)TrimTrailingSpaces(NamePart);
				sprintf(ShortName, "%s: %s (%s) 1:%s", VMesg, NamePart, DatePart, SizePart);
				fseek(DLGLoad, 0, SEEK_SET); // return to beginning
				if (LoadRoot = new (ShortName) Joe)
					{
					LoadRoot->TemplateItem = GlobalApp->TemplateLoadInProgress;
					if (DBHost->AddJoe(LoadRoot, Level, ProjHost))
						{
						NumLoaded = ImportDLG(DLGLoad, LoadRoot);
						} // if
					else
						{
						delete LoadRoot;
						LoadRoot = NULL;
						} // else
					} // if
				} // if
			} // if
		fclose(DLGLoad);
		} // if
	} // if
return(NumLoaded);

} // ImportWizGUI::ImportDLG

/*===========================================================================*/

union MapProjection VCoords;

unsigned long ImportWizGUI::ImportDLG(FILE *fDLG, Joe *FileParent)
{
double Lat, Lon, North, East, TestVal;
BusyWin *Position = NULL;
Joe *AverageGuy = NULL, *AfterGuy = NULL;
VectorPoint *PLink;
char *InPtr;
float Northest, Southest, Eastest, Westest;
//#ifndef WCS_BUILD_VNS
float MyNorth, MySouth, MyEast, MyWest;
//#endif // WCS_BUILD_VNS
unsigned long PossibleObjs, MaxDif = 0;
long Fault, i, j, ObjsImported = 0 /*, Padded = 1*/;
#ifdef WCS_BUILD_VNS
long utmcode;
#endif // WCS_BUILD_VNS
short RefSys, UTMZone, Pairs, Attrs;
unsigned short Major, Minor;
//short AttrHist[5], AttrMax = 0;
short namelen;
char str[100], BailOut, Ident;
char LayerA[5];

LayerA[0] = NULL;

//for (i = 0; i < 10; i++)
//	{
//	AttrHist[i] = 0;
//	} // for

// Minimize the extents
Northest = -90.0f;
Southest = 90.0f;
Eastest = 360.0f;
Westest = -360.0f;

// If there is a CR or LF in the first 80 characters, it's not a padded file.
memset(VMesg, 0, 81);
fread(VMesg, 1, 81, fDLG);
VMesg[81] = NULL;
/*** Padded is not currently used, apparently.
if (strchr(VMesg, 0x0a) || (strchr(VMesg, 0x0d)))
	{
	Padded = 0;
	} // if
***/

fseek(fDLG, 0, SEEK_SET);

// Skip over first & second line
fgetline(VMesg, 81, fDLG);
fgetline(VMesg, 81, fDLG);
VMesg[40] = 0;									// mark end of map name area
namelen = (short)strcspn(VMesg, ",");
strncpy(Importing->NameBase, VMesg, namelen);	// copy DLG map name
Importing->NameBase[namelen] = 0;
// Skip over third & fourth line
fgetline(VMesg, 81, fDLG);
fgetline(VMesg, 81, fDLG);

if (VMesg[5] == '3')
	{
	// do it
	RefSys = VMesg[11] - '0';
	if ((RefSys == 1) || (RefSys == 3))
		{
		sscanf(&VMesg[12], "%6hd", &UTMZone);
		if (RefSys == 1)	// 24K & 100K files
			{
			MaxDif = 10000;
#ifndef WCS_BUILD_VNS
			UTMLatLonCoords_Init(&VCoords.UTM, UTMZone);
#endif // !WCS_BUILD_VNS
			// Eat up five lines of projection params
			fgetline(VMesg, 81, fDLG); fgetline(VMesg, 81, fDLG);
			fgetline(VMesg, 81, fDLG); fgetline(VMesg, 81, fDLG);
			fgetline(VMesg, 81, fDLG);
#ifdef WCS_BUILD_VNS
			Importing->IWCoordSys.SetSystem("UTM - NAD 27");
			utmcode = UTMZone;
			UTMZoneNum2DBCode(&utmcode);
			Importing->IWCoordSys.SetZoneByCode(utmcode);
#endif // WCS_BUILD_VNS
			} // if
		if (RefSys == 3)	// 2M files
			{
			MaxDif = 200000;
			// Record 5, 3 proj params
			fgetline(VMesg, 81, fDLG);
			sscanf(VMesg, "%24s", str);
			VCoords.Alb.ProjPar[0] = FCvt(str);
			sscanf(&VMesg[24], "%24s", str);
			VCoords.Alb.ProjPar[1] = FCvt(str);
			sscanf(&VMesg[48], "%24s", str);
			VCoords.Alb.ProjPar[2] = FCvt(str);

			// Record 6, 3 proj params
			fgetline(VMesg, 81, fDLG);
			sscanf(VMesg, "%24s", str);
			VCoords.Alb.ProjPar[3] = FCvt(str);
			sscanf(&VMesg[24], "%24s", str);
			VCoords.Alb.ProjPar[4] = FCvt(str);
			sscanf(&VMesg[48], "%24s", str);
			VCoords.Alb.ProjPar[5] = FCvt(str);

			// Record 7, 2 proj params
			fgetline(VMesg, 81, fDLG);
			sscanf(VMesg, "%24s", str);
			VCoords.Alb.ProjPar[6] = FCvt(str);
			sscanf(&VMesg[24], "%24s", str);
			VCoords.Alb.ProjPar[7] = FCvt(str);

			// Eat up two remaining lines of projection params
			fgetline(VMesg, 81, fDLG); fgetline(VMesg, 81, fDLG);

#ifdef WCS_BUILD_VNS
			// ???
#else
			AlbLatLonCoords_Init(&VCoords.Alb);
#endif // WCS_BUILD_VNS

			} // if
		if (Importing->TestOnly)
			return 1;
		// Eat up another line
		fgetline(VMesg, 81, fDLG);

		// Read in corner bounds
		for (i = 0; i < 4; i++)
			{
			j = 4;
			fgetline(VMesg, 81, fDLG);
			strupr(VMesg);
			if ((VMesg[0] == 'S') && (VMesg[1] == 'W')) j = 0;
			if ((VMesg[0] == 'N') && (VMesg[1] == 'W')) j = 1;
			if ((VMesg[0] == 'N') && (VMesg[1] == 'E')) j = 2;
			if ((VMesg[0] == 'S') && (VMesg[1] == 'E')) j = 3;
			if (j < 4)
				{
				sscanf(&VMesg[2], "%le%le%le%le", &VCoords.UTM.RefLat[j],
				 &VCoords.UTM.RefLon[j], &VCoords.UTM.RefEast[j], &VCoords.UTM.RefNorth[j]);
				} // if
			} // for
		fgetline(VMesg, 81, fDLG); // Get record 15, with category and # of objs
		VMesg[62] = NULL;
		PossibleObjs = atoi(&VMesg[56]);
		if (PossibleObjs)
			{
			Position = new BusyWin ("Import DLG", PossibleObjs, 'IDLG', 0);
			if (((UTMZone > 0) && (UTMZone < 60)) || UTMZone == 9999)
				{
				BailOut = 0;
				while (fgetline(VMesg, 81, fDLG) && !BailOut)
					{
					if (Position)
						{
						Position->Update(ObjsImported);
						BailOut += Position->CheckAbort();
						} // if
					if (VMesg[0] == 'L')
						{
//							printf("*"); fflush(stdout);
						//fscanf(fDLG, "%d", &LongDummy);		// IDNum
						//fscanf(fDLG, "%d", &LongDummy);		// StartNode
						//fscanf(fDLG, "%d", &LongDummy);		// EndNode
						//fscanf(fDLG, "%hd", &ShortDummy);	// LeftArea
						//fscanf(fDLG, "%hd", &ShortDummy);	// RightArea
						//fscanf(fDLG, "%hd", &Pairs);			// Pairs
						//fscanf(fDLG, "%hd", &Attrs);			// Attrs
						//fscanf(fDLG, "%hd", &ShortDummy);	// Text
						sscanf(&VMesg[42], "%hd", &Pairs);			// Pairs
						sscanf(&VMesg[48], "%hd", &Attrs);			// Attrs
						if ((Pairs >= 1) && (Pairs < WCS_DXF_MAX_OBJPTS)) // sanity check
							{
							if (AverageGuy = new (NULL) Joe)
								{
								unsigned long PtsToAlloc;

								AverageGuy->TemplateItem = GlobalApp->TemplateLoadInProgress;
								PtsToAlloc = Pairs + Joe::GetFirstRealPtNum();
								if (AverageGuy->Points(DBHost->MasterPoint.Allocate(PtsToAlloc)))
									{
									LayerA[0] = NULL;
									AverageGuy->NumPoints(PtsToAlloc);
//#ifndef WCS_BUILD_VNS
									MyNorth = -90.0f;
									MySouth = 90.0f;
									MyEast = 360.0f;
									MyWest = -360.0f;
//#endif // !WCS_BUILD_VNS
									PLink = AverageGuy->GetFirstRealPoint();
									East = North = 0;
									AverageGuy->AttribInfo.LineStyle = 4;	// we want SOLID
									AverageGuy->AttribInfo.LineWidth = 1;
									AverageGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
									if (!Importing->DBRenderFlag)
										AverageGuy->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
									for (Fault = j = 0; j < Pairs; j++)
										{
										if ((j % 3) == 0)
											{
											// Need to fetch a new line/record
											fgetline(VMesg, 81, fDLG);
											InPtr = VMesg;
											} // if
										if (j)
											{
											sscanf(InPtr, "%le", &TestVal);
											InPtr = &InPtr[12];
											// This is that sanity checker
											if (fabs(TestVal - East) < (float)(MaxDif))
												{
												East = TestVal;
												} // if
											else
												{
												Fault = 1;
												break;
												} // else
											sscanf(InPtr, "%le", &TestVal);
											InPtr = &InPtr[12];
											if (fabs(TestVal - North) < (float)(MaxDif))
												{
												North = TestVal;
												} // if
											else
												{
												Fault = 1;
												break;
												} // else
											} // if
										else
											{
											sscanf(InPtr, "%le", &East);
											InPtr = &InPtr[12];
											sscanf(InPtr, "%le", &North);
											InPtr = &InPtr[12];
											} // else
										if (!Fault)
											{
#ifdef WCS_BUILD_VNS
											Lat = North;
											Lon = East;
#else // WCS_BUILD_VNS
											VCoords.UTM.North = North;
											VCoords.UTM.East = East;
											if (RefSys == 1)
												{
												UTM_LatLon(&VCoords.UTM);
												} // if
											if (RefSys == 3)
												{
												Alb_LatLon(&VCoords.Alb);
												} // if
											Lon = VCoords.UTM.Lon;
											Lat = VCoords.UTM.Lat;
#endif // WCS_BUILD_VNS
											if (PLink)
												{
												PLink->Latitude  = Lat;
												PLink->Longitude = Lon;
												PLink->Elevation = 0.0f;
												PLink = PLink->Next;
#ifndef WCS_BUILD_VNS
												if (Lat > MyNorth)
													{
													MyNorth = (float)Lat;
													} // if
												if (Lat < MySouth)
													{
													MySouth = (float)Lat;
													} // if
												if (Lon > MyWest)
													{
													MyWest = (float)Lon;
													} // if
												if (Lon < MyEast)
													{
													MyEast = (float)Lon;
													} // if
#endif // !WCS_BUILD_VNS
												} // if
											} // if
										} // for j
									#ifdef WCS_JOE_LABELPOINTEXISTS
									if (AverageGuy->GetFirstRealPoint())
										{
										AverageGuy->Points()->Latitude  = AverageGuy->GetFirstRealPoint()->Latitude;
										AverageGuy->Points()->Longitude = AverageGuy->GetFirstRealPoint()->Longitude;
										AverageGuy->Points()->Elevation = AverageGuy->GetFirstRealPoint()->Elevation;
										} // if
									#endif // WCS_JOE_LABELPOINTEXISTS
									if (!Fault)
										{ // Do attribs
//											if (MyStdOut)
//												{
//												fprintf(MyStdOut, "\n");
//												} // if
										Ident = 0;
//										if (Attrs > AttrMax) AttrMax = Attrs;
//										if (Attrs < 10)
//											{
//											AttrHist[Attrs]++;
//											} // if
										for (j = 0; j < Attrs; j++)
											{
											if ((j % 6) == 0)
												{
												// Reload record buffer
												fgetline(VMesg, 81, fDLG);
												InPtr = VMesg;
												} // if
											sscanf(InPtr, "%s", str);
											InPtr = &InPtr[6];
//												if (j == 0)
												{
//													if (MyStdOut)
//														{
//														fprintf(MyStdOut, "Major = \"%s\" ", str);
//														} // if
												switch(Major = (unsigned short)atoi(str))
													{
													case 50:
														{
														//AverageGuy->AttribInfo.DrawPen = WCS_PALETTE_DEF_LTBLUE;
														AverageGuy->AttribInfo.RedGun = 0x033;
														AverageGuy->AttribInfo.GreenGun = 0x77;
														AverageGuy->AttribInfo.BlueGun = 0xcc;
														Ident = 1;
														strcpy(LayerA, "HYD");
														break;
														} // hydro
													case 170:
														{
														//AverageGuy->AttribInfo.DrawPen = WCS_PALETTE_DEF_RED;
														AverageGuy->AttribInfo.RedGun = 0xbb;
														AverageGuy->AttribInfo.GreenGun = 0x11;
														AverageGuy->AttribInfo.BlueGun = 0x00;
														Ident = 1;
														strcpy(LayerA, "ROA");
														break;
														} // roads
													case 180:
														{
														//AverageGuy->AttribInfo.DrawPen = WCS_PALETTE_DEF_YELLOW;
														AverageGuy->AttribInfo.RedGun = 0xdd;
														AverageGuy->AttribInfo.GreenGun = 0xdd;
														AverageGuy->AttribInfo.BlueGun = 0x22;
														Ident = 1;
														strcpy(LayerA, "RAI");
														break;
														} // railroads
													case 190:
														{
														//AverageGuy->AttribInfo.DrawPen = WCS_PALETTE_DEF_DKBLUE;
														AverageGuy->AttribInfo.RedGun = 0x33;
														AverageGuy->AttribInfo.GreenGun = 0x44;
														AverageGuy->AttribInfo.BlueGun = 0x88;
														strcpy(LayerA, "PIP");
														Ident = 1;
														break;
														} // piping
													default:
														{
														if (!Ident)
															{
															//AverageGuy->AttribInfo.DrawPen = WCS_PALETTE_DEF_WHITE;
															AverageGuy->AttribInfo.RedGun = 0xdd;
															AverageGuy->AttribInfo.GreenGun = 0xdd;
															AverageGuy->AttribInfo.BlueGun = 0xdd;
															strcpy(LayerA, "DLG");
															} // if
														break;
														} // default
													} // switch on Major
												} // if
											sscanf(InPtr, "%s", str);
											InPtr = &InPtr[6];
											if (Major != 0)
												{
												Minor = (unsigned short)atoi(str);
												// No need to test for success decoding, always store it.
												//if (DecodeCanonName(Major, Minor))
													{
													if (AverageGuy->AttribInfo.CanonicalTypeMajor == 0)
														{
														AverageGuy->AttribInfo.CanonicalTypeMajor = Major;
														AverageGuy->AttribInfo.CanonicalTypeMinor = Minor;
														} // if
													else if (AverageGuy->AttribInfo.SecondTypeMajor == 0)
														{
														AverageGuy->AttribInfo.SecondTypeMajor = Major;
														AverageGuy->AttribInfo.SecondTypeMinor = Minor;
														} // else
													} // if
												} // if
											} // for
										if (VectApproveHook(AverageGuy))
											{
#ifdef WCS_BUILD_VNS
											if (NewCoordSys)
												AverageGuy->AddEffect(NewCoordSys, -1);
											AverageGuy->RecheckBounds();
											MyNorth = (float)AverageGuy->NWLat;
											MyWest = (float)AverageGuy->NWLon;
											MySouth = (float)AverageGuy->SELat;
											MyEast = (float)AverageGuy->SELon;
#else // WCS_BUILD_VNS
											AverageGuy->NWLat = MyNorth;
											AverageGuy->NWLon = MyWest;
											AverageGuy->SELat = MySouth;
											AverageGuy->SELon = MyEast;
#endif // WCS_BUILD_VNS
											FileParent->AddChild(AverageGuy, AfterGuy);
											AfterGuy = AverageGuy;
											if (LayerA[0])
												{
												DBHost->DBLayers.AddObjectToLayer(AverageGuy, LayerA);
												} // if
//												AddJoe(AverageGuy, FileParent);
											DBHost->DBLayers.AddObjectToLayer(AverageGuy, Importing->NameBase);	// map name
											ObjsImported++;
//#ifndef WCS_BUILD_VNS
											if (MyNorth > Northest)
												{
												Northest = MyNorth;
												} // if
											if (MySouth < Southest)
												{
												Southest = MySouth;
												} // if
											if (MyEast < Eastest)
												{
												Eastest = MyEast;
												} // if
											if (MyWest > Westest)
												{
												Westest = MyWest;
												} // if
//#endif // !WCS_BUILD_VNS
											} // if
										else
											{
											DBHost->MasterPoint.DeAllocate(AverageGuy->Points());
											AverageGuy->NumPoints(0);
											AverageGuy->Points(NULL);
											delete AverageGuy;
											AverageGuy = NULL;
											} // else
										} // if
									} // if
								else
									{
									UserMessageOK("Import DLG", "Couldn't allocate points. Aborting.", REQUESTER_ICON_STOP);
									delete AverageGuy;
									AverageGuy = NULL;
									BailOut = 1;
									} // else
								} // if
							else
								{
								UserMessageOK("Import DLG", "Couldn't make new object. Aborting.", REQUESTER_ICON_STOP);
								BailOut = 1;
								break;
								} // else
							} // if
						else
							{
							GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, "Import DLG: Improper number of points. Continuing.");
							} // else
						} // if
					} // while
				} // if
			else
				{
				sprintf(str, "Inappropriate UTM Zone %d. DLG Import operation terminated.", UTMZone);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, str);
				UserMessageOK("Import DLG", str, REQUESTER_ICON_STOP);
				} // else
			if (Position)
				{
				delete Position;
				Position = NULL;
				} // if
			} // good
//#ifndef WCS_BUILD_VNS
		FileParent->NWLat = Northest;
		FileParent->NWLon = Westest;
		FileParent->SELat = Southest;
		FileParent->SELon = Eastest;
//#endif // !WCS_BUILD_VNS
		DBHost->BoundUpTree(FileParent);
		// Log Messages
		//sprintf(str, "Imported %d DLG Objects, Max Attribs: %d", ObjsImported, AttrMax);
		sprintf(str, "Imported %d DLG Objects.", ObjsImported);
		GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_VEC_LOAD, str);
		//sprintf(str, "Attr Hist: 0:%04d, 1:%04d, 2:%04d", AttrHist[0], AttrHist[1], AttrHist[2]);
		//GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_NULL, str);
		//sprintf(str, "           3:%04d, 4:%04d, 5:%04d", AttrHist[3], AttrHist[4], AttrHist[5]);
		//GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_NULL, str);
		return(ObjsImported); // good

		} // if
	else
		{
		UserMessageOK("Import DLG", "Inappropriate Reference System! Operation terminated.", REQUESTER_ICON_STOP);
		} // bad
	} // if
else
	{
	UserMessageOK("Import DLG", "File not a USGS Optional DLG! Operation terminated.", REQUESTER_ICON_STOP);
	} // else
	
return(0);

} // ImportWizGUI::ImportDLG

/*===========================================================================*/

// SetNullObject is used for SHAPE import
Joe *ImportWizGUI::SetNullObject(char *Name, char *Layer, char Style, char Color)
{
Joe *AverageGuy;
LayerEntry *TargetLayer;
int i;
unsigned char RGB[8][3] = {(unsigned char)136, (unsigned char)153, (unsigned char)187,  (unsigned char)0, (unsigned char)0, (unsigned char)0,  (unsigned char)221, (unsigned char)221, (unsigned char)221,  (unsigned char)187, (unsigned char)31, (unsigned char)0,  (unsigned char)51, (unsigned char)68, (unsigned char)136,
					(unsigned char)51, (unsigned char)153, (unsigned char)34,  (unsigned char)51, (unsigned char)119, (unsigned char)204,  (unsigned char)221, (unsigned char)221, (unsigned char)34};

if (AverageGuy = new (Name) Joe)
	{
	AverageGuy->TemplateItem = GlobalApp->TemplateLoadInProgress;
	AverageGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
	if (!Importing->DBRenderFlag)
		AverageGuy->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
	AverageGuy->SetLineStyle(Style);
	AverageGuy->SetLineWidth((unsigned char)1);
	AverageGuy->SetRGB(RGB[Color][0], RGB[Color][1], RGB[Color][2]);
	i = 0;
	while (Layer[i * WCS_DXF_MAX_LAYERLENGTH] && i < WCS_DXF_MAX_LAYERS)
		{
		if (TargetLayer = DBHost->DBLayers.MatchMakeLayer(&Layer[i * WCS_DXF_MAX_LAYERLENGTH], 0))
			{
			AverageGuy->AddObjectToLayer(TargetLayer);	// if it fails, tough!
			} // if
		++i;
		} // while
	} // if

return (AverageGuy);

} // ImportWizGUI::SetNullObject

/*===========================================================================*/

// ImportWizGUI::SetSHPObjTriple is currently only used in ImportShape
// Pairs is the actual number of real points but the arrays have an extra value for the first value 
// if a label point is in use for vectors
Joe *ImportWizGUI::SetSHPObjTriple(char *Name, long Pairs, char Style, unsigned char Color, double *Lat, double *Lon, float *Elev, bool isPoint)
{
unsigned long AvgFlags;
unsigned long Point;
#ifndef WCS_BUILD_VNS
float MyNorth, MySouth, MyEast, MyWest;
#endif // !WCS_BUILD_VNS
Joe *AverageGuy;
VectorPoint *PLink;
unsigned char RGB[8][3] = {(unsigned char)136, (unsigned char)153, (unsigned char)187,  (unsigned char)0, (unsigned char)0, (unsigned char)0,  (unsigned char)221, (unsigned char)221, (unsigned char)221,  (unsigned char)187, (unsigned char)31, (unsigned char)0,  (unsigned char)51, (unsigned char)68, (unsigned char)136,
					(unsigned char)51, (unsigned char)153, (unsigned char)34,  (unsigned char)51, (unsigned char)119, (unsigned char)204,  (unsigned char)221, (unsigned char)221, (unsigned char)34};

if (AverageGuy = new (Name) Joe)
	{
	unsigned long PtsToAlloc;

	AverageGuy->TemplateItem = GlobalApp->TemplateLoadInProgress;
	PtsToAlloc = Pairs + Joe::GetFirstRealPtNum();
	if (AverageGuy->Points(GlobalApp->AppDB->MasterPoint.Allocate(PtsToAlloc)))
		{
		AverageGuy->NumPoints(PtsToAlloc);
#ifndef WCS_BUILD_VNS
		MyNorth = -90.0f;
		MySouth = 90.0f;
		MyEast = 360.0f;
		MyWest = -360.0f;
#endif // !WCS_BUILD_VNS
		PLink = AverageGuy->Points();
		for (Point = 0; Point < PtsToAlloc; Point++)
			{
			cpstats.n++;
			if (Lat[Point] > cpstats.maxY)
				cpstats.maxY = Lat[Point];
			if (Lat[Point] < cpstats.minY)
				cpstats.minY = Lat[Point];
			cpstats.sumY += Lat[Point];
			cpstats.sumY2 += Lat[Point] * Lat[Point];
			if (Lon[Point] > cpstats.maxX)
				cpstats.maxX = Lon[Point];
			if (Lon[Point] < cpstats.minX)
				cpstats.minX = Lon[Point];
			cpstats.sumX += Lon[Point];
			cpstats.sumX2 += Lon[Point] * Lon[Point];
			PLink->Latitude = Lat[Point];
			PLink->Longitude = Lon[Point];
			PLink->Elevation = (float)Elev[Point];
#ifndef WCS_BUILD_VNS
			if (Lat[Point] > MyNorth)
				{
				MyNorth = (float)Lat[Point];
				} // if
			if (Lat[Point] < MySouth)
				{
				MySouth = (float)Lat[Point];
				} // if
			if (Lon[Point] > MyWest)
				{
				MyWest = (float)Lon[Point];
				} // if
			if (Lon[Point] < MyEast)
				{
				MyEast = (float)Lon[Point];
				} // if
#endif // !WCS_BUILD_VNS
			PLink = PLink->Next;
			} // for

#ifndef WCS_BUILD_VNS
		AverageGuy->NWLat = MyNorth;
		AverageGuy->NWLon = MyWest;
		AverageGuy->SELat = MySouth;
		AverageGuy->SELon = MyEast;
#endif // !WCS_BUILD_VNS

		/***
		AverageGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
		if (!Importing->DBRenderFlag)
			AverageGuy->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
		***/
		AvgFlags = WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED;
		if (isPoint)
			AvgFlags |= WCS_JOEFLAG_ISCONTROL;
		if (Importing->DBRenderFlag)
			AvgFlags |= WCS_JOEFLAG_RENDERENABLED;
		AverageGuy->SetFlags(AvgFlags);
		AverageGuy->SetLineStyle(Style);
		AverageGuy->SetLineWidth((unsigned char)1);
		AverageGuy->SetRGB(RGB[Color][0], RGB[Color][1], RGB[Color][2]);
		#ifdef WCS_BUILD_VNS
		if (NewCoordSys)
			//AverageGuy->AddEffect(NewCoordSys, -1);
			AverageGuy->AddCoordSysFastDangerous(NewCoordSys, -1);
		AverageGuy->RecheckBounds();
		#endif // WCS_BUILD_VNS
		} // if
	} // if

return (AverageGuy);

} // ImportWizGUI::SetSHPObjTriple

/*===========================================================================*/

// I don't feel like passing all these between Import Shape & AvgShapeStuff
static long  NumSelected, NumSelected2, NumSelected3, NumParts, PartNum;
static long NumDB3Layers;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
//static long NumDB3Layers4;
static long NumSelected4, *LayerStart4;
static DB3LayerList *LayerList4;
static unsigned char *LayerLength4 = NULL;
static char *DBRecord4;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
static char TargetName[256];
static unsigned long ObjsImported;
static unsigned char *LayerLength, *LayerLength2, *LayerLength3;
static long *LayerStart, *LayerStart2, *LayerStart3;
static short DBRecordRead = 0;
static unsigned short RecordLength;
static char *DBRecord;
static long putback;
static char shapeTypeName[WCS_DXF_MAX_LAYERLENGTH];

/*===========================================================================*/

short ImportWizGUI::AvgShapeStuff(Joe *FileParent, Joe *AverageGuy, Joe *AfterGuy, FILE *fDBF)
{
short BailOut = 0, j;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
short i;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
LayerEntry *TargetEntry;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
LayerStub *Stubby;
char *LayerDude;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

FileParent->AddChild(AverageGuy, AfterGuy);
ObjsImported++;

// indicate what kind of shape file the object came from for easier debugging
if (shapeTypeName[0] && (TargetEntry = DBHost->DBLayers.MatchMakeLayer(shapeTypeName, 0)))
	{
	AverageGuy->AddObjectToLayer(TargetEntry);
	} // if

// currently only used by EDSS
if (Importing->LayerName[0] && (TargetEntry = DBHost->DBLayers.MatchMakeLayer(Importing->LayerName, 0)))
	{
	AverageGuy->AddObjectToLayer(TargetEntry);
	} // if

if (NumSelected)
	{
	if (! DBRecordRead)
		{
		if (NumSelected2)
			fseek(fDBF, putback, SEEK_SET);
		if (ReadDB3Record(fDBF, DBRecord, RecordLength))
			{
			DBRecordRead = 1;
			for (j = 0; j < NumSelected; j ++)
				{
				strncpy(TargetName, &DBRecord[LayerStart[j]], LayerLength[j]);
				TargetName[LayerLength[j]] = 0;
				(void)TrimTrailingSpaces(TargetName);
				if (TargetName[0] && (TargetEntry = DBHost->DBLayers.MatchMakeLayer(TargetName, 0)))
					{
					AverageGuy->AddObjectToLayer(TargetEntry);
					} // if
				} // for
			} // if
		else
			BailOut = 1;
		} // if
	else
		{
		for (j = 0; j < NumSelected; j++)
			{
			strncpy(TargetName, &DBRecord[LayerStart[j]], LayerLength[j]);
			TargetName[LayerLength[j]] = 0;
			(void)TrimTrailingSpaces(TargetName);
			if (TargetName[0] && (TargetEntry = DBHost->DBLayers.MatchMakeLayer(TargetName, 0)))
				{
				AverageGuy->AddObjectToLayer(TargetEntry);
				} // if
			} // for
		} // else
	} // if
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (NumSelected4)
	{
	for (i = j = 0; i < NumDB3Layers && j < NumSelected4; i ++)
		{
		LayerDude = (char *)&LayerList4[i];
		if ((UseGUI && LayerDude[31]) || Importing->Scratch4[i])	// selected?
			{
			strncpy(TargetName, &DBRecord4[LayerStart4[j]], LayerLength4[j]);
			TargetName[LayerLength4[j]] = 0;
			(void)TrimTrailingSpaces(TargetName);
			if (TargetName[0])
				{
				// see if we have a numeric field
				if ((LayerDude[11] == 'N') || (LayerDude[11] == 'F'))
					AverageGuy->AddIEEEAttribute(LayerDude, atof(TargetName));
				else
					AverageGuy->AddTextAttribute(LayerDude, TargetName);
				} // if TargetName
			j++;
			} // if selected
		} // for j
	} // if NumSelected4

// add an attribute if it is a multipart polygon
// attribute should be file name plus part number
if (NumParts > 1)
	{
	sprintf(TargetName, "%1d", PartNum);

	char name[32], timeString[16];
	struct tm *newTime;
	time_t szClock;
	// try to create a unique ID from project name & time
	strcpy(name, "Link_");
	strncat(name, GlobalApp->MainProj->projectname, 8);
	time(&szClock); // Get time in seconds
	newTime = gmtime(&szClock);
	sprintf(timeString, "_%d%02d%02d%02d%02d%02d", newTime->tm_year, newTime->tm_mon + 1, newTime->tm_mday,
			newTime->tm_hour, newTime->tm_min, newTime->tm_sec);
	strcat(name, timeString);

	if (Stubby = AverageGuy->AddTextAttribute(name, TargetName))
		Stubby->MyLayer()->SetFlags(WCS_LAYER_LINKATTRIBUTE);
	} // if
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

// Conform to loaded terrain
if (Importing->Conform)
	AverageGuy->ConformToTopo();

return BailOut;

} // ImportWizGUI::AvgShapeStuff

/*===========================================================================*/

unsigned long ImportWizGUI::ImportShape(FILE *fShape, FILE *fDBF, FILE *fIndex, Joe *FileParent, char *FileName)
{
double Northest, Southest, Eastest, Westest;
double *TempLon, *TempLat, Bounds[4];
double measure, z;	// Mmin, Mmax
float *TempElev;
BusyWin *Position = NULL;
Joe *AverageGuy = NULL, *AfterGuy = NULL;
DB3LayerList *LayerList = NULL, *LayerList2 = NULL, *LayerList3 = NULL;
DB3LayersGUI *LayerGUI = NULL;
DB3OneLayerGUI *Layer2GUI = NULL;
DB3OneLayerGUI *Layer3GUI = NULL;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
DB3LayersGUI *Layer4GUI = NULL;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
long IndexRec = 0, MaxParts = 0, *PartIndex, TotalRecs = 0, FirstVectorPoint;
long FileCode, Version, ShapeType, NumPoints, ContLength, RecNum = 0, LastPartPoint, PointCt, Vertex, MaxPoints = 0;
short i, j, k, BailOut = 0, ElevLayer, WestNeg = 0;	// NameLayer
long fpos, rpos, zseek;	// file position, record position, seek for z value
#ifdef DEBUG
#ifdef BAILEARLY
unsigned long bail_count = 0;
#endif // BAILEARLY
#endif // DEBUG
long putback3 = 0;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
long putback4 = 0;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
#ifdef SHAPE_TRACE
long LastRecNum, where;
char logmsg[80];
#endif // SHAPE_TRACE
#ifdef WCS_BUILD_VNS
char FlipSignX = FALSE;
#else // WCS_BUILD_VNS
short RefSys, UTMZone;
double value;
#endif // WCS_BUILD_VNS
long offset;
//time_t time1, time2;
NotifyTag Changes[2];
bool makeCP = false;
// DBRecord = string containing a complete row of fields
// LayerStart = character position that a field starts at
// LayerLength = length of a field in characters
char str[256], ObjName[256], Layer[WCS_DXF_MAX_LAYERS][WCS_DXF_MAX_LAYERLENGTH], Color, Style = 4, DBaseObjName[256];
char *DBRecord2 = NULL, *DBRecord3 = NULL, *LayerDude;
char UTMRejectStr[] = ":;*?/`#%";
char BailMsg[40];

NumDB3Layers = 0;
NumSelected = 0, NumSelected2 = 0, NumSelected3 = 0;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
//NumDB3Layers4 = 0;
NumSelected4 = 0; LayerStart4 = NULL;
LayerList4 = NULL;
DBRecord4 = NULL;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
ObjsImported = 0;
LayerLength = LayerLength2 = LayerLength3 = NULL;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
LayerLength4 = NULL;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
LayerStart = LayerStart2 = LayerStart3 = NULL;
DBRecordRead = 0;
RecordLength = 0;
DBRecord = NULL;
shapeTypeName[0] = 0;

FirstVectorPoint = Joe::GetFirstRealPtNum();

#ifdef WCS_BUILD_VNS
if (Importing->IWCoordSys.GetGeographic() && Importing->PosEast)
	FlipSignX = TRUE;
#endif // WCS_BUILD_VNS

if (Importing->NameBase[0] == 0)
	{
	strcpy(Importing->NameBase, FileName);
	PointCt = 1;
	while ((unsigned long)PointCt < strlen(Importing->NameBase))
		{
		if (Importing->NameBase[PointCt] == '.')
			{
			Importing->NameBase[PointCt] = 0;
			break;
			} // if
		PointCt ++;
		} // while
	} // if
strcpy(ObjName, Importing->NameBase);

if (Importing->TestOnly)
	{
	AttributeList[0] = 0;
	if (fDBF)
		{
		NumDB3Layers = ReadDB3LayerTable(fDBF, LayerList, RecordLength);
		if (NumDB3Layers && LayerList)
			{
			strcat(AttributeList, "\r\r\nAttributes:\r\n");
			for (i = 0; i < NumDB3Layers; i++)
				{
				LayerDude = (char *)&LayerList[i];
				strcat(AttributeList, LayerDude);
				strcat(AttributeList, "\r\n");
				} // for
			} // if
		if (LayerList)
			FreeDB3LayerTable(LayerList, NumDB3Layers);
		LayerList = NULL;
		} // if
	return 1;
	} // if

TempLon = (double *)AppMem_Alloc(WCS_DXF_MAX_OBJPTS * sizeof (double), 0);
TempLat = (double *)AppMem_Alloc(WCS_DXF_MAX_OBJPTS * sizeof (double), 0);
TempElev = (float *)AppMem_Alloc(WCS_DXF_MAX_OBJPTS * sizeof (float), APPMEM_CLEAR);
PartIndex = (long *)AppMem_Alloc(WCS_DXF_MAX_OBJPTS * sizeof (long), 0);

if (! TempLon || ! TempLat || ! TempElev || ! PartIndex)
	{
	UserMessageOK("Import Wizard: Import Shape", "Out of memory allocating temporary arrays!\nOperation terminated.");
	goto Cleanup;
	} // if no database loaded

#ifndef WCS_BUILD_VNS
fseek(fShape, 52L, SEEK_SET);
value = GetL64(fShape);	// read Xmax
if (value > 100000)
	Importing->HasUTM = TRUE;
else
	{
	value = GetL64(fShape);	// read Ymax
	if (value > 100000)
		Importing->HasUTM = TRUE;
	} // else

// RefSys = 0 for Lat/Lon, 1 for UTM, 2 for arbitrary
if (Importing->HasUTM)
	{
	RefSys = UserMessageCustom("Import Wizard: Import Shape",
		"Your data looks like it's in UTM Coordinates.\r\r\nIs this correct?", "Yes", "No", NULL, 1);
	} // if
else
	{
	RefSys = UserMessageCustom("Import Wizard: Import Shape",
		"Your data looks like it's in Lat/Lon Coordinates.\r\r\nIs this correct?", "Yes", "No", NULL, 1);
	RefSys = 1 - RefSys;
	} // else

if (! RefSys)
	{
	//WestNeg = !UserMessageCustom("Import Wizard: Import Shape", "Direction of positive longitude?", "West", "East", NULL, 0);
	WestNeg = Importing->PosEast;
	}

if (RefSys == 1)
	{
	sprintf(str, "%d", GlobalApp->MainProj->Prefs.LastUTMZone);
	if (GetInputString("Enter the UTM Zone number (1-60, negative if southern hemisphere) if you know it.", UTMRejectStr, str))
		{
		UTMZone = (short)atoi(str);
		GlobalApp->MainProj->Prefs.LastUTMZone = UTMZone;
		Importing->HasUTM = TRUE;
		}
	else
		goto Cleanup;
	UTMLatLonCoords_Init(&VCoords.UTM, UTMZone);
	} // if 
#endif // !WCS_BUILD_VNS

 /*** F2 Note: LayerDude[31] should probably be used to store the selection signals of all 3 cases,
      but I don't have the time to make all the code changes required & test for any side effects,
	  so I'm using multiple LayerLists instead ***/
if (fDBF)
	{
	NumDB3Layers = ReadDB3LayerTable(fDBF, LayerList, RecordLength);
	#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
//#ifdef WCS_BUILD_VNS
	if (Importing->ShapeAttribs)
//#else // WCS_BUILD_VNS
//	if (UserMessageYN("Import Shape File", "Load Attributes?", 1))
//#endif // WCS_BUILD_VNS
		{
		rewind(fDBF);
		ReadDB3LayerTable(fDBF, LayerList4, RecordLength);
		if (NumDB3Layers)
			{
			if (! UseGUI)
				{
				NumSelected4 = 0;
				for (i = 0; i < 2048; i++)
					{
					if (Importing->Scratch4[i])
						NumSelected4++;
					} // for
				if (NumSelected4)
					{
					if ((LayerStart4 = (long *)AppMem_Alloc(NumSelected4 * sizeof (long), 0)) &&
						(DBRecord4 = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength4 = (unsigned char *)AppMem_Alloc(NumSelected4, 0)))
						{
						for (i = j = 0, offset = 0; i < NumDB3Layers && j < NumSelected4; i ++)
							{
							LayerDude = (char *)&LayerList4[i];
							if (Importing->Scratch4[i])	// layer selected flag
								{
								LayerStart4[j] = offset;
								LayerLength4[j] = (unsigned char)LayerDude[16];
								j ++;
								} // if
							offset += (unsigned char)LayerDude[16];
							} // for
						} // if
					} // if
				} // if TemplateLoad
			else if (Layer4GUI = new DB3LayersGUI(LayerList4, NumDB3Layers, &NumSelected4))
				{
				Layer4GUI->WidgetSetText(IDC_LABEL_PROJ, "Select fields for Attribute Load");
				while (! Layer4GUI->CheckAbort()) {};
				delete Layer4GUI;
				if (NumSelected4)
					{
					if ((LayerStart4 = (long *)AppMem_Alloc(NumSelected4 * sizeof (long), 0)) &&
						(DBRecord4 = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength4 = (unsigned char *)AppMem_Alloc(NumSelected4, 0)))
						{
						for (i = j = 0; i < NumDB3Layers && j < NumSelected4; i ++)
							{
							LayerDude = (char *)&LayerList4[i];
							if (LayerDude[31])	// layer selected flag
								{
								LayerStart4[j] = *((long *)&LayerDude[24]);
								LayerLength4[j] = (unsigned char)LayerDude[16];
								j ++;
								Importing->Scratch4[i] = 1;	// flag this as selected in the byte field
								} // if
							} // for
						} // if
					} // if
				} // else
			} // if
		} // if attaching attributes to objects
	#endif // WCS_SUPPORT_GENERIC_ATTRIBS
//#ifdef WCS_BUILD_VNS
	if (Importing->ShapeLayers)
//#else // WCS_BUILD_VNS
//	if (UserMessageYN("Import Shape File", "Attach Layers to objects?", 1))
//#endif // WCS_BUILD_VNS
		{
//		rewind(fDBF);
//		ReadDB3LayerTable(fDBF, LayerList, RecordLength);
		if (NumDB3Layers)
			{
			if (! UseGUI)
				{
				NumSelected = 0;
				for (i = 0; i < 2048; i++)
					{
					if (Importing->Scratch1[i])
						NumSelected++;
					} // for
				if (NumSelected)
					{
					if ((LayerStart = (long *)AppMem_Alloc(NumSelected * sizeof (long), 0)) &&
						(DBRecord = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength = (unsigned char *)AppMem_Alloc(NumSelected, 0)))
						{
						for (i = j = 0, offset = 0; i < NumDB3Layers && j < NumSelected; i ++)
							{
							LayerDude = (char *)&LayerList[i];
							if (Importing->Scratch1[i])	// layer selected flag
								{
								LayerStart[j] = offset;
								LayerLength[j] = (unsigned char)LayerDude[16];
								j ++;
								} // if
							offset += (unsigned char)LayerDude[16];
							} // for
						} // if
					} // if
				} // if Template Load
			else if (LayerGUI = new DB3LayersGUI(LayerList, NumDB3Layers, &NumSelected))
				{
				while (! LayerGUI->CheckAbort()) {};
				delete LayerGUI;
				if (NumSelected)
					{
					if ((LayerStart = (long *)AppMem_Alloc(NumSelected * sizeof (long), 0)) &&
						(DBRecord = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength = (unsigned char *)AppMem_Alloc(NumSelected, 0)))
						{
						for (i = j = 0; i < NumDB3Layers && j < NumSelected; i ++)
							{
							LayerDude = (char *)&LayerList[i];
							if (LayerDude[31])	// layer selected flag
								{
								LayerStart[j] = *((long *)&LayerDude[24]);
								LayerLength[j] = (unsigned char)LayerDude[16];
								j ++;
								Importing->Scratch1[i] = 1;	// flag this as selected in the byte field
								} // if
							} // for
						} // if
					} // if
				} // if
			} // if
		} // if attaching layers to objects
	if (Importing->Flags != GET_ELEV_UNITS)	// if we have a 3D shape file, don't need this
		{
//#ifdef WCS_BUILD_VNS
		if (Importing->ShapeElevs)
//#else // WCS_BUILD_VNS
//		if (UserMessageYN("Import Shape File", "Assign elevations from an attribute field?", 1))
//#endif // WCS_BUILD_VNS
			{
			rewind(fDBF);
			ReadDB3LayerTable(fDBF, LayerList2, RecordLength);
			if (NumDB3Layers)
				{
				if (! UseGUI)
					{
					NumSelected2 = 0;
					for (i = 0; i < 2048; i++)
						{
						if (Importing->Scratch2[i])
							NumSelected2++;
						} // for
					if (NumSelected2)
						{
						if ((LayerStart2 = (long *)AppMem_Alloc(NumSelected2 * sizeof (long), 0)) &&
							(DBRecord2 = (char *)AppMem_Alloc(RecordLength, 0)) &&
							(LayerLength2 = (unsigned char *)AppMem_Alloc(NumSelected2, 0)))
							{
							for (i = j = 0, offset = 0; i < NumDB3Layers && j < NumSelected2; i ++)
								{
								LayerDude = (char *)&LayerList2[i];
								if (Importing->Scratch2[i])	// layer selected flag
									{
									LayerStart2[j] = offset;
									LayerLength2[j] = (unsigned char)LayerDude[16];
									j ++;
									} // if
								offset += (unsigned char)LayerDude[16];
								} // for
							} // if
						} // if
					} // if Template Load
				else if (Layer2GUI = new DB3OneLayerGUI(LayerList2, NumDB3Layers, &NumSelected2))
					{
					while (! Layer2GUI->CheckAbort()) {};
					delete Layer2GUI;
					if (NumSelected2)
						{
						if ((LayerStart2 = (long *)AppMem_Alloc(NumSelected2 * sizeof (long), 0)) &&
							(DBRecord2 = (char *)AppMem_Alloc(RecordLength, 0)) &&
							(LayerLength2 = (unsigned char *)AppMem_Alloc(NumSelected2, 0)))
							{
							for (i = j = 0; i < NumDB3Layers && j < 1; i ++)
								{
								LayerDude = (char *)&LayerList2[i];
								if (LayerDude[31])	// layer selected flag
									{
									ElevLayer = i;
									LayerStart2[0] = *((long *)&LayerDude[24]);
									LayerLength2[0] = (unsigned char)LayerDude[16];
									j ++;
									Importing->Scratch2[i] = 1;	// flag this as selected in the byte field
									} // if
								} // for
							} // if
						} // if
					} // if
				} // if
			} // if assigning elevations from field contents
		}
//#ifdef WCS_BUILD_VNS
	if (Importing->ShapeDBNames)
//#else // WCS_BUILD_VNS
//	if (UserMessageYN("Import Shape File", "Assign database names from an attribute field?", 1))
//#endif // WCS_BUILD_VNS
		{
		rewind(fDBF);
		ReadDB3LayerTable(fDBF, LayerList3, RecordLength);
		if (NumDB3Layers)
			{
			if (! UseGUI)
				{
				NumSelected3 = 0;
				for (i = 0; i < 2048; i++)
					{
					if (Importing->Scratch3[i])
						NumSelected3++;
					}
				if (NumSelected3)
					{
					if ((LayerStart3 = (long *)AppMem_Alloc(NumSelected3 * sizeof (long), 0)) &&
						(DBRecord3 = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength3 = (unsigned char *)AppMem_Alloc(NumSelected3, 0)))
						{
						for (i = j = 0, offset = 0; i < NumDB3Layers && j < NumSelected3; i ++)
							{
							LayerDude = (char *)&LayerList3[i];
							if (Importing->Scratch3[i])	// layer selected flag
								{
								LayerStart3[j] = offset;
								LayerLength3[j] = (unsigned char)LayerDude[16];
								j ++;
								} // if
							offset += (unsigned char)LayerDude[16];
							} // for
						} // if
					} // if
				} // if Template Load
			else if (Layer3GUI = new DB3OneLayerGUI(LayerList3, NumDB3Layers, &NumSelected3))
				{
				Layer3GUI->SetCaption("Select a field to assign names from");
				while (! Layer3GUI->CheckAbort()) {};
				delete Layer3GUI;
				if (NumSelected3)
					{
					if ((LayerStart3 = (long *)AppMem_Alloc(NumSelected3 * sizeof (long), 0)) &&
						(DBRecord3 = (char *)AppMem_Alloc(RecordLength, 0)) &&
						(LayerLength3 = (unsigned char *)AppMem_Alloc(NumSelected3, 0)))
						{
						for (i = j = 0; i < NumDB3Layers && j < 1; i ++)
							{
							LayerDude = (char *)&LayerList3[i];
							if (LayerDude[31])	// layer selected flag
								{
								LayerStart3[0] = *((long *)&LayerDude[24]);
								LayerLength3[0] = (unsigned char)LayerDude[16];
								j ++;
								Importing->Scratch3[i] = 1;	// flag this as selected in the byte field
								} // if
							} // for
						} // if
					} // if
				} // if
			} // if
		} // if assigning database names from field contents
	} // if database file

ObjName[0] = 0;
for (i = 0; i < WCS_DXF_MAX_LAYERS; i++)
	Layer[i][0] = 0;

strcpy(ObjName, Importing->NameBase);
PointCt = 1;
while ((unsigned long)PointCt < strlen(ObjName))
	{
	if (ObjName[PointCt] == '.')
		{
		ObjName[PointCt] = 0;
		break;
		} // if
	PointCt ++;
	} // while

// Minimize the extents
Northest = -90.0;
Southest = 90.0;
Eastest = 360.0;
Westest = -360.0;

fseek(fShape, 0L, SEEK_END);
Position = new BusyWin ("Import Shape", ftell(fShape), 'IDLG', 0);
fseek(fShape, 0L, SEEK_SET);
// read file header
FileCode = GetB32S(fShape);
fseek(fShape, 28, SEEK_SET);
Version = GetL32S(fShape);
ShapeType = GetL32S(fShape);
Bounds[0] = GetL64(fShape);
Bounds[1] = GetL64(fShape);
Bounds[2] = GetL64(fShape);
Bounds[3] = GetL64(fShape);
fseek(fShape, 100, SEEK_SET);
if (fIndex)
	{
	long tmp;

	fseek(fIndex, 24, SEEK_SET);	// seek to the File Length
	tmp = GetB32S(fIndex);
	TotalRecs = (tmp - 50) / 4;		// TotalRecs = total # words - 50 words for header, divided by 4
	} // if

//GetTime(time1);

if (Importing->LoadAs == LAS_CP)
	{
	Style = 0;
	makeCP = true;
	} // if
else
	Style = 4;
Color = 3;
// read records
while (! BailOut)
	{
	#ifdef SHAPE_TRACE
	where = ftell(fShape);
	// these values should jive with the offsets listed in the .shx file
	sprintf(&logmsg[0], "@ %8x {hex words}, (%d bytes)", where / 2, where);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
	LastRecNum = RecNum;
	sprintf(&logmsg[0], "--> LastRecNum = %d", LastRecNum);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
	#endif // SHAPE_TRACE
	// if the shapefile index exists, use that to seek to records, since shapefiles have been found with records longer than the content length
	if (fIndex)
		{
		long NdxContLength, WordSeek;

		fseek(fIndex, 100 + (IndexRec << 3), SEEK_SET);
		WordSeek = GetB32S(fIndex);
		NdxContLength = GetB32S(fIndex);
		fseek(fShape, WordSeek * 2, SEEK_SET);
		IndexRec++;
		RecNum = GetB32S(fShape);
		ContLength = GetB32S(fShape);
		if (RecNum != IndexRec)
			{
			sprintf(BailMsg, "Record number mismatch found");
			UserMessageOK("Import Wizard", BailMsg, REQUESTER_ICON_STOP);
			BailOut = 1;
			} // if
		else if (NdxContLength != ContLength)
			{
			sprintf(BailMsg, "Content length mismatch found");
			UserMessageOK("Import Wizard", BailMsg, REQUESTER_ICON_STOP);
			BailOut = 1;
			} // else
		} // if
	else
		{
		RecNum = GetB32S(fShape);
		ContLength = GetB32S(fShape);
		} // else
	if (BailOut)
		break;
	rpos = ftell(fShape);	// where in the file this record (including shape type) starts at
	ShapeType = GetL32S(fShape);
	#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
	if (NumSelected4)
		{
		putback = ftell(fDBF);
		if (putback4 == 0)
			putback4 = putback;
		fseek(fDBF, putback4, SEEK_SET);
		if (ReadDB3Record(fDBF, DBRecord4, RecordLength))
			{
			putback4 = ftell(fDBF);
			fseek(fDBF, putback, SEEK_SET);
			} // if
		} // NumSelected4
	#endif // WCS_SUPPORT_GENERIC_ATTRIBS
	// load or create a database name
	if (NumSelected3)
		{
		putback = ftell(fDBF);
		if (putback3 == 0)
			putback3 = putback;
		fseek(fDBF, putback3, SEEK_SET);
		if (ReadDB3Record(fDBF, DBRecord3, RecordLength))
			{
			strncpy(DBaseObjName, &DBRecord3[LayerStart3[0]], LayerLength3[0]);
			DBaseObjName[LayerLength3[0]] = 0;
			TrimTrailingSpaces(DBaseObjName);
			putback3 = ftell(fDBF);
			fseek(fDBF, putback, SEEK_SET);
			} // if
		else
			{
			BailOut = 1;
			break;
			} // else
		} // if
	else
		sprintf(DBaseObjName, "%s%1d", ObjName, RecNum);
	DBRecordRead = 0;	// FPW2 091201 - This seems like this needs to be done for each object, should be able to remove other clears later
	switch (ShapeType)
		{
		case WCS_ARCVIEW_SHAPETYPE_NULL:
			{
			DBRecordRead = 0;
			if (NumSelected2)
				{
				putback = ftell(fDBF);
				if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
					{
					strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
					TargetName[LayerLength2[0]] = 0;
					TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
					} // if
				else
					{
					BailOut = 1;
					break;
					} // else
				} // if
			if (BailOut)
				break;
			strcpy(Layer[0], "Null");
			NumParts = 0; 
			NumPoints = 0; 
			// save it here
			if (AverageGuy = SetNullObject(DBaseObjName, &Layer[0][0], Style, Color))
				{
				AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
				#ifdef WCS_BUILD_V3
				AfterGuy = AverageGuy; // ensure we keep same order
				#endif // WCS_BUILD_V3
				// no bound computations
				} // if
			break;
			} // WCS_ARCVIEW_SHAPETYPE_NULL
		case WCS_ARCVIEW_SHAPETYPE_POINT:
			{
			strcpy(shapeTypeName, "Point");
			TempLon[FirstVectorPoint] = GetL64(fShape);
			TempLat[FirstVectorPoint] = GetL64(fShape);
			if (GetFileResult() != 1)
				break;
			TempElev[FirstVectorPoint] = 0.0f;
			NumParts = 0;
			NumPoints = 1;
			// scale linear units
			TempLat[FirstVectorPoint] *= Importing->HScale;
			TempLon[FirstVectorPoint] *= Importing->HScale;
#ifdef WCS_BUILD_VNS
			if (FlipSignX)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
#else // WCS_BUILD_VNS
			if (! RefSys && WestNeg)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
			else if (RefSys == 1)
				{
				VCoords.UTM.East = TempLon[FirstVectorPoint];
				VCoords.UTM.North = TempLat[FirstVectorPoint];
				UTM_LatLon(&VCoords.UTM);
				TempLon[FirstVectorPoint] = VCoords.UTM.Lon;
				TempLat[FirstVectorPoint] = VCoords.UTM.Lat;
				} // if UTM
#endif // WCS_BUILD_VNS
			// save it here
			#ifdef WCS_JOE_LABELPOINTEXISTS
			TempLon[0] = TempLon[FirstVectorPoint];
			TempLat[0] = TempLat[FirstVectorPoint];
			TempElev[0] = 0.0f;
			#endif // WCS_JOE_LABELPOINTEXISTS
			if (NumSelected2)
				{
				putback = ftell(fDBF);		// where NumSelected wants to be reading from
				if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
					{
//					for (j = 0; j < ElevLayer; j ++)
//						strncpy(TargetName, &DBRecord2[LayerStart2[j]], LayerLength2[j]);
//					TargetName[LayerLength2[j]] = 0;
					strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
					TargetName[LayerLength2[0]] = 0;
					(void)TrimTrailingSpaces(TargetName);
					TempElev[FirstVectorPoint] = (float)(atof(TargetName) * Importing->VScale);
					} // if
				else
					{
					BailOut = 1;
					break;
					} // else
				} // if NumSelected2
			if (AverageGuy = SetSHPObjTriple(DBaseObjName, NumPoints, Style, Color, TempLat, TempLon, TempElev, makeCP))
				{
				AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
				#ifdef WCS_BUILD_V3
				AfterGuy = AverageGuy; // ensure we keep same order
				#endif // WCS_BUILD_V3
				if (AverageGuy->NWLat > Northest)
					{
					Northest = AverageGuy->NWLat;
					} // if
				if (AverageGuy->SELat < Southest)
					{
					Southest = AverageGuy->SELat;
					} // if
				if (AverageGuy->SELon < Eastest)
					{
					Eastest = AverageGuy->SELon;
					} // if
				if (AverageGuy->NWLon > Westest)
					{
					Westest = AverageGuy->NWLon;
					} // if
				} // if AverageGuy
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POINT
		case WCS_ARCVIEW_SHAPETYPE_POLYLINE:
			{
			strcpy(shapeTypeName, "PolyLine");
			TempElev[0] = 0.0f;
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			DBRecordRead = 0;
			if (NumSelected2)
				{
				putback = ftell(fDBF);
				if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
					{
					strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
					TargetName[LayerLength2[0]] = 0;
					TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
					} // if
				else
					{
					BailOut = 1;
					break;
					} // else
				} // if
			if (BailOut)
				break;
			for (PartNum = 0; PartNum < NumParts; PartNum++)
				{
				PartIndex[PartNum] = GetL32S(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				} // for
			if (BailOut)
				break;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1)
				PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						/***
						if (NumSelected2)
							{
							putback = ftell(fDBF);
							if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
								{
								strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
								TargetName[LayerLength2[0]] = 0;
								TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
								for (k = 0; k < WCS_DXF_MAX_OBJPTS; k++)
									TempElev[k] = TempElev[0];
								}
							else
								{
								BailOut = 1;
								break;
								} // else
							}
						***/
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if AverageGuy
						PointCt = FirstVectorPoint;
						} // if
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					TempElev[PointCt] = 0.0f;
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for Vertex
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					/***
					if (NumSelected2)
						{
						putback = ftell(fDBF);
						if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
							{
							strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
							TargetName[LayerLength2[0]] = 0;
							TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
							for (k = 0; k < PointCt; k++)
								TempElev[k] = TempElev[0];
							}
						else
							{
							BailOut = 1;
							break;
							} // else
						} // if
					***/
					if (NumSelected2)
						{
						for (k = 0; k < PointCt; k++)
							TempElev[k] = TempElev[0];
						} // if
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if AverageGuy
					} // if (PointCt > 1)
				} // for PartNum
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYLINE
		case WCS_ARCVIEW_SHAPETYPE_POLYGON:
			{
			strcpy(shapeTypeName, "Polygon");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			DBRecordRead = 0;
			if (NumSelected2)
				{
				putback = ftell(fDBF);
				if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
					{
					strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
					TargetName[LayerLength2[0]] = 0;
					TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
					} // if
				else
					{
					BailOut = 1;
					break;
					} // else
				} // if
			if (BailOut)
				break;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (fread(&PartIndex[PartNum], 4, 1, fShape) != 1)
					{
					BailOut = 1;
					break;
					}
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip32S(PartIndex[PartNum], &PartIndex[PartNum]);
				#endif // BYTEORDER_BIGENDIAN
				} // for
			if (BailOut)
				break;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1) PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						/***
						if (NumSelected2)
							{
							putback = ftell(fDBF);
							if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
								{
								strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
								TargetName[LayerLength2[0]] = 0;
								TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
								for (k = 0; k < WCS_DXF_MAX_OBJPTS; k++)
									TempElev[k] = TempElev[0];
								}
							else
								{
								BailOut = 1;
								break;
								} // else
							}
						***/
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if AverageGuy
						PointCt = FirstVectorPoint;
						} // if
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					TempElev[PointCt] = 0.0;
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for Vertex
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					/***
					if (NumSelected2)
						{
						putback = ftell(fDBF);
						if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
							{
							strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
							TargetName[LayerLength2[0]] = 0;
							TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
							for (k = 0; k < PointCt; k++)
								TempElev[k] = TempElev[0];
							}
						else
							{
							BailOut = 1;
							break;
							} // else
						} // if
					***/
					if (NumSelected2)
						{
						for (k = 0; k < PointCt; k++)
							TempElev[k] = TempElev[0];
						} // if
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if AverageGuy
					} // if (PointCt > FirstVectorPoint)
				} // for PartNum
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYGON
		case WCS_ARCVIEW_SHAPETYPE_MULTIPOINT:
			{
			strcpy(shapeTypeName, "MultiPoint");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumPoints = GetL32S(fShape);
			DBRecordRead = 0;
			if (NumSelected2)
				{
				putback = ftell(fDBF);
				if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
					{
					strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
					TargetName[LayerLength2[0]] = 0;
					TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
					} // if
				else
					{
					BailOut = 1;
					break;
					} // else
				} // if
			if (BailOut)
				break;
			for (Vertex = 0, PointCt = FirstVectorPoint; Vertex < NumPoints; Vertex ++, PointCt ++)
				{
				if (PointCt >= WCS_DXF_MAX_OBJPTS)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					/***
					if (NumSelected2)
						{
						putback = ftell(fDBF);
						if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
							{
							strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
							TargetName[LayerLength2[0]] = 0;
							TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
							for (k = 0; k < WCS_DXF_MAX_OBJPTS; k++)
								TempElev[k] = TempElev[0];
							}
						else
							{
							BailOut = 1;
							break;
							} // else
						}
					***/
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					PointCt = FirstVectorPoint;
					} // if
				TempLon[PointCt] = GetL64(fShape);
				TempLat[PointCt] = GetL64(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				TempElev[PointCt] = 0.0f;
				// scale linear units
				TempLat[PointCt] *= Importing->HScale;
				TempLon[PointCt] *= Importing->HScale;
#ifdef WCS_BUILD_VNS
				if (FlipSignX)
					TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
				if (! RefSys && WestNeg)
					TempLon[PointCt] = -TempLon[PointCt];
				if (RefSys == 1)
					{
					VCoords.UTM.East = TempLon[PointCt];
					VCoords.UTM.North = TempLat[PointCt];
					UTM_LatLon(&VCoords.UTM);
					TempLon[PointCt] = VCoords.UTM.Lon;
					TempLat[PointCt] = VCoords.UTM.Lat;
					} // if UTM
#endif // WCS_BUILD_VNS
				} // for
			NumParts = 0;
			// save it here
			if (PointCt > FirstVectorPoint)
				{
				#ifdef WCS_JOE_LABELPOINTEXISTS
				TempLon[0] = TempLon[FirstVectorPoint];
				TempLat[0] = TempLat[FirstVectorPoint];
				#endif // WCS_JOE_LABELPOINTEXISTS
				/***
				if (NumSelected2)
					{
					putback = ftell(fDBF);
					if (ReadDB3Record(fDBF, DBRecord2, RecordLength))
						{
						strncpy(TargetName, &DBRecord2[LayerStart2[0]], LayerLength2[0]);
						TargetName[LayerLength2[0]] = 0;
						TempElev[0] = (float)(atof(TargetName) * Importing->VScale);
						for (k = 0; k < PointCt; k++)
							TempElev[k] = TempElev[0];
						}
					else
						{
						BailOut = 1;
						break;
						} // else
					}
				***/
				if (NumSelected2)
					{
					for (k = 0; k < PointCt; k++)
						TempElev[k] = TempElev[0];
					} // if
				if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
					{
					AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
					#ifdef WCS_BUILD_V3
					AfterGuy = AverageGuy; // ensure we keep same order
					#endif // WCS_BUILD_V3
					if (AverageGuy->NWLat > Northest)
						{
						Northest = AverageGuy->NWLat;
						} // if
					if (AverageGuy->SELat < Southest)
						{
						Southest = AverageGuy->SELat;
						} // if
					if (AverageGuy->SELon < Eastest)
						{
						Eastest = AverageGuy->SELon;
						} // if
					if (AverageGuy->NWLon > Westest)
						{
						Westest = AverageGuy->NWLon;
						} // if
					} // if AverageGuy
				} // if PointCt
			break;
			} // WCS_ARCVIEW_SHAPETYPE_MULTIPOINT
		case WCS_ARCVIEW_SHAPETYPE_POINTZ:
			{
			strcpy(shapeTypeName, "PointZ");
			TempLon[FirstVectorPoint] = GetL64(fShape);
			TempLat[FirstVectorPoint] = GetL64(fShape);
			z = GetL64(fShape);
			measure = GetL64(fShape);
			if (GetFileResult() != 1)
				break;
			NumParts = 0;
			NumPoints = 1;
			// scale linear units
			TempLat[FirstVectorPoint] *= Importing->HScale;
			TempLon[FirstVectorPoint] *= Importing->HScale;
			TempElev[FirstVectorPoint] = (float)(z * Importing->VScale);
#ifdef WCS_BUILD_VNS
			if (FlipSignX)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
#else // WCS_BUILD_VNS
			if (! RefSys && WestNeg)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
			else if (RefSys == 1)
				{
				VCoords.UTM.East = TempLon[FirstVectorPoint];
				VCoords.UTM.North = TempLat[FirstVectorPoint];
				UTM_LatLon(&VCoords.UTM);
				TempLon[FirstVectorPoint] = VCoords.UTM.Lon;
				TempLat[FirstVectorPoint] = VCoords.UTM.Lat;
				} // if UTM
#endif // WCS_BUILD_VNS
			// save it here
			#ifdef WCS_JOE_LABELPOINTEXISTS
			TempLon[0] = TempLon[FirstVectorPoint];
			TempLat[0] = TempLat[FirstVectorPoint];
			TempElev[0] = TempElev[FirstVectorPoint];
			#endif // WCS_JOE_LABELPOINTEXISTS
			if (AverageGuy = SetSHPObjTriple(DBaseObjName, NumPoints, Style, Color, TempLat, TempLon, TempElev, makeCP))
				{
				AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
				#ifdef WCS_BUILD_V3
				AfterGuy = AverageGuy; // ensure we keep same order
				#endif // WCS_BUILD_V3
				if (AverageGuy->NWLat > Northest)
					{
					Northest = AverageGuy->NWLat;
					} // if
				if (AverageGuy->SELat < Southest)
					{
					Southest = AverageGuy->SELat;
					} // if
				if (AverageGuy->SELon < Eastest)
					{
					Eastest = AverageGuy->SELon;
					} // if
				if (AverageGuy->NWLon > Westest)
					{
					Westest = AverageGuy->NWLon;
					} // if
				} // if
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POINTZ
		case WCS_ARCVIEW_SHAPETYPE_POLYLINEZ:
			{
			strcpy(shapeTypeName, "PolyLineZ");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			zseek = rpos + 44 + (4 * NumParts);	// position of Byte "X"
			zseek += (16 * NumPoints) + 16;		// position of Zarray[0]
			DBRecordRead = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (fread(&PartIndex[PartNum], 4, 1, fShape) != 1)
					{
					BailOut = 1;
					break;
					} // if
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip32S(PartIndex[PartNum], &PartIndex[PartNum]);
				#endif // BYTEORDER_BIGENDIAN
				} // for
			if (BailOut)
				break;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1)
				PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						TempElev[0] = TempElev[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if
						PointCt = FirstVectorPoint;
						} // if
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					fpos = ftell(fShape);
					// the offset in the file to ZArray[Point#]
					fseek(fShape, zseek, SEEK_SET);
					zseek += 8;	// position to next Z value for next seek
					z = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					fseek(fShape, fpos, SEEK_SET);	// put us back where we were
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
					TempElev[PointCt] = (float)(z * Importing->VScale);
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for Vertex
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					} // if PointCt > FirstVectorPoint
				} // for PartNum
				// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYLINEZ
		case WCS_ARCVIEW_SHAPETYPE_POLYGONZ:
			{
			strcpy(shapeTypeName, "PolygonZ");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			zseek = rpos + 44 + (4 * NumParts);	// position of byte "X"
			zseek += (16 * NumPoints);			// position of byte "Y"
			zseek += 16;						// position of Zarray[0]
			DBRecordRead = 0;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1)
				PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				PartIndex[PartNum] = GetL32S(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				} // for
			if (BailOut)
				break;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						TempElev[0] = TempElev[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if
						PointCt = FirstVectorPoint;
						} // if PointCt >= WCS_DXF_MAX_OBJPTS
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					fpos = ftell(fShape);
					// the offset in the file to Zarray[point#]
					fseek(fShape, zseek, SEEK_SET);
					zseek += 8;	// position to next Z value for next seek
					z = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					fseek(fShape, fpos, SEEK_SET);	// put us back where we were
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
					TempElev[PointCt] = (float)(z * Importing->VScale);
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
						 	{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					} // if PointCt > FirstVectorPoint
				} // for PartNum
			// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYGONZ
		case WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ:
			{
			strcpy(shapeTypeName, "MultiPointZ");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumPoints = GetL32S(fShape);
			zseek = rpos + 40 + (16 * NumPoints);	// position of byte "X"
			zseek += 16;							// position of Zarray[0]
			DBRecordRead = 0;
			for (Vertex = 0, PointCt = FirstVectorPoint; Vertex < NumPoints; Vertex ++, PointCt ++)
				{
				if (PointCt >= WCS_DXF_MAX_OBJPTS)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					PointCt = FirstVectorPoint;
					} // if
				TempLon[PointCt] = GetL64(fShape);
				TempLat[PointCt] = GetL64(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				fpos = ftell(fShape);
				// the offset in the file to Zarray[point#]
				fseek(fShape, zseek, SEEK_SET);
				zseek += 8;	// position to next Z value for next seek
				z = GetL64(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				fseek(fShape, fpos, SEEK_SET);	// put us back where we were
				// scale linear units
				TempLat[PointCt] *= Importing->HScale;
				TempLon[PointCt] *= Importing->HScale;
				TempElev[PointCt] = (float)(z * Importing->VScale);
#ifdef WCS_BUILD_VNS
				if (FlipSignX)
					TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
				if (! RefSys && WestNeg)
					TempLon[PointCt] = -TempLon[PointCt];
				if (RefSys == 1)
					{
					VCoords.UTM.East = TempLon[PointCt];
					VCoords.UTM.North = TempLat[PointCt];
					UTM_LatLon(&VCoords.UTM);
					TempLon[PointCt] = VCoords.UTM.Lon;
					TempLat[PointCt] = VCoords.UTM.Lat;
					} // if UTM
#endif // WCS_BUILD_VNS
				} // for
			NumParts = 0;
			// save it here
			if (PointCt > FirstVectorPoint)
				{
				#ifdef WCS_JOE_LABELPOINTEXISTS
				TempLon[0] = TempLon[FirstVectorPoint];
				TempLat[0] = TempLat[FirstVectorPoint];
				TempElev[0] = TempElev[FirstVectorPoint];
				#endif // WCS_JOE_LABELPOINTEXISTS
				if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
					{
					AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
					#ifdef WCS_BUILD_V3
					AfterGuy = AverageGuy; // ensure we keep same order
					#endif // WCS_BUILD_V3
					if (AverageGuy->NWLat > Northest)
						{
						Northest = AverageGuy->NWLat;
						} // if
					if (AverageGuy->SELat < Southest)
						{
						Southest = AverageGuy->SELat;
						} // if
					if (AverageGuy->SELon < Eastest)
						{
						Eastest = AverageGuy->SELon;
						} // if
					if (AverageGuy->NWLon > Westest)
						{
						Westest = AverageGuy->NWLon;
						} // if
					} // if
				} // if PointCt
			// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ
		case WCS_ARCVIEW_SHAPETYPE_POINTM:
			{
			strcpy(shapeTypeName, "PointM");
			TempLon[FirstVectorPoint] = GetL64(fShape);
			TempLat[FirstVectorPoint] = GetL64(fShape);
			measure = GetL64(fShape);
			if (GetFileResult() != 1)
				break;
			TempElev[FirstVectorPoint] = 0.0f;
			NumParts = 0;
			NumPoints = 1;
			// scale linear units
			TempLat[FirstVectorPoint] *= Importing->HScale;
			TempLon[FirstVectorPoint] *= Importing->HScale;
#ifdef WCS_BUILD_VNS
			if (FlipSignX)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
#else // WCS_BUILD_VNS
			if (! RefSys && WestNeg)
				TempLon[FirstVectorPoint] = -TempLon[FirstVectorPoint];
			else if (RefSys == 1)
				{
				VCoords.UTM.East = TempLon[FirstVectorPoint];
				VCoords.UTM.North = TempLat[FirstVectorPoint];
				UTM_LatLon(&VCoords.UTM);
				TempLon[FirstVectorPoint] = VCoords.UTM.Lon;
				TempLat[FirstVectorPoint] = VCoords.UTM.Lat;
				} // if UTM
#endif // WCS_BUILD_VNS
			// save it here
			#ifdef WCS_JOE_LABELPOINTEXISTS
			TempLon[0] = TempLon[FirstVectorPoint];
			TempLat[0] = TempLat[FirstVectorPoint];
			#endif // WCS_JOE_LABELPOINTEXISTS
			if (AverageGuy = SetSHPObjTriple(DBaseObjName, NumPoints, Style, Color, TempLat, TempLon, TempElev, makeCP))
				{
				AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
				#ifdef WCS_BUILD_V3
				AfterGuy = AverageGuy; // ensure we keep same order
				#endif // WCS_BUILD_V3
				if (AverageGuy->NWLat > Northest)
					{
					Northest = AverageGuy->NWLat;
					} // if
				if (AverageGuy->SELat < Southest)
					{
					Southest = AverageGuy->SELat;
					} // if
				if (AverageGuy->SELon < Eastest)
					{
					Eastest = AverageGuy->SELon;
					} // if
				if (AverageGuy->NWLon > Westest)
					{
					Westest = AverageGuy->NWLon;
					} // if
				} // if
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POINTM
		case WCS_ARCVIEW_SHAPETYPE_POLYLINEM:
			{
			strcpy(shapeTypeName, "PolyLineM");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			DBRecordRead = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (fread(&PartIndex[PartNum], 4, 1, fShape) != 1)
					{
					BailOut = 1;
					break;
					} // if
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip32S(PartIndex[PartNum], &PartIndex[PartNum]);
				#endif // BYTEORDER_BIGENDIAN
				} // for
			if (BailOut)
				break;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1)
				PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						TempElev[0] = TempElev[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if
						PointCt = FirstVectorPoint;
						} // if
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
					TempElev[PointCt] = 0.0;
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for Vertex
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					} // if PointCt > FirstVectorPoint
				} // for PartNum
			// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYLINEM
		case WCS_ARCVIEW_SHAPETYPE_POLYGONM:
			{
			strcpy(shapeTypeName, "PolygonM");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumParts = GetL32S(fShape);
			NumPoints = GetL32S(fShape);
			DBRecordRead = 0;
			// This is an attempt to read improperly formatted IDRISI shapefiles who
			// write single-part entites with the first PartIndex field set to 1 instead
			// of 0 as they should be. -CXH/FW2
			if (NumParts == 1)
				PartIndex[0] = 0;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				PartIndex[PartNum] = GetL32S(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				} // for
			if (BailOut)
				break;
			for (PartNum = 0; PartNum < NumParts; PartNum ++)
				{
				if (PartNum < NumParts - 1)
					LastPartPoint = PartIndex[PartNum + 1] - 1;
				else
					LastPartPoint = NumPoints - 1;
				for (Vertex = PartIndex[PartNum], PointCt = FirstVectorPoint; Vertex <= LastPartPoint; Vertex ++, PointCt ++)
					{
					if (PointCt >= WCS_DXF_MAX_OBJPTS)
						{
						#ifdef WCS_JOE_LABELPOINTEXISTS
						TempLon[0] = TempLon[FirstVectorPoint];
						TempLat[0] = TempLat[FirstVectorPoint];
						TempElev[0] = TempElev[FirstVectorPoint];
						#endif // WCS_JOE_LABELPOINTEXISTS
						if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
							{
							AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
							#ifdef WCS_BUILD_V3
							AfterGuy = AverageGuy; // ensure we keep same order
							#endif // WCS_BUILD_V3
							if (AverageGuy->NWLat > Northest)
								{
								Northest = AverageGuy->NWLat;
								} // if
							if (AverageGuy->SELat < Southest)
								{
								Southest = AverageGuy->SELat;
								} // if
							if (AverageGuy->SELon < Eastest)
								{
								Eastest = AverageGuy->SELon;
								} // if
							if (AverageGuy->NWLon > Westest)
								{
								Westest = AverageGuy->NWLon;
								} // if
							} // if
						PointCt = FirstVectorPoint;
						} // if PointCt >= WCS_DXF_MAX_OBJPTS
					TempLon[PointCt] = GetL64(fShape);
					TempLat[PointCt] = GetL64(fShape);
					if (GetFileResult() != 1)
						{
						BailOut = 1;
						break;
						} // if
					// scale linear units
					TempLat[PointCt] *= Importing->HScale;
					TempLon[PointCt] *= Importing->HScale;
					TempElev[PointCt] = 0.0;
#ifdef WCS_BUILD_VNS
					if (FlipSignX)
						TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
					if (! RefSys && WestNeg)
						TempLon[PointCt] = -TempLon[PointCt];
					else if (RefSys == 1)
						{
						VCoords.UTM.East = TempLon[PointCt];
						VCoords.UTM.North = TempLat[PointCt];
						UTM_LatLon(&VCoords.UTM);
						TempLon[PointCt] = VCoords.UTM.Lon;
						TempLat[PointCt] = VCoords.UTM.Lat;
						} // if UTM
#endif // WCS_BUILD_VNS
					} // for
				// save it here
				if (PointCt > FirstVectorPoint)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
						 	{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					} // if PointCt > FirstVectorPoint
				} // for PartNum
			// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_POLYGONM
		case WCS_ARCVIEW_SHAPETYPE_MULTIPOINTM:
			{
			strcpy(shapeTypeName, "MultiPointM");
			Bounds[0] = GetL64(fShape);
			Bounds[1] = GetL64(fShape);
			Bounds[2] = GetL64(fShape);
			Bounds[3] = GetL64(fShape);
			NumPoints = GetL32S(fShape);
			//zseek = rpos + 40 + (16 * NumPoints);
			DBRecordRead = 0;
			for (Vertex = 0, PointCt = FirstVectorPoint; Vertex < NumPoints; Vertex ++, PointCt ++)
				{
				if (PointCt >= WCS_DXF_MAX_OBJPTS)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					TempLon[0] = TempLon[FirstVectorPoint];
					TempLat[0] = TempLat[FirstVectorPoint];
					TempElev[0] = TempElev[FirstVectorPoint];
					#endif // WCS_JOE_LABELPOINTEXISTS
					if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
						{
						AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
						#ifdef WCS_BUILD_V3
						AfterGuy = AverageGuy; // ensure we keep same order
						#endif // WCS_BUILD_V3
						if (AverageGuy->NWLat > Northest)
							{
							Northest = AverageGuy->NWLat;
							} // if
						if (AverageGuy->SELat < Southest)
							{
							Southest = AverageGuy->SELat;
							} // if
						if (AverageGuy->SELon < Eastest)
							{
							Eastest = AverageGuy->SELon;
							} // if
						if (AverageGuy->NWLon > Westest)
							{
							Westest = AverageGuy->NWLon;
							} // if
						} // if
					PointCt = FirstVectorPoint;
					} // if
				TempLon[PointCt] = GetL64(fShape);
				TempLat[PointCt] = GetL64(fShape);
				if (GetFileResult() != 1)
					{
					BailOut = 1;
					break;
					} // if
				// scale linear units
				TempLat[PointCt] *= Importing->HScale;
				TempLon[PointCt] *= Importing->HScale;
				TempElev[PointCt] = 0.0;
#ifdef WCS_BUILD_VNS
				if (FlipSignX)
					TempLon[PointCt] = -TempLon[PointCt];
#else // WCS_BUILD_VNS
				if (! RefSys && WestNeg)
					TempLon[PointCt] = -TempLon[PointCt];
				if (RefSys == 1)
					{
					VCoords.UTM.East = TempLon[PointCt];
					VCoords.UTM.North = TempLat[PointCt];
					UTM_LatLon(&VCoords.UTM);
					TempLon[PointCt] = VCoords.UTM.Lon;
					TempLat[PointCt] = VCoords.UTM.Lat;
					} // if UTM
#endif // WCS_BUILD_VNS
				} // for
			NumParts = 0;
			// save it here
			if (PointCt > FirstVectorPoint)
				{
				#ifdef WCS_JOE_LABELPOINTEXISTS
				TempLon[0] = TempLon[FirstVectorPoint];
				TempLat[0] = TempLat[FirstVectorPoint];
				TempElev[0] = TempElev[FirstVectorPoint];
				#endif // WCS_JOE_LABELPOINTEXISTS
				if (AverageGuy = SetSHPObjTriple(DBaseObjName, PointCt - FirstVectorPoint, Style, Color, TempLat, TempLon, TempElev, makeCP))
					{
					AvgShapeStuff(FileParent, AverageGuy, AfterGuy, fDBF);
					#ifdef WCS_BUILD_V3
					AfterGuy = AverageGuy; // ensure we keep same order
					#endif // WCS_BUILD_V3
					if (AverageGuy->NWLat > Northest)
						{
						Northest = AverageGuy->NWLat;
						} // if
					if (AverageGuy->SELat < Southest)
						{
						Southest = AverageGuy->SELat;
						} // if
					if (AverageGuy->SELon < Eastest)
						{
						Eastest = AverageGuy->SELon;
						} // if
					if (AverageGuy->NWLon > Westest)
						{
						Westest = AverageGuy->NWLon;
						} // if
					} // if
				} // if PointCt
			// since we're not reading all the record, seek to the end of the record
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			fseek(fShape, zseek, SEEK_SET);
			break;
			} // WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ
		// this is currently defined in the specs, but unimplemented
		case WCS_ARCVIEW_SHAPETYPE_MULTIPATCH:
			zseek = rpos + (ContLength * 2); // beginning of record + record size in words
			sprintf(BailMsg, "Unhandled Shape Type #%ld", ShapeType);
			UserMessageOK("Import Wizard", BailMsg, REQUESTER_ICON_STOP);
			fseek(fShape, zseek, SEEK_SET);
			break;
		default:
			BailOut = 1;
			sprintf(BailMsg, "Unknown Shape Type #%ld", ShapeType);
			UserMessageOK("Import Wizard", BailMsg, REQUESTER_ICON_STOP);
			break;
		} // switch ShapeType

// test stuff
	if (NumParts > MaxParts)
		MaxParts = NumParts;
	if (NumPoints > MaxPoints)
		MaxPoints = NumPoints;
	if (Position)
		{
		Position->Update(ftell(fShape));
		BailOut += Position->CheckAbort();
		} // if
	if (fIndex && (IndexRec == TotalRecs))	// see if we're done
		BailOut = 1;

#ifdef DEBUG
#ifdef BAILEARLY
	bail_count++;
	if (bail_count == 1200)	// set to last record number to read
		BailOut = 1;
#endif // BAILEARLY
#endif // DEBUG

	} // while

//test stuff
sprintf(str, "Shape '%s': Max Points = %d, Max Parts = %d, Records = %d", Importing->NameBase, MaxPoints, MaxParts, RecNum);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, str);

if (FileParent)
	{
	FileParent->NWLat = Northest;
	FileParent->NWLon = Westest;
	FileParent->SELat = Southest;
	FileParent->SELon = Eastest;
	DBHost->BoundUpTree(FileParent);
	} // if

Cleanup:

if (Position)
	{
	delete Position;
	Position = NULL;
	} // if

if (TempLon)
	AppMem_Free(TempLon, WCS_DXF_MAX_OBJPTS * sizeof(double));
if (TempLat)
	AppMem_Free(TempLat, WCS_DXF_MAX_OBJPTS * sizeof(double));
if (TempElev)
	AppMem_Free(TempElev, WCS_DXF_MAX_OBJPTS * sizeof(float));
if (PartIndex)
	AppMem_Free(PartIndex, WCS_DXF_MAX_OBJPTS * sizeof(long));
if (LayerList)
	FreeDB3LayerTable(LayerList, NumDB3Layers);
if (LayerList2)
	FreeDB3LayerTable(LayerList2, NumDB3Layers);
if (LayerList3)
	FreeDB3LayerTable(LayerList3, NumDB3Layers);
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (LayerList4)
	FreeDB3LayerTable(LayerList4, NumDB3Layers);
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
if (LayerStart)
	AppMem_Free(LayerStart, NumSelected * sizeof(long));
if (LayerStart2)
	AppMem_Free(LayerStart2, NumSelected2 * sizeof(long));
if (LayerStart3)
	AppMem_Free(LayerStart3, NumSelected3 * sizeof(long));
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (LayerStart4)
	AppMem_Free(LayerStart4, NumSelected4 * sizeof(long));
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
if (DBRecord)
	AppMem_Free(DBRecord, RecordLength);
if (DBRecord2)
	AppMem_Free(DBRecord2, RecordLength);
if (DBRecord3)
	AppMem_Free(DBRecord3, RecordLength);
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (DBRecord4)
	AppMem_Free(DBRecord4, RecordLength);
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
if (LayerLength)
	AppMem_Free(LayerLength, NumSelected);
if (LayerLength2)
	AppMem_Free(LayerLength2, NumSelected2);
if (LayerLength3)
	AppMem_Free(LayerLength3, NumSelected3);
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (LayerLength4)
	AppMem_Free(LayerLength4, NumSelected4);
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

sprintf(str, "Imported %d Shape Objects.", ObjsImported);
GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_VEC_LOAD, str);

if (ObjsImported && AverageGuy)
	{
	Changes[0] = MAKE_ID(AverageGuy->GetNotifyClass(), AverageGuy->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, AverageGuy->GetRAHostRoot());
	} // if

//GetTime(time2);
//sprintf(str, "Time = %d", time2 - time1);
//GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_VEC_LOAD, str);

return (ObjsImported);

} // ImportWizGUI::ImportShape

/*===========================================================================*/

long ImportWizGUI::ReadDB3LayerTable(FILE *fDBF, DB3LayerList *&LayerList, unsigned short &RecordLength)
{
long LayersRead = 0, LayersToRead;
//long where;
unsigned char Version;
char LayerName[32];

LayerName[0] = 0;

if (((fread((char *)&Version, 1, 1, fDBF)) == 1) && Version == 3)
	{
	//where = ftell(fDBF);
	if (! fseek(fDBF, 10, SEEK_SET))
		{
		//where = ftell(fDBF);
		if (((fread((char *)&RecordLength, 2, 1, fDBF)) == 1) && RecordLength)
			{
			//where = ftell(fDBF);
			#ifdef BYTEORDER_BIGENDIAN
			SimpleEndianFlip16U(RecordLength, &RecordLength);
			#endif // BYTEORDER_BIGENDIAN
			if (! fseek(fDBF, 32, SEEK_SET))
				{
				//where = ftell(fDBF);
				while (LayerName[0] != 13)
					{
					if (((fread((char *)LayerName, 32, 1, fDBF)) == 1))
						{
						//where = ftell(fDBF);
						if (LayerName[0] != 13)
							LayersRead ++;
						} // if
					else
						{
						break;
						} // else
					} // while
				} // if
			if (LayerList = (DB3LayerList *)AppMem_Alloc(LayersRead * sizeof(DB3LayerList), 0))
				{
				LayersToRead = LayersRead;
				if (! fseek(fDBF, 32, SEEK_SET))
					{
					for (LayersRead = 0; LayersRead < LayersToRead; LayersRead ++)
						{
						if (((fread((char *)&LayerList[LayersRead], 32, 1, fDBF)) != 1))
							{
							break;
							} // else
						} // for
					fseek(fDBF, 2, SEEK_CUR);
					} // if
				} // if
			else
				{
				UserMessageOK("Import Wizard", "Out of memory creating Layer table!\nNo layer information is available.");
				} // else
			} // if
		} // if
	} // if
else
	{
	UserMessageOK("Import Wizard", "File is not a DBase III file.\nNo layer information is available.");
	} // else

return (LayersRead);

} // ImportWizGUI::ReadDB3LayerTable

/*===========================================================================*/

void ImportWizGUI::FreeDB3LayerTable(DB3LayerList *List, long NumLayers)
{

if (List)
	AppMem_Free(List, NumLayers * sizeof(DB3LayerList));

} // ImportWizGUI::FreeDB3LayerTable

/*===========================================================================*/

// WARNING: this reads from the current file position only!
int ImportWizGUI::ReadDB3Record(FILE *fDBF, char *DBRecord, unsigned short RecordLength)
{

if (((fread((char *)DBRecord, RecordLength, 1, fDBF)) == 1))
	{
	return (1);
	} // else

return (0);

} // ImportWizGUI::ReadDB3Record

/*===========================================================================*/

static unsigned long VectApproveHook(Joe *Scrutiny)
{
return(1);
/*if ((Scrutiny->AttribInfo.CanonicalTypeMajor == 170) &&
 (Scrutiny->AttribInfo.CanonicalTypeMinor == 210))
	{
	return(0);
	} // if
else
	{
	return(1);
	} // else
*/
} // VectApproveHook

/*===========================================================================*/

void ScaleLatitude(struct ImportData *DaData, double &Value)
{

Value = DaData->OutputLowLat + DaData->OutputYScale * (Value - DaData->InputLow[WCS_IMPORTDATA_COORD_Y]);

} // ScaleLatitude

/*===========================================================================*/

void ScaleLongitude(struct ImportData *DaData, double &Value)
{

//if (! DaData->ReverseX)
	Value = DaData->OutputLowLon + DaData->OutputXScale * (Value - DaData->InputLow[WCS_IMPORTDATA_COORD_X]);
//else
//	Value = DaData->OutputLowLon - DaData->OutputXScale * (Value - DaData->InputLow[WCS_IMPORTDATA_COORD_X]);

} // ScaleLongitude

/*===========================================================================*/

void ScaleElevation(struct ImportData *DaData, float &Value)
{

Value = (float)(DaData->OutputLowElev + DaData->OutputElScale * (Value - DaData->InputLow[WCS_IMPORTDATA_COORD_Z]));

} // ScaleElevation

/*===========================================================================*/

// each color pen index should be 256 chars
void InitDXFPens(struct DXF_Pens *dxfpens)
{
// these are the initializer arrays, used to generate the AutoCAD palette algorithmically
unsigned char R57[] = {57, 49, 33, 24, 16};
unsigned char R123[] = {123, 99, 74, 57, 33};
unsigned char R156[] = {156, 123, 90, 74, 41};
unsigned char R189[] = {189, 156, 115, 90, 57};
unsigned char R222[] = {222, 181, 132, 107, 66};
unsigned char R255[] = {255, 206, 156, 123, 74};
unsigned char G60[] = {60, 48, 36, 28, 16};
unsigned char G125[] = {125, 101, 77, 60, 36};
unsigned char G158[] = {158, 125, 93, 77, 44};
unsigned char G190[] = {190, 154, 113, 93, 56};
unsigned char G223[] = {223, 178, 134, 109, 65};
unsigned char G255[] = {255, 207, 154, 125, 77};
unsigned char B57[] = {57, 49, 33, 24, 16};
unsigned char B123[] = {123, 99, 74, 57, 33};
unsigned char B156[] = {156, 123, 90, 74, 41};
unsigned char B189[] = {189, 156, 115, 90 ,57};
unsigned char B222[] = {222, 181, 132, 107, 66};
unsigned char B255[] = {255, 206, 156, 123, 74};
unsigned char redval, grnval, bluval;
short i, j, k;

// clear for our sanity
for (i = 0; i <= 255; i++)
	{
	dxfpens->r[i] = 0;
	dxfpens->g[i] = 0;
	dxfpens->b[i] = 0;
	}

// Standard Colors
dxfpens->r[1] = 255; dxfpens->g[1] = 0; dxfpens->b[1] = 0;		// red
dxfpens->r[2] = 255; dxfpens->g[2] = 255; dxfpens->b[2] = 0;	// yellow
dxfpens->r[3] = 0; dxfpens->g[3] = 255; dxfpens->b[3] = 0;		// green
dxfpens->r[4] = 0; dxfpens->g[4] = 255; dxfpens->b[4] = 255;	// cyan
dxfpens->r[5] = 0; dxfpens->g[5] = 0; dxfpens->b[5] = 255;		// blue
dxfpens->r[6] = 255; dxfpens->g[6] = 0; dxfpens->b[6] = 255;	// magenta
dxfpens->r[7] = dxfpens->g[7] = dxfpens->b[7] = 255;			// default = black/white, I choose white
dxfpens->r[8] = 132; dxfpens->g[8] = 130; dxfpens->b[8] = 132;
dxfpens->r[9] = 198; dxfpens->g[9] = 195, dxfpens->b[9] = 198;

// Gray Shades
dxfpens->r[250] = 49; dxfpens->g[250] = 48; dxfpens->b[250] = 49;
dxfpens->r[251] = 90; dxfpens->g[251] = 89; dxfpens->b[251] = 90;
dxfpens->r[252] = 132; dxfpens->g[252] = 134; dxfpens->b[252] = 132;
dxfpens->r[253] = 173; dxfpens->g[253] = 174; dxfpens->b[253] = 173;
dxfpens->r[254] = 214; dxfpens->g[254] = 215; dxfpens->b[254] = 214;
dxfpens->r[255] = dxfpens->g[255] = dxfpens->b[255] = 255;

//lint -save -e744
for (i = 10; i < 250; i+= 10)
	{
	// do even pen numbers
	for (j = 0; j < 5; j++)
		{
		// init red gun
		switch (i)
			{
			case 10:
			case 20:
			case 30:
			case 40:
			case 50:
			case 210:
			case 220:
			case 230:
			case 240:
				redval = R255[j];
				break;
			case 60:
			case 200:
				redval = R189[j];
				break;
			case 70:
			case 190:
				redval = R123[j];
				break;
			case 80:
			case 180:
				redval = R57[j];
				break;
			default:
				redval = 0;
				break;
			} // reds
		// init green gun
		switch (i)
			{
			case 20:
			case 160:
				grnval = G60[j];
				break;
			case 30:
			case 150:
				grnval = G125[j];
				break;
			case 40:
			case 140:
				grnval = G190[j];
				break;
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
				grnval = G255[j];
				break;
			default:
				grnval = 0;
				break;
			} // greens
		// init blue gun
		switch (i)
			{
			case 100:
			case 240:
				bluval = B57[j];
				break;
			case 110:
			case 230:
				bluval = B123[j];
				break;
			case 120:
			case 220:
				bluval = B123[j];
				break;
			case 130:
			case 140:
			case 150:
			case 160:
			case 170:
			case 180:
			case 190:
			case 200:
			case 210:
				bluval = B255[j];
				break;
			default:
				bluval = 0;
				break;
			} // blues
		dxfpens->r[i + j * 2] = redval;
		dxfpens->g[i + j * 2] = grnval;
		dxfpens->b[i + j * 2] = bluval;
		}
	// do odd pen numbers
	for (k = 0; k < 5; k++)
		{
		// init red gun
		switch (i)
			{
			case 10:
			case 20:
			case 30:
			case 40:
			case 50:
			case 200:
			case 210:
			case 220:
			case 230:
			case 240:
				redval = R255[k];
				break;
			case 60:
			case 190:
				redval = R222[k];
				break;
			case 70:
			case 180:
				redval = R189[k];
				break;
			case 80:
			case 170:
				redval = R156[k];
				break;
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
			case 140:
			case 150:
			case 160:
				redval = R123[k];
				break;
			} // reds
		// init green gun
		switch (i)
			{
			case 10:
			case 170:
			case 180:
			case 190:
			case 200:
			case 210:
			case 220:
			case 230:
			case 240:
				grnval = G125[k];
				break;
			case 20:
			case 160:
				grnval = G158[k];
				break;
			case 30:
			case 150:
				grnval = G190[k];
				break;
			case 40:
			case 140:
				grnval = G223[k];
				break;
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
				grnval = G255[k];
				break;
			} // greens
		// init blue gun
		switch (i)
			{
			case 10:
			case 20:
			case 30:
			case 40:
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
				bluval = B123[k];
				break;
			case 100:
			case 240:
				bluval = B156[k];
				break;
			case 110:
			case 230:
				bluval = B189[k];
				break;
			case 120:
			case 220:
				bluval = B222[k];
				break;
			case 130:
			case 140:
			case 150:
			case 160:
			case 170:
			case 180:
			case 190:
			case 200:
			case 210:
				bluval = B255[k];
				break;
			} // blues
		dxfpens->r[i + k * 2 + 1] = redval;
		dxfpens->g[i + k * 2 + 1] = grnval;
		dxfpens->b[i + k * 2 + 1] = bluval;
		} // for k
	} // for i
//lint -restore

} // InitDXFPens

/*===========================================================================*/
