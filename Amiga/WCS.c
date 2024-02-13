/* WCS.c (ne gis.c 14 Jan 1994 CXH)
** The Main Vein, The Real Deal. The Right Stuff. The terrain renderer program.
** Hacked up into little bitty bloody bits and header files and reintegrated
** with stuff from map...* on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent wrinkles, crinkles and crenulations
** by Gary R. Huber.
** Initialization and event loop here, in support.c and gui.c recoded on
** 23 Mar 1994 by CXH.
*/

#define CATCOMP_NUMBERS 1
//#define CATCOMP_STRINGS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "WCS_locale.h" // prototypes Locale_Open(), Locale_Close() and GetString()


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

// AF, 23.Feb,2023
// include <time.h> breaks GST on SAS/C
// (AGUI.c(!) no longer compiler clean when WCS.c includes time.h)
// therefore use wrappers here. defined in Version.c
unsigned long cvt_TIME(char const *time);
unsigned long get_time(unsigned long *result);

int main(void)
{
    short ResetScrn = 0;
    struct WCSApp *WCSRootApp;
    struct WCSScreenMode *ScreenModes, *ModeSelect;
    char *AppBaseName;

    struct Task *me=FindTask(NULL);
    ULONG stack=(IPTR)me->tc_SPUpper-(IPTR)me->tc_SPLower;
    //printf("Stack ist %lu Bytes\n",stack);
    if(stack < MinStack)
    {
        printf("Stack to small! (%lu) Bytes\n",(unsigned long)stack);
        printf("Please set Stack to %lu Bytes!\n",(unsigned long)MinStack);
        return 20;
    }

    Locale_Open("WCS.catalog",1,1);  // Version, revision  - Simplecat Doc says: There is no need to check any result.
    // set correct locale strings to the menu
	WCSNewMenus[ 0].nm_Label= GetString(MSG_MENU_PROJECT);
	WCSNewMenus[ 1].nm_Label= GetString(MSG_MENU_PR_NEW);
	WCSNewMenus[ 2].nm_Label= GetString(MSG_MENU_PR_EDIT);
	WCSNewMenus[ 3].nm_Label= GetString(MSG_MENU_PR_OPEN);
	WCSNewMenus[ 4].nm_Label= GetString(MSG_MENU_PR_SAVE);
	WCSNewMenus[ 5].nm_Label= GetString(MSG_MENU_PR_SAVEAS);
	WCSNewMenus[ 6].nm_Label= GetString(MSG_MENU_PR_LOADCONFIG);
	WCSNewMenus[ 7].nm_Label= GetString(MSG_MENU_PR_SAVECONFIG);
	WCSNewMenus[ 8].nm_Label= GetString(MSG_MENU_PR_SAVESCREEN);
//	WCSNewMenus[ 9].nm_Label= NM_BARLABEL,	 0 , 0, 0, 0 };
	WCSNewMenus[10].nm_Label= GetString(MSG_MENU_PR_INFO);
	WCSNewMenus[11].nm_Label= GetString(MSG_MENU_PR_VERSION);
	WCSNewMenus[12].nm_Label= GetString(MSG_MENU_PR_CREDITS);
	WCSNewMenus[13].nm_Label= GetString(MSG_MENU_PR_LOG);
//	WCSNewMenus[14].nm_Label= NM_BARLABEL,	 0 , 0, 0, 0 },
	WCSNewMenus[15].nm_Label= GetString(MSG_MENU_PR_QUIT);
	WCSNewMenus[16].nm_Label= GetString(MSG_MENU_PR_ICONIFY);

	WCSNewMenus[17].nm_Label= GetString(MSG_MENU_MODULES);
	WCSNewMenus[18].nm_Label= GetString(MSG_MENU_MOD_DATABASE);
	WCSNewMenus[19].nm_Label= GetString(MSG_MENU_MOD_DATAOPS);
	WCSNewMenus[20].nm_Label= GetString(MSG_MENU_MOD_MAPVIEW);
	WCSNewMenus[21].nm_Label= GetString(MSG_MENU_MOD_PARAMETERS);
	WCSNewMenus[22].nm_Label= GetString(MSG_MENU_MOD_RENDER);
//	WCSNewMenus[23].nm_Label= NM_BARLABEL,	 0 , 0, 0, 0 },
	WCSNewMenus[24].nm_Label= GetString(MSG_MENU_MOD_MOTIONEDITOR);
	WCSNewMenus[25].nm_Label= GetString(MSG_MENU_MOD_COLOREDITOR);
	WCSNewMenus[26].nm_Label= GetString(MSG_MENU_MOD_ECOSYSEDITOR);

	WCSNewMenus[27].nm_Label= GetString(MSG_MENU_PREFS);
	WCSNewMenus[28].nm_Label= GetString(MSG_MENU_PREF_PREFERENCES);
	WCSNewMenus[29].nm_Label= GetString(MSG_MENU_PREF_SCREENMODE);

	WCSNewMenus[30].nm_Label= GetString(MSG_MENU_PARAMETERS);
	WCSNewMenus[31].nm_Label= GetString(MSG_MENU_PAR_LOADALL);
	WCSNewMenus[32].nm_Label= GetString(MSG_MENU_PAR_SAVEALL);
	WCSNewMenus[33].nm_Label= GetString(MSG_MENU_PAR_FREEZE);
	WCSNewMenus[34].nm_Label= GetString(MSG_MENU_PAR_RESTORE);

//    WCSNewMenus[35].nm_Label=	NULL,		 0 , 0, 0, 0 },
//	WCSNewMenus[36].nm_Label= 	NULL,		(STRPTR)"[", 0, 0, 0 },
//	WCSNewMenus[37].nm_Label=	NULL,		(STRPTR)"]", 0, 0, 0 },
//	WCSNewMenus[38].nm_Label= (STRPTR)"Load Active...";
//	WCSNewMenus[39].nm_Label= (STRPTR)"Save Active...";

//    WCSNewMenus[40].nm_Label= 	NULL,		 0 , 0,	0, 0 },

	// Ecosystem Editor -> Class
	typename[0]=  (char*)GetString( MSG_TYPENAME_WATER );
	typename[1]=  (char*)GetString( MSG_TYPENAME_SNOW );
	typename[2]=  (char*)GetString( MSG_TYPENAME_ROCK );
	typename[3]=  (char*)GetString( MSG_TYPENAME_GROUND );

	typename[4]=  (char*)GetString( MSG_TYPENAME_CONIFER );
	typename[5]=  (char*)GetString( MSG_TYPENAME_DECID );
	typename[6]=  (char*)GetString( MSG_TYPENAME_LOWVEG );
	typename[7]=  (char*)GetString( MSG_TYPENAME_SNAG );
	typename[8]=  (char*)GetString( MSG_TYPENAME_STUMP );

	varname[ 0]=   (char*)GetString( MSG_VARNAME_CAMERAALTITUDE );
	varname[ 1]=   (char*)GetString( MSG_VARNAME_CAMERALATITUDE );
	varname[ 2]=   (char*)GetString( MSG_VARNAME_CAMERALONGITUDE );
	varname[ 3]=   (char*)GetString( MSG_VARNAME_FOCUSALTITUDE );
	varname[ 4]=   (char*)GetString( MSG_VARNAME_FOCUSLATITUDE );
	varname[ 5]=   (char*)GetString( MSG_VARNAME_FOCUSLONGITUDE );
	varname[ 6]=   (char*)GetString( MSG_VARNAME_CENTERX );
	varname[ 7]=   (char*)GetString( MSG_VARNAME_CENTERY );
	varname[ 8]=   (char*)GetString( MSG_VARNAME_BANK );
	varname[ 9]=   (char*)GetString( MSG_VARNAME_EARTHROTATION );
	varname[10]=   (char*)GetString( MSG_VARNAME_SCALE );
	varname[11]=   (char*)GetString( MSG_VARNAME_VIEWARC );
	varname[12]=   (char*)GetString( MSG_VARNAME_FLATTENING );
	varname[13]=   (char*)GetString( MSG_VARNAME_DATUM );
	varname[14]=   (char*)GetString( MSG_VARNAME_VERTICALEXAG );
	varname[15]=   (char*)GetString( MSG_VARNAME_SUNLIGHTLAT );
	varname[16]=   (char*)GetString( MSG_VARNAME_SUNLIGHTLON );
	varname[17]=   (char*)GetString( MSG_VARNAME_HORIZONLINE );
	varname[18]=   (char*)GetString( MSG_VARNAME_HORIZONPOINT );
	varname[19]=   (char*)GetString( MSG_VARNAME_HORIZONSTRETCH );
	varname[20]=   (char*)GetString( MSG_VARNAME_HAZESTART);
	varname[21]=   (char*)GetString( MSG_VARNAME_HAZERANGE );
	varname[22]=   (char*)GetString( MSG_VARNAME_SHADEFACTOR );
	varname[23]=   (char*)GetString( MSG_VARNAME_FOGNONE );
	varname[24]=   (char*)GetString( MSG_VARNAME_FOGFULL );
	varname[25]=   (char*)GetString( MSG_VARNAME_ZMINIMUM);
	varname[26]=   (char*)GetString( MSG_VARNAME_SUNLAT );
	varname[27]=   (char*)GetString( MSG_VARNAME_SUNLON );
	varname[28]=   (char*)GetString( MSG_VARNAME_SUNSIZE );
	varname[29]=   (char*)GetString( MSG_VARNAME_MOONLAT );
	varname[30]=   (char*)GetString( MSG_VARNAME_MOONLON );
	varname[31]=   (char*)GetString( MSG_VARNAME_MOONSIZE );
	varname[32]=   (char*)GetString( MSG_VARNAME_REFLECTION );


	StdMesg[ 0]=   (char*)GetString( MSG_STDMESG_OUTOFMEMORY );
	StdMesg[ 1]=   (char*)GetString( MSG_STDMESG_OPENFILEFAILED );
	StdMesg[ 2]=   (char*)GetString( MSG_STDMESG_READFILEFAILED );
	StdMesg[ 3]=   (char*)GetString( MSG_STDMESG_WRITINGTOFILEFAILED );
	StdMesg[ 4]=   (char*)GetString( MSG_STDMESG_WRONGFILETYPE );
	StdMesg[ 5]=   (char*)GetString( MSG_STDMESG_ILLEGALINSTRUCTION );
	StdMesg[ 6]=   (char*)GetString( MSG_STDMESG_ILLEGALVALUE );
	StdMesg[ 7]=   (char*)GetString( MSG_STDMESG_FILENOTLOADED );
	StdMesg[ 8]=   (char*)GetString( MSG_STDMESG_OPENFILEFAILED );
	StdMesg[ 9]=   (char*)GetString( MSG_STDMESG_READFILEFAILED );
	StdMesg[10]=   (char*)GetString( MSG_STDMESG_WRONGFILETYPE );
	StdMesg[11]=   (char*)GetString( MSG_STDMESG_ILLEGALINSTRUCTION );
	StdMesg[12]=   (char*)GetString( MSG_STDMESG_ILLEGALVALUE );
	StdMesg[13]=   (char*)GetString( MSG_STDMESG_MODULENOTIMPLEMENTED );
	StdMesg[14]=   (char*)GetString( MSG_STDMESG_GUINOTIMPLEMENTED );
	StdMesg[15]=   (char*)GetString( MSG_STDMESG_PARAMETERFILELOADED );
	StdMesg[16]=   (char*)GetString( MSG_STDMESG_PARAMETERFILESAVED );
	StdMesg[17]=   (char*)GetString( MSG_STDMESG_DATABASEFILELOADED );
	StdMesg[18]=   (char*)GetString( MSG_STDMESG_DATABASEFILESAVED );
	StdMesg[19]=   (char*)GetString( MSG_STDMESG_DEMFILELOADED );
	StdMesg[20]=   (char*)GetString( MSG_STDMESG_DEMFILESAVED );
	StdMesg[21]=   (char*)GetString( MSG_STDMESG_VECTORFILELOADED );
	StdMesg[22]=   (char*)GetString( MSG_STDMESG_VECTORFILESAVED );
	StdMesg[23]=   (char*)GetString( MSG_STDMESG_IMAGEFILELOADED );
	StdMesg[24]=   (char*)GetString( MSG_STDMESG_IMAGEFILESAVED );
	StdMesg[25]=   (char*)GetString( MSG_STDMESG_COLORMAPFILELOADED );
	StdMesg[26]=   (char*)GetString( MSG_STDMESG_COLORMAPFILESAVED );
	StdMesg[27]=   (char*)GetString( MSG_STDMESG_FILENOTLOADED );
	StdMesg[28]=   (char*)GetString( MSG_STDMESG_ENABLE_SOFT_OUTLINE_STYLE );
	StdMesg[29]=   (char*)GetString( MSG_STDMESG_MAPPINGMODULE );
	StdMesg[30]=   (char*)GetString( MSG_STDMESG_DIRECTORYNOTFOUND );
	StdMesg[31]=   (char*)GetString( MSG_STDMESG_OPENWINDOWFAILED );
	StdMesg[32]=   (char*)GetString( MSG_STDMESG_EMPTY );
	StdMesg[33]=   (char*)GetString( MSG_STDMESG_EMPTY );
	StdMesg[34]=   (char*)GetString( MSG_STDMESG_INCORRECTFILESIZE );
	StdMesg[35]=   (char*)GetString( MSG_STDMESG_OPENWINDOWFAILED );
	StdMesg[36]=   (char*)GetString( MSG_STDMESG_INCORRECTFILESIZE );
	StdMesg[37]=   (char*)GetString( MSG_STDMESG_INCORRECTFILEVERSION );
	StdMesg[38]=   (char*)GetString( MSG_STDMESG_RELATIVEELEVATIONFILESAVED );
	StdMesg[39]=   (char*)GetString( MSG_STDMESG_EMPTY );
	StdMesg[40]=   (char*)GetString( MSG_STDMESG_VECTOROBJECTSLOADED );
	StdMesg[41]=   (char*)GetString( MSG_STDMESG_PROJECTFILELOADED );
	StdMesg[42]=   (char*)GetString( MSG_STDMESG_PROJECTFILESAVED );
	StdMesg[43]=   (char*)GetString( MSG_STDMESG_DIRECTORYLISTLOADED );
	StdMesg[44]=   (char*)GetString( MSG_STDMESG_INCORRECTFILEVERSION );
	StdMesg[45]=   (char*)GetString( MSG_STDMESG_RENDERTIMEFORFRAME );
	StdMesg[46]=   (char*)GetString( MSG_STDMESG_RENDERTIMEFORANIM );
	StdMesg[47]=   (char*)GetString( MSG_STDMESG_EMPTY );

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
#if defined __AROS__

#ifdef __x86_64
	#define MUIMASTER_LIB_REV 67
#elif defined __i386
	#define MUIMASTER_LIB_REV 55
#endif
    	if( (MUIMasterBase->lib_Version==19 && MUIMasterBase->lib_Revision>=MUIMASTER_LIB_REV))  // AF: deadw00d's muimaster.library 19.67 fixes notification-event-loops that made WCS flickering and unuseable
    	{
#endif
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
         extern char Date[]; // set in Version.c
         extern char ExtAboutVers[];  // set in Version.c

      app = WCSRootApp->MUIApp;
      
      get(app, MUIA_Application_Base, &AppBaseName);

      if(strstr(ExtAboutVers, "beta") != NULL)
      {
          printf("%lu\n",cvt_TIME(Date)+BETA_DAYS*24*60*60+102030405);  // disguise it a bit
      }
      if((strstr(ExtAboutVers, "beta") != NULL) && (cvt_TIME(Date)+BETA_DAYS*24*60*60 <get_time(NULL)))  // beta and beta period over
      {

          User_Message((CONST_STRPTR)"World Construction set",
                  (CONST_STRPTR)"Beta period expired...", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
      }
      else
      {
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
          SA_Depth, 4, SA_Title, (IPTR)APP_TITLE, SA_Type, CUSTOMSCREEN,
          ScrnData.OTag, ScrnData.OVal, ScrnData.AutoTag, (ULONG)ScrnData.AutoVal, SA_Colors, (IPTR)NewAltColors,
          SA_Pens, (IPTR)PenSpec, SA_PubName, (IPTR)AppBaseName, TAG_END);
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
         SA_Depth, 4, SA_Title, (IPTR)APP_TITLE, SA_Type, CUSTOMSCREEN,
         SA_Overscan, OSCAN_MAX, SA_Colors, (IPTR)NewAltColors,
         SA_Pens, (IPTR)PenSpec, SA_PubName, (IPTR)AppBaseName, TAG_END);
        } /* else */
       } /* if no screen data in WCS.Prefs */
      else
       {
       WCSScrn = OpenScreenTags(NULL, SA_DisplayID, ScrnData.ModeID,
        SA_Width, ScrnData.Width, SA_Height, ScrnData.Height,
        SA_Depth, 4, SA_Title, (IPTR)APP_TITLE, SA_Type, CUSTOMSCREEN,
        ScrnData.OTag, ScrnData.OVal, ScrnData.AutoTag, ScrnData.AutoVal, SA_Colors, (IPTR)NewAltColors,
        SA_Pens, (IPTR)PenSpec, SA_PubName, (IPTR)AppBaseName, TAG_END);
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
      }   // beta not expired
      if(WCSRootApp) /* May have already shut down above. */
       {
       WCS_App_Del(WCSRootApp);
       WCSRootApp = NULL;
       } /* if */
     CloseLibrary(MUIMasterBase);
     MUIMasterBase = NULL;
    	}
#if defined __AROS__
     } // AROS muimaster.library > 19.67
	else
	{
		MUI_RequestA(NULL, NULL, 0, (CONST_STRPTR)"Error", (CONST_STRPTR)"Cancel", (CONST_STRPTR)"For WCS AROS\nmuimaster.library revision 19.67\nor higher required.",0);

	} /* else */
#endif
	CloseLibrary(MUIMasterBase);
	MUIMasterBase=NULL;
	CloseLibrary(GadToolsBase);
	GadToolsBase = NULL;
	}

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

 Locale_Close();

 return 0;



} /* main() */






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

void SimpleEndianFlip16U(unsigned short int Source16, unsigned short int *Dest16) {(*Dest16) = (unsigned short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
void SimpleEndianFlip16S(  signed short int Source16, signed short int   *Dest16) {(*Dest16) = (  signed short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}


// AF, 4.Feb.2024: currently remove() contains an unwanted puts() (leftiver from libnix debugging?) So we overwrite is here without this puts()
// copied from ./libnix/sources/nix/stdio/remove.c to overwrite the libnix-remove() function.
// delete as soon as libnix has been fixed.
#if defined  __GNUC__ && !defined __AROS__
#include <errno.h>
#include <proto/dos.h>
#include "stdio.h"

extern void __seterrno(void);

asm("_rmdir: .global _rmdir");
asm("_unlink: .global _unlink");
extern __stdargs int remove(const char *filename)
{
#ifdef IXPATHS
        extern char *__amigapath(const char *path);
  if((filename=__amigapath(filename))==NULL)
    return -1;
#endif
//puts(filename);    // <--- here is the unwanted puts()
  if(DeleteFile((CONST_STRPTR)filename))
    return 0;
  else
  { __seterrno(); return -1; }
}
#endif
