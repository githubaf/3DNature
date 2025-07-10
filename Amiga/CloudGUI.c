/* CloudGUI.c
** World Construction Set GUI for Clouds and Waves.
** Copyright 1995 by Gary R. Huber and Chris Eric Hanson.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"


#include "GUIDefines.h"
#include "WCS.h"
#include "Wave.h"
#include "GenericParams.h"
#include "GUIExtras.h"
#include "Cloud.h"

STATIC_FCN void GUICloudKey_SetGads(struct CloudWindow *CL_Win,
        struct CloudData *CD, short Frame); // used locally only -> static, AF 26.7.2021
STATIC_FCN void GUICloud_SetGads(struct CloudWindow *CL_Win,
        struct CloudData *CD); // used locally only -> static, AF 26.7.2021

/*STATIC_FCN*/ void Make_CL_Window(void) // used locally only -> static, AF 26.7.2021 ( but now also in test)
{
 char filename[256];
 long i, open;
 static const char *CL_CloudTypes[5];
 static int Init=TRUE;  // necessary only once
 if(Init)
 {
	 Init=FALSE;
	 CL_CloudTypes[0] = (const char*)GetString( MSG_CLOUDGUI_CIRRUS );   // "Cirrus"
	 CL_CloudTypes[1] = (const char*)GetString( MSG_CLOUDGUI_STRATUS );  // "Stratus"
	 CL_CloudTypes[2] = (const char*)GetString( MSG_CLOUDGUI_NIMBUS );   // "Nimbus"
	 CL_CloudTypes[3] = (const char*)GetString( MSG_CLOUDGUI_CUMULUS );  // "Cumulus"
	 CL_CloudTypes[4] = NULL;
 }

 if (CL_Win)
  {
  DoMethod(CL_Win->CloudWin, MUIM_Window_ToFront);
  set(CL_Win->CloudWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((CL_Win = (struct CloudWindow *)
	get_Memory(sizeof (struct CloudWindow), MEMF_CLEAR)) == NULL)
   return;
 if ((CL_Win->CD = (struct CloudData *)
	get_Memory(sizeof (struct CloudData), MEMF_CLEAR)) == NULL)
  {
  Close_CL_Window();
  return;
  } /* if */

  Set_Param_Menu(10);

 CL_Win->WKS.Group = 3;
 CL_Win->WKS.Item = 0;
 CL_Win->WKS.NumValues = 7;
 CL_Win->WKS.Precision = WCS_KFPRECISION_FLOAT;

     CL_Win->CloudWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_CLOUDGUI_CLOUDEDITOR ),  // "Cloud Editor"
      MUIA_Window_ID		, MakeID('C','L','O','D'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, Label2(GetString( MSG_CLOUDGUI_OPTIONS )),                                                      // "Options"
          Child, CL_Win->BT_Settings[0] = KeyButtonFunc('1', (char*)GetString( MSG_CLOUDGUI_CLOUDS )),       // "\33cClouds"
          Child, CL_Win->BT_Settings[1] = KeyButtonFunc('2', (char*)GetString( MSG_CLOUDGUI_CLOUDSHADOWS )), // "\33cCloud Shadows"
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label2(GetString( MSG_CLOUDGUI_CLOUDTYPE )),  // "Cloud Type"
	  Child, CL_Win->Cycle = CycleObject,
		MUIA_Cycle_Entries, CL_CloudTypes,
		MUIA_Cycle_Active, 0, End,
	  Child, Label2( GetString( MSG_CLOUDGUI_SEED )) , // "Seed"
	  Child, CL_Win->IntStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234",
		MUIA_String_Accept, "0123456789", End,
	  End, /* HGroup */
	Child, Label(GetString( MSG_CLOUDGUI_CLOUDWAVES )),  // "\33c\0334Cloud Waves"
	Child, HGroup,
	  Child, Label2(GetString( MSG_CLOUDGUI_WAVES )),  // "Waves"
	  Child, CL_Win->Text = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123", End,
	  Child, CL_Win->BT_AddWave = KeyButtonFunc('a', (char*)GetString( MSG_CLOUDGUI_MAPADD )),  // "\33cMap Add..."
	  Child, CL_Win->BT_EditWave = KeyButtonFunc('e', (char*)GetString( MSG_CLOUDGUI_EDIT )),   // "\33cEdit..."
	  Child, Label2(GetString( MSG_CLOUDGUI_ANIMATE )),                                         // "Animate")
	  Child, CL_Win->Check = CheckMark(0),
	  End, /* HGroup */
	Child, Label(GetString( MSG_CLOUDGUI_LOUDMAPSIZERANGE )),  // "\33c\0334Cloud Map Size & Range"
	Child, ColGroup(4),
	  Child, Label2(GetString( MSG_CLOUDGUI_ROWS )),  // "Rows"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "0123456789", End,
	    Child, CL_Win->CloudArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_COLS ) ),  // "Cols"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "0123456789", End,
	    Child, CL_Win->CloudArrow[1][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[1][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_LATMAX ) ),  // "Lat Max"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->CloudArrow[2][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[2][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_MIN ) ),  // "Min"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[3] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->CloudArrow[3][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[3][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_LONMAX ) ),  // "Lon Max"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[4] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->CloudArrow[4][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[4][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_MIN ) ),  // "Min"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->CloudStr[5] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->CloudArrow[5][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->CloudArrow[5][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  End, /* ColGroup */
	Child, Label(GetString( MSG_CLOUDGUI_ANIMATION ) ),  // "\33c\0334Animation"
	Child, ColGroup(4),
	  Child, Label2(GetString( MSG_CLOUDGUI_COVERAGE ) ),  // "Coverage"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, ".0123456789", End,
	    Child, CL_Win->FloatArrow[0][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[0][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_DENSITY ) ),  // "Density"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, ".0123456789", End,
	    Child, CL_Win->FloatArrow[1][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[1][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_ROUGHNESS ) ),  // "Roughness"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, ".0123456789", End,
	    Child, CL_Win->FloatArrow[2][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[2][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_FRACTDIM ) ),  // "Fract Dim"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[3] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, ".0123456789", End,
	    Child, CL_Win->FloatArrow[3][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[3][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_MOVELAT ) ),  // "Move Lat"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[5] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->FloatArrow[5][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[5][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  Child, Label2(GetString( MSG_CLOUDGUI_LON ) ),  // "Lon"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[6] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->FloatArrow[6][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[6][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  End, /* ColGroup */
	Child, HGroup,
	  Child, Label2(GetString( MSG_CLOUDGUI_ALTITUDE ) ),  // "Altitude"
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->FloatStr[4] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "-.0123456789", End,
	    Child, CL_Win->FloatArrow[4][0] = ImageButtonWCS(MUII_ArrowLeft),
	    Child, CL_Win->FloatArrow[4][1] = ImageButtonWCS(MUII_ArrowRight),
	    End, /* HGroup */
	  End, /* HGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

/* Frame stuff */
        Child, VGroup,
	  Child, TextObject, MUIA_Text_Contents, GetString( MSG_CLOUDGUI_EYFRAMES ), End,  // "\33c\0334Key Frames"
          Child, HGroup,
            Child, CL_Win->GKS.BT_PrevKey = KeyButtonFunc('v', (char*)GetString( MSG_CLOUDGUI_PREV ) ),  // "\33cPrev"
            Child, Label2(GetString( MSG_CLOUDGUI_FRAME ) ),  // "Frame"
            Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, CL_Win->GKS.Str[0] = StringObject, StringFrame,
			MUIA_String_Integer, 0,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
              Child, CL_Win->GKS.StrArrow[0] = ImageButtonWCS(MUII_ArrowLeft),
              Child, CL_Win->GKS.StrArrow[1] = ImageButtonWCS(MUII_ArrowRight),
              End, /* HGroup */
            Child, CL_Win->GKS.BT_NextKey = KeyButtonFunc('x', (char*)GetString( MSG_CLOUDGUI_NEXT ) ),  // "\33cNext"
            End, /* HGroup */

	  Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
            Child, CL_Win->GKS.BT_MakeKey = KeyButtonFunc('m', (char*)GetString( MSG_CLOUDGUI_MAKEKEY ) ),    // "\33cMake Key"
            Child, CL_Win->GKS.BT_UpdateKeys = KeyButtonFunc('u', (char*)GetString( MSG_CLOUDGUI_UPDATE ) ),  // "\33cUpdate"
	    End, /* HGroup */
          Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
            Child, CL_Win->GKS.BT_DeleteKey = KeyButtonFunc(127, (char*)GetString( MSG_CLOUDGUI_DELETE ) ),     // "\33c\33uDel\33nete"
            Child, CL_Win->GKS.BT_DeleteAll = KeyButtonFunc('d', (char*)GetString( MSG_CLOUDGUI_DELETEALL ) ),  // "\33cDelete All"
	    End, /* HGroup */

	  Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_Group_HorizSpacing, 0,
	    Child, CL_Win->GKS.FramePages = VGroup,
              Child, CL_Win->GKS.BT_TimeLines = KeyButtonFunc('t', (char*)GetString( MSG_CLOUDGUI_TIMELINES ) ),  // "\33cTime Lines "
	      End, /* VGroup */
            Child, CL_Win->GKS.BT_KeyScale = KeyButtonFunc('s', (char*)GetString( MSG_CLOUDGUI_SCALEKEYS ) ),     // "\33cScale Keys "
	    End, /* HGroup */
	  End, /* VGroup */

	Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	Child, HGroup,
	    Child, CL_Win->BT_DrawCloud = KeyButtonFunc('r', (char*)GetString( MSG_CLOUDGUI_DRAWCLOUD ) ),  // "\33cDraw Cloud"
	    Child, CL_Win->BT_SetBounds = KeyButtonFunc('b', (char*)GetString( MSG_CLOUDGUI_SETBOUNDS ) ),  // "\33cSet Bounds"
	    End, /* HGroup */
	Child, HGroup,
	    Child, CL_Win->BT_Save = KeyButtonFunc('s', (char*)GetString( MSG_CLOUDGUI_SAVE ) ),  // "\33cSave"
	    Child, CL_Win->BT_Load = KeyButtonFunc('l', (char*)GetString( MSG_CLOUDGUI_LOAD ) ),  // "\33cLoad"
	    End, /* HGroup */

	End, /* VGroup */
      End; /* Window object */

  if (! CL_Win->CloudWin)
   {
   Close_CL_Window();
   User_Message((CONST_STRPTR) GetString( MSG_CLOUDGUI_MAPVIEWCLOUDS ) ,  // "Map View: Clouds"
                (CONST_STRPTR) GetString( MSG_GLOBAL_OUTOFMEMORY ) ,    // "Out of memory!" 
                (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,             // "OK"
                (CONST_STRPTR)"o");  
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, CL_Win->CloudWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

  strcpy(CL_Win->CloudDir, cloudpath);
  strcpy(CL_Win->CloudFile, cloudfile);
  strmfp(filename, cloudpath, cloudfile);
  if (! Cloud_Load(filename, &CL_Win->CD))
   Cloud_SetDefaults(CL_Win->CD, 0, 1);

/* ReturnIDs */
  DoMethod(CL_Win->CloudWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_CL_CLOSE);  

  MUI_DoNotiPresFal(app,
   CL_Win->BT_AddWave, ID_CL_WAVEADD,
   CL_Win->BT_EditWave, ID_CL_WAVEEDIT,
   CL_Win->BT_DrawCloud, ID_CL_DRAW,
   CL_Win->BT_SetBounds, ID_CL_BOUNDS, 
   CL_Win->BT_Save, ID_CL_SAVE,
   CL_Win->BT_Load, ID_CL_LOAD,
   CL_Win->BT_Settings[0], ID_SB_SETPAGE(7),
   CL_Win->BT_Settings[1], ID_SB_SETPAGE(7),
   NULL);

  MUI_DoNotiPresFal(app,
   CL_Win->GKS.BT_TimeLines, ID_CL_TIMELINES,
   CL_Win->GKS.BT_DeleteKey, ID_CL_DELETEKEY,
   CL_Win->GKS.BT_KeyScale, ID_CL_SCALEKEYS,
   CL_Win->GKS.BT_NextKey, ID_CL_NEXTKEY,
   CL_Win->GKS.BT_PrevKey, ID_CL_PREVKEY,
   CL_Win->GKS.BT_UpdateKeys, ID_CL_UPDATEKEYS,
   CL_Win->GKS.BT_MakeKey, ID_CL_MAKEKEY,
   CL_Win->GKS.BT_DeleteAll, ID_CL_DELKEYS,
   NULL);

/* CYCLE1 */
   DoMethod(CL_Win->Cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_CL_CYCLE);

/* STRING1 */
  for (i=0; i<7; i++)
   {
   DoMethod(CL_Win->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_CL_FLOATSTR(i));
   } /* for i=0... */
  for (i=0; i<6; i++)
   {
   DoMethod(CL_Win->CloudStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_CL_CLOUDSTR(i));
   } /* for i=0... */
  DoMethod(CL_Win->IntStr[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_CL_INTSTR(0));
  DoMethod(CL_Win->GKS.Str[0], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_CL_FRAMESTR);

/* ARROWS */
  for (i=0; i<7; i++)
   {
   MUI_DoNotiPresFal(app, CL_Win->FloatArrow[i][0], ID_CL_FLOATARROWLEFT(i),
   	CL_Win->FloatArrow[i][1], ID_CL_FLOATARROWRIGHT(i), NULL);
   } /* for i=0... */
  for (i=0; i<6; i++)
   {
   MUI_DoNotiPresFal(app, CL_Win->CloudArrow[i][0], ID_CL_CLOUDARROWLEFT(i),
   	CL_Win->CloudArrow[i][1], ID_CL_CLOUDARROWRIGHT(i), NULL);
   } /* for i=0... */
  for (i=0; i<2; i++)
   {
   MUI_DoNotiPresFal(app, CL_Win->GKS.StrArrow[i], ID_CL_STRARROW(i), NULL);
   } /* for i=0... */

/* Checkmark */
  DoMethod(CL_Win->Check, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_CL_CHECK);

/* Set tab cycle chain */

  DoMethod(CL_Win->CloudWin, MUIM_Window_SetCycleChain,
	NULL);

/* return cycle */
  DoMethod(CL_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[0]);
  DoMethod(CL_Win->CloudStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[1]);
  DoMethod(CL_Win->CloudStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[2]);
  DoMethod(CL_Win->CloudStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[3]);
  DoMethod(CL_Win->CloudStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[4]);
  DoMethod(CL_Win->CloudStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->CloudStr[5]);
  DoMethod(CL_Win->CloudStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[0]);
  DoMethod(CL_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[1]);
  DoMethod(CL_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[2]);
  DoMethod(CL_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[3]);
  DoMethod(CL_Win->FloatStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[4]);
  DoMethod(CL_Win->FloatStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[5]);
  DoMethod(CL_Win->FloatStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->FloatStr[6]);
  DoMethod(CL_Win->FloatStr[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	CL_Win->CloudWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, CL_Win->IntStr[0]);

  UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
	CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
	CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
  GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
  GUICloud_SetGads(CL_Win, CL_Win->CD);
  GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);

  set(CL_Win->GKS.BT_KeyScale, MUIA_Disabled, TRUE);

/* Open window */
  CL_Win->ReGen = 1;

  set(CL_Win->CloudWin, MUIA_Window_Open, TRUE);
  get(CL_Win->CloudWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_CL_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(CL_Win->CloudWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_CL_ACTIVATE);

} /* Make_CL_Window() */

/*********************************************************************/

void Close_CL_Window(void)
{

 if (CL_Win)
  {
  if (CL_Win->TL)
   Close_TL_Window(&TLWin[CL_Win->TL->WinNum], 1);
  if (WVWin[1])
   Close_WV_Window(&WVWin[1]);
  if (CL_Win->CloudWin)
   {
   if (CL_Win->Mod)
    {
    if (User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ) ,  // "Parameters Module: Model"
    		         GetString( MSG_CLOUDGUI_THECURRENTCLOUDMODELHASBEENMODIFIEDDOYOUWISHTOSAVE ) ,  // "The current Cloud Model has been modified. Do you wish to save it before closing?"
		         GetString( MSG_GLOBAL_YESNO ) ,  // "Yes|No"
                         (CONST_STRPTR)"yn", 1))
     {
     char filename[256];

     if (getfilename(1, (char*)GetString( MSG_CLOUDGUI_CLOUDPATHFILE ), CL_Win->CloudDir, CL_Win->CloudFile))  // "Cloud Path/File"
      {
      strmfp(filename, CL_Win->CloudDir, CL_Win->CloudFile);
      if (Cloud_Save(filename, CL_Win->CD))
       {
       if (strcmp(cloudpath, CL_Win->CloudDir) || strcmp(cloudfile, CL_Win->CloudFile))
        {
        if (User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,               // "Cloud Editor"
        		GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ), // "Make this file the Project Cloud File?" 
                        GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                        (CONST_STRPTR)"yn", 1))
         {
         strcpy(cloudpath, CL_Win->CloudDir);
         strcpy(cloudfile, CL_Win->CloudFile);
         Proj_Mod = 1;
	 } /* if */
	} /* if */
       } /* if */
      } /* if */
     }
    } /* if old model modified */
   set(CL_Win->CloudWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, CL_Win->CloudWin);
   MUI_DisposeObject(CL_Win->CloudWin);
   } /* if window created */
  if (CL_Win->CD)
   CloudData_Del(CL_Win->CD);
  free_Memory(CL_Win, sizeof (struct CloudWindow));
  CL_Win = NULL;
  } /* if memory allocated */

} /* Close_CL_Window() */

/*********************************************************************/

void Handle_CL_Window(ULONG WCS_ID)
{
char *FloatData;
short i;
long data;
double FloatVal;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_CL_Window();
   return;
   } /* Open Directory List Window */

  if (! CL_Win)
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
     case ID_CL_WAVEADD:
      {
      short WaveNum;
      char TextStr[32];

      if ((WaveNum = Wave_Add(&CL_Win->CD->WD->Wave)) >= 0)
       {
       CL_Win->CD->WD->NumWaves = WaveNum;
       sprintf(TextStr, "%d", WaveNum);
       set(CL_Win->Text, MUIA_Text_Contents, (IPTR)TextStr);
       CL_Win->Mod = CL_Win->ReGen = 1;
       if (WVWin[1])
        {
        BuildWaveList(WVWin[1], WVWin[1]->WD);
        WVWin[1]->WaveEntries = WVWin[1]->WD->NumWaves;
        WVWin[1]->CurrentWave = WVWin[1]->WaveEntries - 1;
        WVWin[1]->CurWave = WVWin[1]->WaveAddrList[WVWin[1]->CurrentWave];
        nnset(WVWin[1]->LS_WaveList, MUIA_List_Active, WVWin[1]->CurrentWave);
        GUIWave_SetGads(WVWin[1], WVWin[1]->CurWave);
	} /* if */
       } /* if something actually done */
      break;
      }
     case ID_CL_WAVEEDIT:
      {
      Make_WV_Window(1, (char*)GetString( MSG_CLOUDGUI_CLOUDWAVEEDITOR ) );  // "Cloud Wave Editor"
      break;
      }
     case ID_CL_DRAW:
      {
      if (CL_Win->ReGen || ! CL_Win->CD->CloudPlane)
       {
       if (Cloud_Generate(CL_Win->CD, (double)CL_Win->WKS.Frame))
        {
/* rows and columns may be changed during generation */
        set(CL_Win->CloudStr[0], MUIA_String_Integer, CL_Win->CD->Rows);
        set(CL_Win->CloudStr[1], MUIA_String_Integer, CL_Win->CD->Cols);
        CL_Win->ReGen = 0;
	} /* if */
       else
        break;
       }
      Cloud_Draw(CL_Win->CD);
      break;
      }
     case ID_CL_BOUNDS:
      {
      if (Cloud_SetBounds(CL_Win->CD))
       {
       set(CL_Win->CloudStr[0], MUIA_String_Integer, CL_Win->CD->Rows);
       set(CL_Win->CloudStr[1], MUIA_String_Integer, CL_Win->CD->Cols);
       setfloat(CL_Win->CloudStr[2], CL_Win->CD->Lat[0]);
       setfloat(CL_Win->CloudStr[3], CL_Win->CD->Lat[1]);
       setfloat(CL_Win->CloudStr[4], CL_Win->CD->Lon[0]);
       setfloat(CL_Win->CloudStr[5], CL_Win->CD->Lon[1]);
       CL_Win->Mod = CL_Win->ReGen = 1;
       } /* if cloud bounds set */
      break;
      }
     case ID_CL_MAKEKEY:
      {
      short KeyFrame;
      char FrameStr[32];

      sprintf(FrameStr, "%d", CL_Win->WKS.Frame);
      if (! GetInputString((char*)GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ) ,  // "Enter frame to make key for."
    		  (char*)"abcdefghijklmnopqrstuvwxyz", FrameStr))
       break;

      KeyFrame = atoi(FrameStr);
      if (MakeGenericKeyFrame(&CL_Win->CD->CloudKey, &CL_Win->CD->KFSize,
	&CL_Win->CD->NumKeys, KeyFrame, CL_Win->WKS.Group, CL_Win->WKS.Item,
	CL_Win->WKS.Item, CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
	CL_Win->WKS.TCB, CL_Win->WKS.Linear, CL_Win->WKS.Precision))
       {
       UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
		CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
		CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
		CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
       GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
       GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);
       TL_Recompute(CL_Win->TL);
       CL_Win->Mod = 1;
       } /* if */
      break;
      }
     case ID_CL_UPDATEKEYS:
      {
      UpdateGenericKeyFrames(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, CL_Win->WKS.Item,
	CL_Win->WKS.NumValues,
	&CL_Win->CD->Coverage, NULL, NULL, CL_Win->WKS.TCB, CL_Win->WKS.Linear,
	0, 0, NULL, NULL, CL_Win->WKS.Precision);
      TL_Recompute(CL_Win->TL);
      CL_Win->Mod = 1;
      break;
      }
     case ID_CL_DELKEYS:
      {
      if (User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,     // "Cloud Editor"
    		  GetString( MSG_CLOUDGUI_DELETEALLCLOUDKEYFRAMES ) ,  // "Delete all cloud key frames?"
                  GetString( MSG_GLOBAL_OKCANCEL ) ,                 // "OK|Cancel"
                  (CONST_STRPTR)"oc", 1))
       {
       CL_Win->CD->NumKeys = 0;
       memset(CL_Win->CD->CloudKey, 0, CL_Win->CD->KFSize);
       UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
	CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
	CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
       GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
       GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);
       TL_Recompute(CL_Win->TL);
       CL_Win->Mod = 1;
       } /* if */
      break;
      }
     case ID_CL_DELETEKEY:
      {
      DeleteGenericKeyFrame(CL_Win->CD->CloudKey, &CL_Win->CD->NumKeys,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, 0,
	NULL, NULL);
      UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
	CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
	CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
      GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
      GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);
      TL_Recompute(CL_Win->TL);
      CL_Win->Mod = 1;
      break;
      }
     case ID_CL_NEXTKEY:
      {
      if (CL_Win->WKS.NextKey >= 0)
       {
       CL_Win->WKS.Frame = CL_Win->WKS.NextKey;
       set(CL_Win->GKS.Str[0], MUIA_String_Integer, CL_Win->WKS.Frame);
       } /* if */
      break;
      } /* Next Key */
     case ID_CL_PREVKEY:
      {
      if (CL_Win->WKS.PrevKey >= 0)
       {
       CL_Win->WKS.Frame = CL_Win->WKS.PrevKey;
       set(CL_Win->GKS.Str[0], MUIA_String_Integer, CL_Win->WKS.Frame);
       } /* if */
      break;
      } /* Prev Key */
     case ID_CL_LOAD:
      {
      char filename[256], *Ptrn = "#?.cld";

      if (getfilenameptrn
	(0, (char*)GetString( MSG_CLOUDGUI_CLOUDPATHFILE ) , CL_Win->CloudDir, CL_Win->CloudFile, Ptrn))  // "Cloud Path/File"
       {
       strmfp(filename, CL_Win->CloudDir, CL_Win->CloudFile);
       if (Cloud_Load(filename, &CL_Win->CD))
        {
        UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
		CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
		CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
		CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
        GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
        GUICloud_SetGads(CL_Win, CL_Win->CD);
        GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);
        if (strcmp(cloudpath, CL_Win->CloudDir) || strcmp(cloudfile, CL_Win->CloudFile))
         {
         if (User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
        		 GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                         GetString( MSG_GLOBAL_YESNO ),                             // "Yes|No"
                         (CONST_STRPTR)"yn", 1))
          {
          strcpy(cloudpath, CL_Win->CloudDir);
          strcpy(cloudfile, CL_Win->CloudFile);
          Proj_Mod = 1;
	  } /* if */
	 } /* if */
        TL_Recompute(CL_Win->TL);
        CL_Win->Mod = 0;
	} /* if */
       } /* if */
      break;
      }
     case ID_CL_SAVE:
      {
      char filename[256], *Ptrn = "#?.cld";

      if (getfilenameptrn
	(1, (char*)GetString( MSG_CLOUDGUI_CLOUDPATHFILE ), CL_Win->CloudDir, CL_Win->CloudFile, Ptrn))  // "Cloud Path/File"
       {
       if (strcmp(&CL_Win->CloudFile[strlen(CL_Win->CloudFile) - 4], ".cld"))
        strcat(CL_Win->CloudFile, ".cld");
       strmfp(filename, CL_Win->CloudDir, CL_Win->CloudFile);
       if (Cloud_Save(filename, CL_Win->CD))
        {
        if (strcmp(cloudpath, CL_Win->CloudDir) || strcmp(cloudfile, CL_Win->CloudFile))
         {
         if (User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
        		 GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                         GetString( MSG_GLOBAL_YESNO ) ,                            // "Yes|No", 
                         (CONST_STRPTR)"yn", 1))
          {
          strcpy(cloudpath, CL_Win->CloudDir);
          strcpy(cloudfile, CL_Win->CloudFile);
          Proj_Mod = 1;
	  } /* if */
	 } /* if */
        CL_Win->Mod = 0;
	} /* if saved OK */
       } /* if file name */
      break;
      }
     case ID_CL_TIMELINES:
      {
      static const char *Titles[8];
      static int Init=TRUE;
      if(Init)  // Only once necessary
      {
       Init=FALSE;
       Titles[0] = (const char*)GetString( MSG_CLOUDGUI_COVERAGE );          // "Coverage"
       Titles[1] = (const char*)GetString( MSG_CLOUDGUI_DENSITY );           // "Density"
       Titles[2] = (const char*)GetString( MSG_CLOUDGUI_ROUGHNESS );         // "Roughness"
       Titles[3] = (const char*)GetString( MSG_CLOUDGUI_FRACTALDIMENSION );  // "Fractal Dimension"
       Titles[4] = (const char*)GetString( MSG_CLOUDGUI_ALTITUDE );          // "Altitude"
       Titles[5] = (const char*)GetString( MSG_CLOUDGUI_MOVELATITUDE );      // "Move Latitude"
       Titles[6] = (const char*)GetString( MSG_CLOUDGUI_MOVELONGITUDE );     // "Move Longitude"
       Titles[7] = NULL;
      }
      Make_TL_Window((char*)GetString( MSG_CLOUDGUI_CLOUDTIMELINES ), (char **)Titles, &CL_Win->TL,  // "Cloud Time Lines"
	CL_Win->FloatStr, &CL_Win->WKS, &CL_Win->GKS, &CL_Win->CD->CloudKey,
	&CL_Win->CD->KFSize, &CL_Win->CD->NumKeys, &CL_Win->CD->Coverage,
	NULL, NULL);
      break;
      }
     case ID_CL_SCALEKEYS:
      {
      break;
      }
     case ID_CL_CLOSE:
      {
      Close_CL_Window();
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    get(CL_Win->Check, MUIA_Selected, &data);
    CloudData_SetShort(CL_Win->CD, CLOUDDATA_DYNAMIC, (short)data);
    CL_Win->Mod = CL_Win->ReGen = 1;
    break;
    } /* BUTTONS2 */

   case GP_CYCLE1:
    {
    get(CL_Win->Cycle, MUIA_Cycle_Active, &data);
    Cloud_SetDefaults(CL_Win->CD, (short)data, 0);
    GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
    GUICloud_SetGads(CL_Win, CL_Win->CD);
    CL_Win->Mod = CL_Win->ReGen = 1;
    break;
    } /* CYCLE1 */

   case GP_STRING1:
    {
    char *floatdata;

    i = WCS_ID - ID_CL_FLOATSTR(0);
    get(CL_Win->FloatStr[i], MUIA_String_Contents, &floatdata);
    switch (i)
     {
     case 0:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_COVERAGE, atof(floatdata));
      break; 
      }
     case 1:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_DENSITY, atof(floatdata));
      break; 
      }
     case 2:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_STDDEV, atof(floatdata));
      break; 
      }
     case 3:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_H, atof(floatdata));
      break; 
      }
     case 4:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_MINALT, atof(floatdata));
      break; 
      }
     case 5:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_LATOFF, atof(floatdata));
      break; 
      }
     case 6:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_LONOFF, atof(floatdata));
      break; 
      }
     } /* switch */
    Update_TL_Win(CL_Win->TL, i);
    CL_Win->Mod = 1;
    if (i > 1 && i != 6)
     CL_Win->ReGen = 1;
    break;
    } /* STRING1 */

   case GP_STRING2:
    {
    char *floatdata;

    i = WCS_ID - ID_CL_CLOUDSTR(0);
    get(CL_Win->CloudStr[i], MUIA_String_Contents, &floatdata);
    switch (i)
     {
     case 0:
      {
      CloudData_SetLong(CL_Win->CD, CLOUDDATA_ROWS, atoi(floatdata));
      break; 
      }
     case 1:
      {
      CloudData_SetLong(CL_Win->CD, CLOUDDATA_COLS, atoi(floatdata));
      break; 
      }
     case 2:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_NWLAT, atof(floatdata));
      break; 
      }
     case 3:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_SELAT, atof(floatdata));
      break; 
      }
     case 4:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_NWLON, atof(floatdata));
      break; 
      }
     case 5:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_SELON, atof(floatdata));
      break; 
      }
     } /* switch */
    CL_Win->Mod = 1;
    CL_Win->ReGen = 1;
    break;
    } /* STRING2 */

   case GP_STRING3:
    {
    get(CL_Win->IntStr[0], MUIA_String_Integer, &data);
    CloudData_SetLong(CL_Win->CD, CLOUDDATA_RANDSEED, data);
    CL_Win->Mod = CL_Win->ReGen = 1;
    break;
    } /* STRING2 */

   case GP_STRING4:
    {
    get(CL_Win->GKS.Str[0], MUIA_String_Integer, &data);
    CL_Win->WKS.Frame = data;
    UnsetGenericKeyFrame(CL_Win->CD->CloudKey, CL_Win->CD->NumKeys, &CL_Win->WKS,
	CL_Win->WKS.Frame, CL_Win->WKS.Group, CL_Win->WKS.Item, 0, CL_Win->WKS.Item,
	CL_Win->WKS.NumValues, &CL_Win->CD->Coverage, NULL, NULL,
	CL_Win->WKS.TCB, &CL_Win->WKS.Linear, CL_Win->WKS.Precision);
    GUICloudKey_SetGads(CL_Win, CL_Win->CD, CL_Win->WKS.Frame);
    GUIDisableKeyButtons(&CL_Win->GKS, CL_Win->TL, &CL_Win->WKS);
    if (CL_Win->TL)
     {
     long data2;

     get(CL_Win->TL->Prop[2], MUIA_Prop_First, &data2);
     data = (100.0 * ((float)CL_Win->WKS.Frame / (float)CL_Win->TL->Frames));
     if (data != data2 && ! CL_Win->WKS.PropBlock)
      { 
      set(CL_Win->TL->Prop[2], MUIA_Prop_First, data);
      CL_Win->WKS.StrBlock = 1;
      } /* if */      
     CL_Win->WKS.PropBlock = 0;
     if (CL_Win->WKS.IsKey >= 0)
      {
      CL_Win->TL->ActiveKey = GetActiveGenericKey(CL_Win->TL->SKT, CL_Win->WKS.Frame);
      sprintf(str, "%d", CL_Win->WKS.Frame);
      set(CL_Win->TL->FrameTxt, MUIA_Text_Contents, (IPTR)str);
      TL_Redraw(CL_Win->TL);
      }/* if key frame */
     } /* if time line window open */
    CL_Win->ReGen = 1;
    break;
    } /* STRING4 */

   case GP_ARROW1:
   case GP_ARROW2:
    {
    double Sign = .10;

    if ((WCS_ID & 0x0000ff00) == GP_ARROW1)
     {
     i = WCS_ID - ID_CL_FLOATARROWLEFT(0);
     Sign = -.10;
     }
    else
     i = WCS_ID - ID_CL_FLOATARROWRIGHT(0);

    if (i == 0 || i == 1)
     Sign *= 100.0;

    get(CL_Win->FloatStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal += Sign;
    setfloat(CL_Win->FloatStr[i], FloatVal);
     
    switch (i)
     {
     case 0:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_COVERAGE, FloatVal);
      break;
      }
     case 1:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_DENSITY, FloatVal);
      break;
      }
     case 2:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_STDDEV, FloatVal);
      break;
      }
     case 3:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_H, FloatVal);
      break;
      }
     case 4:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_MINALT, FloatVal);
      break;
      }
     case 5:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_LATOFF, FloatVal);
      break;
      }
     case 6:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_LONOFF, FloatVal);
      break;
      }
     } /* switch */
    Update_TL_Win(CL_Win->TL, i);
    CL_Win->Mod = 1;
    break;
    } /* ARROW1 ARROW2 */

   case GP_ARROW3:
   case GP_ARROW4:
    {
    double Sign = .10;

    if ((WCS_ID & 0x0000ff00) == GP_ARROW3)
     {
     i = WCS_ID - ID_CL_CLOUDARROWLEFT(0);
     Sign = -.10;
     }
    else
     i = WCS_ID - ID_CL_CLOUDARROWRIGHT(0);

    if (i == 0 || i == 1)
     Sign *= 100.0;

    get(CL_Win->CloudStr[i], MUIA_String_Contents, &FloatData);
    FloatVal = atof(FloatData);
    FloatVal += Sign;
    setfloat(CL_Win->CloudStr[i], FloatVal);
     
    switch (i)
     {
     case 0:
      {
      CloudData_SetLong(CL_Win->CD, CLOUDDATA_ROWS, (long)FloatVal);
      break;
      }
     case 1:
      {
      CloudData_SetLong(CL_Win->CD, CLOUDDATA_COLS, (long)FloatVal);
      break;
      }
     case 2:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_NWLAT, FloatVal);
      break;
      }
     case 3:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_SELAT, FloatVal);
      break;
      }
     case 4:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_NWLON, FloatVal);
      break;
      }
     case 5:
      {
      CloudData_SetDouble(CL_Win->CD, CLOUDDATA_SELON, FloatVal);
      break;
      }
     } /* switch */
    CL_Win->Mod = 1;
    break;
    } /* ARROW3 ARROW4 */

   case GP_ARROW5:
    {
    i = WCS_ID - ID_CL_STRARROW(0);
    
    i = i - (1 - i);
    CL_Win->WKS.Frame += i;

    if (CL_Win->WKS.Frame < 0)
     CL_Win->WKS.Frame = 0;
    set(CL_Win->GKS.Str[0], MUIA_String_Integer, CL_Win->WKS.Frame);
    break;
    } /* frame arrows */
   } /* switch gadget group */

} /* Handle_CL_Window() */

/********************************************************************/

short Cloud_Load(char *filename, struct CloudData **CDPtr)
{
char Title[32];
short success = 1, KFItems = 0, Version;
long i, Item, Color;
double DoubleVal;
FILE *fCloud;
struct CloudData *CD;
struct WaveData *WD = NULL;
struct Wave *WV = NULL;
struct CloudLayer *CL, *CLPrev;
union KeyFrame *KFPtr;

 if (! filename)
  return (0);

 if ((fCloud = fopen(filename, "r")))
  {
  fgets(Title, 24, fCloud);
  Title[8] = '\0';
  if (! strcmp(Title, "WCSCloud"))
   {
   fscanf(fCloud, "%hd", &Version);
   
   if (*CDPtr)
    CloudData_Del(*CDPtr);

   *CDPtr = CloudData_New();
   CD = *CDPtr;
   if (CD)
    {
    WD = CD->WD;
    while ((fscanf(fCloud, "%ld", &Item) == 1) && success)
     {
     switch (Item)
      {
      case CLOUDDATA_SHT_NUMKEYS:
       fscanf(fCloud, "%hd", &CD->NumKeys);
       break;
      case CLOUDDATA_LNG_ROWS:
       fscanf(fCloud, "%ld", &CD->Rows);
       break;
      case CLOUDDATA_LNG_COLS:
       fscanf(fCloud, "%ld", &CD->Cols);
       break;
      case CLOUDDATA_SHT_NUMWAVES:
       fscanf(fCloud, "%hd", &CD->WD->NumWaves);
       break;
      case CLOUDDATA_SHT_NUMLAYERS:
       fscanf(fCloud, "%hd", &CD->NumLayers);
       break;
      case CLOUDDATA_SHT_DYNAMIC:
       fscanf(fCloud, "%hd", &CD->Dynamic);
       break;
      case CLOUDDATA_SHT_CLOUDTYPE:
       fscanf(fCloud, "%hd", &CD->CloudType);
       break;
      case CLOUDDATA_DBL_COVERAGE:
       fscanf(fCloud, "%le", &CD->Coverage);
       break;
      case CLOUDDATA_DBL_DENSITY:
       fscanf(fCloud, "%le", &CD->Density);
       break;
      case CLOUDDATA_DBL_NWLAT:
       fscanf(fCloud, "%le", &CD->Lat[0]);
       break;
      case CLOUDDATA_DBL_SELAT:
       fscanf(fCloud, "%le", &CD->Lat[1]);
       break;
      case CLOUDDATA_DBL_NWLON:
       fscanf(fCloud, "%le", &CD->Lon[0]);
       break;
      case CLOUDDATA_DBL_SELON:
       fscanf(fCloud, "%le", &CD->Lon[1]);
       break;
      case CLOUDDATA_DBL_MAXALT:
       fscanf(fCloud, "%le", &CD->AltDiff);
       CD->AltDiff = .0083;		/* changed this to be the interval between layers */
       break;
      case CLOUDDATA_DBL_MINALT:
       fscanf(fCloud, "%le", &CD->Alt);
       break;
      case CLOUDDATA_LNG_RANDSEED:
       fscanf(fCloud, "%ld", &CD->RandSeed);
       break;
      case CLOUDDATA_DBL_STDDEV:
       fscanf(fCloud, "%le", &CD->StdDev);
       break;
      case CLOUDDATA_DBL_H:
       fscanf(fCloud, "%le", &CD->H);
       break;
      case CLOUDDATA_DBL_LATOFF:
       fscanf(fCloud, "%le", &CD->LatOff);
       break;
      case CLOUDDATA_DBL_LONOFF:
       fscanf(fCloud, "%le", &CD->LonOff);
       break;
      case CLOUDDATA_BYT_COLOR:
       for (i=0; i<9; i++)
        {
        fscanf(fCloud, "%ld", &Color);
/* this is a dummy since colors are not used */
	} /* for i=0... */
       break;

      case CLOUDLAYER_NEW:
       if (CD->Layer == NULL)
        {
        if ((CD->Layer = CloudLayer_New()) == NULL)
         {
         success = 0;
         break;
	 }
        CL = CD->Layer;
	} /* if first layer */
       else
        {
        CLPrev = CL;
        if ((CL->Next = CloudLayer_New()) == NULL)
         {
         success = 0;
         break;
	 }
        CL = CL->Next;
        CL->Prev = CLPrev;
	} /* else */
       break;
      case CLOUDLAYER_DBL_ALT:
       fscanf(fCloud, "%le", &CL->Alt);
       break;
      case CLOUDLAYER_DBL_COVG:
       fscanf(fCloud, "%le", &CL->Covg);
       break;
      case CLOUDLAYER_DBL_DENS:
       fscanf(fCloud, "%le", &CL->Dens);
       break;
      case CLOUDLAYER_DBL_ILLUM:
       fscanf(fCloud, "%le", &CL->Illum);
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
       if (WV)
        fscanf(fCloud, "%hd", &WV->NumKeys);
       else
        success = 0;
       break;
      case WAVE_DBL_AMP:
       if (WV)
        fscanf(fCloud, "%le", &WV->Amp);
       else
        success = 0;
       break;
      case WAVE_DBL_LENGTH:
       if (WV)
        fscanf(fCloud, "%le", &WV->Length);
       else
        success = 0;
       break;
      case WAVE_DBL_VELOCITY:
       if (WV)
        fscanf(fCloud, "%le", &WV->Velocity);
       else
        success = 0;
       break;
      case WAVE_DBL_LAT:
       if (WV)
        fscanf(fCloud, "%le", &WV->Pt.Lat);
       else
        success = 0;
       break;
      case WAVE_DBL_LON:
       if (WV)
        fscanf(fCloud, "%le", &WV->Pt.Lon);
       else
        success = 0;
       break;

      case CLOUDDATA_SHT_KFITEMS:
       fscanf(fCloud, "%hd", &KFItems);
       break;
      case CLOUDDATA_SHT_KEYFRAME:
       if (! CD->CloudKey)
        {
        if ((CD->CloudKey = (union KeyFrame *)
		get_Memory((CD->NumKeys + 10) * sizeof (union KeyFrame),
		MEMF_CLEAR)) == NULL)
         {
         success = 0;
         break;
	 } /* if */
        CD->KFSize = (CD->NumKeys + 10) * sizeof (union KeyFrame);
        KFPtr = CD->CloudKey;
	} /* if */
       else
        {
        KFPtr ++;
	} /* else */
       fscanf(fCloud, "%hd", &KFPtr->CldKey.KeyFrame);
       break;
      case CLOUDDATA_SHT_KFGROUP:
       fscanf(fCloud, "%hd", &KFPtr->CldKey.Group);
       break;
      case CLOUDDATA_SHT_KFITEM:
       fscanf(fCloud, "%hd", &KFPtr->CldKey.Item);
       break;
      case CLOUDDATA_SHT_KFLINEAR:
       fscanf(fCloud, "%hd", &KFPtr->CldKey.Linear);
       break;
      case CLOUDDATA_FLT_KFTCB:
       for (i=0; i<3; i++)
        {
        fscanf(fCloud, "%le", &DoubleVal);
        KFPtr->CldKey.TCB[i] = DoubleVal;
	} /* for i=0... */
       break;
      case CLOUDDATA_FLT_KFVALUES:
       for (i=0; i<KFItems; i++)
        {
        fscanf(fCloud, "%le", &DoubleVal);
        KFPtr->CldKey.Value[i] = DoubleVal;
	} /* for i=0... */
       break;
      } /* switch */
     } /* while */
    } /* if CD */
   else
    {
    Log(ERR_MEM_FAIL, GetString( MSG_CLOUDGUI_CLOUDFILE ));  // "Cloud File"
    success = 0;
    }
   } /* if correct file type */
  else
   {
   Log(ERR_WRONG_TYPE, GetString( MSG_CLOUDGUI_CLOUDFILE ));  // "Cloud File"
   success = 0;
   } /* else */
  fclose(fCloud);
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, GetString( MSG_CLOUDGUI_CLOUDFILE ));  // "Cloud File"
  success = 0;
  } /* else no file */

 return (success);

} /* Cloud_Load() */

/*********************************************************************/

short Cloud_Save(char *filename, struct CloudData *CD)
{
short i, j, KFItems = 7, Version = CLOUD_CURRENT_VERSION;
FILE *fCloud;
struct Wave *WV;
struct CloudLayer *CL;
union KeyFrame *KFPtr;

 if (! filename || ! CD)
  return (0);

 if ((fCloud = fopen(filename, "w")))
  {
  fprintf(fCloud, "WCSCloud\n");

  fprintf(fCloud, "%d\n", Version);

  fprintf(fCloud, "%d\n", CLOUDDATA_SHT_NUMKEYS);
  fprintf(fCloud, "%d\n", CD->NumKeys);
  fprintf(fCloud, "%d\n", CLOUDDATA_LNG_ROWS);
  fprintf(fCloud, "%ld\n", CD->Rows);
  fprintf(fCloud, "%d\n", CLOUDDATA_LNG_COLS);
  fprintf(fCloud, "%ld\n", CD->Cols);
  fprintf(fCloud, "%d\n", CLOUDDATA_SHT_NUMWAVES);
  fprintf(fCloud, "%d\n", CD->WD->NumWaves);
  fprintf(fCloud, "%d\n", CLOUDDATA_SHT_NUMLAYERS);
  fprintf(fCloud, "%d\n", CD->NumLayers);
  fprintf(fCloud, "%d\n", CLOUDDATA_SHT_DYNAMIC);
  fprintf(fCloud, "%d\n", CD->Dynamic);
  fprintf(fCloud, "%d\n", CLOUDDATA_SHT_CLOUDTYPE);
  fprintf(fCloud, "%d\n", CD->CloudType);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_COVERAGE);
  fprintf(fCloud, "%f\n", CD->Coverage);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_DENSITY);
  fprintf(fCloud, "%f\n", CD->Density);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_NWLAT);
  fprintf(fCloud, "%f\n", CD->Lat[0]);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_SELAT);
  fprintf(fCloud, "%f\n", CD->Lat[1]);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_NWLON);
  fprintf(fCloud, "%f\n", CD->Lon[0]);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_SELON);
  fprintf(fCloud, "%f\n", CD->Lon[1]);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_MAXALT);
  fprintf(fCloud, "%f\n", CD->AltDiff);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_MINALT);
  fprintf(fCloud, "%f\n", CD->Alt);
  fprintf(fCloud, "%d\n", CLOUDDATA_LNG_RANDSEED);
  fprintf(fCloud, "%ld\n", CD->RandSeed);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_STDDEV);
  fprintf(fCloud, "%f\n", CD->StdDev);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_H);
  fprintf(fCloud, "%f\n", CD->H);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_LATOFF);
  fprintf(fCloud, "%f\n", CD->LatOff);
  fprintf(fCloud, "%d\n", CLOUDDATA_DBL_LONOFF);
  fprintf(fCloud, "%f\n", CD->LonOff);

  CL = CD->Layer;
  while (CL)
   {
   fprintf(fCloud, "%d\n", CLOUDLAYER_NEW);
   fprintf(fCloud, "%d\n", CLOUDLAYER_DBL_ALT);
   fprintf(fCloud, "%f\n", CL->Alt);
   fprintf(fCloud, "%d\n", CLOUDLAYER_DBL_COVG);
   fprintf(fCloud, "%f\n", CL->Covg);
   fprintf(fCloud, "%d\n", CLOUDLAYER_DBL_DENS);
   fprintf(fCloud, "%f\n", CL->Dens);
   fprintf(fCloud, "%d\n", CLOUDLAYER_DBL_ILLUM);
   fprintf(fCloud, "%f\n", CL->Illum);
   CL = CL->Next;
   } /* while */

  if (CD->WD)
   {
   WV = CD->WD->Wave;
   while (WV)
    {
    fprintf(fCloud, "%d\n", WAVE_NEW);
    fprintf(fCloud, "%d\n", WAVE_SHT_NUMKEYS);
    fprintf(fCloud, "%d\n", WV->NumKeys);
    fprintf(fCloud, "%d\n", WAVE_DBL_AMP);
    fprintf(fCloud, "%f\n", WV->Amp);
    fprintf(fCloud, "%d\n", WAVE_DBL_LENGTH);
    fprintf(fCloud, "%f\n", WV->Length);
    fprintf(fCloud, "%d\n", WAVE_DBL_VELOCITY);
    fprintf(fCloud, "%f\n", WV->Velocity);
    fprintf(fCloud, "%d\n", WAVE_DBL_LAT);
    fprintf(fCloud, "%f\n", WV->Pt.Lat);
    fprintf(fCloud, "%d\n", WAVE_DBL_LON);
    fprintf(fCloud, "%f\n", WV->Pt.Lon);
    WV = WV->Next;
    } /* while */
   } /* if */

  KFPtr = CD->CloudKey;
  if (CD->NumKeys)
   {
   fprintf(fCloud, "%d\n", CLOUDDATA_SHT_KFITEMS);
   fprintf(fCloud, "%d\n", KFItems);
   for (i=0; i<CD->NumKeys; i++, KFPtr ++)
    {
    fprintf(fCloud, "%d\n", CLOUDDATA_SHT_KEYFRAME);
    fprintf(fCloud, "%d\n", KFPtr->CldKey.KeyFrame);
    fprintf(fCloud, "%d\n", CLOUDDATA_SHT_KFGROUP);
    fprintf(fCloud, "%d\n", KFPtr->CldKey.Group);
    fprintf(fCloud, "%d\n", CLOUDDATA_SHT_KFITEM);
    fprintf(fCloud, "%d\n", KFPtr->CldKey.Item);
    fprintf(fCloud, "%d\n", CLOUDDATA_SHT_KFLINEAR);
    fprintf(fCloud, "%d\n", KFPtr->CldKey.Linear);
    fprintf(fCloud, "%d\n", CLOUDDATA_FLT_KFTCB);
    fprintf(fCloud, "%f ",  KFPtr->CldKey.TCB[0]);
    fprintf(fCloud, "%f ",  KFPtr->CldKey.TCB[1]);
    fprintf(fCloud, "%f\n", KFPtr->CldKey.TCB[2]);
    fprintf(fCloud, "%d\n", CLOUDDATA_FLT_KFVALUES);
    for (j=0; j<KFItems; j++)
     fprintf(fCloud, "%f ", KFPtr->CldKey.Value[j]);
    fprintf(fCloud, "\n");
    } /* for i=0... */
   } /* if key frames exist */  
  fclose(fCloud);
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, GetString( MSG_CLOUDGUI_CLOUDFILE ));  // "Cloud File"
  return (0);
  } /* else no file */

 return (1);

} /* Cloud_Save() */

/*********************************************************************/

STATIC_FCN void GUICloud_SetGads(struct CloudWindow *CL_Win,
	struct CloudData *CD) // used locally only -> static, AF 26.7.2021
{
char TextStr[32];

 nnset(CL_Win->Cycle, MUIA_Cycle_Active, CloudData_GetShort(CD, CLOUDDATA_CLOUDTYPE));

 if(CD->WD)  // AF, 13.Dec.2022 -386-aros prevent NULL-Ptr Access. (New Start, Parameter Module -> Clouds
 {
 sprintf(TextStr, "%d", CloudData_GetShort(CD, CLOUDDATA_NUMWAVES));  // ALEXANDER: Crash NULL-Ptr Access
 }
 else
 {
     sprintf(TextStr,"0");
 }
 set(CL_Win->Text, MUIA_Text_Contents, (IPTR)TextStr);

 nnset(CL_Win->IntStr[0], MUIA_String_Integer, CloudData_GetLong(CD, CLOUDDATA_RANDSEED));
 nnset(CL_Win->CloudStr[0], MUIA_String_Integer, CloudData_GetLong(CD, CLOUDDATA_ROWS));
 nnset(CL_Win->CloudStr[1], MUIA_String_Integer, CloudData_GetLong(CD, CLOUDDATA_COLS));
 nnsetfloat(CL_Win->CloudStr[2], CloudData_GetDouble(CD, CLOUDDATA_NWLAT));
 nnsetfloat(CL_Win->CloudStr[3], CloudData_GetDouble(CD, CLOUDDATA_SELAT));
 nnsetfloat(CL_Win->CloudStr[4], CloudData_GetDouble(CD, CLOUDDATA_NWLON));
 nnsetfloat(CL_Win->CloudStr[5], CloudData_GetDouble(CD, CLOUDDATA_SELON));
 nnset(CL_Win->Check, MUIA_Selected, CloudData_GetShort(CD, CLOUDDATA_DYNAMIC));
 
} /* GUICloud_SetGads() */

/***********************************************************************/

STATIC_FCN void GUICloudKey_SetGads(struct CloudWindow *CL_Win,
	struct CloudData *CD, short Frame) // used locally only -> static, AF 26.7.2021
{

 if (CD && CL_Win)
  {
  GetGenericKeyTableValues(&CD->KT, CD->CloudKey, CD->NumKeys,
	&CD->KT_MaxFrames, CL_Win->WKS.NumValues, CL_Win->WKS.Group,
	CL_Win->WKS.Item, Frame, &CD->Coverage, NULL, NULL,
	CL_Win->WKS.Precision);
  nnsetfloat(CL_Win->FloatStr[0], CloudData_GetDouble(CD, CLOUDDATA_COVERAGE));
  nnsetfloat(CL_Win->FloatStr[1], CloudData_GetDouble(CD, CLOUDDATA_DENSITY));
  nnsetfloat(CL_Win->FloatStr[2], CloudData_GetDouble(CD, CLOUDDATA_STDDEV));
  nnsetfloat(CL_Win->FloatStr[3], CloudData_GetDouble(CD, CLOUDDATA_H));
  nnsetfloat(CL_Win->FloatStr[5], CloudData_GetDouble(CD, CLOUDDATA_LATOFF));
  nnsetfloat(CL_Win->FloatStr[6], CloudData_GetDouble(CD, CLOUDDATA_LONOFF));
  nnsetfloat(CL_Win->FloatStr[4], CloudData_GetDouble(CD, CLOUDDATA_MINALT));
  } /* if */
 
} /* GUICloudKey_SetGads() */

/***********************************************************************/
#ifdef __SEEMS_TO_BE_COMMENTED__
short CloudKeyFrame_Make(struct CloudData *CD)
{
short i, KeyFrame, Found = 0;
union KeyFrame *KF;

/* get frame number for new key */

 sprintf(str, "%d", 0);
 if (! GetInputString((char*)GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ),  // "Enter frame to make key for."
		 (char*)"abcdefghijklmnopqrstuvwxyz", str))
  return (0);

 KeyFrame = atoi(str);

/* does a key frame already exist at that frame? */

 for (i=0; i<CD->NumKeys; i++)
  {
  if (CD->CloudKey[i].CldKey.KeyFrame == KeyFrame)
   {
   Found = 1;
   break;
   } /* if found */
  } /* for i=0... */

/* if no key frame exists here find the place for the new one and make space */

 if (! Found)
  {
  if ((CD->NumKeys + 1) * sizeof (union KeyFrame) > CD->KFSize)
   {
   if (! AllocNewKeyArray(&CD->CloudKey, &CD->KFSize))
    {
    return (0);
    } /* else */
   } /* if need new key frame array */

  if (CD->NumKeys > 0)
   {
   i = CD->NumKeys;
   while (KeyFrame < CD->CloudKey[i - 1].CldKey.KeyFrame && i > 0)
    {
    i --;
    } /* while */
   memmove(&CD->CloudKey[i + 1], &CD->CloudKey[i], (CD->NumKeys - 1) * sizeof (union KeyFrame));
   } /* if already at least one key frame */
  } /* if a key frame doesn't exist at the desired frame number */

 KF = &CD->CloudKey[i];

 KF->CldKey.KeyFrame = KeyFrame;
 KF->CldKey.Group = 3;
 KF->CldKey.Item = 0;
 KF->CldKey.Linear = 0;
 KF->CldKey.TCB[0] = KF->CldKey.TCB[1] = KF->CldKey.TCB[2] = 0.0; 
 KF->CldKey.Value[0] = CD->Coverage;
 KF->CldKey.Value[1] = CD->Density;
 KF->CldKey.Value[2] = CD->StdDev;
 KF->CldKey.Value[3] = CD->H;
 KF->CldKey.Value[4] = CD->Alt;
 KF->CldKey.Value[5] = CD->LatOff;
 KF->CldKey.Value[6] = CD->LonOff;
 if (! Found)
  CD->NumKeys ++;

 return (1);

} /* CloudKeyFrame_Make() */
#endif // __SEEMS_TO_BE_COMMENTED__
