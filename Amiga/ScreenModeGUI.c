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
   DTAG_NAME, DInfoID))
   {
   if(GetDisplayInfoData(NULL, (UBYTE *)&Sizes, sizeof(Sizes),
    DTAG_DIMS, DInfoID))
    {
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
      ThisMode->MaxX = Sizes.MaxRasterWidth;
      ThisMode->MaxY = Sizes.MaxRasterHeight;
      if(GetDisplayInfoData(NULL, (UBYTE *)&Properties, sizeof(Properties),
       DTAG_DISP, DInfoID))
       {
       ThisMode->PropertyFlags = Properties.PropertyFlags;
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

static const char *Cycle_Colors[11];

struct WCSScreenMode *Scan, *Selected;
APTR ModeSelWin, SM_Save, SM_Use, SM_Exit, SM_Width, SM_Height, SM_List, SM_Text, SM_OSCAN, SM_COLORS;
ULONG Signalz, Finished, ReturnID;
int CheckVal, Update;
char *ModeText, ModeInfo[255];

Cycle_OSCAN[0]= (const char*)GetString( MSG_SCNRMODGUI_NONE );
Cycle_OSCAN[1]= (const char*)GetString( MSG_SCNRMODGUI_TEXT );
Cycle_OSCAN[2]= (const char*)GetString( MSG_SCNRMODGUI_STANDARD );
Cycle_OSCAN[3]= (const char*)GetString( MSG_SCNRMODGUI_MAX );
Cycle_OSCAN[4]= (const char*)GetString( MSG_SCNRMODGUI_VIDEO );
Cycle_OSCAN[5]=NULL;

Cycle_Colors[ 0]="   2";   // not allowed by WCS
Cycle_Colors[ 1]="   4";   // not allowed by WCS
Cycle_Colors[ 2]="   8";   // not allowed by WCS
Cycle_Colors[ 3]="  16";  // default
Cycle_Colors[ 4]="  32";
Cycle_Colors[ 5]="  64";
Cycle_Colors[ 6]=" 128";
Cycle_Colors[ 7]=" 256";
Cycle_Colors[ 8]=" 32k"; // RTG
Cycle_Colors[ 9]=" 64k"; // RTG
Cycle_Colors[10]=" 16M"; // RTG



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
//   Child, HGroup,
//    Child, Label2( "Farben: " /*GetString( MSG_SCNRMODGUI_OVERSCAN )*/ ),
//    Child, SM_COLORS = CycleObject, MUIA_Cycle_Entries, Cycle_Colors, End,
//	End,
//   	/////////////
	Child, HGroup,
    Child, Label2( GetString( MSG_SCNRMODGUI_OVERSCAN ) ),
    Child, SM_OSCAN = CycleObject, MUIA_Cycle_Entries, Cycle_OSCAN, End,
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
    Update = 1;
    if(Selected->UX != Selected->OX)
     Update = 0;
    if(Selected->UY != Selected->OY)
     Update = 0;
    
    if(CheckVal == 0)
     {
     Selected->OX = Selected->X;
     Selected->OY = Selected->Y;
     } /* if */
    else
     {
     Selected->OX = Selected->OScans[CheckVal - 1].x;
     Selected->OY = Selected->OScans[CheckVal - 1].y;
     } /* else */
    if(Update)
     {
     Selected->UX = Selected->OX;
     Selected->UY = Selected->OY;
     } /* if */

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
    
    set(SM_Width, MUIA_String_Integer, Selected->UX);
    set(SM_Height, MUIA_String_Integer, Selected->UY);
    
    break;
    } /* ID_SM_LIST */
   case ID_SM_HEIGHT:
    {
    get(SM_Height, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OY)
     {
     CheckVal = Selected->OY;
     set(SM_Height, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
     } /* if */
    else if(CheckVal > Selected->MaxY)
     {
     CheckVal = Selected->MaxY;
     set(SM_Height, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
     } /* else if */
    else
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);
    Selected->UY = CheckVal;
    break;
    } /* ID_SM_HEIGHT */
   case ID_SM_WIDTH:
    {
    get(SM_Width, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OX)
     {
     CheckVal = Selected->OX;
     set(SM_Width, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);
     } /* if */
    else if(CheckVal > Selected->MaxX)
     {
     CheckVal = Selected->MaxX;
     set(SM_Width, MUIA_String_Integer, CheckVal);
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Width);
     } /* else if */
    else
     set(ModeSelWin, MUIA_Window_ActiveObject, (IPTR)SM_Height);
    Selected->UX = CheckVal;
    break;
    } /* ID_SM_WIDTH */
   case ID_SM_SAVE:
    {
    ScrnData->ModeID = Selected->ModeID;
    ScrnData->Width = Selected->UX;
    ScrnData->Height = Selected->UY;
    /* Do something more here */
    } /* ID_SM_SAVE */
   case ID_SM_USE:
    {
    
    Finished = 1;
    
    get(SM_Height, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OY)
     {
     CheckVal = Selected->OY;
     set(SM_Height, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    if(CheckVal > Selected->MaxY)
     {
     CheckVal = Selected->MaxY;
     set(SM_Height, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    Selected->UY = CheckVal;

    get(SM_Width, MUIA_String_Integer, &CheckVal);
    if(CheckVal < Selected->OX)
     {
     CheckVal = Selected->OX;
     set(SM_Width, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    if(CheckVal > Selected->MaxX)
     {
     CheckVal = Selected->MaxX;
     set(SM_Width, MUIA_String_Integer, CheckVal);
     Finished = 0;
     } /* if */
    Selected->UX = CheckVal;
    
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

