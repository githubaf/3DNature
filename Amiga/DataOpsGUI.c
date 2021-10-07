/* DataOpsGUI.c
** World Construction Set GUI for Data Operations module.
** Copyright 1994 by Gary R. Huber and Chris Eric Hanson.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Version.h"

STATIC_FCN void Set_DC_Data(void); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Set_DI_Data(void); // used locally only -> static, AF 25.7.2021
STATIC_FCN double DegMinSecToDegrees(char *str); // used locally only -> static, AF 25.7.2021
STATIC_FCN void Get_DC_InputFile(void);// used locally only -> static, AF 25.7.2021/


#define DEM_DATA_INPUT_ARRAY		0
#define DEM_DATA_INPUT_WCSDEM		1
#define DEM_DATA_INPUT_ZBUF		2
#define DEM_DATA_INPUT_ASCII		3
#define DEM_DATA_INPUT_VISTA		4
#define DEM_DATA_INPUT_IFF		5
#define DEM_DATA_INPUT_DTED		6
#define DEM_DATA_OUTPUT_ARRAY		0
#define DEM_DATA_OUTPUT_WCSDEM		1
#define DEM_DATA_OUTPUT_ZBUF		2
#define DEM_DATA_OUTPUT_COLORMAP	3
#define DEM_DATA_OUTPUT_GRAYIFF		4
#define DEM_DATA_OUTPUT_COLORIFF	5
#define DEM_DATA_UNITS_KILOM		0	
#define DEM_DATA_UNITS_METERS		1
#define DEM_DATA_UNITS_CENTIM		2
#define DEM_DATA_UNITS_MILES		3
#define DEM_DATA_UNITS_FEET		4
#define DEM_DATA_UNITS_INCHES		5
#define DEM_DATA_UNITS_OTHER		6

/* moved to WCS.h 
struct ILBMHeader {
 UBYTE ChunkID[4];
 LONG ChunkSize;
};

struct WcsBitMapHeader {
 USHORT Width, Height;
 SHORT XPos, YPos;
 UBYTE Planes, Masking, Compression, Pad;
 USHORT Transparent;
 UBYTE XAspect, YAspect;
 SHORT PageWidth, PageHeight;
};

struct ZBufferHeader {
 ULONG  Width, Height;
 USHORT VarType, Compression, Sorting, Units;
 float  Min, Max, Bkgrnd, ScaleFactor, ScaleBase;
};*/

void Make_DC_Window(void)
{
 short i;
 long open;
 static const char *InputCycle[] =
	 {"Binary Array", "WCS DEM", "Z Buffer", "Ascii Array", "Vista DEM",
	"IFF", "DTED", NULL};
 static const char *OutputCycle[] =
	 {"Bin Array", "WCS DEM", "Z Buffer", "Color Map", "Gray IFF", "Color IFF", NULL};
 static const char *FormatCycle[] =
	 {"Signed Int", "Unsigned Int", "Floating Pt",/* "Unknown",*/ NULL};
 static const char *ValSizeCycle[] =
	 {"One", "Two", "Four", "Eight",/* "Unknown",*/ NULL};
 static const char *ByteOrderCycle[] = {"High-Low", "Low-High",/* "Unknown",*/ NULL};
 static const char *ReadOrderCycle[] = {"By Row", "By Column", NULL};
 static const char *RowCycle[] = {"Latitude", "Longitude", NULL};
 static const char *UnitCycle[] = {"Kilometers", "Meters", "Centimeters",
	"Miles", "Feet", "Inches", "Other", NULL};
 static const char *ScalePages[] = {"\0334Two Value Equivalence",
			 "\0334One Value Equivalence", "\0334Max-Min Stretch", NULL};
 static const char *ScaleCycle[] = {"Max Out", "Min Out", "I/O Scale", NULL};
 static const char *ProcessPages[] = {"\0334DEM Registration", "\0334Value Format & Sampling", NULL};
 static const char *InputPages[] = {"\0334Value Format", "\0334Pre-Process", NULL};

 if (DC_Win)
  {
  DoMethod(DC_Win->ConvertWin, MUIM_Window_ToFront);
  set(DC_Win->ConvertWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((DC_Win = (struct DEMConvertWindow *)
	get_Memory(sizeof (struct DEMConvertWindow), MEMF_CLEAR)) == NULL)
   return;
 if ((DC_Win->DEMData = (struct DEMConvertData *)
	get_Memory(sizeof (struct DEMConvertData), MEMF_CLEAR)) == NULL)
   {
   Close_DC_Window();
   return;
   } /* if no memory */

  Set_Param_Menu(10);

     DC_Win->ConvertWin = WindowObject,
      MUIA_Window_Title		, "DEM Converter",
      MUIA_Window_ID		, MakeID('D','O','C','V'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, HGroup,
/* Input */
	  Child, VGroup,
	    Child, HGroup,
	      Child, Label1(" Input Format"),
	      Child, DC_Win->Cycle[0] = CycleObject,
		 MUIA_Cycle_Entries, InputCycle, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, DC_Win->BT_GetFile = ImageButton(MUII_Disk),
	      Child, DC_Win->FileNameStr = StringObject, StringFrame, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label1("Input File Size"),
	      Child, DC_Win->FileSizeTxt = TextObject, TextFrame, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label1("Header Bytes"),
	      Child, DC_Win->FormatIntStr[0] = StringObject, StringFrame,
		MUIA_String_Accept, "0123456789", End,
	      End, /* HGroup */
	    Child, RegisterGroup(InputPages),
	      Child, VGroup,
	        Child, HGroup,
	          Child, Label1("Value Format"),
	          Child, DC_Win->Cycle[2] = CycleObject,
		 	MUIA_Cycle_Entries, FormatCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1(" Value Bytes"),
	          Child, DC_Win->Cycle[3] = CycleObject,
		 	MUIA_Cycle_Entries, ValSizeCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1("  Byte Order"),
	          Child, DC_Win->Cycle[4] = CycleObject,
		 	MUIA_Cycle_Entries, ByteOrderCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1("  Read Order"),
	          Child, DC_Win->Cycle[5] = CycleObject,
		 	MUIA_Cycle_Entries, ReadOrderCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1("  Rows Equal"),
	          Child, DC_Win->Cycle[6] = CycleObject,
		 	MUIA_Cycle_Entries, RowCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1("  Data Units"),
	          Child, DC_Win->Cycle[7] = CycleObject,
			MUIA_Cycle_Active, 1,
		 	MUIA_Cycle_Entries, UnitCycle, End,
	          End, /* HGroup */
		End, /*VGroup */
	      Child, VGroup,
		Child, HGroup,
	          Child, Label2("  Floor"),
	          Child, DC_Win->FloorCeilingCheck[0] = CheckMark(0),
	          Child, DC_Win->FloatStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567890",
			MUIA_String_Accept, "+-.0123456789", End,
	          End, /* HGroup */
		Child, HGroup,
	          Child, Label2("Ceiling"),
	          Child, DC_Win->FloorCeilingCheck[1] = CheckMark(0),
	          Child, DC_Win->FloatStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567890",
			MUIA_String_Accept, "+-.0123456789", End,
	          End, /* HGroup */
		Child, RowGroup(4),
	          Child, Label2("Crop Left"),
	          Child, DC_Win->CropStr[0] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,

	          Child, Label2("    Right"),
	          Child, DC_Win->CropStr[1] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,

	          Child, Label2(" Crop Top"),
	          Child, DC_Win->CropStr[2] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,

	          Child, Label2("   Bottom"),
	          Child, DC_Win->CropStr[3] = StringObject, StringFrame,
			MUIA_FixWidthTxt, "012345",
			MUIA_String_Accept, "0123456789", End,
	          End, /* RowGroup */
		End, /* VGroup */
	      End, /* RegisterGroup */

	    Child, HGroup,
	      Child, Label2("Input Cols"),
	      Child, DC_Win->FormatIntStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123456", End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("Input Rows"),
	      Child, DC_Win->FormatIntStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123456", End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("Wrap Longitude"),
	      Child, DC_Win->WrapCheck = CheckMark(0),
	      End, /* HGroup */
	    Child, HGroup,
	      Child, DC_Win->BT_Test = KeyButtonFunc('T', "\33cTest"),
	      Child, Label2("Min"),
	      Child, DC_Win->MinValTxt = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234567", End,
	      Child, Label2("Max"),
	      Child, DC_Win->MaxValTxt = TextObject, TextFrame,
			MUIA_FixWidthTxt, "01234567", End,
	      End, /* HGroup */
	    Child, RectangleObject, End,
	    End, /* VGroup */
/* Spacer */
	  Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, End,
/* Output */
	  Child, VGroup,
	    Child, HGroup,
	      Child, Label1("Output Format"),
	      Child, DC_Win->Cycle[1] = CycleObject,
		 	MUIA_Cycle_Entries, OutputCycle,
			MUIA_Cycle_Active, 1, End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("Out Dir"),
	      Child, DC_Win->OutDirStr = StringObject, StringFrame,
			MUIA_String_Contents, dirname, End,
	      Child, DC_Win->BT_GetOutDir = ImageButton(MUII_Disk),
	      End, /* HGroup */
	    Child, HGroup,
/*	      Child, DC_Win->OutputDBaseCheck = CheckMark(0),*/
	      Child, Label2("Name"),
	      Child, DC_Win->DBaseNameStr = StringObject, StringFrame,
			MUIA_FixWidthTxt, "01234567890",
			MUIA_String_Reject, ":/*#`%?", End,
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("DEMs Row-Wise E/W"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, DC_Win->OutputMapStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789", End,
	        Child, DC_Win->OutputMapArrow[0][0] = ImageButton(MUII_ArrowLeft),
	        Child, DC_Win->OutputMapArrow[0][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */
	    Child, HGroup,
	      Child, Label2("  Column-Wise N/S"),
	      Child, HGroup, MUIA_Group_HorizSpacing, 0,
	        Child, DC_Win->OutputMapStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789", End,
	        Child, DC_Win->OutputMapArrow[1][0] = ImageButton(MUII_ArrowLeft),
	        Child, DC_Win->OutputMapArrow[1][1] = ImageButton(MUII_ArrowRight),
	        End, /* HGroup */
	      End, /* HGroup */

	    Child, RegisterGroup(ProcessPages),
	      Child, VGroup,
	        Child, HGroup,
	          Child, Label2("High Lat"),
	          Child, DC_Win->LatScaleStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "0000.000000", End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2(" Low Lat"),
	          Child, DC_Win->LatScaleStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "0000.000000", End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2("High Lon"),
	          Child, DC_Win->LatScaleStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "0000.000000", End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2(" Low Lon"),
	          Child, DC_Win->LatScaleStr[3] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "0000.000000", End,
	          End, /* HGroup */
		End, /* VGroup */

	      Child, VGroup,
	        Child, HGroup,
	          Child, Label1("Value Format"),
	          Child, DC_Win->Cycle[8] = CycleObject,
			MUIA_Cycle_Entries, FormatCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label1(" Value Bytes"),
	          Child, DC_Win->Cycle[9] = CycleObject,
	 		MUIA_Cycle_Entries, ValSizeCycle, End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2(" Output Cols"),
	          Child, DC_Win->FormatIntStr[4] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123456", End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2(" Output Rows"),
	          Child, DC_Win->FormatIntStr[3] = StringObject, StringFrame,
			MUIA_String_Accept, "0123456789",
			MUIA_FixWidthTxt, "0123456", End,
	          End, /* HGroup */
	        Child, HGroup,
	          Child, Label2("Spline Constraint"),
	          Child, DC_Win->ConstraintCheck = CheckMark(0),
		  End, /* HGroup */
	        End, /* VGroup */

	      End, /* RegisterGroup */

	    Child, DC_Win->VSRegister = RegisterGroup(ScalePages),
	      Child, RowGroup(3),
		Child, RectangleObject, End,
		Child, TextObject, MUIA_Text_Contents, "\0334  Input  ", End,
		Child, TextObject, MUIA_Text_Contents, "\0334  Output ", End,

		Child, Label2("Value 1"),
		Child, DC_Win->VertScaleStr[5] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
		Child, DC_Win->VertScaleStr[6] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,

		Child, Label2("Value 2"),
		Child, DC_Win->VertScaleStr[7] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
		Child, DC_Win->VertScaleStr[8] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
		End, /* RowGroup */

	      Child, VGroup,
		Child, RowGroup(2),
		  Child, RectangleObject, End,
		  Child, TextObject, MUIA_Text_Contents, "\0334  Input  ", End,
		  Child, TextObject, MUIA_Text_Contents, "\0334  Output ", End,

		  Child, Label2("Value 1"),
		  Child, DC_Win->VertScaleStr[2] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
		  Child, DC_Win->VertScaleStr[3] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
		  End, /* RowGroup */
		Child, HGroup,
		  Child, RectangleObject, End,
		  Child, DC_Win->ScaleCycle = CycleObject,
			MUIA_Cycle_Entries, ScaleCycle, End,
		  Child, DC_Win->VertScaleStr[4] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.01", End,
	          End, /* HGroup */
		End, /* VGroup */

	      Child, VGroup,
		Child, HGroup,
		  Child, Label2("Max Out Val"),
		  Child, DC_Win->VertScaleStr[0] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.012", End,
		  End, /* HGroup */
		Child, HGroup,
		  Child, Label2("Min Out Val"),
		  Child, DC_Win->VertScaleStr[1] = StringObject, StringFrame,
			MUIA_String_Accept, "+-.0123456789",
			MUIA_FixWidthTxt, "01235.012", End,
		  End, /* HGroup */
		End, /* VGroup */

	      End, /* RegisterGroup */

	    End, /* VGroup */

	  End, /* HGroup */

        Child, HGroup,
	  Child, RectangleObject, End,
          Child, DC_Win->BT_Convert = KeyButtonFunc('v', "\33cConvert"), 
	  Child, RectangleObject, End,
          End, /* HGroup */
	End, /* VGroup */
      End; /* WindowObject */

  if (! DC_Win->ConvertWin)
   {
   Close_DC_Window();
   User_Message((CONST_STRPTR)"DEM Converter", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DC_Win->ConvertWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Application returnIDs */
  DoMethod(DC_Win->ConvertWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DC_CLOSE);  

  MUI_DoNotiPresFal(app,
   DC_Win->BT_Convert, ID_DC_CONVERT, DC_Win->BT_GetFile, ID_DC_GETFILE,
   DC_Win->BT_GetOutDir, ID_DC_GETOUTDIR, DC_Win->BT_Test, ID_DC_TEST, NULL);

/* Arrow notifications */
  for (i=0; i<2; i++)
   {
   DoMethod(DC_Win->OutputMapArrow[i][0], MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_DC_ARROWLEFT(i));
   DoMethod(DC_Win->OutputMapArrow[i][1], MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_DC_ARROWRIGHT(i));
   } /* for i=0... */

/* set a few values */
  set(DC_Win->OutputMapStr[0], MUIA_String_Integer, 1);
  set(DC_Win->OutputMapStr[1], MUIA_String_Integer, 1);

/* set tab cycle chain */
  DoMethod(DC_Win->ConvertWin, MUIM_Window_SetCycleChain,
   DC_Win->Cycle[0], DC_Win->BT_GetFile, DC_Win->FormatIntStr[0],
   DC_Win->Cycle[2], DC_Win->Cycle[3], DC_Win->Cycle[4], DC_Win->Cycle[5],
   DC_Win->Cycle[6], DC_Win->Cycle[7], DC_Win->FormatIntStr[2],
   DC_Win->FormatIntStr[1], DC_Win->WrapCheck, DC_Win->BT_Test, DC_Win->FloorCeilingCheck[0],
   DC_Win->FloatStr[0], DC_Win->FloorCeilingCheck[1], DC_Win->FloatStr[1],
   DC_Win->CropStr[0], DC_Win->CropStr[1], DC_Win->CropStr[2], DC_Win->CropStr[3],
   DC_Win->Cycle[1], DC_Win->OutDirStr, DC_Win->BT_GetOutDir,
   DC_Win->DBaseNameStr, DC_Win->OutputMapStr[0], DC_Win->OutputMapStr[1],
   DC_Win->LatScaleStr[0], DC_Win->LatScaleStr[1], DC_Win->LatScaleStr[2],
   DC_Win->LatScaleStr[3], DC_Win->Cycle[8], DC_Win->Cycle[9],
   DC_Win->FormatIntStr[4], DC_Win->FormatIntStr[3],
   DC_Win->VertScaleStr[0], DC_Win->VertScaleStr[1],
   DC_Win->VertScaleStr[2], DC_Win->VertScaleStr[3], DC_Win->ScaleCycle,
   DC_Win->VertScaleStr[4], DC_Win->VertScaleStr[5], DC_Win->VertScaleStr[6],
   DC_Win->VertScaleStr[7], DC_Win->VertScaleStr[8], DC_Win->BT_Convert,
   NULL);

/* set return cycle chain */
 DoMethod(DC_Win->FormatIntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[2]); 
 DoMethod(DC_Win->FormatIntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[1]); 
 DoMethod(DC_Win->FormatIntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->OutDirStr); 
 DoMethod(DC_Win->OutDirStr,    MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->DBaseNameStr); 
 DoMethod(DC_Win->DBaseNameStr,    MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->OutputMapStr[0]); 
 DoMethod(DC_Win->OutputMapStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->OutputMapStr[1]); 
 DoMethod(DC_Win->OutputMapStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->LatScaleStr[0]); 
 DoMethod(DC_Win->LatScaleStr[0],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->LatScaleStr[1]); 
 DoMethod(DC_Win->LatScaleStr[1],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->LatScaleStr[2]); 
 DoMethod(DC_Win->LatScaleStr[2],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->LatScaleStr[3]); 
 DoMethod(DC_Win->LatScaleStr[3],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[5]); 

 DoMethod(DC_Win->FormatIntStr[4],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[3]); 
 DoMethod(DC_Win->FormatIntStr[3],  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[5]); 

 DoMethod(DC_Win->VertScaleStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[1]); 
 DoMethod(DC_Win->VertScaleStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[0]); 

 DoMethod(DC_Win->VertScaleStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[3]); 
 DoMethod(DC_Win->VertScaleStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[4]); 
 DoMethod(DC_Win->VertScaleStr[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[0]); 

 DoMethod(DC_Win->VertScaleStr[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[6]); 
 DoMethod(DC_Win->VertScaleStr[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[7]); 
 DoMethod(DC_Win->VertScaleStr[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->VertScaleStr[8]); 
 DoMethod(DC_Win->VertScaleStr[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[0]); 

 DoMethod(DC_Win->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FloatStr[1]); 
 DoMethod(DC_Win->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->CropStr[0]); 
 DoMethod(DC_Win->CropStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->CropStr[1]); 
 DoMethod(DC_Win->CropStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->CropStr[2]); 
 DoMethod(DC_Win->CropStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->CropStr[3]); 
 DoMethod(DC_Win->CropStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DC_Win->ConvertWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DC_Win->FormatIntStr[2]); 

/* set active gadget */
 set(DC_Win->ConvertWin, MUIA_Window_ActiveObject, DC_Win->Cycle[0]); 

 strcpy(DC_Win->InPath, dirname);

/* Open window */
  set(DC_Win->ConvertWin, MUIA_Window_Open, TRUE);
  get(DC_Win->ConvertWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DC_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DC_Win->ConvertWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DC_ACTIVATE);

/* Get Window structure pointer */
  get(DC_Win->ConvertWin, MUIA_Window_Window, &DC_Win->Win);

} /* Open_DC_Window() */

/**********************************************************************/

void Close_DC_Window(void)
{

 if (DC_Win)
  {
  if (DC_Win->ConvertWin)
   {
   set(DC_Win->ConvertWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, DC_Win->ConvertWin);
   MUI_DisposeObject(DC_Win->ConvertWin);
   } /* if window created */
  if (DC_Win->DEMData) free_Memory(DC_Win->DEMData, sizeof (struct DEMConvertData));
  free_Memory(DC_Win, sizeof (struct DEMConvertWindow));
  DC_Win = NULL;
  } /* if memory allocated */

} /* Close_DC_Window() */

/*********************************************************************/

void Handle_DC_Window(ULONG WCS_ID)
{
 short i;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_DC_Window();
   return;
   } /* Open  Window */

  if (! DC_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
/*    DoMethod(DC_Win->ConvertWin, MUIM_Window_ToFront);*/
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate DEM Convert window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_DC_GETFILE:
      {
      Get_DC_InputFile();
      break;
      } /* check file size DEM */
     case ID_DC_GETOUTDIR:
      {
      char dirpath[256], filename[32], *CurStr;

      get(DC_Win->OutDirStr, MUIA_String_Contents, &CurStr);
      strcpy(dirpath, CurStr);
      get(DC_Win->DBaseNameStr, MUIA_String_Contents, &CurStr);
      strcpy(filename, CurStr);
      getfilename(1, "Output Directory", dirpath, filename);
      set(DC_Win->OutDirStr, MUIA_String_Contents, dirpath);
      set(DC_Win->DBaseNameStr, MUIA_String_Contents, filename);
      break;
      } /* check file size DEM */
     case ID_DC_TEST:
      {
      char *filename;

      get(DC_Win->FileNameStr, MUIA_String_Contents, &filename);
      Set_DC_Data();
      SetPointer(DC_Win->Win, WaitPointer, 16, 16, -6, 0);
      ConvertDEM(DC_Win->DEMData, filename, 1);
      ClearPointer(DC_Win->Win);
      if (abs(DC_Win->DEMData->MaxMin[0]) < 9999.9)
       sprintf(str, "%f", DC_Win->DEMData->MaxMin[0]);
      else
       {
       if (DC_Win->DEMData->MaxMin[0] > 0.0)
        sprintf(str, "%1.2lE", DC_Win->DEMData->MaxMin[0]);
       else
        sprintf(str, "%1.1lE", DC_Win->DEMData->MaxMin[0]);
       }
      set(DC_Win->MinValTxt, MUIA_Text_Contents, str);
      if (abs(DC_Win->DEMData->MaxMin[1]) < 9999.9)
       sprintf(str, "%f", DC_Win->DEMData->MaxMin[1]);
      else
       {
       if (DC_Win->DEMData->MaxMin[1] > 0.0)
        sprintf(str, "%1.2lE", DC_Win->DEMData->MaxMin[1]);
       else
        sprintf(str, "%1.1lE", DC_Win->DEMData->MaxMin[1]);
       }
      set(DC_Win->MaxValTxt, MUIA_Text_Contents, str);
      break;
      } /* convert DEM */
     case ID_DC_CONVERT:
      {
      char *filename;

      get(DC_Win->FileNameStr, MUIA_String_Contents, &filename);
      Set_DC_Data();
      SetPointer(DC_Win->Win, WaitPointer, 16, 16, -6, 0);
      ConvertDEM(DC_Win->DEMData, filename, 0);
      ClearPointer(DC_Win->Win);
      break;
      } /* convert DEM */
     case ID_DC_CLOSE:
      {
      Close_DC_Window();
      break;
      } /* close */
     } /* switch Gadget ID */
    break;
    } /* BUTTONS1 */

   case GP_ARROW1:
    {
    LONG data;

    i = WCS_ID - ID_DC_ARROWLEFT(0);
    get(DC_Win->OutputMapStr[i], MUIA_String_Integer, &data);
    if (data > 1) set(DC_Win->OutputMapStr[i], MUIA_String_Integer, (data - 1));
    break;
    } /* ARROW1 */

   case GP_ARROW2:
    {
    LONG data;

    i = WCS_ID - ID_DC_ARROWRIGHT(0);
    get(DC_Win->OutputMapStr[i], MUIA_String_Integer, &data);
    set(DC_Win->OutputMapStr[i], MUIA_String_Integer, (data + 1));
    break;
    } /* ARROW2 */

   } /* switch Gadget Group */

} /* Handle_DC_Window() */

/***********************************************************************/

STATIC_FCN void Get_DC_InputFile(void) // used locally only -> static, AF 25.7.2021
{
 char filename[255];
 long filesize, fh, data, headersize;

 if (! getfilename(0, "File to Convert", DC_Win->InPath, DC_Win->InFile))
  return;

 strmfp(filename, DC_Win->InPath, DC_Win->InFile);
 if ((fh = open(filename, O_RDONLY)) < 0)
  {
  User_Message((CONST_STRPTR)DC_Win->InFile,
		  (CONST_STRPTR)"Unable to open file for input!\n", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(WNG_OPEN_FAIL, (CONST_STRPTR)DC_Win->InFile);
  return;
  } /* if open error */

 if ((filesize = lseek(fh, 0L, 2)) < 0)
  {
  User_Message((CONST_STRPTR)DC_Win->InFile,
		  (CONST_STRPTR)"Unable to read file size!\n", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(WNG_READ_FAIL, (CONST_STRPTR)DC_Win->InFile);
  goto EndCheck;
  } /* if file size read fail */
 else
  {
  set(DC_Win->FileNameStr, MUIA_String_Contents, filename);
  sprintf(str, "%ld", filesize);
  set(DC_Win->FileSizeTxt, MUIA_Text_Contents, str);
  } /* else file size read OK */

 get(DC_Win->Cycle[0], MUIA_Cycle_Active, &data);

 lseek(fh, 0L, 0);

 switch (data)
  {
  case DEM_DATA_INPUT_WCSDEM: 		/* supposedly WCS DEM input file */
   {
   struct elmapheaderV101 Hdr;
   float Version;

   read(fh, (char *)&Version, sizeof (float));

   if (abs(Version - 1.00) < .0001)
    {
    headersize = ELEVHDRLENV100 + sizeof (float);
    }
   else if (abs(Version - 1.01) < .0001 || abs(Version - 1.02) < .0001)
    {
    headersize = ELEVHDRLENV101 + sizeof (float);
    }
   else
    {
    Version = 0.0;
    lseek(fh, 0L, 0);
    headersize = ELEVHDRLENV100;
    }

   set(DC_Win->FormatIntStr[0], MUIA_String_Integer, headersize);
   read(fh, (char *)&Hdr, ELEVHDRLENV101);
   set(DC_Win->FormatIntStr[1], MUIA_String_Integer, (Hdr.rows + 1)); 
   set(DC_Win->FormatIntStr[2], MUIA_String_Integer, Hdr.columns);
   set(DC_Win->Cycle[3], MUIA_Cycle_Active, 1); /* value size */
   set(DC_Win->Cycle[2], MUIA_Cycle_Active, 0); /* value format */
   set(DC_Win->Cycle[4], MUIA_Cycle_Active, 0); /* value byte order */
   set(DC_Win->Cycle[5], MUIA_Cycle_Active, 1); /* read order */
   set(DC_Win->Cycle[6], MUIA_Cycle_Active, 1); /* rows equal */
   if (abs (Hdr.elscale - ELSCALE_KILOM) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 0); /* data units km */
   else if (abs (Hdr.elscale - ELSCALE_METERS) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 1); /* data units meters */
   else if (abs (Hdr.elscale - ELSCALE_CENTIM) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 2); /* data units centimeters */
   else if (abs (Hdr.elscale - ELSCALE_MILES) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 3); /* data units miles */
   else if (abs (Hdr.elscale - ELSCALE_FEET) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 4); /* data units feet */
   else if (abs (Hdr.elscale - ELSCALE_INCHES) < .0001)
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 5); /* data units inches */
   else
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, 6); /* data units other */

   if (Version == 0.0)
    {
    Hdr.columns += 2;
    } /* if old file version with padding */
   if (filesize != headersize + (Hdr.rows + 1) * Hdr.columns * sizeof (short))
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert",
    		(CONST_STRPTR)"Warning!\nFile is not a WCS DEM file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    } /* if not WCS DEM */
   break;
   } /* if possibly WCS DEM */
  case DEM_DATA_INPUT_ZBUF:
   {
   long Ptr = 0L, ValSize, Signed;
   struct ILBMHeader Hdr;
   struct ZBufferHeader ZBHdr;

   if (CheckIFF(fh, &Hdr))
    {
    if (FindIFFChunk(fh, &Hdr, "ZBUF"))
     {
     if (read(fh, &ZBHdr, sizeof (struct ZBufferHeader)) == sizeof (struct ZBufferHeader))
      {
      if (FindIFFChunk(fh, &Hdr, "ZBOD"))
       {
       Ptr = tell(fh);
       }
      }
     }
    }
   if (Ptr)
    {
    set(DC_Win->FormatIntStr[0], MUIA_String_Integer, Ptr);
/* rows */
    set(DC_Win->FormatIntStr[1], MUIA_String_Integer, ZBHdr.Height);
/* columns */
    set(DC_Win->FormatIntStr[2], MUIA_String_Integer, ZBHdr.Width);
    switch (ZBHdr.VarType)
     {
     case 0:	/* BYTE */
      {
      ValSize = 0;
      Signed = 0;
      break;
      }
     case 1:	/* UBYTE */
      {
      ValSize = 0;
      Signed = 1;
      break;
      }
     case 2:	/* SHORT */
      {
      ValSize = 1;
      Signed = 0;
      break;
      }
     case 3:	/* USHORT */
      {
      ValSize = 1;
      Signed = 1;
      break;
      }
     case 4:	/* LONG */
      {
      ValSize = 2;
      Signed = 0;
      break;
      }
     case 5:	/* ULONG */
      {
      ValSize = 2;
      Signed = 1;
      break;
      }
     case 6:	/* FLOAT */
      {
      ValSize = 2;
      Signed = 2;
      break;
      }
     case 7:	/* DOUBLE */
      {
      ValSize = 3;
      Signed = 2;
      break;
      }
     } /* switch */
    set(DC_Win->Cycle[2], MUIA_Cycle_Active, Signed);	/* value format */
    set(DC_Win->Cycle[3], MUIA_Cycle_Active, ValSize);	/* value size */
    set(DC_Win->Cycle[4], MUIA_Cycle_Active, 0);	/* value byte order */
    set(DC_Win->Cycle[5], MUIA_Cycle_Active, 0);	/* read order */
    switch (ZBHdr.Units)
     {
     case 0:
      {
      data = 6;
      break;
      } /* dimensionless */
     case 1:
      {
      data = 6;
      break;
      } /* millimeters */
     case 2:
      {
      data = 1;
      break;
      } /* meters */
     case 3:
      {
      data = 0;
      break;
      } /* kilometers */
     case 4:
      {
      data = 5;
      break;
      } /* inches */
     case 5:
      {
      data = 4;
      break;
      } /* feet */
     case 6:
      {
      data = 6;
      break;
      } /* yards */
     case 7:
      {
      data = 3;
      break;
      } /* miles */
     case 8:
      {
      data = 6;
      break;
      } /* light years */
     case 9:
      {
      data = 6;
      break;
      } /* undefined */
     } /* switch */
    set(DC_Win->Cycle[7], MUIA_Cycle_Active, data);	/* read order */
    if (abs(ZBHdr.Min) < 9999.9)
     sprintf(str, "%f", ZBHdr.Min);
    else if (ZBHdr.Min > 0.0)
     sprintf(str, "%1.2lE", ZBHdr.Min);
    else
     sprintf(str, "%1.1lE", ZBHdr.Min);
    set(DC_Win->MinValTxt, MUIA_Text_Contents, str);
    if (abs(ZBHdr.Max) < 9999.9)
     sprintf(str, "%f", ZBHdr.Max);
    else if (ZBHdr.Max > 0.0)
     sprintf(str, "%1.2lE", ZBHdr.Max);
    else
     sprintf(str, "%1.1lE", ZBHdr.Max);
    set(DC_Win->MaxValTxt, MUIA_Text_Contents, str);
    }
   else
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert",
    		(CONST_STRPTR)"Warning!\nFile is not an IFF Z Buffer file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    }
   break;
   } /* if possibly an  IFF Z Buffer file */
  case DEM_DATA_INPUT_VISTA:
   {
   unsigned char id[32], *LatStr, *LonStr;
   unsigned char name[32];
   long Compression, HeaderType, DataPts;

   read(fh, id, 32);
   id[31] = 0;
   if (strcmp(id, "Vista DEM File"))
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Warning\nFile is not a Vista DEM file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* if no file id match */
   read(fh, name, 32);
   name[9] = 0;
   lseek(fh, 128, 0);
   read(fh, &Compression, 4);
   read(fh, &HeaderType, 4);
   if (! Compression)
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Warning\nFile is not a compressed Vista file and cannot be imported.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    }
   if (HeaderType)
    {
    HeaderType = User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Is this a Small, Large or Huge Vista file?", (CONST_STRPTR)"Small|Large|Huge", (CONST_STRPTR)"slh");
    }
   else
    HeaderType = 1;
   switch (HeaderType)
    {
    case 0:		/* Huge */
     LatStr = "0.276901";
     LonStr = "180.276901";
     DataPts = 1026;
     break;
    case 1:		/* Small */
     LatStr = "0.069428";
     LonStr = "180.069428";
     DataPts = 258;
     break;
    case 2:		/* Large */
     LatStr = "0.138586";
     LonStr = "180.13858";
     DataPts = 514;
     break;
    }
   set(DC_Win->DBaseNameStr, MUIA_String_Contents, name);
   set(DC_Win->FormatIntStr[2], MUIA_String_Integer, DataPts);
   set(DC_Win->FormatIntStr[1], MUIA_String_Integer, DataPts);
   set(DC_Win->LatScaleStr[0], MUIA_String_Contents, LatStr);
   set(DC_Win->LatScaleStr[1], MUIA_String_Contents, "0.0");
   set(DC_Win->LatScaleStr[2], MUIA_String_Contents, LonStr);
   set(DC_Win->LatScaleStr[3], MUIA_String_Contents, "180.0");
   set(DC_Win->Cycle[3], MUIA_Cycle_Active, 1); /* value size */
   set(DC_Win->Cycle[2], MUIA_Cycle_Active, 0); /* value format */
   set(DC_Win->Cycle[4], MUIA_Cycle_Active, 0); /* value byte order */
   set(DC_Win->Cycle[5], MUIA_Cycle_Active, 0); /* read order */
   set(DC_Win->Cycle[6], MUIA_Cycle_Active, 0); /* rows equal */
   set(DC_Win->Cycle[7], MUIA_Cycle_Active, 1); /* data units meters */
   break;
   } /* vista DEM */
  case DEM_DATA_INPUT_IFF:
   {
   struct ILBMHeader Hdr;
   struct WcsBitMapHeader BMHdr;

   if (! CheckIFF(fh, &Hdr))
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Warning\nFile is not an IFF file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* not IFF file */
   if (! FindIFFChunk(fh, &Hdr, "BMHD"))
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Warning\nFile is not an IFF image file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* not IFF image file */
   if ((read(fh, (char *)&BMHdr, sizeof (struct WcsBitMapHeader))) !=
	 sizeof (struct WcsBitMapHeader))
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Error reading bitmap header.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* read error */
   set(DC_Win->FormatIntStr[0], MUIA_String_Integer, 0);
   set(DC_Win->FormatIntStr[2], MUIA_String_Integer, BMHdr.Width);
   set(DC_Win->FormatIntStr[1], MUIA_String_Integer, BMHdr.Height);
   set(DC_Win->Cycle[3], MUIA_Cycle_Active, 0); /* value size */
   set(DC_Win->Cycle[2], MUIA_Cycle_Active, 1); /* value format */
   set(DC_Win->Cycle[4], MUIA_Cycle_Active, 0); /* value byte order */
   set(DC_Win->Cycle[5], MUIA_Cycle_Active, 0); /* read order */
   set(DC_Win->Cycle[6], MUIA_Cycle_Active, 0); /* rows equal */
   set(DC_Win->Cycle[7], MUIA_Cycle_Active, 1); /* data units meters */
   break;
   } /* IFF */
  case DEM_DATA_INPUT_DTED:
   {
   char test[16];
   long StartPt = 0, ct;
   double Coord;

   lseek(fh, 160, 0);
   read(fh, test, 3);
   test[3] = 0;
   if (strcmp(test, "DSI"))
    {
    lseek(fh, 80, 0);
    read(fh, test, 3);
    test[3] = 0;
    if (! strcmp(test, "DSI"))
     StartPt = -80;
    else
     {
     StartPt = -30000;
     lseek(fh, 0, 0);
     for (ct=0; ct<20000; ct++)
      {
      read(fh, test, 1);
      if (test[0] == 'D')
       {
       read(fh, test, 1);
       if (test[0] == 'S')
        {
        read(fh, test, 1);
        if (test[0] == 'I')
         {
         StartPt = tell(fh) - 163;
         break;
         } /* if */
        } /* if */
       } /* if */
      } /* for col=0... */
     } /* else look for DSI */
    } /* if */
   if (StartPt <= -30000)
    {
    User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
    		(CONST_STRPTR)"Warning\nFile is not recognized as a DTED file.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* if */

   lseek(fh, StartPt + 364, 0);
   read(fh, test, 7);
   test[7] = 0;
   Coord = DegMinSecToDegrees(test);
   if (test[6] == 'S')
    {
    Coord = -Coord;
    }
   sprintf(str, "%f", Coord);
   set(DC_Win->LatScaleStr[1], MUIA_String_Contents, str);

   read(fh, test, 8);
   test[8] = 0;
   Coord = DegMinSecToDegrees(test);
   if (test[7] == 'E')
    {
    Coord = -Coord;
    }
   sprintf(str, "%f", Coord);
   set(DC_Win->LatScaleStr[2], MUIA_String_Contents, str);

   read(fh, test, 7);
   test[7] = 0;
   Coord = DegMinSecToDegrees(test);
   if (test[6] == 'S')
    {
    Coord = -Coord;
    }
   sprintf(str, "%f", Coord);
   set(DC_Win->LatScaleStr[0], MUIA_String_Contents, str);

   lseek(fh, StartPt + 401, 0);
   read(fh, test, 8);
   test[8] = 0;
   Coord = DegMinSecToDegrees(test);
   if (test[7] == 'E')
    {
    Coord = -Coord;
    }
   sprintf(str, "%f", Coord);
   set(DC_Win->LatScaleStr[3], MUIA_String_Contents, str);

   lseek(fh, StartPt + 441, 0);
   read(fh, test, 4);
   test[4] = 0;
   set(DC_Win->FormatIntStr[1], MUIA_String_Contents, test);

   read(fh, test, 4);
   set(DC_Win->FormatIntStr[2], MUIA_String_Contents, test);
   
   set(DC_Win->Cycle[3], MUIA_Cycle_Active, 1); /* value size */
   set(DC_Win->Cycle[2], MUIA_Cycle_Active, 0); /* value format */
   set(DC_Win->Cycle[4], MUIA_Cycle_Active, 0); /* value byte order */
   set(DC_Win->Cycle[5], MUIA_Cycle_Active, 1); /* read order */
   set(DC_Win->Cycle[6], MUIA_Cycle_Active, 0); /* rows equal */
   set(DC_Win->Cycle[7], MUIA_Cycle_Active, 1); /* data units meters */
   break;
   } /* DTED */
  } /* switch */

EndCheck:

 close(fh);

} /* Set_DC_InputFile() */

/***********************************************************************/

/* For converting a coordinate string of the form dddmmss to decimal degrees */

STATIC_FCN double DegMinSecToDegrees(char *str) // used locally only -> static, AF 25.7.2021
{
long Deg, Min, DegMinSec;

 DegMinSec = atoi(str);

 Deg = DegMinSec / 10000;
 DegMinSec -= Deg * 10000;
 Min = DegMinSec / 100;
 DegMinSec -= Min / 100;

 return ((double)Deg + (double)Min / 60.0 + (double)DegMinSec / 3600.0);
 
} /* DegMinSecToDegrees() */

/***********************************************************************/

STATIC_FCN void Set_DC_Data(void) // used locally only -> static, AF 25.7.2021
{
 short i;
 char *floatdata;
 LONG data;

 for (i=0; i<10; i++)
  {
  get(DC_Win->Cycle[i], MUIA_Cycle_Active, &data);
  DC_Win->DEMData->FormatCy[i] = data;
  } /* for i=0... */
 for (i=0; i<5; i++)
  {
  get(DC_Win->FormatIntStr[i], MUIA_String_Integer, &data);
  DC_Win->DEMData->FormatInt[i] = data;
  } /* for i=0... */
 if (DC_Win->DEMData->FormatCy[0] == DEM_DATA_INPUT_WCSDEM)
  DC_Win->DEMData->WrapLon = 0;
 else
  {
  get(DC_Win->WrapCheck, MUIA_Selected, &data);
  DC_Win->DEMData->WrapLon = data;
  } /* if not WCSDEM */
 for (i=0; i<2; i++)
  {
  get(DC_Win->OutputMapStr[i], MUIA_String_Integer, &data);
  DC_Win->DEMData->OutputMaps[i] = data;
  get(DC_Win->FloorCeilingCheck[i], MUIA_Selected, &data);
  DC_Win->DEMData->ActiveFC[i] = data;
  get(DC_Win->FloatStr[i], MUIA_String_Contents, &floatdata);
  DC_Win->DEMData->FloorCeiling[i] = atof(floatdata);
  } /* for i=0... */
 get(DC_Win->ConstraintCheck, MUIA_Selected, &data);
 DC_Win->DEMData->SplineConstrain = data;
 for (i=0; i<4; i++)
  {
  get(DC_Win->LatScaleStr[i], MUIA_String_Contents, &floatdata);
  DC_Win->DEMData->LateralScale[i] = atof(floatdata);
  get(DC_Win->CropStr[i], MUIA_String_Integer, &data);
  DC_Win->DEMData->Crop[i] = data;
  } /* for i=0... */
 for (i=0; i<9; i++)
  {
  get(DC_Win->VertScaleStr[i], MUIA_String_Contents, &floatdata);
  DC_Win->DEMData->VertScale[i] = atof(floatdata);
  } /* for i=0... */
 get(DC_Win->ScaleCycle, MUIA_Cycle_Active, &data);
 DC_Win->DEMData->ScaleType = data;
 get(DC_Win->VSRegister, MUIA_Group_ActivePage, &data);
 DC_Win->DEMData->VSOperator = data;
 get(DC_Win->DBaseNameStr, MUIA_String_Contents, &floatdata);
 strncpy(DC_Win->DEMData->NameBase, floatdata, 24);
 DC_Win->DEMData->NameBase[23] = '\0';
 get(DC_Win->OutDirStr, MUIA_String_Contents, &floatdata);
 strcpy(DC_Win->DEMData->OutputDir, floatdata);

} /* Set_DC_Data() */

/********************************************************************/

void Make_DI_Window(void)
{
 long open;

 if (DI_Win)
  {
  DoMethod(DI_Win->InterpWin, MUIM_Window_ToFront);
  set(DI_Win->InterpWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if (! dbaseloaded)
  {
  NoLoad_Message((CONST_STRPTR)"Data Ops: Interp DEM", (CONST_STRPTR)"a Database");
  return;
  }

 if ((DI_Win = (struct DEMInterpolateWindow *)
	get_Memory(sizeof (struct DEMInterpolateWindow), MEMF_CLEAR)) == NULL)
   return;
 if ((DI_Win->DEMInterp = (struct DEMInterpolateData *)
	get_Memory(sizeof (struct DEMInterpolateData), MEMF_CLEAR)) == NULL)
   {
   Close_DI_Window();
   return;
   } /* if no memory */

  Set_Param_Menu(10);

     DI_Win->InterpWin = WindowObject,
      MUIA_Window_Title		, "DEM Interpolate",
      MUIA_Window_ID		, MakeID('D','O','I','N'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, DI_Win->DirTxt = TextObject, TextFrame, End,
	Child, HGroup,
	  Child, DI_Win->BT_GetFiles = KeyButtonFunc('S', "\33cSelect Files"),
	  Child, Label2(" Selected"),
	  Child, DI_Win->SumFilesTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2("Elevation Var %"),
	  Child, DI_Win->ElVarStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Contents, "2.0",
		MUIA_String_Accept, "-.0123456789", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2("   Max Flat Var"),
	  Child, DI_Win->FlatMaxStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Contents, "2.0",
		MUIA_String_Accept, ".0123456789", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DI_Win->BT_Interpolate = KeyButtonFunc('I', "\33cInterpolate"),
	  Child, RectangleObject, End,
	  End, /* HGroup */
        End, /* VGroup */
      End; /* WindowObject */

  if (! DI_Win->InterpWin)
   {
   Close_DI_Window();
   User_Message((CONST_STRPTR)"DEM Interpolate", (CONST_STRPTR)"Out of memory!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DI_Win->InterpWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(DI_Win->InterpWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DI_CLOSE);  

  MUI_DoNotiPresFal(app,
   DI_Win->BT_Interpolate, ID_DI_INTERP, DI_Win->BT_GetFiles, ID_DI_GETFILES, NULL);  

/* set tab cycle chain */
  DoMethod(DI_Win->InterpWin, MUIM_Window_SetCycleChain,
   DI_Win->BT_GetFiles, DI_Win->ElVarStr, DI_Win->FlatMaxStr,
   DI_Win->BT_Interpolate, NULL);

/* set return cycle chain */
 DoMethod(DI_Win->ElVarStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DI_Win->InterpWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DI_Win->FlatMaxStr); 
 DoMethod(DI_Win->FlatMaxStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DI_Win->InterpWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DI_Win->ElVarStr); 

/* set active gadget */
 set(DI_Win->InterpWin, MUIA_Window_ActiveObject, DI_Win->BT_GetFiles); 
 set(DI_Win->BT_Interpolate, MUIA_Disabled, TRUE);

 strcpy(DI_Win->DEMInterp->elevpath, dirname);
 DI_Win->DEMInterp->pattern = "#?(.elev)";

/* Open window */
  set(DI_Win->InterpWin, MUIA_Window_Open, TRUE);
  get(DI_Win->InterpWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DI_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DI_Win->InterpWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DI_ACTIVATE);

DI_Win->DEMInterp->FrFile = NULL;

} /* Open_DI_Window() */

/********************************************************************/

void Close_DI_Window(void)
{

 if (DI_Win)
  {
  if (DI_Win->InterpWin)
   {
   set(DI_Win->InterpWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, DI_Win->InterpWin);
   MUI_DisposeObject(DI_Win->InterpWin);
   } /* if window created */
  if (DI_Win->DEMInterp)
    {
    if(DI_Win->DEMInterp->FrFile)
     {
     freemultifilename(DI_Win->DEMInterp->FrFile);
     DI_Win->DEMInterp->FrFile = NULL;
     } /* if */
    free_Memory(DI_Win->DEMInterp, sizeof (struct DEMInterpolateData));
    } /* if */
  free_Memory(DI_Win, sizeof (struct DEMInterpolateWindow));
  DI_Win = NULL;
  } /* if memory allocated */

} /* Close_DI_Window() */

/********************************************************************/

void Handle_DI_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_DI_Window();
   return;
   } /* Open DEM Interpolate Window */

  if (! DI_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate DEM Interpolation window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_DI_GETFILES:
      {
      if(DI_Win->DEMInterp->FrFile)
        {
        freemultifilename(DI_Win->DEMInterp->FrFile);
        DI_Win->DEMInterp->FrFile = NULL;
        } /* if */
      if(!(DI_Win->DEMInterp->FrFile = getmultifilename("DEM Files", DI_Win->DEMInterp->elevpath, DI_Win->DEMInterp->elevfile, DI_Win->DEMInterp->pattern)))
       {
       set(DI_Win->SumFilesTxt, MUIA_Text_Contents, "0");
       set(DI_Win->BT_Interpolate, MUIA_Disabled, TRUE);
       break;
       }
      set (DI_Win->DirTxt, MUIA_Text_Contents, DI_Win->DEMInterp->elevpath);
      sprintf(str, "%1ld", DI_Win->DEMInterp->FrFile->rf_NumArgs);
      set(DI_Win->SumFilesTxt, MUIA_Text_Contents, str);
      if (DI_Win->DEMInterp->FrFile->rf_NumArgs > 0)
       set(DI_Win->BT_Interpolate, MUIA_Disabled, FALSE);
      break;
      } /* get file list */
     case ID_DI_INTERP:
      {
      Set_DI_Data();
      InterpDEM(DI_Win->DEMInterp);
      freemultifilename(DI_Win->DEMInterp->FrFile);
      DI_Win->DEMInterp->FrFile = NULL;
      break;
      } /* interpolate */
     case ID_DI_CLOSE:
      {
      Close_DI_Window();
      break;
      } /* close */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */
   } /* switch gadget group */

} /* Handle_DI_Window() */

/**********************************************************************/

STATIC_FCN void Set_DI_Data(void) // used locally only -> static, AF 25.7.2021
{
 char *floatstr;

 get(DI_Win->ElVarStr, MUIA_String_Contents, &floatstr);
 DI_Win->DEMInterp->elvar = .01 * atof(floatstr);
 get(DI_Win->FlatMaxStr, MUIA_String_Contents, &floatstr);
 DI_Win->DEMInterp->flatmax = atof(floatstr);

} /* Set_DI_Data() */

/********************************************************************/
