/* DataBase.c (ne gisdatabase.c 14 Jan 1994 CXH)
** Database load/save/edit functions for WCS
** Original code written September, 1992 by Gary R. Huber.
** modified November 11, 1993.
*/

#include "WCS.h"
#include "GUIDefines.h"

#define CUR_DBASE_FIELDS	13
#define CUR_DBASE_RECLNGTH	53

/* Obsolete

short loaddbase(short lowi, short AskName)
{
 char filename[256], Header[24], oldpath[256], oldname[32];
 FILE *fname;
 short i, j, NoOfRecords;

 strcpy(oldpath, dbasepath);
 strcpy(oldname, dbasename);
 if (AskName)
  {
  if (! getdbasename(0))
   return (0);
  } // if new name

 strmfp(filename, dbasepath, dbasename);

 if ((fname = fopen(filename, "r")) == NULL)
  {
  Log(ERR_OPEN_FAIL, dbasename);
  goto RestoreName;
  }
 fgets(Header, 24, fname);
 Header[11] = '\0';
 if (strcmp(Header, "WCSDatabase"))
  {
  fclose(fname);
  User_Message("Database: Load",
	"Not a WCS Database file!\nOperation terminated.", "OK", "o");
  goto RestoreName;
  } // if old database format

 fscanf(fname, "%hd", &NoOfFields);
 if (NoOfFields != 13)
  {
  Log(ERR_WRONG_TYPE, "Unsupported Database file format.");
  fclose(fname);
  goto RestoreName;
  } // if

 fscanf(fname, "%hd%hd", &RecordLength, &NoOfRecords);

 for (i=0; i<=NoOfFields; i++)       // read file fields
  {
  fscanf(fname, "%s", fieldname[i]);
  fscanf(fname, "%hd", &length[i]);
  } // if
 fgets(str, 2, fname);

 if (lowi > 0)
  {
  struct database *NewBase;

  if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
	NoOfObjects + NoOfRecords + 20)) == NULL)
   {
   User_Message("Database Module: Append",
	"Out of memory allocating database!\nOperation terminated.", "OK", "o");
   fclose(fname);
   return (0);
   } // if out of memory
  else
   {
   DBase = NewBase;
   DBaseRecords = NoOfObjects + NoOfRecords + 20;
   } // else new database allocated
  } // if append database
 else
  {
  dbaseloaded = 0;
  DataBase_Del(DBase, DBaseRecords);
  if ((DBase = DataBase_New(NoOfRecords + 20)) == NULL)
   {
   User_Message("Database Module: Load",
	"Out of memory allocating database!\nOperation terminated.", "OK", "o");
   DBaseRecords = 0;
   fclose(fname);
   return (0);
   }
  DBaseRecords = NoOfRecords + 20;
  } // else allocate new database

 for (i=lowi, j=0; i<NoOfRecords+lowi; i++, j++)
  {
  fgets(DBase[i].Name, length[0] + FGETS_KLUDGE, fname);
  DBase[i].Name[length[0]] = 0;
  fgets(DBase[i].Layer1, length[1] + FGETS_KLUDGE, fname);
  DBase[i].Layer1[length[1]] = 0;
  fgets(DBase[i].Layer2, length[2] + FGETS_KLUDGE, fname);
  DBase[i].Layer2[length[2]] = 0;
  fgets(str, length[3] + FGETS_KLUDGE, fname);
  DBase[i].LineWidth = atoi(str);
  fgets(str, length[4] + FGETS_KLUDGE, fname);
  DBase[i].Color = atoi(str);
  fgets(DBase[i].Pattern, length[5] + FGETS_KLUDGE, fname);
  DBase[i].Pattern[length[5]] = 0;
  fgets(DBase[i].Label, length[6] + FGETS_KLUDGE, fname);
  DBase[i].Label[length[6]] = 0;
  fgets(str, length[7] + FGETS_KLUDGE, fname);
  DBase[i].Points = atoi(str);
  fgets(DBase[i].Mark, length[8] + FGETS_KLUDGE, fname);
  DBase[i].Mark[length[8]] = 0;
  if (DBase[i].Mark[0] == 'Y')
   {
   DBase[i].Flags = 6;
   DBase[i].Enabled = '*';
   } // if enabled
  else
   {
   DBase[i].Flags = 0;
   DBase[i].Enabled = ' ';
   j--;
   } // else
  fgets(str, length[9] + FGETS_KLUDGE, fname);
  DBase[i].Red = atoi(str);
  fgets(str, length[10] + FGETS_KLUDGE, fname);
  DBase[i].Grn = atoi(str);
  fgets(str, length[11] + FGETS_KLUDGE, fname);
  DBase[i].Blu = atoi(str);
  fgets(DBase[i].Special, length[12] + FGETS_KLUDGE, fname);
  DBase[i].Special[length[12]] = 0;
  fgets(str, length[13] + FGETS_KLUDGE, fname);
  DBase[i].MaxFract = atoi(str);
  fgets(str, 2, fname);
  } // for

 fclose(fname);
 NoOfObjects = i;
 OBN = 0;

 Log(MSG_DBS_LOAD, dbasename);
 sprintf(str, "Records = %d, Marked = %d", NoOfRecords, j);
 Log(MSG_UTIL_TAB, str); 

 strcpy(str, dbasename);
 strcat(str, ".object");
 if (AskName)
  {
  if (strcmp(str, dirname))
   {
   if (User_Message_Def(str,
	"Make this the default object directory?", "OK|Cancel", "oc", 1))
    {
    strmfp(dirname, dbasepath, str);
    Proj_Mod = 1;
    } // if
   } // if not already default directory
  strmfp(filename, dbasepath, dbasename);
  strcat(filename, ".object");
  if (! DirList_ItemExists(DL, filename))
   {
   if (DL)
    DirList_Add(DL, filename, 0);
   else
    DirList_New(filename, 0);
   Proj_Mod = 1;
   } // if
  } // if ask name

 if (lowi > 0)
  DB_Mod = 1;
 else
  DB_Mod = 0;

 return (1);

RestoreName:
 strcpy(dbasepath, oldpath);
 strcpy(dbasename, oldname);
 return (0);

} // loaddbase()
*/

/*********************************************************************/

short Database_Load(short lowi, char *filename)
{
 char Header[24];
 FILE *fname;
 short i, j, NoOfRecords, error = 0;

 if ((fname = fopen(filename, "r")) != NULL)
  {
  fgets(Header, 24, fname);
  Header[11] = '\0';
  if (! strcmp(Header, "WCSDatabase"))
   {
   fscanf(fname, "%hd", &NoOfFields);
   if (NoOfFields == 13)
    {
    fscanf(fname, "%hd%hd", &RecordLength, &NoOfRecords);

    for (i=0; i<=NoOfFields; i++)       /* read file fields */
     {
     fscanf(fname, "%s", fieldname[i]);
     fscanf(fname, "%hd", &length[i]);
     } /* if */
    fgets(str, 2, fname);

    if (lowi > 0)
     {
     struct database *NewBase;

     if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
	NoOfObjects + NoOfRecords + 20)) == NULL)
      {
      error = 4;
      } /* if out of memory */
     else
      {
      DBase = NewBase;
      DBaseRecords = NoOfObjects + NoOfRecords + 20;
      } /* else new database allocated */
     } /* if append database */
    else
     {
     dbaseloaded = 0;
     DataBase_Del(DBase, DBaseRecords);
     NoOfObjects = 0;
     if ((DBase = DataBase_New(NoOfRecords + 20)) == NULL)
      {
      error = 4;
      }
     DBaseRecords = NoOfRecords + 20;
     } /* else allocate new database */

    if (! error)
     {
     for (i=lowi, j=0; i<NoOfRecords+lowi; i++, j++)
      {
      fgets(DBase[i].Name, length[0] + FGETS_KLUDGE, fname);
      DBase[i].Name[length[0]] = 0;
      fgets(DBase[i].Layer1, length[1] + FGETS_KLUDGE, fname);
      DBase[i].Layer1[length[1]] = 0;
      fgets(DBase[i].Layer2, length[2] + FGETS_KLUDGE, fname);
      DBase[i].Layer2[length[2]] = 0;
      fgets(str, length[3] + FGETS_KLUDGE, fname);
      DBase[i].LineWidth = atoi(str);
      fgets(str, length[4] + FGETS_KLUDGE, fname);
      DBase[i].Color = atoi(str);
      fgets(DBase[i].Pattern, length[5] + FGETS_KLUDGE, fname);
      DBase[i].Pattern[length[5]] = 0;
      fgets(DBase[i].Label, length[6] + FGETS_KLUDGE, fname);
      DBase[i].Label[length[6]] = 0;
      fgets(str, length[7] + FGETS_KLUDGE, fname);
      DBase[i].Points = atoi(str);
      fgets(DBase[i].Mark, length[8] + FGETS_KLUDGE, fname);
      DBase[i].Mark[length[8]] = 0;
      if (DBase[i].Mark[0] == 'Y')
       {
       DBase[i].Flags = 6;
       DBase[i].Enabled = '*';
       } /* if enabled */
      else
       {
       DBase[i].Flags = 0;
       DBase[i].Enabled = ' ';
       j--;
       } /* else */
      fgets(str, length[9] + FGETS_KLUDGE, fname);
      DBase[i].Red = atoi(str);
      fgets(str, length[10] + FGETS_KLUDGE, fname);
      DBase[i].Grn = atoi(str);
      fgets(str, length[11] + FGETS_KLUDGE, fname);
      DBase[i].Blu = atoi(str);
      fgets(DBase[i].Special, length[12] + FGETS_KLUDGE, fname);
      DBase[i].Special[length[12]] = 0;
      fgets(str, length[13] + FGETS_KLUDGE, fname);
      DBase[i].MaxFract = atoi(str);
      fgets(str, 2, fname);
      } /* for i=lowi... */
     NoOfObjects = i;
     OBN = 0;
     Log(MSG_DBS_LOAD, dbasename);
     sprintf(str, "Records = %d, Marked = %d", NoOfRecords, j);
     Log(MSG_UTIL_TAB, str); 
     } /* if no memory error */
    }
   else
    {
    error = 2;
    } /* else incorrect number of database fields */
   }
  else
   {
   error = 2;
   } /* if old database format */
  fclose(fname);
  }
 else
  {
  error = 1;
  } /* if no file to open */

 return (error);

} /* Database_Load() */

/*********************************************************************/

short makedbase(short SaveNewDBase)
{
 char tempname[64], temppath[255];
 short TempObjects, TempRecords;
 struct database *TempBase;

 TempObjects = NoOfObjects;
 TempRecords = DBaseRecords;
 TempBase = DBase;
 strcpy(temppath, dbasepath);
 strcpy(tempname, dbasename);
 NoOfObjects = 0;
 NoOfFields = CUR_DBASE_FIELDS;
 RecordLength = CUR_DBASE_RECLNGTH;
 DBaseRecords = 20;
 strcpy(fieldname[0], "Name");
 length[0] = 10;
 strcpy(fieldname[1], "Layer1");
 length[1] = 2;
 strcpy(fieldname[2], "Layer2");
 length[2] = 3;
 strcpy(fieldname[3], "LineWidth");
 length[3] = 1;
 strcpy(fieldname[4], "Color");
 length[4] = 2;
 strcpy(fieldname[5], "Pattern");
 length[5] = 1;
 strcpy(fieldname[6], "Label");
 length[6] = 15;
 strcpy(fieldname[7], "Points");
 length[7] = 5;
 strcpy(fieldname[8], "Mark");
 length[8] = 1;
 strcpy(fieldname[9], "Red");
 length[9] = 3;
 strcpy(fieldname[10], "Grn");
 length[10] = 3;
 strcpy(fieldname[11], "Blu");
 length[11] = 3;
 strcpy(fieldname[12], "Class");
 length[12] = 3;
 strcpy(fieldname[13], "MaxFract");
 length[13] = 1;

 if ((DBase = DataBase_New(20)) == NULL)
  goto CancelCreate;
 if (SaveNewDBase)
  {
  if (savedbase(1))
   {
   goto CancelCreate;
   } /* if save new database failed */
  } /* if save */

 DataBase_Del(TempBase, TempRecords);

 sprintf(str, "New database created: %s", dbasename);
 Log(MSG_NULL, str);
 return (1);

CancelCreate:
 if (DBase)
  DataBase_Del(DBase, DBaseRecords);
 DBase = TempBase;
 DBaseRecords = TempRecords;
 NoOfObjects = TempObjects;
 strcpy(dbasename, tempname);
 strcpy(dbasepath, temppath);
 return (dbaseloaded);

} /* makedbase() */

/*********************************************************************/

short savedbase(short askname)
{
 char filename[255], oldpath[255], oldname[64];
 FILE *fname;
 short i, error = 0;
 
 strcpy(oldpath, dbasepath);
 strcpy(oldname, dbasename);

RepeatSave:
 if(askname)
  {
  if (! getdbasename(1))
   return (1);
  } /* if */

 strmfp(filename, dbasepath, dbasename);

#ifdef DATABASE_OVERWRITE_QUERY
 if(askname)
  { /* Of course we'll overwrite. */
  if ((fname = fopen(filename, "r")) != 0)
   {
   fclose(fname);
   if (! FileExists_Message(dbasename))
    {
    strcpy(dbasepath, oldpath);
    strcpy(dbasename, oldname);
    return (1);
    } /* if not overwrite */
   } /* if open succeeds */
  } /* if */
#endif

 if ((fname = fopen(filename,"w")) == NULL)
  {
  Log(ERR_OPEN_FAIL, dbasename);
  strcpy(dbasepath, oldpath);
  strcpy(dbasename, oldname);
  return (1);
  } /* if open fails */
 fprintf(fname, "%s\n", "WCSDatabase");
 fprintf(fname, "%hd\n%hd\n%hd\n", NoOfFields, RecordLength, NoOfObjects);
 for (i=0; i<=NoOfFields; i++)           /* write file fields */
  {
  fprintf(fname, "%s\n", fieldname[i]);
  fprintf(fname, "%hd\n", length[i]);
  } /* for i=0... */
 for (i=0; i<NoOfObjects && error>=0; i++)
  {
  fputs(DBase[i].Name, fname);
  fputs(DBase[i].Layer1, fname);
  fputs(DBase[i].Layer2, fname);
  fprintf(fname, "%1.1hd", DBase[i].LineWidth);
  fprintf(fname, "%-2.2hd", DBase[i].Color);
  fputs(DBase[i].Pattern, fname);
  fputs(DBase[i].Label, fname);
  fprintf(fname, "%-5.5hd", DBase[i].Points);
  fputs(DBase[i].Mark, fname);
  fprintf(fname, "%-3.3hd", DBase[i].Red);
  fprintf(fname, "%-3.3hd", DBase[i].Grn);
  fprintf(fname, "%-3.3hd", DBase[i].Blu);
  fputs(DBase[i].Special, fname);
  error = fprintf(fname, "%1.1hd", DBase[i].MaxFract);
  fputc(10, fname);
  } /* for i=0... */
 fclose(fname);

 if (error < 0)
  {
  if (User_Message(dbasename, "Error saving database!\nSelect a new directory?",
		"OK|CANCEL", "oc"))
   {
   error = 0;
   askname = 1;
   goto RepeatSave;
   } /* if */
  }

 if (error < 0)
  {
  Log(ERR_WRITE_FAIL, "Database");
  strcpy(dbasepath, oldpath);
  strcpy(dbasename, oldname);
  return (1);
  }
 else
  {
  Log(MSG_DBS_SAVE, dbasename);
  sprintf(str, "Number of objects = %d", NoOfObjects);
  Log(MSG_UTIL_TAB, str);
  } /* else no save error */
 strmfp(filename, dbasepath, dbasename);
 strcat(filename, ".object");
 if (! Mkdir(filename))
  {
  sprintf(str, "Directory Created: %s", filename);
  Log(MSG_NULL,	str);
  sprintf(str, "New directory created: %s. Make it the default directory?", filename);
  if (User_Message_Def("Database Module: Save", str, "OK|Cancel", "oc", 1))
   strcpy(dirname, filename);
  } /* if no error making directory (as in "already exists") */
 if (! DirList_ItemExists(DL, filename))
  {
  DirList_Add(DL, filename, 0);
  Proj_Mod = 1;
  }
 if (DL_Win)
  {
  Update_DL_Win();
  } /* if Dir List window open */

 DB_Mod = 0;

 return(0);

} /* savedbase() */

/*********************************************************************/

struct database *DataBase_New(short Records)
{

 if (Records > 0)
  {
  return ((struct database *)get_Memory(Records * sizeof (struct database),
	MEMF_CLEAR));
  } /* if */

 User_Message("Database Module",
	"Illegal number of database records: less than one!\nOperation terminated.",
	"OK", "o");
 return ((struct database *)NULL);

} /* DataBase_New() */

/*********************************************************************/

struct database *DataBase_Copy(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords)
{
 struct database *NewBase = NULL;

 if (NewRecords > 0)
  {
  if ((NewBase = DataBase_New(NewRecords)) != NULL)
   {
   if (OldBase)
    {
    if (OldUsedRecords > 0)
     {
     memcpy(NewBase, OldBase, OldUsedRecords * sizeof (struct database));
     } /* if number of records to copy > 0 */
    free_Memory(OldBase, OldRecords * sizeof (struct database));
    } /* if old database exists */
   } /* if new database allocated */
  } /* if NewRecords > 0 */

 return (NewBase);

} /* DataBase_Copy() */

/*********************************************************************/

struct database *DataBase_Expand(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords)
{
struct database *NewBase = NULL;
long i;

 if ((NewBase = DataBase_Copy(OldBase, OldRecords,
	OldUsedRecords, NewRecords)) != NULL)
  {
  if (DE_Win)
   {
   if (DBList_New(NewRecords))
    {
    for (i=0; i<NoOfObjects; i++)
     {
     DE_Win->DBName[i] = &DBase[i].Enabled;
     } /* for i=0... */
    DE_Win->DBName[NoOfObjects] = NULL;
    }
   else
    {
    User_Message("Database Module",
	"Out of memory!\nCan't update database list.", "OK", "o");
    } /* else */
   } /* if */
  } /* if */

 return (NewBase);

} /* DataBase_Expand */

/*********************************************************************/

void DataBase_Del(struct database *DelBase, short Records)
{

 if (DelBase)
  {
  if (Records > 0)
   {
   if (FreeAllVectors())
    savedbase(1);
   free_Memory(DelBase, Records * sizeof (struct database));
   } /* if */
  } /* if database exists */

} /* DataBase_Del() */

/*********************************************************************/

short loadmapbase(short lowi, short onlyselected)
{
 char *LastDir = NULL;
 short loaderror, i, LoadedObjects = 0, Warn = 0;
 struct BusyWindow *BusyLoad;

 if (lowi == 0)
  {
  for (i=0; i<NoOfObjects; i++)
   freevecarray(i);
#ifdef DBASE_SAVE_COMPOSITE
  if (DBase_LoadComposite() > 0)
   {
   LoadedObjects = NoOfObjects;
   goto EndLoad;
   } /* if objects loaded OK from composite file */
#endif /* DBASE_SAVE_COMPOSITE */
  } /* if load new database */

 BusyLoad = BusyWin_New("Vector Load", NoOfObjects - lowi, 0, 'MVLD');
 for (i=lowi; i<NoOfObjects; i++)
  {
  if (DBase[i].Lat)
   {
   LoadedObjects ++;
   goto SkipIt;
   } /* if loaded from master file */
  if (onlyselected && ! (DBase[i].Flags & 2)) goto SkipIt;
  if ((loaderror = Load_Object(i, &LastDir)) == 1) goto SkipIt;
  if (loaderror == 2) break;
  if (loaderror == -1)
   Warn = 1;
  LoadedObjects ++;

SkipIt:

  if (CheckInput_ID() == ID_BW_CLOSE)
   break;
  if(BusyLoad)
  	{
  	BusyWin_Update(BusyLoad, i - lowi);
  	} /* if */
  } /* for i=lowi... */
 if(BusyLoad)
  {
  BusyWin_Del(BusyLoad);
  } /* if */

EndLoad:

 OBN = 0;

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
   } /* if */
  if (TopoOBN) free_Memory(TopoOBN, TopoOBNSize);
  TopoOBN = NULL;
  if (mapcoords) free_Memory(mapcoords, MapCoordSize);
  mapcoords = NULL;
  topoload = 0;
  topomaps = 0;
  } /* if topos loaded previously */

 sprintf(str, "%d objects", LoadedObjects);
 Log(MSG_VCS_LOAD, str);


 if (Warn)
  {
  sprintf(str, "At least one vector file was found to contain a number of\
 points different from that in its Database record!\nThe record has been\
 updated.\nDatabase should be re-saved.");
  User_Message("Map View: Load", str, "OK", "o");
  } /* if */

 return (readmapprefs());

} /* loadmapbase() */

/*********************************************************************/

short Load_Object(short i, char **LastDir)
{
 char filename[255], filetype[10];
 short OpenOK = 0, ReadOK = 0, PtError = 0;
 long extrapts, Size;
 float Version;
 double dummy;
 FILE *fobject;
 struct vectorheaderV100 Hdr;
 struct DirList *DLItem;

RepeatLoad:
 if (DBase[i].Lat || DBase[i].Lon)
  freevecarray(i);

 DBase[i].Flags &= (255 ^ 1);			/* modification status */

 if (LastDir)
  {
  if (*LastDir)
   {
   strmfp(filename, *LastDir, DBase[i].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "rb")) != NULL)
    OpenOK = 1;
   } /* if */
  } /* if */

 if (! OpenOK)
  {
  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, DBase[i].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    } /* if open fails */
   else
    {
    OpenOK = 1;
    *LastDir = DLItem->Name;
    break;
    } /* else open succeeds */
   } /* while */
  } /* else */

 if (! OpenOK)
  {
  Log(WNG_OPEN_FAIL, DBase[i].Name);
/*  DBase[i].Flags &= (255 ^ 2);*/			/* enabled status */
  return (1);
  } /* if */

 fread((char *)filetype, 9, 1, fobject);
 filetype[9] = 0;
 if (! strcmp(filetype, "WCSVector"))
  {
  fread((char *)&Version, sizeof (float), 1, fobject);
  if (abs(Version - 1.00) < .001)
   {
   if (fread((char *)&Hdr, sizeof (struct vectorheaderV100), 1, fobject) == 1)
    {
    if (Hdr.points != DBase[i].Points)
     {
     PtError = -1;
     DBase[i].Points = Hdr.points;
     } /* if wrong number of points */
    Size = (Hdr.points + 1) * sizeof (double);
    if (allocvecarray(i, Hdr.points, 1))
     {
     if (fread((char *)&DBase[i].Lon[0], Size, 1, fobject) == 1)
      {
      if (fread((char *)&DBase[i].Lat[0], Size, 1, fobject) == 1)
       {
       if (fread((char *)&DBase[i].Elev[0], Size / 4, 1, fobject) == 1)
        {
        ReadOK = 1;
	} /* if elevation read */
       else
        User_Message(DBase[i].Name, "Error reading elevations! Object not loaded.",
		"OK", "o");
       } /* if latitude read */
      else
       User_Message(DBase[i].Name, "Error reading latitudes! Object not loaded.",
	"OK", "o");
      } /* if longitude read */
     else
      User_Message(DBase[i].Name, "Error reading longitudes! Object not loaded.",
	"OK", "o");
     } /* if memory allocated */
    else
     User_Message(DBase[i].Name, "Out of memory! Object not loaded.",
	"OK", "o");
    } /* if header read */
   else
    User_Message(DBase[i].Name, "Error reading header! Object not loaded.",
	"OK", "o");
   } /* if version 1.0 */
  else
   User_Message(DBase[i].Name, "Unsupported file version! Object not loaded.",
	"OK", "o");
  if (! ReadOK)
   {
   fclose(fobject);
   freevecarray(i);
   Log(WNG_READ_FAIL, DBase[i].Name);
   return (1);
   } /* if read error */
  } /* if binary file format */

 else
  {
  fclose(fobject);
  if ((fobject = fopen(filename,"r")) == NULL)
   {
   Log(WNG_OPEN_FAIL, DBase[i].Name);
   return (1);
   } /* if */

  if (! allocvecarray(i, DBase[i].Points, 1))
   {
   User_Message(DBase[i].Name, "Out of memory! Object not loaded.",
	"OK", "o");
   fclose(fobject);
   return (2);
   } /* if out of memory */

  for (pts=0; pts<=DBase[i].Points; pts++)
   {
   if (fscanf(fobject, "%le%le", &DBase[i].Lon[pts], &DBase[i].Lat[pts]) != 2)
    {
    PtError = -1;
    DBase[i].Points = pts - 1;
    break;
    } /* if unexpected end of file */
   } /* for pts=0... */
  if (fscanf(fobject, "%le%le", &dummy, &dummy) == 2)
   {
   extrapts = 1;
   while (fscanf(fobject, "%le%le", &dummy, &dummy) == 2)
    {
    extrapts ++;
    }
   fclose(fobject);
   DBase[i].Points += extrapts;
   PtError = -1;
   goto RepeatLoad;
   } /* if stored object is larger than database record indicates */
  } /* else not binary file */  

 fclose(fobject);

 if (DBase[i].Mark[0] == 'Y') DBase[i].Flags |= 2;
 else DBase[i].Flags &= (255 ^ 2);

 return (PtError);

} /* Load_Object() */

/*********************************************************************/

short Find_DBObjPts(char *filename)
{
 char filetype[10];
 short Points = 0;
 float Version;
 double dummy;
 FILE *fobject;
 struct vectorheaderV100 Hdr;

 if ((fobject = fopen(filename,"rb")) != NULL)
  {
  if (fread((char *)filetype, 9, 1, fobject) == 1)
   {
   filetype[9] = 0;
   if (! strcmp(filetype, "WCSVector"))
    {
    if (fread((char *)&Version, sizeof (float), 1, fobject) == 1)
     {
     if (abs(Version - 1.00) < .001)
      {
      if (fread((char *)&Hdr, sizeof (struct vectorheaderV100), 1, fobject) == 1)
       Points = Hdr.points;
      } /* if version 1.0 */
     } /* if header read OK */
    } /* if WCS Vector format */
   else
    {
    fclose(fobject);
    if ((fobject = fopen(filename, "r")) == NULL)
     {
     return (0);
     } /* if */ 

    for (Points=0; ;Points++)
     {
     if (fscanf(fobject, "%le%le", &dummy, &dummy) != 2)
      {
      break;
      } /* if end of file */
     } /* for Points=0... */

    } /* else not binary file */
   } /* if file type read OK */
  } /* if file opened OK */

 fclose(fobject);

 return (Points);

} /* Find_DBObjPts() */

/*********************************************************************/

struct DirList *DirList_New(char *firstpath, short ReadOnly)
{
 struct DirList *DLNew;

 if ((DLNew = get_Memory(sizeof (struct DirList), MEMF_CLEAR)) != NULL)
  {
  strcpy(DLNew->Name, firstpath);
  if (ReadOnly)
   DLNew->Read = '*';
  else
   DLNew->Read = ' ';
  } /* if memory OK */

 return (DLNew);
 
} /* DirList_New() */

/*********************************************************************/

short DirList_Add(struct DirList *DLOld, char *addpath, short ReadOnly)
{
 struct DirList *DLNew;
 char ObjPath[256] = {0}, ObjFile[32] = {0};

 if (! DLOld) return (0);

 while (DLOld->Next)
  {
  DLOld = DLOld->Next;
  } /* while */

 if (! addpath)
  {
  strcpy(ObjPath, "WCSProjects:");
  if (! getfilename(0, "Object Directory", ObjPath, ObjFile))
   return (0);
  } /* if no path provided */
 else
  {
  strcpy(ObjPath, addpath);
  } /* else path provided */

 if ((DLNew = get_Memory(sizeof (struct DirList), MEMF_CLEAR)) != NULL)
  {
  strcpy(DLNew->Name, ObjPath);
  DLOld->Next = DLNew;
  if (ReadOnly)
   DLNew->Read = '*';
  else
   DLNew->Read = ' ';
  return (1);
  } /* if memory OK */

 return (0);

} /* DirList_Add() */

/*********************************************************************/

struct DirList *DirList_Remove(struct DirList *DLOld, short Item)
{
 struct DirList *DLRem, *DLFirst;

 DLFirst = DLOld;
 if (Item > 0)
  {
  DLOld = DirList_Search(DLOld, Item - 1);
  DLRem = DLOld->Next;
  DLOld->Next = DLRem->Next;
  }
 else
  {
  DLRem = DLOld;
  if (! DLRem->Next)
   {
   return (DLFirst);
   } /* only one item in list-can't be deleted */
  DLFirst = DLRem->Next;
  }

 free_Memory(DLRem, sizeof (struct DirList));

 return (DLFirst);

} /* DirList_Remove() */

/*********************************************************************/

struct DirList *DirList_Search(struct DirList *DLItem, short Item)
{
 short i;

 for (i=0; i<Item; i++)
  {
  DLItem = DLItem->Next;
  } /* for i=0... */

 return (DLItem);

} /* DirList_Search() */

/*********************************************************************/

short DirList_ItemExists(struct DirList *DLItem, char *Item)
{
 short found = 0;

 while (DLItem)
  {
  if (! strcmp(DLItem->Name, Item))
   {
   found = 1;
   break;
   }
  DLItem = DLItem->Next;
  } /* while */

 return (found);

} /* DirList_ItemExists() */

/*********************************************************************/

void DirList_Move(struct DirList *DLOld, short Move, short MoveTo)
{
 short i;
 struct DirList *DLItem1, *DLItem2;

 if (Move == MoveTo) return;

 if (MoveTo > Move)
  {
  DLItem1 = DirList_Search(DLOld, Move);
  DLItem2 = DLItem1->Next;
  for (i=Move; i<MoveTo-1; i++)
   {
   swmem(&DLItem1->Read, &DLItem2->Read, 256);
   DLItem1 = DLItem2;
   DLItem2 = DLItem2->Next;
   } /* for i=... */
  } /* if move<moveto */
 else
  {
  for (i=Move; i>MoveTo; i--)
   {
   DLItem1 = DirList_Search(DLOld, i - 1);
   DLItem2 = DLItem1->Next;
   swmem(&DLItem1->Read, &DLItem2->Read, 256);
   } /* for i=... */
  } /* move>moveto */

} /* DirList_Move() */

/*********************************************************************/

void DirList_Del(struct DirList *DLDel)
{
 struct DirList *DLNext;

 while (DLDel)
  {
  DLNext = DLDel->Next;
  free_Memory(DLDel, sizeof (struct DirList));
  DLDel = DLNext;
  } /* while */

} /* DirList_Del() */

/*********************************************************************/

struct DirList *DirList_Copy(struct DirList *DLOld)
{
 struct DirList *DLCopy, *DLItem;

 if (! DLOld) return (NULL);

 if ((DLCopy = get_Memory(sizeof (struct DirList), MEMF_CLEAR)) != NULL)
  {
  DLItem = DLCopy;
  while (DLOld)
   {
   strcpy(&DLItem->Read, &DLOld->Read);
   DLOld = DLOld->Next;
   if (DLOld)
    {
    if ((DLItem->Next = get_Memory(sizeof (struct DirList), MEMF_CLEAR)) != NULL)
     {
     DLItem = DLItem->Next;
     }
    else
     {
     break;
     } /* else memory failed */
    } /* if another to copy */
   } /* while */
  } /* if memory OK */

 return (DLCopy);

} /* DirList_Copy() */

/***********************************************************************/

short DBaseObject_New(void)
{
 short i, found;
 struct database *NewBase;

 str[0] = 0;
NewName2:
 found = 0;
 if (! GetInputString("Enter new object name.", ":;*/?`#%", str))
  return (0);
 while (strlen(str) < length[0])
  strcat(str, " ");
 for (i=0; i<NoOfObjects; i++)
  {
  if (! strnicmp(str, DBase[i].Name, length[0]))
   {
   found = 1;
   break;
   } /* if name already in use */
  } /* for i=0... */
 if (found)
  {
  if (! User_Message_Def("Database Module: Name",
	"Vector name already present in database!\nTry a new name?",
		"OK|Cancel", "oc", 1))
   return (0);
  goto NewName2;
  }
 if (NoOfObjects + 1 > DBaseRecords)
  {
  if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
   {
   User_Message("Database Module",
	"Out of memory expanding database!\nOperation terminated.", "OK", "o");
   return (0);
   } /* if new database allocation fails */
  else
   {
   DBase = NewBase;
   DBaseRecords += 20;
   } /* else new database allocated and copied */
  } /* if need larger database memory */
 memcpy(&DBase[NoOfObjects], &DBase[OBN], sizeof (struct database));
 OBN = NoOfObjects;
 NoOfObjects ++;
 DBase[OBN].Lat = DBase[OBN].Lon = NULL;
 DBase[OBN].Elev = NULL;
 DBase[OBN].VecArraySize = 0;
 strncpy(DBase[OBN].Name, str, length[0]);
 strcpy(DBase[OBN].Special, "VEC");
 DBase[OBN].Name[length[0]] = 0;
 DBase[OBN].Points = 0;
 DBase[OBN].Mark[0] = 'Y';
 DBase[OBN].Enabled = '*';
 DBase[OBN].Flags = 7;
 while (strlen(DBase[OBN].Layer1) < length[1])
  strcat(DBase[OBN].Layer1, " ");
 while (strlen(DBase[OBN].Layer2) < length[2])
  strcat(DBase[OBN].Layer2, " ");
 while (strlen(DBase[OBN].Label) < length[6])
  strcat(DBase[OBN].Label, " ");
 if (allocvecarray(OBN, 1, 1))
  {
  DBase[OBN].Points = 1;
  if (MapWind0 && mapscale != 0.0)
   {
   struct clipbounds cb;

   setclipbounds(MapWind0, &cb);
   mapxx[0] = mapxx[1] = MapWind0->Width / 2;
   mapyy[0] = mapyy[1] = MapWind0->Height / 2;
   XY_latlon(OBN, 0, 1);
   outline(MapWind0, OBN, 2, &cb);
   } /* if map module open */
  else
   {
   DBase[OBN].Lat[0] = DBase[OBN].Lat[1] = PAR_FIRST_MOTION(4);
   DBase[OBN].Lon[0] = DBase[OBN].Lon[1] = PAR_FIRST_MOTION(5);
   DBase[OBN].Elev[0] = DBase[OBN].Elev[1] = 0;
   } /* else not map window not open */
  } /* if memory allocated for lat/lon arrays */
 else
  {
  User_Message("Database Module: Editor",
	"No memory for vector coordinates!\nNew object has been created\
 but can not be edited until memory is available.",
	"OK", "o");
  } /* else no memory, not even a little bit! */
 if (DE_Win)
  {
  if (! Add_DE_NewItem())
   {
   User_Message("Database Module: Editor",
	"Out of memory expanding Database Editor List!\nNew object has been created but will not appear in list view.",
	"OK", "o");
   } /* if no new list item */
  else
   {
   set(DE_Win->DatabaseEditWin, MUIA_Window_ActiveObject, DE_Win->Str[0]);
   } /* else memory OK */
  } /* if database editor open */

 DB_Mod = 1;

 return (1);

} /* DBaseObject_New() */

/*********************************************************************/

short DBaseObject_Add(void)
{
 char *pattern = "#?.Obj";
 char newfile[64], newpath[256], extension[32], filename[256];
 short i, j, found, warned = 0, NumPts;
 struct FileRequester *FrFile;
 struct database *NewBase;
 FILE *felev;

 strcpy(newpath, dirname);
 newfile[0] = 0;
 if (!(FrFile = getmultifilename("Add Object", newpath, newfile, pattern)))
  return (0);

 if (FrFile->rf_NumArgs > 0)
  {
  if (! DirList_ItemExists(DL, newpath))
   {
   DirList_Add(DL, newpath, 0);
   Proj_Mod = 1;
   } /* if new directory */
  if (DL_Win)
   {
   Update_DL_Win();
   } /* if Dir List window open */
  } /* if */

 for (i=0; i<FrFile->rf_NumArgs; i++)
  {
  strcpy(newfile, FrFile->rf_ArgList[i].wa_Name);
  
  if (newfile[0] == NULL)
   {
   User_Message("Database: Add Object",
	"No file(s) selected!",	"OK", "o");
   break;
   } /* if no file */

  strsfn(newfile, NULL, NULL, str, extension);
  if (strcmp(extension, "Obj"))
   {
   Log(ERR_WRONG_TYPE, newfile);
   User_Message(newfile, "Object must end in suffix \"Obj\"!", "OK", "o");
   break;
   } /* if wrong file type */

  str[length[0]] = '\0';
  while (strlen(str) < length[0])
   strcat(str, " ");

/* check to see if object is already in database */
  for (j=found=0; j<NoOfObjects && !found; j++)
   {
   if (! strnicmp(str, DBase[j].Name, length[0]))
    {
    found = 1;
    } /* if name already in use */
   } /* for j=0... */
  if (found && ! warned)
   {
   User_Message_Def(str,
	"Object name already present in database!\nDuplicate items will be skipped.",
	"OK", "o", 1);
   warned = 1;
   continue;
   } /* if duplicate object name */

  strmfp(filename, newpath, newfile);
  NumPts = Find_DBObjPts(filename);

  if (NoOfObjects + 1 > DBaseRecords)
   {
   if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
    {
    User_Message("Database Module",
	"Out of memory expanding database!\nOperation terminated.", "OK", "o");
    break;
    } /* if new database allocation fails */
   else
    {
    DBase = NewBase;
    DBaseRecords += 20;
    } /* else new database allocated and copied */
   } /* if need larger database memory */
  memcpy(&DBase[NoOfObjects], &DBase[OBN], sizeof (struct database));
  OBN = NoOfObjects;
  NoOfObjects ++;
  DBase[OBN].Lat = DBase[OBN].Lon = NULL;
  DBase[OBN].Elev = NULL;
  DBase[OBN].VecArraySize = 0;
  strncpy(DBase[OBN].Name, str, length[0]);
  DBase[OBN].Name[length[0]] = 0;
  DBase[OBN].Points = NumPts;
  DBase[OBN].Mark[0] = 'Y';
  DBase[OBN].Enabled = '*';
  DBase[OBN].LineWidth = 1;
  DBase[OBN].MaxFract = 9;
  DBase[OBN].Flags = 6;
  strcpy(DBase[OBN].Pattern, "L");
  strncpy(DBase[OBN].Layer1, "       ", length[1]);
  strncpy(DBase[OBN].Layer2, "       ", length[2]);
  strncpy(DBase[OBN].Label, "                       ", length[6]);
  DBase[OBN].Layer1[length[1]] = DBase[OBN].Layer2[length[2]]
	 = DBase[OBN].Label[length[6]] = 0;
/* check to see if there is an elev file - DEM object */
  newfile[length[0]] = 0;
  strcat(newfile, ".elev");
  strmfp(filename, newpath, newfile);
  if ((felev = fopen(filename, "rb")) == NULL)
   {
   strcpy(DBase[OBN].Special, "VEC");
   DBase[OBN].Color = 3;
   }
  else
   {
   strcpy(DBase[OBN].Special, "TOP");
   fclose(felev);
   DBase[OBN].Color = 2;
   } /* else DEM object */
  if (DE_Win)
   {
   if (! Add_DE_NewItem())
    {
    User_Message("Database Module: Editor",
	"Out of memory expanding Database Editor List!\nNew object has been added but will not appear in list view.",
	"OK", "o");
    } /* if no new list item */
   Map_DB_Object(OBN, 1, 0);
   } /* if database editor open */

  DB_Mod = 1;

  } /* for i=0... */

 freemultifilename(FrFile);

 return (1);

} /* DBaseObject_Add() */

/**********************************************************************/

#ifndef USE_GIS_GUI

short eddbase(void)
{
 char num[32];
 short menuitem, abort = 0;

 do {
  printf("\n********** WCS Database Editing: DataBase \"%s\" **********\n\n",
		 dbasename);
  printf("  0. Set Mark Status\n");
  printf("  1. Edit Database Entries\n");
  printf("  2. Delete Database Entries\n");
  printf("200. Return to Database Menu\n");
  printf("100. Exit WCS\n\n");

  printf("Your Choice: ");
  scanf("%s", &num);
  menuitem = atoi(num);

  switch (menuitem)
   {
   case 0:
    editmarkstatus();
    break;
   case 1:
    editdbaseitems();
    break;
   case 2:
    deletedbaseitems();
    break;
   case 200:
    return (0);
    break;
   case 100:
    return (1);
    break;
   } /* switch menuitem */

  } while (! abort);

 return (abort);

} /* eddbase() */  

/*********************************************************************/

void editmarkstatus(void)
{
 char num[32];
 short i;

 do
  {
  for (i=0; i<NoOfObjects; i +=4)
   {
   printf("%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s\n", 
	i, DBase[i].Name, DBase[i].Mark, 
	i + 1, DBase[i + 1].Name, DBase[i + 1].Mark, 
	i + 2, DBase[i + 2].Name, DBase[i + 2].Mark, 
	i + 3, DBase[i + 3].Name, DBase[i + 3].Mark); 
   } /* for i=0... */
  printf("\n");

  do
   {
   printf("Item to change (-1 to quit, > to repeat list): ");
   scanf("%s", &num);
   if (num[0] == '>') break;
   i = atoi(num);
   if (i >= 0 && i < NoOfObjects)
    {
    if (DBase[i].Mark[0] == 'Y') DBase[i].Mark[0] = 'N';
    else if (DBase[i].Mark[0] == 'N') DBase[i].Mark[0] = 'Y';
    } /* if */

   } while (i>=0);

  } while (i>=0);

} /* editmarkstatus() */

/*********************************************************************/

void editdbaseitems(void)
{
 char num[32], *c;
 short i, j;

 do
  {
  for (i=0; i<NoOfObjects; i +=4)
   {
   printf("%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s\n", 
	i, DBase[i].Name, DBase[i].Mark, 
	i + 1, DBase[i + 1].Name, DBase[i + 1].Mark, 
	i + 2, DBase[i + 2].Name, DBase[i + 2].Mark, 
	i + 3, DBase[i + 3].Name, DBase[i + 3].Mark); 
   } /* for i=0... */
  printf("\n");

  do
   {
   printf("\nItem to edit (-1 to quit, > to repeat list): ");
   scanf("%s", &num);
   if (num[0] == '>') break;
   i = atoi(num);

   if (i >= 0 && i < NoOfObjects)
    {
    do
     {
     printf("0. Name: %s		1. Layer1: %s\n",
	DBase[i].Name, DBase[i].Layer1);
     printf("2. Layer2: %s			3. LineWidth: %hd\n",
	DBase[i].Layer2, DBase[i].LineWidth);
     printf("4. Color: %hd			5. Pattern: %s\n",
	DBase[i].Color, DBase[i].Pattern);
     printf("6. Label: %s	7. Points: %hd\n",
	DBase[i].Label, DBase[i].Points);
     printf("8. Mark: %s\n\n", DBase[i].Mark);

     printf("Item to Modify (-1 to quit): ");
     scanf("%s", &num);
     j = atoi(num);

     switch (j)
      {
      case 0:
       {
       printf("WARNING: Modifying the name of an object may be hazardous\n");
       printf("  to your program's health!!!  Proceed? (1= Yes, 0= No): ");
       scanf("%s", &str);
       if (atoi(str) == 1)
        {
        printf("Name: <%s> ", DBase[i].Name);
        scanf("%s", &str);
        strncpy(DBase[i].Name, str, length[0]);
        DBase[i].Name[length[0]] = 0;
        } /* if */
       break;
       }
      case 1:
       {
       printf("Layer1: <%s> ", DBase[i].Layer1);
       scanf("%s", &str);
       strncpy(DBase[i].Layer1, str, length[1]);
       DBase[i].Layer1[length[1]] = 0;
       c = &DBase[i].Layer1[0];
       while ( *c ) {
        *c = toupper(*c);
        c++;
       } /* while */
       break;
       }
      case 2:
       {
       printf("Layer2: <%s> ", DBase[i].Layer2);
       scanf("%s", &str);
       strncpy(DBase[i].Layer2, str, length[2]);
       DBase[i].Layer2[length[2]] = 0;
       c = &DBase[i].Layer2[0];
       while ( *c )
        {
        *c = toupper(*c);
        c++;
        } /* while */
       break;
       }
      case 3:
       {
       printf("Line Width: <%hd> ", DBase[i].LineWidth);
       scanf("%s", &str);
       DBase[i].LineWidth = atoi(str);
       break;
       }
      case 4:
       {
       printf("Color: <%hd> ", DBase[i].Color);
       scanf("%s", &str);
       DBase[i].Color = atoi(str);
       break;
       }
      case 5:
       {
       printf("Pattern: <%s> ", DBase[i].Pattern);
       scanf("%s", &str);
       strncpy(DBase[i].Pattern, str, length[5]);
       DBase[i].Pattern[length[5]] = 0;
       c = &DBase[i].Pattern[0];
       while ( *c )
        {
        *c = toupper(*c);
        c++;
        } /* while */
       break;
       }
      case 6:
       {
       printf("Label: <%s> ", DBase[i].Label);
       scanf("%s", &str);
       strncpy(DBase[i].Label, str, length[6]);
       DBase[i].Label[length[6]] = 0;
       c = &DBase[i].Label[0];
       while ( *c )
        {
        *c = toupper(*c);
        c++;
        } /* while */
       break;
       }
      case 7:
       {
       printf("WARNING: Modifying the number of points may be hazardous\n");
       printf("  to your program's health!!!  Proceed? (1= Yes, 0= No): ");
       scanf("%s", &str);
       if (atoi(str) == 1)
        {
        printf("Points: <%hd> ", DBase[i].Points);
        scanf("%s", &str);
        DBase[i].Points = atoi(str);
        } /* if */
       break;
       }
      case 8:
       {
       printf("Mark: <%s> ", DBase[i].Mark);
       scanf("%s", &str);
       strncpy(DBase[i].Mark, str, length[8]);
       DBase[i].Mark[length[8]] = 0;
       c = &DBase[i].Mark[0];
       while ( *c )
        {
        *c = toupper(*c);
        c++;
        } /* while */
       break;
       }
      } /* switch j */
     } while (j >= 0);
    } /* if */

   } while (i>=0);

  } while (i>=0);

} /* editdbaseitems() */

/*********************************************************************/

void deletedbaseitems(void)
{
 char num[32];
 short i, j;

 do
  {
  for (i=0; i<NoOfObjects; i +=4)
   {
   printf("%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s |%3d. %10s %1s\n", 
	i, DBase[i].Name, DBase[i].Mark, 
	i + 1, DBase[i + 1].Name, DBase[i + 1].Mark, 
	i + 2, DBase[i + 2].Name, DBase[i + 2].Mark, 
	i + 3, DBase[i + 3].Name, DBase[i + 3].Mark); 
   } /* for i=0... */
  printf("\n");

  do
   {
   printf("Item to delete (-1 to quit, > to repeat list): ");
   scanf("%s", &num);
   if (num[0] == '>') break;
   i = atoi(num);
   if (i >= 0 && i < NoOfObjects)
    {
    for (j=i+1; j<NoOfObjects; j++)
     {
     memcpy(&DBase[j-1], &DBase[j], sizeof (struct database));
     } /* for j>i... */
    NoOfObjects --;
    memset(&DBase[NoOfObjects], 0, sizeof (struct database));
    } /* if */

   } while (i>=0);

  } while (i>=0);

} /* editmarkstatus() */

#endif /*USE_GIS_GUI */

/***********************************************************************/
/* Constructs a database out of the USGS dem250.txt file

void ConstructDEMObj(void)
{
char Text[80], Label[32], Name[24], Layer1[8], Layer2[8], LatStr[8], LonStr[8],
	filename[256];
short i, j;
double SELat, SELon;
FILE *fTxt;
struct database *NewBase;

 if ((fTxt = fopen("Dh3:Data/TextFiles/dem250.txt", "r")) != NULL)
  {
  for (i=0; i<1384; i++)
   {
   fgets(Text, 70, fTxt);
   Text[65] = 0;
   strncpy(Layer1, &Text[0], length[1]);
   Layer1[length[1]] = 0;
   strncpy(Layer2, &Text[3], length[2]);
   Layer2[length[2]] = 0;
   Layer2[length[2] - 1] = ' ';
   strncpy(Label, &Text[15], length[6]);
   Label[length[6]] = 0;
   strncpy(LonStr, &Text[42], 7);
   LonStr[7] = 0;
   strncpy(LatStr, &Text[50], 6);
   LatStr[6] = 0;
   SELat = atof(LatStr);
   SELon = atof(LonStr);

   j = 0;
   while (LonStr[++j] != '.') {};
   LonStr[j] = 0;
   j = 0;
   while (LatStr[++j] != '.') {};
   LatStr[j] = 0;
    
   if (strlen(LatStr) < 2)
    {
    LatStr[2] = 0;
    LatStr[1] = LatStr[0];
    LatStr[0] = '0';
    }
   if (strlen(LonStr) < 3)
    {
    LonStr[3] = 0;
    LonStr[2] = LonStr[1];
    LonStr[1] = LonStr[0];
    LonStr[0] = '0';
    }

   sprintf(Name, "%s%s", LatStr, LonStr);
   Name[length[0]] = 0;
   while(strlen(Name) < length[0])
    strcat(Name, " ");



   if (NoOfObjects + 1 > DBaseRecords)
    {
    if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
     {
     User_Message("Database Module",
	"Out of memory expanding database!\nOperation terminated.", "OK", "o");
     break;
     } // if new database allocation fails
    else
     {
     DBase = NewBase;
     DBaseRecords += 20;
     } // else new database allocated and copied
    } // if need larger database memory
   OBN = NoOfObjects;
   NoOfObjects ++;
   DBase[OBN].Lat = DBase[OBN].Lon = NULL;
   DBase[OBN].Elev = NULL;
   DBase[OBN].VecArraySize = 0;
   strncpy(DBase[OBN].Name, Name, length[0]);
   strcpy(DBase[OBN].Special, "TOP");
   DBase[OBN].LineWidth = 1;
   DBase[OBN].Color = 2;
   DBase[OBN].Pattern[0] = 'L';
   DBase[OBN].Red = 255;
   DBase[OBN].Grn = 255;
   DBase[OBN].Blu = 255;
   DBase[OBN].Name[length[0]] = 0;
   DBase[OBN].Points = 0;
   DBase[OBN].Mark[0] = 'Y';
   DBase[OBN].Enabled = '*';
   DBase[OBN].Flags = 7;
   strncpy(DBase[OBN].Layer1, Layer1, length[1]);
   while (strlen(DBase[OBN].Layer1) < length[1])
    strcat(DBase[OBN].Layer1, " ");
   strncpy(DBase[OBN].Layer2, Layer2, length[2]);
   while (strlen(DBase[OBN].Layer2) < length[2])
    strcat(DBase[OBN].Layer2, " ");
   strncpy(DBase[OBN].Label, Label, length[6]);
   while (strlen(DBase[OBN].Label) < length[6])
    strcat(DBase[OBN].Label, " ");
   if (allocvecarray(OBN, 5, 1))
    {
    DBase[OBN].Points = 5;
    DBase[OBN].Lat[0] = SELat;
    DBase[OBN].Lon[0] = SELon;
    DBase[OBN].Lat[1] = SELat;
    DBase[OBN].Lon[1] = SELon;
    DBase[OBN].Lat[2] = SELat;
    DBase[OBN].Lon[2] = SELon + 1.0;
    DBase[OBN].Lat[3] = SELat + 1.0;
    DBase[OBN].Lon[3] = SELon + 1.0;
    DBase[OBN].Lat[4] = SELat + 1.0;
    DBase[OBN].Lon[4] = SELon;
    DBase[OBN].Lat[5] = SELat;
    DBase[OBN].Lon[5] = SELon;
    if (MapWind0 && mapscale != 0.0)
     {
     struct clipbounds cb;

     setclipbounds(MapWind0, &cb);
     outline(MapWind0, OBN, 2, &cb);
     } // if map module open
    } // if memory allocated for lat/lon arrays
   else
    {
    User_Message("Database Module: Editor",
	"No memory for vector coordinates!\nNew object has been created\
 but can not be edited until memory is available.",
	"OK", "o");
    break;
    } // else no memory, not even a little bit!

   strmfp(filename, dirname, DBase[OBN].Name);
   strcat(filename, ".Obj");
   if (saveobject(OBN, filename, DBase[OBN].Lon, DBase[OBN].Lat,
	DBase[OBN].Elev))
    break;
   } // for i=0...
  fclose(fTxt);
  DB_Mod = 1;
  } // if

} // ConstructDEMObj()
*/

/**********************************************************************/
#ifdef DBASE_SAVE_COMPOSITE

short DBase_SaveComposite(void)
{
char filename[256], *Title = "WCSMasterObject", *LastDir = NULL;
short i, error = 0;
FILE *fDbs;

 for (i=0; i<NoOfObjects; i++)
  {
  if (! DBase[i].Lat || ! DBase[i].Lon || ! DBase[i].Elev)
   {
   freevecarray(i);	/* make sure all vertex memory is freed */
   if (Load_Object(i, &LastDir))
    {
    error = 2;
    User_Message(DBase[i].Name,
	"Error loading this Object!\nOperation terminated.", "OK", "o");
    break;
    } /* if */
   } /* if */
  } /* for i=0... */

 if (! error)
  {
  strmfp(filename, dbasepath, dbasename);
  strcat(filename, ".MDB");
 
  if ((fDbs = fopen(filename, "wb")) != NULL)
   {
   fwrite(Title, 16, 1, fDbs);
   fwrite((char *)&NoOfObjects, sizeof (short), 1, fDbs);
   for (i=0; i<NoOfObjects; i++)
    {
    fwrite((char *)&DBase[i].Points, sizeof (short), 1, fDbs);

    if (fwrite((char *)DBase[i].Lon,
	 (DBase[i].Points + 1) * sizeof (double), 1, fDbs) == 1)
     {
     if (fwrite((char *)DBase[i].Lat,
	 (DBase[i].Points + 1) * sizeof (double), 1, fDbs) == 1)
      {
      if (fwrite((char *)DBase[i].Elev,
	 (DBase[i].Points + 1) * sizeof (short), 1, fDbs) != 1)
       error = 1;
      } /* if */
     else
      error = 1;
     } /* if */
    else
     error = 1;
    if (error)
     {
     User_Message("Map View: Save All",
	"Error writing Master Object file!\nOperation terminated.", "OK", "o");
     break;
     } /* if */
    } /* for i=0... */
   fclose(fDbs);
   } /* if */
  } /* if all objects loaded */

 return ((short)(! error));

} /* DBase_SaveComposite() */

/**********************************************************************/

short DBase_LoadComposite(void)
{
char filename[256], Title[32];
short i, error = 0, Objects, Points, ReadAll = 1;
FILE *fDbs;
struct BusyWindow *BusyLoad;

 strmfp(filename, dbasepath, dbasename);
 strcat(filename, ".MDB");
 
 BusyLoad = BusyWin_New("Vector Load", NoOfObjects, 0, 'MVLD');
 if ((fDbs = fopen(filename, "rb")) != NULL)
  {
  fread(Title, 16, 1, fDbs);
  if (! strcmp(Title, "WCSMasterObject"))
   {
   fread(&Objects, sizeof (short), 1, fDbs);
   if (Objects == NoOfObjects)
    {
    for (i=0; i<NoOfObjects; i++)
     {
     fread(&Points, sizeof (short), 1, fDbs);
     if (Points == DBase[i].Points)
      {
      if (allocvecarray(i, DBase[i].Points, 1))
       {
       if (fread((char *)&DBase[i].Lon[0],
	 (DBase[i].Points + 1) * sizeof (double), 1, fDbs) == 1)
        {
        if (fread((char *)&DBase[i].Lat[0],
	 (DBase[i].Points + 1) * sizeof (double), 1, fDbs) == 1)
         {
         if (fread((char *)&DBase[i].Elev[0],
		 (DBase[i].Points + 1) * sizeof (short), 1, fDbs) != 1)
          error = 1;
         } /* if */
        else
         error = 1;
        } /* if */
       else
        error = 1;
       } /* if memory allocated */
      else
       error = 6;

      } /* if read from master file */
     else
      {
      ReadAll = 0;
      error = fseek(fDbs, Points * (2 * sizeof (double) + sizeof (short)), SEEK_CUR);
      } /* else load object from its own file later */
     
     if (error)
      {
      if (error == 6)
       User_Message("Map View: Load", 
	"Out of memory loading Master Object File!\nEnabled Objects will be loaded individually.", "OK", "o");
      else
       User_Message("Map View: Load", 
	"Error reading Master Object file!\nOperation terminated.", "OK", "o");
      break;
      } /* if */
     if (CheckInput_ID() == ID_BW_CLOSE)
      break;
     if(BusyLoad)
      BusyWin_Update(BusyLoad, i + 1);
     } /* for i=0... */
    } /* if correct number of objects */
   else
    {
    User_Message("Map View: Load",
	"Number of Objects in the Master Object file does not match the\
 number of Objects in the current Database!\
 Master Object file cannot be used. Objects will be loaded from individual files", "OK", "o");
    error = 5;
    } /* if wrong number of objects */
   } /* if correct file type */
  else
   {
   error = 3;
   User_Message("Map View: Load", 
	".MDB is not a WCS Master Object file!", "OK", "o");
   } /* else */

  fclose(fDbs);
  if (error)
   {
   for (i=0; i<NoOfObjects; i++)
    freevecarray(i);
   } /* if */
  } /* if */
 else
  {
  error = 4;
  } /* else */

 if(BusyLoad)
  BusyWin_Del(BusyLoad);


 if (error)
  return (0);
 if (! ReadAll)
  return (-1);
 return (1);

} /* DBase_LoadComposite() */

#endif /* DBASE_SAVE_COMPOSITE */
