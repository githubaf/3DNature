// RasterDrawWidget.h
// built from AnimGraph.h by Gary R. Huber
// Copyright 2001 by Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// header stuff - put this into WCSWidgets.h
#ifndef WCS_RASTERWIDGET_H
#define WCS_RASTERWIDGET_H

#include "Types.h"
#include "GUI.h"
#include "Fenetre.h"

class Raster;

// longword Offsets into GraphWidget class
enum
	{
	WCSW_RD_OFF_RDDATA =		0
//	WCSW_RD_OFF_ =	4,
//	WCSW_RD_OFF_ =	8,
//	WCSW_RD_OFF_ =			12
	};

void ConfigureRD(NativeGUIWin Dest, int Target, struct RasterWidgetData *NewData);
void ConfigureRD(NativeGUIWin Dest, struct RasterWidgetData *NewData);

struct RasterWidgetData
	{
	double Visible, OffsetX, OffsetY;
	Raster *MainRast, *OverRast; // Initialized at start
	WCSModule *RDWin;
	short x, y, sx, sy;
	short left, right, top, bottom;
	// CollectMouseMove must be initialized to 0.
	char CollectMouseMove;
	};

#define WCSW_GRW_NODE_SELECT		(0<<0)
#define WCSW_GRW_NODE_TOGGLESELECT	(1<<0)
#define WCSW_GRW_NODE_NEW			(1<<1)
#define WCSW_GRW_NODE_ALTERPOSITION	(1<<2)
#define WCSW_GRW_NODE_ALTERVALUE	(1<<3)

#define WCSW_GRW_MADF_DRAWUPDATE			(1<<0)
#define WCSW_GRW_MADF_DRAWOBJECT			(1<<1)
#define WCSW_GRW_MADF_DRAWGRAPH				(1<<2)
#define WCSW_GRW_MADF_DRAWPARTGRAPH			(1<<3)

#endif // WCS_RASTERWIDGET_H
