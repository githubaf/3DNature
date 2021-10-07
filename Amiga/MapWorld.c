/* MapWorld.c (ne gismapworld.c 14 Jan 1994 CXH)
** Has the mapworld function, eh?
** Ripped/Slashed/Hacked/Built from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson
** Original code by Gary R. Huber the magnificent. (We're not worthy! ;)
*/

#include "WCS.h"

void mapworld(struct Window *win)
{
 FILE *fname;
 char filename[256];
 short intval, error=0, hemisphere = 0, outj, outk, OpenOK;
 long i, j, k, l, lastel = 0;
 float val;
 double lonscale, latscale, latitude;
 struct DirList *DLItem;

 treehtfact = 0.0;
 cloudcover = 1.0;
 aspect = 0.0;
 relel = 0.0;
 elmap[0].elscale = .0006210023;

 if (!settings.worldbase)
  {
  elmap[0].size = 720 * 2;
  elmap[0].map = get_Memory(elmap[0].size, MEMF_CLEAR);
  if (! elmap[0].map) return;
  latscale = 1.39444;				/* .02*69.7222 */

  OpenOK = 0;

  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, "earth_topo_720x360");
   if ((fname = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    }
   else
    {
    OpenOK = 1;
    break;
    } /* file opened */
   } /* while */

  if (! OpenOK)
   {
   Log(WNG_OPEN_FAIL, (CONST_STRPTR)"earth_topo_720x360");
   return;
   }

  for (j=0; j<360; j++)
   {
   lonscale = latscale * cos((90.0 - 180.0 *(j + .5) / 360.0) * PiOver180);
   for (i=0; i<720; i++)
    {
    if (fread((char *)&val, 4, 1, fname) != 1)
     {
     Log(ERR_READ_FAIL, (CONST_STRPTR)"world topo data");
     error = 1;
     break;
     } /* if */
    faceel = 10 + 1.488 * val;
    diplat = atan((faceel - *(elmap[0].map + i)) * elmap[0].elscale
		/ latscale);
    if (i==0) diplong = atan((faceel - *(elmap[0].map + 719))
		* elmap[0].elscale / lonscale);
    else diplong = atan((faceel - *(elmap[0].map + i - 1))
		* elmap[0].elscale / lonscale);
    slope = .7071 * sqrt(diplat * diplat + diplong * diplong);
    ept[0][1] = *(elmap[0].map + i);
    if (i)
     {
     ept[0][0] = lastel;
     ept[1][0] = *(elmap[0].map + i - 1);
     } /* if */
    else ept[1][0] =ept[0][0] = *(elmap[0].map + 719);
    ept[1][1] = faceel;
    lastel = *(elmap[0].map + i);
    *(elmap[0].map + i) = faceel;
    DP.alt = faceel + (MoShift[13].Value[2] - faceel) * MoShift[12].Value[2];
    DP.alt *= MoShift[14].Value[2] * elmap[0].elscale;
    for (k=outj=outk=0; k<2; k++)
     {
     for (l=0; l<2; l++)
      {
      DP.lat = 90.0 - 180.0 * (j + k) / 360.0;
      DP.lon = 360.0 - 360.0 * (i + l) / 720.0;
      if (DP.lon>180.0) DP.lon -= 360.0;
      facelat = DP.lat;
      facelong = DP.lon;
      getscrnpt();
      if (DP.z <= 0.0) qpt[k][l] = -1.0;
      if (DP.q > qmax || DP.q < 0.0)
       {
       outj = 4;
       break;
       } /* if */
      xpt[k][l] = MoShift[6].Value[2] + (DP.x * PROJPLANE / DP.z) / horscale;
      ypt[k][l] = MoShift[7].Value[2] - (DP.y * PROJPLANE / DP.z) / vertscale;
      qpt[k][l] = DP.q;		/* sqrt(DP.x*DP.x+DP.y*DP.y+DP.z*DP.z); */
      if (xpt[k][l] < 0) {xpt[k][l] = 0; outj += 1;}
      else if (xpt[k][l] > wide) {xpt[k][l] = wide; outj += 1;}
      if (ypt[k][l] < 0) {ypt[k][l] = 0; outk += 1;}
      else if (ypt[k][l] > high) {ypt[k][l] = high; outk += 1;}
      } /* for l=0... */
     if (outj>3 || outk>3) break;
     } /* for k=0... */
    if (outj>3 || outk>3) continue;
    prepmap(win, i, j);
    } /* for i<720 */
   if (error) break;
   } /* for j<360 */
  fclose(fname);
  free_Memory(elmap[0].map, elmap[0].size);
  } /* if ! settings.worldbase */

 else
  {
  OpenOK = 0;

  elmap[0].size = 4320 * 2;
  elmap[0].map = get_Memory(elmap[0].size, MEMF_CLEAR);
  if (! elmap[0].map) return;
  latscale = .4;		/* smaller value to exagerate topo shading */
  if (settings.highglobe <= 0.0) goto MapSouth;

  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, "NORTH.BTH");
   if ((fname = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    } /* if open fail */
   else
    {
    OpenOK = 1;
    break;
    } /* file opened */
   } /* while */

  if (! OpenOK)
   {
   Log(WNG_OPEN_FAIL, (CONST_STRPTR)"NORTH.BTH");
   return;
   } /* if */

ReMap:
  for (j=0; j<1080; j++)
   {
   if (! hemisphere) latitude = (90.0 - 90.0 * j / 1080.0);
   else latitude = -90.0 * j / 1080.0;
   if (latitude > settings.highglobe)
    {
    if (fseek(fname, 8640, 1)) break;
    continue;
    } /* if */
   else if (latitude < settings.lowglobe) break;
   lonscale = latscale * cos(latitude * PiOver180);
   for (i=0; i<4320; i++)
    {
    if (fread((char *)&intval, 2, 1, fname) != 1)
     {
     printf("Error reading world topo data. j=%d, i=%d\n",j,i);
     error=1;
     break;
     } /* if */
    faceel = intval;
    diplat = atan((faceel - *(elmap[0].map + i)) * elmap[0].elscale
		/ latscale);
    if (i == 0) diplong = atan((faceel - *(elmap[0].map + 4319))
		* elmap[0].elscale / lonscale);
    else diplong = atan((faceel - *(elmap[0].map + i - 1))
		* elmap[0].elscale / lonscale);
    slope = .7071 * sqrt(diplat * diplat + diplong * diplong);
    ept[0][1] = *(elmap[0].map + i);
    if (i)
     {
     ept[0][0] = lastel;
     ept[1][0] = *(elmap[0].map + i - 1);
     } /* if */
    else ept[1][0] = ept[0][0] = *(elmap[0].map + 4319);
    ept[1][1] = faceel;
    lastel = *(elmap[0].map + i);
    *(elmap[0].map + i) = faceel;
    DP.alt = faceel + (MoShift[13].Value[2] - faceel) * MoShift[12].Value[2];
    DP.alt *= MoShift[14].Value[2] * elmap[0].elscale;
    for (k=outj=outk=0; k<2; k++)
     {
     facelat = latitude - 90.0 * (k - 1) / 1080.0;
     for (l=0; l<2; l++)
      {
      DP.lat = facelat;
      DP.lon = 360.0 - 360.0 * (i + l - 1) / 4320.0;
      if (DP.lon > 180.0) DP.lon -= 360.0;
      facelong = DP.lon;
      getscrnpt();
      if (DP.z <= 0.0) qpt[k][l] = -1.0;
      if (DP.q > qmax || DP.q < 0.0)
       {
       outj = 4;
       break;
       } /* if */
      xpt[k][l] = MoShift[6].Value[2] + (DP.x * PROJPLANE / DP.z) / horscale;
      ypt[k][l] = MoShift[7].Value[2] - (DP.y * PROJPLANE / DP.z) / vertscale;
      qpt[k][l] = DP.q;		/* sqrt(DP.x*DP.x+DP.y*DP.y+DP.z*DP.z); */
      if (xpt[k][l] < 0) {xpt[k][l] = 0; outj += 1;}
      else if (xpt[k][l] > wide) {xpt[k][l] = wide; outj += 1;}
      if (ypt[k][l] < 0) {ypt[k][l] = 0; outk += 1;}
      else if (ypt[k][l] > high) {ypt[k][l] = high; outk += 1;}
      } /* for l=0... */
     if (outj > 3 || outk > 3) break;
     } /* for k=outj... */
    if (outj > 3 || outk > 3) continue;
    prepmap(win, i, j);
    } /* for i<4320 */
   if (error) break;
   } /* for j<1080 */
  fclose(fname);

MapSouth:
  if (!hemisphere)
   {
   if (settings.lowglobe <= 0.0)
    {
    OpenOK = 0;

    DLItem = DL;
    while (DLItem)
     {
     strmfp(filename, DLItem->Name, "SOUTH.BTH");
     if ((fname = fopen(filename, "rb")) == NULL)
      {
      DLItem = DLItem->Next;
      }
     else
      {
      OpenOK = 1;
      break;
      } /* file opened */
     } /* while */

    if (! OpenOK)
     {
     Log(WNG_OPEN_FAIL, (CONST_STRPTR)"SOUTH.BTH");
     return;
     }
    hemisphere = 1;
    goto ReMap;
    }
   }
  free_Memory(elmap[0].map, elmap[0].size);
  } /* settings.worldbase==1 */

 settings.worldmap = 0;

} /* worldmap() */

