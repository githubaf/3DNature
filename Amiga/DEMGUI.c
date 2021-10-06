/* DEMGUI.c
** Copyright 1995 by Questar Productions. All rights reserved.
** Code written 1995 by Gary R. Huber helpful suggestions from Chris Hanson.
*/

#include "WCS.h"
#include "GUIDefines.h"

STATIC_FCN short Grid_Save(struct NNGrid *NNG, short Units); // used locally only -> static, AF 19.7.2021
STATIC_FCN short Noise_Init(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short Contour_Import(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short XYZ_Export(struct datum *DT); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Make_GR_Window(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short DEMBuild_Import(short FileType); // used locally only -> static, AF 20.7.2021
STATIC_FCN short Grid_Draw(struct NNGrid *NNG); // used locally only -> static, AF 20.7.2021
STATIC_FCN void MD_Window_Init(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short DXF_Import(short CoordSys); // used locally only -> static, AF 20.7.2021
STATIC_FCN short Object_ImportXYZ(short Points, double *Lat, double *Lon,
        short *ElevPtr, float El, struct datum *DT); // used locally only -> static, AF 20.7.2021
STATIC_FCN void GR_Window_Init(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short XYZ_Import(short CoordSys); // used locally only -> static, AF 20.7.2021

/*
EXTERN struct MakeDEMWindow {
  APTR MakeDEMWin, Cycle[5], Text, IntStr[3], FloatStr[3], Prop, ArrowLeft[3],
	ArrowRight[3], Button[5], ModeButton[6], ReverseButton;
  struct datum *TC, *CurDat;
  short LastX, LastY, PMode, DMode, MaxEl, MinEl, CurEl, Units, ElSource,
	ControlPts, Displace, FileType, NoReversal;
  double UnitScale, StdDev, MinDist, NonLin;
  char FileIn[32], FileOut[32], DirIn[256], DirOut[256];
} *MD_Win;

EXTERN struct NNGridWindow {
  APTR NNGridWin, FloatStr[15], IntStr[5], Check[6], Button[3], Str[1],
	Text, Arrow[2], ApplyButton;
  struct NNGrid NNG;
  long NRows, NCols, RandSeed, Scope, OffX, OffY, NoiseSize;
  double XPow, YPow, Delta, H;
  float *NoiseMap;
} GR_Win;
*/

void Make_MD_Window(void)
{
long open, i;
static const char *MD_FileTypes[] = {"D'base Objects", "XYZ Lat/Lon", "XYZ UTM", "DXF Lat/Lon",
 "DXF UTM", NULL};
static const char *MD_ElevSource[] = {"Slider", "End Points", "DEM", "Numeric", NULL};
static const char *MD_DrawMode[] = {"Isoline", "Gradient", "Concave", "Convex", NULL};
static const char *MD_Displace[] = {"None", "Lines/Points", "Lines Only",
	"Points Only", NULL};
static const char *MD_ElevUnits[] = {"Kilometers", "Meters", "Centimeters",
	"Miles", "Feet", "Inches", NULL};

 if (MD_Win)
  {
  DoMethod(MD_Win->MakeDEMWin, MUIM_Window_ToFront);
  set(MD_Win->MakeDEMWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((MD_Win = (struct MakeDEMWindow *)
	get_Memory(sizeof (struct MakeDEMWindow), MEMF_CLEAR)) == NULL)
   return;

 if ((MD_Win->TC = Datum_New()) == NULL)	/* returns cleared datum struct */
  {
  Close_MD_Window();
  return;
  } /* if */
 MD_Win->CurDat = MD_Win->TC;
 MD_Window_Init();

  Set_Param_Menu(10);

     MD_Win->MakeDEMWin = WindowObject,
      MUIA_Window_Title		, "DEM Designer",
      MUIA_Window_ID		, 'DEMB',
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, VGroup,
	      Child, HGroup,
	        Child, Label1("Control Pts"),
	        Child, MD_Win->Text = TextObject, TextFrame,
			MUIA_Text_Contents, "0", End,
		End, /* HGroup */
	      Child, HGroup, MUIA_Group_SameWidth, TRUE,
		Child, MD_Win->ModeButton[0] = KeyButtonObject('m'),
			 MUIA_Selected, TRUE,
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cMap", End, 
		Child, MD_Win->ModeButton[1] = KeyButtonObject('a'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cAdd", End, 
		Child, MD_Win->ModeButton[2] = KeyButtonObject('v'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cMove", End, 
		Child, MD_Win->ModeButton[3] = KeyButtonObject('d'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cDel", End, 
		End, /* HGroup */
	      Child, HGroup, MUIA_Group_SameWidth, TRUE,
		Child, MD_Win->ModeButton[4] = KeyButtonObject('i'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cPt Info", End, 
		Child, MD_Win->ModeButton[5] = KeyButtonObject('t'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cSet El", End, 
		End, /* HGroup */
	      Child, HGroup,
		Child, Label2("Elev Source"),
		Child, MD_Win->Cycle[0] = CycleObject,
			MUIA_Cycle_Entries, MD_ElevSource,
			MUIA_Cycle_Active, MD_Win->ElSource, End,
		End, /* HGroup */
	      Child, HGroup,
		Child, Label2(" Elev Units"),
		Child, MD_Win->Cycle[1] = CycleObject,
			MUIA_Cycle_Entries, MD_ElevUnits,
			MUIA_Cycle_Active, MD_Win->Units, End,
		End, /* HGroup */
	      Child, HGroup,
		Child, Label2("   Displace"),
		Child, MD_Win->Cycle[2] = CycleObject,
			MUIA_Cycle_Entries, MD_Displace,
			MUIA_Cycle_Active, MD_Win->Displace, End,
		End, /* HGroup */
	      Child, HGroup,
		Child, Label2("  Draw Mode"),
		Child, MD_Win->Cycle[3] = CycleObject,
			MUIA_Cycle_Entries, MD_DrawMode,
			MUIA_Cycle_Active, MD_Win->DMode, End,
		End, /* HGroup */
	      Child, MD_Win->ReverseButton = KeyButtonObject('n'),
			 MUIA_InputMode, MUIV_InputMode_Toggle,
			 MUIA_Text_Contents, "\33cNo Gradient Reversal", End, 
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Minimum Spacing "),
	        Child, MD_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, MD_Win->ArrowLeft[0] = ImageButton(MUII_ArrowLeft),
                Child, MD_Win->ArrowRight[0] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("  Std Deviation "),
	        Child, MD_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, ".0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, MD_Win->ArrowLeft[1] = ImageButton(MUII_ArrowLeft),
                Child, MD_Win->ArrowRight[1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("  Non-linearity "),
	        Child, MD_Win->FloatStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, ".0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, MD_Win->ArrowLeft[2] = ImageButton(MUII_ArrowLeft),
                Child, MD_Win->ArrowRight[2] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup,
		Child, MD_Win->Button[0] = KeyButtonFunc('i', "\33cImport"), 
		Child, MD_Win->Cycle[4]= CycleObject,
			MUIA_Cycle_Entries, MD_FileTypes,
			MUIA_Cycle_Active, MD_Win->FileType, End,
		End, /* HGroup */
	      End, /* VGroup */
	    Child, HGroup,
	      Child, VGroup,
		Child, MD_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789-",
			MUIA_FixWidthTxt, "01234",
			MUIA_String_Integer, MD_Win->MaxEl, End,
		Child, RectangleObject, End,
		Child, MD_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789-",
			MUIA_FixWidthTxt, "01234",
			MUIA_String_Integer, MD_Win->CurEl, End,
		Child, RectangleObject, End,
		Child, MD_Win->IntStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789-",
			MUIA_FixWidthTxt, "01234",
			MUIA_String_Integer, MD_Win->MinEl, End,
		End, /* VGroup */
	      Child, MD_Win->Prop = PropObject, PropFrame,
			MUIA_Prop_Entries, 10130,
			MUIA_Prop_First, 5000,
			MUIA_Prop_Visible, 100, End,
	      End, /* HGroup */
	    End, /* HGroup */
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
	    Child, MD_Win->Button[1] = KeyButtonFunc('s', "\33cSave Pts"), 
	    Child, MD_Win->Button[2] = KeyButtonFunc('g', "\33cBuild..."), 
	    Child, MD_Win->Button[4] = KeyButtonFunc('d', "\33cDraw Pts"), 
	    Child, MD_Win->Button[3] = KeyButtonFunc('c', "\33cClear Pts"), 
	    End, /* HGroup */
	  End, /* VGroup */
	End; /* Window object */

  if (! MD_Win->MakeDEMWin)
   {
   Close_MD_Window();
   User_Message((CONST_STRPTR)"Map View: Build DEM", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, MD_Win->MakeDEMWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* set float values */
  setfloat(MD_Win->FloatStr[0], MD_Win->MinDist);
  setfloat(MD_Win->FloatStr[1], MD_Win->StdDev);
  setfloat(MD_Win->FloatStr[2], MD_Win->NonLin);

/* ReturnIDs */
  DoMethod(MD_Win->MakeDEMWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_MD_CLOSE);  

  MUI_DoNotiPresFal(app, MD_Win->Button[0], ID_MD_IMPORT,
   MD_Win->Button[1], ID_MD_SAVE, MD_Win->Button[2], ID_GR_WINDOW,
   MD_Win->Button[3], ID_MD_CLEAR, MD_Win->Button[4], ID_MD_DRAW, NULL);

  for (i=0; i<6; i++)
   DoMethod(MD_Win->ModeButton[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MD_MODE(i));

  DoMethod(MD_Win->ReverseButton, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MD_NOREVERSE);

/* STRING1 */
  for (i=0; i<3; i++)
   {
   DoMethod(MD_Win->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_MD_INTSTR(i));
   } /* for i=0... */
  for (i=0; i<3; i++)
   {
   DoMethod(MD_Win->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_MD_FLOATSTR(i));
   } /* for i=0... */

/* ARROW */
  for (i=0; i<3; i++)
   {
   MUI_DoNotiPresFal(app, MD_Win->ArrowLeft[i], ID_MD_ARROWLEFT(i), NULL);
   MUI_DoNotiPresFal(app, MD_Win->ArrowRight[i], ID_MD_ARROWRIGHT(i), NULL);
   } /* for i=0... */

/* CYCLE1 */
  for (i=0; i<5; i++)
   {
   DoMethod(MD_Win->Cycle[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_MD_CYCLE(i));
   } /* for i=0... */

/* PROP1 */
  DoMethod(MD_Win->Prop, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_MD_PROP);

/* set tab cycle chain */
  DoMethod(MD_Win->MakeDEMWin, MUIM_Window_SetCycleChain,
   MD_Win->ModeButton[0], MD_Win->ModeButton[1],
   MD_Win->ModeButton[2], MD_Win->ModeButton[3], MD_Win->ModeButton[4], MD_Win->ModeButton[5],
   MD_Win->Cycle[0], MD_Win->Cycle[1], MD_Win->Cycle[2],
   MD_Win->Cycle[3], MD_Win->FloatStr[0],
   MD_Win->FloatStr[1], MD_Win->FloatStr[2], MD_Win->Button[0], MD_Win->Cycle[4],
   MD_Win->IntStr[0], MD_Win->IntStr[1], MD_Win->IntStr[2],
   MD_Win->Prop, MD_Win->Button[1], MD_Win->Button[2], MD_Win->Button[4],
   MD_Win->Button[3],
   NULL);

/* set return cycle chain */
  DoMethod(MD_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->FloatStr[1]);
  DoMethod(MD_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->FloatStr[2]);
  DoMethod(MD_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->IntStr[0]);
  DoMethod(MD_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->IntStr[1]);
  DoMethod(MD_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->IntStr[2]);
  DoMethod(MD_Win->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MD_Win->MakeDEMWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MD_Win->FloatStr[0]);

/* Open window */
  set(MD_Win->MakeDEMWin, MUIA_Window_Open, TRUE);
  get(MD_Win->MakeDEMWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_MD_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

#ifdef UNDER_CONST
 UnderConst_New();
#endif /* UNDER_CONST */

} /* Make_MD_Window() */

/**********************************************************************/

void Close_MD_Window(void)
{

 if (MD_Win)
  {
  if (MD_Win->TC)
   Datum_Del(MD_Win->TC);
  if (MD_Win->MakeDEMWin)
   {
   set(MD_Win->MakeDEMWin, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, MD_Win->MakeDEMWin);
   MUI_DisposeObject(MD_Win->MakeDEMWin);
   } /* if */
  free_Memory(MD_Win, sizeof (struct MakeDEMWindow));
  MD_Win = NULL;
  } /* if */

} /* Close_MD_Window() */

/***********************************************************************/

void Handle_MD_Window(ULONG WCS_ID)
{
long i, data;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_MD_Window();
   return;
   } /* Open Render Settings Editor Window */

  if (! MD_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_MD_CLOSE:
      {
      if (GR_Win)
       {
       if (! User_Message((CONST_STRPTR)"Map View: Build DEM",
    		   (CONST_STRPTR)"This window must remain open while the DEM Gridder is open!\n\
Do you wish to close them both?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
        break;
       Close_GR_Window();
       } /* if DEM Gridder window is open */
      Close_MD_Window();
      break;
      } /* Close Window */
     case ID_MD_IMPORT:
      {
      DEMBuild_Import(MD_Win->FileType);
      sprintf(str, "%d", MD_Win->ControlPts);
      set(MD_Win->Text, MUIA_Text_Contents, str);
      break;
      } /*  */
     case ID_MD_SAVE:
      {
      XYZ_Export(MD_Win->TC->nextdat);
      break;
      } /*  */
     case ID_MD_DRAW:
      {
      long X, Y;
      struct datum *DT;

      if (MapWind0)
       {
       SetAPen(MapWind0->RPort, 7);
       DT = MD_Win->TC->nextdat;
       while (DT)
        {
        X = Lon_X_Convert(DT->values[0]);
        Y = Lat_Y_Convert(DT->values[1]);
        WritePixel(MapWind0->RPort, X, Y);
        DT = DT->nextdat;
	} /* while */
       } /* if */
      break;
      } /*  */
     case ID_MD_CLEAR:
      {
      long X, Y;
      struct datum *DT;

      if (MapWind0)
       {
       SetAPen(MapWind0->RPort, 1);
       DT = MD_Win->TC->nextdat;
       while (DT)
        {
        X = Lon_X_Convert(DT->values[0]);
        Y = Lat_Y_Convert(DT->values[1]);
        WritePixel(MapWind0->RPort, X, Y);
        DT = DT->nextdat;
	} /* while */
       } /* if */
      Datum_Del(MD_Win->TC);
      if ((MD_Win->TC = Datum_New()) == NULL)
       {
       Close_MD_Window();
       break;
       } /* if */
      MD_Win->ControlPts = 0;
      MD_Win->CurDat = MD_Win->TC;
      set(MD_Win->Text, MUIA_Text_Contents, "0");
      break;
      } /*  */
     } /* switch */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    i = WCS_ID - ID_MD_MODE(0);
    get(MD_Win->ModeButton[i], MUIA_Selected, &data);
    if (data)
     {
     nnset(MD_Win->ModeButton[MD_Win->PMode], MUIA_Selected, FALSE);
     MD_Win->PMode = i;
     }
    else
     {
     MD_Win->PMode = 0;
     nnset(MD_Win->ModeButton[0], MUIA_Selected, TRUE);
     }
    break;
    } /* BUTTONS2 */

   case GP_BUTTONS3:
    {
    get(MD_Win->ReverseButton, MUIA_Selected, &data);
    MD_Win->NoReversal = data;
    break;
    } /* BUTTONS3 */

   case GP_STRING1:
    {
    i = WCS_ID - ID_MD_INTSTR(0);
    get(MD_Win->IntStr[i], MUIA_String_Integer, &data);
    switch (i)
     {
     case 0:
      {
      MD_Win->MaxEl = data;
      data = 10130.0 - (((double)MD_Win->CurEl - MD_Win->MinEl) / ((double)MD_Win->MaxEl - MD_Win->MinEl)
	 * 10130.0);
      nnset(MD_Win->Prop, MUIA_Prop_First, data);
      break;
      }
     case 1:
      {
      MD_Win->CurEl = data;
      data = 10130.0 - (((double)data - MD_Win->MinEl) / ((double)MD_Win->MaxEl - MD_Win->MinEl)
	 * 10130.0);
      nnset(MD_Win->Prop, MUIA_Prop_First, data);
      break;
      }
     case 2:
      {
      MD_Win->MinEl = data;
      data = 10130.0 - (((double)MD_Win->CurEl - MD_Win->MinEl) / ((double)MD_Win->MaxEl - MD_Win->MinEl)
	 * 10130.0);
      nnset(MD_Win->Prop, MUIA_Prop_First, data);
      break;
      }
     }
    break;
    } /* INTSTRING */

   case GP_STRING2:
    {
    char *floatdata;
    double value;

    i = WCS_ID - ID_MD_FLOATSTR(0);
    get(MD_Win->FloatStr[i], MUIA_String_Contents, &floatdata);
    value = atof(floatdata);
    switch (i)
     {
     case 0:
      {
      MD_Win->MinDist = value;
      break;
      }
     case 1:
      {
      MD_Win->StdDev = value;
      break;
      }
     case 2:
      {
      MD_Win->NonLin = value;
      break;
      }
     } /* switch */
    break;
    } /* FLOATSTRING */

   case GP_ARROW1:
    {
    i = WCS_ID - ID_MD_ARROWLEFT(0);

    switch (i)
     {
     case 0:
      {
      MD_Win->MinDist -= 1.0;
      if (MD_Win->MinDist < 0.0)
       MD_Win->MinDist = 0.0;
      setfloat(MD_Win->FloatStr[0], MD_Win->MinDist);
      break;
      }
     case 1:
      {
      MD_Win->StdDev -= 1.0;
      if (MD_Win->StdDev < 0.0)
       MD_Win->StdDev = 0.0;
      setfloat(MD_Win->FloatStr[1], MD_Win->StdDev);
      break;
      }
     case 2:
      {
      MD_Win->NonLin -= .1;
      if (MD_Win->NonLin < 0.0)
       MD_Win->NonLin = 0.0;
      setfloat(MD_Win->FloatStr[2], MD_Win->NonLin);
      break;
      }

     } /* switch */
    break;
    } /* ARROWLEFT */

   case GP_ARROW2:
    {
    i = WCS_ID - ID_MD_ARROWRIGHT(0);

    switch (i)
     {
     case 0:
      {
      MD_Win->MinDist += 1.0;
      setfloat(MD_Win->FloatStr[0], MD_Win->MinDist);
      break;
      }
     case 1:
      {
      MD_Win->StdDev += 1.0;
      setfloat(MD_Win->FloatStr[1], MD_Win->StdDev);
      break;
      }
     case 2:
      {
      MD_Win->NonLin += .1;
      setfloat(MD_Win->FloatStr[2], MD_Win->NonLin);
      break;
      }
     } /* switch */
    break;
    } /* ARROWRIGHT */

   case GP_CYCLE1:
    {
    i = WCS_ID - ID_MD_CYCLE(0);
    get(MD_Win->Cycle[i], MUIA_Cycle_Active, &data);
    switch (i)
     {
     case 0:
      {
      MD_Win->ElSource = data;
      break;
      } /* elev source */
     case 1:
      {
      MD_Win->Units = data;
      switch (data)
       {
       case 0:
        {
        MD_Win->UnitScale = ELSCALE_METERS / ELSCALE_KILOM;
        break;
	} /* km */
       case 1:
        {
        MD_Win->UnitScale = 1.0;
        break;
	} /* meters */
       case 2:
        {
        MD_Win->UnitScale = ELSCALE_METERS / ELSCALE_CENTIM;
        break;
	} /* cm */
       case 3:
        {
        MD_Win->UnitScale = ELSCALE_METERS / ELSCALE_MILES;
        break;
	} /* miles */
       case 4:
        {
        MD_Win->UnitScale = ELSCALE_METERS / ELSCALE_FEET;
        break;
	} /* feet */
       case 5:
        {
        MD_Win->UnitScale = ELSCALE_METERS / ELSCALE_INCHES;
        break;
	} /* inches */
       } /* switch */
      break;
      } /* elev units */
     case 2:
      {
      MD_Win->Displace = data;
      break;
      } /* displace */
     case 3:
      {
      MD_Win->DMode = data;
      break;
      } /* draw mode */
     case 4:
      {
      MD_Win->FileType = data;
      break;
      } /* import file type */
     } /* switch */
    break;
    } /* CYCLE */

   case GP_PROP1:
    {
    get(MD_Win->Prop, MUIA_Prop_First, &data);
    data = (1.0 - ((double)data / 10130.0)) * (MD_Win->MaxEl - MD_Win->MinEl) + MD_Win->MinEl;
    set(MD_Win->IntStr[1], MUIA_String_Integer, data);
    MD_Win->CurEl = data;
    break;
    } /* PROP */
   } /* switch */

} /* Handle_DEM_Window() */

/************************************************************************/

STATIC_FCN void MD_Window_Init(void) // used locally only -> static, AF 20.7.2021
{

 MD_Win->MaxEl = 4000;
 MD_Win->MinEl = 0;
 MD_Win->CurEl = 2000;
 MD_Win->Units = 1;
 MD_Win->DMode = 0;
 MD_Win->PMode = 0;
 MD_Win->ElSource = 0;
 MD_Win->ControlPts = 0;
 MD_Win->Displace = 0;
 MD_Win->FileType = 0;
 MD_Win->NoReversal = 0;
 MD_Win->UnitScale = 1.0;
 MD_Win->StdDev = 10.0;
 MD_Win->MinDist = 5.0;
 MD_Win->NonLin = .5;
 strcpy(MD_Win->DirIn, dirname);
 strcpy(MD_Win->DirOut, dirname);

} /* MD_Window_Init() */

/***********************************************************************/

STATIC_FCN void Make_GR_Window(void) // used locally only -> static, AF 20.7.2021
{
long open;

 if (GR_Win)
  {
  DoMethod(GR_Win->NNGridWin, MUIM_Window_ToFront);
  set(GR_Win->NNGridWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((GR_Win = (struct NNGridWindow *)
	get_Memory(sizeof (struct NNGridWindow), MEMF_CLEAR)) == NULL)
   return;

  Set_Param_Menu(10);

     GR_Win->NNGridWin = WindowObject,
      MUIA_Window_Title		, "DEM Builder",
      MUIA_Window_ID		, 'NNGR',
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, Label("\33c\0334Elevation Model Grid"),
	Child, HGroup,
	  Child, VGroup,
	    Child, Label("\33c\0334Options"),
	    Child, HGroup,
	      Child, GR_Win->Check[0] = CheckMark(1),
	      Child, Label1("Gradients  "),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, GR_Win->Check[1] = CheckMark(0),
	      Child, Label1("Non-Neg    "),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, GR_Win->Check[2] = CheckMark(0),
	      Child, Label1("Choropleth "),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, GR_Win->Check[3] = CheckMark(0),
	      Child, Label1("Density    "),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, GR_Win->Check[4] = CheckMark(1),
	      Child, Label1("Extrapolate"),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, GR_Win->Check[5] = CheckMark(0),
	      Child, Label1("South Hemi."),
	      End, /* HGroup */
	    End, /* VGroup */
	  Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,
	  Child, VGroup,
	    Child, Label("\33c\0334Boundaries"),
	    Child, ColGroup(4),
	      Child, Label2("North"),
	      Child, GR_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "-.0123456789", End,

	      Child, Label2("South"),
	      Child, GR_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "-.0123456789", End,

	      Child, Label2("East"),
	      Child, GR_Win->FloatStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "-.0123456789", End,

	      Child, Label2("West"),
	      Child, GR_Win->FloatStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "0123456",
			MUIA_String_Accept, "-.0123456789", End, 
	      End, /* ColGroup */
	    Child, Label("\33c\0334Cell Overlap"),
	    Child, ColGroup(4),
	      Child, Label2("Horiz"),
	      Child, GR_Win->FloatStr[4] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "-.0123456789", End,

	      Child, Label2("Vert"),
	      Child, GR_Win->FloatStr[5] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "-.0123456789", End,
	      End, /* ColGroup */
	    Child, Label("\33c\0334Surface Tautness"),
	    Child, ColGroup(4),
	      Child, Label2("1"),
	      Child, GR_Win->FloatStr[6] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "-.0123456789", End,

	      Child, Label2("2"),
	      Child, GR_Win->FloatStr[7] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "-.0123456789", End,
	      End, /* ColGroup */
	    End, /* VGroup */
	  End, /* HGroup */
	Child, VGroup,
	  Child, Label("\33c\0334Scale"),
	  Child, HGroup,
	    Child, Label2("X"),
	    Child, GR_Win->FloatStr[9] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    Child, Label2("Y"),
	    Child, GR_Win->FloatStr[10] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    Child, Label2("Z"),
	    Child, GR_Win->FloatStr[11] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    End, /* HGroup */
	  Child, Label("\33c\0334Output"),
	  Child, HGroup,
	    Child, Label2("Cols"),
	    Child, GR_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Rows"),
	    Child, GR_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Null"),
	    Child, GR_Win->FloatStr[8] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    End, /* HGroup */

	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, Label("\33c\0334Optional Noise Map"),
	  Child, HGroup,
	    Child, Label2("Seed"),
	    Child, GR_Win->IntStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Delta"),
	    Child, GR_Win->FloatStr[12] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    Child, Label2("Fract"),
	    Child, GR_Win->FloatStr[13] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, ".0123456789", End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2("Offset X"),
	    Child, GR_Win->IntStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label2("Y"),
	    Child, GR_Win->IntStr[4] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	    Child, Label1("Scope"),
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, GR_Win->Text = TextObject, TextFrame, End,
              Child, GR_Win->Arrow[0] = ImageButton(MUII_ArrowLeft),
              Child, GR_Win->Arrow[1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    End, /* HGroup */
	  Child, GR_Win->ApplyButton = KeyButtonObject('a'),
			MUIA_InputMode, MUIV_InputMode_Toggle,
			MUIA_Text_Contents, "\33cApply Noise Map", End, 
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, HGroup, MUIA_Group_HorizSpacing, 0,
	    Child, Label2("Output DEM Name "),
	    Child, GR_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890123456789012345", End,
	    End, /* HGroup */
	  End, /* VGroup */
	Child, HGroup, MUIA_Group_HorizSpacing, 0,
	  Child, GR_Win->Button[0] = KeyButtonFunc('g', "\33cGrid"),
	  Child, GR_Win->Button[1] = KeyButtonFunc('n', "\33cNoise"),
	  Child, GR_Win->Button[2] = KeyButtonFunc('d', "\33cDraw"),
	  Child, GR_Win->Button[3] = KeyButtonFunc('s', "\33cSave"),
	  End, /* HGroup */
	End, /* VGroup */
      End; /* WindowObject */

  if (! GR_Win->NNGridWin)
   {
   Close_GR_Window();
   User_Message((CONST_STRPTR)"Map View: DEM Gridder", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, GR_Win->NNGridWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* set string values */
 GR_Window_Init();

/* ReturnIDs */
  DoMethod(GR_Win->NNGridWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_GR_CLOSE);  

  MUI_DoNotiPresFal(app, GR_Win->Button[0], ID_GR_MAKEGRID,
	GR_Win->Button[1], ID_GR_MAKENOISE,
	GR_Win->Button[2], ID_GR_DRAWDEM,
	GR_Win->Button[3], ID_GR_SAVEDEM,
	NULL);
  MUI_DoNotiPresFal(app, GR_Win->Arrow[0], ID_GR_ARROW(0),
  	GR_Win->Arrow[1], ID_GR_ARROW(1), NULL);

/* set tab cycle chain */
  DoMethod(GR_Win->NNGridWin, MUIM_Window_SetCycleChain,
   GR_Win->Check[0],
   GR_Win->Check[1],
   GR_Win->Check[2],
   GR_Win->Check[3],
   GR_Win->Check[4],
   GR_Win->Check[5],
   GR_Win->FloatStr[0],
   GR_Win->FloatStr[1],
   GR_Win->FloatStr[2],
   GR_Win->FloatStr[3],
   GR_Win->FloatStr[4],
   GR_Win->FloatStr[5],
   GR_Win->FloatStr[6],
   GR_Win->FloatStr[7],
   GR_Win->FloatStr[9],
   GR_Win->FloatStr[10],
   GR_Win->FloatStr[11],
   GR_Win->IntStr[0],
   GR_Win->IntStr[1],
   GR_Win->FloatStr[8],
   GR_Win->IntStr[2],
   GR_Win->FloatStr[12],
   GR_Win->FloatStr[13],
   GR_Win->IntStr[3],
   GR_Win->IntStr[4],
   GR_Win->ApplyButton,
   GR_Win->Str[0],
   GR_Win->Button[0],
   GR_Win->Button[1],
   GR_Win->Button[2],
   GR_Win->Button[3],
   NULL);

/* set return cycle chain */
  DoMethod(GR_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[1]);
  DoMethod(GR_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[2]);
  DoMethod(GR_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[3]);
  DoMethod(GR_Win->FloatStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[4]);
  DoMethod(GR_Win->FloatStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[5]);
  DoMethod(GR_Win->FloatStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[6]);
  DoMethod(GR_Win->FloatStr[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[7]);
  DoMethod(GR_Win->FloatStr[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[9]);
  DoMethod(GR_Win->FloatStr[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[10]);
  DoMethod(GR_Win->FloatStr[10], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[11]);
  DoMethod(GR_Win->FloatStr[11], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->IntStr[0]);
  DoMethod(GR_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->IntStr[1]);
  DoMethod(GR_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[8]);
  DoMethod(GR_Win->FloatStr[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->IntStr[2]);
  DoMethod(GR_Win->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[12]);
  DoMethod(GR_Win->FloatStr[12], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[13]);
  DoMethod(GR_Win->FloatStr[13], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->IntStr[3]);
  DoMethod(GR_Win->IntStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->IntStr[4]);
  DoMethod(GR_Win->IntStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->Str[0]);
  DoMethod(GR_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   GR_Win->NNGridWin, 3, MUIM_Set, MUIA_Window_ActiveObject, GR_Win->FloatStr[0]);

  set(GR_Win->ApplyButton, MUIA_Disabled, TRUE);
  set(GR_Win->Button[1], MUIA_Disabled, TRUE);
  set(GR_Win->Button[2], MUIA_Disabled, TRUE);
  set(GR_Win->Button[3], MUIA_Disabled, TRUE);

/* Open window */
  set(GR_Win->NNGridWin, MUIA_Window_Open, TRUE);
  get(GR_Win->NNGridWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_GR_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

} /* Make_GR_Window() */

/**********************************************************************/

void Close_GR_Window(void)
{

 if (GR_Win)
  {
  if (GR_Win->NoiseMap)
   free_Memory(GR_Win->NoiseMap, GR_Win->NoiseSize);
  if (GR_Win->NNG.Grid)
   free_Memory(GR_Win->NNG.Grid, GR_Win->NNG.GridSize);
  if (GR_Win->NNGridWin)
   {
   set(GR_Win->NNGridWin, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, GR_Win->NNGridWin);
   MUI_DisposeObject(GR_Win->NNGridWin);
   } /* if */
  free_Memory(GR_Win, sizeof (struct NNGridWindow));
  GR_Win = NULL;
  } /* if */

} /* Close_GR_Window() */

/***********************************************************************/

void Handle_GR_Window(ULONG WCS_ID)
{
short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_GR_Window();
   return;
   } /* Open Render Settings Editor Window */

  if (! GR_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_GR_CLOSE:
      {
      Close_GR_Window();
      break;
      } /* Close Window */
     case ID_GR_MAKEGRID:
      {
      if (nngridr())
       {
       set(GR_Win->Button[1], MUIA_Disabled, FALSE);
       set(GR_Win->Button[2], MUIA_Disabled, FALSE);
       set(GR_Win->Button[3], MUIA_Disabled, FALSE);
       set(GR_Win->ApplyButton, MUIA_Disabled, FALSE);
       } /* if */
      else
       {
       set(GR_Win->Button[1], MUIA_Disabled, TRUE);
       set(GR_Win->Button[2], MUIA_Disabled, TRUE);
       set(GR_Win->Button[3], MUIA_Disabled, TRUE);
       set(GR_Win->ApplyButton, MUIA_Disabled, TRUE);
       } /* if */
      break;
      } /*  */
     case ID_GR_MAKENOISE:
      {
      Noise_Init();
      break;
      } /*  */
     case ID_GR_DRAWDEM:
      {
      Grid_Draw(&GR_Win->NNG);
      break;
      } /*  */
     case ID_GR_SAVEDEM:
      {
      Grid_Save(&GR_Win->NNG, MD_Win->Units);
      break;
      } /*  */
     } /* switch */
    break;
    } /* BUTTONS1 */

   case GP_ARROW1:
    {
    char *data;
    long val;

    i = WCS_ID - ID_GR_ARROW(0);
    get(GR_Win->Text, MUIA_Text_Contents, &data);
    val = atoi(data);
    if (i == 0)
     {
     if (val > 1)
      val --;
     } /* if */
    else
     {
     val ++;
     } /* else */
    sprintf(str, "%ld", val);
    set(GR_Win->Text, MUIA_Text_Contents, str);
    break;
    } /* arrow */

   } /* switch */

} /* Handle_DEM_Window() */

/************************************************************************/

STATIC_FCN void GR_Window_Init(void) // used locally only -> static, AF 20.7.2021
{
long LowRow, LowCol, HighRow, HighCol, rows, cols;
double North = -10000.0, South = 10000.0, East = 10000.0, West = -10000.0;
struct datum *DT;

 if (! MD_Win || ! GR_Win || ! MP)
  return;

 DT = MD_Win->TC->nextdat;
 while (DT)
  {
  if (DT->values[1] < South)
   South = DT->values[1];
  if (DT->values[1] > North)
   North = DT->values[1];
  if (DT->values[0] < East)
   East = DT->values[0];
  if (DT->values[0] > West)
   West = DT->values[0];
  DT = DT->nextdat;
  } /* while */

 setfloat(GR_Win->FloatStr[0], North);
 setfloat(GR_Win->FloatStr[1], South);
 setfloat(GR_Win->FloatStr[2], East);
 setfloat(GR_Win->FloatStr[3], West);

 LowCol = Lon_X_Convert(West);
 HighCol = Lon_X_Convert(East);
 LowRow = Lat_Y_Convert(North);
 HighRow = Lat_Y_Convert(South);

 rows = HighRow - LowRow + 1;
 cols = HighCol - LowCol + 1;

 set(GR_Win->IntStr[0], MUIA_String_Integer, cols);
 set(GR_Win->IntStr[1], MUIA_String_Integer, rows);
 set(GR_Win->IntStr[2], MUIA_String_Integer, 1000);
 set(GR_Win->IntStr[3], MUIA_String_Integer, 0);
 set(GR_Win->IntStr[4], MUIA_String_Integer, 0);

 setfloat(GR_Win->FloatStr[4], 1.0);	/* horlap */
 setfloat(GR_Win->FloatStr[5], 1.0);	/* vertlap */
 setfloat(GR_Win->FloatStr[6], 1.5);	/* taut 1 */
 setfloat(GR_Win->FloatStr[7], 7.0);	/* taut 2 */
 setfloat(GR_Win->FloatStr[9], 1.0);	/* magx */
 setfloat(GR_Win->FloatStr[10], 1.0);	/* magy */
 setfloat(GR_Win->FloatStr[11], .01);	/* magz */
 setfloat(GR_Win->FloatStr[8], 0.0);	/* nuldat */
 setfloat(GR_Win->FloatStr[12], 10.0);	/* delta */
 setfloat(GR_Win->FloatStr[13], .5);	/* fractal dimension */

 set(GR_Win->Str[0], MUIA_String_Contents, "NewDEM");

 set(GR_Win->Text, MUIA_Text_Contents, "3");

} /* GR_Window_Init() */

/************************************************************************/

STATIC_FCN short DEMBuild_Import(short FileType) // used locally only -> static, AF 20.7.2021
{
short success = 0;

 switch (FileType)
  {
  case 0:
   {
   success = Contour_Import();
   break;
   }
  case 1:
   {
   success = XYZ_Import(0);	/* 0 signifies lat/lon coords */
   break;
   }
  case 2:
   {
   success = XYZ_Import(1);	/* 1 signifies UTM coords */
   break;
   }
  case 3:
   {
   success = DXF_Import(0);	/* 0 signifies lat/lon coords */
   break;
   }
  case 4:
   {
   success = DXF_Import(1);	/* 1 signifies UTM coords */
   break;
   }
  } /* switch */

 return (success);

} /* DEMBuild_Import() */

/************************************************************************/

STATIC_FCN short Contour_Import(void) // used locally only -> static, AF 20.7.2021
{
char *LastDir = NULL;
short ElSource, ActiveItem, j, *ElevPtr, success = 1, Warn = 0;
long state;
float El;

 if (! DE_Win)
  {
  Make_DE_Window();
  if (DE_Win)
   User_Message((CONST_STRPTR)"Map View: Build DEM",
		   (CONST_STRPTR)"Select contour objects to import and reselect \"Import\" when done.",
		   (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  else
   User_Message((CONST_STRPTR)"Map View: Export Contours",
		   (CONST_STRPTR)"Can't open Database Editor window!\nOperation terminated.",(CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (0);
  } /* if Database Editor not open */


 ElSource = User_Message("Map View: Export Contours",
	"Extract elevation values from Object Names, Label fields\
 or use the values embedded in the Objects themselves?",
	"Name|Label|Embedded", "nle");

 get(DE_Win->LS_List, MUIA_List_Active, &ActiveItem);
 for (j=0; j<NoOfObjects; j++)
  {
  DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_Ask, &state);
  if (state || j == ActiveItem)
   {
   if (! DBase[j].Lat || ! DBase[j].Lon || ! DBase[j].Elev)
    {
    if (Load_Object(j, &LastDir) > 0)
     {
     Warn = 1;
     continue;
     } /* if load failed */
    } /* if not loaded */
    
   if (ElSource)
    {
    ElevPtr = NULL;
    if (ElSource == 1)
     El = atof(DBase[j].Name);
    else
     El = atof(DBase[j].Label);
    } /* if derive elevation */
   else
    ElevPtr = DBase[j].Elev;
    
   if (! Object_ImportXYZ(DBase[j].Points, DBase[j].Lat,
		DBase[j].Lon, ElevPtr, El, MD_Win->CurDat))
    {
    User_Message((CONST_STRPTR)"Map View: Build DEM",
    		(CONST_STRPTR)"Error importing contour data!\nOperation terminated.",(CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    success = 0;
    break;
    } /* if error */
   } /* if selected or active */
  } /* for j=0... */

 if (Warn)
  User_Message("Map View: Build DEM",
		  (CONST_STRPTR)"At least one Object failed to load and could not be imported.",(CONST_STRPTR)"OK", (CONST_STRPTR)"o");

 return (success);

} /* Contour_Import() */

/***********************************************************************/

STATIC_FCN short Object_ImportXYZ(short Points, double *Lat, double *Lon,
	short *ElevPtr, float El, struct datum *DT) // used locally only -> static, AF 20.7.2021
{
short pt, success = 1;
long X, Y;

 if (MapWind0)
  SetAPen(MapWind0->RPort, 7);
 if (DT)
  {
  for (pt=1; pt<=Points; pt++)
   {
   if ((DT->nextdat = Datum_New()))
    {
    DT = DT->nextdat;
    if (ElevPtr)
     El = ElevPtr[pt];
    DT->values[0] = Lon[pt];
    DT->values[1] = Lat[pt];
    DT->values[2] = El;
    if (MD_Win)
     {
     MD_Win->CurDat = DT;
     MD_Win->ControlPts ++;
     } /* if */
    if (MapWind0)
     {
     X = Lon_X_Convert(DT->values[0]);
     Y = Lat_Y_Convert(DT->values[1]);
     WritePixel(MapWind0->RPort, X, Y);
     } /* if */
    } /* if new datum created */
   else
    {
    success = 0;
    User_Message((CONST_STRPTR)"Map View: Import Contours",
    		(CONST_STRPTR)"Out of memory!\nOperation terminated.",(CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* else */
   } /* for pt=1... */
  } /* if */

 return (success);

} /* Object_ImportXYZ() */

/**********************************************************************/

STATIC_FCN short XYZ_Import(short CoordSys) // used locally only -> static, AF 20.7.2021
{
char filename[256], XYZPath[256], XYZFile[32];
short success = 1, Zone;
long X, Y;
double dX, dY, dZ;
FILE *fXYZ;
struct datum *DT;
struct UTMLatLonCoords UTM;

 if (MD_Win)
  {
  strcpy(XYZPath, MD_Win->DirIn);
  strcpy(XYZFile, MD_Win->FileIn);
  } /* if */
 else
  {
  strcpy(XYZPath, dirname);
  XYZFile[0] = 0;
  } /* else */
 if (! getfilename(0, "XYZ Path/File", XYZPath, XYZFile))
  return (0);
 if (XYZFile[0] == 0)
  {
  User_Message((CONST_STRPTR)"Map View: Build DEM",
		  (CONST_STRPTR)"You did not select a file to import!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (0);
  } /* if */
 strmfp(filename, XYZPath, XYZFile);
 if (MD_Win)
  {
  strcpy(MD_Win->DirIn, XYZPath);
  strcpy(MD_Win->FileIn, XYZFile);
  } /* if */

 if (CoordSys == 1)
  {
  if (GetInputString("Enter the UTM zone number (0-60) for the data you are about to import.",
	"abcdefghijklmnopqrstuvwxyz.:;*/?`#%", str))
   {
   Zone = atoi(str);
   if (Zone < 0 || Zone > 60)
    {
    User_Message((CONST_STRPTR)"Map View: Build DEM",
    		(CONST_STRPTR)"UTM zones may be from 0 to 60! The selected zone is out of range.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    return (0);
    } /* if zone out of range */
   UTMLatLonCoords_Init(&UTM, Zone);
   } /* if */
  else
   return (0);
  } /* if UTM */

 DT = MD_Win->CurDat;

 if (MapWind0)
  SetAPen(MapWind0->RPort, 7);
 if ((fXYZ = fopen(filename, "r")) != NULL)
  {
  while (fscanf(fXYZ, "%le%le%le", &dX, &dY, &dZ) != EOF)
   {
   if ((DT->nextdat = Datum_New()))
    {
    DT = DT->nextdat;
    if (CoordSys == 1)
     {
     UTM.North = dY;
     UTM.East = dX;
     UTM_LatLon(&UTM);
     DT->values[0] = UTM.Lon;
     DT->values[1] = UTM.Lat;
     } /* if UTM */
    else
     {
     DT->values[0] = dX;
     DT->values[1] = dY;
     } /* if */
    DT->values[2] = dZ;
    if (MD_Win)
     {
     MD_Win->CurDat = DT;
     MD_Win->ControlPts ++;
     } /* if */
    if (MapWind0)
     {
     X = Lon_X_Convert(DT->values[0]);
     Y = Lat_Y_Convert(DT->values[1]);
     WritePixel(MapWind0->RPort, X, Y);
     } /* if */
    } /* if new datum created */
   else
    {
    success = 0;
    User_Message((CONST_STRPTR)"Map View: Build DEM",
    		(CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* else out of memory */
   } /* while */
  fclose(fXYZ);
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, XYZFile);
  User_Message((CONST_STRPTR)"Map View: Build DEM",
		  (CONST_STRPTR)"Error opening XYZ file to import!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  success = 0;
  } /* else open file failed */

 return (success);

} /* XYZ_Import() */

/**********************************************************************/

STATIC_FCN short DXF_Import(short CoordSys) // used locally only -> static, AF 20.7.2021
{


return (1);

} /* DXF_Import() */

/*********************************************************************/

STATIC_FCN short XYZ_Export(struct datum *DT) // used locally only -> static, AF 20.7.2021
{
char XYZPath[256], XYZFile[32], filename[256];
short success = 1;
FILE *fXYZ;

 if (MD_Win)
  {
  strcpy(XYZPath, MD_Win->DirOut);
  strcpy(XYZFile, MD_Win->FileOut);
  } /* if */
 else
  {
  strcpy(XYZPath, dirname);
  XYZFile[0] = 0;
  } /* else */
 if (! getfilename(1, "XYZ Path/File", XYZPath, XYZFile))
  return (0);
 if (! XYZFile[0])
  {
  User_Message((CONST_STRPTR)"Map View: XYZ Export",
		  (CONST_STRPTR)"You must specify an output file name!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (0);
  } /* if no file name */
 strmfp(filename, XYZPath, XYZFile);
 if (MD_Win)
  {
  strcpy(MD_Win->DirOut, XYZPath);
  strcpy(MD_Win->FileOut, XYZFile);
  } /* if */

 if ((fXYZ = fopen(filename, "w")))
  {
  while(DT)
   {
   if ((fprintf(fXYZ, "%13.8f  %13.8f  %13.8f\n", DT->values[0],
	DT->values[1], DT->values[2])) < 0)
    {
    User_Message((CONST_STRPTR)"Map View: XYZ Export",
    		(CONST_STRPTR)"Error writing to XYZ file! Partial file written.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    Log(ERR_WRITE_FAIL, XYZFile);
    success = 0;
    break;
    } /* if error */
   DT = DT->nextdat;
   } /* while */
  fclose(fXYZ);
  } /* if file opened */
 else
  {
  User_Message((CONST_STRPTR)"Map View: XYZ Export",
		  (CONST_STRPTR)"Unable to open XYZ file for export!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_OPEN_FAIL, XYZFile);
  success = 0;
  } /* else */

 return (success);

} /* XYZ_Export() */

/*********************************************************************/

STATIC_FCN short Noise_Init(void) // used locally only -> static, AF 20.7.2021
{
short success = 0;
char *floatdata;
long data, zip, i, j;

 if (GR_Win->NNG.x_nodes <= 0 || GR_Win->NNG.y_nodes <= 0)
  return (0);

 get(GR_Win->FloatStr[12], MUIA_String_Contents, &floatdata);
 GR_Win->Delta = atof(floatdata);
 get(GR_Win->FloatStr[13], MUIA_String_Contents, &floatdata);
 GR_Win->H = atof(floatdata);
 get(GR_Win->IntStr[2], MUIA_String_Integer, &data);
 GR_Win->RandSeed = data;
 get(GR_Win->IntStr[3], MUIA_String_Integer, &data);
 GR_Win->OffX = data;
 get(GR_Win->IntStr[4], MUIA_String_Integer, &data);
 GR_Win->OffY = data;
 get(GR_Win->Text, MUIA_Text_Contents, &floatdata);
 GR_Win->Scope = atoi(floatdata);

 GR_Win->XPow = GR_Win->YPow = 0.0;
 while (pow(2.0, GR_Win->XPow) < GR_Win->NNG.x_nodes)
  GR_Win->XPow += 1.0;
 while (pow(2.0, GR_Win->YPow) < GR_Win->NNG.y_nodes)
  GR_Win->YPow += 1.0;
 GR_Win->NCols = pow(2.0, GR_Win->XPow);
 GR_Win->NRows = pow(2.0, GR_Win->YPow);
 GR_Win->NCols ++;
 GR_Win->NRows ++;

/* free old map */
 if (GR_Win->NoiseMap)
  free_Memory(GR_Win->NoiseMap, GR_Win->NoiseSize);

/* allocate noise map */
 GR_Win->NoiseSize = GR_Win->NCols * GR_Win->NRows * sizeof (float); 
 if ((GR_Win->NoiseMap = (float *)get_Memory(GR_Win->NoiseSize, MEMF_ANY)))
  {
/* initialize noise map and define scope of randomization */
  zip = 0;
  for (i=0; i<GR_Win->NRows; i++)
   {
   for (j=0; j<GR_Win->NCols; j++, zip++)
    GR_Win->NoiseMap[zip] = 0.0;
   } /* for i=0... */
  if (GR_Win->Scope <= 0)
   GR_Win->Scope = 1;
  data = min(GR_Win->XPow, GR_Win->YPow);
  GR_Win->Scope = min(GR_Win->Scope, data);

  Raster_Fract(GR_Win->NoiseMap, GR_Win->NCols, GR_Win->NRows, GR_Win->RandSeed,
	GR_Win->Delta, GR_Win->H, GR_Win->Scope);
  } /* if noise map allocated */

 return (success);
  
} /* Noise_Init() */

/***********************************************************************/

STATIC_FCN short Grid_Draw(struct NNGrid *NNG) // used locally only -> static, AF 20.7.2021
{
short col;
float *NoisePtr;
double MaxAmp, MinAmp, RangeAmp, DataRow, DataCol, LatStep, LonStep;
long x, y, k, Low_X, Low_Y, High_X, High_Y, zip, RowZip, Row, Col, UseNoise;
struct BusyWindow *BWGR;
struct clipbounds cb;

 if (! GR_Win)
  return (0);

/* display map */

 if (GR_Win->NoiseMap)
  {
  get(GR_Win->ApplyButton, MUIA_Selected, &UseNoise);

  if (UseNoise)
   {
   get(GR_Win->IntStr[3], MUIA_String_Integer, &GR_Win->OffX);
   get(GR_Win->IntStr[4], MUIA_String_Integer, &GR_Win->OffY);

   GR_Win->OffX = min(GR_Win->OffX, GR_Win->NCols - NNG->x_nodes);
   GR_Win->OffY = min(GR_Win->OffY, GR_Win->NRows - NNG->y_nodes);
   } /* if */
  } /* if */
 else
  UseNoise = 0;

 if (MapWind0)
  {
  setclipbounds(MapWind0, &cb);

  Low_X = Lon_X_Convert(-NNG->xstart);
  Low_Y = Lat_Y_Convert(NNG->yterm);
  High_X = Lon_X_Convert(-NNG->xterm);
  High_Y = Lat_Y_Convert(NNG->ystart);
  LatStep = ((double)(NNG->y_nodes - 1)) / ((double)(High_Y - Low_Y));
  LonStep = ((double)(NNG->x_nodes - 1)) / ((double)(High_X - Low_X));

/* find largest data amplitudes */

  zip = 0;
  MaxAmp = -100000.0;
  MinAmp = 100000.0;
  if (UseNoise)
   {
   for (y=0; y<NNG->y_nodes; y++)
    {
    NoisePtr = GR_Win->NoiseMap + (y + GR_Win->OffY) * GR_Win->NCols
	+ GR_Win->OffX;
    for (x=0; x<NNG->x_nodes; x++, zip++, NoisePtr++)
     {
     if (NNG->Grid[zip] + *NoisePtr > MaxAmp)
      MaxAmp = NNG->Grid[zip] + *NoisePtr;
     if (NNG->Grid[zip] + *NoisePtr < MinAmp)
      MinAmp = NNG->Grid[zip] + *NoisePtr;
     } /* for y=... */
    } /* for x=... */
   } /* if use noise */
  else
   {
   for (y=0; y<NNG->y_nodes; y++)
    {
    for (x=0; x<NNG->x_nodes; x++, zip++)
     {
     if (NNG->Grid[zip] > MaxAmp)
      MaxAmp = NNG->Grid[zip];
     if (NNG->Grid[zip] < MinAmp)
      MinAmp = NNG->Grid[zip];
     } /* for y=... */
    } /* for x=... */
   } /* else no noise */
  MaxAmp += 1.0;
  MinAmp -= 1.0;
  RangeAmp = MaxAmp - MinAmp;

/* plot color in Map View, brighter indicates higher amplitude */

  BWGR = BusyWin_New("Drawing...", High_Y - Low_Y + 1, 0, 'BWGR');

  DataRow = 0.0;
  for (y=Low_Y, k=0; y<=High_Y; y++, DataRow+=LatStep, k++)
   {
   if (y < cb.lowy)
    continue;
   if (y > cb.highy)
    break;
   Row = DataRow;
   RowZip = Row * NNG->x_nodes;
   DataCol = 0.0;
   if (UseNoise)
    NoisePtr = GR_Win->NoiseMap + (Row + GR_Win->OffY) * GR_Win->NCols;
   for (x=Low_X; x<=High_X; x++, DataCol+=LonStep)
    {
    if (x < cb.lowx)
     continue;
    if (x > cb.highx)
     break;
    Col = DataCol;
    zip = RowZip + Col;
    if (UseNoise)
     col = 15.99 - ((NNG->Grid[zip] + *(NoisePtr + Col + GR_Win->OffX) - MinAmp) / RangeAmp) * 7.99;
    else
     col = 15.99 - ((NNG->Grid[zip] - MinAmp) / RangeAmp) * 7.99;
    if (col < 8)
     col = 8;
    SetAPen(MapWind0->RPort, col);
    WritePixel(MapWind0->RPort, x, y);
    } /* for x=... */
   BusyWin_Update(BWGR, k + 1);
   } /* for y=... */

  if (BWGR) BusyWin_Del(BWGR);
  } /* if MapWind0 */

 return (1);

} /* Grid_Draw() */

/***********************************************************************/

STATIC_FCN short Grid_Save(struct NNGrid *NNG, short Units) // used locally only -> static, AF 19.7.2021
{
char *OutFile, ObjName[32];
short *SaveMap, *SavePtr;
long UseNoise;
float *GridPtr, *NoisePtr;
long x, y, SaveSize;
struct elmapheaderV101 Hdr;

 if (! GR_Win)
  return (0);

 if (GR_Win->NoiseMap)
  {
  get(GR_Win->ApplyButton, MUIA_Selected, &UseNoise);

  if (UseNoise)
   {
   get(GR_Win->IntStr[3], MUIA_String_Integer, &GR_Win->OffX);
   get(GR_Win->IntStr[4], MUIA_String_Integer, &GR_Win->OffY);

   GR_Win->OffX = min(GR_Win->OffX, GR_Win->NCols - NNG->x_nodes);
   GR_Win->OffY = min(GR_Win->OffY, GR_Win->NRows - NNG->y_nodes);
   } /* if */
  } /* if */
 else
  UseNoise = 0;

 get(GR_Win->Str[0], MUIA_String_Contents, &OutFile);
 strcpy(NNG->grd_file, OutFile);

 Hdr.rows = NNG->x_nodes - 1;
 Hdr.columns = NNG->y_nodes;

 Hdr.lolong = -NNG->xstart;
 Hdr.lolat = NNG->ystart;
 Hdr.steplat = (NNG->yterm - NNG->ystart) / (NNG->y_nodes - 1);
 Hdr.steplong = (NNG->xterm - NNG->xstart) / (NNG->x_nodes - 1);
 switch (Units)
  {
  case 0:
   {
   Hdr.elscale = ELSCALE_KILOM;
   break;
   } /* kilometers */
  case 1:
   {
   Hdr.elscale = ELSCALE_METERS;
   break;
   } /* meters */
  case 2:
   {
   Hdr.elscale = ELSCALE_CENTIM;
   break;
   } /* centimeters */
  case 3:
   {
   Hdr.elscale = ELSCALE_MILES;
   break;
   } /* miles */
  case 4:
   {
   Hdr.elscale = ELSCALE_FEET;
   break;
   } /* feet */
  case 5:
   {
   Hdr.elscale = ELSCALE_INCHES;
   break;
   } /* inches */
  } /* switch */

 SaveSize = NNG->GridSize / 2;
 if ((SaveMap = (short *)get_Memory(SaveSize, MEMF_ANY)) != NULL)
  {
  SavePtr = SaveMap;
  for (x=0; x<NNG->x_nodes; x++)
   {
   GridPtr = NNG->Grid + (NNG->y_nodes - 1) * NNG->x_nodes + x;
   if (UseNoise)
    NoisePtr = GR_Win->NoiseMap + (GR_Win->OffY + NNG->y_nodes - 1) * GR_Win->NCols
	+ GR_Win->OffX + x;
   for (y=NNG->y_nodes; y>0; y--, SavePtr++, GridPtr -= NNG->x_nodes)
    {
    if (UseNoise)
     {
     *SavePtr = *GridPtr + *NoisePtr;
     NoisePtr -= GR_Win->NCols;
     }
    else
     *SavePtr = *GridPtr;
    } /* for y=... */
   } /* for x=... */
  strcpy(ObjName, NNG->grd_file);
  ObjName[length[0]] = 0;
  while(strlen(ObjName) < length[0])
   strcat(ObjName, " ");
  DEMFile_Save(ObjName, &Hdr, SaveMap, SaveSize);

  free_Memory(SaveMap, SaveSize);
  } /* if memory OK */
 else
  return (0);

 return (1);

} /* Grid_Save() */
