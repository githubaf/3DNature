/* GenericTimeLineSupport.c
** World Construction Set GUI support functions for Time Line Editing modules.
** By Gary R. Huber, 1994.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "TimeLinesGUI.h"
#include "GenericParams.h"

#include <SDI_compiler.h>

/**************************************************************************/

__saveds ULONG GNTL_HandleInput(struct IClass *cl, Object *obj,
	 struct MUIP_HandleInput *msg)
{
 #define _between(a,x,b) ((x)>=(a) && (x)<=(b))
 #define _isinobject(x,y) (_between(data->left,(x),data->right) \
		&& _between(data->top,(y),data->bottom))

 short frame, i, j, found = 0;
 float lr_offset, tb_offset, floatvalue;
 struct Data *data = INST_DATA(cl, obj);

/* Mouse events */
 if (msg->imsg)
  {
  switch (msg->imsg->Class)
   {
   case IDCMP_MOUSEBUTTONS:
    {
    if (msg->imsg->Code == SELECTDOWN)
     {
     if (_isinobject(msg->imsg->MouseX, msg->imsg->MouseY))
      {
      data->x = msg->imsg->MouseX;
      data->y = msg->imsg->MouseY;
      switch (data->inputflags)
       {
       case KEYFRAME_NEW:
        {
        data->win->WKS->Frame = data->lowframe + (data->x - data->left) / data->framepixpt;
        for (i=0; i<data->win->WKS->NumValues; i++)
         {
         if (data->win->DblValue)
          data->win->DblValue[i]
		 = data->SKT->Val[i][data->win->WKS->Frame];
         else if (data->win->FltValue)
          data->win->FltValue[i]
		 = data->SKT->Val[i][data->win->WKS->Frame];
         else if (data->win->ShortValue)
          data->win->ShortValue[i]
		 = data->SKT->Val[i][data->win->WKS->Frame];
	 } /* for i=0... */
        data->inputflags ^= (KEYFRAME_NEW | POINT_SELECTED);
        break;
	} /* new key frame position */
       case KEYFRAME_SELECT:
        {
/* find out whether clicked point is already selected */
        for (i=0; i<data->SKT->NumKeys; i++)
         {
         frame = data->SKT->Key[i]->MoKey.KeyFrame;
         lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
         for (j=data->baseitem; j<data->baseitem+data->dataitems; j++)
          {
          tb_offset = data->bottom - data->valpixpt * 
		(data->SKT->Val[j][frame] - data->textlowval);
          if (data->x > lr_offset - 4 && data->x < lr_offset + 4
		&& data->y > tb_offset - 4 && data->y < tb_offset + 4)
           {
           found = 1;
           if (i == data->activekey && j == data->activeitem)
            {
            MUI_RequestIDCMP(obj, IDCMP_MOUSEMOVE);
            break;
	    } /* if already active prepare to drag point */
           else
            {
            data->activekey = i;
            data->win->ActiveKey = i;
            set(data->win->GKS->Str[0], MUIA_String_Integer, frame);
            break;
	    } /* else new active key selected */
           } /* if in bounds */
          } /* for i=0... */
         if (found) break;
	 } /* for j=data->baseitem... */
        break;
        } /* KEYFRAME_SELECT */
       } /* switch data->inputflags */
      } /* if within area */
     } /* if SELECTDOWN */
    else
     MUI_RejectIDCMP(obj, IDCMP_MOUSEMOVE);
    break;
    } /* MOUSEBUTTONS */

   case IDCMP_MOUSEMOVE:
    {
    data->sy = data->y - msg->imsg->MouseY;
    if (data->sy)
     {
     data->y = msg->imsg->MouseY;
     floatvalue = data->textlowval + (data->bottom - data->y) / data->valpixpt;
     if (data->win->WKS->Precision == WCS_KFPRECISION_DOUBLE)
      data->SKT->Key[data->activekey]->MoKey2.Value[data->activeitem]
		 = floatvalue;
     else if (data->win->WKS->Precision == WCS_KFPRECISION_FLOAT)
      data->SKT->Key[data->activekey]->EcoKey2.Value[data->activeitem]
		 = floatvalue;
     else if (data->win->WKS->Precision == WCS_KFPRECISION_SHORT)
      data->SKT->Key[data->activekey]->CoKey.Value[data->activeitem]
		 = floatvalue;
     nnsetfloat(data->win->ValStr[0], floatvalue);
     nnsetfloat(data->win->PrntValStr[data->activeitem], floatvalue);
     SplineGenericKeys(&data->win->SKT, &data->win->Frames, data->win->WKS->NumValues,
	data->win->WKS->Precision, &data->win->MaxMin[0][0]);
     Set_TL_Data(data->win, data->activeitem);
     MUI_Redraw(obj, MADF_DRAWOBJECT);
     } /* if */
    break;
    } /* MOUSEMOVE */
   } /* switch msg->imsg->Class */
  } /* if msg->imsg */

 return(0);

} /* GNTL_HandleInput() */

/**************************************************************************/
/*
** Here comes the dispatcher for our custom class. We only need to
** care about MUIM_AskMinMax and MUIM_Draw in this simple case.
** Unknown/unused methods are passed to the superclass immediately.
*/

SAVEDS ASM ULONG GNTL_Dispatcher(REG(a0, struct IClass *cl), REG( a2, Object *obj), REG(a1, Msg msg))
{

 switch (msg->MethodID)
  {
  case MUIM_AskMinMax  : return(TL_AskMinMax  		(cl, obj, (APTR)msg));
  case MUIM_Draw       : return(TL_Draw       		(cl, obj, (APTR)msg));
  case MUIM_HandleInput: return(GNTL_HandleInput	(cl, obj, (APTR)msg));
  case MUIM_Setup      : return(TL_Setup      		(cl, obj, (APTR)msg));
  case MUIM_Cleanup    : return(TL_Cleanup    		(cl, obj, (APTR)msg));
  } /* switch msg->MethodID */

 return(DoSuperMethodA(cl, obj, msg));
} /* GNTL_Dispatcher() */

/**********************************************************************/

short Set_TL_Item(struct TimeLineWindow *TL_Win, short item)
{
 LONG data;

 if (! BuildGenericKeyTable(&TL_Win->SKT, *TL_Win->KFPtr,
	*TL_Win->KeyFramesPtr, &TL_Win->Frames, TL_Win->WKS->Group,
	TL_Win->WKS->Item, TL_Win->WKS->NumValues, TL_Win->WKS->Precision,
	&TL_Win->MaxMin[0][0]))
  return (0);
 TL_Win->ActiveKey = GetActiveGenericKey(TL_Win->SKT, TL_Win->WKS->Frame);
 TL_Win->WKS->Frame = TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.KeyFrame;
 set(TL_Win->GKS->Str[0], MUIA_String_Integer, TL_Win->WKS->Frame);
 set(TL_Win->BT_Linear, MUIA_Selected,
	TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.Linear);
 sprintf(str, "%d", TL_Win->WKS->Frame);
 set(TL_Win->FrameTxt, MUIA_Text_Contents, str);
 get (TL_Win->TCB_Cycle, MUIA_Cycle_Active, &data);
 sprintf(str, "%3.2f", TL_Win->WKS->TCB[data]);
 set(TL_Win->CycleStr, MUIA_String_Contents, str);
 if (TL_Win->WKS->Precision == WCS_KFPRECISION_DOUBLE)
  setfloat(TL_Win->ValStr[0], TL_Win->SKT->Key[TL_Win->ActiveKey]->MoKey2.Value[TL_Win->ActiveItem]);
 else if (TL_Win->WKS->Precision == WCS_KFPRECISION_FLOAT)
  setfloat(TL_Win->ValStr[0], TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.Value[TL_Win->ActiveItem]);
 else if (TL_Win->WKS->Precision == WCS_KFPRECISION_SHORT)
  set(TL_Win->ValStr[0], MUIA_String_Integer, TL_Win->SKT->Key[TL_Win->ActiveKey]->CoKey.Value[TL_Win->ActiveItem]);
 TL_Win->KeyItem = item;

 return (1);

} /* Set_TL_Item() */

/*********************************************************************/

void Set_TL_Data(struct TimeLineWindow *TL_Win, short subitem)
{
 LONG first, visible, SelState;
 float valdif, lowval, highval;
 struct Data *data = INST_DATA(TL_Win->TL_Class, TL_Win->TimeLineObj[subitem]);

 get(TL_Win->Prop[0], MUIA_Prop_First, &first);
 get(TL_Win->Prop[0], MUIA_Prop_Visible, &visible);
 first = (first * TL_Win->Frames) / 100;
 data->lowframe = first < TL_Win->Frames - 10 ? first: TL_Win->Frames - 10;
 if (data->lowframe < 0) data->lowframe = 0;
 visible = (visible * TL_Win->Frames) / 100;
 data->highframe = first + (visible > 10 ? visible: 10);
 if (data->highframe > TL_Win->Frames) data->highframe = TL_Win->Frames;
 else if (data->highframe > TL_Win->Frames - 2) data->highframe = TL_Win->Frames;
 valdif = TL_Win->MaxMin[subitem][0] - TL_Win->MaxMin[subitem][1];

 if (valdif < 10.0)
  {
  TL_Win->MaxMin[subitem][0] += 5.0;
  TL_Win->MaxMin[subitem][1] -= 5.0;
  valdif = TL_Win->MaxMin[subitem][0] - TL_Win->MaxMin[subitem][1];
  } /* if */

 highval = TL_Win->MaxMin[subitem][0] + .1 * valdif;
 lowval = TL_Win->MaxMin[subitem][1] - .1 * valdif;

 data->texthighval = highval;
 data->textlowval = lowval;

 if ( (data->highframe - data->lowframe) > 5000) data->framegrid = 500;
 else if ( (data->highframe - data->lowframe) > 1000) data->framegrid = 100;
 else if ( (data->highframe - data->lowframe) > 500) data->framegrid = 50;
 else if ( (data->highframe - data->lowframe) > 100) data->framegrid = 10;
 else if ( (data->highframe - data->lowframe) > 50) data->framegrid = 5;
 else data->framegrid = 1;

 data->framegridlg = data->framegrid * 5;
 data->framegridfirst =
	 data->lowframe - (data->lowframe % data->framegrid) + data->framegrid;

 if ( (data->texthighval - data->textlowval) > 10000.0)
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

 get(TL_Win->BT_Grid, MUIA_Selected, &SelState);
 data->drawgrid = SelState;

 data->group = TL_Win->WKS->Group;
 data->SKT = TL_Win->SKT;
 data->activekey = TL_Win->ActiveKey;
 data->activeitem = subitem;
 data->baseitem = subitem;
 data->dataitems = 1;
 data->win = TL_Win;

} /* Set_TL_Data() */

/***********************************************************************/

void TL_Redraw(struct TimeLineWindow *TL_Win)
{
short i;

 if (TL_Win)
  {
  for (i=0; i<TL_Win->WKS->NumValues; i++)
   {
   Set_TL_Data(TL_Win, i);
   } /* for i=0... */
  MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
  } /* if */

} /* TL_Redraw() */

/***********************************************************************/

void TL_Recompute(struct TimeLineWindow *TL_Win)
{

 if (TL_Win)
  {
  Set_TL_Item(TL_Win, TL_Win->ActiveItem);
  TL_Redraw(TL_Win);
  } /* if */

} /* TL_Recompute() */

/***********************************************************************/

void Update_TL_Win(struct TimeLineWindow *TL_Win, short subitem)
{

 if (TL_Win)
  {
  if (subitem == TL_Win->ActiveItem)
   {
   if (TL_Win->DblValue)
    setfloat(TL_Win->ValStr[0], TL_Win->DblValue[TL_Win->ActiveItem]);
   else if (TL_Win->FltValue)
    setfloat(TL_Win->ValStr[0], TL_Win->FltValue[TL_Win->ActiveItem]);
   else if (TL_Win->ShortValue)
    set(TL_Win->ValStr[0], MUIA_String_Integer, TL_Win->ShortValue[TL_Win->ActiveItem]);
   } /* if */
  } /* if */

} /* Update_TL_Win() */

/***********************************************************************/

void Make_TL_Window(char *NameStr, char **Titles,
	struct TimeLineWindow **TLPtr, APTR *ValStringGads,
	struct WindowKeyStuff *WKS, struct GUIKeyStuff *GKS,
	union KeyFrame **KFPtr, long *KFSizePtr, short *KeyFramesPtr,
	double *DblValue, float *FltValue, short *ShortValue)
{
 struct TimeLineWindow *TL_Win = NULL;
 short i, WinNum = -1;
 long open;
 static const char *TL_Cycle_TCB[] = {"Tens", "Cont", "Bias", NULL};

 for (i=0; i<WCS_MAX_TLWINDOWS; i++)
  {
  if (TLWin[i])
   {
   if (TLWin[i]->WKS == WKS)
    TL_Win = TLWin[i];
   } /* if */
  } /* if */

 if (TL_Win)
  {
  DoMethod(TL_Win->TimeLineWin, MUIM_Window_ToFront);
  set(TL_Win->TimeLineWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  return;
  } /* if window already exists */

 if (WKS->NumValues > WCS_MAX_TLVALUES)
  {
  User_Message("Time Lines",
	"OK, Gary! You know you can't have more than ten values per Time Line.\
 Maybe now you will concede the value of dynamic allocation.",
	"Sure, anything you say!", "s");
  return;
  } /* if no windows left */

 for (i=0; i<WCS_MAX_TLWINDOWS; i++)
  {
  if (! TLWin[i])
   {
   WinNum = i;
   break;
   } /* if */
  } /* if */

 if (WinNum < 0)
  {
  User_Message("Time Lines",
	"You've reached the limit of open Time Line windows. Please close one and try again.",
	"OK", "o");
  return;
  } /* if no windows left */

 if ((TLWin[WinNum] = (struct TimeLineWindow *)
	get_Memory(sizeof (struct TimeLineWindow), MEMF_CLEAR)) == NULL)
  return;

 TL_Win = TLWin[WinNum];
 TL_Win->TLPtr = TLPtr;
 *TLPtr = TL_Win;
 TL_Win->WinNum = WinNum;

 if ((TL_Win->AltKF = (union KeyFrame *)
	get_Memory(*KFSizePtr, MEMF_ANY)) == NULL)
  {
  Close_TL_Window(&TLWin[WinNum], 1);
  return;
  } /* if out of memory */
 memcpy(TL_Win->AltKF, *KFPtr, *KFSizePtr);
 TL_Win->AltKFsize = *KFSizePtr;
 TL_Win->AltKeyFrames = *KeyFramesPtr;
 TL_Win->KFPtr = KFPtr;
 TL_Win->KFSizePtr = KFSizePtr;
 TL_Win->KeyFramesPtr = KeyFramesPtr;
 TL_Win->DblValue = DblValue;
 TL_Win->FltValue = FltValue;
 TL_Win->ShortValue = ShortValue;
 TL_Win->PrntValStr = ValStringGads;
 TL_Win->WKS = WKS;
 TL_Win->GKS = GKS;

 if ( ! (TL_Win->SuperClass = MUI_GetClass(MUIC_Area)))
  {
  Close_TL_Window(&TLWin[WinNum], 1);
  return;
  } /* if out of memory */

/* create the new class */
 if (!(TL_Win->TL_Class =
	 MakeClass(NULL, NULL, TL_Win->SuperClass, sizeof(struct Data), 0)))
  {
  MUI_FreeClass(TL_Win->SuperClass);
  return;
  } /* if */

/* set the dispatcher for the new class */
 TL_Win->TL_Class->cl_Dispatcher.h_Entry    = (APTR)GNTL_Dispatcher;
 TL_Win->TL_Class->cl_Dispatcher.h_SubEntry = NULL;
 TL_Win->TL_Class->cl_Dispatcher.h_Data     = NULL;

  Set_Param_Menu(2);

     TL_Win->TimeLineWin = WindowObject,
      MUIA_Window_Title		, NameStr,
      MUIA_Window_ID		, 'GNTL',
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
        Child, HGroup, MUIA_Group_HorizSpacing, 0,
	  Child, RectangleObject, End,
	  Child, TL_Win->ValStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678",
		MUIA_String_Accept, ".-0123456789", End,
	  Child, RectangleObject, End,
          End, /* HGroup */
	Child, TL_Win->TimeLineGroup = Make_TLRegisterGroup(TL_Win,
		WKS->NumValues, Titles),
	Child, HGroup,
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, VGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1("  Pan "),
                Child, TL_Win->Prop[0] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 100,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 100, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1(" Zoom "),
                Child, TL_Win->Prop[1] = PropObject, PropFrame,
			MUIA_HorizWeight, 400,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_First, 100,
			MUIA_Prop_Visible, 2, End,
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, Label1("Frame "),
                Child, TL_Win->Prop[2] = PropObject, PropFrame,
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
	      Child, TL_Win->BT_PrevKey = KeyButtonObject('v'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, "\33cPrev", End, 
	      Child, TL_Win->BT_NextKey = KeyButtonObject('x'),
		MUIA_FixWidthTxt, "0123456",
		MUIA_Text_Contents, "\33cNext", End, 
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
		MUIA_Group_SameWidth, TRUE,
	      Child, TL_Win->BT_AddKey = KeyButtonFunc('a', "\33cAdd Key"), 
	      Child, TL_Win->BT_DelKey = KeyButtonFunc(127, "\33c\33uDel\33n Key"), 
	      End, /* HGroup */
	    Child, TL_Win->KeysExistTxt = TextObject, TextFrame, End,
	    End, /* VGroup */
	  End, /* HGroup */

	Child, HGroup,
	  Child, TL_Win->BT_Linear = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33cLinear", End,
	  Child, TL_Win->TCB_Cycle = Cycle(TL_Cycle_TCB),
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, TL_Win->CycleStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345", End,
            Child, TL_Win->StrArrow[0] = ImageButton(MUII_ArrowLeft),
            Child, TL_Win->StrArrow[1] = ImageButton(MUII_ArrowRight),
            End, /* HGroup */
	  Child, RectangleObject, End,
	  Child, TL_Win->FrameTxtLbl = Label1("Frame"),
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, TL_Win->FrameTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
            Child, TL_Win->TxtArrowLg[0] = ImageButton(MUII_ArrowLeft),
            Child, TL_Win->TxtArrow[0] = ImageButton(MUII_ArrowLeft),
            Child, TL_Win->TxtArrow[1] = ImageButton(MUII_ArrowRight),
            Child, TL_Win->TxtArrowLg[1] = ImageButton(MUII_ArrowRight),
            End, /* HGroup */
	  End, /* HGroup */
	Child, HGroup,
	  Child, TL_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"),
	  Child, TL_Win->BT_Grid = KeyButtonObject('g'),
	 	 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, TRUE,
	 	 MUIA_Text_Contents, "\33cGrid", End,
	  Child, TL_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"),
	  End, /* HGroup */ 
	End, /* VGroup */
      End; /* WindowObject TL_Win->TimeLineWin */

  if (! TL_Win->TimeLineWin)
   {
   Close_TL_Window(&TLWin[WinNum], 1);
   User_Message("Time Line", "Out of memory!", "OK", "o");
   return;
   } /* out of memory */

  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);

  DoMethod(app, OM_ADDMEMBER, TL_Win->TimeLineWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(TL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_GNTL_CLOSEQUERY(WinNum));  

  MUI_DoNotiPresFal(app, TL_Win->BT_Apply, ID_GNTL_APPLY(WinNum),
   TL_Win->BT_Cancel, ID_GNTL_CLOSE(WinNum),
   TL_Win->BT_AddKey, ID_GNTL_ADDKEY(WinNum),
   TL_Win->BT_DelKey, ID_GNTL_DELETEKEY(WinNum),
   TL_Win->BT_PrevKey, ID_GNTL_PREVKEY(WinNum),
   TL_Win->BT_NextKey, ID_GNTL_NEXTKEY(WinNum), NULL);

  DoMethod(TL_Win->TimeLineGroup, MUIM_Notify, MUIA_Group_ActivePage, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_GNTL_TLGROUP(WinNum));  

  for (i=0; i<WKS->NumValues; i++)
   MUI_DoNotiPresFal(app, TL_Win->TimeLineObj[i], ID_GNTL_TIMELINEOBJ(WinNum, i), NULL);

  DoMethod(TL_Win->BT_Grid, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_GNTL_GRID(WinNum));  
  DoMethod(TL_Win->BT_Linear, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_GNTL_LINEAR(WinNum));  

/* set values */
  set(TL_Win->Prop[0], MUIA_Prop_First, 0);
  set(TL_Win->Prop[0], MUIA_Prop_Visible, 100);
  set(TL_Win->Prop[1], MUIA_Prop_First, 100);
  TL_Win->ActiveItem = 0;  
  if (! Set_TL_Item(TL_Win, WKS->Item))
   {
   Close_TL_Window(&TLWin[WinNum], 1);
   User_Message("Time Lines",
	"At least two key frames for this parameter must be created prior to opening the time line window",
	"OK", "o");
   return;
   } /* if build key table failed */
  for (i=0; i<WKS->NumValues; i++)
   {
   Set_TL_Data(TL_Win, i);
   } /* for i=0... */
  GUIDisableKeyButtons(GKS, TL_Win, WKS);

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app,
   TL_Win->TxtArrow[0], ID_GNTL_TXTARROWLEFT(WinNum),
   TL_Win->TxtArrow[1], ID_GNTL_TXTARROWRIGHT(WinNum),
   TL_Win->TxtArrowLg[0], ID_GNTL_TXTARROWLGLEFT(WinNum),
   TL_Win->TxtArrowLg[1], ID_GNTL_TXTARROWLGRIGHT(WinNum),
   TL_Win->StrArrow[0], ID_GNTL_STRARROWLEFT(WinNum),
   TL_Win->StrArrow[1], ID_GNTL_STRARROWRIGHT(WinNum), NULL);

/* Link prop gadgets to application and each other */
  DoMethod(TL_Win->Prop[0], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_GNTL_PANPROP(WinNum));
  DoMethod(TL_Win->Prop[0], MUIM_Notify, MUIA_Prop_Visible, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_GNTL_PANPROP(WinNum));
  DoMethod(TL_Win->Prop[1], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     TL_Win->Prop[0], 3, MUIM_Set, MUIA_Prop_Visible, MUIV_TriggerValue);
  DoMethod(TL_Win->Prop[2], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_GNTL_FRAMEPROP(WinNum));
	
/* link string gadgets to application */
  DoMethod(TL_Win->CycleStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_GNTL_CYCLESTR(WinNum));
  DoMethod(TL_Win->ValStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_GNTL_VALSTR(WinNum, 0));

/* Link cycle gadgets to application */
  DoMethod(TL_Win->TCB_Cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_GNTL_CYCLE(WinNum));

/* Open window */
  set(TL_Win->TimeLineWin, MUIA_Window_Open, TRUE);
  get(TL_Win->TimeLineWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_TL_Window(&TLWin[WinNum], 1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(TL_Win->TimeLineWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_GNTL_ACTIVATE(WinNum));

/* Get Window structure pointer */
  get(TL_Win->TimeLineWin, MUIA_Window_Window, &TL_Win->Win);

} /* Make_TL_Window() */

/*********************************************************************/

APTR Make_TLRegisterGroup(struct TimeLineWindow *TL_Win, short NumValues,
	char **Titles)
{
APTR group;
short i, error = 0;

 group = RegisterGroup(Titles), End;

 for (i=0; i<NumValues; i++)
  {
  TL_Win->TimeLineObj[i] = NewObject(TL_Win->TL_Class, NULL,
		InputListFrame,
		InnerSpacing(0, 0),
		TAG_DONE);

  if (TL_Win->TimeLineObj[i])
   DoMethod(group, OM_ADDMEMBER, TL_Win->TimeLineObj[i]);
  else
   {
   error = 1;
   break;
   } /* if */
  } /* for i=0... */

  if (error)
   {
   MUI_DisposeObject(group);
   return (NULL);
   } /* if error creating list */

  return (group);

} /* Make_TLRegisterGroup() */

/************************************************************************/

void Close_TL_Window(struct TimeLineWindow **TLWinPtr, short apply)
{
struct TimeLineWindow *TL_Win;

 TL_Win = *TLWinPtr;

 if (TL_Win)
  {
  if (TL_Win->SKT) FreeGenericKeyTable(&TL_Win->SKT, &TL_Win->Frames);
  if (TL_Win->AltKF)
   {
   if (apply) free_Memory(TL_Win->AltKF, TL_Win->AltKFsize);
   else
    {
    free_Memory(*TL_Win->KFPtr, *TL_Win->KFSizePtr);
    *TL_Win->KFPtr = TL_Win->AltKF;
    *TL_Win->KFSizePtr = TL_Win->AltKFsize;
    *TL_Win->KeyFramesPtr = TL_Win->AltKeyFrames;
/* do something here to reset parent window gadgets */
    } /* else discard changes */
   } /* if */
  if (TL_Win->TimeLineWin)
   {
   set(TL_Win->TimeLineWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, TL_Win->TimeLineWin);
   MUI_DisposeObject(TL_Win->TimeLineWin);
   } /* if window created */
  if (TL_Win->TL_Class) FreeClass(TL_Win->TL_Class);         /* free our custom class. */
  MUI_FreeClass(TL_Win->SuperClass); /* release super class pointer. */
  *TL_Win->TLPtr = NULL;
  free_Memory(TL_Win, sizeof (struct TimeLineWindow));
  *TLWinPtr = NULL;
  } /* if */

} /* Close_TL_Window() */

/*********************************************************************/

void Handle_TL_Window(ULONG WCS_ID)
{
 struct TimeLineWindow *TL_Win;
 short i, WinNum;

  WinNum = (WCS_ID & 0x00ff0000) >> 16;
  TL_Win = TLWin[WinNum];

  if (! TL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Time Line window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID & 0x000000ff)
     {
     case 2:					/* ID_GNTL_ADDKEY: */
      {
      struct Data *data = INST_DATA(TL_Win->TL_Class,
	 TL_Win->TimeLineObj[TL_Win->ActiveItem]);

      data->inputflags = KEYFRAME_NEW;
      if (GetInput_Pt(TL_Win->TL_Class, TL_Win->TimeLineObj[TL_Win->ActiveItem]))
       {
       if (MakeGenericKeyFrame(
	TL_Win->KFPtr, TL_Win->KFSizePtr, TL_Win->KeyFramesPtr, TL_Win->WKS->Frame,
	TL_Win->WKS->Group, TL_Win->WKS->Item, TL_Win->WKS->Item,
	TL_Win->WKS->NumValues, TL_Win->DblValue, TL_Win->FltValue,
	TL_Win->ShortValue, TL_Win->WKS->TCB, TL_Win->WKS->Linear,
	TL_Win->WKS->Precision))
        {
        UnsetGenericKeyFrame(*TL_Win->KFPtr, *TL_Win->KeyFramesPtr,
	TL_Win->WKS, TL_Win->WKS->Frame, TL_Win->WKS->Group, TL_Win->WKS->Item,
	0, TL_Win->WKS->Item, TL_Win->WKS->NumValues, TL_Win->DblValue,
	TL_Win->FltValue, TL_Win->ShortValue, TL_Win->WKS->TCB,
	&TL_Win->WKS->Linear, TL_Win->WKS->Precision);
        GUIDisableKeyButtons(TL_Win->GKS, TL_Win, TL_Win->WKS);
        Set_TL_Item(TL_Win, TL_Win->WKS->Item);
        for (i=0; i<TL_Win->WKS->NumValues; i++)
         Set_TL_Data(TL_Win, i);
        MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT); 
        } /* if new key frame */
       else data->inputflags = 0;
       } /* if input point */
      break;
      }
     case 5:					/* ID_GNTL_LINEAR: */
      {
      long SelState;

      get(TL_Win->BT_Linear, MUIA_Selected, &SelState);
      TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.Linear = SelState;
      SplineGenericKeys(&TL_Win->SKT, &TL_Win->Frames, TL_Win->WKS->NumValues,
	TL_Win->WKS->Precision, &TL_Win->MaxMin[0][0]);
      for (i=0; i<TL_Win->WKS->NumValues; i++)
       {
       Set_TL_Data(TL_Win, i);
       } /* for i=0... */
      MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
      break;
      } /* Linear */
     case 4:					/* ID_GNTL_GRID: */
      {
      long SelState;
      
      get(TL_Win->BT_Grid, MUIA_Selected, &SelState);
      for (i=0; i<TL_Win->WKS->NumValues; i++)
       {
       struct Data *data = INST_DATA(TL_Win->TL_Class,
	 TL_Win->TimeLineObj[i]);

       data->drawgrid = SelState;
       } /* for i=0... */
      MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT); 
      break;
      }
     case 6:					/* ID_GNTL_DELETEKEY: */
      {
      set(TL_Win->GKS->BT_DeleteKey, MUIA_Pressed, TRUE);
      set(TL_Win->GKS->BT_DeleteKey, MUIA_Pressed, FALSE);
      break;
      }
     case 7:					/* ID_GNTL_PREVKEY: */
      {
      set(TL_Win->GKS->BT_PrevKey, MUIA_Pressed, TRUE);
      set(TL_Win->GKS->BT_PrevKey, MUIA_Pressed, FALSE);
      break;
      }
     case 8:					/* ID_GNTL_NEXTKEY: */
      {
      set(TL_Win->GKS->BT_NextKey, MUIA_Pressed, TRUE);
      set(TL_Win->GKS->BT_NextKey, MUIA_Pressed, FALSE);
      break;
      }
     case 1:					/* ID_GNTL_APPLY: */
      {
      Close_TL_Window(&TLWin[WinNum], 1);
      break;
      }
     case 0:					/* ID_GNTL_CLOSE: */
      {
      Close_TL_Window(&TLWin[WinNum], 0);
      break;
      }
     case 50:					/* ID_GNTL_CLOSEQUERY: */
      {
      if (*TL_Win->KFSizePtr != TL_Win->AltKFsize
	 || memcmp(*TL_Win->KFPtr, TL_Win->AltKF, *TL_Win->KFSizePtr))
       Close_TL_Window(&TLWin[WinNum], CloseWindow_Query("Time Lines"));
      else
       Close_TL_Window(&TLWin[WinNum], 1);
      break;
      }
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    char *value;
    LONG item;

    get(TL_Win->CycleStr, MUIA_String_Contents, &value);
    get(TL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    TL_Win->WKS->TCB[item] = atof(value);
    TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.TCB[item] = atof(value);
    SplineGenericKeys(&TL_Win->SKT, &TL_Win->Frames, TL_Win->WKS->NumValues,
	TL_Win->WKS->Precision, &TL_Win->MaxMin[0][0]);
    for (i=0; i<TL_Win->WKS->NumValues; i++)
     {
     Set_TL_Data(TL_Win, i);
     } /* for i=0... */
    MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* TCB string */

   case GP_STRING2:
    {
    char *floatvalue;
    double value;

    get(TL_Win->ValStr[0], MUIA_String_Contents, &floatvalue);
    value = atof(floatvalue);
    if (TL_Win->WKS->Precision == WCS_KFPRECISION_DOUBLE)
     TL_Win->SKT->Key[TL_Win->ActiveKey]->MoKey2.Value[TL_Win->ActiveItem]
	 = value;
    else if (TL_Win->WKS->Precision == WCS_KFPRECISION_FLOAT)
     TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.Value[TL_Win->ActiveItem]
	 = value;
    else if (TL_Win->WKS->Precision == WCS_KFPRECISION_SHORT)
     TL_Win->SKT->Key[TL_Win->ActiveKey]->CoKey.Value[TL_Win->ActiveItem]
	 = value;
    setfloat(TL_Win->PrntValStr[TL_Win->ActiveItem], value);
    SplineGenericKeys(&TL_Win->SKT, &TL_Win->Frames, TL_Win->WKS->NumValues,
	TL_Win->WKS->Precision, &TL_Win->MaxMin[0][0]);
    Set_TL_Data(TL_Win, TL_Win->ActiveItem);
    MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* Value string */

   case GP_PROP1:
    {
    ULONG signals;
    struct Data *data = INST_DATA(TL_Win->TL_Class, TL_Win->TimeLineObj[0]);

    data->inputflags |= QUICK_DRAW;
    Set_TL_Data(TL_Win, TL_Win->ActiveItem);
    MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);

    data->inputflags |= NO_CLEAR;

    while ((WCS_ID = DoMethod(app, MUIM_Application_Input, &signals))
            == ID_GNTL_PANPROP(WinNum));
    {
        Set_TL_Data(TL_Win, TL_Win->ActiveItem);
        MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
    };

    data->inputflags ^= (QUICK_DRAW | NO_CLEAR);
    MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
    break;
    } /* pan prop */

   case GP_PROP2:
    {
    long data;

    if (! TL_Win->WKS->StrBlock)
     {
     get(TL_Win->Prop[2], MUIA_Prop_First, &data);
     data = ((float)data / 100.0) * TL_Win->Frames;
     if (data < 1) data = 1;
     set(TL_Win->GKS->Str[0], MUIA_String_Integer, data);
     TL_Win->WKS->PropBlock = 1;
     }
    TL_Win->WKS->StrBlock = 0;
    break;
    } /* frame prop */

   case GP_CYCLE1:
    {
    LONG item;

    get(TL_Win->TCB_Cycle, MUIA_Cycle_Active, &item);
    sprintf(str, "%3.2f", TL_Win->WKS->TCB[item]);
    set(TL_Win->CycleStr, MUIA_String_Contents, str);
    break;
    } /* TCB Cycle */

   case GP_CYCLE3:
    {
    LONG item;

    get(TL_Win->TimeLineGroup, MUIA_Group_ActivePage, &item);
    TL_Win->ActiveItem = item;
    if (TL_Win->WKS->Precision == WCS_KFPRECISION_DOUBLE)
     setfloat(TL_Win->ValStr[0],
	 TL_Win->SKT->Key[TL_Win->ActiveKey]->MoKey2.Value[TL_Win->ActiveItem]);
    else if (TL_Win->WKS->Precision == WCS_KFPRECISION_FLOAT)
     setfloat(TL_Win->ValStr[0], (double)
	 TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.Value[TL_Win->ActiveItem]);
    else if (TL_Win->WKS->Precision == WCS_KFPRECISION_SHORT)
     set(TL_Win->ValStr[0], MUIA_String_Integer,
	 TL_Win->SKT->Key[TL_Win->ActiveKey]->CoKey.Value[TL_Win->ActiveItem]);
    break;
    } /* Register group */

   case GP_ARROW1:
    {
    char *value;

    get(TL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) - .1);
    set(TL_Win->CycleStr, MUIA_String_Contents, str);
    break;
    } /* TCB Arrow */

   case GP_ARROW2:
    {
    char *value;

    get(TL_Win->CycleStr, MUIA_String_Contents, &value);
    sprintf(str, "%3.2f", atof(value) + .1);
    set(TL_Win->CycleStr, MUIA_String_Contents, str);
    break;
    } /* TCB Arrow */

   case GP_ARROW3:
   case GP_ARROW9:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_GNTL_TXTARROWLGLEFT(WinNum)) mult = 10;
    get(TL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame >= mult && (! TL_Win->WKS->PrevKey || frame > TL_Win->WKS->PrevKey + mult))
     {
     frame -= mult;
     sprintf(str, "%ld", frame); 
     set(TL_Win->FrameTxt, MUIA_Text_Contents, str);
     TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.KeyFrame = frame;
     SplineGenericKeys(&TL_Win->SKT, &TL_Win->Frames, TL_Win->WKS->NumValues,
	TL_Win->WKS->Precision, &TL_Win->MaxMin[0][0]);
     Set_TL_Data(TL_Win, TL_Win->ActiveItem);
     MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
     set(TL_Win->GKS->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0100;
     } /* if */
    break;
    } /* ARROW3 */

   case GP_ARROW4:
   case GP_ARROW10:
    {
    char *data;
    long frame, mult = 1;

    if (WCS_ID == ID_GNTL_TXTARROWLGRIGHT(WinNum)) mult = 10;
    get(TL_Win->FrameTxt, MUIA_Text_Contents, &data);
    frame = atoi(data);
    if (frame < TL_Win->WKS->NextKey - mult)
     {
     frame += mult;
     sprintf(str, "%ld", frame);
     set(TL_Win->FrameTxt, MUIA_Text_Contents, str);
     TL_Win->SKT->Key[TL_Win->ActiveKey]->EcoKey2.KeyFrame = frame;
     SplineGenericKeys(&TL_Win->SKT, &TL_Win->Frames, TL_Win->WKS->NumValues,
	TL_Win->WKS->Precision, &TL_Win->MaxMin[0][0]);
     Set_TL_Data(TL_Win, TL_Win->ActiveItem);
     MUI_Redraw(TL_Win->TimeLineObj[TL_Win->ActiveItem], MADF_DRAWOBJECT);
     set(TL_Win->GKS->Str[0], MUIA_String_Integer, frame);
     Par_Mod |= 0x0100;
     } /* if */
    break;
    } /* ARROW4 */

   } /* switch gadget group */

} /* Handle_GenericTL_Window() */
