// DEMCore.h
// The very essential pieces necessary to load a WCS/VNS ELEV format DEM file
// WCS's DEM.h defines WCS_DEM_H, which we use to conditionally enable capabilities
// that other stand-alone builds won't need

#ifndef WCS_DEMCORE_H
#define WCS_DEMCORE_H

#include <stdio.h>

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
	friend class DEMRandomGUI;
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
		unsigned long int Flags;
		short *RelElMap;
		float *RawMap;
		signed char *FractalMap;
		unsigned long int *XIndex, *YIndex, ScreenWidth, ScreenHeight, XIMax;
#ifndef BUILD_LIB // building as library we won't need this
		VertexDEM *Vertices;
		Joe *MoJoe;
#endif // !BUILD_LIB
		unsigned char RawCount;

		// Stuff that's public, but should be read-only so access via member
		GeoPoint pNorthWest, pSouthEast;
		// LonEntries used to be Rows, LatEntries was Columns
		unsigned long int pLonEntries, pLatEntries, pElSamples, pRelElSamples;
		unsigned long int pDSLonEntries, pDSLatEntries, pDSSamples;
		float pElMaxEl, pElMinEl, pRelElMaxEl, pRelElMinEl, pNullValue;
		float pElSumElDif, pElSumElDifSq, pRelElSumElDif, pRelElSumElDifSq, pFileVersion;
		double pLatStep, pLonStep, pElScale, pElDatum, pRelElScale, pRelElDatum;

		inline unsigned long int MapSize(void) {return(pLonEntries * pLatEntries);};

#ifndef BUILD_LIB
		unsigned long int ReadV101Header(FILE *In);
		inline unsigned long int FractalMapSize(void) {return((2 * (pLatEntries - 1)) * (pLonEntries - 1));};

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
		inline unsigned long int LonEntries (void) {return(pLonEntries);};
		inline unsigned long int LatEntries (void) {return(pLatEntries);};
		inline double Northest  (void) {return(pNorthWest.Lat);};
		inline double Southest  (void) {return(pSouthEast.Lat);};
		inline double Eastest   (void) {return(pSouthEast.Lon);};
		inline double Westest   (void) {return(pNorthWest.Lon);};
		inline double LatStep   (void) {return(pLatStep);};
		inline double LonStep   (void) {return(pLonStep);};
		double RoughArea   (void);
		inline double ElScale   (void) {return(pElScale);};
		inline double ElDatum   (void) {return(pElDatum);};
		inline float MaxEl (void) {return(pElMaxEl);};
		inline float MinEl (void) {return(pElMinEl);};
		inline float NullValue (void) {return(pNullValue);};
		inline unsigned long int Samples (void) {return(pElSamples);};
		inline double SumElDif  (void) {return(pElSumElDif);};
		inline double SumElDifSq(void) {return(pElSumElDifSq);};

		inline unsigned long int DSLonEntries (void) {return(pDSLonEntries);};
		inline unsigned long int DSLatEntries (void) {return(pDSLatEntries);};
		inline unsigned long int DSSamples (void) {return(pDSSamples);};
		inline float *Map (void) {return(RawMap);};
		inline short *RelEl (void) {return(RelElMap);};

		inline unsigned long int CheckFlags(unsigned long int F) {return(F & Flags);};
		inline void SetFlags(unsigned long int F) {Flags |= F;};

#ifndef BUILD_LIB
		void Copy(DEM *CopyFrom, short CopyPointers);

		short AllocFaceArray(void);
		short AllocFractalMap(int SetToDefault);
		short AllocRelElMap(void);
		short AllocVertices(void);
#endif // !BUILD_LIB
		short AllocRawMap(void);

		void FreeRawElevs(void);
		void FreeRawMap(void);

#ifndef BUILD_LIB
		void FreeFractalMap(void);
		void FreeVertices(void);

		unsigned long int LoadRawElevs(char *InName);
		unsigned long int LoadRawElevs(FILE *Input);
		unsigned long int LoadRelElevs(FILE *Input);
		int TransferToVerticesLatLon(void);
		int TransferToVerticesXYZ(void);
		int TransferXYZtoLatLon(void);
#endif // !BUILD_LIB


		short FindElMaxMin(void);

#ifndef BUILD_LIB // building as library we won't need this
		short FindRelElMaxMin(void);
		short ComputeRelEl(MessageLog *ErrorReport = NULL);

		unsigned long int ScreenTransform(unsigned long int Width, unsigned long int Height);
		short GetRelElFromScreenCoord(long int X, long int Y, short int Max = SHRT_MAX, short int Min = SHRT_MIN);
		short GetFractalFromScreenCoord(long int X, long int Y);
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
#endif // !BUILD_LIB
		inline void StoreElevation(long LatEntry, long LonEntry, float NewElev)	{if (RawMap) RawMap[LonEntry * pLatEntries + LatEntry] = NewElev;};
		inline void SetNullValue   (float Val) {pNullValue = Val;};
		inline void SetNullReject   (short Val) {NullReject = Val;};
		inline short GetNullReject   (void) {return (NullReject);};
		inline int TestNullReject   (unsigned long VertNumber) {return (NullReject && (RawMap[VertNumber] == pNullValue));};
		inline int TestNullValue   (unsigned long VertNumber) {return (RawMap[VertNumber] == pNullValue);};



		// See IterateTypes below for the ItType argument. Returns
		// number of output samples per horiztonal axis.
		//unsigned long int SetupIterate(char ItType, double LowVertAxis,
		// double HighVertAxis, double LowHorizAxis, double HighHorizAxis,
		// double VertAxisInc, double HorizAxisInc);
		//inline double NextSample(void);
		
		inline float Sample(unsigned long int Horiz, unsigned long int Vert) {return(RawMap[Vert + (Horiz * pLatEntries)]);};
		inline float SampleRaw(unsigned long int Offset) {return(RawMap[Offset]);};
#ifndef BUILD_LIB // building as library we won't need this
		inline signed short int XYRelEl(unsigned long int Horiz, unsigned long int Vert) {return(RelElMap ? RelElMap[Vert + (Horiz * pLatEntries)]: 0);};
		inline signed short int SampleRelEl(unsigned long int Offset) {return(RelElMap ? RelElMap[Offset]: 0);};
		
		unsigned long int LoadDEM(char *InName, short LoadRelEl, CoordSys **MyCoords);
#endif // !BUILD_LIB
		unsigned long int LoadDEM(FILE *ffile, short LoadRelEl, CoordSys **MyCoords, struct ProjectIODetail *Detail = NULL);
		unsigned long int Load(FILE *ffile, short ByteFlip, short LoadRelEl);
		unsigned long int Coverage_Load(FILE *ffile, short ByteFlip, short LoadRelEl);
		unsigned long int Elevation_Load(FILE *ffile, short ByteFlip);
#ifndef BUILD_LIB
		unsigned long int RelEl_Load(FILE *ffile, short ByteFlip);

		short SaveDEM(char *filename, MessageLog *ErrorReport);
		unsigned long int Save(FILE *ffile, char *filename);
		unsigned long int Coverage_Save(FILE *ffile);
		unsigned long int Elevation_Save(FILE *ffile, char Compression = WCS_DEM_COMPRESSION_NONE);
		unsigned long int RelEl_Save(FILE *ffile, char Compression = WCS_DEM_COMPRESSION_NONE);

		char *AttemptFindDEMPath(char *ObjName, Project *OpenFrom);
		unsigned long int AttemptLoadDEM(Joe *MyJoe, short LoadRelEl, Project *OpenFrom);
		// One of these is consistant with arg order, the other is for backward compat
		int AttemptLoadDownSampled(Joe *MyJoe, Project *OpenFrom, long GridSize);
		int AttemptLoadDownSampled(Joe *MyJoe, long GridSize, Project *OpenFrom) {return(AttemptLoadDownSampled(MyJoe, OpenFrom, GridSize));};
		int AttemptLoadBetterDownSampled(Joe *MyJoe, Project *OpenFrom, int MaxPolys);
		int AttemptLoadFractalFile(char *ObjName, Project *OpenFrom);
		int AttemptSaveFractalFile(char *ObjName, Project *OpenFrom);
		int AttemptCheckExistColorMapFile(char *ObjName, Project *OpenFrom, char *ReturnPath, char *OptionalDir = NULL);
#endif // !BUILD_LIB
		// needed for terrain-following in NV
		int TestPointNULL(double X, double Y);
		double DEMArrayPointExtract(CoordSys *SourceCoords, double Lat, double Lon);
#ifndef BUILD_LIB
		double DEMRelElPointExtract(CoordSys *SourceCoords, double Lat, double Lon);
		int GetDEMCellSizeMeters(double &CellSizeNS, double &CellSizeWE);
		// this sets -1 in fractal map if any vertex of a cell has the NULL reject values
		void NullRejectSetFractalMap(void);
		int GeographicPointContained(CoordSys *SourceCoords, double SourceLat, double SourceLon, int RejectNULL);
		int XYFromLatLon(CoordSys *SourceCoords, double SourceLat, double SourceLon, double &X, double &Y);

		void Disect(void);
		void Erode(void);
#endif // !BUILD_LIB

		int CalcPolys(void); // tells you how many polygons this DEM would have

	}; // DEM

#endif // WCS_DEMCORE_H
