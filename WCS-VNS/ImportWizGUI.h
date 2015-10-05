// ImportWizGUI.h
// Header for the MegaOneStopImport Wizard
// 09/09/99 FPW2 {the dreaded date}
// Copyright 3D Nature 1999

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMPORTWIZGUI_H
#define WCS_IMPORTWIZGUI_H

struct ImportData;

#include <libxml/xmlreader.h>
#include "Application.h"
#include "Database.h"
#include "Fenetre.h"
#include "ImportThing.h"
#include "Points.h"
#include "NNGrid.h"
#include "Requester.h"
#include "EffectsLib.h"
#include "Log.h"
#include "Exports.h"
#include "DataOpsDefs.h"
#include "RasterAnimHost.h"

enum
	{
	IW_ANIMPAR_GRIDNS,
	IW_ANIMPAR_GRIDWE,
	IW_ANIMPAR_GRIDSIZENS,
	IW_ANIMPAR_GRIDSIZEWE,
	IW_ANIMPAR_WIDTH,
	IW_ANIMPAR_HEIGHT,
	IW_ANIMPAR_N,
	IW_ANIMPAR_S,
	IW_ANIMPAR_E,
	IW_ANIMPAR_W,
	IW_NUMANIMPAR
	}; // "animated" Import Wizard params - used for Smart Numerics on Horizontal Extents page

enum 
	{
	HEADER_NONE = 0,
	HEADER_GTOPO30,
	HEADER_NED,
	HEADER_ARCVIEW,
	HEADER_ARCINFO
	};

struct CtrlPtStats
	{
	double maxX, maxY, minX, minY, sumX, sumX2, sumY, sumY2;
	double deltaX, gridX, meanX, offsetX, pctX, psdX, ssdX;
	double deltaY, gridY, meanY, offsetY, pctY, psdY, ssdY;
	double avglat, ratio, xn, yn;
	unsigned long n;
	};

struct DXF_Pens
	{
	unsigned char r[256];
	unsigned char g[256];
	unsigned char b[256];
	};

// Very important that we list the base classes in this order.
class ImportWizGUI : public WCSModule, public GUIFenetre
	{
	private:
		Project *ProjHost;
		EffectsLib *EffectsHost;
		Database *DBHost;
		MessageLog *LocalLog;
		SBYTE  *OutputData1S, *InputData1S;
		UBYTE  *OutputData1U, *InputData1U;
		SHORT  *OutputData2S, *InputData2S;
		USHORT *OutputData2U, *InputData2U;
		LONG   *OutputData4S, *InputData4S;
		ULONG  *OutputData4U, *InputData4U;
		FLOAT  *OutputData4F, *InputData4F;
		DOUBLE *OutputData8F, *InputData8F;
		TerraGridder *TGinfo;
		unsigned long ActivePanel, CFBranch, PanelNum, *PanelOrder;

	public:
		int ConstructError;

		ControlPointDatum *TC, *CurDat, *FirstDat;
		long ControlPts;
		struct ImportData *DXFinfo;

		long header, rows, cols, bands, depth;
		double XShift, YShift;
		double IWGlobeRad;	// in meters!
		long HeaderType, Num2Load, NumLoaded;

		char AttributeList[2048];	// from shapefiles
		char completename[256+32];
		char noextname[256+32];
		char tried[80];

		char whatitis[80];
		bool MergeLoad;
		char MergeName[80];
		class Pier1 *HeadPier, *Importing, *Templated;
		double minxhrs, minyhrs;
		double RUTMMaxNorthing, RUTMMinNorthing, RUTMMaxEasting, RUTMMinEasting;	// Region bounds
		double RUTMBoundN, RUTMBoundS, RUTMBoundE, RUTMBoundW;	// Region bound for UTM data sets
		short RUTMZone;											// Region UTM zone (must all be same)
		CtrlPtStats  cpstats;
		bool UseGUI;
#ifdef WCS_BUILD_VNS
		CoordSys *DefCoordSys, *NewCoordSys;
#endif // WCS_BUILD_VNS

		AnimDoubleTime AnimPar[IW_NUMANIMPAR];

		ImportWizGUI(Database *DBSource, Project *ProjSource, EffectsLib *EffectsHost, MessageLog *LogInit, bool ShowGUI = FALSE);
		~ImportWizGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		void AutoGrid(void);
		int AutoImport(Pier1 *HeadPier);
		short AvgShapeStuff(Joe *FileParent, Joe *AverageGuy, Joe *AfterGuy, FILE *fDBF);
		void CalcUTMCorners(struct DEMInfo *UTMInfo, double *minx, double *maxx, double *miny, double *maxy);
#ifdef WCS_BUILD_VNS
		void ChangeCoordSys(void);
#endif // WCS_BUILD_VNS
		short CheckGTOPO30(void);
		short CheckNED_Bin(void);
		short CheckNED_FP(void);
		void CloseSDTSDBGroup(void);
		void CloseUSGSDBGroup(void);
		void ConfigureGPSFieldsList1(void);
		void ConfigureGPSFieldsList2F(void);
		void ConfigureGPSFieldsList2D(void);
		void ConfigureGPSFieldsList3(void);
		void ConfigureWidgets(void);
		bool ControlFlow(unsigned long PanelIndex, bool forward);
		void CopyCoordSys(CoordSys *TempCS);
		short CPSave(void);
//		void CreateSDTSDBGroup(char *Name);
//		void CreateUSGSDBGroup(char *Name);
		short DataOpsAlloc(void);
		short DataOpsCeiling(void);
		void DataOpsCleanup(short error);
		short DataOpsDEMInit(void);
		void DataOpsDimsFromBounds(void);
//		char DataOpsEarlySave(void);
		char DataOpsEarlyDEMSave(void);
		void DataOpsErrMsg(short error);
		void DataOpsFlipYAxis(unsigned char *buffer, long rowsize, long numrows);
		short DataOpsFloor(void);
		void DataOpsInit(void);
		short DataOpsInvert(void);
		short DataOpsNull2Min(void);
		short DataOpsResample(void);
		char DataOpsSameFormat(void);
		short DataOpsSaveOutput(void *OutputData, long OutputDataSize, short i, short j, long rows, long cols,
			long OutputRows, long OutputCols, char *RGBComp);
		short DataOpsScale(void);
		short DataOpsTransform(void);
		bool DBAddSDTS_DEM(char *BaseName, DEM *Topo, float *MapArray, long MapSize);
		short DBAddUSGS_UTMDEM(char *BaseName, DEM *Topo, float *MapArray, long MapSize);
		short DEMLoad(char TestOnly, NativeGUIWin NW);
		void dlg_exit(int code);
//		void DoASCII(void);
		void DoBinFactors(void);
		void DoBinHdrSize(void);
		void DoBinSetPanel(void);
		void DoCellOrderPanel(void);
#ifdef WCS_BUILD_VNS
		void DoCoordSysPanel(void);
#endif // WCS_BUILD_VNS
		void DoElevMethodPanel(void);
		void DoElevUnitsPanel(void);
//		void DoExponential(void);
		void DoGridItPanel(void);
		void DoGPSAttribName(void);
		void DoGPSFieldChange(void);
		void DoGPSStep1Panel(void);
		void DoGPSStep2FPanel(void);
		void DoGPSStep2DPanel(void);
		void DoGPSStep3Panel(void);
		void DoHorExChange(int);
		void DoHorExLabels(void);
		void DoHorExPanel(void);
		void DoHorUnitsPanel(void);
		void DoICRHeadPanel(void);
		void DoIdentPanel(void);
		short DoLoad(NativeGUIWin NW);
		void DoLoadAsPanel(void);
		void DoLonDirPanel(void);
		void DoNameBaseExtension(void);
		void DoNewOutputName(void);
		void DoNewShapeLayerName(void);
		void DoNullPanel(void);
		void DoOverride(void);
		void DoOutParamsPanel(void);
		//void DoOutRegPanel(void);
		void DoOutTilesPanel(void);
		void DoOutFormPanel(void);
		void DoOutTypePanel(void);
		void DoOutScalePanel(void);
		void DoOutValueBytes(void);
		void DoOutValueFormat(void);
		void DoPreProPanel(void);
		void DoRefCoPanel(void);
		void DoSaveGeoPanel(void);
		void DoShapeOptsPanel(void);
		void DoTest(void);
		void DoTestButton(NativeGUIWin NW);
		void DoTestPanel(void);
		void DoText(void);
		void DoValBytes(void);
		void DoValFmt(void);
		void DoVertExPanel(void);
		void DoWCSKnowsPanel(void);
		void DoWCSUnitsPanel(void);
		void DoWrapPanel(void);
		short ExportSettings(FILE *log, Pier1 *DumpMe);
		short ExtractUTM_DEM(void);
		short ExtractGeo_DEM(void);
		void FinishOuttype(void);
		void FreeDB3LayerTable(DB3LayerList *List, long NumLayers);
		float GaussFix(float *UTMArray, long UTMRows, long Col2Do, long Row2Do);
		void GenericBinary(void);
		void GenGPSParsing(void);
		short GetInputFile(void);
		short GetSECornerFromUser(void);
//		short GetSWCornerFromFiller(struct USGS_DEMHeader *USGSHdr);
		short GetUTMBounds(struct DEMInfo *Info);
		short GetWCSInputFile(void);
		bool GoodBounds(void);
		void GPSParseLineD(char *string, long passNum);
		void GPSParseLineF(char *string, long passNum);
		void GPX_Node(xmlTextReaderPtr reader);
		void GPX_Route(xmlTextReaderPtr reader);
		void GPX_Waypoint(Joe *loadRoot, xmlTextReaderPtr reader, xmlChar *latStr, xmlChar *lonStr);
		char *GuessBetter(void);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListViewColSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleTextColumnMarkerChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long ChangeMode);
		bool IdentByExtension(char *ext);
		void IdentByAssociation(char *fullbasename);
		unsigned long Import(char *FileName, Joe *Level, unsigned char Type);
		unsigned long ImportDLG(char *FileName, Joe *Level);
		unsigned long ImportDLG(FILE *fDLG, Joe *FileParent);
		unsigned long ImportMeridian2(FILE *fNTF_M2, Joe *Level);

/*		unsigned long ImportGNIS(FILE *fPPL, Joe *FileParent);
		unsigned long ImportPPL(FILE *fPPL, Joe *FileParent);
		unsigned long ImportCOUNTY(FILE *fPPL, Joe *FileParent);
*/
		unsigned long ImportShape(FILE *fShape, FILE *fDBF, FILE *fIndex, Joe *FileParent, char *FileName);
		bool IsTIFFDEM(char *filename);
		short LoadASCII_DEM(char *filename, float *Output, char TestOnly);
		short LoadASCII_GPS(char *filename, char TestOnly);
		short LoadARCBinaryADFDEM(char *filename, float *Output, char TestOnly);
		short LoadARCExportDEM(char *filename, float *Output, char TestOnly);
		short LoadBinary(char *filename, UBYTE *Output, char TestOnly);
		short LoadBryceDEM(char *filename, unsigned short *Output, char TestOnly);
		short LoadDEMImage(char *filename, unsigned long *Output, char TestOnly, unsigned char valueFmt = DEM_DATA_FORMAT_UNSIGNEDINT);
		short LoadDTED(char *filename, short *Output, char TestOnly);
		short LoadGPX(char *filename, char TestOnly);
		void LoadingMsg(void);
		short LoadLandFormPanoramaDTM(FILE *input, float *Output, char TestOnly);
		short LoadLandFormProfileDTM(FILE *input, float *Output, char TestOnly);
		short LoadMDEM(char *filename, short *Output, char TestOnly);
#ifdef WCS_BUILD_VNS
		short LoadNTF_DTM(char *filename, float *Output, char TestOnly);
		short LoadNTF_Meridian2(char *filename, float *Output, char TestOnly);
		short LoadNTFMeridian2DTM(FILE *input, float *Output, char TestOnly);
		short LoadNTFMeridian2Vect(FILE *input, float *Output, char TestOnly);
#endif // WCS_BUILD_VNS
		short LoadSDTS_DEM(char *filename, float *Output, char TestOnly);
		short LoadSDTS_DLG(bool TestOnly);
		short LoadSurfer(char *filename, float *Output, char TestOnly);
		short LoadTerragen(char *filename, float *Output, char TestOnly);
		short LoadTIFFDEM(char *filename, float *output, char testOnly);
		short LoadTIN(char *filename, char TestOnly);
		short LoadUSGS_DEM(char *filename, float *InputData, char TestOnly);
//		short LoadUSGS_DLG(char *filename, bool TestOnly);
		short LoadVistaDEM(char *filename, short *Output, char TestOnly);
		short LoadWCS_ZBuffer(char *filename, char *Output, char TestOnly);
		short LoadWCSDEM(void);
		short LoadWXYZ_CP(char *filename, char TestOnly);
		short LoadXYZ_CP(char *filename, char TestOnly);
		short LoadXYZ_WYXZ_CP(FILE *phyle, char TestOnly, bool LoadLinks);
		short Match250FileName(char *InputName, char *OutBaseName);
		void NextButton(NativeGUIWin NW);
		long ParseField(long fieldNum);
		void PrevButton(void);
#ifdef WCS_BUILD_VNS
		void ReadArcPrj(char *PathAndName);
		void ReadArcPrj(FILE *input);
#endif // WCS_BUILD_VNS
		void ReadGPSAttribField(Joe *theJoe, const char *string, long index);
		double ReadGPSElevField(const char *elevString);
		short ReadGPSFileType0(char *filename);
		short ReadGPSFileType1(char *filename);
		short ReadGPSFileType2(char *filename);
		short ReadGPSFileType3(char *filename);
		double ReadGPSLatField(const char *latString);
		double ReadGPSLonField(const char *lonString);
		void ReadGPSLabelField(const char *string);
		void ReadGPSLayerField(char *string);
		void ReadGPSNameField(const char *string);
		bool ReadMDEMHdr(MDEM_Hdr *hdr, FILE *fin);
		short Read_USGSHeader(FILE *DEM, struct USGS_DEMHeader *Hdr);
		short Read_USGSProfHeader(FILE *DEM, struct USGS_DEMProfileHeader *ProfHdr);
		short Read_USGSDEMProfile(FILE *DEM, float *ProfPtr, short ProfItems, double ElevFactor, double ElevDatum, double ElMax, double ElMin);
		bool ReadBinHeader(FILE *binhdr);
		long ReadDB3LayerTable(FILE *fDBF, DB3LayerList *&LayerList, unsigned short &RecordLength);
		int ReadDB3Record(FILE *fDBF, char *DBRecord, unsigned short RecordLength);
		bool ReadSDTS_CellRange(void);
		bool ReadSDTS_DEM(struct DEMExtractData *DEMExtract);
		bool ReadSDTS_DEM_LayerDef(void);
		bool ReadSDTS_DEM_SpatDomain(void);
		bool ReadSDTS_Elevations(FILE *DEM, float *ProfPtr, short Rows, short Columns, long start_y, 
			long start_x, double ElevFactor, int voidvalue, int fillvalue, struct DEMExtractData *DEMExtract);
		bool ReadSDTS_ElevUnits(void);
		bool ReadSDTS_Ident(void);
		bool ReadSDTS_IRef(void);
		bool ReadSDTS_RasterDef(void);
		bool ReadSDTS_XRef(void);
		bool ReadUSGSProjection(FILE *usgsproj);
		bool ReadWorldFile(char *FilePath);
		bool ReadWorldFile(FILE *tfw);
		bool ReadWorldFiles(void);
		void ResetCPStats(void);
		int SaveImageFile(long FileFormat, char *OutFileName, long Width, long Height);
		unsigned long ScanDXF(void);
		void SetGPSDelims(void);
		void SDTS_degenlines(int status);
		void SDTS_dlg_areas(int status);
		void SDTS_dlg_geo_mbr(int status);
		void SDTS_dlg_get_iref(void);
		void SDTS_dlg_get_xref(void);
		void SDTS_dlg_head(int status);
		void SDTS_dlg_linecount(int status);
		void SDTS_dlg_lines(int status);
		void SDTS_dlg_loadpoints(int status);
		void SDTS_dlg_mbr(int status);
		void SDTS_dlg_more_header(int status);
		void SDTS_dlg_nodes(int status);
		void SDTS_dlg_proj(int status);
		void SelectNewCoords(void);
		void SelectPanel(unsigned long PanelID);
		void SetBoundsType(char BoundTypeID);
		void SetDatum(char *name);
		void SetElevMethodHeights(void);
		void SetElevUnits(void);
		bool SetFileNames(void);
		void SetGridSpacing(void);
		void SetMetrics(void);
		void SetSNDefaults(void);
		// SetNullObject is used for SHAPE import
		Joe *SetNullObject(char *Name, char *Layer, char Style, char Color);
		// SetSHPObjTriple is currently only used in ImportShape
		Joe *SetSHPObjTriple(char *Name, long Pairs, char Style, unsigned char Color, double *Lat, double *Lon, float *Elev, bool isPoint = false);
		void SetPanelOrder(void);
		void SetUTMZone(int zone);
		void SetHScale(long cursel);
		void SetVScale(long cursel);
		void ShowID(char *whatitis);
		bool Sniff3DS(FILE *phyle);
		bool SniffArcASCIIArray(FILE *phyle);
		bool SniffArcBinaryADFGrid(FILE *phyle, char *completename);
		bool SniffArcExportDEM(FILE *phyle);
		SBYTE SniffASCII(FILE *phyle);
		bool SniffASCIIArray(FILE *phyle);
		bool SniffBIL(FILE *phyle);
		bool SniffBinary(FILE *phyle);
		bool SniffBIP(FILE *phyle);
		bool SniffBMP(FILE *phyle);
		bool SniffBryce(FILE *phyle);
		bool SniffBSQ(FILE *phyle);
		bool SniffDTED(FILE *phyle, const char *completename);
		bool SniffDXF(FILE *phyle);
		bool SniffGPS(FILE *phyle, bool force = false);
		bool SniffGPX(FILE *phyle);
		bool SniffGTOPO30(FILE *phyle, char *completename);
		bool SniffGZIP(FILE *phyle);
		bool SniffIFF(FILE *phyle);
		bool SniffLWOB(FILE *phyle);
		bool SniffMDEM(FILE *phyle);
		bool SniffNEDBinary(FILE *phyle, char *completename);
		bool SniffNEDGridFloat(FILE *phyle, char *completename);
		bool SniffNTFDTM(FILE *phyle);
		bool SniffNTFMeridian2(FILE *phyle);
		bool SniffPICT(FILE *phyle);
		bool SniffSDTSDEM(FILE *phyle, char *completename);
		bool SniffSDTSDLG(FILE *phyle, char *completename);
		bool SniffShapefile(FILE *phyle, char *completename);
		bool SniffSRTM(FILE *phyle, char *filename);
		bool SniffSTM(FILE *phyle);
#ifdef GO_SURFING
		bool SniffSurfer(FILE *phyle);
#endif // GO_SURFING
		bool SniffTarga(char *fullname);
		bool SniffTerragen(FILE *phyle);
		bool SniffUSGS_DEM(FILE *phyle);
		bool SniffUSGS_DLG(FILE *phyle);
		bool SniffVistaPro(FILE *phyle);
		bool SniffWCS_3Dobj(FILE *phyle);
		bool SniffWCS_DEM(FILE *phyle);
		bool SniffWCS_Proj(FILE *phyle);
		bool SniffWCS_ZBuffer(FILE *phyle);
		bool SniffWXYZ(FILE *phyle);
		bool SniffXYZ(FILE *phyle);
		bool SniffZip(FILE *phyle);
		void Snoopy(char *loadname, char *thisload, unsigned char *load_as);
#ifdef WCS_BUILD_VNS
		void SyncCoordSys(void);
#endif // WCS_BUILD_VNS
		void SyncGrid(void);
		void TryAllImportable(FILE *unknown);
		void UpdateGPSAttribName(void);
		short USGSDEMFile_Save(char *BaseName, DEM *Topo, float *MapArray, long MapSize);
		void UTMZoneNum2DBCode(long *utmzone);
		short VectLoad(char TestOnly);
		bool VNS_HorexCheckFails(void);
		void WidgetSetDisabledRO(WIDGETID WidgetID, char Disabled, NativeGUIWin DestWin = NULL);
	}; // class ImportWizGUI

void InitDXFPens(struct DXF_Pens *dxfpens);

#endif // WCS_IMPORTWIZGUI_H
