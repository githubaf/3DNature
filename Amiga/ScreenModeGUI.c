/* ScreenMode.c
** MUI ScreenMode requester
** Created on 24 Mar 1994 from code mistakenly written into WCS.c
** by Chris "Xenon" Hanson
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"
#include "GUIDefines.h"


struct WCSScreenMode *ModeList_New(void)
{
struct NameInfo ModeName;
struct DisplayInfo Properties;
struct DimensionInfo Sizes;
struct WCSScreenMode *ModeList, *ThisMode, *Sort, DummyFirst;
ULONG DInfoID;
int Order;

DummyFirst.Next = NULL;

ThisMode = NULL;
ModeList = &DummyFirst;
DInfoID = INVALID_ID;
DInfoID = NextDisplayInfo(DInfoID);
while (DInfoID != INVALID_ID)
 {
 if(!(ModeNotAvailable(DInfoID)))
  {
  if(GetDisplayInfoData(NULL, (UBYTE *)&ModeName, sizeof(ModeName),
   DTAG_NAME, DInfoID))  // (NameInfo)      - a user friendly way to refer to this mode.
   {
   if(GetDisplayInfoData(NULL, (UBYTE *)&Sizes, sizeof(Sizes),
    DTAG_DIMS, DInfoID))  // (DimensionInfo) - default dimensions and overscan info.
    {
	// StdOScan editable via preferences   --  Screenmode at least 500x300 with at least 16 colors
    if((Sizes.StdOScan.MaxX - Sizes.StdOScan.MinX + 1 >= 500) &&
     (Sizes.StdOScan.MaxY - Sizes.StdOScan.MinY + 1 >= 300) && (Sizes.MaxDepth >= 4))
     {
     if((ThisMode = get_Memory(sizeof(struct WCSScreenMode), MEMF_CLEAR)))
      {
      ThisMode->ModeID = DInfoID;
      strcpy(ThisMode->ModeName, (char*)ModeName.Name);
      ThisMode->X  = ThisMode->UX = ThisMode->OX =Sizes.Nominal.MaxX - Sizes.Nominal.MinX + 1;
      ThisMode->Y  = ThisMode->UY = ThisMode->OY =Sizes.Nominal.MaxY - Sizes.Nominal.MinY + 1;
      ThisMode->OScans[0].x = Sizes.TxtOScan.MaxX - Sizes.TxtOScan.MinX + 1;
      ThisMode->OScans[0].y = Sizes.TxtOScan.MaxY - Sizes.TxtOScan.MinY + 1;
      ThisMode->OScans[1].x = Sizes.StdOScan.MaxX - Sizes.StdOScan.MinX + 1;
      ThisMode->OScans[1].y = Sizes.StdOScan.MaxY - Sizes.StdOScan.MinY + 1;
      ThisMode->OScans[2].x = Sizes.MaxOScan.MaxX - Sizes.MaxOScan.MinX + 1;
      ThisMode->OScans[2].y = Sizes.MaxOScan.MaxY - Sizes.MaxOScan.MinY + 1;
      ThisMode->OScans[3].x = Sizes.VideoOScan.MaxX - Sizes.VideoOScan.MinX + 1;
      ThisMode->OScans[3].y = Sizes.VideoOScan.MaxY - Sizes.VideoOScan.MinY + 1;
      ThisMode->MaxX = Sizes.MaxRasterWidth;   // maximum width in pixels
      ThisMode->MaxY = Sizes.MaxRasterHeight;  // maximum height in pixels
      ThisMode->MaxDepth = Sizes.MaxDepth;     // log2( max number of colors )

      if(GetDisplayInfoData(NULL, (UBYTE *)&Properties, sizeof(Properties),
       DTAG_DISP, DInfoID))
       {
       ThisMode->PropertyFlags = Properties.PropertyFlags;  // Properties of this mode see define (DIPF_IS_LACE, DUALPF, PF2PRI, HAM...)
       ThisMode->PixelSpeed = Properties.PixelSpeed;
       } /* if */
      for(Sort = ModeList; Sort;)
       {
       if(Sort->Next)
        {
        Order = stricmp(Sort->Next->ModeName, ThisMode->ModeName);
        if (Order > 0)
         {
         ThisMode->Next = Sort->Next;
         Sort->Next = ThisMode;
         Sort = NULL; /* We're done. */
         } /* else */
        else if (Order == 0)
         {
         /* We already have an entry by that name, don't
         ** bother adding a new one. Break out. */
         free_Memory(ThisMode, sizeof(struct WCSScreenMode));
         Sort = NULL;
         } /* else */ 
        } /* if */
       else
        {
/*        ThisMode->Next = NULL; */ /* Unnecessary: MEMF_CLEAR */
        Sort->Next = ThisMode;
        Sort = NULL; /* break out */
        } /* else */
       if(Sort) /* prevent many fine enforcer hits */
        {
        Sort = Sort->Next;
        } /* if */
       } /* for */
      } /* if */
     } /* if */
    } /* if */
   } /* if */
  } /* if */
 
 DInfoID = NextDisplayInfo(DInfoID);
 } /* while */

ModeList = ModeList->Next; /* Pull off DummyFirst */
    
return(ModeList);
} /* ModeList_New() */


struct WCSScreenMode *ModeList_Choose(struct WCSScreenMode *This,
 struct WCSScreenData *ScrnData)
{
static const char *Cycle_OSCAN[6];

static const char *NumColorsStrings[8];

struct WCSScreenMode *Scan, *Selected;
APTR ModeSelWin, SM_Save, SM_Use, SM_Exit, SM_Width, SM_Height, SM_List, SM_Text, SM_OSCAN, SM_COLORS,SM_COLORS_TEXT;
ULONG Signalz, Finished, ReturnID;
int CheckVal, Update;
char *ModeText, ModeInfo[255];

Cycle_OSCAN[0]= (const char*)GetString( MSG_SCNRMODGUI_NONE );
Cycle_OSCAN[1]= (const char*)GetString( MSG_SCNRMODGUI_TEXT );
Cycle_OSCAN[2]= (const char*)GetString( MSG_SCNRMODGUI_STANDARD );
Cycle_OSCAN[3]= (const char*)GetString( MSG_SCNRMODGUI_MAX );
Cycle_OSCAN[4]= (const char*)GetString( MSG_SCNRMODGUI_VIDEO );
Cycle_OSCAN[5]=NULL;

NumColorsStrings[ 0]= "    16";  // default
NumColorsStrings[ 1]= "    32";
NumColorsStrings[ 2]= "    64";
NumColorsStrings[ 3]= "   128";
NumColorsStrings[ 4]= "   256";
NumColorsStrings[ 5]= " 32768"; // RTG
NumColorsStrings[ 6]= " 65536"; // RTG
NumColorsStrings[ 7]= "   16M"; // RTG




ModeSelWin = WindowObject,
 MUIA_Window_Title,  GetString( MSG_SCNRMODGUI_WORLDCONSTRUCTIONSETSCREENMODE ) ,
 MUIA_Window_ID, "SCRN",
 MUIA_Window_SizeGadget, TRUE,
 WindowContents, VGroup,
  Child, ColGroup(2), MUIA_Group_SameWidth, TRUE,
    MUIA_Group_HorizSpacing, 4, MUIA_Group_VertSpacing, 3,
   Child, VGroup, /* MUIA_HorizWeight, 150, */
    Child, TextObject, MUIA_Text_Contents,  GetString( MSG_SCNRMODGUI_DISPLAYMODE ) , End,
    Child, SM_List = ListviewObject,
     MUIA_Listview_Input, TRUE,
     MUIA_Listview_List, ListObject, ReadListFrame, End,
     End,
    End,
   Child, VGroup,
    Child, TextObject, MUIA_Text_Contents,  GetString( MSG_SCNRMODGUI_MODEINFORMATION ) , End,
    Child, SM_Text = FloattextObject, ReadListFrame,
     MUIA_Floattext_Text,  GetString( MSG_SCNRMODGUI_MODEESUTOCANNATTRIBUTESN ) , End,
    End,

	/////////////  AF: We need numbers of colors
Child,VGroup,
   Child, HGroup,
    Child, Label2( GetString( MSG_SCNRMODGUI_COLORS )),
	Child, SM_COLORS_TEXT=TextObject,NoFrame, MUIA_Text_Contents, NumColorsStrings[ 0], MUIA_FixWidthTxt, NumColorsStrings[ 0], End,
    Child, SM_COLORS = PropObject, PropFrame,
        	MUIA_Prop_Horiz, TRUE,
        	MUIA_Prop_Entries, 7,
        //	MUIA_Prop_Visible, 7,     // Alexander: Change this according depending on max colors of Screenmode
        	MUIA_Prop_First, 0, End,
        	End,
    //   	/////////////
	Child, HGroup,
    Child, Label2( GetString( MSG_SCNRMODGUI_OVERSCAN ) ),
    Child, SM_OSCAN = CycleObject, MUIA_Cycle_Entries, Cycle_OSCAN, End,
    End,
End,
   Child, HGroup,
    Child, RectangleObject, End,
    Child, HGroup, MUIA_Group_HorizSpacing, 0,
     Child, Label2( GetString( MSG_SCNRMODGUI_WIDTH ) ), /* No End (in sight) */
     Child, SM_Width = StringObject, StringFrame,
      MUIA_String_Integer, 0,
      MUIA_String_Accept, "0123456789",
      MUIA_FixWidthTxt, "01234",
      End,
     End,
    Child, RectangleObject, End,
    Child, HGroup, MUIA_Group_HorizSpacing, 0,
     Child, Label2( GetString( MSG_SCNRMODGUI_HEIGHT ) ), /* No End (in sight) */
     Child, SM_Height = StringObject, StringFrame,
      MUIA_String_Integer, 0,
      MUIA_String_Accept, "0123456789",
      MUIA_FixWidthTxt, "01234",
      End,
     End,
    Child, RectangleObject, End,
    End,
   End,
  Child, RectangleObject, MUIA_FixHeight, 4, End,
  Child, HGroup, MUIA_HorizWeight, 1,
   /* Button button button. Who's got the button? */
   MUIA_Group_SameSize, TRUE,
   Child, SM_Save = KeyButtonObject('s'), MUIA_Text_Contents,  GetString( MSG_SCNRMODGUI_SAVE ) , MUIA_HorizWeight, 200, End,
   Child, RectangleObject, MUIA_HorizWeight, 1, End,
   Child, SM_Use  = KeyButtonObject('u'), MUIA_Text_Contents,  GetString( MSG_SCNRMODGUI_USE ) , MUIA_HorizWeight, 200, End,
   Child, RectangleObject, MUIA_HorizWeight, 1, End,
   Child, SM_Exit = KeyButtonObject('e'), MUIA_Text_Contents,  GetString( MSG_SCNRMODGUI_EXIT ) , MUIA_HorizWeight, 200, End,
   End,
  End,
 End;

if(ModeSelWin)
 {
 // Colors-Slider should change numbers of Colors text
	// we use a ReturnID and handle it below
	 DoMethod(SM_COLORS,MUIM_Notify,MUIA_Prop_First, MUIV_EveryTime,
			 app, 2, MUIM_Application_ReturnID, ID_SM_COLORS);

 DoMethod(ModeSelWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
  app, 2, MUIM_Application_ReturnID, ID_SM_EXIT);

 MUI_DoNotiPresFal(app, SM_Exit, ID_SM_EXIT,
  SM_Use, ID_SM_USE, SM_Save, ID_SM_SAVE, NULL);
 
 DoMethod(SM_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
  app, 2, MUIM_Application_ReturnID, ID_SM_LIST);
 DoMethod(SM_OSCAN, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
  app, 2, MUIM_Application_ReturnID, ID_SM_OSCAN);
 DoMethod(SM_Width, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  app, 2, MUIM_Application_ReturnID, ID_SM_WIDTH);
 DoMethod(SM_Height, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  app, 2, MUIM_Application_ReturnID, ID_SM_HEIGHT);

 DoMethod(ModeSelWin, MUIM_Window_SetCycleChain,
  SM_List, SM_Save, SM_Use, SM_Exit, SM_OSCAN, SM_Width, SM_Height, NULL);

 set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Save);

 for(Scan = This; Scan; Scan = Scan->Next)
  {
  DoMethod(SM_List, MUIM_List_InsertSingle, Scan->ModeName, MUIV_List_Insert_Sorted);
  } /* for */
 
 set(SM_List, MUIA_List_Active, MUIV_List_Active_Top);

 DoMethod(app, OM_ADDMEMBER, ModeSelWin);
#ifdef WCS_MUI_2_HACK
/* This is not needed here, but will be when ScreenMode selection
** can be done from within the program. */

/*    MUI2_MenuCheck_Hack(); */
#endif /* WCS_MUI_2_HACK */
 set(ModeSelWin, MUIA_Window_Open, TRUE);
   } /* if */

Selected = This;

for(Finished = 0; !Finished;)
 {
 if((ReturnID = DoMethod(app, MUIM_Application_Input, &Signalz)))
  {
  switch(ReturnID)
   {
   case ID_SM_OSCAN:
    {
    /* Do not break, fall through to below. */
    } /* ID_SM_OSCAN */
   case ID_SM_LIST:
    {
    get(SM_List, MUIA_List_Active, &CheckVal);
    DoMethod(SM_List, MUIM_List_GetEntry, CheckVal, &ModeText);
    for(Scan = This; Scan;)
     {
     Selected = Scan;
     if((char *)&Scan->ModeName == ModeText)
      {
      Scan = NULL;
      } /* if */
     else
      Scan = Scan->Next;
     } /* for */

    get(SM_OSCAN, MUIA_Cycle_Active, &CheckVal);
    Selected->Overscan=CheckVal;

    if(CheckVal == 0)             // No Overscan
     {
     Selected->OX = Selected->X;  // Then use default values of Screenmode: Sizes.Nominal.MaxX - Sizes.Nominal.MinX + 1
     Selected->OY = Selected->Y;
     } /* if */
    else
     {
     Selected->OX = Selected->OScans[CheckVal - 1].x;  // if we have overscan, use dimesions of that Overscan Mode
     Selected->OY = Selected->OScans[CheckVal - 1].y;  // if we have overscan, use dimesions of that Overscan Mode
     } /* else */

    // Doch
    Update = 1;
    if((Selected->UX == Selected->OX) && (Selected->UY == Selected->OY))
    {
    	Update = 0;  // Width and height string gadgets do not need to be updated.
    }


// Selected->UX/Y have nothing to do with overscan
//Doch
    if(Update)  // user width/heigt string gadgets need to be updated
     {
     Selected->UX = Selected->OX;
     Selected->UY = Selected->OY;
     } /* if */

    printf("Alexander MaxDepth= %d\n", Selected->MaxDepth);
    switch(Selected->MaxDepth)
    {
    	case 15:
    		set(SM_COLORS,MUIA_Prop_Entries,5); // limit slide range
    		set(SM_COLORS,MUIA_Prop_First,0);   // force notification
    		set(SM_COLORS,MUIA_Prop_First,5);   // 32k Colors
    		set(SM_COLORS,MUIA_Disabled, TRUE);
    		break;
    	case 16:
    		set(SM_COLORS,MUIA_Prop_Entries,6); // limit slide range
    		set(SM_COLORS,MUIA_Prop_First,0);   // force notification
    		set(SM_COLORS,MUIA_Prop_First,6);   // 64k Colors
    		set(SM_COLORS,MUIA_Disabled, TRUE);
    		break;
    	case 24:
    		set(SM_COLORS,MUIA_Prop_Entries,7); // full slide range
    		set(SM_COLORS,MUIA_Prop_First,0);   // force notification
    		set(SM_COLORS,MUIA_Prop_First,7);   // 16M Colors
    		set(SM_COLORS,MUIA_Disabled, TRUE);
    		break;
    	default:
    		set(SM_COLORS,MUIA_Prop_First,0);   // 16 Colors default
    		if(Selected->MaxDepth<=8)
    		{
    			set(SM_COLORS,MUIA_Disabled, FALSE);
    			set(SM_COLORS,MUIA_Prop_Entries,Selected->MaxDepth-4);  // limit range of slider to MaxDepth
    			Selected->Depth=4;

    		}
    		else
    		{
    			printf("Invalid MaxDepth %d!\n",Selected->MaxDepth);
    			set(SM_COLORS,MUIA_Prop_First,0);  // 16 Colors default  // strange! ->  permit only Depth = 4
    			set(SM_COLORS,MUIA_Disabled, TRUE);
    		}
    }


/*
       "Mode: 0x%08lx\n                Selected->ModeID
        Res : %dx%d - %dx%d\n          Selected->X, Selected->Y  -  Selected->OX, Selected->OY
		Auto: %dx%d\n                  Selected->MaxX, Selected->MaxY
		Scan: %dns\n\n                 Selected->PixelSpeed
		Attributes\n"                  LACED, HAM, ...
*/
    sprintf(ModeInfo,  GetString( MSG_SCNRMODGUI_MODE0XESXXUTOXCANNSNATTRIBUTES ) ,
     Selected->ModeID, Selected->X, Selected->Y, Selected->OX, Selected->OY,
     Selected->MaxX, Selected->MaxY, Selected->PixelSpeed);
    
    if(Selected->PropertyFlags & DIPF_IS_LACE)
     strcat(ModeInfo, GetString( MSG_SCNRMODGUI_LACED ) );
    if(Selected->PropertyFlags & DIPF_IS_HAM)
     strcat(ModeInfo,"HAM ");
    if(Selected->PropertyFlags & DIPF_IS_ECS)
     strcat(ModeInfo,"ECS ");
    if(Selected->PropertyFlags & DIPF_IS_AA)
     strcat(ModeInfo,"AGA ");
    if(Selected->PropertyFlags & DIPF_IS_PAL)
     strcat(ModeInfo,"PAL ");
    if(Selected->PropertyFlags & DIPF_IS_GENLOCK)
     strcat(ModeInfo, GetString( MSG_SCNRMODGUI_GENLOCKABLE ) );
    if(Selected->PropertyFlags & DIPF_IS_DRAGGABLE)
     strcat(ModeInfo, GetString( MSG_SCNRMODGUI_DRAGGABLE ) );
    if(Selected->PropertyFlags & DIPF_IS_PANELLED)
     strcat(ModeInfo,"Panelled ");
    if(Selected->PropertyFlags & DIPF_IS_EXTRAHALFBRITE)
     strcat(ModeInfo,"EHB ");
    if(Selected->PropertyFlags & DIPF_IS_FOREIGN)
     strcat(ModeInfo,"Foreign ");
    
    set(SM_Text, MUIA_Floattext_Text, (IPTR)ModeInfo);
    
    set(SM_Width, MUIA_String_Integer, Selected->UX);  // SM_Width/Height are the string gadgets
    set(SM_Height, MUIA_String_Integer, Selected->UY);

    // Alexander Depth-Slider auf 4 oder 15/16/24 Bit setzen.
    // bei 15/16/24 Bit Slider sperrenOScans
    // slider-Bereich anpassen

    
    break;
    } /* ID_SM_LIST */
   case ID_SM_HEIGHT: // user entered new height into string gadget
    {
    get(SM_Height, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OY)   // user entered less that Overscan resolution ?
     {
     CheckVal = Selected->OY; // then correct the input! No less than Overscan resolution
     set(SM_Height, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
     } /* if */
    else if(CheckVal > Selected->MaxY) // user entered value bigger than permitted for this ScreenMode
     {
     CheckVal = Selected->MaxY; // then correct the input! No bigger than Max resolution
     set(SM_Height, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
     } /* else if */
    else
    {
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);  // focus to next gadget
    }
    Selected->UY = CheckVal;  // take the selected (and maybe corrected) height value
    break;
    } /* ID_SM_HEIGHT */
   case ID_SM_WIDTH: // user entered new width into string gadget
    {
    get(SM_Width, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OX)   // user entered less that Overscan resolution ?
     {
     CheckVal = Selected->OX; // then correct the input! No less than Overscan resolution
     set(SM_Width, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);
     } /* if */
    else if(CheckVal > Selected->MaxX) // user entered value bigger than permitted for this ScreenMode
     {
     CheckVal = Selected->MaxX; // then correct the input! No bigger than Max resolution
     set(SM_Width, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);
     } /* else if */
    else
    {
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
    }
    Selected->UX = CheckVal;   // take the selected (and maybe corrected) width value
    break;
    } /* ID_SM_WIDTH */
   case ID_SM_USE:
   case ID_SM_SAVE:
    {
    
    Finished = 1;
    
    get(SM_Height, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OY) // user entered less that Overscan resolution ?
     {
     CheckVal = Selected->OY;  // then correct the input! No less than Overscan resolution
     set(SM_Height, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    if(CheckVal > Selected->MaxY) // user entered value bigger than permitted for this ScreenMode
     {
     CheckVal = Selected->MaxY; // then correct the input! No bigger than Max resolution
     set(SM_Height, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    Selected->UY = CheckVal;

    get(SM_Width, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OX) // user entered less that Overscan resolution ?
     {
     CheckVal = Selected->OX; // then correct the input! No less than Overscan resolution
     set(SM_Width, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    if(CheckVal > Selected->MaxX) // user entered value bigger than permitted for this ScreenMode
     {
     CheckVal = Selected->MaxX; // then correct the input! No bigger than Max resolution
     set(SM_Width, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    Selected->UX = CheckVal;
    
    printf("Alexander: UX=%d\n",Selected->UX);
    printf("Alexander: UY=%d\n",Selected->UY);

    if(ReturnID==ID_SM_SAVE)  // if ID_SM_SAVE than additinally to use-code we store the use-values into ScrnData for saving at the end.
    {
    ScrnData->ModeID = Selected->ModeID;
    ScrnData->Width = Selected->UX;
    ScrnData->Height = Selected->UY;
    ScrnData->Depth=Selected->Depth;
    ScrnData->OTag=Selected->Overscan ? SA_Overscan : TAG_IGNORE;
    ScrnData->OVal=Selected->Overscan;
    printf("Alexander: ID_SM_SAVE ScrnData->Depth=Selected->Depth=%d\n",ScrnData->Depth);
    /* Do something more here */
    } /* ID_SM_USE, ID_SM_SAVE */


    if(Finished)
     {
     DoMethod(app, OM_REMMEMBER, ModeSelWin);
     MUI_DisposeObject(ModeSelWin);
     return(Selected);
     } /* if */

    break;
    } /* ID_SM_USE */
   case ID_SM_EXIT:
    {
    return(NULL);
    break;
    } /* ID_SM_EXIT */
   case ID_SM_COLORS:
    {
    	// Alexander
    	static char String[16];
    	char ColorDepth[]={4,5,6,7,8,15,16,24};
       get(SM_COLORS, MUIA_Prop_First, &CheckVal);
//  printf("Alexander: Colors: %ld\n",CheckVal);
	   snprintf(String,8,"%s",NumColorsStrings[CheckVal]);
	   set(SM_COLORS_TEXT, MUIA_Text_Contents,String);
	   printf("Alexander: ColorDepth=%d\n",ColorDepth[CheckVal]);
	   Selected->Depth=ColorDepth[CheckVal];
	   break;
    }
   } /* switch */
  } /* if */
 else
  {
  if(!Finished)
   {
   Wait(Signalz);
   } /* if */
  } /* else */
 } /* for */


#ifdef PRINTMODES_DEBUG
for(Scan = This; Scan; Scan = Scan->Next)
 {
 printf("%08lx: ", Scan->ModeID);
 printf("\"%s\" ", Scan->ModeName);
 printf("%ld x %ld - %ld x %ld.\n", Scan->X, Scan->Y, Scan->OX, Scan->OY);
 printf("%dns ", Scan->PixelSpeed);
 if(Scan->PropertyFlags & DIPF_IS_LACE)
  printf( GetString( MSG_SCNRMODGUI_LACED ) );
 if(Scan->PropertyFlags & DIPF_IS_HAM)
  printf("HAM ");
 if(Scan->PropertyFlags & DIPF_IS_ECS)
  printf("ECS ");
 if(Scan->PropertyFlags & DIPF_IS_AA)
  printf("AGA ");
 if(Scan->PropertyFlags & DIPF_IS_PAL)
  printf("PAL ");
 if(Scan->PropertyFlags & DIPF_IS_GENLOCK)
  printf( GetString( MSG_SCNRMODGUI_GENLOCKABLE ) );
 if(Scan->PropertyFlags & DIPF_IS_DRAGGABLE)
  printf( GetString( MSG_SCNRMODGUI_DRAGGABLE ) );
 if(Scan->PropertyFlags & DIPF_IS_PANELLED)
  printf("Panelled ");
 if(Scan->PropertyFlags & DIPF_IS_EXTRAHALFBRITE)
  printf("EHB ");
 if(Scan->PropertyFlags & DIPF_IS_FOREIGN)
  printf("Foreign ");
 printf("\n");

 } /* for */
#endif /* PRINTMODES_DEBUG */
return(NULL);

} /* ModeList_Choose() */


void ModeList_Del(struct WCSScreenMode *ModeList)
{
struct WCSScreenMode *ThisMode;

while(ModeList) /* Free of these Earthly chains... */
 {
 ThisMode = ModeList->Next;
 free_Memory(ModeList, sizeof(struct WCSScreenMode));
 ModeList = ThisMode;
 } /* while */

} /* ModeList_Del() */

