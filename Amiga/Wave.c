/* Wave.c
** Wave generation and manipulation functions for WCS.
** Written by Gary R. Huber, Jan 1995.
*/

#include "WCS.h"
#include "Wave.h"
#include "GenericParams.h"

/*********************************************************************/

struct Wave *Wave_New(void)
{

 return ((struct Wave *)
	get_Memory(sizeof (struct Wave), MEMF_CLEAR));

} /* Wave_New() */

/*********************************************************************/

void Wave_Del(struct Wave *WV)
{

 if (WV)
  free_Memory(WV, sizeof (struct Wave));

} /* Wave_Del() */

/*********************************************************************/

void Wave_DelAll(struct Wave *WV)
{
struct Wave *WVDel;

 while (WV)
  {
  WVDel = WV;
  WV = WV->Next;
  free_Memory(WVDel, sizeof (struct Wave));
  } /* while */

} /* Wave_Del() */

/*********************************************************************/

void Wave_Set(struct Wave *WV, ULONG Item, double Val)
{

 switch (Item)
  {
  case WCS_WAVE_AMP:
   {
   WV->Amp = Val;
   break;
   }
  case WCS_WAVE_LENGTH:
   {
   WV->Length = Val;
   break;
   }
  case WCS_WAVE_VELOCITY:
   {
   WV->Velocity = Val;
   break;
   }
  case WCS_WAVE_LAT:
   {
   WV->Pt.Lat = Val;
   break;
   }
  case WCS_WAVE_LON:
   {
   WV->Pt.Lon = Val;
   break;
   }
  } /* switch */

} /* Wave_Set() */

/************************************************************************/

double Wave_Get(struct Wave *WV, ULONG Item)
{

 switch (Item)
  {
  case WCS_WAVE_AMP:
   {
   return (WV->Amp);
   break;
   }
  case WCS_WAVE_LENGTH:
   {
   return (WV->Length);
   break;
   }
  case WCS_WAVE_VELOCITY:
   {
   return (WV->Velocity);
   break;
   }
  case WCS_WAVE_LAT:
   {
   return (WV->Pt.Lat);
   break;
   }
  case WCS_WAVE_LON:
   {
   return (WV->Pt.Lon);
   break;
   }
  default:
  {
      printf("Invalid Item %lu in %s %d",Item,__FILE__,__LINE__);
      return 0;
  }
  } /* switch */

} /* Wave_Get() */

/************************************************************************/

void Wave_SetDefaults(struct Wave *NewWave, struct Wave *PtrnWave)
{
short Focus, Speed, Direction;

 if (PtrnWave)
  {
  NewWave->Amp = PtrnWave->Amp * .8;
  NewWave->Length = PtrnWave->Length * .8;
  NewWave->Velocity = PtrnWave->Velocity * .8;
  NewWave->Pt.Lat = PtrnWave->Pt.Lat + GaussRand() * .5;
  NewWave->Pt.Lon = PtrnWave->Pt.Lon + GaussRand() * .5;
  } /* if a pattern exists */
 else
  {
  Focus = User_Message_Def((CONST_STRPTR)"Wave: Set Defaults",
          (CONST_STRPTR)"Select general wave center.", (CONST_STRPTR)"Focus Point|Camera Point", (CONST_STRPTR)"fc", 0);
  Speed = User_Message_Def((CONST_STRPTR)"Wave: Set Defaults",
          (CONST_STRPTR)"Select wave speed.", (CONST_STRPTR)"Fast|Very Fast|Slow", (CONST_STRPTR)"fvs", 1);
  Direction = User_Message_Def((CONST_STRPTR)"Wave: Set Defaults",
          (CONST_STRPTR)"Select wave direction.", (CONST_STRPTR)"Spreading|Converging", (CONST_STRPTR)"sc", 1) * 2 - 1;
  NewWave->Amp = 2.0;
  NewWave->Length = .1;
  NewWave->Velocity = Direction * (5.0 + Speed * 100);
  NewWave->Pt.Lat = PAR_FIRST_MOTION(1 + Focus * 3);
  NewWave->Pt.Lon = PAR_FIRST_MOTION(2 + Focus * 3);
  } /* else */

} /* Wave_SetDefaults() */

/************************************************************************/

struct WaveData *WaveData_New(void)
{

 return ((struct WaveData *)
	get_Memory(sizeof (struct WaveData), MEMF_CLEAR));

} /* WaveData_New() */

/*********************************************************************/

void WaveData_Del(struct WaveData *WD)
{

 if (WD)
  {
  if (WD->Wave)
   Wave_DelAll(WD->Wave);
  if (WD->KT)
   FreeGenericKeyTable(&WD->KT, &WD->KT_MaxFrames);
  if (WD->WaveKey)
   free_Memory(WD->WaveKey, WD->KFSize);

  free_Memory(WD, sizeof (struct WaveData));
  } /* if */

} /* WaveData_Del() */

/*********************************************************************/

void WaveData_SetDouble(struct WaveData *WD, ULONG Item, double Val)
{

 switch (Item)
  {
  case WCS_WAVEDATA_AMPLITUDE:
   {
   WD->Amp = Val;
   break;
   }
  case WCS_WAVEDATA_WHITECAPHT:
   {
   WD->WhiteCapHt = Val;
   break;
   }
  case WCS_WAVEDATA_LATOFF:
   {
   WD->LatOff = Val;
   break;
   }
  case WCS_WAVEDATA_LONOFF:
   {
   WD->LonOff = Val;
   break;
   }
  } /* switch */

} /* WaveData_SetDouble() */

/*********************************************************************/

double WaveData_Get(struct WaveData *WD, ULONG Item)
{

 switch (Item)
  {
  case WCS_WAVEDATA_AMPLITUDE:
   {
   return (WD->Amp);
   break;
   }
  case WCS_WAVEDATA_WHITECAPHT:
   {
   return (WD->WhiteCapHt);
   break;
   }
  case WCS_WAVEDATA_LATOFF:
   {
   return (WD->LatOff);
   break;
   }
  case WCS_WAVEDATA_LONOFF:
   {
   return (WD->LonOff);
   break;
   }
  default:
  {
      printf("Invalid Item %lu in %s %d",Item,__FILE__,__LINE__);
      return 0;
  }

  } /* switch */

} /* WaveData_Get() */

/*********************************************************************/

void WaveData_SetDefaults(struct WaveData *WD, short WaveType, short SetAll)
{

 WD->Amp = 1.0;
 WD->WhiteCapHt = 5.0;
 WD->LatOff = 0.0;
 WD->LonOff = 0.0;

} /* WaveData_SetDefaults() */

/*********************************************************************/

void Wave_Init(struct WaveData *WD, short Frame)
{
 if (WD)
  {
 
  if (Frame > WD->KT_MaxFrames)
   Frame = WD->KT_MaxFrames;

  if (WD->KT)
   {
   if (WD->KT->Key)
    {
    WD->Amp = WD->KT->Val[0][Frame];
    WD->WhiteCapHt = WD->KT->Val[1][Frame];
    WD->LatOff = WD->KT->Val[2][Frame];
    WD->LonOff = WD->KT->Val[3][Frame];
    } /* if */
   } /* if */
  } /* if */

} /* Wave_Init() */

/**********************************************************************/

short BuildWaveKeyTable(struct WaveData *WD)
{

return (BuildGenericKeyTable(&WD->KT, WD->WaveKey, WD->NumKeys,
		&WD->KT_MaxFrames, 4, 0, 4, WCS_KFPRECISION_FLOAT, NULL));

} /* BuildWaveKeyTable() */

/**********************************************************************/

