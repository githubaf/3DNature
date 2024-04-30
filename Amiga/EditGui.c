/* EditGUI.c (ne giseditgui.c 14 Jan 1994 CXH)
** World Construction Set GUI for Editing module.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

STATIC_FCN void SetActiveColor(struct PaletteItem *Pal, short row);  // local --> static AF, 16.July 2021
STATIC_FCN long mid3(long a, long b, long c); // used locally only -> static, AF 20.7.2021
STATIC_FCN void AddColorEntry(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN void UnSet_EC_Item(short item); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Adjust_EcoPal(short i); // used locally only -> static, AF 20.7.2021
STATIC_FCN long max3(long a, long b, long c); // used locally only -> static, AF 20.7.2021
STATIC_FCN void SetColorRequester(short row); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Compute_EcoPal(struct PaletteItem *Pal, short comp_mode); // used locally only -> static, AF 20.7.2021
STATIC_FCN long min3(long a, long b, long c); // used locally only -> static, AF 24.7.2021
STATIC_FCN APTR Make_EC_Group(void); // used locally only -> static, AF 24.7.2021



void Make_EC_Window(void)
{
 short i;
 long open;

 if (EC_Win)
  {
  DoMethod(EC_Win->EcoPalWin, MUIM_Window_ToFront);
  set(EC_Win->EcoPalWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &EC_Win->Colors[0], 16);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),                                      // "Color Editor"
	       GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
               GetString( MSG_GLOBAL_OK ),                                               // "OK"
               (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((EC_Win = (struct EcoPalWindow *)
	get_Memory(sizeof (struct EcoPalWindow), MEMF_CLEAR)) == NULL)
   return;

  if ((EC_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
   {
   Close_EC_Window(1);
   return;
   } /* if out of memory */
  memcpy(EC_Win->AltKF, KF, KFsize);
  EC_Win->AltKFsize = KFsize;
  EC_Win->AltKeyFrames = ParHdr.KeyFrames;

  Set_Param_Menu(1);

     EC_Win->EcoPalWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_EDITGUI_COLOREDITOR ),  // "Color Editor"
      MUIA_Window_ID		, MakeID('E','D','C','O'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2(GetString( MSG_EDITGUI_OPTIONS ) ),  // "Options"
          Child, EC_Win->BT_Settings[0] = KeyButtonFunc('1', (char*)GetString( MSG_EDITGUI_SURFACES ) ),   // "\33cSurfaces"
          Child, EC_Win->BT_Settings[1] = KeyButtonFunc('2', (char*)GetString( MSG_EDITGUI_STRATA ) ),     // "\33cStrata"
          Child, EC_Win->BT_Settings[2] = KeyButtonFunc('3', (char*)GetString( MSG_EDITGUI_CELESTIAL ) ),  // "\33cCelestial"
	  End, /* HGroup */
        Child, HGroup,
/* group on the left */
          Child, VGroup,
/* four rows of colors */
            Child, Make_EC_Group(),
/* list of color names */
            Child, EC_Win->LS_List = ListviewObject,
		MUIA_Listview_Input, TRUE,
              MUIA_Listview_List, ListObject, ReadListFrame, End,
              End, /* ListviewObject */
            End, /* VGroup */

          Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, MUIA_InnerBottom, 0,
           MUIA_HorizWeight, 0, End,
/* Stuff on the right */
          Child, VGroup, MUIA_Group_SameWidth, TRUE,
/* Palette modifying prop gadgets, etc. */
            Child, VGroup, 
              Child, hcenter((EC_Win->Str[5] = StringObject, StringFrame,
			MUIA_String_Contents," ",
			MUIA_String_MaxLen, 21,
			MUIA_FixWidthTxt, "012345678901234567890", End)),

              Child, ColGroup(5), MUIA_Group_HorizSpacing, 0,
                Child, EC_Win->CoStr[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[0] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " R",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,

                Child, EC_Win->CoStr[1] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[1][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[1][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[1] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " G",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,

                Child, EC_Win->CoStr[2] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[2][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[2][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[2] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " B",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,
                End, /* ColGroup */
/*
              Child, hcenter((ImageObject, MUIA_Frame, MUIV_Frame_Text,
			 MUIA_Image_OldImage, &EC_PalGrad, End)),
*/

	      Child, HGroup,
		Child, ImageObject, MUIA_Frame, MUIV_Frame_Text,
			MUIA_InnerTop, 0, MUIA_InnerBottom, 0,
			MUIA_InnerLeft, 0, MUIA_InnerRight, 0,
			MUIA_Image_OldImage, &EC_PalGrad, End,
		Child, RectangleObject, End,
		End, /* HGroup */
              Child, ColGroup(5), MUIA_Group_HorizSpacing, 0,
                Child, EC_Win->CoStr[3] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[3][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[3][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[3] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 367,
			MUIA_Prop_Visible, 7,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " H",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,

                Child, EC_Win->CoStr[4] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[4][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[4][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[4] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_Visible, 2,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " S",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,

                Child, EC_Win->CoStr[5] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                Child, EC_Win->PropArrow[5][0] = ImageButtonWCS(MUII_ArrowLeft),
                Child, EC_Win->PropArrow[5][1] = ImageButtonWCS(MUII_ArrowRight),
                Child, EC_Win->Prop[5] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 102,
			MUIA_Prop_Visible, 2,
			MUIA_Prop_First, 0, End,
                Child, TextObject, MUIA_Text_Contents, " V",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,
                End, /* ColGroup */
              End, /* VGroup */

	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, MUIA_InnerLeft, 0, End,

/* Frame stuff */
            Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDITGUI_EYFRAMES ), End,    // "\33c\0334Key Frames"
              Child, HGroup,
                Child, EC_Win->BT_PrevKey = KeyButtonFunc('v', (char*)GetString( MSG_EDITGUI_PREV )),  // "\33cPrev"
                Child, Label2(GetString( MSG_EDITGUI_FRAME ) ),(char*)                                 // "Frame"
                Child, HGroup, MUIA_Group_HorizSpacing, 0,     (char*)
                  Child, EC_Win->Str[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                  Child, EC_Win->StrArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
                  Child, EC_Win->StrArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
                  End, /* HGroup */
                Child, EC_Win->BT_NextKey = KeyButtonFunc('x', (char*)GetString( MSG_EDITGUI_NEXT )),  // "\33cNext"
                End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EC_Win->BT_MakeKey = KeyButtonFunc('m', (char*)GetString( MSG_EDITGUI_MAKEKEY )),    // "\33cMake Key"
                Child, EC_Win->BT_UpdateKeys = KeyButtonFunc('u', (char*)GetString( MSG_EDITGUI_UPDATE )),  // "\33cUpdate"
                Child, EC_Win->BT_UpdateAll = KeyButtonObject('('),
			MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, GetString( MSG_EDITGUI_ALL0 ), End,  // "\33cAll (0)"
		End, /* HGroup */
              Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EC_Win->BT_DeleteKey = KeyButtonFunc(127, (char*)GetString( MSG_EDITGUI_DELETE ) ),    // "\33c\33uDel\33nete"
                Child, EC_Win->BT_DeleteAll = KeyButtonFunc('d', (char*)GetString( MSG_EDITGUI_DELETEALL )),  // "\33cDelete All"
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, EC_Win->FramePages = VGroup,
                  Child, EC_Win->BT_TimeLines = KeyButtonFunc('t', (char*)GetString( MSG_EDITGUI_TIMELINES ) ),  // "\33cTime Lines "
	          End, /* VGroup */
                Child, EC_Win->BT_KeyScale = KeyButtonFunc('s', (char*)GetString( MSG_EDITGUI_SCALEKEYS ) ),  // "\33cScale Keys "
		End, /* HGroup */
	      End, /* VGroup */

            End, /* VGroup */
          End, /* HGroup */
/* Buttons at bottom */
        Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, MUIA_VertWeight, 0, End,
        Child, VGroup,
          Child, HGroup,
            Child, EC_Win->BT_Copy = KeyButtonFunc('o',  (char*)GetString( MSG_EDITGUI_COPY ) ),     // "\33cCopy"
            Child, EC_Win->BT_Swap = KeyButtonFunc('w', (char*)GetString( MSG_EDITGUI_SWAP ) ),      // "\33cSwap"
            Child, EC_Win->BT_Insert = KeyButtonFunc('i', (char*)GetString( MSG_EDITGUI_INSERT ) ),  // "\33cInsert"
            Child, EC_Win->BT_Remove = KeyButtonFunc('r', (char*)GetString( MSG_EDITGUI_REMOVE ) ),  // "\33cRemove"
            End, /* HGroup */
          Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, EC_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_EDITGUI_KEEP ) ),      // "\33cKeep"
            Child, EC_Win->BT_Cancel = KeyButtonFunc('c',  (char*)GetString( MSG_GLOBAL_33CCANCEL ) ),  // "\33cCancel"
            End, /* HGroup */
          End, /* VGroup */
        End, /* VGroup */
      End; /* WindowObject EC_Win->EcoPalWin */

  if (! EC_Win->EcoPalWin)
   {
   Close_EC_Window(1);
   User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),  // "Color Editor"
                GetString( MSG_EDITGUI_OUTOFMEMORY ),  // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),           // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EC_Win->EcoPalWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* copy color palettes */
  memcpy(&EC_Win->Colors[0], &AltColors[0], sizeof (AltColors));

/* ReturnIDs */
  DoMethod(EC_Win->EcoPalWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EC_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, EC_Win->BT_Apply, ID_EC_APPLY,
   EC_Win->BT_Cancel, ID_EC_CLOSE, EC_Win->BT_MakeKey, ID_EC_MAKEKEY,
   EC_Win->BT_UpdateKeys, ID_EC_UPDATEKEYS, EC_Win->BT_NextKey, ID_EC_NEXTKEY,
   EC_Win->BT_PrevKey, ID_EC_PREVKEY, EC_Win->BT_DeleteKey, ID_EC_DELETEKEY,
   EC_Win->BT_TimeLines, ID_ECTL_WINDOW, EC_Win->BT_KeyScale, ID_PS_WINGRP1,
   EC_Win->BT_Copy, ID_EC_COPY, EC_Win->BT_DeleteAll, ID_EC_DELETEALL,
   EC_Win->BT_Swap, ID_EC_SWAP, EC_Win->BT_Insert, ID_EC_INSERT,
   EC_Win->BT_Remove, ID_EC_REMOVE,
   EC_Win->BT_Settings[0], ID_SB_SETPAGE(4),
   EC_Win->BT_Settings[1], ID_SB_SETPAGE(6),
   EC_Win->BT_Settings[2], ID_SB_SETPAGE(7),
   NULL);


/* set LW style enter command for making key */
  DoMethod(EC_Win->EcoPalWin, MUIM_Notify, MUIA_Window_InputEvent,
	GetString( MSG_EDITGUI_NUMERICPADENTER ), app, 2, MUIM_Application_ReturnID, ID_EC_MAKEKEY);  // "numericpad enter"

/* Link arrow buttons to application */
  for (i=0; i<6; i++)
   MUI_DoNotiPresFal(app, EC_Win->PropArrow[i][0], ID_EC_PARROWLEFT(i),
    EC_Win->PropArrow[i][1], ID_EC_PARROWRIGHT(i), NULL);
  for (i=0; i<1; i++)
   MUI_DoNotiPresFal(app, EC_Win->StrArrow[i][0], ID_EC_SARROWLEFT(i),
    EC_Win->StrArrow[i][1], ID_EC_SARROWRIGHT(i), NULL);

/* Link prop gadgets to strings */
  for (i=0; i<6; i++)
   {
   DoMethod(EC_Win->Prop[i], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
	EC_Win->CoStr[i], 3, MUIM_Set, MUIA_String_Integer, MUIV_TriggerValue);
   } /* for i=0... */

/* link strings to prop gadgets */
  for (i=0; i<6; i++)
   {
/* Worked under MUI 1.4 but not 2.0
   DoMethod(EC_Win->CoStr[i], MUIM_Notify, MUIA_String_Integer, MUIV_EveryTime,
	EC_Win->Prop[i], 3, MUIM_Set, MUIA_Prop_First, MUIV_TriggerValue);
*/
   DoMethod(EC_Win->CoStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EC_COLORSTR(i));
   } /* for i=0... */

/* link prop gadgets to application */
  for (i=0; i<6; i++)
   {
   DoMethod(EC_Win->Prop[i], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EC_PROP(i));
   } /* for i=0... */

/* link color buttons to application */
  set(EC_Win->BT_Col[0], MUIA_Selected, TRUE);
  for (i=0; i<4; i++)
   {
   DoMethod(EC_Win->BT_Col[i], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EC_COLBT0(i));
   } /* for i=0... */

  for (i=0; i<5; i++)
   {
   DoMethod(EC_Win->CoStr[i], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	EC_Win->EcoPalWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, EC_Win->CoStr[i + 1]);
   } /* for i=0... */
  DoMethod(EC_Win->CoStr[5], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	EC_Win->EcoPalWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, EC_Win->CoStr[0]);

/* Set tab cycle chain */
  DoMethod(EC_Win->EcoPalWin, MUIM_Window_SetCycleChain,
	EC_Win->LS_List,
	EC_Win->Str[5], EC_Win->CoStr[0], EC_Win->Prop[0],
	EC_Win->CoStr[1], EC_Win->Prop[1], EC_Win->CoStr[2], EC_Win->Prop[2],
	EC_Win->CoStr[3], EC_Win->Prop[3], EC_Win->CoStr[4], EC_Win->Prop[4],
	EC_Win->CoStr[5], EC_Win->Prop[5],
	EC_Win->BT_PrevKey, EC_Win->Str[0], EC_Win->BT_NextKey,
	EC_Win->BT_MakeKey, EC_Win->BT_UpdateKeys, EC_Win->BT_UpdateAll,
	EC_Win->BT_DeleteKey, 
	EC_Win->BT_DeleteAll, EC_Win->BT_TimeLines, EC_Win->BT_KeyScale,
	EC_Win->BT_Copy, EC_Win->BT_Swap, EC_Win->BT_Insert, EC_Win->BT_Remove,
	EC_Win->BT_Apply, EC_Win->BT_Cancel, NULL);

/* Set active gadget */
  set(EC_Win->EcoPalWin, MUIA_Window_ActiveObject, (IPTR)EC_Win->LS_List);

/* Create color list */
  Set_EC_List(0);

/* Set prop gadgets and list to first palette item */
  EC_Win->PalItem = 0;		/* palette item currently active */
  EC_Win->ActiveRow = 0;
  EC_Win->PaI[0] = 0;		/* palette item in register 0 */
  EC_Win->PaI[1] = 1;		/* palette item in register 1 */
  EC_Win->PaI[2] = 2;		/* palette item in register 2 */
  EC_Win->PaI[3] = 3;		/* palette item in register 3 */
  set(EC_Win->LS_List, MUIA_List_Active, EC_Win->PalItem);

/* Set frame string */
  EC_Win->Frame = 0;
  set(EC_Win->Str[0], MUIA_String_Integer, EC_Win->Frame);

/* disable delete & make keys as appropriate */
  UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
  DisableKeyButtons(1);
  GetKeyTableValues(1, 0, 1);

/* link string gadgets to application */
  DoMethod(EC_Win->Str[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EC_FRAMESTR(0));
  DoMethod(EC_Win->Str[5], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EC_NAMESTR);

/* Set values in text objects */
  SetAllColorRequester();
  Set_EC_Item(EC_Win->PalItem);

/* link list to application */
  DoMethod(EC_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EC_LIST);

/* Open window */
  set(EC_Win->EcoPalWin, MUIA_Window_Open, TRUE);
  get(EC_Win->EcoPalWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EC_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EC_Win->EcoPalWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EC_ACTIVATE);

/* Get Window structure pointer */
  get(EC_Win->EcoPalWin, MUIA_Window_Window, &EC_Win->Win);

} /* Make_EC_Window() */

/*********************************************************************/

STATIC_FCN APTR Make_EC_Group(void) // used locally only -> static, AF 24.7.2021
{
  APTR obj;
  short i, error = 0;
  struct Image *image;

  obj = VGroup, End;

  if (! obj) return (NULL);

  for (i=0; i<4 && ! error; i++)
   {
   Object *name;

   switch (i)
    {
    case 0:
     {
     image = &EC_Button8;
     break;
     }
    case 1:
     {
     image = &EC_Button3;
     break;
     }
    case 2:
     {
     image = &EC_Button5;
     break;
     }
    case 3:
     {
     image = &EC_Button7;
     break;
     }
    } /* switch i */

   name = HGroup, MUIA_Group_HorizSpacing, 2,
     Child, EC_Win->Nme[i] = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234567890123456", End,
     Child, EC_Win->BT_Col[i] = OldImageButton(image),
     End; /* HGroup */
   if (! name) error = 1;
   else DoMethod(obj, OM_ADDMEMBER, name);

   } /* for i=0... */

  if (error)
   {
   MUI_DisposeObject(obj);
   return (NULL);
   } /* if error creating list */

  return (obj);

} /* Make_EC_Group() */

/*********************************************************************/

void Close_EC_Window(short apply)
{

  if (ECTL_Win) Close_ECTL_Window(apply);
  if (EC_Win)
   {
   if (EC_Win->AltKF)
    {
    if (apply) free_Memory(EC_Win->AltKF, EC_Win->AltKFsize);
    else
     {
     MergeKeyFrames(EC_Win->AltKF, EC_Win->AltKeyFrames, &KF,
	 &ParHdr.KeyFrames, &KFsize, 1);
     free_Memory(EC_Win->AltKF, EC_Win->AltKFsize);
     ResetTimeLines(1);
     } /* else discard changes */
    } /* if */
   if (EC_Win->EcoPalWin)
    {
    if (apply)
     {
     FixPar(0, 0x0010);
     FixPar(1, 0x0010);
     }
    else
     {
     UndoPar(0, 0x0010);
     } /* else cancel */
    if (EE_Win)
     {
     short color[3];

     color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 0) / 16;
     color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 1) / 16;
     color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 2) / 16;
     EE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
     color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 0) / 16;
     color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 1) / 16;
     color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 2) / 16;
     EE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
     } /* if eco editor open */
    set(EC_Win->EcoPalWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    DoMethod(app, OM_REMMEMBER, EC_Win->EcoPalWin);
    MUI_DisposeObject(EC_Win->EcoPalWin);
    } /* if window created */
   free_Memory(EC_Win, sizeof (struct EcoPalWindow));
   EC_Win = NULL;
   } /* if */

 if (! apply)
  Par_Mod &= 0x1101;

} /* Close_EC_Window() */

/*********************************************************************/

void Handle_EC_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EC_Window();
   return;
   } /* Open Color Editor Window */

  if (! EC_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &EC_Win->Colors[0], 16);
    break;
    } /* Activate Editing Module window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EC_MAKEKEY:
      {
      long FrameKey;

      sprintf(str, "%d", EC_Win->Frame);
      if (! GetInputString((char*)GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ) ,  // "Enter frame to make key for."
	 "abcdefghijklmnopqrstuvwxyz", str))
       break;
      FrameKey = atoi(str);

      if (MakeKeyFrame(FrameKey, 1, EC_Win->PalItem))
       {
       UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
       GetKeyTableValues(1, EC_Win->PalItem, 1);
       sprintf(EC_Win->Colorname[EC_Win->PalItem], "\0333%s", PAR_NAME_COLOR(EC_Win->PalItem));
       DoMethod(EC_Win->LS_List, MUIM_List_Redraw, EC_Win->PalItem);
       Set_EC_Item(EC_Win->PalItem);
       ResetTimeLines(-1);
       DisableKeyButtons(1);
       } /* if new key frame */
      Par_Mod |= 0x0010;
      break;
      } /* Make Key frame */
     case ID_EC_UPDATEKEYS:
      {
      long UpdateAll;

      get(EC_Win->BT_UpdateAll, MUIA_Selected, &UpdateAll);
      UpdateKeyFrames(EC_Win->Frame, 1, EC_Win->PalItem, (short)UpdateAll, 0);
      Par_Mod |= 0x0010;
      break;
      } /* Update Key frames */
     case ID_EC_NEXTKEY:
      {
      EC_Win->Frame = EC_Win->NextKey;
      set(EC_Win->Str[0], MUIA_String_Integer, EC_Win->Frame);
      break;
      } /* Next Key */
     case ID_EC_PREVKEY:
      {
      EC_Win->Frame = EC_Win->PrevKey;
      set(EC_Win->Str[0], MUIA_String_Integer, EC_Win->Frame);
      break;
      } /* Prev Key */
     case ID_EC_DELETEKEY:
      {
      long DeleteAll;

      get(EC_Win->BT_UpdateAll, MUIA_Selected, &DeleteAll);
      if (DeleteKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem,
	 (short)DeleteAll, 0))
       {
       UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
       GetKeyTableValues(1, EC_Win->PalItem, 1);
       if (DeleteAll)
        {
        Set_EC_List(1);
        if (EE_Win)
         Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ) );  // "Unused"
	} /* if multiple keys deleted */
       else
        {
        if (! CountKeyFrames(1, EC_Win->PalItem))
         {
         sprintf(EC_Win->Colorname[EC_Win->PalItem], "\33n%s", PAR_NAME_COLOR(EC_Win->PalItem));
         DoMethod(EC_Win->LS_List, MUIM_List_Redraw, EC_Win->PalItem);
	 } /* if no more key frames */
	} /* else */
       Set_EC_Item(EC_Win->PalItem);
       ResetTimeLines(-1);
       DisableKeyButtons(1);
       } /* if key frame deleted */
      Par_Mod |= 0x0010;
      break;
      } /* delete key */
     case ID_EC_DELETEALL:
      {
      sprintf(str, (char*)GetString( MSG_EDITGUI_DELETEALLKEYFRAMES ), PAR_NAME_COLOR(EC_Win->PalItem));  // "Delete all %s Key Frames?"
      if (User_Message_Def(GetString( MSG_EDITGUI_PARAMETERSMODULECOLOR ),  // "Parameters Module: Color"
                           (CONST_STRPTR)str,
                           GetString( MSG_GLOBAL_OKCANCEL ),               // "OK|Cancel"
                           (CONST_STRPTR)"oc", 1))
       {
       for (i=ParHdr.KeyFrames-1; i>=0; i--)
        {
        if (KF[i].CoKey.Group == 1)
         {
         if (KF[i].CoKey.Item == EC_Win->PalItem)
          DeleteKeyFrame(KF[i].CoKey.KeyFrame, 1, KF[i].CoKey.Item, 0, 0);
         } /* if group match */
        } /* for i=0... */
       sprintf(EC_Win->Colorname[EC_Win->PalItem], "\33n%s", PAR_NAME_COLOR(EC_Win->PalItem));
       DoMethod(EC_Win->LS_List, MUIM_List_Redraw, EC_Win->PalItem);
       UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
       GetKeyTableValues(1, EC_Win->PalItem, 1);
       ResetTimeLines(-1);
       DisableKeyButtons(1);
       } /* if */
      break;
      } /* delete all */
     case ID_EC_COPY:
      {
      ULONG CopyTo_ID, CopyItem;

      SetPointer(EC_Win->Win, CopyPointer, 16, 16, 0, -5);
      CopyTo_ID = GetInput_ID();
      if ((CopyTo_ID & 0xffffffff) == ID_EC_LIST)
       {
       get(EC_Win->LS_List, MUIA_List_Active, &CopyItem);
       CoPar.cn[CopyItem].Value[0] = CoPar.cn[EC_Win->PalItem].Value[0];
       CoPar.cn[CopyItem].Value[1] = CoPar.cn[EC_Win->PalItem].Value[1];
       CoPar.cn[CopyItem].Value[2] = CoPar.cn[EC_Win->PalItem].Value[2];
       if (CountKeyFrames(1, EC_Win->PalItem))
        {
        if (User_Message_Def(GetString( MSG_EDITGUI_COLOREDITORCOPY ) ,  // "Color Editor: Copy"
        		    GetString( MSG_EDITGUI_COPYKEYFRAMESTOO ),   // "Copy Key Frames too?" 
                            GetString( MSG_EDITGUI_YESNO ),              // "Yes|No"
                            (CONST_STRPTR)"yn", 1))
         {
         for (i=ParHdr.KeyFrames-1; i>=0; i--)
          {
          if (KF[i].CoKey.Group == 1 && KF[i].CoKey.Item == CopyItem)
           DeleteKeyFrame(KF[i].CoKey.KeyFrame, 1, KF[i].CoKey.Item, 0, 0);
          } /* for i=0... */
         for (i=0; i<ParHdr.KeyFrames; i++)
          {
          if (KF[i].CoKey.Group == 1 && KF[i].CoKey.Item == EC_Win->PalItem)
           {
           UnsetKeyFrame(KF[i].CoKey.KeyFrame, 1, EC_Win->PalItem, 0);
           MakeKeyFrame(KF[i].CoKey.KeyFrame, 1, CopyItem);
	   } /* if */
	  } /* for i=0... */
         UnsetKeyFrame(EC_Win->Frame, 1, CopyItem, 0);
	 } /* if copy key frames */
	} /* if key frames exist */
       EC_Win->PalItem = CopyItem;
       Set_EC_List(1);
       if (EE_Win)
        Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
       SetColorRequester(0);
       Set_EC_Item(EC_Win->PalItem);
       Par_Mod |= 0x0010;
       } /* if color button selected */
      ClearPointer(EC_Win->Win);
      break;
      } /* Copy from one color register to another */
     case ID_EC_SWAP:
      {
      ULONG SwapWith_ID, SwapItem;

      SetPointer(EC_Win->Win, SwapPointer, 16, 16, 0, 0);
      SwapWith_ID = GetInput_ID();
      if ((SwapWith_ID & 0xffffffff) == ID_EC_LIST)
       {
       get(EC_Win->LS_List, MUIA_List_Active, &SwapItem);
       if (SwapItem < 24)
        {
        User_Message(GetString( MSG_EDITGUI_COLORPARAMETERSSWAP ),                          // "Color Parameters: Swap"
                     GetString( MSG_EDITGUI_CANTSWAPWITHFIRST24COLORSPERATIONTERMINATED ),  // "Can't swap with first 24 colors!\nOperation terminated."
                     GetString( MSG_GLOBAL_OK ) ,                                          // "OK"
                     (CONST_STRPTR)"oc");
        set(EC_Win->LS_List, MUIA_List_Active, EC_Win->PalItem);
	} /* if */
       else
        {
        swmem(&CoPar.cn[SwapItem],
		 &CoPar.cn[EC_Win->PalItem], sizeof (struct Color));
        AdjustEcosystemColors(WCS_ECOCOLOR_SWAP, SwapItem, EC_Win->PalItem);
        for (i=0; i<ParHdr.KeyFrames; i++)
         {
         if (KF[i].CoKey.Group == 1)
          {
          if (KF[i].CoKey.Item == EC_Win->PalItem)
           KF[i].CoKey.Item = SwapItem;
          else if (KF[i].CoKey.Item == SwapItem)
           KF[i].CoKey.Item = EC_Win->PalItem;
	  } /* if */
	 } /* for i=0... */
        UnsetKeyFrame(EC_Win->Frame, 1, SwapItem, 0);
        EC_Win->PalItem = SwapItem;
        Set_EC_List(1);
        if (EE_Win)
         Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
        SetColorRequester(0);
        Set_EC_Item(EC_Win->PalItem);
        Par_Mod |= 0x0110;
	} /* else not color < 12 */
       } /* if color button selected */
      ClearPointer(EC_Win->Win);
      break;
      } /* Swap two color registers */
     case ID_EC_INSERT:
      {
      memmove(&CoPar.cn[EC_Win->PalItem + 1], &CoPar.cn[EC_Win->PalItem],
	(COLORPARAMS - 1 - EC_Win->PalItem) * sizeof (struct Color));
      memset(&CoPar.cn[EC_Win->PalItem], 0, sizeof (struct Color));
      for (i=0; i<ParHdr.KeyFrames; i++)
       {
       if (KF[i].CoKey.Group == 1)
        {
        if (KF[i].CoKey.Item >= EC_Win->PalItem)
         {
         if (KF[i].CoKey.Item < COLORPARAMS - 1)
          KF[i].CoKey.Item += 1;
         else
          DeleteKeyFrame(KF[i].CoKey.KeyFrame, 1, KF[i].CoKey.Item, 0, 0);
	 } /* if */
	} /* if */
       } /* for i=0... */
      UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
      Set_EC_List(1);
      if (EE_Win)
       Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
      SetColorRequester(0);
      Set_EC_Item(EC_Win->PalItem);
      AdjustEcosystemColors(WCS_ECOCOLOR_INCREMENT, EC_Win->PalItem, 1);
      Par_Mod |= 0x0110;
      break;
      } /* Insert a new color register before the current one */
     case ID_EC_REMOVE:
      {
      short Eco, Remove = 1;
      if ((Eco = SearchEcosystemColorMatch(EC_Win->PalItem)) > -1)
       {
       Remove = User_Message_Def((CONST_STRPTR)PAR_NAME_ECO(Eco),
    		                 GetString( MSG_EDITGUI_THECURRENTCOLORISBEINGUSEDREMOVEITANYWAY ),  // "The current color is being used. Remove it anyway?"
                                 GetString( MSG_GLOBAL_OKCANCEL ),                                  // "OK|Cancel"
                                 (CONST_STRPTR)"oc", 0);
       } /* if in use */
      if (Remove)
       {
       memmove(&CoPar.cn[EC_Win->PalItem], &CoPar.cn[EC_Win->PalItem + 1],
	(COLORPARAMS - 1 - EC_Win->PalItem) * sizeof (struct Color));
       memset(&CoPar.cn[COLORPARAMS - 1], 0, sizeof (struct Color));
       for (i=0; i<ParHdr.KeyFrames; i++)
        {
        if (KF[i].CoKey.Group == 1)
         {
         if (KF[i].CoKey.Item > EC_Win->PalItem)
          {
          KF[i].CoKey.Item -= 1;
 	  } /* if */
	 } /* if */
        } /* for i=0... */
       UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
       Set_EC_List(1);
       if (EE_Win)
        Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
       SetColorRequester(0);
       Set_EC_Item(EC_Win->PalItem);
       AdjustEcosystemColors(WCS_ECOCOLOR_DECREMENT, EC_Win->PalItem + 1, -1);
       Par_Mod |= 0x0110;
       } /* if */
      break;
      } /* Remove current color register */
     case ID_EC_SAVEALL:
      {
      saveparams(0x10, -1, 0);
      break;
      } /* Save entire palette */
     case ID_EC_SAVECURRENT:
      {
      if (saveparams(0x10, EC_Win->PalItem, 0) == 0)
       FixPar(0, 0x0010);
      break;
      } /* Save current color */
     case ID_EC_LOADALL:
      {
      if ((loadparams(0x10, -1)) == 1)
       {
       FixPar(0, 0x0010);
       FixPar(1, 0x0010);
       } /* if load successful */
      UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
      GetKeyTableValues(1, EC_Win->PalItem, 1);
      Set_EC_List(1);
      if (EE_Win)
       Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
      SetAllColorRequester();
      Set_EC_Item(EC_Win->PalItem);
      ResetTimeLines(-1);
      DisableKeyButtons(1);
      break;
      } /* Load entire palette */
     case ID_EC_LOADCURRENT:
      {
      loadparams(0x10, EC_Win->PalItem);
      UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
      GetKeyTableValues(1, EC_Win->PalItem, 1);
      Set_EC_List(1);
      if (EE_Win)
       Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
      SetColorRequester(0);
      Set_EC_Item(EC_Win->PalItem);
      ResetTimeLines(-1);
      DisableKeyButtons(1);
      break;
      } /* Load current color */
     case ID_EC_APPLY:
      {
      Close_EC_Window(1);
      break;
      } /* Apply changes to Palette arrays */
     case ID_EC_CLOSE:
      {
      Close_EC_Window(0);
      break;
      } /* Close and cancel any changes since window opened */
     case ID_EC_CLOSEQUERY:
      {
      if (KFsize != EC_Win->AltKFsize || memcmp(KF, EC_Win->AltKF, KFsize)
		|| memcmp(&CoPar, &UndoCoPar[0], sizeof (CoPar)))
       Close_EC_Window(CloseWindow_Query(GetString( MSG_EDITGUI_COLOREDITOR )));  // "Color Editor"
      else
       Close_EC_Window(1);
      break;
      } /* query and close */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    i = WCS_ID - ID_EC_COLBT0(0);

    if (i == EC_Win->ActiveRow) break;
    set(EC_Win->BT_Col[EC_Win->ActiveRow], MUIA_Selected, FALSE);

    if (EC_Win->PalItem != EC_Win->PaI[i])
     {
     EC_Win->ListBlock = 1;
     UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PaI[i], 0);
     DisableKeyButtons(1);
     GetKeyTableValues(1, EC_Win->PaI[i], 0);
     }
    EC_Win->PalItem = EC_Win->PaI[i];
    EC_Win->ActiveRow = i;
    SetColorRequester(i);
    Set_EC_Item(EC_Win->PalItem);
    set(EC_Win->LS_List, MUIA_List_Active, EC_Win->PalItem);
    break;
    } /* BUTTONS2 */

   case GP_LIST1:
    {
    LONG data;
    if (EC_Win->ListBlock)
     {
     if (ECTL_Win && (EC_Win->IsKey >= 0 || EC_Win->PrevKey >= 0 || EC_Win->NextKey >= 0))
      {
      get(EC_Win->LS_List, MUIA_List_Active, &data);
      EC_Win->PalItem = data;
      if (Set_ECTL_Item(EC_Win->PalItem))
       {
       Set_ECTL_Data(-1);
       MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
       } /* if */
      } /* if Time Line window open */
     EC_Win->ListBlock = 0;
     break;
     } /* if list blocking in effect */
    get(EC_Win->LS_List, MUIA_List_Active, &data);

    set(EC_Win->BT_Col[EC_Win->ActiveRow],
	 MUIA_Selected, FALSE);

    EC_Win->PalItem = data;
    EC_Win->ActiveRow = 0;
    UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
    GetKeyTableValues(1, EC_Win->PalItem, 0);
    set(EC_Win->BT_Col[0], MUIA_Selected, TRUE);
    AddColorEntry();
    if (ECTL_Win && (EC_Win->IsKey >= 0 || EC_Win->PrevKey >= 0 || EC_Win->NextKey >= 0))
     {
     if (Set_ECTL_Item(EC_Win->PalItem))
      {
      Set_ECTL_Data(-1);
      MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWOBJECT);
      } /* if */
     } /* if Time Line window open */
    DisableKeyButtons(1);
    break;
    } /* color list */

   case GP_PROP1:
    {
    LONG data;

    i = WCS_ID - ID_EC_PROP(0);
    Adjust_EcoPal(i);
    if (ECTL_Win && i < 3)
     {
     if ((EC_Win->NextKey >= 0 || EC_Win->PrevKey >= 0) && (EC_Win->IsKey >= 0))
      {
      get(EC_Win->CoStr[i], MUIA_String_Integer, &data);
      set(ECTL_Win->ValStr[i], MUIA_String_Integer, data);
      } /* if key frame */
     } /* if time line window open */
    if (EE_Win)
     {
     short color[3];

     color[0] = PAR_FIRST_COLOR(EC_Win->PalItem, 0) / 16;
     color[1] = PAR_FIRST_COLOR(EC_Win->PalItem, 1) / 16;
     color[2] = PAR_FIRST_COLOR(EC_Win->PalItem, 2) / 16;
     if (EC_Win->PalItem == PAR_COLR_ECO(EE_Win->EcoItem))
      EE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
     if (EC_Win->PalItem == PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)))
      EE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
     } /* if eco editor open */
    break;
    } /* PROP1 */

   case GP_STRING1:
    {
    char *Name;

    get(EC_Win->Str[5], MUIA_String_Contents, &Name);
    strncpy(PAR_NAME_COLOR(EC_Win->PalItem), Name, 23);
    if (CountKeyFrames(1, EC_Win->PalItem))
     {
     sprintf(EC_Win->Colorname[EC_Win->PalItem], "\0333%s", PAR_NAME_COLOR(EC_Win->PalItem));
     } /* if key frame */
    else
     {
     sprintf(EC_Win->Colorname[EC_Win->PalItem], "\33n%s", PAR_NAME_COLOR(EC_Win->PalItem));
     }
    if (EE_Win)
     Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDITGUI_UNUSED ));  // "Unused"
    DoMethod(EC_Win->LS_List, MUIM_List_Redraw, EC_Win->PalItem);
    break;
    } /* Color name string */

   case GP_STRING2:
    {
    LONG data;

    i = WCS_ID - ID_EC_FRAMESTR(0);
    get(EC_Win->Str[i], MUIA_String_Integer, &data);
    switch (i)
     {
     case 0:
      {
      EC_Win->Frame = data;
      if (EC_Win->Frame < 0)
       {
       EC_Win->Frame = 0;
/*       set(EC_Win->Str[0], MUIA_String_Integer, EC_Win->Frame);*/
       } /* if frame < 0 */

      UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 0);
      DisableKeyButtons(1);
      GetKeyTableValues(1, EC_Win->PalItem, 1);
      SetAllColorRequester();
      Set_EC_Item(EC_Win->PalItem);
      if (ECTL_Win)
       {
       long data2;

       get(ECTL_Win->Prop[2], MUIA_Prop_First, &data2);
       data = (100.0 * ((float)EC_Win->Frame / (float)ECTL_Win->Frames));
       if (data != data2 && ! EC_Win->PropBlock)
        { 
        set(ECTL_Win->Prop[2], MUIA_Prop_First, data);
        EC_Win->StrBlock = 1;
        } /* if */      
       EC_Win->PropBlock = 0;
       if (EC_Win->IsKey >= 0)
        {
        ECTL_Win->ActiveKey = GetActiveKey(SKT[1], EC_Win->Frame);
        sprintf(str, "%d", EC_Win->Frame);
        set(ECTL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
        MUI_Redraw(ECTL_Win->TimeLineObj[0], MADF_DRAWUPDATE);
	} /* if key frame */
       } /* if time line window open */
      break;
      } /* frame counter */
     } /* switch i */
    break;
    } /* Frame strings */

   case GP_STRING4:
    {
    LONG data;

    i = WCS_ID - ID_EC_COLORSTR(0);
    get(EC_Win->CoStr[i], MUIA_String_Integer, &data);
    set(EC_Win->Prop[i], MUIA_Prop_First, data);
    break;
    } /* color strings */

   case GP_ARROW1:
    {
    LONG data;

    i = WCS_ID - ID_EC_SARROWLEFT(0);
    get(EC_Win->Str[i], MUIA_String_Integer, &data);
    if (i == 0)
	 set(EC_Win->Str[i], MUIA_String_Integer, (data > 0 ? data - 1: 0));
/*    else set(EC_Win->Str[i], MUIA_String_Integer, (data > 0 ? data - 1: 0));*/
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    LONG data;

    i = WCS_ID - ID_EC_SARROWRIGHT(0);
    get(EC_Win->Str[i], MUIA_String_Integer, &data);
    set(EC_Win->Str[i], MUIA_String_Integer, data + 1);
    break;
    } /* ARROW2 */

   case GP_ARROW3:
    {
    LONG data;

    i = WCS_ID - ID_EC_PARROWLEFT(0);
    get(EC_Win->Prop[i], MUIA_Prop_First, &data);
    set(EC_Win->Prop[i], MUIA_Prop_First, data - 1);
    break;
    } /* ARROW3 */

   case GP_ARROW4:
    {
    LONG data;

    i = WCS_ID - ID_EC_PARROWRIGHT(0);
    get(EC_Win->Prop[i], MUIA_Prop_First, &data);
    set(EC_Win->Prop[i], MUIA_Prop_First, data + 1);
    break;
    } /* ARROW4 */

   } /* switch gadget group */

} /* Handle_EC_Window() */

/*********************************************************************/

STATIC_FCN long max3(long a, long b, long c) // used locally only -> static, AF 20.7.2021
{
 if (a > b)
  {
  if (a > c) return a;
  return c;
  } /* if */
 if (b > c) return b;
 return c;
} /* max3() */

/*********************************************************************/

STATIC_FCN long min3(long a, long b, long c) // used locally only -> static, AF 24.7.2021
{
 if (a < b)
  {
  if (a < c) return a;
  return c;
  } /* if */
 if (b < c) return b;
 return c;
} /* min3() */

/*********************************************************************/

STATIC_FCN long mid3(long a, long b, long c) // used locally only -> static, AF 20.7.2021
{
 if (a < b)
  {
  if (a > c) return a;
  if (b < c) return b;
  return c;
  } /* if */
 if (a < c) return a;
 if (c < b) return b;
 return c;
} /* mid3() */

/*********************************************************************/

STATIC_FCN void Compute_EcoPal(struct PaletteItem *Pal, short comp_mode) // used locally only -> static, AF 20.7.2021
{
 long mmax, mmin, mmid, sign, hueshift;

 if (comp_mode == COMP_HSV)
  {
  mmax = max3(Pal->red, Pal->grn, Pal->blu);
  mmin = min3(Pal->red, Pal->grn, Pal->blu);
  mmid = mid3(Pal->red, Pal->grn, Pal->blu);
  Pal->val = (100 * mmax) / 255;
  if (mmax == 0) Pal->sat = 0;
  else Pal->sat = (100 * (mmax - mmin)) / mmax;
  if (mmax == mmin)
   {
   Pal->hue = 0;
   return;
   } /* if R=G=B */
  if (mmax == Pal->red)
   {
   if (mmid == Pal->grn) sign = +1;
   else sign = -1;
   Pal->hue = 0 + (sign * 60 * (mmid - mmin)) / (mmax - mmin);
   if (Pal->hue < 0) Pal->hue +=359;
   } /* if red max */
  else if (mmax == Pal->grn)
   {
   if (mmid == Pal->blu) sign = +1;
   else sign = -1;
   Pal->hue = 119 + (sign * 60 * (mmid - mmin)) / (mmax - mmin);
   } /* else if grn max */
  else
   {
   if (mmid == Pal->red) sign = +1;
   else sign = -1;
   Pal->hue = 239 + (sign * 60 * (mmid - mmin)) / (mmax - mmin);
   } /* else blu max */
  } /* if comp_mode = COMP_HSV */

 else
  {
  mmax = (Pal->val * 255) / 100;
  mmin = mmax - (Pal->sat * mmax) / 100;
  if (Pal->hue > 59 && Pal->hue <= 179)
   {
   Pal->grn = mmax;
   hueshift = Pal->hue - 119;
   if (hueshift < 0)
    {
    Pal->blu = mmin;
    Pal->red = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* if shift toward red */
   else
    {
    Pal->red = mmin;
    Pal->blu = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* else shift toward blu */
   } /* if dominant hue grn */
  else if (Pal->hue > 179 && Pal->hue <= 299)
   {
   Pal->blu = mmax;
   hueshift = Pal->hue - 239;
   if (hueshift < 0)
    {
    Pal->red = mmin;
    Pal->grn = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* if shift toward grn */
   else
    {
    Pal->grn = mmin;
    Pal->red = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* else shift toward red */
   } /* else if dominant hue blu */
  else
   {
   Pal->red = mmax;
   hueshift = Pal->hue < 119 ? Pal->hue: Pal->hue - 359;
   if (hueshift < 0)
    {
    Pal->grn = mmin;
    Pal->blu = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* if shift toward blu */
   else
    {
    Pal->blu = mmin;
    Pal->grn = mmin + ((mmax - mmin) * abs(hueshift)) / 60;
    } /* else shift toward grn */
   } /* else dominant hue red */
  } /* else comp_mode = COMP_RGB */ 
} /* Compute_EcoPal() */

/*********************************************************************/

void SetAllColorRequester(void)
{

 SetColorRequester(0);
 SetColorRequester(1);
 SetColorRequester(2);
 SetColorRequester(3);

} /* SetAllColorRequester() */

/*********************************************************************/

STATIC_FCN void SetColorRequester(short row) // used locally only -> static, AF 20.7.2021
{
    // Set the Color-Names and Color Components in "Color Editor" Window

 struct PaletteItem Pal;

  Pal.red = PAR_FIRST_COLOR(EC_Win->PaI[row], 0);
  Pal.grn = PAR_FIRST_COLOR(EC_Win->PaI[row], 1);
  Pal.blu = PAR_FIRST_COLOR(EC_Win->PaI[row], 2);
  set(EC_Win->Nme[row], MUIA_Text_Contents, (IPTR)EC_Win->Colorname[EC_Win->PaI[row]]);

  SetActiveColor(&Pal, row);

} /* SetColorRequester() */

/*********************************************************************/

void SetScreen_8(struct PaletteItem *Pal)
{
 short i, color[3];

 for (i=0; i<7; i++)
  {
  color[0] = (Pal->red - ((Pal->red * i) / 7)) / 16;
  color[1] = (Pal->grn - ((Pal->grn * i) / 7)) / 16;
  color[2] = (Pal->blu - ((Pal->blu * i) / 7)) / 16;
  SetRGB4(&WCSScrn->ViewPort, i + 9, color[0], color[1], color[2]);
  EC_Win->Colors[i + 9] = color[0] * 256 + color[1] * 16 + color[2];
  } /* for i=0... */
 
 SetActiveColor(Pal, EC_Win->ActiveRow);

} /* SetScreen_8() */

/*********************************************************************/

STATIC_FCN void SetActiveColor(struct PaletteItem *Pal, short row)
{
 short color[3];

 color[0] = Pal->red / 16;
 color[1] = Pal->grn / 16;
 color[2] = Pal->blu / 16;
 switch (row)
  {
  case 0:
   {
   SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
   EC_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
   break;
   }
  case 1:
   {
   EC_Win->Colors[3] = color[0] * 256 + color[1] * 16 + color[2];
   if (ECTL_Win) break;
   SetRGB4(&WCSScrn->ViewPort, 3, color[0], color[1], color[2]);
   break;
   }
  case 2:
   {
   EC_Win->Colors[5] = color[0] * 256 + color[1] * 16 + color[2];
   if (ECTL_Win) break;
   SetRGB4(&WCSScrn->ViewPort, 5, color[0], color[1], color[2]);
   break;
   }
  case 3:
   {
   EC_Win->Colors[7] = color[0] * 256 + color[1] * 16 + color[2];
   if (ECTL_Win) break;
   SetRGB4(&WCSScrn->ViewPort, 7, color[0], color[1], color[2]);
   break;
   }
  } /* switch active row */

} /* SetActiveColor() */

/*********************************************************************/

STATIC_FCN void Adjust_EcoPal(short i) // used locally only -> static, AF 20.7.2021
{
 struct PaletteItem Pal;

 switch (i)
  {
  case 0:
  case 1:
  case 2:
   {
   get(EC_Win->Prop[0], MUIA_Prop_First, &Pal.red);
   get(EC_Win->Prop[1], MUIA_Prop_First, &Pal.grn);
   get(EC_Win->Prop[2], MUIA_Prop_First, &Pal.blu);
   if (Pal.red == EC_Win->Last[0] && Pal.grn == EC_Win->Last[1] &&
	Pal.blu == EC_Win->Last[2]) return;
   Compute_EcoPal(&Pal, COMP_HSV);
   SetScreen_8(&Pal);
   EC_Win->Last[0] = Pal.red;
   EC_Win->Last[1] = Pal.grn;
   EC_Win->Last[2] = Pal.blu;
   EC_Win->Last[3] = Pal.hue;
   EC_Win->Last[4] = Pal.sat;
   EC_Win->Last[5] = Pal.val;
   set(EC_Win->Prop[3], MUIA_Prop_First, Pal.hue);
   set(EC_Win->Prop[4], MUIA_Prop_First, Pal.sat);
   set(EC_Win->Prop[5], MUIA_Prop_First, Pal.val);
   break;
   } /* case RGB adjustment */
   
  case 3:
  case 4:
  case 5:
   {
   get(EC_Win->Prop[3], MUIA_Prop_First, &Pal.hue);
   get(EC_Win->Prop[4], MUIA_Prop_First, &Pal.sat);
   get(EC_Win->Prop[5], MUIA_Prop_First, &Pal.val);
   if (Pal.hue == EC_Win->Last[3] && Pal.sat == EC_Win->Last[4] &&
	Pal.val == EC_Win->Last[5]) return;
   Compute_EcoPal(&Pal, COMP_RGB);
   SetScreen_8(&Pal);
   EC_Win->Last[3] = Pal.hue;
   EC_Win->Last[4] = Pal.val;
   EC_Win->Last[5] = Pal.sat;
   EC_Win->Last[0] = Pal.red;
   EC_Win->Last[1] = Pal.grn;
   EC_Win->Last[2] = Pal.blu;
   set(EC_Win->Prop[0], MUIA_Prop_First, Pal.red);
   set(EC_Win->Prop[1], MUIA_Prop_First, Pal.grn);
   set(EC_Win->Prop[2], MUIA_Prop_First, Pal.blu);
   break;
   } /* case HSV adjustment */
  } /* switch i */

 UnSet_EC_Item(EC_Win->PalItem);

} /* Adjust_EcoPal() */

/*********************************************************************/

void Set_EC_Item(short item)
{

 set(EC_Win->Str[5], MUIA_String_Contents, (IPTR)PAR_NAME_COLOR(item));
 set(EC_Win->Prop[0], MUIA_Prop_First, PAR_FIRST_COLOR(item, 0));
 set(EC_Win->Prop[1], MUIA_Prop_First, PAR_FIRST_COLOR(item, 1));
 set(EC_Win->Prop[2], MUIA_Prop_First, PAR_FIRST_COLOR(item, 2));
 if (item < 24)
  {
  set(EC_Win->BT_Swap, MUIA_Disabled, TRUE);
  set(EC_Win->BT_Insert, MUIA_Disabled, TRUE);
  set(EC_Win->BT_Remove, MUIA_Disabled, TRUE);
  set(EC_Win->Str[5], MUIA_Disabled, TRUE);
  }
 else
  {
  set(EC_Win->BT_Swap, MUIA_Disabled, FALSE);
  set(EC_Win->BT_Insert, MUIA_Disabled, FALSE);
  set(EC_Win->BT_Remove, MUIA_Disabled, FALSE);
  set(EC_Win->Str[5], MUIA_Disabled, FALSE);
  }
 Adjust_EcoPal(0);

} /* Set_EC_Item() */

/*********************************************************************/

STATIC_FCN void UnSet_EC_Item(short item) // used locally only -> static, AF 20.7.2021
{
 struct PaletteItem Pal;

 get(EC_Win->Prop[0], MUIA_Prop_First, &Pal.red);
 get(EC_Win->Prop[1], MUIA_Prop_First, &Pal.grn);
 get(EC_Win->Prop[2], MUIA_Prop_First, &Pal.blu);
 PAR_FIRST_COLOR(item, 0) = Pal.red;
 PAR_FIRST_COLOR(item, 1) = Pal.grn;
 PAR_FIRST_COLOR(item, 2) = Pal.blu;

} /* UnSet_EC_Item() */

/*********************************************************************/

void Set_EC_List(short update)
{
 short i;

 for (i=0; i<COLORPARAMS; i++)
  {
  if (CountKeyFrames(1, i))
   sprintf(EC_Win->Colorname[i], "\0333%s", PAR_NAME_COLOR(i));
  else
   sprintf(EC_Win->Colorname[i], "\33n%s", PAR_NAME_COLOR(i));
  EC_Win->CName[i] = &EC_Win->Colorname[i][0];
  } /* for i=0... */
 EC_Win->CName[COLORPARAMS] = NULL;

/* Add items or update Color list */
 if (update)
  {
  DoMethod(EC_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(EC_Win->LS_List,
	MUIM_List_Insert, &EC_Win->CName, -1, MUIV_List_Insert_Bottom);
  }

} /* Set_EC_List() */

/*********************************************************************/

STATIC_FCN void AddColorEntry(void) // used locally only -> static, AF 20.7.2021
{

 EC_Win->PaI[3] = EC_Win->PaI[2];
 EC_Win->PaI[2] = EC_Win->PaI[1];
 EC_Win->PaI[1] = EC_Win->PaI[0];
 EC_Win->PaI[0] = EC_Win->PalItem;

 SetAllColorRequester(); 

 Set_EC_Item(EC_Win->PalItem);

} /* AddColorEntry() */
