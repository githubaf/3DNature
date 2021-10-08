/* EvenMoreGUI.c
** World Construction Set GUI for Data Operations module.
** Copyright 1994 by Gary R. Huber and Chris Eric Hanson.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Version.h"
#include "Wave.h"

STATIC_FCN void Set_TS_Reverse(short reverse); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Set_TS_Position(void); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Make_PN_Window(void); // used locally only -> static, AF 25.7.2021
STATIC_FCN short CreateNewProject(char *NewProjName, char *CloneProjName); // used locally only -> static, AF 25.7.2021


void Make_TS_Window(void)
{
 long open;
 static const char *TS_Cycle_AMPM[] = {"AM", "PM", NULL};
 static const char *TS_Cycle_Month[] = {"Jan", "Feb", "Mar", "Apr", "May",
	 "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
 static const char *TS_Cycle_Date[] = {"1", "2", "3", "4", "5", "6", "7", "8",
	 "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
	 "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", NULL};

 if (TS_Win)
  {
  DoMethod(TS_Win->TimeSetWin, MUIM_Window_ToFront);
  set(TS_Win->TimeSetWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((TS_Win = (struct TimeSetWindow *)
	get_Memory(sizeof (struct TimeSetWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((TS_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_TS_Window(0);
  return;
  } /* if out of memory */
 memcpy(TS_Win->AltKF, KF, KFsize);
 TS_Win->AltKFsize = KFsize;
 TS_Win->AltKeyFrames = ParHdr.KeyFrames;
 TS_Win->SunLat = PAR_FIRST_MOTION(15);
 TS_Win->SunLon = PAR_FIRST_MOTION(16);

  Set_Param_Menu(0);

     TS_Win->TimeSetWin = WindowObject,
      MUIA_Window_Title		, "Sun Time",
      MUIA_Window_ID		, MakeID('E','P','T','S'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2("Ref Lon"),
	  Child, TS_Win->LonStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890",
		MUIA_String_Accept, "-.0123456789", End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, Label2("Date"),
	  Child, TS_Win->MonthCycle = CycleObject,
		MUIA_Cycle_Entries, TS_Cycle_Month, End,
	  Child, TS_Win->DateCycle = CycleObject,
		MUIA_Cycle_Entries, TS_Cycle_Date, End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, Label2("Time"),
	  Child, TS_Win->TimeStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345",
		MUIA_String_Accept, ":.0123456789", End,
	  Child, TS_Win->AMPMCycle = CycleObject,
		MUIA_Cycle_Entries, TS_Cycle_AMPM, End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, Label2("Sun Lon"),
	  Child, TS_Win->SunLonStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890",
		MUIA_String_Accept, "-.0123456789", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label2("Sun Lat"),
	  Child, TS_Win->SunLatStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890",
		MUIA_String_Accept, "-.0123456789", End,
	  End, /* HGroup */

	Child, TS_Win->BT_Reverse = KeyButtonFunc('r', "\33cReverse Seasons"),

	Child, HGroup, MUIA_Group_SameWidth, TRUE,
	  Child, TS_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"),
	  Child, TS_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"),
	  End, /* HGroup */
        End, /* VGroup */
      End; /* WindowObject */

  if (! TS_Win->TimeSetWin)
   {
   Close_TS_Window(0);
   User_Message((CONST_STRPTR)"Sun Time Window", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, TS_Win->TimeSetWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

 sprintf(str, "%f", PAR_FIRST_MOTION(5));
 set(TS_Win->LonStr, MUIA_String_Contents, str);
 Set_TS_Reverse(0);

/* ReturnIDs */
  DoMethod(TS_Win->TimeSetWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_TS_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, TS_Win->BT_Reverse, ID_TS_REVERSE,
   TS_Win->BT_Apply, ID_TS_APPLY, TS_Win->BT_Cancel, ID_TS_CANCEL, NULL);

  DoMethod(TS_Win->MonthCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_TS_MONTHCYCLE);
  DoMethod(TS_Win->DateCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_TS_DATECYCLE);
  DoMethod(TS_Win->AMPMCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_TS_AMPMCYCLE);

  DoMethod(TS_Win->LonStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_TS_REFLONSTR);
  DoMethod(TS_Win->TimeStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_TS_TIMESTR);

  DoMethod(TS_Win->SunLatStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_TS_SUNSTR);
  DoMethod(TS_Win->SunLonStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_TS_SUNSTR);

/* set tab cycle chain */
  DoMethod(TS_Win->TimeSetWin, MUIM_Window_SetCycleChain,
   TS_Win->LonStr, TS_Win->MonthCycle, TS_Win->DateCycle,
   TS_Win->TimeStr, TS_Win->AMPMCycle, TS_Win->BT_Reverse,
   TS_Win->SunLonStr, TS_Win->SunLatStr,
   TS_Win->BT_Apply, TS_Win->BT_Cancel, NULL);

/* set return cycle chain */
 DoMethod(TS_Win->LonStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  TS_Win->TimeSetWin, 3, MUIM_Set, MUIA_Window_ActiveObject, TS_Win->TimeStr); 
 DoMethod(TS_Win->TimeStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  TS_Win->TimeSetWin, 3, MUIM_Set, MUIA_Window_ActiveObject, TS_Win->SunLatStr); 
 DoMethod(TS_Win->SunLatStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  TS_Win->TimeSetWin, 3, MUIM_Set, MUIA_Window_ActiveObject, TS_Win->SunLonStr); 
 DoMethod(TS_Win->SunLonStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  TS_Win->TimeSetWin, 3, MUIM_Set, MUIA_Window_ActiveObject, TS_Win->LonStr); 

/* set active gadget */
 set(TS_Win->TimeSetWin, MUIA_Window_ActiveObject, TS_Win->TimeStr); 

/* Open window */
  set(TS_Win->TimeSetWin, MUIA_Window_Open, TRUE);
  get(TS_Win->TimeSetWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_TS_Window(0);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(TS_Win->TimeSetWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_TS_ACTIVATE);

} /* Open_TS_Window() */

/********************************************************************/

void Close_TS_Window(short apply)
{

 if (TS_Win)
  {
  if (TS_Win->TimeSetWin)
   {
   if (apply)
    free_Memory(TS_Win->AltKF, TS_Win->AltKFsize);
   else
    {
    PAR_FIRST_MOTION(15) = TS_Win->SunLat;
    PAR_FIRST_MOTION(16) = TS_Win->SunLon;
    MergeKeyFrames(TS_Win->AltKF, TS_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 0);
    free_Memory(TS_Win->AltKF, TS_Win->AltKFsize);
    UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
    GetKeyTableValues(0, EM_Win->MoItem, 1);
    Set_EM_Item(EM_Win->MoItem);
    Set_Radial_Txt(2);  
    ResetTimeLines(-1);
    DisableKeyButtons(0);
    } /* if apply */
   set(TS_Win->TimeSetWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, TS_Win->TimeSetWin);
   MUI_DisposeObject(TS_Win->TimeSetWin);
   } /* if window created */
  free_Memory(TS_Win, sizeof (struct TimeSetWindow));
  TS_Win = NULL;
  } /* if memory allocated */

} /* Close_TS_Window() */

/********************************************************************/

void Handle_TS_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_TS_Window();
   return;
   } /* Open DEM Extract window */

  if (! TS_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate DEM Extract window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_TS_REVERSE:
      {
      Set_TS_Reverse(1);
      break;
      } /* apply */
     case ID_TS_APPLY:
      {
      Close_TS_Window(1);
      break;
      } /* apply */
     case ID_TS_CANCEL:
      {
      Close_TS_Window(0);
      break;
      } /* cancel */
     case ID_TS_CLOSEQUERY:
      {
      if (TS_Win->Mod)
       Close_TS_Window(CloseWindow_Query((STRPTR)"Sun Time Window"));
      else
       Close_TS_Window(1);
      break;
      } /* close */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
   case GP_CYCLE1:
    {
    switch (WCS_ID)
     {
     case ID_TS_REFLONSTR:
     case ID_TS_TIMESTR:
     case ID_TS_MONTHCYCLE:
     case ID_TS_DATECYCLE:
     case ID_TS_AMPMCYCLE:
      {
      Set_TS_Position();
      TS_Win->Mod = 1;
      break;
      }
     } /* switch gadget ID */
    break;
    } /* CYCLE1, STRING1 */

   case GP_STRING2:
    {
    char *SunLatStr, *SunLonStr;

    get(TS_Win->SunLatStr, MUIA_String_Contents, &SunLatStr);
    get(TS_Win->SunLonStr, MUIA_String_Contents, &SunLonStr);
    PAR_FIRST_MOTION(15) = atof(SunLatStr);
    PAR_FIRST_MOTION(16) = atof(SunLonStr);

    Update_EM_Item();
    Set_Radial_Txt(2);
    break;
    } /* STRING2 */
   } /* switch gadget group */

} /* Handle_TS_Window() */

/**********************************************************************/

STATIC_FCN void Set_TS_Position(void) // used locally only -> static, AF 25.7.2021
{
char *TimeStr, *RefLonStr;
short Pos = 0, Hour, Mins, Days;
long SunDate, SunMonth, AMPM;
double SunLon, SunLat, RefLon, SunTime;

 get(TS_Win->MonthCycle, MUIA_Cycle_Active, &SunMonth);
 get(TS_Win->DateCycle, MUIA_Cycle_Active, &SunDate);
 get(TS_Win->AMPMCycle, MUIA_Cycle_Active, &AMPM);
 get(TS_Win->LonStr, MUIA_String_Contents, &RefLonStr);
 get(TS_Win->TimeStr, MUIA_String_Contents, &TimeStr);

 while (TimeStr[Pos] != '.' && TimeStr[Pos] != ':' && TimeStr[Pos] != 0)
  Pos ++;
 Hour = atoi(TimeStr);

 if (Hour >= 12)
  Hour -= 12;
 if (AMPM)
  Hour += 12;
 
 if (Pos < strlen(TimeStr))
  Mins = atoi(&TimeStr[Pos + 1]);
 else
  Mins = 0;

 switch (SunMonth)
  {
  case 0:
   {
   Days = 10;
   break;
   }
  case 1:
   {
   Days = 41;
   break;
   }
  case 2:
   {
   Days = 69;
   break;
   }
  case 3:
   {
   Days = 100;
   break;
   }
  case 4:
   {
   Days = 130;
   break;
   }
  case 5:
   {
   Days = 161;
   break;
   }
  case 6:
   {
   Days = 191;
   break;
   }
  case 7:
   {
   Days = 222;
   break;
   }
  case 8:
   {
   Days = 253;
   break;
   }
  case 9:
   {
   Days = 283;
   break;
   }
  case 10:
   {
   Days = 314;
   break;
   }
  case 11:
   {
   Days = 344;
   break;
   }
  } /* switch */

 Days += SunDate;
 if (Days > 365)
  Days -= 365;

 SunLat = -23.0 * cos((360.0 * Days / 365.0) * PiOver180); 

 RefLon = atof(RefLonStr);
 
 SunTime = Hour + (double)Mins / 60.0;
 SunTime -= 12.0;

 SunLon = RefLon + 180.0 * SunTime / 12.0;

 sprintf(str, "%f", SunLat);
 set(TS_Win->SunLatStr, MUIA_String_Contents, str); 
 sprintf(str, "%f", SunLon);
 set(TS_Win->SunLonStr, MUIA_String_Contents, str); 

 PAR_FIRST_MOTION(15) = SunLat;
 PAR_FIRST_MOTION(16) = SunLon;

 Update_EM_Item();
 Set_Radial_Txt(2);

} /* Set_TS_Position() */

/************************************************************************/

STATIC_FCN void Set_TS_Reverse(short reverse) // used locally only -> static, AF 25.7.2021
{
char *RefLonStr;
short Hour, Mins, Days;
long SunDate, SunMonth, AMPM = 0;
double FloatDays, RefLon, SunTime;

 get(TS_Win->LonStr, MUIA_String_Contents, &RefLonStr);

 if (PAR_FIRST_MOTION(15) > 23.0)
  {
  SunMonth = 5;
  SunDate = 20;
  }
 else if (PAR_FIRST_MOTION(15) < -23)
  {
  SunMonth = 11;
  SunDate = 20;
  }
 else
  {
  FloatDays = .5 + acos(-(PAR_FIRST_MOTION(15) / 23.0)) * PiUnder180 * 365.0 / 360.0; 
  if (reverse)
   {
   get(TS_Win->MonthCycle, MUIA_Cycle_Active, &SunMonth);
   get(TS_Win->DateCycle, MUIA_Cycle_Active, &SunDate);
   if ((SunMonth == 11 && SunDate > 20) || (SunMonth == 5 && SunDate < 20)
	|| (SunMonth < 5))
    FloatDays = 366.0 - FloatDays;
   }
  Days = FloatDays;
  if (Days < 10)
   {
   SunMonth = 11;
   SunDate = 21 + Days;
   }
  else if (Days < 41)
   {
   SunMonth = 0;
   SunDate = Days - 10;
   }
  else if (Days < 69)
   {
   SunMonth = 1;
   SunDate = Days - 41;
   }
  else if (Days < 100)
   {
   SunMonth = 2;
   SunDate = Days - 69;
   }
  else if (Days < 130)
   {
   SunMonth = 3;
   SunDate = Days - 100;
   }
  else if (Days < 161)
   {
   SunMonth = 4;
   SunDate = Days - 130;
   }
  else if (Days < 191)
   {
   SunMonth = 5;
   SunDate = Days - 161;
   }
  else if (Days < 222)
   {
   SunMonth = 6;
   SunDate = Days - 191;
   }
  else if (Days < 253)
   {
   SunMonth = 7;
   SunDate = Days - 222;
   }
  else if (Days < 283)
   {
   SunMonth = 8;
   SunDate = Days - 253;
   }
  else if (Days < 314)
   {
   SunMonth = 9;
   SunDate = Days - 283;
   }
  else if (Days < 344)
   {
   SunMonth = 10;
   SunDate = Days - 314;
   }
  else
   {
   SunMonth = 11;
   SunDate = Days - 344;
   } /* else */
  } /* else */

 RefLon = atof(RefLonStr);

 SunTime = 12.0 + (PAR_FIRST_MOTION(16) - RefLon) * 12.0 / 180.0;
 while (SunTime < 0.0)
  SunTime += 24.0;
 while (SunTime > 24.0)
  SunTime -= 24.0;
 Hour = SunTime;
 Mins = (SunTime - Hour) * 60;
 if (Hour >= 12.0)
  {
  Hour -= 12.0;
  AMPM = 1;
  } /* if */

 sprintf(str, "%1d:%02d", Hour, Mins);
 set(TS_Win->MonthCycle, MUIA_Cycle_Active, SunMonth);
 set(TS_Win->DateCycle, MUIA_Cycle_Active, SunDate);
 set(TS_Win->AMPMCycle, MUIA_Cycle_Active, AMPM);
 set(TS_Win->TimeStr, MUIA_String_Contents, str);

 sprintf(str, "%f", PAR_FIRST_MOTION(15));
 set(TS_Win->SunLatStr, MUIA_String_Contents, str); 
 sprintf(str, "%f", PAR_FIRST_MOTION(16));
 set(TS_Win->SunLonStr, MUIA_String_Contents, str); 
 
} /* Set_TS_Reverse() */

/************************************************************************/

STATIC_FCN void Make_PN_Window(void) // used locally only -> static, AF 25.7.2021
{
 long open, i;

 if (PN_Win)
  {
  DoMethod(PN_Win->NewProjWin, MUIM_Window_ToFront);
  set(PN_Win->NewProjWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((PN_Win = (struct NewProjectWindow *)
	get_Memory(sizeof (struct NewProjectWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     PN_Win->NewProjWin = WindowObject,
      MUIA_Window_Title		, "New Project",
      MUIA_Window_ID		, MakeID('P','R','O','N'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, Label2("New Project"),
	    Child, PN_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234567890",
		MUIA_String_Contents, "WCSProjects:", End,
	    Child, PN_Win->BT_Get[0] = ImageButton(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2("Clone Project"),
	    Child, PN_Win->Str[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234567890",
		MUIA_String_Contents, "WCSProjects:", End,
	    Child, PN_Win->BT_Get[1] = ImageButton(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, RectangleObject, End,
            Child, PN_Win->BT_Save = KeyButtonFunc('s', "\33cSave"), 
            Child, PN_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"), 
	    Child, RectangleObject, End,
            End, /* HGroup */
	  End, /* VGroup */
	End; /* Window object */

  if (! PN_Win->NewProjWin)
   {
   Close_PN_Window(0);
   User_Message((CONST_STRPTR)"Project: New/Edit", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, PN_Win->NewProjWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(PN_Win->NewProjWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PN_CLOSEQUERY);  

  for (i=0; i<2; i++)
   MUI_DoNotiPresFal(app, PN_Win->BT_Get[i], ID_PN_GET(i), NULL);


  MUI_DoNotiPresFal(app, 
   PN_Win->BT_Save, ID_PN_SAVE,
   PN_Win->BT_Cancel, ID_PN_CLOSE, NULL);

/* Set tab cycle chain */
  DoMethod(PN_Win->NewProjWin, MUIM_Window_SetCycleChain,
	PN_Win->Str[0], PN_Win->BT_Get[0], PN_Win->Str[1], PN_Win->BT_Get[1],
	PN_Win->BT_Save, PN_Win->BT_Cancel, NULL);

/* return cycle */
  DoMethod(PN_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PN_Win->NewProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PN_Win->Str[1]);
  DoMethod(PN_Win->Str[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PN_Win->NewProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PN_Win->Str[0]);

/* Set active gadget */
  set(PN_Win->NewProjWin, MUIA_Window_ActiveObject, PN_Win->Str[0]);

/* Open window */
  set(PN_Win->NewProjWin, MUIA_Window_Open, TRUE);
  get(PN_Win->NewProjWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_PN_Window(0);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(PN_Win->NewProjWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_PN_ACTIVATE);

} /* Make_PN_Window() */


/**********************************************************************/

void Close_PN_Window(short Apply)
{
char *NewProjName, *CloneProjName;

 if (PN_Win)
  {
  if (Apply)
   {
   get(PN_Win->Str[0], MUIA_String_Contents, &NewProjName);
   get(PN_Win->Str[1], MUIA_String_Contents, &CloneProjName);
   if (! CreateNewProject(NewProjName, CloneProjName))
    return;
   } /* if */
  if (PN_Win->NewProjWin)
   {
   set(PN_Win->NewProjWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, PN_Win->NewProjWin);
   MUI_DisposeObject(PN_Win->NewProjWin);
   } /* if window created */
  free_Memory(PN_Win, sizeof (struct NewProjectWindow));
  PN_Win = NULL;
  } /* if memory allocated */

} /* Close_PN_Window() */

/**********************************************************************/

void Handle_PN_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_PN_Window();
   return;
   } /* Open Project Window */

  if (! PN_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Project window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_PN_SAVE:
      {
      Close_PN_Window(1);
      break;
      }
     case ID_PN_CLOSEQUERY:
     case ID_PN_CLOSE:
      {
      Close_PN_Window(0);
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    short i;
    char *Name, filename[256], dummydrive[32], dummypath[256], dummyfile[32], dummyext[32],
	Ptrn[32];

    strcpy(Ptrn, "#?.proj");
    i = WCS_ID - ID_PN_GET(0);
    switch (i)
     {
     case 0:
      {
      get(PN_Win->Str[0], MUIA_String_Contents, &Name);
      strsfn(Name, dummydrive, dummypath, dummyfile, dummyext);
      strcpy(filename, dummydrive);
      strcat(filename, dummypath);
      if (dummyext[0])
       {
       strcat(dummyfile, ".");
       strcat(dummyfile, dummyext);
       }
      getfilenameptrn(1, "New Project Path/Name", filename, dummyfile, Ptrn);
      strmfp(filename, filename, dummyfile);
      set(PN_Win->Str[0], MUIA_String_Contents, filename);
      break;
      } /* new project */
     case 1:
      {
      get(PN_Win->Str[1], MUIA_String_Contents, &Name);
      strsfn(Name, dummydrive, dummypath, dummyfile, dummyext);
      strcpy(filename, dummydrive);
      strcat(filename, dummypath);
      if (dummyext[0])
       {
       strcat(dummyfile, ".");
       strcat(dummyfile, dummyext);
       }
      getfilenameptrn(0, "Clone Project", filename, dummyfile, Ptrn);
      strmfp(filename, filename, dummyfile);
      set(PN_Win->Str[1], MUIA_String_Contents, filename);
      break;
      } /* clone project */
     } /* switch i */
    break;
    } /* get path buttons */
   } /* switch */

} /* Handle_PN_Window() */

/***********************************************************************/

STATIC_FCN short CreateNewProject(char *NewProjName, char *CloneProjName) // used locally only -> static, AF 25.7.2021
{
char filename[256], *NameBlock,
	*NewDrive, *NewPath, *NewName, *NewExt,
	*CloneName,
	*NewDBasePath, *NewDBaseName, *NewDefPath, *Str;
struct WaveData *WD = NULL;
struct CloudData *CD = NULL;
short error = 0;

 if ((NameBlock = (char *)get_Memory(2000, MEMF_CLEAR)) == NULL)
  {
  error = 20;
  goto EndNewProj;
  } /* if no memory */

 NewDrive     = &NameBlock[0];
 NewPath      = &NameBlock[32];
 NewName      = &NameBlock[288];
 NewExt       = &NameBlock[320];
 CloneName    = &NameBlock[352];
 NewDBasePath = &NameBlock[384];
 NewDBaseName = &NameBlock[640];
 NewDefPath   = &NameBlock[672];
 Str          = &NameBlock[928];

 strsfn(NewProjName, NewDrive, NewPath, NewName, NewExt);

 if (NewName[0] == 0)
  {
  error = 1;
  goto EndNewProj;
  } /* if no name */

 strsfn(CloneProjName, NULL, NULL, CloneName, NULL);
 if (CloneName[0])
  {
  if (! LoadProject(CloneProjName, NULL, 1))
   {
   error = 2;
   goto EndNewProj;
   } /* if can't load clone project */
  if (wavefile[0])
   {
   strmfp(filename, wavepath, wavefile);
   if (! Wave_Load(filename, &WD))
    {
    error = 3;
    goto EndNewProj;
    } /* if no wave file */
   } /* if wave file name */
  if (cloudfile[0])
   {
   strmfp(filename, cloudpath, cloudfile);
   if (! Cloud_Load(filename, &CD))
    {
    error = 4;
    goto EndNewProj;
    } /* if no cloud file */
   } /* if cloud file name */
  } /* clone file exists - try and load it */

 strcpy(filename, NewDrive);
 strcat(filename, NewPath);
 if (chdir(filename))
  {
  if (Mkdir(filename))
   {
   error = 5;
   goto EndNewProj;
   } /* if create dir failed */
  } /* if new path doesn't exist */

 if (strcmp(NewExt, "proj") && NewExt[0])
  {
  strcat(NewName, ".");
  strcat(NewName, NewExt);
  } /* if some other extension given */

 strmfp(filename, filename, NewName);
 if (Mkdir(filename))
  {
  error = 6;
  goto EndNewProj;
  } /* if makedir failed */
 strcpy(NewDBasePath, filename);
 strcpy(NewDBaseName, NewName);

 strmfp(filename, filename, NewName);
 strcat(filename, ".object");

 if (Mkdir(filename))
  {
  error = 7;
  goto EndNewProj;
  } /* if makedir failed */
 strcpy(NewDefPath, filename);

 strcpy(projectpath, NewDrive);
 strcat(projectpath, NewPath);
 strcpy(projectname, NewName);
 strcat(projectname, ".proj");
 strcpy(dbasepath, NewDBasePath);
 strcpy(dbasename, NewDBaseName);
 strcpy(parampath, NewDefPath);
 strcpy(cloudpath, NewDefPath);
 strcpy(wavepath, NewDefPath);
 strcpy(framefile, NewName);
 strcpy(graphname, NewName);
 strcpy(linefile, NewName);
 strcpy(dirname, NewDefPath);
 if (DL)
  DirList_Add(DL, NewDefPath, 0);
 else
  DL = DirList_New(NewDefPath, 0);

 if (! CloneName[0])
  {
  if (DL)
   DirList_Del(DL);
  DL = DirList_New(NewDefPath, 0);
  paramfile[0] = 0;
  strcat(linefile, "Vec");
  strcpy(zbufferpath, NewDefPath);
  zbufferfile[0] = 0;
  strcpy(backgroundpath, NewDefPath);
  backgroundfile[0] = 0;
  statname[0] = 0;
  strcpy(colormappath, NewDefPath);
  colormapfile[0] = 0;
  strcpy(modelpath, "WCSProjects:EcoModels");
  cloudfile[0] = 0;
  wavefile[0] = 0;
  strcpy(deformpath, NewDefPath);
  deformfile[0] = 0;
  strcpy(imagepath, "WCSProjects:EcoModels");
  strcpy(sunfile, "Sun.iff8");
  strcpy(moonfile, "Moon.iff8");
  paramsloaded = 0;
  dbaseloaded = makedbase(0);
  } /* if no cloning */

 if (! SaveProject(2, NULL, NULL))
  {
  error = 8;
  goto EndNewProj;
  } /* if error saving project */

 if (WD)
  {
  strmfp(filename, wavepath, wavefile);
  if (! Wave_Save(filename, WD))
   {
   error = 9;
   goto EndNewProj;
   } /* if error saving wave file */
  } /* if wave data */
 if (CD)
  {
  strmfp(filename, cloudpath, cloudfile);
  if (! Cloud_Save(filename, CD))
   {
   error = 10;
   goto EndNewProj;
   } /* if error saving cloud file */
  } /* if cloud data */

EndNewProj:
 if (NameBlock)
  free_Memory(NameBlock, 2000);
 if (WD)
  WaveData_Del(WD);
 if (CD)
  CloudData_Del(CD);
 chdir (path);	/* restore global program pointer */


 if (error)
  {
  switch (error)
   {
   case 1:
    {
    strcpy(str, "You must supply a new project name.");
    break;
    } /* no new name */
   case 2:
    {
    strcpy(str, "Error loading Project file to clone.");
    break;
    } /* loading clone project */
   case 3:
    {
    strcpy(str, "Error loading Wave file to clone.");
    break;
    } /* loading wave */
   case 4:
    {
    strcpy(str, "Error loading Cloud file to clone.");
    break;
    } /* loading cloud */
   case 5:
    {
    sprintf(Str, "Error creating new Project Directory: %s.\
 It may already exist or there may be a file with that name.", filename);
    break;
    } /* creating project directory */
   case 6:
    {
    sprintf(Str, "Error creating new Database Directory: %s.\
 It may already exist or there may be a file with that name.", filename);
    break;
    } /* creating database directory */
   case 7:
    {
    sprintf(Str, "Error creating new Default Directory: %s.\
 It may already exist or there may be a file with that name.", filename);
    break;
    } /* creating default directory */
   case 8:
    {
    strcpy(str, "Error saving the new Project file.");
    break;
    } /* saving project */
   case 9:
    {
    strcpy(str, "Error saving the cloned Wave file.");
    break;
    } /* saving wave */
   case 10:
    {
    strcpy(str, "Error saving the cloned Cloud file.");
    break;
    } /* saving cloud */
   case 20:
    {
    strcpy(str, "Out of memory.");
    break;
    } /* allocating char array */
   } /* switch error */
  User_Message_Def((CONST_STRPTR)"New Project", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
  return (0);
  } /* if an error occurred */

 return (1);

} /* CreateNewProject() */

