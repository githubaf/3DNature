// GraphWidget.h
// new for V4: Graphing capability for GraphData class object
// built from TLWidget.cpp on 5/7/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// header stuff - put this into WCSWidgets.h
#ifndef WCS_GRAPHWIDGET_H
#define WCS_GRAPHWIDGET_H

#include "Types.h"
#include "GUI.h"
#include "Fenetre.h"

class AnimCritter;
class AnimDouble;
class GraphData;
class GraphNode;

// longword Offsets into GraphWidget class
enum
	{
	WCSW_GR_OFF_GRDATA =		0
//	WCSW_GR_OFF_ =	4,
//	WCSW_GR_OFF_ =	8,
//	WCSW_GR_OFF_ =			12
	};

void ConfigureGR(NativeGUIWin Dest, int Target, struct GRData *NewData);
void ConfigureGR(NativeGUIWin Dest, struct GRData *NewData);

class GraphDialog
	{
	public:

		virtual void NewActiveNode(GraphNode *NewActive) = 0;

		}; // class GraphDialog

struct GRData
	{
	short x, y, sx, sy;
	short left, right, top, bottom, textbottom, textzero, 
		textwidthtop, textwidthbottom, textwidthzero,
		DistGridLg, drawgrid, HPan, HVisible, VPan, VVisible, DisplayByFrame, SnapToInt;
	double PixelsPerDistGrid, PixelsPerValGrid, PixelsPerVal, PixelsPerDist,
		LowDrawDist, HighDrawDist, MaxDistRange, FrameRate, MaxHighDrawVal, MinLowDrawVal,
		LowDrawVal, HighDrawVal, DistGridBase, DistGridInt, ValGridBase, ValGridInt,
		TextLowDrawDist, TextHighDrawDist;
	AnimCritter *Crit, *ElevCrit; // Initialized at start
	GraphNode *ActiveNode;
	GraphDialog *GrWin;
	AnimDouble *ValuePrototype, *DistancePrototype;
	char NumGraphs;
	long inputflags, drawflags;
	double DistInterval;	// this is the distance used between computed points - it depends on the display units
	// CollectMouseMove must be initialized to 0.
	char CollectMouseMove;
	};

#define WCSW_GRW_NODE_SELECT		(0<<0)
#define WCSW_GRW_NODE_TOGGLESELECT	(1<<0)
#define WCSW_GRW_NODE_NEW			(1<<1)
#define WCSW_GRW_NODE_ALTERPOSITION	(1<<2)
#define WCSW_GRW_NODE_ALTERVALUE	(1<<3)
// these were defined for TLWidget but they don't seem to be used
//#define WCSW_GRW_POINT_SELECTED	(1<<2)
//#define WCSW_GRW_QUICK_DRAW		(1<<3)
//#define WCSW_GRW_NO_CLEAR			(1<<4)

#ifdef _WIN32
#define WCSW_GRW_MADF_DRAWUPDATE			(1<<0)
#define WCSW_GRW_MADF_DRAWOBJECT			(1<<1)
#define WCSW_GRW_MADF_DRAWGRAPH				(1<<2)
#define WCSW_GRW_MADF_DRAWPARTGRAPH			(1<<3)
#endif // _WIN32

#endif // WCS_GRAPHWIDGET_H
