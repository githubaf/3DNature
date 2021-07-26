/* EdDBaseGUI.c
** World Construction Set GUI for Database Editing module.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

STATIC_FCN void Set_DL_List(struct DirList *DLItem, short Update, short ActiveItem); // used locally only -> static, AF 26.7.2021
STATIC_FCN void Remove_DE_Item(short OBN, short Remove); // used locally only -> static, AF 26.7.2021


void Make_DE_Window(void)
{
 short i;
 long open;
 static const char *DE_LineCycle[] = {"Point", "Circle", "Square", "Cross",
	"Solid", "Dotted", "Dashed", "Broken", NULL};
 static const char *DE_SpecCycle[] = {"Topo", "Surface", "Vector", "Illum Vec",
	"Segment V", "Illum Seg",
#ifdef ENABLE_VECTOR_PATHS
	 "Cam Path", "Foc Path",
#endif /* ENABLE_VECTOR_PATHS */
	 NULL};

 if (DE_Win)
  {
  DoMethod(DE_Win->DatabaseEditWin, MUIM_Window_ToFront);
  set(DE_Win->DatabaseEditWin, MUIA_Window_Activate, TRUE);
  SetRGB4(&WCSScrn->ViewPort, 8,
	 (DE_Win->Colors[8] & 0xf00) / 256,
	 (DE_Win->Colors[8] & 0x0f0) / 16,
	 (DE_Win->Colors[8] & 0x00f));
  SetRGB4(&WCSScrn->ViewPort, 9,
	 (DE_Win->Colors[9] & 0xf00) / 256,
	 (DE_Win->Colors[9] & 0x0f0) / 16,
	 (DE_Win->Colors[9] & 0x00f));
  return;
  } /* if window already exists */

 if (! dbaseloaded)
  {
  User_Message("Database Editor",
	"You must first load or create a database before opening the editor.", "OK", "o");
  return;
  } /* if no database */

 if ((DE_Win = (struct DatabaseEditWindow *)
	get_Memory(sizeof (struct DatabaseEditWindow), MEMF_CLEAR)) == NULL)
   return;

 if (! DBList_New(NoOfObjects + 20))
  {
  User_Message("Database Module",
	"Out of memory!\nCan't open database window.", "OK", "o");
  Close_DE_Window();
  return;
  } /* if out of memory */

  Set_Param_Menu(10);

     DE_Win->DatabaseEditWin = WindowObject,
      MUIA_Window_Title		, "Database Editor",
      MUIA_Window_ID		, 'DBED',
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2("Options"),
          Child, DE_Win->BT_Settings[0] = KeyButtonFunc('1', "\33cVectors"), 
          Child, DE_Win->BT_Settings[1] = KeyButtonFunc('2', "\33cSurfaces"),
          Child, DE_Win->BT_Settings[2] = KeyButtonFunc('3', "\33cFractals"), 
	  End, /* HGroup */
        Child, HGroup,
/* group on the left */
          Child, VGroup,
	/* object ID number (OBN) */
#ifdef dfgdhfgdhfh
            Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Object ID "),
	      Child, DE_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
              Child, DE_Win->LgArrow[0] = ImageButton(MUII_ArrowLeft),
              Child, DE_Win->Arrow[0][0] = ImageButton(MUII_ArrowLeft),
              Child, DE_Win->Arrow[0][1] = ImageButton(MUII_ArrowRight),
              Child, DE_Win->LgArrow[1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
#endif
	/* object name */
	    Child, HGroup,
	      Child, DE_Win->BT_Name = KeyButtonFunc('n', "\33cName"),
	      Child, DE_Win->Str[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234567890", End,
	      Child, DE_Win->Check = CheckMark(0),
	      Child, Label2("Enabled"),
	      End, /* HGroup */
	/* vector points */
	    Child, HGroup,
	      Child, Label2("Points"),
	      Child, DE_Win->PointTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123", End,
	      Child, Label2("Class"),
	      Child, DE_Win->CY_Spec = Cycle(DE_SpecCycle),
	      End, /* HGroup */
	/* layer 1 */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Layer 1 "),
              Child, DE_Win->Str[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123", End,
	      Child, DE_Win->BT_LayerSel[0] = SimpleButton("Sel"),
	      Child, DE_Win->BT_LayerOn[0] = SimpleButton("On"),
	      Child, DE_Win->BT_LayerOff[0] = SimpleButton("Off"),
	      End, /* HGroup */
	/* layer 2 */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Layer 2 "),
              Child, DE_Win->Str[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123", End,
	      Child, DE_Win->BT_LayerSel[1] = SimpleButton("Sel"),
	      Child, DE_Win->BT_LayerOn[1] = SimpleButton("On"),
	      Child, DE_Win->BT_LayerOff[1] = SimpleButton("Off"),
	      End, /* HGroup */
	/* object label */
	    Child, HGroup,
	      Child, DE_Win->BT_Label = KeyButtonFunc('b', "\33cLabel"), 
	      Child, DE_Win->Str[3] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	/* maximum fractal depth for topos */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, RectangleObject, End,
	      Child, Label2("DEM Max Fractal "),
	      Child, DE_Win->IntStr[6] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012", End,
              Child, DE_Win->Arrow[3][0] = ImageButton(MUII_ArrowLeft),
              Child, DE_Win->Arrow[3][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	/* line type */
	    Child, HGroup,
	      Child, RectangleObject, End,
	      Child, Label2("Line Style"),
	      Child, DE_Win->CY_Line = Cycle(DE_LineCycle),
	      End, /* HGroup */
	/* line width */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, RectangleObject, End,
	      Child, Label2("Line Weight "),
	      Child, DE_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012", End,
              Child, DE_Win->Arrow[1][0] = ImageButton(MUII_ArrowLeft),
              Child, DE_Win->Arrow[1][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	/* draw pen */
	    Child, HGroup,
	      Child, RectangleObject, End,
	      Child, Label2("Draw Pen "),
	      Child, SmallImageDisplay(&EC_Button8),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, DE_Win->IntStr[2] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012", End,
                Child, DE_Win->Arrow[2][0] = ImageButton(MUII_ArrowLeft),
                Child, DE_Win->Arrow[2][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      End, /* HGroup */
	/* RGB render values */
	    Child, HGroup,
	      Child, Label2("RGB"),
	      Child, SmallImageDisplay(&EC_Button9),
	      Child, VGroup,
		Child, HGroup,
	          Child, DE_Win->IntStr[3] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                  Child, DE_Win->Prop[0] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                  Child, TextObject, MUIA_Text_Contents, "R",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,
		  End, /* HGroup */
		Child, HGroup,
	          Child, DE_Win->IntStr[4] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                  Child, DE_Win->Prop[1] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                  Child, TextObject, MUIA_Text_Contents, "G",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,
		  End, /* HGroup */
		Child, HGroup,
	          Child, DE_Win->IntStr[5] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123", End,
                  Child, DE_Win->Prop[2] = PropObject, PropFrame,
			MUIA_Prop_Horiz, TRUE,
			MUIA_Prop_Entries, 260,
			MUIA_Prop_Visible, 5,
			MUIA_Prop_First, 0, End,
                  Child, TextObject, MUIA_Text_Contents, "B",
			 MUIA_Text_SetMin, TRUE, MUIA_Text_SetMax, TRUE, End,
		  End, /* HGroup */

		End, /* VGroup */
	      End, /* HGroup */
	    End, /* VGroup */
/* list of database entries */
	  Child, DE_Win->LS_List = ListviewObject,
	      MUIA_Listview_Input, TRUE,
	      MUIA_Listview_MultiSelect, TRUE,
              MUIA_Listview_List, ListObject, ReadListFrame, End,
	    End,

	  End, /* HGroup */
/* Buttons at bottom */
        Child, HGroup, MUIA_Group_SameWidth, TRUE,
          Child, DE_Win->BT_New = KeyButtonFunc('o',    "\33cNew Obj"), 
          Child, DE_Win->BT_Add = KeyButtonFunc('d',    "\33cAdd Obj"), 
          Child, DE_Win->BT_Remove = KeyButtonFunc('m', "\33cRemove �"), 
          Child, DE_Win->BT_Search = KeyButtonFunc('h', "\33cSearch �"), 
          Child, DE_Win->BT_Sort = KeyButtonFunc('r',   "\33cSort"), 
          End, /* HGroup */

        Child, HGroup, MUIA_Group_SameWidth, TRUE,
          Child, DE_Win->BT_Save = KeyButtonFunc('s',   "\33cSave �"), 
          Child, DE_Win->BT_Load = KeyButtonFunc('l',   "\33cLoad �"), 
          Child, DE_Win->BT_Append = KeyButtonFunc('a', "\33cAppend �"), 
          Child, DE_Win->BT_Create = KeyButtonFunc('t', "\33cCreate �"), 
          End, /* HGroup */

        End, /* VGroup */
      End; /* WindowObject DE_Win->DatabaseEditWin */

  if (! DE_Win->DatabaseEditWin)
   {
   Close_DE_Window();
   User_Message("Database Editor", "Out of memory!", "OK", "o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DE_Win->DatabaseEditWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* copy color palettes */
  memcpy(&DE_Win->Colors[0], &AltColors[0], sizeof (AltColors));

/* ReturnIDs */
  DoMethod(DE_Win->DatabaseEditWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DE_APPLY);  

  MUI_DoNotiPresFal(app,
   DE_Win->BT_Save, ID_DB_SAVE, DE_Win->BT_Load, ID_DB_LOAD,
   DE_Win->BT_Append, ID_DB_APPEND, DE_Win->BT_Create, ID_DB_CREATE,
   DE_Win->BT_New, ID_DE_NEW, DE_Win->BT_Add, ID_DE_ADD, DE_Win->BT_Remove, ID_DE_REMOVE,
   DE_Win->BT_Name, ID_DE_NAME, DE_Win->BT_Search, ID_DE_SEARCH,
   DE_Win->BT_Sort, ID_DE_SORT,
   DE_Win->BT_Settings[0], ID_SB_SETPAGE(2),
   DE_Win->BT_Settings[1], ID_SB_SETPAGE(4),
   DE_Win->BT_Settings[2], ID_SB_SETPAGE(5),
   NULL);

  for (i=0; i<2; i++)
   {
   MUI_DoNotiPresFal(app, DE_Win->BT_LayerSel[i], ID_DE_LAYERSEL(i),
    DE_Win->BT_LayerOn[i], ID_DE_LAYERON(i),
    DE_Win->BT_LayerOff[i], ID_DE_LAYEROFF(i), NULL);  
   } /* for i=0... */
  for (i=1; i<4; i++)
   {
   MUI_DoNotiPresFal(app, DE_Win->Arrow[i][0], ID_DE_ARROWLEFT(i),
    DE_Win->Arrow[i][1], ID_DE_ARROWRIGHT(i), NULL);
   } /* for i=0... */

/* Link prop gadgets to strings */
  for (i=0; i<3; i++)
   {
   DoMethod(DE_Win->Prop[i], MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
	DE_Win->IntStr[i + 3], 3, MUIM_Set, MUIA_String_Integer, MUIV_TriggerValue);
   } /* for i=0... */

/* Link checkmark to application */
   DoMethod(DE_Win->Check, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
	 app, 2, MUIM_Application_ReturnID, ID_DE_CHECK);

/* Link cycle gadget to application */
 DoMethod(DE_Win->CY_Line, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_DE_LINECYCLE);
 DoMethod(DE_Win->CY_Spec, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_DE_SPECCYCLE);
  
/* link string gadgets to application */
  for (i=1; i<7; i++)
   {
   DoMethod(DE_Win->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_DE_INTSTR(i));
   } /* for i=1... */

  for (i=1; i<4; i++)
   {
   DoMethod(DE_Win->Str[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_DE_STRING(i));
   } /* for i=1... */

/* Set cycle chain for Int strings */
  for (i=1; i<3; i++)
   {
   DoMethod(DE_Win->Str[i], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	DE_Win->DatabaseEditWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, DE_Win->Str[i + 1]);
   } /* for i=0... */
  DoMethod(DE_Win->Str[3], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	DE_Win->DatabaseEditWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, DE_Win->IntStr[6]);
  DoMethod(DE_Win->IntStr[6], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	DE_Win->DatabaseEditWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, DE_Win->IntStr[1]);
  for (i=1; i<5; i++)
   {
   DoMethod(DE_Win->IntStr[i], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	DE_Win->DatabaseEditWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, DE_Win->IntStr[i + 1]);
   } /* for i=0... */
  DoMethod(DE_Win->IntStr[5], MUIM_Notify,
	MUIA_String_Acknowledge, MUIV_EveryTime,
	DE_Win->DatabaseEditWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, DE_Win->LS_List);

/* Set tab cycle chain */
  DoMethod(DE_Win->DatabaseEditWin, MUIM_Window_SetCycleChain,
/*	DE_Win->IntStr[0],*/
	DE_Win->BT_Name, DE_Win->CY_Spec, DE_Win->Check,
	DE_Win->Str[1], DE_Win->BT_LayerSel[0],
	DE_Win->BT_LayerOn[0], DE_Win->BT_LayerOff[0],
	DE_Win->Str[2], DE_Win->BT_LayerSel[1], DE_Win->BT_LayerOn[1],
	DE_Win->BT_LayerOff[1], DE_Win->BT_Label, DE_Win->Str[3],
	DE_Win->IntStr[6], DE_Win->CY_Line,
	DE_Win->IntStr[1], DE_Win->IntStr[2], DE_Win->IntStr[3], DE_Win->Prop[0],
	DE_Win->IntStr[4], DE_Win->Prop[1], DE_Win->IntStr[5], DE_Win->Prop[2],
	DE_Win->LS_List, DE_Win->BT_New, DE_Win->BT_Add, DE_Win->BT_Search,
	DE_Win->BT_Sort, DE_Win->BT_Remove, DE_Win->BT_Save,
	DE_Win->BT_Load, DE_Win->BT_Append, DE_Win->BT_Create, NULL);

/* Set active gadget */
  set(DE_Win->DatabaseEditWin, MUIA_Window_DefaultObject, DE_Win->LS_List);

/* Create database list */
  Set_DE_List(0);
  set(DE_Win->LS_List, MUIA_List_Active, OBN);
  DoMethod(DE_Win->LS_List, MUIM_List_Jump, OBN);

/* Set gadgets and list to current database item */
  Set_DE_Item(OBN);

/* link list to application */
  DoMethod(DE_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DE_LIST);
  DoMethod(DE_Win->LS_List, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DE_LISTDBCLICK);

/* Open window */
  set(DE_Win->DatabaseEditWin, MUIA_Window_Open, TRUE);
  get(DE_Win->DatabaseEditWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DE_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DE_Win->DatabaseEditWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DE_ACTIVATE);

/* Get Window structure pointer */
  get(DE_Win->DatabaseEditWin, MUIA_Window_Window, &DE_Win->Win);

} /* Make_DE_Window() */

/*********************************************************************/

void Close_DE_Window(void)
{

 if (DE_Win)
  {
  if (DE_Win->DatabaseEditWin)
   {
   set(DE_Win->DatabaseEditWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
   DoMethod(app, OM_REMMEMBER, DE_Win->DatabaseEditWin);
   MUI_DisposeObject(DE_Win->DatabaseEditWin);
   } /* if window created */
  if (DE_Win->DBName) free_Memory(DE_Win->DBName, DE_Win->DBNameSize);
  free_Memory(DE_Win, sizeof (struct DatabaseEditWindow));
  DE_Win = NULL;
  } /* if memory allocated */

} /* Close_DE_Window() */

/*********************************************************************/

void Handle_DE_Window(ULONG WCS_ID)
{
 short i, j;
 long state;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_DE_Window();
   return;
   } /* Open Database Editor Window */

  if (! DE_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    SetRGB4(&WCSScrn->ViewPort, 8,
	 (DE_Win->Colors[8] & 0xf00) / 256,
	 (DE_Win->Colors[8] & 0x0f0) / 16,
	 (DE_Win->Colors[8] & 0x00f));
    SetRGB4(&WCSScrn->ViewPort, 9,
	 (DE_Win->Colors[9] & 0xf00) / 256,
	 (DE_Win->Colors[9] & 0x0f0) / 16,
	 (DE_Win->Colors[9] & 0x00f));
    break;
    } /* Activate Database Editing window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_DE_NAME:
      {
      char *suffix, filename[256], oldname[32], newname[256];
      short found;
      struct DirList *DLItem;
      FILE *ffile;

      strcpy(oldname, DBase[OBN].Name);
      strcpy(str, DBase[OBN].Name);
NewName:
      found = 0;
      if (! GetInputString("Enter new object name.", ":;*/?`#%", str))
       break;
      while (strlen(str) < length[0])
       strcat(str, " ");

      for (i=0; i<NoOfObjects; i++)
       {
       if (! strnicmp(str, DBase[i].Name, length[0]))
        {
        found = 1;
        break;
        } /* if name already in use */
       } /* for i=0... */
      if (found)
       {
       if (! User_Message_Def("Database Module: Name",
		"Object name already present in database!\nTry a new name?",
		"OK|Cancel", "oc", 1))
        break;
       goto NewName;
       }
      strncpy(DBase[OBN].Name, str, length[0]);
      set(DE_Win->Str[0], MUIA_Text_Contents, DBase[OBN].Name);
/* find old file directories */
      for (i=0; i<3; i++)
       {
       switch (i)
        {
        case 0:
         {
         suffix = ".Obj";
         break;
	 }
        case 1:
         {
         suffix = ".elev";
         break;
	 }
        case 2:
         {
         suffix = ".relel";
         break;
	 }
	} /* switch */
       DLItem = DL;
       while (DLItem)
        {
        strmfp(filename, DLItem->Name, oldname);
        strcat(filename, suffix);
        if ((ffile = fopen(filename, "r")) == NULL)
         {
         DLItem = DLItem->Next;
         } /* if file not found */
        else
         {
         fclose (ffile);
         sprintf(str, "%s%s", DBase[OBN].Name, suffix);
         strmfp(newname, DLItem->Name, str);
         rename(filename, newname);
         break;
         } /* else file opened */
        } /* while */
       } /* for i=0... */
      DoMethod(DE_Win->LS_List, MUIM_List_Redraw, OBN);
      DB_Mod = 1;
      break;
      } /* name change */
     case ID_DE_NEW:
      {
      DBaseObject_New();
      break;
      } /* new object */
     case ID_DE_ADD:
      {
      DBaseObject_Add();
      Set_DE_List(0);
      Set_DE_Item(OBN); 
      break;
      } /* new object */
     case ID_DE_SEARCH:
      {
      short StrLength;

      str[0] = 0;
      if (! GetInputString("Enter search string.", ":;*/?`#%", str))
       break;
      StrLength = strlen(str);
      if (StrLength == 0)
       break;
      for (i=0; i<NoOfObjects; i++)
       {
       for (j=0; j<=length[0] - StrLength; j++)
        {
        if (! strnicmp(&DBase[i].Name[j], str, StrLength))
         {
         DoMethod(DE_Win->LS_List, MUIM_List_Select, i,
		 MUIV_List_Select_On, NULL);
         break;
	 } /* if match found */
	} /* for j=0... */
       } /* for i=0... */
      break;
      } /* search select */
     case ID_DE_SORT:
      {
      short Change = 1;
      struct BusyWindow *BWDB;

      BWDB = BusyWin_New("Sorting", NoOfObjects, 0, 'BWDB');
      while (Change)
       {
       Change = 0;
       for (i=0; i<NoOfObjects-1; i++)
        {
        if (stricmp(DBase[i].Name, DBase[i + 1].Name) > 0)
         {
         swmem(&DBase[i], &DBase[i + 1], sizeof (struct database));
         if (! Change)
          BusyWin_Update(BWDB, i);
         Change = 1;
	 } /* if reverse order */
	} /* for i=0... */
       if (CheckInput_ID() == ID_BW_CLOSE)
        break;
       } /* while */
      Set_DE_List(1);
      BusyWin_Del(BWDB);
      DB_Mod = 1;
      break;
      } /* sort database */
     case ID_DE_REMOVE:
      {
      short Remove;
      long ActiveItem;

      if ((Remove = User_Message_Def("Database Module: Remove Item",
	"Delete object, elevation and relative elevation files from disk as\
 well as remove their names from the Database?",
	"From Disk|Database Only|Cancel", "fdc", 1)) == 0)
	break;
      SetPointer(DE_Win->Win, WaitPointer, 16, 16, -6, 0);
      get(DE_Win->LS_List, MUIA_List_Active, &ActiveItem);
      for (j=NoOfObjects-1; j>=0; j--)
       {
       DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_Ask, &state);
       if (state || j == ActiveItem)
        Remove_DE_Item(j, Remove);
       DB_Mod = 1;
       } /* for j=0... */
      if (ActiveItem >= NoOfObjects)
       ActiveItem = 0;
      OBN = ActiveItem;
      Set_DE_List(0);
      Set_DE_Item(OBN);
      ClearPointer(DE_Win->Win);
      break;
      } /* remove database item */
     case ID_DE_APPLY:
      {
      Close_DE_Window();
      break;
      } /* Apply changes to Palette arrays */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    LONG Enabled;
    char mark, mark2;

    get(DE_Win->Check, MUIA_Selected, &Enabled);
    DoMethod(DE_Win->LS_List, MUIM_List_Select, OBN,
	 MUIV_List_Select_Ask, &state);
    
    if (Enabled)
     {
     mark = 'Y';
     mark2 = '*';
     } /* if */
    else
     {
     mark = 'N';
     mark2 = ' ';
     } /* else */
    DBase[OBN].Mark[0] = mark;
    DBase[OBN].Enabled = mark2;
    if (Enabled)
     DBase[OBN].Flags |= 2;
    Map_DB_Object(OBN, Enabled, state);
    if (! Enabled)
     DBase[OBN].Flags &= (255 ^ 2);
    DoMethod(DE_Win->LS_List, MUIM_List_Redraw, OBN);

    if (! DE_Win->Block[0])
     {
     for (j=0; j<NoOfObjects; j++)
      {
      DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
      if (state)
       {
       DBase[j].Mark[0] = mark;
       DBase[j].Enabled = mark2;
       if (Enabled)
        DBase[j].Flags |= 2;
       Map_DB_Object(j, Enabled, state);
       if (! Enabled)
        DBase[j].Flags &= (255 ^ 2);
       DoMethod(DE_Win->LS_List, MUIM_List_Redraw, j);
       } /* if object selected */
      } /* for j=0... */
     } /* if selection state not changing */
    DE_Win->Block[0] = 0;
    DB_Mod = 1;
    break;
    } /* BUTTONS2 */

   case GP_BUTTONS3:
    {
    i = WCS_ID - ID_DE_LAYERSEL(0);

    switch (i)
     {
     case 0:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer1, DBase[j].Layer1))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_On, NULL);
	} /* if same layer */
       } /* for j=0... */
      break;
      } /* layer 1 */
     case 1:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer2, DBase[j].Layer2))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_On, NULL);
	} /* if same layer */
       } /* for j=0... */
      break;
      } /* layer 2 */
     } /* switch i */
    break;
    } /* BUTTONS3 layers selected */

   case GP_BUTTONS4:
    {
    char mark = 'Y', mark2 = '*';

    i = WCS_ID - ID_DE_LAYERON(0);

    switch (i)
     {
     case 0:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer1, DBase[j].Layer1))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        DBase[j].Mark[0] = mark;
        DBase[j].Enabled = mark2;
        DBase[j].Flags |= 2;
        Map_DB_Object(j, 1, state);
        DoMethod(DE_Win->LS_List, MUIM_List_Redraw, j);
	} /* if same layer */
       } /* for j=0... */
      set(DE_Win->Check, MUIA_Selected, TRUE);
      break;
      } /* layer 1 */
     case 1:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer2, DBase[j].Layer2))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        DBase[j].Mark[0] = mark;
        DBase[j].Enabled = mark2;
        DBase[j].Flags |= 2;
        Map_DB_Object(j, 1, state);
        DoMethod(DE_Win->LS_List, MUIM_List_Redraw, j);
	} /* if same layer */
       } /* for j=0... */
      set(DE_Win->Check, MUIA_Selected, TRUE);
      break;
      } /* layer 2 */
     } /* switch i */
    DB_Mod = 1;
    break;
    } /* BUTTONS4 layers on */

   case GP_BUTTONS5:
    {
    char mark = 'N', mark2 = ' ';

    i = WCS_ID - ID_DE_LAYEROFF(0);

    switch (i)
     {
     case 0:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer1, DBase[j].Layer1))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
 	 MUIV_List_Select_Ask, &state);
        Map_DB_Object(j, 0, state);
        DBase[j].Mark[0] = mark;
        DBase[j].Enabled = mark2;
        DBase[j].Flags &= (255 ^ 2);
        DoMethod(DE_Win->LS_List, MUIM_List_Redraw, j);
	} /* if same layer */
       } /* for j=0... */
      set(DE_Win->Check, MUIA_Selected, FALSE);
      break;
      } /* layer 1 */
     case 1:
      {
      for (j=0; j<NoOfObjects; j++)
       {
       if (! strcmp(DBase[OBN].Layer2, DBase[j].Layer2))
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        Map_DB_Object(j, 0, state);
        DBase[j].Mark[0] = mark;
        DBase[j].Enabled = mark2;
        DBase[j].Flags &= (255 ^ 2);
        DoMethod(DE_Win->LS_List, MUIM_List_Redraw, j);
	} /* if same layer */
       } /* for j=0... */
      set(DE_Win->Check, MUIA_Selected, FALSE);
      break;
      } /* layer 2 */
     } /* switch i */
    DB_Mod = 1;
    break;
    } /* BUTTONS5 layers off */

   case GP_LIST1:
    {
    LONG data, OldOBN;

    get(DE_Win->LS_List, MUIA_List_Active, &data);

    Set_DE_Item(data);
    OldOBN = OBN;
    OBN = data;
    if (MapWind0)
     {
     struct clipbounds cb;

     setclipbounds(MapWind0, &cb);
     DoMethod(DE_Win->LS_List, MUIM_List_Select, OldOBN,
	 MUIV_List_Select_Ask, &state);
     if (! state)
      outline(MapWind0, OldOBN, DBase[OldOBN].Color, &cb);
     if (strcmp(DBase[OBN].Special, "TOP") && strcmp(DBase[OBN].Special, "SFC"))
      outline(MapWind0, OBN, 2, &cb);
     else
      outline(MapWind0, OBN, 7, &cb);
     } /* if map view open */
    break;
    } /* ecosystem list */

   case GP_LIST2:
    {
    LONG select;

    if (DBase[OBN].Mark[0] == 'Y')
     select = 0;
    else
     select = 1;
    set(DE_Win->Check, MUIA_Selected, select);
    DB_Mod = 1;
    break;
    } /* double click in ecosystem list */

   case GP_STRING1:
    {
    char *data;
    LONG pos;

    i = WCS_ID - ID_DE_STRING(0);
    get(DE_Win->Str[i], MUIA_String_Contents, &data);
    get(DE_Win->Str[i], MUIA_String_BufferPos, &pos);
    switch (i)
     {
/* case 0 would be the name string which is now a text object */
     case 1:
      {
      short j = 0;

      strncpy(DBase[OBN].Layer1, data, length[1]);
      while (strlen(DBase[OBN].Layer1) < length[1]) strcat(DBase[OBN].Layer1, " ");
      while(DBase[OBN].Layer1[j])
       {
       DBase[OBN].Layer1[j] = toupper(DBase[OBN].Layer1[j]);
       j ++;
       } /* while */
      set(DE_Win->Str[1], MUIA_String_Contents, DBase[OBN].Layer1);
      set(DE_Win->Str[1], MUIA_String_BufferPos, pos);

      if (! DE_Win->Block[1])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_Ask, &state);
        if (state)
         {
         strcpy(DBase[j].Layer1, DBase[OBN].Layer1);
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[1] = 0;
      break;
      } /*  */
     case 2:
      {
      short j = 0;

      strncpy(DBase[OBN].Layer2, data, length[2]);
      while (strlen(DBase[OBN].Layer2) < length[2]) strcat(DBase[OBN].Layer2, " ");
      while(DBase[OBN].Layer2[j])
       {
       DBase[OBN].Layer2[j] = toupper(DBase[OBN].Layer2[j]);
       j ++;
       } /* while */
      set(DE_Win->Str[2], MUIA_String_Contents, DBase[OBN].Layer2);
      set(DE_Win->Str[2], MUIA_String_BufferPos, pos);

      if (! DE_Win->Block[2])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         strcpy(DBase[j].Layer2, DBase[OBN].Layer2);
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[2] = 0;
      break;
      } /*  */
     case 3:
      {
      strncpy(DBase[OBN].Label, data, length[6]);
      while (strlen(DBase[OBN].Label) < length[6]) strcat(DBase[OBN].Label, " ");
      set(DE_Win->Str[3], MUIA_String_Contents, DBase[OBN].Label);
      set(DE_Win->Str[3], MUIA_String_BufferPos, pos);

      if (! DE_Win->Block[3])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         strcpy(DBase[j].Label, DBase[OBN].Label);
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[3] = 0;
      break;
      } /*  */
     } /* switch i */
    DB_Mod = 1;
    break;
    } /* STRING1 */

   case GP_STRING2:
    {
    LONG data;

    i = WCS_ID - ID_DE_INTSTR(0);
    get(DE_Win->IntStr[i], MUIA_String_Integer, &data);
    switch (i)
     {
#ifdef HGHGJHGJ
     case 0:
      {
      if (data > NoOfObjects - 1) data = NoOfObjects - 1;
      Set_DE_Item(data);
      OBN = data;
      break;
      } /* Object ID OBN */
#endif
     case 1:
      {
      DBase[OBN].LineWidth = data;
      if (! DE_Win->Block[4])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].LineWidth = data;
         } /* if object selected */
        } /* for j=0... */
       }
      DE_Win->Block[4] = 0;
      break;
      } /* line width */
     case 2:
      {
      if (data > 15)
       {
       set(DE_Win->IntStr[2], MUIA_String_Integer, 15);
       break;
       }
      if (data < 0)
       {
       set(DE_Win->IntStr[2], MUIA_String_Integer, 0);
       break;
       }
      DBase[OBN].Color = data;
      SetRGB4(&WCSScrn->ViewPort, 8, 
	((AltColors[DBase[OBN].Color] & 0xf00) / 256),
	((AltColors[DBase[OBN].Color] & 0x0f0) / 16),
	((AltColors[DBase[OBN].Color] & 0x00f)));
      DE_Win->Colors[8] = AltColors[DBase[OBN].Color];

      if (! DE_Win->Block[5])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].Color = data;
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[5] = 0;
      break;
      } /* draw pen */
     case 3:
      {
      if (data > 255)
       {
       set(DE_Win->IntStr[3], MUIA_String_Integer, 255);
       break;
       }
      if (data < 0)
       {
       set(DE_Win->IntStr[3], MUIA_String_Integer, 0);
       break;
       }
      DBase[OBN].Red = data;
      SetRGB4(&WCSScrn->ViewPort, 9, 
	(DBase[OBN].Red / 16),
	(DBase[OBN].Grn / 16),
	(DBase[OBN].Blu / 16));
      set(DE_Win->Prop[0], MUIA_Prop_First, data);
      DE_Win->Colors[9] = (DBase[OBN].Red / 16) * 256
	 + (DBase[OBN].Grn / 16) * 16 + DBase[OBN].Blu / 16;
      if (! DE_Win->Block[6])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].Red = data;
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[6] = 0;
      break;
      } /* red */
     case 4:
      {
      if (data > 255)
       {
       set(DE_Win->IntStr[4], MUIA_String_Integer, 255);
       break;
       }
      if (data < 0)
       {
       set(DE_Win->IntStr[4], MUIA_String_Integer, 0);
       break;
       }
      DBase[OBN].Grn = data;
      SetRGB4(&WCSScrn->ViewPort, 9, 
	(DBase[OBN].Red / 16),
	(DBase[OBN].Grn / 16),
	(DBase[OBN].Blu / 16));
      set(DE_Win->Prop[1], MUIA_Prop_First, data);
      DE_Win->Colors[9] = (DBase[OBN].Red / 16) * 256
	 + (DBase[OBN].Grn / 16) * 16 + DBase[OBN].Blu / 16;
      if (! DE_Win->Block[7])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j, 
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].Grn = data;
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[7] = 0;
      break;
      } /* green */
     case 5:
      {
      if (data > 255)
       {
       set(DE_Win->IntStr[5], MUIA_String_Integer, 255);
       break;
       }
      if (data < 0)
       {
       set(DE_Win->IntStr[5], MUIA_String_Integer, 0);
       break;
       }
      DBase[OBN].Blu = data;
      SetRGB4(&WCSScrn->ViewPort, 9, 
	(DBase[OBN].Red / 16),
	(DBase[OBN].Grn / 16),
	(DBase[OBN].Blu / 16));
      set(DE_Win->Prop[2], MUIA_Prop_First, data);
      DE_Win->Colors[9] = (DBase[OBN].Red / 16) * 256
	 + (DBase[OBN].Grn / 16) * 16 + DBase[OBN].Blu / 16;
      if (! DE_Win->Block[8])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].Blu = data;
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[8] = 0;
      break;
      } /* blue */
     case 6:
      {
      if (data > 9)
       {
       set(DE_Win->IntStr[6], MUIA_String_Integer, 9);
       break;
       }
      if (data < 0)
       {
       set(DE_Win->IntStr[6], MUIA_String_Integer, 0);
       break;
       }
      DBase[OBN].MaxFract = data;
      if (! DE_Win->Block[11])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
	 MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].MaxFract = data;
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[11] = 0;
      break;
      } /* max fractal depth */
     } /* switch i */
     
    DB_Mod = 1;
    break;
    } /* Int strings */

   case GP_CYCLE1:
    {
    switch (WCS_ID)
     {
     case ID_DE_LINECYCLE:
      {
      LONG data;

      get(DE_Win->CY_Line, MUIA_Cycle_Active, &data);

      switch (data)
       {
       case 0:
        {
        DBase[OBN].Pattern[0] = 'P';
        break;
        } /* point */
       case 1:
        {
        DBase[OBN].Pattern[0] = 'C';
        break;
        } /* circle */
       case 2:
        {
        DBase[OBN].Pattern[0] = 'R';
        break;
        } /* square */
       case 3:
        {
        DBase[OBN].Pattern[0] = 'X';
        break;
        } /* cross */
       case 4:
        {
        DBase[OBN].Pattern[0] = 'L';
        break;
        } /* solid */
       case 5:
        {
        DBase[OBN].Pattern[0] = 'T';
        break;
        } /* dotted */
       case 6:
        {
        DBase[OBN].Pattern[0] = 'D';
        break;
        } /* dashed */
       case 7:
        {
        DBase[OBN].Pattern[0] = 'B';
        break;
        } /* dashed */
       } /* switch data */
      if (! DE_Win->Block[9])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		MUIV_List_Select_Ask, &state);
        if (state)
         {
         DBase[j].Pattern[0] = DBase[OBN].Pattern[0];
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[9] = 0;
      break;
      } /* LINECYCLE */
     case ID_DE_SPECCYCLE:
      {
      LONG data;

      get(DE_Win->CY_Spec, MUIA_Cycle_Active, &data);

      set(DE_Win->IntStr[6],   MUIA_Disabled, data != 0 && data != 1);
      set(DE_Win->Arrow[3][0], MUIA_Disabled, data != 0 && data != 1);
      set(DE_Win->Arrow[3][1], MUIA_Disabled, data != 0 && data != 1);
      switch (data)
       {
       case 0:
        {
        strcpy(DBase[OBN].Special, "TOP");
        break;
        } /* topo */
       case 1:
        {
        strcpy(DBase[OBN].Special, "SFC");
        break;
        } /* surface */
       case 2:
        {
        strcpy(DBase[OBN].Special, "VEC");
        break;
        } /* vector */
       case 3:
        {
        strcpy(DBase[OBN].Special, "VIL");
        break;
        } /* camera path */
       case 4:
        {
        strcpy(DBase[OBN].Special, "VSG");
        break;
        } /* camera path */
       case 5:
        {
        strcpy(DBase[OBN].Special, "VIS");
        break;
        } /* camera path */
#ifdef ENABLE_VECTOR_PATHS
       case 6:
        {
        strcpy(DBase[OBN].Special, "CAM");
        break;
        } /* camera path */
       case 7:
        {
        strcpy(DBase[OBN].Special, "FOC");
        break;
        } /* focus path */
#endif /* ENABLE_VECTOR_PATHS */
       } /* switch data */
      if (! DE_Win->Block[10])
       {
       for (j=0; j<NoOfObjects; j++)
        {
        DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		MUIV_List_Select_Ask, &state);
        if (state)
         {
         strcpy(DBase[j].Special, DBase[OBN].Special);
         } /* if object selected */
        } /* for j=0... */
       } /* if selection state not changing */
      DE_Win->Block[10] = 0;
      break;
      } /* SPECCYCLE */
     } /* switch WCS_ID */
    DB_Mod = 1;
    break;
    } /* CYCLE1 */

   case GP_PROP1:
    {
    LONG data;

    i = WCS_ID - ID_DE_PROP(0);
    get(DE_Win->Prop[i], MUIA_Prop_First, &data);
    set(DE_Win->IntStr[i + 3], MUIA_String_Integer, data);
    break;
    } /* PROP1 */

   case GP_ARROW1:
    {
    LONG data;

    i = WCS_ID - ID_DE_ARROWLEFT(0);
    if (i > 2) i += 3;
    get(DE_Win->IntStr[i], MUIA_String_Integer, &data);
    data -= 1;
    if (data >= 0)
     {
     set(DE_Win->IntStr[i], MUIA_String_Integer, data);
     }
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    LONG data;

    i = WCS_ID - ID_DE_ARROWRIGHT(0);
    if (i > 2) i += 3;
    get(DE_Win->IntStr[i], MUIA_String_Integer, &data);
    data += 1;
    if (i == 0 && data > NoOfObjects - 1) break;
    set(DE_Win->IntStr[i], MUIA_String_Integer, data);
    break;
    } /* ARROW2 */
#ifdef KHKJHKHSDS
   case GP_ARROW3:
    {
    LONG data;

    get(DE_Win->IntStr[0], MUIA_String_Integer, &data);
    set(DE_Win->IntStr[0], MUIA_String_Integer, (data > 10 ? data - 10: 1));
    break;
    } /* ARROW3 */

   case GP_ARROW4:
    {
    LONG data;

    get(DE_Win->IntStr[0], MUIA_String_Integer, &data);
    set(DE_Win->IntStr[0], MUIA_String_Integer,
	 (data > NoOfObjects - 11 ? NoOfObjects - 1: data + 10));
    break;
    } /* ARROW4 */
#endif
   } /* switch gadget group */

} /* Handle_DE_Window() */

/*********************************************************************/

void Set_DE_Item(short item)
{
 LONG data;

 set(DE_Win->LS_List, MUIA_List_Active, item);
 DoMethod(DE_Win->LS_List, MUIM_List_Jump, item);
/* set(DE_Win->IntStr[0], MUIA_String_Integer, item);*/
 if (DBase[OBN].LineWidth != DBase[item].LineWidth)	 DE_Win->Block[4] = 1;
 if (DBase[OBN].Color != DBase[item].Color)		 DE_Win->Block[5] = 1;
 if (DBase[OBN].Red != DBase[item].Red)			 DE_Win->Block[6] = 1;
 if (DBase[OBN].Grn != DBase[item].Grn)			 DE_Win->Block[7] = 1;
 if (DBase[OBN].Blu != DBase[item].Blu)			 DE_Win->Block[8] = 1;
 if (DBase[OBN].Mark[0] != DBase[item].Mark[0])		 DE_Win->Block[0] = 1;
 if (DBase[OBN].Pattern[0] != DBase[item].Pattern[0])	 DE_Win->Block[9] = 1;
 if (strcmp(DBase[OBN].Layer1, DBase[item].Layer1))	 DE_Win->Block[1] = 1;
 if (strcmp(DBase[OBN].Layer2, DBase[item].Layer2))	 DE_Win->Block[2] = 1;
 if (strcmp(DBase[OBN].Label, DBase[item].Label))	 DE_Win->Block[3] = 1;
 if (strcmp(DBase[OBN].Special, DBase[item].Special))	 DE_Win->Block[10] = 1;
 if (DBase[OBN].MaxFract != DBase[item].MaxFract)	 DE_Win->Block[11] = 1;
 set(DE_Win->IntStr[1], MUIA_String_Integer, DBase[item].LineWidth);
 set(DE_Win->IntStr[2], MUIA_String_Integer, DBase[item].Color);
 set(DE_Win->IntStr[3], MUIA_String_Integer, DBase[item].Red);
 set(DE_Win->IntStr[4], MUIA_String_Integer, DBase[item].Grn);
 set(DE_Win->IntStr[5], MUIA_String_Integer, DBase[item].Blu);
 set(DE_Win->IntStr[6], MUIA_String_Integer, DBase[item].MaxFract);
 set(DE_Win->Str[0], MUIA_Text_Contents, DBase[item].Name);
 set(DE_Win->Str[1], MUIA_String_Contents, DBase[item].Layer1);
 set(DE_Win->Str[2], MUIA_String_Contents, DBase[item].Layer2);
 set(DE_Win->Str[3], MUIA_String_Contents, DBase[item].Label);
 switch (DBase[item].Pattern[0])
  {
  case 'P': data = 0; break;
  case 'C': data = 1; break;
  case 'R': data = 2; break;
  case 'X': data = 3; break;
  case 'L': data = 4; break;
  case 'T': data = 5; break;
  case 'D': data = 6; break;
  case 'B': data = 7; break;
  } /* switch pattern */
 set(DE_Win->CY_Line, MUIA_Cycle_Active, data);
 if      (! strcmp(DBase[item].Special, "TOP")) data = 0;
 else if (! strcmp(DBase[item].Special, "SFC")) data = 1;
 else if (! strcmp(DBase[item].Special, "VIL")) data = 3;
 else if (! strcmp(DBase[item].Special, "VSG")) data = 4;
 else if (! strcmp(DBase[item].Special, "VIS")) data = 5;
#ifdef ENABLE_VECTOR_PATHS
 else if (! strcmp(DBase[item].Special, "CAM")) data = 6;
 else if (! strcmp(DBase[item].Special, "FOC")) data = 7;
#endif /* ENABLE_VECTOR_PATHS */
 else						data = 2;
 set(DE_Win->CY_Spec, MUIA_Cycle_Active, data);
 sprintf(str, "%4d", DBase[item].Points);
 set(DE_Win->PointTxt, MUIA_Text_Contents, str);
 set(DE_Win->Check, MUIA_Selected, (DBase[item].Mark[0] == 'Y'));
 SetRGB4(&WCSScrn->ViewPort, 8, 
	((AltColors[DBase[item].Color] & 0xf00) / 256),
	((AltColors[DBase[item].Color] & 0x0f0) / 16),
	((AltColors[DBase[item].Color] & 0x00f)));
 DE_Win->Colors[8] = AltColors[DBase[item].Color];
 SetRGB4(&WCSScrn->ViewPort, 9, 
	(DBase[item].Red / 16),
	(DBase[item].Grn / 16),
	(DBase[item].Blu / 16));
 DE_Win->Colors[9] = (DBase[OBN].Red / 16) * 256
	 + (DBase[OBN].Grn / 16) * 16 + DBase[OBN].Blu / 16;
 set(DE_Win->IntStr[6],   MUIA_Disabled, strcmp(DBase[item].Special, "TOP")
	&& strcmp(DBase[item].Special, "SFC"));
 set(DE_Win->Arrow[3][0], MUIA_Disabled, strcmp(DBase[item].Special, "TOP")
	&& strcmp(DBase[item].Special, "SFC"));
 set(DE_Win->Arrow[3][1], MUIA_Disabled, strcmp(DBase[item].Special, "TOP")
	&& strcmp(DBase[item].Special, "SFC"));

} /* Set_DE_Item() */

/*********************************************************************/

void Set_DE_List(short update)
{
 short i;

 if (NoOfObjects > DE_Win->MaxDBItems - 20)
  {
  if (! DBList_New(NoOfObjects + 20))
   {
   User_Message("Database Module",
	"Out of memory!\nCan't open database list.", "OK", "o");
   } /* if out of memory */
  } /* if need more list space */

 for (i=0; i<NoOfObjects; i++)
  {
  DE_Win->DBName[i] = &DBase[i].Enabled;
  } /* for i=0... */
 DE_Win->DBName[NoOfObjects] = NULL;

/* Add items or update Database list */
 if (update)
  {
  DoMethod(DE_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(DE_Win->LS_List, MUIM_List_Clear);
  DoMethod(DE_Win->LS_List,
	MUIM_List_Insert, DE_Win->DBName, -1, MUIV_List_Insert_Bottom);
  }

} /* Set_DE_List() */

/*********************************************************************/

STATIC_FCN void Remove_DE_Item(short OBN, short Remove) // used locally only -> static, AF 26.7.2021
{
 char filename[255];
 USHORT MoveItems;
 FILE *fileptr;
 struct DirList *DLItem;

 DoMethod(DE_Win->LS_List, MUIM_List_Select, OBN, MUIV_List_Select_Off, NULL);

/* undraw map */
 if (MapWind0 && vectorenabled && DBase[OBN].Lat)
  {
  struct clipbounds cb;

  setclipbounds(MapWind0, &cb);
  outline(MapWind0, OBN, backpen, &cb);
  } /* if map window open */

 if (Remove == 1)
  {
  DLItem = DL;
  while (DLItem)
   {
   if (DLItem->Read != '*')
    {
    strmfp(filename, DLItem->Name, DBase[OBN].Name);
    strcat(filename, ".Obj");
    if ((fileptr = fopen(filename, "r")) != NULL)
     {
     fclose(fileptr);
     remove(filename);
     } /* if object file exists */
    } /* if not read only */
   DLItem = DLItem->Next;
   } /* while */
  DLItem = DL;
  while (DLItem)
   {
   if (DLItem->Read != '*')
    {
    strmfp(filename, DLItem->Name, DBase[OBN].Name);
    strcat(filename, ".elev");
    if ((fileptr = fopen(filename, "rb")) != NULL)
     {
     fclose(fileptr);
     remove(filename);
     } /* if object file exists */
    } /* if not read only */
   DLItem = DLItem->Next;
   } /* while */
  DLItem = DL;
  while (DLItem)
   {
   if (DLItem->Read != '*')
    {
    strmfp(filename, DLItem->Name, DBase[OBN].Name);
    strcat(filename, ".relel");
    if ((fileptr = fopen(filename, "rb")) != NULL)
     {
     fclose(fileptr);
     remove(filename);
     } /* if object file exists */
    } /* if not read only */
   DLItem = DLItem->Next;
   } /* while */
  } /* if remove files */

 MoveItems = NoOfObjects - 1 - OBN;

/* deallocate vector memory and move pointers */
 freevecarray(OBN);

 memmove(&DBase[OBN], &DBase[OBN + 1], (MoveItems * sizeof (struct database)));
 memset(&DBase[NoOfObjects - 1], 0, sizeof (struct database));

 NoOfObjects --;
 DE_Win->DBName[NoOfObjects] = NULL;

} /* Remove_DE_Item() */

/**********************************************************************/

short DBList_New(short NewRecords)
{
 char **NewList;
 long NewListSize;

 NewListSize = (NewRecords) * (sizeof (char *));
 if ((NewList = (char **)get_Memory(NewListSize, MEMF_CLEAR)) == NULL)
  return (0);

 if (DE_Win->DBName)
  free_Memory(DE_Win->DBName, DE_Win->DBNameSize);
 DE_Win->DBName = NewList;
 DE_Win->DBNameSize = NewListSize;
 DE_Win->MaxDBItems = NewRecords;
 return (1);

} /* DBList_New() */

/**********************************************************************/

void Map_DB_Object(long OBN, long Enabled, long Selected)
{

 if (MapWind0)
  {
  struct clipbounds cb;

  setclipbounds(MapWind0, &cb);
  if (Enabled)
   {
   if (Selected)
    {
    if (strcmp(DBase[OBN].Special, "TOP") && strcmp(DBase[OBN].Special, "SFC"))
     outline(MapWind0, OBN, 2, &cb);
    else
     outline(MapWind0, OBN, 7, &cb);
    } /* if */
   else
    outline(MapWind0, OBN, DBase[OBN].Color, &cb);
   }
  else
   outline(MapWind0, OBN, backpen, &cb);
  } /* if map view open */

} /* Map_DB_Object() */

/**********************************************************************/

void Make_DL_Window(void)
{
 long open;

 if (DL_Win)
  {
  DoMethod(DL_Win->DirListWin, MUIM_Window_ToFront);
  set(DL_Win->DirListWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((DL_Win = (struct DirListWindow *)
	get_Memory(sizeof (struct DirListWindow), MEMF_CLEAR)) == NULL)
   return;

 DL_Win->MaxDLItems = 100;
 DL_Win->DLNameSize = DL_Win->MaxDLItems * (sizeof (char *));
 if ((DL_Win->DLName = (char **)get_Memory(DL_Win->DLNameSize, MEMF_CLEAR)) == NULL)
  {
  User_Message("Database Module",
	"Out of memory!\nCan't open directory list window.", "OK", "o");
  Close_DL_Window(NULL);
  return;
  } /* if out of memory */

 if ((DL_Win->DLCopy = DirList_Copy(DL)) == NULL)
  {
  User_Message("Database Module",
	"Out of memory!\nCan't open directory list window.", "OK", "o");
  Close_DL_Window(NULL);
  return;
  } /* if out of memory */
  strcpy(DL_Win->Dirname, dirname);

  Set_Param_Menu(10);

     DL_Win->DirListWin = WindowObject,
      MUIA_Window_Title		, "Directory List Editor",
      MUIA_Window_ID		, 'DLED',
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, DL_Win->LS_List = ListviewObject,
	      MUIA_Listview_Input, TRUE,
              MUIA_Listview_List, ListObject, ReadListFrame, End,
	  End,

        Child, DL_Win->DefaultTxt = TextObject, TextFrame,
		 MUIA_Text_Contents, dirname, End, 
        Child, DL_Win->BT_Default = KeyButtonFunc('f', "\33cDefault Directory"), 

        Child, HGroup,
          Child, DL_Win->BT_Add = KeyButtonFunc('d', "\33cAdd..."), 
          Child, DL_Win->BT_Swap = KeyButtonFunc('s', "\33cSwap..."), 
          Child, DL_Win->BT_Move = KeyButtonFunc('m', "\33cMove..."), 
          Child, DL_Win->BT_Remove = KeyButtonFunc('r', "\33cRemove"), 
          Child, DL_Win->BT_ReadOnly = KeyButtonObject('*'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c*Read Only", End, 
          End, /* HGroup */

        Child, HGroup,
          Child, DL_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"), 
          Child, DL_Win->BT_Load = KeyButtonFunc('l', "\33cLoad"), 
          Child, DL_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"), 
          End, /* HGroup */

        End, /* VGroup */
      End; /* WindowObject DL_Win->DirListWin */

  if (! DL_Win->DirListWin)
   {
   Close_DL_Window(DL_Win->DLCopy);
   User_Message("Directory List", "Out of memory!", "OK", "o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DL_Win->DirListWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(DL_Win->DirListWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DL_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, DL_Win->BT_Apply, ID_DL_APPLY,
   DL_Win->BT_Cancel, ID_DL_CANCEL, DL_Win->BT_Remove, ID_DL_REMOVE,
   DL_Win->BT_Add, ID_DL_ADD, DL_Win->BT_Swap, ID_DL_SWAP,
   DL_Win->BT_Move, ID_DL_MOVE, DL_Win->BT_Default, ID_DL_DEFAULT,
   DL_Win->BT_Load, ID_DL_LOAD, NULL);

  DoMethod(DL_Win->BT_ReadOnly, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DL_READONLY);  

/* Set tab cycle chain */
  DoMethod(DL_Win->DirListWin, MUIM_Window_SetCycleChain,
	DL_Win->BT_Add, DL_Win->BT_Swap, DL_Win->BT_Move,
	DL_Win->BT_Remove, DL_Win->BT_Default, DL_Win->BT_Apply, DL_Win->BT_Load,
	DL_Win->BT_Cancel, NULL);

/* Set active gadget */
  set(DL_Win->DirListWin, MUIA_Window_ActiveObject, DL_Win->BT_Add);

/* Create directory list */
  Set_DL_List(DL, 0, 0);

/* link list to application */
  DoMethod(DL_Win->LS_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_DL_LIST);

/* Open window */
  set(DL_Win->DirListWin, MUIA_Window_Open, TRUE);
  get(DL_Win->DirListWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DL_Window(DL_Win->DLCopy);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DL_Win->DirListWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DL_ACTIVATE);

/* Get Window structure pointer */
  get(DL_Win->DirListWin, MUIA_Window_Window, &DL_Win->Win);

} /* Make_DL_Window() */

/*********************************************************************/

void Close_DL_Window(struct DirList *DLDel)
{

 if (DLDel) DirList_Del(DLDel);
 
 if (DL_Win)
  {
  if (DL_Win->DirListWin)
   {
   set(DL_Win->DirListWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, DL_Win->DirListWin);
   MUI_DisposeObject(DL_Win->DirListWin);
   } /* if window created */
  if (DL_Win->DLName) free_Memory(DL_Win->DLName, DL_Win->DLNameSize);
  free_Memory(DL_Win, sizeof (struct DirListWindow));
  DL_Win = NULL;
  } /* if memory allocated */

} /* Close_DL_Window() */

/*********************************************************************/

void Handle_DL_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_DL_Window();
   return;
   } /* Open Directory List Window */

  if (! DL_Win)
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
     case ID_DL_ADD:
      {
      if (DirList_Add(DL, NULL, 0))
       {
       Set_DL_List(DL, 0, -1);
       DL_Win->Proj_Mod = 1;
       } /* if added OK */
      break;
      }
     case ID_DL_SWAP:
      {
      ULONG SwapWith_ID;
      long Item;
      struct DirList *DLItem1, *DLItem2;

      get(DL_Win->LS_List, MUIA_List_Active, &Item);
      DLItem1 = DirList_Search(DL, (short)Item);
      SetPointer(DL_Win->Win, SwapPointer, 16, 16, 0, 0);
      SwapWith_ID = GetInput_ID();
      if ((SwapWith_ID & 0xffffffff) == ID_DL_LIST)
       {
       get(DL_Win->LS_List, MUIA_List_Active, &Item);
       DLItem2 = DirList_Search(DL, (short)Item);
       swmem(&DLItem1->Read, &DLItem2->Read, 256);
       Set_DL_List(DL, 0, Item);
       DL_Win->Proj_Mod = 1;
       } /* if a list item selected */
      ClearPointer(DL_Win->Win);
      break;
      }
     case ID_DL_MOVE:
      {
      ULONG MoveTo_ID;
      long Move, MoveTo;

      get(DL_Win->LS_List, MUIA_List_Active, &Move);
      SetPointer(DL_Win->Win, CopyPointer, 16, 16, 0, -5);
      MoveTo_ID = GetInput_ID();
      if ((MoveTo_ID & 0xffffffff) == ID_DL_LIST)
       {
       get(DL_Win->LS_List, MUIA_List_Active, &MoveTo);
       DirList_Move(DL, (short)Move, (short)MoveTo);
       Set_DL_List(DL, 0, MoveTo);
       DL_Win->Proj_Mod = 1;
       } /* if a list item selected */
      ClearPointer(DL_Win->Win);
      break;
      }
     case ID_DL_REMOVE:
      {
      long Item;

      get(DL_Win->LS_List, MUIA_List_Active, &Item);
      DL = DirList_Remove(DL, (short)Item);
      Set_DL_List(DL, 0, Item);
      DL_Win->Proj_Mod = 1;
      break;
      }
     case ID_DL_DEFAULT:
      {
      long item;
      struct DirList *DLDef;

      get(DL_Win->LS_List, MUIA_List_Active, &item);
      DLDef = DirList_Search(DL, (short)item);
      strcpy(dirname, DLDef->Name);
      set(DL_Win->DefaultTxt, MUIA_Text_Contents, dirname);
      DL_Win->Proj_Mod = 1;
      break;
      }
     case ID_DL_READONLY:
      {
      long item, state;
      struct DirList *DLItem;

      get(DL_Win->LS_List, MUIA_List_Active, &item);
      DLItem = DirList_Search(DL, (short)item);
      get(DL_Win->BT_ReadOnly, MUIA_Selected, &state);
      if (state) DLItem->Read = '*';
      else DLItem->Read = ' ';
      DoMethod(DL_Win->LS_List, MUIM_List_Redraw, item);
      DL_Win->Proj_Mod = 1;
      break;
      }      
     case ID_DL_LOAD:
      {
      if (LoadDirList())
       {
       Set_DL_List(DL, 0, 0);
       set(DL_Win->DefaultTxt, MUIA_Text_Contents, dirname);
       }
      break;
      }
     case ID_DL_APPLY:
      {
      Proj_Mod |= DL_Win->Proj_Mod;
      Close_DL_Window(DL_Win->DLCopy);
      break;
      }
     case ID_DL_CANCEL:
      {
      strcpy(dirname, DL_Win->Dirname);
      swmem(DL, DL_Win->DLCopy, sizeof (struct DirList *));
      Close_DL_Window(DL_Win->DLCopy);
      break;
      }
     case ID_DL_CLOSEQUERY:
      {
      if (! CloseWindow_Query("Directory List"))
       {
       strcpy(dirname, DL_Win->Dirname);
       swmem(DL, DL_Win->DLCopy, sizeof (struct DirList *));
       } /* if cancel changes */
      else
       Proj_Mod |= DL_Win->Proj_Mod;
      Close_DL_Window(DL_Win->DLCopy);
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_LIST1:
    {
    long item;

    get(DL_Win->LS_List, MUIA_List_Active, &item);
    set(DL_Win->BT_ReadOnly, MUIA_Selected, (DL_Win->DLName[item][0] == '*'));
    break;
    } /* LIST1 */
   } /* switch gadget group */

} /* Handle_DL_Window() */

/***********************************************************************/

STATIC_FCN void Set_DL_List(struct DirList *DLItem, short Update, short ActiveItem) // used locally only -> static, AF 26.7.2021
{
 short i = 0;

 while (DLItem)
  {
  DL_Win->DLName[i] = &DLItem->Read;
  DLItem = DLItem->Next;
  i ++;
  } /* for i=0... */
 DL_Win->DLName[i] = NULL;

 if (ActiveItem < 0 || ActiveItem > i - 1) ActiveItem = i - 1;

/* Add items or update directory list */
 if (Update)
  {
  DoMethod(DL_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
  }
 else
  {
  DoMethod(DL_Win->LS_List, MUIM_List_Clear);
  DoMethod(DL_Win->LS_List,
	MUIM_List_Insert, DL_Win->DLName, -1, MUIV_List_Insert_Bottom);
  }

 set(DL_Win->LS_List, MUIA_List_Active, ActiveItem);
 set(DL_Win->BT_ReadOnly, MUIA_Selected, (DL_Win->DLName[ActiveItem][0] == '*'));

} /* Set_DL_List() */

/************************************************************************/

void Update_DL_Win(void)
{

 Set_DL_List(DL, 0, -1);
 set(DL_Win->DefaultTxt, MUIA_Text_Contents, dirname);

} /* Update_DL_Win() */

/************************************************************************/

short Add_DE_NewItem(void)
{

 if (NoOfObjects > DE_Win->MaxDBItems)
  {
  if (! DBList_New(NoOfObjects + 20))
   {
   User_Message("Database Module",
		"Out of memory expanding database!\nOperation terminated.", "OK", "o");
   return (0);
   } /* if new list fails */
  } /* if need bigger list */
 DE_Win->DBName[OBN] = &DBase[OBN].Enabled;
 DE_Win->DBName[NoOfObjects] = NULL;
 DoMethod(DE_Win->LS_List, MUIM_List_Insert, &DE_Win->DBName[OBN], 1,
	MUIV_List_Insert_Bottom);
 Set_DE_Item(OBN);

 return (1);

} /* Add_DE_NewItem() */

