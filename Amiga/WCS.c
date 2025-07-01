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


#define AUTOSELFTEST "7462847623" // magic argument to main() to perform automatic tests

#define MAIN

#include "WCS.h"
#include "Version.h"

#include <exec/types.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>

#ifndef __AROS__
   #include <proto/Picasso96.h>
   #include <cybergraphx/cybergraphics.h>
   #include <proto/cybergraphics.h>
#else
   #include <cybergraphx/cybergraphics.h>
   #include <proto/cybergraphics.h>
#endif

#define MIN_LIBRARY_REV 37
#define DITHER_TABLE_SIZE 4096

#include <graphics/gfx.h>

#include "AutoSelfTest.h"

/*
 * AF: We use only Mui 3.8 stuff but bebbo's gcc 6 has MUI-5 SDK. So it sets MUIMASTER_VMIN=20 while 19 would be enough.
 * A new define in the Makefile now overwrites the actual minimum required version from the SDK by 19.
 */
#ifdef FORCE_MUIMASTER_VMIN       /* use -DFORCE_MUIMASTER_VMIN=19 on the gcc commandline or in Makefile */
    #undef MUIMASTER_VMIN
    #define MUIMASTER_VMIN FORCE_MUIMASTER_VMIN
#endif

#ifdef AF_COLOR_TEST
// test neue Farben 8...15
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
  {0x08, 0x00, 0x00, 0x00},
  {0x09, 0x00, 0x00, 0x0f},
  {0x0a, 0x00, 0x0f, 0x00},
  {0x0b, 0x00, 0x0f, 0x0f},
  {0x0c, 0x0f, 0x00, 0x00},
  {0x0d, 0x0f, 0x00, 0x0f},
  {0x0e, 0x0f, 0x0f, 0x00},
  {0x0f, 0x0f, 0x0f, 0x0f},
  {-1, 0, 0, 0}
  }; /* NewAltColors */
#else

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
#endif

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

char *LocaleExtCreditText=NULL;  // here we add ExtCreditText and an optional "translatation by ..." text in case of non English GUI

struct Library	*P96Base=NULL;
struct Library *CyberGfxBase=NULL;

ScreenPixelPlotFnctPtr ScreenPixelPlot; // function pointer to specific ScreenPixelPlot()-function

struct WCSScreenData ScrnData;

// for debug-output
#include <libraries/locale.h>
extern struct LocaleBase  *LocaleBase;
extern struct Locale      *locale_locale;
extern struct Catalog     *locale_catalog;

extern void Handle_RN_Window(ULONG WCS_ID);  // called here for test directly
void SetLoadparamsForceNogetfilenameptrn(int Val);

int WCS_ReturnCode=0;  // return code for main
int Get_WCS_ReturnCode(void)
{
	return WCS_ReturnCode;
}
void Set_WCS_ReturnCode(int RetCode)
{
	WCS_ReturnCode=RetCode;
}

void Test_User_Message(void);  // Test all user Messages

char *ProjectName="empty";

int __stdargs main(int argc, char **argv)   // __stdargs needed because we compile with gcc and -mregparm
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
        printf((char*)GetString( MSG_WCS_STACKTOSMALLUBYTES ),(unsigned long)stack);         // "Stack to small! (%lu Bytes)\n"
        printf((char*)GetString( MSG_WCS_PLEASESETSTACKTOUBYTES ),(unsigned long)MinStack);  // "Please set Stack to %lu Bytes!\n"
        return 20;
    }

    initScreenPixelPlotFnct(); // set function pointer to original gray function for drawing into render window

    Locale_Open((STRPTR)"WCS.catalog",1,1);  // Version, revision  - Simplecat Doc says: There is no need to check any result.

    // some locale Debug output
    if(LocaleBase)
    {
    	if(locale_locale)
    	{
    		printf("Alexander: LocaleName:   %s\n",locale_locale->loc_LocaleName);
    		printf("Alexander: LanguageName: %s\n",locale_locale->loc_LanguageName);
    		if(locale_catalog)
    		{
    			printf("Alexander: cat_Language: %s\n",locale_catalog->cat_Language);
    			printf("Alexander: cat_Version:  %u\n",locale_catalog->cat_Version);
    			printf("Alexander: cat_Revision: %u\n",locale_catalog->cat_Revision);

    		}
    		else
    		{
    			printf("Alexander: Catalog is NULL!\n");
    		}
    	}
    	else
    	{
    		printf("Alexander: Locale is NULL!\n");
    	}
    }
    else
    {
    	printf("Alexander: LocaleBase is NULL!\n");
    }

    //asprintf(&LocaleExtCreditText,"%s%s",ExtCreditText,GetString(MSG_MENU_PR_CREDITSTRANSLATION)); // here we add ExtCreditText and an optional "translatation by ..." text in case of non English GUI
    // SAS/C does not have this function
    LocaleExtCreditText=malloc(strlen(ExtCreditText)+strlen(GetString(MSG_MENU_PR_CREDITSTRANSLATION))+1); // strlen is without trailing \0!
    if(LocaleExtCreditText)
    {
       sprintf(LocaleExtCreditText,"%s%s",ExtCreditText,GetString(MSG_MENU_PR_CREDITSTRANSLATION)); // here we add ExtCreditText and an optional "translatation by ..." text in case of non English GUI
    }

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
	WCSNewMenus[20].nm_Label= GetString(MSG_MAPGUI_MAPVIEW);
	WCSNewMenus[21].nm_Label= GetString(MSG_MENU_MOD_PARAMETERS);
	WCSNewMenus[22].nm_Label= GetString(MSG_MENU_MOD_RENDER);
//	WCSNewMenus[23].nm_Label= NM_BARLABEL,	 0 , 0, 0, 0 },
	WCSNewMenus[24].nm_Label= GetString(MSG_EDMOGUI_MOTIONEDITOR);
	WCSNewMenus[25].nm_Label= GetString(MSG_EDITGUI_COLOREDITOR);
	WCSNewMenus[26].nm_Label= GetString(MSG_MENU_MOD_ECOSYSEDITOR);

	WCSNewMenus[27].nm_Label= GetString(MSG_MENU_PREFS);
	WCSNewMenus[28].nm_Label= GetString(MSG_MENU_PREF_PREFERENCES);
	WCSNewMenus[29].nm_Label= GetString(MSG_MENU_PREF_SCREENMODE);
	WCSNewMenus[30].nm_Label= GetString(MSG_MENU_PREF_MUI_SETTINGS);


	WCSNewMenus[31].nm_Label= GetString(MSG_MENU_PARAMETERS);
	WCSNewMenus[32].nm_Label= GetString(MSG_MENU_PAR_LOADALL);
	WCSNewMenus[33].nm_Label= GetString(MSG_MENU_PAR_SAVEALL);
	WCSNewMenus[34].nm_Label= GetString(MSG_MENU_PAR_FREEZE);
	WCSNewMenus[35].nm_Label= GetString(MSG_MENU_PAR_RESTORE);

//    WCSNewMenus[365].nm_Label=	NULL,		 0 , 0, 0, 0 },
//	WCSNewMenus[37].nm_Label= 	NULL,		(STRPTR)"[", 0, 0, 0 },
//	WCSNewMenus[38].nm_Label=	NULL,		(STRPTR)"]", 0, 0, 0 },
//	WCSNewMenus[39].nm_Label= (STRPTR)"Load Active...";
//	WCSNewMenus[40].nm_Label= (STRPTR)"Save Active...";

//    WCSNewMenus[41].nm_Label= 	NULL,		 0 , 0,	0, 0 },

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
	StdMesg[32]=   " ";
	StdMesg[33]=   " ";
	StdMesg[34]=   (char*)GetString( MSG_STDMESG_INCORRECTFILESIZE );
	StdMesg[35]=   (char*)GetString( MSG_STDMESG_OPENWINDOWFAILED );
	StdMesg[36]=   (char*)GetString( MSG_STDMESG_INCORRECTFILESIZE );
	StdMesg[37]=   (char*)GetString( MSG_STDMESG_INCORRECTFILEVERSION );
	StdMesg[38]=   (char*)GetString( MSG_STDMESG_RELATIVEELEVATIONFILESAVED );
	StdMesg[39]=   " ";
	StdMesg[40]=   (char*)GetString( MSG_STDMESG_VECTOROBJECTSLOADED );
	StdMesg[41]=   (char*)GetString( MSG_STDMESG_PROJECTFILELOADED );
	StdMesg[42]=   (char*)GetString( MSG_STDMESG_PROJECTFILESAVED );
	StdMesg[43]=   (char*)GetString( MSG_STDMESG_DIRECTORYLISTLOADED );
	StdMesg[44]=   (char*)GetString( MSG_STDMESG_INCORRECTFILEVERSION );
	StdMesg[45]=   (char*)GetString( MSG_STDMESG_RENDERTIMEFORFRAME );
	StdMesg[46]=   (char*)GetString( MSG_STDMESG_RENDERTIMEFORANIM );
	StdMesg[47]=   " ";

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

P96Base=OpenLibrary((STRPTR)"Picasso96API.library",2);  // Alexander 27.8.2024: Can fail if no P96 installed. We check this when needed
if(!P96Base)
{
	CyberGfxBase=OpenLibrary((STRPTR)CYBERGFXNAME,0);  // Alexander 18.9.2024: Can fail if no CyberGfx installed. We check this when needed
}

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
                       GetString( MSG_WCS_BETAPERIODEXPIRED ),     // "Beta period expired..."
                       GetString( MSG_GLOBAL_OK ),                    // "OK"
                       (CONST_STRPTR)"o");
      }
      else
      {
      if (ScrnData.ModeID == 0)
       {
          ULONG DClipTag=TAG_IGNORE;
       ModeSelect = NULL;
       ScrnData.AutoTag = TAG_IGNORE;
       ScrnData.AutoVal = 0;
       if(argc==2 && !strcmp(argv[1],AUTOSELFTEST))
       {
    	   ScreenModes=NULL;
       }
       else
       {
    	   ScreenModes = ModeList_New();
       }
       if(ScreenModes)
        {

        if((ModeSelect = ModeList_Choose(ScreenModes, &ScrnData)))
         {
        	struct Rectangle rect;
         if(ModeSelect->OverscanTag==TAG_IGNORE)              // Overscan = None
         {
        	 if(ModeSelect->AutoVal==TRUE)    // but autoscroll enabled (X or Y bigger than visible)
        	 {
        		 QueryOverscan( ModeSelect->ModeID, &rect, ModeSelect->Overscan);
        		 DClipTag=SA_DClip;        // we need to set the clip rect
        	 }
         }
// --> Wir sind hier beim initialen Oeffnen des Screens

       //  printf("Initial OpenScreen, line %d\n",__LINE__);
       //  printf("Alexander: ModeSelect->OverscanTag=%08x\n",ModeSelect->OverscanTag);
       //  printf("Alexander: ModeSelect->Overscan=%d\n",ModeSelect->Overscan);
       //  printf("Alexander: ModeSelect->AutoTag=%08x\n",ModeSelect->AutoTag);
       //  printf("Alexander: ModeSelect->AutoVal=%d\n",ModeSelect->AutoVal);
       //  printf("Alexander: DClipTag=%08x\n",DClipTag);

         WCSScrn = OpenScreenTags(NULL,
          SA_DisplayID, ModeSelect->ModeID,
          SA_Width, ModeSelect->UX,
		  SA_Height, ModeSelect->UY,
          SA_Depth, ModeSelect->Depth,
		  SA_Title, (IPTR)APP_TITLE,
		  SA_Type, CUSTOMSCREEN,
		  ModeSelect->OverscanTag, ModeSelect->Overscan,
		  ModeSelect->AutoTag, ModeSelect->AutoVal,
	      DClipTag, (ULONG)&rect,
		  SA_Colors, (IPTR)NewAltColors,
          SA_Pens, (IPTR)PenSpec,
		  SA_PubName, (IPTR)AppBaseName,
		  TAG_END);
         } /* if */
        else
         {
         printf("Could not open selected Screenmode\n");
         WCSScrn = NULL; /* This'll make it exit. */
         } /* else */

        ModeList_Del(ScreenModes);
        ScreenModes = NULL;
        } /* if */
       else
        { /* Can't get screenmodes, default: NTSC-Hires-Lace */
        WCSScrn = OpenScreenTags(NULL,
#ifndef __AROS__
        		SA_DisplayID, HIRESLACE_KEY,   // ALEXANDER: AROS does not support HIRESLACE_KEY, STDSCREENHEIGHT seems to be 768 on AROS so big enough
#endif
         SA_Width, STDSCREENWIDTH, SA_Height, STDSCREENHEIGHT,
         SA_Depth, 4, SA_Title, (IPTR)APP_TITLE, SA_Type, CUSTOMSCREEN,
         SA_Overscan, OSCAN_MAX, SA_Colors, (IPTR)NewAltColors,
         SA_Pens, (IPTR)PenSpec, SA_PubName, (IPTR)AppBaseName, TAG_END);
        } /* else */
       } /* if no screen data in WCS.Prefs */
      else // read screen data from prefs file
       {
    	 ULONG DClipTag=TAG_IGNORE;
    	 struct Rectangle rect;

    	 if(ScrnData.OTag==TAG_IGNORE)     // Overscan = None
         {
        	 if(ScrnData.AutoVal==TRUE)    // but autoscroll enabled (X or Y bigger than visible)
        	 {
        		 QueryOverscan( ScrnData.ModeID, &rect, ScrnData.OVal);
        		 DClipTag=SA_DClip;        // we need to set the clip rect
        	 }
         }

    // printf("Alexander: ScrnData.OTag=%08x\n",ScrnData.OTag);
    // printf("Alexander: ScrnData.OVal=%d\n",ScrnData.OVal);
    // printf("Alexander: ScrnData.AutoTag=%08x\n",ScrnData.AutoTag);
    // printf("Alexander: ScrnData.AutoVal=%d\n",ScrnData.AutoVal);
    // printf("Alexander: DClipTag=%08x\n",DClipTag);

       WCSScrn = OpenScreenTags(NULL,
    		   SA_DisplayID, ScrnData.ModeID,
               SA_Width, ScrnData.Width,
			   SA_Height, ScrnData.Height,
               SA_Depth, ScrnData.Depth,
			   SA_Title, (IPTR)APP_TITLE,
			   SA_Type, CUSTOMSCREEN,
               ScrnData.OTag, ScrnData.OVal,
			   ScrnData.AutoTag, ScrnData.AutoVal,
		       DClipTag, (ULONG)&rect,
		       SA_Colors, (IPTR)NewAltColors,
               SA_Pens, (IPTR)PenSpec,
			   SA_PubName, (IPTR)AppBaseName,
		       TAG_END);
       } /* else read screen data from prefs file */

      if(WCSScrn)
       {

    	  getGfxInformation();  // Alexander Prints info about RTG, depth etc

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
           if (argc==2 && !strcmp(argv[1],AUTOSELFTEST))  // special Argument to perform test rendering and close again
           {
            // ##########################################
            AutoSelfTest(argv);                           // Render some pictures, open some windows...
            ResetScrn = 0;
            // ##########################################
           }
           else if (argc==2 && !strcmp(argv[1],"Test_User_Message"))
           {
        	   Test_User_Message();  // Test all user Messages
           }
           else
           {
            ResetScrn = WCS_App_EventLoop(WCSRootApp); /* Fa la la la la, la la la la! */
           }
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
      else
      {
    	  printf("Could not open saved Screenmode\n");
      }
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
	      sprintf(str,(char*)GetString( MSG_WCS_FORWCSAROSUIMASTERLIBRARYREVISION1967RHIGHERREQUIRED ),19,67);
	      MUI_RequestA(NULL, NULL, 0,
	      GetString( MSG_WCS_ERROR ),  // "Error",
	      GetString( MSG_WCS_CANCEL ), // "Cancel",
	      (CONST_STRPTR)str,  // "For WCS AROS\nmuimaster.library revision 19.67\nor higher required."
	      0);

	} /* else */
#endif
	CloseLibrary(MUIMasterBase);
	MUIMasterBase=NULL;
	CloseLibrary(GadToolsBase);
	GadToolsBase = NULL;
	}

    else
     {
     printf((char*)GetString( MSG_WCS_FATALERRORMUIMASTERLIBRARYREVISIONREQUIREDABORTING ), MUIMASTER_VMIN);  // "FATAL ERROR: MUIMaster.Library revision %d required. Aborting.\n"
     } /* else */
    CloseLibrary(GadToolsBase);
    GadToolsBase = NULL;
    }
   else
    {
    printf((char*)GetString( MSG_WCS_FATALERRORGADTOOLSLIBRARYREVISIONREQUIREDABORTING ), MIN_LIBRARY_REV);  // "FATAL ERROR: GadTools.Library revision %d required. Aborting.\n"  // AF: was incorrectly MUIMASTER_VMIN
    } /* else */
   CloseLibrary(AslBase);
   AslBase = NULL;
   } /* if */
  else
   {
   printf((char*)GetString( MSG_WCS_FATALERRORASLLIBRARYREVISIONREQUIREDABORTING ), MIN_LIBRARY_REV);  // "FATAL ERROR: ASL.Library revision %d required. Aborting.\n"
   } /* else */
  CloseLibrary((struct Library*)GfxBase);
  GfxBase = NULL;
  } /* if */
 else
  {
  printf((char*)GetString( MSG_WCS_FATALERRORGRAPHICSLIBRARYREVISIONREQUIREDABORTING ), MIN_LIBRARY_REV);  // "FATAL ERROR: Graphics.Library revision %d required. Aborting.\n"
  } /* else */
 CloseLibrary((struct Library *)IntuitionBase);
 IntuitionBase = NULL;
 } /* if */
else
 {
 printf((char*)GetString( MSG_WCS_FATALERRORINTUITIONLIBRARYREVISIONREQUIREDABORTING ), MIN_LIBRARY_REV);  // "FATAL ERROR: Intuition.library revision %d required. Aborting.\n"
 } /* else */

if(CyberGfxBase) // if we could open CyberGfx we have also to close at the end...
{
	CloseLibrary(CyberGfxBase);
	CyberGfxBase=NULL;
}
if(P96Base) // if we could open P96 we have also to close at the end...
{
	CloseLibrary(P96Base);
	P96Base=NULL;
}

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

 if(LocaleExtCreditText)  // was allocated by asprintf()
 {
	 free(LocaleExtCreditText);
	 LocaleExtCreditText=NULL;
 }

 Locale_Close();

 return WCS_ReturnCode;



} /* main() */


 void SimpleEndianFlip64 (double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
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

void SimpleEndianFlip16U(USHORT Source16, USHORT *Dest16) {(*Dest16) = (USHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
void SimpleEndianFlip16S(SHORT Source16, SHORT *Dest16) {(*Dest16) = ( SHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}

void SimpleEndianFlip32U( ULONG Source32, ULONG *Dest32)  // AF, 10Dec22 for i386-aros
{
	(*Dest32) = (ULONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			(ULONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}


void SimpleEndianFlip32S( LONG Source32, LONG *Dest32)  //AF, 10Dec22 for i386-aros
{
	(*Dest32) = ( LONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			( LONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}

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




// #############################################
void IncAndShowTestNumbers(unsigned int TestNumber, unsigned int TotalNumber)
{
    struct RastPort *rp = &WCSScrn->RastPort;
    static char TextBuffer[32];   // Room for "604/604 (100.00%)"

    SetAPen(rp, 1); // Vordergrundfarbe

    sprintf(TextBuffer, "%u/%u (%.2f%%)", TestNumber, TotalNumber, (float)TestNumber/TotalNumber*100.0f); // Formatierter Text
    Move(rp, WCSScrn->Width/2-TextLength(rp,TextBuffer,12)/2, WCSScrn->BarHeight-WCSScrn->Font->ta_YSize/2 +1);
    Text(rp, TextBuffer, strlen(TextBuffer)); // Text ausgeben
} /* TestNumbers() */

void Test_User_Message(void)
{

    unsigned int TestNumber=0, TotalTests=604; // TotalTests is the number of User_Message() calls in WCS.c

    // awk '/BEGIN USER_MESSAGE_TEST/{Start=1; next} /\/\/.*find/{next} /User_Message/{if(Start==1){print $0}}' "WCS.c" | wc -l
    // Count Test-numbers starting from here...   results in 604

    // find . -name "*.c" -exec grep -nHis "User_Message" {} \; | awk -F":" '{print $1}' | sort --unique
    //./AGUI.c
    // find . -name "AGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Parameters Module", "Out of memory!", "OK",

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) , (CONST_STRPTR)"Some default values", (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "Parameter Editing: Defaults", str, "OK|Cancel"


    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) ,  // "Parameter Editing: Defaults"
            (CONST_STRPTR) GetString( MSG_AGUI_YOUMUSTFIRSTLOADADATABASEBEFOREDEFAULTPARAMETERSCANBEC ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "You must first load a Database before Default Parameters can be computed.", "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Database Module", "Out of memory!", "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "DataOps Module", "Out of memory!", "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"World Construction Set",
            (CONST_STRPTR) GetString( MSG_AGUI_PUBLICSCREENSTILLHASVISITORSTRYCLOSINGAGAIN ) ,  // "Public Screen still has visitors. Try closing again?"
            (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2);  // "Close|Warn|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"World Construction Set",
            (CONST_STRPTR) GetString( MSG_AGUI_QUITPROGRAMREYOUSURE ) ,  // )"Quit Program\nAre you sure?"
            (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2);  // "Close|Warn|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_WCSPROJECT ) ,  // "WCS Project"
            (CONST_STRPTR) GetString( MSG_AGUI_PROJECTPATHSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Project paths have been modified. Save them before closing?"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERMODULE ) ,  // "Parameter Module"
            (CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Parameters have been modified. Save them before closing?"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
            (CONST_STRPTR) GetString( MSG_AGUI_DATABASEHASBEENMODIFIEDSAVEITBEFORECLOSING ) ,  // "Database has been modified. Save it before closing?"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_EXTRASMODULE ), (CONST_STRPTR) GetString( MSG_AGUI_NOTYETIMPLEMENTEDTAYTUNED ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "Not yet implemented.\nStay Tuned!", "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def("", (CONST_STRPTR) GetString( MSG_AGUI_KEEPCHANGES ) , (CONST_STRPTR) GetString( MSG_AGUI_KEEPCANCEL ) , (CONST_STRPTR)"kc", 1);  // "Keep changes?", "Keep|Cancel"

    //IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_EXTRASMODULE ), (CONST_STRPTR)"loadmesg", (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def("",  GetString( MSG_AGUI_FILEALREADYEXISTSOYOUWISHTOOVERWRITEIT ) ,  // "File already exists.\nDo you wish to overwrite it?"
            GetString( MSG_GLOBAL_OKCANCEL ) , "oc", 1);  // "OK|CANCEL"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGSTATUSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_CANTOPENLOGSTATUSWINDOW ) ,  // "Log Status Module", "Can't Open Log Status Window!"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGWINDOW ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Log Window", "Out of memory!", "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_AGUI_WCSSCREENMODE ) ,                           // "WCS: Screen Mode"
            GetString( MSG_AGUI_INORDERTORESETTHESCREENMODEWCSWILLHAVETOCLOSEANDREOPEN ),  // "In order to reset the screen mode WCS will have to close and re-open. Any work in progress should be saved before invoking this command.\nDo you wish to proceed now?"
            GetString( MSG_GLOBAL_OKCANCEL ),   // "OK|Cancel"
            (CONST_STRPTR)"oc");

    //./AutoSelfTest.c   // for test only. No Strings to checked

    //./BitMaps.c
    //find . -name "BitMaps.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"Image.iff",
            GetString( MSG_BITMAPS_FILEALREADYEXISTSVERWRITEIT ) , GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc");  // "File already exists!\nOverwrite it?" "OK|CANCEL"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"Image.iff",
            GetString( MSG_BITMAPS_CANTOPENIMAGEFILEFOROUTPUTPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Can't open image file for output!\nOperation terminated.", "OK"


    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"Image.iff", GetString( MSG_BITMAPS_ERRORSAVINGIMAGEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error saving image!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERRORLOADINGZBUFFERPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error loading Z Buffer!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_OUTOFMEMORYMERGINGZBUFFERPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Out of memory merging Z Buffer!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERROROPENINGZBUFFERFILEFORINPUTPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error opening Z Buffer file for input!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADZBUFFILENOTSINGLEPRECISIONFLOATINGPOI ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNot single precision floating point.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADZBUFFILENOZBODCHUNKOPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNo ZBOD chunk.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADZBUFFILENOZBUFCHUNKOPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNo ZBUF chunk.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERRORREADINGZBUFFERFILERONGSIZEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nWrong Size.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERRORLOADINGBACKGROUNDIMAGEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error loading background image!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_OUTOFMEMORYMERGINGBACKGROUNDPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Out of memory merging background!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERROROPENINGBACKGROUNDFILEFORINPUTPERATIONTERMINATE ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error opening Background file for input!\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADBCKGRNDWRONGSIZEPERATIONTERMINATE ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nWrong Size.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADBCKGRNDNOBODYCHUNKPERATIONTERMINA ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nNo BODY Chunk.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERREADBCKGRNDNOBMHDCHUNKPERATIONTERMINA ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nNo BMHD Chunk.\nOperation terminated." "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_BITMAPS_ERRORREADINGBACKGROUNDFILEOMPRESSIONERRORPERATIONTE ) , GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");  // "Error reading Background file!\nCompression error.\nOperation terminated." "OK"


    //./Cloud.c
    //   find . -name "Cloud.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR) GetString( MSG_CLOUD_CLOUDEDITORSETBOUNDS ) ,     // "Cloud Editor:Set Bounds"
            (CONST_STRPTR) GetString( MSG_CLOUD_MAPVIEWMODULEMUSTBEOPEN ) ,  // "Map View Module must be open in order to use this function. Would you like to open it now?"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) ,                 // "OK|Cancel"
            (CONST_STRPTR)"oc",1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),               // "Mapping Module: Align"
            GetString( MSG_CLOUD_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSET ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                      // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    //./CloudGUI.c
    // find . -name "CloudGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR) GetString( MSG_CLOUDGUI_MAPVIEWCLOUDS ) ,  // "Map View: Clouds"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OUTOFMEMORY ) ,    // "Out of memory!"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ) ,  // "Parameters Module: Model"
            GetString( MSG_CLOUDGUI_THECURRENTCLOUDMODELHASBEENMODIFIEDDOYOUWISHTOSAVE ) ,  // "The current Cloud Model has been modified. Do you wish to save it before closing?"
            GetString( MSG_GLOBAL_YESNO ) ,  // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,               // "Cloud Editor"
            GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ), // "Make this file the Project Cloud File?"
            GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,     // "Cloud Editor"
            GetString( MSG_CLOUDGUI_DELETEALLCLOUDKEYFRAMES ) ,  // "Delete all cloud key frames?"
            GetString( MSG_GLOBAL_OKCANCEL ) ,                 // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
            GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
            GetString( MSG_GLOBAL_YESNO ),                             // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
            GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
            GetString( MSG_GLOBAL_YESNO ) ,                            // "Yes|No",
            (CONST_STRPTR)"yn", 1);


    //./DEM.c
    //find . -name "DEM.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSDEMINTERPOLATE ) ,  // "Data Ops: DEM Interpolate"
            GetString( MSG_DEM_NOFILESSELECTED ) ,             // "No file(s) selected!"
            (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,           // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"elevfile",
            GetString( MSG_DEM_ERROROPENINGFILEFORINTERPOLATIONILENOTDEMORREMONTINUE ) ,  // "Error opening file for interpolation!\nFile not DEM or REM\nContinue?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"rootfile",
            GetString( MSG_DEM_DEMNAMEISTOOLONGTOADDANEXTRACHARACTERTODOYOUWISHTOENTER ) ,  // "DEM name is too long to add an extra character to. Do you wish to enter a new base name for the DEM or abort the interpolation?"
            GetString( MSG_DEM_NEWNAMEABORT ),                                              // "New Name|Abort"
            (CONST_STRPTR)"na", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,  // "Data Ops: Interpolate DEM"
            GetString( MSG_DEM_ERRORREADINGELEVATIONFILEONTINUE ) ,  // "Error reading elevation file!\nContinue?",
            GetString( MSG_GLOBAL_OKCANCEL ),                           // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,         // "Data Ops: Interpolate DEM"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                           // "Data Ops: Interpolate DEM"
            GetString( MSG_DEM_ERROROPENINGDEMFILEFOROUTPUTPERATIONTERMINATED ) ,  // "Error opening DEM file for output!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                              // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                  // "Data Ops: Interpolate DEM"
            GetString( MSG_DEM_ERRORWRITINGDEMFILEPERATIONTERMINATED ) ,  // "Error writing DEM file!\nOperation terminated."
            (CONST_STRPTR) GetString( MSG_GLOBAL_OK ),                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                 // "Database Module",
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                           // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                         // "Data Ops: Interpolate DEM"
            GetString( MSG_DEM_ERROROPENINGOBJECTFILEFOROUTPUTPERATIONTERMINATED ) ,  // "Error opening Object file for output!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_OUTOFMEMALLOCDEMINFOHEADERPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Info Header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_OUTOFMEMALLOCDEMINFOHEADERPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Info Header!\nOperation terminated."
            (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_75MINUTEDEMSDONOTALLLIEWITHINSAMEUTMZONEPERATIONTERMINA ) ,  // "7.5 Minute DEMs do not all lie within same UTM Zone!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_OUTOFMEMORYALLOCATINGDEMARRAYSPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Arrays!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_CANTREADDEMPROFILEHEADERPERATIONTERMINATED ) ,  // "Can't read DEM profile header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_CANTREADDEMPROFILEHEADERPERATIONTERMINATED ),  // "Can't read DEM profile header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_OUTOFMEMORYALLOCATINGTEMPORARYBUFFERPERATIONTERMINATED ),  // "Out of memory allocating temporary buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_ERRORREADINGDEMPROFILEPERATIONTERMINATED ),  // "Error reading DEM profile!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_IMPROPERDEMPROFILELENGTHPERATIONTERMINATED ),  // "Improper DEM profile length!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_OUTOFMEMORYALLOCATINGMAPBUFFERPERATIONTERMINATED ),  // "Out of memory allocating map buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                //  "OK"
            (CONST_STRPTR)"o");

    //	    IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("elevbase",
    //	   "Error opening output file!\nOperation terminated.", "OK", "o");
    //
    //	    IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("elevbase",
    //	   "Error writing to output file!\nOperation terminated.", "OK", "o");
    //
    //	    IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("elevbase",
    //	   "Error writing to output file!\nOperation terminated.", "OK", "o");
    //
    //	   IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module",
    //	   "Out of memory expanding database!\nOperation terminated.", "OK", "o");
    //
    //       IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module",
    //	   		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
    //
    //       IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("elevbase",
    //	   	"Error writing to output file!\nOperation terminated.", "OK", "o");
    //
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_ERRORCREATINGOUTPUTFILEPERATIONTERMINATED ),  // "Error creating output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_NOFILESSELECTED ),  // "No file(s) selected!",
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_CANTOPENDEMFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DEM file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"MsgHdr",
            GetString( MSG_DEM_CANTREADDEMFILEHEADERPERATIONTERMINATED ),  // "Can't read DEM file header!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"FileBase",
            GetString( MSG_DEM_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"FileBase",
            GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"FileBase",
            GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                             // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                     // "Database Module"
            GetString( MSG_DEM_OUTOFMEMORYEXPANDINGDATABASEEDITORLIST ),  // "Out of memory expanding Database Editor List!"
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"FileBase",
            GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
            GetString( MSG_DEM_BADARRAYDIMENSIONSSOMETHINGDOESNTCOMPUTEPERATIONTERMINA ),  // "Bad array dimensions! Something doesn't compute.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),     // "Mapping Module: Fix Flats"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                // "Mapping Module: Fix Flats"
            GetString( MSG_DEM_NOFLATSPOTSTOOPERATEONPERATIONTERMINATED ),  // "No flat spots to operate on!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    //	   IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Data Ops Module: DEM Create",
    //	   "Error opening file for output!\nOperation terminated.", "OK", "o");
    //
    //       IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Data Ops Module: DEM Create",
    //	   "Error writing to file!\nOperation terminated.", "OK", "o");


    //./DEMGUI.c
    // find . -name "DEMGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),  // "Map View: Build DEM"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                       // "Map View: Build DEM"
            GetString( MSG_DEMGUI_THISWINDOWMUSTREMAINOPENWHILETHEDEMGRIDDERISOPENOYOU ),  // "This window must remain open while the DEM Gridder is open!\nDo you wish to close them both?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWDEMGRIDDER ),  // "Map View: DEM Gridder"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),        // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,                         // "Map View: Build DEM"
            GetString( MSG_DEMGUI_SELECTCONTOUROBJECTSTOIMPORTANDRESELECT ),  // "Select contour objects to import and reselect \"Import\" when done."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWEXPORTCONTOURS ) ,                          // "Map View: Export Contours",
            GetString( MSG_DEMGUI_CANTOPENDATABASEEDITORWINDOWPERATIONTERMINATED ),  // "Can't open Database Editor window!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                             // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWEXPORTCONTOURS ) ,                                // "Map View: Export Contours"
            GetString( MSG_DEMGUI_EXTRACTELEVATIONVALUESFROMOBJECTNAMESLABELFIELDSORUS ),  // "Extract elevation values from Object Names, Label fields or use the values embedded in the Objects themselves?"
            GetString( MSG_DEMGUI_NAMELABELEMBEDDED ),                                     // "Name|Label|Embedded"
            (CONST_STRPTR)"nle");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,  // "Map View: Build DEM"
            GetString( MSG_DEMGUI_ERRORIMPORTINGCONTOURDATAPERATIONTERMINATED ) ,  // "Error importing contour data!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,  // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                       // "Map View: Build DEM"
            GetString( MSG_DEMGUI_ATLEASTONEOBJECTFAILEDTOLOADANDCOULDNOTBEIMPORTED ),  // "At least one Object failed to load and could not be imported."
            GetString( MSG_GLOBAL_OK ),                                                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWIMPORTCONTOURS ),         // "Map View: Import Contours"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                 // "Map View: Build DEM"
            GetString( MSG_DEMGUI_YOUDIDNOTSELECTAFILETOIMPORTPERATIONTERMINATED ),  // "You did not select a file to import!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                      // "Map View: Build DEM"
            GetString( MSG_DEMGUI_UTMZONESMAYBEFROM0TO60THESELECTEDZONEISOUTOFRANGEPER ),  // "UTM zones may be from 0 to 60! The selected zone is out of range.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    //  "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),  // "Map View: Build DEM"
            GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),   // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,                               // "Map View: Build DEM"
            GetString( MSG_DEMGUI_ERROROPENINGXYZFILETOIMPORTPERATIONTERMINATED ),  // "Error opening XYZ file to import!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ) ,                                 // "Map View: XYZ Export"
            GetString( MSG_DATAOPS_YOUMUSTSPECIFYANOUTPUTFILENAMEPERATIONTERMINATED ),  // "You must specify an output file name!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ),                                     // "Map View: XYZ Export"
            GetString( MSG_DEMGUI_ERRORWRITINGTOXYZFILEPARTIALFILEWRITTENPERATIONTERMI ),  // "Error writing to XYZ file! Partial file written.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message( GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ),                                // "Map View: XYZ Export"
            GetString( MSG_DEMGUI_UNABLETOOPENXYZFILEFOREXPORTPERATIONTERMINATED ),  // "Unable to open XYZ file for export!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK",
            (CONST_STRPTR)"o");

    //./DLG.c
    //find . -name "DLG.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                                  // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),  // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_NOFILESSELECTED ),         // "No file(s) selected!"
            GetString( MSG_GLOBAL_OK ),                      // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                     // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_CANTOPENDLGFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DLG file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                     // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_FILENOTAUSGSOPTIONALDLGPERATIONTERMINATED ),  // "File not a USGS Optional DLG!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                  // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_INAPPROPRIATEUTMZONEPERATIONTERMINATED ),  // "Inappropriate UTM Zone!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                                   // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_THISFILECONTAINSDATAINANUNSUPPORTEDREFERENCESYSTEMPERAT ),  // "This file contains data in an unsupported Reference System!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                             // "Database Module",
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),  // "Data Ops Module: Import DLG"
            GetString( MSG_DLG_ERRORSAVINGOBJECTFILEPERATIONTERMINATED ),  // "Error saving object file!\nOperation terminated"
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                             // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ) ,  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                                  // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),  // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),  // "No file(s) selected!"
            GetString( MSG_GLOBAL_OK ),                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                     // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_CANTOPENDXFFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DXF file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message( GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                               // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_IMPROPERCODEVALUEFOUNDPERATIONTERMINATEDPREMATURELY ),  // "Improper Code value found!\nOperation terminated prematurely."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                           // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),               // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_ERRORSAVINGOBJECTPERATIONTERMINATED ),  // "Error saving object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module",
            GetString( MSG_DLG_OUTOFMEMORYEXPANDINGDATABASEEDITORLISTASTITEMDOESNOTAPP ),  // "Out of memory expanding Database Editor List!\nLast item does not appear in list view."
            GetString( MSG_GLOBAL_OK ),                                                       // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),              // "Data Ops Module: Import DXF"
            GetString( MSG_DLG_ERRORSAVINGLASTOBJECTPERATIONTERMINATED ),  // "Error saving last object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),                                  // "Data Ops Module: Import WDB"
            GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),  // "Data Ops Module: Import WDB"
            GetString( MSG_DLG_NOFILESSELECTED ),              // "No file(s) selected!"
            GetString( MSG_GLOBAL_OK ),                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),                     // "Data Ops Module: Import WDB"
            GetString( MSG_DLG_CANTOPENWDBFILEFORINPUTPERATIONTERMINATED ),  // "Can't open WDB file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                                       // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                            // "Database Module"
            GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                // "Data Ops: Import WDB"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ) ,  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                          // "Data Ops: Import WDB"
            GetString( MSG_DLG_ERROROPENINGSOURCEFILEPERATIONTERMINATED ),  // "Error opening source file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                     // "Data Ops: Import WDB"
            GetString( MSG_DLG_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                         // "Data Ops: Import WDB"
            GetString( MSG_DLG_ERRORSAVINGOBJECTFILEPERATIONTERMINATED ),  // "Error saving object file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                            // "Data Ops: Import WDB"
            GetString( MSG_DLG_UNSUPPORTEDATTRIBUTECODEPERATIONTERMINATED ),  // "Unsupported attribute code!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                               // "Data Ops: Import WDB"
            GetString( MSG_DLG_OBJECTCONTAINSTOOMANYPOINTSPERATIONTERMINATED ),  // "Object contains too many points!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                                // "Data Ops: Import WDB"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    //./DataBase.c
    // find . -name "DataBase.c" -exec grep -A3 -nHis "User_Message" {} \;
    // find . -name "DataBase.c" -exec grep -A3 -nHis "User_Message" {} \;
    //     IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database: Load",
    //     "Not a WCS Database file!\nOperation terminated.", "OK", "o");
    //
    //     IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module: Append",
    //     "Out of memory allocating database!\nOperation terminated.", "OK", "o");
    //
    //      IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module: Load",
    //     "Out of memory allocating database!\nOperation terminated.", "OK", "o");
    //
    //     IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message_Def(str,
    //     "Make this the default object directory?", "OK|Cancel", "oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)dbasename, GetString( MSG_DB_ERRORSAVINGDATABASEELECTANEWDIRECTORY ),  //"Error saving database!\nSelect a new directory?"
            GetString( MSG_DB_OKCANCEL ),  // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_DB_DATABASEMODULESAVE ), (CONST_STRPTR)"Database-File",                   // "Database Module: Save"
            GetString( MSG_DB_OKCANCEL ),                                                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
            GetString( MSG_DB_ILLEGALNUMBEROFDATABASERECORDSLESSTHANONEPERATIONTERMINA ), // "Illegal number of database records: less than one!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYANTUPDATEDATABASELIST ),  // "Out of memory!\nCan't update database list."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWLOAD ),  // "Map View: Load"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),           // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGELEVATIONSOBJECTNOTLOADED ), // "Error reading elevations! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGLATITUDESOBJECTNOTLOADED ),  // "Error reading latitudes! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGLONGITUDESOBJECTNOTLOADED ),  // "Error reading longitudes! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_OUTOFMEMORYOBJECTNOTLOADED ),              // "Out of memory! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGHEADEROBJECTNOTLOADED ),        // "Error reading header! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_UNSUPPORTEDFILEVERSIONOBJECTNOTLOADED ),     // "Unsupported file version! Object not loaded."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_OUTOFMEMORYOBJECTNOTLOADED ),  // "Out of memory! Object not loaded."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_DB_DATABASEMODULENAME ),                            // "Database Module: Name"
            GetString( MSG_DB_VECTORNAMEALREADYPRESENTINDATABASERYANEWNAME ),  // "Vector name already present in database!\nTry a new name?"
            GetString( MSG_DB_OKCANCEL ),                                      // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
            GetString( MSG_DB_NOMEMORYFORVECTORCOORDINATESEWOBJECTHASBEENCREATEDBUTCAN ),  // "No memory for vector coordinates!\nNew object has been created but can not be edited until memory is available."
            GetString( MSG_GLOBAL_OK ),                                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
            GetString( MSG_DB_OUTOFMEXPDBASEEDITORLSTNEWOBJECTHASBEENCRE ),  // "Out of memory expanding Database Editor List!\nNew object has been created but will not appear in list view."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_DATABASEADDOBJECT ),  // "Database: Add Object"
            GetString( MSG_DB_NOFILESSELECTED ),    // "No file(s) selected!"
            GetString( MSG_GLOBAL_OK ),                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"newfile", GetString( MSG_DB_OBJECTMUSTENDINSUFFIXOBJ ),  // "Object must end in suffix \"Obj\"!"
            GetString( MSG_GLOBAL_OK ),  // (CONST_STRPTR)"OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)str,
            GetString( MSG_DB_OBJECTNAMEALREADYPRESENTINDATABASEUPLICATEITEMSWILLBESKI ),  // "Object name already present in database!\nDuplicate items will be skipped."
            GetString( MSG_GLOBAL_OK ),                                                        // "OK"
            (CONST_STRPTR)"o", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                 // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ), // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
            GetString( MSG_DB_OUTOFMEMEXPDBEDITLSTNEWOBJADDED ),  // "Out of memory expanding Database Editor List!\nNew object has been added but will not appear in list view.",
            GetString( MSG_GLOBAL_OK ),                                                        // "OK"
            (CONST_STRPTR)"o");

    //     IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module",
    //     "Out of memory expanding database!\nOperation terminated.", "OK", "o");
    //
    //     IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Database Module: Editor",
    //     "No memory for vector coordinates!\nNew object has been created\
    //      but can not be edited until memory is available.",
    //     "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[i].Name",
            GetString( MSG_DB_ERRORLOADINGTHISOBJECTPERATIONTERMINATED ),  // "Error loading this Object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWSAVEALL ),                                  // "Map View: Save All"
            GetString( MSG_DB_ERRORWRITINGMASTEROBJECTFILEPERATIONTERMINATED ),  // "Error writing Master Object file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                               // "Map View: Load"
            GetString( MSG_DB_OUTOFMEMORYLOADINGMASTEROBJECTFILENABLEDOBJECTSWILLBELOA ),  // "Out of memory loading Master Object File!\nEnabled Objects will be loaded individually."
            GetString( MSG_GLOBAL_OK ),                                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                     // "Map View: Load"
            GetString( MSG_DB_ERRORREADINGMASTEROBJECTFILEPERATIONTERMINATED ),  // "Error reading Master Object file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                               // "Map View: Load"
            GetString( MSG_DB_NUMBEROFOBJECTSINTHEMASTEROBJECTFILEDOESNOTMATCHTHENUMBE ),  // "Number of Objects in the Master Object file does not match the number of Objects in the current Database! Master Object file cannot be used. Objects will be loaded from individual files"
            GetString( MSG_GLOBAL_OK ),                                                        // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                   // "Map View: Load"
            GetString( MSG_DB_MDBISNOTAWCSMASTEROBJECTFILE ),  // ".MDB is not a WCS Master Object file!"
            GetString( MSG_GLOBAL_OK ),                            // "OK"
            (CONST_STRPTR)"o");

    //./DataOps.c
    // find . -name "DataOps.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                              // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_YOUMUSTSPECIFYAFILETOCONVERTPERATIONTERMINATED ),  // "You must specify a file to convert!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_YOUMUSTSPECIFYANOUTPUTFILENAMEPERATIONTERMINATED ),  // "You must specify an output file name!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_YOUMUSTSPECIFYINPUTROWSANDCOLUMNSPERATIONTERMINATED ),   // "You must specify input rows and columns!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                   // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_THEREISNODATABASETODIRECTOUTPUTENTITIESTOPERATIONTE ),  // "There is no Database to direct output entities to!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),     // "Data Ops: Convert DEM"
            (CONST_STRPTR)str,
            GetString( MSG_DATAOPS_CONTINUETRUNCATECANCEL ),   // "Continue|Truncate|Cancel"
            (CONST_STRPTR)"ntc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INCORRECTFILESIZEFORSPECIFIEDHEADERWIDTHANDHEIGHTRO ),  // "Incorrect file size for specified header, width and height!\nProceed anyway?."
            GetString( MSG_GLOBAL_OKCANCEL ) ,                                            // "OK|Cancel",
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),  // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INVERTDATAORDER ),    // "Invert Data order?"
            GetString( MSG_GLOBAL_YESNO ),              // "Yes|No"
            (CONST_STRPTR)"yn");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                           // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_UNABLETOOPENFILEFORINPUTPERATIONTERMINATED ),  // "Unable to open file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INCORRFILSIZFORSPECIFHEADERWDTHANDHIGHTPE ),  // "Incorrect file size for specified header, width and height!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                            // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_UNABLETOOPENFILEFOROUTPUTPERATIONTERMINATED ),  // "Unable to open file for output!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                              // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_ERRORWRITINGDESTINATIONFILEPERATIONTERMINATED ),  // "Error writing destination file!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                         // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_ERRORREADINGSOURCEFILEPERATIONTERMINATED ),  // "Error reading source file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                     // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_NOTACOMPRESSEDFILEPERATIONTERMINATED ),  // "Not a compressed file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
            (CONST_STRPTR)"Extended header!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                             // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INPUTFILECONFIGURATIONNOTYETSUPPORTEDPERATIONTERMIN ),  // "Input file configuration not yet supported!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INPUTDATAFORMATNOTSUPPORTEDHECKYOURSETTINGSPERATION ),  // "Input data format not supported!\nCheck your settings.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                       // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_ERRORSAVINGOBJFILEOPERATIONTERMOINATED ),  // "Error saving \".Obj\" file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_INPUTFILENOTRECOGNIZEDASADTEDFILEPERATIONTERMINATED ),  // "Input file not recognized as a DTED file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_LLEGALSOURCEVALUEFORMATSIZECOMBINATIONPERATIONTERMI ),  // "!\nIllegal source value format/size combination!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPS_LLEGALTARGETVALUEFORMATSIZECOMBINATIONPERATIONTERMI ),  // "!\nIllegal target value format/size combination!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    //./DataOpsGUI.c
    // find . -name "DataOpsGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DEMCONVERTER ),  // "DEM Converter"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)DC_Win->InFile,
            GetString( MSG_DATAOPSGUI_UNABLETOOPENFILEFORINPUT ),  // "Unable to open file for input!\n"
            GetString( MSG_GLOBAL_OK ),                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)DC_Win->InFile,
            GetString( MSG_DATAOPSGUI_UNABLETOREADFILESIZE ),  // "Unable to read file size!\n"
            GetString( MSG_GLOBAL_OK ),                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERT ) ,            // "Data Ops: Convert"
            GetString( MSG_DATAOPSGUI_WARNINGILEISNOTAWCSDEMFILE ),  // "Warning!\nFile is not a WCS DEM file."
            GetString( MSG_GLOBAL_OK ),                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERT ),                   // "Data Ops: Convert"
            GetString( MSG_DATAOPSGUI_WARNINGILEISNOTANIFFZBUFFERFILE ),  // "Warning!\nFile is not an IFF Z Buffer file."
            GetString( MSG_GLOBAL_OK ),                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),             // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_WARNINGILEISNOTAVISTADEMFILE ),  // "Warning\nFile is not a Vista DEM file."
            GetString( MSG_GLOBAL_OK ),                            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),  // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_WARNINGILEISNOTACOMPRESSEDVISTAFILEANDCANNOTBEIM ),  // "Warning\nFile is not a compressed Vista file and cannot be imported."
            GetString( MSG_GLOBAL_OK ),                                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                 // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_ISTHISASMALLLARGEORHUGEVISTAFILE ),  // "Is this a Small, Large or Huge Vista file?"
            GetString( MSG_DATAOPSGUI_SMALLLARGEHUGE ),                    // "Small|Large|Huge"
            (CONST_STRPTR)"slh");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),         // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_WRNFILENOTIFF ),  // "Warning\nFile is not an IFF file."
            GetString( MSG_GLOBAL_OK ),                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_WRNFILENOTIFFIMAGFILE ),  // "Warning\nFile is not an IFF image file."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,        // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_ERRORREADINGBITMAPHEADER ),  // "Error reading bitmap header."
            GetString( MSG_GLOBAL_OK ),                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                     // "Data Ops: Convert DEM"
            GetString( MSG_DATAOPSGUI_WARNINGILEISNOTRECOGNIZEDASADTEDFILE ),  // "Warning\nFile is not recognized as a DTED file."
            GetString( MSG_GLOBAL_OK ),                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DATAOPSGUI_DEMINTERPOLATE ),  // "DEM Interpolate"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");


    //./DefaultParams.c
    //find . -name "DefaultParams.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                  // "Parameters Module: Defaults"
            GetString( MSG_DEFPARM_PLEASEENABLEATLEASTONETOPODEMANDTRYAGAIN ),  // "Please enable at least one topo DEM and try again."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                   // "Parameters Module: Defaults"
            GetString( MSG_DEFPARM_PLEASECLOSEALLTIMELINESWINDOWSANDTRYAGAIN ),  // "Please close all Time Lines windows and try again."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),  // "Parameters Module: Defaults"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),               // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                        // "OK"
            (CONST_STRPTR)"o");


    //./DiagnosticGUI.c
    // find . -name "DiagnosticGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DIAG_RENDERDATA ),   // "Render Data",
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK",
            (CONST_STRPTR)"o");

    //./DispatchGUI.c
    //find . -name "DispatchGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)str,
            GetString( MSG_DISPGUI_MAKETHISTHEDEFAULTOBJECTDIRECTORY ),  // "Make this the default object directory?"
            GetString( MSG_GLOBAL_OKCANCEL ),                           // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                                // "Database: Load"
            GetString( MSG_DISPGUI_ERROROPENINGDATABASEFILEPERATIONTERMINATED ),  // "Error opening Database file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                           // "Database: Load"
            GetString( MSG_DISPGUI_NOTAWCSDATABASEFILEPERATIONTERMINATED ),  // "Not a WCS Database file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                                // "Database: Load"
            GetString( MSG_DISPGUI_ERRORREADINGDATABASEFILEPERATIONTERMINATED ),  // "Error reading Database file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_DISPGUI_DATABASEMODULELOAD ),                               // "Database Module: Load"
            GetString( MSG_DISPGUI_OUTOFMEMORYALLOCATINGDATABASEPERATIONTERMINATED ),  // "Out of memory allocating Database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o");

    //./EdDBaseGUI.c
    // find . -name "EdDBaseGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDDB_DATABASEEDITOR ),                                         // "Database Editor"
            GetString( MSG_EDDB_YOUMUSTFIRSTLOADORCREATEADATABASEBEFOREOPENINGTHEEDITO ), // "You must first load or create a database before opening the editor."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                    // "Database Module"
            GetString( MSG_EDDB_OUTOFMEMORYANTOPENDATABASEWINDOW ),  // "Out of memory!\nCan't open database window.",
            GetString( MSG_GLOBAL_OK ),                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDDB_DATABASEEDITOR ),  // "Database Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_DB_DATABASEMODULENAME ),                             // "Database Module: Name"
            GetString( MSG_EDDB_OBJECTNAMEALREADYPRESENTINDATABASERYANEWNAME ) ,  // "Object name already present in database!\nTry a new name?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDDB_DATABASEMODULEREMOVEITEM ),                                          // "Database Module: Remove Item
            GetString( MSG_EDDB_DELETEOBJECTELEVATIONANDRELATIVEELEVATIONFILESFROMDISKASWELL ),      // "Delete object, elevation and relative elevation files from disk as well as remove their names from the Database?"
            GetString( MSG_EDDB_FROMDISKDATABASEONLYCANCEL ),                                        // "From Disk|Database Only|Cancel",
            (CONST_STRPTR)"fdc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                  // "Database Module"
            GetString( MSG_EDDB_OUTOFMEMCANTOPENDBLIST ),          // "Out of memory!\nCan't open database list."
            GetString( MSG_GLOBAL_OK ),                            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                         // "Database Module"
            GetString( MSG_EDDB_OUTOFMEMORYANTOPENDIRECTORYLISTWINDOW ),  // "Out of memory!\nCan't open directory list window."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                         // "Database Module"
            GetString( MSG_EDDB_OUTOFMEMORYANTOPENDIRECTORYLISTWINDOW ),  // "Out of memory!\nCan't open directory list window."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDDB_DIRECTORYLIST ),  // "Directory List"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
            GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    //./EdEcoGUI.c
    //find . -name "EdEcoGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),                                     // "Ecosystem Editor"
            GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor.",
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),          // "Parameters Module: Ecosystem"
            GetString( MSG_EDECOGUI_OUTOFMEMORYANTOPENECOSYSTEMEDITOR ),  // "Out of memory!\nCan't open Ecosystem Editor."
            GetString( MSG_GLOBAL_OK ),                                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),  // "Ecosystem Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),                          // "Parameters Module: Ecosystem"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OKCANCEL ),                                           // "OK|Cancel",
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMPARAMETERSSWAP ) ,                         // "Ecosystem Parameters: Swap"
            GetString( MSG_EDECOGUI_CANTSWAPWITHFIRST12ECOSYSTEMSPERATIONTERMINATED ),  // "Can't swap with first 12 ecosystems!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"oc");

    //./EdMoGUI.c
    //find . -name "EdMoGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),                                         // "Motion Editor"
            GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),  // "Motion Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_MOTIONEDITORAUTOCENTER ),                          // "Motion Editor: Auto Center"
            GetString( MSG_EDMOGUI_INTERACTIVEMODULEMUSTBEOPENBEFOREAUTOCENTERING ),  // "Interactive module must be open before auto centering!"
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMOTION ),                       // "Parameters Module: Motion"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OKCANCEL ),                                     // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMAKEKEY ),  // "Parameters Module: Make Key"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_YESNO ),                    // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),   // "Camera View"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEWASPECT ),                                     // "Camera View: Aspect"
            GetString( MSG_EDMOGUI_COMPUTEDHEIGHTISLARGERTHANTHECURRENTSCREENHEIGHTDOY ),  // "Computed height is larger than the current screen height. Do you wish to use the screen height?"
            GetString( MSG_GLOBAL_OKCANCEL),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_MOTIONPARAMLIST ),  // "Motion Param List"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    //./EdPar.c
    // find . -name "EdPar.c" -exec grep -A3 -nHis "User_Message" {} \;
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Export",
    //	  "No Key Frames to export!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Export",
    //	  "Out of memory!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Export",
    //	  "Error opening file for output!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Export",
    //	  "Error writing to file!\nOperation terminated prematurely.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Export",
    //	  "Sorry!\nOnly Flat coordinates are presently implemented.\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message_Def("LightWave Motion: Import",
    //	  "Key Frames exist for Motion Parameters!\nOverwrite them?",
    //	  "OK|Cancel", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "No Key Frames to import!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "Out of memory!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion I/0",
    //	  "Error opening file for input!\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "Error reading from file!\nOperation terminated prematurely.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "Sorry!\nOnly Flat coordinates are presently implemented.\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "Selected file is not a LightWave Motion file.\nOperation terminated.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import", str, "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("LightWave Motion: Import",
    //	  "Error creating Key Frame!\nOperation terminated.", "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEBANKKEYS ),             // "Parameters Module: Bank Keys"
            GetString( MSG_EDPAR_KEYFRAMESEXISTFORTHEBANKPARAMETEROVERWRITETHEM ),  // "Key Frames exist for the "Bank" Parameter. Overwrite them?"
            GetString( MSG_GLOBAL_OKCANCEL ), (CONST_STRPTR)"oc");                   // "OK|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),              // "Parameters Module: Export"
            GetString( MSG_EDPAR_ERRORCREATINGKEYFRAMEPERATIONTERMINATED ),  // "Error creating Key Frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),                    // "Parameters Module: Export"
            GetString( MSG_EDPAR_NOCAMERAPATHLATLONKEYFRAMESPERATIONTERMINATED ),  // "No Camera Path Lat/Lon Key Frames!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),    // "Parameters Module: Export"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load",
            GetString( MSG_EDPAR_UNSUPPORTEDPARAMETERFILETYPEORVERSIONPERATIONTERMINAT ),  // "Unsupported Parameter file type or version!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
            GetString( MSG_EDPAR_THISISANOLDV1FORMATFILEWOULDYOULIKETORESAVEITINTHENEW ),  // "This is an old V1 format file! Would you like to re-save it in the new format now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
            GetString( MSG_EDPAR_THEPARAMETERFILEFORMATHASBEENCHANGEDSLIGHTLYSINCETHIS ),  // "The Parameter File format has been changed slightly since this file was saved. Would you like to re-save it in the new format now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
            GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     //  "Load all key frames?"
            GetString( MSG_GLOBAL_YESNO),                 // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),  // "Color Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
            GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     // "Load all key frames?"
            GetString( MSG_GLOBAL_YESNO ),                // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),                                                   // "Color Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                                                       // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),                                                    // "Color Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
            GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
            GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
            GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
            GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETEREDITINGMODULE ),                                 // "Parameter Editing Module"
            GetString( MSG_EDPAR_PARTIALFILESMAYNOTBEWRITTENTOOLDFILEVERSIONSDOYOUWISH ),  // "Partial files may not be written to old file versions!\n\Do you wish to save the entire parameter file?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
            GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
            GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_COLOREDITORSAVECURRENT ),                         // "Color Editor: Save Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORSAVECURRENT ),  // "Ecosystem Editor: Save Current"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMETERMODULESAVE ),     // "Parameter Module: Save"
            GetString( MSG_EDPAR_SAVEALLKEYFRAMESASWELL ),  // "Save all key frames as well?"
            GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDPAR_PARAMFILE ),                                              // "paramfile"
            GetString( MSG_EDPAR_ERRORWRITINGTOPARAMETERFILETHEOUTPUTFILEHASBEENMODIFI ),  // "Error writing to Parameter file!\n\The output file has been modified and may no longer be valid. Try resaving to a different device or freeing some disk space and saving again."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    //./EdSetGUI.c
    //find . -name "EdSetGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_EDSETGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENIN ),  // "You must first load or create a parameter file before opening the Render Module."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDSETGUI_RENDERSETTINGSEDITOR ),  // "Render Settings Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                    // "OK"
            (CONST_STRPTR)"o");

    //./EditGui.c
    // find . -name "EditGui.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),                                      // "Color Editor"
            GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),  // "Color Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDITGUI_PARAMETERSMODULECOLOR ),  // "Parameters Module: Color"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OKCANCEL ),               // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDITGUI_COLOREDITORCOPY ) ,  // "Color Editor: Copy"
            GetString( MSG_EDITGUI_COPYKEYFRAMESTOO ),   // "Copy Key Frames too?"
            GetString( MSG_GLOBAL_YESNO ),              // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDITGUI_COLORPARAMETERSSWAP ),                          // "Color Parameters: Swap"
            GetString( MSG_EDITGUI_CANTSWAPWITHFIRST24COLORSPERATIONTERMINATED ),  // "Can't swap with first 24 colors!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                          // "OK"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)PAR_NAME_ECO(1),
            GetString( MSG_EDITGUI_THECURRENTCOLORISBEINGUSEDREMOVEITANYWAY ),  // "The current color is being used. Remove it anyway?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                  // "OK|Cancel"
            (CONST_STRPTR)"oc", 0);

    //./EvenMoreGUI.c
    // find . -name "EvenMoreGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EVMORGUI_SUNTIMEWINDOW ) ,  // "Sun Time Window"
            GetString( MSG_EVMORGUI_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_PROJECTNEWEDIT ),  // "Project: New/Edit"
            GetString( MSG_EVMORGUI_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),          // "OK"
            (CONST_STRPTR)"o", 0);

    //./FoliageGUI.c
    // find . -name "FoliageGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_FOLIGUI_PARAMETERSMODULEFOLIAGE ),         // "Parameters Module: Foliage"
            GetString( MSG_FOLIGUI_OUTOFMEMORYANTOPENFOLIAGEEDITOR ), // "Out of memory!\nCan't open Foliage Editor."
            GetString( MSG_GLOBAL_OK ),                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_FOLIGUI_PARAMETERSMODULEFOLIAGE ),          // "Parameters Module: Foliage"
            GetString( MSG_FOLIGUI_OUTOFMEMORYANTOPENFOLIAGEEDITOR ),  // b"Out of memory!\nCan't open Foliage Editor."
            GetString( MSG_GLOBAL_OK ),                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
            GetString( MSG_AGUI_KEEPCHANGES ),    // "Keep changes?"
            GetString( MSG_GLOBAL_YESNO ),          // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
            GetString( MSG_FOLIGUI_UNABLETOLOADIMAGEFILEFORVIEWINGPERATIONTERMINATED ),   // "Unable to load image file for viewing!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORLOADECOTYPE ),                   // "Foliage Editor: Load Ecotype"
            GetString( MSG_FOLIGUI_ERRORLOADINGECOTYPEFILEPERATIONTERMINATED ),  // "Error loading Ecotype file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                            // "Foliage Editor: Add Group"
            GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                           // "Foliage Editor: Add Group"
            GetString( MSG_FOLIGUI_ERRORLOADINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error loading Foliage Group file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
            GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
            GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                                              // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEGROUP ),                         // "Foliage Editor: Save Group"
            GetString( MSG_FOLIGUI_ERRORSAVINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error saving Foliage Group file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                             // "Foliage Editor: Add Image"
            GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ) ,  // "Out of memory allocating new group!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                    // "Foliage Editor: Add Image"
            GetString( MSG_FOLIGUI_ERRORLOADINGIMAGEFILEPERATIONTERMINATED ),  // "Error loading image file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
            GetString( MSG_FOLIGUI_THEIMAGELOADEDPROPERLYMAYBESOMEDAYTHEREWILLEVENBEAW ),  // "The image loaded properly. Maybe some day there will even be a way for you to see it!\n"
            GetString( MSG_FOLIGUI_THATWOULDBENICE ), (CONST_STRPTR)"t", 0);               // "That would be nice"


    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEECOTYPE ),                  // "Foliage Editor: Save Ecotype"
            GetString( MSG_FOLIGUI_ERRORSAVINGECOTYPEFILEPERATIONTERMINATED ),  // "Error saving Ecotype file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o", 0);

    //./GUI.c
    // find . -name "GUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    // all not used
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Parameters Module", "Out of memory!", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("Parameter Editing: Defaults", str, "OK|Cancel", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Parameter Editing: Defaults",
    //	  "You must first load a Database before Default Parameters can be computed.", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Database Module", "Out of memory!", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("DataOps Module", "Out of memory!", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("World Construction Set",
    //	   "Public Screen still has visitors. Try closing again?",
    //	   "Close|Warn|Cancel", "owc", 2);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("World Construction Set",
    //	   "Quit Program\nAre you sure?",
    //	   "Close|Warn|Cancel", "owc", 2);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("WCS Project",
    //	    "Project paths have been modified. Save them before closing?",
    //	    "OK|Cancel", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("Parameter Module",
    //	    "Parameters have been modified. Save them before closing?",
    //	    "OK|Cancel", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("Database Module",
    //	    "Database has been modified. Save it before closing?",
    //	    "OK|Cancel", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message("Example-mod", "Not yet implemented.\nStay Tuned!", "OK","o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message_Def("Example-Win", "Keep changes?", "Keep|Cancel", "kc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Example-mod", loadmesg, "OK","o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message_Def("Example-existsfile", "File already exists.\nDo you wish to overwrite it?",
    //	    "OK|CANCEL", "oc", 1);
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Log Status Module", "Can't Open Log Status Window!",
    //	    "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Log Window", "Out of memory!", "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("WCS: Screen Mode",
    //	    "In order to reset the screen mode WCS will have to close and re-open.\
    //	     Any work in progress should be saved before invoking this command.\n\
    //	     Do you wish to proceed now?", "OK|Cancel", "oc");
    //
    //.GenericParams.c
    // find . -name "GenericParams.c" -exec grep -A3 -nHis "User_Message" {} \;
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Key Frame: Cancel", str, "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARAMS_KEYFRAMEMODULE ),                                      // "Key Frame Module"
            GetString( MSG_PARAMS_OUTOFMEMORYALLOCATINGNEWKEYFRAMEPERATIONTERMINATED ),  // "Out of memory allocating new key frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    //.GenericTLGUI.c
    // find . -name "GenericTLGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                           // "Time Lines"
            GetString( MSG_GENTLGUI_OKGARYYOUKNOWYOUCANTHAVEMORETHANTENVALUESPERTIMELI ),  // "OK, Gary! You know you can't have more than ten values per Time Line. Maybe now you will concede the value of dynamic allocation."
            GetString( MSG_GENTLGUI_SUREANYTHINGYOUSAY ),                                  // "Sure, anything you say!"
            (CONST_STRPTR)"s");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                              // "Time Lines"
            GetString( MSG_GENTLGUI_YOUVEREACHEDTHELIMITOFOPENTIMELINEWINDOWSPLEASECLO ),  // "You've reached the limit of open Time Line windows. Please close one and try again."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_GENTLGUI_TIMELINE ),     // "Time Line"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                              // "Time Lines"
            GetString( MSG_GENTLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");


    //.GlobeMap.c
    // find . -name "GlobeMap.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                   // "Render Module"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                // "Render Module"
            GetString( MSG_GLMP_ERROROPENINGRENDERWINDOWPERATIONTERMINATED ),  // "Error opening render window!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                 // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGZBUFFERPERATIONTERMINATED ),  // "Out of memory opening Z buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                  // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGBITMAPSPERATIONTERMINATED ),   // "Out of memory opening bitmaps!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory opening anti-alias buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   //"OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGREFLECTIONBUFFERCONTINUEWITHOUTRE ),  // "Out of memory allocating Reflection buffer!\n\Continue without Reflections?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module
            GetString( MSG_GLMP_DIAGNOSTICBUFFERSCANTBEGENERATEDFORMULTIPLESEGMENTORMU ),  // "Diagnostic buffers can't be generated for multiple segment or multiple frame renderings! Proceed rendering without them?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGDIAGNOSTICBUFFERSPROCEEDRENDERINGWIT ),  // "Out of memory opening Diagnostic buffers! Proceed rendering without them?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ) ,                                       // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),   // "Out of memory opening key frame table!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                          // "Render Module"
            GetString( MSG_GLMP_ERRORLOADINGWAVEFILECONSTSTRPTRCONTINUEWITHOUTWAVES ),   // "Error loading Wave File!\n\Continue without Waves?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                        // "Continue|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                  // "Render Module: Clouds"
            GetString( MSG_GLMP_ERRORLOADINGCLOUDMAPFILEONTINUEWITHOUTCLOUDSHADOWS ),  // "Error loading Cloud Map file!\nContinue without cloud shadows?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                      // "Render Module: Clouds"
            GetString( MSG_GLMP_OUTOFMEMORYCREATINGCLOUDMAPONTINUEWITHOUTCLOUDSHADOWS ),   // "Out of memory creating Cloud Map!\nContinue without cloud shadows?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_ERRORLOADINGMASTERCOLORMAPSEESTATUSLOGFORMOREINFORMATI ),  // "Error loading Master Color Map! See Status Log for more information.\n\Continue rendering without Color Map?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_ERRORLOADINGSTRATADEFORMATIONMAPCONTINUERENDERINGWITHO ),  // "Error loading Strata Deformation Map!\n\Continue rendering without Deformation Map?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYCREATINGNOISEMAPCONTINUERENDERINGWITHOUTTEX ),  // "Out of memory creating Noise Map!\n\Continue rendering without Texture Noise?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
            GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
            GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
            GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
            GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)linefile,
            GetString( MSG_GLMP_CANTOPENVECTORFILEFOROUTPUTONTINUERENDERINGWITHOUTVECT ),  // "Can't open vector file for output!\nContinue rendering without vectors?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                               // "Render Module"
            GetString( MSG_GLMP_ERRORINTERLACINGFIELDSPERATIONTERMINATED ) ,  // "Error interlacing fields!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULESAVE ),                           // "Render Module: Save"
            GetString( MSG_GLMP_ERRORSAVINGBITMAPPEDIMAGETRYANOTHERDEVICE ),  // "Error saving bitmapped image! Try another device?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                   // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),              // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYSAVINGZBUFFER ),  // "Out of memory saving Z Buffer!\n"
            GetString( MSG_INTVIEW_RETRYCANCEL ),               // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULESAVE ),                    // "Render Module: Save"
            GetString( MSG_GLMP_ERRORSAVINGZBUFFERTRYANOTHERDEVICE ),  // "Error saving Z Buffer! Try another device?"
            GetString( MSG_GLOBAL_OKCANCEL ),                            // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Render Module: Statistics",
    //	    "Save statistical data to file?", "YES|NO", "yn");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Render Module: Statistics",
    //	    "Can't open statistics file! Try another?", "OK|CANCEL", "oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                 // "Render Module: Topo"
            (CONST_STRPTR)str,
            GetString( MSG_INTVIEW_RETRYCANCEL ) ,                     // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
            GetString( MSG_INTVIEW_RETRYCANCEL ),                               // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGFRACTALMAPARRAYCONTINUEWITHOUTFRA ),  // "Out of memory allocating Fractal Map array!\n\Continue without Fractal Maps or retry?"
            GetString( MSG_GLMP_CONTINUERETRYCANCEL ),                                     // "Continue|Retry|Cancel"
            (CONST_STRPTR)"orc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory allocating antialias buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                       // "Render Module: Clouds"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGPOLYGONEDGEBUFFERS ),  // "Out of memory allocating polygon edge buffers!",
            GetString( MSG_INTVIEW_RETRYCANCEL ),                              // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
            GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
            GetString( MSG_INTVIEW_RETRYCANCEL ),                  // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
            GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
            GetString( MSG_INTVIEW_RETRYCANCEL ),                  // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
            GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
            GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                     // "Render Module: Clouds"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGCLOUDKEYFRAMESPERATIONTERMINATED ),  // "Out of memory allocating Cloud Key Frames!\nOperation terminated"
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
            GetString( MSG_GLMP_ERRORREADINGPAGEDOUTFILECANTRESTOREREFLECTIONBUFFERSOP),  // original in GlobeMap.c:1974 (CONST_STRPTR)ErrStr,
            GetString( MSG_GLOBAL_OK ),            // "OK"
            (CONST_STRPTR)"o");

    //.GlobeMapSupport.c
    // find . -name "GlobeMapSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
    //  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //User_Message("Parameters Module",
    //   "Warning!\nCamera and focus at same latitude\
    //   and longitude coordinates (possibly a result of \"Look Ahead\" Render Setting\
    //   enabled).\n\
    //   View may be different than expected.", "OK|Cancel", "oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                            // "Render Module"
            GetString( MSG_GLMPSPRT_ERRORLOADINGSUNIMAGEPERATIONTERMINATED ),  // "Error loading Sun Image!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                             // "Render Module"
            GetString( MSG_GLMPSPRT_ERRORLOADINGMOONIMAGEPERATIONTERMINATED ),  // "Error loading Moon Image!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");


    //.HelpGUI.c
    // find . -name "HelpGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    // all not used
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(str, (CONST_STRPTR)"No help available at this time.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message(str, (char *)GadMesgStr[i], "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message(str, (char *)GadMesgIntStr[i], "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message(str, (char *)GadMesgFloatStr[i], "OK", "o");
    //
    //	  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //  User_Message(str, (char *)GadMesgCycle[i], "OK", "o");


    //.InteractiveDraw.c
    //find . -name "InteractiveDraw.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTDRW_INTERACTIVEMOTIONMODULE ),                  // "Interactive Motion Module"
            GetString( MSG_INTDRW_OUTOFMEMORYIDDENLINEREMOVALNOTAVAILABLE ),  // "Out of memory!\nHidden line removal not available."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTDRW_PARAMETERSMODULEPATH ),                               // "Parameters Module: Path"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening key frame table!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                 // "OK"
            (CONST_STRPTR)"o");

    //.InteractiveView.c
    // find . -name "InteractiveView.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),              // "Parameters Module: Camera View"
            GetString( MSG_INTVIEW_YOUMUSTFIRSTLOADACOMPLETEPARAMETERFILE ),  // "You must first load a complete Parameter file!"
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),                         // "Parameters Module: Camera View"
            GetString( MSG_INTVIEW_THEREARENOOBJECTSINTHISDATABASEPERATIONTERMINATED ),  // "There are no objects in this Database!\nOperation terminated"
            GetString( MSG_GLOBAL_OK ),                                                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTVIEW_EDITINGMODULEINTERACTIVE ),                  // "Editing Module: Interactive"
            GetString( MSG_INTVIEW_CAMERAVIEWFAILEDTOOPENPERATIONTERMINATED ),  // "Camera View failed to open!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),                      // "Parameters Module: Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMORYOPENINGCAMERAVIEWPERATIONTERMINATED ),  // "Out of memory opening Camera View!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),             // "Parameters Module: Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMORYLOADINGDEMSNCREASEGRIDSIZE ),  // "Out of memory loading DEMs!\nIncrease grid size?"
            GetString( MSG_GLOBAL_OKCANCEL ),                               // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),            // "Parameters Module: Camera View"
            GetString( MSG_INTVIEW_NODEMOBJECTSACTIVEPERATIONTERMINATED ),  // "No DEM objects active!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                   // "Camera View"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGZBUFFERPERATIONTERMINATED ),  // "Out of memory opening Z buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMORYOPENINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory opening Antialias buffer!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_EDMOGUI_CAMERAVIEW ),                     // "Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMORYALLOCATINGDEMARRAY ),  // "Out of memory allocating DEM array!\n"
            GetString( MSG_INTVIEW_RETRYCANCEL ),                    // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI ),  // "Out of memory allocating Polygon Smoothing array!\nContinue without Polygon Smoothing?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                             // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                // "Camera View"
            GetString( MSG_INTVIEW_GRIDMUSTBEPRESENTPLEASEREDRAWANDTRYAGAIN ),  // "Grid must be present, please redraw and try again."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                          // "Camera View"
            GetString( MSG_INTVIEW_ERROROPENINGSMALLRENDERINGWINDOWPERATIONTERMINATED ),  // "Error opening Small Rendering Window!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)str,
            GetString( MSG_INTVIEW_ERROROPENINGDEMFILEFORINPUTPERATIONTERMINATED ),  // "Error opening DEM file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)str,
            GetString( MSG_INTVIEW_OUTOFMEMORYTRYASMALLERPREVIEWSIZEPERATIONTERMINATED ),  // "Out of memory! Try a smaller preview size.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
            GetString( MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI ),  // "Out of memory allocating Polygon Smoothing array!\nContinue without Polygon Smoothing?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                             // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_INTVIEW_PARAMETERSMODULEPREVIEW ),                      // "Parameters Module: Preview"
            GetString( MSG_INTVIEW_RESTORETHEPARAMETERSUSEDTOCREATETHISPREVIEW ),  // "Restore the Parameters used to create this preview?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                     // GetString( MSG_INTVIEW_OKCANCEL )"OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    //.LWSupport.c
    //find . -name "LWSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                  // "LightWave Motion: Export"
            GetString( MSG_LWSPRT_NOKEYFRAMESTOEXPORTPERATIONTERMINATED ),  // "No Key Frames to export!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),          // "LightWave Motion: Export"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                        // "LightWave Motion: Export"
            GetString( MSG_LWSPRT_ERROROPENINGFILEFOROUTPUTPERATIONTERMINATED ),  // "Error opening file for output!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                            // "LightWave Motion: Export"
            GetString( MSG_LWSPRT_ERRORWRITINGTOFILEPERATIONTERMINATEDPREMATURELY ),  // "Error writing to file!\nOperation terminated prematurely."
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"Example DEMName",
            GetString( MSG_LWSPRT_ERRORLOADINGDEMOBJECTPERATIONTERMINATED ),  // "Error loading DEM Object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"Example DEMName",
            GetString( MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED ),  // "Error loading DEM Object!\nObject not saved."
            GetString( MSG_GLOBAL_OK ),                                  // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_LWSPRT_LWOBJECTEXPORT ),                 // "LW Object Export"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
            GetString( MSG_LWSPRT_APROBLEMOCCURREDSAVINGTHELWSCENEFAFILEWASCREATEDITWILLNOTBE ),  // "A problem occurred saving the LW scene.\nIf a file was created it will not be complete and may not load properly into LightWave."
            GetString( MSG_GLOBAL_OK ),                                                           // "OK"
            (CONST_STRPTR)"o", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
            GetString( MSG_LWSPRT_THEOUTPUTIMAGESIZEISNOTASTANDARDLIGHTWAVEIMAGESIZETHEZOOMFA ),  // "The output image size is not a standard LightWave image size. The zoom factor and image dimensions may not be portrayed correctly in the scene file just created."
            GetString( MSG_GLOBAL_OK ),                                                           // "OK"
            (CONST_STRPTR)"o", 0);

    //.LineSupport.c
    // find . -name "LineSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),  // "Digitize new points for the active vector object or create a new object?"
            GetString( MSG_GLOBAL_ACTIVENEWCANCEL ),                                     // "Active|New|Cancel"
            (CONST_STRPTR)"anc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_DIAGNOSTICDIGITIZE ),                                  // "Diagnostic: Digitize"
            GetString( MSG_MAP_ACTIVEOBJECTISADEMANDMAYNOTBEDIGITIZEDPERATIO ),  // "Active object is a DEM and may not be digitized!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_INTERACTIVEMODULEADDPOINTS ),    // "Interactive Module: Add Points"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ), // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ) ,                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_LINESPRT_SAVEOBJECTPOINTS ),  // "Save object points?"
            GetString( MSG_GLOBAL_OKCANCEL ),          // "OK|CANCEL"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                     // "Mapping Module: Path"
            GetString( MSG_MAP_ERRORLOADINGVECTOROBJECTPERATIONTERMINATED),  // "Error loading vector object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Path",
    //		  "Motion Time Line window must be closed for this operation. Please close and try again.",
    //		  "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_CAMERAKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVAL ),  // "Camera Key Frames exist. Proceeding will delete current values!"
            GetString( MSG_MOREGUI_PROCEEDCANCEL ),                                       // "Proceed|Cancel"
            (CONST_STRPTR)"pc");

    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Path",
    //		  "Motion Time Line window must be closed for this operation. Please close and try again.",
    //		  "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_FOCUSKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVALU ),  // "Focus Key Frames exist. Proceeding will delete current values!"
            GetString( MSG_MOREGUI_PROCEEDCANCEL ) ,                                      // "Proceed|Cancel"
            (CONST_STRPTR)"pc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)str, GetString( MSG_LINESPRT_USEELEVATIONDATA ),  // "Use elevation data?"
            GetString( MSG_GLOBAL_YESNO ),                                // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current flattening, datum and vertical exaggeration?"
            GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                               // "Mapping Module: Path"
            GetString( MSG_LINESPRT_OUTOFMEMORYCREATINGKEYFRAMESPERATIONTERMINATED ),  // "Out of memory creating Key Frames!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_USEALLSPLINEDPOINTSORONLYKEYFRAMES ),  // "Use all splined points or only Key Frames?"
            GetString( MSG_LINESPRT_ALLSPLINEDKEYFRAMES ),                 // "All Splined|Key Frames"
            (CONST_STRPTR)"ak");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current Flattening, Datum and Vertical Exaggeration?"
            GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                  // "Mapping Module: Path"
            GetString( MSG_LINESPRT_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening Key Frame table!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_THEREAREMOREFRAMESTHANALLOWABLEVECTORPOINTSPATHWIL ),  // "There are more frames than allowable vector points! Path will be truncated."
            GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module"
            GetString( MSG_LINESPRT_VECTORNAMEALREADYPRESENTINDATABASEVERWRITEITORTRYA ),  // "Vector name already present in Database!\nOverwrite it or try a new name?"
            GetString( MSG_LINESPRT_OVERWRITENEWCANCEL ),                                  // "Overwrite|New|Cancel"
            (CONST_STRPTR)"onc", 2);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
            GetString( MSG_LINESPRT_OUTOFMEMORYEXPANDINGDATABASEPERATIONTERMINATED ),  // "Out of memory expanding Database!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
            GetString( MSG_LINESPRT_OUTOFMEMCREATNEWVECTOROBJECTPERATIONTERMINAT ),  // "Out of memory creating new vector object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    //.Map.c
    // find . -name "Map.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
            GetString( MSG_MAP_FIRSTSETOFALIGNMENTLATLONCOORDINATESMUSTBELAR ),  // "First set of alignment lat/lon coordinates must be larger than second and map scale must be greater than zero!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
            GetString( MSG_MAP_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSETO ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPVIEWECOSYSTEMS ),                              // "Map View: Ecosystems"
            GetString( MSG_MAP_THEREARENOPARAMETERSLOADEDECOSYSTEMMAPPINGISN ),  // "There are no Parameters loaded! Ecosystem mapping is not available until you load a Parameter file or create Default Parameters."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAP_MAPVIEWTOPODRAW ),                                // "Map View: Topo Draw"
            GetString( MSG_MAP_MEMORYALLOCATIONFAILURECANNOTDRAWTOPOCONTINUE ),  // "Memory allocation failure, cannot draw topo. Continue?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPVIEWECOSYSTEMS ),                              // "Map View: Ecosystems"
            GetString( MSG_MAP_OUTOFMEMORYLOADINGRELATIVEELEVATIONFILEECOSYS ),  // "Out of memory loading Relative Elevation file. Ecosystem mapping not available?"
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"DBase[i].Name",
            GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),  // "Is this the correct object?"
            GetString( MSG_GLOBAL_YESNO ),                   // "YES|NO"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"DBase[i].Name", GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),   // "Is this the correct object?"
            GetString( MSG_GLOBAL_YESNO ),                                                 // "YES|NO"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),   // "Mapping Module"
            GetString( MSG_MAP_OBJECTNOTFOUND ),  // "Object not found!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAP_MAPVIEWMULTISELECT ),     // "Map View: Multi-Select"
            GetString( MSG_MAP_SELECTORDESELECTITEMS ),  // "Select or de-select items?"
            GetString( MSG_MAP_SELECTDESELECTCANCEL ),   // "Select|De-select|Cancel"
            (CONST_STRPTR)"sdc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
            GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),   // "Digitize new points for the active vector object or create a new object?"
            GetString( MSG_GLOBAL_ACTIVENEWCANCEL ), (CONST_STRPTR)"anc", 1);     // "Active|New|Cancel"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPVIEWDIGITIZE ),                                // "Map View: Digitize"
            GetString( MSG_MAP_ACTIVEOBJECTISADEMANDMAYNOTBEDIGITIZEDPERATIO ),  // "Active object is a DEM and may not be digitized!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),   // "Mapping Module: Digitize"
            GetString( MSG_MAP_ACCEPTNEWPOINTS ),         // "Accept new points?"
            GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                           // "Mapping Module: Digitize"
            GetString( MSG_MAP_OUTOFMEMORYALLOCATINGNEWVECTORARRAYPERATIONTE ),   // "Out of memory allocating new vector array!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
            GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEINSERTPOINTS ),   // "Mapping Module: Insert Points"
            GetString( MSG_MAP_OUTOFMEMORYOPERATIONFAILED ),  // "Out of memory! Operation failed."
            GetString( MSG_GLOBAL_OK ),                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
            GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_MAP_CREATEVISUALSENSITIVITYMAPFORTHISOBJECT ),  // "Create Visual Sensitivity map for this object?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                 // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_DBASEOBNNAME ),                                // "DBase[OBN].Name"
            GetString( MSG_MAP_ERRORLOADINGVECTOROBJECTPERATIONTERMINATED ),  // "Error loading vector object!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                       // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                 // "Mapping Module"
            GetString( MSG_MAP_ERROROPENINGVIEWSHEDWINDOWXECUTIONTERMINATED ),  // "Error opening viewshed window!\nExecution terminated."
            GetString( MSG_GLOBAL_OK ),                                            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),          // "Mapping Module: Viewshed"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),          // "Mapping Module: Viewshed"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),                   // "Mapping Module: Viewshed"
            GetString( MSG_MAP_ERRORREADINGTOPOMAPSPERATIONTERMINATED ),  // "Error reading topo maps!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),                // "Mapping Module: Viewshed"
            GetString( MSG_MAP_SMOOTHTHEMAPBEFORECOMPUTINGVIEWSHED ),  // "Smooth the map before computing viewshed?"
            GetString( MSG_GLOBAL_OKCANCEL ),                             // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),           // "Mapping Module: Viewshed"
            GetString( MSG_MAP_DRAWVECTORSONVIEWSHEDRENDERING ),  // "Draw vectors on viewshed rendering?"
            GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
            (CONST_STRPTR)"yn");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
            GetString( MSG_MAP_CANTOPENSERIALDEVICEPERATIONTERMINATED ),  // "Can't open serial device!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
            GetString( MSG_MAP_DIGITIZENEWREGISTRATIONPOINTS ),  // "Digitize new registration points?"
            GetString( MSG_GLOBAL_YESNO ),                          // "YES|NO"
            (CONST_STRPTR)"yn");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                          // "Mapping Module: Digitize"
            GetString( MSG_MAP_ILLEGALVALUEWOREGISTRATIONPOINTSMAYNOTBECOINC ),  // "Illegal value!\nTwo registration points may not be coincident.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                             // "OK
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    //.MapExtra.c
    //find . -name "MapExtra.c" -exec grep -A3 -nHis "User_Message" {} \;
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(statname, "Can't open file!", "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(statname, "File not a WCS statistics file!\nSelect another?",
    //		     "OK|CANCEL", "oc");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(statname, "File is of incorrect size!\nOperation terminated.",
    //		  	"OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(statname, "Out of memory!\n Operation terminated.", "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Statistics",
    //		  	"No statistical data has been loaded to normalize!", "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Statistics",
    //		  		"Load new normalizing data?", "YES|NO", "yn");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Statistics",
    //		     "Out of memory!\n Operation failed.", "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Statistics",
    //  		     "Out of memory!\nOperation failed.", "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Mapping Module: Statistics",
    //		     "No statistical data has been loaded to graph!", "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_MAPEXTRA_OBJECTISNOTCLOSEDHEORIGINCANNOTBEMOVEDETLASTVERTEX ),  // "Object is not closed!\nThe origin cannot be moved.\nSet last vertex equal to first now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
            GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_MAPEXTRA_OBJECTRESULTINGFROMTHISMATCHWOULDBELARGERTHANTHEMA ),  // "Object resulting from this match would be larger than the maximum of MAXOBJPTS !\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),                             // "Mapping Module: Point Match"
            GetString( MSG_MAPEXTRA_ILLEGALNUMBEROFPOINTSFFIRSTANDLASTDESTINATIONPOINT ),  // "Illegal number of points!\nIf first and last destination points are the same, source points must be larger than zero.\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),  // "Mapping Module: Point Match"
            GetString( MSG_MAPEXTRA_PROCEEDWITHRELOCATION ),    // "Proceed with relocation?"
            GetString( MSG_GLOBAL_OKCANCEL ),                 // "OK|CANCEL"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),                        // "Mapping Module: Point Match"
            GetString( MSG_MAPEXTRA_OUTOFMEMORYOTENOUGHFORNEWPOINTSPERATIONFAILED ),  // "Out of memory!\nNot enough for new points.\nOperation failed."
            GetString( MSG_GLOBAL_OK ),                                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
            GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
            GetString( MSG_GLOBAL_OKCANCEL ) ,                               // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
            GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
            GetString( MSG_MAPEXTRA_DUPLICATETHISOBJECT ),  // "Duplicate this object?"
            GetString( MSG_GLOBAL_OKCANCEL ),             // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                            // "Mapping Module: Follow Stream"
            GetString( MSG_MAPEXTRA_OUTOFMEMNOTENOUGHFORTEMPTOPOARRAYPERATIONFAIL ), // "Out of memory!\nNot enough for temporary topo array.\nOperation failed."
            GetString( MSG_GLOBAL_OK ),                                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                   // "Mapping Module: Follow Stream"
            GetString( MSG_MAPEXTRA_POINTMAXIMUMHASBEENREACHEDAPPINGTERMINATED ),  // "Point maximum has been reached!\nMapping terminated"
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ), (CONST_STRPTR)str,  // "Mapping Module: Follow Stream"
            GetString( MSG_GLOBAL_OKCANCEL ),  // "OK|CANCEL"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                           // "Mapping Module: Follow Stream"
            GetString( MSG_MAPEXTRA_INITIALPOINTNOTWITHINCURRENTLYLOADEDTOPOBOUNDARIES ),  // "Initial point not within currently loaded topo boundaries!\nObject points reduced to 1."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                           // "Mapping Module: Follow Stream"
            GetString( MSG_MAPEXTRA_INITIALPOINTNOTWITHINCURRENTLYLOADEDTOPOBOUNDARIES ),  // "Initial point not within currently loaded topo boundaries!\nObject points reduced to 1."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),  // "Mapping Module: Follow Stream"
            GetString( MSG_MAPEXTRA_SAVEVECTOROBJECTNOW ),        // "Save vector object now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULESPLINE ),                       // "Mapping Module: Spline"
            (CONST_STRPTR)str,
            GetString( MSG_MAPEXTRA_OKRESETCANCEL ),                             // "OK|Reset|Cancel"
            (CONST_STRPTR)"orc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize",
            GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save object now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPVIEWMODULEINTERPOLATE ),                            // "Map View Module: Interpolate"
            GetString( MSG_MAPEXTRA_OUTOFMEMORYCANTALLOCATENEWVECTORPERATIONTERMINATED ),  // "Out of memory! Can't allocate new vector.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),  // "Mapping Module: Fix Flats"
            GetString( MSG_MAPEXTRA_PROCEEDORRESETPOINTS ),   // "Proceed or reset points?"
            GetString( MSG_MAPEXTRA_PROCEEDRESETCANCEL ),     // "Proceed|Reset|Cancel"
            (CONST_STRPTR)"prc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),           // "Mapping Module: Fix Flats"
            GetString( MSG_MAPEXTRA_KEEPORSAVEDEMORRESETPARAMETERS ),  // "Keep or save DEM or reset parameters?"
            GetString( MSG_MAPEXTRA_KEEPSAVERESETCANCEL ),             // "Keep|Save|Reset|Cancel"
            (CONST_STRPTR)"ksrc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),          // "Mapping Module: Fix Flats"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
            GetString( MSG_MAPEXTRA_ALLCORNERPOINTSMUSTBEWITHINTOPOMAPBOUNDARIESPERATI ),  // "All corner points must be within topo map boundaries!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                          // "Mapping Module: Fix Flats"
            GetString( MSG_MAPEXTRA_ILLEGALDIMENSIONSTRYMAKINGTHERECTANGLELARGERPERATI ),  // "Illegal dimensions! Try making the rectangle larger.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message( GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
            GetString( MSG_MAPEXTRA_ALLCORNERPNTSMUSTBEWITHINSAMEDEMPERATIONTERMINAT ),  // "All corner points must be within same DEM!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[TopoOBN[i]].Name",
            GetString( MSG_MAPEXTRA_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"DBase[TopoOBN[i]].Name",
            GetString( MSG_MAPEXTRA_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                          // "OK"
            (CONST_STRPTR)"o");

    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message_Def("Particle Tree", "Do another tree?", "Yes|No", "yn", 1);
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message_Def("Particle Tree",
    //		  "Out of memory allocating new branch!\nOperation terminated.",
    //		  "OK", "o", 0);


    //.MapGUI.c
    // find . -name "MapGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                           // "Mapping Module"
            GetString( MSG_MAPGUI_OUTOFMEMORYANTINITIALIZEMAPWINDOWPERATIONTERMINATED ),  // "Out of memory!\nCan't initialize map window!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");                               // "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),  // "Mapping Module: Align"
            GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal."
            GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");  // "OK"

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),     // "Mapping Module: Digitize"
            GetString( MSG_MAPGUI_SETDIGITIZINGINPUTSOURCE ),  // "Set digitizing input source."
            GetString( MSG_MAPGUI_BITPADSUMMAGRIDMOUSE ),      // "Bitpad|Summagrid|Mouse"
            (CONST_STRPTR)"bsm");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                                      // "Map View: Build DEM"
            (CONST_STRPTR) GetString( MSG_MAPGUI_ATLEASTONEENDCONTROLPOINTFORTHELINESEGMENTJUSTDRAWNC ),  // "At least one end control point for the line segment just drawn could not be found!\nDo you wish to use the current and minimum slider elevations for this segment or abort the operation?",
            GetString( MSG_MAPGUI_SLIDERABORT ),                                                          // "Slider|Abort"
            (CONST_STRPTR)"sa");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                                    // "Mapping Module: Align"
            GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal.
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_DATABASESAVE ),                                          // "Database: Save"
            GetString( MSG_MAPGUI_THEDATABASEHASBEENMODIFIEDSINCEITWASLOADEDOYOUWISHTO ),  // "The Database has been modified since it was loaded.\nDo you wish to save it or a Master Object file now?",
            GetString( MSG_MAPGUI_DBASEOBJECTBOTHNEITHER ),                                // "D'base|Object|Both|Neither"
            (CONST_STRPTR)"dmbn");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
            GetString( MSG_MAPGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENINGT ),  // "You must first load or create a parameter file before opening the Legend."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWECOSYSTEMLEGEND ),              // "Map View: Ecosystem Legend"
            GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "Out of memory!\nCan't open Ecosystem Legend."
            GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		     "Select contour objects to export and reselect Export Contours\
    //		      from the menu  when done.",
    //		     "OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		     "Can't open Database Editor window!\nOperation terminated.","OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		     "Extract elevation values from Object Names, Label fields\
    //		      or use the values embedded in the Objects themselves?",
    //		  	 "Name|Label|Embedded", "nle");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		  "Error writing to file!\nOperation terminated.","OK", "o");
    //
    //          IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //          "Error writing to XYZ header file!","OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		  	"Error opening XYZ header file for output!","OK", "o");
    //
    //		  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Export Contours",
    //		  	"Error opening XYZ file for output!\nOperation terminated.","OK", "o");

    //.MapLineObject.c
    // find . -name "MapLineObject.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                               // "Render Module"
            GetString( MSG_MAPLINO_ERRORSAVINGLINEVERTICESTOFILEELECTNEWPATH ),  // "Error saving line vertices to file!\nSelect new path."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                          // "Render Module"
            GetString( MSG_MAPLINO_ERROROPENINGLINESAVEFILEELECTNEWPATH ),  // "Error opening line save file!\nSelect new path?"
            GetString( MSG_GLOBAL_OKCANCEL ),                              // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    //.MapSupport.c
    // find . -name "MapSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)DBase[OBN].Name, GetString( MSG_MAPSUPRT_CANTOPENOBJECTFILEBJECTNOTSAVED ),  // "Can't open object file!\nObject not saved."
            GetString( MSG_GLOBAL_OK ),                                                              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)DBase[OBN].Name, GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),  // "Error saving object file!\nObject not saved."
            GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),                                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPSUPRT_MAPPINGMODULETOPOMAPPING ),                            // "Mapping Module: Topo Mapping"
            GetString( MSG_MAPSUPRT_NOTOPOMAPSFOUNDHECKOBJECTENABLEDSTATUSANDCLASSINDA ),  // "No topo maps found!\nCheck object Enabled Status and Class in database."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),               // "Map View: Load Topos"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)str, GetString( MSG_MAPSUPRT_ERRORLOADINGTOPOMAPCHECKSTATUSLOGTOSEEIFOUTOFMEMOR ),  // "Error loading topo map! Check Status Log to see if out of memory.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),            // "Map View: Load Topos"
            GetString( MSG_MAPSUPRT_ERRORLOADINGDEMSNONELOADED ),  // "Error loading DEMs! None loaded."
            GetString( MSG_GLOBAL_OK ),                          // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                       // "Mapping Module"
            GetString( MSG_MAP_OUTOFMEMORYALLOCATINGNEWVECTORARRAYPERATIONTE ),  // "Out of memory allocating new vector array!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def((CONST_STRPTR)"DBase[i].Name",
            GetString( MSG_MAPSUPRT_VECTOROBJECTHASBEENMODIFIEDAVEITBEFORECLOSING ),  // "Vector object has been modified!\nSave it before closing?"
            GetString( MSG_MAPSUPRT_SAVECANCEL ),                                     // "SAVE|CANCEL"
            (CONST_STRPTR)"sc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
            GetString( MSG_MAPSUPRT_SELECTEDOBJECTMUSTBEATOPODEMEECLASSFIELDINDATABASE ),  // "Selected object must be a Topo DEM!\nSee Class field in Database Editor.\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
            GetString( MSG_MAPSUPRT_SELECTEDMAPISNOTCURRENTLYLOADEDOYOUWISHTOLOADTOPOM ),  // "Selected map is not currently loaded!\nDo you wish to load topo maps?",
            GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|CANCEL"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                               // "Map View: Color Map"
            GetString( MSG_MAPSUPRT_OUTOFMEMORYCREATINGBITMAPSPERATIONTERMINATED ),  // "Out of memory creating bitmaps!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                    // "Map View: Color Map"
            GetString( MSG_MAPSUPRT_INCLUDEDEMELEVATIONDATAINCOLORMAP ),  // "Include DEM elevation data in Color Map?"
            GetString( MSG_GLOBAL_YESNO ),                              // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    //.MapTopoObject.c
    // find . -name "MapTopoObject.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
            GetString( MSG_INTVIEW_RETRYCANCEL ),                               // "Retry|Cancel"
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                       // "Render Module"
            GetString( MSG_MAPTOPOOB_ERRORALLOCATINGORREADINGFRACTALINDEXARRAYSCONTINU ),  // "Error allocating or reading Fractal Index arrays!\nContinue without Fractal Displacement Mapping?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                           // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                     // "Render Module"
            GetString( MSG_MAPTOPOOB_ERRORSAVINGVECTORVERTICESTOFILE ),  // "Error saving vector vertices to file!"
            GetString( MSG_GLOBAL_OK ),                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                 // "Render Module"
            GetString( MSG_MAPTOPOOB_CANTOPENVECTORFILEFOROUTPUT ),  // "Can't open vector file for output!"
            GetString( MSG_GLOBAL_OK ),                           // "OK"
            (CONST_STRPTR)"o");

    //			  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Render Module: Clouds",
    //			  "Out of memory initializing Cloud Map!\nContinue without clouds or cancel rendering?",
    //			  "Continue|Cancel", "oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                        // "Render Module: Topo"
            (CONST_STRPTR)str,
            GetString( MSG_INTVIEW_RETRYCANCEL ),                             //"Retry|Cancel",
            (CONST_STRPTR)"rc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                       // "Render Module"
            GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening key frame table!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                 // "OK"
            (CONST_STRPTR)"o");

    //.Memory.c
    // find . -name "Memory.c" -exec grep -A3 -nHis "User_Message" {} \;
    //				  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Memory Alloc Fail", str, "OK", "o");


    //.MoreGUI.c
    // find . -name "MoreGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_DEMEXTRACT ),   // "DEM Extract"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_DATAOPSMODULEDEMEXTRACT ),                       // "Data Ops Module: DEM Extract"
            GetString( MSG_MOREGUI_PLEASEENTERTHELATITUDEANDLONGITUDEVALUESFORTHESOUTH ),  // "Please enter the latitude and longitude values for the southeast corner of the current DEM in the string gadgets near the top of the DEM Extract Window."
            GetString( MSG_MOREGUI_PROCEEDCANCEL ), (CONST_STRPTR)"pc");                   // "Proceed|Cancel"


    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_PROJECTNEWEDIT ),  // "Project: New/Edit"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              //"OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
            GetString( MSG_MOREGUI_APPLYCHANGES ),          // "Apply changes?"
            GetString( MSG_GLOBAL_OKCANCEL ),              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MENU_PREFS ),  // "Preferences"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");


    //.Params.c
    // find . -name "Params.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARAMS_KEYFRAMECANCEL ),  // "Key Frame: Cancel"
            (CONST_STRPTR)str,
            GetString( MSG_GLOBAL_OK ),               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARAMS_KEYFRAMEMODULE ) ,                                     // "Key Frame Module"
            GetString( MSG_PARAMS_OUTOFMEMORYALLOCATINGNEWKEYFRAMEPERATIONTERMINATED ),  // "Out of memory allocating new key frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARAMS_PARAMETERSMODULEVELOCITYDISTRIBUTION ),                 // "Parameters Module: Velocity Distribution"
            GetString( MSG_PARAMS_EASEINPLUSEASEOUTFRAMEVALUESEXCEEDTOTALNUMBEROFANIMA ),  // "\"Ease In\" plus \"Ease Out\" frame values exceed total number of animated frames.\nThis is illegal! Do you wish to continue without Velocity Distribution?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    //.ParamsGUI.c
    // find . -name "ParamsGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
            GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
            GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),  // "Parameters Module: Scale"
            GetString( MSG_PARGUI_NOKEYFRAMESTOSCALE ),     // "No key frames to scale!"
            GetString( MSG_GLOBAL_OK ),                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
            GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_SCALEKEYS ),    // "Scale Keys"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_LIGHTWAVEMOTIONIO ),                                     // "LightWave Motion I/O"
            GetString( MSG_PARGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREUSINGTHI ),  // "You must first load or create a parameter file before using this feature."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_LIGHTWAVEMOTIONIO ),                                // "LightWave Motion I/O"
            GetString( MSG_PARGUI_ERRORBUILDINGMOTIONVALUETABLEPERATIONTERMINATED ),  // "Error building motion value table\nOperation terminated",
            GetString( MSG_GLOBAL_OK ),                                               // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_LIGHTWAVEIO ),  // "LightWave I/O"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                // "Parameters Module: Model"
            GetString( MSG_PARGUI_OUTOFMEMORYANTOPENMODELDESIGNWINDOW ),  // "Out of memory!\nCan't open model design window."
            GetString( MSG_GLOBAL_OK ),                                   // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),  // "Parameters Module: Model"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),            // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_THECURRENTECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_1 ),  // "The current Ecosystem Model has been modified. Do you wish to save it before closing?"
            GetString( MSG_PARGUI_YESNOCANCEL ),                                           // "Yes|No|Cancel"
            (CONST_STRPTR)"ync", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2 ),  //" The current Ecosystem Model has been modified. Do you wish to save it before proceeding?"
            GetString( MSG_PARGUI_YESNOCANCEL ),                                           // "Yes|No|Cancel"
            (CONST_STRPTR)"ync", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_ERROROPENINGECOSYSTEMMODELFILEFORINPUTPERATIONTERMI ),   // "Error opening Ecosystem Model file for input!\nOperation terminated." // AF: fixed, was "output"
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
            GetString( MSG_PARGUI_ERRORREADINGFROMECOSYSTEMMODELFILEPERATIONTERMINATEDPR ),  // "Error reading from Ecosystem Model file!\nOperation terminated prematurely.", // AF: fixed, was "writing to"
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
            GetString( MSG_PARGUI_NOTAWCSECOSYSTEMMODELFILEPERATIONTERMINATED ),  // "Not a WCS Ecosystem Model file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_UNSUPPORTEDWCSECOSYSTEMMODELFILEVERSIONPERATIONTERMI ),  // "Unsupported WCS Ecosystem Model file version!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),  // "Parameters Module: Model"
            GetString( MSG_PARGUI_YOUHAVENOTSELECTEDAFILENAMEFORINPUTPERATIONTERMINATE ),  // "You have not selected a file name for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_ERROPENCOSYSMODELFILEFOROUTPUTPERATIONTERMI ),  // "Error opening Ecosystem Model file for output!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                  // "Parameters Module: Model"
            GetString( MSG_PARGUI_ERRORWRITINGTOECOSYSTEMMODELFILEPERATIONTERMINATEDPR ),   // "Error writing to Ecosystem Model file!\nOperation terminated prematurely.",
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
            GetString( MSG_PARGUI_NOTSELECTEDAFILENAMEFOROUTPUTPERATIONTERMINAT ),  // "You have not selected a file name for output!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),  // "Parameters Module: Anim"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),                                 // "Parameters Module: Anim"
            GetString( MSG_PARGUI_SPECIFIEDWIDTHISLARGERTHANTHECURRENTSCREENWIDTHDOYOU ),  // "Specified width is larger than the current screen width. Do you wish to use the screen width?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),                                  // "Parameters Module: Anim"
            GetString( MSG_PARGUI_SPECIFIEDORCOMPUTEDHEIGHTISLARGERTHANTHECURRENTSCREE ),  // "Specified or computed height is larger than the current screen height. Do you wish to use the screen height?
            GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc");

    //.Support.c
    // find . -name "Support.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_WCSCONFIGURATIONSAVE ),                         // "WCS Configuration: Save"
            GetString( MSG_SUPPORT_CANTOPENCONFIGURATIONFILEPERATIONTERMINATED ),  // "Can't open configuration file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_WCSCONFIGURATIONLOAD ),                         // "WCS Configuration: Load"
            GetString( MSG_SUPPORT_CANTOPENCONFIGURATIONFILEPERATIONTERMINATED ),  // "Can't open configuration file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_WCSPROJECTSAVE ),                         // "WCS Project: Save"
            GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_SUPPORT_PROJECTSAVE ),                          // "Project: Save"
            GetString( MSG_SUPPORT_SAVEDATABASEANDPARAMETERFILESASWELL ),  // "Save Database and Parameter files as well?"
            GetString( MSG_SUPPORT_BOTHDBASEPARAMSNO ),                    // "Both|D'base|Params|No",
            (CONST_STRPTR)"bdpn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_WCSPROJECTLOAD ),                         // "WCS Project: Load"
            GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_PROJECTLOAD ),                           // "Project: Load"
            GetString( MSG_SUPPORT_NOTAWCSPROJECTFILEPERATIONTERMINATED ),  // "Not a WCS Project file!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                    // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                                        // "Mapping Module: Align"
            GetString( MSG_SUPPORT_ILLEGALMAPREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUAL ),  // "Illegal map registration values! High and low X or Y values are equal."
            GetString( MSG_GLOBAL_OK ),                                                        // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_DIRECTORYLISTLOAD ),                      // "Directory List: Load"
            GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_SUPPORT_DIRECTORYLISTLOAD ),                     // "Directory List: Load"
            GetString( MSG_SUPPORT_NOTAWCSPROJECTFILEPERATIONTERMINATED ),  // "Not a WCS Project file!\nOperation terminated.",
            GetString( MSG_GLOBAL_OK ),                                    // "OK"
            (CONST_STRPTR)"o");

    //.TimeLinesGUI.c
    // find . -name "TimeLinesGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
            GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
            GetString( MSG_GLOBAL_OK ),                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
            GetString( MSG_TLGUI_NOMOTIONPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERM ),  // "No Motion Parameters with more than one Key Frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_MOTIONTIMELINE ),  // "Motion Time Line"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),              // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_MOTIONEDITORTIMELINES ),                                  // "Motion Editor: Time Lines"
            GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
            GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
            GetString( MSG_GLOBAL_OK ),                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
            GetString( MSG_TLGUI_NOCOLORPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERMI ),  // "No Color Parameters with more than one Key Frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_COLORTIMELINE ),  // "Color Time Line"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),             // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_COLOREDITORTIMELINES ),                                   // "Color Editor: Time Lines"
            GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
            GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
            GetString( MSG_GLOBAL_OK ),                                // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
            GetString( MSG_TLGUI_NOECOSYSTEMPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONT ),  // "No Ecosystem Parameters with more than one Key Frame!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_ECOSYSTEMTIMELINE ),  // "Ecosystem Time Line"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),        // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),                 // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_TLGUI_ECOSYSTEMEDITORTIMELINES ),                               // "Ecosystem Editor: Time Lines"
            GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
            GetString( MSG_GLOBAL_OK ),                                                     // "OK"
            (CONST_STRPTR)"o");

    //.Tree.c
    // find . -name "Tree.c" -exec grep -A3 -nHis "User_Message" {} \;
    //						  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Tree Convert", "Error opening file for input!\nOperation terminated.", "OK", "c");
    //
    //						  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Tree Convert", "Out of memory!\nOperation terminated.", "OK", "c");
    //
    //						  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Tree Convert", "Error reading file!\nOperation terminated.", "OK", "c");
    //
    //
    //						  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Tree Convert", "Error opening file for output!\nOperation terminated.", "OK", "c");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
            GetString( MSG_TREE_ERROROPENINGECOSYSTEMMODELFILEFORINPUTPERATIONTERMINAT ),  // "Error opening Ecosystem Model file for input!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK",
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
            GetString( MSG_TREE_ERRORWRITINGTOECOSYSTEMMODELFILEPERATIONTERMINATEDPREM ),  // "Error writing to Ecosystem Model file!\nOperation terminated prematurely."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
            GetString( MSG_PARGUI_NOTAWCSECOSYSTEMMODELFILEPERATIONTERMINATED ),  // "Not a WCS Ecosystem Model file!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
            GetString( MSG_PARGUI_UNSUPPORTEDWCSECOSYSTEMMODELFILEVERSIONPERATIONTERMI ),  // "Unsupported WCS Ecosystem Model file version!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
            GetString( MSG_TREE_OUTOFMEMORYALLOCATINGECOSYSTEMMODELSPERATIONTERMINATED ),  // "Out of memory allocating Ecosystem Models!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                      // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
            GetString( MSG_TREE_NODATAINWCSECOSYSTEMMODELPERATIONTERMINATED ),  // "No data in WCS Ecosystem Model!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                           // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)PAR_NAME_ECO(0), GetString( MSG_TREE_APROBLEMOCCURREDLOADINGATLEASTONEIMAGEFORTHISECOSYSTEM ),  // "A problem occurred loading at least one image for this ecosystem!\nContinue without it or them?"
            GetString( MSG_GLOBAL_OKCANCEL ),   // "OK|Cancel"
            (CONST_STRPTR)"oc");

    //						  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message(PAR_NAME_ECO(0), "No images found for this ecosystem!\n\
    ./Tree.c-1347-Continue without them?", "OK|Cancel", "oc");

    //.WCS.c  // ALEXANDER
    // find . -name "WCS.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message((CONST_STRPTR)"World Construction set",
            GetString( MSG_WCS_BETAPERIODEXPIRED ),     // "Beta period expired..."
            GetString( MSG_GLOBAL_OK ),                    // "OK"
            (CONST_STRPTR)"o");

    //.Wave.c
    // find . -name "Wave.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),          // "Wave: Set Defaults"
            GetString( MSG_WAV_SELECTGENERALWAVECENTER ),  // "Select general wave center."
            GetString( MSG_WAV_FOCUSPOINTCAMERAPOINT ),    // "Focus Point|Camera Point"
            (CONST_STRPTR)"fc", 0);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Wave: Set Defaults"
            GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Select wave speed."
            GetString( MSG_WAV_FASTVERYFASTSLOW ),  //"Fast|Very Fast|Slow"
            (CONST_STRPTR)"fvs", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),      // "Wave: Set Defaults"
            GetString( MSG_WAV_SELECTWAVEDIRECTION ),  // "Select wave direction."
            GetString( MSG_WAV_SPREADINGCONVERGING ),  // "Spreading|Converging"
            (CONST_STRPTR)"sc", 1);;


    //.WaveGUI.c
    // find . -name "WaveGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_WAVGUI_MAPVIEWWAVES ),  // "Map View: Waves"
            GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
            GetString( MSG_GLOBAL_OK ),            // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),  // "Wave Editor"
            GetString( MSG_WAVGUI_THECURRENTWAVEMODELHASBEENMODIFIEDDOYOUWISHTOSAVEITB ),  // "The current Wave Model has been modified. Do you wish to save it before closing?"
            GetString( MSG_GLOBAL_YESNO ),  // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
            GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
            GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ) ,             // "Wave Editor"
            GetString( MSG_WAVGUI_DELETEALLWAVEKEYFRAMES ),  // "Delete all wave key frames?"
            GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
            (CONST_STRPTR)"oc", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
            GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
            GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
            GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
            GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
            (CONST_STRPTR)"yn", 1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message_Def(GetString( MSG_WAVGUI_ADDWAVE ),                                               // "Add Wave"
            GetString( MSG_WAVGUI_MAPVIEWMODULEMUSTBEOPENINORDEROUSETHISFUNCTIONWOULDY ),  // "Map View Module must be open in order\ to use this function. Would you like to open it now?"
            GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
            (CONST_STRPTR)"oc",1);

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_WAVGUI_MAPVIEWWAVEADD ),                                     // "Map View: Wave Add"
            GetString( MSG_WAVGUI_REMOVEALLCURRENTLYDEFINEDWAVESBEFOREADDINGNEWONES ),  // "Remove all currently defined waves before adding new ones?"
            GetString( MSG_GLOBAL_YESNO ),                                              // "Yes|No"
            (CONST_STRPTR)"yn");


    //.nncrunch.c
    // find . -name "nncrunch.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                     // "Map View: Build DEM"
            GetString( MSG_NNCRUNCH_INSUFFICIENTDATAINGRIDDEDREGIONTOTRIANGULATEINCREA ),  // "Insufficient data in gridded region to triangulate! Increase the size of the gridded region or add more control points."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");

    //							  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Build DEM",
    //							     "Unable to open data file.", "OK", "o");
    //
    //							  IncAndShowTestNumbers(++TestNumber,TotalTests);
    //    User_Message("Map View: Build DEM",
    //							     "Insufficient data in gridded region to triangulate!\
    //							     Increase the size of the gridded region or add more control points", "OK", "o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ),                                      // "Map View: Grid DEM"
            GetString( MSG_NNCRUNCH_THERATIOOFVERTICALTOHORIZONTALMAPDIMENSIONSISTOOLA ),  // "The ratio of vertical to horizontal map dimensions is too large for gradient estimation. Scale the data if gradients are required.\nDo you wish to continue without gradient estimation?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ) ,                                     // "Map View: Grid DEM"
            GetString( MSG_NNCRUNCH_RATIOOFVERTTOHORIZMAPDIMENSIONSISTOOSM ),  // "The ratio of vertical to horizontal map dimensions is too small for gradient estimation. Scale the data if gradients are required.\nDo you wish to continue without gradient estimation?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ),                                      // "Map View: Grid DEM"
            GetString( MSG_NNCRUNCH_THERATIOOFWIDTHTOLENGTHOFTHISGRIDDEDREGIONMAYBETOO ),  // "The ratio of width to length of this gridded region may be too extreme for good interpolation.\nChanging the block proportions, or rescaling the x or y coordinate may be a good idea.\nContinue now with the present dimensions?"
            GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
            (CONST_STRPTR)"oc");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                            // "Map View: Build DEM",
            GetString( MSG_NNCRUNCH_OUTOFMEMORYDOUBLEMATRIXPERATIONTERMINATED ),  // "Out of memory Double Matrix!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                         // "OK"
            (CONST_STRPTR)"o");

    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                     // "Map View: Build DEM"
            GetString( MSG_NNCRUNCH_OUTOFMEMORYALLOCATINGDOUBLEMATRIXPERATIONTERMINATE ),  // "Out of memory allocating Double Matrix!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                                                  // "OK"
            (CONST_STRPTR)"o");


    //.nngridr.c
    // find . -name "nngridr.c" -exec grep -A3 -nHis "User_Message" {} \;
    IncAndShowTestNumbers(++TestNumber,TotalTests);
    User_Message(GetString( MSG_NNGRIDR_MAPVIEWGRIDDEM ),                // "Map View: Grid DEM"
            GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
            GetString( MSG_GLOBAL_OK ),                             // "OK"
            (CONST_STRPTR)"o");

    printf("Last User_Message: %d\n", TestNumber);



}
