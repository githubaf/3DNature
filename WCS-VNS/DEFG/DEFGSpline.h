// DEFGSpline.h
// Made from modified parts of WCS GraphData.h
// on July 26 2001 by CXH

//#define DEFG_SPLINE_LOCALTCB

class SimpleGraphNode
	{
	public:
		double Value, Distance;
		#ifdef DEFG_SPLINE_LOCALTCB
		double TCB[3];
		#endif // DEFG_SPLINE_LOCALTCB
	}; // class SimpleGraphNode

double GetSplineValueGlobalT(double Dist, double OneMinusT, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode);
double GetSplineValueNoTCB(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode);

#ifdef DEFG_SPLINE_LOCALTCB
double GetSplineValueTOnly(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode);
double GetSplineValue(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode);
double GetSplineValueNoTCB(double Dist, SimpleGraphNode *PrevPrevNode, SimpleGraphNode *PrevNode, SimpleGraphNode *NextNode, SimpleGraphNode *NextNextNode);
#endif // DEFG_SPLINE_LOCALTCB

