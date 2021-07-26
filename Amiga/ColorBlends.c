/* ColorBlends.c (ne giscolorblends.c 14 Jan 1994 CXH)
** Functions to map and color the various types of terrain.
** Ripped and Built from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. Huber, of course.
*/

#include "WCS.h"

#define RENDER_SCREEN_DITHER


// short ComputeBumpMapTexture(double LatPt, double LonPt); // AF, not used 26.July 2021
// void StrataConvert(void); // AF, not used 26.July 2021
static long MakeNoise(UBYTE *NoiseMap, long MaxNoise, double Lat, double Lon); // used locally only -> static, AF 26.7.2021
static void seashoal(struct ColorComponents *CC); // used locally only -> static, AF 26.7.2021
static void SetScreenColor(short ecotype); // used locally only -> static, AF 26.7.2021


short ecoset(short i, short notsnow, struct ColorComponents *CC)
{
 short j, understory = 1;

 if (notsnow)
  {
  SetScreenColor((PAR_TYPE_ECO(PAR_UNDER_ECO(i)) & 0x00ff) % 50);

  understory = PAR_UNDER_ECO(i);

  if (i == 0)
   {
/* for ocean, the skew variable becomes water depth */
   if (el < SeaLevel)
    {
    seadepthfact = Random + (SeaLevel - el) / PARC_RNDRSK_ECO(0);
    if (seadepthfact > 1.0) seadepthfact = 1.0;
    else if (seadepthfact < 0.0) seadepthfact = 0.0;
    }
   else
    seadepthfact = 0.0;
   FloatCol = 2.0 + 3.99 * seadepthfact * sunshade;
   ColMax = COL_WATER_MAX;
   CC[0].Red  = PARC_MCOL_ECO(0, 0);
   CC[0].Red -= CC[0].Red * seadepthfact;
   CC[0].Red -= CC[0].Red * sunshade;
   CC[0].Grn  = PARC_MCOL_ECO(0, 1);
   CC[0].Grn -= CC[0].Grn * seadepthfact;
   CC[0].Grn -= CC[0].Grn * sunshade;
   CC[0].Blu  = PARC_MCOL_ECO(0, 2);
   CC[0].Blu -= CC[0].Blu * seadepthfact;
   CC[0].Blu -= CC[0].Blu * sunshade;

   if (el <= SeaLevel && el > SeaLevel - 100.0)
    {
    seashoal(&CC[0]);
    } /* if */

   if (FM[0].TM)
    {
    if (PARC_RNDRDN_ECO(0) > treerand2 * 100.0 && settings.rendertrees)
     {
     for (j=0; j<FM[0].Items-1; j++)
      {
      if (treerand < FM[0].TM[j].Pct)
       break;
      }
     CC[2].Red = FM[0].TM[j].Red + redrand;		/* overstory color */
     CC[2].Red -= CC[2].Red * seadepthfact;
     CC[2].Red -= CC[2].Red * sunshade;
     CC[2].Grn = FM[0].TM[j].Grn + greenrand;
     CC[2].Grn -= CC[2].Grn * seadepthfact;
     CC[2].Grn -= CC[2].Grn * sunshade;
     CC[2].Blu = FM[0].TM[j].Blu + bluerand;
     CC[2].Blu -= CC[2].Blu * seadepthfact;
     CC[2].Blu -= CC[2].Blu * sunshade;
     treeheight =
	treerand * treehtfact * FM[0].TM[j].Ht / 21.34;	/* ht in meters */
     treedraw = 2;
     EcoClass = FM[0].TM[j].Class;
     }
    } /* if water model */
   return (0);
   } /* if water */

  CC[0].Red = PARC_SCOL_ECO(PAR_UNDER_ECO(i), 0) + redrand;	/* understory color */
  CC[0].Red -= sunshade * CC[0].Red;			/* shading */
  CC[0].Grn = PARC_SCOL_ECO(PAR_UNDER_ECO(i), 1) + greenrand;
  CC[0].Grn -= sunshade * CC[0].Grn;
  CC[0].Blu = PARC_SCOL_ECO(PAR_UNDER_ECO(i), 2) + bluerand;
  CC[0].Blu -= sunshade * CC[0].Blu;
  if (! settings.rendertrees)
   return (understory);

  if ((PAR_TYPE_ECO(i) & 0x00ff) < 100 && PARC_RNDRDN_ECO(i) > treerand2 * 100.0)
   {
   if (! FM[i].TM)
    {
    treedraw = 1;
    CC[2].Red = PARC_MCOL_ECO(i, 0) + redrand;		/* overstory color */
    CC[2].Red -= sunshade * CC[2].Red;			/* shading */
    CC[2].Grn = PARC_MCOL_ECO(i, 1) + greenrand;
    CC[2].Grn -= sunshade * CC[2].Grn;
    CC[2].Blu = PARC_MCOL_ECO(i, 2) + bluerand;
    CC[2].Blu -= sunshade * CC[2].Blu;
    } /* if no tree model */
   else
    {
    for (j=0; j<FM[i].Items-1; j++)
     {
     if (treerand < FM[i].TM[j].Pct)
      break;
     }
    CC[2].Red = FM[i].TM[j].Red + redrand;		/* overstory color */
    CC[2].Red -= sunshade * CC[2].Red;			/* shading */
    CC[2].Grn = FM[i].TM[j].Grn + greenrand;
    CC[2].Grn -= sunshade * CC[2].Grn;
    CC[2].Blu = FM[i].TM[j].Blu + bluerand;
    CC[2].Blu -= sunshade * CC[2].Blu;
    treeheight =
	treerand * treehtfact * FM[i].TM[j].Ht / 21.34;	/* ht in meters */
    treedraw = 2;
    EcoClass = FM[i].TM[j].Class;
    return (understory);
    } /* else tree model */
   } /* if tree */

  if ((PAR_TYPE_ECO(understory) & 0x00ff) < 100 && PARC_RNDRDN_ECO(understory) > 100.0 - (treerand2 * 100.0))
   {
   undertreedraw = 1;
   CC[1].Red = PARC_SCOL_ECO(i, 0) + redrand;			/* understory color */
   CC[1].Red -= sunshade * CC[1].Red;			/* shading */
   CC[1].Grn = PARC_SCOL_ECO(i, 1) + greenrand;
   CC[1].Grn -= sunshade * CC[1].Grn;
   CC[1].Blu = PARC_SCOL_ECO(i, 2) + bluerand;
   CC[1].Blu -= sunshade * CC[1].Blu;
   } /* if draw understory tree */
  } /* if notsnow */

 else if (! settings.rendertrees)
  return (understory);

 else
  {
  if ((PAR_TYPE_ECO(i) & 0x00ff) < 100 && PARC_RNDRDN_ECO(i) > treerand2 * 100.0)
   {
   if (! FM[i].TM)
    {
    treedraw = 1;
    if (PARC_RNDRHT_ECO(i) > 1)
     {
     CC[2].Red = PARC_MCOL_ECO(i, 0) + redrand;		/* overstory color */
     CC[2].Red -= sunshade * CC[2].Red;			/* shading */
     CC[2].Grn = PARC_MCOL_ECO(i, 1) + greenrand;
     CC[2].Grn -= sunshade * CC[2].Grn;
     CC[2].Blu = PARC_MCOL_ECO(i, 2) + bluerand;
     CC[2].Blu -= sunshade * CC[2].Blu;
     } /* if it's really a tree, color it a tree */
    else
     {
     CC[2].Red = CC[0].Red;
     CC[2].Grn = CC[0].Grn;
     CC[2].Blu = CC[0].Blu;
     } /* else it's only a shrub, cover it with snow */
    } /* if no tree model */
   else
    {
    for (j=0; j<FM[i].Items-1; j++)
     {
     if (treerand < FM[i].TM[j].Pct)
      break;
     }
    if (FM[i].TM[j].Ht > 1)
     {
     CC[2].Red = FM[i].TM[j].Red + redrand;		/* overstory color */
     CC[2].Red -= sunshade * CC[2].Red;			/* shading */
     CC[2].Grn = FM[i].TM[j].Grn + greenrand;
     CC[2].Grn -= sunshade * CC[2].Grn;
     CC[2].Blu = FM[i].TM[j].Blu + bluerand;
     CC[2].Blu -= sunshade * CC[2].Blu;
     } /* if tree height > 1 */
    else
     {
     CC[2].Red = CC[0].Red;
     CC[2].Grn = CC[0].Grn;
     CC[2].Blu = CC[0].Blu;
     } /* else cover with snow */
    treeheight = 
	treerand * treehtfact * FM[i].TM[j].Ht / 21.34;	/* ht in meters */
    treedraw = 2;
    EcoClass = FM[i].TM[j].Class;
    return (understory);
    } /* else tree model */
   } /* if draw tree */

  if ((PAR_TYPE_ECO(PAR_UNDER_ECO(i)) & 0x00ff) < 100 && PARC_RNDRDN_ECO(PAR_UNDER_ECO(i)) > 100.0 - (treerand2 * 100.0))
   {
   undertreedraw = 1;
   if (PARC_RNDRHT_ECO(PAR_UNDER_ECO(i)) > 1)
    {
    SetScreenColor((PAR_TYPE_ECO(PAR_UNDER_ECO(i)) & 0x00ff) % 50);

    CC[1].Red = PARC_SCOL_ECO(i, 0) + redrand;			/* understory color */
    CC[1].Red -= sunshade * CC[1].Red;			/* shading */
    CC[1].Grn = PARC_SCOL_ECO(i, 1) + greenrand;
    CC[1].Grn -= sunshade * CC[1].Grn;
    CC[1].Blu = PARC_SCOL_ECO(i, 2) + bluerand;
    CC[1].Blu -= sunshade * CC[1].Blu;
    understory = PAR_UNDER_ECO(i);
    } /* if understory tree ht > 1 */
   else
    {
    CC[1].Red = CC[0].Red;
    CC[1].Grn = CC[0].Grn;
    CC[1].Blu = CC[0].Blu;
    } /* else it's only a shrub, cover it with snow */
   } /* if draw understory trees */
  } /* else snow on ground */

 return (understory);

} /* ecoset() */

/*********************************************************************/

short WaterEco_Set(short MakeWater, struct ColorComponents *CC)
{
 short j, eco;
 double waveamp, WaterDepth, waterfactor, WhiteCap, ElDepth;

 WaterDepth = SeaLevel - (polyel[b][0] + polyel[b][1] + polyel[b][2]) / 3.0;
 WaterDepth += (Random * 5.0 * WaterDepth);
 ElDepth = MaxSeaLevel - el;
 ElDepth += (Random * 5.0 * ElDepth);
 seadepthfact = WaterDepth / PARC_RNDRSK_ECO(0);
 if (seadepthfact > 1.0) seadepthfact = 1.0;
 else if (seadepthfact < 0.0) seadepthfact = 0.0;

/* for ocean, the skew variable becomes water depth */

 if (MakeWater < 2)	/* water or foam */
  {
  waveamp = el - SeaLevel;
  if (Tsunami)
   WhiteCap = (Tsunami->WhiteCapHt / 10.0) * WaterDepth;

  if (Tsunami && waveamp > WhiteCap && MakeWater)
   {
   CC[0].Red  = PARC_RNDR_COLOR(12, 0) + redrand;
   CC[0].Grn  = PARC_RNDR_COLOR(12, 1) + greenrand;
   CC[0].Blu  = PARC_RNDR_COLOR(12, 2) + bluerand;
   CC[0].Red -= pow(sunshade, 3.0) * CC[0].Red;
   CC[0].Grn -= pow(sunshade, 3.0) * CC[0].Grn;
   CC[0].Blu -= pow(sunshade, 3.0) * CC[0].Blu;
   Reflections = 255;
   eco = 0;
   } /* if foam */
  else
   {
   if (MakeWater)
    {
    waterfactor = (MaxWaveAmp + ElDepth) / (2.0 * MaxWaveAmp);	/* 0.5 - 2.0 */
    Reflections = 254 * ReflectionStrength;
    if (waterfactor >= 1.0)
     {
     waterfactor = 1.0;
     }
    } /* if */
   else
    {
    waterfactor = 1.0;
    Reflections = 254;	/* this should be based on a user-defined value */
    } /* else */
   CC[0].Red = PARC_MCOL_ECO(0, 0) + redrand;
   CC[0].Grn = PARC_MCOL_ECO(0, 1) + greenrand;
   CC[0].Blu = PARC_MCOL_ECO(0, 2) + bluerand;

   if (WaterDepth <= 100.0)
    {
    colavg = 1.0 - (WaterDepth + 50.0) / 150.0;
    CC[0].Red += (PARC_MCOL_ECO(2, 0) - CC[0].Red) * colavg;
    CC[0].Grn += (PARC_MCOL_ECO(2, 1) - CC[0].Grn) * colavg;
    CC[0].Blu += (PARC_MCOL_ECO(2, 2) - CC[0].Blu) * colavg;
    } /* if */

   CC[0].Red -= (CC[0].Red * seadepthfact);
   CC[0].Grn -= (CC[0].Grn * seadepthfact);
   CC[0].Blu -= (CC[0].Blu * seadepthfact);
   CC[0].Red -= (CC[0].Red * sunshade);
   CC[0].Grn -= (CC[0].Grn * sunshade);
   CC[0].Blu -= (CC[0].Blu * sunshade);

   eco = 0;
   } /* else not foam */ 
  } /* if make it water color */
 else
  {
  if (slope <= PARC_MXSL_ECO(2))
   {
   CC[0].Red = PARC_MCOL_ECO(2, 0) + redrand;
   CC[0].Grn = PARC_MCOL_ECO(2, 1) + greenrand;
   CC[0].Blu = PARC_MCOL_ECO(2, 2) + bluerand;
   eco = 2;
   }
  else
   {
   CC[0].Red = PARC_MCOL_ECO(3, 0) + redrand;
   CC[0].Grn = PARC_MCOL_ECO(3, 1) + greenrand;
   CC[0].Blu = PARC_MCOL_ECO(3, 2) + bluerand;
   eco = 3;
   }
  CC[0].Red -= (CC[0].Red * sunshade);
  CC[0].Grn -= (CC[0].Grn * sunshade);
  CC[0].Blu -= (CC[0].Blu * sunshade);
  } /* else beach */
 FloatCol = 2.0 + 3.99 * seadepthfact * sunshade;
 ColMax = COL_WATER_MAX;

 if (FM[0].TM)
  {
  if (PARC_RNDRDN_ECO(0) > treerand2 * 100.0 && settings.rendertrees)
   {
   for (j=0; j<FM[0].Items-1; j++)
    {
    if (treerand < FM[0].TM[j].Pct)
     break;
    }
   CC[2].Red = FM[0].TM[j].Red + redrand;		/* overstory color */
   CC[2].Red -= (CC[2].Red * seadepthfact);
   CC[2].Red -= (CC[2].Red * sunshade);
   CC[2].Grn = FM[0].TM[j].Grn + greenrand;
   CC[2].Grn -= (CC[2].Grn * seadepthfact);
   CC[2].Grn -= (CC[2].Grn * sunshade);
   CC[2].Blu = FM[0].TM[j].Blu + bluerand;
   CC[2].Blu -= (CC[2].Blu * seadepthfact);
   CC[2].Blu -= (CC[2].Blu * sunshade);
   treeheight =
	treerand * treehtfact * FM[0].TM[j].Ht / 21.34;	/* ht in meters */
   treedraw = 2;
   eco = EcoClass = FM[0].TM[j].Class;
   }
  } /* if water model */

 return (eco);

} /* WaterEco_Set() */

/*********************************************************************/

void colmapavg(struct elmapheaderV101 *map, short colpts, struct ColorComponents *CC)
{
 switch (colpts)
  {
  case 1:
   {
   altred = colmap[0][map->facept[0]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[0]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[0]];
   altblue -= sunshade * altblue;
   CC->Red = (CC->Red * 2.0 + altred) / 3.0;
   CC->Grn = (CC->Grn * 2.0 + altgreen) / 3.0;
   CC->Blu = (CC->Blu * 2.0 + altblue) / 3.0;
   break;
   }
  case 2:
   {
   altred = colmap[0][map->facept[1]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[1]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[1]];
   altblue += sunshade * PARC_RNDR_COLOR(1, 2);
   CC->Red = (CC->Red * 2.0 + altred) / 3.0;
   CC->Grn = (CC->Grn * 2.0 + altgreen) / 3.0;
   CC->Blu = (CC->Blu * 2.0 + altblue) / 3.0;
   break;
   }
  case 3:
   {
   altred = colmap[0][map->facept[0]] + colmap[0][map->facept[1]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[0]] + colmap[1][map->facept[1]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[0]] + colmap[2][map->facept[1]];
   altblue -= sunshade * altblue;
   CC->Red = (CC->Red + altred)/3.0;
   CC->Grn = (CC->Grn + altgreen) / 3.0;
   CC->Blu = (CC->Blu + altblue) / 3.0;
   break;
   }
  case 4:
   {
   altred = colmap[0][map->facept[2]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[2]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[2]];
   altblue -= sunshade * altblue;
   CC->Red = (CC->Red * 2.0 + altred) / 3.0;
   CC->Grn = (CC->Grn * 2.0 + altgreen) / 3.0;
   CC->Blu = (CC->Blu * 2.0 + altblue) / 3.0;
   break;
   }
  case 5:
   {
   altred = colmap[0][map->facept[0]] + colmap[0][map->facept[2]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[0]] + colmap[1][map->facept[2]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[0]] + colmap[2][map->facept[2]];
   altblue -= sunshade * altblue;
   CC->Red = (CC->Red + altred)/3.0;
   CC->Grn = (CC->Grn + altgreen) / 3.0;
   CC->Blu = (CC->Blu + altblue) / 3.0;
   break;
   }
  case 6:
   {
   altred = colmap[0][map->facept[1]] + colmap[0][map->facept[2]];
   altred -= sunshade * altred;				/* shading */
   altgreen = colmap[1][map->facept[1]] + colmap[1][map->facept[2]];
   altgreen -= sunshade * altgreen;
   altblue = colmap[2][map->facept[1]] + colmap[2][map->facept[2]];
   altblue -= sunshade * altblue;
   CC->Red = (CC->Red + altred) / 3.0;
   CC->Grn = (CC->Grn + altgreen) / 3.0;
   CC->Blu = (CC->Blu + altblue) / 3.0;
   break;
   }
  } /* switch colpts */

} /* colmapavg() */

/*********************************************************************/

static void seashoal(struct ColorComponents *CC) // used locally only -> static, AF 26.7.2021
{

 if ((SeaLevel - el < 6.0) || (SeaLevel - el > 16 && SeaLevel - el < 20))
  {
  double sinSkewLat = sin(PARC_SKLT_ECO(0)),
	 sinaspect  = sin(aspect),
	 cosSkewLat = cos(PARC_SKLT_ECO(0)),
	 cosaspect  = cos(aspect);

  if (abs(sinSkewLat - sinaspect) < .5 && abs(cosSkewLat - cosaspect) < .5)
   { 
   CC->Red=230;
   CC->Grn=240;
   CC->Blu=250;
   CC->Red -= CC->Red * sunshade;
   CC->Grn -= CC->Grn * sunshade;
   CC->Blu -= CC->Blu * sunshade;
   return;
   } /* wind and slope within 45 degrees bearing */
  } /* if correct depth for breakers */

//watercolor:
 colavg = 1.0 - (SeaLevel + 150 - el) / 250.0;
 altred = PARC_MCOL_ECO(10, 0) - sunshade * PARC_MCOL_ECO(2, 0);
 altgreen = PARC_MCOL_ECO(10, 1) - sunshade * PARC_MCOL_ECO(2, 1);
 altblue = PARC_MCOL_ECO(10, 2) - sunshade * PARC_MCOL_ECO(2, 2);
 CC->Red += (altred - CC->Red) * colavg;
 CC->Grn += (altgreen - CC->Grn) * colavg;
 CC->Blu += (altblue - CC->Blu) * colavg;

} /* seashoal() */

/***********************************************************************/

static void SetScreenColor(short ecotype) // used locally only -> static, AF 26.7.2021
{

  switch (ecotype)
   {
   case 0:
    {				/* water */
    FloatCol = 2.0 + 3.99 * sunfactor;
    ColMax = COL_WATER_MAX;
    break;
    }
   case 1:				/* snow */
    {
    FloatCol = 1.0 + 4.99 * sunfactor;
    ColMax = COL_SNOW_MAX;
    break;
    }
   case 2:				/* rock */
   case 7:				/* snag */
   case 8:				/* stump */
    {
    FloatCol = 13.0 + 2.99 * sunfactor;
    ColMax = COL_ROCK_MAX;
    break;
    }
   case 3:				/* bare */
    {
    FloatCol = 12.0 + 3.99 * sunfactor;
    ColMax = COL_BARE_MAX;
    break;
    }
   case 4:				/* conifer */
    {
    FloatCol = 7.0 + 4.99 * sunfactor;
    ColMax = COL_CONIF_MAX;
    break;
    }
   case 5:				/* deciduous */
    {
    FloatCol = 6.0 + 5.99 * sunfactor;
    ColMax = COL_DECID_MAX;
    break;
    }
   case 6:				/* low vegetation */
    {
    FloatCol = 6.0 + 3.99 * sunfactor;
    ColMax = COL_LOWVEG_MAX;
    break;
    }
   } /* switch type */

} /* SetScreenColor() */

/***********************************************************************/
#define DIP_LATITUDE  0.0	/*5000.0*/
#define DIP_LONGITUDE 0.0	/*4000.0*/

short ComputeTexture(double ElPt, double LatPt, double LonPt, double ElY)
{
long TexEl, SumEl = 0, MacroSum, FirstPass = 1, FirstTexEl, LastTexEl;
double FirstFraction, LastFraction, LastEl, OrigEl, SumSamp;

 ElPt += (LatPt * settings.stratadip);
 ElPt += (LonPt * settings.stratastrike);
 if (settings.deformationmap)
  ElPt -= (settings.deformscale * DEM_InterpPt(&DeformMap, LatPt, LonPt));
 ElPt += MakeNoise(NoiseMap, 50, LonPt, LatPt);
	/* reverse noise map orientation so features don't
	 correspond with darkness noise below */
 ElPt += 100000.0;	/* negative numbers make poor array indices */

 OrigEl = ElPt;

RepeatTex:

 LastEl = ElPt + ElY;

 if (ElY == 0.0)
  {
  TexEl = ceil(ElPt);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumEl = StrataTex[TexEl];
  }
 else if (ElY < 0.0)
  {
  TexEl = FirstTexEl = ceil(ElPt);
  FirstFraction = 1.0 - (TexEl - ElPt);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumEl += (long)(StrataTex[TexEl] * FirstFraction);
  TexEl --;
  ElPt -= (1.0 + FirstFraction);
  SumSamp = FirstFraction;

  LastTexEl = ceil(LastEl);
  if (LastTexEl != FirstTexEl)
   {
   for (; ElPt>=LastEl; TexEl--, ElPt-=1.0)
	/* The equal test is important since LastFraction can be 0 */
    {
    if (TexEl < 0)
     TexEl = 1199;
    SumEl += StrataTex[TexEl];
    SumSamp += 1.0;
    } /* for ... */

   if (TexEl < 0)
    TexEl = 1199;
   LastFraction = LastTexEl - LastEl;
   SumEl += (long)(StrataTex[TexEl] * LastFraction);
   SumSamp += LastFraction;
   } /* if */

  SumEl /= SumSamp;
  } /* if proceed in top down direction */
 else
  {
  TexEl = FirstTexEl = floor(ElPt);
  FirstFraction = 1.0 - (ElPt - TexEl);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumEl += (long)(StrataTex[TexEl] * FirstFraction);
  TexEl ++;
  ElPt += (1.0 + FirstFraction);
  SumSamp = FirstFraction;

  LastTexEl = floor(LastEl);
  if (LastTexEl != FirstTexEl)
   {
   for (; ElPt<=LastEl; TexEl++, ElPt+=1.0)
	/* The equal test is important since LastFraction can be 0 */
    {
    if (TexEl > 1199)
     TexEl = 0;
    SumEl += StrataTex[TexEl];
    SumSamp += 1.0;
    } /* for ... */

   if (TexEl > 1199)
    TexEl = 0;
   LastFraction = LastEl - LastTexEl;
   SumEl += (long)(StrataTex[TexEl] * LastFraction);
   SumSamp += LastFraction;
   } /* if */

  SumEl /= SumSamp;
  } /* if proceed bottom upwards */

/* MicroStrata have about half the amplitude and double the frequency */
 
 if (FirstPass)
  {
  ElPt = 2.0875423 * OrigEl;
  ElY *= 2.0875423;
  MacroSum = SumEl;
  SumEl = 0;
  FirstPass = 0;
  goto RepeatTex;
  } /* if */

 SumEl = SumEl * .5 + MacroSum * .5;
 SumEl = (SumEl - 128) * 2;

/* returns value from 0 to 255 */

 return ((short)(SumEl + MakeNoise(NoiseMap, (255 - SumEl) / 2, LatPt, LonPt)));

} /* ComputeTexture() */

/**********************************************************************/

short ComputeTextureColor(double ElPt, double LatPt, double LonPt, double ElY,
	struct ColorComponents *CC)
{
long TexEl, SumEl = 0, SumRed = 0, SumGrn = 0, SumBlu = 0,
	MacroSum[4], FirstPass = 1, FirstTexEl, LastTexEl;
double FirstFraction, LastFraction, LastEl, OrigEl, SumSamp;

 ElPt += (LatPt * settings.stratadip);
 ElPt += (LonPt * settings.stratastrike);
 if (settings.deformationmap)
  ElPt -= (settings.deformscale * DEM_InterpPt(&DeformMap, LatPt, LonPt));
/*
 ElPt += MakeNoise(NoiseMap, 50, LonPt, LatPt);
	// reverse noise map orientation so features don't
	// correspond with darkness noise below
*/
 ElPt += 100000.0;	/* negative numbers make poor array indices */

 OrigEl = ElPt;

RepeatTex:

 LastEl = ElPt + ElY;

 if (ElY == 0.0)
  {
  TexEl = ceil(ElPt);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumRed = PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0);
  SumGrn = PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1);
  SumBlu = PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2);
  SumEl = StrataTex[TexEl];
  }
 else if (ElY < 0.0)
  {
  TexEl = FirstTexEl = ceil(ElPt);
  FirstFraction = 1.0 - (TexEl - ElPt);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumRed += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0) * FirstFraction);
  SumGrn += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1) * FirstFraction);
  SumBlu += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2) * FirstFraction);
  SumEl += (long)(StrataTex[TexEl] * FirstFraction);
  TexEl --;
  ElPt -= (1.0 + FirstFraction);
  SumSamp = FirstFraction;

  LastTexEl = ceil(LastEl);
  if (LastTexEl != FirstTexEl)
   {
   for (; ElPt>=LastEl; TexEl--, ElPt-=1.0)
	/* The equal test is important since LastFraction can be 0 */
    {
    if (TexEl < 0)
     TexEl = 1199;
    SumRed += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0);
    SumGrn += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1);
    SumBlu += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2);
    SumEl += StrataTex[TexEl];
    SumSamp += 1.0;
    } /* for ... */

   if (TexEl < 0)
    TexEl = 1199;
   LastFraction = LastTexEl - LastEl;
   SumRed += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0) * LastFraction);
   SumGrn += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1) * LastFraction);
   SumBlu += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2) * LastFraction);
   SumEl += (long)(StrataTex[TexEl] * LastFraction);
   SumSamp += LastFraction;
   } /* if */

  SumRed /= SumSamp;
  SumGrn /= SumSamp;
  SumBlu /= SumSamp;
  SumEl /= SumSamp;
  } /* if proceed in top down direction */
 else
  {
  TexEl = FirstTexEl = floor(ElPt);
  FirstFraction = 1.0 - (ElPt - TexEl);
  TexEl %= 1200;		/* 1200 values in the array StrataTex[] */

  SumRed += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0) * FirstFraction);
  SumGrn += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1) * FirstFraction);
  SumBlu += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2) * FirstFraction);
  SumEl += (long)(StrataTex[TexEl] * FirstFraction);
  TexEl ++;
  ElPt += (1.0 + FirstFraction);
  SumSamp = FirstFraction;

  LastTexEl = floor(LastEl);
  if (LastTexEl != FirstTexEl)
   {
   for (; ElPt<=LastEl; TexEl++, ElPt+=1.0)
	/* The equal test is important since LastFraction can be 0 */
    {
    if (TexEl > 1199)
     TexEl = 0;
    SumRed += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0);
    SumGrn += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1);
    SumBlu += PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2);
    SumEl += StrataTex[TexEl];
    SumSamp += 1.0;
    } /* for ... */

   if (TexEl > 1199)
    TexEl = 0;
   LastFraction = LastEl - LastTexEl;
   SumRed += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 0) * LastFraction);
   SumGrn += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 1) * LastFraction);
   SumBlu += (PARC_RNDR_COLOR(StrataColIndex[StrataCol[TexEl]], 2) * LastFraction);
   SumEl += (long)(StrataTex[TexEl] * LastFraction);
   SumSamp += LastFraction;
   } /* if */

  SumRed /= SumSamp;
  SumGrn /= SumSamp;
  SumBlu /= SumSamp;
  SumEl /= SumSamp;
  } /* if proceed bottom upwards */

/* MicroStrata have about half the amplitude and double the frequency */
 
 if (FirstPass)
  {
  ElPt = 2.0875423 * OrigEl;
  ElY *= 2.0875423;
  MacroSum[0] = SumEl;
  MacroSum[1] = SumRed;
  MacroSum[2] = SumGrn;
  MacroSum[3] = SumBlu;
  SumEl = SumRed = SumGrn = SumBlu = 0;
  FirstPass = 0;
  goto RepeatTex;
  } /* if */

 SumEl = SumEl * .5 + MacroSum[0] * .5;
 SumEl = (SumEl - 128) * 2;	/* compensating for the range of texture values */

 CC->Red = SumRed * .5 + MacroSum[1] * .5;
 CC->Grn = SumGrn * .5 + MacroSum[2] * .5;
 CC->Blu = SumBlu * .5 + MacroSum[3] * .5;

/* returns value from 0 to 255 */

 return ((short)(SumEl + MakeNoise(NoiseMap, (255 - SumEl) / 2, LatPt, LonPt)));

} /* ComputeTextureColor() */

/*************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
short ComputeBumpMapTexture(double LatPt, double LonPt)
{

 return ((short)(200 + MakeNoise(NoiseMap, 55, LatPt * 240.0, LonPt * 240.0)));

} /* ComputeBumpMapTexture() */
#endif
/***********************************************************************/

static long MakeNoise(UBYTE *NoiseMap, long MaxNoise, double Lat, double Lon) // used locally only -> static, AF 26.7.2021
{
long Noisy, Col, Row, Colp1, Rowp1;
double Noise, LonOff, LatOff, LonInvOff, LatInvOff, wt[4], val[4];

 if (! NoiseMap)
  return (0);

 Lat -= ((int)Lat);
 Lon -= ((int)Lon);
 Lat *= 256.0;
 Lon *= 256.0;
 Col = Lon;
 Row = Lat;

 Colp1 = Col < 255 ? Col + 1: 0;
 Rowp1 = Row < 255 ? Row + 1: 0;
 LatOff = Lat - Row;
 LonOff = Lon - Col;
 LatInvOff = 1.0 - LatOff;
 LonInvOff = 1.0 - LonOff;

 wt[0] = LatInvOff * LonInvOff;
 val[0] = NoiseMap[Row * 256 + Col];
 wt[1] = LatOff * LonInvOff;
 val[1] = NoiseMap[Row * 256 + Colp1];
 wt[2] = LatOff * LonOff;
 val[2] = NoiseMap[Rowp1 * 256 + Colp1];
 wt[3] = LonOff * LatInvOff;
 val[3] = NoiseMap[Rowp1 * 256 + Col];
 Noise = (wt[0] * val[0] + wt[1] * val[1] + wt[2] * val[2] + wt[3] * val[3]);

 Noisy = (Noise * MaxNoise) / 255.0;

 return (Noisy);
 
} /* MakeNoise() */

/***********************************************************************/

double DEM_InterpPt(struct elmapheaderV101 *Map, double Lat, double Lon)
{
long Row, Col;
double InterpVal = 0.0, LonOff, LatOff, LonInvOff, LatInvOff, wt[4], val[4], Area;

 if (! Map->map)
  return (0.0);

 if (Lat >= Map->lolat &&
	Lat <= Map->lolat + (Map->columns - 1) * Map->steplat &&
	Lon <= Map->lolong &&
	Lon >= Map->lolong - Map->rows * Map->steplong)
  {
  LatOff = Lat - Map->lolat;
  LonOff = Map->lolong - Lon;
  Col = LatOff / Map->steplat;
  Row = LonOff / Map->steplong;

  LatOff -= (Col * Map->steplat);
  LonOff -= (Row * Map->steplong);

  LonInvOff = Map->steplong - LonOff;
  LatInvOff = Map->steplat - LatOff;
  
  if (Col >= Map->columns - 1)
   {
   if (Row >= Map->rows)
    {
    InterpVal = Map->map[Row * Map->columns + Col];
    }
   else
    {
    wt[0] = LonInvOff / Map->steplong;
    val[0] = Map->map[Row * Map->columns + Col];
    wt[3] = 1.0 - wt[0];
    val[3] = Map->map[(Row + 1) * Map->columns + Col];
    InterpVal = (wt[0] * val[0] + wt[3] * val[3]);
    }
   }
  else if (Row >= Map->rows)
   {
   wt[0] = LatInvOff / Map->steplat;
   val[0] = Map->map[Row * Map->columns + Col];
   wt[1] = 1.0 - wt[0];
   val[1] = Map->map[Row * Map->columns + Col + 1];
   InterpVal = (wt[0] * val[0] + wt[1] * val[1]);
   }
  else
   {
   Area = Map->steplat * Map->steplong;
   wt[0] = LatInvOff * LonInvOff / Area;
   val[0] = Map->map[Row * Map->columns + Col];
   wt[1] = LatOff * LonInvOff / Area;
   val[1] = Map->map[Row * Map->columns + Col + 1];
   wt[2] = LatOff * LonOff / Area;
   val[2] = Map->map[(Row + 1) * Map->columns + Col + 1];
   wt[3] = LonOff * LatInvOff / Area;
   val[3] = Map->map[(Row + 1) * Map->columns + Col];
   InterpVal = (wt[0] * val[0] + wt[1] * val[1] + wt[2] * val[2] + wt[3] * val[3]);
   }
  } /* if in bounds */

 return (InterpVal);


} /* DEM_InterpPt() */

/**********************************************************************/

/* Temporary routine to convert a gray-scale image profile picture of a cliff
into a volumetric texture map for stratified rock */

#define INPUT_ROW_LENGTH 640
#define INPUT_HEIGHT 1200

#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void StrataConvert(void)
{
UBYTE val;
long i, j, k, lastpt;
FILE *fStrataIn, *fStrataOut;

 if ((fStrataIn = fopen("DH0:GIS/Documents/Images&Brushes/stratatexture.gray", "rb")))
  {
  if ((fStrataOut = fopen("Ram:StrataTex", "w")))
   {
   for (i=0; i<INPUT_HEIGHT / 10; i++)
    {
    for (k=0; k<10; k++)
     {
     for (j=0, lastpt=-1; j<INPUT_ROW_LENGTH; j++)
      {
      fread((char *)&val, 1, 1, fStrataIn);
      if ((unsigned int)val > 240)
       lastpt ++;
      }
     fprintf(fStrataOut, "%ld, ", lastpt);
     } /* for k=0... */
    fprintf(fStrataOut, "\n");
    } /* for i=0... */

   fprintf(fStrataOut, "\n\n\n");
   fseek(fStrataIn, 0L, 0);

   for (i=0; i<INPUT_HEIGHT / 10; i++)
    {
    for (k=0; k<10; k++)
     {
     for (j=0, lastpt=-1; j<INPUT_ROW_LENGTH; j++)
      {
      fread((char *)&val, 1, 1, fStrataIn);
      if ((unsigned int)val > 240)
       lastpt ++;
      else if ((unsigned int)val > 10)
       {
       if ((unsigned int)val > 180)
        lastpt = 0;
       else if ((unsigned int)val > 130)
        lastpt = 1;
       else if ((unsigned int)val > 70)
        lastpt = 2;
       else
        lastpt = 3;
       } /* for j=... */
      } /* for i=... */
     fprintf(fStrataOut, "%ld, ", lastpt);
     } /* for k=0... */
    fprintf(fStrataOut, "\n");
    } /* for i=0... */

   fclose(fStrataOut);
   } /* if output file opened */
  fclose(fStrataIn);
  } /* if input file opened */

} /* StrataConvert() */
#endif
