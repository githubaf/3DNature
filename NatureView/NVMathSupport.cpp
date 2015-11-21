
#include <math.h>
#include "NVMathSupport.h"
#include "Types.h"
#include "Defines.h"

void CopyPoint3d(Point3d To, Point3d From)
{
To[0] = From[0];
To[1] = From[1];
To[2] = From[2];
};

/***********************************************************************/

void CopyPoint3f(Point3f To, Point3f From)
{
To[0] = From[0];
To[1] = From[1];
To[2] = From[2];
};

/***********************************************************************/

void CopyPoint3f3d(Point3f To, Point3d From)
{
To[0] = (float)From[0];
To[1] = (float)From[1];
To[2] = (float)From[2];
};

/***********************************************************************/

void ZeroPoint3d(Point3d A)
{

A[0] = A[1] = A[2] = 0.0;

} /* ZeroPoint3d() */

/***********************************************************************/

void AddPoint3f(Point3f A, Point3f Add)
{

A[0] += Add[0];
A[1] += Add[1];
A[2] += Add[2];

} /* AddPoint3f() */

void FindPosVector(Point3d OP, Point3d EP, Point3d SP)
{

 OP[0] = EP[0] - SP[0];
 OP[1] = EP[1] - SP[1];
 OP[2] = EP[2] - SP[2];

} /* FindPosVector() */

double VectorMagnitude(Point3d TP)
{

return (sqrt(TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2])); 

} /* VectorMagnitude() */



void UnitVector(Point3d UV)
{
double Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0 / Len;
	UV[0] *= Invert;
	UV[1] *= Invert;
	UV[2] *= Invert;
	} /* if */
#ifdef DEBUG
else
	{
	//assert(Len > 0.0);
	} // else
#endif

} /* UnitVector() */



void SurfaceNormal(Point3d NP, Point3d FP, Point3d LP)
{

 NP[0] = FP[1] * LP[2] - FP[2] * LP[1];
 NP[1] = FP[2] * LP[0] - FP[0] * LP[2];
 NP[2] = FP[0] * LP[1] - FP[1] * LP[0];

 UnitVector(NP);

} /* SurfaceNormal() */


void MakeNormal(Point3d NV, Point3d A, Point3d B, Point3d C)
{
static Point3d Side[2];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(NV, Side[0], Side[1]);
//InvertZ(NV);
} // MakeNormal




/* *************** */

void FindPosVector(Point3f OP, Point3f EP, Point3f SP)
{

 OP[0] = EP[0] - SP[0];
 OP[1] = EP[1] - SP[1];
 OP[2] = EP[2] - SP[2];

} /* FindPosVector() */

double VectorMagnitude(Point3f TP)
{

return (sqrt(TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2])); 

} /* VectorMagnitude() */



void UnitVector(Point3f UV)
{
double Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0 / Len;
	UV[0] *= Invert;
	UV[1] *= Invert;
	UV[2] *= Invert;
	} /* if */
#ifdef DEBUG
else
	{
	//assert(Len > 0.0);
	} // else
#endif

} /* UnitVector() */



void SurfaceNormal(Point3f NP, Point3f FP, Point3f LP)
{

 NP[0] = FP[1] * LP[2] - FP[2] * LP[1];
 NP[1] = FP[2] * LP[0] - FP[0] * LP[2];
 NP[2] = FP[0] * LP[1] - FP[1] * LP[0];

 UnitVector(NP);

} /* SurfaceNormal() */


void MakeNormal(Point3f NV, Point3f A, Point3f B, Point3f C)
{
static Point3f Side[2];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(NV, Side[0], Side[1]);
//InvertZ(NV);
} // MakeNormal


/***********************************************************************/
//      0
//      +a
// 270__|__+b 90
//      |
//     180
double findangle3(double pta, double ptb)
{
 double angle;

 if (pta == 0.0)
  {
  angle = ptb > 0.0 ? HalfPi: OneAndHalfPi;
  } // if
 else
  {
  angle = atan(ptb / pta);
  if (pta < 0.0) angle += Pi;
  } // else

 return angle;

} /* findangle3() */


