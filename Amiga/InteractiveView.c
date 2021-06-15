/* InteractiveView.c (ne gisinteractiveview.c 14 Jan 1994 CXH)
** Allows interactive modification of view parameters associated
** with rendering a scene in GIS. Creates a grid sampled at a specified
** density from the original DEM files and draws it using the
** parameter settings currently in force or as modified within this module.
** Original code written by Gary R. Huber, August 1, 1993, modified,
** appended and generally improved by Chris "Xenon" Hanson 8/93.
** Modified for use with MUI by Gary Huber, 12/93.
*/

#include "WCS.h"


short interactiveview(short new_window)
{
 short error, try = 0;

 if (InterWind0 && new_window)
  {
  WindowToFront(InterWind0);
  if (EMIA_Win) WindowToFront(EMIA_Win->Win);
  if (EMPL_Win) DoMethod(EMPL_Win->ParListWin, MUIM_Window_ToFront);
  return (0);
  } /* if interactive already open */

 if (! paramsloaded)
  {
  User_Message("Parameters Module: Camera View",
	"You must first load a complete Parameter file!", "OK", "o");
  Log(ERR_NO_LOAD, "Complete parameter file");
  return(0);
  } /* if no parameter file loaded */
 if (NoOfObjects == 0)
  {
  User_Message("Parameters Module: Camera View",
	"There are no objects in this Database!\nOperation terminated", "OK", "o");
  Log(ERR_NO_LOAD, "No objects in Database");
  return(0);
  } /* if no parameter file loaded */

 if (new_window)
  {
  NoOfSmWindows = 0;

 if ((IA = (struct InterAction *)
	get_Memory(sizeof (struct InterAction), MEMF_CLEAR)) == NULL)
  {
  goto Cleanup;
  } /* if out of memory */

 if (IA_GridSize <= 0) IA_GridSize = settings.gridsize;

  error = openinterview();
  if (error)
   {
   User_Message("Editing Module: Interactive",
	"Camera View failed to open!\nOperation terminated.", "OK", "o");
   Log(ERR_WIN_FAIL, "Camera View");
   goto Cleanup;
   } /* if error opening window */
  } /* if open new window */

 viewctr = (struct ViewCenter *)get_Memory(sizeof (struct ViewCenter),
		MEMF_ANY);
 FocProf = (struct FocusProfiles *)get_Memory(sizeof (struct FocusProfiles),
		MEMF_CLEAR);
 AltRenderListSize = NoOfObjects * 4;
 AltRenderList = (short *)get_Memory(AltRenderListSize, MEMF_CLEAR);

 if (! viewctr || ! FocProf || ! AltRenderList)
  {
  User_Message("Parameters Module: Camera View",
	"Out of memory opening Camera View!\nOperation terminated.", "OK", "o");
  goto Cleanup;
  } /* if no view center */

 SetPointer(InterWind0, WaitPointer, 16, 16, -6, 0);
 if (EMIA_Win) SetPointer(EMIA_Win->Win, WaitPointer, 16, 16, -6, 0);

RepeatOpen:
 error = initinterview(0);
 if (error)
  {
  if (error == 1)
   {
   if (User_Message_Def("Parameters Module: Camera View",
	"Out of memory loading DEMs!\nIncrease grid size?", "OK|Cancel", "oc", 1))
    {
    closeviewmaps(0);
    IA_GridSize *=2;
    try ++;
    goto RepeatOpen;
    } /* if increase grid size */
   } /* if out of memory */
  else if (error == 2) User_Message("Parameters Module: Camera View",
	"No DEM objects active!\nOperation terminated.", "OK", "o");
  ClearPointer(InterWind0);
  if (EMIA_Win) ClearPointer(EMIA_Win->Win);
  goto Cleanup;
  } /* if error loading maps */

/* findfocprof();*/

 IA->recompute = 1;
 Init_IA_View(1);

 setcompass(0);

 ClearPointer(InterWind0);
 if (EMIA_Win)
  {
  ClearPointer(EMIA_Win->Win);
  if (try > 0)
   {
   set(EMIA_Win->Str[0], MUIA_String_Integer, IA_GridSize);
   EMIA_Win->GridStrBlock = 1;
   } /* if more than one attempt to load maps */
  } /* if EMIA window open */
 if (EMTL_Win) set(EMTL_Win->BT_Play, MUIA_Disabled, FALSE);
 SetWindowTitles(InterWind0, varname[EM_Win->MoItem], (UBYTE *)-1);
 return (1);

Cleanup:
 closeviewmaps(1);
 closeinterview();
 return (0);

} /* interactiveview() */

/************************************************************************/

short openinterview(void)
{
 short error, try = 0;
 ULONG flags, iflags;
 UBYTE c0,c1;
 float hw_ratio, hw_scrn;

#ifdef AMIGA_GUI

RepeatOpen:

 if (! IA_Width || ! IA_Height)
  {
  hw_scrn = (float)WCSScrn->Height / WCSScrn->Width;
  hw_ratio = (float)settings.scrnheight / settings.scrnwidth;
  if (hw_scrn < hw_ratio)
   {
   
   IA_Height = WCSScrn->Height / 2;
   IA_Width = IA_Height / hw_ratio;
   } /* tall image (landscape) */
  else
   {
   IA_Width = WCSScrn->Width / 2;
   IA_Height = IA_Width * hw_ratio;
   } /* wide image (portrait) */
  IA_Top = (WCSScrn->Height - IA_Height) / 2;
  IA_Left = (WCSScrn->Width - IA_Width) / 2;
  } /* if no width or height previously defined */

 c0 = 0x00;
 c1 = 0x01; 
 flags = ACTIVATE | SMART_REFRESH | WINDOWSIZING | WINDOWDRAG | WINDOWCLOSE |
           WINDOWDEPTH | REPORTMOUSE | RMBTRAP | WINDOWREFRESH ;
 iflags = MOUSEBUTTONS | RAWKEY | VANILLAKEY | NEWSIZE | CLOSEWINDOW | ACTIVEWINDOW;

 InterWind0 = (struct Window *)
	make_window(IA_Left, IA_Top, IA_Width, IA_Height,
	"Camera View", flags, NULL, c0, c1, WCSScrn);

 if (! InterWind0)
  {
  if (try > 0)
   {
   error = 1;
   goto EndIt;
   } /* if second attempt failed */
  IA_Width = IA_Height = 0;
  try = 1;
  goto RepeatOpen;
  } /* if no window */
 WindowLimits(InterWind0, 200, 80, -1, -1);

 ModifyIDCMP(InterWind0, iflags);

 InterWind0_Sig = 1L << InterWind0->UserPort->mp_SigBit;
 WCS_Signals |= InterWind0_Sig;

 drawoffsetX = InterWind0->BorderLeft;
 drawoffsetY = InterWind0->BorderTop;
#endif

 error = make_compass();

 setclipbounds(InterWind0, &IA->cb);

EndIt:
 return (error);

} /* openinterview() */

/************************************************************************/

short initinterview(short boundsdiscrim)
{
 char filename[255];
 short newrows, newcolumns, i = 0, error = 0, j, q, a, b, c, d,
	 OrigOBN, OpenOK, Marked = 0;
 long rowzip, Lr, Lc, AvgEl = 0;
 short *mapptr;
 struct DirList *DLItem;
 struct elmapheaderV101 TempHdr;

 OrigOBN = OBN;

 for (OBN=0; OBN<NoOfObjects; OBN++)
  {
  if ((! strcmp(DBase[OBN].Special, "TOP") ||
          ! strcmp(DBase[OBN].Special, "SFC")) && (DBase[OBN].Flags & 2))
   Marked ++;
  } /* for OBN=0... */

 if (Marked <= 0)
  {
  error = 2;
  goto EndLoad;
  } /* if no DEMs enabled */
 ElmapSize = Marked * sizeof (struct elmapheaderV101);
 BBoxSize = Marked * sizeof (struct boundingbox);

 if (((elmap = (struct elmapheaderV101 *)get_Memory(ElmapSize, MEMF_CLEAR))
	 == NULL) ||
 	((BBox = (struct boundingbox *)get_Memory(BBoxSize, MEMF_CLEAR))
	 == NULL))
  {
  error = 1;
  goto EndLoad;
  } /* if out of memory */

 for (OBN=0; OBN<NoOfObjects; OBN++)
  {
  if ((! strcmp(DBase[OBN].Special, "TOP") ||
          ! strcmp(DBase[OBN].Special, "SFC")) && (DBase[OBN].Flags & 2))
   {
   OpenOK = 0;

   DLItem = DL;
   while (DLItem)
    {
    strmfp(filename, DLItem->Name, DBase[OBN].Name);
    strcat(filename, ".elev");
    if (readDEM(filename, &TempHdr) != 0)
     {
     DLItem = DLItem->Next;
     } /* if file not found */
    else
     {
     OpenOK = 1;
     break;
     } /* else file found */
    } /* while */
 
   if (! OpenOK)
    {
    Log(WNG_OPEN_FAIL, DBase[OBN].Name);
    continue;
    } /* if */

   memcpy(&elmap[i], &TempHdr, ELEVHDRLENV101);

   if (elmap[i].MaxEl == elmap[i].MinEl && elmap[i].MaxEl == 0)
    {
    elmap[i].MaxEl = -30000;
    elmap[i].MinEl = 30000;
		
    mapptr = TempHdr.map;
    for (Lr=0; Lr<=elmap[i].rows; Lr++)
     {
     for (Lc=0; Lc<elmap[i].columns; Lc++)
      {
      if (*mapptr > elmap[i].MaxEl)
       {
       elmap[i].MaxEl = *mapptr;
       } /* if */
      else if (*mapptr < elmap[i].MinEl)
       {
       elmap[i].MinEl = *mapptr;
       } /* else if */
      mapptr ++;
      } /* for Lc=0... */
     } /* for Lr=0... */
    } /* if version 1.0- file */

   BBox[i].elev[8] = *(TempHdr.map + elmap[i].rows * elmap[i].columns);
   BBox[i].elev[9] = *(TempHdr.map);
   BBox[i].elev[10] = *(TempHdr.map + elmap[i].columns - 1);
   BBox[i].elev[11] = *(TempHdr.map + elmap[i].columns - 1 +
		elmap[i].rows * elmap[i].columns);
   BBox[i].elev[12] = *(TempHdr.map + (elmap[i].columns - 1) / 2 +
		(elmap[i].rows / 2) * elmap[i].columns);

   BBox[i].lat[0] = BBox[i].lat[1] = BBox[i].lat[4] = BBox[i].lat[5] = 
	   	BBox[i].lat[8] = BBox[i].lat[9] = elmap[i].lolat;
   BBox[i].lat[2] = BBox[i].lat[3]=BBox[i].lat[6]=BBox[i].lat[7] =
	   	BBox[i].lat[10] = BBox[i].lat[11] =
	        elmap[i].lolat + elmap[i].steplat * (elmap[i].columns - 1);
   BBox[i].lat[12] = (BBox[i].lat[0] + BBox[i].lat[2]) / 2.0;
	
   BBox[i].lon[1] = BBox[i].lon[2]=BBox[i].lon[5]=BBox[i].lon[6] =
	   	BBox[i].lon[9] = BBox[i].lon[10] = elmap[i].lolong;
   BBox[i].lon[0] = BBox[i].lon[3]=BBox[i].lon[4]=BBox[i].lon[7] =
	   	BBox[i].lon[8] = BBox[i].lon[11] =
	        elmap[i].lolong - elmap[i].steplong * elmap[i].rows;
   BBox[i].lon[12] = (BBox[i].lon[0] + BBox[i].lon[1]) / 2.0;
	
   BBox[i].elev[0] = BBox[i].elev[1]=BBox[i].elev[2]=BBox[i].elev[3] =
		elmap[i].MinEl;
   BBox[i].elev[4] = BBox[i].elev[5]=BBox[i].elev[6]=BBox[i].elev[7] =
		elmap[i].MaxEl;

   if (boundsdiscrim)
    {
    computesinglebox(i);    
    for (j=q=0; j<13; j++)
     {
     if (j == 8)
      j = 12;
     if (BBox[i].scrnq[j] < 0.01 || (! settings.horizonmax && BBox[i].scrnq[j] > qmax))
      {
      q ++;
      } /* if behind viewer or beyond horizon */
     } /* for j=... */

    if (q < 9)
     {
     for (j=a=b=c=d=0; j<8; j++)
      {
      if (BBox[i].scrnx[j] < 0) a++;
      else if (BBox[i].scrnx[j] >= settings.scrnwidth) b++;
      if (BBox[i].scrny[j] < 0) c++;
      else if (BBox[i].scrny[j] >= settings.scrnheight) d++;
      } /* for j=... */
     } /* if not all corners are behind viewer */
    if (q == 9 || a == 8 || b == 8 || c == 8 || d == 8)
     {
     free_Memory(TempHdr.map, TempHdr.size);
     TempHdr.map = NULL;
     continue;
     }
    RenderList[i][0] = OBN;
    RenderList[i][1] = 0;
    } /* if boundsdiscrim */
   else
    {
    *(AltRenderList + i * 2) = OBN;
    *(AltRenderList + i * 2 + 1) = 0;
/* used to be an end bracket here, now it is below after "MapAsSFC = 1" */

   newrows = elmap[i].rows / IA_GridSize;
   newcolumns = 1 + (elmap[i].columns - 1) / IA_GridSize;

   elmap[i].size = (newrows + 1) * newcolumns * 2;
   elmap[i].scrnptrsize = elmap[i].size * 2;
   elmap[i].map = (short *)get_Memory (elmap[i].size, MEMF_ANY);
   elmap[i].scrnptrx = (float *)get_Memory (elmap[i].scrnptrsize, MEMF_ANY);
   elmap[i].scrnptry = (float *)get_Memory (elmap[i].scrnptrsize, MEMF_ANY);
   elmap[i].scrnptrq = (float *)get_Memory (elmap[i].scrnptrsize, MEMF_ANY);

   if (! elmap[i].map || ! elmap[i].scrnptrx || ! elmap[i].scrnptry
	|| ! elmap[i].scrnptrq)
    {
    if (elmap[i].map) free_Memory (elmap[i].map, elmap[i].size);
    if (elmap[i].scrnptrx) free_Memory (elmap[i].scrnptrx, elmap[i].scrnptrsize);
    if (elmap[i].scrnptry) free_Memory (elmap[i].scrnptry, elmap[i].scrnptrsize);
    if (elmap[i].scrnptrq) free_Memory (elmap[i].scrnptrq, elmap[i].scrnptrsize);
    if (TempHdr.map)       free_Memory (TempHdr.map, TempHdr.size);
    error = 1;
    break;
    } /* if */

   mapptr = (short *)elmap[i].map;

   for (Lr=0; Lr<=elmap[i].rows; Lr +=IA_GridSize)
    {
    rowzip = Lr * elmap[i].columns;
    for (Lc=0; Lc<elmap[i].columns; Lc +=IA_GridSize)
     {
     *mapptr = *(TempHdr.map + rowzip + Lc);
     mapptr++;
     } /* for Lc=0... */
    } /* for Lr=0... */
	
   elmap[i].rows = newrows;
   elmap[i].columns = newcolumns;
   elmap[i].steplat *= IA_GridSize;
   elmap[i].steplong *= IA_GridSize;
   elmap[i].MapAsSFC = 1;
    } /* else */

   if (TempHdr.map)
    free_Memory( TempHdr.map, TempHdr.size);
   TempHdr.map = NULL;

   AvgEl += ((elmap[i].MaxEl + elmap[i].MinEl) / 2);

   i ++;
   } /* if type = TOP or SFC */
  } /* for OBN=0... */

 NoOfElMaps=i;

 if (! boundsdiscrim)
  {
  if (NoOfElMaps > 0)
   AvgEl /= NoOfElMaps;

  for (i=0; i<NoOfElMaps;i++)
   {
   BBox[i].elev[8] = BBox[i].elev[9]
	= BBox[i].elev[10] = BBox[i].elev[11] = AvgEl;
   } /* for i=0... */
  } /* if from interactive view */

 sprintf(str, "Elevation maps loaded = %d.\n",NoOfElMaps);
 Log(MSG_NULL, str);

EndLoad:
 OBN = OrigOBN;
 if (error == 1)
  return (1);
 if (NoOfElMaps==0)
  return (2);

 return (error);

} /* initinterview() */

/************************************************************************/

void closeviewmaps(short CloseAll)
{
 short i;

 if (elmap)
  {
  for (i=0; i<NoOfElMaps; i++)
   {
   if (elmap[i].map) free_Memory(elmap[i].map, elmap[i].size);
   if (elmap[i].lmap) free_Memory(elmap[i].lmap, elmap[i].size * 2);
   if (elmap[i].scrnptrx) free_Memory(elmap[i].scrnptrx, elmap[i].scrnptrsize);
   if (elmap[i].scrnptry) free_Memory(elmap[i].scrnptry, elmap[i].scrnptrsize);
   if (elmap[i].scrnptrq) free_Memory(elmap[i].scrnptrq, elmap[i].scrnptrsize);
   } /* for i=0... */
  free_Memory(elmap, ElmapSize);
  elmap = NULL;
  } /* if elmap */
 if (BBox) free_Memory(BBox, BBoxSize);
 BBox = NULL;
 NoOfElMaps = 0;
 
 if (CloseAll)
  {
  if (viewctr) free_Memory(viewctr, sizeof (struct ViewCenter));
  viewctr = NULL;
  if (FocProf) free_Memory(FocProf, sizeof (struct FocusProfiles));
  FocProf = NULL;
  if (AltRenderList) free_Memory(AltRenderList, AltRenderListSize);
  AltRenderList = NULL;
  } /* if close all */

} /* closeviewmaps() */

/************************************************************************/

void closeinterview(void)
{
 short i;

 if (EMTL_Win) set(EMTL_Win->BT_Play, MUIA_Disabled, TRUE);
 if (IA->Digitizing) QuitDigPerspective();
 for (i=NoOfSmWindows-1; i>=0; i--)
  {
/* Important Note: this must be done from high to low since the windows are
	shuffled to the next lowest when one below is closed */
  Close_Small_Window(i);
  } /* for i=0... */
 if (InterWind2) {
  IA_CompTop = InterWind2->TopEdge;
  IA_CompLeft = InterWind2->LeftEdge;
  IA_CompWidth = InterWind2->Width;
  IA_CompHeight = InterWind2->Height;
  WCS_Signals ^= InterWind2_Sig;
  closesharedwindow(InterWind2, 0);
  InterWind2 = NULL;
  InterWind2_Sig = NULL;
 } /* if */
 if (InterWind0) {
  IA_Top = InterWind0->TopEdge;
  IA_Left = InterWind0->LeftEdge;
  IA_Width = InterWind0->Width;
  IA_Height = InterWind0->Height;
  WCS_Signals ^= InterWind0_Sig;
  closesharedwindow(InterWind0, 0);
  InterWind0 = NULL;
  InterWind0_Sig = NULL;
 } /* if */
 if (IA) free_Memory(IA, sizeof (struct InterAction));
 IA = NULL; 
 LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);

} /* closeinterview() */

/************************************************************************/

void shaderelief(short reliefshade)
{
 short error = 0, i, j, altmapassfc, altgridsize, altreliefshade, altfixfract,
	origOBN;
 long NumPoints;
 double altlineoffset;
 struct BusyWindow *BWIM = NULL;

 origOBN = OBN;
 render = 0x10;
 altreliefshade = settings.reliefshade;
 altmapassfc = settings.mapassfc;
 altlineoffset = settings.lineoffset;
 altgridsize = settings.gridsize;
 settings.gridsize = 1;
 settings.reliefshade = reliefshade;
 altfixfract = settings.fixfract;
 zbufsize = (IA->scrnwidth + 1) * (IA->scrnheight + 1) * 4;
 if ((zbuf = (float *)get_Memory(zbufsize, MEMF_ANY)) == NULL)
  {
  User_Message("Camera View",
	"Out of memory opening Z buffer!\nOperation terminated.", "OK", "o");
  return;
  } /* if */
 if ((bytemap = (USHORT *)get_Memory(zbufsize / 2, MEMF_CLEAR)) == NULL)
  {
  User_Message("Camera View",
	"Out of memory opening Antialias buffer!\nOperation terminated.", "OK", "o");
  goto Cleanup;
  } /* if */

 SetPointer(InterWind0, WaitPointer, 16, 16, -6, 0);
 if (EMIA_Win) SetPointer(EMIA_Win->Win, WaitPointer, 16, 16, -6, 0);
 if (NoOfElMaps > 1)
  {
  BWIM = BusyWin_New("Image", NoOfElMaps, 1, 'BWIM');
  } /* if more than one DEM */
 
 
 for (i=0; i<=IA->scrnheight; i++)
  {
  for (j=0; j<=IA->scrnwidth; j++)
   {
   *(zbuf + i * (IA->scrnwidth + 1) + j) = FLT_MAX;
   } /* for j=0... */
  } /* for i=0... */

 for (i=0; i<=IA->scrnheight; i++)
	scrnrowzip[i] = i * (IA->scrnwidth + 1);
 initpar();
 constructview();
 setvalues();
 oshigh = high = IA->scrnheight + 1;
 wide = IA->scrnwidth;

 for (j=0, i=NoOfElMaps-1; i>=0; i--, j++)
  {
  settings.lineoffset = elmap[i].steplat * LATSCALE;
  elmap[i].MapAsSFC = 1;

RepeatAllocate:
  if ((elmap[i].lmap = (long *)get_Memory(elmap[i].size * 2, MEMF_ANY)) == NULL)
   {
   if (User_Message_Def("Camera View",
	"Out of memory allocating DEM array!\n", "Retry|Cancel", "rc", 1))
    {
    goto RepeatAllocate;
    } /* if out of memory, try again */
   error = 1;
   break;
   } /* else no memory for long array */
  if (settings.smoothfaces ||
	 ! strcmp(DBase[*(AltRenderList + i * 2)].Special, "SFC") || settings.mapassfc)
   {
   NumPoints = 6 * elmap[i].columns;
   if ((elmap[i].face = (struct faces *)get_Memory(NumPoints * sizeof (struct faces) , MEMF_ANY)) == NULL)
    {
    if (User_Message("Camera View",
	"Out of memory allocating Polygon Smoothing array!\n\
Continue without Polygon Smoothing?", "OK|Cancel", "oc"))
     {
     settings.smoothfaces = 0;
     } /* if out of memory, try again */
    else
     {
     error = 1;
     break;
     } /* else */
    } /* else no memory for long array */
   if (! strcmp(DBase[*(AltRenderList + i * 2)].Special, "SFC") || settings.mapassfc)
    elmap[i].ForceBath = 1;
   else
    elmap[i].ForceBath = 0;
   } /* if smooth faces */
  else
   {
   elmap[i].face = NULL;
   elmap[i].ForceBath = 0;
   } /* else */

  if (settings.fractalmap || (! settings.fixfract && settings.displace))
   {
   elmap[i].fractalsize = elmap[i].rows * (elmap[i].columns - 1) * 2;
   if ((elmap[i].fractal =
	 (BYTE *)get_Memory(elmap[i].fractalsize, MEMF_CLEAR)) != NULL)
    {
    settings.fixfract = 0;
    } /* if no memory for fractal map */
   } /* if */
  else
   elmap[i].fractal = NULL;

  SubPixArraySize = 10000;
  EdgeSize = 1000;
  SubPix = get_Memory(SubPixArraySize, MEMF_CLEAR);
  Edge1 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  Edge2 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  if (! Edge1 || ! Edge2 || ! SubPix)
   {
   User_Message("Render Module",
	"Out of memory allocating antialias and edge buffers!\nOperation terminated.", "OK", "o");
   error = 1;
   break;
   } /* if memory fail */
 
  error = maptopoobject(&elmap[i], InterWind0, j + 1, NoOfElMaps, NULL);

  if (elmap[i].lmap) free_Memory(elmap[i].lmap, elmap[i].size * 2);
  elmap[i].lmap = NULL;
  if (elmap[i].fractal) free_Memory(elmap[i].fractal, elmap[i].fractalsize);
  elmap[i].fractal = NULL;
  if (settings.smoothfaces)
   {
   if (elmap[i].face)
    free_Memory(elmap[i].face, NumPoints * sizeof (struct faces));
   elmap[i].face = NULL;
   } /* if */
  if (SubPix)
   free_Memory(SubPix, SubPixArraySize);
  SubPix = NULL;
  if (Edge1) free_Memory(Edge1, EdgeSize);
  if (Edge2) free_Memory(Edge2, EdgeSize);
  Edge1 = Edge2 = NULL;

  if (error) break;
  if (BWIM)
   {
   BusyWin_Update(BWIM, j + 1);
   } /* if Image Busy Window open */

  } /* for i=0... */

/* Map vector objects */
 if (! error)
  {
  InitVectorMap(InterWind0, 1, 0);
  } /* if not abort render */

Cleanup:

 if (zbuf) free_Memory(zbuf, zbufsize);
 zbuf = (float *)NULL;
 if (bytemap) free_Memory(bytemap, zbufsize / 2);
 bytemap = NULL;
 render = 0;
 IA->recompute = 1;
 settings.mapassfc = altmapassfc;
 settings.lineoffset = altlineoffset;
 settings.gridsize = altgridsize;
 settings.reliefshade = altreliefshade;
 settings.fixfract = altfixfract;
 OBN = origOBN;
 IA->gridpresent = 1;

 if (BWIM)
	BusyWin_Del(BWIM);
 ClearPointer(InterWind0);
 if (EMIA_Win) ClearPointer(EMIA_Win->Win);

} /* shaderelief() */

/************************************************************************/

void smallwindow(short diagnostics)
{
 char filename[255], elevpath[256], elevfile[32];
 short i, j, done = 0, abort = 0, OpenOK, firstX = -1000, firstY, lastX, lastY,
	nextX, nextY, lowrow, lowcol, highrow, highcol, lowmap, newrows,
	newcolumns, rowsize, scrnwidth, altdrawoffsetX,
	altdrawoffsetY, scrnheight, altclouds, altfixfract,
	MapOBN, altCenterX, altCenterY, map[4] = {-1,-1,-1,-1},
	UseMaxRows, UseMaxCols;
 short *temparrayptr, maprowsize, maprowbytes;
 short *mapptr;
 long NumPoints, Lr, Lc;
 float lowq;
 float *scrnx, *scrny, *scrnq;
 struct lineseg ls;
 struct IntuiMessage Local;
 UBYTE c0,c1;
 ULONG flags, iflags;
 struct DirList *DLItem;
 struct elmapheaderV101 TempHdr;
 ULONG IDCMPFlags;
 struct Window *ReplyWin;

 IDCMPFlags = EMIA_Win->Win->IDCMPFlags;

 if (! IA->gridpresent)
  {
  User_Message("Camera View",
	"Grid must be present, please redraw and try again.", "OK", "o");
  return;
  } /* if no grid present */

 if (DIAG_Win) Close_Diagnostic_Window();

 ModifyIDCMP(EMIA_Win->Win, IDCMPFlags | VANILLAKEY);
 ModifyIDCMP(InterWind0, MOUSEBUTTONS | INTUITICKS | VANILLAKEY);

 SetDrMd(InterWind0->RPort, COMPLEMENT);
 SetWindowTitles(InterWind0, "Select preview region with two clicks", (UBYTE *)-1);

 while (! done)
  {
  ReplyWin = FetchMultiWindowEvent(&Local, InterWind0, EMIA_Win->Win, NULL);
  switch (Local.Class)
   {
   case VANILLAKEY:
    {
    if (Local.Code == ESC)
     {
     done = 1;
     abort = 1;
     } /* if escape */
    break;
    } /* VANILLAKEY */
   case MOUSEBUTTONS:
    {
    if (ReplyWin != InterWind0)
     break;
    if (Local.Code == SELECTUP)
     {
     if (firstX < -999)
      {
      firstX = Local.MouseX;
      firstY = Local.MouseY;
      lastX = Local.MouseX;
      lastY = Local.MouseY;

      setlineseg(&ls, (double)firstX, (double)firstY, (double)lastX, (double)firstY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)lastX, (double)firstY, (double)lastX, (double)lastY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)lastX, (double)lastY, (double)firstX, (double)lastY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)firstX, (double)lastY, (double)firstX, (double)firstY);
      ClipDraw(InterWind0, &IA->cb, &ls);

      } /* if */
     else
      {
      lastX = Local.MouseX;
      lastY = Local.MouseY;
      done = 1;
      } /* else */
     } /* if SELECTUP */
    else if (Local.Code == MENUUP)
     {
     done = 1;
     abort = 1;
     } /* else if MENUUP abort */
    break;
    } /* MOUSEBUTTONS */
   case INTUITICKS:
    {
    if (ReplyWin != InterWind0)
     break;
    if (firstX >= -999)
     {
     if (lastX == Local.MouseX && lastY == Local.MouseY)
      {
      break;
      } /* if no movement */
     nextX = Local.MouseX;
     nextY = Local.MouseY;

     for (i=0; i<2; i++)
      {
      setlineseg(&ls, (double)firstX, (double)firstY, (double)lastX, (double)firstY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)lastX, (double)firstY, (double)lastX, (double)lastY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)lastX, (double)lastY, (double)firstX, (double)lastY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      setlineseg(&ls, (double)firstX, (double)lastY, (double)firstX, (double)firstY);
      ClipDraw(InterWind0, &IA->cb, &ls);
      lastX = nextX;
      lastY = nextY;
      } /* for undraw and draw */
     } /* if firstX */
    break;
    } /* INTUITICKS */
   } /* switch Local.Class */
  } /* while ! done */

 ModifyIDCMP(EMIA_Win->Win, IDCMPFlags);
 ModifyIDCMP(InterWind0, MOUSEBUTTONS | RAWKEY | VANILLAKEY | NEWSIZE
	 | CLOSEWINDOW | ACTIVEWINDOW);
 SetWindowTitles(InterWind0, varname[EM_Win->MoItem], (UBYTE *)-1);

 if (firstX >= -999)
  {
  setlineseg(&ls, (double)firstX, (double)firstY, (double)lastX, (double)firstY);
  ClipDraw(InterWind0, &IA->cb, &ls);
  setlineseg(&ls, (double)lastX, (double)firstY, (double)lastX, (double)lastY);
  ClipDraw(InterWind0, &IA->cb, &ls);
  setlineseg(&ls, (double)lastX, (double)lastY, (double)firstX, (double)lastY);
  ClipDraw(InterWind0, &IA->cb, &ls);
  setlineseg(&ls, (double)firstX, (double)lastY, (double)firstX, (double)firstY);
  ClipDraw(InterWind0, &IA->cb, &ls);
  }

 SetDrMd(InterWind0->RPort, JAM1);

 if (abort)
  {
  return;
  } /* if abort */


 if (firstX > lastX) swmem(&firstX, &lastX, 2);
 if (firstY > lastY) swmem(&firstY, &lastY, 2);
 WindowNumber = NoOfSmWindows;

 if ((SmWin[WindowNumber] = (struct SmallWindow *)
	get_Memory(sizeof (struct SmallWindow), MEMF_CLEAR)) == NULL)
  {
  return;
  } /* if out of memory */

 NoOfSmWindows ++;

/* open small window */
 flags = ACTIVATE | SMART_REFRESH | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE |
		WINDOWSIZING | REPORTMOUSE | RMBTRAP;
 iflags = NULL;
 c0 = 0x00;
 c1 = 0x01;
 sprintf(SmWin[WindowNumber]->Title, "IA %d", IA->WindowCounter);
 if (lastX - firstX < 20) lastX = firstX + 20; 
 SmWin[WindowNumber]->win = (struct Window *)
     make_window(InterWind0->LeftEdge + firstX - InterWind0->BorderLeft,
	InterWind0->TopEdge + firstY - InterWind0->BorderTop,
	lastX - firstX + InterWind0->BorderLeft + InterWind0->BorderRight + 1,
	lastY - firstY + InterWind0->BorderTop + InterWind0->BorderBottom + 1,
	SmWin[WindowNumber]->Title, flags, iflags, c0, c1, WCSScrn);
 if (! SmWin[WindowNumber]->win)
  {
  User_Message("Camera View",
	"Error opening Small Rendering Window!\nOperation terminated.", "OK", "o");
  Log(ERR_WIN_FAIL, "Small rendering window.");
  goto Cleanup;
  } /* if can't open window */

 ModifyIDCMP(SmWin[WindowNumber]->win, CLOSEWINDOW | ACTIVEWINDOW | MOUSEBUTTONS);
 SmWin[WindowNumber]->Signal = 1L << SmWin[WindowNumber]->win->UserPort->mp_SigBit;
 WCS_Signals |= SmWin[WindowNumber]->Signal;
 SetPointer(SmWin[WindowNumber]->win, WaitPointer, 16, 16, -6, 0);

/* set render bounds */
 SmWin[WindowNumber]->cb.lowx = SmWin[WindowNumber]->win->BorderLeft;
 SmWin[WindowNumber]->cb.highx = SmWin[WindowNumber]->win->BorderLeft
	+ lastX - firstX;
 SmWin[WindowNumber]->cb.lowy = SmWin[WindowNumber]->win->BorderTop;
 SmWin[WindowNumber]->cb.highy = SmWin[WindowNumber]->win->BorderTop
	+ lastY - firstY;
 SmWin[WindowNumber]->width = lastX - firstX + 1;
 IA->WindowCounter ++;


/* copy parameter structures to small window structure */
 memcpy(&SmWin[WindowNumber]->MoPar, &MoPar, sizeof (struct Animation));
 memcpy(&SmWin[WindowNumber]->CoPar, &CoPar, sizeof (struct Palette));
 memcpy(&SmWin[WindowNumber]->EcoPar, &EcoPar, sizeof (union Environment));
 memcpy(&SmWin[WindowNumber]->settings, &settings, sizeof (struct Settings));

/* make temporary settings assignments */

/* draw rendering */
 altdrawoffsetX = drawoffsetX;
 altdrawoffsetY = drawoffsetY;
 altCenterX = CenterX;
 altCenterY = CenterY;
 altclouds = settings.clouds;
 altfixfract = settings.fixfract;
 scrnwidth = lastX - firstX + 1;
 scrnheight = lastY - firstY + 1;
 zbufsize = scrnwidth * scrnheight * 4;
 if ((zbuf = (float *)get_Memory(zbufsize, MEMF_ANY)) == NULL)
  {
  goto Cleanup;
  } /* if */
 for (i=0; i<scrnheight; i++)
  {
  for (j=0; j<scrnwidth; j++)
   {
   *(zbuf + i * scrnwidth + j) = FLT_MAX;
   } /* for j=0... */
  } /* for i=0... */
 if ((bytemap = (USHORT *)get_Memory(zbufsize / 2, MEMF_CLEAR)) == NULL)
  {
  goto Cleanup;
  } /* if */
 if (diagnostics) render = 0x110;
 else render = 0x10;
 if (render & 0x100) 
  {
  QCmapsize = zbufsize;
  allocQCmaps();
  } /* if render QC buffer */

 settings.clouds = 0;
 cmap = 0;
 
 for (i=0; i<scrnheight; i++) scrnrowzip[i] = i * scrnwidth;

 initpar();
 constructview();
 setvalues();
 wide = scrnwidth - 1;
 oshigh = high = scrnheight - 1;
/* Change 7/18 */
 CenterX = (PARC_RNDR_MOTION(6) / IA->wscale) - firstX + drawoffsetX;	/* must follow setvalues() */
 CenterY = (PARC_RNDR_MOTION(7) / IA->hscale) - firstY + drawoffsetY;
 drawoffsetX = SmWin[WindowNumber]->win->BorderLeft;
 drawoffsetY = SmWin[WindowNumber]->win->BorderTop;
 LoadRGB4( &WCSScrn->ViewPort, &Colors[0], 16 );

/* load images if needed */
 if (! BitmapImage_Load())
  goto AbortRender;

 for (i=0; i<ECOPARAMS; i++) ecocount[i] = 0;

/* find closest 4 maps appearing in small window */
 for (j=0; j<4 && j<NoOfElMaps; j++)
  {
  lowq = 10000.0;
  lowmap = -1;
  for (i=0; i<NoOfElMaps; i++)
   {
   if (i == map[0] || i == map[1] || i == map[2])
    continue;
   scrnx=(float *)elmap[i].scrnptrx;
   scrny=(float *)elmap[i].scrnptry;
   scrnq=(float *)elmap[i].scrnptrq;
   for (Lr=0; Lr<=elmap[i].rows; Lr ++)
    {
    for (Lc=0; Lc<elmap[i].columns; Lc++)
     { 
     if (*scrnx + altdrawoffsetX > firstX && *scrnx + altdrawoffsetX < lastX && 
	*scrny + altdrawoffsetY > firstY && *scrny + altdrawoffsetY < lastY)
      {
      if (*scrnq > .01)
       {
       if (*scrnq < lowq)
        {
        lowq = *scrnq;
        lowmap = i;
        } /* if closest point so far */
       } /* if point is in front of viewer */
      } /* if within small window */
     scrnx++;
     scrny++;
     scrnq++;
     } /* for Lc=0... */
    } /* for Lr=0... */
   } /* for i=0... */
  map[j] = lowmap;

  if (lowmap < 0)
   break;

/* find bounds of window within map array */
  lowrow = elmap[lowmap].rows;
  lowcol = elmap[lowmap].columns;
  highrow = 0;
  highcol = 0;
  scrnx=(float *)elmap[lowmap].scrnptrx;
  scrny=(float *)elmap[lowmap].scrnptry;
  scrnq=(float *)elmap[lowmap].scrnptrq;
  for (Lr=0; Lr<=elmap[lowmap].rows; Lr ++)
   {
   for (Lc=0; Lc<elmap[lowmap].columns; Lc++)
    { 
    if (*scrnx + drawoffsetX > firstX && *scrnx + drawoffsetX < lastX &&
	*scrny + drawoffsetY > firstY && *scrny + drawoffsetY < lastY)
     {
     if (*scrnq > .01)
      {
      if (Lr < lowrow)
       {
       lowrow = Lr;
       } /* if new low row */
      if (Lr > highrow)
       {
       highrow = Lr;
       } /* if new high row */
      if (Lc < lowcol)
       {
       lowcol = Lc;
       } /* if new low column */
      if (Lc > highcol)
       {
       highcol = Lc;
       } /* if new high column */
      } /* if point is in front of viewer */
     } /* if within small window */
    scrnx++;
    scrny++;
    scrnq++;
    } /* for Lc=0... */
   } /* for Lr=0... */
  if (lowrow > 0) lowrow --;
  if (highrow < elmap[lowmap].rows) highrow ++;
  if (lowcol > 0) lowcol --;
  if (highcol < elmap[lowmap].columns - 1) highcol ++;
  if (*(AltRenderList + lowmap * 2 + 1))
   {
   lowrow = elmap[lowmap].rows - lowrow;
   highrow = elmap[lowmap].rows - highrow;
   swmem(&lowrow, &highrow, 2);
   } /* if map is reversed */
  if (highrow >= elmap[lowmap].rows) UseMaxRows = 1;
  else UseMaxRows = 0;
  if (highcol >= elmap[lowmap].columns - 1) UseMaxCols = 1;
  else UseMaxCols = 0;
  lowrow *= IA_GridSize;
  highrow *= IA_GridSize;
  lowcol *= IA_GridSize;
  highcol *= IA_GridSize;
 
/* find elevation file corresponding to chosen map */
  MapOBN = *(AltRenderList + lowmap * 2);

  OpenOK = 0;

  DLItem = DL;
  while (DLItem)
   {
   strmfp (filename, DLItem->Name, DBase[MapOBN].Name);
   strcat(filename, ".elev");
   if (readDEM (filename, &TempHdr) != 0)
    {
    DLItem = DLItem->Next;
    } /* if not opened */
   else
    {
    OpenOK = 1;
    strcpy(elevpath, DLItem->Name);
    strcpy(elevfile, DBase[MapOBN].Name);
    strcat(elevfile, ".elev");
    break;
    } /* else file opened */
   } /* while */

  if (! OpenOK)
   {
   sprintf(str, "%s.elev", DBase[MapOBN].Name);
   Log(ERR_OPEN_FAIL, str);
   User_Message(str, 
	"Error opening DEM file for input!\nOperation terminated.", "OK", "o");
   abort = 1;
   goto EndMap;
   } /* if open error */

  memcpy(&SmWin[WindowNumber]->elmap, &TempHdr, ELEVHDRLENV101);

/* allocate map and scrnptr arrays */

  if (UseMaxRows) highrow = SmWin[WindowNumber]->elmap.rows;
  if (UseMaxCols) highcol = SmWin[WindowNumber]->elmap.columns - 1;
  newrows = highrow - lowrow;
  newcolumns = 1 + highcol - lowcol;

  SmWin[WindowNumber]->elmap.size = (newrows + 1) * newcolumns * 2;
  SmWin[WindowNumber]->elmap.scrnptrsize = SmWin[WindowNumber]->elmap.size * 2;
  if (((SmWin[WindowNumber]->elmap.map = (short *)
	get_Memory (SmWin[WindowNumber]->elmap.size, MEMF_ANY)) == NULL)
	|| ((SmWin[WindowNumber]->elmap.scrnptrx = (float *)
	get_Memory (SmWin[WindowNumber]->elmap.scrnptrsize, MEMF_ANY)) == NULL)
	|| ((SmWin[WindowNumber]->elmap.scrnptry = (float *)
	get_Memory (SmWin[WindowNumber]->elmap.scrnptrsize, MEMF_ANY)) == NULL)
	|| ((SmWin[WindowNumber]->elmap.scrnptrq = (float *)
	get_Memory (SmWin[WindowNumber]->elmap.scrnptrsize, MEMF_ANY)) == NULL))
   {
   free_Memory (TempHdr.map, TempHdr.size);
   User_Message(str,
	"Out of memory! Try a smaller preview size.\nOperation terminated.", "OK", "o");
   abort = 1;
   goto EndMap;
   } /* if */

  if (strcmp(DBase[MapOBN].Special, "TOP") || settings.mapassfc)
   SmWin[WindowNumber]->elmap.MapAsSFC = 1;
  else
   SmWin[WindowNumber]->elmap.MapAsSFC = 0;

  if (settings.smoothfaces || SmWin[WindowNumber]->elmap.MapAsSFC)
   {
   NumPoints = 6 * SmWin[WindowNumber]->elmap.columns;
   if ((SmWin[WindowNumber]->elmap.face = (struct faces *)
	get_Memory(NumPoints * sizeof (struct faces), MEMF_ANY)) == NULL)
    {
    if (User_Message("Camera View",
	"Out of memory allocating Polygon Smoothing array!\n\
Continue without Polygon Smoothing?", "OK|Cancel", "oc"))
     {
     settings.smoothfaces = 0;
     SmWin[WindowNumber]->elmap.face = NULL;
     } /* if */
    else
     {
     abort = 1;
     goto EndMap;
     } /* else */
    } /* if no memory */
   if (SmWin[WindowNumber]->elmap.MapAsSFC)
    SmWin[WindowNumber]->elmap.ForceBath = 1;
   else
    SmWin[WindowNumber]->elmap.ForceBath = 0;
   } /* if smooth faces */
  else
   {
   SmWin[WindowNumber]->elmap.face = NULL;
   SmWin[WindowNumber]->elmap.ForceBath = 0;
   } /* else no smoothing */
   
  if (settings.fractalmap || (! settings.fixfract && settings.displace))
   {
   SmWin[WindowNumber]->elmap.fractalsize = SmWin[WindowNumber]->elmap.rows
	 * (SmWin[WindowNumber]->elmap.columns - 1) * 2;
   if ((SmWin[WindowNumber]->elmap.fractal =
	 (BYTE *)get_Memory(SmWin[WindowNumber]->elmap.fractalsize, MEMF_CLEAR)) != NULL)
    {
    settings.fixfract = 0;
    } /* if no memory for fractal map */
   } /* if */
  else
   SmWin[WindowNumber]->elmap.fractal = NULL;

/* transfer data to map array */
  mapptr = (short *)SmWin[WindowNumber]->elmap.map;

  rowsize = SmWin[WindowNumber]->elmap.columns;
  maprowsize = highcol - lowcol + 1;
  maprowbytes = maprowsize * 2;
  temparrayptr = TempHdr.map + lowrow * rowsize + lowcol;
  for (Lr=lowrow; Lr<=highrow; Lr++)
   {
   memcpy(mapptr, temparrayptr, maprowbytes);
   mapptr += maprowsize;
   temparrayptr += rowsize;
   } /* for Lr=0... */

  if (TempHdr.map)
   free_Memory(TempHdr.map, TempHdr.size);
  TempHdr.map = NULL;

/* load relel data */
  if (SmWin[WindowNumber]->elmap.MapAsSFC == 0)
   {
   sprintf(str, "%s.relel", DBase[MapOBN].Name);
   OpenOK = 0;

   DLItem = DL;
   while (DLItem)
    {
    strmfp(filename, DLItem->Name, DBase[MapOBN].Name);
    strcat(filename, ".relel");
    if (readDEM (filename, &TempHdr) != 0)
     {
     DLItem = DLItem->Next;
     } /* if file not found */
    else
     {
     OpenOK = 1;
     break;
     } /* else file opened */
    } /* while */

   if (! OpenOK)
    {
    if (makerelelfile(elevpath, elevfile))
     {
     strmfp(filename, elevpath, elevfile);	/* elevfile suffix changed to .relel */
     if (readDEM(filename, &TempHdr) != 0)
      {
      Log(WNG_OPEN_FAIL, elevfile);
      } /* if still no relel file */
     } /* if make relel successful */
    } /* if open file error */
 
   relelev.size = SmWin[WindowNumber]->elmap.size;
   if ((relelev.map = (short *)get_Memory(relelev.size, MEMF_CLEAR)) == NULL)
    {
    sprintf(str, "%s.relel", DBase[MapOBN].Name);
    User_Message(str,
	"Out of memory!\nOperation terminated.", "OK", "o");
    abort = 1;
    goto EndMap;
    } /* if out of memory */
   if (OpenOK)
    {
    mapptr = (short *)relelev.map;

    rowsize = TempHdr.columns;
    maprowsize = highcol - lowcol + 1;
    maprowbytes = maprowsize * 2;
    temparrayptr = TempHdr.map + lowrow * rowsize + lowcol;
    for (Lr=lowrow; Lr<=highrow; Lr++)
     {
     memcpy(mapptr, temparrayptr, maprowbytes);
     mapptr += maprowsize;
     temparrayptr += rowsize;
     } /* for Lr=0... */
    } /* if relel file found */
   } /* if topo DEM */

  SmWin[WindowNumber]->elmap.rows = relelev.rows = newrows;
  SmWin[WindowNumber]->elmap.columns = relelev.columns = newcolumns;
  SmWin[WindowNumber]->elmap.lolat +=
		lowcol * SmWin[WindowNumber]->elmap.steplat;
  SmWin[WindowNumber]->elmap.lolong -=
		lowrow * SmWin[WindowNumber]->elmap.steplong;

  relelev.lolat = SmWin[WindowNumber]->elmap.lolat;
  relelev.lolong = SmWin[WindowNumber]->elmap.lolong;

/* deallocate temporary array */
 if (TempHdr.map)
  free_Memory(TempHdr.map, TempHdr.size);
 TempHdr.map = NULL;

/* allocate long map array */
  if ((SmWin[WindowNumber]->elmap.lmap =
	 (long *)get_Memory(SmWin[WindowNumber]->elmap.size * 2, MEMF_ANY)) == NULL)
   {
   abort = 1;
   goto EndMap;
   } /* if out of memory */

  SubPixArraySize = 10000;
  TreePixArraySize = 10000;
  EdgeSize = 1000;
  SubPix = get_Memory(SubPixArraySize, MEMF_CLEAR);
  TreePix = get_Memory(TreePixArraySize, MEMF_CLEAR);
  Edge1 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  Edge2 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  if (! Edge1 || ! Edge2 || ! SubPix || ! TreePix)
   {
   User_Message("Render Module",
	"Out of memory allocating antialias and edge buffers!\nOperation terminated.", "OK", "o");
   abort = 1;
   goto EndMap;
   } /* if memory fail */

  if ((abort = maptopoobject(&SmWin[WindowNumber]->elmap,
	 SmWin[WindowNumber]->win, j + 1, min(4, NoOfElMaps), NULL)) != 0)
   goto EndMap; 

EndMap:
  if (SubPix) free_Memory(SubPix, SubPixArraySize);
  SubPix = NULL;
  if (TreePix) free_Memory(TreePix, TreePixArraySize);
  TreePix = NULL;
  if (Edge1) free_Memory(Edge1, EdgeSize);
  if (Edge2) free_Memory(Edge2, EdgeSize);
  Edge1 = Edge2 = NULL;

  if (relelev.map) free_Memory(relelev.map, relelev.size);
  relelev.map = NULL;
  if (SmWin[WindowNumber]->elmap.map)
	free_Memory(SmWin[WindowNumber]->elmap.map,
	SmWin[WindowNumber]->elmap.size);
  SmWin[WindowNumber]->elmap.map = NULL;
  if (SmWin[WindowNumber]->elmap.lmap)
	free_Memory(SmWin[WindowNumber]->elmap.lmap,
	SmWin[WindowNumber]->elmap.size * 2);
  SmWin[WindowNumber]->elmap.lmap = NULL;
  if (SmWin[WindowNumber]->elmap.scrnptrx)
	free_Memory(SmWin[WindowNumber]->elmap.scrnptrx,
	SmWin[WindowNumber]->elmap.scrnptrsize);
  SmWin[WindowNumber]->elmap.scrnptrx = NULL;
  if (SmWin[WindowNumber]->elmap.scrnptry)
	free_Memory(SmWin[WindowNumber]->elmap.scrnptry,
	SmWin[WindowNumber]->elmap.scrnptrsize);
  SmWin[WindowNumber]->elmap.scrnptry = NULL;
  if (SmWin[WindowNumber]->elmap.scrnptrq)
	free_Memory(SmWin[WindowNumber]->elmap.scrnptrq,
	SmWin[WindowNumber]->elmap.scrnptrsize); 
  SmWin[WindowNumber]->elmap.scrnptrq = NULL;
  if (SmWin[WindowNumber]->elmap.face)
	free_Memory(SmWin[WindowNumber]->elmap.face,
	NumPoints * sizeof (struct faces)); 
  SmWin[WindowNumber]->elmap.face = NULL;
  if (SmWin[WindowNumber]->elmap.fractal)
	free_Memory(SmWin[WindowNumber]->elmap.fractal,
	SmWin[WindowNumber]->elmap.fractalsize);
  SmWin[WindowNumber]->elmap.fractal = NULL;

  if (abort) break;
  } /* for j=0... */

 Log(DTA_NULL, "Ecosystems:");
 for (i=0; i<ECOPARAMS; i++)
  {
  if (ecocount[i])
   {
   sprintf(str, "%2d. %-24s %ld\n", i, EcoPar.en[i].Name, ecocount[i]);
   Log(MSG_UTIL_TAB, str);
   } /* if */
  } /* for i=0... */

AbortRender:

 BitmapImage_Unload();

/* reassign settings to original state */
 settings.clouds = altclouds;
 settings.fixfract = altfixfract;
 drawoffsetX = altdrawoffsetX;
 drawoffsetY = altdrawoffsetY;

 CenterX = altCenterX;
 CenterY = altCenterY;
 render = 0;

/* open diagnostic window, clean up unneeded memory */
 if (EMIA_Win) ClearPointer(EMIA_Win->Win);
 ClearPointer(SmWin[WindowNumber]->win);
 if (diagnostics)
  {
  Open_Diagnostic_Window(SmWin[WindowNumber]->win,
	SmWin[WindowNumber]->Title);
  } /* if diagnostics */
 else
  {
  if (zbuf) free_Memory(zbuf, zbufsize);
  zbuf = (float *)NULL;
  } /* else no diagnostics */
 if (bytemap) free_Memory(bytemap, zbufsize / 2);
 bytemap = NULL;

 while (QuickFetchEvent(SmWin[WindowNumber]->win, &Event))
  {
  } /* discard any messages to interactive window */

 return;

Cleanup:
 Close_Small_Window(WindowNumber);

} /* smallwindow() */

/*************************************************************************/

void Close_Small_Window(short win_number)
{
 short i;

 if (DIAG_Win)
  {
  if (DIAG_Win->EcosystemWin == SmWin[win_number]->win)
   Close_Diagnostic_Window();
  } /* if diagnostic window open */

/* deallocate map and scrnptr arrays, NULL pointers */
 if (zbuf) free_Memory(zbuf, zbufsize);
 zbuf = (float *)NULL;
 if (bytemap) free_Memory(bytemap, zbufsize / 2);
 bytemap = NULL;

 if (relelev.map) free_Memory(relelev.map, relelev.size);
 relelev.map = (short *)NULL;

 if (SmWin[win_number])
  {
  WCS_Signals ^= SmWin[win_number]->Signal;
  if (SmWin[win_number]->win) CloseWindow(SmWin[win_number]->win);

/* deallocate memory for arrays in case Close is called from smallwindow/Cleanup */
  if (SmWin[win_number]->elmap.map)
	free_Memory(SmWin[win_number]->elmap.map,
	SmWin[win_number]->elmap.size);
  SmWin[win_number]->elmap.map = NULL;
  if (SmWin[win_number]->elmap.lmap)
	free_Memory(SmWin[win_number]->elmap.lmap,
	SmWin[win_number]->elmap.size * 2);
  SmWin[win_number]->elmap.lmap = NULL;
  if (SmWin[win_number]->elmap.scrnptrx)
	free_Memory(SmWin[win_number]->elmap.scrnptrx,
	SmWin[win_number]->elmap.scrnptrsize);
  SmWin[win_number]->elmap.scrnptrx = NULL;
  if (SmWin[win_number]->elmap.scrnptry)
	free_Memory(SmWin[win_number]->elmap.scrnptry,
	SmWin[win_number]->elmap.scrnptrsize);
  SmWin[win_number]->elmap.scrnptry = NULL;
  if (SmWin[win_number]->elmap.scrnptrq)
	free_Memory(SmWin[win_number]->elmap.scrnptrq,
	SmWin[win_number]->elmap.scrnptrsize); 
  SmWin[win_number]->elmap.scrnptrq = NULL;
  if (SmWin[win_number]->elmap.fractal)
	free_Memory(SmWin[win_number]->elmap.fractal,
	SmWin[win_number]->elmap.fractalsize);
  SmWin[win_number]->elmap.fractal = NULL;

  free_Memory(SmWin[win_number], sizeof (struct SmallWindow)); 
  } /* if memory allocated for small window structure */

 for (i=win_number; i<NoOfSmWindows - 1; i++)
  {
  SmWin[i] = SmWin[i + 1];
  } /* for i=0... */

 NoOfSmWindows --;

 SmWin[NoOfSmWindows] = NULL;

} /* Close_Small_Window() */

/*************************************************************************/

void Handle_Small_Window(short win_number)
{
short abort = 0;

 while (QuickFetchEvent(SmWin[win_number]->win, &Event))
  {
  switch (Event.Class)
   {
   case CLOSEWINDOW:
    {
    Close_Small_Window(win_number);
    abort = 1;
    break;
    } /* CLOSEWINDOW */

   case ACTIVEWINDOW:
    {
    LoadRGB4(&WCSScrn->ViewPort, &Colors[0], 16);
    if (Event.MouseX >= SmWin[win_number]->cb.lowx &&
	Event.MouseX <= SmWin[win_number]->cb.highx &&
	Event.MouseY >= SmWin[win_number]->cb.lowy &&
	Event.MouseY <= SmWin[win_number]->cb.highy)
     {
     if (memcmp(&MoPar, &SmWin[win_number]->MoPar, sizeof (struct Animation)) ||
      memcmp(&CoPar, &SmWin[win_number]->CoPar, sizeof (struct Palette)) ||
      memcmp(&EcoPar, &SmWin[win_number]->EcoPar, sizeof (union Environment)) ||
      memcmp(&settings, &SmWin[win_number]->settings, sizeof (struct Settings)))
      {
      if (User_Message_Def("Parameters Module: Preview",
	"Restore the Parameters used to create this preview?", "OK|Cancel", "oc", 1))
       {
       memcpy(&MoPar, &SmWin[win_number]->MoPar, sizeof (struct Animation));
       memcpy(&CoPar, &SmWin[win_number]->CoPar, sizeof (struct Palette));
       memcpy(&EcoPar, &SmWin[win_number]->EcoPar, sizeof (union Environment));
       memcpy(&settings, &SmWin[win_number]->settings, sizeof (struct Settings));
       Set_EM_Item(EM_Win->MoItem);
       IA->recompute = 1;
       } /* if restore */
      } /* if parameters changed */
     } /* if in rendered portion of window (so as not to respond to window close */
    break;
    } /* ACTIVEWINDOW */

   case MOUSEBUTTONS:
    {
    if (DIAG_Win)
     {
     if (SmWin[win_number]->win == DIAG_Win->EcosystemWin)
      {
      switch (Event.Code)
       {
       case SELECTDOWN:
        {
        if (Event.MouseX >= SmWin[win_number]->cb.lowx &&
		Event.MouseX <= SmWin[win_number]->cb.highx &&
		Event.MouseY >= SmWin[win_number]->cb.lowy &&
		Event.MouseY <= SmWin[win_number]->cb.highy)
         {
         Set_Diagnostic_Point((Event.MouseY - SmWin[win_number]->cb.lowy)
		* SmWin[win_number]->width
		+ Event.MouseX - SmWin[win_number]->cb.lowx);
         if (! IA->Digitizing)
          ModifyIDCMP(SmWin[win_number]->win, MOUSEMOVE | MOUSEBUTTONS);
         } /* if in rendered bounds */
        break;
        } /* SELECTDOWN */
       case SELECTUP:
        {
        if (IA->Digitizing)
         {
         long remainder, zip;

         zip = (Event.MouseY - SmWin[win_number]->cb.lowy)
		* SmWin[win_number]->width
		+ Event.MouseX - SmWin[win_number]->cb.lowx;
         remainder =  *(QCmap[0] + zip) / 65536;
         IA->Digitizing = PerspectivePoint(1, (double)(*(QCcoords[0] + zip)),
		(double)(*(QCcoords[1] + zip)), remainder);
         if (! IA->Digitizing) QuitDigPerspective();
	 }
        else
         ModifyIDCMP(SmWin[win_number]->win, CLOSEWINDOW | ACTIVEWINDOW | MOUSEBUTTONS);
        break;
	} /* SELECTUP */
       case MENUUP:			/* delete last point */
        {
        PerspectivePoint(0, 0.0, 0.0, 0);
        break;
        } /* MENUUP */
       } /* switch Event.Code */
      } /* if correct small window */
     } /* if diagnostic window open */
    break;
    } /* MOUSEBUTTONS */

   case MOUSEMOVE:
    {
    if (Event.MouseX >= SmWin[win_number]->cb.lowx &&
		Event.MouseX <= SmWin[win_number]->cb.highx &&
		Event.MouseY >= SmWin[win_number]->cb.lowy &&
		Event.MouseY <= SmWin[win_number]->cb.highy)
     {
     Set_Diagnostic_Point((Event.MouseY - SmWin[win_number]->cb.lowy)
		* SmWin[win_number]->width
		+ Event.MouseX - SmWin[win_number]->cb.lowx);
     } /* if in rendered bounds */
    break;
    } /* MOUSEMOVE */

   } /* switch Event.Class */

  if (abort) break;

  } /* while */

} /* Handle_Small_Window() */

/*************************************************************************/

void Handle_InterWind0(void)
{
short abort = 0;

 while (QuickFetchEvent(InterWind0, &Event))
  {
  switch (Event.Class)
   {
   case CLOSEWINDOW:
    {
    Close_EMIA_Window(CloseWindow_Query("Interactive Motion"));
    abort = 1;
    break;
    } /* CLOSEWINDOW */

   case ACTIVEWINDOW:
    {
/*    WindowToFront(InterWind0);*/
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* ACTIVEWINDOW */

   case NEWSIZE:
    {
    setclipbounds(InterWind0, &IA->cb);
    Init_IA_View(2);
    if (AN_Win)
     {
     set(AN_Win->IntStr[2], MUIA_String_Integer, InterWind0->Width);
     set(AN_Win->IntStr[3], MUIA_String_Integer, InterWind0->Height);
     } /* if anim window open */
    break;
    } /* NEWSIZE */

   case RAWKEY:
    {
    if (Event.Qualifier & IEQUALIFIER_CONTROL)
     {
     if ((Event.Code == CURSORUP) && EM_Win->MoItem > 0)
	 set(EM_Win->LS_List, MUIA_List_Active, EM_Win->MoItem - 1);
     else if ((Event.Code == CURSORDOWN) && EM_Win->MoItem < USEDMOTIONPARAMS - 1)
	 set(EM_Win->LS_List, MUIA_List_Active, EM_Win->MoItem + 1);
     break;
     } /* if shift key */
    if ((Event.Qualifier & IEQUALIFIER_LSHIFT) ||
	(Event.Qualifier & IEQUALIFIER_RSHIFT))
     {
     IA->mult = 10;
     } /* if shift key */
    else
     {
     IA->mult = 1;
     } /* else no shift key */
    if ((Event.Qualifier & IEQUALIFIER_LALT) ||
	(Event.Qualifier & IEQUALIFIER_RALT))
     {
     IA->button = MENUDOWN;
     } /* if alt key */
    else
     {
     IA->button = SELECTDOWN;
     } /* else not alt key */
    switch (Event.Code)
     {
     case 0x4c:
      {
      IA->shiftx = 0;
      IA->shifty = -1 * IA->mult;
      IA->shifting = 1;
      break;
      } /* arrow up */
     case 0x4d:
      {
      IA->shiftx = 0;
      IA->shifty = 1 * IA->mult;
      IA->shifting = 1;
      break;
      } /* arrow down */
     case 0x4e:
      {
      IA->shiftx = 1 * IA->mult;
      IA->shifty = 0;
      IA->shifting = 1;
      break;
      } /* arrow right */
     case 0x4f:
      {
      IA->shiftx = -1 * IA->mult;
      IA->shifty = 0;
      IA->shifting = 1;
      break;
      } /* arrow left */
     } /* switch Event.Code */
    if (IA->shifting)
     {
     if (item[0] == 2)
      {
      modifycampt(IA->shiftx, IA->shifty, IA->button);
      } /* if */
     else if (item[0] == 5)
      {
      modifyfocpt(IA->shiftx, IA->shifty, IA->button);
      } /* else if */
     else
      {
      modifyinteractive(IA->shiftx, IA->shifty, IA->button);
      boundscheck(item[0]);
      boundscheck(item[1]);
      boundscheck(item[2]);
      } /* else */

     constructview();
     if (IA->gridpresent)
      {
      SetAPen(InterWind0->RPort, 1);
      RectFill(InterWind0->RPort,
	IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
      } /* if */
     if (GridBounds)
      {
/* this needs work - crashes!%$@#%&^#@%#^@!!
      computeview(NULL);
      drawinterview();*/
      } /* if */
     if(BoxBounds)
      {
      drawquick(1, 1, 1, 0);
      computequick();
      drawquick(7, 6, 3, 0);
      } /* if */
     if (ProfileBounds)
      {
      drawfocprof(1);
      if (item[0] == 5)
       {
       findfocprof();
       } /* if modify focus point */
      drawfocprof(0);
      } /* if */
     if(LandBounds)
      {
      drawveryquick(1);
      } /* if */
     if (CompassBounds)
      {
      makeviewcenter(1);
      } /* if */
     Update_EM_Item();

     IA->shifting = 0;
     } /* if IA->shifting */
    break;
    } /* RAWKEY */

   case VANILLAKEY:
    {
    switch(Event.Code)
     {
     case 'v':
     case 'V':
      {
      oshigh = high = InterWind0->Height - InterWind0->BorderTop
	 - InterWind0->BorderBottom - 1;
      wide = InterWind0->Width - InterWind0->BorderLeft
	 - InterWind0->BorderRight - 1;
      constructview();
      InitVectorMap(InterWind0, 0, 1);
      RefreshWindowFrame(InterWind0);
      break;
      } /* draw vector objects */
     case 't':
     case 'T':
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        abort = 1;
        break;
	} /* if four attempts fail */
       } /* if IA->newgridsize */
      else drawgridview();
      break;
      } /* draw grid */
     case 'x':
     case 'X':
      {
      FixXYZ('x');
      break;
      } /* X */
     case 'y':
     case 'Y':
      {
      FixXYZ('y');
      break;
      } /* Y */
     case 'z':
     case 'Z':
      {
      FixXYZ('z');
      break;
      } /* Y */
     case 'b':
     case 'B':
      {
      set(EMIA_Win->BT_BoxBounds, MUIA_Selected, !BoxBounds);
      break;
      } /* BoxBounds */
     case 'l':
     case 'L':
      {
      set(EMIA_Win->BT_LandBounds, MUIA_Selected, !LandBounds);
      break;
      } /* LandBounds */
     case 'g':
     case 'G':
      {
      set(EMIA_Win->BT_ProfileBounds, MUIA_Selected, !ProfileBounds);
      break;
      } /* ProfileBounds now referred to as TargetBounds */
     case 'p':
     case 'P':
      {
      set(EMIA_Win->BT_GridBounds, MUIA_Selected, !GridBounds);
      break;
      } /* GridBounds now referred to as ProfileBounds */
     case 'o':
     case 'O':
      {
      set(EMIA_Win->BT_CompassBounds, MUIA_Selected, !CompassBounds);
      break;
      } /* CompassBounds */
     case 'n':
     case 'N':
      {
      autocenter();
      findfocprof();
      Update_EM_Item();
      drawgridview();
      break;
      } /* ceNter focus() */
     case 'h':
     case 'H':
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        break;
	} /* no new grid */
       } /* if IA->newgridsize */
      shaderelief(0);
      break;
      } /* ElS_H_ade */
     case 's':
     case 'S':
      {
      if (IA->newgridsize || CheckDEMStatus())
       {
       IA->newgridsize = 0;
       if (! OpenNewIAGridSize())
        {
        Close_EMIA_Window(-1);
        break;
		} /* no new grid */
       } /* if IA->newgridsize */
      shaderelief(1);
      break;
      } /* SunShade */
     case 'e':
     case 'E':
      {
      smallwindow(0);
      break;
      } /* render small window */
     case 'd':
     case 'D':
      {
      smallwindow(1);
      break;
      } /* render with diagnostics */
     case 'm':
     case 'M':
      {
      Init_Anim(1);
      break;
      } /* render anim */
     case 13:
     case 10:
      {
      if (Event.Qualifier & IEQUALIFIER_NUMERICPAD)
       MakeMotionKey();
      break;
      } /* enter */
     } /* switch Event.Code */
    break;
    } /* VANILLAKEY */

   case MOUSEBUTTONS:
    {
    switch (Event.Code)
     {
     case SELECTDOWN:
     case MENUDOWN:
      {
      short AltKey = 0;

      if (Event.MouseX < 0 || Event.MouseX >= InterWind0->Width ||
		Event.MouseY < 0 || Event.MouseY >= InterWind0->Height)
	break;

      if (Event.Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
       AltKey = 1;

      ModifyIDCMP(InterWind0, MOUSEBUTTONS | INTUITICKS ); 

      IA->button = Event.Code;
      IA->startx = Event.MouseX;
      IA->starty = Event.MouseY;

      if (IA_Movement)
       {
       if (item[0] == 2) reversecamcompute();
       else if (item[0] == 5) reversefoccompute();
       } /* if rectang movement - unsetting key frames by changing active 
		parameters can mess up the positioning. Not a problem for
		radial movement since azimuth & focdist are not affected by 
		unsetting keys. Altitude could still be bogus if there is an alt
		key frame and altitude is selected as active param
		(either focus or camera can be affected the same way). */

      while (Event.Code != SELECTUP && Event.Code != MENUUP)
       {
       IA->shiftx = IA->fixX ? 0: Event.MouseX - IA->startx;
       if(IA->button == SELECTDOWN)
       	IA->shifty = IA->fixY ? 0: Event.MouseY - IA->starty;
       else /* MENUDOWN */
       	IA->shifty = IA->fixZ ? 0: Event.MouseY - IA->starty;
       IA->startx = Event.MouseX;
       IA->starty = Event.MouseY;
       if (! IA->shiftx && ! IA->shifty) goto EndLoop;
       if (IA->gridpresent)
        {
        DrawInterFresh(0);
	} /* if grid present */

       if ((item[0] == 2 && ! AltKey) || (item[0] == 5 && AltKey))
        {
        if (! IA_Movement)
         {
         modifycampt(IA->shiftx, IA->shifty, IA->button);
	 } /* if radial camera motion */
        else
         {
         if (IA->button == MENUDOWN) IA->shifty *= -1;
         modifyinteractive(IA->shiftx, IA->shifty, IA->button);
         setcompass(0); 
	 } /* else rectangular camera motion */
        } /* if camera motion */
       else if (item[0] == 5 || item[0] == 2)
        {
        if (! IA_Movement)
         {
         modifyfocpt(IA->shiftx, IA->shifty, IA->button);
	 } /* if radial focus motion */
        else
         {
         if (IA->button == MENUDOWN) IA->shifty *= -1;
         modifyinteractive(IA->shiftx, IA->shifty, IA->button);
         setcompass(1); 
	 } /* else rectangular focus motion */
        } /* else if focus motion */
       else
        {
        modifyinteractive(IA->shiftx, IA->shifty, IA->button);
        boundscheck(item[0]);
        boundscheck(item[1]);
        boundscheck(item[2]);
        } /* else not camera or focus motion */

       constructview();
       if (GridBounds)
        {
        drawgridpts(1);
        if (item[0] == 5)
         {
         findfocprof();
         } /* if modify focus point */
        drawgridpts(0);
	}
       if(BoxBounds)
        {
        drawquick(1, 1, 1, 0);
        computequick();
        drawquick(7, 6, 3, 0);
        } /* if */ 
       if (ProfileBounds && ! GridBounds)
        {
        drawfocprof(1);
        if ((item[0] == 5 && ! AltKey) || (item[0] == 2 && AltKey))
         {
         findfocprof();
         } /* if modify focus point */
        drawfocprof(0);
        } /* if */
       if(LandBounds)
        {
        drawveryquick(1);
        } /* if */
       if (CompassBounds)
        {
        makeviewcenter(1);
        } /* if */
       Update_EM_Item();

EndLoop:
       FetchEvent(InterWind0, &Event);

       } /* while Event.Class != MOUSEBUTTONS */

      ModifyIDCMP(InterWind0, MOUSEBUTTONS | RAWKEY | VANILLAKEY | NEWSIZE |
	 CLOSEWINDOW | ACTIVEWINDOW);
/*
      if (MP && MP->ptsdrawn)
       {
       short startX;
       struct clipbounds cb;

       SetPointer(InterWind0, WaitPointer, 16, 16, -6, 0);
       setclipbounds(MapWind0, &cb);
       SetDrMd(MapWind0->RPort, COMPLEMENT);
       SetWrMsk(MapWind0->RPort, MAP_INTER_MASK);
       startX = latscalefactor * PAR_FIRST_MOTION(20) * VERTPIX / mapscale;
       ShowCenters_Map(&cb, startX, 0, 3);
       startX = latscalefactor * (PAR_FIRST_MOTION(20) + PAR_FIRST_MOTION(21))
		* VERTPIX / mapscale;
       ShowCenters_Map(&cb, startX, 0, 4);
       SetDrMd(MapWind0->RPort, JAM1);
       SetWrMsk(MapWind0->RPort, 0x0f);
       ClearPointer(InterWind0);
       } // if map interactive
*/

/* draw terrain grid */
      if (IA_AutoDraw)
       {
       drawgridview();
       } /* if */

      break;
      } /* SELECTDOWN | MENUDOWN */
     } /* switch Event.Code */
    break;
    } /* MOUSEBUTTONS */
   } /* switch Event.Class */

  if (abort) break;

  } /* while QuickFetchEvent() */

} /* modifyintermenu() */

/*************************************************************************/

void Handle_InterWind2(void)
{
 short abort = 0;

 while (QuickFetchEvent(InterWind2, &Event))
  {
  switch (Event.Class)
   {
   case ACTIVEWINDOW:
    {
/*    WindowToFront(InterWind2);*/
    break;
    } /* activate */

   case CLOSEWINDOW:
    {
    IA_CompTop = InterWind2->TopEdge;
    IA_CompLeft = InterWind2->LeftEdge;
    IA_CompWidth = InterWind2->Width;
    IA_CompHeight = InterWind2->Height;
    WCS_Signals ^= InterWind2_Sig;
    closesharedwindow(InterWind2, 0);
    InterWind2 = NULL;
    InterWind2_Sig = NULL;
    abort = 1;
    break;
    } /* CLOSEWINDOW */

   case NEWSIZE:
    {
    struct clipbounds cb;

    setclipbounds(InterWind2, &cb);
    IA->CompRadius = (min(cb.highx - cb.lowx, cb.highy - cb.lowy) - 12) / 2;
    IA->CompHCenter = (cb.lowx + cb.highx) / 2;
    IA->CompVCenter = (cb.lowy + cb.highy) / 2;
    SetAPen(InterWind2->RPort, 0);
    RectFill(InterWind2->RPort, cb.lowx, cb.lowy, cb.highx, cb.highy);
    SetAPen(InterWind2->RPort, 4);
    DrawEllipse(InterWind2->RPort, IA->CompHCenter + 1, IA->CompVCenter + 1,
	 IA->CompRadius + 2, IA->CompRadius + 2);
    SetAPen(InterWind2->RPort, 2);
    DrawEllipse(InterWind2->RPort,
	 IA->CompHCenter, IA->CompVCenter,
	 IA->CompRadius + 2, IA->CompRadius + 2);
    compass(azimuth, azimuth);
    break;
    } /* NEWSIZE */

   } /* switch Event.Class */

  if (abort) break;

  } /* while events are happening */

} /* Handle CompassEvent */

/*************************************************************************/

short OpenNewIAGridSize(void)
{

 closeviewmaps(1);

 return (interactiveview(0));

} /* OpenNewIAGridSize() */
