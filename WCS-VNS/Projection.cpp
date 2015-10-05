// Projection.cpp
// Coordinate transforms for various map projections
// Built from v1 dlg.c on 07 Jun 1995 by Chris "Xenon" Hanson
// Original code written by Gary R. Huber, Jamuary, 1994.
// Southern Hemisphere UTM fixed - 03/01/00 FPW2
// Copyright 2000 by 3D Nature

/*** F2 NOTE: UTM zone irregularities not handled - shouldn't be a problem, but probably should be accounted for
     in VNS.  Specifically "There are special UTM zones between 0 degrees and 36 degrees longitude above 72 degrees
	 latitude and a special zone 32 between 56 degrees and 64 degrees north latitude." (from Peter Dana's webpage
	 at http://www.colorado.Edu/geography/gcraft/notes/coordsys/coordsys_f.html) ***/

#include "stdafx.h"
#include "Projection.h"
#include "Requester.h"
#include "UsefulMath.h"

void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone)
{

Coords->a 	= 6378206.4;
Coords->e_sq 	= 0.00676866;
Coords->k_0 	= 0.9996;
Coords->M_0 	= 0.0;
Coords->lam_0	= (double)(183 - abs(UTMZone) * 6);	//lint !e790

Coords->e_pr_sq 	= Coords->e_sq / (1.0 - Coords->e_sq);
Coords->e_sq_sq 	= Coords->e_sq * Coords->e_sq;
Coords->e_1 		= (1.0 - sqrt(1.0 - Coords->e_sq)) / (1.0 + sqrt(1.0 - Coords->e_sq));
Coords->e_1_sq 	= Coords->e_1 * Coords->e_1;

if (UTMZone < 0)
	Coords->SouthHemi = 1;
else
	Coords->SouthHemi = 0;

} // UTMLatLonCoords_Init

/*===========================================================================*/

void AlbLatLonCoords_Init(struct AlbLatLonCoords *Coords)
{
double phi_0, phi_1, phi_2, m_1, m_2, q_1, q_2, q_0,
	sin_phi_0, sin_phi_1, sin_phi_2, sin_sq_phi_0, sin_sq_phi_1, sin_sq_phi_2;

Coords->a	  = Coords->ProjPar[0];
Coords->e_sq  = Coords->ProjPar[1];
Coords->e	  = sqrt(Coords->e_sq);
Coords->two_e = 2.0 * Coords->e;
Coords->lam_0 = DegMinSecToDegrees2(Coords->ProjPar[4]);
phi_0 = DegMinSecToDegrees2(Coords->ProjPar[5]) * PiOver180;
phi_1 = DegMinSecToDegrees2(Coords->ProjPar[2]) * PiOver180;
phi_2 = DegMinSecToDegrees2(Coords->ProjPar[3]) * PiOver180;

sin_phi_0 = sin(phi_0);  
sin_phi_1 = sin(phi_1);  
sin_phi_2 = sin(phi_2);  
sin_sq_phi_0 = sin_phi_0 * sin_phi_0;
sin_sq_phi_1 = sin_phi_1 * sin_phi_1;
sin_sq_phi_2 = sin_phi_2 * sin_phi_2;

m_1 = cos(phi_1) / sqrt(1.0 - Coords->e_sq * sin_sq_phi_1);
m_2 = cos(phi_2) / sqrt(1.0 - Coords->e_sq * sin_sq_phi_2);

q_0 = (1.0 - Coords->e_sq) * (sin_phi_0 / (1.0 - Coords->e_sq * sin_sq_phi_0)
							  - (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_0)
							  / (1.0 + Coords->e * sin_phi_0)));
q_1 = (1.0 - Coords->e_sq) * (sin_phi_1 / (1.0 - Coords->e_sq * sin_sq_phi_1)
							  - (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_1)
							  / (1.0 + Coords->e * sin_phi_1)));
q_2 = (1.0 - Coords->e_sq) * (sin_phi_2 / (1.0 - Coords->e_sq * sin_sq_phi_2)
							  - (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_2)
							  / (1.0 + Coords->e * sin_phi_2)));
Coords->n	  = (m_1 * m_1 - m_2 * m_2) / (q_2 - q_1);
Coords->C	  = m_1 * m_1 + Coords->n * q_1;
Coords->rho_0 = Coords->a * sqrt(Coords->C - Coords->n * q_0) / Coords->n;

} // AlbLatLonCoords_Init

/*===========================================================================*/

void UTM_LatLon(struct UTMLatLonCoords *Coords)
{
double x, y, M, mu, phi_1, C_1, T_1, N_1, R_1, D_1,
	cos_phi_1, tan_phi_1, sin_phi_1, sin_sq_phi_1, C_1_sq, T_1_sq, D_1_sq;

 x 		= Coords->East;
 if (Coords->SouthHemi)
	 y = Coords->North - 10000000;
 else
	 y = Coords->North;
 x 		-= 500000.0;
 M 		= Coords->M_0 + y / Coords->k_0;
 mu = M / (Coords->a * (1.0 - Coords->e_sq / 4.0 - 3.0 * Coords->e_sq_sq
	/ 64 - 5.0 * Coords->e_sq * Coords->e_sq_sq / 256.0));
 phi_1 = (mu + (3.0 * Coords->e_1 / 2.0 - 27.0 * Coords->e_1 * Coords->e_1_sq / 32.0)
	* sin(2.0 * mu)
	+ (21.0 * Coords->e_1_sq / 16.0 - 55.0 * Coords->e_1_sq * Coords->e_1_sq / 32.0)
	* sin(4.0 * mu)
	+ (151.0 * Coords->e_1 * Coords->e_1_sq / 96.0) * sin(6.0 * mu)); // radians
 cos_phi_1 	= cos(phi_1);
 tan_phi_1 	= tan(phi_1);
 sin_phi_1 	= sin(phi_1);
 sin_sq_phi_1	= sin_phi_1 * sin_phi_1;
 phi_1 		*= PiUnder180;	// degrees
 C_1 		= Coords->e_pr_sq * cos_phi_1 * cos_phi_1;
 C_1_sq 	= C_1 * C_1;
 T_1 		= tan_phi_1 * tan_phi_1;
 T_1_sq 	= T_1 * T_1;
 N_1 		= Coords->a / sqrt(1.0 - Coords->e_sq * sin_sq_phi_1);
 R_1 		= Coords->a * (1.0 - Coords->e_sq) / pow((1.0 - Coords->e_sq * sin_sq_phi_1), 1.5);
 D_1		= x / (N_1 * Coords->k_0);
 D_1_sq 	= D_1 * D_1;

 Coords->Lat = phi_1 - (N_1 * tan_phi_1 / R_1) *
	(D_1_sq / 2.0 
	- (5.0 + 3.0 * T_1 + 10.0 * C_1 - 4.0 * C_1_sq - 9.0 * Coords->e_pr_sq)
	* D_1_sq * D_1_sq / 24.0
	+ (61.0 + 90.0 * T_1 + 298.0 * C_1 + 45.0 * T_1_sq - 252.0 * Coords->e_pr_sq
		- 3.0 * C_1_sq)
	* D_1_sq * D_1_sq * D_1_sq / 720.0)
	* PiUnder180;	// degrees 

/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */

Coords->Lon = Coords->lam_0 - ((D_1 - (1.0 + 2.0 * T_1 + C_1) * D_1 * D_1_sq / 6.0
							   + (5.0 - 2.0 * C_1 + 28.0 * T_1 - 3.0 * C_1_sq + 8.0 * Coords->e_pr_sq
							   + 24.0 * T_1_sq)
							   * D_1 * D_1_sq * D_1_sq / 120.0) / cos_phi_1) * PiUnder180; // degrees

} // UTM_LatLon

/*===========================================================================*/

void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone)
{
 double a, e_sq, k_0, lam_0, M_0, x, y, e_pr_sq, M,
	C, T, N, phi, lam, phi_rad,
	cos_phi, sin_phi, tan_phi, A, A_sq, A_cu, A_fo, A_fi, A_si, T_sq;

 a 	= 6378206.4;
 e_sq 	= 0.00676866;
 k_0 	= 0.9996;
 M_0 	= 0.0;
 phi 	= Coords->Lat;
 lam 	= -Coords->Lon;	// change sign to conform with GIS standard
 lam_0 	= -(double)(183 - abs(UTMZone) * 6); //lint !e790
 phi_rad = phi * PiOver180;

 e_pr_sq = e_sq / (1.0 - e_sq);
 sin_phi = sin(phi_rad);

// use a very large number for tan_phi if tan(phi) is undefined

 tan_phi = fabs(phi) == 90.0 ? (phi >= 0.0 ? 1.0: -1.0) * FLT_MAX / 100.0:
	 tan(phi_rad);
 cos_phi = cos(phi_rad);

 N	= a / sqrt(1.0 - e_sq * sin_phi * sin_phi);
 T	= tan_phi * tan_phi;
 T_sq	= T * T;
 C	= e_pr_sq * cos_phi * cos_phi;
 A	= cos_phi * (lam - lam_0) * PiOver180;
 A_sq	= A * A;
 A_cu	= A * A_sq;
 A_fo	= A * A_cu;
 A_fi	= A * A_fo;
 A_si	= A * A_fi;
 M	= 111132.0894 * phi - 16216.94 * sin (2.0 * phi_rad)
	 + 17.21 * sin(4.0 * phi_rad) - .02 * sin(6.0 * phi_rad);
 x	= k_0 * N * (A + ((1.0 - T + C) * A_cu / 6.0)
	  + ((5.0 - 18.0 * T + T_sq + 72.0 * C - 58.0 * e_pr_sq) * A_fi / 120.0)
	  ) + 500000;
 y	= k_0 * (M - M_0 + N * tan_phi *
	   (A_sq / 2.0 + ((5.0 - T + 9.0 * C + 4.0 * C * C) * A_fo / 24.0)
	   + ((61.0 - 58.0 * T + T_sq + 600.0 * C - 330.0 * e_pr_sq) * A_si / 720.0)
	  ));

/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */

 if (UTMZone < 0)
	 Coords->North = 10000000 + y;
 else
	 Coords->North = y;
 Coords->East  = x;
 

} // LatLon_UTM

/*===========================================================================*/

void Alb_LatLon(struct AlbLatLonCoords *Coords)
{
double x, y, phi, theta, rho, q, phi_rad, sin_phi, sin_sq_phi, delta_phi;
short i;

x = Coords->East;
y = Coords->North;

if (Coords->n < 0.0)
	theta = atan(-x / (y - Coords->rho_0));
else
	theta = atan(x / (Coords->rho_0 - y));

rho = sqrt(x * x + (Coords->rho_0 - y) * (Coords->rho_0 - y));

q = (Coords->C - (rho * rho * Coords->n * Coords->n)
	 / (Coords->a * Coords->a)) / Coords->n;

phi_rad = asin(q / 2.0);
phi = phi_rad * PiUnder180;

for (i=0, delta_phi=10.0; fabs(delta_phi)>.00001 && i<10; i++)
	{
	sin_phi = sin(phi_rad);
	sin_sq_phi = sin_phi * sin_phi;
	delta_phi = ((1.0 - Coords->e_sq * sin_sq_phi) * (1.0 - Coords->e_sq * sin_sq_phi) 
		/ (2.0 * cos(phi_rad))) 
	* (q / (1.0 - Coords->e_sq) 
	- sin_phi / (1.0 - Coords->e_sq * sin_sq_phi) 
	+ (1.0 / Coords->two_e) 
	* log((1.0 - Coords->e * sin_phi) / (1.0 + Coords->e * sin_phi))) * PiUnder180;
	phi += delta_phi;
	phi_rad = phi * PiOver180;
	} // for
if (i >= 10)
	phi = 90.0 * (q >= 0.0 ? 1.0: -1.0);

Coords->Lat = phi;

// change sign for WCS convention
Coords->Lon = -(Coords->lam_0 + PiUnder180 * theta / Coords->n);

} // Alb_LatLon

/*===========================================================================*/

double FCvt(const char *string)
{
short i = 0, j = 0;
char base[64];
char expon[12];

base[0] = expon[0] = 0; 

while (string[i] && (isdigit((unsigned char)string[i]) || string[i] == '.'
 || string[i] == ' ' || string[i] == '-' || string[i] == '+' ))
	{
	base[i] = string[i];
	++i;
	} // while not alphabet character
base[i] = 0;

if (string[i])
	{
	++i;

	while (string[i] && i < 63)
		{
		expon[j] = string[i];
		++i;
		++j;
		} // while
	expon[j] = 0;
	} // if

return (atof(base) * pow(10.0, (double)atoi(expon)));

} // FCvt

/*===========================================================================*/

// For converting a coordinate string of the form dddmmss to decimal degrees
double DegMinSecToDegrees2(double Val)
{
long Deg, Min, DegMinSec;

DegMinSec = quickftol(Val);

Deg = DegMinSec / 1000000;
DegMinSec %= 1000000;
Min = DegMinSec / 1000;
DegMinSec %= 1000;

return ((double)Deg + (double)Min / 60.0 + (double)DegMinSec / 3600.0);
 
} // DegMinSecToDegrees2

/*===========================================================================*/

double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, float *Data, long Rows, long Cols)
{
double RemX = 0.0, RemY = 0.0, P1, P2;
long Row, Col, Col_p, Row_p, TrueRow, TrueCol;

Row = TrueRow = quickftol(((Y - MinY) / IntY));
Col = TrueCol = quickftol(((X - MinX) / IntX));
if (Row < 0)
	Row = 0;
else if (Row >= Rows)
	Row = Rows - 1;
if (Col < 0)
	Col = 0;
if (Col >= Cols)
	Col = Cols - 1;

Row_p = Row;
Col_p = Col;

if (Row < Rows - 1 && TrueRow >= 0)
	{
	RemY = (Y - (Row * IntY + MinY)) / IntY;
	if (RemY < 0.0)
	RemY = 0.0;
	Row_p ++;
	} // if not last row
	if (Col < Cols - 1 && TrueCol >= 0)
	{ 
	RemX = (X - (Col * IntX + MinX)) / IntX;
	if (RemX < 0.0)
	RemX = 0.0;
	Col_p ++;
	} // if not last column

P1 = RemX * (Data[Col_p * Rows + Row] - Data[Col * Rows + Row])
	+ Data[Col * Rows + Row];
P2 = RemX * (Data[Col_p * Rows + Row_p] - Data[Col * Rows + Row_p])
	+ Data[Col * Rows + Row_p];

return ( RemY * (P2 - P1) + P1 );

} // Point_Extract
