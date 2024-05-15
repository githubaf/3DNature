/* DefaultParams.c (ne gisedpar.c 14 Jan 1994 CXH)
** Default Parameter creating function for WCS
** Original code built 9 September, 1992 by Gary R. Huber.
** Incorporated into GIS on 27 July, 1993 by Gary R. Huber.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"

STATIC_FCN void SetParEco(short Eco, char *Name, short Line, short Skew, short SkewAz,
        short RelEl, short MaxRelEl, short MinRelEl, short MaxSlope,
        short MinSlope, short Color, short Understory, short MatchRed,
        short MatchGrn, short MatchBlu, short Type, short TreeDens,
        short TreeHt); // used locally only -> static, AF 19.7.2021
STATIC_FCN void SetParColor(short Color, char *Name, short Red, short Grn, short Blu); // used locally only -> static, AF 26.7.2021


short DefaultParams(void)
{
 short i, FocMap, error = 0;
 double CtrLat = 0.0, CtrLon = 0.0, lonscale, AvgLat, AvgLon, LatDist, LonDist,
	CtrDist, CloseDist;

/* check for timelines */
 if (EMTL_Win || ECTL_Win || EETL_Win)
  {
  error = 2;
  goto EndDefault;
  } /* if */

/* load topos */
 if (! topoload)
  if (loadtopo())
   {
   error = 1;
   goto EndDefault;
   } /* if no topos */

/* delete all key frames */
 for (i=ParHdr.KeyFrames-1; i>=0; i--)
  {
  DeleteKeyFrame(KF[i].MoKey.KeyFrame, KF[i].MoKey.Group, KF[i].MoKey.Item,
	0, 0);
  } /* for i=0... */

/* find topo nearest center */
 for (i=0; i<topomaps; i++)
  {
  CtrLat += mapcoords[i].C[0] + mapcoords[i].C[2];
  CtrLon += mapcoords[i].C[1] + mapcoords[i].C[3];
  } /* for i=0... */

 CtrLat /= (2.0 * topomaps);
 CtrLon /= (2.0 * topomaps);

 lonscale = LATSCALE * cos(CtrLat * PiOver180);

 CloseDist = FLT_MAX;
 for (i=0; i<topomaps; i++)
  {
  AvgLat = (mapcoords[i].C[0] + mapcoords[i].C[2]) / 2.0;
  AvgLon = (mapcoords[i].C[1] + mapcoords[i].C[3]) / 2.0;
  LatDist = (CtrLat - AvgLat) * LATSCALE;
  LonDist = (CtrLon - AvgLon) * lonscale;
  CtrDist = sqrt(LatDist * LatDist + LonDist * LonDist);
  if (CtrDist < CloseDist)
   {
   CloseDist = CtrDist;
   FocMap = i;
   } /* if closer to center */
  } /* for i=0... */

/* find height of map */
 LatDist = (mapcoords[FocMap].C[2] - mapcoords[FocMap].C[0]) * LATSCALE;
  
/* set camera and focus altitudes, lat, lon, sun lat/lon */
 
 PAR_FIRST_MOTION(3) = mapelmap[FocMap].elscale
	 * (mapelmap[FocMap].MaxEl + mapelmap[FocMap].MinEl) / 2;
 PAR_FIRST_MOTION(0) = PAR_FIRST_MOTION(3) + LatDist / 4.0;
 PAR_FIRST_MOTION(4) = (mapcoords[FocMap].C[0] + mapcoords[FocMap].C[2]) / 2.0;
 if (PAR_FIRST_MOTION(4) > 0.0)
  {
  PAR_FIRST_MOTION(1) = (mapcoords[FocMap].C[0]);
  PAR_FIRST_MOTION(15) = PAR_FIRST_MOTION(4) - 30.0;
  }
 else
  {
  PAR_FIRST_MOTION(1) = (mapcoords[FocMap].C[2] + PAR_FIRST_MOTION(4)) / 2.0;
  PAR_FIRST_MOTION(15) = PAR_FIRST_MOTION(4) + 30.0;
  }
 PAR_FIRST_MOTION(5) = (mapcoords[FocMap].C[1] + mapcoords[FocMap].C[3]) / 2.0;
 PAR_FIRST_MOTION(2) = PAR_FIRST_MOTION(5);
 PAR_FIRST_MOTION(16) = PAR_FIRST_MOTION(5) - 30.0;

/* set settings */

 defaultsettings();
 settings.surfel[0] = MapLowEl;
 settings.surfel[1] = MapLowEl + (MapHighEl - MapLowEl) * .3333;
 settings.surfel[2] = MapLowEl + (MapHighEl - MapLowEl) * .6667;
 settings.surfel[3] = MapHighEl;

/* set other motion params */

 PAR_FIRST_MOTION(6) = settings.scrnwidth / 2;	/* Center X */
 PAR_FIRST_MOTION(7) = settings.scrnheight / 2;	/* Center Y */
 PAR_FIRST_MOTION(8) = 0.0;			/* Bank */
 PAR_FIRST_MOTION(9) = 0.0;			/* Earth Rotation Rate */
 PAR_FIRST_MOTION(10) = 0.5;			/* Scale */
 PAR_FIRST_MOTION(11) = 90.0;			/* View Arc */
 PAR_FIRST_MOTION(12) = 0.0;			/* Flattening */
 PAR_FIRST_MOTION(13) = 0.0;			/* Datum */
 PAR_FIRST_MOTION(14) = 1.0;			/* Vertical Exageration */
 PAR_FIRST_MOTION(17) = 30.0;			/* Horizon Line */
 PAR_FIRST_MOTION(18) = 50.0;			/* Horizon Point */
 PAR_FIRST_MOTION(19) = 15.0;			/* Horizon Stretch */
 PAR_FIRST_MOTION(20) = 100.0;			/* Haze Start */
 PAR_FIRST_MOTION(21) = 200.0;			/* Haze Range */
 PAR_FIRST_MOTION(22) = .95;			/* Shade Factor */
 PAR_FIRST_MOTION(23) = 0.0;			/* Fog Full */
 PAR_FIRST_MOTION(24) = 0.0;			/* Fog None */
 PAR_FIRST_MOTION(25) = 0.0;			/* Q Minimum */

/* colors */

 memset(&CoPar, 0, sizeof (CoPar));
 SetParColor(0,  (char*)GetString( MSG_DEFPARM_SUN ),          128, 128, 128);  // "Sun"
 SetParColor(1,  (char*)GetString( MSG_DEFPARM_AMBIENT ),        0,   0,  10);  // "Ambient"
 SetParColor(2,  (char*)GetString( MSG_DEFPARM_HAZE ),         218, 218, 251);  // "Haze"
 SetParColor(3,  (char*)GetString( MSG_DEFPARM_HORIZON ),      255, 255, 255);  // "Horizon"
 SetParColor(4,  (char*)GetString( MSG_DEFPARM_ZENITH ),        80, 130, 255);  // "Zenith"
 SetParColor(5,  (char*)GetString( MSG_DEFPARM_SURFACEGRID ),    0,   0,   0);  // "Surface Grid"
 SetParColor(6,  (char*)GetString( MSG_DEFPARM_SURFACE1 ),     240, 100,  81);  // "Surface 1"
 SetParColor(7,  (char*)GetString( MSG_DEFPARM_SURFACE2 ),     130, 219,  95);  // "Surface 2"
 SetParColor(8,  (char*)GetString( MSG_DEFPARM_SURFACE3 ),     136, 154, 255);  // "Surface 3"
 SetParColor(9,  (char*)GetString( MSG_DEFPARM_SURFACE4 ),     255, 255, 255);  // "Surface 4"
 SetParColor(10, (char*)GetString( MSG_DEFPARM_WATER ),        104, 141, 173);  // "Water"
 SetParColor(11, (char*)GetString( MSG_DEFPARM_SNOW ) ,        229, 229, 229);  // "Snow"
 SetParColor(24, (char*)GetString( MSG_DEFPARM_TUNDRA ) ,      113, 155, 100);  // "Tundra"
 SetParColor(25, (char*)GetString( MSG_DEFPARM_WETLAND ) ,     130, 155,  90);  // "Wetland"
 SetParColor(26, (char*)GetString( MSG_DEFPARM_GRASS ) ,        93, 163, 114);  // "Grass"
 SetParColor(27, (char*)GetString( MSG_DEFPARM_DECIDUOUS ),     81, 155, 100);  // "Deciduous"
 SetParColor(28, (char*)GetString( MSG_DEFPARM_CONIFER ),       49, 111,  72);  // "Conifer"
 SetParColor(29, (char*)GetString( MSG_DEFPARM_GRANITE ),      173, 145, 139);  // "Granite"
 SetParColor(30, (char*)GetString( MSG_DEFPARM_GROUND ),       175, 167, 137);  // "Ground"

/* ecosystems */

 for (i=0; i<ECOPARAMS; i++)
  setecodefault(i);

 SetParEco( 0, (char*)GetString( MSG_DEFPARM_WATER ), 0,                                                  // "Water"
	5000,   0,   0,  1000, -1000, 90,  0, 10, 0, 0, 0, 0, 0,   0,  0);
 SetParEco( 1, (char*)GetString( MSG_DEFPARM_SNOW ), settings.surfel[2],                                  // "Snow"
	200,  -55,   5,  1000, -1000, 35,  0, 11, 1, 0, 0, 0, 1,   0,  0);
 SetParEco(12, (char*)GetString( MSG_DEFPARM_WETLAND ), 32000,                                            // "Wetland"
	0,      0,   0,   -80, -1000, 15,  0, 25, 12, 0, 0, 0, 6, 100,  1),
 SetParEco(13, (char*)GetString( MSG_DEFPARM_RIPARIAN ), settings.surfel[2],                              // "Riparian"
	200,   45,  -5,   -20, -1000, 30,  0, 27, 14, 0, 0, 0, 5,  30, 20);
 SetParEco(14, (char*)GetString( MSG_DEFPARM_GRASS ), (settings.surfel[0] + settings.surfel[1]) / 2,      // "Grass"
	-1000, 45,  -2,  1000, -1000, 30,  0, 26, 14, 0, 0, 0, 6, 100,  1);

 SetParEco(15, (char*)GetString( MSG_DEFPARM_DECIDUOUS ), (settings.surfel[1] + settings.surfel[2]) / 2,  // "Deciduous"
	400,    0,   1,  1000, -1000, 30,  0, 27, 14, 0, 0, 0, 5,  50, 20);
 SetParEco(16, (char*)GetString( MSG_DEFPARM_CONIFER ), settings.surfel[2],                               // "Conifer"
	-250,  75,  -1,  1000, -1000, 30,  0, 28, 19, 0, 0, 0, 4,  75, 25);
 SetParEco(17, (char*)GetString( MSG_DEFPARM_TUNDRA ), (settings.surfel[2] + settings.surfel[3]) / 2,     // "Tundra"
	200,   45,  -1,  1000, -1000, 35,  0, 24, 17, 0, 0, 0, 6, 100, 1);
 SetParEco(18, (char*)GetString( MSG_DEFPARM_ROCK ), 32000,                                               // "Rock"
	0,      0,   0,  1000, -1000, 90, 35, 29, 18, 0, 0, 0, 2, 100, 0);
 SetParEco(19, (char*)GetString( MSG_DEFPARM_BAREGROUND ), 32000,                                         // "Bare Ground"
	0,      0,   0,  1000, -1000, 90,  0, 30, 19, 0, 0, 0, 3, 100, 0);

 strcpy(ParHdr.FType, "%WCSPAR");
 ParHdr.Version = PAR_CURRENT_VERSION;
 ParHdr.ByteOrder = 0xaabbccdd;
 ParHdr.KeyFrames = 0;
 Par_Mod = 0x1111;
 if (KF) free_Memory(KF, KFsize);
 KFsize = (ParHdr.KeyFrames + 20) * (sizeof (union KeyFrame));
 if ((KF = (union KeyFrame *)get_Memory(KFsize, MEMF_CLEAR)) == NULL)
  {
  error = 3;
  goto EndDefault;
  } /* if memory bust */

EndDefault:

 if (topoload && ! MapWind0)
  {
  if (mapelmap)
   {
   for (i=0; i<topomaps; i++)
    {
    if (mapelmap[i].map) free_Memory(mapelmap[i].map, mapelmap[i].size);
    } /* for i=0... */
   free_Memory(mapelmap, MapElmapSize);
   mapelmap = NULL;
   } /* if */
  if (TopoOBN) free_Memory(TopoOBN, TopoOBNSize);
  TopoOBN = NULL;
  if (mapcoords) free_Memory(mapcoords, MapCoordSize);
  mapcoords = NULL;
  topoload = 0;
  topomaps = 0;
  } /* if topos loaded previously */

 switch (error)
  {
  case 1:
   {
   User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                  // "Parameters Module: Defaults"
                GetString( MSG_DEFPARM_PLEASEENABLEATLEASTONETOPODEMANDTRYAGAIN ),  // "Please enable at least one topo DEM and try again."
                GetString( MSG_GLOBAL_OK ),                                        // "OK"
                (CONST_STRPTR)"o");
   break;
   } /* no topos */
  case 2:
   {
   User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                   // "Parameters Module: Defaults"
                GetString( MSG_DEFPARM_PLEASECLOSEALLTIMELINESWINDOWSANDTRYAGAIN ),  // "Please close all Time Lines windows and try again."
                GetString( MSG_GLOBAL_OK ),                                         // "OK"
                (CONST_STRPTR)"o");
   break;
   } /* no topos */
  case 3:
   {
   User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),  // "Parameters Module: Defaults"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),               // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),                        // "OK"
                (CONST_STRPTR)"o");
   break;
   } /* no topos */
  } /* switch */

 return ((short)(! error));

} /* DefaultParams() */

/***********************************************************************/

STATIC_FCN void SetParColor(short Color, char *Name, short Red, short Grn, short Blu) // used locally only -> static, AF 26.7.2021
{

 strcpy(PAR_NAME_COLOR(Color), Name);
 PAR_FIRST_COLOR(Color, 0) = Red;
 PAR_FIRST_COLOR(Color, 1) = Grn;
 PAR_FIRST_COLOR(Color, 2) = Blu;

} /* SetParColor() */

/***********************************************************************/

STATIC_FCN void SetParEco(short Eco, char *Name, short Line, short Skew, short SkewAz,
	short RelEl, short MaxRelEl, short MinRelEl, short MaxSlope,
	short MinSlope, short Color, short Understory, short MatchRed,
	short MatchGrn, short MatchBlu, short Type, short TreeDens,
	short TreeHt) // used locally only -> static, AF 19.7.2021
{

 strcpy(PAR_NAME_ECO(Eco), Name);
 PAR_FIRSTLN_ECO(Eco) = Line;
 PAR_FIRSTSK_ECO(Eco) = Skew;
 PAR_FIRSTSA_ECO(Eco) = SkewAz;
 PAR_FIRSTRE_ECO(Eco) = RelEl;
 PAR_FIRSTXR_ECO(Eco) = MaxRelEl;
 PAR_FIRSTNR_ECO(Eco) = MinRelEl;
 PAR_FIRSTXS_ECO(Eco) = MaxSlope;
 PAR_FIRSTNS_ECO(Eco) = MinSlope;
 PAR_FIRSTDN_ECO(Eco) = TreeDens;
 PAR_FIRSTHT_ECO(Eco) = TreeHt;

 PAR_COLR_ECO(Eco)    = Color;
 PAR_UNDER_ECO(Eco)   = Understory;
 PAR_MTCH_ECO(Eco, 0) = MatchRed;
 PAR_MTCH_ECO(Eco, 1) = MatchGrn;
 PAR_MTCH_ECO(Eco, 2) = MatchBlu;
 PAR_TYPE_ECO(Eco)    = Type;

} /* SetParColor() */

/***********************************************************************/

