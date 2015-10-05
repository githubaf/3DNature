// nngridr.c
// Adapted from code by David F. Watson.
// Modified and Incorporated into World Construction Set in June, 1995 by GRH
// by permission of the author.
// Modified 3/15/00 by Gary R. Huber.
// Copyright 1995-2000 Questar Productions

// Primarily superceded by DEFG by CXH, but DEFG uses NNG's data structures, so we keep bits of it around.

#include "stdafx.h"
//#include "WCS.h"
#include "NNGrid.h"
#include "AppMem.h"
#include "Application.h"
#include "Types.h"
#include "Useful.h"
#include "Random.h"
#include "Requester.h"
#include "Notify.h"
#include "Joe.h"
#include "Database.h"
#include "DEM.h"
#include <string.h>

#include "FeatureConfig.h"

NNGrid::NNGrid()
{
// set everything else to NULL
bI = bJ = 0.0;
non_neg = extrap = 0;

grd_dir[0] = grd_file[0] = 0; 

strcpy(grd_file, "New DEM");
igrad = 1; // no longer used
extrap = 1;
horilap = 100.0;
vertlap = 100.0;
bJ = 7.0; // no loner used
#ifdef WCS_BUILD_VNS
bI = 100.0;
nuldat = -9999;
#else // !VNS
bI = 1.5;
nuldat = 0.0;
#endif // !VNS
yterm = 1.0;
ystart = 0.0;
xstart = 1.0;
xterm = 0.0;
x_nodes = 100;
y_nodes = 100;
} // NNGrid::NNGrid

/*===========================================================================*/

void ControlPointDatum::DeleteAllChildren(void)
{
ControlPointDatum *Current, *Next;

Next = nextdat;

while (Next)
	{
	Current = Next;
	Next = Next->nextdat;
	delete Current;
	} // while

nextdat = NULL;

} // ControlPointDatum::DeleteAllChildren
