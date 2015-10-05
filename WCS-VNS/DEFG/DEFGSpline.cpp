// DEFGSpline.cpp
// Made from modified parts of WCS GraphData.cpp
// on July 26 2001 by CXH

#include "../stdafx.h"
#include "DEFGSpline.h"

double GetSplineValueGlobalT(double Dist, double OneMinusT, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode)
{
// input:
// Nodes: PrevPrevNode, PrevNode, NextNode, NextNextNode
// Dist (between PrevNode and NextNode in linear distance)
double P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, Space;

Space = NextNode->Distance - PrevNode->Distance;
if (Space > 0.0)
	{
	P1 = PrevNode->Value;
	P2 = NextNode->Value;
	D1 = ((.5 * (P1 - PrevPrevNode->Value) * OneMinusT) + (.5 * (P2 - P1) * OneMinusT))
		* (2.0 * Space / ((PrevNode->Distance - PrevPrevNode->Distance) + Space));
	D2 = ((.5 * (P2 - P1) * OneMinusT) + (.5 * (NextNextNode->Value - P2) * OneMinusT))
		* (2.0 * Space / (Space + (NextNextNode->Distance - NextNode->Distance)));
	// Dist is now distance from PrevNode, not total dist
	//S1 = (Dist - PrevNode->Distance) / Space;
	S1 = Dist / Space;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if interesting point lies between prev and next
return (PrevNode->Value);		

} // GetSplineValueGlobalT


double GetSplineValueNoTCB(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode)
{
// input:
// Nodes: PrevPrevNode, PrevNode, NextNode, NextNextNode
// Dist (between PrevNode and NextNode in linear distance)
double P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, Space;

Space = NextNode->Distance - PrevNode->Distance;
if (Space > 0.0)
	{
	P1 = PrevNode->Value;
	P2 = NextNode->Value;
	D1 = ((.5 * (P1 - PrevPrevNode->Value)) + (.5 * (P2 - P1)))
		* (2.0 * Space / ((PrevNode->Distance - PrevPrevNode->Distance) + Space));
	D2 = ((.5 * (P2 - P1)) + (.5 * (NextNextNode->Value - P2)))
		* (2.0 * Space / (Space + (NextNextNode->Distance - NextNode->Distance)));
	// Dist is now distance from PrevNode, not total dist
	//S1 = (Dist - PrevNode->Distance) / Space;
	S1 = Dist / Space;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if interesting point lies between prev and next
return (PrevNode->Value);		

} // GetSplineValueNoTCB


#ifdef DEFG_SPLINE_LOCALTCB

double GetSplineValueTOnly(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode)
{
// input:
// Nodes: PrevPrevNode, PrevNode, NextNode, NextNextNode
// Dist (between PrevNode and NextNode in linear distance)
double P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, Space;

Space = NextNode->Distance - PrevNode->Distance;
if (Space > 0.0)
	{
	P1 = PrevNode->Value;
	P2 = NextNode->Value;
	D1 = ((.5 * (P1 - PrevPrevNode->Value) * (1.0 - PrevNode->TCB[0])) + (.5 * (P2 - P1) * (1.0 - PrevNode->TCB[0])))
		* (2.0 * Space / ((PrevNode->Distance - PrevPrevNode->Distance) + Space));
	D2 = ((.5 * (P2 - P1) * (1.0 - NextNode->TCB[0])) + (.5 * (NextNextNode->Value - P2) * (1.0 - NextNode->TCB[0])))
		* (2.0 * Space / (Space + (NextNextNode->Distance - NextNode->Distance)));
	// Dist is now distance from PrevNode, not total dist
	//S1 = (Dist - PrevNode->Distance) / Space;
	S1 = Dist / Space;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if interesting point lies between prev and next
return (PrevNode->Value);		

} // GetSplineValueTOnly


double GetSplineValue(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode)
{
// input:
// Nodes: PrevPrevNode, PrevNode, NextNode, NextNextNode
// Dist (between PrevNode and NextNode in linear distance)
double P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, Space;

Space = NextNode->Distance - PrevNode->Distance;
if (Space > 0.0)
	{
	P1 = PrevNode->Value;
	P2 = NextNode->Value;
	D1 = 
		((.5 * (P1 - PrevPrevNode->Value)
		* (1.0 - PrevNode->TCB[0])
		* (1.0 + PrevNode->TCB[1])
		* (1.0 + PrevNode->TCB[2]))
		+ (.5 * (P2 - P1)
		* (1.0 - PrevNode->TCB[0])
		* (1.0 - PrevNode->TCB[1])
		* (1.0 - PrevNode->TCB[2])))
		* (2.0 * Space / ((PrevNode->Distance - PrevPrevNode->Distance) + Space));

	D2 = 
		((.5 * (P2 - P1)
		* (1.0 - NextNode->TCB[0])
		* (1.0 - NextNode->TCB[1])
		* (1.0 + NextNode->TCB[2]))
		+ (.5 * (NextNextNode->Value - P2)
		* (1.0 - NextNode->TCB[0])
		* (1.0 + NextNode->TCB[1])
		* (1.0 - NextNode->TCB[2])))
		* (2.0 * Space / (Space + (NextNextNode->Distance - NextNode->Distance)));

	// Dist is now distance from PrevNode, not total dist
	//S1 = (Dist - PrevNode->Distance) / Space;
	S1 = Dist / Space;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
	h2 = -2.0 * S3 + 3.0 * S2;
	h3 = S3 - 2.0 * S2 + S1;
	h4 = S3 - S2;
	return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
	} // if interesting point lies between prev and next
return (PrevNode->Value);		

} // GetSplineValue


#endif // DEFG_SPLINE_LOCALTCB
