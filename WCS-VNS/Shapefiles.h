// Shapefiles.h
// Header file for newest Shapefile code
// Created from scratch on 01/26/05 by Frank Weed II
// Copyright 2005 3D Nature, LLC. All rights reserved.

#ifndef WCS_SHAPEFILES_H
#define WCS_SHAPEFILES_H

class ShapeHdr;

enum ShapeTypes
{
SHP_Null_Shape = 0,
SHP_Point = 1,
SHP_PolyLine = 3,
SHP_Polygon = 5,
SHP_MultiPoint = 8,
SHP_PointZ = 11,
SHP_PolyLineZ = 13,
SHP_PolygonZ = 15,
SHP_MultiPointZ = 18,
SHP_PointM = 21,
SHP_PolyLineM = 23,
SHP_PolygonM = 25,
SHP_MultiPointM = 28,
SHP_MultiPatch = 31
};

class Shapefile {
protected:
	long Offset, Record;
	ShapeTypes ShapeType;
	FILE *fMain, *fIndex, *fdBase;
	ShapeHdr *Hdr;
public:
	Shapefile(ShapeTypes Type) {Offset = 50; Record = 1; ShapeType = Type; };
	~Shapefile();
	FILE* Get_dBase_Handle() { return fdBase; };
	FILE* Get_Shape_Handle() { return fMain; };
	FILE* Get_Index_Handle() { return fIndex; };
	long Init(char *Path, char *Basename);
	long NewRecord(long SizeInBytes);
	void Wrapup();

};	// Shapefile

class ShapeHdr {
	long FileCode, Unused1, Unused2, Unused3, Unused4, Unused5, Version;
	ShapeTypes Type;
	double Xmin, Ymin, Xmax, Ymax, Zmin, Zmax, Mmin, Mmax;
	FILE *dBase, *Index, *Shape;
public:
	ShapeHdr(Shapefile* ShapeObj, ShapeTypes Type);
	~ShapeHdr();
	void UpdateBounds(double *xmin, double *xmax, double *ymin, double *ymax, double *zmin, double *zmax, double *mmin, double *mmax);
	long Write(long FileLen);
};	// ShapeHdr

class Shapefile3D: public Shapefile {
	double X, Y, Z, M;
public:
	Shapefile3D(ShapeTypes Type) : Shapefile(Type) {X = Y = Z = M = 0.0;};
	~Shapefile3D();
	void SetX(double x) {X = x; };
	void SetY(double y) {Y = y; };
	void SetZ(double z) {Z = z; };
	void SetM(double m) {M = m; };
	long WritePointZ(void);
};	// Shapefile3D

#endif // WCS_SHAPEFILES_H
