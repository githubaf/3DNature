/* Map.c (ne gismap.c 14 Jan 1994 CXH)
** 2-D Mapping routines for World Construction Set.
** Programmed by Gary R. Huber in June and July, 1992.
** Modified and incorporated into WCS in November, 1993 by GRH.
** Rewrote makemap on 11 Jan 1995 -CXH
*/

#include "WCS.h"
#include "GUIDefines.h"


#define FAST_MAPDRAW_WPA8
#define DITHERSIZE 4096
#define MED_HACK
/*
#define EL_HIST
*/
#define EMBOSS_SCALE 18
#define READ_BUFFER_SIZE_BITPAD 13
#define READ_BUFFER_SIZE_SUMMAGRID 22
#define MAX_READ_BUFFER_SIZE 22

/* These are private functions, and don't need to be in proto.h,
** but probably should be someday */

/* Rounds number a up to the next multiple of b */
#ifndef ROUNDUP
#define ROUNDUP(a,b)		(((a + (b - 1)) / b) * b)
#endif

STATIC_VAR ULONG MapWind3_Sig;
STATIC_VAR double MP_DigLatScale, MP_DigLonScale,MP_Nlat, MP_Wlon,MP_Rotate, MP_ORx, MP_ORy;

STATIC_FCN short interpolatepts(short j, long x2, long x1, long y2, long y1); // used locally only -> static, AF 19.7.2021
STATIC_FCN void EnsureLoaded(void); // used locally only -> static, AF 23.7.2021
STATIC_FCN void TempRas_Del(struct RastPort *This, int X, int Y); // used locally only -> static, AF 23.7.2021
STATIC_FCN struct RastPort *TempRas_New(struct RastPort *Ancestor, int X, int Y); // used locally only -> static, AF 23.7.2021
STATIC_FCN void RotatePt(double rotate, double *OriginX, double *OriginY,
                double *PointX, double *PointY); // used locally only -> static, AF 23.7.2021
STATIC_FCN short SearchViewLine(short viewpt, short fpx, short fpy,
        short elfp, short elvp); // used locally only -> static, AF 23.7.2021
STATIC_FCN short Set_Eco_Color(long Lra, long Lca, short i, short *RelEl); // used locally only -> static, AF 23.7.2021
STATIC_FCN void MapRelel_Free(struct elmapheaderV101 *This, short MapNum); // used locally only -> static, AF 23.7.2021
STATIC_FCN short CalcRefine(double LowLon, double HighLon, double LowLat, double HighLat,
        short *LocalLowEl, short *LocalHighEl, float *ElevRng); // used locally only -> static, AF 23.7.2021
STATIC_FCN USHORT MapRelel_Load(struct elmapheaderV101 *This, short MapNum); // used locally only -> static, AF 23.7.2021
STATIC_FCN short InputTabletPoints(long lowj, short TabletType); // used locally only -> static, AF 23.7.2021



void alignmap(struct Box *Bx)
{
 short error = 0;

 if (rlat[1] >= rlat[0]) error = 1;
 else if (rlon[1] >= rlon[0]) error = 1;
 else if (mapscale <= 0.0) error = 1;
 if (error)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Align",
          (CONST_STRPTR)"First set of alignment lat/lon coordinates must be larger than second and map scale must be greater than zero!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if */

StartAlign:
 MapGUI_Message(0, "\0338Set northwest reference point with mouse.");
 SetWindowTitles(MapWind0, (STRPTR) "Set northwest reference point", (UBYTE *)-1);

 if (! MousePtSet(&Bx->Low, NULL, 0))
  {
  align = 0;
  goto EndAlign;
  } /* if aborted */

 MapGUI_Message(0, "\0338Set southeast reference point. ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Set southeast reference point", (UBYTE *)-1);

 if (! MousePtSet(&Bx->High, &Bx->Low, 2))
  {
  align = 0;
  goto EndAlign;
  } /* if aborted */

 if (Bx->Low.X == Bx->High.X || Bx->Low.Y == Bx->High.Y)
  {
  if (User_Message_Def((CONST_STRPTR)"Mapping Module: Align",
          (CONST_STRPTR)"Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?",
          (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
   {
   goto StartAlign;
   } /* if try again */
  align = 0;
  goto EndAlign;
  } /* illegal values */

 if (Bx->Low.X > Bx->High.X)
  swmem(&Bx->Low.X, &Bx->High.X, sizeof (short));
 if (Bx->Low.Y > Bx->High.Y)
  swmem(&Bx->Low.Y, &Bx->High.Y, sizeof (short));

 SetAPen(MapWind0->RPort, 7);
 Move(MapWind0->RPort, Bx->Low.X,  Bx->Low.Y);
 Draw(MapWind0->RPort, Bx->High.X, Bx->Low.Y);
 Draw(MapWind0->RPort, Bx->High.X, Bx->High.Y);
 Draw(MapWind0->RPort, Bx->Low.X,  Bx->High.Y);
 Draw(MapWind0->RPort, Bx->Low.X,  Bx->Low.Y);

EndAlign:
 MapGUI_Message(0, " ");
 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

} /* alignmap() */

/************************************************************************/

void makemap(struct Window *win, long lowx, long lowy, long highx, long highy,
 unsigned long int FlagBits)
{
/* Note: Don't call with both Eco and FlatSpots turned on */

float FloatColor, FColorDif, GradFact, MBossScale, ElevColor, LCFac,
 LRFac, ElevRng;
long PixWidth, PixArraySize, *ATVCT, ATVCTSize, SpareLc;
short HorizPix, VertPix, RightHoriz, LeftHoriz, HorizOffset, LocalHighEl = -30000,
 LocalLowEl = 30000, WithinBounds, ElevDiff, DrawSteps, *Sample,
 *OffsetSample, *AltSample, *RowVect, *LastRowVect, SampEl, MapInc, UserAbort,
 GapFill, NeedTopo, NeedEco, ArraysGood, *TopoArray, *EcoArray, *Column,
 Refine, TopoEnabled, EcoEnabled, FlatSpots, El1, El2, El3, MaxLRFac, MaxLCFac;
unsigned short DMod;
unsigned char *PixArray;
struct RastPort *TempRast;
struct BusyWindow *BusyWin;
struct elmapheaderV101 RelElHdr;
struct clipbounds Clip;


Refine      = FlagBits & MMF_REFINE;
TopoEnabled = FlagBits & MMF_TOPO;
EcoEnabled  = FlagBits & MMF_ECO;
FlatSpots   = FlagBits & MMF_FLATSPOTS;

/* if(FlatSpots)
 {
 Status_Log("DBG: FlatSpots is on.", 0);
 } */ /* if */
 
if (lowx > highx)
 {
 Clip.lowx  = highx;
 Clip.highx = lowx;
 } /* if */
else
 {
 Clip.lowx  = lowx;
 Clip.highx = highx;
 } /* else */
if (lowy > highy)
 {
 Clip.lowy  = highy;
 Clip.highy = lowy;
 } /* if */
else
 {
 Clip.lowy  = lowy;
 Clip.highy = highy;
 } /* else */

if (EcoEnabled && !paramsloaded)
 { /* <<<>>> Maybe we should make this have three choices: Cancel, Load, Default */
 User_Message((CONST_STRPTR)"Map View: Ecosystems",
         (CONST_STRPTR)"There are no Parameters loaded! Ecosystem mapping is not available until you \
load a Parameter file or create Default Parameters.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
 set(MP->MapEco, MUIA_Selected, FALSE);
 EcoEnabled = 0;
 } /* if no parameter file */

SetPointer(win, WaitPointer, 16, 16, -6, 0);
if(TopoEnabled || EcoEnabled)
 {
 EnsureLoaded();
 } /* if */

DrawSteps = 0;
if((TopoEnabled || EcoEnabled) && topoload)
 {
 DrawSteps += (topomaps * 10);
 } /* if */
if(vectorenabled)
 {
 DrawSteps += NoOfObjects;
 } /* if */
BusyWin = BusyWin_New("Map Draw", DrawSteps, 0, MakeID('M','A','P','D'));
DMod = UserAbort = DrawSteps = 0;

if(TopoEnabled || EcoEnabled)
 {
 if(topoload)
  {
  PixWidth = ((Clip.highx - Clip.lowx) + 1);

  if((TempRast = TempRas_New(MapWind0->RPort, ROUNDUP(PixWidth, 16) * 2, 1)))
   {
   PixArraySize = ROUNDUP(PixWidth, 16);
   if((PixArray = (UBYTE *)get_Memory(PixArraySize, MEMF_CLEAR)))
    {
    memset(PixArray, backpen, PixArraySize);

    if (EcoEnabled)
     {
     if (EE_Win)
      {
      frame = EE_Win->Frame;
      } /* if */
     else
      {
      frame = 1;
      } /* else */
     initpar();
     initmopar();
     setvalues();
     render = 1;
     } /* if EcoEnabled */

#ifdef EL_HIST
     UserAbort = CalcRefine(X_Lon_Convert(Clip.highx),
      X_Lon_Convert(Clip.lowx),
      Y_Lat_Convert(Clip.highy),
      Y_Lat_Convert(Clip.lowy),
      &LocalLowEl, &LocalHighEl, &ElevRng);

#else
    if (Refine && (! ContInt)) /* Not important to recalc hilo vals for anything but single grad  */
     {
     UserAbort = CalcRefine(X_Lon_Convert(Clip.highx),
      X_Lon_Convert(Clip.lowx),
      Y_Lat_Convert(Clip.highy),
      Y_Lat_Convert(Clip.lowy),
      &LocalLowEl, &LocalHighEl, &ElevRng);
     } /* if */    
    else
     {
     LocalLowEl = MapLowEl;
     ElevRng = ((MapHighEl - MapLowEl) / 254.99);
     } /* if topo refine */
#endif
    
    /* Here's where stuff beings to happen */
    
    for(MapInc = 0; ((MapInc < topomaps) && (!UserAbort)); MapInc++)
     {
     if(BusyWin)
      {
      BusyWin_Update(BusyWin, DrawSteps);
      DrawSteps += 10;
      } /* if BusyWin */
     
     if(DBase[TopoOBN[MapInc]].Lat)
      {
      latlon_XY(TopoOBN[MapInc]);
      /* compute various per-map constants */
      /* Note: mapnn[1] seems to be upper right,
      **       mapnn[2] seems to be upper left
      */
      LCFac = ((float)(mapelmap[MapInc].columns - 1))
       		/ ((float)(max(mapyy[1] - mapyy[4], 1))); /* columns per pixel */
      LRFac = ((float)(mapelmap[MapInc].rows))
       		/ ((float)(max(mapxx[1] - mapxx[2], 1))); /* rows per pixel */
      MaxLCFac = max((int)(LCFac + .5), 1);
      MaxLRFac = max((int)(LRFac + .5), 1);
      /* MinVert is generated and used inline below */
      /* MinVert = min(mapyy[1], Clip.highy); */
      RightHoriz = min(mapxx[1], Clip.highx); /* Will most likely be bigger than LeftHoriz */
      LeftHoriz = max(mapxx[2], Clip.lowx); /* smaller than RightHoriz */
      MBossScale = MaxElevDiff * (mapelmap[MapInc].steplat / .0008333333);
      
      /* At this point, we should know the horizontal pixel
      ** size, and can calculate the address translation vector
      ** cache table */
      
      ATVCTSize = (RightHoriz - LeftHoriz) + 1;
      if(ATVCTSize <= 0)
       { /* This should throw out topos that are off the left or right edge */
       continue; /* Skip this... */
       } /* if */
      

      if((mapyy[1] < MapWind0->BorderTop && mapyy[3] < MapWind0->BorderTop) ||
       (mapyy[1] > (MapWind0->Height - MapWind0->BorderBottom - 1) &&
       (mapyy[3] > MapWind0->Height - MapWind0->BorderBottom - 1)))
       { /* Discriminate out topos above or below window. */
         /* Mostly speeds up by preventing loading of unused relels */
       continue;
       } /* if */
      
/* load relel file if ecoenabled */
      if(EcoEnabled)
       {
       EcoEnabled = MapRelel_Load(&RelElHdr, MapInc);
       } /* if EcoEnabled */      

      ArraysGood = 1;

      if(TopoEnabled || FlatSpots)
       {
       if((TopoArray = (/*unsigned*/ short *)get_Memory((mapelmap[MapInc].columns + 1) * sizeof(short), MEMF_CLEAR)))
        {
        memset(TopoArray, 0xFF, mapelmap[MapInc].columns * sizeof(short));
        } /* if */
       else
        {
        ArraysGood = 0;
        } /* else */
       } /* if TopoEnabled */
      else
       {
       TopoArray = NULL;
       } /* else */
  
      if(EcoEnabled)
       {
       if((EcoArray = (/*unsigned*/ short *)get_Memory((mapelmap[MapInc].columns + 1) * sizeof(short), MEMF_CLEAR)))
        {
        memset(EcoArray, 0xFF, mapelmap[MapInc].columns * sizeof(short));
        } /* if */
       else
        {
        ArraysGood = 0;
        } /* else */
       } /* if Eco */
      else
       {
       EcoArray = NULL;
       } /* else */

      if((ATVCT = get_Memory(sizeof(long) * ATVCTSize, MEMF_CLEAR)))
       {
       if((Column = get_Memory(sizeof(unsigned short) * ATVCTSize, MEMF_CLEAR)))
        {
        /* Initialize the ATVCT */
        for(HorizPix = LeftHoriz; HorizPix <= RightHoriz; HorizPix++)
         {
         /* Historical note: I found a way around it. -CXH */
         /* We used to do the following for _every_ map pixel we drew,
         ** now we do it once for every column in each map we draw. */
         Column[HorizPix - LeftHoriz] = (short)(((float)(HorizPix - mapxx[2]) / (float)(mapxx[1] - mapxx[2])) * (float)(mapelmap[MapInc].columns));
         ATVCT[HorizPix - LeftHoriz] = (long)(((long)(LRFac * (HorizPix - mapxx[2]))) * mapelmap[MapInc].columns);
         } /* for */
        } /* if Column */
       else
        {
        ArraysGood = 0;
        } /* else */
       } /* if ATVCT */
      else
       {
       ArraysGood = 0;
       } /* else */
       
      if(ArraysGood)
       {
       VertPix = max(mapyy[4], Clip.lowy); /*v used to be MinVert v */
       for(LastRowVect = NULL;((VertPix <= min(mapyy[1], Clip.highy)) && !UserAbort); VertPix++)
        {
        if(EcoEnabled)
         {
         SpareLc = (LCFac * (VertPix - mapyy[4]));
         } /* if */
        RowVect = (short *)(((short)(LCFac * (mapyy[1] - VertPix))) + (mapelmap[MapInc].map));
        
        /* check for copy-forward (GapFill) optimization here. */
        WithinBounds = GapFill = (LastRowVect == RowVect); /* Makes sense */
        
        for(HorizPix = LeftHoriz; HorizPix <= RightHoriz; HorizPix++)
         {
         NeedTopo = TopoEnabled;
         NeedEco = EcoEnabled;

         HorizOffset = HorizPix - LeftHoriz;
         
         if(NeedTopo && NeedEco)
          {
          NeedTopo = !(NeedEco = (HorizOffset + VertPix + 2) % 2);
          } /* if */
         
         if(GapFill)
          {
          if(NeedTopo)
           {
           if(TopoArray[Column[HorizOffset]] != 0xFFFF)
            {
            NeedTopo = 0;
            } /* if */
           } /* if */
          if(NeedEco)
           {
           if(EcoArray[Column[HorizOffset]] != 0xFFFF)
            {
            NeedEco = 0;
            } /* if */
           } /* if */
          } /* if */
         else
          {
          WithinBounds = 1;
          } /* else */
         
         if(NeedTopo || NeedEco)
          {
          Sample = RowVect + ATVCT[HorizOffset]; /* Rock _ON_! */
          } /* if */
         
         SampEl = *Sample * mapelmap[MapInc].elscale * 1000.0;
         if (FlatSpots && SampEl == -30000)
          {
          TopoArray[Column[HorizOffset]] = -30000;
	  }
         else
          {
          if(NeedEco)
           {
           /* <<<>>> I think maybe Set_Eco_Color could be optimized to use */
           /* similar techniques, but I don't understand it well enough to */
           /* do it myself. The calculation of Lr and SpareLc are probably not */
           /* optimal. */
           if((EcoArray[Column[HorizOffset]] =
            Set_Eco_Color((long)(LRFac * (HorizPix - mapxx[2])),
				 mapelmap[MapInc].columns - SpareLc - 1,
				 MapInc, RelElHdr.map)) == 1)
            {
            if(TopoEnabled)
             {
             NeedTopo = 1; /* See through ecosystem */
             } /* if */
            } /* if */
           } /* if */
          if(NeedTopo)
           { /* have to compute it */
           switch(ContInt)
            {
            case 0: /* single gradient */
             {
             TopoArray[Column[HorizOffset]] = 254.99 + ROUNDING_KLUDGE - (SampEl - LocalLowEl) / ElevRng;
             break;
             } /* single gradient */
            case 1: /* repeating gradients */
             {
             FloatColor = SampEl % ContInterval;
             if(FloatColor < 0)
              {
              FloatColor += ContInterval;
              } /* if */
             TopoArray[Column[HorizOffset]] = 254.99 - 254.99 * FloatColor / (float)ContInterval;
             break;
             } /* repeating gradients */
            case 2:
             {
             if (SampEl < settings.surfel[1])
              {
              GradFact = ((double)settings.surfel[1] - SampEl) / (settings.surfel[1] - settings.surfel[0]);
              } /* low gradient */
             else if (SampEl >= settings.surfel[1] && SampEl < settings.surfel[2])
              {
              GradFact = ((double)settings.surfel[2] - SampEl) / (settings.surfel[2] - settings.surfel[1]);
              } /* else if middle gradient */
             else
              {
              GradFact = ((double)settings.surfel[3] - SampEl) / (settings.surfel[3] - settings.surfel[2]);
              } /* else upper gradient */
             if (GradFact > 1.0)
              {
              GradFact = 1.0;
              } /* if */
             else if (GradFact < 0.0)
              {
              GradFact = 0.0;
              } /* else if */
             TopoArray[Column[HorizOffset]] = 254.99 * GradFact + ROUNDING_KLUDGE;
             break;
             } /* surface elevations */
            case 3: /* emboss */
             {
             ElevColor = 254.99 + ROUNDING_KLUDGE - (SampEl - LocalLowEl) / ElevRng;
             } /* emboss falls through to below */
            case 4: /* slope */
             {
             if (!(HorizPix - mapxx[2]))
              {
              if (! Column[HorizOffset])
               {
               OffsetSample = Sample;
               } /* if last column */
              else
               {
               OffsetSample = Sample + 1;
               } /* else not last column */
              } /* if first row */
             else
              {
              if (! Column[HorizOffset])
               {
               OffsetSample = Sample - mapelmap[MapInc].columns;
               } /* if last column */
              else
               {
               OffsetSample = Sample - mapelmap[MapInc].columns + 1;
               } /* else not last column */
              } /* else not first row */
             ElevDiff = (*OffsetSample * mapelmap[MapInc].elscale * 1000.0) - SampEl;
             if(ElevDiff > MBossScale)
              {
              ElevDiff = MBossScale;
              } /* if */
             else if (ElevDiff < -MBossScale)
              {
              ElevDiff = -MBossScale;
              } /* else if */
             if(ContInt == 3)
              {
              TopoArray[Column[HorizOffset]] =
               (ElevColor + (128.0 + 127.99 * ((float)ElevDiff / MBossScale))) / 2.0;
              } /* if */
             else
              {
              TopoArray[Column[HorizOffset]] =
               254.99 - fabs(254.99 * ((float)ElevDiff / MBossScale));
              } /* else */
             break;
             } /* embossed and slope */
            case 5:
             {
             if (((int)((HorizPix - mapxx[2]) * MaxLRFac)) <= 0)
              {
              OffsetSample = Sample;
              } /* if first row */
             else
              {
              OffsetSample = Sample - MaxLRFac * mapelmap[MapInc].columns;
              } /* else not first row */
             if (((int)(VertPix - mapyy[4]) * LCFac - .5) <= 0)
              {
              AltSample = Sample;
              } /* if last column */
             else
              {
              AltSample = Sample + MaxLCFac;
              } /* else not last column */

 	     El1 = SampEl / ContInterval;
             El2 = (Sample == OffsetSample) ? El1: ((int)(*OffsetSample * mapelmap[MapInc].elscale * 1000.0)) / ContInterval;
             El3 = (Sample == AltSample) ?    El1: ((int)(*AltSample * mapelmap[MapInc].elscale * 1000.0)) / ContInterval;
             TopoArray[Column[HorizOffset]] = (El1 == El2 && El1 == El3) ? 254.99: 0.0;
             break;
	     } /* contour */
            } /* switch(ContInt) */
           } /* if */
	  } /* else not flat spot */
         } /* for HorizPix */
        
        if(WithinBounds)
         {
         for(HorizPix = 0; HorizPix <= (RightHoriz - LeftHoriz); HorizPix++)
          {
          if (FlatSpots && TopoArray[Column[HorizPix]] == -30000)
           {
           PixArray[HorizPix] = 3; /* red */
	   }
          else
           {
           if(TopoEnabled && EcoEnabled)
            {
            if(((HorizPix - LeftHoriz) + VertPix + 2) % 2)
             { /* Punch in the eco pixel */
             NeedEco = 1;
             } /* if */
            else
             { /* punch in a topo pixel */
             NeedEco = 0;
             } /* else */
            } /* if */
           else
            {
            if(TopoEnabled)
             {
             NeedEco = 0;
             } /* if */
            else
             { /* EcoEnabled */
             NeedEco = 1;
             } /* else */
            } /* if */
          
           if(NeedEco)
            { /* put in eco pixel */
            if((PixArray[HorizPix] = EcoArray[Column[HorizPix]]) == 1)
             {
             if(TopoEnabled)
              {
              NeedEco = 0;
              } /* if */
             } /* if */
            } /* if */
           if(!NeedEco) /* Not exactly an else, since NeedEco may change above */
            {
            if(MapDither)
             { /* Put in dithered topo pixel */
             FloatColor = (float)TopoArray[Column[HorizPix]] / (float)32;
             PixArray[HorizPix] = (short)FloatColor;
             FColorDif = FloatColor - ROUNDING_KLUDGE - PixArray[HorizPix];
             if(PixArray[HorizPix] < 7)
              {
              if(FColorDif > DTable[(DMod++%DITHERSIZE)])
               {
               PixArray[HorizPix]++;
               } /* if */
              } /* if */
             PixArray[HorizPix] += 8;
             } /* if */
            else
             {
             PixArray[HorizPix] = 8 + TopoArray[Column[HorizPix]] / 32;
             } /* else */
            } /* if !NeedEco */
 	   } /* else not flat spot */         
	  } /* for HorizPix... */
         WritePixelLine8(MapWind0->RPort, LeftHoriz, VertPix, ATVCTSize, PixArray, TempRast);
         if(BusyWin)
          {
          if(CheckInput_ID() == ID_BW_CLOSE)
           {
           UserAbort = 1;
           } /* if */
          } /* if */
         } /* if */
        
        LastRowVect = RowVect;
        } /* for VertPix */
       
       
       } /* if ArraysGood */
      else
       {
       UserAbort += (!User_Message_Def((CONST_STRPTR)"Map View: Topo Draw",
               (CONST_STRPTR)"Memory allocation failure, cannot draw topo. Continue?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1));
       } /* else */
       if(mapelmap[MapInc].lmap) /* Even if Eco isn't enabled */
       { /* unload the relel file */
       MapRelel_Free(&RelElHdr, MapInc);
       } /* if */
      
      /* Free stuff that might have allocated */
      if(Column)
       {
       free_Memory(Column, sizeof(unsigned short) * ATVCTSize);
       Column = NULL;
       } /* if */
      if(ATVCT)
       {
       free_Memory(ATVCT, sizeof(long) * ATVCTSize);
       /* ATVCT = NULL; */ /* optimizer says currently unnecessary */
       } /* if */
      if(EcoArray)
       {
       free_Memory(EcoArray, (mapelmap[MapInc].columns + 1) * sizeof(short));
       EcoArray = NULL;
       } /* if */
      if(TopoArray)
       {
       free_Memory(TopoArray, (mapelmap[MapInc].columns + 1) * sizeof(short));
       TopoArray = NULL;
       } /* if */
      } /* if Object's Lat */
     } /* for MapInc < topomaps */
    free_Memory(PixArray, PixArraySize);
    } /* if PixArray */
   else
    {
    /* <<<>>> Warn: No PixArray, No Draw */
    } /* else !PixArray */
   TempRas_Del(TempRast, ROUNDUP(PixWidth, 16) * 2, 1);
   } /* if TempRast */
  else
   {
   /* <<<>>> Warn: Couldn't draw, no TempRast */
   } /* else !TempRast */
  } /* if topoload */
 else
  {
  /* <<<>>> Perhaps we should warn the user that the maps didn't load */
  } /* else !topoload */
 } /* if Topo || Eco */

/* map vector objects */

if(vectorenabled && (UserAbort == 0))
 {
 for (MapInc = 0; (MapInc < NoOfObjects)  && (UserAbort == 0); MapInc++)
  {
  if(BusyWin)
   {
   BusyWin_Update(BusyWin, DrawSteps++);
   } /* if */
  outline(win, MapInc, DBase[MapInc].Color, &Clip);
  if(BusyWin)
  	{
  	if(CheckInput_ID() == ID_BW_CLOSE)
  	 {
  	 UserAbort = 1;
  	 } /* if */
  	} /* if */
  } /* for MapInc = 0... */
 } /* if */

latlon_XY(OBN);

if(BusyWin)
 {
 BusyWin_Del(BusyWin);
 } /* if */
ClearPointer(win);

} /* makemap() */

/************************************************************************/

STATIC_FCN void EnsureLoaded(void) // used locally only -> static, AF 23.7.2021
{
short i, Obj, Topo, NeedIt;

if(topoload == 0)
 {
 loadtopo();
 } /* if */
else
 { /* else check to see if DEMs have been enabled or disabled */
 for(Obj = 0; Obj < NoOfObjects; Obj++)
  {
  if(!strcmp(DBase[Obj].Special, "TOP") || !strcmp(DBase[Obj].Special, "SFC"))
   {
   if(DBase[Obj].Flags & 2)
    {
    NeedIt = 1;
    } /* if */
   else
    {
    NeedIt = 0;
    } /* else */
   for(Topo = 0; Topo < topomaps; Topo++)
    {
    if(Obj == TopoOBN[Topo])
     {
     NeedIt = 0;
     if(!(DBase[Obj].Flags & 2))
      {
      /* disabled, unload it right here instead of calling loadtopo */
      /* <<<>>>Note: Doing it this way means MapElHigh and MapElLow */
      /* are _not_ recalculated, but I think it's worth it. */
      if (mapelmap)
       {
       if (mapelmap[Topo].map)
        {
        free_Memory(mapelmap[Topo].map, mapelmap[Topo].size);
        mapelmap[Topo].map = NULL;
        mapelmap[Topo].size = 0;
/* Need to rearrange the rest of the map structures so there aren't any gaps
   There isn't any checking to see if every pointer is valid since by definition
   when the structures are allocated they are all valid up to the number of
   topomaps loaded. */
        for (i=Topo; i<topomaps-1; i++)
         {
         memcpy(&mapelmap[i], &mapelmap[i + 1], sizeof (struct elmapheaderV101));
         memcpy(&mapcoords[i], &mapcoords[i + 1], sizeof (struct MapCoords));
         TopoOBN[i] = TopoOBN[i + 1];
	 } /* for i=... */
        topomaps --;
        } /* if */
       } /* if mapelmap */
      } /* else disabled */
     } /* if match found */
    } /* for Topo = 0... */
   if(NeedIt)
    {
    loadtopo(); /* Dump everything and reload: _Not_ optimal. */
    return;
    } /* else not already loaded */
   } /* if DEM */
  } /* for Obj = 0... */
 } /* else check to see if DEMs have been enabled or disabled */

} /* EnsureLoaded() */

/***************************************************************************/

STATIC_FCN USHORT MapRelel_Load(struct elmapheaderV101 *This, short MapNum) // used locally only -> static, AF 23.7.2021
{
long ct, Row, Col, OpenOK;
struct DirList *DLItem;
static char filename[256]; /* <<<>>> to keep it off the stack */
static char elevpath[256], elevfile[32];
FILE *felev;

 if(mapelmap[MapNum].lmap == NULL)
  {
  if((mapelmap[MapNum].lmap = (long *)get_Memory(mapelmap[MapNum].size * 2, MEMF_ANY)))
   {
   ct = 0;
   for(Row = 0; Row <= mapelmap[MapNum].rows; Row++)
    {
    for(Col = 0; Col < mapelmap[MapNum].columns; Col++)
     {
     mapelmap[MapNum].lmap[ct] = (mapelmap[MapNum].map[ct] + 
	     (PARC_RNDR_MOTION(13) - mapelmap[MapNum].map[ct]) *
	     PARC_RNDR_MOTION(12)) * mapelmap[MapNum].elscale / ELSCALE_METERS;
     ct ++;
     } /* for Col = 0... */
    } /* for Row = 0... */
   } /* if lmap allocated */
  else
   {
   return (User_Message((CONST_STRPTR)"Map View: Ecosystems",
           (CONST_STRPTR)"Out of memory loading Relative Elevation file. Ecosystem mapping not available?",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o"));
   } /* else no memory for lmap */
  } /* if lmap not already allocated, Don't know why it already would be but... */

 for (DLItem = DL; DLItem;)
  {
  strmfp(filename, DLItem->Name, DBase[TopoOBN[MapNum]].Name);
  strcat(filename, ".relel");
  if(readDEM(filename, This))
   {
   DLItem = DLItem->Next;
   } /* if open fail */
  else
   {
   return(1);
   } /* else open OK */
  } /* for */

/* Couldn't find a relel file, try making one */ 

/* First figure out what directory the elev file is in so the relel file can go
   there too */

 OpenOK = 0;

 for (DLItem=DL; DLItem; )
  {
  strmfp(filename, DLItem->Name, DBase[TopoOBN[MapNum]].Name);
  strcat(filename, ".elev");
  if ((felev = fopen(filename, "r")) == NULL)
   {
   DLItem = DLItem->Next;
   } /* if open fail */
  else
   {
   OpenOK = 1;
   fclose(felev);
   strcpy(elevpath, DLItem->Name);
   strcpy(elevfile, DBase[TopoOBN[MapNum]].Name);
   strcat(elevfile, ".elev");
   break;
   } /* else open OK */
  } /* for */

 if (! OpenOK)
  {
  strcpy(elevpath, dirname);
  strcpy(elevfile, DBase[TopoOBN[MapNum]].Name);
  strcat(elevfile, ".elev");
  } /* if no elev file found - maybe directory list changed by user, put relel file in default dir */
 if (makerelelfile(elevpath, elevfile))
  {
  strmfp(filename, elevpath, elevfile);	/* elevfile suffix changed to .relel */
  if (readDEM(filename, This) == 0)
   {
   return (1);
   } /* if still no relel file */
  } /* if make relel successful */

/* couldn't make a relel file either */

 Log(WNG_OPEN_FAIL, (CONST_STRPTR)"Relative elevation file");
 free_Memory(mapelmap[MapNum].lmap, mapelmap[MapNum].size * 2);
 This->map = NULL;
 mapelmap[MapNum].lmap = NULL;
 return(0);

} /* MapRelel_Load() */

/************************************************************************/

STATIC_FCN void MapRelel_Free(struct elmapheaderV101 *This, short MapNum) // used locally only -> static, AF 23.7.2021
{

if(mapelmap[MapNum].lmap)
 {
 free_Memory(mapelmap[MapNum].lmap, mapelmap[MapNum].size * 2);
 mapelmap[MapNum].lmap = NULL;
 } /* if */
if(This->map)
 {
 free_Memory(This->map, This->size);
 This->map = NULL;
 } /* if */
} /* MapRelel_Free() */

/************************************************************************/

STATIC_FCN short CalcRefine(double LowLon, double HighLon, double LowLat, double HighLat,
	short *LocalLowEl, short *LocalHighEl, float *ElevRng) // used locally only -> static, AF 23.7.2021
{ /* Broke this out into a seperate function since it only gets called
  ** under some circumstances, it uses some values and variables that
  ** are not necessary in the rest of the code, and it break the continuity
  ** of the main function. */
short MapInc;
short *Sample, SampEl;
long MapRow, MapCol;
double LatPoint, LonPoint;
#ifdef EL_HIST
short *HistTable = NULL, HistWidth, HistHeight, HistWScale, HistHScale, i;
short XC, YC;
struct Window *HistWin = NULL;
struct IntuiMessage *Spin = NULL;
HistTable = AllocMem(65600 * 2, MEMF_CLEAR);

#endif /* EL_HIST */

for (MapInc = 0; MapInc < topomaps; MapInc++)
 {
 if (CheckInput_ID() == ID_BW_CLOSE)
  {
  return(1);
  } /* if user abort */
 Sample = mapelmap[MapInc].map;
 for (MapRow = 0; MapRow <= mapelmap[MapInc].rows; MapRow++)
  {
  LonPoint = mapcoords[MapInc].C[1] - MapRow * mapelmap[MapInc].steplong;
  if (LonPoint > HighLon)
   {
   Sample += mapelmap[MapInc].columns;
   continue;
   }
  if (LonPoint < LowLon)
   {
   break;
   } /* if */
  for (MapCol = 0; MapCol < mapelmap[MapInc].columns; MapCol++)
   {
   LatPoint = mapcoords[MapInc].C[0] + MapCol * mapelmap[MapInc].steplat;
   if (LatPoint < LowLat)
    {
    Sample ++;
    continue;
    } /* if */
   if (LatPoint > HighLat)
    {
    Sample = mapelmap[MapInc].map + (MapRow + 1) * mapelmap[MapInc].columns;
    break;
    } /* if */
   SampEl = *Sample * mapelmap[MapInc].elscale * 1000.0;
#ifdef EL_HIST
   if(HistTable)
   	{
   	HistTable[SampEl + 32768] ++;
   	} /* if */
#endif /* EL_HIST */
   if (SampEl > *LocalHighEl)  
    {
    *LocalHighEl = SampEl;
    } /* if */
   if (SampEl < *LocalLowEl) 
    {
    *LocalLowEl = SampEl;
    } /* if */
   Sample ++;
   } /* for MapCol = 0... */
  } /* for MapRow = 0... */
 } /* for i=0... */
(*LocalHighEl) ++;            /* watch out for roundoff errors */
(*LocalLowEl) --;
*ElevRng = ((*LocalHighEl - *LocalLowEl) / 254.99);

#ifdef EL_HIST
if(HistTable)
	{
	HistWScale = HistHScale = 1;
	HistWidth = HistHeight = 0;
	for(i = *LocalLowEl; i < *LocalHighEl; i++)
		{
		if(HistTable[i + 32768] > HistHeight)
			{
			HistHeight = HistTable[32768 + i];
			} /* if */
		} /* for */
	HistWidth = *LocalHighEl - *LocalLowEl;
	if(HistWidth > 640)
		{
		HistWScale = (HistWidth / 640) + 1;
		HistWidth = HistWidth / HistWScale;
		} /* if */
	if(HistHeight > 400)
		{
		HistHScale = (HistHeight / 400) + 1;
		HistHeight = HistHeight / HistHScale;
		} /* if */
	HistWin = OpenWindowTags(NULL, WA_InnerWidth, HistWidth, WA_InnerHeight,
	 HistHeight + 1, WA_IDCMP, IDCMP_CLOSEWINDOW, WA_CloseGadget, TRUE, WA_Title, "Histogram",
	 WA_SizeGadget, FALSE, WA_NoCareRefresh, TRUE, WA_SmartRefresh, TRUE,
	 WA_CustomScreen, WCSScrn, WA_DepthGadget, TRUE, WA_DragBar, TRUE,
	 WA_Activate, TRUE);
	if(HistWin)
		{
		SetAPen(HistWin->RPort, 1);
		for(i = *LocalLowEl; i < *LocalHighEl; i++)
			{
			XC = (i - *LocalLowEl) / HistWScale;
			YC = HistHeight - (HistTable[i + 32768] / HistHScale);
			Move(HistWin->RPort, HistWin->BorderLeft + XC, HistWin->BorderTop + HistHeight);
			Draw(HistWin->RPort, HistWin->BorderLeft + XC, HistWin->BorderTop + YC);
			} /* for */
		while((Spin = (struct IntuiMessage *)GetMsg(HistWin->UserPort)) == NULL)
			{
			Wait(1 << HistWin->UserPort->mp_SigBit);
			} /* while */
		ReplyMsg((struct Message *)Spin);
		CloseWindow(HistWin);
		HistWin = NULL;
		} /* if */
	FreeMem(HistTable, 65600 * 2);
	HistTable = NULL;
	} /* if */

#endif /* EL_HIST */

return(0);

} /* CalcRefine */

/************************************************************************/

short shiftmap(int OnePoint, int XCen, int YCen)
{
 short shiftx, shifty, success, HalfHeight, HalfWidth;
 double lonscale;
/* struct clipbounds cb; */
 struct Box Bx;

 if(OnePoint != 2)
  {
  if(OnePoint)
 	 {
 	 MapGUI_Message(0, "\0338Set center point.");
	 SetWindowTitles(MapWind0, (STRPTR) "Set center point", (UBYTE *)-1);
 	 } /* if */
  else
 	 {
	 MapGUI_Message(0, "\0338Set origin point.");
	 SetWindowTitles(MapWind0, (STRPTR) "Set origin point", (UBYTE *)-1);
	 } /* else */
  } /* != 2 */
 else
  { /* OnePoint == 2 */
  Bx.Low.X = XCen;
  Bx.Low.Y = YCen;
  } /* OnePoint == 2 */

 if (! (success = MousePtSet(&Bx.Low, NULL, 0)))
  goto EndShift;

 HalfHeight = (MapWind0->BorderTop + MapWind0->Height
	 - MapWind0->BorderBottom) / 2;
 HalfWidth = (MapWind0->BorderLeft + MapWind0->Width
	 - MapWind0->BorderRight) / 2;

 if(OnePoint != 0)
 	{
 	Bx.High.X = HalfWidth;
 	Bx.High.Y = HalfHeight;
 	} /* if */
 else
 	{
	MapGUI_Message(0, "\0338Set destination point. ESC=abort");
	SetWindowTitles(MapWind0, (STRPTR) "Set destination point", (UBYTE *)-1);
	
	if (! (success = MousePtSet(&Bx.High, &Bx.Low, 1)))
	 goto EndShift;
	} /* else */ 
 
 shiftx = Bx.High.X - Bx.Low.X;
 shifty = Bx.High.Y - Bx.Low.Y;
 lonzero = lonzero + shiftx * x_lon / lonscalefactor;
 latzero = latzero + shifty * y_lat / latscalefactor;
 maplat = latzero - mapscale * HalfHeight / (VERTPIX * LATSCALE); /*27.044;*/
 lonscale = LATSCALE * cos(maplat * PiOver180);
 maplon = lonzero - mapscale * HalfWidth / (HORPIX * lonscale); /*(19.657 * cos(maplat/DTOR));*/

EndShift:
 MapIDCMP_Restore(MapWind0);
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

 return (success);

} /* shiftmap() */

/************************************************************************/

short maptotopo(long OBN)
{
 short j, done = 0, elproj, topoct = 0;
 long Lr, Lc;
 float remr, remc, Elev;

 if (! DBase[OBN].Lat)
  {
  return (1);
  }
 memset (DBase[OBN].Elev, 0, DBase[OBN].VecArraySize / 4);

 for (j=0; j<=DBase[OBN].Points; j++, done=0, topoct=0)
  {
  while (! done && topoct < topomaps)
   {
   if (DBase[OBN].Lat[j] < mapcoords[topoct].C[0] || DBase[OBN].Lat[j] > mapcoords[topoct].C[2] || 
       DBase[OBN].Lon[j] > mapcoords[topoct].C[1] || DBase[OBN].Lon[j] < mapcoords[topoct].C[3]) 
    topoct ++;
   else
    done = 1;
   } /* while ! done */

  if (! done)
   continue;
  
  Lc = ((DBase[OBN].Lat[j] - mapcoords[topoct].C[0])
		/ mapelmap[topoct].steplat) + ROUNDING_KLUDGE;
  Lr = ((mapcoords[topoct].C[1] - DBase[OBN].Lon[j])
		/ mapelmap[topoct].steplong) + ROUNDING_KLUDGE;

  remc = (DBase[OBN].Lat[j] - (Lc * mapelmap[topoct].steplat
	+ mapcoords[topoct].C[0])) / mapelmap[topoct].steplat;
  remr = (mapcoords[topoct].C[1] - Lr * mapelmap[topoct].steplong
	- DBase[OBN].Lon[j]) / mapelmap[topoct].steplong;

  if (remc + remr < 1.0)
   {           /* lower right polygon */
   remc = 1.0 - remc;
   elproj = (remr / remc) *
	(*(mapelmap[topoct].map + (Lr+1) * mapelmap[topoct].columns + Lc)
	- *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc))
	+ *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc);
   Elev = remc * (elproj
	- *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc + 1))
	+ *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc + 1);
   DBase[OBN].Elev[j] = Elev * mapelmap[topoct].elscale / ELSCALE_METERS;
   } /* if lower right polygon */
  else
   { 		            /* upper left polygon */
   remc = 1.0 - remc;
   elproj = (remc / remr) *
	(*(mapelmap[topoct].map + (Lr+1) * mapelmap[topoct].columns + Lc)
	- *(mapelmap[topoct].map + (Lr+1) * mapelmap[topoct].columns + Lc + 1))
	+ *(mapelmap[topoct].map + (Lr+1) * mapelmap[topoct].columns + Lc + 1);
   Elev = remr * (elproj
	- *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc + 1))
	+ *(mapelmap[topoct].map + Lr * mapelmap[topoct].columns + Lc + 1);
   DBase[OBN].Elev[j] = Elev * mapelmap[topoct].elscale / ELSCALE_METERS;
   } /* else upper left polygon */

  } /* for j=0 to number of points */

 return(saveobject(OBN, NULL, DBase[OBN].Lon, DBase[OBN].Lat, DBase[OBN].Elev));

} /* maptotopo() */

/************************************************************************/

short findmouse(short X, short Y, short IdentifyOnly)
{
 long xlow, xhigh, ylow, yhigh;
 short i, j, done = 0, abortitem = 0, ItemFound = -1;
 struct clipbounds cb;

 setclipbounds(MapWind0, &cb);
 if (! X && ! Y)
  {
  MapGUI_Message(0, "\0338Select object. ESC=abort");
  SetWindowTitles(MapWind0, (STRPTR) "Select object", (UBYTE *)-1);
  ModifyIDCMP(MapWind0, IDCMP_MOUSEBUTTONS | IDCMP_VANILLAKEY);
  while (! done)
   {
   FetchEvent(MapWind0, &Event);
   if (Event.Class == IDCMP_VANILLAKEY && Event.Code == CARRIAGE_RET)
    {
    done = 1;
    abortitem = 1;
    }
   else if (Event.Class == IDCMP_MOUSEBUTTONS && Event.Code == SELECTUP)
    done = 1;
   } /* while */

  MapIDCMP_Restore(MapWind0);
  MapGUI_Message(0, " ");
  SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
  if (abortitem)
   {
   return (0);
   } /* if */
  X = Event.MouseX;
  Y = Event.MouseY;
  } /* if coordinates not passed to findmouse() */

 xlow  = X - 5;
 xhigh = X + 5;
 ylow  = Y - 4;
 yhigh = Y + 4;
 if (! IdentifyOnly)
  outline(MapWind0, OBN, DBase[OBN].Color, &cb); 
 done = 0;

 for (i=0; i<NoOfObjects; i++)
  {
  if (! (DBase[i].Flags & 2) || ! DBase[i].Lat ) continue;
  latlon_XY(i);
  for (j=1; j<=DBase[i].Points; j++)
   {
   if (mapxx[j] < xlow || mapxx[j] > xhigh || mapyy[j] < ylow || mapyy[j] > yhigh)
    continue;
   if (IdentifyOnly)
    {
    ItemFound = i;
    break;
    } /* if */

   if (DBase[i].Color == 2) outline(MapWind0, i, 7, &cb);
   else outline(MapWind0, i, 2, &cb);

   if (User_Message_Def((CONST_STRPTR)DBase[i].Name, (CONST_STRPTR)"Is this the correct object?", (CONST_STRPTR)"YES|NO",
           (CONST_STRPTR)"yn", 1))
    {
    done = 1;
    OBN = i;
    if (DE_Win)
     {
     set(DE_Win->LS_List, MUIA_List_Active, OBN);
     } /* if database edit window open */
    break;
    } /* if correct object */
   else
    {
    outline(MapWind0, i, DBase[i].Color, &cb);
    break;
    } /* else not right object */
   } /* for j=1... */
  if (done || ItemFound >= 0) break;
  } /* for i=0... */
 
 if (! done && ItemFound < 0)
  {
  for (i=0; i<NoOfObjects; i++)
   {
   if (! (DBase[i].Flags & 2) || ! DBase[i].Lat ) continue;
   if (! strcmp(DBase[i].Special, "TOP") || ! strcmp(DBase[i].Special, "SFC"))
    {
    latlon_XY(i);
    if (mapxx[2] > X || mapxx[1] < X || mapyy[2] < Y || mapyy[3] > Y) continue;
    
    if (IdentifyOnly)
     {
     ItemFound = i;
     break;
     } /* if */

    if (DBase[i].Color == 2) outline(MapWind0, i, 7, &cb);
    else outline(MapWind0, i, 2, &cb);

    if (User_Message_Def((CONST_STRPTR)DBase[i].Name, (CONST_STRPTR)"Is this the correct object?", (CONST_STRPTR)"YES|NO",
            (CONST_STRPTR)"yn", 1))
     {
     done = 1;
     OBN = i;
     if (DE_Win)
      {
      set(DE_Win->LS_List, MUIA_List_Active, OBN);
      } /* if database edit window open */
     break;
     } /* if correct object */
    else
     {
     outline(MapWind0, i, DBase[i].Color, &cb);
     } /* else not right object */
    } /* if topo map */
   } /* for i=0... */
  } /* if vector object not found */

 if (IdentifyOnly)
  {
  latlon_XY(OBN);
  return (ItemFound);
  } /* if */

 if (! done)
  {
  User_Message((CONST_STRPTR)"Mapping Module", (CONST_STRPTR)"Object not found!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  if (DBase[OBN].Color == 2) outline(MapWind0, OBN, 7, &cb);
  else outline(MapWind0, OBN, 2, &cb);
  return (-1);
  } /* if not found */

 return (OBN);

} /* findmouse() */

/************************************************************************/

short findmulti(void)
{
 long SelState;
 short i, j;
 struct clipbounds cb;
 struct Box Bx;

 if (! DE_Win)
  {
  Make_DE_Window();
  if (! DE_Win)
   return (-1);
  } /* if no database editor */

 SelState = User_Message_Def((CONST_STRPTR)"Map View: Multi-Select",
         (CONST_STRPTR)"Select or de-select items?", (CONST_STRPTR)"Select|De-select|Cancel", (CONST_STRPTR)"sdc", 1);
 if (SelState == 0)
  return (-1);
 if (SelState == 1)
  SelState = MUIV_List_Select_On;
 else
  SelState = MUIV_List_Select_Off;

 MapGUI_Message(0, "\0338Set first corner point with mouse.");
 SetWindowTitles(MapWind0, (STRPTR) "Set first corner point with mouse", (UBYTE *)-1);

 if (! MousePtSet(&Bx.Low, NULL, 0))
  {
  MapIDCMP_Restore(MapWind0);
  MapGUI_Message(0, " ");
  SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
  return (-1);
  } /* if aborted */

 MapGUI_Message(0, "Set second corner point. ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Set second corner point", (UBYTE *)-1);

 if (! MousePtSet(&Bx.High, &Bx.Low, 2))
  {
  MapIDCMP_Restore(MapWind0);
  MapGUI_Message(0, " ");
  SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
  return (-1);
  } /* if aborted */
 MapIDCMP_Restore(MapWind0);
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

 if (Bx.Low.X > Bx.High.X)
  swmem(&Bx.Low.X, &Bx.High.X, sizeof (short));
 if (Bx.Low.Y > Bx.High.Y)
  swmem(&Bx.Low.Y, &Bx.High.Y, sizeof (short));

 setclipbounds(MapWind0, &cb);

 outline(MapWind0, OBN, DBase[OBN].Color, &cb); 

 for (i=0; i<NoOfObjects; i++)
  {
  if (! (DBase[i].Flags & 2) || ! DBase[i].Lat ) continue;
  latlon_XY(i);
  for (j=1; j<=DBase[i].Points; j++)
   {
   if (mapxx[j] < Bx.Low.X || mapxx[j] > Bx.High.X ||
	 mapyy[j] < Bx.Low.Y || mapyy[j] > Bx.High.Y)
    continue;

   DoMethod(DE_Win->LS_List, MUIM_List_Select, i, SelState, NULL);

   if (SelState == MUIV_List_Select_On)
    {
    if (DBase[i].Color == 2) outline(MapWind0, i, 7, &cb);
    else outline(MapWind0, i, 2, &cb);
    }
   else
    {
    outline(MapWind0, i, DBase[i].Color, &cb);
    }
   break;
   } /* for j=1... */
  } /* for i=0... */
 
 if (DBase[OBN].Color == 2) outline(MapWind0, OBN, 7, &cb);
 else outline(MapWind0, OBN, 2, &cb);
 set (DE_Win->LS_List, MUIA_List_Active, OBN);

 return (OBN);

} /* findmulti() */

/************************************************************************/

void addpoints(long lowj, long insert)
{
 short i, j, done = 0, interp = 0, close = 0, error = 0, digitizing = 0, newobj;
 struct clipbounds cb;
 struct Vertex Low, High;

 if ((newobj = User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
         (CONST_STRPTR)"Digitize new points for the active vector object or create a new object?",
         (CONST_STRPTR)"Active|New|Cancel", (CONST_STRPTR)"anc", 1)) == 0)
  return;
 if (newobj == 2)
  DBaseObject_New();

 if (! strcmp(DBase[OBN].Special, "TOP") || 
 	! strcmp(DBase[OBN].Special, "SFC"))
  {
  User_Message((CONST_STRPTR)"Map View: Digitize",
          (CONST_STRPTR)"Active object is a DEM and may not be digitized!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if DEM */ 

 setclipbounds(MapWind0, &cb);

 if (MP_DigMode)
  {
  double *TempLat = NULL, *TempLon = NULL;
  short *TempElev = NULL;
  long TempSize = 0, OrigPts;

  OrigPts = DBase[OBN].Points;
  if (DBase[OBN].Lat)
   {
   TempSize = DBase[OBN].VecArraySize;
   if (((TempLat = (double *)get_Memory(TempSize, MEMF_ANY)) == NULL)
	|| ((TempLon = (double *)get_Memory(TempSize, MEMF_ANY)) == NULL)
	|| ((TempElev = (short *)get_Memory(TempSize / 4, MEMF_ANY)) == NULL))
    {
    error = 1;
    goto EndDig;
    } /* if no memory */
   memcpy(TempLat, DBase[OBN].Lat, TempSize);
   memcpy(TempLon, DBase[OBN].Lon, TempSize);
   memcpy(TempElev, DBase[OBN].Elev, TempSize / 4);
   } /* if points already exist */
  if (! allocvecarray(OBN, MAXOBJPTS, 0))
   {
   error = 1;
   goto EndDig;
   } /* if not enough memory */
  outline(MapWind0, OBN, backpen, &cb);
  if (InputTabletPoints(lowj, MP_DigMode))
   {
   outline(MapWind0, OBN, 2, &cb);
   if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
           (CONST_STRPTR)"Accept new points?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
    DBase[OBN].Flags |= 1;
   else
    {
    error = 10;
    outline(MapWind0, OBN, backpen, &cb);
    }
   } /* if points digitized */
  else
   error = 10;

EndDig:
  if (error == 10)
   {
   freevecarray(OBN);
   DBase[OBN].Lat = TempLat;
   DBase[OBN].Lon = TempLon;
   DBase[OBN].Elev = TempElev;
   DBase[OBN].VecArraySize = TempSize;
   DBase[OBN].Points = OrigPts;
   outline(MapWind0, OBN, 2, &cb);
   } /* if restore */
  else
   {
   if (TempLat)
    free_Memory(TempLat, TempSize); 
   if (TempLon)
    free_Memory(TempLon, TempSize);
   if (TempElev)
    free_Memory(TempElev, TempSize / 4);
   } /* else */
  if (error == 1)
   User_Message((CONST_STRPTR)"Mapping Module: Digitize",
           (CONST_STRPTR)"Out of memory allocating new vector array!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  if (! error)
   {
   if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
           (CONST_STRPTR)"Conform vector to terrain now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
    {
    if (! topoload)
     error = loadtopo();
    if (! error)
     {
     if (! maptotopo(OBN))
      {
      sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
      Log(MSG_NULL, (CONST_STRPTR)str);
      } /* if conform to terrain */
     } /* if topos loaded */
    } /* if map to topo */
   } /* if no error */
  return;
  } /* if input from tablet */

 if (align)
  {
  Low.X  = Lon_X_Convert(rlon[0]);
  High.X = Lon_X_Convert(rlon[1]);
  Low.Y  = Lat_Y_Convert(rlat[0]);
  High.Y = Lat_Y_Convert(rlat[1]);
  SetAPen(MapWind0->RPort, 7);
  Move(MapWind0->RPort, Low.X,  Low.Y);
  Draw(MapWind0->RPort, High.X, Low.Y);
  Draw(MapWind0->RPort, High.X, High.Y);
  Draw(MapWind0->RPort, Low.X,  High.Y);
  Draw(MapWind0->RPort, Low.X,  Low.Y);
  } /* if align */
 j = lowj;

 SetAPen(MapWind0->RPort, 2);

 ModifyIDCMP(MapWind0, IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);
 while (! done)
  {

  if (! j) strcpy(str, "\0338Set Label Point.  RETURN=interpolate C=close Q=quit");
  else sprintf(str, "\0338Set Point: %d.  RETURN=interpolate C=close Q=quit", j);
  MapGUI_Message(0, str);

  FetchEvent(MapWind0, &Event);
  switch (Event.Class)
   {
   case IDCMP_MOUSEBUTTONS:
    {
    switch (Event.Code)
     {
     case SELECTDOWN:
      {
      if (interp && j > 1)
       {
       j = interpolatepts(j, Event.MouseX, mapxx[j - 1],
		 Event.MouseY, mapyy[j - 1]);
       break;
       } /* if */
      else
       {
       ModifyIDCMP(MapWind0, IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);
       if (j < MAXOBJPTS)
        {
        mapxx[j] = Event.MouseX;
        mapyy[j] = Event.MouseY;
        WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
        digitizing = 1;
        j ++;
        } /* if */
       } /* else */
      break;
      } /* SELECTDOWN */
     case SELECTUP:
      {
      ModifyIDCMP(MapWind0, IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);
      digitizing = 0;
      break;
      } /* SELECTUP */
     } /* switch Event.Code */
    break;
    } /* IDCMP_MOUSEBUTTONS */
   case IDCMP_INTUITICKS:
    {
    sprintf(str, "X: %d, Y: %d", Event.MouseX, Event.MouseY);
    LatLonElevScan(&Event, str, 0);
    if (digitizing)
     {
     if (j < MAXOBJPTS)
      {
      if (abs(Event.MouseX - mapxx[j - 1]) > 1 ||
	 abs(Event.MouseY - mapyy[j - 1] > 1))
       {
       mapxx[j] = Event.MouseX;
       mapyy[j] = Event.MouseY;
       WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
       j ++;
       } /* if movement exceeds 1 pixel */
      } /* if */
     } /* if digitizing */
    break;
    } /* IDCMP_INTUITICKS */
   case IDCMP_VANILLAKEY:
    {
    switch (Event.Code)
     {
     case CARRIAGE_RET:
      {
      if (j) interp = 1 - interp;
      break;
      }
     case 'c':
     case 'C':
      {
      if (j < MAXOBJPTS)
       {
       if (interp)
        j = interpolatepts(j, mapxx[1], mapxx[j - 1], mapyy[1], mapyy[j - 1]);
       else
        {
        mapxx[j] = mapxx[1];
        mapyy[j] = mapyy[1];
        close = 1;
        j ++;
        } /* else */
       done=1;
       } /* if */
      break;
      } /* close object */
     case 'q':
     case 'Q':
      {
      done = 1;
      break;
      } /* quit */
     } /* switch Event.Code */
    break;
    } /* IDCMP_VANILLAKEY */
   } /* switch Event.Class */
  } /* while ! done */

 MapGUI_Message(0, "\0338RETURN to accept, ESC to cancel");
 done = 0;
 ModifyIDCMP(MapWind0, IDCMP_VANILLAKEY);
 while (! done)
  {
  FetchEvent(MapWind0, &Event);
  if (Event.Class == IDCMP_VANILLAKEY)
   {
   if (Event.Code == CARRIAGE_RET)
    {
    if (insert)
     {
     if (! allocvecarray(OBN, DBase[OBN].Points + j - lowj, 0))
      {
      User_Message((CONST_STRPTR)"Mapping Module: Insert Points",
              (CONST_STRPTR)"Out of memory! Operation failed.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
      } /* if out of memory */
     else
      {
      for (i=DBase[OBN].Points; i>=lowj; i--)
       {
       DBase[OBN].Lon[i + (j - lowj)] = DBase[OBN].Lon[i];
       DBase[OBN].Lat[i + (j - lowj)] = DBase[OBN].Lat[i];
       } /* for i=0... */
      DBase[OBN].Points += j - lowj;
      XY_latlon(OBN, lowj, j - 1);
      } /* else */
     } /* if insert */

    else
     {
     if (XY_latlon(OBN, lowj, j - 1))
      DBase[OBN].Points = j - 1;
     if (close)
      {
      DBase[OBN].Lon[DBase[OBN].Points] = DBase[OBN].Lon[1];
      DBase[OBN].Lat[DBase[OBN].Points] = DBase[OBN].Lat[1];
      } /* if closed object */
     } /* else not insert */

    done = 1;
    outline(MapWind0, OBN, 2, &cb);
    DBase[OBN].Flags |= 1;
    if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
            (CONST_STRPTR)"Conform vector to terrain now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
     {
     if (! topoload)
      error = loadtopo();
     if (! error)
      {
      if (! maptotopo(OBN))
       {
       sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
       Log(MSG_NULL, (CONST_STRPTR)str);
       } /* if conform to terrain */
      } /* if topos loaded */
     } /* if map to topo */
    } /* if RETURN */
   else if (Event.Code == ESC)
    done = 1;
   } /* if IDCMP_VANILLAKEY */
  } /* while ! done */

 MapGUI_Message(0, " ");
 MapIDCMP_Restore(MapWind0);

} /* addpoints() */

/************************************************************************/

STATIC_FCN short interpolatepts(short j, long x2, long x1, long y2, long y1) // used locally only -> static, AF 19.7.2021
{
 long x,y;
 float m;
 
 if (y2 == y1 && x2 == x1) return(j);
 if (abs(y2 - y1) > abs(x2 - x1))
  {       /* y difference greater */
  m = ((float)x2 - x1) / (y2 - y1);
  if (y2 > y1)
   {
   for (y=1; y<=y2-y1 && j<MAXOBJPTS; y+=2, j++)
    {
    mapxx[j] = m * y + x1;
    mapyy[j] = y + y1;
    WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
    } /* for y=1... */
   } /* if last y greater */
  else
   {
   for (y=-1; y>=y2-y1 && j<MAXOBJPTS; y-=2, j++)
    {
    mapxx[j] = m * y + x1;
    mapyy[j] = y + y1;
    WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
    } /* for y=-1... */
   } /* else */
  } /* if y difference greater */
 else
  {                 /* x difference greater */
  m = ((float)y2 - y1) / (x2 - x1);
  if (x2 > x1)
   {
   for (x=1; x<=x2-x1 && j<MAXOBJPTS; x+=2, j++)
    {
    mapxx[j] = x + x1;
    mapyy[j] = m * x + y1;
    WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
    } /* for x=1... */
   } /* if last x greater */
  else
   {
   for (x=-1; x>=x2-x1 && j<MAXOBJPTS; x-=2, j++)
    {
    mapxx[j] = x + x1;
    mapyy[j] = m * x + y1;
    WritePixel(MapWind0->RPort, mapxx[j], mapyy[j]);
    } /* for x=-1... */
   } /* else */
  } /* else x difference greater */ 

 return(j);    

} /* interpolatepts() */

/************************************************************************/

void Viewshed_Map(long OBN)
{
 ULONG zipoffset;
 short *zip, *mapzip, *vpzip, *viewzip;
 short error, i, j, k, col, map_width, map_height, map_left, map_top,
	highx, lowy, SmoothMap, ElOffset;
 long Lr, Lc;
 struct BusyWindow *BWVS;
 
 if (! User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
         (CONST_STRPTR)"Create Visual Sensitivity map for this object?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
  return;

 if (! DBase[OBN].Lat)
  {
  if (Load_Object(OBN, NULL) > 0)
   {
   User_Message((CONST_STRPTR)"DBase[OBN].Name",
           (CONST_STRPTR)"Error loading vector object!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* if load fail */
  } /* if not loaded */
 if (MapWind3)
  {
  Close_Viewshed_Window();
  } /* if already open, close it to start fresh */

 map_width = MapWind0->Width;
 map_height = MapWind0->Height;
 map_left = MapWind0->LeftEdge;
 map_top = MapWind0->TopEdge;

 MapWind3 = OpenWindowTags(NULL,
	WA_Left,	map_left,
	WA_Top,		map_top,
	WA_Width,	map_width,
	WA_Height,	map_height,
	WA_DragBar,	TRUE,
	WA_CloseGadget,	TRUE,
	WA_SmartRefresh,TRUE,
	WA_SizeGadget,	TRUE,
	WA_SizeBBottom,	TRUE,
	WA_DepthGadget,	TRUE,
	WA_ReportMouse,	TRUE,
	WA_AutoAdjust,	TRUE,
	WA_Activate,	TRUE,
	WA_MinWidth,	50,
	WA_MinHeight,	30,
	WA_MaxWidth,	WCSScrn->Width,
	WA_MaxHeight,	WCSScrn->Height,
	WA_IDCMP,	IDCMP_CLOSEWINDOW,
	WA_Title,	(ULONG)"Visual Sensitivity",
	WA_CustomScreen,(ULONG)WCSScrn,
	TAG_DONE);

 if (! MapWind3)
  {
  User_Message((CONST_STRPTR)"Mapping Module",
          (CONST_STRPTR)"Error opening viewshed window!\nExecution terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_WIN_FAIL, (CONST_STRPTR)"Mapping module");
  return;
  }

 MapWind3_Sig = 1L << MapWind3->UserPort->mp_SigBit;
 WCS_Signals |= MapWind3_Sig;

/* Allocate other resources */
 if ((VS = (struct Viewshed *)get_Memory(sizeof (struct Viewshed), MEMF_CLEAR))
	== NULL)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Viewshed",
          (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_Viewshed_Window();
  } /* if memory bust */

 setclipbounds(MapWind3, &VS->cb);
 SetAPen(MapWind3->RPort,backpen);
 RectFill(MapWind3->RPort, VS->cb.lowx, VS->cb.lowy, VS->cb.highx, VS->cb.highy);

 VS->Width = 1 + VS->cb.highx - VS->cb.lowx;
 VS->Height = 1 + VS->cb.highy - VS->cb.lowy;
 VS->Mapsize = 2 * VS->Width * VS->Height;

 VS->Map = (short *)get_Memory(VS->Mapsize, MEMF_CLEAR);
 VS->View = (short *)get_Memory(VS->Mapsize, MEMF_CLEAR);

 if (VS->Map == NULL || VS->View == NULL)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Viewshed",
          (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_Viewshed_Window();
  } /* if memory bust */

/* Load topo maps if not already in memory */
 error = (topoload==0) ? loadtopo():0;
 if (error)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Viewshed",
          (CONST_STRPTR)"Error reading topo maps!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Close_Viewshed_Window();
  return;
  }

 SmoothMap = User_Message((CONST_STRPTR)"Mapping Module: Viewshed",
         (CONST_STRPTR)"Smooth the map before computing viewshed?", (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc");
 sprintf(str, "%d", 5);
 if (! GetInputString("Enter vertical offset in meters.",
	 "+.,abcdefghijklmnopqrstuvwxyz", str))
  {
  Close_Viewshed_Window();
  return;
  } /* if cancel */
 ElOffset = atoi(str);

/* Place elevations for mapped area in one array at screen density */
 for (i=0; i<topomaps; i++)
  {
  latlon_XY(TopoOBN[i]);
  map_width = max(mapxx[1] - mapxx[2], 1);
  map_height = max(mapyy[1] - mapyy[4], 1);
  highx = min(mapxx[1], VS->cb.highx);
  lowy = max(mapyy[4], VS->cb.lowy);
  
  for (j=max(mapxx[2],VS->cb.lowx); j<=highx; j++)
   {
   Lr = (mapelmap[i].rows * (j - mapxx[2])) / map_width;
   for (k=min(mapyy[1],VS->cb.highy); k>=lowy; k--)
    {
    Lc = ((mapelmap[i].columns - 1) * (mapyy[1] - k)) / map_height;
    zip = mapelmap[i].map + Lr * mapelmap[i].columns + Lc;
    mapzip = VS->Map + (j - VS->cb.lowx) * VS->Height + k - VS->cb.lowy;
    *mapzip = *zip;
    } /* for k=min... */
   } /* for j=max... */
  } /* for i=0 <topomaps */

/* Smooth map */
 if (SmoothMap)
  {
  short l;
  long sum;

  BWVS = BusyWin_New("Smoothing...", VS->Width, 0, (ULONG)"BWVS");

  memcpy(VS->View, VS->Map, VS->Mapsize);

  for (i=2; i<VS->Width - 2; i++)
   {
   viewzip = VS->View + i * VS->Height + 2;
   for (j=2; j<VS->Height - 2; j++)
    {
    sum = 0;
    for (k=-2; k<3; k++)
     {
     mapzip = VS->Map + (i + k) * VS->Height + j - 2;
     for (l=-2; l<3; l++)
      {
      sum += *mapzip;
      mapzip ++;
      } /* for l=-1 */
     } /* for k=-1... */
    *viewzip = (double)sum / 9.0;
    viewzip ++;
    } /* for j=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWVS, i + 3);
   } /* for i=0... */
  if (BWVS) BusyWin_Del(BWVS);

  if (error)
   {
   Close_Viewshed_Window();
   return;
   } /* if cancel */

  memcpy(VS->Map, VS->View, VS->Mapsize);
  memset(VS->View, 0, VS->Mapsize);

  } /* if Smooth Map */

/* Convert path object to screen coords and store in mapxx[], mapyy[] */
 latlon_XY(OBN);

/*
** For [each path point [for each map point [for each point on line between]]]
**	determine if elevation at in between points is higher than a ratio
**	of distance from path to focus * elevation difference.
*/

 BWVS = BusyWin_New("Path Point", DBase[OBN].Points, 0, (ULONG)"BWVS");

 for (i=1; i<=DBase[OBN].Points; i++)
  {
  if (mapxx[i] < VS->cb.lowx || mapxx[i] > VS->cb.highx
	|| mapyy[i] < VS->cb.lowy || mapyy[i] > VS->cb.highy)
   continue;
  vpzip = VS->Map + 
	(mapxx[i] - VS->cb.lowx) * VS->Height + (mapyy[i] - VS->cb.lowy);
  if (! *vpzip)
   {
   continue;
   } /* view point falls outside map boundaries */

  for (j=0; j<VS->Width; j++)
   {
   for (k=0; k<VS->Height; k++)
    {
    zipoffset = j * VS->Height + k;
    mapzip = VS->Map + zipoffset;
    if (! *mapzip)
     {
     continue;
     } /* point falls outside map boundaries */
    viewzip = VS->View + zipoffset;
    if (vpzip == mapzip)
     {
     *viewzip += 1;
     } /* view and focus points coincident */
    else
     {
     *viewzip += SearchViewLine(i, j, k, *mapzip, *vpzip + ElOffset);
     }
    col = 15.999 - ((*viewzip) / (float)i) * 7.99; 
    SetAPen(MapWind3->RPort, col);
    WritePixel(MapWind3->RPort, j + VS->cb.lowx, k + VS->cb.lowy);
    } /* for k=0... for each y point within mapped window (excluding border) */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   } /* for j=0... for each x point within mapped window (excluding border) */
  if (error)
   break;
  BusyWin_Update(BWVS, i);
  } /* for i=0... each path point */
 if (BWVS) BusyWin_Del(BWVS);

 if (User_Message((CONST_STRPTR)"Mapping Module: Viewshed",
         (CONST_STRPTR)"Draw vectors on viewshed rendering?", (CONST_STRPTR)"Yes|No", (CONST_STRPTR)"yn"))
  makemap(MapWind3, VS->cb.lowx, VS->cb.lowy, VS->cb.highx, VS->cb.highy, (ULONG)NULL);

} /* Viewshed() */

/************************************************************************/

void Close_Viewshed_Window(void)
{

 if (VS->Map) free_Memory(VS->Map, VS->Mapsize);
 if (VS->View) free_Memory(VS->View, VS->Mapsize);
 if (VS) free_Memory(VS, sizeof (struct Viewshed));
 VS = NULL;
 if (MapWind3)
  {
  WCS_Signals ^= MapWind3_Sig;
  MapWind3_Sig = 0;
  closesharedwindow(MapWind3, 0);
  MapWind3 = NULL;
 }

} /* Close_Viewshed_Window() */

/************************************************************************/

void Handle_Viewshed_Window(void)
{

 QuickFetchEvent(MapWind3, &Event);

 switch (Event.Class)
  {
  case IDCMP_CLOSEWINDOW:
   {
   Close_Viewshed_Window();
   break;
   } /* CLOSEWINDOW */
  } /* switch Event.Class */

} /* Handle_Viewshed_Window() */

/************************************************************************/

STATIC_FCN short SearchViewLine(short viewpt, short fpx, short fpy,
	short elfp, short elvp) // used locally only -> static, AF 23.7.2021
{
 short offsetX, offsetY, x, y, vpx, vpy, offel, ptx, pty;
 double m, eldif, offfract;

 eldif = elfp - elvp;
 vpx = mapxx[viewpt] - VS->cb.lowx;
 vpy = mapyy[viewpt] - VS->cb.lowy;

 offsetX = fpx - vpx;
 offsetY = fpy - vpy;

 if (abs(offsetY) > abs(offsetX))
  {
  m = ((double)fpx - vpx) / (fpy - vpy);
  if (fpy > vpy)
   {
   for (y=1; y<offsetY; y++)
    {
    ptx = m * y + vpx;
    pty = y + vpy;
    offfract = fabs((double)y / offsetY);
    offel = elvp + offfract * eldif;
    if (offel < *(VS->Map + ptx * VS->Height + pty)) return (0);
    } /* for y=1... */
   } /* if looking to right */
  else
   {
   for (y=-1; y>fpy-vpy; y--)
    {
    ptx = m * y + vpx;
    pty = y + vpy;
    offfract = fabs((double)y / offsetY);
    offel = elvp + offfract * eldif;
    if (offel < *(VS->Map + ptx * VS->Height + pty)) return (0);
    } /* for y=-1... */
   } /* if looking west */
  } /* if Y difference greater */
 else
  {
  m = ((double)fpy - vpy) / (fpx - vpx);
  if (fpx > vpx)
   {
   for (x=1; x<offsetX; x++)
    {
    ptx = x + vpx;
    pty = m * x + vpy;
    offfract = fabs((double)x / offsetX);
    offel = elvp + offfract * eldif;
    if (offel < *(VS->Map + ptx * VS->Height + pty)) return (0);
    } /* for x=1... */
   } /* if looking south */
  else
   {
   for (x=-1; x>offsetX; x--)
    {
    ptx = x + vpx;
    pty = m * x + vpy;
    offfract = fabs((double)x / offsetX);
    offel = elvp + offfract * eldif;
    if (offel < *(VS->Map + ptx * VS->Height + pty)) return (0);
    } /* for x=-1... */
   } /* else if looking north */
  } /* else X difference greater */

 return (1);

} /* SearchViewLine() */

/************************************************************************/

STATIC_FCN struct RastPort *TempRas_New(struct RastPort *Ancestor, int X, int Y) // used locally only -> static, AF 23.7.2021
{
struct RastPort *This;
int bloop;

This = (struct RastPort *)get_Memory(sizeof(struct RastPort), MEMF_CLEAR);
if(This)
	{
	memcpy(This, Ancestor, sizeof(struct RastPort));
	This->Layer = NULL;
	if((This->BitMap = (struct BitMap *)get_Memory(sizeof(struct BitMap), MEMF_CLEAR)))
		{
		memcpy(This->BitMap, Ancestor->BitMap, sizeof(struct BitMap));
		This->BitMap->Rows = Y;
		This->BitMap->BytesPerRow = X;
/*		This->BitMap->BytesPerRow = (((X+15)>>4)<<1); */
		for(bloop=0;bloop<This->BitMap->Depth;bloop++)
			{
			if(!(This->BitMap->Planes[bloop] = get_Memory(This->BitMap->BytesPerRow, MEMF_CHIP)))
				{
				bloop=200;
				} /* if */
			} /* for */
		if(bloop == 200)
			{
			for(bloop=0;bloop<This->BitMap->Depth;bloop++)
				{
				if(This->BitMap->Planes[bloop])
					{
					free_Memory(This->BitMap->Planes[bloop], This->BitMap->BytesPerRow);
					This->BitMap->Planes[bloop] = NULL;
					} /* if */
				} /* for */
			free_Memory(This->BitMap, sizeof(struct BitMap));
			free_Memory(This, sizeof(struct RastPort));
			return(NULL);
			} /* if */
		else
			{
			return(This);
			} /* else */
		} /* if */
	else
		{
		free_Memory(This, sizeof(struct RastPort));
		return(NULL);
		} /* else */
	} /* if */
    return NULL;
} /* TempRas_New() */

/************************************************************************/

STATIC_FCN void TempRas_Del(struct RastPort *This, int X, int Y) // used locally only -> static, AF 23.7.2021
{
int bloop;

for(bloop=0;bloop<This->BitMap->Depth;bloop++)
	{
	if(This->BitMap->Planes[bloop])
		{
		free_Memory(This->BitMap->Planes[bloop], This->BitMap->BytesPerRow);
		This->BitMap->Planes[bloop] = NULL;
		} /* if */
	} /* for */
free_Memory(This->BitMap, sizeof(struct BitMap));
free_Memory(This, sizeof(struct RastPort));

} /* TempRas_Del() */

/************************************************************************/

double *DitherTable_New(int size)
{
double *This;
int randloop;

if((This = (double *)get_Memory(sizeof(double) * size, MEMF_ANY)))
	{
	for(randloop = 0; randloop < size; randloop++)
		{
		This[randloop] = drand48();
		} /* for */
	return(This);
	} /* if */

return(NULL);
} /* DitherTable_New() */

/************************************************************************/

void DitherTable_Del(double *This, long size) // used locally only -> static, AF 23.7.2021
{

free_Memory(This, sizeof(double) * size);

} /* DitherTable_Del() */

/************************************************************************/

STATIC_FCN short InputTabletPoints(long lowj, short TabletType) // used locally only -> static, AF 23.7.2021
{
struct MsgPort *SerialMP;
struct IOExtSer *SerialIO;

UBYTE SerialReadBuffer[MAX_READ_BUFFER_SIZE];
long WaitMask;
double Slat, Elon, lengthX, lengthY, length, Rx[3], Ry[3];
short i, b, error = 0, ReadSize, ByteX, ByteY, ByteButton;

 switch (TabletType)
  {
  case 1:
   {
   ReadSize = READ_BUFFER_SIZE_BITPAD;
   ByteX = 0;
   ByteY = 5;
   ByteButton = 10;
   break;
   } /* Summagraphics Bitpad */
  case 2:
   {
   ReadSize = READ_BUFFER_SIZE_SUMMAGRID;
   ByteX = 0;
   ByteY = 8;
   ByteButton = 16;
   break;
   } /* Summagraphics Summagrid */
  } /* switch */

 if ((SerialMP = CreateMsgPort()))
  {
  if ((SerialIO = (struct IOExtSer *)
	CreateExtIO(SerialMP, sizeof (struct IOExtSer))))
   {
   if (OpenDevice((STRPTR)SERIALNAME, 0, (struct IORequest *)SerialIO, 0))
    {
    error = 1;
    User_Message((CONST_STRPTR)"Mapping Module: Digitize",
            (CONST_STRPTR)"Can't open serial device!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    } /* if open serial device failed */
   else
    {
    WaitMask = 1L << SerialMP->mp_SigBit;
/*
    SerialIO->IOSer.io_Length  = 1;
    SerialIO->IOSer.io_Data    = (APTR)"s";
    SerialIO->IOSer.io_Command = CMD_WRITE;

    SendIO((struct IORequest *)SerialIO);
    while (! CheckIO((struct IORequest *)SerialIO));
    printf("%s sending configuration\n",SERIALNAME);
*/
    SerialIO->IOSer.io_Length  = 1;
    SerialIO->IOSer.io_Data    = (APTR)&SerialReadBuffer[0];
    SerialIO->IOSer.io_Command = CMD_READ;

    SendIO((struct IORequest *)SerialIO);
    Delay(10);

    AbortIO((struct IORequest *)SerialIO);
    WaitIO((struct IORequest *)SerialIO);

    
    if (User_Message((CONST_STRPTR)"Mapping Module: Digitize",
            (CONST_STRPTR)"Digitize new registration points?", (CONST_STRPTR)"YES|NO", (CONST_STRPTR)"yn"))
     {
     MP_Nlat = rlat[0];
     Slat = rlat[1];
     MP_Wlon = rlon[0];
     Elon = rlon[1];

/* Digitize three points (button 3 to break) */

     for (i=0; i<3; i++)
      {
      if (i == 0)
       MapGUI_Message(0, "\0338Set NW registration point. Button 3=abort");
      else if (i == 1)
       MapGUI_Message(0, "\0338Set NE registration point. Button 3=abort");
      else
       MapGUI_Message(0, "\0338Set SE registration point. Button 3=abort");
      SerialIO->IOSer.io_Length  = ReadSize;
      SerialIO->IOSer.io_Data    = (APTR)&SerialReadBuffer[0];
      SerialIO->IOSer.io_Command = CMD_READ;

      SendIO((struct IORequest *)SerialIO);

      while (1)
       {
       Wait(WaitMask);
       if (CheckIO((struct IORequest *)SerialIO))
        {
        WaitIO((struct IORequest *)SerialIO);
        Rx[i] = atof((char*)&SerialReadBuffer[ByteX]);
        Ry[i] = atof((char*)&SerialReadBuffer[ByteY]);
        b = atoi((char*)&SerialReadBuffer[ByteButton]);
        if (b == 3)
         {
         error = 1;
         break; 
	 } /* if abort */
        sprintf(str, "Rx[%d]=%ld  Ry[%d]=%ld", i, (long)Rx[i], i, (long)Ry[i]);
        Log(DTA_NULL, (CONST_STRPTR)str);
        MapGUI_Message(1, str);
        break;
        } /* if Serial input */
       } /* while */
      if (error)
       break;
      } /* for i=0... */
     if (error)
      goto EndCheck;

/* Compute scales & rotation */
     lengthX = Rx[1] - Rx[0];
     lengthY = Ry[1] - Ry[0];
     length = sqrt(lengthX * lengthX + lengthY * lengthY);
     sprintf(str, "length %f %f %f\n",lengthX, lengthY, length);
     Log(DTA_NULL, (CONST_STRPTR)str);
     if (Ry[1] == Ry[0])
      {
      if (Rx[1] > Rx[0]) MP_Rotate = 0.0;
      else if (Rx[0] > Rx[1]) MP_Rotate = Pi;
      else
       {
       User_Message((CONST_STRPTR)"Mapping Module: Digitize",
               (CONST_STRPTR)"Illegal value!\nTwo registration points may not be coincident.\nOperation terminated.",
               (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
       Log(WNG_ILL_VAL, (CONST_STRPTR)"Registration points coincident");
       error = 1;
       goto EndCheck;
       } /* else two points identical */
      } /* if Ry constant */
     else if (Rx[1] == Rx[0])
      {
      if (Ry[1] > Ry[0]) MP_Rotate = Pi;
      else MP_Rotate = OneAndHalfPi;
      } /* else if Rx constant */
     else
      {
      MP_Rotate = atan(lengthY / lengthX);
      if (lengthX < 0.0) MP_Rotate += Pi;
      } /* else */

     sprintf(str, "Map rotation = %f\n", MP_Rotate * PiUnder180);
     Log(DTA_NULL, (CONST_STRPTR)str);

     MP_DigLonScale = (Elon - MP_Wlon) / length;
     sprintf(str, "Longitude scale = %f /point", MP_DigLonScale);
     Log(DTA_NULL, (CONST_STRPTR)str);

     RotatePt(-MP_Rotate, &Rx[0], &Ry[0], &Rx[2], &Ry[2]);

     MP_DigLatScale = (Slat - MP_Nlat) / (Ry[2] - Ry[0]);
     sprintf(str, "Latitude scale = %f /point", MP_DigLatScale);
     Log(DTA_NULL, (CONST_STRPTR)str);
     MP_ORy = Ry[0];
     MP_ORx = Rx[0];
     } /* if make new setup */
    else
     {
     Ry[0] = MP_ORy;
     Rx[0] = MP_ORx;
     } /* else */

    for (i=lowj; i<MAXOBJPTS; i++)
     {
     sprintf(str, "Set point %d. Button 2=close, 3=abort, 4=done", i);
     MapGUI_Message(0, str);
     SerialIO->IOSer.io_Length  = ReadSize;
     SerialIO->IOSer.io_Data    = (APTR)&SerialReadBuffer[0];
     SerialIO->IOSer.io_Command = CMD_READ;

     SendIO((struct IORequest *)SerialIO);

     while (1)
      {
      Wait(WaitMask);
      if (CheckIO((struct IORequest *)SerialIO))
       {
       WaitIO((struct IORequest *)SerialIO);
       Rx[2] = atof((char*)&SerialReadBuffer[ByteX]);
       Ry[2] = atof((char*)&SerialReadBuffer[ByteY]);
       b = atoi((char*)&SerialReadBuffer[ByteButton]);
       if (b == 2)
        {
        DBase[OBN].Lat[i] = DBase[OBN].Lat[1];
        DBase[OBN].Lon[i] = DBase[OBN].Lon[1];
        break;
	} /* else close object */
       if (b == 3)
        {
        error = 1;
        break;
	} /* if cancel */
       if (b == 4)
        {
        if (i > 1)
         i --;
        break;
        } /* if break time */

/* Compute coordinates */
       RotatePt(-MP_Rotate, &Rx[0], &Ry[0], &Rx[2], &Ry[2]);

       DBase[OBN].Lat[i] = MP_Nlat + MP_DigLatScale * (Ry[2] - Ry[0]);
       DBase[OBN].Lon[i] = MP_Wlon + MP_DigLonScale * (Rx[2] - Rx[0]);
       sprintf(str, "Pt %d  Lat=%f  Lon=%f", i, DBase[OBN].Lat[i], DBase[OBN].Lon[i]);
       MapGUI_Message(1, str);
       break;
       } /* if Serial input */
      } /* while */
     if (b == 3 || b == 2 || b == 4)
      break;
     } /* for i=0... */

    DBase[OBN].Points = i;

EndCheck:

    AbortIO((struct IORequest *)SerialIO);
    WaitIO((struct IORequest *)SerialIO);

    CloseDevice((struct IORequest *)SerialIO);
    } /* else Serial Device opened */
   DeleteExtIO((struct IORequest *)SerialIO);
   } /* if SerialIO created */
  else
   {
   error = 1;
   User_Message((CONST_STRPTR)"Mapping Module: Digitize",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   } /* else IORequest not created */
  DeleteMsgPort(SerialMP);
  } /* if Message Port created */
 else
  {
  error = 1;
  User_Message((CONST_STRPTR)"Mapping Module: Digitize",
          (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  } /* else Message Port not created */

 MapGUI_Message(0, " ");
 MapGUI_Message(1, " ");

 return ((short)(! error));

}/* InputTabletPoints() */

/************************************************************************/

STATIC_FCN void RotatePt(double rotate, double *OriginX, double *OriginY,
		double *PointX, double *PointY) // used locally only -> static, AF 23.7.2021
{
 double angle, length;

 length = sqrt((*PointX - *OriginX) * (*PointX - *OriginX) +
		(*PointY - *OriginY) * (*PointY - *OriginY));

 if (length == 0.0) return;
 if (*PointY == *OriginY)
  {
  if (*PointX > *OriginX) angle = 0.0;
  else if (*OriginX > *PointX) angle = Pi;
  } /* if Y constant */
 else if (*PointX == *OriginX)
  {
  if (*PointY > *OriginY) angle = Pi;
  else angle = OneAndHalfPi;
  } /* else if X constant */
 else
  {
  angle = atan((*PointY - *OriginY) / (*PointX - *OriginX));
  if ((*PointX - *OriginX) < 0.0) angle += Pi;
  } /* else */
 
 angle += rotate;
 *PointX = *OriginX + length * cos(angle);
 *PointY = *OriginY + length * sin(angle);

} /* RotatePt() */

/*************************************************************************/

void FindCenter(double *Lat, double *Lon)
{
int i, j;
float CurLat, CurLon, MinLat, MaxLat, MinLon, MaxLon;

MinLat = 89.999; 
MinLon = 360.0;
MaxLat = -89.999;
MaxLon = -360.0;

for(i = 0; i < NoOfObjects; i++)
	{
	if (! DBase[i].Lat || ! (DBase[i].Flags & 2))
		continue;
	for(j = 0; j <=DBase[i].Points; j++)
		{
		CurLat = DBase[i].Lat[j];
		CurLon = DBase[i].Lon[j];
		
		if(CurLat > MaxLat)
			MaxLat = CurLat;
		if(CurLat < MinLat)
			MinLat = CurLat;

		if(CurLon > MaxLon)
			MaxLon = CurLon;
		if(CurLon < MinLon)
			MinLon = CurLon;
		} /* for */
	} /* for */

CurLat = (MaxLat + MinLat) / 2.0;
CurLon = (MaxLon + MinLon) / 2.0;

*Lat = CurLat;
*Lon = CurLon;

return;
} /* FindCenter() */

/**********************************************************************/

void ClearWindow(struct Window *Win, int col)
{

SetAPen(Win->RPort, col);
RectFill(Win->RPort, Win->BorderLeft, Win->BorderTop,
 Win->Width - Win->BorderRight - 1, Win->Height - Win->BorderBottom - 1);

return;
} /* ClearWindow() */

/**********************************************************************/

STATIC_FCN short Set_Eco_Color(long Lra, long Lca, short i, short *RelEl) // used locally only -> static, AF 23.7.2021
{	/* returns 1 by default */
 short eco, Col = 1, relel;
 long elev, ecoline, zipa, zip;

 zipa = Lra * mapelmap[i].columns + Lca;
 elev = mapelmap[i].lmap[zipa];

 if (RelEl)
  relel = RelEl[zipa];
 else
  relel = 0;

 if (Lra == mapelmap[i].rows)
  Lra --;
 if (Lca == mapelmap[i].columns - 1)
  Lca --;
 mapelmap[i].Lr = Lra;
 mapelmap[i].Lc = Lca;

 facelat = mapelmap[i].lolat + mapelmap[i].Lc * mapelmap[i].steplat;
 zip = mapelmap[i].Lr * mapelmap[i].columns + mapelmap[i].Lc;
 mapelmap[i].facept[0] = zip;
 mapelmap[i].facept[1] = zip + mapelmap[i].columns;
 mapelmap[i].facept[2] = zip + 1;
 faceone(&mapelmap[i]);	/* computes slope, diplong, diplat & aspect */

   ecoline = PARC_RNDRLN_ECO(0);
   if (settings.flatteneco)
    ecoline += ( (PARC_RNDR_MOTION(13) - ecoline) * PARC_RNDR_MOTION(12) );
   if (elev < ecoline)
    {
    eco = 0;
    goto EndDraw;
    } /* water */

   ecoline = PARC_RNDRLN_ECO(1) +
	diplong * PARC_SKLN_ECO(1) + diplat
	* PARC_SKLT_ECO(1) + PARC_RNDRRE_ECO(1) * relel;
   if (settings.worldmap)
    ecoline -= ((abs(facelat) - abs(settings.globreflat)) * settings.globecograd);
   if (settings.flatteneco)
    ecoline += ( (PARC_RNDR_MOTION(13) - ecoline) * PARC_RNDR_MOTION(12) );

   if (elev >= ecoline)
    {
    if (slope >= PARC_MNSL_ECO(1)
	 && slope <= PARC_MXSL_ECO(1))
     {
     if (relel >= PARC_RNDRNR_ECO(1)
		&& relel <= PARC_RNDRXR_ECO(1))
      {
      eco = 1;
      goto EndDraw;
      } /* if relel matchup */
     } /* if slope matchup */
    } /* if el > snowline */

   for (eco=12; eco<ECOPARAMS; eco++)
    {
    ecoline = PARC_RNDRLN_ECO(eco) +
	diplong * PARC_SKLN_ECO(eco) + diplat
	* PARC_SKLT_ECO(eco) + PARC_RNDRRE_ECO(eco) * relel;
    if (settings.worldmap)
     ecoline -= ((abs(facelat) - abs(settings.globreflat)) * settings.globecograd);
    if (settings.flatteneco)
     ecoline += ( (PARC_RNDR_MOTION(13) - ecoline) * PARC_RNDR_MOTION(12) );

    if (elev <= ecoline)
     {
     if (slope >= PARC_MNSL_ECO(eco)
	 && slope <= PARC_MXSL_ECO(eco))
      {
      if (relel >= PARC_RNDRNR_ECO(eco)
		&& relel <= PARC_RNDRXR_ECO(eco))
       {
       break;
       } /* if relel matchup */
      } /* if slope matchup */
     } /* if elevation matchup */
    } /* for eco=12... */

   if (eco >= ECOPARAMS)
    {
    Log(WNG_ILL_VAL, (CONST_STRPTR)"Ecosystem out of range.");
    eco = ECOPARAMS - 1;
    } /* if */

EndDraw:

   if (eco == EcoLegend[0] && EcoUse[0])
    {
    Col = 2;
    }
   else if (eco == EcoLegend[1] && EcoUse[1])
    {
    Col = 3;
    }
   else if (eco == EcoLegend[2] && EcoUse[2])
    {
    Col = 4;
    }
   else if (eco == EcoLegend[3] && EcoUse[3])
    {
    Col = 5;
    }
   else if (eco == EcoLegend[4] && EcoUse[4])
    {
    Col = 6;
    }
   else if (eco == EcoLegend[5] && EcoUse[5])
    {
    Col = 7;
    }
 
 return (Col);

} /* Set_Eco_Color() */
