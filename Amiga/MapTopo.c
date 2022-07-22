/* MapTopo.c (ne gismaptopo.c 14 Jan 1994 CXH)
** Maps a Topo polygon
** Grabbed/Built from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. Huber.
*/

#include "WCS.h"
#define ZBLEND

#define MODE_REPLACE 0
#define MODE_AVERAGE 1

void MapTopo(struct elmapheaderV101 *map, struct Window *win, short MapAsSFC,
	short MakeWater, short Visible, double *Elev)
{
 short eco = 0, understory = 0, notsnow = 0, NoAlias = 0, VolumeTexture = 0,
	 Texture, QInterp = 1, ElInterp = 1, i, j, k, l, sum, Mode;
 long NewSubPixArraySize, SubPixWidth, SubPixHeight, MinX, MaxX, StartY, EndY,
	 colval, x, y, zip, WtAvg;
/* changed from float to double 10/19/95 */
 double AAX[3], AAY[3], Xoffset, dist;
 double ecoline, snowline, m, h, d;
 struct QCvalues QC;
 UBYTE *PixPtr;
 struct ColorComponents CC[4];

 double fred, fgrn, fblu;
 short PolyMinXPt, PolyMaxXPt, Tex, MinPt, MidPt, MaxPt, MinQPt, MaxQPt;
 double ElX, ElY, LatX, LatY, LonX, LonY, QX, QY,
	 ElStart, LatStart, LonStart, QStart, ElPt, LatPt, LonPt, QPt,
	Temp, X5, Y5, Y4, X3, El3, EloQY;

/* initialize */

 Reflections = 0; /* enabled in WaterEco_Set() */
 treedraw = undertreedraw = 0;	/* enabled in ecoset() */

 if (OldLightingModel)
  {
  h = sunlong - facelong * PiOver180 - diplong;
  if (h > Pi) h -= TwoPi;
  else if (h < -Pi) h += TwoPi;
  d = sunlat - facelat * PiOver180 - diplat;
  if (d > Pi) d -= TwoPi;
  else if (d < -Pi) d += TwoPi;
  sunangle = sqrt(h * h + d * d) * .7071;
  if (sunangle > HalfPi) sunangle = HalfPi;
  sunfactor = 1.0 - cos(sunangle);
  sunshade = sunfactor * PARC_RNDR_MOTION(22);
  sunshade += ((1.0 - sunshade) * cloudcover);
  } /* if */

/* compute fade and fog factors */

 fade = (qqq - PARC_RNDR_MOTION(20)) / PARC_RNDR_MOTION(21);
 if (fade > 1.0) fade = 1.0;
 else if (fade < 0.0) fade = 0.0;
 if (fogrange == 0.0) fog = 0.0;
 else
  {
  fog = (el - PARC_RNDR_MOTION(23)) / fogrange;
  if (fog > 1.0) fog = 1.0;
  else if (fog < 0.0) fog = 0.0;
  } /* else */

/* if surface */

 if (MapAsSFC)
  {
  short range;
  double gradfact;

  if (settings.reliefshade || (render & 0x01))
   {
   FloatCol = 8.0 + 7.99 * sunfactor + ROUNDING_KLUDGE;
   ColMax = COL_SFC_MAX;
   } /* if shading by sun angle */
  if ((! settings.reliefshade) || (render & 0x01))
   {
   if (el < settings.surfel[1])
    {
    gradfact = ((double)settings.surfel[1] - el)
	 / (settings.surfel[1] - settings.surfel[0]);
    range = 0;
    } /* low gradient */
   else if (el >= settings.surfel[1] && el < settings.surfel[2])
    {
    gradfact = ((double)settings.surfel[2] - el)
	 / (settings.surfel[2] - settings.surfel[1]);
    range = 1;
    } /* else if middle gradient */
   else
    {
    gradfact = ((double)settings.surfel[3] - el)
	 / (settings.surfel[3] - settings.surfel[2]);
    range = 2;
    } /* else upper gradient */
   if (gradfact > 1.0) gradfact = 1.0;
   else if (gradfact < 0.0) gradfact = 0.0;
   FloatCol = 8.0 + 7.99 * gradfact + ROUNDING_KLUDGE;
   ColMax = COL_SFC_MAX;
   } /* if */

  if (render & 0x01)
   {
   switch (range)
    {
    case 0:
     {
     CC[0].Red = PARC_RNDR_COLOR(7, 0);
     CC[0].Red += (PARC_RNDR_COLOR(6, 0) - CC[0].Red) * gradfact;
     CC[0].Grn = PARC_RNDR_COLOR(7, 1);
     CC[0].Grn += (PARC_RNDR_COLOR(6, 1) - CC[0].Grn) * gradfact;
     CC[0].Blu = PARC_RNDR_COLOR(7, 2);
     CC[0].Blu += (PARC_RNDR_COLOR(6, 2) - CC[0].Blu) * gradfact;
     break;
     } /* low range */
    case 1:
     {
     CC[0].Red = PARC_RNDR_COLOR(8, 0);
     CC[0].Red += (PARC_RNDR_COLOR(7, 0) - CC[0].Red) * gradfact;
     CC[0].Grn = PARC_RNDR_COLOR(8, 1);
     CC[0].Grn += (PARC_RNDR_COLOR(7, 1) - CC[0].Grn) * gradfact;
     CC[0].Blu = PARC_RNDR_COLOR(8, 2);
     CC[0].Blu += (PARC_RNDR_COLOR(7, 2) - CC[0].Blu) * gradfact;
     break;
     } /* middle range */
    case 2:
     {
     CC[0].Red = PARC_RNDR_COLOR(9, 0);
     CC[0].Red += (PARC_RNDR_COLOR(8, 0) - CC[0].Red) * gradfact;
     CC[0].Grn = PARC_RNDR_COLOR(9, 1);
     CC[0].Grn += (PARC_RNDR_COLOR(8, 1) - CC[0].Grn) * gradfact;
     CC[0].Blu = PARC_RNDR_COLOR(9, 2);
     CC[0].Blu += (PARC_RNDR_COLOR(8, 2) - CC[0].Blu) * gradfact;
     break;
     } /* high range */
    } /* switch color range */
   CC[0].Red -= CC[0].Red * sunshade;
   CC[0].Grn -= CC[0].Grn * sunshade;
   CC[0].Blu -= CC[0].Blu * sunshade;
   } /* if render to RGB */

  eco = understory = 50;

  goto EndDrawFace;
  } /* if surface */

/* compute the shading factors for normal (not water) ecosystems */

 relfactor = relel + 100.0 * Random;

/* see if there is snow on the ground */

 snowline = PARC_RNDRLN_ECO(1) +
	diplong * PARC_SKLN_ECO(1) + diplat * PARC_SKLT_ECO(1)
        + PARC_RNDRRE_ECO(1) * relfactor;
 if (settings.worldmap)
  snowline -= ((fabs(facelat) - fabs(settings.globreflat)) * settings.globecograd);
 if (settings.flatteneco)
  snowline += ( (PARC_RNDR_MOTION(13) - snowline) * PARC_RNDR_MOTION(12) );

 if (el >= snowline)
  {
  if (slope >= PARC_MNSL_ECO(1) && slope <= PARC_MXSL_ECO(1))
   {
   if (relfactor >= PARC_RNDRNR_ECO(1)
		&& relfactor <= PARC_RNDRXR_ECO(1))
    {
    FloatCol = 1.0 + 4.99 * sunfactor;
    ColMax = COL_SNOW_MAX;
    CC[0].Red  = PARC_MCOL_ECO(1, 0);
    CC[0].Red -= pow(sunshade, 3.3) * CC[0].Red * PARC_RNDR_MOTION(22);
    CC[0].Grn  = PARC_MCOL_ECO(1, 1);
    CC[0].Grn -= pow(sunshade, 3.4) * CC[0].Grn * PARC_RNDR_MOTION(22);
    CC[0].Blu  = PARC_MCOL_ECO(1, 2);
    CC[0].Blu -= pow(sunshade, 3.8) * CC[0].Blu * PARC_RNDR_MOTION(22);
    } /* if relel matchup */
   else notsnow = 1;
   } /* if slope matchup */
  else notsnow = 1;
  } /* if el > snowline */
 else notsnow = 1;

/* We'll add some randomness to our colors for other than snow.
** Since "random" varies from -.1 to +.1, this will give us a variation of
** ±20 */

 switch (dir)
  {
  case 0:
   redrand = Random * 200.0;
   greenrand = Random * 200.0;
   bluerand = 0;
   break;
  case 1:
   redrand = 0;
   greenrand = Random * 200.0;
   bluerand = Random * 200.0;
   break;
  case 2:
   redrand = Random * 200.0;
   greenrand = 0;
   bluerand = Random * 200.0;
   break;
  case 3:
   redrand = 0;
   greenrand = Random * 200.0;
   bluerand = 0;
   break;
  } /* switch */

/* if below water level, make it wet */

 if (MakeWater || el <= SeaLevel)
  {
  eco = WaterEco_Set(MakeWater, CC);
  understory = PAR_UNDER_ECO(eco);
  ecocount[eco] ++;
  goto EndDrawFace;
  } /* if make water */

/* Do the colormap thing. If it finds a result >= 0, we're outa here! */

 if (cmap)
  {
  if ((eco = colormap(map, notsnow, CC, &understory)) >= 0)
   {
   ecocount[eco] ++;
   goto EndDrawFace;
   }
  } /* if cmap */

/* We now know whether there is snow on the ground and that this is not
** a color-mapped polygon. Now search for an ecosystem match between
** the physical terrain conditions and the ecosystem parameter list */

 for (eco=12; eco<ECOPARAMS; eco++)
  {
  ecoline = PARC_RNDRLN_ECO(eco) +
	diplong * PARC_SKLN_ECO(eco) + diplat * PARC_SKLT_ECO(eco)
        + PARC_RNDRRE_ECO(eco) * relfactor;
  if (settings.worldmap)
   ecoline -= ((fabs(facelat) - fabs(settings.globreflat)) * settings.globecograd);
  if (settings.flatteneco)
   ecoline += ( (PARC_RNDR_MOTION(13) - ecoline) * PARC_RNDR_MOTION(12) );

  if (el <= ecoline)
   {
   if (slope >= PARC_MNSL_ECO(eco) && slope <= PARC_MXSL_ECO(eco))
    {
    if (relfactor >= PARC_RNDRNR_ECO(eco) &&
		relfactor <= PARC_RNDRXR_ECO(eco))
     {
     understory = ecoset(eco, notsnow, CC);
     break;
     } /* if relel matchup */
    } /* if slope matchup */
   } /* if elevation matchup */
  } /* for eco=12... */

 if (eco >= ECOPARAMS)
  {
  Log(WNG_ILL_VAL, (CONST_STRPTR)"Ecosystem out of range.");
  eco = settings.defaulteco;
  understory = ecoset(eco, notsnow, CC);
  } /* if */
 ecocount[eco] ++;

/* Now hopefully, we have colors in all the CC arrays we need:
** CC[0] = the color of the bare ground (the understory's understory color
**  unless there is snow).
** CC[1] = the understory texture color (only used if "undertreedraw"
**  is 1 or more. It could be trees or rock texture, grass or whatever.
** CC[2] = the overstory color if there are trees or textures in the overstory.
**  If the understory and overstory are the same ecosystem, this one will not
**  be drawn since it would be rather redundant.
** We will now draw the plain polygon, unadorned by any bitmapped textures.
**  If the ecosystem understory class is strata rock then the polygons will
**  be given the strata treatment.
*/

EndDrawFace:

 if (Visible)
  {
  if (render & 0x100)
   {
   long ct;

   QC.compval1 = (long)(min(el, 32767.0)) * 65536 + eco * 256 + understory;
   ct = aspect < 0.0 ? 0: aspect * PiUnder180 - 90.0;
   if (ct < 0) ct  += 360;
   QC.compval2 = ((long)relfactor + 1000) * 65536 + ct;
   QC.compval3 = (short)(slope * PiUnder180) * 256 + (short)(sunangle * PiUnder180);
   } /* if render to QC buffer */

  if (render & 0x01)
   {
   CC[3].Red = sunshade * PARC_RNDR_COLOR(1, 0);	/* ambient pre-calc */
   CC[3].Grn = sunshade * PARC_RNDR_COLOR(1, 1);
   CC[3].Blu = sunshade * PARC_RNDR_COLOR(1, 2);
/* This is now done on a pixel basis to facilitate texture mapping
   CC[0].Red += sunshade * PARC_RNDR_COLOR(1, 0);	// ambient
   CC[0].Grn += sunshade * PARC_RNDR_COLOR(1, 1);
   CC[0].Blu += sunshade * PARC_RNDR_COLOR(1, 2);
   CC[0].Red += (PARC_RNDR_COLOR(2, 0) - CC[0].Red) * fade;
   CC[0].Red += (PARC_RNDR_COLOR(2, 0) - CC[0].Red) * fog;
   CC[0].Red *= redsun; 
   CC[0].Grn += (PARC_RNDR_COLOR(2, 1) - CC[0].Grn) * fade;
   CC[0].Grn += (PARC_RNDR_COLOR(2, 1) - CC[0].Grn) * fog;
   CC[0].Grn *= greensun;
   CC[0].Blu += (PARC_RNDR_COLOR(2, 2) - CC[0].Blu) * fade;
   CC[0].Blu += (PARC_RNDR_COLOR(2, 2) - CC[0].Blu) * fog;
   CC[0].Blu *= bluesun;
   if (CC[0].Red > 255) red = 255;
   else red = (CC[0].Red < 0) ? 0: CC[0].Red;
   if (CC[0].Grn > 255) green = 255;
   else  green = CC[0].Grn < 0 ? 0: CC[0].Grn;
   if (CC[0].Blu > 255) blue = 255;
   else blue = CC[0].Blu < 0 ? 0: CC[0].Blu;
*/
   } /* if render to RGB */

/* determine which poly corner is largest and smallest along x axis */

/* temporary switch - should also be enabled in settings editor */

  Texture = PAR_TYPE_ECO(PAR_UNDER_ECO(understory));
  if (! MapAsSFC)
   {
   if (((Texture & 0x00ff) >= 100)
   	&& ((Texture & 0x00ff) < 150)
	&& (PARC_RNDRDN_ECO(PAR_UNDER_ECO(understory)) > treerand2 * 100.0))	/* if procedural texture */
    VolumeTexture = 1;
   }

  if (polyx[b][2] > polyx[b][0])
   {
   if (polyx[b][2] > polyx[b][1])
    {
    PolyMaxXPt = 2;
    PolyMinXPt = polyx[b][1] < polyx[b][0] ? 1: 0;
    } /* if */
   else
    {
    PolyMaxXPt = 1;
    PolyMinXPt = polyx[b][2] < polyx[b][0] ? 2: 0;
    } /* else */
   } /* if */
  else
   {
   if (polyx[b][0] > polyx[b][1])
    {
    PolyMaxXPt = 0;
    PolyMinXPt = polyx[b][2] < polyx[b][1] ? 2: 1;
    } /* if */
   else
    {
    PolyMaxXPt = 1;
    PolyMinXPt = polyx[b][2] < polyx[b][0] ? 2: 0;
    } /* else */
   } /* else */

  if (polyy[b][0] < 0.0) yy[0] --;
  if (polyy[b][1] < 0.0) yy[1] --;
  if (polyy[b][2] < 0.0) yy[2] --;
  if (polyx[b][0] < 0.0) xx[0] --;
  if (polyx[b][1] < 0.0) xx[1] --;
  if (polyx[b][2] < 0.0) xx[2] --;

  MinX = xx[PolyMinXPt];
  MaxX = xx[PolyMaxXPt];

  if (MaxX < 0 || MinX > wide || yy[0] > high || yy[2] < 0)
   goto DrawTrees;

  SubPixWidth = 10 * (1 + MaxX - MinX);
  SubPixHeight = 10 * (1 + yy[2] - yy[0]);
  NewSubPixArraySize = SubPixWidth * SubPixHeight;
  if (NewSubPixArraySize > SubPixArraySize)
   {
   free_Memory(SubPix, SubPixArraySize);
   SubPixArraySize = 0;
   if ((SubPix = get_Memory(NewSubPixArraySize, MEMF_CLEAR)) == NULL)
    {
    NoAlias = 1;
    }
   else
    {
    SubPixArraySize = NewSubPixArraySize;
    }
   } /* if need larger array */
  else
   {
   memset(SubPix, 0, NewSubPixArraySize);
   } /* else clear as much of array as needed */
  if (EdgeSize < SubPixHeight * sizeof (short))
   {
   free_Memory(Edge1, EdgeSize);
   free_Memory(Edge2, EdgeSize);
   EdgeSize = 0;
   Edge1 = (short *)get_Memory(SubPixHeight * sizeof (short), MEMF_CLEAR);
   Edge2 = (short *)get_Memory(SubPixHeight * sizeof (short), MEMF_CLEAR);
   if (! Edge1 || ! Edge2)
    {
    NoAlias = 1;
    }
   else
    {
    EdgeSize = SubPixHeight * sizeof (short);
    }
   } /* if need new edge arrays */

  if (! NoAlias)
   {
   AAX[0] = 10.0 * (polyx[b][0] - MinX);
   AAX[1] = 10.0 * (polyx[b][1] - MinX);
   AAY[0] = 10.0 * (polyy[b][0] - yy[0]);
   AAY[1] = 10.0 * (polyy[b][1] - yy[0]);
   AAY[2] = 10.0 * (polyy[b][2] - yy[0]);

   if (polyy[b][0] == polyy[b][2]) m = 0.0;
   else m = (polyx[b][2] - polyx[b][0]) / (polyy[b][2] - polyy[b][0]);
   StartY = AAY[0] - (int)AAY[0] < .5 ? (int)AAY[0]: 1 + (int)AAY[0];
   EndY =   AAY[2] - (int)AAY[2] > .5 ? (int)AAY[2]: (int)AAY[2] - 1;
   Xoffset = AAX[0] + (StartY + .5 - AAY[0]) * m;

   for (y=StartY; y<=EndY; y++)
    {
    Edge1[y] = (int)Xoffset;
    Xoffset += m;
    }

   if (polyy[b][0] == polyy[b][1]) m = 0.0;
   else m = (polyx[b][1] - polyx[b][0]) / (polyy[b][1] - polyy[b][0]);
   EndY =   AAY[1] - (int)AAY[1] > .5 ? (int)AAY[1]: (int)AAY[1] - 1;
   Xoffset = AAX[0] + (StartY + .5 - AAY[0]) * m;

   for (y=StartY; y<=EndY; y++)
    {
    Edge2[y] = (int)Xoffset;
    Xoffset += m;
    }

   PixPtr = &SubPix[StartY * SubPixWidth];
   for (y=StartY; y<=EndY; y++)
    {
    if (Edge2[y] >= Edge1[y])
     {
     for (x=Edge1[y]; x<=Edge2[y]; x++)
      {
      PixPtr[x] = 0x01;
      }
     }
    else
     {
     for (x=Edge2[y]; x<=Edge1[y]; x++)
      {
      PixPtr[x] = 0x01;
      }
     }
    PixPtr += SubPixWidth;
    }

   if (polyy[b][2] == polyy[b][1]) m = 0.0;
   else m = (polyx[b][2] - polyx[b][1]) / (polyy[b][2] - polyy[b][1]);
   StartY = AAY[1] - (int)AAY[1] < .5 ? (int)AAY[1]: 1 + (int)AAY[1];
   EndY =   AAY[2] - (int)AAY[2] > .5 ? (int)AAY[2]: (int)AAY[2] - 1;
   Xoffset = AAX[1] + (StartY + .5 - AAY[1]) * m;

   for (y=StartY; y<=EndY; y++)
    {
    Edge2[y] = (int)Xoffset;
    Xoffset += m;
    }

   PixPtr = &SubPix[StartY * SubPixWidth];
   for (y=StartY; y<=EndY; y++)
    {
    if (Edge2[y] >= Edge1[y])
     {
     for (x=Edge1[y]; x<=Edge2[y]; x++)
      {
      PixPtr[x] = 0x01;
      }
     }
    else
     {
     for (x=Edge2[y]; x<=Edge1[y]; x++)
      {
      PixPtr[x] = 0x01;
      }
     }
    PixPtr += SubPixWidth;
    }

/* figure x and y gradients across polygon for lat, lon, Z and elev */

/* latitude */

   if (polylat[b][2] > polylat[b][0])
    {
    if (polylat[b][2] > polylat[b][1])
     {
     MaxPt = 2;
     MinPt = polylat[b][1] < polylat[b][0] ? 1: 0;
     MidPt = MinPt == 1 ? 0: 1;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = polylat[b][2] < polylat[b][0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* if */
   else
    {
    if (polylat[b][0] > polylat[b][1])
     {
     MaxPt = 0;
     MinPt = polylat[b][2] < polylat[b][1] ? 2: 1;
     MidPt = MinPt == 2 ? 1: 2;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = polylat[b][2] < polylat[b][0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* else */

   if (polylat[b][MaxPt] == polylat[b][MinPt])
    {
    LatX = 0.0;
    LatY = 0.0;
    } /* that was easy */
   else
    {
    Temp = (polylat[b][MidPt] - polylat[b][MinPt]) /
	(polylat[b][MaxPt] - polylat[b][MinPt]);
    X5 = Temp * (polyx[b][MaxPt] - polyx[b][MinPt]) + polyx[b][MinPt];
    Y5 = Temp * (polyy[b][MaxPt] - polyy[b][MinPt]) + polyy[b][MinPt];
    if (polyy[b][MaxPt] == polyy[b][MinPt])
     {
     LatY = LatX = 0.0;
     LatStart = facelat;
     goto StartLon;
/*    X3 = Y4 = infinity;*/
     } /* if */
    X3 = polyx[b][MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(polyx[b][MaxPt] - polyx[b][MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    if (X5 == polyx[b][MidPt])
     {
     LatY = LatX = 0.0;
     LatStart = facelat;
     goto StartLon;
/*    Y4 = infinity;*/
     } /* if */
    Y4 = polyy[b][MidPt] + (X3 - polyx[b][MidPt]) *
	(Y5 - polyy[b][MidPt]) / (X5 - polyx[b][MidPt]);
    if (polyx[b][MaxPt] != polyx[b][MinPt])
     El3 = polylat[b][MinPt] + (X3 - polyx[b][MinPt]) *
	(polylat[b][MaxPt] - polylat[b][MinPt]) /
	(polyx[b][MaxPt] - polyx[b][MinPt]);
    else if (polyy[b][MaxPt] != polyy[b][MinPt])
     El3 = polylat[b][MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(polylat[b][MaxPt] - polylat[b][MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    else
     {
     LatY = LatX = 0.0;
     LatStart = facelat;
     goto StartLon;
     } /* else */

    if (X3 != polyx[b][MidPt] && Y4 != polyy[b][MidPt])
     {
     LatX = (El3 - polylat[b][MidPt]) / (X3 - polyx[b][MidPt]);
     LatY = (El3 - polylat[b][MidPt]) / (polyy[b][MidPt] - Y4);
     } /* if */
    else
     {
     LatY = LatX = 0.0;
     LatStart = facelat;
     goto StartLon;
     } /* else */
    } /* else not so easy */
   LatStart = polylat[b][MidPt] + (polyx[b][PolyMinXPt] - polyx[b][MidPt])
	 * LatX;
   LatStart += (polyy[b][0] - polyy[b][MidPt]) * LatY;

/* longitude */

StartLon:

   if (polylon[b][2] > polylon[b][0])
    {
    if (polylon[b][2] > polylon[b][1])
     {
     MaxPt = 2;
     MinPt = polylon[b][1] < polylon[b][0] ? 1: 0;
     MidPt = MinPt == 1 ? 0: 1;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = polylon[b][2] < polylon[b][0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* if */
   else
    {
    if (polylon[b][0] > polylon[b][1])
     {
     MaxPt = 0;
     MinPt = polylon[b][2] < polylon[b][1] ? 2: 1;
     MidPt = MinPt == 2 ? 1: 2;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = polylon[b][2] < polylon[b][0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* else */

   if (polylon[b][MaxPt] == polylon[b][MinPt])
    {
    LonX = 0.0;
    LonY = 0.0;
    } /* that was easy */
   else
    {
    Temp = (polylon[b][MidPt] - polylon[b][MinPt]) /
	(polylon[b][MaxPt] - polylon[b][MinPt]);
    X5 = Temp * (polyx[b][MaxPt] - polyx[b][MinPt]) + polyx[b][MinPt];
    Y5 = Temp * (polyy[b][MaxPt] - polyy[b][MinPt]) + polyy[b][MinPt];
    if (polyy[b][MaxPt] == polyy[b][MinPt])
     {
     LonY = LonX = 0.0;
     LonStart = facelong;
     goto StartZ;
/*    X3 = Y4 = infinity;*/
     } /* if */
    X3 = polyx[b][MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(polyx[b][MaxPt] - polyx[b][MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    if (X5 == polyx[b][MidPt])
     {
     LonY = LonX = 0.0;
     LonStart = facelong;
     goto StartZ;
/*    Y4 = infinity;*/
     } /* if */
    Y4 = polyy[b][MidPt] + (X3 - polyx[b][MidPt]) *
	(Y5 - polyy[b][MidPt]) / (X5 - polyx[b][MidPt]);
    if (polyx[b][MaxPt] != polyx[b][MinPt])
     El3 = polylon[b][MinPt] + (X3 - polyx[b][MinPt]) *
	(polylon[b][MaxPt] - polylon[b][MinPt]) /
	(polyx[b][MaxPt] - polyx[b][MinPt]);
    else if (polyy[b][MaxPt] != polyy[b][MinPt])
     El3 = polylon[b][MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(polylon[b][MaxPt] - polylon[b][MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    else
     {
     LonY = LonX = 0.0;
     LonStart = facelong;
     goto StartZ;
     } /* else */

    if (X3 != polyx[b][MidPt] && Y4 != polyy[b][MidPt])
     {
     LonX = (El3 - polylon[b][MidPt]) / (X3 - polyx[b][MidPt]);
     LonY = (El3 - polylon[b][MidPt]) / (polyy[b][MidPt] - Y4);
     } /* if */
    else
     {
     LonY = LonX = 0.0;
     LonStart = facelong;
     goto StartZ;
     } /* else */
    } /* else not so easy */
   LonStart = polylon[b][MidPt] + (polyx[b][PolyMinXPt] - polyx[b][MidPt])
	* LonX;
   LonStart += (polyy[b][0] - polyy[b][MidPt]) * LonY;

StartZ:

/* z buffer */

   if (polyq[b][2] > polyq[b][0])
    {
    if (polyq[b][2] > polyq[b][1])
     {
     MaxQPt = 2;
     MinQPt = polyq[b][1] < polyq[b][0] ? 1: 0;
     MidPt = MinQPt == 1 ? 0: 1;
     } /* if */
    else
     {
     MaxQPt = 1;
     MinQPt = polyq[b][2] < polyq[b][0] ? 2: 0;
     MidPt = MinQPt == 2 ? 0: 2;
     } /* else */
    } /* if */
   else
    {
    if (polyq[b][0] > polyq[b][1])
     {
     MaxQPt = 0;
     MinQPt = polyq[b][2] < polyq[b][1] ? 2: 1;
     MidPt = MinQPt == 2 ? 1: 2;
     } /* if */
    else
     {
     MaxQPt = 1;
     MinQPt = polyq[b][2] < polyq[b][0] ? 2: 0;
     MidPt = MinQPt == 2 ? 0: 2;
     } /* else */
    } /* else */

   if (polyq[b][MaxQPt] == polyq[b][MinQPt])
    {
    QX = 0.0;
    QY = 0.0;
    } /* that was easy */
   else
    {
    Temp = (polyq[b][MidPt] - polyq[b][MinQPt]) /
	(polyq[b][MaxQPt] - polyq[b][MinQPt]);
    X5 = Temp * (polyx[b][MaxQPt] - polyx[b][MinQPt]) + polyx[b][MinQPt];
    Y5 = Temp * (polyy[b][MaxQPt] - polyy[b][MinQPt]) + polyy[b][MinQPt];
    if (polyy[b][MaxQPt] == polyy[b][MinQPt])
     {
     QInterp = 0;
     QY = QX = 0.0;
     QStart = qqq;
     goto StartEl;
/*    X3 = Y4 = infinity;*/
     } /* if */
    X3 = polyx[b][MinQPt] + (polyy[b][MidPt] - polyy[b][MinQPt]) *
	(polyx[b][MaxQPt] - polyx[b][MinQPt]) /
	(polyy[b][MaxQPt] - polyy[b][MinQPt]);
    if (X5 == polyx[b][MidPt])
     {
     QInterp = 0;
     QY = QX = 0.0;
     QStart = qqq;
     goto StartEl;
/*    Y4 = infinity;*/
     } /* if */
    Y4 = polyy[b][MidPt] + (X3 - polyx[b][MidPt]) *
	(Y5 - polyy[b][MidPt]) / (X5 - polyx[b][MidPt]);
    if (polyx[b][MaxQPt] != polyx[b][MinQPt])
     El3 = polyq[b][MinQPt] + (X3 - polyx[b][MinQPt]) *
	(polyq[b][MaxQPt] - polyq[b][MinQPt]) /
	(polyx[b][MaxQPt] - polyx[b][MinQPt]);
    else if (polyy[b][MaxQPt] != polyy[b][MinQPt])
     El3 = polyq[b][MinQPt] + (polyy[b][MidPt] - polyy[b][MinQPt]) *
	(polyq[b][MaxQPt] - polyq[b][MinQPt]) /
	(polyy[b][MaxQPt] - polyy[b][MinQPt]);
    else
     {
     QInterp = 0;
     QY = QX = 0.0;
     QStart = qqq;
     goto StartEl;
     } /* else */

    if (X3 != polyx[b][MidPt] && Y4 != polyy[b][MidPt])
     {
     QX = (El3 - polyq[b][MidPt]) / (X3 - polyx[b][MidPt]);
     QY = (El3 - polyq[b][MidPt]) / (polyy[b][MidPt] - Y4);
     } /* if */
    else
     {
     QInterp = 0;
     QY = QX = 0.0;
     QStart = qqq;
     goto StartEl;
     } /* else */
    } /* else not so easy */
   QStart = polyq[b][MidPt] + (polyx[b][PolyMinXPt] - polyx[b][MidPt])
	* QX;
   QStart += (polyy[b][0] - polyy[b][MidPt]) * QY;

/* elevation */

StartEl:

   if (Elev[2] > Elev[0])
    {
    if (Elev[2] > Elev[1])
     {
     MaxPt = 2;
     MinPt = Elev[1] < Elev[0] ? 1: 0;
     MidPt = MinPt == 1 ? 0: 1;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = Elev[2] < Elev[0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* if */
   else
    {
    if (Elev[0] > Elev[1])
     {
     MaxPt = 0;
     MinPt = Elev[2] < Elev[1] ? 2: 1;
     MidPt = MinPt == 2 ? 1: 2;
     } /* if */
    else
     {
     MaxPt = 1;
     MinPt = Elev[2] < Elev[0] ? 2: 0;
     MidPt = MinPt == 2 ? 0: 2;
     } /* else */
    } /* else */

   if (Elev[MaxPt] == Elev[MinPt])
    {
    ElX = 0.0;
    ElY = 0.0;
    } /* that was easy */
   else
    {
    Temp = (Elev[MidPt] - Elev[MinPt]) /
	(Elev[MaxPt] - Elev[MinPt]);
    X5 = Temp * (polyx[b][MaxPt] - polyx[b][MinPt]) + polyx[b][MinPt];
    Y5 = Temp * (polyy[b][MaxPt] - polyy[b][MinPt]) + polyy[b][MinPt];
    if (polyy[b][MaxPt] == polyy[b][MinPt])
     {
     ElInterp = 0;
     ElY = ElX = 0.0;
     ElStart = el;
     goto StartDraw;
/*    X3 = Y4 = infinity;*/
     } /* if */
    X3 = polyx[b][MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(polyx[b][MaxPt] - polyx[b][MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    if (X5 == polyx[b][MidPt])
     {
     ElInterp = 0;
     ElY = ElX = 0.0;
     ElStart = el;
     goto StartDraw;
/*    Y4 = infinity;*/
     } /* if */
    Y4 = polyy[b][MidPt] + (X3 - polyx[b][MidPt]) *
	(Y5 - polyy[b][MidPt]) / (X5 - polyx[b][MidPt]);
    if (polyx[b][MaxPt] != polyx[b][MinPt])
     El3 = Elev[MinPt] + (X3 - polyx[b][MinPt]) *
	(Elev[MaxPt] - Elev[MinPt]) /
	(polyx[b][MaxPt] - polyx[b][MinPt]);
    else if (polyy[b][MaxPt] != polyy[b][MinPt])
     El3 = Elev[MinPt] + (polyy[b][MidPt] - polyy[b][MinPt]) *
	(Elev[MaxPt] - Elev[MinPt]) /
	(polyy[b][MaxPt] - polyy[b][MinPt]);
    else
     {
     ElInterp = 0;
     ElY = ElX = 0.0;
     ElStart = el;
     goto StartDraw;
     } /* else */

    if (X3 != polyx[b][MidPt] && Y4 != polyy[b][MidPt])
     {
     ElX = (El3 - Elev[MidPt]) / (X3 - polyx[b][MidPt]);
     ElY = (El3 - Elev[MidPt]) / (polyy[b][MidPt] - Y4);
     } /* if */
    else
     {
     ElInterp = 0;
     ElY = ElX = 0.0;
     ElStart = el;
     goto StartDraw;
     } /* else */
    } /* else not so easy */
   ElStart = Elev[MidPt] + (polyx[b][PolyMinXPt] - polyx[b][MidPt]) * ElX;
   ElStart += (polyy[b][0] - polyy[b][MidPt]) * ElY;

StartDraw:

   if (QY != 0.0)
    EloQY = .001 * ElY / QY;
   else
    EloQY = 0.0;

   for (y=yy[0], k=0; y<=yy[2]; y++,k++,QStart+=QY,ElStart+=ElY,LatStart+=LatY,LonStart+=LonY)
    {
    if (y < 0) continue;
    if (y > oshigh) break;
    zip = scrnrowzip[y] + MinX;

    for (x=MinX, l=0, QPt=QStart, ElPt=ElStart, LatPt=LatStart, LonPt=LonStart;
	 x<=MaxX; x++, l++, zip++, QPt+=QX, ElPt+=ElX, LatPt+=LatX, LonPt+=LonX)
     {
     if (x < 0)
      continue;
     if (x > wide)
      break;
     dist = *(zbuf + zip);

     if (! QInterp)
      QPt = qqq;
     else
      {
      if (QPt < polyq[b][MinQPt])
       QPt = polyq[b][MinQPt];
      else if (QPt > polyq[b][MaxQPt])
       QPt = polyq[b][MaxQPt];
      } /* else */

     if (! ElInterp)
      ElPt = el;
     else
      {
      if (ElPt < Elev[MinPt])
       ElPt = Elev[MinPt];
      else if (ElPt > Elev[MaxPt])
       ElPt = Elev[MaxPt];
      } /* else */

     if (QPt <= dist + PtrnOffset || (unsigned int)bytemap[zip] < 100)
      {
      sum = 0;
      PixPtr = &SubPix[k * 10 * SubPixWidth + l * 10];
      for (i=0; i<10; i++)
       {
       for (j=0; j<10; j++)
        {
        sum += (unsigned int)PixPtr[j];
        } /* for j=0... */
       PixPtr += SubPixWidth;
       } /* for i=0... */
      if (sum)
       {
       if (render & 0x01)
        {
        /* changed this from ptrnoffset to HalfPtrnOffset 1/2/96 to elliminate
        spider webs */
        if (bytemap[zip] 
		 && (QPt >= dist || ((sum < 100) && (dist - QPt < HalfPtrnOffset))))
         Mode = MODE_AVERAGE;
        else
         Mode = MODE_REPLACE;

        if (VolumeTexture)
         {
         if (Texture & 0x0200)		/* if strata colors */
          {
          Tex = ComputeTextureColor(ElPt, LatPt, LonPt, ElY, &CC[0]);
          CC[0].Red -= sunshade * CC[0].Red;			/* shading */
          CC[0].Grn -= sunshade * CC[0].Grn;
          CC[0].Blu -= sunshade * CC[0].Blu;
	  } /* if */
         else if (Texture & 0x0100)		/* if plain strata */
          {
          Tex = ComputeTexture(ElPt, LatPt, LonPt, ElY);
	  } /* else if */
         else Tex = 255;
/* This is a test - replace with optional fracture texture
         Tex *= ((355 - MakeNoise(NoiseMap, 200, LatPt * 480.0, LonPt * 480.0)) / 255.0);
         if (Tex > 255)
          Tex = 255;
*/
	 } /* if strata */
/*
        else if (NoiseTexture)
         {
         Tex = ((255 - MakeNoise(NoiseMap, 75, LatPt * 480.0, LonPt * 480.0)) / 255.0);
 //        Tex = ComputeBumpMapTexture(LatPt, LonPt);
	 } // else no strata
*/
        else
         Tex = 255;

        fred = (Tex * CC[0].Red) / 255.;
        fgrn = (Tex * CC[0].Grn) / 255.;
        fblu = (Tex * CC[0].Blu) / 255.;
        fred += CC[3].Red; /* sunshade * PARC_RNDR_COLOR(1, 0);	ambient */
        fgrn += CC[3].Grn; /* sunshade * PARC_RNDR_COLOR(1, 1); */
        fblu += CC[3].Blu; /* sunshade * PARC_RNDR_COLOR(1, 2); */
        fred += (PARC_RNDR_COLOR(2, 0) - fred) * fade;
        fgrn += (PARC_RNDR_COLOR(2, 1) - fgrn) * fade;
        fblu += (PARC_RNDR_COLOR(2, 2) - fblu) * fade;
        fred += (PARC_RNDR_COLOR(2, 0) - fred) * fog;
        fgrn += (PARC_RNDR_COLOR(2, 1) - fgrn) * fog;
        fblu += (PARC_RNDR_COLOR(2, 2) - fblu) * fog;
        fred *= redsun; 
        fgrn *= greensun;
        fblu *= bluesun;
        if (fred > 255.) fred = 255.;
        else if (fred < 0.) fred = 0.;
        if (fgrn > 255.) fgrn = 255.;
        else  if (fgrn < 0.) fgrn = 0.;
        if (fblu > 255.) fblu = 255.;
        else if (fblu < 0.) fblu = 0.;

        if (Mode)
         {
         if ((unsigned int)bytemap[zip] >= 110)
          continue;	/* added 9/25/95 */
         WtAvg = (unsigned int)bytemap[zip] + sum >= 110 ? 110:
		(unsigned int)bytemap[zip] + sum; /* changed 9/25/95 */
         sum = WtAvg - bytemap[zip];	/* added 9/25/95 */
         colval = (*(bitmap[0] + zip) * (unsigned int)bytemap[zip] + fred * sum)
		 / WtAvg;
         *(bitmap[0] + zip) = (UBYTE)colval;
         colval = (*(bitmap[1] + zip) * (unsigned int)bytemap[zip] + fgrn * sum)
		 / WtAvg;
         *(bitmap[1] + zip) = (UBYTE)colval;
         colval = (*(bitmap[2] + zip) * (unsigned int)bytemap[zip] + fblu * sum)
		 / WtAvg;
         *(bitmap[2] + zip) = (UBYTE)colval;
	 } /* if already a value */
        else
         {
         *(bitmap[0] + zip) = (UBYTE)fred;
         *(bitmap[1] + zip) = (UBYTE)fgrn;
         *(bitmap[2] + zip) = (UBYTE)fblu;
	 } /* else first value */
        } /* if render to bitmaps */

       if (QPt < dist)
        {
        if (render & 0x10)
         {
         if (render & 0x01)
          {
          ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
	  } /* if render to bitmaps */
         else
          {
          NoRGBScreenPixelPlot(win, FloatCol, ColMax, x + drawoffsetX, y + drawoffsetY);
	  } /* else no bitmaps */
         } /* if render to screen */
        if (render & 0x100)
         {
         *(QCmap[0] + zip) = QC.compval1;
         *(QCmap[1] + zip) = QC.compval2;
         *(QCmap[2] + zip) = QC.compval3;
         *(QCcoords[0] + zip) = facelat;
         *(QCcoords[1] + zip) = facelong;
         } /* if render & 0x100 */
        *(zbuf + zip) = QPt;
        if (Reflections && ReflectionMap)
          ReflectionMap[zip] = Reflections;
        if (ElevationMap)
         ElevationMap[zip] = ElPt;
        if (SlopeMap)
         SlopeMap[zip] = EloQY;
        } /* if lower QPt value */
       bytemap[zip] += sum;
       } /* if at least one point */
      } /* if QPt */
     } /* for x=... */
    } /* for y=... */
   } /* if array is large enough to hold current polygon */

  } /* if polygon is visible */

/* Now we draw the understory layer if undertreedraw > 0 */

DrawTrees:

 if (undertreedraw)
  {
  DetailTree(win, understory, &QC, 1, &CC[1], &CC[3], Elev);
  } /* if understory tree draw */

/* Now we draw the overstory layer if treedraw > 0.
**  treedraw could be 2 if forest model
*/

 if (treedraw)
  {
  DetailTree(win, eco, &QC, (short)(treedraw == 1), &CC[2], &CC[3], Elev);
  } /* if treedraw */

} /* MapTopo() */

/***********************************************************************/

short colormap(struct elmapheaderV101 *map, short notsnow,
	struct ColorComponents *CC, short *understory)
{
 UBYTE matchred, matchgreen, matchblue;
 short colpts = 0, cpts = 0, eco;
 long Row, Col, facept[3];
#ifdef ENABLE_STATISTICS
 long ct, elct;
#endif /* ENABLE_STATISTICS */
 double ecoline;

/* determine number of points falling on color map */

 if (settings.mastercmap)
  {
  if (facelat >= CMap->South && facelat <= CMap->North &&
	facelong <= CMap->West && facelong >= CMap->East)
   {
   if (settings.cmaporientation)
    {
    Row = (CMap->North - facelat) / CMap->StepLat;
    Col = (CMap->West - facelong) / CMap->StepLon;
    facept[0] = facept[1] = facept[2] = Row * CMap->Cols + Col;
    } /* if image orientation */
   else
    {
    Col = (facelat - CMap->South) / CMap->StepLat;
    Row = (CMap->West - facelong) / CMap->StepLon;
    facept[0] = facept[1] = facept[2] = Row * CMap->Cols + Col;
    } /* else WCS DEM orientation */
   if (Row < CMap->Rows && Col < CMap->Cols)
    {
    if ( colmap[0][facept[0]] || colmap[1][facept[0]] || colmap[2][facept[0]])
     {
     colpts = 7;
     cpts = 3;
     } /* if */
    else
     return (-1);
    } /* if */
   else
    {
    return (-1);
    }
   } /* if in bounds */
  else
   return (-1);
  } /* if mastercmap */
 else if (settings.borderandom)
  {
  randomint = treerand * 2.99 + ROUNDING_KLUDGE;
  facept[0] = map->facept[randomint];
  facept[1] = map->facept[randomint];
  facept[2] = map->facept[randomint];
  if ( colmap[0][facept[0]] || colmap[1][facept[0]] || colmap[2][facept[0]] )
   {
   colpts = 7;
   cpts = 3;
   } /* if */
  } /* if settings.borderandom */

 else
  {
  facept[0] = map->facept[0];
  facept[1] = map->facept[1];
  facept[2] = map->facept[2];
  if ( colmap[0][facept[0]] || colmap[1][facept[0]] || colmap[2][facept[0]] )
   {
   cpts = colpts = 1;
   } /* if */
  if ( colmap[0][facept[1]] || colmap[1][facept[1]]
		|| colmap[2][facept[1]] )
   {
   colpts += 2;
   cpts += 1;
   } /* if */
  if ( colmap[0][facept[2]] || colmap[1][facept[2]]
		|| colmap[2][facept[2]] )
   {
   colpts += 4;
   cpts += 1;
   } /* if */
  } /* else not random borders */

/* if at least one color map point */

 if (cpts)
  {
#ifdef ENABLE_STATISTICS
  if (settings.statistics && cpts > 1)
   {
   ct = aspect * 5.72957795 + 1.0;
   elct = (el - settings.surfel[3]) * 20
	 / (settings.surfel[3] - settings.surfel[0]);
   if (elct < 0) elct = 0;
   else if (elct > 19) elct = 19;
   statcount[ct][elct] ++;
   meanel += el;
   meanaspect += aspect;
   meanslope +=slope;
   samples ++;
   } /* if settings.statistics */
#endif /* ENABLE_STATISTICS */

/* if match colors to ecosystems */

  if (settings.ecomatch)
   {
   switch (colpts)
    {
    case 1:
     {
     matchred = colmap[0][facept[0]];
     matchgreen = colmap[1][facept[0]];
     matchblue = colmap[2][facept[0]];
     break;
     } /* case 1 */
    case 2:
     {
     matchred = (UBYTE)colmap[0][facept[1]];
     matchgreen = (UBYTE)colmap[1][facept[1]];
     matchblue = (UBYTE)colmap[2][facept[1]];
     break;
     } /* case 2 */
    case 3:
     {
     if (treerand > .5)
      {
      matchred = (UBYTE)colmap[0][facept[0]];
      matchgreen = (UBYTE)colmap[1][facept[0]];
      matchblue = (UBYTE)colmap[2][facept[0]];
      } /* if */
     else
      {
      matchred = (UBYTE)colmap[0][facept[1]];
      matchgreen = (UBYTE)colmap[1][facept[1]];
      matchblue = (UBYTE)colmap[2][facept[1]];
      } /* else */
     break;
     } /* case 3 */
    case 4:
     {
     matchred = (UBYTE)colmap[0][facept[2]];
     matchgreen = (UBYTE)colmap[1][facept[2]];
     matchblue = (UBYTE)colmap[2][facept[2]];
     break;
     } /* case 4 */       
    case 5:
     {
     if (treerand > .5)
      {
      matchred = (UBYTE)colmap[0][facept[0]];
      matchgreen = (UBYTE)colmap[1][facept[0]];
      matchblue = (UBYTE)colmap[2][facept[0]];
      } /* if */
     else
      {
      matchred = (UBYTE)colmap[0][facept[2]];
      matchgreen = (UBYTE)colmap[1][facept[2]];
      matchblue = (UBYTE)colmap[2][facept[2]];
      } /* else */
     break;
     } /* case 5 */       
    case 6:
     {
     if (treerand > .5)
      {
      matchred = (UBYTE)colmap[0][facept[1]];
      matchgreen = (UBYTE)colmap[1][facept[1]];
      matchblue = (UBYTE)colmap[2][facept[1]];
      } /* if */
     else
      {
      matchred = (UBYTE)colmap[0][facept[2]];
      matchgreen = (UBYTE)colmap[1][facept[2]];
      matchblue = (UBYTE)colmap[2][facept[2]];
      } /* else */
     break;
     } /* case 6 */
    case 7:
     {      
     if (treerand > .67)
      {
      matchred = colmap[0][facept[0]];
      matchgreen = colmap[1][facept[0]];
      matchblue = colmap[2][facept[0]];
      } /* if */
     else if (treerand > .33)
      {
      matchred = colmap[0][facept[1]];
      matchgreen = colmap[1][facept[1]];
      matchblue = colmap[2][facept[1]];
      } /* else if */
     else
      {
      matchred = colmap[0][facept[2]];
      matchgreen = colmap[1][facept[2]];
      matchblue = colmap[2][facept[2]];
      } /* else */
     break;
     } /* case 7 */
    } /* switch colpts */

/* find ecosystem match if there is one and slope is appropriate */ 

   for (eco=0; eco<ECOPARAMS; eco++)
    {
    if (matchred == PAR_MTCH_ECO(eco, 0))
     {
     if (matchgreen == PAR_MTCH_ECO(eco, 1))
      {
      if (matchblue == PAR_MTCH_ECO(eco, 2))
       {
       if (cpts > 1 && slope <= PARC_MXSL_ECO(eco))
        {
        if (eco != 1) *understory = ecoset(eco, notsnow, CC);
        return (eco);
	} /* if cpts > 1 */
       else
        {
        return (-1);
	} /* else cpts=1 */
       } /* match blue */
      } /* match green */
     } /* match red */
    } /* for eco=0... */
   } /* if settings.ecomatch */

/* normal color mapping */

/* if all three points color mapped set color to average */
  if (cpts == 3)
   {
   CC[0].Red = ((UBYTE)colmap[0][facept[0]] + (UBYTE)colmap[0][facept[1]] +
	(UBYTE)colmap[0][facept[2]]) / 3.0;
   CC[0].Grn = ((UBYTE)colmap[1][facept[0]] + (UBYTE)colmap[1][facept[1]] +
	(UBYTE)colmap[1][facept[2]]) / 3.0;
   CC[0].Blu = ((UBYTE)colmap[2][facept[0]] + (UBYTE)colmap[2][facept[1]] +
	(UBYTE)colmap[2][facept[2]]) / 3.0;
   if (! settings.cmapluminous)
    {
    CC[0].Red -= sunshade * CC[0].Red;			/* shading */
    CC[0].Red += sunshade * PARC_RNDR_COLOR(1, 0);	/* ambient */
    CC[0].Grn -= sunshade * CC[0].Grn;
    CC[0].Grn += sunshade * PARC_RNDR_COLOR(1, 1);
    CC[0].Blu -= sunshade * CC[0].Blu;
    CC[0].Blu += sunshade * PARC_RNDR_COLOR(1, 2);
    } /* if not luminous color mapping */
   eco = settings.defaulteco;
   } /* if cpts = 3 */

/* otherwise find normal ecosystem */

  if (cpts < 3 || settings.cmaptrees)
   {  
   if (notsnow)
    {
    for (eco=12; eco<ECOPARAMS; eco++)
     {
     ecoline = PARC_RNDRLN_ECO(eco) +
	diplong * PARC_SKLN_ECO(eco) + diplat * PARC_SKLT_ECO(eco)
        + PARC_RNDRRE_ECO(eco) * relfactor;
     if (settings.worldmap)
      ecoline -= ((fabs(facelat) - fabs(settings.globreflat)) * settings.globecograd);
     if (settings.flatteneco)
      ecoline += ( (PARC_RNDR_MOTION(13) - ecoline) * PARC_RNDR_MOTION(12) );

     if (el <= ecoline)
      {
      if (slope >= PARC_MNSL_ECO(eco) && slope <= PARC_MXSL_ECO(eco))
       {
       if (relfactor >= PARC_RNDRNR_ECO(eco) &&
		relfactor <= PARC_RNDRXR_ECO(eco))
        {
        break;
        } /* if relel matchup */
       } /* if slope matchup */
      } /* if elevation matchup */
     } /* for eco=12... */

    if (eco >= ECOPARAMS)
     {
     Log(WNG_ILL_VAL, (CONST_STRPTR)"Ecosystem out of range.");
     eco = settings.defaulteco;
     } /* if */

    CC[0].Red = PARC_SCOL_ECO(eco, 0);
    CC[0].Grn = PARC_SCOL_ECO(eco, 1);
    CC[0].Blu = PARC_SCOL_ECO(eco, 2);
    if (! settings.cmapluminous)
     {
     CC[0].Red -= sunshade * CC[0].Red;			/* shading */
     CC[0].Red += sunshade * PARC_RNDR_COLOR(1, 0);	/* ambient */
     CC[0].Grn -= sunshade * CC[0].Grn;
     CC[0].Grn += sunshade * PARC_RNDR_COLOR(1, 1);
     CC[0].Blu -= sunshade * CC[0].Blu;
     CC[0].Blu += sunshade * PARC_RNDR_COLOR(1, 2);
     } /* if not luminous */
    } /* if not snow */

   colmapavg(map, colpts, &CC[0]);
   } /* if cpts < 3 || cmaptrees */
  else
   eco = settings.defaulteco;

/* set colors for rendering */

  if (settings.cmaptrees && PARC_RNDRDN_ECO(eco) > treerand2 * 100)
	 treedraw = 1;
  CC[2].Red = CC[1].Red = CC[0].Red;
  CC[2].Grn = CC[1].Grn = CC[0].Grn;
  CC[2].Blu = CC[1].Blu = CC[0].Blu;

  return (eco);

  } /* if cpts */

 else
  {
  return (-1);
  } /* else ! cpts */

} /* colormap() */

/********************************************************************/

#define SCAN_NORMAL 	1
#define SCAN_REVERSE 	0

void MapCloud(struct Window *win, short *CloudVal, short *IllumVal, long Elev)
{
short x, y, ScanOrder = SCAN_NORMAL, Edge2aHt, Edge2bHt, Edge1Ht;
long scrnrow, zip;
double m, EdgeVal, ValCX, ValCY, dCX, dCY, ValIX, ValIY, dIX, dIY;

 if (! CloudVal)
  return;

 Edge2aHt = yy[1] - yy[0];
 Edge2bHt = yy[2] - yy[1];
 Edge1Ht  = yy[2] - yy[0];

 if (EdgeSize < Edge1Ht * sizeof (short))
  {
  free_Memory(Edge1, EdgeSize);
  free_Memory(Edge2, EdgeSize);
  EdgeSize = Edge1Ht * sizeof (short);
  Edge1 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  Edge2 = (short *)get_Memory(EdgeSize, MEMF_CLEAR);
  if (! Edge1 || ! Edge2)
   {
   return;
   }
  } /* if need new edge arrays */

 if (Edge1Ht)
  {
  m = ((float)xx[2] - xx[0]) / (Edge1Ht);
  dCY = ((float)CloudVal[2] - CloudVal[0]) / (float)Edge1Ht;
  dIY = ((float)IllumVal[2] - IllumVal[0]) / (float)Edge1Ht;
  } /* if */
 else
  {
  m = 0.0;
  dCY = 0.0;
  dIY = 0.0;
  }
 EdgeVal = xx[0];
 for (y=0; y<=Edge1Ht; y++, EdgeVal += m)
  Edge1[y] = EdgeVal;

 if (Edge2aHt)
  m = ((float)xx[1] - xx[0]) / (Edge2aHt);
 else
  m = 0.0;
 EdgeVal = xx[0];
 for (y=0; y<=Edge2aHt; y++, EdgeVal += m)
  Edge2[y] = EdgeVal;

 if (Edge2bHt)
  m = ((float)xx[2] - xx[1]) / (Edge2bHt);
 else
  m = 0.0;
 EdgeVal = xx[1];
 for (y=Edge2aHt; y<=Edge1Ht; y++, EdgeVal += m)
  Edge2[y] = EdgeVal;

 if (Edge1[Edge2aHt] > Edge2[Edge2aHt])
  ScanOrder = SCAN_REVERSE;

 if (Edge2[Edge2aHt] != Edge1[Edge2aHt])
  {
  ValCY = CloudVal[0] + dCY * Edge2aHt;
  ValIY = IllumVal[0] + dIY * Edge2aHt;
  dCX = ((float)CloudVal[1] - ValCY) / ((float)Edge2[Edge2aHt] - Edge1[Edge2aHt]);
  dIX = ((float)IllumVal[1] - ValIY) / ((float)Edge2[Edge2aHt] - Edge1[Edge2aHt]);
  } /* if */
 else
  {
  dCX = 0.0;
  dIX = 0.0;
  }

 ValCY = CloudVal[0];
 ValIY = IllumVal[0];

 if (render & 0x010)
  SetAPen(win->RPort, AltPen[1]);
 if (ScanOrder)
  {
  for (y=0, scrnrow=yy[0]; y<=Edge1Ht; y++, scrnrow++, ValCY += dCY, ValIY += dIY)
   {
   if (scrnrow < 0)
    continue;
   if (scrnrow > high)
    break;
   zip = scrnrowzip[scrnrow] + Edge1[y];
   ValCX = ValCY;
   ValIX = ValIY;
   for (x=Edge1[y]; x<=Edge2[y]; x++, zip++, ValCX += dCX, ValIX += dIX)
    {
    if (x < 0)
     continue;
    if (x > wide)
     break;
    if (qqq < *(zbuf + zip))
     {
     if (ValCX < 0.0)
      ValCX = 0.0;
     else if (ValCX > 255.0)
      ValCX = 255.0;
     if (ValIX < 0.0)
      ValIX = 0.0;
     else if (ValIX > 255.0)
      ValIX = 255.0;
     *(bytemap + zip) = (USHORT)ValCX;
     *(bytemap + zip) += (((USHORT)ValIX) << 8);
     *(zbuf + zip) = qqq;
     if (ElevationMap)
      ElevationMap[zip] = Elev;
     } /* if */
    } /* for x=... */
   } /* for y=0... */
  } /* if normal scan - left to right */
 else
  {
  for (y=0, scrnrow=yy[0]; y<=Edge1Ht; y++, scrnrow++, ValCY += dCY, ValIY += dIY)
   {
   if (scrnrow < 0)
    continue;
   if (scrnrow > high)
    break;
   zip = scrnrowzip[scrnrow] + Edge1[y];
   ValCX = ValCY;
   ValIX = ValIY;
   for (x=Edge1[y]; x>=Edge2[y]; x--, zip--, ValCX -= dCX, ValIX -= dIX)
    {
    if (x < 0)
     continue;
    if (x > wide)
     break;
    if (qqq < *(zbuf + zip))
     {
     if (ValCX < 0.0)
      ValCX = 0.0;
     else if (ValCX > 255.0)
      ValCX = 255.0;
     if (ValIX < 0.0)
      ValIX = 0.0;
     else if (ValIX > 255.0)
      ValIX = 255.0;
     *(bytemap + zip) = (USHORT)ValCX;
     *(bytemap + zip) += (((USHORT)ValIX) << 8);
     *(zbuf + zip) = qqq;
     if (ElevationMap)
      ElevationMap[zip] = Elev;
     } /* if */
    } /* for x=... */
   } /* for y=0... */
  } /* else scan reverse */ 

} /* MapCloud() */

/***********************************************************************/

double CloudCover_Set(struct CloudData *CD, double Lat, double Lon)
{

 return(DEM_InterpPt(&CD->Map, Lat, Lon) / 510.0);


/*
long Row, Col;
double CloudCover = 0.0, LonOff, LatOff, LonInvOff, LatInvOff, wt[4], val[4], Area;

 if (Lat >= CD->Map.lolat &&
	Lat <= CD->Map.lolat + (CD->Map.columns - 1) * CD->Map.steplat &&
	Lon <= CD->Map.lolong &&
	Lon >= CD->Map.lolong - CD->Map.rows * CD->Map.steplong)
  {
  LatOff = Lat - CD->Map.lolat;
  LonOff = CD->Map.lolong - Lon;
  Col = LatOff / CD->Map.steplat;
  Row = LonOff / CD->Map.steplong;

  LatOff -= (Col * CD->Map.steplat);
  LonOff -= (Row * CD->Map.steplong);

  LonInvOff = CD->Map.steplong - LonOff;
  LatInvOff = CD->Map.steplat - LatOff;
  
  if (Col >= CD->Map.columns - 1)
   {
   if (Row >= CD->Map.rows)
    {
    CloudCover = CD->Map.map[Row * CD->Map.columns + Col] / 510.0;
    }
   else
    {
    wt[0] = LonInvOff / CD->Map.steplong;
    val[0] = CD->Map.map[Row * CD->Map.columns + Col];
    wt[3] = 1.0 - wt[0];
    val[3] = CD->Map.map[(Row + 1) * CD->Map.columns + Col];
    CloudCover = (wt[0] * val[0] + wt[3] * val[3]) / 510.0;
    }
   }
  else if (Row >= CD->Map.rows)
   {
   wt[0] = LatInvOff / CD->Map.steplat;
   val[0] = CD->Map.map[Row * CD->Map.columns + Col];
   wt[1] = 1.0 - wt[0];
   val[1] = CD->Map.map[Row * CD->Map.columns + Col + 1];
   CloudCover = (wt[0] * val[0] + wt[1] * val[1]) / 510.0;
   }
  else
   {
   Area = CD->Map.steplat * CD->Map.steplong;
   wt[0] = LatInvOff * LonInvOff / Area;
   val[0] = CD->Map.map[Row * CD->Map.columns + Col];
   wt[1] = LatOff * LonInvOff / Area;
   val[1] = CD->Map.map[Row * CD->Map.columns + Col + 1];
   wt[2] = LatOff * LonOff / Area;
   val[2] = CD->Map.map[(Row + 1) * CD->Map.columns + Col + 1];
   wt[3] = LonOff * LatInvOff / Area;
   val[3] = CD->Map.map[(Row + 1) * CD->Map.columns + Col];
   CloudCover = (wt[0] * val[0] + wt[1] * val[1] + wt[2] * val[2] + wt[3] * val[3]) / 510.0;
   }

// max value of cloud map is 255 so dividing by 510 allows cloudcover
// to reach 50%
  } // if in bounds

 return (CloudCover);
*/
} /* CloudCover_Set() */
