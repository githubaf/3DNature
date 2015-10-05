/* Illustrator.cpp
** Adobe Illustrator drawing functions and support code
** Resurrection & recoding from LineSupport.c by FPW2 06/17/99
** original code by Gary R. Huber
*/

#include "stdafx.h"
#include "WCSVersion.h"
// Don't need Illustrator.h anymore
//#include "Illustrator.h"
#include "ImageFormat.h"
#include "Application.h"
#include "Project.h"
#include "Render.h"
#include "Joe.h"
#include "Points.h"
#include "EffectsLib.h"
#include "PixelManager.h"
#include "Useful.h"

// Structure no longer used
/*
struct AIRasterPoint
	{
	float x, y;
	}; // AIRasterPoint
*/

extern WCSApp *GlobalApp;

#define ILLUS_RASTER

static long imagewidth, imageheight;	// screen width & height - need height at least since y is flipped in Illustrator

/*===========================================================================*/

ImageFormatAI *VectorOutput;

// possible parameters to pass: page size, page resolution, others?
long ImageFormatAI::IllustratorInit(double VecDPI, double RasDPI, long screenwidth, long screenheight, long Frame)
{
double pagewidth, pageheight;
char *timebuf;
time_t now;
int err = 0, WriteRaster = 0;
long  ipagewidth, ipageheight;

if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
	{
	imagewidth = screenwidth;
	imageheight = screenheight;
	fprintf(fHandle, "%%!PS-Adobe-3.0\n");
	fprintf(fHandle, "%%%%Creator: " APP_TITLE " "APP_VERS "\n");
	/*** The next 3 lines are optional ***/
	// We should save the user name & company here
	fprintf(fHandle, "%%%%For: (%s %s)\n", GlobalApp->MainProj->UserName, GlobalApp->MainProj->UserEmail);
	// The title of this document should be saved here - ie: WCS project name?
	fprintf(fHandle, "%%%%Title: (%s)\n", GlobalApp->MainProj->projectname);
	// The creation date is saved here
	(void)time(&now);
	timebuf = ctime(&now);
	timebuf[strlen(timebuf) - 1] = NULL;
	fprintf(fHandle, "%%%%CreationDate: (%s)\n", timebuf);
	// Don't know what the bounding box should really be - setting to 8.5" x 11 at this DPI"
	pagewidth = imagewidth;
	ipagewidth = quicklongceil(pagewidth);	// integer version
	pageheight = imageheight;
	ipageheight = quicklongceil(pageheight);
	fprintf(fHandle, "%%%%BoundingBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%%%HiResBoundingBox: 0.0000 0.0000 %.4lf %.4lf\n", pagewidth, pageheight);
	fprintf(fHandle, "%%%%DocumentProcessColors: Cyan Magenta Yellow Black\n");
	fprintf(fHandle, "%%%%DocumentNeededResources: procset Adobe_level2_AI5 1.2 0\n");
	fprintf(fHandle, "%%%%+ procset Adobe_ColorImage_AI6 1.1 0\n");
	fprintf(fHandle, "%%%%+ procset Adobe_Illustrator_AI5 1.0 0\n");
	fprintf(fHandle, "%%AI5_FileFormat 2.0\n");
	fprintf(fHandle, "%%AI3_ColorUsage: Color\n");
	fprintf(fHandle, "%%%%AI6_ColorSeparationSet: 1 1 (AI6 Default Color Separation Set)\n");
	fprintf(fHandle, "%%%%+ Options: 1 16 0 1 0 1 1 1 0 1 1 1 1 18 0 0 0 0 0 0 0 0 -1 -1\n");
	fprintf(fHandle, "%%%%+ PPD: 1 21 0 0 60 45 2 2 1 0 0 1 0 0 0 0 0 0 0 0 0 0 ()\n");
	fprintf(fHandle, "%%AI3_TemplateBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI3_TileBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI3_DocumentPreview: None\n");
	fprintf(fHandle, "%%AI5_ArtSize: %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI5_RulerUnits: 2\n");	// 0 = in, 1 = mm, 2 = pt, others unknown
	fprintf(fHandle, "%%AI5_ArtFlags: 0 0 0 1 0 0 1 1 0\n");
	fprintf(fHandle, "%%AI5_TargetResolution: %ld\n", (long)VecDPI);
	fprintf(fHandle, "%%AI5_NumLayers: 1\n");
	// viewable coordinates when doc is opened lowx, highx, ???
	// give 1/2" border
	fprintf(fHandle, "%%AI5_OpenToView: -269 800 -1.5 1137 819 18 0 1 7 43 0 0\n");
	fprintf(fHandle, "%%AI5_OpenViewLayers: 7\n");
	fprintf(fHandle, "%%%%EndComments\n");
	fprintf(fHandle, "%%%%BeginProlog\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_level2_AI5 1.2 0\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_ColorImage_AI6 1.1 0\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_Illustrator_AI5 1.0 0\n");
	fprintf(fHandle, "%%%%EndProlog\n");
	fprintf(fHandle, "%%%%BeginSetup\n");
	fprintf(fHandle, "Adobe_level2_AI5 /initialize get exec\n");
	fprintf(fHandle, "Adobe_ColorImage_AI6 /initialize get exec\n");
	fprintf(fHandle, "Adobe_Illustrator_AI5 /initialize get exec\n");
	// No clue for the following - just cutting & pasting
	fprintf(fHandle, "%%AI5_BeginPalette\n");
	fprintf(fHandle, "0 0 Pb\n");
	fprintf(fHandle, "1 1 1 1 k\n");
	fprintf(fHandle, "Pc\n");
	fprintf(fHandle, "PB\n");
	fprintf(fHandle, "%%AI5_EndPalette\n");
	fprintf(fHandle, "%%%%EndSetup\n");

#ifdef ILLUS_RASTER

	if(IOE)
		{
		if(strstr(IOE->Codec, "linked"))
			{
			if(IOE->Next && IOE->Next->Enabled)
				{
				WriteRaster = 1;
				} // if
			} // if
		} // if

	if(WriteRaster)
		{
		fprintf(fHandle, "%%AI5_BeginLayer\n");							// ?
		fprintf(fHandle, "1 1 1 1 0 0 1 1 255 79 79 0 50 Lb\n");	// vis preview, enabled, print, dimmed, multimask, index, r, g, b, ? ?
		fprintf(fHandle, "(Raster) Ln\n");
		fprintf(fHandle, "0 A\n");							// edit lock - OFF
		fprintf(fHandle, "0 O\n");							// ?
		fprintf(fHandle, "0 g\n");							// ?
		fprintf(fHandle, "%ld Ar\n", (long)VecDPI);						// output res
		fprintf(fHandle, "0 J 0 j 1 w 4 M []0 d\n");					// ?
		fprintf(fHandle, "%%AI3_Note:\n");
		fprintf(fHandle, "0 D\n");							// winding # for fills
		fprintf(fHandle, "0 XR\n");
		fprintf(fHandle, "%%AI5_File:\n");
		fprintf(fHandle, "%%AI5_BeginRaster\n");
		fprintf(fHandle, "(%s) 0 XG\n", EscapeBackslashes(GlobalApp->MainProj->MungPath(IOE->Next->PrepCompleteOutputPath(Frame))));
		//fprintf(fHandle, "[ 1.0 0 0 1.0 0 %d ] %d %d 0 Xh\n", screenheight, screenwidth, screenheight);
		fprintf(fHandle, "[ 1.0 0 0 1.0 0 %d ] 0 0 %d %d %d %d 8 3 0 0 0 0 XF\n", screenheight, screenwidth, screenheight, screenwidth, screenheight);
		//fprintf(fHandle, "XH\n");
		fprintf(fHandle, "%%AI5_EndRaster\n");
		fprintf(fHandle, "F\n");							// ?
		fprintf(fHandle, "LB\n");							// ?
		fprintf(fHandle, "%%AI5_EndLayer--\n");							// ?
		} // if
#endif // ILLUS_RASTER


	fprintf(fHandle, "%%AI5_BeginLayer\n");
	fprintf(fHandle, "1 1 1 1 0 0 0 0 79 128 255 0 100 Lb\n");	// vis preview, enabled, print, dimmed, multimask, index, r, g, b, ? ?
	fprintf(fHandle, "(Vectors) Ln\n");
	fprintf(fHandle, "0 A\n");							// edit lock - OFF
	fprintf(fHandle, "1 Ap\n");							// ???
	fprintf(fHandle, "0 R\n");							// overprinting on stroke path = OFF
	fprintf(fHandle, "0 0 0 1 K\n");					// stroke setcmykcolor
	fprintf(fHandle, "%ld Ar\n", (long)VecDPI);						// output res
	fprintf(fHandle, "0 D\n");							// winding # for fills
	err = (fprintf(fHandle, "0 XR\n") == 0);			// ???

	} // if
VectorOutput = this;
return (long)(err);
} // ImageFormatAI::IllustratorInit

/*===========================================================================*/

long ImageFormatAI::IllustratorEnd(void)
{
int err;
VectorOutput = NULL;
if(!fHandle) return(0); // error: file handle is NULL
fprintf(fHandle, "LB\n");
fprintf(fHandle, "%%AI5_EndLayer--\n");
fprintf(fHandle, "%%%%PageTrailer\n");
fprintf(fHandle, "gsave annotatepage grestore showpage\n");
fprintf(fHandle, "%%%%Trailer\n");
fprintf(fHandle, "Adobe_Illustrator_AI5 /terminate get exec\n");
fprintf(fHandle, "Adobe_ColorImage_AI6 /terminate get exec\n");
fprintf(fHandle, "Adobe_level2_AI5 /terminate get exec\n");
err = fprintf(fHandle, "%%EOF\n");
fclose(fHandle);
return (long)err;
}

/*===========================================================================*/

// this assumes you call BuildRasterPointList or similar
long ImageFormatAI::WriteIllustratorVector(CoordSys *DefCoords, Camera *Cam, PlanetOpt *PO, double VecDPI, double RasDPI, const char *obj_id,
   Joe *DrawMe, unsigned long numpts, unsigned long weight,
   unsigned char R, unsigned char G, unsigned char B,
   double EarthLatScaleMeters, double PlanetRad, double CenterPixelSize, long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, 
   rPixelHeader *rPixelFragMap)
{
double dx, dy, lastx, lasty, xpos, ypos;
unsigned long screenx, screeny, Zip;
struct AIRGBcolor rgb;
//struct AIRasterPoint start;
struct AICMYKcolor cmyk;
float  DPIfac;
unsigned long int plotcount = 0, ptcount;
int err;
VertexDEM FromVertex, ToVertex, TestVert;
VectorPoint *VectPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
#ifdef WCS_BUILD_VNS
#endif // WCS_BUILD_VNS
char plot;

if (MyAttr = (JoeCoordSys *)DrawMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;


// compute scaling factor in points (1/72") - this is hard coded to 8.5" x 11" page size for the moment
//DPIfac = (float)min((72.0 * 8.5) / imagewidth, (72.0 * 11.0) / imageheight);	// compute best fit to page
DPIfac = 1.0f;

rgb.r = R;
rgb.g = G;
rgb.b = B;
/*
 * we need to make sure that single point vectors work
 */
//start.x = DPIfac * ScreenX[1];
//start.y = DPIfac * ScreenY[1];
// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
fprintf(fHandle, "2 J 0 j %d w 4 M []0 d\n", weight);
fprintf(fHandle, "%%AI3_Note: %s\n", obj_id);
cmyk = ToCMYK(rgb);
// set stroke color - values are 0..1 with 4 decimal places
fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);
/*
fprintf(fHandle, "%.4f %.4f m\n", DPIfac * ScreenX[1], DPIfac * (imageheight - ScreenY[1]));
for (i = 2; i < numpts; i++)
	{
	fprintf(fHandle, "%.4lf %.4lf L\n", DPIfac * ScreenX[i], DPIfac * (imageheight - ScreenY[i]));
	}
*/
for (ptcount = 0, VectPt = DrawMe->GetFirstRealPoint(); VectPt; VectPt = VectPt->Next)
	{
	//ToVertex.Lat = VectPt->Latitude;
	//ToVertex.Lon = VectPt->Longitude;
	if (VectPt->ProjToDefDeg(MyCoords, &ToVertex))
		{
		ToVertex.Elev = CalcExag((double)VectPt->Elevation, PO);
		Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
		screenx = (unsigned long)(ToVertex.ScrnXYZ[0]);
		xpos = DPIfac * ToVertex.ScrnXYZ[0];
		screeny = (unsigned long)(ToVertex.ScrnXYZ[1]);
		ypos = DPIfac * (imageheight - ToVertex.ScrnXYZ[1]);
		if ((xpos >= 0 && xpos < TotalWidth) && (ypos >= 0 && ypos < TotalHeight))
			{
			plot = TRUE;
			Zip = screeny * TotalWidth + screenx;
			}
		else
			plot = FALSE;
#ifdef WCS_BUILD_VNS
		// Test to see if projected planimetric.  If so, we may subdivide this vector more.
		if ((Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) && Cam->Projected)
			{
			// If distance is > sqrt(2), subdivide.
			dx = lastx - xpos;
			dy = lasty - ypos;
			if (ptcount && ((dx * dx + dy * dy) > 4.0))
				{
				double invparts, partnum = 1.0, parts;
				double dLat, dLon, dElev = 0.0;
				VertexDEM OrigToVertex = ToVertex;

				parts = sqrt(dx * dx + dy * dy) * 1.5;
				dLat = OrigToVertex.Lat - FromVertex.Lat;
				dLon = OrigToVertex.Lon - FromVertex.Lon;
				dElev = OrigToVertex.Elev - FromVertex.Elev;
				invparts = 1.0 / parts;
				dLat = dLat * invparts;
				dLon = dLon * invparts;
				dElev = dElev * invparts;

				// now draw first n - 1 parts, then fall thru to plot last piece
				ToVertex.Lat = FromVertex.Lat + dLat;
				ToVertex.Lon = FromVertex.Lon + dLon;
				ToVertex.Elev = FromVertex.Elev + dElev;
				Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
				screenx = (unsigned long)(ToVertex.ScrnXYZ[0]);
				xpos = DPIfac * ToVertex.ScrnXYZ[0];
				screeny = (unsigned long)(ToVertex.ScrnXYZ[1]);
				ypos = DPIfac * (imageheight - ToVertex.ScrnXYZ[1]);
				if ((xpos >= 0 && xpos < TotalWidth) && (ypos >= 0 && ypos < TotalHeight))
					{
					plot = TRUE;
					Zip = screeny * TotalWidth + screenx;
					}
				else
					plot = FALSE;
				while (partnum < parts)
					{
					if (plot)
						{
						if (TileWidth == TotalWidth && TileHeight == TotalHeight 
							&& rPixelFragMap && (rPixelFragMap[Zip].UsedFrags > 0))
							{
							if (rPixelFragMap[Zip].FragList->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK)
								plot = FALSE;
							} // if
						else
							{
							TestVert.ScrnXYZ[0] = screenx + .5;
							TestVert.ScrnXYZ[1] = screeny + .5;
							TestVert.ScrnXYZ[2] = TestVert.Q = 0.0;
							if (! Cam->TestValidScreenPosition(&TestVert, CenterPixelSize))
								{
								plot = FALSE;
								} // if
							} // else
						}
					if (plot)
						{
						dx = lastx - xpos;
						dy = lasty - ypos;
						if (plotcount != 0 && ((dx * dx + dy * dy) > 4.0))
							{
							plotcount = 0;
							err = fprintf(fHandle, "S\n");
							}
						if (plotcount != 0)
							fprintf(fHandle, "%.4lf %.4lf L\n", xpos, ypos);
						else
							fprintf(fHandle, "%.4lf %.4lf m\n", xpos, ypos);
						lastx = xpos;
						lasty = ypos;
						plotcount++;
						} // if
					else
						{
						if (plotcount != 0)
							{
							err = fprintf(fHandle, "S\n");
							plotcount = 0;
							} // if
						} // else
					FromVertex.CopyLatLon(&ToVertex);
					FromVertex.CopyXYZ(&ToVertex);
					FromVertex.CopyScrnXYZQ(&ToVertex);
					ToVertex.Lat = FromVertex.Lat + dLat;
					ToVertex.Lon = FromVertex.Lon + dLon;
					ToVertex.Elev = FromVertex.Elev + dElev;
					Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
					screenx = (unsigned long)(ToVertex.ScrnXYZ[0]);
					xpos = DPIfac * ToVertex.ScrnXYZ[0];
					screeny = (unsigned long)(ToVertex.ScrnXYZ[1]);
					ypos = DPIfac * (imageheight - ToVertex.ScrnXYZ[1]);
					if ((xpos >= 0 && xpos < TotalWidth) && (ypos >= 0 && ypos < TotalHeight))
						{
						plot = TRUE;
						Zip = screeny * TotalWidth + screenx;
						}
					else
						plot = FALSE;
					partnum += 1.0;
					} // while
				} // if subdividing
			if (plot)
				{
				if (TileWidth == TotalWidth && TileHeight == TotalHeight 
					&& rPixelFragMap && (rPixelFragMap[Zip].UsedFrags > 0))
					{
					if (rPixelFragMap[Zip].FragList->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK)
						plot = FALSE;
					} // if
				else
					{
					TestVert.ScrnXYZ[0] = screenx + .5;
					TestVert.ScrnXYZ[1] = screeny + .5;
					TestVert.ScrnXYZ[2] = TestVert.Q = 0.0;
					if (! Cam->TestValidScreenPosition(&TestVert, CenterPixelSize))
						{
						plot = FALSE;
						} // if
					} // else
				}
			} // if projected planimetric
#endif // WCS_BUILD_VNS
		if (plot)
			{
			dx = lastx - xpos;
			dy = lasty - ypos;
			/*
			if (plotcount != 0 && ((dx * dx + dy * dy) > 4.0))
				{
				plotcount = 0;
				err = fprintf(fHandle, "S\n");
				}
			*/
			if (plotcount != 0)
				fprintf(fHandle, "%.4lf %.4lf L\n", xpos, ypos);
			else
				fprintf(fHandle, "%.4lf %.4lf m\n", xpos, ypos);
			plotcount++;
			} // if
		else
			{
			if (plotcount != 0)
				{
				err = fprintf(fHandle, "S\n");
				plotcount = 0;
				} // if
			} // else
		lastx = xpos;
		lasty = ypos;
		FromVertex.CopyLatLon(&ToVertex);
		FromVertex.CopyXYZ(&ToVertex);
		FromVertex.CopyScrnXYZQ(&ToVertex);
		ptcount++;
		} // if
	} // for

//if ((start.x == (long)(DPIfac * ScreenX[numpts - 1])) && (start.y == (long)(DPIfac * ScreenY[numpts - 1])))
/***
if(ptcount == 1)
	err = fprintf(fHandle, "s\n");	// closed path & stroke
else
	err = fprintf(fHandle, "S\n");	// stroke path
***/
if (plotcount != 0)
	err = fprintf(fHandle, "S\n");

return (long)err;

} // ImageFormatAI::WriteIllustratorVector

/*===========================================================================*/
