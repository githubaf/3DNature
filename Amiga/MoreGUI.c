/* MoreGUI.c
** World Construction Set GUI for Data Operations module.
** Copyright 1994 by Gary R. Huber and Chris Eric Hanson.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include "Version.h"

STATIC_VAR short SaveAscii;

STATIC_FCN void ApplyImageScale(void);// used locally only -> static, AF 26.7.2021


void Make_DM_Window(void)
{
 long open;

 if (DM_Win)
  {
  DoMethod(DM_Win->ExtractWin, MUIM_Window_ToFront);
  set(DM_Win->ExtractWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if (! dbaseloaded)
  {
  NoLoad_Message(GetString( MSG_MOREGUI_ADATABASE ), GetString( MSG_AGUI_ADATABASE ));  // "a Database"   "Data Ops: Extract DEM"
  return;
  }

 if ((DM_Win = (struct DEMExtractWindow *)
	get_Memory(sizeof (struct DEMExtractWindow), MEMF_CLEAR)) == NULL)
   return;
 if ((DM_Win->DEMExtract = (struct DEMExtractData *)
	get_Memory(sizeof (struct DEMExtractData), MEMF_CLEAR)) == NULL)
   {
   Close_DM_Window();
   return;
   } /* if no memory */

  Set_Param_Menu(10);

     DM_Win->ExtractWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_MOREGUI_DEMEXTRACT ),  // "DEM Extract"
      MUIA_Window_ID		, MakeID('D','O','E','X'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, DM_Win->BT_GetFiles = KeyButtonFunc('S', (char*)GetString( MSG_MOREGUI_SELECTFILES )),  // "\33cSelect Files"
	  Child, DM_Win->DirTxt = TextObject, TextFrame,
		MUIA_HorizWeight, 300, End,
	  Child, Label2(GetString( MSG_MOREGUI_SELECTED )),  // " Selected"
	  Child, DM_Win->SumFilesTxt = TextObject, TextFrame,
		MUIA_FixWidthTxt, "01234", End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, Label2(GetString( MSG_MOREGUI_SELATITUDE )),  // " SE Latitude"
	  Child, DM_Win->LatStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890",
		MUIA_String_Accept, "-.0123456789", End,
	  Child, Label2(GetString( MSG_MOREGUI_SELONGITUDE )),  // " SE Longitude"
	  Child, DM_Win->LonStr = StringObject, StringFrame,
		MUIA_FixWidthTxt, "01234567890",
		MUIA_String_Accept, "-.0123456789", End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_DEMNAME )),  // "DEM Name"
	  Child, DM_Win->Txt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt,
		"012345678901234567890123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_LEVEL )),  // "Level"
	  Child, DM_Win->Txt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVPTRN )),  // " Elev Ptrn"
	  Child, DM_Win->Txt[2] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_REFSYS )),  // " Ref Sys"
	  Child, DM_Win->Txt[3] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ZONE )),  // " Zone"
	  Child, DM_Win->Txt[4] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label(GetString( MSG_MOREGUI_ROJECTIONPARAMETERS )),  // "\0334Projection Parameters"
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->ProjTxt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[2] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->ProjTxt[3] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[4] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[5] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->ProjTxt[6] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[7] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[8] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->ProjTxt[9] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[10] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[11] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->ProjTxt[12] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[13] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, DM_Win->ProjTxt[14] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_GROUNDUNITS )),  // "Ground Units"
	  Child, DM_Win->Txt[5] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVUNITS )),  // " Elev Units"
	  Child, DM_Win->Txt[6] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_POLYSIDES )),  // " Poly Sides"
	  Child, DM_Win->Txt[7] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label(GetString( MSG_MOREGUI_COORDINATEPAIRS )),  // "\0334Coordinate Pairs"
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_SWE )),  // "SW E"
	  Child, DM_Win->CoordTxt[0][0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_N )),  // " N"
	  Child, DM_Win->CoordTxt[0][1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_NWE )),  // "NW E"
	  Child, DM_Win->CoordTxt[1][0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_N )),  // " N"
	  Child, DM_Win->CoordTxt[1][1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_NEE )),  // "NE E"
	  Child, DM_Win->CoordTxt[2][0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_N )),  // " N"
	  Child, DM_Win->CoordTxt[2][1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_SEE )),  // "SE E"
	  Child, DM_Win->CoordTxt[3][0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_N )),  // " N"
	  Child, DM_Win->CoordTxt[3][1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVMIN )),  // "Elev Min"
	  Child, DM_Win->Txt[8] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVMAX )),  // " Elev Max"
	  Child, DM_Win->Txt[9] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_AXISROTATION )),  // "Axis Rotation"
	  Child, DM_Win->Txt[10] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_ACCURACY )),  // " Accuracy"
	  Child, DM_Win->Txt[11] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label(GetString( MSG_MOREGUI_RESOLUTIONX )),  // "Resolution X"
	  Child, DM_Win->ResTxt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012", End,
	  Child, Label2(" Y"),
	  Child, DM_Win->ResTxt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012", End,
	  Child, Label2(" Z"),
	  Child, DM_Win->ResTxt[2] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ROWS )),  // "Rows"
	  Child, DM_Win->Txt[12] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_COLUMNS )),  // " Columns"
	  Child, DM_Win->Txt[13] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label(GetString( MSG_MOREGUI_PROFILEDATA )),  // "\0334Profile Data"
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ROW )),  // "Row"
	  Child, DM_Win->ProfTxt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_COLUMN )),  // " Column"
	  Child, DM_Win->ProfTxt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_PROFROWS )),  // " Prof Rows"
	  Child, DM_Win->ProfTxt[2] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, Label2(GetString( MSG_MOREGUI_PROFCOLS )),  // " Prof Cols"
	  Child, DM_Win->ProfTxt[3] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_COORDSE )),  // "Coords E"
	  Child, DM_Win->ProfCoordTxt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_N )),  // " N"
	  Child, DM_Win->ProfCoordTxt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVATIONDATUM )),  // "Elevation Datum"
	  Child, DM_Win->ProfTxt[4] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVMIN )),  //  "Elev Min"
	  Child, DM_Win->ProfTxt[5] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, Label2(GetString( MSG_MOREGUI_ELEVMAX )),  // "Elev Max"
	  Child, DM_Win->ProfTxt[6] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "0123456789012345678901234", End,
	  Child, RectangleObject, End,
	  End, /* HGroup */

	Child, HGroup,
	  Child, RectangleObject, End,
	  Child, DM_Win->BT_Extract = KeyButtonFunc('E', (char*)GetString( MSG_MOREGUI_EXTRACT )),  // "\33cExtract"
	  Child, RectangleObject, End,
	  End, /* HGroup */
        End, /* VGroup */
      End; /* WindowObject */

  if (! DM_Win->ExtractWin)
   {
   Close_DM_Window();
   User_Message(GetString( MSG_MOREGUI_DEMEXTRACT ),   // "DEM Extract"
                GetString( MSG_MOREGUI_OUTOFMEMORY ),  // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),           // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DM_Win->ExtractWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(DM_Win->ExtractWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DM_CLOSE);  

  MUI_DoNotiPresFal(app,
   DM_Win->BT_Extract, ID_DM_EXTRACT, DM_Win->BT_GetFiles, ID_DM_GETFILES, NULL);

/* set tab cycle chain */
  DoMethod(DM_Win->ExtractWin, MUIM_Window_SetCycleChain,
   DM_Win->BT_GetFiles, DM_Win->LatStr, DM_Win->LonStr,
   DM_Win->BT_Extract, NULL);

/* set return cycle chain */
 DoMethod(DM_Win->LatStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DM_Win->ExtractWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DM_Win->LonStr); 
 DoMethod(DM_Win->LonStr, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
  DM_Win->ExtractWin, 3, MUIM_Set, MUIA_Window_ActiveObject, DM_Win->LatStr); 

/* set active gadget */
 set(DM_Win->ExtractWin, MUIA_Window_ActiveObject, (IPTR)DM_Win->BT_GetFiles); 
 set(DM_Win->BT_Extract, MUIA_Disabled, TRUE); 

 strcpy(DM_Win->DEMExtract->elevpath, dirname);
 DM_Win->DEMExtract->pattern = "";

/* Open window */
  set(DM_Win->ExtractWin, MUIA_Window_Open, TRUE);
  get(DM_Win->ExtractWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_DM_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DM_Win->ExtractWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DM_ACTIVATE);

DM_Win->DEMExtract->FrFile = NULL;

} /* Open_DM_Window() */

/********************************************************************/

void Close_DM_Window(void)
{

 if (DM_Win)
  {
  if (DM_Win->ExtractWin)
   {
   set(DM_Win->ExtractWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, DM_Win->ExtractWin);
   MUI_DisposeObject(DM_Win->ExtractWin);
   } /* if window created */
  if (DM_Win->DEMExtract)
    {
    if(DM_Win->DEMExtract->FrFile)
      {
      freemultifilename(DM_Win->DEMExtract->FrFile);
      DM_Win->DEMExtract->FrFile = NULL;
      } /* if */
    free_Memory(DM_Win->DEMExtract, sizeof (struct DEMExtractData));
    } /* if */
  free_Memory(DM_Win, sizeof (struct DEMExtractWindow));
  DM_Win = NULL;
  } /* if memory allocated */

} /* Close_DM_Window() */

/********************************************************************/

void Handle_DM_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_DM_Window();
   return;
   } /* Open DEM Extract window */

  if (! DM_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate DEM Extract window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_DM_GETFILES:
      {
      if(DM_Win->DEMExtract->FrFile)
        {
        freemultifilename(DM_Win->DEMExtract->FrFile);
        DM_Win->DEMExtract->FrFile = NULL;
        } /* if */
      if(!(DM_Win->DEMExtract->FrFile = getmultifilename((char*)GetString( MSG_MOREGUI_USGSDEMFILES ),  // "USGS DEM Files"
       DM_Win->DEMExtract->elevpath, DM_Win->DEMExtract->elevfile,
       DM_Win->DEMExtract->pattern)))
        {
        set(DM_Win->BT_Extract, MUIA_Disabled, TRUE); 
        set(DM_Win->SumFilesTxt, MUIA_Text_Contents, (IPTR)"0");
        break;
        } /* if */
      set (DM_Win->DirTxt, MUIA_Text_Contents, (IPTR)DM_Win->DEMExtract->elevpath);
      sprintf(str, "%1ld", DM_Win->DEMExtract->FrFile->rf_NumArgs);
      set(DM_Win->SumFilesTxt, MUIA_Text_Contents, (IPTR)str);
      if  (DM_Win->DEMExtract->FrFile->rf_NumArgs > 0)
       set(DM_Win->BT_Extract, MUIA_Disabled, FALSE); 
      break;
      } /* get file list */
     case ID_DM_EXTRACT:
      {
      ExtractDEM(DM_Win->DEMExtract);
      if(DM_Win->DEMExtract->FrFile)
        {
        freemultifilename(DM_Win->DEMExtract->FrFile);
        DM_Win->DEMExtract->FrFile = NULL;
        } /* if */
      break;
      } /* interpolate */
     case ID_DM_CLOSE:
      {
      Close_DM_Window();
      break;
      } /* close */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */
   } /* switch gadget group */

} /* Handle_DM_Window() */

/**********************************************************************/

short Set_DM_Data(struct DEMExtractData *DEMExtract)
{
 char *floatstr;
 short Proceed;

/* ask for lat/lon values to use */
 Proceed = User_Message(GetString( MSG_MOREGUI_DATAOPSMODULEDEMEXTRACT ),                       // "Data Ops Module: DEM Extract"
		 GetString( MSG_MOREGUI_PLEASEENTERTHELATITUDEANDLONGITUDEVALUESFORTHESOUTH ),  // "Please enter the latitude and longitude values for the southeast corner of the current DEM in the string gadgets near the top of the DEM Extract Window."
		 GetString( MSG_MOREGUI_PROCEEDCANCEL ), (CONST_STRPTR)"pc");                   // "Proceed|Cancel"

/* get values from string gadgets */
 get(DM_Win->LatStr, MUIA_String_Contents, &floatstr);
 DEMExtract->SELat = atof(floatstr);
 get(DM_Win->LonStr, MUIA_String_Contents, &floatstr);
 DEMExtract->SELon = atof(floatstr);

 return (Proceed);

} /* Set_DM_Data() */

/**********************************************************************/

void Set_DM_HdrData(struct USGS_DEMHeader *Hdr)
{
 short i;

 set(DM_Win->Txt[0], MUIA_Text_Contents, (IPTR)Hdr->FileName);
 set(DM_Win->Txt[1], MUIA_Text_Contents, (IPTR)Hdr->LevelCode);
 set(DM_Win->Txt[2], MUIA_Text_Contents, (IPTR)Hdr->ElPattern);
 set(DM_Win->Txt[3], MUIA_Text_Contents, (IPTR)Hdr->RefSysCode);
 set(DM_Win->Txt[4], MUIA_Text_Contents, (IPTR)Hdr->Zone);
 for (i=0; i<15; i++)
  {
  set(DM_Win->ProjTxt[i], MUIA_Text_Contents, (IPTR)Hdr->ProjPar[i]);
  } /* for i=0... */
 set(DM_Win->Txt[5], MUIA_Text_Contents, (IPTR)Hdr->GrUnits);
 set(DM_Win->Txt[6], MUIA_Text_Contents, (IPTR)Hdr->ElUnits);
 set(DM_Win->Txt[7], MUIA_Text_Contents, (IPTR)Hdr->PolySides);
 for (i=0; i<4; i++)
  {
  set(DM_Win->CoordTxt[i][0], MUIA_Text_Contents, (IPTR)Hdr->Coords[i][0]);
  set(DM_Win->CoordTxt[i][1], MUIA_Text_Contents, (IPTR)Hdr->Coords[i][1]);
  } /* for i=0... */
 set(DM_Win->Txt[8], MUIA_Text_Contents, (IPTR)Hdr->ElMin);
 set(DM_Win->Txt[9], MUIA_Text_Contents, (IPTR)Hdr->ElMax);
 set(DM_Win->Txt[10], MUIA_Text_Contents, (IPTR)Hdr->AxisRot);
 set(DM_Win->Txt[11], MUIA_Text_Contents, (IPTR)Hdr->Accuracy);
 for (i=0; i<3; i++)
  {
  set(DM_Win->ResTxt[i], MUIA_Text_Contents, (IPTR)Hdr->Resolution[i]);
  } /* for i=0... */
 set(DM_Win->Txt[12], MUIA_Text_Contents, (IPTR)Hdr->Rows);
 set(DM_Win->Txt[13], MUIA_Text_Contents, (IPTR)Hdr->Columns);

} /* Set_DM_Data() */

/**********************************************************************/

void Set_DM_ProfData(struct USGS_DEMProfileHeader *ProfHdr)
{

 set(DM_Win->ProfTxt[0], MUIA_Text_Contents, (IPTR)ProfHdr->Row);
 set(DM_Win->ProfTxt[1], MUIA_Text_Contents, (IPTR)ProfHdr->Column);
 set(DM_Win->ProfTxt[2], MUIA_Text_Contents, (IPTR)ProfHdr->ProfRows);
 set(DM_Win->ProfTxt[3], MUIA_Text_Contents, (IPTR)ProfHdr->ProfCols);
 set(DM_Win->ProfCoordTxt[0], MUIA_Text_Contents, (IPTR)ProfHdr->Coords[0]);
 set(DM_Win->ProfCoordTxt[1], MUIA_Text_Contents, (IPTR)ProfHdr->Coords[1]);
 set(DM_Win->ProfTxt[4], MUIA_Text_Contents, (IPTR)ProfHdr->ElDatum);
 set(DM_Win->ProfTxt[5], MUIA_Text_Contents, (IPTR)ProfHdr->ElMin);
 set(DM_Win->ProfTxt[6], MUIA_Text_Contents, (IPTR)ProfHdr->ElMax);

} /* Set_DM_Data() */

/*
 printf("File Name:		%s\n", atoi(Hdr->FileName));
 printf("Level Code: 		%d\n", atoi(Hdr->LevelCode));
 printf("Elev Ptrn: 		%d\n", atoi(Hdr->ElPattern));
 printf("Ref Sys Code: 		%d\n", atoi(Hdr->RefSysCode));
 printf("Zone: 			%d\n", atoi(Hdr->Zone));
 printf("Projection Par: 	%f\n", FCvt(Hdr->ProjPar[0]));
 printf("Ground Units: 		%d\n", atoi(Hdr->GrUnits));
 printf("Elev Units: 		%d\n", atoi(Hdr->ElUnits));
 printf("Polygon Sides: 	%d\n", atoi(Hdr->PolySides));
 printf("SW Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[0][0]), FCvt(Hdr->Coords[0][1]));
 printf("NW Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[1][0]), FCvt(Hdr->Coords[1][1]));
 printf("NE Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[2][0]), FCvt(Hdr->Coords[2][1]));
 printf("SE Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[3][0]), FCvt(Hdr->Coords[3][1]));
 printf("Min Elevation:		%f\n", FCvt(Hdr->ElMin));
 printf("Max Elevation: 	%f\n", FCvt(Hdr->ElMax));
 printf("Axis Rotation: 	%f\n", FCvt(Hdr->AxisRot));
 printf("Accuracy: 		%d\n", atoi(Hdr->Accuracy));
 printf("Resolution X: 		%f\n", FCvt(Hdr->Resolution[0]));
 printf("Resolution Y: 		%f\n", FCvt(Hdr->Resolution[1]));
 printf("Resolution Z: 		%f\n", FCvt(Hdr->Resolution[2]));
 printf("Rows: 			%d\n", atoi(Hdr->Rows));
 printf("Columns: 		%d\n", atoi(Hdr->Columns));

  if (! fgets(ProfHdr->Row,	 7,	 DEM)) return (0);
  value = atoi(ProfHdr->Row);
  }
 fgets(ProfHdr->Column,		 7,	 DEM);
 fgets(ProfHdr->ProfRows,	 7,	 DEM);
 fgets(ProfHdr->ProfCols,	 7,	 DEM);
 fgets(ProfHdr->Coords[0],	 25,	 DEM);
 fgets(ProfHdr->Coords[1],	 25,	 DEM);
 fgets(ProfHdr->ElDatum,	 25,	 DEM);
 fgets(ProfHdr->ElMin,		 25,	 DEM);
 if (! fgets(ProfHdr->ElMax,	 25,	 DEM)) return (0);

EXTERN struct USGS_DEMHeader {
 char 	FileName[145],
	LevelCode[7],
	ElPattern[7],
	RefSysCode[7],
	Zone[7],
	ProjPar[15][25],
	GrUnits[7],
	ElUnits[7],
	PolySides[7],
	Coords[4][2][25],
	ElMin[25],
	ElMax[25],
	AxisRot[25],
	Accuracy[7],
	Resolution[3][13],
	Rows[7],
	Columns[7];
};

EXTERN struct USGS_DEMProfileHeader {
 char	Row[7],
	Column[7],
	ProfRows[7],
	ProfCols[7],
	Coords[2][25],
	ElDatum[25],
	ElMin[25],
	ElMax[25];
};

*/

/**********************************************************************/

void Make_PJ_Window(void)
{
 long open, i;

 static int Init=TRUE;
 static const char *PageNames[3]={NULL};

 if(Init)
 {
	 Init=FALSE;
	 PageNames[0] = (char*)GetString( MSG_MOREGUI_FIRSTPAGE ),   // "First Page"
	 PageNames[1] = (char*)GetString( MSG_MOREGUI_SECONDPAGE ),  //"Second Page",
	 PageNames[2] = NULL;
 }

 if (PJ_Win)
  {
  DoMethod(PJ_Win->ProjWin, MUIM_Window_ToFront);
  set(PJ_Win->ProjWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((PJ_Win = (struct ProjectWindow *)
	get_Memory(sizeof (struct ProjectWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     PJ_Win->ProjWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_MOREGUI_PROJECTEDITOR ),  // "Project Editor"
      MUIA_Window_ID		, MakeID('P','R','O','J'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, RegisterGroup(PageNames),
	  Child, VGroup,
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PROJECTPATH_SPACES )),  // "Project Path        "
	    Child, PJ_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, projectpath, End,
	    Child, PJ_Win->BT_Get[0] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PROJECTNAME_SPACES )),  // "Project Name        "
	    Child, PJ_Win->Str[1] = StringObject, StringFrame,
		MUIA_String_Contents, projectname, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_DATABASEPATH_SPACES )),  // "Database Path       "
	    Child, PJ_Win->Str[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, dbasepath, End,
	    Child, PJ_Win->BT_Get[1] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_DATABASENAME_SPACES )),  // "Database Name       "
	    Child, PJ_Win->Str[3] = StringObject, StringFrame,
		MUIA_String_Contents, dbasename, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PARAMETERPATH_SPACES )),  // "Parameter Path      "
	    Child, PJ_Win->Str[4] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, parampath, End,
	    Child, PJ_Win->BT_Get[2] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PARAMETERNAME_SPACES )),  // "Parameter Name      "
	    Child, PJ_Win->Str[5] = StringObject, StringFrame,
		MUIA_String_Contents, paramfile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_FRAMESAVEPATH_SPACES )),  // "Frame Save Path     "
	    Child, PJ_Win->Str[6] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, framepath, End,
	    Child, PJ_Win->BT_Get[3] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_FRAMESAVENAME_SPACES )),  // "Frame Save Name     "
	    Child, PJ_Win->Str[7] = StringObject, StringFrame,
		MUIA_String_Contents, framefile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_TEMPFRAMEPATH_SPACES )),  // "Temp Frame Path     "
	    Child, PJ_Win->Str[8] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, temppath, End,
	    Child, PJ_Win->BT_Get[4] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_VECTORSAVEPATH_SPACES )),  // "Vector Save Path    "
	    Child, PJ_Win->Str[10] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, linepath, End,
	    Child, PJ_Win->BT_Get[5] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_VECTORSAVENAME_SPACES )),  // "Vector Save Name    "
	    Child, PJ_Win->Str[11] = StringObject, StringFrame,
		MUIA_String_Contents, linefile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_ZBUFFERPATH_SPACES )),  // "Z Buffer Path       "
	    Child, PJ_Win->Str[12] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, zbufferpath, End,
	    Child, PJ_Win->BT_Get[6] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_ZBUFFERNAME_SPACES )),  // "Z Buffer Name       "
	    Child, PJ_Win->Str[13] = StringObject, StringFrame,
		MUIA_String_Contents, zbufferfile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_BACKGROUNDPATH_SPACES )),  // "Background Path     "
	    Child, PJ_Win->Str[14] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, backgroundpath, End,
	    Child, PJ_Win->BT_Get[7] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_BACKGROUNDNAME_SPACES )),  // "Background Name     "
	    Child, PJ_Win->Str[15] = StringObject, StringFrame,
		MUIA_String_Contents, backgroundfile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_DEFAULTDIRECTORY_SPACES )),  // "Default Directory   "
	    Child, PJ_Win->Str[20] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, dirname, End,
	    Child, PJ_Win->BT_Get[11] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  End, /* VGroup */

	  Child, VGroup,
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_GRAPHICSAVEPATH_SPACES )),  // "Graphic Save Path   "
	    Child, PJ_Win->Str[16] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, graphpath, End,
	    Child, PJ_Win->BT_Get[8] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_GRAPHICSAVENAME_SPACES )),  // "Graphic Save Name   "
	    Child, PJ_Win->Str[17] = StringObject, StringFrame,
		MUIA_String_Contents, graphname, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_COLORMAPPATH_SPACES )),  // "Color Map Path      "
	    Child, PJ_Win->Str[18] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, colormappath, End,
	    Child, PJ_Win->BT_Get[9] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_COLORMAPNAME_SPACES )),  // "Color Map Name      ")
	    Child, PJ_Win->Str[9] = StringObject, StringFrame,
		MUIA_String_Contents, colormapfile, End,
	    End, /* HGroup */

	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_CLOUDMAPPATH_SPACES )),  // "Cloud Map Path      "
	    Child, PJ_Win->Str[21] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, cloudpath, End,
	    Child, PJ_Win->BT_Get[12] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_CLOUDMAPNAME_SPACES )),  // "Cloud Map Name      "
	    Child, PJ_Win->Str[22] = StringObject, StringFrame,
		MUIA_String_Contents, cloudfile, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_WAVEFILEPATH_SPACES )),  // "Wave File Path      "
	    Child, PJ_Win->Str[23] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, wavepath, End,
	    Child, PJ_Win->BT_Get[13] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_WAVEFILENAME_SPACES )),  // "Wave File Name      "
	    Child, PJ_Win->Str[24] = StringObject, StringFrame,
		MUIA_String_Contents, wavefile, End,
	    End, /* HGroup */

	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_DEFORMATIONMAPPATH_SPACES )),  // "Deformation Map Path"
	    Child, PJ_Win->Str[25] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, deformpath, End,
	    Child, PJ_Win->BT_Get[14] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_DEFORMATIONMAPNAME_SPACES )),  // "Deformation Map Name"
	    Child, PJ_Win->Str[26] = StringObject, StringFrame,
		MUIA_String_Contents, deformfile, End,
	    End, /* HGroup */

	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_ECOSYSTEMMODELPATH_SPACES )),  // "Ecosystem Model Path"
	    Child, PJ_Win->Str[19] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, modelpath, End,
	    Child, PJ_Win->BT_Get[10] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */

	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_IMAGEPATH_SPACES )),  // "Image Path          "
	    Child, PJ_Win->Str[27] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, imagepath, End,
	    Child, PJ_Win->BT_Get[15] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_SUNIMAGEFILE_SPACES )),  // "Sun Image File      "
	    Child, PJ_Win->Str[28] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, sunfile, End,
	    Child, PJ_Win->BT_Get[16] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_MOONIMAGEFILE_SPACES )),  // "Moon Image File     "
	    Child, PJ_Win->Str[29] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, moonfile, End,
	    Child, PJ_Win->BT_Get[17] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */

	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PCPROJECTDIRECTORY_SPACES )),  // "PC Project Directory"
	    Child, PJ_Win->Str[30] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, pcprojectpath, End,
	    Child, PJ_Win->BT_Get[18] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PCFRAMESDIRECTORY_SPACES )),  // "PC Frames Directory "
	    Child, PJ_Win->Str[31] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "012345678901234567890",
		MUIA_String_Contents, pcframespath, End,
	    Child, PJ_Win->BT_Get[19] = ImageButtonWCS(MUII_Disk),
	    End, /* HGroup */
	  End, /* VGroup */
	  End, /* RegisterGroup */

	  Child, HGroup,
	    Child, RectangleObject, End,
            Child, PJ_Win->BT_DirList = KeyButtonFunc('d', (char*)GetString( MSG_MOREGUI_DIRECTORYLIST )),  // "\33cDirectory List"
            Child, PJ_Win->BT_Save = KeyButtonFunc('s', (char*)GetString( MSG_MOREGUI_SAVE )),              // "\33cSave"
	    Child, RectangleObject, End,
            End, /* HGroup */
/*
	  Child, HGroup,
	    Child, RectangleObject, End,
            Child, PJ_Win->BT_Apply = KeyButtonFunc('k', "\33cKeep"), 
            Child, PJ_Win->BT_Cancel = KeyButtonFunc('c', "\33cCancel"), 
	    Child, RectangleObject, End,
            End, // HGroup
*/
	  End, /* VGroup */
	End; /* Window object */

  if (! PJ_Win->ProjWin)
   {
   Close_PJ_Window(1);
   User_Message(GetString( MSG_MOREGUI_PROJECTNEWEDIT ),  // "Project: New/Edit"
                GetString( MSG_MOREGUI_OUTOFMEMORY ),     // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),              //"OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, PJ_Win->ProjWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(PJ_Win->ProjWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PJ_CLOSEQUERY);  

  for (i=0; i<20; i++)
   MUI_DoNotiPresFal(app, PJ_Win->BT_Get[i], ID_PJ_GET(i), NULL);


  for (i=0; i<32; i++)
   DoMethod(PJ_Win->Str[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_PJ_STRING(i));

  MUI_DoNotiPresFal(app, PJ_Win->BT_DirList, ID_DL_WINDOW,
   PJ_Win->BT_Save, ID_SAVEPROJNEW,/* PJ_Win->BT_Apply, ID_PJ_APPLY,*/
   /*PJ_Win->BT_Cancel, ID_PJ_CLOSE,*/ NULL);

/* Set tab cycle chain */
  DoMethod(PJ_Win->ProjWin, MUIM_Window_SetCycleChain,
	PJ_Win->BT_DirList,/* PJ_Win->BT_Apply,*/ PJ_Win->BT_Save,
	/*PJ_Win->BT_Cancel,*/ NULL);

/* return cycle */
  DoMethod(PJ_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[1]);
  DoMethod(PJ_Win->Str[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[2]);
  DoMethod(PJ_Win->Str[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[3]);
  DoMethod(PJ_Win->Str[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[4]);
  DoMethod(PJ_Win->Str[4], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[5]);
  DoMethod(PJ_Win->Str[5], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[6]);
  DoMethod(PJ_Win->Str[6], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[7]);
  DoMethod(PJ_Win->Str[7], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[8]);
  DoMethod(PJ_Win->Str[8], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[10]);
  DoMethod(PJ_Win->Str[10], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[11]);
  DoMethod(PJ_Win->Str[11], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[12]);
  DoMethod(PJ_Win->Str[12], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[13]);
  DoMethod(PJ_Win->Str[13], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[14]);
  DoMethod(PJ_Win->Str[14], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[15]);
  DoMethod(PJ_Win->Str[15], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[20]);
  DoMethod(PJ_Win->Str[20], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[0]);

  DoMethod(PJ_Win->Str[16], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[17]);
  DoMethod(PJ_Win->Str[17], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[18]);
  DoMethod(PJ_Win->Str[18], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[9]);
  DoMethod(PJ_Win->Str[9], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[21]);
  DoMethod(PJ_Win->Str[21], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[22]);
  DoMethod(PJ_Win->Str[22], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[23]);
  DoMethod(PJ_Win->Str[23], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[24]);
  DoMethod(PJ_Win->Str[24], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[25]);
  DoMethod(PJ_Win->Str[25], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[26]);
  DoMethod(PJ_Win->Str[26], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[19]);
  DoMethod(PJ_Win->Str[19], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[27]);
  DoMethod(PJ_Win->Str[27], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[28]);
  DoMethod(PJ_Win->Str[28], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[29]);
  DoMethod(PJ_Win->Str[29], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[30]);
  DoMethod(PJ_Win->Str[30], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[31]);
  DoMethod(PJ_Win->Str[31], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	PJ_Win->ProjWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, PJ_Win->Str[16]);

/* Set active gadget */
  set(PJ_Win->ProjWin, MUIA_Window_ActiveObject, (IPTR)PJ_Win->Str[0]);

/* Open window */
  set(PJ_Win->ProjWin, MUIA_Window_Open, TRUE);
  get(PJ_Win->ProjWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_PJ_Window(1);
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(PJ_Win->ProjWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_PJ_ACTIVATE);

} /* Make_PJ_Window() */


/**********************************************************************/

void Close_PJ_Window(short Apply)
{
 if (PJ_Win)
  {
  if (PJ_Win->ProjWin)
   {
   set(PJ_Win->ProjWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, PJ_Win->ProjWin);
   MUI_DisposeObject(PJ_Win->ProjWin);
   } /* if window created */
  free_Memory(PJ_Win, sizeof (struct ProjectWindow));
  PJ_Win = NULL;
  } /* if memory allocated */

} /* Close_PJ_Window() */

/**********************************************************************/

void Handle_PJ_Window(ULONG WCS_ID)
{

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_PJ_Window();
   return;
   } /* Open Project Window */

  if (! PJ_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Project window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_PJ_APPLY:
     case ID_PJ_CLOSEQUERY:
     case ID_PJ_CLOSE:
      {
      Close_PJ_Window(1);
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    short i;
    char dummyfile[32];

    dummyfile[0] = 0;
    i = WCS_ID - ID_PJ_GET(0);
    switch (i)
     {
     case 0:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_PROJECTPATHNAME ), projectpath, projectname);  // "Project Path/Name"
      set(PJ_Win->Str[0], MUIA_String_Contents, (IPTR)projectpath);
      set(PJ_Win->Str[1], MUIA_String_Contents, (IPTR)projectname);
      break;
      } /* framepath */
     case 1:
      {
      getfilename(0,(char*)GetString( MSG_MOREGUI_DATABASEPATHNAME ), dbasepath, dbasename);  // "Database Path/Name"
      set(PJ_Win->Str[2], MUIA_String_Contents, (IPTR)dbasepath);
      set(PJ_Win->Str[3], MUIA_String_Contents, (IPTR)dbasename);
      break;
      } /* framepath */
     case 2:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_PARAMETERPATHNAME ), parampath, paramfile);  // GetString( MSG_MOREGUI_PARAMETERPATHNAME )
      set(PJ_Win->Str[4], MUIA_String_Contents, (IPTR)parampath);
      set(PJ_Win->Str[5], MUIA_String_Contents, (IPTR)paramfile);
      break;
      } /* framepath */
     case 3:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_FRAMESAVEPATHNAME ), framepath, framefile);  // "Frame Save Path/Name"
      set(PJ_Win->Str[6], MUIA_String_Contents, (IPTR)framepath);
      set(PJ_Win->Str[7], MUIA_String_Contents, (IPTR)framefile);
      break;
      } /* framepath */
     case 4:
      {
      strcpy(tempfile, framefile);
      strcat(tempfile, ".temp");
      getfilename(1, (char*)GetString( MSG_MOREGUI_TEMPFRAMEPATHNAME ), temppath, tempfile);  // "Temp Frame Path/Name"
      set(PJ_Win->Str[8], MUIA_String_Contents, (IPTR)temppath);
/*      set(PJ_Win->Str[9], MUIA_String_Contents, tempfile);*/
      break;
      } /* temppath */
     case 5:
      {
      getfilename(1, (char*)GetString( MSG_MOREGUI_VECTORSAVEPATHNAME ), linepath, linefile);  // "Vector Save Path/Name"
      set(PJ_Win->Str[10], MUIA_String_Contents, (IPTR)linepath);
      set(PJ_Win->Str[11], MUIA_String_Contents, (IPTR)linefile);
      break;
      } /* linepath */
     case 6:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_ZBUFFERPATHNAME ), zbufferpath, zbufferfile);  // "Z Buffer Path/Name"
      set(PJ_Win->Str[12], MUIA_String_Contents, (IPTR)zbufferpath);
      set(PJ_Win->Str[13], MUIA_String_Contents, (IPTR)zbufferfile);
      break;
      } /* zbufferpath */
     case 7:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_BACKGROUNDPATHNAME ), backgroundpath, backgroundfile);  // "Background Path/Name"
      set(PJ_Win->Str[14], MUIA_String_Contents, (IPTR)backgroundpath);
      set(PJ_Win->Str[15], MUIA_String_Contents, (IPTR)backgroundfile);
      break;
      } /* backgroundpath */
     case 8:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_GRAPHICSAVEPATHNAME ), graphpath, graphname);  // "Graphic Save Path/Name"
      set(PJ_Win->Str[16], MUIA_String_Contents, (IPTR)graphpath);
      set(PJ_Win->Str[17], MUIA_String_Contents, (IPTR)graphname);
      break;
      } /* graphpath */
     case 9:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_COLORMAPFILEPATH ), colormappath, colormapfile);  // "Color Map File Path"
      set(PJ_Win->Str[18], MUIA_String_Contents, (IPTR)colormappath);
      set(PJ_Win->Str[9], MUIA_String_Contents, (IPTR)colormapfile);
      break;
      } /* colormappath */
     case 10:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_ECOSYSTEMMODELPATH ), modelpath, dummyfile);  // "Ecosystem Model Path"
      set(PJ_Win->Str[19], MUIA_String_Contents, (IPTR)modelpath);
      break;
      } /* modelpath */
     case 11:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_DEFAULTDIRECTORY ), dirname, dummyfile);  // "Default Directory"
      set(PJ_Win->Str[20], MUIA_String_Contents, (IPTR)dirname);
      break;
      } /* dirname */
     case 12:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_CLOUDMAPPATH ), cloudpath, cloudfile);  // "Cloud Map Path"
      set(PJ_Win->Str[21], MUIA_String_Contents, (IPTR)cloudpath);
      set(PJ_Win->Str[22], MUIA_String_Contents, (IPTR)cloudfile);
      break;
      } /* dirname */
     case 13:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_WAVEPATH ), wavepath, wavefile);  // "Wave Path"
      set(PJ_Win->Str[23], MUIA_String_Contents, (IPTR)wavepath);
      set(PJ_Win->Str[24], MUIA_String_Contents, (IPTR)wavefile);
      break;
      } /* dirname */
     case 14:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_WAVEPATH ), deformpath, deformfile);  // "Deformation Path"
      set(PJ_Win->Str[25], MUIA_String_Contents, (IPTR)deformpath);
      set(PJ_Win->Str[26], MUIA_String_Contents, (IPTR)deformfile);
      break;
      } /* dirname */
     case 15:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_IMAGEPATH ), imagepath, dummyfile);  // "Image Path"
      set(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
      break;
      } /* dirname */
     case 16:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_SUNIMAGEFILE ), imagepath, sunfile);  // "Sun Image File"
      set(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
      set(PJ_Win->Str[28], MUIA_String_Contents, (IPTR)sunfile);
      break;
      } /* dirname */
     case 17:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_MOONIMAGEFILE ), imagepath, moonfile);  // "Moon Image File"
      set(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
      set(PJ_Win->Str[29], MUIA_String_Contents, (IPTR)moonfile);
      break;
      } /* dirname */
     case 18:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_PCPROJECTPATH ), pcprojectpath, dummyfile);  // "PC Project Path"
      set(PJ_Win->Str[30], MUIA_String_Contents, (IPTR)pcprojectpath);
      break;
      } /* dirname */
     case 19:
      {
      getfilename(0, (char*)GetString( MSG_MOREGUI_PCFRAMESPATH ), pcframespath, dummyfile);  // "PC Frames Path"
      set(PJ_Win->Str[31], MUIA_String_Contents, (IPTR)pcframespath);
      break;
      } /* dirname */
     } /* switch i */
    break;
    } /* get path buttons */

   case GP_STRING1:
    {
    short i;
    char *name;

    i = WCS_ID - ID_PJ_STRING(0);
    get(PJ_Win->Str[i], MUIA_String_Contents, &name);
    switch (i)
     {
     case 0:
      {
      strcpy(projectpath, name);
      break;
      } /*  */
     case 1:
      {
      strcpy(projectname, name);
      break;
      } /*  */
     case 2:
      {
      strcpy(dbasepath, name);
      break;
      } /*  */
     case 3:
      {
      strcpy(dbasename, name);
      break;
      } /*  */
     case 4:
      {
      strcpy(parampath, name);
      break;
      } /*  */
     case 5:
      {
      strcpy(paramfile, name);
      break;
      } /*  */
     case 6:
      {
      strcpy(framepath, name);
      break;
      } /*  */
     case 7:
      {
      strcpy(framefile, name);
      break;
      } /*  */
     case 8:
      {
      strcpy(temppath, name);
      break;
      } /*  */
     case 9:
      {
      strcpy(colormapfile, name);
      break;
      } /*  */
     case 10:
      {
      strcpy(linepath, name);
      break;
      } /*  */
     case 11:
      {
      strcpy(linefile, name);
      break;
      } /*  */
     case 12:
      {
      strcpy(zbufferpath, name);
      break;
      } /*  */
     case 13:
      {
      strcpy(zbufferfile, name);
      break;
      } /*  */
     case 14:
      {
      strcpy(backgroundpath, name);
      break;
      } /*  */
     case 15:
      {
      strcpy(backgroundfile, name);
      break;
      } /*  */
     case 16:
      {
      strcpy(graphpath, name);
      break;
      } /*  */
     case 17:
      {
      strcpy(graphname, name);
      break;
      } /*  */
     case 18:
      {
      strcpy(colormappath, name);
      break;
      } /*  */
     case 19:
      {
      strcpy(modelpath, name);
      break;
      } /*  */
     case 20:
      {
      strcpy(dirname, name);
      break;
      } /*  */
     case 21:
      {
      strcpy(cloudpath, name);
      break;
      } /*  */
     case 22:
      {
      strcpy(cloudfile, name);
      break;
      } /*  */
     case 23:
      {
      strcpy(wavepath, name);
      break;
      } /*  */
     case 24:
      {
      strcpy(wavefile, name);
      break;
      } /*  */
     case 25:
      {
      strcpy(deformpath, name);
      break;
      } /*  */
     case 26:
      {
      strcpy(deformfile, name);
      break;
      } /*  */
     case 27:
      {
      strcpy(imagepath, name);
      break;
      } /*  */
     case 28:
      {
      strcpy(sunfile, name);
      break;
      } /*  */
     case 29:
      {
      strcpy(moonfile, name);
      break;
      } /*  */
     case 30:
      {
      strcpy(pcprojectpath, name);
      break;
      } /*  */
     case 31:
      {
      strcpy(pcframespath, name);
      break;
      } /*  */
     } /* switch i */
    break;
    } /* path/name strings */
   } /* switch gadget group */

 Proj_Mod = 1;

} /* Handle_PJ_Window() */

/***********************************************************************/

void Make_SC_Window(void)
{
 char *floatdata;
 long open;

 if (SC_Win)
  {
  DoMethod(SC_Win->ScaleWin, MUIM_Window_ToFront);
  set(SC_Win->ScaleWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((SC_Win = (struct ScaleImageWindow *)
	get_Memory(sizeof (struct ScaleImageWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     SC_Win->ScaleWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_MOREGUI_IMAGESCALE ),  // "Image Scale"
      MUIA_Window_ID		, MakeID('I','M','S','C'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_WIDTH )),  // " Width"
	    Child, SC_Win->Str[0] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "0123456789",
		MUIA_String_Integer, settings.scrnwidth, End,
	    Child, Label2(GetString( MSG_MOREGUI_HEIGHT )),  // "Height"
	    Child, SC_Win->Str[1] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456",
		MUIA_String_Accept, "0123456789",
		MUIA_String_Integer, settings.scrnheight, End,
	    End, /* HGroup */
	  Child, HGroup,
	    Child, Label2(GetString( MSG_MOREGUI_PIXELASPECT )),  // "Pixel Aspect"
	    Child, SC_Win->Str[2] = StringObject, StringFrame,
		MUIA_FixWidthTxt, "0123456", End,
		MUIA_String_Accept, ".0123456789",
	    End, /* HGroup */

	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
	    Child, RectangleObject, End,
            Child, SC_Win->BT_Halve = KeyButtonFunc('h', (char*)GetString( MSG_MOREGUI_HALVE )),     // "\33cHalve"
            Child, SC_Win->BT_Double = KeyButtonFunc('d',  (char*)GetString( MSG_MOREGUI_DOUBLE )),  // "\33cDouble"
	    Child, RectangleObject, End,
            End, /* HGroup */

	  Child, HGroup,
	    Child, RectangleObject, End,
            Child, SC_Win->BT_Apply = KeyButtonFunc('a', (char*)GetString( MSG_MOREGUI_APPLY )),    // "\33cApply"
            Child, SC_Win->BT_Cancel = KeyButtonFunc('c', (char*)GetString( MSG_MOREGUI_CANCEL )),  // "\33cCancel"),
	    Child, RectangleObject, End,
            End, /* HGroup */

	  End, /* VGroup */
	End; /* Window object */

  if (! SC_Win->ScaleWin)
   {
   Close_SC_Window();
   User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
                GetString( MSG_MOREGUI_OUTOFMEMORY ),           // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),                    // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, SC_Win->ScaleWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(SC_Win->ScaleWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_SC_CLOSEQUERY);  

  MUI_DoNotiPresFal(app, SC_Win->BT_Halve, ID_SC_HALVE,
   SC_Win->BT_Double, ID_SC_DOUBLE, SC_Win->BT_Apply, ID_SC_APPLY,
   SC_Win->BT_Cancel, ID_SC_CLOSE, NULL);

/* Set tab cycle chain */
  DoMethod(SC_Win->ScaleWin, MUIM_Window_SetCycleChain,
	SC_Win->Str[0], SC_Win->Str[1], SC_Win->Str[2],
	SC_Win->BT_Halve, SC_Win->BT_Double, SC_Win->BT_Apply,
	SC_Win->BT_Cancel, NULL);

/* return cycle */
  DoMethod(SC_Win->Str[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	SC_Win->ScaleWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, SC_Win->Str[1]);
  DoMethod(SC_Win->Str[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	SC_Win->ScaleWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, SC_Win->Str[2]);
  DoMethod(SC_Win->Str[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
	SC_Win->ScaleWin, 3,
	MUIM_Set, MUIA_Window_ActiveObject, SC_Win->Str[0]);

/* Set active gadget */
  set(SC_Win->ScaleWin, MUIA_Window_ActiveObject, (IPTR)SC_Win->Str[0]);

/* set aspect string */
  get(ES_Win->FloatStr[0], MUIA_String_Contents, &floatdata);
  set(SC_Win->Str[2], MUIA_String_Contents, (IPTR)floatdata);

/* Open window */
  set(SC_Win->ScaleWin, MUIA_Window_Open, TRUE);
  get(SC_Win->ScaleWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_SC_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(SC_Win->ScaleWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_SC_ACTIVATE);

} /* Make_SC_Window() */

/***********************************************************************/

void Close_SC_Window(void)
{
 if (SC_Win)
  {
  if (SC_Win->ScaleWin)
   {
   set(SC_Win->ScaleWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, SC_Win->ScaleWin);
   MUI_DisposeObject(SC_Win->ScaleWin);
   } /* if window created */
  free_Memory(SC_Win, sizeof (struct ScaleImageWindow));
  SC_Win = NULL;
  } /* if memory allocated */

} /* Close_SC_Window() */

/***********************************************************************/

void Handle_SC_Window(ULONG WCS_ID)
{
 long data1, data2;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_SC_Window();
   return;
   } /* Open Image Scale Window */

  if (! SC_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Image Scale window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_SC_HALVE:
      {
      get(SC_Win->Str[0], MUIA_String_Integer, &data1);
      get(SC_Win->Str[1], MUIA_String_Integer, &data2);
      data1 /= 2;
      data2 /= 2;
      if (data1 > 0 && data2 > 0)
       {
       set(SC_Win->Str[0], MUIA_String_Integer, data1);
       set(SC_Win->Str[1], MUIA_String_Integer, data2);
       }
      break;
      }
     case ID_SC_DOUBLE:
      {
      get(SC_Win->Str[0], MUIA_String_Integer, &data1);
      get(SC_Win->Str[1], MUIA_String_Integer, &data2);
      data1 *= 2;
      data2 *= 2;
      if (data1 < 32767 && data2 < 32767)
       {
       set(SC_Win->Str[0], MUIA_String_Integer, data1);
       set(SC_Win->Str[1], MUIA_String_Integer, data2);
       }
      break;
      }
     case ID_SC_APPLY:
      {
      ApplyImageScale();
      Close_SC_Window();
      break;
      }
     case ID_SC_CLOSEQUERY:
      {
      if (User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
                       GetString( MSG_MOREGUI_APPLYCHANGES ),          // "Apply changes?"
                       GetString( MSG_GLOBAL_OKCANCEL ),              // "OK|Cancel"
                       (CONST_STRPTR)"oc"))
       ApplyImageScale();
      Close_SC_Window();
      break;
      }
     case ID_SC_CLOSE:
      {
      Close_SC_Window();
      break;
      }
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   } /* switch gadget group */

} /* Handle_SC_Window() */

/*********************************************************************/

/* fractal depth must be modified by user, it is not set here automatically */

STATIC_FCN void ApplyImageScale(void) // used locally only -> static, AF 26.7.2021
{
short i;
long NewWidth, NewHeight;
double NewAspect, WidthFact, HeightFact;
char *floatdata;

get(SC_Win->Str[0], MUIA_String_Integer, &NewWidth);
get(SC_Win->Str[1], MUIA_String_Integer, &NewHeight);
get(SC_Win->Str[2], MUIA_String_Contents, &floatdata);
WidthFact = (double)NewWidth / settings.scrnwidth;
HeightFact = (double)NewHeight / settings.scrnheight;
NewAspect = atof(floatdata);

PAR_FIRST_MOTION(6) *= WidthFact;
PAR_FIRST_MOTION(7) *= HeightFact;
PAR_FIRST_MOTION(10) /= WidthFact;

for (i=0; i<ParHdr.KeyFrames; i++)
 {
 if (KF[i].MoKey.Item == 6)
  KF[i].MoKey.Value *= WidthFact;
 else if (KF[i].MoKey.Item == 7)
  KF[i].MoKey.Value *= HeightFact;
 else if (KF[i].MoKey.Item == 10)
  KF[i].MoKey.Value /= WidthFact;
 } /* for i=0... */

settings.scrnwidth = NewWidth;
settings.scrnheight = NewHeight;
settings.picaspect = NewAspect;
if (ES_Win)
 {
 set(ES_Win->IntStr[4], MUIA_String_Integer, settings.scrnwidth);
 set(ES_Win->IntStr[5], MUIA_String_Integer, settings.scrnheight);
 set(ES_Win->FloatStr[0], MUIA_String_Contents, (IPTR)floatdata);
 } /* if settings window open */

} /* ApplyImageScale() */

/***********************************************************************/

void Make_PR_Window(void)
{
 long i, open;

 if (PR_Win)
  {
  DoMethod(PR_Win->PrefsWin, MUIM_Window_ToFront);
  set(PR_Win->PrefsWin, MUIA_Window_Activate, TRUE);
  return;
  } /* if window already exists */

 if ((PR_Win = (struct PrefsWindow *)
	get_Memory(sizeof (struct PrefsWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(10);

     PR_Win->PrefsWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_MOREGUI_PREFERENCES ),  // "Preferences"
      MUIA_Window_ID		, MakeID('P','R','E','F'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	  Child, Label(GetString( MSG_MOREGUI_RENDERTASKPRIORITY )),  // "\33c\0334Render Task Priority"
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, PR_Win->BT_LoPri = KeyButtonObject('l'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_LOW ), End,  // "\33cLow"
            Child, PR_Win->BT_NorPri = KeyButtonObject('n'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_NORMAL ), End,  // "\33cNormal"
            Child, PR_Win->BT_HiPri = KeyButtonObject('h'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_HIGH ), End,  // "\33cHigh"
	    End, /* HGroup */
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, Label(GetString( MSG_MOREGUI_RENDERSIZE )),  // "\33c\0334Render Size"
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, PR_Win->BT_RenderQuarter = KeyButtonObject('4'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/4", End,
            Child, PR_Win->BT_RenderHalf = KeyButtonObject('2'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/2", End,
            Child, PR_Win->BT_RenderFull = KeyButtonObject('1'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Text_Contents, "\33c1/1", End,
	    End, /* HGroup */
	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, Label(GetString( MSG_MOREGUI_STATUSLOGMESSAGES )),  // "\33c\0334Status Log Messages"
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, PR_Win->BT_ERR[0] = KeyButtonObject('e'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, ReportMesg[0],
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_ERR ), End,  // "\33cERR"
            Child, PR_Win->BT_ERR[1] = KeyButtonObject('w'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, ReportMesg[1],
		 MUIA_Text_Contents,  GetString( MSG_MOREGUI_WNG ), End,  // "\33cWNG"
            Child, PR_Win->BT_ERR[2] = KeyButtonObject('m'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, ReportMesg[2],
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_MSG ), End,  // "\33cMSG"
            Child, PR_Win->BT_ERR[3] = KeyButtonObject('d'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Selected, ReportMesg[3],
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_DTA ), End,  // "\33cDTA"
	    End, /* HGroup */

	  Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,
	  Child, Label(GetString( MSG_MOREGUI_PARAMFILESAVEFORMAT )),  // "\33c\0334Param File Save Format"
	  Child, HGroup, MUIA_Group_SameWidth, TRUE,
            Child, PR_Win->BT_SaveBin = KeyButtonObject('b'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Disabled, TRUE,
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_BINARY ), End,  // "\33cBinary"
            Child, PR_Win->BT_SaveAsc = KeyButtonObject('a'),
		 MUIA_InputMode, MUIV_InputMode_Toggle,
		 MUIA_Disabled, TRUE,
		 MUIA_Text_Contents, GetString( MSG_MOREGUI_ASCII ), End,  // "\33cAscii"
	    End, /* HGroup */
	  End, /* VGroup */
	End; /* Window object */

  if (! PR_Win->PrefsWin)
   {
   Close_PR_Window();
   User_Message(GetString( MSG_MOREGUI_PREFERENCES ),  // "Preferences"
                GetString( MSG_MOREGUI_OUTOFMEMORY ),  // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),           // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, PR_Win->PrefsWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(PR_Win->PrefsWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_CLOSE);  

/* Set tab cycle chain */
  DoMethod(PR_Win->PrefsWin, MUIM_Window_SetCycleChain,
	PR_Win->BT_LoPri, PR_Win->BT_NorPri, PR_Win->BT_HiPri,
	PR_Win->BT_RenderQuarter, PR_Win->BT_RenderHalf, PR_Win->BT_RenderFull,
	PR_Win->BT_ERR[0],PR_Win->BT_ERR[1],PR_Win->BT_ERR[2],PR_Win->BT_ERR[3],
	PR_Win->BT_SaveBin, PR_Win->BT_SaveAsc, NULL);

/* Set selected status */
  set(PR_Win->BT_LoPri, MUIA_Selected, (RenderTaskPri < 0));
  set(PR_Win->BT_NorPri, MUIA_Selected, (RenderTaskPri == 0));
  set(PR_Win->BT_HiPri, MUIA_Selected, (RenderTaskPri > 0));
  set(PR_Win->BT_RenderQuarter, MUIA_Selected, (RenderSize == -2));
  set(PR_Win->BT_RenderHalf, MUIA_Selected, (RenderSize == -1));
  set(PR_Win->BT_RenderFull, MUIA_Selected, (RenderSize == 0));
  set(PR_Win->BT_SaveBin, MUIA_Selected, (SaveAscii == 0));
  set(PR_Win->BT_SaveAsc, MUIA_Selected, (SaveAscii == 1));

/* set return ID's */
  DoMethod(PR_Win->BT_LoPri, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_LOPRI);
  DoMethod(PR_Win->BT_NorPri, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_NORPRI);
  DoMethod(PR_Win->BT_HiPri, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_HIPRI);
  DoMethod(PR_Win->BT_SaveBin, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_SAVEBIN);
  DoMethod(PR_Win->BT_SaveAsc, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_SAVEASC);
  DoMethod(PR_Win->BT_LoPri, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_LOPRI);
  DoMethod(PR_Win->BT_NorPri, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_NORPRI);
  DoMethod(PR_Win->BT_HiPri, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_HIPRI);

  DoMethod(PR_Win->BT_RenderQuarter, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRQTR);
  DoMethod(PR_Win->BT_RenderHalf, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRHLF);
  DoMethod(PR_Win->BT_RenderFull, MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRFUL);
  DoMethod(PR_Win->BT_RenderQuarter, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRQTR);
  DoMethod(PR_Win->BT_RenderHalf, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRHLF);
  DoMethod(PR_Win->BT_RenderFull, MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_RNDRFUL);

  for (i=0; i<4; i++)
   {
   DoMethod(PR_Win->BT_ERR[i], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_PR_REPTERR(i));
   DoMethod(PR_Win->BT_ERR[i], MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_PR_REPTERR(i));
   } /* for i=0... */

/* Open window */
  set(PR_Win->PrefsWin, MUIA_Window_Open, TRUE);
  get(PR_Win->PrefsWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_PR_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(PR_Win->PrefsWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_PR_ACTIVATE);

} /* Make_PR_Window() */

/***********************************************************************/

void Close_PR_Window(void)
{
 if (PR_Win)
  {
  if (PR_Win->PrefsWin)
   {
   set(PR_Win->PrefsWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, PR_Win->PrefsWin);
   MUI_DisposeObject(PR_Win->PrefsWin);
   } /* if window created */
  free_Memory(PR_Win, sizeof (struct PrefsWindow));
  PR_Win = NULL;
  } /* if memory allocated */

} /* Close_PR_Window() */

/***********************************************************************/

void Handle_PR_Window(ULONG WCS_ID)
{
 long i, data;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_PR_Window();
   return;
   } /* Open Preferences Window */

  if (! PR_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16 );
    break;
    } /* Activate Preferences window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_PR_LOPRI:
      {
      if (! PR_Win->LoPriBlock)
       {
       get(PR_Win->BT_LoPri, MUIA_Selected, &data);
       RenderTaskPri = (data) ? -1: 0;

       get(PR_Win->BT_NorPri, MUIA_Selected, &data);
       if ((data && RenderTaskPri != 0) || (! data && RenderTaskPri == 0))
        PR_Win->NorPriBlock = 1;
       set(PR_Win->BT_NorPri, MUIA_Selected, (RenderTaskPri == 0));

       get(PR_Win->BT_HiPri, MUIA_Selected, &data);
       if (data)
        PR_Win->HiPriBlock = 1;
       set(PR_Win->BT_HiPri, MUIA_Selected, FALSE);
       } /* if no blocking */
      PR_Win->LoPriBlock = 0;
      break;
      } /* low render priority */
     case ID_PR_NORPRI:
      {
      if (! PR_Win->NorPriBlock)
       {
       get(PR_Win->BT_NorPri, MUIA_Selected, &data);
       RenderTaskPri = (data == 1) ? 0: -1;

       get(PR_Win->BT_HiPri, MUIA_Selected, &data);
       if (data)
        PR_Win->HiPriBlock = 1;
       set(PR_Win->BT_HiPri, MUIA_Selected, FALSE);

       get(PR_Win->BT_LoPri, MUIA_Selected, &data);
       if ((data && RenderTaskPri == 0) || (! data && RenderTaskPri < 0))
        PR_Win->LoPriBlock = 1;
       set(PR_Win->BT_LoPri, MUIA_Selected, (RenderTaskPri < 0));
       } /* if no blocking */
      PR_Win->NorPriBlock = 0;
      break;
      } /* normal render priority */
     case ID_PR_HIPRI:
      {
      if (! PR_Win->HiPriBlock)
       {
       get(PR_Win->BT_HiPri, MUIA_Selected, &data);
       RenderTaskPri = (data) ? 1: 0;

       get(PR_Win->BT_NorPri, MUIA_Selected, &data);
       if ((data && RenderTaskPri != 0) || (! data && RenderTaskPri == 0))
        PR_Win->NorPriBlock = 1;
       set(PR_Win->BT_NorPri, MUIA_Selected, (RenderTaskPri == 0));

       get(PR_Win->BT_LoPri, MUIA_Selected, &data);
       if (data)
        PR_Win->LoPriBlock = 1;
       set(PR_Win->BT_LoPri, MUIA_Selected, FALSE);
       } /* if no blocking */
      PR_Win->HiPriBlock = 0;
      break;
      } /* high render priority */

     case ID_PR_RNDRQTR:
      {
      if (! PR_Win->RndrQBlock)
       {
       get(PR_Win->BT_RenderQuarter, MUIA_Selected, &data);
       RenderSize = (data) ? -2: 0;

       get(PR_Win->BT_RenderHalf, MUIA_Selected, &data);
       if (data)
        PR_Win->RndrHBlock = 1;
       set(PR_Win->BT_RenderHalf, MUIA_Selected, FALSE);

       get(PR_Win->BT_RenderFull, MUIA_Selected, &data);
       if ((data && RenderSize != 0) || (! data && RenderSize == 0))
        PR_Win->RndrFBlock = 1;
       set(PR_Win->BT_RenderFull, MUIA_Selected, (RenderSize == 0));
       } /* if no blocking */
      PR_Win->RndrQBlock = 0;
      break;
      } /* quarter size */
     case ID_PR_RNDRHLF:
      {
      if (! PR_Win->RndrHBlock)
       {
       get(PR_Win->BT_RenderHalf, MUIA_Selected, &data);
       RenderSize = (data) ? -1: 0;

       get(PR_Win->BT_RenderQuarter, MUIA_Selected, &data);
       if (data)
        PR_Win->RndrQBlock = 1;
       set(PR_Win->BT_RenderQuarter, MUIA_Selected, FALSE);

       get(PR_Win->BT_RenderFull, MUIA_Selected, &data);
       if ((data && RenderSize != 0) || (! data && RenderSize == 0))
        PR_Win->RndrFBlock = 1;
       set(PR_Win->BT_RenderFull, MUIA_Selected, (RenderSize == 0));
       } /* if no blocking */
      PR_Win->RndrHBlock = 0;
      break;
      } /* half size */
     case ID_PR_RNDRFUL:
      {
      if (! PR_Win->RndrFBlock)
       {
       get(PR_Win->BT_RenderFull, MUIA_Selected, &data);
       RenderSize = (data) ? 0: -1;

       get(PR_Win->BT_RenderHalf, MUIA_Selected, &data);
       if ((data && RenderSize == 0) || (! data && RenderSize != 0))
        PR_Win->RndrHBlock = 1;
       set(PR_Win->BT_RenderHalf, MUIA_Selected, (RenderSize == -1));

       get(PR_Win->BT_RenderQuarter, MUIA_Selected, &data);
       if (data)
        PR_Win->RndrQBlock = 1;
       set(PR_Win->BT_RenderQuarter, MUIA_Selected, FALSE);
       } /* if no blocking */
      PR_Win->RndrFBlock = 0;
      break;
      } /* full size */

     case ID_PR_SAVEBIN:
      {
      SaveAscii = 0;
      set(PR_Win->BT_SaveAsc, MUIA_Selected, FALSE);
      break;
      } /* save params in binary format */
     case ID_PR_SAVEASC:
      {
      SaveAscii = 1;
      set(PR_Win->BT_SaveBin, MUIA_Selected, FALSE);
      break;
      } /* save params in ascii format */
     case ID_PR_CLOSE:
      {
      Close_PR_Window();
      break;
      } /* close prefs window */
     } /* switch WCS ID */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    i = WCS_ID - ID_PR_REPTERR(0);

    get(PR_Win->BT_ERR[i], MUIA_Selected, &data);
    ReportMesg[i] = data;
    break;
    } /* Buttons2 */
   } /* switch gadget group */

} /* Handle_PR_Window() */

