// main.cpp
// test shell for exercising DEFG gridder
// created from scratch and parts of DEFG.cpp on 
// Fri Aug 24, 2001 by CXH

#include <stdio.h>
#include "DEFG.h"
#include "DEFGSupport.h"

//#define DEFG_INFILE				"RMNP42.xyz"
//#define DEFG_INFILE				"Green.xyz"
#define DEFG_INFILE				"etopo511B.xyz"
#define DEFG_GRID_WIDTH			1000
#define DEFG_GRID_HEIGHT		1000
#define DEFG_MAX_INPOINTS		60000

int main(int Count, char *Vector[])
{
DEFG Gridder;
int NumPoints = 0;


Gridder.InitSizes(DEFG_GRID_WIDTH, DEFG_GRID_HEIGHT, 1.0, 1.0, DEFG_MAX_INPOINTS);

printf("Loading points...\n");
if(Count > 1)
	{
	NumPoints = LoadPoints(Vector[1], &Gridder);
	} // if
else
	{
	NumPoints = LoadPoints(DEFG_INFILE, &Gridder);
	} // else

printf("Total Points loaded: %d.\n", NumPoints);

if(NumPoints)
	{
	printf("Bounding points.\n");

	Gridder.AutoBoundPoints();
	Gridder.Grid();
	} // if

return(0);
} // main
