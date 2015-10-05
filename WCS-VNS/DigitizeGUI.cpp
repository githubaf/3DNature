// DigitizeGUI.cpp
// Code for Vector Digitizer
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "DigitizeGUI.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Database.h"
#include "Joe.h"
#include "Points.h"
#include "Layers.h"
#include "Useful.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "SceneViewGUI.h"
#include "Random.h"
#include "resource.h"

extern NotifyTag DBChangeEventPOINTS[];
extern unsigned short Default8[8];

NativeGUIWin DigitizeGUI::Open(Project *Proj)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Proj))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // DigitizeGUI::Open

/*===========================================================================*/

static const char *DigiStyleList[] = {"Point", "Circle", "Square", "Cross",
	"Solid", "Dotted", "Dashed", "Broken", NULL};
static const char *FullClassList[] = {"Terrain", "Vector", "Control Pts", NULL };
static const char *ElevSourceList[] = {"Elev. Controls", "Control Pts", "Terrain", NULL };

NativeGUIWin DigitizeGUI::Construct(void)
{
int ListEntry;
LayerEntry *Entry;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_CREATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_CREATE_SUMMARY, 0, 0);
	CreateSubWinFromTemplate(IDD_CREATE_INPUTMODE, 0, 1);
	if (AddInfo.NewCat == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		CreateSubWinFromTemplate(IDD_CREATE_CONTROLPT, 0, 2);
		CreateSubWinFromTemplate(IDD_CREATE_CONFORM, 0, 4);	// never displayed
		} // if
	else
		{
		CreateSubWinFromTemplate(IDD_CREATE_CONFORM, 0, 2);
		CreateSubWinFromTemplate(IDD_CREATE_CONTROLPT, 0, 4);	// never displayed
		} // else
	CreateSubWinFromTemplate(IDD_CREATE_VECATTRIB, 0, 3);

	if (NativeWin)
		{
		ListEntry = 0;
		WidgetTCInsertItem(IDC_TAB1, ListEntry ++, "Summary");
		WidgetTCInsertItem(IDC_TAB1, ListEntry ++, "Mouse");
		if (AddInfo.NewCat == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			WidgetTCInsertItem(IDC_TAB1, ListEntry ++, "Control Pt.");
			} // if
		else
			{
			WidgetTCInsertItem(IDC_TAB1, ListEntry ++, "Conform");
			} // if

		if ((AddInfo.Mode != WCS_DIGITIZE_DIGOBJTYPE_PATH) && (AddInfo.Mode != WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH))
			{
			WidgetTCInsertItem(IDC_TAB1, ListEntry ++, "Vector Appearance");
			} // if
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);

		for (ListEntry = 0; ListEntry < 8; ListEntry ++)
			WidgetCBInsert(IDC_STYLEDROP, -1, DigiStyleList[ListEntry]);
		for (ListEntry = 0; ListEntry < 3; ListEntry ++)
			WidgetCBInsert(IDC_ELEVSOURCEDROP, -1, ElevSourceList[ListEntry]);
		Entry = DBHost->DBLayers.FirstEntry();
		WidgetCBInsert(IDC_LAYERDROP, -1, "");
		while (Entry)
			{
			if (Entry->GetName())
				WidgetCBInsert(IDC_LAYERDROP, -1, Entry->GetName());
			Entry = DBHost->DBLayers.NextEntry(Entry);
			} // while
		SetTitle(PaletteTitle);
		WidgetSetText(IDC_CREATE_SUMMARY_LABEL, SummaryTitle);

		ShowPanel(0, ActivePage);
		ConfigProfile();
		ConfigureWidgets();
		} // if
	} // if
 
return(NativeWin);

} // DigitizeGUI::Construct

/*===========================================================================*/

DigitizeGUI::DigitizeGUI(Project *ProjSource, EffectsLib *EffectsSource, Database *DBSource, short DigitizeActive)
: GUIFenetre('DIGO', this, "Create Palette")
{
double ValRange, StdDevDefaults[3] = {1000.0, 0.0, 1.0};
NotifyTag OpenEvents[] = {MAKE_ID(WCS_PROJECTCLASS_MODULE, WCS_SUBCLASS_MODULE_OPEN, WCS_ITEM_MODULE_DIGITIZEGUI, 0),
							NULL};
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

ConstructError = 0;
DBHost = DBSource;
ProjHost = ProjSource;
EffectsHost = EffectsSource;
ActivePage = 0;
Profile = -1;

AddInfo.AddToObj = NULL;
AddInfo.HostObj  = NULL;
AddInfo.NewCat = 0;
AddInfo.DrawEnabled = AddInfo.RenderEnabled = 1;
AddInfo.LineWeight = 1;
AddInfo.LineStyle  = 4; // solid
AddInfo.Red = AddInfo.Green = AddInfo.Blue = 0;
AddInfo.Class = 1; // Vector
AddInfo.Point = AddInfo.FirstPoint = AddInfo.PointsAdded = 0;
AddInfo.Append = 0;

// <<<>>>gh new control pt variables
AddInfo.ControlPtMode = WCS_DEMDESIGN_DMODE_GRADIENT;	//WCS_DEMDESIGN_DMODE_ISOLINE;
AddInfo.ElSource = WCS_DEMDESIGN_ELSOURCE_ELEVCTRL;
AddInfo.NoReversal = 0;
AddInfo.Tension1 = 0.0;
AddInfo.Tension2 = 0.0;
AddInfo.ElevADD.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AddInfo.StdDev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AddInfo.Elev1ADT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AddInfo.Elev2ADT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AddInfo.ElevADD.AddNode(0.0, GlobalApp->MainProj->Interactive->DigElevation.CurValue, 0.0);
AddInfo.Elev1ADT.SetValue(GlobalApp->MainProj->Interactive->DigElevation.CurValue);
AddInfo.Elev2ADT.SetValue(GlobalApp->MainProj->Interactive->DigElevation.CurValue);
AddInfo.StdDev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AddInfo.StdDev.SetValue(0.0);
AddInfo.StdDev.SetRangeDefaults(StdDevDefaults);
AddInfo.Elev1ADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AddInfo.Elev2ADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

memset(&WidgetLocal, 0, sizeof WidgetLocal);
WidgetLocal.GrWin = this;
WidgetLocal.Crit = &AddInfo.ElevADD;
WidgetLocal.ValuePrototype = NULL;	// setting these to NULL will mean no value or distance labels in graph
WidgetLocal.DistancePrototype = NULL;
WidgetLocal.ActiveNode = WidgetLocal.Crit->GetFirstSelectedNode(0);
WidgetLocal.drawgrid = 0;
WidgetLocal.NumGraphs = WidgetLocal.Crit->GetNumGraphs();
WidgetLocal.HPan = 0;
WidgetLocal.HVisible = 100;
WidgetLocal.VPan = 0;
WidgetLocal.VVisible = 100;
WidgetLocal.DistGridLg = 5;
WidgetLocal.SnapToInt = 0;
WidgetLocal.DisplayByFrame = 0;
WidgetLocal.MaxDistRange = 10;
WidgetLocal.LowDrawDist = 0.0;
WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;
WidgetLocal.Crit->GetMinMaxVal(WidgetLocal.MinLowDrawVal, WidgetLocal.MaxHighDrawVal);
if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
	{
	ValRange = WidgetLocal.Crit->GetIncrement();
	WidgetLocal.MaxHighDrawVal += ValRange;
	WidgetLocal.MinLowDrawVal -= ValRange;
	} // if
else
	{
	ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .5;
	WidgetLocal.MaxHighDrawVal += ValRange;
	WidgetLocal.MinLowDrawVal -= ValRange;
	} // else
WidgetLocal.FrameRate = 1.0;

//GlobalApp->MainProj->Interactive->GetParam(&AddInfo.Mode, MAKE_ID(WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE, 0));
if (GlobalApp->MainProj->Interactive->GetDigDrawMode() == 0)
	{
	GlobalApp->MainProj->Interactive->SetDigDrawMode(1);
	} // if
Initialized = Digitizing = 0;

ProjHost->GenerateNotify(OpenEvents, this);
/*
if (! InitDigitize(DigitizeActive))
	ConstructError = 1;
*/
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
PaletteTitle[0] = SummaryTitle[0] = NULL;
SetPaletteTitle(NULL);
GlobalApp->AppEx->RegisterClient(this, AllEvents);
GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);

VectorColor.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // DigitizeGUI::DigitizeGUI

/*===========================================================================*/

DigitizeGUI::~DigitizeGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // DigitizeGUI::~DigitizeGUI()

/*===========================================================================*/
static char ComplexPaletteTitleTemp[200];

// mode argument to ConfigureDigitize
//	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_NEW = 0,
//	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE = 1,
//	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND = 2,

int DigitizeGUI::ConfigureDigitize(short Mode, long ForceCategory)
{
RasterAnimHost *RAH = NULL, *RAHRoot = NULL;
long int Cat;
RasterAnimHostProperties Prop;
char *EntName = NULL;
Joe *CloneJoe;


AddInfo.NewCat = 0;

if (! (RAH = RasterAnimHost::GetActiveRAHost()))
	{
	if (ForceCategory == 0)
		{
		RAH = GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(Cat);
		} // if
	} // if

if (RAH) // is there an active object?
	{
	RAHRoot = RAH->GetRAHostRoot();
	// check type
	Prop.PropMask  = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_NAME;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGTARGET;
	RAHRoot->GetRAHostProperties(&Prop);
	Cat = Prop.TypeNumber;
	// Can we (create and) attach a vector to this type of RasterAnimHost
	// Can I drop a vector on you?
	Prop.TypeNumber = WCS_RAHOST_OBJTYPE_VECTOR;
	Prop.PropMask   = WCS_RAHOST_MASKBIT_DROPOK;
	RAHRoot->GetRAHostProperties(&Prop);
	if (Prop.DropOK)
		{
		// Are we making a vector from a vector?
		if ((Cat == WCS_RAHOST_OBJTYPE_VECTOR) || (Cat == WCS_RAHOST_OBJTYPE_CONTROLPT))
			{
			AddInfo.Mode     = Mode;	// new = 0, replace = 1, append = 2
			AddInfo.NewCat   = Cat;
			AddInfo.AddToObj = (Joe *)RAH;
			AddInfo.HostObj  = NULL;
			CloneJoe = (Joe *)RAH;
			AddInfo.DrawEnabled	= (short)CloneJoe->TestFlags(WCS_JOEFLAG_DRAWENABLED);
			AddInfo.RenderEnabled = (short)CloneJoe->TestFlags(WCS_JOEFLAG_RENDERENABLED);
			AddInfo.Red	= CloneJoe->Red(); AddInfo.Green = CloneJoe->Green(); AddInfo.Blue = CloneJoe->Blue();
			AddInfo.LineWeight		= CloneJoe->GetLineWidth();
			AddInfo.LineStyle		= CloneJoe->GetLineStyle();
			AddInfo.Class			= CloneJoe->GetClass();
			// check see if there are points to append to
			if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND && ! (CloneJoe->GetFirstRealPoint()))
				AddInfo.Mode = WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE;
			Profile = WCS_DIGITIZE_ELEVTYPE_VECTOR;
			if (Cat == WCS_RAHOST_OBJTYPE_VECTOR)
				{
				if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE)
					{
					sprintf(ComplexPaletteTitleTemp, "Replace Vector %s.", Prop.Name);
					SetPaletteTitle(ComplexPaletteTitleTemp);
					sprintf(ComplexPaletteTitleTemp, "%s Vector Attributes", Prop.Name);
					SetSummary(ComplexPaletteTitleTemp);
					} // else
				else if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND)
					{
					sprintf(ComplexPaletteTitleTemp, "Append to Vector %s.", Prop.Name);
					SetPaletteTitle(ComplexPaletteTitleTemp);
					sprintf(ComplexPaletteTitleTemp, "%s Vector Attributes", Prop.Name);
					SetSummary(ComplexPaletteTitleTemp);
					} // else
				else
					{
					SetPaletteTitle("Create a new Vector");
					SetSummary("New Vector Attributes");
					} // else
				} // if
			else if (Cat == WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE)
					{
					sprintf(ComplexPaletteTitleTemp, "Replace Control Points %s.", Prop.Name);
					SetPaletteTitle(ComplexPaletteTitleTemp);
					sprintf(ComplexPaletteTitleTemp, "%s Control Point Attributes", Prop.Name);
					SetSummary(ComplexPaletteTitleTemp);
					} // else
				else if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND)
					{
					sprintf(ComplexPaletteTitleTemp, "Append to Control Points %s.", Prop.Name);
					SetPaletteTitle(ComplexPaletteTitleTemp);
					sprintf(ComplexPaletteTitleTemp, "%s Control Point Attributes", Prop.Name);
					SetSummary(ComplexPaletteTitleTemp);
					} // else
				else
					{
					SetPaletteTitle("Create new Control Points");
					SetSummary("New Control Point Attributes");
					} // else
				} // if
			} // if
		else
			{
			if (Cat < WCS_MAXIMPLEMENTED_EFFECTS && Cat >= WCS_EFFECTSSUBCLASS_LAKE)
				{
				// replace & append are ineffective when creating for existing effect
				if (Mode == 0)
					{
					AddInfo.Mode = WCS_DIGITIZE_DIGOBJTYPE_VECTOR_EFFECT;
					AddInfo.NewCat = Cat;
					AddInfo.AddToObj = NULL;
					AddInfo.HostObj  = RAHRoot;
					AddInfo.DrawEnabled	= 1; AddInfo.RenderEnabled = 0;
					AddInfo.Red	= 0; AddInfo.Green = 0; AddInfo.Blue = 0;
					AddInfo.LineWeight		= 0;
					AddInfo.LineStyle		= 4;
					AddInfo.Class			= 2;
					Profile = WCS_DIGITIZE_ELEVTYPE_VECTOR;
					SetAttribByCat(Cat);
					sprintf(ComplexPaletteTitleTemp, "Create a new Vector for the existing %s, %s.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), Prop.Name);
					SetPaletteTitle(ComplexPaletteTitleTemp);
					sprintf(ComplexPaletteTitleTemp, "New %s Vector Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
					SetSummary(ComplexPaletteTitleTemp);
					} // if
				else // WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE  or WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND
					{
					UserMessageOK("Digitize", "You cannot replace or append the active Item."); 
					return(0);
					} // else
				} // if
			else
				{
				UserMessageOK("Digitize", "You cannot create, replace or append the active item."); 
				return(0);
				} // else
			} // else
		} // if
	else
		{
		if ((Mode == 0) || (Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE)) // mean the same
			{
			if (Cat == WCS_JOE_ATTRIB_INTERNAL_CAMERA)
				{
				AddInfo.NewCat = Cat;
				AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_PATH;
				AddInfo.AddToObj = NULL;
				AddInfo.HostObj  = RAHRoot;
				AddInfo.DrawEnabled	= 1; AddInfo.RenderEnabled = 1;
				AddInfo.Red	= 255; AddInfo.Green = 255; AddInfo.Blue = 128;
				AddInfo.LineWeight		= 1;
				AddInfo.LineStyle		= 4;
				AddInfo.Class			= 1;
				Profile = WCS_DIGITIZE_ELEVTYPE_SMALLPLANE;
				sprintf(ComplexPaletteTitleTemp, "Create a new Path for the %s %s.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), Prop.Name);
				SetPaletteTitle(ComplexPaletteTitleTemp);
				sprintf(ComplexPaletteTitleTemp, "New %s Path Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
				SetSummary(ComplexPaletteTitleTemp);
				//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), "Path");
				if (RAHRoot)
					{
					if (EntName = RAHRoot->GetCritterName(RAH))
						{
						if (!strnicmp(EntName, "TARGET", 6))
							{
							AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH;
							sprintf(ComplexPaletteTitleTemp, "Create a new Path for the Camera Target %s.", Prop.Name);
							SetPaletteTitle(ComplexPaletteTitleTemp);
							sprintf(ComplexPaletteTitleTemp, "New %s Target Path Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
							SetSummary(ComplexPaletteTitleTemp);
							//SetPaletteTitle("Create a new Path for the Camera Target");
							} // if
						} // if
					} // if
				} // if
			else if (((Cat == WCS_JOE_ATTRIB_INTERNAL_LIGHT) || (Cat == WCS_JOE_ATTRIB_INTERNAL_CELESTIAL)))
				{
				AddInfo.NewCat = Cat;
				AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_PATH;
				AddInfo.AddToObj = NULL;
				AddInfo.HostObj  = RAHRoot;
				AddInfo.DrawEnabled	= 1; AddInfo.RenderEnabled = 1;
				AddInfo.Red	= 255; AddInfo.Green = 255; AddInfo.Blue = 128;
				AddInfo.LineWeight		= 1;
				AddInfo.LineStyle		= 4;
				AddInfo.Class			= 1;
				Profile = WCS_DIGITIZE_ELEVTYPE_CELESTIAL;
				sprintf(ComplexPaletteTitleTemp, "Create a Path for the %s %s.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), Prop.Name);
				SetPaletteTitle(ComplexPaletteTitleTemp);
				sprintf(ComplexPaletteTitleTemp, "New %s Path Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
				SetSummary(ComplexPaletteTitleTemp);
				//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), "Path");
				} // else
			} // if
		else
			{ // WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND
			UserMessageOK("Digitize", "You cannot append the active Item."); 
			return(0);
			} // else
		} // else
	} // if

if (!AddInfo.NewCat) // still not sure what to do
	{
	if (ForceCategory)
		{
		Cat = ForceCategory;
		} // if
	// Are we making a vector or a control point?
	if ((Cat == WCS_RAHOST_OBJTYPE_VECTOR) || (Cat == WCS_RAHOST_OBJTYPE_CONTROLPT))
		{
		unsigned char TempPenNum;
		AddInfo.Mode     = 0;	//Mode; can't replace or append since no vector is active
		AddInfo.NewCat   = Cat;
		AddInfo.AddToObj = NULL;
		AddInfo.HostObj  = NULL;
		AddInfo.DrawEnabled	= 1;
		AddInfo.RenderEnabled = 0;
		TempPenNum				= Cat == WCS_RAHOST_OBJTYPE_CONTROLPT ? WCS_PALETTE_DEF_WHITE: WCS_PALETTE_DEF_RED;
		AddInfo.Red				= RedPart(Default8[TempPenNum]);
		AddInfo.Green			= GreenPart(Default8[TempPenNum]);
		AddInfo.Blue			= BluePart(Default8[TempPenNum]);
		AddInfo.LineWeight		= 1;
		AddInfo.LineStyle		= Cat == WCS_RAHOST_OBJTYPE_CONTROLPT ? 6: 4;
		AddInfo.Class			= Cat == WCS_RAHOST_OBJTYPE_CONTROLPT ? 2: 1;
		Profile = WCS_DIGITIZE_ELEVTYPE_VECTOR;
		if (Cat == WCS_RAHOST_OBJTYPE_VECTOR)
			{
			SetPaletteTitle("Create a new Vector.");
			SetSummary("New Vector Attributes");
			} // if
		else if (Cat == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			SetPaletteTitle("Create new Control Points.");
			SetSummary("New Control Point Attributes");
			} // else if
		//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		} // if
	// Can we (make one and) make a path for it? (Replace/Append are ineffective)
	else if ((Mode == 0) && ((Cat == WCS_JOE_ATTRIB_INTERNAL_LIGHT) || (Cat == WCS_JOE_ATTRIB_INTERNAL_CELESTIAL)))
		{
		AddInfo.NewCat = Cat;
		AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_PATH;
		AddInfo.AddToObj = NULL;
		AddInfo.HostObj  = NULL;
		AddInfo.DrawEnabled	= 1; AddInfo.RenderEnabled = 1;
		AddInfo.Red	= 255; AddInfo.Green = 255; AddInfo.Blue = 128;
		AddInfo.LineWeight		= 1;
		AddInfo.LineStyle		= 4;
		AddInfo.Class			= 1;
		Profile = WCS_DIGITIZE_ELEVTYPE_CELESTIAL;
		sprintf(ComplexPaletteTitleTemp, "Create a new %s and a path for it.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetPaletteTitle(ComplexPaletteTitleTemp);
		sprintf(ComplexPaletteTitleTemp, "New %s Path Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetSummary(ComplexPaletteTitleTemp);
		//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		} // else
	else if ((Mode == 0) && (Cat == WCS_JOE_ATTRIB_INTERNAL_CAMERA))
		{
		AddInfo.NewCat = Cat;
		AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_PATH;
		AddInfo.AddToObj = NULL;
		AddInfo.HostObj  = RAHRoot;
		AddInfo.DrawEnabled	= 1; AddInfo.RenderEnabled = 1;
		AddInfo.Red	= 255; AddInfo.Green = 255; AddInfo.Blue = 128;
		AddInfo.LineWeight		= 1;
		AddInfo.LineStyle		= 4;
		AddInfo.Class			= 1;
		Profile = WCS_DIGITIZE_ELEVTYPE_SMALLPLANE;
		sprintf(ComplexPaletteTitleTemp, "Create a new %s and a path for it.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetPaletteTitle(ComplexPaletteTitleTemp);
		sprintf(ComplexPaletteTitleTemp, "New %s Path Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetSummary(ComplexPaletteTitleTemp);
		//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat), "Path");
		} // else
	// Can we make one and make a vector for it? (Replace/Append are ineffective)
	else if ((Mode == 0) && (RasterAnimHost::GetRADropVectorOK(Cat)))
		{
		AddInfo.NewCat = Cat;
		AddInfo.Mode     = WCS_DIGITIZE_DIGOBJTYPE_VECTOR_EFFECT;
		AddInfo.AddToObj = NULL;
		AddInfo.HostObj  = RAHRoot;
		AddInfo.DrawEnabled	= 0; AddInfo.RenderEnabled = 0;
		AddInfo.Red	= 0; AddInfo.Green = 0; AddInfo.Blue = 0;
		AddInfo.LineWeight		= 0;
		AddInfo.LineStyle		= 4;
		AddInfo.Class			= 1;
		Profile = WCS_DIGITIZE_ELEVTYPE_VECTOR;
		SetAttribByCat(Cat);
		sprintf(ComplexPaletteTitleTemp, "Create a new %s with an attached Vector.", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetPaletteTitle(ComplexPaletteTitleTemp);
		sprintf(ComplexPaletteTitleTemp, "New %s Vector Attributes", GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		SetSummary(ComplexPaletteTitleTemp);
		//SetPaletteTitle(GlobalApp->AppEffects->GetEffectTypeNameNonPlural(Cat));
		} // else
	} // if

if (AddInfo.NewCat)
	{
	Initialized = 1;
	InitDigitize(1);
	return(1);
	} // if

return(0);
} // DigitizeGUI::ConfigureDigitize

/*===========================================================================*/

void DigitizeGUI::SetAttribByCat(long Cat)
{
unsigned char TempPenNum;

AddInfo.DrawEnabled	= 1;
AddInfo.RenderEnabled = 0;

AddInfo.Red	= 0; AddInfo.Green = 0; AddInfo.Blue = 0;

AddInfo.LineWeight		= 1;
AddInfo.LineStyle		= 4;
AddInfo.Class			= 1;

switch(Cat)
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		{
		TempPenNum = WCS_PALETTE_DEF_LTBLUE;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		{
		TempPenNum = WCS_PALETTE_DEF_GREEN;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		{
		TempPenNum = WCS_PALETTE_DEF_BLUGRY;
		AddInfo.LineStyle		= 5;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		{
		TempPenNum = WCS_PALETTE_DEF_WHITE;
		AddInfo.LineStyle		= 6;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		{
		TempPenNum = WCS_PALETTE_DEF_GREEN;
		AddInfo.LineStyle		= 6;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		TempPenNum = WCS_PALETTE_DEF_RED;
		AddInfo.LineStyle		= 6;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		TempPenNum = WCS_PALETTE_DEF_RED;
		AddInfo.LineStyle		= 5;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		{
		TempPenNum = WCS_PALETTE_DEF_GREEN;
		AddInfo.LineStyle		= 1;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		{
		TempPenNum = WCS_PALETTE_DEF_DKBLUE;
		AddInfo.LineStyle		= 2;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		{
		TempPenNum = WCS_PALETTE_DEF_DKBLUE;
		AddInfo.LineStyle		= 4;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		{
		TempPenNum = WCS_PALETTE_DEF_BLACK;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		{
		TempPenNum = WCS_PALETTE_DEF_LTBLUE;
		AddInfo.LineStyle		= 7;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		{
		TempPenNum = WCS_PALETTE_DEF_GREEN;
		AddInfo.LineStyle		= 6;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		{
		TempPenNum = WCS_PALETTE_DEF_WHITE;
		break;
		} //
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		{
		TempPenNum = WCS_PALETTE_DEF_YELLOW;
		break;
		} //
	default:
		{
		TempPenNum = WCS_PALETTE_DEF_BLACK;
		break;
		} //
	} //

AddInfo.Red	  = RedPart(Default8[TempPenNum]);
AddInfo.Green = GreenPart(Default8[TempPenNum]);
AddInfo.Blue  = BluePart(Default8[TempPenNum]);

} // DigitizeGUI::SetAttribByCat

/*===========================================================================*/

long DigitizeGUI::HandleCloseWin(NativeGUIWin NW)
{

DoClose();
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DIG, 0);

return(0);

} // DigitizeGUI::HandleCloseWin

/*===========================================================================*/

long DigitizeGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		if ((NewPageID >= 0) && (NewPageID < 4))
			ShowPanel(0, NewPageID);
		break;
		}
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // DigitizeGUI::HandlePageChange

/*===========================================================================*/

long DigitizeGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(012, 12);
switch(ButtonID)
	{
	case IDC_BEHAVIOUR_ANT:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_ANT;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_PERSON:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_PERSON;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_SPARROW:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_SPARROW;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_HAWK:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_HAWK;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_EAGLE:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_EAGLE;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_PLANE:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_SMALLPLANE;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_JET:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_JUMBOJET;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_SAT:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_SATELLITE;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_CELES:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_CELESTIAL;
		ConfigProfile();
		break;
		} //
	case IDC_BEHAVIOUR_VECTOR:
		{
		Profile = WCS_DIGITIZE_ELEVTYPE_VECTOR;
		ConfigProfile();
		break;
		} //
	case IDC_INPUT_SKETCH:
		{
		SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0, WCS_DIGITIZE_ADDPOINTS_SKETCH, 0);
		break;
		} //
	case IDC_INPUT_CONNECT:
		{
		SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0, WCS_DIGITIZE_ADDPOINTS_CONNECT, 0);
		break;
		} //
	case IDC_INPUT_POINT:
		{
		SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0, WCS_DIGITIZE_ADDPOINTS_SINGLE, 0);
		break;
		} //
	case IDC_CHECKNOGREDREV:
		{
		AddInfo.NoReversal = WidgetGetCheck(IDC_CHECKNOGREDREV);
		break;
		} //
	case IDC_RADIOCONTOUR:
		{
		AddInfo.ControlPtMode = WCS_DEMDESIGN_DMODE_ISOLINE;
		break;
		} //
	case IDC_RADIOBREAKLINE:
		{
		AddInfo.ControlPtMode = WCS_DEMDESIGN_DMODE_GRADIENT;
		break;
		} //
	case IDC_ENABLED3:
		{
		AddInfo.RenderEnabled = WidgetGetCheck(IDC_ENABLED3);
		break;
		} //
	case IDC_ENABLED2:
		{
		AddInfo.DrawEnabled = WidgetGetCheck(IDC_ENABLED2);
		break;
		} //
	default: break;
	} // ButtonID

ConfigureWidgets();

return(0);

} // DigitizeGUI::HandleButtonClick

/*===========================================================================*/

long DigitizeGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_STYLEDROP:
		{
		DoLineStyle();
		break;
		}
	case IDC_ELEVSOURCEDROP:
		{
		SelectElevSource();
		break;
		}
	} // switch CtrlID

return (0);

} // DigitizeGUI::HandleCBChange

/*===========================================================================*/

long DigitizeGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
GraphNode *Node;

switch (CtrlID)
	{
	case IDC_ELEV1:
	case IDC_ELEV2:
		{
		// elevations are transferred in ConfigureWidgets()
		break;
		}
	case IDC_TENSION1:
	case IDC_TENSION2:
		{
		if (Node = AddInfo.ElevADD.GetFirstNode(0))
			{
			Node->SetTension(AddInfo.Tension1);
			if (Node->Next)
				Node->Next->SetTension(AddInfo.Tension2);
			} // if
		break;
		}
	} // switch CtrlID

ConfigureWidgets();
return (0);

} // DigitizeGUI::HandleFIChange

/*===========================================================================*/

void DigitizeGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	//ConfigureWidgets((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	// Mouse mode is in item part of NotifyTag
	AddNextPoint((DiagnosticData *)Activity->ChangeNotify->NotifyData, (Changed & 0xff00) >> 8);
	return;
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_ENDDIGITIZE, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	EndAddPoints();
	return;
	} // if
Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->MainProj->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	return;
	} // if
Interested[0] = MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// was it from our VectorColor?
	if (Activity->ChangeNotify->NotifyData == &VectorColor)
		{
		// color of vector is being changed
		AddInfo.Red = (short)WCS_round(VectorColor.CurValue[0] * 255.0);
		AddInfo.Green = (short)WCS_round(VectorColor.CurValue[1] * 255.0);
		AddInfo.Blue = (short)WCS_round(VectorColor.CurValue[2] * 255.0);
		UpdateVecColor();
		} // if
	} // if

} // DigitizeGUI::HandleNotifyEvent()

/*===========================================================================*/

static char WidgetTextBuf[90];
static char WidgetTextBufLarge[300];

void DigitizeGUI::ConfigureWidgets(void)
{
double ValRange;
short InputMode;

/*
if (AddInfo.DigObjectType == WCS_DIGITIZE_DIGOBJTYPE_VECTOR)
	{
	WidgetSetDisabled(IDC_CONNECT, FALSE);
	WidgetSetDisabled(IDC_SPEED, TRUE);
	} // if
else
	{
	WidgetSetDisabled(IDC_CONNECT, TRUE);
	WidgetSetDisabled(IDC_SPEED, FALSE);
	if (AddInfo.Mode == WCS_DIGITIZE_ADDPOINTS_CONNECT)
		AddInfo.Mode = WCS_DIGITIZE_ADDPOINTS_SKETCH;
	} // else
*/


// Configure
WidgetSetCheck(IDC_ENABLED3, AddInfo.RenderEnabled);
WidgetSetCheck(IDC_ENABLED2, AddInfo.DrawEnabled);

WidgetSNConfig(IDC_PTSPACE, &GlobalApp->MainProj->Interactive->DigSpace);
WidgetSNConfig(IDC_PTSPACETWO, &GlobalApp->MainProj->Interactive->DigSpace);

WidgetSNConfig(IDC_ELEV, &GlobalApp->MainProj->Interactive->DigElevation);
WidgetSNConfig(IDC_SMOOTH, &GlobalApp->MainProj->Interactive->DigSmooth);
WidgetSNConfig(IDC_SPEED, &GlobalApp->MainProj->Interactive->DigSpeed);

InputMode = GlobalApp->MainProj->Interactive->GetDigDrawMode();
WidgetSetCheck(IDC_INPUT_POINT, InputMode == WCS_DIGITIZE_ADDPOINTS_SINGLE);
WidgetSetCheck(IDC_INPUT_SKETCH, InputMode == WCS_DIGITIZE_ADDPOINTS_SKETCH);
WidgetSetCheck(IDC_INPUT_CONNECT, InputMode == WCS_DIGITIZE_ADDPOINTS_CONNECT);
//ConfigureSR(NativeWin, IDC_INPUT_POINT, IDC_INPUT_POINT, NULL, SRFlag_Short, WCS_DIGITIZE_ADDPOINTS_SINGLE, (SetCritter *)this, MAKE_ID(WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0));
//ConfigureSR(NativeWin, IDC_INPUT_POINT, IDC_INPUT_SKETCH, NULL, SRFlag_Short, WCS_DIGITIZE_ADDPOINTS_SKETCH, (SetCritter *)this, MAKE_ID(WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0));
//ConfigureSR(NativeWin, IDC_INPUT_POINT, IDC_INPUT_CONNECT, NULL, SRFlag_Short, WCS_DIGITIZE_ADDPOINTS_CONNECT, (SetCritter *)this, MAKE_ID(WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DRAWMODE, 0, 0));

WidgetSetCheck(IDC_BEHAVIOUR_ANT, Profile == WCS_DIGITIZE_ELEVTYPE_ANT);
WidgetSetCheck(IDC_BEHAVIOUR_PERSON, Profile == WCS_DIGITIZE_ELEVTYPE_PERSON);
WidgetSetCheck(IDC_BEHAVIOUR_SPARROW, Profile == WCS_DIGITIZE_ELEVTYPE_SPARROW);
WidgetSetCheck(IDC_BEHAVIOUR_HAWK, Profile == WCS_DIGITIZE_ELEVTYPE_HAWK);
WidgetSetCheck(IDC_BEHAVIOUR_EAGLE, Profile == WCS_DIGITIZE_ELEVTYPE_EAGLE);
WidgetSetCheck(IDC_BEHAVIOUR_PLANE, Profile == WCS_DIGITIZE_ELEVTYPE_SMALLPLANE);
WidgetSetCheck(IDC_BEHAVIOUR_JET, Profile == WCS_DIGITIZE_ELEVTYPE_JUMBOJET);
WidgetSetCheck(IDC_BEHAVIOUR_SAT, Profile == WCS_DIGITIZE_ELEVTYPE_SATELLITE);
WidgetSetCheck(IDC_BEHAVIOUR_CELES, Profile == WCS_DIGITIZE_ELEVTYPE_CELESTIAL);
WidgetSetCheck(IDC_BEHAVIOUR_VECTOR, Profile == WCS_DIGITIZE_ELEVTYPE_VECTOR);

// VecAttrib
ConfigureFI(NativeWin, IDC_LINEWT,
 &AddInfo.LineWeight,
  1.0,
   0.0,
	50.0,
	 FIOFlag_Short,
	  (SetCritter *)NULL,
	   NULL);

VectorColor.SetValue3(AddInfo.Red / 255.0, AddInfo.Green / 255.0, AddInfo.Blue / 255.0);
WidgetSmartRAHConfig(ID_COLORPOT1, &VectorColor, NULL);


// Summary
WidgetSNGetEditText(IDC_ELEV, 20, WidgetTextBuf);
WidgetSetText(IDC_SUM_ELEV, WidgetTextBuf);

WidgetSNGetEditText(IDC_SMOOTH, 20, WidgetTextBuf);
WidgetSetText(IDC_SUM_SMOOTH, WidgetTextBuf);

WidgetSNGetEditText(IDC_PTSPACETWO, 20, WidgetTextBuf);
WidgetSetText(IDC_SUM_SPTSPACE, WidgetTextBuf);
sprintf(WidgetTextBufLarge, "Connect - Add a point for each mouse click. When finished, add new points every %s along the original lines.", WidgetTextBuf);
WidgetSetText(IDC_INPUT_CONNECTTXT, WidgetTextBufLarge);

sprintf(WidgetTextBufLarge, "Sketch - Hold the mousebutton down while moving, automatically adding points every %s.", WidgetTextBuf);
WidgetSetText(IDC_INPUT_SKETCHTXT, WidgetTextBufLarge);

WidgetSNGetEditText(IDC_SPEED, 20, WidgetTextBuf);
WidgetSetText(IDC_SUM_SPTSPACE2, WidgetTextBuf);

switch (InputMode)
	{
	case WCS_DIGITIZE_ADDPOINTS_SINGLE:
		{
		WidgetSetText(IDC_SUM_MOUSE, "Single");
		break;
		} // 
	case WCS_DIGITIZE_ADDPOINTS_SKETCH:
		{
		WidgetSetText(IDC_SUM_MOUSE, "Sketch");
		break;
		} // 
	case WCS_DIGITIZE_ADDPOINTS_CONNECT:
		{
		WidgetSetText(IDC_SUM_MOUSE, "Connect");
		break;
		} // 
	} // switch

WidgetSNGetEditText(IDC_PTSPACETWO, 20, WidgetTextBuf);

WidgetTextBuf[0] = 0;
if ((AddInfo.Mode != WCS_DIGITIZE_DIGOBJTYPE_PATH) && (AddInfo.Mode != WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH))
 	{
	WidgetSNGetEditText(IDC_LINEWT, 20, WidgetTextBuf);
	WidgetSetText(IDC_SUM_WEIGHT, WidgetTextBuf);

	sprintf(WidgetTextBuf, "%03d, %03d, %03d", AddInfo.Red, AddInfo.Green, AddInfo.Blue);
	WidgetSetText(IDC_SUM_RENDCOL, WidgetTextBuf);
	WidgetTextBuf[0] = 0;
	if (AddInfo.DrawEnabled) strcat(WidgetTextBuf, "Realtime");
	if (AddInfo.DrawEnabled && AddInfo.RenderEnabled) strcat(WidgetTextBuf, "&&");
	if (AddInfo.RenderEnabled) strcat(WidgetTextBuf, "Render");
	WidgetSetText(IDC_SUM_ENABLE, WidgetTextBuf);
	WidgetSetText(IDC_SUM_CLASS, FullClassList[AddInfo.Class]);
	WidgetSetText(IDC_SUM_STYLE, DigiStyleList[AddInfo.LineStyle]);
	WidgetCBSetCurSel(IDC_STYLEDROP, AddInfo.LineStyle);
	} // if
else
	{
	WidgetSetText(IDC_SUM_WEIGHT, WidgetTextBuf);
	// View color no longer used
	//WidgetSetText(IDC_SUM_VIEWCOL, WidgetTextBuf);
	WidgetSetText(IDC_SUM_RENDCOL, WidgetTextBuf);
	WidgetSetText(IDC_SUM_ENABLE, WidgetTextBuf);
	WidgetSetText(IDC_SUM_CLASS, WidgetTextBuf);
	WidgetSetText(IDC_SUM_STYLE, WidgetTextBuf);
	} // else

// <<<>>>gh new control pt variables
WidgetSetCheck(IDC_RADIOCONTOUR, AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE);
WidgetSetCheck(IDC_RADIOBREAKLINE, AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_GRADIENT);
//ConfigureSR(NativeWin, IDC_RADIOCONTOUR, IDC_RADIOCONTOUR, &AddInfo.ControlPtMode, SRFlag_Short, WCS_DEMDESIGN_DMODE_ISOLINE, NULL, NULL);
//ConfigureSR(NativeWin, IDC_RADIOCONTOUR, IDC_RADIOBREAKLINE, &AddInfo.ControlPtMode, SRFlag_Short, WCS_DEMDESIGN_DMODE_GRADIENT, NULL, NULL);

WidgetSetCheck(IDC_CHECKNOGREDREV, AddInfo.NoReversal);
//ConfigureSC(NativeWin, IDC_CHECKNOGREDREV, &AddInfo.NoReversal, SCFlag_Short, NULL, NULL);

WidgetCBSetCurSel(IDC_ELEVSOURCEDROP, AddInfo.ElSource);
WidgetSNConfig(IDC_MINSPC, &GlobalApp->MainProj->Interactive->DigSpace);
WidgetSNConfig(IDC_ELEV1, &AddInfo.Elev1ADT);
WidgetSNConfig(IDC_ELEV2, &AddInfo.Elev2ADT);
WidgetSNConfig(IDC_STDDEV, &AddInfo.StdDev);

ConfigureFI(NativeWin, IDC_TENSION1,
 &AddInfo.Tension1,
  .1,
   -10.0,
	10.0,
	 FIOFlag_Double,
	  (SetCritter *)NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_TENSION2,
 &AddInfo.Tension2,
  .1,
   -10.0,
	10.0,
	 FIOFlag_Double,
	  (SetCritter *)NULL,
	   NULL);

WidgetSetDisabled(IDC_TENSION1, AddInfo.ElSource != WCS_DEMDESIGN_ELSOURCE_ELEVCTRL || AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE);
WidgetSetDisabled(IDC_TENSION2, AddInfo.ElSource != WCS_DEMDESIGN_ELSOURCE_ELEVCTRL || AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE);
WidgetSetDisabled(IDC_ELEV1, AddInfo.ElSource != WCS_DEMDESIGN_ELSOURCE_ELEVCTRL);
WidgetSetDisabled(IDC_ELEV2, AddInfo.ElSource != WCS_DEMDESIGN_ELSOURCE_ELEVCTRL || AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE);
WidgetSetDisabled(IDC_CHECKNOGREDREV, AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE);

if (AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_GRADIENT)
	AddInfo.ElevADD.AddNode(10.0, AddInfo.Elev2ADT.CurValue, 0.0);
else
	AddInfo.ElevADD.RemoveNode(10.0, 0.0);
AddInfo.ElevADD.AddNode(0.0, AddInfo.Elev1ADT.CurValue, 0.0);
AddInfo.ElevADD.GetMinMaxVal(WidgetLocal.MinLowDrawVal, WidgetLocal.MaxHighDrawVal);

if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
	{
	ValRange = WidgetLocal.Crit->GetIncrement();
	WidgetLocal.MaxHighDrawVal += ValRange;
	WidgetLocal.MinLowDrawVal -= ValRange;
	} // if
else
	{
	ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .5;
	WidgetLocal.MaxHighDrawVal += ValRange;
	WidgetLocal.MinLowDrawVal -= ValRange;
	} // else
WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_GRAPH, &WidgetLocal);

} // DigitizeGUI::ConfigureWidgets()

/*===========================================================================*/

void DigitizeGUI::ConfigProfile(void)
{
int ConfigDefs = 0;
double Defaults[3];

Defaults[0] = FLT_MAX;
Defaults[1] = 0.0;
Defaults[2] = 1.0;

if (Profile == -1 && GlobalApp->MainProj->Interactive->DigSmooth.GetCurValue() == -1.0 &&
 GlobalApp->MainProj->Interactive->DigSpace.GetCurValue() == -1.0 &&
 GlobalApp->MainProj->Interactive->DigSpeed.GetCurValue() == -1.0)
	{
	Profile = WCS_DIGITIZE_ELEVTYPE_SMALLPLANE;
	} // if

switch (Profile)
	{
	case WCS_DIGITIZE_ELEVTYPE_VECTOR:
		{
		//double CellSizeNS, CellSizeWE;
		//if (GlobalApp->AppDB->GetMinDEMCellSizeMeters(CellSizeNS, CellSizeWE))
		//	GlobalApp->MainProj->Interactive->DigSpace.SetCurValue((double)(max(1.0, 2 * (long)(min(CellSizeNS, CellSizeWE)))));
		//else
			GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(20.0);
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(0.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(0.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(0.1);
		Defaults[2] = .1;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_ANT:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(0.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(0.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(0.1);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(0.1);
		Defaults[2] = .1;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_PERSON:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(2.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(0.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(2.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(3.0);
		Defaults[2] = .5;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_SPARROW:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(10.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(5.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(10.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(15.0);
		Defaults[2] = 2.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_HAWK:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(50.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(10.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(50.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(30.0);
		Defaults[2] = 10.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_EAGLE:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(200.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(30.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(200.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(60.0);
		Defaults[2] = 20.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_SMALLPLANE:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(500.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(75.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(1000.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(200.0);
		Defaults[2] = 50.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_JUMBOJET:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(10000.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(90.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(5000.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(1000.0);
		Defaults[2] = 1000.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_SATELLITE:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(100000.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(100.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(50000.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(1665.0);
		Defaults[2] = 10000.;
		ConfigDefs = 1;
		break;
		} //
	case WCS_DIGITIZE_ELEVTYPE_CELESTIAL:
		{
		GlobalApp->MainProj->Interactive->DigElevation.SetCurValue(500000.0);
		GlobalApp->MainProj->Interactive->DigSmooth.SetCurValue(100.0);
		GlobalApp->MainProj->Interactive->DigSpace.SetCurValue(100000.0);
		GlobalApp->MainProj->Interactive->DigSpeed.SetCurValue(1665.0);
		Defaults[2] = 10000.;
		ConfigDefs = 1;
		break;
		} //
	} // switch

// Setup increment
if (ConfigDefs)
	{
	GlobalApp->MainProj->Interactive->DigElevation.SetRangeDefaults(Defaults);
	GlobalApp->MainProj->Interactive->DigSpace.SetRangeDefaults(Defaults);
	GlobalApp->MainProj->Interactive->DigSpeed.SetRangeDefaults(Defaults);

	Defaults[0] = 100.0;
	Defaults[1] = 0.0;
	Defaults[2] = 1.0;
	GlobalApp->MainProj->Interactive->DigSmooth.SetRangeDefaults(Defaults);
	} // if

} // DigitizeGUI::ConfigProfile

/*===========================================================================*/

void DigitizeGUI::UpdateVecColor(void)
{

WidgetSNSync(ID_COLORPOT1, WP_FISYNC_NONOTIFY);

} // DigitizeGUI::UpdateVecColor

/*===========================================================================*/

void DigitizeGUI::DoClose(void)
{
NotifyTag PreCloseEvent[] = {MAKE_ID(WCS_PROJECTCLASS_MODULE, WCS_SUBCLASS_MODULE_CLOSE, WCS_ITEM_MODULE_DIGITIZEGUI, 0),
							NULL};

if (Digitizing)
	EndAddPoints();

ProjHost->GenerateNotify(PreCloseEvent);

} // DigitizeGUI::DoClose()

/*===========================================================================*/

void DigitizeGUI::SetParam(int Notify, ...)
{
va_list VarA;
unsigned int Change = 0, MainClass, SubClass, Item, Component;
ULONG NotifyDigChanges[WCS_MAXDIG_NOTIFY_CHANGES + 1];

#ifdef BUILD_VIEWTOY
return;
#else // !BUILD_VIEWTOY

va_start(VarA, Notify);

MainClass = va_arg(VarA, int);

while (MainClass && Change < WCS_MAXPROJ_NOTIFY_CHANGES)
	{
	if (MainClass > 255)
		{
		NotifyDigChanges[Change] = MainClass;
		SubClass = (MainClass & 0xff0000) >> 16;
		Item = (MainClass & 0xff00) >> 8;
		Component = (MainClass & 0xff);
		MainClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		NotifyDigChanges[Change] = (MainClass << 24) + (SubClass << 16) + (Item << 8) + Component;
		} // else event passed as decomposed list of class, subclass, item and component
	switch (MainClass)
		{
		case WCS_DIGCLASS_POINTS:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_FIRSTPOINT:
					{
					AddInfo.FirstPoint = (long)va_arg(VarA, int);
					break;
					} // first point
				case WCS_SUBCLASS_PTSADDED:
					{
					AddInfo.PointsAdded = (long)va_arg(VarA, int);
					break;
					} // points added
				case WCS_SUBCLASS_CURRENTPOINT:
					{
					AddInfo.Point = (long)va_arg(VarA, int);
					break;
					} // current point
				case WCS_SUBCLASS_DRAWMODE:
					{
					GlobalApp->MainProj->Interactive->SetDigDrawMode((short)va_arg(VarA, int));
					//GlobalApp->MainProj->Interactive->SetParam(1, MAKE_ID(WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE, 0), AddInfo.Mode, NULL);
					break;
					} // draw mode
				case WCS_SUBCLASS_DIGSTART:
					{
					break;
					} // WCS_SUBCLASS_DIGSTART
				case WCS_SUBCLASS_DIGDONE:
					{
					break;
					} // WCS_SUBCLASS_DIGDONE
				} // switch
			break;
			} // points
		case WCS_DIGCLASS_OUTPUT:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_ADDTOOBJECT:
					{
					AddInfo.AddToObj = va_arg(VarA, Joe *);
					break;
					} // Joe object
				case WCS_SUBCLASS_ELEVTYPE:
					{
					AddInfo.ElevType = (short)va_arg(VarA, int);
					break;
					} // Joe object
				case WCS_SUBCLASS_OBJECTTYPE:
					{
					AddInfo.DigObjectType = (short)va_arg(VarA, int);
					break;
					} // Joe object
				case WCS_SUBCLASS_ELEV:
					{
					AddInfo.Datum = va_arg(VarA, double);
					break;
					} // Joe object
				case WCS_SUBCLASS_SMOOTH:
					{
					AddInfo.Smooth = va_arg(VarA, double);
					break;
					} // Joe object
				case WCS_SUBCLASS_PTSPACE:
					{
					AddInfo.PtSpace = va_arg(VarA, double);
					break;
					} // Joe object
				case WCS_SUBCLASS_SPEED:
					{
					AddInfo.GroundSpeed = va_arg(VarA, double);
					break;
					} // Joe object
				} // switch SubClass
			break;
			} // Output
		} // switch

	MainClass = va_arg(VarA, int);
	Change ++;

	} // while

va_end(VarA);

NotifyDigChanges[Change] = 0;

if (Notify)
	{
	GenerateNotify(NotifyDigChanges);
	} // if notify clients of changes

#endif // !BUILD_VIEWTOY
} // DigitizeGUI::SetParam

/*===========================================================================*/

void DigitizeGUI::GetParam(void *Value, ...)
{
va_list VarA;
unsigned int MainClass, SubClass, Item, Component;
long *LngPtr = (long *)Value;
short *ShtPtr = (short *)Value;
double *DblPtr = (double *)Value;
Joe **JoePtr = (Joe **)Value;

va_start(VarA, Value);

MainClass = va_arg(VarA, int);

if (MainClass)
	{
	if (MainClass > 255)
		{
		SubClass = (MainClass & 0xff0000) >> 16;
		Item = (MainClass & 0xff00) >> 8;
		Component = (MainClass & 0xff);
		MainClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		} // else event passed as decomposed list of class, subclass, item and component
	switch (MainClass)
		{
		case WCS_DIGCLASS_POINTS:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_FIRSTPOINT:
					{
					*LngPtr = AddInfo.FirstPoint;
					break;
					} // first point
				case WCS_SUBCLASS_PTSADDED:
					{
					*LngPtr = AddInfo.PointsAdded;
					break;
					} // points added
				case WCS_SUBCLASS_CURRENTPOINT:
					{
					*LngPtr = AddInfo.Point;
					break;
					} // current point
				case WCS_SUBCLASS_DRAWMODE:
					{
					*ShtPtr = GlobalApp->MainProj->Interactive->GetDigDrawMode();
					break;
					} // draw mode
				case WCS_SUBCLASS_DIGSTART:
					{
					break;
					} // pen number
				case WCS_SUBCLASS_DIGDONE:
					{
					break;
					} // pen number
				} // switch
			break;
			} // points
		case WCS_DIGCLASS_OUTPUT:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_ADDTOOBJECT:
					{
					*JoePtr = AddInfo.AddToObj;
					break;
					} // Joe object
				case WCS_SUBCLASS_ELEVTYPE:
					{
					*ShtPtr= AddInfo.ElevType;
					break;
					} // Joe object
				case WCS_SUBCLASS_OBJECTTYPE:
					{
					*ShtPtr = AddInfo.DigObjectType;
					break;
					} // Joe object
				case WCS_SUBCLASS_ELEV:
					{
					*DblPtr = AddInfo.Datum;
					break;
					} // Joe object
				case WCS_SUBCLASS_SMOOTH:
					{
					*DblPtr = AddInfo.Smooth;
					break;
					} // Joe object
				case WCS_SUBCLASS_PTSPACE:
					{
					*DblPtr = AddInfo.PtSpace;
					break;
					} // Joe object
				case WCS_SUBCLASS_SPEED:
					{
					*DblPtr = AddInfo.GroundSpeed;
					break;
					} // Joe object
				} // switch SubClass
			break;
			} // Output
		} // switch

	} // if

va_end(VarA);

} // DigitizeGUI::GetParam

/*===========================================================================*/

void DigitizeGUI::SelectElevSource(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_ELEVSOURCEDROP);
AddInfo.ElSource = (short)Current;
WidgetCBSetCurSel(IDC_ELEVSOURCEDROP, AddInfo.ElSource);
ConfigureWidgets();

} // DigitizeGUI::SelectElevSource()

/*===========================================================================*/

void DigitizeGUI::DoLineStyle(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_STYLEDROP);
AddInfo.LineStyle = (short)Current;
WidgetCBSetCurSel(IDC_STYLEDROP, AddInfo.LineStyle);

} // DigitizeGUI::DoLineStyle()

/*===========================================================================*/

int DigitizeGUI::InitDigitize(int DigitizeActive)
{

if (Initialized)
	{
	Digitizing = 1;
	Initialized = 1;
	AddInfo.Point = 0;
	AddInfo.FirstPoint = (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND) ? 1: 0;
	SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_DIGSTART, 0, 0, NULL);
	} // if

return (1);

} // DigitizeGUI::InitDigitize()

/*===========================================================================*/

long DigitizeGUI::AddNextPoint(DiagnosticData *Data, int MouseMode)
{
double LatIncr, LonIncr, ElIncr, PtSpace, InEl, InLat = 0.0, InLon = 0.0, PlanetRad = EARTHRAD, ElevDatum = 0.0, Exageration = 0.0;
PlanetOpt *NewPlanet;
long Pt, ConnectedPts, PtsAdded = 0;

if (NewPlanet = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
	{
	PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
	ElevDatum = NewPlanet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
	Exageration = NewPlanet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
	} // if

if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
	InLat = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
else
	return (0);
if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
	InLon = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
else
	return (0);
if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] && Data->ValueValid[WCS_DIAGNOSTIC_SLOPE])
	InEl  = Data->Value[WCS_DIAGNOSTIC_ELEVATION];
else
	{
	InEl = GlobalApp->MainProj->Interactive->ElevationPoint(InLat, InLon);
	InEl = ElevDatum + (InEl - ElevDatum) * Exageration;
	} // else

/***
char digitizeMsg[256];
sprintf(digitizeMsg, "Lat = %lf\nLon = %lf\n, Elev = %lf\n\n", InLat, InLon, InEl);
OutputDebugString(digitizeMsg);
***/

if (Digitizing && Data && Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
	{
	if (GlobalApp->MainProj->Interactive->GetDigDrawMode() == WCS_DIGITIZE_ADDPOINTS_SKETCH)
		{ // discard point if too close to previous point and mouse is being dragged
		if (AddInfo.Point && NewPlanet)
			{
			if (MouseMode == WCS_DIAGNOSTIC_ITEM_MOUSEDRAG && (FindDistance(AddInfo.Lat[AddInfo.Point - 1], AddInfo.Lon[AddInfo.Point - 1], InLat, InLon, PlanetRad)) < GlobalApp->MainProj->Interactive->DigSpace.GetCurValue())
				{
				return(1); // discard
				} // if
			} // if
		} // if
	else if (GlobalApp->MainProj->Interactive->GetDigDrawMode() != WCS_DIGITIZE_ADDPOINTS_SKETCH && MouseMode == WCS_DIAGNOSTIC_ITEM_MOUSEDRAG)
		{
		return (1);	// discard
		} // else if
	if (AddInfo.Point + AddInfo.FirstPoint == 0)
		{
		AddInfo.FirstLat = InLat;
		AddInfo.FirstLon = InLon;
		AddInfo.FirstElev = InEl;
		PtsAdded ++;
		AddInfo.Lat[AddInfo.Point] = InLat;
		AddInfo.Lon[AddInfo.Point] = InLon;
		AddInfo.Elev[AddInfo.Point] = InEl;
		AddInfo.Point ++;
		PtsAdded ++;
		SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_PTSADDED, 0, 0, 1, NULL);
		} // if first point
	else if (AddInfo.Point && GlobalApp->MainProj->Interactive->GetDigDrawMode() == WCS_DIGITIZE_ADDPOINTS_CONNECT)
		{
		PtSpace = GlobalApp->MainProj->Interactive->DigSpace.CurValue <= 0.0 ? 1.0: GlobalApp->MainProj->Interactive->DigSpace.CurValue;
		ConnectedPts = (long)(FindDistance(InLat, InLon, AddInfo.Lat[AddInfo.Point - 1], AddInfo.Lon[AddInfo.Point - 1], PlanetRad) / PtSpace);
		if (AddInfo.Point + ConnectedPts - 1 >= WCS_DIGITIZE_MAX_OBJPTS)
			ConnectedPts = WCS_DIGITIZE_MAX_OBJPTS - AddInfo.Point;
		if (ConnectedPts > 0)
			{
			LatIncr = (InLat - AddInfo.Lat[AddInfo.Point - 1]) / ConnectedPts;
			LonIncr = (InLon - AddInfo.Lon[AddInfo.Point - 1]) / ConnectedPts;
			ElIncr = (float)((InEl - AddInfo.Elev[AddInfo.Point - 1]) / ConnectedPts);
			for (InLat = AddInfo.Lat[AddInfo.Point - 1] + LatIncr, InLon = AddInfo.Lon[AddInfo.Point - 1] + LonIncr, 
				InEl = AddInfo.Elev[AddInfo.Point - 1] + ElIncr, Pt = 0; 
				Pt < ConnectedPts; Pt ++, InLat += LatIncr, InLon += LonIncr, InEl += ElIncr)
				{
				AddInfo.Lat[AddInfo.Point] = InLat;
				AddInfo.Lon[AddInfo.Point] = InLon;
				if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] && Data->ValueValid[WCS_DIAGNOSTIC_SLOPE])
					AddInfo.Elev[AddInfo.Point] = InEl;
				else
					{
					AddInfo.Elev[AddInfo.Point] = GlobalApp->MainProj->Interactive->ElevationPoint(InLat, InLon);
					AddInfo.Elev[AddInfo.Point] = ElevDatum + (AddInfo.Elev[AddInfo.Point] - ElevDatum) * Exageration;
					} // else
				AddInfo.Point ++;
				} // for
			PtsAdded += ConnectedPts;
			SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_PTSADDED, 0, 0, 1, NULL);
			} // if 
		} // else if connect mode
	else if (AddInfo.Point < WCS_DIGITIZE_MAX_OBJPTS)
		{
		AddInfo.Lat[AddInfo.Point] = InLat;
		AddInfo.Lon[AddInfo.Point] = InLon;
		AddInfo.Elev[AddInfo.Point] = InEl;
		AddInfo.Point ++;
		PtsAdded ++;
		SetParam(1, WCS_DIGCLASS_POINTS, WCS_SUBCLASS_PTSADDED, 0, 0, 1, NULL);
		} // else if under point limit
	UpdatePaletteTitle();

	if (AddInfo.Point >= WCS_DIGITIZE_MAX_OBJPTS)
		{
		UserMessageOK("Digitize", "You've reached the limit for new object points!\nComplete this object and start a new one or append to this one.");
		EndAddPoints();
		} // if too many points
	} // if

return (PtsAdded);

} // DigitizeGUI::AddNextPoint()

/*===========================================================================*/

void DigitizeGUI::EndAddPoints(void)
{
/*
double MaxElev, Datum, Flattening, TempLength, TotalLength, SpotLength, TempElev;
VectorPoint *PLink, **PointsAddr;
int Point, Frames, Abort = 0, CreateNew = 2, ReboundAll = 0;
char *MesgLabel, SpeedStr[256];
struct WindowKeyStuff WKS;
NotifyTag DoneTag[] = {MAKE_ID(WCS_PARAMCLASS_MOTION, WCS_SUBCLASS_KEYFRAME, WCS_KEYFRAME_MAKE, 0),
						MAKE_ID(WCS_PARAMCLASS_FRAME, 0, 0, 0),  NULL};
*/

char SpeedStr[256], NameStr[256];
Camera *CamObj = NULL;
Light *LightObj = NULL;
CelestialEffect *CelesObj = NULL;
Joe *DestVec = NULL;
RasterAnimHost *DestHost = NULL, *CurHost;
double MyNorth, MySouth, MyEast, MyWest, ElevDatum = 0.0, Exageration = 0.0;

long PointLoop, FinalNumNodes = 0;
char VectorMode = 1, *EffectName = NULL;
double TotalDist, Incr, DistFromLast, PlanetRad = EARTHRAD, Speed, Flattening, UserElev, MaxElev,
	FirstElev, LastElev = 0.0, PtDist;
PlanetOpt *NewPlanet;
AnimDoubleTime LatNodes, LonNodes, ElevNodes;
NotifyTag Changes[2];
RasterAnimHostProperties RAHP;
GraphNode *LatGN, *LonGN, *ElGN;
VectorPoint *PointWalk, **PointPtr;
GeneralEffect *EffectCreated = NULL;
JoeCoordSys *MyCSAttr;
CoordSys *MyCoords;
VertexDEM VDEM;

if (NewPlanet = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
	{
	PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
	ElevDatum = NewPlanet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
	Exageration = NewPlanet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
	} // if

// int bounds
MyNorth = -DBL_MAX;
MySouth = DBL_MAX;
MyEast = DBL_MAX;
MyWest = -DBL_MAX;

Speed = GlobalApp->MainProj->Interactive->DigSpeed.GetCurValue();

LatNodes.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
LonNodes.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ElevNodes.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
LatNodes.ReleaseGraph(0);
LonNodes.ReleaseGraph(0);
ElevNodes.ReleaseGraph(0);

if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE || AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND)
	DestVec  = AddInfo.AddToObj;
DestHost = AddInfo.HostObj;

AddInfo.DrawEnabled = WidgetGetCheck(IDC_ENABLED2);
AddInfo.RenderEnabled = WidgetGetCheck(IDC_ENABLED3);

if (AddInfo.Point == 0)
	{ // nothin to do
	return;
	} // if

// Pull RGB from our temporary AnimColorTime
AddInfo.Red   = (short)(VectorColor.CurValue[0] * 255.0);
AddInfo.Green = (short)(VectorColor.CurValue[1] * 255.0);
AddInfo.Blue  = (short)(VectorColor.CurValue[2] * 255.0);


// Ok, figure out what the hell to do with these points
// Here are the clues:
/*
AddInfo.Mode
AddInfo.NewCat
AddInfo.AddToObj
AddInfo.HostObj
AddInfo.DrawEnabled, AddInfo.RenderEnabled
AddInfo.Red, AddInfo.Green, AddInfo.Blue
AddInfo.LineWeight
AddInfo.LineStyle
AddInfo.Class
*/

if ((AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_PATH) || (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH))
	{
	VectorMode = 0;
	} // if

if ((DestVec) && (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND))
	{
	MyNorth = DestVec->NWLat;
	MySouth = DestVec->SELat;
	MyEast = DestVec->SELon;
	MyWest = DestVec->NWLon;
	} // if

// SpeedLimit
if (! VectorMode)
	{
	for(TotalDist = Incr = 0.0, PointLoop = 0; PointLoop < AddInfo.Point; PointLoop++)
		{
		if (PointLoop)
			{ // figure out distance from last node to calculate time for this node
			DistFromLast = FindDistance(AddInfo.Lat[PointLoop - 1], AddInfo.Lon[PointLoop - 1], AddInfo.Lat[PointLoop], AddInfo.Lon[PointLoop], PlanetRad);
			TotalDist += DistFromLast;
			} // if
		//meters / (m/s) = seconds
		if (PointLoop && (Speed > 0.0))
			{
			Incr += DistFromLast / Speed;
			} // if
		} // for
	sprintf(SpeedStr, "%fs", Incr);
	if (GetInputString("Confirm length of time (in seconds) of motion path", "", SpeedStr))
		{
		if (atof(SpeedStr) > 0.0)
			{
			Speed = TotalDist / atof(SpeedStr);
			} // if
		else
			{
			if (strlen(SpeedStr))
				{
				Speed = 0.0;
				} // if
			else
				{
				return;
				} // else
			} // else
		} // if
	else
		{
		return; // bail out
		} // else
	} // if

// elevations are given in exagerated units
if (VectorMode && Exageration != 0.0)
	{
	for (PointLoop = 0; PointLoop < AddInfo.Point; PointLoop ++)
		{
		AddInfo.Elev[PointLoop] = ((AddInfo.Elev[PointLoop] - ElevDatum) / Exageration) + ElevDatum;
		} // for
	} // if

// <<<>>>gh This looks like a good place to do control point calculations.
// Control points may need to be processed all in a whack for gradients
// and if elevation control is from pre-existing control point data.
// Control points taken from terrain need no special processing.

// if control points do this
if (VectorMode && AddInfo.NewCat == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{
	// what elevation source? AddInfo.ElSource

	// what line type - contour or gradient? AddInfo.ControlPtMode

	if (AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_GRADIENT && AddInfo.Point > 1)
		{
		// figure out first & last elevations
		if (AddInfo.ElSource == WCS_DEMDESIGN_ELSOURCE_ELEVCTRL)
			{
			FirstElev = AddInfo.ElevADD.GetValue(0, 0.0);
			LastElev = AddInfo.ElevADD.GetValue(0, 10.0);
			} // if elev control
		else if (AddInfo.ElSource == WCS_DEMDESIGN_ELSOURCE_ENDPOINTS)
			{
			// search for closest end points to first and last points
			if (PointWalk = DBHost->NearestControlPointSearch(AddInfo.Lat[0], AddInfo.Lon[0], PlanetRad))
				FirstElev = PointWalk->Elevation;
			else
				FirstElev = AddInfo.Elev[0];
			if (PointWalk = DBHost->NearestControlPointSearch(AddInfo.Lat[AddInfo.Point - 1], AddInfo.Lon[AddInfo.Point - 1], PlanetRad))
				LastElev = PointWalk->Elevation;
			else
				LastElev = AddInfo.Elev[AddInfo.Point - 1];
			} // else if end points
		else
			{
			FirstElev = AddInfo.Elev[0];
			LastElev = AddInfo.Elev[AddInfo.Point - 1];
			} // else if DEM

		// set first and last
		AddInfo.Elev[0] = FirstElev;
		AddInfo.Elev[AddInfo.Point - 1] = LastElev;

		// stuff values in ADD
		if (ElGN = AddInfo.ElevADD.GetFirstNode(0))
			{
			ElGN->SetValue(FirstElev);
			if (ElGN->Next)
				ElGN->Next->SetValue(LastElev);
			} // if

		// find total vector length
		TotalDist = 0.0;
		for (PointLoop = 1; PointLoop < AddInfo.Point; PointLoop ++)
			{
			TotalDist += FindDistance(AddInfo.Lat[PointLoop - 1], AddInfo.Lon[PointLoop - 1], AddInfo.Lat[PointLoop], AddInfo.Lon[PointLoop], PlanetRad);
			} // for

		// find proportion of distance for each point
		if (TotalDist > 0.0)
			{
			// look up value in ADD
			PtDist = 0.0;
			for (PointLoop = 1; PointLoop < AddInfo.Point - 1; PointLoop ++)
				{
				PtDist += FindDistance(AddInfo.Lat[PointLoop - 1], AddInfo.Lon[PointLoop - 1], AddInfo.Lat[PointLoop], AddInfo.Lon[PointLoop], PlanetRad);
				AddInfo.Elev[PointLoop] = AddInfo.ElevADD.GetValue(0, 10.0 * PtDist / TotalDist);
				} // for
			} // if
		else
			{
			for (PointLoop = 1; PointLoop < AddInfo.Point - 1; PointLoop ++)
				{
				AddInfo.Elev[PointLoop] = AddInfo.Elev[0];
				} // for
			} // else
		} // if gradient
	else
		{
		if (AddInfo.ElSource == WCS_DEMDESIGN_ELSOURCE_ELEVCTRL)
			{
			FirstElev = AddInfo.ElevADD.GetValue(0, 0.0);
			} // if elev control
		else if (AddInfo.ElSource == WCS_DEMDESIGN_ELSOURCE_ENDPOINTS)
			{
			// search for closest end points to first and last points
			if (PointWalk = DBHost->NearestControlPointSearch(AddInfo.Lat[0], AddInfo.Lon[0], PlanetRad))
				FirstElev = PointWalk->Elevation;
			else
				FirstElev = AddInfo.Elev[0];
			} // else if end points
		else
			{
			FirstElev = AddInfo.Elev[0];
			} // else if DEM

		for (PointLoop = 0; PointLoop < AddInfo.Point; PointLoop ++)
			{
			AddInfo.Elev[PointLoop] = FirstElev;
			} // for

		} // contour

	// apply random factors
	if (AddInfo.StdDev.CurValue != 0.0)
		{
		for (PointLoop = 1; PointLoop < AddInfo.Point - 1; PointLoop ++)
			{
			// use the old GaussRand function since we don't want to bother seeding 
			// and don't want the same sequence every time as would happen if we had 
			// a PRNGX auto variable in this function.
			AddInfo.Elev[PointLoop] += GaussRand() * AddInfo.StdDev.CurValue;
			} // for
		} // if

	// enforce no reversals
	if (AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_GRADIENT && AddInfo.NoReversal)
		{
		if (FirstElev > LastElev)
			{
			for (PointLoop = 1; PointLoop < AddInfo.Point; PointLoop ++)
				{
				if (AddInfo.Elev[PointLoop] > AddInfo.Elev[PointLoop - 1])
					AddInfo.Elev[PointLoop] = AddInfo.Elev[PointLoop - 1];
				if (AddInfo.Elev[PointLoop] < LastElev)
					AddInfo.Elev[PointLoop] = LastElev;
				} // for
			} // if
		else if (FirstElev < LastElev)
			{
			for (PointLoop = 1; PointLoop < AddInfo.Point; PointLoop ++)
				{
				if (AddInfo.Elev[PointLoop] < AddInfo.Elev[PointLoop - 1])
					AddInfo.Elev[PointLoop] = AddInfo.Elev[PointLoop - 1];
				if (AddInfo.Elev[PointLoop] > LastElev)
					AddInfo.Elev[PointLoop] = LastElev;
				} // for
			} // if
		else
			{
			for (PointLoop = 1; PointLoop < AddInfo.Point; PointLoop ++)
				{
				AddInfo.Elev[PointLoop] = FirstElev;
				} // for
			} // if
		} // if

	} // if control points
// otherwise not for control points do this
else
	{
	// Apply smoothing
	// find max elevation of terrain along the path.
	// note this only looks at the elevations where there are path points.
	// - you could still plow into a mountain
	MaxElev = -FLT_MAX;
	for (PointLoop = 0; PointLoop < AddInfo.Point; PointLoop ++)
		{
		if (AddInfo.Elev[PointLoop] > MaxElev)
			{
			MaxElev = AddInfo.Elev[PointLoop];
			} // if
		} // for
	Flattening = GlobalApp->MainProj->Interactive->DigSmooth.CurValue / 100.0;
	UserElev = GlobalApp->MainProj->Interactive->DigElevation.GetCurValue();
	for (PointLoop = 0; PointLoop < AddInfo.Point; PointLoop++)
		{
		if (AddInfo.Smooth == 0.0)
			AddInfo.Elev[PointLoop] = (AddInfo.Elev[PointLoop] + UserElev);
		else
			AddInfo.Elev[PointLoop] = (AddInfo.Elev[PointLoop] + UserElev + (MaxElev - AddInfo.Elev[PointLoop]) * Flattening);
		} // for
	} // else

// Build the object in temporary AnimDoubles -- tranfer to vector if necessary
for(Incr = 0.0, PointLoop = 0; PointLoop < AddInfo.Point; PointLoop++)
	{
	if (VectorMode)
		{
		Incr = (double)PointLoop;
		} // if
	else
		{
		if (PointLoop)
			{ // figure out distance from last node to calculate time for this node
			DistFromLast = FindDistance(AddInfo.Lat[PointLoop - 1], AddInfo.Lon[PointLoop - 1], AddInfo.Lat[PointLoop], AddInfo.Lon[PointLoop], PlanetRad);
			} // if
		//meters / (m/s) = seconds
		if (PointLoop && (Speed > 0.0))
			{
			Incr += DistFromLast / Speed;
			} // if
		} // else
	LatNodes.AddNode(Incr, AddInfo.Lat[PointLoop], 0.0);
	LonNodes.AddNode(Incr, AddInfo.Lon[PointLoop], 0.0);
	ElevNodes.AddNode(Incr, AddInfo.Elev[PointLoop], 0.0);
	FinalNumNodes++;

	if (AddInfo.Lat[PointLoop] > MyNorth)
		{
		MyNorth = AddInfo.Lat[PointLoop];
		} // if
	if (AddInfo.Lat[PointLoop] < MySouth)
		{
		MySouth = AddInfo.Lat[PointLoop];
		} // if
	if (AddInfo.Lon[PointLoop] > MyWest)
		{
		MyWest = AddInfo.Lon[PointLoop];
		} // if
	if (AddInfo.Lon[PointLoop] < MyEast)
		{
		MyEast = AddInfo.Lon[PointLoop];
		} // if
	} // for

switch(AddInfo.Mode)
	{
	default:
		break;
	case WCS_DIGITIZE_DIGOBJTYPE_VECTOR:
	case WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE:
	case WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND:
	case WCS_DIGITIZE_DIGOBJTYPE_VECTOR_EFFECT:
		{
		// Make Joe if necessary
		if (!DestVec)
			{
			char TempName[256];
			// if control points fabricate a name 
			if (AddInfo.NewCat == WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				NameStr[0] = 0;
				WidgetGetText(IDC_NAME, 256, NameStr);
				if (NameStr[0])
					strcpy(TempName, NameStr);
				else if (AddInfo.ControlPtMode == WCS_DEMDESIGN_DMODE_ISOLINE)
					sprintf(TempName, "%s %.0fm",  "Contour",
						AddInfo.Elev1ADT.CurValue);
				else
					sprintf(TempName, "%s %.0fm to %.0fm", "Break Line",
						AddInfo.Elev[0], AddInfo.Elev[AddInfo.Point - 1]);
				} // if
			if (DestVec = DBHost->NewObject(ProjHost, AddInfo.NewCat == WCS_RAHOST_OBJTYPE_CONTROLPT ? TempName: NULL))
				{

				switch(AddInfo.Class)
					{
					// 1=Vector, 2=Control Points
					case 1: break;
					case 2: DestVec->SetFlags(WCS_JOEFLAG_ISCONTROL); break;
					} // class
				if (AddInfo.AddToObj)
					{
					// add effect attributes
					CurHost = NULL; 
					while (CurHost = AddInfo.AddToObj->GetRAHostChild(CurHost, 0))
						{
						RAHP.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
						CurHost->GetRAHostProperties(&RAHP);
						if (RAHP.TypeNumber >= WCS_JOE_ATTRIB_INTERNAL_LAKE && RAHP.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
							DestVec->AddEffect((GeneralEffect *)CurHost, -1);
						} // while 
					} // if
				} // if
			} // 

		// Transfer nodes into Joe
		if (DestVec)
			{
			DestVec->SetLineStyle(AddInfo.LineStyle);
			DestVec->SetLineWidth((unsigned char)AddInfo.LineWeight);
			DestVec->SetRGB((unsigned char)AddInfo.Red, (unsigned char)AddInfo.Green, (unsigned char)AddInfo.Blue);
			if (AddInfo.DrawEnabled)
				DestVec->SetFlags(WCS_JOEFLAG_DRAWENABLED);
			else
				DestVec->ClearFlags(WCS_JOEFLAG_DRAWENABLED);
			if (AddInfo.RenderEnabled)
				DestVec->SetFlags(WCS_JOEFLAG_RENDERENABLED);
			else
				DestVec->ClearFlags(WCS_JOEFLAG_RENDERENABLED);

			EffectName = (char *)DestVec->GetBestName();
			// free any existing points
			if (DestVec->Points() && AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND)
				{
				PointPtr = DestVec->PointsAddr();
				while (*PointPtr)
					PointPtr = &(*PointPtr)->Next;
				} // if
			else if (DestVec->Points())
				{
				DBHost->MasterPoint.DeAllocate(DestVec->Points());
				DestVec->NumPoints(0);
				DestVec->Points(NULL);
				PointPtr = DestVec->PointsAddr();
				} // else if
			else
				{
				PointPtr = DestVec->PointsAddr();
				} // else
			// allocate new points
			if (*PointPtr = GlobalApp->AppDB->MasterPoint.Allocate(
				AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND ? FinalNumNodes: FinalNumNodes + Joe::GetFirstRealPtNum()))
				{
				// transfer points from temporary node AnimDoubles into Joe points
				LatGN = LatNodes.GetFirstNode(0);
				LonGN = LonNodes.GetFirstNode(0);
				ElGN  = ElevNodes.GetFirstNode(0);

				// if the vector is attached to a CoordSys, need to project the digitized points as they are transferrred
				if (MyCSAttr = (JoeCoordSys *)DestVec->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyCSAttr->Coord;
				else
					MyCoords = NULL;

				for(PointWalk = *PointPtr; PointWalk; PointWalk = PointWalk->Next)
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					if (PointWalk == DestVec->Points())
						{ // first point -- add label point, don't advance
						if (LatGN)
							{
							VDEM.Lat = LatGN->GetValue();
							} // if
						if (LonGN)
							{
							VDEM.Lon = LonGN->GetValue();
							} // if
						if (ElGN)
							{
							VDEM.Elev  = ElGN->GetValue();
							} // if
						} // if
					else
					#endif // WCS_JOE_LABELPOINTEXISTS
						{
						if (LatGN)
							{
							VDEM.Lat = LatGN->GetValue();
							LatGN   = LatGN->Next;
							} // if
						if (LonGN)
							{
							VDEM.Lon = LonGN->GetValue();
							LonGN   = LonGN->Next;
							} // if
						if (ElGN)
							{
							VDEM.Elev  = ElGN->GetValue();
							ElGN    = ElGN->Next;
							} // if
						} // else
					// Convert from default degrees into CoordSys projection
					// if MyCoords is NULL then the geographic values are copied directly into the VectorPoint structure
					PointWalk->DefDegToProj(MyCoords, &VDEM);
					} // for
				if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND)
					DestVec->NumPoints(DestVec->NumPoints() + FinalNumNodes);
				else
					DestVec->NumPoints(FinalNumNodes + Joe::GetFirstRealPtNum());

				// Set our bounds
				DestVec->NWLat = MyNorth;
				DestVec->NWLon = MyWest;
				DestVec->SELat = MySouth;
				DestVec->SELon = MyEast;

				DestVec->ZeroUpTree();
				DBHost->ReBoundTree(WCS_DATABASE_STATIC);
				DBHost->GenerateNotify(DBChangeEventPOINTS, DestVec);
				GlobalApp->MainProj->Interactive->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);
				} // if
			} // if

		// Make effect object if necessary
		if (!DestHost && (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_VECTOR_EFFECT))
			{
			DestHost = GlobalApp->AppEffects->AddEffect(AddInfo.NewCat, EffectName);
			EffectCreated = (GeneralEffect *)DestHost;
			} // if

		// Connect vector to effect object if necessary
		if (DestHost && DestVec)
			{

			RAHP.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			RAHP.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			DestVec->GetRAHostProperties(&RAHP);

			RAHP.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
			RAHP.DropSource = DestVec;
			// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
			//  eg. still in progress through a DragnDropListGUI
			if (EffectCreated && EffectCreated->EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D)
				((Object3DEffect *)EffectCreated)->GeographicInstance = 0;
			if (DestHost->SetRAHostProperties(&RAHP) < 1)
				{
				// Clean up any new effect objects we created
				if (! AddInfo.HostObj)
					{
					Changes[0] = MAKE_ID(DestHost->GetNotifyClass(), DestHost->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEffects->RemoveEffect((GeneralEffect *)DestHost);
					DestHost = NULL;
					EffectCreated = NULL;
					// send message
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					} // if
				// kill any new Joe we created
				if (! AddInfo.AddToObj)
					{
					// sends its own message
					DBHost->RemoveRAHost(DestVec, ProjHost, EffectsHost, 0, 1);
					DestVec = NULL;
					} // if
				} // if
			} // if

		// Generate notifies to tell the world what we did
		if (DestVec)// && !AddInfo.HostObj) changed 3/26/01 by GRH to update vector lists even if an effect is being linked
			{
			Changes[0] = MAKE_ID(DestVec->GetNotifyClass(), DestVec->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, DestVec);
			} // if
		break;
		} // VECTOR-related
	case WCS_DIGITIZE_DIGOBJTYPE_PATH: // camera, target, light, celestial
	case WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH:
		{
		if (AddInfo.Mode == WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH)
			{ // can only be camera
			if (CamObj = (Camera *)AddInfo.HostObj)
				{
				CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], &LatNodes);
				CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], &LonNodes);
				CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], &ElevNodes);
				Changes[0] = MAKE_ID(CamObj->GetNotifyClass(), CamObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CamObj->GetRAHostRoot());
				} // if
			} // if
		else
			{
			if (!DestHost)
				{
				DestHost = GlobalApp->AppEffects->AddEffect(AddInfo.NewCat);
				} // if
			LatNodes.SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
			LonNodes.SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
			ElevNodes.SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
			switch(AddInfo.NewCat)
				{
				case WCS_JOE_ATTRIB_INTERNAL_CAMERA:
					{
					if (CamObj = (Camera *)DestHost)
						{
						LatNodes.CopyRangeDefaults(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT]);
						LonNodes.CopyRangeDefaults(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON]);
						ElevNodes.CopyRangeDefaults(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV]);
						CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], &LatNodes);
						CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], &LonNodes);
						CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].Copy(&CamObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], &ElevNodes);
						Changes[0] = MAKE_ID(CamObj->GetNotifyClass(), CamObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, CamObj->GetRAHostRoot());
						} // if
					break;
					} // 
				case WCS_JOE_ATTRIB_INTERNAL_LIGHT:
					{
					if (LightObj = (Light *)DestHost)
						{
						LatNodes.CopyRangeDefaults(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
						LonNodes.CopyRangeDefaults(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
						ElevNodes.CopyRangeDefaults(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
						LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &LatNodes);
						LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &LonNodes);
						LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&LightObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ElevNodes);
						Changes[0] = MAKE_ID(LightObj->GetNotifyClass(), LightObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, LightObj->GetRAHostRoot());
						} // if
					break;
					} // 
				case WCS_JOE_ATTRIB_INTERNAL_CELESTIAL:
					{
					if (CelesObj = (CelestialEffect *)DestHost)
						{
						LatNodes.CopyRangeDefaults(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE]);
						LonNodes.CopyRangeDefaults(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE]);
						ElevNodes.CopyRangeDefaults(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE]);
						CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].Copy(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE], &LatNodes);
						CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].Copy(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE], &LonNodes);
						CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].Copy(&CelesObj->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE], &ElevNodes);
						Changes[0] = MAKE_ID(CelesObj->GetNotifyClass(), CelesObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, CelesObj->GetRAHostRoot());
						} // if
					break;
					} // 
				default:
					break;
				} // switch
			} // else
		break;
		} // PATH-related
	} // switch

if (EffectCreated)
	{
	EffectCreated->EditRAHost();
	EffectCreated->OpenGallery(GlobalApp->AppEffects);
	} // if

Digitizing = 0;
HandleCloseWin(NativeWin);
} // DigitizeGUI::EndAddPoints()

/*===========================================================================*/

void DigitizeGUI::SetPaletteTitle(char *Synopsis, char *Suffix)
{
if (Synopsis)
	{
	if (Suffix)
		{
		sprintf(PaletteTitle, "%s%s", Synopsis, Suffix);
		} // if
	else
		{
		strcpy(PaletteTitle, Synopsis);
		} // else
	} // if
else
	{
	sprintf(PaletteTitle, "Create Palette");
	} // else
UpdatePaletteTitle();

} // DigitizeGUI::SetPaletteTitle

/*===========================================================================*/

void DigitizeGUI::UpdatePaletteTitle(void)
{
char PaletteTitleTemp[200];

if (AddInfo.Point)
	{
	if (AddInfo.Point == 1)
		{
		sprintf(PaletteTitleTemp, "%s: %d Point", PaletteTitle, AddInfo.Point);
		SetTitle(PaletteTitleTemp);
		} // if
	else
		{
		sprintf(PaletteTitleTemp, "%s: %d Points", PaletteTitle, AddInfo.Point);
		SetTitle(PaletteTitleTemp);
		} // else
	} // if
else
	{
	SetTitle(PaletteTitle);
	} // else

} // DigitizeGUI::UpdatePaletteTitle

/*===========================================================================*/

void DigitizeGUI::SetSummary(char *Synopsis)
{

strcpy(SummaryTitle, Synopsis);
WidgetSetText(IDC_CREATE_SUMMARY_LABEL, Synopsis);

} // DigitizeGUI::SetSummary
