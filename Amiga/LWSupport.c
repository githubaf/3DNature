/* LWSupport.c
** LightWave 3D support functions for WCS
** Written on 1/5/95 by Gary R. Huber.
** Motion import and export functions formerly in EdPar.c, written 1994.
*/

#include "WCS.h"
/*
struct LightWaveMotion {
 double XYZ[3], HPB[3], SCL[3];
 long Frame, Linear;
 double TCB[3];
};
*/

// AF: 9.Jan23, Write short in Big-Endian (i.e. native Amiga-) format
int fwrite_short_BE(const short *Value, FILE *file);

// AF: 16.Feb23, Write LONG in Big-Endian (i.e. native Amiga-) format
int fwrite_LONG_BE(const LONG *Value, FILE *file);

// size in Bytes, not SHORTs!
// returns 1 if all bytes written, otherwise 0
ssize_t fwrite_SHORT_Array_BE(SHORT *SHORTArray, size_t size, FILE *file); // AF, HGW, 16.Feb23

// returns 1 if all bytes written, otherwise 0
ssize_t fwrite_float_Array_BE(float *FloatArray, size_t size,FILE *file); // AF, HGW, 16.Feb23


short Set_LWM(struct LightWaveMotion *LWM, struct LightWaveInfo *LWInfo,
        short Frame, double Scale); // used locally only -> static, AF 23.7.2021



// use LONG (4 bytes values) not long (8 bytes on 64 bit AROS) AF, 16.Feb.23
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
STATIC_VAR LONG LWNullObj[] = {
0x464F524D, 0x00000028, 0x4C574F42, 0x504E5453,
0x0000000C, 0x00000000, 0x00000000, 0x00000000,
0x53524653, 0x00000000, 0x504F4C53, 0x00000000
};
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
STATIC_VAR LONG LWNullObj[] = {
0x4D524f46, 0x28000000, 0x424F574c, 0x53544e50,
0x0C000000, 0x00000000, 0x00000000, 0x00000000,
0x53465253, 0x00000000, 0x534C4F50, 0x00000000
};
#else
#error "Unsupported Byte-Order"
#endif

short ExportWave(struct LightWaveInfo *LWInfo, FILE *Supplied)
{
 char filename[256];
 short Version, Channels, TotalKeys = 0, i, error = 0;
 long LWMsize;
 double Scale;
 struct LightWaveMotion *LWM = NULL;
 FILE *fLWM;

 if (! Supplied)
  {
  if (LWInfo->Path[0] == 0)
   {
   if (altobjectpath[0] == 0)
    strcpy(altobjectpath, "WCSProjects:");
   strcpy(LWInfo->Path, altobjectpath);
   } /* if */
  strcpy(LWInfo->Name, projectname);
  LWInfo->Name[strlen(LWInfo->Name) - 4] = 0;
  strcat(LWInfo->Name, "LWM");
  if (! getfilename(1, "Export Motion Path/File", LWInfo->Path, LWInfo->Name))
  {
   return (0);
  }
  if (LWInfo->Name[0] == 0)
  {
   return (0);
  }
  strmfp(filename, LWInfo->Path, LWInfo->Name);
  } /* if */

/* set scale factor for appropriate units of measure */

  Scale = 1.0 / ELSCALE_METERS;

/* determine if key frames exist for the motion path */

 for (i=0; i<6; i++)
  {
  TotalKeys += CountKeyFrames(0, i);
  } /* for i=0... */
 TotalKeys += CountKeyFrames(0, 8);

 if (TotalKeys < 1)
  {
  error = 2;
  goto EndWave;
  } /* no motion key frames */

/* allocate some essential resources */

 LWMsize = sizeof (struct LightWaveMotion);
 if ((LWM = (struct LightWaveMotion *)get_Memory(LWMsize, MEMF_CLEAR)) == NULL)
  {
  error = 3;
  goto EndWave;
  } /* if out of memory */
 LWM->TCB[0] = LWM->TCB[1] = LWM->TCB[2] = 0.0;
 LWM->Linear = 0;

 if (! BuildKeyTable())
  {
  error = 3;
  goto EndWave;
  } /* if no key table = out of memory */

/* fill in LightWaveMotion structure */

 Version = 1;
 Channels = 9;

 if (! Supplied)
  {
  if ((fLWM = fopen(filename, "w")) == NULL)
   {
   error = 4;
   goto EndWave;
   } /* if open fail */
  } /* if no file supplied */
 else
  fLWM = Supplied;

 if (! Supplied)
  fprintf(fLWM, "%s\n%1d\n", "LWMO", Version);

 fprintf(fLWM, "%1d\n%1d\n", Channels, KT_MaxFrames + 1);

 for (i=0; i<=KT_MaxFrames; i++)
  {
  LWM->Frame = i;
  Set_LWM(LWM, LWInfo, i, Scale);

  if (fprintf(fLWM, "%f %f %f %f %f %f %f %f %f\n",
	LWM->XYZ[0], LWM->XYZ[1], LWM->XYZ[2],
	LWM->HPB[0], LWM->HPB[1], LWM->HPB[2],
	LWM->SCL[0], LWM->SCL[1], LWM->SCL[2]) < 0)
   {
   error = 5;
   break;
   } /* if write error */
  if (fprintf(fLWM, "%1ld %1ld %f %f %f\n",
	LWM->Frame, LWM->Linear,
	LWM->TCB[0], LWM->TCB[1], LWM->TCB[2]) < 0)
   {
   error = 5;
   break;
   } /* if write error */
  } /* for i=0... */

 if (! Supplied)
  fclose(fLWM);

EndWave:

 if (LWM)
  free_Memory(LWM, LWMsize);
 if (KT)
  FreeKeyTable();
// if (! Supplied) // AF: 17.Mar.23 commented out
  if (TRUE) // AF: 17.Mar.23 Always show the error description, not only for "Lighwave Export - Motion only"
  {
  switch (error)
   {
   case 2:
    {
    User_Message((CONST_STRPTR)"LightWave Motion: Export",
            (CONST_STRPTR)"No Key Frames to export!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* no key frames */
   case 3:
    {
    User_Message((CONST_STRPTR)"LightWave Motion: Export",
            (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    break;
    } /* file write fail */
   case 4:
    {
    User_Message((CONST_STRPTR)"LightWave Motion: Export",
            (CONST_STRPTR)"Error opening file for output!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)LWInfo->Name);
    break;
    } /* no memory */
   case 5:
    {
    User_Message((CONST_STRPTR)"LightWave Motion: Export",
            (CONST_STRPTR)"Error writing to file!\nOperation terminated prematurely.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    Log(ERR_WRITE_FAIL, (CONST_STRPTR)LWInfo->Name);
    break;
    } /* file open fail */
   } /* switch */
  }/* if file name not supplied - function called from LW scene file save */

 return (error);

} /* ExportWave() */

/***********************************************************************/

short Set_LWM(struct LightWaveMotion *LWM, struct LightWaveInfo *LWInfo,
	short Frame, double Scale)
{
 short j, AheadFrame;
 double BankFactor;

 if (KT[2].Key)
  VP.lon = KT[2].Val[0][Frame];	/* X = longitude */
 else
  VP.lon = PAR_FIRST_MOTION(2);
 if (KT[0].Key)
  VP.alt = KT[0].Val[0][Frame];	/* Y = altitude */
 else
  VP.alt = PAR_FIRST_MOTION(0);
 if (KT[1].Key)
  VP.lat = KT[1].Val[0][Frame];	/* Z = latitude */
 else
  VP.lat = PAR_FIRST_MOTION(1);

 if (settings.lookahead)
  {
  if (Frame + settings.lookaheadframes > KT_MaxFrames)
   AheadFrame = KT_MaxFrames;
  else
   AheadFrame = Frame + settings.lookaheadframes;
  if (AheadFrame == Frame && Frame > 1)
   {
   if (KT[2].Key)
    FP.lon = KT[2].Val[0][Frame] + (KT[2].Val[0][Frame] - KT[2].Val[0][Frame - 1]);
   else
    FP.lon = PAR_FIRST_MOTION(2);
   if (KT[0].Key)
    FP.alt = KT[0].Val[0][Frame] + (KT[0].Val[0][Frame] - KT[0].Val[0][Frame - 1]);
   else
    FP.alt = PAR_FIRST_MOTION(0);
   if (KT[1].Key)
    FP.lat = KT[1].Val[0][Frame] + (KT[1].Val[0][Frame] - KT[1].Val[0][Frame - 1]);
   else
    FP.lat = PAR_FIRST_MOTION(1);
   } /* if focus and camera same pt */
  else
   {   
   if (KT[2].Key)
    FP.lon = KT[2].Val[0][AheadFrame];	/* X = longitude */
   else
    FP.lon = PAR_FIRST_MOTION(2);
   if (KT[0].Key)
    FP.alt = KT[0].Val[0][AheadFrame];	/* Y = altitude */
   else
    FP.alt = PAR_FIRST_MOTION(0);
   if (KT[1].Key)
    FP.lat = KT[1].Val[0][AheadFrame];	/* Z = latitude */
   else
    FP.lat = PAR_FIRST_MOTION(1);
   } /* else normal look ahead */
  } /* if look ahead */
 else
  {
  if (KT[5].Key)
   FP.lon = KT[5].Val[0][Frame];	/* X = longitude */
  else
   FP.lon = PAR_FIRST_MOTION(5);
  if (KT[3].Key)
   FP.alt = KT[3].Val[0][Frame];	/* Y = altitude */
  else
   FP.alt = PAR_FIRST_MOTION(3);
  if (KT[4].Key)
   FP.lat = KT[4].Val[0][Frame];	/* Z = latitude */
  else
   FP.lat = PAR_FIRST_MOTION(4);
  } /* else no look ahead */

 if (KT[8].Key)
  PARC_RNDR_MOTION(8) = KT[8].Val[0][Frame];	/* Bank */
 else
  PARC_RNDR_MOTION(8) = PAR_FIRST_MOTION(8);

 if (settings.bankturn)
  {
  BankFactor = 0.0;
  for (j=-5; j<=5; j++)
   {
   BankFactor += ComputeBanking(Frame + j);
   }
  PARC_RNDR_MOTION(8) += (BankFactor / 11.0);
  } /* if automatic turn banking - multiple frames to smooth out the wobblies */

 if (LWInfo->RotateHorizontal)
  {
  VP.lon -= LWInfo->RefLon;
  FP.lon -= LWInfo->RefLon;
  } /* if */

 DP.lat = FP.lat;
 DP.lon = FP.lon;
 DP.alt = FP.alt + 1000;
 convertpt(&VP);
 convertpt(&FP);
 findposvector(&FP, &VP);
 yrot = findangle2(FP.x, FP.z);
 rotate(&FP.x, &FP.z, yrot, yrot);
 xrot = findangle2(FP.y, FP.z);
 rotate(&FP.y, &FP.z, xrot, xrot);
 convertpt(&DP);
 findposvector(&DP, &VP);
 rotate(&DP.x, &DP.z, yrot, findangle2(DP.x, DP.z));
 rotate(&DP.y, &DP.z, xrot, findangle2(DP.y, DP.z));
 zrot = findangle2(DP.x, DP.y);
 rotate(&DP.x, &DP.y, zrot, zrot);
 zrot -=PARC_RNDR_MOTION(8) * PiOver180;

 LWM->XYZ[0] = VP.x * Scale;	/* X */
 LWM->XYZ[1] = VP.y * Scale;	/* Y */
 LWM->XYZ[2] = VP.z * Scale;	/* Z */

 LWM->HPB[0] = yrot * PiUnder180;				/* Heading */
 LWM->HPB[1] = -xrot * PiUnder180;				/* Pitch */
 LWM->HPB[2] = -zrot * PiUnder180;				/* Bank */

 LWM->SCL[0] = 0.0;			/* Scale X */
 LWM->SCL[1] = 0.0;			/* Scale Y */
 LWM->SCL[2] = 0.0;			/* Scale Z */

 return (1);

} /* Set_LWM() */

/**************************************************************************/

short LWOB_Export(char *ObjectName, char *OutputName, struct coords *PP,
	LONG MaxVertices, LONG MaxPolys, short SaveObject,
	short Bathymetry, double LonRot)
{
char filename[256], DEMPath[256], DEMName[32], Ptrn[32], *ChunkTag;
FILE *fLWOB = NULL;
float *VertexData = NULL;
struct elmapheaderV101 map;
short Done = 0, success = 0, ShortPad = 0, OpenOK;
SHORT *PolyData = NULL;
LONG Vertices, Polys, LWRows, LWCols, LWRowInt, LWColInt, FORMSize = 0,
	PNTSSize, SRFSSize, POLSSize, zip, pzip, LWr, LWc, Lr, Lc, CurVtx;
double PivotPt[3], ptelev;
struct DirList *DLItem;
struct coords DP;

 memset (&map, 0, sizeof (struct elmapheaderV101));

/* select DEM for export */
 if (! ObjectName)
  {
  strcpy(Ptrn, "#?.elev");
  strcpy(DEMPath, dirname);
  DEMName[0] = 0;
  if (! getfilenameptrn(0, "DEM file to export", DEMPath, DEMName, Ptrn))
   {
   return (0);
   } /* if */
  strmfp(filename, DEMPath, DEMName);
  if (readDEM(filename, &map))
   {
   User_Message_Def((CONST_STRPTR)DEMName,
           (CONST_STRPTR)"Error loading DEM Object!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   goto EndExport;
   } /* if error reading DEM */
  } /* if no object name supplied */
 else
  {
  OpenOK = 0;

  strcpy(DEMName, ObjectName);
  strcat(DEMName, ".elev");
  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, DEMName);
   if (readDEM(filename, &map) != 0)
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
   Log(WNG_OPEN_FAIL, (CONST_STRPTR)DEMName);
   User_Message_Def((CONST_STRPTR)DEMName,
           (CONST_STRPTR)"Error loading DEM Object!\nObject not saved.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
   goto EndExport;
   } /* if */
  } /* else find the object and read it */

/* prepare object for export */
 LWRows = map.rows + 1;
 LWCols = map.columns;
 Vertices = LWRows * LWCols;
 Polys = (LWRows - 1) * (LWCols - 1) * 2;
 LWColInt = LWRowInt = 1;

 if (Vertices > 32768 || Vertices >= MaxVertices || Polys >= MaxPolys)
  Done = 0;
 else
  Done = 1;

 while (! Done && LWRows > 3 && LWCols > 3)
  {
  if (LWRows > LWCols)
   {
   LWRows = LWRows / 2 + LWRows % 2;
   LWRowInt *= 2;
   } /* if */
  else
   {
   LWCols = LWCols / 2 + LWCols % 2;
   LWColInt *= 2;
   } /* else */ 
  Vertices = LWRows * LWCols;
  Polys = (LWRows - 1) * (LWCols - 1) * 2;
  if (Vertices < 32768 && Vertices <= MaxVertices && Polys <= MaxPolys)
   Done = 1;
  } /* while need to decimate DEM points */

/* allocate vertex array */
 PNTSSize = Vertices * 12;
 POLSSize = Polys * 10;
 SRFSSize = 8;
 FORMSize = PNTSSize + SRFSSize + POLSSize + 4 + 8 + 8 + 8;

 VertexData = (float *)get_Memory(PNTSSize, MEMF_ANY);
 PolyData = (short *)get_Memory(POLSSize, MEMF_ANY);

 if (! VertexData || ! PolyData)
  {
  User_Message_Def((CONST_STRPTR)"LW Object Export", (CONST_STRPTR)"Out of memory!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);
  goto EndExport;
  } /* if */

 if (! BuildKeyTable())
  {
  goto EndExport;
  } /* if */
 frame = 0;
 initmopar();
 initpar();
 setvalues();

 ptelev = map.map[0] * map.elscale / ELSCALE_METERS;
 ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
 if (ptelev <= SeaLevel)
 {
  ptelev = SeaLevel;
 }
 DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
 DP.lat = map.lolat;
 DP.lon = map.lolong + LonRot;
 convertpt(&DP);
 PivotPt[0] = DP.x;
 PivotPt[1] = DP.y;
 PivotPt[2] = DP.z;
 if (PP)
  {
  PP->x = DP.x;
  PP->y = DP.y;
  PP->z = DP.z;
  } /* if coords struct provided */

 if (! SaveObject)
  {
  success = 1;
  goto EndExport;
  } /* if we just needed the pivot point coords */

 for (zip=pzip=LWr=Lr=0; LWr<LWRows; Lr+=LWRowInt, LWr++)
  {
  for (LWc=Lc=0; LWc<LWCols; Lc+=LWColInt, LWc++)
   {
   ptelev = map.map[Lr * map.columns + Lc] * map.elscale / ELSCALE_METERS;
   ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
   if (! Bathymetry && ptelev <= SeaLevel)
    ptelev = SeaLevel;
   DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
   DP.lat = map.lolat + Lc * map.steplat;
   DP.lon = map.lolong - Lr * map.steplong + LonRot;
   convertpt(&DP);

   VertexData[zip] = (DP.x - PivotPt[0]) * 1000.0;
   zip ++;
   VertexData[zip] = (DP.y - PivotPt[1]) * 1000.0;
   zip ++;
   VertexData[zip] = (DP.z - PivotPt[2]) * 1000.0;
   zip ++;

   if (LWr < LWRows - 1 && LWc < LWCols - 1)
    {
    CurVtx = LWr * LWCols + LWc;
    PolyData[pzip] = 3;
    pzip ++;
    PolyData[pzip] = CurVtx;
    pzip ++;
    PolyData[pzip] = CurVtx + 1;
    pzip ++;
    PolyData[pzip] = CurVtx + LWCols;
    pzip ++;
    PolyData[pzip] = 1;
    pzip ++;

    PolyData[pzip] = 3;
    pzip ++;
    PolyData[pzip] = CurVtx + 1;
    pzip ++;
    PolyData[pzip] = CurVtx + 1 + LWCols;
    pzip ++;
    PolyData[pzip] = CurVtx + LWCols;
    pzip ++;
    PolyData[pzip] = 1;
    pzip ++;
    } /* if */
   } /* for LWc=0... */
  } /* for LWr=0... */

/* get output file name */
 if (! OutputName)
  {
  if (! strcmp(&DEMName[strlen(DEMName) - 5], ".elev"))
   DEMName[strlen(DEMName) - 5] = 0;
  strcat(DEMName, ".LWO");
  if (altobjectpath[0] == 0)
   strcpy(altobjectpath, "WCSProjects:");
  if (! getfilename(1, "LW Object path/file", altobjectpath, DEMName))
   {
   goto EndExport;
   } /* if */
  strmfp(filename, altobjectpath, DEMName);
  } /* if no output name supplied */
 else
  strcpy(filename, OutputName);

/* open file */
 if ((fLWOB = fopen(filename, "wb")) == NULL)
  {
  goto EndExport;
  } /* if */

/* write "FORM" and temporary size */
 ChunkTag = "FORM";
 fwrite((char *)ChunkTag, 4, 1, fLWOB);
 fwrite_LONG_BE(&FORMSize,fLWOB);
 ChunkTag = "LWOB";
 fwrite((char *)ChunkTag, 4, 1, fLWOB);
 ChunkTag = "PNTS";
 fwrite((char *)ChunkTag, 4, 1, fLWOB);
 fwrite_LONG_BE(&PNTSSize, fLWOB);

 /* write the points array to the file */
 fwrite_float_Array_BE(VertexData, PNTSSize,fLWOB);

/* write "SRFS" */
 ChunkTag = "SRFS";
 fwrite((char *)ChunkTag, 4, 1, fLWOB);
 fwrite_LONG_BE(&SRFSSize, fLWOB);

/* write "WCSDEM" + 0 + 0 */
 ChunkTag = "WCSDEM";
 fwrite((char *)ChunkTag, 6, 1, fLWOB);
 fwrite_short_BE(&ShortPad, fLWOB);

/* write "POLS" */
 ChunkTag = "POLS";
 fwrite((char *)ChunkTag, 4, 1, fLWOB);
 fwrite_LONG_BE(&POLSSize, fLWOB);

/* write out polygon data */
 if (fwrite_SHORT_Array_BE(PolyData, POLSSize, fLWOB) == 1)
  success = 1;

EndExport:
 if (fLWOB)
  fclose(fLWOB);
 if (KT)
  FreeKeyTable();
 if (map.map)
  free_Memory(map.map, map.size);
 if (VertexData)
  free_Memory(VertexData, PNTSSize);
 if (PolyData)
  free_Memory(PolyData, POLSSize);

 return (success);

} /* LWOB_Export() */

/**************************************************************************/

void LWScene_Export(struct LightWaveInfo *LWInfo)
{
char filename[256], ScenePath[256], SceneFile[32], DEMPath[256],
	DEMName[32];
FILE *fScene, *fNullObj;
long i, StartFile, KeyFrames, length,
	error = 0, MaxHt, NotFound = 0, LastKey;
double value;
struct coords PP;

/* scene file */

 if (LWInfo->Path[0] == 0)
  {
  if (altobjectpath[0] == 0)
   strcpy(altobjectpath, "WCSProjects:");
  strcpy(ScenePath, altobjectpath);
  } /* if */
 else
  strcpy(ScenePath, LWInfo->Path);
 strcpy(SceneFile, projectname);
 if (! strcmp(&SceneFile[strlen(SceneFile) - 5], ".proj"))
  SceneFile[strlen(SceneFile) - 5] = 0;
 strcat(SceneFile, ".LWS");
 if (! getfilename(1, "Scene Path/File", ScenePath, SceneFile))
  {
  return;
  } /* if */
 strmfp(filename, ScenePath, SceneFile);

 StartFile = 0;
 length = strlen(ScenePath) - 6;
 while (StartFile <= length)
  {
  if (strnicmp(&ScenePath[StartFile], "Scenes", 6))
   StartFile ++;
  else
   break;
  } /* while */
 if (StartFile <= length)
  {
  ScenePath[StartFile] = 0;
  if (ScenePath[StartFile - 1] == '/' || ScenePath[StartFile - 1] == '\\')
   ScenePath[StartFile - 1] = 0;
  }
 strcpy(LWInfo->Path, ScenePath);
 strcpy(altobjectpath, ScenePath);

/* save objects */

/* dem path */
 strcpy(DEMPath, ScenePath);
 if(strlen(DEMPath)==0){
     printf("strlen von DEMPath ist 0!\n");
     return;
 }
 if(DEMPath[strlen(DEMPath)-1]!=':')
 {
     strcat(DEMPath, "/");     // AF: only a / of path does not end with : (Drive-Name)
 }
 strcat(DEMPath, "Objects");

 DEMName[0] = 0;
 if (! getfilename(1, "LW DEM Object Path", DEMPath, DEMName))
  {
  return;
  } /* if */

 StartFile = 0;
 length = strlen(DEMPath) - 7;
 while (StartFile <= length)
  {
  if (strnicmp(&DEMPath[StartFile], "Objects", 7))
   StartFile ++;
  else
   break;
  } /* while */
 if (StartFile > length)
  StartFile = 0;

/* save objects */

  LWInfo->MaxVerts = min(LWInfo->MaxVerts, 32767);
  if (LWInfo->MaxPolys < 2 || LWInfo->MaxVerts < 4)
   return;
 
 if ((fScene = fopen(filename, "w")))
  {
  fprintf(fScene, "%s\n%d\n\n", "LWSC", 1);

  fprintf(fScene, "%s %d\n", "FirstFrame", settings.startframe);
  fprintf(fScene, "%s %d\n", "LastFrame",
	settings.startframe + (settings.maxframes - 1) * settings.stepframes);
  fprintf(fScene, "%s %d\n\n", "FrameStep", settings.stepframes);

  strcpy(DEMName, "WCSNull");
  strcat(DEMName, ".LWO");
  strmfp(filename, DEMPath, DEMName);

  fprintf(fScene, "%s %s\n", "LoadObject", &filename[StartFile]);
  fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, 1);
  fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
	0.0, (LWInfo->RotateHorizontal ? -6362683.0: 0.0), 0.0, 0.0,
	(LWInfo->RotateHorizontal ? (90.0 - LWInfo->RefLat): 0.0),
	0.0, 1.0, 1.0, 1.0);
  fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", 0, 0, 0.0, 0.0, 0.0);
  fprintf(fScene, "%s %d\n", "EndBehavior", 1);
  fprintf(fScene, "%s %d\n\n", "ShadowOptions", 7);

  if ((fNullObj = fopen(filename, "wb")))
   {
   fwrite((char *)LWNullObj, 48, 1, fNullObj);
   fclose(fNullObj);
   } /* if null object file opened */

  for (i=0; i<NoOfObjects; i++)
   {
   if ((! strcmp(DBase[i].Special, "TOP") ||
          ! strcmp(DBase[i].Special, "SFC")) && (DBase[i].Flags & 2))
    {
    strcpy(DEMName, DBase[i].Name);
    strcat(DEMName, ".LWO");
    strmfp(filename, DEMPath, DEMName);
    if (! LWOB_Export(DBase[i].Name, filename, &PP, LWInfo->MaxVerts,
	LWInfo->MaxPolys, LWInfo->SaveDEMs,
	LWInfo->Bathymetry,
	(LWInfo->RotateHorizontal ? -LWInfo->RefLon: 0.0)))
     {
     error = 1;
     break;
     } /* if object save error */
    fprintf(fScene, "%s %s\n", "LoadObject", &filename[StartFile]);
    fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, 1);
    fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
	PP.x * 1000.0, PP.y * 1000.0, PP.z * 1000.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", 0, 0, 0.0, 0.0, 0.0);
    fprintf(fScene, "%s %d\n", "EndBehavior", 1);
    fprintf(fScene, "%s %d\n", "ParentObject", 1);
    fprintf(fScene, "%s %d\n\n", "ShadowOptions", 7);
    } /* if */
   } /* for i=0... */

  if (error)
   goto EndExport;

  fprintf(fScene, "%s %d %d %d\n", "AmbientColor",
	 PAR_FIRST_COLOR(1, 0), PAR_FIRST_COLOR(1, 1), PAR_FIRST_COLOR(1, 2));
  fprintf(fScene, "%s %f\n\n", "AmbIntensity", 1.0);

  fprintf(fScene, "%s\n", "AddLight");
  fprintf(fScene, "%s\n", "LightName Sun");

  LastKey = -1;
  for (i=KeyFrames=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == 0 && 
	(KF[i].MoKey.Item == 15 || KF[i].MoKey.Item == 16)))
    {
    LastKey = KF[i].MoKey.KeyFrame;
    KeyFrames ++;
    } /* if */
   } /* for i=0... */
  if (KeyFrames	&& BuildKeyTable())
   {
   LastKey = -1;
   fprintf(fScene, "%s\n%d\n%ld\n", "LightMotion (unnamed)", 9, KeyFrames);
   for (i=0; i<ParHdr.KeyFrames; i++)
    {
    if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == 0 &&
	 (KF[i].MoKey.Item == 15 || KF[i].MoKey.Item == 16)))
     {
     LastKey = KF[i].MoKey.KeyFrame;
     if (KT[15].Key)
      {
      PP.lat = KT[15].Val[0][KF[i].MoKey.KeyFrame];
      } /* if */
     else
      PP.lat = PAR_FIRST_MOTION(15);
     if (KT[16].Key)
      {
      PP.lon = KT[16].Val[0][KF[i].MoKey.KeyFrame];
      } /* if */
     else
      PP.lon = PAR_FIRST_MOTION(16);
     if (LWInfo->RotateHorizontal)
      PP.lon -= LWInfo->RefLon;
     PP.alt = 149.0E+6;
     convertpt(&PP);
     PP.x *= 1000.0;
     PP.y *= 1000.0;
     PP.z *= 1000.0;
     fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
	PP.x, PP.y, PP.z, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
     fprintf(fScene, "%d %d %f %f %f\n",
	KF[i].MoKey.KeyFrame, KF[i].MoKey.Linear,
	KF[i].MoKey.TCB[0], KF[i].MoKey.TCB[1],	KF[i].MoKey.TCB[2]);
     } /* if sun key frame */
    } /* for i=0... */
   FreeKeyTable();
   } /* if sun motion keys */
  else
   {
   PP.lat = PAR_FIRST_MOTION(15);
   PP.lon = PAR_FIRST_MOTION(16);
   if (LWInfo->RotateHorizontal)
    PP.lon -= LWInfo->RefLon;
   PP.alt = 149.0E+6;
   convertpt(&PP);
   PP.x *= 1000.0;
   PP.y *= 1000.0;
   PP.z *= 1000.0;
   fprintf(fScene, "%s\n%d\n%d\n", "LightMotion (unnamed)", 9, 1);
   fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
	PP.x, PP.y, PP.z, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
   fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", 0, 0, 0.0, 0.0, 0.0);
   } /* else */
  fprintf(fScene, "%s %d\n", "EndBehavior", 1);
  fprintf(fScene, "%s %d\n", "ParentObject", 1);
  fprintf(fScene, "%s %d %d %d\n", "LightColor",
	 min(PAR_FIRST_COLOR(0, 0) * 2, 255),
	 min(PAR_FIRST_COLOR(0, 1) * 2, 255),
	 min(PAR_FIRST_COLOR(0, 2) * 2, 255));
  fprintf(fScene, "%s %d\n", "LightType", 1);
  fprintf(fScene, "%s %f\n", "Falloff", 0.0);
  fprintf(fScene, "%s %d\n", "LensFlare", 0);
  fprintf(fScene, "%s %d\n\n", "ShadowCasting", 1);

  fprintf(fScene, "%s\n", "CameraMotion (unnamed)");
  if (ExportWave(LWInfo, fScene))
   {
   error = 1;
   goto EndExport;
   } /* if motion file write failed */
  fprintf(fScene, "%s %d\n", "EndBehavior", 1);
  fprintf(fScene, "%s %d\n", "ParentObject", 1);
  LastKey = -1;
  for (i=KeyFrames=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == 0 && 
	(KF[i].MoKey.Item == 10 || KF[i].MoKey.Item == 11)))
    {
    LastKey = KF[i].MoKey.KeyFrame;
    KeyFrames ++;
    } /* if */
   } /* for i=0... */
  if (KeyFrames	&& BuildKeyTable())
   {
   LastKey = -1;
   fprintf(fScene, "%s\n%d\n%ld\n", "ZoomFactor (envelope)", 1, KeyFrames);
   for (i=0; i<ParHdr.KeyFrames; i++)
    {
    if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == 0 && (KF[i].MoKey.Item == 10 ||
		 KF[i].MoKey.Item == 11)))
     {
     LastKey = KF[i].MoKey.KeyFrame;
     if (KT[10].Key)
      {
      value = KT[10].Val[0][KF[i].MoKey.KeyFrame];
      } /* if */
     else
      value = PAR_FIRST_MOTION(10);
     if (KT[11].Key)
      {
      value = 
	 1.14 /*1.175*/ / (tan((KT[11].Val[0][KF[i].MoKey.KeyFrame] / 2.0) * PiOver180)
	 * value);
      } /* if */
     else
      value = 
	 1.14 /*1.175*/ / (tan((PAR_FIRST_MOTION(11) / 2.0) * PiOver180)
	 * value);
     fprintf(fScene, "%f\n", value);
     fprintf(fScene, "%d %d %f %f %f\n",
	KF[i].MoKey.KeyFrame, KF[i].MoKey.Linear,
	KF[i].MoKey.TCB[0], KF[i].MoKey.TCB[1],	KF[i].MoKey.TCB[2]);
     } /* if sun key frame */
    } /* for i=0... */
   FreeKeyTable();
   fprintf(fScene, "%s %d\n", "EndBehavior", 1);
   } /* if zoom factor keys */
  else
   {
   fprintf(fScene, "%s %f\n", "ZoomFactor",
	 1.14 /*1.175*/ / (tan((PAR_FIRST_MOTION(11) / 2.0) * PiOver180)
	 * PAR_FIRST_MOTION(10)));
   } /* else */
  fprintf(fScene, "%s %d\n", "RenderMode", 2);
  fprintf(fScene, "%s %d\n", "RayTraceEffects", 0);

  if (settings.scrnheight <= 120)
   {
   MaxHt = 120;
   fprintf(fScene, "%s %d\n", "Resolution", -1);
   }
  else if (settings.scrnheight <= 240)
   {
   MaxHt = 240;
   fprintf(fScene, "%s %d\n", "Resolution", 0);
   }
  else if (settings.scrnheight <= 480)
   {
   MaxHt = 480;
   fprintf(fScene, "%s %d\n", "Resolution", 1);
   }
  else if (settings.scrnheight <= 960)
   {
   MaxHt = 960;
   fprintf(fScene, "%s %d\n", "Resolution", 2);
   }
  else
   {
   MaxHt = 1920;
   fprintf(fScene, "%s %d\n", "Resolution", 3);
   }
   
  if (settings.scrnheight == (MaxHt * 4) / 6)
   {
   if (settings.scrnwidth == (MaxHt * 47) / 30)
    {
    fprintf(fScene, "%s %d\n", "Overscan", 1);
    fprintf(fScene, "%s %d\n", "Letterbox", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 8) / 6)
    {
    fprintf(fScene, "%s %d\n", "Letterbox", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 9) / 6)
    {
    fprintf(fScene, "%s %d\n", "Overscan", 1);
    fprintf(fScene, "%s %d\n", "Letterbox", 1);
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 51) / 40)
    {
    fprintf(fScene, "%s %d\n", "Letterbox", 1);
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 17) / 15)
    {
    fprintf(fScene, "%s %d\n", "Letterbox", 1);
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 2);
    }
   else
    NotFound = 1;
   }
  else if (settings.scrnheight == (MaxHt * 5) / 6)
   {
   if (settings.scrnwidth == (MaxHt * 8) / 6)
    {
    }
   else if (settings.scrnwidth == (MaxHt * 51) / 40)
    {
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 17) / 15)
    {
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 2);
    }
   else
    NotFound = 1;
   }
  else if (settings.scrnheight == MaxHt)
   {
   if (settings.scrnwidth == (MaxHt * 47) / 30)
    {
    fprintf(fScene, "%s %d\n", "Overscan", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 9) / 6)
    {
    fprintf(fScene, "%s %d\n", "Overscan", 1);
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 1);
    }
   else if (settings.scrnwidth == (MaxHt * 8) / 6)
    {
    fprintf(fScene, "%s %d\n", "Overscan", 1);
    fprintf(fScene, "%s %d\n", "PixelAspectRatio", 2);
    }
   else
    NotFound = 1;
   }
  else
   NotFound = 1;

  fprintf(fScene, "%s %d\n", "Antialiasing", 0);
  fprintf(fScene, "%s %d\n", "AdaptiveSampling", 1);
  fprintf(fScene, "%s %d\n", "AdaptiveThreshold", 8);
  fprintf(fScene, "%s %d\n", "FilmSize", 2);
  fprintf(fScene, "%s %d\n", "FieldRendering", settings.fieldrender);
  fprintf(fScene, "%s %d\n", "MotionBlur", 0);
  fprintf(fScene, "%s %d\n\n", "DepthOfField", 0);

  strmfp(filename, framepath, framefile);
  if (settings.maxframes > 1)
   {
   fprintf(fScene, "%s %s %s\n", "BGImage", filename, "(sequence)");
   fprintf(fScene, "%s %d %d %d\n", "ImageSequenceInfo", 0, 0, settings.maxframes);
   } /* if image sequence */
  else
   fprintf(fScene, "%s %s\n", "BGImage", filename);
  fprintf(fScene, "%s %d\n", "SolidBackdrop", 1);
  fprintf(fScene, "%s %d %d %d\n", "BackdropColor", 0, 0, 0);
  fprintf(fScene, "%s %d %d %d\n", "ZenithColor", PAR_FIRST_COLOR(4, 0),
	 PAR_FIRST_COLOR(4, 1), PAR_FIRST_COLOR(4, 2));
  fprintf(fScene, "%s %d %d %d\n", "SkyColor", PAR_FIRST_COLOR(3, 0),
	 PAR_FIRST_COLOR(3, 1), PAR_FIRST_COLOR(3, 2));
  fprintf(fScene, "%s %d %d %d\n", "GroundColor", 50, 40, 30);
  fprintf(fScene, "%s %d %d %d\n", "NadirColor", PAR_FIRST_COLOR(4, 0),
	 PAR_FIRST_COLOR(4, 1), PAR_FIRST_COLOR(4, 2));
  fprintf(fScene, "%s %d\n", "FogType", 1);
  for (i=KeyFrames=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == 0 && KF[i].MoKey.Item == 20)
    KeyFrames ++;
   } /* for i=0... */
  if (KeyFrames	&& BuildKeyTable())
   {
   fprintf(fScene, "%s\n%d\n%ld\n", "FogMinDist (envelope)", 1, KeyFrames);
   for (i=0; i<ParHdr.KeyFrames; i++)
    {
    if (KF[i].MoKey.Group == 0 && KF[i].MoKey.Item == 20)
     {
     value = KT[20].Val[0][KF[i].MoKey.KeyFrame];
     value *= 1000.0;
     fprintf(fScene, "%f\n", value);
     fprintf(fScene, "%d %d %f %f %f\n",
	KF[i].MoKey.KeyFrame, KF[i].MoKey.Linear,
	KF[i].MoKey.TCB[0], KF[i].MoKey.TCB[1],	KF[i].MoKey.TCB[2]);
     } /* if sun key frame */
    } /* for i=0... */
   FreeKeyTable();
   fprintf(fScene, "%s %d\n", "EndBehavior", 1);
   } /* if haze start keys */
  else
   {
   value = PAR_FIRST_MOTION(20);
   value *= 1000.0;
   fprintf(fScene, "%s %f\n", "FogMinDist", value);
   } /* else */

  for (i=KeyFrames=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == 0 && (KF[i].MoKey.Item == 20 || KF[i].MoKey.Item == 21))
    KeyFrames ++;
   } /* for i=0... */
  if (KeyFrames	&& BuildKeyTable())
   {
   fprintf(fScene, "%s\n%d\n%ld\n", "FogMaxDist (envelope)", 1, KeyFrames);
   for (i=0; i<ParHdr.KeyFrames; i++)
    {
    if (KF[i].MoKey.Group == 0 && (KF[i].MoKey.Item == 20 || 
		KF[i].MoKey.Item == 21))
     {
     if (KT[20].Key)
      {
      value = KT[20].Val[0][KF[i].MoKey.KeyFrame];
      } /* if */
     else
      value = PAR_FIRST_MOTION(20);
     if (KT[21].Key)
      {
      value += KT[21].Val[0][KF[i].MoKey.KeyFrame];
      } /* if */
     else
      value += PAR_FIRST_MOTION(21);
     value *= 1000.0;
     fprintf(fScene, "%f\n", value);
     fprintf(fScene, "%d %d %f %f %f\n",
	KF[i].MoKey.KeyFrame, KF[i].MoKey.Linear,
	KF[i].MoKey.TCB[0], KF[i].MoKey.TCB[1],	KF[i].MoKey.TCB[2]);
     } /* if sun key frame */
    } /* for i=0... */
   FreeKeyTable();
   fprintf(fScene, "%s %d\n", "EndBehavior", 1);
   } /* if haze end keys */
  else
   {
   value = PAR_FIRST_MOTION(20) + PAR_FIRST_MOTION(21);
   value *= 1000.0;
   fprintf(fScene, "%s %f\n", "FogMaxDist", value);
   } /* else */
  fprintf(fScene, "%s %d %d %d\n", "FogColor",
	 PAR_FIRST_COLOR(2, 0),
	 PAR_FIRST_COLOR(2, 1),
	 PAR_FIRST_COLOR(2, 2));
  fprintf(fScene, "%s %d\n", "DitherIntensity", 1);
  fprintf(fScene, "%s %d\n\n", "AnimatedDither", 0);

  strmfp(filename, framepath, framefile);
  strcat(filename, "LW");
  fprintf(fScene, "%s %s\n\n", "SaveRGBImagesPrefix", filename);

  fprintf(fScene, "%s %d\n", "ViewMode", 5);
  fprintf(fScene, "%s %f %f %f\n", "ViewAimPoint", 0.0, 0.0, 0.0);
  fprintf(fScene, "%s %f %f %f\n", "ViewDirection", 0.0, 0.0, 0.0);
  fprintf(fScene, "%s %f\n", "ViewZoomFactor", 3.2);
  fprintf(fScene, "%s %d\n", "LayoutGrid", 3);
  fprintf(fScene, "%s %f\n", "GridSize", 20000.0);
  fprintf(fScene, "%s %d\n", "ShowObjects", 1);
  fprintf(fScene, "%s %d\n", "ShowBones", 1);
  fprintf(fScene, "%s %d\n", "ShowLights", 0);
  fprintf(fScene, "%s %d\n", "ShowCamera", 1);
  fprintf(fScene, "%s %d\n", "ShowMotionPath", 1);
  fprintf(fScene, "%s %d\n", "ShowSafeAreas", 0);
  fprintf(fScene, "%s %d\n", "ShowBGImage", 0);
  fprintf(fScene, "%s %d\n", "ShowFogRadius", 1);
  fprintf(fScene, "%s %d\n", "ShowRedraw", 0);

EndExport:
  fclose(fScene);
  } /* if file opened */

 if (error)
  User_Message_Def((CONST_STRPTR)"LW Scene Export", (CONST_STRPTR)"A problem occurred saving the LW scene.\n\
If a file was created it will not be complete and may not load properly into LightWave.",
(CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);

 if (NotFound)
  User_Message_Def((CONST_STRPTR)"LW Scene Export", (CONST_STRPTR)"The output image size is not a standard\
 LightWave image size. The zoom factor and image dimensions may not be\
 portrayed correctly in the scene file just created.",
 (CONST_STRPTR)"OK", (CONST_STRPTR)"o", 0);

} /* LWScene_Export() */

/***********************************************************************/

/*
LWSC
1

FirstFrame 600
LastFrame 600
FrameStep 1

LoadObject wcsprojects:Topos.object/40105.6.LWOBb
ObjectMotion (unnamed)
  9
  1
  -4678872.000000 4115472.000000 1319578.000000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7

AmbientColor 255 255 255
AmbIntensity 0.250000

AddLight
LightName Light
LightMotion (unnamed)
  9
  1
  0.0 0.0 0.0 59.999996 29.999998 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
LightColor 255 255 255
LgtIntensity 1.000000
LightType 0
LensFlare 0
ShadowCasting 1

CameraMotion (unnamed)
  9
  2
  -4677688.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  0 0 0.0 0.0 0.0
  -4677299.500000 4120908.750000 1305275.750000 -43.875767 31.254276 23.428293 0.0 0.0 0.0
  600 0 0.0 0.0 0.0
EndBehavior 1
ZoomFactor 3.200000
RenderMode 2
RayTraceEffects 0
Resolution 1
Overscan 1
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 8
FilmSize 2
FieldRendering 0
MotionBlur 0
DepthOfField 0

SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 0 40 80
SkyColor 120 180 240
GroundColor 50 40 30
NadirColor 100 80 60
FogType 0
DitherIntensity 1
AnimatedDither 0

SaveRGBImagesPrefix wcsframes:LWRMNP

ViewMode 5
ViewAimpoint 0.000000 0.000000 0.000000
ViewDirection 0.000000 -0.174533 0.000000
ViewZoomFactor 3.200000
LayoutGrid 3
GridSize 20000.000000
ShowObjects 1
ShowBones 1
ShowLights 0
ShowCamera 1
ShowMotionPath 1
ShowSafeAreas 0
ShowBGImage 0
ShowFogRadius 0
ShowRedraw 0


LWSC
1

FirstFrame 0
LastFrame 0
FrameStep 1

LoadObject WCSProjects:Colorado/RMNP.object/40105.6   .LWOB
ObjectMotion (unnamed)
  9
  1
  -4678872.000000 4115472.000000 1319578.375000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7

AmbientColor 0 0 10
AmbIntensity 1.000000

AddLight
LightName Sun
LightMotion (unnamed)
  9
  1
  -126057988096.000000 30863413248.000000 -73210175488.000000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
LightColor 255 255 255
LgtIntensity (envelope)
  1
  2
  1.0
  0 0 0.0 0.0 0.0
  0.570000
  37 0 0.0 0.0 0.0
EndBehavior 1
LightType 1
Falloff 0.000000
LensFlare 0
ShadowCasting 1

CameraMotion (unnamed)
  9
  3
  -4677688.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  0 0 0.0 0.0 0.0
  -2700000.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  310 1 0.0 0.0 0.0
  -4677299.500000 4120908.750000 1305275.750000 -43.875767 31.254276 23.428293 0.0 0.0 0.0
  600 0 0.0 0.0 0.0
EndBehavior 1
ZoomFactor (envelope)
  1
  3
  2.976381
  0 1 1.0 0.0 0.0
  4.500000
  31 0 0.0 0.0 0.0
  1.0
  59 0 1.0 0.0 0.0
EndBehavior 1
RenderMode 2
RayTraceEffects 0
Resolution 3
Overscan 1
Letterbox 1
PixelAspectRatio 2
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 8
FilmSize 2
FieldRendering 1
MotionBlur 0
DepthOfField 0

BGImage WCSFrames:Molten001 (sequence)
ImageSequenceInfo 0 0 30
SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 80 130 255
SkyColor 255 255 255
GroundColor 50 40 30
NadirColor 80 130 255
FogType 1
FogMinDist 0.011100
FogMaxDist (envelope)
  1
  2
  19999.000000
  0 0 0.0 0.0 0.0
  8000.000000
  31 0 0.0 0.0 0.0
EndBehavior 1
FogColor 129 21 180
BackdropFog 0
DitherIntensity 1
AnimatedDither 0

SaveRGBImagesPrefix WCSFrames:EcoEditor

ViewMode 5
ViewAimpoint 0.000000 0.000000 0.000000
ViewDirection 0.000000 0.000000 0.000000
ViewZoomFactor 3.200000
LayoutGrid 3
GridSize 20000.000000
ShowObjects 1
ShowBones 1
ShowLights 1
ShowCamera 1
ShowMotionPath 1
ShowSafeAreas 0
ShowBGImage 0
ShowFogRadius 1
ShowRedraw 0

*/
