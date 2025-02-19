/*
 * AutoSelfTest.c
 *
 *  Created on: Feb 14, 2025
 *      Author: afritsch
 */

#include "WCS.h"

void Handle_RN_Window(ULONG WCS_ID);
void SetLoadparamsForceNogetfilenameptrn(int Val);
void SetUser_Message_ForcedReturn(int Val);

static void MakeNewframefileName(char *argv0)  // AF, 12.Dec.24, WCSname_image -> adds wcs_68020_ in front of CanyonSet for automatic testing
{

	 static char temp[128];
	 {  // simulation of basename(), sets pointer to the beginning of the file name, i.e. skips path
		 char *progname = argv0+strlen(argv0)-1;   // pointer to the end of the progname
		 while(progname>=argv0 && *progname !='/' &&  *progname !=':')
		 {
			 progname--;
		 }
		 progname++;

		 sprintf(temp,"%s_%s",progname,framefile);
		 strncpy(framefile,temp,256-1);
		 framefile[64-1]='\0';
		 printf("New framefile is now <%s>\n",framefile);
	 }
}


short AutoSelfTest(char **argv)
{
	// ###############################################################################################################
	SetUser_Message_ForcedReturn(0); // do not save Old Param-File in new Format for automatic testing
	SetLoadparamsForceNogetfilenameptrn(TRUE); // do not open a File requester for the param file in loadparams() for automatic testing
	LoadProject("WCSProjects:CanyonSunset.proj", NULL, 0); // WCSProjects:Arizona/SunsetAnim "Format of Parameterfile has been changed slightly..."
	if (0 == Database_Load(0, "WCSProjects:Arizona/SunsetAnim")) // 0 mean no error
	//-----
	//LoadProject("WCSProjects:RMNPAnim.proj", NULL, 0);   // WCSProjects:Colorado/RMNP.object/RMNPAnim.par "Format of Parameterfile has been changed slightly..."
	//if(0==Database_Load(0,"WCSProjects:Colorado/RMNP"))   // 0 mean no error
	//-----
	//LoadProject("WCSProjects:ColoDemo.proj", NULL, 0);   // WCSProjects:ColoDemo/ColoDemo.object/Demo1.par "Format of Parameterfile has been changed slightly..."
	//if(0==Database_Load(0,"WCSProjects:ColoDemo/ColoDemo"))   // 0 mean no error
	//-----
	// very slow!
	//LoadProject("WCSProjects:LargeWorld.proj", NULL, 0);   // WCSProjects:LargeWorld/LargeWorld.object/WorldTest.par "This is an old V1 format file!"
	//if(0==Database_Load(0,"WCSProjects:LargeWorld/LargeWorld"))   // 0 mean no error
	{
		dbaseloaded = 1;
	}
	if (loadparams(0x1111, -1) == 1)
	{
		paramsloaded = 1;
		FixPar(0, 0x1111);
		FixPar(1, 0x1111);
	}
	// -------------------------------------------------------
	//               50 lines more Coverage in AGUI.c when active
	//               Window: "Parameter Module"
	Make_EP_Window(0); // stand up
	Close_EP_Window();
	Make_EP_Window(1);
	Close_EP_Window(); // lay down
	// -------------------------------------------------------
	//               Window: "Database Module"
	Make_DB_Window(0); // stand up
	Close_DB_Window();
	Make_DB_Window(1);
	Close_DB_Window(); // lay down
	// -------------------------------------------------------
	//               Window: "DataOps Module"
	Make_DO_Window(0); // stand up
	Close_DO_Window();
	Make_DO_Window(0);
	Close_DO_Window(); // lay down
	// -------------------------------------------------------
	Handle_APP_Windows( WI_WINDOW0 | GP_ACTIVEWIN );
	// -------------------------------------------------------


	Make_ES_Window();
	settings.maxframes = 1; // simulate Max Frames setting
	settings.renderopts &= ~0x20; // clear gray/color
	settings.renderopts |= 0x20; // gray  0x10=gray, 0x20=color
	strncpy(framepath, "VBox:SelcoGit/3DNature/Amiga/RenderTestImages/",
			sizeof(framepath)); // where to store the image
	MakeNewframefileName(argv[0]); // WCSname_image -> adds wcs_68020_ in front of CanyonSet for automatic testing
	Handle_RN_Window(MO_RENDER); // simulate pressing Render-Button
	//------------
	LoadProject("WCSProjects:RMNPAnim.proj", NULL, 0); // WCSProjects:Colorado/RMNP.object/RMNPAnim.par "Format of Parameterfile has been changed slightly..."
	if (0 == Database_Load(0, "WCSProjects:Colorado/RMNP")) // 0 mean no error
	{
		dbaseloaded = 1;
	}
	if (loadparams(0x1111, -1) == 1)
	{
		paramsloaded = 1;
		FixPar(0, 0x1111);
		FixPar(1, 0x1111);
	}
	Make_ES_Window();
	settings.maxframes = 1; // simulate Max Frames setting
	settings.renderopts &= ~0x20; // clear gray/color
	settings.renderopts |= 0x20; // gray  0x10=gray, 0x20=color
	strncpy(framepath, "VBox:SelcoGit/3DNature/Amiga/RenderTestImages/",
			sizeof(framepath)); // where to store the image
	MakeNewframefileName(argv[0]); // WCSname_image -> adds wcs_68020_ in front of RMNP for automatic testing
	Handle_RN_Window(MO_RENDER); // simulate pressing Render-Button
	// Vorgabe Screenmode
	// --------------
	LoadProject("WCSProjects:ColoDemo.proj", NULL, 0); // WCSProjects:ColoDemo/ColoDemo.object/Demo1.par "Format of Parameterfile has been changed slightly..."
	if (0 == Database_Load(0, "WCSProjects:ColoDemo/ColoDemo")) // 0 mean no error
	{
		dbaseloaded = 1;
	}
	if (loadparams(0x1111, -1) == 1)
	{
		paramsloaded = 1;
		FixPar(0, 0x1111);
		FixPar(1, 0x1111);
	}
	Make_ES_Window();
	//settings.maxframes=1;                                              // simulate Max Frames setting
	settings.renderopts &= ~0x20; // clear gray/color
	settings.renderopts |= 0x20; // gray  0x10=gray, 0x20=color
	strncpy(framepath, "VBox:SelcoGit/3DNature/Amiga/RenderTestImages/",
			sizeof(framepath)); // where to store the image
	MakeNewframefileName(argv[0]); // WCSname_image -> adds wcs_68020_ in front of DemoFrame for automatic testing
	Handle_RN_Window(MO_RENDER); // simulate pressing Render-Button
	// Press button bei Parameter-Loading -> extra parameter fuer Filenamen einbauen?
	// Vorgabe Screenmode
	/// -----------------------------
	LoadProject("WCSProjects:WorldVector.proj", NULL, 0); // WCSProjects:WorldVector/WorldVector.object/WorldTest.par "This is an old V1 format file!"
	if (0 == Database_Load(0, "WCSProjects:WorldVector/WorldVector")) // 0 mean no error
	{
		dbaseloaded = 1;
	}
	if (loadparams(0x1111, -1) == 1)
	{
		paramsloaded = 1;
		FixPar(0, 0x1111);
		FixPar(1, 0x1111);
	}
	Make_ES_Window();
	//settings.maxframes=1;                                              // simulate Max Frames setting
	settings.renderopts &= ~0x20; // clear gray/color
	settings.renderopts |= 0x20; // gray  0x10=gray, 0x20=color
	strncpy(framepath, "VBox:SelcoGit/3DNature/Amiga/RenderTestImages/",
			sizeof(framepath)); // where to store the image
	MakeNewframefileName(argv[0]); // WCSname_image -> adds wcs_68020_ in front of WorldTest for automatic testing
	Handle_RN_Window(MO_RENDER); // simulate pressing Render-Button
	// Press button bei Parameter-Loading -> extra parameter fuer Filenamen einbauen?
	// Vorgabe Screenmode
}
