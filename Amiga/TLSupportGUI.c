/* TimeLineSupport.c
** World Construction Set GUI support functions for Time Line Editing modules.
** By Gary R. Huber, 1994.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "TimeLinesGUI.h"

STATIC_FCN __saveds ULONG TL_HandleInput(struct IClass *cl, Object *obj,
         struct MUIP_HandleInput *msg); // used locally only -> static, AF 19.7.2021

/*
** AskMinMax method will be called before the window is opened
** and before layout takes place. We need to tell MUI the
** minimum, maximum and default size of our object.
*/

__saveds ULONG TL_AskMinMax(struct IClass *cl, Object *obj,
	 struct MUIP_AskMinMax *msg)
{
 /*
 ** let our superclass first fill in what it thinks about sizes.
 ** this will e.g. add the size of frame and inner spacing.
 */

 DoSuperMethodA(cl, obj, msg);

 /*
 ** now add the values specific to our object. note that we
 ** indeed need to *add* these values, not just set them!
 */

 msg->MinMaxInfo->MinWidth  += 300;
 msg->MinMaxInfo->DefWidth  += 300;
 msg->MinMaxInfo->MaxWidth  += MUI_MAXMAX;

 msg->MinMaxInfo->MinHeight += 100;
 msg->MinMaxInfo->DefHeight += 100;
 msg->MinMaxInfo->MaxHeight += MUI_MAXMAX;

 return(0);
} /* TL_AskMinMax() */


/*
** Draw method is called whenever MUI feels we should render
** our object. This usually happens after layout is finished
** or when we need to refresh in a simplerefresh window.
** Note: You may only render within the rectangle
**       _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj).
*/

__saveds ULONG TL_Draw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
 short i, j, halftextht;
 struct Data *data = INST_DATA(cl,obj);
 float lr_offset, tb_offset, value;
 struct TextExtent TE;

 /*
 ** let our superclass draw itself first, area class would
 ** e.g. draw the frame and clear the whole region. What
 ** it does exactly depends on msg->flags.
 */

 
 DoSuperMethodA(cl, obj, msg);

/* compute values for redraw */
 TextExtent(_rp(obj), "A", 1, &TE);
 halftextht = TE.te_Height / 2;
/*
 data->right 		= _mright(obj) 	- 4;
 data->top 		= _mtop(obj) 	+ 6;
 data->bottom 		= _mbottom(obj) - 12;
 data->textbottom	= _mbottom(obj) - 2;
*/
 data->right 		= _mright(obj) 	- 4;
 data->top 		= _mtop(obj) 	+ 2 + halftextht;
 data->bottom 		= _mbottom(obj) - 4 - TE.te_Height;
 data->textbottom	= _mbottom(obj) - 2;
 sprintf(str, "%3.2f", data->texthighval);
 data->textwidthtop 	= TextLength(_rp(obj), str, strlen(str));
 sprintf(str, "%3.2f", data->textlowval);
 data->textwidthbottom = TextLength(_rp(obj), str, strlen(str));
 data->textwidthzero = TextLength(_rp(obj), "0.00", 4);
 data->left 		= _mleft(obj)
	 	+ 4 + max(data->textwidthtop, data->textwidthbottom);
 data->framepixpt = ((float)data->right - data->left) /
	((float)data->highframe - data->lowframe);
 data->framepixgrid = (float)data->framegrid * data->framepixpt;
 data->valpixpt = ((float)data->bottom - data->top) /
	(data->texthighval - data->textlowval);
 data->valpixgrid = data->valgrid * data->valpixpt;
 data->textzero = data->bottom - (-data->textlowval) * data->valpixpt;

 if (data->group == 0) data->activekey = EMTL_Win->ActiveKey;
 else if (data->group == 1) data->activekey = ECTL_Win->ActiveKey;
 else if (data->group == 2) data->activekey = EETL_Win->ActiveKey;

 /*
 ** if MADF_DRAWOBJECT isn't set, we shouldn't draw anything.
 ** MUI just wanted to update the frame or something like that.
 */

 if (msg->flags & MADF_DRAWUPDATE) /* called from our input method */
  {
/* draw key frame points */
  SetAPen(_rp(obj), 2);
  for (j=data->baseitem; j<data->baseitem+data->dataitems; j++)
   {
   for (i=0; i<data->SKT->NumKeys; i++)
    {
    frame = data->SKT->Key[i]->MoKey.KeyFrame;
    lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
    tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[j][frame] - data->textlowval);
    if (lr_offset >= data->left && lr_offset <= data->right + 1
	&& tb_offset > data->top && tb_offset <= data->bottom)
     {
     Move(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
     Draw(_rp(obj), (long)lr_offset + 3, (long)tb_offset + 2);
     Draw(_rp(obj), (long)lr_offset - 3, (long)tb_offset + 2);
     Draw(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
     } /* if in bounds */
    } /* for i=0..., draw key frame triangles */
   } /* for j=data->baseitem... */
  SetAPen(_rp(obj), 7);
  frame = data->SKT->Key[data->activekey]->MoKey.KeyFrame;
  lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
  tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[data->activeitem][frame] - data->textlowval);
  if (lr_offset >= data->left && lr_offset <= data->right + 1
	&& tb_offset > data->top && tb_offset <= data->bottom)
   {
   Move(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
   Draw(_rp(obj), (long)lr_offset + 3, (long)tb_offset + 2);
   Draw(_rp(obj), (long)lr_offset - 3, (long)tb_offset + 2);
   Draw(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
   } /* if in bounds */
  } /* if update only */
 else if (msg->flags & MADF_DRAWOBJECT)
  {
/* draw outline box and scales */
   SetAPen(_rp(obj), 1);
   RectFill(_rp(obj), _mleft(obj), _mtop(obj), _mright(obj), _mbottom(obj));
/*   RectFill(_rp(obj), data->left, data->top, data->right, data->bottom); */
   SetAPen(_rp(obj), 6); /*_dri(obj)->dri_Pens[SHADOWPEN]);*/
   Move(_rp(obj), data->left, data->top);
   Draw(_rp(obj), data->right, data->top);
   Draw(_rp(obj), data->right, data->bottom);
   Draw(_rp(obj), data->left, data->bottom);
   Draw(_rp(obj), data->left, data->top);
   sprintf(str, "%2.2f", data->texthighval);
   Move(_rp(obj), data->left - 2 - data->textwidthtop, data->top + halftextht);
   Text(_rp(obj), str, strlen(str));
   sprintf(str, "%2.2f", data->textlowval);
   Move(_rp(obj), data->left - 2 - data->textwidthbottom, data->bottom + halftextht);
   Text(_rp(obj), str, strlen(str));
   if (data->textzero >= data->top + TE.te_Height
	 && data->textzero < data->bottom - TE.te_Height)
    {
    sprintf(str, "%3.2f", 0.00);
    Move(_rp(obj), data->left - 2 - data->textwidthzero, data->textzero + halftextht);
    Text(_rp(obj), str, strlen(str));
    } /* if */
   sprintf(str, "%1d", data->lowframe);
   Move(_rp(obj), data->left, data->textbottom);
   Text(_rp(obj), str, strlen(str));
   sprintf(str, "%2d", data->highframe);
   Move(_rp(obj), data->right - TextLength(_rp(obj), str, strlen(str)), data->textbottom);
   Text(_rp(obj), str, strlen(str));
/*   } */ /* if clear display */

/* draw grids */
  if (data->drawgrid)
   {
   tb_offset = data->bottom - data->valpixgrid *
	(data->valgridfirst - data->textlowval) / data->valgrid;
   SetAPen(_rp(obj), 4); /*_dri(obj)->dri_Pens[SHINEPEN]);*/
   for (value=data->valgridfirst; value<data->texthighval; value+=data->valgrid)
    {
    Move(_rp(obj), data->left + 1, (long)tb_offset); 
    Draw(_rp(obj), data->right - 1, (long)tb_offset); 
    tb_offset -= data->valpixgrid;
    } /* for value=..., draw value grid */

   lr_offset = data->left + data->framepixgrid *
	((float)data->framegridfirst - data->lowframe) / (float)data->framegrid;
   for (frame=data->framegridfirst; frame<data->highframe; frame+=data->framegrid)
    {
    if (frame % data->framegridlg)
     SetAPen(_rp(obj), 4); /*_dri(obj)->dri_Pens[SHINEPEN]);*/
    else
     SetAPen(_rp(obj), 6); /*1);*/
    Move(_rp(obj), (long)lr_offset, data->top + 1); 
    Draw(_rp(obj), (long)lr_offset, data->bottom - 1); 
    lr_offset += data->framepixgrid;
    } /* for frame=..., draw frame grid */
   } /* if draw grid */

/* draw graph */
  for (j=data->baseitem; j<data->baseitem+data->dataitems; j++)
   {
   if (j == data->baseitem) SetAPen(_rp(obj), 3);
   else if (j == data->baseitem + 1) SetAPen(_rp(obj), 5);
   else SetAPen(_rp(obj), 6);
   lr_offset = data->left;
   tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[j][data->lowframe] - data->textlowval);
   Move(_rp(obj), (long)lr_offset, (long)tb_offset);
   for (frame = data->lowframe + 1; frame<=data->highframe; frame++)
    {
    lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
    tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[j][frame] - data->textlowval);
    Draw(_rp(obj), (long)lr_offset, (long)tb_offset);
    } /* for frames=... */
   } /* for j=data->baseitem... */

/* draw key frame points */
  SetAPen(_rp(obj), 2);
  for (j=data->baseitem; j<data->baseitem+data->dataitems; j++)
   {
   for (i=0; i<data->SKT->NumKeys; i++)
    {
    frame = data->SKT->Key[i]->MoKey.KeyFrame;
    lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
    tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[j][frame] - data->textlowval);
    if (lr_offset >= data->left && lr_offset <= data->right + 1
	&& tb_offset > data->top && tb_offset <= data->bottom)
     {
     Move(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
     Draw(_rp(obj), (long)lr_offset + 3, (long)tb_offset + 2);
     Draw(_rp(obj), (long)lr_offset - 3, (long)tb_offset + 2);
     Draw(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
     } /* if in bounds */
    } /* for i=0..., draw key frame triangles */
   } /* for j=data->baseitem... */

  SetAPen(_rp(obj), 7);
  frame = data->SKT->Key[data->activekey]->MoKey.KeyFrame;
  lr_offset = data->left + data->framepixpt * (frame - data->lowframe);
  tb_offset = data->bottom - data->valpixpt * 
	(data->SKT->Val[data->activeitem][frame] - data->textlowval);
  if (lr_offset >= data->left && lr_offset <= data->right + 1
	&& tb_offset > data->top && tb_offset <= data->bottom)
   {
   Move(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
   Draw(_rp(obj), (long)lr_offset + 3, (long)tb_offset + 2);
   Draw(_rp(obj), (long)lr_offset - 3, (long)tb_offset + 2);
   Draw(_rp(obj), (long)lr_offset, (long)tb_offset - 3);
   } /* if in bounds */

  } /* else if draw entire object */

 return(0);

} /* TL_Draw() */


__saveds ULONG TL_Setup(struct IClass *cl, Object *obj,
	 struct MUIP_HandleInput *msg)
{
 if (!(DoSuperMethodA(cl, obj, msg)))
  return(FALSE);

 MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS/* | IDCMP_RAWKEY*/);

 return(TRUE);
} /* TL_Setup() */


__saveds ULONG TL_Cleanup(struct IClass *cl, Object *obj,
	 struct MUIP_HandleInput *msg)
{
 MUI_RejectIDCMP(obj, IDCMP_MOUSEBUTTONS/* | IDCMP_RAWKEY*/);
 return(DoSuperMethodA(cl, obj, msg));
} /* TL_Cleanup() */


STATIC_FCN __saveds ULONG TL_HandleInput(struct IClass *cl, Object *obj,
	 struct MUIP_HandleInput *msg) // used locally only -> static, AF 19.7.2021
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
        switch (data->group)
         {
         case 0:
          {
          long GroupKey;

          get(EM_Win->BT_GroupKey, MUIA_Selected, &GroupKey);
          EM_Win->Frame = data->lowframe + (data->x - data->left) / data->framepixpt;
          PAR_FIRST_MOTION(EMTL_Win->KeyItem) = data->SKT->Val[0][EM_Win->Frame];
/*          MakeKeyFrame(EM_Win->Frame, 0, EMTL_Win->KeyItem);*/
          if (GroupKey)
           {
           if (incr[0] != 0.0 && EM_Win->MoItem != item[0])
            {
            if (BuildSingleKeyTable(0, item[0]))
             {
             if (EM_Win->Frame <= EMTL_Win->Frames)
              PAR_FIRST_MOTION(item[0]) = SKT[0]->Val[0][EM_Win->Frame];
             else
              PAR_FIRST_MOTION(item[0]) = SKT[0]->Val[0][EMTL_Win->Frames];
	     }
            MakeKeyFrame(EM_Win->Frame, 0, item[0]);
	    }
           if (incr[1] != 0.0 && EM_Win->MoItem != item[1])
            {
            if (BuildSingleKeyTable(0, item[1]))
             {
             if (EM_Win->Frame <= EMTL_Win->Frames)
              PAR_FIRST_MOTION(item[1]) = SKT[0]->Val[0][EM_Win->Frame];
             else
              PAR_FIRST_MOTION(item[1]) = SKT[0]->Val[0][EMTL_Win->Frames];
	     }
            MakeKeyFrame(EM_Win->Frame, 0, item[1]);
	    }
           if (incr[2] != 0.0 && EM_Win->MoItem != item[2])
            {
            if (BuildSingleKeyTable(0, item[2]))
             {
             if (EM_Win->Frame <= EMTL_Win->Frames)
              PAR_FIRST_MOTION(item[2]) = SKT[0]->Val[0][EM_Win->Frame];
             else
              PAR_FIRST_MOTION(item[2]) = SKT[0]->Val[0][EMTL_Win->Frames];
	     }
            MakeKeyFrame(EM_Win->Frame, 0, item[2]);
	    }
	   } /* if group */
          break;
	  } /* motion */
         case 1:
          {
          EC_Win->Frame = data->lowframe + (data->x - data->left) / data->framepixpt;
          PAR_FIRST_COLOR(EC_Win->PalItem, 0) = data->SKT->Val[0][EC_Win->Frame];
          PAR_FIRST_COLOR(EC_Win->PalItem, 1) = data->SKT->Val[1][EC_Win->Frame];
          PAR_FIRST_COLOR(EC_Win->PalItem, 2) = data->SKT->Val[2][EC_Win->Frame];
          break;
	  } /* color */
         case 2:
          {
          EE_Win->Frame = data->lowframe + (data->x - data->left) / data->framepixpt;
          for (i=0; i<10; i++)
           {
           EcoPar.en2[EE_Win->EcoItem].FltValue[i]
		 = data->SKT->Val[i][EE_Win->Frame];
	   } /* for i=0... */
          break;
	  } /* ecosystem */
	 } /* switch group */
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
            switch (data->group)
             {
             case 0:
              {
              EMTL_Win->ActiveKey = i;
              set(EM_Win->Str[0], MUIA_String_Integer, frame);
              break;
	      } /* motion */
             case 1:
              {
	      data->activeitem = j;
              if (ECTL_Win->ActiveKey == i)
	       MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWUPDATE);
              else
               {
               ECTL_Win->ActiveKey = i;
               set(EC_Win->Str[0], MUIA_String_Integer, frame);
	       } /* else active key changed */
              break;
	      } /* color */
             case 2:
              {
              EETL_Win->ActiveKey = i;
              set(EE_Win->Str[0], MUIA_String_Integer, frame);
              break;
	      } /* ecosystem */
	     } /* switch group */
            break;
	    } /* else new active key selected */
           } /* if in bounds */
          } /* for i=0... */
         if (found) break;
	 } /* for j=data->baseitem... */
        break;
        }
       } /* switch data->inputflags */
      } /* if within area */
     } /* if SELECTDOWN */
    else
     MUI_RejectIDCMP(obj, IDCMP_MOUSEMOVE);
    break;
    } /* MOUSEBUTTONS */

   case IDCMP_MOUSEMOVE:
    {
/*    if (_isinobject(msg->imsg->MouseX, msg->imsg->MouseY))
     {*/
     data->sy = data->y - msg->imsg->MouseY;
     if (data->sy)
      {
      data->y = msg->imsg->MouseY;
      floatvalue = data->textlowval + (data->bottom - data->y) / data->valpixpt;
      switch (data->group)
       {
       case 0:
        {
        data->SKT->Key[data->activekey]->MoKey.Value = floatvalue;

        if (floatvalue > parambounds[EMTL_Win->KeyItem][0])
         floatvalue = parambounds[EMTL_Win->KeyItem][0];
        else if (floatvalue < parambounds[EMTL_Win->KeyItem][1])
         floatvalue = parambounds[EMTL_Win->KeyItem][1];
        PAR_FIRST_MOTION(EMTL_Win->KeyItem) = floatvalue;
        setfloat(data->win->ValStr[0], floatvalue);
        Set_Radial_Txt(2);
        Update_EM_Item();
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
        break;
	} /* motion */
       case 1:
        {
        long newdata;

        newdata = floatvalue;
        if (newdata > 255)
         newdata = 255;
        else if (newdata < 0)
         newdata = 0;
        data->SKT->Key[data->activekey]->CoKey.Value[data->activeitem]
	 	= (short)newdata;
        sprintf(str, "%1d", (short)newdata);
        set(data->win->ValStr[data->activeitem], MUIA_String_Contents, str);
	set(EC_Win->CoStr[data->activeitem], MUIA_String_Contents, str);
        SplineSingleKey(1, 0);
        Set_ECTL_Data(data->activeitem);
        break;
	} /* color */
       case 2:
        {
        if (floatvalue > EE_Win->EcoLimits[data->activeitem][0])
         floatvalue = EE_Win->EcoLimits[data->activeitem][0];
        else if (floatvalue < EE_Win->EcoLimits[data->activeitem][1])
         floatvalue = EE_Win->EcoLimits[data->activeitem][1];
        data->SKT->Key[data->activekey]->EcoKey2.Value[data->activeitem]
		 = floatvalue;
        setfloat(data->win->ValStr[0], floatvalue);
        setfloat(EE_Win->IntStr[data->activeitem], floatvalue);
        SplineSingleKey(2, 0);
        Set_EETL_Data(data->activeitem);
        break;
	} /* ecosystem */
       } /* switch group */
      MUI_Redraw(obj, MADF_DRAWOBJECT);
      }
/*     } */ /* if mouse within graph */
    break;
    } /* MOUSEMOVE */
   } /* switch msg->imsg->Class */
  } /* if msg->imsg */

 return(0);

} /* TL_HandleInput() */


/*
** Here comes the dispatcher for our custom class. We only need to
** care about MUIM_AskMinMax and MUIM_Draw in this simple case.
** Unknown/unused methods are passed to the superclass immediately.
*/

SAVEDS ASM ULONG TL_Dispatcher(REG(a0, struct IClass *cl), REG(a2,
	 Object *obj), REG(a1, Msg msg))
{

 switch (msg->MethodID)
  {
  case MUIM_AskMinMax  : return(TL_AskMinMax  (cl, obj, (APTR)msg));
  case MUIM_Draw       : return(TL_Draw       (cl, obj, (APTR)msg));
  case MUIM_HandleInput: return(TL_HandleInput(cl, obj, (APTR)msg));
  case MUIM_Setup      : return(TL_Setup      (cl, obj, (APTR)msg));
  case MUIM_Cleanup    : return(TL_Cleanup    (cl, obj, (APTR)msg));
  } /* switch msg->MethodID */

 return(DoSuperMethodA(cl, obj, msg));
} /* TL_Dispatcher() */

/**********************************************************************/

void ResetTimeLines(short NullGroup)
{
 short i;

 if (EETL_Win && NullGroup != 2)
  {
  if (Set_EETL_Item(EETL_Win->KeyItem))
   {
   for (i=0; i<10; i++)
    {
    Set_EETL_Data(i);
    } /* for i=0... */
   MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
   } /* if */
  Set_PS_List(EETL_Win->List, EETL_Win->ListID, 2, 2, NULL);
  } /* if ecosystem time line window open */
 if (ECTL_Win && NullGroup != 1)
  {
  if (Set_ECTL_Item(ECTL_Win->KeyItem))
   {
   Set_ECTL_Data(-1);
   MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
   } /* if */
  Set_PS_List(ECTL_Win->List, ECTL_Win->ListID, 1, 2, NULL);
  } /* if color time line window open */
 if (EMTL_Win && NullGroup != 0)
  {
  if (Set_EMTL_Item(EMTL_Win->KeyItem))
   {
   Set_EMTL_Data();
   MUI_Redraw(EMTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
   } /* if */
  Set_PS_List(EMTL_Win->List, EMTL_Win->ListID, 0, 2, NULL);
  } /* if motion time line window open */

} /* ResetTimeLines() */

/************************************************************************/

short GetInput_Pt(struct IClass *cl, Object *obj)
{
 short done = 0;
 ULONG Input_ID, signals;
 struct Data *data = INST_DATA(cl, obj);

 while (! done)
  {
  Input_ID = DoMethod(app, MUIM_Application_Input, &signals);

  if (Input_ID > 0) done = 1;
  else if (data->inputflags & POINT_SELECTED) done = 2;

  if (! done && signals) Wait(signals);
  
  } /* while waiting for mouse click in object */

 if (done == 2)
  {
  data->inputflags ^= POINT_SELECTED;
  return (1);
  } /* if mouse pt input */

 return (0);

} /* GetInput_Pt() */

/***********************************************************************/

short Set_EETL_Item(short item)
{
 short i;
 LONG data;

 if (! BuildSingleKeyTable(2, item)) return (0);
 EETL_Win->ActiveKey = GetActiveKey(SKT[2], EE_Win->Frame);
 EE_Win->Frame = SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.KeyFrame;
 set(EE_Win->Str[0], MUIA_String_Integer, EE_Win->Frame);
 set(EETL_Win->BT_Linear, MUIA_Selected,
	SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.Linear);
 for (i=0; i<ECOPARAMS; i++)
  {
  if (item == EETL_Win->ListID[i])
   {
   set(EETL_Win->ParCycle, MUIA_Cycle_Active, i);
   break;
   } /* if item matches list ID */
  } /* for i=0... */
/* set(EETL_Win->ParTxt, MUIA_Text_Contents, PAR_NAME_ECO(EE_Win->EcoItem));*/
 sprintf(str, "%d", EE_Win->Frame);
 set(EETL_Win->FrameTxt, MUIA_Text_Contents, str);
 get (EETL_Win->TCB_Cycle, MUIA_Cycle_Active, &data);
 sprintf(str, "%3.2f", EE_Win->TCB[data]);
 set(EETL_Win->CycleStr, MUIA_String_Contents, str);
 sprintf(str, "%ld",(long int)   // param is double! (Timelines Editor Water Elev initial value)
	 SKT[2]->Key[EETL_Win->ActiveKey]->EcoKey2.Value[EETL_Win->ActiveItem]);
 set(EETL_Win->ValStr[0], MUIA_String_Contents, str);
 EETL_Win->KeyItem = item;

 return (1);
} /* Set_EETL_Item() */

/*********************************************************************/

void Set_EETL_Data(short subitem)
{
 LONG first, visible, SelState;
 float valdif, lowval, highval;
 struct Data *data = INST_DATA(EETL_Win->TL_Class, EETL_Win->TimeLineObj[subitem]);

 get(EETL_Win->Prop[0], MUIA_Prop_First, &first);
 get(EETL_Win->Prop[0], MUIA_Prop_Visible, &visible);
 first = (first * EETL_Win->Frames) / 100;
 data->lowframe = first < EETL_Win->Frames - 10 ? first: EETL_Win->Frames - 10;
 if (data->lowframe < 0) data->lowframe = 0;
 visible = (visible * EETL_Win->Frames) / 100;
 data->highframe = first + (visible > 10 ? visible: 10);
 if (data->highframe > EETL_Win->Frames) data->highframe = EETL_Win->Frames;
 else if (data->highframe > EETL_Win->Frames - 2) data->highframe = EETL_Win->Frames;
 valdif = EETL_Win->MaxMin[subitem][0] - EETL_Win->MaxMin[subitem][1];

 if (valdif < 10.0)
  {
  EETL_Win->MaxMin[subitem][0] += 5.0;
  EETL_Win->MaxMin[subitem][1] -= 5.0;
  valdif = EETL_Win->MaxMin[subitem][0] - EETL_Win->MaxMin[subitem][1];
  } /* if */

 highval = EETL_Win->MaxMin[subitem][0] + .1 * valdif;
 lowval = EETL_Win->MaxMin[subitem][1] - .1 * valdif;
 if (highval > EE_Win->EcoLimits[subitem][0])
  highval = EE_Win->EcoLimits[subitem][0]; 
 if (lowval < EE_Win->EcoLimits[subitem][1])
  lowval = EE_Win->EcoLimits[subitem][1]; 

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

 get(EETL_Win->BT_Grid, MUIA_Selected, &SelState);
 data->drawgrid = SelState;

 data->group = 2;
 data->SKT = SKT[2];
 data->activekey = EETL_Win->ActiveKey;
 data->activeitem = subitem;
 data->baseitem = subitem;
 data->dataitems = 1;
 data->win = EETL_Win;

} /* Set_EETL_Data() */

