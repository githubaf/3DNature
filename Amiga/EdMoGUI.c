/* EdMoGUI.c (ne gisedmogui.c 14 Jan 1994 CXH)
** World Construction Set GUI for Motion Editing module.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

STATIC_FCN void Update_EMTL_Item(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN void Close_EMPL_Window(void); // used locally only -> static, AF 26.7.2021

void Make_EM_Window(void)
{
 short i;
 IPTR open;

 if (EM_Win)
  {
  DoMethod(EM_Win->MotionWin, MUIM_Window_ToFront);
  set(EM_Win->MotionWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),                                         // "Motion Editor"
               GetString( MSG_EDMOGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
               GetString( MSG_GLOBAL_OK ),                                                   // "OK"
               (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((EM_Win = (struct MotionWindow *)
	get_Memory(sizeof (struct MotionWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((EM_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_EM_Window(1);
  return;
  } /* if out of memory */
 memcpy(EM_Win->AltKF, KF, KFsize);
 EM_Win->AltKFsize = KFsize;
 EM_Win->AltKeyFrames = ParHdr.KeyFrames;

  Set_Param_Menu(0);

     EM_Win->MotionWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_EDMOGUI_MOTIONEDITOR ),  // "Motion Editor"
      MUIA_Window_ID		, MakeID('E','D','M','O'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2(GetString( MSG_EDMOGUI_OPTIONS ) ),                                                    // "Options"
          Child, EM_Win->BT_Settings[0] = KeyButtonFunc('1', (char*)GetString( MSG_EDMOGUI_PATHS )),       // "\33cPaths"
          Child, EM_Win->BT_Settings[1] = KeyButtonFunc('2', (char*)GetString( MSG_EDMOGUI_FRACTAL )),     // "\33cFractal"
          Child, EM_Win->BT_Settings[2] = KeyButtonFunc('3', (char*)GetString( MSG_EDMOGUI_HORIZON )),     // "\33cHorizon"
          Child, EM_Win->BT_Settings[3] = KeyButtonFunc('4', (char*)GetString( MSG_EDMOGUI_CELESTIAL )),   // "\33cCelestial"
          Child, EM_Win->BT_Settings[4] = KeyButtonFunc('5', (char*)GetString( MSG_EDMOGUI_REFLECTION )),  // "\33cReflection"
	  End, /* HGroup */
	Child, HGroup,
/* Editing panel */
	  Child, VGroup,
/* Motion list */
	    Child, Label((char*)GetString( MSG_EDMOGUI_PARAMETERLIST ) ),  // "\33c\0334Parameter List"
	    Child, EM_Win->LS_List = ListviewObject, 
	        MUIA_Listview_Input, TRUE,
              MUIA_Listview_List, ListObject, ReadListFrame, End,
              End, /* ListviewObject */
	    End, /* VGroup */

 	  Child, VGroup,
/* Interactive group */
	    Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_INTERACTIVEGROUP ), End,  // "\33c\0334Interactive Group"
	      Child, HGroup,
                Child, Label2("X"), 
                Child, EM_Win->ParTxt[0] = TextObject, TextFrame,
           		MUIA_FixWidthTxt, "0123456789012345", End,
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
                  Child, EM_Win->ValTxt[0] = StringObject, StringFrame,
			   MUIA_FixWidthTxt, "01234567890", End,
		  Child, EM_Win->L_IAArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
		  Child, EM_Win->R_IAArrow[0] = ImageButtonWCS(MUII_ArrowRight),
		  End, /* HGroup */
                End, /* HGroup */
	      Child, HGroup,
                Child, Label2("Y"),
                Child, EM_Win->ParTxt[2] = TextObject, TextFrame,
			  MUIA_FixWidthTxt, "0123456789012345", End,
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
		     Child, EM_Win->ValTxt[2] = StringObject, StringFrame,
			   MUIA_FixWidthTxt, "01234567890", End,
		     Child, EM_Win->L_IAArrow[2] = ImageButtonWCS(MUII_ArrowLeft),
		     Child, EM_Win->R_IAArrow[2] = ImageButtonWCS(MUII_ArrowRight),
		     End, /* HGroup */
                End, /* HGroup */
	      Child, HGroup,
                Child, Label2("Z"),
                Child, EM_Win->ParTxt[1] = TextObject, TextFrame,
			  MUIA_FixWidthTxt, "0123456789012345", End,
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
		     Child, EM_Win->ValTxt[1] = StringObject, StringFrame,
			   MUIA_FixWidthTxt, "01234567890", End,
		     Child, EM_Win->L_IAArrow[1] = ImageButtonWCS(MUII_ArrowLeft),
		     Child, EM_Win->R_IAArrow[1] = ImageButtonWCS(MUII_ArrowRight),
		     End, /* HGroup */
                End, /* HGroup */
	      Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	      Child, HGroup,
/*
                Child, Label2("A"),
*/
                Child, TextObject, TextFrame,
			  MUIA_Text_Contents, GetString( MSG_EDMOGUI_FOCALAZIMUTH ),  // "Focal Azimuth"
			  MUIA_FixWidthTxt, "0123456789012345", End,
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
		    Child, EM_Win->ValTxt[3] = StringObject, StringFrame,
			  MUIA_FixWidthTxt, "01234567890", End,
		  Child, EM_Win->L_IAArrow[3] = ImageButtonWCS(MUII_ArrowLeft),
		  Child, EM_Win->R_IAArrow[3] = ImageButtonWCS(MUII_ArrowRight),
		  End, /* HGroup */
                End, /* HGroup */
	      Child, HGroup,
/*
                Child, Label2("Q"),
*/
                Child, TextObject, TextFrame,
			MUIA_Text_Contents, GetString( MSG_EDMOGUI_FOCALDISTANCE ),  // "Focal Distance"
			MUIA_FixWidthTxt, "0123456789012345", End,
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
		  Child, EM_Win->ValTxt[4] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567890", End,
		  Child, EM_Win->L_IAArrow[4] = ImageButtonWCS(MUII_ArrowLeft),
		  Child, EM_Win->R_IAArrow[4] = ImageButtonWCS(MUII_ArrowRight),
		  End, /* HGroup */
                End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2(GetString( MSG_EDMOGUI_SENSITIVITY )),  // "Sensitivity "
                Child, EM_Win->IA_SensStr = StringObject, StringFrame,
			MUIA_String_Integer, IA_Sensitivity,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, EM_Win->IA_L_SensArrow = ImageButtonWCS(MUII_ArrowLeft),
                Child, EM_Win->IA_R_SensArrow = ImageButtonWCS(MUII_ArrowRight),

		Child, EM_Win->BT_Sens[0] = KeyButtonFunc('l', (char*)GetString( MSG_EDMOGUI_LOW )),   // "\33cLow"
		Child, EM_Win->BT_Sens[1] = KeyButtonFunc('e', (char*)GetString( MSG_EDMOGUI_MED )),   // "\33cMed"
		Child, EM_Win->BT_Sens[2] = KeyButtonFunc('h', (char*)GetString( MSG_EDMOGUI_HIGH )),  // "\33cHigh"
	        End, /* HGroup */
	      End, /* VGroup */

	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, MUIA_InnerLeft, 0, End,

/* Frame stuff */
            Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_KEYFRAMES ), End,     // "\33c\0334Key Frames"
              Child, HGroup,
                Child, EM_Win->BT_PrevKey = KeyButtonFunc('v', (char*)GetString( MSG_EDMOGUI_PREV ) ),  // "\33cPrev"
                Child, Label2(GetString( MSG_EDMOGUI_FRAME )),                                          // "Frame"
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
                  Child, EM_Win->Str[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                  Child, EM_Win->StrArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
                  Child, EM_Win->StrArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
                  End, /* HGroup */
                Child, EM_Win->BT_NextKey = KeyButtonFunc('x', (char*)GetString( MSG_EDMOGUI_NEXT )),       // "\33cNext"
                End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EM_Win->BT_MakeKey = KeyButtonFunc('m', (char*)GetString( MSG_EDMOGUI_MAKEKEY )),    // "\33cMake Key"
                Child, EM_Win->BT_GroupKey = KeyButtonObject('g'),
			MUIA_InputMode, MUIV_InputMode_Toggle,
			MUIA_Selected, TRUE,
		 	MUIA_Text_Contents, GetString( MSG_EDMOGUI_GROUP ), End,                     // "\33cGroup"
                Child, EM_Win->BT_UpdateKeys = KeyButtonFunc('u', (char*)GetString( MSG_EDMOGUI_UPDATE )),  // "\33cUpdate"
                Child, EM_Win->BT_AllKeys = KeyButtonObject('('),
			MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, GetString( MSG_EDMOGUI_ALL0 ), End,  // "\33cAll (0)"
		End, /* HGroup */
              Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EM_Win->BT_DeleteKey = KeyButtonFunc(127, (char*)GetString( MSG_EDMOGUI_DELETE )),     // "\33c\33uDel\33nete"
                Child, EM_Win->BT_DeleteAll = KeyButtonFunc('d', (char*)GetString( MSG_EDMOGUI_DELETEALL )),  // "\33cDelete All"
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, EM_Win->FramePages = VGroup,
                  Child, EM_Win->BT_TimeLines = KeyButtonFunc('t', (char*)GetString( MSG_EDMOGUI_TIMELINES )),  // "\33cTime Lines "
	          End, /* VGroup */
                Child, EM_Win->BT_BankKeys = KeyButtonFunc('b', (char*)GetString( MSG_EDMOGUI_BANK )),   // "\33cBank "
                Child, EM_Win->BT_KeyScale = KeyButtonFunc('s', (char*)GetString( MSG_EDMOGUI_SCALE )),  // "\33cScale "
                Child, EM_Win->BT_SunSet = KeyButtonFunc('n', (char*)GetString( MSG_EDMOGUI_SUN )),      // "\33cSun "
		End, /* HGroup */
	      End, /* VGroup */

	    End, /* VGroup */
	  End, /* HGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, MUIA_VertWeight, 1, End,

/* Buttons at bottom */
        Child, HGroup, MUIA_Group_SameWidth, TRUE,
          Child, EM_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_EDMOGUI_KEEP )),           // "\33cKeep"
          Child, EM_Win->BT_WinSize = KeyButtonFunc('w', (char*)GetString( MSG_EDMOGUI_LISTWIN )),      // "\33cList Win "
          Child, EM_Win->BT_Interactive = KeyButtonFunc('i', (char*)GetString( MSG_EDMOGUI_CAMVIEW )),  // "\33cCam View "
          Child, EM_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),        // "\33cCancel"
          End, /* HGroup */

        End, /* VGroup */
      End; /* WindowObject EM_Win->MotionWin */

  if (! EM_Win->MotionWin)
   {
   Close_EM_Window(1);
   User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),  // "Motion Editor"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),            // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EM_Win->MotionWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(EM_Win->MotionWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EM_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, EM_Win->BT_Apply, ID_EM_APPLY,
   EM_Win->BT_Cancel, ID_EM_CLOSE, EM_Win->BT_WinSize, ID_EMPL_WINDOW,
   EM_Win->BT_Interactive, ID_EMIA_WINDOW,
   EM_Win->BT_MakeKey, ID_EM_MAKEKEY, EM_Win->BT_UpdateKeys, ID_EM_UPDATEKEYS,
   EM_Win->BT_NextKey, ID_EM_NEXTKEY, EM_Win->BT_PrevKey, ID_EM_PREVKEY,
   EM_Win->BT_DeleteKey, ID_EM_DELETEKEY, EM_Win->BT_TimeLines, ID_EMTL_WINDOW,
   EM_Win->BT_BankKeys, ID_EM_BANKKEYS, EM_Win->BT_KeyScale, ID_PS_WINGRP0,
   EM_Win->BT_SunSet, ID_TS_WINDOW,
   EM_Win->BT_DeleteAll, ID_EM_DELETEALL,
   EM_Win->BT_Sens[0], ID_EM_SENS(0),
   EM_Win->BT_Sens[1], ID_EM_SENS(1),
   EM_Win->BT_Sens[2], ID_EM_SENS(2),
   EM_Win->BT_Settings[0], ID_SB_SETPAGE(2),
   EM_Win->BT_Settings[1], ID_SB_SETPAGE(5),
   EM_Win->BT_Settings[2], ID_SB_SETPAGE(7),
   EM_Win->BT_Settings[3], ID_SB_SETPAGE(7),
   EM_Win->BT_Settings[4], ID_SB_SETPAGE(7),
   NULL);

/* set LW style enter command for making key */
  DoMethod(EM_Win->MotionWin, MUIM_Notify, MUIA_Window_InputEvent,
	GetString( MSG_EDMOGUI_NUMERICPADENTER ), app, 2, MUIM_Application_ReturnID, ID_EM_MAKEKEY);  // "numericpad enter"

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app,
    EM_Win->StrArrow[0][0], ID_EM_SARROWLEFT(0),
    EM_Win->StrArrow[0][1], ID_EM_SARROWRIGHT(0), NULL);
  for (i=0; i<5; i++)
   MUI_DoNotiPresFal(app,
    EM_Win->L_IAArrow[i], ID_EM_IAARROWLEFT(i),
    EM_Win->R_IAArrow[i], ID_EM_IAARROWRIGHT(i), NULL);

/* link sensitivity arrows to application */
   MUI_DoNotiPresFal(app, EM_Win->IA_L_SensArrow, ID_EM_SENSLARROW,
    EM_Win->IA_R_SensArrow, ID_EM_SENSRARROW, NULL);

/* link sensitivity string to application */
   DoMethod(EM_Win->IA_SensStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EM_SENSSTR);

/* link string gadgets to application */
  DoMethod(EM_Win->Str[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EM_FRAMESTR(0));
  for (i=0; i<5; i++)  // was i<6 -> ValTxt has only index 0...4, see definition EM_Win->ValTxt[5]  AF, 12.July2021
   {
   DoMethod(EM_Win->ValTxt[i], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EM_VALSTR(i));
   } /* for i=0... */

/* link list to application */
  DoMethod(EM_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EM_LIST);

/* Set cycle chain for frame strings */

/* Set tab cycle chain */
  DoMethod(EM_Win->MotionWin, MUIM_Window_SetCycleChain,
	EM_Win->LS_List,
	EM_Win->ValTxt[0],EM_Win->ValTxt[2],EM_Win->ValTxt[1],
	EM_Win->ValTxt[3],EM_Win->ValTxt[4],
	EM_Win->IA_SensStr, EM_Win->BT_Sens[0],
	EM_Win->BT_Sens[1], EM_Win->BT_Sens[2],
	EM_Win->BT_PrevKey, EM_Win->Str[0], EM_Win->BT_NextKey,
	EM_Win->BT_MakeKey, EM_Win->BT_GroupKey, EM_Win->BT_UpdateKeys,
	EM_Win->BT_AllKeys,
	EM_Win->BT_DeleteKey, EM_Win->BT_DeleteAll,
	EM_Win->BT_TimeLines, EM_Win->BT_BankKeys, EM_Win->BT_KeyScale,
	EM_Win->BT_SunSet,
	EM_Win->BT_Apply, EM_Win->BT_WinSize, EM_Win->BT_Interactive,
	EM_Win->BT_Cancel, NULL);

/* set return cycle chain */
  DoMethod(EM_Win->ValTxt[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	EM_Win->MotionWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EM_Win->ValTxt[2]);
  DoMethod(EM_Win->ValTxt[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	EM_Win->MotionWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EM_Win->ValTxt[1]);
  DoMethod(EM_Win->ValTxt[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	EM_Win->MotionWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EM_Win->ValTxt[3]);
  DoMethod(EM_Win->ValTxt[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	EM_Win->MotionWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EM_Win->ValTxt[4]);
  DoMethod(EM_Win->ValTxt[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	EM_Win->MotionWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EM_Win->ValTxt[0]);

/* Set active gadget */
  set(EM_Win->MotionWin, MUIA_Window_ActiveObject, (IPTR)EM_Win->LS_List);

/* Set list view to Motion list */
  Set_EM_List(0);

/* Set list to first item */
  EM_Win->MoItem = 0;
  set(EM_Win->LS_List, MUIA_List_Active, (long)EM_Win->MoItem);

/* Set frame string */
  EM_Win->Frame = 0;
  set(EM_Win->Str[0], MUIA_String_Integer, EM_Win->Frame);

/* disable delete & make keys as appropriate */
  UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
  DisableKeyButtons(0);
  GetKeyTableValues(0, 0, 1);


/* Open window */
  set(EM_Win->MotionWin, MUIA_Window_Open, TRUE);
  get(EM_Win->MotionWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EM_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EM_Win->MotionWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EM_ACTIVATE);

/* Get Window structure pointer */
  get(EM_Win->MotionWin, MUIA_Window_Window, &EM_Win->Win);

/* this comes after window opening so that value texts do not get stretched
   if an item is larger than 10 characters */
  Set_EM_Item(EM_Win->MoItem);

/* Set azimuth and focus distance */
  Set_Radial_Txt(0);

} /* Make_EM_Window() */

/*********************************************************************/

void Close_EM_Window(short apply)
{

  if (EMIA_Win) Close_EMIA_Window(-1);
  if (EMTL_Win) Close_EMTL_Window(apply);
  if (EMPL_Win) Close_EMPL_Window();
  if (TS_Win)   Close_TS_Window(apply);
  if (EM_Win)
   {
   if (EM_Win->AltKF)
    {
    if (apply) free_Memory(EM_Win->AltKF, EM_Win->AltKFsize);
    else
     {
     MergeKeyFrames(EM_Win->AltKF, EM_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 0);
     free_Memory(EM_Win->AltKF, EM_Win->AltKFsize);
     ResetTimeLines(0);
     } /* else discard changes */
    } /* if */
   if (EM_Win->MotionWin)
    {
    if (apply)
     {
     FixPar(0, 0x0001);
     FixPar(1, 0x0001);
     }
    else
     {
     struct clipbounds cb;

     if (MP && MP->ptsdrawn)
      {
      setclipbounds(MapWind0, &cb);
      ShowPaths_Map(&cb);
      }
     UndoPar(0, 0x0001);
     if (MP && MP->ptsdrawn)
      {
      ShowView_Map(&cb);
      ShowPaths_Map(&cb);
      }
     }
    set(EM_Win->MotionWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
    DoMethod(app, OM_REMMEMBER, EM_Win->MotionWin);
    MUI_DisposeObject(EM_Win->MotionWin);
    } /* if window created */
   free_Memory(EM_Win, sizeof (struct MotionWindow));
   EM_Win = NULL;
   } /* if */

 if (! apply)
  Par_Mod &= 0x1110;

} /* Close_EM_Window() */

/*********************************************************************/

void Handle_EM_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EM_Window();
   return;
   } /* Open Motion Editor Window */

  if (! EM_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Motion Editing window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EM_AUTOCENTER:
      if (EMIA_Win)
       {
       autocenter();
       findfocprof();
       Set_EM_Item(EM_Win->MoItem);
       Set_Radial_Txt(2);
       } /* if interactive open */
      else
       {
       User_Message(GetString( MSG_EDMOGUI_MOTIONEDITORAUTOCENTER ),                          // "Motion Editor: Auto Center"
                    GetString( MSG_EDMOGUI_INTERACTIVEMODULEMUSTBEOPENBEFOREAUTOCENTERING ),  // "Interactive module must be open before auto centering!"
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");
       } /* else */
      break;
     case ID_EM_MAKEKEY:
      {
      MakeMotionKey();
      break;
      } /* Make Key frame */
     case ID_EM_UPDATEKEYS:
      {
      IPTR UpdateGp, UpdateAll;
      struct clipbounds cb;

      if (MP && MP->ptsdrawn)
       {
       setclipbounds(MapWind0, &cb);
       ShowPaths_Map(&cb);
       } /* if */
      get(EM_Win->BT_GroupKey, MUIA_Selected, &UpdateGp);
      get(EM_Win->BT_AllKeys, MUIA_Selected, &UpdateAll);
      UpdateKeyFrames(EM_Win->Frame, 0, EM_Win->MoItem,
	 (short)UpdateAll, (short)UpdateGp);
      if (BuildKeyTable())
       {
       short frame;

       frame = EM_Win->Frame > KT_MaxFrames ? KT_MaxFrames: EM_Win->Frame;
       for (i=0; i<USEDMOTIONPARAMS; i++)
        {
        if (KT[i].Key)
         PAR_FIRST_MOTION(i) = KT[i].Val[0][frame];
        } /* for i=0... */
       if (settings.lookahead)
        {
        frame = EM_Win->Frame + settings.lookaheadframes > KT_MaxFrames ?
		 KT_MaxFrames: EM_Win->Frame + settings.lookaheadframes;
        if (KT[0].Key)
         PAR_FIRST_MOTION(3) = KT[0].Val[0][frame];
        if (KT[1].Key)
         PAR_FIRST_MOTION(4) = KT[1].Val[0][frame];
        if (KT[2].Key)
         PAR_FIRST_MOTION(5) = KT[2].Val[0][frame];
	} /* if look ahead */
       FreeKeyTable();
       Set_EM_Item(EM_Win->MoItem);
       Set_Radial_Txt(2);
       } /* if key table */
      if (MP && MP->ptsdrawn)
       {
       ShowPaths_Map(&cb);
       }
      Par_Mod |= 0x0001;
      break;
      } /* Update Key frames */
     case ID_EM_NEXTKEY:
      {
      EM_Win->Frame = EM_Win->NextKey;
      set(EM_Win->Str[0], MUIA_String_Integer, EM_Win->Frame);
      break;
      } /* Next Key */
     case ID_EM_PREVKEY:
      {
      EM_Win->Frame = EM_Win->PrevKey;
      set(EM_Win->Str[0], MUIA_String_Integer, EM_Win->Frame);
      break;
      } /* Prev Key */
     case ID_EM_DELETEKEY:
      {
      IPTR DeleteGp, DeleteAll;
      struct clipbounds cb;

      if (MP && MP->ptsdrawn)
       {
       setclipbounds(MapWind0, &cb);
       ShowPaths_Map(&cb);
       } /* if */

      get(EM_Win->BT_GroupKey, MUIA_Selected, &DeleteGp);
      get(EM_Win->BT_AllKeys, MUIA_Selected, &DeleteAll);
      if (DeleteKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem,
	 (short)DeleteAll, (short)DeleteGp))
       {
       UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
       GetKeyTableValues(0, EM_Win->MoItem, 1);
       if (DeleteAll || DeleteGp)
        {
        Set_EM_List(1);
        if (EMPL_Win)
         DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
	} /* if multiple keys deleted */
       else
        {
        if (! CountKeyFrames(0, EM_Win->MoItem))
         {
         sprintf(EM_Win->Motionname[EM_Win->MoItem], "\33n%s", varname[EM_Win->MoItem]);
         DoMethod(EM_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
         if (EMPL_Win)
          DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
	 } /* if no more key frames */
	} /* else */
       Set_EM_Item(EM_Win->MoItem);
       Set_Radial_Txt(2);
       ResetTimeLines(-1);
       } /* if key frame deleted */
      DisableKeyButtons(0);
      Par_Mod |= 0x0001;
      if (MP && MP->ptsdrawn)
       {
       ShowPaths_Map(&cb);
       }
      break;
      } /* delete key */
     case ID_EM_DELETEALL:
      {
      struct clipbounds cb;

      sprintf(str, (char*)GetString( MSG_EDMOGUI_DELETEALLKEYFRAMES ), varname[EM_Win->MoItem]);  // "Delete all %s Key Frames?"
      if (User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMOTION ),                       // "Parameters Module: Motion"
                           (CONST_STRPTR)str, 
                           GetString( MSG_GLOBAL_OKCANCEL ),                                     // "OK|Cancel"
                           (CONST_STRPTR)"oc", 1))
       {
       if (MP && MP->ptsdrawn)
        {
        setclipbounds(MapWind0, &cb);
        ShowPaths_Map(&cb);
        } /* if */
       for (i=ParHdr.KeyFrames-1; i>=0; i--)
        {
        if (KF[i].MoKey.Group == 0)
         {
         if (KF[i].MoKey.Item == EM_Win->MoItem)
          DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, KF[i].MoKey.Item, 0, 0);
         } /* if group match */
        } /* for i=0... */
       sprintf(EM_Win->Motionname[EM_Win->MoItem], "\33n%s", varname[EM_Win->MoItem]);
       DoMethod(EM_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
       if (EMPL_Win)
        DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
       UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
       GetKeyTableValues(0, EM_Win->MoItem, 1);
       ResetTimeLines(-1);
       DisableKeyButtons(0);
       Par_Mod |= 0x0001;
       if (MP && MP->ptsdrawn)
        {
        ShowPaths_Map(&cb);
        }
       } /* if */
      break;
      } /* delete all */
     case ID_EM_BANKKEYS:
      {
      if (CreateBankKeys())
       {
       UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
       GetKeyTableValues(0, EM_Win->MoItem, 1);
       Set_EM_List(1);
       if (EMPL_Win)
        DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
       ResetTimeLines(-1);
       DisableKeyButtons(0);
       Par_Mod |= 0x0001;
       } /* if operation success */
      break;
      } /* create bank keys from motion path */
     case ID_EM_SAVEALL:
      {
      if (saveparams(0x01, -1, 0) == 0)
       FixPar(0, 0x0001);
      break;
      } /* Save entire motion */
     case ID_EM_SAVECURRENT:
      {
      saveparams(0x01, EM_Win->MoItem, 0);
      break;
      } /* Save current motion */
     case ID_EM_LOADALL:
      {
      struct clipbounds cb;

      if (MP && MP->ptsdrawn)
       {
       setclipbounds(MapWind0, &cb);
       ShowPaths_Map(&cb);
       } /* if */
      if ((loadparams(0x01, -1)) == 1)
       {
       FixPar(0, 0x0001);
       FixPar(1, 0x0001);
       } /* if load successful */
      UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
      GetKeyTableValues(0, EM_Win->MoItem, 1);
      Set_EM_List(1);
      Set_EM_Item(EM_Win->MoItem);
      Set_Radial_Txt(0);
      if (EMIA_Win) Init_IA_View(1);
      ResetTimeLines(-1);
      DisableKeyButtons(0);
      if (MP && MP->ptsdrawn)
       {
       ShowPaths_Map(&cb);
       }
      break;
      } /* Load entire motion */
     case ID_EM_LOADCURRENT:
      {
      struct clipbounds cb;

      if (MP && MP->ptsdrawn)
       {
       setclipbounds(MapWind0, &cb);
       ShowPaths_Map(&cb);
       } /* if */
      loadparams(0x01, EM_Win->MoItem);
      UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
      GetKeyTableValues(0, EM_Win->MoItem, 1);
      Set_EM_List(1);
      Set_EM_Item(EM_Win->MoItem);
      if (EM_Win->MoItem < 6) Set_Radial_Txt(0);
      if (EMIA_Win) Init_IA_Item(EM_Win->MoItem, 1);
      ResetTimeLines(-1);
      DisableKeyButtons(0);
      if (MP && MP->ptsdrawn)
       {
       ShowPaths_Map(&cb);
       }
      break;
      } /* Load current motion */
     case ID_EM_APPLY:
      {
      Close_EM_Window(1);
      break;
      } /* Apply changes to Motion arrays */
     case ID_EM_CLOSE:
      {
      Close_EM_Window(0);
      break;
      } /* Close and cancel any changes since window opened */
     case ID_EM_CLOSEQUERY:
      {
      if (KFsize != EM_Win->AltKFsize || memcmp(KF, EM_Win->AltKF, KFsize)
		|| memcmp(&MoPar, &UndoMoPar[0], sizeof (MoPar)))
       Close_EM_Window(CloseWindow_Query(GetString( MSG_EDMOGUI_MOTIONEDITOR ) ));  // "Motion Editor"
      else
       Close_EM_Window(1);
      break;
      } /* Close and cancel any changes since window opened */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    LONG value;

    i = WCS_ID - ID_EM_SENS(0);

    value = pow((double)10, (double)(i + 1));
    set(EM_Win->IA_SensStr, MUIA_String_Integer, value);
    break;
    } /* Sensitivity buttons */

   case GP_LIST1:
    {
    IPTR data;

    get(EM_Win->LS_List, MUIA_List_Active, &data);
    EM_Win->MoItem = data;

    if (EMPL_Win)
     {
     set(EMPL_Win->LS_List, MUIA_List_Active, data);
     } /* if parameter list window open */
    UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
    GetKeyTableValues(0, EM_Win->MoItem, 1);
    Set_EM_Item(EM_Win->MoItem);
    Set_Radial_Txt(2);
    if (InterWind0)
     SetWindowTitles(InterWind0, (STRPTR) varname[EM_Win->MoItem], (UBYTE *)-1);
    if (EMTL_Win && (EM_Win->IsKey >= 0 || EM_Win->PrevKey >= 0	 || EM_Win->NextKey >= 0))
     {
     if (Set_EMTL_Item(EM_Win->MoItem))
      {
      Set_EMTL_Data();
      MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      } /* if */
     } /* if Time Line window open */
    DisableKeyButtons(0);
    break;
    } /* LIST1 */

   case GP_STRING1:
    {
    char *floatdata;
    double value;

    i = WCS_ID - ID_EM_VALSTR(0);

    get(EM_Win->ValTxt[i], MUIA_String_Contents, &floatdata);
    value = atof(floatdata);
    if (i < 3)
     {
     PAR_FIRST_MOTION(item[i]) = value;
     boundscheck(item[i]);
     sprintf(str, "%f", PAR_FIRST_MOTION(item[i]));
     set(EM_Win->ValTxt[i], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[i], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* if rectilinear motion */

    else
     {
     oldazimuth = azimuth;
     if (i == 3)
      {
      azimuth = value * PiOver180;
      } /* if azimuth changed */
     else
      {
      focdist = value;
      if (focdist < .00001)
       focdist = .00001;
      } /* else focus distance changed */
     if (item[0] < 3)
      {
      reversecamcompute();
      compass(oldazimuth, azimuth);
      if (MP && MP->ptsdrawn)
       {
       Update_InterMap(1);
       Update_InterMap(3);
       Update_InterMap(4);
       } /* if map interactive */
      } /* if camera interactive group */
     else
      {
      reversefoccompute();	/* recomputes lat & lon */
      compass(oldazimuth, azimuth);
      if (MP && MP->ptsdrawn)
       {
       Update_InterMap(2);
       } /* if map interactive */
      } /* else focus interactive group */
     sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
     set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
     sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
     set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* else radial motion */
    break;
    } /* Value strings in interactive group */

   case GP_STRING2:
    {
    IPTR data;

    get(EM_Win->IA_SensStr, MUIA_String_Integer, &data);
    if (data > 1000)
     data = 1000;
    if (data < 1)
     data = 1;
    IA_Sensitivity = data;
    SetIncrements(EM_Win->MoItem);
    break;
    } /* STRING2 */

   case GP_STRING3:
    {
    IPTR data;

    i = WCS_ID - ID_EM_FRAMESTR(0);
    get(EM_Win->Str[i], MUIA_String_Integer, &data);
    switch (i)
     {
     case 0:
      {
      EM_Win->Frame = data;
      if (EMIA_Win)
       nnset(EMIA_Win->Str[1], MUIA_String_Integer, data);
      if (EM_Win->Frame < 0)
       {
       EM_Win->Frame = 0;
       } /* if frame < 0 */

      UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
      DisableKeyButtons(0);
      GetKeyTableValues(0, EM_Win->MoItem, 1);
      Set_EM_Item(EM_Win->MoItem);
      Set_Radial_Txt(2);
      if (MP && MP->ptsdrawn)
       {
       struct clipbounds cb;

       setclipbounds(MapWind0, &cb);
       ShowView_Map(&cb);
       } /* if map window */
      if (EMTL_Win)
       {
       IPTR data2;

       get(EMTL_Win->Prop[2], MUIA_Prop_First, &data2);
       data = (100.0 * ((float)EM_Win->Frame / (float)EMTL_Win->Frames));
       if (data != data2 && ! EM_Win->PropBlock)
        { 
        set(EMTL_Win->Prop[2], MUIA_Prop_First, data);
        EM_Win->StrBlock = 1;
        } /* if */      
       EM_Win->PropBlock = 0;
       if (EM_Win->IsKey >= 0)
        {
        EMTL_Win->ActiveKey = GetActiveKey(SKT[0], EM_Win->Frame);
        sprintf(str, "%d", EM_Win->Frame);
        set(EMTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
        MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWUPDATE);
	} /* if key frame */
       } /* if time line window open */
      break;
      } /* frame counter */
     } /* switch i */
    break;
    } /* STRING3 */

   case GP_ARROW1:
    {
    IPTR data;

    i = WCS_ID - ID_EM_SARROWLEFT(0);
    get(EM_Win->Str[i], MUIA_String_Integer, &data);
    if (i == 0)
	 set(EM_Win->Str[i], MUIA_String_Integer, (data > 0 ? data - 1: 0));
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    IPTR data;

    i = WCS_ID - ID_EM_SARROWRIGHT(0);
    get(EM_Win->Str[i], MUIA_String_Integer, &data);
    set(EM_Win->Str[i], MUIA_String_Integer, data + 1);
    break;
    } /* ARROW2 */

   case GP_ARROW3:
    {
    i = WCS_ID - ID_EM_IAARROWLEFT(0);
    if (i < 3)
     {
     if (item[i] == 1 || item[i] == 2 || item[i] == 4 || item[i] == 5)
      {
      PAR_FIRST_MOTION(item[i]) -= (.001 * IA_Sensitivity);
      }
     else
      {
      if (item[0] == 16 || item[0] == 27 || item[0] == 30)
       PAR_FIRST_MOTION(item[i]) += incr[i];
      else
       PAR_FIRST_MOTION(item[i]) -= incr[i];
      }
     sprintf(str, "%f", PAR_FIRST_MOTION(item[i]));
     set(EM_Win->ValTxt[i], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[i], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* if rectilinear motion */

    else
     {

     if (i == 3)
      {
      if (item[0] < 3) modifycampt(1, 0, SELECTDOWN);
      else modifyfocpt(1, 0, SELECTDOWN);
      }
     else
      {
      if (item[0] < 3) modifycampt(0, -1, SELECTDOWN);
      else modifyfocpt(0, 1, SELECTDOWN);
      }
     sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
     set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
     sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
     set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* else radial motion */
    break;
    } /* ARROW3 */

   case GP_ARROW4:
    {
    i = WCS_ID - ID_EM_IAARROWRIGHT(0);
    if (i < 3)
     {
     if (item[i] == 1 || item[i] == 2 || item[i] == 4 || item[i] == 5)
      {
      PAR_FIRST_MOTION(item[i]) += (.001 * IA_Sensitivity);
      }
     else
      {
      if (item[0] == 16 || item[0] == 27 || item[0] == 30)
       PAR_FIRST_MOTION(item[i]) -= incr[i];
      else
       PAR_FIRST_MOTION(item[i]) += incr[i];
      }
     sprintf(str, "%f", PAR_FIRST_MOTION(item[i]));
     set(EM_Win->ValTxt[i], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[i], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* if rectilinear motion */

    else
     {

     if (i == 3)
      {
      if (item[0] < 3) modifycampt(-1, 0, SELECTDOWN);
      else modifyfocpt(-1, 0, SELECTDOWN);
      }
     else
      {
      if (item[0] < 3) modifycampt(0, 1, SELECTDOWN);
      else modifyfocpt(0, -1, SELECTDOWN);
      }
     sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
     set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
     sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
     set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)str);
     set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
     Set_Radial_Txt(1);
     } /* else radial motion */
    sprintf(str, "%f", PAR_FIRST_MOTION(EM_Win->MoItem));
    break;
    } /* ARROW4 */

   case GP_ARROW5:
    {
    IPTR data;

    get(EM_Win->IA_SensStr, MUIA_String_Integer, &data);
    if (data > 1)
     set(EM_Win->IA_SensStr, MUIA_String_Integer, data - 1);    
    break;
    } /* ARROW5 */

   case GP_ARROW6:
    {
    IPTR data;

    get(EM_Win->IA_SensStr, MUIA_String_Integer, &data);
    if (data < 1000)
     set(EM_Win->IA_SensStr, MUIA_String_Integer, data + 1);    
    break;
    } /* ARROW6 */

   } /* switch gadget group */

} /* Handle_EM_Window() */

/*********************************************************************/

void Set_EM_Item(short i)
{
 SetIncrements(i);
 if (i == item[0]) EM_Win->IA_Item = 0;
 else if (i == item[1]) EM_Win->IA_Item = 1;
 else EM_Win->IA_Item = 2;

/* set values in interactive group texts */
 if (incr[0] == 0.0)
  {
  set(EM_Win->ParTxt[0], MUIA_Text_Contents, (IPTR)"\0");
  sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
  set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)"\0");
  set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[0], MUIA_Disabled, TRUE);
  set(EM_Win->R_IAArrow[0], MUIA_Disabled, TRUE);
  }
 else
  {
  set(EM_Win->ParTxt[0], MUIA_Text_Contents, (IPTR)varname[item[0]]);
  sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
  set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[0], MUIA_Disabled, FALSE);
  set(EM_Win->R_IAArrow[0], MUIA_Disabled, FALSE);
  } /* else */
 if (incr[1] == 0.0)
  {
  set(EM_Win->ParTxt[1], MUIA_Text_Contents, (IPTR)"\0");
  sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
  set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)"\0");
  set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[1], MUIA_Disabled, TRUE);
  set(EM_Win->R_IAArrow[1], MUIA_Disabled, TRUE);
  } /* if only one item active */
 else
  {
  set(EM_Win->ParTxt[1], MUIA_Text_Contents, (IPTR)varname[item[1]]);
  sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
  set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[1], MUIA_Disabled, FALSE);
  set(EM_Win->R_IAArrow[1], MUIA_Disabled, FALSE);
  } /* else */
 if (incr[2] == 0.0)
  {
  set(EM_Win->ParTxt[2], MUIA_Text_Contents, (IPTR)"\0");
  sprintf(str, "%f", PAR_FIRST_MOTION(item[2]));
  set(EM_Win->ValTxt[2], MUIA_String_Contents, (IPTR)"\0");
  set(EM_Win->ValTxt[2], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[2], MUIA_Disabled, TRUE);
  set(EM_Win->R_IAArrow[2], MUIA_Disabled, TRUE);
  } /* if */
 else
  {
  set(EM_Win->ParTxt[2], MUIA_Text_Contents, (IPTR)varname[item[2]]);
  sprintf(str, "%f", PAR_FIRST_MOTION(item[2]));
  set(EM_Win->ValTxt[2], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[2], MUIA_String_BufferPos, 0);
  set(EM_Win->L_IAArrow[2], MUIA_Disabled, FALSE);
  set(EM_Win->R_IAArrow[2], MUIA_Disabled, FALSE);
  } /* else */

/* disable/enable arrow buttons */
 if (i < 6)
  {
  set(EM_Win->L_IAArrow[3], MUIA_Disabled, FALSE);
  set(EM_Win->L_IAArrow[4], MUIA_Disabled, FALSE);
  set(EM_Win->R_IAArrow[3], MUIA_Disabled, FALSE);
  set(EM_Win->R_IAArrow[4], MUIA_Disabled, FALSE);
  set(EM_Win->ValTxt[3], MUIA_Disabled, FALSE);
  set(EM_Win->ValTxt[4], MUIA_Disabled, FALSE);
  } /* if camera or focus item */
 else
  {
  set(EM_Win->L_IAArrow[3], MUIA_Disabled, TRUE);
  set(EM_Win->L_IAArrow[4], MUIA_Disabled, TRUE);
  set(EM_Win->R_IAArrow[3], MUIA_Disabled, TRUE);
  set(EM_Win->R_IAArrow[4], MUIA_Disabled, TRUE);
  set(EM_Win->ValTxt[3], MUIA_Disabled, TRUE);
  set(EM_Win->ValTxt[4], MUIA_Disabled, TRUE);
  } /* else not camera or focus item */

 Update_EMTL_Item();

} /* Set_EM_Item() */

/*********************************************************************/

void Update_EM_Item(void)
{

/* set values in interactive group texts */
 if (incr[0] != 0.0)
  {
  sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
  set(EM_Win->ValTxt[0], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[0], MUIA_String_BufferPos, 0);
  } /* if incr[0] */
 if (incr[1] != 0.0)
  {
  sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
  set(EM_Win->ValTxt[1], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[1], MUIA_String_BufferPos, 0);
  } /* if incr[1] */
 if (incr[2] != 0.0)
  {
  sprintf(str, "%f", PAR_FIRST_MOTION(item[2]));
  set(EM_Win->ValTxt[2], MUIA_String_Contents, (IPTR)str);
  set(EM_Win->ValTxt[2], MUIA_String_BufferPos, 0);
  } /* if incr[2] */

/* update radial text */
 sprintf(str, "%f", azimuth * PiUnder180);
 set(EM_Win->ValTxt[3], MUIA_String_Contents, (IPTR)str);
 set(EM_Win->ValTxt[3], MUIA_String_BufferPos, 0);
 sprintf(str, "%f", focdist);
 set(EM_Win->ValTxt[4], MUIA_String_Contents, (IPTR)str);
 set(EM_Win->ValTxt[4], MUIA_String_BufferPos, 0);

 Update_EMTL_Item();

} /* Update_EM_Item() */

/*********************************************************************/

void Set_Radial_Txt(short drawIA)
{
 if (EMIA_Win)
  {
  IA->recompute = 1;
  if (EM_Win->MoItem > 2 && EM_Win->MoItem < 6) setcompass(1);
  else if (EM_Win->MoItem < 3) setcompass(0);
  if (drawIA)
   {
   constructview();
   DrawInterFresh(1);
   } /* if redraw IA window */
  } /* if IA Window open */
 else
  {
  if (EM_Win->MoItem > 2 && EM_Win->MoItem < 6)
   azimuthcompute(1, PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5),
	PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2));
  else azimuthcompute(0, PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2),
	PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
  } /* else no IA window */
 if (MP && MP->ptsdrawn)
  {
  struct clipbounds cb;

  setclipbounds(MapWind0, &cb);
  ShowView_Map(&cb);
  } /* if map interactive */
 sprintf(str, "%f", azimuth * PiUnder180);
 set(EM_Win->ValTxt[3], MUIA_String_Contents, (IPTR)str);
 set(EM_Win->ValTxt[3], MUIA_String_BufferPos, 0);
 sprintf(str, "%f", focdist);
 set(EM_Win->ValTxt[4], MUIA_String_Contents, (IPTR)str);
 set(EM_Win->ValTxt[4], MUIA_String_BufferPos, 0);

 Update_EMTL_Item();

} /* Set_Radial_Txt() */

/*********************************************************************/

STATIC_FCN void Update_EMTL_Item(void) // used locally only -> static, AF 19.7.2021
{

 if (EMTL_Win)
  {
  if (EM_Win->MoItem == EMTL_Win->KeyItem)
   {
   if (EMTL_Win->KeyItem == item[0])
    sprintf(str, "%f", PAR_FIRST_MOTION(item[0]));
   else if (EMTL_Win->KeyItem == item[1])
    sprintf(str, "%f", PAR_FIRST_MOTION(item[1]));
   else if (EMTL_Win->KeyItem == item[2])
    sprintf(str, "%f", PAR_FIRST_MOTION(item[2]));
   if ((EM_Win->NextKey >= 0 || EM_Win->PrevKey >= 0) && (EM_Win->IsKey >= 0))
    set(EMTL_Win->ValStr[0], MUIA_String_Contents, (IPTR)str);
   } /* if */
  } /* if time lines open */

} /* Update_EMTL_String() */

/**********************************************************************/

void Set_EM_List(short update)
{
 short i;

 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  if (CountKeyFrames(0, i))
   sprintf(EM_Win->Motionname[i], "\0333%s", varname[i]);
  else
   sprintf(EM_Win->Motionname[i], "\33n%s", varname[i]);

  //KPrintF("%ld %s\n",i,EM_Win->Motionname[i]);   // 6 Center X
  //                                               // 7 Center Y

  EM_Win->MName[i] = &EM_Win->Motionname[i][0];
  } /* for i=0... */
 EM_Win->MName[USEDMOTIONPARAMS] = NULL;

/* Add items or update Motion list */
 if (update)
  {
  DoMethod(EM_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(EM_Win->LS_List,
	MUIM_List_Insert, &EM_Win->MName, -1, MUIV_List_Insert_Bottom);
  }

} /* Set_EM_List() */

/**********************************************************************/

void MakeMotionKey(void)
{
long i;
IPTR GroupKey, FrameKey, MoItem;
struct clipbounds cb;

 if (MP && MP->ptsdrawn)
  {
  setclipbounds(MapWind0, &cb);
  ShowPaths_Map(&cb);
  } /* if */

 if (EM_Win)
  {
  get(EM_Win->BT_GroupKey, MUIA_Selected, &GroupKey);
  sprintf(str, "%d", EM_Win->Frame);
  MoItem = EM_Win->MoItem;
  } /* if motion editor open */
 else
  {
  GroupKey = 1;
  str[0] = 0;
  MoItem = 0;
  SetIncrements(0);
  } /* else */
 if (! GetInputString((char*)GetString( MSG_EDMOGUI_ENTERFRAMETOMAKEKEYFOR ),  // "Enter frame to make key for."
	 "abcdefghijklmnopqrstuvwxyz", str))
  return;
 FrameKey = atoi(str);
 if (MakeKeyFrame((short)FrameKey, 0, MoItem))
  {
  if (GroupKey)
   {
   if (incr[0] != 0.0 && MoItem != item[0])
		MakeKeyFrame((short)FrameKey, 0, item[0]);
   if (incr[1] != 0.0 && MoItem != item[1])
        	MakeKeyFrame((short)FrameKey, 0, item[1]);
   if (incr[2] != 0.0 && MoItem != item[2])
        	MakeKeyFrame((short)FrameKey, 0, item[2]);
   if (item[0] < 6)
    {
    if (item[0] < 3)
     {
     strcpy(str, (char*)GetString( MSG_EDMOGUI_MAKEKEYFRAMESFORFOCUSPARAMETERSALSO ) );   // "Make key frames for Focus Parameters also?"
     i = 3;
     } /* if camera group */
    else
     {
     strcpy(str, (char*)GetString( MSG_EDMOGUI_MAKEKEYFRAMESFORCAMERAPARAMETERSALSO ) );  // "Make key frames for Camera Parameters also?"
     i = 0;
     } /* else focus group */
    if (User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMAKEKEY ),  // "Parameters Module: Make Key"
                         (CONST_STRPTR)str,
                         GetString( MSG_GLOBAL_YESNO ),                    // "Yes|No"
                         (CONST_STRPTR)"yn", 1))
     {
     MakeKeyFrame((short)FrameKey, 0, i    );
     MakeKeyFrame((short)FrameKey, 0, i + 1);
     MakeKeyFrame((short)FrameKey, 0, i + 2);
     } /* if */
    } /* if camera or focus */
   } /* if group key */
  if (EM_Win)
   {
   UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
   GetKeyTableValues(0, EM_Win->MoItem, 1);
   if (GroupKey)
    {
    Set_EM_List(1);
    if (EMPL_Win)
     DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
    } /* if make multiple keys */
   else
    {
    sprintf(EM_Win->Motionname[EM_Win->MoItem], "\0333%s", varname[EM_Win->MoItem]);
    DoMethod(EM_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
    if (EMPL_Win)
     DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, EM_Win->MoItem);
    }
   Set_EM_Item(EM_Win->MoItem);
   Set_Radial_Txt(2);
   ResetTimeLines(-1);
   DisableKeyButtons(0);
   } /* if new key frame */
  } /* if motion editor open */
 else
  ResetTimeLines(0);
 if (MP && MP->ptsdrawn)
  {
  ShowPaths_Map(&cb);
  }

 Par_Mod |= 0x0001;

} /* MakeMotionKey() */

/***********************************************************************/

void Make_EMIA_Window(void)
{
 short i;
 IPTR open;
 static const char *EMIA_Cycle_Page[3]={NULL};
 static const char *EMIA_Cycle_Move[3]={NULL};
 static const char *EMIA_Cycle_Grid[3]={NULL};
 static int Init = TRUE;

 if (Init)
 {
	 Init = FALSE;

	 EMIA_Cycle_Page[0] = (char*) GetString(MSG_EDMOGUI_DRAW);       // "\0334Draw"
	 EMIA_Cycle_Page[1] = (char*) GetString(MSG_EDMOGUI_BOUNDS);     // "\0334Bounds"
	 EMIA_Cycle_Page[2] = NULL;

	 EMIA_Cycle_Move[0] = (char*) GetString( MSG_EDMOGUI_RADIAL);   // "Radial"
	 EMIA_Cycle_Move[1] = (char*) GetString( MSG_EDMOGUI_RECTANG);  // "Rectang"
	 EMIA_Cycle_Move[2] = NULL;

	 EMIA_Cycle_Grid[0] = (char*) GetString( MSG_EDMOGUI_SOLID);    // "Solid"
	 EMIA_Cycle_Grid[1] = (char*) GetString( MSG_EDMOGUI_WIRE);     // "Wire"
	 EMIA_Cycle_Grid[2] = NULL;
 } /* if */



 if (EMIA_Win)
  {
  WindowToFront(InterWind0);
  DoMethod(EMIA_Win->IAMotionWin, MUIM_Window_ToFront);
  if (EMPL_Win) DoMethod(EMPL_Win->ParListWin, MUIM_Window_ToFront);
  set(EMIA_Win->IAMotionWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((EMIA_Win = (struct IAMotionWindow *)
	get_Memory(sizeof (struct IAMotionWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((EMIA_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_EMIA_Window(-1);
  return;
  } /* if out of memory */
 memcpy(EMIA_Win->AltKF, KF, KFsize);
 EMIA_Win->AltKFsize = KFsize;
 EMIA_Win->AltKeyFrames = ParHdr.KeyFrames;

  Set_Param_Menu(0);

     EMIA_Win->IAMotionWin = WindowObject,
      MUIA_Window_Title		, (char*)GetString( MSG_EDMOGUI_CAMVC ) ,  // "Cam VC"
      MUIA_Window_ID		, MakeID('E','D','I','A'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, RegisterGroup(EMIA_Cycle_Page),
	  Child, VGroup, 
/*	    Child, TextObject, MUIA_Text_Contents, "\33cDraw", End,*/
            Child, EMIA_Win->BT_Grid = KeyButtonFunc('t', (char*)GetString( MSG_EDMOGUI_TERRAIN )),       // "\33cTerrain"
            Child, EMIA_Win->BT_ElShade = KeyButtonFunc('h', (char*)GetString( MSG_EDMOGUI_ELSHADE )),    // "\33cElShade"
            Child, EMIA_Win->BT_SunShade = KeyButtonFunc('s', (char*)GetString( MSG_EDMOGUI_SUNSHADE )),  // "\33cSunShade"
            Child, EMIA_Win->BT_EcoRender = KeyButtonFunc('e', (char*)GetString( MSG_EDMOGUI_ECOSYS )),   // "\33cEcoSys "
            Child, EMIA_Win->BT_DiagRender = KeyButtonFunc('d', (char*)GetString( MSG_EDMOGUI_DIAG )),    // "\33cDiag "
            Child, EMIA_Win->BT_Vector = KeyButtonFunc('v', (char*)GetString( MSG_EDMOGUI_VECTORS )),     // "\33cVectors"
            Child, EMIA_Win->BT_Anim = KeyButtonFunc('a', (char*)GetString( MSG_EDMOGUI_ANIM )),          // "\33cAnim "
	    End, /* VGroup */

	  Child, VGroup,
/*	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	    Child, TextObject, MUIA_Text_Contents, "\33cBounds", End,*/
            Child, EMIA_Win->BT_CompassBounds = KeyButtonObject('o'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_COMPASS ), End,                // "\33cCompass"
            Child, EMIA_Win->BT_LandBounds = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_LAND ), End,                   // "\33cLand"
            Child, EMIA_Win->BT_ProfileBounds = KeyButtonObject('g'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_TARGET ), End,                 // "\33cTarget"
            Child, EMIA_Win->BT_BoxBounds = KeyButtonObject('b'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_BOX ), End,                    // "\33cBox"
            Child, EMIA_Win->BT_GridBounds = KeyButtonObject('p'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_PROFILE ), End,                // "\33cProfile"
	    Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_PROFDENS ), End,  // "\33c\0334Prof Dens"
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, EMIA_Win->BT_GBDens[0] = KeyButtonObject('1'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/1", End,
              Child, EMIA_Win->BT_GBDens[1] = KeyButtonObject('2'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/2", End,
              Child, EMIA_Win->BT_GBDens[2] = KeyButtonObject('4'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/4", End,
	      End, /* HGroup */ 
            Child, EMIA_Win->BT_AutoDraw = KeyButtonObject('w'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_EDMOGUI_AUTODRAW ), End,  // "\33cAuto Draw"
	    Child, RectangleObject, End,
	    End, /* VGroup */
	  End, /* RegisterGroup */
	Child, VGroup,
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_GRID ), End,  // "\33c\0334Grid"
          Child, HGroup, MUIA_Group_HorizSpacing, 0, 
            Child, EMIA_Win->Str[0] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		/*MUIA_FixWidthTxt, "01234",*/ End,
            Child, EMIA_Win->StrArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMIA_Win->StrArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  Child, HGroup,
            Child, EMIA_Win->BT_ShowLat = KeyButtonObject('-'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c-", End, 
            Child, EMIA_Win->BT_ShowLon = KeyButtonObject('|'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c|", End,
	    End, /* HGroup */ 
          Child, EMIA_Win->CY_Grid = CycleObject,
		 MUIA_Cycle_Entries, EMIA_Cycle_Grid, End, 
	  End, /* VGroup */

	Child, VGroup,
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_MOVEMENT ), End,  // "\33c\0334Movement"
	  Child, HGroup,
            Child, EMIA_Win->BT_MoveX = KeyButtonObject('x'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cX", End,
            Child, EMIA_Win->BT_MoveZ = KeyButtonObject('y'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cY", End,
            Child, EMIA_Win->BT_MoveY = KeyButtonObject('z'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cZ", End,
	    End, /* HGroup */
          Child, EMIA_Win->CY_Move = CycleObject,
		 MUIA_Cycle_Entries, EMIA_Cycle_Move, End, 
	  End, /* VGroup */

	Child, VGroup,
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDMOGUI_FRAME_COLOR ), End,  // "\33c\0334Frame"
          Child, HGroup, MUIA_Group_HorizSpacing, 0,
            Child, EMIA_Win->Str[1] = StringObject, StringFrame,
		MUIA_String_Integer, EM_Win->Frame,
		/*MUIA_FixWidthTxt, "01234",*/
		MUIA_String_Accept, "0123456789", End,
            Child, EMIA_Win->StrArrow[2][0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMIA_Win->StrArrow[1][0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMIA_Win->StrArrow[1][1] = ImageButtonWCS(MUII_ArrowRight),
            Child, EMIA_Win->StrArrow[2][1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
          Child, EMIA_Win->BT_MakeKey = KeyButtonFunc('m', (char*)GetString( MSG_EDMOGUI_MAKEKEY )),     // "\33cMake Key"
	  End, /* VGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
        Child, EMIA_Win->BT_AutoCenter = KeyButtonFunc('n', (char*)GetString( MSG_EDMOGUI_CENTERFOC )),  // "CenterFoc"
        Child, EMIA_Win->BT_Aspect = KeyButtonFunc('i', (char*)GetString( MSG_EDMOGUI_IMGASPECT )),      // "Img Aspect"

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
        Child, EMIA_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_EDMOGUI_KEEP )),            // "\33cKeep"
        Child, EMIA_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),         // "\33cCancel"
	End, /* VGroup */
      End; /* WindowObject */

  if (! EMIA_Win->IAMotionWin)
   {
   Close_EMIA_Window(-1);
   User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),   // "Camera View"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),           // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EMIA_Win->IAMotionWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Close requests */
  DoMethod(EMIA_Win->IAMotionWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, EMIA_Win->BT_AutoCenter, ID_EM_AUTOCENTER,
   EMIA_Win->BT_Apply, ID_EMIA_APPLY,
   EMIA_Win->BT_Cancel, ID_EMIA_CLOSE, NULL);

/* set selected state */
  set(EMIA_Win->BT_GBDens[0], MUIA_Selected, (IA_GBDens <= 1));
  set(EMIA_Win->BT_GBDens[1], MUIA_Selected, (IA_GBDens == 2));
  set(EMIA_Win->BT_GBDens[2], MUIA_Selected, (IA_GBDens >= 4));
  set(EMIA_Win->BT_MoveX, MUIA_Selected, TRUE);
  set(EMIA_Win->BT_MoveY, MUIA_Selected, TRUE);
  set(EMIA_Win->BT_MoveZ, MUIA_Selected, TRUE);
  set(EMIA_Win->BT_ShowLat, MUIA_Selected, showX);
  set(EMIA_Win->BT_ShowLon, MUIA_Selected, showY);
  set(EMIA_Win->BT_CompassBounds, MUIA_Selected, CompassBounds);
  set(EMIA_Win->BT_LandBounds, MUIA_Selected, LandBounds);
  set(EMIA_Win->BT_ProfileBounds, MUIA_Selected, ProfileBounds);
  set(EMIA_Win->BT_BoxBounds, MUIA_Selected, BoxBounds);
  set(EMIA_Win->BT_GridBounds, MUIA_Selected, GridBounds);
  set(EMIA_Win->BT_AutoDraw, MUIA_Selected, IA_AutoDraw);

/* set disabled state */
  set(EMIA_Win->BT_GBDens[0], MUIA_Disabled, (! GridBounds));
  set(EMIA_Win->BT_GBDens[1], MUIA_Disabled, (! GridBounds));
  set(EMIA_Win->BT_GBDens[2], MUIA_Disabled, (! GridBounds));
  
/* Link buttons to application */

  MUI_DoNotiPresFal(app, EMIA_Win->BT_Grid, ID_EMIA_GRID,
   EMIA_Win->BT_ElShade, ID_EMIA_ELSHADE,
   EMIA_Win->BT_SunShade, ID_EMIA_SUNSHADE,
   EMIA_Win->BT_EcoRender, ID_EMIA_ECORENDER,
   EMIA_Win->BT_DiagRender, ID_EMIA_DIAGRENDER,
   EMIA_Win->BT_Vector, ID_EMIA_VECTOR,
   EMIA_Win->BT_Anim, ID_AN_WINDOW,
   EMIA_Win->BT_MakeKey, ID_EM_MAKEKEY,
   EMIA_Win->BT_Aspect, ID_EMIA_ASPECT,
   EMIA_Win->BT_AutoDraw, ID_EMIA_AUTODRAW,
   NULL);

  DoMethod(EMIA_Win->BT_CompassBounds, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_COMPASSBDS);  
  DoMethod(EMIA_Win->BT_LandBounds, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_LANDBDS);  
  DoMethod(EMIA_Win->BT_ProfileBounds, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_PROFILEBDS);  
  DoMethod(EMIA_Win->BT_BoxBounds, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_BOXBDS);  
  DoMethod(EMIA_Win->BT_GridBounds, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GRIDBDS);  
  DoMethod(EMIA_Win->BT_ShowLat, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_SHOWLAT);  
  DoMethod(EMIA_Win->BT_ShowLon, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_SHOWLON);  
  DoMethod(EMIA_Win->BT_MoveX, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_MOVEX);  
  DoMethod(EMIA_Win->BT_MoveY, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_MOVEY);  
  DoMethod(EMIA_Win->BT_MoveZ, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_MOVEZ);  
  DoMethod(EMIA_Win->BT_AutoDraw, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_AUTODRAW);  

/* Grid Bounds Density buttons */
  DoMethod(EMIA_Win->BT_GBDens[0], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(0));  
  DoMethod(EMIA_Win->BT_GBDens[1], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(1));  
  DoMethod(EMIA_Win->BT_GBDens[2], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(2));  
  DoMethod(EMIA_Win->BT_GBDens[0], MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(0));  
  DoMethod(EMIA_Win->BT_GBDens[1], MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(1));  
  DoMethod(EMIA_Win->BT_GBDens[2], MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GBDENS(2));  

  DoMethod(EMIA_Win->CY_Move, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_MOVECY);  
  DoMethod(EMIA_Win->CY_Grid, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMIA_GRIDCY);  

/* set LW style enter command for making key */
  DoMethod(EMIA_Win->IAMotionWin, MUIM_Notify, MUIA_Window_InputEvent,
	GetString( MSG_EDMOGUI_NUMERICPADENTER ), app, 2, MUIM_Application_ReturnID, ID_EM_MAKEKEY);  // "numericpad enter"

/* tab cycle stuff */

  DoMethod(EMIA_Win->IAMotionWin, MUIM_Window_SetCycleChain,
        EMIA_Win->BT_Grid, EMIA_Win->BT_ElShade, EMIA_Win->BT_SunShade,
        EMIA_Win->BT_EcoRender, EMIA_Win->BT_DiagRender, EMIA_Win->BT_Vector,
        EMIA_Win->BT_Anim, EMIA_Win->BT_CompassBounds, EMIA_Win->BT_LandBounds,
        EMIA_Win->BT_ProfileBounds, EMIA_Win->BT_BoxBounds, EMIA_Win->BT_GridBounds,
	EMIA_Win->BT_GBDens[0],EMIA_Win->BT_GBDens[1],EMIA_Win->BT_GBDens[2],
	EMIA_Win->BT_AutoDraw,
        EMIA_Win->Str[0], EMIA_Win->BT_ShowLat, EMIA_Win->BT_ShowLon,
        EMIA_Win->CY_Grid, EMIA_Win->BT_MoveX, EMIA_Win->BT_MoveZ,
        EMIA_Win->BT_MoveY, EMIA_Win->CY_Move, EMIA_Win->BT_AutoCenter,
	EMIA_Win->BT_Aspect,
	EMIA_Win->BT_Apply, EMIA_Win->BT_Cancel, NULL);

/* set string values before setting notification events */
  set(EMIA_Win->Str[0], MUIA_String_Integer, IA_GridSize);

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app, EMIA_Win->StrArrow[0][0], ID_EMIA_SARROWLEFT(0),
    EMIA_Win->StrArrow[0][1], ID_EMIA_SARROWRIGHT(0),
    EMIA_Win->StrArrow[1][0], ID_EM_SARROWLEFT(0),
    EMIA_Win->StrArrow[1][1], ID_EM_SARROWRIGHT(0),
    EMIA_Win->StrArrow[2][0], ID_EM_PREVKEY,
    EMIA_Win->StrArrow[2][1], ID_EM_NEXTKEY,
    NULL);

/* link string gadgets to application */
  for (i=0; i<2; i++)
   {
   DoMethod(EMIA_Win->Str[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMIA_STRING(i));
   } /* for i=1... */

/* Set Radio and Cycle active entries */
  set(EMIA_Win->CY_Move, MUIA_Cycle_Active, IA_Movement);
  set(EMIA_Win->CY_Grid, MUIA_Cycle_Active, IA_GridStyle);

/* Open window */
  set(EMIA_Win->IAMotionWin, MUIA_Window_Open, TRUE);
  get(EMIA_Win->IAMotionWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EMIA_Window(-1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EMIA_Win->IAMotionWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EMIA_ACTIVATE);

/* Get Window structure pointer */
  get(EMIA_Win->IAMotionWin, MUIA_Window_Window, &EMIA_Win->Win);

} /* Make_EMIA_Window() */

/*********************************************************************/

void Close_EMIA_Window(short apply)
{
 if (AN_Win)
  Close_AN_Window();
 if (EMIA_Win)
  {
  if (EMIA_Win->AltKF)
   {
   if (apply)
    {
    free_Memory(EMIA_Win->AltKF, EMIA_Win->AltKFsize);
    if (apply > 0)
     {
     FixPar(0, 0x0001);
     FixPar(1, 0x0001);
     }
    } /* if */
   else
    {
    UndoPar(0, 0x0001);
    MergeKeyFrames(EMIA_Win->AltKF, EMIA_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 0);
    free_Memory(EMIA_Win->AltKF, EMIA_Win->AltKFsize);
    if (EM_Win)
     {
     UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
     GetKeyTableValues(0, EM_Win->MoItem, 1);
     Set_EM_Item(EM_Win->MoItem);
     Set_Radial_Txt(0);
     ResetTimeLines(-1);
     DisableKeyButtons(0);
     } /* if */
    } /* else discard changes */
   } /* if */
  if (EMIA_Win->IAMotionWin)
   {
   set(EMIA_Win->IAMotionWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, EMIA_Win->IAMotionWin);
   MUI_DisposeObject(EMIA_Win->IAMotionWin);
   } /* if window created */
  free_Memory(EMIA_Win, sizeof (struct IAMotionWindow));
  EMIA_Win = NULL;
  } /* if IA Motion window open */
  closeviewmaps(1);
  closeinterview();
} /* Close_EMIA_Window() */

/*********************************************************************/

void Handle_EMIA_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   if (interactiveview(1))
    {
    Make_EMIA_Window();
    if (EMPL_Win) DoMethod(EMPL_Win->ParListWin, MUIM_Window_ToFront);
    } /* if interactive view opens */
   return;
   } /* Open Motion IA Window */

  if (! EMIA_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_OPEN_WINDOW:
    {
    if (interactiveview(1))
     {
     Make_EMIA_Window();
     if (EMPL_Win) DoMethod(EMPL_Win->ParListWin, MUIM_Window_ToFront);
     } /* if interactive view opens */
    break;
    } /* Open IA Motion Editing window */

   case GP_ACTIVEWIN:
    {
/*    DoMethod(EMIA_Win->IAMotionWin, MUIM_Window_ToFront);*/
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate IA Motion Editing Window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EMIA_GRID:
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        break;
	} /* if no new grid */
       } /* if IA->newgridsize */
      else drawgridview();
      break;
      } /* draw grid view */
     case ID_EMIA_ELSHADE:
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        break;
	} /* no new grid */
       } /* if IA->newgridsize */
      shaderelief(0);
      break;
      } /* elevation shade render */
     case ID_EMIA_SUNSHADE:
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        break;
	} /* if no new grid */
       } /* if IA->newgridsize */
      shaderelief(1);
      break;
      } /* sunshade render */
     case ID_EMIA_ECORENDER:
      {
      smallwindow(0);
      break;
      } /* render small window */
     case ID_EMIA_DIAGRENDER:
      {
      smallwindow(1);
      break;
      } /* render with diagnostics */
     case ID_EMIA_VECTOR:
      {
      oshigh = high = InterWind0->Height - InterWind0->BorderTop
	 - InterWind0->BorderBottom - 1;
      wide = InterWind0->Width - InterWind0->BorderLeft
	 - InterWind0->BorderRight - 1;
      constructview();
      InitVectorMap(InterWind0, 0, 1);
      RefreshWindowFrame(InterWind0);
      break;
      } /* draw vectors */
     case ID_EMIA_COMPASSBDS:
      {
      Set_CompassBds();
      break;
      } /* set compass bounds */
     case ID_EMIA_LANDBDS:
      {
      Set_LandBds();
      break;
      } /* set land bounds */
     case ID_EMIA_PROFILEBDS:
      {
      Set_ProfileBds();
      break;
      } /* set profile bounds */
     case ID_EMIA_BOXBDS:
      {
      Set_BoxBds();
      break;
      } /* set box bounds */
     case ID_EMIA_GRIDBDS:
      {
      Set_GridBds();
      break;
      } /* set grid bounds */
     case ID_EMIA_SHOWLAT:
      {
      showY = 1 - showY;
      break;
      } /* show latitude grid */
     case ID_EMIA_SHOWLON:
      {
      showX = 1 - showX;
      break;
      } /* show longitude grid */
     case ID_EMIA_MOVEX:
      {
      IA->fixX = 1 - IA->fixX;
      break;
      } /* fix select button x movement */
     case ID_EMIA_MOVEY:
      {
      IA->fixY = 1 - IA->fixY;
      break;
      } /* fix select button y movement */
     case ID_EMIA_MOVEZ:
      {
      IA->fixZ = 1 - IA->fixZ;
      break;
      } /* fix menu button movement */
     case ID_EMIA_AUTODRAW:
      {
      IPTR data;

      get(EMIA_Win->BT_AutoDraw, MUIA_Selected, &data);
      IA_AutoDraw = data;
      break;
      } /* fix menu button movement */
     case ID_EMIA_ASPECT:
      {
      short Height, NewYPos, Reposition;
/* compute correct height for given width */
      Height = ((float)settings.scrnheight / settings.scrnwidth) * InterWind0->Width;

      if (Height > WCSScrn->Height)
       {
       if (User_Message(GetString( MSG_EDMOGUI_CAMERAVIEWASPECT ),                                     // "Camera View: Aspect"
                        GetString( MSG_EDMOGUI_COMPUTEDHEIGHTISLARGERTHANTHECURRENTSCREENHEIGHTDOY ),  // "Computed height is larger than the current screen height. Do you wish to use the screen height?"
                        GetString( MSG_GLOBAL_OKCANCEL),                                              // "OK|Cancel"
                        (CONST_STRPTR)"oc"))
        {
        Height = WCSScrn->Height;
        } /* if */
       else
        return;
       } /* if */

      NewYPos = InterWind0->TopEdge; 
      if (Height != InterWind0->Height)
       {
       NewYPos = (WCSScrn->Height - Height) / 2;
       Reposition = 1;
       }
      if (Reposition)
       {
       SetAPen(InterWind0->RPort, 1);
       ChangeWindowBox(InterWind0, InterWind0->LeftEdge, NewYPos, InterWind0->Width, Height);
       Delay(25);	/* ChangeWindowBox() appears to be asynchronous */
       Init_IA_View(2);
       } /* if */
      break;
      } /* use image aspect */
     case ID_EMIA_APPLY:
      {
      Close_EMIA_Window(1);
      break;
      } /* Apply changes to Motion arrays */
     case ID_EMIA_CLOSE:
      {
      Close_EMIA_Window(0);
      break;
      } /* Close and cancel any changes since window opened */
     case ID_EMIA_CLOSEQUERY:
      {
      if (KFsize != EM_Win->AltKFsize || memcmp(KF, EM_Win->AltKF, KFsize)
		|| memcmp(&MoPar, &UndoMoPar[0], sizeof (MoPar)))
       Close_EMIA_Window(CloseWindow_Query(GetString( MSG_EDMOGUI_INTERACTIVEMOTION ) ));  // "Interactive Motion"
      else
       Close_EMIA_Window(1);
      break;
      } /* Close and cancel any changes since window opened */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    IPTR data;
    long redraw = 0;

    i = WCS_ID - ID_EMIA_GBDENS(0);
    get(EMIA_Win->BT_GBDens[i], MUIA_Selected, &data);

    if (i == 0)
     {
     IA_GBDens = pow2((double)(1 - data));
     set(EMIA_Win->BT_GBDens[1], MUIA_Selected, (IA_GBDens == 2));
     set(EMIA_Win->BT_GBDens[2], MUIA_Selected, FALSE);
     redraw = 1;
     }
    else if (i == 1)
     {
     IA_GBDens = pow2((double)(2 - data));
     set(EMIA_Win->BT_GBDens[0], MUIA_Selected, FALSE);
     set(EMIA_Win->BT_GBDens[2], MUIA_Selected, (IA_GBDens == 4));
     redraw = 1;
     }
    else
     {
     IA_GBDens = pow2((double)(1 + data));
     set(EMIA_Win->BT_GBDens[0], MUIA_Selected, FALSE);
     set(EMIA_Win->BT_GBDens[1], MUIA_Selected, (IA_GBDens == 2));
     redraw = 1;
     }
    if (redraw)
     DrawInterFresh(1);
    break;
    } /* Grid Bounds Density buttons */

   case GP_ARROW1:
    {
    IPTR data;

    i = WCS_ID - ID_EMIA_SARROWLEFT(0);
    get(EMIA_Win->Str[i], MUIA_String_Integer, &data);
    if (i == 0)
     set(EMIA_Win->Str[0], MUIA_String_Integer, (data > 1 ? data - 1: 1));
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    IPTR data;

    i = WCS_ID - ID_EMIA_SARROWRIGHT(0);
    get(EMIA_Win->Str[i], MUIA_String_Integer, &data);
    set(EMIA_Win->Str[i], MUIA_String_Integer, data + 1);
    break;
    } /* ARROW2 */

   case GP_STRING1:
    {
    IPTR data;

    i = WCS_ID - ID_EMIA_STRING(0);
    get(EMIA_Win->Str[i], MUIA_String_Integer, &data);
    if (i == 0)
     {
     if (EMIA_Win->GridStrBlock)
      {
      EMIA_Win->GridStrBlock = 0;
      break;
      } /* if grid size string set by application */
     IA_GridSize = data;    
     IA->newgridsize = 1;
     } /* if new grid size */

    else if (i == 1)
     {
     set(EM_Win->Str[0], MUIA_String_Integer, data);
     } /* else frame string */
    break;
    } /* STRING1 */

   case GP_CYCLE1:
    {
    IPTR data;

    get(EMIA_Win->CY_Move, MUIA_Cycle_Active, &data);
    IA_Movement = data;
    SetIncrements(EM_Win->MoItem);
    break;
    } /* CYCLE1 */

   case GP_CYCLE2:
    {
    IPTR data;

    get(EMIA_Win->CY_Grid, MUIA_Cycle_Active, &data);
    IA_GridStyle = data;
    break;
    } /* CYCLE2 */

   case GP_CYCLE3:
    {
    break;
    } /* CYCLE3 */

   } /* switch gadget group */

} /* Handle_EMIA_Window() */

/**********************************************************************/

void Make_EMPL_Window(void)
{
 IPTR open;

 if (EMPL_Win)
  {
  DoMethod(EMPL_Win->ParListWin, MUIM_Window_ToFront);
  set(EMPL_Win->ParListWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((EMPL_Win = (struct ParListWindow *)
	get_Memory(sizeof (struct ParListWindow), MEMF_CLEAR)) == NULL)
   return;

     EMPL_Win->ParListWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_EDMOGUI_MOTIONPARAMLIST ),  // "Motion Param List"
      MUIA_Window_ID		, MakeID('E','D','P','L'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,

/* ParList list */
	Child, EMPL_Win->LS_List = ListviewObject, 
	        MUIA_Listview_Input, TRUE,
        	MUIA_Listview_List, ListObject, ReadListFrame, End,
          End, /* ListviewObject */

        End, /* VGroup */
      End; /* WindowObject EMPL_Win->ParListWin */

  if (! EMPL_Win->ParListWin)
   {
   Close_EMPL_Window();
   User_Message(GetString( MSG_EDMOGUI_MOTIONPARAMLIST ),  // "Motion Param List"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),               // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EMPL_Win->ParListWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Close requests */
  DoMethod(EMPL_Win->ParListWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMPL_CLOSE);  

/* link list to application */
  DoMethod(EMPL_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMPL_LIST);

/* Set active gadget */
  set(EMPL_Win->ParListWin, MUIA_Window_ActiveObject, (IPTR)EMPL_Win->LS_List);

/* Set list view to ParList list */
  DoMethod(EMPL_Win->LS_List,
	MUIM_List_Insert, &EM_Win->MName, -1, MUIV_List_Insert_Bottom);

/* Set list to first item */
  set(EMPL_Win->LS_List, MUIA_List_Active, (long)EM_Win->MoItem);

/* Open window */
  set(EMPL_Win->ParListWin, MUIA_Window_Open, TRUE);
  get(EMPL_Win->ParListWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EMPL_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EMPL_Win->ParListWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EMPL_ACTIVATE);

} /* Make_EMPL_Window() */

/*********************************************************************/

STATIC_FCN void Close_EMPL_Window(void) // used locally only -> static, AF 26.7.2021
{
 
 if (EMPL_Win)
  {
  if (EMPL_Win->ParListWin)
   {
   set(EMPL_Win->ParListWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, EMPL_Win->ParListWin);
   MUI_DisposeObject(EMPL_Win->ParListWin);
   } /* if window created */
  free_Memory(EMPL_Win, sizeof (struct ParListWindow));
  EMPL_Win = NULL;
  } /* if memory allocated */

} /* Close_EMPL_Window() */

/*********************************************************************/

void Handle_EMPL_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EMPL_Window();
   return;
   } /* Open Param List Editing Window */

  if (! EMPL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate ParList Editing window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EMPL_CLOSE:
      {
      Close_EMPL_Window();
      break;
      } /* Close param window */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_LIST1:
    {
    IPTR data;

    get(EMPL_Win->LS_List, MUIA_List_Active, &data);
    set(EM_Win->LS_List, MUIA_List_Active, data);
    break;
    } /* LIST1 */

   } /* switch gadget group */

} /* Handle_EMPL_Window() */
