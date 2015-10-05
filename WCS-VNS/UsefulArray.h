// UsefulArray.h
// Array related code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULARRAY_H
#define WCS_USEFULARRAY_H

// These create and destroy image row indices, and will probably be
// moved into the Image class stuff, once we get that
// organized.
long *Zip_New(long Width, long Height);
void Zip_Del(long *Me, long Width, long Height);

double ArrayPointExtract(float *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows);

double ArrayPointExtract(short *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows);

#endif // WCS_USEFULARRAY_H
