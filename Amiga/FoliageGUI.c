/* FoliageGUI.c
** World Construction Set GUI for Foliage Editing module.
** By Gary Huber 12/95.
** Copyright 1995 Questar Productions.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Foliage.h"


STATIC_FCN void GUIFoliageGroup_SetGads(struct FoliageGroup *This); // used locally only -> static, AF 19.7.2021
STATIC_FCN short SearchColorListMatch(char *Name); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliage_Remove(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliageGroup_Add(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN void GUIFoliage_SetGads(struct Foliage *This); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIEcotype_Load(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN void EcotypeDefaultColors(struct Ecotype *Eco, short Color); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIImagePath(struct Foliage *Fol); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliageGroup_New(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliage_Add(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliage_View(struct Foliage *Fol, UBYTE **Bitmap); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliageGroup_Remove(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN void BuildFoliageList(struct FoliageGroup *FolGp); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIEcotype_Save(void); // used locally only -> static, AF 19.7.2021
STATIC_FCN void DisableColorChecks(struct FoliageGroup *FolGp, struct Foliage *Fol); // used locally only -> static, AF 19.7.2021
STATIC_FCN void BuildFoliageGroupList(struct Ecotype *Eco); // used locally only -> static, AF 19.7.2021
STATIC_FCN short GUIFoliageGroup_Save(void); // used locally only -> static, AF 19.7.202
STATIC_FCN void FoliageGroupDefaultColors(struct FoliageGroup *FolGp, short Color); // used locally only -> static, AF 19.7.2021
STATIC_FCN void FoliageDefaultColors(struct Foliage *Fol, short Color); // used locally only -> static, AF 19.7.2021

STATIC_FCN void Make_FE_Window(void) // used locally only -> static, AF 19.7.2021
{
 long i, open;

 if (FE_Win)
  {
  DoMethod(FE_Win->FoliageWin, MUIM_Window_ToFront);
  set(FE_Win->FoliageWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &FE_Win->Colors[0], 16);
  return;
  } /* if window already exists */

 if ((FE_Win = (struct FoliageWindow *)
	get_Memory(sizeof (struct FoliageWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(2);

 FE_Win->ECListSize = (COLORPARAMS + 1) * (sizeof (char *));
 FE_Win->GroupListSize = 100 * sizeof (char *);
 FE_Win->ImageListSize = 100 * sizeof (char *);
 FE_Win->ECList = (char **)get_Memory(FE_Win->ECListSize, MEMF_CLEAR);
 FE_Win->GroupList = (char **)get_Memory(FE_Win->GroupListSize, MEMF_CLEAR);
 FE_Win->GrpAddrList = (struct FoliageGroup **)
	get_Memory(FE_Win->GroupListSize, MEMF_CLEAR);
 FE_Win->ImageList = (char **)get_Memory(FE_Win->ImageListSize, MEMF_CLEAR);
 FE_Win->ImgAddrList = (struct Foliage **)
	get_Memory(FE_Win->ImageListSize, MEMF_CLEAR);
 FE_Win->GroupEntries = FE_Win->ImageEntries = 0;
 FE_Win->CurrentGroup = FE_Win->CurrentImage = -1;
 FE_Win->CurGrp = NULL;
 FE_Win->CurImg = NULL;

 if (! FE_Win->ECList)
  {
  User_Message((CONST_STRPTR)"Parameters Module: Foliage",
		  (CONST_STRPTR)"Out of memory!\nCan't open Foliage Editor.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_FE_Window(1);
  return;
  } /* if out of memory */

 Set_PS_List(FE_Win->ECList, NULL, 1, 0, "Unused");

 FE_Win->FolEco = EE_Win->EcoItem;

 if (EcoShift[FE_Win->FolEco].Ecotype)
  {
  if ((FE_Win->Backup = Ecotype_Copy(EcoShift[FE_Win->FolEco].Ecotype)) == NULL)
   {
   User_Message((CONST_STRPTR)"Parameters Module: Foliage",
		   (CONST_STRPTR)"Out of memory!\nCan't open Foliage Editor.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Close_FE_Window(1);
   return;
   } /* if out of memory */
  } /* if */
  

     FE_Win->FoliageWin = WindowObject,
      MUIA_Window_Title		, "Foliage Editor",
      MUIA_Window_ID		, MakeID('E','D','F','O'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label1("Ecosystem"),
	  Child, FE_Win->EcosysText = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345",
		MUIA_Text_Contents, PAR_NAME_ECO(FE_Win->FolEco), End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, VGroup,
	    Child, Label("\33c\0334Group"),
            Child, FE_Win->LS_GroupList = ListviewObject,
		MUIA_Listview_Input, TRUE,
               	MUIA_Listview_List, ListObject, ReadListFrame, End,
              End, /* ListviewObject */
	    Child, FE_Win->BT_Suggest = KeyButtonFunc('s', "\33cSuggest..."),
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Max Pic Ht "),
	      Child, FE_Win->FloatStr[4] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[4][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[4][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2(" Density % "),
	      Child, FE_Win->FloatStr[0] = StringObject, StringFrame,
		MUIA_String_Accept, ".0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[0][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[0][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("  Height % "),
	      Child, FE_Win->FloatStr[1] = StringObject, StringFrame,
		MUIA_String_Accept, ".0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[1][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[1][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("Use Image Colors"),
	      Child, FE_Win->Check[0] = CheckMark(0),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, SmallImageDisplay(&EC_Button8),
	      Child, FE_Win->PalColCy[0] = CycleObject,
			MUIA_Cycle_Entries, FE_Win->ECList, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, FE_Win->BT_AddGroup = KeyButtonFunc('a', "\33cAdd..."),
	      Child, FE_Win->BT_RemoveGroup = KeyButtonFunc('v', "\33cRemove"),
	      Child, FE_Win->BT_NewGroup = KeyButtonFunc('n', "\33cNew..."),
	      End, /* HGroup */
	    Child, FE_Win->BT_Export = KeyButtonFunc('e', "\33cExport..."),
	    End, /* VGroup */

	  Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,

	  Child, VGroup,
	    Child, Label("\33c\0334Images"),
            Child, FE_Win->LS_ImageList = ListviewObject,
		MUIA_Listview_Input, TRUE,
               	MUIA_Listview_List, ListObject, ReadListFrame, End,
              End, /* ListviewObject */
	    Child, HGroup,
	      Child, Label1("Width"),
	      Child, FE_Win->WidthText = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123", End,
	      Child, Label1("Ht"),
	      Child, FE_Win->HeightText = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123", End,
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Max Pic Ht "),
	      Child, FE_Win->FloatStr[5] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[5][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[5][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2(" Density % "),
	      Child, FE_Win->FloatStr[2] = StringObject, StringFrame,
		MUIA_String_Accept, ".0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[2][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[2][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("  Height % "),
	      Child, FE_Win->FloatStr[3] = StringObject, StringFrame,
		MUIA_String_Accept, ".0123456789",
		MUIA_FixWidthTxt, "012345", End,
	      Child, FE_Win->Arrow[3][0] = ImageButton(MUII_ArrowLeft),
	      Child, FE_Win->Arrow[3][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("Use Image Colors"),
	      Child, FE_Win->Check[1] = CheckMark(0),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, SmallImageDisplay(&EC_Button9),
	      Child, FE_Win->PalColCy[1] = CycleObject,
			MUIA_Cycle_Entries, FE_Win->ECList, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, FE_Win->BT_AddImage = KeyButtonFunc('d', "\33cAdd..."),
	      Child, FE_Win->BT_RemoveImage = KeyButtonFunc('r', "\33cRemove"),
	      End, /* HGroup */
	    Child, FE_Win->BT_ViewImage = KeyButtonFunc('v', "\33cView..."),
	    End, /* VGroup */
	  End, /* HGroup */
	Child, HGroup, MUIA_Group_SameWidth, TRUE,
	  Child, FE_Win->BT_Keep = KeyButtonFunc('k', "\33cKeep"),
	  Child, FE_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"),
	  End, /* HGroup */
	End, /* VGroup */
      End; /* WindowObject FE_Win->FoliageWin */

  if (! FE_Win->FoliageWin)
   {
   Close_FE_Window(1);
   User_Message((CONST_STRPTR)"Foliage Editor", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, FE_Win->FoliageWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* copy color palettes */
 memcpy(&FE_Win->Colors[0], &EE_Win->Colors[0], sizeof (FE_Win->Colors));

/* ReturnIDs */
  DoMethod(FE_Win->FoliageWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_FE_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, FE_Win->BT_Keep, ID_FE_APPLY,
   FE_Win->BT_Cancel, ID_FE_CLOSE, FE_Win->BT_Suggest, ID_FE_SUGGEST,
   FE_Win->BT_AddGroup, ID_FE_ADDGROUP, FE_Win->BT_NewGroup, ID_FE_NEWGROUP,
   FE_Win->BT_Export, ID_FE_EXPORT, FE_Win->BT_AddImage, ID_FE_ADDIMAGE,
   FE_Win->BT_RemoveImage, ID_FE_REMOVEIMAGE,
   FE_Win->BT_RemoveGroup, ID_FE_REMOVEGROUP,
   FE_Win->BT_ViewImage, ID_FE_VIEWIMAGE, NULL);

  DoMethod(FE_Win->FoliageWin, MUIM_Notify, MUIA_Window_InputEvent,
	"numericpad enter", app, 2, MUIM_Application_ReturnID, ID_FE_EXPORTECO);
  DoMethod(FE_Win->FoliageWin, MUIM_Notify, MUIA_Window_InputEvent,
	"help", app, 2, MUIM_Application_ReturnID, ID_FE_IMAGEPATH);

/* Link arrow buttons to application */
  for (i=0; i<6; i++)
   MUI_DoNotiPresFal(app, FE_Win->Arrow[i][0], ID_FE_ARROWLEFT(i),
    FE_Win->Arrow[i][1], ID_FE_ARROWRIGHT(i), NULL);

/* Link strings to application */
  for (i=0; i<6; i++)
   DoMethod(FE_Win->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_FE_FLOATSTR(i));

/* link palette cycles to app */
  for (i=0; i<2; i++)
   DoMethod(FE_Win->PalColCy[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_FE_PALCOLCYCLE(i));

/* link check boxes to app */
  for (i=0; i<2; i++)
   DoMethod(FE_Win->Check[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_FE_CHECK(i));

/* link lists to application */
  DoMethod(FE_Win->LS_GroupList, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_FE_GROUPLIST);
  DoMethod(FE_Win->LS_GroupList, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_FE_GROUPLISTDBCLICK);
  DoMethod(FE_Win->LS_ImageList, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_FE_IMAGELIST);
  DoMethod(FE_Win->LS_ImageList, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_FE_IMAGELISTDBCLICK);

/* build foliage group list */

 BuildFoliageGroupList(EcoShift[FE_Win->FolEco].Ecotype);
 DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);

/* Open window */
  set(FE_Win->FoliageWin, MUIA_Window_Open, TRUE);
  get(FE_Win->FoliageWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_FE_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(FE_Win->FoliageWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_FE_ACTIVATE);

/* Get Window structure pointer */
  get(FE_Win->FoliageWin, MUIA_Window_Window, &FE_Win->Win);

} /* Make_FE_Window() */

/*********************************************************************/

void Close_FE_Window(short apply)
{
 if (FE_Win)
  {
  if (FE_Win->FoliageWin)
   {
   if (apply)
    {
    Ecotype_Del(FE_Win->Backup);
    } /* if */
   else
    {
    Ecotype_Del(EcoShift[FE_Win->FolEco].Ecotype);
    EcoShift[FE_Win->FolEco].Ecotype = FE_Win->Backup;
    } /* else cancel */
   set(FE_Win->FoliageWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
   DoMethod(app, OM_REMMEMBER, FE_Win->FoliageWin);
   MUI_DisposeObject(FE_Win->FoliageWin);
   } /* if window created */
  if (FE_Win->ECList) free_Memory(FE_Win->ECList, FE_Win->ECListSize);
  if (FE_Win->GroupList) free_Memory(FE_Win->GroupList, FE_Win->GroupListSize);
  if (FE_Win->GrpAddrList) free_Memory(FE_Win->GrpAddrList, FE_Win->GroupListSize);
  if (FE_Win->ImageList) free_Memory(FE_Win->ImageList, FE_Win->ImageListSize);
  if (FE_Win->ImgAddrList) free_Memory(FE_Win->ImgAddrList, FE_Win->ImageListSize);
  free_Memory(FE_Win, sizeof (struct FoliageWindow));
  FE_Win = NULL;
  } /* if */

} /* Close_FE_Window() */

/*********************************************************************/

void Handle_FE_Window(ULONG WCS_ID)
{
short i;
long data;
char *FloatData;
double FloatVal;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_FE_Window();
   return;
   } /* Open Foliage Editor Window */

  if (! FE_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &FE_Win->Colors[0], 16);
    break;
    } /* Activate Settings Editing Window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_FE_APPLY:
      {
      Close_FE_Window(1);
      break;
      } /* Close window */
     case ID_FE_CLOSE:
      {
      Close_FE_Window(0);
      break;
      } /* Close window */
     case ID_FE_CLOSEQUERY:
      {
      short apply;

      apply = FE_Win->Mod && User_Message_Def((CONST_STRPTR)"Foliage Editor", (CONST_STRPTR)"Keep changes?", (CONST_STRPTR)"Yes|No", (CONST_STRPTR)"yn", 1);
      Close_FE_Window(apply);
      break;
      } /* Close window */

     case ID_FE_SUGGEST:
      {
      GUIEcotype_Load();
      break;
      }
     case ID_FE_ADDGROUP:
      {
      GUIFoliageGroup_Add();
      break;
      }
     case ID_FE_REMOVEGROUP:
      {
      GUIFoliageGroup_Remove();
      break;
      }
     case ID_FE_NEWGROUP:
      {
      GUIFoliageGroup_New();
      break;
      }
     case ID_FE_EXPORT:
      {
      GUIFoliageGroup_Save();
      break;
      }
     case ID_FE_ADDIMAGE:
      {
      GUIFoliage_Add();
      break;
      }
     case ID_FE_REMOVEIMAGE:
      {
      GUIFoliage_Remove();
      break;
      }
     case ID_FE_VIEWIMAGE:
      {
      UBYTE *Bitmap[3];
      short Width, Height, Planes;

      SetPointer(FE_Win->Win, WaitPointer, 16, 16, -6, 0);
      Bitmap[0] = Bitmap[1] = Bitmap[2] = NULL;
      if (LoadImage(FE_Win->CurImg->Root.Name, 1, Bitmap, 0, 0, 0, &Width,
		&Height, &Planes))
       {
       GUIFoliage_View(FE_Win->CurImg, Bitmap);
       } /* if image loaded */
      else
       User_Message_Def((CONST_STRPTR)"Foliage Editor: View Image",
    		   (CONST_STRPTR)"Unable to load image file for viewing!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
      if (Bitmap[0])
       free_Memory(Bitmap[0], Width * Height);
      if (Bitmap[1])
       free_Memory(Bitmap[1], Width * Height);
      if (Bitmap[2])
       free_Memory(Bitmap[2], Width * Height);
      ClearPointer(FE_Win->Win);
      break;
      }
     case ID_FE_EXPORTECO:
      {
      GUIEcotype_Save();
      break;
      }
     case ID_FE_IMAGEPATH:
      {
      if (GUIImagePath(FE_Win->CurImg))
       BuildFoliageList(FE_Win->CurGrp);
      break;
      }
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    i = WCS_ID -ID_FE_FLOATSTR(0);

    get(FE_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    if (FloatVal < 0.0)
     FloatVal = 0.0;

    switch (i)
     {
     case 0:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_DENSITY, 
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 1:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 2:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_DENSITY,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 3:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 4:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     case 5:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     } /* switch */
    FE_Win->Mod = 1;
    break;
    } /* STRING1 */

   case GP_ARROW1:
    {
    i = WCS_ID -ID_FE_ARROWLEFT(0);

    get(FE_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal -= 10.0;
    if (FloatVal < 0.0)
     FloatVal = 0.0;
    setfloat(FE_Win->FloatStr[i], FloatVal);

    switch (i)
     {
     case 0:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_DENSITY,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 1:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 2:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_DENSITY,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 3:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 4:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     case 5:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     } /* switch */

    FE_Win->Mod = 1;
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    i = WCS_ID - ID_FE_ARROWRIGHT(0);

    get(FE_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal += 10.0;
    setfloat(FE_Win->FloatStr[i], FloatVal);
     
    switch (i)
     {
     case 0:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_DENSITY,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 1:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 2:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_DENSITY,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 3:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_HEIGHT,
	(float)(FloatVal / 100.0), 0);
      break;
      }
     case 4:
      {
      Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     case 5:
      {
      Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_MAXIMGHT,
	(float)0.0, (short)FloatVal);
      break;
      }
     } /* switch */

    FE_Win->Mod = 1;
    break;
    } /*ARROW2 */

   case GP_CYCLE1:
    {
    short color[3];

    i = WCS_ID - ID_FE_PALCOLCYCLE(0);
    get(FE_Win->PalColCy[i], MUIA_Cycle_Active, &data);

    switch (i)
     {
     case 0:
      {
      if (FE_Win->CurGrp)
       {
       Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, data);
       Rootstock_SetColorName(&FE_Win->CurGrp->Root, PAR_NAME_COLOR(data));
       color[0] = PAR_FIRST_COLOR(data, 0) / 16;
       color[1] = PAR_FIRST_COLOR(data, 1) / 16;
       color[2] = PAR_FIRST_COLOR(data, 2) / 16;
       SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
       FE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
       }
      break;
      }
     case 1:
      {
      if (FE_Win->CurImg)
       {
       Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_PALCOL, (float)0.0, data);
       Rootstock_SetColorName(&FE_Win->CurImg->Root, PAR_NAME_COLOR(data));
       color[0] = PAR_FIRST_COLOR(data, 0) / 16;
       color[1] = PAR_FIRST_COLOR(data, 1) / 16;
       color[2] = PAR_FIRST_COLOR(data, 2) / 16;
       SetRGB4(&WCSScrn->ViewPort, 9, color[0], color[1], color[2]);
       FE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
       }
      break;
      }
     } /* switch */

    FE_Win->Mod = 1;
    break;
    } /* CYCLE1 */

   case GP_BUTTONS2:
    {
    i = WCS_ID - ID_FE_CHECK(0);
    get(FE_Win->Check[i], MUIA_Selected, &data);

    switch (i)
     {
     case 0:
      {
      if (FE_Win->CurGrp)
       {
       Rootstock_SetValue(&FE_Win->CurGrp->Root, WCS_ECOTYPE_USEIMGCOL, (float)0.0, data);
       DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
       }
      break;
      }
     case 1:
      {
      if (FE_Win->CurImg)
       {
       Rootstock_SetValue(&FE_Win->CurImg->Root, WCS_ECOTYPE_USEIMGCOL, (float)0.0, data);
       DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
       }
      break;
      }
     } /* switch */

    FE_Win->Mod = 1;
    break;
    } /* BUTTONS2 */

   case GP_LIST1:
    {
    get(FE_Win->LS_GroupList, MUIA_List_Active, &data);
    FE_Win->CurrentGroup = data;
    FE_Win->CurGrp = FE_Win->GrpAddrList[data];
    GUIFoliageGroup_SetGads(FE_Win->CurGrp);
    BuildFoliageList(FE_Win->CurGrp);
    DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
    break;
    } /* ID_FE_GROUPLIST */

   case GP_LIST2:
    {
    break;
    } /* ID_FE_GROUPLISTDBCLICK */

   case GP_LIST3:
    {
    get(FE_Win->LS_ImageList, MUIA_List_Active, &data);
    FE_Win->CurrentImage = data;
    FE_Win->CurImg = FE_Win->ImgAddrList[data];
    GUIFoliage_SetGads(FE_Win->CurImg);
    DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
    break;
    } /* ID_FE_IMAGELIST */

   case GP_LIST4:
    {
    break;
    } /* ID_FE_IMAGELISTDBCLICK */

   } /* switch gadget group */

} /* Handle_FE_Window() */

/***********************************************************************/

STATIC_FCN short GUIEcotype_Load(void) // used locally only -> static, AF 19.7.2021
{
char filename[256], Path[256], Name[32], Ptrn[32];
FILE *ffile;
short success = 0;

 strcpy(Path, imagepath);
 Name[0] = 0;
 strcpy(Ptrn, "#?.etp");
 if (getfilenameptrn(0, "Select an Ecotype", Path, Name, Ptrn))
  {
  strmfp(filename, Path, Name);
  if ((ffile = fopen(filename, "rb")))
   {
   if (EcoShift[FE_Win->FolEco].Ecotype)
    Ecotype_Del(EcoShift[FE_Win->FolEco].Ecotype);
   GUIList_Clear(FE_Win->GroupList, FE_Win->GroupListSize, FE_Win->LS_GroupList);
   GUIList_Clear(FE_Win->ImageList, FE_Win->ImageListSize, FE_Win->LS_ImageList);

   if ((EcoShift[FE_Win->FolEco].Ecotype = Ecotype_Load(ffile, 0)))
    {
    EcotypeDefaultColors(EcoShift[FE_Win->FolEco].Ecotype, PAR_COLR_ECO(FE_Win->FolEco));
    BuildFoliageGroupList(EcoShift[FE_Win->FolEco].Ecotype);
    DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
    success = 1;
    }
   else
    User_Message_Def((CONST_STRPTR)"Foliage Editor: Load Ecotype", (CONST_STRPTR)"Error loading Ecotype file!\nOperation terminated.",
    		(CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   fclose(ffile);
   FE_Win->Mod = 1;
   } /* if file opened */
  } /* if file name */

 return (success);

} /* GUIEcotype_Load() */

/***********************************************************************/

STATIC_FCN short GUIFoliageGroup_Add(void) // used locally only -> static, AF 19.7.2021
{
char filename[256], Path[256], Name[32], Ptrn[32], **NameAddr;
FILE *ffile;
short success = 0;
struct FoliageGroup **NewGroupAddr, *NewGroup;

 strcpy(Path, imagepath);
 Name[0] = 0;
 strcpy(Ptrn, "#?.fgp");
 if (getfilenameptrn(0, "Select a Foliage Group", Path, Name, Ptrn))
  {
  strmfp(filename, Path, Name);
  if ((ffile = fopen(filename, "rb")))
   {
   if (! EcoShift[FE_Win->FolEco].Ecotype)
    {
    if ((EcoShift[FE_Win->FolEco].Ecotype = Ecotype_New()) == NULL)
     {
     User_Message_Def((CONST_STRPTR)"Foliage Editor: Add Group",
    		 (CONST_STRPTR)"Out of memory allocating new group!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
     fclose(ffile);
     return (0);
     } /* if no memory */
    Ecotype_SetDefaults(EcoShift[FE_Win->FolEco].Ecotype, &EcoPar.en[FE_Win->FolEco]);
    } /* if no ecotype structure */

   NewGroupAddr = &EcoShift[FE_Win->FolEco].Ecotype->FolGp;
   NewGroup = *NewGroupAddr;

   while (NewGroup)
    {
    NewGroupAddr = &NewGroup->Next;
    NewGroup = NewGroup->Next;
    } /* while */

   if ((*NewGroupAddr = FoliageGroup_Load(ffile, 0)))
    {
    NewGroup = *NewGroupAddr;
    FoliageGroupDefaultColors(NewGroup, PAR_COLR_ECO(FE_Win->FolEco));
    GUIList_Clear(FE_Win->ImageList, FE_Win->ImageListSize, FE_Win->LS_ImageList);
    FE_Win->CurrentGroup = FE_Win->GroupEntries;
    FE_Win->GroupEntries ++;
    FE_Win->CurGrp = NewGroup;
    FE_Win->GrpAddrList[FE_Win->CurrentGroup] = NewGroup;
    NameAddr = Rootstock_GetNameAddr(&FE_Win->CurGrp->Root);
    DoMethod(FE_Win->LS_GroupList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
    nnset(FE_Win->LS_GroupList, MUIA_List_Active, FE_Win->CurrentGroup);
    GUIFoliageGroup_SetGads(FE_Win->CurGrp);
    BuildFoliageList(FE_Win->CurGrp);
    DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
    success = 1;
    FE_Win->Mod = 1;
    } /* if new group created */
   else
    User_Message_Def((CONST_STRPTR)"Foliage Editor: Add Group", (CONST_STRPTR)"Error loading Foliage Group file!\nOperation terminated.",
    		(CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   fclose(ffile);
   } /* if file opened */
  } /* if file name */

 return (success);

} /* GUIFoliageGroup_Add() */

/***********************************************************************/

STATIC_FCN short GUIFoliageGroup_New(void) // used locally only -> static, AF 19.7.2021
{
char Name[64], **NameAddr;
struct Ecotype *Prototype;
struct FoliageGroup **NewGroupAddr, *NewGroup;

 Name[0] = 0;
 if (! GetInputString("Enter new group name.", ":;*/?`#%", Name))
  return (0);

 if (! EcoShift[FE_Win->FolEco].Ecotype)
  {
  if ((EcoShift[FE_Win->FolEco].Ecotype = Ecotype_New()) == NULL)
   {
   User_Message_Def((CONST_STRPTR)"Foliage Editor: New Group",
		   (CONST_STRPTR)"Out of memory allocating new group!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   return (0);
   } /* if no memory */
  Ecotype_SetDefaults(EcoShift[FE_Win->FolEco].Ecotype, &EcoPar.en[FE_Win->FolEco]);
  } /* if no ecotype structure */

 Prototype = EcoShift[FE_Win->FolEco].Ecotype;

 NewGroupAddr = &EcoShift[FE_Win->FolEco].Ecotype->FolGp;
 NewGroup = *NewGroupAddr;

 while (NewGroup)
  {
  NewGroupAddr = &NewGroup->Next;
  NewGroup = NewGroup->Next;
  } /* while */

 if ((*NewGroupAddr = FoliageGroup_New()))
  {
  NewGroup = *NewGroupAddr;
  GUIList_Clear(FE_Win->ImageList, FE_Win->ImageListSize, FE_Win->LS_ImageList);
  FoliageGroup_SetDefaults(NewGroup, Prototype);
  Rootstock_SetName(&NewGroup->Root, Name);
  FE_Win->CurrentGroup = FE_Win->GroupEntries;
  FE_Win->GroupEntries ++;
  FE_Win->CurrentImage = -1;
  FE_Win->ImageEntries = 0;
  FE_Win->CurGrp = NewGroup;
  FE_Win->GrpAddrList[FE_Win->CurrentGroup] = NewGroup;
  FE_Win->CurImg = NULL;
  NameAddr = Rootstock_GetNameAddr(&FE_Win->CurGrp->Root);
  DoMethod(FE_Win->LS_GroupList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
  nnset(FE_Win->LS_GroupList, MUIA_List_Active, FE_Win->CurrentGroup);
  GUIFoliageGroup_SetGads(NewGroup);
  DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
  FE_Win->Mod = 1;
  return (1);
  } /* if new group created */
 else
  User_Message_Def((CONST_STRPTR)"Foliage Editor: New Group",
		  (CONST_STRPTR)"Out of memory allocating new group!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);

 return (0);

} /* GUIFoliageGroup_New() */

/***********************************************************************/

STATIC_FCN short GUIFoliageGroup_Save(void) // used locally only -> static, AF 19.7.2021
{
char filename[256], Path[256], Name[32], Ptrn[32];
FILE *ffile;
short success = 0;

 if (FE_Win->CurGrp)
  {
  strcpy(Path, imagepath);
  Name[0] = 0;
  strcpy(Ptrn, "#?.fgp");
  if (getfilenameptrn(1, "Ecotype Save Path/File", Path, Name, Ptrn))
   {
   if (strcmp(&Name[strlen(Name) - 4], ".fgp"))
    strcat(Name, ".fgp");
   strmfp(filename, Path, Name);
   if ((ffile = fopen(filename, "wb")))
    {
    if (FoliageGroup_Save(FE_Win->CurGrp, ffile))
     success = 1;
    else
     User_Message_Def((CONST_STRPTR)"Foliage Editor: Save Group", (CONST_STRPTR)"Error saving Foliage Group file!\nOperation terminated.",
    		 (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
    fclose(ffile);
    } /* if file opened */
   } /* if file name */
  } /* if */

 return (success);

} /* GUIFoliageGroup_Save() */

/***********************************************************************/

STATIC_FCN short GUIFoliage_Add(void) // used locally only -> static, AF 19.7.2021
{
char filename[256], Path[256], Name[32], **NameAddr;
struct Foliage *NewFol, **NewFolAddr;
short Width, Height, Planes, success = 0;
UBYTE *Bitmap[3];

 if (! FE_Win->CurGrp)
  return (0);

 strcpy(Path, imagepath);
 Name[0] = 0;
 if (getfilename(0, "Image Path/File", Path, Name))
  {
  Bitmap[0] = Bitmap[1] = Bitmap[2] = NULL;
  strmfp(filename, Path, Name);
  SetPointer(FE_Win->Win, WaitPointer, 16, 16, -6, 0);
  if (LoadImage(filename, 1, Bitmap, 0, 0, 0, &Width, &Height, &Planes))
   {
   NewFolAddr = &FE_Win->CurGrp->Fol;
   NewFol = *NewFolAddr;
   while (NewFol)
    {
    NewFolAddr = &NewFol->Next;
    NewFol = NewFol->Next;
    } /* while */

   if ((*NewFolAddr = Foliage_New()))
    {
    NewFol = *NewFolAddr;
    Foliage_SetDefaults(NewFol, FE_Win->CurGrp);
    Rootstock_SetName(&NewFol->Root, filename);
    Foliage_SetImgSize(NewFol, WCS_ECOTYPE_IMGWIDTH, Width);
    Foliage_SetImgSize(NewFol, WCS_ECOTYPE_IMGHEIGHT, Height);
    Foliage_SetImgSize(NewFol, WCS_ECOTYPE_COLORIMAGE, (short)(Planes == 24));
    FE_Win->CurrentImage = FE_Win->ImageEntries;
    FE_Win->ImageEntries ++;
    FE_Win->CurImg = NewFol;
    FE_Win->ImgAddrList[FE_Win->CurrentImage] = NewFol;
    NameAddr = Foliage_GetImageNameAddr(FE_Win->CurImg);
    DoMethod(FE_Win->LS_ImageList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
    nnset(FE_Win->LS_ImageList, MUIA_List_Active, FE_Win->CurrentImage);
    GUIFoliage_SetGads(NewFol);
    DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);
    FE_Win->Mod = 1;
    success= 1;
    } /* if new image created */
   else
    User_Message_Def((CONST_STRPTR)"Foliage Editor: Add Image",
    		(CONST_STRPTR)"Out of memory allocating new group!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   } /* if image loaded */
  else
   User_Message_Def((CONST_STRPTR)"Foliage Editor: Add Image",
		   (CONST_STRPTR)"Error loading image file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
  if (Bitmap[0])
   free_Memory(Bitmap[0], Width * Height);
  if (Bitmap[1])
   free_Memory(Bitmap[1], Width * Height);
  if (Bitmap[2])
   free_Memory(Bitmap[2], Width * Height);
  ClearPointer(FE_Win->Win);
  } /* if file name */

 return (success);

} /* GUIFoliage_Add() */

/***********************************************************************/

STATIC_FCN short GUIImagePath(struct Foliage *Fol) // used locally only -> static, AF 19.7.2021
{
char NewPath[256];

 if (Fol)
  {
  strcpy(NewPath, Rootstock_GetName(&Fol->Root));

  if (GetInputString("Enter new image path and name.",
	 "!@#$%&*()=", NewPath))
   {
   Rootstock_SetName(&Fol->Root, NewPath);
   return (1);
   } /* if */
  } /* if */

 return (0);

} /* GUIImagePath() */

/***********************************************************************/

STATIC_FCN short GUIFoliage_Remove(void) // used locally only -> static, AF 19.7.2021
{
struct Foliage *Fol, *FolPrev = NULL;

 Fol = FE_Win->CurGrp->Fol;

 if (Fol)
  {
  while (Fol && Fol != FE_Win->CurImg)
   {
   FolPrev = Fol;
   Fol = Fol->Next;
   } /* while */

  if (FolPrev && Fol)
   FolPrev->Next = Fol->Next;
  else if (Fol)
   FE_Win->CurGrp->Fol = Fol->Next;
  if (Fol)
   Foliage_Del(Fol, 0);

  BuildFoliageList(FE_Win->CurGrp);
  DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);

  FE_Win->Mod = 1;
  return (1);
  } /* if */

 return (0);

} /* GUIFoliage_Remove() */

/***********************************************************************/

STATIC_FCN short GUIFoliageGroup_Remove(void) // used locally only -> static, AF 19.7.2021
{
struct FoliageGroup *FolGp, *FolGpPrev = NULL;

 if (EcoShift[FE_Win->FolEco].Ecotype)
  {
  FolGp = EcoShift[FE_Win->FolEco].Ecotype->FolGp;

  if (FolGp)
   {
   while (FolGp && FolGp != FE_Win->CurGrp)
    {
    FolGpPrev = FolGp;
    FolGp = FolGp->Next;
    } /* while */

   if (FolGpPrev && FolGp)
    FolGpPrev->Next = FolGp->Next;
   else if (FolGp)
    EcoShift[FE_Win->FolEco].Ecotype->FolGp = FolGp->Next;
   if (FolGp)
    FoliageGroup_Del(FolGp, 0);

   BuildFoliageGroupList(EcoShift[FE_Win->FolEco].Ecotype);
   DisableColorChecks(FE_Win->CurGrp, FE_Win->CurImg);

   FE_Win->Mod = 1;
   return (1);
   } /* if */
  } /* if */

 return (0);

} /* GUIFoliageGroup_Remove() */

/***********************************************************************/

STATIC_FCN short GUIFoliage_View(struct Foliage *Fol, UBYTE **Bitmap) // used locally only -> static, AF 19.7.2021
{

 User_Message_Def((CONST_STRPTR)"Foliage Editor: View Image",
		 (CONST_STRPTR)"The image loaded properly. Maybe some day there will even be a way for you to see it!\n",
		 (CONST_STRPTR)"That would be nice", (CONST_STRPTR)"t", 0);

 return (0);

} /* GUIFoliage_View() */

/***********************************************************************/

STATIC_FCN void EcotypeDefaultColors(struct Ecotype *Eco, short Color) // used locally only -> static, AF 19.7.2021
{
struct FoliageGroup *FolGp;

 if (Eco)
  {
  FolGp = Eco->FolGp;
  while (FolGp)
   {
   FoliageGroupDefaultColors(FolGp, Color);
   FolGp = FolGp->Next;
   } /* while */
  } /* if */

} /* EcotypeDefaultColors() */

/***********************************************************************/

STATIC_FCN void FoliageGroupDefaultColors(struct FoliageGroup *FolGp, short Color) // used locally only -> static, AF 19.7.2021
{
short MatchColor;
struct Foliage *Fol;

 if (FolGp)
  {
  if ((MatchColor = SearchColorListMatch(Rootstock_GetColorName(&FolGp->Root))) >= 0)
   Color = MatchColor;
  Rootstock_SetValue(&FolGp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color);
  Rootstock_SetColorName(&FolGp->Root, PAR_NAME_COLOR(Color));
  Fol = FolGp->Fol;
  while (Fol)
   {
   FoliageDefaultColors(Fol, Color);
   Fol = Fol->Next;
   } /* while */
  } /* if */

} /* FoliageGroupDefaultColors() */

/***********************************************************************/

STATIC_FCN void FoliageDefaultColors(struct Foliage *Fol, short Color) // used locally only -> static, AF 19.7.2021
{
short MatchColor;

 if (Fol)
  {
  if ((MatchColor = SearchColorListMatch(Rootstock_GetColorName(&Fol->Root))) >= 0)
   Color = MatchColor;
  Rootstock_SetValue(&Fol->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color);
  Rootstock_SetColorName(&Fol->Root, PAR_NAME_COLOR(Color));
  } /* if */

} /* FoliageDefaultColors() */

/***********************************************************************/

STATIC_FCN short SearchColorListMatch(char *Name) // used locally only -> static, AF 19.7.2021
{
short i, FoundColor = -1;

 for (i=0; i<COLORPARAMS; i++)
  {
  if (! strcmp(Name, PAR_NAME_COLOR(i)))
   {
   FoundColor = i;
   break;
   } /* if match */
  } /* for i=0... */

 return (FoundColor);

} /* SearchColorListMatch() */

/**********************************************************************/

short SearchEcotypeColorMatch(struct Ecotype *Eco, short Color)
{
short Found = 0;
struct FoliageGroup *FolGp;
struct Foliage *Fol;

 if (Eco)
  {
  FolGp = Eco->FolGp;
  while (FolGp && ! Found)
   {
   if (Color == Rootstock_GetShortValue(&FolGp->Root, WCS_ECOTYPE_PALCOL))
    {
    Found = 1;
    break;
    } /* if */
   Fol = FolGp->Fol;
   while (Fol)
    {
    if (Color == Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_PALCOL))
     {
     Found = 1;
     break;
     } /* if */
    Fol = Fol->Next;
    } /* while */
   FolGp = FolGp->Next;
   } /* while */
  } /* if */

 return (Found);

} /* SearchColorListMatch() */

/***********************************************************************/

void AdjustFoliageColors(struct Ecotype *Eco, short Operation,
	short First, short Last)
{
struct FoliageGroup *FolGp;
struct Foliage *Fol;
short Color;

 if (Eco)
  {
  switch (Operation)
   {
   case WCS_ECOCOLOR_DECREMENT:
    {
    if ((Color = Rootstock_GetShortValue(&Eco->Root, WCS_ECOTYPE_PALCOL))
	>= First)
     {
     Rootstock_SetValue(&Eco->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color - 1);
     Rootstock_SetColorName(&Eco->Root, PAR_NAME_COLOR(Color - 1));
     } /* if */
    FolGp = Eco->FolGp;
    while (FolGp)
     {
     if ((Color = Rootstock_GetShortValue(&FolGp->Root, WCS_ECOTYPE_PALCOL))
	>= First)
      {
      Rootstock_SetValue(&FolGp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color - 1);
      Rootstock_SetColorName(&FolGp->Root, PAR_NAME_COLOR(Color - 1));
      } /* if */
     Fol = FolGp->Fol;
     while (Fol)
      {
      if ((Color = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_PALCOL))
	>= First)
       {
       Rootstock_SetValue(&Fol->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color - 1);
       Rootstock_SetColorName(&Fol->Root, PAR_NAME_COLOR(Color - 1));
       } /* if */
      if (FE_Win)
       {
       if (FolGp == FE_Win->CurGrp)
        {
        GUIFoliageGroup_SetGads(FolGp);
        if (Fol == FE_Win->CurImg)
         GUIFoliage_SetGads(Fol);
	} /* if */
       } /* if */
      Fol = Fol->Next;
      } /* while */
     FolGp = FolGp->Next;
     } /* while */
    break;
    } /* decrement */
   case WCS_ECOCOLOR_INCREMENT:
    {
    if ((Color = Rootstock_GetShortValue(&Eco->Root, WCS_ECOTYPE_PALCOL))
	>= First)
     {
     if (Color < COLORPARAMS - 1)
      {
      Rootstock_SetValue(&Eco->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color + 1);
      Rootstock_SetColorName(&Eco->Root, PAR_NAME_COLOR(Color + 1));
      } /* if */
     } /* if */
    FolGp = Eco->FolGp;
    while (FolGp)
     {
     if ((Color = Rootstock_GetShortValue(&FolGp->Root, WCS_ECOTYPE_PALCOL))
	>= First)
      {
      if (Color < COLORPARAMS - 1)
       {
       Rootstock_SetValue(&FolGp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color + 1);
       Rootstock_SetColorName(&FolGp->Root, PAR_NAME_COLOR(Color + 1));
       } /* if */
      } /* if */
     Fol = FolGp->Fol;
     while (Fol)
      {
      if ((Color = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_PALCOL))
	>= First)
       {
       if (Color < COLORPARAMS - 1)
        {
        Rootstock_SetValue(&Fol->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Color + 1);
        Rootstock_SetColorName(&Fol->Root, PAR_NAME_COLOR(Color + 1));
        } /* if */
       } /* if */
      if (FE_Win)
       {
       if (FolGp == FE_Win->CurGrp)
        {
        GUIFoliageGroup_SetGads(FolGp);
        if (Fol == FE_Win->CurImg)
         GUIFoliage_SetGads(Fol);
	} /* if */
       } /* if */
      Fol = Fol->Next;
      } /* while */
     FolGp = FolGp->Next;
     } /* while */
    break;
    } /* increment */
   case WCS_ECOCOLOR_SWAP:
    {
    if ((Color = Rootstock_GetShortValue(&Eco->Root, WCS_ECOTYPE_PALCOL))
	== Last)
     {
     Rootstock_SetValue(&Eco->Root, WCS_ECOTYPE_PALCOL, (float)0.0, First);
     Rootstock_SetColorName(&Eco->Root, PAR_NAME_COLOR(First));
     } /* if */
    else if (Color == First)
     {
     Rootstock_SetValue(&Eco->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Last);
     Rootstock_SetColorName(&Eco->Root, PAR_NAME_COLOR(Last));
     } /* if */
    FolGp = Eco->FolGp;
    while (FolGp)
     {
     if ((Color = Rootstock_GetShortValue(&FolGp->Root, WCS_ECOTYPE_PALCOL))
	== Last)
      {
      Rootstock_SetValue(&FolGp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, First);
      Rootstock_SetColorName(&FolGp->Root, PAR_NAME_COLOR(First));
      } /* if */
     else if (Color == First)
      {
      Rootstock_SetValue(&FolGp->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Last);
      Rootstock_SetColorName(&FolGp->Root, PAR_NAME_COLOR(Last));
      } /* if */
     Fol = FolGp->Fol;
     while (Fol)
      {
      if ((Color = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_PALCOL))
	== Last)
       {
       Rootstock_SetValue(&Fol->Root, WCS_ECOTYPE_PALCOL, (float)0.0, First);
       Rootstock_SetColorName(&Fol->Root, PAR_NAME_COLOR(First));
       } /* if */
      else if (Color == First) 
       {
       Rootstock_SetValue(&Fol->Root, WCS_ECOTYPE_PALCOL, (float)0.0, Last);
       Rootstock_SetColorName(&Fol->Root, PAR_NAME_COLOR(Last));
       } /* if */
      if (FE_Win)
       {
       if (FolGp == FE_Win->CurGrp)
        {
        GUIFoliageGroup_SetGads(FolGp);
        if (Fol == FE_Win->CurImg)
         GUIFoliage_SetGads(Fol);
	} /* if */
       } /* if */
      Fol = Fol->Next;
      } /* while */
     FolGp = FolGp->Next;
     } /* while */
    break;
    } /* swap */
   } /* switch */
  } /* if */

} /* AdjustFoliageColors() */

/***********************************************************************/

void GetFoliageLimits(struct Ecotype *Eco, short *Limit)
{
struct FoliageGroup *FolGp;
struct Foliage *Fol;
short ImgHt;

Limit[0] = Limit[1] = Limit[2] = 0;

 if (Eco)
  {
  FolGp = Eco->FolGp;
  while (FolGp)
   {
   Fol = FolGp->Fol;
   while (Fol)
    {
    Limit[0] ++;
    if (Fol->Root.Height * 100.0 > Limit[1])
     Limit[1] = Fol->Root.Height * 100.0;
    ImgHt = min(Fol->ImgHeight, Fol->Root.MaxImgHeight);
    if (ImgHt > Limit[2])
     Limit[2] =  ImgHt;
    Fol = Fol->Next;
    } /* while Foliage */
   FolGp = FolGp->Next;
   } /* while foliage group */
  } /* if Ecotype */ 

} /* GetFoliageLimits() */

/***********************************************************************/

STATIC_FCN short GUIEcotype_Save(void) // used locally only -> static, AF 19.7.2021
{
char filename[256], Path[256], Name[32], Ptrn[32];
FILE *ffile;
short success = 0;

 if (EcoShift[FE_Win->FolEco].Ecotype)
  {
  strcpy(Path, imagepath);
  Name[0] = 0;
  strcpy(Ptrn, "#?.etp");
  if (getfilenameptrn(1, "Ecotype Save Path/File", Path, Name, Ptrn))
   {
   if (strcmp(&Name[strlen(Name) - 4], ".etp"))
    strcat(Name, ".etp");
   strmfp(filename, Path, Name);
   if ((ffile = fopen(filename, "wb")))
    {
    if (Ecotype_Save(EcoShift[FE_Win->FolEco].Ecotype, ffile))
     success = 1;
    else
     User_Message_Def((CONST_STRPTR)"Foliage Editor: Save Ecotype", (CONST_STRPTR)"Error saving Ecotype file!\nOperation terminated.",
             (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
    fclose(ffile);
    } /* if file opened */
   } /* if file name */
  } /* if */

 return (success);

} /* GUIEcotype_Save() */

/***********************************************************************/

void GUIList_Clear(char **List, long ListSize, APTR ListView)
{

 if (List && ListSize > 0)
  memset(List, 0, ListSize);

 DoMethod(ListView, MUIM_List_Clear);

} /* GUIList_Clear() */

/***********************************************************************/

STATIC_FCN void GUIFoliageGroup_SetGads(struct FoliageGroup *This) // used locally only -> static, AF 19.7.2021
{
short color[3];
float Height, Density;
short UseImgCol, PalCol, MaxImgHt;

 if (This)
  {
  Density = Rootstock_GetFloatValue(&This->Root, WCS_ECOTYPE_DENSITY);
  Height = Rootstock_GetFloatValue(&This->Root, WCS_ECOTYPE_HEIGHT);
  UseImgCol = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_USEIMGCOL);
  PalCol = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_PALCOL);
  MaxImgHt = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_MAXIMGHT);

  setfloat(FE_Win->FloatStr[0], 100.0 * Density);
  setfloat(FE_Win->FloatStr[1], 100.0 * Height);
  setfloat(FE_Win->FloatStr[4], (double)MaxImgHt);
  nnset(FE_Win->Check[0], MUIA_Selected, UseImgCol);
  nnset(FE_Win->PalColCy[0], MUIA_Cycle_Active, PalCol);

  color[0] = PAR_FIRST_COLOR(PalCol, 0) / 16;
  color[1] = PAR_FIRST_COLOR(PalCol, 1) / 16;
  color[2] = PAR_FIRST_COLOR(PalCol, 2) / 16;
  SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
  FE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];
  set(FE_Win->BT_AddImage, MUIA_Disabled, FALSE);
  } /* if */
 else
  set(FE_Win->BT_AddImage, MUIA_Disabled, TRUE);

} /* GUIFoliageGroup_SetGads() */

/***********************************************************************/

STATIC_FCN void GUIFoliage_SetGads(struct Foliage *This) // used locally only -> static, AF 19.7.2021
{
short color[3];
char SizeStr[32];
float Height, Density;
short UseImgCol, PalCol, ImgWidth, ImgHeight, MaxImgHt;

 if (This)
  {
  Density = Rootstock_GetFloatValue(&This->Root, WCS_ECOTYPE_DENSITY);
  Height = Rootstock_GetFloatValue(&This->Root, WCS_ECOTYPE_HEIGHT);
  UseImgCol = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_USEIMGCOL);
  PalCol = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_PALCOL);
  ImgWidth = Foliage_GetImgSize(This, WCS_ECOTYPE_IMGWIDTH);
  ImgHeight = Foliage_GetImgSize(This, WCS_ECOTYPE_IMGHEIGHT);
  MaxImgHt = Rootstock_GetShortValue(&This->Root, WCS_ECOTYPE_MAXIMGHT);

  sprintf(SizeStr, "%d", ImgWidth);
  set(FE_Win->WidthText, MUIA_Text_Contents, (ULONG)SizeStr);
  sprintf(SizeStr, "%d", ImgHeight);
  set(FE_Win->HeightText, MUIA_Text_Contents, (ULONG)SizeStr);
  setfloat(FE_Win->FloatStr[2], 100.0 * Density);
  setfloat(FE_Win->FloatStr[3], 100.0 * Height);
  setfloat(FE_Win->FloatStr[5], (double)MaxImgHt);
  nnset(FE_Win->Check[1], MUIA_Selected, UseImgCol);
  nnset(FE_Win->PalColCy[1], MUIA_Cycle_Active, PalCol);

  color[0] = PAR_FIRST_COLOR(PalCol, 0) / 16;
  color[1] = PAR_FIRST_COLOR(PalCol, 1) / 16;
  color[2] = PAR_FIRST_COLOR(PalCol, 2) / 16;
  SetRGB4(&WCSScrn->ViewPort, 9, color[0], color[1], color[2]);
  FE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
  } /* if */

} /* GUIFoliage_SetGads() */

/***********************************************************************/

STATIC_FCN void BuildFoliageGroupList(struct Ecotype *Eco) // used locally only -> static, AF 19.7.2021
{
short color[3];
char **NameAddr;
struct FoliageGroup *CurGrp, *FirstGrp;

 GUIList_Clear(FE_Win->GroupList, FE_Win->GroupListSize, FE_Win->LS_GroupList);

 FE_Win->GroupEntries = 0;
 FE_Win->CurGrp = NULL;
 FE_Win->CurrentGroup = -1;

 if (Eco)
  {
  CurGrp = Eco->FolGp;
  if (CurGrp)
   {
   FirstGrp = CurGrp;
   while (CurGrp)
    {
    FE_Win->GrpAddrList[FE_Win->GroupEntries] = CurGrp;
    NameAddr = Rootstock_GetNameAddr(&CurGrp->Root);
    DoMethod(FE_Win->LS_GroupList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
    FE_Win->GroupEntries ++;
    CurGrp = CurGrp->Next;
    } /* while */
   FE_Win->CurrentGroup = 0;
   FE_Win->CurGrp = FirstGrp;
   nnset(FE_Win->LS_GroupList, MUIA_List_Active, 0);
   GUIFoliageGroup_SetGads(FirstGrp);
   BuildFoliageList(FirstGrp);
   } /* if at least one group exists */
  else
   {
   set(FE_Win->BT_AddImage, MUIA_Disabled, TRUE);
   BuildFoliageList(NULL);
   } /* else */
  } /* if */
 else
  {
  nnset(FE_Win->PalColCy[0], MUIA_Cycle_Active, PAR_COLR_ECO(FE_Win->FolEco));
  color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 0) / 16;
  color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 1) / 16;
  color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 2) / 16;
  SetRGB4(&WCSScrn->ViewPort, 8, color[0], color[1], color[2]);
  FE_Win->Colors[8] = color[0] * 256 + color[1] * 16 + color[2];

  set(FE_Win->BT_AddImage, MUIA_Disabled, TRUE);
  nnset(FE_Win->PalColCy[1], MUIA_Cycle_Active, PAR_COLR_ECO(FE_Win->FolEco));

  color[0] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 0) / 16;
  color[1] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 1) / 16;
  color[2] = PAR_FIRST_COLOR(PAR_COLR_ECO(FE_Win->FolEco), 2) / 16;
  SetRGB4(&WCSScrn->ViewPort, 9, color[0], color[1], color[2]);
  FE_Win->Colors[9] = color[0] * 256 + color[1] * 16 + color[2];
  } /* else */

} /* BuildfoliageGroupList() */

/*********************************************************************/

STATIC_FCN void BuildFoliageList(struct FoliageGroup *FolGp) // used locally only -> static, AF 19.7.2021
{
char **NameAddr;
struct Foliage *CurImg, *FirstImg;

 GUIList_Clear(FE_Win->ImageList, FE_Win->ImageListSize, FE_Win->LS_ImageList);

 FE_Win->ImageEntries = 0;
 FE_Win->CurImg = NULL;
 FE_Win->CurrentImage = -1;

 if (FolGp)
  {
  CurImg = FolGp->Fol;
  if (CurImg)
   {
   FirstImg = CurImg;
   while (CurImg)
    {
    FE_Win->ImgAddrList[FE_Win->ImageEntries] = CurImg;
    NameAddr = Foliage_GetImageNameAddr(CurImg);
    DoMethod(FE_Win->LS_ImageList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
    FE_Win->ImageEntries ++;
    CurImg = CurImg->Next;
    } /* while */
   FE_Win->CurrentImage = 0;
   FE_Win->CurImg = FirstImg;
   nnset(FE_Win->LS_ImageList, MUIA_List_Active, 0);
   GUIFoliage_SetGads(FirstImg);
   } /* if at least one image exists */
  } /* if */

} /* BuildfoliageList() */

/***********************************************************************/

STATIC_FCN void DisableColorChecks(struct FoliageGroup *FolGp, struct Foliage *Fol) // used locally only -> static, AF 19.7.2021
{
short ColorImage = 0, ColorGroup = 0, UseImgCol = 0, UseGrpCol = 1,
	BWImage = 0;

 if (Fol)
  {
  ColorImage = Foliage_GetImgSize(Fol, WCS_ECOTYPE_COLORIMAGE);
  UseImgCol = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_USEIMGCOL);
  } /* if */

 if (FolGp)
  {
  Fol = FolGp->Fol;
  while (Fol)
   {
   if (Foliage_GetImgSize(Fol, WCS_ECOTYPE_COLORIMAGE))
    {
    ColorGroup = 1;
    if (! Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_USEIMGCOL))
     UseGrpCol = 0;
    }
   else
    BWImage = 1;
   Fol = Fol->Next;
   } /* while */
  } /* if */

 set(FE_Win->Check[0], MUIA_Disabled, ! ColorGroup);
 set(FE_Win->PalColCy[0], MUIA_Disabled, ColorGroup && UseGrpCol && ! BWImage);
 set(FE_Win->Check[1], MUIA_Disabled, ! ColorImage);
 set(FE_Win->PalColCy[1], MUIA_Disabled, ColorImage && UseImgCol);

} /* DisableColorChecks() */
