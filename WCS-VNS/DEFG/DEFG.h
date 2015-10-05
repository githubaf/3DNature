// DEFG.h
// Header for DEFG
// Built from scratch and from parts of DEFG.cpp on
// Fri Aug 24, 2001 by CXH

#if defined(WCS_BUILD_VNS) || defined(WCS_BUILD_W6)
#include "../Types.h"
#endif // VNS or WCS6+

#define DEFG_ENABLE_DIAG

#define DEFG_SPAN_MULT			8
#define DEFG_GAPFILL_HORIZ_WEIGHT 1.0f
#define DEFG_GAPFILL_VERT_WEIGHT 1.0f
#define DEFG_GAPFILL_DIAG_WEIGHT .7071f
#define DEFG_NULL_VALUE			-9999.0f
#define DEFG_NORMAL_BINS		36
#define DEFG_NORMAL_BINMOD		(360 / DEFG_NORMAL_BINS)

#define DEFGPi     3.1415926535898
#define DEFGHalfPi 1.5707963267949
#define DEFGTwoPi  6.2831853071796
#define DEFGOneAndHalfPi 4.7123889803847
#define DEFGPiOver180 1.74532925199433E-002
#define DEFGPiUnder180 5.72957795130823E+001

#define DEFG_SPANBLOCK_QUANTA	10

#define simplemin(a,b)            (((a) < (b)) ? (a) : (b))
#define simplemax(a,b)            (((a) > (b)) ? (a) : (b))

struct Span;
class ControlPointDatum;
class Database;
class NNGrid;
class CoordSys;
class TerraGridder;
class Joe;
class Project;
class EffectsLib;

/*
	double X, Y, Z;
	signed long int PointNum;
	char Disabled;
	int NumSpans, MaxSpans;
	struct Span **SpanBlock;
*/
#define DEFG_INPOINT_DEF 	double X, Y, Z; signed long int PointNum; char Disabled; int NumSpans, MaxSpans; struct Span **SpanBlock;

struct InputPoint
	{
	DEFG_INPOINT_DEF
	}; // InputPoint

struct SmoothedInputPoint
	{
	DEFG_INPOINT_DEF
	Point3f Normal;
	}; // InputPoint

// crude non-C++ subclassing
#define DEFG_SPAN_DEF unsigned long int PointA, PointB; char OrigConnect, TrisRef;



struct Span
	{
	DEFG_SPAN_DEF
	}; // Span

struct SmoothedSpan
	{
	DEFG_SPAN_DEF
	int ABSlope; // [0...360]
	Point3f Normal;
	}; // Span


class DEFG
	{
	private:
/*		unsigned char TempBuf[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT];
		float TempFloat[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT], FinalOutput[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT], EdgeBuf[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT];
		double XCoordBuf[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT];
		unsigned long int ID[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT];
		unsigned char NonInfectious[DEFG_GRID_WIDTH * DEFG_GRID_HEIGHT];
*/
		unsigned char *TempBuf;
		float *TempFloat, *FinalOutput, *EdgeBuf;
		float *SNX;
		double *XCoordBuf;
		unsigned long *ID;
		unsigned char *NonInfectious;

		long DefgGridWidth, DefgGridHeight;
		long WriteGridWidth, WriteGridHeight;
		long WriteGridOffX, WriteGridOffY;
		long MaxPoints, MaxSpans;
		int Extrapolate;
		bool densify;

		struct InputPoint	*InPoints;
		struct SmoothedInputPoint	*SmoothedInPoints;
		struct Span			*Edges;
		struct SmoothedSpan			*SmoothedEdges;

		double LowX, HighX, LowY, HighY, LowZ, HighZ, XRange, YRange;
		double BoundLowX, BoundHighX, BoundLowY, BoundHighY;
		double XStart, XTerm, YStart, YTerm;

		int LoadedPoints, TotalSpans;
		float NullVal, SmoothVal;
		double LSmoothFrac, SSmoothFrac;
		char Smoothing, SmoothPass;

		double GridCoordRangeX, GridCoordLowX, GridCoordRangeY, GridCoordLowY, GridCoordSizeXmOneUnderRangeX, GridCoordSizeYmOneUnderRangeY;
		int GridCoordSizeX, GridCoordSizeY;
		int DefgGridCells;

		// set by SetupUnifyInfo() prior to tiled gridding
		float *RowUnify, *ColUnify;
		int UnifyNSTileNum, UnifyEWTileNum, UnifyNSTiles, UnifyEWTiles;

	public:
		int Grid(void);
		DEFG();
		~DEFG();

		int InitSizes(int FinalGridWidth, int FinalGridHeight, double HOver, double VOver, int MaxPoints);
		int InitSpans(int NewMaxSpans);
		int AllocRasterBufs(int FullGridWidth, int FullGridHeight);
		void SetExtrap(int NewExtrap) {Extrapolate = NewExtrap;};
		void SetBounds(double NewMinX, double NewMaxX, double NewMinY, double NewMaxY);
		void SetNullVal(float NewNullVal) {NullVal = NewNullVal;};
		void SetSmoothVal(float NewSmoothVal);
		void SetDensify(bool NewVal) {densify = NewVal;};

		void SetupUnifyInfo(float *SetupRowUnify, float *SetupColUnify, int SetupUnifyNSTileNum, int SetupUnifyEWTileNum, int SetupUnifyNSTiles, int SetupUnifyEWTiles)
		 {RowUnify = SetupRowUnify; ColUnify = SetupColUnify; UnifyNSTileNum = SetupUnifyNSTileNum; UnifyEWTileNum = SetupUnifyEWTileNum; UnifyNSTiles = SetupUnifyNSTiles; UnifyEWTiles = SetupUnifyEWTiles;};

		void ClearDist(float Value);
		void ClearFinalOutput(void);
		void ClearEdgeBuf(void);
		void ClearXBuffer(void);
		int AutoBoundPoints(void);
		void SetupGridCoordX(int GridSize, double LowCoord, double HighCoord);
		void SetupGridCoordY(int GridSize, double LowCoord, double HighCoord);
		int GridFromCoord(int GridSize, double LowCoord, double HighCoord, double Coord);
		double fGridFromCoord(int GridSize, double LowCoord, double HighCoord, double Coord);
		int GridFromCoordX(double Coord);
		int GridFromCoordY(double Coord);
		double fGridFromCoordX(double Coord);
		double fGridFromCoordY(double Coord);
		double CoordFromGrid(int GridSize, double LowCoord, double HighCoord, double Coord);
		double CoordFromGridX(double Coord);
		double CoordFromGridY(double Coord);
		double EvalDistNoSq(double XA, double YA, double XB, double YB);
		void FlagInterps(void);
		int InfectCellVal(int GX, int GY, int XOff, unsigned long int Idx, float Weight);
		void NormalizeExtrap(void);
		int InfectCell(int GX, int GY, int XOff, unsigned long int Idx, unsigned long int CenterCellID, double CX, double CY, double RX, double RY);
		int AddSpanToNode(InputPoint *AddNode, struct Span *AddSpan);
		int SpanCell(int GX, int GY, int XOff, unsigned long int Idx, InputPoint *CenterCell);
		void RasterizeSpanElevPoints(InputPoint *PA, InputPoint *PB);
		void SmoothRasterizeSpanElevPoints(InputPoint *IPA, InputPoint *IPB);
		void RasterizeSpanElevGridHor(int XS, double DXS, int YS, double ZS, int XE, double DXE, float ZE, double SNXS, float SNXE);
		void RasterizeSpanElev(struct Span *RasterSpan);
		void SmoothRasterizeSpanElev(struct SmoothedSpan *RasterSpan);
		void CalcSpanNormals(struct SmoothedSpan *RasterSpan);
		void CalcSpanNormalsPoints(InputPoint *IPA, InputPoint *IPB);
		bool AddPoint(double X, double Y, double Z);
		bool AddPoints(Point3d &pt1, Point3d &pt2, unsigned long num2add);

		float *GetFinalArray(void) {return(FinalOutput);};
		int GetFinalArrayWidth(void) {return(DefgGridWidth);};
		int GetFinalArrayHeight(void) {return(DefgGridHeight);};
		int SaveDEM(CoordSys *MyCS, Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, NNGrid *nng);

		// API used to be compatible with nngridr
		int DoGrid(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, TerraGridder *TG, Joe **TCJoeList, int JoeCount, int TCCount, NNGrid *nng, double xstart, double xterm, double ystart, double yterm, CoordSys *MyCS = NULL);


#ifdef DEFG_ENABLE_DIAG
		void DumpPoints(char *FileName);
		void DumpID(char *FileName);
		void DumpTempBuf(char *FileName);
		void DumpDist(char *FileName);
		void DumpElev(char *FileName);
		void DumpEdges(char *FileName);
		void CreateDumpEdgeBuf(char *FileName);
		void CreateDumpTempElev(char *FileName);
		void CreateDumpTempDist(char *FileName);
		void CreateDumpSNX(char *FileName);
		void CheckForDBLMAX(void);
		void PrintStatus(char *Status);
#endif // DEFG_ENABLE_DIAG

	}; // class DEFG

#ifdef DEFG_ENABLE_DIAG
void DoSplineTest(double AX, double AY, double BX, double BY, double CX, double CY, double DX, double DY, char *FN, double Tension = 0.0);
#endif // DEFG_ENABLE_DIAG
