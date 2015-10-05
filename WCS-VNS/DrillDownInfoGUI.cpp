// DrillDownInfoGUI.cpp
// Code for DrillDownInfo editor
// Built from scratch on 4/14/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "DrillDownInfoGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "EffectsLib.h"
#include "Database.h"
#include "Joe.h"
#include "Layers.h"
#include "Useful.h"
#include "resource.h"
#include "Lists.h"
// these next two are to dock ourselves into S@G
#include "Conservatory.h"
#include "SceneViewGUI.h"

extern WCSApp *GlobalApp;

NativeGUIWin DrillDownInfoGUI::Open(Project *Moi)
{
NativeGUIWin Success;

Success = GUIFenetre::Open(Moi);

return (Success);

} // DrillDownInfoGUI::Open

/*===========================================================================*/

NativeGUIWin DrillDownInfoGUI::Construct(void)
{
if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DRILLDOWN_VECTOR, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		WidgetLBSetHorizExt(IDC_VECTORLIST, 300);
		ConfigureWidgets();

		if(GlobalApp->GUIWins->SAG)
			{
			GlobalApp->GUIWins->SAG->AddLowerPanelTab("Point", WCS_SCENEVIEWGUI_TAB_POINT, NativeWin);
			GlobalApp->GUIWins->SAG->SwitchToTab(WCS_SCENEVIEWGUI_TAB_POINT);
			} // if
		} // if
	} // if
 
return (NativeWin);

} // DrillDownInfoGUI::Construct

/*===========================================================================*/

DrillDownInfoGUI::DrillDownInfoGUI(EffectsLib *EffectsSource, Database *DBSource, JoeList *JoeSource)
: GUIFenetre('DRIL', this, "Info About Point") // Yes, I know...
{

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Joes = JoeSource;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_ISSUBDIALOG);

} // DrillDownInfoGUI::DrillDownInfoGUI

/*===========================================================================*/

DrillDownInfoGUI::~DrillDownInfoGUI()
{

DisposeJoeList();

} // DrillDownInfoGUI::~DrillDownInfoGUI()


/*===========================================================================*/

void DrillDownInfoGUI::SetNewJoeList(JoeList *JoeSource)
{

DisposeJoeList();
Joes = JoeSource;
ConfigureWidgets();
GlobalApp->GUIWins->SAG->SwitchToTab(WCS_SCENEVIEWGUI_TAB_POINT);

} // DrillDownInfoGUI::SetNewJoeList

/*===========================================================================*/

void DrillDownInfoGUI::DisposeJoeList(void)
{
JoeList *CurJL;

while (Joes)
	{
	CurJL = Joes;
	Joes = Joes->Next;
	delete CurJL;
	} // while

} // DrillDownInfoGUI::DisposeJoeList

/*===========================================================================*/

long DrillDownInfoGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DRL, 0);

return(0);

} // DrillDownInfoGUI::HandleCloseWin

/*===========================================================================*/

long DrillDownInfoGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_VECTORLIST:
		{
		EditItem(CtrlID);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // DrillDownInfoGUI::HandleListDoubleClick

/*===========================================================================*/

void DrillDownInfoGUI::ConfigureWidgets(void)
{
JoeList *CurJL;

CurJL = Joes;

WidgetLBClear(IDC_VECTORLIST);

// Add a heading
WidgetLBInsert(IDC_VECTORLIST, -1, "* These vectors enclose the ");
WidgetLBInsert(IDC_VECTORLIST, -1, "* point clicked:");
WidgetLBInsert(IDC_VECTORLIST, -1, "");

while (CurJL)
	{
	AddVectorEntry(CurJL->Me);
	CurJL = CurJL->Next;
	} // while

AddOtherEntries();

} // DrillDownInfoGUI::ConfigureWidgets()

/*===========================================================================*/

void DrillDownInfoGUI::AddVectorEntry(Joe *CurVec)
{
double VecArea, VecLength, North, South;
char *AreaMetric = "sq m";
char *LenMetric = "m";
LayerStub *MyLayerStub;
LayerEntry *MyLayer;
const char *MyName;
JoeAttribute *MyJoeAttr;
long Place, DoLayers = 0, DoAttribs = 0;
long IsAttribute, ListPos;
char LNameTest[256];

sprintf(LNameTest, "# %s", (char *)CurVec->GetBestName());
if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest)) != LB_ERR)
	{
	WidgetLBSetItemData(IDC_VECTORLIST, Place, CurVec);
	} // if

North = CurVec->GetNorth();
South = CurVec->GetSouth();
VecArea = CurVec->ComputeAreaDegrees();
VecArea *= LatScale(EffectsHost->GetPlanetRadius());
VecArea *= LonScale(EffectsHost->GetPlanetRadius(), (North + South) * .5);
if (fabs(VecArea) >= 1000.0)
	{
	VecArea /= 10000.0;
	AreaMetric = "ha";
	} // if
VecLength = CurVec->GetVecLength(0);
if (VecLength >= 1000.0)
	{
	VecLength /= 1000.0;
	LenMetric = "km";
	} // if
sprintf(LNameTest, "*    Area = %f%s", VecArea, AreaMetric);
WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest);
sprintf(LNameTest, "*    Length = %f%s", VecLength, LenMetric);
WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest);

#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
if (MyLayerStub = CurVec->FirstLayer())
	{
	// are there attributes?
	while (MyLayerStub)
		{
		if (MyLayer = MyLayerStub->MyLayer())
			{
			IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
			if (IsAttribute)
				{
				DoAttribs = 1;
				break;
				} // if
			} // if
		MyLayerStub = CurVec->NextLayer(MyLayerStub);
		} // while
	if (DoAttribs)
		{
		strcpy(LNameTest, "#    ");
		strcat(LNameTest, "Attributes");
		WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest);
		MyLayerStub = CurVec->FirstLayer();
		while (MyLayerStub)
			{
			if (MyLayer = MyLayerStub->MyLayer())
				{
				if (MyName = MyLayer->GetName())
					{
					IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
					if (IsAttribute)
						{
						if (! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
							{
							if (MyLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
								{
								sprintf(LNameTest, "*       %s=%s", &MyName[1], MyLayerStub->GetTextAttribVal());
								if ((ListPos = WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest)) != LB_ERR)
									{
									WidgetLBSetItemData(IDC_VECTORLIST, ListPos, MyLayer);
									} // if
								} // if
							else
								{
								sprintf(LNameTest, "*       %s=%g", &MyName[1], MyLayerStub->GetIEEEAttribVal());
								//TrimZeros(LNameTest);
								if ((ListPos = WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest)) != LB_ERR)
									{
									WidgetLBSetItemData(IDC_VECTORLIST, ListPos, MyLayer);
									} // if
								} // else
							} // if not link
						} // if
					} // if
				} // if
			MyLayerStub = CurVec->NextLayer(MyLayerStub);
			} // while
		} // if
	} // if
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

if (MyLayerStub = CurVec->FirstLayer())
	{
	// are there layers?
	while (MyLayerStub)
		{
		if (MyLayer = MyLayerStub->MyLayer())
			{
			IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
			if (! IsAttribute)
				{
				DoLayers = 1;
				break;
				} // if
			} // if
		MyLayerStub = CurVec->NextLayer(MyLayerStub);
		} // while
	if (DoLayers)
		{
		strcpy(LNameTest, "#    ");
		strcat(LNameTest, "Layers");
		WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest);
		MyLayerStub = CurVec->FirstLayer();
		while (MyLayerStub)
			{
			if (MyLayer = MyLayerStub->MyLayer())
				{
				if (MyName = MyLayer->GetName())
					{
					IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
					if (! IsAttribute)
						{
						sprintf(LNameTest, "*       %s", MyName);
						if ((ListPos = WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest)) != LB_ERR)
							{
							WidgetLBSetItemData(IDC_VECTORLIST, ListPos, MyLayer);
							} // if
						} // if
					} // if
				} // if
			MyLayerStub = CurVec->NextLayer(MyLayerStub);
			} // while
		} // if
	} // if

strcpy(LNameTest, "#    ");
strcat(LNameTest, "Components");
WidgetLBInsert(IDC_VECTORLIST, -1, LNameTest);
if (MyJoeAttr = CurVec->GetFirstAttribute())
	{
	while (MyJoeAttr)
		{
		if (MyJoeAttr->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && MyJoeAttr->MinorAttrib() != WCS_JOE_ATTRIB_INTERNAL_DEM)
			{
			AddEffectToList(MyJoeAttr);
			} // if
		MyJoeAttr = MyJoeAttr->GetNextAttribute();
		} // while
	} // if

WidgetLBInsert(IDC_VECTORLIST, -1, "");

} // DrillDownInfoGUI::AddVectorEntry

/*===========================================================================*/

long DrillDownInfoGUI::AddEffectToList(JoeAttribute *Me)
{
long Place = 0;
char StrStr[256];

if(Me)
	{
	switch (Me->MinorAttrib())
		{
		case WCS_JOE_ATTRIB_INTERNAL_LAKE:
			{
			JoeLake *MyAttrib = (JoeLake *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Lake, "Lake");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Lake);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_LAKE
		case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
			{
			JoeEcosystem *MyAttrib = (JoeEcosystem *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Eco, "Ecosystem");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Eco);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
			{
			JoeRasterTA *MyAttrib = (JoeRasterTA *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Terra, "Area Terraffector");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Terra);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
		case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
			{
			JoeTerraffector *MyAttrib = (JoeTerraffector *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Terra, "Terraffector");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Terra);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
		case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
			{
			JoeShadow *MyAttrib = (JoeShadow *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Shadow, "Shadow");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Shadow);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
		case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
			{
			JoeFoliage *MyAttrib = (JoeFoliage *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Foliage, "Foliage");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Foliage);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
		case WCS_JOE_ATTRIB_INTERNAL_STREAM:
			{
			JoeStream *MyAttrib = (JoeStream *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Stream, "Stream");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Stream);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_STREAM
		case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
			{
			JoeObject3D *MyAttrib = (JoeObject3D *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Object, "3D Object");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Object);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
		/*
		case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
			{
			JoeTerrainParam *MyAttrib = (JoeTerrainParam *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->TerrainPar, "Terrain Parameters");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->TerrainPar);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
		*/
		case WCS_JOE_ATTRIB_INTERNAL_GROUND:
			{
			JoeGround *MyAttrib = (JoeGround *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Ground, "Ground");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Ground);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_GROUND
		case WCS_JOE_ATTRIB_INTERNAL_SNOW:
			{
			JoeSnow *MyAttrib = (JoeSnow *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->SnowBusinessLikeShowBusiness, "Snow");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->SnowBusinessLikeShowBusiness);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SNOW
		case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
			{
			JoeEnvironment *MyAttrib = (JoeEnvironment *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Env, "Environment");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Env);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
		case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
			{
			JoeCloud *MyAttrib = (JoeCloud *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Cloud, "Cloud");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Cloud);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
		case WCS_JOE_ATTRIB_INTERNAL_WAVE:
			{
			JoeWave *MyAttrib = (JoeWave *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Wave, "Wave");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Wave);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_WAVE
		#ifdef WCS_THEMATIC_MAP
		case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
			{
			JoeThematicMap *MyAttrib = (JoeThematicMap *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Theme, "Thematic Map");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Theme);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
		#endif // WCS_THEMATIC_MAP
		#ifdef WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
			{
			JoeCoordSys *MyAttrib = (JoeCoordSys *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Coord, "Coordinate System");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Coord);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
		#endif // WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_FENCE:
			{
			JoeFence *MyAttrib = (JoeFence *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Fnce, "Wall");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Fnce);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_FENCE
		case WCS_JOE_ATTRIB_INTERNAL_LABEL:
			{
			JoeLabel *MyAttrib = (JoeLabel *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Labl, "Label");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Labl);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_LABEL
		/*
		case WCS_JOE_ATTRIB_INTERNAL_CMAP:
			{
			JoeCmap *MyAttrib = (JoeCmap *)Me;

			BuildEffectListEntry(StrStr, (GeneralEffect *)MyAttrib->Cmap, "Color Map");
			if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_VECTORLIST, Place, MyAttrib->Cmap);
				} // if
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CMAP
		*/
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
		} // switch
	} // if

return(Place);

} // DrillDownInfoGUI::AddEffectToList()

/*===========================================================================*/

void DrillDownInfoGUI::BuildEffectListEntry(char *ListName, GeneralEffect *Me, char *EffectType)
{

strcpy(ListName, "*       ");
strcat(ListName, Me->Name);
strcat(ListName, " (");
strcat(ListName, EffectType);
strcat(ListName, ")");

} // DrillDownInfoGUI::BuildEffectListEntry()

/*===========================================================================*/

extern int JoeLinkableList[];
extern int JoeLinkableAreaEffect[];

void DrillDownInfoGUI::AddOtherEntries(void)
{
long EffectType, LinkableCt, Place = 0, Titled, AddedSomething = 0;
RenderJoeList *RJL, *CurRJL;
JoeList *CurJL;
char StrStr[256];

// for each effect class - if Joes are allowed, test each component to see if there are any attached 
// vectors or search query supplied vectors that match the vectors in the JoeList
// for classes that are linear - streams and terraffectors find vectors which are closer than
// the effect radius

if (Joes)
	{
	for (LinkableCt = 0; JoeLinkableList[LinkableCt]; LinkableCt ++)
		{
		Titled = 0;
		EffectType = JoeLinkableList[LinkableCt];
		if (RJL = DBHost->CreateRenderJoeList(EffectsHost, EffectType))
			{
			// for area effects
			if (JoeLinkableAreaEffect[LinkableCt] == 1)
				{
				// look for the JoeList entries in RJL
				// if found, report the effect
				for (CurRJL = RJL; CurRJL; CurRJL = (RenderJoeList *)CurRJL->Next)
					{
					for (CurJL = Joes; CurJL; CurJL = CurJL->Next)
						{
						if (CurJL->Me == CurRJL->Me)
							{
							if(!AddedSomething)
								{
								// Add a heading
								WidgetLBInsert(IDC_VECTORLIST, -1, "* These enabled components");
								WidgetLBInsert(IDC_VECTORLIST, -1, "* were found at the point clicked:");
								WidgetLBInsert(IDC_VECTORLIST, -1, "");
								AddedSomething = 1;
								} // if
							// report the effect type
							if (! Titled)
								{
								sprintf(StrStr, "# %s", EffectsHost->GetEffectTypeName(EffectType));
								WidgetLBInsert(IDC_VECTORLIST, -1, StrStr);
								Titled = 1;
								} // if
							// report the effect
							BuildOtherListEntry(StrStr, CurRJL->Effect);
							StrStr[0] = '*';
							if ((Place = WidgetLBInsert(IDC_VECTORLIST, -1, StrStr)) != LB_ERR)
								{
								WidgetLBSetItemData(IDC_VECTORLIST, Place, CurRJL->Effect);
								} // if
							if (EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
								{
								// evaluate	area terraffectors and report each one's effect
								} // if
							break;
							} // if
						} // for
					} // for
				} // if
			else if (JoeLinkableAreaEffect[LinkableCt] == 2)
				{
				// linear effect
				// check to see if within Effect Radius of any of the vectors in RJL
				// or an arbitrary distance from a wall vector.
				// evaluate	terraffectors and report each one's effect
				if (EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
					{
					// evaluate	area terraffectors and report each one's effect
					} // if
				} // else if
			else
				{
				// point effect
				// check to see if within some arbitrary radius of any of the vectors in RJL
				} // else
			while (RJL)
				{
				CurRJL = (RenderJoeList *)RJL->Next;
				delete RJL;
				RJL = CurRJL;
				} // while
			} // if
		if (Titled)
			WidgetLBInsert(IDC_VECTORLIST, -1, "");
		} // for
	} // if

} // DrillDownInfoGUI::AddOtherEntries

/*===========================================================================*/

void DrillDownInfoGUI::BuildOtherListEntry(char *ListName, GeneralEffect *Me)
{

strcpy(ListName, "#    ");
strcat(ListName, Me->Name);

} // DrillDownInfoGUI::BuildOtherListEntry()

/*===========================================================================*/

void DrillDownInfoGUI::EditItem(int CtrlID)
{
RasterAnimHost *EditMe;
long Ct, NumListEntries;

if ((NumListEntries = WidgetLBGetCount(CtrlID)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(CtrlID, Ct))
			{
			if ((EditMe = (RasterAnimHost *)WidgetLBGetItemData(CtrlID, Ct)) && EditMe != (RasterAnimHost *)LB_ERR)
				{
				if (EffectsHost->IsEffectValid((GeneralEffect *)EditMe, 0) || DBHost->ValidateJoe(EditMe))
					EditMe->EditRAHost();
				} // if
			} // if
		} // for
	} // if

} // DrillDownInfoGUI::EditItem

/*===========================================================================*/

long DrillDownInfoGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT ListRect;
POINT TransUL;
NativeControl ListBox;
int Top, Left;
int NewListWidth, NewListHeight;

if(ListBox = GetWidgetFromID(IDC_VECTORLIST))
	{
	GetWindowRect(ListBox, &ListRect);
	TransUL.x = ListRect.left;
	TransUL.y = ListRect.top;
	ScreenToClient(NativeWin, &TransUL);
	Top = TransUL.y;
	Left = TransUL.x;
	NewListWidth = NewWidth - (TransUL.x + TransUL.x ); // subtract left margin twice to repeat it as right margin
	NewListHeight = NewHeight - (TransUL.y + TransUL.y); // subtract top margin twice to repeat it as bottom margin
	SetWindowPos(ListBox, NULL, 0, 0, NewListWidth, NewListHeight, SWP_NOMOVE | SWP_NOZORDER);
	} // if

return(0);

} // DrillDownInfoGUI::HandleReSized
