// AuthorizeGUI.cpp
// Code for Authorize window
// Built from VersionGUI.cpp on Oct 9 2003 by CHX
// Copyright 1996

#include "stdafx.h"
#include "AppMem.h"
#include "AuthorizeGUI.h" // will become StartupWin.h
#include "WCSVersion.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Conservatory.h"
#include "resource.h"

#define WCS_AUTHGUI_MAX_STEPS		34 // max number of authorization query steps

NativeGUIWin AuthorizeGUI::Open(Project *Moi)
{
NativeGUIWin Success;
if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // AuthorizeGUI::Open

/*===========================================================================*/

NativeGUIWin AuthorizeGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_SX_AUTH, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // AuthorizeGUI::Construct

/*===========================================================================*/

AuthorizeGUI::AuthorizeGUI()
: GUIFenetre('AUTH', this, "Authorization")
{

SerialStr[0] = KeyID[0] = WCSAuth[0] = VNSAuth[0] = SXAuth[0] = SXFormats[0] = NULL;

} // AuthorizeGUI::AuthorizeGUI

/*===========================================================================*/

AuthorizeGUI::~AuthorizeGUI()
{
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // AuthorizeGUI::~AuthorizeGUI()

/*===========================================================================*/

long AuthorizeGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_AUT, 0);

return(0);

} // AuthorizeGUI::HandleCloseWin

/*===========================================================================*/

long AuthorizeGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
/*
	case ID_OK:
	case ID_KEEP:
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_AUT, 0);
		break;
		} // 
*/
	case IDC_AUTHORIZE:
		{
		HandleNewAuth();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // AuthorizeGUI::HandleButtonClick

/*===========================================================================*/

long AuthorizeGUI::HandleEvent(void)
{

return(0);

} // AuthorizeGUI::HandleEvent

/*===========================================================================*/

void AuthorizeGUI::ConfigureWidgets(void)
{
BusyWin *AUTH = NULL;
int AuthCheckSteps = 0;

#ifndef WCS_BUILD_DEMO
SXAuth[0] = NULL;
if(GlobalApp->Sentinal)
	{

	if(!AUTH)
		{
		AUTH = new BusyWin("Querying Authorization Key", WCS_AUTHGUI_MAX_STEPS, 'BUSY', 0); // update WCS_AUTHGUI_MAX_STEPS at top of file when you add query steps
		} // if

	if(GlobalApp->Sentinal->CheckSerial())
		{
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		sprintf(SerialStr, "0x%08x", GlobalApp->Sentinal->CheckSerial());
		} // if
	if(GlobalApp->Sentinal->CheckKeySerial())
		{
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		sprintf(KeyID, "%d", GlobalApp->Sentinal->CheckKeySerial());
		} // if
	// Check WCS field
	if(GlobalApp->Sentinal->ReadAuthField(WCS_SECURITY_AUTH_FIELD_A)) // is there something in the WCS field?
		{
		int WCSSE = 0, WCSV5 = 0, WCSV6 = 0, WCSV7 = 0, WCSV8 = 0, WCSV9 = 0, WCSV10 = 0;
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthSE())
			{
			WCSSE = 1;
			} // if

		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(10)))
			{
			WCSV10 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(9)))
			{
			WCSV9 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(8)))
			{
			WCSV8 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(7)))
			{
			WCSV7 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(6)))
			{
			WCSV6 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"S", WCS_SECURITY_AUTH_FIELD_A, GlobalApp->Sentinal->CalcWCSVerNum(5)))
			{
			WCSV5 = 1;
			} // if
		sprintf(WCSAuth, "V1 - V4%s%s%s%s%s%s%s",
		 (WCSV5 ? ", V5" : ""),  (WCSV6 ? ", V6" : ""),  (WCSV7 ? ", V7" : ""),  (WCSV8 ? ", V8" : ""),  (WCSV9 ? ", V9" : ""),  (WCSV10 ? ", V10" : ""),
		 (WCSSE ? " SE" : ""));
		} // if
	// VNS
	if(AUTH) AUTH->Update(AuthCheckSteps++);
	if(GlobalApp->Sentinal->ReadAuthField(WCS_SECURITY_AUTH_FIELD_B)) // is there something in the VNS field?
		{
		int VNSEDSS = 0;
		int VNSFW = 0;
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthField('E', NULL, WCS_SECURITY_AUTH_FIELD_B))
			{
			VNSEDSS = 1;
			} // if
		if(GlobalApp->Sentinal->CheckAuthField('W', NULL, WCS_SECURITY_AUTH_FIELD_B))
			{
			VNSFW = 1;
			} // if

		int VNSV1 = 0, VNSV2 = 0, VNSV3 = 0, VNSV4 = 0, VNSV5 = 0, VNSV6 = 0;
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(6)))
			{
			VNSV6 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(5)))
			{
			VNSV5 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(4)))
			{
			VNSV4 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(3)))
			{
			VNSV3 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(2)))
			{
			VNSV2 = 1;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckAuthFieldVersion(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B, GlobalApp->Sentinal->CalcVNSVerNum(1)))
			{
			VNSV1 = 1;
			} // if
		sprintf(VNSAuth, "%s%s%s%s%s%s%s%s",
		 (VNSV1 ? "V1" : ""),  (VNSV2 ? ", V2" : ""),  (VNSV3 ? ", V3" : ""),  (VNSV4 ? ", V4" : ""),  (VNSV5 ? ", V5" : ""),  (VNSV6 ? ", V6" : ""),
		 (VNSEDSS ? " EDSS" : ""), (VNSFW ? " FORESTRY" : "")); // the comma formatting may get weird after VNSV4 and need some twiddling
		} // if
	// RTX
	if(AUTH) AUTH->Update(AuthCheckSteps++);
#ifdef WCS_BUILD_RTX
	if(GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0)
		{
		sprintf(SXAuth, "V1 ");
		#ifdef WCS_BUILD_SX2
		if(GlobalApp->Sentinal->CheckAuthFieldSX2() ? 1: 0)
			{
			strcat(SXAuth, "V2 ");
			} // if
		#endif // WCS_BUILD_SX2

		SXFormats[0] = NULL;
		int SXFirst = 1;

		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_NVE))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "NatureView Express");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VRML_WEB))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "Web VRML");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_3DS))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "3D Studio");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_LW))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "Lightwave");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VTP))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "Virtual Terrain Project");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_STL))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "STL Stereolithography");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VRMLSTL))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "VRML for STL");
			SXFirst = 0;
			} // if

		// Hide VNS-only formats like OpenFlight and TerraPage
		#ifdef WCS_BUILD_VNS
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_OPENFLIGHT))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "OpenFlight");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_TERRAPAGE))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "TerraPage");
			SXFirst = 0;
			} // if
		#ifdef WCS_BUILD_SX2
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_GIS))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "GIS");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_WCSVNS))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "VNS");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_COLLADA))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "COLLADA");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_FBX))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "FBX");
			SXFirst = 0;
			} // if
		#endif // WCS_BUILD_SX2
		#endif // WCS_BUILD_VNS

		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_MAYA))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "Maya");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_SOFTIMAGE))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "SoftImage");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_KML))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "Google Earth");
			SXFirst = 0;
			} // if
		if(AUTH) AUTH->Update(AuthCheckSteps++);
		if(GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_WW))
			{
			if(!SXFirst) {strcat(SXFormats, ", ");}
			strcat(SXFormats, "WorldWind");
			SXFirst = 0;
			} // if
		} // if
#endif // WCS_BUILD_RTX
	} // if

	if(AUTH)
		{
		delete AUTH;
		AUTH = NULL;
		} // if

#endif // !WCS_BUILD_DEMO


WidgetSetText(IDC_SERIAL, SerialStr);
WidgetSetText(IDC_KEYID, KeyID);
WidgetSetText(IDC_WCS, WCSAuth);
WidgetSetText(IDC_VNS, VNSAuth);
WidgetSetText(IDC_SX, SXAuth);
WidgetSetText(IDC_FORMATS, SXFormats);

} // AuthorizeGUI::ConfigureWidgets()

/*===========================================================================*/

long AuthorizeGUI::HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

/*
switch (CtrlID)
	{

	case IDC_EDIT3:
		{
		HandleNewAuth();
		break;
		} // 
	} // switch CtrlID
*/
return (0);

} // AuthorizeGUI::HandleStringEdit

/*===========================================================================*/

long AuthorizeGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
/*
switch (CtrlID)
	{
	case IDC_EDIT3:
		{
		HandleNewAuth();
		break;
		} // 
	} // switch CtrlID
*/
return (0);

} // AuthorizeGUI::HandleStringLoseFocus


void AuthorizeGUI::HandleNewAuth(void)
{

#ifdef WCS_BUILD_DEMO

UserMessageDemo("Authorization codes are ignored in Demo version.");
return;
#else // !WCS_BUILD_DEMO

#ifdef WCS_BUILD_RTX
char NewAuth[201];

WidgetGetText(IDC_EDIT3, 200, NewAuth);
NewAuth[200] = NULL;

if(GlobalApp->Sentinal)
	{
	unsigned long AuthResult;

	if(strlen(NewAuth) < 6) // old short-form upgrade code
		{
		if(GlobalApp->Sentinal->DecodeAuthString(NewAuth) != 0) // non-empty authorization string
			{
			int FieldLoc;
			#ifdef WCS_BUILD_VNS
			FieldLoc = WCS_SECURITY_AUTH_FIELD_B;
			#else // WCS
			FieldLoc = WCS_SECURITY_AUTH_FIELD_A;
			#endif // WCS
			if(GlobalApp->Sentinal->ErrorCheckNewAuthorizationValue((WORD)GlobalApp->Sentinal->DecodeAuthString(NewAuth)))
				{
				GlobalApp->Sentinal->WriteAuthField((WORD)GlobalApp->Sentinal->DecodeAuthString(NewAuth), FieldLoc);
				UserMessageOK("Authorization Updated", "Program will now exit. Please restart " APP_SHORTTITLE ".");
				GlobalApp->SetTerminate(1);
				} // if
			else
				{
				UserMessageOK("Authorization Failed", "This authorization code is not recognized by this version of " APP_SHORTTITLE ".");
				} // else
			} // if
		else
			{
			UserMessageOK("Invalid authorization code", "Please enter a correct authorization code.");
			} // else
		} // if
	else
		{
		AuthResult = GlobalApp->Sentinal->HandleArmoredAuth(NewAuth);
		if(AuthResult == 1)
			{
			UserMessageOK("Authorization", "New Authorization Accepted.\nAuthorization will be effective upon restart.\nProgram will now exit. Please restart.");
			GlobalApp->SetTerminate(); // Bail out!
			} // if
		else if(AuthResult == 0xffffffff)
			{
			UserMessageOK("Authorization", "Authorization code not intended for this key.");
			} // else
		else
			{
			UserMessageOK("Authorization", "Authorization code not recognized.");
			} // else
		} // else
	} // if
#endif // WCS_BUILD_RTX

#endif // !WCS_BUILD_DEMO
} // AuthorizeGUI::HandleNewAuth
