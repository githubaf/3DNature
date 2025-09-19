/* Wave.h
** Headers and defines for wave and wave gui functions in WCS
** Created 12/95 by Gary Huber
** Copyright 1995 by Questar Productions
*/

#ifndef WAVE_H
#define WAVE_H

#define WCS_WAVE_LAT		0x0001
#define WCS_WAVE_LON		0x0002
#define WCS_WAVE_AMP		0x0003
#define WCS_WAVE_LENGTH		0x0004
#define WCS_WAVE_VELOCITY	0x0005

#define WCS_WAVEDATA_LATOFF	0x0100
#define WCS_WAVEDATA_LONOFF	0x0200
#define WCS_WAVEDATA_AMPLITUDE	0x0300
#define WCS_WAVEDATA_WHITECAPHT	0x0400

#define FRAMES_PER_HOUR		108000.0

/* file stuff - don't touch! */

#define WAVE_CURRENT_VERSION	1

#define WAVE_NEW		100
#define WAVE_SHT_NUMKEYS	101	
#define WAVE_DBL_AMP		102
#define WAVE_DBL_LENGTH		103
#define WAVE_DBL_VELOCITY	104
#define WAVE_DBL_LAT		105
#define WAVE_DBL_LON		106

#define WAVEDATA_SHT_NUMKEYS	1
#define WAVEDATA_SHT_NUMWAVES	2
#define WAVEDATA_DBL_AMPLITUDE	3
#define WAVEDATA_DBL_WHITECAP	4
#define WAVEDATA_DBL_LATOFF	5
#define WAVEDATA_DBL_LONOFF	6
#define WAVEDATA_SHT_KFITEMS	22
#define WAVEDATA_SHT_KEYFRAME	23
#define WAVEDATA_SHT_KFGROUP	24
#define WAVEDATA_SHT_KFITEM	25
#define WAVEDATA_FLT_KFTCB	26
#define WAVEDATA_SHT_KFLINEAR	27
#define WAVEDATA_FLT_KFVALUES	28

struct WaveWindow {
  APTR WaveWin, FloatStr[4], WaveStr[5], BT_AddWave, BT_MapAddWave, BT_RemoveWave,
	BT_DrawWaves, BT_DrawDetail, BT_Load, BT_Save, FloatArrow[4][2],
	WaveArrow[5][2], LS_WaveList, BT_Settings[4];
  ULONG WaveListSize;
  struct Wave *CurWave, **WaveAddrList;
  struct WaveData *WD;
  struct Window *Win;
  struct GUIKeyStuff GKS;
  struct WindowKeyStuff WKS;
  struct TimeLineWindow *TL;
  short Mod, WaveEntries, CurrentWave;
  char **WaveList, WaveDir[256], WaveFile[32], WaveNames[100][4];
};

/* Wave.c */
struct Wave *Wave_New(void);
void Wave_Del(struct Wave *WV);
void Wave_DelAll(struct Wave *WV);
void Wave_Set(struct Wave *WV, ULONG Item, double Val);
double Wave_Get(struct Wave *WV, ULONG Item);
void Wave_SetDefaults(struct Wave *NewWave, struct Wave *PtrnWave);
struct WaveData *WaveData_New(void);
void WaveData_Del(struct WaveData *WD);
void WaveData_SetDouble(struct WaveData *WD, ULONG Item, double Val);
double WaveData_Get(struct WaveData *WD, ULONG Item);
void WaveData_SetDefaults(struct WaveData *WD, short WaveType, short SetAll);
void Wave_Init(struct WaveData *WD, short Frame);
short BuildWaveKeyTable(struct WaveData *WD);

/* WaveGUI.c */
void Close_WV_Window(struct WaveWindow **WVWinPtr);
void Handle_WV_Window(ULONG WCS_ID);
short Wave_Load(char *filename, struct WaveData **WDPtr);
short Wave_Save(char *filename, struct WaveData *WD);
short Wave_Add(struct Wave **Wave);
void BuildWaveList(struct WaveWindow *WV_Win, struct WaveData *WD);
void GUIWave_SetGads(struct WaveWindow *WV_Win, struct Wave *WV);

void GUIDisableKeyButtons(struct GUIKeyStuff *GKS,
	struct TimeLineWindow *TL, struct WindowKeyStuff *WKS);

#endif /* WAVE_H */
