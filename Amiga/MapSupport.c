/* MapSupport.c (ne gismapsupport.c 14 Jan 1994 CXH)
** Support functions for 2-D mapping.
** Code by Gary R. Huber, 1992 - 1993.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"
#include "GUIDefines.h"
#include "BigEndianReadWrite.h"

STATIC_VAR double lat_y, lon_x;

STATIC_FCN void DrawSun(struct Window *Win, int X, int Y, int Rad, struct clipbounds *cb); // used locally only -> static, AF 26.7.2021


#define MAPPREFS_DBL_MAPSCALE	2
#define MAPPREFS_DBL_MAPLAT	3
#define MAPPREFS_DBL_MAPLON	4
#define MAPPREFS_DBL_LATZERO	5
#define MAPPREFS_DBL_LONZERO	6
#define MAPPREFS_DBL_LATSCALEF	7
#define MAPPREFS_DBL_LONSCALEF	8
#define MAPPREFS_DBL_LATY	9
#define MAPPREFS_DBL_LONX	10
#define MAPPREFS_DBL_YLAT	11
#define MAPPREFS_DBL_XLON	12
#define MAPPREFS_SHT_LOWX	13
#define MAPPREFS_SHT_LOWY	14
#define MAPPREFS_SHT_HIGHX	15
#define MAPPREFS_SHT_HIGHY	16
#define MAPPREFS_DBL_RLAT0	17
#define MAPPREFS_DBL_RLON0	18
#define MAPPREFS_DBL_RLAT1	19
#define MAPPREFS_DBL_RLON1	20
#define MAPPREFS_SHT_TOPO	21
#define MAPPREFS_SHT_ALIGN	22
#define MAPPREFS_SHT_VECENABLE	23
#define MAPPREFS_SHT_INTERSTUFF	24
#define MAPPREFS_SHT_AUTOCLEAR	25
#define MAPPREFS_SHT_ECOENABLE	26
#define MAPPREFS_DBL_EXAG	27
	
void savemapprefs(void)
{
 FILE *fprefs;
 char filename[256];

 strmfp(filename, dirname, "Map.Prefs");

 if ((fprefs = fopen(filename, "w")) == NULL)
  {
  return;
  } /* if open fail */

 fprintf(fprefs, "%s\n", "WCSMVPrefs");

 fprintf(fprefs, "%d\n", MAPPREFS_SHT_TOPO);
 fprintf(fprefs, "%hu\n", topo);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_ALIGN);
 fprintf(fprefs, "%hu\n", align);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_VECENABLE);
 fprintf(fprefs, "%hu\n", vectorenabled);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_ECOENABLE);
 fprintf(fprefs, "%hu\n", ecoenabled);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_INTERSTUFF);
 fprintf(fprefs, "%hu\n", InterStuff);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_AUTOCLEAR);
 fprintf(fprefs, "%hu\n", AutoClear);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_LOWX);
 fprintf(fprefs, "%hd\n", AlignBox.Low.X);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_LOWY);
 fprintf(fprefs, "%hd\n", AlignBox.Low.Y);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_HIGHX);
 fprintf(fprefs, "%hd\n", AlignBox.High.X);
 fprintf(fprefs, "%d\n", MAPPREFS_SHT_HIGHY);
 fprintf(fprefs, "%hd\n", AlignBox.High.Y);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_MAPSCALE);
 fprintf(fprefs, "%f\n", mapscale);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_MAPLAT);
 fprintf(fprefs, "%f\n", maplat);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_MAPLON);
 fprintf(fprefs, "%f\n", maplon);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LATZERO);
 fprintf(fprefs, "%f\n", latzero);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LONZERO);
 fprintf(fprefs, "%f\n", lonzero);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LATSCALEF);
 fprintf(fprefs, "%f\n", latscalefactor);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LONSCALEF);
 fprintf(fprefs, "%f\n", lonscalefactor);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LATY);
 fprintf(fprefs, "%f\n", lat_y);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_LONX);
 fprintf(fprefs, "%f\n", lon_x);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_YLAT);
 fprintf(fprefs, "%f\n", y_lat);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_XLON);
 fprintf(fprefs, "%f\n", x_lon);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_RLAT0);
 fprintf(fprefs, "%f\n", rlat[0]);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_RLON0);
 fprintf(fprefs, "%f\n", rlon[0]);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_RLAT1);
 fprintf(fprefs, "%f\n", rlat[1]);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_RLON1);
 fprintf(fprefs, "%f\n", rlon[1]);
 fprintf(fprefs, "%d\n", MAPPREFS_DBL_EXAG);
 fprintf(fprefs, "%f\n", MaxElevDiff);

 fclose(fprefs);

} /* savemapprefs() */

/************************************************************************/

short readmapprefs(void)
{
 FILE *fprefs;
 char filename[256];
 long item;

 if (! MP)
  return (0);

 strmfp(filename, dirname, "Map.Prefs");

 if ((fprefs = fopen(filename, "r")) == NULL)
  {
  return (0);
  } /* if open fail */

 fgets(filename, 12, fprefs);
 filename[10] = '\0';
 if (strcmp(filename, "WCSMVPrefs"))
  {
  fclose(fprefs);
  return (0);
  } /* if not WCS Map Prefs file */
 for (; ;)
  {
  if (fscanf(fprefs, "%ld", &item) == EOF) break;
  switch (item)
   {
   case MAPPREFS_SHT_TOPO:
    {
    fscanf(fprefs, "%hu", &topo);
    break;
    } /*  */
   case MAPPREFS_SHT_ALIGN:
    {
    fscanf(fprefs, "%hu", &align);
    break;
    } /*  */
   case MAPPREFS_SHT_VECENABLE:
    {
    fscanf(fprefs, "%hu", &vectorenabled);
    break;
    } /*  */
   case MAPPREFS_SHT_ECOENABLE:
    {
    fscanf(fprefs, "%hu", &ecoenabled);
    break;
    } /*  */
   case MAPPREFS_SHT_INTERSTUFF:
    {
    fscanf(fprefs, "%hu", &InterStuff);
    break;
    } /*  */
   case MAPPREFS_SHT_AUTOCLEAR:
    {
    fscanf(fprefs, "%hu", &AutoClear);
    break;
    } /*  */
   case MAPPREFS_SHT_LOWX:
    {
    fscanf(fprefs, "%hd", &AlignBox.Low.X);
    break;
    } /*  */
   case MAPPREFS_SHT_LOWY:
    {
    fscanf(fprefs, "%hd", &AlignBox.Low.Y);
    break;
    } /*  */
   case MAPPREFS_SHT_HIGHX:
    {
    fscanf(fprefs, "%hd", &AlignBox.High.X);
    break;
    } /*  */
   case MAPPREFS_SHT_HIGHY:
    {
    fscanf(fprefs, "%hd", &AlignBox.High.Y);
    break;
    } /*  */
   case MAPPREFS_DBL_MAPSCALE:
    {
    fscanf(fprefs, "%le", &mapscale);
    break;
    } /*  */
   case MAPPREFS_DBL_MAPLAT:
    {
    fscanf(fprefs, "%le", &maplat);
    break;
    } /*  */
   case MAPPREFS_DBL_MAPLON:
    {
    fscanf(fprefs, "%le", &maplon);
    break;
    } /*  */
   case MAPPREFS_DBL_LATZERO:
    {
    fscanf(fprefs, "%le", &latzero);
    break;
    } /*  */
   case MAPPREFS_DBL_LONZERO:
    {
    fscanf(fprefs, "%le", &lonzero);
    break;
    } /*  */
   case MAPPREFS_DBL_LATSCALEF:
    {
    fscanf(fprefs, "%le", &latscalefactor);
    break;
    } /*  */
   case MAPPREFS_DBL_LONSCALEF:
    {
    fscanf(fprefs, "%le", &lonscalefactor);
    break;
    } /*  */
   case MAPPREFS_DBL_LATY:
    {
    fscanf(fprefs, "%le", &lat_y);
    break;
    } /*  */
   case MAPPREFS_DBL_LONX:
    {
    fscanf(fprefs, "%le", &lon_x);
    break;
    } /*  */
   case MAPPREFS_DBL_YLAT:
    {
    fscanf(fprefs, "%le", &y_lat);
    break;
    } /*  */
   case MAPPREFS_DBL_XLON:
    {
    fscanf(fprefs, "%le", &x_lon);
    break;
    } /*  */
   case MAPPREFS_DBL_RLAT0:
    {
    fscanf(fprefs, "%le", &rlat[0]);
    break;
    } /*  */
   case MAPPREFS_DBL_RLON0:
    {
    fscanf(fprefs, "%le", &rlon[0]);
    break;
    } /*  */
   case MAPPREFS_DBL_RLAT1:
    {
    fscanf(fprefs, "%le", &rlat[1]);
    break;
    } /*  */
   case MAPPREFS_DBL_RLON1:
    {
    fscanf(fprefs, "%le", &rlon[1]);
    break;
    } /*  */
   case MAPPREFS_DBL_EXAG:
    {
    fscanf(fprefs, "%le", &MaxElevDiff);
    break;
    } /*  */
   } /* switch */

  } /* for */

 fclose(fprefs);

 return (1);

} /* readmapprefs() */

/************************************************************************/

short saveobject(long OBN, char *fname, double *Lon, double *Lat, short *Elev)
{
 char filename[255], filetype[10];
 short j, OpenOK, WriteOK = 0;
 long Size;
 float Version;
 double LatSum = 0.0, LonSum = 0.0;
 struct vectorheaderV100 Hdr={0};  // init, AF, 23.Mar.23
 struct DirList *DLItem;
 FILE *fobject;

 Version = VEC_CURRENT_VERSION;
 if (! Lat || ! Lon || ! Elev)
  {
  return (1);
  }

 OpenOK = 0;

 if (! fname)
  {
  DLItem = DL;
  while (DLItem)
   {
   if (DLItem->Read == '*')
    {
    DLItem = DLItem->Next;
    continue;
    } /* if directory is write protected */
   strmfp(filename, DLItem->Name, DBase[OBN].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    }
   else
    {
    fclose(fobject);
    if ((fobject = fopen(filename, "wb")) != NULL)
     OpenOK = 1;
    break;
    }
   } /* while */

  if (! OpenOK)
   {
   strmfp(filename, dirname, DBase[OBN].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "wb")) == NULL)
    {
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)DBase[OBN].Name);
    User_Message((CONST_STRPTR)DBase[OBN].Name, GetString( MSG_MAPSUPRT_CANTOPENOBJECTFILEBJECTNOTSAVED ),  // "Can't open object file!\nObject not saved."
                 GetString( MSG_GLOBAL_OK ),                                                              // "OK"
                 (CONST_STRPTR)"o");
    return(1);
    } /* if open fail */
   } /* if pre-existing file not found */
  } /* if no supplied name */
 else
  {
  if ((fobject = fopen(fname, "wb")) == NULL)
   {
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)fname);
   return(1);
   } /* if open fail */
  } /* if name passed to saveobject() */

 strcpy(filetype, "WCSVector");
 strncpy(Hdr.Name, DBase[OBN].Name, 10);
 Hdr.points = DBase[OBN].Points;
 Hdr.elevs = 0;
 
 for (j=1; j<=DBase[OBN].Points; j++)
  {
  LatSum += Lat[j];
  LonSum += Lon[j];
  } /* for j=0... */
 Hdr.avglat = LatSum / Hdr.points;
 Hdr.avglon = LonSum / Hdr.points;
 Hdr.elscale = ELSCALE_METERS;

 Size = (DBase[OBN].Points + 1) * sizeof (double);

 if (fwrite((char *)filetype, 9, 1, fobject) == 1)
  {
  if (fwrite_float_BE(&Version,fobject)==1)   //(fwrite((char *)&Version, sizeof (float), 1, fobject) == 1) AF: 22.Mar.23
   {
   if (fwriteVectorheaderV100_BE(&Hdr, fobject) == 1) // (fwrite((char *)&Hdr, sizeof (struct vectorheaderV100), 1, fobject) == 1)  AF: 22.Mar.23
    {
    if (fwrite_double_Array_BE(&Lon[0], Size,fobject)==1) //(fwrite((char *)&Lon[0], Size, 1, fobject) == 1) AF: 22.Mar.23
     {
     if (fwrite_double_Array_BE(&Lat[0], Size, fobject) == 1) // (fwrite((char *)&Lat[0], Size, 1, fobject) == 1) AF: 22.Mar.23
      {
      if (fwrite_SHORT_Array_BE(&Elev[0], Size / 4, fobject) == 1) // (fwrite((char *)&Elev[0], Size / 4, 1, fobject) == 1) AF: 22.Mar.23
       {
       WriteOK = 1;
       }
      }
     }
    }
   }
  }
 fclose(fobject);
 
 if (WriteOK)
  {
  DBase[OBN].Flags &= (255 ^ 1);
  sprintf(str, (char*)GetString( MSG_MAPSUPRT_VECTORSAVEDPOINTS ), DBase[OBN].Name, DBase[OBN].Points);  // "%s vector saved. %d points"
  Log(MSG_NULL, (CONST_STRPTR)str);
  DB_Mod = 1;
  }
 else
  {
  User_Message((CONST_STRPTR)DBase[OBN].Name, GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),  // "Error saving object file!\nObject not saved."
               GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),                                 // "OK"
               (CONST_STRPTR)"o");
  Log(ERR_WRITE_FAIL, (CONST_STRPTR)DBase[OBN].Name);
  return (1);
  }

 return(0);

} /* saveobject() */

/**********************************************************************/

short loadtopo(void)
{
 char filename[255];
 short i, topoct = 0, error = 0, OpenOK;
 short *mapptr;
 long Lr, Lc, ElDif, Samples = 0;
 float SumElDif = 0.0, SumElDifSq = 0.0;
 struct DirList *DLItem;
 struct BusyWindow *BusyLoad;

 if (topoload)
  {
  if (mapelmap)
   {
   for (i=0; i<topomaps; i++)
    {
    if (mapelmap[i].map) free_Memory(mapelmap[i].map, mapelmap[i].size);
    } /* for i=0... */
   free_Memory(mapelmap, MapElmapSize);
   mapelmap = NULL;
   } /* if mapelmap */
  if (mapcoords) free_Memory(mapcoords, MapCoordSize);
  mapcoords = NULL;
  if (TopoOBN) free_Memory(TopoOBN, TopoOBNSize);
  TopoOBN = NULL;
  } /* if topos already in memory */
 topomaps = topoload = 0;
 MapHighEl = -30000;
 MapLowEl = 30000;

 for (i=0; i<NoOfObjects; i++)
   { /* estimate number of topos */
   if ((! strcmp(DBase[i].Special, "TOP") || ! strcmp(DBase[i].Special, "SFC"))
	&& (DBase[i].Flags & 2))
     topomaps++;
   } /* for */


 if (topomaps <= 0)
  {
  User_Message(GetString( MSG_MAPSUPRT_MAPPINGMODULETOPOMAPPING ),                            // "Mapping Module: Topo Mapping"
               GetString( MSG_MAPSUPRT_NOTOPOMAPSFOUNDHECKOBJECTENABLEDSTATUSANDCLASSINDA ),  // "No topo maps found!\nCheck object Enabled Status and Class in database."
               GetString( MSG_GLOBAL_OK ),                                                  // "OK"
               (CONST_STRPTR)"o");
  return (1);
  } /* if */

 MapElmapSize = topomaps * sizeof (struct elmapheaderV101);
 MapCoordSize = topomaps * sizeof (struct MapCoords);
 TopoOBNSize = topomaps * sizeof (float);

 if (((mapelmap = (struct elmapheaderV101 *)get_Memory(MapElmapSize, MEMF_CLEAR))
	== NULL) ||
	((mapcoords = (struct MapCoords *)get_Memory(MapCoordSize, MEMF_CLEAR))
	== NULL) ||
	((TopoOBN = (short *)get_Memory(TopoOBNSize, MEMF_CLEAR)) == NULL))
  {
  User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),               // "Map View: Load Topos"
               GetString( MSG_MAPSUPRT_OUTOFMEMORYPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                             // "OK"
               (CONST_STRPTR)"o");
  return (1);
  } /* if memory fail */

 BusyLoad = BusyWin_New((char*)GetString( MSG_MAPSUPRT_TOPOLOAD ), topomaps, 0, MakeID('M','T','L','D'));  // "Topo Load"
 topomaps = 0;

 for (i=0; i<NoOfObjects; i++)
  {
  if (strcmp(DBase[i].Special, "TOP") && strcmp(DBase[i].Special, "SFC"))
   continue;
  if (! (DBase[i].Flags & 2)) continue;

  OpenOK = 0;

  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, DBase[i].Name);
   strcat(filename, ".elev");
   if (readDEM(filename, &mapelmap[topoct]) != 0)
    {
    DLItem = DLItem->Next;
    } /* if open/read fails */
   else
    {
    OpenOK = 1;
    break;
    } /* else open succeeds */
   } /* while */

  if (! OpenOK)
   {
   error = 1;
   sprintf(str, "%s.elev", DBase[i].Name);
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)str);
   User_Message((CONST_STRPTR)str, GetString( MSG_MAPSUPRT_ERRORLOADINGTOPOMAPCHECKSTATUSLOGTOSEEIFOUTOFMEMOR ),  // "Error loading topo map! Check Status Log to see if out of memory.\nOperation terminated."
                GetString( MSG_GLOBAL_OK ),                                                                     // "OK"
                (CONST_STRPTR)"o");
   break;
   } /* if file not found */

  mapcoords[topoct].C[0] = mapelmap[topoct].lolat;
  mapcoords[topoct].C[1] = mapelmap[topoct].lolong;
  mapcoords[topoct].C[2] = mapelmap[topoct].lolat
	+ (mapelmap[topoct].columns - 1) * mapelmap[topoct].steplat;
  mapcoords[topoct].C[3] = mapelmap[topoct].lolong
	- (mapelmap[topoct].rows * mapelmap[topoct].steplong);
  TopoOBN[topoct] = i;

/* compare elevations */
  if (mapelmap[topoct].MaxEl == mapelmap[topoct].MinEl && mapelmap[topoct].MaxEl == 0)
   {
   mapelmap[topoct].MaxEl = -30000;
   mapelmap[topoct].MinEl = 30000;
   mapelmap[topoct].Samples = 0;
   mapelmap[topoct].SumElDif = mapelmap[topoct].SumElDifSq = 0.0;
   mapptr = mapelmap[topoct].map;
   for (Lr=0; Lr<=mapelmap[topoct].rows; Lr++)
    {
    for (Lc=0; Lc<mapelmap[topoct].columns; Lc++)
     {
     if (*mapptr > mapelmap[topoct].MaxEl)
      {
      mapelmap[topoct].MaxEl = *mapptr;
      } /* if */
     else if (*mapptr < mapelmap[topoct].MinEl)
      {
      mapelmap[topoct].MinEl = *mapptr;
      } /* else if */
     if (Lr != 0 && Lc != mapelmap[topoct].columns - 1)
      {
      ElDif = abs(*mapptr - *(mapptr - mapelmap[topoct].columns + 1));
      mapelmap[topoct].SumElDif += ElDif;
      mapelmap[topoct].SumElDifSq += ElDif * ElDif;
      mapelmap[topoct].Samples ++;
      } /* if not first row */
     mapptr ++;
     } /* for Lc=0... */
    } /* for Lr=0... */
   } /* if version 1.0- file */
  
  if (mapelmap[topoct].MaxEl * (mapelmap[topoct].elscale * 1000.0) > MapHighEl)
   MapHighEl = mapelmap[topoct].MaxEl * mapelmap[topoct].elscale * 1000.0;
  if (mapelmap[topoct].MinEl * mapelmap[topoct].elscale * 1000.0 < MapLowEl)
   MapLowEl = mapelmap[topoct].MinEl * mapelmap[topoct].elscale * 1000.0;
  SumElDif += (mapelmap[topoct].SumElDif * mapelmap[topoct].elscale * 1000.0);
  SumElDifSq += (mapelmap[topoct].SumElDifSq
	 * (mapelmap[topoct].elscale * 1000.0) * (mapelmap[topoct].elscale * 1000.0));
  Samples += mapelmap[topoct].Samples;

  topoct ++;
  if(BusyLoad)
  {
    if(CheckInput_ID() == ID_BW_CLOSE)
      {
      error = 1;
      break;
      } /* if */
  }
  if(BusyLoad)
  	{
  	BusyWin_Update(BusyLoad, topoct);
  	} /* if */

 } /* for i=0 to NoOfObjects */

if(BusyLoad)
 {
 BusyWin_Del(BusyLoad);
 } /* if */


/* compute standard deviation factors */
 if (MaxElevDiff < 0.001)
  MaxElevDiff = 8 * sqrt((SumElDifSq - (SumElDif * SumElDif / Samples))
		 / ((float)Samples - 1));

 if (error)
  {
  if (mapelmap[topoct].map) free_Memory(mapelmap[topoct].map,
	mapelmap[topoct].size);
  } /* if */

 if (! topoct)
  {
  User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),            // "Map View: Load Topos"
               GetString( MSG_MAPSUPRT_ERRORLOADINGDEMSNONELOADED ),  // "Error loading DEMs! None loaded."
               GetString( MSG_GLOBAL_OK ),                          // "OK"
               (CONST_STRPTR)"o");
  return (1);
  } /* if */

 topoload = 1;
 topomaps = topoct;
 MapHighEl ++;
 MapLowEl --;

 return(0);

} /* loadtopo() */

/************************************************************************/

short GetBounds(struct Box *Bx)
{
 short success;

 MapGUI_Message(0, (char*)GetString( MSG_MAPSUPRT_0338SETUPPERLEFTCORNER ));            // "\0338Set upper left corner."
 SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_SETUPPERLEFTCORNER ), (UBYTE *)-1);  // "Set upper left corner"

 if (! (success = MousePtSet(&Bx->Low, NULL, 0)))
  goto EndGet;

 MapGUI_Message(0, (char*)GetString( MSG_MAPSUPRT_0338SETLOWERRIGHTCORNERESCABORT ));    // "\0338Set lower right corner. ESC=abort"
 SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_SETLOWERRIGHTCORNER ), (UBYTE *)-1);  // "Set lower right corner"

 success = MousePtSet(&Bx->High, &Bx->Low, 2);

EndGet:
 MapGUI_Message(0, " ");
 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_MAPVIEW ), (UBYTE *)-1);  // "Map View"

 return (success);

} /* GetBounds() */

/************************************************************************/

void valueset(void)
{
 short HalfHeight, HalfWidth;
 double LonFactor, LatFactor, lonscale;

 if (mapscale <= 0.0) return;
 if (maplat > 89.999) maplat = 89.999;
 else if (maplat < -89.999) maplat = -89.999;
 HalfHeight = (MapWind0->BorderTop + MapWind0->Height
	 - MapWind0->BorderBottom) / 2;
 HalfWidth = (MapWind0->BorderLeft + MapWind0->Width
	 - MapWind0->BorderRight) / 2;
 lonscale = LATSCALE * cos(maplat * PiOver180);
 LatFactor = LATSCALE * VERTPIX;
 LonFactor = lonscale * HORPIX;
 lat_y = LatFactor / mapscale;
 lon_x = LonFactor / mapscale;
 y_lat = mapscale / LatFactor;
 x_lon = mapscale / LonFactor;
 latzero = maplat + y_lat * HalfHeight; /*/ 27.044;*/
 lonzero = maplon + x_lon * HalfWidth; /*(19.657 * cos(maplat * PiOver180));*/
 latscalefactor = 1.0;
 lonscalefactor = 1.0;

} /* valueset() */

/**********************************************************************/

void valuesetalign(void)
{

 latscalefactor = y_lat / ((rlat[0] - rlat[1]) / (AlignBox.High.Y
		 - AlignBox.Low.Y));
 lonscalefactor = x_lon / ((rlon[0] - rlon[1]) / (AlignBox.High.X
		 - AlignBox.Low.X));

 lonzero = rlon[0] + AlignBox.Low.X * (rlon[0] - rlon[1])
		 / (AlignBox.High.X - AlignBox.Low.X);
 latzero = rlat[0] + AlignBox.Low.Y * (rlat[0] - rlat[1])
		 / (AlignBox.High.Y - AlignBox.Low.Y);

} /* valuesetalign() */

/************************************************************************/

short MousePtSet(struct Vertex *Vtx, struct Vertex *FixVtx, short Mode)
{
 short done = 0, success = 1;
 struct Window *ReplyWin;
 ULONG IDCMPFlags;

 IDCMPFlags = MP->MAPCWin->IDCMPFlags;

 ModifyIDCMP(MapWind0, IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);
 ModifyIDCMP(MP->MAPCWin, IDCMPFlags | IDCMP_VANILLAKEY);

 if (FixVtx && Mode)
  {
  SetDrMd(MapWind0->RPort, COMPLEMENT);
  Move(MapWind0->RPort, FixVtx->X, FixVtx->Y);
  Vtx->X = FixVtx->X;
  Vtx->Y = FixVtx->Y;
  }

 while (! done)
  {
  ReplyWin = FetchMultiWindowEvent(&Event, MapWind0, MP->MAPCWin, NULL);
  if (Event.Class == IDCMP_VANILLAKEY && Event.Code == ESC)
   {
   done = 1;
   success = 0;
   } /* if */
  else if (Event.Class == IDCMP_INTUITICKS && ReplyWin == MapWind0)
   {
   if (Event.MouseX != Vtx->X || Event.MouseY != Vtx->Y)
    {
    if (FixVtx)
     {
     if (Mode == 2)
      {
      Draw(MapWind0->RPort, Vtx->X, FixVtx->Y);
      Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
      Draw(MapWind0->RPort, FixVtx->X, Vtx->Y);
      Draw(MapWind0->RPort, FixVtx->X, FixVtx->Y);
      } /* if draw box */
     else if (Mode == 1)
      {
      Move(MapWind0->RPort, FixVtx->X, FixVtx->Y);
      Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
      } /* if draw line */
     } /* if fixed vertex supplied */
    Vtx->X = Event.MouseX;
    Vtx->Y = Event.MouseY;
    if (FixVtx)
     {
     if (Mode == 2)
      {
      Draw(MapWind0->RPort, Vtx->X, FixVtx->Y);
      Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
      Draw(MapWind0->RPort, FixVtx->X, Vtx->Y);
      Draw(MapWind0->RPort, FixVtx->X, FixVtx->Y);
      } /* if draw box */
     else if (Mode == 1)
      {
      Move(MapWind0->RPort, FixVtx->X, FixVtx->Y);
      Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
      } /* if draw line */
     sprintf(str, "->X: %d, ->Y: %d", Vtx->X - FixVtx->X, Vtx->Y - FixVtx->Y);
     } /* if fixed vertex supplied */
    else
     sprintf(str, "X: %d, Y: %d", Event.MouseX, Event.MouseY);
    LatLonElevScan(&Event, str, 0);
    } /* if position changed */
   } /* else if */
  else if (Event.Class == IDCMP_MOUSEBUTTONS && Event.Code == SELECTUP && ReplyWin == MapWind0)
   {
   done = 1;
   Vtx->X = Event.MouseX;
   Vtx->Y = Event.MouseY;
   } /* else */
  } /* while */

 if (FixVtx)
  {
  if (Mode == 2)
   {
   Draw(MapWind0->RPort, Vtx->X, FixVtx->Y);
   Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
   Draw(MapWind0->RPort, FixVtx->X, Vtx->Y);
   Draw(MapWind0->RPort, FixVtx->X, FixVtx->Y);
   } /* if draw box */
  else if (Mode == 1)
   {
   Move(MapWind0->RPort, FixVtx->X, FixVtx->Y);
   Draw(MapWind0->RPort, Vtx->X, Vtx->Y);
   } /* if draw line */
  SetDrMd(MapWind0->RPort, JAM2);
  } /* if fixed vertex supplied */

 ModifyIDCMP(MP->MAPCWin, IDCMPFlags);
 MapGUI_Message(1, " ");
 return (success);

} /* MousePtSet() */

/**********************************************************************/

long LatLonElevScan(struct IntuiMessage *Event, char *MsgSupplement,
	short FindIt)
{
 float TempLat, TempLon, SubLat, SubLon;
 long TempElev = 0, i, row, col;
 char ReadOut[80];

 if((Event->MouseX != MP->LastWinX) || (Event->MouseY != MP->LastWinY) || FindIt)
  {

  MP->LastWinX = Event->MouseX;
  MP->LastWinY = Event->MouseY;
  TempLon = X_Lon_Convert((long)Event->MouseX);
  TempLat = Y_Lat_Convert((long)Event->MouseY);
    
  i = 0;
    
  if(MP->LastObject)
   if(MP->LastObject < topomaps)
    if ((DBase[TopoOBN[MP->LastObject]].Flags & 2) && DBase[TopoOBN[MP->LastObject]].Lat)
     if (! strcmp(DBase[TopoOBN[i]].Special, "TOP") || ! strcmp(DBase[TopoOBN[i]].Special, "SFC"))
      if(!(DBase[TopoOBN[i]].Lon[2] > TempLon || DBase[TopoOBN[i]].Lon[1] < TempLon ||
       DBase[TopoOBN[i]].Lat[2] < TempLat || DBase[TopoOBN[i]].Lat[3] > TempLat))
        i = MP->LastObject;
    
  if(i == 0)
   {
   for (i=0; i<topomaps; i++)
    {
    if (((DBase[TopoOBN[i]].Flags & 2) == 0) || (DBase[TopoOBN[i]].Lat == 0)) continue;
    if (! strcmp(DBase[TopoOBN[i]].Special, "TOP") || ! strcmp(DBase[TopoOBN[i]].Special, "SFC"))
     {
     if(DBase[TopoOBN[i]].Lon[2] < TempLon || DBase[TopoOBN[i]].Lon[1] > TempLon ||
          DBase[TopoOBN[i]].Lat[2] > TempLat || DBase[TopoOBN[i]].Lat[3] < TempLat) continue;
     /* found the right one... */
     break;
     } /* if */
    } /* for i=0... */
   } /* if */

  if (MsgSupplement)
   sprintf(ReadOut, (char*)GetString( MSG_MAPSUPRT_SLATFLONF ), MsgSupplement, TempLat, TempLon);  // "%s, LAT: %f, LON: %f"
  else
   sprintf(ReadOut, (char*)GetString( MSG_MAPSUPRT_LATFLONF ), TempLat, TempLon);                  // "LAT: %f, LON: %f"
    
  if(i < topomaps)
   { /* Look up elevation */
   /* Yugg. I can't remember how this works. If I ever understood it. */
   SubLat = TempLat - mapelmap[i].lolat;
   SubLon = mapelmap[i].lolong - TempLon;
     
   col = SubLat / mapelmap[i].steplat;
   row = SubLon / mapelmap[i].steplong;
     
   TempElev = ((float)mapelmap[i].map[col + (row * mapelmap[i].columns)])
	* mapelmap[i].elscale / ELSCALE_METERS;
   sprintf(&ReadOut[strlen(ReadOut)], ", ELV: %ld", TempElev);
     
   } /* if */
    
  MapGUI_Message(1, ReadOut);
  } /* if */

 return (TempElev);
 
} /* LatLonElevScan() */  

/************************************************************************/

short XY_latlon(long i, long lowj, long highj)
{
 long j;
 short NewArray;

 NewArray = DBase[i].Lat ? 0: 1;

 if (((highj + 1) * 8 > DBase[i].VecArraySize) || NewArray)
  {
  if (! allocvecarray(i, highj, NewArray))
   {
   User_Message(GetString( MSG_MAPSUPRT_MAPPINGMODULE ),                                       // "Mapping Module"
                GetString( MSG_MAPSUPRT_OUTOFMEMORYALLOCATINGNEWVECTORARRAYPERATIONTERMINA ),  // "Out of memory allocating new vector array!\nOperation terminated."
                GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                (CONST_STRPTR)"o");
   return (0);
   } /* if not enough memory */
  } /* if array size too small */

 for (j=lowj; j<=highj; j++)
  {
  DBase[i].Lon[j] = X_Lon_Convert((long)mapxx[j]);
  DBase[i].Lat[j] = Y_Lat_Convert((long)mapyy[j]);
  } /* for j=... */

 return (1);

} /* XY_latlon() */

/************************************************************************/

double Y_Lat_Convert(long Y)
{

 return (latzero - Y * y_lat / latscalefactor);

} /* Y_Lat_Convert() */

/************************************************************************/

double X_Lon_Convert(long X)
{

 return (lonzero - X * x_lon / lonscalefactor);

} /* Y_Lat_Convert() */

/************************************************************************/

void latlon_XY(long i)
{
 long j;

 if(!DBase) // AF 15.12.2022, prevents crash on AROS when start, no project loaded, click modules -> Map view, Databse File Loader -> Cancel
 {
     return;
 }

 if (! DBase[i].Lat)  // <- AF: could crash here id DBase is NULL
 {
     return;
 }

 for (j=0; j<=DBase[i].Points; j++)
  {
  mapxx[j] = Lon_X_Convert(DBase[i].Lon[j]);
  mapyy[j] = Lat_Y_Convert(DBase[i].Lat[j]);
  } /* for j=0... */

} /* latlon_XY() */

/************************************************************************/

long Lat_Y_Convert(double Lat)
{

 return ((long)(.5 + latscalefactor * (latzero - Lat) * lat_y));

} /* Lat_Y_Convert() */

/************************************************************************/

long Lon_X_Convert(double Lon)
{
/* This is for map division at 180 degrees instead of 0 - needs to be user set
and derivation of DEM elevations needs to be adjusted accordingly.
 Lon = Lon > 180.0 ? Lon - 360.0: Lon;
*/

 return ((long)(.5 + lonscalefactor * (lonzero - Lon) * lon_x));

} /* Lon_X_Convert() */

/************************************************************************/

#ifdef ENABLE_STATISTICS
void makebox(struct RastPort *r, long x, long y, long w, long h)
{
 Move(r, x, y);
 Draw(r, x + w, y);
 Draw(r, x + w, y + h);
 Draw(r, x, y + h);
 Draw(r, x, y);
} /* makebox() */

/************************************************************************/

void checkbox(struct RastPort *r, long x, long y, long w, long h)
{
 Move(r, x + 1, y + 1);
 Draw(r, x + w - 1, y + h - 1);
 Move(r, x + 1, y + h - 1);
 Draw(r, x + w - 1, y + 1);
} /* checkbox() */
#endif /* ENABLE_STATISTICS */
/************************************************************************/

void outline(struct Window *win, long i, USHORT colr, struct clipbounds *cb)
{
 long j, p, sign = -1, pts, ptsm1, nextpt;
 double LonPt, LonNextPt;
 struct lineseg ls;

 if (! (DBase[i].Flags & 2))
  return;
 if (! DBase[i].Lat)
  {
  if (Load_Object(i, NULL) > 0)
   return;
  } /* if object not loaded */

 latlon_XY(i);
 SetAPen(win->RPort, colr);
 SetBPen(win->RPort, backpen);

 if (DBase[i].Pattern[0] == 'D' || DBase[i].Pattern[0] == 'T'
	|| DBase[i].Pattern[0] == 'L' || DBase[i].Pattern[0] == 'B')
  {
  if (DBase[i].Pattern[0] == 'D') 	{SetDrPt(win->RPort, 0xff00);}
  else if (DBase[i].Pattern[0] == 'T') {SetDrPt(win->RPort, 0x8888);}
  else if (DBase[i].Pattern[0] == 'B') {SetDrPt(win->RPort, 0xf24f);}
  for (j=0; j<DBase[i].LineWidth; j++, sign*=(-1))
   {
   /* <<<>>> Here's a quick hack to speed up vector drawing by not
   ** drawing lines until they span >1 pixel. */
   double ZX, ZY, ZX1, ZY1;
   p = (j / 2 + j % 2) * sign;
   ptsm1 = DBase[i].Points - 1;
   for (pts=1; pts<=DBase[i].Points; pts++)
    {
    nextpt = (pts == DBase[i].Points) ? pts: pts + 1;
    ZX  = (double)mapxx[pts];
    ZY  = (double)mapyy[pts];
    ZX1 = (double)mapxx[nextpt];
    ZY1 = (double)mapyy[nextpt];

    /* this 'if' line will eliminate drawing segments beginning on the ultimate
    ** point unless it is a degenerate line (pts == 1). */

    if((ZX != ZX1) || (ZY != ZY1) || (pts == 1) || (pts == ptsm1))
     {

     /* <<<>>> Probably should be some clever code here to wrap vectors
     ** around the "far" side of the planet, but we just discard any
     ** offending lines. So there. */
/* enable this when map split is at 180 degrees 
     LonPt = DBase[i].Lon[pts] > 180.0 ? DBase[i].Lon[pts] - 360.0: DBase[i].Lon[pts];
     LonNextPt = DBase[i].Lon[nextpt] > 180.0 ?
	 DBase[i].Lon[nextpt] - 360.0: DBase[i].Lon[pts + 1];
*/
     LonPt = DBase[i].Lon[pts];
     LonNextPt = DBase[i].Lon[pts + 1];

     if(fabs(LonPt - LonNextPt) < 170.0) /* Hmm. */
      {
      setlineseg(&ls, ZX + p, ZY, ZX1 + p, ZY1);
      ClipDraw(win, cb, &ls);
      if(p)
       {
       setlineseg(&ls, ZX, ZY + p, ZX1, ZY1 + p);
       ClipDraw(win, cb, &ls);
       } /* if */
      } /* if */
     } /* if ... all that */
    } /* for pts=1... */
   } /* for j=0... */
  SetDrPt(win->RPort, 0xffff);
  } /* if line */

/* map point objects */
 else if (DBase[i].Pattern[0] == 'P')
  {
  for (pts=1; pts<=DBase[i].Points; pts++)
   {
   if (mapxx[pts] >= cb->lowx && mapxx[pts] <= cb->highx
	&& mapyy[pts] >= cb->lowy && mapyy[pts] <= cb->highy)
    WritePixel(win->RPort, mapxx[pts], mapyy[pts]);
   } /* for pts=1... */
  } /* if point */
 else if (DBase[i].Pattern[0] == 'X')
  {
  for (j=0; j<DBase[i].LineWidth; j++, sign*=(-1))
   {
   p = (j / 2 + j % 2) * sign;
   for (pts=1; pts<=DBase[i].Points; pts++)
    {
    if (mapxx[pts] + p >= cb->lowx && mapxx[pts] + p <= cb->highx
	&& mapyy[pts] >= cb->lowy && mapyy[pts] <= cb->highy)
     WritePixel(win->RPort, mapxx[pts] + p, mapyy[pts]);
    if (p)
     {
     if (mapxx[pts] >= cb->lowx && mapxx[pts] <= cb->highx
	&& mapyy[pts] + p >= cb->lowy && mapyy[pts] + p <= cb->highy)
      WritePixel(win->RPort, mapxx[pts], mapyy[pts] + p);
     } /* if p */
    } /* for pts=1... */
   } /* for j=0... */
  } /* if cross */
 else if (DBase[i].Pattern[0] == 'C')
  {
  short radius = DBase[i].LineWidth / 2 + DBase[i].LineWidth % 2;

  for (pts=1; pts<=DBase[i].Points; pts++)
   {
   if (mapxx[pts] - radius >= cb->lowx && mapxx[pts] + radius <= cb->highx
	&& mapyy[pts] - radius >= cb->lowy && mapyy[pts] + radius <= cb->highy)
    DrawEllipse(win->RPort, mapxx[pts], mapyy[pts], radius, radius);
   } /* for pts=1... */
  } /* if circle */
 else if (DBase[i].Pattern[0] == 'R')
  {
  short XYplus, XYminus;

  XYminus = (DBase[i].LineWidth - 1) / 2;
  XYplus  = DBase[i].LineWidth / 2;
  for (pts=1; pts<=DBase[i].Points; pts++)
   {
   if (mapxx[pts] - XYminus >= cb->lowx && mapxx[pts] + XYplus <= cb->highx
	&& mapyy[pts] - XYminus >= cb->lowy && mapyy[pts] + XYplus <= cb->highy)
    RectFill(win->RPort, mapxx[pts] - XYminus, mapyy[pts] - XYminus,
	mapxx[pts] + XYplus, mapyy[pts] + XYplus);
   } /* for pts=1... */
  } /* if circle */

} /* outline() */

/************************************************************************/
/* Obsolete
short getval(struct Window *w,char *string,short start_x,short end_x,
          long number)
{
 short x, y, mod = 0, flag = 0;

 for ( ; ; ) {
  FetchEvent(w, &Event);
  if (Event.Class==CLOSEWINDOW) {
   flag=1;
   break;
  }
  if (Event.Class==IDCMP_MOUSEBUTTONS) {
   if (Event.Code==SELECTDOWN || Event.Code==SELECTUP) break;
  }
  else if (Event.Class==IDCMP_VANILLAKEY) {
   if (Event.Code==CARRIAGE_RET) break;
   else if (Event.Code==BACK_SP) {
    if (w->RPort->cp_x <= start_x) continue;
    string--;
    x=w->RPort->cp_x;
    y=w->RPort->cp_y;
    Move(w->RPort,x-8,y);
    Text(w->RPort," ",1);
    Move(w->RPort,x-8,y);
    if (w->RPort->cp_x <= start_x) mod=0;
   }
   else if (w->RPort->cp_x < end_x) {
    *string=Event.Code;
    Text(w->RPort,string,1);
    string++;
    mod=1;
   }
  }
 }
 if (number && mod) {
  *string='.';
  string++;
  *string='0';
  string++;
 }
 *string='\0';
 return(flag);
}
*/
/************************************************************************/

short FreeAllVectors(void)
{
 short i, ObjectSaved = 0;

 for (i=0; i<NoOfObjects; i++)
  {
  if (DBase[i].Flags & 1)
   {
   if (User_Message_Def((CONST_STRPTR)DBase[i].Name,
                        GetString( MSG_MAPSUPRT_VECTOROBJECTHASBEENMODIFIEDAVEITBEFORECLOSING ),  // "Vector object has been modified!\nSave it before closing?"
                        GetString( MSG_MAPSUPRT_SAVECANCEL ),                                     // "SAVE|CANCEL"
                        (CONST_STRPTR)"sc", 1))
    {
    if (! saveobject(i, NULL, DBase[i].Lon, DBase[i].Lat, DBase[i].Elev))
     ObjectSaved ++;
    } /* if save object */
   DBase[i].Flags &= (255 ^ 1);
   } /* if object modified */

  freevecarray(i);

  } /* for i=0... */

 return (ObjectSaved);

} /* FreeAllVectors() */

/**********************************************************************/

void freevecarray(short i)
{

 if (DBase[i].Lon) free_Memory(DBase[i].Lon, DBase[i].VecArraySize);
 if (DBase[i].Lat) free_Memory(DBase[i].Lat, DBase[i].VecArraySize);
 if (DBase[i].Elev) free_Memory(DBase[i].Elev, DBase[i].VecArraySize / 4);
 DBase[i].Lon = NULL;
 DBase[i].Lat = NULL;
 DBase[i].Elev = NULL;
 DBase[i].VecArraySize = 0L;

} /* freevecarray() */

/************************************************************************/

short allocvecarray(long obn, long pts, short newobj)
{
 ULONG newvecarraysize, newelevsize;
 double *newmptlon, *newmptlat;
 short *newmptelev;

 newvecarraysize = (pts + 1) * sizeof (double);
 newelevsize = (pts + 1) * sizeof (short);
 if (newvecarraysize <= DBase[obn].VecArraySize) return (1);
 newmptlon = (double *)get_Memory(newvecarraysize, MEMF_CLEAR);
 newmptlat = (double *)get_Memory(newvecarraysize, MEMF_CLEAR);
 newmptelev = (short *)get_Memory(newelevsize, MEMF_CLEAR);
 if (newmptlon && newmptlat && newmptelev)
  {
  if (! newobj)
   {
   memcpy(newmptlon, DBase[obn].Lon, DBase[obn].VecArraySize);
   memcpy(newmptlat, DBase[obn].Lat, DBase[obn].VecArraySize);
   memcpy(newmptelev, DBase[obn].Elev, DBase[obn].VecArraySize / 4);
   freevecarray(obn);
   } /* if not new object */
  DBase[obn].VecArraySize = newvecarraysize;
  DBase[obn].Lon = newmptlon;
  DBase[obn].Lat = newmptlat;
  DBase[obn].Elev = newmptelev;
  return (1);
 } /* if memory allocated for new arrays */

/* memory for new points not available */
 if (newmptlon) free_Memory(newmptlon, newvecarraysize);
 if (newmptlat) free_Memory(newmptlat, newvecarraysize);
 if (newmptelev) free_Memory(newmptelev, newelevsize);
 return (0);

} /* allocvecarray() */

/************************************************************************/

void SetView_Map(short camera)
{
 short i, abort;
 struct clipbounds cb;
 struct Vertex Vtx;

 if (camera)
  {
  MapGUI_Message(0, (char*)GetString( MSG_MAPSUPRT_0338SELECTCAMERAPOINT ));            // "\0338Select Camera Point"
  SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_SELECTCAMERAPOINT ), (UBYTE *)-1);  // "Select Camera Point"
  i = 1;
  } /* if camera point */
 else
  {
  MapGUI_Message(0, (char*)GetString( MSG_MAPSUPRT_0338SELECTFOCUSPOINT ));            // "\0338Select Focus Point"
  SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_SELECTFOCUSPOINT ), (UBYTE *)-1);  // "Select Focus Point"
  i = 4;
  } /* else focus point */

 setclipbounds(MapWind0, &cb);

 abort = ! MousePtSet(&Vtx, NULL, 0);

 MapIDCMP_Restore(MapWind0);
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_MAPVIEW ), (UBYTE *)-1);  // "Map View"
 if (abort) return;

 PAR_FIRST_MOTION(i + 1) = X_Lon_Convert((long)Vtx.X);
 PAR_FIRST_MOTION(i) = Y_Lat_Convert((long)Vtx.Y);
 
 ShowView_Map(&cb);

 if (EM_Win)
  {
  if (EMIA_Win)
   {
   if (camera) setcompass(0);
   else setcompass(1);
   if (IA_AutoDraw)
    {
    drawgridview();
    } /* if */
   else
    {
    constructview();
    DrawInterFresh(0);
    } /* else */
   } /* if IA window open */
  Update_EM_Item();
  } /* if Motion editor open */

} /* SetView_Map() */

/************************************************************************/

void ShowView_Map(struct clipbounds *cb)
{
 short startX, startY;
 double SunLon;

 SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);
 SetDrMd(MapWind0->RPort, COMPLEMENT);
 SetWrMsk(MapWind0->RPort, MAP_INTER_MASK);

/* camera and focus points */
 azimuthcompute(0, PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2),
	PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
 startX = Lon_X_Convert(PAR_FIRST_MOTION(2));
 startY = Lat_Y_Convert(PAR_FIRST_MOTION(1));
 ShowCenters_Map(cb, startX, startY, 1);
 MP->ptsdrawn |= 1;
 startX = Lon_X_Convert(PAR_FIRST_MOTION(5));
 startY = Lat_Y_Convert(PAR_FIRST_MOTION(4));
 ShowCenters_Map(cb, startX, startY, 2);

/* haze */
 startX = latscalefactor * PAR_FIRST_MOTION(20) * VERTPIX / mapscale;
 ShowCenters_Map(cb, startX, 0, 3);
 startX = latscalefactor * (PAR_FIRST_MOTION(20) + PAR_FIRST_MOTION(21))
		* VERTPIX / mapscale;
 ShowCenters_Map(cb, startX, 0, 4);
 MP->ptsdrawn |= 2;

/* sun */
 SunLon = PAR_FIRST_MOTION(16);
 while (SunLon > 360.0)
  SunLon -= 360.0;
 while (SunLon < 0.0)
  SunLon += 360.0;
 startX = Lon_X_Convert(SunLon);
 startY = Lat_Y_Convert(PAR_FIRST_MOTION(15));
 ShowCenters_Map(cb, startX, startY, 7);
 MP->ptsdrawn |= 4;

 SetDrMd(MapWind0->RPort, JAM1);
 SetWrMsk(MapWind0->RPort, 0x0f);

 ClearPointer(MapWind0);

} /* ShowView_Map() */

/************************************************************************/

short SetIAView_Map(struct IntuiMessage *Event)
{
 short i, modval = 0, abort = 0, startX, startY, oldX;
 struct clipbounds cb;

 if (! MP->ptsdrawn)
  return (0);

/* first identify the operation if there is supposed to be one */
 startX = oldX = Event->MouseX;
 startY = Event->MouseY;

 if (abs(startX - MP->camptx[0]) < 6 && abs(startY - MP->campty[0]) < 6)
  {
  modval = 1;
  PAR_FIRST_MOTION(2) = X_Lon_Convert((long)startX);
  PAR_FIRST_MOTION(1) = Y_Lat_Convert((long)startY);
  SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_INTERACTIVECAMERAPOINT ), (UBYTE *)-1);  // "Interactive Camera Point"
  i = 1;
  } /* if camera motion */
 else if (abs(startX - MP->focctrx) < 6 && abs(startY - MP->focctry) < 6)
  {
  modval = 2;
  PAR_FIRST_MOTION(5) = X_Lon_Convert((long)startX);
  PAR_FIRST_MOTION(4) = Y_Lat_Convert((long)startY);
  SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_INTERACTIVEFOCUSPOINT ), (UBYTE *)-1);  // "Interactive Focus Point"
  i = 4;
  } /* else if focus motion */
 else if (abs(startX - MP->sunctrx) < 6 && abs(startY - MP->sunctry) < 6)
  {
  modval = 7;
  PAR_FIRST_MOTION(16) = X_Lon_Convert((long)startX);
  PAR_FIRST_MOTION(15) = Y_Lat_Convert((long)startY);
  SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_INTERACTIVESUNPOSITION ), (UBYTE *)-1);  // "Interactive Sun Position"
  }
 else
  {
  short ptrad;
/*  float ptslope;*/

  ptrad = sqrt(((double)startX - MP->camptx[0]) * (startX - MP->camptx[0]) +
	(startY - MP->campty[0]) * (startY - MP->campty[0]));
  if (abs(ptrad - MP->hazerad[0]) < 5)
   {
   modval = 3;
   startX = ptrad;
   SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_INTERACTIVESTARTHAZE ), (UBYTE *)-1);  // "Interactive Start Haze"
   } /* if modify haze start */
  else if (abs(ptrad - MP->hazerad[1]) < 5)
   {
   modval = 4;
   startX = ptrad;
   SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_INTERACTIVEFULLHAZE ), (UBYTE *)-1);  // "Interactive Full Haze"
   } /* if modify haze end (range) */
/*
  else
   {
   if (startX != MP->camptx[0])
	ptslope = ((float)startY - MP->campty[0]) / (startX - MP->camptx[0]);
   if (abs(ptslope - MP->viewlineslope[0]) < .1)
    {
    modval = 5;
    SetWindowTitles(MapWind0, (STRPTR) "Interactive View Arc", (UBYTE *)-1);
    }
   else if (abs(ptslope - MP->viewlineslope[1]) < .1)
    {
    modval = 6;
    SetWindowTitles(MapWind0, (STRPTR) "Interactive View Arc", (UBYTE *)-1);
    }
   else return (0);
   } // else may be view arc motion
*/
  else return (0);
  } /* else may be haze or view arc motion */ 
      
 ModifyIDCMP(MapWind0, IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);

 setclipbounds(MapWind0, &cb);
 
 SetDrMd(MapWind0->RPort, COMPLEMENT);
 SetWrMsk(MapWind0->RPort, MAP_INTER_MASK);

 if (EMIA_Win)
  {
  IA->recompute = 1;
  if (modval == 1) setcompass(0);
  else if (modval == 2) setcompass(1);
  ShowCenters_Map(&cb, startX, startY, modval);
  constructview();
  DrawInterFresh(0);
  Update_EM_Item();
  } /* if IA window open */
 else
  {
  if (modval == 2)
   azimuthcompute(1, PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5),
	PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2));
  else if (modval == 1)
   azimuthcompute(0, PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2),
	PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
  ShowCenters_Map(&cb, startX, startY, modval);
  if (EM_Win)
   {
   Update_EM_Item();
   } /* if Motion editor window open */
  } /* if IA window not open compute view azimuth */
 startX = oldX;

 while (! abort)
  {
  FetchEvent(MapWind0, Event);

  switch (Event->Class)
   {
   case IDCMP_MOUSEBUTTONS:
    {
    switch (Event->Code)
     {
     case SELECTUP:
      {
      abort = 1;
      break;
      } /* SELECTUP */
     } /* switch Event->Code */
    break;
    } /* IDCMP_MOUSEBUTTONS */
   case IDCMP_INTUITICKS:
    {
    if (modval)
     {
     if (Event->MouseX == startX && Event->MouseY == startY) break;
     startX = oldX = Event->MouseX;
     startY = Event->MouseY;
     switch (modval)
      {
      case 1:
      case 2:
       {
       PAR_FIRST_MOTION(i + 1) = X_Lon_Convert((long)startX);
       PAR_FIRST_MOTION(i) = Y_Lat_Convert((long)startY);
       break;
       } /* camera-focus */
      case 3:
       {
       double oldHazeStart;

       oldHazeStart = PAR_FIRST_MOTION(20);
       startX = sqrt(((double)startX - MP->hazectrx[0]) * (startX - MP->hazectrx[0]) +
		(startY - MP->hazectry[0]) * (startY - MP->hazectry[0]));
       PAR_FIRST_MOTION(20) = startX * mapscale / (latscalefactor * VERTPIX);
       PAR_FIRST_MOTION(21) += oldHazeStart - PAR_FIRST_MOTION(20);
       if (PAR_FIRST_MOTION(21) < 0.001)
        {
        PAR_FIRST_MOTION(21) = .001;
        startX = MP->hazerad[1];
	} /* if trying to move haze start outside haze end circle */
       break;
       } /* haze start */
      case 4:
       {
       startX = sqrt(((double)startX - MP->hazectrx[1]) * (startX - MP->hazectrx[1]) +
		(startY - MP->hazectry[1]) * (startY - MP->hazectry[1]));
       PAR_FIRST_MOTION(21) = (startX * mapscale / (latscalefactor * VERTPIX))
			 - PAR_FIRST_MOTION(20);
       if (PAR_FIRST_MOTION(21) < 0.001)
        {
        PAR_FIRST_MOTION(21) = .001;
        startX = MP->hazerad[0];
	} /* if trying to move haze start outside haze end circle */
       break;
       } /* haze range */
      case 7:
       {
       PAR_FIRST_MOTION(16) = X_Lon_Convert((long)startX);
       PAR_FIRST_MOTION(15) = Y_Lat_Convert((long)startY);
       break;
       } /* sun */
      } /* switch modval */
     if (EMIA_Win)
      {
      IA->recompute = 1;
      switch (modval)
       {
       case 1:
       	{
       	setcompass(0);
       	break;
       	}
       case 2:
       	{
       	setcompass(1);
       	break;
       	} 
       } /* switch */
      ShowCenters_Map(&cb, startX, startY, modval);
      if (modval != 3)
       ShowCenters_Map(&cb, MP->hazerad[0], 0, 3);
      if (modval != 4)
       ShowCenters_Map(&cb, MP->hazerad[1], 0, 4);
      constructview();
      if (GridBounds)
       {
       drawgridpts(1);
       if (modval == 2)
        {
        findfocprof();
	}/* if */
       drawgridpts(0);
       } /* if */
      if(BoxBounds)
       {
       drawquick(1, 1, 1, 0);
       computequick();
       drawquick(7, 6, 3, 0);
       } /* if */ 
      if (ProfileBounds && ! GridBounds)
       {
       drawfocprof(1);
       if (modval == 2)
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
      } /* if IA window open */
     else
      {
      switch(modval)
       {
       case 2:
       	{
       	azimuthcompute(1, PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5),
		 	 PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2));
 	break;
 	} /* 2 */
       case 1:
       	{
       	azimuthcompute(0, PAR_FIRST_MOTION(1), PAR_FIRST_MOTION(2),
		 	 PAR_FIRST_MOTION(4), PAR_FIRST_MOTION(5));
 	break;
 	} /* 1 */
       } /* switch */
      ShowCenters_Map(&cb, startX, startY, modval);
      if (modval != 3)
       ShowCenters_Map(&cb, MP->hazerad[0], 0, 3);
      if (modval != 4)
       ShowCenters_Map(&cb, MP->hazerad[1], 0, 4);
      if (EM_Win)
       {
       Update_EM_Item();
       } /* if Motion editor window open */
      } /* else IA window not open. compute view azimuth */
     } /* if modval */
    startX = oldX;
    break;
    } /* IDCMP_INTUITICKS */
   } /* switch */
  } /* while ! abort */

 SetDrMd(MapWind0->RPort, JAM1);
 SetWrMsk(MapWind0->RPort, 0x0f);

 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, GetString( MSG_MAPSUPRT_MAPVIEW ), (UBYTE *)-1);  // "Map View"

 if (EMIA_Win && IA_AutoDraw && modval != 7)
  {
  drawgridview();
  } /* if */

 return (1);

} /* SetIAView_Map() */

/************************************************************************/

void ShowCenters_Map(struct clipbounds *cb, short startX, short startY,
	short modval)
{
 short x, y;
 struct lineseg ls;

/* draw over old positions */
 if (MP->ptsdrawn)
  {
  switch (modval)
   {
   case 1:
   case 2:
    {
    setlineseg(&ls, (double)MP->focptx[0], (double)MP->focpty[0],
	 (double)MP->focptx[1], (double)MP->focpty[1]);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->focptx[2], (double)MP->focpty[2],
	 (double)MP->focptx[3], (double)MP->focpty[3]);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->camptx[1], (double)MP->campty[1]);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0] - 5, (double)MP->campty[0] - 5,
	 (double)MP->camptx[0] + 5, (double)MP->campty[0] - 5);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0] + 5, (double)MP->campty[0] - 5,
	 (double)MP->camptx[0] + 5, (double)MP->campty[0] + 5);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0] + 5, (double)MP->campty[0] + 5,
	 (double)MP->camptx[0] - 5, (double)MP->campty[0] + 5);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0] - 5, (double)MP->campty[0] + 5,
	 (double)MP->camptx[0] - 5, (double)MP->campty[0] - 5);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[0], (double)MP->viewlinepty[0]);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[1], (double)MP->viewlinepty[1]);
    ClipDraw(MapWind0, cb, &ls);
    break; 
    } /* if camera or focus motion */
   case 3: 
    {
    if (MP->ptsdrawn & 2)
     DrawHazeRPort(MapWind0->RPort, MP->hazectrx[0], MP->hazectry[0], MP->hazerad[0], cb);
    break;
    }
   case 4:
    {
    if (MP->ptsdrawn & 2)
     DrawHazeRPort(MapWind0->RPort, MP->hazectrx[1], MP->hazectry[1], MP->hazerad[1], cb);
    break;
    } /* if haze motion */
   case 5:
   case 6:
    {
    setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[0], (double)MP->viewlinepty[0]);
    ClipDraw(MapWind0, cb, &ls);
    setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[1], (double)MP->viewlinepty[1]);
    ClipDraw(MapWind0, cb, &ls);
    break;
    }
   case 7:
    {
    if (MP->ptsdrawn & 4)
     DrawSun(MapWind0, MP->sunctrx, MP->sunctry, 10, cb);
    break;
    }
   } /* switch */
  } /* if pts already drawn */

/* compute new points */
 x = 96.0 * sin(azimuth) - ROUNDING_KLUDGE;
 y = -96.0 * cos(azimuth) - ROUNDING_KLUDGE;

 if (modval == 1)
  {
  MP->camptx[0] = startX;
  MP->campty[0] = startY;
  MP->camptx[1] = startX + x / 8;
  MP->campty[1] = startY + y / 8;
  MP->focptx[0] = MP->focctrx + x / 16;
  MP->focpty[0] = MP->focctry + y / 16;
  MP->focptx[1] = MP->focctrx - x / 8;
  MP->focpty[1] = MP->focctry - y / 8;
  MP->focptx[2] = MP->focctrx + y / 16;
  MP->focpty[2] = MP->focctry - x / 16;
  MP->focptx[3] = MP->focctrx - y / 16;
  MP->focpty[3] = MP->focctry + x / 16;
  } /* if camera motion */
 else if (modval == 2)
  {
  MP->camptx[1] = MP->camptx[0] + x / 8;
  MP->campty[1] = MP->campty[0] + y / 8;
  MP->focctrx = startX;
  MP->focctry = startY;
  MP->focptx[0] = MP->focctrx + x / 16;
  MP->focpty[0] = MP->focctry + y / 16;
  MP->focptx[1] = MP->focctrx - x / 8;
  MP->focpty[1] = MP->focctry - y / 8;
  MP->focptx[2] = MP->focctrx + y / 16;
  MP->focpty[2] = MP->focctry - x / 16;
  MP->focptx[3] = MP->focctrx - y / 16;
  MP->focpty[3] = MP->focctry + x / 16;
  } /* else if focus motion */
 else if (modval == 3)
  {
  MP->hazerad[0] = startX;
  MP->hazectrx[0] = MP->camptx[0];
  MP->hazectry[0] = MP->campty[0];
  } /* if haze motion */
 else if (modval == 4)
  {
  MP->hazerad[1] = startX;
  MP->hazectrx[1] = MP->camptx[0];
  MP->hazectry[1] = MP->campty[0];
  } /* if haze motion */
 else if (modval == 7)
  {
  MP->sunctrx = startX;
  MP->sunctry = startY;
  } /* if sun motion */

 if (modval == 5 || modval == 6 || modval == 1 || modval == 2)
  {
  short h, focusdist;

  focusdist = sqrt(((double)MP->focctrx - MP->camptx[0]) * (MP->focctrx - MP->camptx[0]) +
		(MP->focctry - MP->campty[0]) * (MP->focctry - MP->campty[0]));

  h = focusdist * PAR_FIRST_MOTION(10) * tan((PAR_FIRST_MOTION(11) * .5)
	 * PiOver180);
  h *= ((float)settings.scrnwidth / 640.0);
  MP->viewlineptx[0] = MP->focctrx + (y * h) / 96;
  MP->viewlinepty[0] = MP->focctry - (x * h) / 96;
  MP->viewlineptx[1] = MP->focctrx - (y * h) / 96;
  MP->viewlinepty[1] = MP->focctry + (x * h) / 96;
/*
  if (modval == 5)
   {
   double angle, anglechg;

   angle = findangle2((double)startX - MP->camptx[0], - (double)(startY - MP->campty[0]));
   anglechg = angle - MP->viewlineaz[0];
   if (angle > azimuth)
    {
    PAR_FIRST_MOTION(11) += (PiUnder180 * 2.0 * anglechg);
    MP->viewlineaz[0] += anglechg;
    MP->viewlineaz[1] -= anglechg;
    }
   else
    {
    PAR_FIRST_MOTION(11) -= (PiUnder180 * 2.0 * anglechg);
    MP->viewlineaz[0] -= anglechg;
    MP->viewlineaz[1] += anglechg;
    }
   } // if view arc motion
  else if (modval == 6)
   {
   double angle, anglechg;

   angle = findangle2((double)startX - MP->camptx[0], - (double)(startY - MP->campty[0]));
   anglechg = angle - MP->viewlineaz[1];
   if (angle > azimuth)
    {
    PAR_FIRST_MOTION(11) -= (PiUnder180 * 2.0 * anglechg);
    MP->viewlineaz[0] -= anglechg;
    MP->viewlineaz[1] += anglechg;
    }
   else
    {
    PAR_FIRST_MOTION(11) += (PiUnder180 * 2.0 * anglechg);
    MP->viewlineaz[0] += anglechg;
    MP->viewlineaz[1] -= anglechg;
    }
   } // if view arc motion
*/
  MP->viewlineslope[0] = ((float)MP->viewlinepty[0] - MP->campty[0])
			 / (MP->viewlineptx[0] - MP->camptx[0]);
  MP->viewlineslope[1] = ((float)MP->viewlinepty[1] - MP->campty[0])
			 / (MP->viewlineptx[1] - MP->camptx[0]);
  } /* if view arc, camera or focus motion */

/* redraw new positions */
 switch (modval)
  {
  case 1:
  case 2:
   {
   setlineseg(&ls, (double)MP->focptx[0], (double)MP->focpty[0],
	 (double)MP->focptx[1], (double)MP->focpty[1]);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->focptx[2], (double)MP->focpty[2],
	 (double)MP->focptx[3], (double)MP->focpty[3]);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->camptx[1], (double)MP->campty[1]);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0] - 5, (double)MP->campty[0] - 5,
	 (double)MP->camptx[0] + 5, (double)MP->campty[0] - 5);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0] + 5, (double)MP->campty[0] - 5,
	 (double)MP->camptx[0] + 5, (double)MP->campty[0] + 5);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0] + 5, (double)MP->campty[0] + 5,
	 (double)MP->camptx[0] - 5, (double)MP->campty[0] + 5);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0] - 5, (double)MP->campty[0] + 5,
	 (double)MP->camptx[0] - 5, (double)MP->campty[0] - 5);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[0], (double)MP->viewlinepty[0]);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[1], (double)MP->viewlinepty[1]);
   ClipDraw(MapWind0, cb, &ls);
   break;
   } /* if camera or focus motion */
  case 3:
   { 
   DrawHazeRPort(MapWind0->RPort, MP->hazectrx[0], MP->hazectry[0], MP->hazerad[0], cb);
   break;
   }
  case 4:
   {
   DrawHazeRPort(MapWind0->RPort, MP->hazectrx[1], MP->hazectry[1], MP->hazerad[1], cb);
   break;
   } /* else if haze motion */
  case 5:
  case 6:
   {
   setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[0], (double)MP->viewlinepty[0]);
   ClipDraw(MapWind0, cb, &ls);
   setlineseg(&ls, (double)MP->camptx[0], (double)MP->campty[0],
	 (double)MP->viewlineptx[1], (double)MP->viewlinepty[1]);
   ClipDraw(MapWind0, cb, &ls);
   break;
   }
  case 7:
   {
   DrawSun(MapWind0, MP->sunctrx, MP->sunctry, 10, cb);
   break;
   }
  } /* switch */

} /* ShowCenters_Map() */

/************************************************************************/
/* Not called from ShowView_Map anymore 'cause it took too long to draw
**  when Interactive was open and because you can import a motion path as
**  a vector now. It also didn't look good at small map scales and didn't
**  undraw itself when key frame positions changed. */

void ShowPaths_Map(struct clipbounds *cb)
{
 short i, j, k;
 double X[2], Y[2], V[2];
 struct lineseg ls;

 SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);
 SetDrMd(MapWind0->RPort, COMPLEMENT);
 SetWrMsk(MapWind0->RPort, MAP_INTER_MASK);

 if (BuildKeyTable())
  {
  if (KT[1].Key || KT[2].Key)
   {
   for (j=0,k=1; j<2; j++,k++)
    {
    if (KT[k].Key)
     V[j] = KT[k].Val[0][0];
    else
     V[j] = PAR_FIRST_MOTION(k);
    } /* for j=0... */
   X[0] = Lon_X_Convert(V[1]);
   Y[0] = Lat_Y_Convert(V[0]);
   for (i=1; i<=KT_MaxFrames; i++)
    {
    if (KT[2].Key)
     X[1] = Lon_X_Convert(KT[2].Val[0][i]);
    if (KT[1].Key)
     Y[1] = Lat_Y_Convert(KT[1].Val[0][i]);
    setlineseg(&ls, X[0], Y[0], X[1], Y[1]);
    ClipDraw(MapWind0, cb, &ls);
    X[0] = X[1];
    Y[0] = Y[1];
    } /* for i=1... */
   } /* if camera point key frames */

  if (KT[4].Key || KT[5].Key)
   {
   for (j=0,k=4; j<2; j++,k++)
    {
    if (KT[k].Key)
     V[j] = KT[k].Val[0][1];
    else
     V[j] = PAR_FIRST_MOTION(k);
    } /* for j=0... */
   X[1] = X[0] = Lon_X_Convert(V[1]);
   Y[1] = Y[0] = Lat_Y_Convert(V[0]);
   for (i=1; i<=KT_MaxFrames; i++)
    {
    if (KT[5].Key)
     X[1] = Lon_X_Convert(KT[5].Val[0][i]);
    if (KT[4].Key)
     Y[1] = Lat_Y_Convert(KT[4].Val[0][i]);
    setlineseg(&ls, X[0], Y[0], X[1], Y[1]);
    ClipDraw(MapWind0, cb, &ls);
    X[0] = X[1];
    Y[0] = Y[1];
    } /* for i=1... */
   } /* if focus point key frames */
 
  FreeKeyTable();
  } /* if key table */

 SetDrMd(MapWind0->RPort, JAM1);
 SetWrMsk(MapWind0->RPort, 0x0f);

 ClearPointer(MapWind0);

} /* ShowPaths_Map() */

/************************************************************************/

void MakeColorMap(void)
{
 char CmapPath[256], CmapName[32];
 UBYTE *bitmap[3], Red, Grn, Blu;
 short a, b, scrnrow, i, j, topomap, wide, high, InBounds, outj, outk,
	el1, el2, el3 , LWidth, UWidth;
 long *scrnrowzip, bitmapsize, xx[2], yy[2], x, y, zip, zap, zop, Lr, Lc;
 double latscale, lonscale, m, color, MBossScale, gradfact, TestColor,
	ElevRng, ElevDiff, TestColor2;

 if (strcmp(DBase[OBN].Special, "TOP"))
  {
  User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
               GetString( MSG_MAPSUPRT_SELECTEDOBJECTMUSTBEATOPODEMEECLASSFIELDINDATABASE ),  // "Selected object must be a Topo DEM!\nSee Class field in Database Editor.\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                  // "OK"
               (CONST_STRPTR)"o");
  return;
  } /* object not topo map */

RepeatCheck:
 
 for (topomap=0; topomap<topomaps; topomap++)
  {
  if (TopoOBN[topomap] == OBN) break;
  } /* find correct topo map */

 if (topomap == topomaps)
  {
  if (User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
                       GetString( MSG_MAPSUPRT_SELECTEDMAPISNOTCURRENTLYLOADEDOYOUWISHTOLOADTOPOM ),  // "Selected map is not currently loaded!\nDo you wish to load topo maps?",
                       GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|CANCEL"
                       (CONST_STRPTR)"oc", 1))
   {
   loadtopo();
   if (topoload) goto RepeatCheck;
   else return;
   } /* if re-load topos */
  else return;
  } /* if correct topo not found */

/* allocate bitmaps */
 wide = mapelmap[topomap].columns - 1;
 high = mapelmap[topomap].rows;
 bitmapsize = (high + 1) * (wide + 1);
 bitmap[0] = (UBYTE *)get_Memory(bitmapsize, MEMF_CLEAR);
 bitmap[1] = (UBYTE *)get_Memory(bitmapsize, MEMF_CLEAR);
 bitmap[2] = (UBYTE *)get_Memory(bitmapsize, MEMF_CLEAR);
 scrnrowzip = (long *)get_Memory((high + 1) * 4, MEMF_CLEAR);
 if (! bitmap[0] || ! bitmap[1] || ! bitmap[2] || ! scrnrowzip)
  {
  User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                               // "Map View: Color Map"
               GetString( MSG_MAPSUPRT_OUTOFMEMORYCREATINGBITMAPSPERATIONTERMINATED ),  // "Out of memory creating bitmaps!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                            // "OK"
               (CONST_STRPTR)"o");
  goto Cleanup;
  } /* if out of memory */

/* compute parameters */
 latscale = wide / (DBase[OBN].Lat[3] - DBase[OBN].Lat[2]);
 lonscale = high / (DBase[OBN].Lon[2] - DBase[OBN].Lon[1]);
 for (i=0; i<high+1; i++)
  {
  scrnrowzip[i] = i * (wide + 1);
  } /* set scrnrowzip values */

/* topo drawing */
 if (User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                    // "Map View: Color Map"
                      GetString( MSG_MAPSUPRT_INCLUDEDEMELEVATIONDATAINCOLORMAP ),  // "Include DEM elevation data in Color Map?"
                      GetString( MSG_GLOBAL_YESNO ),                              // "Yes|No"
                      (CONST_STRPTR)"yn", 1))
  {
  SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);

  MBossScale = MaxElevDiff * (mapelmap[topomap].steplat / .0008333333);
  ElevRng = (MapHighEl - MapLowEl) / 255.0;

  for (Lr=0; Lr<=mapelmap[topomap].rows; Lr++)
   {
   for (Lc=0; Lc<mapelmap[topomap].columns; Lc++)
    {
    zip = (Lr * mapelmap[topomap].columns) + Lc;
    
    el1 = *(mapelmap[topomap].map + zip) * mapelmap[topomap].elscale * 1000.0;

    switch (ContInt)
     {
     case 0:
      {    
      color = (el1 - MapLowEl) / ElevRng;
      break; 
      } /* single gradient */
     case 1:
      {
      color = el1 % ContInterval;
      if (color < 0.0) color += ContInterval;
      color =  255.0 * color / (float)ContInterval;
      break;
      } /* repeating gradients */
     case 2:
      {
      if (el1 < settings.surfel[1])
       {
       gradfact = (el1 - (double)settings.surfel[0])
		 / (settings.surfel[1] - settings.surfel[0]);
       } /* low gradient */
      else if (el1 >= settings.surfel[1] && el1 < settings.surfel[2])
       {
       gradfact = (el1 - (double)settings.surfel[1])
		 / (settings.surfel[2] - settings.surfel[1]);
       } /* else if middle gradient */
      else
       {
       gradfact = (el1 - (double)settings.surfel[2])
		 / (settings.surfel[3] - settings.surfel[2]);
       } /* else upper gradient */
      if (gradfact > 1.0) gradfact = 1.0;
      else if (gradfact < 0.0) gradfact = 0.0;
      color = 255.0 * gradfact;
      break;
      } /* surface elevations */
     case 3:
      {
      TestColor = (el1 - MapLowEl) / ElevRng;
      if (! Lr)
       {
       if (Lc == mapelmap[topomap].columns - 1)
        {
        zap = zip;
	} /* if last column */
       else
        {
        zap = zip + 1;
	} /* else not last column */
       } /* if first row */
      else
       {
       if (Lc == mapelmap[topomap].columns - 1)
        {
        zap = zip - mapelmap[topomap].columns;
	} /* if last column */
       else
        {
        zap = zip - mapelmap[topomap].columns + 1;
	} /* else not last column */
       } /* else not first row */
      el2 = *(mapelmap[topomap].map + zap) * mapelmap[topomap].elscale * 1000.0;
      ElevDiff = el2 - el1;
      if(ElevDiff > MBossScale) ElevDiff = MBossScale;
      else if (ElevDiff < -MBossScale) ElevDiff = -MBossScale;
      TestColor2 = 128.0 - 127.0 * ((float)ElevDiff / MBossScale);
      color = (TestColor + TestColor2) / 2.0;
      break;
      } /* embossed */
     case 4:
      {
      if (! Lr)
       {
       if (Lc == mapelmap[topomap].columns - 1)
        {
        zap = zip;
	} /* if last column */
       else
        {
        zap = zip + 1;
	} /* else not last column */
       } /* if first row */
      else
       {
       if (Lc == mapelmap[topomap].columns - 1)
        {
        zap = zip - mapelmap[topomap].columns;
	} /* if last column */
       else
        {
        zap = zip - mapelmap[topomap].columns + 1;
	} /* else not last column */
       } /* else not first row */
      el2 = *(mapelmap[topomap].map + zap) * mapelmap[topomap].elscale * 1000.0;
      ElevDiff = el2 - el1;
      if(ElevDiff > MBossScale) ElevDiff = MBossScale;
      else if(ElevDiff < -MBossScale) ElevDiff = -MBossScale;
      color = fabs(255.0 * ((float)ElevDiff / MBossScale));
      break;
      } /* slope */
     case 5:
      {
      if (! Lr)
       {
       zap = zip;
       } /* if first row */
      else
       {
       zap = zip - mapelmap[topomap].columns;
       } /* else not first row */
      if (Lc == mapelmap[topomap].columns - 1)
       {
       zop = zip;
       } /* if last column */
      else
       {
       zop = zip + 1;
       } /* else not last column */

      el1 = el1 / ContInterval;
      el2 = (zip == zap) ? el1: (*(mapelmap[topomap].map + zap)
	 * mapelmap[topomap].elscale * 1000.0) / ContInterval;
      el3 = (zip == zop) ? el1: (*(mapelmap[topomap].map + zop)
	 * mapelmap[topomap].elscale * 1000.0) / ContInterval;
     
      color = (el1 == el2 && el1 == el3) ? 0: 255;
      break;
      } /* contour */

     } /* switch map style */

     *(bitmap[0] + zip) = (UBYTE)color;
     *(bitmap[1] + zip) = (UBYTE)color;
     *(bitmap[2] + zip) = (UBYTE)color;

    }
   }
  } /* if elevation output */

/* object loop */

 SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);

 for (i=0; i<NoOfObjects; i++)
  {
  if (DBase[i].Special[0] != 'V' || ! DBase[i].Lat 
	|| ! (DBase[i].Flags & 2)) continue;
  InBounds = 0;
  for (j=0; j<=DBase[i].Points; j++)
   {
   mapxx[j] = latscale * (DBase[i].Lat[j] - mapelmap[topomap].lolat);
   mapyy[j] = lonscale * (mapelmap[topomap].lolong - DBase[i].Lon[j]);
   if (mapxx[j] >= 0 && mapxx[j] <= wide) InBounds = 1;
   else if (mapyy[j] >= 0 && mapyy[j] <=high) InBounds = 1;
   } /* for j=0... */
  if (! InBounds) continue;

  Red = (UBYTE)DBase[i].Red;
  Grn = (UBYTE)DBase[i].Grn;
  Blu = (UBYTE)DBase[i].Blu;

/* drawing loop */

  LWidth = DBase[i].LineWidth / 2;
  UWidth = DBase[i].LineWidth / 2 + DBase[i].LineWidth % 2;

/* if line draw style */

  if (DBase[i].Pattern[0] == 'D' || DBase[i].Pattern[0] == 'T'
	|| DBase[i].Pattern[0] == 'L' || DBase[i].Pattern[0] == 'B')
   {
   for (j=1; j<DBase[i].Points; j++)
    {
    outj = outk = 0;
    xx[0] = mapxx[j];
    xx[1] = mapxx[j + 1];
    yy[0] = mapyy[j];
    yy[1] = mapyy[j + 1];

    if (xx[0] < 0) outj += 1;
    else if (xx[0] > wide) outj += 1;
    if (yy[0] < 0) outk += 1;
    else if (yy[0] > high) outk += 1;
    if (xx[1] < 0) outj += 1;
    else if (xx[1] > wide) outj += 1;
    if (yy[1] < 0) outk += 1;
    else if (yy[1] > high) outk += 1;

    if (outj < 2 && outk < 2)
     {
     if (yy[1]<yy[0])
      {
      swmem(&xx[0],&xx[1],4);
      swmem(&yy[0],&yy[1],4);
      }
     if (yy[1] == yy[0]) m = 0.0;
     else m = ((float)xx[1] - xx[0]) / (yy[1] - yy[0]);
     for (a=-LWidth; a<UWidth; a++)
      {
      for (b=-LWidth; b<UWidth; b++)
       {
       scrnrow = b + yy[0];
       for (y=0; y<=yy[1]-yy[0]; y++, scrnrow++)
        {
        if (scrnrow > high) break;
        if (scrnrow < 0) continue;
        x = m * y + xx[0] + a;
        if (x < 0 || x > wide) continue;
        zip = scrnrowzip[scrnrow] + x;
        *(bitmap[0] + zip) = Red;
        *(bitmap[1] + zip) = Grn;
        *(bitmap[2] + zip) = Blu;
        } /* for y=0... */
       } /* for b=... */
      } /* for a=... */

     if (xx[1] < xx[0])
      {
      swmem(&xx[0], &xx[1], 4);
      swmem(&yy[0], &yy[1], 4);
      }
     if (xx[1] == xx[0]) m = 0.0;
     else m = ((float)yy[1] - yy[0]) / (xx[1] - xx[0]);
     for (a=-LWidth; a<UWidth; a++)
      {
      for (b=-LWidth; b<UWidth; b++)
       {
       scrnrow = b + xx[0];
       for (x=0; x<=xx[1]-xx[0]; x++, scrnrow++)
        {
        if (scrnrow > wide) break;
        if (scrnrow < 0) continue;
        y = m * x + yy[0] + a;
        if (y < 0 || y > high) continue;
        zip = scrnrowzip[y] + scrnrow;
        *(bitmap[0] + zip) = Red;
        *(bitmap[1] + zip) = Grn;
        *(bitmap[2] + zip) = Blu;
        } /* for x=0... */
       } /* for b=... */
      } /* for a=... */

     } /* if within bounds */

    } /* for j=0... line segments */

   } /* if line pattern */

  else
   {
   for (j=1; j<=DBase[i].Points; j++)
    {
    outj = outk = 0;
    xx[0] = mapxx[j];
    yy[0] = mapyy[j];

    if (xx[0] < 0) outj += 1;
    else if (xx[0] > wide) outj += 1;
    if (yy[0] < 0) outk += 1;
    else if (yy[0] > high) outk += 1;

    if (outj < 1 && outk < 1)
     {
     for (a=-LWidth; a<UWidth; a++)
      {
      x = xx[0] + a;
      if (x > wide) break;
      if (x < 0) continue;
      for (b=-LWidth; b<UWidth; b++)
       {
       scrnrow = b + yy[0];
       if (scrnrow > high) break;
       if (scrnrow < 0) continue;
       zip = scrnrowzip[scrnrow] + x;
       *(bitmap[0] + zip) = Red;
       *(bitmap[1] + zip) = Grn;
       *(bitmap[2] + zip) = Blu;
       } /* for b=... */
      } /* for a=... */
     } /* if in bounds */
    } /* for j=1... */
   } /* else point pattern */

  } /* for i=0... objects */

 ClearPointer(MapWind0);

/* select drawer and filename - default to cmappath directory, OBN name */
 strcpy(CmapPath, colormappath);
 strcpy(CmapName, DBase[OBN].Name);
 while (CmapName[strlen(CmapName) - 1] == ' ')
  CmapName[strlen(CmapName) - 1] = '\0';
 if (! getfilename(1, (char*)GetString( MSG_MAPSUPRT_SAVECOLORMAPAS ), CmapPath, CmapName))  // "Save Color Map As:"
  {
  goto Cleanup;
  } /* if cancel selected */
 strmfp(ILBMname, CmapPath, CmapName);

/* save the bitmaps */
 saveILBM(24, 0, NULL, bitmap, scrnrowzip, 0, 1, 1, wide + 1, high + 1);

Cleanup:
 if (bitmap[0]) free_Memory(bitmap[0], bitmapsize);
 if (bitmap[1]) free_Memory(bitmap[1], bitmapsize);
 if (bitmap[2]) free_Memory(bitmap[2], bitmapsize);
 if (scrnrowzip) free_Memory(scrnrowzip, (high + 1) * 4);

} /* MakeColorMap() */

/************************************************************************/

void DrawHaze(struct Window *Win, int X, int Y, int Rad, struct clipbounds *cb)
{

 DrawHazeRPort(Win->RPort, X, Y, Rad, cb);

} /* DrawHaze() */

/************************************************************************/

void DrawHazeRPort(struct RastPort *Rast, int X, int Y, int Rad, struct clipbounds *cb)
{
int ASize, BSize, CSize;
struct lineseg ls;

ASize = (int)((float)Rad * .38268343F);
BSize = (int)((float)Rad * .70710678F);
CSize = (int)((float)Rad * .92387953F);

/* 12 O'Clock to 3 O'Clock */
setlineseg(&ls, (double)(X), (double)(Y - Rad), (double)(X + ASize), (double)(Y - CSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + ASize), (double)(Y - CSize), (double)(X + BSize), (double)(Y - BSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + BSize), (double)(Y - BSize), (double)(X + CSize), (double)(Y - ASize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + CSize), (double)(Y - ASize), (double)(X + Rad), (double)(Y));
ClipDrawRPort(Rast, cb, &ls);


/* 3 O'Clock to 6 O'Clock */
setlineseg(&ls, (double)(X + Rad), (double)(Y), (double)(X + CSize), (double)(Y + ASize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + CSize), (double)(Y + ASize), (double)(X + BSize), (double)(Y + BSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + BSize), (double)(Y + BSize), (double)(X + ASize), (double)(Y + CSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X + ASize), (double)(Y + CSize), (double)(X), (double)(Y + Rad));
ClipDrawRPort(Rast, cb, &ls);


/* 6 O'Clock to 9 O'Clock */
setlineseg(&ls, (double)(X), (double)(Y + Rad), (double)(X - ASize), (double)(Y + CSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - ASize), (double)(Y + CSize), (double)(X - BSize), (double)(Y + BSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - BSize), (double)(Y + BSize), (double)(X - CSize), (double)(Y + ASize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - CSize), (double)(Y + ASize), (double)(X - Rad), (double)(Y));
ClipDrawRPort(Rast, cb, &ls);


/* 9 O'Clock to 12 O'Clock */
setlineseg(&ls, (double)(X - Rad), (double)(Y), (double)(X - CSize), (double)(Y - ASize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - CSize), (double)(Y - ASize), (double)(X - BSize), (double)(Y - BSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - BSize), (double)(Y - BSize), (double)(X - ASize), (double)(Y - CSize));
ClipDrawRPort(Rast, cb, &ls);

setlineseg(&ls, (double)(X - ASize), (double)(Y - CSize), (double)(X), (double)(Y - Rad));
ClipDrawRPort(Rast, cb, &ls);

/* without this one pixel gets left behind when movement begins */
WritePixel(Rast, X, Y - Rad);

return;
} /* DrawHaze() */

/***********************************************************************/

STATIC_FCN void DrawSun(struct Window *Win, int X, int Y, int Rad, struct clipbounds *cb) // used locally only -> static, AF 26.7.2021
{
 int OutEnd;
 struct lineseg ls;

 DrawHaze(Win, X, Y, Rad, cb);

 OutEnd = Rad + 10;

 WritePixel(Win->RPort, X, Y);

/* 12:00 */
 setlineseg(&ls, (double)(X), (double)(Y - Rad), (double)(X), (double)(Y - OutEnd));
 ClipDrawRPort(Win->RPort, cb, &ls);

/* 3:00 */
 setlineseg(&ls, (double)(X + Rad), (double)(Y), (double)(X + OutEnd), (double)(Y));
 ClipDrawRPort(Win->RPort, cb, &ls);

/* 6:00 */
 setlineseg(&ls, (double)(X), (double)(Y + Rad), (double)(X), (double)(Y + OutEnd));
 ClipDrawRPort(Win->RPort, cb, &ls);

/* 9:00 */
 setlineseg(&ls, (double)(X - Rad), (double)(Y), (double)(X - OutEnd), (double)(Y));
 ClipDrawRPort(Win->RPort, cb, &ls);

/* WritePixel(Win->RPort, X, Y - Rad);*/

} /* DrawSun() */

/*********************************************************************/

struct datum *Datum_MVFind(struct datum *DT, struct datum *ET, long *PT, short WhichOne)
{
long CoordX, CoordY;
struct datum *LD;

 if (DT)
  {
  LD = DT;
  DT = DT->nextdat;
  while (DT && DT != ET)
   {
   CoordX = Lon_X_Convert(DT->values[0]);
   CoordY = Lat_Y_Convert(DT->values[1]);
   if (CoordX >= PT[0] && CoordX <= PT[1] && CoordY >= PT[2] && CoordY <= PT[3])
    break;
   LD = DT;
   DT = DT->nextdat;
   }
  } /* if DT */

 if (DT && DT != ET)
  return (WhichOne ? LD: DT);
 return (NULL);

} /* Datum_MVFind() */
