// AppHelp.cpp
// OnlineHelp support.
// Created from Scratch on 1/10/98 by Chris 'Xenon' Hanson

#undef WIN32_LEAN_AND_MEAN // need the Shell API stuff

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "AppHelp.h"
#include "Application.h"
#include "GUI.h"
#include "Project.h"
#include "Requester.h"
#include "Security.h"
#include "WCSVersion.h"
#include "ImageFormatConfig.h"
#include "ImageInputFormat.h"
#include "Raster.h"
#include "FSSupport.h"

// from pdfdde.cpp
// Placed here individually because pdfdde.cpp didn't have a header
bool TellReaderToJumpToNamedDestViaDDE(const char *FullFilePath, const char *NamedDest);

/*===========================================================================*/

int AppHelp::OpenHelpTopic(unsigned long TopicID)
{
char *HelpFileBaseName = NULL;
char *HelpFileTopic = NULL;
int HelpOpenStatus = 0;

SECURITY_INLINE_CHECK(047, 47);

if (TopicID)
	{
	HelpFileTopic = FormatIDforPDFDest(AssociateIDtoFile(TopicID));
	} // if

HelpFileBaseName = WCS_APPHELP_DEFAULTBASE;

HelpOpenStatus = OpenHelpFile(HelpFileBaseName, HelpFileTopic);

return(HelpOpenStatus);
} // AppHelp::OpenHelpTopic

/*===========================================================================*/

int AppHelp::OpenHelpFile(char *HelpFile, char *TopicName)
{
char *HelpFileFullPath = NULL;
int HelpOpenStatus = 0;

if (HelpFileFullPath = AttemptOpenHelpFile(HelpFile, 1))
	{
	HelpOpenStatus = ShellOpenHelpFilePDF(HelpFileFullPath, TopicName);
	} // if
else
	{
	UserMessageOK("Help System", "Could not open help file.");
	} // else

return(HelpOpenStatus);
} // AppHelp::OpenHelpFile

/*===========================================================================*/

static char TempURLFilename[512];
int AppHelp::OpenURLIndirect(char *URL)
{
int URLOpenStatus = 0;
/*
FILE *TmpHTML = NULL;
char *TempName = NULL;

if (TempName = _tempnam(NULL, "redirect"))
	{
	sprintf(TempURLFilename, "%s.html", TempName);
	free(TempName);
	if (TmpHTML = fopen(TempURLFilename, "w"))
		{
		fprintf(TmpHTML, "<html>\n<head>\n");
		fprintf(TmpHTML, "<meta http-equiv=\"refresh\" content=\"0; url=%s\" border=0>\n", URL);
		fprintf(TmpHTML, "</head>\n");
		fprintf(TmpHTML, "<title>Redirect to %s</title>", URL);
		fprintf(TmpHTML, "<body>Redirecting to <a href=\"%s\">%s</a></body>", URL, URL);
		fprintf(TmpHTML, "</body></html>\n");
		fclose(TmpHTML);
		URLOpenStatus = ShellOpenHelpFile(TempURLFilename);
		} // if
	} // if
*/

if ((int)ShellExecute((HWND)GlobalApp->WinSys->GetRoot(), "open", URL, NULL, "\\", SW_SHOWNORMAL) > 32)
	{
	URLOpenStatus = 1;
	} // if

return(URLOpenStatus);
} // AppHelp::OpenURLIndirect

/*===========================================================================*/

int AppHelp::OpenCredits(void)
{
int URLOpenStatus = 0;
int AdditionalCode = 0;
FILE *TmpHTML = NULL;
char *TempName = NULL;

if (TempName = _tempnam(NULL, "credits"))
	{
	sprintf(TempURLFilename, "%s.html", TempName);
	free(TempName);
	if (TmpHTML = fopen(TempURLFilename, "w"))
		{
		fprintf(TmpHTML, "<html>\n<head>\n");
		fprintf(TmpHTML, "</head>\n");
		fprintf(TmpHTML, "<title>%s Credits</title>", APP_TITLE);
		fprintf(TmpHTML, "<body>");

		fprintf(TmpHTML, "<br>\n");
		fprintf(TmpHTML, "<center>\n");
		fprintf(TmpHTML, "<font size=3>\n");
		fprintf(TmpHTML, "Concept, initial implementation, algorithms, renderer,");
		fprintf(TmpHTML, " good ideas, principal programming, Scene Express core, Lightwave and 3DS export, Forestry Wizard:<br>");
		fprintf(TmpHTML, " <b>Gary R. Huber</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Optimization, weird ideas, widgets, security,");
		fprintf(TmpHTML, " Views/OpenGL wrangling, image formats, installers, gridder, VTP export, NatureView, additional programming:<br>");
		fprintf(TmpHTML, " <b>Christopher Eric &quot;Xenon&quot; Hanson</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Import Wizardry, DEM Painter, DEM Merge, RLA/RPF, VRML, OpenFlight, VRML, STL, FBX, projection library, additional programming, <i>MORE</i> optimization:<br>");
		fprintf(TmpHTML, " <b>Frank Weed II</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Documentation:<br>");
		fprintf(TmpHTML, " <b>Adam Hauldren</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Video and HTML Tutorials:<br>");
		fprintf(TmpHTML, " <b>Scott Cherba</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "SuperConductor, and other bonuses:<br>");
		fprintf(TmpHTML, " <b>David Kopp</b>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Thanks to Frank Warmerdam, Peter Guth, Paul Martz and many others for sharing");
		fprintf(TmpHTML, " their specialized expertise with us.<br>");
		fprintf(TmpHTML, "<br><br>\n");
		fprintf(TmpHTML, "Tremendous thanks to our Factory Test Pilots for");
		fprintf(TmpHTML, " putting their sanity on the line in the name of testing.<br>");
		fprintf(TmpHTML, "<br><br>\n");


#ifdef WCS_BUILD_JPEG_SUPPORT
		AdditionalCode = 1;
#endif // WCS_BUILD_JPEG_SUPPORT
#ifdef WCS_BUILD_TIFF_SUPPORT
		AdditionalCode = 1;
#endif // WCS_BUILD_TIFF_SUPPORT
#ifdef WCS_BUILD_PNG_SUPPORT
		AdditionalCode = 1;
#endif // WCS_BUILD_PNG_SUPPORT

		if (AdditionalCode)
			{
			fprintf(TmpHTML, " "APP_SHORTTITLE" includes code from the following entities:<br><br>");
			} // if

#ifdef WCS_BUILD_JPEG_SUPPORT
		fprintf(TmpHTML, "Independent JPEG Group<br><br>");
#endif // WCS_BUILD_JPEG_SUPPORT
#ifdef WCS_BUILD_TIFF_SUPPORT
		fprintf(TmpHTML, "LIBTIFF: Copyright (c) 1988-1997 Sam Leffler<br>");
		fprintf(TmpHTML, "LIBTIFF: Copyright (c) 1991-1997 Silicon Graphics, Inc.<br><br>");
#endif // WCS_BUILD_TIFF_SUPPORT
#ifdef WCS_BUILD_GEOTIFF_SUPPORT
		fprintf(TmpHTML, "GEOTIFF Group<br><br>");
#endif // WCS_BUILD_GEOTIFF_SUPPORT
#ifdef WCS_BUILD_PNG_SUPPORT
		fprintf(TmpHTML, "PNG Development Group<br><br>");
#endif // WCS_BUILD_PNG_SUPPORT
#ifdef WCS_BUILD_PNG_SUPPORT
		fprintf(TmpHTML, "Jean-loup Gailly and Mark Adler (ZLIB)<br><br>");
#endif // WCS_BUILD_PNG_SUPPORT

#ifdef WCS_BUILD_FBX
// FBX crap
		fprintf(TmpHTML, "This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2008 Autodesk, Inc.  All rights, reserved. Such code is provided \"as is\" and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights.  In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code.");
		fprintf(TmpHTML, "<br><br>\n");
#endif // WCS_BUILD_FBX

		if (AdditionalCode)
			{
		fprintf(TmpHTML, "<br>\n");
			} // if


		fprintf(TmpHTML, " "APP_TITLE " " APP_VERS" is copyright " APP_COPYRIGHT_BASEDATE);
		fprintf(TmpHTML, " by " APP_PUBLISHER ". All rights reserved.<br><br>");

		fprintf(TmpHTML, "<br>\n");
		fprintf(TmpHTML, " " APP_TITLE " " APP_VERS " documentation is copyright");
		fprintf(TmpHTML, " " APP_COPYRIGHT_BASEDATE " by " APP_PUBLISHER ". All rights reserved.<br><br>");
		fprintf(TmpHTML, "<br>\n");


		fprintf(TmpHTML, "<br>\n");
		fprintf(TmpHTML, " Configuration information:<br>");
#ifdef WCS_BUILD_ECW_SUPPORT
		if (GlobalApp->ECWSupport)
			{
		fprintf(TmpHTML, " ECW DLLs available.<br>");
			} // if
#endif // WCS_BUILD_ECW_SUPPORT
		fprintf(TmpHTML, "<br>\n");



		fprintf(TmpHTML, "</font>\n");
		fprintf(TmpHTML, "</center>\n");
		fprintf(TmpHTML, "</body></html>\n");
		fclose(TmpHTML);
		URLOpenStatus = ShellOpenHelpFile(TempURLFilename);
		} // if
	} // if

return(URLOpenStatus);

} // AppHelp::OpenCredits

/*===========================================================================*/

static char UpdateURL[5000], UpdateVersStr[50];
int AppHelp::DoOnlineUpdate(void)
{

#ifdef WCS_BUILD_DEMO
return(0);
#else // !WCS_BUILD_DEMO
time_t NowTime;
char *UpdateBetaStr = "";
int Vers = 0, Platform, TestPilot = 0, Result;
unsigned long AuthChal, Auth = 0, SN = 0, ID = 0;

(void)time(&NowTime);

AuthChal = (unsigned long)(time_t)(NowTime % 100);

GlobalApp->MainProj->Prefs.LastUpdateDate = (unsigned long int)NowTime; // <<<>>> TIME64 issue, should use time_t for future-proofing
Vers = APP_VERS_NUM;
if (GlobalApp->Sentinal)
	{
	SN   = GlobalApp->Sentinal->CheckSerial();
	ID   = GlobalApp->Sentinal->CheckKeySerial();
	Auth = GlobalApp->Sentinal->CheckDongle(AuthChal);
	} // if

#ifdef _M_IX86 // intel
Platform = 1;
#endif // _M_IX86

(void)EscapeURI(ExtVersion, UpdateVersStr, 50);

if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("testpilot"))
	{
	Result = UserMessageYNCAN("Test Pilot Online Update", "Would you like to download from the TestPilot web page?");
	if (Result == 2) // yes
		{
		TestPilot = 1;
		} // if
	if (Result == 0) // cancel
		{
		return(0);
		} // if
	} // if

if (TestPilot)
	{
	UpdateBetaStr = "Beta=Beta&what=prog&";
	} // if

sprintf(UpdateURL, "http://WWW.3DNature.COM/update.php?%sprod=%s&plat=%d&vers=%d&sn=%08x&id=%d&auth=%d&cur=%s",
 UpdateBetaStr, APP_TLA, Platform, Vers, SN, ID, Auth, UpdateVersStr);
return(OpenURLIndirect(UpdateURL));

#endif // !WCS_BUILD_DEMO
} // AppHelp::DoOnlineUpdate

/*===========================================================================*/

char TempStr[200];
static char HardwareStr[200], SysNameStr[200], VideoStr[200], OSStr[200], UpdateEmail[200], UpdateFN[200], UpdateLN[200];
int AppHelp::DoOnlineReport(void)
{
int SplitLoop;
unsigned long SN = 0;

#ifndef WCS_BUILD_DEMO
if (GlobalApp->Sentinal)
	{
	SN   = GlobalApp->Sentinal->CheckSerial();
	} // if
#endif // !WCS_BUILD_DEMO

// Hardware
(void)GlobalApp->InquireCPUDescString(HardwareStr, 200);
(void)GlobalApp->InquireSystemNameString(SysNameStr, 200);
sprintf(TempStr, "%s (%s)", HardwareStr, SysNameStr);
(void)EscapeURI(TempStr, HardwareStr, 200);

// OS
(void)GlobalApp->InquireOSDescString(TempStr, 200);
(void)EscapeURI(TempStr, OSStr, 200);

// Video
(void)GlobalApp->InquireDisplayResString(VideoStr, 200);
sprintf(TempStr, "%s %s %s %s", VideoStr, GlobalApp->InquireGLVendorString(), GlobalApp->InquireGLRenderString(), 
 GlobalApp->InquireGLVersionString());
(void)EscapeURI(TempStr, VideoStr, 200);

// WCS Version
(void)EscapeURI(ExtVersion, UpdateVersStr, 50);

// User Name (use Email buffer temporarily for splitting)
UpdateFN[0] = UpdateLN[0] = 0;
strcpy(UpdateEmail, GlobalApp->MainProj->UserName);
for (SplitLoop = 0; UpdateEmail[SplitLoop]; SplitLoop++)
	{
	if (isspace(UpdateEmail[SplitLoop]))
		{
		UpdateEmail[SplitLoop] = 0;
		strncpy(UpdateLN, &UpdateEmail[SplitLoop + 1], 48);
		UpdateLN[49] = NULL;
		} // if
	} // for
strncpy(UpdateFN, UpdateEmail, 48);
UpdateFN[49] = NULL;

// User Email
(void)EscapeURI(GlobalApp->MainProj->UserEmail, UpdateEmail, 50);

sprintf(UpdateURL, "http://WWW.3DNature.COM/testpilot/report.php?Hardware=%s&OS=%s&Video=%s&Serial=%08x&FirstName=%s&LastName=%s&Email=%s&Versions=%s",
 HardwareStr, OSStr, VideoStr, SN, UpdateFN, UpdateLN, UpdateEmail, UpdateVersStr);
return(OpenURLIndirect(UpdateURL));
} // AppHelp::DoOnlineReport

/*===========================================================================*/

int AppHelp::DoOnlineRegister(void)
{
unsigned long SN = 0, SplitLoop;
#ifndef WCS_BUILD_DEMO
if (GlobalApp->Sentinal)
	{
	SN   = GlobalApp->Sentinal->CheckSerial();
	} // if
#endif // !WCS_BUILD_DEMO

// Hardware
(void)GlobalApp->InquireCPUDescString(HardwareStr, 200);
(void)GlobalApp->InquireOSDescString(OSStr, 200);
sprintf(TempStr, "%s %s", HardwareStr, OSStr);
(void)EscapeURI(TempStr, HardwareStr, 200);

// Video
(void)GlobalApp->InquireDisplayResString(VideoStr, 200);
sprintf(TempStr, "%s %s %s %s", VideoStr, GlobalApp->InquireGLVendorString(), GlobalApp->InquireGLRenderString(), 
 GlobalApp->InquireGLVersionString());
(void)EscapeURI(TempStr, VideoStr, 200);

// WCS Version
(void)EscapeURI(ExtVersion, UpdateVersStr, 50);

// User Name (use Email buffer temporarily for splitting)
UpdateFN[0] = UpdateLN[0] = 0;
strcpy(UpdateEmail, GlobalApp->MainProj->UserName);
for (SplitLoop = 0; UpdateEmail[SplitLoop]; SplitLoop++)
	{
	if (isspace(UpdateEmail[SplitLoop]))
		{
		UpdateEmail[SplitLoop] = 0;
		strncpy(UpdateLN, &UpdateEmail[SplitLoop + 1], 48);
		UpdateLN[49] = NULL;
		} // if
	} // for
strncpy(UpdateFN, UpdateEmail, 48);
UpdateFN[49] = NULL;
(void)EscapeURI(UpdateFN, TempStr, 50);
strcpy(UpdateFN, TempStr);
(void)EscapeURI(UpdateLN, TempStr, 50);
strcpy(UpdateLN, TempStr);


// User Email
(void)EscapeURI(GlobalApp->MainProj->UserEmail, UpdateEmail, 50);

sprintf(UpdateURL, "http://WWW.3DNature.COM/registration.php?Hardware=%s&Video=%s&Serial=%08x&FirstName=%s&LastName=%s&Email=%s&Versions=%s",
 HardwareStr, VideoStr, SN, UpdateFN, UpdateLN, UpdateEmail, UpdateVersStr);
return(OpenURLIndirect(UpdateURL));
} // AppHelp::DoOnlineRegister

/*===========================================================================*/

int AppHelp::DoOnlinePurchase(void)
{
int SplitLoop;
char *Product = "";

// WCS Version
(void)EscapeURI(ExtVersion, UpdateVersStr, 50);

// User Name (use Email buffer temporarily for splitting)
UpdateFN[0] = UpdateLN[0] = 0;
strcpy(UpdateEmail, GlobalApp->MainProj->UserName);
for (SplitLoop = 0; UpdateEmail[SplitLoop]; SplitLoop++)
	{
	if (isspace(UpdateEmail[SplitLoop]))
		{
		UpdateEmail[SplitLoop] = 0;
		strncpy(UpdateLN, &UpdateEmail[SplitLoop + 1], 48);
		UpdateLN[49] = NULL;
		} // if
	} // for
strncpy(UpdateFN, UpdateEmail, 48);
UpdateFN[49] = NULL;
(void)EscapeURI(UpdateFN, TempStr, 50);
strcpy(UpdateFN, TempStr);
(void)EscapeURI(UpdateLN, TempStr, 50);
strcpy(UpdateLN, TempStr);

#ifdef WCS_BUILD_VNS
#ifdef _M_IX86
// intel
Product = "RETAILVNS"APP_VERS"INTEL";
#endif // _M_IX86
#else // !WCS_BUILD_VNS
#ifdef _M_IX86
// intel
Product = "RETAILV"APP_VERS"INTEL";
#endif // _M_IX86
#endif // !WCS_BUILD_VNS

// User Email
(void)EscapeURI(GlobalApp->MainProj->UserEmail, UpdateEmail, 50);

#ifdef WCS_BUILD_VNS
sprintf(UpdateURL, "https://3dnature.com/store/index.php?cPath=2_23");
#else // !WCS_BUILD_VNS
sprintf(UpdateURL, "https://3dnature.com/store/index.php?cPath=2_24");
#endif // !WCS_BUILD_VNS
return(OpenURLIndirect(UpdateURL));
} // AppHelp::DoOnlinePurchase

/*===========================================================================*/

char *AppHelp::AttemptOpenHelpFile(char *HelpBaseName, int AddSuffix)
{
FILE *CheckLocate = NULL;
char *CDRoot = NULL;

(void)GlobalApp->GetProgDir(TempScratch, 254);
if (AddSuffix)
	{
	sprintf(&TempScratch[strlen(TempScratch)], WCS_APPHELP_PATH, HelpBaseName, WCS_APPHELP_PDFSUFFIX);
	} // if
else
	{
	strcat(&TempScratch[strlen(TempScratch)], HelpBaseName);
	} // else

SECURITY_INLINE_CHECK(033, 33);

if (CheckLocate = PROJ_fopen(TempScratch, "rb"))
	{
	} // if
else
	{ // Try the CD
	while (!(CDRoot = FindCDRoot()) && UserMessageOKCAN("Help System", "Please insert your "APP_TLA" DVD-ROM in\nyour DVD-ROM drive and select OK."))
		{
		CDRoot = FindCDRoot();
		} // while
	if (CDRoot)
		{
		strcpy(TempScratch, CDRoot);
		sprintf(&TempScratch[strlen(TempScratch)], WCS_APPHELP_PATH, HelpBaseName, WCS_APPHELP_PDFSUFFIX);
		CheckLocate = PROJ_fopen(TempScratch, "rb");
		} // if
	} // else

if (CheckLocate)
	{
	SECURITY_INLINE_CHECK(022, 22);
	fclose(CheckLocate);
	CheckLocate = NULL;
	return(TempScratch);
	} // if

return(NULL);
} // AppHelp::AttemptOpenHelpFile

/*===========================================================================*/

int AppHelp::ShellOpenHelpFile(char *FullFileName)
{
SECURITY_INLINE_CHECK(053, 53);
#ifdef _WIN32

if ((int)ShellExecute((HWND)GlobalApp->WinSys->GetRoot(), "open", FullFileName, NULL, "\\", SW_SHOWNORMAL) > 32)
	{
	return(1);
	} // if
#endif // _WIN32
return(0);
} // AppHelp::ShellOpenHelpFile

/*===========================================================================*/

int AppHelp::ShellOpenHelpFilePDF(char *FullFileName, char *NamedDestination)
{
SECURITY_INLINE_CHECK(053, 53);
#ifdef _WIN32
char OpenArgs[1024], OpenActionsArgs[1024];

if (NamedDestination)
	{
	sprintf(OpenArgs, "/A \"nameddest=%s\" \"%s\"", NamedDestination, FullFileName);
	sprintf(OpenActionsArgs, "/A \"nameddest=%s=OpenActions\" \"%s\"", NamedDestination, FullFileName);
	} // if
else
	{
	sprintf(OpenArgs, "\"%s\"", FullFileName);
	sprintf(OpenActionsArgs, "\"%s\"", FullFileName);
	} // else

if (TellReaderToJumpToNamedDestViaDDE(FullFileName, NamedDestination))
	{
	return(1);
	} // if
else if ((int)ShellExecute((HWND)GlobalApp->WinSys->GetRoot(), "open", "acrord32.exe", OpenArgs, "", SW_SHOWNORMAL) > 32)
	{
	return(1);
	} // if
else if ((int)ShellExecute((HWND)GlobalApp->WinSys->GetRoot(), "open", "acrobat.exe", OpenArgs, "", SW_SHOWNORMAL) > 32)
	{
	return(1);
	} // if
else if ((int)ShellExecute((HWND)GlobalApp->WinSys->GetRoot(), "open", FullFileName, OpenArgs, "", SW_SHOWNORMAL) > 32)
	{
	return(1);
	} // if
#endif // _WIN32
return(0);
} // AppHelp::ShellOpenHelpFilePDF

/*===========================================================================*/

char *AppHelp::AssociateIDtoFile(unsigned long TopicID)
{

// Matches 'TML*':
if ((TopicID ^ 'TML\0') == 0) return("Timeline");

SECURITY_INLINE_CHECK(008, 8);

#ifdef WCS_BUILD_VNS
switch(TopicID)
	{
	//case '': return("");
	// special cases
	case 'MENU': return("Menus"); // menus
	case 'TOOL': return("Toolbar"); // toolbar
	case 'TIPS': return("Tips"); // tips
	case 'ZIPS': return("CompInfo"); // compression

	// S@G
	case 'SAAG': return("SceneAtAGlance");
	
	// Views
	case 'VEWT': 
	case 'VW00': 
	case 'VW01': 
	case 'VW02': 
	case 'VW03': 
	case 'VW04': 
	case 'VW05': 
	case 'VW06': 
	case 'VW07': 
	case 'VW08': 
	case 'VW09': return("Views");

	case 'MATE': return("3D_Material_Editor"); // material
	case '3DOE': return("3D_Object_Editor"); // 3d object
	case 'ATEF': return("ATFX_Ed"); // area tfx
	case 'ATMG': return("Atmos_Ed"); // atmosphere
	case 'CAMP': return("Cam_Ed"); // camera
	case 'CELP': return("Cel_Obj_Ed"); // celestial
	case 'CLOD': return("Cloud_Ed"); // cloud
	case 'CMAP': return("Color_Map_Ed"); // colormap
	case 'DEME': return("DEM_Ed"); // DEM Editor
	case 'ECEF':
		{
		#ifdef WCS_FORESTRY_WIZARD
		if (GlobalApp->ForestryAuthorized)
			return("Ecosys_Ed_FE"); // ecosys editor
		else
		#endif // WCS_FORESTRY_WIZARD
			return("Ecosys_Ed"); // ecosys editor
		} // ECEF
	case 'ENVI': return("Env_Ed"); // environment
	case 'EDFO':
		{
		#ifdef WCS_FORESTRY_WIZARD
		if (GlobalApp->ForestryAuthorized)
			return("Foliage_Effect_FE"); // foliage effect <<<>>>
		else
		#endif // WCS_FORESTRY_WIZARD
			return("Foliage_Effect_Ed"); // foliage effect
		} // EDFO
	case 'GRND': return("Ground_Ed"); // ground effect
	case 'LKEF':
		{
		#ifdef WCS_FORESTRY_WIZARD
		if (GlobalApp->ForestryAuthorized)
			return("Lake_Ed_FE"); // ecosys editor
		else
		#endif // WCS_FORESTRY_WIZARD
			return("Lake_Ed"); // lake
		} // LKEF
	case 'SUNP': return("Light_Pos_Time"); // sun pos by time
	case 'EPTS': return("Light_Ed"); // sun pos by time goes to light editor
	case 'MATS': return("Strata_Ed"); // Material Strata Editor
	case 'PLAN': return("Planet_Opts_Ed"); // Planet Options Editor
	case 'PPRC': return("PP_Ed"); // postprocess editor
	case 'RNDJ': return("Render_Job_Ed"); // Render Job Editor
	case 'RNDO': return("Render_Opts_Ed"); // Render Options Editor
	case 'SHEF': return("Shadow_Ed"); // Shadow Editor
	case 'SKYP': return("Sky_Ed"); // Sky Editor
	case 'SNOW': return("Snow_Ed"); // Snow Effect Editor
	case 'STAR': return("Star_Ed"); // Starfield Editor
	case 'STRM':
		{
		#ifdef WCS_FORESTRY_WIZARD
		if (GlobalApp->ForestryAuthorized)
			return("Stream_Ed_FE"); // ecosys editor
		else
		#endif // WCS_FORESTRY_WIZARD
			return("Stream_Ed"); // Stream Editor
		} // STRM
	case 'TERF': return("TFX_Ed"); // Terraffector Editor
	case 'TGEN': return("Terrain_Gen"); // new terrain generator
	case 'TGRD': return("Terrain_Grid"); // Terrain Gridder Editor
	case 'TERP': return("Terrain_Param_Ed"); // Terrain Parameter Editor
	case 'FNCE': return("Wall_Ed"); // fence/wall editor
	case 'WVED': return("Wave_Ed"); // Wave Model Editor
	//case '': return("_50");  // "other windows"
	case 'ANEN': return("Amp_Env_Ed"); // "Amplitude Envelope Editor" aka AnimGraphGUI
	//case 'TRVW': return("_52"); // Animation Track View
	case 'AUTH': return("Auth_Window"); // Authorization Window
	case 'EDCO': return("Color_Ed"); // Color Editor
	case 'LOUV': return("Comp_Gall"); // Component Gallery
	case 'EFFL': return("Comp_Lib"); // Component Library
	case 'BSIN': return("Comp_Sig"); // Component Signature Window
	case 'DIGO': return("Create_Palette"); // Create Palette Window
	case 'CRDT': return("Credits"); // Credits Window
	case 'ANCS': return("X-Sect_Ed"); // "Cross Section Profile Editor"
	case 'DBED': return("DB_Ed"); // Database Editor
	case 'WEDT': return("DEM_Interp"); // DEM Interpolator
	case 'DEMP': return("DEM_Paint"); // DEM Paint
	case 'DIAG': return("Diag_Data"); // Diagnostic Data Window
	case 'ANEF': return("Edge_Profile_Ed"); // "Edge Feathering Profile Editor
	case 'ANCP': return("Coverage_Profile_Ed"); // "Coverage Profile Editor
	case 'ANCC': return("Custom_Curve_Ed"); // "Custom Curve Editor
	case 'ANDP': return("Density_Profile_Ed"); // "Density Profile Editor
	case 'ANSP': return("Shading_Profile_Ed"); // "Shading Profile Editor
	case 'DBOU': return("Export_DB"); // export database
	case 'IMGL': return("IOL"); // Image Object Library
	case 'IMWZ': return("Import_Wiz"); // Import Wizard
	case 'NUME': return("Num_Input"); // Numeric Input Window
	case 'PRON': return("New_Proj"); // New Project Window
	case 'PREF': return("Prefs"); // Preferences Window
	//case 'PRJU': return("Proj_Update"); // Project Update Wizard
	case 'VRTP': return("RT_Foliage_Prefs"); // RT Foliage
	case 'RCTL': return("Render_Control"); // Render Control Window
	case 'KDSG': return("Scale_Del_Key"); // Scale/Delete Key Frames Window
	case 'PTHT': return("Path_Vec_Trans"); // path vector transfer
	case 'VESC': return("Scale_Vec_Elev"); // Scale Vector Elevations Window
	case 'LWMO': return("Scene_Export"); // Scene Export Window
	case 'SCIM': return("Scene_Import"); // Scene Import Window
	case 'STLG': return("Status_Log"); // Status Log
	case 'TEXG': return("Texture_Ed"); // Texture Editor
	case 'ANGR': return("Timeline"); // timeline
	case 'VTEG': return("Vector_Ed"); // Vector Editor
	case 'VPEF': return("Vector_Profile_Ed"); // Vector Profile Editor
	case 'VERS': return("Version"); // Version Window
	case 'VPRF': return("View_Prefs"); // View Preferences Window
	case 'DB3L': return("SHP_Attrib"); // DB3Layers goes to IMWiz
	case 'IMVG': return("Image_Viewer"); // Image Viewer goes to Image Object Library
	case 'GWIZ': return("Terrain_Grid_Wiz")		; // Gridder Wizard <<<>>>

	// VNS
	case 'COSY': return("CoordSys_Ed"); // CoordSys editor
	case 'SEQU': return("Query_Ed"); // Search Query
	case 'TBLG': return("CoordSys_Ed"); // CoordSys selector goes to CoordSys editor
	case 'TPLM': return("Template_Mgr"); // template manager
	case 'THEM': return("TM_Ed"); // thematic map editor
	case 'DMRG': return("DEM_Mrg"); // DEM Merger
	case 'SCEN': return("Scenario_Ed"); // Render Scenario Editor
	case 'VPEX': return("Vector_Profile_Export"); // Vector Profile Export
	case 'MWIZ': return("DEM_Mrg_Wiz")		; // Merger Wizard <<<>>>

	// EDSS (thought we could do in-page targets, but we can't for now.)
	case 'EDSS': return("edss"); // EDSS generic help
	case 'EDS0': return("edss"); // cam start
	case 'EDS1': return("edss"); // cam mid
	case 'EDS2': return("edss"); // cam end
	case 'EDSL': return("edss"); // Light
	case 'EDSA': return("edss"); // Atmos
	case 'EDSM': return("edss"); // Misc

	#ifdef WCS_BUILD_RTX
	case 'ECTL': return("Export_Control"); // Scene Express Export Control
	case 'RTXP': return("SX_Ed"); // Scene Exporter Editor
	#endif // WCS_BUILD_RTX

	case 'SXFM': return("_101"); // Scene Express Formats (not ever invoked from here, but listed for completeness)
	case 'LABL': return("Label_Ed"); // Label Editor
	case 'FOWZ': return("Forestry_Wiz"); // Forestry Wizard
	case 'DRIL': return("Point_Info"); // Info About Point


	} // switch
#else // !WCS_BUILD_VNS, must be WCS
switch(TopicID)
	{
	//case '': return("");
	// special cases
	case 'MENU': return("_05");
	case 'TOOL': return("_07");
	case 'TIPS': return("_88");
	case 'ZIPS': return("_89");

	// S@G
	case 'SAAG': return("_09");
	
	// Views
	case 'VEWT': return("_07");
	case 'VW00': return("_11");
	case 'VW01': return("_11");
	case 'VW02': return("_11");
	case 'VW03': return("_11");
	case 'VW04': return("_11");
	case 'VW05': return("_11");
	case 'VW06': return("_11");
	case 'VW07': return("_11");
	case 'VW08': return("_11");
	case 'VW09': return("_11");

	case 'MATE': return("_13");
	case '3DOE': return("_14");
	case 'ATEF': return("_15");
	case 'ATMG': return("_16");
	case 'CAMP': return("_17");
	case 'CELP': return("_18");
	case 'CLOD': return("_19");
	case 'CMAP': return("_20");
	case 'DEME': return("_21"); // DEM Editor
	case 'ECEF': return("_22");
	case 'ECTP': return("_23");
	case 'ENVI': return("_24");
	case 'EDFO': return("_25");
	case 'GRND': return("_26");
	case 'LKEF': return("_27");
	case 'SUNP': return("_28");
	case 'EPTS': return("_28"); // sun pos by time goes to light editor
	case 'MATS': return("_29");
	case 'PTHT': return("_30"); // path vector transfer
	case 'PLAN': return("_31");
	case 'PPRC': return("_32"); // postprocess editor
	case 'RNDJ': return("_33");
	case 'RNDO': return("_34");
	case 'SHEF': return("_35");
	case 'SKYP': return("_36");
	case 'SNOW': return("_37");
	case 'STAR': return("_38");
	case 'STRM': return("_39");
	case 'TERF': return("_40");
	case 'TGEN': return("_41"); // new terrain generator
	case 'TGRD': return("_42");
	case 'TERP': return("_43");
	case 'FNCE': return("_44"); // fence/wall editor
	case 'WVED': return("_45");
	//case '': return("_46");  // "other windows"
	case 'ANEN': return("_47");  // "Amplitude Envelope Editor" aka AnimGraphGUI
	//case 'TRVW': return("_48");
	case 'EDCO': return("_49");
	case 'LOUV': return("_50");
	case 'EFFL': return("_51");
	case 'BSIN': return("_52");
	case 'DIGO': return("_53");
	case 'CRDT': return("_54");
	case 'ANCS': return("_55");  // "Cross Section Profile Editor"
	case 'DBED': return("_56");
	case 'WEDT': return("_57"); // DEM Interpolator
	case 'DEMP': return("_58"); // DEM Paint
	case 'DIAG': return("_59");
	case 'ANEF': return("_60");  // "Edge Feathering Profile Editor
	case 'ANCP': return("_61");  // "Coverage Profile Editor
	case 'ANCC': return("_62");  // "Custom Curve Editor
	case 'ANDP': return("_63");  // "Density Profile Editor
	case 'ANSP': return("_64");  // "Shading Profile Editor
	case 'DBOU': return("_65"); // export database
	case 'IMGL': return("_66");
	case 'IMWZ': return("_67");
	case 'NUME': return("_68");
	case 'PRON': return("_69");
	case 'PREF': return("_70");
	//case 'PRJU': return("_71");
	case 'VRTP': return("_72"); // RT Foliage
	case 'RCTL': return("_73");
	case 'KDSG': return("_74");
	case 'VESC': return("_75");
	case 'LWMO': return("_76");
	case 'SCIM': return("_77");
	case 'STLG': return("_78");
	case 'TEXG': return("_79");
	case 'ANGR': return("_80"); // timeline
	case 'VTEG': return("_81");
	case 'VPEF': return("_82");
	case 'VERS': return("_83");
	case 'VPRF': return("_84");
	case 'DB3L': return("_67"); // DB3Layers goes to IMWiz
	case 'IMVG': return("_66"); // Image Viewer goes to Image Object Library
	case 'GWIZ': return(WCS_APPHELP_FILEPREFIX)		; // Gridder Wizard <<<>>>

	// VNS
	case 'COSY': return("_21"); // CoordSys editor
	case 'SEQU': return("_35"); // Search Query
	case 'TBLG': return("_21"); // CoordSys selector goes to CoordSys editor
	case 'TPLM': return("_75"); // template manager
	case 'THEM': return("_45"); // thematic map editor

	// EDSS (thought we could do in-page targets, but we can't for now.)
	case 'EDSS': return("edss"); // EDSS generic help
	case 'EDS0': return("edss"); // cam start
	case 'EDS1': return("edss"); // cam mid
	case 'EDS2': return("edss"); // cam end
	case 'EDSL': return("edss"); // Light
	case 'EDSA': return("edss"); // Atmos
	case 'EDSM': return("edss"); // Misc

	#ifdef WCS_BUILD_RTX
	case 'ECTL': return("_92"); // Scene Express Export Control
	case 'RTXP': return("_91"); // Scene Exporter Editor
	#endif // WCS_BUILD_RTX

	} // switch

#endif // !WCS_BUILD_VNS, must be WCS


return(NULL);
} // AppHelp::AssociateIDtoFile

/*===========================================================================*/

char *AppHelp::FormatIDforPDFDest(const char *NamedDest)
{
static char NamedDestBuf[128];
char *DestScan;

if (NamedDest)
	{
	strcpy(NamedDestBuf, WCS_APPHELP_PDFDESTPREFIX);
	DestScan = &NamedDestBuf[strlen(NamedDestBuf)];
	for (int StringScan = 0; NamedDest[StringScan] != NULL; StringScan++)
		{
		if (NamedDest[StringScan] != '_')
			{
			*DestScan = NamedDest[StringScan];
			DestScan++;
			} // if
		} // for
	*DestScan = NULL;
	return(NamedDestBuf);
	} // if

return(NULL);
} // AppHelp::FormatIDforPDFDest


/*===========================================================================*/
/*===========================================================================*/

HelpFileMetaData::HelpFileMetaData()
{
Title[0] = Brief[0] = IconFileName[0] = Keywords[0] = Level[0] = Set[0] = Product[0] = NULL;
Abstract = NULL;

Priority = 0;

TN = NULL;
ThumbHostRaster = NULL;

} // HelpFileMetaData::HelpFileMetaData

/*===========================================================================*/

HelpFileMetaData::~HelpFileMetaData()
{

delete [] Abstract;
Abstract = NULL;

delete TN;
TN = NULL;

if (ThumbHostRaster)
	{
	ThumbHostRaster->Thumb = NULL; // deleted above
	delete ThumbHostRaster;
	ThumbHostRaster = NULL;
	} // if

Next = NULL;
} // HelpFileMetaData::~HelpFileMetaData

/*===========================================================================*/

void HelpFileMetaData::SetFileName(char *NewPath, char *NewName)
{

PAF.SetPathAndName(NewPath, NewName);

} // HelpFileMetaData::SetFileName

/*===========================================================================*/

int HelpFileMetaData::ReadMetaTags(void)
{
FILE *HelpFile = NULL;
int Success = 0;
char FileName[1000];

if (PAF.GetPathAndName(FileName))
	{
	if (HelpFile = PROJ_fopen(FileName, "r"))
		{
		Success = ReadMetaTags(HelpFile);
		fclose(HelpFile);
		HelpFile = NULL;
		} // if
	} // if

return(Success);
} // HelpFileMetaData::ReadMetaTags

/*===========================================================================*/

int HelpFileMetaData::ReadMetaTags(FILE *HelpFile)
{
int Success = 0;
char MetaInputBuf[2048], MetaInputBufTemp[2048];
char PriStr[20];

if (HelpFile)
	{
	while (fgetline(MetaInputBuf, 2000, HelpFile, 0, 0))
		{
		MetaInputBuf[2000] = NULL;
		char *SkipSpace, *SkipData, *MetaName = NULL, *MetaData = NULL;
		// for case-insensitive comparison against upper-case constants
		strncpyupr(MetaInputBufTemp, MetaInputBuf, 2000);
		if (strstr(MetaInputBufTemp, "</HEAD>"))
			{
			break; // stop scanning once we're out of </head>
			} // if
		SkipSpace = &MetaInputBuf[0];
		if (isspace(SkipSpace[0]))
			{
			SkipSpace = SkipPastNextSpace(SkipSpace);
			} // if
		if (SkipSpace)
			{
			if (!strnicmp("<META ", SkipSpace, 5))
				{
				SkipData = &SkipSpace[6];
				if (!strnicmp("NAME=\"", SkipData, 6))
					{
					MetaName = SkipSpace = &SkipData[6]; // should be after open-quote
					while ((*SkipSpace != '\"') && (*SkipSpace != NULL))
						{
						SkipSpace++;
						} // while
					if (SkipSpace[0] == '\"')
						{
						SkipSpace[0] = NULL; // NULL-terminate Meta Name string
						SkipSpace++;
						} // if
					if (SkipSpace[0]) // more to come?
						{
						if (!strnicmp(" CONTENT=\"", SkipSpace, 10))
							{ // content available?
							MetaData = SkipSpace = &SkipSpace[10]; // should be after open-quote
							while ((*SkipSpace != '\"') && (*SkipSpace != NULL))
								{
								SkipSpace++;
								} // while
							if (SkipSpace[0] == '\"')
								{
								SkipSpace[0] = NULL; // NULL-terminate Meta Content string
								SkipSpace++;
								} // if
							} // if
						} // if
					} // if
				} // if
			} // if
		if (MetaName && MetaData) // did we get both?
			{
			if (!stricmp(MetaName, "title"))
				{
				strncpy(Title, MetaData, 49);
				Title[49] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "product"))
				{
				strncpy(Product, MetaData, 49);
				Product[49] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "abstract_short"))
				{
				strncpy(Brief, MetaData, 99);
				Brief[99] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "abstract_long"))
				{
				if (Abstract = new char[strlen(MetaData) + 1])
					{
					strcpy(Abstract, MetaData);
					Success++;
					} // if
				} // if
			else if (!stricmp(MetaName, "thumb"))
				{
				strncpy(IconFileName, MetaData, 99);
				IconFileName[99] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "level"))
				{
				strncpy(Level, MetaData, 29);
				Level[29] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "keywords"))
				{
				strncpy(Keywords, MetaData, 299);
				Keywords[299] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "set"))
				{
				strncpy(Set, MetaData, 99);
				Set[99] = NULL;
				Success++;
				} // if
			else if (!stricmp(MetaName, "priority"))
				{
				strncpy(PriStr, MetaData, 19);
				PriStr[19] = NULL;
				Priority = atoi(PriStr);
				Success++;
				} // if
			} // if
		} // while
	} // if

return(Success);
} // HelpFileMetaData::ReadMetaTags

/*===========================================================================*/

Thumbnail *HelpFileMetaData::AttemptLoadThumb(void)
{
Raster ThumbLoadRas;
FILE *ThumbFile;
char TempFileName[1024];
char RelativePathResolve[1000];

if (IconFileName[0] && PAF.GetPath()[0])
	{
	strcpy(RelativePathResolve, PAF.GetPath());
	strmfp(TempFileName, RelativePathResolve, IconFileName);
	if (ThumbFile = PROJ_fopen(TempFileName, "rb"))
		{
		fclose(ThumbFile);
		ThumbFile = NULL;
		if (LoadRasterImage(TempFileName, &ThumbLoadRas, 1))
			{
			// create more long-lived empty raster to hold thumbnail
			if (ThumbHostRaster = new Raster)
				{
				// loaded image gets discarded when ThumbLoadRas goes out of scope when AttemptLoadThumb returns
				ThumbHostRaster->Thumb = TN = ThumbLoadRas.CreateThumbnail();
				} // if
			} // if
		} // if
	} // if

return(TN);
} // HelpFileMetaData::AttemptLoadThumb

/*===========================================================================*/

int HelpFileMetaData::Launch(void)
{
int Success = 0;
char HelpFileFullPath[1024];
char *HelpFileFoundPath = NULL;

PAF.GetPathAndName(HelpFileFullPath);
if (HelpFileFoundPath = GlobalApp->HelpSys->AttemptOpenHelpFile(HelpFileFullPath, 0))
	{
	Success = GlobalApp->HelpSys->ShellOpenHelpFile(HelpFileFoundPath);
	} // if

return(Success);
} // HelpFileMetaData::Launch

/*===========================================================================*/
/*===========================================================================*/

HelpFileIndex::HelpFileIndex()
{
NumHelpFiles = 0;
MDChain = NULL;
} // HelpFileIndex::HelpFileIndex

/*===========================================================================*/

HelpFileIndex::~HelpFileIndex()
{
DiscardFileIndex();
} // HelpFileIndex::~HelpFileIndex

/*===========================================================================*/

int HelpFileIndex::BuildFileIndex(char *BaseDir)
{
int NumFilesLoaded = 0;
HANDLE IHand = NULL;
WIN32_FIND_DATA FileData;
char INameStr[256];
HelpFileMetaData *HFMD = NULL;

DiscardFileIndex();

if (BaseDir)
	{
	strmfp(INameStr, BaseDir, "*");
	do
		{
		if (!IHand) // first iteration
			{
			IHand = FindFirstFile(INameStr, &FileData);
			} // if
		else // subsequent iterations
			{
			if (!(FindNextFile(IHand, &FileData))) break;
			} // else

		if ((IHand != NULL) && (IHand != INVALID_HANDLE_VALUE))
			{
			if (FileData.cFileName[0] != '.')
				{
				if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
					strmfp(INameStr, BaseDir, FileData.cFileName);
					if (HFMD = new HelpFileMetaData)
						{
						HFMD->SetFileName(INameStr, WCS_APPHELP_TUTORIALBASE);
						if (HFMD->ReadMetaTags())
							{
							NumFilesLoaded++;
							HFMD->AttemptLoadThumb();
							} // if
						else
							{
							delete HFMD;
							HFMD = NULL;
							} // else
						if (HFMD)
							{ // add to chain
							HFMD->Next = MDChain;
							MDChain = HFMD;
							HFMD = NULL; // so it doesn't get discarded later
							} // if
						} // if
					} // if
				} // if
			} // if
		} while ((IHand != NULL) && (IHand != INVALID_HANDLE_VALUE));
	if (IHand)
		{
		FindClose(IHand);
		} // if
	} // if

delete HFMD; // NULL-safe
HFMD = NULL;

return(NumHelpFiles = NumFilesLoaded);

} // HelpFileIndex::BuildFileIndex

/*===========================================================================*/

void HelpFileIndex::DiscardFileIndex(void)
{
HelpFileMetaData *ChainNext = NULL, *ChainThis = NULL;

if (NumHelpFiles && MDChain)
	{
	for (ChainThis = MDChain; ChainThis; ChainThis = ChainNext)
		{
		ChainNext = ChainThis->Next;
		delete ChainThis;
		} // for
	} // if
MDChain = NULL;
NumHelpFiles = 0;

} // HelpFileIndex::DiscardFileIndex

/*===========================================================================*/

HelpFileMetaData *HelpFileIndex::GetNum(int FileNum)
{
int Counter = 0;
HelpFileMetaData *ChainThis = NULL;

for (ChainThis = MDChain; ChainThis; ChainThis = ChainThis->Next)
	{
	if (Counter == FileNum)
		{
		return(ChainThis);
		} // if
	Counter++;
	} // for

return(NULL);

} // HelpFileIndex::GetNum

/*===========================================================================*/

int HelpFileIndex::FindHighestPriorityNum(void)
{
int Highest = -1, Count = 0, HighestVal = -1000;
HelpFileMetaData *Search = NULL;

for (Search = MDChain; Search; Search = Search->Next)
	{
	if (Search->Priority > HighestVal)
		{
		Highest = Count;
		HighestVal = Search->Priority;
		} // if
	Count++;
	} // if

return(Highest);

} // HelpFileIndex::FindHighestPriorityNum
