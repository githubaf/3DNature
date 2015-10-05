// DEMCore.h
// The very essential pieces necessary to load a WCS/VNS ELEV format DEM file
// WCS's DEM.h defines WCS_DEM_H, which we use to conditionally enable capabilities
// that other stand-alone builds won't need

#include "stdafx.h"
#include "time.h"	// for time_t

#ifndef WCS_DEMCODE_H
#define WCS_DEMCORE_H

#define WCS_DEM_CURVERSION			1.02
#define WCS_DEM_HEADERSIZE_V102		64

// Definitions for the flag bits in the DEM object, use SetFlags and CheckFlags
enum
	{
	WCS_DEM_FLAG_MAPASSFC
	}; // DEMFlags

enum
	{
	WCS_DEM_ITERATE_BYSAMPLE,
	WCS_DEM_ITERATE_BYCOORD
	}; // IterateTypes

#define WCS_DEM_PRECISION_SHORT		0
#define WCS_DEM_PRECISION_FLOAT		1

#define WCS_DEM_COMPRESSION_NONE	0
#define WCS_DEM_COMPRESSION_ZLIB	1

#define WCS_DEMDATA_NAME		0x01000000
#define WCS_DEMDATA_COVERAGE	0x02000000

#define WCS_DEM_COVERAGE_ELEVATION		0x00100000
#define WCS_DEM_COVERAGE_RELEL			0x00200000
#define WCS_DEM_COVERAGE_ROWS			0x00300000
#define WCS_DEM_COVERAGE_COLUMNS		0x00400000
#define WCS_DEM_COVERAGE_NWLAT			0x00500000
#define WCS_DEM_COVERAGE_NWLON			0x00600000
#define WCS_DEM_COVERAGE_SELAT			0x00700000
#define WCS_DEM_COVERAGE_SELON			0x00800000
#define WCS_DEM_COVERAGE_LATSTEP		0x00900000
#define WCS_DEM_COVERAGE_LONSTEP		0x00a00000

#define WCS_DEM_COVERAGE_DATA_PRECISION		0x00010000
#define WCS_DEM_COVERAGE_DATA_COMPRESSION	0x00020000
#define WCS_DEM_COVERAGE_DATA_DATUM			0x00030000
#define WCS_DEM_COVERAGE_DATA_ELSCALE		0x00040000
#define WCS_DEM_COVERAGE_DATA_MAXEL			0x00050000
#define WCS_DEM_COVERAGE_DATA_MINEL			0x00060000
#define WCS_DEM_COVERAGE_DATA_SAMPLES		0x00070000
#define WCS_DEM_COVERAGE_DATA_SUMELDIF		0x00080000
#define WCS_DEM_COVERAGE_DATA_SUMELDIFSQ	0x00090000
#define WCS_DEM_COVERAGE_DATA_DATABLOCK		0x000a0000
#define WCS_DEM_COVERAGE_DATA_NULLREJECT	0x000b0000
#define WCS_DEM_COVERAGE_DATA_NULLVALUE		0x000c0000




#ifndef WCS_PARAM_DONE
#define WCS_PARAM_DONE						0xffff0000
#endif // !WCS_PARAM_DONE

class CoordSys;
class VectorNode;

class GeoPoint
	{
	public:
		double Lat, Lon;
	}; // GeoPoint

class DEM
	{
#ifndef BUILD_LIB // building as library we won't need this
	friend class Database;
	friend class Renderer;
	friend class ViewGUI;
	friend class InterpDEMGUI;
	friend class TerraGridder;
	friend class TerraGenerator;
	friend class NNGrid;
	friend class DEFG;
	friend class DEMMerger;
	friend class DEMMergeGUI;
	friend class DEMPaintGUI;
	friend class Joe;
	friend class ImportWizGUI;
	friend class ImageFormatWCSELEV;
	friend class InterCommon;
	friend class SceneExportGUI;
	friend class DEMEval;
	private:
#else // BUILD_LIB
	public:
#endif // BUILD_LIB
		unsigned long Flags;
		short *RelElMap;
		float *RawMap;
		signed char *FractalMap;
		#ifdef WCS_VECPOLY_EFFECTS
		VectorPolygonListDouble **VPData;
		time_t LastTouchedTime;
		#endif // WCS_VECPOLY_EFFECTS
		unsigned long *XIndex, *YIndex, ScreenWidth, ScreenHeight, XIMax;
#ifndef BUILD_LIB // building as library we won't need this
		VertexDEM *Vertices;
		Joe *MoJoe;
#endif // !BUILD_LIB
		unsigned char RawCount;

		// Stuff that's public, but should be read-only so access via member
		GeoPoint pNorthWest, pSouthEast;
		// LonEntries used to be Rows, LatEntries was Columns
		unsigned long pLonEntries, pLatEntries, pElSamples, pRelElSamples;
		unsigned long pDSLonEntries, pDSLatEntries, pDSSamples;
		float pElMaxEl, pElMinEl, pRelElMaxEl, pRelElMinEl, pNullValue;
		float pElSumElDif, pElSumElDifSq, pRelElSumElDif, pRelElSumElDifSq, pFileVersion;
		double pLatStep, pLonStep, pElScale, pElDatum, pRelElScale, pRelElDatum;
		double InvLatStep, InvLonStep; // inverse of pLatStep and pLonStep for multiplying instead of dividing
		double RelativeEffectAdd, RelativeEffectSubtract, AbsoluteEffectMax, AbsoluteEffectMin; // added for VNS 3

		void PrecalculateCommonFactors(void);
		inline unsigned long MapSize(void) {return(pLonEntries * pLatEntries);};

#ifndef BUILD_LIB
		unsigned long ReadV101Header(FILE *In);
		inline unsigned long FractalMapSize(void) {return((2 * (pLatEntries - 1)) * (pLonEntries - 1));};
		inline unsigned long RelElMapSize(void)	{return(MapSize() * sizeof (short));};
		inline unsigned long RawMapSize(void)	{return(MapSize() * sizeof (float));};
		inline unsigned long VertexMapSize(void);
		#ifdef WCS_VECPOLY_EFFECTS
		inline unsigned long VPDataMapEntries(void) {return((pLatEntries - 1) * (pLonEntries - 1));};
		inline unsigned long VPDataMapSize(void) {return(VPDataMapEntries() * sizeof (VectorPolygonListDouble *));};
		#endif // WCS_VECPOLY_EFFECTS

		short EnterBoxWeights(void);
		void PadArray(float *AltMap);
		short ComputeRelEl(float *AltMap, short boxsize);

		int CheckExistColorMap(char *InputName);
		int LoadFractalMap(char *InputName);
		int SaveFractalMap(char *OutputName);
		void FreeRelElMap(void);
		void SetCell(unsigned long col, unsigned long row, float elev);
#endif // !BUILD_LIB
		float GetCell(unsigned long col, unsigned long row);



	public:
		// used by Renderer mostly
		long Lr, Lc;
		short NullReject;

	public:
		DEM();
		~DEM();
		void SetNullPointers(void);

		// These are just read-only inline interfaces to the private
		// variables above. They'll be optimized into an inline
		// access by the compiler, but this keeps them from being
		// written to from outside the object.

		// LonEntries used to be Rows, LatEntries was Columns
		inline unsigned long LonEntries (void) {return(pLonEntries);};
		inline unsigned long LatEntries (void) {return(pLatEntries);};
		// By definition, latitudes are always positive in the northern hemisphere and
		// negative in the south and always increase to the north.
		// Northings (projected data), regardless of their origin, increase to the north.
		inline double Northest  (void) {return(pNorthWest.Lat);};
		inline double Southest  (void) {return(pSouthEast.Lat);};
		// By definition, longitudes are positive in the western hemisphere and increase to the west.
		// Eastings (projected data), regardless of their origin, increase to the east.
		inline double Eastest   (void) {return(pSouthEast.Lon);};
		inline double Westest   (void) {return(pNorthWest.Lon);};
		// By definition LatStep will always be positive.
		// LonStep is positive for geographic data and negative for projected data and is in the same units
		// as the DEM bounds.
		inline double LatStep(void) {return(pLatStep);};
		inline double LonStep(void) {return(pLonStep);};
		//double RoughArea(void);
		inline double ElScale(void) {return(pElScale);};
		inline double ElDatum(void) {return(pElDatum);};
		inline float MaxEl(void) {return(pElMaxEl);};
		inline float MinEl(void) {return(pElMinEl);};
		inline float NullValue(void) {return(pNullValue);};
		inline unsigned long Samples(void) {return(pElSamples);};
		inline double SumElDif(void) {return(pElSumElDif);};
		inline double SumElDifSq(void) {return(pElSumElDifSq);};

		inline unsigned long DSLonEntries (void) {return(pDSLonEntries);};
		inline unsigned long DSLatEntries (void) {return(pDSLatEntries);};
		inline unsigned long DSSamples (void) {return(pDSSamples);};
		inline float *Map (void) {return(RawMap);};
		inline short *RelEl (void) {return(RelElMap);};

		inline unsigned long CheckFlags(unsigned long F) {return(F & Flags);};
		inline void SetFlags(unsigned long F) {Flags |= F;};

#ifndef BUILD_LIB
		void Copy(DEM *CopyFrom, short CopyPointers);

		short AllocFaceArray(void);
		short AllocFractalMap(int SetToDefault);
		short AllocRelElMap(void);
		short AllocVertices(void);
		#ifdef WCS_VECPOLY_EFFECTS
		short AllocVPData(void);
		void SetLastTouchedTime(time_t SetTime)	{LastTouchedTime = SetTime;};
		time_t GetLastTouchedTime(void)	{return(LastTouchedTime);};
		#endif // WCS_VECPOLY_EFFECTS
#endif // !BUILD_LIB
		short AllocRawMap(void);

		void FreeRawElevs(void);
		void FreeRawMap(void);

#ifndef BUILD_LIB
		void FreeFractalMap(void);
		void FreeVertices(void);
		#ifdef WCS_VECPOLY_EFFECTS
		unsigned long FreeVPData(bool ForceFree);
		unsigned long CalcFullMemoryReq(void);
		#endif // WCS_VECPOLY_EFFECTS

		unsigned long LoadRawElevs(char *InName);
		unsigned long LoadRawElevs(FILE *Input);
		unsigned long LoadRelElevs(FILE *Input);
		int TransferToVerticesLatLon(int ValidateInBounds);
		int TransferToVerticesXYZ(void);
		int TransferXYZtoLatLon(void);
#endif // !BUILD_LIB

		short FindElMaxMin(void);
		long FillVoids(void);
		bool DumpRaster(const char *filename, float refMin, float refMax, bool range254 = false);

#ifndef BUILD_LIB // building as library we won't need this
		short FindRelElMaxMin(void);
		short ComputeRelEl(MessageLog *ErrorReport = NULL);

		unsigned long ScreenTransform(unsigned long Width, unsigned long Height);
		short GetRelElFromScreenCoord(long X, long Y, short Max = SHRT_MAX, short Min = SHRT_MIN);
		short GetFractalFromScreenCoord(long X, long Y);
		void FreeScreenData(void);

		inline void SetNullFractalMap   (void) {FractalMap = NULL;};
		inline void SetLatEntries(long NewLatEntries)	{pLatEntries = NewLatEntries;};
		inline void SetLonEntries(long NewLonEntries)	{pLonEntries = NewLonEntries;};
		inline void SetLatStep(double NewLatStep)	{pLatStep = NewLatStep;};
		inline void SetLonStep(double NewLonStep)	{pLonStep = NewLonStep;};
		inline void SetElScale(double NewElScale)	{pElScale = NewElScale;};
		inline void SetElDatum(double NewElDatum)	{pElDatum = NewElDatum;};
		inline void SetMaxEl(double NewMaxEl)	{pElMaxEl = (float)NewMaxEl;};
		inline void SetMinEl(double NewMinEl)	{pElMinEl = (float)NewMinEl;};
		void SetBounds(double North, double South, double West, double East);
		inline void StoreElevation(long LatEntry, long LonEntry, float NewElev)	{if (RawMap) RawMap[LonEntry * pLatEntries + LatEntry] = NewElev;};
#endif // !BUILD_LIB
		inline void SetNullValue   (float Val) {pNullValue = Val;};
		inline void SetNullReject   (short Val) {NullReject = Val;};
		inline short GetNullReject   (void) {return (NullReject);};
		inline int TestNullReject   (unsigned long VertNumber) {return (NullReject && (RawMap[VertNumber] == pNullValue));};
		inline int TestNullValue   (unsigned long VertNumber) {return (RawMap[VertNumber] == pNullValue);};

		// used by VNS 3 renderer to sample splined elevation values
#ifndef BUILD_LIB // building as library we won't need this
		void SampleCellCorner(unsigned long PositionFlags, VectorNode *CurNode);
		bool SplineInternalCellValue(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode, bool CellUnknown);
		bool SplineCellEdgeNorthSouth(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode);
		bool SplineCellEdgeEastWest(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode);
#endif // !BUILD_LIB

		// See IterateTypes below for the ItType argument. Returns
		// number of output samples per horiztonal axis.
		//unsigned long SetupIterate(char ItType, double LowVertAxis,
		// double HighVertAxis, double LowHorizAxis, double HighHorizAxis,
		// double VertAxisInc, double HorizAxisInc);
		//inline double NextSample(void);
		
		inline float Sample(unsigned long Horiz, unsigned long Vert) {return(RawMap[Vert + (Horiz * pLatEntries)]);};
		inline float SampleRaw(unsigned long Offset) {return(RawMap[Offset]);};
#ifndef BUILD_LIB // building as library we won't need this
		inline signed short XYRelEl(unsigned long Horiz, unsigned long Vert) {return(RelElMap ? RelElMap[Vert + (Horiz * pLatEntries)]: 0);};
		inline signed short SampleRelEl(unsigned long Offset) {return(RelElMap ? RelElMap[Offset]: 0);};
		
		unsigned long LoadDEM(char *InName, short LoadRelEl, CoordSys **MyCoords);
#endif // !BUILD_LIB
		unsigned long LoadDEM(FILE *ffile, short LoadRelEl, CoordSys **MyCoords, struct ProjectIODetail *Detail = NULL);
		unsigned long Load(FILE *ffile, short ByteFlip, short LoadRelEl);
		unsigned long Coverage_Load(FILE *ffile, short ByteFlip, short LoadRelEl);
		unsigned long Elevation_Load(FILE *ffile, short ByteFlip);
#ifndef BUILD_LIB
		unsigned long RelEl_Load(FILE *ffile, short ByteFlip);

		short SaveDEM(char *filename, MessageLog *ErrorReport);
		unsigned long Save(FILE *ffile, char *filename);
		unsigned long Coverage_Save(FILE *ffile);
		unsigned long Elevation_Save(FILE *ffile, char Compression = WCS_DEM_COMPRESSION_NONE);
		unsigned long RelEl_Save(FILE *ffile, char Compression = WCS_DEM_COMPRESSION_NONE);

		char *AttemptFindDEMPath(char *ObjName, Project *OpenFrom);
		unsigned long AttemptLoadDEM(Joe *MyJoe, short LoadRelEl, Project *OpenFrom);
		// One of these is consistant with arg order, the other is for backward compat
		int AttemptLoadDownSampled(Joe *MyJoe, Project *OpenFrom, long GridSize);
		int AttemptLoadDownSampled(Joe *MyJoe, long GridSize, Project *OpenFrom) {return(AttemptLoadDownSampled(MyJoe, OpenFrom, GridSize));};
		int AttemptLoadBetterDownSampled(Joe *MyJoe, Project *OpenFrom, int MaxPolys);
#ifdef WCS_MULTIFRD
		int AttemptLoadFractalFile(char *ObjName, char *ParamName, Project *OpenFrom);
		int AttemptSaveFractalFile(char *ObjName, char *ParamName, Project *OpenFrom);
#else // WCS_MULTIFRD
		int AttemptLoadFractalFile(char *ObjName, Project *OpenFrom);
		int AttemptSaveFractalFile(char *ObjName, Project *OpenFrom);
#endif // WCS_MULTIFRD
		int AttemptCheckExistColorMapFile(char *ObjName, Project *OpenFrom, char *ReturnPath, char *OptionalDir = NULL);
#endif // !BUILD_LIB
		// needed for terrain-following in NV
		int TestPointNULL(double X, double Y);
		int TestPointNULL2(double X, double Y);	// used by DEM Merger
		double DEMArrayPointExtract(CoordSys *SourceCoords, double Lat, double Lon);
		double DEMArrayPointExtract2(CoordSys *SourceCoords, double Lat, double Lon);	// used by DEM Merger
#ifndef BUILD_LIB
		double DEMRelElPointExtract(CoordSys *SourceCoords, double Lat, double Lon);
		int GetDEMCellSizeMeters(double &CellSizeNS, double &CellSizeWE);
		// this sets -1 in fractal map if any vertex of a cell has the NULL reject values
		void NullRejectSetFractalMap(void);
		int GeographicPointContained(CoordSys *SourceCoords, double SourceLat, double SourceLon, int RejectNULL);
		int XYFromLatLon(CoordSys *SourceCoords, double SourceLat, double SourceLon, double &X, double &Y);
		// used by DEM Merger
		double ArrayPointExtract2(double X, double Y, double MinX, double MinY,
			double IntX, double IntY, long Cols, long Rows);
		void Disect(void);
		void Erode(void);
#endif // !BUILD_LIB

		int CalcPolys(void); // tells you how many polygons this DEM would have

	}; // DEM

#endif // WCS_DEMCORE_H
