/*
 * AutoSelfTest.c
 *
 *  Created on: Feb 14, 2025
 *      Author: afritsch
 */

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"
#include <unistd.h> // for sleep()

#include <time.h>

#ifdef __SASC
/* There is no sleep() in SAS/C */
#include <dos/dos.h>
#include <proto/dos.h>

int sleep(int seconds) {
    Delay(seconds * 50); // 1 Sekunde = 50 Ticks
    return 0;
}
#endif


struct Window* FindWindow(STRPTR WindowName)
{
   struct Window *Window;
   struct Window *Result=NULL;

   // Forbid multitasking to safely traverse the Window list
   Forbid();

//   // for debug
//   printf("Open Windows: ");
//   for (Window = WCSScrn->FirstWindow; Window->NextWindow != NULL; Window=Window->NextWindow)
//   {
//	  printf("<%s> ",(char*)Window->Title);
//   }
//   printf("\n\n");
//   // end for debug


   // Traverse the chain of open windows
   for (Window = WCSScrn->FirstWindow; Window->NextWindow != NULL; Window=Window->NextWindow)
   {
      if (strcmp((char*)Window->Title, (char*)WindowName) == 0)
      {
    	  Result=Window;
          break;
      }

      if (Window->NextWindow == NULL)
      {

         break;
      }

   }

   // Permit multitasking again
   Permit();
   return Result;
}

int CheckWindowOpen(STRPTR WindowTitle, unsigned int Line)
{
	if(FindWindow(WindowTitle))
	{
		// OK
		return 1;
	}
	else
	{
		printf("Window <%s> not open in Line %u!!!\n",WindowTitle, Line);
		// NOT OPEN!!!!, False
		return 0;
	}
}


int CheckWindowClosed(STRPTR WindowTitle, unsigned int Line)
{
	if(FindWindow(WindowTitle))
	{
		printf("Window <%s> is still open in Line %u!!!\n",WindowTitle,Line);
		// OPEN!!!, False
		return 0;
	}
	else
	{
		// NOT OPEN!, OK
		return 1;
	}
}




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

FILE*composefile=NULL;

static char* MakeNewcomposefileName(char *argv0)
{

	 static char temp[128];
	 {  // simulation of basename(), sets pointer to the beginning of the file name, i.e. skips path
		 char *progname = argv0+strlen(argv0)-1;   // pointer to the end of the progname
		 while(progname>=argv0 && *progname !='/' &&  *progname !=':')
		 {
			 progname--;
		 }
		 progname++;

		 sprintf(temp,"%s_%s",progname,"trace.txt");
		 printf("New composefile is now <%s>\n",temp);
	 }
	 return temp;
}



// returns a static string HH:MM:SS containing the time elapsed since StartTime
static char* MakeTimeString(time_t StartTime)
{
	static char time_string[12];  // space for HHHH:MM:SS (should be enough ;-)
	int hours,minutes, seconds;
	time_t elapsed_time = time(NULL) - StartTime;

	hours = elapsed_time / 3600;
	minutes = (elapsed_time % 3600) / 60;
	seconds = elapsed_time % 60;
	sprintf(time_string,"%02d:%02d:%02d",hours,minutes,seconds);
	return time_string;
}

unsigned long AF_DrandCounter=0;   // for display, how often af_drand48 was called
extern char *ProjectName;   // for compose debugging


void AutoSelfTest(char **argv)
{
	unsigned int TotalStartTime =time(NULL);

	composefile=fopen(MakeNewcomposefileName(argv[0]),"w");
	if(!composefile) { printf("Unable to open composfile!\n");}

	// ###############################################################################################################
	SetUser_Message_ForcedReturn(0); // do not save Old Param-File in new Format for automatic testing
	SetLoadparamsForceNogetfilenameptrn(TRUE); // do not open a File requester for the param file in loadparams() for automatic testing

	printf("sizeof(SHORT)=%d     (Amiga: 2)\n",sizeof(SHORT));
	printf("sizeof(short)=%d     (Amiga: 2)\n",sizeof(short));
	printf("sizeof(WORD)=%d      (Amiga: 2)\n",sizeof(WORD));
	printf("sizeof(int)=%d       (Amiga: 4)\n",sizeof(int));
	printf("sizeof(LONG)=%d      (Amiga: 4)\n",sizeof(LONG));
	printf("sizeof(long)=%d      (Amiga: 4)\n",sizeof(long));
	printf("sizeof(long long)=%d (Amiga: 8)\n",sizeof(long long));
	printf("sizeof(float)=%d     (Amiga: 4)\n",sizeof(float));
	printf("sizeof(double)=%d    (Amiga: 8)\n",sizeof(double));

	//-------------------------------------------------------------------------------
	// initial checks
/*
	Handle_APP_Windows( WI_WINDOW0 | GP_ACTIVEWIN );
	printf("WI_WINDOW0 | GP_ACTIVEWIN done. sleeping a moment...\n"); sleep (5);
*/
	//	Handle_APP_Windows( WI_WINDOW2 | GP_ACTIVEWIN| GP_BUTTONS1 | ID_MCP_ACTIVATE ); impossible case in AGUI.c
	//printf("WI_WINDOW2 | GP_ACTIVEWIN| GP_BUTTONS1 | ID_MCP_ACTIVATE done. sleeping a moment...\n"); sleep (5);

	if(!CheckWindowClosed(GetString (MSG_AGUI_STATUSLOG ),__LINE__)) {Set_WCS_ReturnCode(1); return;}
	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS1 | ID_LOG );
	//printf("WI_WINDOW2 | GP_BUTTONS1 | ID_LOG  done. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString (MSG_AGUI_STATUSLOG ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS1 | ID_LOG_HIDE );
	//printf("WI_WINDOW2 | GP_BUTTONS1 | ID_LOG_HIDE done. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowClosed(GetString (MSG_AGUI_STATUSLOG ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS1 | ID_LOG );
	//printf("WI_WINDOW2 | GP_BUTTONS1 | ID_LOG  done. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString (MSG_AGUI_STATUSLOG ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	if(!CheckWindowClosed(GetString( MSG_AGUI_INFO ),__LINE__)) {Set_WCS_ReturnCode(1); return;}
	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_INFO );
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_INFO done. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString( MSG_AGUI_INFO ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_INFO_FLUSH );  // flush button in Info Window
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_INFO_FLUSH done. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString( MSG_AGUI_INFO ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_INFO_ACTIVATE );
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_INFO_ACTIVATE...\n"); sleep (5);
	if(!CheckWindowOpen(GetString( MSG_AGUI_INFO ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_INFO );  // toggle?
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_INFO done. Info sollte geschlossen sein. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowClosed(GetString( MSG_AGUI_INFO ),__LINE__)) {Set_WCS_ReturnCode(1); return;}


	if(!CheckWindowOpen(GetString( MSG_AGUI_VERSION ),__LINE__)) {Set_WCS_ReturnCode(1); return;}
	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_VERSION );  // toggle Version, i.e. close it it
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_VERSION done. Should have closed now.  sleeping a moment...\n"); sleep (5);
	if(!CheckWindowClosed(GetString( MSG_AGUI_VERSION ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_VERSION );  // toggle Version, i.e. open it again
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_VERSION done. Should have opened now.  sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString( MSG_AGUI_VERSION ),__LINE__)) {Set_WCS_ReturnCode(1); return;}


	if(!CheckWindowClosed(GetString( MSG_AGUI_CREDITS ),__LINE__)) {Set_WCS_ReturnCode(1); return;}
	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_CREDITS );  // toggle Credits. i.e. open it
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_CREDITS done. Credits sollte ausgegenagen sein. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowOpen(GetString( MSG_AGUI_CREDITS ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

	Handle_APP_Windows( WI_WINDOW2 | GP_BUTTONS2 | ID_CREDITS );  // toggle Credits. i.e. close it
	//printf("WI_WINDOW2 | GP_BUTTONS2 | ID_CREDITS done. Credits sollte zugegangen sein. sleeping a moment...\n"); sleep (5);
	if(!CheckWindowClosed(GetString( MSG_AGUI_CREDITS ),__LINE__)) {Set_WCS_ReturnCode(1); return;}

// Das Modul Control Pannel ich anscheinend (noch) nicht automatisch geoeffnet worden...
//	printf("Nach %s schauen!",GetString( MSG_AGUI_MODULECONTROLPANEL ));
//	//printf("Module Control Panel should have been opened automatically. sleeping a moment...\n"); sleep (5);
//	if(!CheckWindowOpen(GetString( MSG_AGUI_MODULECONTROLPANEL ),__LINE__)) {Set_WCS_ReturnCode(1); return;} // Module Control Panel should have been opened automatically
//	sleep(20);
//// -------------------------------------------------------------------------------------------

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


	{
		{
			unsigned int SingleTestStartTime=time(NULL);
ProjectName="CanyonSunset.proj";
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


		Make_ES_Window();
		settings.maxframes = 1; // simulate Max Frames setting
		settings.renderopts &= ~0x20; // clear gray/color
		settings.renderopts |= 0x20; // gray  0x10=gray, 0x20=color
		strncpy(framepath, "VBox:SelcoGit/3DNature/Amiga/RenderTestImages/",
				sizeof(framepath)); // where to store the image
		MakeNewframefileName(argv[0]); // WCSname_image -> adds wcs_68020_ in front of CanyonSet for automatic testing
		Handle_RN_Window(MO_RENDER); // simulate pressing Render-Button
		printf("WCSProjects:CanyonSunset.proj finished after %s\n",MakeTimeString(SingleTestStartTime));
		}
		//------------
		{
			unsigned int SingleTestStartTime=time(NULL);
			ProjectName="RMNPAnim.proj";
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
			printf("WCSProjects:RMNPAnim.proj finished after %s\n",MakeTimeString(SingleTestStartTime));
		}
		// --------------
		{
			unsigned int SingleTestStartTime=time(NULL);
			LoadProject("WCSProjects:ColoDemo.proj", NULL, 0);   // WCSProjects:ColoDemo/ColoDemo.object/Demo1.par "Format of Parameterfile has been changed slightly..."
			ProjectName="ColoDemo.proj";
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
			printf("WCSProjects:ColoDemo.proj finished after %s\n",MakeTimeString(SingleTestStartTime));
		}
//		/// -----------------------------
		{
			unsigned int SingleTestStartTime=time(NULL);
			ProjectName="WorldVector.proj";
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
			printf("WCSProjects:WorldVector.proj finished after %s\n",MakeTimeString(SingleTestStartTime));
		}
		printf("All tests finished after %s\n",MakeTimeString(TotalStartTime));
	}

	printf("ALEXANDER: af_drand48() called %lu times.\n",AF_DrandCounter);
if(composefile) {fclose(composefile);}
}
