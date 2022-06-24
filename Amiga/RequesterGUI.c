/* Requester.c (ne gisrequester.c 14 Jan 1994 CXH)
** Requester stuff for WCS. 
** Written by Gary R. Huber, January, 1993 and modified for modular
** implementation (a la Chris Hanson) in July 1993.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include <time.h>
#include <stdarg.h>

// Not used, AF, 21.Jun22, found with --gc-sections,--print-gc-sections
#ifdef THIS_IS_NOT_USED
STATIC_VAR struct DateTime PocketWatch;
#endif
STATIC_VAR char PWDate[26], Today[26], ProjDate[26];
STATIC_VAR struct FileRequester *frbase,
// Not used, AF, 21.Jun22, found with --gc-sections,--print-gc-sections
#ifdef THIS_IS_NOT_USED
*frparam,
#endif
*frfile;

// void MUI_DoNotifyPressed(APTR Target, ULONG ID); // AF, not used 26.July 2021


short getdbasename(long mode)
{
#ifdef AMIGA_GUI
get(ModControlWin, MUIA_Window_Window, &MCPWin); 
 if ((frbase = (struct FileRequester *)
   AllocAslRequestTags(ASL_FileRequest,
     ASLFR_InitialDrawer, (ULONG)dbasepath,
     ASLFR_InitialFile, (ULONG)dbasename,
     ASLFR_InitialHeight, 200,
     ASLFR_InitialLeftEdge, 250,
     ASLFR_InitialTopEdge, 150,
     ASLFR_Window, (ULONG)MCPWin,
     TAG_DONE)) == NULL) {
  printf("Can't initialize file requester!\n");
  return 0;
 }
 if (mode) {
  if ((AslRequestTags(frbase,
      ASL_Hail, (ULONG)"DataBase File Saver",
      ASL_FuncFlags, FILF_PATGAD | FILF_SAVE | FILF_NEWIDCMP,
      TAG_DONE)) == 0) {
   FreeAslRequest(frbase);
   return 0;
  }
 }
 else {
  if ((AslRequestTags(frbase,
      ASL_Hail,(ULONG)"DataBase File Loader",
      ASL_FuncFlags,FILF_PATGAD | FILF_NEWIDCMP,
      TAG_DONE))==0) {
   FreeAslRequest(frbase);
   return 0;
  }
 }
 strcpy(dbasepath, (char*)frbase->fr_Drawer);
 strcpy(dbasename, (char*)frbase->fr_File);
 FreeAslRequest(frbase);
 return 1;
#else
 printf("Enter Database path/name: ");
 scanf("%s",str);
 stcgfn(dbasename,str);			/* lattice function */
 stcgfp(dbasepath,str);			lattice function
 return 1;
#endif /* AMIGA_GUI */
}

/**************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
short getparamfile(long mode)
{
#ifdef AMIGA_GUI
get(ModControlWin, MUIA_Window_Window, &MCPWin); 

 if ((frparam = (struct FileRequester *)
   AllocAslRequestTags(ASL_FileRequest,
     ASLFR_InitialDrawer, parampath,
     ASLFR_InitialFile, paramfile,
     ASLFR_InitialHeight, 200,
     ASLFR_InitialLeftEdge, 250,
     ASLFR_InitialTopEdge, 150,
     ASLFR_Window, MCPWin,
     TAG_DONE)) == NULL) {
  printf("Can't initialize file requester!\n");
  return 0;
 }

 if (mode) {
  if ((AslRequestTags(frparam,
      ASL_Hail, (ULONG)"Parameter File Saver",
      ASL_FuncFlags, FILF_PATGAD | FILF_SAVE | FILF_NEWIDCMP,
      ASL_Pattern, (ULONG)"#?.par",
      TAG_DONE)) == 0) {
   FreeAslRequest(frparam);
   return 0;
  }
 }
 else {
  if ((AslRequestTags(frparam,
      ASL_Hail, (ULONG)"Parameter File Loader",
      ASL_FuncFlags, FILF_PATGAD | FILF_NEWIDCMP,
      ASL_Pattern, (ULONG)"#?.par",
      TAG_DONE)) == 0) {
   FreeAslRequest(frparam);
   return 0;
  }
 }
 strcpy(parampath, frparam->fr_Drawer);
 strcpy(paramfile, frparam->fr_File);
 FreeAslRequest(frparam);
 return 1;
#else
 printf("Enter Parameter file path/name: ");
 scanf("%s", str);
 stcgfn(paramfile, str);		/* lattice function */
 stcgfp(parampath, str);		/* lattice function */
 return 1;
#endif /* AMIGA_GUI */
}
#endif
/**************************************************************************/

short getfilename(long mode, char *requestname, char *pathname,
    char *filename)
{
#ifdef AMIGA_GUI
 get(ModControlWin, MUIA_Window_Window, &MCPWin); 

 if ((frfile=(struct FileRequester *)
   AllocAslRequestTags(ASL_FileRequest,
     ASLFR_InitialDrawer, (ULONG)pathname,
     ASLFR_InitialFile, (ULONG)filename,
     ASLFR_InitialHeight, 200,
     ASLFR_InitialLeftEdge, 250,
     ASLFR_InitialTopEdge, 150,
     ASLFR_Window, (ULONG)MCPWin,
     TAG_DONE)) == NULL) {
  printf("Can't initialize file requester!\n");
  return 0;
 }
 if (mode) {
  if ((AslRequestTags(frfile,
      ASL_Hail, (ULONG)requestname,
      ASL_FuncFlags, FILF_PATGAD | FILF_SAVE | FILF_NEWIDCMP,
      TAG_DONE)) == 0) {
   FreeAslRequest(frfile);
   return 0;
  }
 }
 else {
  if ((AslRequestTags(frfile,
      ASL_Hail, (ULONG)requestname,
      ASL_FuncFlags, FILF_PATGAD | FILF_NEWIDCMP,
      TAG_DONE)) == 0) {
   FreeAslRequest(frfile);
   return 0;
  }
 }
 strcpy(pathname, (char*)frfile->fr_Drawer);
 strcpy(filename, (char*)frfile->fr_File);
 FreeAslRequest(frfile);
 return 1;
#else
 printf("Enter file path/name: ");
 scanf("%s", str);
 stcgfn(filename, str);			/* lattice function */
 stcgfp(pathname, str);			/* lattice function */
 return 1;
#endif /* AMIGA_GUI */
}

/**************************************************************************/

short getfilenameptrn(long mode, char *requestname, char *pathname,
    char *filename, char *ptrn)
{
#ifdef AMIGA_GUI
 get(ModControlWin, MUIA_Window_Window, &MCPWin); 

 if ((frfile=(struct FileRequester *)
   AllocAslRequestTags(ASL_FileRequest,
     ASLFR_InitialDrawer, (ULONG)pathname,
     ASLFR_InitialFile, (ULONG)filename,
     ASLFR_InitialPattern, (ULONG)ptrn,
     ASLFR_InitialHeight, 200,
     ASLFR_InitialLeftEdge, 250,
     ASLFR_InitialTopEdge, 150,
     ASLFR_Window, (ULONG)MCPWin,
     TAG_DONE)) == NULL) {
  printf("Can't initialize file requester!\n");
  return 0;
 }
 if (mode) {
  if ((AslRequestTags(frfile,
      ASL_Hail, (ULONG)requestname,
      ASL_FuncFlags, FILF_PATGAD | FILF_SAVE | FILF_NEWIDCMP,
      TAG_DONE)) == 0) {
   FreeAslRequest(frfile);
   return 0;
  }
 }
 else {
  if ((AslRequestTags(frfile,
      ASL_Hail, (ULONG)requestname,
      ASL_FuncFlags, FILF_PATGAD | FILF_NEWIDCMP,
      TAG_DONE)) == 0) {
   FreeAslRequest(frfile);
   return 0;
  }
 }
 strcpy(pathname, (char*)frfile->fr_Drawer);
 strcpy(filename, (char*)frfile->fr_File);
 FreeAslRequest(frfile);
 return 1;
#else
 printf("Enter file path/name: ");
 scanf("%s", str);
 stcgfn(filename, str);			/* lattice function */
 stcgfp(pathname, str);			/* lattice function */
 return 1;
#endif /* AMIGA_GUI */
}

/**************************************************************************/

struct FileRequester *getmultifilename(char *requestname, char *pathname,
    char *filename, char *pattern)
{
 struct FileRequester *frfile;
 static struct WBArg SingleFile;
 static char SingleFileName[40];

#ifdef AMIGA_GUI
 get(ModControlWin, MUIA_Window_Window, &MCPWin); 

 if ((frfile = (struct FileRequester *)
   AllocAslRequestTags(ASL_FileRequest,
     ASLFR_InitialDrawer, (ULONG)pathname,
     ASLFR_InitialFile, (ULONG)filename,
     ASLFR_InitialHeight, 200,
     ASLFR_InitialLeftEdge, 250,
     ASLFR_InitialTopEdge, 150,
     ASLFR_Window, (ULONG)MCPWin,
     TAG_DONE)) == NULL) {
  printf("Can't initialize file requester!\n");
  return (0);
 }
 if ((AslRequestTags(frfile,
     ASL_Hail, (ULONG)requestname,
     ASL_FuncFlags, FILF_PATGAD | FILF_MULTISELECT | FILF_NEWIDCMP,
     ASL_Pattern, (ULONG)pattern,
     TAG_DONE)) == 0) {
  FreeAslRequest(frfile);
  return (NULL);
 }
 if (frfile->rf_NumArgs == 0)
  {
  if (frfile->rf_File[0])
   {
   strcpy(SingleFileName, (char*)frfile->rf_File);
   SingleFile.wa_Name = (BYTE*)SingleFileName;
   frfile->rf_ArgList = &SingleFile; /* <<<>>> May be a no-no */
   frfile->rf_NumArgs = 1; /* <<<>>> May be a no-no */
   }
  else
   frfile->rf_NumArgs = 0;
  }
 strcpy(pathname, (char*)frfile->rf_Dir);
 return (frfile);
#else
 printf("Enter file path/name: ");
 scanf("%s", str);
 stcgfn(filename, str);			/* lattice function */
 stcgfp(pathname, str);			/* lattice function */
 return (1);
#endif /* AMIGA_GUI */
}

/**************************************************************************/

void freemultifilename(struct FileRequester *This)
{

if(This)
 FreeAslRequest(This);

} /* freemultifilename() */

/**************************************************************************/

struct BusyWindow *BusyWin_New(char *Title, int Steps, int TimeEst, ULONG Section)
{
struct BusyWindow *This;
long open;

Set_Param_Menu(10);

if ((This = (struct BusyWindow *)get_Memory(sizeof(struct BusyWindow), MEMF_CLEAR)))
	{
	if(TimeEst)
		{
		time((time_t *)&This->StartSeconds);
		} /* if */
	This->MaxSteps = Steps;
	This->CurSteps = 0;
	This->BusyWin = WindowObject,
	 MUIA_Window_Title,	Title,
	 MUIA_Window_ID,		Section,
	 MUIA_Window_Screen,	WCSScrn,
	 MUIA_Window_Activate, FALSE,
	 WindowContents, VGroup,
	  Child, This->BW_Percent = GaugeObject,
	   GaugeFrame,
	   MUIA_Background, MUII_BACKGROUND,
	   MUIA_Gauge_Max, Steps,
	   MUIA_Gauge_Current, 0,
	   MUIA_Gauge_Horiz, TRUE,
	   End,
	  Child, ScaleObject, MUIA_Scale_Horiz, TRUE,
	   End,
	  Child, HGroup,
	   Child, This->BW_Elapse = TextObject,
	    MUIA_Weight, 0,
	    MUIA_Text_Contents, "        ",
	    End,
	   Child, RectangleObject, End,
	   Child, This->BW_Remain = TextObject,
	    MUIA_Weight, 0,
	    MUIA_Text_Contents, "        ",
	    End,
	   End,
	  Child, HGroup,
	   Child, RectangleObject, End,
	   Child, This->BW_Cancel = KeyButtonFunc('c', "\33cCancel"),
	   Child, RectangleObject, End,
	   End,
	  End,
	 End;
	if(This->BusyWin)
	  {
	  if(This->StartSeconds)
	  	{
	  	set(This->BW_Remain, MUIA_Text_Contents, (ULONG)"00:00:00");
	  	set(This->BW_Elapse, MUIA_Text_Contents, (ULONG)"00:00:00");
	  	set(This->BW_Percent, MUIA_Gauge_InfoText, (ULONG)"\33c\033200:00:00");
	  	} /* if */
	  DoMethod(app, OM_ADDMEMBER, This->BusyWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
	  DoMethod(This->BusyWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
	   app, 2, MUIM_Application_ReturnID, ID_BW_CLOSE);
	  MUI_DoNotiPresFal(app, This->BW_Cancel, ID_BW_CLOSE, NULL);
	  set(This->BusyWin, MUIA_Window_Open, TRUE);
	  get(This->BusyWin, MUIA_Window_Open, &open);
          if (! open)
           {
           BusyWin_Del(This);
           return (NULL);
	   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
	  return(This);
	  } /* if */
	else
	  {
	  free_Memory(This, sizeof(struct BusyWindow));
	  } /* else */
	} /* if */

return(NULL);
} /* BusyWin_New() */

/**************************************************************************/

void BusyWin_Del(struct BusyWindow *This)
{

if(This)
	{
	set(This->BusyWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, This->BusyWin);
   MUI_DisposeObject(This->BusyWin);
   free_Memory(This, sizeof(struct BusyWindow));
	} /* if */

} /* BusyWin_Del() */

/**************************************************************************/

void BusyWin_Update(struct BusyWindow *This, int Step)
{
ULONG NowSecs, Elapsed, Remain, Projected;
UBYTE ElapHrs, ElapMin, ElapSec,
      RemHrs, RemMin, RemSec;

if(This)
	{
	if(Step == 0)
		{
		Step = 1; /* Prevent / by 0 */
		} /* if */
	set(This->BW_Percent, MUIA_Gauge_Current, Step);
	if(This->StartSeconds)
		{
		time((time_t *)&NowSecs);
		Elapsed = NowSecs - This->StartSeconds;
		ElapSec = Elapsed % 60;
		ElapMin = (Elapsed / 60) % 60;
		ElapHrs = Elapsed / 3600;

		/* Replace this with the algorithm of your choice, Gary. ;) */
		Remain = (Elapsed * This->MaxSteps / Step) - Elapsed; /* Fixed for more accuracy */
		RemSec = Remain % 60;
		RemMin = (Remain / 60) % 60;
		RemHrs = Remain / 3600;
		Projected = NowSecs + Remain;
		strcpy(PWDate, ctime((time_t *)&Projected));
		strcpy(Today, ctime((time_t *)&This->StartSeconds));
		if(strncmp(Today, PWDate, 10))
			{
			sprintf(ProjDate, "\0332%19s", PWDate);
			ProjDate[21] = 0;
/*			strncpy(ProjDate, PWDate, 19);
			ProjDate[19] = NULL;*/
			} /* if */
		else
			{
			sprintf(ProjDate, "\0332%8s", &PWDate[11]);
			ProjDate[10] = 0;
/*			strncpy(ProjDate, &PWDate[11], 8);
			ProjDate[8] = NULL;*/
			} /* if */
		sprintf(PWDate, "%02d:%02d:%02d", ElapHrs, ElapMin, ElapSec); /* Elapsed */
		sprintf(Today, "%02d:%02d:%02d", RemHrs, RemMin, RemSec); /* Remaining */
		set(This->BW_Percent, MUIA_Gauge_InfoText, (ULONG)ProjDate);
		set(This->BW_Elapse, MUIA_Text_Contents, (ULONG)PWDate);
		set(This->BW_Remain, MUIA_Text_Contents, (ULONG)Today);
		} /* if */
	} /* if */

} /* BusyWin_Update() */

/**************************************************************************/

void Log_ElapsedTime(ULONG StartSecs, ULONG FirstSecs, long Frames)
{
ULONG NowSecs, Elapsed;
UBYTE ElapHrs, ElapMin, ElapSec;
char FrameStr[16], TotalFrames[16];

if(StartSecs)
	{
	time((time_t *)&NowSecs);
	Elapsed = NowSecs - StartSecs;
	ElapSec = Elapsed % 60;
	ElapMin = (Elapsed / 60) % 60;
	ElapHrs = Elapsed / 3600;

        if (settings.fieldrender)
         {
         sprintf(FrameStr, "%1d", (frame / 2) + (frame % 2));
         sprintf(TotalFrames, "%1ld", Frames / 2);
         if (frame % 2)
          {
	  strcat(FrameStr, "A");
	  strcat(TotalFrames, ".5");
	  }
	 else
          {
	  strcat(FrameStr, "B");
	  }
	 } /* if */
        else
         {
         sprintf(FrameStr, "%1d", frame);
         sprintf(TotalFrames, "%1ld", Frames);
	 } /* else */
	sprintf(str, "\0334%s:  %02d:%02d:%02d", FrameStr, ElapHrs, ElapMin, ElapSec);
	Log(MSG_TIME_ELAPSE, (CONST_STRPTR)str);

	Elapsed = NowSecs - FirstSecs;
	ElapSec = Elapsed % 60;
	ElapMin = (Elapsed / 60) % 60;
	ElapHrs = Elapsed / 3600;

	sprintf(str, "\0334%s frames:  %02d:%02d:%02d", TotalFrames, ElapHrs, ElapMin, ElapSec);
	Log(MSG_TOTAL_ELAPS, (CONST_STRPTR)str);
	} /* if */

} /* Log_ElapsedTime() */

/**************************************************************************/

/* Takes place of alot of macros */

APTR KeyButtonFunc(char ControlChar, char *Contents)
{
return(KeyButtonObject(ControlChar), MUIA_Text_Contents, Contents, End);
} /* KeyButtonFunc() */

/**************************************************************************/

/* Stand and watch the world fall in. */
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void MUI_DoNotifyPressed(APTR Target, ULONG ID)
{

DoMethod(Target, MUIM_Notify, MUIA_Pressed, FALSE, app, 2,
 MUIM_Application_ReturnID, ID);
return;
} /* MUI_DoNotifyPressed() */
#endif
/**************************************************************************/

/* More of the same, but BIGGER BETTER FASTER! */

void MUI_DoNotiPresFal(APTR App, ...)
{
va_list VarA;
APTR Target;
ULONG ID;

va_start(VarA, App);

while(1)
	{
	Target = va_arg(VarA, APTR);
	if(Target == NULL)
		break;
	ID = va_arg(VarA, ULONG);
	if(ID)
		DoMethod(Target, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
		 MUIM_Application_ReturnID, ID);
	} /* whilever */

va_end(VarA);

return;
} /* MUI_DoNotiPresFal() */
