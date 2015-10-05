// ExportFormatSTL.cpp
// Code module for STL export code
// Created from ExportFormat.cpp on 5/18/04 by CXH
// ExportFormat.cpp Created from scratch 07/01/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ExportFormat.h"
#include "EffectsLib.h"
#include "ExportControlGUI.h"
#include "IncludeExcludeList.h"
#include "Raster.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Requester.h"
#include "TerrainWriter.h"
#include "CubeSphere.h"
#include "AppMem.h"
#include "RasterResampler.h"
#include "Log.h"
#include "SceneExportGUI.h"
#include "SXExtension.h"
#include "zlib.h"

ExportFormatSTL::ExportFormatSTL(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

} // ExportFormatSTL::ExportFormatSTL

/*===========================================================================*/

ExportFormatSTL::~ExportFormatSTL()
{

} // ExportFormatSTL::~ExportFormatSTL

/*===========================================================================*/

float ExportFormatSTL::MinElev(FILE *RawElevs, long Cols, long Rows)
{
float  min = FLT_MAX, val;
long i, j = Cols * Rows, k = 0;

rewind(RawElevs);
for (i = 0; i < j; i++)
	{
	fread(&val, 1, 4, RawElevs);
	if ((val != -9999.0f) && (val < min))
		min = val;
	} // for

if (min == -9999.0f)
	val = 0.0f;

rewind(RawElevs);
return min;

} // ExportFormatSTL::MinElev

/*===========================================================================*/

int ExportFormatSTL::PackageExport(NameList **FileNamesCreated)
{
double offset = 0.001, xmin, xmax, ymin, ymax, zmin, zmax, scalefactor = 1.0, xrange, yrange, zrange, dX, dY, xpos, ypos, VertExag;
double MinThickness = 1.0;
double xspace, yspace, sizex, sizey; //, sizez;
FILE *fRaw = NULL, *fSTL = NULL;
float *Line1 = NULL, *Line2 = NULL;
const char *OutputFilePath, *RawTerrainFile;
float xout, yout, zout;
int Success = 0;
long ASCIImode = 0;	// Not implemented in interface, and probably never will be.  Useful for debugging though.
long BuildToFit = 1, scancol, scanrow;
long FileType;
size_t LineSize;
unsigned short zero = 0;
PathAndFile SceneOutput;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

if ((Master->FormatExtension) && (Master->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL))
	{
	class SXExtensionSTL *SXExtSTL = (class SXExtensionSTL *)Master->FormatExtension;

	//BuildEnvelope[0] = (float)SXExtSTL->ActualDimX;
	//BuildEnvelope[1] = (float)SXExtSTL->ActualDimY;
	//BuildEnvelope[2] = (float)SXExtSTL->ActualDimZ;
	BuildEnvelope[0] = (float)SXExtSTL->MaxDimX;
	BuildEnvelope[1] = (float)SXExtSTL->MaxDimY;
	BuildEnvelope[2] = (float)SXExtSTL->MaxDimZ;
	VertExag = SXExtSTL->VertExag / 100.0;
	if (SXExtSTL->BuildMode == WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOSCALE)
		BuildToFit = 0;
	if (SXExtSTL->UnitOfMeasure == WCS_EFFECTS_SXEXTENSION_BUILDUNIT_INCHES)
		{
		scalefactor = 1.0 / 25.4;
		BuildEnvelope[0] *= 25.4f;
		BuildEnvelope[1] *= 25.4f;
		BuildEnvelope[2] *= 25.4f;
		MinThickness = SXExtSTL->MinThickness * scalefactor;
		} // if
	} // if
else
	{
	// shouldn't get here, but set some default just in case
	// defaults to Z Corp's Z406 dimension
	BuildEnvelope[0] = 254.0f;
	BuildEnvelope[1] = 203.0f;
	BuildEnvelope[2] = 203.0f;
	VertExag = 1.0;
	MinThickness = 5.0;
	} // else

// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

SceneOutput.SetPath((char *)Master->OutPath.GetPath());
SceneOutput.SetName((char *)Master->OutPath.GetName());
SceneOutput.GetFramePathAndName(TempFullPath, ".StL", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);

// STL can't be tiled
FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
RawTerrainFile = (*FileNamesCreated)->FindNameOfType(FileType);
fRaw = PROJ_fopen(RawTerrainFile, "rb");

xspace = Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale;	// convert geographic to meters
yspace = Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale;
sizex = (Master->DEMResX - 1) * Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale;
sizey = (Master->DEMResY - 1) * Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale;

if (ASCIImode)
	fSTL = PROJ_fopen(TempFullPath, "w");
else
	fSTL = PROJ_fopen(TempFullPath, "wb");

LineSize = Master->DEMResX * sizeof(float);
Line1 = (float *)AppMem_Alloc(LineSize, 0);
Line2 = (float *)AppMem_Alloc(LineSize, 0);
if ((Line1 == NULL) || (Line2 == NULL))
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "STL Export aborted - critically short on memory!");

if (Line1 && Line2 && fRaw && fSTL)
	{
	xmin = 0.0;
	ymin = 0.0;
	xmax = sizex;
	ymax = sizey;
	//zmin = Master->ExportRefData.RefElev; -> can't rely on this because water & bathymetry would screw things up
	zmin = MinElev(fRaw, Master->DEMResX, Master->DEMResY);
	zmax = Master->ExportRefData.MaxElev;
	xrange = xmax - xmin;
	yrange = ymax - ymin;
	zrange = (zmax * VertExag) - (zmin * VertExag);
	if (BuildToFit)
		{
		double xfactor, yfactor, zfactor;

		// find out which dimension needs to be scaled the most, use the inverse of that as the scaling factor
		xfactor = xrange / (BuildEnvelope[0] - offset);	// apparently, 0.0 isn't a valid output coordinate
		yfactor = yrange / (BuildEnvelope[1] - offset);
		zfactor = zrange / (BuildEnvelope[2] - offset);
		if (xfactor >= yfactor)
			{
			if (xfactor >= zfactor)
				scalefactor *= 1.0 / xfactor;
			else
				scalefactor *= 1.0 / zfactor;
			} // if
		else if (yfactor >= zfactor)
			{
			scalefactor *= 1.0 / yfactor;
			} // else if
		else
			{
			scalefactor *= 1.0 / zfactor;
			}
		} // if BuildToFit
	else
		{
		// Build to scale
		} // else

	dX = xrange / (Master->DEMResX - 1);
	dY = yrange / (Master->DEMResY - 1);

	if (ASCIImode)
		fprintf(fSTL, "solid\n");
	else
		{
		unsigned long facets;
		char Header[80];
		char String[] = "STL generated by 3D Nature's Scene Express";

		// facets for top + xsides + ysides + bottom (formula corrected 10/12/05 FW2)
		facets = ((Master->DEMResX - 1) * (Master->DEMResY - 1)* 2) + ((Master->DEMResX - 1) * 4) + ((Master->DEMResY - 1) * 4) + 2;
		memset(Header, 0, 80);
		memcpy(Header, String, sizeof(String));
		fwrite(Header, 1, 80, fSTL);
		Put32U(LITTLE_END_DATA, facets, fSTL);
		} // else

	// do the top surface
	ypos = yrange;
	fread(Line1, 1, LineSize, fRaw);
	for (scanrow = 1; scanrow < Master->DEMResY; scanrow++, ypos -= dY)
		{
		xpos = offset;	// 0.0 apparently isn't a legal coordinate for STL
		fread(Line2, 1, LineSize, fRaw);
		// write triangles for the two lines we have
		for (scancol = 1; scancol < Master->DEMResX; scancol++, xpos += dX)
			{
			long backcol = scancol - 1;
			// triangles are counterclockwise on the outer surface
			// triangle #1 with verts 0 1 2, verts 0 & 2 on scancol, vert 1 on backcol:
			// 1 - 0 -> Line1
			//   \ |
			//     2 -> Line2
			if (ASCIImode)
				{
				fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
				fprintf(fSTL, "    outer loop\n");
				} // if
			else
				{
				xout = 0.0f; yout = 0.0f, zout = 1.0f;
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				}
			// vert 0
			xout = (float)((xpos + dX) * scalefactor + offset);
			yout = (float)((ypos) * scalefactor + offset);
			zout = (float)((Line1[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				}
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			// vert 1
			xout = (float)((xpos) * scalefactor + offset);
			yout = (float)((ypos) * scalefactor + offset);
			zout = (float)((Line1[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				}
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			// vert 2
			xout = (float)((xpos + dX) * scalefactor + offset);
			yout = (float)((ypos - dY) * scalefactor + offset);
			zout = (float)((Line2[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				}
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			if (ASCIImode)
				{
				fprintf(fSTL, "    end loop\n");
				fprintf(fSTL, "  endfacet\n");
				} // if
			else
				{
				Put16U(LITTLE_END_DATA, zero, fSTL);
				}
			// triangle #2 with verts 3 4 5, vert 3 on scancol, verts 4 & 5 on backcol:
			// 4     -> Line1	***
			// | \				***
			// 5 - 3 -> Line2	***
			if (ASCIImode)
				{
				fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
				fprintf(fSTL, "    outer loop\n");
				} // if
			else
				{
				xout = 0.0f; yout = 0.0f, zout = 1.0f;
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				}
			// vert 3
			xout = (float)((xpos + dX) * scalefactor + offset);
			yout = (float)((ypos - dY) * scalefactor + offset);
			zout = (float)((Line2[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				}
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			// vert 4
			xout = (float)((xpos) * scalefactor + offset);
			yout = (float)((ypos) * scalefactor + offset);
			zout = (float)((Line1[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				} // if
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			// vert 5
			xout = (float)((xpos) * scalefactor + offset);
			yout = (float)((ypos - dY) * scalefactor + offset);
			zout = (float)((Line2[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
			if (ASCIImode)
				{
				fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
				}
			else
				{
				Put32F(LITTLE_END_DATA, &xout, fSTL);
				Put32F(LITTLE_END_DATA, &yout, fSTL);
				Put32F(LITTLE_END_DATA, &zout, fSTL);
				} // else
			if (ASCIImode)
				{
				fprintf(fSTL, "    end loop\n");
				fprintf(fSTL, "  endfacet\n");
				} // if
			else
				{
				Put16U(LITTLE_END_DATA, zero, fSTL);
				}
			} // for scancol
		memcpy(Line1, Line2, LineSize);
		} // for scanrow

	// do the south wall - Line1 still has the data in it's buffer
	xpos = offset;
	ypos = offset;
	for (scancol = 1; scancol < Master->DEMResX; scancol++, xpos += dX)
		{
		long backcol = scancol - 1;
		// triangles are counterclockwise on the outer surface
		// triangle #1 with verts 0 1 2, verts 0 & 2 on scancol, vert 1 on backcol:
		// 1 - 0 -> Elev
		//   \ |
		//     2 -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 0
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 1
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 2
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		// triangle #2 with verts 3 4 5, vert 3 on scancol, verts 4 & 5 on backcol:
		// 4     -> Elev	***
		// | \				***
		// 5 - 3 -> 0.001	***
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 3
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 4
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			} // if
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 5
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		} // for scancol

	// do the north wall
	rewind(fRaw);
	fread(Line1, 1, LineSize, fRaw);
	xpos = offset;
	ypos = yrange;
	for (scancol = 1; scancol < Master->DEMResX; scancol++, xpos += dX)
		{
		long backcol = scancol - 1;
		// triangles are counterclockwise on the outer surface
		// triangle #1 with verts 0 1 2, verts 0 & 2 on backcol, vert 1 on scancol: { viewed from south }
		// 0 - 1 -> Elev
		// | /
		// 2     -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 0
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[backcol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 1
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 2
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		// triangle #2 with verts 3 4 5, verts 3 & 4 on scancol, vert 5 on backcol:
		//     3 -> Elev
		//   / |
		// 5 - 4 -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 3
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[scancol] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 4
		xout = (float)((xpos + dX) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			} // if
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 5
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		} // for scancol

	// do the W & E walls (Line1 still has the first line of data)
	//ypos = (BuildEnvelope[1] - offset) / scalefactor;
	ypos = yrange;
	for (scanrow = 1; scanrow < Master->DEMResY; scanrow++, ypos -= dY)
		{
		xpos = offset;
		fread(Line2, 1, LineSize, fRaw);
		// do the West wall - triangles are counterclockwise on the outer surface
		// triangle #1 with verts 0 1 2, verts 1 & 2 on Line1, vert 0 on Line2: { viewed from west }
		// 1 - 0 -> Elev
		// | /
		// 2     -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 0
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)((Line2[0] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 1
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[0] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 2
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		// triangle #2 with verts 3 4 5, verts 3 & 5 on Line2, vert 4 on Line1:
		//     3 -> Elev
		//   / |
		// 4 - 5 -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 3
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)((Line2[0] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 4
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			} // if
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 5
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		xpos = xmax;
		// do the East wall - triangles are counterclockwise on the outer surface
		// triangle #1 with verts 0 1 2, verts 1 & 2 on Line2, vert 0 on Line1: { viewed from east }
		// 1 - 0 -> Elev
		// | /
		// 2     -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 0
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[Master->DEMResX - 1] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 1
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)((Line2[Master->DEMResX - 1] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 2
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		// triangle #2 with verts 3 4 5, verts 3 & 5 on Line1, vert 4 on Line2:
		//     3 -> Elev
		//   / |
		// 4 - 5 -> 0.001
		if (ASCIImode)
			{
			fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
			fprintf(fSTL, "    outer loop\n");
			} // if
		else
			{
			xout = 0.0f; yout = 0.0f, zout = 1.0f;
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			}
		// vert 3
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)((Line1[Master->DEMResX - 1] - zmin) * VertExag * scalefactor + MinThickness + offset);
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 4
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos - dY) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			} // if
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		// vert 5
		xout = (float)((xpos) * scalefactor + offset);
		yout = (float)((ypos) * scalefactor + offset);
		zout = (float)offset;
		if (ASCIImode)
			{
			fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
			}
		else
			{
			Put32F(LITTLE_END_DATA, &xout, fSTL);
			Put32F(LITTLE_END_DATA, &yout, fSTL);
			Put32F(LITTLE_END_DATA, &zout, fSTL);
			} // else
		if (ASCIImode)
			{
			fprintf(fSTL, "    end loop\n");
			fprintf(fSTL, "  endfacet\n");
			} // if
		else
			{
			Put16U(LITTLE_END_DATA, zero, fSTL);
			}
		memcpy(Line1, Line2, LineSize);
		} // for scanrow

	// do the bottom (Line1 & Line2 both have copies of the last row of data)
	// triangles are counterclockwise on the outer surface
	// triangle #1 with verts 0 1 2, vert 1 @ NE, vert 0 @ NW, vert 2 @ SE: { viewed from bottom }
	// 1 - 0 -> 0.001
	// | /
	// 2     -> 0.001
	if (ASCIImode)
		{
		fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
		fprintf(fSTL, "    outer loop\n");
		} // if
	else
		{
		xout = 0.0f; yout = 0.0f, zout = 1.0f;
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		}
	// vert 0
	xout = (float)offset;
	yout = (float)(ymax * scalefactor + offset);
	zout = (float)offset;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		}
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	// vert 1
	xout = (float)(xmax * scalefactor + offset);
	yout = (float)(ymax * scalefactor + offset);
	zout = (float)offset;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		}
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	// vert 2
	xout = (float)(xmax * scalefactor + offset);
	yout = (float)offset;
	zout = (float)offset;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		}
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	if (ASCIImode)
		{
		fprintf(fSTL, "    end loop\n");
		fprintf(fSTL, "  endfacet\n");
		} // if
	else
		{
		Put16U(LITTLE_END_DATA, zero, fSTL);
		}
	// triangle #2 with verts 3 4 5, vert 3 @ NW, vert 4 @ SW, vert 5 @ SE
	//     3 -> 0.001
	//   / |
	// 4 - 5 -> 0.001
	if (ASCIImode)
		{
		fprintf(fSTL, "  facet normal 0.0 0.0 1.0\n");
		fprintf(fSTL, "    outer loop\n");
		} // if
	else
		{
		xout = 0.0f; yout = 0.0f, zout = 1.0f;
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		}
	// vert 3
	xout = (float)offset;
	yout = (float)(ymax * scalefactor + offset);
	zout = 0.001f;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		}
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	// vert 4
	xout = (float)(xmax * scalefactor + offset);
	yout = (float)offset;
	zout = (float)offset;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		} // if
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	// vert 5
	xout = (float)offset;
	yout = (float)offset;
	zout = (float)offset;
	if (ASCIImode)
		{
		fprintf(fSTL, "      vertex %f %f %f\n", xout, yout, zout);
		}
	else
		{
		Put32F(LITTLE_END_DATA, &xout, fSTL);
		Put32F(LITTLE_END_DATA, &yout, fSTL);
		Put32F(LITTLE_END_DATA, &zout, fSTL);
		} // else
	if (ASCIImode)
		{
		fprintf(fSTL, "    end loop\n");
		fprintf(fSTL, "  endfacet\n");
		} // if
	else
		{
		Put16U(LITTLE_END_DATA, zero, fSTL);
		}

	if (ASCIImode)
		{
		fprintf(fSTL, "endsolid\n");
		} // if

	fclose(fSTL);
	fSTL = NULL;
	Success = 1;
	} // if fRaw && fSTL

if (Line2)
	AppMem_Free(Line2, LineSize);

if (Line1)
	AppMem_Free(Line1, LineSize);

if (fRaw)
	{
	fclose(fRaw);
	fRaw = NULL;
	PROJ_remove(RawTerrainFile);
	} // if

return Success;

} // ExportFormatSTL::PackageExport
