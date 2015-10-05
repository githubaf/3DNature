// InterpDEMGUI.cpp
// Code for Interpolate DEM editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "InterpDEMGUI.h"
#include "Database.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Random.h"
#include "Joe.h"
#include "DEM.h"
#include "Log.h"
#include "WCSVersion.h"
#include "AppMem.h"
#include "resource.h"

NativeGUIWin InterpDEMGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // InterpDEMGUI::Open

/*===========================================================================*/

NativeGUIWin InterpDEMGUI::Construct(void)
{

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DEM_INTERP, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		ConfigureWidgets();
		WidgetSetDisabled(IDC_INTERP, TRUE);
		} // if

	} // if
 
return(NativeWin);

} // InterpDEMGUI::Construct

/*===========================================================================*/

InterpDEMGUI::InterpDEMGUI(Project *Moi, Database *DB, EffectsLib *EffectsSource)
: GUIFenetre('WEDT', this, "DEM Interpolater") // Yes, I know...
{

ConstructError = 0;
HostProj = Moi;
DBHost = DB;
EffectsHost = EffectsSource;

FR = NULL;
elevfile[0] = 0;
strcpy(elevpath, Moi->dirname);
ElVar = 2.0;
FlatMax = 2.0;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // InterpDEMGUI::InterpDEMGUI

/*===========================================================================*/

InterpDEMGUI::~InterpDEMGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // InterpDEMGUI::~InterpDEMGUI()

/*===========================================================================*/

long InterpDEMGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_IDG, 0);

return(0);

} // InterpDEMGUI::HandleCloseWin

/*===========================================================================*/

long InterpDEMGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(090, 90);
switch (ButtonID)
	{
	case IDC_SELECT:
		{
		DoSelect();
		break;
		} // 
	case IDC_INTERP:
		{
		DoInterp();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // InterpDEMGUI::HandleButtonClick

/*===========================================================================*/

void InterpDEMGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes;
//short ListEntry;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

} // InterpDEMGUI::HandleNotifyEvent()

/*===========================================================================*/

void InterpDEMGUI::ConfigureWidgets(void)
{

ConfigureFI(NativeWin, IDC_ELVAR, &ElVar, 1.0, 0.0, 1000.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_MAXFLAT, &FlatMax, 1.0, 0.0, 1000.0, FIOFlag_Double, NULL, NULL);

} // InterpDEMGUI::ConfigureWidgets()

/*===========================================================================*/

void InterpDEMGUI::DoSelect(void)
{
char NumSelected[24];

if (FR)
	delete FR;

if (FR = new FileReq)
	{
	FR->SetDefPat(WCS_REQUESTER_PARTIALWILD("elev"));
	FR->SetDefPath(elevpath);
	if (FR->Request(WCS_REQUESTER_FILE_MULTI))
		{
		sprintf(NumSelected, "%1d", FR->FilesSelected());
		WidgetSetText(IDC_NUMSELECTED, NumSelected);
		elevfile[0] = 0;
		elevpath[0] = 0;
		strcpy(elevfile, FR->GetFirstName());
		WidgetSetText(IDC_SELECTEDFILES, elevfile);
		BreakFileName((char *)FR->GetFirstName(), elevpath, 256, elevfile, 256);
		WidgetSetDisabled(IDC_INTERP, FALSE);
		} // if
	else
		{
		sprintf(NumSelected, "%1d", 0);
		WidgetSetText(IDC_NUMSELECTED, NumSelected);
		delete FR;
		FR = NULL;
		WidgetSetText(IDC_SELECTEDFILES, "<no files selected>");
		WidgetSetDisabled(IDC_INTERP, TRUE);
		} // else
	} // if
else
	{
	sprintf(NumSelected, "%1d", 0);
	WidgetSetText(IDC_NUMSELECTED, NumSelected);
	WidgetSetText(IDC_SELECTEDFILES, "<no files selected>");
	WidgetSetDisabled(IDC_INTERP, TRUE);
	} // else

} // InterpDEMGUI::DoSelect

/*===========================================================================*/

void InterpDEMGUI::DoInterp(void)
{

InterpDEM();

} // InterpDEMGUI::DoInterp

/*===========================================================================*/

short InterpDEMGUI::InterpDEM(void)
{
double lolat, lolong;
float *arrayptr = NULL, *mapptr, *dataptr;
Joe *NewJoe = NULL;
BusyWin *BWFI;
CoordSys *MyCoords = NULL;
unsigned long i, j, k;
short filelen, error = 0, newrows, newcols, Lr, Lc;
long arraysize, maprowbytes;
DEM Topo, Rel; //, *RelEl;
NotifyTag Changes[2];
char rootfile[64], newrootfile[64], extension[32], filename[256],
	NameStash[256], RealFile[64], OrigFile[64], BaseName[64], append;

BWFI = new BusyWin("Files", FR->FilesSelected(), 'BWFI', 0);

// LINT thinks this should be initialized
newrootfile[0] = 0;

for (k = 0; k < FR->FilesSelected(); k ++)
	{
	if (k == 0)
		{
		strcpy(filename, FR->GetFirstName());
		BreakFileName((char *)filename, elevpath, 256, elevfile, 256);
		strcpy(RealFile, elevfile);
		} // if
	else
		{
		strcpy(filename, FR->GetNextName());
		BreakFileName((char *)filename, elevpath, 256, elevfile, 256);
		strcpy(RealFile, elevfile);
		} // else

	if (elevfile[0] == NULL)
		{
		UserMessageOK("Data Ops: DEM Interpolate",
			"No file(s) selected!");
		break;
		} // if no file 

//RelelRepeat:

	strcpy (OrigFile, RealFile);
	strcpy (rootfile, RealFile);
	stcgfe(extension, rootfile);
	rootfile[strlen(rootfile) - strlen(extension) - (strlen(extension) > 0 ? 1: 0)] = 0;
	if (stricmp(extension, "elev"))
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_TYPE, elevfile);
		if (! UserMessageOKCAN(elevfile,
			"Error opening file for interpolation!\nFile not a "APP_TLA" DEM\nContinue?", 1))
			{
			break;
			}
		continue;
		}

	if (! stricmp(extension, "elev"))
		{
		TrimTrailingSpaces(rootfile);
		strcpy(newrootfile, rootfile);
		} // if elev file 

	strmfp(filename, elevpath, RealFile);
	if (! Topo.LoadDEM(filename, 0, &MyCoords))
		{
		if (! UserMessageOKCAN("Data Ops: Interpolate DEM",
			"Error reading elevation file!\nContinue with next file?", 0))
			{
			error = 1;
			break;
			} // if not continue to next file 
		if (MyCoords)
			{
			delete MyCoords;
			MyCoords = NULL;
			} // if
		continue;
		} // if error reading DEM file 
	else
		{
		strcpy(NameStash, filename);
		} // else read OK

	if (! GlobalApp->MainProj->DirList_ItemExists(GlobalApp->MainProj->DL, elevpath))
		GlobalApp->MainProj->DirList_Add(GlobalApp->MainProj->DL, elevpath, 0);

	Topo.pLatStep *= .5;
	Topo.pLonStep *= .5;

	newrows = (short)(Topo.pLonEntries * 2 - 1);
	newcols = (short)(Topo.pLatEntries * 2 - 1);
	arraysize = newrows * newcols * sizeof (float);
	arrayptr = (float *)AppMem_Alloc (arraysize, APPMEM_CLEAR);
	if (arrayptr == NULL)
		{
		UserMessageOK("Data Ops: Interpolate DEM",
			"Out of memory!\nOperation terminated.");
		error = 1;
		break;
		} // if 

	mapptr = Topo.RawMap;
	dataptr = arrayptr;
	for (Lr = 0; Lr < (long)Topo.pLonEntries; Lr ++)
		{
		for (Lc = 0; Lc < (long)Topo.pLatEntries; Lc ++)
			{
			*dataptr = *mapptr;
			mapptr ++;
			dataptr += 2;
			} // for Lc=columns... 
		dataptr += (newcols - 1);
		} // for Lr=0... 

	xseed48((ULONG)(fabs(Topo.pSouthEast.Lat - (int)Topo.pSouthEast.Lat) * ULONG_MAX), (ULONG)(fabs(Topo.pNorthWest.Lon - (int)Topo.pNorthWest.Lon) * ULONG_MAX));

	if (! SplineMap(arrayptr, newrows, newcols,	ElVar / 100.0, FlatMax, Topo.GetNullReject(), Topo.NullValue()))
		{
		error = 1;
		break;
		} // if abort 

	// turn off old map in database 
	if (! stricmp(extension, "elev"))
		{
		DisableOrigDEM(rootfile);
		} // if elevation file 

	append = 'A';
	lolat = Topo.pSouthEast.Lat;
	lolong = Topo.pNorthWest.Lon;
	maprowbytes = Topo.pLatEntries * sizeof (float);
	filelen = (short)(strlen(newrootfile) - 1);
	while (newrootfile[filelen] == ' ' && filelen)
		newrootfile[filelen--] = 0;

	for (i = 0; i < 2; ++i)
		{
		for (j = 0; j < 2; ++j)
			{
			Topo.pSouthEast.Lat = lolat + j * (Topo.pLatEntries - 1) * Topo.pLatStep;	//lint !e790
			Topo.pNorthWest.Lon = lolong - i * (Topo.pLonEntries - 1) * Topo.pLonStep;	//lint !e790
			Topo.pSouthEast.Lon = Topo.pNorthWest.Lon - (Topo.pLonEntries - 1) * Topo.pLonStep;
			Topo.pNorthWest.Lat = Topo.pSouthEast.Lat + (Topo.pLatEntries - 1) * Topo.pLatStep;
			mapptr = Topo.RawMap;
			dataptr = &arrayptr[i * (Topo.pLonEntries - 1) * newcols + j * (Topo.pLatEntries - 1)];
			for (Lr = 0; Lr < (long)Topo.pLonEntries; Lr ++)
				{
				memcpy(mapptr, dataptr, maprowbytes);
				mapptr += Topo.pLatEntries;
				dataptr += newcols;
				} // for Lr=0... 
			Topo.PrecalculateCommonFactors();

			strcpy(elevfile, newrootfile);
			elevfile[filelen + 1] = append;
			elevfile[filelen + 2] = 0;
			strcpy(BaseName, elevfile);
			strcat(elevfile, ".");
			strcat(elevfile, extension);
			strmfp(filename, elevpath, elevfile);

			if (! stricmp(extension, "elev"))
				NewJoe = DBHost->AddDEMToDatabase("DEM Interpolate", BaseName, &Topo, MyCoords, HostProj, EffectsHost);
			if (Topo.SaveDEM(filename, GlobalApp->StatusLog))
				{
				NewJoe->RecheckBounds();
				GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DEM_SAVE, elevfile);
				} // if
			else
				{
				error = 1;
				} // else

			++append;
			if (error)
				break;
			} // for
		if (error)
			break;
		} // for

	Topo.FreeRawElevs();
	if (MyCoords)
		{
		delete MyCoords;
		MyCoords = NULL;
		} // if
	if (arrayptr)
		AppMem_Free(arrayptr, arraysize);
	arrayptr = NULL;
	if (error)
		break;

	if (BWFI)
		{
		if (BWFI->CheckAbort())
			{
			error = 1;
			break;
			}
		BWFI->Update(k + 1);
		} // if

	// now repeat operation for relel file or create it if it doesn't exist 
	/* saving the DEM will cause relels to be created
	if (! stricmp(extension, "elev"))
		{
		FILE *felev;

		RealFile[strlen(RealFile) - strlen(extension)] = 0;
		strcat(RealFile, "relel");
		strmfp(filename, elevpath, RealFile);
		if ((felev = PROJ_fopen(filename, "rb")) == NULL)
			{
			// need to reload the original DEM since it was destroyed and gets used in relel creation
			if (Rel.LoadRawElevs(NameStash))
				{
				if (RelEl = Rel.makerelelfile(NameStash))
					{
					RelEl->FreeRawElevs();
					delete RelEl;
					RelEl = NULL;
					Rel.FreeRawElevs();
					goto RelelRepeat;
					} // if make relel successful 
				Rel.FreeRawElevs();
				} // if
			} // if relel file not exists 
		else
			{
			fclose(felev);
			goto RelelRepeat;
			} // else file exists 
		} // if elev file 
	*/
	} // for

delete (BWFI);

if (arrayptr)
	AppMem_Free(arrayptr, arraysize);
if (MyCoords)
	{
	delete MyCoords;
	MyCoords = NULL;
	} // if

if (NewJoe)
	{
	// notify the world that the database has changed
	Changes[0] = MAKE_ID(NewJoe->GetNotifyClass(), NewJoe->GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

return ((short)(! error));

} // InterpDEMGUI::InterpDEM() 

/*===========================================================================*/

short InterpDEMGUI::SplineMap(float *map, short Xrows, short Ycols,
	double elvar, double flatmax, short NullReject, float NullValue)
{
long i, j, CurPt, NxtPt, Steps, LastStep, rowsize, Row;
double Delta = .5, P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, TempVal, RandVar;
short error = 0;
BusyWin *BWRE;

BWRE = new BusyWin("Computing", Xrows, 'BWRE', 0);


Steps = Ycols - 2;
LastStep = Steps - 2;

for (i=0; i<Xrows; i+=2)
	{
	Row = i * Ycols;
	for (j=0; j<Steps; j+=2)
		{
		CurPt = j;
		NxtPt = CurPt + 2;
		if (! NullReject || (map[Row + CurPt] != NullValue && map[Row + NxtPt] != NullValue))
			{
			P1 = map[Row + CurPt];
			P2 = map[Row + NxtPt];
			D1 = (j > 0 && (! NullReject || (map[Row + CurPt - 2] != NullValue))) ? 
				(.5 * (P2 - map[Row + CurPt - 2])): (P2 - P1);
			D2 = (j < LastStep && (! NullReject || (map[Row + NxtPt + 2] != NullValue))) ?	
				(.5 * (map[Row + NxtPt + 2] - P1)): (P2 - P1);

			S1 = Delta;
			S2 = S1 * S1;
			S3 = S1 * S2;
			h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
			h2 = -2.0 * S3 + 3.0 * S2;
			h3 = S3 - 2.0 * S2 + S1;
			h4 = S3 - S2;
			TempVal = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
			if (TempVal > P1 && TempVal > P2)
				TempVal = max(P1, P2);
			else if (TempVal < P1 && TempVal < P2)
				TempVal = min(P1, P2);
			RandVar = elvar * fabs(P2 - P1);
			if (RandVar < flatmax)
				RandVar = flatmax;
			RandVar *= GaussRand();
			map[Row + CurPt + 1] = (float)(TempVal + RandVar);
			} // if not NULL
		else
			map[Row + CurPt + 1] = NullValue;
		} // for
	} // for

Steps = Xrows - 2;
LastStep = Steps - 2;
rowsize = 2 * Ycols;

for (i=0; i<Ycols; i++)
	{
	for (j=0; j<Steps; j+=2)
		{
		CurPt = i + j * Ycols;
		NxtPt = CurPt + rowsize;
		if (! NullReject || (map[CurPt] != NullValue && map[NxtPt] != NullValue))
			{
			P1 = map[CurPt];
			P2 = map[NxtPt];
			D1 = (j > 0 && (! NullReject || (map[CurPt - rowsize] != NullValue))) ? 
				(.5 * (P2 - map[CurPt - rowsize])): (P2 - P1);
			D2 = (j < LastStep && (! NullReject || (map[NxtPt + rowsize] != NullValue))) ?	
				(.5 * (map[NxtPt + rowsize] - P1)): (P2 - P1);

			S1 = Delta;
			S2 = S1 * S1;
			S3 = S1 * S2;
			h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
			h2 = -2.0 * S3 + 3.0 * S2;
			h3 = S3 - 2.0 * S2 + S1;
			h4 = S3 - S2;
			TempVal = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
			if (TempVal > P1 && TempVal > P2)
				TempVal = max(P1, P2);
			else if (TempVal < P1 && TempVal < P2)
				TempVal = min(P1, P2);
			RandVar = elvar * fabs(P2 - P1);
			if (RandVar < flatmax)
				RandVar = flatmax;
			RandVar *= GaussRand();
			map[CurPt + Ycols] = (float)(TempVal + RandVar);
			} // if
		else
			map[CurPt + Ycols] = NullValue;
		} // for
	if (BWRE)
		{
		if (BWRE->CheckAbort())
			{
			error = 1;
			break;
			} // if
		BWRE->Update(i + 1);
		} // if
	} // for

delete (BWRE);

return ((short)(! error));

} // InterpDEMGUI::SplineMap

/*===========================================================================*/

short InterpDEMGUI::DisableOrigDEM(char *BaseName)
{
Joe *Clip;
short Found = 0;

for (Clip = DBHost->GetFirst(); Clip ; Clip = DBHost->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (! stricmp(BaseName, Clip->FileName()))
			{
			Found = 1;
			break;
			} // 
		} // if
	} // for

if (Found)
	{
	Clip->ClearFlags  (WCS_JOEFLAG_ACTIVATED);
	} // if

return (Found);

} // InterpDEMGUI::DisableOrigDEM

/*===========================================================================*/

/* now in Database.cpp
short InterpDEMGUI::AddToDatabase(char *BaseName, struct elmapheaderV101 *Hdr)
{
char NameStr[64];
short Found = 0;
Joe *Clip, *Added;
JoeDEM *MyDEM;
LayerEntry *LE;
NotifyTag ChangeEvent[2];

for (Clip = DBHost->GetFirst(); Clip ; Clip = DBHost->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (! stricmp(BaseName, Clip->FileName()))
			{
			Found = 1;
			break;
			} // 
		} // if
	} // for

if (Found)
	{
	Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
	Clip->SetLineWidth(1);
	Clip->SetLineStyle(4);
	Clip->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);
	if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		MyDEM->MaxFract = 9;
		// Transfer DEM bounds values into Joe
		Clip->NWLat = (float)(Hdr->lolat + Hdr->steplat * (Hdr->columns - 1));
		Clip->NWLon = (float)Hdr->lolong;
		Clip->SELon = (float)(Hdr->lolong - Hdr->steplong * Hdr->rows);
		Clip->SELat = (float)Hdr->lolat;

		// And into JoeDEM
		MyDEM->MaxEl = Hdr->MaxEl;
		MyDEM->MinEl = Hdr->MinEl;
		MyDEM->SumElDif = (float)Hdr->SumElDif;
		MyDEM->SumElDifSq = (float)Hdr->SumElDifSq;
		MyDEM->ElScale = Hdr->elscale;
		} // if
	Clip->ZeroUpTree();
	} // if
else
	{
	sprintf(NameStr, "%s\n%s", BaseName, BaseName);
	if (Clip = new (NameStr) Joe)
		{
		Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
		Clip->SetLineWidth(1);
		Clip->SetLineStyle(4);
		Clip->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);

		Added = DBHost->AddJoe(Clip, WCS_DATABASE_STATIC);
		if (Added)
			{
			DBHost->SetActiveObj(Clip);
			} // if
		else
			{
			UserMessageOK("DEM Interpolate", "Could not add object to Database.\nOperation terminated.");
			return (0);
			} // else

		if (LE = DBHost->DBLayers.MatchMakeLayer("TOP", 0))
			{
			Clip->AddObjectToLayer(LE);
			} // if

		//if (MyDEM = (JoeDEM *)AppMem_Alloc(sizeof(JoeDEM), APPMEM_CLEAR))
		if (MyDEM = new JoeDEM)
			{
			// Don't look too hard at this. It's intentional.
			//MyDEM->JoeDEM::JoeDEM();
			MyDEM->MaxFract = 9;
			// Transfer DEM bounds values into Joe
			Clip->NWLat = (float)(Hdr->lolat + Hdr->steplat * (Hdr->columns - 1));
			Clip->NWLon = (float)Hdr->lolong;
			Clip->SELon = (float)(Hdr->lolong - Hdr->steplong * Hdr->rows);
			Clip->SELat = (float)Hdr->lolat;

			// And into JoeDEM
			MyDEM->MaxEl = Hdr->MaxEl;
			MyDEM->MinEl = Hdr->MinEl;
			MyDEM->SumElDif = (float)Hdr->SumElDif;
			MyDEM->SumElDifSq = (float)Hdr->SumElDifSq;
			MyDEM->ElScale = Hdr->elscale;
			Clip->AddAttribute(MyDEM);
			} // if
		else
			{
			UserMessageOK("DEM Interpolate", "Could not add DEM Attribute tag.");
			} // 
		} // if
	} // else

DBHost->BoundUpTree(Clip);


ChangeEvent[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_ADDOBJ, 0, 0);
ChangeEvent[1] = NULL;

DBHost->GenerateNotify(ChangeEvent);

return (1);

} // InterpDEMGUI::AddToDatabase
*/

/*===========================================================================*/
