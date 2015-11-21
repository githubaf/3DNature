// TerrainTexCoordCalc.cpp
// mini-object for encapsulating NVE terrain texture coordinate calculations
// created on 8/24/05 by CXH

#include "TerrainTexCoordCalc.h"

TerrainTexCoordCalc::TerrainTexCoordCalc(double XMin, double XMax, double YMin, double YMax)
{
SetupCoords(XMin, XMax, YMin, YMax);
} // TerrainTexCoordCalc::TerrainTexCoordCalc

void TerrainTexCoordCalc::SetupCoords(double XMin, double XMax, double YMin, double YMax)
{
TexOriginX = XMax;
TexOriginY = YMin;

TexYRange = (YMax - YMin);
TexXRange = (XMax - XMin);

TexYRangeInv = 1.0 / TexYRange; // optimize: avoid repeated divide
TexXRangeInv = 1.0 / TexXRange; // optimize: avoid repeated divide
} // TerrainTexCoordCalc::SetupCoords


float TerrainTexCoordCalc::GetTexXCoord(double VertexXCoord)
{
return((float)((TexOriginX - VertexXCoord)  * TexXRangeInv));
} // TerrainTexCoordCalc::GetTexXCoord


float TerrainTexCoordCalc::GetTexYCoord(double VertexYCoord)
{
return((float)((VertexYCoord - TexOriginY) * TexYRangeInv));
} // TerrainTexCoordCalc::GetTexYCoord


double TerrainTexCoordCalc::GetGeoXCoord(float TexXCoord)
{
return(TexOriginX - (TexXCoord) * TexXRange);
} // TerrainTexCoordCalc::GetGeoXCoord

double TerrainTexCoordCalc::GetGeoYCoord(float TexYCoord)
{
return(TexOriginY + TexYCoord * TexYRange);
} // TerrainTexCoordCalc::GetGeoYCoord



unsigned long int RoundUpPowTwo(unsigned long int Input)
{
unsigned long int Shifter;

for(Shifter = ~0; Input;)
	{
	Input = Input >> 1;
	Shifter = Shifter << 1;
	} // for

return((~Shifter) + 1);
} // RoundUpPowTwo
