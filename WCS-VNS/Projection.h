// Projection.h
// Coordinate transforms for various map projections
// Built from v1 dlg.c on 07 Jun 1995 by Chris "Xenon" Hanson
// Original code written by Gary R. Huber, Jamuary, 1994.
// Copyright 1995

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_PROJECTION_H
#define WCS_PROJECTION_H

struct UTMLatLonCoords {
 double	North, East, Lat, Lon;
 double	RefNorth[4], RefEast[4], RefLat[4], RefLon[4];
 double a, e_sq, k_0, M_0, lam_0, e_pr_sq, e_sq_sq, e_1, e_1_sq;
 char   SouthHemi;
};

struct AlbLatLonCoords {
 double	North, East, Lat, Lon;
 double	RefNorth[4], RefEast[4], RefLat[4], RefLon[4];
 double ProjPar[8];
 double a, e_sq, e, two_e, C, n, rho_0, lam_0;
};

union MapProjection {
 struct UTMLatLonCoords UTM;
 struct AlbLatLonCoords Alb;
};

struct DEMInfo {
 short	Code,
	Zone,
	HUnits,
	VUnits;
 double Res[3],
	UTM[4][2],
	LL[4][2];
};

#define PiUnder180 5.72957795130823E+001
#define PiOver180 1.74532925199433E-002

void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone);
void AlbLatLonCoords_Init(struct AlbLatLonCoords *Coords);

void Alb_LatLon(struct AlbLatLonCoords *Coords);
void UTM_LatLon(struct UTMLatLonCoords *Coords);
void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone);

double FCvt(const char *string);
double DegMinSecToDegrees2(double Val);

double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, float *Data, long Rows, long Cols);

#endif // WCS_PROJECTION_H
