/* DXF.c
** DXF file loader for WCS.
** Original code written by Gary R. Huber, Jamuary, 1994.
*/

#include "WCS.h"
#include "GUIDefines.h"

short ImportDLG(void)
{
static char filename[255], DLGfile[32], DLGpath[256], UserPrefix[32];
char str[80];
FILE *fDLG = NULL;
short done = 0, error = 0, AttrsRead, ReadyToSave = 0, *TempElev, dummy, RefSys,
	UTMZone;
double *TempLon, *TempLat;
double TestVal;
long i, j;
union MapProjection *Coords;
struct DLGInfo DLG;
struct BusyWindow *BWDL;

 TempLon = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempLat = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempElev = (short *)get_Memory(MAXOBJPTS * sizeof (short), MEMF_ANY);
 Coords = (union MapProjection *)get_Memory(sizeof (union MapProjection), MEMF_CLEAR);

 if (! TempLon || ! TempLat || ! TempElev || ! Coords)
  {
  User_Message("Data Ops Module: Import DLG",
	"Out of memory allocating temporary arrays!\nOperation terminated.",
	"OK", "o");
  goto Cleanup;
  } /* if no database loaded */

 strcpy(DLGpath, dirname);
 DLGfile[0] = 0;
 if (! getfilename(0, "DLG File", DLGpath, DLGfile)) goto Cleanup;

 if (DLGfile[0] == 0)
  {
  User_Message("Data Ops Module: Import DLG",
	"No file(s) selected!",	"OK", "o");
  goto Cleanup;
  } /* if no file */

 strmfp(filename, DLGpath, DLGfile);

 if ((fDLG = fopen(filename, "r")) == NULL)
  {
  User_Message("Data Ops Module: Import DLG",
	"Can't open DLG file for input!\nOperation terminated.", "OK", "o");
  Log(ERR_OPEN_FAIL, DLGfile);
  goto Cleanup;
  } /* if file open failed */

 fscanf(fDLG, "%72s", &str);
 fseek(fDLG, 80, SEEK_SET);
 fscanf(fDLG, "%40s", &str);

 fseek(fDLG, 240, SEEK_SET);
 for (i=0; i<3; i++)
  {
  fscanf(fDLG, "%6hd", &dummy);
  if (dummy == 3)
   break;
  }
 if (i >= 3)
  {
  Log(ERR_WRONG_TYPE, DLGfile);
  User_Message("Data Ops Module: Import DLG",
	"File not a USGS Optional DLG!\nOperation terminated.", "OK", "o");
  goto Cleanup;
  }
 fscanf(fDLG, "%6hd", &RefSys);

 fscanf(fDLG, "%6hd", &UTMZone);

 if ((UTMZone < 0 || UTMZone > 60) && UTMZone != 9999)
  {
  User_Message("Data Ops Module: Import DLG",
	"Inappropriate UTM Zone!\nOperation terminated.", "OK", "o");
  Log(ERR_WRONG_TYPE, DLGfile);
  goto Cleanup;
  } /* if zone out of range */

 if (RefSys == 1)
  UTMLatLonCoords_Init(&Coords->UTM, UTMZone);
 else if (RefSys == 3)
  {
  fseek(fDLG, 320, SEEK_SET);
  for (i=0; i<8; i++)
   {
   fscanf(fDLG, "%24s", str);
   Coords->Alb.ProjPar[i] = FCvt(str);
   } /* for i=0... */
  AlbLatLonCoords_Init(&Coords->Alb);
  }
 else
  {
  Log(ERR_WRONG_TYPE, DLGfile);
  User_Message("Data Ops Module: Import DLG",
	"This file contains data in an unsupported Reference System!\nOperation terminated.", "OK", "o");
  goto Cleanup;
  }

 fseek(fDLG, 800, SEEK_SET);		/* end of headers */
 for (i=0; i<4; i++)
  {
  fscanf(fDLG, "%s%le%le%le%le", &str, &Coords->UTM.RefLat[i],
	&Coords->UTM.RefLon[i], &Coords->UTM.RefEast[i], &Coords->UTM.RefNorth[i]);
  } /* for i=0... */

 fseek(fDLG, 0L, SEEK_END);
 BWDL = BusyWin_New("Reading", ftell(fDLG), 0, 'BWDL');

 str[0] = 0;
 if (GetInputString("Enter up to 3 characters as a prefix for this DLG set if you desire.", ":;*/?`#%", str))
  {
  strncpy(UserPrefix, str, 3);
  UserPrefix[3] = 0;
  }
 else
  {
  UserPrefix[0] = 0;
  }

 fseek(fDLG, 1120, SEEK_SET);
 fscanf(fDLG, "%s", &str);

 i = 1200;
 fseek(fDLG, i, SEEK_SET);
 while (done != EOF)
  {
  if ((done = fscanf(fDLG, "%s", &str)) == EOF) break;
  if (! strcmp(str, "L"))
   {
   fscanf(fDLG, "%ld", &DLG.IDNum);
   fscanf(fDLG, "%ld", &DLG.StartNode);
   fscanf(fDLG, "%ld", &DLG.EndNode);
   fscanf(fDLG, "%hd", &DLG.LeftArea);
   fscanf(fDLG, "%hd", &DLG.RightArea);
   fscanf(fDLG, "%hd", &DLG.Pairs);
   fscanf(fDLG, "%hd", &DLG.Attrs);
   fscanf(fDLG, "%hd", &DLG.Text);

   if (DLG.Pairs < 1 || DLG.Pairs > 1500)
    goto EndError;

   fscanf(fDLG, "%le", &TestVal);
   Coords->UTM.East = TestVal;
   fscanf(fDLG, "%le", &TestVal);
   Coords->UTM.North = TestVal;
   if (RefSys == 1)
    UTM_LatLon(&Coords->UTM);
   else
    Alb_LatLon(&Coords->Alb);
   TempLon[0] = TempLon[1] = Coords->UTM.Lon;
   TempLat[0] = TempLat[1] = Coords->UTM.Lat;

   for (j=2; j<=DLG.Pairs; j++)
    {
    fscanf(fDLG, "%le", &TestVal);
    if (RefSys == 1 &&  abs(TestVal - Coords->UTM.East) < 10000)
     Coords->UTM.East = TestVal;
    else if (abs(TestVal - Coords->UTM.East) < 100000)
     Coords->UTM.East = TestVal;
    else
     {
     error = 1;
     break;
     }
    fscanf(fDLG, "%s", str);
    TestVal = atof(str);
    if (RefSys == 1 && abs(TestVal - Coords->UTM.North) < 10000)
     Coords->UTM.North = TestVal;
    else if (abs(TestVal - Coords->UTM.North) < 100000)
     Coords->UTM.North = TestVal;
    else
     {
     error = 1;
     break;
     }
    if (RefSys == 1)
     UTM_LatLon(&Coords->UTM);
    else
     Alb_LatLon(&Coords->Alb);
    TempLon[j] = Coords->UTM.Lon;
    TempLat[j] = Coords->UTM.Lat;
    TempElev[j] = 0;
    } /* for j=0... */

   if (error)
    {
    error = 0;
    goto EndError;
    } /* if read error, sometimes not enough data pairs! */

   AttrsRead = 0;

RepeatReadAttrs:
   fscanf(fDLG, "%s", str);
   DLG.MajorAttr = atoi(str);
   fscanf(fDLG, "%s", str);
   DLG.MinorAttr = atoi(str);
   AttrsRead ++;

   if (! AssignDLGAttrs(&DLG, UserPrefix))
    {
    if (DLG.Attrs > AttrsRead)
     goto RepeatReadAttrs;
    else
     goto EndError;
    } /* if no minor attribute */
   else if (! strcmp(DLG.Layer1, "  ") && DLG.Attrs > AttrsRead)
    goto RepeatReadAttrs;

   if ((ReadyToSave = OpenObjFile(DLG.Name)) == 0)
    break;

   if (NoOfObjects + 1 > DBaseRecords)
    {
    struct database *NewBase;

    if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 100)) == NULL)
     {
     User_Message("Database Module",
		"Out of memory expanding database!\nOperation terminated.", "OK", "o");
     break;
     } /* if new database allocation fails */
    else
     {
     DBase = NewBase;
     DBaseRecords += 100;
     } /* else new database allocated and copied */
    } /* if need larger database memory */

   Set_DLGObject(&DLG);

   strmfp(filename, dirname, DLG.Name);
   strcat(filename, ".Obj");

   if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
    {
    User_Message("Data Ops Module: Import DLG",
	"Error saving object file!\nOperation terminated", "OK", "o");
    break;
    } /* if save fail */

   if (DE_Win)
    {
    if (! Add_DE_NewItem())
     {
     User_Message("Database Module",
		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
     break;
     } /* if new list fails */
    } /* if database editor open */

EndError:
   i = ftell(fDLG);
   i = (1 + (i - 1) / 80) * 80;
   fseek(fDLG, i, SEEK_SET);
   } /* if Line element */

  else
   {
   i += 80;
   fseek(fDLG, i, SEEK_SET);
   } /* else not line */

  if (CheckInput_ID() == ID_BW_CLOSE)
   break;
  BusyWin_Update(BWDL, i);

  } /* while ! EOF */


 BusyWin_Del(BWDL);

Cleanup:

 if (fDLG) fclose(fDLG);
 if (TempLon) free_Memory(TempLon, MAXOBJPTS * sizeof (double));
 if (TempLat) free_Memory(TempLat, MAXOBJPTS * sizeof (double));
 if (TempElev) free_Memory(TempElev, MAXOBJPTS * sizeof (short));
 if (Coords) free_Memory(Coords, sizeof (union MapProjection));
 return (1);

} /* ImportDLG() */

/************************************************************************/

short ImportDXF(void)
{
char filename[255], DXFfile[64], DXFpath[255], DefaultName[32];
char str[80], ObjName[64], Layer1[64], Pattern[8], DBaseObjName[16];
FILE *fDXF;
short newline = 1, pts = 0, done = 0, abort = 0, code, ReadyToSave = 0,
	*TempElev;
float FloatValue;
double *TempLon, *TempLat;
long IntValue;
struct BusyWindow *BWDL;

 TempLon = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempLat = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempElev = (short *)get_Memory(MAXOBJPTS * sizeof (short), MEMF_CLEAR);
 
 if (! TempLon || ! TempLat || ! TempElev)
  {
  User_Message("Data Ops Module: Import DXF",
	"Out of memory allocating temporary arrays!\nOperation terminated.",
	"OK", "o");
  goto Cleanup;
  } /* if no database loaded */

 strcpy(DXFpath, dirname);
 DXFfile[0] = 0;
 if (! getfilename(0, "DXF File", DXFpath, DXFfile)) goto Cleanup;

 if (DXFfile[0] == 0)
  {
  User_Message("Data Ops Module: Import DXF",
	"No file(s) selected!",	"OK", "o");
  goto Cleanup;
  } /* if no file */

 strmfp(filename, DXFpath, DXFfile);

 if ((fDXF = fopen(filename, "r")) == NULL)
  {
  User_Message("Data Ops Module: Import DXF",
	"Can't open DXF file for input!\nOperation terminated.", "OK", "o");
  Log(ERR_OPEN_FAIL, DXFfile);
  goto Cleanup;
  } /* if file open failed */

 Pattern[0] = 0;
 DefaultName[0] = 0;

 fseek(fDXF, 0L, SEEK_END);
 BWDL = BusyWin_New("Reading", ftell(fDXF), 0, 'BWDL');
 fseek(fDXF, 0L, SEEK_SET);

 while (done != EOF)
  {
  if ((done = fscanf(fDXF, "%hd", &code)) == EOF) break;

  if (code >= 0 && code <= 9)
   {
   fscanf(fDXF, "%s", str);
   }
  else if (code >= 10 && code <= 59)
   {
   fscanf(fDXF, "%f", &FloatValue);
   }
  else if (code >= 60 && code <= 79)
   {
   fscanf(fDXF, "%ld", &IntValue);
   }
  else if (code >= 210 && code <= 239)
   {
   fscanf(fDXF, "%f", &FloatValue);
   }
  else if (code == 999)
   {
   fscanf(fDXF, "%s", str);
   }
  else
   {
   User_Message("Data Ops Module: Import DXF",
	"Improper Code value found!\nOperation terminated prematurely.", "OK", "o");
   break;
   }

  switch (code)
   {
   case 0:
    {
    if (! strcmp(str, "POLYLINE"))
     {
     if (ReadyToSave)
      {

      if (NoOfObjects + 1 > DBaseRecords)
       {
       struct database *NewBase;

       if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 100)) == NULL)
        {
        User_Message("Database Module",
		"Out of memory expanding database!\nOperation terminated.", "OK", "o");
        done = 1;
        break;
        } /* if new database allocation fails */
       else
        {
        DBase = NewBase;
        DBaseRecords += 100;
        } /* else new database allocated and copied */
       } /* if need larger database memory */

      ReadyToSave = 0;
      Set_DXFObject(DBaseObjName, pts, Pattern);
      Layer1[0] = 0;

      if (DE_Win)
       {
       if (! Add_DE_NewItem())
        {
        User_Message("Database Module",
		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
        done = 1;
        break;
	} /* if error adding item */
       } /* if database editor open */

      strmfp(filename, dirname, ObjName);
      strcat(filename, ".Obj");

      if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
       {
       User_Message("Data Ops Module: Import DXF",
	"Error saving object!\nOperation terminated.", "OK", "o");
       done = 1;
       break;
       } /* if save error */

      if (CheckInput_ID() == ID_BW_CLOSE)
       break;
      BusyWin_Update(BWDL, ftell(fDXF));
      } /* if ready to save */

     pts = 0;
     newline = 1;
     ObjName[0] = 0;
     } /* end old, begin new object */

    if (! strcmp(str, "VERTEX") || ! strcmp(str, "POINT"))
     {
     if (newline)
      {
      if (! strcmp(str, "VERTEX")) strcpy(Pattern, "L");
      else if (! strcmp(str, "POINT")) strcpy(Pattern, "P");
      if (ObjName[0])
       {
       if ((ReadyToSave = OpenObjFile(ObjName)) == 0)
        {
        abort = 1;
        break;
	}
       strcpy(DBaseObjName, ObjName);
       }
      else if (Layer1[0])
       {
       if ((ReadyToSave = OpenObjFile(Layer1)) == 0)
        {
        abort = 1;
        break;
	}
       strcpy(DBaseObjName, Layer1);
       strcpy(ObjName, Layer1);
       }
      else
       {
       if (! DefaultName[0])
        {
        if (! GetInputString("An entity has been found with no name identifier. Please enter a default name.",
		":;*/?`#%", DefaultName))
         {
         abort = 1;
         break; 
	 }
	} /* if */
       strcpy(ObjName, DefaultName);
       if ((ReadyToSave = OpenObjFile(ObjName)) == 0)
        {
        abort = 1;
        break;
	}
       strcpy(DBaseObjName, ObjName);
       }
      newline = 0;
      } /* if new line started */
     else 
      {
      pts ++;
      } /* else not new line */
     } /* begin new point */
    break;
    } /* section start */

   case 1:
    {
    strcpy(ObjName, str);
    break;
    } /* entity name */

   case 8:
    {
    strcpy(Layer1, str);
    break;
    } /* entity name */

   case 10:
    {
    if (newline) break;
    TempLon[pts] = -FloatValue;
    if (pts == 0)
     {
     TempLon[1] = -FloatValue;
     }
    break;
    } /* X coordinate */

   case 20:
    {
    if (newline) break;
    TempLat[pts] = FloatValue;
    TempElev[pts] = 0;
    if (pts == 0)
     {
     pts ++;
     TempLat[1] = FloatValue;
     }
    break;
    } /* Y coordinate */

   case 30:
    {
    if (newline) break;
    TempElev[pts] = (short)FloatValue;
    if (pts == 1 || pts == 0)
     {
     TempElev[0] = TempElev[1] = (short)FloatValue;
     }
    break;
    } /* Y coordinate */

   } /* switch code */
   if (abort) break;

  } /* while (! done) */

 BusyWin_Del(BWDL);

 if (fDXF) fclose(fDXF);
 if (ReadyToSave)
  {
  Set_DXFObject(DBaseObjName, pts, Pattern);

  if (DE_Win)
   {
   if (! Add_DE_NewItem())
    User_Message("Database Module",
	"Out of memory expanding Database Editor List!\nLast item does not appear in list view.", "OK", "o");
   } /* if database editor open */

  strmfp(filename, dirname, ObjName);
  strcat(filename, ".Obj");
  if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
   {
   User_Message("Data Ops Module: Import DXF",
	"Error saving last object!\nOperation terminated.", "OK", "o");
   } /* if save error */
  } /* if object output file open (not yet written) */

Cleanup:

 if (TempLon) free_Memory(TempLon, MAXOBJPTS * sizeof (double));
 if (TempLat) free_Memory(TempLat, MAXOBJPTS * sizeof (double));
 if (TempElev) free_Memory(TempElev, MAXOBJPTS * sizeof (short));
 return (1);

} /* ImportDXF() */

/*********************************************************************/

short OpenObjFile(char *ObjName)
{
 short i = 0, j = 0, k = 0, l, OBNexists;

 ObjName[length[0]] = 0;
 while (strlen(ObjName) < length[0]) strcat(ObjName, " ");

Repeat:

 OBNexists = 0;
 for (l=NoOfObjects-1; l>=0; l--)
  {
  if (! stricmp(ObjName, DBase[l].Name))
   {
   OBNexists = 1;
   break;
   } /* if */
  } /* for l=0... */

 if (OBNexists)
  {

  if (i < 26 && j == 0 && k == 0)
   {
   ObjName[length[0] - 1] = 65 + i;
   } /* if */

  else if (j < 26 && k == 0)
   {
   ObjName[length[0] - 2] = 64 + j;
   ObjName[length[0] - 1] = 65 + i;
   } /* else */

  else
   {
   ObjName[length[0] - 3] = 64 + k;
   ObjName[length[0] - 2] = 65 + j;
   ObjName[length[0] - 1] = 65 + i;
   }

  i ++;
  if (i >= 26)
   {
   i = 0;
   j ++;
   } /* if */

  if (j >= 26)
   {
   j = 0;
   k ++;
   } /* if */

  if (k >= 26) return (0);

  goto Repeat;

  } /* if name already used */

 return (1);

} /* OpenObjFile() */

/*********************************************************************/

void Set_DXFObject(char *name, short pts, char *ptrn)
{

  OBN = NoOfObjects;
  strncpy(DBase[OBN].Name, name, length[0]);
  strncpy(DBase[OBN].Layer1, "      ", length[1]);
  strncpy(DBase[OBN].Layer2, "      ", length[2]);
  DBase[OBN].Color = 3;
  DBase[OBN].LineWidth = 1;
  strncpy(DBase[OBN].Label, "                       ", length[6]);
  DBase[OBN].MaxFract = 9;
  DBase[OBN].Pattern[0] = ptrn[0];
  DBase[OBN].Points = pts;
  DBase[OBN].Mark[0] = 'Y';
  DBase[OBN].Enabled = '*';
  DBase[OBN].Red = 180;
  DBase[OBN].Grn = 180;
  DBase[OBN].Blu = 180;
  DBase[OBN].Flags = 6;
  strcpy(DBase[OBN].Special, "VEC");
  NoOfObjects ++;
  DB_Mod = 1;

} /* Set_DXFObject() */

/************************************************************************/

void Set_DLGObject(struct DLGInfo *DLG)
{
 short i;

 OBN = NoOfObjects;
 strncpy(DBase[OBN].Name, DLG->Name, length[0]);
 strcpy(DBase[OBN].Layer1, DLG->Layer1);
 strcpy(DBase[OBN].Layer2, DLG->Layer2);
 DBase[OBN].Color = DLG->Color;
 DBase[OBN].LineWidth = DLG->LineWidth;
 strncpy(DBase[OBN].Label, DLG->Name, length[0]);
 for (i=length[0]; i<length[6]; i++)
   strcat(DBase[OBN].Label, " ");
 DBase[OBN].MaxFract = 9;
 DBase[OBN].Pattern[0] = 'L';
 DBase[OBN].Points = DLG->Pairs;
 DBase[OBN].Mark[0] = 'Y';
 DBase[OBN].Enabled = '*';
 DBase[OBN].Red = (AltColors[DBase[OBN].Color] & 0xf00) / 256;
 DBase[OBN].Red += DBase[OBN].Red * 16;
 DBase[OBN].Grn = (AltColors[DBase[OBN].Color] & 0x0f0) / 16;
 DBase[OBN].Grn += DBase[OBN].Grn * 16;
 DBase[OBN].Blu = (AltColors[DBase[OBN].Color] & 0x00f);
 DBase[OBN].Blu += DBase[OBN].Blu * 16;
 DBase[OBN].Flags = 6;
 strcpy(DBase[OBN].Special, "VEC");
 NoOfObjects ++;
 DB_Mod = 1;

} /* Set_DLGObject() */

/************************************************************************/

short AssignDLGAttrs(struct DLGInfo *DLG, char *Prefix)
{
 short SaveObject = 1;

 DLG->LineWidth = 1;

 strcpy(DLG->Name, Prefix);

 switch (DLG->MajorAttr)
  {
  case 40:
   {
   strcpy(DLG->Layer2, "HYD");
   strcpy(DLG->Layer1, "  ");
   strcat(DLG->Name, "Area");
   DLG->Color = 6;
   break;
   } /* Water Bodies 1:2000000 */
  case 50:
   {
   strcpy(DLG->Layer2, "HYD");
   DLG->Color = 6;
   if (DLG->MinorAttr == 0)
    {
    SaveObject = 0;
    break;
    } /* photo-revision */
   if (DLG->MinorAttr >= 200 && DLG->MinorAttr < 300)
    {
    strcpy(DLG->Layer1, "SH");
    switch (DLG->MinorAttr)
     {
     case 200:
      {
      strcat(DLG->Name, "Shore");
      break;
      } /* shoreline */
     case 201:
      {
      strcat(DLG->Name, "ManMdShore");
      break;
      } /* shoreline */
     case 202:
      {
      strcat(DLG->Name, "ClosureLn");
      break;
      } /* shoreline */
     case 203:
      {
      strcat(DLG->Name, "IndefShore");
      break;
      } /* shoreline */
     case 204:
      {
      strcat(DLG->Name, "ApparLimit");
      break;
      } /* shoreline */
     case 205:
      {
      strcat(DLG->Name, "OutlineBay");
      break;
      } /* shoreline */
     case 206:
      {
      strcat(DLG->Name, "DangerCrve");
      break;
      } /* shoreline */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch */
    } /* if */
   else if (DLG->MinorAttr >= 300 && DLG->MinorAttr < 400)
    {
    strcpy(DLG->Layer1, "PT");
    switch (DLG->MinorAttr)
     {
     case 300:
      {
      strcat(DLG->Name, "Spring");
      break;
      } /*  */
     case 301:
      {
      strcat(DLG->Name, "NonFloWell");
      break;
      } /*  */
     case 302:
      {
      strcat(DLG->Name, "FlowngWell");
      break;
      } /*  */
     case 303:
      {
      strcat(DLG->Name, "Riser");
      break;
      } /*  */
     case 304:
      {
      strcat(DLG->Name, "Geyser");
      break;
      } /*  */
     case 305:
      {
      strcat(DLG->Name, "Windmill");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 400 && DLG->MinorAttr < 500)
    {
    strcpy(DLG->Layer1, "ST");
    switch (DLG->MinorAttr)
     {
     case 400:
      {
      strcat(DLG->Name, "Rapids");
      break;
      } /*  */
     case 401:
      {
      strcat(DLG->Name, "Falls");
      break;
      } /*  */
     case 402:
      {
      strcat(DLG->Name, "GravelPit");
      break;
      } /*  */
     case 403:
      {
      strcat(DLG->Name, "GagingSta");
      break;
      } /*  */
     case 404:
      {
      strcat(DLG->Name, "PumpingSta");
      break;
      } /*  */
     case 405:
      {
      strcat(DLG->Name, "WaterIntak");
      break;
      } /*  */
     case 406:
      {
      strcat(DLG->Name, "Dam");
      break;
      } /*  */
     case 407:
      {
      strcat(DLG->Name, "CanalLock");
      break;
      } /*  */
     case 408:
      {
      strcat(DLG->Name, "Spillway");
      break;
      } /*  */
     case 409:
      {
      strcat(DLG->Name, "Gate");
      break;
      } /*  */
     case 410:
      {
      strcat(DLG->Name, "Rock");
      break;
      } /*  */
     case 411:
      {
      strcat(DLG->Name, "Crevasse");
      break;
      } /*  */
     case 412:
      {
      strcat(DLG->Name, "Stream");
      break;
      } /*  */
     case 413:
      {
      strcat(DLG->Name, "BraidStrm");
      break;
      } /*  */
     case 414:
      {
      strcat(DLG->Name, "Ditch");
      break;
      } /*  */
     case 415:
      {
      strcat(DLG->Name, "Aqueduct");
      break;
      } /*  */
     case 416:
      {
      strcat(DLG->Name, "Flume");
      break;
      } /*  */
     case 417:
      {
      strcat(DLG->Name, "Penstock");
      break;
      } /*  */
     case 418:
      {
      strcat(DLG->Name, "Siphon");
      break;
      } /*  */
     case 419:
      {
      strcat(DLG->Name, "Channel");
      break;
      } /*  */
     case 420:
      {
      strcat(DLG->Name, "Ephemeral");
      break;
      } /*  */
     case 421:
      {
      strcat(DLG->Name, "Lake");
      break;
      } /*  */
     case 422:
      {
      strcat(DLG->Name, "CoralReef");
      break;
      } /*  */
     case 423:
      {
      strcat(DLG->Name, "Sand");
      break;
      } /*  */
     case 424:
      {
      strcat(DLG->Name, "Spoil");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 500)
    {
    strcpy(DLG->Layer1, "MI");
    switch (DLG->MinorAttr)
     {
     case 601:
      {
      strcat(DLG->Name, "Undergrnd");
      break;
      } /*  */
     case 602:
      {
      strcat(DLG->Name, "Overpassg");
      break;
      } /*  */
     case 603:
      {
      strcat(DLG->Name, "Elevated");
      break;
      } /*  */
     case 604:
      {
      strcat(DLG->Name, "Tunnel");
      break;
      } /*  */
     case 605:
      {
      strcat(DLG->Name, "RightBank");
      break;
      } /*  */
     case 606:
      {
      strcat(DLG->Name, "LeftBank");
      break;
      } /*  */
     case 607:
      {
      strcat(DLG->Name, "UnderConst");
      break;
      } /*  */
     case 608:
      {
      strcat(DLG->Name, "Salt");
      break;
      } /*  */
     case 609:
      {
      strcat(DLG->Name, "Unsurvey");
      break;
      } /*  */
     case 610:
      {
      strcat(DLG->Name, "Intermitt");
      break;
      } /*  */
     case 611:
      {
      strcat(DLG->Name, "Abandoned");
      break;
      } /*  */
     case 612:
      {
      strcat(DLG->Name, "Submerged");
      break;
      } /*  */
     case 614:
      {
      strcat(DLG->Name, "Dry");
      break;
      } /*  */
     case 615:
      {
      strcat(DLG->Name, "Mineral");
      break;
      } /*  */
     case 616:
      {
      strcat(DLG->Name, "Navigable");
      break;
      } /*  */
     case 617:
      {
      strcat(DLG->Name, "Underpass");
      break;
      } /*  */
     case 618:
      {
      strcat(DLG->Name, "Earthen");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else
    {
    SaveObject = 0;
    } /* else */
   break;
   } /* hydrography */
  case 90:
  case 91:
  case 92:
   {
   strcpy(DLG->Layer2, "BDY");
   strcpy(DLG->Layer1, "  ");
   strcat(DLG->Name, "Area");
   DLG->Color = 3;
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 102:
   {
   strcpy(DLG->Layer2, "ROA");
   strcpy(DLG->Layer1, "IS");
   sprintf(str, "I-%d", DLG->MinorAttr);
   strcat(DLG->Name, str);
   DLG->Color = 4;
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 103:
   {
   strcpy(DLG->Layer2, "ROA");
   strcpy(DLG->Layer1, "US");
   sprintf(str, "US-%d", DLG->MinorAttr);
   strcat(DLG->Name, str);
   DLG->Color = 4;
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 104:
   {
   strcpy(DLG->Layer2, "ROA");
   strcpy(DLG->Layer1, "ST");
   sprintf(str, "ST-%d", DLG->MinorAttr);
   strcat(DLG->Name, str);
   DLG->Color = 4;
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 170:
   {
   strcpy(DLG->Layer2, "ROA");
   DLG->Color = 4;
   if (DLG->MinorAttr == 0)
    {
    SaveObject = 0;
    break;
    } /* photo-revision */
   if (DLG->MinorAttr >= 200 && DLG->MinorAttr < 300)
    {
    strcpy(DLG->Layer1, "SF");
    switch (DLG->MinorAttr)
     {
     case 201:
      {
      strcat(DLG->Name, "C1Undivide");
      break;
      } /*  */
     case 202:
      {
      strcat(DLG->Name, "C1CentLine");
      break;
      } /*  */
     case 203:
      {
      strcat(DLG->Name, "C1Divided");
      break;
      } /*  */
     case 204:
      {
      strcat(DLG->Name, "C1OneWay");
      break;
      } /*  */
     case 205:
      {
      strcat(DLG->Name, "C2Undivide");
      break;
      } /*  */
     case 206:
      {
      strcat(DLG->Name, "C2CentLine");
      break;
      } /*  */
     case 207:
      {
      strcat(DLG->Name, "C2Divided");
      break;
      } /*  */
     case 208:
      {
      strcat(DLG->Name, "C2OneWay");
      break;
      } /*  */
     case 209:
      {
      strcat(DLG->Name, "C3Road");
      break;
      } /*  */
     case 210:
      {
      strcat(DLG->Name, "C4Road");
      break;
      } /*  */
     case 211:
      {
      strcat(DLG->Name, "C5Trail");
      break;
      } /*  */
     case 212:
      {
      strcat(DLG->Name, "C5FourWD");
      break;
      } /*  */
     case 213:
      {
      strcat(DLG->Name, "FootBridge");
      break;
      } /*  */
     case 214:
      {
      strcat(DLG->Name, "FerryCross");
      break;
      } /*  */
     case 215:
      {
      strcat(DLG->Name, "ParkingAr");
      break;
      } /*  */
     case 216:
      {
      strcat(DLG->Name, "ArbitrExt");
      break;
      } /*  */
     case 217:
      {
      strcat(DLG->Name, "C3CentLine");
      break;
      } /*  */
     case 218:
      {
      strcat(DLG->Name, "C3Divided");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* if */
   else if (DLG->MinorAttr >= 400 && DLG->MinorAttr < 500)
    {
    strcpy(DLG->Layer1, "PT");
    switch (DLG->MinorAttr)
     {
     case 401:
      {
      strcat(DLG->Name, "TrafCircle");
      break;
      } /*  */
     case 402:
      {
      strcat(DLG->Name, "Interchnge");
      break;
      } /*  */
     case 403:
      {
      strcat(DLG->Name, "TollGate");
      break;
      } /*  */
     case 404:
      {
      strcat(DLG->Name, "WeighSta");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 500)
    {
    strcpy(DLG->Layer1, "MI");
    switch (DLG->MinorAttr)
     {
     case 600:
      {
      strcat(DLG->Name, "Historical");
      break;
      } /*  */
     case 601:
      {
      strcat(DLG->Name, "InTunnel");
      break;
      } /*  */
     case 602:
      {
      strcat(DLG->Name, "Overpassng");
      break;
      } /*  */
     case 603:
      {
      strcat(DLG->Name, "UnderConst");
      break;
      } /*  */
     case 604:
      {
      strcat(DLG->Name, "UnderConst");
      break;
      } /*  */
     case 605:
      {
      strcat(DLG->Name, "OldRRGrade");
      break;
      } /*  */
     case 606:
      {
      strcat(DLG->Name, "Submerged");
      break;
      } /*  */
     case 607:
      {
      strcat(DLG->Name, "Underpassg");
      break;
      } /*  */
     case 608:
      {
      strcat(DLG->Name, "LimitedAcc");
      break;
      } /*  */
     case 609:
      {
      strcat(DLG->Name, "TollRoad");
      break;
      } /*  */
     case 610:
      {
      strcat(DLG->Name, "Private");
      break;
      } /*  */
     case 611:
      {
      strcat(DLG->Name, "Proposed");
      break;
      } /*  */
     case 612:
      {
      strcat(DLG->Name, "DoubleDeck");
      break;
      } /*  */
     case 613:
      {
      strcat(DLG->Name, "RestArea");
      break;
      } /*  */
     case 614:
      {
      strcat(DLG->Name, "Elevated");
      break;
      } /*  */
     case 615:
      {
      strcat(DLG->Name, "BypassRte");
      break;
      } /*  */
     case 616:
      {
      strcat(DLG->Name, "Alternate");
      break;
      } /*  */
     case 617:
      {
      strcat(DLG->Name, "BusinessRt");
      break;
      } /*  */
     case 618:
      {
      strcat(DLG->Name, "Drawbridge");
      break;
      } /*  */
     case 619:
      {
      strcat(DLG->Name, "Spur");
      break;
      } /*  */
     case 650:
      {
      strcat(DLG->Name, "Rd46-55");
      break;
      } /*  */
     case 651:
      {
      strcat(DLG->Name, "Rd56-65");
      break;
      } /*  */
     case 652:
      {
      strcat(DLG->Name, "Rd66-75");
      break;
      } /*  */
     case 653:
      {
      strcat(DLG->Name, "Rd76-85");
      break;
      } /*  */
     case 654:
      {
      strcat(DLG->Name, "Rd86-95");
      break;
      } /*  */
     case 655:
      {
      strcat(DLG->Name, "Rd96-105");
      break;
      } /*  */
     case 656:
      {
      strcat(DLG->Name, "Rd106-115");
      break;
      } /*  */
     case 657:
      {
      strcat(DLG->Name, "Rd116-125");
      break;
      } /*  */
     case 658:
      {
      strcat(DLG->Name, "Rd126-135");
      break;
      } /*  */
     case 659:
      {
      strcat(DLG->Name, "Rd136-145");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else
    {
    SaveObject = 0;
    } /* else */
   break;
   } /* transportation */
  case 180:
   {
   strcpy(DLG->Layer2, "RAI");
   DLG->Color = 3;
   if (DLG->MinorAttr == 0)
    {
    SaveObject = 0;
    break;
    } /* photo-revision */
   if (DLG->MinorAttr >= 200 && DLG->MinorAttr < 300)
    {
    strcpy(DLG->Layer1, "TR");
    switch (DLG->MinorAttr)
     {
     case 201:
      {
      strcat(DLG->Name, "Railroad");
      break;
      } /*  */
     case 202:
      {
      strcat(DLG->Name, "RRInStreet");
      break;
      } /*  */
     case 204:
      {
      strcat(DLG->Name, "Carline");
      break;
      } /*  */
     case 205:
      {
      strcat(DLG->Name, "CogRR");
      break;
      } /*  */
     case 207:
      {
      strcat(DLG->Name, "FerryCross");
      break;
      } /*  */
     case 208:
      {
      strcat(DLG->Name, "RRSiding");
      break;
      } /*  */
     case 209:
      {
      strcat(DLG->Name, "YardLimit");
      break;
      } /*  */
     case 210:
      {
      strcat(DLG->Name, "ArbitrExt");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* if */
   else if (DLG->MinorAttr >= 400 && DLG->MinorAttr < 500)
    {
    strcpy(DLG->Layer1, "PT");
    switch (DLG->MinorAttr)
     {
     case 400:
      {
      strcat(DLG->Name, "RRStation");
      break;
      } /*  */
     case 401:
      {
      strcat(DLG->Name, "Turntable");
      break;
      } /*  */
     case 402:
      {
      strcat(DLG->Name, "Roundhouse");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 500)
    {
    strcpy(DLG->Layer1, "MI");
    switch (DLG->MinorAttr)
     {
     case 600:
      {
      strcat(DLG->Name, "Historical");
      break;
      } /*  */
     case 601:
      {
      strcat(DLG->Name, "InTunnel");
      break;
      } /*  */
     case 602:
      {
      strcat(DLG->Name, "Overpassng");
      break;
      } /*  */
     case 603:
      {
      strcat(DLG->Name, "Abandoned");
      break;
      } /*  */
     case 604:
      {
      strcat(DLG->Name, "Dismantled");
      break;
      } /*  */
     case 605:
      {
      strcat(DLG->Name, "Underpassg");
      break;
      } /*  */
     case 606:
      {
      strcat(DLG->Name, "NarrowGage");
      break;
      } /*  */
     case 607:
      {
      strcat(DLG->Name, "InSnowshed");
      break;
      } /*  */
     case 608:
      {
      strcat(DLG->Name, "UnderConst");
      break;
      } /*  */
     case 609:
      {
      strcat(DLG->Name, "Elevated");
      break;
      } /*  */
     case 610:
      {
      strcat(DLG->Name, "RapidTrans");
      break;
      } /*  */
     case 611:
      {
      strcat(DLG->Name, "Drawbridge");
      break;
      } /*  */
     case 612:
      {
      strcat(DLG->Name, "Private");
      break;
      } /*  */
     case 613:
      {
      strcat(DLG->Name, "USGovmt");
      break;
      } /*  */
     case 614:
      {
      strcat(DLG->Name, "Juxtapos");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else
    {
    SaveObject = 0;
    } /* else */
   break;
   } /* railroad */
  case 190:
   {
   strcpy(DLG->Layer2, "PIP");
   DLG->Color = 7;
   if (DLG->MinorAttr == 0)
    {
    SaveObject = 0;
    break;
    } /* photo-revision */
   if (DLG->MinorAttr >= 200 && DLG->MinorAttr < 300)
    {
    strcpy(DLG->Layer1, "PP");
    switch (DLG->MinorAttr)
     {
     case 201:
      {
      strcat(DLG->Name, "Pipeline");
      break;
      } /*  */
     case 202:
      {
      strcat(DLG->Name, "PowerTrans");
      break;
      } /*  */
     case 203:
      {
      strcat(DLG->Name, "Telephone");
      break;
      } /*  */
     case 204:
      {
      strcat(DLG->Name, "AerialTram");
      break;
      } /*  */
     case 205:
      {
      strcat(DLG->Name, "ArbitrExt");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* if */
   else if (DLG->MinorAttr >= 300 && DLG->MinorAttr < 400)
    {
    strcpy(DLG->Layer1, "SA");
    switch (DLG->MinorAttr)
     {
     case 300:
      {
      strcat(DLG->Name, "SeaplnAnch");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 400 && DLG->MinorAttr < 500)
    {
    strcpy(DLG->Layer1, "PT");
    switch (DLG->MinorAttr)
     {
     case 400:
      {
      strcat(DLG->Name, "PowerStatn");
      break;
      } /*  */
     case 401:
      {
      strcat(DLG->Name, "SubStation");
      break;
      } /*  */
     case 402:
      {
      strcat(DLG->Name, "HydroPlant");
      break;
      } /*  */
     case 403:
      {
      strcat(DLG->Name, "Airport");
      break;
      } /*  */
     case 404:
      {
      strcat(DLG->Name, "Heliport");
      break;
      } /*  */
     case 405:
      {
      strcat(DLG->Name, "LaunchComp");
      break;
      } /*  */
     case 406:
      {
      strcat(DLG->Name, "PumpStatn");
      break;
      } /*  */
     case 407:
      {
      strcat(DLG->Name, "SeaplnRmp");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else if (DLG->MinorAttr >= 500)
    {
    strcpy(DLG->Layer1, "MI");
    switch (DLG->MinorAttr)
     {
     case 600:
      {
      strcat(DLG->Name, "Undergrnd");
      break;
      } /*  */
     case 601:
      {
      strcat(DLG->Name, "UnderConst");
      break;
      } /*  */
     case 602:
      {
      strcat(DLG->Name, "Abandoned");
      break;
      } /*  */
     case 603:
      {
      strcat(DLG->Name, "AboveGrnd");
      break;
      } /*  */
     case 604:
      {
      strcat(DLG->Name, "Closed");
      break;
      } /*  */
     case 605:
      {
      strcat(DLG->Name, "Unimproved");
      break;
      } /*  */
     case 606:
      {
      strcat(DLG->Name, "Submerged");
      break;
      } /*  */
     case 607:
      {
      strcat(DLG->Name, "Nuclear");
      break;
      } /*  */
     default:
      {
      SaveObject = 0;
      break;
      } /* default */
     } /* switch minor attribute code */
    } /* else if */
   else
    {
    SaveObject = 0;
    } /* else */
   break;
   } /* pipeline */
  case 290:
   {
   if (DLG->MinorAttr == 0)
    {
    SaveObject = 0;
    break;
    } /* photo-revision */
   if (DLG->MinorAttr == 2017)
    {
    strcpy(DLG->Layer2, "GEO");
    strcpy(DLG->Layer1, "CD");
    strcat(DLG->Name, "ContDivide");
    DLG->Color = 7;
    }
   else if (DLG->MinorAttr >= 3000 && DLG->MinorAttr < 5000)
    {
    strcpy(DLG->Layer2, "HYD");
    DLG->Color = 6;
    if (DLG->MinorAttr >= 3000 && DLG->MinorAttr < 3070)
     {
     strcpy(DLG->Layer1, "ST");
     strcat(DLG->Name, "Stream");
     }
    else if (DLG->MinorAttr >= 3070 && DLG->MinorAttr < 4000)
     {
     strcpy(DLG->Layer1, "CA");
     strcat(DLG->Name, "Canal");
     }
    else if (DLG->MinorAttr >= 4000 && DLG->MinorAttr < 4040)
     {
     strcpy(DLG->Layer1, "WB");
     strcat(DLG->Name, "WaterBody");
     }
    else if (DLG->MinorAttr >= 4040 && DLG->MinorAttr < 4050)
     {
     strcpy(DLG->Layer1, "MA");
     strcat(DLG->Name, "Marsh");
     }
    else if (DLG->MinorAttr >= 4050 && DLG->MinorAttr < 4060)
     {
     strcpy(DLG->Layer1, "DL");
     strcat(DLG->Name, "DryLake");
     }
    else if (DLG->MinorAttr >= 4060 && DLG->MinorAttr < 5000)
     {
     strcpy(DLG->Layer1, "GL");
     strcat(DLG->Name, "Glacier");
     }
    } /* if */
   else if (DLG->MinorAttr >= 5000 && DLG->MinorAttr < 5070)
    {
    strcpy(DLG->Layer2, "ROA");
    strcpy(DLG->Layer1, "  ");
    strcat(DLG->Name, "Highway");
    DLG->Color = 4;
    } /* if */
   else if (DLG->MinorAttr >= 5070 && DLG->MinorAttr < 6000)
    {
    strcpy(DLG->Layer2, "RAI");
    strcpy(DLG->Layer1, "  ");
    strcat(DLG->Name, "Railroad");
    DLG->Color = 3;
    } /* if */
   else if (DLG->MinorAttr >= 6000 && DLG->MinorAttr < 7000)
    {
    strcpy(DLG->Layer2, "BDY");
    strcpy(DLG->Layer1, "  ");
    strcat(DLG->Name, "Boundary");
    DLG->Color = 5;
    } /* if */
   else if (DLG->MinorAttr >= 7000 && DLG->MinorAttr < 8000)
    {
    strcpy(DLG->Layer2, "CUL");
    strcpy(DLG->Layer1, "  ");
    strcat(DLG->Name, "Culture");
    DLG->Color = 3;
    } /* if */
   else
    {
    DLG->Color = 2;
    strcpy(DLG->Layer1, "  ");
    strcpy(DLG->Layer2, "   ");
    strcat(DLG->Name, "Other");
    }
   break;
   } /* case 290 1:2000000 */
  default:
   {
   DLG->Color = 2;
   strcpy(DLG->Layer1, "  ");
   strcpy(DLG->Layer2, "   ");
   strcat(DLG->Name, "Other");
   break;
   } /* default - not save as object */
  } /* switch major attribute code */

 return (SaveObject);

} /* AssignDLGAttrs() */

/***********************************************************************/

struct WDBCoordinates {
 short Attr, Lat, Lon;
};

short ImportWDB(void)
{
char filename[255], WDBfile[32], WDBpath[256];
FILE *fWDB;
short error = 0, Code, ReadyToSave = 0, *TempElev;
double *TempLon, *TempLat, Lat, Lon;
struct DLGInfo DLG;
struct WDBCoordinates WDBCoords;
struct BusyWindow *BWDL = NULL;

 TempLon = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempLat = (double *)get_Memory(MAXOBJPTS * sizeof (double), MEMF_ANY);
 TempElev = (short *)get_Memory(MAXOBJPTS * sizeof (short), MEMF_ANY);
 
 if (! TempLon || ! TempLat || ! TempElev)
  {
  User_Message("Data Ops Module: Import WDB",
	"Out of memory allocating temporary arrays!\nOperation terminated.",
	"OK", "o");
  error = 1;
  goto Cleanup;
  } /* if no database loaded */

 strcpy(WDBpath, dirname);
 WDBfile[0] = 0;
 if (! getfilename(0, "WDB File", WDBpath, WDBfile)) goto Cleanup;

 if (WDBfile[0] == 0)
  {
  User_Message("Data Ops Module: Import WDB",
	"No file(s) selected!",	"OK", "o");
  goto Cleanup;
  } /* if no file */

 strmfp(filename, WDBpath, WDBfile);

 if ((fWDB = fopen(filename, "rb")) == NULL)
  {
  User_Message("Data Ops Module: Import WDB",
	"Can't open WDB file for input!\nOperation terminated.", "OK", "o");
  Log(ERR_OPEN_FAIL, WDBfile);
  error = 2;
  goto Cleanup;
  } /* if file open failed */

 DLG.Pairs = 0;

 fseek(fWDB, 0L, SEEK_END);
 BWDL = BusyWin_New("Reading", ftell(fWDB), 0, 'BWDL');
 fseek(fWDB, 0L, SEEK_SET);

 while (1)

  {
  if ((fread(&WDBCoords, sizeof (struct WDBCoordinates), 1, fWDB)) != 1)
   break;
  DLG.Pairs ++;
  Code = WDBCoords.Attr / 1000;
  Lat = (float)(WDBCoords.Lat / 60) + (float)(WDBCoords.Lat % 60) / 60.0;
  Lon = -((float)(WDBCoords.Lon / 60) + (float)(WDBCoords.Lon % 60) / 60.0);
  if (Lon < 0.0) Lon += 360.0;
  if (Code > 0)
   {
   if (ReadyToSave)
    {
    ReadyToSave = 0;
    DBase[OBN].Points = DLG.Pairs - 1;
    strmfp(filename, dirname, DLG.Name);
    strcat(filename, ".Obj");
    if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
     {
     error = 4;
     }
    if (DE_Win)
     {
     if (! Add_DE_NewItem())
      {
      User_Message("Database Module",
		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
      break;
      } /* if new list fails */
     } /* if database editor open */
    if (error)
     break;
    if (CheckInput_ID() == ID_BW_CLOSE)
     break;
    BusyWin_Update(BWDL, ftell(fWDB));
    } /* if ready to save */

   switch (Code)
    {
    case 1:
     {
     strcpy(DLG.Name, "Coast");
     strcpy(DLG.Layer1, "CN");
     strcpy(DLG.Layer2, "CST");
     DLG.Color = 5;
     break;
     } /* coastline */
    case 2:
     {
     strcpy(DLG.Name, "Country");
     strcpy(DLG.Layer1, "CO");
     strcpy(DLG.Layer2, "BDY");
     DLG.Color = 3;
     break;
     } /* country boundary */
    case 4:
     {
     strcpy(DLG.Name, "State");
     strcpy(DLG.Layer1, "ST");
     strcpy(DLG.Layer2, "BDY");
     DLG.Color = 3;
     break;
     } /* state boundary */
    case 5:
     {
     strcpy(DLG.Name, "Island");
     strcpy(DLG.Layer1, "IS");
     strcpy(DLG.Layer2, "CST");
     DLG.Color = 5;
     break;
     } /* island */
    case 6:
     {
     strcpy(DLG.Name, "Lake");
     strcpy(DLG.Layer1, "LK");
     strcpy(DLG.Layer2, "HYD");
     DLG.Color = 6;
     break;
     } /* lake */
    case 7:
     {
     strcpy(DLG.Name, "River");
     strcpy(DLG.Layer1, "RV");
     strcpy(DLG.Layer2, "HYD");
     DLG.Color = 6;
     break;
     } /* river */
    default:
     {
     error = 5;
     break;
     } /* unsupported code */
    } /* switch */
   DLG.LineWidth = 1;

/* set new name and database fields */

   if (NoOfObjects + 1 > DBaseRecords)
    {
    struct database *NewBase;

    if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 100)) == NULL)
     {
     error = 7;
     break;
     } /* if new database allocation fails */
    else
     {
     DBase = NewBase;
     DBaseRecords += 100;
     } /* else new database allocated and copied */
    } /* if need larger database memory */

   if ((ReadyToSave = OpenObjFile(DLG.Name)) == 0)
    {
    error = 3;
    break;
    } /* if open error */
   else
    {
    TempLat[0] = TempLat[1] = Lat;
    TempLon[0] = TempLon[1] = Lon;
    TempElev[0] = TempElev[1] = 0;
    DLG.Pairs = 1;
    Set_DLGObject(&DLG);
    } /* else open OK */

   } /* if header record, Code > 0 start new object, save the old */

  else
   {
   if (DLG.Pairs >= MAXOBJPTS)
    {
    if (ReadyToSave)
     {
     ReadyToSave = 0;
     DBase[OBN].Points = DLG.Pairs - 1;
     strmfp(filename, dirname, DLG.Name);
     strcat(filename, ".Obj");
     if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
      {
      error = 4;
      }
     if (DE_Win)
      {
      if (! Add_DE_NewItem())
       {
       User_Message("Database Module",
		"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
       break;
       } /* if new list fails */
      } /* if database editor open */
     if (error)
      break;
     if (CheckInput_ID() == ID_BW_CLOSE)
      break;
     BusyWin_Update(BWDL, ftell(fWDB));
     } /* if ready to save */

    if (NoOfObjects + 1 > DBaseRecords)
     {
     struct database *NewBase;

     if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 100)) == NULL)
      {
      error = 7;
      break;
      } /* if new database allocation fails */
     else
      {
      DBase = NewBase;
      DBaseRecords += 100;
      } /* else new database allocated and copied */
     } /* if need larger database memory */

    if ((ReadyToSave = OpenObjFile(DLG.Name)) == 0)
     {
     error = 3;
     break;
     } /* if open error */
    else
     {
     TempLat[0] = TempLat[1] = TempLat[DLG.Pairs - 1];
     TempLon[0] = TempLon[1] = TempLon[DLG.Pairs - 1];
     TempElev[0] = TempElev[1] = 0;
     DLG.Pairs = 2;
     Set_DLGObject(&DLG);
     } /* else open OK */

    } /* if too many points */

   TempLat[DLG.Pairs] = Lat;
   TempLon[DLG.Pairs] = Lon;
   TempElev[DLG.Pairs] = 0;
   } /* else point record */

  } /* while */

 fclose(fWDB);

 if (ReadyToSave && ! error)
  {
  DBase[OBN].Points = DLG.Pairs;
  strmfp(filename, dirname, DLG.Name);
  strcat(filename, ".Obj");
  if (saveobject(OBN, filename, &TempLon[0], &TempLat[0], &TempElev[0]))
   {
   error = 4;
   }
  if (DE_Win)
   {
   if (! Add_DE_NewItem())
    {
    User_Message("Database Module",
	"Out of memory expanding Database Editor List!\nOperation terminated.", "OK", "o");
    } /* if new list fails */
   } /* if database editor open */
  } /* if readt to save open */

Cleanup:

 if (TempLon) free_Memory(TempLon, MAXOBJPTS * sizeof (double));
 if (TempLat) free_Memory(TempLat, MAXOBJPTS * sizeof (double));
 if (TempElev) free_Memory(TempElev, MAXOBJPTS * sizeof (short));

 if (BWDL)
  BusyWin_Del(BWDL);

 switch (error)
  {
  case 1:
   {
   User_Message("Data Ops: Import WDB",
		"Out of memory!\nOperation terminated.", "OK", "o");
   break;
   } /* memory */
  case 2:
   {
   User_Message("Data Ops: Import WDB",
		"Error opening source file!\nOperation terminated.", "OK", "o");
   break;
   } /* source file open */
  case 3:
   {
   User_Message("Data Ops: Import WDB",
		"Error opening output file!\nOperation terminated.", "OK", "o");
   break;
   } /* out file open */
  case 4:
   {
   User_Message("Data Ops: Import WDB",
		"Error saving object file!\nOperation terminated.", "OK", "o");
   break;
   } /*  out file write */
  case 5:
   {
   User_Message("Data Ops: Import WDB",
		"Unsupported attribute code!\nOperation terminated.", "OK", "o");
   break;
   } /*  unsupported code */
  case 6:
   {
   User_Message("Data Ops: Import WDB",
		"Object contains too many points!\nOperation terminated.", "OK", "o");
   break;
   } /*  too many points */
  case 7:   {
   User_Message("Data Ops: Import WDB",
		"Out of memory expanding database!\nOperation terminated.", "OK", "o");
   break;
   } /*  database memory */
  } /* switch */

 return (1);

} /* ImportWDB() */
