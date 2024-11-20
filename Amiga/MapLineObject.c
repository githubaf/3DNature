/* MapLineObject.c (ne gismaplineobject.c 14 Jan 1994 CXH)
** More of what's in gisam.c
** Ripped out of gisam.c and Built on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. Huber
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"

short maplineobject(struct elmapheaderV101 *map, struct Window *win,
	short zbufplot, double *Lat, double *Lon)
{
 short j, k, z, error, framemod;
 long y, zip;
 float ppp;
 double ptelev, h, d;

 framemod = frame % (DBase[OBN].Points + 1);

 for (zip=1; zip<=DBase[OBN].Points; zip++)
  {
  ptelev = *(map->map + zip);  
  ptelev += (PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12);
  *(map->lmap + zip) = (long)ptelev;
  DP.alt = ptelev * PARC_RNDR_MOTION(14) * map->elscale;
  DP.lat = Lat[zip];
  DP.lon = Lon[zip];
  getscrncoords((map->scrnptrx + zip), (map->scrnptry + zip),
		(map->scrnptrq + zip), NULL, NULL);
  } /* for zip=1... */

 if (!settings.linetoscreen)
  {
SaveRepeat:
  error=writelinefile(map, 0);
  if (error)
   {
   fclose(fvector);
   Log(ERR_WRITE_FAIL, (CONST_STRPTR)linefile);
   User_Message(GetString( MSG_AGUI_RENDERMODULE ),                               // "Render Module"
                GetString( MSG_MAPLINO_ERRORSAVINGLINEVERTICESTOFILEELECTNEWPATH ),  // "Error saving line vertices to file!\nSelect new path."
                GetString( MSG_GLOBAL_OK ),                                         // "OK"
                (CONST_STRPTR)"o");
NewFileRequest:
   if (getfilename(1,(char*)GetString( MSG_MAPLINO_NEWLINESAVEPATH ), linepath, linefile))  // "New Line Save Path"
    {
    char filename[262]; // was 255, but sprintf(filename,"%s%d", str, frame); could write up to 262 bytes! AF, 19. Nov 24, Werror=format-overflow

    strmfp(str, linepath, linefile);
    sprintf(filename,"%s%d", str, frame);
    if ((fvector = fopen(filename, "w")) == NULL)
     {
     Log(ERR_OPEN_FAIL, (CONST_STRPTR)linefile);
     if (User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                          // "Render Module"
                          GetString( MSG_MAPLINO_ERROROPENINGLINESAVEFILEELECTNEWPATH ),  // "Error opening line save file!\nSelect new path?"
                          GetString( MSG_GLOBAL_OKCANCEL ),                              // "OK|Cancel"
                          (CONST_STRPTR)"oc", 1))
      goto NewFileRequest;
     } /* if open fail */
    else goto SaveRepeat;
    }
   settings.linetoscreen = -1;
   return (1);
   }
  return (0);
  } /* if ! settings.linetoscreen */

 for (z=1,j=k=0; z<=DBase[OBN].Points; z++,j=k=0)
  {
  if ((! strcmp(DBase[OBN].Special, "VSG") ||
	 ! strcmp(DBase[OBN].Special, "VIS"))
	&& (framemod < (z % DBase[OBN].Points)
	 || framemod >= z + settings.vecsegs))
   continue;
  if ( *(map->scrnptrq + z) < 0.0) continue;
  facelat = Lat[z];
  facelong = Lon[z];
  h = sunlong - ((facelong + PARC_RNDR_MOTION(9)) * PiOver180);
  if (h > Pi) h -= TwoPi;
  else if (h < -Pi) h += TwoPi;
  d = sunlat - (facelat * PiOver180);
  if (d > Pi) d -= TwoPi;
  else if (d < -Pi) d += TwoPi;
  sunangle = sqrt(h * h + d * d) * .7071;
  if (sunangle > HalfPi) sunangle = HalfPi;
  sunfactor = 1.0 - cos(sunangle);
  sunshade = sunfactor * PARC_RNDR_MOTION(22);

  if (DBase[OBN].Special[1] == 'I' && sunshade > .7)
   {
   altred = ILLUMVECRED;
   altgreen = ILLUMVECGRN;
   altblue = ILLUMVECBLU;
   transpar = 0.0; /*.3 / DBase[OBN].LineWidth;*/
   } /* if illuminated vector object */
  else
   {
   altred = DBase[OBN].Red;
   altgreen = DBase[OBN].Grn;
   altblue = DBase[OBN].Blu;
   if (settings.linefade)
    {
    altred -=sunshade * altred;
    altgreen -=sunshade * altgreen;
    altblue -=sunshade * altblue;
    } /* if */
   transpar = 0.0;
   } /* else not illuminated vector object */

  if (DBase[OBN].Pattern[0] == 'P' || DBase[OBN].Pattern[0] == 'C'
	 || DBase[OBN].Pattern[0] == 'R' || DBase[OBN].Pattern[0] == 'X')
   {
   xx[0] = *(map->scrnptrx + z) - (DBase[OBN].LineWidth / 2);
   yy[0] = *(map->scrnptry + z) - (DBase[OBN].LineWidth / 2);
   qqq = *(map->scrnptrq + z) - settings.lineoffset;
   if (qqq < 0.0) qqq = 0.0;
   if (qqq < qmin) continue;
   if (xx[0] < -10) j = 1;
   else if (xx[0] > wide) j = 1;
   if (yy[0]< -10) j = 1;
   else if (yy[0] > high) j = 1;
   if (! j)
    {
    FadeLine(map->lmap[z]);
    EndDrawPoint(win, zbufplot, DBase[OBN].Color);
    } /* if */
   } /* if point, circle, rectangle or cross */

  else
   {
   if (z == DBase[OBN].Points) break;
   if (*(map->scrnptrq + z + 1) < 0.0) continue;
   map->facept[0] = z;
   map->facept[1] = z+1;
   for (y=0; y<2; y++)
    {
    xx[y] = *(map->scrnptrx + map->facept[y]) - DBase[OBN].LineWidth / 2;
    yy[y] = *(map->scrnptry + map->facept[y]) - DBase[OBN].LineWidth / 2;
    if (!y) qqq = *(map->scrnptrq + map->facept[y]);
    else
     {
     ppp = *(map->scrnptrq + map->facept[y]);
     if (ppp < qqq) qqq = ppp;
     qqq -= settings.lineoffset;
     if (qqq < 0.0) qqq = 0.0;
     } /* else */
    if (xx[y] < 0) j += 1;
    else if (xx[y] > wide) j += 1;
    if (yy[y] < 0) k += 1;
    else if (yy[y] > high) k += 1;
    } /* for y=0... */
   if (qqq < qmin)
    continue;
   if (j < 2 && k < 2)
    {
    FadeLine(map->lmap[z]);
    EndDrawLine(win, zbufplot, DBase[OBN].Color);
    } /* if */
   } /* else */

  } /* for z=1... */

 return 0;

} /* maplineobject() */

