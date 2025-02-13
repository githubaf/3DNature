/* GUI.c (ne gisgui.c 14 Jan 1994 CXH)
** World Construction Set GUI.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include <time.h>
#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Version.h"
#include <stdarg.h>

#define WCS_MUI_2_HACK
#define MENU_STOP 35

#ifdef WCS_MUI_2_HACK
STATIC_FCN void MUI2_MenuCheck_Hack(void); // used locally only -> static, AF 25.7.2021
#endif /* WCS_MUI_2_HACK */

extern struct WaveWindow *WV_Win;

extern char* LocaleExtCreditText; // in allocated iand intialized in WCS.c

/*STATIC_FCN*/ void Handle_RN_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Handle_DB_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Close_DB_Window(void); // used locally only -> static, AF 25.7.2021
void Close_EP_Window(void); // needed in WCS.c for automatic testing...
STATIC_FCN short Handle_APP_Windows(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
STATIC_FCN void NoMod_Message(STRPTR mod); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Close_Log_Window(int StayClosed); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Close_DO_Window(void); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Handle_EP_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
STATIC_FCN void InfoWin_Update(int flush); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Handle_DO_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Status_Log(STRPTR logtext, int Severity); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Make_Log_Window(int Severity); // used locally only -> static, AF 26.7.2021

//// SAS/C and deadw00d's gcc for AROS x86_64/i386 do not have asprintf()
//#if defined __SASC || defined __AROS__
//// chatgpt suggested the following:
//int asprintf(char **strp, const char *fmt, ...) {
//    va_list args;
//    va_list args_copy;
//    int len;
//
//    // Initialize the variable argument list
//    va_start(args, fmt);
//
//    // Copy the argument list to measure the length
//    va_copy(args_copy, args);
//    len = vsnprintf(NULL, 0, fmt, args_copy);
//    va_end(args_copy);
//
//    if (len < 0) {
//        va_end(args);
//        return -1;
//    }
//
//    // Allocate memory for the resulting string
//    *strp = (char *)malloc(len + 1);
//    if (!*strp) {
//        va_end(args);
//        return -1;
//    }
//
//    // Format the string
//    len = vsnprintf(*strp, len + 1, fmt, args);
//    va_end(args);
//
//    if (len < 0) {
//        free(*strp);
//        return -1;
//    }
//
//    return len;
//}
//
//#endif

void Make_EP_Window(short hor_win)
{
 long open;
 static const char *LayoutCycle[3];
 LayoutCycle[0]= (char*)GetString( MSG_AGUI_STANDUP );  // "Stand Up"
 LayoutCycle[1]= (char*)GetString( MSG_AGUI_LAYDOWN );  // "Lay Down"
 LayoutCycle[2]= NULL;

 if (EP_Win)
  {
  DoMethod(EP_Win->EditWindow, MUIM_Window_ToFront);
  set(EP_Win->EditWindow, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((EP_Win = (struct EditModWindow *)
	get_Memory(sizeof (struct EditModWindow), MEMF_CLEAR)) == NULL)
   return;

 EP_Win->Layout = hor_win;

 Set_Param_Menu(10);

      EP_Win->EditWindow = WindowObject,
      MUIA_Window_Title		,  GetString( MSG_AGUI_PARAMETERMODULE ) ,  /* "End" == "TAG_DONE)" */  // "Parameter Module"
      MUIA_Window_ID		, (hor_win ? MakeID('E','P','A','H'): MakeID('E','P','A','V')),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_LeftEdge	, MUIV_Window_LeftEdge_Moused,
      MUIA_Window_TopEdge	, MUIV_Window_TopEdge_Moused,

      WindowContents, ColGroup(hor_win * 3 + 1),
	Child, EP_Win->CY_Layout = CycleObject,
        MUIA_Cycle_Entries, LayoutCycle, End,
        Child, EP_Win->BT_EdMoPar = KeyButtonFunc('m',    (char*)GetString( MSG_AGUI_MOTION ) ),       // "\33l Motion     "
        Child, EP_Win->BT_EdCoPar = KeyButtonFunc('c',    (char*)GetString( MSG_AGUI_COLOR ) ),        // "\33l Color      "
        Child, EP_Win->BT_EdEcoPar = KeyButtonFunc('e',   (char*)GetString( MSG_AGUI_ECOSYSTEM ) ),    // "\33l Ecosystem  "
        Child, EP_Win->BT_EdClouds = KeyButtonFunc('l',   (char*)GetString( MSG_AGUI_CLOUDS ) ),       // "\33l Clouds     "
        Child, EP_Win->BT_EdWaves  = KeyButtonFunc('w',   (char*)GetString( MSG_AGUI_WAVES ) ),        // "\33l Waves      "
        Child, EP_Win->BT_Defaults = KeyButtonFunc('d',   (char*)GetString( MSG_AGUI_DEFAULTS ) ),     // "\33l Defaults   "
/*        Child, EP_Win->BT_ExportMo = KeyButtonFunc('i',   "\33l Motion I/O "),*/
        End, /* VGroup */
      End; /* EP_win->EditWindow */

  if (! EP_Win->EditWindow)
   {
   Close_EP_Window();
   User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Parameters Module", "Out of memory!", "OK", 
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EP_Win->EditWindow);

/* Close requests */
  DoMethod(EP_Win->EditWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EP_QUIT);
#ifdef XENON_DOESNT_LIKE_THIS
  MUI_DoNotifyPressed(EP_Win->BT_Quit, ID_EP_QUIT);
#endif /* XENON_DOESNT_LIKE_THIS */

/* Set Application notification events */
  DoMethod(EP_Win->CY_Layout, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EP_LAYOUT);
  MUI_DoNotiPresFal(app, EP_Win->BT_EdMoPar, ID_EM_WINDOW,
   EP_Win->BT_EdCoPar, ID_EC_WINDOW, EP_Win->BT_EdEcoPar, ID_EE_WINDOW,
   EP_Win->BT_Defaults, ID_EP_DEFAULTS, EP_Win->BT_EdClouds, ID_CL_WINDOW,
   EP_Win->BT_EdWaves, ID_WV_WINDOW(0),
   /*EP_Win->BT_ExportMo, ID_LW_WINDOW, */NULL);

/* Set tab cycle chain */
  DoMethod(EP_Win->EditWindow, MUIM_Window_SetCycleChain,
	EP_Win->CY_Layout, EP_Win->BT_EdMoPar,
	EP_Win->BT_EdCoPar, EP_Win->BT_EdEcoPar,
	EP_Win->BT_EdClouds, EP_Win->BT_EdWaves, EP_Win->BT_Defaults,
	/*EP_Win->BT_ExportMo, */NULL);

/* set active cycle gadget entry */
  set(EP_Win->CY_Layout, MUIA_Cycle_Active, hor_win);

/* Set active gadget */
  set(EP_Win->EditWindow, MUIA_Window_ActiveObject, (IPTR)EP_Win->BT_EdMoPar);

/* Open window */
  set(EP_Win->EditWindow, MUIA_Window_Open, TRUE);
  get(EP_Win->EditWindow, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EP_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EP_Win->EditWindow, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EP_ACTIVATE);

} /* Make_EP_Window(void) */

/************************************************************************/

void Close_EP_Window(void) // needed in WCS.c for automatic testing, so no longer static AF, 13.Feb.2025
{
 if (EP_Win)
  {
  if (EP_Win->EditWindow)
   {
   set(EP_Win->EditWindow, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, EP_Win->EditWindow);
   MUI_DisposeObject(EP_Win->EditWindow);
   } /* if */
  free_Memory(EP_Win, sizeof (struct EditModWindow));
  EP_Win = NULL;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
  } /* if */
} /* Close_EP_Window() */

/************************************************************************/

STATIC_FCN void Handle_EP_Window(ULONG WCS_ID) // used locally only -> static, AF 25.7.2021
{
 short i;

  switch (WCS_ID & 0x00ff0000)
   {
   case WI_WINDOW0:
    {
    switch (WCS_ID & 0x0000ff00)
     {
     case GP_OPEN_WINDOW:
      {

      if(WCS_ID & GA_GADNUM(1))
      	{ /* came from menu item, not gadget */
      	if(EP_Win)
      		{ /* it's already open, close it */
      		Close_EP_Window();
      		} /* if */
      	else
      		{ /* not open yet, open it */
      		Make_EP_Window(0);
      		} /* else */
      	} /* if */
      else
      	{ /* came from gadget */
	      Make_EP_Window(0);
	      } /* else */

	   /* Set menu checkmark state based on whether window is open or not */
	   DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_EDITING), EP_Win);
	   
#ifdef WCS_MUI_2_HACK
/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

      break;
      } /* Open Editing Master window */

     case GP_ACTIVEWIN:
      {
/*      DoMethod(EP_Win->EditWindow, MUIM_Window_ToFront);*/
      LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
      break;
      } /* Activate Editing Module window */

     case GP_CYCLE1:
      {
      switch (WCS_ID)
       {
       case ID_EP_LAYOUT:
        {
        LONG selected;
        get(EP_Win->CY_Layout, MUIA_Cycle_Active, &selected);
        if (selected != EP_Win->Layout)
         {
         Close_EP_Window();
         Make_EP_Window(selected);

         /* Set menu checkmark state based on whether window is open or not */
         DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_EDITING), EP_Win);

#ifdef WCS_MUI_2_HACK
/*		   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

         } /* if change layout */
        break;
        } /* change window orientation */
       } /* switch Gadget ID */
      break;
      } /* CYCLE1 */

     case GP_BUTTONS1:
      {
      switch (WCS_ID)
       {
       case ID_EP_DEFAULTS:
        {
        if (dbaseloaded)
         {
         sprintf(str, (char*)GetString( MSG_AGUI_CREATEDEFAULTPARAMETERSFORDATABASEALLCURRENTPARAMETERS ) , dbasename);  // "Create Default Parameters for Database %s? All current Parameters will be overwritten."
         if (User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) , (CONST_STRPTR)str, (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1))  // "Parameter Editing: Defaults", str, "OK|Cancel"
          {
          paramsloaded = DefaultParams();
          if (paramsloaded)
           {
	   strcpy(paramfile, "Default.par");
           for (i=0; i<2; i++)
            {
            FixPar(i, 0x1111);
            } /* for */
           if (EC_Win)
            {
            UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
            DisableKeyButtons(1);
            GetKeyTableValues(1, EC_Win->PalItem, 1);
            SetAllColorRequester();
            Set_EC_Item(EC_Win->PalItem);
            } /* if color requester open */
           if (EMIA_Win) Init_IA_View(1);
           if (EM_Win)
            {
            UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
            DisableKeyButtons(0);
            GetKeyTableValues(0, EM_Win->MoItem, 1);
            Set_EM_Item(EM_Win->MoItem);
            Set_Radial_Txt(0);
  	    } /* if motion requester open */
           if (EE_Win)
            {
            UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
            DisableKeyButtons(2);
            GetKeyTableValues(2, EE_Win->EcoItem, 1);
            Set_EE_List(1);
            Set_EE_Item(EE_Win->EcoItem);
	    } /* if motion requester open */
           if (ES_Win)
            {
            Set_ES_Window();
	    } /* if settings window open */
           } /* else if */
	  } /* if paramsloaded */
	 } /* if database loaded */
        else
         {
         User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) ,  // "Parameter Editing: Defaults"
        		 (CONST_STRPTR) GetString( MSG_AGUI_YOUMUSTFIRSTLOADADATABASEBEFOREDEFAULTPARAMETERSCANBEC ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "You must first load a Database before Default Parameters can be computed.", "OK"
	 } /* else */
        break;
	} /* create default parameters */
       case ID_EP_SAVE:
        {
        saveparams(0x1111, -1, 0);
        break;
	} /* Save complete parameter file */
       case ID_EP_LOAD:
        {
        short loadstatus;
        if ((loadstatus = loadparams(0x1111, -1)) == -1) paramsloaded = 0;
        else if (loadstatus == 1)
         {
         paramsloaded = 1;
         FixPar(0, 0x1111);
         FixPar(1, 0x1111);
         if (EC_Win)
          {
          UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
          DisableKeyButtons(1);
          GetKeyTableValues(1, EC_Win->PalItem, 1);
          SetAllColorRequester();
          Set_EC_List(1);
          Set_EC_Item(EC_Win->PalItem);
          } /* if color requester open */
         if (EM_Win)
          {
          UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
          DisableKeyButtons(0);
          GetKeyTableValues(0, EM_Win->MoItem, 1);
          Set_EM_List(1);
          Set_EM_Item(EM_Win->MoItem);
          Set_Radial_Txt(0);
	  } /* if motion requester open */
         if (EMIA_Win) Init_IA_View(1);
         if (EE_Win)
          {
          UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
          DisableKeyButtons(2);
          GetKeyTableValues(2, EE_Win->EcoItem, 1);
          Set_EE_List(1);
          Set_EE_Item(EE_Win->EcoItem);
	  } /* if motion requester open */
         if (ES_Win)
          {
          Set_ES_Window();
	  } /* if settings window open */
         ResetTimeLines(-1);
         } /* else if */
        break;
        } /* Load complete parameter file */
       case ID_EP_FIX:
        {
        FixPar(1, 0x1111);
        break;
	} /* Fix all parameter sets' values for later undo */
       case ID_EP_UNDO:
        {
        UndoPar(1, 0x1111);
        if (EC_Win)
         {
         UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
         DisableKeyButtons(1);
         GetKeyTableValues(1, EC_Win->PalItem, 1);
         Set_EC_List(1);
         SetAllColorRequester();
         Set_EC_Item(EC_Win->PalItem);
         } /* if color requester open */
        if (EM_Win)
         {
         UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
         DisableKeyButtons(0);
         GetKeyTableValues(0, EM_Win->MoItem, 1);
         Set_EM_List(1);
         Set_EM_Item(EM_Win->MoItem);
         Set_Radial_Txt(0);
	 } /* if motion requester open */
        if (EMIA_Win)
         {
         IA->recompute = 1;
         drawgridview();
         } /* if interactive window open */
        if (EE_Win)
         {
         UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
         DisableKeyButtons(2);
         GetKeyTableValues(2, EE_Win->EcoItem, 1);
         Set_EE_List(1);
         Set_EE_Item(EE_Win->EcoItem);
	 } /* if motion requester open */
        ResetTimeLines(-1);
        break;
	} /* Undo recent changes to all parameter sets */ 
       case ID_EP_QUIT:
        {
        Close_EP_Window();
        /* Set menu checkmark state based on whether window is open or not */
        DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_EDITING), EP_Win);
        break;
	} /* close window */
       } /* switch gadget ID */
      break;
      } /* BUTTONS1 */
     } /* switch gadget group */
    break;
    } /* Edit Module Window */
   case WI_WINDOW1:
    {
    Handle_EM_Window(WCS_ID);
    break;
    } /* Edit Motion Window */
   case WI_WINDOW2:
    {
    Handle_EC_Window(WCS_ID);
    break;
    } /* Edit Color Window */
   case WI_WINDOW3:
    {
    Handle_EE_Window(WCS_ID);
    break;
    } /* Edit Ecosystem Window */
   case WI_WINDOW4:
    {
    Handle_ES_Window(WCS_ID);
    break;
    } /* Edit Render Settings */
   case WI_WINDOW6:
    {
    Handle_ECTL_Window(WCS_ID);
    break;
    } /* Color Time Lines Window */
   case WI_WINDOW7:
    {
    Handle_EMTL_Window(WCS_ID);
    break;
    } /* Motion Time Lines Window */
   case WI_WINDOW8:
    {
    Handle_EMIA_Window(WCS_ID);
    break;
    } /* Interactive Motion Window */
   case WI_WINDOW9:
    {
    Handle_EETL_Window(WCS_ID);
    break;
    } /* Interactive Motion Window */
   case WI_WINDOW10:
    {
    Handle_Diagnostic_Window(WCS_ID);
    break;
    } /* Interactive Motion Window */
   case WI_WINDOW11:
    {
    Handle_EMPL_Window(WCS_ID);
    break;
    } /* Motion Parameter List Window */
   case WI_WINDOW12:
    {
    Handle_PS_Window(WCS_ID);
    break;
    } /* Scale Keys Window */
   case WI_WINDOW13:
    {
    Handle_LW_Window(WCS_ID);
    break;
    } /* LightWave IO Window */
   case WI_WINDOW14:
    {
    Handle_FM_Window(WCS_ID);
    break;
    } /* Ecosystem Model Window */
   case WI_WINDOW15:
    {
    Handle_AN_Window(WCS_ID);
    break;
    } /* Anim Control Window */
   case WI_WINDOW16:
    {
    Handle_EL_Window(WCS_ID);
    break;
    } /* Ecosystem Legend Window */
   case WI_WINDOW17:
    {
    Handle_SC_Window(WCS_ID);
    break;
    } /*  Image Scale Window */
   case WI_WINDOW18:
    {
    Handle_TS_Window(WCS_ID);
    break;
    } /*  Sun Time Window */
   case WI_WINDOW19:
    {
    Handle_CL_Window(WCS_ID);
    break;
    } /* Cloud Editor Window */
   case WI_WINDOW20:
    {
    Handle_SB_Buttons(WCS_ID);
    break;
    } /* Various Editor Option Buttons */
   case WI_WINDOW21:
    {
    Handle_FE_Window(WCS_ID);
    break;
    } /* Foliage Editor Window */
   } /* switch Window ID */

} /* Handle_EP_Window() */

/************************************************************************/

void Make_DB_Window(short hor_win)
{
 long open;
 static const char *LayoutCycle[3];
 static int Init = TRUE;

 if(Init)
 {
	 Init = FALSE;
	 LayoutCycle[0]= (char*)GetString( MSG_AGUI_STANDUP );  // "Stand Up"
	 LayoutCycle[1]= (char*)GetString( MSG_AGUI_LAYDOWN );  // "Lay Down"
	 LayoutCycle[2]= NULL;
 }

 if (DB_Win)
  {
  DoMethod(DB_Win->DatabaseWindow, MUIM_Window_ToFront);
  set(DB_Win->DatabaseWindow, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((DB_Win = (struct DatabaseModWindow *)
	get_Memory(sizeof (struct DatabaseModWindow), MEMF_CLEAR)) == NULL)
   return;

 DB_Win->Layout = hor_win;

 Set_Param_Menu(10);

     DB_Win->DatabaseWindow = WindowObject,
      MUIA_Window_Title		,  GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
      MUIA_Window_ID		, (hor_win ? MakeID('D','B','A','H'): MakeID('D','B','A','V')),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_LeftEdge	, MUIV_Window_LeftEdge_Moused,
      MUIA_Window_TopEdge	, MUIV_Window_TopEdge_Moused,

      WindowContents, ColGroup(hor_win * 3 + 1),
	Child, DB_Win->CY_Layout = CycleObject,
		 MUIA_Cycle_Entries, LayoutCycle, End,
        Child, DB_Win->BT_Load = KeyButtonFunc('l',    (char*)GetString( MSG_AGUI_LOAD ) ),    // "\33l Load     "
        Child, DB_Win->BT_Append = KeyButtonFunc('a',  (char*)GetString( MSG_AGUI_APPEND ) ),  // "\33l Append   "
        Child, DB_Win->BT_Create = KeyButtonFunc('c',  (char*)GetString( MSG_AGUI_CREATE ) ),  // "\33l Create   "
        Child, DB_Win->BT_Edit = KeyButtonFunc('e',    (char*)GetString( MSG_AGUI_EDIT ) ),    // "\33l Edit     "
        Child, DB_Win->BT_SaveAs = KeyButtonFunc('s',  (char*)GetString( MSG_AGUI_SAVE ) ),    // "\33l Save     "
        Child, DB_Win->BT_DirList = KeyButtonFunc('d', (char*)GetString( MSG_AGUI_DIRLIST ) ), // "\33l Dir List "
        Child, RectangleObject, MUIA_FixHeight, 0, End,
        End, /* VGroup */
      End; /* DB_win->EditWindow */

  if (! DB_Win->DatabaseWindow)
   {
   Close_DB_Window();
   User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Database Module", "Out of memory!", "OK"
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DB_Win->DatabaseWindow);

/* ReturnIDs */
  DoMethod(DB_Win->DatabaseWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DB_QUIT);
  DoMethod(DB_Win->CY_Layout, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DB_LAYOUT);
  
  MUI_DoNotiPresFal(app, 
   DB_Win->BT_Load, ID_DB_LOAD, DB_Win->BT_Append, ID_DB_APPEND,
   DB_Win->BT_Create, ID_DB_CREATE, DB_Win->BT_Edit, ID_DE_WINDOW,
   DB_Win->BT_SaveAs, ID_DB_SAVE,
   DB_Win->BT_DirList, ID_DL_WINDOW, NULL);

/* Set tab cycle chain */
  DoMethod(DB_Win->DatabaseWindow, MUIM_Window_SetCycleChain,
	DB_Win->CY_Layout, DB_Win->BT_Load,
	DB_Win->BT_Append, DB_Win->BT_Create, DB_Win->BT_Edit,
	DB_Win->BT_SaveAs,
	DB_Win->BT_DirList, NULL);

/* Set active cycle gadget entry */
  set(DB_Win->CY_Layout, MUIA_Cycle_Active, hor_win);

/* Set active gadget */
  set(DB_Win->DatabaseWindow, MUIA_Window_ActiveObject,
		(IPTR)(dbaseloaded ? DB_Win->BT_Edit: DB_Win->BT_Load));

/* Open window */
  set(DB_Win->DatabaseWindow, MUIA_Window_Open, TRUE);
  get(DB_Win->DatabaseWindow, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DB_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DB_Win->DatabaseWindow, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DB_ACTIVATE);

} /* Make_DB_Window(void) */

/************************************************************************/

STATIC_FCN void Close_DB_Window(void) // used locally only -> static, AF 25.7.2021
{
 if (DB_Win)
  {
  if (DB_Win->DatabaseWindow)
   {
   set(DB_Win->DatabaseWindow, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, DB_Win->DatabaseWindow);
   MUI_DisposeObject(DB_Win->DatabaseWindow);
   } /* if */
  free_Memory(DB_Win, sizeof (struct DatabaseModWindow));
  DB_Win = NULL;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
  } /* if */
} /* Close_DB_Window() */

/************************************************************************/

STATIC_FCN void Handle_DB_Window(ULONG WCS_ID) // used locally only -> static, AF 25.7.2021
{

  switch (WCS_ID & 0x00ff0000)
   {
   case WI_WINDOW0:
    {
    switch (WCS_ID & 0x0000ff00)
     {
     case GP_OPEN_WINDOW:
      {
      if(WCS_ID & GA_GADNUM(1))
      	{ /* came from menu item, not gadget */
      	if(DB_Win)
      		{ /* it's already open, close it */
      		Close_DB_Window();
      		} /* if */
      	else
      		{ /* not open yet, open it */
      		Make_DB_Window(0);
      		} /* else */
      	} /* if */
      else
      	{ /* came from gadget */
	      Make_DB_Window(0);
	      } /* else */

	   /* Set menu checkmark state based on whether window is open or not */
	   DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATABASE), DB_Win);

#ifdef WCS_MUI_2_HACK
/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

      break;
      } /* Open Database Master window */

     case GP_ACTIVEWIN:
      {
/*      DoMethod(DB_Win->DatabaseWindow, MUIM_Window_ToFront);*/
      LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
      break;
      } /* Activate Editing Module window */

     case GP_CYCLE1:
      {
      switch (WCS_ID)
       {
       case ID_DB_LAYOUT:
        {
        LONG selected;
        get(DB_Win->CY_Layout, MUIA_Cycle_Active, &selected);
        if (selected != DB_Win->Layout)
         {
         Close_DB_Window();
         Make_DB_Window(selected);

		   /* Set menu checkmark state based on whether window is open or not */
		   /* Unnecessary, but safe */
		   DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATABASE), DB_Win);

#ifdef WCS_MUI_2_HACK
/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

         } /* if change layout */
        break;
        } /* change window orientation */
       } /* switch Gadget ID */
      break;
      } /* CYCLE1 */

     case GP_BUTTONS1:
      {
      switch (WCS_ID)
       {
       case ID_DB_LOAD:
        {
        Database_LoadDisp(0, 1, NULL, 1);
        break;
        } /* Load database */
       case ID_DB_APPEND:
        {
        if (dbaseloaded)
         {
         Database_LoadDisp(NoOfObjects, 1, NULL, 1);
	 } /* if database loaded */
        else NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULEAPPEND ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Database Module: Append", "a Database"
        break;
        } /* Append database */
       case ID_DB_CREATE:
        {
        dbaseloaded = makedbase(1);
        break;
        } /* Create new database */
       case ID_DB_SAVE:
        {
        if (dbaseloaded) savedbase(1);
         else NoLoad_Message((CONST_STRPTR) GetString( MSG_DB_DATABASEMODULESAVE ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Database Module: Save", "a Database"
        break;
        } /* SaveAs database */
       case ID_DB_SAVECUR:
        {
        if (dbaseloaded) savedbase(0);
         else NoLoad_Message((CONST_STRPTR) GetString( MSG_DB_DATABASEMODULESAVE ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Database Module: Save", "a Database"
        break;
        } /* Save database */
       case ID_DB_LOADCONFIG:
        {
        LoadConfig();
        break;
        } /* Load WCS window configuration */
       case ID_DB_SAVECONFIG:
        {
        SaveConfig();
        break;
        } /* Save WCS window configuration */
       case ID_DB_QUIT:
        {
        Close_DB_Window();
        /* Set menu checkmark state based on whether window is open or not */
	     DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATABASE), DB_Win);
	     break;
        } /* close window */
       } /* switch gadget ID */
      break;
      } /* BUTTONS1 */
     } /* switch gadget group */
    break;
    } /* Database Module Window */

   case WI_WINDOW1:
    {
    Handle_DE_Window(WCS_ID);
    break;
    } /* Database editing window */
   case WI_WINDOW2:
    {
    Handle_DL_Window(WCS_ID);
    break;
    } /* Database editing window */

   } /* switch window ID */

} /* Handle_DB_Window() */

/************************************************************************/

void Make_DO_Window(short hor_win)
{
 long open;
 static const char *LayoutCycle[3];
 static int Init = 1;
 if (Init)
 {
	 Init = 0;
	 LayoutCycle[0]= (char*)GetString( MSG_AGUI_STANDUP );  // "Stand Up"
	 LayoutCycle[1]= (char*)GetString( MSG_AGUI_LAYDOWN );  // "Lay Down"
	 LayoutCycle[2]= NULL;
 }

 if (DO_Win)
  {
  DoMethod(DO_Win->DataOpsWindow, MUIM_Window_ToFront);
  set(DO_Win->DataOpsWindow, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((DO_Win = (struct DataOpsModWindow *)
	get_Memory(sizeof (struct DataOpsModWindow), MEMF_CLEAR)) == NULL)
   return;

 DO_Win->Layout = hor_win;

 Set_Param_Menu(10);

     DO_Win->DataOpsWindow = WindowObject,
      MUIA_Window_Title		,  GetString( MSG_AGUI_DATAOPSMODULE ) ,  // "DataOps Module"
      MUIA_Window_ID		, (hor_win ? MakeID('D','O','A','H'): MakeID('D','O','A','V')),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_LeftEdge	, MUIV_Window_LeftEdge_Moused,
      MUIA_Window_TopEdge	, MUIV_Window_TopEdge_Moused,

      WindowContents, ColGroup(hor_win * 3 + 1),
	Child, DO_Win->CY_Layout = CycleObject,
		 MUIA_Cycle_Entries, LayoutCycle, End,
        Child, DO_Win->BT_Extract = KeyButtonFunc('e',   (char*)GetString( MSG_AGUI_EXTRACTDEM ) ),  // "\33l Extract DEM "
        Child, DO_Win->BT_Convert = KeyButtonFunc('c',   (char*)GetString( MSG_AGUI_CONVERTDEM ) ),  // "\33l Convert DEM "
        Child, DO_Win->BT_InterpMap = KeyButtonFunc('p', (char*)GetString( MSG_AGUI_INTERPDEM ) ),   // "\33l Interp DEM  "
        Child, DO_Win->BT_ImportDLG = KeyButtonFunc('i', (char*)GetString( MSG_AGUI_IMPORTDLG ) ),   // "\33l Import DLG  "
        Child, DO_Win->BT_ImportDXF = KeyButtonFunc('x', (char*)GetString( MSG_AGUI_IMPORTDXF ) ),   // "\33l Import DXF  "
        Child, DO_Win->BT_ImportWDB = KeyButtonFunc('w', (char*)GetString( MSG_AGUI_IMPORTWDB ) ),   // "\33l Import WDB  "
        Child, DO_Win->BT_ExportLWOB = KeyButtonFunc('l',(char*)GetString( MSG_AGUI_EXPORTLW ) ),    // "\33l Export LW   "
	Child, RectangleObject, End,
        End, /* VGroup */
      End; /* DO_win->EditWindow */

  if (! DO_Win->DataOpsWindow)
   {
   Close_DO_Window();
   User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "DataOps Module", "Out of memory!", "OK"
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DO_Win->DataOpsWindow);

/* Set Application notification events */
  DoMethod(DO_Win->DataOpsWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DO_QUIT);  
  DoMethod(DO_Win->CY_Layout, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DO_LAYOUT);

  MUI_DoNotiPresFal(app,
   DO_Win->BT_Extract, ID_DM_WINDOW, DO_Win->BT_Convert, ID_DC_WINDOW,
   DO_Win->BT_ImportDLG, ID_DO_IMPORTDLG, DO_Win->BT_ImportDXF, ID_DO_IMPORTDXF,
   DO_Win->BT_ImportWDB, ID_DO_IMPORTWDB, 
   DO_Win->BT_ExportLWOB, ID_LW_WINDOW, 
   DO_Win->BT_InterpMap, ID_DI_WINDOW, NULL);

/* Set tab cycle chain */
  DoMethod(DO_Win->DataOpsWindow, MUIM_Window_SetCycleChain,
	DO_Win->CY_Layout, DO_Win->BT_Extract, DO_Win->BT_Convert,
	DO_Win->BT_InterpMap,
	DO_Win->BT_ImportDLG, DO_Win->BT_ImportDXF, DO_Win->BT_ImportWDB, NULL);

/* Set active cycle gadget entry */
  set(DO_Win->CY_Layout, MUIA_Cycle_Active, hor_win);

/* Set active gadget */
  set(DO_Win->DataOpsWindow, MUIA_Window_ActiveObject, (IPTR)DO_Win->BT_Extract);

/* Open window */
  set(DO_Win->DataOpsWindow, MUIA_Window_Open, TRUE);
  get(DO_Win->DataOpsWindow, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DO_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DO_Win->DataOpsWindow, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DO_ACTIVATE);

} /* Make_DO_Window(void) */

/************************************************************************/

STATIC_FCN void Close_DO_Window(void) // used locally only -> static, AF 25.7.2021
{
 if (DO_Win)
  {
  if (DO_Win->DataOpsWindow)
   {
   set(DO_Win->DataOpsWindow, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, DO_Win->DataOpsWindow);
   MUI_DisposeObject(DO_Win->DataOpsWindow);
   } /* if */
  free_Memory(DO_Win, sizeof (struct DataOpsModWindow));
  DO_Win = NULL;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
  } /* if */
} /* Close_DO_Window() */

/************************************************************************/

STATIC_FCN void Handle_DO_Window(ULONG WCS_ID) // used locally only -> static, AF 25.7.2021
{

  switch (WCS_ID & 0x00ff0000)
   {
   case WI_WINDOW0:
    {
    switch (WCS_ID & 0x0000ff00)
     {
     case GP_OPEN_WINDOW:
      {
      if(WCS_ID & GA_GADNUM(1))
      	{ /* came from menu item, not gadget */
      	if(DO_Win)
      		{ /* it's already open, close it */
      		Close_DO_Window();
      		} /* if */
      	else
      		{ /* not open yet, open it */
      		Make_DO_Window(0);
      		} /* else */
      	} /* if */
      else
      	{ /* came from gadget */
	      Make_DO_Window(0);
	      } /* else */

	   /* Set menu checkmark state based on whether window is open or not */
	   DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATAOPS), DO_Win);

#ifdef WCS_MUI_2_HACK
/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

      break;
      } /* Open DataOps Master window */

     case GP_ACTIVEWIN:
      {
/*      DoMethod(DO_Win->DataOpsWindow, MUIM_Window_ToFront);*/
      LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
      break;
      } /* Activate Editing Module window */

     case GP_CYCLE1:
      {
      switch (WCS_ID)
       {
       case ID_DO_LAYOUT:
        {
        LONG selected;
        get(DO_Win->CY_Layout, MUIA_Cycle_Active, &selected);
        if (selected != DO_Win->Layout)
         {
         Close_DO_Window();
         Make_DO_Window(selected);
		   /* Set menu checkmark state based on whether window is open or not */
		   DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATAOPS), DO_Win);

#ifdef WCS_MUI_2_HACK
/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

         } /* if change layout */
        break;
        } /* change window orientation */
       } /* switch Gadget ID */
      break;
      } /* CYCLE1 */

     case GP_BUTTONS1:
      {
      switch (WCS_ID)
       {
       case ID_DO_IMPORTDLG:
        {
        if (! dbaseloaded)
         NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSIMPORTDLG ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Data Ops: Import DLG", "a Database"
        else
         ImportDLG();
        break;
        } /* Import DLG */
       case ID_DO_IMPORTDXF:
        {
        if (! dbaseloaded)
         NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSIMPORTDXF ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Data Ops: Import DXF", "a Database"
        else
         ImportDXF();
        break;
        } /* Import DLG */
       case ID_DO_IMPORTWDB:
        {
/*        ConvertTree(); See Tree.c for converting a brush into tree array */
/*        LandscapeGen(); See DEM.c for generating fractal landscapes */
/*        FixDEM(); See DEM.c for copying some corner header info from one USGS DEM to another*/
/*        ConstructDEMObj(); See Database.c for constructiong a set of vector objects from the USGS dem250.txt file */
/*        StrataConvert(); See ColorBlends.c for converting gray-scale image into a strata texture array */

        if (! dbaseloaded)
         NoLoad_Message((CONST_STRPTR) GetString( MSG_DLG_DATAOPSIMPORTWDB ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Data Ops: Import WDB", "a Database"
        else
         ImportWDB();

        break;
        } /* Import World Data Bank */
       case ID_DO_QUIT:
        {
        Close_DO_Window();
        /* Set menu checkmark state based on whether window is open or not */
        DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATAOPS), DO_Win);
        break;
        } /* close window */
       } /* switch gadget ID */
      break;
      } /* BUTTONS1 */
     } /* switch gadget group */
    break;
    } /* DataOps Module Window */

   case WI_WINDOW2:
    {
    Handle_DC_Window(WCS_ID);
    break;
    } /* Convert DEM window */
   case WI_WINDOW3:
    {
    if (! dbaseloaded)
     NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSMODULEINTERPDEM ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Data Ops Module: Interp DEM", "a Database"
    else
     Handle_DI_Window(WCS_ID);
    break;
    } /* Interpolate DEM window */
   case WI_WINDOW4:
    {
    if (! dbaseloaded)
     NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSMODULEEXTRACTDEM ) , (CONST_STRPTR) GetString( MSG_DATAOPSGUI_ADATABASE ) );  // "Data Ops Module: Extract DEM", "a Database"
    else
     Handle_DM_Window(WCS_ID);
    break;
    } /* Extract DEM window */
   case WI_WINDOW5:
    {
    Handle_MD_Window(WCS_ID);
    break;
    } /* Extract DEM window */
   case WI_WINDOW6:
    {
    Handle_GR_Window(WCS_ID);
    break;
    } /* Extract DEM window */

   } /* switch window ID */

} /* Handle_DO_Window() */

/************************************************************************/

/*STATIC_FCN*/ void Handle_RN_Window(ULONG WCS_ID) // used locally only -> static, AF 25.7.2021
{
if (dbaseloaded && paramsloaded)
 {
 Close_ES_Window(1);
 render = settings.renderopts;
 globemap();
 } /* if dbaseloaded and paramsloaded */
else
 NoLoad_Message((CONST_STRPTR) GetString( MSG_AGUI_RENDERMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_ADATABASEANDPARAMETERFILE ) );  // "Render Module", "a Database and Parameter file"

} /* Handle_RN_Window() */

/************************************************************************/

struct WCSApp *WCS_App_New(void)
{
 struct WCSApp *This;
 char *AppName;

 Log_Win = NULL;
 CreditWin = NULL;

 if((This = (struct WCSApp *)get_Memory(sizeof(struct WCSApp), MEMF_CLEAR)))
  {
  This->MUIApp = ApplicationObject,
    MUIA_Application_Title			, APP_TITLE,
    MUIA_Application_Version		, APP_VERSION,
    MUIA_Application_Copyright	, APP_COPYRIGHT,
    MUIA_Application_Author		, APP_AUTHOR,
    MUIA_Application_Description	, APP_DESCRIPTION,
    MUIA_Application_Base			, APP_TLA,
#ifndef __AROS__
    MUIA_Application_Menu			, WCSNewMenus,
#else
    MUIA_Application_Menustrip,  MUI_MakeObject(MUIO_MenustripNM,WCSNewMenus,0),  // AF: MUIA_Application_Menu is obsolete and not suppurted by Aros Zune
#endif
    MUIA_Application_UseRexx		, FALSE,
    MUIA_Application_UseCommodities,	FALSE,
   End; /* App */

  if(This->MUIApp)
	{
	get(This->MUIApp, MUIA_Application_Base, &AppName);
	RexxAp = Rexx_New(AppName);
	return(This);
	} /* if */
  else
	{
	free_Memory(This, sizeof(struct WCSApp));
	return(NULL);
	} /* else */
  } /* if */

 return(NULL);

} /* WCS_App_New() */

/**************************************************************************/

struct WCSApp *WCS_App_Startup(struct WCSApp *This)
{
 APTR BT_Database, BT_DataOps, BT_Mapping, BT_Editing,
	 BT_Render;
 APTR BT_AboutOK;

 ModControlWin = WindowObject,
      MUIA_Window_Title      , GetString( MSG_AGUI_MODULECONTROLPANEL ) ,  // "Module Control Panel",
      MUIA_Window_ID         , MakeID('W','C','S','M'),
      MUIA_Window_SizeGadget , FALSE,
      MUIA_Window_Screen     , WCSScrn,

      WindowContents, ColGroup(7),
        Child, BT_Database = FixedImgObject, MUIA_Image_OldImage, &DatabaseMod,
         MUIA_InnerBottom, 0, MUIA_InnerTop, 0, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End,
        Child, BT_DataOps = FixedImgObject, MUIA_Image_OldImage, &DataOpsMod,
         MUIA_InnerBottom, 0, MUIA_InnerTop, 0, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End,
        Child, BT_Mapping = FixedImgObject, MUIA_Image_OldImage, &MappingMod,
         MUIA_InnerBottom, 0, MUIA_InnerTop, 0, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End,
        Child, BT_Editing = FixedImgObject, MUIA_Image_OldImage, &EditingMod,
         MUIA_InnerBottom, 0, MUIA_InnerTop, 0, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End,
        Child, BT_Render = FixedImgObject, MUIA_Image_OldImage, &RenderMod,
         MUIA_InnerBottom, 0, MUIA_InnerTop, 0, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End,
        End, /* ColGroup */

      End; /* ModControlWin */

 if(ModControlWin)
  {
  DoMethod(app, OM_ADDMEMBER, ModControlWin);

  /* Don't think we need MUI2_MenuCheck_Hack() here */
 
  AboutWin =  WindowObject,
      MUIA_Window_Title      ,  GetString( MSG_AGUI_VERSION ) ,  // "Version"
      MUIA_Window_ID         , MakeID('A','B','U','T'),
      MUIA_Window_SizeGadget  , FALSE,
      MUIA_Window_Screen    , WCSScrn,

      WindowContents, VGroup,
        Child, HGroup,
          Child, ImageObject, MUIA_Image_OldImage, &CompRose,
           MUIA_Weight, 1, End,
          Child, ImageObject, MUIA_Frame, MUIV_Frame_Text,
           MUIA_Image_OldImage, &AboutScape, MUIA_Weight, 1,
           MUIA_InnerRight, 0, MUIA_InnerLeft, 0,
           MUIA_InnerTop, 0, MUIA_InnerBottom, 0, End,
          End, /* HGroup */
        Child, VSpace(5),
        Child, TextObject, MUIA_Text_Contents,
		 "\33c" APP_TITLE "", End,                              // \33c == center text
        Child, TextObject, MUIA_Text_Contents,
		 ExtAboutVers, End,
        Child, TextObject, MUIA_Text_Contents,
		 ExtAboutBuild, End,
        Child, TextObject, MUIA_Text_Contents,
		 "\33cCopyright "APP_COPYR, End,
        Child, TextObject, MUIA_Text_Contents,
		 "\33cby Questar Productions", End,
        Child, VSpace(5),
        Child, BT_AboutOK = KeyButtonFunc('o', (char*)GetString( MSG_AGUI_OKAY ) ),  // "\33cOkay"
        End, /* VGroup */
      End; /* SubWindow AboutWin */
 
	if(AboutWin)
		{
		DoMethod(app, OM_ADDMEMBER, AboutWin);

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

		DoMethod(app, MUIM_Application_SetMenuCheck, ID_VERSION, TRUE);
		} /* if */
	else
		{
		return(NULL);
		} /* if */
	} /* if */
 else
	{
	return(NULL);
	} /* else */


 if(ModControlWin && AboutWin) /* D'oh. We wouldn't get here if they weren't, but... */
  {
/*
** Install notification events...
*/

  DoMethod(ModControlWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_QUIT);
  DoMethod(AboutWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    AboutWin, 3, MUIM_Set, MUIA_Window_Open, FALSE);  
  DoMethod(BT_AboutOK, MUIM_Notify, MUIA_Pressed, FALSE,
    AboutWin, 3, MUIM_Set, MUIA_Window_Open, FALSE);
  DoMethod(AboutWin, MUIM_Notify, MUIA_Window_Open, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, MO_DUMMYMENU);


#ifdef XENON_DOESNT_LIKE_THIS
  MUI_DoNotiPresFal(app, BT_Database, MO_DATABASE,
   BT_DataOps, MO_DATAOPS,
   BT_Mapping, MO_MAPPING, BT_Editing, MO_EDITING,
   BT_Render, ID_ES_WINDOW, NULL);
#else
  MUI_DoNotiPresFal(app, BT_Database, MO_DATABASE,
   BT_DataOps, MO_DATAOPS,
   BT_Mapping, MO_MAPPING, BT_Editing, MO_EDITING,
   BT_Render, ID_ES_WINDOW, NULL);
#endif /* XENON_DOESNT_LIKE_THIS */

/* Notify of window activation */
  DoMethod(ModControlWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_MCP_ACTIVATE);

/* Set tab cycle chains */
  DoMethod(AboutWin, MUIM_Window_SetCycleChain, BT_AboutOK, NULL);
  set(AboutWin, MUIA_Window_ActiveObject, (IPTR)BT_AboutOK);
  DoMethod(ModControlWin, MUIM_Window_SetCycleChain,
	BT_Database, BT_DataOps, BT_Mapping, BT_Editing, BT_Render, NULL);

  set(ModControlWin, MUIA_Window_ActiveObject, (IPTR)BT_Database);

/* Open AboutWindow and Module Control Panel */
  set(ModControlWin,MUIA_Window_Open,TRUE);
  set(AboutWin,MUIA_Window_Open,TRUE);

/* Get MCP Window structure pointer */
  get(ModControlWin, MUIA_Window_Window, &MCPWin); 

/* Set system requests to WCS Screen */
  This->Proc = (struct Process *)FindTask(NULL);
  This->OldSysReqWin = This->Proc->pr_WindowPtr;
  This->Proc->pr_WindowPtr = MCPWin;

#ifdef MUI_PUBSCREEN_STUFF
  get(ModControlWin,MUIA_Window_Screen  ,&WCSScrn);
  if(!strnicmp(WCSScrn->Title, "World", 5))
    {
    if(WCSScrn->BitMap.Depth > 3)
      {
      LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
      } /* if */
    } /* if */
#endif /* MUI_PUBSCREEN_STUFF */

	return(This);
	} /* if */

 return(NULL);

} /* WCS_App_Startup() */

/*************************************************************************/
/*
** Input loop...
*/

short WCS_App_EventLoop(struct WCSApp *This)
{
 short i, ResetScrn = 0;
 ULONG /*IDCMP_Signals = 0L,*/ signals, WCS_ID;
 BOOL CheckBack, running = TRUE;
 struct RexxMsg *RexxCom;

  while (running)
   {
   CheckBack = 0; /* Have not processed any events this go-around */

   if(InterWind0)
    if(QuickCheckEvent(InterWind0))
      {
      Handle_InterWind0();
      CheckBack = 1;
      } /* if */
   if(InterWind2)
    if(QuickCheckEvent(InterWind2))
      {
      Handle_InterWind2();
      CheckBack = 1;
      } /* if */
   if(MapWind0)
    if(QuickCheckEvent(MapWind0))
      {
      Handle_Map_Window(0);
      CheckBack = 1;
      } /* if */
   if(MapWind3)
    if(QuickCheckEvent(MapWind3))
      {
      Handle_Viewshed_Window();
      CheckBack = 1;
      } /* if */
   
   for(i=0; i<NoOfSmWindows; i++)
    {
    if(QuickCheckEvent(SmWin[i]->win)) /* Not Quiche CheckEvent() */
     {
     Handle_Small_Window(i);
     CheckBack = 1;
     } /* if */
    } /* for */

   if ((WCS_ID = DoMethod(app, MUIM_Application_Input, &signals)))
    {
    CheckBack = 1;
    switch (WCS_ID & 0x0f000000)
     {
     case MO_DATABASE:
      {
      Handle_DB_Window(WCS_ID);
      break;
      } /* Database module */
     case MO_DATAOPS:
      {
      Handle_DO_Window(WCS_ID);
      break;
      } /* Data Operations module */
     case MO_MAPPING:
      {
      if((WCS_ID == MO_MAPPING) || (WCS_ID == (GA_GADNUM(1) | MO_MAPPING)))
       {
       if(WCS_ID == (GA_GADNUM(1) | MO_MAPPING))
       	{
       	if(MapWind0)
         Handle_Map_Window(ID_MC_QUIT);	
        else
         map();
        }
       else
        map();
#ifdef WCS_MUI_2_HACK
       MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
       } /* if */
      else
       {
       Handle_Map_Window(WCS_ID);
       } /* else */
      break;
      } /* Mapping module */
     case MO_EDITING:
      {
      Handle_EP_Window(WCS_ID);
      break;
      } /* Editing Module */
     case MO_WAVEED:
      {
      Handle_WV_Window(WCS_ID);
      break;
      } /* Editing Module */
     case MO_TIMELINES:
      {
      Handle_TL_Window(WCS_ID);
      break;
      } /* Editing Module */
     case MO_RENDER:
      {
      Handle_RN_Window(WCS_ID);
      break;
      } /* Render module */
     case MO_EXTRAS:
      {
      NoMod_Message(GetString( MSG_AGUI_EXTRASMODULE ) );  // "Extras Module"
      break;
      } /* Extras module */
     case MO_DUMMYMENU:
      {
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
		break;
      } /* Dummy menu refresh message */
     case MO_APPLICATION:
      {
      if ((ResetScrn = Handle_APP_Windows(WCS_ID)) == 0)
       break;
      } /* Application Module */
     case MUIV_Application_ReturnID_Quit:
     case MO_EXIT:
      {
      short ans;

      int ReadyToClose = 0;
      if(!PubScreenStatus(WCSScrn, PSNF_PRIVATE))
      	{
      	while(! ReadyToClose)
      		{
      		if ((ans = User_Message_Def((CONST_STRPTR)"World Construction Set",
      				(CONST_STRPTR) GetString( MSG_AGUI_PUBLICSCREENSTILLHASVISITORSTRYCLOSINGAGAIN ) ,  // "Public Screen still has visitors. Try closing again?"
					(CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2)) > 0)  // "Close|Warn|Cancel"
			ReadyToClose = PubScreenStatus(WCSScrn, PSNF_PRIVATE);
		else
			break; /* I hope this punches far enough out. */
		} /* while */
	if(ReadyToClose)
		{
		if (ans == 2)
			Mod_Warn = 1;
		running = FALSE;
		} /* if no guests */
      	} /* if */
      else
      	{
	if ((ans = User_Message_Def((CONST_STRPTR)"World Construction Set",
			(CONST_STRPTR) GetString( MSG_AGUI_QUITPROGRAMREYOUSURE ) ,  // )"Quit Program\nAre you sure?"
			(CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2)) > 0)  // "Close|Warn|Cancel"
		{
		running = FALSE;
		if (ans == 2)
			Mod_Warn = 1;
		} /* if close now! */
	else
		PubScreenStatus(WCSScrn, 0);
	} /* else */
      break;
      } /* Exit WCS */
     } /* switch module */
    } /* if not a custom window IDCMP event */
    
   if(RexxAp)
    {
    if((RexxCom = Rexx_GetMsg(RexxAp)))
    	{
    	Cmd_ParseDispatch(RexxAp, RexxCom);
    	/* Status_Log(ARG0(RexxCom), 0); */
    	/* Rexx_ReplyMsg(RexxCom, "Foo.", 0); */
    	} /* if */
    } /* if */

/* This is a hack. I know it. I don't think it should run this often, but I can't
** hunt down each case where a new MUI window gets opened at this time. This
** works around what I think is an MUI bug in Application_Menu, and hopefully
** when THAT is fixed, we can do away with this.
*/
#ifdef WCS_MUI_2_HACK
/* I think I have removed all reasons to do this here. This process is now
** done each and every time a window is created. So it is commented out here,
** but the MUI2_MenuCheck_Hack() function must still exist, since it is called
** from all over.
*/

/*	   MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */

    /* If CheckBack is set it means we did process an event this time through
    ** the loop, and we may have possibly missed an event, so lets do another
    ** run through the input checking loop quickly, and make sure there's
    ** NO work to be done before we go to sleep. */
    if(CheckBack == 0)
      if (running && signals)
        /*IDCMP_Signals =*/ Wait(signals | WCS_Signals | Rexx_SigMask(RexxAp));

   } /* while running */

 return (ResetScrn);

} /* WCS_App_EventLoop() */



/***********************************************************************/

void WCS_App_Del(struct WCSApp *This)
{
short i;

 if (Proj_Mod && Mod_Warn)
	{
        if (User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_WCSPROJECT ) ,  // "WCS Project"
        		(CONST_STRPTR) GetString( MSG_AGUI_PROJECTPATHSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Project paths have been modified. Save them before closing?"
				(CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1))  // "OK|Cancel"
         SaveProject(-1, NULL, NULL);
	} /* if project modified */
 if (Par_Mod && Mod_Warn)
	{
        if (User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERMODULE ) ,  // "Parameter Module"
        		(CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Parameters have been modified. Save them before closing?"
				(CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1))  // "OK|Cancel"
         saveparams(0x1111, -1, 0);
	} /* if parameters modified */

#ifdef UNDER_CONST
 if (UnderConst)
	{
	UnderConst_Del();
	} /* if */
#endif /* UNDER_CONST */
 if (PJ_Win)
	{
	Close_PJ_Window(1);
	} /* if */
 if (PN_Win)
	{
	Close_PN_Window(0);
	} /* if */
 if (PS_Win)
	{
	Close_PS_Window(1);
	} /* if */
 if (PR_Win)
	{
	Close_PR_Window();
	} /* if */
 if (EC_Win)
	{
	Close_EC_Window(1);
	} /* if */
 if (EM_Win)
	{
	Close_EM_Window(1);
	} /* if */
 if (FM_Win)
	{
	Close_FM_Window();
	} /* if */
 if (EL_Win)
	{
	Close_EL_Window();
	} /* if */
 if (FE_Win)
	{
	Close_FE_Window(1);
	} /* if */
 if (EE_Win)
	{
	Close_EE_Window(1);
	} /* if */
 if (SC_Win)
	{
	Close_SC_Window();
	} /* if */
 if (ES_Win)
	{
	Close_ES_Window(1);
	} /* if */
 if (CL_Win)
	{
	Close_CL_Window();
	} /* if */
 for (i=0; i<WCS_MAX_TLWINDOWS; i++)
 	{
	if (TLWin[i])
		{
		Close_TL_Window(&TLWin[i], 1);
		} /* if */
	} /* for i=0... */
 if (WVWin[0])
	{
	Close_WV_Window(&WVWin[0]);
	} /* if */
 if (WVWin[1])
	{
	Close_WV_Window(&WVWin[1]);
	} /* if */
 if (MapWind0)
	{
	Close_Map_Window(1);
	} /* if */
 if (DB_Mod && Mod_Warn)
	{
        if (User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
        		(CONST_STRPTR) GetString( MSG_AGUI_DATABASEHASBEENMODIFIEDSAVEITBEFORECLOSING ) ,  // "Database has been modified. Save it before closing?"
				(CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1))  // "OK|Cancel"
         savedbase(1);
	} /* if database modified */
 if (DBase)
	{
	DataBase_Del(DBase, DBaseRecords);
        DBase = NULL;
	} /* if */
 if (MapWind3)
	{
	Close_Viewshed_Window();
	} /* if */
 if (EP_Win)
	{
	Close_EP_Window();
	} /* if */
 if (DC_Win)
	{
	Close_DC_Window();
	} /* if */
 if (DM_Win)
	{
	Close_DM_Window();
	} /* if */
 if (DI_Win)
	{
	Close_DI_Window();
	} /* if */
 if (DO_Win)
	{
	Close_DO_Window();
	} /* if */
 if (DB_Win)
	{
	Close_DB_Window();
	} /* if */
 if (DE_Win)
	{
	Close_DE_Window();
	} /* if */
 if (DL_Win)
	{
	Close_DL_Window(DL_Win->DLCopy);
	} /* if */
 if (LW_Win)
	{
	Close_LW_Window();
	} /* if */
 if (GR_Win)
	{
	Close_GR_Window();
	} /* if */
 if (MD_Win)
	{
	Close_MD_Window();
	} /* if */
 if (Log_Win)
	{
	Close_Log_Window(2);
	} /* if */

 if(RexxAp)
 	{
 	Rexx_Del(RexxAp);
 	RexxAp = NULL;
 	} /* if */

/* Will happen in DisposeObject */
/*  if (AboutWin) set(AboutWin, MUIA_Window_Open, FALSE); */

 if(This->Proc)
	This->Proc->pr_WindowPtr = This->OldSysReqWin;

/* Will get caught in DisposeObject calamity */
/* if(ModControlWin)
   set(ModControlWin, MUIA_Window_Open, FALSE); */

 if(This->MUIApp)
	{
	MUI_DisposeObject(This->MUIApp);
	This->MUIApp = NULL;
	} /* if */

 if(This)
	{
	free_Memory(This, sizeof(struct WCSApp));
	This = NULL;
	} /* if */

} /* WCS_App_Del() */

/***********************************************************************/

ULONG GetInput_ID(void)
{
 ULONG Input_ID, signals;
 short done = FALSE;

  while (! done)
   {

   Input_ID = DoMethod(app, MUIM_Application_Input, &signals);

   if (Input_ID > 0) done = TRUE;

   if (! done && signals) Wait(signals);

  } /* while ! done */

 return (Input_ID);

} /* GetInput_ID() */

/************************************************************************/

ULONG CheckInput_ID(void)
{
 ULONG signals;
 
 return (DoMethod(app, MUIM_Application_Input, &signals));

} /* CheckInput_ID() */

/************************************************************************/

USHORT User_Message(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey)
{
return(User_Message_Def(outlinetxt, message, buttons, buttonkey, 0));

} /* User_Message() */

/************************************************************************/

int User_Message_ForcedReturnValue=-1;   // 0=Cancel

void SetUser_Message_ForcedReturn(int Val)
{
	User_Message_ForcedReturnValue=Val;
}

int getUser_Message_ForcedReturn()
{
	return User_Message_ForcedReturnValue;
}

USHORT User_Message_Def(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey, int Default)
{
	if(getUser_Message_ForcedReturn()==-1)
	{
		 USHORT j = 0, i = 0, k = 0, error = 0, done = FALSE, numbuttons = 1;
		 ULONG UM_ID, signals;
		 long open;
		 APTR UM_Win, UM_BTGroup, UM_BT[11];
		 char buttontext[11][20];

		 //Set_Param_Menu(10);

		 while (buttons[i] && numbuttons<12)
		  {
		  if (buttons[i] == '|')
		   {
		   buttontext[j][k] = 0;
		   j++;
		   i++;
		   k=0;
		   continue;
		   }
		  buttontext[j][k] = buttons[i];
		  i++;
		  k++;
		  }
		 buttontext[j][k] = 0;
		 numbuttons = j + 1;

		 if (numbuttons > 10) numbuttons = 10;
		 if(Default > numbuttons) Default = 0;

		     UM_Win = WindowObject,
		      MUIA_Window_Title		, GetString( MSG_AGUI_MESSAGE ) ,  // "Message",
		      MUIA_Window_ID		, MakeID('U','M','E','S'),
		      MUIA_Window_Screen	, WCSScrn,

		      WindowContents, VGroup,
		        Child, TextObject, TextFrame,
				 MUIA_Text_PreParse, "\33c",
				 MUIA_Text_Contents, outlinetxt, End,

		        Child, FloattextObject, ReadListFrame,
				 MUIA_Floattext_Text, message,
				 MUIA_List_Format, "P=\33c", End,

			Child, UM_BTGroup = HGroup, MUIA_Group_SameSize, TRUE, End,
		        End, /* VGroup */
		      End; /* UM_Win */

		  if (! UM_Win) return(0);

		  for (j=0; j<numbuttons-1  && ! error; j++)
		   {
		   UM_BT[j + 1] = KeyButtonObject(buttonkey[j]),
			MUIA_Text_PreParse, "\33c",
			MUIA_Text_Contents, buttontext[j], End;
		   if (! UM_BT[j + 1]) error = 1;
		   else DoMethod(UM_BTGroup, OM_ADDMEMBER, UM_BT[j + 1]);
		   } /* for j=0... */

		  UM_BT[0] = KeyButtonObject(buttonkey[numbuttons - 1]),
			MUIA_Text_PreParse, "\33c",
			MUIA_Text_Contents, buttontext[numbuttons - 1], End;
		  if (! UM_BT[0]) error = 1;
		  else DoMethod(UM_BTGroup, OM_ADDMEMBER, UM_BT[0]);

		  if (error)
		   {
		   MUI_DisposeObject(UM_Win);
		   return (0);
		/*   break;*/
		   } /* if error creating buttons */

		  UM_BT[numbuttons] = NULL;

		/* Add window to application */
		  DoMethod(app, OM_ADDMEMBER, UM_Win);


		/* Install notifications */
		  DoMethod(UM_Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
			app, 2, MUIM_Application_ReturnID, ID_UM_BUTTON(0));

		  for (j=0; j<numbuttons; j++)
		   {
		   DoMethod(UM_BT[j], MUIM_Notify, MUIA_Pressed, FALSE,
			app, 2, MUIM_Application_ReturnID, ID_UM_BUTTON(j));
		   } /* for j=0... */

		/* Set tab cycle chain */
		  DoMethod(UM_Win, MUIM_Window_SetCycleChain,
			UM_BT[0], UM_BT[1], UM_BT[2], UM_BT[3], UM_BT[4],
			UM_BT[5], UM_BT[6], UM_BT[7], UM_BT[8], UM_BT[9], UM_BT[10]);
		  set(UM_Win, MUIA_Window_ActiveObject, (IPTR)UM_BT[Default]);

		/* Open Window and wait for button press */
		  set(UM_Win, MUIA_Window_Open, TRUE);
		  get(UM_Win, MUIA_Window_Open, &open);
		  if (! open)
		   {
		   done = 1;
		   j = 0;
		   } /* if no window */

		#ifdef WCS_MUI_2_HACK
			   MUI2_MenuCheck_Hack();
		#endif /* WCS_MUI_2_HACK */

		  while (! done)
		   {
		   UM_ID = DoMethod(app, MUIM_Application_Input, &signals);

		   switch (UM_ID & 0xff000000)
		    {
		    case MO_APPLICATION:
		     {
		     switch (UM_ID & 0x00ff0000)
		      {
		      case WI_WINDOW1:
		       {
		       switch (UM_ID & 0x0000ff00)
		        {
		        case GP_BUTTONS1:
		         {
		         j = UM_ID - ID_UM_BUTTON(0);
		         done = TRUE;
		         break;
			 } /* BUTTONS1 */
			} /* switch Gadget Group */
		       break;
		       } /* User Message Window */
		      } /* switch Window ID */
		     break;
		     } /* APPLICATION MODULE */
		    } /* switch Module ID */

		   if (! done && signals) Wait(signals);

		   } /* while ! done */

		  set(UM_Win, MUIA_Window_Open, FALSE);
		  DoMethod(app, OM_REMMEMBER, UM_Win);
		  MUI_DisposeObject(UM_Win);
		 return (j);
	}
	else  // forced behavior for automatic test. Do not show Requester, immediately return the value of getUser_Message_ForcedReturn()
	{
		return getUser_Message_ForcedReturn();
	}



} /* User_Message() */

/************************************************************************/

STATIC_FCN void NoMod_Message(STRPTR mod) // used locally only -> static, AF 25.7.2021
{

  User_Message(mod, (CONST_STRPTR) GetString( MSG_AGUI_NOTYETIMPLEMENTEDTAYTUNED ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "Not yet implemented.\nStay Tuned!", "OK"

  Log(MSG_NO_MOD, mod);

} /* NoMod_Message() */

/************************************************************************/

USHORT CloseWindow_Query(STRPTR win)
{

 return (User_Message_Def(win, (CONST_STRPTR) GetString( MSG_AGUI_KEEPCHANGES ) , (CONST_STRPTR) GetString( MSG_AGUI_KEEPCANCEL ) , (CONST_STRPTR)"kc", 1));  // "Keep changes?", "Keep|Cancel"

} /* CloseWindow_Query() */

/************************************************************************/

void NoLoad_Message(CONST_STRPTR mod, CONST_STRPTR loaditem)
{
 char loadmesg[255];

  sprintf(loadmesg, 
		  (char*)GetString( MSG_AGUI_SORRYOUMUSTFIRSTLOAD338NBEFOREUSINGTHISFEATURE ) , loaditem);  // "Sorry!\nYou must first load\n\338%s\0332\nbefore using this feature."

  User_Message(mod, (CONST_STRPTR)loadmesg, (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "OK"

  Log(ERR_NO_LOAD, loaditem);

} /* NoLoad_Message() */

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
USHORT FileExists_Message(STRPTR existsfile)
{

  return(User_Message_Def(existsfile,  GetString( MSG_AGUI_FILEALREADYEXISTSOYOUWISHTOOVERWRITEIT ) ,  // "File already exists.\nDo you wish to overwrite it?"
	 GetString( MSG_GLOBAL_OKCANCEL ) , "oc", 1));  // "OK|CANCEL"

} /* FileExists_Message() */
#endif
/************************************************************************/

short GetInputString(char *message, char *reject, char *string)
{
 char *outstring;
 APTR IS_Win, InputStr, BT_OK, BT_Cancel;
 short done = 0, retval;
 ULONG IS_ID, signals;

 Set_Param_Menu(10);

     IS_Win = WindowObject,
      MUIA_Window_Title		, GetString( MSG_AGUI_INPUTREQUEST ) ,  // "Input Request",
      MUIA_Window_ID		, MakeID('I','S','R','Q'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
        Child, FloattextObject, ReadListFrame,
		 MUIA_Floattext_Text, message,
		 MUIA_List_Format, "P=\33c", End,

	Child, InputStr = StringObject, StringFrame,
		MUIA_String_Reject, reject,
		MUIA_String_Contents, string,
		MUIA_String_BufferPos, strlen(string), End,

	Child, HGroup,
	  Child, BT_OK = KeyButtonFunc('o', (char*)GetString( MSG_GLOBAL_OK ) ),          // "\33cOK"
	  Child, BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL ) ),  // "\33cCancel"
	  End, /* HGroup */
        End, /* VGroup */
      End; /* IS_Win */

  if (! IS_Win) return(0);

/* Add window to application */
  DoMethod(app, OM_ADDMEMBER, IS_Win);



/* Install notifications */
  DoMethod(IS_Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_IS_CANCEL);

  MUI_DoNotiPresFal(app, BT_OK, ID_IS_OK, BT_Cancel, ID_IS_CANCEL, NULL);

/* Set tab cycle chain */
  DoMethod(IS_Win, MUIM_Window_SetCycleChain,
	InputStr, BT_OK, BT_Cancel, NULL);
  set(IS_Win, MUIA_Window_ActiveObject, (IPTR)InputStr);

/* set return cycle */
/*
  DoMethod(InputStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	IS_Win, 3, MUIM_Set, MUIA_Window_ActiveObject, BT_OK);
*/
/* Auto-Ack mode */
  DoMethod(InputStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_IS_OK);
 
/* Open Window and wait for button press */
  set(IS_Win, MUIA_Window_Open, TRUE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

  while (! done)
   {
   IS_ID = DoMethod(app, MUIM_Application_Input, &signals);

   switch (IS_ID & 0xff000000)
    {
    case MO_APPLICATION:
     {
     switch (IS_ID & 0x00ff0000)
      {
      case WI_WINDOW4:
       {
       switch (IS_ID & 0x0000ff00)
        {
        case GP_BUTTONS1:
         {
         done = TRUE;
         break;
	 } /* BUTTONS1 */
	} /* switch Gadget Group */
       break;
       } /* Input String Window */
      } /* switch Window ID */
     break;
     } /* APPLICATION MODULE */
    } /* switch Module ID */

   if (! done && signals) Wait(signals);

   } /* while ! done */

 if (IS_ID == ID_IS_OK)
  {
  get(InputStr, MUIA_String_Contents, &outstring);
  strcpy(string, outstring);
  retval = 1;
  } /* if */
 else
  retval = 0;

 set(IS_Win, MUIA_Window_Open, FALSE);
 DoMethod(app, OM_REMMEMBER, IS_Win);
 MUI_DisposeObject(IS_Win);

 return (retval);

} /* GetInputString() */

/************************************************************************/

STATIC_FCN void Status_Log(STRPTR logtext, int Severity) // used locally only -> static, AF 25.7.2021
{

  Make_Log_Window(Severity);
  if (! Log_Win)
   {
   User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGSTATUSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_CANTOPENLOGSTATUSWINDOW ) ,  // "Log Status Module", "Can't Open Log Status Window!"
		   (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "OK"
   return;
   } /* if no window */

 if (LogItem == 0)
  {
  DoMethod(Log_Win->LS_List, MUIM_List_Clear);
  } /* else reached max list entry */
 
 DoMethod(Log_Win->LS_List,
	MUIM_List_Insert, &logtext, 1, MUIV_List_Insert_Bottom);
 DoMethod(Log_Win->LS_List, MUIM_List_Jump, (LONG)LogItem);
 LogItem += 1;
 if (LogItem > 99) LogItem = 0;

} /* Status_Log */


/***********************************************************************/


STATIC_FCN void Make_Log_Window(int Severity) // used locally only -> static, AF 26.7.2021
{
 long open;

 if (Log_Win)
  {
  if(Log_Win->Hiding)
  	{
  	if(Severity > 127)
  	 {
  	 set(Log_Win->LogWindow, MUIA_Window_Open, TRUE);
  	 Log_Win->Hiding = 0;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
  	 } /* if */
  	} /* if */
  return;
  } /* if log already exists */
 else
  {
  /* Set/Reset log list array index to zero */
  LogItem = 0;
  } /* else */

 if ((Log_Win = (struct StatusLogWindow *)
	get_Memory(sizeof (struct StatusLogWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     Log_Win->LogWindow = WindowObject,
      MUIA_Window_Title		, GetString (MSG_AGUI_STATUSLOG ),  // "Status Log"
      MUIA_Window_ID		, MakeID('S','T','L','G'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup, MUIA_Group_SameWidth, TRUE,
        Child, Log_Win->LS_List = ListviewObject,
	        MUIA_Listview_Input, FALSE,
          MUIA_Listview_List, ListObject, ReadListFrame, End,
          End, /* ListviewObject */
        Child, HGroup, MUIA_Group_SameSize, TRUE,
          Child, Log_Win->BT_Clear = KeyButtonFunc('c', (char*)GetString( MSG_AGUI_CLEAR ) ),   // "\33cClear"
          Child, Log_Win->BT_Hide = KeyButtonFunc('h',  (char*)GetString( MSG_AGUI_HIDE ) ),    // "\33cHide"
          Child, Log_Win->BT_Quit = KeyButtonFunc('l',  (char*)GetString( MSG_AGUI_CLOSE ) ),   // "\33cClose"
          End, /* HGroup */
        End, /* VGroup */
      End; /* Log_Win */

  if (! Log_Win->LogWindow)
   {
   Close_Log_Window(2);
   User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGWINDOW ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Log Window", "Out of memory!", "OK"
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, Log_Win->LogWindow);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Close requests */
  DoMethod(Log_Win->LogWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_LOG);  

  MUI_DoNotiPresFal(app, Log_Win->BT_Quit, ID_LOG,
   Log_Win->BT_Hide, ID_LOG_HIDE, NULL);
   
  DoMethod(Log_Win->BT_Clear, MUIM_Notify, MUIA_Pressed, FALSE,
    Log_Win->LS_List, 1, MUIM_List_Clear);


/* Set tab cycle chain */
  DoMethod(Log_Win->LogWindow, MUIM_Window_SetCycleChain,
	Log_Win->BT_Quit, Log_Win->BT_Hide,
	Log_Win->BT_Clear,
	NULL);

/* Set active gadget */
  set(Log_Win->LogWindow, MUIA_Window_ActiveObject, (IPTR)Log_Win->BT_Quit);

/* Open window */
  set(Log_Win->LogWindow, MUIA_Window_Open, TRUE);
  get(Log_Win->LogWindow, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_Log_Window(2);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

} /* Make_Log_Window() */

/************************************************************************/

/* Note: 
** To make the Log window STAY closed, set it closed via MUI, but do not free
** the Log_Win or set it NULL. If you set Log_Win NULL, we'll just ploink it
** right back open next time it gets a message. However, if Log_Win is there,
** but it is not open, we'll quietly log the message without popping open.
**
** To reopen:
** If Log_Win is still there, just set() the Window back open. If Log_Win
** is not there, send a Log() message, and it'll open if it can.
*/
STATIC_FCN void Close_Log_Window(int StayClosed) // used locally only -> static, AF 25.7.2021
{
if(Log_Win)
  {
  if (Log_Win->LogWindow)
   {
   set(Log_Win->LogWindow, MUIA_Window_Open, FALSE);
   Log_Win->Hiding = !StayClosed;
   } /* if */
  if(StayClosed == 2) /* Shutdown, kill it all! */
   {
   DoMethod(app, OM_REMMEMBER, Log_Win->LogWindow);
   MUI_DisposeObject(Log_Win->LogWindow);
   free_Memory(Log_Win, sizeof (struct StatusLogWindow));
   Log_Win = NULL;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   } /* else */
  } /* if */
} /* Close_Log_Window() */

/************************************************************************/

STATIC_FCN short Handle_APP_Windows(ULONG WCS_ID) // used locally only -> static, AF 25.7.2021
{
 short ResetScrn = 0;

  switch (WCS_ID & 0x00ff0000)
   {
   case WI_WINDOW0:
    {
    switch (WCS_ID & 0x0000ff00)
     {
     case GP_ACTIVEWIN:
      {
      LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
      break;
      } /* ID_MCP_ACTIVEWIN */
     } /* switch gadget group */
    break;
    } /* MCPWindow */
   case WI_WINDOW2: /* Log module and misc menus */
    {
    switch (WCS_ID & 0x0000ff00)
     {
     case GP_BUTTONS1:
      {
      switch (WCS_ID)
       {
       case ID_MCP_ACTIVATE:
        {
        LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
        break;
        } /* ID_MCP_ACTIVEWIN */
       case ID_LOG:
        {
        LONG Open;
        
        if (Log_Win)
          {
          get(Log_Win->LogWindow, MUIA_Window_Open, &Open);
          if(Open)
            {
            Close_Log_Window(1);
            } /* if */
          else
            {
            set(Log_Win->LogWindow, MUIA_Window_Open, TRUE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
            } /* else */
          } /* if window already exists */
        else
          { /* need to create log window */
          Log(32, (CONST_STRPTR)GetString( MSG_AGUI_LOGWINDOWOPENED ) );  // "Log window opened."
          } /* else */
        get(Log_Win->LogWindow, MUIA_Window_Open, &Open);
        DoMethod(app, MUIM_Application_SetMenuCheck, ID_LOG, Open);
        break;
        } /* ID_LOG */
       case ID_LOG_HIDE:
        {
        Close_Log_Window(0);
        break;
        } /* ID_LOG_HIDE */
       } /* switch gadget ID */
      break;
      } /* BUTTONS1 */
     case GP_BUTTONS2:
      { /* BUTTONS2 */
      switch(WCS_ID)
       {
       case ID_INFO:
        {
        long open;

        if(InfoWin)
          {
          set(InfoWin, MUIA_Window_Open, FALSE);
          DoMethod(app, OM_REMMEMBER, InfoWin);
          MUI_DisposeObject(InfoWin);
/*          DoMethod(app, MUIM_Application_SetMenuCheck, ID_INFO, FALSE); */
          InfoWin = NULL;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
          } /* if */
        else
          {
          Set_Param_Menu(10);

          InfoWin = WindowObject,
            MUIA_Window_Title		, GetString( MSG_AGUI_INFO ),  // "Info"
            MUIA_Window_ID		, MakeID('I','N','F','O'),
            MUIA_Window_Screen	, WCSScrn,
            MUIA_Window_SizeGadget  , FALSE,

            WindowContents, VGroup,
              Child, ColGroup(2),
               Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_TIME ) , End,  // "\33rTime "
               Child, InfoTime = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_Text_PreParse, "\33c", End,

               Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_DATE ) , End,  // "\33rDate "
               Child, InfoDate = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_Text_PreParse, "\33c", End,

               Child, VGroup,
                Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                 TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_MEMORY ) , End,  // "\33rMemory:"
                Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                 TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_AVAILABLE ) , End,  // "\33rAvailable "
                Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                 TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_LARGEST ) , End,  // "\33rLargest "
                End,
               Child, VGroup,
                Child, HGroup, MUIA_Group_SameWidth, TRUE,
                 Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_CHIP ) , End,  // "\33cChip"
                 Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_FAST ) , End,  // "\33cFast"
                 End,
                Child, ColGroup(3), MUIA_Frame, MUIV_Frame_Group,
                 MUIA_InnerRight, 0, MUIA_InnerLeft, 0, MUIA_InnerTop, 0, MUIA_InnerBottom, 0,
                 MUIA_Group_Spacing, 0,
                 Child, InfoChipAvail = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, MUIA_FixWidth, 4, End,
                 Child, InfoFastAvail = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
                 Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
                 Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
                 Child, InfoChipLarge = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 Child, RectangleObject, MUIA_Rectangle_VBar, MUIA_FixWidth, 4, TRUE, End,
                 Child, InfoFastLarge = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 End,
                End,
                
               Child, VGroup,
                Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz, TRUE, End,
                Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                 TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_TOPOMAPS ) , End,  // "\33rTopo Maps "
                End,
               Child, VGroup,
                Child, HGroup, MUIA_Group_SameWidth, TRUE,
                 Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_INTER ) , End,  // "\33cInter"
                 Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_MAP ) , End,  // "\33cMap"
                 End,
                Child, HGroup, MUIA_Frame, MUIV_Frame_Group,
                 MUIA_InnerRight, 0, MUIA_InnerLeft, 0, MUIA_InnerTop, 0, MUIA_InnerBottom, 0,
                 MUIA_Group_Spacing, 0,
                 Child, InfoInterTopos = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 Child, RectangleObject, MUIA_Rectangle_VBar, MUIA_FixWidth, 4, TRUE, End,
                 Child, InfoMapTopos = TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                  TRUE, MUIA_Text_PreParse, "\33c", End,
                 End,
                End,

               Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_AREXXPORT ) , End,  // "\33rARexx Port "
               Child, InfoARexx = TextObject, MUIA_Frame, MUIV_Frame_Text,
                MUIA_Text_PreParse, "\33c", End,

               Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_DATABASE ) , End,  // "\33rDatabase "
               Child, InfoDataBase = TextObject, MUIA_Frame, MUIV_Frame_Text,
                MUIA_Text_PreParse, "\33c", End,

               Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_PARFILE ) , End,  // "\33rPar File "
               Child, InfoPar = TextObject, MUIA_Frame, MUIV_Frame_Text,
                MUIA_Text_PreParse, "\33c", End,
              Child, TextObject, MUIA_Frame, MUIV_Frame_Text, MUIA_FramePhantomHoriz,
                TRUE, MUIA_Text_Contents,  GetString( MSG_AGUI_SCREENMODE ) , End,  // "\33rScreenMode "
               Child, InfoScreenMode = TextObject, MUIA_Frame, MUIV_Frame_Text,
                MUIA_Text_PreParse, "\33c", End,

               
               End,
              Child, InfoWinFlush = KeyButtonFunc('F',  (char*)GetString( MSG_AGUI_FLUSH ) ),  // "\33cFlush"
#ifdef XENON_DOESNT_LIKE_THIS
              Child, InfoWinOK = KeyButtonObject('O'), MUIA_Text_Contents,  (char*)GetString( MSG_AGUI_OKAY ) ,  // "\33cOkay"
               End,
#endif /* XENON_DOESNT_LIKE_THIS */
              End,
             End;
          if(InfoWin)
          	{
#ifdef XENON_DOESNT_LIKE_THIS
          	DoMethod(InfoWinOK, MUIM_Notify, MUIA_Pressed, FALSE,
          	 app, 2, MUIM_Application_ReturnID, ID_INFO);
#endif /* XENON_DOESNT_LIKE_THIS */

          	MUI_DoNotiPresFal(app, InfoWinFlush, ID_INFO_FLUSH, NULL);
          	
          	DoMethod(InfoWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
          	 app, 2, MUIM_Application_ReturnID, ID_INFO);

            DoMethod(InfoWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
             app, 2, MUIM_Application_ReturnID, ID_INFO_ACTIVATE);

          	DoMethod(app, OM_ADDMEMBER, InfoWin);

/*          	DoMethod(app, MUIM_Application_SetMenuCheck, ID_INFO, TRUE); */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
          	
          	InfoWin_Update(0);

          	set(InfoWin, MUIA_Window_Open, TRUE);
          	get(InfoWin, MUIA_Window_Open, &open);
                if (! open)
                 {
                 set(InfoWin, MUIA_Window_Open, FALSE);
                 DoMethod(app, OM_REMMEMBER, InfoWin);
                 MUI_DisposeObject(InfoWin);
/*                 DoMethod(app, MUIM_Application_SetMenuCheck, ID_INFO, FALSE); */
                 InfoWin = NULL;
		 } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
          	} /* if */
          } /* else */
        break;
        } /* ID_INFO */
       case ID_INFO_FLUSH:
        {
        InfoWin_Update(1);
        break;
        } /* ID_INFO_FLUSH */
       case ID_INFO_ACTIVATE:
        {
        InfoWin_Update(0);
        break;
        } /* ID_INFO_ACTIVATE */
       case ID_VERSION:
        {
        LONG WindowState;

        get(AboutWin, MUIA_Window_Open, &WindowState);
        set(AboutWin, MUIA_Window_Open, !WindowState);
/*        DoMethod(app, MUIM_Application_SetMenuCheck, ID_VERSION, !WindowState); */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
        break;
        } /* ID_VERSION */
       case ID_CREDITS:
        {
        long open;

        if(CreditWin)
          {
          set(CreditWin, MUIA_Window_Open, FALSE);
          DoMethod(app, OM_REMMEMBER, CreditWin);
          MUI_DisposeObject(CreditWin);
          CreditWin = NULL;
/*          DoMethod(app, MUIM_Application_SetMenuCheck, ID_CREDITS, FALSE); */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
          } /* if */
        else
          {
          Set_Param_Menu(10);

          CreditWin = WindowObject,
            MUIA_Window_Title		, GetString( MSG_AGUI_CREDITS ),  // "Credits"
            MUIA_Window_ID		, MakeID('C','R','E','D'),
            MUIA_Window_Screen	, WCSScrn,
            WindowContents, VGroup,
              Child, HGroup,
               Child, RectangleObject, End,
               Child, VGroup,
                Child, ImageObject, MUIA_Frame, MUIV_Frame_Text,
                 MUIA_Image_OldImage, &Gary, MUIA_Weight, 1,
                 MUIA_InnerRight, 0, MUIA_InnerLeft, 0,
                 MUIA_InnerTop, 0, MUIA_InnerBottom, 0,
                 End,
                Child, TextObject, MUIA_Text_Contents, "\33cGary",
                 End,
                End,
               Child, RectangleObject, End,
               Child, VGroup,
                Child, ImageObject, MUIA_Frame, MUIV_Frame_Text,
                 MUIA_Image_OldImage, &Xenon, MUIA_Weight, 1,
                 MUIA_InnerRight, 0, MUIA_InnerLeft, 0,
                 MUIA_InnerTop, 0, MUIA_InnerBottom, 0, End,
                Child, TextObject, MUIA_Text_Contents, "\33cXenon",
                 End,
                End,
               Child, RectangleObject, End,
               End,
              Child, CreditList = ListviewObject, MUIA_Listview_Input, FALSE,
               MUIA_Listview_List,
                FloattextObject, ReadListFrame,
                MUIA_Floattext_Text, LocaleExtCreditText?LocaleExtCreditText:ExtCreditText,
                MUIA_List_Format, "P=\33c",
                End,
               End,
#ifdef XENON_DOESNT_LIKE_THIS
              Child, CreditWinOK = KeyButtonObject('O'), MUIA_Text_Contents, GetString( MSG_AGUI_OKAY ) ,  // "\33cOkay"
               End,
#endif /* XENON_DOESNT_LIKE_THIS */
              End,
            End;
          if(CreditWin)
          	{
#ifdef XENON_DOESNT_LIKE_THIS
          	DoMethod(CreditWinOK, MUIM_Notify, MUIA_Pressed, FALSE,
          	 app, 2, MUIM_Application_ReturnID, ID_CREDITS);
#endif /* XENON_DOESNT_LIKE_THIS */
          	
          	DoMethod(CreditWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
          	 app, 2, MUIM_Application_ReturnID, ID_CREDITS);

          	DoMethod(app, OM_ADDMEMBER, CreditWin);
/*          	DoMethod(app, MUIM_Application_SetMenuCheck, ID_CREDITS, TRUE); */
          	
          	set(CreditWin, MUIA_Window_ActiveObject, (IPTR)CreditList);

          	set(CreditWin, MUIA_Window_Open, TRUE);
          get(CreditWin, MUIA_Window_Open, &open);
          if (! open)
           {
           DoMethod(app, OM_REMMEMBER, CreditWin);
           MUI_DisposeObject(CreditWin);
           CreditWin = NULL;
	   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

          	} /* if */
          } /* else */
        break;
        } /* ID_CREDITS */
       case ID_LOADPROJ:
        {
        LoadProject(NULL, NULL, 0);
        break;
	} /* load project */
       case ID_SAVEPROJ:
        {
        SaveProject(0, NULL, NULL);
        break;
	} /* save with current name */
       case ID_SAVEPROJNEW:
        {
        SaveProject(1, NULL, NULL);
        break;
	} /* save with new name */
       case ID_SCREENSAVE:
        {
        saveILBM(0, 1, NULL, NULL, NULL, 0, 1, 1, WCSScrn->Width, WCSScrn->Height);
        break;
	} /* save screen shot */
       case ID_SCRNRESET:
        {
        ResetScrn = User_Message((CONST_STRPTR) GetString( MSG_AGUI_WCSSCREENMODE ) ,                           // "WCS: Screen Mode"
                                 GetString( MSG_AGUI_INORDERTORESETTHESCREENMODEWCSWILLHAVETOCLOSEANDREOPEN ),  // "In order to reset the screen mode WCS will have to close and re-open. Any work in progress should be saved before invoking this command.\nDo you wish to proceed now?"
                                 GetString( MSG_GLOBAL_OKCANCEL ),   // "OK|Cancel"
                                 (CONST_STRPTR)"oc");
        break;
	} /* reset screen mode */
       case ID_MUI_SETTINGS:     // AF, 11.10.2024, MUI Preferences
       {
           DoMethod(app,MUIM_Application_OpenConfigWindow,0);
    	   break;
       }
       } /* switch (WCS_ID) */
      break;
      } /* BUTTONS2 */
     } /* switch gadget group */
    break;
    } /* Log Module Window and misc menus */
   case WI_WINDOW6:
    {
    Handle_PJ_Window(WCS_ID);
    break;
    } /* Project Editor */
   case WI_WINDOW7:
    {
    Handle_PR_Window(WCS_ID);
    break;
    } /* Project Editor */
   case WI_WINDOW8:
    {
    Handle_PN_Window(WCS_ID);
    break;
    } /* Project Editor */
#ifdef UNDER_CONST
   case WI_WINDOW20:
    {
    /* Must be ID_UNDER_CONST */
    UnderConst_Del();
    break;
    } /* WINDOW20 */
#endif /* UNDER_CONST */
   } /* switch window ID */

 return (ResetScrn);

} /* Handle_APP_Windows() */

/************************************************************************/

void Log(USHORT StdMesgNum, CONST_STRPTR LogTag)
{
char *Prefix;

  if(ErrMagnitude[StdMesgNum] >= 200)
    {
    if (! ReportMesg[0])
     return;
    Prefix = (char*)GetString( MSG_AGUI_ERR );  // "ERR:"
    }
  else if(ErrMagnitude[StdMesgNum] >= 128)
    {
    if (! ReportMesg[1])
     return;
    Prefix = (char*)GetString( MSG_AGUI_WNG );  // "WNG:"
    }
  else if(ErrMagnitude[StdMesgNum] >= 100)
    {
    if (! ReportMesg[2])
     return;
    Prefix = (char*)GetString( MSG_AGUI_MSG );  // "MSG:"
    }
  else if(ErrMagnitude[StdMesgNum] >= 50)
    {
    if (! ReportMesg[3])
     return;
    Prefix = (char*)GetString( MSG_AGUI_DTA );  // "DTA:"
    }
  else
    {
    if (! ReportMesg[3])
     return;
    Prefix = "";
    }
  sprintf(LogMesg[LogItem], "%s%s %s", Prefix, StdMesg[StdMesgNum], LogTag);
  Status_Log((STRPTR)LogMesg[LogItem], ErrMagnitude[StdMesgNum]);

} /* Log() */

/************************************************************************/

void DisableKeyButtons(short group)
{
 short DisableAll;
 long item;

 switch (group)
  {
  case 0:
   {
   if (EM_Win->IsKey >= 0)
    {
    set(EM_Win->BT_DeleteKey, MUIA_Disabled, FALSE);
    }
   else
    {
    set(EM_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
    }
   if (EM_Win->PrevKey >= 0)
    {
    set(EM_Win->BT_PrevKey, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_PK ), EM_Win->PrevKey);  // "PK %d"
    set(EM_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
    if (EMTL_Win)
     {
     set(EMTL_Win->BT_PrevKey, MUIA_Disabled, FALSE);
     set(EMTL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
     } /* if motion time line window open */
    if (EMIA_Win)
     {
     set(EMIA_Win->StrArrow[2][0], MUIA_Disabled, FALSE);
     } /* if motion time line window open */
    }
   else
    {
    set(EM_Win->BT_PrevKey, MUIA_Disabled, TRUE);
    set(EM_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
    if (EMTL_Win)
     {
     set(EMTL_Win->BT_PrevKey, MUIA_Disabled, TRUE);
     set(EMTL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
     } /* if motion time line window open */
    if (EMIA_Win)
     {
     set(EMIA_Win->StrArrow[2][0], MUIA_Disabled, TRUE);
     } /* if motion time line window open */
    }
   if (EM_Win->NextKey >= 0)
    {
    set(EM_Win->BT_NextKey, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_NK ), EM_Win->NextKey);  // "NK %d"
    set(EM_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
    if (EMTL_Win)
     {
     set(EMTL_Win->BT_NextKey, MUIA_Disabled, FALSE);
     set(EMTL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
     set(EMTL_Win->TxtArrow[0], MUIA_Disabled, FALSE);
     set(EMTL_Win->TxtArrow[1], MUIA_Disabled, FALSE);
     set(EMTL_Win->TxtArrowLg[0], MUIA_Disabled, FALSE);
     set(EMTL_Win->TxtArrowLg[1], MUIA_Disabled, FALSE);
     } /* if motion time line window open */
    if (EMIA_Win)
     {
     set(EMIA_Win->StrArrow[2][1], MUIA_Disabled, FALSE);
     } /* if motion time line window open */
    }
   else
    {
    set(EM_Win->BT_NextKey, MUIA_Disabled, TRUE);
    set(EM_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
    if (EMTL_Win)
     {
     set(EMTL_Win->BT_NextKey, MUIA_Disabled, TRUE);
     set(EMTL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
     set(EMTL_Win->TxtArrow[0], MUIA_Disabled, TRUE);
     set(EMTL_Win->TxtArrow[1], MUIA_Disabled, TRUE);
     set(EMTL_Win->TxtArrowLg[0], MUIA_Disabled, TRUE);
     set(EMTL_Win->TxtArrowLg[1], MUIA_Disabled, TRUE);
     } /* if motion time line window open */
    if (EMIA_Win)
     {
     set(EMIA_Win->StrArrow[2][1], MUIA_Disabled, TRUE);
     } /* if motion time line window open */
    }
   if (EM_Win->KeysExist)
    {
    set(EM_Win->BT_UpdateKeys, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_ALL ), EM_Win->KeysExist);  // "All (%d)"
    set(EM_Win->BT_AllKeys, MUIA_Text_Contents, (IPTR)str);
    if (EMTL_Win)
     {
     sprintf(str, (char*)GetString( MSG_AGUI_KEYSEXIST ), EM_Win->KeysExist);  // "Keys Exist (%d)"
     set(EMTL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)str);
     } /* if motion time line window open */
    }
   else
    {
    set(EM_Win->BT_UpdateKeys, MUIA_Disabled, TRUE);
    set(EM_Win->BT_AllKeys, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_ALL0 ));  // "\33cAll (0)"
    if (EMTL_Win)
     {
     set(EMTL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NOOTHERKEYS ));  // "No Other Keys"
     } /* if motion time line window open */
    }
   if (EM_Win->ItemKeys > 1)
    set(EM_Win->FramePages, MUIA_Disabled, FALSE);
   else
    set(EM_Win->FramePages, MUIA_Disabled, TRUE);
   if (EM_Win->ItemKeys > 0)
    set(EM_Win->BT_DeleteAll, MUIA_Disabled, FALSE);
   else
    set(EM_Win->BT_DeleteAll, MUIA_Disabled, TRUE);
   if (EMTL_Win)
    {
    set(EMTL_Win->BT_Linear, MUIA_Selected, EM_Win->Linear);
    get(EMTL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EM_Win->TCB[item]);
    set(EMTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    DisableAll = ((EM_Win->MoItem != EMTL_Win->KeyItem)
	 || EM_Win->ItemKeys < 2);
    set(EMTL_Win->BT_AddKey, MUIA_Disabled, DisableAll);
    set(EMTL_Win->ValStr[0], MUIA_Disabled, DisableAll);
    set(EMTL_Win->TimeLineObj[0], MUIA_Disabled, DisableAll);
    set(EMTL_Win->CycleStr, MUIA_Disabled, DisableAll);
    set(EMTL_Win->StrArrow[0], MUIA_Disabled, DisableAll);
    set(EMTL_Win->StrArrow[1], MUIA_Disabled, DisableAll);
    if (EM_Win->ItemKeys > 2)
     set(EMTL_Win->BT_DelKey, MUIA_Disabled, FALSE);
    else
     {
     set(EM_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
     set(EMTL_Win->BT_DelKey, MUIA_Disabled, TRUE);
     }
    }
   break;
   } /* motion */

  case 1:
   {
   if (EC_Win->IsKey >= 0)
    {
    set(EC_Win->BT_DeleteKey, MUIA_Disabled, FALSE);
    }
   else
    {
    set(EC_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
    }
   if (EC_Win->PrevKey >= 0)
    {
    set(EC_Win->BT_PrevKey, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_PK ), EC_Win->PrevKey);  // "PK %d"
    set(EC_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
    if (ECTL_Win)
     {
     set(ECTL_Win->BT_PrevKey, MUIA_Disabled, FALSE);
     set(ECTL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
     } /* if color time line window open */
    }
   else
    {
    set(EC_Win->BT_PrevKey, MUIA_Disabled, TRUE);
    set(EC_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
    if (ECTL_Win)
     {
     set(ECTL_Win->BT_PrevKey, MUIA_Disabled, TRUE);
     set(ECTL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
     } /* if color time line window open */
    }
   if (EC_Win->NextKey >= 0)
    {
    set(EC_Win->BT_NextKey, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_NK ), EC_Win->NextKey);  // "NK %d"
    set(EC_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
    if (ECTL_Win)
     {
     set(ECTL_Win->BT_NextKey, MUIA_Disabled, FALSE);
     set(ECTL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
     set(ECTL_Win->TxtArrow[0], MUIA_Disabled, FALSE);
     set(ECTL_Win->TxtArrow[1], MUIA_Disabled, FALSE);
     set(ECTL_Win->TxtArrowLg[0], MUIA_Disabled, FALSE);
     set(ECTL_Win->TxtArrowLg[1], MUIA_Disabled, FALSE);
     } /* if color time line window open */
    }
   else
    {
    set(EC_Win->BT_NextKey, MUIA_Disabled, TRUE);
    set(EC_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
    if (ECTL_Win)
     {
     set(ECTL_Win->BT_NextKey, MUIA_Disabled, TRUE);
     set(ECTL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
     set(ECTL_Win->TxtArrow[0], MUIA_Disabled, TRUE);
     set(ECTL_Win->TxtArrow[1], MUIA_Disabled, TRUE);
     set(ECTL_Win->TxtArrowLg[0], MUIA_Disabled, TRUE);
     set(ECTL_Win->TxtArrowLg[1], MUIA_Disabled, TRUE);
     } /* if color time line window open */
    }
   if (EC_Win->KeysExist)
    {
    set(EC_Win->BT_UpdateKeys, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_ALL ) , EC_Win->KeysExist);  // "All (%d)"
    set(EC_Win->BT_UpdateAll, MUIA_Text_Contents, (IPTR)str);
    if (ECTL_Win)
     {
     sprintf(str, (char*)GetString( MSG_AGUI_KEYSEXIST ), EC_Win->KeysExist);  // "Keys Exist (%d)"
     set(ECTL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)str);
     } /* if color time line window open */
    }
   else
    {
    set(EC_Win->BT_UpdateKeys, MUIA_Disabled, TRUE);
    set(EC_Win->BT_UpdateAll, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_ALL0 ));  // "\33cAll (0)"
    if (ECTL_Win)
     {
     set(ECTL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NOOTHERKEYS ));  // "No Other Keys"
     } /* if color time line window open */
    }
   if (EC_Win->ItemKeys > 1)
    set(EC_Win->FramePages, MUIA_Disabled, FALSE);
   else
    set(EC_Win->FramePages, MUIA_Disabled, TRUE);
   if (EC_Win->ItemKeys > 0)
    set(EC_Win->BT_DeleteAll, MUIA_Disabled, FALSE);
   else
    set(EC_Win->BT_DeleteAll, MUIA_Disabled, TRUE);
   if (ECTL_Win)
    {
    set(ECTL_Win->BT_Linear, MUIA_Selected, EC_Win->Linear);
    get(ECTL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EC_Win->TCB[item]);
    set(ECTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    DisableAll = ((EC_Win->PalItem != ECTL_Win->KeyItem)
	 || EC_Win->ItemKeys < 2);
    set(ECTL_Win->BT_AddKey, MUIA_Disabled, DisableAll);
    set(ECTL_Win->ValStr[0], MUIA_Disabled, DisableAll);
    set(ECTL_Win->ValStr[1], MUIA_Disabled, DisableAll);
    set(ECTL_Win->ValStr[2], MUIA_Disabled, DisableAll);
    set(ECTL_Win->TimeLineObj[0], MUIA_Disabled, DisableAll);
    set(ECTL_Win->CycleStr, MUIA_Disabled, DisableAll);
    set(ECTL_Win->StrArrow[0], MUIA_Disabled, DisableAll);
    set(ECTL_Win->StrArrow[1], MUIA_Disabled, DisableAll);
    if (EC_Win->ItemKeys > 2)
     set(ECTL_Win->BT_DelKey, MUIA_Disabled, FALSE);
    else
     {
     set(EC_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
     set(ECTL_Win->BT_DelKey, MUIA_Disabled, TRUE);
     }
    }
   break;
   } /* color */

  case 2:
   {
   if (EE_Win->IsKey >= 0)
    {
    set(EE_Win->BT_DeleteKey, MUIA_Disabled, FALSE);
    }
   else
    {
    set(EE_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
    }
   if (EE_Win->PrevKey >= 0)
    {
    set(EE_Win->BT_PrevKey, MUIA_Disabled, FALSE);
    sprintf(str, "PK %d", EE_Win->PrevKey);
    set(EE_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
    if (EETL_Win)
     {
     set(EETL_Win->BT_PrevKey, MUIA_Disabled, FALSE);
     set(EETL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)str);
     } /* if ecosystem time line window open */
    }
   else
    {
    set(EE_Win->BT_PrevKey, MUIA_Disabled, TRUE);
    set(EE_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
    if (EETL_Win)
     {
     set(EETL_Win->BT_PrevKey, MUIA_Disabled, TRUE);
     set(EETL_Win->BT_PrevKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_PREV ));  // "\33cPrev"
     } /* if ecosystem time line window open */
    }
   if (EE_Win->NextKey >= 0)
    {
    set(EE_Win->BT_NextKey, MUIA_Disabled, FALSE);
    sprintf(str, "NK %d", EE_Win->NextKey);
    set(EE_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
    if (EETL_Win)
     {
     set(EETL_Win->BT_NextKey, MUIA_Disabled, FALSE);
     set(EETL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)str);
     set(EETL_Win->TxtArrow[0], MUIA_Disabled, FALSE);
     set(EETL_Win->TxtArrow[1], MUIA_Disabled, FALSE);
     set(EETL_Win->TxtArrowLg[0], MUIA_Disabled, FALSE);
     set(EETL_Win->TxtArrowLg[1], MUIA_Disabled, FALSE);
     } /* if ecosystem time line window open */
    }
   else
    {
    set(EE_Win->BT_NextKey, MUIA_Disabled, TRUE);
    set(EE_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
    if (EETL_Win)
     {
     set(EETL_Win->BT_NextKey, MUIA_Disabled, TRUE);
     set(EETL_Win->BT_NextKey, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NEXT ));  // "\33cNext"
     set(EETL_Win->TxtArrow[0], MUIA_Disabled, TRUE);
     set(EETL_Win->TxtArrow[1], MUIA_Disabled, TRUE);
     set(EETL_Win->TxtArrowLg[0], MUIA_Disabled, TRUE);
     set(EETL_Win->TxtArrowLg[1], MUIA_Disabled, TRUE);
     } /* if ecosystem time line window open */
    }
   if (EE_Win->KeysExist)
    {
    set(EE_Win->BT_UpdateKeys, MUIA_Disabled, FALSE);
    sprintf(str, (char*)GetString( MSG_AGUI_ALL ), EE_Win->KeysExist);  // "All (%d)"
    set(EE_Win->BT_UpdateAll, MUIA_Text_Contents, (IPTR)str);
    if (EETL_Win)
     {
     sprintf(str, (char*)GetString( MSG_AGUI_KEYSEXIST ), EE_Win->KeysExist);  // "Keys Exist (%d)"
     set(EETL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)str);
     } /* if ecosystem time line window open */
    }
   else
    {
    set(EE_Win->BT_UpdateKeys, MUIA_Disabled, TRUE);
    set(EE_Win->BT_UpdateAll, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_ALL0 ));  // "\33cAll (0)"
    if (EETL_Win)
     {
     set(EETL_Win->KeysExistTxt, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NOOTHERKEYS ));  // "No Other Keys"
     } /* if ecosystem time line window open */
    }
   if (EE_Win->ItemKeys > 1)
    set(EE_Win->FramePages, MUIA_Disabled, FALSE);
   else
    set(EE_Win->FramePages, MUIA_Disabled, TRUE);
   if (EE_Win->ItemKeys > 0)
    set(EE_Win->BT_DeleteAll, MUIA_Disabled, FALSE);
   else
    set(EE_Win->BT_DeleteAll, MUIA_Disabled, TRUE);
   if (EETL_Win)
    {
    set(EETL_Win->BT_Linear, MUIA_Selected, EE_Win->Linear);
    get(EETL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EE_Win->TCB[item]);
    set(EETL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    DisableAll = ((EE_Win->EcoItem != EETL_Win->KeyItem)
	 || EE_Win->ItemKeys < 2);
    set(EETL_Win->BT_AddKey, MUIA_Disabled, DisableAll);
    set(EETL_Win->ValStr[0], MUIA_Disabled, DisableAll);
    set(EETL_Win->TimeLineGroup, MUIA_Disabled, DisableAll);
    set(EETL_Win->CycleStr, MUIA_Disabled, DisableAll);
    set(EETL_Win->StrArrow[0], MUIA_Disabled, DisableAll);
    set(EETL_Win->StrArrow[1], MUIA_Disabled, DisableAll);
    if (EE_Win->ItemKeys > 2)
     set(EETL_Win->BT_DelKey, MUIA_Disabled, FALSE);
    else
     {
     set(EE_Win->BT_DeleteKey, MUIA_Disabled, TRUE);
     set(EETL_Win->BT_DelKey, MUIA_Disabled, TRUE);
     }
    }
   break;
   } /* ecosystem */

  } /* switch group */

} /* DisableKeyButtons() */

/************************************************************************/

#ifdef WCS_MUI_2_HACK

STATIC_FCN void MUI2_MenuCheck_Hack(void) // used locally only -> static, AF 25.7.2021
{
LONG WindowState;

DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_EDITING), EP_Win);

DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATABASE), DB_Win);

DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_DATAOPS), DO_Win);

DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_MAPPING), MP);

get(AboutWin, MUIA_Window_Open, &WindowState);
DoMethod(app, MUIM_Application_SetMenuCheck, ID_VERSION, WindowState);

if(Log_Win)
  get(Log_Win->LogWindow, MUIA_Window_Open, &WindowState);
else
  WindowState = 0;
DoMethod(app, MUIM_Application_SetMenuCheck, ID_LOG, WindowState);

DoMethod(app, MUIM_Application_SetMenuCheck, ID_CREDITS, CreditWin);

DoMethod(app, MUIM_Application_SetMenuCheck, ID_INFO, InfoWin);


} /* MUI2_MenuCheck_Hack() */

#endif /* WCS_MUI_2_HACK */

/***********************************************************************/

STATIC_FCN void InfoWin_Update(int flush) // used locally only -> static, AF 25.7.2021
{
char InfoData[80];
long now, chipavail, fastlarge, chiplarge;
struct NameInfo ModeName;
char *AppRexxName;

time((time_t *)&now);
strncpy(InfoData, ctime((time_t *)&now), 26);
InfoData[19] = 0;

set(InfoTime, MUIA_Text_Contents, (IPTR)&InfoData[11]);

strncpy(InfoData, ctime((time_t *)&now), 26);
InfoData[24] = 0;
strcpy(&InfoData[11], &InfoData[20]);

set(InfoDate, MUIA_Text_Contents, (IPTR)InfoData);

if(flush)
  {
  AllocMem(0xFFFFFFFF, MEMF_FAST);
  AllocMem(0xFFFFFFFF, MEMF_CHIP);
  } /* if flush */

now = AvailMem(MEMF_FAST);
chipavail = AvailMem(MEMF_CHIP);
fastlarge = AvailMem(MEMF_FAST | MEMF_LARGEST);
chiplarge = AvailMem(MEMF_CHIP | MEMF_LARGEST);

stcul_d(InfoData, chipavail);
set(InfoChipAvail, MUIA_Text_Contents, (IPTR)InfoData);

stcul_d(InfoData, now);
set(InfoFastAvail, MUIA_Text_Contents, (IPTR)InfoData);

stcul_d(InfoData, chiplarge);
set(InfoChipLarge, MUIA_Text_Contents, (IPTR)InfoData);

stcul_d(InfoData, fastlarge);
set(InfoFastLarge, MUIA_Text_Contents, (IPTR)InfoData);


sprintf(InfoData, "%1d", NoOfElMaps);
/*stcul_d(InfoData, NoOfElMaps);*/
set(InfoInterTopos, MUIA_Text_Contents, (IPTR)InfoData);

sprintf(InfoData, "%1d", topomaps);
/*stcul_d(InfoData, topomaps);*/
set(InfoMapTopos, MUIA_Text_Contents, (IPTR)InfoData);

/* There's probably a clever MUI way to do this in one DoMethod, but I don't care. */
get(app, MUIA_Application_Base, &AppRexxName);
set(InfoARexx, MUIA_Text_Contents, (IPTR)AppRexxName);

if(dbaseloaded)
  set(InfoDataBase, MUIA_Text_Contents, (IPTR)dbasename);
else
  set(InfoDataBase, MUIA_Text_Contents, (IPTR) GetString( MSG_AGUI_NONE ));  // " -none- "

if(paramsloaded)
  set(InfoPar, MUIA_Text_Contents, (IPTR)paramfile);
else
  set(InfoPar, MUIA_Text_Contents, (IPTR)GetString( MSG_AGUI_NONE ));  // " -none- "

InfoData[0] = 0;
if(GetDisplayInfoData(NULL, (UBYTE *)&ModeName, sizeof(ModeName), DTAG_NAME, WCSScrn->ViewPort.ColorMap->VPModeID/*ScrnData.ModeID*/)) // ScrnData.ModeID is always 0 if "Save"-Button in World Construction Set ScreenMode-Window was not pressed, i.e. if uer pressed "Use" instead))
  {
  strncpy(InfoData, (char*)ModeName.Name, 75);
  InfoData[75] = 0;
  } /* if */

set(InfoScreenMode, MUIA_Text_Contents, (IPTR)InfoData);

} /* InfoWin_Update() */

/*********************************************************************/

void Set_Param_Menu(short Group)
{

 switch (Group)
  {
  case 0:  // Motion Editor active
   {
   WCSNewMenus[MENU_STOP].nm_Type = NM_ITEM;    
   WCSNewMenus[MENU_STOP].nm_Label =  (STRPTR)NM_BARLABEL;    
   WCSNewMenus[MENU_STOP + 1].nm_Label =  GetString( MSG_AGUI_LOADMOTION );  // "Load Motion...";
   WCSNewMenus[MENU_STOP + 2].nm_Label =  GetString( MSG_AGUI_SAVEMOTION );  // "Save Motion...";
   WCSNewMenus[MENU_STOP + 1].nm_UserData = (APTR)(ID_EM_LOADALL);
   WCSNewMenus[MENU_STOP + 2].nm_UserData = (APTR)(ID_EM_SAVEALL);
   WCSNewMenus[MENU_STOP + 3].nm_Type = NM_ITEM;
   WCSNewMenus[MENU_STOP + 3].nm_Label =  GetString( MSG_AGUI_LOADACTIVE );  // "Load Active...";
   WCSNewMenus[MENU_STOP + 4].nm_Label =  GetString( MSG_AGUI_SAVEACTIVE );  // "Save Active...";
   WCSNewMenus[MENU_STOP + 3].nm_UserData = (APTR)(ID_EM_LOADCURRENT);
   WCSNewMenus[MENU_STOP + 4].nm_UserData = (APTR)(ID_EM_SAVECURRENT);
   break;
   }
  case 1:  // Color Editor active
   {
   WCSNewMenus[MENU_STOP].nm_Type = NM_ITEM;    
   WCSNewMenus[MENU_STOP].nm_Label =  (STRPTR)NM_BARLABEL;    
   WCSNewMenus[MENU_STOP + 1].nm_Label =  GetString( MSG_AGUI_LOADCOLOR );  // "Load Color...";
   WCSNewMenus[MENU_STOP + 2].nm_Label =  GetString( MSG_AGUI_SAVECOLOR );  // "Save Color...";
   WCSNewMenus[MENU_STOP + 1].nm_UserData = (APTR)(ID_EC_LOADALL);
   WCSNewMenus[MENU_STOP + 2].nm_UserData = (APTR)(ID_EC_SAVEALL);
   WCSNewMenus[MENU_STOP + 3].nm_Type = NM_ITEM;
   WCSNewMenus[MENU_STOP + 3].nm_Label =  GetString( MSG_AGUI_LOADACTIVE );  // "Load Active...";
   WCSNewMenus[MENU_STOP + 4].nm_Label =  GetString( MSG_AGUI_SAVEACTIVE );  // "Save Active...";
   WCSNewMenus[MENU_STOP + 3].nm_UserData = (APTR)(ID_EC_LOADCURRENT);
   WCSNewMenus[MENU_STOP + 4].nm_UserData = (APTR)(ID_EC_SAVECURRENT);
   break;
   }
  case 2:  // Ecosystem Editor active
   {
   WCSNewMenus[MENU_STOP].nm_Type = NM_ITEM;    
   WCSNewMenus[MENU_STOP].nm_Label =  (STRPTR)NM_BARLABEL;    
   WCSNewMenus[MENU_STOP + 1].nm_Label =  GetString( MSG_AGUI_LOADECO );  // "Load Eco...";
   WCSNewMenus[MENU_STOP + 2].nm_Label =  GetString( MSG_AGUI_SAVEECO );  // "Save Eco...";
   WCSNewMenus[MENU_STOP + 1].nm_UserData = (APTR)(ID_EE_LOADALL);
   WCSNewMenus[MENU_STOP + 2].nm_UserData = (APTR)(ID_EE_SAVEALL);
   WCSNewMenus[MENU_STOP + 3].nm_Type = NM_ITEM;
   WCSNewMenus[MENU_STOP + 3].nm_Label =  GetString( MSG_AGUI_LOADACTIVE );  // "Load Active...";
   WCSNewMenus[MENU_STOP + 4].nm_Label =  GetString( MSG_AGUI_SAVEACTIVE );  // "Save Active...";
   WCSNewMenus[MENU_STOP + 3].nm_UserData = (APTR)(ID_EE_LOADCURRENT);
   WCSNewMenus[MENU_STOP + 4].nm_UserData = (APTR)(ID_EE_SAVECURRENT);
   break;
   }
  case 3:
   {
   WCSNewMenus[MENU_STOP].nm_Type = NM_ITEM;    
   WCSNewMenus[MENU_STOP].nm_Label =  (STRPTR)NM_BARLABEL;    
   WCSNewMenus[MENU_STOP + 1].nm_Label =  GetString( MSG_AGUI_LOADSETTINGS );  // "Load Settings...";
   WCSNewMenus[MENU_STOP + 2].nm_Label =  GetString( MSG_AGUI_SAVESETTINGS );  // "Save Settings...";
   WCSNewMenus[MENU_STOP + 1].nm_UserData = (APTR)(ID_ES_LOAD);
   WCSNewMenus[MENU_STOP + 2].nm_UserData = (APTR)(ID_ES_SAVE);
   WCSNewMenus[MENU_STOP + 3].nm_Type = NM_END;
   WCSNewMenus[MENU_STOP + 3].nm_UserData = 0;
   break;
   }
  default:
   {
   WCSNewMenus[MENU_STOP].nm_Type = NM_END;    
   WCSNewMenus[MENU_STOP].nm_Label =  (STRPTR)0;    
   break;
   }
  } /* switch */

} /* Set_Param_Menu() */

/***********************************************************************/

void settextint(APTR Obj, long Val)
{
char Str[32];

 sprintf(Str, "%ld", Val);
 set(Obj, MUIA_Text_Contents, (IPTR)Str);
 
} /* settextint() */

/*************************************************************************/

void setfloat(APTR Obj, double Val)
{
char Str[32];

 sprintf(Str, "%f", Val);
 TrimZeros(Str);
 set(Obj, MUIA_String_Contents, (IPTR)Str);
 set(Obj, MUIA_String_DisplayPos, 0);
 set(Obj, MUIA_String_BufferPos, 0);
 
} /* setfloat() */

/*************************************************************************/

void nnsetfloat(APTR Obj, double Val)
{
char Str[32];

 sprintf(Str, "%f", Val);
 TrimZeros(Str);
 nnset(Obj, MUIA_String_Contents, (IPTR)Str);
 nnset(Obj, MUIA_String_DisplayPos, 0);
 nnset(Obj, MUIA_String_BufferPos, 0);
 
} /* nnsetfloat() */

/**********************************************************************/

