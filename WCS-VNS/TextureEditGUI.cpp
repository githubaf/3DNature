// TextureEditGUI.cpp
// Code for Texture editor
// Built from DefaultEditGUI.cpp on 9/11/98 Gary Huber
// Copyright 1998 by Questar Productions. All rights reserved

#include "stdafx.h"
#include "TextureEditGUI.h"
#include "Texture.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Raster.h"
#include "Useful.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "AnimGraphGUI.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "IncludeExcludeTypeList.h"
#include "AppMem.h"
#include "resource.h"

/*===========================================================================*/

// Material GUI
#define WCS_TEXED_MATGRADSET	3

bool TextureEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANLOAD:
	case WCS_FENETRE_WINCAP_CANSAVE:
	case WCS_FENETRE_WINCAP_CANUNDO:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // TextureEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin TextureEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TextureEditGUI::Open

/*===========================================================================*/

NativeGUIWin TextureEditGUI::Construct(void)
{
Raster *MyRast;
char *BlendStyles[] = {"Sharp Edge", "Soft Edge", "Quarter Blend", "Half Blend", "Full Blend", "Fast Increase", "Slow Increase", "S-Curve"};
int Pos;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_TEXTURE_EDIT, LocalWinSys()->RootWin);

	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_IMAGEOBJ, 0, 0, false);
	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_COLOR, 0, 1, false);
	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_VALUE, 0, 2, false);

	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_BITMAP, 1, 0, false);
	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_PROCEDURAL, 1, 1, false);
	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_INCLUDE, 1, 2, false);

	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_POSLATLON, 2, 0, false);
	CreateSubWinFromTemplate(IDD_TEXTURE_EDIT_POSSIZE, 2, 1, false);

	if (NativeWin)
		{

		WidgetCBSetUIMode(IDC_TYPEDROP, 1);
		WidgetCBSetUIMode(IDC_IMAGEDROP, 1);

		WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
		for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
			{
			Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
			WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
			} // for
		for (Pos = 0; Pos < 8; Pos ++)
			WidgetCBInsert(IDC_BLENDSTYLE, -1, BlendStyles[Pos]);
		if (ActiveRoot)
			SetViewDropContents(ActiveRoot->ApplyToEcosys);
		if (ActiveRoot)
			SetTerrainDropContents(ActiveRoot->ApplyToDisplace);

		WidgetTCInsertItem(IDC_TAB3, 0, "Size");
		WidgetTCInsertItem(IDC_TAB3, 1, "Center");
		WidgetTCInsertItem(IDC_TAB3, 2, "Falloff");
		WidgetTCInsertItem(IDC_TAB3, 3, "Velocity");
		WidgetTCInsertItem(IDC_TAB3, 4, "Rotation");

		// Material GUI
		TexMatGUI->Construct(WCS_TEXED_MATGRADSET, WCS_TEXED_MATGRADSET + 1);

		HideWidgets();
		SetPanels();
		FillClassDrop();
		FillClassTypeDrop();


		ConfigureWidgets();
		UpdateThumbnails(TRUE);
		} // if

	} // if
 
return(NativeWin);

} // TextureEditGUI::Construct

/*===========================================================================*/

TextureEditGUI::TextureEditGUI(ImageLib *ImageSource, RasterAnimHost *HostSource, RootTexture *RootSource, AnimColorTime *DefColorSource)
: GUIFenetre('TEXG', this, "Texture Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(0, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								//MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ImageHost = ImageSource;
ConstructError = 0;
ActiveRoot = RootSource;
TextureHost = HostSource;
EffectsHost = GlobalApp->AppEffects;
NumListTextures = 0;
DefaultColor = DefColorSource;
ActiveNode = NULL;
ActiveGrad = NULL;
GradientActive = NULL;
SampleData[0] = SampleData[1] = SampleData[2] = NULL;
BackgroundInstalled = 0;
VertexUVWAvailable = VertexCPVAvailable = 0;
TNailDrawn[0] = TNailDrawn[1] = TNailDrawn[2] = 0;
ReceivingDiagnostics = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;
ActiveParent = NULL;
PostProc = 0;
// Material GUI
TexMatGUI = NULL;

if (ActiveRoot && TextureHost)
	{
	strcpy(NameStr, "Texture Editor - ");
	if (Root = TextureHost->GetRAHostRoot())
		{
		strcat(NameStr, Root->GetRAHostName());
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		Root->GetRAHostProperties(&Prop);
		PostProc = Prop.TypeNumber == WCS_EFFECTSSUBCLASS_POSTPROC;
		} // if
	if (Root != TextureHost && TextureHost->RAParent && Root != TextureHost->RAParent)
		{
		strcat(NameStr, " ");
		strcat(NameStr, TextureHost->RAParent->GetCritterName(TextureHost));
		} // if
	if (ActiveRoot->RAParent)
		{
		strcat(NameStr, " ");
		strcat(NameStr, ActiveRoot->RAParent->GetCritterName(ActiveRoot));
		} // if
	SetTitle(NameStr);
	AllEvents[0] = MAKE_ID(TextureHost->GetNotifyClass(), 0xff, 0xff, 0xff);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	// temporary
	if (! (Active = ActiveRoot->Tex))
		{
		if (! (ActiveRoot->Tex = (Texture *)new FractalNoiseTexture(ActiveRoot, WCS_TEXTURE_ROOT, ActiveRoot->ApplyToEcosys, ActiveRoot->ApplyToColor, NULL, DefaultColor)))
			ConstructError = 1;
		Active = ActiveRoot->Tex;
		} // if
	if (! (RootRast = new Raster()))
		ConstructError = 1;
	if (! (CompChildRast = new Raster()))
		ConstructError = 1;
	if (! (CompRast = new Raster()))
		ConstructError = 1;
	if (! (TextureList = (long *)AppMem_Alloc(WCS_TEXTURE_NUMTYPES * sizeof (long), APPMEM_CLEAR)))
		ConstructError = 1;
	if (! ConstructError)
		ActiveRoot->Copy(&Backup, ActiveRoot);
	if (! (SampleData[0] = new TextureSampleData()))
		ConstructError = 1;
	if (! (SampleData[1] = new TextureSampleData()))
		ConstructError = 1;
	if (! (SampleData[2] = new TextureSampleData()))
		ConstructError = 1;
	ActiveRoot->InitAAChain();
	// Material GUI
	if (TexMatGUI = new PortableMaterialGUI(0, this, GlobalApp->AppEffects, NULL, &Active->ColorGrad)) // init ordinal 0
		{
		PopDropMaterialNotifier.Host = this; // to be able to call notifications later
		TexMatGUI->SetNotifyFunctor(&PopDropMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	VertexUVWAvailable = ActiveRoot->VertexUVWAvailable();
	VertexCPVAvailable = ActiveRoot->VertexCPVAvailable();
	} // if
else
	ConstructError = 1;

} // TextureEditGUI::TextureEditGUI

/*===========================================================================*/

TextureEditGUI::~TextureEditGUI()
{

GlobalApp->RemoveBGHandler(this);

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
if (RootRast)
	delete RootRast;
if (CompChildRast)
	delete CompChildRast;
if (CompRast)
	delete CompRast;
if (TextureList)
	AppMem_Free(TextureList, WCS_TEXTURE_NUMTYPES * sizeof (long));
if (SampleData[0])
	delete SampleData[0];
if (SampleData[1])
	delete SampleData[1];
if (SampleData[2])
	delete SampleData[2];
// Material GUI
if (TexMatGUI)
	delete TexMatGUI;
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // TextureEditGUI::~TextureEditGUI()

/*===========================================================================*/

long TextureEditGUI::HandleCloseWin(NativeGUIWin NW)
{

DoKeep();
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TXG, 0);

return(0);

} // TextureEditGUI::HandleCloseWin

/*===========================================================================*/

long TextureEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
short UpdateThumbs = 0, UpdateRoot = 1;

switch (ButtonID)
	{
	case IDC_TNAIL1:
	case IDC_TNAIL2:
	case IDC_TNAIL3:
		{
		UpdateThumbs = 1;
		break;
		} // 
	case IDC_TNAIL:
		{
		if (ActiveRoot && Active && Active->Img && Active->Img->GetRaster())
			{
			Active->Img->GetRaster()->OpenPreview(FALSE);
			} // if
		break;
		} // image
	} // ButtonID

if (UpdateThumbs)
	{
	UpdateThumbnails(UpdateRoot);
	} // if
return(0);
} // TextureEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long TextureEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
SECURITY_INLINE_CHECK(061, 61);
switch (ButtonID)
	{
	case ID_KEEP:
		{
		DoKeep();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TXG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_TEX16:
		{
		EnableTexture(ButtonID, WCS_TEXTURE_OPACITY);
		break;
		} // 
	case IDC_TEXTURE_ADD:
		{
		AddNewTexture();
		break;
		} // 
	case IDC_TEXTURE_DELETE:
		{
		RemoveTexture();
		break;
		} // 
	case IDC_EDITIMAGE:
		{
		DoEditImage();
		break;
		} // 
	case IDC_TEX10:
		{
		EnableGradientTexture(ButtonID);
		break;
		} // 
	case IDC_TEX0:
		{
		EnableTexture(ButtonID, 1);
		break;
		} // 
	case IDC_TEX1:
		{
		EnableTexture(ButtonID, 2);
		break;
		} // 
	case IDC_TEX12:
	case IDC_TEX14:
	case IDC_TEX20:
		{
		EnableTexture(ButtonID, WCS_TEXTURE_STRATAFUNC);
		break;
		} // 
	case IDC_TEX13:
	case IDC_TEX15:
	case IDC_TEX21:
		{
		EnableTexture(ButtonID, WCS_TEXTURE_BLENDINGFUNC);
		break;
		} // 
	case IDC_TEX2:
		{
		EnableTexture(ButtonID, 3);
		break;
		} // 
	case IDC_TEX3:
		{
		EnableTexture(ButtonID, 4);
		break;
		} // 
	case IDC_TEX4:
		{
		EnableTexture(ButtonID, 5);
		break;
		} // 
	case IDC_TEX5:
		{
		EnableTexture(ButtonID, 6);
		break;
		} // 
	case IDC_TEX6:
		{
		EnableTexture(ButtonID, 7);
		break;
		} // 
	case IDC_TEX7:
		{
		EnableTexture(ButtonID, 8);
		break;
		} // 
	case IDC_TEX8:
		{
		EnableTexture(ButtonID, 9);
		break;
		} // 
	case IDC_TEX9:
		{
		EnableTexture(ButtonID, 10);
		break;
		} // 
	case IDC_TEX17:
		{
		EnableInputParamTexture(ButtonID);
		break;
		} // 
	case IDC_TEX18:
		{
		EnableInputParamTexture(ButtonID);
		break;
		} // 
	case IDC_TEX19:
		{
		EnableInputParamTexture(ButtonID);
		break;
		} // 
	case IDC_ADDCOLOR:
		{
		AddColorNode();
		break;
		} // 
	case IDC_REMOVECOLOR:
		{
		RemoveColorNode();
		break;
		} // 
	case IDC_MOVETEMPUP:
		{
		AdjustTexturePosition(-1);
		UpdateThumbnails(true);
		break;
		} // IDC_MOVETEMPUP
	case IDC_MOVETEMPDOWN:
		{
		AdjustTexturePosition(1);
		UpdateThumbnails(true);
		break;
		} // IDC_MOVETEMPDOWN
	case IDC_AUTOSIZE:
		{
		if (! Active->SetSizeToFitMaterial())
			UserMessageOK("Size to Fit", "Unable to determine the size of the material.");
		break;
		} // IDC_AUTOSIZE
	case IDC_EDITCURVE:
		{
		if (ActiveRoot && Active && Active->GetTexType() == WCS_TEXTURE_TYPE_CUSTOMCURVE && ((CustomCurveTexture *)Active)->CurveADP)
			((CustomCurveTexture *)Active)->CurveADP->EditRAHost();
		break;
		} // 
	case IDC_ADDITEM:
		{
		AddItem();
		break;
		} // IDC_ADDITEM
	case IDC_REMOVEITEM:
		{
		RemoveItem();
		break;
		} // IDC_REMOVEITEM
	case IDC_GALLERY: // from titlebar button
		{
		if (ActiveRoot)
			ActiveRoot->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVE: // from titlebar button
		{
		if (ActiveRoot)
			ActiveRoot->OpenBrowseData(EffectsHost);
		break;
		} //
	// Material GUI
	case IDC_POPDROP0:
		{
		if(WidgetGetCheck(IDC_POPDROP0))
			{
			ShowMaterialPopDrop(true);			} // if
		else
			{
			ShowMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	default: break;
	} // switch

// Material GUI
TexMatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // TextureEditGUI::HandleButtonClick

/*===========================================================================*/

long TextureEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_TYPEDROP:
		{
		DoTextureType();
		break;
		} // IDC_TYPEDROP
	case IDC_VIEWDROP:
		{
		DoViewDirection();
		break;
		} // IDC_VIEWDROP
	case IDC_IMAGEDROP:
		{
		DoNewImage();
		break;
		} // IDC_IMAGEDROP
	case IDC_PARAMDROP:
		{
		DoSelectParam();
		break;
		} // IDC_PARAMDROP
	case IDC_BLENDSTYLE:
		{
		BlendStyle();
		break;
		}
	case IDC_COORDSPACEDROP:
		{
		SelectCoordSpace();
		break;
		}
	case IDC_VERTEXMAPDROP:
		{
		SelectVertexMap();
		break;
		}
	} // switch

// Material GUI
TexMatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // TextureEditGUI::HandleCBChange

/*===========================================================================*/

long TextureEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
short UpdateThumbs = 0, UpdateRoot = 1;

SECURITY_INLINE_CHECK(010, 10);
switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		DoTextureList();
		UpdateThumbs = 1;
		UpdateRoot = 0;
		break;
		} // if list
	} // switch CtrlID

if (UpdateThumbs)
	{
	UpdateThumbnails(UpdateRoot);
	} // if

return (0);

} // TextureEditGUI::HandleListSel

/*===========================================================================*/

long TextureEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ITEMLIST:
		{
		EditItem();
		break;
		}
	} // switch CtrlID

return (0);

} // TextureEditGUI::HandleListDoubleClick

/*===========================================================================*/

long TextureEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		RemoveTexture();
		break;
		} // IDC_PARLIST
	case IDC_ITEMLIST:
		{
		RemoveItem();
		break;
		} // IDC_ITEMLIST
	} // switch

return(0);

} // TextureEditGUI::HandleListDelItem

/*===========================================================================*/

long TextureEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		CopyTexture();
		break;
		} // IDC_PARLIST
	} // switch

return(0);

} // TextureEditGUI::HandleListCopyItem

/*===========================================================================*/

long TextureEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		PasteTexture();
		break;
		} // IDC_PARLIST
	} // switch

return(0);

} // TextureEditGUI::HandleListPasteItem

/*===========================================================================*/

long TextureEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARAM1:
		{
		InitNoise(3);
		break;
		} // Start
	case IDC_PARAM2:
		{
		InitNoise(4);
		break;
		} // Start
	case IDC_PARAM3:
		{
		InitNoise(5);
		break;
		} // Start
	case IDC_PARAM4:
		{
		InitNoise(6);
		break;
		} // Start
	case IDC_PARAM7:
		{
		InitNoise(9);
		break;
		} // Start
	case IDC_PARAM8:
		{
		InitNoise(10);
		break;
		} // Start
	case IDC_GRADIENTPOS:
		{
		if (ActiveRoot && Active)
			Active->ColorGrad.CertifyNodePosition(Active->ColorGrad.GetActiveNode());
		break;
		} // Start
	} // switch

// Material GUI
TexMatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // TextureEditGUI::HandleFIChange

/*===========================================================================*/

long TextureEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		DoTextureEnabled();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_CHECKSELFOPACITY:
	case IDC_CHECKANTIALIAS:
		{
		if (ActiveRoot)
			Active->InitAAConditional();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_CHECK3D:
		{
		InitNoise(WCS_TEXTURE_3DIMENSIONS);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_CHECKROOTANDCHILD:
	case IDC_CHECKCOMPONENTANDCHILD:
	case IDC_CHECKCOMPONENT:
	case IDC_CHECKTILEWIDTH:
	case IDC_CHECKTILEHEIGHT:
	case IDC_CHECKFLIPWIDTH:
	case IDC_CHECKFLIPHEIGHT:
	case IDC_CHECKREVERSEWIDTH:
	case IDC_CHECKREVERSEHEIGHT:
	case IDC_CHECKNEGIMAGE:
	case IDC_CHECKALPHAONLY:
	case IDC_CHECKPIXELBLEND:
	case IDC_CHECKQUANTIZE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	} // switch

return(0);

} // TextureEditGUI::HandleSCChange

/*===========================================================================*/

long TextureEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOALIGNX:
	case IDC_RADIOALIGNY:
	case IDC_RADIOALIGNZ:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_RADIOINCLUDE:
	case IDC_RADIOEXCLUDE:
		{
		if (Active && Active->InclExcl)
			{
			Changes[0] = MAKE_ID(Active->InclExcl->GetNotifyClass(), Active->InclExcl->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->InclExcl->GetRAHostRoot());
			} // if
		else if (Active && Active->InclExclType)
			{
			Changes[0] = MAKE_ID(Active->InclExclType->GetNotifyClass(), Active->InclExclType->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->InclExclType->GetRAHostRoot());
			} // if
		break;
		} // 
	} // switch

return(0);

} // TextureEditGUI::HandleSRChange


/*===========================================================================*/

long TextureEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB3:
		{
		switch (NewPageID)
			{
			case 1:
				{
				ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_CENTER;
				break;
				} // 1, Center
			case 2:
				{
				ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_FALLOFF;
				break;
				} // 2, Falloff
			case 3:
				{
				ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_VELOCITY;
				break;
				} // 3, Velocity
			case 4:
				{
				ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_ROTATION;
				break;
				} // 4, Rotation
			default:
				{
				ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_SIZE;
				break;
				} // 0, Size
			} // switch
		ConfigureXYZWidgets(0);
		break;
		} // TAB3
	} // switch

return(0);

} // TextureEditGUI::HandlePageChange

/*===========================================================================*/

void TextureEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[8];
long CurPos, Pos, UpdateThumbs = 0, Done = 0;
Raster *MyRast, *MatchRast;
Texture *OldActive = Active;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

if (ActiveRoot && (Active = ActiveRoot->GetValidTexture(Active)) && Active == OldActive)
	{
	Interested[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
		{
		// dangerous to leave these because new gradient and node might have same value as old ones but have 
		// different RAParent causing crashing when syncing color gradient widgets
		ActiveGrad = NULL;
		ActiveNode = NULL;
		HideWidgets();
		SetPanels();
		ConfigureWidgets();
		UpdateThumbnails();
		Done = 1;
		} // if whole root texture changed
	else if (Active == OldActive)
		{
		Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
		Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
		Interested[3] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
		Interested[4] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
		Interested[5] = NULL;
		if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
			{
			SyncWidgets();
			UpdateThumbs = 1;
			Done = 1;
			} // if frame number changed

		Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
		Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
		Interested[4] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
		Interested[5] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
		Interested[6] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Interested[7] = NULL;
		if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
			{
			ConfigureColors();
			UpdateThumbs = 1;
			Done = 1;
			} // if color name changed

		Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Interested[1] = NULL;
		if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
			{
			BuildList();
			Done = 1;
			} // if color name changed

		Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Interested[1] = NULL;
		if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
			{
			ActiveRoot->InitAAChain();		// I think this belongs here
			SyncWidgets();
			DisableWidgets();
			UpdateThumbs = 1;
			Done = 1;
			} // if value changed
		} // if
	} // if ActiveRoot && Active

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchRast = (ActiveRoot && Active && Active->Img) ? Active->Img->GetRaster(): NULL;
	WidgetCBClear(IDC_IMAGEDROP);
	WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
	for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
		{
		Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
		WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
		if (MyRast == MatchRast)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_IMAGEDROP, CurPos);
	Done = 1;
	} // if image name changed
else
	{
	Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		MatchRast = (ActiveRoot && Active && Active->Img) ? Active->Img->GetRaster(): NULL;
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MatchRast);
		if (ActiveRoot && Active)
			{
			UpdateThumbs = 1;
			} // if
		Done = 1;
		} // else
	} // else

if (! Done)
	{
	ConfigureWidgets();
	UpdateThumbnails();
	} // if
else if (UpdateThumbs)
	UpdateThumbnails();

} // TextureEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void TextureEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
RasterAnimHost *Root;

strcpy(TextStr, "Texture Editor - ");
if (Root = TextureHost->GetRAHostRoot())
	{
	strcat(TextStr, Root->GetRAHostName());
	} // if
if (Root != TextureHost && TextureHost->RAParent && Root != TextureHost->RAParent)
	{
	strcat(TextStr, " ");
	strcat(TextStr, TextureHost->RAParent->GetCritterName(TextureHost));
	} // if
if (ActiveRoot && ActiveRoot->RAParent)
	{
	strcat(TextStr, " ");
	strcat(TextStr, ActiveRoot->RAParent->GetCritterName(ActiveRoot));
	} // if
SetTitle(TextStr);

ConfigureTB(NativeWin, IDC_TEXTURE_ADD, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_TEXTURE_DELETE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDCOLOR, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVECOLOR, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDITEM, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEITEM, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPDOWN, IDI_ARROWDOWN, NULL);
// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);

ConfigureColors();

if (ActiveRoot && Active)
	{
	int ParentParam = Active->FindTextureNumberInParent();
	if((ParentParam == WCS_TEXTURE_STRATAFUNC) || (ParentParam == WCS_TEXTURE_BLENDINGFUNC)) // are we a remap?
		{ // we are a remap
		BuildComboList(false, true);
		} // if
	else // we are not a remap
		{
		BuildComboList(true, false);
		} // else

	WidgetCBSetCurSel(IDC_TYPEDROP, FindInComboList(Active->GetTexType()));
	WidgetCBSetCurSel(IDC_VIEWDROP, ActiveRoot->GetViewDirection());

	//WidgetSNConfig(IDC_TEXWEIGHT, &Active->TexParam[0]);
	WidgetSmartRAHConfig(IDC_TEXWEIGHT, &Active->TexParam[0], Active);

	ConfigureFI(NativeWin, IDC_AASAMPLES,
	 &Active->AASamples,
	  1.0,
	   2.0,
		50.0,
		 FIOFlag_Char,
		  NULL,
		   NULL);

	if (Active->Tex[WCS_TEXTURE_OPACITY])
		ConfigureTB(NativeWin, IDC_TEX16, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX16, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX16, Active->GetTexEnabled(WCS_TEXTURE_OPACITY));
	WidgetSetText(IDC_TEXWEIGHT, Active->ParamName[WCS_TEXTURE_OPACITY]);

	ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKANTIALIAS, &Active->Antialias, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKSELFOPACITY, &Active->SelfOpacity, SCFlag_Char, NULL, NULL);

	// Image Panel
	ConfigureImageWidgets();

	if (Active->Tex[WCS_TEXTURE_STRATAFUNC])
		ConfigureTB(NativeWin, IDC_TEX20, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX20, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX20, Active->GetTexEnabled(WCS_TEXTURE_STRATAFUNC));
	WidgetSetText(IDC_STRATAFUNCTXT3, Active->ParamName[WCS_TEXTURE_STRATAFUNC]);

	if (Active->Tex[WCS_TEXTURE_BLENDINGFUNC])
		ConfigureTB(NativeWin, IDC_TEX21, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX21, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX21, Active->GetTexEnabled(WCS_TEXTURE_BLENDINGFUNC));
	WidgetSetText(IDC_BLENDFUNCTXT3, Active->ParamName[WCS_TEXTURE_BLENDINGFUNC]);


	// Color Panel

	if (Active->Tex[WCS_TEXTURE_STRATAFUNC])
		ConfigureTB(NativeWin, IDC_TEX12, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX12, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX12, Active->GetTexEnabled(WCS_TEXTURE_STRATAFUNC));
	WidgetSetText(IDC_STRATAFUNCTXT, Active->ParamName[WCS_TEXTURE_STRATAFUNC]);

	if (Active->Tex[WCS_TEXTURE_BLENDINGFUNC])
		ConfigureTB(NativeWin, IDC_TEX13, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX13, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX13, Active->GetTexEnabled(WCS_TEXTURE_BLENDINGFUNC));
	WidgetSetText(IDC_BLENDFUNCTXT, Active->ParamName[WCS_TEXTURE_BLENDINGFUNC]);

	//WidgetSNConfig(IDC_HIRANGE2, &Active->TexParam[2]);
	WidgetSmartRAHConfig(IDC_HIRANGE2, &Active->TexParam[2], Active);
	WidgetSetText(IDC_HIRANGE2, Active->ParamName[2]);

	// Value Panel
	//WidgetSNConfig(IDC_LORANGE1, &Active->TexParam[1]);
	//WidgetSNConfig(IDC_HIRANGE1, &Active->TexParam[2]);
	WidgetSmartRAHConfig(IDC_LORANGE1, &Active->TexParam[1], Active);
	WidgetSmartRAHConfig(IDC_HIRANGE1, &Active->TexParam[2], Active);

	if (Active->Tex[1])
		ConfigureTB(NativeWin, IDC_TEX0, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX0, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX0, Active->GetTexEnabled(1));
	WidgetSetText(IDC_LORANGE1, Active->ParamName[1]);

	if (Active->Tex[2])
		ConfigureTB(NativeWin, IDC_TEX1, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX1, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX1, Active->GetTexEnabled(2));
	WidgetSetText(IDC_HIRANGE1, Active->ParamName[2]);

	if (Active->Tex[WCS_TEXTURE_STRATAFUNC])
		ConfigureTB(NativeWin, IDC_TEX14, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX14, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX14, Active->GetTexEnabled(WCS_TEXTURE_STRATAFUNC));
	WidgetSetText(IDC_STRATAFUNCTXT2, Active->ParamName[WCS_TEXTURE_STRATAFUNC]);

	if (Active->Tex[WCS_TEXTURE_BLENDINGFUNC])
		ConfigureTB(NativeWin, IDC_TEX15, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX15, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX15, Active->GetTexEnabled(WCS_TEXTURE_BLENDINGFUNC));
	WidgetSetText(IDC_BLENDFUNCTXT2, Active->ParamName[WCS_TEXTURE_BLENDINGFUNC]);

	// Bitmap Panel
	ConfigureSC(NativeWin, IDC_CHECKTILEWIDTH, &Active->TileWidth, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKTILEHEIGHT, &Active->TileHeight, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKFLIPWIDTH, &Active->FlipWidth, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKFLIPHEIGHT, &Active->FlipHeight, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKREVERSEWIDTH, &Active->ReverseWidth, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKREVERSEHEIGHT, &Active->ReverseHeight, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNEGIMAGE, &Active->ImageNeg, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPIXELBLEND, &Active->PixelBlend, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKALPHAONLY, &Active->AlphaOnly, SCFlag_Char, NULL, NULL);
	ConfigureFI(NativeWin, IDC_WRAPWIDTH,
	 &Active->WrapWidth,
	  1.0,
	   0.0,
		100.0,
		 FIOFlag_Double,
		  NULL,
		   NULL);
	ConfigureFI(NativeWin, IDC_WRAPHEIGHT,
	 &Active->WrapHeight,
	  1.0,
	   0.0,
		100.0,
		 FIOFlag_Double,
		  NULL,
		   NULL);

	// Procedural Panel
	//WidgetSNConfig(IDC_PARAM1, &Active->TexParam[3]);
	WidgetSmartRAHConfig(IDC_PARAM1, &Active->TexParam[3], Active);
	if (Active->Tex[3])
		ConfigureTB(NativeWin, IDC_TEX2, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX2, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX2, Active->GetTexEnabled(3));
	WidgetSetText(IDC_PARAM1, Active->ParamName[3]);

	//WidgetSNConfig(IDC_PARAM2, &Active->TexParam[4]);
	WidgetSmartRAHConfig(IDC_PARAM2, &Active->TexParam[4], Active);
	if (Active->Tex[4])
		ConfigureTB(NativeWin, IDC_TEX3, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX3, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX3, Active->GetTexEnabled(4));
	WidgetSetText(IDC_PARAM2, Active->ParamName[4]);

	//WidgetSNConfig(IDC_PARAM3, &Active->TexParam[5]);
	WidgetSmartRAHConfig(IDC_PARAM3, &Active->TexParam[5], Active);
	if (Active->Tex[5])
		ConfigureTB(NativeWin, IDC_TEX4, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX4, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX4, Active->GetTexEnabled(5));
	WidgetSetText(IDC_PARAM3, Active->ParamName[5]);

	//WidgetSNConfig(IDC_PARAM4, &Active->TexParam[6]);
	WidgetSmartRAHConfig(IDC_PARAM4, &Active->TexParam[6], Active);
	if (Active->Tex[6])
		ConfigureTB(NativeWin, IDC_TEX5, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX5, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX5, Active->GetTexEnabled(6));
	WidgetSetText(IDC_PARAM4, Active->ParamName[6]);

	//WidgetSNConfig(IDC_PARAM5, &Active->TexParam[7]);
	WidgetSmartRAHConfig(IDC_PARAM5, &Active->TexParam[7], Active);
	if (Active->Tex[7])
		ConfigureTB(NativeWin, IDC_TEX6, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX6, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX6, Active->GetTexEnabled(7));
	WidgetSetText(IDC_PARAM5, Active->ParamName[7]);

	//WidgetSNConfig(IDC_PARAM6, &Active->TexParam[8]);
	WidgetSmartRAHConfig(IDC_PARAM6, &Active->TexParam[8], Active);
	if (Active->Tex[8])
		ConfigureTB(NativeWin, IDC_TEX7, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX7, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX7, Active->GetTexEnabled(8));
	WidgetSetText(IDC_PARAM6, Active->ParamName[8]);

	//WidgetSNConfig(IDC_PARAM7, &Active->TexParam[9]);
	WidgetSmartRAHConfig(IDC_PARAM7, &Active->TexParam[9], Active);
	if (Active->Tex[9])
		ConfigureTB(NativeWin, IDC_TEX8, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX8, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX8, Active->GetTexEnabled(9));
	WidgetSetText(IDC_PARAM7, Active->ParamName[9]);

	//WidgetSNConfig(IDC_PARAM8, &Active->TexParam[10]);
	WidgetSmartRAHConfig(IDC_PARAM8, &Active->TexParam[10], Active);
	if (Active->Tex[10])
		ConfigureTB(NativeWin, IDC_TEX9, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
	else
		ConfigureTB(NativeWin, IDC_TEX9, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX9, Active->GetTexEnabled(10));
	WidgetSetText(IDC_PARAM8, Active->ParamName[10]);

	WidgetCBSetCurSel(IDC_PARAMDROP, FindTerrainDropItem(Active->Misc));
	ConfigureSC(NativeWin, IDC_CHECKQUANTIZE, &Active->Quantize, SCFlag_Char, NULL, NULL);

	// PosLatLon Panel
	//WidgetSNConfig(IDC_NORTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH]);
	//WidgetSNConfig(IDC_SOUTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH]);
	//WidgetSNConfig(IDC_WEST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST]);
	//WidgetSNConfig(IDC_EAST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST]);
	//WidgetSNConfig(IDC_FEATHER, &Active->TexFeather);
	WidgetSmartRAHConfig(IDC_FEATHER, &Active->TexFeather, Active);

	// PosSize Panel
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNX, &Active->TexAxis, SRFlag_Char, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNY, &Active->TexAxis, SRFlag_Char, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNZ, &Active->TexAxis, SRFlag_Char, 2, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECK3D, &Active->ThreeD, SCFlag_Char, NULL, NULL);
	ConfigureXYZWidgets();

	ConfigureSC(NativeWin, IDC_CHECKROOTANDCHILD, &ActiveRoot->ShowRootNail, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOMPONENTANDCHILD, &ActiveRoot->ShowCompChildNail, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOMPONENT, &ActiveRoot->ShowComponentNail, SCFlag_Char, NULL, NULL);

	//WidgetSNConfig(IDC_PREVIEWSIZE, &ActiveRoot->PreviewSize, FIOFlag_Frac);
	WidgetSmartRAHConfig(IDC_PREVIEWSIZE, &ActiveRoot->PreviewSize, ActiveRoot, FIOFlag_Frac);

	if (Active->InclExcl)
		{
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOINCLUDE, &Active->InclExcl->Include, SRFlag_Char, 1, NULL, NULL);
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOEXCLUDE, &Active->InclExcl->Include, SRFlag_Char, 0, NULL, NULL);
		BuildItemList();
		} // if
	else if (Active->InclExclType)
		{
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOINCLUDE, &Active->InclExclType->Include, SRFlag_Char, 1, NULL, NULL);
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOEXCLUDE, &Active->InclExclType->Include, SRFlag_Char, 0, NULL, NULL);
		BuildItemList();
		} // if
	else
		{
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOINCLUDE, NULL, 0, 1, NULL, NULL);
		ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOEXCLUDE, NULL, 0, 0, NULL, NULL);
		ClearItemList();
		} // else

	if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_SIZE)
		{
		if (Active->Tex[WCS_TEXTURE_SIZE1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_SIZE1));
		if (Active->Tex[WCS_TEXTURE_SIZE2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_SIZE2));
		if (Active->Tex[WCS_TEXTURE_SIZE3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_SIZE3));
		} // if size
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_CENTER)
		{
		if (Active->Tex[WCS_TEXTURE_CENTER1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_CENTER1));
		if (Active->Tex[WCS_TEXTURE_CENTER2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_CENTER2));
		if (Active->Tex[WCS_TEXTURE_CENTER3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_CENTER3));
		} // if center
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_FALLOFF)
		{
		if (Active->Tex[WCS_TEXTURE_FALLOFF1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF1));
		if (Active->Tex[WCS_TEXTURE_FALLOFF2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF2));
		if (Active->Tex[WCS_TEXTURE_FALLOFF3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF3));
		} // if falloff
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY)
		{
		ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, FALSE);
		ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, FALSE);
		ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, FALSE);
		} // if velocity
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_ROTATION)
		{
		if (Active->Tex[WCS_TEXTURE_ROTATION1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_ROTATION1));
		if (Active->Tex[WCS_TEXTURE_ROTATION2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_ROTATION2));
		if (Active->Tex[WCS_TEXTURE_ROTATION3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_ROTATION3));
		} // if rotation

	FillCoordSpaceDrop();
	SetCoordSpaceDrop();
	} // if
else
	{
	ConfigureTB(NativeWin, IDC_TEX16, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX10, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX12, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX13, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX0, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX1, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX14, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX15, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX20, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX21, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX2, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX3, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX4, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX5, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX6, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX7, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX8, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX9, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSmartRAHConfig(IDC_TEXWEIGHT, (RasterAnimHost *)NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKANTIALIAS, NULL, 0, NULL, NULL);

	// Image Panel
	ConfigureImageWidgets(TRUE);

	// Color Panel
	//ConfigureFI(NativeWin, IDC_HIRANGE2, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_HIRANGE2, (RasterAnimHost *)NULL, NULL);

	// Value Panel
	//ConfigureFI(NativeWin, IDC_LORANGE1, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_HIRANGE1, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_LORANGE1, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_HIRANGE1, (RasterAnimHost *)NULL, NULL);

	// Bitmap Panel
	ConfigureSC(NativeWin, IDC_CHECKTILEWIDTH, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKTILEHEIGHT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKFLIPWIDTH, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKFLIPHEIGHT, NULL, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_WRAPWIDTH, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_WRAPHEIGHT, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);

	// Procedural Panel
	//ConfigureFI(NativeWin, IDC_PARAM1, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM2, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM3, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM4, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM5, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM6, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM7, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PARAM8, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM1, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM2, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM3, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM4, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM5, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM6, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM7, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PARAM8, (RasterAnimHost *)NULL, NULL);

	// PosLatLon Panel
	//ConfigureFI(NativeWin, IDC_NORTH, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_SOUTH, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_WEST, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_EAST, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_FEATHER, NULL, 0.0, 0.0, 0.0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_FEATHER, (RasterAnimHost *)NULL, NULL);

	// PosSize Panel
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNX, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNY, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNZ, NULL, 0, 0, NULL, NULL);
	ConfigureXYZWidgets(TRUE);

	ConfigureSC(NativeWin, IDC_CHECKROOTANDCHILD, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOMPONENTANDCHILD, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOMPONENT, NULL, 0, NULL, NULL);
	//ConfigureFI(NativeWin, IDC_PREVIEWSIZE, NULL, 0.0, 0.0,	0.0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_PREVIEWSIZE, (RasterAnimHost *)NULL, NULL);
	} // else

DisableWidgets();
BuildList();

} // TextureEditGUI::ConfigureWidgets()

/*===========================================================================*/

void TextureEditGUI::ConfigureColors(void)
{
GradientCritter *TempGrad = ActiveGrad;

if (Active)
	{
	TempGrad = Active->ColorGrad.GetActiveNode();
	if (Active != GradientActive)
		{
		WidgetAGConfig(IDC_ANIMGRADIENT, &Active->ColorGrad);
		// Material GUI
		TexMatGUI->Setup(0, this, GlobalApp->AppEffects, NULL, &Active->ColorGrad);
		TexMatGUI->ConfigureWidgets();
		GradientActive = Active;
		} // if need to reconfigure
	else
		{
		if (ActiveGrad != TempGrad)
			TexMatGUI->ConfigureWidgets();
		else
			TexMatGUI->SyncWidgets();
		WidgetAGSync(IDC_ANIMGRADIENT);
		} // else
	if ((ActiveGrad = TempGrad) && (ActiveNode = (ColorTextureThing *)ActiveGrad->GetThing()))
		{
		//Red = (unsigned char)(ActiveNode->Color.GetClampedCompleteValue(0) * 255);
		//Grn = (unsigned char)(ActiveNode->Color.GetClampedCompleteValue(1) * 255);
		//Blu = (unsigned char)(ActiveNode->Color.GetClampedCompleteValue(2) * 255);
		//SetColorPot(0, Red, Grn, Blu, 1);
		//ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);
		WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &ActiveNode->Color, ActiveNode);
		if (ActiveNode->Tex)
			ConfigureTB(NativeWin, IDC_TEX10, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX10, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX10, ActiveNode->Tex && ActiveNode->Tex->Enabled);
		WidgetSmartRAHConfig(IDC_GRADIENTPOS, &ActiveGrad->Position, &Active->ColorGrad);
		WidgetCBSetCurSel(IDC_BLENDSTYLE, ActiveGrad->BlendStyle);
		} // if
	// Material GUI
	TexMatGUI->ConfigureMaterial();
	} // if
else
	{
	ActiveNode = NULL;
	ActiveGrad = NULL;
	GradientActive = NULL;
	//SetColorPot(0, 0, 0, 0, 1);
	//ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	ConfigureTB(NativeWin, IDC_TEX10, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
	WidgetSetCheck(IDC_TEX10, 0);
	WidgetAGConfig(IDC_ANIMGRADIENT, NULL);
	//WidgetSNConfig(IDC_GRADIENTPOS, NULL);
	WidgetSmartRAHConfig(IDC_GRADIENTPOS, (RasterAnimHost *)NULL, NULL);
	WidgetCBSetCurSel(IDC_BLENDSTYLE, 0);
	TexMatGUI->Setup(0, this, GlobalApp->AppEffects, NULL, NULL);
	} // else

} // TextureEditGUI::ConfigureColors

/*===========================================================================*/

void TextureEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_TEXWEIGHT, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_LORANGE1, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_HIRANGE1, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_HIRANGE2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM1, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM3, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM4, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM5, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM6, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM7, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PARAM8, WP_FISYNC_NONOTIFY);
//WidgetFISync(IDC_NORTH, WP_FISYNC_NONOTIFY);
//WidgetFISync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
//WidgetFISync(IDC_WEST, WP_FISYNC_NONOTIFY);
//WidgetFISync(IDC_EAST, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FEATHER, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_XSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_YSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_ZSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_XVELOCITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_YVELOCITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_ZVELOCITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PREVIEWSIZE, WP_FISYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCLUDE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOEXCLUDE, WP_SRSYNC_NONOTIFY);

} // TextureEditGUI::SyncWidgets

/*===========================================================================*/

void TextureEditGUI::SetViewDropContents(short Ecosys)
{

if (Ecosys)
	{
	WidgetCBInsert(IDC_VIEWDROP, -1, "East +X");
	WidgetCBInsert(IDC_VIEWDROP, -1, "North +Y");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Above +Z");
	WidgetCBInsert(IDC_VIEWDROP, -1, "West -X");
	WidgetCBInsert(IDC_VIEWDROP, -1, "South -Y");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Below -Z");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Cube");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Sphere");
	} // if
else
	{
	WidgetCBInsert(IDC_VIEWDROP, -1, "Right +X");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Top +Y");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Rear +Z");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Left -X");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Bottom -Y");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Front -Z");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Cube");
	WidgetCBInsert(IDC_VIEWDROP, -1, "Sphere");
	} // else

} // TextureEditGUI::SetViewDropContents

/*===========================================================================*/

void TextureEditGUI::SetTerrainDropContents(short Displace)
{
int CurDrop = 0;

if (Displace)
	{
	WidgetCBInsert(IDC_PARAMDROP, -1, "Elevation (m)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Relative Elevation");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Latitude (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Longitude (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Water Depth (m)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Vector Slope (%)");
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_ELEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_RELELEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_LATITUDE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_LONGITUDE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_WATERDEPTH;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE;
	} // if
else
	{
	WidgetCBInsert(IDC_PARAMDROP, -1, "Elevation (m)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Relative Elevation");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Slope (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Aspect (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "North Deviation (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "East Deviation (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Latitude (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Longitude (deg)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Z Distance (m)");
	WidgetCBInsert(IDC_PARAMDROP, -1, "Angle from Camera (deg)");
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_ELEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_RELELEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_SLOPE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_ASPECT;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_NORTHDEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_EASTDEV;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_LATITUDE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_LONGITUDE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_ZDISTANCE;
	ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA;
	if (PostProc)
		{
		WidgetCBInsert(IDC_PARAMDROP, -1, "Reflectivity (%)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Illumination (%)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Luminosity (%)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Hue (0-360)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Saturation (%)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Value (%)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Red (0-255+)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Green (0-255+)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Blue (0-255+)");
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_ILLUMINATION;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_LUMINOSITY;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_HUE;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_SATURATION;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_VALUE;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_RED;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_GREEN;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_BLUE;
		} // if
	else
		{
		WidgetCBInsert(IDC_PARAMDROP, -1, "Water Depth (m)");
		WidgetCBInsert(IDC_PARAMDROP, -1, "Vector Slope (%)");
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_WATERDEPTH;
		ParamDropItem[CurDrop ++] = WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE;
		} // else
	} // else

} // TextureEditGUI::SetTerrainDropContents

/*===========================================================================*/

long TextureEditGUI::FindTerrainDropItem(unsigned char TerrainItem)
{
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_TERRAINPARAM_MAXPARAMS; Ct ++)
	{
	if (ParamDropItem[Ct] == TerrainItem)
		return (Ct);
	} // for

return (-1);

} // TextureEditGUI::FindTerrainDropItem

/*===========================================================================*/

void TextureEditGUI::ConfigureXYZWidgets(short DisableAll)
{

if (Active && ! DisableAll)
	{
	WidgetTCSetCurSel(IDC_TAB3, ActiveRoot->ShowSize);
	// no keyframes for these widgets
	WidgetSmartRAHConfig(IDC_XVELOCITY, &Active->TexVelocity[0], Active);
	WidgetSmartRAHConfig(IDC_YVELOCITY, &Active->TexVelocity[1], Active);
	WidgetSmartRAHConfig(IDC_ZVELOCITY, &Active->TexVelocity[2], Active);
	WidgetShow(IDC_AUTOSIZE, ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_SIZE || ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_CENTER);
	WidgetShow(IDC_UNITSTEXT, ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_FALLOFF);
	if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_SIZE)
		{
		WidgetSetText(IDC_UNITSTEXT, "");
		WidgetSmartRAHConfig(IDC_XSIZE, &Active->TexSize[0], Active, FIOFlag_Frac);
		WidgetSmartRAHConfig(IDC_YSIZE, &Active->TexSize[1], Active, FIOFlag_Frac);
		WidgetSmartRAHConfig(IDC_ZSIZE, &Active->TexSize[2], Active, FIOFlag_Frac);
		if (Active->Tex[WCS_TEXTURE_SIZE1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_SIZE1));
		if (Active->Tex[WCS_TEXTURE_SIZE2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_SIZE2));
		if (Active->Tex[WCS_TEXTURE_SIZE3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_SIZE3));
		} // if size
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_CENTER)
		{
		WidgetSetText(IDC_UNITSTEXT, "");
		WidgetSmartRAHConfig(IDC_XSIZE, &Active->TexCenter[0], Active);
		WidgetSmartRAHConfig(IDC_YSIZE, &Active->TexCenter[1], Active);
		WidgetSmartRAHConfig(IDC_ZSIZE, &Active->TexCenter[2], Active);
		if (Active->Tex[WCS_TEXTURE_CENTER1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_CENTER1));
		if (Active->Tex[WCS_TEXTURE_CENTER2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_CENTER2));
		if (Active->Tex[WCS_TEXTURE_CENTER3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_CENTER3));
		} // if center
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_FALLOFF)
		{
		WidgetSetText(IDC_UNITSTEXT, "(%/m)");
		WidgetSmartRAHConfig(IDC_XSIZE, &Active->TexFalloff[0], Active);
		WidgetSmartRAHConfig(IDC_YSIZE, &Active->TexFalloff[1], Active);
		WidgetSmartRAHConfig(IDC_ZSIZE, &Active->TexFalloff[2], Active);
		if (Active->Tex[WCS_TEXTURE_FALLOFF1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF1));
		if (Active->Tex[WCS_TEXTURE_FALLOFF2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF2));
		if (Active->Tex[WCS_TEXTURE_FALLOFF3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_FALLOFF3));
		} // if falloff
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY)
		{
		WidgetSetText(IDC_UNITSTEXT, "");
		// need to configure these widgets to something, velocity widgets don't have keyframes so are different
		WidgetSmartRAHConfig(IDC_XSIZE, &Active->TexSize[0], Active, FIOFlag_Frac);
		WidgetSmartRAHConfig(IDC_YSIZE, &Active->TexSize[1], Active, FIOFlag_Frac);
		WidgetSmartRAHConfig(IDC_ZSIZE, &Active->TexSize[2], Active, FIOFlag_Frac);
		ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, FALSE);
		ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, FALSE);
		ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, FALSE);
		} // if velocity
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_ROTATION)
		{
		WidgetSetText(IDC_UNITSTEXT, "");
		WidgetSmartRAHConfig(IDC_XSIZE, &Active->TexRotation[0], Active);
		WidgetSmartRAHConfig(IDC_YSIZE, &Active->TexRotation[1], Active);
		WidgetSmartRAHConfig(IDC_ZSIZE, &Active->TexRotation[2], Active);
		if (Active->Tex[WCS_TEXTURE_ROTATION1])
			ConfigureTB(NativeWin, IDC_TEX17, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX17, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX17, Active->GetTexEnabled(WCS_TEXTURE_ROTATION1));
		if (Active->Tex[WCS_TEXTURE_ROTATION2])
			ConfigureTB(NativeWin, IDC_TEX18, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX18, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX18, Active->GetTexEnabled(WCS_TEXTURE_ROTATION2));
		if (Active->Tex[WCS_TEXTURE_ROTATION3])
			ConfigureTB(NativeWin, IDC_TEX19, IDI_TEXTUREDISABLED_SMALL, IDI_TEXTURE_SMALL);
		else
			ConfigureTB(NativeWin, IDC_TEX19, IDI_NOTEXTURE_SMALL, IDI_TEXTURE_SMALL);
		WidgetSetCheck(IDC_TEX19, Active->GetTexEnabled(WCS_TEXTURE_ROTATION3));
		} // if rotation
	WidgetShow(IDC_XSIZE, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_YSIZE, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_ZSIZE, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_XVELOCITY, ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_YVELOCITY, ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_ZVELOCITY, ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_TEX17, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_TEX18, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	WidgetShow(IDC_TEX19, ActiveRoot->ShowSize != WCS_TEXTURE_SHOW_VELOCITY);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_XSIZE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_YSIZE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ZSIZE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_XVELOCITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_YVELOCITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ZVELOCITY, (RasterAnimHost *)NULL, NULL);
	WidgetShow(IDC_TEX17, FALSE);
	WidgetShow(IDC_TEX18, FALSE);
	WidgetShow(IDC_TEX19, FALSE);
	} // else

} // TextureEditGUI::ConfigureXYZWidgets

/*===========================================================================*/

void TextureEditGUI::ConfigureImageWidgets(short DisableAll)
{
long ListPos, NumEntries, Ct;
char Str[24];
RasterShell *MyShell;
Raster *MyRast, *TestRast;

if (! DisableAll)
	{
	if (MyShell = Active->Img)
		{
		if (MyRast = MyShell->GetRaster())
			{
			sprintf(Str, "%d", MyRast->GetWidth());
			WidgetSetText(IDC_IMAGEWIDTH, Str);
			sprintf(Str, "%d", MyRast->GetHeight());
			WidgetSetText(IDC_IMAGEHEIGHT, Str);
			sprintf(Str, "%d", (MyRast->GetIsColor() ? 3: 1));
			if (MyRast->GetAlphaStatus())
				strcat(Str, " + Alpha");
			WidgetSetText(IDC_IMAGECOLORS, Str);
			ListPos = -1;
			NumEntries = WidgetCBGetCount(IDC_IMAGEDROP);
			for (Ct = 0; Ct < NumEntries; Ct ++)
				{
				if ((TestRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Ct)) != (Raster *)-1 && TestRast == MyRast)
					{
					ListPos = Ct;
					break;
					} // if
				} // for
			WidgetCBSetCurSel(IDC_IMAGEDROP, ListPos);
			ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MyRast);
			} // if
		else
			{
			WidgetSetText(IDC_IMAGEWIDTH, "");
			WidgetSetText(IDC_IMAGEHEIGHT, "");
			WidgetSetText(IDC_IMAGECOLORS, "");
			WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
			ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
			} // else
		} // if
	else
		{
		WidgetSetText(IDC_IMAGEWIDTH, "");
		WidgetSetText(IDC_IMAGEHEIGHT, "");
		WidgetSetText(IDC_IMAGECOLORS, "");
		WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
		} // else
	} // if
else
	{
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
	} // else

} // TextureEditGUI::ConfigureImageWidgets

/*===========================================================================*/

void TextureEditGUI::SelectColorPanel(short PanelID)
{

switch (PanelID)
	{
	case IDD_TEXTURE_EDIT_IMAGEOBJ:
		{
		ShowPanel(0, 0);
		WidgetSetText(IDC_COLORVALUEBOX, "Image Object");
		break;
		} // SCALE
	case IDD_TEXTURE_EDIT_COLOR:
		{
		ShowPanel(0, 1);
		WidgetSetText(IDC_COLORVALUEBOX, "Colors");
		break;
		} // SCALE
	case IDD_TEXTURE_EDIT_VALUE:
		{
		ShowPanel(0, 2);
		WidgetSetText(IDC_COLORVALUEBOX, "Values");
		break;
		} // PROC
	} // PanelID

} // TextureEditGUI::SelectColorPanel()

/*===========================================================================*/

void TextureEditGUI::SelectBitmapPanel(short PanelID)
{

switch (PanelID)
	{
	case IDD_TEXTURE_EDIT_BITMAP:
		{
		ShowPanel(1, 0);
		WidgetSetText(IDC_PARAMBITMAPBOX, "Image Settings");
		break;
		} // SCALE
	case IDD_TEXTURE_EDIT_PROCEDURAL:
		{
		ShowPanel(1, 1);
		WidgetSetText(IDC_PARAMBITMAPBOX, "Procedural Parameters");
		break;
		} // PROC
	case IDD_TEXTURE_EDIT_INCLUDE:
		{
		ShowPanel(1, 2);
		if (Active && Active->InclExcl)
			WidgetSetText(IDC_PARAMBITMAPBOX, "Include/Exclude List");
		else
			WidgetSetText(IDC_PARAMBITMAPBOX, "Object Type List");
		break;
		} // PROC
	} // PanelID

} // TextureEditGUI::SelectBitmapPanel()

/*===========================================================================*/

void TextureEditGUI::SelectPosPanel(short PanelID)
{

switch (PanelID)
	{
	case IDD_TEXTURE_EDIT_POSLATLON:
		{
		ShowPanel(2, 0);
		WidgetSetText(IDC_SIZEPOSBOX, "Position && Feathering");
		break;
		} // SCALE
	case IDD_TEXTURE_EDIT_POSSIZE:
		{
		ShowPanel(2, 1);
		WidgetSetText(IDC_SIZEPOSBOX, "Size && Position");
		break;
		} // PROC
	default:
		{
		ShowPanel(2, -1);
		WidgetSetText(IDC_SIZEPOSBOX, "");
		} // default
	} // PanelID

} // TextureEditGUI::SelectPosPanel()

/*===========================================================================*/

void TextureEditGUI::SetPanels(void)
{
char TexType;

if (ActiveRoot && Active)
	{
	TexType = Active->GetTexType();
	SelectColorPanel(WCS_TEXTURETYPE_ISBITMAP(TexType) ? IDD_TEXTURE_EDIT_IMAGEOBJ: Active->ApplyToColor ? IDD_TEXTURE_EDIT_COLOR: IDD_TEXTURE_EDIT_VALUE);
	SelectBitmapPanel(WCS_TEXTURETYPE_ISBITMAP(TexType) ? IDD_TEXTURE_EDIT_BITMAP: TexType == WCS_TEXTURE_TYPE_INCLUDEEXCLUDE || TexType == WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE ? IDD_TEXTURE_EDIT_INCLUDE: IDD_TEXTURE_EDIT_PROCEDURAL);
	SelectPosPanel(WCS_TEXTURETYPE_ISBITMAP(TexType) && Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED ? 
		IDD_TEXTURE_EDIT_POSLATLON: WCS_TEXTUREPARAM_NOSIZECENTERETC(TexType) ? 
		NULL: IDD_TEXTURE_EDIT_POSSIZE);
	} // if

} // TextureEditGUI::SetPanels

/*===========================================================================*/

void TextureEditGUI::DisableWidgets(void)
{
char TexType;

if (ActiveRoot && Active)
	{
	TexType = Active->GetTexType();
	WidgetSetDisabled(IDC_CHECKSELFOPACITY, ! WCS_TEXTURETYPE_ISTWOVALUE(TexType) && ! WCS_TEXTURETYPE_ISBITMAP(TexType));
	WidgetSetDisabled(IDC_AASAMPLES, (! Active->Antialias));
	WidgetSetDisabled(IDC_CHECKTILEWIDTH, (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED) || (TexType != WCS_TEXTURE_TYPE_PLANARIMAGE && TexType != WCS_TEXTURE_TYPE_CUBICIMAGE));
	WidgetSetDisabled(IDC_CHECKFLIPWIDTH, (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED) || (TexType != WCS_TEXTURE_TYPE_PLANARIMAGE && TexType != WCS_TEXTURE_TYPE_CUBICIMAGE) || (! Active->TileWidth));
	WidgetSetDisabled(IDC_WRAPWIDTH, (TexType != WCS_TEXTURE_TYPE_CYLINDRICALIMAGE) && (TexType != WCS_TEXTURE_TYPE_SPHERICALIMAGE));
	WidgetSetDisabled(IDC_CHECKTILEHEIGHT, (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED) || (TexType != WCS_TEXTURE_TYPE_PLANARIMAGE && TexType != WCS_TEXTURE_TYPE_CUBICIMAGE && TexType != WCS_TEXTURE_TYPE_CYLINDRICALIMAGE));
	WidgetSetDisabled(IDC_CHECKFLIPHEIGHT, (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED) || (TexType != WCS_TEXTURE_TYPE_PLANARIMAGE && TexType != WCS_TEXTURE_TYPE_CUBICIMAGE && TexType != WCS_TEXTURE_TYPE_CYLINDRICALIMAGE) || (! Active->TileHeight));
	WidgetSetDisabled(IDC_WRAPHEIGHT, (TexType != WCS_TEXTURE_TYPE_SPHERICALIMAGE));

	WidgetSetDisabled(IDC_RADIOALIGNX, ! WCS_TEXTURETYPE_USEAXIS(TexType));
	WidgetSetDisabled(IDC_RADIOALIGNY, ! WCS_TEXTURETYPE_USEAXIS(TexType));
	WidgetSetDisabled(IDC_RADIOALIGNZ, ! WCS_TEXTURETYPE_USEAXIS(TexType));

	if ((ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_SIZE && ! WCS_TEXTURETYPE_USESIZE(TexType)) ||
		(ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_VELOCITY && ! WCS_TEXTURETYPE_USEVELOCITY(TexType)) ||
		(ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_ROTATION && ! WCS_TEXTURETYPE_USEROTATION(TexType)))
		{
		ActiveRoot->ShowSize = WCS_TEXTURE_SHOW_CENTER;
		ConfigureXYZWidgets();
		} // if
/*
	WidgetSetDisabled(IDC_BTNSHOWSIZE, ! WCS_TEXTURETYPE_USESIZE(TexType));
	WidgetSetDisabled(IDC_BTNSHOWVELOCITY, ! WCS_TEXTURETYPE_USEVELOCITY(TexType));
	WidgetSetDisabled(IDC_BTNSHOWROTATION, ! WCS_TEXTURETYPE_USEROTATION(TexType));
*/
	WidgetSetDisabled(IDC_AUTOSIZE, ! ((ActiveRoot->ApplyToEcosys && Active->CoordSpace == WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ)
		|| (! ActiveRoot->ApplyToEcosys && Active->CoordSpace == WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN)));

	WidgetSetDisabled(IDC_TEXWEIGHT, Active->Tex[WCS_TEXTURE_OPACITY] && Active->Tex[WCS_TEXTURE_OPACITY]->Enabled && Active->Tex[WCS_TEXTURE_OPACITY]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_LORANGE1, Active->Tex[1] && Active->Tex[1]->Enabled && Active->Tex[1]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_HIRANGE1, Active->Tex[2] && Active->Tex[2]->Enabled && Active->Tex[2]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_HIRANGE2, Active->Tex[2] && Active->Tex[2]->Enabled && Active->Tex[2]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM1, Active->Tex[3] && Active->Tex[3]->Enabled && Active->Tex[3]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM2, Active->Tex[4] && Active->Tex[4]->Enabled && Active->Tex[4]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM3, Active->Tex[5] && Active->Tex[5]->Enabled && Active->Tex[5]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM4, Active->Tex[6] && Active->Tex[6]->Enabled && Active->Tex[6]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM5, Active->Tex[7] && Active->Tex[7]->Enabled && Active->Tex[7]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM6, Active->Tex[8] && Active->Tex[8]->Enabled && Active->Tex[8]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM7, Active->Tex[9] && Active->Tex[9]->Enabled && Active->Tex[9]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	WidgetSetDisabled(IDC_PARAM8, Active->Tex[10] && Active->Tex[10]->Enabled && Active->Tex[10]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);

	//WidgetSetDisabled(IDC_LOCOLORDROP, Active->Tex[WCS_TEXTURE_COLOR1] && Active->UseTexture[WCS_TEXTURE_COLOR1] && Active->Tex[WCS_TEXTURE_COLOR1]->Enabled && Active->Tex[WCS_TEXTURE_COLOR1]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);
	//WidgetSetDisabled(IDC_HICOLORDROP, Active->Tex[WCS_TEXTURE_COLOR2] && Active->UseTexture[WCS_TEXTURE_COLOR2] && Active->Tex[WCS_TEXTURE_COLOR2]->Enabled && Active->Tex[WCS_TEXTURE_COLOR2]->TexParam[WCS_TEXTURE_OPACITY].CurValue >= 100.0);

	// Material GUI
	TexMatGUI->DisableWidgets();

	} // if

} // TextureEditGUI::DisableWidgets

/*===========================================================================*/

void TextureEditGUI::HideWidgets(void)
{
int ParentParam;
char TexType;

if (ActiveRoot && Active)
	{
	TexType = Active->GetTexType();
	ParentParam = Active->FindTextureNumberInParent();
	WidgetShow(IDC_LORANGE1, Active->ParamAvailable(1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_TEX0, Active->TextureAvailable(1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_HIRANGE1, Active->ParamAvailable(2));
	WidgetShow(IDC_TEX1, Active->TextureAvailable(2));
	WidgetShow(IDC_TEX14, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_STRATAFUNCTXT2, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_TEX15, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_BLENDFUNCTXT2, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_EDITCURVE, TexType == WCS_TEXTURE_TYPE_CUSTOMCURVE);

	WidgetShow(IDC_ANIMGRADIENT, Active->ParamAvailable(WCS_TEXTURE_COLOR1) && WCS_TEXTURETYPE_ISTWOVALUE(TexType)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_SELECTEDCOLORBOX, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_GRADIENTPOS, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_ADDCOLOR, Active->ParamAvailable(WCS_TEXTURE_COLOR1) && WCS_TEXTURETYPE_ISTWOVALUE(TexType)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_REMOVECOLOR, Active->ParamAvailable(WCS_TEXTURE_COLOR1) && WCS_TEXTURETYPE_ISTWOVALUE(TexType)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_DIFFUSECOLOR, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	//WidgetShow(IDC_SELCOLORTXT, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
	//	&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_BLENDSTYLE, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	//WidgetShow(IDC_LOCOLORTEXT, Active->ParamAvailable(WCS_TEXTURE_COLOR1)
	//	&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_TEX10, Active->TextureAvailable(WCS_TEXTURE_COLOR1)
		&& (ParentParam != WCS_TEXTURE_STRATAFUNC) && (ParentParam != WCS_TEXTURE_BLENDINGFUNC));
	//WidgetShow(ID_COLORPOT2, Active->ParamAvailable(WCS_TEXTURE_COLOR2));
	//WidgetShow(IDC_HICOLORTEXT, Active->ParamAvailable(WCS_TEXTURE_COLOR2));
	WidgetShow(IDC_HIRANGE2, Active->ParamAvailable(2) && ! Active->ParamAvailable(WCS_TEXTURE_COLOR2));
	WidgetShow(IDC_TEX12, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_STRATAFUNCTXT, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_TEX13, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_BLENDFUNCTXT, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_TEX20, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_STRATAFUNCTXT3, Active->TextureAvailable(WCS_TEXTURE_STRATAFUNC));
	WidgetShow(IDC_TEX21, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_BLENDFUNCTXT3, Active->TextureAvailable(WCS_TEXTURE_BLENDINGFUNC));
	WidgetShow(IDC_CHECK3D, WCS_TEXTURETYPE_IS3DOPTIONAL(TexType));
	WidgetShow(IDC_CHECKQUANTIZE, WCS_TEXTURETYPE_QUANTIZEENABLED(TexType));

	WidgetShow(IDC_PARAM1, Active->ParamAvailable(3));
	WidgetShow(IDC_TEX2, Active->TextureAvailable(3));
	WidgetShow(IDC_PARAM2, Active->ParamAvailable(4));
	WidgetShow(IDC_TEX3, Active->TextureAvailable(4));
	WidgetShow(IDC_PARAM3, Active->ParamAvailable(5));
	WidgetShow(IDC_TEX4, Active->TextureAvailable(5));
	WidgetShow(IDC_PARAM4, Active->ParamAvailable(6));
	WidgetShow(IDC_TEX5, Active->TextureAvailable(6));
	WidgetShow(IDC_PARAM5, Active->ParamAvailable(7));
	WidgetShow(IDC_TEX6, Active->TextureAvailable(7));
	WidgetShow(IDC_PARAM6, Active->ParamAvailable(8));
	WidgetShow(IDC_TEX7, Active->TextureAvailable(8));
	WidgetShow(IDC_PARAM7, Active->ParamAvailable(9));
	WidgetShow(IDC_TEX8, Active->TextureAvailable(9));
	WidgetShow(IDC_PARAM8, Active->ParamAvailable(10));
	WidgetShow(IDC_TEX9, Active->TextureAvailable(10));

	WidgetShow(IDC_PARAMDROP, WCS_TEXTURETYPE_ISPARAMETER(TexType));
	WidgetShow(IDC_PARAMDROP_TEXT, WCS_TEXTURETYPE_ISPARAMETER(TexType));

	WidgetShow(IDC_CHECKANTIALIAS, WCS_TEXTURETYPE_USEANTIALIAS(TexType));
	WidgetShow(IDC_AASAMPLES, WCS_TEXTURETYPE_USESUMTABLEANTIALIAS(TexType));

	WidgetShow(IDC_AXISTEXT, WCS_TEXTURETYPE_USEAXIS(TexType));
	WidgetShow(IDC_RADIOALIGNX, WCS_TEXTURETYPE_USEAXIS(TexType));
	WidgetShow(IDC_RADIOALIGNY, WCS_TEXTURETYPE_USEAXIS(TexType));
	WidgetShow(IDC_RADIOALIGNZ, WCS_TEXTURETYPE_USEAXIS(TexType));

	WidgetShow(IDC_CLASSDROP, Active->InclExcl ? 1: 0);
	WidgetShow(IDC_CLASSTYPEDROP, Active->InclExclType ? 1: 0);
	WidgetShow(IDC_COORDSPACEDROP, WCS_TEXTUREPARAM_USECOORDSPACE(TexType) ? 1: 0);
	WidgetShow(IDC_COORDSPACETEXT, WCS_TEXTUREPARAM_USECOORDSPACE(TexType) ? 1: 0);
	WidgetShow(IDC_VERTEXMAPDROP, Active->CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_UVW || Active->CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX);
	} // if

} // TextureEditGUI::HideWidgets

/*===========================================================================*/

void TextureEditGUI::BuildComboList(bool ShowNonRemaps, bool ShowRemaps)
{
long ListEntry, ParentParam;

WidgetCBClear(IDC_TYPEDROP);
NumListTextures = 0;

if (ActiveRoot && Active)
	{
	ParentParam = Active->FindTextureNumberInParent();
	for (ListEntry = 0; ListEntry < WCS_TEXTURE_NUMTYPES; ListEntry ++)
		{
		bool AddIt = false;
		if (! (Active->ApplyToEcosys && WCS_TEXTURETYPE_ECOILLEGAL(ListEntry)) &&
			! ((ParentParam == WCS_TEXTURE_STRATAFUNC || ParentParam == WCS_TEXTURE_BLENDINGFUNC) && WCS_TEXTURETYPE_BLENDILLEGAL(ListEntry)) &&
			! (! Active->ApplyToEcosys && WCS_TEXTURETYPE_OBJECT3DILLEGAL2(ListEntry)) &&
			! (ListEntry == WCS_TEXTURE_TYPE_UVW && ! VertexUVWAvailable) &&
			! (ListEntry == WCS_TEXTURE_TYPE_COLORPERVERTEX && ! VertexCPVAvailable))
			{
			if(RootTexture::TextureRemapType[ListEntry]) // is it a Remap type?
				{
				if(ShowRemaps) AddIt = true;
				} // if
			else
				{
				if(ShowNonRemaps) AddIt = true;
				} // if
			if(AddIt)
				{
				WidgetCBAddEnd(IDC_TYPEDROP, RootTexture::GetUserName((char)ListEntry));
				TextureList[NumListTextures] = ListEntry;
				NumListTextures ++;
				} // if
			} // if
		} // for
	} // if

} // TextureEditGUI::BuildComboList

/*===========================================================================*/

long TextureEditGUI::FindInComboList(long TexNum)
{
long Ct;

for (Ct = 0; Ct < NumListTextures; Ct ++)
	{
	if (TextureList[Ct] == TexNum)
		return (Ct);
	} // for

return (-1);

} // TextureEditGUI::FindInComboList

/*===========================================================================*/

long TextureEditGUI::BuildList(Texture *Source, short Indent, short Enabled, char *TexName)
{
short TexNum, TempEnabled, MemAllocated = 0;
long ListPos, Selected = 0, TestSelected;
GradientCritter *CurGrad;

if (ActiveRoot && Active)
	{
	if (! Source)
		{
		if (! (TexName = (char *)AppMem_Alloc(256, 0)))
			return (0);
		MemAllocated = 1;
		Source = ActiveRoot->Tex;
		WidgetLBClear(IDC_PARLIST);
		} // if

	while (Source)
		{
		if (MemAllocated)
			{
			Indent = 0;
			Enabled = Source->Enabled;
			BuildListEntry(NULL, Source, 0, Indent, Enabled, TexName);
			ListPos = WidgetLBInsert(IDC_PARLIST, -1, TexName);
			WidgetLBSetItemData(IDC_PARLIST, ListPos, Source);
			if (Active == Source)
				Selected = ListPos;
			Indent = 1;
			} // if

		// do color textures first
		CurGrad = NULL;
		while (CurGrad = Source->ColorGrad.GetNextNode(CurGrad))
			{
			if (Source->ColorGrad.GetSpecialDataExists(CurGrad))
				{
				TempEnabled = (Enabled && ((ColorTextureThing *)CurGrad->GetThing())->Tex->Enabled);
				BuildListEntry(Source, ((ColorTextureThing *)CurGrad->GetThing())->Tex, WCS_TEXTURE_COLOR1, Indent, TempEnabled, TexName);
				ListPos = WidgetLBInsert(IDC_PARLIST, -1, TexName);
				WidgetLBSetItemData(IDC_PARLIST, ListPos, ((ColorTextureThing *)CurGrad->GetThing())->Tex);
				if (Active == ((ColorTextureThing *)CurGrad->GetThing())->Tex)
					Selected = ListPos;
				if ((TestSelected = BuildList(((ColorTextureThing *)CurGrad->GetThing())->Tex, Indent + 1, TempEnabled, TexName)) > 0)
					Selected = TestSelected;
				} // if
			} // while

		// now the rest
		for (TexNum = 0; TexNum < WCS_TEXTURE_MAXPARAMTEXTURES; TexNum ++)
			{
			// these two are obsolete - replaced by AnimColorGradient
			if (TexNum == WCS_TEXTURE_COLOR1 || TexNum == WCS_TEXTURE_COLOR2)
				continue;

			if (Source->Tex[TexNum])
				{
				TempEnabled = (Enabled && Source->Tex[TexNum]->Enabled);
				BuildListEntry(Source, Source->Tex[TexNum], TexNum, Indent, TempEnabled, TexName);
				ListPos = WidgetLBInsert(IDC_PARLIST, -1, TexName);
				WidgetLBSetItemData(IDC_PARLIST, ListPos, Source->Tex[TexNum]);
				if (Active == Source->Tex[TexNum])
					Selected = ListPos;
				if ((TestSelected = BuildList(Source->Tex[TexNum], Indent + 1, TempEnabled, TexName)) > 0)
					Selected = TestSelected;
				} // if
			} // for

		if (MemAllocated)
			{
			Source = Source->Next;
			} // if
		else
			Source = NULL;
		} // while

	if (MemAllocated)
		{
		AppMem_Free(TexName, 256);
		WidgetLBSetCurSel(IDC_PARLIST, Selected);
		} // if
	} // if
else
	WidgetLBClear(IDC_PARLIST);

return (Selected);

} // TextureEditGUI::BuildList

/*===========================================================================*/

void TextureEditGUI::BuildListEntry(Texture *Host, Texture *Source, short TexNum, short Indent, short Enabled, char *Name)
{
short Ct;

if (Enabled)
	strcpy(Name, "* ");
else
	strcpy(Name, "  ");

for (Ct = 0; Ct < Indent * 3; Ct ++)
	strcat(Name, " ");

if (Host)
	{
	strcat(Name, Host->ParamName[TexNum]);
	strcat(Name, " <- ");
	} // if

strcat(Name, RootTexture::GetUserName(Source->GetTexType()));

} // TextureEditGUI::BuildListEntry

/*===========================================================================*/

void TextureEditGUI::ClearItemList(void)
{

WidgetLBClear(IDC_ITEMLIST);

} // TextureEditGUI::ClearItemList

/*===========================================================================*/

void TextureEditGUI::BuildItemList(void)
{
RasterAnimHostList *Current;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 64];
RasterAnimHost *CurrentRAHost = NULL;
RasterAnimHost **SelectedItems = NULL;

if (Active && Active->InclExclType)
	{
	BuildItemTypeList();
	return;
	} // if

if (! (Active && Active->InclExcl))
	return;

Current = Active->InclExcl->RAHList;

NumListItems = WidgetLBGetCount(IDC_ITEMLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (RasterAnimHost *)CurrentRAHost)
				{
				BuildItemListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_ITEMLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_ITEMLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_ITEMLIST, 1, Ct);
							break;
							} // if
						} // for
					} // if
				Ct ++;
				} // if
			else
				{
				FoundIt = 0;
				for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
					{
					if (Current->Me == (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildItemListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_ITEMLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_ITEMLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ITEMLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_ITEMLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildItemListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_ITEMLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_ITEMLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ITEMLIST, 1, Place);
								break;
								} // if
							} // for
						} // if
					NumListItems ++;
					Ct ++;
					} // else
				} // if
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_ITEMLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (RasterAnimHost *));

} // TextureEditGUI::BuildItemList

/*===========================================================================*/

void TextureEditGUI::BuildItemListEntry(char *ListName, RasterAnimHost *Me)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_FLAGS;
Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;

Me->GetRAHostProperties(&Prop);

if (Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Prop.Name);
strcat(ListName, " ");
strcat(ListName, Prop.Type);

} // TextureEditGUI::BuildItemListEntry()

/*===========================================================================*/

void TextureEditGUI::BuildItemTypeList(void)
{
TypeList *Current;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 64];
unsigned char CurrentType = 0;
unsigned char *SelectedItems = NULL;

if (! (Active && Active->InclExclType))
	return;

Current = Active->InclExclType->ItemTypes;

NumListItems = WidgetLBGetCount(IDC_ITEMLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (unsigned char *)AppMem_Alloc(NumSelected * sizeof (unsigned char), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (unsigned char)WidgetLBGetItemData(IDC_ITEMLIST, TempCt);	//lint !e507
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentType = Ct < NumListItems ? (unsigned char)WidgetLBGetItemData(IDC_ITEMLIST, Ct): 0xff;	//lint !e507
	
	if (Current)
		{
		if (Current->TypeNumber == CurrentType)
			{
			BuildItemTypeListEntry(ListName, Current->TypeNumber);
			WidgetLBReplace(IDC_ITEMLIST, Ct, ListName);
			WidgetLBSetItemData(IDC_ITEMLIST, Ct, (void *)Current->TypeNumber);
			if (SelectedItems)
				{
				for (SelCt = 0; SelCt < NumSelected; SelCt ++)
					{
					if (SelectedItems[SelCt] == Current->TypeNumber)
						{
						WidgetLBSetSelState(IDC_ITEMLIST, 1, Ct);
						break;
						} // if
					} // for
				} // if
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current->TypeNumber == (unsigned char)WidgetLBGetItemData(IDC_ITEMLIST, TempCt))	//lint !e507
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildItemTypeListEntry(ListName, Current->TypeNumber);
				WidgetLBReplace(IDC_ITEMLIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_ITEMLIST, TempCt, (void *)Current->TypeNumber);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->TypeNumber)
							{
							WidgetLBSetSelState(IDC_ITEMLIST, 1, TempCt);
							break;
							} // if
						} // for
					} // if
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_ITEMLIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildItemTypeListEntry(ListName, Current->TypeNumber);
				Place = WidgetLBInsert(IDC_ITEMLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_ITEMLIST, Place, (void *)Current->TypeNumber);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->TypeNumber)
							{
							WidgetLBSetSelState(IDC_ITEMLIST, 1, Place);
							break;
							} // if
						} // for
					} // if
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_ITEMLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (unsigned char));

} // TextureEditGUI::BuildItemTypeList

/*===========================================================================*/

void TextureEditGUI::BuildItemTypeListEntry(char *ListName, unsigned char Me)
{
strcpy(ListName, "* ");
strcat(ListName, Active->InclExclType->GetTypeName(Me));

} // TextureEditGUI::BuildItemTypeListEntry()

/*===========================================================================*/

void TextureEditGUI::FillClassDrop(void)
{
long Pos, MyClass;

for (MyClass = WCS_EFFECTSSUBCLASS_LAKE; MyClass < WCS_MAXIMPLEMENTED_EFFECTS; MyClass ++)
	{
	if (Active->ApproveInclExclClass(MyClass))
		{
		Pos = WidgetCBInsert(IDC_CLASSDROP, -1, EffectsHost->GetEffectTypeNameNonPlural(MyClass));
		WidgetCBSetItemData(IDC_CLASSDROP, Pos, (void *)MyClass);
		} // if
	} // for
WidgetCBSetCurSel(IDC_CLASSDROP, 0);

} // TextureEditGUI::FillClassDrop

/*===========================================================================*/

void TextureEditGUI::FillClassTypeDrop(void)
{
long Pos;
unsigned char MyType = 0;
char *TypeName;

while (TypeName = IncludeExcludeTypeList::GetTypeName(MyType))
	{
	// snow is not currently supported as a separate rendering layer
	if (MyType != WCS_INCLEXCLTYPE_SNOW)
		{
		Pos = WidgetCBInsert(IDC_CLASSTYPEDROP, -1, TypeName);
		WidgetCBSetItemData(IDC_CLASSTYPEDROP, Pos, (void *)MyType);
		} // if
	MyType ++;
	} // for
WidgetCBSetCurSel(IDC_CLASSTYPEDROP, 0);

} // TextureEditGUI::FillClassTypeDrop

/*===========================================================================*/

void TextureEditGUI::AddItem(void)
{
long AddClass, Current;

if (Active && Active->InclExclType)
	{
	AddItemType();
	return;
	} // if

if (! (Active && Active->InclExcl))
	return;

Current = WidgetCBGetCurSel(IDC_CLASSDROP);
if ((AddClass = (long)WidgetCBGetItemData(IDC_CLASSDROP, Current, 0)) != (long)LB_ERR)
	EffectsHost->AddAttributeByList(Active->InclExcl, AddClass);

} // TextureEditGUI::AddItem

/*===========================================================================*/

void TextureEditGUI::AddItemType(void)
{
long Current;
long AddItemType;

if (! (Active && Active->InclExclType))
	return;

Current = WidgetCBGetCurSel(IDC_CLASSTYPEDROP);
if ((AddItemType = (long)WidgetCBGetItemData(IDC_CLASSTYPEDROP, Current, 0)) != (long)LB_ERR)
	Active->InclExclType->AddType((unsigned char)AddItemType);

} // TextureEditGUI::AddItemType

/*===========================================================================*/

void TextureEditGUI::RemoveItem(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if (Active && Active->InclExclType)
	{
	RemoveItemType();
	return;
	} // if

if (! (Active && Active->InclExcl))
	return;

if ((NumListEntries = WidgetLBGetCount(IDC_ITEMLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
			{
			NumSelected ++;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					if (Active->InclExcl->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
						{
						//EffectsHost->RemoveRAHost(RemoveItems[Ct], 0);
						} // if
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Item", "There are no items selected to remove.");
		} // else
	} // if

BuildItemList();

} // TextureEditGUI::RemoveItem

/*===========================================================================*/

void TextureEditGUI::RemoveItemType(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
unsigned char *RemoveItems;

if (! (Active && Active->InclExclType))
	return;

if ((NumListEntries = WidgetLBGetCount(IDC_ITEMLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
			{
			NumSelected ++;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (unsigned char *)AppMem_Alloc(NumSelected * sizeof (unsigned char), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
					{
					RemoveItems[Found ++] = (unsigned char)WidgetLBGetItemData(IDC_ITEMLIST, Ct);	//lint !e507
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				Active->InclExclType->RemoveType(RemoveItems[Ct]);
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (unsigned char));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Item", "There are no items selected to remove.");
		} // else
	} // if

BuildItemTypeList();

} // TextureEditGUI::RemoveItemType

/*===========================================================================*/

void TextureEditGUI::EditItem(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if (Active && Active->InclExclType)
	{
	return;
	} // if

if (! (Active && Active->InclExcl))
	return;

if ((NumListEntries = WidgetLBGetCount(IDC_ITEMLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // TextureEditGUI::EditItem

/*===========================================================================*/

void TextureEditGUI::DoKeep(void)
{

Active = NULL;	// set this to NULL so that any notifications received will not be processed in HandleNotifyEvent

} // TextureEditGUI::DoKeep()

/*===========================================================================*/

void TextureEditGUI::Cancel(void)
{
NotifyTag Changes[2];

ActiveRoot->Copy(ActiveRoot, &Backup);
Active = ActiveRoot->Tex;
ActiveGrad = NULL;
ActiveNode = NULL;
HideWidgets();
SetPanels();

Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());

} // TextureEditGUI::DoCancel()

/*===========================================================================*/

void TextureEditGUI::DoTextureType(void)
{
long Current, Found = 0, ParentParam;
Texture *NewTex = NULL, *PrevTex, *CurTex;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_TYPEDROP);
if (Current >= 0 && Current < WCS_TEXTURE_NUMTYPES)
	{
	Current = TextureList[Current];

	if (ActiveRoot && Active)
		{
		if (ActiveRoot->ApplyToEcosys && WCS_TEXTURETYPE_ECOILLEGAL(Current))
			{
			WidgetCBSetCurSel(IDC_TYPEDROP, FindInComboList(Active->GetTexType()));
			} // if
		else if ((((ParentParam = Active->FindTextureNumberInParent()) == WCS_TEXTURE_STRATAFUNC) || (ParentParam == WCS_TEXTURE_BLENDINGFUNC)) && WCS_TEXTURETYPE_BLENDILLEGAL(Current))
			{
			WidgetCBSetCurSel(IDC_TYPEDROP, FindInComboList(Active->GetTexType()));
			} // else if
		else
			{
			ParentParam = Active->FindTextureNumberInParent();
			if (NewTex = NewTexture(Active->RAParent, (short)Current, (short)ParentParam, TRUE, FALSE, NULL))
				{
				CurTex = ActiveRoot->Tex;
				PrevTex = NULL;
				while (CurTex && ! Found)
					{
					if (Active == CurTex)
						{
						if (PrevTex)
							{
							PrevTex->Next = NewTex;
							//NewTex->Prev = PrevTex;
							NewTex->Next = Active->Next;
							//if (NewTex->Next)
							//	NewTex->Next->Prev = NewTex;
							Active->Next = NULL;
							Active->Prev = NULL;
							} // if
						else
							{
							ActiveRoot->Tex = NewTex;
							//NewTex->Prev = NULL;
							NewTex->Next = Active->Next;
							//if (NewTex->Next)
							//	NewTex->Next->Prev = NewTex;
							Active->Next = NULL;
							Active->Prev = NULL;
							} // else
						Found = 1;
						} // if
					else
						{
						if (Found = SearchAndReplace(CurTex, NewTex))
							{
							break;
							} // if
						} // else
					PrevTex = CurTex;
					CurTex = CurTex->Next;
					} // while
				delete Active;
				Active = NewTex;
				HideWidgets();
				SetPanels();
				Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());
				ConfigureWidgets();
				ImageHost->SetParam(1, WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ATTRIBUTEADDED, 0, 0);
				} // if
			} // else
		} // if
	} // if

} // TextureEditGUI::DoTextureType

/*===========================================================================*/

short TextureEditGUI::SearchAndReplace(Texture *CurTex, Texture *NewTex)
{
short TexNum, ChildTexNum, Found = 0;
GradientCritter *CurGrad;

for (TexNum = 0; TexNum < WCS_TEXTURE_MAXPARAMTEXTURES; TexNum ++)
	{
	if (CurTex->Tex[TexNum])
		{
		if (Active == CurTex->Tex[TexNum])
			{
			for (ChildTexNum = 0; ChildTexNum < WCS_TEXTURE_MAXPARAMTEXTURES; ChildTexNum ++)
				{
				if (CurTex->Tex[TexNum]->Tex[ChildTexNum])
					{
					CurTex->Tex[TexNum]->Tex[ChildTexNum]->Prev = NewTex;
					CurTex->Tex[TexNum]->Tex[ChildTexNum]->RAParent = NewTex;
					} // if
				} // for
			CurTex->Tex[TexNum] = NewTex;
			NewTex->Prev = CurTex;
			Found = 1;
			break;
			} // if
		else
			{
			if (Found = SearchAndReplace(CurTex->Tex[TexNum], NewTex))
				break;
			} // else
		} // if
	} // for
if (! Found)
	{
	CurGrad = NULL;
	while (CurGrad = CurTex->ColorGrad.GetNextNode(CurGrad))
		{
		if (((ColorTextureThing *)CurGrad->GetThing())->Tex)
			{
			if (Active == ((ColorTextureThing *)CurGrad->GetThing())->Tex)
				{
				for (ChildTexNum = 0; ChildTexNum < WCS_TEXTURE_MAXPARAMTEXTURES; ChildTexNum ++)
					{
					if (((ColorTextureThing *)CurGrad->GetThing())->Tex->Tex[ChildTexNum])
						{
						((ColorTextureThing *)CurGrad->GetThing())->Tex->Tex[ChildTexNum]->Prev = NewTex;
						((ColorTextureThing *)CurGrad->GetThing())->Tex->Tex[ChildTexNum]->RAParent = NewTex;
						} // if
					} // for
				((ColorTextureThing *)CurGrad->GetThing())->Tex = NewTex;
				NewTex->Prev = CurTex;
				Found = 1;
				break;
				} // if
			else
				{
				if (Found = SearchAndReplace(((ColorTextureThing *)CurGrad->GetThing())->Tex, NewTex))
					break;
				} // else
			} // if
		} // while
	} // if

return (Found);

} // TextureEditGUI::SearchAndReplace

/*===========================================================================*/

void TextureEditGUI::DoViewDirection(void)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_VIEWDROP);
if (ActiveRoot)
	ActiveRoot->PreviewDirection = (unsigned char)Current;
Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());

} // TextureEditGUI::DoViewDirection

/*===========================================================================*/

void TextureEditGUI::DoSelectParam(void)
{
long Current;
NotifyTag Changes[2];

if (ActiveRoot && Active)
	{
	Current = WidgetCBGetCurSel(IDC_PARAMDROP);
	Active->Misc = ParamDropItem[Current];
	Active->SetMiscDefaults();
	Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());
	} // if

} // TextureEditGUI::DoSelectParam

/*===========================================================================*/

void TextureEditGUI::DoTextureEnabled(void)
{

BuildList();
ImageHost->SetParam(1, WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ATTRIBUTEADDED, 0, 0);

} // TextureEditGUI::DoTextureEnabled

/*===========================================================================*/

void TextureEditGUI::EnableInputParamTexture(unsigned short WidID)
{
short TexOffset;

if (ActiveRoot)
	{
	TexOffset = (WidID == IDC_TEX17) ? 0: (WidID == IDC_TEX18) ? 1: 2;
	if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_SIZE)
		{
		EnableTexture(WidID, WCS_TEXTURE_SIZE1 + TexOffset);
		} // if size
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_CENTER)
		{
		EnableTexture(WidID, WCS_TEXTURE_CENTER1 + TexOffset);
		} // if center
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_FALLOFF)
		{
		EnableTexture(WidID, WCS_TEXTURE_FALLOFF1 + TexOffset);
		} // if falloff
	else if (ActiveRoot->ShowSize == WCS_TEXTURE_SHOW_ROTATION)
		{
		EnableTexture(WidID, WCS_TEXTURE_ROTATION1 + TexOffset);
		} // if rotation
	} // if

} // TextureEditGUI::EnableInputParamTexture

/*===========================================================================*/

void TextureEditGUI::EnableTexture(unsigned short WidID, short TexNum)
{
NotifyTag Changes[2];

Changes[1] = NULL;

if (ActiveRoot && Active)
	{
	if (WidgetGetCheck(WidID))
		{
		if (! Active->Tex[TexNum])
			{
			if (Active->Tex[TexNum] = 
				NewTexture(Active, (TexNum == WCS_TEXTURE_STRATAFUNC || TexNum == WCS_TEXTURE_BLENDINGFUNC) ? WCS_TEXTURE_TYPE_BELLCURVE: WCS_TEXTURE_TYPE_FRACTALNOISE, TexNum, FALSE, TRUE, NULL))
				{
				Active->Tex[TexNum]->Prev = Active;
				Active = Active->Tex[TexNum];
				HideWidgets();
				SetPanels();
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				} // if
			} // if
		else
			{
			Active->Tex[TexNum]->Enabled = 1;
			Changes[0] = MAKE_ID(Active->Tex[TexNum]->GetNotifyClass(), Active->Tex[TexNum]->GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->Tex[TexNum]->GetRAHostRoot());
			} // else
		} // if
	else if (Active->Tex[TexNum])
		{
		Active->Tex[TexNum]->Enabled = 0;
		Changes[0] = MAKE_ID(Active->Tex[TexNum]->GetNotifyClass(), Active->Tex[TexNum]->GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->Tex[TexNum]->GetRAHostRoot());
		} // else
	ImageHost->SetParam(1, WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ATTRIBUTEADDED, 0, 0);
	} // if

} // TextureEditGUI::EnableTexture

/*===========================================================================*/

void TextureEditGUI::EnableGradientTexture(unsigned short WidID)
{
NotifyTag Changes[2];
GradientCritter *CurNode;

Changes[1] = NULL;

if (ActiveRoot && Active && (CurNode = Active->ColorGrad.GetActiveNode()))
	{
	if (WidgetGetCheck(WidID))
		{
		if (! ((ColorTextureThing *)CurNode->GetThing())->Tex)
			{
			if (((ColorTextureThing *)CurNode->GetThing())->Tex = 
				NewTexture((ColorTextureThing *)CurNode->GetThing(), WCS_TEXTURE_TYPE_FRACTALNOISE, WCS_TEXTURE_COLOR1, FALSE, TRUE, &((ColorTextureThing *)CurNode->GetThing())->Color))
				{
				((ColorTextureThing *)CurNode->GetThing())->Tex->Prev = Active;
				Active = ((ColorTextureThing *)CurNode->GetThing())->Tex;
				HideWidgets();
				SetPanels();
				// call this on the texture so that it causes a ConfigureWidgets()
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				} // if
			} // if
		else
			{
			((ColorTextureThing *)CurNode->GetThing())->Tex->Enabled = 1;
			Changes[0] = MAKE_ID(((ColorTextureThing *)CurNode->GetThing())->Tex->GetNotifyClass(), ((ColorTextureThing *)CurNode->GetThing())->Tex->GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurNode->GetThing())->Tex->GetRAHostRoot());
			} // else
		} // if
	else if (((ColorTextureThing *)CurNode->GetThing())->Tex)
		{
		((ColorTextureThing *)CurNode->GetThing())->Tex->Enabled = 0;
		Changes[0] = MAKE_ID(((ColorTextureThing *)CurNode->GetThing())->Tex->GetNotifyClass(), ((ColorTextureThing *)CurNode->GetThing())->Tex->GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurNode->GetThing())->Tex->GetRAHostRoot());
		} // else
	ImageHost->SetParam(1, WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ATTRIBUTEADDED, 0, 0);
	} // if

} // TextureEditGUI::EnableGradientTexture

/*===========================================================================*/

void TextureEditGUI::DoTextureList(void)
{
long Current;
Texture *Test;

if ((Current = WidgetLBGetCurSel(IDC_PARLIST)) != -1)
	{
	if ((Test = (Texture *)WidgetLBGetItemData(IDC_PARLIST, Current)) != (Texture *)-1 && Test)
		{
		Active = Test;
		HideWidgets();
		SetPanels();
		ConfigureWidgets();
		} // if
	} // if

} // TextureEditGUI::DoTextureList

/*===========================================================================*/

Texture *TextureEditGUI::NewTexture(RasterAnimHost *Parent, short TexType, short TexNum, short CopyActive, int TestActiveForColor, AnimColorTime *ColorCopy)
{
Texture *NewTex = NULL;
unsigned char ColorSource;

if (! ColorCopy)
	ColorCopy = DefaultColor;
// not setting this to NULL causes crash trying to repaint gradient widget before ConfigureWidgets is called again
// to reconfigure the gradient. I have no idea how this is possible.
WidgetAGConfig(IDC_ANIMGRADIENT, NULL);
if (TestActiveForColor)	// when creating a new texture
	ColorSource = Active->ApplyToColor && (WCS_TEXTUREPARAM_ISCOLOR(TexNum) || Active->IsColorChild(TexNum));
else if (Active->Prev)	// when retyping an existing texture
	ColorSource = Active->ApplyToColor && (WCS_TEXTUREPARAM_ISCOLOR(TexNum) || Active->Prev->IsColorChild(TexNum));
else
	ColorSource = Active->ApplyToColor && WCS_TEXTUREPARAM_ISCOLOR(TexNum);

switch (TexType)
	{
	case WCS_TEXTURE_TYPE_PLANARIMAGE:
		{
		NewTex = (Texture *)new PlanarImageTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CYLINDRICALIMAGE:
		{
		NewTex = (Texture *)new CylindricalImageTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SPHERICALIMAGE:
		{
		NewTex = (Texture *)new SphericalImageTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CUBICIMAGE:
		{
		NewTex = (Texture *)new CubicImageTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE:
		{
		NewTex = (Texture *)new FrontProjectionTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE:
		{
		NewTex = (Texture *)new EnvironmentMapTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_UVW:
		{
		NewTex = (Texture *)new UVImageTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_COLORPERVERTEX:
		{
		NewTex = (Texture *)new ColorPerVertexTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_STRIPES:
		{
		NewTex = (Texture *)new StripeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SOFTSTRIPES:
		{
		NewTex = (Texture *)new SoftStripeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SINGLESTRIPE:
		{
		NewTex = (Texture *)new SingleStripeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE:
		{
		NewTex = (Texture *)new SingleSoftStripeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_WOODGRAIN:
		{
		NewTex = (Texture *)new WoodGrainTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MARBLE:
		{
		NewTex = (Texture *)new MarbleTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BRICK:
		{
		NewTex = (Texture *)new BrickTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_DOTS:
		{
		NewTex = (Texture *)new DotsTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_VEINS:
		{
		NewTex = (Texture *)new VeinTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CRUST:
		{
		NewTex = (Texture *)new CrustTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_UNDERWATER:
		{
		NewTex = (Texture *)new UnderwaterTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_FRACTALNOISE:
		{
		NewTex = (Texture *)new FractalNoiseTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new MultiFractalNoiseTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new HybridMultiFractalNoiseTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new RidgedMultiFractalNoiseTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HETEROTERRAINNOISE:
		{
		NewTex = (Texture *)new HeteroTerrainNoiseTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_TURBULENCE:
		{
		NewTex = (Texture *)new TurbulenceTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F1CELLBASIS:
		{
		NewTex = (Texture *)new F1CellBasisTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2CELLBASIS:
		{
		NewTex = (Texture *)new F2CellBasisTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MF1CELLBASIS:
		{
		NewTex = (Texture *)new F2mF1CellBasisTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F3MF1CELLBASIS:
		{
		NewTex = (Texture *)new F3mF1CellBasisTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F1MANHATTAN:
		{
		NewTex = (Texture *)new F1ManhattanTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MANHATTAN:
		{
		NewTex = (Texture *)new F2ManhattanTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MF1MANHATTAN:
		{
		NewTex = (Texture *)new F2mF1ManhattanTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BIRDSHOT:
		{
		NewTex = (Texture *)new BirdshotTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_ADD:
		{
		NewTex = (Texture *)new AddTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_COMPOSITE:
		{
		NewTex = (Texture *)new CompositeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CONTRAST:
		{
		NewTex = (Texture *)new ContrastTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_DARKEN:
		{
		NewTex = (Texture *)new DarkenTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_LIGHTEN:
		{
		NewTex = (Texture *)new LightenTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_LEVELS:
		{
		NewTex = (Texture *)new LevelsTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SKEW:
		{
		NewTex = (Texture *)new SkewTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BELLCURVE:
		{
		NewTex = (Texture *)new BellCurveTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SQUAREWAVE:
		{
		NewTex = (Texture *)new SquareWaveTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SAWTOOTH:
		{
		NewTex = (Texture *)new SawtoothTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_STEP:
		{
		NewTex = (Texture *)new StepTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SLOPE:
		{
		NewTex = (Texture *)new SlopeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GAMMA:
		{
		NewTex = (Texture *)new GammaTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BIAS:
		{
		NewTex = (Texture *)new BiasTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GAIN:
		{
		NewTex = (Texture *)new GainTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CUSTOMCURVE:
		{
		NewTex = (Texture *)new CustomCurveTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MAXIMUM:
		{
		NewTex = (Texture *)new MaximumTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MAXIMUMSWITCH:
		{
		NewTex = (Texture *)new MaximumSwitchTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MINIMUM:
		{
		NewTex = (Texture *)new MinimumTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MINIMUMSWITCH:
		{
		NewTex = (Texture *)new MinimumSwitchTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MULTIPLY:
		{
		NewTex = (Texture *)new MultiplyTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SUBTRACT:
		{
		NewTex = (Texture *)new SubtractTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_THRESHOLD:
		{
		NewTex = (Texture *)new ThresholdTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_TERRAINPARAM:
		{
		NewTex = (Texture *)new TerrainParameterTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GRADIENT:
		{
		NewTex = (Texture *)new GradientTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDE:
		{
		NewTex = (Texture *)new IncludeExcludeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE:
		{
		NewTex = (Texture *)new IncludeExcludeTypeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HSVMERGE:
		{
		NewTex = (Texture *)new HSVMergeTexture(Parent, TexNum, Active->ApplyToEcosys, ColorSource, CopyActive ? Active: NULL, ColorCopy);
		break;
		} // 
// <<<>>> ADD_NEW_TEXTURE
	} // switch

return (NewTex);

} // TextureEditGUI::NewTexture

/*===========================================================================*/

void TextureEditGUI::DoEditImage(void)
{

if (ActiveRoot && Active && Active->Img && Active->Img->GetRaster())
	{
	ImageHost->SetActive(Active->Img->GetRaster());
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // if

} // TextureEditGUI::DoEditImage()

/*===========================================================================*/

void TextureEditGUI::DoNewImage(void)
{
long Current;
NotifyTag Changes[2];
Raster *NewRast, *MyRast, *MadeRast = NULL;
RasterShell *MyShell;

if (ActiveRoot && Active)
	{
	Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
	if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current)) != (Raster *)-1 && NewRast)
		|| (MadeRast = NewRast = ImageHost->AddRequestRaster()))
		{
		if (MyShell = Active->Img)
			{
			if (MyRast = MyShell->GetRaster())
				{
				MyShell->SetHost(NULL);
				if (MyRast->RemoveAttribute((RasterShell *)MyShell))
					{
					Active->Img = NULL;
					MyShell = NULL;
					} // if
				} // if
			} // if
		Active->SetRaster(NewRast);
		if (MadeRast)
			{
			ImageHost->SetActive(MadeRast);
			} // if
		ConfigureImageWidgets();
		if (Active->Img && Active->Img->GetRaster() && Active->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
			{
			if (Active->CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				if (Active->ApproveCoordSpace(WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED))
					{
					Active->SetCoordSpace(WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED, TRUE);
					} // if
				} // if
			SetPanels();
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		else if (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED && ! ValidateGeoRefImage())
			{
			SetPanels();
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		} // if
	} // if

} // TextureEditGUI::DoNewImage

/*===========================================================================*/

void TextureEditGUI::AdjustTexturePosition(long Direction)
{
#ifdef _WIN32
if (ActiveRoot && Active)
	{
	if (ActiveRoot->AdjustTextureOrder(Active, Direction))
		{
		BuildList();
		} // if
	} // if
#endif // _WIN32

} // TextureEditGUI::AdjustTexturePosition

/*===========================================================================*/

void TextureEditGUI::AddNewTexture(void)
{
Texture *NewActive;
NotifyTag Changes[2];

if (ActiveRoot)
	{
	if (NewActive = ActiveRoot->AddNewTexture(DefaultColor))
		{
		Active = NewActive;
		HideWidgets();
		SetPanels();
		// send the message again now that there is a new Active
		Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());
		} // if
	} // if

} // TextureEditGUI::AddNewTexture

/*===========================================================================*/

void TextureEditGUI::RemoveTexture(void)
{
NotifyTag Changes[2];

if (ActiveRoot && Active)
	{
	if (UserMessageOKCAN(ActiveRoot->GetUserName(Active->GetTexType()), "Remove this texture?"))
		{
		Active = ActiveRoot->RemoveTexture(Active);
		HideWidgets();
		SetPanels();
		ConfigureWidgets();
		Changes[0] = MAKE_ID(ActiveRoot->GetNotifyClass(), ActiveRoot->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveRoot->GetRAHostRoot());
		} // if
	} // if

} // TextureEditGUI::RemoveTexture

/*===========================================================================*/

void TextureEditGUI::CopyTexture(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost;
char CopyMsg[256];

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveRoot && (CopyHost = Active))
		{
		CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		CopyHost->GetRAHostProperties(CopyProp);
		if (CopyProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			RasterAnimHost::SetCopyOfRAHost(CopyHost);
			sprintf(CopyMsg, "%s %s copied to clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
			} // if
		else
			{
			UserMessageOK("Copy", "Selected item cannot be copied.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // TextureEditGUI::CopyTexture

/*===========================================================================*/

void TextureEditGUI::PasteTexture(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *PasteHost, *CopyHost;
char CopyMsg[256], CopyIllegal = 0;
Texture *TempHost;

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveRoot && (PasteHost = Active))
		{
		if (CopyHost = RasterAnimHost::GetCopyOfRAHost())
			{
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			CopyHost->GetRAHostProperties(CopyProp);
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
			PasteHost->GetRAHostProperties(CopyProp);
			if (CopyProp->DropOK)
				{
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				CopyProp->DropSource = CopyHost;
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				// determine if copying is legal - can't copy from a texture to a child of itself or a parent of itself
				TempHost = (Texture *)PasteHost;
				while (TempHost)
					{
					if ((Texture *)CopyHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->Prev;
					} // while
				TempHost = (Texture *)CopyHost;
				while (TempHost)
					{
					if ((Texture *)PasteHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->Prev;
					} // while
				if (! CopyIllegal)
					{
					PasteHost->SetRAHostProperties(CopyProp);
					sprintf(CopyMsg, "%s %s pasted from clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Can't copy from parent to child or child to its parent or copy texture to itself.");
					} // else
				} // if
			else
				{
				UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // TextureEditGUI::PasteTexture

/*===========================================================================*/

long TextureEditGUI::HandleBackgroundCrunch(int Siblings)
{
int Handled = 0;
Texture *TexOp;

if (ActiveRoot && Active)
	{
	if (SampleData[0]->Running)
		{
		if (ActiveRoot->EvalOneSampleLine(SampleData[0]) || ! (SampleData[0]->y % 10))
			ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, RootRast);
		Handled = 1;
		} // if
	if (SampleData[1]->Running)
		{
		TexOp = ActiveParent ? ActiveParent: Active;
		if (TexOp->EvalOneSampleLine(SampleData[1]) || ! (SampleData[1]->y % 10))
			ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, CompChildRast);
		if (SampleData[1]->Running && (TexOp->EvalOneSampleLine(SampleData[1]) || ! (SampleData[1]->y % 10)))
			ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, CompChildRast);
		Handled = 1;
		} // if
	if (SampleData[2]->Running)
		{
		if (Active->EvalOneSampleLine(SampleData[2]) || ! (SampleData[2]->y % 10))
			ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, CompRast);
		if (SampleData[2]->Running && (Active->EvalOneSampleLine(SampleData[2]) || ! (SampleData[2]->y % 10)))
			ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, CompRast);
		if (SampleData[2]->Running && (Active->EvalOneSampleLine(SampleData[2]) || ! (SampleData[2]->y % 10)))
			ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, CompRast);
		Handled = 1;
		} // if
	} // if

if (! Handled)
	{
	// we're all done
	BackgroundInstalled = 0;
	return (1);		// 1 uninstalls background process
	} // if

// we've got more to do, come again when you have more time
return (0);

} // TextureEditGUI::HandleBackgroundCrunch

/*===========================================================================*/

void TextureEditGUI::UpdateThumbnails(short UpdateRoot)
{
int Result;
Texture *TexOp;

if (ActiveRoot && Active)
	{
	// component only
	if (ActiveRoot->ShowComponentNail)
		{
		SampleData[2]->PreviewSize = ActiveRoot->PreviewSize.CurValue;
		SampleData[2]->PreviewDirection = ActiveRoot->PreviewDirection;
		SampleData[2]->AndChildren = 0;
		if ((Result = Active->EvalSampleInit(CompRast, SampleData[2])) == 1)
			{
			if (! BackgroundInstalled)
				{
				if (GlobalApp->AddBGHandler(this))
					BackgroundInstalled = 1;
				} // if
			} // if initialized
		else if (Result == 2)	// already finished the plot
			ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, CompRast);
		TNailDrawn[2] = 1;
		} // if
	else if (TNailDrawn[2])
		{
		TNailDrawn[2] = 0;
		ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, NULL);
		} // else if erase it

	// component and children
	if (ActiveRoot->ShowCompChildNail)
		{
		SampleData[1]->PreviewSize = ActiveRoot->PreviewSize.CurValue;
		SampleData[1]->PreviewDirection = ActiveRoot->PreviewDirection;
		SampleData[1]->AndChildren = 1;
		if (WCS_TEXTURETYPE_PARENTPREVIEW(Active->TexType) && ActiveRoot->ShowComponentNail)
			{
			TexOp = (ActiveParent = Active->Prev) ? ActiveParent: Active;
			} // if
		else
			{
			ActiveParent = NULL;
			TexOp = Active;
			} // else
		WidgetSetText(IDC_CHECKCOMPONENTANDCHILD, TexOp == Active ? "Element && Children": "Parent && Children");
		if ((Result = TexOp->EvalSampleInit(CompChildRast, SampleData[1])) == 1)
			{
			if (! BackgroundInstalled)
				{
				if (GlobalApp->AddBGHandler(this))
					BackgroundInstalled = 1;
				} // if
			} // if initialized
		else if (Result == 2)	// already finished the plot
			ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, CompChildRast);
		TNailDrawn[1] = 1;
		} // if
	else
		{
		ActiveParent = NULL;
		if (WCS_TEXTURETYPE_ISCURVE(Active->TexType) && ActiveRoot->ShowComponentNail)
			TexOp = Active->Prev ? Active->Prev: Active;
		else
			TexOp = Active;
		WidgetSetText(IDC_CHECKCOMPONENTANDCHILD, TexOp == Active ? "Element && Children": "Parent && Children");
		if (TNailDrawn[1])
			{
			TNailDrawn[1] = 0;
			ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, NULL);
			} // if
		} // else if erase it

	// root
	if (ActiveRoot->ShowRootNail)
		{
		SampleData[0]->PreviewSize = ActiveRoot->PreviewSize.CurValue;
		SampleData[0]->PreviewDirection = ActiveRoot->PreviewDirection;
		SampleData[0]->AndChildren = 1;
		if (UpdateRoot && ActiveRoot->EvalSampleInit(RootRast, SampleData[0]))
			{
			if (! BackgroundInstalled)
				{
				if (GlobalApp->AddBGHandler(this))
					BackgroundInstalled = 1;
				} // if
			} // if initialized
		TNailDrawn[0] = 1;
		} // if
	else if (TNailDrawn[0])
		{
		TNailDrawn[0] = 0;
		ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, NULL);
		} // else if erase it
	} // if
else
	{
	ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, NULL);
	} // else

} // TextureEditGUI::UpdateThumbnails

/*===========================================================================*/

void TextureEditGUI::InitNoise(int ParamNum)
{

if (ActiveRoot && Active)
	{
	if (Active->ReInitNoise(ParamNum))
		Active->InitNoise();
	if (Active->ReInitBasis(ParamNum))
		Active->InitBasis();
	} // if

} // TextureEditGUI::InitNoise

/*===========================================================================*/

void TextureEditGUI::AddColorNode(void)
{
double NewPos;
GradientCritter *NewNode;
char PositionStr[64];

if (ActiveRoot && Active)
	{
	ActiveGrad = Active->ColorGrad.GetActiveNode();
	strcpy(PositionStr, ActiveGrad ? "50 %": "0 %");
	if (! ActiveGrad || GetInputString("Enter gradient position (in percent) for new color.", WCS_REQUESTER_POSDIGITS_ONLY, PositionStr))
		{
		NewPos = atof(PositionStr) * (1.0 / 100.0);
		if (NewNode = Active->ColorGrad.AddNodeNotify(NewPos, 1))
			{
			//ActiveGrad = NewNode;
			Active->ColorGrad.SetActiveNode(NewNode);
			} // if
		} // if
	} // if

} // TextureEditGUI::AddColorNode

/*===========================================================================*/

void TextureEditGUI::RemoveColorNode(void)
{
GradientCritter *RemoveNode;

if (ActiveRoot && Active && (RemoveNode = Active->ColorGrad.GetActiveNode()))
	{
	ActiveGrad = NULL;
	Active->ColorGrad.RemoveNode(RemoveNode);
	} // if

} // TextureEditGUI::RemoveColorNode

/*===========================================================================*/

void TextureEditGUI::BlendStyle(void)
{
long Current;
NotifyTag Changes[2];

if (ActiveRoot && Active && (ActiveGrad = Active->ColorGrad.GetActiveNode()))
	{
	Current = WidgetCBGetCurSel(IDC_BLENDSTYLE);
	ActiveGrad->BlendStyle = (unsigned char)Current;
	WidgetCBSetCurSel(IDC_BLENDSTYLE, ActiveGrad->BlendStyle);
	Changes[0] = MAKE_ID(Active->ColorGrad.GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->ColorGrad.GetRAHostRoot());
	} // if

} // TextureEditGUI::BlendStyle()

/*===========================================================================*/

int TextureEditGUI::ValidateGeoRefImage(void)
{
RasterAttribute *MyAttr;

if (ActiveRoot && Active && Active->CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	if (Active->Img && Active->Img->GetRaster())
		{
		if (! ((MyAttr = Active->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell()))
			{
			UserMessageOK("Georeferenced Image", "Selected image is not geo-referenced.\n Set up reference coordinates in the Image Object Library.");
			Active->CoordSpace = WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ;
			SetCoordSpaceDrop();
			} // if
		else
			return (1);
		} // if
	else
		{
		UserMessageOK("Georeferenced Image", "There is no image selected to geo-reference.");
		Active->CoordSpace = WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ;
		SetCoordSpaceDrop();
		} // else
	} // if

return (0);

} // TextureEditGUI::ValidateGeoRefImage

/*===========================================================================*/

void TextureEditGUI::FillCoordSpaceDrop(void)
{
long ListEntry, Pos;

WidgetCBClear(IDC_COORDSPACEDROP);

if (ActiveRoot && Active)
	{
	for (ListEntry = 0; ListEntry < WCS_TEXTURE_MAX_COORDSPACES; ListEntry ++)
		{
		if (Active->ApproveCoordSpace((unsigned char)ListEntry))
			{
			Pos = WidgetCBAddEnd(IDC_COORDSPACEDROP, RootTexture::GetCoordSpaceName((char)ListEntry));
			WidgetCBSetItemData(IDC_COORDSPACEDROP, Pos, (void *)ListEntry);
			} // if
		} // for
	} // if

} // TextureEditGUI::FillCoordSpaceDrop

/*===========================================================================*/

void TextureEditGUI::SetCoordSpaceDrop(void)
{
long ListEntry, ItemCount, CoordSpace, Pos = -1;

if (ActiveRoot && Active)
	{
	ItemCount = WidgetCBGetCount(IDC_COORDSPACEDROP);
	for (ListEntry = 0; ListEntry < ItemCount; ListEntry ++)
		{
		CoordSpace = (long)WidgetCBGetItemData(IDC_COORDSPACEDROP, ListEntry);
		if (CoordSpace == Active->CoordSpace)
			Pos = ListEntry;
		} // for
	} // if

WidgetCBSetCurSel(IDC_COORDSPACEDROP, Pos);
if (Active->CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_UVW || Active->CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX)
	{
	FillVertexMapCombo();
	SetVertexMapDrop();
	} // if

} // TextureEditGUI::SetCoordSpaceDrop

/*===========================================================================*/

void TextureEditGUI::SelectCoordSpace(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSPACEDROP);
Current = (long)WidgetCBGetItemData(IDC_COORDSPACEDROP, Current);
if (Current >= 0 && Current < WCS_TEXTURE_MAX_COORDSPACES)
	Active->SetCoordSpace((unsigned char)Current, TRUE);

if (Current == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	ValidateGeoRefImage();
	} // Start
HideWidgets();
SetPanels();

} // TextureEditGUI::SelectCoordSpace

/*===========================================================================*/

void TextureEditGUI::FillVertexMapCombo(void)
{
char **VertMapNames = NULL;
int MapType, Ct;

WidgetCBClear(IDC_VERTEXMAPDROP);

if (ActiveRoot && Active)
	{
	MapType = Active->CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_UVW ? 0: 1;
	if (VertMapNames = Active->GetVertexMapNames(MapType))
		{
		for (Ct = 0; VertMapNames[Ct]; Ct ++)
			{
			WidgetCBAddEnd(IDC_VERTEXMAPDROP, VertMapNames[Ct]);
			} // for
		AppMem_Free(VertMapNames, (Ct + 1) * sizeof (char *));
		} // if
	} // if

} // TextureEditGUI::FillVertexMapCombo

/*===========================================================================*/

void TextureEditGUI::SetVertexMapDrop(void)
{
long ListEntry, ItemCount, Pos = -1;
char VertMapName[128];

if (ActiveRoot && Active)
	{
	ItemCount = WidgetCBGetCount(IDC_VERTEXMAPDROP);
	for (ListEntry = 0; ListEntry < ItemCount; ListEntry ++)
		{
		if (WidgetCBGetText(IDC_VERTEXMAPDROP, ListEntry, VertMapName) && VertMapName[0])
			{
			if (! strcmp(VertMapName, Active->MapName))
				Pos = ListEntry;
			} // if
		} // for
	} // if

WidgetCBSetCurSel(IDC_VERTEXMAPDROP, Pos);

} // TextureEditGUI::SetVertexMapDrop

/*===========================================================================*/

void TextureEditGUI::SelectVertexMap(void)
{
long Current;
char VertMapName[128];

Current = WidgetCBGetCurSel(IDC_VERTEXMAPDROP);
if (Current >= 0)
	{
	if (WidgetCBGetText(IDC_VERTEXMAPDROP, Current, VertMapName) && VertMapName[0])
		{
		Active->SetVertexMap(VertMapName);
		} // if
	} // if

} // TextureEditGUI::SelectVertexMap

/*===========================================================================*/

// Material GUI
void TextureEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureColors();
} // TextureEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void TextureEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveGrad(NewNode);
} // TextureEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void TextureEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, TexMatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(TexMatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT, true);
	//WidgetShow(IDC_ANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // TextureEditGUI::ShowMaterialPopDrop

/*===========================================================================*/

