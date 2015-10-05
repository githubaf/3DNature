// Shapefiles.cpp
// Code module for shapefile code
// Created on 02/01/05 by FPW2
// Copyright 2004 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "PathAndFile.h"
#include "Project.h"
#include "Shapefiles.h"
#include "UsefulIO.h"

/*===========================================================================*/

Shapefile::~Shapefile(void)
{

if (Hdr)
	delete Hdr;

if (fdBase)
	fclose(fdBase);

if (fIndex)
	fclose(fIndex);

if (fMain)
	fclose(fMain);

} // Shapefile::~Shapefile

/*===========================================================================*/

long Shapefile::Init(char *Path, char *Basename)
{
PathAndFile PaF;
long Err = 1;
char fName[WCS_PATHANDFILE_PATH_LEN + WCS_PATHANDFILE_NAME_LEN];

PaF.SetPath(Path);
PaF.SetName(Basename);
PaF.GetPathAndName(fName);

strcat(fName, ".shp");
fMain = PROJ_fopen(fName, "wb");

PaF.GetPathAndName(fName);
strcat(fName, ".shx");
fIndex = PROJ_fopen(fName, "wb");

PaF.GetPathAndName(fName);
strcat(fName, ".dbf");
fdBase = PROJ_fopen(fName, "wb");

Hdr = new ShapeHdr(this, ShapeType);

if (Hdr && fMain && fIndex && fdBase)
	{
	Hdr->Write(this->Offset);
	Err = 0;
	} // if

return Err;
} // Shapefile::Init

long Shapefile::NewRecord(long SizeInBytes)
{
long RC;
long Words = SizeInBytes / 2;

PutB32S(Record, fMain);
PutB32S(Words, fMain);
PutB32S(Offset, fIndex);
RC = PutB32S(Words, fIndex);

Record++;
Offset += Words + 4;

return RC;
} // Shapefile::NewRecord

void Shapefile::Wrapup(void)
{
Hdr->Write(this->Offset);
if (fdBase)
	{
	fputc(0x1a, fdBase);			// write dBase end-of-file marker
	fseek(fdBase, 4, SEEK_SET);		// fix # records field
	PutL32S((Record - 1), fdBase);
	fclose(fdBase);
	fdBase = NULL;
	} // if
fclose(fIndex);
fIndex = NULL;
fclose(fMain);
fMain = NULL;
} // Shapefiel::Wrapup

/*===========================================================================*/

ShapeHdr::ShapeHdr(Shapefile* ShapeObj, ShapeTypes ShapeType)
{
FileCode = 9994;
Unused1 = Unused2 = Unused3 = Unused4 = Unused5 = 0;
Version = 1000;
Type = ShapeType;
Xmin = Ymin = Zmin = Mmin = FLT_MAX;
Xmax = Ymax = Zmax = Mmax = -FLT_MAX;

dBase = ShapeObj->Get_dBase_Handle();
Index = ShapeObj->Get_Index_Handle();
Shape = ShapeObj->Get_Shape_Handle();
} // ShapeHdr::ShapeHdr

ShapeHdr::~ShapeHdr()
{
} // ShapeHdr::~ShapeHdr

long ShapeHdr::Write(long FileLen)
{
long NdxFileLen, RC;

rewind(Shape);
PutB32S(FileCode,  Shape);
PutB32S(Unused1,  Shape);
PutB32S(Unused2,  Shape);
PutB32S(Unused3,  Shape);
PutB32S(Unused4,  Shape);
PutB32S(Unused5,  Shape);
PutB32S(FileLen,  Shape);
PutL32S(Version,  Shape);
PutL32S(Type,  Shape);
PutL64(&Xmin, Shape);
PutL64(&Ymin, Shape);
PutL64(&Xmax, Shape);
PutL64(&Ymax, Shape);
PutL64(&Zmin, Shape);
PutL64(&Zmax, Shape);
PutL64(&Mmin, Shape);
PutL64(&Mmax, Shape);

(void)fseek(Index, 0, SEEK_END);
NdxFileLen = ftell(Index) / 2;
rewind(Index);

PutB32S(FileCode,  Index);
PutB32S(Unused1,  Index);
PutB32S(Unused2,  Index);
PutB32S(Unused3,  Index);
PutB32S(Unused4,  Index);
PutB32S(Unused5,  Index);
PutB32S(NdxFileLen,  Index);
PutL32S(Version,  Index);
PutL32S(Type,  Index);
PutL64(&Xmin, Index);
PutL64(&Ymin, Index);
PutL64(&Xmax, Index);
PutL64(&Ymax, Index);
PutL64(&Zmin, Index);
PutL64(&Zmax, Index);
PutL64(&Mmin, Index);
RC = PutL64(&Mmax, Index);

return RC;
} // ShapeHdr::Write

void ShapeHdr::UpdateBounds(double *xmin, double *xmax, double *ymin, double *ymax, double *zmin, double *zmax, double *mmin, double *mmax)
{
if (*xmin < Xmin)
	Xmin = *xmin;

if (*xmax > Xmax)
	Xmax = *xmax;

if (*ymin < Ymin)
	Ymin = *ymin;

if (*ymax > Ymax)
	Ymax = *ymax;

if (*zmin < Zmin)
	Zmin = *zmin;

if (*zmax > Zmax)
	Zmax = *zmax;

if (*mmin < Mmin)
	Mmin = *mmin;

if (*mmax > Mmax)
	Mmax = *mmax;

} // ShapeHdr::UpdateBounds

/*===========================================================================*/

long Shapefile3D::WritePointZ(void)
{
long RC;

NewRecord(36);	// size of this record only in bytes
Hdr->UpdateBounds(&X, &X, &Y, &Y, &Z, &Z, &M, &M);

PutL32S(SHP_PointZ, fMain);
PutL64(&X, fMain);
PutL64(&Y, fMain);
PutL64(&Z, fMain);
RC = PutL64(&M, fMain);

return RC;
} // Shapefile3D::Write

/*===========================================================================*/

Shapefile3D::~Shapefile3D()
{
} // Shapefile3D::~Shapefile3D

/*===========================================================================*/
