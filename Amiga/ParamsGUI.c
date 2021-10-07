/* ParamsGUI.c
** World Construction Set GUI for miscellaneous parameter functions.
** By Gary R. Huber, 1994.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

STATIC_FCN short Add_FM_Item(void);  // used locally only -> static, AF 19.7.2021
STATIC_FCN void Set_PS_Info(void); // used locally only -> static, AF 24.7.2021
STATIC_FCN void Remove_FM_Item(long item); // used locally only -> static, AF 24.7.2021
STATIC_FCN void Set_LW_Info(void); // used locally only -> static, AF 24.7.2021
STATIC_FCN short Load_FM_Win(void); // used locally only -> static, AF 24.7.2021
STATIC_FCN void Unset_FM_Item(long item); // used locally only -> static, AF 24.7.2021
STATIC_FCN short Save_FM_Win(void); // used locally only -> static, AF 24.7.2021
STATIC_FCN void Set_FM_List(short Update, short ActiveItem); // used locally only -> static, AF 24.7.2021


STATIC_FCN void Make_PS_Window(ULONG WCS_ID) // used locally only -> static, AF 24.7.2021
{
 long open;

 if (PS_Win)
  {
  DoMethod(PS_Win->ScaleWin, MUIM_Window_ToFront);
  set(PS_Win->ScaleWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((PS_Win = (struct ScaleWindow *)
	get_Memory(sizeof (struct ScaleWindow), MEMF_CLEAR)) == NULL)
   return;

 PS_Win->Group = WCS_ID - ID_PS_WINDOW;

 switch (PS_Win->Group)
  {
  case 0:
   {
   PS_Win->PSListSize = (USEDMOTIONPARAMS + 1) * (sizeof (char *));
   PS_Win->PSListIDSize = (USEDMOTIONPARAMS) * (sizeof (short));
   PS_Win->Frame = EM_Win->Frame;
   PS_Win->Item = EM_Win->MoItem;
   break;
   }
  case 1:
   { 
   PS_Win->PSListSize = (COLORPARAMS + 1) * (sizeof (char *));
   PS_Win->PSListIDSize = (COLORPARAMS) * (sizeof (short));
   PS_Win->Frame = EC_Win->Frame;
   PS_Win->Item = EC_Win->PalItem;
   break;
   }
  case 2:
   { 
   PS_Win->PSListSize = (ECOPARAMS + 1) * (sizeof (char *));
   PS_Win->PSListIDSize = (ECOPARAMS) * (sizeof (short));
   PS_Win->Frame = EE_Win->Frame;
   PS_Win->Item = EE_Win->EcoItem;
   break;
   }
  default:
   {
   Close_PS_Window(1);
   return;
   break;
   }
  } /* switch */

 if ((PS_Win->PSList = (char **)get_Memory(PS_Win->PSListSize, MEMF_CLEAR)) == NULL)
  {
  User_Message((CONST_STRPTR)"Parameters Module: Scale",
		  (CONST_STRPTR)"Out of memory!\nCan't open Scale window.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_PS_Window(1);
  return;
  } /* if out of memory */
 if ((PS_Win->PSListID = (short *)get_Memory(PS_Win->PSListIDSize, MEMF_CLEAR)) == NULL)
  {
  User_Message((CONST_STRPTR)"Parameters Module: Scale",
		  (CONST_STRPTR)"Out of memory!\nCan't open Scale window.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_PS_Window(1);
  return;
  } /* if out of memory */

 if (! Set_PS_List(PS_Win->PSList, PS_Win->PSListID, PS_Win->Group, 1, NULL))
  {
  User_Message((CONST_STRPTR)"Parameters Module: Scale",
		  (CONST_STRPTR)"No key frames to scale!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_PS_Window(1);
  return;
  } /* if out of memory */

 if ((PS_Win->AltKF = (union KeyFrame *)get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  User_Message((CONST_STRPTR)"Parameters Module: Scale",
		  (CONST_STRPTR)"Out of memory!\nCan't open Scale window.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_PS_Window(1);
  return;
  } /* if out of memory */
 memcpy(PS_Win->AltKF, KF, KFsize);
 PS_Win->AltKFsize = KFsize;
 PS_Win->AltKeyFrames = ParHdr.KeyFrames;

  Set_Param_Menu(10);

     PS_Win->ScaleWin = WindowObject,
      MUIA_Window_Title		, "Scale Keys",
      MUIA_Window_ID		, MakeID('P','S','S','C'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, Label2("Param"),
	    Child, PS_Win->ItemCycle = Cycle(PS_Win->PSList),
	    Child, Label2("Frame"),
	    Child, PS_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "0123456789",
			MUIA_String_Integer, PS_Win->Frame, End,
	    End, /* HGroup */ 
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, PS_Win->BT_Group[0] = KeyButtonObject('i'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cMotion", End, 
            Child, PS_Win->BT_Group[1] = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cColor", End, 
            Child, PS_Win->BT_Group[2] = KeyButtonObject('e'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cEcosystem", End, 
	    End, /* HGroup */
	  Child, HGroup, 
            Child, PS_Win->BT_AllFrames = KeyButtonObject('f'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, TRUE,
		 MUIA_Text_Contents, "\33cAll Frames", End, 
            Child, PS_Win->BT_OneFrame = KeyButtonObject('t'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cThis Frame", End, 
	    End, /* HGroup */
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, HGroup,
	    Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, "\33c\0334Scale", End,
	      Child, HGroup,
                Child, PS_Win->BT_FrameScale = KeyButtonObject('r'),
		 	MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, "\33cFrame(s)", End,
		Child, Label2("x"),
	        Child, PS_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "+-.0123456789",
			MUIA_String_Contents, "1.0", End,
		End, /* HGroup */
	      Child, HGroup,
                Child, PS_Win->BT_ValueScale = KeyButtonObject('v'),
		 	MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, "\33cValue(s)", End,
		Child, Label2("x"),
	        Child, PS_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "+-.0123456789",
			MUIA_String_Contents, "1.0", End,
		End, /* HGroup */
	      End, /* VGroup */
	    Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,
	    Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, "\33c\0334Shift", End,
	      Child, HGroup,
                Child, PS_Win->BT_FrameShift = KeyButtonObject('m'),
		 	MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, "\33cFrame(s)", End,
		Child, Label2("+"),
	        Child, PS_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "+-0123456789",
			MUIA_String_Contents, "0", End,
		End, /* HGroup */
	      Child, HGroup,
                Child, PS_Win->BT_ValueShift = KeyButtonObject('u'),
		 	MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, "\33cValue(s)", End,
		Child, Label2("+"),
	        Child, PS_Win->FloatStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "+-.0123456789",
			MUIA_String_Contents, "0.0", End,
		End, /* HGroup */
	      End, /* VGroup */
	    End, /* HGroup */
	  Child, HGroup,
            Child, PS_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"), 
            Child, PS_Win->BT_Operate = KeyButtonFunc('o', "\33cOperate"), 
            Child, PS_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"), 
            End, /* HGroup */
	  End, /* VGroup */

      End; /* WindowObject PS_Win->ScaleWin */

  if (! PS_Win->ScaleWin)
   {
   Close_PS_Window(1);
   User_Message((CONST_STRPTR)"Scale Keys", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, PS_Win->ScaleWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(PS_Win->ScaleWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PS_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, PS_Win->BT_Apply, ID_PS_APPLY,
   PS_Win->BT_Cancel, ID_PS_CLOSE, PS_Win->BT_Operate, ID_PS_OPERATE,
   NULL);

/* Notifications */
  DoMethod(PS_Win->BT_AllFrames, MUIM_Notify, MUIA_Selected, TRUE,
    PS_Win->BT_OneFrame, 3, MUIM_Set, MUIA_Selected, FALSE);
  DoMethod(PS_Win->BT_AllFrames, MUIM_Notify, MUIA_Selected, FALSE,
    PS_Win->BT_OneFrame, 3, MUIM_Set, MUIA_Selected, TRUE);
  DoMethod(PS_Win->BT_OneFrame, MUIM_Notify, MUIA_Selected, TRUE,
    PS_Win->BT_AllFrames, 3, MUIM_Set, MUIA_Selected, FALSE);
  DoMethod(PS_Win->BT_OneFrame, MUIM_Notify, MUIA_Selected, FALSE,
    PS_Win->BT_AllFrames, 3, MUIM_Set, MUIA_Selected, TRUE);

/* Set tab cycle chain */
  DoMethod(PS_Win->ScaleWin, MUIM_Window_SetCycleChain,
	PS_Win->ItemCycle, PS_Win->IntStr[0], PS_Win->BT_Group[0],
	PS_Win->BT_Group[1], PS_Win->BT_Group[2], PS_Win->BT_AllFrames,
	PS_Win->BT_OneFrame, PS_Win->BT_FrameScale, PS_Win->FloatStr[0],
	PS_Win->BT_ValueScale, PS_Win->FloatStr[1], PS_Win->BT_FrameShift,
	PS_Win->IntStr[1], PS_Win->BT_ValueShift, PS_Win->FloatStr[2],
	PS_Win->BT_Apply, PS_Win->BT_Operate,
	PS_Win->BT_Cancel, NULL);

/* Set return cycle chain */
  DoMethod(PS_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->ScaleWin, 3, MUIM_Set, MUIA_Window_ActiveObject, PS_Win->FloatStr[0]);
  DoMethod(PS_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->ScaleWin, 3, MUIM_Set, MUIA_Window_ActiveObject, PS_Win->FloatStr[1]);
  DoMethod(PS_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->ScaleWin, 3, MUIM_Set, MUIA_Window_ActiveObject, PS_Win->IntStr[1]);
  DoMethod(PS_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->ScaleWin, 3, MUIM_Set, MUIA_Window_ActiveObject, PS_Win->FloatStr[2]);
  DoMethod(PS_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->ScaleWin, 3, MUIM_Set, MUIA_Window_ActiveObject, PS_Win->IntStr[0]);

/* Set return cycle chain */
  DoMethod(PS_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->BT_OneFrame, 3, MUIM_Set, MUIA_Selected, TRUE);
  DoMethod(PS_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->BT_FrameScale, 3, MUIM_Set, MUIA_Selected, TRUE);
  DoMethod(PS_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->BT_ValueScale, 3, MUIM_Set, MUIA_Selected, TRUE);
  DoMethod(PS_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->BT_FrameShift, 3, MUIM_Set, MUIA_Selected, TRUE);
  DoMethod(PS_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   PS_Win->BT_ValueShift, 3, MUIM_Set, MUIA_Selected, TRUE);

/* Set active gadget */
  set(PS_Win->ScaleWin, MUIA_Window_ActiveObject, PS_Win->BT_Operate);

/* Open window */
  set(PS_Win->ScaleWin, MUIA_Window_Open, TRUE);
  get(PS_Win->ScaleWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_PS_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(PS_Win->ScaleWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_PS_ACTIVATE);

} /* Make_PS_Window() */

/************************************************************************/

void Close_PS_Window(short apply)
{
 if (PS_Win)
  {
  if (PS_Win->AltKF)
   {
   if (apply)
    {
    free_Memory(PS_Win->AltKF, PS_Win->AltKFsize);
    } /* if apply changes */
   else
    {
    MergeKeyFrames(PS_Win->AltKF, PS_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 0);
    MergeKeyFrames(PS_Win->AltKF, PS_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 1);
    MergeKeyFrames(PS_Win->AltKF, PS_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 2);
    free_Memory(PS_Win->AltKF, PS_Win->AltKFsize);
    ResetTimeLines(-1);
    if (EM_Win)
     {
     UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
     GetKeyTableValues(0, EM_Win->MoItem, 1);
     Set_EM_Item(EM_Win->MoItem);
     Set_Radial_Txt(2);
     DisableKeyButtons(0);
     }
    if (EC_Win)
     {
     UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
     GetKeyTableValues(1, EC_Win->PalItem, 1);
     Set_EC_Item(EC_Win->PalItem);
     DisableKeyButtons(1);
     }
    if (EE_Win)
     {
     UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
     GetKeyTableValues(2, EE_Win->EcoItem, 1);
     Set_EE_Item(EE_Win->EcoItem);
     DisableKeyButtons(2);
     }
    } /* else cancel */
   } /* if AltKF*/

  if (PS_Win->ScaleWin)
   {
   set(PS_Win->ScaleWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, PS_Win->ScaleWin);
   MUI_DisposeObject(PS_Win->ScaleWin);
   } /* if window created */
  if (PS_Win->PSList) free_Memory(PS_Win->PSList, PS_Win->PSListSize);
  if (PS_Win->PSListID) free_Memory(PS_Win->PSListID, PS_Win->PSListIDSize);
  free_Memory(PS_Win, sizeof (struct ScaleWindow));
  PS_Win = NULL;
  } /* if memory allocated */


} /* Close_PS_Window() */

/************************************************************************/

void Handle_PS_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_PS_Window(WCS_ID);
   return;
   } /* Open Parameter Scale Window */

  if (! PS_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Directory List window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_PS_OPERATE:
      {
      Set_PS_Info();
      ScaleKeys(&PS_Win->SKInfo);
      ResetTimeLines(-1);
      if (EM_Win)
       {
       UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
       GetKeyTableValues(0, EM_Win->MoItem, 1);
       Set_EM_Item(EM_Win->MoItem);
       Set_Radial_Txt(2);
       DisableKeyButtons(0);
       }
      if (EC_Win)
       {
       UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
       GetKeyTableValues(1, EC_Win->PalItem, 1);
       Set_EC_Item(EC_Win->PalItem);
       DisableKeyButtons(1);
       }
      if (EE_Win)
       {
       UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
       GetKeyTableValues(2, EE_Win->EcoItem, 1);
       Set_EE_Item(EE_Win->EcoItem);
       DisableKeyButtons(2);
       }
      break;
      }
     case ID_PS_APPLY:
      {
      Close_PS_Window(1);
      break;
      }
     case ID_PS_CLOSE:
      {
      Close_PS_Window(0);
      break;
      } /*  */
     case ID_PS_CLOSEQUERY:
      {
      if (KFsize != PS_Win->AltKFsize || memcmp(KF, PS_Win->AltKF, KFsize))
       Close_PS_Window(CloseWindow_Query((CONST_STRPTR)"Scale Keys"));
      else
       Close_PS_Window(1);
      break;
      }
     } /* switch button */
    } /* BUTTONS1 */
   } /* switch */

} /* Handle_PS_Window() */

/***********************************************************************/

short Set_PS_List(char **List, short *ListID, short group, short ReqKeys,
	char *TruncateText)
{
 short i = 0, j = 0, TruncTextLen = 0, lastused = 0;

 if (TruncateText)
  TruncTextLen = strlen(TruncateText);

 switch (group)
  {
  case 0:
   {
   for ( ; i<USEDMOTIONPARAMS; i++)
    {
    if (CountKeyFrames(0, i) >= ReqKeys)
     {
     List[j] = varname[i];
     if (ListID) ListID[j] = i;
     j ++;
     } /* if key frames exist */
    } /* for i=0... */
   break;
   } /* motion */
  case 1:
   {
   for ( ; i<COLORPARAMS; i++)
    {
    if (CountKeyFrames(1, i) >= ReqKeys)
     {
     List[j] = PAR_NAME_COLOR(i);
     if (ListID) ListID[j] = i;
     if (TruncateText)
      {
      if (strncmp(List[j], TruncateText, TruncTextLen))
       lastused = j;
      } /* if */
     j ++;
     } /* if key frames exist */
    } /* for i=0... */
   break;
   } /* colors */
  case 2:
   {
   for ( ; i<ECOPARAMS; i++)
    {
    if (CountKeyFrames(2, i) >= ReqKeys)
     {
     List[j] = PAR_NAME_ECO(i);
     if (ListID) ListID[j] = i;
     if (TruncateText)
      {
      if (strncmp(List[j], TruncateText, TruncTextLen))
       lastused = j;
      } /* if */
     j ++;
     } /* if key frames exist */
    } /* for i=0... */
   break;
   } /* ecosystems */
  } /* switch */

 if (TruncateText)
  {
  if (lastused < j)
   List[lastused + 1] = NULL;
  } /* if */
 List[j] = NULL;

 if (j == 0)
  return (0);
 return (1);

} /* Set_PS_List() */

/**********************************************************************/

STATIC_FCN void Set_PS_Info(void) // used locally only -> static, AF 24.7.2021
{
 long data;
 char *floatdata;

 get(PS_Win->BT_Group[0], MUIA_Selected, &data);
 PS_Win->SKInfo.ModGroup[0] = data;
 get(PS_Win->BT_Group[1], MUIA_Selected, &data);
 PS_Win->SKInfo.ModGroup[1] = data;
 get(PS_Win->BT_Group[2], MUIA_Selected, &data);
 PS_Win->SKInfo.ModGroup[2] = data;

 get(PS_Win->BT_AllFrames, MUIA_Selected, &data);
 PS_Win->SKInfo.AllFrames = data;
 get(PS_Win->BT_FrameScale, MUIA_Selected, &data);
 PS_Win->SKInfo.FrameScale = data;
 get(PS_Win->BT_ValueScale, MUIA_Selected, &data);
 PS_Win->SKInfo.ValueScale = data;
 get(PS_Win->BT_FrameShift, MUIA_Selected, &data);
 PS_Win->SKInfo.FrameShift = data;
 get(PS_Win->BT_ValueShift, MUIA_Selected, &data);
 PS_Win->SKInfo.ValueShift = data;

 get(PS_Win->FloatStr[0], MUIA_String_Contents, &floatdata);
 PS_Win->SKInfo.FSc = atof(floatdata);
 get(PS_Win->FloatStr[1], MUIA_String_Contents, &floatdata);
 PS_Win->SKInfo.VSc = atof(floatdata);
 get(PS_Win->IntStr[1], MUIA_String_Integer, &data);
 PS_Win->SKInfo.FSh = data;
 get(PS_Win->FloatStr[2], MUIA_String_Contents, &floatdata);
 PS_Win->SKInfo.VSh = atof(floatdata);
 
 PS_Win->SKInfo.Group = PS_Win->Group;
 get(PS_Win->IntStr[0], MUIA_String_Integer, &data);
 PS_Win->SKInfo.Frame = data;
 get(PS_Win->ItemCycle, MUIA_Cycle_Active, &data);
 PS_Win->SKInfo.Item = PS_Win->PSListID[data];
 
} /* Set_PS_Info() */

/***********************************************************************/

void Make_LW_Window(void)
{
 short i;
 long open;
 static const char *LW_ExportItems[] = {"Scene", "Scene + DEMs", "DEM Only", "Motion Only", NULL};

 if (LW_Win)
  {
  DoMethod(LW_Win->IOWin, MUIM_Window_ToFront);
  set(LW_Win->IOWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message((CONST_STRPTR)"LightWave Motion I/O",
		  (CONST_STRPTR)"You must first load or create a parameter file before using this feature.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((LW_Win = (struct LightWaveIOWindow *)
	get_Memory(sizeof (struct LightWaveIOWindow), MEMF_CLEAR)) == NULL)
   return;

/* set preliminary values */

 if (! BuildKeyTable())
  {
  Close_LW_Window();
  User_Message((CONST_STRPTR)"LightWave Motion I/O", (CONST_STRPTR)"Error building motion value table\nOperation terminated",
		  (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no key table = out of memory */

/* find central lat & lon */

 if (! settings.lookahead)
  {
  if (KT[4].Key)
   {
   for (i=1; i<=KT_MaxFrames; i++)
    {
    LW_Win->LWInfo.RefLat += KT[4].Val[0][i];
    } /* for i=0... */
   LW_Win->LWInfo.RefLat /= KT_MaxFrames;
   }
  else
   LW_Win->LWInfo.RefLat = PAR_FIRST_MOTION(4);

  if (KT[5].Key)
   {
   for (i=1; i<=KT_MaxFrames; i++)
    {
    LW_Win->LWInfo.RefLon += KT[5].Val[0][i];
    } /* for i=0... */
   LW_Win->LWInfo.RefLon /= KT_MaxFrames;
   }
  else
   LW_Win->LWInfo.RefLon = PAR_FIRST_MOTION(5);
  } /* if not look ahead */
 else
  {
  if (KT[1].Key)
   {
   for (i=1; i<=KT_MaxFrames; i++)
    {
    LW_Win->LWInfo.RefLat += KT[1].Val[0][i];
    } /* for i=0... */
   LW_Win->LWInfo.RefLat /= KT_MaxFrames;
   }
  else
   LW_Win->LWInfo.RefLat = PAR_FIRST_MOTION(1);

  if (KT[2].Key)
   {
   for (i=1; i<=KT_MaxFrames; i++)
    {
    LW_Win->LWInfo.RefLon += KT[2].Val[0][i];
    } /* for i=0... */
   LW_Win->LWInfo.RefLon /= KT_MaxFrames;
   }
  else
   LW_Win->LWInfo.RefLon = PAR_FIRST_MOTION(2);
  } /* else lookahead - use camera positions */

 FreeKeyTable();

  Set_Param_Menu(10);

     LW_Win->IOWin = WindowObject,
      MUIA_Window_Title		, "LightWave Export",
      MUIA_Window_ID		, MakeID('L','W','M','O'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, VGroup,
	      Child, Label("\33c\0334Export Entity"),
	      Child, LW_Win->ExportItem = RadioObject,
		MUIA_Radio_Entries, LW_ExportItems, End,
	      End, /* VGroup */
	    Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,
	    Child, VGroup,
	      Child, HGroup,
	        Child, Label2("Max Polygons"),
	        Child, LW_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "0123456789", End,
		End, /* HGroup */
	      Child, HGroup,
	        Child, Label2("Max Vertices"),
	        Child, LW_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "0123456789", End,
		End, /* HGroup */
              Child, LW_Win->BT_Bathymetry = KeyButtonObject('b'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cBathymetry", End, 
	      End, /* VGroup */
	    End, /* HGroup */
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, HGroup,
	    Child, Label2("Ref Lat"),
	    Child, LW_Win->FloatStr[0] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789", End,
	    Child, Label2("Lon"),
	    Child, LW_Win->FloatStr[1] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789", End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, RectangleObject, End,
            Child, LW_Win->BT_RotateHorizontal = KeyButtonObject('h'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cRotate to Horizontal", End, 
	    Child, RectangleObject, End,
	    End, /* HGroup */
          Child, LW_Win->BT_Export = KeyButtonFunc('e', "\33cExport"), 
	  End, /* VGroup */

      End; /* WindowObject LW_Win->IOWin */

  if (! LW_Win->IOWin)
   {
   Close_LW_Window();
   User_Message((CONST_STRPTR)"LightWave I/O", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, LW_Win->IOWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(LW_Win->IOWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_LW_CLOSE);  

  MUI_DoNotiPresFal(app, LW_Win->BT_Export, ID_LW_EXPORT, NULL);

/* Set tab cycle chain */
  DoMethod(LW_Win->IOWin, MUIM_Window_SetCycleChain,
	LW_Win->ExportItem,
	LW_Win->IntStr[0], LW_Win->IntStr[1], LW_Win->BT_Bathymetry,
	LW_Win->FloatStr[0], LW_Win->FloatStr[1], LW_Win->BT_RotateHorizontal,
	LW_Win->BT_Export, NULL);

/* Set return cycle chain */
  DoMethod(LW_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   LW_Win->IOWin, 3, MUIM_Set, MUIA_Window_ActiveObject, LW_Win->IntStr[1]);
  DoMethod(LW_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   LW_Win->IOWin, 3, MUIM_Set, MUIA_Window_ActiveObject, LW_Win->FloatStr[0]);
  DoMethod(LW_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   LW_Win->IOWin, 3, MUIM_Set, MUIA_Window_ActiveObject, LW_Win->FloatStr[1]);
  DoMethod(LW_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   LW_Win->IOWin, 3, MUIM_Set, MUIA_Window_ActiveObject, LW_Win->IntStr[0]);

/* set reference string contents */
  setfloat(LW_Win->FloatStr[0], LW_Win->LWInfo.RefLat);
  setfloat(LW_Win->FloatStr[1], LW_Win->LWInfo.RefLon);
  set(LW_Win->IntStr[0], MUIA_String_Integer, 60000);
  set(LW_Win->IntStr[1], MUIA_String_Integer, 32767);


/* Open window */
  set(LW_Win->IOWin, MUIA_Window_Open, TRUE);
  get(LW_Win->IOWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_LW_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(LW_Win->IOWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_LW_ACTIVATE);

/* Get Window structure pointer */
  get(LW_Win->IOWin, MUIA_Window_Window, &LW_Win->Win);

} /* Make_LW_Window() */

/***********************************************************************/

void Close_LW_Window(void)
{
 if (LW_Win)
  {
  if (LW_Win->IOWin)
   {
   set(LW_Win->IOWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, LW_Win->IOWin);
   MUI_DisposeObject(LW_Win->IOWin);
   } /* if window created */
  free_Memory(LW_Win, sizeof (struct LightWaveIOWindow));
  LW_Win = NULL;
  } /* if memory allocated */

} /* Close_LW_Window() */

/************************************************************************/

void Handle_LW_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_LW_Window();
   return;
   } /* Open LightWave I/O Window */

  if (! LW_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Directory List window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_LW_EXPORT:
      {
      SetPointer(LW_Win->Win, WaitPointer, 16, 16, -6, 0);

      Set_LW_Info();
      switch (LW_Win->LWInfo.ExportItem)
       {
       case 0:
        {
        LW_Win->LWInfo.SaveDEMs = 0;
        LWScene_Export(&LW_Win->LWInfo);
        break;
	} /* scene */
       case 1:
        {
        LW_Win->LWInfo.SaveDEMs = 1;
        LWScene_Export(&LW_Win->LWInfo);
        break;
	} /* DEM */
       case 2:
        {
        LWOB_Export(NULL, NULL, NULL, LW_Win->LWInfo.MaxVerts,
		LW_Win->LWInfo.MaxPolys, 1, LW_Win->LWInfo.Bathymetry,
		(LW_Win->LWInfo.RotateHorizontal ? -LW_Win->LWInfo.RefLon: 0.0));
        break;
	} /* DEM */
       case 3:
        {
        ExportWave(&LW_Win->LWInfo, NULL);
        break;
	} /* motion */
       } /* switch */
      ClearPointer(LW_Win->Win);
      break;
      }
     case ID_LW_CLOSE:
      {
      Close_LW_Window();
      break;
      } /*  */
     } /* switch button */
    break;
    } /* BUTTONS1 */

   } /* switch */

} /* Handle_LW_Window() */

/***********************************************************************/

STATIC_FCN void Set_LW_Info(void) // used locally only -> static, AF 24.7.2021
{
 char *floatdata;
long data;

 get(LW_Win->FloatStr[0], MUIA_String_Contents, &floatdata);
 LW_Win->LWInfo.RefLat = atof(floatdata);
 get(LW_Win->FloatStr[1], MUIA_String_Contents, &floatdata);
 LW_Win->LWInfo.RefLon = atof(floatdata);
 get(LW_Win->IntStr[0], MUIA_String_Integer, &data);
 LW_Win->LWInfo.MaxPolys = data;
 get(LW_Win->IntStr[1], MUIA_String_Integer, &data);
 LW_Win->LWInfo.MaxVerts = data;
 get(LW_Win->ExportItem, MUIA_Radio_Active, &data);
 LW_Win->LWInfo.ExportItem = data;
 get(LW_Win->BT_RotateHorizontal, MUIA_Selected, &data);
 LW_Win->LWInfo.RotateHorizontal = data;
 get(LW_Win->BT_Bathymetry, MUIA_Selected, &data);
 LW_Win->LWInfo.Bathymetry = data;

} /* Set_LW_Info() */

/************************************************************************/

void Make_FM_Window(void)
{
 long open;
 char *Name;

 if (FM_Win)
  {
  DoMethod(FM_Win->ModelWin, MUIM_Window_ToFront);
  set(FM_Win->ModelWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((FM_Win = (struct ForestModelWindow *)
	get_Memory(sizeof (struct ForestModelWindow), MEMF_CLEAR)) == NULL)
   return;

 FM_Win->MaxItems = 100;
 FM_Win->ItemListSize = FM_Win->MaxItems * (sizeof (char *));
 if ((FM_Win->Item = (char **)get_Memory(FM_Win->ItemListSize, MEMF_CLEAR)) == NULL)
  {
  User_Message((CONST_STRPTR)"Parameters Module: Model",
		  (CONST_STRPTR)"Out of memory!\nCan't open model design window.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_FM_Window();
  return;
  } /* if out of memory */


  Set_Param_Menu(10);

     FM_Win->ModelWin = WindowObject,
      MUIA_Window_Title		, "Ecosystem Model Editor",
      MUIA_Window_ID		, MakeID('F','O','M','O'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, Label2("Name "),
	    Child, FM_Win->NameStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012", End,
            Child, FM_Win->BT_Add = KeyButtonFunc('a', "\33cAdd"), 
            Child, FM_Win->BT_Remove = KeyButtonFunc('r', "\33cRemove"), 
            Child, FM_Win->BT_Load = KeyButtonFunc('l', "\33cLoad"), 
            Child, FM_Win->BT_Save = KeyButtonFunc('s', "\33cSave"), 
	    End, /* HGroup */
	  Child, FM_Win->LS_List = ListviewObject,
	      MUIA_Listview_Input, TRUE,
              MUIA_Listview_List, ListObject, ReadListFrame, End,
	    End,
	  Child, HGroup,
	    Child, Label2("Ht (m)"),
	    Child, FM_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Items"),
	    Child, FM_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Class"),
	    Child, FM_Win->ClassCycle = CycleObject,
			MUIA_Cycle_Entries, typename,
			MUIA_Cycle_Active, 4, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2("Red"),
	    Child, FM_Win->IntStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Grn"),
	    Child, FM_Win->IntStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Blu"),
	    Child, FM_Win->IntStr[4] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123",
			MUIA_String_Accept, "0123456789", End,
	    End, /* HGroup */ 
	  End, /* VGroup */
	End; /* Window object */

  if (! FM_Win->ModelWin)
   {
   Close_FM_Window();
   User_Message((CONST_STRPTR)"Parameters Module: Model", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, FM_Win->ModelWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(FM_Win->ModelWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_FM_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, FM_Win->BT_Add, ID_FM_ADD,
   FM_Win->BT_Remove, ID_FM_REMOVE, FM_Win->BT_Load, ID_FM_LOAD,
   FM_Win->BT_Save, ID_FM_SAVE,
   NULL);

/* Set tab cycle chain */
  DoMethod(FM_Win->ModelWin, MUIM_Window_SetCycleChain,
	FM_Win->NameStr,
	FM_Win->BT_Add,	FM_Win->BT_Remove, FM_Win->BT_Load, FM_Win->BT_Save,
	FM_Win->LS_List, FM_Win->IntStr[0], FM_Win->IntStr[1],
	FM_Win->ClassCycle, FM_Win->IntStr[2],
	FM_Win->IntStr[3], FM_Win->IntStr[4], NULL);

/* return cycle */
  DoMethod(FM_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->IntStr[1]);
  DoMethod(FM_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->IntStr[2]);
  DoMethod(FM_Win->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->IntStr[3]);
  DoMethod(FM_Win->IntStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->IntStr[4]);
  DoMethod(FM_Win->IntStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->NameStr);
  DoMethod(FM_Win->NameStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	FM_Win->ModelWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, FM_Win->IntStr[0]);

/* Set active gadget */
  set(FM_Win->ModelWin, MUIA_Window_ActiveObject, FM_Win->IntStr[0]);

  get(EE_Win->ModelStr, MUIA_String_Contents, &Name);
  set(FM_Win->NameStr, MUIA_String_Contents, Name);

/* Create directory list */
  strcpy(&FM_Win->ItemStr[0][0],
	 "\0338    Ht Items Class   Red   Grn   Blu");
  FM_Win->ItemNum = 1;
  Set_FM_List(0, 0);

/* link list to application */
  DoMethod(FM_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_FM_LIST);

/* Open window */
  set(FM_Win->ModelWin, MUIA_Window_Open, TRUE);
  get(FM_Win->ModelWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_FM_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(FM_Win->ModelWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_FM_ACTIVATE);

} /* Make_FM_Window() */

/*********************************************************************/

void Close_FM_Window(void)
{
short SaveOld;

 if (FM_Win)
  {
  if (FM_Win->ModelWin)
   {
   if (FM_Win->Mod)
    {
    if ((SaveOld = User_Message_Def((CONST_STRPTR)"Parameters Module: Model",
    		(CONST_STRPTR)"The current Ecosystem Model has been modified. Do you wish to save it before closing?",
			(CONST_STRPTR)"Yes|No|Cancel", (CONST_STRPTR)"ync", 1)) == 0)
     return;
    else if (SaveOld == 1)
     Save_FM_Win();
    } /* if old model modified */
   set(FM_Win->ModelWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, FM_Win->ModelWin);
   MUI_DisposeObject(FM_Win->ModelWin);
   } /* if window created */
  if (FM_Win->Item) free_Memory(FM_Win->Item, FM_Win->ItemListSize);
  free_Memory(FM_Win, sizeof (struct ForestModelWindow));
  FM_Win = NULL;
  } /* if memory allocated */

} /* Close_FM_Window() */

/*********************************************************************/

void Handle_FM_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_FM_Window();
   return;
   } /* Open Directory List Window */

  if (! FM_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Directory List window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_FM_ADD:
      {
      Add_FM_Item();
      break;
      }
     case ID_FM_REMOVE:
      {
      long Item;

      get(FM_Win->LS_List, MUIA_List_Active, &Item);
      Remove_FM_Item(Item);
      break;
      }
     case ID_FM_LOAD:
      {
      Load_FM_Win();
      break;
      }
     case ID_FM_SAVE:
      {
      Save_FM_Win();
      break;
      }
     case ID_FM_CLOSEQUERY:
      {
      Close_FM_Window();
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_LIST1:
    {
    long item;

    get(FM_Win->LS_List, MUIA_List_Active, &item);
    Unset_FM_Item(item);
    break;
    } /* LIST1 */

   } /* switch gadget group */

} /* Handle_FM_Window() */

/***********************************************************************/

STATIC_FCN void Set_FM_List(short Update, short ActiveItem) // used locally only -> static, AF 24.7.2021
{
 short i;

 for (i=0; i<FM_Win->ItemNum && i<FM_Win->MaxItems-1; i++)
  {
  FM_Win->Item[i] = &FM_Win->ItemStr[i][0];
  } /* for i=0... */
 FM_Win->Item[i] = NULL;

/* Add items or update directory list */
 if (Update)
  {
  DoMethod(FM_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(FM_Win->LS_List, MUIM_List_Clear);
  DoMethod(FM_Win->LS_List,
	MUIM_List_Insert, FM_Win->Item, -1, MUIV_List_Insert_Bottom);
  }

 set(FM_Win->LS_List, MUIA_List_Active, ActiveItem);

} /* Set_FM_List() */

/************************************************************************/

STATIC_FCN short Add_FM_Item(void)  // used locally only -> static, AF 19.7.2021
{
 long i, Ht, Stems, Red, Grn, Blu, OldHt, MoveItem, Class;

 if (FM_Win->ItemNum >= FM_Win->MaxItems - 1)
  return (0);

 get(FM_Win->IntStr[0], MUIA_String_Integer, &Ht);
 get(FM_Win->IntStr[1], MUIA_String_Integer, &Stems);
 get(FM_Win->IntStr[2], MUIA_String_Integer, &Red);
 get(FM_Win->IntStr[3], MUIA_String_Integer, &Grn);
 get(FM_Win->IntStr[4], MUIA_String_Integer, &Blu);
 get(FM_Win->ClassCycle, MUIA_Cycle_Active, &Class);
 if (Ht > 99999)
  Ht = 99999;
 if (Stems > 99999)
  Stems = 99999;
 if (Red > 255)
  Red = 255;
 if (Grn > 255)
  Grn = 255;
 if (Blu > 255)
  Blu = 255;

 MoveItem = FM_Win->ItemNum;
 for (i=1; i<FM_Win->ItemNum; i++)
  {
  OldHt = atoi(&FM_Win->ItemStr[i][0]);
  if (Ht < OldHt)
   {
   MoveItem = i;
   break;
   } /* if */
  } /* for i=1... */

 for (i=FM_Win->ItemNum-1; i>=MoveItem; i--)
  {
  strcpy(&FM_Win->ItemStr[i + 1][0], &FM_Win->ItemStr[i][0]);
  }

 switch (Class)
  {
  case 0:
   {
   strcpy(str, " Water");
   break;
   }
  case 1:
   {
   strcpy(str, "  Snow");
   break;
   }
  case 2:
   {
   strcpy(str, "  Rock");
   break;
   }
  case 3:
   {
   strcpy(str, "  Bare");
   break;
   }
  case 4:
   {
   strcpy(str, " Conif");
   break;
   }
  case 5:
   {
   strcpy(str, " Decid");
   break;
   }
  case 6:
   {
   strcpy(str, " LowVg");
   break;
   }
  case 7:
   {
   strcpy(str, "  Snag");
   break;
   }
  case 8:
   {
   strcpy(str, " Stump");
   break;
   }

  } /* switch */
  
 sprintf(&FM_Win->ItemStr[MoveItem][0],
	 "%6ld%6ld%s%6ld%6ld%6ld", Ht, Stems, str, Red, Grn, Blu);
 FM_Win->ItemNum ++;

 Set_FM_List(0, MoveItem);
 FM_Win->Mod = 1;

 return (1);

} /* Add_FM_Item() */

/************************************************************************/

STATIC_FCN void Remove_FM_Item(long item) // used locally only -> static, AF 24.7.2021
{

 for ( ; item<FM_Win->ItemNum; item++)
  strcpy(&FM_Win->ItemStr[item][0], &FM_Win->ItemStr[item + 1][0]);

 FM_Win->ItemNum --;

 FM_Win->Item[FM_Win->ItemNum] = NULL;

 Set_FM_List(0, item);
 FM_Win->Mod = 1;

} /* Remove_FM_Item() */

/************************************************************************/

STATIC_FCN void Unset_FM_Item(long item) // used locally only -> static, AF 24.7.2021
{
 long Ht, Stems, Red, Grn, Blu, Class;

 if (item < 1)
  return;

 Ht = atoi(&FM_Win->ItemStr[item][0]);
 Stems = atoi(&FM_Win->ItemStr[item][6]);

 if (! strncmp(&FM_Win->ItemStr[item][12], " Water", 6))
  Class = 0;
 else if (! strncmp(&FM_Win->ItemStr[item][12], "  Snow", 6))
  Class = 1;
 else if (! strncmp(&FM_Win->ItemStr[item][12], "  Rock", 6))
  Class = 2;
 else if (! strncmp(&FM_Win->ItemStr[item][12], " Strat", 6))
  Class = 2;	/* probably not used anywhere since this was only briefly an option */
 else if (! strncmp(&FM_Win->ItemStr[item][12], "  Bare", 6))
  Class = 4;
 else if (! strncmp(&FM_Win->ItemStr[item][12], " Conif", 6))
  Class = 5;
 else if (! strncmp(&FM_Win->ItemStr[item][12], " Decid", 6))
  Class = 6;
 else if (! strncmp(&FM_Win->ItemStr[item][12], " LowVg", 6))
  Class = 7;
 else if (! strncmp(&FM_Win->ItemStr[item][12], "  Snag", 6))
  Class = 8;
 else
  Class = 9;

 Red = atoi(&FM_Win->ItemStr[item][18]);
 Grn = atoi(&FM_Win->ItemStr[item][24]);
 Blu = atoi(&FM_Win->ItemStr[item][30]);

 set(FM_Win->IntStr[0], MUIA_String_Integer, Ht);
 set(FM_Win->IntStr[1], MUIA_String_Integer, Stems);
 set(FM_Win->IntStr[2], MUIA_String_Integer, Red);
 set(FM_Win->IntStr[3], MUIA_String_Integer, Grn);
 set(FM_Win->IntStr[4], MUIA_String_Integer, Blu);
 set(FM_Win->ClassCycle, MUIA_Cycle_Active, Class);

} /* Unset_FM_Item() */

/************************************************************************/

STATIC_FCN short Load_FM_Win(void) // used locally only -> static, AF 24.7.2021
{
 short error = 0, Items, Version, SaveOld;
 long Ht, Stems, Red, Grn, Blu;
 char filename[256], name[32], *nameptr, IDStr[32], Class[12];
 FILE *fModel;

 if (FM_Win->Mod)
  {
  if ((SaveOld = User_Message_Def((CONST_STRPTR)"Parameters Module: Model",
		  (CONST_STRPTR)"The current Ecosystem Model has been modified. Do you wish to save it before proceeding?",
		  (CONST_STRPTR)"Yes|No|Cancel", (CONST_STRPTR)"ync", 1)) == 0)
   return (0);
  else if (SaveOld == 1)
   Save_FM_Win();
  } /* if old model modified */

 get(FM_Win->NameStr, MUIA_String_Contents, &nameptr);
 strcpy(name, nameptr);

 if (! getfilename(0, "Load Model Path/Name", modelpath, name))
  return (0);

 if (! name[0])
  {
  error = 5;
  goto EndLoad;
  }
 strmfp(filename, modelpath, name);
 if ((fModel = fopen(filename, "r")) == NULL)
  {
  error = 1;
  goto EndLoad;
  }
 fscanf(fModel, "%s", &IDStr);
 if (strcmp(IDStr, "WCSModel"))
  {
  error = 3;
  fclose(fModel);
  goto EndLoad;
  } /* if wrong type */

 fscanf(fModel, "%hd", &Version);
 if (Version != 1)
  {
  error = 4;
  fclose(fModel);
  goto EndLoad;
  } /* wrong version */

 fscanf(fModel, "%hd", &Items);
 for (FM_Win->ItemNum=1; FM_Win->ItemNum<=Items; FM_Win->ItemNum++)
  {
  if (fscanf(fModel, "%ld%ld%s%ld%ld%ld", &Ht, &Stems, &Class, &Red, &Grn, &Blu)
	!= 6)
   {
   error = 2;
   break;
   } /* if write error */

  if (! strcmp(Class, "Water"))
   strcpy(str, " Water");
  else if (! strcmp(Class, "Snow"))
   strcpy(str, "  Snow");
  else if (! strcmp(Class, "Rock"))
   strcpy(str, "  Rock");
  else if (! strcmp(Class, "Strat"))
   strcpy(str, " Strat");
  else if (! strcmp(Class, "Bare"))
   strcpy(str, "  Bare");
  else if (! strcmp(Class, "Conif"))
   strcpy(str, " Conif");
  else if (! strcmp(Class, "Decid"))
   strcpy(str, " Decid");
  else if (! strcmp(Class, "LowVg"))
   strcpy(str, " LowVg");
  else if (! strcmp(Class, "Snag"))
   strcpy(str, "  Snag");
  else
   strcpy(str, " Stump");
  
  sprintf(&FM_Win->ItemStr[FM_Win->ItemNum][0],
	 "%6ld%6ld%s%6ld%6ld%6ld", Ht, Stems, str, Red, Grn, Blu);

  } /* for FM_Win->ItemNum=1... */
 fclose(fModel);

 Set_FM_List(0, 1);
 FM_Win->Mod = 0;

EndLoad:

 set(FM_Win->NameStr, MUIA_String_Contents, name);
 if (EE_Win)
  set(EE_Win->ModelStr, MUIA_String_Contents, name);

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Error opening Ecosystem Model file for output!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)name);
   break;
   } /* open fail */
  case 2:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Error writing to Ecosystem Model file!\nOperation terminated prematurely.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRITE_FAIL, (CONST_STRPTR)name);
   break;
   } /* write fail */
  case 3:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Not a WCS Ecosystem Model file!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_TYPE, (CONST_STRPTR)name);
   break;
   } /* wrong type */
  case 4:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Unsupported WCS Ecosystem Model file version!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_VER, (CONST_STRPTR)name);
   break;
   } /* wrong version */
  case 5:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"You have not selected a file name for input!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* no name */
  } /* switch */

 return ((short)(! error));
 

} /* Load_FM_Win() */

/************************************************************************/

STATIC_FCN short Save_FM_Win(void) // used locally only -> static, AF 24.7.2021
{
 short i, error = 0, Version = 1;
 char filename[256], name[32], *nameptr;
 FILE *fModel;

 get(FM_Win->NameStr, MUIA_String_Contents, &nameptr);
 strcpy(name, nameptr);

 if (! getfilename(1, "Save Model Path/Name", modelpath, name))
  return (0);

 if (! name[0])
  {
  error = 5;
  goto EndSave;
  }
 strmfp(filename, modelpath, name);
 if ((fModel = fopen(filename, "w")) == NULL)
  {
  error = 1;
  goto EndSave;
  }
 fprintf(fModel, "%s\n", "WCSModel");
 fprintf(fModel, "%1d\n", Version);
 fprintf(fModel, "%1d\n", FM_Win->ItemNum - 1);
 for (i=1; i<FM_Win->ItemNum; i++)
  {
  if (fprintf(fModel, "%s\n", &FM_Win->ItemStr[i][0]) < 0)
   {
   error = 2;
   break;
   } /* if write error */
  } /* for i=1... */
 fclose(fModel);

EndSave:

 set(FM_Win->NameStr, MUIA_String_Contents, name);
 if (EE_Win)
  set(EE_Win->ModelStr, MUIA_String_Contents, name);

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Error opening Ecosystem Model file for output!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)name);
   break;
   } /* open fail */
  case 2:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"Error writing to Ecosystem Model file!\nOperation terminated prematurely.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)name);
   break;
   } /* read fail */
  case 5:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
		   (CONST_STRPTR)"You have not selected a file name for output!\nOperation terminated.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* no name */
  } /* switch */

 if (! error)
  FM_Win->Mod = 0;
 return ((short)(! error));
 
} /* Save_FM_Win() */

/***********************************************************************/
/* Anim window has controls for creating wire-frame animation frames
and saving them to disk, also for sizing the Camera View window and
giving it the correct aspect ratio based on Image width and height
specified in Settings Editor.
*/
void Make_AN_Window(void)
{
 long open;

 if (AN_Win)
  {
  DoMethod(AN_Win->AnimWin, MUIM_Window_ToFront);
  set(AN_Win->AnimWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((AN_Win = (struct AnimWindow *)
	get_Memory(sizeof (struct AnimWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     AN_Win->AnimWin = WindowObject,
      MUIA_Window_Title		, "Anim Control Window",
      MUIA_Window_ID		, MakeID('A','N','I','M'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
/* save path and disk icon */
	  Child, HGroup,
	    Child, Label2("Anim Path"),
	    Child, AN_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345",
		MUIA_String_Contents, graphpath, End,
	    Child, AN_Win->BT_GetPath = ImageButton(MUII_Disk),
	    End, /* HGroup */
/* save name */
	  Child, HGroup,
	    Child, Label2("Anim Name"),
	    Child, AN_Win->Str[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345",
		MUIA_String_Contents, graphname, End,
	    End, /* HGroup */
/* frame start, end strings */
	  Child, HGroup,
	    Child, Label2("Frame"),
	    Child, AN_Win->IntStr[0] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_String_Integer, IA_AnimStart,
		MUIA_FixWidthTxt, "01234", End,
	    Child, Label2("To"),
	    Child, AN_Win->IntStr[1] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_String_Integer, (IA_AnimEnd ? IA_AnimEnd: KT_MaxFrames),
		MUIA_FixWidthTxt, "01234", End,
	    Child, Label2("By"),
	    Child, AN_Win->IntStr[4] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_String_Integer, IA_AnimStep ? IA_AnimStep: abs(settings.stepframes),
		MUIA_FixWidthTxt, "01234", End,
	    End, /* HGroup */
/* window width, height strings */
	  Child, HGroup,
	    Child, Label2("Window Width"),
	    Child, AN_Win->IntStr[2] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "01234",
		MUIA_String_Integer, InterWind0->Width, End,
	    Child, Label2(" Height"),
	    Child, AN_Win->IntStr[3] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "01234",
		MUIA_String_Integer, InterWind0->Height, End,
	    End, /* HGroup */
/* aspect check box */
	  Child, HGroup,
	    Child, AN_Win->AspectCheck = CheckMark(FALSE),
	    Child, Label2("Use Render Image Aspect"),
	    End, /* HGroup */
/* render only, render & save */
	  Child, HGroup,
	    Child, AN_Win->BT_Render = KeyButtonFunc('r', "\33cRender Only"), 
	    Child, AN_Win->BT_Save   = KeyButtonFunc('s', "\33cRender & Save"), 
	    End, /* HGroup */
	  End, /* VGroup */
	End; /* Window object */

  if (! AN_Win->AnimWin)
   {
   Close_AN_Window();
   User_Message((CONST_STRPTR)"Parameters Module: Anim", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, AN_Win->AnimWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(AN_Win->AnimWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_AN_CLOSE);  

  MUI_DoNotiPresFal(app, AN_Win->BT_GetPath, ID_AN_GETPATH,
   AN_Win->BT_Render, ID_AN_RENDER, AN_Win->BT_Save, ID_AN_SAVE, NULL);

/* Set tab cycle chain */
  DoMethod(AN_Win->AnimWin, MUIM_Window_SetCycleChain,
	AN_Win->Str[0],	AN_Win->BT_GetPath, AN_Win->Str[1], AN_Win->IntStr[0],
	AN_Win->IntStr[1], AN_Win->IntStr[2], AN_Win->IntStr[3],
	AN_Win->AspectCheck, AN_Win->BT_Render, AN_Win->BT_Save, NULL);

/* return cycle */
  DoMethod(AN_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->Str[1]);
  DoMethod(AN_Win->Str[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->IntStr[0]);
  DoMethod(AN_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->IntStr[1]);
  DoMethod(AN_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->IntStr[4]);
  DoMethod(AN_Win->IntStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->IntStr[2]);
  DoMethod(AN_Win->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->IntStr[3]);
  DoMethod(AN_Win->IntStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	AN_Win->AnimWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, AN_Win->Str[0]);

/* Set active gadget */
  set(AN_Win->AnimWin, MUIA_Window_ActiveObject, AN_Win->IntStr[0]);

/* Open window */
  set(AN_Win->AnimWin, MUIA_Window_Open, TRUE);
  get(AN_Win->AnimWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_AN_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(AN_Win->AnimWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_AN_ACTIVATE);

} /* Make_AN_Window() */

/***********************************************************************/

void Close_AN_Window(void)
{
 if (AN_Win)
  {
  if (AN_Win->AnimWin)
   {
   set(AN_Win->AnimWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, AN_Win->AnimWin);
   MUI_DisposeObject(AN_Win->AnimWin);
   } /* if window created */
  free_Memory(AN_Win, sizeof (struct AnimWindow));
  AN_Win = NULL;
  } /* if memory allocated */

} /* Close_AN_Window() */

/***********************************************************************/

void Handle_AN_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_AN_Window();
   return;
   } /* Open Directory List Window */

  if (! AN_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Directory List window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_AN_GETPATH:
      {
      if (getfilename(1, "Anim Frames Path/Name", graphpath, graphname))
       {
       set(AN_Win->Str[0], MUIA_String_Contents, graphpath);
       set(AN_Win->Str[1], MUIA_String_Contents, graphname);
       } /* if */
      break;
      }
     case ID_AN_RENDER:
      {
      Init_Anim(0);
      break;
      }
     case ID_AN_SAVE:
      {
      Init_Anim(1);
      break;
      }
     case ID_AN_CLOSE:
      {
      Close_AN_Window();
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   } /* switch gadget group */

} /* Handle_AN_Window() */

/***********************************************************************/

void Init_Anim(short SaveAnim)
{
 char *name;
 long UseAspect, Reposition = 0, NewXPos, NewYPos;
 struct RenderAnim RA;

 get(AN_Win->Str[0], MUIA_String_Contents, &name);
 strcpy(RA.AnimPath, name);
 get(AN_Win->Str[1], MUIA_String_Contents, &name);
 strcpy(RA.AnimName, name);
 get(AN_Win->AspectCheck, MUIA_Selected, &UseAspect);
 get(AN_Win->IntStr[0], MUIA_String_Integer, &RA.StartFrame);
 get(AN_Win->IntStr[1], MUIA_String_Integer, &RA.EndFrame);
 get(AN_Win->IntStr[2], MUIA_String_Integer, &RA.Width);
 get(AN_Win->IntStr[3], MUIA_String_Integer, &RA.Height);
 get(AN_Win->IntStr[4], MUIA_String_Integer, &RA.FrameStep);
 RA.OutToFile = SaveAnim;
 IA_AnimStart = RA.StartFrame;
 IA_AnimEnd = RA.EndFrame;
 IA_AnimStep = RA.FrameStep;

/* check to see that width specified does not exceed screen width */
 if (RA.Width > WCSScrn->Width)
  {
  if (User_Message((CONST_STRPTR)"Parameters Module: Anim",
		  (CONST_STRPTR)"Specified width is larger than the current screen width.\
 Do you wish to use the screen width?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
   {
   RA.Width = WCSScrn->Width;
   set(AN_Win->IntStr[2], MUIA_String_Integer, RA.Width);
   } /* if */
  else
   return;
  } /* if */

/* compute correct height for given width */
 if (UseAspect)
  {
  RA.Height = ((float)settings.scrnheight / settings.scrnwidth) * RA.Width;
  } /* if */ 

 if (RA.Height > WCSScrn->Height)
  {
  if (User_Message((CONST_STRPTR)"Parameters Module: Anim",
		  (CONST_STRPTR)"Specified or computed height is larger than the current screen height.\
 Do you wish to use the screen height?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
   {
   RA.Height = WCSScrn->Height;
   set(AN_Win->IntStr[3], MUIA_String_Integer, RA.Height);
   } /* if */
  else
   return;
  } /* if */

 NewXPos = InterWind0->LeftEdge;
 NewYPos = InterWind0->TopEdge; 
 if (RA.Width != InterWind0->Width)
  {
  NewXPos = (WCSScrn->Width - RA.Width) / 2;
  Reposition = 1;
  }
 if (RA.Height != InterWind0->Height)
  {
  NewYPos = (WCSScrn->Height - RA.Height) / 2;
  Reposition = 1;
  }
 if (Reposition)
  {
  SetAPen(InterWind0->RPort, 1);
  ChangeWindowBox(InterWind0, NewXPos, NewYPos, RA.Width, RA.Height);
  Delay(25);	/* ChangeWindowBox() appears to be asynchronous */
  Init_IA_View(2);
  } /* if */

 Play_Motion(&RA);

} /* Init_Anim() */
