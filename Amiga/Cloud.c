/* Cloud.c
** Cloud generation and manipulation functions for WCS.
** Written by Gary R. Huber, Jan 1995.
*/

#include "WCS.h"
#include "GenericParams.h"
#include "Cloud.h"
#include "Wave.h"

/*********************************************************************/

struct CloudLayer *CloudLayer_New(void)
{

 return ((struct CloudLayer *)
	get_Memory(sizeof (struct CloudLayer), MEMF_CLEAR));

} /* CloudLayer_New() */

/*********************************************************************/

void CloudLayer_Del(struct CloudLayer *CL)
{

 if (CL)
  free_Memory(CL, sizeof (struct CloudLayer));

} /* CloudLayer_Del() */

/**************************:*******************************************/

void CloudLayer_DelAll(struct CloudLayer *CL)
{
struct CloudLayer *CLDel;

 while (CL)
  {
  CLDel = CL;
  CL = CL->Next;
  free_Memory(CLDel, sizeof (struct CloudLayer));
  } /* while */

} /* CloudLayer_Del() */

/*********************************************************************/

void CloudLayer_SetDouble(struct CloudLayer *CL, ULONG Item, double Val)
{

 switch (Item)
  {
  case CLOUDLAYER_ALT:
   {
   CL->Alt = Val;
   break;
   }
  } /* switch */

} /* CloudLayer_Set() */

/**********************************************************************/

void CloudLayer_SetShort(struct CloudLayer *CL, ULONG Item, short Val)
{

 switch (Item)
  {
  case CLOUDLAYER_COVG:
   {
   CL->Covg = Val;
   break;
   }
  case CLOUDLAYER_DENS:
   {
   CL->Dens = Val;
   break;
   }
  case CLOUDLAYER_ILLUM:
   {
   CL->Illum = Val;
   break;
   }
  } /* switch */

} /* CloudLayer_Set() */

/**********************************************************************/

struct CloudData *CloudData_New(void)
{
struct CloudData *CD;

 if ((CD = (struct CloudData *)
	get_Memory(sizeof (struct CloudData), MEMF_CLEAR)))
  {
  if ((CD->WD = WaveData_New()) == NULL)
   {
   CloudData_Del(CD);
   return (NULL);
   } /* if */
  } /* if */

 return (CD);

} /* CloudData_New() */

/*********************************************************************/

void CloudData_Del(struct CloudData *CD)
{

 if (CD)
  {
  if (CD->CloudPlane)
   free_Memory(CD->CloudPlane, CD->PlaneSize);
  if (CD->WD)
   WaveData_Del(CD->WD);
  if (CD->Layer)
   CloudLayer_DelAll(CD->Layer);
  if (CD->KT)
   FreeGenericKeyTable(&CD->KT, &CD->KT_MaxFrames);
  if (CD->CloudKey)
   free_Memory(CD->CloudKey, CD->KFSize);

  free_Memory(CD, sizeof (struct CloudData));
  } /* if */

} /* CloudData_Del() */

/*********************************************************************/

void CloudData_SetLong(struct CloudData *CD, ULONG Item, long Val)
{

 switch (Item)
  {
  case CLOUDDATA_RANDSEED:
   {
   CD->RandSeed = Val;
   break;
   }
  case CLOUDDATA_ROWS:
   {
   CD->Rows = Val;
   break;
   }
  case CLOUDDATA_COLS:
   {
   CD->Cols = Val;
   break;
   }
  } /* switch */

} /* CloudData_SetLong() */

/*********************************************************************/

long CloudData_GetLong(struct CloudData *CD, ULONG Item)
{

 switch (Item)
  {
  case CLOUDDATA_RANDSEED:
   {
   return (CD->RandSeed);
   break;
   }
  case CLOUDDATA_ROWS:
   {
   return (CD->Rows);
   break;
   }
  case CLOUDDATA_COLS:
   {
   return (CD->Cols);
   break;
   }
  default:
   {
      printf("Invalid Item %lu in %s %d()",Item,__FILE__,__LINE__);
      return 0;
   }
  } /* switch */

} /* CloudData_GetLong() */

/*********************************************************************/

void CloudData_SetShort(struct CloudData *CD, ULONG Item, short Val)
{

 switch (Item)
  {
  case CLOUDDATA_NUMKEYS:
   {
   CD->NumKeys = Val;
   break;
   }
  case CLOUDDATA_NUMWAVES:
   {
   CD->WD->NumWaves = Val;
   break;
   }
  case CLOUDDATA_NUMLAYERS:
   {
   CD->NumLayers = Val;
   break;
   }
  case CLOUDDATA_DYNAMIC:
   {
   CD->Dynamic = Val;
   break;
   }
  case CLOUDDATA_CLOUDTYPE:
   {
   CD->CloudType = Val;
   break;
   }
  } /*switch */

} /* CloudData_SetShort() */

/**********************************************************************/

short CloudData_GetShort(struct CloudData *CD, ULONG Item)
{

 switch (Item)
  {
  case CLOUDDATA_NUMKEYS:
   {
   return (CD->NumKeys);
   break;
   }
  case CLOUDDATA_NUMWAVES:
   {
   return (CD->WD->NumWaves);
   break;
   }
  case CLOUDDATA_NUMLAYERS:
   {
   return (CD->NumLayers);
   break;
   }
  case CLOUDDATA_DYNAMIC:
   {
   return (CD->Dynamic);
   break;
   }
  case CLOUDDATA_CLOUDTYPE:
   {
   return (CD->CloudType);
   break;
   }
  default:
   {
      printf("Invalid Item %lu in %s %d()",Item,__FILE__,__LINE__);
      return 0;
   }

  } /*switch */

} /* CloudData_GetShort() */

/**********************************************************************/

void CloudData_SetDouble(struct CloudData *CD, ULONG Item, double Val)
{

 switch (Item)
  {
  case CLOUDDATA_COVERAGE:
   {
   CD->Coverage = Val;
   break;
   }
  case CLOUDDATA_DENSITY:
   {
   CD->Density = Val;
   break;
   }
  case CLOUDDATA_NWLAT:
   {
   CD->Lat[0] = Val;
   break;
   }
  case CLOUDDATA_NWLON:
   {
   CD->Lon[0] = Val;
   break;
   }
  case CLOUDDATA_SELAT:
   {
   CD->Lat[1] = Val;
   break;
   }
  case CLOUDDATA_SELON:
   {
   CD->Lon[1] = Val;
   break;
   }
  case CLOUDDATA_MAXALT:
   {
   CD->AltDiff = Val;
   break;
   }
  case CLOUDDATA_MINALT:
   {
   CD->Alt = Val;
   break;
   }
  case CLOUDDATA_STDDEV:
   {
   CD->StdDev = Val;
   break;
   }
  case CLOUDDATA_H:
   {
   CD->H = Val;
   break;
   }
  case CLOUDDATA_LATOFF:
   {
   CD->LatOff = Val;
   break;
   }
  case CLOUDDATA_LONOFF:
   {
   CD->LonOff = Val;
   break;
   }
  } /* switch */

} /* CloudData_SetDouble() */

/**********************************************************************/

double CloudData_GetDouble(struct CloudData *CD, ULONG Item)
{

 switch (Item)
  {
  case CLOUDDATA_COVERAGE:
   {
   return (CD->Coverage);
   break;
   }
  case CLOUDDATA_DENSITY:
   {
   return (CD->Density);
   break;
   }
  case CLOUDDATA_NWLAT:
   {
   return (CD->Lat[0]);
   break;
   }
  case CLOUDDATA_NWLON:
   {
   return (CD->Lon[0]);
   break;
   }
  case CLOUDDATA_SELAT:
   {
   return (CD->Lat[1]);
   break;
   }
  case CLOUDDATA_SELON:
   {
   return (CD->Lon[1]);
   break;
   }
  case CLOUDDATA_MAXALT:
   {
   return (CD->AltDiff);
   break;
   }
  case CLOUDDATA_MINALT:
   {
   return (CD->Alt);
   break;
   }
  case CLOUDDATA_STDDEV:
   {
   return (CD->StdDev);
   break;
   }
  case CLOUDDATA_H:
   {
   return (CD->H);
   break;
   }
  case CLOUDDATA_LATOFF:
   {
   return (CD->LatOff);
   break;
   }
  case CLOUDDATA_LONOFF:
   {
   return (CD->LonOff);
   break;
   }
  default:
   {
      printf("Invalid Item %lu in %s %d()",Item,__FILE__,__LINE__);
      return 0;
   }

  } /* switch */

} /* CloudData_GetDouble() */

/**********************************************************************/

short Cloud_SetBounds(struct CloudData *CD)
{
short error = 0;
long XRange, YRange, Low_X, Low_Y, High_X, High_Y;
struct Box Bx;

if (! MapWind0)
  {
  if (User_Message_Def("Cloud Editor:Set Bounds", "Map View Module must be open in order\
 to use this funcion. Would you like to open it now?", "OK|Cancel", "oc",1))
   {
   map();

   if (! MapWind0)
    return (0);

   } /* else */
  else
   return (0);
  } /* if */

StartAlign:
 MapGUI_Message(0, "\0338Set northwest corner with mouse.");
 SetWindowTitles(MapWind0, "Set northwest corner", (UBYTE *)-1);

 if (! MousePtSet(&Bx.Low, NULL, 0))
  {
  error = 1;
  goto EndAlign;
  } /* if aborted */

 MapGUI_Message(0, "\0338Set southeast corner. ESC=abort");
 SetWindowTitles(MapWind0, "Set southeast corner", (UBYTE *)-1);

 if (! MousePtSet(&Bx.High, &Bx.Low, 2))
  {
  error = 1;
  goto EndAlign;
  } /* if aborted */

 if (Bx.Low.X == Bx.High.X || Bx.Low.Y == Bx.High.Y)
  {
  if (User_Message_Def("Mapping Module: Align",
	"Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?",
	"OK|Cancel", "oc", 1))
   {
   goto StartAlign;
   } /* if try again */
  error = 1;
  goto EndAlign;
  } /* illegal values */

 if (Bx.Low.X > Bx.High.X)
  swmem(&Bx.Low.X, &Bx.High.X, sizeof (short));
 if (Bx.Low.Y > Bx.High.Y)
  swmem(&Bx.Low.Y, &Bx.High.Y, sizeof (short));

 SetAPen(MapWind0->RPort, 7);
 Move(MapWind0->RPort, Bx.Low.X,  Bx.Low.Y);
 Draw(MapWind0->RPort, Bx.High.X, Bx.Low.Y);
 Draw(MapWind0->RPort, Bx.High.X, Bx.High.Y);
 Draw(MapWind0->RPort, Bx.Low.X,  Bx.High.Y);
 Draw(MapWind0->RPort, Bx.Low.X,  Bx.Low.Y);

EndAlign:
 MapGUI_Message(0, " ");
 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, "Map View", (UBYTE *)-1);
 if (error)
  return (0);

 Low_X = Bx.Low.X;
 Low_Y = Bx.Low.Y;
 High_X = Bx.High.X;
 High_Y = Bx.High.Y;

 XRange = Bx.High.X - Bx.Low.X;
 YRange = Bx.High.Y - Bx.Low.Y;
 while (XRange % 8)
  {
  High_X ++;
  XRange ++;
  }
 while (YRange % 8)
  {
  High_Y ++;
  YRange ++;
  }
 XRange ++;
 YRange ++;

 CD->Rows = YRange;
 CD->Cols = XRange;
 CD->Lon[0] = X_Lon_Convert((long)Low_X);
 CD->Lat[0] = Y_Lat_Convert((long)Low_Y);
 CD->Lon[1] = X_Lon_Convert((long)High_X);
 CD->Lat[1] = Y_Lat_Convert((long)High_Y);

 return (1);

} /* Cloud_SetBounds() */

/**********************************************************************/

short Cloud_Generate(struct CloudData *CD, double Frame)
{
short error = 0, success = 0;
long x, y, zip;
double ptlat, ptlon, avglat, d1, d2, dist, lonscale, waveamp,
	LatStep, LonStep;
struct BusyWindow *BWMD;
struct Wave *WV;

 if (CD->CloudPlane)
  free_Memory(CD->CloudPlane, CD->PlaneSize);

 while ((CD->Cols - 1) % 8)
  {
  CD->Cols ++;
  }
 while ((CD->Rows - 1) % 8)
  {
  CD->Rows ++;
  }

 CD->PlaneSize = CD->Cols * CD->Rows * sizeof (float);
 if (CD->PlaneSize <= 0 || CD->Rows <= 1 || CD->Cols <= 1)
  return (0);

 if ((CD->CloudPlane = get_Memory(CD->PlaneSize, MEMF_ANY)) == NULL)
  return (0);

//Repeat:

 if (Frame < 0.0)
  {
  strcpy(str, "");
  GetInputString("Enter Frame Number.", 
	 ",abcdefghijklmnopqrstuvwxyz", str);
  Frame = atof(str);
  } /* if no frame number supplied */

 LatStep = 8.0 * (CD->Lat[1] - CD->Lat[0]) / (CD->Rows - 1);
 LonStep = 8.0 * (CD->Lon[1] - CD->Lon[0]) / (CD->Cols - 1);

 BWMD = BusyWin_New("Computing...", CD->Rows, 0, 'BWMD');

 zip = 0;
 for (y=0, ptlat=CD->Lat[0] + CD->LatOff; y<=CD->Rows; y+=8, ptlat+=LatStep)
  {
  for (x=0, zip=y*CD->Cols, ptlon=CD->Lon[0] + CD->LonOff; x<=CD->Cols; x+=8, zip+=8, ptlon+=LonStep)
   {
   waveamp = 0.0;
   if (CD->WD)
    {
    WV = CD->WD->Wave;
    while (WV)
     { /* compute wave height in kilometers */
     avglat = (WV->Pt.Lat + CD->LatOff + ptlat) / 2.0;
     lonscale = LATSCALE * cos(avglat * PiOver180);
     d1 = (WV->Pt.Lat + CD->LatOff - ptlat) * LATSCALE;
     d2 = (WV->Pt.Lon + CD->LonOff - ptlon) * lonscale;
     dist = sqrt(d1 * d1 + d2 * d2);
     waveamp += (WV->Amp * sin(TwoPi * ((dist / WV->Length)
	- (Frame * WV->Velocity) / (WV->Length * FRAMES_PER_HOUR))));
     WV = WV->Next;
     } /* while */
    } /* if */

   CD->CloudPlane[zip] = waveamp;
   } /* for x=... */

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWMD, y + 1);
  } /* for y=... */

 BusyWin_Del(BWMD);

/* Fractal interpolate */

 if (! error)
  success = Raster_Fract(CD->CloudPlane, CD->Cols, CD->Rows, CD->RandSeed,
	CD->StdDev, CD->H, 3);

 return (success);

} /* Cloud_Generate() */

/**********************************************************************/

short Cloud_Draw(struct CloudData *CD)
{
short col, success = 1;
long x, y, k, Low_X, Low_Y, High_X, High_Y, zip, RowZip, Row, Col;
double MaxAmp, MinAmp, CloudMinAmp, CloudRangeAmp, DataRow, DataCol, Density,
	LatStep, LonStep;
struct BusyWindow *BWMD;
struct clipbounds cb;

 if (! MapWind0 || ! CD->CloudPlane)
  return (0);

 setclipbounds(MapWind0, &cb);

 Low_X = Lon_X_Convert(CD->Lon[0] + CD->LonOff);
 Low_Y = Lat_Y_Convert(CD->Lat[0] + CD->LatOff);
 High_X = Lon_X_Convert(CD->Lon[1] + CD->LonOff);
 High_Y = Lat_Y_Convert(CD->Lat[1] + CD->LatOff);
 LatStep = ((double)(CD->Rows - 1)) / ((double)(High_Y - Low_Y));
 LonStep = ((double)(CD->Cols - 1)) / ((double)(High_X - Low_X));

/* find largest cloud amplitudes */

 zip = 0;
 MaxAmp = -10000.0;
 MinAmp = 10000.0;
 for (y=0; y<CD->Rows; y++)
  {
  for (x=0; x<CD->Cols; x++, zip++)
   {
   if (CD->CloudPlane[zip] > MaxAmp)
    MaxAmp = CD->CloudPlane[zip];
   if (CD->CloudPlane[zip] < MinAmp)
    MinAmp = CD->CloudPlane[zip];
   } /* for y=... */
  } /* for x=... */

 CloudMinAmp = MinAmp + (((MaxAmp - MinAmp) * (100.0 - CD->Coverage)) / 100.0);
 CloudRangeAmp = MaxAmp - CloudMinAmp;

/* plot color in Map View, brighter indicates higher wave amplitude */

 BWMD = BusyWin_New("Drawing...", High_Y - Low_Y + 1, 0, 'BWMD');

 Density = CD->Density / 100.0;

 DataRow = 0.0;
 for (y=Low_Y, k=0; y<=High_Y; y++, DataRow+=LatStep, k++)
  {
  if (y < cb.lowy)
   continue;
  if (y > cb.highy)
   break;
  Row = DataRow;
  RowZip = Row * CD->Cols;
  DataCol = 0.0;
  for (x=Low_X; x<=High_X; x++, DataCol+=LonStep)
   {
   if (x < cb.lowx)
    continue;
   if (x > cb.highx)
    break;
   Col = DataCol;
   zip = RowZip + Col;

   if (CD->CloudPlane[zip] > CloudMinAmp)
    {
    col = 15.99 - (Density * (CD->CloudPlane[zip] - CloudMinAmp)
	 / CloudRangeAmp) * 7.99;
    if (col < 8)
     col = 8;
    }
   else
    col = 1;
   SetAPen(MapWind0->RPort, col);
   WritePixel(MapWind0->RPort, x, y);
   } /* for x=... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   success = 0;
   break;
   } /* if user abort */
  BusyWin_Update(BWMD, k + 1);
  } /* for y=... */

 if (BWMD) BusyWin_Del(BWMD);

 return (success);

} /* Cloud_Draw() */

/**********************************************************************/

void Cloud_SetDefaults(struct CloudData *CD, short CloudType, short SetAll)
{
short i;
struct CloudLayer *CL, *CLPrev;

 if (CD->Layer)
  CloudLayer_DelAll(CD->Layer);
 CD->CloudType = CloudType;

 if (SetAll)
  {
  CD->RandSeed = 1000;
  CD->StdDev = 2.0;
  CD->H = .5;
  } /* if */

 switch (CloudType)
  {
  case CLOUDTYPE_CIRRUS:
   {
   CD->NumLayers = 1;
   CD->Alt = 6.0;
   CD->Coverage = 50.0;
   CD->Density = 50.0;
   break;
   }
  case CLOUDTYPE_STRATUS:
   {
   CD->NumLayers = 3;
   CD->Alt = 5.0;
   CD->Coverage = 100.0;
   CD->Density = 100.0;
   break;
   }
  case CLOUDTYPE_NIMBUS:
   {
   CD->NumLayers = 20;
   CD->Alt = 5.0;
   CD->Coverage = 75.0;
   CD->Density = 100.0;
   break;
   }
  case CLOUDTYPE_CUMULUS:
   {
   CD->NumLayers = 60;
   CD->Alt = 5.0;
   CD->Coverage = 50.0;
   CD->Density = 150.0;
   break;
   }
  } /* switch */

 CD->AltDiff = .0083;

 CL = CD->Layer = CloudLayer_New();
 for (i=1; i<CD->NumLayers; i++)
  {
  CLPrev = CL;
  CL->Next = CloudLayer_New();
  CL = CL->Next;
  CL->Prev = CLPrev;
  } /* for i=0... */

 for (i=0, CL=CD->Layer; CL; i++, CL=CL->Next)
  {
  CL->Alt = CD->Alt + i * .0083;
  switch (CloudType)
   {
   case CLOUDTYPE_CIRRUS:
    {
    CL->Covg = 100.0;
    CL->Dens = 100.0;
    CL->Illum = 1.0;
    break;
    }
   case CLOUDTYPE_STRATUS:
    {
    CL->Covg = 100.0 - ((1.0 - (i / 2.0)) * 100.0 * .5);
    CL->Dens = 100.0;
    CL->Illum = 1.0 - ((1.0 - (i / 2.0)) * .25);
    break;
    }
   case CLOUDTYPE_NIMBUS:
    {
    if (i <= 10)
     {
     CL->Covg = 100.0 - ((1.0 - (i / 10.0)) * 100.0 * .5);
     CL->Dens = 100.0;
     CL->Illum = 1.0 - ((1.0 - (i / 10.0)) * .5);
     }
    else
     {
     CL->Covg = 100.0 - (((i - 10) / 9.0) * 100.0 * .75);
     CL->Dens = 100.0;
     CL->Illum = 1.0;
     }
    break;
    }
   case CLOUDTYPE_CUMULUS:
    {
    if (i <= 20)
     {
     CL->Covg = 100.0 - ((1.0 - (i / 20.0)) * 100.0 * .5);
     CL->Dens = 100.0;
     CL->Illum = 1.0 - ((1.0 - (i / 20.0)) * .5);
     }
    else
     {
     CL->Covg = 100.0 - (((i - 20) / 39.0) * 100.0 * .75);
     CL->Dens = 100.0;
     CL->Illum = 1.0;
     }
    break;
    }
   } /* switch */
  } /* for i=0... */

} /* Cloud_SetDefaults() */

/**********************************************************************/

void CloudWave_Init(struct CloudData *CD, short Frame)
{
double Alt;
struct Wave *WV = NULL;
struct CloudLayer *CL;

 if (CD->WD)
  WV = CD->WD->Wave;
 CL = CD->Layer;
 
 if (Frame > CD->KT_MaxFrames)
  Frame = CD->KT_MaxFrames;

 if (CD->KT->Key)
  {
  CD->Coverage = CD->KT->Val[0][Frame];
  CD->Density  = CD->KT->Val[1][Frame];
  CD->StdDev = CD->KT->Val[2][Frame];
  CD->H = CD->KT->Val[3][Frame];
  CD->Alt = CD->KT->Val[4][Frame];
  CD->LatOff = CD->KT->Val[5][Frame];
  CD->LonOff = CD->KT->Val[6][Frame];
  } /* if */

 Alt = CD->Alt;
 while (CL)
  {
  CL->Alt = Alt;
  Alt += CD->AltDiff;
  CL = CL->Next;
  } /* while */

} /* CloudWave_Init() */

/**********************************************************************/

short BuildCloudKeyTable(struct CloudData *CD)
{

return (BuildGenericKeyTable(&CD->KT, CD->CloudKey, CD->NumKeys,
		&CD->KT_MaxFrames, 3, 0, 7, WCS_KFPRECISION_FLOAT, NULL));

} /* BuildCloudKeyTable() */

/*********************************************************************/
