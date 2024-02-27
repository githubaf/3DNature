/* DiagnosticGUI.c
** Diagnostic window for rendered images in World Construction Set.
** By Gary R. Huber and Chris Hanson, 1993, 1994.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

void Open_Diagnostic_Window(struct Window *EcoWin, char *WinTitle)
{
 long open;

 if ((DIAG_Win = (struct DiagnosticWindow *)
	get_Memory(sizeof (struct DiagnosticWindow), MEMF_CLEAR)) == NULL)
   return;

  Set_Param_Menu(10);

     DIAG_Win->DiagnosticWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_DIAG_DIAGNOSTICDATA ),  // "Diagnostic Data"
      MUIA_Window_ID		, MakeID('D','I','A','G'),
      MUIA_Window_Screen	, WCSScrn,

      WindowContents, VGroup,
	Child, DIAG_Win->TitleText = TextObject, TextFrame,
		MUIA_Text_Contents, WinTitle, End,
        Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_DISTANCE )),  // "  Distance"
	  Child, DIAG_Win->Txt[0] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_ELEVATION )),  // " Elevation"
	  Child, DIAG_Win->Txt[1] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_OVERSTORY )),  // " Overstory"
	  Child, DIAG_Win->Txt[2] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_UNDERSTORY )),  // "Understory"
	  Child, DIAG_Win->Txt[3] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_RELEL )),  // "    Rel El"
	  Child, DIAG_Win->Txt[4] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_ASPECT )),  // "    Aspect"
	  Child, DIAG_Win->Txt[5] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_SLOPE )),  // "     Slope"
	  Child, DIAG_Win->Txt[6] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_SUNANGLE )),  // " Sun Angle"
	  Child, DIAG_Win->Txt[7] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_LATITUDE )),  // "  Latitude"
	  Child, DIAG_Win->Txt[8] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, Label1(GetString( MSG_DIAG_LONGITUDE )),  // " Longitude"
	  Child, DIAG_Win->Txt[9] = TextObject, TextFrame,
		MUIA_FixWidthTxt, "012345678901", End,
	  End, /* HGroup */

	Child, HGroup,
          Child, DIAG_Win->BT_Database = KeyButtonFunc('b', (char*)GetString( MSG_DIAG_DATABASE )),  // "\33cDatabase"
          Child, DIAG_Win->BT_Digitize = KeyButtonFunc('d', (char*)GetString( MSG_DIAG_DIGITIZE )),  // "\33cDigitize"
	  End, /* HGroup */
	End, /* VGroup */
      End; /* WindowObject */

  if (! DIAG_Win->DiagnosticWin)
   {
   Close_Diagnostic_Window();
   User_Message(GetString( MSG_DIAG_RENDERDATA ),   // "Render Data",
                GetString( MSG_DIAG_OUTOFMEMORY ),  // "Out of memory!"
                GetString( MSG_DIAG_OK ),           // "OK",
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, DIAG_Win->DiagnosticWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

  DIAG_Win->EcosystemWin = EcoWin;

/* Close requests */
  DoMethod(DIAG_Win->DiagnosticWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_DIAG_CLOSE);  

/* Other buttons */
  MUI_DoNotiPresFal(app, DIAG_Win->BT_Database, ID_DE_WINDOW,
   DIAG_Win->BT_Digitize, ID_DIAG_DIGITIZE, NULL);

  if (EcoWin == RenderWind0)
   set(DIAG_Win->BT_Digitize, MUIA_Disabled, TRUE);

/* Set tab cycle chain */
  DoMethod(DIAG_Win->DiagnosticWin, MUIM_Window_SetCycleChain,
	DIAG_Win->BT_Database, DIAG_Win->BT_Digitize, NULL);

/* Open window */
  set(DIAG_Win->DiagnosticWin, MUIA_Window_Open, TRUE);
  get(DIAG_Win->DiagnosticWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_Diagnostic_Window();
   return;
   }
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(DIAG_Win->DiagnosticWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_DIAG_ACTIVATE);

} /* Open_Diagnostic_Window() */

/*********************************************************************/

void Close_Diagnostic_Window(void)
{
 if (! RenderWind0)
  {
  if (IA->Digitizing) QuitDigPerspective();
  } /* if not for render window */
 set(DIAG_Win->DiagnosticWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
 DoMethod(app, OM_REMMEMBER, DIAG_Win->DiagnosticWin);
 MUI_DisposeObject(DIAG_Win->DiagnosticWin);
 free_Memory(DIAG_Win, sizeof (struct DiagnosticWindow));
 DIAG_Win = NULL;

 if (zbuf) free_Memory(zbuf, zbufsize);
 zbuf = (float *)NULL;
 freeQCmaps();

} /* Close_Diagnostic_Window() */

/*********************************************************************/

void Handle_Diagnostic_Window(ULONG WCS_ID)
{

 if (! DIAG_Win)
 {
     return;
 }

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_DIAG_DIGITIZE:
      {
      if (IA->Digitizing)
       {
       QuitDigPerspective();
       IA->Digitizing = 0;
       set(DIAG_Win->BT_Digitize, MUIA_Text_Contents, (IPTR)GetString( MSG_DIAG_DIGITIZE ));  // "\33cDigitize"
       } /* if digitizing */
      else
       {
       if (allocvecarray(OBN, MAXOBJPTS, 0))
		IA->Digitizing = InitDigPerspective();
       if (IA->Digitizing)
        set(DIAG_Win->BT_Digitize, MUIA_Text_Contents, (IPTR)GetString( MSG_DIAG_QUITDIG ));  // "\33cQuit Dig"
       } /* else */
      break;
      } /* digitize vector */
     case ID_DIAG_CLOSE:
      {
      Close_Diagnostic_Window();
      break;
      } /* close */
     } /* switch gadget ID */
    break;
    } /* BUTTONS1 */

   } /* switch gadget group */

} /* Handle_Diagnostic_Window() */

/*********************************************************************/

void Set_Diagnostic_Point(LONG zip)
{
 long compval, remainder;
 float z;

 z = *(zbuf + zip);
 sprintf(str, "%-10f", z);
 set(DIAG_Win->Txt[0], MUIA_Text_Contents, (IPTR)str);

 compval = *(QCmap[0] + zip);
 remainder = compval / 65536;
 sprintf(str, "%-5ld", remainder);
 set(DIAG_Win->Txt[1], MUIA_Text_Contents, (IPTR)str);

 compval -= remainder * 65536;
 remainder = compval / 256;
 if (z < 10.0E+010)
  {
  if (remainder < ECOPARAMS)
   sprintf(str, "%-2s", PAR_NAME_ECO(remainder));
  else
   sprintf(str, "%-2s", GetString( MSG_DIAG_SURFACE ));  // "Surface"
  } /* if a z value */
 else
  sprintf(str, "%-2s", GetString( MSG_DIAG_NONE ));  // "None"
 set(DIAG_Win->Txt[2], MUIA_Text_Contents, (IPTR)str);

 compval -= remainder * 256;
 if (z < 10.0E+010)
  {
  if (compval < ECOPARAMS)
   sprintf(str, "%-2s", PAR_NAME_ECO(compval));
  else
   sprintf(str, "%-2s", GetString( MSG_DIAG_SURFACE ));  // "Surface"
  } /* if a z value */
 else
  sprintf(str, "%-2s", GetString( MSG_DIAG_NONE ));  // "None"
 set(DIAG_Win->Txt[3], MUIA_Text_Contents, (IPTR)str);

 compval = *(QCmap[1] + zip);
 remainder = compval / 65536 - 1000;
 sprintf(str, "%-5ld", remainder);
 set(DIAG_Win->Txt[4], MUIA_Text_Contents, (IPTR)str);

 compval -= (remainder + 1000) * 65536;
 sprintf(str, "%-4ld", compval);
 set(DIAG_Win->Txt[5], MUIA_Text_Contents, (IPTR)str);

 compval = *(QCmap[2] + zip);
 remainder = compval / 256;
 sprintf(str, "%-2ld", remainder);
 set(DIAG_Win->Txt[6], MUIA_Text_Contents, (IPTR)str);

 compval -= remainder * 256;
 sprintf(str, "%-2ld", compval);
 set(DIAG_Win->Txt[7], MUIA_Text_Contents, (IPTR)str);

 sprintf(str, "%f", *(QCcoords[0] + zip));
 set(DIAG_Win->Txt[8], MUIA_Text_Contents, (IPTR)str);

 sprintf(str, "%f", *(QCcoords[1] + zip));
 set(DIAG_Win->Txt[9], MUIA_Text_Contents, (IPTR)str);

} /* Set_Diagnostic_Point() */


