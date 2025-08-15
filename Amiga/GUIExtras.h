/* GUIExtras.h
** (renamed from gisgui_extras on 14 Jan 1994 by CXH)
**
** Additional MUI macros...
** Reworked and cleaned up on 22 May 1995 by CXH
** And more, 03 Sep 1995 -CXH
*/

#ifndef _GUIEXTRAS_H_
#define _GUIEXTRAS_H_

/* Used only in EditGUI.c */
#define hcenter(obj)  HGroup, Child, sp, Child, obj, Child, sp, End

/* Probably used all over the place. */
#define sp            RectangleObject, MUIA_Weight, 1, End

/* NOTE: KeyButtonObject and FixedImgObject do not have a hanging , of their
	 own! */

#define FixedImgObject ImageObject,\
  ButtonFrame,\
  MUIA_InputMode, MUIV_InputMode_RelVerify,\
  MUIA_Image_FreeHoriz, FALSE,\
  MUIA_Image_FreeVert, FALSE,\
  MUIA_Background, MUII_BACKGROUND

#define KeyButtonObject(key) TextObject,\
  ButtonFrame,\
  MUIA_Text_PreParse, "\33c",\
  MUIA_Text_SetMax, FALSE,\
  MUIA_Text_HiChar, key,\
  MUIA_ControlChar, key,\
  MUIA_InputMode,MUIV_InputMode_RelVerify,\
  MUIA_Background,MUII_ButtonBack

#define FloatStringObject StringObject,\
  MUIA_String_Accept, "0123456789.-+"


#define ImageButtonWCS(i) ImageObject,\
  ImageButtonFrame,\
  MUIA_Background, MUII_ButtonBack,\
  MUIA_InputMode, MUIV_InputMode_RelVerify,\
  MUIA_Image_Spec, i,\
  End

#define OldImageButton(i) ImageObject,\
  ImageButtonFrame,\
  MUIA_InputMode, MUIV_InputMode_Toggle,\
  MUIA_Image_OldImage, i,\
  End

#define SmallImageButton(i) ImageObject,\
  ImageButtonFrame,\
  MUIA_InputMode, MUIV_InputMode_Toggle,\
  MUIA_InnerRight, 0, MUIA_InnerLeft, 0,\
  MUIA_InnerTop, 0, MUIA_InnerBottom, 0,\
  MUIA_Image_OldImage, i,\
  End

#define SmallImageDisplay(i) ImageObject,\
/*  TextFrame,*/\
  MUIA_InnerRight, 0, MUIA_InnerLeft, 0,\
  MUIA_InnerTop, 0, MUIA_InnerBottom, 0,\
  MUIA_Image_OldImage, i,\
  End

/*
 * For different locale strings we need to find the maximum width of strings
 * that should be aligned vertically. Then MUIA_FixWidth, maxwidth, can be used.
 * arguments: Array of locale-define-numbers, number of locale strings
 * returns: maximum width in pixels
 */
ULONG GetMaxTextWidth(struct RastPort *rp, ULONG *label_id, ULONG num_labels);

#endif
