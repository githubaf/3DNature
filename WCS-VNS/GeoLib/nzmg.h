#ifndef NZMG_H
#define NZMG_H

// remapped these variables to GCTP names
// a -> r_major
// n0 -> false_northing
// e0 -> false_easting
// lt0 -> central_lat
// ln0 -> central_lon

typedef struct { double real, imag; } geolib_complex;

//  Coefficients for converting NZMG coordinates to longitude and latitude
//  From L+S technical note set 1973/32

static double nzmg_cfi[] = {   0.6399175073,
                         -0.1358797613,
                           0.063294409,
                           -0.02526853,
                             0.0117879,
                            -0.0055161,
                             0.0026906,
                             -0.001333,
                               0.00067,
                              -0.00034 };

static double nzmg_cfl[] = {   1.5627014243,
                          0.5185406398,
                           -0.03333098,
                            -0.1052906,
                            -0.0368594,
			      0.007317,
			       0.01220,
			       0.00394,
			       -0.0013 };

static geolib_complex nzmg_cfb1[] = { {0.7557853228,           0.0},
			  { 0.249204646,   0.003371507},
			  {-0.001541739,   0.041058560},
			  { -0.10162907,    0.01727609},
			  { -0.26623489,   -0.36249218},
			  {  -0.6870983,    -1.1651967} };

static geolib_complex nzmg_cfb2[] = { {1.3231270439,           0.0},
			  {-0.577245789,  -0.007809598},
			  { 0.508307513,  -0.112208952},
			  { -0.15094762,    0.18200602},
			  {  1.01418179,    1.64497696},
			  {   1.9660549,     2.5127645} };


/*----------------------------------------------------------------*/
/*                                                                */
/*  Basic complex arithmetic routines                             */
/*                                                                */
/*----------------------------------------------------------------*/

static geolib_complex *cadd(geolib_complex *cr, geolib_complex *c1, geolib_complex *c2) {
   cr->real = c1->real + c2->real;
   cr->imag = c1->imag + c2->imag;
   return cr;
   }

static geolib_complex *csub(geolib_complex *cr, geolib_complex *c1, geolib_complex *c2) {
   cr->real = c1->real - c2->real;
   cr->imag = c1->imag - c2->imag;
   return cr;
   }

static geolib_complex *cmult(geolib_complex *cr, geolib_complex *c1, geolib_complex *c2) {
   geolib_complex temp;
   temp.real = c1->real * c2->real - c1->imag * c2->imag;
   temp.imag = c1->real * c2->imag + c1->imag * c2->real;
   cr->real = temp.real;
   cr->imag = temp.imag;
   return cr;
   }

static geolib_complex *cdiv(geolib_complex *cr, geolib_complex *c1, geolib_complex *c2) {
   geolib_complex temp;
   double cmod2;
   cmod2 = (c2->real*c2->real + c2->imag*c2->imag);
   temp.real = c2->real/cmod2;
   temp.imag = -c2->imag/cmod2;
   cmult( cr, c1, &temp );
   return cr;
   }

static geolib_complex *cscale(geolib_complex *cr, geolib_complex *c1, double sc) {
   cr->real = c1->real * sc;
   cr->imag = c1->imag * sc;
   return cr;
   }

#endif // NZMG_H
