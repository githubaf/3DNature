/* GlobeMap.c (ne gisglobemap.c 14 Jan 1994 CXH)
** The globemap() function, not so enormous as it was.
** Built (ripped) from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent modifications by Gary R. Huber, 1992 & 1993.
*/

#include "WCS.h"
#include "GUIDefines.h"
#include <time.h>

STATIC_VAR float *ACosTable, *ASinTable,* SinTable,*CosTable;
STATIC_VAR __far char ILBMnum[32];
STATIC_VAR long lastfacect;
STATIC_VAR ULONG RenderWind0_Sig;
STATIC_VAR char statfile[64];
//STATIC_VAR long TrigTableEntries=361;   Aerger mit WCS.c ????

STATIC_FCN void Close_Render_Window(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN short InitDEMMap(struct Window *win, struct CloudData *CD); // used locally only -> static, AF 20.7.2021
STATIC_FCN short BuildTrigTables(void); // used locally only -> static, AF 26.7.2021
STATIC_FCN void FreeTrigTables(void); // used locally only -> static, AF 26.7.2021
STATIC_FCN short InitCloudMap(struct Window *win, struct CloudData *CD); // used locally only -> static, AF 26.7.2021


void globemap(void)
{
 ULONG flags, iflags, lmemblock;
 UBYTE c0, c1;
#ifdef ENABLE_STATISTICS
 FILE *fstat;
 short j;
 long elct;
 char extension[32];
#endif /* ENABLE_STATISTICS */
 char filename[256];
 short i, error = 0, altscrnheight, renderseg, altframe, altcomposite,
	altsaveIFF, WinWidth, WinHeight, EvenField;
 long savebmapsize, ct;
 struct BusyWindow *BWAN = NULL;
 union KeyFrame *AltKF = NULL;
 ULONG StartSecs, FirstSecs;
 struct Task *ThisTask;
 struct CloudData *CD = NULL;
 float RenderScale;

 ThisTask = FindTask(NULL);
 SetTaskPri(ThisTask, RenderTaskPri);
 if (RenderWind0) Close_Render_Window();
 if (DIAG_Win) Close_Diagnostic_Window();

 altscrnheight = settings.scrnheight;
 altsaveIFF = settings.saveIFF;
 altcomposite = settings.composite;

 FixPar(0, 0x1111);

 if (settings.fieldrender)
  {
  strcpy(tempfile, framefile);
  strcat(tempfile, ".temp");
  if ((AltKF = (union KeyFrame *)get_Memory(KFsize, MEMF_ANY)) == NULL)
   {
   User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto Cleanup2;
   } /* if out of memory */
  memcpy(AltKF, KF, KFsize);
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   KF[i].MoKey.KeyFrame *= 2;
   } /* for i=0... */
  settings.maxframes *= 2;
  settings.startframe *= 2;
  settings.startframe --;
  if (settings.startframe < 0)
   settings.startframe = 1;
  } /* if field rendering */

 if (RenderSize)
  {
  if (RenderSize == -1)
   {
   RenderScale = .5;
   if (settings.fractal > 0)
    settings.fractal -= 1;
   }
  else
   {
   RenderScale = .25;
   if (settings.fractal > 1)
    settings.fractal -= 1;
   if (settings.fractal > 1)
    settings.fractal -= 1;
   }
  settings.scrnwidth  *= RenderScale;
  settings.scrnheight *= RenderScale;
  PAR_FIRST_MOTION(6) *= RenderScale;
  PAR_FIRST_MOTION(7) *= RenderScale;
  PAR_FIRST_MOTION(10) /= RenderScale;
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Item == 6)
    KF[i].MoKey.Value *= RenderScale;
   else if (KF[i].MoKey.Item == 7)
    KF[i].MoKey.Value *= RenderScale;
   else if (KF[i].MoKey.Item == 10)
    KF[i].MoKey.Value /= RenderScale;
   else if (KF[i].MoKey.Item == 28)
    KF[i].MoKey.Value *= RenderScale;
   else if (KF[i].MoKey.Item == 31)
    KF[i].MoKey.Value *= RenderScale;
   } /* for i=0... */
  } /* if not render full size */

 if (settings.background || settings.zbuffer || settings.fieldrender)
	 settings.composite = 0;

 initmopar();
 initpar();
 frame = settings.startframe;

 Log(MSG_NULL, (CONST_STRPTR)"Render initialization complete."); 

#ifdef AMIGA_GUI
 if (render & 0x10)
  {
  c0 = 0x00;
  c1 = 0x01; 

  flags = ACTIVATE | SMART_REFRESH | WINDOWDEPTH | WINDOWCLOSE | WINDOWDRAG |
	REPORTMOUSE;
  iflags = 0;
  WinWidth = settings.scrnwidth + WCSScrn->WBorLeft + WCSScrn->WBorRight + 1;
  WinHeight = settings.scrnheight + WCSScrn->WBorTop + WCSScrn->WBorBottom + 1;
  if(WinWidth > WCSScrn->Width)
  	WinWidth = WCSScrn->Width;
  if(WinHeight > WCSScrn->Height)
   WinHeight = WCSScrn->Height;
  RenderWind0 = (struct Window *)
	make_window(0, 0, WinWidth, WinHeight, "WCS Render Window",
	flags, iflags, c0, c1, WCSScrn);
  if (!RenderWind0)
   {
   Log(ERR_WIN_FAIL, (CONST_STRPTR)"Render window.");
   User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Error opening render window!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto Cleanup2;
   }
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  SetRast(RenderWind0->RPort, 8);
  ModifyIDCMP(RenderWind0, CLOSEWINDOW | ACTIVEWINDOW | MOUSEBUTTONS);
  RenderWind0_Sig = 1L << RenderWind0->UserPort->mp_SigBit;
  WCS_Signals |= RenderWind0_Sig;
  } /* if render to screen */
#endif

 if (settings.rendersegs < 1) settings.rendersegs = 1;
 if (settings.startseg < 0) settings.startseg = 0;
 if (settings.startseg >= settings.rendersegs) settings.startseg = 0;
/* scrnwidth = WCSScrn->Width;*/
 if (WCSScrn->Width < settings.scrnwidth)
  drawoffsetX = (WCSScrn->Width - settings.scrnwidth) / 2;
 else drawoffsetX = RenderWind0->BorderLeft;
 if (WCSScrn->Height < settings.scrnheight / settings.rendersegs)
  drawoffsetY = (WCSScrn->Height - (settings.scrnheight / settings.rendersegs)) / 2;
 else drawoffsetY = RenderWind0->BorderTop;

 for (i=0; i<settings.overscan+settings.scrnheight/settings.rendersegs; i++)
	scrnrowzip[i] = i * settings.scrnwidth;

#ifdef AMIGA_GUI
 lmemblock = AvailMem(MEMF_FAST | MEMF_LARGEST);
 sprintf(str, "Largest available memory block = %lu", lmemblock); 
 Log(MSG_NULL, (CONST_STRPTR)str);
 lmemblock = AvailMem(MEMF_FAST);
 sprintf(str, "Fast memory available = %lu", lmemblock);
 Log(MSG_NULL, (CONST_STRPTR)str);
#endif

 bmapsize = settings.scrnwidth * (settings.overscan
		+ settings.scrnheight / settings.rendersegs);
 zbufsize = bmapsize * 4;
 QCmapsize = bmapsize * 4;
 if ((zbuf = (float *)get_Memory(zbufsize, MEMF_ANY)) == NULL)
  {
  User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory opening Z buffer!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto Cleanup2;
  } /* if */
 if (render & 0x01)
  {
  if ((error = openbitmaps(bitmap, bmapsize)) == 1)
   {
   User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory opening bitmaps!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto Cleanup2;
   } /* if */
  savebmapsize = settings.scrnwidth * 
		(settings.scrnheight / settings.rendersegs);
  } /* if render to RGB buffer */
 if ((bytemap = (USHORT *)get_Memory(bmapsize * 2, MEMF_CLEAR)) == NULL)
  {
  User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory opening anti-alias buffer!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto Cleanup2;
  } /* if */
 if (settings.reflections)
  {
  SlopeMap = (float *)get_Memory(bmapsize * sizeof (float), MEMF_ANY);
  ElevationMap = (float *)get_Memory(bmapsize * sizeof (float), MEMF_ANY);
  if ((ReflectionMap = (UBYTE *)get_Memory(bmapsize, MEMF_ANY)) == NULL
	|| ! ElevationMap || ! SlopeMap)
   {
   if (User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory allocating Reflection buffer!\n\
Continue without Reflections?", (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc"))
    settings.reflections = 0;
   else
    {
    error = 1;
    goto Cleanup2;
    } /* else cancel rendering */
   } /* if no memory for reflection buffer */
  } /* if reflections enabled */

 if (render & 0x100)
  {
  if (settings.rendersegs > 1 || settings.maxframes > 1)
   {
   if (! User_Message_Def((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Diagnostic buffers can't be generated for multiple segment or multiple frame renderings! Proceed rendering without them?",
           (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc", 1))
    goto Cleanup2;
   render ^= 0x100;
   } /* if */
  else
   {
   allocQCmaps();
   if (! (render & 0x100))
    {
    if (! User_Message_Def((CONST_STRPTR)"Render Module",
            (CONST_STRPTR)"Out of memory opening Diagnostic buffers! Proceed rendering without them?",
            (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc", 1))
     goto Cleanup2;
    } /* out of memory */
   } /* else */
  } /* if render QC buffer */

 if (! BuildKeyTable())
  {
  User_Message((CONST_STRPTR)"Render Module",
          (CONST_STRPTR)"Out of memory opening key frame table!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto Cleanup2;
  } /* if no key table */

 if (! LoadForestModels())
  {
  goto Cleanup2;
  } /* if forest model error */

 if (! BitmapImage_Load())
  {
  goto Cleanup2;
  } /* if bitmap image error */

 if (settings.waves)
  {
  strmfp(filename, wavepath, wavefile);
  if (! Wave_Load(filename, &Tsunami))
   {
   if (User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Error loading Wave File!\n\
(CONST_STRPTR)Continue without Waves?", (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc"))
    settings.waves = 0;
   else
    {
    error = 1;
    goto Cleanup2;
    } /* else cancel rendering */
   } /* if error loading */
  else
   {
   if (settings.fieldrender)
    {
    for (i=0; i<Tsunami->NumKeys; i++)
     Tsunami->WaveKey[i].EcoKey2.KeyFrame *= 2;
    } /* if field render */
   if (! BuildWaveKeyTable(Tsunami))
    {
    error = 1;
    goto Cleanup2;
    } /* if no memory for key table */
   } /* else loaded OK */
  } /* if waves */
 else
  Tsunami = NULL;

 if (settings.clouds)
  {
  if ((CD = CloudData_New()))
   {
   strmfp(filename, cloudpath, cloudfile);
   if (! Cloud_Load(filename, &CD))
    {
    if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
            (CONST_STRPTR)"Error loading Cloud Map file!\nContinue without cloud shadows?",
            (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc", 1))
     {
     CloudData_Del(CD);
     settings.clouds = 0;
     } /* if */
    else
     {
     error = 1;
     goto Cleanup2;
     } /* else we're outa here */
    } /* if Cloud Data read */
   else
    {
    if (settings.fieldrender)
     {
     for (i=0; i<CD->NumKeys; i++)
      CD->CloudKey[i].EcoKey2.KeyFrame *= 2;
     } /* if field render */
    if (! BuildCloudKeyTable(CD))
     {
     error = 1;
     goto Cleanup2;
     } /* if no memory for key table */
    } /* else loaded OK */
   } /* if Cloud Data allocated */
  else
   {
   if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
           (CONST_STRPTR)"Out of memory creating Cloud Map!\nContinue without cloud shadows?",
           (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc", 1))
    {
    settings.clouds = 0;
    } /* if */
   else
    {
    error = 1;
    goto Cleanup2;
    } /* else we're outa here */
   } /* else no cloud data memory */
  } /* if cloud shadows */

 if (settings.colrmap && settings.mastercmap)
  {
  char tag[32];
  short ReadError = 0, DummyShort;
  double val;
  FILE *fHdr;

/* read CMap header to determine size */

  if ((CMap = (struct Color_Map *)get_Memory(sizeof (struct Color_Map), MEMF_CLEAR)))
   {
   strmfp(filename, colormappath, colormapfile);
   strcat(filename, ".hdr");
   if ((fHdr = fopen(filename, "r")))
    {
    for (i=0; i<6; i++)
     {
     if (fscanf(fHdr, "%30s%le", tag, &val) == 2)
      {
      switch (tag[0])
       {
       case 'n':
       case 'N':
        {
        CMap->North = val;
        break;
        }
       case 's':
       case 'S':
        {
        CMap->South = val;
        break;
        }
       case 'e':
       case 'E':
        {
        CMap->East = val;
        break;
        }
       case 'w':
       case 'W':
        {
        CMap->West = val;
        break;
        }
       case 'r':
       case 'R':
        {
        CMap->Rows = (long)val;
        break;
        }
       case 'c':
       case 'C':
        {
        CMap->Cols = (long)val;
        break;
        }
       } /* switch */
      } /* if values read */  
     else
      {
      ReadError = 1;
      break;
      } /* else */
     } /* for i=0... */
    fclose(fHdr);
    } /* if hdr open */
   else
    {
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)"Master CMap Header File");
    ReadError = 1;
    } /* else no header file */

/* allocate cmap memory and read file */
/* If Image orientation CMap Rows are n-s and Cols are e-w
   If DEM   orientation CMap Rows are e-w and Cols are n-s
*/
   if (! ReadError)
    {
    if ((CMap->Size = CMap->Rows * CMap->Cols) > 0)
     {
     if (settings.cmaporientation)
      {
      CMap->StepLat = (CMap->North - CMap->South) / (CMap->Rows - 1);
      CMap->StepLon = (CMap->West - CMap->East) / (CMap->Cols - 1);
      } /* if image orientation */
     else
      {
      CMap->StepLat = (CMap->North - CMap->South) / (CMap->Cols - 1);
      CMap->StepLon = (CMap->West - CMap->East) / (CMap->Rows - 1);
      } /* else DEM orientation */
     if ((colmap[0]   = (UBYTE *)get_Memory(CMap->Size, MEMF_CLEAR)) != NULL 
	&& (colmap[1] = (UBYTE *)get_Memory(CMap->Size, MEMF_CLEAR)) != NULL
	&& (colmap[2] = (UBYTE *)get_Memory(CMap->Size, MEMF_CLEAR)) != NULL)
      {
      strmfp(filename, colormappath, colormapfile);
      if (! LoadImage(filename, 1, colmap, CMap->Cols, CMap->Rows, 1,
	&DummyShort, &DummyShort, &DummyShort))
       {
       strmfp(filename, colormappath, colormapfile);
       if (strcmp(&colormapfile[strlen(colormapfile) - 4], ".red"))
        strcat(filename, ".red");
       if (! LoadImage(filename, 1, colmap, CMap->Cols, CMap->Rows, 0,
	&DummyShort, &DummyShort, &DummyShort))
        {
        settings.colrmap = 0;
        } /* if image not loaded OK */
       } /* if image not loaded try another file name */
      } /* if memory allocated */
     else
      settings.colrmap = 0;
     } /* if size > 0 */
    else
     settings.colrmap = 0;
    } /* if ! ReadError */
   else
    settings.colrmap = 0;
   }
  else
   settings.colrmap = 0;

/* cleanup if color map error */

  if (! settings.colrmap)
   {
   if (colmap[0])    free_Memory(colmap[0], CMap->Size);
   if (colmap[1])    free_Memory(colmap[1], CMap->Size);
   if (colmap[2])    free_Memory(colmap[2], CMap->Size);
   colmap[0] = colmap[1] = colmap[2] = NULL;
   if (CMap)	    free_Memory(CMap, sizeof (struct Color_Map));
   CMap = NULL;
   if (! User_Message_Def((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error loading Master Color Map! See Status Log for more information.\n\
 Continue rendering without Color Map?",
 (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc", 1))
    goto Cleanup2;
   } /* if error loading color map */
  } /* open color map */

 if (settings.deformationmap)
  {
  strmfp(filename, deformpath, deformfile);
  if (readDEM(filename, &DeformMap) != 0)
   {
   if (! User_Message_Def((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error loading Strata Deformation Map!\n\
 Continue rendering without Deformation Map?",
 (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc", 1))
    goto Cleanup2;
   settings.deformationmap = 0;
   } /* if */
  } /* if deformation map */
 if ((NoiseMap = (UBYTE *)get_Memory(65536, MEMF_ANY)))
  {
  srand48(11111);
  for (ct=0; ct<65536; ct++)
   NoiseMap[ct] = (UBYTE)(127.5 + GaussRand() * 127.49999);
  } /* if noise map created */
 else
  {
  if (! User_Message_Def((CONST_STRPTR)"Render Module",
          (CONST_STRPTR)"Out of memory creating Noise Map!\n\
 Continue rendering without Texture Noise?",
 (CONST_STRPTR)"Continue|Cancel", (CONST_STRPTR)"oc", 1))
   goto Cleanup2;
  } /* else */

 BuildTrigTables();

#ifdef AMIGA_GUI
 Log(MSG_NULL, (CONST_STRPTR)"Render memory allocated.");
 lmemblock = AvailMem(MEMF_FAST | MEMF_LARGEST);
 sprintf(str, "Largest available memory block = %lu", lmemblock); 
 Log(MSG_NULL, (CONST_STRPTR)str);
 lmemblock = AvailMem(MEMF_FAST);
 sprintf(str, "Fast memory available = %lu", lmemblock);
 Log(MSG_NULL, (CONST_STRPTR)str);
#endif

 settings.scrnheight /= settings.rendersegs;
 high = settings.scrnheight - 1;
 oshigh = high + settings.overscan;
 wide = settings.scrnwidth - 1;

 if (settings.maxframes > 1)
  {
  BWAN = BusyWin_New("Animation", settings.maxframes, 1, MakeID('B','W','A','N'));
  } /* if rendering more than one frame */
  
 time((time_t *)&FirstSecs);

 for (i=1; i<=settings.maxframes && frame >= 0; i++)
  {
  time((time_t *)&StartSecs);
  if (Tsunami)	/* this needs to be before setvalues() or in setvalues */
   Wave_Init(Tsunami, frame);
  setvalues();
  ralt = EARTHRAD + PARC_RNDR_MOTION(0);
  qmax = sqrt(ralt * ralt - EARTHRAD * EARTHRAD);
  horscale = (PARC_RNDR_MOTION(10) * PROJPLANE * tan(.5 * PARC_RNDR_MOTION(11) *
        PiOver180) / 3.5469) / HORSCALEFACTOR;
  vertscale = horscale * settings.picaspect;
  cosviewlat = cos(PARC_RNDR_MOTION(1) * PiOver180);
  
  setview();

  if (CD)
   {
RepeatAlloc1:
   CloudWave_Init(CD, frame);
   if (Cloud_Generate(CD,
	(double)(frame * CD->Dynamic) / (double)(settings.fieldrender + 1)))
    {
    CD->Map.rows = CD->Cols - 1;
    CD->Map.columns = CD->Rows;
    CD->Map.steplat = (CD->Lat[0] - CD->Lat[1]) / (CD->Rows - 1);
    CD->Map.steplong = (CD->Lon[0] - CD->Lon[1]) / (CD->Cols - 1);
    CD->Map.lolat = CD->Lat[1] + CD->LatOff;
    CD->Map.lolong = CD->Lon[0] + CD->LonOff;
    CD->Map.elscale = ELSCALE_METERS;
    CD->Map.scrnptrsize = CD->PlaneSize;
    CD->Map.size = CD->PlaneSize / 2;	/* sizeof (float) / sizeof (short) */
RepeatAlloc2:
    if (! CD->Map.map)
     CD->Map.map = (short *)get_Memory (CD->Map.size, MEMF_ANY);
    if (CD->Map.map)
     {
     error = CloudShadow_Init(&CD->Map, CD);
     }
    else
     {
     if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
             (CONST_STRPTR)"Error creating Cloud Map! Either out of memory or user aborted.", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
      {
      goto RepeatAlloc2;
      } /* if try again */
     error = 1;
     } /* else out of memory */
    } /* if Cloud Map generated OK */
   else
    {
    if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
            (CONST_STRPTR)"Error creating Cloud Map! Either out of memory or user aborted.", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
     {
     goto RepeatAlloc1;
     } /* if try again */
    else
     error = 1;
    } /* else */
   if (error)
    break;
   } /* if cloud shadows */

  for (renderseg=settings.startseg; renderseg<settings.rendersegs; renderseg++)

   {
   if (settings.rendersegs > 1)
    {
    sprintf(str, "Segment %d", renderseg);
    Log(MSG_NULL, (CONST_STRPTR)str);
    } /* if */

   if (RenderWind0)
    {
    SetRast(RenderWind0->RPort, 8);
    } /* if render to screen */

   CenterX = PARC_RNDR_MOTION(6);
   CenterY = PARC_RNDR_MOTION(7) - renderseg
		* altscrnheight / settings.rendersegs; 
   autoactivate();

   for (ct=0; ct<bmapsize; ct ++) *(zbuf + ct) = FLT_MAX;
   if (ReflectionMap)
    memset(ReflectionMap, 0, bmapsize);
   if (ElevationMap)
    for (ct=0; ct<bmapsize; ct ++) *(ElevationMap + ct) = 0.0;
   if (SlopeMap)
    for (ct=0; ct<bmapsize; ct ++) *(SlopeMap + ct) = 0.0;

   lastfacect = 0;

   for (ct=0; ct<ECOPARAMS; ct++) ecocount[ct] = 0;

#ifdef ENABLE_STATISTICS
   if (settings.statistics)
    {
    samples = 0;
    meanaspect = meanslope = meanel = 0.0;
    for (ct=0; ct<37; ct++)
     {
     for (elct=0; elct<20; elct++) statcount[ct][elct] = 0;
     } /* for */
    } /* if */
#endif /* ENABLE_STATISTICS */
   if (! settings.linetoscreen)
    {
    strmfp(str, linepath, linefile);
    sprintf(filename, "%s%d", str, frame);
    if ((fvector = fopen(filename, "w")) == NULL)
     {
     Log(ERR_OPEN_FAIL, (CONST_STRPTR)linefile);
     if (! User_Message((CONST_STRPTR)linefile,
             (CONST_STRPTR)"Can't open vector file for output!\nContinue rendering without vectors?",
             (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc"))
      {
      error = 1;
      break;
      } /* if cancel rendering */
     settings.linetoscreen = -1;
     } /* if */
    } /* if */

/* map topo objects and surfaces */
   if ((error = InitDEMMap(RenderWind0, CD)) == 1)
    {
    if (! settings.linetoscreen) fclose(fvector);
    break;
    }

/* map vector objects */
   error = InitVectorMap(RenderWind0, 1, 0);
   if (! settings.linetoscreen) fclose(fvector);
   if (error) break;

   Log(DTA_NULL, (CONST_STRPTR)"Fractals:");
   for (ct=0; ct<=settings.fractal; ct++)
    {
    if (fracount[ct])
     {
     sprintf(str, "Level %ld = %ld", ct, fracount[ct]);
     Log(MSG_UTIL_TAB, (CONST_STRPTR)str);
     fracount[ct] = 0;
     } /* if fracount */
    } /* for ct=0... */

   Log(DTA_NULL, (CONST_STRPTR)"Ecosystems:");
   for (ct=0; ct<ECOPARAMS; ct++)
    {
    if (ecocount[ct])
     {
     sprintf(str, "%2ld. %-24s %ld",
		ct, PAR_NAME_ECO(ct), ecocount[ct]);
     Log(MSG_UTIL_TAB, (CONST_STRPTR)str);
     } /* if ecocount */
    } /* for ct=0... */

/* merge with background and z buffer or render sky */

   if (settings.zbuffer || settings.background)
    {
    altframe = frame;
    if (settings.fieldrender)
     frame = frame / 2 + frame % 2;
    MergeZBufBack(renderseg, settings.scrnwidth, settings.scrnheight, RenderWind0);
    frame = altframe;
    } /* if */
   else
    {
    if ((error = makesky(renderseg, RenderWind0)) == 1)
     {
     if (! settings.linetoscreen) fclose(fvector);
     break;
     } /* if user canceled during sky rendering */
    } /* else */

/* add celestial objects */

   if ((error = Celestial_Bodies(bitmap, (long)settings.scrnwidth,
	 (long)settings.scrnheight, RenderWind0)))
    break;

/* add clouds */
   
   if (settings.realclouds)
    {
    if ((error = InitCloudMap(RenderWind0, CD)) == 1)
     break;
    } /* if */

/* apply reflections */

   if (settings.reflections)
    {
    if ((error = Reflection_Render(RenderWind0)) == 1)
     break;
    } /* if reflections - remember this can be disabled if out of memory */

   if (render & 0x01)
    {
/* apply blur operator */

    if (settings.antialias) antialias();

/* scale image */


/* interlace even field with previously rendered odd */

    EvenField = 0;
    if (settings.fieldrender && ! (frame % 2))
     {
     EvenField = 1;
     strmfp(ILBMname, temppath, tempfile);
     if (settings.composite || settings.rendersegs == 1)
      {
      if (frame - 1 < 10) sprintf(ILBMnum, "00%d", frame - 1);
      else if (frame - 1 < 100) sprintf(ILBMnum, "0%d", frame - 1);
      else sprintf(ILBMnum, "%d", frame - 1);
      }
     else
      {
      if (settings.rendersegs <= 26)
       {
       if (frame - 1 < 10) sprintf(ILBMnum, "%00d%c", frame - 1, 65 + renderseg);
       else if (frame - 1 < 100) sprintf(ILBMnum, "%0d%c", frame - 1, 65 + renderseg);
       else sprintf(ILBMnum, "%d%c", frame - 1, 65 + renderseg);
       }
      else
       {
       if (frame - 1 < 10) sprintf(ILBMnum, "%00d%c%c", frame - 1, 65 + renderseg / 26, 65 + renderseg % 26);
       else if (frame - 1 < 100) sprintf(ILBMnum, "%0d%c%c", frame - 1, 65 + renderseg / 26, 65 + renderseg % 26);
       else sprintf(ILBMnum, "%d%c%c", frame - 1, 65 + renderseg / 26, 65 + renderseg % 26);
       }
      } /* else */
     strcat(ILBMname, ILBMnum);
     error = InterlaceFields(ILBMname, settings.scrnwidth, settings.scrnheight,
	settings.fielddominance);
     if (error)
      {
      User_Message((CONST_STRPTR)"Render Module",
              (CONST_STRPTR)"Error interlacing fields!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
      break;
      } /* if interlace error */
     } /* if field render and even field */

SaveRepeat:
    if (settings.fieldrender && ! EvenField)
     strmfp(ILBMname, temppath, tempfile);
    else
     strmfp(ILBMname, framepath, framefile);
    altframe = frame;
    if (settings.fieldrender && EvenField) frame /= 2;
    if ((settings.composite && (EvenField || ! settings.fieldrender))
	|| settings.rendersegs == 1)
     {
     if (frame < 10) sprintf(ILBMnum, "00%d", frame);
     else if (frame < 100) sprintf(ILBMnum, "0%d", frame);
     else sprintf(ILBMnum, "%d", frame);
     }
    else
     {
     if (settings.rendersegs <= 26)
      {
      if (frame < 10) sprintf(ILBMnum, "%00d%c", frame, 65 + renderseg);
      else if (frame < 100) sprintf(ILBMnum, "%0d%c", frame, 65 + renderseg);
      else sprintf(ILBMnum, "%d%c", frame, 65 + renderseg);
      }
     else
      {
      if (frame < 10) sprintf(ILBMnum, "%00d%c%c", frame, 65 + renderseg / 26, 65 + renderseg % 26);
      else if (frame < 100) sprintf(ILBMnum, "%0d%c%c", frame, 65 + renderseg / 26, 65 + renderseg % 26);
      else sprintf(ILBMnum, "%d%c%c", frame, 65 + renderseg / 26, 65 + renderseg % 26);
      }
     } /* else */
    strcat(ILBMname, ILBMnum);
    if (settings.fieldrender && ! EvenField)
     {
     settings.saveIFF = 0;
     settings.composite = 0;
     error = savebitmaps(bitmap, savebmapsize, renderseg);
     settings.saveIFF = altsaveIFF;
     settings.composite = altcomposite;
     } /* if field rendering */ 
    else if (settings.saveIFF == 2)
      error = saveILBM(24, 0, NULL, bitmap, scrnrowzip, renderseg, settings.rendersegs,
	settings.composite, settings.scrnwidth, settings.scrnheight);
    else error = savebitmaps(bitmap, savebmapsize, renderseg); 
    frame = altframe;

    if (error)
     {
     if (User_Message_Def((CONST_STRPTR)"Render Module: Save",
             (CONST_STRPTR)"Error saving bitmapped image! Try another device?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
      {
      if (getfilename(1, "New Frame Save Path", framepath, framefile))
       {
       Proj_Mod = 1;
       goto SaveRepeat;
       }
      }
      break;
     } /* if error */

    } /* if render to RGB buffers */

RepeatSaveZBuf:
   if (settings.exportzbuf && (! settings.fieldrender || EvenField))
    {
    error = SaveZBuf(settings.zformat, renderseg, savebmapsize, bitmap[0],
	zbuf, ILBMname, 0, 0);
    if (error == 2)
     {
     if (User_Message_Def((CONST_STRPTR)"Render Module",
             (CONST_STRPTR)"Out of memory saving Z Buffer!\n", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
      {
      goto RepeatSaveZBuf;
      } /* if user wants to try again */
     break;
     } /* memory failed */
    else if (error == 1)
     {
     if (User_Message_Def((CONST_STRPTR)"Render Module: Save",
             (CONST_STRPTR)"Error saving Z Buffer! Try another device?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
      {
      char tempzfile[32] = {0};

      if (getfilename(1, "Z Buffer Save Path", framepath, tempzfile))
       {
       strmfp(ILBMname, framepath, framefile);
       strcat(ILBMname, ILBMnum);
       Proj_Mod = 1;
       goto RepeatSaveZBuf;
       }
      }
     break; 
     } /* if disk full or other file error */
    } /* if save z buffer */

   if (renderseg < settings.rendersegs - 1)
    {
    if (render & 0x01)
     {
     memset(bitmap[0], 0, bmapsize);
     memset(bitmap[1], 0, bmapsize);
     memset(bitmap[2], 0, bmapsize);
     memset(bytemap, 0, bmapsize * 2);
     } /* if render to RGB buffer */
    } /* if not last segment */
   } /* for renderseg=0... */

  if (error) break; 

  Log_ElapsedTime(StartSecs, FirstSecs, (long)i);

  if (i<settings.maxframes)
   {
   if (render & 0x10)
    {
#ifdef AMIGA_GUI
    Move(RenderWind0->RPort, 0, 0);
    ClearScreen(RenderWind0->RPort);
#endif
    } /* if render to screen */

   if (settings.fieldrender)
    {
    if (EvenField)
     frame += (settings.stepframes * 2 - 1);
    else
     frame += 1;
    }
   else
    frame += settings.stepframes;
   if (render & 0x01)
    {
    memset(bitmap[0], 0, bmapsize);
    memset(bitmap[1], 0, bmapsize);
    memset(bitmap[2], 0, bmapsize);
    memset(bytemap, 0, bmapsize * 2);
    } /* if render to RGB buffer */
   } /* if i<settings.maxframes */

  if (BWAN)
   {
   if (CheckInput_ID() == ID_BW_CLOSE)
    break;
   BusyWin_Update(BWAN, i);
   } /* if Animation Busy Window open */

  } /* for i=1 i<=settings.maxframes */

 if (! error && ((render & 0x100) && (render & 0x10)))
  {
  Open_Diagnostic_Window(RenderWind0, "Render Window");
  } /* if render to QC buffer */

Cleanup2:
 if (BWAN)
	BusyWin_Del(BWAN);

 FreeTrigTables();

 if (! DIAG_Win)
  {
  if (zbuf) free_Memory(zbuf, zbufsize);
  zbuf = (float *)NULL;
  freeQCmaps();
#ifdef AMIGA_GUI
  if (RenderWind0) Close_Render_Window();
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
#endif /* AMIGA_GUI */
  } /* if no diagnostics */

 for (i=0; i<ECOPARAMS; i++)
  {
  if (FM[i].TM)
   free_Memory(FM[i].TM, FM[i].Items * sizeof (struct TreeModel));
  FM[i].TM = NULL;
  } /* for i=2... */
 BitmapImage_Unload();
 if (bytemap) free_Memory(bytemap, bmapsize * 2);
 bytemap = NULL;
 if (KT) FreeKeyTable();
 closebitmaps(bitmap, bmapsize);
 bitmap[0] = bitmap[1] = bitmap[2] = NULL;
 if (settings.colrmap && settings.mastercmap)
  {
  if (colmap[0])    free_Memory(colmap[0], CMap->Size);
  if (colmap[1])    free_Memory(colmap[1], CMap->Size);
  if (colmap[2])    free_Memory(colmap[2], CMap->Size);
  colmap[0] = colmap[1] = colmap[2] = NULL;
  if (CMap)	    free_Memory(CMap, sizeof (struct Color_Map));
  CMap = NULL;
  } /* if use master color map */
 if (Tsunami)
  WaveData_Del(Tsunami);
 Tsunami = NULL;
 if (ReflectionMap) free_Memory(ReflectionMap, bmapsize);
 ReflectionMap = NULL;
 if (ElevationMap) free_Memory(ElevationMap, bmapsize * sizeof (float));
 ElevationMap = NULL;
 if (SlopeMap) free_Memory(SlopeMap, bmapsize * sizeof (float));
 SlopeMap = NULL;
 if (CD)
  {
  if (CD->Map.map) free_Memory(CD->Map.map, CD->Map.size);
  CD->Map.map = (short *)NULL;
  CloudData_Del(CD);
  CD = NULL;
  } /* if cloud shadows */
 if (DeformMap.map)
  free_Memory(DeformMap.map, DeformMap.size);
 DeformMap.map = NULL;
 if (NoiseMap)
  free_Memory(NoiseMap, 65536);
 NoiseMap = NULL;

 if (settings.fieldrender)
  {
  if (AltKF)
   {
   memcpy(KF, AltKF, KFsize);
   free_Memory(AltKF, KFsize);
   } /* if Alt Key Frame allocated */
  } /* if field render */

#ifdef ENABLE_STATISTICS
 if (settings.statistics)
  {
  if (User_Message("Render Module: Statistics",
	"Save statistical data to file?", "YES|NO", "yn"));
   {
NewStatFile:
   if (getstatfile(1))
    {
    stcgfe(extension, statname);		/* lattice function */
    sprintf(str, ".%s", extension);
    if (strcmp(str, ".stat"))
     {
     strcat(statname, ".stat");
     } /* if */
    strmfp(filename, statpath, statname);
    if ((fstat = fopen(filename, "w")) == NULL)
     {
     Log(ERR_OPEN_FAIL, statname);
     if (User_Message("Render Module: Statistics", 
	"Can't open statistics file! Try another?", "OK|CANCEL", "oc"))
      goto NewStatFile;
     } /* if */
    else
     {
     fprintf(fstat,"STAT\n37\n20\n");
     for (i=0; i<37; i++)
      {
      for (j=0; j<20; j++)
       {
       fprintf(fstat, "%d\n", statcount[i][j]);
       } /* for */
      } /* for */
     fprintf(fstat,"%f\n%f\n%f\n%d\n", meanaspect * PiUnder180 / samples,
	meanslope * PiUnder180 / samples, meanel / samples, samples);
     fclose(fstat);
     sprintf(str, "Statistical data saved. %s", statname);
     Log(MSG_NULL, str);
     } /* else */
    } /* if */
   } /* if(ans) */
  } /* if */
#endif /* ENABLE_STATISTICS */

 UndoPar(0, 0x1111);
 render = 0;
 settings.scrnheight = altscrnheight;
 if (InterWind0)
  {
  drawoffsetX = InterWind0->BorderLeft;
  drawoffsetY = InterWind0->BorderTop;
  } /* if Camera View is open */
 SetTaskPri(ThisTask, 0);

} /* globemap() */

/**********************************************************************/

STATIC_FCN short InitDEMMap(struct Window *win, struct CloudData *CD) // used locally only -> static, AF 20.7.2021
{
 char filename[256], colormapdir[256], FrameStr[16], elevfile[32], elevpath[256];
 short Ans, OrigOBN, error = 0, ct, objectlimit, objectcount, OpenOK,
	CMapEnabled = 1, DummyShort;
 long fract, FacePoints;
 struct elmapheaderV101 map;
 struct BusyWindow *BWIM = NULL;
 struct DirList *DLItem;

 memset(&map, 0, sizeof (struct elmapheaderV101));

 if (settings.colrmap && ! settings.mastercmap)
  {
  strcpy(colormapdir, colormappath);
  if (chdir(colormapdir))
   {
   short i = 0;

   for (; i<10; i++)
    {
    strcpy(colormapdir, colormappath);
    for (ct=0; ct<i; ct++) strcat(colormapdir, "0");
    sprintf(str, "%hd", frame);
    strcat(colormapdir, str);
    if (! chdir(colormapdir)) break;
    } /* for i=0... */
   if (i == 10)
    {
    CMapEnabled = 0;
    Log(WNG_NULL, (CONST_STRPTR)"Color map directory not found.");
    } /* directory not found by appending frame # */
   } /* if directory not found as entered by user */
  chdir(path);
  } /* if use color maps */
 OrigOBN = OBN;
 Log(DTA_NULL, (CONST_STRPTR)"Polygons:");
 objectlimit = RenderObjects;

 if (settings.fieldrender)
  {
  sprintf(FrameStr, "Frame %d", (frame / 2) + (frame % 2));
  if (frame % 2)
   strcat(FrameStr, "A");
  else
   strcat(FrameStr, "B");
  sprintf(str, "/%d", settings.maxframes);
  strcat(FrameStr, str);
  } /* if */
 else
  sprintf(FrameStr, "Frame %d/%d", frame, settings.maxframes);
 BWIM = BusyWin_New(FrameStr, objectlimit, 1, MakeID('B','W','I','M'));

 for (objectcount=0; objectcount<objectlimit; objectcount++)
  {
  OBN = RenderList[objectcount][0];
  if ((! strcmp(DBase[OBN].Special, "TOP") ||
	! strcmp(DBase[OBN].Special, "SFC")) && (DBase[OBN].Flags & 2))
   {
RepeatLoad:
   OpenOK = 0;

   DLItem = DL;
   while (DLItem)
    {
    strmfp(filename, DLItem->Name, DBase[OBN].Name);
    strcat(filename, ".elev");
    if (readDEM(filename, &map) != 0)
     {
     DLItem = DLItem->Next;
     } /* if open/read fails */
    else
     {
     OpenOK = 1;
     strcpy(elevpath, DLItem->Name);
     strcpy(elevfile, DBase[OBN].Name);
     strcat(elevfile, ".elev");
     break;
     } /* else open succeeds */
    } /* while */

   if (! OpenOK)
    {
    error = 1;
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)DBase[OBN].Name);
    break;
    } /* if file not found */

   FacePoints = 6 * map.columns;
   map.lmap = (long *)get_Memory (map.size * 2, MEMF_ANY);
   map.scrnptrx = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
   map.scrnptry = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
   map.scrnptrq = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
   if (! map.lmap || ! map.scrnptrx || ! map.scrnptry || ! map.scrnptrq)
    {
    sprintf(str, "Out of memory reading map %s!", DBase[OBN].Name);
    if (User_Message_Def((CONST_STRPTR)"Render Module: Topo", (CONST_STRPTR)str, (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
     {
     error = -1;
     goto MapCleanup;
     } /* if try again */
    error = 1;
    goto MapCleanup;
    } /* if */

   if (settings.smoothfaces || ! strcmp(DBase[OBN].Special, "SFC") ||
	settings.mapassfc)
    {
RetrySmooth:
    if ((map.face = (struct faces *)get_Memory
	(FacePoints * sizeof (struct faces), MEMF_ANY)) == NULL)
     {
     if (User_Message_Def((CONST_STRPTR)"Render Module",
             (CONST_STRPTR)"Out of memory allocating Smoothing Index array!",
             (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
      goto RetrySmooth;
     else
      {
      error = 1;
      goto MapCleanup;
      } /* else cancel rendering */
     } /* if no memory for face array */
    if (! strcmp(DBase[OBN].Special, "SFC") || settings.mapassfc)
     map.ForceBath = 1;
    else
     map.ForceBath = 0;
    } /* if smooth faces */
   else
    {
    map.face = NULL;
    map.ForceBath = 0;
    } /* else no smoothing */

   if (settings.fractalmap || (! settings.fixfract && settings.displace))
    {
    map.fractalsize = map.rows * (map.columns - 1) * 2;
RetryFractal:
    if ((map.fractal = (BYTE *)get_Memory(map.fractalsize, MEMF_CLEAR)) != NULL)
     {
     if (settings.fractalmap)
      {
      strmfp(filename, dirname, DBase[OBN].Name);
      strcat(filename, ".frd");
      LoadImage(filename, 0, (UBYTE **)&map.fractal,
		2 * (map.columns - 1), map.rows, 1, &DummyShort,
		&DummyShort, &DummyShort);
      } /* if */
     } /* if memory allocated */
    else
     {
     if ((Ans = User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory allocating Fractal Map array!\n\
Continue without Fractal Maps or retry?", (CONST_STRPTR)"Continue|Retry|Cancel", (CONST_STRPTR)"orc")) == 1)
      settings.fractalmap = 0;
     else if (Ans == 2)
      goto RetryFractal;
     else
      {
      error = 1;
      goto MapCleanup;
      } /* else cancel rendering */
     } /* if no memory for fractal map */
    } /* if */
   else
    map.fractal = NULL;

   if (strcmp(DBase[OBN].Special, "SFC") && ! settings.mapassfc)
    {
    if (settings.colrmap && settings.mastercmap)
     cmap = 1;
    else if (settings.colrmap && CMapEnabled)
     {
     cmap = 1;
     if ((colmap[0] = (UBYTE *)get_Memory(map.size / 2, MEMF_CLEAR)) != NULL
	&& (colmap[1] = (UBYTE *)get_Memory(map.size / 2, MEMF_CLEAR)) != NULL
	&& (colmap[2] = (UBYTE *)get_Memory(map.size / 2, MEMF_CLEAR)) != NULL)
      {
      strmfp(filename, colormapdir, DBase[OBN].Name);
      if (! LoadImage(filename, 1, colmap, map.columns, map.rows + 1, 1,
	&DummyShort, &DummyShort, &DummyShort))
       {
       strmfp(filename, colormapdir, DBase[OBN].Name);
       strcat(filename, ".red");
       if (! LoadImage(filename, 1, colmap, map.columns, map.rows + 1, 0,
		&DummyShort, &DummyShort, &DummyShort))
        {
        cmap = 0;
        free_Memory(colmap[0], map.size / 2);
        free_Memory(colmap[1], map.size / 2);
        free_Memory(colmap[2], map.size / 2);
        colmap[0] = colmap[1] = colmap[2] = NULL;
        } /* if image not loaded OK */
       } /* if image not loaded try another file name */
      } /* if memory allocated */
     else cmap = 0;
     } /* if settings.colrmap */
    else cmap = 0;

//EndCMap:

    OpenOK = 0;

    DLItem = DL;
    while (DLItem)
     {
     strmfp(filename, DLItem->Name, DBase[OBN].Name);
     strcat(filename, ".relel");
     if (readDEM(filename, &relelev) != 0)
      {
      DLItem = DLItem->Next;
      } /* if relel file open/read failed */
     else
      {
      OpenOK = 1;
      break;
      } /* else file found */
     } /* while */
    if (! OpenOK)
     {
     if (makerelelfile(elevpath, elevfile))
      {
      strmfp(filename, elevpath, elevfile);	/* elevfile suffix changed to .relel */
      if (readDEM(filename, &relelev) != 0)
       {
       Log(WNG_OPEN_FAIL, (CONST_STRPTR)elevfile);
       } /* if still no relel file */
      } /* if make relel successful */
     } /* if relel file not found */
    if (relelev.map && (relelev.size != map.size))
     {
     free_Memory(relelev.map, relelev.size);
     relelev.map = (short *)NULL;
     } /* if */
    if (relelev.map == NULL)
     {
     relelev.size = map.size;
     relelev.map = get_Memory(relelev.size, MEMF_CLEAR);
     if (! relelev.map)
      {
      error = 1;
      goto MapCleanup;
      } /* if */
     } /* if */

    TreePixArraySize = 10000;
    if ((TreePix = get_Memory(TreePixArraySize, MEMF_CLEAR)) == NULL)
     {
     User_Message((CONST_STRPTR)"Render Module",
             (CONST_STRPTR)"Out of memory allocating antialias buffer!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     error = 1;
     goto MapCleanup;
     } /* if memory fail */
    map.MapAsSFC = 0;
    } /* if not surface */
   else
    map.MapAsSFC = 1;

   SubPixArraySize = 10000;
   EdgeSize = 1000;
   SubPix = get_Memory(SubPixArraySize, MEMF_CLEAR);
   Edge1 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
   Edge2 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
   if (! Edge1 || ! Edge2 || ! SubPix)
    {
    User_Message((CONST_STRPTR)"Render Module",
            (CONST_STRPTR)"Out of memory allocating antialias and edge buffers!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    error = 1;
    goto MapCleanup;
    } /* if memory fail */

   error = maptopoobject(&map, win, objectcount + 1, objectlimit, CD);
   if (error) goto MapCleanup;
   fract = 0;
   for (ct=0; ct<=settings.fractal; ct++) fract += fracount[ct];
   sprintf(str, "%s = %ld\n", DBase[OBN].Name, fract - lastfacect);
   Log(MSG_UTIL_TAB, (CONST_STRPTR)str);
   lastfacect = fract;

 MapCleanup:
   if (SubPix) free_Memory(SubPix, SubPixArraySize);
   SubPix = NULL;
   if (TreePix) free_Memory(TreePix, TreePixArraySize);
   TreePix = NULL;
   if (Edge1) free_Memory(Edge1, EdgeSize);
   if (Edge2) free_Memory(Edge2, EdgeSize);
   Edge1 = Edge2 = NULL;
   if (map.map) free_Memory(map.map, map.size);
   if (map.lmap) free_Memory(map.lmap, map.size * 2);
   if (relelev.map) free_Memory(relelev.map, relelev.size);
   if (map.scrnptrx) free_Memory(map.scrnptrx, map.scrnptrsize);
   if (map.scrnptry) free_Memory(map.scrnptry, map.scrnptrsize);
   if (map.scrnptrq) free_Memory(map.scrnptrq, map.scrnptrsize);
   if (map.face)     free_Memory(map.face, FacePoints * sizeof (struct faces));
   if (map.fractal) free_Memory(map.fractal, map.fractalsize);
   map.map = relelev.map = (short *)NULL;
   map.lmap = (long *)NULL;
   map.scrnptrx = map.scrnptry = (float *)NULL;
   map.scrnptrq = (float *)NULL;
   map.face = NULL;
   map.fractal = NULL;
   if (cloudmap) free_Memory(cloudmap, map.size / 2);
   if (! settings.mastercmap)
    {
    if (colmap[0]) free_Memory(colmap[0], map.size / 2);
    if (colmap[1]) free_Memory(colmap[1], map.size / 2);
    if (colmap[2]) free_Memory(colmap[2], map.size / 2);
    colmap[0] = colmap[1] = colmap[2] = (UBYTE *)NULL;
    } /* if */
   cloudmap = (UBYTE *)NULL;
   if (error == -1) goto RepeatLoad;
   if (error) break;

   } /* if topo object */
  if (BWIM)
   {
   BusyWin_Update(BWIM, objectcount + 1);
   } /* if Image Busy Window open */
  } /* for objectcount=0... */

 OBN = OrigOBN;

 if (BWIM)
  BusyWin_Del(BWIM);
 return (error);

} /* InitDEMMap() */

/**********************************************************************/

short InitVectorMap(struct Window *win, short zbufplot, short override)
{
 char filename[256], *LastDir = NULL;
 short error = 0, OrigOBN, FreeVec, LoadIt, Points, PointSize = 2 * sizeof (double) + sizeof (short);
 FILE *fMObj;
 struct elmapheaderV101 map;
 struct BusyWindow *BWVC;

 OrigOBN = OBN;
 if (settings.linetoscreen < 0 && ! override)
  {
  return(0);
  } /* if no vectors */

 BWVC = BusyWin_New("Vectors", NoOfObjects, 1, MakeID('B','W','V','C'));

/* try opening master Object file */
 strmfp(filename, dbasepath, dbasename);
 strcat(filename, ".MDB");
 if ((fMObj = fopen(filename, "rb")) != NULL)
  {
  char Title[32];
  short Objects;

  fread(Title, 16, 1, fMObj);
  if (! strcmp(Title, "WCSMasterObject"))
   {
   fread(&Objects, sizeof (short), 1, fMObj);
   if (Objects != NoOfObjects)
    {
    fclose(fMObj);
    fMObj = NULL;
    } /* if wrong number of objects */
   } /* if title correct */
  else
   {
   fclose(fMObj);
   fMObj = NULL;
   } /* if incorrect file title */
  } /* if master object file opened */

 for (OBN=0, LoadIt=1; OBN<NoOfObjects; OBN++, LoadIt=1)
  {
  if (fMObj)
   fread(&Points, sizeof (short), 1, fMObj);
  if (! (DBase[OBN].Flags & 2))
   {
   if (fMObj)
    fseek(fMObj, (Points + 1) * PointSize, SEEK_CUR);
   continue;
   } /* if */
  if (DBase[OBN].Special[0] == 'V')
   {
   if (! DBase[OBN].Lat)
    {
    FreeVec = 1;
    if (fMObj)
     {
     if (Points == DBase[OBN].Points)
      {
      LoadIt = 0;
      if (allocvecarray(OBN, DBase[OBN].Points, 1))
       {
       if (fread((char *)&DBase[OBN].Lon[0],
	 (DBase[OBN].Points + 1) * sizeof (double), 1, fMObj) == 1)
        {
        if (fread((char *)&DBase[OBN].Lat[0],
	 (DBase[OBN].Points + 1) * sizeof (double), 1, fMObj) == 1)
         {
         if (fread((char *)&DBase[OBN].Elev[0],
		 (DBase[OBN].Points + 1) * sizeof (short), 1, fMObj) != 1)
          error = 1;
         } /* if */
        else
         error = 1;
        } /* if */
       else
        error = 1;
       } /* if memory allocated */
      else
       error = 1;
      } /* if correct number of points */
     else
      fseek(fMObj, (Points + 1) * PointSize, SEEK_CUR);
     } /* if Master Object file open */
    if (error)
     goto LineCleanup;
    if (LoadIt)
     {
     if (Load_Object(OBN, &LastDir) > 0)
      goto LineCleanup;
     } /* if */
    } /* if object not already in memory */
   else
    {
    if (fMObj)
     fseek(fMObj, (Points + 1) * PointSize, SEEK_CUR);
    FreeVec = 0;
    }
   map.size = 2 * (DBase[OBN].Points + 1);
   map.scrnptrsize = 2 * map.size;
   map.map = (short *)DBase[OBN].Elev;
   map.lmap = (long *)get_Memory(map.size * 2, MEMF_CLEAR);
   map.scrnptrx = (float *)get_Memory(map.scrnptrsize, MEMF_ANY);
   map.scrnptry = (float *)get_Memory(map.scrnptrsize, MEMF_ANY);
   map.scrnptrq = (float *)get_Memory(map.scrnptrsize, MEMF_ANY);
   if (! map.map || ! map.lmap || ! map.scrnptrx || ! map.scrnptry ||
		! map.scrnptrq)
    {
    error = 1;
    goto LineCleanup;
    } /* if ! elmap... */
/*   memcpy(map.map, DBase[OBN].Elev, map.size); Talk about tits on a bull */
   map.elscale = ELSCALE_METERS;

   error = maplineobject(&map, win, zbufplot, DBase[OBN].Lat, DBase[OBN].Lon);

LineCleanup:
   if (FreeVec)
    freevecarray(OBN);
   if (map.lmap) free_Memory(map.lmap, map.size * 2);
   if (map.scrnptrx) free_Memory(map.scrnptrx, map.scrnptrsize);
   if (map.scrnptry) free_Memory(map.scrnptry, map.scrnptrsize);
   if (map.scrnptrq) free_Memory(map.scrnptrq, map.scrnptrsize);
   map.map = (short *)NULL;
   map.lmap = (long *)NULL;
   map.scrnptrx = map.scrnptry = (float *)NULL;
   map.scrnptrq = (float *)NULL;
   if (error) break;
   } /* if vector object */
  else if (fMObj)
   fseek(fMObj, (Points + 1) * PointSize, SEEK_CUR);

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  if (BWVC)
   {
   BusyWin_Update(BWVC, OBN + 1);
   } /* if Image Busy Window open */
  }/* for OBN=0 to NoOfObjects */

 if (fMObj)
  fclose(fMObj);
 OBN = OrigOBN;

 if (BWVC)
	BusyWin_Del(BWVC);
 return (error);

} /* InitVectorMap() */


/**********************************************************************/

STATIC_FCN short InitCloudMap(struct Window *win, struct CloudData *CD) // used locally only -> static, AF 26.7.2021
{
 char filename[255], FStr[32];
 short error = 0, PageOutSl = 0, PageOutRf = 0, PageOutFail = 0, PageOutAttempt = 0;
 FILE *fPageOut;

 PageOutSl = SlopeMap      ? 1: 0;
 PageOutRf = ReflectionMap ? 1: 0;

 if (SlopeMap)
  {
  sprintf(FStr, "WCSSlMap%1d.Temp", frame);
  strmfp(filename, temppath, FStr);
  if ((fPageOut = fopen(filename, "wb")))
   {
   if (fwrite((char *)SlopeMap, bmapsize * sizeof (float), 1, fPageOut) == 1)
    {
    free_Memory(SlopeMap, bmapsize * sizeof (float));
    SlopeMap = NULL;
    PageOutSl = 1;
    } /* if write */
   fclose(fPageOut);
   } /* if open */
  } /* if SlopeMap */
 if (ReflectionMap)
  {
  sprintf(FStr, "WCSRfMap%1d.Temp", frame);
  strmfp(filename, temppath, FStr);
  if ((fPageOut = fopen(filename, "wb")))
   {
   if (fwrite((char *)ReflectionMap, bmapsize, 1, fPageOut) == 1)
    {
    free_Memory(ReflectionMap, bmapsize);
    ReflectionMap = NULL;
    PageOutRf = 1;
    } /* if write */
   fclose(fPageOut);
   } /* if open */
  } /* if ReflectionMap */

RepeatAlloc4:
 EdgeSize = 1000;
 Edge1 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
 Edge2 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
 if (! Edge1 || ! Edge2)
  {
  if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
          (CONST_STRPTR)"Out of memory allocating polygon edge buffers!",
          (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
   {
   error = -1;
   goto MapCleanup3;
   }
  error = 1;
  goto MapCleanup3;
  } /* if memory fail */

 if (CD)
  {
RepeatAlloc3:

  CD->Map.scrnptrx = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
  CD->Map.scrnptry = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
  CD->Map.scrnptrq = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
  if (! CD->Map.scrnptrx || ! CD->Map.scrnptry || ! CD->Map.scrnptrq)
   {
   if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
           (CONST_STRPTR)"Out of memory creating Cloud Map!", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
    {
    error = -1;
    goto MapCleanup2;
    } /* if try again */
   error = 1;
   goto MapCleanup2;
   } /* if */

  error = MapCloudObject(&CD->Map, CD, win);

MapCleanup2:
  if (CD->Map.scrnptrx) free_Memory(CD->Map.scrnptrx, CD->Map.scrnptrsize);
  if (CD->Map.scrnptry) free_Memory(CD->Map.scrnptry, CD->Map.scrnptrsize);
  if (CD->Map.scrnptrq) free_Memory(CD->Map.scrnptrq, CD->Map.scrnptrsize);
  CD->Map.scrnptrx = CD->Map.scrnptry = (float *)NULL;
  CD->Map.scrnptrq = (float *)NULL;
  if (error == -1)
   {
   error = 0;
   goto RepeatAlloc3;
   } /* if */
  } /* if cloud shadows */
 else
  {
  if ((CD = CloudData_New()))
   {
   strmfp(filename, cloudpath, cloudfile);
   if (Cloud_Load(filename, &CD))
    {
    if (BuildCloudKeyTable(CD))
     {
RepeatAlloc1:
     CloudWave_Init(CD, frame);
     if (Cloud_Generate(CD,
	(double)(frame * CD->Dynamic) / (double)(settings.fieldrender + 1)))
      {
RepeatAlloc2:
      CD->Map.rows = CD->Cols - 1;
      CD->Map.columns = CD->Rows;
      CD->Map.steplat = (CD->Lat[0] - CD->Lat[1]) / (CD->Rows - 1);
      CD->Map.steplong = (CD->Lon[0] - CD->Lon[1]) / (CD->Cols - 1);
      CD->Map.lolat = CD->Lat[1] + CD->LatOff;
      CD->Map.lolong = CD->Lon[0] + CD->LonOff;
      CD->Map.elscale = ELSCALE_METERS;
      CD->Map.scrnptrsize = CD->PlaneSize;
      CD->Map.size = CD->PlaneSize / 2;	/* sizeof (float) / sizeof (short) */
      CD->Map.map      = (short *)get_Memory (CD->Map.size,        MEMF_ANY);
      CD->Map.scrnptrx = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
      CD->Map.scrnptry = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
      CD->Map.scrnptrq = (float *)get_Memory (CD->Map.scrnptrsize, MEMF_ANY);
      if (! CD->Map.map || ! CD->Map.scrnptrx || ! CD->Map.scrnptry || ! CD->Map.scrnptrq)
       {
       if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
               (CONST_STRPTR)"Out of memory creating Cloud Map!", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
        {
        error = -1;
        goto MapCleanup;
        } /* if try again */
       error = 1;
       goto MapCleanup;
       } /* if */

      error = MapCloudObject(&CD->Map, CD, win);

MapCleanup:
      if (CD->Map.map) free_Memory(CD->Map.map, CD->Map.size);
      if (CD->Map.scrnptrx) free_Memory(CD->Map.scrnptrx, CD->Map.scrnptrsize);
      if (CD->Map.scrnptry) free_Memory(CD->Map.scrnptry, CD->Map.scrnptrsize);
      if (CD->Map.scrnptrq) free_Memory(CD->Map.scrnptrq, CD->Map.scrnptrsize);
      CD->Map.map = (short *)NULL;
      CD->Map.scrnptrx = CD->Map.scrnptry = (float *)NULL;
      CD->Map.scrnptrq = (float *)NULL;
      if (error == -1)
       {
       error = 0;
       goto RepeatAlloc2;
       } /* if */
      } /* if Cloud Map generated OK */
     else
      {
      if (User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
              (CONST_STRPTR)"Error creating Cloud Map! Either out of memory or user aborted.", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
       {
       goto RepeatAlloc1;
       } /* if try again */
      else
       error = 1;
      } /* else */
     }
    else
     {
     User_Message_Def((CONST_STRPTR)"Render Module: Clouds",
             (CONST_STRPTR)"Out of memory allocating Cloud Key Frames!\nOperation terminated", (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
     error = 1;
     } /* else */
    } /* if Cloud Data read */
  
   CloudData_Del(CD);
   } /* if Cloud Data allocated */
  } /* if need to create new cloud map */

MapCleanup3:
 if (Edge1)
  free_Memory(Edge1, EdgeSize);
 if (Edge2)
  free_Memory(Edge2, EdgeSize);
 Edge1 = Edge2 = NULL;
 if (error == -1)
  {
  error = 0;
  goto RepeatAlloc4;
  } /* if */

 if (PageOutSl)
  {
  sprintf(FStr, "WCSSlMap%1d.Temp", frame);
  strmfp(filename, temppath, FStr);
  if (! error)
   {
   if ((fPageOut = fopen(filename, "rb")))
    {
TryAgain:
    if ((SlopeMap = (float *)get_Memory(bmapsize * sizeof (float), MEMF_ANY)))
     {
     if (fread((char *)SlopeMap, bmapsize * sizeof (float), 1, fPageOut) != 1)
      {
      free_Memory(SlopeMap, bmapsize * sizeof (float));
      SlopeMap = NULL;
      PageOutFail = 1;
      } /* if write */
     }
    else
     {
     if (! PageOutAttempt)
      {
      PageOutAttempt = 1;
      goto TryAgain;
      } /* if */
     PageOutFail = 2;
     } /* else no memory */
    fclose(fPageOut);
    } /* if open */
   else
    PageOutFail = 3;
   } /* if */
  remove(filename);
  } /* if SlopeMap paged out */

 if (PageOutRf)
  {
  PageOutAttempt = 0;
  sprintf(FStr, "WCSRfMap%1d.Temp", frame);
  strmfp(filename, temppath, FStr);
  if (! PageOutFail && ! error)
   {
   if ((fPageOut = fopen(filename, "rb")))
    {
TryAgain2:
    if ((ReflectionMap = (UBYTE *)get_Memory(bmapsize, MEMF_ANY)))
     {
     if (fread((char *)ReflectionMap, bmapsize, 1, fPageOut) != 1)
      {
      free_Memory(ReflectionMap, bmapsize);
      ReflectionMap = NULL;
      PageOutFail = 1;
      } /* if write */
     }
    else
     {
     if (! PageOutAttempt)
      {
      PageOutAttempt = 1;
      goto TryAgain2;
      } /* if */
     PageOutFail = 2;
     } /* else no memory */
    fclose(fPageOut);
    } /* if open */
   else
    PageOutFail = 3;
   } /* if */
  remove(filename);
  } /* if ReflectionMap paged out */

 if (PageOutFail)
  {
  char ErrStr[120];

  error = 1;
  switch (PageOutFail)
   {
   case 1:
    {
    strcpy(ErrStr, 
 "Error reading paged-out file! Can't restore Reflection buffers. Operation terminated.");
    break;
    }
   case 2:
    {
    strcpy(ErrStr,
 "Error allocating memory for paged-out file! Can't restore Reflection buffers. Operation terminated.");
    break;
    }
   case 3:
    {
    strcpy(ErrStr,
 "Error opening paged-out file! Can't restore Reflection buffers. Operation terminated.");
    break;
    }
   } /* switch */
  User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)ErrStr, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  } /* if */

 return (error);

} /* InitCloudMap() */

/*************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void Handle_Render_Window(void)
{
 short abort = 0;
 struct clipbounds cb;

 setclipbounds(RenderWind0, &cb);

 while (QuickFetchEvent(RenderWind0, &Event))
  {
  switch (Event.Class)
   {
   case CLOSEWINDOW:
    {
    Close_Render_Window();
    abort = 1;
    break;
    } /* CLOSEWINDOW */

   case ACTIVEWINDOW:
    {
    LoadRGB4(&WCSScrn->ViewPort, &Colors[0], 16);
    break;
    } /* ACTIVEWINDOW */

   case MOUSEBUTTONS:
    {
    if (DIAG_Win)
     {
      switch (Event.Code)
       {
       case SELECTDOWN:
        {
        if (Event.MouseX >= cb.lowx &&
		Event.MouseX <= cb.highx &&
		Event.MouseY >= cb.lowy &&
		Event.MouseY <= cb.highy)
         {
         Set_Diagnostic_Point((Event.MouseY - drawoffsetY)
		* settings.scrnwidth + Event.MouseX - drawoffsetX);
         ModifyIDCMP(RenderWind0, MOUSEMOVE | MOUSEBUTTONS);
         } /* if in rendered bounds */
        break;
        } /* SELECTDOWN */
       case SELECTUP:
        {
        ModifyIDCMP(RenderWind0, CLOSEWINDOW | ACTIVEWINDOW | MOUSEBUTTONS);
        break;
	} /* SELECTUP */
       } /* switch Event.Code */
     } /* if diagnostic window open */
    break;
    } /* MOUSEBUTTONS */

   case MOUSEMOVE:
    {
    if (Event.MouseX >= cb.lowx &&
		Event.MouseX <= cb.highx &&
		Event.MouseY >= cb.lowy &&
		Event.MouseY <= cb.highy)
     {
     Set_Diagnostic_Point((Event.MouseY - drawoffsetY)
		* settings.scrnwidth + Event.MouseX - drawoffsetX);
     } /* if in rendered bounds */
    break;
    } /* MOUSEMOVE */

   } /* switch Event.Class */

  if (abort) break;

  } /* while */

} /* Handle_Render_Window() */
#endif
/*************************************************************************/

STATIC_FCN void Close_Render_Window(void) // used locally only -> static, AF 20.7.2021
{

 if (RenderWind0)
  {
  WCS_Signals ^= RenderWind0_Sig;
  closesharedwindow(RenderWind0, 0);
  RenderWind0 = NULL;
  RenderWind0_Sig = 0;
  } /* if window opened */

} /* Close_Render_Window() */

/************************************************************************/

STATIC_FCN short BuildTrigTables(void) // used locally only -> static, AF 26.7.2021
{
short success = 1, i;
double Val, Interval;

 if ((CosTable = (float *)
	get_Memory(TrigTableEntries * sizeof (float), MEMF_ANY)))
  {
  Interval = TwoPi / (TrigTableEntries - 1);
  for (i=0, Val=0.0; i<TrigTableEntries; i++, Val+=Interval)
   {
   CosTable[i] = cos(Val);
   } /* for i=0... */
  if ((SinTable = (float *)
	get_Memory(TrigTableEntries * sizeof (float), MEMF_ANY)))
   {
   Interval = (2.0 * TwoPi) / (TrigTableEntries - 1);
   for (i=0, Val=-TwoPi; i<TrigTableEntries; i++, Val+=Interval)
    {
    SinTable[i] = sin(Val);
    } /* for i=0... */
   if ((ASinTable = (float *)
	get_Memory(TrigTableEntries * sizeof (float), MEMF_ANY)))
    {
    Interval = 2.0 / (TrigTableEntries - 1);
    for (i=0, Val=-1.0; i<TrigTableEntries; i++, Val+=Interval)
     {
     ASinTable[i] = asin(Val);
     } /* for i=0... */
    if ((ACosTable = (float *)
	get_Memory(TrigTableEntries * sizeof (float), MEMF_ANY)))
     {
     Interval = 2.0 / (TrigTableEntries - 1);
     for (i=0, Val=-1.0; i<TrigTableEntries; i++, Val+=Interval)
      {
      ACosTable[i] = acos(Val);
      } /* for i=0... */
     } /* if */
    else
     success = 0;
    } /* if */
   else
    success = 0;
   }
  else
   success = 0;
  }
 else
  success = 0;

 return (success);

} /* BuildTrigTables() */

/************************************************************************/

STATIC_FCN void FreeTrigTables(void) // used locally only -> static, AF 26.7.2021
{

 if (CosTable)
  free_Memory(CosTable, TrigTableEntries * sizeof (float));
 if (SinTable)
  free_Memory(SinTable, TrigTableEntries * sizeof (float));
 if (ASinTable)
  free_Memory(ASinTable, TrigTableEntries * sizeof (float));
 if (ACosTable)
  free_Memory(ACosTable, TrigTableEntries * sizeof (float));
 CosTable = SinTable = ACosTable = ASinTable = NULL;

} /* FreeTrigTables() */

/************************************************************************/

double ASin_Table(double sine)
{
long Entry;

 if (fabs(sine) > 1.0)
  return (0.0);

 if (ASinTable)
  {
  sine += 1.0;
  sine /= 2.0;
  sine *= (TrigTableEntries - 1);
  Entry = sine;

  return ((double)ASinTable[Entry]);
  } /* if */

 return (asin(sine));
 
} /* ASin_Table() */

/************************************************************************/

double ACos_Table(double cosine)
{
long Entry;

 if (fabs(cosine) > 1.0)
  return (0.0);

 if (ACosTable)
  {
  cosine += 1.0;
  cosine /= 2.0;
  cosine *= (TrigTableEntries - 1);
  Entry = cosine;

  return ((double)ACosTable[Entry]);
  } /* if */

 return (acos(cosine));
 
} /* ACos_Table() */

/************************************************************************/

double Sin_Table(double arcsine)
{
long Entry;

 if (fabs(arcsine) > TwoPi)
  return (0.0);

 if (SinTable)
  {
  arcsine += TwoPi;
  arcsine /= (2.0 * TwoPi);
  arcsine *= (TrigTableEntries - 1);
  Entry = arcsine;

  return ((double)SinTable[Entry]);
  } /* if */

 return (sin(arcsine));
 
} /* Sin_Table() */

/************************************************************************/

double Cos_Table(double arccosine)
{
long Entry;

 if (CosTable)
  {
  arccosine = fabs(arccosine);
  while (arccosine > TwoPi)
   arccosine -= TwoPi;

  arccosine /= TwoPi;
  arccosine *= (TrigTableEntries - 1);
  Entry = arccosine;

  return ((double)CosTable[Entry]);
  } /* if */

 return (cos(arccosine));
 
} /* Cos_Table() */
