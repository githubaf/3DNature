/* EdSetGUI.c (ne gisedsetgui.c 14 Jan 1994 CXH)
** World Construction Set GUI for Settings Editing module.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include <stdarg.h>


void Make_ES_Window(void)
{
 short i;
 long open;
 static const char *ES_PageCycle[] = {"\0334Render & Size", "\0334Image Save",
	"\0334Motion Paths & Vectors", "\0334Color Maps", "\0334Surfaces",
	"\0334Fractals", "\0334Ecosystems & Strata", "\0334Miscellaneous", "\0334Processing",
#ifdef ENABLE_SCALING
	"\0334Post-Process",
#endif /* ENABLE_SCALING */
	NULL};

 static const char *ES_Cycle_RGB[] = {"No RGB", "\338RGB", NULL};
 static const char *ES_Cycle_Screen[] = {"No Screen", "\338Screen", NULL};
 static const char *ES_Cycle_Data[] = {"No Data", "\338Data", NULL};
 static const char *ES_Cycle_SaveFormat[] = {"Sculpt RGB", "Raw intrlvd RGB", "\338IFF ILBM", NULL};
 static const char *ES_Cycle_Concat[] = {"No Conctatenate", "\338Concatenate", NULL};
 static const char *ES_Cycle_BankTurn[] = {"No Bank Turns", "\338Bank Turns", NULL};
 static const char *ES_Cycle_VecRender[] = {"\338Rndr To File", "\338Rndr To Bitmap", "No Vectors", NULL};
 static const char *ES_Cycle_VecHaze[] = {"No Haze Eff", "\338Haze Effect", NULL};
 static const char *ES_Cycle_FixFract[] = {"Variable Fractal Depth", "\338Constant Fractal Depth", "\338Fractal Depth Maps", NULL};
 static const char *ES_Cycle_ColorMap[] = {"No Color Maps", "\338Color Maps", NULL};
 static const char *ES_Cycle_BorderRand[] = {"No CMap Random Borders", "\338CMap Random Borders", NULL};
 static const char *ES_Cycle_CMapTrees[] = {"No CMap Textures", "\338CMap Textures", NULL};
 static const char *ES_Cycle_ColorMatch[] = {"No CMap Color Match", "\338CMap Color Match", NULL};
 static const char *ES_Cycle_Grid[] = {"No Surface Grid", "\338Surface Grid", NULL};
 static const char *ES_Cycle_HorizonFix[] = {"No Fixed Horizon", "\338Fixed Horizon", NULL};
 static const char *ES_Cycle_AltQ[] = {"No Alt Z Reference", "\338Alt Z Reference", NULL};
 static const char *ES_Cycle_CloudShadows[] = {"No Cloud Shadows", "\338Cloud Shadows", NULL};
 static const char *ES_Cycle_Background[] = {"No Background", "\338Background", NULL};
 static const char *ES_Cycle_ZBuffer[] = {"No Z Buffer", "\338Z Buffer", NULL};
 static const char *ES_Cycle_Blur[] = {"No Blur", "\338Blur", NULL};
 static const char *ES_Cycle_ZBufBlur[] = {"No Z Buffered Blur", "\338Z Buffered Blur", NULL};
#ifdef ENABLE_SCALING
 static const char *ES_Cycle_Scaling[] = {"No Scaling", "\338Scaling", NULL};
#endif /* ENABLE_SCALING */
 static const char *ES_Cycle_MapAsSfc[] = {"No Topos As Surfaces", "\338Topos As Surfaces", NULL};
 static const char *ES_Cycle_ExportZ[] = {"No Export Z Buffer", "\338Export Z Buffer", NULL};
 static const char *ES_Cycle_ZFormat[] = {"\338Z As Floating Pt IFF",
	 "Z As Gray Scale IFF", "\338Z As Floating Pt Array",
	 "Z As Gray Scale Array", NULL};
 static const char *ES_Cycle_FieldRender[] = {"No Field Rendering", "\338Field Rendering", NULL};
 static const char *ES_Cycle_GlobalGradients[] = {"No Global Gradients", "\338Global Gradients", NULL};
 static const char *ES_Cycle_FlattenEco[] = {"No Ecosystem Flattening", "\338Ecosystem Flattening", NULL};
 static const char *ES_Cycle_LookAhead[] = {"No Look Ahead", "\338Look Ahead", NULL};
 static const char *ES_Cycle_VelocDist[] = {"No Velocity Distribution", "\338Velocity Distribution", NULL};
 static const char *ES_Cycle_RenderTrees[] = {"No Trees or Textures", "\338Trees and Textures", NULL};
 static const char *ES_Cycle_HorizonMax[] = {"No Render Beyond Horizon", "\338Render Beyond Horizon", NULL};
 static const char *ES_Cycle_Luminous[] = {"No CMap Luminous Colors", "\338CMap Luminous Colors", NULL};
 static const char *ES_Cycle_RenderStyle[] = {"Normal Shading", "\338Polygon Smoothing", "\338Fractal Displacement", NULL};
 static const char *ES_Cycle_MasterCMap[] = {"Individual Color Maps", "\338Master Color Map", NULL};
 static const char *ES_Cycle_CMapOrient[] = {"Master CMap DEM Oriented", "\338Master CMap Image Oriented", NULL};
 static const char *ES_Cycle_FieldDomin[] = {"Field Dominance Normal", "\338Field Dominance Reverse", NULL};
 static const char *ES_Cycle_RealClouds[] = {"No 3D Clouds", "\3383D Clouds", NULL};
 static const char *ES_Cycle_Perturb[] = {"No Perturbance", "\338Perturbance", NULL};
 static const char *ES_Cycle_Waves[] = {"No Waves", "\338Waves", NULL};
 static const char *ES_Cycle_Reflections[] = {"No Reflections", "\338Reflections", NULL};
 static const char *ES_Cycle_DeformationMap[] = {"No Strata Deformation Map", "\338Strata Deformation Map", NULL};
 static const char *ES_Cycle_SurfaceCMaps[] = {"No Surface Color Maps", "\338Surface Color Maps", NULL};
 static const char *ES_Cycle_Sun[] = {"No Sun", "\338Sun", NULL};
 static const char *ES_Cycle_Moon[] = {"No Moon", "\338Moon", NULL};
 static const char *ES_Cycle_Tides[] = {"No Tides", "\338Tides", NULL};
 static const char *ES_Cycle_SunHalo[] = {"No Sun Halo", "\338Sun Halo", NULL};
 static const char *ES_Cycle_MoonHalo[] = {"No Moon Halo", "\338Moon Halo", NULL};
 
 if (ES_Win)
  {
  DoMethod(ES_Win->SettingsWin, MUIM_Window_ToFront);
  set(ES_Win->SettingsWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message((CONST_STRPTR)"Render Module",
		  (CONST_STRPTR)"You must first load or create a parameter file before opening the Render Module.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((ES_Win = (struct SettingsWindow *)
	get_Memory(sizeof (struct SettingsWindow), MEMF_CLEAR)) == NULL)
   return;

  Set_Param_Menu(3);

     ES_Win->SettingsWin = WindowObject,
      MUIA_Window_Title		, "Render Settings Editor",
      MUIA_Window_ID		, MakeID('E','D','S','T'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
        Child, ES_Win->Pages = RegisterGroup(ES_PageCycle),
/* Render & Size */
	  Child, VGroup, GroupFrame,
	    Child, RectangleObject, End,
	    Child, HGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Width "),
	        Child, ES_Win->IntStr[4] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->IntStrArrow[4][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[4][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Height "),
	        Child, ES_Win->IntStr[5] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
		 	MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->IntStrArrow[5][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[5][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */
	    Child, HGroup,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Aspct "),
	        Child, ES_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[0][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[0][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("V Oscn "),
	        Child, ES_Win->IntStr[6] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->IntStrArrow[6][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[6][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */
            Child, ES_Win->BT_ChangeScale = KeyButtonFunc('i', "\33cChange Image Size"), 

	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	    Child, ColGroup(2),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2(" Start "),
	        Child, ES_Win->IntStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[0][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[0][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("    End "),
	        Child, ES_Win->IntStr[22] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[22][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[22][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("  Step "),
	        Child, ES_Win->IntStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "-0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[2][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[2][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2(" Frames "),
	        Child, ES_Win->IntStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[1][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[1][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */

	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Segmnt "),
	        Child, ES_Win->IntStr[3] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[3][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[3][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("1st Seg "),
	        Child, ES_Win->IntStr[21] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[21][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[21][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* ColGroup */
	    Child, ES_Win->Cycle[0] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_RGB, End, 
	    Child, ES_Win->Cycle[1] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Screen, End, 
	    Child, ES_Win->Cycle[2] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Data, End, 
	    Child, ES_Win->Cycle[36] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_FieldRender, End, 
	    Child, ES_Win->Cycle[18] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_FieldDomin, End,

	    Child, RectangleObject, End,
	    End, /* VGroup Render & Size */

/* Image Save */
	  Child, VGroup, GroupFrame,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Save Path "),
	      Child, ES_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[0] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Save File "),
	      Child, ES_Win->Str[8] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Temp Path "),
	      Child, ES_Win->Str[7] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[5] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[4] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_SaveFormat, End, 
	    Child, ES_Win->Cycle[5] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Concat, End, 
	    Child, ES_Win->Cycle[34] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ExportZ, End, 
	    Child, ES_Win->Cycle[35] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ZFormat, End, 
	    End, /* VGroup Image Save */

/* Follow Paths & Vectors */
	  Child, VGroup, GroupFrame,
	    Child, RectangleObject, End,
	    Child, HGroup,
	      Child, ES_Win->Cycle[24] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_LookAhead, End,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, ES_Win->IntStr[18] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[18][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[18][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
              Child, Label2("Frames"),
	      End, /* HGroup */
	    Child, HGroup, 
	      Child, ES_Win->Cycle[8] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_BankTurn, End, 
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, ES_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[1][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[1][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
              Child, Label2("Banking"),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[32] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_VelocDist, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Ease In "),
	        Child, ES_Win->IntStr[19] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[19][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[19][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2(" Out "),
	        Child, ES_Win->IntStr[20] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "012345", End,
                Child, ES_Win->IntStrArrow[20][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[20][1] = ImageButton(MUII_ArrowRight),
		End, /* HGroup */
	      End, /* HGroup */
	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, ES_Win->Cycle[9] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_VecRender, End,
	      Child, ES_Win->Cycle[10] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_VecHaze, End, 
	      End, /* HGroup */ 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Vector Path "),
	      Child, ES_Win->Str[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[1] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Vector File "),
	      Child, ES_Win->Str[10] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Z Offset "),
	        Child, ES_Win->FloatStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[2][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[2][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2(" Segs "),
	        Child, ES_Win->IntStr[17] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->IntStrArrow[17][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->IntStrArrow[17][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */
	    Child, RectangleObject, End,
	    End, /* VGroup Follow Paths & Vectors */

/* Color Maps */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[13] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ColorMap, End, 
	    Child, ES_Win->Cycle[37] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_MasterCMap, End, 
	    Child, ES_Win->Cycle[38] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_CMapOrient, End, 
	    Child, ES_Win->Cycle[14] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_BorderRand, End, 
	    Child, ES_Win->Cycle[16] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ColorMatch, End, 
	    Child, ES_Win->Cycle[17] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Luminous, End, 
	    Child, ES_Win->Cycle[15] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_CMapTrees, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Default Eco "),
	      Child, ES_Win->Txt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789", End,
              Child, ES_Win->TxtArrow[0][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->TxtArrow[0][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("CMap Path "),
	      Child, ES_Win->Str[4] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890123456", End,
	      Child, ES_Win->BT_Get[4] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("CMap File "),
	      Child, ES_Win->Str[12] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    End, /* VGroup Color Maps */

/* Surfaces */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[19] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Grid, End,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Spacing "),
	      Child, ES_Win->IntStr[8] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[8][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[8][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[30] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_MapAsSfc, End,
	    Child, ES_Win->Cycle[45] = CycleObject,
		MUIA_Disabled, TRUE,
		MUIA_Cycle_Entries, ES_Cycle_SurfaceCMaps, End,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Surface El 1 "),
	      Child, ES_Win->IntStr[9] = StringObject, StringFrame,
		MUIA_String_Accept, "+-0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[9][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[9][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Surface El 2 "),
	      Child, ES_Win->IntStr[10] = StringObject, StringFrame,
		MUIA_String_Accept, "+-0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[10][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[10][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Surface El 3 "),
	      Child, ES_Win->IntStr[11] = StringObject, StringFrame,
		MUIA_String_Accept, "+-0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[11][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[11][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Surface El 4 "),
	      Child, ES_Win->IntStr[12] = StringObject, StringFrame,
		MUIA_String_Accept, "+-0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[12][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[12][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    End, /* VGroup Surfaces */

/* Fractals */
	  Child, VGroup, GroupFrame,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Fractal Depth "),
	      Child, ES_Win->IntStr[7] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "01", End,
              Child, ES_Win->IntStrArrow[7][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[7][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[6] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_RenderStyle, End, 
	    Child, ES_Win->Cycle[11] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_FixFract, End, 
	    Child, ES_Win->Cycle[39] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Perturb, End, 
	    Child, HGroup, 
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, ES_Win->FloatStr[11] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[11][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[11][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
              Child, Label2("Displacement"),
	      End, /* HGroup */
	    Child, HGroup, 
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, ES_Win->FloatStr[12] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[12][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[12][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
              Child, Label2("Slope Factor"),
	      End, /* HGroup */
            Child, ES_Win->BT_FractalSet = KeyButtonFunc('f', "\33cCreate Fractal Maps"), 
	    Child, VGroup,
              Child, ES_Win->SpeedGauge = GaugeObject, GaugeFrame, 
		   MUIA_Background, MUII_BACKGROUND,
		   MUIA_Gauge_Max, 10000,
		   MUIA_Gauge_Current, 0,
		   MUIA_Gauge_Horiz, TRUE,
		   MUIA_FixHeight, 20,
		   End,
	      Child, HGroup,
                Child, Label("Short"),
	        Child, RectangleObject, End,
                Child, Label("Long"),
		End, /* HGroup */
	      End, /* VGroup */
	    Child, HGroup,
	      Child, RectangleObject, End,
              Child, Label("Relative Render Time"),
	      Child, RectangleObject, End,
	      End, /* HGroup */
	    End, /* VGroup Fractals */

/* Ecosystems and Strata */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[31] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_RenderTrees, End,
	    Child, ES_Win->Cycle[23] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_FlattenEco, End, 
	    Child, ES_Win->Cycle[3] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_GlobalGradients, End,
	    Child, HGroup, 
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Eco m/"),
	        Child, ES_Win->FloatStr[8] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[8][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[8][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
                Child, Label2("Snow m/"),
	        Child, ES_Win->FloatStr[9] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01234", End,
                Child, ES_Win->FloatStrArrow[9][0] = ImageButton(MUII_ArrowLeft),
                Child, ES_Win->FloatStrArrow[9][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Ref Latitude "),
	      Child, ES_Win->FloatStr[10] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[10][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[10][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Tree Ht Fact "),
	      Child, ES_Win->FloatStr[4] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[4][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[4][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Model Path "),
	      Child, ES_Win->Str[11] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890123456", End,
	      Child, ES_Win->BT_Get[6] = ImageButton(MUII_Disk),
	      End, /* HGroup */

	    Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	    Child, ES_Win->Cycle[43] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_DeformationMap, End,
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Def Map Path "),
	      Child, ES_Win->Str[13] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[7] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Def Map File "),
	      Child, ES_Win->Str[14] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("   Strata Dip "),
	      Child, ES_Win->FloatStr[13] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[13][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[13][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Strata Strike "),
	      Child, ES_Win->FloatStr[14] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[14][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[14][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("  Deformation "),
	      Child, ES_Win->FloatStr[15] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[15][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[15][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */

	    End, /* VGroup Ecosystems */

/* Miscellaneous */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[12] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_HorizonMax, End, 
	    Child, ES_Win->Cycle[20] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_HorizonFix, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("   Zenith Alt "),
	      Child, ES_Win->FloatStr[3] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[3][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[3][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Sky Dither "),
	      Child, ES_Win->IntStr[13] = StringObject, StringFrame,
		MUIA_String_Accept, "+-0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[13][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[13][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[21] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_AltQ, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2(" Ref Latitude "),
	      Child, ES_Win->FloatStr[5] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[5][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[5][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Ref Longitude "),
	      Child, ES_Win->FloatStr[6] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[6][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[6][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */ 
	    Child, ES_Win->Cycle[22] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_CloudShadows, End, 
	    Child, ES_Win->Cycle[40] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_RealClouds, End, 
	    Child, ES_Win->Cycle[41] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Waves, End, 
	    Child, ES_Win->Cycle[42] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Reflections, End, 
	    Child, ES_Win->Cycle[46] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Sun, End, 
	    Child, ES_Win->Cycle[47] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Moon, End, 
	    Child, ES_Win->Cycle[49] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_SunHalo, End, 
	    Child, ES_Win->Cycle[50] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_MoonHalo, End, 
	    Child, ES_Win->Cycle[48] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Tides, End, 
	    End, /* VGroup Miscellaneous */

/* Processing */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[25] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Background, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("   BG Path "),
	      Child, ES_Win->Str[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[2] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("   BG File "),
	      Child, ES_Win->Str[5] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    Child, ES_Win->Cycle[26] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ZBuffer, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Z Buf Path "),
	      Child, ES_Win->Str[3] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      Child, ES_Win->BT_Get[3] = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
	      Child, Label2("Z Buf File "),
	      Child, ES_Win->Str[6] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456789012345", End,
	      End, /* HGroup */
	    Child, ES_Win->Cycle[27] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_Blur, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Blur Factor "),
	      Child, ES_Win->IntStr[14] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[14][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[14][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, ES_Win->Cycle[28] = CycleObject,
		MUIA_Cycle_Entries, ES_Cycle_ZBufBlur, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("Max Blur Offset "),
	      Child, ES_Win->FloatStr[7] = StringObject, StringFrame,
		MUIA_String_Accept, "+-.0123456789",
		MUIA_FixWidthTxt, "01234567890", End,
              Child, ES_Win->FloatStrArrow[7][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->FloatStrArrow[7][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    End, /* VGroup Processing */

#ifdef ENABLE_SCALING
/* Post-Process */
	  Child, VGroup, GroupFrame,
	    Child, ES_Win->Cycle[] = CycleObject,
		MUIA_Disabled, TRUE,
		MUIA_Cycle_Entries, ES_Cycle_Scaling, End, 
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("   Final Width "),
	      Child, ES_Win->IntStr[15] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[15][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[15][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    Child, HGroup, MUIA_Group_HorizSpacing, 0,
              Child, Label2("  Final Height "),
	      Child, ES_Win->IntStr[16] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
              Child, ES_Win->IntStrArrow[16][0] = ImageButton(MUII_ArrowLeft),
              Child, ES_Win->IntStrArrow[16][1] = ImageButton(MUII_ArrowRight),
	      End, /* HGroup */
	    End, /* VGroup Post-Process */
#endif /* ENABLE_SCALING */
	  End, /* PageGroup */

	Child, HGroup,
	  Child, Label2("Render Memory"),
	  Child, ES_Win->UseTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012", End,
	  End, /* HGroup */
        Child, HGroup,
          Child, ES_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"), 
          Child, ES_Win->BT_Render = KeyButtonFunc('r', "\33cRender"), 
          Child, ES_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"), 
          End, /* HGroup */

	End, /* VGroup Window Contents */
      End; /* WindowObject */

  if (! ES_Win->SettingsWin)
   {
   Close_ES_Window(1);
   User_Message((CONST_STRPTR)"Render Settings Editor", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, ES_Win->SettingsWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Close requests */
  DoMethod(ES_Win->SettingsWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_ES_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, ES_Win->BT_ChangeScale, ID_SC_WINDOW,
   ES_Win->BT_FractalSet, ID_ES_FRACTALSET,
   ES_Win->BT_Apply, ID_ES_APPLY, ES_Win->BT_Render, MO_RENDER,
   ES_Win->BT_Cancel, ID_ES_CLOSE, NULL);

/* set values in gadgets */
  Set_ES_Window();
  SetMemoryReqTxt();
  SetRenderSpeedGauge();

/* set notifications */
/* STRING1 */
  for (i=0; i<23; i++)
   {
   DoMethod(ES_Win->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ES_INTSTR(i));
   } /* for i=0... */
/* ARROW1 */
  for (i=0; i<23; i++)
   MUI_DoNotiPresFal(app, ES_Win->IntStrArrow[i][0], ID_ES_INTSTRARROWLEFT(i), NULL);

/* ARROW2 */
  for (i=0; i<23; i++)
   MUI_DoNotiPresFal(app, ES_Win->IntStrArrow[i][1], ID_ES_INTSTRARROWRIGHT(i), NULL);

/* STRING2 */
  for (i=0; i<16; i++)
   {
   DoMethod(ES_Win->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ES_FLOATSTR(i));
   } /* for i=0... */
/* ARROW3-6 */
  for (i=0; i<16; i++)
   MUI_DoNotiPresFal(app, ES_Win->FloatStrArrow[i][0], ID_ES_FLOATSTRARROWLEFT(i),
    ES_Win->FloatStrArrow[i][1], ID_ES_FLOATSTRARROWRIGHT(i), NULL);
  MUI_DoNotiPresFal(app,
   ES_Win->TxtArrow[0][0], ID_ES_TEXTARROWLEFT(0),
   ES_Win->TxtArrow[0][1], ID_ES_TEXTARROWRIGHT(0), NULL);

/* CYCLE1 */
  for (i=0; i<51; i++)
   {
   DoMethod(ES_Win->Cycle[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ES_CYCLE(i));
   } /* for i=0... */

/* BUTTONS2 */
  for (i=0; i<8; i++)
   MUI_DoNotiPresFal(app, ES_Win->BT_Get[i], ID_ES_GETPATH(i), NULL);

/* STRING3 */
  for (i=0; i<15; i++)
   {
   DoMethod(ES_Win->Str[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_ES_PATHSTR(i));
   } /* for i=0... */

/* set tab cycle chain */
  DoMethod(ES_Win->SettingsWin, MUIM_Window_SetCycleChain,
/* Render & Size*/
   ES_Win->IntStr[4], ES_Win->IntStr[5], ES_Win->FloatStr[0], ES_Win->IntStr[6],
   ES_Win->BT_ChangeScale,
   ES_Win->IntStr[0], ES_Win->IntStr[22],
   ES_Win->IntStr[2], ES_Win->IntStr[1], ES_Win->IntStr[3],
   ES_Win->IntStr[21],
   ES_Win->Cycle[0], ES_Win->Cycle[1], ES_Win->Cycle[2], ES_Win->Cycle[36],
   ES_Win->Cycle[18],
/* Image Save */
   ES_Win->Str[0], ES_Win->BT_Get[0], ES_Win->Str[8], ES_Win->Str[7],
   ES_Win->BT_Get[5],/* ES_Win->Str[9],*/ ES_Win->Cycle[4], ES_Win->Cycle[5],
   ES_Win->Cycle[34], ES_Win->Cycle[35],
/* Follow paths & vectors */
   ES_Win->Cycle[24], ES_Win->IntStr[18],
   ES_Win->Cycle[8], ES_Win->FloatStr[1],
   ES_Win->Cycle[32], ES_Win->IntStr[19], ES_Win->IntStr[20],
   ES_Win->Cycle[9], ES_Win->Cycle[10],
   ES_Win->Str[1], ES_Win->BT_Get[1], ES_Win->Str[10],
   ES_Win->FloatStr[2], ES_Win->IntStr[17],
/* Color Maps */
   ES_Win->Cycle[13], ES_Win->Cycle[37], ES_Win->Cycle[38],
   ES_Win->Cycle[14], ES_Win->Cycle[15], ES_Win->Cycle[16],
   ES_Win->Str[4],
   ES_Win->BT_Get[4],
   ES_Win->Str[12],
/* Surfaces and strata */
   ES_Win->Cycle[19], ES_Win->IntStr[8], ES_Win->Cycle[30], ES_Win->IntStr[9],
   ES_Win->IntStr[10], ES_Win->IntStr[11], ES_Win->IntStr[12],
/* Fractals */
   ES_Win->IntStr[7], ES_Win->Cycle[11], ES_Win->Cycle[39],
   ES_Win->Cycle[29], ES_Win->Cycle[6],
   ES_Win->Cycle[7],
   ES_Win->FloatStr[11], ES_Win->FloatStr[12], ES_Win->BT_FractalSet,
/* Ecosystems */
   ES_Win->Cycle[31], ES_Win->Cycle[23],
   ES_Win->Cycle[3], ES_Win->FloatStr[8], ES_Win->FloatStr[9],
   ES_Win->FloatStr[10], ES_Win->FloatStr[4],
   ES_Win->Str[11], ES_Win->BT_Get[6],
   ES_Win->Cycle[43], ES_Win->Str[13], ES_Win->BT_Get[7],
   ES_Win->Str[14], ES_Win->FloatStr[13], ES_Win->FloatStr[14],
/* Miscellaneous */
   ES_Win->Cycle[12],
   ES_Win->Cycle[20], ES_Win->FloatStr[3], ES_Win->IntStr[13], ES_Win->Cycle[21],
   ES_Win->FloatStr[5], ES_Win->FloatStr[6], ES_Win->Cycle[22], ES_Win->Cycle[40],
   ES_Win->Cycle[41], ES_Win->Cycle[42], ES_Win->Cycle[46], ES_Win->Cycle[47],
   ES_Win->Cycle[49], ES_Win->Cycle[50], ES_Win->Cycle[48],
/* Pre-process */
   ES_Win->Cycle[25], ES_Win->Str[2], ES_Win->BT_Get[2], ES_Win->Str[5],
   ES_Win->Cycle[26], ES_Win->Str[3], ES_Win->BT_Get[3], ES_Win->Str[6],
/* Post-process */
   ES_Win->Cycle[27], ES_Win->IntStr[14], ES_Win->Cycle[28],
   ES_Win->FloatStr[7],
#ifdef ENABLE_SCALING
   ES_Win->Cycle[], ES_Win->IntStr[15], ES_Win->IntStr[16],
#endif /* ENABLE_SCALING */
/* Buttons at bottom */
   ES_Win->BT_Apply, ES_Win->BT_Render, ES_Win->BT_Cancel, NULL);

/* set return cycle chain */
/* Render & Size*/
  DoMethod(ES_Win->IntStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[5]);
  DoMethod(ES_Win->IntStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[0]);
  DoMethod(ES_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[6]);
  DoMethod(ES_Win->IntStr[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[0]);
  DoMethod(ES_Win->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[22]);
  DoMethod(ES_Win->IntStr[22], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[2]);
  DoMethod(ES_Win->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[1]);
  DoMethod(ES_Win->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[3]);
  DoMethod(ES_Win->IntStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[21]);
  DoMethod(ES_Win->IntStr[21], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[4]);

/* Image Save */
  DoMethod(ES_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[8]);
  DoMethod(ES_Win->Str[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[7]);
  DoMethod(ES_Win->Str[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[0]);
/*
  DoMethod(ES_Win->Str[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[0]);
*/
/* Follow paths & vectors */
  DoMethod(ES_Win->IntStr[18], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[1]);
  DoMethod(ES_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[19]);
  DoMethod(ES_Win->IntStr[19], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[20]);
  DoMethod(ES_Win->IntStr[20], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[1]);

  DoMethod(ES_Win->Str[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[10]);
  DoMethod(ES_Win->Str[10], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[2]);
  DoMethod(ES_Win->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[17]);
  DoMethod(ES_Win->IntStr[17], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[18]);

/* Color Maps */
  DoMethod(ES_Win->Str[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[12]);
  DoMethod(ES_Win->Str[12], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[4]);

/* Surfaces */
  DoMethod(ES_Win->IntStr[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[9]);
  DoMethod(ES_Win->IntStr[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[10]);
  DoMethod(ES_Win->IntStr[10], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[11]);
  DoMethod(ES_Win->IntStr[11], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[12]);
  DoMethod(ES_Win->IntStr[12], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[8]);

/* Fractals */
  DoMethod(ES_Win->IntStr[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[11]);
  DoMethod(ES_Win->FloatStr[11], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[12]);
  DoMethod(ES_Win->FloatStr[12], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[7]);

/* Ecosystems */
  DoMethod(ES_Win->FloatStr[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[9]);
  DoMethod(ES_Win->FloatStr[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[10]);
  DoMethod(ES_Win->FloatStr[10], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[4]);
  DoMethod(ES_Win->FloatStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[11]);
  DoMethod(ES_Win->Str[11], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[13]);

  DoMethod(ES_Win->Str[13], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[14]);
  DoMethod(ES_Win->Str[14], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[13]);
  DoMethod(ES_Win->FloatStr[13], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[14]);
  DoMethod(ES_Win->FloatStr[14], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[15]);
  DoMethod(ES_Win->FloatStr[15], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[8]);

/* Miscellaneous */
  DoMethod(ES_Win->FloatStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[13]);
  DoMethod(ES_Win->IntStr[13], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[5]);
  DoMethod(ES_Win->FloatStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[6]);
  DoMethod(ES_Win->FloatStr[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[3]);

/* Pre-process */
  DoMethod(ES_Win->Str[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[5]);
  DoMethod(ES_Win->Str[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[3]);
  DoMethod(ES_Win->Str[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[6]);
  DoMethod(ES_Win->Str[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->Str[2]);

/* Post-process */
  DoMethod(ES_Win->IntStr[14], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->FloatStr[7]);
  DoMethod(ES_Win->FloatStr[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[15]);
  DoMethod(ES_Win->IntStr[15], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[16]);
  DoMethod(ES_Win->IntStr[16], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   ES_Win->SettingsWin, 3, MUIM_Set, MUIA_Window_ActiveObject, ES_Win->IntStr[14]);

#ifdef USE_WCS_HELP
#ifdef USE_SETTINGS_HELP
  DoMethod(ES_Win->SettingsWin, MUIM_Notify,
	 MUIA_Window_InputEvent, "help",/*"rcommand h",*/
	app, 2, MUIM_Application_ReturnID, ID_ES_HELP);
#endif /* USE_SETTINGS_HELP */
#endif /* USE_WCS_HELP */

  set(ES_Win->SettingsWin, MUIA_Window_ActiveObject, (ULONG)ES_Win->IntStr[0]);

/* Open window */
  set(ES_Win->SettingsWin, MUIA_Window_Open, TRUE);
  get(ES_Win->SettingsWin, MUIA_Window_Open, &open);

  if (! open)
   {
   Close_ES_Window(1);
   return;
   } /* out of memory */

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(ES_Win->SettingsWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_ES_ACTIVATE);

/* Get Window structure pointer */
  get(ES_Win->SettingsWin, MUIA_Window_Window, &ES_Win->Win);

} /* Make_ES_Window() */

/**********************************************************************/

void Close_ES_Window(short apply)
{

 if (ES_Win)
  {
  if (ES_Win->SettingsWin)
   {
   if (apply)
    {
    FixPar(0, 0x1000);
    FixPar(1, 0x1000);
    }
   else
    {
    UndoPar(0, 0x1000);
    }

   set(ES_Win->SettingsWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, ES_Win->SettingsWin);
   MUI_DisposeObject(ES_Win->SettingsWin);
   } /* if */
  free_Memory(ES_Win, sizeof (struct SettingsWindow));
  ES_Win = NULL;
  } /* if */
/*
 if (! apply)
  Par_Mod &= 0x0111;
*/
} /* Close_ES_Window() */

/**********************************************************************/

void Handle_ES_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_ES_Window();
   return;
   } /* Open Render Settings Editor Window */

  if (! ES_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
/*    DoMethod(ES_Win->SettingsWin, MUIM_Window_ToFront);*/
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Settings Editing Window */
#ifdef USE_WCS_HELP
#ifdef USE_SETTINGS_HELP
   case GP_HELP:
    {
    APTR data;

    get(ES_Win->SettingsWin, MUIA_Window_ActiveObject, &data);
    Help_ES_Window(data);
    break;
    } /* Help message */
#endif /* USE_SETTINGS_HELP */
#endif /* USE_WCS_HELP */
   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_ES_FIX:
      {
      FixPar(1, 0x1000);
      break;
      } /*  */
     case ID_ES_UNDO:
      {
      UndoPar(1, 0x1000);
      Set_ES_Window();
      break;
      } /*  */
     case ID_ES_RESTORE:
      {
      UndoPar(0, 0x1000);
      Set_ES_Window();
      break;
      } /*  */
     case ID_ES_SAVE:
      {
      if (saveparams(0x1000, 0, 0) == 0)
       FixPar(0, 0x1000);
      break;
      } /*  */
     case ID_ES_LOAD:
      {
      loadparams(0x1000, 0);
      FixPar(0, 0x1000);
      FixPar(1, 0x1000);
      Set_ES_Window();
      break;
      } /*  */
     case ID_ES_FRACTALSET:
      {
      FractalDepth_Preset();
      break;
      } /* precompute optimal fractal depth per polygon */
     case ID_ES_APPLY:
      {
      if (memcmp(&settings, &UndoSetPar[0], sizeof (settings)))
       Par_Mod |= 0x1000;
      Close_ES_Window(1);
      break;
      } /* Apply */
     case ID_ES_CLOSE:
      {
      Close_ES_Window(0);
      break;
      } /* Close Window */
     case ID_ES_CLOSEQUERY:
      {
      if (memcmp(&settings, &UndoSetPar[0], sizeof (settings)))
       {
       Par_Mod |= 0x1000;
       Close_ES_Window(CloseWindow_Query((STRPTR)"Settings Editor"));
       }
      else
       Close_ES_Window(1);
      break;
      } /* Close Window */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    char dummyfile[32] = {0};

    i = WCS_ID - ID_ES_GETPATH(0);

    switch (i)
     {
     case 0:
      {
      getfilename(1, "Frame Path/Name", framepath, framefile);
      set(ES_Win->Str[0], MUIA_String_Contents, (ULONG)framepath);
      set(ES_Win->Str[8], MUIA_String_Contents, (ULONG)framefile);
      break;
      } /* framepath */
     case 1:
      {
      getfilename(1, "Vector File Path/Name", linepath, linefile);
      set(ES_Win->Str[1], MUIA_String_Contents, (ULONG)linepath);
      set(ES_Win->Str[10], MUIA_String_Contents, (ULONG)linefile);
      break;
      } /* linepath */
     case 2:
      {
      getfilename(0, "Background File Path/Name",
	 backgroundpath, backgroundfile);
      set(ES_Win->Str[2], MUIA_String_Contents, (ULONG)backgroundpath);
      set(ES_Win->Str[5], MUIA_String_Contents, (ULONG)backgroundfile);
      break;
      } /* backgroundpath */
     case 3:
      {
      getfilename(0, "Z Buffer File Path/Name",
	 zbufferpath, zbufferfile);
      set(ES_Win->Str[3], MUIA_String_Contents, (ULONG)zbufferpath);
      set(ES_Win->Str[6], MUIA_String_Contents, (ULONG)zbufferfile);
      break;
      } /* zbufferpath */
     case 4:
      {
      getfilename(0, "Color Map File Path",
	 colormappath, colormapfile);
      set(ES_Win->Str[4], MUIA_String_Contents, (ULONG)colormappath);
      set(ES_Win->Str[12], MUIA_String_Contents, (ULONG)colormapfile);
      break;
      } /* colormappath */
     case 5:
      {
      strcpy(tempfile, framefile);
      strcat(tempfile, ".temp");
      getfilename(1, "Temporary File Path/Name", temppath, tempfile);
      set(ES_Win->Str[7], MUIA_String_Contents, (ULONG)temppath);
/*      set(ES_Win->Str[9], MUIA_String_Contents, tempfile);*/
      break;
      } /* temppath */
     case 6:
      {
      getfilename(1, "Ecosystem Model Path", modelpath, dummyfile);
      set(ES_Win->Str[11], MUIA_String_Contents, (ULONG)modelpath);
      break;
      } /* temppath */
     case 7:
      {
      getfilename(1, "Deformation Map Path", deformpath, deformfile);
      set(ES_Win->Str[13], MUIA_String_Contents, (ULONG)deformpath);
      set(ES_Win->Str[14], MUIA_String_Contents, (ULONG)deformfile);
      break;
      } /* temppath */
     } /* switch i */
    break;
    } /* BUTTONS2 */

   case GP_STRING1:
    {
    LONG data;

    i = WCS_ID - ID_ES_INTSTR(0);

    get(ES_Win->IntStr[i], MUIA_String_Integer, &data);
    switch (i)
     {
     case 0:
      {
      if (data < 0) data = 0;
      settings.startframe = data;
      get(ES_Win->IntStr[22], MUIA_String_Integer, &data);
      settings.maxframes =
      	1 + (data - settings.startframe ) / settings.stepframes;
      set(ES_Win->IntStr[1], MUIA_String_Integer, settings.maxframes);
      break;
      } /* startframe */
     case 1:
      {
      if (data < 1) data = 1;
      settings.maxframes = data;
      Disable_ES_Gads(data == 1 && settings.rendersegs == 1,
	ES_Win->Cycle[2], NULL);
      data = settings.startframe + (settings.maxframes - 1) * settings.stepframes;
      if (data < 0)
       data = 0;
      set(ES_Win->IntStr[22], MUIA_String_Integer, data);
      break;
      } /* maxframes */
     case 2:
      {
      if (data == 0) data = 1;
      settings.stepframes = data;
      get(ES_Win->IntStr[22], MUIA_String_Integer, &data);
      settings.maxframes =
      	1 + (data - settings.startframe ) / settings.stepframes;
      set(ES_Win->IntStr[1], MUIA_String_Integer, settings.maxframes);
      break;
      } /* stepframes */
     case 3:
      {
      if (data < 1) data = 1;
      settings.rendersegs = data;
      Disable_ES_Gads(data > 1, ES_Win->Cycle[5], ES_Win->IntStr[21], NULL);
      Disable_ES_Gads(data == 1 && settings.maxframes == 1,
	ES_Win->Cycle[2], NULL);
      break;
      } /* rendersegs */
     case 4:
      {
      if (data < 1) data = 1;
      settings.scrnwidth = data;
      break;
      } /* scrnwidth */
     case 5:
      {
      if (data < 1) data = 1;
      settings.scrnheight = data;
      break;
      } /* scrnheight */
     case 6:
      {
      if (data < 0) data = 0;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.overscan = data;
      break;
      } /* overscan */
     case 7:
      {
      if (data < 0) data = 0;
      else if (data > 9) data = 9;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.fractal = data;
      SetRenderSpeedGauge();
      break;
      } /* fractal */
     case 8:
      {
      if (data < 1) data = 1;
      settings.gridsize = data;
      break;
      } /* gridsize */
     case 9:
      {
      settings.surfel[0] = data;
      break;
      } /* surfel[0] */
     case 10:
      {
      settings.surfel[1] = data;
      break;
      } /* surfel[1] */
     case 11:
      {
      settings.surfel[2] = data;
      break;
      } /* surfel[2] */
     case 12:
      {
      settings.surfel[3] = data;
      break;
      } /* surfel[3] */
     case 13:
      {
      if (data < 0) data = 0;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.skyalias = (double)data;
      break;
      } /* (long)skyalias */
     case 14:
      {
      if (data < 1) data = 1;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.aliasfactor = data;
      break;
      } /* aliasfactor */
#ifdef ENABLE_SCALING
     case 15:
      {
      if (data < 1) data = 1;
      settings.scalewidth = data;
      break;
      } /* scalewidth */
     case 16:
      {
      if (data < 1) data = 1;
      settings.scaleheight = data;
      break;
      } /* scaleheight */
#endif /* ENABLE_SCALING */
     case 17:
      {
      if (data < 1) data = 1;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.vecsegs = data;
      break;
      } /* active vector segments */
     case 18:
      {
      if (data < 1) data = 1;
      settings.lookaheadframes = data;
      break;
      } /* scaleheight */
     case 19:
      {
      if (data < 0) data = 0;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.easein = data;
      break;
      } /* scaleheight */
     case 20:
      {
      if (data < 0) data = 0;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.easeout = data;
      break;
      } /* scaleheight */
     case 21:
      {
      if (data < 0) data = 0;
      set(ES_Win->IntStr[i], MUIA_String_Integer, data);
      settings.startseg = data;
      break;
      } /* scaleheight */
     case 22:
      {
      if (data < 0)
       {
       data = 0;
       set(ES_Win->IntStr[22], MUIA_String_Integer, data);
       } /* if */
      if (data < settings.startframe) settings.stepframes = -abs(settings.stepframes);
      else settings.stepframes = abs(settings.stepframes);
      settings.maxframes = 1 + (data - settings.startframe) / settings.stepframes;
      set(ES_Win->IntStr[2], MUIA_String_Integer, settings.stepframes);
      set(ES_Win->IntStr[1], MUIA_String_Integer, settings.maxframes);
      break;
      } /* last frame */
     } /* switch i */
    SetMemoryReqTxt();
    break;
    } /* STRING1 */

   case GP_STRING2:
    {
    char *data;
    double floatdata;

    i = WCS_ID - ID_ES_FLOATSTR(0);

    get(ES_Win->FloatStr[i], MUIA_String_Contents, &data);
    floatdata = atof(data);
    switch (i)
     {
     case 0:
      {
      settings.picaspect = floatdata;
      break;
      } /* picaspect */
     case 1:
      {
      settings.bankfactor = floatdata;
      break;
      } /* bankfactor */
     case 2:
      {
      settings.lineoffset = floatdata;
      break;
      } /* lineoffset */
     case 3:
      {
      settings.zenith = floatdata;
      break;
      } /* zenith */
     case 4:
      {
      settings.treefactor = floatdata;
      break;
      } /* treefactor */
     case 5:
      {
      settings.altqlat = floatdata;
      break;
      } /* altqlat */
     case 6:
      {
      settings.altqlon = floatdata;
      break;
      } /* altqlon */
     case 7:
      {
      settings.zalias = floatdata;
      break;
      } /* zalias */
     case 8:
      {
      settings.globecograd = floatdata;
      break;
      } /* zalias */
     case 9:
      {
      settings.globsnowgrad = floatdata;
      break;
      } /* */
     case 10:
      {
      settings.globreflat = floatdata;
      break;
      } /*  */
     case 11:
      {
      settings.displacement = floatdata;
      break;
      } /*  */
     case 12:
      {
      settings.dispslopefact = floatdata;
      break;
      } /*  */
     case 13:
      {
      settings.stratadip = floatdata;
      break;
      } /*  */
     case 14:
      {
      settings.stratastrike = floatdata;
      break;
      } /*  */
     case 15:
      {
      settings.deformscale = floatdata;
      break;
      } /*  */
     } /* switch i */
    break;
    } /* STRING2 */

   case GP_STRING3:
    {
    char *data;

    i = WCS_ID - ID_ES_PATHSTR(0);
    get(ES_Win->Str[i], MUIA_String_Contents, &data);
    switch (i)
     {
     case 0:
      {
      strcpy(framepath, data);
      break;
      } /* frame path */
     case 1:
      {
      strcpy(linepath, data);
      break;
      } /* vector path */
     case 2:
      {
      strcpy(backgroundpath, data);
      break;
      } /* background path */
     case 3:
      {
      strcpy(zbufferpath, data);
      break;
      } /* z buffer file */
     case 4:
      {
      strcpy(colormappath, data);
      break;
      } /* color map path */
     case 5:
      {
      strcpy(backgroundfile, data);
      break;
      } /* background file */
     case 6:
      {
      strcpy(zbufferfile, data);
      break;
      } /* z buffer file */
     case 7:
      {
      strcpy(temppath, data);
      break;
      } /* temp path */
     case 8:
      {
      strcpy(framefile, data);
      break;
      } /* frame file */
     case 9:
      {
      strcpy(tempfile, data);
      break;
      } /* temp file */
     case 10:
      {
      strcpy(linefile, data);
      break;
      } /* vector file */
     case 11:
      {
      strcpy(modelpath, data);
      break;
      } /* ecosystem model path */
     case 12:
      {
      strcpy(colormapfile, data);
      break;
      } /* master color map file */
     case 13:
      {
      strcpy(deformpath, data);
      break;
      } /* strata deformation map path */
     case 14:
      {
      strcpy(deformfile, data);
      break;
      } /* strata deformation map file */
     } /* switch i */
    Proj_Mod = 1;
    break;
    } /* STRING3 */

   case GP_ARROW1:
    {
    LONG data;

    i = WCS_ID - ID_ES_INTSTRARROWLEFT(0);
    get(ES_Win->IntStr[i], MUIA_String_Integer, &data);
    Set_IntStringValue(i, -1, &data);
    set(ES_Win->IntStr[i], MUIA_String_Integer, data);
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    LONG data;

    i = WCS_ID - ID_ES_INTSTRARROWRIGHT(0);
    get(ES_Win->IntStr[i], MUIA_String_Integer, &data);
    Set_IntStringValue(i, 1, &data);
    set(ES_Win->IntStr[i], MUIA_String_Integer, data);
    break;
    } /* ARROW2 */

   case GP_ARROW3:
    {
    char *data;
    double floatdata;

    i = WCS_ID - ID_ES_FLOATSTRARROWLEFT(0);
    get(ES_Win->FloatStr[i], MUIA_String_Contents, &data);
    floatdata = atof(data);
    Set_FloatStringValue(i, -1, &floatdata);
    setfloat(ES_Win->FloatStr[i], floatdata);
    break;
    } /* ARROW3 */

   case GP_ARROW4:
    {
    char *data;
    double floatdata;

    i = WCS_ID - ID_ES_FLOATSTRARROWRIGHT(0);
    get(ES_Win->FloatStr[i], MUIA_String_Contents, &data);
    floatdata = atof(data);
    Set_FloatStringValue(i, 1, &floatdata);
    setfloat(ES_Win->FloatStr[i], floatdata);
    break;
    } /* ARROW4 */

   case GP_ARROW5:
    {
    i = WCS_ID - ID_ES_TEXTARROWLEFT(0);
    switch (i)
     {
     case 0:
      {
      if (settings.defaulteco > 0) settings.defaulteco --;
      set(ES_Win->Txt[0], MUIA_Text_Contents,
	 (ULONG)PAR_NAME_ECO(settings.defaulteco));
      break;
      } /* default ecosystem */
     } /* switch (i) */
    break;
    } /* ARROW5 */

   case GP_ARROW6:
    {
    i = WCS_ID - ID_ES_TEXTARROWRIGHT(0);
    switch (i)
     {
     case 0:
      {
      if (settings.defaulteco < ECOPARAMS - 1) settings.defaulteco ++;
      set(ES_Win->Txt[0], MUIA_Text_Contents,
	 (ULONG)PAR_NAME_ECO(settings.defaulteco));
      break;
      } /* default ecosystem */
     } /* switch (i) */
    break;
    } /* ARROW6 */

   case GP_CYCLE1:
    {
    LONG data;

    i = WCS_ID - ID_ES_CYCLE(0);
    get(ES_Win->Cycle[i], MUIA_Cycle_Active, &data);
    switch (i)
     {
     case 0:
      {
      if (data) settings.renderopts |= 0x01;
      else settings.renderopts ^=0x01;
      Disable_ES_Gads(data, ES_Win->Cycle[4], ES_Win->Cycle[36],
	ES_Win->Str[0], ES_Win->Str[8], NULL);
      Disable_ES_Gads(data && settings.fieldrender,
	ES_Win->Str[7], ES_Win->Cycle[18], NULL);
      break;
      } /* renderopts RGB */
     case 1:
      {
      if (data) settings.renderopts |= 0x10;
      else settings.renderopts ^=0x10;
      break;
      } /* renderopts screen */
     case 2:
      {
      if (data) settings.renderopts |= 0x100;
      else settings.renderopts ^=0x100;
      break;
      } /* renderopts data */
     case 3:
      {
      settings.worldmap = data;
      Disable_ES_Gads(data, ES_Win->FloatStr[8], ES_Win->FloatStr[9],
	ES_Win->FloatStr[10], NULL);
      break;
      } /* global gradients */
     case 4:
      {
      settings.saveIFF = data;
      break;
      } /* saveIFF */
     case 5:
      {
      settings.composite = data;
      break;
      } /* composite */
     case 6:
      {
      settings.smoothfaces = (data == 1);
      settings.displace = (data == 2);
      Disable_ES_Gads(data < 2, ES_Win->Cycle[39], NULL);
      Disable_ES_Gads(settings.displace,
	ES_Win->FloatStr[11], ES_Win->FloatStr[12], ES_Win->Cycle[41], NULL);
      SetRenderSpeedGauge();
      break;
      } /*  */
     case 7:
      {
      break;
      } /*  */
     case 8:
      {
      settings.bankturn = data;
      break;
      } /* bankturn */
     case 9:
      {
      if (data == 2) settings.linetoscreen = -1;
      else settings.linetoscreen = data;
      Disable_ES_Gads((short)(data < 2), ES_Win->IntStr[17], ES_Win->FloatStr[2], 
	ES_Win->Str[1], ES_Win->Str[10], NULL);
      Disable_ES_Gads((short)(data == 1), ES_Win->Cycle[10], NULL);
      break;
      } /* linetoscreen */
     case 10:
      {
      settings.linefade = data;
      break;
      } /* linefade */
     case 11:
      {
      settings.fixfract = (data > 0);
      settings.fractalmap = (data == 2);
      SetRenderSpeedGauge();
      break;
      } /* fixfract */
     case 12:
      {
      settings.horizonmax = data;
      break;
      } /* premap */
     case 13:
      {
      settings.colrmap = data;
      Disable_ES_Gads(data, ES_Win->Cycle[37], ES_Win->Cycle[14], ES_Win->Cycle[15],
	ES_Win->Cycle[16], ES_Win->Cycle[17],
	ES_Win->Str[4], NULL);
      Disable_ES_Gads(settings.mastercmap && settings.colrmap,
	ES_Win->Cycle[38], ES_Win->Str[12], NULL);
      break;
      } /* colrmap */
     case 14:
      {
      settings.borderandom = data;
      break;
      } /* borderandom */
     case 15:
      {
      settings.cmaptrees = data;
      break;
      } /* cmaptrees */
     case 16:
      {
      settings.ecomatch = data;
      break;
      } /* ecomatch */
     case 17:
      {
      settings.cmapluminous = data;
      break;
      } /* treemodel */
     case 18:
      {
      settings.fielddominance = data;
      break;
      } /* statistics */
     case 19:
      {
      settings.drawgrid = data;
      Disable_ES_Gads(data, ES_Win->IntStr[8], NULL);
      break;
      } /* drawgrid */
     case 20:
      {
      settings.horfix = data;
      Disable_ES_Gads(! data, ES_Win->FloatStr[3], NULL);
      break;
      } /* horfix */
     case 21:
      {
      settings.alternateq = data;
      Disable_ES_Gads(data, ES_Win->FloatStr[5], ES_Win->FloatStr[6], NULL);
      break;
      } /* alternateq */
     case 22:
      {
      settings.clouds = data;
      break;
      } /* clouds */
     case 23:
      {
      settings.flatteneco = data;
      break;
      } /* clouds */
     case 24:
      {
      settings.lookahead = data;
      Disable_ES_Gads(data, ES_Win->IntStr[18], NULL);
      break;
      } /* clouds */
     case 25:
      {
      settings.background = data;
      Disable_ES_Gads(data, ES_Win->Str[2], ES_Win->Str[5], NULL);
      break;
      } /* background */
     case 26:
      {
      settings.zbuffer = data;
      Disable_ES_Gads(data, ES_Win->Str[3], ES_Win->Str[6], NULL);
      break;
      } /* zbuffer */
     case 27:
      {
      settings.antialias = data;
      Disable_ES_Gads(data, ES_Win->IntStr[14], ES_Win->Cycle[28], NULL);
      Disable_ES_Gads(data && settings.zbufalias, ES_Win->FloatStr[7], NULL);
      break;
      } /* antialias */
     case 28:
      {
      settings.zbufalias = data;
      Disable_ES_Gads(data, ES_Win->FloatStr[7], NULL);
      break;
      } /* zbufalias */
     case 29:
      {
      break;
      } /* */
     case 30:
      {
      settings.mapassfc = data;
      break;
      } /* mapassfc */
     case 31:
      {
      settings.rendertrees = data;
      break;
      } /* */
     case 32:
      {
      settings.velocitydistr = data;
      Disable_ES_Gads(settings.velocitydistr, ES_Win->IntStr[19],
	ES_Win->IntStr[20], NULL);
      break;
      } /* */
     case 33:
      {
      break;
      } /* */
     case 34:
      {
      settings.exportzbuf = data;
      Disable_ES_Gads(data, ES_Win->Cycle[35], NULL);
      break;
      } /* exportzbuf */
     case 35:
      {
      settings.zformat = data;
      break;
      } /* zformat */
     case 36:
      {
      settings.fieldrender = data;
      Disable_ES_Gads(data && (settings.renderopts & 0x01),
	ES_Win->Str[7], ES_Win->Cycle[18], NULL);
      break;
      } /* field rendering */
     case 37:
      {
      settings.mastercmap = data;
      Disable_ES_Gads(settings.mastercmap && settings.colrmap,
	ES_Win->Cycle[38], ES_Win->Str[12], NULL);
      break;
      } /* use master color map */
     case 38:
      {
      settings.cmaporientation = data;
      break;
      } /* master color map orientation */
     case 39:
      {
      settings.perturb = data;
      SetRenderSpeedGauge();
      break;
      } /* fractal color perturbation */
     case 40:
      {
      settings.realclouds = data;
      break;
      } /* 3D clouds */
     case 41:
      {
      settings.waves = data;
      break;
      } /* waves */
     case 42:
      {
      settings.reflections = data;
      break;
      } /* reflections */
     case 43:
      {
      settings.deformationmap = data;
      Disable_ES_Gads(settings.deformationmap,
	ES_Win->Str[13], ES_Win->Str[14], ES_Win->FloatStr[15], NULL);
      break;
      } /* deformation map */
     case 45:
      {
      settings.cmapsurface = data;
      break;
      } /* color map surfaces */
     case 46:
      {
      settings.sun = data;
      Disable_ES_Gads(settings.sun, ES_Win->Cycle[49], NULL);
      break;
      } /* sun */
     case 47:
      {
      settings.moon = data;
      Disable_ES_Gads(settings.moon, ES_Win->Cycle[50], NULL);
      break;
      } /* moon */
     case 48:
      {
      settings.tides = data;
      break;
      } /* tides */
     case 49:
      {
      settings.sunhalo = data;
      break;
      } /* sun halo */
     case 50:
      {
      settings.moonhalo = data;
      break;
      } /* moon halo */
     } /* switch i */
    SetMemoryReqTxt();
    break;
    } /* CYCLE1 */

   } /* switch gadget group */

} /* Handle_ES_Window() */

/**********************************************************************/

void Handle_SB_Buttons(ULONG WCS_ID)
{
long i;

 if (! ES_Win)
  {
  Make_ES_Window();
  } /* Open Render Settings Editor Window */

 if (! ES_Win)
  return;

 switch (WCS_ID & 0x0000ff00)
  {
  case GP_BUTTONS1:
   {
   i = WCS_ID - ID_SB_SETPAGE(0);

   set(ES_Win->Pages, MUIA_Group_ActivePage, i);
   break;
   }
  } /* switch */

} /* Handle_SB_Buttons() */
