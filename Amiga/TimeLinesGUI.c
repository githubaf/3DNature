/* TimeLinesGUI.c
** World Construction Set GUI for Time Line Editing modules.
** By Gary R. Huber, 1994.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "TimeLinesGUI.h"

/**********************************************************************/
/* Motion Time Lines */

void Make_EMTL_Window(void)
{
 long open;
 static const char *EMTL_Cycle_TCB[4] = {NULL};
 static int Init=TRUE;

 if(Init)
 {
	 Init=FALSE;
	 EMTL_Cycle_TCB[0] = (char*)GetString( MSG_TLGUI_TENS );  // "Tens"
	 EMTL_Cycle_TCB[1] = (char*)GetString( MSG_TLGUI_CONT );  // "Cont"
	 EMTL_Cycle_TCB[2] = (char*)GetString( MSG_TLGUI_BIAS );  // "Bias"
 }

 if (EMTL_Win)
  {
  DoMethod(EMTL_Win->TimeLineWin, MUIM_Window_ToFront);
  set(EMTL_Win->TimeLineWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  return;
  } /* if window already exists */

 if ((EMTL_Win = (struct TimeLineWindow *)
	get_Memory(sizeof (struct TimeLineWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((EMTL_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_EMTL_Window(1);
  return;
  } /* if out of memory */
 memcpy(EMTL_Win->AltKF, KF, KFsize);
 EMTL_Win->AltKFsize = KFsize;
 EMTL_Win->AltKeyFrames = ParHdr.KeyFrames;

 if ( ! (EMTL_Win->SuperClass = MUI_GetClass(MUIC_Area)))
  {
  Close_EMTL_Window(1);
  return;
  } /* if out of memory */

/* create the new class */
 if (!(EMTL_Win->TL_Class =
	 MakeClass(NULL, NULL, EMTL_Win->SuperClass, sizeof(struct Data), 0)))
 {
  MUI_FreeClass(EMTL_Win->SuperClass);
  return;
 }

/* set up parameter cycle list */
 EMTL_Win->ListSize = (USEDMOTIONPARAMS + 1) * (sizeof (char *));
 EMTL_Win->ListIDSize = (USEDMOTIONPARAMS) * (sizeof (short));

 if (((EMTL_Win->List = (char **)get_Memory(EMTL_Win->ListSize, MEMF_CLEAR)) == NULL)
	|| ((EMTL_Win->ListID = (short *)get_Memory(EMTL_Win->ListIDSize, MEMF_CLEAR)) == NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
               GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
               GetString( MSG_GLOBAL_OK ),                                // "OK"
               (CONST_STRPTR)"o");
  Close_EMTL_Window(1);
  return;
  } /* if out of memory */

 if (! Set_PS_List(EMTL_Win->List, EMTL_Win->ListID, 0, 2, NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
               GetString( MSG_TLGUI_NOMOTIONPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERM ),  // "No Motion Parameters with more than one Key Frame!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                     // "OK"
               (CONST_STRPTR)"o");
  Close_EMTL_Window(1);
  return;
  } /* if out of memory */

/* set the dispatcher for the new class */
 EMTL_Win->TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
 EMTL_Win->TL_Class->cl_Dispatcher.h_SubEntry = NULL;
 EMTL_Win->TL_Class->cl_Dispatcher.h_Data     = NULL;

  Set_Param_Menu(0);

     EMTL_Win->TimeLineWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_TLGUI_MOTIONTIMELINE ),  // "Motion Time Line"
      MUIA_Window_ID		, MakeID('E','M','T','L'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
        Child, HGroup, MUIA_Group_HorizSpacing, 0,
	  Child, RectangleObject, End,
	  Child, EMTL_Win->ParCycle = CycleObject,
		MUIA_Cycle_Entries, EMTL_Win->List, End,
	  Child, RectangleObject, End,
	  Child, EMTL_Win->ValStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890123", End,
	  Child, RectangleObject, End,
          End, /* HGroup */
	Child, EMTL_Win->TimeLineObj[0] = NewObject(EMTL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	Child, HGroup,
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, VGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_PAN )),  // "  Pan "
                Child, EMTL_Win->Prop[0] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 100,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 100, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_ZOOM )),  // " Zoom "
                Child, EMTL_Win->Prop[1] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_FRAME_SPACE )),  // "Frame "
                Child, EMTL_Win->Prop[2] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 0,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      End, /* VGroup */
	    End, /* HGroup */
	  Child, VGroup,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, EMTL_Win->BT_PrevKey = KeyButtonObject('v'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_PREV ), End,  // "\33cPrev"
	      Child, EMTL_Win->BT_NextKey = KeyButtonObject('x'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_NEXT ), End,  // "\33cNext"
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, EMTL_Win->BT_AddKey = KeyButtonFunc('a', (char*)GetString( MSG_TLGUI_ADDKEY )),  // "\33cAdd Key"
	      Child, EMTL_Win->BT_DelKey = KeyButtonFunc(127, (char*)GetString( MSG_TLGUI_DELKEY )),  // "\33c\33uDel\33n Key"
	      End, /* HGroup */
	    Child, EMTL_Win->KeysExistTxt = TextObject, TextFrame, End,
	    End, /* VGroup */
	  End, /* HGroup */

	Child, HGroup,
	  Child, EMTL_Win->BT_Linear = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_LINEAR ), End,  // "\33cLinear"
	  Child, EMTL_Win->TCB_Cycle = Cycle(EMTL_Cycle_TCB),
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, EMTL_Win->CycleStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345", End,
            Child, EMTL_Win->StrArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMTL_Win->StrArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  Child, RectangleObject, End,
	  Child, EMTL_Win->FrameTxtLbl = Label1(GetString( MSG_TLGUI_FRAME )),  // "Frame"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, EMTL_Win->FrameTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
            Child, EMTL_Win->TxtArrowLg[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMTL_Win->TxtArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EMTL_Win->TxtArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            Child, EMTL_Win->TxtArrowLg[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  End, /* HGroup */
	Child, HGroup,
	  Child, EMTL_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_TLGUI_KEEP )),  // "\33cKeep"
	  Child, EMTL_Win->BT_Grid = KeyButtonObject('g'),
	 	 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, TRUE,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_GRID ), End,  // "\33cGrid"
	  Child, EMTL_Win->BT_Play = KeyButtonObject('p'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_PLAY ), End,  // "\33cPlay"
	  Child, EMTL_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),  // "\33cCancel"
	  End, /* HGroup */ 
	End, /* VGroup */
      End; /* WindowObject EMTL_Win->TimeLineWin */

  if (! EMTL_Win->TimeLineWin)
   {
   Close_EMTL_Window(1);
   User_Message(GetString( MSG_TLGUI_MOTIONTIMELINE ),  // "Motion Time Line"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),              // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EMTL_Win->TimeLineWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(EMTL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_CLOSEQUERY);  

  DoMethod(EMTL_Win->BT_Apply, MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_APPLY);  
  DoMethod(EMTL_Win->BT_Cancel, MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_CLOSE);  

  MUI_DoNotiPresFal(app, EMTL_Win->BT_AddKey, ID_EMTL_ADDKEY,
   EMTL_Win->BT_DelKey, ID_EM_DELETEKEY, EMTL_Win->BT_PrevKey, ID_EM_PREVKEY,
   EMTL_Win->BT_NextKey, ID_EM_NEXTKEY,
   EMTL_Win->TimeLineObj[0], ID_EMTL_TIMELINEOBJ, NULL);

  DoMethod(EMTL_Win->BT_Play, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_PLAY);  
  DoMethod(EMTL_Win->BT_Grid, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_GRID);  
  DoMethod(EMTL_Win->BT_Linear, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EMTL_LINEAR);  

/* set values */
  set(EMTL_Win->Prop[0], MUIA_Prop_First, 0);
  set(EMTL_Win->Prop[0], MUIA_Prop_Visible, 100);
  set(EMTL_Win->Prop[1], MUIA_Prop_First, 100);
  if (! Set_EMTL_Item(EM_Win->MoItem))
   {
   Close_EMTL_Window(1);
   User_Message(GetString( MSG_TLGUI_MOTIONEDITORTIMELINES ),                                  // "Motion Editor: Time Lines"
                GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* if build key table failed */
  Set_EMTL_Data();
  DisableKeyButtons(0);
  set(EMTL_Win->Prop[2], MUIA_Prop_First,
	 (100 * EM_Win->Frame) / EMTL_Win->Frames);

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app,
   EMTL_Win->TxtArrow[0], ID_EMTL_TXTARROWLEFT,
   EMTL_Win->TxtArrow[1], ID_EMTL_TXTARROWRIGHT,
   EMTL_Win->TxtArrowLg[0], ID_EMTL_TXTARROWLGLEFT,
   EMTL_Win->TxtArrowLg[1], ID_EMTL_TXTARROWLGRIGHT,
   EMTL_Win->StrArrow[0], ID_EMTL_STRARROWLEFT,
   EMTL_Win->StrArrow[1], ID_EMTL_STRARROWRIGHT, NULL);

/* Link prop gadgets to application and each other */
  DoMethod(EMTL_Win->Prop[0], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMTL_PANPROP);
  DoMethod(EMTL_Win->Prop[0], MUIM_Notify, MUIA_Prop_Visible, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMTL_PANPROP);
  DoMethod(EMTL_Win->Prop[1], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     EMTL_Win->Prop[0], 3, MUIM_Set, MUIA_Prop_Visible, MUIV_TriggerValue);
  DoMethod(EMTL_Win->Prop[2], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMTL_FRAMEPROP);
	
/* link string gadgets to application */
  DoMethod(EMTL_Win->CycleStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMTL_CYCLESTR);
  DoMethod(EMTL_Win->ValStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EMTL_VALSTR(0));

/* Link cycle gadgets to application */
  DoMethod(EMTL_Win->TCB_Cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EMTL_CYCLE);
  DoMethod(EMTL_Win->ParCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EMTL_PARCYCLE);

/* Disable Play button if no IA window */
  if (! InterWind0) set(EMTL_Win->BT_Play, MUIA_Disabled, TRUE);

/* Open window */
  set(EMTL_Win->TimeLineWin, MUIA_Window_Open, TRUE);
  get(EMTL_Win->TimeLineWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EMTL_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EMTL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EMTL_ACTIVATE);

/* Get Window structure pointer */
  get(EMTL_Win->TimeLineWin, MUIA_Window_Window, &EMTL_Win->Win);

} /* Make_MTL_Window() */

/*********************************************************************/

void Close_EMTL_Window(short apply)
{
 short i;

 if (EMTL_Win)
  {
  if (topoload && ! MapWind0)
   {
   if (mapelmap)
    {
    for (i=0; i<topomaps; i++)
     {
     if (mapelmap[i].map) free_Memory(mapelmap[i].map, mapelmap[i].size);
     } /* for i=0... */
    free_Memory(mapelmap, MapElmapSize);
    mapelmap = NULL;
    } /* if mapelmap */
   if (mapcoords) free_Memory(mapcoords, MapCoordSize);
   mapcoords = NULL;
   if (TopoOBN) free_Memory(TopoOBN, TopoOBNSize);
   TopoOBN = NULL;
   topoload = 0;
   topomaps = 0;
   } /* if topos loaded previously */
  if (SKT[0]) FreeSingleKeyTable(0, EMTL_Win->Frames);
  if (EMTL_Win->AltKF)
   {
   if (apply)
    free_Memory(EMTL_Win->AltKF, EMTL_Win->AltKFsize);
   else
    {
    MergeKeyFrames(EMTL_Win->AltKF, EMTL_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 0);
    free_Memory(EMTL_Win->AltKF, EMTL_Win->AltKFsize);
    ResetTimeLines(0);
    UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
    Set_EM_Item(EM_Win->MoItem);
    } /* else discard changes */
   } /* if */
  if (EMTL_Win->TimeLineWin)
   {
   set(EMTL_Win->TimeLineWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, EMTL_Win->TimeLineWin);
   MUI_DisposeObject(EMTL_Win->TimeLineWin);
   } /* if window created */
  if (EMTL_Win->List) free_Memory(EMTL_Win->List, EMTL_Win->ListSize);
  if (EMTL_Win->ListID) free_Memory(EMTL_Win->ListID, EMTL_Win->ListIDSize);
  if (EMTL_Win->TL_Class) FreeClass(EMTL_Win->TL_Class);         /* free our custom class. */
  MUI_FreeClass(EMTL_Win->SuperClass); /* release super class pointer. */
  free_Memory(EMTL_Win, sizeof (struct TimeLineWindow));
  EMTL_Win = NULL;
  DisableKeyButtons(0);
  } /* if */

} /* Close_EMTL_Window() */

/*********************************************************************/

void Handle_EMTL_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EMTL_Window();
   return;
   } /* Open Motion Time Line Window */

  if (! EMTL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Motion Time Line window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EMTL_ADDKEY:
      {
      struct Data *data = INST_DATA(EMTL_Win->TL_Class, EMTL_Win->TimeLineObj[0]);

      data->inputflags = KEYFRAME_NEW;
      if (GetInput_Pt(EMTL_Win->TL_Class, EMTL_Win->TimeLineObj[0]))
       {
       MakeKeyFrame((short)EM_Win->Frame, 0, EMTL_Win->KeyItem);
       UnsetKeyFrame(EM_Win->Frame, 0, EMTL_Win->KeyItem, 0);
       DisableKeyButtons(0);
       if (Set_EMTL_Item(EMTL_Win->KeyItem))
        {
        Set_EMTL_Data();
        MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
	}
       ResetTimeLines(0);
       Par_Mod |= 0x0001;
       data->inputflags = 0;
       } /* if input point */
      break;
      }
     case ID_EMTL_LINEAR:
      {
      long SelState;

      get(EMTL_Win->BT_Linear, MUIA_Selected, &SelState);
      SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.Linear = SelState;
      SplineSingleKey(0, 0);
      if ((EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2
	|| EMTL_Win->KeyItem == 4 || EMTL_Win->KeyItem == 5)
		 && EMTL_Win->ElevProf)
       {
       EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
       EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
       if (EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2)
        BuildVelocityTable(0, &SKT[0]->Val[1],
		EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
       else
        BuildVelocityTable(1, &SKT[0]->Val[1],
		EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
       } /* if camera lat or lon and velocity table already exists */
      Set_EMTL_Data();
      MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      break;
      }
     case ID_EMTL_GRID:
      {
      long SelState;
      struct Data *data = INST_DATA(EMTL_Win->TL_Class, EMTL_Win->TimeLineObj[0]);

      get(EMTL_Win->BT_Grid, MUIA_Selected, &SelState);
      data->drawgrid = SelState;
      MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      break;
      }
     case ID_EMTL_PLAY:
      {
      Play_Motion(NULL);
      break;
      }
     case ID_EMTL_APPLY:
      {
      Close_EMTL_Window(1);
      break;
      }
     case ID_EMTL_CLOSE:
      {
      Close_EMTL_Window(0);
      break;
      }
     case ID_EMTL_CLOSEQUERY:
      {
      if (KFsize != EMTL_Win->AltKFsize || memcmp(KF, EMTL_Win->AltKF, KFsize))
       Close_EMTL_Window(CloseWindow_Query(GetString( MSG_TLGUI_MOTIONTIMELINES )));  // "Motion Time Lines"
      else
       Close_EMTL_Window(1);
      break;
      }
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    char *value;
    short i;
    LONG cyitem, GroupKey;

    get(EMTL_Win->CycleStr, MUIA_String_Contents, &value);
    get(EMTL_Win->TCB_Cycle, MUIA_Cycle_Active, &cyitem);
    EM_Win->TCB[cyitem] = atof(value);
    SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.TCB[cyitem] = atof(value);
    SplineSingleKey(0, 0);
    if ((EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2
	|| EMTL_Win->KeyItem == 4 || EMTL_Win->KeyItem == 5)
		 && EMTL_Win->ElevProf)
     {
     EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
     EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
     if (EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2)
      BuildVelocityTable(0, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
     else
      BuildVelocityTable(1, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
     } /* if camera lat or lon and velocity table already exists */
    Set_EMTL_Data();
    MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
/* check for other key frames at this frame for the interactive group */
    get(EM_Win->BT_GroupKey, MUIA_Selected, &GroupKey);
    if (GroupKey)
     {
     if (item[0] != EMTL_Win->KeyItem)
      {
      if ((i = SearchKeyFrame(SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame,
	 0, item[0])) >= 0)
       KF[i].MoKey.TCB[cyitem] = EM_Win->TCB[cyitem];
      }
     if (item[1] != item[0] && item[1] != EMTL_Win->KeyItem)
      {
      if ((i = SearchKeyFrame(SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame,
	 0, item[1])) >= 0)
       KF[i].MoKey.TCB[cyitem] = EM_Win->TCB[cyitem];
      }
     if (item[2] != item[1] && item[2] != EMTL_Win->KeyItem)
      {
      if ((i = SearchKeyFrame(SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame,
	 0, item[2])) >= 0)
       KF[i].MoKey.TCB[cyitem] = EM_Win->TCB[cyitem];
      }
     } /* if group key selected */
    break;
    } /* TCB string */

   case GP_STRING2:
    {
    char *floatvalue;
    double value;

    get(EMTL_Win->ValStr[0], MUIA_String_Contents, &floatvalue);
    value = atof(floatvalue);
    if (value > parambounds[EMTL_Win->KeyItem][0])
     value = parambounds[EMTL_Win->KeyItem][0];
    else if (value < parambounds[EMTL_Win->KeyItem][1])
     value = parambounds[EMTL_Win->KeyItem][1];
    SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.Value = value;
    PAR_FIRST_MOTION(EMTL_Win->KeyItem) = value;
    Set_Radial_Txt(2);
    SplineSingleKey(0, 0);
    if ((EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2
	|| EMTL_Win->KeyItem == 4 || EMTL_Win->KeyItem == 5)
		 && EMTL_Win->ElevProf)
     {
     EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
     EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
     if (EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2)
      BuildVelocityTable(0, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
     else
      BuildVelocityTable(1, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
     } /* if camera/focus lat or lon and velocity table already exists */
    Set_EMTL_Data();
    MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    break;
    } /* Value string */

   case GP_PROP1:
    {
    Set_EMTL_Data();
    MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    break;
    } /* pan prop */

   case GP_PROP2:
    {
    long data;

    if (! EM_Win->StrBlock)
     {
     get(EMTL_Win->Prop[2], MUIA_Prop_First, &data);
     data = ((float)data / 100.0) * EMTL_Win->Frames;
     if (data < 1) data = 1;
     set(EM_Win->Str[0], MUIA_String_Integer, data);
     EM_Win->PropBlock = 1;
     }
    EM_Win->StrBlock = 0;
    break;
    } /* pan prop */

   case GP_CYCLE1:
    {
    LONG item;

    get(EMTL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EM_Win->TCB[item]);
    set(EMTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Cycle */

   case GP_CYCLE2:
    {
    long data;

    get(EMTL_Win->ParCycle, MUIA_Cycle_Active, &data);
    set(EM_Win->LS_List, MUIA_List_Active, EMTL_Win->ListID[data]);
    break;
    } /* parameter cycle */

   case GP_ARROW1:
    {
    char *value;

    get(EMTL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) - .1);
    set(EMTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW2:
    {
    char *value;

    get(EMTL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) + .1);
    set(EMTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW3:
   case GP_ARROW9:
    {
    char *data;
    short i, oldframe;
    long frame, mult = 1, GroupKey;

    if (WCS_ID == ID_EMTL_TXTARROWLGLEFT) mult = 10;
    get(EMTL_Win->FrameTxt, MUIA_Text_Contents, &data);
    oldframe = frame = atoi(data);
    if (frame >= mult && (! EM_Win->PrevKey || frame > EM_Win->PrevKey + mult))
     {
     frame -= mult;
     sprintf(str, "%ld", frame);
     set(EMTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame = frame;
     SplineSingleKey(0, 0);
     if ((EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2
	|| EMTL_Win->KeyItem == 4 || EMTL_Win->KeyItem == 5)
		 && EMTL_Win->ElevProf)
      {
      EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
      EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
      if (EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2)
       BuildVelocityTable(0, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
      else
       BuildVelocityTable(1, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
      } /* if camera lat or lon and velocity table already exists */
     Set_EMTL_Data();
     MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
     set(EM_Win->Str[0], MUIA_String_Integer, frame);
/* check for other key frames at this frame for the interactive group */
     get(EM_Win->BT_GroupKey, MUIA_Selected, &GroupKey);
     if (GroupKey)
      {
      if (item[0] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[0])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      if (item[1] != item[0] && item[1] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[1])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      if (item[2] != item[1] && item[2] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[2])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      } /* if group key selected */
     Par_Mod |= 0x0001;
     } /* if */
    break;
    } /* ARROW3 */

   case GP_ARROW4:
   case GP_ARROW10:
    {
    char *data;
    short i, oldframe;
    long frame, mult = 1, GroupKey;

    if (WCS_ID == ID_EMTL_TXTARROWLGRIGHT) mult = 10;
    get(EMTL_Win->FrameTxt, MUIA_Text_Contents, &data);
    oldframe = frame = atoi(data);
    if (frame < EM_Win->NextKey - mult)
     {
     frame += mult;
     sprintf(str, "%ld", frame);
     set(EMTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame = frame;
     SplineSingleKey(0, 0);
     if ((EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2
	|| EMTL_Win->KeyItem == 4 || EMTL_Win->KeyItem == 5)
		 && EMTL_Win->ElevProf)
      {
      EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
      EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
      if (EMTL_Win->KeyItem == 1 || EMTL_Win->KeyItem == 2)
       BuildVelocityTable(0, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
      else
       BuildVelocityTable(1, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
      } /* if camera lat or lon and velocity table already exists */
     Set_EMTL_Data();
     MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
     set(EM_Win->Str[0], MUIA_String_Integer, frame);
/* check for other key frames at this frame for the interactive group */
     get(EM_Win->BT_GroupKey, MUIA_Selected, &GroupKey);
     if (GroupKey)
      {
      if (item[0] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[0])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      if (item[1] != item[0] && item[1] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[1])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      if (item[2] != item[1] && item[2] != EMTL_Win->KeyItem)
       {
       if ((i = SearchKeyFrame(oldframe, 0, item[2])) >= 0)
        KF[i].MoKey.KeyFrame = frame;
       }
      } /* if group key selected */
     Par_Mod |= 0x0001;
     } /* if */
    break;
    } /* ARROW4 */

   } /* switch gadget group */

} /* Handle_EMTL_Window() */

/************************************************************************/
#define PATH_VECTOR 		0x0000
#define PATH_MOTION 		0x0001
#define PATH_MOTION_CAMERA	0x0011
#define PATH_MOTION_FOCUS	0x0021

short Set_EMTL_Item(short item)
{
 short i;
 LONG data;

 if (! BuildSingleKeyTable(0, item))
  return (0);
 if (item == 0 || item == 3)
  {
  EMTL_Win->ElevProf = BuildElevTable(&SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0],
	(item == 0 ? PATH_MOTION_CAMERA: PATH_MOTION_FOCUS));
  }
 else if (item == 1 || item == 2 || item == 4 || item == 5)
  {
  EMTL_Win->MaxMin[1][0] = EMTL_Win->MaxMin[0][0];
  EMTL_Win->MaxMin[1][1] = EMTL_Win->MaxMin[0][1];
  if (item < 3)
   EMTL_Win->ElevProf = BuildVelocityTable(0, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
  else
   EMTL_Win->ElevProf = BuildVelocityTable(1, &SKT[0]->Val[1],
	EMTL_Win->Frames, &EMTL_Win->MaxMin[1][0]);
  }
 else
  EMTL_Win->ElevProf = 0;

 EMTL_Win->ActiveKey = GetActiveKey(SKT[0], EM_Win->Frame);
 EM_Win->Frame = SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.KeyFrame;
 set(EM_Win->Str[0], MUIA_String_Integer, EM_Win->Frame);
 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  if (item == EMTL_Win->ListID[i])
   {
   set(EMTL_Win->ParCycle, MUIA_Cycle_Active, i);
   break;
   } /* if item matches list ID */
  } /* for i=0... */
 sprintf(str, "%d", EM_Win->Frame);
 set(EMTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
 set(EMTL_Win->BT_Linear, MUIA_Selected,
	SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.Linear);
 get (EMTL_Win->TCB_Cycle, MUIA_Cycle_Active, &data);
 sprintf(str, "%3.2f", EM_Win->TCB[data]);
 set(EMTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
 sprintf(str, "%f", SKT[0]->Key[EMTL_Win->ActiveKey]->MoKey.Value);
 set(EMTL_Win->ValStr[0], MUIA_String_Contents, (IPTR)str);
 EMTL_Win->KeyItem = item;

 return (1);

} /* Set_EMTL_Item() */

/************************************************************************/

void Set_EMTL_Data(void)
{
 LONG first, visible, SelState;
 float valdif;
 struct Data *data = INST_DATA(EMTL_Win->TL_Class, EMTL_Win->TimeLineObj[0]);

 get(EMTL_Win->Prop[0], MUIA_Prop_First, &first);
 get(EMTL_Win->Prop[0], MUIA_Prop_Visible, &visible);
 first = (first * EMTL_Win->Frames) / 100;
 data->lowframe = first < EMTL_Win->Frames - 10 ? first: EMTL_Win->Frames - 10;
 if (data->lowframe < 0) data->lowframe = 0;
 visible = (visible * EMTL_Win->Frames) / 100;
 data->highframe = first + (visible > 10 ? visible: 10);
 if (data->highframe > EMTL_Win->Frames) data->highframe = EMTL_Win->Frames;
 else if (data->highframe > EMTL_Win->Frames - 2) data->highframe = EMTL_Win->Frames;

 if (EMTL_Win->ElevProf)
  {
  if (EMTL_Win->MaxMin[1][0] > EMTL_Win->MaxMin[0][0])
   EMTL_Win->MaxMin[0][0] = EMTL_Win->MaxMin[1][0];
  if (EMTL_Win->MaxMin[1][1] < EMTL_Win->MaxMin[0][1])
   EMTL_Win->MaxMin[0][1] = EMTL_Win->MaxMin[1][1];
  } /* if elevation profile */
 valdif = EMTL_Win->MaxMin[0][0] - EMTL_Win->MaxMin[0][1];
 if (valdif < .02)
  {
  EMTL_Win->MaxMin[0][0] += .01;
  EMTL_Win->MaxMin[0][1] -= .01;
  valdif = EMTL_Win->MaxMin[0][0] - EMTL_Win->MaxMin[0][1];
  } /* if */ 
 data->texthighval = EMTL_Win->MaxMin[0][0] + .1 * valdif;
 data->textlowval = EMTL_Win->MaxMin[0][1] - .1 * valdif;
 if ( (data->highframe - data->lowframe) > 5000) data->framegrid = 500;
 else if ( (data->highframe - data->lowframe) > 1000) data->framegrid = 100;
 else if ( (data->highframe - data->lowframe) > 500) data->framegrid = 50;
 else if ( (data->highframe - data->lowframe) > 100) data->framegrid = 10;
 else if ( (data->highframe - data->lowframe) > 50) data->framegrid = 5;
 else data->framegrid = 1;

 data->framegridlg = data->framegrid * 5;
 data->framegridfirst =
	 data->lowframe - (data->lowframe % data->framegrid) + data->framegrid;

 if ( (data->texthighval - data->textlowval) > 50000.0)
  data->valgrid = 10000.0;
 else if ( (data->texthighval - data->textlowval) > 10000.0)
  data->valgrid = 5000.0;
 else if ( (data->texthighval - data->textlowval) > 5000.0)
  data->valgrid = 1000.0;
 else if ( (data->texthighval - data->textlowval) > 1000.0)
  data->valgrid = 500.0;
 else if ( (data->texthighval - data->textlowval) > 500.0)
  data->valgrid = 100.0;
 else if ( (data->texthighval - data->textlowval) > 100.0)
  data->valgrid = 50.0;
 else if ( (data->texthighval - data->textlowval) > 50.0)
  data->valgrid = 10.0;
 else if ( (data->texthighval - data->textlowval) > 10.0)
  data->valgrid = 5.0;
 else if ( (data->texthighval - data->textlowval) > 5.0)
  data->valgrid = 1.0;
 else if ( (data->texthighval - data->textlowval) > 1.0)
  data->valgrid = .5;
 else if ( (data->texthighval - data->textlowval) > .5)
  data->valgrid = .1;
 else if ( (data->texthighval - data->textlowval) > .1)
  data->valgrid = .05;
 else
  data->valgrid = .005;

 if (data->textlowval >= 0.0) data->valgridfirst =
	 ((long)(data->textlowval / data->valgrid) + 1) * data->valgrid;
 else data->valgridfirst =
	 ((long)(data->textlowval / data->valgrid)) * data->valgrid;

 get(EMTL_Win->BT_Grid, MUIA_Selected, &SelState);
 data->drawgrid = SelState;

 data->group = 0;
 data->SKT = SKT[0];
 data->activekey = EMTL_Win->ActiveKey;
 data->activeitem = 0;
 data->baseitem = 0;
 data->dataitems = 1 + EMTL_Win->ElevProf;
 data->win = EMTL_Win;

} /* Set_EMTL_Data() */

/************************************************************************/
/* Color Time Lines */

void Make_ECTL_Window(void)
{
 short i;
 long open;
 static const char *ECTL_Cycle_TCB[4] = {NULL};
 static int Init=TRUE;

 if(Init)
 {
     Init=FALSE;
     ECTL_Cycle_TCB[0] = (char*)GetString( MSG_TLGUI_TENS );  // "Tens"
     ECTL_Cycle_TCB[1] = (char*)GetString( MSG_TLGUI_CONT );  // "Cont"
     ECTL_Cycle_TCB[2] = (char*)GetString( MSG_TLGUI_BIAS );  // "Bias"
 }

 if (ECTL_Win)
  {
  DoMethod(ECTL_Win->TimeLineWin, MUIM_Window_ToFront);
  set(ECTL_Win->TimeLineWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  return;
  } /* if window already exists */

 if ((ECTL_Win = (struct TimeLineWindow *)
	get_Memory(sizeof (struct TimeLineWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((ECTL_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_ECTL_Window(1);
  return;
  } /* if out of memory */
 memcpy(ECTL_Win->AltKF, KF, KFsize);
 ECTL_Win->AltKFsize = KFsize;
 ECTL_Win->AltKeyFrames = ParHdr.KeyFrames;

 if ( ! (ECTL_Win->SuperClass = MUI_GetClass(MUIC_Area)))
  {
  Close_ECTL_Window(1);
  return;
  } /* if out of memory */

/* create the new class */
 if (!(ECTL_Win->TL_Class =
	 MakeClass(NULL, NULL, ECTL_Win->SuperClass, sizeof(struct Data), 0)))
 {
  MUI_FreeClass(ECTL_Win->SuperClass);
  return;
 }

/* set up parameter cycle list */
 ECTL_Win->ListSize = (COLORPARAMS + 1) * (sizeof (char *));
 ECTL_Win->ListIDSize = (COLORPARAMS) * (sizeof (short));

 if (((ECTL_Win->List = (char **)get_Memory(ECTL_Win->ListSize, MEMF_CLEAR)) == NULL)
	|| ((ECTL_Win->ListID = (short *)get_Memory(ECTL_Win->ListIDSize, MEMF_CLEAR)) == NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
               GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
               GetString( MSG_GLOBAL_OK ),                                // "OK"
               (CONST_STRPTR)"o");
  Close_ECTL_Window(1);
  return;
  } /* if out of memory */

 if (! Set_PS_List(ECTL_Win->List, ECTL_Win->ListID, 1, 2, NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
               GetString( MSG_TLGUI_NOCOLORPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERMI ),  // "No Color Parameters with more than one Key Frame!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                     // "OK"
               (CONST_STRPTR)"o");
  Close_ECTL_Window(1);
  return;
  } /* if out of memory */

/* set the dispatcher for the new class */
 ECTL_Win->TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
 ECTL_Win->TL_Class->cl_Dispatcher.h_SubEntry = NULL;
 ECTL_Win->TL_Class->cl_Dispatcher.h_Data     = NULL;

  Set_Param_Menu(1);

     ECTL_Win->TimeLineWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_TLGUI_COLORTIMELINE ),  // "Color Time Line"
      MUIA_Window_ID		, MakeID('E','C','T','L'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
        Child, HGroup, MUIA_Group_HorizSpacing, 0,
	  Child, RectangleObject, End,
	  Child, ECTL_Win->ParCycle = CycleObject,
		MUIA_Cycle_Entries, ECTL_Win->List, End,
	  Child, RectangleObject, End,
	  Child, Label2(" R:"),
	  Child, ECTL_Win->ValStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123",
		MUIA_String_Accept, "0123456789", End,
	  Child, Label2(" G:"),
	  Child, ECTL_Win->ValStr[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123",
		MUIA_String_Accept, "0123456789", End,
	  Child, Label2(" B:"),
	  Child, ECTL_Win->ValStr[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123",
		MUIA_String_Accept, "0123456789", End,
	  Child, RectangleObject, End,
          End, /* HGroup */
	Child, ECTL_Win->TimeLineObj[0] = NewObject(ECTL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	Child, HGroup,
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, VGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_PAN )),  // "  Pan "
                Child, ECTL_Win->Prop[0] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 100,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 100, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_ZOOM )),  // " Zoom "
                Child, ECTL_Win->Prop[1] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_FRAME_SPACE )),  // "Frame "
                Child, ECTL_Win->Prop[2] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      End, /* VGroup */
	    End, /* HGroup */
	  Child, VGroup,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, ECTL_Win->BT_PrevKey = KeyButtonObject('v'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_PREV ), End,  // "\33cPrev"
	      Child, ECTL_Win->BT_NextKey = KeyButtonObject('x'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_NEXT ), End,  // "\33cNext"
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, ECTL_Win->BT_AddKey = KeyButtonFunc('a', (char*)GetString( MSG_TLGUI_ADDKEY )),  // "\33cAdd Key"
	      Child, ECTL_Win->BT_DelKey = KeyButtonFunc(127, (char*)GetString( MSG_TLGUI_DELKEY )),  // "\33c\33uDel\33n Key"
	      End, /* HGroup */
	    Child, ECTL_Win->KeysExistTxt = TextObject, TextFrame, End,
	    End, /* VGroup */
	  End, /* HGroup */

	Child, HGroup,
	  Child, ECTL_Win->BT_Linear = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_LINEAR ), End,  // "\33cLinear"
	  Child, ECTL_Win->TCB_Cycle = Cycle(ECTL_Cycle_TCB),
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, ECTL_Win->CycleStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345", End,
            Child, ECTL_Win->StrArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, ECTL_Win->StrArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  Child, RectangleObject, End,
	  Child, ECTL_Win->FrameTxtLbl = Label1(GetString( MSG_TLGUI_FRAME )),  // "Frame"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, ECTL_Win->FrameTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
            Child, ECTL_Win->TxtArrowLg[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, ECTL_Win->TxtArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, ECTL_Win->TxtArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            Child, ECTL_Win->TxtArrowLg[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  End, /* HGroup */
	Child, HGroup,
	  Child, ECTL_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_TLGUI_KEEP )),  // "\33cKeep"
	  Child, ECTL_Win->BT_Grid = KeyButtonObject('g'),
	 	 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, TRUE,
	 	 MUIA_Text_Contents, GetString( MSG_TLGUI_GRID ), End,  // "\33cGrid"
	  Child, ECTL_Win->BT_Play = KeyButtonObject('p'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_PLAY ), End,  // "\33cPlay"
	  Child, ECTL_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),  // "\33cCancel"
	  End, /* HGroup */ 
	End, /* VGroup */
      End; /* WindowObject ECTL_Win->TimeLineWin */

  if (! ECTL_Win->TimeLineWin)
   {
   Close_ECTL_Window(1);
   User_Message(GetString( MSG_TLGUI_COLORTIMELINE ),  // "Color Time Line"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),             // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  DoMethod(app, OM_ADDMEMBER, ECTL_Win->TimeLineWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(ECTL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_ECTL_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, ECTL_Win->BT_Apply, ID_ECTL_APPLY,
   ECTL_Win->BT_Cancel, ID_ECTL_CLOSE,
   ECTL_Win->BT_AddKey, ID_ECTL_ADDKEY,
   ECTL_Win->BT_DelKey, ID_EC_DELETEKEY,
   ECTL_Win->BT_PrevKey, ID_EC_PREVKEY,
   ECTL_Win->BT_NextKey, ID_EC_NEXTKEY,
   ECTL_Win->TimeLineObj[0], ID_ECTL_TIMELINEOBJ, NULL);

  /* <<<>>> Condense these */
  DoMethod(ECTL_Win->BT_Play, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_ECTL_PLAY);  
  DoMethod(ECTL_Win->BT_Grid, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_ECTL_GRID);  
  DoMethod(ECTL_Win->BT_Linear, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_ECTL_LINEAR);  

/* set values */
  set(ECTL_Win->Prop[0], MUIA_Prop_First, 0);
  set(ECTL_Win->Prop[0], MUIA_Prop_Visible, 100);
  set(ECTL_Win->Prop[1], MUIA_Prop_First, 100);
  if (! Set_ECTL_Item(EC_Win->PalItem))
   {
   Close_ECTL_Window(1);
   User_Message(GetString( MSG_TLGUI_COLOREDITORTIMELINES ),                                   // "Color Editor: Time Lines"
                GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* if build key table failed */
  Set_ECTL_Data(0);
  DisableKeyButtons(1);

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app,
   ECTL_Win->TxtArrow[0], ID_ECTL_TXTARROWLEFT,
   ECTL_Win->TxtArrow[1], ID_ECTL_TXTARROWRIGHT,
   ECTL_Win->TxtArrowLg[0], ID_ECTL_TXTARROWLGLEFT,
   ECTL_Win->TxtArrowLg[1], ID_ECTL_TXTARROWLGRIGHT,
   ECTL_Win->StrArrow[0], ID_ECTL_STRARROWLEFT,
   ECTL_Win->StrArrow[1], ID_ECTL_STRARROWRIGHT, NULL);

/* Link prop gadgets to application and each other */
  DoMethod(ECTL_Win->Prop[0], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_ECTL_PANPROP);
  DoMethod(ECTL_Win->Prop[0], MUIM_Notify, MUIA_Prop_Visible, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_ECTL_PANPROP);
  DoMethod(ECTL_Win->Prop[1], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     ECTL_Win->Prop[0], 3, MUIM_Set, MUIA_Prop_Visible, MUIV_TriggerValue);
  DoMethod(ECTL_Win->Prop[2], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_ECTL_FRAMEPROP);
	
/* link string gadgets to application */
  DoMethod(ECTL_Win->CycleStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_ECTL_CYCLESTR);
  for (i=0; i<3; i++)
   {
   DoMethod(ECTL_Win->ValStr[i], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_ECTL_VALSTR(i));
   } /* for i=0... */

/* Link cycle gadgets to application */
  DoMethod(ECTL_Win->TCB_Cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ECTL_CYCLE);
  DoMethod(ECTL_Win->ParCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ECTL_PARCYCLE);

/* Open window */
  set(ECTL_Win->TimeLineWin, MUIA_Window_Open, TRUE);
  get(ECTL_Win->TimeLineWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_ECTL_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(ECTL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_ECTL_ACTIVATE);

/* Get Window structure pointer */
  get(ECTL_Win->TimeLineWin, MUIA_Window_Window, &ECTL_Win->Win);

} /* Make_MTL_Window() */

/*********************************************************************/

void Close_ECTL_Window(short apply)
{
 if (ECTL_Win)
  {
  if (SKT[1]) FreeSingleKeyTable(1, ECTL_Win->Frames);
  if (ECTL_Win->AltKF)
   {
   if (apply) free_Memory(ECTL_Win->AltKF, ECTL_Win->AltKFsize);
   else
    {
    MergeKeyFrames(ECTL_Win->AltKF, ECTL_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 1);
    free_Memory(ECTL_Win->AltKF, ECTL_Win->AltKFsize);
    ResetTimeLines(1);
    UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
    Set_EC_Item(EC_Win->PalItem);
    } /* else discard changes */
   } /* if */
  if (ECTL_Win->TimeLineWin)
   {
   set(ECTL_Win->TimeLineWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, ECTL_Win->TimeLineWin);
   MUI_DisposeObject(ECTL_Win->TimeLineWin);
   } /* if window created */
  if (ECTL_Win->List) free_Memory(ECTL_Win->List, ECTL_Win->ListSize);
  if (ECTL_Win->ListID) free_Memory(ECTL_Win->ListID, ECTL_Win->ListIDSize);
  if (ECTL_Win->TL_Class) FreeClass(ECTL_Win->TL_Class);         /* free our custom class. */
  MUI_FreeClass(ECTL_Win->SuperClass); /* release super class pointer. */
  free_Memory(ECTL_Win, sizeof (struct TimeLineWindow));
  ECTL_Win = NULL;
  DisableKeyButtons(1);
  } /* if */

} /* Close_ECTL_Window() */

/*********************************************************************/

void Handle_ECTL_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_ECTL_Window();
   return;
   } /* Open Color Time Line Window */

  if (! ECTL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Color Time Line window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_ECTL_ADDKEY:
      {
      struct Data *data = INST_DATA(ECTL_Win->TL_Class, ECTL_Win->TimeLineObj[0]);

      data->inputflags = KEYFRAME_NEW;
      if (GetInput_Pt(ECTL_Win->TL_Class, ECTL_Win->TimeLineObj[0]))
       {
       if (MakeKeyFrame((short)EC_Win->Frame, 1, EC_Win->PalItem))
        {
        UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
        DisableKeyButtons(1);
        Set_ECTL_Item(EC_Win->PalItem);
        Set_ECTL_Data(-1);
        MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
        ResetTimeLines(1); 
        Par_Mod |= 0x0010;
        } /* if new key frame */
       else data->inputflags = 0;
       } /* if input point */
      break;
      }
     case ID_ECTL_LINEAR:
      {
      long SelState;

      get(ECTL_Win->BT_Linear, MUIA_Selected, &SelState);
      SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.Linear = SelState;
      SplineSingleKey(1, 0);
      Set_ECTL_Data(-1);
      MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      break;
      } /* Linear */
     case ID_ECTL_GRID:
      {
      long SelState;
      struct Data *data = INST_DATA(ECTL_Win->TL_Class, ECTL_Win->TimeLineObj[0]);

      get(ECTL_Win->BT_Grid, MUIA_Selected, &SelState);
      data->drawgrid = SelState;
      MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      break;
      }
     case ID_ECTL_PLAY:
      {
      Play_Colors();
      break;
      }
     case ID_ECTL_APPLY:
      {
      Close_ECTL_Window(1);
      break;
      }
     case ID_ECTL_CLOSE:
      {
      Close_ECTL_Window(0);
      break;
      }
     case ID_ECTL_CLOSEQUERY:
      {
      if (KFsize != ECTL_Win->AltKFsize || memcmp(KF, ECTL_Win->AltKF, KFsize))
       Close_ECTL_Window(CloseWindow_Query(GetString( MSG_TLGUI_COLORTIMELINES )));  // "Color Time Lines"
      else
       Close_ECTL_Window(1);
      break;
      }
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    char *value;
    LONG item;

    get(ECTL_Win->CycleStr, MUIA_String_Contents, &value);
    get(ECTL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    EC_Win->TCB[item] = atof(value);
    SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.TCB[item] = atof(value);
    SplineSingleKey(1, 0);
    Set_ECTL_Data(-1);
    MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    break;
    } /* TCB string */

   case GP_STRING2:
    {
    LONG value;

    i = WCS_ID - ID_ECTL_VALSTR(0);
    get(ECTL_Win->ValStr[i], MUIA_String_Integer, &value);
    if (value > 255)
     value = 255;
    else if (value < 0)
     value = 0;
    SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.Value[i] = (short)value;
    set(EC_Win->CoStr[i], MUIA_String_Integer, value);
    SplineSingleKey(1, 0);
    Set_ECTL_Data(-1);
    MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    break;
    } /* Value string */

   case GP_PROP1:
    {
    ULONG signals;
    struct Data *data = INST_DATA(ECTL_Win->TL_Class, ECTL_Win->TimeLineObj[0]);

    data->inputflags |= QUICK_DRAW;
    Set_ECTL_Data(-1);
    MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);

    data->inputflags |= NO_CLEAR;

    while ((WCS_ID = DoMethod(app, MUIM_Application_Input, &signals))
            == ID_ECTL_PANPROP)
    {

    };
    {
        Set_ECTL_Data(-1);
        MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    };

    data->inputflags ^= (QUICK_DRAW | NO_CLEAR);
    MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
    break;
    } /* pan prop */

   case GP_PROP2:
    {
    long data;

    if (! EC_Win->StrBlock)
     {
     get(ECTL_Win->Prop[2], MUIA_Prop_First, &data);
     data = ((float)data / 100.0) * ECTL_Win->Frames;
     if (data < 1) data = 1;
     set(EC_Win->Str[0], MUIA_String_Integer, data);
     EC_Win->PropBlock = 1;
     }
    EC_Win->StrBlock = 0;
    break;
    } /* frame prop */

   case GP_CYCLE1:
    {
    LONG item;

    get(ECTL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EC_Win->TCB[item]);
    set(ECTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Cycle */

   case GP_CYCLE2:
    {
    long data;

    get(ECTL_Win->ParCycle, MUIA_Cycle_Active, &data);
    set(EC_Win->LS_List, MUIA_List_Active, ECTL_Win->ListID[data]);
    break;
    } /* parameter cycle */

   case GP_ARROW1:
    {
    char *value;

    get(ECTL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) - .1);
    set(ECTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW2:
    {
    char *value;

    get(ECTL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) + .1);
    set(ECTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW3:
   case GP_ARROW9:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_ECTL_TXTARROWLGLEFT) mult = 10;
    get(ECTL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame >= mult && (! EC_Win->PrevKey || frame > EC_Win->PrevKey + mult))
     {
     frame -= mult;
     sprintf(str, "%ld", frame);
     set(ECTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.KeyFrame = frame;
     SplineSingleKey(1, 0);
     Set_ECTL_Data(-1);
     MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
     set(EC_Win->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0010;
     } /* if */
    break;
    } /* ARROW3 */

   case GP_ARROW4:
   case GP_ARROW10:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_ECTL_TXTARROWLGRIGHT) mult = 10;
    get(ECTL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame < EC_Win->NextKey - mult)
     {
     frame += mult;
     sprintf(str, "%ld", frame);
     set(ECTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.KeyFrame = frame;
     SplineSingleKey(1, 0);
     Set_ECTL_Data(-1);
     MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
     set(EC_Win->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0010;
     } /* if */
    break;
    } /* ARROW4 */

#ifdef KJLKJLKJDS
// Parameter Arrows - obsolete.
 Also the function GetNextKeyitem is obsolete I think
   case GP_ARROW5:
    {
    if (EC_Win->PalItem > 0)
     {
     EC_Win->PalItem = GetNextKeyItem(1, EC_Win->PalItem, -1);
     set(EC_Win->LS_List, MUIA_List_Active, EC_Win->PalItem);
     } /* if */
    break;
    } /* parameter name arrow */

   case GP_ARROW6:
    {
    if (EC_Win->PalItem < COLORPARAMS - 1)
     {
     EC_Win->PalItem = GetNextKeyItem(1, EC_Win->PalItem, +1);
     set(EC_Win->LS_List, MUIA_List_Active, EC_Win->PalItem);
     } /* if */
    break;
    } /* parameter name arrow */
#endif
   } /* switch gadget group */

} /* Handle_ECTL_Window() */

/*********************************************************************/

short Set_ECTL_Item(short item)
{
 short i;
 LONG data;

 if (! BuildSingleKeyTable(1, item)) return (0);
 ECTL_Win->ActiveKey = GetActiveKey(SKT[1], EC_Win->Frame);
 EC_Win->Frame = SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.KeyFrame;
 set(EC_Win->Str[0], MUIA_String_Integer, EC_Win->Frame);
 set(ECTL_Win->BT_Linear, MUIA_Selected,
	SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.Linear);
 for (i=0; i<COLORPARAMS; i++)
  {
  if (item == ECTL_Win->ListID[i])
   {
   set(ECTL_Win->ParCycle, MUIA_Cycle_Active, i);
   break;
   } /* if item matches list ID */
  } /* for i=0... */
/* set(ECTL_Win->ParTxt, MUIA_Text_Contents, PAR_NAME_COLOR(EC_Win->PalItem));*/
 sprintf(str, "%d", EC_Win->Frame);
 set(ECTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
 get (ECTL_Win->TCB_Cycle, MUIA_Cycle_Active, &data);
 sprintf(str, "%3.2f", EC_Win->TCB[data]);
 set(ECTL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
 for (i=0; i<3; i++)
  {
  sprintf(str, "%1d", SKT[1]->Key[ECTL_Win->ActiveKey]->CoKey.Value[i]);
  set(ECTL_Win->ValStr[i], MUIA_String_Contents, (IPTR)str);
  } /* for i=0... */
 ECTL_Win->KeyItem = item;

 return (1);
} /* Set_ECTL_Item() */

/*********************************************************************/

void Set_ECTL_Data(short subitem)
{
 LONG first, visible, SelState;
 float maxval, minval, valdif;
 struct Data *data = INST_DATA(ECTL_Win->TL_Class, ECTL_Win->TimeLineObj[0]);

 get(ECTL_Win->Prop[0], MUIA_Prop_First, &first);
 get(ECTL_Win->Prop[0], MUIA_Prop_Visible, &visible);
 first = (first * ECTL_Win->Frames) / 100;
 data->lowframe = first < ECTL_Win->Frames - 10 ? first: ECTL_Win->Frames - 10;
 if (data->lowframe < 0) data->lowframe = 0;
 visible = (visible * ECTL_Win->Frames) / 100;
 data->highframe = first + (visible > 10 ? visible: 10);
 if (data->highframe > ECTL_Win->Frames) data->highframe = ECTL_Win->Frames;
 else if (data->highframe > ECTL_Win->Frames - 2) data->highframe = ECTL_Win->Frames;
 maxval = max(ECTL_Win->MaxMin[0][0], ECTL_Win->MaxMin[1][0]);
 maxval = max(maxval, ECTL_Win->MaxMin[2][0]);

 minval = min(ECTL_Win->MaxMin[0][1], ECTL_Win->MaxMin[1][1]);
 minval = min(minval, ECTL_Win->MaxMin[2][1]);

 valdif = maxval - minval;
 if (valdif < 10.0)
  {
  maxval += 5.0;
  minval -= 5.0;
  valdif = maxval - minval;
  } /* if */ 
 data->texthighval = (short)(maxval + .1 * valdif);
 data->textlowval = (short)(minval - .1 * valdif);
 if (data->texthighval > 255) data->texthighval = 255;
 if (data->textlowval < 0) data->textlowval = 0;
 if ( (data->highframe - data->lowframe) > 5000) data->framegrid = 500;
 else if ( (data->highframe - data->lowframe) > 1000) data->framegrid = 100;
 else if ( (data->highframe - data->lowframe) > 500) data->framegrid = 50;
 else if ( (data->highframe - data->lowframe) > 100) data->framegrid = 10;
 else if ( (data->highframe - data->lowframe) > 50) data->framegrid = 5;
 else data->framegrid = 1;

 data->framegridlg = data->framegrid * 5;
 data->framegridfirst =
	 data->lowframe - (data->lowframe % data->framegrid) + data->framegrid;

 if ( (data->texthighval - data->textlowval) > 100.0)
  data->valgrid = 50.0;
 else if ( (data->texthighval - data->textlowval) > 50.0)
  data->valgrid = 10.0;
 else if ( (data->texthighval - data->textlowval) > 10.0)
  data->valgrid = 5.0;
 else 
  data->valgrid = 1.0;

 if (data->textlowval >= 0.0) data->valgridfirst =
	 ((long)(data->textlowval / data->valgrid) + 1) * data->valgrid;
 else data->valgridfirst =
	 ((long)(data->textlowval / data->valgrid)) * data->valgrid;

 get(ECTL_Win->BT_Grid, MUIA_Selected, &SelState);
 data->drawgrid = SelState;

 data->group = 1;
 data->SKT = SKT[1];
 data->activekey = ECTL_Win->ActiveKey;
 if (subitem >= 0) data->activeitem = subitem;
 data->baseitem = 0;
 data->dataitems = 3;
 data->win = ECTL_Win;

} /* Set_ECTL_Data() */


/***********************************************************************/
/* Ecosystem Time Lines */

void Make_EETL_Window(void)
{
 short i;
 long open;
 static const char *EETL_Cycle_TCB[3] = {NULL};

 static const char *EETL_TimeLines[11] = {NULL};

 static int Init=TRUE;

 if(Init)
 {
	 Init=FALSE;

	 EETL_Cycle_TCB[0] = (char*)GetString( MSG_TLGUI_TENS );  // "Tens"
	 EETL_Cycle_TCB[1] = (char*)GetString( MSG_TLGUI_CONT );  // "Cont"
	 EETL_Cycle_TCB[2] = (char*)GetString( MSG_TLGUI_BIAS );  // "Bias"

	 EETL_TimeLines[0] = (char*)GetString( MSG_TLGUI_ELEVATIONLINE );            // "\0334Elevation Line"
	 EETL_TimeLines[1] = (char*)GetString( MSG_TLGUI_SKEW );                     // "\0334Skew"
	 EETL_TimeLines[2] = (char*)GetString( MSG_TLGUI_AZIMUTH );                  // "\0334Azimuth"
	 EETL_TimeLines[3] = (char*)GetString( MSG_TLGUI_RELATIVEELEVATIONEFFECT );  // "\0334Relative Elevation Effect"
	 EETL_TimeLines[4] = (char*)GetString( MSG_TLGUI_MXRELATIVEELEVATION );      // "\0334Mx Relative Elevation"
	 EETL_TimeLines[5] = (char*)GetString( MSG_TLGUI_MNRELATIVEELEVATION );      // "\0334Mn Relative Elevation"
	 EETL_TimeLines[6] = (char*)GetString( MSG_TLGUI_MXSLOPE );                  // "\0334Mx Slope"
	 EETL_TimeLines[7] = (char*)GetString( MSG_TLGUI_MNSLOPE );                  // "\0334Mn Slope"
	 EETL_TimeLines[8] = (char*)GetString( MSG_TLGUI_DENSITY );                  // "\0334Density"
	 EETL_TimeLines[9] = (char*)GetString( MSG_TLGUI_HEIGHT );                   // "\0334Height"
 }

 if (EETL_Win)
  {
  DoMethod(EETL_Win->TimeLineWin, MUIM_Window_ToFront);
  set(EETL_Win->TimeLineWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  return;
  } /* if window already exists */

 if ((EETL_Win = (struct TimeLineWindow *)
	get_Memory(sizeof (struct TimeLineWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((EETL_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_EETL_Window(1);
  return;
  } /* if out of memory */
 memcpy(EETL_Win->AltKF, KF, KFsize);
 EETL_Win->AltKFsize = KFsize;
 EETL_Win->AltKeyFrames = ParHdr.KeyFrames;

 if ( ! (EETL_Win->SuperClass = MUI_GetClass(MUIC_Area)))
  {
  Close_EETL_Window(1);
  return;
  } /* if out of memory */

/* create the new class */
 if (!(EETL_Win->TL_Class =
	 MakeClass(NULL, NULL, EETL_Win->SuperClass, sizeof(struct Data), 0)))
 {
  MUI_FreeClass(EETL_Win->SuperClass);
  return;
 }

/* set up parameter cycle list */
 EETL_Win->ListSize = (USEDMOTIONPARAMS + 1) * (sizeof (char *));
 EETL_Win->ListIDSize = (USEDMOTIONPARAMS) * (sizeof (short));

 if (((EETL_Win->List = (char **)get_Memory(EETL_Win->ListSize, MEMF_CLEAR)) == NULL)
	|| ((EETL_Win->ListID = (short *)get_Memory(EETL_Win->ListIDSize, MEMF_CLEAR)) == NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
               GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
               GetString( MSG_GLOBAL_OK ),                                // "OK"
               (CONST_STRPTR)"o");
  Close_EETL_Window(1);
  return;
  } /* if out of memory */

 if (! Set_PS_List(EETL_Win->List, EETL_Win->ListID, 2, 2, NULL))
  {
  User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
               GetString( MSG_TLGUI_NOECOSYSTEMPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONT ),  // "No Ecosystem Parameters with more than one Key Frame!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                     // "OK"
               (CONST_STRPTR)"o");
  Close_EETL_Window(1);
  return;
  } /* if out of memory */

/* set the dispatcher for the new class */
 EETL_Win->TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
 EETL_Win->TL_Class->cl_Dispatcher.h_SubEntry = NULL;
 EETL_Win->TL_Class->cl_Dispatcher.h_Data     = NULL;

  Set_Param_Menu(2);

     EETL_Win->TimeLineWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_TLGUI_ECOSYSTEMTIMELINE ),  // "Ecosystem Time Line"
      MUIA_Window_ID		, MakeID('E','E','T','L'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
        Child, HGroup, MUIA_Group_HorizSpacing, 0,
	  Child, RectangleObject, End,
	  Child, EETL_Win->ParCycle = CycleObject,
		MUIA_Cycle_Entries, EETL_Win->List, End,
	  Child, RectangleObject, End,
	  Child, EETL_Win->ValStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345",
		MUIA_String_Accept, ".-0123456789", End,
	  Child, RectangleObject, End,
          End, /* HGroup */
	Child, EETL_Win->TimeLineGroup = RegisterGroup(EETL_TimeLines),
	  Child, EETL_Win->TimeLineObj[0] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[1] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[2] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[3] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[4] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[5] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[6] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[7] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[8] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  Child, EETL_Win->TimeLineObj[9] = NewObject(EETL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE),
	  End, /* RegisterGroup */
	Child, HGroup,
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, VGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_PAN )),  // "  Pan "
                Child, EETL_Win->Prop[0] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 100,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 100, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_ZOOM )),  // " Zoom "
                Child, EETL_Win->Prop[1] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(GetString( MSG_TLGUI_FRAME_SPACE )),  // "Frame "
                Child, EETL_Win->Prop[2] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      End, /* VGroup */
	    End, /* HGroup */
	  Child, VGroup,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, EETL_Win->BT_PrevKey = KeyButtonObject('v'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_PREV ), End,  // "\33cPrev"
	      Child, EETL_Win->BT_NextKey = KeyButtonObject('x'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, GetString( MSG_TLGUI_NEXT ), End,  // "\33cNext"
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, EETL_Win->BT_AddKey = KeyButtonFunc('a', (char*)GetString( MSG_TLGUI_ADDKEY )),  // "\33cAdd Key"
	      Child, EETL_Win->BT_DelKey = KeyButtonFunc(127, (char*)GetString( MSG_TLGUI_DELKEY )),  // "\33c\33uDel\33n Key"
	      End, /* HGroup */
	    Child, EETL_Win->KeysExistTxt = TextObject, TextFrame, End,
	    End, /* VGroup */
	  End, /* HGroup */

	Child, HGroup,
	  Child, EETL_Win->BT_Linear = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_TLGUI_LINEAR ), End,  // "\33cLinear"
	  Child, EETL_Win->TCB_Cycle = Cycle(EETL_Cycle_TCB),
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, EETL_Win->CycleStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345", End,
            Child, EETL_Win->StrArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EETL_Win->StrArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  Child, RectangleObject, End,
	  Child, EETL_Win->FrameTxtLbl = Label1(GetString( MSG_TLGUI_FRAME )),  // "Frame"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, EETL_Win->FrameTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
            Child, EETL_Win->TxtArrowLg[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EETL_Win->TxtArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
            Child, EETL_Win->TxtArrow[1] = ImageButtonWCS(MUII_ArrowRight),
            Child, EETL_Win->TxtArrowLg[1] = ImageButtonWCS(MUII_ArrowRight),
            End, /* HGroup */
	  End, /* HGroup */
	Child, HGroup,
	  Child, EETL_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_TLGUI_KEEP )),  // "\33cKeep"
	  Child, EETL_Win->BT_Grid = KeyButtonObject('g'),
	 	 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, TRUE,
	 	 MUIA_Text_Contents, GetString( MSG_TLGUI_GRID ), End,  // "\33cGrid"
/* No way to play yet devised but we'll find one!!
	  Child, EETL_Win->BT_Play = KeyButtonObject('p'),
		 MUIA_Text_Contents, "\33cPlay", End, 
*/
	  Child, EETL_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),  // "\33cCancel"
	  End, /* HGroup */ 
	End, /* VGroup */
      End; /* WindowObject EETL_Win->TimeLineWin */

  if (! EETL_Win->TimeLineWin)
   {
   Close_EETL_Window(1);
   User_Message(GetString( MSG_TLGUI_ECOSYSTEMTIMELINE ),  // "Ecosystem Time Line"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),        // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),                 // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);

  DoMethod(app, OM_ADDMEMBER, EETL_Win->TimeLineWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(EETL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EETL_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, EETL_Win->BT_Apply, ID_EETL_APPLY,
   EETL_Win->BT_Cancel, ID_EETL_CLOSE,
   EETL_Win->BT_AddKey, ID_EETL_ADDKEY,
   EETL_Win->BT_DelKey, ID_EE_DELETEKEY,
   EETL_Win->BT_PrevKey, ID_EE_PREVKEY,
   EETL_Win->BT_NextKey, ID_EE_NEXTKEY, NULL);

  DoMethod(EETL_Win->TimeLineGroup, MUIM_Notify, MUIA_Group_ActivePage, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EETL_TLGROUP);  

  for (i=0; i<10; i++)
   MUI_DoNotiPresFal(app, EETL_Win->TimeLineObj[i], ID_EETL_TIMELINEOBJ(i), NULL);

/*  
  DoMethod(EETL_Win->BT_Play, MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EETL_PLAY);  
*/
  DoMethod(EETL_Win->BT_Grid, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EETL_GRID);  
  DoMethod(EETL_Win->BT_Linear, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EETL_LINEAR);  

/* set values */
  set(EETL_Win->Prop[0], MUIA_Prop_First, 0);
  set(EETL_Win->Prop[0], MUIA_Prop_Visible, 100);
  set(EETL_Win->Prop[1], MUIA_Prop_First, 100);
  EETL_Win->ActiveItem = 0;  
  if (! Set_EETL_Item(EE_Win->EcoItem))
   {
   Close_EETL_Window(1);
   User_Message(GetString( MSG_TLGUI_ECOSYSTEMEDITORTIMELINES ),                               // "Ecosystem Editor: Time Lines"
                GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* if build key table failed */
  for (i=0; i<10; i++)
   {
   Set_EETL_Data(i);
   } /* for i=0... */
  DisableKeyButtons(2);

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app,
   EETL_Win->TxtArrow[0], ID_EETL_TXTARROWLEFT,
   EETL_Win->TxtArrow[1], ID_EETL_TXTARROWRIGHT,
   EETL_Win->TxtArrowLg[0], ID_EETL_TXTARROWLGLEFT,
   EETL_Win->TxtArrowLg[1], ID_EETL_TXTARROWLGRIGHT,
   EETL_Win->StrArrow[0], ID_EETL_STRARROWLEFT,
   EETL_Win->StrArrow[1], ID_EETL_STRARROWRIGHT, NULL);

/* Link prop gadgets to application and each other */
  DoMethod(EETL_Win->Prop[0], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EETL_PANPROP);
  DoMethod(EETL_Win->Prop[0], MUIM_Notify, MUIA_Prop_Visible, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EETL_PANPROP);
  DoMethod(EETL_Win->Prop[1], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     EETL_Win->Prop[0], 3, MUIM_Set, MUIA_Prop_Visible, MUIV_TriggerValue);
  DoMethod(EETL_Win->Prop[2], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EETL_FRAMEPROP);
	
/* link string gadgets to application */
  DoMethod(EETL_Win->CycleStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EETL_CYCLESTR);
  DoMethod(EETL_Win->ValStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EETL_VALSTR(0));

/* Link cycle gadgets to application */
  DoMethod(EETL_Win->TCB_Cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EETL_CYCLE);
  DoMethod(EETL_Win->ParCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EETL_PARCYCLE);

/* Open window */
  set(EETL_Win->TimeLineWin, MUIA_Window_Open, TRUE);
  get(EETL_Win->TimeLineWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EETL_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EETL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EETL_ACTIVATE);

/* Get Window structure pointer */
  get(EETL_Win->TimeLineWin, MUIA_Window_Window, &EETL_Win->Win);

} /* Make_MTL_Window() */

/*********************************************************************/

void Close_EETL_Window(short apply)
{
 if (EETL_Win)
  {
  if (SKT[2]) FreeSingleKeyTable(2, EETL_Win->Frames);
  if (EETL_Win->AltKF)
   {
   if (apply) free_Memory(EETL_Win->AltKF, EETL_Win->AltKFsize);
   else
    {
    MergeKeyFrames(EETL_Win->AltKF, EETL_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 2);
    free_Memory(EETL_Win->AltKF, EETL_Win->AltKFsize);
    ResetTimeLines(2);
    UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
    Set_EE_Item(EE_Win->EcoItem);
    } /* else discard changes */
   } /* if */
  if (EETL_Win->TimeLineWin)
   {
   set(EETL_Win->TimeLineWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, EETL_Win->TimeLineWin);
   MUI_DisposeObject(EETL_Win->TimeLineWin);
   } /* if window created */
  if (EETL_Win->List) free_Memory(EETL_Win->List, EETL_Win->ListSize);
  if (EETL_Win->ListID) free_Memory(EETL_Win->ListID, EETL_Win->ListIDSize);
  if (EETL_Win->TL_Class) FreeClass(EETL_Win->TL_Class);         /* free our custom class. */
  MUI_FreeClass(EETL_Win->SuperClass); /* release super class pointer. */
  free_Memory(EETL_Win, sizeof (struct TimeLineWindow));
  EETL_Win = NULL;
  DisableKeyButtons(2);
  } /* if */

} /* Close_EETL_Window() */

/*********************************************************************/

void Handle_EETL_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EETL_Window();
   return;
   } /* Open Ecosystem Time Line Window */

  if (! EETL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Ecosystem Time Line window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EETL_ADDKEY:
      {
      struct Data *data = INST_DATA(EETL_Win->TL_Class,
	 EETL_Win->TimeLineObj[EETL_Win->ActiveItem]);

      data->inputflags = KEYFRAME_NEW;
      if (GetInput_Pt(EETL_Win->TL_Class, EETL_Win->TimeLineObj[EETL_Win->ActiveItem]))
       {
       if (MakeKeyFrame((short)EE_Win->Frame, 2, EE_Win->EcoItem))
        {
        UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
        DisableKeyButtons(2);
        Set_EETL_Item(EE_Win->EcoItem);
        for (i=0; i<10; i++)
         Set_EETL_Data(i);
        MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT); 
        ResetTimeLines(2);
        Par_Mod |= 0x0100;
        } /* if new key frame */
       else data->inputflags = 0;
       } /* if input point */
      break;
      }
     case ID_EETL_LINEAR:
      {
      long SelState;

      get(EETL_Win->BT_Linear, MUIA_Selected, &SelState);
      SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.Linear = SelState;
      SplineSingleKey(2, 0);
      for (i=0; i<10; i++)
       {
       Set_EETL_Data(i);
       } /* for i=0... */
      MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
      break;
      } /* Linear */
     case ID_EETL_GRID:
      {
      long SelState;
      
      get(EETL_Win->BT_Grid, MUIA_Selected, &SelState);
      for (i=0; i<10; i++)
       {
       struct Data *data = INST_DATA(EETL_Win->TL_Class,
	 EETL_Win->TimeLineObj[i]);

       data->drawgrid = SelState;
       } /* for i=0... */
      MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT); 
      break;
      }
     case ID_EETL_PLAY:
      {
      break;
      }
     case ID_EETL_APPLY:
      {
      Close_EETL_Window(1);
      break;
      }
     case ID_EETL_CLOSE:
      {
      Close_EETL_Window(0);
      break;
      }
     case ID_EETL_CLOSEQUERY:
      {
      if (KFsize != EETL_Win->AltKFsize || memcmp(KF, EETL_Win->AltKF, KFsize))
       Close_EETL_Window(CloseWindow_Query(GetString( MSG_TLGUI_ECOSYSTEMTIMELINES )));  // "Ecosystem Time Lines"
      else
       Close_EETL_Window(1);
      break;
      }
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    char *value;
    LONG item;

    get(EETL_Win->CycleStr, MUIA_String_Contents, &value);
    get(EETL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    EE_Win->TCB[item] = atof(value);
    SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.TCB[item] = atof(value);
    SplineSingleKey(2, 0);
    for (i=0; i<10; i++)
     {
     Set_EETL_Data(i);
     } /* for i=0... */
    MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* TCB string */

   case GP_STRING2:
    {
    char *floatvalue;
    double value;

    get(EETL_Win->ValStr[0], MUIA_String_Contents, &floatvalue);
    value = atof(floatvalue);
    if (value > EE_Win->EcoLimits[EETL_Win->ActiveItem][0])
     value = EE_Win->EcoLimits[EETL_Win->ActiveItem][0];
    else if (value < EE_Win->EcoLimits[EETL_Win->ActiveItem][1])
     value = EE_Win->EcoLimits[EETL_Win->ActiveItem][1];
    SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.Value[EETL_Win->ActiveItem]
	 = value;
    setfloat(EE_Win->IntStr[EETL_Win->ActiveItem], value);
    SplineSingleKey(2, 0);
    Set_EETL_Data(EETL_Win->ActiveItem);
    MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* Value string */

   case GP_PROP1:
    {
    ULONG signals;
    struct Data *data = INST_DATA(EETL_Win->TL_Class, EETL_Win->TimeLineObj[0]);

    data->inputflags |= QUICK_DRAW;
    Set_EETL_Data(EETL_Win->ActiveItem);
    MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);

    data->inputflags |= NO_CLEAR;

    while ((WCS_ID = DoMethod(app, MUIM_Application_Input, &signals))
            == ID_EETL_PANPROP)
    {

    };

    {
        Set_EETL_Data(EETL_Win->ActiveItem);
        MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
    };

    data->inputflags ^= (QUICK_DRAW | NO_CLEAR);
    MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* pan prop */

   case GP_PROP2:
    {
    long data;

    if (! EE_Win->StrBlock)
     {
     get(EETL_Win->Prop[2], MUIA_Prop_First, &data);
     data = ((float)data / 100.0) * EETL_Win->Frames;
     if (data < 1) data = 1;
     set(EE_Win->Str[0], MUIA_String_Integer, data);
     EE_Win->PropBlock = 1;
     }
    EE_Win->StrBlock = 0;
    break;
    } /* frame prop */

   case GP_CYCLE1:
    {
    LONG item;

    get(EETL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", EE_Win->TCB[item]);
    set(EETL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Cycle */

   case GP_CYCLE2:
    {
    long data;

    get(EETL_Win->ParCycle, MUIA_Cycle_Active, &data);
    set(EE_Win->LS_List, MUIA_List_Active, EETL_Win->ListID[data]);
    break;
    } /* parameter cycle */

   case GP_CYCLE3:
    {
    LONG item;

    get(EETL_Win->TimeLineGroup, MUIA_Group_ActivePage, &item);
    EETL_Win->ActiveItem = item;
    setfloat(EETL_Win->ValStr[0],
	 SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.Value[EETL_Win->ActiveItem]);
    break;
    } /* Register group */

   case GP_ARROW1:
    {
    char *value;

    get(EETL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) - .1);
    set(EETL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW2:
    {
    char *value;

    get(EETL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) + .1);
    set(EETL_Win->CycleStr, MUIA_String_Contents, (IPTR)str);
    break;
    } /* TCB Arrow */

   case GP_ARROW3:
   case GP_ARROW9:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_EETL_TXTARROWLGLEFT) mult = 10;
    get(EETL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame >= mult && (! EE_Win->PrevKey || frame > EE_Win->PrevKey + mult))
     {
     frame -= mult;
     sprintf(str, "%ld", frame);
     set(EETL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.KeyFrame = frame;
     SplineSingleKey(2, 0);
     Set_EETL_Data(EETL_Win->ActiveItem);
     MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
     set(EE_Win->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0100;
     } /* if */
    break;
    } /* ARROW3 */

   case GP_ARROW4:
   case GP_ARROW10:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_EETL_TXTARROWLGRIGHT) mult = 10;
    get(EETL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame < EE_Win->NextKey - mult)
     {
     frame += mult;
     sprintf(str, "%ld", frame);
     set(EETL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
     SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.KeyFrame = frame;
     SplineSingleKey(2, 0);
     Set_EETL_Data(EETL_Win->ActiveItem);
     MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
     set(EE_Win->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0100;
     } /* if */
    break;
    } /* ARROW4 */

#ifdef HGJHSAJGSDJAHSDG
// Parameter Arrows - obsolete.
 Also the function GetNextKeyitem is obsolete I think
   case GP_ARROW5:
    {
    if (EE_Win->EcoItem > 0)
     {
     EE_Win->EcoItem = GetNextKeyItem(2, EE_Win->EcoItem, -1);
     set(EE_Win->LS_List, MUIA_List_Active, EE_Win->EcoItem);
     } /* if */
    break;
    } /* parameter name arrow */

   case GP_ARROW6:
    {
    if (EE_Win->EcoItem < ECOPARAMS - 1)
     {
     EE_Win->EcoItem = GetNextKeyItem(2, EE_Win->EcoItem, +1);
     set(EE_Win->LS_List, MUIA_List_Active, EE_Win->EcoItem);
     } /* if */
    break;
    } /* parameter name arrow */
#endif
   } /* switch gadget group */

} /* Handle_EETL_Window() */
