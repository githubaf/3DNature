// NNGrid.h
// Header file for DEM Gridding
// Created from WCS.h on 7/5/96 by GRH, adapted from Dave Watson
// Modified 3/15/00 by Gary R. Huber.
// Copyright 1996-2000 Questar Productions

// Primarily superceded by DEFG by CXH, but DEFG uses NNG's data structures, so we keep bits of it around.


#ifndef WCS_NNGRID_H
#define WCS_NNGRID_H

class ControlPointDatum
	{
	public:
		double       values[3];
		ControlPointDatum *nextdat;
		char LinkToLast;

		ControlPointDatum() {values[0] = values[1] = values[2] = 0.0; nextdat = NULL; LinkToLast = 0;};
		ControlPointDatum(double Lat, double Lon, double Elev) {values[0] = Lon; values[1] = Lat; values[2] = Elev; nextdat = NULL; LinkToLast = 0;};
		void SetValues(double Lat, double Lon, double Elev) {values[0] = Lon; values[1] = Lat; values[2] = Elev; };
		void DeleteAllChildren(void);
	}; // ControlPointDatum


class NNGrid
	{
	public:

		double  xstart, ystart, xterm, yterm, horilap, 
			vertlap, bI, bJ, nuldat; // bJ no longer used but loaded, saved, copied
		long     igrad, x_nodes, y_nodes, non_neg, // igrad, non_neg no longer used, but loaded, saved, copied
			extrap;
		char    grd_dir[256], grd_file[64]; 

		NNGrid();
		~NNGrid() {}; // nothing to do
	};


#endif // WCS_NNGRID_H
