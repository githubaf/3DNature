/* WCS.c (ne gis.c 14 Jan 1994 CXH)
** The Main Vein, The Real Deal. The Right Stuff. The terrain renderer program.
** Hacked up into little bitty bloody bits and header files and reintegrated
** with stuff from map...* on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent wrinkles, crinkles and crenulations
** by Gary R. Huber.
** Initialization and event loop here, in support.c and gui.c recoded on
** 23 Mar 1994 by CXH.
*/

#define MAIN

#include "WCS.h"
#include "Version.h"

#define MIN_LIBRARY_REV 37
#define DITHER_TABLE_SIZE 4096

/*
 * AF: We use only Mui 3.8 stuff but bebbo's gcc 6 has MUI-5 SDK. So it sets MUIMASTER_VMIN=20 while 19 would be enough.
 * A new define in the Makefile now overwrites the actual minimum required version from the SDK by 19.
 */
#ifdef FORCE_MUIMASTER_VMIN       /* use -DFORCE_MUIMASTER_VMIN=19 on the gcc commandline or in Makefile */
    #undef MUIMASTER_VMIN
    #define MUIMASTER_VMIN FORCE_MUIMASTER_VMIN
#endif


STATIC_FCN struct ColorSpec NewAltColors[]=
{
  {0x00, 0x08, 0x09, 0x0b},
  {0x01, 0x00, 0x00, 0x00},
  {0x02, 0x0d, 0x0d, 0x0d},
  {0x03, 0x0b, 0x01, 0x00},
  {0x04, 0x03, 0x04, 0x08},
  {0x05, 0x03, 0x09, 0x02},
  {0x06, 0x03, 0x07, 0x0c},
  {0x07, 0x0d, 0x0d, 0x02},
  {0x08, 0x0f, 0x0f, 0x0f},
  {0x09, 0x0d, 0x0d, 0x0d},
  {0x0a, 0x0b, 0x0b, 0x0b},
  {0x0b, 0x09, 0x09, 0x09},
  {0x0c, 0x07, 0x07, 0x07},
  {0x0d, 0x05, 0x05, 0x05},
  {0x0e, 0x03, 0x03, 0x03},
  {0x0f, 0x01, 0x01, 0x01},
  {-1, 0, 0, 0}
  }; /* NewAltColors */

STATIC_FCN WORD PenSpec[]=
{
  6,    /* cycle dropdowns text normal           */
  2,    /* cycle dropdowns text highlighted      */
  1,    /* text, window title inactive,
    cycle drop downs background normal       */
  2,    /* shine                     */
  4,    /* shadow, active gadget outline         */
  6,    /* window frame fill active          */
  1,    /* window title active               */
  0,    /* cycle dropdowns background highlighted    */
  3,    /* special text                  */
  ~0    /* ???                       */
  };


unsigned long long Swap1=0,Swap2=0,Swap4=0,Swap8=0,Swapother=0, SwapTotal=0;  // ALEXANDER: Test fuer swapmem()

int main(void)
{
short ResetScrn = 0;
struct WCSApp *WCSRootApp;
struct WCSScreenMode *ScreenModes, *ModeSelect;
char *AppBaseName;

ResetScreenMode:

if (IntuitionBase = (struct IntuitionBase *)
 OpenLibrary((STRPTR)"intuition.library", MIN_LIBRARY_REV))
 {
 if (GfxBase = (struct GfxBase *)
  OpenLibrary((STRPTR)"graphics.library", MIN_LIBRARY_REV))
  {
  if (AslBase = OpenLibrary((STRPTR)"asl.library", MIN_LIBRARY_REV))
   {
   if (GadToolsBase = OpenLibrary((STRPTR)"gadtools.library", MIN_LIBRARY_REV))
    {
    if(MUIMasterBase = OpenLibrary((STRPTR)MUIMASTER_NAME,MUIMASTER_VMIN))
     {
     getcwd(path, 255);

     memset(&ScrnData, 0, sizeof (struct WCSScreenData));
     if (! LoadProject("WCS.Prefs", &ScrnData, 0))
      {
      strcpy(dirname, "WCSProjects:");
      strcpy(dbasepath, "WCSProjects:");
      strcpy(parampath, "WCSProjects:");
      strcpy(framepath, "WCSFrames:");
      strcpy(framefile, "Frame");
      strcpy(linepath, "WCSFrames:");
      strcpy(linefile, "Lines");
      strcpy(zbufferpath, "WCSProjects:");
      strcpy(backgroundpath, "WCSProjects:");
      strcpy(graphpath, "WCSFrames:");
      strcpy(colormappath, "WCSProjects:");
      strcpy(statpath, "WCSProjects:");
      strcpy(temppath, "WCSFrames:");
      strcpy(tempfile, "TempFrame");
      strcpy(modelpath, "WCSProjects:");
      strcpy(cloudpath, "WCSProjects:");
      strcpy(wavepath, "WCSProjects:");
      strcpy(deformpath, "WCSProjects:");
      strcpy(imagepath, "WCSFrames:");
      } /* if */
     if (ResetScrn)
      {
      ResetScrn = 0;
      ScrnData.ModeID = 0;
      } /* if user wishes to reset screen mode */
     
     if((WCSRootApp = WCS_App_New()))
      {
      app = WCSRootApp->MUIApp;
      
      get(app, MUIA_Application_Base, &AppBaseName);
      
      if (ScrnData.ModeID == 0)
       {
       ModeSelect = NULL;
       ScrnData.AutoTag = TAG_IGNORE;
       ScrnData.AutoVal = 0;
       if((ScreenModes = ModeList_New()))
        {
        if((ModeSelect = ModeList_Choose(ScreenModes, &ScrnData)))
         {
         if(ModeSelect->OX > ModeSelect->X)
          { /* Enable Oscan */
          ScrnData.OTag = SA_Overscan;
          ScrnData.OVal = OSCAN_TEXT;
          if(ModeSelect->OX == ModeSelect->OScans[1].x)
           ScrnData.OVal = OSCAN_STANDARD;
          if(ModeSelect->OX == ModeSelect->OScans[2].x)
           ScrnData.OVal = OSCAN_MAX;
          if(ModeSelect->OX == ModeSelect->OScans[3].x)
           ScrnData.OVal = OSCAN_VIDEO;
          if(ModeSelect->UX > ModeSelect->OX)
           {
           ScrnData.AutoTag = SA_AutoScroll;
           ScrnData.AutoVal = TRUE;
           } /* if */
          } /* if */
         else
          { /* Turn off OverScan */
          ScrnData.OTag = TAG_IGNORE;
          ScrnData.OVal = 0;
          } /* else */
         WCSScrn = OpenScreenTags(NULL, SA_DisplayID, ModeSelect->ModeID,
          SA_Width, ModeSelect->UX, SA_Height, ModeSelect->UY,
          SA_Depth, 4, SA_Title, (ULONG)APP_TITLE, SA_Type, CUSTOMSCREEN,
          ScrnData.OTag, ScrnData.OVal, ScrnData.AutoTag, (ULONG)ScrnData.AutoVal, SA_Colors, (ULONG)NewAltColors,
          SA_Pens, (ULONG)PenSpec, SA_PubName, (ULONG)AppBaseName, TAG_END);
         } /* if */
        else
         {
         WCSScrn = NULL; /* This'll make it exit. */
         } /* else */
        ModeList_Del(ScreenModes);
        ScreenModes = NULL;
        } /* if */
       else
        { /* Can't get screenmodes, default: NTSC-Hires-Lace */
        WCSScrn = OpenScreenTags(NULL, SA_DisplayID, HIRESLACE_KEY,
         SA_Width, STDSCREENWIDTH, SA_Height, STDSCREENHEIGHT,
         SA_Depth, 4, SA_Title, (ULONG)APP_TITLE, SA_Type, CUSTOMSCREEN,
         SA_Overscan, OSCAN_MAX, SA_Colors, (ULONG)NewAltColors,
         SA_Pens, (ULONG)PenSpec, SA_PubName, (ULONG)AppBaseName, TAG_END);
        } /* else */
       } /* if no screen data in WCS.Prefs */
      else
       {
       WCSScrn = OpenScreenTags(NULL, SA_DisplayID, ScrnData.ModeID,
        SA_Width, ScrnData.Width, SA_Height, ScrnData.Height,
        SA_Depth, 4, SA_Title, (ULONG)APP_TITLE, SA_Type, CUSTOMSCREEN,
        ScrnData.OTag, ScrnData.OVal, ScrnData.AutoTag, ScrnData.AutoVal, SA_Colors, (ULONG)NewAltColors,
        SA_Pens, (ULONG)PenSpec, SA_PubName, (ULONG)AppBaseName, TAG_END);
       } /* else read screen data from prefs file */

      if(WCSScrn)
       {
       DTable = DitherTable_New(DITHER_TABLE_SIZE);
       PubScreenStatus(WCSScrn, 0);

       if (DTable)
        {
        if (! DL)
         DL = DirList_New(dirname, 0);

        if (DL)
         {
         if(WCS_App_Startup(WCSRootApp))
          {
          ResetScrn = WCS_App_EventLoop(WCSRootApp); /* Fa la la la la, la la la la! */
          WCS_App_Del(WCSRootApp); /* Erk. Aaaaarrrrggggh... */
          WCSRootApp = NULL;
          app = NULL;
          } /* if */
         SaveProject(0, "WCS.Prefs", &ScrnData);
         DirList_Del(DL);
         DL = NULL;
	 } /* if */
        DitherTable_Del(DTable, DITHER_TABLE_SIZE);
        DTable = NULL;
        } /* if dither table */

       CloseScreen(WCSScrn);
       WCSScrn = NULL;
       } /* if */
      if(WCSRootApp) /* May have already shut down above. */
       {
       WCS_App_Del(WCSRootApp);
       WCSRootApp = NULL;
       } /* if */
      } /* if */
     CloseLibrary(MUIMasterBase);
     MUIMasterBase = NULL;
     } /* if */
    else
     {
     printf("FATAL ERROR: MUIMaster.Library revision %d required. Aborting.\n", MUIMASTER_VMIN);
     } /* else */
    CloseLibrary(GadToolsBase);
    GadToolsBase = NULL;
    }
   else
    {
    printf("FATAL ERROR: GadTools.Library revision %d required. Aborting.\n", MIN_LIBRARY_REV);  // AF: was incorrectly MUIMASTER_VMIN
    } /* else */
   CloseLibrary(AslBase);
   AslBase = NULL;
   } /* if */
  else
   {
   printf("FATAL ERROR: ASL.Library revision %d required. Aborting.\n", MIN_LIBRARY_REV);
   } /* else */
  CloseLibrary((struct Library*)GfxBase);
  GfxBase = NULL;
  } /* if */
 else
  {
  printf("FATAL ERROR: Graphics.Library revision %d required. Aborting.\n", MIN_LIBRARY_REV);
  } /* else */
 CloseLibrary((struct Library *)IntuitionBase);
 IntuitionBase = NULL;
 } /* if */
else
 {
 printf("FATAL ERROR: Intuition.library revision %d required. Aborting.\n", MIN_LIBRARY_REV);
 } /* else */


//Cleanup:
 if (DL) DirList_Del(DL);
 DL = NULL;
 if (KF) free_Memory(KF, KFsize);
 KF = NULL;
 if (UndoKF) free_Memory(UndoKF, UndoKFsize);
 UndoKF = NULL;
 if (KT) FreeKeyTable();
 KT = NULL;
 DisposeEcotypes();
#ifdef MEMTRACK
 if (MemTrack)
  printf("\n**** Memory Not Returned to System : %d bytes ****\n\n", MemTrack);
#endif /* MEMTRACK */

 if (ResetScrn)
  {
  dbaseloaded = 0;
  paramsloaded = 0;
  goto ResetScreenMode;
  } /* if user wishes to reset screen mode */

 printf("swmem(total)=%9llu\n",SwapTotal);
 printf("swmem(1)    =%9llu (%4.1f%%)\n",Swap1,(Swap1*100.0)/SwapTotal);   // %4.1f -> 4 chars in total, 1 dot, 1 digit after the dot and 2 before
 printf("swmem(2)    =%9llu (%4.1f%%)\n",Swap2,(Swap2*100.0)/SwapTotal);
 printf("swmem(4)    =%9llu (%4.1f%%)\n",Swap4,(Swap4*100.0)/SwapTotal);
 printf("swmem(8)    =%9llu (%4.1f%%)\n",Swap8,(Swap8*100.0)/SwapTotal);
 printf("swmem(other)=%9llu (%4.1f%%)\n",Swapother,(Swapother*100.0)/SwapTotal);


 return 0;



} /* main() */


#ifdef __GNUC__
// ALEXANDER
// SAS/C has a mkdir() function with path parameter only. (gcc has additional mode parameter)
// This is a modified version from projects/libnix/sources/nix/extra/mkdir.c
extern void __seterrno(void);
int Mkdir(const char *name)
{
  BPTR fl;
  int ret;

  if ((fl=CreateDir((STRPTR)name)))
  {
    ret=0;
  }
  else
  {
    __seterrno(); ret=-1;
  }
  return ret;
}
#else
int Mkdir(const char *name)
{
    return mkdir(name);
}
#endif

