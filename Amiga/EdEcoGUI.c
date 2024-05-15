/* EdEcoGUI.c
** World Construction Set GUI for Ecosystem Editing module.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

STATIC_FCN APTR Make_EE_Group(void); // used locally only -> static, AF 26.7.2021
STATIC_FCN void UnSet_EE_Item(short item, double data); // used locally only -> static, AF 26.7.2021
STATIC_FCN void Set_EcoLimits(void); // used locally only -> static, AF 26.7.2021


void Make_EE_Window(void)
{
 short i;
 long open;
 static const char *TexClass[5]={NULL};
 static int Init=TRUE;

 if(Init)
 {
	 Init=FALSE;
	 TexClass[0]=(char*)GetString( MSG_EDECOGUI_BRUSHSTAMP );    // "Brush Stamp"
	 TexClass[1]=(char*)GetString( MSG_EDECOGUI_SCALEDIMAGES );  // "Scaled Images"
	 TexClass[2]=(char*)GetString( MSG_EDECOGUI_PROCEDURAL );    // "Procedural"
	 TexClass[3]=(char*)GetString( MSG_EDECOGUI_NONE );          // "None"
	 TexClass[4]=(char*)NULL;
 }

 if (EE_Win)
  {
  DoMethod(EE_Win->EcosystemWin, MUIM_Window_ToFront);
  set(EE_Win->EcosystemWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &EE_Win->Colors[0], 16);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),                                     // "Ecosystem Editor"
               GetString( MSG_EDECOGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENIN ),  // "You must first load or create a parameter file before opening the Editor.",
               GetString( MSG_GLOBAL_OK ),                                                  // "OK"
               (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((EE_Win = (struct EcosystemWindow *)
	get_Memory(sizeof (struct EcosystemWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((EE_Win->AltKF = (union KeyFrame *)
	get_Memory(KFsize, MEMF_ANY)) == NULL)
  {
  Close_EE_Window(1);
  return;
  } /* if out of memory */
 memcpy(EE_Win->AltKF, KF, KFsize);
 EE_Win->AltKFsize = KFsize;
 EE_Win->AltKeyFrames = ParHdr.KeyFrames;

 Set_Param_Menu(2);

 EE_Win->EEListSize = (ECOPARAMS + 1) * (sizeof (char *));
 EE_Win->ECListSize = (COLORPARAMS + 1) * (sizeof (char *));

 EE_Win->EEList = (char **)get_Memory(EE_Win->EEListSize, MEMF_CLEAR);
 EE_Win->ECList = (char **)get_Memory(EE_Win->ECListSize, MEMF_CLEAR);

 if (! EE_Win->EEList || ! EE_Win->ECList)
  {
  User_Message(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),          // "Parameters Module: Ecosystem"
	       GetString( MSG_EDECOGUI_OUTOFMEMORYANTOPENECOSYSTEMEDITOR ),  // "Out of memory!\nCan't open Ecosystem Editor."
               GetString( MSG_GLOBAL_OK ),                                 // "OK"
                (CONST_STRPTR)"o");
  Close_EE_Window(1);
  return;
  } /* if out of memory */

 Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ));  //  "Unused"
 Set_PS_List(EE_Win->ECList, NULL, 1, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ));  // "Unused"

     EE_Win->EcosystemWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),  // "Ecosystem Editor"
      MUIA_Window_ID		, MakeID('E','D','E','C'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2(GetString( MSG_EDECOGUI_OPTIONS )),                                          // "Options"
          Child, EE_Win->BT_Settings[0] = KeyButtonFunc('1', (char*)GetString( MSG_EDECOGUI_CMAPS )),       // "\33cCMaps"
          Child, EE_Win->BT_Settings[1] = KeyButtonFunc('2', (char*)GetString( MSG_EDECOGUI_SURFACE )),     // "\33cSurface"
          Child, EE_Win->BT_Settings[2] = KeyButtonFunc('3', (char*)GetString( MSG_EDECOGUI_FRACTALS )),    // "\33cFractals"
          Child, EE_Win->BT_Settings[3] = KeyButtonFunc('4', (char*)GetString( MSG_EDECOGUI_ECOSYSTEMS )),  // "\33cEcosystems"
          Child, EE_Win->BT_Settings[4] = KeyButtonFunc('5', (char*)GetString( MSG_EDECOGUI_STRATA )),      // "\33cStrata"
          Child, EE_Win->BT_Settings[5] = KeyButtonFunc('6', (char*)GetString( MSG_EDECOGUI_TIDES )),       // "\33cTides"
	  End, /* HGroup */
        Child, HGroup,
/* group on the left */
	  Child, VGroup,
	    Child, HGroup,
	      Child, VGroup,
		Child, HGroup,
	          Child, Label2(GetString( MSG_EDECOGUI_NAME )),  // "Name"
	          Child, EE_Win->NameStr = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456789012", End,
		  End, /* HGroup */
                Child, EE_Win->LS_List = ListviewObject,
			MUIA_Listview_Input, TRUE,
                	MUIA_Listview_List, ListObject, ReadListFrame, End,
                  End, /* ListviewObject */
		End, /* VGroup */
	      Child, VGroup,
	        Child, Label(GetString( MSG_EDECOGUI_EXTURE )),  // "\33c\0334Texture"
	        Child, EE_Win->TexClassCycle = CycleObject,
			MUIA_Cycle_Entries, TexClass, End,
/* begin Texture class page group */
		Child, EE_Win->TexPageGrp = GroupObject, MUIA_Group_PageMode, TRUE,
		  Child, VGroup,
		    Child, HGroup,
	              Child, Label(GetString( MSG_EDECOGUI_CLASS )),  // "Class"
	              Child, EE_Win->ClassCycle = CycleObject,
			MUIA_Cycle_Entries, typename, End,
		      End, /* HGroup */
	            Child, Label2(GetString( MSG_EDECOGUI_DETAILMODEL )),  // "\33cDetail Model"
	            Child, EE_Win->ModelStr = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345678901",
			MUIA_String_Reject, ":;*/?`#%", End,
	            Child, EE_Win->BT_MakeModel = KeyButtonFunc('g', (char*)GetString( MSG_EDECOGUI_DESIGN )),  // "\33cDesign..."
		    End, /* VGroup - Page 0 */
		  Child, VGroup,
		    Child, ColGroup(2),
	              Child, Label(GetString( MSG_EDECOGUI_IMAGES )),  // "Images"
	              Child, EE_Win->TextureText[0] = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234", End,
	              Child, Label(GetString( MSG_EDECOGUI_MAXHT )),  // "Max Ht %"
	              Child, EE_Win->TextureText[1] = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234", End,
	              Child, Label(GetString( MSG_EDECOGUI_MAXIMGHT )),  // "Max Img Ht"
	              Child, EE_Win->TextureText[2] = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234", End,
		      End, /* ColGroup */
	            Child, EE_Win->BT_Edit[0] = KeyButtonFunc('a', (char*)GetString( MSG_EDECOGUI_EDITIMAGES )),  // "\33cEdit Images..."
		    End, /* VGroup - Page 1 */
		  Child, VGroup,
		    Child, HGroup,
	              Child, EE_Win->ProcCheck[0] = CheckMark(0),
		      Child, Label2(GetString( MSG_EDECOGUI_STRATA )),  // "Strata"
		      Child, RectangleObject, End,
		      End, /* HGroup */
		    Child, HGroup,
	              Child, EE_Win->ProcCheck[1] = CheckMark(0),
		      Child, Label2(GetString( MSG_EDECOGUI_STRATACOLORS )),  // "Strata Colors"
		      Child, RectangleObject, End,
		      End, /* HGroup */
		    Child, HGroup,
	              Child, EE_Win->ProcCheck[2] = CheckMark(0),
		      Child, Label2(GetString( MSG_EDECOGUI_FRACTURES )),  // "Fractures"
		      Child, RectangleObject, End,
		      End, /* HGroup */
		    Child, HGroup,
	              Child, EE_Win->ProcCheck[3] = CheckMark(0),
		      Child, Label2(GetString( MSG_EDECOGUI_MUDCRACKS )),  // "Mud Cracks"
		      Child, RectangleObject, End,
		      End, /* HGroup */
		    End, /* VGroup - Page 2 */
		  Child, VGroup,
		      Child, RectangleObject, End,
		    End, /* VGroup - blank page for no texture */
		  End, /* PageGroup */
/* end page group */
	        Child, VGroup,
		  Child, Label(GetString( MSG_EDECOGUI_OLORMAP )),  // "\33c\0334Color Map"
	          Child, HGroup, MUIA_Group_HorizSpacing, 0,
	            Child, Label2(GetString( MSG_EDECOGUI_MATCHRED )),  // " Match Red "		/* Match Red */
	            Child, EE_Win->IntStr[12] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
	            End, /* HGroup */
	          Child, HGroup, MUIA_Group_HorizSpacing, 0,
	            Child, Label2(GetString( MSG_EDECOGUI_MATCHGRN )),  // " Match Grn "		/* Match Green */
	            Child, EE_Win->IntStr[13] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
	            End, /* HGroup */
	          Child, HGroup, MUIA_Group_HorizSpacing, 0,
	            Child, Label2(GetString( MSG_EDECOGUI_MATCHBLU )),  // " Match Blu "		/* Match Blue */
	            Child, EE_Win->IntStr[14] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
	            End, /* HGroup */
		  Child, RectangleObject, End,
	          End, /* VGroup */
		End, /* VGroup */
	      End, /* HGroup */
	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

/* Frame stuff */
            Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, GetString( MSG_EDECOGUI_EYFRAMES ), End,    // "\33c\0334Key Frames"
              Child, HGroup,
                Child, EE_Win->BT_PrevKey = KeyButtonFunc('v', (char*)GetString( MSG_EDECOGUI_PREV )),  // "\33cPrev"
                Child, Label2(GetString( MSG_EDECOGUI_FRAME )),                                  // "Frame"
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
                  Child, EE_Win->Str[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                  Child, EE_Win->StrArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
                  Child, EE_Win->StrArrow[1] = ImageButtonWCS(MUII_ArrowRight),
                  End, /* HGroup */
                Child, EE_Win->BT_NextKey = KeyButtonFunc('x', (char*)GetString( MSG_EDECOGUI_NEXT )),  // "\33cNext"
                End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EE_Win->BT_MakeKey = KeyButtonFunc('m', (char*)GetString( MSG_EDECOGUI_MAKEKEY )),    // "\33cMake Key"
                Child, EE_Win->BT_UpdateKeys = KeyButtonFunc('u', (char*)GetString( MSG_EDECOGUI_UPDATE )),  // "\33cUpdate"
                Child, EE_Win->BT_UpdateAll = KeyButtonObject('('),
			MUIA_InputMode, MUIV_InputMode_Toggle,
		 	MUIA_Text_Contents, (char*)GetString( MSG_EDECOGUI_ALL0 ), End,  // "\33cAll (0)"
		End, /* HGroup */
              Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, EE_Win->BT_DeleteKey = KeyButtonFunc(127, (char*)GetString( MSG_EDECOGUI_DELETE )),     // "\33c\33uDel\33nete"
                Child, EE_Win->BT_DeleteAll = KeyButtonFunc('d', (char*)GetString( MSG_EDECOGUI_DELETEALL )),  // "\33cDelete All"
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, EE_Win->FramePages = VGroup,
                  Child, EE_Win->BT_TimeLines = KeyButtonFunc('t', (char*)GetString( MSG_EDECOGUI_TIMELINES )),  // "\33cTime Lines "
	          End, /* VGroup */
                Child, EE_Win->BT_KeyScale = KeyButtonFunc('s', (char*)GetString( MSG_EDECOGUI_SCALEKEYS )),     // "\33cScale Keys "
		End, /* HGroup */
	      End, /* VGroup */

            End, /* VGroup */

	  Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,

/* group on the right */
          Child, VGroup,
/* ecosystem variables */
            Child, Make_EE_Group(),		/* 12 rows of gadgets */
	    End, /*VGroup */

          End, /* HGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

/* Buttons at bottom */
        Child, VGroup,
          Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, EE_Win->BT_Copy = KeyButtonFunc('o', (char*)GetString( MSG_EDECOGUI_COPY )),      // "\33cCopy"
            Child, EE_Win->BT_Swap = KeyButtonFunc('w', (char*)GetString( MSG_EDECOGUI_SWAP )),      // "\33cSwap"
            Child, EE_Win->BT_Insert = KeyButtonFunc('i', (char*)GetString( MSG_EDECOGUI_INSERT )),  // "\33cInsert"
            Child, EE_Win->BT_Remove = KeyButtonFunc('r', (char*)GetString( MSG_EDECOGUI_REMOVE )),  // "\33cRemove"
            Child, EE_Win->BT_Sort = KeyButtonFunc('l', (char*)GetString( MSG_EDECOGUI_SORTLIST )),  // "\33cSort List"
            End, /* HGroup */
          Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, EE_Win->BT_Apply = KeyButtonFunc('k', (char*)GetString( MSG_EDECOGUI_KEEP )),     // "\33cKeep"
            Child, EE_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_GLOBAL_33CCANCEL )),  // "\33cCancel"
            End, /* HGroup */
          End, /* VGroup */
        End, /* VGroup */
      End; /* WindowObject EE_Win->EcosystemWin */

  if (! EE_Win->EcosystemWin)
   {
   Close_EE_Window(1);
   User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),  // "Ecosystem Editor"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),               // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EE_Win->EcosystemWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* copy color palettes */
 memcpy(&EE_Win->Colors[0], &AltColors[0], sizeof (AltColors));

/* ReturnIDs */
  DoMethod(EE_Win->EcosystemWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EE_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, EE_Win->BT_Apply, ID_EE_APPLY,
   EE_Win->BT_Cancel, ID_EE_CLOSE, EE_Win->BT_MakeKey, ID_EE_MAKEKEY,
   EE_Win->BT_UpdateKeys, ID_EE_UPDATEKEYS, EE_Win->BT_NextKey, ID_EE_NEXTKEY,
   EE_Win->BT_PrevKey, ID_EE_PREVKEY, EE_Win->BT_DeleteKey, ID_EE_DELETEKEY,
   EE_Win->BT_DeleteAll, ID_EE_DELETEALL,
   EE_Win->BT_TimeLines, ID_EETL_WINDOW, EE_Win->BT_KeyScale, ID_PS_WINGRP2,
   EE_Win->BT_MakeModel, ID_FM_WINDOW, EE_Win->BT_Edit[0], ID_FE_WINDOW,
   EE_Win->BT_Copy, ID_EE_COPY,
   EE_Win->BT_Swap, ID_EE_SWAP, EE_Win->BT_Insert, ID_EE_INSERT,
   EE_Win->BT_Remove, ID_EE_REMOVE, EE_Win->BT_Sort, ID_EE_SORT,
   EE_Win->BT_Settings[0], ID_SB_SETPAGE(3),
   EE_Win->BT_Settings[1], ID_SB_SETPAGE(4),
   EE_Win->BT_Settings[2], ID_SB_SETPAGE(5),
   EE_Win->BT_Settings[3], ID_SB_SETPAGE(6),
   EE_Win->BT_Settings[4], ID_SB_SETPAGE(6),
   EE_Win->BT_Settings[5], ID_SB_SETPAGE(7),
   NULL);

/* set LW style enter command for making key */
  DoMethod(EE_Win->EcosystemWin, MUIM_Notify, MUIA_Window_InputEvent,
	GetString( MSG_EDECOGUI_NUMERICPADENTER ), app, 2, MUIM_Application_ReturnID, ID_EE_MAKEKEY);  // "numericpad enter"

/* Link arrow buttons to application */
  MUI_DoNotiPresFal(app, EE_Win->StrArrow[0], ID_EE_SARROWLEFT,
    EE_Win->StrArrow[1], ID_EE_SARROWRIGHT, NULL);
  for (i=0; i<10; i++)
   MUI_DoNotiPresFal(app, EE_Win->IntArrow[i][0], ID_EE_INTARROWLEFT(i),
    EE_Win->IntArrow[i][1], ID_EE_INTARROWRIGHT(i), NULL);

  DoMethod(EE_Win->ClassCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_CLASS);
  DoMethod(EE_Win->TexClassCycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_TEXCLASS);
  for (i=0; i<2; i++)
   DoMethod(EE_Win->ColorCy[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_COLOR(i));

/* link procedural check buttons to app */
  for (i=0; i<4; i++)
   DoMethod(EE_Win->ProcCheck[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_PROCEDURAL(i));

  for (i=2; i<4; i++)
   set(EE_Win->ProcCheck[i], MUIA_Disabled, TRUE);

/* Set cycle chain for Int strings */
  for (i=0; i<7; i++)
   {
   DoMethod(EE_Win->IntStr[i], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	EE_Win->EcosystemWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[i+1]);
   } /* for i=0... */
  for (i=12; i<14; i++)
   {
   DoMethod(EE_Win->IntStr[i], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	EE_Win->EcosystemWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[i+1]);
   } /* for i=0... */

/* Set cycle chain for frame strings */
  DoMethod(EE_Win->ModelStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[0]);
  DoMethod(EE_Win->IntStr[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->NameStr);
  DoMethod(EE_Win->NameStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[8]);
  DoMethod(EE_Win->IntStr[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[9]);
  DoMethod(EE_Win->IntStr[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->IntStr[12]);
  DoMethod(EE_Win->IntStr[14], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   EE_Win->EcosystemWin, 3, MUIM_Set, MUIA_Window_ActiveObject, EE_Win->ModelStr);

/* Set tab cycle chain */
  DoMethod(EE_Win->EcosystemWin, MUIM_Window_SetCycleChain,
	EE_Win->ModelStr, EE_Win->BT_MakeModel,
	EE_Win->ColorCy[0], EE_Win->ColorCy[1],
	EE_Win->IntStr[0], 
	EE_Win->IntStr[1], EE_Win->IntStr[2],
	EE_Win->IntStr[3], EE_Win->IntStr[4],
	EE_Win->IntStr[5], EE_Win->IntStr[6],
	EE_Win->IntStr[7], 
	EE_Win->LS_List,
	EE_Win->NameStr, EE_Win->ClassCycle,
	EE_Win->IntStr[8], EE_Win->IntStr[9],
	EE_Win->IntStr[12], EE_Win->IntStr[13], EE_Win->IntStr[14],
	EE_Win->BT_PrevKey, EE_Win->Str[0], EE_Win->BT_NextKey,
	EE_Win->BT_MakeKey, EE_Win->BT_UpdateKeys, EE_Win->BT_UpdateAll,
	EE_Win->BT_DeleteKey, EE_Win->BT_DeleteAll,
	EE_Win->BT_TimeLines, EE_Win->BT_KeyScale, EE_Win->BT_Copy, EE_Win->BT_Swap,
	EE_Win->BT_Insert, EE_Win->BT_Remove, EE_Win->BT_Sort,
	EE_Win->BT_Apply, EE_Win->BT_Cancel, NULL);

/* Set active gadget */
  set(EE_Win->EcosystemWin, MUIA_Window_ActiveObject, (IPTR)EE_Win->LS_List);

/* Create color list */
  Set_EE_List(0);
  Set_EcoLimits();

/* Set list to first ecosystem item */
  EE_Win->EcoItem = 0;		/* ecosystem item currently active */
  set(EE_Win->LS_List, MUIA_List_Active, EE_Win->EcoItem);

/* Set frame string */
  EE_Win->Frame = 0;
  set(EE_Win->Str[0], MUIA_String_Integer, EE_Win->Frame);

/* disable delete & make keys as appropriate */
  UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
  DisableKeyButtons(2);
  GetKeyTableValues(2, 0, 1);

/* link string gadgets to application */
  for (i=0; i<10; i++)
   {
   DoMethod(EE_Win->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_INTSTR(i));
   } /* for i=0... */
  for (i=12; i<15; i++)
   {
   DoMethod(EE_Win->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EE_INTSTR(i));
   } /* for i=0... */
  DoMethod(EE_Win->Str[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EE_FRAMESTR(0));
  DoMethod(EE_Win->NameStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EE_NAMESTR);
  DoMethod(EE_Win->ModelStr, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_EE_MODELSTR);

/* Set values in text objects */
  Set_EE_Item(EE_Win->EcoItem);

/* link list to application */
  DoMethod(EE_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_EE_LIST);

/* Open window */
  set(EE_Win->EcosystemWin, MUIA_Window_Open, TRUE);
  get(EE_Win->EcosystemWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EE_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EE_Win->EcosystemWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EE_ACTIVATE);

/* Get Window structure pointer */
  get(EE_Win->EcosystemWin, MUIA_Window_Window, &EE_Win->Win);

} /* Make_EE_Window() */

/*********************************************************************/

STATIC_FCN APTR Make_EE_Group(void) // used locally only -> static, AF 26.7.2021
{
  APTR obj, morestuff;
  short i, error = 0;
  static char *LabelText[10] = {NULL};
  static int Init=TRUE;

  if(Init)
  {
 	Init=FALSE;
 	LabelText[0]=(char*)GetString( MSG_EDECOGUI_ELEVLINE );  // " Elev Line ",
 	LabelText[1]=(char*)GetString( MSG_EDECOGUI_ELEVSKEW );  // " Elev Skew ",
 	LabelText[2]=(char*)GetString( MSG_EDECOGUI_ELSKEWAZ );  // "El Skew Az ",
 	LabelText[3]=(char*)GetString( MSG_EDECOGUI_RELELEFF );  // "Rel El Eff ",
 	LabelText[4]=(char*)GetString( MSG_EDECOGUI_MAXRELEL );  // "Max Rel El ",
 	LabelText[5]=(char*)GetString( MSG_EDECOGUI_MINRELEL );  // "Min Rel El ",
 	LabelText[6]=(char*)GetString( MSG_EDECOGUI_MAXSLOPE );  // " Max Slope ",
 	LabelText[7]=(char*)GetString( MSG_EDECOGUI_MINSLOPE );  // " Min Slope ",
	LabelText[8]=(char*)GetString( MSG_EDECOGUI_DENSITY );   // "   Density ",
	LabelText[9]=(char*)GetString( MSG_EDECOGUI_HEIGHT );    // "    Height "
 	};

  obj = VGroup, End;
  if (! obj) return (NULL);

  morestuff = VGroup,
    Child, Label2(GetString( MSG_EDECOGUI_ECOSYSTEMCOLOR )),  // "\33c\0334Ecosystem Color"		/* Overstory Palette */
    Child, HGroup, MUIA_Group_HorizSpacing, 0,
      Child, SmallImageDisplay(&EC_Button8),
      Child, EE_Win->ColorCy[0] = CycleObject,
	MUIA_Cycle_Entries, EE_Win->ECList, End,
      End, /* HGroup */

    Child, Label2(GetString( MSG_EDECOGUI_UNDERSTORYECOSYSTEM )),  // "\33c\0334Understory Ecosystem"	/* Understory Eco */
    Child, HGroup, MUIA_Group_HorizSpacing, 0,
      Child, SmallImageDisplay(&EC_Button9),
      Child, EE_Win->ColorCy[1] = CycleObject,
	MUIA_Cycle_Entries, EE_Win->EEList, End,
      End, /* HGroup */

    End; /* VGroup morestuff */

  if (! morestuff)
   {
   error = 1;
   goto EndIt;
   } /* if */
  else DoMethod(obj, OM_ADDMEMBER, morestuff);


  for (i=8; i<10 && ! error; i++)
   {
   Object *name;

   name = HGroup, MUIA_Group_HorizSpacing, 0,
     Child, EE_Win->Label[i] = Label2(LabelText[i]),
     Child, EE_Win->IntStr[i] = StringObject, StringFrame,
	MUIA_String_Integer, -20000,
	MUIA_String_Accept, ".+-0123456789",
	MUIA_FixWidthTxt, "0123456", End,
     Child, EE_Win->IntArrow[i][0] = ImageButtonWCS(MUII_ArrowLeft),
     Child, EE_Win->IntArrow[i][1] = ImageButtonWCS(MUII_ArrowRight),
     End; /* HGroup */

   if (! name) error = 1;
   else DoMethod(obj, OM_ADDMEMBER, name);

   } /* for i=0... */
  for (i=0; i<8 && ! error; i++)
   {
   Object *name;

   name = HGroup, MUIA_Group_HorizSpacing, 0,
     Child, EE_Win->Label[i] = Label2(LabelText[i]),
     Child, EE_Win->IntStr[i] = StringObject, StringFrame,
	MUIA_String_Integer, -20000,
	MUIA_String_Accept, ".+-0123456789",
	MUIA_FixWidthTxt, "0123456", End,
     Child, EE_Win->IntArrow[i][0] = ImageButtonWCS(MUII_ArrowLeft),
     Child, EE_Win->IntArrow[i][1] = ImageButtonWCS(MUII_ArrowRight),
     End; /* HGroup */

   if (! name) error = 1;
   else DoMethod(obj, OM_ADDMEMBER, name);

   } /* for i=0... */

EndIt:
  if (error)
   {
   MUI_DisposeObject(obj);
   return (NULL);
   } /* if error creating list */

  return (obj);

} /* Make_EE_Group() */

/*********************************************************************/

void Close_EE_Window(short apply)
{
 if (EETL_Win) Close_EETL_Window(apply);
 if (EE_Win)
  {
   if (EE_Win->AltKF)
    {
    if (apply) free_Memory(EE_Win->AltKF, EE_Win->AltKFsize);
    else
     {
     MergeKeyFrames(EE_Win->AltKF, EE_Win->AltKeyFrames,
	&KF, &ParHdr.KeyFrames, &KFsize, 2);
     free_Memory(EE_Win->AltKF, EE_Win->AltKFsize);
     ResetTimeLines(2);
     } /* else discard changes */
    } /* if */
  if (EE_Win->EcosystemWin)
   {
   if (apply)
    {
    FixPar(0, 0x0100);
    FixPar(1, 0x0100);
    }
   else
    {
    UndoPar(0, 0x0100);
    }
   set(EE_Win->EcosystemWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
   DoMethod(app, OM_REMMEMBER, EE_Win->EcosystemWin);
   MUI_DisposeObject(EE_Win->EcosystemWin);
   } /* if window created */
  if (EE_Win->EEList) free_Memory(EE_Win->EEList, EE_Win->EEListSize);
  if (EE_Win->ECList) free_Memory(EE_Win->ECList, EE_Win->ECListSize);
  free_Memory(EE_Win, sizeof (struct EcosystemWindow));
  EE_Win = NULL;
  } /* if */

 if (! apply)
  Par_Mod &= 0x1011;

} /* Close_EE_Window() */

/*********************************************************************/

void Handle_EE_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EE_Window();
   return;
   } /* Open Ecosystem Editor Window */

  if (! EE_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &EE_Win->Colors[0], 16);
    break;
    } /* Activate Settings Editing Window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EE_MAKEKEY:
      {
      long FrameKey;

      sprintf(str, "%d", EE_Win->Frame);
      if (! GetInputString((char*)GetString( MSG_EDECOGUI_ENTERFRAMETOMAKEKEYFOR ),  // "Enter frame to make key for."
	 "abcdefghijklmnopqrstuvwxyz", str))
       break;
      FrameKey = atoi(str);

      if (MakeKeyFrame(FrameKey, 2, EE_Win->EcoItem))
       {
       UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
       GetKeyTableValues(2, EE_Win->EcoItem, 1);
       sprintf(EE_Win->Econame[EE_Win->EcoItem], "\0333%s", PAR_NAME_ECO(EE_Win->EcoItem));
       DoMethod(EE_Win->LS_List, MUIM_List_Redraw, EE_Win->EcoItem);
       Set_EE_Item(EE_Win->EcoItem);
       ResetTimeLines(-1);
       DisableKeyButtons(2);
       } /* if new key frame */
      Par_Mod |= 0x0100;
      break;
      } /* Make Key frame */
     case ID_EE_UPDATEKEYS:
      {
      long UpdateAll;

      get(EE_Win->BT_UpdateAll, MUIA_Selected, &UpdateAll);
      UpdateKeyFrames(EE_Win->Frame, 2, EE_Win->EcoItem, (short)UpdateAll, 0);
      Par_Mod |= 0x0100;
      break;
      } /* Update Key frames */
     case ID_EE_NEXTKEY:
      {
      EE_Win->Frame = EE_Win->NextKey;
      set(EE_Win->Str[0], MUIA_String_Integer, EE_Win->Frame);
      break;
      } /* Next Key */
     case ID_EE_PREVKEY:
      {
      EE_Win->Frame = EE_Win->PrevKey;
      set(EE_Win->Str[0], MUIA_String_Integer, EE_Win->Frame);
      break;
      } /* Prev Key */
     case ID_EE_DELETEKEY:
      {
      long DeleteAll;

      get(EE_Win->BT_UpdateAll, MUIA_Selected, &DeleteAll);
      if (DeleteKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem,
	 (short)DeleteAll, 0))
       {
       UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
       GetKeyTableValues(2, EE_Win->EcoItem, 1);
       if (DeleteAll)
        {
        Set_EE_List(1);
        Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ));  // "Unused"
	} /* if multiple keys deleted */
       else
        {
        if (! CountKeyFrames(2, EE_Win->EcoItem))
         {
         sprintf(EE_Win->Econame[EE_Win->EcoItem], "\33n%s", PAR_NAME_ECO(EE_Win->EcoItem));
         DoMethod(EE_Win->LS_List, MUIM_List_Redraw, EE_Win->EcoItem);
	 } /* if no more key frames */
	} /* else */
       Set_EE_Item(EE_Win->EcoItem);
       ResetTimeLines(-1);
       DisableKeyButtons(2);
       } /* if key frame deleted */
      Par_Mod |= 0x0100;
      break;
      } /* delete key */
     case ID_EE_DELETEALL:
      {
      sprintf(str, (char*)GetString( MSG_EDECOGUI_DELETEALLKEYFRAMES ), PAR_NAME_ECO(EE_Win->EcoItem));  // "Delete all %s Key Frames?"
      if (User_Message_Def(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),                          // "Parameters Module: Ecosystem"
                           (CONST_STRPTR)str,
                           GetString( MSG_GLOBAL_OKCANCEL ),                                           // "OK|Cancel",
                           (CONST_STRPTR)"oc", 1))
       {
       for (i=ParHdr.KeyFrames-1; i>=0; i--)
        {
        if (KF[i].EcoKey.Group == 2)
         {
         if (KF[i].EcoKey.Item == EE_Win->EcoItem)
          DeleteKeyFrame(KF[i].EcoKey.KeyFrame, 2, KF[i].EcoKey.Item, 0, 0);
         } /* if group match */
        } /* for i=0... */
       sprintf(EE_Win->Econame[EE_Win->EcoItem], "\33n%s", PAR_NAME_ECO(EE_Win->EcoItem));
       DoMethod(EE_Win->LS_List, MUIM_List_Redraw, EE_Win->EcoItem);
       UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
       GetKeyTableValues(2, EE_Win->EcoItem, 1);
       ResetTimeLines(-1);
       DisableKeyButtons(2);
       } /* if */
      break;
      } /* delete all */
     case ID_EE_COPY:
      {
      ULONG CopyTo_ID, CopyItem;

      SetPointer(EE_Win->Win, CopyPointer, 16, 16, 0, -5);
      CopyTo_ID = GetInput_ID();
      if ((CopyTo_ID & 0xffffffff) == ID_EE_LIST)
       {
       get(EE_Win->LS_List, MUIA_List_Active, &CopyItem);
/* this could be a problem on other systems if structure sizes vary */
       memcpy(&EcoPar.en[CopyItem].Line,
	 &EcoPar.en[EE_Win->EcoItem].Line, sizeof (struct Ecosystem) - 24);
       EE_Win->EcoItem = CopyItem;
       Set_EE_List(1);
       Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ));  // "Unused"
       Set_EE_Item(EE_Win->EcoItem);
       Par_Mod |= 0x0100;
       } /* if ecosystem item selected */
      ClearPointer(EE_Win->Win);
      break;
      } /* Copy from one ecosystem register to another */
     case ID_EE_SWAP:
      {
      ULONG SwapWith_ID, SwapItem;

      SetPointer(EE_Win->Win, SwapPointer, 16, 16, 0, 0);
      SwapWith_ID = GetInput_ID();
      if ((SwapWith_ID & 0xffffffff) == ID_EE_LIST)
       {
       get(EE_Win->LS_List, MUIA_List_Active, &SwapItem);
       if (SwapItem < 12)
        {
        User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMPARAMETERSSWAP ) ,                         // "Ecosystem Parameters: Swap"
                     GetString( MSG_EDECOGUI_CANTSWAPWITHFIRST12ECOSYSTEMSPERATIONTERMINATED ),  // "Can't swap with first 12 ecosystems!\nOperation terminated."
                     GetString( MSG_GLOBAL_OK ),                                               // "OK"
                     (CONST_STRPTR)"oc");
        set(EE_Win->LS_List, MUIA_List_Active, EE_Win->EcoItem);
	}
       else
        {
        swmem(&EcoPar.en[SwapItem],
		 &EcoPar.en[EE_Win->EcoItem], sizeof (struct Ecosystem));
        for (i=0; i<ECOPARAMS; i++)
         {
         if (PAR_UNDER_ECO(i) == EE_Win->EcoItem)
          PAR_UNDER_ECO(i) = SwapItem;
         else if (PAR_UNDER_ECO(i) == SwapItem)
          PAR_UNDER_ECO(i) = EE_Win->EcoItem;
         } /* for i=... */
        swmem(&EcoShift[SwapItem].Ecotype, &EcoShift[EE_Win->EcoItem].Ecotype,
		sizeof (struct Ecotype *));
        Update_EcoLegend(EE_Win->EcoItem, SwapItem, 2);
        EE_Win->EcoItem = SwapItem;
        Set_EE_List(1);
        Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
        Set_EE_Item(EE_Win->EcoItem);
        Par_Mod |= 0x0100;
	} /* else not water or snow */
       } /* if ecosystem item selected */
      ClearPointer(EE_Win->Win);
      break;
      } /* Swap two ecosystem registers */
     case ID_EE_INSERT:
      {
      memmove(&EcoPar.en[EE_Win->EcoItem + 1], &EcoPar.en[EE_Win->EcoItem],
	(ECOPARAMS - 1 - EE_Win->EcoItem) * sizeof (struct Ecosystem));
      memmove(&EcoShift[EE_Win->EcoItem + 1], &EcoShift[EE_Win->EcoItem],
	(ECOPARAMS - 1 - EE_Win->EcoItem) * sizeof (struct EcosystemShift));
      setecodefault(EE_Win->EcoItem);
      Set_EE_List(1);
      Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
      for (i=0; i<ECOPARAMS; i++)
       {
       if (PAR_UNDER_ECO(i) >= EE_Win->EcoItem && PAR_UNDER_ECO(i) < ECOPARAMS - 1)
        PAR_UNDER_ECO(i) ++;
       } /* for i=... */
      Set_EE_Item(EE_Win->EcoItem);
      Update_EcoLegend(EE_Win->EcoItem, 0, 3);
      Par_Mod |= 0x0100;
      break;
      } /* Insert a new ecosystem register before the current one */
     case ID_EE_REMOVE:
      {
      memmove(&EcoPar.en[EE_Win->EcoItem], &EcoPar.en[EE_Win->EcoItem + 1],
	(ECOPARAMS - 1 - EE_Win->EcoItem) * sizeof (struct Ecosystem));
      memmove(&EcoShift[EE_Win->EcoItem], &EcoShift[EE_Win->EcoItem + 1],
	(ECOPARAMS - 1 - EE_Win->EcoItem) * sizeof (struct EcosystemShift));
      setecodefault(ECOPARAMS - 1);
      Set_EE_List(1);
      Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
      for (i=0; i<ECOPARAMS; i++)
       {
       if (PAR_UNDER_ECO(i) > EE_Win->EcoItem)
        PAR_UNDER_ECO(i) --;
       } /* for i=... */
      Set_EE_Item(EE_Win->EcoItem);
      Update_EcoLegend(EE_Win->EcoItem, 0, 4);
      Par_Mod |= 0x0100;
      break;
      } /* Remove current ecosystem register */
     case ID_EE_SORT:
      {
      Sort_Eco_Params();
      Set_EE_List(1);
      Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
      Set_EE_Item(EE_Win->EcoItem);
      set(EE_Win->LS_List, MUIA_List_Active, EE_Win->EcoItem);
      break;
      } /* sort eco list */
     case ID_EE_SAVEALL:
      {
      if (saveparams(0x0100, -1, 0) == 0)
       {
       FixPar(0, 0x0100);
       } /* if no save error */
      break;
      } /* Save entire palette */
     case ID_EE_SAVECURRENT:
      {
      if (saveparams(0x0100, EE_Win->EcoItem, 0) == 0)
       FixPar(0, 0x0100);
      break;
      } /* Save current ecosystem */
     case ID_EE_LOADALL:
      {
      if ((loadparams(0x0100, -1)) == 1)
       {
       FixPar(0, 0x0100);
       FixPar(1, 0x0100);
       } /* if load successful */
      UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
      GetKeyTableValues(2, EE_Win->EcoItem, 1);
      Set_EE_List(1);
      Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
      Set_EE_Item(EE_Win->EcoItem);
      ResetTimeLines(-1);
      DisableKeyButtons(2);
      break;
      } /* Load entire palette */
     case ID_EE_LOADCURRENT:
      {
      loadparams(0x0100, EE_Win->EcoItem);
      UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
      GetKeyTableValues(2, EE_Win->EcoItem, 1);
      Set_EE_List(1);
      Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
      Set_EE_Item(EE_Win->EcoItem);
      ResetTimeLines(-1);
      DisableKeyButtons(2);
      break;
      } /* Load current ecosystem */
     case ID_EE_APPLY:
      {
      Close_EE_Window(1);
      break;
      } /* Apply changes to Palette arrays */
     case ID_EE_CLOSE:
      {
      Close_EE_Window(0);
      break;
      } /* Close and cancel any changes since window opened */
     case ID_EE_CLOSEQUERY:
      {
      if (KFsize != EE_Win->AltKFsize || memcmp(KF, EE_Win->AltKF, KFsize)
		|| memcmp(&EcoPar, &UndoEcoPar[0], sizeof (EcoPar)))
       Close_EE_Window(CloseWindow_Query((STRPTR)GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ) ));  // "Ecosystem Editor"
      else
       Close_EE_Window(1);
      break;
      } /* Close and cancel any changes since window opened */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_LIST1:
    {
    LONG data;

    get(EE_Win->LS_List, MUIA_List_Active, &data);

    EE_Win->EcoItem = data;
    UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
    GetKeyTableValues(2, EE_Win->EcoItem, 0);
/* set mod frame button when it is created */
    Set_EE_Item(EE_Win->EcoItem);
    if (EETL_Win && (EE_Win->IsKey >= 0 || EE_Win->PrevKey >= 0 || EE_Win->NextKey >= 0))
     {
     if (Set_EETL_Item(EE_Win->EcoItem))
      {
      for (i=0; i<10; i++)
       {
       Set_EETL_Data(i);
       } /* for i=0... */
      MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWOBJECT);
      } /* if */
     } /* if Time Line window open */
    DisableKeyButtons(2);
    break;
    } /* ecosystem list */

   case GP_STRING1:
    {
    char *Name;

    get(EE_Win->NameStr, MUIA_String_Contents, &Name);
    strncpy(PAR_NAME_ECO(EE_Win->EcoItem), Name, 23);
    if (CountKeyFrames(2, EE_Win->EcoItem))
     {
     sprintf(EE_Win->Econame[EE_Win->EcoItem], "\0333%s", PAR_NAME_ECO(EE_Win->EcoItem));
     } /* if key frame */
    else
     {
     sprintf(EE_Win->Econame[EE_Win->EcoItem], "\33n%s", PAR_NAME_ECO(EE_Win->EcoItem));
     }
    Set_PS_List(EE_Win->EEList, NULL, 2, 0, (char*)GetString( MSG_EDECOGUI_UNUSED ) );  // "Unused"
    DoMethod(EE_Win->LS_List, MUIM_List_Redraw, EE_Win->EcoItem);
    break;
    } /* Ecosystem name string */

   case GP_STRING2:
    {
    LONG data;

    i = WCS_ID - ID_EE_FRAMESTR(0);
    get(EE_Win->Str[i], MUIA_String_Integer, &data);
    switch (i)
     {
     case 0:
      {
      EE_Win->Frame = data;
      if (EE_Win->Frame < 0)
       {
       EE_Win->Frame = 0;
       set(EE_Win->Str[0], MUIA_String_Integer, EE_Win->Frame);
       } /* if frame < 1 */

      UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 0);
      Set_EE_Item(EE_Win->EcoItem);
      GetKeyTableValues(2, EE_Win->EcoItem, 1);
      if (EETL_Win)
       {
       long data2;

       get(EETL_Win->Prop[2], MUIA_Prop_First, &data2);
       data = (100.0 * ((float)EE_Win->Frame / (float)EETL_Win->Frames));
       if (data != data2 && ! EE_Win->PropBlock)
        { 
        set(EETL_Win->Prop[2], MUIA_Prop_First, data);
        EE_Win->StrBlock = 1;
        } /* if */      
       EE_Win->PropBlock = 0;
       if (EE_Win->IsKey >= 0)
        {
        EETL_Win->ActiveKey = GetActiveKey(SKT[2], EE_Win->Frame);
        sprintf(str, "%d", EE_Win->Frame);
        set(EETL_Win->FrameTxt, MUIA_Text_Contents, (IPTR)str);
        MUI_Redraw(EETL_Win->TimeLineObj[EETL_Win->ActiveItem], MADF_DRAWUPDATE);
	} /* if key frame */
       } /* if time line window open */
      DisableKeyButtons(2);
      break;
      } /* frame counter */
     } /* switch i */
    break;
    } /* Frame strings */

   case GP_STRING3:
    {
    char *floatdata;
    double data;

    i = WCS_ID - ID_EE_INTSTR(0);
    get(EE_Win->IntStr[i], MUIA_String_Contents, &floatdata);
    data = atof(floatdata);
    if (data > EE_Win->EcoLimits[i][0])
     {
     data = EE_Win->EcoLimits[i][0];
     }
    else if (data < EE_Win->EcoLimits[i][1])
     {
     data = EE_Win->EcoLimits[i][1];
     }
    UnSet_EE_Item(i, data);
    if (EETL_Win && i == EETL_Win->ActiveItem)
     {
     if ((EE_Win->NextKey >= 0|| EE_Win->PrevKey>= 0) && (EE_Win->IsKey >= 0))
      {
      setfloat(EETL_Win->ValStr[0], data);
      } /* if key frame */
     } /* if time line window open */

    break;
    } /* STRING3 */

   case GP_STRING5:
    {
    char *name;

    get(EE_Win->ModelStr, MUIA_String_Contents, &name);
    strcpy(PAR_MODEL_ECO(EE_Win->EcoItem), name);
    break;
    } /* STRING5 - Model name */

   case GP_CYCLE1:
    {
    LONG data;

    get(EE_Win->ClassCycle, MUIA_Cycle_Active, &data);
    PAR_TYPE_ECO(EE_Win->EcoItem) = data
	 + (PAR_TYPE_ECO(EE_Win->EcoItem) & 0xff00);
    break;
    } /* CYCLE1 */

   case GP_CYCLE2:
    {
    LONG data;
    short color[3];

    i = WCS_ID - ID_EE_COLOR(0);

    get(EE_Win->ColorCy[i], MUIA_Cycle_Active, &data);

    if (i == 0)
     {
     PAR_COLR_ECO(EE_Win->EcoItem) = data;
     color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 0) / 16;
     color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 1) / 16;
     color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(EE_Win->EcoItem), 2) / 16;
     SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
     EE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
     } /* set color */
    else
     {
     PAR_UNDER_ECO(EE_Win->EcoItem) = data;
     color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 0) / 16;
     color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 1) / 16;
     color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(EE_Win->EcoItem)), 2) / 16;
     SetRGB4(&WCSScrn->ViewPort, 9, color[0], color[1], color[2]);
     EE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
     } /* set color */
    break;
    } /* color/understory cycles */

   case GP_CYCLE3:
    {
    LONG data, ProcFlags;

    ProcFlags = PAR_TYPE_ECO(EE_Win->EcoItem) & 0xff00;
    PAR_TYPE_ECO(EE_Win->EcoItem) -= ProcFlags;
    get(EE_Win->TexClassCycle, MUIA_Cycle_Active, &data);
    PAR_TYPE_ECO(EE_Win->EcoItem) %= 50;
    PAR_TYPE_ECO(EE_Win->EcoItem) += (50 * data + ProcFlags);
    set(EE_Win->TexPageGrp, MUIA_Group_ActivePage, data);
    if (data > 1)
     set(EE_Win->IntStr[9], MUIA_Disabled, TRUE);
    else
     set(EE_Win->IntStr[9], MUIA_Disabled, FALSE);
    if (data > 2)
     set(EE_Win->IntStr[8], MUIA_Disabled, TRUE);
    else
     set(EE_Win->IntStr[8], MUIA_Disabled, FALSE);
    break;
    } /* CYCLE1 */

   case GP_ARROW1:
    {
    char *floatdata;
    double data;

    i = WCS_ID - ID_EE_INTARROWLEFT(0);
    get(EE_Win->IntStr[i], MUIA_String_Contents, &floatdata);
    data = atof(floatdata);
    data -= max(1.0, (data * .1));
    if (data < EE_Win->EcoLimits[i][1])
     data = EE_Win->EcoLimits[i][1];
    else if (data > EE_Win->EcoLimits[i][0])
     data = EE_Win->EcoLimits[i][0];
    setfloat(EE_Win->IntStr[i], data);
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    char *floatdata;
    double data;

    i = WCS_ID - ID_EE_INTARROWRIGHT(0);
    get(EE_Win->IntStr[i], MUIA_String_Contents, &floatdata);
    data = atof(floatdata);
    data += max(1.0, (data * .1));
    if (data < EE_Win->EcoLimits[i][1])
     data = EE_Win->EcoLimits[i][1];
    else if (data > EE_Win->EcoLimits[i][0])
     data = EE_Win->EcoLimits[i][0];
    setfloat(EE_Win->IntStr[i], data);
    break;
    } /* ARROW2 */

   case GP_ARROW3:
    {
    LONG data;

    get(EE_Win->Str[0], MUIA_String_Integer, &data);
    set(EE_Win->Str[0], MUIA_String_Integer, (data > 0 ? data - 1: 0));
    break;
    } /* ARROW3 */

   case GP_ARROW4:
    {
    LONG data;

    get(EE_Win->Str[0], MUIA_String_Integer, &data);
    set(EE_Win->Str[0], MUIA_String_Integer, data + 1);
    break;
    } /* ARROW4 */

   case GP_BUTTONS2:
    {
    LONG data;

    i = WCS_ID - ID_EE_PROCEDURAL(0);
    get(EE_Win->ProcCheck[i], MUIA_Selected, &data);
    PAR_TYPE_ECO(EE_Win->EcoItem) ^= (1 << (i + 8));
    PAR_TYPE_ECO(EE_Win->EcoItem) |= (data << (i + 8));
    break;
    } /* BUTTONS2 */

   } /* switch gadget group */

} /* Handle_EE_Window() */

/*********************************************************************/

void Set_EE_Item(short item)
{
 short color[3], FoliageLimit[3];

 set(EE_Win->NameStr, MUIA_String_Contents, (IPTR)PAR_NAME_ECO(item));
 nnset(EE_Win->ClassCycle, MUIA_Cycle_Active, (PAR_TYPE_ECO(item) & 0x00ff) % 50);
 nnset(EE_Win->ProcCheck[0], MUIA_Selected, PAR_TYPE_ECO(item) & 0x0100);
 nnset(EE_Win->ProcCheck[1], MUIA_Selected, PAR_TYPE_ECO(item) & 0x0200);
 set(EE_Win->ColorCy[0], MUIA_Cycle_Active, PAR_COLR_ECO(item));
 set(EE_Win->ColorCy[1], MUIA_Cycle_Active, PAR_UNDER_ECO(item));
 setfloat(EE_Win->IntStr[0], PAR_FIRSTLN_ECO(item));
 setfloat(EE_Win->IntStr[1], PAR_FIRSTSK_ECO(item));
 setfloat(EE_Win->IntStr[2], PAR_FIRSTSA_ECO(item));
 setfloat(EE_Win->IntStr[3], PAR_FIRSTRE_ECO(item));
 setfloat(EE_Win->IntStr[4], PAR_FIRSTXR_ECO(item));
 setfloat(EE_Win->IntStr[5], PAR_FIRSTNR_ECO(item));
 setfloat(EE_Win->IntStr[6], PAR_FIRSTXS_ECO(item));
 setfloat(EE_Win->IntStr[7], PAR_FIRSTNS_ECO(item));
 setfloat(EE_Win->IntStr[8], PAR_FIRSTDN_ECO(item));
 setfloat(EE_Win->IntStr[9], PAR_FIRSTHT_ECO(item));
 set(EE_Win->IntStr[12], MUIA_String_Integer, PAR_MTCH_ECO(item, 0));
 set(EE_Win->IntStr[13], MUIA_String_Integer, PAR_MTCH_ECO(item, 1));
 set(EE_Win->IntStr[14], MUIA_String_Integer, PAR_MTCH_ECO(item, 2));

 color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(item), 0) / 16;
 color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(item), 1) / 16;
 color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(item), 2) / 16;
 SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
 EE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];

 color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(item)), 0) / 16;
 color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(item)), 1) / 16;
 color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(item)), 2) / 16;
 SetRGB4(&WCSScrn->ViewPort, 9, color[0], color[1], color[2]);
 EE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];

 set(EE_Win->ModelStr, MUIA_String_Contents, (IPTR)PAR_MODEL_ECO(item));
 GetFoliageLimits(EcoShift[item].Ecotype, FoliageLimit);

 settextint(EE_Win->TextureText[0], (long)FoliageLimit[0]);
 settextint(EE_Win->TextureText[1], (long)FoliageLimit[1]);
 settextint(EE_Win->TextureText[2], (long)FoliageLimit[2]);
 nnset(EE_Win->TexClassCycle, MUIA_Cycle_Active,
	(PAR_TYPE_ECO(item) & 0x00ff) / 50);
 nnset(EE_Win->TexPageGrp, MUIA_Group_ActivePage,
	(PAR_TYPE_ECO(item) & 0x00ff) / 50);
 if ((PAR_TYPE_ECO(item) & 0x00ff) / 50 > 1)
  set(EE_Win->IntStr[9], MUIA_Disabled, TRUE);
 else
  set(EE_Win->IntStr[9], MUIA_Disabled, FALSE);
 if ((PAR_TYPE_ECO(item) & 0x00ff) / 50 > 2)
  set(EE_Win->IntStr[8], MUIA_Disabled, TRUE);
 else
  set(EE_Win->IntStr[8], MUIA_Disabled, FALSE);

 if (item < 12)
  {
  set(EE_Win->BT_Swap, MUIA_Disabled, TRUE);
  set(EE_Win->BT_Insert, MUIA_Disabled, TRUE);
  set(EE_Win->BT_Remove, MUIA_Disabled, TRUE);
  set(EE_Win->NameStr, MUIA_Disabled, TRUE);
  }
 else
  {
  set(EE_Win->BT_Swap, MUIA_Disabled, FALSE);
  set(EE_Win->BT_Insert, MUIA_Disabled, FALSE);
  set(EE_Win->BT_Remove, MUIA_Disabled, FALSE);
  set(EE_Win->NameStr, MUIA_Disabled, FALSE);
  }
 if (item == 0)
  {
  set(EE_Win->Label[0], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_SEALEVEL ) );  // " Sea Level "
  set(EE_Win->Label[1], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_SEADEPTH ) );  // " Sea Depth "
  set(EE_Win->Label[2], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_WINDAZ ) );    // "   Wind Az "
  }
 else
  {
  set(EE_Win->Label[0], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_ELEVLINE ) );  // " Elev Line "
  set(EE_Win->Label[1], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_ELEVSKEW ) );  // " Elev Skew "
  set(EE_Win->Label[2], MUIA_Text_Contents, (IPTR)GetString( MSG_EDECOGUI_ELSKEWAZ ) );  // "El Skew Az "
  }

} /* Set_EE_Item() */

/*********************************************************************/

STATIC_FCN void UnSet_EE_Item(short item, double data) // used locally only -> static, AF 26.7.2021
{
 switch (item)
  {
  case 0:
   {
   PAR_FIRSTLN_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 1:
   {
   PAR_FIRSTSK_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 2:
   {
   PAR_FIRSTSA_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 3:
   {
   PAR_FIRSTRE_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 4:
   {
   PAR_FIRSTXR_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 5:
   {
   PAR_FIRSTNR_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 6:
   {
   PAR_FIRSTXS_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 7:
   {
   PAR_FIRSTNS_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 8:
   {
   PAR_FIRSTDN_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 9:
   {
   PAR_FIRSTHT_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 10:
   {
   PAR_COLR_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 11:
   {
   PAR_UNDER_ECO(EE_Win->EcoItem) = data;
   break;
   }
  case 12:
   {
   PAR_MTCH_ECO(EE_Win->EcoItem, 0) = data;
   break;
   }
  case 13:
   {
   PAR_MTCH_ECO(EE_Win->EcoItem, 1) = data;
   break;
   }
  case 14:
   {
   PAR_MTCH_ECO(EE_Win->EcoItem, 2) = data;
   break;
   }
  } /* switch item */

} /* UnSet_EE_Item() */

/*********************************************************************/

void Set_EE_List(short update)
{
 short i;

 for (i=0; i<ECOPARAMS; i++)
  {
  if (CountKeyFrames(2, i))
   sprintf(EE_Win->Econame[i], "\0333%s", PAR_NAME_ECO(i));
  else
   sprintf(EE_Win->Econame[i], "\33n%s", PAR_NAME_ECO(i));
  EE_Win->EName[i] = &EE_Win->Econame[i][0];
  } /* for i=0... */
 EE_Win->EName[ECOPARAMS] = NULL;

/* Add items or update Ecosystem list */
 if (update)
  {
  DoMethod(EE_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(EE_Win->LS_List,
	MUIM_List_Insert, &EE_Win->EName, -1, MUIV_List_Insert_Bottom);
  }

} /* Set_EE_List() */

/*********************************************************************/

STATIC_FCN void Set_EcoLimits(void) // used locally only -> static, AF 26.7.2021
{
 short Limits[15][2] = {
	{32000,	 -32000},
	{32000,	 -32000},
	{360,	 -360},
	{25,	 -25},
	{20000,	 -20000},
	{20000,	 -20000},
	{91,	 0},
	{91,	 0},
	{100,	 0},
	{500,	 0},
	{COLORPARAMS - 1, 0},
	{ECOPARAMS - 1,	 0},
	{255,	 0},
	{255,	 0},
	{255,	 0}
 };
 
 memcpy(&EE_Win->EcoLimits[0][0], &Limits, 60);

} /* Set_EcoLimits() */

/***********************************************************************/

short SearchEcosystemColorMatch(short Color)
{
short i;

 for (i=0; i<ECOPARAMS; i++)
  {
  if (Color == PAR_COLR_ECO(i))
   return (i);
  if (SearchEcotypeColorMatch(EcoShift[i].Ecotype, Color))
   return (i);
  } /* for i=0... */

 return (-1);

} /* SearchEcosystemColorMatch() */

/***********************************************************************/

void AdjustEcosystemColors(short Operation, short First, short Last)
{
short i;

 for (i=0; i<ECOPARAMS; i++)
  {
  switch (Operation)
   {
   case WCS_ECOCOLOR_DECREMENT:
    {
    if (PAR_COLR_ECO(i) >= First)
         PAR_COLR_ECO(i) --;
    break;
    }
   case WCS_ECOCOLOR_INCREMENT:
    {
    if (PAR_COLR_ECO(i) >= First && PAR_COLR_ECO(i) < COLORPARAMS - 1)
     PAR_COLR_ECO(i) ++;
    break;
    }
   case WCS_ECOCOLOR_SWAP:
    {
    if (PAR_COLR_ECO(i) == Last)
     PAR_COLR_ECO(i) = First;
    else if (PAR_COLR_ECO(i) == First)
     PAR_COLR_ECO(i) = Last;
    break;
    }
   } /* switch */
  AdjustFoliageColors(EcoShift[i].Ecotype, Operation, First, Last);
  if (EE_Win)
   {
   if (i == EE_Win->EcoItem)
    Set_EE_Item(i);
   } /* if */
  } /* for i=0... */

} /* AdjustEcosystemColors() */


