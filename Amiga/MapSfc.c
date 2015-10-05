/* MapSfc.c (ne gismapsfc.c 14 Jan 1994 CXH)
** Maps a non-topo Surface.
** Ripped out of gisam.c and Built on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. Huber.
*/

#include "WCS.h"

/* Render:
** 0x1111
** 0x1000 Undefined
** 0x0100 Render to Diagnostic buffer
** 0x0010 Render to screen
** 0x0001 Render to RGB buffer
*/

#define RENDER_SCREEN_DITHER_SIZE 4096

#ifdef OLD_DITHER_HACK
double *SfcDTable;
#endif /* OLD_DITHER_HACK */

void MapSFC(struct Window *win)
{
 short drawcol, range;
 long x, y, scrnrow, zip;
 float color;
 double gradfact, colordif, h, d, m;

#ifdef OLD_DITHER_HACK
 if(!SfcDTable)
   SfcDTable = DitherTable_New(RENDER_SCREEN_DITHER_SIZE);
#endif /* OLD_DITHER_HACK */

 if (settings.reliefshade || (render & 0x01))
  {
  h = sunlong - (facelong + PARC_RNDR_MOTION(9)) * PiOver180 - diplong;
  if (h > Pi) h -= TwoPi;
  else if (h < -Pi) h += TwoPi;
  d = sunlat - facelat * PiOver180 - diplat;
  if (d > Pi) d -= TwoPi;
  else if (d < -Pi) d += TwoPi;
  sunangle = sqrt(h * h + d * d) * .7071;
  if (sunangle > HalfPi) sunangle = HalfPi;
  sunfactor = sin(sunangle);
  sunshade = sunfactor * PARC_RNDR_MOTION(22);
  color = 8.0 + 7.99 * sunfactor + ROUNDING_KLUDGE;
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
  color = 8.0 + 7.99 * gradfact + ROUNDING_KLUDGE;
  } /* if */

 col = color;
 colordif = color - ROUNDING_KLUDGE - col;

 if (render & 0x01)
  {
  fade = (qqq - PARC_RNDR_MOTION(20)) / PARC_RNDR_MOTION(21);
  if (fade > 1.0)
 	 fade = 1.0;
  else if (fade < 0.0)
	 fade = 0.0;

  if (fogrange == 0.0)
	 fog = 0.0;
  else
   {
   fog = (el - PARC_RNDR_MOTION(23)) / fogrange;
   if (fog > 1.0) fog = 1.0;
   else if (fog < 0.0) fog = 0.0;
   } /* else */

  switch (range)
   {
   case 0:
    {
    aliasred = surfcol1[0];
    aliasred += (surfcol0[0] - aliasred) * gradfact;
    aliasgreen = surfcol1[1];
    aliasgreen += (surfcol0[1] - aliasgreen) * gradfact;
    aliasblue = surfcol1[2];
    aliasblue += (surfcol0[2] - aliasblue) * gradfact;
    break;
    } /* low range */
   case 1:
    {
    aliasred = surfcol2[0];
    aliasred += (surfcol1[0] - aliasred) * gradfact;
    aliasgreen = surfcol2[1];
    aliasgreen += (surfcol1[1] - aliasgreen) * gradfact;
    aliasblue = surfcol2[2];
    aliasblue += (surfcol1[2] - aliasblue) * gradfact;
    break;
    } /* middle range */
   case 2:
    {
    aliasred = surfcol3[0];
    aliasred += (surfcol2[0] - aliasred) * gradfact;
    aliasgreen = surfcol3[1];
    aliasgreen += (surfcol2[1] - aliasgreen) * gradfact;
    aliasblue = surfcol3[2];
    aliasblue += (surfcol2[2] - aliasblue) * gradfact;
    break;
    } /* high range */
   } /* switch color range */
  aliasred -= aliasred * sunshade;
  aliasred += (fadecol[0] - aliasred) * fade;
  aliasred *= redsun;
  aliasgreen -= aliasgreen * sunshade;
  aliasgreen += (fadecol[1] - aliasgreen) * fade;
  aliasgreen *= greensun;
  aliasblue -= aliasblue * sunshade;
  aliasblue += (fadecol[2] - aliasblue) * fade;
  aliasblue *= bluesun;
  if (aliasred > 255) red = 255;
  else red = aliasred < 0 ? 0: aliasred;
  if (aliasgreen > 255) green = 255;
  else  green = aliasgreen < 0 ? 0: aliasgreen;
  if (aliasblue > 255) blue = 255;
  else blue = aliasblue < 0 ? 0: aliasblue;
  } /* if render to RGB */

 if (yy[1] < yy[0])
  {
  swmem(&yy[0], &yy[1], 4);
  swmem(&xx[0], &xx[1], 4);
  }
 if (yy[2] < yy[1])
  {
  swmem(&yy[1], &yy[2], 4);
  swmem(&xx[1], &xx[2], 4);
  if (yy[1] < yy[0])
   {
   swmem(&yy[0], &yy[1], 4);
   swmem(&xx[0], &xx[1], 4);
   } /* if */
  } /* if */

 if (yy[2] == yy[0])
	 m = 0.0;
 else
	 m = ((float)xx[2] - xx[0]) / (yy[2] - yy[0]);
 for (y=0; y<=yy[2]-yy[0]; y++)
	 edge1[y] = m * y + xx[0];
 if (yy[1] == yy[0])
	 m = 0.0;
 else
	 m = ((float)xx[1] - xx[0]) / (yy[1] - yy[0]);
 for (y=0; y<=yy[1]-yy[0]; y++)
	 edge2[y] = m * y + xx[0];
 if (yy[2] == yy[1])
	 m = 0.0;
 else
	 m = ((float)xx[2] - xx[1]) / (yy[2] - yy[1]);
 for (y=yy[1]-yy[0]; y<=yy[2]-yy[0]; y++)
  {
  edge2[y] = m * (y - yy[1] + yy[0]) + xx[1];
  } /* for y=... */

 scrnrow = yy[0] - 1;

 for (y=0; y<=yy[2]-yy[0]; y++)
  {
  if (edge1[y] > edge2[y])
	 swmem(&edge1[y], &edge2[y],4);
  scrnrow++;
  if (scrnrow >= oshigh) break;
  for (x=edge1[y]; x<=edge2[y]; x++)
   {
   zip = scrnrowzip[scrnrow] + x;
   if (qqq < *(zbuf + zip))
    {
#ifdef AMIGA_GUI
    if (render & 0x10)
     {
     drawcol = col;
     if (drawcol < 15)
      {
#ifdef OLD_DITHER_HACK
      if(SfcDTable)
        {
        if (colordif > SfcDTable[(DMod++ % RENDER_SCREEN_DITHER_SIZE)])  
          drawcol ++;
        } /* if */
      else
        {
        if (colordif > drand48())  
          drawcol ++;
        } /* else */
#else
      if (colordif > DTable[(DMod++ % RENDER_SCREEN_DITHER_SIZE)])
          drawcol ++;
#endif /* OLD_DITHER_HACK */
      } /* if drawcol < 15 */

     SetAPen(win->RPort, AltPen[drawcol]);
     WritePixel(win->RPort, x + drawoffsetX, scrnrow + drawoffsetY);
     } /* if render to screen */
#endif /* AMIGA_GUI */
    if (render & 0x01)
     {
     *(bitmap[0] + zip) = red;
     *(bitmap[1] + zip) = green;
     *(bitmap[2] + zip) = blue;
     } /* if render */
    *(zbuf + zip) = qqq;
    } /* if qqq */
   else if ((render & 0x01) && (qqq < *(zbuf + zip) + .005))
    {
    *(bitmap[0] + zip) = ((UBYTE)*(bitmap[0] + zip) + red) / 2;
    *(bitmap[1] + zip) = ((UBYTE)*(bitmap[1] + zip) + green) / 2;
    *(bitmap[2] + zip) = ((UBYTE)*(bitmap[2] + zip) + blue) / 2;
    *(zbuf + zip) = qqq;
    } /* else */
   } /* for x=edge1... */
  } /* for y=0... */

/* It'll do it's own thing. */

/* if(SfcDTable)
	DitherTable_Del(SfcDTable, RENDER_SCREEN_DITHER_SIZE); */

} /* mapSFC() */

