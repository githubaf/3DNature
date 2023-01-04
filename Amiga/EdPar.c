/* EdPar.c (ne gisedpar.c 14 Jan 1994 CXH)
** Parameter editing function for WCS
** Original code built 9 September, 1992 by Gary R. Huber.
** Incorporated into GIS on 27 July, 1993 by Gary R. Huber.
*/

#include "WCS.h"
#include "Foliage.h"

// AF, 10.Dec.22
#include "Useful.h"

STATIC_VAR short UndoKeyFrames;

STATIC_FCN short Set_Bank_Key(short Frame); // used locally only -> static, AF 23.7.2021
STATIC_FCN short loadparamsV2(USHORT loadcode, short loaditem, char *parampath,
        char *paramfile, struct ParHeader *TempHdr, short ExistingKeyFrames); // used locally only -> static, AF 23.7.2021
STATIC_FCN short loadparamsV1(USHORT loadcode, short loaditem, char *parampath,
        char *paramfile, struct ParHeader *TempHdr, short ExistingKeyFrames); // used locally only -> static, AF 23.7.2021
STATIC_FCN void ParamFmtV1V2_Convert(struct AnimationV1 *MoParV1, struct PaletteV1 *CoParV1,
        union EnvironmentV1 *EcoParV1, struct SettingsV1 *settingsV1,
        struct ParHeaderV1 *ParHdrV1, union KeyFrameV1 *KFV1, USHORT loadcode,
        short loaditem, short LoadKeys); // used locally only -> static, AF 23.7.2021


#ifdef KJHKJDFHKDHFKJDFH // Now in LWSupport.c

/* ALEXANDER now in WCS.h struct LightWaveMotion {
 double XYZ[3], HPB[3], SCL[3];
 long Frame, Linear;
 double TCB[3];
};*/

#define SYSTEM_FLAT 	0
#define SYSTEM_SPHERE 	1

/*
System = 0 = Flat, 1 = Sphere
Units = 0 = km, 1 = meters, 2 = cm, 3 = mi, 4 = ft, 5 = in
Scale
*/

void ExportWave(struct LightWaveInfo *LWInfo)
{
 char FileType[5];
 short Version, Channels, TotalKeys = 0, i, error = 0, Key, LastFrame;
 long LWMsize;
 double Scale;
 struct LightWaveMotion *LWM = NULL;
 FILE *fLWM;

 SetPointer(LW_Win->Win, WaitPointer, 16, 16, -6, 0);

 if (! paramsloaded)
  {
  error = 1;
  goto EndWave;
  } /* if no params */

/* determine which coordinate system to use */
//
// if (LWInfo->System == 1)
//  {
//  error = 6;
//  goto EndWave;
//  } // if spherical system (not yet supported)
//
/* set scale factor for appropriate units of measure */

 switch (LWInfo->Units)
  {
  case 0:
   {
   Scale = ELSCALE_KILOM;
   break;
   } /* Kilometers */
  case 1:
   {
   Scale = 1.0 / ELSCALE_METERS;
   break;
   } /* Meters */
  case 2:
   {
   Scale = 1.0 / ELSCALE_CENTIM;
   break;
   } /* Centimeters */
  case 3:
   {
   Scale = 1.0 / ELSCALE_MILES;
   break;
   } /* Miles */
  case 4:
   {
   Scale = 1.0 / ELSCALE_FEET;
   break;
   } /* Feet */
  case 5:
   {
   Scale = 1.0 / ELSCALE_INCHES;
   break;
   } /* Inches */
  } /* switch */

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

 LWMsize = TotalKeys * sizeof (struct LightWaveMotion);
 if ((LWM = (struct LightWaveMotion *)get_Memory(LWMsize, MEMF_CLEAR)) == NULL)
  {
  error = 3;
  goto EndWave;
  } /* if out of memory */

 if (! BuildKeyTable())
  {
  error = 3;
  goto EndWave;
  } /* if no key table = out of memory */

/* fill in LightWaveMotion structure */

 strcpy(FileType, "LWMO");
 Version = 1;
 Channels = 9;

 Key = 0;
 LastFrame = -1;
 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  if (KF[i].MoKey.Group == 0 && (KF[i].MoKey.Item < 3 || KF[i].MoKey.Item == 8
	|| (KF[i].MoKey.Item < 6 && ! settings.lookahead)))
   {
   if (KF[i].MoKey.KeyFrame != LastFrame)
    {
    LWM[Key].TCB[0] = KF[i].MoKey.TCB[0];
    LWM[Key].TCB[1] = KF[i].MoKey.TCB[1];
    LWM[Key].TCB[2] = KF[i].MoKey.TCB[2];
    if (Set_LWM(LWM, LWInfo, Key, KF[i].MoKey.KeyFrame, Scale, KF[i].MoKey.Linear))
     Key ++;
    LastFrame = KF[i].MoKey.KeyFrame;
    } /* frame number changed, compute position info for last key frame */ 
   } /* if a vital key frame */
  }/* for Frame=1... */

 if ((fLWM = fopen(LWInfo->Name, "w")) == NULL)
  {
  error = 4;
  goto EndWave;
  } /* if open fail */

 fprintf(fLWM, "%s\n%1d\n%1d\n%1d\n\n", FileType, Version, Channels, Key);
 for (i=0; i<Key; i++)
  {
  if (fprintf(fLWM, "%f %f %f %f %f %f %f %f %f\n",/* 1.6 1.1 */
	LWM[i].XYZ[0], LWM[i].XYZ[1], LWM[i].XYZ[2],
	LWM[i].HPB[0], LWM[i].HPB[1], LWM[i].HPB[2],
	LWM[i].SCL[0], LWM[i].SCL[1], LWM[i].SCL[2]) < 0)
   {
   error = 5;
   break;
   }
  if (fprintf(fLWM, "%1d %1d %1.3f %1.3f %1.3f\n",/* 1.1 */
	LWM[i].Frame, LWM[i].Linear,
	LWM[i].TCB[0], LWM[i].TCB[1], LWM[i].TCB[2]) < 0)
   {
   error = 5;
   break;
   }
  } /* for i=0... */
 fclose(fLWM);

EndWave:

 if (LWM)
  free_Memory(LWM, LWMsize);
 if (KT)
  FreeKeyTable();
 ClearPointer(LW_Win->Win);

 switch (error)
  {
  case 1:
   {
   NoLoad_Message("LightWave Motion: Export", "a Parameter file");
   break;
   } /* no params loaded */
  case 2:
   {
   User_Message("LightWave Motion: Export",
	"No Key Frames to export!\nOperation terminated.", "OK", "o");
   break;
   } /* no key frames */
  case 3:
   {
   User_Message("LightWave Motion: Export",
	"Out of memory!\nOperation terminated.", "OK", "o");
   break;
   } /* file write fail */
  case 4:
   {
   User_Message("LightWave Motion: Export",
	"Error opening file for output!\nOperation terminated.", "OK", "o");
   Log(ERR_OPEN_FAIL, LWInfo->Name);
   break;
   } /* no memory */
  case 5:
   {
   User_Message("LightWave Motion: Export",
	"Error writing to file!\nOperation terminated prematurely.", "OK", "o");
   Log(ERR_WRITE_FAIL, LWInfo->Name);
   break;
   } /* file open fail */
  case 6:
   {
   User_Message("LightWave Motion: Export",
	"Sorry!\nOnly Flat coordinates are presently implemented.\nOperation terminated.", "OK", "o");
   break;
   } /* file write fail */
  case 10:
   {
   break;
   } /* cancel export */
  } /* switch */

} /* ExportWave() */

/***********************************************************************/

STATIC_FCN short Set_LWM(struct LightWaveMotion *LWM, struct LightWaveInfo *LWInfo,
	short Key, short Frame, double Scale, short Linear) // used locally only -> static, AF 23.7.2021
{
 short j, AheadFrame;
 double lonscale, xxx, yyy, zzz, angle, BankFactor;

 lonscale = LATSCALE * cos(LWInfo->RefLat * PiOver180);

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

 if (LWInfo->System == SYSTEM_FLAT)
  {

/* convert positions to XYZ */

  xxx = (LWInfo->RefLon - VP.lon) * lonscale;
  zzz = (VP.lat - LWInfo->RefLat) * LATSCALE;

  LWM[Key].XYZ[0] = xxx * Scale * LWInfo->Scale[0] + LWInfo->Shift[0];	/* X */
  LWM[Key].XYZ[1] = VP.alt * Scale * LWInfo->Scale[1] + LWInfo->Shift[1];/* Y */
  LWM[Key].XYZ[2] = zzz * Scale * LWInfo->Scale[2] + LWInfo->Shift[2];	/* Z */

/* heading */

  if (VP.lat == FP.lat && VP.lon == FP.lon)
   {
   if (Key > 1)
    {
    LWM[Key].HPB[0] = LWM[Key - 1].HPB[0];
    LWM[Key].HPB[1] = LWM[Key - 1].HPB[1];
    LWM[Key].HPB[2] = LWM[Key - 1].HPB[2];
    } /* if not first key */
   else
    {
    return (0);
    } /* else first key - no substitute value available */
   } /* if camera and focus the same */
  else
   {
   xxx = (FP.lon - VP.lon) * lonscale * LWInfo->Scale[0];
   zzz = (FP.lat - VP.lat) * LATSCALE * LWInfo->Scale[2];
   angle = (findangle(xxx, zzz) + HalfPi);
   while (angle < 0.0) angle += TwoPi;
   while (angle >= TwoPi) angle -= TwoPi;
   LWM[Key].HPB[0] = angle * PiUnder180;	/* Heading */

/* pitch */
   yyy = (FP.alt - VP.alt) * LWInfo->Scale[1];
   angle = -atan(yyy / sqrt(xxx * xxx + zzz * zzz));
   LWM[Key].HPB[1] = angle * PiUnder180;	/* Pitch */
   } /* else focus and camera positions different */

/* bank */
  LWM[Key].HPB[2] = PARC_RNDR_MOTION(8);

  } /* if flat earth coordinate system */

 else
  {
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

  LWM[Key].XYZ[0] = VP.x * Scale * LWInfo->Scale[0] + LWInfo->Shift[0];	/* X */
  LWM[Key].XYZ[1] = VP.y * Scale * LWInfo->Scale[0] + LWInfo->Shift[1]; /* Y */
  LWM[Key].XYZ[2] = VP.z * Scale * LWInfo->Scale[0] + LWInfo->Shift[2];	/* Z */

  LWM[Key].HPB[0] = yrot * PiUnder180;				/* Heading */
  LWM[Key].HPB[1] = -xrot * PiUnder180;				/* Pitch */
  LWM[Key].HPB[2] = -zrot * PiUnder180;				/* Bank */

  } /* else spherical earth - polar coordinates converted to cartesian */

 LWM[Key].SCL[0] = 0.0;			/* Scale X */
 LWM[Key].SCL[1] = 0.0;			/* Scale Y */
 LWM[Key].SCL[2] = 0.0;			/* Scale Z */

/* subtract 1 from frame numbers to place first key at frame 0 in LW */
 LWM[Key].Linear = Linear;
 LWM[Key].Frame = Frame;

 return (1);

} /* Set_LWM() */

/**************************************************************************/

void ImportWave(struct LightWaveInfo *LWInfo)
{
 char FileType[8];
 short Version, Channels, TotalKeys = 0, i, j, error = 0, Key, TempEM = 0,
	OldMoItem;
 long LWMsize;
 double Scale, lonscale, xxx, yyy, zzz, angle;
 struct LightWaveMotion *LWM = NULL;
 FILE *fLWM;

 SetPointer(LW_Win->Win, WaitPointer, 16, 16, -6, 0);

 if (! paramsloaded)
  {
  error = 1;
  goto EndWave;
  } /* if no params */

/* determine which coordinate system to use */

 if (LWInfo->System == 1)
  {
  error = 6;
  goto EndWave;
  } /* if spherical system (not yet supported) */

 for (i=0; i<6; i++)
  {
  TotalKeys += CountKeyFrames(0, i);
  } /* for i=0... */
 TotalKeys += CountKeyFrames(0, 8);

 if (TotalKeys > 0)
  {
  if (! User_Message_Def("LightWave Motion: Import",
	"Key Frames exist for Motion Parameters!\nOverwrite them?",
	"OK|Cancel", "oc", 1))
   {
   error = 10;
   goto EndWave;
   } /* if cancel */
  } /* if keys exist */

/* set scale factor for appropriate units of measure */

 switch (LWInfo->Units)
  {
  case 0:
   {
   Scale = ELSCALE_KILOM;
   break;
   } /* Kilometers */
  case 1:
   {
   Scale = ELSCALE_METERS;
   break;
   } /* Meters */
  case 2:
   {
   Scale = ELSCALE_CENTIM;
   break;
   } /* Centimeters */
  case 3:
   {
   Scale = ELSCALE_MILES;
   break;
   } /* Miles */
  case 4:
   {
   Scale = ELSCALE_FEET;
   break;
   } /* Feet */
  case 5:
   {
   Scale = ELSCALE_INCHES;
   break;
   } /* Inches */
  } /* switch */

/* determine if key frames exist for the motion path */

 if ((fLWM = fopen(LWInfo->Name, "r")) == NULL)
  {
  error = 4;
  goto EndWave;
  } /* if open fail */

 fgets(FileType, 8, fLWM);
 FileType[4] = '\0';

 if (strcmp(FileType, "LWMO"))
  {
  error = 7;
  fclose(fLWM);
  goto EndWave;
  }

 fscanf(fLWM, "%hd%hd%hd", &Version, &Channels, &TotalKeys);

 if (Channels != 9)
  {
  error = 8;
  goto EndWave;
  } /* no motion key frames */

 if (Version != 1)
  {
  error = 8;
  goto EndWave;
  } /* no motion key frames */

 if (TotalKeys < 1)
  {
  error = 2;
  goto EndWave;
  } /* no motion key frames */

/* allocate some essential resources */

 LWMsize = TotalKeys * sizeof (struct LightWaveMotion);
 if ((LWM = (struct LightWaveMotion *)get_Memory(LWMsize, MEMF_CLEAR)) == NULL)
  {
  error = 3;
  fclose(fLWM);
  goto EndWave;
  } /* if out of memory */

 if (! EM_Win)
  {
  if ((EM_Win = (struct MotionWindow *)
	get_Memory(sizeof (struct MotionWindow), MEMF_CLEAR)) == NULL)
   {
   error = 3;
   fclose(fLWM);
   goto EndWave;
   } /* if out of memory */
  TempEM = 1;
  }
 OldMoItem = EM_Win->MoItem;

 for (i=0; i<TotalKeys; i++)
  {
  if (fscanf(fLWM, "%le%le%le%le%le%le%le%le%le",
	&LWM[i].XYZ[0], &LWM[i].XYZ[1], &LWM[i].XYZ[2],
	&LWM[i].HPB[0], &LWM[i].HPB[1], &LWM[i].HPB[2],
	&LWM[i].SCL[0], &LWM[i].SCL[1], &LWM[i].SCL[2]) != 9)
   {
   error = 5;
   break;
   }
  if (fscanf(fLWM, "%d%d%le%le%le",
	&LWM[i].Frame, &LWM[i].Linear,
	&LWM[i].TCB[0], &LWM[i].TCB[1], &LWM[i].TCB[2]) != 5)
   {
   error = 5;
   break;
   }
  } /* for i=0... */
 fclose(fLWM);

 if (error)
  goto EndWave;

/* delete current key frames */

 for (i=ParHdr.KeyFrames-1; i>=0; i--)
  {
  if (KF[i].MoKey.Group == 0)
   {
   if (KF[i].MoKey.Item == 8 || KF[i].MoKey.Item < 6)
    DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, KF[i].MoKey.Item, 0, 0);
   } /* if group match */
  } /* for i=0... */

/* Convert LWM to WCS parameters */

/* XYZ */

 lonscale = LATSCALE * cos(LWInfo->RefLat * PiOver180);

 for (i=0; i<TotalKeys; i++)
  {
  Key = LWM[i].Frame;

  if (LWInfo->System == SYSTEM_FLAT)
   {

/* X - Lon */
   PAR_FIRST_MOTION(2) = LWInfo->RefLon - ((LWM[i].XYZ[0] + LWInfo->Shift[0])
	 * Scale * LWInfo->Scale[0] / lonscale);

/* Y - Alt */
   PAR_FIRST_MOTION(0) = (LWM[i].XYZ[1] + LWInfo->Shift[1])
	 * Scale * LWInfo->Scale[1];

/* Z - Lat */
   PAR_FIRST_MOTION(1) = LWInfo->RefLat + ((LWM[i].XYZ[2] + LWInfo->Shift[2])
	 * Scale * LWInfo->Scale[2] / LATSCALE);

/* HPB */

/* Bank */
   PAR_FIRST_MOTION(8) = LWM[i].HPB[2];

/* Heading */
   while (LWM[i].HPB[0] > 360.0)
    LWM[i].HPB[0] -= 360.0;
   while (LWM[i].HPB[0] < 0.0)
    LWM[i].HPB[0] += 360.0;
   if (LWM[i].HPB[0] == 90.0)
    {
    xxx = -1.0;
    zzz = 0.0;
    }
   else if (LWM[i].HPB[0] == 270.0)
    {
    xxx = 1.0;
    zzz = 0.0;
    }
   else
    {
    angle = LWM[i].HPB[0] / PiUnder180;
    if (LWM[i].HPB[0] > 180.0)
     {
     xxx = 1.0;
     zzz = -1.0 / tan(angle);
     }
    else
     {
     xxx = -1.0;
     zzz = 1.0 / tan(angle);
     }
    if (fabs(zzz) > 1.0)
     {
     if (LWM[i].HPB[0] < 90.0 || LWM[i].HPB[0] > 270.0)
      {
      zzz = 1.0;
      xxx = -tan(angle);
      }
     else
      {
      zzz = -1.0;
      xxx = tan(angle);
      }
     }
    }

   PAR_FIRST_MOTION(5) = PAR_FIRST_MOTION(2)
	+ (xxx / lonscale) * LWInfo->Scale[0];
   PAR_FIRST_MOTION(4) = PAR_FIRST_MOTION(1)
	+ (zzz / LATSCALE) * LWInfo->Scale[2];

/* Pitch */
   while (LWM[i].HPB[1] > 360.0)
    LWM[i].HPB[1] -= 360.0;
   while (LWM[i].HPB[1] < 0.0)
    LWM[i].HPB[1] += 360.0;

   if (fabs(LWM[i].HPB[1]) == 90.0)
    {
    PAR_FIRST_MOTION(3) = 0.0;
    PAR_FIRST_MOTION(4) = 0.0;
    PAR_FIRST_MOTION(5) = 0.0;
    }
   else
    {
    yyy = -tan(LWM[i].HPB[1] / PiUnder180) * sqrt(xxx * xxx + zzz * zzz);
    PAR_FIRST_MOTION(3) = PAR_FIRST_MOTION(0) + yyy * LWInfo->Scale[1];
    }

   }/* if flat coordinate system */

  else
   {

   } /* else spherical coords */

/* TCB */
  EM_Win->TCB[0] = LWM[i].TCB[0];
  EM_Win->TCB[1] = LWM[i].TCB[1];
  EM_Win->TCB[2] = LWM[i].TCB[2];

/* Linear */
  EM_Win->Linear = LWM[i].Linear;

  for (j=0; j<6; j++)
   {
   EM_Win->MoItem = j;			/* so SetKeyFrame() will set TCB & Linear */
   if (! MakeKeyFrame(Key, 0, j))
    {
    error = 9;
    break;
    } /* if error */
   } /* for j=0... */
  if (error)
   break;
  EM_Win->MoItem = 8;
  if (! MakeKeyFrame(Key, 0, 8))
   {
   error = 9;
   break;
   }
  } /* for i=0... */

EndWave:

 EM_Win->MoItem = OldMoItem;
 if (LWM)
  free_Memory(LWM, LWMsize);
 if (TempEM && EM_Win)
  {
  free_Memory(EM_Win, sizeof (struct MotionWindow));
  EM_Win = NULL;
  } /* if temporary EM_Win structure created */
 ClearPointer(LW_Win->Win);

 switch (error)
  {
  case 1:
   {
   NoLoad_Message("LightWave Motion: Import", "a parameter file");
   break;
   } /* no params loaded */
  case 2:
   {
   User_Message("LightWave Motion: Import",
	"No Key Frames to import!\nOperation terminated.", "OK", "o");
   break;
   } /* no key frames */
  case 3:
   {
   User_Message("LightWave Motion: Import",
	"Out of memory!\nOperation terminated.", "OK", "o");
   break;
   } /* file write fail */
  case 4:
   {
   User_Message("LightWave Motion I/0",
	"Error opening file for input!\nOperation terminated.", "OK", "o");
   Log(ERR_OPEN_FAIL, LWInfo->Name);
   break;
   } /* no memory */
  case 5:
   {
   User_Message("LightWave Motion: Import",
	"Error reading from file!\nOperation terminated prematurely.", "OK", "o");
   Log(ERR_WRITE_FAIL, LWInfo->Name);
   break;
   } /* file open fail */
  case 6:
   {
   User_Message("LightWave Motion: Import",
	"Sorry!\nOnly Flat coordinates are presently implemented.\nOperation terminated.", "OK", "o");
   break;
   } /* unimplemented system */
  case 7:
   {
   User_Message("LightWave Motion: Import",
	"Selected file is not a LightWave Motion file.\nOperation terminated.", "OK", "o");
   Log(ERR_WRONG_TYPE, LWInfo->Name);
   break;
   } /* file wrong type */
  case 8:
   {
   sprintf(str,	"Unsupported LightWave Motion file format (version %hd).\nOperation terminated.", Version);
   User_Message("LightWave Motion: Import", str, "OK", "o");
   Log(ERR_WRONG_VER, LWInfo->Name);
   break;
   } /* file wrong version */
  case 9:
   {
   User_Message("LightWave Motion: Import",
	"Error creating Key Frame!\nOperation terminated.", "OK", "o");
   break;
   } /* key frame create fail */
  case 10:
   {
   break;
   } /* cancel export */
  } /* switch */

} /* ImportWave() */
#endif
/**************************************************************************/

short  CreateBankKeys(void)
{
 short error = 0, i, LastFrame, Key, TotalKeys = 0, FrameInt;

 for (i=0; i<6; i++)
  {
  if ((Key = CountKeyFrames(0, i)) > TotalKeys)
   TotalKeys = Key;
  } /* for i=0... */

 if (TotalKeys < 1)
  {
  error = 2;
  goto EndBank;
  } /* no motion key frames */

 if (CountKeyFrames(0, 8) > 0)
  if (! User_Message((CONST_STRPTR)"Parameters Module: Bank Keys",
          (CONST_STRPTR)"Key Frames exist for the \"Bank\" Parameter. Overwrite them?",
          (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
   {
   error = 10;
   goto EndBank;
   } /* if cancel */

 strcpy(str, "K");
 if (! GetInputString("Enter Key Frame interval or 'K' for current Key Frames.",
	 ".", str))
  {
  error = 10;
  goto EndBank;
  } /* if cancel */
 if (str[0] == 'k' || str[0] == 'K')
  FrameInt = -1;
 else
  FrameInt = atoi(str);

 SetPointer(EM_Win->Win, WaitPointer, 16, 16, -6, 0);

/* delete current key frames */
 for (i=ParHdr.KeyFrames-1; i>=0; i--)
  {
  if (KF[i].MoKey.Group == 0)
   {
   if (KF[i].MoKey.Item == 8)
    DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, 8, 0, 0);
   } /* if group match */
  } /* for i=0... */

/* allocate some essential resources */

 if (! BuildKeyTable())
  {
  error = 3;
  goto EndBank;
  } /* if no key table = out of memory */

 if (FrameInt < 0)
  {
  LastFrame = -1;
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == 0 &&
	(KF[i].MoKey.Item == 1 || KF[i].MoKey.Item == 2))
    {
    if (KF[i].MoKey.KeyFrame != LastFrame)
     {
     if (! Set_Bank_Key(KF[i].MoKey.KeyFrame))
      {
      error = 1;
      break;
      } /* if error creating key */
     LastFrame = KF[i].MoKey.KeyFrame;
     } /* frame number changed, compute position info for last key frame */ 

    } /* if a vital key frame */
   }/* for i=0... */
  } /* if make bank keys only at current key frames */
 else
  {
  for (i=0; i<KT_MaxFrames; i+=FrameInt)
   {
   if (! Set_Bank_Key(i))
    {
    error = 1;
    break;
    } /* if error creating key */
   } /* for i=0... */
  if (! error)
   if (! Set_Bank_Key(KT_MaxFrames))
    error = 1;
  } /* else use fixed interval */

EndBank:

 if (KT)
  FreeKeyTable();

 ClearPointer(EM_Win->Win);

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Export",
           (CONST_STRPTR)"Error creating Key Frame!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* no params loaded */
  case 2:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Export",
           (CONST_STRPTR)"No Camera Path Lat/Lon Key Frames!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* no key frames */
  case 3:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Export",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* file write fail */
  } /* switch */

 if (error < 2)
  return (1);
 return (0);

} /* CreateBankKeys() */

/***********************************************************************/

STATIC_FCN short Set_Bank_Key(short Frame) // used locally only -> static, AF 23.7.2021
{
 short j;

 PAR_FIRST_MOTION(8) = 0.0;
 for (j=-5; j<=5; j++)
  {
  PAR_FIRST_MOTION(8) += ComputeBanking(Frame + j);
  }
 PAR_FIRST_MOTION(8) /= 11.0;

 return (MakeKeyFrame(Frame, 0, 8));

} /* Set_Bank_Key() */

/************************************************************************/

short ScaleKeys(struct ScaleKeyInfo *SKI)
{
 short i, Changes = 1;

 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  if (SKI->ModGroup[KF[i].MoKey.Group]
	|| (SKI->Group == KF[i].MoKey.Group && SKI->Item == KF[i].MoKey.Item))
   {
   if (SKI->AllFrames || (SKI->Frame == KF[i].MoKey.KeyFrame))
    {
    if (SKI->FrameScale)
     {
     KF[i].MoKey.KeyFrame *= SKI->FSc;
     } /* if scale frames */
    if (SKI->ValueScale)
     {
     switch (KF[i].MoKey.Group)
      {
      case 0:
       {
       KF[i].MoKey.Value *= SKI->VSc;
       break;
       } /* motion */
      case 1:
       {
       KF[i].CoKey.Value[0] *= SKI->VSc;
       if (KF[i].CoKey.Value[0] > 255)
        KF[i].CoKey.Value[0] = 255;
       else if (KF[i].CoKey.Value[0] < 0)
        KF[i].CoKey.Value[0] = 0;
       KF[i].CoKey.Value[1] *= SKI->VSc;
       if (KF[i].CoKey.Value[1] > 255)
        KF[i].CoKey.Value[1] = 255;
       else if (KF[i].CoKey.Value[1] < 0)
        KF[i].CoKey.Value[1] = 0;
       KF[i].CoKey.Value[2] *= SKI->VSc;
       if (KF[i].CoKey.Value[2] > 255)
        KF[i].CoKey.Value[2] = 255;
       else if (KF[i].CoKey.Value[2] < 0)
        KF[i].CoKey.Value[2] = 0;
       break;
       } /* color */
      case 2:
       {
       KF[i].EcoKey.Line *= SKI->VSc;
       break;
       } /* ecosystem */
      } /* switch */
     } /* if scale values */
    if (SKI->FrameShift)
     {
     KF[i].MoKey.KeyFrame += SKI->FSh;
     } /* if shift frames */
    if (SKI->ValueShift)
     {
     switch (KF[i].MoKey.Group)
      {
      case 0:
       {
       KF[i].MoKey.Value += SKI->VSh;
       break;
       } /* motion */
      case 1:
       {
       KF[i].CoKey.Value[0] += SKI->VSh;
       if (KF[i].CoKey.Value[0] > 255)
        KF[i].CoKey.Value[0] = 255;
       else if (KF[i].CoKey.Value[0] < 0)
        KF[i].CoKey.Value[0] = 0;
       KF[i].CoKey.Value[1] += SKI->VSh;
       if (KF[i].CoKey.Value[1] > 255)
        KF[i].CoKey.Value[1] = 255;
       else if (KF[i].CoKey.Value[1] < 0)
        KF[i].CoKey.Value[1] = 0;
       KF[i].CoKey.Value[2] += SKI->VSh;
       if (KF[i].CoKey.Value[2] > 255)
        KF[i].CoKey.Value[2] = 255;
       else if (KF[i].CoKey.Value[2] < 0)
        KF[i].CoKey.Value[2] = 0;
       break;
       } /* color */
      case 2:
       {
       KF[i].EcoKey.Line += SKI->VSh;
       break;
       } /* ecosystem */
      } /* switch */
     } /* if shift values */
    } /* if all keys or key frame match */
   } /* if entire group or group/item match */
  } /* for i=0... */

 while (Changes)
  {
  Changes = 0;
  for (i=1; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.KeyFrame < KF[i - 1].MoKey.KeyFrame)
    {
    swmem(&KF[i], &KF[i - 1], sizeof (union KeyFrame));
    Changes = 1;
    } /* if need to swap two key frames */
   } /* for i=1... */
  } /* while */


 return (1);

} /* ScaleKeys() */

/********************************************************************/
 
void defaultsettings(void)
{
 short i;

 settings.startframe = 0;
 settings.maxframes = 1;
 settings.startseg = 0;		/* formerly campath */
 settings.smoothfaces = 0;	/* formerly focpath */
 settings.bankturn = 0;
 settings.colrmap = 0;
 settings.borderandom = 0;
 settings.cmaptrees = 0;
 settings.rendertrees = 1;
 settings.statistics = 0;
 settings.stepframes = 1;
 settings.zbufalias = 0;
 settings.horfix = 0;
 settings.horizonmax = 0;
 settings.clouds = 0;
 settings.linefade = 1;
 settings.drawgrid = 0;
 settings.gridsize = 10;
 settings.alternateq = 0;
 settings.linetoscreen = 1;
 settings.mapassfc = 0;
 settings.surfel[0] = 0;
 settings.surfel[1] = 1250;
 settings.surfel[2] = 2500;
 settings.surfel[3] = 5000;
 settings.worldmap = 0;
 settings.flatteneco = 1;
 settings.fixfract = 1;
 settings.vecsegs = 1;
 settings.reliefshade = 0;
 settings.renderopts = 0x11;
 settings.scrnwidth = 752;
 settings.scrnheight = 480;
 settings.rendersegs = 1;
 settings.overscan = 40;
 settings.lookahead = 0;
 settings.composite = 0;
 settings.defaulteco = 1;
 settings.ecomatch = 0;
 settings.Yoffset = 0;
 settings.saveIFF = 2;
 settings.background = 0;
 settings.zbuffer = 0;
 settings.antialias = 0;
 settings.scaleimage = 0;
 settings.fractal = 1;
 settings.aliasfactor = 1;
 settings.scalewidth = 752;
 settings.scaleheight = 480;
 settings.exportzbuf = 0;
 settings.zformat = 0;
 settings.fieldrender = 0;
 settings.lookaheadframes = 10;
 settings.velocitydistr = 0;
 settings.easein = 0;
 settings.easeout = 0;
 settings.displace = 1;
 settings.fielddominance = 0;
 settings.fractalmap = 0;
 settings.perturb = 0;
 settings.realclouds = 0;
 settings.reflections = 0;
 settings.waves = 0;
 settings.colorstrata = 0;
 settings.cmapsurface = 0;
 settings.deformationmap = 0;
 settings.moon = 0;
 settings.sun = 0;
 settings.tides = 0;
 settings.sunhalo = 1;
 settings.moonhalo = 1;
 
 for (i=0; i<EXTRASHORTSETTINGS; i++) settings.extrashorts[i] = 0;
 for (i=0; i<EXTRADOUBLESETTINGS; i++) settings.extradoubles[i] = 0.0;

 settings.zalias = .1;
 settings.bankfactor = 1.0;
 settings.skyalias = 20.0;
 settings.lineoffset = .08;
 settings.altqlat = PAR_FIRST_MOTION(1);
 settings.altqlon = PAR_FIRST_MOTION(2);
 settings.treefactor = 15.0;
 settings.displacement = 1.0;
 settings.picaspect = 1.2;
 settings.zenith = 100.0;
 settings.dispslopefact = 1.4;
 settings.globecograd = 100.0;
 settings.globsnowgrad = 100.0;
 settings.globreflat = 0.0;
 settings.stratadip = 0.0;
 settings.stratastrike = 0.0;
 settings.deformscale = 1.0;

} /* defaultsettings() */

/**********************************************************************/

void initmopar(void)
{
 short i;

 fixfocus = 0;
/* if any focus parameter is not 0.0, set fixfocus. fixfocus means that the
   focal point is determined by focus parameters, not center earth. */
 if (PAR_FIRST_MOTION(4) != 0.0) fixfocus = 1;
 if (PAR_FIRST_MOTION(5) != 0.0) fixfocus = 1;
 if (PAR_FIRST_MOTION(3) != 0.0) fixfocus = 1;
 if (settings.lookahead) fixfocus = 1;

/* I'm not so sure we want to do this: */
/*
 if (fixfocus == 1)
  {
  if (PAR_FIRST_MOTION(4) == 0.0) PAR_FIRST_MOTION(4) = PAR_FIRST_MOTION(1);
  if (PAR_FIRST_MOTION(5) == 0.0) PAR_FIRST_MOTION(5) = PAR_FIRST_MOTION(2);
  } // if
*/
 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  PARC_RNDR_MOTION(i) = PAR_FIRST_MOTION(i);
  } /* for i=0... */
 PARC_RNDR_MOTION(2) += PARC_RNDR_MOTION(9);
 if (fixfocus)
  PARC_RNDR_MOTION(5) += PARC_RNDR_MOTION(9);
 PARC_RNDR_MOTION(16) += PARC_RNDR_MOTION(9);

} /* initmopar() */

/************************************************************************/

void initpar(void)
{
 short i, j;

 for (i=0; i<COLORPARAMS; i++)
   {
   for (j=0; j<3; j++)
    {
    PARC_RNDR_COLOR(i, j) = PAR_FIRST_COLOR(i, j);
    } /* for j=0... */
   } /* for i=0... */

 for (i=0; i<ECOPARAMS; i++)
  {
  PARC_RNDRLN_ECO(i) = PAR_FIRSTLN_ECO(i);
  PARC_RNDRSK_ECO(i) = PAR_FIRSTSK_ECO(i);
  PARC_RNDRSA_ECO(i) = PAR_FIRSTSA_ECO(i);
  PARC_RNDRRE_ECO(i) = PAR_FIRSTRE_ECO(i);
  PARC_RNDRXR_ECO(i) = PAR_FIRSTXR_ECO(i);
  PARC_RNDRNR_ECO(i) = PAR_FIRSTNR_ECO(i);
  PARC_RNDRXS_ECO(i) = PAR_FIRSTXS_ECO(i);
  PARC_RNDRNS_ECO(i) = PAR_FIRSTNS_ECO(i);
  PARC_RNDRDN_ECO(i) = PAR_FIRSTDN_ECO(i);
  PARC_RNDRHT_ECO(i) = PAR_FIRSTHT_ECO(i);
  } /* for i=0... */

/* adjust sea depth for flattening */
 if (settings.flatteneco)
  PARC_RNDRSK_ECO(0) += ( (0.0 - PARC_RNDRSK_ECO(0)) * PARC_RNDR_MOTION(12) );

 if (PARC_RNDRSK_ECO(0) <= 0.0001) PARC_RNDRSK_ECO(0) = 0.0001;


} /* initpar() */

/************************************************************************/

#ifdef FHDJALDJFHADLKDFh    // Obsolete - for reference only, please do not delete !!!
void moparamcheck(short oframe)
{
 short i;

 while (frame < oframe + settings.stepframes) {
  for (i=0; i<USEDMOTIONPARAMS; i++) {
   if (frame >= MoPar.mn[i].Frame[0] && frame < MoPar.mn[i].Frame[1]) {
    if (frame < MoPar.mn[i].Frame[0] + MoPar.mn[i].Ease[0])
       MoShift[i].Value[0] = (frame+1 - (float)MoPar.mn[i].Frame[0]) /
       ((float)MoPar.mn[i].Ease[0] + 1);
    else if (frame >= MoPar.mn[i].Frame[1] - MoPar.mn[i].Ease[1])
       MoShift[i].Value[0] = ((float)MoPar.mn[i].Frame[1] - frame) /
       ((float)MoPar.mn[i].Ease[1] + 1);
    else MoShift[i].Value[0] = 1.0;
   } /* if */
   else MoShift[i].Value[0] = 0.0;
   MoShift[i].Value[2] += MoShift[i].Value[0] * MoShift[i].Value[1];
  } /* for i=0...*/

  MoShift[2].Value[2] +=MoShift[9].Value[2];
  MoShift[16].Value[2] +=MoShift[9].Value[2];

  frame ++;
 } /* while */

 CenterX = MoShift[6].Value[2];
 CenterY = MoShift[7].Value[2];

} /* moparamcheck() */

void paramcheck(short frame, short oframe)
{
 short i, j;

 while (frame < oframe + settings.stepframes) {
  for (i=0; i<COLORPARAMS; i++) {
   for (j=0; j<3; j++) {
    if (frame >= CoPar.cn[i].Frame[0] && frame < CoPar.cn[i].Frame[1]) {
     if (frame < CoPar.cn[i].Frame[0] + CoPar.cn[i].Ease[0])
        CoShift[i].Value[0][j] = (frame+1 - (float)CoPar.cn[i].Frame[0]) /
        ((float)CoPar.cn[i].Ease[0] + 1);
     else if (frame >= CoPar.cn[i].Frame[1] - CoPar.cn[i].Ease[1])
        CoShift[i].Value[0][j] = ((float)CoPar.cn[i].Frame[1] - frame) /
        ((float)CoPar.cn[i].Ease[1] + 1);
     else CoShift[i].Value[0][j] = 1.0;
    } /* if */
    else CoShift[i].Value[0][j] = 0.0;
    CoShift[i].Value[2][j] += CoShift[i].Value[0][j] * CoShift[i].Value[1][j];
   } /* for j=0... */
  } /* for i=0... */
  frame ++;
 } /* while */
} /* paramcheck() */

/*End of obsolete code for "simple" animation*/
#endif

/************************************************************************/

void setvalues(void)
{
 long i, j, k, tempframe;
 struct Wave *WV;

 if (KT)
  {
  tempframe = frame < KT_MaxFrames ? frame: KT_MaxFrames;
  PARC_RNDR_MOTION(8) = PAR_FIRST_MOTION(8);
  for (i=0; i<USEDMOTIONPARAMS; i++)
   {
   if (KT[i].Key)
    {
    PARC_RNDR_MOTION(i) = KT[i].Val[0][tempframe];
    } /* if key exists */
   } /* for i=0... */
  for (i=USEDMOTIONPARAMS, j=0; i<USEDMOTIONPARAMS + COLORPARAMS; i++, j++)
   {
   if (KT[i].Key)
    {
    for (k=0; k<3; k++)
     {
     PARC_RNDR_COLOR(j, k) = KT[i].Val[k][tempframe];
     } /* for k=0... */
    } /* if key exists */
   } /* for i=0... */
  for (i=USEDMOTIONPARAMS + COLORPARAMS, j=0;
	 i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++, j++)
   {
   if (KT[i].Key)
    {
    PARC_RNDRLN_ECO(j) = KT[i].Val[0][tempframe];
    PARC_RNDRSK_ECO(j) = KT[i].Val[1][tempframe];
    PARC_RNDRSA_ECO(j) = KT[i].Val[2][tempframe];
    PARC_RNDRRE_ECO(j) = KT[i].Val[3][tempframe];
    PARC_RNDRXR_ECO(j) = KT[i].Val[4][tempframe];
    PARC_RNDRNR_ECO(j) = KT[i].Val[5][tempframe];
    PARC_RNDRXS_ECO(j) = KT[i].Val[6][tempframe];
    PARC_RNDRNS_ECO(j) = KT[i].Val[7][tempframe];
    PARC_RNDRDN_ECO(j) = KT[i].Val[8][tempframe];
    PARC_RNDRHT_ECO(j) = KT[i].Val[9][tempframe];
    } /* if key exists */
   } /* for i=0... */

/* adjust sea depth for flattening */

  if (settings.flatteneco && KT[USEDMOTIONPARAMS + COLORPARAMS].Key)
   {
   PARC_RNDRSK_ECO(0) += ( (PARC_RNDR_MOTION(13) - PARC_RNDRSK_ECO(0))
	 * PARC_RNDR_MOTION(12) );
   if (PARC_RNDRSK_ECO(0) <= 0.0001) PARC_RNDRSK_ECO(0) = 0.0001;
   } /* if */

/* account for earth rotation */

  if (KT[2].Key)
   PARC_RNDR_MOTION(2) += PARC_RNDR_MOTION(9);
  else
   PARC_RNDR_MOTION(2) = PAR_FIRST_MOTION(2) + PARC_RNDR_MOTION(9);
  if (fixfocus)
   {
   if (KT[5].Key)
    PARC_RNDR_MOTION(5) += PARC_RNDR_MOTION(9);
   else
    PARC_RNDR_MOTION(5) = PAR_FIRST_MOTION(5) + PARC_RNDR_MOTION(9);
   } /* if */
  if (KT[16].Key)
   PARC_RNDR_MOTION(16) += PARC_RNDR_MOTION(9);
  else
   PARC_RNDR_MOTION(16) = PAR_FIRST_MOTION(16) + PARC_RNDR_MOTION(9);
  } /* if */
 else
  {
  PARC_RNDR_MOTION(2) = PAR_FIRST_MOTION(2) + PARC_RNDR_MOTION(9);
  if (fixfocus)
   PARC_RNDR_MOTION(5) = PAR_FIRST_MOTION(5) + PARC_RNDR_MOTION(9);
  PARC_RNDR_MOTION(16) = PAR_FIRST_MOTION(16) + PARC_RNDR_MOTION(9);
  } /* else */

/* set some useful values */
 sunlat = 	PARC_RNDR_MOTION(15) * PiOver180;
 sunlong = 	PARC_RNDR_MOTION(16) * PiOver180;
 qmin = 	PARC_RNDR_MOTION(25);
 fogrange = 	PARC_RNDR_MOTION(24) - PARC_RNDR_MOTION(23);
 horline = 	PARC_RNDR_MOTION(17)
		* settings.scrnheight * settings.rendersegs / 100;
 if (PARC_RNDR_MOTION(21) == 0.0)
		PARC_RNDR_MOTION(21) = .001;
 horpt = 	PARC_RNDR_MOTION(18) * (float)settings.scrnwidth / 100;
 horstretch = 	fabs(1.0 / PARC_RNDR_MOTION(19));
 SeaLevel = settings.flatteneco ? PARC_RNDRLN_ECO(0) + (PARC_RNDR_MOTION(13)
		- PARC_RNDRLN_ECO(0)) * PARC_RNDR_MOTION(12):
		 PARC_RNDRLN_ECO(0);

 MaxSeaLevel = SeaLevel;

 if (settings.tides && KT)
  {
  if (KT[USEDMOTIONPARAMS + COLORPARAMS].Key)
   {
   double TempSeaLevel;

   for (i=0; i<=KT_MaxFrames; i++)
    {
    if (settings.flatteneco)
     {
     if (KT[12].Key)
      {
      if (KT[13].Key)
       TempSeaLevel = KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i]
		 + (KT[13].Val[0][i]
		 - KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i])
		 * KT[12].Val[0][i];
      else
       TempSeaLevel = KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i]
		 + (PARC_RNDR_MOTION(13)
		 - KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i])
		 * KT[12].Val[0][i];
      } /* if flattening key frames */
     else
      TempSeaLevel = KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i]
		 + (PARC_RNDR_MOTION(13)
		 - KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i])
		 * PARC_RNDR_MOTION(12);
     } /* if flatten ecosystems */
    else
     TempSeaLevel = KT[USEDMOTIONPARAMS + COLORPARAMS].Val[0][i];
    if (TempSeaLevel > MaxSeaLevel)
     MaxSeaLevel = TempSeaLevel;
    } /* for i=0... */
   } /* if sea level key frames */
  } /* if tides enabled and key table generated */

 if (Tsunami)
  {
  double MaxAmp;

  if (Tsunami->KT)
   {
   if (Tsunami->KT->Key)
    {
    MaxAmp = 0.0;
    for (i=0; i<Tsunami->KT_MaxFrames; i++)
     {
     if (Tsunami->KT->Val[0][i] > MaxAmp)
      MaxAmp = Tsunami->KT->Val[0][i];
     } /* for i=0... */
    } /* if key frames */
   else
    MaxAmp = Tsunami->Amp;
   } /* else */
  else
   MaxAmp = Tsunami->Amp;
  WV = Tsunami->Wave;
  while (WV)
   {
   MaxSeaLevel += fabs(WV->Amp * MaxAmp);
   WV = WV->Next;
   } /* while */
  } /* if */
 MaxWaveAmp = MaxSeaLevel - SeaLevel;
 if (MaxWaveAmp < .001)
  MaxWaveAmp = .001;

 ReflectionStrength = PARC_RNDR_MOTION(32) / 100.0;
 if (ReflectionStrength > 1.0)
  ReflectionStrength = 1.0;
 if (ReflectionStrength < 0.0)
  ReflectionStrength = 0.0;

/* set this in wave editor
 WhiteCapHt = 8.65;	// 8.65 for Maui, .65 for RMNP & GrandCanyon
*/
 for (i=0; i<ECOPARAMS; i++)
  {
  for (j=0; j<3; j++)
   {
   PARC_MCOL_ECO(i, j) = (short)PARC_RNDR_COLOR(PAR_COLR_ECO(i), j);
   PARC_SCOL_ECO(i, j) =
	 (short)PARC_RNDR_COLOR(PAR_COLR_ECO(PAR_UNDER_ECO(i)), j);
   } /* for j=0 */

  PARC_SKLT_ECO(i) = PARC_RNDRSK_ECO(i) *
	sin(PARC_RNDRSA_ECO(i) * PiOver180 - HalfPi);
  PARC_SKLN_ECO(i) = PARC_RNDRSK_ECO(i) *
	cos(PARC_RNDRSA_ECO(i) * PiOver180 - HalfPi);
  PARC_MXSL_ECO(i) = PARC_RNDRXS_ECO(i) * PiOver180;
  PARC_MNSL_ECO(i) = PARC_RNDRNS_ECO(i) * PiOver180;

  } /* for i=0 */

/* Make Water SkewLat consistent with aspect values for wave generation */
 PARC_SKLT_ECO(0) = PARC_RNDRSA_ECO(0) * PiOver180 + HalfPi;

 redsun = (double)PARC_RNDR_COLOR(0, 0) / 128.0;
 greensun = (double)PARC_RNDR_COLOR(0, 1) / 128.0;
 bluesun = (double)PARC_RNDR_COLOR(0, 2) / 128.0;

} /* setvalues() */

/************************************************************************/

void boundscheck(short parameter)
{

  if (PAR_FIRST_MOTION(parameter) > parambounds[parameter][0])
   PAR_FIRST_MOTION(parameter) = parambounds[parameter][0];
  else if (PAR_FIRST_MOTION(parameter) < parambounds[parameter][1])
   PAR_FIRST_MOTION(parameter) = parambounds[parameter][1];

} /* boundscheck() */

/************************************************************************/

void setecodefault(short i)
{

 memset(&EcoPar.en[i], 0, sizeof (struct Ecosystem));
 strcpy(EcoPar.en[i].Name, "Unused");
 PAR_FIRSTLN_ECO(i) = 32000.0;
 PAR_FIRSTXR_ECO(i) = 10000.0;
 PAR_FIRSTNR_ECO(i) = -10000.0;
 PAR_FIRSTXS_ECO(i) = 91.0;

 PAR_FIRSTSK_ECO(i) = 0.0;
 PAR_FIRSTSA_ECO(i) = 0.0;
 PAR_FIRSTRE_ECO(i) = 0.0;
 PAR_FIRSTNS_ECO(i) = 0.0;
 PAR_FIRSTDN_ECO(i) = 0.0;
 PAR_FIRSTHT_ECO(i) = 0.0;

 EcoShift[i].Ecotype = NULL;

} /* setecodefault() */

/************************************************************************/

void Sort_Eco_Params(void)
{
short i, j, Changes = 1, Order[ECOPARAMS];

 for (i=0; i<ECOPARAMS; i++)
  {
  Order[i] = i;
  } /* for i=0... */

 while (Changes)
  {
  Changes = 0;
  for (i=2; i<ECOPARAMS - 1; i++)
   {
   if (PAR_FIRSTXR_ECO(i) > PAR_FIRSTXR_ECO(i + 1))
    {
    swmem(&EcoPar.en[i], &EcoPar.en[i + 1], sizeof (struct Ecosystem));
    swmem(&EcoShift[i].Ecotype, &EcoShift[i + 1].Ecotype,
	sizeof (struct Ecotype *));
    swmem(&Order[i], &Order[i + 1], sizeof (short));
    Changes = 1;
    } /* if */
   else if (PAR_FIRSTXR_ECO(i) == PAR_FIRSTXR_ECO(i + 1))
    {
    if (PAR_FIRSTNR_ECO(i) < PAR_FIRSTNR_ECO(i + 1))
     {
     swmem(&EcoPar.en[i], &EcoPar.en[i + 1], sizeof (struct Ecosystem));
     swmem(&EcoShift[i].Ecotype, &EcoShift[i + 1].Ecotype,
	sizeof (struct Ecotype *));
     swmem(&Order[i], &Order[i + 1], sizeof (short));
     Changes = 1;
     } /* if */
    else if (PAR_FIRSTNR_ECO(i) == PAR_FIRSTNR_ECO(i + 1))
     {
     if (PAR_FIRSTLN_ECO(i) > PAR_FIRSTLN_ECO(i + 1))
      {
      swmem(&EcoPar.en[i], &EcoPar.en[i + 1], sizeof (struct Ecosystem));
      swmem(&EcoShift[i].Ecotype, &EcoShift[i + 1].Ecotype,
	sizeof (struct Ecotype *));
      swmem(&Order[i], &Order[i + 1], sizeof (short));
      Changes = 1;
      } /* if */
     else if (PAR_FIRSTLN_ECO(i) == PAR_FIRSTLN_ECO(i + 1))
      {
      if (PAR_FIRSTXS_ECO(i) > PAR_FIRSTXS_ECO(i + 1))
       {
       swmem(&EcoPar.en[i], &EcoPar.en[i + 1], sizeof (struct Ecosystem));
       swmem(&EcoShift[i].Ecotype, &EcoShift[i + 1].Ecotype,
	sizeof (struct Ecotype *));
       swmem(&Order[i], &Order[i + 1], sizeof (short));
       Changes = 1;
       } /* if */
      else if (PAR_FIRSTXS_ECO(i) == PAR_FIRSTXS_ECO(i + 1))
       {
       if (PAR_FIRSTNS_ECO(i) < PAR_FIRSTNS_ECO(i + 1))
        {
        swmem(&EcoPar.en[i], &EcoPar.en[i + 1], sizeof (struct Ecosystem));
        swmem(&EcoShift[i].Ecotype, &EcoShift[i + 1].Ecotype,
	sizeof (struct Ecotype *));
        swmem(&Order[i], &Order[i + 1], sizeof (short));
        Changes = 1;
        } /* if */
       } /* else if */
      } /* else if */
     } /* else if */
    } /* else if */
   } /* for i=0... */
  if (Changes)
   Par_Mod |= 0x0100;
  } /* while */

 for (i=0; i<ECOPARAMS; i++)
  {
  for (j=0; j<ECOPARAMS; j++)
   {
   if (PAR_UNDER_ECO(i) == Order[j])
    {
    PAR_UNDER_ECO(i) = j;
    break;
    } /* if */
   } /* for j=0... */
  } /* for i=0... */

 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  for (j=0; j<ECOPARAMS; j++)
   {
   if (KF[i].EcoKey.Item == Order[j])
    {
    KF[i].EcoKey.Item = j;
    break;
    } /* if */
   } /* for j=0... */
  } /* for i=0... */

 for (j=0; j<ECOPARAMS; j++)
  {
  if (EE_Win->EcoItem == Order[j])
   {
   EE_Win->EcoItem = j;
   break;
   } /* if */
  } /* for j=0... */

} /* Sort_Eco_Params() */

/**********************************************************************/

void FixPar(short k, short ParSet)
{

 if (ParSet & 0x0001)
  {
  memcpy(&UndoMoPar[k], &MoPar, sizeof (MoPar));
  } /* motion */
 if (ParSet & 0x0010)
  {
  memcpy(&UndoCoPar[k], &CoPar, sizeof (CoPar));
  } /* color */
 if (ParSet & 0x0100)
  {
  memcpy(&UndoEcoPar[k], &EcoPar, sizeof (EcoPar));
  } /* ecosystem */
 if (ParSet & 0x1000)
  {
  memcpy(&UndoSetPar[k], &settings, sizeof (settings));
  } /* settings */

 if (k == 1)
  {
  if (KFsize > 0)
   {
   switch (ParSet)
    {
    case 0x1111:
     {
     if (UndoKF)
      free_Memory(UndoKF, UndoKFsize);
     if ((UndoKF = (union KeyFrame *)get_Memory(KFsize, MEMF_ANY)) != NULL)
      {
      memcpy(UndoKF, KF, KFsize);
      UndoKFsize = KFsize;
      UndoKeyFrames = ParHdr.KeyFrames;
      }
     else
      UndoKFsize = 0;
     break;
     } /* case 0x1111 */
    case 0x0001:
     {
     MergeKeyFrames(KF, ParHdr.KeyFrames, &UndoKF, &UndoKeyFrames, &UndoKFsize, 0);
     break;
     } /* case 0x0001 */
    case 0x0010:
     {
     MergeKeyFrames(KF, ParHdr.KeyFrames, &UndoKF, &UndoKeyFrames, &UndoKFsize, 1);
     break;
     } /* case 0x0010 */
    case 0x0100:
     {
     MergeKeyFrames(KF, ParHdr.KeyFrames, &UndoKF, &UndoKeyFrames, &UndoKFsize, 2);
     break;
     } /* case 0x0100 */
    } /* switch */
   } /* if keyframes allocated */
  } /* if undo buffer */

} /* FixPar() */

/**********************************************************************/

void UndoPar(short k, short ParUndo)
{

 if (ParUndo & 0x0001)
  {
  memcpy(&MoPar, &UndoMoPar[k], sizeof (MoPar));
  } /* motion */
 if (ParUndo & 0x0010)
  {
  memcpy(&CoPar, &UndoCoPar[k], sizeof (CoPar));
  } /* color */
 if (ParUndo & 0x0100)
  {
  memcpy(&EcoPar, &UndoEcoPar[k], sizeof (EcoPar));
  } /* ecosystem */
 if (ParUndo & 0x1000)
  {
  memcpy(&settings, &UndoSetPar[k], sizeof (settings));
  } /* settings */

 if (k == 1 && ParUndo == 0x1111)
  {
  if (UndoKF && UndoKFsize > 0)
   {
   if (KF)
    free_Memory(KF, KFsize);
   if ((KF = (union KeyFrame *)get_Memory(UndoKFsize, MEMF_ANY)) != NULL)
    {
    memcpy(KF, UndoKF, UndoKFsize);
    KFsize = UndoKFsize;
    } /* if memory OK */
   else
    {
    KF = UndoKF;
    KFsize = UndoKFsize;
    UndoKF = NULL;
    UndoKFsize = 0;
    } /* else no memory, reassign pointer */
   ParHdr.KeyFrames = UndoKeyFrames;
   } /* if keyframes allocated */
  } /* if undo buffer and all parameter groups */

} /* UndoPar() */

/**********************************************************************/

short loadparams(USHORT loadcode, short loaditem)
{
 short success = 1;
// long ByteOrder;
 char temppath[256], tempfile[32], filename[255], Ptrn[32];
 float fileversion;
 FILE *fparam;
 struct ParHeader TempHdr;

 strcpy(temppath, parampath);
 strcpy(tempfile, paramfile);
 strcpy(Ptrn, "#?.par");
 if (! getfilenameptrn(0, "Load Parameter File", temppath, tempfile, Ptrn))
  return (0);

 strmfp(filename, temppath, tempfile);
 if ((fparam = fopen(filename, "rb")) != NULL)
  {
  if ((fread((char *)&TempHdr, sizeof (struct ParHeader), 1, fparam)) == 1)
   {
// AF: 10.Dec.2022, Endian correction for i386-aros
#ifdef __AROS__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      SimpleEndianFlip32F(TempHdr.Version,        &TempHdr.Version);
      SimpleEndianFlip32S(TempHdr.ByteOrder,      &TempHdr.ByteOrder);
      SimpleEndianFlip16S(TempHdr.KeyFrames,      &TempHdr.KeyFrames);
      SimpleEndianFlip32S(TempHdr.MotionParamsPos,&TempHdr.MotionParamsPos);
      SimpleEndianFlip32S(TempHdr.ColorParamsPos, &TempHdr.ColorParamsPos);
      SimpleEndianFlip32S(TempHdr.EcoParamsPos,   &TempHdr.EcoParamsPos);
      SimpleEndianFlip32S(TempHdr.SettingsPos,    &TempHdr.SettingsPos);
      SimpleEndianFlip32S(TempHdr.KeyFramesPos,   &TempHdr.KeyFramesPos);

#endif
#endif
   if (! strncmp(TempHdr.FType, "%GISPAR", 8))
    {
    fileversion = TempHdr.Version;
    }
   else if (! strncmp(TempHdr.FType, "%WCSPAR", 8))
    {
//    ByteOrder = TempHdr.ByteOrder;
    fileversion = TempHdr.Version;
    }
   else fileversion = 0.0;
   if (fileversion < 1.0)
    {
    fclose(fparam);
    Log(ERR_WRONG_TYPE, (CONST_STRPTR)"Version < 1.0");
    User_Message((CONST_STRPTR)"Parameter Module: Load",
            (CONST_STRPTR)"Unsupported Parameter file type or version!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    } /*  if version = 0.0 */

   else if (fileversion < 2.0)
    {
    fclose(fparam);
    if (loadcode & 0x0100)
     DisposeEcotypes();
    if ((success = loadparamsV1(loadcode, loaditem, temppath,
	tempfile, &TempHdr, ParHdr.KeyFrames)) > 0
	&& loadcode == 0x1111)
     {
     memcpy(&ParHdr, &TempHdr, sizeof (struct ParHeader));
     if (loadcode == 0x1111)
      {
      if (User_Message_Def((CONST_STRPTR)"Parameter Module: Load",
              (CONST_STRPTR)"This is an old V1 format file! Would you like to re-save it in the new format now?",
	(CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
       {
       strcpy(parampath, temppath);
       strcpy(paramfile, tempfile);
       saveparams(0x1111, -1, 0);
       } /* if re-save */
      } /* if load all */
     } /* if success */
    } /* if version 1.x */
   else if (fileversion < 3.0)
    {
    fclose(fparam);
    if (loadcode & 0x0100)
     DisposeEcotypes();
    if ((success = loadparamsV2(loadcode, loaditem, temppath,
	tempfile, &TempHdr, ParHdr.KeyFrames)) > 0
	&& loadcode == 0x1111)
     {
     memcpy(&ParHdr, &TempHdr, sizeof (struct ParHeader));
     if (fileversion < PAR_CURRENT_VERSION - .05)
      {
      if (User_Message_Def((CONST_STRPTR)"Parameter Module: Load",
              (CONST_STRPTR)"The Parameter File format has been changed slightly since this file was saved.\
 Would you like to re-save it in the new format now?",
 (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
       {
       strcpy(parampath, temppath);
       strcpy(paramfile, tempfile);
       saveparams(0x1111, -1, 0);
       } /* if re-save */
      } /* if outdated version */
     } /* if success and complete file */
    } /* else version 2.x */
   } /* if header read */
  else
   {
   fclose(fparam);
   Log(ERR_READ_FAIL, (CONST_STRPTR)tempfile);
   success = -1;
   } /* else read fail */
  } /* if file opened */
 else
  {
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)tempfile);
  success = -1;
  }

 if (success > 0 && loadcode == 0x1111)
  {
  if (strcmp(temppath, parampath) || strcmp(tempfile, paramfile))
   Proj_Mod = 1;
  strcpy(parampath, temppath);
  strcpy(paramfile, tempfile);
  } /* if load successful */

 return (success);

} /* loadparams() */

/**********************************************************************/

STATIC_FCN short loadparamsV2(USHORT loadcode, short loaditem, char *parampath,
	char *paramfile, struct ParHeader *TempHdr, short ExistingKeyFrames) // used locally only -> static, AF 23.7.2021
{
 short k, LoadKeys, KeyFrames;
 char filename[255], TagItem[32];
 float fileversion;
 FILE *fparam;
 struct Color TempCo;
 struct Ecosystem TempEco;

 LoadKeys = loadcode == 0x1111 ? 1: 0;

 strmfp(filename, parampath, paramfile);
 if ((fparam = fopen(filename, "rb")) == NULL)
  {
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)paramfile);
  goto ReadError;
  } /* if */

 if ((fread((char *)TempHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
  goto ReadError;

 // AF: 10.Dec.2022, Endian correction for i386-aros
 #ifdef __AROS__
 #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
       SimpleEndianFlip32F(TempHdr->Version,        &TempHdr->Version);
       SimpleEndianFlip32S(TempHdr->ByteOrder,      &TempHdr->ByteOrder);
       SimpleEndianFlip16S(TempHdr->KeyFrames,      &TempHdr->KeyFrames);
       SimpleEndianFlip32S(TempHdr->MotionParamsPos,&TempHdr->MotionParamsPos);
       SimpleEndianFlip32S(TempHdr->ColorParamsPos, &TempHdr->ColorParamsPos);
       SimpleEndianFlip32S(TempHdr->EcoParamsPos,   &TempHdr->EcoParamsPos);
       SimpleEndianFlip32S(TempHdr->SettingsPos,    &TempHdr->SettingsPos);
       SimpleEndianFlip32S(TempHdr->KeyFramesPos,   &TempHdr->KeyFramesPos);
 #endif
 #endif

 if (! strncmp(TempHdr->FType, "%WCSPAR", 8))
  fileversion = TempHdr->Version;
 else fileversion = 0.0;
 if (fileversion < 2.0)
  {
  goto ReadError;
  } /*  if version = 0.0 */
 KeyFrames = TempHdr->KeyFrames;

 if (loadcode != 0x1111)
  {
  if ((loadcode & 0x0111) && KeyFrames > 0)
   LoadKeys = User_Message_Def((CONST_STRPTR)"Parameter Module: Load",
           (CONST_STRPTR)"Load all key frames?", (CONST_STRPTR)"Yes|No", (CONST_STRPTR)"yn", 1);
  } /* if load partial file */

 if (LoadKeys)
  {
  if (KF) free_Memory(KF, KFsize);
  TempHdr->KeyFrames = 0;
  ParHdr.KeyFrames = 0;
  KFsize = (KeyFrames + 20) * (sizeof (union KeyFrame));
  if ((KF = (union KeyFrame *)get_Memory(KFsize, MEMF_CLEAR)) == NULL)
   {
   User_Message((CONST_STRPTR)"Parameter Module: Load",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   KFsize = 0;
   goto ReadError;
   } /* if memory bust */
  } /* if load all parameters */

 if (loadcode & 0x01)
  {
  if (loaditem < 0)
   {
   if ((fread((char *)&MoPar, sizeof (struct Animation), 1, fparam)) != 1)
    goto ReadError;

   // AF: 10.Dec.2022, Endian correction for i386-aros
   #ifdef __AROS__
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   for(unsigned int i=0;i<MOTIONPARAMS;i++)
   {
         SimpleEndianFlip64(MoPar.mn[i].Value,&MoPar.mn[i].Value);
   }
   #endif
   #endif

   }
  else
   {
   fseek(fparam, loaditem * (sizeof (struct Motion)), 1);
   if ((fread((char *)&MoPar.mn[loaditem], sizeof (struct Motion), 1, fparam)) != 1)
    goto ReadError;

   // AF: 10.Dec.2022, Endian correction for i386-aros
   #ifdef __AROS__
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
         SimpleEndianFlip64(MoPar.mn[loaditem].Value,&MoPar.mn[loaditem].Value);
   #endif
   #endif

   fseek(fparam, TempHdr->ColorParamsPos, 0);
   } /* else */
  } /*  if */
 else
  fseek(fparam, sizeof (struct Animation), 1);

 if (loadcode & 0x10)
  {
  if (loaditem < 0)
   {
   if ((fread((char *)&CoPar, sizeof (struct Palette), 1, fparam)) != 1)
    goto ReadError;

   // AF: 10.Dec.2022, Endian correction for i386-aros
   ENDIAN_CHANGE_IF_NEEDED(
           for(unsigned int i=0; i<COLORPARAMS;i++)
           {
               SimpleEndianFlip16S(CoPar.cn[i].Value[0],&CoPar.cn[i].Value[0]);
               SimpleEndianFlip16S(CoPar.cn[i].Value[1],&CoPar.cn[i].Value[1]);
               SimpleEndianFlip16S(CoPar.cn[i].Value[2],&CoPar.cn[i].Value[2]);
           }
       )
   }
  else
   {
   if (loaditem < 24)
    {
    fseek(fparam, loaditem * (sizeof (struct Color)), 1);
    if ((fread((char *)&CoPar.cn[loaditem], sizeof (struct Color), 1, fparam)) != 1)
     goto ReadError;

    // AF: 10.Dec.2022, Endian correction for i386-aros
    ENDIAN_CHANGE_IF_NEEDED(
         SimpleEndianFlip16S(CoPar.cn[loaditem].Value[0],&CoPar.cn[loaditem].Value[0]);
         SimpleEndianFlip16S(CoPar.cn[loaditem].Value[1],&CoPar.cn[loaditem].Value[1]);
         SimpleEndianFlip16S(CoPar.cn[loaditem].Value[2],&CoPar.cn[loaditem].Value[2]);
       )
    } /* if load defined color */
   else
    {
    short found = 0;

    fseek(fparam, 24 * (sizeof (struct Color)), 1); 
    for (k=24; k<COLORPARAMS; k++)
     {
     if ((fread((char *)&TempCo, sizeof (struct Color), 1, fparam)) != 1)
      goto ReadError;

     // AF: 10.Dec.2022, Endian correction for i386-aros
     ENDIAN_CHANGE_IF_NEEDED(
           SimpleEndianFlip16S(TempCo.Value[0],&TempCo.Value[0]);
           SimpleEndianFlip16S(TempCo.Value[1],&TempCo.Value[1]);
           SimpleEndianFlip16S(TempCo.Value[2],&TempCo.Value[2]);
        )

     if (! strcmp(TempCo.Name, PAR_NAME_COLOR(loaditem)))
      {
      memcpy(&CoPar.cn[loaditem], &TempCo, sizeof (struct Color));
      found = 1;
      break;
      } /* if */
     } /* for */
    if (! found)
     {
     sprintf(str, "Color item %s not found in this file!\nOperation terminated.", PAR_NAME_COLOR(loaditem));
     User_Message((CONST_STRPTR)"Color Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     } /* if not found */ 
    } /* else load user color */
   fseek(fparam, TempHdr->EcoParamsPos, 0);
   } /* else load one color */
/* got to modify at least those parameter sets that will be distributed */
  } /* if load color */
 else fseek(fparam, sizeof (struct Palette), 1);

 if (loadcode & 0x100)
  {
  if (loaditem < 0)
   {
   if ((fread((char *)&EcoPar, sizeof (union Environment), 1, fparam)) != 1)
    goto ReadError;

   // AF: 10.Dec.2022, Endian correction for i386-aros
   #ifdef __AROS__
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

   for(unsigned int i=0; i< ECOPARAMS; i++)
   {
   SimpleEndianFlip32F(EcoPar.en[i].Line,         &(EcoPar.en[i].Line));
   SimpleEndianFlip32F(EcoPar.en[i].Skew,         &(EcoPar.en[i].Skew));
   SimpleEndianFlip32F(EcoPar.en[i].SkewAz,       &(EcoPar.en[i].SkewAz));
   SimpleEndianFlip32F(EcoPar.en[i].RelEl,        &(EcoPar.en[i].RelEl));
   SimpleEndianFlip32F(EcoPar.en[i].MaxRelEl,     &(EcoPar.en[i].MaxRelEl));
   SimpleEndianFlip32F(EcoPar.en[i].MinRelEl,     &(EcoPar.en[i].MinRelEl));
   SimpleEndianFlip32F(EcoPar.en[i].MaxSlope,     &(EcoPar.en[i].MaxSlope));
   SimpleEndianFlip32F(EcoPar.en[i].MinSlope,     &(EcoPar.en[i].MinSlope));
   SimpleEndianFlip32F(EcoPar.en[i].Density,      &(EcoPar.en[i].Density));
   SimpleEndianFlip32F(EcoPar.en[i].Height,       &(EcoPar.en[i].Height));
   SimpleEndianFlip16S(EcoPar.en[i].Type,         &(EcoPar.en[i].Type));
   SimpleEndianFlip16S(EcoPar.en[i].Color,        &(EcoPar.en[i].Color));
   SimpleEndianFlip16S(EcoPar.en[i].UnderEco,     &(EcoPar.en[i].UnderEco));
   SimpleEndianFlip16S(EcoPar.en[i].MatchColor[0],&(EcoPar.en[i].MatchColor[0]));
   SimpleEndianFlip16S(EcoPar.en[i].MatchColor[1],&(EcoPar.en[i].MatchColor[1]));
   SimpleEndianFlip16S(EcoPar.en[i].MatchColor[2],&(EcoPar.en[i].MatchColor[2]));
   }
   #endif
   #endif

   } /* if load all ecosystems */
  else
   {
   if (loaditem < 12)
    {
    fseek(fparam, loaditem * (sizeof (struct Ecosystem)), 1);
    if ((fread((char *)&EcoPar.en[loaditem], sizeof (struct Ecosystem), 1, fparam)) != 1)
     goto ReadError;

    // AF: 10.Dec.2022, Endian correction for i386-aros
    #ifdef __AROS__
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    SimpleEndianFlip32F(EcoPar.en[loaditem].Line,         &EcoPar.en[loaditem].Line);
    SimpleEndianFlip32F(EcoPar.en[loaditem].Skew,         &EcoPar.en[loaditem].Skew);
    SimpleEndianFlip32F(EcoPar.en[loaditem].SkewAz,       &EcoPar.en[loaditem].SkewAz);
    SimpleEndianFlip32F(EcoPar.en[loaditem].RelEl,        &EcoPar.en[loaditem].RelEl);
    SimpleEndianFlip32F(EcoPar.en[loaditem].MaxRelEl,     &EcoPar.en[loaditem].MaxRelEl);
    SimpleEndianFlip32F(EcoPar.en[loaditem].MinRelEl,     &EcoPar.en[loaditem].MinRelEl);
    SimpleEndianFlip32F(EcoPar.en[loaditem].MaxSlope,     &EcoPar.en[loaditem].MaxSlope);
    SimpleEndianFlip32F(EcoPar.en[loaditem].MinSlope,     &EcoPar.en[loaditem].MinSlope);
    SimpleEndianFlip32F(EcoPar.en[loaditem].Density,      &EcoPar.en[loaditem].Density);
    SimpleEndianFlip32F(EcoPar.en[loaditem].Height,       &EcoPar.en[loaditem].Height);
    SimpleEndianFlip16S(EcoPar.en[loaditem].Type,         &EcoPar.en[loaditem].Type);
    SimpleEndianFlip16S(EcoPar.en[loaditem].Color,        &EcoPar.en[loaditem].Color);
    SimpleEndianFlip16S(EcoPar.en[loaditem].UnderEco,     &EcoPar.en[loaditem].UnderEco);
    SimpleEndianFlip16S(EcoPar.en[loaditem].MatchColor[0],&EcoPar.en[loaditem].MatchColor[0]);
    SimpleEndianFlip16S(EcoPar.en[loaditem].MatchColor[1],&EcoPar.en[loaditem].MatchColor[1]);
    SimpleEndianFlip16S(EcoPar.en[loaditem].MatchColor[2],&EcoPar.en[loaditem].MatchColor[2]);

    #endif
    #endif

    } /* if */
   else
    {
    short found = 0;

    fseek(fparam, 12 * (sizeof (struct Ecosystem)), 1);
    for (k=12; k<ECOPARAMS; k++)
     {
     if ((fread((char *)&TempEco, sizeof (struct Ecosystem), 1, fparam)) != 1)
      goto ReadError;

     // AF: 10.Dec.2022, Endian correction for i386-aros
     #ifdef __AROS__
     #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

     SimpleEndianFlip32F(TempEco.Line,         &TempEco.Line);
     SimpleEndianFlip32F(TempEco.Skew,         &TempEco.Skew);
     SimpleEndianFlip32F(TempEco.SkewAz,       &TempEco.SkewAz);
     SimpleEndianFlip32F(TempEco.RelEl,        &TempEco.RelEl);
     SimpleEndianFlip32F(TempEco.MaxRelEl,     &TempEco.MaxRelEl);
     SimpleEndianFlip32F(TempEco.MinRelEl,     &TempEco.MinRelEl);
     SimpleEndianFlip32F(TempEco.MaxSlope,     &TempEco.MaxSlope);
     SimpleEndianFlip32F(TempEco.MinSlope,     &TempEco.MinSlope);
     SimpleEndianFlip32F(TempEco.Density,      &TempEco.Density);
     SimpleEndianFlip32F(TempEco.Height,       &TempEco.Height);
     SimpleEndianFlip16S(TempEco.Type,         &TempEco.Type);
     SimpleEndianFlip16S(TempEco.Color,        &TempEco.Color);
     SimpleEndianFlip16S(TempEco.UnderEco,     &TempEco.UnderEco);
     SimpleEndianFlip16S(TempEco.MatchColor[0],&TempEco.MatchColor[0]);
     SimpleEndianFlip16S(TempEco.MatchColor[1],&TempEco.MatchColor[1]);
     SimpleEndianFlip16S(TempEco.MatchColor[2],&TempEco.MatchColor[2]);

     #endif
     #endif

     if (! strcmp(TempEco.Name, PAR_NAME_ECO(loaditem)))
      {
      memcpy(&EcoPar.en[loaditem], &TempEco, sizeof (struct Ecosystem));
      found = 1;
      break;
      } /* if */
     } /* for */
    if (! found)
     {
     sprintf(str, "Ecosystem item %s not found in this file!\nOperation terminated.", PAR_NAME_ECO(loaditem));
     User_Message((CONST_STRPTR)"Ecosystem Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     } /* if not found */ 
    } /* else */
   fseek(fparam, TempHdr->SettingsPos, 0);
   } /* else */
  } /* if */
 else
  fseek(fparam, sizeof (union Environment), 1);

 if (loadcode & 0x1000)
  {
  if ((fread((char *)&settings, sizeof (struct Settings), 1, fparam)) != 1)
   goto ReadError;
  
  // AF: 10.Dec.2022, Endian correction for i386-aros
  #ifdef __AROS__
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

  SimpleEndianFlip16S(settings.startframe,   &settings.startframe);
  SimpleEndianFlip16S(settings.maxframes,    &settings.maxframes);
  SimpleEndianFlip16S(settings.startseg,     &settings.startseg);
  SimpleEndianFlip16S(settings.smoothfaces,  &settings.smoothfaces);
  SimpleEndianFlip16S(settings.bankturn,     &settings.bankturn);
  SimpleEndianFlip16S(settings.colrmap,      &settings.colrmap);
  SimpleEndianFlip16S(settings.borderandom,  &settings.borderandom);
  SimpleEndianFlip16S(settings.cmaptrees,    &settings.cmaptrees);
  SimpleEndianFlip16S(settings.rendertrees,  &settings.rendertrees);
  SimpleEndianFlip16S(settings.statistics,   &settings.statistics);
  SimpleEndianFlip16S(settings.stepframes,   &settings.stepframes);
  SimpleEndianFlip16S(settings.zbufalias,    &settings.zbufalias);
  SimpleEndianFlip16S(settings.horfix,       &settings.horfix);
  SimpleEndianFlip16S(settings.horizonmax,   &settings.horizonmax);
  SimpleEndianFlip16S(settings.clouds,       &settings.clouds);
  SimpleEndianFlip16S(settings.linefade,     &settings.linefade);
  SimpleEndianFlip16S(settings.drawgrid,     &settings.drawgrid);
  SimpleEndianFlip16S(settings.gridsize,     &settings.gridsize);
  SimpleEndianFlip16S(settings.alternateq,   &settings.alternateq);
  SimpleEndianFlip16S(settings.linetoscreen, &settings.linetoscreen);
  SimpleEndianFlip16S(settings.mapassfc,     &settings.mapassfc);
  SimpleEndianFlip16S(settings.cmapluminous, &settings.cmapluminous);
  SimpleEndianFlip16S(settings.surfel[0],    &settings.surfel[0]);
  SimpleEndianFlip16S(settings.surfel[1],    &settings.surfel[1]);
  SimpleEndianFlip16S(settings.surfel[2],    &settings.surfel[2]);
  SimpleEndianFlip16S(settings.surfel[3],    &settings.surfel[3]);


  SimpleEndianFlip16S(settings.worldmap,   &settings.worldmap);
  SimpleEndianFlip16S(settings.flatteneco, &settings.flatteneco);
  SimpleEndianFlip16S(settings.fixfract,   &settings.fixfract);
  SimpleEndianFlip16S(settings.vecsegs,    &settings.vecsegs);
  SimpleEndianFlip16S(settings.reliefshade,&settings.reliefshade);
  SimpleEndianFlip16S(settings.renderopts, &settings.renderopts);
  SimpleEndianFlip16S(settings.scrnwidth,  &settings.scrnwidth);
  SimpleEndianFlip16S(settings.scrnheight, &settings.scrnheight);
  SimpleEndianFlip16S(settings.rendersegs, &settings.rendersegs);
  SimpleEndianFlip16S(settings.overscan,   &settings.overscan);
  SimpleEndianFlip16S(settings.lookahead,  &settings.lookahead);
  SimpleEndianFlip16S(settings.composite,  &settings.composite);
  SimpleEndianFlip16S(settings.defaulteco, &settings.defaulteco);
  SimpleEndianFlip16S(settings.ecomatch,   &settings.ecomatch);
  SimpleEndianFlip16S(settings.Yoffset,    &settings.Yoffset);
  SimpleEndianFlip16S(settings.saveIFF,    &settings.saveIFF);
  SimpleEndianFlip16S(settings.background, &settings.background);
  SimpleEndianFlip16S(settings.zbuffer,    &settings.zbuffer);
  SimpleEndianFlip16S(settings.antialias,  &settings.antialias);
  SimpleEndianFlip16S(settings.scaleimage, &settings.scaleimage);
  SimpleEndianFlip16S(settings.fractal,    &settings.fractal);
  SimpleEndianFlip16S(settings.aliasfactor,&settings.aliasfactor);
  SimpleEndianFlip16S(settings.scalewidth, &settings.scalewidth);
  SimpleEndianFlip16S(settings.scaleheight,&settings.scaleheight);

  SimpleEndianFlip16S(settings.exportzbuf,     &settings.exportzbuf);
  SimpleEndianFlip16S(settings.zformat,        &settings.zformat);
  SimpleEndianFlip16S(settings.fieldrender,    &settings.fieldrender);
  SimpleEndianFlip16S(settings.lookaheadframes,&settings.lookaheadframes);
  SimpleEndianFlip16S(settings.velocitydistr,  &settings.velocitydistr);
  SimpleEndianFlip16S(settings.easein,         &settings.easein);
  SimpleEndianFlip16S(settings.easeout,        &settings.easeout);
  SimpleEndianFlip16S(settings.displace,       &settings.displace);
  SimpleEndianFlip16S(settings.mastercmap,     &settings.mastercmap);
  SimpleEndianFlip16S(settings.cmaporientation,&settings.cmaporientation);
  SimpleEndianFlip16S(settings.fielddominance, &settings.fielddominance);
  SimpleEndianFlip16S(settings.fractalmap,     &settings.fractalmap);
  SimpleEndianFlip16S(settings.perturb,        &settings.perturb);
  SimpleEndianFlip16S(settings.realclouds,     &settings.realclouds);
  SimpleEndianFlip16S(settings.reflections,    &settings.reflections);
  SimpleEndianFlip16S(settings.waves,          &settings.waves);
  SimpleEndianFlip16S(settings.colorstrata,    &settings.colorstrata);
  SimpleEndianFlip16S(settings.cmapsurface,    &settings.cmapsurface);
  SimpleEndianFlip16S(settings.deformationmap, &settings.deformationmap);
  SimpleEndianFlip16S(settings.moon,           &settings.moon);
  SimpleEndianFlip16S(settings.sun,            &settings.sun);
  SimpleEndianFlip16S(settings.tides,          &settings.tides);
  SimpleEndianFlip16S(settings.sunhalo,        &settings.sunhalo);
  SimpleEndianFlip16S(settings.moonhalo,       &settings.moonhalo);

  for(unsigned int i=0; i<EXTRASHORTSETTINGS; i++)
  {
      SimpleEndianFlip16S(settings.extrashorts[i],&settings.extrashorts[i]);
  }

  for(unsigned int i=0; i<EXTRADOUBLESETTINGS; i++)
  {
      SimpleEndianFlip64(settings.extradoubles[i],&settings.extradoubles[i]);
  }

  SimpleEndianFlip64(settings.deformscale,  &settings.deformscale);
  SimpleEndianFlip64(settings.stratadip,    &settings.stratadip);
  SimpleEndianFlip64(settings.stratastrike, &settings.stratastrike);
  SimpleEndianFlip64(settings.dispslopefact,&settings.dispslopefact);
  SimpleEndianFlip64(settings.globecograd,  &settings.globecograd);
  SimpleEndianFlip64(settings.globsnowgrad, &settings.globsnowgrad);
  SimpleEndianFlip64(settings.globreflat,   &settings.globreflat);
  SimpleEndianFlip64(settings.zalias,       &settings.zalias);
  SimpleEndianFlip64(settings.bankfactor,   &settings.bankfactor);
  SimpleEndianFlip64(settings.skyalias,     &settings.skyalias);
  SimpleEndianFlip64(settings.lineoffset,   &settings.lineoffset);
  SimpleEndianFlip64(settings.altqlat,      &settings.altqlat);
  SimpleEndianFlip64(settings.altqlon,      &settings.altqlon);
  SimpleEndianFlip64(settings.treefactor,   &settings.treefactor);
  SimpleEndianFlip64(settings.displacement, &settings.displacement);
  SimpleEndianFlip64(settings.unused3,      &settings.unused3);
  SimpleEndianFlip64(settings.picaspect,    &settings.picaspect);
  SimpleEndianFlip64(settings.zenith,       &settings.zenith);

  #endif
  #endif
  
  }
 else
  fseek(fparam, sizeof (struct Settings), 1);

 if (LoadKeys)
  {
  if (KeyFrames > 0)
   {
   if ((fread((char *)KF, KeyFrames * sizeof (union KeyFrame), 1, fparam)) != 1)
    {
    goto ReadError;
    }
   ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-aros */
		   for(unsigned int i=0;i<KeyFrames;i++)
		   {
			   SimpleEndianFlip16S(KF[i].MoKey.KeyFrame,&KF[i].MoKey.KeyFrame);
			   SimpleEndianFlip16S(KF[i].MoKey.Group,   &KF[i].MoKey.Group);
			   SimpleEndianFlip16S(KF[i].MoKey.Item,    &KF[i].MoKey.Item);
			   SimpleEndianFlip32F(KF[i].MoKey.TCB[0],  &KF[i].MoKey.TCB[0]);
			   SimpleEndianFlip32F(KF[i].MoKey.TCB[1],  &KF[i].MoKey.TCB[1]);
			   SimpleEndianFlip32F(KF[i].MoKey.TCB[2],  &KF[i].MoKey.TCB[2]);
			   SimpleEndianFlip16S(KF[i].MoKey.Linear,  &KF[i].MoKey.Linear);
			   // all union-components are identical up to here

			   if(KF[i].MoKey.Group == 0)  // Motion Key
			   {
			       SimpleEndianFlip64(KF [i].MoKey.Value,   &KF[i].MoKey.Value);;
			   }
			   else if (KF[i].MoKey.Group == 1)  // Color Key
                {
			       SimpleEndianFlip16S(KF[i].CoKey.Value[0],  &KF[i].CoKey.Value[0]);
			       SimpleEndianFlip16S(KF[i].CoKey.Value[1],  &KF[i].CoKey.Value[1]);
			       SimpleEndianFlip16S(KF[i].CoKey.Value[2],  &KF[i].CoKey.Value[2]);
                }
			   else // 2=Ecosystem Key, ??=Cloud and ??=Wave
			   {

			       // AF: 2.Jan.23: Eco and Eco2 have 10 floats, Cloud has 7 and wave has 4 floats. So with 10 flips we change all in all cases
			       for(unsigned int k=0;k<10;k++)
			       {
			           SimpleEndianFlip32F(KF[i].EcoKey2.Value[k],&KF[i].EcoKey2.Value[k]); // EcoKey and EcoKey2 have both 10 floats
			       }
			   }
		   }
           )

   TempHdr->KeyFrames = KeyFrames;
   } /* if key frames to read */
  else
   TempHdr->KeyFrames = 0;
  } /* if try to load key frames */
 else
  TempHdr->KeyFrames = ExistingKeyFrames;


 if ((fread((char *)TagItem, sizeof (long), 1, fparam)) == 1)
  {
  TagItem[4] = 0;
  if (! strncmp(TagItem, "ECOT", 4))
   {
   if (loadcode & 0x0100)
    {
    while (! strncmp(TagItem, "ECOT", 4))
     {
     if ((fread((char *)&k, sizeof (short), 1, fparam)) != 1)
      goto ReadError;

     // AF: 10.Dec.2022, Endian correction for i386-aros
     #ifdef __AROS__
     #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

     SimpleEndianFlip16S(k, &k);
#endif
#endif

     if ((EcoShift[k].Ecotype = Ecotype_Load(fparam, 0)) == NULL)
      goto ReadError;
     if ((fread((char *)TagItem, sizeof (long), 1, fparam)) != 1)
      break;

//     // AF: 10.Dec.2022, Endian correction for i386-aros
//     #ifdef __AROS__
//     #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//
//     SimpleEndianFlip32S(TagItem,&TagItem);
//#endif
//#endif


     } /* while */
    } /* if loading ecosystem */
   } /* if tag is ECOT - ecotype */
  } /* if read tag OK */

 if ((fileversion < 2.05) && (loadcode & 0x0100) && (loaditem < 0))
  {
  for (k=0; k<ECOPARAMS; k++)
   {
   if ((PAR_TYPE_ECO(k) % 50) >= 3)
    {
    if ((PAR_TYPE_ECO(k) % 50) == 3)
     {
     if (PAR_TYPE_ECO(k) == 3)
      {
      PAR_TYPE_ECO(k) = 103;
      PAR_FIRSTDN_ECO(k) = 100.0;
      } /* if actually strata rock */
     PAR_TYPE_ECO(k) += 0x0100;
     if (settings.colorstrata)
      PAR_TYPE_ECO(k) += 0x0200;
     } /* if strata rock */
    PAR_TYPE_ECO(k) --;
    } /* if */
   } /* for k=0... */
  settings.colorstrata = 0;
  } /* if Version 2.0 */

 fclose(fparam);
 switch (loadcode)
  {
  case 0x0001:
   {
   sprintf(str, "%s motion, Ver %f", paramfile, TempHdr->Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* motion only */
  case 0x0010:
   {
   sprintf(str, "%s colors, Ver %f", paramfile, TempHdr->Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* colors only */
  case 0x0100:
   {
   sprintf(str, "%s ecosystems, Ver %f", paramfile, TempHdr->Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* ecosystems only */
  case 0x1000:
   {
   sprintf(str, "%s settings, Ver %f", paramfile, TempHdr->Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* settings only */
  case 0x1111:
   {
   sprintf(str, "%s all, Ver %f", paramfile, TempHdr->Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* all parameters */
  } /* switch loadcode */

 Par_Mod &= (loadcode ^ 0x1111);

 strcpy(TempHdr->FType, "%WCSPAR");
 TempHdr->Version = PAR_CURRENT_VERSION;
 TempHdr->ByteOrder = 0xaabbccdd;

/* temporary fix for those image ecosystems I already created */
 if (loadcode & 0x0100 && loaditem < 0)
 {
     for (k=0; k<ECOPARAMS; k++)
     {  /* I guess the original authors forget the {} of the for loop? (AF, 7.7.2021) */
         if (PAR_TYPE_ECO(k) == 10)
         {
             PAR_TYPE_ECO(k) = 5;
         }
         else if (PAR_TYPE_ECO(k) == 60) /* without the {} of the for loop we would have an buffer-overlow here! (k=50, PAR_TYPE_ECO() 0...49 ((AF, 7.7.2021)) */
         {
             PAR_TYPE_ECO(k) = 55;
         }
     }
 } /* if */

 return (1);

ReadError:
 Log(ERR_READ_FAIL, (CONST_STRPTR)paramfile);
 fclose(fparam);

 return (-1);

} /* loadparamsV2() */

/************************************************************************/

STATIC_FCN short loadparamsV1(USHORT loadcode, short loaditem, char *parampath,
	char *paramfile, struct ParHeader *TempHdr, short ExistingKeyFrames) // used locally only -> static, AF 23.7.2021
{
 short k, LoadKeys, KeyFrames;
 long KFV1size;
 char filename[255];
 float fileversion;
 FILE *fparam;
 struct ParHeaderV1 TempHdrV1;
 struct ColorV1 TempCo;
 struct EcosystemV1 TempEco;
 union NoLinearKeyFrame TempKF;
 union KeyFrameV1 *KFV1 = NULL;
 struct SettingsV1 *settingsV1;
 union EnvironmentV1 *EcoParV1;
 struct PaletteV1 *CoParV1;
 struct AnimationV1 *MoParV1;

 settingsV1 = (struct SettingsV1 *)
	get_Memory(sizeof (struct SettingsV1), MEMF_CLEAR);
 MoParV1 = (struct AnimationV1 *)
	get_Memory(sizeof (struct AnimationV1), MEMF_CLEAR);
 CoParV1 = (struct PaletteV1 *)
	get_Memory(sizeof (struct PaletteV1), MEMF_CLEAR);
 EcoParV1 = (union EnvironmentV1 *)
	get_Memory(sizeof (union EnvironmentV1), MEMF_CLEAR);

 if (! settingsV1 || ! MoParV1 || ! CoParV1 || ! EcoParV1)
  {
  User_Message((CONST_STRPTR)"Parameter Module: Load",
          (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto ReadError;
  } /* if memory bust */

 LoadKeys = loadcode == 0x1111 ? 1: 0;

 strmfp(filename, parampath, paramfile);
 if ((fparam = fopen(filename, "rb")) == NULL)
  {
  Log(ERR_OPEN_FAIL, (CONST_STRPTR)paramfile);
  goto ReadError;
  } /* if */

 if ((fread((char *)&TempHdrV1, sizeof TempHdrV1, 1, fparam)) != 1)
  goto ReadError;

 ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-aros */
     SimpleEndianFlip32F(TempHdrV1.Version,&TempHdrV1.Version);
     SimpleEndianFlip16S(TempHdrV1.FirstFrame,&TempHdrV1.FirstFrame);
     SimpleEndianFlip16S(TempHdrV1.LastFrame,&TempHdrV1.LastFrame);
     SimpleEndianFlip16S(TempHdrV1.KeyFrames,&TempHdrV1.KeyFrames);
     SimpleEndianFlip16S(TempHdrV1.CurrentKey,&TempHdrV1.CurrentKey);
         )

 if (! strncmp(TempHdrV1.FType, "%GISPAR", 8))
  fileversion = TempHdrV1.Version;
 else fileversion = 0.0;
 if (fileversion < 1.0)
  {
  goto ReadError;
  } /*  if version = 0.0 */
 KeyFrames = TempHdrV1.KeyFrames;

 if (loadcode != 0x1111)
  {
  if (loadcode & 0x0111 && KeyFrames > 0)
   LoadKeys = User_Message_Def((CONST_STRPTR)"Parameter Module: Load",
           (CONST_STRPTR)"Load all key frames?", (CONST_STRPTR)"Yes|No", (CONST_STRPTR)"yn", 1);
  } /* if load partial file */

 if (LoadKeys)
  {
  if (KF) free_Memory(KF, KFsize);
  TempHdrV1.KeyFrames = 0;
  ParHdr.KeyFrames = 0;
  KFsize = (KeyFrames + 20) * (sizeof (union KeyFrame));
  KFV1size = (KeyFrames + 20) * (sizeof (union KeyFrameV1));
  if ((KF = (union KeyFrame *)get_Memory(KFsize, MEMF_CLEAR)) == NULL ||
	(KFV1 = (union KeyFrameV1 *)get_Memory(KFV1size, MEMF_CLEAR)) == NULL)
   {
   User_Message((CONST_STRPTR)"Parameter Module: Load",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto ReadError;
   } /* if memory bust */
  } /* if load all parameters */

 if (loadcode & 0x01)
  {
  if (loaditem < 0)
   {
   if ((fread((char *)MoParV1, sizeof (struct AnimationV1), 1, fparam)) != 1)
   {
    goto ReadError;
   }
   ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-aros */
        for(unsigned int i=0;i<MOTIONPARAMSV1;i++)
        {
           SimpleEndianFlip64(MoParV1->mn[i].Value[0],&MoParV1->mn[i].Value[0]);
           SimpleEndianFlip64(MoParV1->mn[i].Value[1],&MoParV1->mn[i].Value[1]);
           SimpleEndianFlip16S(MoParV1->mn[i].Frame[0],&MoParV1->mn[i].Frame[0]);
           SimpleEndianFlip16S(MoParV1->mn[i].Frame[1],&MoParV1->mn[i].Frame[1]);
           SimpleEndianFlip16S(MoParV1->mn[i].Ease[0],&MoParV1->mn[i].Ease[0]);
           SimpleEndianFlip16S(MoParV1->mn[i].Ease[1],&MoParV1->mn[i].Ease[1]);
        }
           )
   }
  else
   {
   fseek(fparam, loaditem * (sizeof (struct MotionV1)), 1);
   if ((fread((char *)&MoParV1->mn[loaditem], sizeof (struct MotionV1), 1, fparam)) != 1)
   {
    goto ReadError;
   }
   ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-aros */
           SimpleEndianFlip64(MoParV1->mn[loaditem].Value[0],&MoParV1->mn[loaditem].Value[0]);
           SimpleEndianFlip64(MoParV1->mn[loaditem].Value[1],&MoParV1->mn[loaditem].Value[1]);
           SimpleEndianFlip16S(MoParV1->mn[loaditem].Frame[0],&MoParV1->mn[loaditem].Frame[0]);
           SimpleEndianFlip16S(MoParV1->mn[loaditem].Frame[1],&MoParV1->mn[loaditem].Frame[1]);
           SimpleEndianFlip16S(MoParV1->mn[loaditem].Ease[0],&MoParV1->mn[loaditem].Ease[0]);
           SimpleEndianFlip16S(MoParV1->mn[loaditem].Ease[1],&MoParV1->mn[loaditem].Ease[1]);
           )
   fseek(fparam, sizeof (struct AnimationV1) + sizeof TempHdrV1, 0);
   } /* else */
  } /*  if */
 else
  fseek(fparam, sizeof (struct AnimationV1), 1);

 if (loadcode & 0x10)
  {
  if (loaditem < 0)
   {
   if ((fread((char *)CoParV1, sizeof (struct PaletteV1), 1, fparam)) != 1)
   {
    goto ReadError;
   }
   ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-aros */
           for(unsigned int i=0;i<MOTIONPARAMSV1;i++)
           {
               SimpleEndianFlip16S(CoParV1->cn[i].Value[0],&CoParV1->cn[i].Value[0]);
               SimpleEndianFlip16S(CoParV1->cn[i].Value[1],&CoParV1->cn[i].Value[1]);
               SimpleEndianFlip16S(CoParV1->cn[i].Value[2],&CoParV1->cn[i].Value[2]);
               SimpleEndianFlip16S(CoParV1->cn[i].Value[3],&CoParV1->cn[i].Value[3]);
               SimpleEndianFlip16S(CoParV1->cn[i].Value[4],&CoParV1->cn[i].Value[4]);
               SimpleEndianFlip16S(CoParV1->cn[i].Value[5],&CoParV1->cn[i].Value[5]);
               SimpleEndianFlip16S(CoParV1->cn[i].Frame[0],&CoParV1->cn[i].Frame[0]);
               SimpleEndianFlip16S(CoParV1->cn[i].Frame[1],&CoParV1->cn[i].Frame[1]);
               SimpleEndianFlip16S(CoParV1->cn[i].Ease[0],&CoParV1->cn[i].Ease[0]);
               SimpleEndianFlip16S(CoParV1->cn[i].Ease[1],&CoParV1->cn[i].Ease[1]);
           }
       )
   }
  else
   {
   if (loaditem < 12)
    {
    fseek(fparam, loaditem * (sizeof (struct ColorV1)), 1);
    if ((fread((char *)&CoParV1->cn[loaditem], sizeof (struct ColorV1), 1, fparam)) != 1)
    {
        goto ReadError;
    }
    ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[0],&CoParV1->cn[loaditem].Value[0]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[1],&CoParV1->cn[loaditem].Value[1]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[2],&CoParV1->cn[loaditem].Value[2]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[3],&CoParV1->cn[loaditem].Value[3]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[4],&CoParV1->cn[loaditem].Value[4]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Value[5],&CoParV1->cn[loaditem].Value[5]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Frame[0],&CoParV1->cn[loaditem].Frame[0]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Frame[1],&CoParV1->cn[loaditem].Frame[1]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Ease[0],&CoParV1->cn[loaditem].Ease[0]);
            SimpleEndianFlip16S(CoParV1->cn[loaditem].Ease[1],&CoParV1->cn[loaditem].Ease[1]);
       )

    } /* if load all colors */
   else if (loaditem >= 24)
    {
    short found = 0;

    fseek(fparam, 12 * (sizeof (struct ColorV1)), 1); 
    for (k=12; k<COLORPARAMSV1; k++)
     {
     if ((fread((char *)&TempCo, sizeof (struct ColorV1), 1, fparam)) != 1)
      {
         goto ReadError;
      }
     ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
             SimpleEndianFlip16S(TempCo.Value[0],&TempCo.Value[0]);
             SimpleEndianFlip16S(TempCo.Value[1],&TempCo.Value[1]);
             SimpleEndianFlip16S(TempCo.Value[2],&TempCo.Value[2]);
             SimpleEndianFlip16S(TempCo.Value[3],&TempCo.Value[3]);
             SimpleEndianFlip16S(TempCo.Value[4],&TempCo.Value[4]);
             SimpleEndianFlip16S(TempCo.Value[5],&TempCo.Value[5]);
             SimpleEndianFlip16S(TempCo.Frame[0],&TempCo.Frame[0]);
             SimpleEndianFlip16S(TempCo.Frame[1],&TempCo.Frame[1]);
             SimpleEndianFlip16S(TempCo.Ease[0],&TempCo.Ease[0]);
             SimpleEndianFlip16S(TempCo.Ease[1],&TempCo.Ease[1]);
             )
     if (! strcmp(TempCo.Name, PAR_NAME_COLOR(loaditem)))
      {
      memcpy(&CoParV1->cn[loaditem - 12], &TempCo, sizeof (struct ColorV1));
      found = 1;
      break;
      } /* if */
     } /* for */
    if (! found)
     {
     sprintf(str, "Color item %s not found in this file!\nOperation terminated.", PAR_NAME_COLOR(loaditem));
     User_Message((CONST_STRPTR)"Color Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     goto ReadError;
     } /* if not found */ 
    } /* else */
   else
    {
    sprintf(str, "Color item %s not found in this file!\nOperation terminated.", PAR_NAME_COLOR(loaditem));
    User_Message((CONST_STRPTR)"Color Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    goto ReadError;
    }
   fseek(fparam, sizeof (struct PaletteV1) + sizeof (struct AnimationV1)
		+ sizeof TempHdrV1, 0);
   } /* else load one color */
/* got to modify at least those parameter sets that will be distributed */
  } /* if load color */
 else fseek(fparam, sizeof (struct PaletteV1), 1);

 if (loadcode & 0x100)
  {
  if (loaditem < 0)
   {
   if (fileversion > 1.025)
    {
    if ((fread((char *)EcoParV1, sizeof (union EnvironmentV1), 1, fparam)) != 1)
     {
        goto ReadError;
     }
    ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
            for(unsigned int i=0;i<ECOPARAMSV1;i++)
            {
                SimpleEndianFlip16S(EcoParV1->en[i].Line[0],&EcoParV1->en[i].Line[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].Line[1],&EcoParV1->en[i].Line[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].Skew[0],&EcoParV1->en[i].Skew[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].Skew[1],&EcoParV1->en[i].Skew[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].SkewAz[0],&EcoParV1->en[i].SkewAz[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].SkewAz[1],&EcoParV1->en[i].SkewAz[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].RelEl[0],&EcoParV1->en[i].RelEl[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].RelEl[1],&EcoParV1->en[i].RelEl[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].MaxRelEl[0],&EcoParV1->en[i].MaxRelEl[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].MaxRelEl[1],&EcoParV1->en[i].MaxRelEl[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].MinRelEl[0],&EcoParV1->en[i].MinRelEl[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].MinRelEl[1],&EcoParV1->en[i].MinRelEl[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].MaxSlope[0],&EcoParV1->en[i].MaxSlope[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].MaxSlope[1],&EcoParV1->en[i].MaxSlope[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].MinSlope[0],&EcoParV1->en[i].MinSlope[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].MinSlope[1],&EcoParV1->en[i].MinSlope[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].Type,&EcoParV1->en[i].Type);
                SimpleEndianFlip16S(EcoParV1->en[i].Tree[0],&EcoParV1->en[i].Tree[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].Tree[1],&EcoParV1->en[i].Tree[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].Color,&EcoParV1->en[i].Color);
                SimpleEndianFlip16S(EcoParV1->en[i].UnderEco,&EcoParV1->en[i].UnderEco);
                SimpleEndianFlip16S(EcoParV1->en[i].MatchColor[0],&EcoParV1->en[i].MatchColor[0]);
                SimpleEndianFlip16S(EcoParV1->en[i].MatchColor[1],&EcoParV1->en[i].MatchColor[1]);
                SimpleEndianFlip16S(EcoParV1->en[i].MatchColor[2],&EcoParV1->en[i].MatchColor[2]);
            }
            )
    }
   else
    {
    for (k=0; k<ECOPARAMSV1; k++)
     {
     if ((fread((char *)&EcoParV1->en[k], sizeof (struct OldEcosystemV1), 1, fparam)) != 1)
     {
         goto ReadError;
     }
     ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
             {
     struct OldEcosystemV1 *ptr=(struct OldEcosystemV1*) &EcoParV1->en[k];

             SimpleEndianFlip16S(ptr->Line[0],&ptr->Line[0]);
             SimpleEndianFlip16S(ptr->Line[1],&ptr->Line[1]);
             SimpleEndianFlip16S(ptr->Skew[0],&ptr->Skew[0]);
             SimpleEndianFlip16S(ptr->Skew[1],&ptr->Skew[1]);
             SimpleEndianFlip16S(ptr->SkewAz[0],&ptr->SkewAz[0]);
             SimpleEndianFlip16S(ptr->SkewAz[1],&ptr->SkewAz[1]);
             SimpleEndianFlip16S(ptr->RelEl[0],&ptr->RelEl[0]);
             SimpleEndianFlip16S(ptr->RelEl[1],&ptr->RelEl[1]);
             SimpleEndianFlip16S(ptr->MaxRelEl[0],&ptr->MaxRelEl[0]);
             SimpleEndianFlip16S(ptr->MaxRelEl[1],&ptr->MaxRelEl[1]);
             SimpleEndianFlip16S(ptr->MinRelEl[0],&ptr->MinRelEl[0]);
             SimpleEndianFlip16S(ptr->MinRelEl[1],&ptr->MinRelEl[1]);
             SimpleEndianFlip16S(ptr->MaxSlope[0],&ptr->MaxSlope[0]);
             SimpleEndianFlip16S(ptr->MaxSlope[1],&ptr->MaxSlope[1]);
             SimpleEndianFlip16S(ptr->MinSlope[0],&ptr->MinSlope[0]);
             SimpleEndianFlip16S(ptr->MinSlope[1],&ptr->MinSlope[1]);
             SimpleEndianFlip16S(ptr->Type,&ptr->Type);
             SimpleEndianFlip16S(ptr->Tree[1],&ptr->Tree[1]);
             SimpleEndianFlip16S(ptr->Tree[0],&ptr->Tree[0]);
             SimpleEndianFlip16S(ptr->Color,&ptr->Color);
             SimpleEndianFlip16S(ptr->UnderEco,&ptr->UnderEco);
             SimpleEndianFlip16S(ptr->MatchColor[0],&ptr->MatchColor[0]);
             SimpleEndianFlip16S(ptr->MatchColor[1],&ptr->MatchColor[1]);
             SimpleEndianFlip16S(ptr->MatchColor[2],&ptr->MatchColor[2]);
             SimpleEndianFlip16S(ptr->Frame[0],&ptr->Frame[0]);
             SimpleEndianFlip16S(ptr->Frame[1],&ptr->Frame[1]);
             SimpleEndianFlip16S(ptr->Ease[0],&ptr->Ease[0]);
             SimpleEndianFlip16S(ptr->Ease[1],&ptr->Ease[1]);
            };
     )
     strcpy(EcoParV1->en[k].Model, "\0");
     } /* for k=0... */
    } /* else version < 1.025 */
   } /* if load all ecosystems */
  else
   {
   if (loaditem < 2)
    {
    if (fileversion > 1.025)
     {
     fseek(fparam, loaditem * (sizeof (struct EcosystemV1)), 1);
     if ((fread((char *)&EcoParV1->en[loaditem], sizeof (struct EcosystemV1), 1, fparam)) != 1)
      {
         goto ReadError;
      }
     ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Line[0],&EcoParV1->en[loaditem].Line[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Line[1],&EcoParV1->en[loaditem].Line[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Skew[0],&EcoParV1->en[loaditem].Skew[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Skew[1],&EcoParV1->en[loaditem].Skew[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].SkewAz[0],&EcoParV1->en[loaditem].SkewAz[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].SkewAz[1],&EcoParV1->en[loaditem].SkewAz[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].RelEl[0],&EcoParV1->en[loaditem].RelEl[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].RelEl[1],&EcoParV1->en[loaditem].RelEl[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MaxRelEl[0],&EcoParV1->en[loaditem].MaxRelEl[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MaxRelEl[1],&EcoParV1->en[loaditem].MaxRelEl[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MinRelEl[0],&EcoParV1->en[loaditem].MinRelEl[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MinRelEl[1],&EcoParV1->en[loaditem].MinRelEl[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MaxSlope[0],&EcoParV1->en[loaditem].MaxSlope[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MaxSlope[1],&EcoParV1->en[loaditem].MaxSlope[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MinSlope[0],&EcoParV1->en[loaditem].MinSlope[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MinSlope[1],&EcoParV1->en[loaditem].MinSlope[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Type,&EcoParV1->en[loaditem].Type);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Tree[0],&EcoParV1->en[loaditem].Tree[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Tree[1],&EcoParV1->en[loaditem].Tree[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].Color,&EcoParV1->en[loaditem].Color);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].UnderEco,&EcoParV1->en[loaditem].UnderEco);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MatchColor[0],&EcoParV1->en[loaditem].MatchColor[0]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MatchColor[1],&EcoParV1->en[loaditem].MatchColor[1]);
             SimpleEndianFlip16S(EcoParV1->en[loaditem].MatchColor[2],&EcoParV1->en[loaditem].MatchColor[2]);
         )
     } /* if version > 1.025 */
    else
     {
     fseek(fparam, loaditem * (sizeof (struct OldEcosystemV1)), 1);
     if ((fread((char *)&EcoParV1->en[loaditem], sizeof (struct OldEcosystemV1), 1, fparam)) != 1)
      {
         goto ReadError;
      }
     ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
             {
     struct OldEcosystemV1 *ptr=(struct OldEcosystemV1*) &EcoParV1->en[loaditem];

             SimpleEndianFlip16S(ptr->Line[0],&ptr->Line[0]);
             SimpleEndianFlip16S(ptr->Line[1],&ptr->Line[1]);
             SimpleEndianFlip16S(ptr->Skew[0],&ptr->Skew[0]);
             SimpleEndianFlip16S(ptr->Skew[1],&ptr->Skew[1]);
             SimpleEndianFlip16S(ptr->SkewAz[0],&ptr->SkewAz[0]);
             SimpleEndianFlip16S(ptr->SkewAz[1],&ptr->SkewAz[1]);
             SimpleEndianFlip16S(ptr->RelEl[0],&ptr->RelEl[0]);
             SimpleEndianFlip16S(ptr->RelEl[1],&ptr->RelEl[1]);
             SimpleEndianFlip16S(ptr->MaxRelEl[0],&ptr->MaxRelEl[0]);
             SimpleEndianFlip16S(ptr->MaxRelEl[1],&ptr->MaxRelEl[1]);
             SimpleEndianFlip16S(ptr->MinRelEl[0],&ptr->MinRelEl[0]);
             SimpleEndianFlip16S(ptr->MinRelEl[1],&ptr->MinRelEl[1]);
             SimpleEndianFlip16S(ptr->MaxSlope[0],&ptr->MaxSlope[0]);
             SimpleEndianFlip16S(ptr->MaxSlope[1],&ptr->MaxSlope[1]);
             SimpleEndianFlip16S(ptr->MinSlope[0],&ptr->MinSlope[0]);
             SimpleEndianFlip16S(ptr->MinSlope[1],&ptr->MinSlope[1]);
             SimpleEndianFlip16S(ptr->Type,&ptr->Type);
             SimpleEndianFlip16S(ptr->Tree[1],&ptr->Tree[1]);
             SimpleEndianFlip16S(ptr->Tree[0],&ptr->Tree[0]);
             SimpleEndianFlip16S(ptr->Color,&ptr->Color);
             SimpleEndianFlip16S(ptr->UnderEco,&ptr->UnderEco);
             SimpleEndianFlip16S(ptr->MatchColor[0],&ptr->MatchColor[0]);
             SimpleEndianFlip16S(ptr->MatchColor[1],&ptr->MatchColor[1]);
             SimpleEndianFlip16S(ptr->MatchColor[2],&ptr->MatchColor[2]);
             SimpleEndianFlip16S(ptr->Frame[0],&ptr->Frame[0]);
             SimpleEndianFlip16S(ptr->Frame[1],&ptr->Frame[1]);
             SimpleEndianFlip16S(ptr->Ease[0],&ptr->Ease[0]);
             SimpleEndianFlip16S(ptr->Ease[1],&ptr->Ease[1]);
            };
     )
     strcpy(EcoParV1->en[k].Model, "\0");
     } /* else version < 1.025 */
    } /* if */
   else if (loaditem >= 12)
    {
    short found = 0;

    if (fileversion > 1.025)
     {
     fseek(fparam, 2 * (sizeof (struct EcosystemV1)), 1);
     for (k=2; k<ECOPARAMSV1; k++)
      {
      if ((fread((char *)&TempEco, sizeof (struct EcosystemV1), 1, fparam)) != 1)
      {
          goto ReadError;
      }
      ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.DeTempEcocorrection for i386-loaditemros */
      SimpleEndianFlip16S(TempEco.Line[0],&TempEco.Line[0]);
      SimpleEndianFlip16S(TempEco.Line[1],&TempEco.Line[1]);
      SimpleEndianFlip16S(TempEco.Skew[0],&TempEco.Skew[0]);
      SimpleEndianFlip16S(TempEco.Skew[1],&TempEco.Skew[1]);
      SimpleEndianFlip16S(TempEco.SkewAz[0],&TempEco.SkewAz[0]);
      SimpleEndianFlip16S(TempEco.SkewAz[1],&TempEco.SkewAz[1]);
      SimpleEndianFlip16S(TempEco.RelEl[0],&TempEco.RelEl[0]);
      SimpleEndianFlip16S(TempEco.RelEl[1],&TempEco.RelEl[1]);
      SimpleEndianFlip16S(TempEco.MaxRelEl[0],&TempEco.MaxRelEl[0]);
      SimpleEndianFlip16S(TempEco.MaxRelEl[1],&TempEco.MaxRelEl[1]);
      SimpleEndianFlip16S(TempEco.MinRelEl[0],&TempEco.MinRelEl[0]);
      SimpleEndianFlip16S(TempEco.MinRelEl[1],&TempEco.MinRelEl[1]);
      SimpleEndianFlip16S(TempEco.MaxSlope[0],&TempEco.MaxSlope[0]);
      SimpleEndianFlip16S(TempEco.MaxSlope[1],&TempEco.MaxSlope[1]);
      SimpleEndianFlip16S(TempEco.MinSlope[0],&TempEco.MinSlope[0]);
      SimpleEndianFlip16S(TempEco.MinSlope[1],&TempEco.MinSlope[1]);
      SimpleEndianFlip16S(TempEco.Type,&TempEco.Type);
      SimpleEndianFlip16S(TempEco.Tree[0],&TempEco.Tree[0]);
      SimpleEndianFlip16S(TempEco.Tree[1],&TempEco.Tree[1]);
      SimpleEndianFlip16S(TempEco.Color,&TempEco.Color);
      SimpleEndianFlip16S(TempEco.UnderEco,&TempEco.UnderEco);
      SimpleEndianFlip16S(TempEco.MatchColor[0],&TempEco.MatchColor[0]);
      SimpleEndianFlip16S(TempEco.MatchColor[1],&TempEco.MatchColor[1]);
      SimpleEndianFlip16S(TempEco.MatchColor[2],&TempEco.MatchColor[2]);
      )
      if (! strcmp(TempEco.Name, PAR_NAME_ECO(loaditem)))
       {
       memcpy(&EcoParV1->en[loaditem - 10], &TempEco, sizeof (struct EcosystemV1));
       found = 1;
       break;
       } /* if */
      } /* for */
     } /* if version > 1.025 */
    else
     {
     fseek(fparam, 2 * (sizeof (struct OldEcosystemV1)), 1);
     for (k=2; k<ECOPARAMSV1; k++)
      {
      if ((fread((char *)&TempEco, sizeof (struct OldEcosystemV1), 1, fparam)) != 1)
       {
          goto ReadError;
       }
      ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
              {
      struct OldEcosystemV1 *ptr=(struct OldEcosystemV1*) &TempEco;

              SimpleEndianFlip16S(ptr->Line[0],&ptr->Line[0]);
              SimpleEndianFlip16S(ptr->Line[1],&ptr->Line[1]);
              SimpleEndianFlip16S(ptr->Skew[0],&ptr->Skew[0]);
              SimpleEndianFlip16S(ptr->Skew[1],&ptr->Skew[1]);
              SimpleEndianFlip16S(ptr->SkewAz[0],&ptr->SkewAz[0]);
              SimpleEndianFlip16S(ptr->SkewAz[1],&ptr->SkewAz[1]);
              SimpleEndianFlip16S(ptr->RelEl[0],&ptr->RelEl[0]);
              SimpleEndianFlip16S(ptr->RelEl[1],&ptr->RelEl[1]);
              SimpleEndianFlip16S(ptr->MaxRelEl[0],&ptr->MaxRelEl[0]);
              SimpleEndianFlip16S(ptr->MaxRelEl[1],&ptr->MaxRelEl[1]);
              SimpleEndianFlip16S(ptr->MinRelEl[0],&ptr->MinRelEl[0]);
              SimpleEndianFlip16S(ptr->MinRelEl[1],&ptr->MinRelEl[1]);
              SimpleEndianFlip16S(ptr->MaxSlope[0],&ptr->MaxSlope[0]);
              SimpleEndianFlip16S(ptr->MaxSlope[1],&ptr->MaxSlope[1]);
              SimpleEndianFlip16S(ptr->MinSlope[0],&ptr->MinSlope[0]);
              SimpleEndianFlip16S(ptr->MinSlope[1],&ptr->MinSlope[1]);
              SimpleEndianFlip16S(ptr->Type,&ptr->Type);
              SimpleEndianFlip16S(ptr->Tree[1],&ptr->Tree[1]);
              SimpleEndianFlip16S(ptr->Tree[0],&ptr->Tree[0]);
              SimpleEndianFlip16S(ptr->Color,&ptr->Color);
              SimpleEndianFlip16S(ptr->UnderEco,&ptr->UnderEco);
              SimpleEndianFlip16S(ptr->MatchColor[0],&ptr->MatchColor[0]);
              SimpleEndianFlip16S(ptr->MatchColor[1],&ptr->MatchColor[1]);
              SimpleEndianFlip16S(ptr->MatchColor[2],&ptr->MatchColor[2]);
              SimpleEndianFlip16S(ptr->Frame[0],&ptr->Frame[0]);
              SimpleEndianFlip16S(ptr->Frame[1],&ptr->Frame[1]);
              SimpleEndianFlip16S(ptr->Ease[0],&ptr->Ease[0]);
              SimpleEndianFlip16S(ptr->Ease[1],&ptr->Ease[1]);
             };
      )
      if (! strcmp(TempEco.Name, PAR_NAME_ECO(loaditem)))
       {
       memcpy(&EcoParV1->en[loaditem - 10], &TempEco, sizeof (struct OldEcosystemV1));
       strcpy(EcoParV1->en[loaditem].Model, "\0");
       found = 1;
       break;
       } /* if */
      } /* for */
     } /* else version < 1.025 */
    if (! found)
     {
     sprintf(str, "Ecosystem item %s not found in this file!\nOperation terminated.", PAR_NAME_ECO(loaditem));
     User_Message((CONST_STRPTR)"Ecosystem Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     goto ReadError;
     } /* if not found */ 
    } /* else */
   else
    {
    sprintf(str, "Ecosystem item %s not found in this file!\nOperation terminated.", PAR_NAME_ECO(loaditem));
    User_Message((CONST_STRPTR)"Ecosystem Editor: Load Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    goto ReadError;
    }
   if (fileversion > 1.025)
    fseek(fparam, sizeof (union EnvironmentV1) + sizeof (struct PaletteV1)
	 + sizeof (struct AnimationV1) + sizeof TempHdrV1, 0);
   else
    fseek(fparam, ECOPARAMSV1 * sizeof (struct OldEcosystemV1) +
	 sizeof (union EnvironmentV1) + sizeof (struct PaletteV1)
	 + sizeof (struct AnimationV1) + sizeof TempHdrV1, 0);
   } /* else */
  } /* if */
 else
  {
  if (fileversion > 1.025)
   fseek(fparam, sizeof (union EnvironmentV1), 1);
  else
   fseek(fparam, ECOPARAMSV1 * sizeof (struct OldEcosystemV1), 1);
  } /* no load ecosystems */

 if (loadcode & 0x1000)
  {
  if ((fread((char *)settingsV1, sizeof (struct SettingsV1), 1, fparam)) != 1)
   {
      goto ReadError;
   }
  ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
          SimpleEndianFlip16S(settingsV1->startframe,&settingsV1->startframe);
          SimpleEndianFlip16S(settingsV1->maxframes,&settingsV1->maxframes);
          SimpleEndianFlip16S(settingsV1->startseg,&settingsV1->startseg);
          SimpleEndianFlip16S(settingsV1->smoothfaces,&settingsV1->smoothfaces);
          SimpleEndianFlip16S(settingsV1->bankturn,&settingsV1->bankturn);
          SimpleEndianFlip16S(settingsV1->colrmap,&settingsV1->colrmap);
          SimpleEndianFlip16S(settingsV1->borderandom,&settingsV1->borderandom);
          SimpleEndianFlip16S(settingsV1->cmaptrees,&settingsV1->cmaptrees);
          SimpleEndianFlip16S(settingsV1->rendertrees,&settingsV1->rendertrees);
          SimpleEndianFlip16S(settingsV1->statistics,&settingsV1->statistics);
          SimpleEndianFlip16S(settingsV1->stepframes,&settingsV1->stepframes);
          SimpleEndianFlip16S(settingsV1->zbufalias,&settingsV1->zbufalias);
          SimpleEndianFlip16S(settingsV1->horfix,&settingsV1->horfix);
          SimpleEndianFlip16S(settingsV1->horizonmax,&settingsV1->horizonmax);
          SimpleEndianFlip16S(settingsV1->clouds,&settingsV1->clouds);
          SimpleEndianFlip16S(settingsV1->linefade,&settingsV1->linefade);
          SimpleEndianFlip16S(settingsV1->drawgrid,&settingsV1->drawgrid);
          SimpleEndianFlip16S(settingsV1->gridsize,&settingsV1->gridsize);
          SimpleEndianFlip16S(settingsV1->alternateq,&settingsV1->alternateq);
          SimpleEndianFlip16S(settingsV1->linetoscreen,&settingsV1->linetoscreen);
          SimpleEndianFlip16S(settingsV1->mapassfc,&settingsV1->mapassfc);
          SimpleEndianFlip16S(settingsV1->cmapluminous,&settingsV1->cmapluminous);
          SimpleEndianFlip16S(settingsV1->surfel[0],&settingsV1->surfel[0]);
          SimpleEndianFlip16S(settingsV1->surfel[1],&settingsV1->surfel[1]);
          SimpleEndianFlip16S(settingsV1->surfel[2],&settingsV1->surfel[2]);
          SimpleEndianFlip16S(settingsV1->surfel[3],&settingsV1->surfel[3]);
          SimpleEndianFlip16S(settingsV1->worldmap,&settingsV1->worldmap);
          SimpleEndianFlip16S(settingsV1->flatteneco,&settingsV1->flatteneco);
          SimpleEndianFlip16S(settingsV1->fixfract,&settingsV1->fixfract);
          SimpleEndianFlip16S(settingsV1->vecsegs,&settingsV1->vecsegs);
          SimpleEndianFlip16S(settingsV1->reliefshade,&settingsV1->reliefshade);
          SimpleEndianFlip16S(settingsV1->renderopts,&settingsV1->renderopts);
          SimpleEndianFlip16S(settingsV1->scrnwidth,&settingsV1->scrnwidth);
          SimpleEndianFlip16S(settingsV1->scrnheight,&settingsV1->scrnheight);
          SimpleEndianFlip16S(settingsV1->rendersegs,&settingsV1->rendersegs);
          SimpleEndianFlip16S(settingsV1->overscan,&settingsV1->overscan);
          SimpleEndianFlip16S(settingsV1->lookahead,&settingsV1->lookahead);
          SimpleEndianFlip16S(settingsV1->composite,&settingsV1->composite);
          SimpleEndianFlip16S(settingsV1->defaulteco,&settingsV1->defaulteco);
          SimpleEndianFlip16S(settingsV1->ecomatch,&settingsV1->ecomatch);
          SimpleEndianFlip16S(settingsV1->Yoffset,&settingsV1->Yoffset);
          SimpleEndianFlip16S(settingsV1->saveIFF,&settingsV1->saveIFF);
          SimpleEndianFlip16S(settingsV1->background,&settingsV1->background);
          SimpleEndianFlip16S(settingsV1->zbuffer,&settingsV1->zbuffer);
          SimpleEndianFlip16S(settingsV1->antialias,&settingsV1->antialias);
          SimpleEndianFlip16S(settingsV1->scaleimage,&settingsV1->scaleimage);
          SimpleEndianFlip16S(settingsV1->fractal,&settingsV1->fractal);
          SimpleEndianFlip16S(settingsV1->aliasfactor,&settingsV1->aliasfactor);
          SimpleEndianFlip16S(settingsV1->scalewidth,&settingsV1->scalewidth);
          SimpleEndianFlip16S(settingsV1->scaleheight,&settingsV1->scaleheight);
          SimpleEndianFlip64(settingsV1->zalias,&settingsV1->zalias);
          SimpleEndianFlip64(settingsV1->bankfactor,&settingsV1->bankfactor);
          SimpleEndianFlip64(settingsV1->skyalias,&settingsV1->skyalias);
          SimpleEndianFlip64(settingsV1->lineoffset,&settingsV1->lineoffset);
          SimpleEndianFlip64(settingsV1->altqlat,&settingsV1->altqlat);
          SimpleEndianFlip64(settingsV1->altqlon,&settingsV1->altqlon);
          SimpleEndianFlip64(settingsV1->treefactor,&settingsV1->treefactor);
          SimpleEndianFlip64(settingsV1->displacement,&settingsV1->displacement);
          SimpleEndianFlip64(settingsV1->unused3,&settingsV1->unused3);
          SimpleEndianFlip64(settingsV1->picaspect,&settingsV1->picaspect);
          SimpleEndianFlip64(settingsV1->zenith,&settingsV1->zenith);
          SimpleEndianFlip16S(settingsV1->exportzbuf,&settingsV1->exportzbuf);
          SimpleEndianFlip16S(settingsV1->zformat,&settingsV1->zformat);
          SimpleEndianFlip16S(settingsV1->fieldrender,&settingsV1->fieldrender);
          SimpleEndianFlip16S(settingsV1->lookaheadframes,&settingsV1->lookaheadframes);
          SimpleEndianFlip16S(settingsV1->velocitydistr,&settingsV1->velocitydistr);
          SimpleEndianFlip16S(settingsV1->easein,&settingsV1->easein);
          SimpleEndianFlip16S(settingsV1->easeout,&settingsV1->easeout);
          SimpleEndianFlip16S(settingsV1->displace,&settingsV1->displace);
          SimpleEndianFlip16S(settingsV1->mastercmap,&settingsV1->mastercmap);
          SimpleEndianFlip16S(settingsV1->cmaporientation,&settingsV1->cmaporientation);
          SimpleEndianFlip16S(settingsV1->fielddominance,&settingsV1->fielddominance);
          SimpleEndianFlip16S(settingsV1->fractalmap,&settingsV1->fractalmap);
          SimpleEndianFlip16S(settingsV1->perturb,&settingsV1->perturb);
          SimpleEndianFlip16S(settingsV1->realclouds,&settingsV1->realclouds);
          SimpleEndianFlip16S(settingsV1->reflections,&settingsV1->reflections);
          SimpleEndianFlip16S(settingsV1->waves,&settingsV1->waves);
          SimpleEndianFlip64(settingsV1->dispslopefact,&settingsV1->dispslopefact);
          SimpleEndianFlip64(settingsV1->globecograd,&settingsV1->globecograd);
          SimpleEndianFlip64(settingsV1->globsnowgrad,&settingsV1->globsnowgrad);
          SimpleEndianFlip64(settingsV1->globreflat,&settingsV1->globreflat);

  )
  }
 else
  fseek(fparam, sizeof (struct SettingsV1), 1);

 if (TempHdrV1.Version > 1.015)
  {
  if (LoadKeys)
   {
   if (KeyFrames > 0)
    {
    if (TempHdrV1.Version > 1.045)
     {
     if ((fread((char *)KFV1, KeyFrames * sizeof (union KeyFrameV1), 1, fparam)) 	!= 1)
     {
      goto ReadError;
     }
     ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
    		 for(unsigned int i=0;i<KeyFrames;i++)
    		 {
    			 SimpleEndianFlip16S(KFV1[i].MoKey.KeyFrame,&KFV1[i].MoKey.KeyFrame);
    			 SimpleEndianFlip16S(KFV1[i].MoKey.Group,   &KFV1[i].MoKey.Group);
    			 SimpleEndianFlip16S(KFV1[i].MoKey.Item,    &KFV1[i].MoKey.Item);
    			 SimpleEndianFlip32F(KFV1[i].MoKey.TCB[0],  &KFV1[i].MoKey.TCB[0]);
    			 SimpleEndianFlip32F(KFV1[i].MoKey.TCB[1],  &KFV1[i].MoKey.TCB[1]);
    			 SimpleEndianFlip32F(KFV1[i].MoKey.TCB[2],  &KFV1[i].MoKey.TCB[2]);
    			 SimpleEndianFlip16S(KFV1[i].MoKey.Linear,  &KFV1[i].MoKey.Linear);
    			 SimpleEndianFlip64 (KFV1[i].MoKey.Value,   &KFV1[i].MoKey.Value);
    		 }
         )
     }
    else
     {
     short i;

     for (k=0; k<KeyFrames; k++)
      {
      if ((fread((char *)&TempKF, sizeof (union NoLinearKeyFrame), 1, fparam)) != 1)
      {
          break;
      }
      ENDIAN_CHANGE_IF_NEEDED( /* AF: 16.Dec.2022, Endian correction for i386-loaditemros */
              SimpleEndianFlip16S(TempKF.MoKey.KeyFrame,&TempKF.MoKey.KeyFrame);
              SimpleEndianFlip16S(TempKF.MoKey.Group,&TempKF.MoKey.Group);
              SimpleEndianFlip16S(TempKF.MoKey.Item,&TempKF.MoKey.Item);
              SimpleEndianFlip32F(TempKF.MoKey.TCB[0],&TempKF.MoKey.TCB[0]);
              SimpleEndianFlip32F(TempKF.MoKey.TCB[1],&TempKF.MoKey.TCB[1]);
              SimpleEndianFlip32F(TempKF.MoKey.TCB[2],&TempKF.MoKey.TCB[2]);
              SimpleEndianFlip64(TempKF.MoKey.Value,&TempKF.MoKey.Value);
          )
      KFV1[k].MoKey.KeyFrame = TempKF.MoKey.KeyFrame;
      KFV1[k].MoKey.Group    = TempKF.MoKey.Group;
      KFV1[k].MoKey.Item     = TempKF.MoKey.Item;
      KFV1[k].MoKey.TCB[0]   = TempKF.MoKey.TCB[0];
      KFV1[k].MoKey.TCB[1]   = TempKF.MoKey.TCB[1];
      KFV1[k].MoKey.TCB[2]   = TempKF.MoKey.TCB[2];
      KFV1[k].MoKey.Linear   = 0;
      switch (TempKF.MoKey.Group)
       {
       case 0:
        {
        KFV1[k].MoKey.Value = TempKF.MoKey.Value;
        break;
        } /* Motion */
       case 1:
        {
        KFV1[k].CoKey.Value[0] = TempKF.CoKey.Value[0];
        KFV1[k].CoKey.Value[1] = TempKF.CoKey.Value[1];
        KFV1[k].CoKey.Value[2] = TempKF.CoKey.Value[2];
        break;
        } /* Color */
       case 2:
        {
        for (i=0; i<8; i++)
         KFV1[k].EcoKey2.Value[i] = TempKF.EcoKey2.Value[i];
        break;
        } /* Ecosystem */
       } /* switch */
      } /* for k=0... */
     if (k < KeyFrames)
      goto ReadError;
     } /* else old version key frames */
    TempHdrV1.KeyFrames = KeyFrames;
    } /* if key frames to read */
   else
    TempHdrV1.KeyFrames = 0;
   } /* if would like to read key frames */
  else
   TempHdrV1.KeyFrames = ExistingKeyFrames;
  } /* if version supports key frames */
 else
  {
  TempHdrV1.KeyFrames = 0;
  } /* else old version, set key frames to zero */

//EndLoad:
 fclose(fparam);
 switch (loadcode)
  {
  case 0x0001:
   {
   sprintf(str, "%s motion, Ver %f", paramfile, TempHdrV1.Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* motion only */
  case 0x0010:
   {
   sprintf(str, "%s colors, Ver %f", paramfile, TempHdrV1.Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* colors only */
  case 0x0100:
   {
   sprintf(str, "%s ecosystems, Ver %f", paramfile, TempHdrV1.Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* ecosystems only */
  case 0x1000:
   {
   sprintf(str, "%s settings, Ver %f", paramfile, TempHdrV1.Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* settings only */
  case 0x1111:
   {
   sprintf(str, "%s all, Ver %f", paramfile, TempHdrV1.Version);
   Log(MSG_PAR_LOAD, (CONST_STRPTR)str);
   break;
   } /* all parameters */
  } /* switch loadcode */

 if (TempHdrV1.Version < 1.045)
  {
  if (loadcode & 0x1000 && TempHdrV1.Version < 1.035)
   settingsV1->rendertrees = 1;
  } /* if old format */

 Par_Mod &= (loadcode ^ 0x1111);

/* convert format to current version */

 ParamFmtV1V2_Convert(MoParV1, CoParV1, EcoParV1, settingsV1, &TempHdrV1,
	KFV1, loadcode, loaditem, LoadKeys);
 TempHdr->KeyFrames = TempHdrV1.KeyFrames;
 strcpy(TempHdr->FType, "%WCSPAR");
 TempHdr->Version = PAR_CURRENT_VERSION;
 TempHdr->ByteOrder = 0xaabbccdd;

 if (settingsV1)
  free_Memory(settingsV1, sizeof (struct SettingsV1));
 if (MoParV1)
  free_Memory(MoParV1, sizeof (struct AnimationV1));
 if (CoParV1)
  free_Memory(CoParV1, sizeof (struct PaletteV1));
 if (EcoParV1)
  free_Memory(EcoParV1, sizeof (union EnvironmentV1));
 if (KFV1)
  free_Memory(KFV1, KFV1size);

 return (1);

ReadError:
 Log(ERR_READ_FAIL, (CONST_STRPTR)paramfile);
 fclose(fparam);
 if (settingsV1)
  free_Memory(settingsV1, sizeof (struct SettingsV1));
 if (MoParV1)
  free_Memory(settingsV1, sizeof (struct AnimationV1));
 if (CoParV1)
  free_Memory(settingsV1, sizeof (struct PaletteV1));
 if (EcoParV1)
  free_Memory(settingsV1, sizeof (union EnvironmentV1));
 if (KFV1)
  free_Memory(KFV1, KFV1size);
 if (! KF)
  KFsize = 0;

 return (-1);

} /* loadparamsV1() */

/************************************************************************/

STATIC_FCN void ParamFmtV1V2_Convert(struct AnimationV1 *MoParV1, struct PaletteV1 *CoParV1,
	union EnvironmentV1 *EcoParV1, struct SettingsV1 *settingsV1,
	struct ParHeaderV1 *ParHdrV1, union KeyFrameV1 *KFV1, USHORT loadcode,
	short loaditem, short LoadKeys) // used locally only -> static, AF 23.7.2021
{
short i, j, k;

 if (MoParV1 && (loadcode & 0x0001))
  {
  for (i=0; i<MOTIONPARAMSV1; i++)
   {
   if (loaditem < 0 || loaditem == i)
    MoPar.mn[i].Value = MoParV1->mn[i].Value[0];
   } /* for i=0... */
  if (loaditem < 0)
   {
   MoPar.mn[26].Value = 0.0;
   MoPar.mn[27].Value = 0.0;
   MoPar.mn[28].Value = 0.0;
   MoPar.mn[29].Value = 0.0;
   MoPar.mn[30].Value = 0.0;
   MoPar.mn[31].Value = 0.0;
   MoPar.mn[32].Value = 0.0;
   } /* if load all motion */
  } /* if motion */

 if (CoParV1 && (loadcode & 0x0010))
  {
  if (loaditem < 0)
   {
   memset(&CoPar, 0, sizeof (struct Palette));
   for (i=0; i<COLORPARAMS; i++)
    strcpy(CoPar.cn[i].Name, "Unused");
   } /* if load all colors */

  k = 0;
  for (i=0; i<COLORPARAMSV1; i++)
   {
   if (i == 12)
    k = 12;
   if (loaditem < 0 || loaditem == i + k)
    {
    strcpy(CoPar.cn[i + k].Name, CoParV1->cn[i].Name);
    for (j=0; j<3; j++)
     {
     CoPar.cn[i + k].Value[j] = CoParV1->cn[i].Value[j];
     }
    } /* if */
   } /* for i=0... */

  if (loaditem < 0)
   {
   strcpy(CoPar.cn[12].Name, "Water Foam");
   CoPar.cn[12].Value[0] = 245;
   CoPar.cn[12].Value[1] = 250;
   CoPar.cn[12].Value[2] = 255;
   strcpy(CoPar.cn[13].Name, "Beach Sand");
   CoPar.cn[13].Value[0] = 206;
   CoPar.cn[13].Value[1] = 196;
   CoPar.cn[13].Value[2] = 163;
   strcpy(CoPar.cn[14].Name, "Beach Rock");
   CoPar.cn[14].Value[0] = 160;
   CoPar.cn[14].Value[1] = 157;
   CoPar.cn[14].Value[2] = 154;
   strcpy(CoPar.cn[15].Name, "Strata 1");
   CoPar.cn[15].Value[0] = 199;
   CoPar.cn[15].Value[1] = 187;
   CoPar.cn[15].Value[2] = 183;
   strcpy(CoPar.cn[16].Name, "Strata 2");
   CoPar.cn[16].Value[0] = 207;
   CoPar.cn[16].Value[1] = 167;
   CoPar.cn[16].Value[2] = 155;
   strcpy(CoPar.cn[17].Name, "Strata 3");
   CoPar.cn[17].Value[0] = 207;
   CoPar.cn[17].Value[1] = 187;
   CoPar.cn[17].Value[2] = 159;
   strcpy(CoPar.cn[18].Name, "Strata 4");
   CoPar.cn[18].Value[0] = 179;
   CoPar.cn[18].Value[1] = 183;
   CoPar.cn[18].Value[2] = 195;
   strcpy(CoPar.cn[19].Name, "Sun");
   CoPar.cn[19].Value[0] = 255;
   CoPar.cn[19].Value[1] = 255;
   CoPar.cn[19].Value[2] = 255;
   strcpy(CoPar.cn[20].Name, "Moon");
   CoPar.cn[20].Value[0] = 255;
   CoPar.cn[20].Value[1] = 250;
   CoPar.cn[20].Value[2] = 240;
   strcpy(CoPar.cn[21].Name, "Clouds");
   CoPar.cn[21].Value[0] = 255;
   CoPar.cn[21].Value[1] = 255;
   CoPar.cn[21].Value[2] = 255;
   } /* if load all ecosystems */
  } /* if colors */

 if (EcoParV1 && (loadcode & 0x0100))
  {
  k = 0;
  if (loaditem < 0)
   {
   for (i=0; i<ECOPARAMS; i++)
    setecodefault(i);
   } /* if load all ecosystems */

  for (i=0; i<ECOPARAMSV1; i++)
   {
   if (i == 2)
    k = 10;
   if (loaditem < 0 || loaditem == i + k)
    {
    strcpy(EcoPar.en[i + k].Name, EcoParV1->en[i].Name);
    strcpy(EcoPar.en[i + k].Model, EcoParV1->en[i].Model);
    EcoPar.en[i + k].Line = EcoParV1->en[i].Line[0];
    EcoPar.en[i + k].Skew = EcoParV1->en[i].Skew[0];
    EcoPar.en[i + k].SkewAz = EcoParV1->en[i].SkewAz[0];
    EcoPar.en[i + k].RelEl = EcoParV1->en[i].RelEl[0];
    EcoPar.en[i + k].MaxRelEl = EcoParV1->en[i].MaxRelEl[0];
    EcoPar.en[i + k].MinRelEl = EcoParV1->en[i].MinRelEl[0];
    EcoPar.en[i + k].MaxSlope = EcoParV1->en[i].MaxSlope[0];
    EcoPar.en[i + k].MinSlope = EcoParV1->en[i].MinSlope[0];
    EcoPar.en[i + k].Density = EcoParV1->en[i].Tree[0];
    EcoPar.en[i + k].Height = EcoParV1->en[i].Tree[1];
    EcoPar.en[i + k].Type = EcoParV1->en[i].Type;
    EcoPar.en[i + k].Color = EcoParV1->en[i].Color;
    EcoPar.en[i + k].UnderEco = EcoParV1->en[i].UnderEco;
    EcoPar.en[i + k].MatchColor[0] = EcoParV1->en[i].MatchColor[0];
    EcoPar.en[i + k].MatchColor[1] = EcoParV1->en[i].MatchColor[1];
    EcoPar.en[i + k].MatchColor[2] = EcoParV1->en[i].MatchColor[2];
    if (EcoPar.en[i + k].Color >= 12)
     EcoPar.en[i + k].Color += 12;
    if (EcoPar.en[i + k].UnderEco >= 2)
     EcoPar.en[i + k].UnderEco += 10;
    } /* if */
   } /* for i=0... */
  if (loaditem < 0)
   {
   strcpy(EcoPar.en[2].Name, "Beach Sand");
   EcoPar.en[2].MaxSlope = 10.0;
   EcoPar.en[2].Color = 13;
   EcoPar.en[2].UnderEco = 2;
   strcpy(EcoPar.en[3].Name, "Beach Rock");
   EcoPar.en[3].Color = 14;
   EcoPar.en[3].UnderEco = 3;
   } /* if load all ecosystems */
  } /* if ecosystems */

 if (settingsV1 && (loadcode & 0x1000))
  {
  defaultsettings();
  settings.startframe 	= settingsV1->startframe;
  settings.maxframes 	= settingsV1->maxframes;
  settings.startseg 	= settingsV1->startseg;
/*  settings.smoothfaces 	= settingsV1->smoothfaces;*/
  settings.bankturn 	= settingsV1->bankturn;
  settings.colrmap 	= settingsV1->colrmap;
  settings.borderandom 	= settingsV1->borderandom;
  settings.cmaptrees 	= settingsV1->cmaptrees;
  settings.rendertrees 	= settingsV1->rendertrees;
  settings.statistics 	= 0;
  settings.stepframes 	= settingsV1->stepframes;
  settings.zbufalias 	= settingsV1->zbufalias;
  settings.horfix 	= settingsV1->horfix;
  settings.horizonmax 	= settingsV1->horizonmax;
  settings.clouds 	= settingsV1->clouds;
  settings.linefade 	= settingsV1->linefade;
  settings.drawgrid 	= settingsV1->drawgrid;
  settings.gridsize 	= settingsV1->gridsize;
  settings.alternateq 	= settingsV1->alternateq;
  settings.linetoscreen = settingsV1->linetoscreen;
  settings.mapassfc 	= settingsV1->mapassfc;
  settings.cmapluminous = settingsV1->cmapluminous;
  settings.surfel[0] 	= settingsV1->surfel[0];
  settings.surfel[1] 	= settingsV1->surfel[1];
  settings.surfel[2] 	= settingsV1->surfel[2];
  settings.surfel[3] 	= settingsV1->surfel[3];
  settings.worldmap 	= settingsV1->worldmap;
  settings.flatteneco 	= settingsV1->flatteneco;
  settings.fixfract 	= settingsV1->fixfract;
  settings.vecsegs 	= settingsV1->vecsegs;
  settings.reliefshade 	= settingsV1->reliefshade;
  settings.renderopts 	= settingsV1->renderopts;
  settings.scrnwidth 	= settingsV1->scrnwidth;
  settings.scrnheight 	= settingsV1->scrnheight;
  settings.rendersegs 	= settingsV1->rendersegs;
  settings.overscan 	= settingsV1->overscan;
  settings.lookahead 	= settingsV1->lookahead;
  settings.composite 	= settingsV1->composite;
  settings.defaulteco 	= settingsV1->defaulteco >= 2 ?
	settingsV1->defaulteco + 10: settingsV1->defaulteco;
  settings.ecomatch 	= settingsV1->ecomatch;
  settings.Yoffset 	= settingsV1->Yoffset;
  settings.saveIFF 	= settingsV1->saveIFF;
  settings.background 	= settingsV1->background;
  settings.zbuffer 	= settingsV1->zbuffer;
  settings.antialias 	= settingsV1->antialias;
  settings.scaleimage 	= settingsV1->scaleimage;
  settings.fractal 	= settingsV1->fractal;
  settings.aliasfactor 	= settingsV1->aliasfactor;
  settings.scalewidth 	= settingsV1->scalewidth;
  settings.scaleheight 	= settingsV1->scaleheight;
  settings.exportzbuf 	= settingsV1->exportzbuf;
  settings.zformat 	= settingsV1->zformat;
  settings.fieldrender 	= settingsV1->fieldrender;
  settings.lookaheadframes = settingsV1->lookaheadframes;
  settings.velocitydistr = settingsV1->velocitydistr;
  settings.easein 	= settingsV1->easein;
  settings.easeout 	= settingsV1->easeout;
/*  settings.displace 	= settingsV1->displace;*/
/*  settings.mastercmap 	= settingsV1->mastercmap;*/
/*  settings.cmaporientation = settingsV1->cmaporientation;*/
/*  settings.fielddominance = settingsV1->fielddominance;*/
/*  settings.fractalmap 	= settingsV1->fractalmap;*/
/*  settings.perturb 	= settingsV1->perturb;*/
/*  settings.realclouds 	= settingsV1->realclouds;*/
/*  settings.reflections 	= settingsV1->reflections;*/
/*  settings.waves 	= settingsV1->waves;*/
/*  settings.dispslopefact = settingsV1->dispslopefact;*/
  settings.globecograd 	= settingsV1->globecograd;
  settings.globsnowgrad = settingsV1->globsnowgrad;
  settings.globreflat 	= settingsV1->globreflat;
  settings.zalias 	= settingsV1->zalias;
  settings.bankfactor 	= settingsV1->bankfactor;
  settings.skyalias 	= settingsV1->skyalias;
  settings.lineoffset 	= settingsV1->lineoffset;
  settings.altqlat 	= settingsV1->altqlat;
  settings.altqlon 	= settingsV1->altqlon;
  settings.treefactor 	= settingsV1->treefactor;
/*  settings.displacement = settingsV1->displacement;*/
  settings.unused3 	= 0;
  settings.picaspect 	= settingsV1->picaspect;
  settings.zenith 	= settingsV1->zenith;
  } /* if settings */

 if (LoadKeys)
  {
  for (i=0; i<ParHdrV1->KeyFrames; i++)
   {
   if (KFV1[i].MoKey.Group == 1)
    {
    if (KFV1[i].CoKey.Item >= 12)
     KFV1[i].CoKey.Item += 12;
    }
   else if (KFV1[i].MoKey.Group == 2)
    {
    if (KFV1[i].EcoKey.Item >= 2)
     KFV1[i].EcoKey.Item += 10;
    }

   KF[i].MoKey.KeyFrame = KFV1[i].MoKey.KeyFrame;
   KF[i].MoKey.Group    = KFV1[i].MoKey.Group;
   KF[i].MoKey.Item     = KFV1[i].MoKey.Item;
   KF[i].MoKey.TCB[0]   = KFV1[i].MoKey.TCB[0];
   KF[i].MoKey.TCB[1]   = KFV1[i].MoKey.TCB[1];
   KF[i].MoKey.TCB[2]   = KFV1[i].MoKey.TCB[2];
   KF[i].MoKey.Linear   = KFV1[i].MoKey.Linear;
   switch (KFV1[i].MoKey.Group)
    {
    case 0:
     {
     KF[i].MoKey.Value = KFV1[i].MoKey.Value;
     break;
     } /* Motion */
    case 1:
     {
     KF[i].CoKey.Value[0] = KFV1[i].CoKey.Value[0];
     KF[i].CoKey.Value[1] = KFV1[i].CoKey.Value[1];
     KF[i].CoKey.Value[2] = KFV1[i].CoKey.Value[2];
     break;
     } /* Color */
    case 2:
     {
     for (j=0; j<8; j++)
      KF[i].EcoKey2.Value[j] = KFV1[i].EcoKey2.Value[j];
     KF[i].EcoKey2.Value[8] = PAR_FIRSTDN_ECO(KFV1[i].EcoKey.Item);
     KF[i].EcoKey2.Value[9] = PAR_FIRSTHT_ECO(KFV1[i].EcoKey.Item);
     break;
     } /* Ecosystem */
    } /* switch */
   ParHdrV1->CurrentKey = 0;
   } /* for i=0... */
  } /* convert key frame item numbers and par header */
 
} /* ParamFmtV1V2_Convert() */

/**********************************************************************/

short saveparams(USHORT savecode, short saveitem, short savecur)
{
 short k, error = 0;
 char extension[32], TagItem[8];
 char temppath[255], tempfile[64], filename[255], Ptrn[32];
 float fileversion;
 FILE *fparam;
 struct Color TempCo;
 struct Ecosystem TempEco;
 struct ParHeader TempHdr;

 strcpy(temppath, parampath);
 strcpy(tempfile, paramfile);

SaveRepeat:
 if (! savecur)
  {
  strcpy(Ptrn, "#?.par");
  if (! getfilenameptrn(1, "Save Parameter File", temppath, tempfile, Ptrn))
   {
   return (1);
   } /* if aborted */
  } /* if not use current name */
 
 stcgfe(extension, tempfile);			/* lattice function */
 sprintf(str, ".%s", extension);
 if (strcmp(str, ".par")) strcat(tempfile, ".par");
 strmfp(filename, temppath, tempfile);

 if (savecode == 0x1111)
  {
  if ((fparam = fopen(filename, "wb")) == NULL)
   {
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)paramfile);
   if (User_Message((CONST_STRPTR)"paramfile",
           (CONST_STRPTR)"Error opening file for output!\nTry again?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
    goto SaveRepeat;
   return 1;
   } /* if open fail */
  if ((fwrite((char *)&ParHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
   goto SaveError;
  } /* if save entire file */
 else
  {
  if ((fparam = fopen(filename, "rb+")) == NULL)
   {
   savecode = 0x1111;
   if ((fparam = fopen(filename, "wb")) == NULL)
    {
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)paramfile);
    if (User_Message((CONST_STRPTR)"paramfile",
            (CONST_STRPTR)"Error opening file for output!\nTry again?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
     goto SaveRepeat;
    return 1;
    } /* if open fail */
   if ((fwrite((char *)&ParHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
    goto SaveError;
   } /* if open to append fail */
  else
   {
   if ((fread((char *)&TempHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
    {
    goto SaveError;
    }

/* the following do-nothing command is necessary to work around a compiler bug
   that won't let you write after a read without a seek inbetween */

   fseek(fparam, 0L, 1);

   if (! strncmp(TempHdr.FType, "%WCSPAR", 8))
    fileversion = TempHdr.Version;
   else fileversion = 0.0;
   if (fileversion < 2.0)
    {
    Log(ERR_WRONG_TYPE, (CONST_STRPTR)"Version < 2.0");
    if (! User_Message((CONST_STRPTR)"Parameter Editing Module",
            (CONST_STRPTR)"Partial files may not be written to old file versions!\n\
	Do you wish to save the entire parameter file?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
     goto SaveError;
    fclose(fparam);
    savecode = 0x1111;
    if ((fparam = fopen(filename, "wb")) == NULL)
     {
     Log(ERR_OPEN_FAIL, (CONST_STRPTR)paramfile);
     if (User_Message((CONST_STRPTR)"paramfile",
             (CONST_STRPTR)"Error opening file for output!\nTry again?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
      goto SaveRepeat;
     return 1;
     } /* if open fail */
    if ((fwrite((char *)&ParHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
     goto SaveError;
    } /*  if version = 0.0 */
   } /* else file opened to append */
  } /* else only save partial file */

 ParHdr.MotionParamsPos = ftell(fparam);
 if (savecode & 0x01)
  {
  if (saveitem < 0)
   {
   if ((fwrite((char *)&MoPar, sizeof MoPar, 1, fparam)) != 1)
    {
    goto SaveError;
    }
   } /* if */
  else
   {
   fseek(fparam, saveitem * (sizeof (struct Motion)), 1);
   if ((fwrite((char *)&MoPar.mn[saveitem], sizeof (struct Motion), 1, fparam))!=1)
    goto SaveError;
   goto EndSave;
   } /* else */
  } /* if */
 else
  fseek(fparam, sizeof MoPar, 1);


 ParHdr.ColorParamsPos = ftell(fparam);
 if (savecode & 0x10)
  {
  if (saveitem < 0)
   {
   if ((fwrite((char *)&CoPar, sizeof CoPar, 1, fparam)) != 1)
    goto SaveError;
   } /* if */
  else
   {
   if (saveitem < 24)
    {
    fseek(fparam, saveitem * (sizeof (struct Color)), 1);
    if ((fwrite((char *)&CoPar.cn[saveitem], sizeof (struct Color), 1, fparam))!=1)
     goto SaveError;
    } /* if */
   else
    {
    short found = 0;

    fseek(fparam, 24 * (sizeof (struct Color)), 1);
    for (k=24; k<COLORPARAMS; k++)
     {
     if ((fread((char *)&TempCo, sizeof (struct Color), 1, fparam)) != 1)
      goto SaveError;
     if (! strcmp(TempCo.Name, PAR_NAME_COLOR(saveitem)))
      {
      fseek(fparam, -(sizeof (struct Color)), 1);
      if ((fwrite((char *)&CoPar.cn[saveitem], sizeof (struct Color), 1, fparam))!=1)
       goto SaveError;
      found = 1;
      break;
      } /* if */
     } /* for */
    if (! found)
     {
     sprintf(str, "Color item %s not found in this file!\nOperation terminated.", PAR_NAME_COLOR(saveitem));
     User_Message((CONST_STRPTR)"Color Editor: Save Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     goto SaveError;
     } /* if not found */ 
    } /* else */
   goto EndSave;
   } /* else */
  } /* if */
 else fseek(fparam, sizeof CoPar, 1);

 ParHdr.EcoParamsPos = ftell(fparam);
 if (savecode & 0x100)
  {
  if (saveitem < 0)
   {
   if ((fwrite((char *)&EcoPar, sizeof EcoPar, 1, fparam))!=1)
    goto SaveError;
   } /* if */
  else
   {
   if (saveitem < 12)
    {
    fseek(fparam, saveitem * (sizeof (struct Ecosystem)), 1);
    if ((fwrite((char *)&EcoPar.en[saveitem], sizeof (struct Ecosystem), 1, fparam))!=1)
     goto SaveError;
    } /* if */
   else
    {
    short found = 0;

    fseek(fparam, 12 * (sizeof (struct Ecosystem)), 1);
    for (k=12; k<ECOPARAMS; k++)
     {
     if ((fread((char *)&TempEco, sizeof (struct Ecosystem), 1, fparam)) != 1)
      goto SaveError;
     if (! strcmp(TempEco.Name, PAR_NAME_ECO(saveitem)))
      {
      fseek(fparam, -(sizeof (struct Ecosystem)), 1);
      if ((fwrite((char *)&EcoPar.en[saveitem], sizeof (struct Ecosystem), 1, fparam))!=1)
       goto SaveError;
      found = 1;
      break;
      } /* if */
     } /* for k=2... */
    if (! found)
     {
     sprintf(str, "Ecosystem item %s not found in this file!\nOperation terminated.", PAR_NAME_ECO(saveitem));
     User_Message((CONST_STRPTR)"Ecosystem Editor: Save Current", (CONST_STRPTR)str, (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
     goto SaveError;
     } /* if not found */ 
    } /* else */
   goto EndSave;
   } /* else */
  } /* if */
 else fseek(fparam, sizeof EcoPar, 1);

 ParHdr.SettingsPos = ftell(fparam);
 if (savecode & 0x1000)
  {
  if ((fwrite((char *)&settings, sizeof settings, 1, fparam)) != 1)
   goto SaveError;
  } /* if */
 else fseek(fparam, sizeof settings, 1);

 ParHdr.KeyFramesPos = ftell(fparam);
 if (ParHdr.KeyFrames > 0)
  {
  if (savecode & 0x0111 && savecode != 0x1111)
   {
   if (User_Message((CONST_STRPTR)"Parameter Module: Save",
           (CONST_STRPTR)"Save all key frames as well?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
    {
    if ((fwrite((char *)KF, ParHdr.KeyFrames * sizeof (union KeyFrame), 1, fparam)) != 1)
     goto SaveError;
    } /* if save key frames */
   } /* if not save whole file and not just save settings */
  else if ((fwrite((char *)KF, ParHdr.KeyFrames * sizeof (union KeyFrame), 1, fparam)) 	!= 1)
   goto SaveError;
  } /* if key frames to save */

 if (savecode & 0x0100)
  {
  strcpy(TagItem, "ECOT"); 
  for (k=0; k<ECOPARAMS; k++)
   {
   if (EcoShift[k].Ecotype)
    {
    if ((fwrite((char *)TagItem, sizeof (long), 1, fparam)) != 1)
     goto SaveError;
    if ((fwrite((char *)&k, sizeof (short), 1, fparam)) != 1)
     goto SaveError;
    if ((Ecotype_Save(EcoShift[k].Ecotype, fparam)) == 0)
     goto SaveError;
    } /* if ecotype data exists */
   } /* for k=0... */
  } /* if saving ecosystems */
 
 fseek(fparam, 0L, 0);
 if ((fwrite((char *)&ParHdr, sizeof (struct ParHeader), 1, fparam)) != 1)
     goto SaveError;

EndSave:
 fclose(fparam);
 switch (savecode)
  {
  case 0x0001:
   {
   sprintf(str, "%s motion", tempfile);
   Log(MSG_PAR_SAVE, (CONST_STRPTR)str);
   break;
   } /* motion only */
  case 0x0010:
   {
   sprintf(str, "%s colors", tempfile);
   Log(MSG_PAR_SAVE, (CONST_STRPTR)str);
   break;
   } /* colors only */
  case 0x0100:
   {
   sprintf(str, "%s ecosystems", tempfile);
   Log(MSG_PAR_SAVE, (CONST_STRPTR)str);
   break;
   } /* ecosystems only */
  case 0x1000:
   {
   sprintf(str, "%s settings", tempfile);
   Log(MSG_PAR_SAVE, (CONST_STRPTR)str);
   break;
   } /* settings only */
  case 0x1111:
   {
   sprintf(str, "%s all", tempfile);
   Log(MSG_PAR_SAVE, (CONST_STRPTR)str);
   break;
   } /* all parameters */
  } /* switch savecode */
 if (savecode == 0x1111)
  {
  if (strcmp(temppath, parampath) || strcmp(tempfile, paramfile))
   Proj_Mod = 1;
  strcpy(parampath, temppath);
  strcpy(paramfile, tempfile);
  } /* if save all parameters */

 Par_Mod &= (savecode ^ 0x1111);

 return (error);

SaveError:
 fclose(fparam);
 Log(ERR_WRITE_FAIL, (CONST_STRPTR)paramfile);
 User_Message((CONST_STRPTR)"paramfile",
         (CONST_STRPTR)"Error writing to Parameter file!\n\
The output file has been modified and may no longer be valid.\
 Try resaving to a different device or freeing some disk space and saving\
 again.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");

 return (1);

} /* saveparams() */

/**********************************************************************/

