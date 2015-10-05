// AmigaScreenModeGUI.h
// minor stuff for screen mode selector
// created from scratch on 11 Jul 1995 by Chris "Xenon" Hanson
// Copyright 1995


#include "GUI.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "AppMem.h"
#include <graphics/gfx.h>

struct WCSScreenMode
	{
	struct WCSScreenMode *Next;
	ULONG ModeID;
	char ModeName[40];
	char Depth;
	int X, Y, OX, OY, MaxX, MaxY, UX, UY;
	struct tPoint OScans[4]; /* By order: Text, Std, Max, Video */
	ULONG PropertyFlags;
	UWORD PixelSpeed;
	}; /* struct WCSScreenMode */

struct WCSScreenData
	{
	ULONG ModeID, OTag, OVal, AutoTag, AutoVal;
	long Width, Height;
	char Depth;
	};

#define ID_SM_LIST	10
#define ID_SM_OSCAN	11
#define ID_SM_HEIGHT	12
#define ID_SM_WIDTH	13
#define ID_SM_SAVE	14
#define ID_SM_USE		15
#define ID_SM_EXIT	16

void MUI_DoNotiPresFal(APTR App, ...);

/* NOTE: KeyButtonObject and FixedImgObject do not have a hanging , of their
	 own! */

#define FixedImgObject ImageObject,ButtonFrame,MUIA_InputMode,MUIV_InputMode_RelVerify,MUIA_Image_FreeHoriz, FALSE,MUIA_Image_FreeVert, FALSE,MUIA_Background, MUII_BACKGROUND

#define KeyButtonObject(key) TextObject,ButtonFrame,MUIA_Text_PreParse, "\33c",MUIA_Text_SetMax, FALSE,MUIA_Text_HiChar, key,MUIA_ControlChar, key,MUIA_InputMode,MUIV_InputMode_RelVerify,MUIA_Background,MUII_ButtonBack

#define FloatStringObject StringObject,MUIA_String_Accept,"0123456789.-+"

/*
#define FixedImgObject ImageObject,\
  ButtonFrame,\
  MUIA_InputMode, MUIV_InputMode_RelVerify,\
  MUIA_Image_FreeHoriz, FALSE,\
  MUIA_Image_FreeVert, FALSE,\
  MUIA_Background, MUII_BACKGROUND

#define KeyButtonObject(key) TextObject,\
  ButtonFrame,\
  MUIA_Text_PreParse, "\33c",\
  MUIA_Text_SetMax, FALSE,\
  MUIA_Text_HiChar, key,\
  MUIA_ControlChar, key,\
  MUIA_InputMode,MUIV_InputMode_RelVerify,\
  MUIA_Background,MUII_ButtonBack

#define FloatStringObject StringObject,\
  MUIA_String_Accept, "0123456789.-+"
*/