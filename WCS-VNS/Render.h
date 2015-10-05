// Render.h
// Header for the all new for v5 Renderer
// Built mostly from scratch by Gary R. Huber, 8/99
// Copyright 1999 by Questar Productions. All rights reserved.

#ifndef WCS_RENDER_H
#define WCS_RENDER_H

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

class WCSApp;
class Renderer;
class Database;
class EffectsLib;
class ImageLib;
class Project;
class RenderJob;
class RenderOpt;
class Camera;
class Raster;
class BufferNode;
class Polygon3D;
class RenderInterface;
class DrawingFenetre;
class TerrainParamEffect;
class PolygonData;
class VertexBase;
class VertexData;
class VertexDEM;
class VertexCloud;
class Vertex3D;
class PixelData;
class TextureData;
class Joe;
class JoeDEM;
class GeneralEffect;
class EcosystemEffect;
class SnowEffect;
class GroundEffect;
class LakeEffect;
class StreamEffect;
class CmapEffect;
class EnvironmentEffect;
class MaterialEffect;
class Fence;
class CloudEffect;
class CloudLayerData;
class AnimMaterialGradient;
class PlanetOpt;
class Light;
class CoordSys;
class DEM;
class Ecotype;
class PolygonEdge;
class RenderQ;
class MessageLog;
class BoundsPackage;
class FoliageLink;
class InterCommon;
class RenderPreviewGUI;
class VectorPoint;
class Object3DEffect;
class ObjectMaterialEntry;
class PixelFragment;
class Object3DVertexList;
class FoliageVertexList;
class FenceVertexList;
class LabelVertexList;
class RealtimeFoliageData;
class rPixelFragment;
class rPixelHeader;
class rPixelBlockHeader;
class RealTimeFoliageWriteConfig;
class PostProcess;
class ObjectPerVertexMap;
class VertexReferenceData;
class VolumetricSubstance;
class SceneExporter;
class RasterBounds;
class Label;
class FoliageGroup;
class Foliage;
class FoliagePreviewData;
class EffectEval;
class VectorPolygonListDouble;
class VectorPolygon;
class VectorNode;
class VectorNodeRenderData;
class MaterialList;
class MaterialTable;
class QuantaAllocator;
class BusyWin;

#include <time.h> // for time_t type

#include "Random.h"
#include "TrigTable.h"
#include "Types.h"
#include "Texture.h"
#include "FeatureConfig.h"
#include "VectorNode.h"
#include "QuantaAllocator.h"

#undef RGB // stupid Microsoft makes macros with hazardously-common names

#define MAX_MATERIALTABLE_ENTRIES	20
#define MATERIALTABLESIZE	(MAX_MATERIALTABLE_ENTRIES + 1)
#define RENDERLISTSIZE	1000
#define INV_DISPLACE_FRACT_SLOPEFACT	(1.0 / 10.0)
#define WCS_MAX_FRACTALDEPTH	9
#define WCS_RENDER_FLAGBUFBIT_WATER		(1 << 0)

struct PixelFragSort
	{
	double Covg;
	rPixelFragment *Frag;
	float Z;
	}; // struct PixelFragSort

// in Render.cpp
class VertexBase
	{
	public:
		double xyz[3], XYZ[3], ScrnXYZ[3], Q;

		VertexBase();
		void CopyVBase(VertexBase *CopyFrom);
		void CopyXYZ(VertexBase *CopyFrom);
		void CopyScrnXYZQ(VertexBase *CopyFrom);
		void GetPosVector(VertexBase *Origin);
		void UnGetPosVector(VertexBase *Origin);
		void UnitVector(void);
		void AddXYZ(VertexBase *Increment);
		void MultiplyXYZ(double Multiplier);
		void RotateX(double Angle);
		void RotateY(double Angle);
		void RotateZ(double Angle);
		double FindAngleYfromZ(void);
		double FindRoughAngleYfromZ(void); // this one uses approximated atan for faster but less precise results, adequate for aspect
		double FindAngleXfromZ(void);
		double FindRoughAngleXfromZ(void); // this one uses approximated atan for faster but less precise results
		double FindAngleXfromY(void);
		double FindAngleZfromY(void);
		void SetDefaultViewVector(void);
		void SetDefaultCamVerticalVector(void);
		void ProjectPoint(Matx3x3 M);
		void UnProjectPoint(Matx3x3 M);
		void Transform3DPoint(Matx4x4 M);
		void Transform3DPointTo_xyz(Matx4x4 M);
		void InterpolateVertexBase(VertexBase *FromVtx, VertexBase *ToVtx, double LerpVal);

	}; // class VertexBase

// in Render.cpp
class VertexDEM : public VertexBase
	{
	public:
		double Lat, Lon, Elev;
		unsigned char Flags;

		VertexDEM();
		void CopyVDEM(VertexDEM *CopyFrom);
#ifdef SPEED_MOD
		void FASTCALL CopyLatLon(VertexDEM *CopyFrom);
#else // SPEED_MOD
		void CopyLatLon(VertexDEM *CopyFrom);
#endif // SPEED_MOD
		void DegToCart(double PlanetRad);
		void CartToDeg(double PlanetRad);
		void CartToDeg(double SMajor, double SMinor);
		void RotateToHome(void);
		void RotateFromHome(void);
		void ProjectAsPlanView(void) {XYZ[0] = Lon; XYZ[1] = Lat; XYZ[2] = Elev;};
		void InterpolateVertexDEM(VertexDEM *FromVtx, VertexDEM *ToVtx, double LerpVal);

	}; // class VertexDEM

// in Render.cpp
class VertexCloud : public VertexDEM
	{
	public:

		VertexCloud();
		void InterpolateVertexCloud(VertexCloud *FromVtx, VertexCloud *ToVtx, double LerpVal);

	}; // class VertexDEM

// in Render.cpp
class VertexData : public VertexDEM
	{
	public:
		double WaterElev, WaveHeight, WaterDepth, Displacement, oElev, oDisplacement, BeachLevel;	
			// oElev stores the vertex elevation sans-terraffectors. It is used for fractal subdivision calculations.
		EcosystemEffect *Eco;
		LakeEffect *Lake;
		StreamEffect *Stream;
		AnimMaterialGradient *Beach;
		Joe *Vector;
		DEM *Map;
		PolygonEdge *EdgeParent;
		#ifdef WCS_VECPOLY_EFFECTS
		VectorNodeRenderData *NodeData;
		#endif // WCS_VECPOLY_EFFECTS
		float VectorSlope, RelEl, VecOffsets[3];
		unsigned long LatSeed, LonSeed;
		unsigned char Frd;
		char VectorType;

		VertexData();
		VertexData(PolygonEdge *NewPolyParent);
		void InterpolateVertexData(VertexData *FromVtx, VertexData *ToVtx, double LerpVal);

	}; // class VertexData

// for 3D objects
class Vertex3D : public VertexDEM
	{
	public:
		double Normal[3];
		//float RGB[3];
		long *PolyRef;
		short NumPolys;
		char Normalized;

		Vertex3D();
		~Vertex3D();
		void Copy(Vertex3D *CopyTo, Vertex3D *CopyFrom);
		void InterpolateVertex3D(Vertex3D *FromVtx, Vertex3D *ToVtx, double LerpVal);
		void Normalize(Polygon3D *Poly, Vertex3D *Vert, Polygon3D *RefPoly, ObjectMaterialEntry *Matl);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, ObjectPerVertexMap **LoadToColors, 
			long NumVerts, long &NumCPVMaps, long VertsRead);
		unsigned long Save(FILE *ffile, int SaveNormals);

	}; // Vertex3D

#ifdef WCS_FORESTRY_WIZARD
// see RenderForestry.cpp
class ForestryRenderData
	{
	public:
		double AvgNumStems, NormalizedDensity, RawDensity,
			MaxHeight, MinHeight, HeightRange, AgvHeight, MaxDbh, MaxAge, MaxClosure, MaxStems,
			MaxBasalArea, RenderOpacity, MatCovg;
		double *VertexWeights;
		Ecotype *Eco;
		PolygonData *Poly;
		VertexData **Vtx;
		unsigned long SeedOffset;
		bool EcoDensityComputed, EcoSizeComputed, GroupDensityComputed, GroupSizeComputed, FolEffect, RenderAboveBeachLevel;

		ForestryRenderData();
	}; // class ForestryRenderData
#endif // WCS_FORESTRY_WIZARD

class FoliagePlotData
	{
	public:
		double WidthInPixels, HeightInPixels, TopElev, ElevGrad, Opacity, ColorImageOpacity,
			OrientationShading, MaxZOffset, TestPtrnOffset;
		double *ReplaceColor;
		PolygonData *Poly;
		VertexDEM *Vert;
		Raster *SourceRast;
		int RenderOccluded, ColorImage, Shade3D;

		FoliagePlotData();
	}; // class FoliagePlotData

class RendererQuantaAllocatorErrorFunctor : public QuantaAllocatorErrorFunctor
	{
	private:
		Renderer *MyRend;
		
	public:
		RendererQuantaAllocatorErrorFunctor(Renderer *SetRend)	{MyRend = SetRend;};
		// in Render.cpp:
		bool operator() (QuantaAllocator *ManagingAllocator);// {UserMessageOK("QuantaAllocator", "Here, we would try to free some resources.", REQUESTER_ICON_STOP); return(true);};
		void SetRend(Renderer *NewRend)	{MyRend = NewRend;};
	}; // RendererQuantaAllocatorErrorFunctor

// see RenderData.cpp
class RenderData
	{
	public:
		double EarthLatScaleMeters, RenderTime, PlanetRad, EarthRotation, ElevDatum, Exageration, PixelAspect, FrameRate, 
			TexRefLon, TexRefLat, TexRefElev, RefLonScaleMeters,
			ShadowMapDistanceOffset;

		Renderer *Rend;
		RenderOpt *Opt;
		Camera *Cam;
		InterCommon *Interactive;
		Database *DBase;
		EffectsLib *EffectsBase;
		Project *ProjectBase;
		CoordSys *DefCoords;
		EnvironmentEffect *DefaultEnvironment;
		VolumetricSubstance *Substances;
		TextureData TexData, TexData1;
		VertexDEM BumpVert[3];
		VertexData BumpSampleVert;
		TextureData BumpBackupTextureData;

		long PanoPanel, Segment, NumPanels, NumSegments, StereoSide, SetupWidth, SetupHeight, Width, Height;
		// F2 NOTE
		// IsCamView (and presumably, IsProcessing) stall the CPU when defined as chars, should be longs
		char IsCamView, IsProcessing, ExagerateElevLines, IsTextureMap, ShadowMapInProgress;

		RenderData(Renderer *MyRenderer);
		int InitToView(EffectsLib *EffectsSource, Project *ProjectSource, Database *NewDB, InterCommon *NewInter, RenderOpt *NewOpt, Camera *NewCam, long ImageWidth, long CamSetupWidth);
		void InitFrameToRender(Renderer *Rend);
		double GetElevationPoint(double Lat, double Lon);
		TextureData *TransferTextureData(PolygonData *Poly);
		TextureData *TransferTextureData(VertexData *Vert);
		TextureData *TransferTextureData(VertexDEM *Vert);
		TextureData *TransferTextureData(PixelData *Pix);
		TextureData *TransferTextureData(VectorNode *Node, Joe *RefVec, MaterialList *RefMat);
		TextureData *TransferTextureDataRange(VertexData *Vert, TextureData *TxData, double PixWidth);
		TextureData *Transfer3DTextureDataRange(VertexData *Vert, double PixWidth, double ObjPixWidth);
		VertexData *ReverseTransferTextureData(VertexData *Vert, TextureData *TxData);
		double Exaggerate(double Elev)	{return (ElevDatum + (Elev - ElevDatum) * Exageration);};
	}; // class RenderData

class Renderer
	{
	friend class RenderControlGUI;
	friend class ExportControlGUI;
	friend class RenderData;
	friend class RenderPreviewGUI;
	friend class ViewGUI;
	friend class PostProcText;
	friend class PostProcImage;
	friend class ExportFormatKML;
	friend class RendererQuantaAllocatorErrorFunctor;
	friend class EffectEval;

	private:
		RenderJob *Job;
		WCSApp *AppBase;
		EffectsLib *EffectsBase;
		ImageLib *ImageBase;
		Database *DBase;
		Project *ProjectBase;
		MessageLog *LocalLog;
		BufferNode *Buffers;
		Raster *Rast, *LastRast;
		RenderPreviewGUI *Preview;
		DrawingFenetre *Pane;
		VertexData *VertexRoot[3];
		VertexDEM *TexRefVert;
		PolygonEdge *EdgeRoot[3];
		DEM *CurDEM;
		VectorPoint *FoliageVertexPtr, *Object3DVertexPtr;
		rPixelBlockHeader *rPixelBlock;
		rPixelHeader *rPixelFragMap;
		VolumetricSubstance *Substances;
		SceneExporter *Exporter;
		RenderQ *DEMCue;
		BusyWin *RemoteGauge;
		#ifdef WCS_VECPOLY_EFFECTS
		EffectEval *EvalEffects;
		static RendererQuantaAllocatorErrorFunctor *MyErrorFunctor;
		static QuantaAllocator *Quanta;
		bool QuantaCreated;
		MaterialList RendererScopeWaterMatList[MAX_MATERIALTABLE_ENTRIES * 3];
		#endif // WCS_VECPOLY_EFFECTS

		PRNGX RendRand;
		RenderData RendData;
		FoliagePlotData FolPlotData;

		double PerturbTable[WCS_MAX_FRACTALDEPTH + 1];
		double *VertexWeight[3];
		unsigned char *PixelWeight, *PixelBitBytes;
		long PixelWeightSize;
		char DEMstr[256], TileStr[64], DisplayBuffer;

	public:
		RenderInterface *Master;
		// F2 NOTE
		// flags should be longs, not chars, due to CPU stall issues
		char IsCamView, IsProcessing, MultiBuffer, ExagerateElevLines, ShadowMapInProgress, MaxFractalDepth, FrdWarned, DefaultFrd,
			ShadowsExist, RasterTAsExist, TerraffectorsExist, StreamsExist, LakesExist, WaterExists, SnowsExist, RenderBathos,
			CmapsExist, EcosExist, EcoRastersExist, VolumetricAtmospheresExist, FoliageEffectsExist, Object3DsExist, LightTexturesExist, 
			AtmoTexturesExist, BackfaceCull, CamPlanOrtho, PlotFromFrags, PhongTerrain;
		long SetupWidth, SetupHeight, Width, Height, OSLowX, OSHighX, OSLowY, OSHighY, DrawOffsetX, DrawOffsetY, PanoPanel, 
			Segment, NumPanels, NumSegments, PolyEdgeSize, RenderImageSegments, XTiles, YTiles,
			FoliageRastNum, Object3DRastNum, StereoSide, StereoStart, StereoEnd, FragmentDepth, CurPolyNumber, LastPolyNumber;
		time_t StartSecs, FirstSecs, FrameSecs, NowTime;
		unsigned long FramesToRender, FramesRendered;
		#ifdef WCS_VECPOLY_EFFECTS
		static unsigned long DEMMemoryCurrentSize;
		static unsigned long FrdDataRows[10];
		static VectorNode *SubNodes[129][129];
		MaterialTable EcoMatTable[MATERIALTABLESIZE], GroundMatTable[MATERIALTABLESIZE], 
			SnowMatTable[MATERIALTABLESIZE], WaterMatTable[MATERIALTABLESIZE], FoliageTintTable[MATERIALTABLESIZE],
			*WaterMatTableStart, *WaterMatTableEnd;
		#endif // WCS_VECPOLY_EFFECTS
		double FieldInterval, MoBlurInterval, StashProjectTime, StashProjectFrameRate, 
			EarthLatScaleMeters, RefLonScaleMeters, TexRefLat, TexRefLon, TexRefElev,
			ElevDatum, Exageration, PlanetRad, EarthRotation, RenderTime, ShadowMinFolHt, ShadowMapDistanceOffset, QMax, ZMergeDistance,
			SegmentOffsetX, SegmentOffsetY, SegmentOffsetUnityX, SegmentOffsetUnityY, MinimumZ, MaximumZ, ZTranslator, InverseFromZ,
			CenterPixelSize, DEMCellSize, TrueCenterX, TrueCenterY, ElScaleMult;
		unsigned char *Bitmap[3], *AABuf, *TreeBitmap[3], *TreeAABuf, *CloudBitmap[3], *CloudAABuf, *FlagBuf, *ObjTypeBuf, *PlotBuf[3], 
			ThresholdValid, Threshold[2];
		long *PolyEdge1, *PolyEdge2;
		float *ZBuf, *TreeZBuf, *CloudZBuf, *LatBuf, *LonBuf, *ElevBuf, *RelElBuf, *ReflectionBuf, *IllumBuf, 
			*SlopeBuf, *AspectBuf, *NormalBuf[3];
		RealTimeFoliageWriteConfig *RealTimeFoliageWrite;
		unsigned short *ExponentBuf;
		RasterAnimHost **ObjectBuf;
		Camera *Cam, *ViewCamCopy;
		RenderOpt *Opt;
		CoordSys *DefCoords;
		Light *ShadowLight;
		RealtimeFoliageData *CurFolListDat,	*FolListDat, *CurLabelListDat,	*LabelListDat;

		TerrainParamEffect *DefaultTerrainPar;
		PlanetOpt *DefaultPlanetOpt;
		EnvironmentEffect *DefaultEnvironment;

		Renderer();
		~Renderer();

		void SetFrdWarned(char NewWarned)	{FrdWarned = NewWarned;};
		char GetFrdWarned(void)				{return(FrdWarned);};

		// in Render.cpp
		int Init(RenderJob *JobSource, WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
			Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
			DrawingFenetre *SuppliedPane, int FirstInit);
		int InitForProcessing(RenderJob *JobSource, WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
			Database *DBSource, Project *ProjectSource, MessageLog *LogSource, int ElevationsOnly);
		void InitTextureData(TextureData *TexData);
		void PlotPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, double *Value);
		void GetPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, double *Value);
		void GetDisplayPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, unsigned char *Value);
		int SetupAndAllocBitmaps(long ImageWidth, long ImageHeight);
		void InitRemoteGauge(char *NewTitle, unsigned long NewMaxSteps, unsigned long NewCurStep);
		void RemoveRemoteGauge(void);
		bool UpdateRemoteGauge(unsigned long Step);
		bool DEMCueReady(void)	{return (DEMCue ? true: false);};
		bool DEMCueGetBounds(double &LowLat, double &LowLon, double &HighLat, double &HighLon);
		void ReportQuantaResources(void);
		void ReportPixelFragResources(void);

		// in RenderMaterials.cpp
		RenderPreviewGUI *GetPreview(void) {return(Preview);};
		void ClosePreview(void);

		// in RenderDEM.cpp
		int MakeAllFractalDepthMaps(int ReportOptimum);
		int ApplyPostProc(PostProcess *ActivePostProc);

	private:
		// in Render.cpp
		int AddBasicBufferNodes(void);
		int AllocateBuffers(void);
		void ClearBuffers(void);
		int ClearCloudBuffers(void);
		int SetImageSize(int InitForProcessingOnly);
		int InitOncePerRender(void);
		void SetDrawOffsets(void);
		void SetInitialTexts(void);
		int RenderFrame(double CurTime, long FrameNumber);
		int InitFrame(double CurTime, long FrameNumber, int SetTime);
		void PlotDemoRainbowFrags(void);
		void PlotProjectedNULLRegionFrags(void);
		void ClearProjectedNULLRegionFrags(void);
		double ComputeCenterPixelSize(void);
		void Cleanup(bool KeepRaster, bool FromShadows, int RenderSuccess, bool OpenDiagnostic);
		#ifdef WCS_VECPOLY_EFFECTS
		QuantaAllocator *AddQuantaAllocator(void);
		void RemoveQuantaAllocator(void);
		bool FreeSomeQAResources(QuantaAllocator *ManagingAllocator, int DemandLevel = 0);
		bool FreeSomeDEMResources(void);
		#endif // WCS_VECPOLY_EFFECTS
		#ifdef WCS_TEST_EARLY_TERRAIN_INIT
		int InitTerrainFactors(void);
		#endif // WCS_VECPOLY_EFFECTS
		void CollapseMap(void);
		DrawingFenetre *OpenPreview(void);
		void MergeZBufRGBPanels(unsigned char **BaseBitmap, float *BaseZBuf, unsigned char *BaseAABuf, 
			unsigned char **AltBitmap, float *AltZBuf, unsigned char *AltAABuf);
		void ScreenPixelPlotFragments(rPixelHeader *Frag, long x, long y);
		void ScreenPixelPlot(unsigned char **Bitmap, long x, long y, long zip);
		void ScreenPixelPlotTwoBuf(unsigned char **BaseBitmap, float *BaseZBuf, unsigned char *BaseAABuf, 
			unsigned char **AltBitmap, float *AltZBuf, unsigned char *AltAABuf, long x, long y, long Zip);
		void UpdatePreview(void);
		void DrawPreview(void);
		void SetMultiBuf(char MBufOn)	{MultiBuffer = MBufOn;};
		void SampleDiagnostics(long X, long Y, long MoveX, long MoveY);
		void EditDiagnosticObject(long X, long Y);
		void SelectDiagnosticObject(long X, long Y);
		void SetDisplayBuffer(unsigned char NewBuf, PostProcess *ProcessMe = NULL);
		void SetDisplayThreshold(unsigned char LowThresh, unsigned char HighThresh);
		void ConvertDiagnostics(int NumBuffers, float **InBuf, unsigned char BufType, PostProcess *ProcessMe = NULL);
		void LogElapsedTime(long CurFrame, bool ShowFrameTimeSummary, bool ShowJobTimeSummary, bool ShowCompletionPopup);
		void CopySettings(Renderer *CopyFrom);
		void SaveDisplayedBuffers(int SaveDefault);
		int AddFoliageList(RealtimeFoliageData *FolDat);
		int AddLabelList(RealtimeFoliageData *LabelDat);
		int ProcessFoliageList(void);
		//int ProcessFoliageListVF(void);
		void DestroyFoliageList(void);
		void SetRBounds(RasterBounds *RBounds, long CurWidth, long CurHeight, long PanoPanels, 
			long ImageSegments, int ConcatenateTiles);

		// in RenderDEM.cpp
		#ifdef WCS_VECPOLY_EFFECTS
		bool SubstituteInMasterLists(VectorPolygon *FindPoly, VectorPolygonListDouble *DEMPolyList, 
			VectorPolygonListDouble *SubList, VectorPolygon *OriginalPoly);
		int RenderTerrain(long FrameNumber, long Field, double CurTime);
		int RenderDEMPoly(int MakeFractalMap, bool DEMNeedsProcessing, VectorPolygon *DEMVecPoly, VectorPolygonListDouble *DEMPolyList, double CurTime);
		int PrepVectorPoly(unsigned long *VertNum, unsigned long PolyNumber, bool OverrideCulling, 
			VectorPolygon *DEMVecPoly, VectorPolygonListDouble *DEMPolyList, double CurTime, bool &RenderIt);
		int TuneVectorPoly(unsigned long PolyNumber, unsigned long PolysPerRow, VectorPolygon *DEMVecPoly, 
			VectorPolygonListDouble *DEMPolyList, double CurTime);
		int RenderVectorPoly(unsigned long PolyNumber);
		void CompleteBoundingPolygons(VectorPolygonListDouble **AdjoiningPolyData,
			unsigned long *AdjoiningPolyNumber, unsigned long PolyNumber, unsigned long PolysPerRow);
		int LinkAdjoiningPolygons(VectorPolygonListDouble **AdjoiningPolyData, 
			unsigned long *AdjoiningPolyNumber, VectorPolygon *MyPolygon, VectorNode *CurNode);
		void CullNonTerrainPolys(VectorPolygonListDouble *&VPCellList, GeneralEffect *TerrainEffect);
		bool TestPolygonInBounds(unsigned long *VertNum, bool &HiddenRemovalSafe);
		int GetFractalLevel(unsigned long *VertNum, unsigned long PolyNumber, bool &RenderIt, bool TestVisibility);
		bool SubdivideTerrainPolygon(VectorPolygonListDouble *&HeadList, unsigned long PolyNumber, int FractalLevel);
		bool GetPolygonBlockRenderStatus(VectorPolygonListDouble *TheList);
		bool GetPolygonRenderStatus(VectorPolygon *TestMe);
		int PrepPolyToRender(VectorPolygon *RenderMe);
		int AnalyzeNodeData(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown);
		int AnalyzeNodeWater(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, 
			bool CellUnknown, bool AddBeachMaterials);
		int AnalyzeNodeElevation(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown);
		int AnalyzeNodeNormals(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown);
		int AnalyzeNodeWaterNormals(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown);
		long FractalLevelPoly(double MaxPixelSize, unsigned long V0, unsigned long LatEntries);
		#else // WCS_VECPOLY_EFFECTS
		int RenderTerrain(long FrameNumber, long Field);
		#endif // WCS_VECPOLY_EFFECTS
		int RenderDEM(int MakeFractalMap);
		int RenderStrayObjects(void);
		int DisplaceDEMVertices(DEM *Map);
		int ApplyFractalDepthTexture(DEM *Map);
		int ScaleDEMElevs(DEM *Map);
		int ProjectDEMVertices(DEM *Map);
		int AllocVertices(void);
		int SubdividePolygon(PolygonEdge *Edge[3], int Frd);
		int PrepPoly(unsigned long V0, unsigned long V1, unsigned long V2, unsigned long PolyNumber);
		int TestInBounds(VertexBase *Vert[3]);
		int FractalizePolygon(PolygonEdge *Edge[3], unsigned char MaxFrd, unsigned char CurFrd, DEM *Map);
		void SetBoundsPackage(BoundsPackage *Conditions);
		int CreateFractalMap(double MaxPixelSize, unsigned long *FractalDepth);
		long FractalLevel(double MaxPixelSize, unsigned long V0, unsigned long V1, unsigned long V2);

		// in RenderSky.cpp
		int RenderSky(void);
		int GetSkyColor(double PixColor[3], double ViewVec[3], double OriginPos[3]);
		int RenderClouds(CloudEffect *CloudSource);
		//int RenderCloudPolygon(VertexCloud *Vert[3], unsigned char *CloudStuffBuf, unsigned char *CloudRGBBuf[3], int LastLayer);
		//int ApplyCloudTranslucency(void);
		//void IlluminateCloudVertex(VertexCloud *Vert, CloudLayerData *LayerData);
		//void MergeCloudLayer(unsigned char *CloudStuffBuf, unsigned char *CloudRGBBuf[3]);
		int RenderStars(void);
		int RenderCelestial(void);
		int PlotCelest(VertexDEM *Vert, double WidthInPixels, double Brightness, double Transparency, 
			double CelestColor[3], Raster *Rast, GeneralEffect *CurObject, Light *PosSource, int ShowPhase, int DoGauge, 
			int &Abort, unsigned char PlotFlags);
		int RenderVolumetrics(int BeforeReflections);
		int RenderVolumetricsSpeedBoost(int BeforeReflections, int BoostRate);
		void SampleVolumetricRay(long X, long Y, AnimDoubleTime *CurTrans, AnimColorTime *CurColor, float *CurFinalT, 
			TextureData *TexData, PixelData *PixData, VertexDEM *ViewVec, VertexDEM *FragVert, VertexDEM *SubVert,
			int BeforeReflections);
		int RenderVolumetricsNoBoost(int BeforeReflections);
		double IlluminateVolumetric(VolumetricSubstance *BacklightSub, TextureData *TexData, PixelData *PixData, VertexDEM *PointPos, double PointRGB[3], double ShadingAllowed);
		void VolumetricPointShader(TextureData *TexData, PixelData *PixData, VertexDEM *PointPos, double PointRGB[3], double ShadingAllowed);

		// in RenderPoly.cpp
		void CalculateFoliageDissolve(PolygonData *Poly);
		#ifdef WCS_VECPOLY_EFFECTS
		int InterpretLandBumpMaps(PixelData *Pix, PolygonData *Poly, double *VtxWt, long *VertIndex);
		int InterpretWaterBumpMaps(PixelData *Pix, double *VtxWt, long *VertIndex);
		#else // WCS_VECPOLY_EFFECTS
		int InterpretLandBumpMaps(PixelData *Pix, PolygonData *Poly);
		int InterpretWaterBumpMaps(PixelData *Pix, PolygonData *Poly);
		#endif // WCS_VECPOLY_EFFECTS
		int InterpretFenceBumpMaps(PixelData *Pix, PolygonData *Poly);
		int Interpret3DObjectBumpMaps(PixelData *Pix, PolygonData *Poly);
		void PlotCloudPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg);
		void PlotWaterPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg);
		void PlotTerrainPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg);
		void PlotFencePixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg);
		void Plot3DObjectPixel(rPixelBlockHeader *FragBlock, PixelData *Pix, PolygonData *Poly, long X, long Y, long LocalZip, long PixZip, double NewWt, 
			unsigned long TopPixCovg, unsigned long BotPixCovg);
		void Plot3DObjectPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt,
			unsigned long TopPixCovg, unsigned long BotPixCovg);
		#ifdef WCS_VECPOLY_EFFECTS
		int TerrainVectorPolygonSetup(VectorPolygon *VecPoly, VertexData *Vtx[3]);
		void BuildMaterialListTable(VertexData *Vtx[3], long EffectType, MaterialTable *FillMe, bool AppendTable);
		void BuildWaterMaterialListTable(VertexData *Vtx[3], MaterialTable *WaterTable, MaterialTable *EcoTable);
		void CalculateFoliageDissolve(PolygonData *Poly, MaterialTable *EcoMatTab);
		#endif // WCS_VECPOLY_EFFECTS
		int TerrainPolygonSetup(VertexData *Vtx[3], unsigned char CurFrd);
		int InstigateTerrainPolygon(PolygonData *Poly, VertexData *Vtx[3]);
		int RenderPolygon(PolygonData *Poly, VertexBase *Vtx[3], char PolyType);
		void FillTerrainTextureData(TextureData *TexDat, VertexData *Vert[3], double CenterPixWt[3], 
			long BoxWidth, long BoxHeight, long WtZip, long Xp, long Yp, char PolyType, CloudEffect *Cloud);
		void Fill3DObjectTextureData(TextureData *TexDat, Vertex3D *Vert[3], double CenterPixWt[3], 
			long BoxWidth, long BoxHeight, long WtZip, long Xp, long Yp);
		int PixelFragMemoryError(void);

		// in RenderMaterials.cpp
	public:
		#ifdef SPEED_MOD
		void FASTCALL IlluminatePixel(PixelData *Pix);	// this needs to be called from cloud effect
		#else // SPEED_MOD
		void IlluminatePixel(PixelData *Pix);	// this needs to be called from cloud effect
		#endif // SPEED_MOD
	private:
		double IlluminateAtmoPixel(PixelData *Pix, Light *CurLight);
		void IlluminateExportPixel(PixelData *Pix, Light *CurLight);
		//int RenderFoliage(PolygonData *Poly, VertexData *Vtx[3], Ecotype *Eco, double Opacity, 
		//	double MatCovg, double *VertexWeights, unsigned long SeedOffset, int FolEffect, bool RenderAboveBeachLevel);
		int RenderFoliageFileFoliage(PolygonData *Poly, VertexDEM *FolVtx, FoliagePreviewData *FPD, Ecotype *Ecotp, double Opacity, unsigned long SeedOffset);
		//void PlotFoliage(FoliagePlotData *FolPlotData);
		//void PlotFoliageReverse(FoliagePlotData *FolPlotData);
		void PlotFoliage(void);
		void PlotFoliageReverse(void);
		int Render3DObject(PolygonData *Poly, VertexDEM *Vtx, Object3DEffect *Object3D, 
			double ExtraRotation[3], double ScaleHeight);
		int Composite3DObjectPass(Object3DEffect *Object, Raster *SourceRast, double PassWt, double DxStart, double DyStart);
		void Composite3DObject(Object3DEffect *Object, Raster *SourceRast, double DxStart, double DyStart);
		int RenderLabel(PolygonData *Poly, VertexDEM *LabelVtx, Label *Labl, Joe *CurVec);
		void ReplaceStringTokens(char *MesgCopy, char *OrigMesg, Joe *CurVec, VertexDEM *CurVert);

		// in RenderVector.cpp
		int RenderVectors(long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, rPixelHeader *rPixelFragMap);
		int PrepVectorToRender(Joe *DrawMe, PixelData *Pix, long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, rPixelHeader *rPixelFragMap);
		void RenderVectorInit(double px, double py, UBYTE width, struct VectColor *color);
		void VectorDrawAAPixel(double x, double y, USHORT wu, double dx, double dy, VertexDEM *FromVDEM, PixelData *Pix);
		void VectorDrawWuPixel(double x, double y, USHORT wu, double dx, double dy, VertexDEM *FromVDEM, PixelData *Pix);
		void VectorDrawWuPixel2(int x, int y, USHORT wu, short dx, short dy, VertexDEM *FromVDEM, PixelData *Pix);
		void VectorDrawWuLine(VertexDEM *FromVDEM, VertexDEM *ToVDEM, PixelData *Pix, unsigned short Style);
		void VectCirclePoints(int cx, int cy, int x, int y, UBYTE wu, VertexDEM *FromVDEM, PixelData *Pix);
		void VectMidpointCircle(VertexDEM *FromVDEM, PixelData *Pix);
		void VectorWuCircle(VertexDEM *FromVDEM, PixelData *Pix);
		void VectorWuSquare(VertexDEM *FromVDEM, PixelData *Pix);
		void VectorWuCross(VertexDEM *FromVDEM, PixelData *Pix);

		// in RenderPost.cpp
		int RenderReflections(void);
		int RenderReflections_BEAMTRACE(rPixelHeader *FragMap);
		#ifdef SPEED_MOD
		int FASTCALL RenderReflections_BTFRAGS(rPixelHeader *const FragMap, long StartCol, long EndCol, bool ShowProgress);
		#else // SPEED_MOD
		int RenderReflections_BTFRAGS(rPixelHeader *const FragMap, long StartCol, long EndCol, bool ShowProgress);
		#endif // SPEED_MOD
		void SampleImageReflection(double AreaToSample, double PixCtrX, double PixCtrY, unsigned char *Bitmap[3], double SampleRGB[3]);
		int RenderBackground(void);
		int RenderDepthOfField(void);
		int RenderBoxFilter(void);
		int RenderPostProc(int PreReflection, unsigned char **OptionalBitmaps, PostProcess *ProcessMe, long FrameNum);
		long AddReflFragEntry(PixelFragSort *SortList, long CurEntry, rPixelFragment *AddFrag, double NewFragCovg);
		double AddReflFragEntry(PixelFragSort *SortList, long &CurEntry, double CovgWt, rPixelFragment *AddFrag,
			int ConsiderMask);

		// in RenderShadow.cpp
		#ifdef WCS_VECPOLY_EFFECTS
		int InitForShadows(WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
			Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
			RenderOpt *HostOpt, Renderer *HostRend, long FrameNum, double CurTime);
		int CreateShadowMap(Light *CurLight, int ObjectType, GeneralEffect *CurObj, 
			Joe *CurJoe, VectorPoint *CurVtx, long VtxNum, long FrameNum, double CurTime);
		int RenderShadow(long FrameNum, double CurTime);
		#else // WCS_VECPOLY_EFFECTS
		int InitForShadows(WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
			Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
			RenderOpt *HostOpt, Renderer *HostRend, long FrameNum);
		int CreateShadowMap(Light *CurLight, int ObjectType, GeneralEffect *CurObj, 
			Joe *CurJoe, VectorPoint *CurVtx, long VtxNum, long FrameNum);
		int RenderShadow(long FrameNum);
		#endif // WCS_VECPOLY_EFFECTS
		int RenderCloudShadow(CloudEffect *CurCloud);

		// in RenderMaterials.cpp
		int Render3DObject(Object3DVertexList *ListElement);
		int RenderFoliage(FoliageVertexList *ListElement);
		int RenderFence(FenceVertexList *ListElement);
		int RenderLabel(LabelVertexList *ListElement);
		int ProcessOneFoliageExport(VertexDEM *Vert, Raster *SourceRast, AnimColorTime *ReplaceRGB, int Shade3D,
			double TopElev, double ElevGrad, int CreateClipMap, Light *Lumens, int DoubleSample, long ActualWidth);

		// in PixelManager.cpp
		rPixelHeader *rAllocPixelFragmentMap(void);
		void rFreePixelFragmentMap(void);

		// in RenderForestry.cpp
		#ifdef WCS_FORESTRY_WIZARD
		int RenderForestryFoliage(PolygonData *Poly, VertexData *Vtx[3], Ecotype *Eco, double RenderOpacity, 
			double MatCovg, double *VertexWeights, unsigned long SeedOffset, int FolEffect, bool RenderAboveBeachLevel);
		int RenderForestryEcotype(ForestryRenderData *ForDat);
		int OneStepCloserToRenderingFoliage(ForestryRenderData *ForDat, Foliage *FolToRender, VertexDEM *FolVtx, double *Random, double FolHeight);
		#endif // WCS_FORESTRY_WIZARD

	}; // class Renderer


// Render support classes and defines

#define WCS_VERTEXDATA_FLAG_DISPLACEMENTAPPLIED		(1 << 0)
#define WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED		(1 << 1)
#define WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED		(1 << 2)
#define WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED		(1 << 3)
#define WCS_VERTEXDATA_FLAG_LAKEAPPLIED				(1 << 4)
#define WCS_VERTEXDATA_FLAG_STREAMAPPLIED			(1 << 5)
#define WCS_VERTEXDATA_FLAG_WAVEAPPLIED				(1 << 6)
#define WCS_VERTEXDATA_FLAG_WATERVERTEX				(1 << 7)
// additional flags available if a separate long flag value is used, value in VertexDEM is a char
#define WCS_VERTEXDATA_FLAG_ELEVATION				(1 << 8)
#define WCS_VERTEXDATA_FLAG_RELEL					(1 << 9)
#define WCS_VERTEXDATA_FLAG_SLOPE					(1 << 10)
#define WCS_VERTEXDATA_FLAG_ASPECT					(1 << 11)
#define WCS_VERTEXDATA_FLAG_NORMAL					(1 << 12)
#define WCS_VERTEXDATA_FLAG_WATERELEV				(1 << 13)
#define WCS_VERTEXDATA_FLAG_WATERDEPTH				(1 << 14)
#define WCS_VERTEXDATA_FLAG_WAVEHEIGHT				(1 << 15)

enum
	{
	WCS_PIXELTYPE_TERRAIN,
	WCS_PIXELTYPE_WATER,
	WCS_PIXELTYPE_CLOUD,
	WCS_PIXELTYPE_CELESTIAL,
	WCS_PIXELTYPE_FOLIAGE,
	WCS_PIXELTYPE_3DOBJECT,
	WCS_PIXELTYPE_FENCE
	}; // PixelTypes

enum
	{
	WCS_POLYGONTYPE_TERRAIN,
	WCS_POLYGONTYPE_WATER,
	WCS_POLYGONTYPE_CLOUD,
	WCS_POLYGONTYPE_3DOBJECT,
	WCS_POLYGONTYPE_FENCE
	}; // PixelTypes

enum
	{
	WCS_POLYSORTTYPE_DEM,
	WCS_POLYSORTTYPE_3DOBJECT,
	WCS_POLYSORTTYPE_FOLIAGE,
	WCS_POLYSORTTYPE_FENCE,
	WCS_POLYSORTTYPE_LABEL
	}; // Poly sort types

struct TerrainPolygonSort
	{
	unsigned char PolyType;
	unsigned long PolyNumber;
	float PolyQ;
	}; // struct TerrainPolygonSort

// in RenderDEM.cpp
class RenderQItem
	{
	public:
		Joe *QJoe;
		JoeDEM *QJoeDEM;
		DEM *QDEM;
		#ifdef WCS_VECPOLY_EFFECTS
		VectorPolygonListDouble *DEMPolyList;
		VectorPolygon *DEMVectorPoly;
		bool PolygonListformed;
		#endif // WCS_VECPOLY_EFFECTS
		float QDist;
		char RenderMe, MaxFrd;
		
		RenderQItem();
		~RenderQItem();

	}; // class RenderQItem

// in RenderDEM.cpp
class RenderQ
	{
	public:
		RenderQItem *TheQ;
		long NumItems, ActiveItems;
		
		RenderQ();
		~RenderQ();
		int FillList(Database *DBase, int Draw0Render1);
		int SortList(Camera *Cam, int BoundsDiscrim, BoundsPackage *Conditions);
		RenderQItem *GetFirst(long &QNumber);
		RenderQItem *GetNext(long &QNumber);
		bool GetBounds(double &LowLat, double &LowLon, double &HighLat, double &HighLon);

	}; // class RenderQ

class TwinMaterials
	{
	public:
		double Covg[2];
		MaterialEffect *Mat[2];

		TwinMaterials()	{Mat[0] = Mat[1] = 0; Covg[0] = Covg[1] = 0.0;};

	}; // class TwinMaterials

// in RenderDEM.cpp
class PolygonEdge
	{
	public:
		VertexData *End[2], *Mid;
		PolygonEdge *Child[2], *StepChild[2], *Parent;
		unsigned char Frd, Displaced;

		PolygonEdge(PolygonEdge *NewParent);
		~PolygonEdge();
		int Subdivide(void);

	}; // class PolygonEdge

// in Render.cpp
class PolygonData
	{
	public:
		double Lat, Lon, Elev, WaterElev, 
			Slope, Aspect, // slope and aspect are in degrees
			Z, Q, OverstoryDissolve[2], UnderstoryDissolve[2], 
			SnowCoverage, Area, ReceivedShadowIntensity, ShadowOffset, RenderPassWeight, 
			RGB[3], Normal[3], ViewVec[3];
		TwinMaterials BaseMat, SnowMat, GroundMat;

		EcosystemEffect *Eco;
		LakeEffect *Lake;
		StreamEffect *Stream;
		CmapEffect *Cmap;
		EnvironmentEffect *Env;
		GroundEffect *Ground;
		SnowEffect *Snow;
		Fence *Fnce;
		CloudEffect *Cloud;
		CloudLayerData *CloudLayer;
		AnimMaterialGradient *Beach;
		Joe *Vector;
		RasterAnimHost *Object;
		Raster *Plot3DRast;

		VertexReferenceData *VertRefData[3];
		float RelEl, VectorSlope;
		unsigned long LatSeed, LonSeed, ShadowFlags;
		long PlotOffset3DX, PlotOffset3DY, VtxNum[3];
		char TintFoliage, ShadeType, VectorType, RenderCmapColor, LuminousColor, FenceType;

		PolygonData();
		void ExtractMatTableData(MaterialTable *MatTable);

	}; // class PolygonData

// in Render.cpp
class PixelData
	{
	public:
		double RGB[3], SpecRGB[3], Spec2RGB[3], Normal[3], XYZ[3], ViewVec[3], Elev, Lat, Lon, Q, Illum, CloudShading,
			Luminosity, Transparency, Specularity, SpecularExp, Specularity2, SpecularExp2, Translucency, TranslucencyExp,
			Reflectivity, WaterDepth, OpticalDepth, OpacityUsed, SpecularWt, SpecularWt2, TranslucencyWt, 
			ReceivedShadowIntensity, ShadowOffset, SpecRGBPct, Spec2RGBPct, AltBacklightColorPct, AltBacklightColor[3];
		TextureData *TexData;
		Light *IgnoreLight;
		RasterAnimHost *Object;
		float Zflt;
		unsigned long ShadowFlags;
		long PixZip;
		char PixelType, UseSpecColor;	// used to elliminate certain material evaluation steps

		PixelData()	{
					ShadowOffset = Normal[2] = Normal[1] = Normal[0] = 0.0;
					ReceivedShadowIntensity = 1.0; AltBacklightColorPct = 0.0;
					TexData = NULL; IgnoreLight = NULL; Object = NULL;
					ShadowFlags = 0; 
					};
		void SetDefaults(void);

	}; // class PixelData

struct RenderPolygon3D
	{
	double qqq;
	Polygon3D *Poly;
	}; // struct RenderPolygon3D

// in RenderDEM.cpp
class DEMBoundingBox
	{
	public:
		VertexDEM Vtx[13]; // Start at SE, go clockwise, 0-3 are minel, 4-7 are maxel, 8-11 are actual el

		void TransformToScreen(Camera *Cam, BoundsPackage *Conditions);

	}; // class DEMBoundingBox

// in RenderDEM.cpp
class BoundsPackage
	{
	public:
		double QMax, ZMax, Exageration, ElevDatum, PlanetRad, EarthLatScaleMeters;
		CoordSys *DefCoords;
		long OSLowX, OSHighX, OSLowY, OSHighY;

		BoundsPackage();

	}; // class BoundsPackage

//enum
//	{
//	WCS_SHADOWTYPE_TERRAIN,
//	WCS_SHADOWTYPE_FOLIAGE,
//	WCS_SHADOWTYPE_CLOUD,
//	WCS_SHADOWTYPE_3DOBJECT
//	}; // shadow map types

// in RenderShadow.cpp
class ShadowMap3D
	{
	public:
		double CenterX, CenterY, HorScale, ZOffset, DistanceOffset;
		VertexDEM VP, RefPos;
		Matx3x3 SolarRotMatx;
		char MyType;
		Raster *Rast;
		ShadowMap3D *Next;
		Joe *MyJoe;
		VectorPoint *MyVertex;
		GeneralEffect *MyEffect;
		unsigned char *AABuf;
		float *ZBuf;

		ShadowMap3D();
		~ShadowMap3D();
		int ProjectPoint(double PtXYZ[3], double ScrnCoords[3]);
		double SampleWeighted(long SampleCell, double X, double Y);

	}; // ShadowMap3D

// in EffectObject3D.cpp
int CompareRenderPolygon3D(const void *elem1, const void *elem2);

// in RenderDEM.cpp
int CompareTerrainPolygonSort(const void *elem1, const void *elem2);

//in RenderPost.cpp
int ComparePixelFragSort(const void *elem1, const void *elem2);

// for strata, found in Textures.cpp
extern const short StrataTex[1200];
extern const short StrataCol[1200];

#endif // WCS_RENDER_H
