/* WaveGUI.c
** World Construction Set GUI for Waves.
** Copyright 1995 by Gary R. Huber and Chris Eric Hanson.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Wave.h"
#include "GenericParams.h"

#define RENDER_SCREEN_DITHER_SIZE 4096

struct WaveWindow *WVWin[2] = {NULL, NULL};


STATIC_FCN void Wave_Draw(struct WaveData *WD, double Frame, short Detail); // used locally only -> static, AF 24.7.2021
STATIC_FCN short GUIWave_Add(struct WaveWindow *WV_Win, struct WaveData *WD,
        struct Wave *CurWave); // used locally only -> static, AF 24.7.2021
STATIC_FCN APTR Make_WVOption_Group(struct WaveWindow *WV_Win, short WinNum); // used locally only -> static, AF 24.7.2021
STATIC_FCN short GUIWave_Remove(struct WaveWindow *WV_Win, struct WaveData *WD,
        struct Wave *WV); // used locally only -> static, AF 24.7.2021
STATIC_FCN APTR Make_WVTL_Group(struct WaveWindow *WV_Win, short WinNum); // used locally only -> static, AF 24.7.2021
STATIC_FCN void GUIWaveKey_SetGads(struct WaveWindow *WV_Win, struct WaveData *WD,
        short frame); // used locally only -> static, AF 24.7.2021
STATIC_FCN APTR Make_WVAnim_Group(struct WaveWindow *WV_Win, short WinNum); // used locally only -> static, AF 24.7.2021




void Make_WV_Window(short WinNum, char *NameStr) // used locally only -> static, AF 24.7.2021
{
 char filename[256];
 long i, open;
 ULONG WinID;
 struct WaveWindow *WV_Win;

 if (WVWin[WinNum])
  {
  DoMethod(WVWin[WinNum]->WaveWin, MUIM_Window_ToFront);
  set(WVWin[WinNum]->WaveWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((WVWin[WinNum] = (struct WaveWindow *)
	get_Memory(sizeof (struct WaveWindow), MEMF_CLEAR)) == NULL)
   return;
 if (WinNum == 0)
  {
  if ((WVWin[WinNum]->WD = WaveData_New()) == NULL)
   {
   Close_WV_Window(&WVWin[WinNum]);
   return;
   } /* if */
  } /* if wave editor */
 else
  {
  if (CL_Win)
   {
   if (CL_Win->CD->WD)
    WVWin[WinNum]->WD = CL_Win->CD->WD;
   else
    {
    if ((WVWin[WinNum]->WD = WaveData_New()) == NULL)
     {
     Close_WV_Window(&WVWin[WinNum]);
     return;
     } /* if */
    CL_Win->CD->WD = WVWin[WinNum]->WD;
    } /* else */
   } /* if */
  } /* if */

 WV_Win = WVWin[WinNum];
 WinID = WinNum == 0 ? 'WVED': 'WVEC';

 Set_Param_Menu(10);

 WV_Win->WaveListSize = 100 * sizeof (char *);
 WV_Win->WaveList = (char **)get_Memory(WV_Win->WaveListSize, MEMF_CLEAR);
 WV_Win->WaveAddrList = (struct Wave **)
	get_Memory(WV_Win->WaveListSize, MEMF_CLEAR);
 WV_Win->WaveEntries = 0;
 WV_Win->CurrentWave = -1;
 WV_Win->CurWave = NULL;
 WV_Win->WKS.Group = 4;
 WV_Win->WKS.Item = 0;
 WV_Win->WKS.NumValues = 4;
 WV_Win->WKS.Precision = WCS_KFPRECISION_FLOAT;
 for (i=0; i<100; i++)
  {
  sprintf(WV_Win->WaveNames[i], "%ld", i);
  } /* for i=... */

    WV_Win->WaveWin = WindowObject,
      MUIA_Window_Title		, NameStr,
      MUIA_Window_ID		, WinID,
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, Make_WVOption_Group(WV_Win, WinNum),
	Child, HGroup,
	  Child, VGroup,
	    Child, Label("\33c\0334Waves"),
            Child, WV_Win->LS_WaveList = ListviewObject,
		MUIA_Listview_Input, TRUE,
               	MUIA_Listview_List, ListObject, ReadListFrame, End,
              End, /* ListviewObject */
	    End, /* VGroup */
	  Child, VGroup,
	    Child, Label("\33c\0334Active Wave"),
	    Child, ColGroup(2),
	      Child, Label2("Latitude"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->WaveStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->WaveArrow[0][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->WaveArrow[0][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Longitude"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->WaveStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->WaveArrow[1][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->WaveArrow[1][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Amplitude (m)"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->WaveStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->WaveArrow[2][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->WaveArrow[2][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Wave Length (km)"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->WaveStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->WaveArrow[3][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->WaveArrow[3][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Velocity (km/hr)"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->WaveStr[4] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->WaveArrow[4][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->WaveArrow[4][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      End, /* ColGroup */
	    Child, HGroup,
	      Child, WV_Win->BT_AddWave = KeyButtonFunc('a', "\33cAdd..."),
	      Child, WV_Win->BT_MapAddWave = KeyButtonFunc('p', "\33cMap Add..."),
	      Child, WV_Win->BT_RemoveWave = KeyButtonFunc('r', "\33cRemove"),
	      End, /* HGroup */

	    Child, Make_WVAnim_Group(WV_Win, WinNum),

	    End, /* VGroup */
	  End, /* HGroup */

	Child, Make_WVTL_Group(WV_Win, WinNum),

	Child, HGroup, MUIA_Group_SameWidth, TRUE,
	      Child, WV_Win->BT_Load = KeyButtonFunc('l', "\33cLoad"),
	      Child, WV_Win->BT_Save = KeyButtonFunc('e', "\33cSave"),
	      End, /* HGroup */

	End, /* VGroup */    
       End; /* Window object */

  if (! WV_Win->WaveWin)
   {
   Close_WV_Window(&WVWin[WinNum]);
   User_Message("Map View: Waves", "Out of memory!", "OK", "o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, WV_Win->WaveWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

  strcpy(WV_Win->WaveDir, wavepath);
  strcpy(WV_Win->WaveFile, wavefile);
  strmfp(filename, wavepath, wavefile);
  if (WinNum == 0)
   {
   if (! Wave_Load(filename, &WV_Win->WD))
    WaveData_SetDefaults(WV_Win->WD, 0, 1);
   }

/* ReturnIDs */
  DoMethod(WV_Win->WaveWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_WV_CLOSE(WinNum));  

 if (WinNum == 0)
  {
  MUI_DoNotiPresFal(app,
   WV_Win->GKS.BT_TimeLines, ID_WV_TIMELINES(WinNum),
   WV_Win->GKS.BT_DeleteKey, ID_WV_DELETEKEY(WinNum),
   WV_Win->GKS.BT_KeyScale, ID_WV_SCALEKEYS(WinNum),
   WV_Win->GKS.BT_NextKey, ID_WV_NEXTKEY(WinNum),
   WV_Win->GKS.BT_PrevKey, ID_WV_PREVKEY(WinNum),
   WV_Win->GKS.BT_UpdateKeys, ID_WV_UPDATEKEYS(WinNum),
   WV_Win->GKS.BT_MakeKey, ID_WV_MAKEKEY(WinNum),
   WV_Win->GKS.BT_DeleteAll, ID_WV_DELKEYS(WinNum),
   NULL);

  MUI_DoNotiPresFal(app,
   WV_Win->BT_Settings[0], ID_SB_SETPAGE(7),
   WV_Win->BT_Settings[1], ID_SB_SETPAGE(7),
   WV_Win->BT_Settings[2], ID_SB_SETPAGE(7),
   WV_Win->BT_Settings[3], ID_SB_SETPAGE(5),
   NULL);

/* STRING */
  for (i=0; i<4; i++)
   {
   DoMethod(WV_Win->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_WV_FLOATSTR(WinNum, i));
   } /* for i=0... */
  DoMethod(WV_Win->GKS.Str[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_WV_FRAMESTR(WinNum));

/* ARROWS */
  for (i=0; i<4; i++)
   {
   MUI_DoNotiPresFal(app, WV_Win->FloatArrow[i][0], ID_WV_FLOATARROWLEFT(WinNum, i),
   	WV_Win->FloatArrow[i][1], ID_WV_FLOATARROWRIGHT(WinNum, i), NULL);
   } /* for i=0... */
  for (i=0; i<2; i++)
   {
   MUI_DoNotiPresFal(app, WV_Win->GKS.StrArrow[i], ID_WV_STRARROW(WinNum, i), NULL);
   } /* for i=0... */

/* Set tab cycle chain */

/* return cycle */
  DoMethod(WV_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->FloatStr[1]);
  DoMethod(WV_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->FloatStr[2]);
  DoMethod(WV_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->FloatStr[3]);
  DoMethod(WV_Win->FloatStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->FloatStr[0]);

  } /* if normal (not cloud) waves */


  MUI_DoNotiPresFal(app,
   WV_Win->BT_AddWave, ID_WV_ADDWAVE(WinNum),
   WV_Win->BT_MapAddWave, ID_WV_MAPADDWAVE(WinNum),
   WV_Win->BT_RemoveWave, ID_WV_REMOVEWAVE(WinNum),
   WV_Win->BT_DrawDetail, ID_WV_DRAWDETAIL(WinNum),
   WV_Win->BT_DrawWaves, ID_WV_DRAW(WinNum),
   WV_Win->BT_Save, ID_WV_SAVE(WinNum), 
   WV_Win->BT_Load, ID_WV_LOAD(WinNum),
   NULL);

/* STRING */
  for (i=0; i<5; i++)
   {
   DoMethod(WV_Win->WaveStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_WV_WAVESTR(WinNum, i));
   } /* for i=0... */

/* ARROWS */
  for (i=0; i<5; i++)
   {
   MUI_DoNotiPresFal(app, WV_Win->WaveArrow[i][0], ID_WV_WAVEARROWLEFT(WinNum, i),
   	WV_Win->WaveArrow[i][1], ID_WV_WAVEARROWRIGHT(WinNum, i), NULL);
   } /* for i=0... */

/* link list to application */
  DoMethod(WV_Win->LS_WaveList, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_WV_WAVELIST(WinNum));

/* Set tab cycle chain */

  DoMethod(WV_Win->WaveWin, MUIM_Window_SetCycleChain,
	NULL);

/* return cycle */
  DoMethod(WV_Win->WaveStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->WaveStr[1]);
  DoMethod(WV_Win->WaveStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->WaveStr[2]);
  DoMethod(WV_Win->WaveStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->WaveStr[3]);
  DoMethod(WV_Win->WaveStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->WaveStr[4]);
  DoMethod(WV_Win->WaveStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	WV_Win->WaveWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, WV_Win->WaveStr[0]);



 BuildWaveList(WV_Win, WV_Win->WD);
 if (WinNum == 0)
  {
  UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
	WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
	WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
  GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
  GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
  }
 GUIWave_SetGads(WV_Win, WV_Win->CurWave);

 if (WinNum == 0)
  set(WV_Win->GKS.BT_KeyScale, MUIA_Disabled, TRUE);

/* Open window */

  set(WV_Win->WaveWin, MUIA_Window_Open, TRUE);
  get(WV_Win->WaveWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_WV_Window(&WVWin[WinNum]);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(WV_Win->WaveWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_WV_ACTIVATE(WinNum));

} /* Make_WV_Window() */

/*************************************************************************/

STATIC_FCN APTR Make_WVOption_Group(struct WaveWindow *WV_Win, short WinNum) // used locally only -> static, AF 24.7.2021
{
APTR Obj;

 if (WinNum == 0)
  {
  Obj = HGroup,
	  Child, Label2("Options"),
          Child, WV_Win->BT_Settings[0] = KeyButtonFunc('1', "\33cWaves"), 
          Child, WV_Win->BT_Settings[1] = KeyButtonFunc('2', "\33cTides"), 
          Child, WV_Win->BT_Settings[2] = KeyButtonFunc('3', "\33cReflect"), 
          Child, WV_Win->BT_Settings[3] = KeyButtonFunc('4', "\33cFractal"), 
	  End;
  } /* if waves */
 else
  {
  Obj = RectangleObject, End;
  } /* else cloud waves */

 return (Obj);

} /* Make_WVOption_Group() */

/*********************************************************************/

STATIC_FCN APTR Make_WVAnim_Group(struct WaveWindow *WV_Win, short WinNum) // used locally only -> static, AF 24.7.2021
{
APTR Obj;

 if (WinNum == 0)
  {
  Obj = VGroup,
	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	    Child, Label("\33c\0334Animation"),
	    Child, ColGroup(2),
	      Child, Label2("Move Latitude"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->FloatStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->FloatArrow[2][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->FloatArrow[2][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Move Longitude"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->FloatStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->FloatArrow[3][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->FloatArrow[3][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("Amplitude Fact"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->FloatArrow[0][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->FloatArrow[0][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, Label2("WhiteCap Ht (m)"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
		Child, WV_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567",
			MUIA_String_Accept, "-.0123456789", End,
		Child, WV_Win->FloatArrow[1][0] = ImageButton(MUII_ArrowLeft),
		Child, WV_Win->FloatArrow[1][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      End, /* ColGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	Child, HGroup, MUIA_Group_SameWidth, TRUE,
	      Child, WV_Win->BT_DrawWaves = KeyButtonFunc('w', "\33cDraw Waves"),
	      Child, WV_Win->BT_DrawDetail = KeyButtonFunc('i', "\33cDraw Detail"),
	      End, /* HGroup */

    End; /* VGroup */
  } /* if waves */
 else
  {
  Obj = RectangleObject, MUIA_Rectangle_HBar, TRUE, End;
  } /* else cloud waves */

 return (Obj);

} /* Make_WVAnim_Group() */

/*********************************************************************/

STATIC_FCN APTR Make_WVTL_Group(struct WaveWindow *WV_Win, short WinNum) // used locally only -> static, AF 24.7.2021
{
APTR Obj;

 if (WinNum == 0)
  {
  Obj = VGroup,
	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

/* Frame stuff */
            Child, VGroup,
	      Child, TextObject, MUIA_Text_Contents, "\33c\0334Key Frames", End,
              Child, HGroup,
                Child, WV_Win->GKS.BT_PrevKey = KeyButtonFunc('v', "\33cPrev"), 
                Child, Label2("Frame"),
                Child, HGroup, MUIA_Group_HorizSpacing, 0,
                  Child, WV_Win->GKS.Str[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                  Child, WV_Win->GKS.StrArrow[0] = ImageButton(MUII_ArrowLeft),
                  Child, WV_Win->GKS.StrArrow[1] = ImageButton(MUII_ArrowRight),
                  End, /* HGroup */
                Child, WV_Win->GKS.BT_NextKey = KeyButtonFunc('x', "\33cNext"), 
                End, /* HGroup */

	      Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
                Child, WV_Win->GKS.BT_MakeKey = KeyButtonFunc('m', "\33cMake Key"), 
                Child, WV_Win->GKS.BT_UpdateKeys = KeyButtonFunc('u', "\33cUpdate"),
		End, /* HGroup */
              Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
                Child, WV_Win->GKS.BT_DeleteKey = KeyButtonFunc(127, "\33c\33uDel\33nete"),
                Child, WV_Win->GKS.BT_DeleteAll = KeyButtonFunc('d', "\33cDelete All"), 
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
	        Child, WV_Win->GKS.FramePages = VGroup,
                  Child, WV_Win->GKS.BT_TimeLines = KeyButtonFunc('t', "\33cTime Lines "), 
	          End, /* VGroup */
                Child, WV_Win->GKS.BT_KeyScale = KeyButtonFunc('s', "\33cScale Keys "), 
		End, /* HGroup */
	      End, /* VGroup */

    End; /* VGroup */
  } /* if waves */
 else
  {
  Obj = RectangleObject, End;
  } /* else cloud waves */

 return (Obj);

} /* Make_WVTL_Group() */

/*********************************************************************/

void Close_WV_Window(struct WaveWindow **WVWinPtr)
{
struct WaveWindow *WV_Win;
short WinNum = 0;

 WV_Win = *WVWinPtr;
 if (WV_Win == WVWin[1])
  WinNum = 1;

 if (WV_Win)
  {
  if (WV_Win->TL)
   Close_TL_Window(&TLWin[WV_Win->TL->WinNum], 1);
  if (WV_Win->WaveWin)
   {
   if (WV_Win->Mod && WinNum == 0)
    {
    if (User_Message_Def("Wave Editor",
	"The current Wave Model has been modified. Do you wish to save it before closing?",
	"Yes|No", "yn", 1))
     {
     char filename[256];

     if (getfilename(1, "Wave Path/File", WV_Win->WaveDir, WV_Win->WaveFile))
      {
      strmfp(filename, WV_Win->WaveDir, WV_Win->WaveFile);
      if (Wave_Save(filename, WV_Win->WD))
       {
       if (strcmp(wavepath, WV_Win->WaveDir) || strcmp(wavefile, WV_Win->WaveFile))
        {
        if (User_Message_Def("Wave Editor",
	"Make this file the Project Wave File?", "Yes|No", "yn", 1))
         {
         strcpy(wavepath, WV_Win->WaveDir);
         strcpy(wavefile, WV_Win->WaveFile);
         Proj_Mod = 1;
	 } /* if */
	} /* if */
       } /* if */
      } /* if */
     }
    } /* if old model modified */
   set(WV_Win->WaveWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, WV_Win->WaveWin);
   MUI_DisposeObject(WV_Win->WaveWin);
   } /* if window created */
  if (WV_Win->WaveList)
   free_Memory(WV_Win->WaveList, WV_Win->WaveListSize);
  if (WV_Win->WaveAddrList)
   free_Memory(WV_Win->WaveAddrList, WV_Win->WaveListSize);

  if (WV_Win->WD && WinNum == 0)
   WaveData_Del(WV_Win->WD);
  free_Memory(WV_Win, sizeof (struct WaveWindow));
  *WVWinPtr = NULL;
  } /* if memory allocated */

} /* Close_WV_Window() */

/*********************************************************************/

void Handle_WV_Window(ULONG WCS_ID)
{
short i, WinNum;
char *FloatData;
long data;
double FloatVal;
struct WaveWindow *WV_Win;

 WinNum = (WCS_ID & 0x00ff0000) >> 16;
 WV_Win = WVWin[WinNum];


  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   if (WinNum == 0)
    Make_WV_Window(WinNum, "Wave Editor");
   else
    Make_WV_Window(WinNum, "Cloud Wave Editor");
   return;
   } /* Open Wave Editor Window */

  if (! WV_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID & 0x000000ff)
     {
     case 3:				/* ID_WV_ADDWAVE(WinNum): */
      {
      if (GUIWave_Add(WV_Win, WV_Win->WD, WV_Win->CurWave))
       {
       WV_Win->Mod = 1;
       if (WinNum == 1 && CL_Win)
        {
        CL_Win->Mod = 1;
        sprintf(str, "%d", CL_Win->CD->WD->NumWaves);
        set(CL_Win->Text, MUIA_Text_Contents, str);
	} /* if */
       } /* if something actually done */
      break;
      }
     case 15:				/* ID_WV_MAPADDWAVE(WinNum): */
      {
      short WaveNum;
      char TextStr[32];

      if ((WaveNum = Wave_Add(&WV_Win->WD->Wave)) >= 0)
       {
       WV_Win->WD->NumWaves = WaveNum;
       WV_Win->Mod = 1;
       if (WinNum == 1 && CL_Win)
        {
        sprintf(TextStr, "%d", WaveNum);
        set(CL_Win->Text, MUIA_Text_Contents, TextStr);
        CL_Win->Mod = CL_Win->ReGen = 1;
	}
       BuildWaveList(WV_Win, WV_Win->WD);
       WV_Win->WaveEntries = WV_Win->WD->NumWaves;
       WV_Win->CurrentWave = WV_Win->WaveEntries - 1;
       WV_Win->CurWave = WV_Win->WaveAddrList[WV_Win->CurrentWave];
       nnset(WV_Win->LS_WaveList, MUIA_List_Active, WV_Win->CurrentWave);
       GUIWave_SetGads(WV_Win, WV_Win->CurWave);
       } /* if something actually done */
      break;
      }
     case 4:				/* ID_WV_REMOVEWAVE(WinNum): */
      {
      GUIWave_Remove(WV_Win, WV_Win->WD, WV_Win->CurWave);
      if (WinNum == 1 && CL_Win)
       {
       CL_Win->Mod = 1;
       sprintf(str, "%d", CL_Win->CD->WD->NumWaves);
       set(CL_Win->Text, MUIA_Text_Contents, str);
       } /* if */
      break;
      }
     case 5:					/* ID_WV_DRAW(WinNum): */
      {
      Wave_Draw(WV_Win->WD, (double)WV_Win->WKS.Frame, 0);
      break;
      }
     case 10:					/* ID_WV_DRAWDETAIL(WinNum): */
      {
      Wave_Draw(WV_Win->WD, (double)WV_Win->WKS.Frame, 1);
      break;
      }
     case 6:					/* ID_WV_MAKEKEY(WinNum): */
      {
      short KeyFrame;
      char FrameStr[32];

      sprintf(FrameStr, "%d", WV_Win->WKS.Frame);
      if (! GetInputString("Enter frame to make key for.",
	 "abcdefghijklmnopqrstuvwxyz", FrameStr))
       break;

      KeyFrame = atoi(FrameStr);
      if (MakeGenericKeyFrame(&WV_Win->WD->WaveKey, &WV_Win->WD->KFSize,
	&WV_Win->WD->NumKeys, KeyFrame, WV_Win->WKS.Group, WV_Win->WKS.Item,
	WV_Win->WKS.Item, WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
	WV_Win->WKS.TCB, WV_Win->WKS.Linear, WV_Win->WKS.Precision))
       {
       UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
		WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
		WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
		WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
       GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
       GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
       TL_Recompute(WV_Win->TL);
       WV_Win->Mod = 1;
       } /* if */
      break;
      }
     case 9:					/* ID_WV_UPDATEKEYS(WinNum): */
      {
      UpdateGenericKeyFrames(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, WV_Win->WKS.Item,
	WV_Win->WKS.NumValues,
	&WV_Win->WD->Amp, NULL, NULL, WV_Win->WKS.TCB, WV_Win->WKS.Linear,
	0, 0, NULL, NULL, WV_Win->WKS.Precision);
      TL_Recompute(WV_Win->TL);
      WV_Win->Mod = 1;
      break;
      }
     case 7:					/* ID_WV_DELKEYS(WinNum): */
      {
      if (User_Message_Def("Wave Editor",
	"Delete all wave key frames?", "OK|Cancel", "oc", 1))
       {
       WV_Win->WD->NumKeys = 0;
       memset(WV_Win->WD->WaveKey, 0, WV_Win->WD->KFSize);
       UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
	WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
	WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
       GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
       GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
       TL_Recompute(WV_Win->TL);
       WV_Win->Mod = 1;
       } /* if */
      break;
      }
     case 8:					/* ID_WV_DELETEKEY(WinNum): */
      {
      DeleteGenericKeyFrame(WV_Win->WD->WaveKey, &WV_Win->WD->NumKeys,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, 0,
	NULL, NULL);
      UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
	WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
	WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
      GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
      GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
      TL_Recompute(WV_Win->TL);
      WV_Win->Mod = 1;
      break;
      }
     case 12:					/* ID_WV_NEXTKEY(WinNum): */
      {
      if (WV_Win->WKS.NextKey >= 0)
       {
       WV_Win->WKS.Frame = WV_Win->WKS.NextKey;
       set(WV_Win->GKS.Str[0], MUIA_String_Integer, WV_Win->WKS.Frame);
       } /* if */
      break;
      } /* Next Key */
     case 11:					/* ID_WV_PREVKEY(WinNum): */
      {
      if (WV_Win->WKS.PrevKey >= 0)
       {
       WV_Win->WKS.Frame = WV_Win->WKS.PrevKey;
       set(WV_Win->GKS.Str[0], MUIA_String_Integer, WV_Win->WKS.Frame);
       } /* if */
      break;
      } /* Prev Key */
     case 1:					/* ID_WV_LOAD(WinNum): */
      {
      char filename[256], *Ptrn = "#?.wve";

      if (getfilenameptrn
	(0, "Wave Path/File", WV_Win->WaveDir, WV_Win->WaveFile, Ptrn))
       {
       strmfp(filename, WV_Win->WaveDir, WV_Win->WaveFile);
       if (Wave_Load(filename, &WV_Win->WD))
        {
        BuildWaveList(WV_Win, WV_Win->WD);
        if (WinNum == 0)
         {
         UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
		WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
		WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
		WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
         GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
         GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
	 }
        GUIWave_SetGads(WV_Win, WV_Win->CurWave);
        if ((WinNum == 0 && strcmp(wavepath, WV_Win->WaveDir)) || strcmp(wavefile, WV_Win->WaveFile))
         {
         if (User_Message_Def("Wave Editor",
	"Make this file the Project Wave File?", "Yes|No", "yn", 1))
          {
          strcpy(wavepath, WV_Win->WaveDir);
          strcpy(wavefile, WV_Win->WaveFile);
          Proj_Mod = 1;
	  } /* if */
	 } /* if */
        TL_Recompute(WV_Win->TL);
        WV_Win->Mod = 0;
	} /* if */
       } /* if */
      break;
      }
     case 2:					/* ID_WV_SAVE(WinNum): */
      {
      char filename[256], *Ptrn = "#?.wve";

      if (getfilenameptrn
	(1, "Wave Path/File", WV_Win->WaveDir, WV_Win->WaveFile, Ptrn))
       {
       if (stricmp(&WV_Win->WaveFile[strlen(WV_Win->WaveFile) - 4], ".wve"))
        strcat(WV_Win->WaveFile, ".wav");
       strmfp(filename, WV_Win->WaveDir, WV_Win->WaveFile);
       if (Wave_Save(filename, WV_Win->WD))
        {
        if ((WinNum == 0 && strcmp(wavepath, WV_Win->WaveDir)) || strcmp(wavefile, WV_Win->WaveFile))
         {
         if (User_Message_Def("Wave Editor",
	"Make this file the Project Wave File?", "Yes|No", "yn", 1))
          {
          strcpy(wavepath, WV_Win->WaveDir);
          strcpy(wavefile, WV_Win->WaveFile);
          Proj_Mod = 1;
	  } /* if */
	 } /* if */
        WV_Win->Mod = 0;
	} /* if saved OK */
       } /* if file name */
      break;
      }
     case 14:					/* ID_WV_TIMELINES(WinNum): */
      {
      char *WindowTitle;
      static const char *Titles[] = {"Amplitude", "WhiteCap Height",
	"Move Latitude", "Move Longitude", NULL};

      if (WinNum == 0)
       WindowTitle = "Wave Time Lines";
      else
       WindowTitle = "Cloud Wave Time Lines";

      Make_TL_Window(WindowTitle, (char **)Titles, &WV_Win->TL,
	WV_Win->FloatStr, &WV_Win->WKS, &WV_Win->GKS, &WV_Win->WD->WaveKey,
	&WV_Win->WD->KFSize, &WV_Win->WD->NumKeys, &WV_Win->WD->Amp,
	NULL, NULL);
      break;
      }
     case 13:					/* ID_WV_SCALEKEYS(WinNum): */
      {
      break;
      }
     case 0:					/* ID_WV_CLOSE(WinNum): */
      {
      Close_WV_Window(&WVWin[WinNum]);
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_STRING1:
    {
    i = WCS_ID - ID_WV_FLOATSTR(WinNum, 0);
    get(WV_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    switch (i)
     {
     case 2:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_LATOFF, atof(FloatData));
      break;
      }
     case 3:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_LONOFF, atof(FloatData));
      break;
      }
     case 0:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_AMPLITUDE, atof(FloatData));
      break;
      }
     case 1:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_WHITECAPHT, atof(FloatData));
      break;
      }
     } /* switch */
    Update_TL_Win(WV_Win->TL, i);
    WV_Win->Mod = 1;
    break;
    } /* STRING1 */

   case GP_STRING2:
    {
    i = WCS_ID - ID_WV_WAVESTR(WinNum, 0);
    get(WV_Win->WaveStr[i], MUIA_String_Contents, &FloatData);
    switch (i)
     {
     case 0:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LAT, atof(FloatData));
      break;
      }
     case 1:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LON, atof(FloatData));
      break;
      }
     case 2:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_AMP, atof(FloatData));
      break;
      }
     case 3:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LENGTH, atof(FloatData));
      break;
      }
     case 4:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_VELOCITY, atof(FloatData));
      break;
      }
     } /* switch */
    WV_Win->Mod = 1;
    break;
    } /* STRING2 */

   case GP_STRING3:
    {
    get(WV_Win->GKS.Str[0], MUIA_String_Integer, &data);
    WV_Win->WKS.Frame = data;
    UnsetGenericKeyFrame(WV_Win->WD->WaveKey, WV_Win->WD->NumKeys, &WV_Win->WKS,
	WV_Win->WKS.Frame, WV_Win->WKS.Group, WV_Win->WKS.Item, 0, WV_Win->WKS.Item,
	WV_Win->WKS.NumValues, &WV_Win->WD->Amp, NULL, NULL,
	WV_Win->WKS.TCB, &WV_Win->WKS.Linear, WV_Win->WKS.Precision);
    GUIWaveKey_SetGads(WV_Win, WV_Win->WD, WV_Win->WKS.Frame);
    GUIDisableKeyButtons(&WV_Win->GKS, WV_Win->TL, &WV_Win->WKS);
    if (WV_Win->TL)
     {
     long data2;

     get(WV_Win->TL->Prop[2], MUIA_Prop_First, &data2);
     data = (100.0 * ((float)WV_Win->WKS.Frame / (float)WV_Win->TL->Frames));
     if (data != data2 && ! WV_Win->WKS.PropBlock)
      { 
      set(WV_Win->TL->Prop[2], MUIA_Prop_First, data);
      WV_Win->WKS.StrBlock = 1;
      } /* if */      
     WV_Win->WKS.PropBlock = 0;
     if (WV_Win->WKS.IsKey >= 0)
      {
      WV_Win->TL->ActiveKey = GetActiveGenericKey(WV_Win->TL->SKT, WV_Win->WKS.Frame);
      sprintf(str, "%d", WV_Win->WKS.Frame);
      set(WV_Win->TL->FrameTxt, MUIA_Text_Contents, str);
      TL_Redraw(WV_Win->TL);
      }/* if key frame */
     } /* if time line window open */
    break;
    } /* STRING3 */

   case GP_ARROW1:
   case GP_ARROW2:
    {
    double Sign = .10;

    if ((WCS_ID & 0x0000ff00) == GP_ARROW1)
     {
     i = WCS_ID - ID_WV_FLOATARROWLEFT(WinNum, 0);
     Sign = -.10;
     }
    else
     i = WCS_ID - ID_WV_FLOATARROWRIGHT(WinNum, 0);

    get(WV_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal += Sign;
    setfloat(WV_Win->FloatStr[i], FloatVal);
     
    switch (i)
     {
     case 2:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_LATOFF, FloatVal);
      break;
      }
     case 3:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_LONOFF, FloatVal);
      break;
      }
     case 0:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_AMPLITUDE, FloatVal);
      break;
      }
     case 1:
      {
      WaveData_SetDouble(WV_Win->WD, WCS_WAVEDATA_WHITECAPHT, FloatVal);
      break;
      }
     } /* switch */
    Update_TL_Win(WV_Win->TL, i);
    WV_Win->Mod = 1;
    break;
    } /* ARROW1 ARROW2 */

   case GP_ARROW3:
   case GP_ARROW4:
    {
    double Sign = .10;

    if ((WCS_ID & 0x0000ff00) == GP_ARROW3)
     {
     i = WCS_ID - ID_WV_WAVEARROWLEFT(WinNum, 0);
     Sign = -.10;
     }
    else
     i = WCS_ID - ID_WV_WAVEARROWRIGHT(WinNum, 0);

    if (i == 4)
     Sign *= 100.0;

    get(WV_Win->WaveStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal += Sign;
    setfloat(WV_Win->WaveStr[i], FloatVal);
     
    switch (i)
     {
     case 0:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LAT, FloatVal);
      break;
      }
     case 1:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LON, FloatVal);
      break;
      }
     case 2:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_AMP, FloatVal);
      break;
      }
     case 3:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_LENGTH, FloatVal);
      break;
      }
     case 4:
      {
      Wave_Set(WV_Win->CurWave, WCS_WAVE_VELOCITY, FloatVal);
      break;
      }
     } /* switch */
    WV_Win->Mod = 1;
    break;
    } /* ARROW3 ARROW4 */

   case GP_ARROW5:
    {
    i = WCS_ID - ID_WV_STRARROW(WinNum, 0);
    
    i = i - (1 - i);
    WV_Win->WKS.Frame += i;

    if (WV_Win->WKS.Frame < 0)
     WV_Win->WKS.Frame = 0;
    set(WV_Win->GKS.Str[0], MUIA_String_Integer, WV_Win->WKS.Frame);
    break;
    } /* frame arrows */

   case GP_LIST1:
    {
    get(WV_Win->LS_WaveList, MUIA_List_Active, &data);
    WV_Win->CurrentWave = data;
    WV_Win->CurWave = WV_Win->WaveAddrList[data];
    GUIWave_SetGads(WV_Win, WV_Win->CurWave);
    break;
    } /* LIST */

   } /* switch gadget group */

} /* Handle_WV_Window() */

/********************************************************************/

short Wave_Load(char *filename, struct WaveData **WDPtr)
{
char Title[32];
short success = 1, KFItems = 7, Version;
long i, Item;
double DoubleVal;
FILE *fWave;
struct WaveData *WD;
struct Wave *WV;
union KeyFrame *KFPtr;

 if (! filename)
  return (0);

 if ((fWave = fopen(filename, "r")))
  {
  fgets(Title, 24, fWave);
  Title[7] = '\0';
  if (! strcmp(Title, "WCSWave"))
   {
   fscanf(fWave, "%hd", &Version);
   
   if (*WDPtr)
    WaveData_Del(*WDPtr);

   *WDPtr = WaveData_New();
   WD = *WDPtr;
   if (WD)
    {
    while ((fscanf(fWave, "%ld", &Item) == 1) && success)
     {
     switch (Item)
      {
      case WAVEDATA_SHT_NUMKEYS:
       fscanf(fWave, "%hd", &WD->NumKeys);
       break;
      case WAVEDATA_SHT_NUMWAVES:
       fscanf(fWave, "%hd", &WD->NumWaves);
       break;
      case WAVEDATA_DBL_LATOFF:
       fscanf(fWave, "%le", &WD->LatOff);
       break;
      case WAVEDATA_DBL_LONOFF:
       fscanf(fWave, "%le", &WD->LonOff);
       break;
      case WAVEDATA_DBL_AMPLITUDE:
       fscanf(fWave, "%le", &WD->Amp);
       break;
      case WAVEDATA_DBL_WHITECAP:
       fscanf(fWave, "%le", &WD->WhiteCapHt);
       break;

      case WAVE_NEW:
       if (WD->Wave == NULL)
        {
        if ((WD->Wave = Wave_New()) == NULL)
         {
         success = 0;
         break;
	 }
        WV = WD->Wave;
	} /* if first wave */
       else
        {
        if ((WV->Next = Wave_New()) == NULL)
         {
         success = 0;
         break;
	 }
        WV = WV->Next;
	} /* else */
       break;
      case WAVE_SHT_NUMKEYS:
       fscanf(fWave, "%ld", &WV->NumKeys);
       break;
      case WAVE_DBL_AMP:
       fscanf(fWave, "%le", &WV->Amp);
       break;
      case WAVE_DBL_LENGTH:
       fscanf(fWave, "%le", &WV->Length);
       break;
      case WAVE_DBL_VELOCITY:
       fscanf(fWave, "%le", &WV->Velocity);
       break;
      case WAVE_DBL_LAT:
       fscanf(fWave, "%le", &WV->Pt.Lat);
       break;
      case WAVE_DBL_LON:
       fscanf(fWave, "%le", &WV->Pt.Lon);
       break;

      case WAVEDATA_SHT_KFITEMS:
       fscanf(fWave, "%hd", &KFItems);
       break;
      case WAVEDATA_SHT_KEYFRAME:
       if (! WD->WaveKey)
        {
        if ((WD->WaveKey = (union KeyFrame *)
		get_Memory((WD->NumKeys + 10) * sizeof (union KeyFrame),
		MEMF_CLEAR)) == NULL)
         {
         success = 0;
         break;
	 } /* if */
        WD->KFSize = (WD->NumKeys + 10) * sizeof (union KeyFrame);
        KFPtr = WD->WaveKey;
	} /* if */
       else
        {
        KFPtr ++;
	} /* else */
       fscanf(fWave, "%hd", &KFPtr->WvKey.KeyFrame);
       break;
      case WAVEDATA_SHT_KFGROUP:
       fscanf(fWave, "%hd", &KFPtr->WvKey.Group);
       break;
      case WAVEDATA_SHT_KFITEM:
       fscanf(fWave, "%hd", &KFPtr->WvKey.Item);
       break;
      case WAVEDATA_SHT_KFLINEAR:
       fscanf(fWave, "%hd", &KFPtr->WvKey.Linear);
       break;
      case WAVEDATA_FLT_KFTCB:
       for (i=0; i<3; i++)
        {
        fscanf(fWave, "%le", &DoubleVal);
        KFPtr->WvKey.TCB[i] = DoubleVal;
	} /* for i=0... */
       break;
      case WAVEDATA_FLT_KFVALUES:
       for (i=0; i<KFItems; i++)
        {
        fscanf(fWave, "%le", &DoubleVal);
        KFPtr->WvKey.Value[i] = DoubleVal;
	} /* for i=0... */
       break;
      } /* switch */
     } /* while */
    } /* if WV */
   else
    {
    Log(ERR_MEM_FAIL, "Wave File");
    success = 0;
    }
   } /* if correct file type */
  else
   {
   Log(ERR_WRONG_TYPE, "Wave File");
   success = 0;
   } /* else */
  fclose(fWave);
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, "Wave File");
  success = 0;
  } /* else no file */

 return (success);

} /* Wave_Load() */

/*********************************************************************/

short Wave_Save(char *filename, struct WaveData *WD)
{
short i, j, KFItems = 4, Version = WAVE_CURRENT_VERSION;
FILE *fWave;
struct Wave *WV;
union KeyFrame *KFPtr;

 if (! filename || ! WD)
  return (0);

 if ((fWave = fopen(filename, "w")))
  {
  fprintf(fWave, "WCSWave\n");

  fprintf(fWave, "%d\n", Version);

  fprintf(fWave, "%d\n", WAVEDATA_SHT_NUMKEYS);
  fprintf(fWave, "%d\n", WD->NumKeys);
  fprintf(fWave, "%d\n", WAVEDATA_SHT_NUMWAVES);
  fprintf(fWave, "%d\n", WD->NumWaves);
  fprintf(fWave, "%d\n", WAVEDATA_DBL_LATOFF);
  fprintf(fWave, "%f\n", WD->LatOff);
  fprintf(fWave, "%d\n", WAVEDATA_DBL_LONOFF);
  fprintf(fWave, "%f\n", WD->LonOff);
  fprintf(fWave, "%d\n", WAVEDATA_DBL_AMPLITUDE);
  fprintf(fWave, "%f\n", WD->Amp);
  fprintf(fWave, "%d\n", WAVEDATA_DBL_WHITECAP);
  fprintf(fWave, "%f\n", WD->WhiteCapHt);

  WV = WD->Wave;
  while (WV)
   {
   fprintf(fWave, "%d\n", WAVE_NEW);
   fprintf(fWave, "%d\n", WAVE_SHT_NUMKEYS);
   fprintf(fWave, "%d\n", WV->NumKeys);
   fprintf(fWave, "%d\n", WAVE_DBL_AMP);
   fprintf(fWave, "%f\n", WV->Amp);
   fprintf(fWave, "%d\n", WAVE_DBL_LENGTH);
   fprintf(fWave, "%f\n", WV->Length);
   fprintf(fWave, "%d\n", WAVE_DBL_VELOCITY);
   fprintf(fWave, "%f\n", WV->Velocity);
   fprintf(fWave, "%d\n", WAVE_DBL_LAT);
   fprintf(fWave, "%f\n", WV->Pt.Lat);
   fprintf(fWave, "%d\n", WAVE_DBL_LON);
   fprintf(fWave, "%f\n", WV->Pt.Lon);
   WV = WV->Next;
   } /* while */

  KFPtr = WD->WaveKey;
  if (WD->NumKeys)
   {
   fprintf(fWave, "%d\n", WAVEDATA_SHT_KFITEMS);
   fprintf(fWave, "%d\n", KFItems);
   for (i=0; i<WD->NumKeys; i++, KFPtr ++)
    {
    fprintf(fWave, "%d\n", WAVEDATA_SHT_KEYFRAME);
    fprintf(fWave, "%d\n", KFPtr->WvKey.KeyFrame);
    fprintf(fWave, "%d\n", WAVEDATA_SHT_KFGROUP);
    fprintf(fWave, "%d\n", KFPtr->WvKey.Group);
    fprintf(fWave, "%d\n", WAVEDATA_SHT_KFITEM);
    fprintf(fWave, "%d\n", KFPtr->WvKey.Item);
    fprintf(fWave, "%d\n", WAVEDATA_SHT_KFLINEAR);
    fprintf(fWave, "%d\n", KFPtr->WvKey.Linear);
    fprintf(fWave, "%d\n", WAVEDATA_FLT_KFTCB);
    fprintf(fWave, "%f ",  KFPtr->WvKey.TCB[0]);
    fprintf(fWave, "%f ",  KFPtr->WvKey.TCB[1]);
    fprintf(fWave, "%f\n", KFPtr->WvKey.TCB[2]);
    fprintf(fWave, "%d\n", WAVEDATA_FLT_KFVALUES);
    for (j=0; j<KFItems; j++)
     fprintf(fWave, "%f ", KFPtr->WvKey.Value[j]);
    fprintf(fWave, "\n");
    } /* for i=0... */
   } /* if key frames exist */  
  fclose(fWave);
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, "Wave File");
  return (0);
  } /* else no file */

 return (1);

} /* Wave_Save() */

/*********************************************************************/

STATIC_FCN short GUIWave_Add(struct WaveWindow *WV_Win, struct WaveData *WD,
	struct Wave *CurWave) // used locally only -> static, AF 24.7.2021
{
struct Wave **NewWaveAddr, *NewWave;
short success = 0;

 if (WD)
  {
  NewWaveAddr = &WD->Wave;
  NewWave = *NewWaveAddr;
  while (NewWave)
   {
   NewWaveAddr = &NewWave->Next;
   NewWave = NewWave->Next;
   } /* while */

  if ((*NewWaveAddr = Wave_New()))
   {
   NewWave = *NewWaveAddr;
   Wave_SetDefaults(NewWave, CurWave);
   BuildWaveList(WV_Win, WD);
   WV_Win->CurrentWave = WV_Win->WaveEntries - 1;
   WV_Win->CurWave = NewWave;
   WD->NumWaves ++;
   nnset(WV_Win->LS_WaveList, MUIA_List_Active, WV_Win->CurrentWave);
   GUIWave_SetGads(WV_Win, WV_Win->CurWave);
   WV_Win->Mod = 1;
   success= 1;
   } /* if new image created */
  } /* if WD */

 return (success);

} /* GUIWave_Add() */

/***********************************************************************/

short Wave_Add(struct Wave **Wave)
{
short done = 0, WaveNum = 0;
struct Box Bx;
struct Wave *WV;

 if (! MapWind0)
  {
  if (User_Message_Def("Add Wave", "Map View Module must be open in order\
 to use this funcion. Would you like to open it now?", "OK|Cancel", "oc",1))
   {
   map();

   if (! MapWind0)
    return (-1);

   } /* else */
  else
   return (-1);
  } /* if */

/* determine if start from scratch or resume */

 if (*Wave)
  {
  if (User_Message("Map View: Wave Add",
	"Remove all currently defined waves before adding new ones?", "Yes|No", "yn"))
   {
   Wave_DelAll(*Wave);
   *Wave = NULL;
   } /* if remove waves first */
  else
   {
   WV = *Wave;
   WaveNum ++;
   while (WV->Next)
    {
    WV = WV->Next;
    WaveNum ++;
    } /* while */
   } /* else don't remove waves */
  } /* if at least one wave already defined */

/* Set Cloud wave sources */

 SetAPen(MapWind0->RPort, 7);
 while (! done)
  {
  sprintf(str, "\0338Set Cloud Wave Source Point %d", WaveNum);
  MapGUI_Message(0, str);
  sprintf(str, "Set Cloud Wave Source Point %d", WaveNum);
  SetWindowTitles(MapWind0, str, (UBYTE *)-1);

  if (! MousePtSet(&Bx.Low, NULL, 0))
   {
   done = 1;
   } /* if aborted */

  MapGUI_Message(0, " ");
  MapIDCMP_Restore(MapWind0);
  SetWindowTitles(MapWind0, "Map View", (UBYTE *)-1);
  if (done)
   break;

  WritePixel(MapWind0->RPort, Bx.Low.X, Bx.Low.Y);

  if (*Wave == NULL)
   {
   *Wave = Wave_New();
   WV = *Wave;
   WaveNum ++;
   } /* if first wave */
  else
   {
   WV->Next = Wave_New();
   WV = WV->Next;
   WaveNum ++;
   } /* else add another wave */

  Wave_Set(WV, WCS_WAVE_LON, X_Lon_Convert((long)Bx.Low.X));
  Wave_Set(WV, WCS_WAVE_LAT, Y_Lat_Convert((long)Bx.Low.Y));

  strcpy(str, "");
  if (! GetInputString("Enter Wave Amplitude.", 
	 ",abcdefghijklmnopqrstuvwxyz", str))
   {
   break;
   } /* if cancel */
  Wave_Set(WV, WCS_WAVE_AMP, atof(str));

  strcpy(str, "");
  GetInputString("Enter Wave Length (km).", 
	 ",abcdefghijklmnopqrstuvwxyz", str);
  if (atof(str) > 0.0)
   Wave_Set(WV, WCS_WAVE_LENGTH, atof(str));
  else
   Wave_Set(WV, WCS_WAVE_LENGTH, 1.0);

  strcpy(str, "");
  GetInputString("Enter Wave Velocity (km/hr).", 
	 ",abcdefghijklmnopqrstuvwxyz", str);
  Wave_Set(WV, WCS_WAVE_VELOCITY, atof(str));

  done = 1;	/* I just decided that one wave per button push is enough! */
  } /* while */

 return (WaveNum);

} /* Wave_Add() */

/**********************************************************************/

STATIC_FCN short GUIWave_Remove(struct WaveWindow *WV_Win, struct WaveData *WD,
	struct Wave *WV) // used locally only -> static, AF 24.7.2021
{
struct Wave *ThisOne, *WVPrev = NULL;

 ThisOne = WD->Wave;

 if (ThisOne && WV)
  {
  while (ThisOne && ThisOne != WV)
   {
   WVPrev = ThisOne;
   ThisOne = ThisOne->Next;
   } /* while */

  if (WVPrev && ThisOne)
   WVPrev->Next = ThisOne->Next;
  else if (ThisOne)
   WD->Wave = ThisOne->Next;
  if (ThisOne)
   Wave_Del(ThisOne);

  WD->NumWaves --;
  BuildWaveList(WV_Win, WD);

  WV_Win->Mod = 1;
  return (1);
  } /* if */

 return (0);

} /* GUIWave_Remove() */

/***********************************************************************/

void BuildWaveList(struct WaveWindow *WV_Win, struct WaveData *WD)
{
struct Wave *CurWave, *FirstWave;
char **NameAddr;

 GUIList_Clear(WV_Win->WaveList, WV_Win->WaveListSize, WV_Win->LS_WaveList);

 WV_Win->WaveEntries = 0;
 WV_Win->CurWave = NULL;
 WV_Win->CurrentWave = -1;

 if (WD)
  {
  CurWave = WD->Wave;
  if (CurWave)
   {
   FirstWave = CurWave;
   while (CurWave)
    {
    WV_Win->WaveAddrList[WV_Win->WaveEntries] = CurWave;
    WV_Win->WaveList[WV_Win->WaveEntries] =
	WV_Win->WaveNames[WV_Win->WaveEntries];
    NameAddr = &WV_Win->WaveList[WV_Win->WaveEntries];
    DoMethod(WV_Win->LS_WaveList, MUIM_List_Insert, NameAddr, 1, MUIV_List_Insert_Bottom);
    WV_Win->WaveEntries ++;
    CurWave = CurWave->Next;
    } /* while */
   WV_Win->CurrentWave = 0;
   WV_Win->CurWave = FirstWave;
   nnset(WV_Win->LS_WaveList, MUIA_List_Active, 0);
   GUIWave_SetGads(WV_Win, FirstWave);
   } /*if at least one wave already */
  } /* if */

} /* BuildWaveList() */

/***********************************************************************/

void GUIWave_SetGads(struct WaveWindow *WV_Win, struct Wave *WV)
{

 if (WV)
  {
  nnsetfloat(WV_Win->WaveStr[0], Wave_Get(WV, WCS_WAVE_LAT));
  nnsetfloat(WV_Win->WaveStr[1], Wave_Get(WV, WCS_WAVE_LON));
  nnsetfloat(WV_Win->WaveStr[2], Wave_Get(WV, WCS_WAVE_AMP));
  nnsetfloat(WV_Win->WaveStr[3], Wave_Get(WV, WCS_WAVE_LENGTH));
  nnsetfloat(WV_Win->WaveStr[4], Wave_Get(WV, WCS_WAVE_VELOCITY));
  } /* if wave data */
 
} /* GUIWave_SetGads() */

/***********************************************************************/

STATIC_FCN void GUIWaveKey_SetGads(struct WaveWindow *WV_Win, struct WaveData *WD,
	short frame) // used locally only -> static, AF 24.7.2021
{
struct WaveData2 *WD2;

 WD2 = (struct WaveData2 *)WD;

 if (WD2 && WV_Win)
  {
  GetGenericKeyTableValues(&WD2->KT, WD2->WaveKey, WD2->NumKeys,
	&WD2->KT_MaxFrames, WV_Win->WKS.NumValues, WV_Win->WKS.Group,
	WV_Win->WKS.Item, frame, WD2->Value, NULL, NULL,
	WV_Win->WKS.Precision);
  nnsetfloat(WV_Win->FloatStr[2], WaveData_Get(WD, WCS_WAVEDATA_LATOFF));
  nnsetfloat(WV_Win->FloatStr[3], WaveData_Get(WD, WCS_WAVEDATA_LONOFF));
  nnsetfloat(WV_Win->FloatStr[0], WaveData_Get(WD, WCS_WAVEDATA_AMPLITUDE));
  nnsetfloat(WV_Win->FloatStr[1], WaveData_Get(WD, WCS_WAVEDATA_WHITECAPHT));
  } /* if wave data */

} /* GUIWaveKey_SetGads() */

/**********************************************************************/

STATIC_FCN void Wave_Draw(struct WaveData *WD, double Frame, short Detail) // used locally only -> static, AF 24.7.2021
{
long Col, x, y, row, Low_X, Low_Y, High_X, High_Y;
double ptlat, ptlon, avglat, d1, d2, dist, lonscale, waveamp,
	LatStep, LonStep, LowLat, HighLat,
	LowLon, HighLon, MaxAmp, FloatCol;
struct BusyWindow *BWMD;
struct Wave *WV;
struct clipbounds cb;

/* determine area to draw */

 if (! MapWind0)
  return;

 setclipbounds(MapWind0, &cb);

 if (Detail)
  {
  Low_X = Lon_X_Convert(PAR_FIRST_MOTION(2)) - 10;
  Low_Y = Lat_Y_Convert(PAR_FIRST_MOTION(1)) - 10;
  High_X = Low_X + 20;
  High_Y = Low_Y + 20;

  LowLon = X_Lon_Convert(High_X);
  HighLon = X_Lon_Convert(Low_X);
  LowLat = Y_Lat_Convert(High_Y);
  HighLat = Y_Lat_Convert(Low_Y);

  Low_X = Lon_X_Convert(PAR_FIRST_MOTION(2)) - 100;
  Low_Y = Lat_Y_Convert(PAR_FIRST_MOTION(1)) - 100;
  High_X = Low_X + 200;
  High_Y = Low_Y + 200;
  LatStep = (LowLat - HighLat) / ((double)(High_Y - Low_Y));
  LonStep = (LowLon - HighLon) / ((double)(High_X - Low_X));
  }
 else
  {
  Low_X = Lon_X_Convert(PAR_FIRST_MOTION(2)) - 100;
  Low_Y = Lat_Y_Convert(PAR_FIRST_MOTION(1)) - 100;
  High_X = Low_X + 200;
  High_Y = Low_Y + 200;

  Low_X = max(Low_X, cb.lowx);
  Low_Y = max(Low_Y, cb.lowy);
  High_X = min(High_X, cb.highx);
  High_Y = min(High_Y, cb.highy);

  LowLon = X_Lon_Convert(High_X);
  HighLon = X_Lon_Convert(Low_X);
  LowLat = Y_Lat_Convert(High_Y);
  HighLat = Y_Lat_Convert(Low_Y);
  LatStep = (LowLat - HighLat) / ((double)(High_Y - Low_Y));
  LonStep = (LowLon - HighLon) / ((double)(High_X - Low_X));
  } /* else */

/* find maximum wave amplitude possible */

 MaxAmp = 0.0;
 WV = WD->Wave;
 while (WV)
  {
  MaxAmp += (WD->Amp * WV->Amp);
  WV = WV->Next;
  } /* while */
 if (MaxAmp == 0.0)
  MaxAmp = 1.0;

 BWMD = BusyWin_New("Drawing...", High_Y - Low_Y + 1, 0, 'BWMD');

 for (row=0, y=Low_Y, ptlat=HighLat; y<=High_Y; y++, row++, ptlat+=LatStep)
  {
  if (y < cb.lowy)
   continue;
  if (y > cb.highy)
   break;
  for (x=Low_X, ptlon=HighLon; x<=High_X; x++, ptlon+=LonStep)
   {
   if (x < cb.lowx)
    continue;
   if (x > cb.highx)
    break;
   waveamp = 0.0;
   WV = WD->Wave;
   while (WV)
    { /* compute wave height in meters */
    avglat = (WV->Pt.Lat + WD->LatOff + ptlat) / 2.0;
    lonscale = LATSCALE * cos(avglat * PiOver180);
    d1 = (WV->Pt.Lat + WD->LatOff - ptlat) * LATSCALE;
    d2 = (WV->Pt.Lon + WD->LonOff - ptlon) * lonscale;
    dist = sqrt(d1 * d1 + d2 * d2);
    waveamp += (WV->Amp * WD->Amp * sin(TwoPi * ((dist / WV->Length)
	- (Frame * WV->Velocity) / (WV->Length * FRAMES_PER_HOUR))));
    WV = WV->Next;
    } /* while */

/* draw pixel = waveamp */
   FloatCol = 11.99 - (waveamp / MaxAmp) * 3.99;
   Col = FloatCol;
   if (DTable)
    {
    if ((FloatCol - (double)Col) > DTable[(DMod++ % RENDER_SCREEN_DITHER_SIZE)])
     {
     if (Col < 15)
      Col ++;
     } /* if */
    } /* if */
   SetAPen(MapWind0->RPort, Col);
   WritePixel(MapWind0->RPort, x, y);
   } /* for x=... */

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   break;
   } /* if user abort */
  BusyWin_Update(BWMD, row + 1);
  } /* for y=... */

 BusyWin_Del(BWMD);

} /* Wave_Draw() */

/**********************************************************************/

void GUIDisableKeyButtons(struct GUIKeyStuff *GKS, struct TimeLineWindow *TL,
	struct WindowKeyStuff *WKS)
{
 short DisableAll;
 long item;

 if (WKS && GKS)
  {
  if (WKS->IsKey >= 0)
   {
   set(GKS->BT_DeleteKey, MUIA_Disabled, FALSE);
   } /* if iskey */
  else
   {
   set(GKS->BT_DeleteKey, MUIA_Disabled, TRUE);
   } /* else */
  if (WKS->PrevKey >= 0)
   {
   set(GKS->BT_PrevKey, MUIA_Disabled, FALSE);
   sprintf(str, "PK %d", WKS->PrevKey);
   set(GKS->BT_PrevKey, MUIA_Text_Contents, str);
   if (TL)
    {
    set(TL->BT_PrevKey, MUIA_Disabled, FALSE);
    set(TL->BT_PrevKey, MUIA_Text_Contents, str);
    } /* if motion time line window open */
   } /* if prev key */
  else
   {
   set(GKS->BT_PrevKey, MUIA_Disabled, TRUE);
   set(GKS->BT_PrevKey, MUIA_Text_Contents, "\33cPrev");
   if (TL)
    {
    set(TL->BT_PrevKey, MUIA_Disabled, TRUE);
    set(TL->BT_PrevKey, MUIA_Text_Contents, "\33cPrev");
    } /* if motion time line window open */
   } /* else */
  if (WKS->NextKey >= 0)
   {
   set(GKS->BT_NextKey, MUIA_Disabled, FALSE);
   sprintf(str, "NK %d", WKS->NextKey);
   set(GKS->BT_NextKey, MUIA_Text_Contents, str);
   if (TL)
    {
    set(TL->BT_NextKey, MUIA_Disabled, FALSE);
    set(TL->BT_NextKey, MUIA_Text_Contents, str);
    set(TL->TxtArrow[0], MUIA_Disabled, FALSE);
    set(TL->TxtArrow[1], MUIA_Disabled, FALSE);
    set(TL->TxtArrowLg[0], MUIA_Disabled, FALSE);
    set(TL->TxtArrowLg[1], MUIA_Disabled, FALSE);
    } /* if motion time line window open */
   } /* if next key */
  else
   {
   set(GKS->BT_NextKey, MUIA_Disabled, TRUE);
   set(GKS->BT_NextKey, MUIA_Text_Contents, "\33cNext");
   if (TL)
    {
    set(TL->BT_NextKey, MUIA_Disabled, TRUE);
    set(TL->BT_NextKey, MUIA_Text_Contents, "\33cNext");
    set(TL->TxtArrow[0], MUIA_Disabled, TRUE);
    set(TL->TxtArrow[1], MUIA_Disabled, TRUE);
    set(TL->TxtArrowLg[0], MUIA_Disabled, TRUE);
    set(TL->TxtArrowLg[1], MUIA_Disabled, TRUE);
    } /* if motion time line window open */
   } /* else */
  if (WKS->KeysExist)
   {
   set(GKS->BT_UpdateKeys, MUIA_Disabled, FALSE);
   if (GKS->BT_AllKeys)
    {
    sprintf(str, "All (%d)", WKS->KeysExist);
    set(GKS->BT_AllKeys, MUIA_Text_Contents, str);
    } /* if */
   if (TL)
    {
    sprintf(str, "Keys Exist (%d)", WKS->KeysExist);
    set(TL->KeysExistTxt, MUIA_Text_Contents, str);
    } /* if motion time line window open */
   } /* if keys exist */
  else
   {
   set(GKS->BT_UpdateKeys, MUIA_Disabled, TRUE);
   if (GKS->BT_AllKeys)
    set(GKS->BT_AllKeys, MUIA_Text_Contents, "\33cAll (0)");
   if (TL)
    {
    set(TL->KeysExistTxt, MUIA_Text_Contents, "No Other Keys");
    } /* if motion time line window open */
   } /* else */
  if (WKS->ItemKeys > 1)
   set(GKS->FramePages, MUIA_Disabled, FALSE);
  else
   set(GKS->FramePages, MUIA_Disabled, TRUE);
  if (WKS->ItemKeys > 0)
   set(GKS->BT_DeleteAll, MUIA_Disabled, FALSE);
  else
   set(GKS->BT_DeleteAll, MUIA_Disabled, TRUE);
  if (TL)
   {
   set(TL->BT_Linear, MUIA_Selected, WKS->Linear);
   get(TL->TCB_Cycle, MUIA_Cycle_Active, &item);
   sprintf(str, "%3.2f", WKS->TCB[item]);
   set(TL->CycleStr, MUIA_String_Contents, str);
   DisableAll = ((WKS->Item != TL->KeyItem)
	 || WKS->ItemKeys < 2);
   set(TL->BT_AddKey, MUIA_Disabled, DisableAll);
   set(TL->ValStr[0], MUIA_Disabled, DisableAll);
   set(TL->TimeLineObj[0], MUIA_Disabled, DisableAll);
   set(TL->CycleStr, MUIA_Disabled, DisableAll);
   set(TL->StrArrow[0], MUIA_Disabled, DisableAll);
   set(TL->StrArrow[1], MUIA_Disabled, DisableAll);
   if (WKS->ItemKeys > 2)
    set(TL->BT_DelKey, MUIA_Disabled, FALSE);
   else
    {
    set(GKS->BT_DeleteKey, MUIA_Disabled, TRUE);
    set(TL->BT_DelKey, MUIA_Disabled, TRUE);
    } /* else */
   } /* if TL */
  } /* if */

} /* GUIDisableKeyButtons() */
