/* DEM.c (ne gisdem.c 14 Jan 1994 CXH)
** Digital elevation model manipulations for WCS.
** Original code written by Gary R. Huber, July 1992. Incorporated into
** GIS July 30, 1993.
*/

#include "WCS.h"
#include "GUIDefines.h"

STATIC_FCN short computerelel(short boxsize, short *arrayptr, struct elmapheaderV101 *map); // used locally only -> static, AF 19.7.2021
STATIC_FCN short Read_USGSProfHeader(FILE *DEM, struct USGS_DEMProfileHeader *ProfHdr); // used locally only -> static, AF 19.7.2021
STATIC_FCN FILE *DEMFile_Init(struct DEMExtractData *DEMExtract, short k, char *MsgHdr); // used locally only -> static, AF 23.7.2021
STATIC_FCN short enterbox(void); // used locally only -> static, AF 23.7.2021
STATIC_FCN short Read_USGSDEMProfile(FILE *DEM, short *ProfPtr, short ProfItems); // used locally only -> static, AF 23.7.2021
STATIC_FCN short Read_USGSHeader(FILE *DEM, struct USGS_DEMHeader *Hdr); // used locally only -> static, AF 23.7.2021
STATIC_FCN short saveDEM(char *filename, struct elmapheaderV101 *Map, short *Array); // used locally only -> static, AF 23.7.2021
STATIC_FCN short SplineMap(short *map, short Xrows, short Ycols, double elvar, double flatmax); // used locally only -> static, AF 23.7.2021
STATIC_FCN void padarray(short *arrayptr, struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021

short readDEM(char *filename, struct elmapheaderV101 *map)
{
 short rowlength, error = 0;
 short *mapptr;
 LONG fhelev, Lr;
 float Version;

 if ((fhelev = open(filename, O_RDONLY, 0)) == -1)
  {
  return (1);
  } /* if open file failed */
 read(fhelev, &Version, 4);

 if (fabs(Version - 1.00) < .0001)
  {
  if ((read (fhelev, map, ELEVHDRLENV100)) != ELEVHDRLENV100)
   {
   Log(WNG_READ_FAIL, (CONST_STRPTR)filename);
   close (fhelev);
   return (1);
   } /* if */
  map->MaxEl = map->MinEl = 0;
  map->Samples = 0;
  map->SumElDif = map->SumElDifSq = 0.0;
  }
 else
 {
     if (fabs(Version - 1.01) < .0001 || fabs(Version - 1.02) < .0001)
  {
  if ((read (fhelev, map, ELEVHDRLENV101)) != ELEVHDRLENV101)
   {
   Log(WNG_READ_FAIL, (CONST_STRPTR)filename);
   close (fhelev);
   return (1);
   } /* if */
  }
 else
  {
  Version = 0.0;
  lseek(fhelev, 0L, 0);
  if ((read (fhelev, map, ELEVHDRLENV100)) != ELEVHDRLENV100)
   {
   Log(WNG_READ_FAIL, (CONST_STRPTR)filename);
   close (fhelev);
   return (1);
   } /* if */
  map->MaxEl = map->MinEl = 0;
  map->Samples = 0;
  map->SumElDif = map->SumElDifSq = 0.0;
  } /* if oldest file version */
}

 map->size = (map->rows + 1) * (map->columns) * 2;
 map->scrnptrsize = map->size * 2;
 map->map = (short *)get_Memory (map->size, MEMF_ANY);

 if (! map->map)
  {
  close (fhelev);
  goto Cleanup;
  } /* if */

 if (Version == 0.0)
  {
  mapptr = (short *)map->map;
  rowlength = 2 * map->columns;
		
  for (Lr=0; Lr<=map->rows; Lr++)
   {
   lseek(fhelev, 2, 1);
   if ((read (fhelev, mapptr, rowlength)) != rowlength)
    {
    error = 1;
    Log(WNG_READ_FAIL, (CONST_STRPTR)filename);
    break;
    } /* if */
   mapptr += map->columns;
   lseek(fhelev, 2, 1);
   } /* for */
  close(fhelev);
  } /* if oldest file version */
 else
  {
  if ((read (fhelev, map->map, map->size)) != map->size)
   {
   Log(WNG_READ_FAIL, (CONST_STRPTR)filename);
   error = 1;
   } /* if */
  close (fhelev);
  } /* else version 1.0+ file */

 if (! error)
  {
  if (fabs(Version - DEM_CURRENT_VERSION) > .0001)
   {
   map->elscale = ELSCALE_METERS;
   if (map->MaxEl == map->MinEl && map->MaxEl == 0)
    FindElMaxMin(map, map->map);
   saveDEM(filename, map, map->map);
   } /* if old file version, re-save it */

  return (0);

  } /* if no error */

Cleanup:

 if (map->map) free_Memory(map->map, map->size);
 map->map = (short *)NULL;

 return (1);

} /* readDEM() */

/*********************************************************************/

STATIC_FCN short saveDEM(char *filename, struct elmapheaderV101 *Map, short *Array) // used locally only -> static, AF 23.7.2021
{
 long fhelev,
	 protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE;
 float Version = DEM_CURRENT_VERSION;

 if ((fhelev = open (filename, O_WRONLY | O_TRUNC | O_CREAT, protflags)) == -1)
  {
  return (0);
  } /* if error opening output file */
 write (fhelev, (char *)&Version, 4);
 write (fhelev, (char *)Map, ELEVHDRLENV101);
 if (write (fhelev, (char *)Array, Map->size) != Map->size)
  {
  close (fhelev);
  return (0);
  } /* if error writing file */
 close(fhelev);

 return (1);

} /* saveDEM() */

/*********************************************************************/

short makerelelfile(char *elevpath, char *elevfile)
{
 char extension[32], filename[255];
 short error = 0, boxsize, Lr;
 short *arrayptr, *mapptr, *rowptr;
 long fhelev, arraysize, rowsize, maprowsize, maprowbytes;
 float Version = DEM_CURRENT_VERSION;
 struct elmapheaderV101 map;

 boxsize = enterbox();

 strsfn(elevfile, NULL, NULL, str, extension);
 if (strcmp(extension, "elev"))
  {
  Log(ERR_WRONG_TYPE, (CONST_STRPTR)elevfile);
  return (0);
  } /* if wrong file type */
 strmfp(filename, elevpath, elevfile);
 if ((error = readDEM(filename, &map)) != 0)
  {
  Log(ERR_READ_FAIL, (CONST_STRPTR)elevfile);
  return (0);
  } /* if error reading DEM file */

 arraysize = (map.rows + 11) * (map.columns + 10) * sizeof (short);
 if ((arrayptr = (short *)get_Memory (arraysize, MEMF_ANY)) == NULL)
  {
  Log(ERR_MEM_FAIL, (CONST_STRPTR)"Creating Relative Elevation Model");
  error = 1;
  goto EndMap;
  } /* if */

 rowsize = map.columns + 10;
 rowptr = arrayptr + 5 * rowsize + 5;
 mapptr = map.map;
 maprowsize = map.columns;
 maprowbytes = maprowsize * sizeof (short);

 for (Lr=0; Lr<=map.rows; Lr++)
  {
  memcpy (rowptr, mapptr, maprowbytes);
  rowptr += rowsize;
  mapptr += maprowsize;
  } /* for Lr=0... */

 padarray(arrayptr, &map);

 if (! computerelel(boxsize, arrayptr, &map))
  {
  error = 1;
  goto EndMap;
  } /* if abort */

 FindElMaxMin(&map, map.map);

 strsfn(elevfile, NULL, NULL, str, extension);
 strmfp(filename, elevpath, str);
 strcat(filename, ".relel");

 if ((fhelev = open (filename, O_WRONLY | O_TRUNC | O_CREAT, 0)) == -1)
  {
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)"Relative elevation");
  error = 1;
  goto EndMap;
  } /* if error opening output file */
 write (fhelev, (char *)&Version, 4);
 write (fhelev, (char *)&map, ELEVHDRLENV101);
 if (write (fhelev, (char *)map.map, map.size) != map.size)
  {
  Log(ERR_WRITE_FAIL, (CONST_STRPTR)"Relative elevation");
  close (fhelev);
  error = 1;
  goto EndMap;
  } /* if error writing file */
 else
  {
  close(fhelev);
  elevfile[strlen(elevfile) - 4] = 0;
  strcat(elevfile, "relel");
  Log(MSG_RELEL_SAVE, (CONST_STRPTR)elevfile);
  } /* else saved OK */

EndMap:
 if (arrayptr) free_Memory (arrayptr, arraysize);
 if (map.map) free_Memory (map.map, map.size);
 map.map = NULL;

 return ((short)(! error));

} /* makerelelfile() */

/*********************************************************************/

STATIC_FCN void padarray(short *arrayptr, struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 short *rowptr, *mapptr, Lr, Lc;
 long rowsize, maprowsize, maprowbytes;

 rowsize = map->columns + 10;
 maprowsize = map->columns;
 maprowbytes = maprowsize * sizeof (short);
 
 rowptr = arrayptr + 5 * rowsize + 5;
 mapptr = arrayptr + 5;
 for (Lr=0; Lr<5; Lr++)
  {
  memcpy(mapptr, rowptr, maprowbytes);
  mapptr += rowsize;
  } /* for Lr=0... */

 rowptr = arrayptr + (map->rows + 5) * rowsize + 5;
 mapptr = rowptr + rowsize;
 for (Lr=0; Lr<5; Lr++)
  {
  memcpy(mapptr, rowptr, maprowbytes);
  mapptr += rowsize;
  } /* for Lr=0... */

 rowptr = arrayptr + 5;
 for (Lr=0; Lr<=map->rows + 10; Lr++)
  {
  mapptr = arrayptr + Lr * rowsize;
  for (Lc=0; Lc<5; Lc++)
   {
   *mapptr = *rowptr;
   mapptr ++;
   } /* for Lc=0... */
  rowptr += rowsize;
  } /* for Lr=0... */

 rowptr = arrayptr + 4 + maprowsize;
 for (Lr=0; Lr<=map->rows + 10; Lr++)
  {
  mapptr = arrayptr + Lr * rowsize + 5 + maprowsize;
  for (Lc=0; Lc<5; Lc++)
   {
   *mapptr = *rowptr;
   mapptr ++;
   } /* for Lc=0... */
  rowptr += rowsize;
  } /* for Lr=0... */

} /*  padarray() */

/*********************************************************************/

STATIC_FCN short enterbox(void) // used locally only -> static, AF 23.7.2021
{
 static const short stdweight11[11][11] = {    // static -> init the array only once and reduce stack usage AF, 23.July 2021
             {0,0,1,1,1, 2,1,1,1,0,0},
             {0,1,1,2,3, 3,3,2,1,1,0},
             {1,1,2,3,4, 4,4,3,2,1,1},
             {1,2,3,4,5, 6,5,4,3,2,1},
             {1,3,4,5,7, 8,7,5,4,3,1},
             {2,3,4,6,8,10,8,6,4,3,2},
             {1,3,4,5,7, 8,7,5,4,3,1},
             {1,2,3,4,5, 6,5,4,3,2,1},
             {1,1,2,3,4, 4,4,3,2,1,1},
             {0,1,1,2,3, 3,3,2,1,1,0},
             {0,0,1,1,1, 2,1,1,1,0,0}
             };
 short boxsize = 11, i, j;
 double sum = 0.0;

#ifdef USER_SPECIFIED_PARAMETERS
/* Current implementation assumes a box size of 11 with standard weighting */ 
 printf("Do you wish to weight the array filter? (2= Std, 1= Input, 0= No): ");
 scanf("%hd", &weightarray);

 if (weightarray == 2) boxsize = 11;
 else
  {
  printf("Enter an odd number for box filter size: ");
  scanf("%hd", &boxsize);
  if (boxsize < 3) boxsize = 1;
  if (boxsize > 11) boxsize = 11;
  }

 if (weightarray)
  {
  if (weightarray == 2)
   {
#endif
 for (i=0; i<boxsize; i++)
  {
  for (j=0; j<boxsize; j++)
   {
   weight[i][j] = stdweight11[i][j];
   sum += weight[i][j];
   } /* for j=0... */
  } /* for i=0... */

#ifdef USER_SPECIFIED_PARAMETERS
   } /* if use standard weight array */
  else
   {
   printf("Enter rows of weight values.\n");
   for (i=0; i<boxsize; i++)
    {
    for (j=0; j<boxsize; j++)
     {
     scanf("%le", &weight[i][j]);
     sum += weight[i][j];
     } /* for j=0... */
    } /* for i=0... */
   } /* else not standard array */
  printf("Thank you. Input finished. Return to WCS Screen\n");
#endif

 if (sum) sum = 1.0 / sum;
 for (i=0; i<boxsize; i++)
  {
  for (j=0; j<boxsize; j++)
   {
   weight[i][j] *=sum;
   } /* for j=0... */
  } /* for i=0... */

#ifdef USER_SPECIFIED_PARAMETERS
  } /* if weighted array */
 else
  {
  printf("Thank you.\n");
  sum=1.0 / (boxsize * boxsize);
  for (i=0; i<boxsize; i++)
   {
   for (j=0; j<boxsize; j++)
    {
    weight[i][j] = sum;
    } /* for j=0... */
   } /* for i=0... */
  } /* else not weighted array */
#endif

 return (boxsize);

} /* enterbox() */

/*********************************************************************/

STATIC_FCN short computerelel(short boxsize, short *arrayptr, struct elmapheaderV101 *map) // used locally only -> static, AF 19.7.2021
{
 short a, b, Lr, Lc, boxoffset, error = 0;
 short *mapptr, *dataptr, *firstpt, *firstptr;
 long datarowsize;
 double sum, ScaleFactor;
 struct BusyWindow *BWRE;

 BWRE = BusyWin_New("Computing", map->rows + 1, 0, MakeID('B','W','R','E'));
 boxoffset = (boxsize - 1) / 2;
 datarowsize = map->columns + 10;
 mapptr = map->map;
 ScaleFactor =
	 (((1.0 / 1200.0) / map->steplat) + ((1.0 / 1200.0) / map->steplong))
	 / 2.0;

 for (Lr=0; Lr<=map->rows; Lr++)
  {
  firstpt = arrayptr + (Lr + 5 - boxoffset) * datarowsize + 5 - boxoffset;
  for (Lc=0; Lc<map->columns; Lc++)
   {
   sum = 0.0;
   firstptr = firstpt;
   for (a=0; a<boxsize; a++)
    {
    dataptr = firstptr;
    for (b=0; b<boxsize; b++)
     {
     sum += (*dataptr * weight[a][b]);
     dataptr ++;
     } /* for b=0... */
    firstptr += datarowsize;
    } /* for a=0... */
   *mapptr -= sum;
   *mapptr *= ScaleFactor;
   mapptr ++;
   firstpt ++;
   } /* for Lc=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWRE, Lr + 1);
  } /* for Lr=0... */

 BusyWin_Del(BWRE);

 return ((short)(! error));

} /* computerelel() */

/*********************************************************************/

short InterpDEM(struct DEMInterpolateData *DEMInterp)
{
 char append, rootfile[32], newrootfile[32], extension[32],
	filename[256], elevfile[32];
 short filelen, i, j, k, error = 0, newrows, newcols, Lr, Lc, OBNexists, Elev[6];
 short *arrayptr, *mapptr, *dataptr;
 long fhelev, arraysize, maprowbytes,
	 protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE;
 float Version = DEM_CURRENT_VERSION;
 double lolat, lolong, Lon[6], Lat[6];
 struct elmapheaderV101 map;
 struct BusyWindow *BWFI;

 BWFI = BusyWin_New("Files", DEMInterp->FrFile->rf_NumArgs, 0, MakeID('B','W','F','I'));

 for (k=0; k<DEMInterp->FrFile->rf_NumArgs; k++)
  {
  strcpy((char*)DEMInterp->elevfile, (char*)DEMInterp->FrFile->rf_ArgList[k].wa_Name);

  if (DEMInterp->elevfile[0] == 0)
   {
   User_Message((CONST_STRPTR)"Data Ops: DEM Interpolate",
           (CONST_STRPTR)"No file(s) selected!",	(CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* if no file */

RelelRepeat:

  strcpy (elevfile, DEMInterp->elevfile);
  strsfn(DEMInterp->elevfile, NULL, NULL, rootfile, extension);
  if (stricmp(extension, "elev") && stricmp(extension, "relel"))   // AF: make it case independent
   {
   Log(ERR_WRONG_TYPE, (CONST_STRPTR)DEMInterp->elevfile);
   if (! User_Message((CONST_STRPTR)DEMInterp->elevfile,
           (CONST_STRPTR)"Error opening file for interpolation!\nFile not DEM or REM\nContinue?",
           (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc"))
    {
    break;
    }
   continue;
   }

/* warn if not enough room for interpolated name */
  if (! strcmp(extension, "elev"))
   {
   if (rootfile[length[0] - 1] != ' ')
    {
    if (User_Message_Def((CONST_STRPTR)rootfile,
            (CONST_STRPTR)"DEM name is too long to add an extra character to.\
 Do you wish to enter a new base name for the DEM or abort the interpolation?",
 (CONST_STRPTR)"New Name|Abort", (CONST_STRPTR)"na", 1))
     {
     strcpy(str, rootfile);
     if (GetInputString("Enter new object name.", ":;*/?`#%", str))
      {
      while (strlen(str) < length[0])
       strcat(str, " ");
      str[length[0]] = 0;
      strcpy(newrootfile, str);
      } /* if new name entered */
     else
      continue;
     } /* if chooses to enter new name */
    else
     continue;
    } /* if name too long to append letters */
   else
    strcpy(newrootfile, rootfile);
   } /* if elev file */

  strmfp(filename, DEMInterp->elevpath, DEMInterp->elevfile);
  if ((error = readDEM(filename, &map)) != 0)
   {
   if (! User_Message((CONST_STRPTR)"Data Ops: Interpolate DEM",
           (CONST_STRPTR)"Error reading elevation file!\nContinue?",
           (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc"))
    {
    break;
    } /* if not continue to next file */
   continue;
   } /* if error reading DEM file */

  map.steplat *= .5;
  map.steplong *= .5;

  newrows = map.rows * 2;
  newcols = map.columns * 2 - 1;
  arraysize = (newrows + 1) * newcols * 2;
  arrayptr = (short *)get_Memory (arraysize, MEMF_CLEAR);
  if (arrayptr == NULL)
   {
   free_Memory (map.map, map.size);
   User_Message((CONST_STRPTR)"Data Ops: Interpolate DEM",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   } /* if */

  mapptr = map.map;
  dataptr = arrayptr;
  for (Lr=0; Lr<=map.rows; Lr++)
   {
   for (Lc=0; Lc<map.columns; Lc++)
    {
    *dataptr = *mapptr;
    mapptr ++;
    dataptr += 2;
    } /* for Lc=columns... */
   dataptr += (newcols - 1);
   } /* for Lr=0... */

  if (! SplineMap(arrayptr, newrows + 1, newcols,
	 DEMInterp->elvar, DEMInterp->flatmax))
   {
   error = 1;
   break;
   } /* if abort */

/* turn off old map in database */
  if (! strcmp(extension, "elev"))
   {
   for (i=0; i<NoOfObjects; i++)
    {
    if (! strcmp(DBase[i].Name, rootfile))
     {
     DBase[i].Mark[0] = 'N';
     DBase[i].Enabled = ' ';
     DBase[i].Flags &= (255 ^ 2);
     break;
     } /* if matching database item found */
    } /* for i=0... */
   } /* if elevation file */

  append = 'A';
  lolat = map.lolat;
  lolong = map.lolong;
  maprowbytes = map.columns * sizeof (short);
  filelen = strlen(newrootfile) - 1;
  while (newrootfile[filelen] == ' ' && filelen) newrootfile[filelen--] = 0;

  for (i=0; i<2; i++)
   {
   for (j=0; j<2; j++)
    {
    map.lolat = lolat + j * (map.columns - 1) * map.steplat;
    map.lolong = lolong - i * map.rows * map.steplong;
    mapptr = map.map;
    dataptr = arrayptr + i * map.rows * newcols + j * (map.columns - 1);
    for (Lr=0; Lr<=map.rows; Lr++)
     {
     memcpy(mapptr, dataptr, maprowbytes);
     mapptr += map.columns;
     dataptr += newcols;
     } /* for Lr=0... */

    strcpy(DEMInterp->elevfile, newrootfile);
    DEMInterp->elevfile[filelen + 1] = append;
    DEMInterp->elevfile[filelen + 2] = 0;
    while (strlen(DEMInterp->elevfile) < 10) strcat(DEMInterp->elevfile, " ");
    strcat(DEMInterp->elevfile, ".");
    strcat(DEMInterp->elevfile, extension);
    strmfp(filename, DEMInterp->elevpath, DEMInterp->elevfile);

    if ((fhelev = open (filename, O_WRONLY | O_TRUNC | O_CREAT, protflags))
	 == -1)
     {
     User_Message((CONST_STRPTR)"Data Ops: Interpolate DEM",
             (CONST_STRPTR)"Error opening DEM file for output!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_OPEN_FAIL, (CONST_STRPTR)DEMInterp->elevfile);
     error = 1;
     break;
     } /* if open file failed */
    FindElMaxMin(&map, map.map);
    write (fhelev, (char *)&Version, 4);
    write (fhelev, (char *)&map, ELEVHDRLENV101);
    if (write (fhelev, (char *)map.map, map.size) != map.size)
     {
     User_Message((CONST_STRPTR)"Data Ops: Interpolate DEM",
             (CONST_STRPTR)"Error writing DEM file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_WRITE_FAIL, (CONST_STRPTR)DEMInterp->elevfile);
     close (fhelev);
     error = 1;
     break;
     }
    else
     {
     close (fhelev);
     Log(MSG_DEM_SAVE, (CONST_STRPTR)DEMInterp->elevfile);
     if (! strcmp(extension, "elev"))
      {
/* find out if object is already in the database */
      DEMInterp->elevfile[length[0]] = 0;
      OBNexists = 0;
      for (OBN=0; OBN<NoOfObjects; OBN++)
       {
       if (! strcmp(DEMInterp->elevfile, DBase[OBN].Name))
        {
        OBNexists = 1;
        break;
	}
       } /* for OBN=0... */
      if (! OBNexists)
       {
       if (NoOfObjects + 1 > DBaseRecords)
        {
        struct database *NewBase;

        if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
         {
         User_Message((CONST_STRPTR)"Database Module",
                 (CONST_STRPTR)"Out of memory expanding database!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
         error = 1;
         break;
         } /* if new database allocation fails */
        else
         {
         DBase = NewBase;
         DBaseRecords += 20;
         } /* else new database allocated and copied */
	} /* if need more database space */
       strncpy(DBase[OBN].Name, DEMInterp->elevfile, length[0]);
       strcpy(DBase[OBN].Layer1, "  ");
       strcpy(DBase[OBN].Layer2, "TOP");
       DBase[OBN].Color = 2;
       DBase[OBN].LineWidth = 1;
       strcpy(DBase[OBN].Label, "               ");
       DBase[OBN].MaxFract = 9;
       DBase[OBN].Pattern[0] = 'L';
       DBase[OBN].Red = 255;
       DBase[OBN].Grn = 255;
       DBase[OBN].Blu = 255;
       NoOfObjects ++;
       if (DE_Win)
        {
        if (! Add_DE_NewItem())
         {
         User_Message((CONST_STRPTR)"Database Module",
                 (CONST_STRPTR)"Out of memory expanding Database Editor List!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
         error = 1;
         } /* if new list fails */
        } /* if database editor open */
       } /* if object file not already exists */
      DBase[OBN].Points = 5;
      DBase[OBN].Mark[0] = 'Y';
      DBase[OBN].Enabled = '*';
      DBase[OBN].Flags = 6;
      strcpy(DBase[OBN].Special, "TOP");
      DB_Mod = 1;

      Lat[0] = Lat[1] = Lat[2] = Lat[5] = map.lolat;
      Lat[3] = Lat[4]  = map.lolat + (map.columns - 1) * map.steplat;
      Lon[0] = Lon[1] = Lon[4] = Lon[5] = map.lolong - map.rows * map.steplong;
      Lon[2] = Lon[3] = map.lolong;
      memset(&Elev[0], 0, 6 * sizeof (short));

      DEMInterp->elevfile[length[0]] = 0;
      strcat(DEMInterp->elevfile, ".Obj");
      strmfp(filename, DEMInterp->elevpath, DEMInterp->elevfile);

      if (saveobject(OBN, filename, &Lon[0], &Lat[0], &Elev[0]))
       {
       User_Message((CONST_STRPTR)"Data Ops: Interpolate DEM",
               (CONST_STRPTR)"Error opening Object file for output!\nOperation terminated.",
               (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
       error = 1;
       break;
       } /* error saving .Obj file */
      } /* if elevation file */
     } /* else file saved OK */
    append ++;
    if (error) break;
    } /* for j=0... */
   if (error) break;
   } /* for i=0... */

//EndMap:
  if (map.map) free_Memory(map.map, map.size);
  map.map = NULL;
  if (arrayptr) free_Memory(arrayptr, arraysize);
  arrayptr = NULL;
  if (error) break;

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWFI, k + 1);

/* now repeat operation for relel file or create it if it doesn't exist */
  if (! strcmp(extension, "elev"))
   {
   FILE *felev;

   strcpy(DEMInterp->elevfile, elevfile);
   elevfile[strlen(elevfile) - 4] = 0;
   strcat(elevfile, "relel");
   strmfp(filename, DEMInterp->elevpath, elevfile);
   if ((felev = fopen(filename, "rb")) == NULL)
    {
    if (makerelelfile(DEMInterp->elevpath, DEMInterp->elevfile))
     {
     goto RelelRepeat;
     } /* if make relel successful */
    } /* if relel file not exists */
   else
    {
    fclose(felev);
    strcpy(DEMInterp->elevfile, elevfile);
    goto RelelRepeat;
    } /* else file exists */
   } /* if elev file */

  } /* for k=0... */

 BusyWin_Del(BWFI);

 if (map.map) free_Memory(map.map, map.size);
 if (arrayptr) free_Memory(arrayptr, arraysize);

 return ((short)(! error));

} /* InterpDEM() */

/*********************************************************************/

STATIC_FCN short SplineMap(short *map, short Xrows, short Ycols,
	double elvar, double flatmax) // used locally only -> static, AF 23.7.2021
{
 long i, j, CurPt, NxtPt, Steps, LastStep, rowsize;
 double Delta, P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, TempVal, RandVar;
 short *rowptr, error = 0;
 struct BusyWindow *BWRE;

 BWRE = BusyWin_New("Computing", Xrows, 0, MakeID('B','W','R','E'));

 Steps = Ycols - 2;
 LastStep = Steps - 2;
 rowptr = map;
 rowsize = 2 * Ycols;

 for (i=0; i<Xrows; i+=2)
  {
  for (j=0; j<Steps; j+=2)
   {
   CurPt = j;
   NxtPt = CurPt + 2;
   Delta = .5;
   P1 = rowptr[CurPt];
   P2 = rowptr[NxtPt];
   D1 = j > 0 ? (.5 * (P2 - rowptr[CurPt - 2])): (P2 - P1);
   D2 = j < LastStep ?	(.5 * (rowptr[NxtPt + 2] - P1)): (P2 - P1);

   S1 = Delta;
   S2 = S1 * S1;
   S3 = S1 * S2;
   h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
   h2 = -2.0 * S3 + 3.0 * S2;
   h3 = S3 - 2.0 * S2 + S1;
   h4 = S3 - S2;
   TempVal = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
   if (TempVal > P1 && TempVal > P2) TempVal = max(P1, P2);
   else if (TempVal < P1 && TempVal < P2) TempVal = min(P1, P2);
   RandVar = elvar * fabs(P2 - P1);
   if (RandVar < flatmax) RandVar = flatmax;
   RandVar *= (2.0 * (.5 - drand48()));
   rowptr[CurPt + 1] = (short)(TempVal + RandVar);
   } /* for j=0... */
  rowptr += rowsize;
  } /* for i=0... */

 Steps = Xrows - 2;
 LastStep = Steps - 2;
 rowptr = map;
 rowsize = 2 * Ycols;

 for (i=0; i<Ycols; i++)
  {
  for (j=0; j<Steps; j+=2)
   {
   CurPt = j * Ycols;
   NxtPt = CurPt + rowsize;
   Delta = .5;
   P1 = rowptr[CurPt];
   P2 = rowptr[NxtPt];
   D1 = j > 0 ? (.5 * (P2 - rowptr[CurPt - rowsize])): (P2 - P1);
   D2 = j < LastStep ?	(.5 * (rowptr[NxtPt + rowsize] - P1)): (P2 - P1);

   S1 = Delta;
   S2 = S1 * S1;
   S3 = S1 * S2;
   h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
   h2 = -2.0 * S3 + 3.0 * S2;
   h3 = S3 - 2.0 * S2 + S1;
   h4 = S3 - S2;
   TempVal = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
   if (TempVal > P1 && TempVal > P2) TempVal = max(P1, P2);
   else if (TempVal < P1 && TempVal < P2) TempVal = min(P1, P2);
   RandVar = elvar * fabs(P2 - P1);
   if (RandVar < flatmax) RandVar = flatmax;
   RandVar *= (2.0 * (.5 - drand48()));
   rowptr[CurPt + Ycols] = (short)(TempVal + RandVar);
   } /* for j=0... */
  rowptr ++;
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWRE, i + 1);
  } /* for i=0... */

 BusyWin_Del(BWRE);

 return ((short)(! error));

}/* SplineMap() */

/*********************************************************************/

short ExtractDEM(struct DEMExtractData *DEMExtract)
{
 char elevbase[32], ExtChar[2],	*MsgHdr = "Data Ops: DEM Extract";
 short *TmpPtr, *MapPtr, error = 0, k, Elev, RowSign, ColSign, UTMZone;
 long i, j, Lr, Rows, Columns, lowrow, lowcol, MapRowSize,
	Row, Col, MapCtrX, MapCtrY, CornerPtX, CornerPtY, RowQuit, ColQuit;
 double Slope;
 FILE *DEMFile;
 struct elmapheaderV101 MapHdr;
 struct BusyWindow *BWFI, *BWRE;

 MapHdr.elscale 	= ELSCALE_METERS;

 DEMExtract->UTMData 	= NULL;
 DEMExtract->LLData 	= NULL;

/* Allocate some basic structures */

 if ((DEMExtract->Info = (struct DEMInfo *)get_Memory
	(DEMExtract->FrFile->rf_NumArgs * sizeof (struct DEMInfo), MEMF_CLEAR))
	== NULL)
  {
  User_Message((CONST_STRPTR)MsgHdr,
          (CONST_STRPTR)"Out of memory allocating DEM Info Header!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (0);
  } /* if */
 if ((DEMExtract->Convert = (struct UTMLatLonCoords *)get_Memory
	(sizeof (struct UTMLatLonCoords), MEMF_CLEAR))
	== NULL)
  {
  User_Message((CONST_STRPTR)MsgHdr,
          (CONST_STRPTR)"Out of memory allocating DEM Info Header!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto EndExtract;
  } /* if */

/* Get information about each DEM */
 
 for (k=0; k<DEMExtract->FrFile->rf_NumArgs; k++)
  {
  DEMFile = DEMFile_Init(DEMExtract, k, MsgHdr);
  fclose(DEMFile);

  DEMExtract->Info[k].Code = atoi(DEMExtract->USGSHdr.RefSysCode);
  DEMExtract->Info[k].Zone = atoi(DEMExtract->USGSHdr.Zone);
  DEMExtract->Info[k].HUnits = atoi(DEMExtract->USGSHdr.GrUnits);
  DEMExtract->Info[k].VUnits = atoi(DEMExtract->USGSHdr.ElUnits);
  for (i=0; i<3; i++)
   DEMExtract->Info[k].Res[i] = FCvt(DEMExtract->USGSHdr.Resolution[i]);
  UTMZone = DEMExtract->Info[k].Zone;
  UTMLatLonCoords_Init(DEMExtract->Convert, UTMZone);
  for (i=0; i<4; i++)
   {
   DEMExtract->Info[k].UTM[i][0] = FCvt(DEMExtract->USGSHdr.Coords[i][0]);
   DEMExtract->Info[k].UTM[i][1] = FCvt(DEMExtract->USGSHdr.Coords[i][1]);
   DEMExtract->Convert->East = DEMExtract->Info[k].UTM[i][0];
   DEMExtract->Convert->North = DEMExtract->Info[k].UTM[i][1];
   UTM_LatLon(DEMExtract->Convert);
   DEMExtract->Info[k].LL[i][0] = DEMExtract->Convert->Lon;
   DEMExtract->Info[k].LL[i][1] = DEMExtract->Convert->Lat;
   } /* for i=0... */
  } /* for k=0... determine file types, corner pts... */

 if (error)
  goto EndExtract;

/* figure out the boundaries of the map area and grid intervals */

 DEMExtract->MaxEast[0] 		= -FLT_MAX;
 DEMExtract->MinEast[0] 		= FLT_MAX;
 DEMExtract->MaxNorth[0] 		= -FLT_MAX;
 DEMExtract->MinNorth[0] 		= FLT_MAX;
 DEMExtract->MaxLon 		= -FLT_MAX;
 DEMExtract->MinLon 		= FLT_MAX;
 DEMExtract->MaxLat 		= -FLT_MAX;
 DEMExtract->MinLat 		= FLT_MAX;
 UTMZone 			= -1;
 for (k=0; k<DEMExtract->FrFile->rf_NumArgs; k++)
  {
  if (DEMExtract->Info[k].Code == 1)
   {
   if (UTMZone == -1)
    {
    UTMZone 		  = DEMExtract->Info[k].Zone;
    DEMExtract->UTMColInt = DEMExtract->Info[k].Res[0];
    DEMExtract->UTMRowInt = DEMExtract->Info[k].Res[1];
    }
   else if (DEMExtract->Info[k].Zone != UTMZone)
    {
    User_Message((CONST_STRPTR)MsgHdr,
            (CONST_STRPTR)"7.5 Minute DEMs do not all lie within same UTM Zone!\nOperation terminated.",
            (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    error = 1;
    break;
    } /* else if */
   
   if (DEMExtract->Info[k].UTM[2][0] > DEMExtract->MaxEast[0])
    {
    DEMExtract->MaxEast[0] = DEMExtract->Info[k].UTM[2][0];
    DEMExtract->MaxEast[1] = DEMExtract->Info[k].UTM[2][1];
    }
   if (DEMExtract->Info[k].UTM[3][0] > DEMExtract->MaxEast[0])
    {
    DEMExtract->MaxEast[0] = DEMExtract->Info[k].UTM[3][0];
    DEMExtract->MaxEast[1] = DEMExtract->Info[k].UTM[3][1];
    }

   if (DEMExtract->Info[k].UTM[0][0] < DEMExtract->MinEast[0])
    {
    DEMExtract->MinEast[0] = DEMExtract->Info[k].UTM[0][0];
    DEMExtract->MinEast[1] = DEMExtract->Info[k].UTM[0][1];
    }
   if (DEMExtract->Info[k].UTM[1][0] < DEMExtract->MinEast[0])
    {
    DEMExtract->MinEast[0] = DEMExtract->Info[k].UTM[1][0];
    DEMExtract->MinEast[1] = DEMExtract->Info[k].UTM[1][1];
    }

   if (DEMExtract->Info[k].UTM[1][1] > DEMExtract->MaxNorth[0])
    {
    DEMExtract->MaxNorth[0] = DEMExtract->Info[k].UTM[1][1];
    DEMExtract->MaxNorth[1] = DEMExtract->Info[k].UTM[1][0];
    }
   if (DEMExtract->Info[k].UTM[2][1] > DEMExtract->MaxNorth[0])
    {
    DEMExtract->MaxNorth[0] = DEMExtract->Info[k].UTM[2][1];
    DEMExtract->MaxNorth[1] = DEMExtract->Info[k].UTM[2][0];
    }

   if (DEMExtract->Info[k].UTM[0][1] < DEMExtract->MinNorth[0])
    {
    DEMExtract->MinNorth[0] = DEMExtract->Info[k].UTM[0][1];
    DEMExtract->MinNorth[1] = DEMExtract->Info[k].UTM[0][0];
    }
   if (DEMExtract->Info[k].UTM[3][1] < DEMExtract->MinNorth[0])
    {
    DEMExtract->MinNorth[0] = DEMExtract->Info[k].UTM[3][1];
    DEMExtract->MinNorth[1] = DEMExtract->Info[k].UTM[3][0];
    }

   if (DEMExtract->Info[k].LL[0][0] > DEMExtract->MaxLon)
    DEMExtract->MaxLon = DEMExtract->Info[k].LL[0][0];
   if (DEMExtract->Info[k].LL[1][0] > DEMExtract->MaxLon)
    DEMExtract->MaxLon = DEMExtract->Info[k].LL[1][0];

   if (DEMExtract->Info[k].LL[2][0] < DEMExtract->MinLon)
    DEMExtract->MinLon = DEMExtract->Info[k].LL[2][0];
   if (DEMExtract->Info[k].LL[3][0] < DEMExtract->MinLon)
    DEMExtract->MinLon = DEMExtract->Info[k].LL[3][0];

   if (DEMExtract->Info[k].LL[1][1] > DEMExtract->MaxLat)
    DEMExtract->MaxLat = DEMExtract->Info[k].LL[1][1];
   if (DEMExtract->Info[k].LL[2][1] > DEMExtract->MaxLat)
    DEMExtract->MaxLat = DEMExtract->Info[k].LL[2][1];

   if (DEMExtract->Info[k].LL[0][1] < DEMExtract->MinLat)
    DEMExtract->MinLat = DEMExtract->Info[k].LL[0][1];
   if (DEMExtract->Info[k].LL[3][1] < DEMExtract->MinLat)
    DEMExtract->MinLat = DEMExtract->Info[k].LL[3][1];
   } /* if */
  } /* for k=0... */

 if (error)
  goto EndExtract;

/* if at least one 7.5 minute DEM found, process it first */

 if (UTMZone >= 0)
  {
  long temp;

  temp = 1 + DEMExtract->MinEast[0] / DEMExtract->UTMColInt;
  DEMExtract->FirstCol = temp * DEMExtract->UTMColInt;
  DEMExtract->UTMCols = 1 + fabs((DEMExtract->MaxEast[0] - DEMExtract->FirstCol)
	 / DEMExtract->UTMColInt); 
  temp = 1 + DEMExtract->MinNorth[0] / DEMExtract->UTMRowInt;
  DEMExtract->FirstRow = temp * DEMExtract->UTMRowInt;
  DEMExtract->UTMRows = 1 + fabs((DEMExtract->MaxNorth[0] - DEMExtract->FirstRow)
	 / DEMExtract->UTMRowInt);

/* derive the output intervals from the input intervals */

  DEMExtract->LLRowInt = DEMExtract->UTMRowInt / (1000.0 * LATSCALE);
  
  DEMExtract->LLColInt = DEMExtract->UTMColInt /
	(1000.0 * (LATSCALE * 
	cos(PiOver180 * (DEMExtract->MaxLat + DEMExtract->MinLat) / 2.0)));

  DEMExtract->LLRows = abs((DEMExtract->MaxLat - DEMExtract->MinLat)
	 / DEMExtract->LLRowInt);
  DEMExtract->LLCols = abs((DEMExtract->MaxLon - DEMExtract->MinLon)
	 / DEMExtract->LLColInt);

/* recompute the output intervals based on the total spread of data */

  DEMExtract->LLRowInt = (DEMExtract->MaxLat - DEMExtract->MinLat)
	 / (DEMExtract->LLRows - 1);
  DEMExtract->LLColInt = (DEMExtract->MaxLon - DEMExtract->MinLon)
	 / (DEMExtract->LLCols - 1);

  DEMExtract->UTMSize = DEMExtract->UTMRows * DEMExtract->UTMCols * sizeof (short);
  DEMExtract->LLSize = DEMExtract->LLRows * DEMExtract->LLCols * sizeof (short);

  DEMExtract->UTMData = (short *)get_Memory(DEMExtract->UTMSize, MEMF_ANY);
  DEMExtract->LLData = (short *)get_Memory(DEMExtract->LLSize, MEMF_CLEAR);

  if (! DEMExtract->UTMData || ! DEMExtract->LLData)
   {
   User_Message((CONST_STRPTR)MsgHdr,
           (CONST_STRPTR)"Out of memory allocating DEM Arrays!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   error = 1;
   goto EndPhase1;
   } /* if */

  MapPtr = DEMExtract->UTMData;
  for (i=0; i<DEMExtract->UTMCols; i++)
   {
   for (j=0; j<DEMExtract->UTMRows; j++, MapPtr++)
    *MapPtr = -32000;
   } /* for i=0... */
  
/*
printf("\nXE %f NE %f XN %f NN %f\n", DEMExtract->MaxEast[0], DEMExtract->MinEast[0],
	DEMExtract->MaxNorth[0], DEMExtract->MinNorth[0]);

printf("XT %f NT %f XL %f NL %f\n", DEMExtract->MaxLat, DEMExtract->MinLat,
	DEMExtract->MaxLon, DEMExtract->MinLon);

printf("UR %d UC %d LR %d LC %d\n", DEMExtract->UTMRows, DEMExtract->UTMCols,
	DEMExtract->LLRows, DEMExtract->LLCols);

printf("URI %f UCI %f LRI %f LCI %f\n", DEMExtract->UTMRowInt, DEMExtract->UTMColInt,
	DEMExtract->LLRowInt, DEMExtract->LLColInt);
*/

/* Extract 7.5 Min DEM's */

  BWFI = BusyWin_New("7.5 Minute", DEMExtract->FrFile->rf_NumArgs, 0, MakeID('B','W','F','I'));

  for (k=0; k<DEMExtract->FrFile->rf_NumArgs; k++)
   {
   if (DEMExtract->Info[k].Code != 1)
    continue;

   if ((DEMFile = DEMFile_Init(DEMExtract, k, MsgHdr)) == NULL)
    break;

   Columns = atoi(DEMExtract->USGSHdr.Columns);

   BWRE = BusyWin_New("Reading", Columns, 0, MakeID('B','W','R','E'));

   fseek(DEMFile, 1024, SEEK_SET);
   for (i=0; i<Columns; i++)
    {
    if (! Read_USGSProfHeader(DEMFile, &DEMExtract->ProfHdr))
     {
     fclose(DEMFile);
     User_Message((CONST_STRPTR)MsgHdr,
             (CONST_STRPTR)"Can't read DEM profile header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
     break;
     } /* if read header failed */
    if (atoi(DEMExtract->ProfHdr.Column) != i + 1)
     {
     User_Message((CONST_STRPTR)MsgHdr,
             (CONST_STRPTR)"Error reading DEM profile header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_READ_FAIL,(CONST_STRPTR) DEMExtract->elevfile);
     break;
     } /* if read header failed */

    Rows = atoi(DEMExtract->ProfHdr.ProfRows);

    Col = (FCvt(DEMExtract->ProfHdr.Coords[0]) +.0000001 - DEMExtract->FirstCol)
	/ DEMExtract->UTMColInt;
    Row = (FCvt(DEMExtract->ProfHdr.Coords[1]) +.0000001 - DEMExtract->FirstRow)
	/ DEMExtract->UTMRowInt;

    if (Row < 0 || Col < 0)
     {
     User_Message((CONST_STRPTR)MsgHdr,
             (CONST_STRPTR)"Error reading DEM profile header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
     break;
     }
    Read_USGSDEMProfile(DEMFile,
	&DEMExtract->UTMData[Col * DEMExtract->UTMRows + Row], Rows);

    if (CheckInput_ID() == ID_BW_CLOSE)
     {
     error = 1;
     break;
     }
    BusyWin_Update(BWRE, i + 1);
    } /* for i=0... */
   BusyWin_Del(BWRE);

   fclose (DEMFile);
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    }
   BusyWin_Update(BWFI, k + 1);
   } /* for k=0... */
  BusyWin_Del(BWFI);

  if (error)
   goto EndPhase1;

/* Test view the extracted data
if (1)
 {
 FILE *fTemp;

 if (fTemp = fopen("RAM:DEMtest", "w"))
  {
  fwrite((char *)DEMExtract->UTMData, DEMExtract->UTMSize, 1, fTemp);
  fclose(fTemp);
  printf("Rows %d  Cols %d\n", DEMExtract->UTMRows, DEMExtract->UTMCols);
  }
 else
  printf("Error opening file\n");
 } // if 1
*/

/* fill any missing cells with adjacent elevations */

  BWRE = BusyWin_New("Blank Patch", 4, 0, MakeID('B','W','R','E'));

  MapCtrX 	= DEMExtract->UTMCols / 2;  
  MapCtrY 	= DEMExtract->UTMRows / 2;  

  for (k=0; k<4; k++)
   {
   switch (k)
    {
    case 0:
     {
     CornerPtX 	= 0;
     CornerPtY	= 0;
     RowSign	= -1;
     ColSign	= -1;
     RowQuit	= -1;
     ColQuit	= -1;
     break;
     } /* SouthWest */
    case 1:
     {
     CornerPtX 	= 0;
     CornerPtY	= DEMExtract->UTMRows - 1;
     RowSign	= +1;
     ColSign	= -1;
     RowQuit	= DEMExtract->UTMRows;
     ColQuit	= -1;
     break;
     } /* NorthWest */
    case 2:
     {
     CornerPtX 	= DEMExtract->UTMCols - 1;
     CornerPtY	= DEMExtract->UTMRows - 1;
     RowSign	= +1;
     ColSign	= +1;
     RowQuit	= DEMExtract->UTMRows;
     ColQuit	= DEMExtract->UTMCols;
     break;
     } /* NorthEast */
    case 3:
     {
     CornerPtX 	= DEMExtract->UTMCols - 1;
     CornerPtY	= 0;
     RowSign	= -1;
     ColSign	= +1;
     RowQuit	= -1;
     ColQuit	= DEMExtract->UTMCols;
     break;
     } /* SouthEast */
    } /* switch */

   Slope	= ((double)CornerPtY - MapCtrY) / ((double)CornerPtX - MapCtrX);

   Elev = DEMExtract->UTMData[MapCtrX * DEMExtract->UTMRows + MapCtrY];
   for (Row=MapCtrY, i=0; Row!=RowQuit; Row+=RowSign, i+=RowSign)
    {
    Col = MapCtrX + i / Slope;
    MapPtr = DEMExtract->UTMData + Col * DEMExtract->UTMRows + Row;
    while (Col != ColQuit && *MapPtr != -32000)
     {
     Elev = *MapPtr;
     Col += ColSign;
     MapPtr += (ColSign * DEMExtract->UTMRows);
     }
    while (Col != ColQuit)
     {
     *MapPtr = Elev;
     Col += ColSign;
     MapPtr += (ColSign * DEMExtract->UTMRows);
     } /* while */
    } /* for Row=... */

   Elev = DEMExtract->UTMData[MapCtrX * DEMExtract->UTMRows + MapCtrY];
   for (Col=MapCtrX, i=0; Col!=ColQuit; Col+=ColSign, i+=ColSign)
    {
    Row = MapCtrY + i * Slope;
    MapPtr = DEMExtract->UTMData + Col * DEMExtract->UTMRows + Row;
    while (Row != RowQuit && *MapPtr != -32000)
     {
     Elev = *MapPtr;
     Row += RowSign;
     MapPtr += RowSign;
     }
    while (Row != RowQuit)
     {
     *MapPtr = Elev;
     Row += RowSign;
     MapPtr += RowSign;
     } /* while */
    } /* for Row=... */

   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    }
   BusyWin_Update(BWRE, k + 1);

   } /* for k=0... */

  BusyWin_Del(BWRE);

  if (error)
   goto EndPhase1;

/* Resample UTM grid into Lat/Lon grid */

  BWRE = BusyWin_New("Resample", DEMExtract->LLCols, 0, MakeID('B','W','R','E'));

  DEMExtract->Convert->Lon = DEMExtract->MaxLon;
  for (i=0; i<DEMExtract->LLCols; i++, DEMExtract->Convert->Lon -= DEMExtract->LLColInt)
   {
   DEMExtract->Convert->Lat = DEMExtract->MinLat;
   for (j=0; j<DEMExtract->LLRows; j++, DEMExtract->Convert->Lat += DEMExtract->LLRowInt)
    {
    LatLon_UTM(DEMExtract->Convert, UTMZone);

    DEMExtract->LLData[i * DEMExtract->LLRows + j] = Point_Extract
	(DEMExtract->Convert->East, DEMExtract->Convert->North,
	DEMExtract->FirstCol, DEMExtract->FirstRow, DEMExtract->UTMColInt,
	DEMExtract->UTMRowInt, DEMExtract->UTMData, DEMExtract->UTMRows,
	DEMExtract->UTMCols);
    } /* for j=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    }
   BusyWin_Update(BWRE, i + 1);
   } /* for i=0... */
  BusyWin_Del(BWRE);

  if (error)
   goto EndPhase1;

/* 7.5 Minute file save */

  MapHdr.columns 	= DEMExtract->LLRows;
  MapHdr.rows 		= DEMExtract->LLCols - 1;
  MapHdr.steplat 	= DEMExtract->LLRowInt;
  MapHdr.steplong 	= DEMExtract->LLColInt;
  MapHdr.lolat 		= DEMExtract->MinLat;
  MapHdr.lolong 	= DEMExtract->MaxLon;

  strcpy(elevbase, DEMExtract->elevfile);
  elevbase[length[0]] = 0;

  if (GetInputString("Enter a name for the 30 meter DEM object.",
	":;*/?`#%", elevbase))
   {
   elevbase[length[0]] = 0;
   while (strlen(elevbase) < length[0])
    strcat(elevbase, " ");

   error = DEMFile_Save(elevbase, &MapHdr, DEMExtract->LLData, DEMExtract->LLSize);
   } /* if a name provided */
  else
   error = 1;
  } /* if at least one 7.5 minute DEM */

EndPhase1:

 if (DEMExtract->UTMData)
  free_Memory(DEMExtract->UTMData, DEMExtract->UTMSize);
 if (DEMExtract->LLData)
  free_Memory(DEMExtract->LLData, DEMExtract->LLSize);
 DEMExtract->UTMData = DEMExtract->LLData = NULL;
 DEMExtract->UTMSize = DEMExtract->LLSize - 0;

 if (error)
  goto EndExtract;

/* Extract One Degree DEM's if any are selected */

 BWFI = BusyWin_New("One Degree", DEMExtract->FrFile->rf_NumArgs, 0, MakeID('B','W','F','I'));

 for (k=0; k<DEMExtract->FrFile->rf_NumArgs; k++)
  {
  if (DEMExtract->Info[k].Code != 0)
   continue;

  if ((DEMFile = DEMFile_Init(DEMExtract, k, MsgHdr)) == NULL)
   {
   error = 1;
   break;
   } /* if initialization of DEM file fails */

  if (! Read_USGSProfHeader(DEMFile, &DEMExtract->ProfHdr))
   {
   fclose(DEMFile);
   User_Message((CONST_STRPTR)MsgHdr,
           (CONST_STRPTR)"Can't read DEM profile header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
   error = 1;
   break;
   } /* if read header failed */
  Rows = atoi(DEMExtract->ProfHdr.ProfRows);
  Columns = atoi(DEMExtract->USGSHdr.Columns);

  MapHdr.rows 		= (Columns - 1) / 4;
  MapHdr.columns 	= 1 + ((Rows - 1) / 4);
  MapHdr.steplat 	= 1.0 / (Rows - 1);
  MapHdr.steplong 	= 1.0 / (Columns - 1);

  if (! Set_DM_Data(DEMExtract))
   {
   fclose(DEMFile);
   error = 1;
   break;
   } /* if user cancels */

  DEMExtract->UTMSize = Rows * Columns * sizeof (short);
  if ((DEMExtract->UTMData = (short *)get_Memory
	(DEMExtract->UTMSize, MEMF_ANY)) == NULL)
   {
   fclose(DEMFile);
   User_Message((CONST_STRPTR)MsgHdr,
           (CONST_STRPTR)"Out of memory allocating temporary buffer!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
   error = 1;
   break;
   } /* if read header failed */

  BWRE = BusyWin_New("Reading", Columns, 0, MakeID('B','W','R','E'));
  for (i=0; i<Columns; i++)
   {
   TmpPtr = DEMExtract->UTMData + i * Rows;
   if (! Read_USGSDEMProfile(DEMFile, TmpPtr, Rows))
    {
    User_Message((CONST_STRPTR)MsgHdr,
            (CONST_STRPTR)"Error reading DEM profile!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
    error = 1;
    break;
    } /* if not read error */
   if (i < Columns - 1)
    {
    if (! Read_USGSProfHeader(DEMFile, &DEMExtract->ProfHdr))
     {
     User_Message((CONST_STRPTR)MsgHdr,
             (CONST_STRPTR)"Error reading DEM profile header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
     error = 1;
     break;
     } /* if not read error */
    if (atoi(DEMExtract->ProfHdr.ProfRows) != Rows)
     {
     User_Message((CONST_STRPTR)MsgHdr,
             (CONST_STRPTR)"Improper DEM profile length!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
     error = 1;
     break;
     } /* if unequal row lengths */
    } /* if not last column */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    }
   BusyWin_Update(BWRE, i + 1);
   } /* for i=0... */
  BusyWin_Del(BWRE);

  fclose(DEMFile);
 
  if (error)
   {
   if (DEMExtract->UTMData) free_Memory(DEMExtract->UTMData, DEMExtract->UTMSize);
   break;
   } /* if error reading DEM */

/* allocate small array to copy data to */
  DEMExtract->LLCols 	= MapHdr.rows + 1;
  DEMExtract->LLRows 	= MapHdr.columns;
  DEMExtract->LLSize 	= DEMExtract->LLCols * DEMExtract->LLRows * sizeof (short);
  MapRowSize 		= DEMExtract->LLRows * sizeof (short);
  if ((DEMExtract->LLData = (short *)get_Memory(DEMExtract->LLSize, MEMF_ANY)) == NULL)
   {
   User_Message((CONST_STRPTR)MsgHdr,
           (CONST_STRPTR)"Out of memory allocating map buffer!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
   if (DEMExtract->UTMData)
    free_Memory(DEMExtract->UTMData, DEMExtract->UTMSize);
   break;
   } /* if read header failed */

/* copy data one quad at a time and save */
  ExtChar[1] = 0;
  for (i=0; i<4; i++)
   {
   for (j=0; j<4; j++)
    {
    lowcol = i * (DEMExtract->LLCols - 1);
    lowrow = j * (DEMExtract->LLRows - 1);
    MapPtr = DEMExtract->LLData;
    TmpPtr = DEMExtract->UTMData + lowcol * Rows + lowrow;

    for (Lr=0; Lr<DEMExtract->LLCols; Lr++)
     {
     memcpy(MapPtr, TmpPtr, MapRowSize);
     MapPtr += DEMExtract->LLRows;
     TmpPtr += Rows;
     } /* for Lr=0... */

/* set file name and lat/long parameters */

    MapHdr.lolat = DEMExtract->SELat + lowrow * MapHdr.steplat;
    MapHdr.lolong = DEMExtract->SELon + (Columns - lowcol) * MapHdr.steplong;

    strcpy(elevbase, DEMExtract->elevfile);
    elevbase[length[0] - 2] = 0;
    strcat(elevbase, ".");
    ExtChar[0] = 65 + i * 4 + j;
    strcat(elevbase, ExtChar);
    while (strlen(elevbase) < length[0]) strcat(elevbase, " ");

    error = DEMFile_Save(elevbase, &MapHdr, DEMExtract->LLData, DEMExtract->LLSize);

/*
    strcat(elevbase, ".elev");
    strmfp(filename, dirname, elevbase);

    if ((fhelev = open(filename, flags, protflags)) == -1)
     {
     User_Message(elevbase,
	"Error opening output file!\nOperation terminated.", "OK", "o");
     Log(ERR_OPEN_FAIL, elevbase);
     error = 1;
     break;
     } // error opening file
    FindElMaxMin(&MapHdr, DEMExtract->LLData);
    write(fhelev, (char *)&Version, 4);
    if (write(fhelev, (char *)&MapHdr, ELEVHDRLENV101) != ELEVHDRLENV101)
     {
     User_Message(elevbase,
	"Error writing to output file!\nOperation terminated.", "OK", "o");
     Log(ERR_WRITE_FAIL, elevbase);
     error = 1;
     close(fhelev);
     break;
     } // if error writing to file
    if (write(fhelev,(char *)DEMExtract->LLData, DEMExtract->LLSize) != DEMExtract->LLSize)
     {
     User_Message(elevbase,
	"Error writing to output file!\nOperation terminated.", "OK", "o");
     Log(ERR_WRITE_FAIL, elevbase);
     error=1;
     close(fhelev);
     break;
     } // if error writing to file
    close(fhelev);
    Log(MSG_DEM_SAVE, elevbase);

    OBNexists = 0;
    for (OBN=0; OBN<NoOfObjects; OBN++)
     {
     if (! strcmp(elevbase, DBase[OBN].Name))
      {
      OBNexists = 1;
      break;
      }
     } // for OBN=0...
    if (! OBNexists)
     {
     if (NoOfObjects + 1 > DBaseRecords)
      {
      struct database *NewBase;

      if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
       {
       User_Message("Database Module",
		"Out of memory expanding database!\nOperation terminated.", "OK", "o");
       error = 1;
       break;
       } // if new database allocation fails
      else
       {
       DBase = NewBase;
       DBaseRecords += 20;
       } // else new database allocated and copied
      } // if need more database space
     strncpy(DBase[OBN].Name, elevbase, length[0]);
     strcpy(DBase[OBN].Layer1, "  ");
     strcpy(DBase[OBN].Layer2, "TOP");
     DBase[OBN].Color = 2;
     DBase[OBN].LineWidth = 1;
     strcpy(DBase[OBN].Label, "               ");
     DBase[OBN].MaxFract = 9;
     DBase[OBN].Pattern[0] = 'L';
     DBase[OBN].Red = 255;
     DBase[OBN].Grn = 255;
     DBase[OBN].Blu = 255;
     NoOfObjects ++;
     if (DE_Win)
      {
      if (! Add_DE_NewItem())
       {
       User_Message("Database Module",
		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
       error = 1;
       } // if new list fails
      } // if database editor open
     } // if object not already exists
    DBase[OBN].Points = 5;
    DBase[OBN].Mark[0] = 'Y';
    DBase[OBN].Enabled = '*';
    DBase[OBN].Flags = 6;
    strcpy(DBase[OBN].Special, "TOP");
    DB_Mod = 1;
    if (error) break;

    strcpy(elevbase, DEMExtract->elevfile);
    strcat(elevbase, ".");
    ExtChar[0] = 65 + i * 4 + j;
    strcat(elevbase, ExtChar);
    while (strlen(elevbase) < length[0]) strcat(elevbase, " ");
    strcat(elevbase, ".Obj");
    strmfp(filename, dirname, elevbase);

    Lat[0] = Lat[1] = Lat[2] = Lat[5] = MapHdr.lolat;
    Lat[3] = Lat[4] = MapHdr.lolat + (MapHdr.columns - 1) * MapHdr.steplat;
    Lon[0] = Lon[1] = Lon[4] = Lon[5] = MapHdr.lolong - MapHdr.rows * MapHdr.steplong;
    Lon[2] = Lon[3] = MapHdr.lolong;
    memset(&Elev[0], 0, 6 * sizeof (short));

    if (saveobject(OBN, filename, &Lon[0], &Lat[0], &Elev[0]))
     {
     User_Message(elevbase,
	"Error writing to output file!\nOperation terminated.", "OK", "o");
     error = 1;
     break;
     } // if write error
    elevbase[length[0]] = 0;

*/

    } /* for j=0... */
   if (error) break;

   } /* for i=0... */

  if (DEMExtract->UTMData)
   free_Memory(DEMExtract->UTMData, DEMExtract->UTMSize);
  if (DEMExtract->LLData)
   free_Memory(DEMExtract->LLData, DEMExtract->LLSize);

  if (error)
   {
   User_Message((CONST_STRPTR)MsgHdr,
           (CONST_STRPTR)"Error creating output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* if file output error */

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if abort */
  BusyWin_Update(BWFI, k + 1);
  } /* for k=0... */

 BusyWin_Del(BWFI);

EndExtract:

 if (DEMExtract->Info)
  free_Memory(DEMExtract->Info, 
	DEMExtract->FrFile->rf_NumArgs * sizeof (struct DEMInfo));
 if (DEMExtract->Convert)
  free_Memory(DEMExtract->Convert, sizeof (struct UTMLatLonCoords));
 DEMExtract->Info = NULL;
 DEMExtract->Convert = NULL;

 return ((short)(! error));

} /* ExtractDEM() */

/*********************************************************************/

STATIC_FCN FILE *DEMFile_Init(struct DEMExtractData *DEMExtract, short k, char *MsgHdr) // used locally only -> static, AF 23.7.2021
{
char filename[256];
FILE *DEMFile;

 strcpy(DEMExtract->elevfile, (char*)DEMExtract->FrFile->rf_ArgList[k].wa_Name);

 if (DEMExtract->elevfile[0] == 0)
  {
  User_Message((CONST_STRPTR)MsgHdr,
          (CONST_STRPTR)"No file(s) selected!",	(CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (NULL);
  } /* if no file */

 strmfp(filename, DEMExtract->elevpath, DEMExtract->elevfile);

 if ((DEMFile = fopen(filename, "r")) == NULL)
  {
  User_Message((CONST_STRPTR)MsgHdr,
          (CONST_STRPTR)"Can't open DEM file for input!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
  return (NULL);
  } /* if file open failed */

 if (! Read_USGSHeader(DEMFile, &DEMExtract->USGSHdr))
  {
  fclose(DEMFile);
  User_Message((CONST_STRPTR)MsgHdr,
          (CONST_STRPTR)"Can't read DEM file header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_READ_FAIL, (CONST_STRPTR)DEMExtract->elevfile);
  return (NULL);
  } /* if read header failed */

 return (DEMFile);

} /* DEMFile_Init() */

/*********************************************************************/

short DEMFile_Save(char *BaseName, struct elmapheaderV101 *Hdr,
	short *MapArray, long MapSize)
{
char filename[256], FileBase[32];
short OBNexists, OBN, Elev[6];
long fh, protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE,
	flags = O_WRONLY | O_CREAT | O_TRUNC;
const float Version = DEM_CURRENT_VERSION;
double Lon[6], Lat[6];

 strcpy(FileBase, BaseName);
 strcat(FileBase, ".elev");
 strmfp(filename, dirname, FileBase);

 if ((fh = open(filename, flags, protflags)) == -1)
  {
  User_Message((CONST_STRPTR)FileBase,
	(CONST_STRPTR)"Error opening output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)FileBase);
  return (1);
  } /* error opening file */
 FindElMaxMin(Hdr, MapArray);
 write(fh, (char *)&Version, 4);
 if (write(fh, (char *)Hdr, ELEVHDRLENV101) != ELEVHDRLENV101)
  {
  User_Message((CONST_STRPTR)FileBase,
          (CONST_STRPTR)"Error writing to output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_WRITE_FAIL, (CONST_STRPTR)FileBase);
  close(fh);
  return (1);
  } /* if error writing to file */
 if (write(fh,(char *)MapArray, MapSize) != MapSize)
  {
  User_Message((CONST_STRPTR)FileBase,
          (CONST_STRPTR)"Error writing to output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_WRITE_FAIL, (CONST_STRPTR)FileBase);
  close(fh);
  return (1);
  } /* if error writing to file */
 close(fh);
 Log(MSG_DEM_SAVE, (CONST_STRPTR)FileBase);

 OBNexists = 0;
 for (OBN=0; OBN<NoOfObjects; OBN++)
  {
  if (! strcmp(BaseName, DBase[OBN].Name))
   {
   OBNexists = 1;
   break;
   }
  } /* for OBN=0... */
 if (! OBNexists)
  {
  if (NoOfObjects + 1 > DBaseRecords)
   {
   struct database *NewBase;

   if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
    {
    User_Message((CONST_STRPTR)"Database Module",
            (CONST_STRPTR)"Out of memory expanding database!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    return (1);
    } /* if new database allocation fails */
   else
    {
    DBase = NewBase;
    DBaseRecords += 20;
    } /* else new database allocated and copied */
   } /* if need more database space */
  strncpy(DBase[OBN].Name, BaseName, length[0]);
  strcpy(DBase[OBN].Layer1, "  ");
  strcpy(DBase[OBN].Layer2, "TOP");
  DBase[OBN].Color = 2;
  DBase[OBN].LineWidth = 1;
  strcpy(DBase[OBN].Label, "               ");
  DBase[OBN].MaxFract = 9;
  DBase[OBN].Pattern[0] = 'L';
  DBase[OBN].Red = 255;
  DBase[OBN].Grn = 255;
  DBase[OBN].Blu = 255;
  NoOfObjects ++;
  if (DE_Win)
   {
   if (! Add_DE_NewItem())
    {
    User_Message((CONST_STRPTR)"Database Module",
            (CONST_STRPTR)"Out of memory expanding Database Editor List!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    } /* if new list fails */
   } /* if database editor open */
  } /* if object not already exists */
 DBase[OBN].Points = 5;
 DBase[OBN].Mark[0] = 'Y';
 DBase[OBN].Enabled = '*';
 DBase[OBN].Flags = 6;
 strcpy(DBase[OBN].Special, "TOP");
 DB_Mod = 1;

 strcpy(FileBase, BaseName);
 strcat(FileBase, ".Obj");
 strmfp(filename, dirname, FileBase);

 Lat[0] = Lat[1] = Lat[2] = Lat[5] = Hdr->lolat;
 Lat[3] = Lat[4] = Hdr->lolat + (Hdr->columns - 1) * Hdr->steplat;
 Lon[0] = Lon[1] = Lon[4] = Lon[5] = Hdr->lolong - Hdr->rows * Hdr->steplong;
 Lon[2] = Lon[3] = Hdr->lolong;
 memset(&Elev[0], 0, 6 * sizeof (short));

 if (saveobject(OBN, filename, &Lon[0], &Lat[0], &Elev[0]))
  {
  User_Message((CONST_STRPTR)FileBase,
          (CONST_STRPTR)"Error writing to output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return (1);
  } /* if write error */

 return (0);

} /* DEMFile_Save() */

/*********************************************************************/

STATIC_FCN short Read_USGSHeader(FILE *DEM, struct USGS_DEMHeader *Hdr) // used locally only -> static, AF 23.7.2021
{
 short i, j;

 fgets(Hdr->FileName,	 145,	 DEM);
 fgets(Hdr->LevelCode,	 7,	 DEM);
 fgets(Hdr->ElPattern,	 7,	 DEM);
 fgets(Hdr->RefSysCode,  7,	 DEM);
 fgets(Hdr->Zone,	 7,	 DEM);
 for (i=0; i<15; i++)
  {
  fgets(Hdr->ProjPar[i], 25,	 DEM);
  } /* for i=0... */
 fgets(Hdr->GrUnits,	 7,	 DEM);
 fgets(Hdr->ElUnits,	 7,	 DEM);
 fgets(Hdr->PolySides,	 7,	 DEM);
 for (i=0; i<4; i++)
  {
  for (j=0; j<2; j++)
   {
   fgets(Hdr->Coords[i][j],	 25,	 DEM);
   } /* for j=0... */
  } /* for i=0... */
 fgets(Hdr->ElMin,	 25,	 DEM);
 fgets(Hdr->ElMax,	 25,	 DEM);
 fgets(Hdr->AxisRot,	 25,	 DEM);
 fgets(Hdr->Accuracy,	 7,	 DEM);
 for (i=0; i<3; i++)
  {
  fgets(Hdr->Resolution[i],	13,	 DEM);
  } /* for i=0... */
 fgets(Hdr->Rows,	 7,	 DEM);
 if (! fgets(Hdr->Columns,	 7,	 DEM)) return (0);

 Set_DM_HdrData(Hdr);
/*
 printf("File Name:		%s\n", atoi(Hdr->FileName));
 printf("Level Code: 		%d\n", atoi(Hdr->LevelCode));
 printf("Elev Ptrn: 		%d\n", atoi(Hdr->ElPattern));
 printf("Ref Sys Code: 		%d\n", atoi(Hdr->RefSysCode));
 printf("Zone: 			%d\n", atoi(Hdr->Zone));
 printf("Projection Par: 	%f\n", FCvt(Hdr->ProjPar[0]));
 printf("Ground Units: 		%d\n", atoi(Hdr->GrUnits));
 printf("Elev Units: 		%d\n", atoi(Hdr->ElUnits));
 printf("Polygon Sides: 	%d\n", atoi(Hdr->PolySides));
 printf("SW Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[0][0]), FCvt(Hdr->Coords[0][1]));
 printf("NW Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[1][0]), FCvt(Hdr->Coords[1][1]));
 printf("NE Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[2][0]), FCvt(Hdr->Coords[2][1]));
 printf("SE Corner Coordinates:	%f	%f\n", FCvt(Hdr->Coords[3][0]), FCvt(Hdr->Coords[3][1]));
 printf("Min Elevation:		%f\n", FCvt(Hdr->ElMin));
 printf("Max Elevation: 	%f\n", FCvt(Hdr->ElMax));
 printf("Axis Rotation: 	%f\n", FCvt(Hdr->AxisRot));
 printf("Accuracy: 		%d\n", atoi(Hdr->Accuracy));
 printf("Resolution X: 		%f\n", FCvt(Hdr->Resolution[0]));
 printf("Resolution Y: 		%f\n", FCvt(Hdr->Resolution[1]));
 printf("Resolution Z: 		%f\n", FCvt(Hdr->Resolution[2]));
 printf("Rows: 			%d\n", atoi(Hdr->Rows));
 printf("Columns: 		%d\n", atoi(Hdr->Columns));
*/
 return (1);

} /* Read_USGSHeader() */

/*********************************************************************/

STATIC_FCN short Read_USGSProfHeader(FILE *DEM, struct USGS_DEMProfileHeader *ProfHdr) // used locally only -> static, AF 19.7.2021
{
 short value = 0;

 while (! value)
  {
  if (! fgets(ProfHdr->Row,	 7,	 DEM)) return (0);
  value = atoi(ProfHdr->Row);
  }
 fgets(ProfHdr->Column,		 7,	 DEM);
 fgets(ProfHdr->ProfRows,	 7,	 DEM);
 fgets(ProfHdr->ProfCols,	 7,	 DEM);
 fgets(ProfHdr->Coords[0],	 25,	 DEM);
 fgets(ProfHdr->Coords[1],	 25,	 DEM);
 fgets(ProfHdr->ElDatum,	 25,	 DEM);
 fgets(ProfHdr->ElMin,		 25,	 DEM);
 if (! fgets(ProfHdr->ElMax,	 25,	 DEM)) return (0);

 Set_DM_ProfData(ProfHdr);

 return (1);

} /* Read_USGSProfHeader() */

/*********************************************************************/

STATIC_FCN short Read_USGSDEMProfile(FILE *DEM, short *ProfPtr, short ProfItems) // used locally only -> static, AF 23.7.2021
{
 short i, error = 0;
 short value;

 for (i=0; i<ProfItems; i++)
  {
  if ((fscanf(DEM, "%hd", &value)) != 1)
   {
   if ((fscanf(DEM, "%hd", &value)) != 1)
    {
    if ((fscanf(DEM, "%hd", &value)) != 1)
     {
     error = 1;
     break;
     }
    }
   }
  *ProfPtr = value;
  ProfPtr ++;
  } /* for i=0... */

 if (error) return (0);

 return (1);

} /* Read_USGSDEMProfile() */

/*********************************************************************/

double FCvt(const char *string)
{
 short i = 0, j = 0;
 char base[64];
 char exp[12];

 base[0] = exp[0] = 0; 

 while ( isdigit(string[i]) || string[i] == '.'
	 || string[i] == ' ' || string[i] == '-' || string[i] == '+' )
  {
  base[i] = string[i];
  i ++;
  } /* while not alphabet character */
 base[i] = 0;

 i ++;

 while (string[i] && i < 63)
  {
  exp[j] = string[i];
  i ++;
  j ++;
  }
 exp[j] = 0;

 return (atof(base) * pow(10.0, (double)atoi(exp)));

} /* FCvt() */

/**********************************************************************/


short FixFlatSpots(short *Data, short Rows, short Cols, float T)
{
 short i, j, FirstRow = -1, FirstCol = -1, Pts, MaxPts, MaxRowPts, MaxColPts,
	FirstPt, LastPt, Lr, Lc, error = 0, PT[4], IbtFr, LstInt, NxtInt;
 short *DataCopy;
 long zip;
 double P[4], Delta, D1, D2, S1, S2, S3, h1, h2, h3, h4;


/* make a copy of the data to average later */

 if ((DataCopy = (short *)get_Memory(Rows * Cols * sizeof (short), MEMF_CLEAR))
	== NULL)
  {
  error = 2;
  goto EndSpline;
  } /* if out of memory */

 memcpy(DataCopy, Data, Rows * Cols * sizeof (short));


/* find out which has the largest number of flat points - rows or columns */

 for (Lr=0, MaxRowPts=0; Lr<Rows; Lr++)
  {
  zip = Lr * Cols;
  for (Lc=0, Pts=0; Lc<Cols; Lc++, zip++)
   {
   if (Data[zip] == -30000)
    Pts ++;
   } /* for Lc=0... */
  if (Pts > MaxRowPts)
   {
   MaxRowPts = Pts;
   FirstRow = Lr;
   } /* if new max */
  } /* for Lr=0... */

 if (FirstRow < 0)
  {
  error = 3;
  goto EndSpline;
  }

 for (Lc=0, MaxColPts=0; Lc<Cols; Lc++)
  {
  zip = Lc;
  for (Lr=0, Pts=0; Lr<Rows; Lr++, zip+=Cols)
   {
   if (Data[zip] == -30000)
    Pts ++;
   } /* for Lr=0... */
  if (Pts > MaxColPts)
   {
   MaxColPts = Pts;
   FirstCol = Lc;
   } /* if new max */
  } /* for Lc=0... */

 if (FirstCol < 0)
  {
  error = 3;
  goto EndSpline;
  }

/* Find central point of widest flat area on axis with most flat points */

 if (MaxRowPts > MaxColPts)
  {
  zip = FirstRow * Cols;
  for (Lc=0, MaxPts=0, Pts=0; Lc<Cols; Lc++, zip++)
   {
   if (Data[zip] == -30000)
    {
    if (Pts == 0)
     {
     FirstPt = Lc;
     }
    Pts ++;
    }
   else if (Pts > 0)
    {
    LastPt = Lc - 1;
    if (Pts > MaxPts)
     {
     MaxPts = Pts;
     FirstCol = (FirstPt + LastPt) / 2;
     }
    Pts = 0;
    }
   } /* for Lc=0... */
  } /* if max row > max col */
 else
  {
  zip = FirstCol;
  for (Lr=0, MaxPts=0, Pts=0; Lr<Rows; Lr++, zip+=Cols)
   {
   if (Data[zip] == -30000)
    {
    if (Pts == 0)
     {
     FirstPt = Lr;
     }
    Pts ++;
    }
   else if (Pts > 0)
    {
    LastPt = Lr - 1;
    if (Pts > MaxPts)
     {
     MaxPts = Pts;
     FirstRow = (FirstPt + LastPt) / 2;
     }
    Pts = 0;
    }
   } /* for Lr=0... */
  } /* if max row > max col */

/* spline value at intersection of FirstRow and FirstCol */

#ifdef PRINT_FIRST_VALUES
Data[FirstRow * Cols + FirstCol] = 999;

for (Lr=0, zip=0; Lr<Rows; Lr++)
 {
 for (Lc=0; Lc<Cols; Lc++, zip++)
  {
  printf("%4d", Data[zip]);
  }
 printf("\n\n");
 }
Data[FirstRow * Cols + FirstCol] = -30000;

#endif /* PRINT_FIRST_VALUES */

 zip = FirstRow * Cols + FirstCol - 1;
 for (Lc=FirstCol-1, PT[0]=0, PT[1]=0; Lc>=0; Lc--, zip--)
  {
  if (Data[zip] == -30000)
   continue;
  if (PT[1] == 0)
   PT[1] = Lc;
  else if (PT[0] == 0)
   {
   PT[0] = Lc;
   break;
   }
  } /* for Lc=... */
 if (PT[0] == PT[1])
  {
  error = 1;
  goto EndSpline;
  }
#ifndef USE_NEAR_VALUES
 PT[0] = 0;
#endif

 zip = FirstRow * Cols + FirstCol + 1;
 for (Lc=FirstCol+1, PT[2]=0, PT[3]=0; Lc<Cols; Lc++, zip++)
  {
  if (Data[zip] == -30000)
   continue;
  if (PT[2] == 0)
   PT[2] = Lc;
  else if (PT[3] == 0)
   {
   PT[3] = Lc;
   break;
   }
  } /* for Lc=... */
 if (PT[2] == PT[3])
  {
  error = 1;
  goto EndSpline;
  }
#ifndef USE_NEAR_VALUES
 PT[3] = Cols - 1;
#endif

 zip = FirstRow * Cols;
 P[0] = Data[zip + PT[0]];
 P[1] = Data[zip + PT[1]];
 P[2] = Data[zip + PT[2]];
 P[3] = Data[zip + PT[3]];
 IbtFr  = PT[2] - PT[1];
 LstInt = PT[1] - PT[0];
 NxtInt = PT[3] - PT[2];

 Delta = 1.0 / (float)IbtFr;
 D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
 D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);
 S1 = (FirstCol - PT[1]) * Delta;
 S2 = S1 * S1;
 S3 = S1 * S2;
 h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
 h2 = -2.0 * S3 + 3.0 * S2;
 h3 = S3 - 2.0 * S2 + S1;
 h4 = S3 - S2;
 Data[zip + FirstCol] = P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;

 zip = (FirstRow - 1) * Cols + FirstCol;
 for (Lr=FirstRow-1, PT[0]=0, PT[1]=0; Lr>=0; Lr--, zip-=Cols)
  {
  if (Data[zip] == -30000)
   continue;
  if (PT[1] == 0)
   PT[1] = Lr;
  else if (PT[0] == 0)
   {
   PT[0] = Lr;
   break;
   }
  } /* for Lr=... */
 if (PT[0] == PT[1])
  {
  error = 1;
  goto EndSpline;
  }
#ifndef USE_NEAR_VALUES
 PT[0] = 0;
#endif

 zip = (FirstRow + 1) * Cols + FirstCol;
 for (Lr=FirstRow+1, PT[2]=0, PT[3]=0; Lr<Rows; Lr++, zip +=Cols)
  {
  if (Data[zip] == -30000)
   continue;
  if (PT[2] == 0)
   PT[2] = Lr;
  else if (PT[3] == 0)
   {
   PT[3] = Lr;
   break;
   }
  } /* for Lr=... */
 if (PT[2] == PT[3])
  {
  error = 1;
  goto EndSpline;
  }
#ifndef USE_NEAR_VALUES
 PT[3] = Rows - 1;
#endif

 P[0] = Data[PT[0] * Cols + FirstCol];
 P[1] = Data[PT[1] * Cols + FirstCol];
 P[2] = Data[PT[2] * Cols + FirstCol];
 P[3] = Data[PT[3] * Cols + FirstCol];
 IbtFr  = PT[2] - PT[1];
 LstInt = PT[1] - PT[0];
 NxtInt = PT[3] - PT[2];

 Delta = 1.0 / (float)IbtFr;
 D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
 D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);
 S1 = (FirstRow - PT[1]) * Delta;
 S2 = S1 * S1;
 S3 = S1 * S2;
 h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
 h2 = -2.0 * S3 + 3.0 * S2;
 h3 = S3 - 2.0 * S2 + S1;
 h4 = S3 - S2;

 Data[FirstRow * Cols + FirstCol] += P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;
 Data[FirstRow * Cols + FirstCol] /= 2;

/* Spline all points along the central column */

 PT[0] = Lr = 0;
 while (Lr < Rows-1)
  {
  zip = PT[0] * Cols + FirstCol;
  for (Lr=PT[0],PT[1]=PT[2]=PT[3]=0; Lr<Rows; Lr++, zip+=Cols)
   {
   if ((PT[1] == 0 && Data[zip] != -30000) ||
	(PT[1] != 0 && Data[zip] == -30000))
    continue;
   if (PT[1] == 0)
    {
    PT[0] = Lr - 2;
    PT[1] = Lr - 1;
    }
   else if (PT[2] == 0)
    {
    PT[2] = Lr;
    }
   else
    {
    PT[3] = Lr;

    P[0] = Data[PT[0] * Cols + FirstCol];
    P[1] = Data[PT[1] * Cols + FirstCol];
    P[2] = Data[PT[2] * Cols + FirstCol];
    P[3] = Data[PT[3] * Cols + FirstCol];
    IbtFr  = PT[2] - PT[1];
    LstInt = PT[1] - PT[0];
    NxtInt = PT[3] - PT[2];
    if (IbtFr < 2)
     {
     error = 1;
     goto EndSpline;
     }

    Delta = 1.0 / (float)IbtFr;
    D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
    D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);

    for (i=1; i<IbtFr; i++)
     {
     S1 = i * Delta;
     S2 = S1 * S1;
     S3 = S1 * S2;
     h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
     h2 = -2.0 * S3 + 3.0 * S2;
     h3 = S3 - 2.0 * S2 + S1;
     h4 = S3 - S2;

     Data[(PT[1] + i) * Cols + FirstCol] = P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;
     } /* for i=0... */

    break;
    }
   } /* for Lr=0... */
  
  } /* while */

/* Spline all points along the central row */

 PT[0] = Lc = 0;
 while (Lc < Cols-1)
  {
  zip = FirstRow * Cols + PT[0];
  for (Lc=PT[0],PT[1]=PT[2]=PT[3]=0; Lc<Cols; Lc++, zip++)
   {
   if ((PT[1] == 0 && Data[zip] != -30000) ||
	(PT[1] != 0 && Data[zip] == -30000))
    continue;
   if (PT[1] == 0)
    {
    PT[0] = Lc - 2;
    PT[1] = Lc - 1;
    }
   else if (PT[2] == 0)
    {
    PT[2] = Lc;
    }
   else
    {
    PT[3] = Lc;

    zip = FirstRow * Cols;
    P[0] = Data[zip + PT[0]];
    P[1] = Data[zip + PT[1]];
    P[2] = Data[zip + PT[2]];
    P[3] = Data[zip + PT[3]];
    IbtFr  = PT[2] - PT[1];
    LstInt = PT[1] - PT[0];
    NxtInt = PT[3] - PT[2];
    if (IbtFr < 2)
     {
     error = 1;
     goto EndSpline;
     }
    
    Delta = 1.0 / (float)IbtFr;
    D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
    D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);

    for (i=1; i<IbtFr; i++)
     {
     S1 = i * Delta;
     S2 = S1 * S1;
     S3 = S1 * S2;
     h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
     h2 = -2.0 * S3 + 3.0 * S2;
     h3 = S3 - 2.0 * S2 + S1;
     h4 = S3 - S2;

     Data[zip + PT[1] + i] = P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;
     } /* for i=0... */

    break;
    }
   } /* for Lr=0... */
  
  } /* while */

 memcpy(DataCopy, Data, Rows * Cols * sizeof (short));

/* perform column by column row-wise splines */

 for (j=0; j<Cols; j++)
  {
  PT[0] = Lr = 0;
  while (Lr < Rows-1)
   {
   zip = PT[0] * Cols + j;
   for (Lr=PT[0],PT[1]=PT[2]=PT[3]=0; Lr<Rows; Lr++, zip+=Cols)
    {
    if ((PT[1] == 0 && Data[zip] != -30000) ||
	(PT[1] != 0 && Data[zip] == -30000))
     continue;
    if (PT[1] == 0)
     {
     PT[0] = Lr - 2;
     PT[1] = Lr - 1;
     }
    else if (PT[2] == 0)
     {
     PT[2] = Lr;
     }
    else
     {
     PT[3] = Lr;

     P[0] = Data[PT[0] * Cols + j];
     P[1] = Data[PT[1] * Cols + j];
     P[2] = Data[PT[2] * Cols + j];
     P[3] = Data[PT[3] * Cols + j];
     IbtFr  = PT[2] - PT[1];
     LstInt = PT[1] - PT[0];
     NxtInt = PT[3] - PT[2];
     if (IbtFr < 2)
      {
      error = 1;
      goto EndSpline;
      }

     Delta = 1.0 / (float)IbtFr;
     D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
     D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);

     for (i=1; i<IbtFr; i++)
      {
      S1 = i * Delta;
      S2 = S1 * S1;
      S3 = S1 * S2;
      h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
      h2 = -2.0 * S3 + 3.0 * S2;
      h3 = S3 - 2.0 * S2 + S1;
      h4 = S3 - S2;

      Data[(PT[1] + i) * Cols + j] = P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;
      } /* for i=0... */

     break;
     } /* else */
    } /* for Lr=0... */
  
   } /* while */
  } /* for j=0... */

 swmem(DataCopy, Data, Rows * Cols * sizeof (short));

/* perform row by row column-wise splines */

 for (j=0; j<Rows; j++)
  {
  PT[0] = Lc = 0;
  while (Lc < Cols-1)
   {
   zip = j * Cols + PT[0];
   for (Lc=PT[0],PT[1]=PT[2]=PT[3]=0; Lc<Cols; Lc++, zip++)
    {
    if ((PT[1] == 0 && Data[zip] != -30000) ||
	(PT[1] != 0 && Data[zip] == -30000))
     continue;
    if (PT[1] == 0)
     {
     PT[0] = Lc - 2;
     PT[1] = Lc - 1;
     }
    else if (PT[2] == 0)
     {
     PT[2] = Lc;
     }
    else
     {
     PT[3] = Lc;

     zip = j * Cols;
     P[0] = Data[zip + PT[0]];
     P[1] = Data[zip + PT[1]];
     P[2] = Data[zip + PT[2]];
     P[3] = Data[zip + PT[3]];
     IbtFr  = PT[2] - PT[1];
     LstInt = PT[1] - PT[0];
     NxtInt = PT[3] - PT[2];
     if (IbtFr < 2)
      {
      error = 1;
      goto EndSpline;
      }
    
     Delta = 1.0 / (float)IbtFr;
     D1 = (P[2] - P[0]) * (1.0 - T) * IbtFr / (LstInt + IbtFr);
     D2 = (P[3] - P[1]) * (1.0 - T) * IbtFr / (IbtFr + NxtInt);

     for (i=1; i<IbtFr; i++)
      {
      S1 = i * Delta;
      S2 = S1 * S1;
      S3 = S1 * S2;
      h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
      h2 = -2.0 * S3 + 3.0 * S2;
      h3 = S3 - 2.0 * S2 + S1;
      h4 = S3 - S2;

      Data[zip + PT[1] + i] = P[1] * h1 + P[2] * h2 + D1 * h3 + D2 * h4;
      } /* for i=0... */

     break;
     } /* else */
    } /* for Lr=0... */
  
   } /* while */
  } /* for j=0... */

 for (Lr=0, zip=0; Lr<Rows; Lr++)
  {
  for (Lc=0; Lc<Cols; Lc++, zip++)
   {
   Data[zip] += DataCopy[zip];
   Data[zip] /=2;
   }
  }

#ifdef PRINT_SECOND_VALUES

printf("\n\n");
for (Lr=0, zip=0; Lr<Rows; Lr++)
 {
 for (Lc=0; Lc<Cols; Lc++, zip++)
  {
  printf("%4d", Data[zip]);
  }
 printf("\n\n");
 }

#endif /* PRINT_SECOND_VALUES */

EndSpline:
 if (DataCopy)
  free_Memory(DataCopy, Rows * Cols * sizeof (short));

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"Bad array dimensions! Something doesn't compute.\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   }
  case 2:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   }
  case 3:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"No flat spots to operate on!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   }
  } /* switch */

 if (error)
  return (0);

 return (1);

} /* FixFlatSpots() */

/**********************************************************************/

short FindElMaxMin(struct elmapheaderV101 *Map, short *Array)
{
 short *mapptr;
 long Lr, Lc, LastCol, ElDif;

 if (Map == NULL || Array == NULL)
  return (0);

 Map->MaxEl = -30000;
 Map->MinEl = 30000;
 Map->Samples = 0;
 Map->SumElDif = Map->SumElDifSq = 0.0;
 mapptr = Array;
 LastCol = Map->columns - 1;

 for (Lr=0; Lr<=Map->rows; Lr++)
  {
  for (Lc=0; Lc<Map->columns; Lc++)
   {
   if (*mapptr > Map->MaxEl)
    {
    Map->MaxEl = *mapptr;
    } /* if */
   if (*mapptr < Map->MinEl)
    {
    Map->MinEl = *mapptr;
    } /* else if */
   if (Lr != 0 && Lc != LastCol)
    {
    ElDif = abs(*mapptr - *(mapptr - Map->columns + 1));
    Map->SumElDif += ElDif;
    Map->SumElDifSq += (ElDif * ElDif);
    Map->Samples ++;
    } /* if not first row */
   mapptr ++;
   } /* for Lc=0... */
  } /* for Lr=0... */

 return (1);

} /* FindElMaxMin() */

/**********************************************************************/

short LoadVistaDEM(char *filename, short *Output, short DataPts)
{
 short error = 0, elev;
 long Columns, Compression, HeaderType, i, j, ndx, tmpndx;
 short *DataPtr;
 USHORT count;
 BYTE  *buffy = NULL, *tmp = NULL, delta;
 FILE *fDEM;

 if ((fDEM = fopen(filename, "r")) == NULL)
  {
  return (2);
  } /* if file open error */

 fseek(fDEM, 128, SEEK_SET);
 fread((char *)&Compression, 4, 1, fDEM);
 fread((char *)&HeaderType, 4, 1, fDEM);
 if (! Compression)
  {
  error = 7;
  goto Cleanup;
  }
/*
 if (HeaderType)
  {
  error = 8;
  goto Cleanup;
  }
*/
 buffy = (BYTE *)get_Memory(DataPts * 2, MEMF_CLEAR);
 tmp = (BYTE *)get_Memory(DataPts * 2, MEMF_CLEAR);
 if (! buffy || ! tmp)
  {
  error = 1;
  goto Cleanup;
  }

 Columns = DataPts;
 DataPtr = &Output[(DataPts - 1) * Columns];

 fseek(fDEM, 2048, 0);
 for (i=0; i<DataPts; i++)
  {
  if ((fread(&count, 1, 2, fDEM)) != 2)
   break;
  if ((fread(buffy, 1, count, fDEM)) != count)
   break;
  ndx = 0;
  tmpndx = 0;
  do
   {
   if (buffy[ndx] <= 0)
    {
    delta = buffy[ndx + 1];
    for (j=0; j<(1-buffy[ndx]); j++)
     tmp[tmpndx++] = delta;
    ndx += 2;
    } /* if */
   else
    {
    for (j=0; j<(buffy[ndx] + 1); j++)
     {
     delta = buffy[ndx + j + 1];
     tmp[tmpndx++] = delta;
     } /* for j=0... */
    ndx += buffy[ndx] + 2;
    } /* else */
   } while (ndx < count);
  ndx = 0;
  elev = (UBYTE)tmp[0] * 256 + (UBYTE)tmp[1];
  DataPtr[ndx++] = elev;
  for (j=2; ndx<DataPts; j++)
   {
   delta = tmp[j];
   if (delta == -128)
    {
    elev = (UBYTE)tmp[j + 1] * 256 + (UBYTE)tmp[j + 2];
    j += 2;
    } /* if */
   else
    elev += delta;
   DataPtr[ndx++] = elev;
   } /* for j=2... */
  DataPtr -= DataPts;
  } /* for i=0... */

 if (i < DataPts)
  error = 6;

Cleanup:
 fclose(fDEM);
 if (buffy)
  free_Memory(buffy, DataPts * 2);
 if (tmp)
  free_Memory(tmp, DataPts * 2);

 return (error);

} /* LoadVistaDEM() */

/**********************************************************************/

short LoadDTED(char *filename, short *Output, long OutputSize)
{
char Sentinel[16];
short error = 0, *DataPtr, PtCt[2];
long row, col, Rows, Cols, ColSize, StartPt = 0;
FILE *fDEM;

 if ((fDEM = fopen(filename, "r")) == NULL)
  {
  return (2);
  } /* if file open error */

 fseek(fDEM, 160, SEEK_SET);
 fread((char *)Sentinel, 3, 1, fDEM);
 Sentinel[3] = 0;
 if (strcmp(Sentinel, "DSI"))
  {
  StartPt = -30000;
  fseek(fDEM, 0L, SEEK_SET);
  for (col=0; col<20000; col++)
   {
   fread((char *)Sentinel, 1, 1, fDEM);
   if (Sentinel[0] == 'D')
    {
    fread((char *)Sentinel, 1, 1, fDEM);
    if (Sentinel[0] == 'S')
     {
     fread((char *)Sentinel, 1, 1, fDEM);
     if (Sentinel[0] == 'I')
      {
      StartPt = ftell(fDEM) - 163;
      break;
      } /* if */
     } /* if */
    } /* if */
   } /* for col=0... */
  } /* if */
 if (StartPt <= -30000)
  {
  error = 15;
  goto EndLoad;
  } /* if can't find DSI record */

 fseek(fDEM, StartPt + 441, SEEK_SET);
 fread((char *)Sentinel, 4, 1, fDEM);
 Sentinel[4] = 0;
 Rows = atoi(Sentinel);

 fread((char *)Sentinel, 4, 1, fDEM);
 Cols = atoi(Sentinel);
 ColSize = Rows * sizeof (short);

 if (ColSize * Cols > OutputSize)
  {
  error = 3;
  goto EndLoad;
  }

 fseek(fDEM, StartPt + 3508, SEEK_SET);
 DataPtr = Output;
 
 for (col=0; col<Cols; col++)
  {
  fread((char *)Sentinel, 4, 1, fDEM);
  fread((char *)&PtCt[0], 4, 1, fDEM);
  if (PtCt[0] != col)
   {
   error = 6;
   break;
   }
  fread((char *)DataPtr, ColSize, 1, fDEM);
  fread((char *)Sentinel, 4, 1, fDEM);
  for (row=0; row<Rows; row++)
   {
   if (DataPtr[row] & 0x8000)
    DataPtr[row] = -(DataPtr[row] & 0x7fff);
   } /* for row... */
  DataPtr += Rows;
  } /* for col=0... */

EndLoad:
 fclose(fDEM);

 return (error);

} /* LoadDTED() */

/**********************************************************************/

#ifdef HDHDHDHDHDHDHDHDHSJDHKS
#define DATA_SET_VAL -32768

long Poly[20][4][3];	/* 0=position counter, 1=row, 2=col */

void SubDivide(short *Data, long b, long Cols);

void LandscapeGen(void)
{
char filename[256], DEMPath[256], DEMFile[32];
long i, j, k, Rows, Cols, DataSize, DataValues, RangeEl, MinEl;
short *Data, error = 0;
FILE *fOutput;

 strcpy(DEMPath, "WCSProjects:");
 DEMFile[0] = '\0';

 if (! getfilename(1, "DEM Export Path/Name", DEMPath, DEMFile))
  return;

 sprintf(str, "%d", 301);
 if (! GetInputString("Enter data columns.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
  {
  return;
  } /* if abort */
 Cols = atoi(str);
 if (! GetInputString("Enter data rows.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
  {
  return;
  } /* if abort */
 Rows = atoi(str);

 DataValues = Rows * Cols;
 DataSize = DataValues * sizeof (short);
 if (DataSize <= 0)
  {
  printf("ERROR: DataSize <= 0\n");
  return;
  }
 if ((Data = (short *)get_Memory(DataSize, MEMF_ANY)) == NULL)
  {
  printf("ERROR: No memory available\n");
  return;
  }
 for (i=0; i<DataValues; i++)
  {
  Data[i] = DATA_SET_VAL;
  } /* for i=0... */

 Poly[0][0][0] = 0;
 Poly[0][0][1] = 0;
 Poly[0][0][2] = 0;

 Poly[0][1][0] = Cols - 1;
 Poly[0][1][1] = 0;
 Poly[0][1][2] = Cols - 1;

 Poly[0][2][0] = Cols * (Rows - 1);
 Poly[0][2][1] = Rows - 1;
 Poly[0][2][2] = 0;

 Poly[0][3][0] = Cols * Rows - 1;
 Poly[0][3][1] = Rows - 1;
 Poly[0][3][2] = Cols - 1;


 sprintf(str, "%d", 0);
 if (! GetInputString("Enter northwest elevation.",
	 "+.,abcdefghijklmnopqrstuvwxyz", str))
  {
  goto Cleanup;
  } /* if abort */
 Data[Poly[0][0][0]] = atoi(str);
 if (! GetInputString("Enter southeast elevation.",
	 "+.,abcdefghijklmnopqrstuvwxyz", str))
  {
  goto Cleanup;
  } /* if abort */
 Data[Poly[0][3][0]] = atoi(str);

 srand48(Data[Poly[0][0][0]] * Data[Poly[0][3][0]] + Rows + Cols);

 MinEl = min(Data[Poly[0][0][0]], Data[Poly[0][3][0]]);
 RangeEl = abs(Data[Poly[0][0][0]] - Data[Poly[0][3][0]]);

 Data[Poly[0][1][0]] = MinEl + RangeEl * drand48();
 Data[Poly[0][2][0]] = MinEl + RangeEl * drand48();

 SubDivide(Data, 1, Cols);

 strmfp(filename, DEMPath, DEMFile);
 if ((fOutput = fopen(filename, "w")) == NULL)
  {
  User_Message("Data Ops Module: DEM Create",
	"Error opening file for output!\nOperation terminated.", "OK", "o");
  Log(ERR_OPEN_FAIL, DEMFile);
  goto Cleanup;
  } /* if open fail */
 
 for (i=k=0; i<Rows; i++)
  {
  for (j=0; j<Cols; j++, k++)
   {
   if (fprintf(fOutput, "%d ", Data[k]) < 0)
    {
    error = 1;
    User_Message("Data Ops Module: DEM Create",
	"Error writing to file!\nOperation terminated.", "OK", "o");
    Log(ERR_WRITE_FAIL, DEMFile);
    break;
    }
   } /* for j=0... */
  if (error)
   break;
  fprintf(fOutput, "\n");
  } /* for i=0... */
 fclose(fOutput);

Cleanup:

 if (Data)
  free_Memory(Data, DataSize);

} /* LandscapeGen() */

/**********************************************************************/

void SubDivide(short *Data, long b, long Cols)
{
 short ct, d, ElMin, ElMax, ElRange, NewVal[5];
 long NewRow, NewCol, NewPt[5];

 d = b - 1;

 NewRow = (Poly[d][0][1] + Poly[d][2][1]) / 2;
 NewCol = (Poly[d][0][2] + Poly[d][1][2]) / 2;
 NewPt[4] = NewRow * Cols + NewCol;

 if (Data[NewPt[4]] == DATA_SET_VAL)
  {
  NewPt[0] = Poly[d][0][1] * Cols + NewCol;
  if (Data[NewPt[0]] == DATA_SET_VAL)
   {
   ElMin = min(Data[Poly[d][0][0]], Data[Poly[d][1][0]]);
   ElRange =  abs(Data[Poly[d][0][0]] - Data[Poly[d][1][0]]);

   NewVal[0] = ElMin + ElRange * drand48();
   Data[NewPt[0]] = NewVal[0];
   }
  NewPt[1] = NewRow * Cols + Poly[d][0][2];
  if (Data[NewPt[1]] == DATA_SET_VAL)
   {
   ElMin = min(Data[Poly[d][0][0]], Data[Poly[d][2][0]]);
   ElRange =  abs(Data[Poly[d][0][0]] - Data[Poly[d][2][0]]);

   NewVal[1] = ElMin + ElRange * drand48();
   Data[NewPt[1]] = NewVal[1];
   }
  NewPt[2] = NewRow * Cols + Poly[d][1][2];
  if (Data[NewPt[2]] == DATA_SET_VAL)
   {
   ElMin = min(Data[Poly[d][1][0]], Data[Poly[d][3][0]]);
   ElRange =  abs(Data[Poly[d][1][0]] - Data[Poly[d][3][0]]);

   NewVal[1] = ElMin + ElRange * drand48();
   Data[NewPt[2]] = NewVal[1];
   }
  NewPt[3] = Poly[d][2][1] * Cols + NewCol;
  if (Data[NewPt[3]] == DATA_SET_VAL)
   {
   ElMin = min(Data[Poly[d][2][0]], Data[Poly[d][3][0]]);
   ElRange =  abs(Data[Poly[d][2][0]] - Data[Poly[d][3][0]]);

   NewVal[0] = ElMin + ElRange * drand48();
   Data[NewPt[3]] = NewVal[0];
   }

  ElMin = min(Data[NewPt[0]], Data[NewPt[1]]);
  ElMin = min(ElMin, Data[NewPt[2]]);
  ElMin = min(ElMin, Data[NewPt[3]]);
  ElMax = max(Data[NewPt[0]], Data[NewPt[1]]);
  ElMax = max(ElMax, Data[NewPt[2]]);
  ElMax = max(ElMax, Data[NewPt[3]]);
  ElRange = 2 * (ElMax - ElMin);
  if (ElMin + ElRange > 32767)
   ElRange = 32767 - ElMin;

  NewVal[4] = ElMin + ElRange * drand48();
  Data[NewPt[4]] = NewVal[4];

  for (ct=0; ct<4; ct++)
   {
   switch (ct)
    {
    case 0:
     {
     Poly[b][0][0] = Poly[d][0][0];
     Poly[b][0][1] = Poly[d][0][1];
     Poly[b][0][2] = Poly[d][0][2];

     Poly[b][1][0] = NewPt[0];
     Poly[b][1][1] = Poly[d][0][1];
     Poly[b][1][2] = NewCol;

     Poly[b][2][0] = NewPt[1];
     Poly[b][2][1] = NewRow;
     Poly[b][2][2] = Poly[d][0][2];

     Poly[b][3][0] = NewPt[4];
     Poly[b][3][1] = NewRow;
     Poly[b][3][2] = NewCol;
     break;
     }
    case 1:
     {
     Poly[b][0][0] = NewPt[0];
     Poly[b][0][1] = Poly[d][0][1];
     Poly[b][0][2] = NewCol;

     Poly[b][1][0] = Poly[d][1][0];
     Poly[b][1][1] = Poly[d][1][1];
     Poly[b][1][2] = Poly[d][1][2];

     Poly[b][2][0] = NewPt[4];
     Poly[b][2][1] = NewRow;
     Poly[b][2][2] = NewCol;

     Poly[b][3][0] = NewPt[2];
     Poly[b][3][1] = NewRow;
     Poly[b][3][2] = Poly[d][1][2];
     break;
     }
    case 2:
     {
     Poly[b][0][0] = NewPt[1];
     Poly[b][0][1] = NewRow;
     Poly[b][0][2] = Poly[d][0][2];

     Poly[b][1][0] = NewPt[4];
     Poly[b][1][1] = NewRow;
     Poly[b][1][2] = NewCol;

     Poly[b][2][0] = Poly[d][2][0];
     Poly[b][2][1] = Poly[d][2][1];
     Poly[b][2][2] = Poly[d][2][2];

     Poly[b][3][0] = NewPt[3];
     Poly[b][3][1] = Poly[d][2][1];
     Poly[b][3][2] = NewCol;
     break;
     }
    case 3:
     {
     Poly[b][0][0] = NewPt[4];
     Poly[b][0][1] = NewRow;
     Poly[b][0][2] = NewCol;

     Poly[b][1][0] = NewPt[2];
     Poly[b][1][1] = NewRow;
     Poly[b][1][2] = Poly[d][1][2];

     Poly[b][2][0] = NewPt[3];
     Poly[b][2][1] = Poly[d][2][1];
     Poly[b][2][2] = NewCol;

     Poly[b][3][0] = Poly[d][3][0];
     Poly[b][3][1] = Poly[d][3][1];
     Poly[b][3][2] = Poly[d][3][2];
     break;
     }
    }
   SubDivide(Data, b + 1, Cols);
   } /* for ct=0... */

  } /* if need to interpolate */

} /* SubDivide() */

#endif
/* This copies some header information from one USGS DEM file to another
void FixDEM(void)
{
char Value[200], filename[256], dname1[60], dname2[60], fname1[32], fname2[32];
FILE *fSrc, *fDest;

 fname1[0] = fname2[0] = 0;
 strcpy(dname1, "DH3:Data/7.5MinDEM/Unfiltered");
 strcpy(dname2, "DH3:Data/7.5MinDEM");


 if (! getfilename(0, "source file", dname1, fname1));
 strcpy(fname2, fname1);
 fname2[strlen(fname2) - 3] = 0;
 strcat(fname2, "flt");

 strmfp(filename, dname1, fname1);
 if ((fSrc = fopen(filename, "r")) != NULL)
  {
  strmfp(filename, dname2, fname2);
  if ((fDest = fopen(filename, "r+")) != NULL)
   {
   fseek(fSrc, 546, SEEK_SET);
   fseek(fDest, 546, SEEK_SET);
   fread((char *)Value, 192, 1, fSrc);
   fwrite((char *)Value, 192, 1, fDest);
   fclose(fDest);
   } //if
  else
   printf("error opening destination file\n");
  fclose(fSrc);
  } // if
 else
  printf("error opening source file\n");

} // FixDEM()
*/
