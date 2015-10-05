// CommonComponentEditor.cpp
// Common code for Component Editors
// Created from existing code by CXH 11/16/07

#include "stdafx.h"
#include "CommonComponentEditor.h"
#include "Requester.h"
#include "EffectsLib.h"
#include "resource.h"
#include "UsefulPathString.h"

bool CommonComponentEditor::RespondToInquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_IS_A_COMPONENT_EDITOR:
	case WCS_FENETRE_WINCAP_CANNEXT:
	case WCS_FENETRE_WINCAP_CANPREV:
	case WCS_FENETRE_WINCAP_CANUNDO:
	case WCS_FENETRE_WINCAP_CANLOAD:
	case WCS_FENETRE_WINCAP_CANSAVE:
		return(true);
	case WCS_FENETRE_WINCAP_CANEMBED:
		{
		GeneralEffect *RealStoredActiveComponent;
		// determine if we can offer embed or not
		if (StoredActiveComponent)
			{
			RealStoredActiveComponent = *StoredActiveComponent;
			if (RealStoredActiveComponent && RealStoredActiveComponent->TemplateItem)
				{
				return(true);
				} // if
			} // if
		return(false);
		} // embed
	case WCS_FENETRE_WINCAP_CANSHOWADV:
		return(RespondToCanShowAdvancedFeatures());
	default:
		return(false);
	} // AskAbout
//lint -restore

} // CommonComponentEditor::RespondToInquireWindowCapabilities

/*===========================================================================*/

long CommonComponentEditor::HandleCommonEvent(int EventCode, EffectsLib *Lib, GeneralEffect *Foo, Database *DBHost)
{
GeneralEffect *RealStoredActiveComponent;
long rVal = 1;

// determine if we can offer embed or not
if (StoredActiveComponent)
	{
	RealStoredActiveComponent = *StoredActiveComponent;
	} // if

switch (EventCode)
	{
	case IDC_PREV:
		{
		//Lib->EditNext(0, RealStoredActiveComponent, RealStoredActiveComponent->EffectType);
		ULONG NotifyChanges[2] = {MAKE_ID(WCS_NOTIFYCLASS_DELAYEDEDIT, WCS_NOTIFYSUBCLASS_REVERSE, 0, 0), 0};
		GlobalApp->AppEx->GenerateDelayedNotify(NotifyChanges, Foo);
		break;
		} //
	case IDC_NEXT:
		{
		//Lib->EditNext(1, RealStoredActiveComponent, RealStoredActiveComponent->EffectType);
		ULONG NotifyChanges[2] = {MAKE_ID(WCS_NOTIFYCLASS_DELAYEDEDIT, 0, 0, 0), 0};
		GlobalApp->AppEx->GenerateDelayedNotify(NotifyChanges, Foo);
		break;
		} //
	case IDC_GALLERY:
		{
		RealStoredActiveComponent->OpenGallery(Lib);
		break;
		} //
	case IDC_LOAD:
		{
		RealStoredActiveComponent->LoadComponentFile(NULL);
		break;
		} //
	case IDC_SAVE:
		{
		RealStoredActiveComponent->OpenBrowseData(Lib);
		break;
		} //
	case IDC_EMBED:
		{
		if (UserMessageOKCAN(RealStoredActiveComponent->GetName(), "Are you sure you wish to embed this component into the current Project?"))
			RealStoredActiveComponent->Embed();
		if (StoredFenetre)
			StoredFenetre->UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_EMBED);
		break;
		} //
	case IDC_HARDLINK:
		{
		RealStoredActiveComponent->HardLinkVectors(DBHost);
		break;
		} // IDC_HARDLINK
	default:
		rVal = 0;
		break;
	} // EventCode

return(rVal);

} // CommonComponentEditor::HandleCommonEvent
