// TerrainTexCoordCalc.h
// mini-object for encapsulating NVE terrain texture coordinate calculations
// created on 8/24/05 by CXH

#ifndef TERRAINTEXCOORDCALC_H
#define TERRAINTEXCOORDCALC_H

class TerrainTexCoordCalc
	{
	public:
		TerrainTexCoordCalc() {};
		TerrainTexCoordCalc(double XMin, double XMax, double YMin, double YMax);
		void SetupCoords(double XMin, double XMax, double YMin, double YMax);

		// Converts Geospatial coords to UV coords
		float GetTexXCoord(double VertexXCoord); // VertexCoord is geospatial, return value is texture UV coord (OGL LL origin) [should be const]
		float GetTexYCoord(double VertexYCoord); // VertexCoord is geospatial, return value is texture UV coord (OGL LL origin) [should be const]
		// Converts UV coords to Geospatial
		double GetGeoXCoord(float TexXCoord); // TexXCoord is UV (OGL LL origin), return value is geospatial coord [should be const]
		double GetGeoYCoord(float TexYCoord); // TexYCoord is UV(OGL LL origin), return value is geospatial coord [should be const]

/*		
		// these two round DOWN (truncate), to which is fine for pixels on the left/top edge of a bounds
		unsigned long int GetTexXPixelCoord(double VertexXCoord, unsigned long int ImageWidth) {return((unsigned long int)(ImageWidth * GetTexXCoord(VertexXCoord)));};
		unsigned long int GetTexYPixelCoord(double VertexYCoord, unsigned long int ImageHeight) {return((unsigned long int)(ImageHeight * GetTexYCoord(VertexYCoord)));};
		// these two round UP, to incorporate partial pixels on the right/bottom edge of a bounds
		unsigned long int GetTexXPixelCoordRUP(double VertexXCoord, unsigned long int ImageWidth) {return((unsigned long int)(0.5 + (double)ImageWidth * GetTexXCoord(VertexXCoord)));};
		unsigned long int GetTexYPixelCoordRUP(double VertexYCoord, unsigned long int ImageHeight) {return((unsigned long int)(0.5 + (double)ImageHeight *GetTexYCoord(VertexYCoord)));};
*/
		
	private:
		double TexXRange, TexYRange, TexOriginX, TexOriginY, TexXRangeInv, TexYRangeInv;
	}; // TerrainTexCoordCalc

class ImageTileBounds
	{
	public:
		signed long int OriX, OriY;
		unsigned long int SubstantialOriX, SubstantialOriY;
		unsigned long int SubstantialOffsetX, SubstantialOffsetY;
		unsigned long int Width, Height;
		unsigned long int SubstantialWidth, SubstantialHeight;
		unsigned long int SubstantialWidthDS, SubstantialHeightDS;
		double SLat, WLon, NLat, ELon; // bounds of center of pixels in corners of full-size (expanded) texturemap.
	}; // ImageTileBounds


unsigned long int RoundUpPowTwo(unsigned long int Input);

#endif // TERRAINTEXCOORDCALC_H
