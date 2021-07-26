/* InteractiveUtils.c (ne gisinteractiveutils.c 14 Jan 1994 CXH)
** Parameter modification utility routines for GISinteractiveview.c
** Written by Gary R. Huber and Chris "Xenon" Hanson, 8/93.
*/

#include "WCS.h"

STATIC_FCN void DisableBoundsButtons(void); // used locally only -> static, AF 26.7.2021


short SetIncrements(short selectitem)
{
 short error = 0;

 incr[1] = 0.0;
 incr[2] = 0.0;
 switch (selectitem)
  {
  case 0:
  case 1:
  case 2:
   if (! IA_Movement)
    {
    incr[0] = -.00017453292;	/* Camera Azimuth Radians (longitude) */
    incr[1] = -.001;		/* Camera Distance (latitude) */
    } /* if radial motion */
   else
    {
    incr[0] = -.00001;		/* Camera Longitude */
    incr[1] = -.00001;		/* Camera Latitude */
    } /* else rectangular motion */
   incr[2] = .001;		/* Camera Altitude */
   item[0] = 2;
   item[1] = 1;
   item[2] = 0;
   break;
  case 3:
  case 4:
  case 5:
   if (! IA_Movement)
    {
    incr[0] = -.00017453292;	/* Focus Radius Radians (longitude) */
    incr[1] = -.001;		/* Focus Distance (latitude) */
    } /* if radial motion */
   else
    {
    incr[0] = -.00001;		/* Focus Longitude */
    incr[1] = -.00001;		/* Focus Latitude */
    } /* else rectangular motion */
   incr[2] = .001;		/* Focus Altitude */
   item[0] = 5;
   item[1] = 4;
   item[2] = 3;
   break;
  case 6:
  case 7:
   incr[0] = .1;		/* Center X */
   incr[1] = .1;		/* Center Y */
   item[0] = 6;
   item[1] = item[2] = 7;
   break;
  case 8:
   incr[0] = .005;		/* Bank */
   item[0] = item[1] = item[2] = 8;
   break;
  case 9:
   incr[0] = .001;		/* Earth Rotation */
   item[0] = item[1] = item[2] = 9;
   break;
  case 10:
   incr[0] = .001;		/* Scale */
   item[0] = item[1] = item[2] = 10;
   break;
  case 11:
   incr[0] = .005;		/* View Arc */
   item[0] = item[1] = item[2] = 11;
   break;
  case 12:
  case 13:
   incr[0] = .001;		/* Flattening	 */
   incr[2] = .5;		/* Datum */
   item[0] = 12;
   item[1] = item[2] = 13;
   break;
  case 14:
   incr[0] = .005;		/* Vertical Exag */
   item[0] = item[1] = item[2] = 14;
   break;
  case 15:
  case 16:
   incr[0] = -.01;		/* Sun Longitude */
   incr[1] = -.01;		/* Sun Latitude */
   item[0] = 16;
   item[1] = item[2] = 15;
   break;
  case 17:
  case 18:
  case 19:
   incr[0] = .01;		/* Horizon Point */
   incr[1] = .01;		/* Horizon Line */
   incr[2] = .01;		/* Horizon Stretch */
   item[0] = 18;
   item[1] = 17;
   item[2] = 19;
   break;
  case 20:
  case 21:
   incr[0] = .01;		/* Haze Start */
   incr[1] = .01;		/* Haze Range */
   item[0] = 20;
   item[1] = item[2] = 21;
   break;
  case 22:
   incr[0] = .0001;		/* Shade Factor */
   item[0] = item[1] = item[2] = 22;
   break;
  case 23:
  case 24:
   incr[0] = 1.0;		/* Fog Full */
   incr[1] = 1.0;		/* Fog None */
   item[0] = 23;
   item[1] = item[2] = 24;
   break;
  case 25:
   incr[0] = .01;		/* Q Minimum */
   item[0] = item[1] = item[2] = 25;
   break;
  case 26:
  case 27:
  case 28:
   incr[0] = -.01;		/* Sun Lon */
   incr[1] = -.01;		/* Sun Lat */
   incr[2] = -1.0;		/* Sun Size */
   item[0] = 27;
   item[1] = 26;
   item[2] = 28;
   break;
  case 29:
  case 30:
  case 31:
   incr[0] = -.01;		/* Moon Lon */
   incr[1] = -.01;		/* Moon Lat */
   incr[2] = -1.0;		/* Moon Size */
   item[0] = 30;
   item[1] = 29;
   item[2] = 31;
   break;
  case 32:
   incr[0] = 1.0;
   incr[1] = incr[2] = 0.0;
   item[0] = item[1] = item[2] = 32;
   break;

  default:
   error = 1;
   item[0] = item[1] = item[2] = selectitem;
   incr[0] = 0.0;
   break;
  } /* switch selectitem */

 incr[0] *= IA_Sensitivity;
 incr[1] *= IA_Sensitivity;
 incr[2] *= IA_Sensitivity;

 return error;

} /* SetIncrements() */

/************************************************************************/

void autocenter(void)
{
 short i;
 double maxlat = -90.0, minlat = 90.0,
	maxlon = -10000.0, minlon = 10000.0,
	maxalt = -EARTHRAD, minalt = 2 * EARTHRAD;

 for (i=0; i<NoOfElMaps; i++)
  {
  if (BBox[i].lat[2] > maxlat) maxlat = BBox[i].lat[2];
  if (BBox[i].lat[0] < minlat) minlat = BBox[i].lat[0];
  if (BBox[i].lon[0] > maxlon) maxlon = BBox[i].lon[0];
  if (BBox[i].lon[1] < minlon) minlon = BBox[i].lon[1];
  if (BBox[i].elev[4] * elmap[i].elscale > maxalt)
	maxalt = BBox[i].elev[4] * elmap[i].elscale;
  if (BBox[i].elev[0] * elmap[i].elscale < minalt)
	minalt = BBox[i].elev[0] * elmap[i].elscale;
  } /* for i=0... */

 PAR_FIRST_MOTION(3) = (maxalt + minalt) * .5;
 PAR_FIRST_MOTION(3) += ((PAR_FIRST_MOTION(13) * ELSCALE_METERS - PAR_FIRST_MOTION(3))
			* PAR_FIRST_MOTION(12));
 PAR_FIRST_MOTION(3) *= PAR_FIRST_MOTION(14);
 PAR_FIRST_MOTION(4) = (maxlat + minlat) * .5;
 PAR_FIRST_MOTION(5) = (maxlon + minlon) * .5;

 if (item[0] > 2 && item[0] < 6) setcompass(1);
 else setcompass(0);

 if (MP && MP->ptsdrawn)
  {
  struct clipbounds cb;

  setclipbounds(MapWind0, &cb);
  ShowView_Map(&cb);
  } /* if map interactive */

 IA->recompute = 1;

 sprintf(str, "Auto-centered: alt = %f  lat = %f  lon = %f\n",
		PAR_FIRST_MOTION(3),
		PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
 Log(MSG_NULL, str);

} /* autocenter() */

/************************************************************************/

void modifyinteractive(short shiftx, short shifty, long button)
{
 if (button == SELECTDOWN)
  {
  PAR_FIRST_MOTION(item[0]) += shiftx * incr[0];
  PAR_FIRST_MOTION(item[1]) += shifty * incr[1];
  if (MP && MP->ptsdrawn)
   {
   switch (item[0])
    {
    case 2:		/* camera */
     {
     Update_InterMap(1);
     Update_InterMap(3);
     Update_InterMap(4);
     break;
     }
    case 5:		/* focus */
     {
     Update_InterMap(2);
     break;
     }
    case 20:		/* haze */
     {
     Update_InterMap(3);
     Update_InterMap(4);
     break;
     }
    case 10:		/* scale */
    case 11:		/* view arc */
     {
     Update_InterMap(5);
     break;
     }
    case 16:		/* sun */
     {
     Update_InterMap(7);
     break;
     }
    } /* switch */
   } /* if map interactive */
  } /* if */
 else
  PAR_FIRST_MOTION(item[2]) += shifty * incr[2];

 if (EMIA_Win) IA->recompute = 1;

} /* modifyinteractive() */

/************************************************************************/

void modifycampt(short shiftx, short shifty, short button)
{
 if (button == MENUDOWN)
  {
  PAR_FIRST_MOTION(0) -= shifty * incr[2];
  if (EMIA_Win) IA->recompute = 1;
  } /* if menubutton */
 else
  {
  oldazimuth = azimuth;
  azimuth += (shiftx * incr[0]);
  focdist -= (shifty * incr[1]);
  if (focdist < .00001)
   focdist = .00001;
  reversecamcompute();
  compass(oldazimuth, azimuth);
  if (MP && MP->ptsdrawn)
   {
   Update_InterMap(1);
   Update_InterMap(3);
   Update_InterMap(4);
   } /* if map interactive */
  } /* else */

} /* modifycampt() */

/************************************************************************/

void modifyfocpt(short shiftx, short shifty, short button)
{
 if (button == MENUDOWN)
  {
  PAR_FIRST_MOTION(3) -= shifty * incr[2];
  if (EMIA_Win) IA->recompute = 1;
  } /* if menubutton */
 else
  {
  oldazimuth = azimuth;
  azimuth += (shiftx * incr[0]);
  focdist += (shifty * incr[1]);
  if (focdist < 0.00001)
   focdist = .00001;
  reversefoccompute();	/* recomputes lat & lon */
  compass(oldazimuth, azimuth);
  if (MP && MP->ptsdrawn)
   {
   Update_InterMap(2);
   } /* if map interactive */
  } /* else */

} /* modifyfocpt() */

/************************************************************************/

void Update_InterMap(short modval)
{
 short startX, startY;
 struct clipbounds cb;

 setclipbounds(MapWind0, &cb);
 switch (modval)
  {
  case 1:
   {
   startX = Lon_X_Convert(PAR_FIRST_MOTION(2));
   startY = Lat_Y_Convert(PAR_FIRST_MOTION(1));
   break;
   } /* if */
  case 2:
   { 
   startX = Lon_X_Convert(PAR_FIRST_MOTION(5));
   startY = Lat_Y_Convert(PAR_FIRST_MOTION(4));
   break;
   } /* else */
  case 3:
   {
   startY = 0;
   startX = latscalefactor * PAR_FIRST_MOTION(20) * VERTPIX / mapscale;
   break;
   }
  case 4:
   {
   startY = 0;
   startX = latscalefactor * (PAR_FIRST_MOTION(20) + PAR_FIRST_MOTION(21))
		* VERTPIX / mapscale;
   break;
   }
  case 5:
  case 6:
   {
   startX = startY = 0;
   break;
   }
  case 7:
   {
   double SunLon;

   SunLon = PAR_FIRST_MOTION(16);
   while (SunLon > 360.0)
    SunLon -= 360.0;
   while (SunLon < 0.0)
    SunLon += 360.0;
   startX = Lon_X_Convert(SunLon);
   startY = Lat_Y_Convert(PAR_FIRST_MOTION(15));
   break;
   }
  } /* switch */
 SetDrMd(MapWind0->RPort, COMPLEMENT);
 SetWrMsk(MapWind0->RPort, 0x07);
 ShowCenters_Map(&cb, startX, startY, modval);
 SetDrMd(MapWind0->RPort, JAM1);
 SetWrMsk(MapWind0->RPort, 0x0f);

} /* Update_InterMap() */

/************************************************************************/

void azimuthcompute(short focusaz, double flat, double flon,
          double tlat, double tlon)
{
 double lat, lon, lonscale;

 lat = (tlat - flat) * LATSCALE;
 lonscale = LATSCALE * cos(tlat * PiOver180);
 lon = (tlon - flon) * lonscale;
 focdist = sqrt(lat * lat + lon * lon);
 azimuth = findangle2(lat, lon) - HalfPi;
 if (azimuth < 0.0) azimuth += TwoPi;
 if (focusaz) azimuth += Pi;
 if (azimuth > TwoPi) azimuth -= TwoPi;

} /* azimuthcompute() */

/************************************************************************/

void reversecamcompute(void)
{
 double x, y, lonscale;

 x = -focdist * sin(azimuth + Pi);
 y = focdist * cos(azimuth + Pi);
 lonscale = LATSCALE * cos(PAR_FIRST_MOTION(4) * PiOver180);
 PAR_FIRST_MOTION(1) = PAR_FIRST_MOTION(4) + y / LATSCALE;
 PAR_FIRST_MOTION(2) = PAR_FIRST_MOTION(5) + x / lonscale;
 if (EMIA_Win) IA->recompute = 1;
 if (MP && MP->ptsdrawn)
  {
  Update_InterMap(1);
  Update_InterMap(3);
  Update_InterMap(4);
  } /* if map interactive */

} /* reversecamcompute() */

/************************************************************************/

void reversefoccompute(void)
{
 double x, y, lonscale;

 x = -focdist * sin(azimuth);
 y = focdist * cos(azimuth);
 lonscale = LATSCALE * cos(PAR_FIRST_MOTION(1) * PiOver180);
 PAR_FIRST_MOTION(4) = PAR_FIRST_MOTION(1) + y / LATSCALE;
 PAR_FIRST_MOTION(5) = PAR_FIRST_MOTION(2) + x / lonscale;
 if (EMIA_Win) IA->recompute = 1;
 if (MP && MP->ptsdrawn)
  {
  Update_InterMap(2);
  } /* if map interactive */

} /* reversefoccompute() */

/************************************************************************/

void setcompass(short focusaz)
{
 oldazimuth = azimuth;
 if (! focusaz)
  azimuthcompute(0, PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2),
     PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
 else
  azimuthcompute(1, PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5),
     PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2));
 compass(oldazimuth, azimuth);

} /* setcompass */

/************************************************************************/

void findfocprof(void)
{
 short i;
 double ProxLat, ProxLon, ProxFact = LARGENUM;

/* find the map containing the focus point */
 for (i=0; i<NoOfElMaps; i++) {
  ProxLat = abs(PAR_FIRST_MOTION(4)
		- (BBox[i].lat[0] + BBox[i].lat[3]) * .5);
  ProxLon = abs(PAR_FIRST_MOTION(5)
		- (BBox[i].lon[0] + BBox[i].lon[1]) * .5);
  if (ProxLat + ProxLon < ProxFact) {
   ProxFact = ProxLat + ProxLon;
   FocProf->map = &elmap[i];
  } /* if found new proximal map */
 } /* for i=0... */

/* find the nearest row and column */
 FocProf->row = ((FocProf->map->lolong - PAR_FIRST_MOTION(5))
	/ FocProf->map->steplong) + .5 + ROUNDING_KLUDGE;
 FocProf->col = ((PAR_FIRST_MOTION(4) - FocProf->map->lolat)
	/ FocProf->map->steplat) + .5 + ROUNDING_KLUDGE;
 if (FocProf->row < 0) FocProf->row = 0;
 else if (FocProf->row > FocProf->map->rows) FocProf->row = FocProf->map->rows;
 if (FocProf->col < 0) FocProf->col = 0;
 else if (FocProf->col >= FocProf->map->columns)
	FocProf->col = FocProf->map->columns - 1;

} /* findfocprof() */

/************************************************************************/

void Set_CompassBds(void)
{
 if(CompassBounds)
  {
  if(LandBounds || ProfileBounds || BoxBounds || GridBounds)
   {
   CompassBounds = 0;
   } /* if */
  } /* CompassBounds */
 else
  {
  CompassBounds = 1;
  } /* else */
 DisableBoundsButtons();
 DrawInterFresh(1);

} /* Set_CompassBds() */

/************************************************************************/

void Set_LandBds(void)
{
 if(LandBounds)
  {
  if(BoxBounds || ProfileBounds || CompassBounds || GridBounds)
   {
   LandBounds = 0;
   } /* if */
  } /* if */
 else
  {
  LandBounds = 1;
  } /* else */
 DisableBoundsButtons();
 DrawInterFresh(1);

} /* Set_LandBds() */

/************************************************************************/

void Set_ProfileBds(void)
{
 if(ProfileBounds)
  {
  if(BoxBounds || LandBounds || CompassBounds || GridBounds)
   {
   ProfileBounds = 0;
   } /* if */
  } /* if */
 else
  {
  ProfileBounds = 1;
  findfocprof();
  } /* else */
 DisableBoundsButtons();
 DrawInterFresh(1);

} /* Set_ProfileBds() */

/************************************************************************/

void Set_BoxBds(void)
{
 if(BoxBounds)
  {
  if(LandBounds || ProfileBounds || CompassBounds || GridBounds)
   {
   BoxBounds = 0;
   } /* if */
  } /* BoxBounds */
 else
  {
  BoxBounds = 1;
  } /* else */
 DisableBoundsButtons();
 DrawInterFresh(1);

} /* Set_BoxBds() */

/************************************************************************/

void Set_GridBds(void)
{
 if(GridBounds)
  {
  if(BoxBounds || LandBounds || CompassBounds || ProfileBounds)
   {
   GridBounds = 0;
   } /* if */
  } /* if */
 else
  {
  GridBounds = 1;
  } /* else */
 DisableBoundsButtons();
 DrawInterFresh(1);

} /* Set_GridBds() */

/************************************************************************/

STATIC_FCN void DisableBoundsButtons(void) // used locally only -> static, AF 26.7.2021
{

 if (CompassBounds)
  {
  if (! LandBounds && ! ProfileBounds && ! BoxBounds && ! GridBounds)
   set(EMIA_Win->BT_CompassBounds, MUIA_Disabled, TRUE);
  else
   set(EMIA_Win->BT_CompassBounds, MUIA_Disabled, FALSE);
  }
 if (LandBounds)
  {
  if (! CompassBounds && ! ProfileBounds && ! BoxBounds && ! GridBounds)
   set(EMIA_Win->BT_LandBounds, MUIA_Disabled, TRUE);
  else
   set(EMIA_Win->BT_LandBounds, MUIA_Disabled, FALSE);
  }
 if (ProfileBounds)
  {
  if (! CompassBounds && ! LandBounds && ! BoxBounds && ! GridBounds)
   set(EMIA_Win->BT_ProfileBounds, MUIA_Disabled, TRUE);
  else
   set(EMIA_Win->BT_ProfileBounds, MUIA_Disabled, FALSE);
  }
 if (BoxBounds)
  {
  if (! CompassBounds && ! LandBounds && ! ProfileBounds && ! GridBounds)
   set(EMIA_Win->BT_BoxBounds, MUIA_Disabled, TRUE);
  else
   set(EMIA_Win->BT_BoxBounds, MUIA_Disabled, FALSE);
  }
 if (GridBounds)
  {
  if (! CompassBounds && ! LandBounds && ! ProfileBounds && ! BoxBounds)
   set(EMIA_Win->BT_GridBounds, MUIA_Disabled, TRUE);
  else
   set(EMIA_Win->BT_GridBounds, MUIA_Disabled, FALSE);
  }
 set(EMIA_Win->BT_GBDens[0], MUIA_Disabled, (! GridBounds));
 set(EMIA_Win->BT_GBDens[1], MUIA_Disabled, (! GridBounds));
 set(EMIA_Win->BT_GBDens[2], MUIA_Disabled, (! GridBounds));

} /* DisableBoundsButtons() */

/************************************************************************/

void FixXYZ(char item)
{
 ActivateWindow(EMIA_Win->Win);

 switch (item)
  {
  case 'x':
   {
   set(EMIA_Win->BT_MoveX, MUIA_Selected, IA->fixX);
   break;
   } 
  case 'y':
   {
   set(EMIA_Win->BT_MoveY, MUIA_Selected, IA->fixY);
   break;
   }
  case 'z':
   {
   set(EMIA_Win->BT_MoveZ, MUIA_Selected, IA->fixZ);
   break;
   }
  } /* switch item */

 ActivateWindow(InterWind0);

} /* FixXYZ() */

/************************************************************************/

void Init_IA_View(short view)
{
 IA->scrnwidth = IA->cb.highx - IA->cb.lowx;
 IA->scrnheight = IA->cb.highy - IA->cb.lowy;

 IA->wscale = (double)settings.scrnwidth / IA->scrnwidth;
 IA->hscale = (double)settings.scrnheight / IA->scrnheight;

 if (view) IA->recompute = 1;
 if (view == 1) drawgridview();
 else if (view == 2)
  {
  constructview();
  DrawInterFresh(1);
  } /* else */

} /* Init_IA_View() */

/************************************************************************/

void Init_IA_Item(short MoItem, short view)
{

 if (view) IA->recompute = 1;
 if (view == 1) drawgridview();
 else if (view == 2)
  {
  constructview();
  DrawInterFresh(1);
  } /* else */

} /* Init_IA_Item() */

/********************************************************************/

short CheckDEMStatus(void)
{
short i, j, found, reload = 0;

if (! AltRenderList)
{
 return (0);
}

for (i=0; i<NoOfObjects; i++)
 {
 if (! strcmp(DBase[i].Special, "TOP") || ! strcmp(DBase[i].Special, "SFC"))
  {
  found = 0;
  for (j=0; j<NoOfElMaps; j++)
   {
   if (i == *(AltRenderList + j * 2))
    {
    found = 1;
    if (! (DBase[i].Flags & 2))
     {
     reload = 2;
     break;
     } /* if disabled */
    } /* if match found */
   } /* for j=0... */
  if (! found)
   {
   if (DBase[i].Flags & 2)
    {
    reload = 1;
    } /* if enabled */
   } /* else not already loaded */
  if (reload)
   break;
  } /* if DEM */
 } /* for i=0... */

 return (reload);

} /* CheckDEMStatus() */
