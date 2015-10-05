// rd.h - header for Geolib RD / Netherlands New routines
//
// Created 02/16/04 FPW2

#ifndef RD_H
#define RD_H

static double rd_fe = 155000.0;
static double rd_fn = 463000.0;
static double rd_lat_origin = 52.1551744;
static double rd_lon_origin = 5.38720621;

static unsigned char RD_K_P[] =
	{ 0, 2, 0, 2, 0, 2, 1, 4, 2, 4, 1 };

static unsigned char RD_K_Q[] =
	{ 1, 0, 2, 1, 3, 2, 0, 0, 3, 1, 1 };

static double RD_Kpq[] =
	{ 3235.65389, -32.58297, -0.24750, -0.84978, -0.06550,
	-0.01709, -0.00738, 0.00530, -0.00039, 0.00033, -0.00012 };

static unsigned char RD_L_P[] =
	{ 1, 1, 1, 3, 1, 3, 0, 3, 1, 0, 2, 5 };

static unsigned char RD_L_Q[] =
	{ 0, 1, 2, 0, 3, 1, 1, 2, 4, 2, 0, 0 };

static double RD_Lpq[] =
	{ 5260.52916, 105.94684, 2.45656, -0.81885, 0.05594, -0.05607,
	0.01199, -0.00256, 0.00128, 0.00022, -0.00022, 0.00026 };

static unsigned char RD_R_P[] =
	{ 0, 1, 2, 0, 1, 3, 1, 0, 2 };

static unsigned char RD_R_Q[] =
	{ 1, 1, 1, 3, 0, 1, 3, 2, 3 };

static double RD_Rpq[] =
	{ 190094.945, -11832.228, -114.221, -32.391,
	-0.705, -2.340, -0.608, -0.008, 0.148 };

static unsigned char RD_S_P[] =
	{ 1, 0, 2, 1, 3, 0, 2, 1, 0, 1 };

static unsigned char RD_S_Q[] =
	{ 0, 2, 0, 2, 0, 1, 2, 1, 4, 4 };

static double RD_Spq[] =
	{ 309056.544, 3638.893, 73.077, -157.984, 59.788,
	0.433, -6.439, -0.032, 0.092, -0.054 };

#endif // RD_H
