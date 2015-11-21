// NVMathSupport.h

#include "Types.h"

void CopyPoint3d(Point3d To, Point3d From);
void CopyPoint3f(Point3f To, Point3f From);
void CopyPoint3f3d(Point3f To, Point3d From);
void ZeroPoint3d(Point3d A);
void AddPoint3f(Point3f A, Point3f Add);
void FindPosVector(Point3d OP, Point3d EP, Point3d SP);
double VectorMagnitude(Point3d TP);
void UnitVector(Point3d UV);
void SurfaceNormal(Point3d NP, Point3d FP, Point3d LP);
void MakeNormal(Point3d NV, Point3d A, Point3d B, Point3d C);
void FindPosVector(Point3f OP, Point3f EP, Point3f SP);
double VectorMagnitude(Point3f TP);
void UnitVector(Point3f UV);
void SurfaceNormal(Point3f NP, Point3f FP, Point3f LP);
void MakeNormal(Point3f NV, Point3f A, Point3f B, Point3f C);
double findangle3(double pta, double ptb);

