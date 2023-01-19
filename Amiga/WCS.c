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

#include <exec/types.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>

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

const char min_stack[] = "\0$STACK: 8192";  // ask for enough stack. (OS 3.14, OS3.2)
const ULONG MinStack=8192;   // abort if stack is actually smaller

// AF: 9.Dec.22 Change the images to little Endian in case of i386-aros
#ifdef __AROS__
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void FlipImageWords(struct Image *img)
{
    UWORD *ImageData=img->ImageData;
    unsigned int i;

    unsigned int WordsPerLine=img->Width/16;
    if(WordsPerLine*16 != img->Width)
    {
        WordsPerLine+=1;  // each line is organised in 16 bit words.
    }

    for(i=0;i<WordsPerLine*img->Height*img->Depth;i++)
    {
           *ImageData=(((*ImageData & 0xff) << 8) | ((*ImageData & 0xff00) >> 8));
           ImageData++;
    }
}


   #endif
#endif


int main(void)
{
    short ResetScrn = 0;
    struct WCSApp *WCSRootApp;
    struct WCSScreenMode *ScreenModes, *ModeSelect;
    char *AppBaseName;

    struct Task *me=FindTask(NULL);
    ULONG stack=(ULONG)me->tc_SPUpper-(ULONG)me->tc_SPLower;
    //printf("Stack ist %lu Bytes\n",stack);
    if(stack < MinStack)
    {
        printf("Stack to small! (%lu) Bytes\n",stack);
        printf("Please set Stack to %lu Bytes!\n",MinStack);
        return 20;
    }

// AF: 9.Dec.22 Change the images to little Endian in case of i386-aros
#ifdef __AROS__
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      FlipImageWords(&AboutScape);
      FlipImageWords(&CompRose);
      FlipImageWords(&DatabaseMod);
      FlipImageWords(&DataOpsMod);
      FlipImageWords(&MappingMod);
      FlipImageWords(&EditingMod);
      FlipImageWords(&RenderMod);
      FlipImageWords(&EC_PalGrad);
      FlipImageWords(&EC_Button2);
      FlipImageWords(&EC_Button3);
      FlipImageWords(&EC_Button4);
      FlipImageWords(&EC_Button5);
      FlipImageWords(&EC_Button6);
      FlipImageWords(&EC_Button7);
      FlipImageWords(&EC_Button8);
      FlipImageWords(&EC_Button9);
      FlipImageWords(&Gary);
      FlipImageWords(&Xenon);
      FlipImageWords(&DangerSign);

   #endif
#endif

ResetScreenMode:

if ((IntuitionBase = (struct IntuitionBase *)
 OpenLibrary((STRPTR)"intuition.library", MIN_LIBRARY_REV)))
 {
 if ((GfxBase = (struct GfxBase *)
  OpenLibrary((STRPTR)"graphics.library", MIN_LIBRARY_REV)))
  {
  if ((AslBase = OpenLibrary((STRPTR)"asl.library", MIN_LIBRARY_REV)))
   {
   if ((GadToolsBase = OpenLibrary((STRPTR)"gadtools.library", MIN_LIBRARY_REV)))
    {
    if((MUIMasterBase = OpenLibrary((STRPTR)MUIMASTER_NAME,MUIMASTER_VMIN)))
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
     #ifndef __AROS__
        __seterrno();
     #endif
     ret=-1;
  }
  return ret;
}
#else
int Mkdir(const char *name)
{
    return mkdir(name);
}
#endif




void SimpleEndianFlip64 (            double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
{
    double retVal;
    char *doubleToConvert = ( char* ) & Source64;
    char *returnDouble = ( char* ) & retVal;

    // swap the bytes into a temporary buffer
    returnDouble[0] = doubleToConvert[7];
    returnDouble[1] = doubleToConvert[6];
    returnDouble[2] = doubleToConvert[5];
    returnDouble[3] = doubleToConvert[4];
    returnDouble[4] = doubleToConvert[3];
    returnDouble[5] = doubleToConvert[2];
    returnDouble[6] = doubleToConvert[1];
    returnDouble[7] = doubleToConvert[0];

    *Dest64=retVal;

}

void SimpleEndianFlip32F(             float Source32, float  *Dest32)  // AF, 10Dec22 for i386-aros
       {
           float retVal;
           char *floatToConvert = ( char* ) & Source32;
           char *returnFloat = ( char* ) & retVal;

           // swap the bytes into a temporary buffer
           returnFloat[0] = floatToConvert[3];
           returnFloat[1] = floatToConvert[2];
           returnFloat[2] = floatToConvert[1];
           returnFloat[3] = floatToConvert[0];

           *Dest32=retVal;
       }
void SimpleEndianFlip32U( unsigned long int Source32, unsigned long int *Dest32)  // AF, 10Dec22 for i386-aros
       {
           (*Dest32) = (unsigned long int)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
                       (unsigned long int)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
       }


void SimpleEndianFlip32S(   signed long int Source32, signed long int   *Dest32)  //AF, 10Dec22 for i386-aros
       {
           (*Dest32) = ( long int)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
                       ( long int)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
       }

void SimpleEndianFlip16U(unsigned short int Source16, unsigned short int *Dest16) {(*Dest16) = (unsigned short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};
void SimpleEndianFlip16S(  signed short int Source16, signed short int   *Dest16) {(*Dest16) = (  signed short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};

// AF, 17.12.2022

ssize_t write_UShort_BigEndian (int filedes, const void *buffer, size_t size)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if(size==2)
    {
        unsigned short int Value=*(USHORT*)buffer;
        SimpleEndianFlip16U(Value,&Value);
        return write(filedes, &Value, size);
    }
    else if(size==4)
    {
        unsigned long int Value=*(ULONG*)buffer;
        SimpleEndianFlip32U(Value,&Value);
        return write(filedes, &Value, size);
    }
    else if(size==8)
    {
        double Value=*(double*)buffer;
        SimpleEndianFlip64(Value,&Value);
        return write(filedes, &Value, size);
    }
    else
    {
        KPrintF((STRPTR) "AF: wrong size for %s L:%d %s(%d)\n",__FILE__, __LINE__,__func__,size);
        return 0;
    }

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ // Big endian? Then just call the original version
    return write(filedes, buffer, size);
#else
    error Not implemented!
#endif
}

