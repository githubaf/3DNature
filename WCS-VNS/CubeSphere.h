// CubeSphere.h
// Cubic panoramic to spherical or hemispherical texture mapping
// Created from scratch Jul 17, 2003 by CXH
// Copyright 2003

// faces are numbered:
enum
	{ // more readable
	STC_CUBEFACENAME_TOP = 0,
	STC_CUBEFACENAME_FRONT_NORTH,
	STC_CUBEFACENAME_LEFT_WEST,
	STC_CUBEFACENAME_RIGHT_EAST,
	STC_CUBEFACENAME_REAR_SOUTH,
	STC_CUBEFACENAME_BOT
	}; // cubefaces

enum
	{ // more readable
	STC_COORDINDEX_X = 0,
	STC_COORDINDEX_Y,
	STC_COORDINDEX_Z
	}; // coordindex


// returns 1 for success or 0 for error, In and Out variables are named in prototype
int SphereToCube(double InLon, double InLat, unsigned char &OutCubeFace, double &OutX, double &OutY);

void MaximizeToXAxis(double *XYZIn, double Mag, double *MaxOut);
void MaximizeToYAxis(double *XYZIn, double Mag, double *MaxOut);
void MaximizeToZAxis(double *XYZIn, double Mag, double *MaxOut);
