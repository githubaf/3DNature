/* Support.c (ne gissupport.c 14 Jan 1994 CXH)
** Intuition stuff, setup, etc for renderer.
** Built/ripped from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and many more happy changes by Gary R. Huber.
*/

#include "WCS.h"
#include "GUIDefines.h"
#include <stdarg.h>

extern char ExtVersion[];


#ifdef AMIGA_GUI

static void stripintuimessages(struct MsgPort *mp, struct Window *win); // used locally only -> static, AF 19.7.2021
static short checkuserabort(void); // used locally only -> static, AF 26.7.2021

/************************************************************************/
  
struct Window *make_window(short x, short y, short w, short h, char name[80],
         ULONG flags, ULONG iflags, UBYTE color0, UBYTE color1,
         struct Screen *screen)
{
 struct NewWindow NewWindow;
 NewWindow.LeftEdge=x;
 NewWindow.TopEdge=y;
 NewWindow.Width=w;
 NewWindow.Height=h;
 NewWindow.DetailPen=color1;
 NewWindow.BlockPen=color0;
 NewWindow.IDCMPFlags=iflags;
 NewWindow.Flags=flags | WFLG_SIZEBBOTTOM;
 NewWindow.FirstGadget=NULL;
 NewWindow.CheckMark=NULL;
 NewWindow.Title=name;
 NewWindow.Screen=(struct Screen *)screen;
 NewWindow.BitMap=NULL;
 NewWindow.MinWidth=20;
 NewWindow.MinHeight=20;
 NewWindow.MaxWidth=640;
 NewWindow.MaxHeight=200;
 if (screen==NULL) NewWindow.Type=WBENCHSCREEN;
 else NewWindow.Type=CUSTOMSCREEN;

 return(OpenWindow(&NewWindow)); 

} /* make_window() */

#endif /* AMIGA_GUI */

/*************************************************************************/

void FetchEvent(struct Window *Win, struct IntuiMessage *Event)
{
struct IntuiMessage *Brief = NULL;

while(Brief == NULL)
	{
	Brief = (struct IntuiMessage *)GetMsg(Win->UserPort);
	if(Brief == NULL)
		{
		Wait(1 << Win->UserPort->mp_SigBit);
		} /* if */
	} /* while */

memcpy(Event, Brief, sizeof(struct IntuiMessage));

ReplyMsg((struct Message *)Brief);

} /* FetchEvent() */

/*************************************************************************/

struct Window *FetchMultiWindowEvent(struct IntuiMessage *Event, ...)
{
va_list VarA;
struct IntuiMessage *Brief = NULL;
ULONG Signals = NULL;
struct Window *ThisWin = NULL;

va_start(VarA, Event);

while (Brief == NULL)
 {
 ThisWin = va_arg(VarA, struct Window *);
 if (ThisWin == NULL)
  break;
 Signals += (1 << ThisWin->UserPort->mp_SigBit);
 Brief = (struct IntuiMessage *)GetMsg(ThisWin->UserPort);
 } /* while */

va_end(VarA);

while(Brief == NULL)
	{
	va_start(VarA, Event);
	while (Brief == NULL)
		{
		ThisWin = va_arg(VarA, struct Window *);
		if (ThisWin == NULL)
			break;
		Brief = (struct IntuiMessage *)GetMsg(ThisWin->UserPort);
		}
	va_end(VarA);
	if(Brief == NULL)
		{
		Wait(Signals);
		} /* if */
	} /* while */

memcpy(Event, Brief, sizeof(struct IntuiMessage));

ReplyMsg((struct Message *)Brief);

return (ThisWin);

} /* FetchMultiWindowEvent() */

/***********************************************************************/

ULONG QuickFetchEvent(struct Window *Win, struct IntuiMessage *Event)
{
struct IntuiMessage *Brief;

  Brief = (struct IntuiMessage *)GetMsg(Win->UserPort);

  if (Brief)
   {
   memcpy(Event, Brief, sizeof(struct IntuiMessage));
   ReplyMsg((struct Message *)Brief);
   } /* if */

/* The return value used only as an indicator of a message present!!
** Do not try to access the IntuiMessage through it since the Message itself
** was destroyed by ReplyMsg.
*/
  return ((ULONG)Brief);

} /* QuickFetchEvent() */

/* Quickly check to see if any messages are pending. See defines.h for the
** macro which replaced this. */

/* ULONG QuickCheckEvent(struct Window *Win)
{
  return(Win->UserPort->mp_MsgList->lh_Head);
} */ /* QuickCheckEvent() */


/***********************************************************************/

void closesharedwindow(struct Window *win, short shared)
{
 Forbid();
 stripintuimessages(win->UserPort, win);
 if (shared) win->UserPort = NULL;
 ModifyIDCMP(win, NULL);
 Permit();
 CloseWindow(win);
}

/***********************************************************************/

static void stripintuimessages(struct MsgPort *mp, struct Window *win) // used locally only -> static, AF 19.7.2021
{
 struct IntuiMessage *msg;
 struct Node *succ;

 msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

 while ((succ = msg->ExecMessage.mn_Node.ln_Succ)) {
  if (msg->IDCMPWindow == win) {
   Remove((struct Node *)msg);
   ReplyMsg((struct Message *)msg);
  }
  msg = (struct IntuiMessage *)succ;
 } /* while succ = msg... */
}

#ifdef CLI_BREAK
static short checkuserabort(void) // used locally only -> static, AF 26.7.2021
{
 short abort;

 printf("CTRL_C BREAK: Abort rendering? (1= Yes, 0= No): ");
 scanf("%hd", &abort);
 printf("\n");
 return (abort);
} /* checkuserabort() */
#endif /* CLI_BREAK */

/***********************************************************************/

void SaveConfig(void)
{
 FILE *fconfig;

 if (chdir("ENVARC:WCS")) Mkdir("ENVARC:WCS");
 chdir(path);

 if (! (fconfig = fopen("ENVARC:WCS/WCS.config", "w")))
  {
  User_Message("WCS Configuration: Save",
	"Can't open configuration file!\nOperation terminated.", "OK", "o");
  return;
  } /* if, presumably, no ENVARC: */

 if (DB_Win)
  {
  if (DB_Win->Layout) 	fprintf(fconfig, "%d\n", CONFIG_DB_HORWIN);
     fprintf(fconfig, "%d\n", CONFIG_DB);
  }
 if (DO_Win)
  {
     if (DO_Win->Layout)
     {
         fprintf(fconfig, "%d\n", CONFIG_DO_HORWIN);
     }
     fprintf(fconfig, "%d\n", CONFIG_DO);
  }
 if (EP_Win)
  {
     if (EP_Win->Layout)
     {
         fprintf(fconfig, "%d\n", CONFIG_EP_HORWIN);
     }
     fprintf(fconfig, "%d\n", CONFIG_EP);
  }
 if (DE_Win) 		fprintf(fconfig, "%d\n", CONFIG_DE);
 if (DC_Win) 		fprintf(fconfig, "%d\n", CONFIG_DC);
 if (DL_Win) 		fprintf(fconfig, "%d\n", CONFIG_DL);
 if (DM_Win) 		fprintf(fconfig, "%d\n", CONFIG_DM);
 if (EE_Win) 		fprintf(fconfig, "%d\n", CONFIG_EE);
 if (EC_Win) 		fprintf(fconfig, "%d\n", CONFIG_EC);
 if (EM_Win) 		fprintf(fconfig, "%d\n", CONFIG_EM);
 if (ES_Win) 		fprintf(fconfig, "%d\n", CONFIG_ES);
 if (SC_Win) 		fprintf(fconfig, "%d\n", CONFIG_SC);
 if (EMIA_Win)
  {
     fprintf(fconfig, "%d  %hd  %hd  %hd  %hd\n",
             CONFIG_EMIA_SIZE,
             InterWind0->TopEdge, InterWind0->LeftEdge,
             InterWind0->Width,   InterWind0->Height);
     if (InterWind2)
     {
        fprintf(fconfig, "%d  %hd  %hd  %hd  %hd\n",
                CONFIG_EMIA_COMPSIZE,
                InterWind2->TopEdge, InterWind2->LeftEdge,
                InterWind2->Width,   InterWind2->Height);
     }
     fprintf(fconfig, "%d\n", CONFIG_EMIA);
  }
 if (EETL_Win) 		fprintf(fconfig, "%d\n", CONFIG_EETL);
 if (ECTL_Win) 		fprintf(fconfig, "%d\n", CONFIG_ECTL);
 if (EMTL_Win) 		fprintf(fconfig, "%d\n", CONFIG_EMTL);
 if (EMPL_Win) 		fprintf(fconfig, "%d\n", CONFIG_EMPL);
 if (FM_Win) 		fprintf(fconfig, "%d\n", CONFIG_FM);
 if (LW_Win) 		fprintf(fconfig, "%d\n", CONFIG_LW);
 if (AN_Win) 		fprintf(fconfig, "%d\n", CONFIG_AN);
 if (MapWind0 && MP)
  {
		 	fprintf(fconfig, "%d  %hd  %hd  %hd  %hd\n",
		 		CONFIG_MAP_SIZE, 
				MapWind0->TopEdge, MapWind0->LeftEdge, 
				MapWind0->Width,   MapWind0->Height);
		 	fprintf(fconfig, "%d\n", CONFIG_MAP);
  if (MP->AlignWin)	fprintf(fconfig, "%d\n", CONFIG_MA);
  }
 if (EL_Win) 		fprintf(fconfig, "%d\n", CONFIG_EL);
 if (PJ_Win) 		fprintf(fconfig, "%d\n", CONFIG_PJ);
 if (PR_Win) 		fprintf(fconfig, "%d\n", CONFIG_PR);
 if (DI_Win) 		fprintf(fconfig, "%d\n", CONFIG_DI);
 if (TS_Win) 		fprintf(fconfig, "%d\n", CONFIG_TS);

 fclose(fconfig);

} /* SaveConfig() */

/***********************************************************************/

void LoadConfig(void)
{
 short DB_Hor = FALSE, DO_Hor = FALSE, EP_Hor = FALSE/*, RN_Hor = FALSE*/;
 FILE *fconfig;
 LONG item;

 if (! (fconfig = fopen("ENVARC:WCS/WCS.config", "r")))
  {
  User_Message("WCS Configuration: Load",
	"Can't open configuration file!\nOperation terminated.", "OK", "o");
  return;
  } /* if no configuration file */

 if (! dbaseloaded) Database_LoadDisp(0, 1, NULL, 0);
 if (! paramsloaded)
  {
  if (loadparams(0x1111, -1) == 1)
   {
   paramsloaded = 1;
   FixPar(0, 0x1111);
   FixPar(1, 0x1111);
   } /* if */
  } /* if */

 for (; ;)
  {
  if (fscanf(fconfig, "%ld", &item) == EOF) break;

  switch (item)
   {
   case CONFIG_DB_HORWIN:
    {
    DB_Hor = TRUE;
    break;
    } /* database window orientation */
   case CONFIG_DB:
    {
    Make_DB_Window(DB_Hor);
    break;
    } /* database module */
   case CONFIG_DO_HORWIN:
    {
    DO_Hor = TRUE;
    break;
    } /* data operations window orientation */
   case CONFIG_DO:
    {
    Make_DO_Window(DO_Hor);
    break;
    } /* data operations module */
   case CONFIG_EP_HORWIN:
    {
    EP_Hor = TRUE;
    break;
    } /* editing window orientation */
   case CONFIG_EP:
    {
    Make_EP_Window(EP_Hor);
    break;
    } /* editing module */
   case CONFIG_DE:
    {
    Make_DE_Window();
    break;
    } /* database editor */
   case CONFIG_EE:
    {
    Make_EE_Window();
    break;
    } /* ecosystem editor */
   case CONFIG_EC:
    {
    Make_EC_Window();
    break;
    } /* color editor */
   case CONFIG_EM:
    {
    Make_EM_Window();
    break;
    } /* motion editor */
   case CONFIG_ES:
    {
    Make_ES_Window();
    break;
    } /* settings editor */
   case CONFIG_EMIA_SIZE:
    {
    fscanf(fconfig, "%hd", &IA_Top);
    fscanf(fconfig, "%hd", &IA_Left);
    fscanf(fconfig, "%hd", &IA_Width);
    fscanf(fconfig, "%hd", &IA_Height);
    break;
    } /* interactive view size */
   case CONFIG_EMIA_COMPSIZE:
    {
    fscanf(fconfig, "%hd", &IA_CompTop);
    fscanf(fconfig, "%hd", &IA_CompLeft);
    fscanf(fconfig, "%hd", &IA_CompWidth);
    fscanf(fconfig, "%hd", &IA_CompHeight);
    break;
    } /* interactive view size */
   case CONFIG_EMIA:
    {
    if (EM_Win)
     {
     if (interactiveview(1))
      Make_EMIA_Window();
     } /* if motion editor open */
    break;
    } /* interactive view */
   case CONFIG_EETL:
    {
    if (EE_Win)
     Make_EETL_Window();
    break;
    } /* ecosystem time lines */
   case CONFIG_ECTL:
    {
    if (EC_Win)
     Make_ECTL_Window();
    break;
    } /* color time lines */
   case CONFIG_EMTL:
    {
    if (EM_Win)
     Make_EMTL_Window();
    break;
    } /* motion time lines */
   case CONFIG_FM:
    {
    Make_FM_Window();
    break;
    } /* ecosystem model */
   case CONFIG_LW:
    {
    Make_LW_Window();
    break;
    } /* LightWave motion */
   case CONFIG_AN:
    {
    Make_AN_Window();
    break;
    } /* anim control */
   case CONFIG_MAP_SIZE:
    {
    fscanf(fconfig, "%hd", &MP_Top);
    fscanf(fconfig, "%hd", &MP_Left);
    fscanf(fconfig, "%hd", &MP_Width);
    fscanf(fconfig, "%hd", &MP_Height);
    break;
    } /* map size */
   case CONFIG_MAP:
    {
    struct clipbounds cb;

    map();
    if (MapWind0)
     {
     setclipbounds(MapWind0, &cb);
     makemap(MapWind0, cb.lowx, cb.lowy, cb.highx, cb.highy,
      (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0));
     if(InterStuff)
      {
      ShowView_Map(&cb);
      ShowPaths_Map(&cb);
      } /* if */
     MapGUI_Update(MP); /* In case Exag changed as a result of loading Topos */
     } /* if map window opened */
    break;
    } /* map view */
   case CONFIG_EL:
    {
    if (MapWind0)
     Make_EL_Window();
    break;
    } /* anim control */
   case CONFIG_DL:
    {
    Make_DL_Window();
    break;
    } /* directory list */
   case CONFIG_DC:
    {
    Make_DC_Window();
    break;
    } /* DEM convert */
   case CONFIG_DI:
    {
    Make_DI_Window();
    break;
    } /* DEM interpolate */
   case CONFIG_DM:
    {
    Make_DM_Window();
    break;
    } /* DEM extract */
   case CONFIG_EMPL:
    {
    if (EM_Win)
     Make_EMPL_Window();
    break;
    } /* parameter list */
   case CONFIG_PJ:
    {
    Make_PJ_Window();
    break;
    } /* project */
   case CONFIG_SC:
    {
    if (ES_Win)
     Make_SC_Window();
    break;
    } /* scale image */
   case CONFIG_TS:
    {
    if (EM_Win)
     Make_TS_Window();
    break;
    } /* scale image */
   case CONFIG_PR:
    {
    Make_PR_Window();
    break;
    } /* preferences */
   case CONFIG_MA:
    {
    if (MapWind0 && MP)
     Make_MA_Window(MP);
    break;
    } /* align map */
   } /* switch item */
  } /* for */

 fclose(fconfig);

} /* LoadConfig() */

/***********************************************************************/

#define PROJECT_DB_PTH 		1
#define PROJECT_DB_NME		2
#define PROJECT_PR_PTH		3
#define PROJECT_PR_NME		4
#define PROJECT_FR_PTH		5
#define PROJECT_LN_PTH		6
#define PROJECT_LN_NME		7
#define PROJECT_ZB_PTH		8
#define PROJECT_ZB_NME		9
#define PROJECT_BG_PTH		10
#define PROJECT_BG_NME		11
#define PROJECT_GR_PTH		12
#define PROJECT_GR_NME		13
#define PROJECT_ST_PTH		14
#define PROJECT_ST_NME		15
#define PROJECT_CM_PTH		16
#define PROJECT_DR_PTH		17
#define PROJECT_DL_NEW		18
#define PROJECT_DL_ADD		19
#define PROJECT_TM_PTH		20
#define PROJECT_TM_NME		21
#define PROJECT_FR_NME		22
#define PROJECT_PJ_PTH		23
#define PROJECT_PJ_NME		24
#define PROJECT_FM_PTH		25
#define PROJECT_CM_NME		26
#define PROJECT_CL_PTH		27
#define PROJECT_CL_NME		28
#define PROJECT_WV_PTH		29
#define PROJECT_WV_NME		30
#define PROJECT_DF_PTH		31
#define PROJECT_DF_NME		32
#define PROJECT_IM_PTH		33
#define PROJECT_SN_NME		34
#define PROJECT_MN_NME		35
#define PROJECT_PP_PTH		36
#define PROJECT_PF_PTH		37
#define PROJECT_AO_PTH		38

#define PROJECT_IA_SENSITIV	50
#define PROJECT_IA_GRIDSTYLE	51
#define PROJECT_IA_MOVEMENT	52
#define PROJECT_MP_DITHER	53
#define PROJECT_MP_CONTINT	54
#define PROJECT_MP_CONTINT2	55
#define PROJECT_MP_DIGMODE	56
#define PROJECT_IA_GRIDSIZE	57
#define PROJECT_MP_ECOLEGEND	58
#define PROJECT_IA_GBDENS	59
#define PROJECT_IA_AUTODRAW	60
#define PROJECT_IA_ANIMSTART	61
#define PROJECT_IA_ANIMEND	62
#define PROJECT_IA_COMPASSBDS	63
#define PROJECT_IA_LANDBDS	64
#define PROJECT_IA_BOXBDS	65
#define PROJECT_IA_PROFBDS	66
#define PROJECT_IA_GRIDBDS	67

#define PROJECT_EMIA_SIZE	80
#define PROJECT_EMIA_COMPSIZE	81
#define PROJECT_MAP_SIZE	82

#define PROJECT_SM_MODEID	100
#define PROJECT_SM_WIDTH	101
#define PROJECT_SM_HEIGHT	102
#define PROJECT_SM_OTAG		103
#define PROJECT_SM_OVAL		104
#define PROJECT_SM_AUTOTAG	105
#define PROJECT_SM_AUTOVAL	106

#define MAPPREFS_DBL_MAPSCALE	202
#define MAPPREFS_DBL_MAPLAT	203
#define MAPPREFS_DBL_MAPLON	204
#define MAPPREFS_DBL_LATZERO	205
#define MAPPREFS_DBL_LONZERO	206
#define MAPPREFS_DBL_LATSCALEF	207
#define MAPPREFS_DBL_LONSCALEF	208
#define MAPPREFS_DBL_LATY	209
#define MAPPREFS_DBL_LONX	210
#define MAPPREFS_DBL_YLAT	211
#define MAPPREFS_DBL_XLON	212
#define MAPPREFS_SHT_LOWX	213
#define MAPPREFS_SHT_LOWY	214
#define MAPPREFS_SHT_HIGHX	215
#define MAPPREFS_SHT_HIGHY	216
#define MAPPREFS_DBL_RLAT0	217
#define MAPPREFS_DBL_RLON0	218
#define MAPPREFS_DBL_RLAT1	219
#define MAPPREFS_DBL_RLON1	220
#define MAPPREFS_SHT_TOPO	221
#define MAPPREFS_SHT_ALIGN	222
#define MAPPREFS_SHT_VECENABLE	223
#define MAPPREFS_SHT_INTERSTUFF	224
#define MAPPREFS_SHT_AUTOCLEAR	225
#define MAPPREFS_SHT_ECOENABLE	226

short SaveProject(short NewName, char *SaveName, struct WCSScreenData *ScrnData)
{
 FILE *fproject;
 char filename[256], Ptrn[32];
 struct DirList *DLItem;
 short i;

/* NewName values: 	-1 = old name & no save files,
			 0 = old name & save files if no SaveName,
			 1 = new name & save files if no SaveName,
			 2 = old name & force save files */

 if (SaveName)
  {
  strcpy(filename, SaveName);
  }
 else if ((NewName && NewName != 2) || ! projectpath[0] || ! projectname[0])
  {
  strcpy(Ptrn, "#?.proj");
  if (! getfilenameptrn(1, "Project Path/Name", projectpath, projectname, Ptrn))
   return (0);
  strmfp(filename, projectpath, projectname);
  } /* if new name */
 else
  {
  strmfp(filename, projectpath, projectname);
  }

 if ((fproject = fopen(filename, "w")) == NULL)
  {
  if (! SaveName)
   User_Message("WCS Project: Save",
	"Can't open project file!\nOperation terminated.", "OK", "o");
  return (0);
  } /* if open fail */

 fprintf(fproject, "%s\n", "WCSProject");

 if (pcprojectpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PP_PTH);
  fprintf(fproject, "%s\n", pcprojectpath);
  }
 if (pcframespath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PF_PTH);
  fprintf(fproject, "%s\n", pcframespath);
  }
 if (projectpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PJ_PTH);
  fprintf(fproject, "%s\n", projectpath);
  }
 if (projectname[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PJ_NME);
  fprintf(fproject, "%s\n", projectname);
  }
 if (dbasepath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_DB_PTH);
  fprintf(fproject, "%s\n", dbasepath);
  }
 if (dbasename[0])
  {
  fprintf(fproject, "%d\n", PROJECT_DB_NME);
  fprintf(fproject, "%s\n", dbasename);
  }
 if (parampath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PR_PTH);
  fprintf(fproject, "%s\n", parampath);
  }
 if (paramfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_PR_NME);
  fprintf(fproject, "%s\n", paramfile);
  }
 if (framepath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_FR_PTH);
  fprintf(fproject, "%s\n", framepath);
  }
 if (framefile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_FR_NME);
  fprintf(fproject, "%s\n", framefile);
  }
 if (linepath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_LN_PTH);
  fprintf(fproject, "%s\n", linepath);
  }
 if (linefile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_LN_NME);
  fprintf(fproject, "%s\n", linefile);
  }
 if (zbufferpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_ZB_PTH);
  fprintf(fproject, "%s\n", zbufferpath);
  }
 if (zbufferfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_ZB_NME);
  fprintf(fproject, "%s\n", zbufferfile);
  }
 if (backgroundpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_BG_PTH);
  fprintf(fproject, "%s\n", backgroundpath);
  }
 if (backgroundfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_BG_NME);
  fprintf(fproject, "%s\n", backgroundfile);
  }
 if (graphpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_GR_PTH);
  fprintf(fproject, "%s\n", graphpath);
  }
 if (graphname[0])
  {
  fprintf(fproject, "%d\n", PROJECT_GR_NME);
  fprintf(fproject, "%s\n", graphname);
  }
 if (statpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_ST_PTH);
  fprintf(fproject, "%s\n", statpath);
  }
 if (statname[0])
  {
  fprintf(fproject, "%d\n", PROJECT_ST_NME);
  fprintf(fproject, "%s\n", statname);
  }
 if (colormappath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_CM_PTH);
  fprintf(fproject, "%s\n", colormappath);
  }
 if (colormapfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_CM_NME);
  fprintf(fproject, "%s\n", colormapfile);
  }
 if (dirname[0])
  {
  fprintf(fproject, "%d\n", PROJECT_DR_PTH);
  fprintf(fproject, "%s\n", dirname);
  }
 if (modelpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_FM_PTH);
  fprintf(fproject, "%s\n", modelpath);
  }
 if (cloudpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_CL_PTH);
  fprintf(fproject, "%s\n", cloudpath);
  }
 if (cloudfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_CL_NME);
  fprintf(fproject, "%s\n", cloudfile);
  }
 if (wavepath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_WV_PTH);
  fprintf(fproject, "%s\n", wavepath);
  }
 if (wavefile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_WV_NME);
  fprintf(fproject, "%s\n", wavefile);
  }
 if (deformpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_DF_PTH);
  fprintf(fproject, "%s\n", deformpath);
  }
 if (deformfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_DF_NME);
  fprintf(fproject, "%s\n", deformfile);
  }
 if (temppath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_TM_PTH);
  fprintf(fproject, "%s\n", temppath);
  }
 if (imagepath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_IM_PTH);
  fprintf(fproject, "%s\n", imagepath);
  }
 if (sunfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_SN_NME);
  fprintf(fproject, "%s\n", sunfile);
  }
 if (moonfile[0])
  {
  fprintf(fproject, "%d\n", PROJECT_MN_NME);
  fprintf(fproject, "%s\n", moonfile);
  }
 if (altobjectpath[0])
  {
  fprintf(fproject, "%d\n", PROJECT_AO_PTH);
  fprintf(fproject, "%s\n", altobjectpath);
  }
 if (DL)
  {
  short ReadOnly;

  DLItem = DL;
  fprintf(fproject, "%d\n", PROJECT_DL_NEW);
  fprintf(fproject, "%s\n", DLItem->Name);
  if (DLItem->Read == '*') ReadOnly = 1;
  else ReadOnly = 0;
  fprintf(fproject, "%d\n", ReadOnly);
  while ((DLItem = DLItem->Next) != NULL)
   {
   fprintf(fproject, "%d\n", PROJECT_DL_ADD);
   fprintf(fproject, "%s\n", DLItem->Name);
   if (DLItem->Read == '*') ReadOnly = 1;
   else ReadOnly = 0;
   fprintf(fproject, "%d\n", ReadOnly);
   } /* while */
  }

 fprintf(fproject, "%d\n", PROJECT_IA_SENSITIV);
 fprintf(fproject, "%d\n", IA_Sensitivity);
 fprintf(fproject, "%d\n", PROJECT_IA_GRIDSTYLE);
 fprintf(fproject, "%d\n", IA_GridStyle);
 fprintf(fproject, "%d\n", PROJECT_IA_GRIDSIZE);
 fprintf(fproject, "%d\n", IA_GridSize);
 fprintf(fproject, "%d\n", PROJECT_IA_MOVEMENT);
 fprintf(fproject, "%d\n", IA_Movement);
 fprintf(fproject, "%d\n", PROJECT_IA_GBDENS);
 fprintf(fproject, "%d\n", IA_GBDens);
 fprintf(fproject, "%d\n", PROJECT_IA_AUTODRAW);
 fprintf(fproject, "%d\n", IA_AutoDraw);
 fprintf(fproject, "%d\n", PROJECT_IA_ANIMSTART);
 fprintf(fproject, "%d\n", IA_AnimStart);
 fprintf(fproject, "%d\n", PROJECT_IA_ANIMEND);
 fprintf(fproject, "%d\n", IA_AnimEnd);
 fprintf(fproject, "%d\n", PROJECT_IA_COMPASSBDS);
 fprintf(fproject, "%d\n", CompassBounds);
 fprintf(fproject, "%d\n", PROJECT_IA_LANDBDS);
 fprintf(fproject, "%d\n", LandBounds);
 fprintf(fproject, "%d\n", PROJECT_IA_BOXBDS);
 fprintf(fproject, "%d\n", BoxBounds);
 fprintf(fproject, "%d\n", PROJECT_IA_PROFBDS);
 fprintf(fproject, "%d\n", ProfileBounds);
 fprintf(fproject, "%d\n", PROJECT_IA_GRIDBDS);
 fprintf(fproject, "%d\n", GridBounds);
 fprintf(fproject, "%d\n", PROJECT_MP_DITHER);
 fprintf(fproject, "%d\n", MapDither);
 fprintf(fproject, "%d\n", PROJECT_MP_CONTINT);
 fprintf(fproject, "%d\n", ContInt);
 fprintf(fproject, "%d\n", PROJECT_MP_CONTINT2);
 fprintf(fproject, "%d\n", ContInterval);
 fprintf(fproject, "%d\n", PROJECT_MP_DIGMODE);
 fprintf(fproject, "%d\n", MP_DigMode);
 if (IA_Width > 0 && IA_Height > 0)
  {
  fprintf(fproject, "%d\n", PROJECT_EMIA_SIZE);
  fprintf(fproject, "%d %d %d %d\n", IA_Top, IA_Left, IA_Width, IA_Height);
  }
 if (IA_CompWidth > 0 && IA_CompHeight > 0)
  {
  fprintf(fproject, "%d\n", PROJECT_EMIA_COMPSIZE);
  fprintf(fproject, "%d %d %d %d\n", IA_CompTop, IA_CompLeft, IA_CompWidth, IA_CompHeight);
  }
 if (MP_Width > 0 && MP_Height > 0)
  {
  fprintf(fproject, "%d\n", PROJECT_MAP_SIZE);
  fprintf(fproject, "%d %d %d %d\n", MP_Top, MP_Left, MP_Width, MP_Height);
  }
 fprintf(fproject, "%d\n", PROJECT_MP_ECOLEGEND);
 for (i=0; i<6; i++)
  {
  fprintf(fproject, "%d %d\n", EcoUse[i], EcoLegend[i]);
  } /* for i=0... */

/* Screen mode info - this must be last in the file */
 if (ScrnData)
  {
  fprintf(fproject, "%d\n", PROJECT_SM_MODEID);
  fprintf(fproject, "%lu\n", ScrnData->ModeID);
  fprintf(fproject, "%d\n", PROJECT_SM_WIDTH);
  fprintf(fproject, "%ld\n", ScrnData->Width);
  fprintf(fproject, "%d\n", PROJECT_SM_HEIGHT);
  fprintf(fproject, "%ld\n", ScrnData->Height);
  fprintf(fproject, "%d\n", PROJECT_SM_OTAG);
  fprintf(fproject, "%lu\n", ScrnData->OTag);
  fprintf(fproject, "%d\n", PROJECT_SM_OVAL);
  fprintf(fproject, "%lu\n", ScrnData->OVal);
  fprintf(fproject, "%d\n", PROJECT_SM_AUTOTAG);
  fprintf(fproject, "%lu\n", ScrnData->AutoTag);
  fprintf(fproject, "%d\n", PROJECT_SM_AUTOVAL);
  fprintf(fproject, "%lu\n", ScrnData->AutoVal);
  } /* if ScrnData */

 fclose(fproject);

 if (! SaveName) Log(MSG_PROJ_SAVE, projectname);
 Proj_Mod = 0;

 if (! SaveName && NewName >= 0)
  {
  short SaveParts;

  if (NewName == 2)
   SaveParts = 1;
  else
   SaveParts = User_Message_Def("Project: Save",
	"Save Database and Parameter files as well?", "Both|D'base|Params|No",
	"bdpn", 1);
  if (SaveParts > 0)
   {
   if (dbaseloaded && (SaveParts == 1 || SaveParts == 2))
    savedbase(0);
   if (paramsloaded && (SaveParts == 1 || SaveParts == 3))
    saveparams(0x1111, -1, 1);
   } /* if save other files */
  } /* if not Savename, as in not prefs file */

 return (1);

} /* SaveProject() */

/***********************************************************************/
/* Note: Any items added to the project files must also be added to the
**    LoadDirList function below.
*/
short LoadProject(char *LoadName, struct WCSScreenData *ScrnData, short ForceLoad)
{
 FILE *fproject;
 char filename[256], Ptrn[32];
 long i, item, update = 0;

 if (LoadName)
  {
  strcpy(filename, LoadName);
  } /* if name provided, as in prefs file */
 else
  {
  strcpy(Ptrn, "#?.proj");
  if (! getfilenameptrn(0, "Project", projectpath, projectname, Ptrn))
  return (0);
  strmfp(filename, projectpath, projectname);
  } /* else */

 if ((fproject = fopen(filename, "r")) == NULL)
  {
  if (! LoadName)
   User_Message("WCS Project: Load",
	"Can't open project file!\nOperation terminated.", "OK", "o");
  return (0);
  } /* if open fail */

 fgets(filename, 12, fproject);
 filename[10] = '\0';
 if (strcmp(filename, "WCSProject"))
  {
  fclose(fproject);
  if (! LoadName)
   User_Message("Project: Load", "Not a WCS Project file!\nOperation terminated.",
	"OK", "o");
  return (0);
  } /* if not WCS Project */
 for (; ;)
  {
  if (fscanf(fproject, "%ld", &item) == EOF) break;
  switch (item)
   {
   case PROJECT_PJ_PTH:
    {
    fscanf(fproject, "%s", &projectpath);
    break;
    } /*  */
   case PROJECT_PJ_NME:
    {
    fscanf(fproject, "%s", &projectname);
    break;
    } /*  */
   case PROJECT_DB_PTH:
    {
    fscanf(fproject, "%s", &dbasepath);
    break;
    } /*  */
   case PROJECT_DB_NME:
    {
    fscanf(fproject, "%s", &dbasename);
    if (! LoadName || ForceLoad)
     {
     Database_LoadDisp(0, 0, NULL, 0);
     update = 1;
     } /* if */
    break;
    } /*  */
   case PROJECT_PR_PTH:
    {
    fscanf(fproject, "%s", &parampath);
    break;
    } /*  */
   case PROJECT_PR_NME:
    {
    fscanf(fproject, "%s", &paramfile);
    if (! LoadName || ForceLoad)
     {
     if (loadparams(0x1111, -1) == 1)
      {
      update = paramsloaded = 1;
      FixPar(0, 0x1111);
      FixPar(1, 0x1111);
      } /* if */
     } /* if not prefs file */
    break;
    } /*  */
   case PROJECT_FR_PTH:
    {
    fscanf(fproject, "%s", &framepath);
    break;
    } /*  */
   case PROJECT_FR_NME:
    {
    fscanf(fproject, "%s", &framefile);
    break;
    } /*  */
   case PROJECT_LN_PTH:
    {
    fscanf(fproject, "%s", &linepath);
    break;
    } /*  */
   case PROJECT_LN_NME:
    {
    fscanf(fproject, "%s", &linefile);
    break;
    } /*  */
   case PROJECT_ZB_PTH:
    {
    fscanf(fproject, "%s", &zbufferpath);
    break;
    } /*  */
   case PROJECT_ZB_NME:
    {
    fscanf(fproject, "%s", &zbufferfile);
    break;
    } /*  */
   case PROJECT_BG_PTH:
    {
    fscanf(fproject, "%s", &backgroundpath);
    break;
    } /*  */
   case PROJECT_BG_NME:
    {
    fscanf(fproject, "%s", &backgroundfile);
    break;
    } /*  */
   case PROJECT_GR_PTH:
    {
    fscanf(fproject, "%s", &graphpath);
    break;
    } /*  */
   case PROJECT_GR_NME:
    {
    fscanf(fproject, "%s", &graphname);
    break;
    } /*  */
   case PROJECT_ST_PTH:
    {
    fscanf(fproject, "%s", &statpath);
    break;
    } /*  */
   case PROJECT_ST_NME:
    {
    fscanf(fproject, "%s", &statname);
    break;
    } /*  */
   case PROJECT_CM_PTH:
    {
    fscanf(fproject, "%s", &colormappath);
    break;
    } /*  */
   case PROJECT_CM_NME:
    {
    fscanf(fproject, "%s", &colormapfile);
    break;
    } /*  */
   case PROJECT_DR_PTH:
    {
    fscanf(fproject, "%s", &dirname);
    break;
    } /*  */
   case PROJECT_TM_PTH:
    {
    fscanf(fproject, "%s", &temppath);
    break;
    } /*  */
   case PROJECT_TM_NME:
    {
    fscanf(fproject, "%s", &tempfile);
    break;
    } /*  */
   case PROJECT_FM_PTH:
    {
    fscanf(fproject, "%s", &modelpath);
    break;
    } /*  */
   case PROJECT_CL_PTH:
    {
    fscanf(fproject, "%s", &cloudpath);
    break;
    } /*  */
   case PROJECT_CL_NME:
    {
    fscanf(fproject, "%s", &cloudfile);
    break;
    } /*  */
   case PROJECT_WV_PTH:
    {
    fscanf(fproject, "%s", &wavepath);
    break;
    } /*  */
   case PROJECT_WV_NME:
    {
    fscanf(fproject, "%s", &wavefile);
    break;
    } /*  */
   case PROJECT_DF_PTH:
    {
    fscanf(fproject, "%s", &deformpath);
    break;
    } /*  */
   case PROJECT_DF_NME:
    {
    fscanf(fproject, "%s", &deformfile);
    break;
    } /*  */
   case PROJECT_IM_PTH:
    {
    fscanf(fproject, "%s", &imagepath);
    break;
    } /*  */
   case PROJECT_SN_NME:
    {
    fscanf(fproject, "%s", &sunfile);
    break;
    } /*  */
   case PROJECT_MN_NME:
    {
    fscanf(fproject, "%s", &moonfile);
    break;
    } /*  */
   case PROJECT_PP_PTH:
    {
    fscanf(fproject, "%s", &pcprojectpath);
    break;
    } /*  */
   case PROJECT_PF_PTH:
    {
    fscanf(fproject, "%s", &pcframespath);
    break;
    } /*  */
   case PROJECT_AO_PTH:
    {
    fscanf(fproject, "%s", &altobjectpath);
    break;
    } /*  */
   case PROJECT_DL_NEW:
    {
    short ReadOnly;

    fscanf(fproject, "%s", &filename);
    fscanf(fproject, "%hd", &ReadOnly);
    if (DL) DirList_Del(DL);
    DL = DirList_New(filename, ReadOnly);
    break;
    } /*  */
   case PROJECT_DL_ADD:
    {
    short ReadOnly;

    fscanf(fproject, "%s", &filename);
    fscanf(fproject, "%hd", &ReadOnly);
    if (DL) DirList_Add(DL, filename, ReadOnly);
    break;
    } /*  */
   case PROJECT_IA_SENSITIV:
    {
    fscanf(fproject, "%hd", &IA_Sensitivity);
    break;
    } /*  */
   case PROJECT_IA_GRIDSTYLE:
    {
    fscanf(fproject, "%hd", &IA_GridStyle);
    break;
    } /*  */
   case PROJECT_IA_GRIDSIZE:
    {
    fscanf(fproject, "%hd", &IA_GridSize);
    break;
    } /*  */
   case PROJECT_IA_MOVEMENT:
    {
    fscanf(fproject, "%hd", &IA_Movement);
    break;
    } /*  */
   case PROJECT_IA_GBDENS:
    {
    fscanf(fproject, "%hd", &IA_GBDens);
    break;
    } /*  */
   case PROJECT_IA_AUTODRAW:
    {
    fscanf(fproject, "%hd", &IA_AutoDraw);
    break;
    } /*  */
   case PROJECT_IA_ANIMSTART:
    {
    fscanf(fproject, "%hd", &IA_AnimStart);
    break;
    } /*  */
   case PROJECT_IA_ANIMEND:
    {
    fscanf(fproject, "%hd", &IA_AnimEnd);
    break;
    } /*  */
   case PROJECT_IA_COMPASSBDS:
    {
    fscanf(fproject, "%hd", &CompassBounds);
    break;
    } /*  */
   case PROJECT_IA_LANDBDS:
    {
    fscanf(fproject, "%hd", &LandBounds);
    break;
    } /*  */
   case PROJECT_IA_BOXBDS:
    {
    fscanf(fproject, "%hd", &BoxBounds);
    break;
    } /*  */
   case PROJECT_IA_PROFBDS:
    {
    fscanf(fproject, "%hd", &ProfileBounds);
    break;
    } /*  */
   case PROJECT_IA_GRIDBDS:
    {
    fscanf(fproject, "%hd", &GridBounds);
    break;
    } /*  */
   case PROJECT_MP_DITHER:
    {
    fscanf(fproject, "%hu", &MapDither);
    break;
    } /*  */
   case PROJECT_MP_CONTINT:
    {
    fscanf(fproject, "%hu", &ContInt);
    break;
    } /*  */
   case PROJECT_MP_CONTINT2:
    {
    fscanf(fproject, "%hd", &ContInterval);
    break;
    } /*  */
   case PROJECT_MP_DIGMODE:
    {
    fscanf(fproject, "%hd", &MP_DigMode);
    break;
    } /*  */
   case PROJECT_EMIA_SIZE:
    {
    fscanf(fproject, "%hd", &IA_Top);
    fscanf(fproject, "%hd", &IA_Left);
    fscanf(fproject, "%hd", &IA_Width);
    fscanf(fproject, "%hd", &IA_Height);
    break;
    } /* interactive view size */
   case PROJECT_EMIA_COMPSIZE:
    {
    fscanf(fproject, "%hd", &IA_CompTop);
    fscanf(fproject, "%hd", &IA_CompLeft);
    fscanf(fproject, "%hd", &IA_CompWidth);
    fscanf(fproject, "%hd", &IA_CompHeight);
    break;
    } /* interactive view size */
   case PROJECT_MAP_SIZE:
    {
    fscanf(fproject, "%hd", &MP_Top);
    fscanf(fproject, "%hd", &MP_Left);
    fscanf(fproject, "%hd", &MP_Width);
    fscanf(fproject, "%hd", &MP_Height);
    break;
    } /* interactive view size */
   case PROJECT_MP_ECOLEGEND:
    {
    for (i=0; i<6; i++)
     {
     fscanf(fproject, "%hd", &EcoUse[i]);
     fscanf(fproject, "%hd", &EcoLegend[i]);
     } /* for i=0... */
    break;
    } /* ecosystem legend */

/* Screen mode info */
   case PROJECT_SM_MODEID:
    {
    if (ScrnData) fscanf(fproject, "%lu", &ScrnData->ModeID);
    break;
    } /*  */
   case PROJECT_SM_WIDTH:
    {
    if (ScrnData) fscanf(fproject, "%ld", &ScrnData->Width);
    break;
    } /*  */
   case PROJECT_SM_HEIGHT:
    {
    if (ScrnData) fscanf(fproject, "%ld", &ScrnData->Height);
    break;
    } /*  */
   case PROJECT_SM_OTAG:
    {
    if (ScrnData) fscanf(fproject, "%lu", &ScrnData->OTag);
    break;
    } /*  */
   case PROJECT_SM_OVAL:
    {
    if (ScrnData) fscanf(fproject, "%lu", &ScrnData->OVal);
    break;
    } /*  */
   case PROJECT_SM_AUTOTAG:
    {
    if (ScrnData) fscanf(fproject, "%lu", &ScrnData->AutoTag);
    break;
    } /*  */
   case PROJECT_SM_AUTOVAL:
    {
    if (ScrnData) fscanf(fproject, "%lu", &ScrnData->AutoVal);
    break;
    } /*  */

   } /* switch */

  } /* for */

 fclose(fproject);

 if (PJ_Win)
  Set_PJ_Window();

 if (update)
  {
  if (EC_Win)
   {
   UnsetKeyFrame(EC_Win->Frame, 1, EC_Win->PalItem, 1);
   DisableKeyButtons(1);
   GetKeyTableValues(1, EC_Win->PalItem, 1);
   SetAllColorRequester();
   Set_EC_List(1);
   Set_EC_Item(EC_Win->PalItem);
   } /* if color requester open */
  if (EM_Win)
   {
   UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 1);
   DisableKeyButtons(0);
   GetKeyTableValues(0, EM_Win->MoItem, 1);
   Set_EM_List(1);
   Set_EM_Item(EM_Win->MoItem);
   Set_Radial_Txt(0);
   } /* if motion requester open */
  if (EMIA_Win)
   {
   IA->newgridsize = 0;
   if (! OpenNewIAGridSize())
    {
    Close_EMIA_Window(-1);
    } /* if no new grid */
   } /* if Camera View */
  if (EE_Win)
   {
   UnsetKeyFrame(EE_Win->Frame, 2, EE_Win->EcoItem, 1);
   DisableKeyButtons(2);
   GetKeyTableValues(2, EE_Win->EcoItem, 1);
   Set_EE_List(1);
   Set_EE_Item(EE_Win->EcoItem);
   } /* if motion requester open */
  if (ES_Win)
   {
   Set_ES_Window();
   } /* if settings window open */
  ResetTimeLines(-1);
  if (MapWind0)
   {
   struct clipbounds Clip;

   ClearWindow(MapWind0, 1);
   loadmapbase(0, 1);
   valueset();
   if (align)
    {
    if  ((AlignBox.High.Y != AlignBox.Low.Y) && (AlignBox.High.X - AlignBox.Low.Y))
     valuesetalign();
    else
     User_Message("Mapping Module: Align",
	"Illegal map registration values! High and low X or Y values are equal.",
	"OK", "o");
    } /* if align */
   if(AutoClear)
    {
    ClearWindow(MapWind0, backpen);
    MP->ptsdrawn = 0;
    } /* if */
   setclipbounds(MapWind0, &Clip);
   makemap(MapWind0, Clip.lowx, Clip.lowy, Clip.highx, Clip.highy,
    (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0));
   if(InterStuff)
    {
    ShowView_Map(&Clip);
    ShowPaths_Map(&Clip);
    } /* if */
   MapGUI_Update(MP); /* In case Exag changed as a result of loading Topos */
   } /* if map window */
  }/* if update */

 if (! LoadName || ForceLoad) Log(MSG_PROJ_LOAD, projectname);
 Proj_Mod = 0;
 return (1);

} /* LoadProject() */

/**********************************************************************/

short LoadDirList(void)
{
 char filename[256], path[256], name[32], Ptrn[32];
 short dummy, i;
 long item;
 FILE *fproject;

 strcpy(path, projectpath);
 strcpy(name, projectname);
 strcpy(Ptrn, "#?.proj");

 if (! getfilenameptrn(0, "Directory List", path, name, Ptrn))
  return (0);

 strmfp(filename, path, name);
 if ((fproject = fopen(filename, "r")) == NULL)
  {
  User_Message("Directory List: Load",
	"Can't open project file!\nOperation terminated.", "OK", "o");
  return (0);
  } /* if open fail */

 fgets(filename, 12, fproject);
 filename[10] = '\0';
 if (strcmp(filename, "WCSProject"))
  {
  fclose(fproject);
  User_Message("Directory List: Load", "Not a WCS Project file!\nOperation terminated.",
	"OK", "o");
  return (0);
  } /* if not WCS Project */

 for (; ;)
  {
  if (fscanf(fproject, "%ld", &item) == EOF) break;

  switch (item)
   {
   case PROJECT_PJ_PTH:
   case PROJECT_PJ_NME:
   case PROJECT_DB_PTH:
   case PROJECT_DB_NME:
   case PROJECT_PR_PTH:
   case PROJECT_PR_NME:
   case PROJECT_FR_PTH:
   case PROJECT_FR_NME:
   case PROJECT_LN_PTH:
   case PROJECT_LN_NME:
   case PROJECT_ZB_PTH:
   case PROJECT_ZB_NME:
   case PROJECT_BG_PTH:
   case PROJECT_BG_NME:
   case PROJECT_GR_PTH:
   case PROJECT_GR_NME:
   case PROJECT_ST_PTH:
   case PROJECT_ST_NME:
   case PROJECT_CM_PTH:
   case PROJECT_TM_PTH:
   case PROJECT_TM_NME:
   case PROJECT_FM_PTH:
   case PROJECT_CL_PTH:
   case PROJECT_CL_NME:
   case PROJECT_WV_PTH:
   case PROJECT_WV_NME:
   case PROJECT_DF_PTH:
   case PROJECT_DF_NME:
   case PROJECT_IM_PTH:
   case PROJECT_SN_NME:
   case PROJECT_MN_NME:
   case PROJECT_PP_PTH:
   case PROJECT_PF_PTH:
   case PROJECT_AO_PTH:
    {
    fscanf(fproject, "%s", &filename);
    break;
    } /* scan string */
   case PROJECT_DR_PTH:
    {
    fscanf(fproject, "%s", &dirname);
    break;
    } /* default dir */
   case PROJECT_DL_NEW:
    {
    short ReadOnly;

    fscanf(fproject, "%s", &filename);
    fscanf(fproject, "%hd", &ReadOnly);
    if (DL) DirList_Del(DL);
    DL = DirList_New(filename, ReadOnly);
    break;
    } /* new */
   case PROJECT_DL_ADD:
    {
    short ReadOnly;

    fscanf(fproject, "%s", &filename);
    fscanf(fproject, "%hd", &ReadOnly);
    if (DL) DirList_Add(DL, filename, ReadOnly);
    break;
    } /* add */
   case PROJECT_IA_SENSITIV:
   case PROJECT_IA_GRIDSTYLE:
   case PROJECT_IA_GRIDSIZE:
   case PROJECT_IA_MOVEMENT:
   case PROJECT_MP_DITHER:
   case PROJECT_MP_CONTINT:
   case PROJECT_MP_CONTINT2:
   case PROJECT_MP_DIGMODE:
   case PROJECT_IA_GBDENS:
   case PROJECT_IA_AUTODRAW:
   case PROJECT_IA_ANIMSTART:
   case PROJECT_IA_ANIMEND:
   case PROJECT_IA_COMPASSBDS:
   case PROJECT_IA_LANDBDS:
   case PROJECT_IA_BOXBDS:
   case PROJECT_IA_PROFBDS:
   case PROJECT_IA_GRIDBDS:
    {
    fscanf(fproject, "%hd", &dummy);
    break;
    } /* scan short value */
   case PROJECT_EMIA_SIZE:
   case PROJECT_EMIA_COMPSIZE:
   case PROJECT_MAP_SIZE:
    {
    fscanf(fproject, "%hd%hd%hd%hd", &dummy, &dummy, &dummy, &dummy);
    break;
    } /* scan multiple short values */
   case PROJECT_MP_ECOLEGEND:
    {
    for (i=0; i<12; i++)
     {
     fscanf(fproject, "%hd", &dummy);
     } /* for i=0... */
    break;
    } /* ecosystem legend */

   case PROJECT_SM_MODEID:
   case PROJECT_SM_WIDTH:
   case PROJECT_SM_HEIGHT:
   case PROJECT_SM_OTAG:
   case PROJECT_SM_OVAL:
   case PROJECT_SM_AUTOTAG:
   case PROJECT_SM_AUTOVAL:
    {
    fscanf(fproject, "%ld", &item);
    break;
    } /* scan long value */

   } /* switch */

  } /* for */

 fclose(fproject);

 Log(MSG_DIRLST_LOAD, name);
 Proj_Mod = 1;
 return (1);

} /* LoadDirList() */

/************************************************************************/

long PrintScreen(struct Screen *scr, UWORD srcx, UWORD srcy,
	UWORD srcw, UWORD srch, LONG destcols, UWORD iospecial)
{
 ULONG tmpl, PrinterSignals = 0L, signals;
 long error = PDERR_BADDIMENSION, done = 0;
 struct IODRPReq *iodrp;
 struct MsgPort *printerPort;
 struct ViewPort *vp;
 struct BusyWindow *BWPR;

 if (! scr)
  return (error);

 if (! destcols && ! iospecial)
  {
  if (! srcx && ! srcy && srcw == scr->Width && srch == scr->Height)
   {
   iospecial = SPECIAL_FULLCOLS | SPECIAL_ASPECT;
   } /* if print full screen */
  else
   {
   iospecial = SPECIAL_FRACCOLS | SPECIAL_ASPECT;
   tmpl = srcw;
   tmpl = tmpl << 16;
   destcols = (tmpl / scr->Width) << 16;
   } /* else */
  } /* if */

 if ((printerPort = CreatePort(0, 0)))
  {
  if ((iodrp =
	 (struct IODRPReq *)CreateExtIO(printerPort, sizeof (struct IODRPReq))))
   {
   if (! (error = OpenDevice("printer.device", 0, iodrp, 0)))
    {
    vp = &scr->ViewPort;
    iodrp->io_Command = PRD_DUMPRPORT;
    iodrp->io_RastPort = &scr->RastPort;
    iodrp->io_ColorMap = vp->ColorMap;
    iodrp->io_Modes = (ULONG)vp->Modes;
    iodrp->io_SrcX = srcx;
    iodrp->io_SrcY = srcy;
    iodrp->io_SrcWidth = srcw;
    iodrp->io_SrcHeight = srch;
    iodrp->io_DestCols = destcols;
/*    iodrp->io_DestRows = 0;*/
    iodrp->io_Special = iospecial;

    BWPR = BusyWin_New("Printing...", 1, 0, 'BWPR');

    Delay(250);

    SendIO(iodrp);

    while (! done)
     {
     if (DoMethod(app, MUIM_Application_Input, &signals) == ID_BW_CLOSE)
      {
      AbortIO((struct IORequest *)iodrp);
      WaitIO((struct IORequest *)iodrp);
      done = 1;
      } /* if MUI signal */
     else if (PrinterSignals & (1 << printerPort->mp_SigBit))
      {
      while (GetMsg(printerPort));
      done = 1;
      } /* else if printer signal */

     if (! done && signals)
      PrinterSignals = Wait(signals | (1 << printerPort->mp_SigBit));

     } /* while ! done */
    BusyWin_Del(BWPR);

    CloseDevice(iodrp);
    } /* if printer.device opened */
   DeleteExtIO(iodrp);
   } /* if iodrp */
  DeletePort(printerPort);
  } /* if printerPort */
 
 return (error);

} /* PrintScreen() */ 

/***********************************************************************/

