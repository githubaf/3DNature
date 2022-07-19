/* Proto.h
** Prototypes for all functions in GIS source code
** Renamed from gis_proto.h on 14 Jan 1994 by CXH
** Built on 24 Jul 1993 from gis.c and gisam.c by Chris "Xenon" Hanson.
** Original code by Gary R. Huber.
*/

#ifndef WCS_PROTO_H
#define WCS_PROTO_H

#include <SDI_compiler.h>
typedef CONST unsigned char  *CONST_STRPTR;

#include "RexxSupport.h"

#include <SDI_compiler.h>

typedef double Matx3x3[3][3]; // moved to WCS.h

#ifndef __SASC   // ALEXANDER: SAS/C meckert, wenn ich ie forward deklarationen mache???
// forward declarations
struct ZBufferHeader;
struct ILBMHeader;
struct ColorComponents;
struct elmapheaderV101;
struct DEMConvertData;
struct DEMInterpolateData;
struct DEMExtractData;
struct USGS_DEMHeader;
struct USGS_DEMProfileHeader;
struct DLGInfo;
struct ScaleKeyInfo;
struct ParHeader;
union KeyFrameV1;
struct ParHeaderV1;
struct SettingsV1;
union EnvironmentV1;
struct ParHeaderV1;
struct SettingsV1;
union EnvironmentV1;
struct PaletteV1;
struct AnimationV1;
struct FaceData;
struct VertexIndex;
struct faces;
struct WaveData;
struct RenderAnim;
struct LightWaveInfo;
struct LightWaveInfo;
struct LightWaveMotion;
struct coords;
struct RenderAnim;
struct Box;
struct VelocityDistr;
struct Gauss;
struct Vertex;
struct clipbounds;
struct lineseg;
struct poly4;
struct UTMLatLonCoords;
struct AlbLatLonCoords;
union KeyFrame;
struct KeyTable;
struct WCSScreenData;
struct WindowKeyStuff;
struct TimeLineWindow;
struct GUIKeyStuff;
struct QCvalues;
struct Wave;
struct CloudWindow;
struct NNGrid;
struct PaletteItem;
struct Foliage;
struct Ecotype;
struct FoliageGroup;
struct MapData;
struct MaxMin3;
struct WaveWindow;
struct simp;
struct temp;
struct neig;
#endif

/* Bitmaps.c */
extern short openbitmaps(UBYTE **bitmap, long zsize);
extern short savebitmaps(UBYTE **bitmap, long zsize, short renderseg);
extern void closebitmaps(UBYTE **bitmap, long zsize);
extern void allocQCmaps(void);
extern void freeQCmaps(void);
extern short saveILBM(short saveRGB, short AskFile, struct RastPort *RPort,
	UBYTE **bitmap, long *scrnrowzip, short renderseg, short segments,
	short concat, short scrnwidth, short scrnheight);
extern short SaveZBuf(short zformat, short renderseg, long numpts,
	UBYTE *ScratchPad, float *ZBuf, char *Name, long Rows, long Cols);
extern void ModFileName(char *tempfile, short renderseg, short imagearray,
	short imagenum);
extern short LoadImage(char *Name, short ColorImage, UBYTE **bitmap,
	short Width, short Height, short SupressWng,
	short *NewWidth, short *NewHeight, short *NewPlanes);
/*extern short LoadZBuf(char *Name, float *ZBuf, struct ZBufferHeader *ZBHdr,
	short Width, short Height);*/ // used locally only -> static, AF 20.7.2021
extern short CheckIFF(long fh, struct ILBMHeader *Hdr);
extern short FindIFFChunk(long fh, struct ILBMHeader *Hdr, char *Chunk);
extern short MergeZBufBack(short renderseg, short Width, short Height, struct Window *win);
extern short InterlaceFields(char *Name, short Width, short Height, short Dominance);

/* Cloud.c */
extern struct CloudLayer *CloudLayer_New(void);
//extern void CloudLayer_Del(struct CloudLayer *CL); // AF, not used 26.July 2021
//extern void CloudLayer_DelAll(struct CloudLayer *CL); // used locally only -> static, AF 19.7.2021
extern void CloudLayer_SetDouble(struct CloudLayer *CL, ULONG Item, double Val);
extern void CloudLayer_SetShort(struct CloudLayer *CL, ULONG Item, short Val);
extern struct CloudData *CloudData_New(void);
extern void CloudData_Del(struct CloudData *CD);
extern void CloudData_SetLong(struct CloudData *CD, ULONG Item, long Val);
extern void CloudData_SetShort(struct CloudData *CD, ULONG Item, short Val);
extern void CloudData_SetDouble(struct CloudData *CD, ULONG Item, double Val);
extern long CloudData_GetLong(struct CloudData *CD, ULONG Item);
extern short CloudData_GetShort(struct CloudData *CD, ULONG Item);
extern double CloudData_GetDouble(struct CloudData *CD, ULONG Item);
extern short Cloud_SetBounds(struct CloudData *CD);
extern short Cloud_Generate(struct CloudData *CD, double Frame);
extern short Cloud_Draw(struct CloudData *CD);
extern void Cloud_SetDefaults(struct CloudData *CD, short CloudType, short SetAll);
extern void CloudWave_Init(struct CloudData *CD, short Frame);
extern short BuildCloudKeyTable(struct CloudData *CD);
/* see GenericParams.h
extern short BuildGenericKeyTable(struct KeyTable **KTPtr, union KeyFrame *KF,
	short NumKeys, short *MaxFrames, short Group, short Item, short Elements);
extern void SetGenericKeyTableEntry(union KeyFrame *KF, union KeyFrame **Key,
	short NumKeys, short Group, short Item);
extern void FreeGenericKeyTable(struct KeyTable **KTPtr, short *MaxFrames);
extern short SplineGenericKeys(struct KeyTable **KTPtr, short *MaxFrames, short Elements);
*/

/* ColorBlends.c */
extern short ecoset(short i, short notsnow, struct ColorComponents *CC);
extern short WaterEco_Set(short MakeWater, struct ColorComponents *CC);
// extern void seashoal(struct ColorComponents *CC); // used locally only -> static, AF 26.7.2021
extern void colmapavg(struct elmapheaderV101 *map, short colpts, struct ColorComponents *CC);
// extern void SetScreenColor(short ecotype); // used locally only -> static, AF 26.7.2021
extern short ComputeTexture(double ElPt, double LatPt, double LonPt, double ElY);
extern short ComputeTextureColor(double ElPt, double LatPt, double LonPt, double ElY,
	struct ColorComponents *CC);
// extern short ComputeBumpMapTexture(double LatPt, double LonPt); // used locally only -> static, AF 26.7.2021
// extern long MakeNoise(UBYTE *NoiseMap, long MaxNoise, double Lat, double Lon); // used locally only -> static, AF 26.7.2021
extern double DEM_InterpPt(struct elmapheaderV101 *Map, double Lat, double Lon);
// extern void StrataConvert(void); // used locally only -> static, AF 26.7.2021

/* Database.c */
extern short Database_Load(short lowi, char *filename);
extern short makedbase(short SaveNewDBase);
extern short savedbase(short restorename);
// extern struct database *DataBase_New(short Records); // used locally only -> static, AF 20.7.2021
/* extern struct database *DataBase_Copy(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords);*/ // used locally only -> static, AF 20.7.2021
extern struct database *DataBase_Expand(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords);
extern void DataBase_Del(struct database *DelBase, short Records);
extern short loadmapbase(short lowi, short onlyselected);
extern short Load_Object(short i, char **LastDir);
// extern short Find_DBObjPts(char *filename); // used locally only -> static, AF 20.7.2021
extern struct DirList *DirList_New(char *firstpath, short ReadOnly);
extern short DirList_Add(struct DirList *DLOld, char *addpath, short ReadOnly);
extern struct DirList *DirList_Remove(struct DirList *DLOld, short Item);
extern struct DirList *DirList_Search(struct DirList *DLOld, short Item);
extern short DirList_ItemExists(struct DirList *DLItem, char *Item);
extern void DirList_Move(struct DirList *DLOld, short Move, short MoveTo);
extern void DirList_Del(struct DirList *DLDel);
extern struct DirList *DirList_Copy(struct DirList *DLOld);
extern short DBaseObject_New(void);
extern short DBaseObject_Add(void);
/*extern void ConstructDEMObj(void);*/
extern short DBase_SaveComposite(void);
// extern short DBase_LoadComposite(void); // used locally only -> static, AF 20.7.2021

/* DataOps.c */
extern void ConvertDEM(struct DEMConvertData *data, char *filename, short TestOnly);
// extern short SaveConvertOutput(struct DEMConvertData *data, struct elmapheaderV101 *DEMHdr,
//	void *OutputData, long OutputDataSize, short i, short j,
//	long rows, long cols, long OutputRows, long OutputCols, char *RGBComp); // used locally only -> static, AF 26.7.2021

/* DEM.c */
extern short readDEM(char *filename, struct elmapheaderV101 *map);
//extern short saveDEM(char *filename, struct elmapheaderV101 *Map, short *Array); // used locally only -> static, AF 23.7.2021
extern short makerelelfile(char *elevpath, char *elevfile);
// extern void padarray(short *arrayptr, struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
// extern short enterbox(void); // used locally only -> static, AF 23.7.2021
// extern short computerelel(short boxsize, short *arrayptr, struct elmapheaderV101 *map); // used locally only -> static, AF 19.7.2021
extern short InterpDEM(struct DEMInterpolateData *DEMInterp);
// extern short SplineMap(short *map, short Xrows, short Ycols, double elvar, double flatmax); // used locally only -> static, AF 23.7.2021
extern short ExtractDEM(struct DEMExtractData *DEMExtract);
// extern FILE *DEMFile_Init(struct DEMExtractData *DEMExtract, short k, char *MsgHdr); // used locally only -> static, AF 23.7.2021
extern short DEMFile_Save(char *BaseName, struct elmapheaderV101 *Hdr,
	short *MapArray, long MapSize);
//extern short Read_USGSHeader(FILE *DEM, struct USGS_DEMHeader *Hdr); // used locally only -> static, AF 23.7.2021
//extern short Read_USGSProfHeader(FILE *DEM, struct USGS_DEMProfileHeader *Hdr); // used locally only -> static, AF 19.7.2021
//extern short Read_USGSDEMProfile(FILE *DEM, short *ProfPtr, short ProfItems); // used locally only -> static, AF 23.7.2021
extern double FCvt(const char *string);
extern short FixFlatSpots(short *Data, short Rows, short Cols, float T);
extern short FindElMaxMin(struct elmapheaderV101 *Map, short *Array);
extern short LoadVistaDEM(char *filename, short *Output, short DataPts);
extern short LoadDTED(char *filename, short *Output, long OutputSize);
/*
extern void LandscapeGen(void);
extern void FixDEM(void);
*/

/* DLG.c */
extern short ImportDLG(void);
extern short ImportDXF(void);
// extern short OpenObjFile(char *filename); // used locally only -> static, AF 25.7.2021
// extern void Set_DXFObject(char *name, short pts, char *ptrn); // used locally only -> static, AF 25.7.2021
// extern void Set_DLGObject(struct DLGInfo *DLG); // used locally only -> static, AF 25.7.2021
// extern short AssignDLGAttrs(struct DLGInfo *DLG, char *Prefix); // used locally only -> static, AF 25.7.2021
extern short ImportWDB(void);
/*
extern void *NameHash_New(void);
extern void NameHash_Del(void *HashTable);
extern char *NameHash_AddExisting(void *This, char *ExistingName);
extern char *NameHash_IndexObject(void *This, char *ObjectName);
extern unsigned int NameHash_MakeSubscript(int Index, char *DestBuf);
extern unsigned int NameHash_MakeIndex(int HashDigits, char *SubBuf);
*/
/* EdPar.c */
extern short  CreateBankKeys(void);
// extern short Set_Bank_Key(short Frame); // used locally only -> static, AF 23.7.2021
extern short ScaleKeys(struct ScaleKeyInfo *SKI);
extern void defaultsettings(void);
extern void initmopar(void);
extern void initpar(void);
/*
extern void moparamcheck(short oframe);
extern void paramcheck(short frame, short oframe);
*/
extern void setvalues(void);
extern void boundscheck(short parameter);
extern void setecodefault(short i);
extern void FixPar(short k, short ParSet);
extern void UndoPar(short k, short ParUndo);
extern short loadparams(USHORT loadcode, short loaditem);
//extern short loadparamsV2(USHORT loadcode, short loaditem, char *parampath,
//	char *paramfile, struct ParHeader *TempHdr, short ExistingKeyFrames); // used locally only -> static, AF 23.7.2021
//extern short loadparamsV1(USHORT loadcode, short loaditem, char *parampath,
//	char *paramfile, struct ParHeader *TempHdr, short ExistingkeyFrames); // used locally only -> static, AF 23.7.2021
//extern void ParamFmtV1V2_Convert(struct AnimationV1 *MoParV1, struct PaletteV1 *CoParV1,
//	union EnvironmentV1 *EcoParV1, struct SettingsV1 *settingsV1,
//	struct ParHeaderV1 *ParHdrV1, union KeyFrameV1 *KFV1, USHORT loadcode,
//	short loaditem, short LoadKeys); // used locally only -> static, AF 23.7.2021
extern short saveparams(USHORT savecode, short saveitem, short savecur);
extern short DefaultParams(void);
// extern void SetParColor(short Color, char *Name, short Red, short Grn, short Blu); // used locally only -> static, AF 26.7.2021
/*extern void SetParEco(short Eco, char *Name, short Line, short Skew, short SkewAz,
	short RelEl, short MaxRelEl, short MinRelEl, short MaxSlope,
	short MinSlope, short Color, short Understory, short MatchRed,
	short MatchGrn, short MatchBlu, short Type, short TreeDens,
	short TreeHt);*/ // used locally only -> static, AF 19.7.2021
extern void Sort_Eco_Params(void);

/* Foliage.c - more functions in Foliage.h */
extern void DisposeEcotypes(void);

/* Fractal.c */
extern void Recurse(struct elmapheaderV101 *map, struct Window *win,
	short MapAsSFC, struct CloudData *CD, struct FaceData *Data);
extern void FractRecurse(struct Window *win, struct elmapheaderV101 *map,
	short MapAsSFC, struct FaceData *Data, struct faces *Face,
	struct VertexIndex *Vtx, struct CloudData *CD);
// extern void FractPoly_Divide(struct elmapheaderV101 *map, struct VertexIndex *Vtx); // used locally only -> static, AF 23.7.2021
extern double GaussRand(void);
//extern void Poly_Divide(void); // used locally only -> static, AF 23.7.2021
extern void recurse(struct elmapheaderV101 *map, struct Window *win,
	short MapAsSFC, struct CloudData *CD, struct FaceData *Data);
// extern void polydivide(void); // used locally only -> static, AF 23.7.2021
extern void Point_Sort(void);
extern void FractPoint_Sort(double *Elev);
extern void PointSort2(void);
extern void SmoothFace_ColSet(struct elmapheaderV101 *map, struct faces **Face,
	long FaceColSize);
extern void SmoothFace_IncrColSwap(struct elmapheaderV101 *map, struct faces **Face,
	struct faces *MaxBase, long FaceColSize);
extern void SmoothFace_DecrColSwap(struct elmapheaderV101 *map, struct faces **Face,
	struct faces *MaxBase, long FaceColSize);
extern void FaceIndex_Set(struct faces **Face, struct faces **FaceIndex,
	long Lc, short WhichFace);
extern void VertexOne_Set(struct elmapheaderV101 *map, struct faces **FaceIndex,
	struct faces *Vertex);
extern void VertexTwo_Set(struct elmapheaderV101 *map, struct faces **FaceIndex,
	struct faces *Vertex);
// extern void Vertex_Sum(struct faces *Vertex, struct faces *Face); // used locally only -> static, AF 19.7.2021
/*
extern void FaceIndex_Increment(long *FaceIndex, short Incr);
extern void VertexOne_IncrNorth(struct elmapheaderV101 *map, long *FaceIndex,
	struct faces *Vertex);
extern void VertexTwo_IncrNorth(struct elmapheaderV101 *map, long *FaceIndex,
	struct faces *Vertex);
extern void VertexOne_IncrSouth(struct elmapheaderV101 *map, long *FaceIndex,
	struct faces *Vertex);
extern void VertexTwo_IncrSouth(struct elmapheaderV101 *map, long *FaceIndex,
	struct faces *Vertex);
*/
extern void VertexIndexFaceOne_EdgeSet(struct elmapheaderV101 *map,
	struct VertexIndex *Vtx, long MaxFract);
extern void VertexIndexFaceTwo_EdgeSet(struct elmapheaderV101 *map,
	struct VertexIndex *Vtx, long MaxFract);
// extern long MinInt5(long Num1, long Num2, long Num3, long Num4, long Num5); // used locally only -> static, AF 23.7.2021
extern void WaveAmp_Compute(double *Elev, double *Alt,
	double *PolyEl, double *PolyLat, double *PolyLon,
	struct WaveData *WD, short *MakeWater, short *MakeBeach,
	double Frame);

/* GlobeMap.c */
extern void globemap(void);
//extern short InitDEMMap(struct Window *win, struct CloudData *CD); // used locally only -> static, AF 20.7.2021
extern short InitVectorMap(struct Window *win, short zbufplot, short override);
// extern short InitCloudMap(struct Window *win, struct CloudData *CD); // used locally only -> static, AF 26.7.2021
//extern void Close_Render_Window(void); // used locally only -> static, AF 20.7.2021
extern void Handle_Render_Window(void);

/* GlobeMapSupport.c */
/*extern short computebanking(void);*/
extern double ComputeBanking(short frame);
extern void setview(void);
extern short setquickview(void);
extern short makesky(short renderseg, struct Window *win);
// extern void getskypt(void); // used locally only -> static, AF 23.7.2021
extern void antialias(void);
extern short Reflection_Render(struct Window *win);
extern short Celestial_Bodies(UBYTE **Bitmap, long Width, long Height,
	struct Window *win);
/*extern short HaloEffect(UBYTE **Bitmap, long Width, long Height,
	double Dr, double Dx, double Dy, double Intensity, double NoHaloDist,
	struct ColorComponents *CC, char *NameStr, struct Window *win);*/ // used locally only -> static, AF 19.7.2021
/*extern short Image_Composite(UBYTE **Bitmap, UBYTE **Source, long Iw, long Ih,
	short Sw, short Sh, double Dr, double Dx, double Dy, double Distance,
	double *Visible, double *Luminosity, struct ColorComponents *CC,
	char *NameStr, struct Window *win);*/ // used locally only -> static, AF 23.7.2021
// extern short BuildTrigTables(void); // used locally only -> static, AF 26.7.2021
// extern void FreeTrigTables(void); // used locally only -> static, AF 26.7.2021
extern double ASin_Table(double sine);
extern double ACos_Table(double cosine);
extern double Sin_Table(double arcsine);
extern double Cos_Table(double arcosine);

/* InteractiveDraw.c */
extern void drawgridview(void);
extern short drawgridpts(short erase);
// extern short computeview(struct elmapheaderV101 *Map); // used locally only -> static, AF 19.7.2021
extern void makeviewcenter(short erase);
extern void computequick(void);
extern void DrawInterFresh(short drawgrid);
// extern short drawinterview(void); // used locally only -> static, AF 19.7.2021
extern void drawveryquick(short undraw);
extern void drawquick(short a, short b, short c, short Clear);
extern void constructview(void);
extern void compass(double oldaz, double newaz);
extern short make_compass(void);
extern void computesinglebox(short i);
extern void drawfocprof(short erase);
//extern void computefocprof(void); // used locally only -> static, AF 19.7.2021
extern void Play_Motion(struct RenderAnim *RA);

/* InteractiveView.c */
extern short interactiveview(short new_window);
// extern short openinterview(void); // used locally only -> static, AF 26.7.2021
extern short initinterview(short boundsdiscrim);
extern void closeviewmaps(short CloseAll);
extern void closeinterview(void);
extern void shaderelief(short reliefshade);
extern void smallwindow(short diagnostics);
// extern void Close_Small_Window(short win_number); // used locally only -> static, AF 26.7.2021
extern void Handle_Small_Window(short win_number);
extern void Handle_InterWind0(void);
extern void Handle_InterWind2(void);
extern short OpenNewIAGridSize(void);

/* InteractiveUtils.c */
extern short SetIncrements(short selectitem);
extern void autocenter(void);
extern void modifyinteractive(short shiftx, short shifty, long button);
extern void modifycampt(short shiftx, short shifty, short button);
extern void modifyfocpt(short shiftx, short shifty, short button);
extern void Update_InterMap(short modval);
extern void azimuthcompute(short focusaz, double flat, double flon,
        double tlat, double tlon);
extern void reversecamcompute(void);
extern void reversefoccompute(void);
extern void setcompass(short focusaz);
extern void findfocprof(void);
extern void Set_CompassBds(void);
extern void Set_LandBds(void);
extern void Set_ProfileBds(void);
extern void Set_BoxBds(void);
extern void Set_GridBds(void);
// extern void DisableBoundsButtons(void); // used locally only -> static, AF 26.7.2021
extern void FixXYZ(char item);
extern void Init_IA_View(short view);
extern void Init_IA_Item(short MoItem, short view);
extern short CheckDEMStatus(void);

/* LineSupport.c */
extern void FadeLine(long el);
extern void EndDrawLine(struct Window *win, short zbufplot, short Color);
extern void EndDrawPoint(struct Window *win, short zbufplot, short Color);
extern short writelinefile(struct elmapheaderV101 *map, long mode);
extern short InitDigPerspective(void);
extern void QuitDigPerspective(void);
extern short PerspectivePoint(short addpoint, double lat, double lon,
	long elevation);
extern short VectorToPath(short item);
extern short PathToVector(short item);

/* LWSupport.c */
extern short ExportWave(struct LightWaveInfo *LWInfo, FILE *Supplied);
//extern short Set_LWM(struct LightWaveMotion *LWM, struct LightWaveInfo *LWInfo,
//	short Frame, double Scale); // used locally only -> static, AF 23.7.2021
extern short LWOB_Export(char *ObjectName, char *OutputName, struct coords *PP,
	long MaxVertices, long MaxPolys, short SaveObject,
	short Bathymetry, double LonRot);
extern void LWScene_Export(struct LightWaveInfo *LWInfo);

/* MakeFaces.c */
extern short faceone(struct elmapheaderV101 *map);
extern short facetwo(struct elmapheaderV101 *map);
// extern short writeface(struct elmapheaderV101 *map); // used locally only -> static, AF 20.7.2021
// extern short minmax(void); // used locally only -> static, AF 26.7.2021
// extern void facecompute(struct elmapheaderV101 *map, short low, short mid, short hi); // used locally only -> static, AF 19.7.2021
extern void FaceOne_Setup(struct elmapheaderV101 *map, struct faces *Face,
	long Lr, long Lc);
extern void FaceTwo_Setup(struct elmapheaderV101 *map, struct faces *Face,
	long Lr, long Lc);
extern void FractFaceOne_Setup(struct elmapheaderV101 *map,
	long Lr, long Lc);
extern void FractFaceTwo_Setup(struct elmapheaderV101 *map,
	long Lr, long Lc);
extern void FacePt_Switch(struct elmapheaderV101 *map, struct FaceData *Data,
	struct faces *Face);
// extern short FacePt_Order(struct FaceData *Data); // used locally only -> static, AF 26.7.2021
/*extern void Face_Compute(struct elmapheaderV101 *map, struct FaceData *Data,
	struct faces *Face, short low, short mid, short hi);*/ // used locally only -> static, AF 20.7.2021

/* Map.c */
/*extern short paramset(void);*/
extern void alignmap(struct Box *Bx);
extern void makemap(struct Window *win, long lowx, long lowy, long highx, long highy,
	unsigned long int FlagBits);
//extern void EnsureLoaded(void); // used locally only -> static, AF 23.7.2021
//extern USHORT MapRelel_Load(struct elmapheaderV101 *This, short MapNum); // used locally only -> static, AF 23.7.2021
//extern void MapRelel_Free(struct elmapheaderV101 *This, short MapNum); // used locally only -> static, AF 23.7.2021
//extern short CalcRefine(double LowLon, double HighLon, double LowLat, double HighLat,
//	short *LocalLowEl, short *LocalHighEl, float *ElevRng); // used locally only -> static, AF 23.7.2021
extern short shiftmap(int OnePoint, int XCen, int YCen);
/*extern void setlinewidth(void);*/
extern short maptotopo(long OBN);
extern short findmouse(short X, short Y, short IdentifyOnly);
extern short findmulti(void);
/*extern void findname(void);*/
extern void addpoints(long lowj,long insert);
//extern short interpolatepts(short j,long x2,long x1,long y2,long y1); interpolatepts
extern void Viewshed_Map(long OBN);
extern void Close_Viewshed_Window(void);
extern void Handle_Viewshed_Window(void);
//extern short SearchViewLine(short viewpt, short fpx, short fpy,
//	short elfp, short elvp); // used locally only -> static, AF 23.7.2021
// extern struct RastPort *TempRas_New(struct RastPort *Ancestor, int X, int Y); // used locally only -> static, AF 23.7.2021
// extern void TempRas_Del(struct RastPort *This, int X, int Y); // used locally only -> static, AF 23.7.2021
extern double *DitherTable_New(int size);
extern void DitherTable_Del(double *This, long size);
// extern short InputTabletPoints(long lowj, short TabletType); // used locally only -> static, AF 23.7.2021
//extern void RotatePt(double rotate, double *OriginX, double *OriginY,
//		double *PointX, double *PointY); // used locally only -> static, AF 23.7.2021
extern void FindCenter(double *Lat, double *Lon);
extern void ClearWindow(struct Window *Win, int col);
// extern short Set_Eco_Color(long Lra, long Lca, short i, short *RelEl); // used locally only -> static, AF 23.7.2021

/* MapExtra.c */
/*
extern double *loadstat(double *dataptr,long type);
extern void normalize(void);
extern void graphset(void);
extern void clearchecks(short j);
extern void graphdraw(double *dataptr);
extern void grapherase(void);
*/
extern void findarea(short OBN);
extern void FindDistance(void);
//extern double SolveArcAng(double CartDist, double SphereRad); // used locally only -> static, AF 23.7.2021
/*extern double SolveDistCart(double XJ, double YJ, double ZJ,
	double XK, double YK, double ZK);*/ // used locally only -> static, AF 23.7.2021
/*extern void MakeTempCart(double Lat, double Lon, double SphereRad,
	double *X, double *Y, double *Z);*/ // used locally only -> static, AF 23.7.2021
extern void setorigin(void);
extern void matchpoints(void);
extern short modpoints(short modify);
// extern void markpt(short edpt,short col); // used locally only -> static, AF 19.7.2021
//extern void unmarkpt(short edpt); // used locally only -> static, AF 23.7.2021
extern short CloneObject(void);
extern void makestream(short lowj);
extern void interpolatepath(void);
extern void SetSurface_Map(ULONG surface);
extern void FlatFixer(struct Box *Bx);
extern short BuildElevTable(double **ElTable, short Frames, float *MaxMin,
	short PathSource);
extern short BuildVelocityTable(short Foc, double **ElTable, short Frames, float *MaxMin);
extern short DistributeVelocity(struct VelocityDistr *Vel);
//extern void InitGauss(struct Gauss *Gauss); // used locally only -> static, AF 23.7.2021
//extern double DoGauss(struct Gauss *Gauss); // used locally only -> static, AF 23.7.2021
extern short Raster_Fract(float *Rast, long N1, long N2, long Seed, double delta,
	double H, short MaxStage);
extern void FractalMap_Draw(void);

/* MapLineObject.c */
extern short maplineobject(struct elmapheaderV101 *map, struct Window *win,
	short zbufplot, double *Lat, double *Lon);

/* MapSupport.c */
extern short saveobject(long OBN, char *fname, double *Lon, double *Lat,
	short *Elev);
extern short readmapprefs(void);
extern void savemapprefs(void);
extern short loadtopo(void);
extern short GetBounds(struct Box *Bx);
extern void valueset(void);
extern void valuesetalign(void);
extern short MousePtSet(struct Vertex *Vtx, struct Vertex *FixVtx, short Mode);
extern long LatLonElevScan(struct IntuiMessage *Event, char *MsgSupplement, short FindIt);
extern short XY_latlon(long i, long lowj, long highj);
extern double Y_Lat_Convert(long Y);
extern double X_Lon_Convert(long X);
extern void latlon_XY(long i);
extern long Lat_Y_Convert(double Lat);
extern long Lon_X_Convert(double Lon);
/*
extern void clearmesg(struct Window *w);
extern void makebox(struct RastPort *r, long x, long y, long w, long h);
extern void checkbox(struct RastPort *r, long x, long y, long w, long h);
*/
extern void outline(struct Window *win, long i, USHORT colr, struct clipbounds *cb);
extern short FreeAllVectors(void);
extern void freevecarray(short lowi);
extern short allocvecarray(long obn, long pts, short newobj);
extern void SetView_Map(short camera);
extern void ShowView_Map(struct clipbounds *cb);
extern short SetIAView_Map(struct IntuiMessage *Event);
extern void ShowCenters_Map(struct clipbounds *cb, short startX, short startY,
	short camera);
extern void ShowPaths_Map(struct clipbounds *cb);
extern void MakeColorMap(void);
extern void DrawHaze(struct Window *Win, int X, int Y, int Rad, struct clipbounds *cb);
extern void DrawHazeRPort(struct RastPort *Rast, int X, int Y, int Rad, struct clipbounds *cb);
// extern void DrawSun(struct Window *Win, int X, int Y, int Rad, struct clipbounds *cb); // used locally only -> static, AF 26.7.2021
extern struct datum *Datum_MVFind(struct datum *DT, struct datum *ET, long *PT, short WhichOne);
	
/* MapTopo.c */
extern void MapTopo(struct elmapheaderV101 *map, struct Window *win,
	short MapAsSFC, short MakeWater, short Visible, double *Elev);
extern short colormap(struct elmapheaderV101 *map, short notsnow,
	struct ColorComponents *CC, short *understory);
extern void MapCloud(struct Window *win, short *CloudVal, short *IllumVal,
	long Elev);
extern double CloudCover_Set(struct CloudData *CD, double Lat, double Lon);

/* MapTopoObject.c */
extern short maptopoobject(struct elmapheaderV101 *map, struct Window *win,
	short DEMnum, short NumDEMs, struct CloudData *CD);
/*extern void Face_Render(struct elmapheaderV101 *map, struct faces *Vertex,
	struct Window *win, struct CloudData *CD);*/ // used locally only -> static, AF 23.7.2021
/*extern void FractFace_Render(struct elmapheaderV101 *map,
	struct Window *win, short WhichFace, struct CloudData *CD);*/ // used locally only -> static, AF 23.7.2021
// extern void renderface(struct elmapheaderV101 *map, struct Window *win, struct CloudData *CD); // used locally only -> static, AF 23.7.2021
/* extern void rendercloud(struct Window *win, short *CloudVal,
	short *IllumVal, short Elev);*/ // used locally only -> static, AF 23.7.2021
//extern short setfaceone(struct elmapheaderV101 *map);  // local and therefore static AF, 16.July 2021
//extern short setfacetwo(struct elmapheaderV101 *map);  // local and therefore static AF, 16.July 2021
//extern short setface(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
//extern void CloudPointSort(short *CloudVal, short *IllumVal); // used locally only -> static, AF 23.7.2021
extern short CloudShadow_Init(struct elmapheaderV101 *map, struct CloudData *CD);
extern short MapCloudObject(struct elmapheaderV101 *map,
	struct CloudData *CD, struct Window *win);
/*extern short MapCloudLayer(struct elmapheaderV101 *map,
	struct CloudData *CD,  struct CloudLayer *CL, struct CloudLayer *More,
	double MinAmp, double MaxAmp, short j, struct Window *win*//*, UBYTE *CldMap,*//*);*/ // used locally only -> static, AF 23.7.2021
//extern short setcloudfaceone(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
//extern short setcloudfacetwo(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
// extern short VertexIndex_New(struct VertexIndex *Vtx, long MaxFract); // used locally only -> static, AF 23.7.2021
//extern void VertexIndex_Del(struct VertexIndex *Vtx); // used locally only -> static, AF 19.7.2021
extern short FractalDepth_Preset(void);
// extern short setquickfaceone(struct elmapheaderV101 *map, long Lr, long Lc); // used locally only -> static, AF 23.7.2021
// extern short setquickfacetwo(struct elmapheaderV101 *map, long Lr, long Lc); // used locally only -> static, AF 23.7.2021
// extern short setquickface(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
// extern short FractalLevel(short MaxSize); // used locally only -> static, AF 23.7.2021
/*extern short MakeFractalMap(struct elmapheaderV101 *map,
	BYTE *FractalMap, long FractalMapSize);*/ // used locally only -> static, AF 23.7.2021

/* MapUtil.c */
extern void getscrnpt(struct coords *TP, struct coords *SP);
extern void getscrncoords(float *scrnx, float *scrny, float *scrnq,
	struct coords *TP, struct coords *SP);
extern void getdblscrncoords(double *scrnx, double *scrny, double *scrnq,
	struct coords *TP, struct coords *SP);
extern void getviewscrncoords(float *scrnx, float *scrny, float *scrnq);
extern void convertpt(struct coords *PT);
extern void findposvector(struct coords *EP, struct coords *SP);
extern void VectorMagnitude(struct coords *TP);
extern void SurfaceNormal(struct coords *NP, struct coords *FP, struct coords *LP);
extern void UnitVector(struct coords *UV);
extern short SurfaceVisible(struct coords *SN, struct coords *VP, short Face);
extern double VectorAngle(struct coords *SN, struct coords *VP);
extern double SignedVectorAngle2D(struct coords *From, struct coords *To, short Axis);
extern void BuildRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx);
// extern void RotationMatrix3D(short m, double theta, Matx3x3 A); // used locally only -> static, AF 24.7.2021
// extern void ZeroMatrix3x3(Matx3x3 A); // used locally only -> static, AF 23.7.2021
extern void ZeroCoords(struct coords *A);
extern void NegateVector(struct coords *A);
//extern void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C); // used locally only -> static, AF 23.7.2021
extern void RotatePoint(struct coords *A, Matx3x3 M);
extern double findangle(double pta, double ptb);
extern double findangle2(double pta, double ptb);
extern double findangle3(double pta, double ptb);
extern void rotate(double *pta, double *ptb, double rotangle, double angle);
extern void rotate2(double *pta, double *ptb, double rotangle, double angle);
extern short autoactivate(void);
// extern void sortrenderlist(void); // used locally only -> static, AF 24.7.2021
extern void SortAltRenderList(void);
extern void setclipbounds(struct Window *win, struct clipbounds *cb);
extern void setlineseg(struct lineseg *ls, double firstx, double firsty,
	double lastx, double lasty);
extern void ClipDraw(struct Window *win, struct clipbounds *cb,
	struct lineseg *ls);
extern void ClipDrawRPort(struct RastPort *Rast, struct clipbounds *cb,
	struct lineseg *ls);
//extern void ClipAreaDraw(struct Window *win, struct poly4 *Poly,
//	struct clipbounds *cb); // used locally only -> static, AF 24.7.2021
//extern void ClipPoly4(struct Window *win, struct poly4 *Poly,
//	struct clipbounds *cb); // AF, not used 26.July 2021
extern short ClipPolySeg(struct clipbounds *cb, struct lineseg *ls);
extern void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone);
extern void UTM_LatLon(struct UTMLatLonCoords *Coords);
extern void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone);
extern void AlbLatLonCoords_Init(struct AlbLatLonCoords *Coords);
extern void Alb_LatLon(struct AlbLatLonCoords *Coords);
// extern double DegMinSecToDegrees2(double Val); // used locally only -> static, AF 24.7.2021
extern double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, short *Data, long Rows, long Cols);

/* Memory.c */
extern void *get_Memory(long zsize, long attributes);
extern void free_Memory(void *memblock, long zsize);

/* Params.c */
extern void MergeKeyFrames(union KeyFrame *MF, short MFKeys,
	union KeyFrame **OF, short *OFKeys, long *OFsize, short group);
extern short MakeKeyFrame(short frame, short group, short item);
// extern short AllocNewKeyArray(union KeyFrame **KF, long *KFsize); // used locally only -> static, AF 30.7.2021
extern short CheckKeyFrame(short frame, short group, short item);
extern short DeleteKeyFrame(short frame, short group, short Item,
	 short DeleteAll, short DeleteGp);
extern short SearchKeyFrame(short frame, short group, short item);
//extern void SetKeyFrame(short i, short frame, short group, short item); // used locally only -> static, AF 24.7.2021
extern short UnsetKeyFrame(short frame, short group, short item, short unset);
extern void UpdateKeyFrames(short frame, short group, short Item,
	 short UpdateAll, short UpdateGp);
//extern void UnsetKeyItem(union KeyFrame *Key); // used locally only -> static, AF 24.7.2021
extern short BuildKeyTable(void);
extern short BuildSingleKeyTable(short group, short item);
extern void GetKeyTableValues(short group, short item, short allvalues);
extern short CountKeyFrames(short group, short item);
//extern short GetNextKeyItem(short group, short curitem, short dir); // used locally only -> static, AF 24.7.2021
//extern void SetKeyTableEntry(union KeyFrame **Key, short group, short item); // used locally only -> static, AF 24.7.2021
extern void FreeKeyTable(void);
extern void FreeSingleKeyTable(short group, short frames);
// extern short SplineAllKeys(void); // used locally only -> static, AF 24.7.2021
extern short SplineSingleKey(short group, short newkey);
extern short GetActiveKey(struct KeyTable *KTbl, short frame);
extern void Play_Colors(void);

/* Plot.c */
extern void ScreenPixelPlot(struct Window *win, UBYTE **Bitmap,
	short x, short y, long zip);
extern void NoRGBScreenPixelPlot(struct Window *win,
	double FloatCol, short ColMax, short x, short y);
extern void NoDitherScreenPixelPlot(struct Window *win, short Col, short x, short y);

/* RequesterGUI.c */
extern short getdbasename(long mode);
extern short getparamfile(long mode);
extern short getstatfile(long mode);
extern short getfilename(long mode, char *requestname, char *pathname,
    	char *filename);
struct FileRequester *getmultifilename(char *requestname, char *pathname,
    char *filename, char *pattern);
void freemultifilename(struct FileRequester *This);
extern struct BusyWindow *BusyWin_New(char *Title, int Steps, int TimeEst, ULONG Section);
extern void BusyWin_Del(struct BusyWindow *This);
extern void BusyWin_Update(struct BusyWindow *This, int Step);
extern void Log_ElapsedTime(ULONG StartSecs, ULONG FirstSecs, long Frames);
extern APTR KeyButtonFunc(char ControlChar, char *Contents);
// extern void MUI_DoNotifyPressed(APTR Object, ULONG ID); // used locally only -> static, AF 25.7.2021
extern void MUI_DoNotiPresFal(APTR App, ...);
extern short getfilenameptrn(long mode, char *requestname, char *pathname,
    char *filename, char *ptrn);

/* ScratchPad.c and HyperKhorner4M-1.asm */
extern struct RastPort *ScratchRast_New(int Width, int Height, char Planes);
extern void ScratchRast_Del(struct RastPort *This);
struct vgl_pixmap;  // ALEXANDER forward declaration
extern void ScratchRast_CornerTurn(struct vgl_pixmap *This, struct RastPort *ScratchRast);

#ifdef __SASC
extern void ASM HK4M(REG(a0, void *Plane0), REG(a1, void *Plane1),
                     REG(a2, void *Plane2), REG(a3, void *Plane3),
                     REG(a4, void *PlaneM), REG(a5, unsigned long int PixCnt),
                     REG(a6, void *InPtr));
#else
// Do not use so many registers as parameters! Do not use A4 if you compile with baserel. Do not touch A5 in case of gcc!
// changed that function according to a suggestion by bebbo from here: https://github.com/bebbo/amiga-gcc/issues/296
extern void ASM HK4M(REG(a0, void *registers));
#endif
/* ScreenModeGUI.c */
extern struct WCSScreenMode *ModeList_New(void);
extern struct WCSScreenMode *ModeList_Choose(struct WCSScreenMode *This,
	struct WCSScreenData *ScrnData);
extern void ModeList_Del(struct WCSScreenMode *ModeList);

/* Support.c */
extern struct Window *make_window(short x, short y, short w, short h,
	char name[80], ULONG flags, ULONG iflags, UBYTE color0, UBYTE color1,
	struct Screen *screen);
extern void FetchEvent(struct Window *Win, struct IntuiMessage *Event);
extern struct Window *FetchMultiWindowEvent(struct IntuiMessage *Event, ...);
extern ULONG QuickFetchEvent(struct Window *Win, struct IntuiMessage *Event);
/* extern ULONG QuickCheckEvent(struct Window *Win); */ /* It's a macro now */
extern void closesharedwindow(struct Window *win, short shared);
//extern void stripintuimessages(struct MsgPort *mp, struct Window *win); // used locally only -> static, AF 19.7.2021
// extern short checkuserabort(void); // used locally only -> static, AF 26.7.2021
extern void SaveConfig(void);
extern void LoadConfig(void);
extern short SaveProject(short NewName,
	char *SaveName, struct WCSScreenData *ScrnData);
extern short LoadProject(char *LoadName, struct WCSScreenData *ScrnData,
	short ForceLoad);
extern short LoadDirList(void);
extern long PrintScreen(struct Screen *scr, UWORD srcx, UWORD srcy,
	UWORD srcw, UWORD srch, LONG destcols, UWORD iospecial);

/* Tree.c */
extern void DetailTree(struct Window *win, short eco, struct QCvalues *QC,
	short Computeht, struct ColorComponents *CC,
	struct ColorComponents *AM, double *Elev);
extern void ConvertTree(void);
extern short LoadForestModels(void);
extern short BitmapImage_Load(void);
extern void BitmapImage_Unload(void);
// extern struct BitmapImage *BitmapImage_New(void); // used locally only -> static, AF 26.7.2021
// extern void BitmapImage_Del(struct BitmapImage *BMI); // used locally only -> static, AF 23.7.2021
// extern void BitmapImage_DelSingle(struct BitmapImage *BMI); // used locally only -> static, AF 26.7.2021
//extern void ColorToGray(UBYTE **Bitmap, long MaxZip); // used locally only -> static, AF 23.7.2021
//extern void SetBitmapImageSpan(struct BitmapImage *BMI); // used locally only -> static, AF 19.7.2021
//extern void BitmapImage_Scale(struct BitmapImage *SBMI, struct BitmapImage *DBMI,
//	double Dx, double Dy); // used locally only -> static, AF 26.7.2021
//extern void Image_Paste(struct BitmapImage *SBMI, UBYTE **Bitmap,
//	double Dw, double Dh, double Dx, double Dy,
//	double Distance, struct ColorComponents *CC, 
//	struct ColorComponents *AM, double ElStart,
//	double ElIncr, struct Window *win, struct QCvalues *QC); // used locally only -> static, AF 23.7.2021

/* Wave.c */
extern void Wave_DelAll(struct Wave *WV);
extern void WaveData_Del(struct WaveData *WD);
extern void Wave_Init(struct WaveData *WD, short Frame);
extern short BuildWaveKeyTable(struct WaveData *WD);

/* WCS.c includes the 
main(int argc, char *argv[]) function */

#endif /* WCS_PROTO_H */


#ifndef WCS_GUI_PROTO_H
#define WCS_GUI_PROTO_H

/* CloudGUI.c */
// extern void Make_CL_Window(void); // used locally only -> static, AF 26.7.2021
extern void Close_CL_Window(void);
extern void Handle_CL_Window(ULONG WCS_ID);
extern short Cloud_Load(char *filename, struct CloudData **CDPtr);
extern short Cloud_Save(char *filename, struct CloudData *CD);
// extern void GUICloud_SetGads(struct CloudWindow *CL_Win,
//	struct CloudData *CD); // used locally only -> static, AF 26.7.2021
//extern void GUICloudKey_SetGads(struct CloudWindow *CL_Win,
//	struct CloudData *CD, short Frame); // used locally only -> static, AF 26.7.2021

/* DataOpsGUI.c */
extern void Make_DC_Window(void);
extern void Close_DC_Window(void);
extern void Handle_DC_Window(ULONG WCS_ID);
// extern void Get_DC_InputFile(void); // used locally only -> static, AF 25.7.2021
// extern double DegMinSecToDegrees(char *str); //AF, ???
// extern void Set_DC_Data(void); // used locally only -> static, AF 25.7.2021
extern void Make_DI_Window(void);
extern void Close_DI_Window(void);
extern void Handle_DI_Window(ULONG WCS_ID);
// extern void Set_DI_Data(void); // used locally only -> static, AF 25.7.2021

/* DiagnosticGUI.c */
extern void Open_Diagnostic_Window(struct Window *EcoWin, char *WinTitle);
extern void Close_Diagnostic_Window(void);
extern void Handle_Diagnostic_Window(ULONG WCS_ID);
extern void Set_Diagnostic_Point(LONG zip);

/* DispatchGUI.c */
extern short Database_LoadDisp(short lowi, short AskName, char *FileName, short MapUpdate);

/* DEMGUI.c */
extern void Make_MD_Window(void);
extern void Close_MD_Window(void);
extern void Handle_MD_Window(ULONG WCS_ID);
// extern void MD_Window_Init(void); // used locally only -> static, AF 20.7.2021
// extern void Make_GR_Window(void); // used locally only -> static, AF 20.7.2021
extern void Close_GR_Window(void);
extern void Handle_GR_Window(ULONG WCS_ID);
// extern void GR_Window_Init(void); // used locally only -> static, AF 20.7.2021
// extern short DEMBuild_Import(short FileType); // used locally only -> static, AF 20.7.2021
// extern short Contour_Import(void); // used locally only -> static, AF 20.7.2021
/*extern short Object_ImportXYZ(short Points, double *Lat, double *Lon,
 	short *ElevPtr, float El, struct datum *DT);*/ // used locally only -> static, AF 20.7.2021
// extern short XYZ_Import(short CoordSys); // used locally only -> static, AF 20.7.2021
// extern short DXF_Import(short CoordSys); // used locally only -> static, AF 20.7.2021
// extern short XYZ_Export(struct datum *DT); // used locally only -> static, AF 20.7.2021
//extern short Noise_Init(void); // used locally only -> static, AF 20.7.2021
// extern short Grid_Draw(struct NNGrid *NNG); // used locally only -> static, AF 20.7.2021
// extern short Grid_Save(struct NNGrid *NNG, short Units); // used locally only -> static, AF 19.7.2021

/* EdDBaseGUI.c */
extern void Make_DE_Window(void);
extern void Close_DE_Window(void);
extern void Handle_DE_Window(ULONG WCS_ID);
extern void Set_DE_Item(short item);
extern void Set_DE_List(short update);
// extern void Remove_DE_Item(short OBN, short Remove); // used locally only -> static, AF 26.7.2021
extern short DBList_New(short NewRecords);
extern void Map_DB_Object(long OBN, long Enabled, long Selected);
extern void Make_DL_Window(void);
extern void Close_DL_Window(struct DirList *DLDel);
extern void Handle_DL_Window(ULONG WCS_ID);
// extern void Set_DL_List(struct DirList *DLItem, short Update, short ActiveItem); // used locally only -> static, AF 26.7.2021
extern void Update_DL_Win(void);
extern short Add_DE_NewItem(void);

/* EdEcoGUI.c */
extern void Make_EE_Window(void);
// extern APTR Make_EE_Group(void); // used locally only -> static, AF 26.7.2021
extern void Close_EE_Window(short apply);
extern void Handle_EE_Window(ULONG WCS_ID);
extern void Set_EE_Item(short item);
// extern void UnSet_EE_Item(short item, double data); // used locally only -> static, AF 26.7.2021
extern void Set_EE_List(short update);
// extern void Set_EcoLimits(void); // used locally only -> static, AF 26.7.2021
extern short Load_EE_Map(long left, long top, long width, long height);
extern void Draw_EcoMap(short left, short top);
extern short SearchEcosystemColorMatch(short Color);
extern void AdjustEcosystemColors(short Operation, short First, short Last);

/* EditGUI.c */
extern void Make_EC_Window(void);
// extern APTR Make_EC_Group(void); // used locally only -> static, AF 24.7.2021
extern void Close_EC_Window(short apply);
extern void Handle_EC_Window(ULONG WCS_ID);
//extern long max3(long a, long b, long c); // used locally only -> static, AF 20.7.2021
//extern long min3(long a, long b, long c); // used locally only -> static, AF 24.7.2021
//extern long mid3(long a, long b, long c); // used locally only -> static, AF 20.7.2021
//extern void Compute_EcoPal(struct PaletteItem *Pal, short comp_mode); // used locally only -> static, AF 20.7.2021
extern void SetAllColorRequester(void);
//extern void SetColorRequester(short row); // used locally only -> static, AF 20.7.2021
extern void SetScreen_8(struct PaletteItem *Pal);
//extern void SetActiveColor(struct PaletteItem *Pal, short row);  // local static, AF, 16.July 2021
//extern void Adjust_EcoPal(short i); // used locally only -> static, AF 20.7.2021
extern void Set_EC_Item(short item);
//extern void UnSet_EC_Item(short item); // used locally only -> static, AF 20.7.2021
extern void Set_EC_List(short update);
//extern void AddColorEntry(void); // used locally only -> static, AF 20.7.2021

/* EdMoGUI.c */
extern void Set_EM_Item(short i);
extern void Update_EM_Item(void);
extern void Close_EM_Window(short apply);
extern void Handle_EM_Window(ULONG WCS_ID);
extern void Make_EM_Window(void);
extern void Set_Radial_Txt(short IAdraw);
// extern void Update_EMTL_Item(void); // used locally only -> static, AF 19.7.2021
extern void Set_EM_List(short update);
extern void MakeMotionKey(void);
extern void Close_EMIA_Window(short apply);
extern void Handle_EMIA_Window(ULONG WCS_ID);
extern void Make_EMIA_Window(void);
// extern void Close_EMPL_Window(void); // used locally only -> static, AF 26.7.2021
extern void Handle_EMPL_Window(ULONG WCS_ID);
extern void Make_EMPL_Window(void);

/* EdSetGUI.c */
extern void Close_ES_Window(short apply);
extern void Handle_ES_Window(ULONG WCS_ID);
extern void Make_ES_Window(void);
extern void Handle_SB_Buttons(ULONG WCS_ID);

/* EdSetExtrasGUI.c */
extern void Set_ES_Window(void);
// extern void Disable_ES_Window(void); // used locally only -> static, AF 26.7.2021
extern void Disable_ES_Gads(short status, ...);
extern void Set_FloatStringValue(short i, short factor, double *value);
extern void Set_IntStringValue(short i, short factor, long *value);
extern void SetMemoryReqTxt(void);
extern void SetRenderSpeedGauge(void);
extern void Set_PJ_Window(void);

/* EvenMoreGUI.c */
extern void Make_TS_Window(void);
extern void Close_TS_Window(short apply);
extern void Handle_TS_Window(ULONG WCS_ID);
// extern void Set_TS_Position(void); // used locally only -> static, AF 25.7.2021
// extern void Set_TS_Reverse(short reverse); // used locally only -> static, AF 25.7.2021
// extern void Make_PN_Window(void); // used locally only -> static, AF 25.7.2021
extern void Close_PN_Window(short Apply);
extern void Handle_PN_Window(ULONG WCS_ID);
// extern short CreateNewProject(char *NewProjName, char *CloneProjName); // used locally only -> static, AF 25.7.2021

/* FoliageGUI.c */
// extern void Make_FE_Window(void); // used locally only -> static, AF 19.7.2021
extern void Close_FE_Window(short apply);
extern void Handle_FE_Window(ULONG WCS_ID);
// extern short GUIEcotype_Load(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliageGroup_Add(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliageGroup_New(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliageGroup_Save(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliage_Add(void); // used locally only -> static, AF 19.7.2021
// extern short GUIImagePath(struct Foliage *Fol); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliage_Remove(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliageGroup_Remove(void); // used locally only -> static, AF 19.7.2021
// extern short GUIFoliage_View(struct Foliage *Fol, UBYTE **Bitmap); // used locally only -> static, AF 19.7.2021
// extern void EcotypeDefaultColors(struct Ecotype *Eco, short Color); // used locally only -> static, AF 19.7.2021
// extern void FoliageGroupDefaultColors(struct FoliageGroup *FolGp, short Color); // used locally only -> static, AF 19.7.2021
// extern void FoliageDefaultColors(struct Foliage *Fol, short Color); // used locally only -> static, AF 19.7.2021
//extern short SearchColorListMatch(char *Name); // used locally only -> static, AF 19.7.2021
extern short SearchEcotypeColorMatch(struct Ecotype *Eco, short Color);
extern void AdjustFoliageColors(struct Ecotype *Eco, short Operation,
	short First, short Last);
extern void GetFoliageLimits(struct Ecotype *Eco, short *Limit);
// extern short GUIEcotype_Save(void); // used locally only -> static, AF 19.7.2021
extern void GUIList_Clear(char **List, long ListSize, APTR ListView);
//extern void GUIFoliageGroup_SetGads(struct FoliageGroup *This); // used locally only -> static, AF 19.7.2021
//extern void GUIFoliage_SetGads(struct Foliage *This); // used locally only -> static, AF 19.7.2021
// extern void BuildFoliageGroupList(struct Ecotype *Eco); // used locally only -> static, AF 19.7.2021
// extern void BuildFoliageList(struct FoliageGroup *FolGp); // used locally only -> static, AF 19.7.2021
// extern void DisableColorChecks(struct FoliageGroup *FolGp, struct Foliage *Fol); // used locally only -> static, AF 19.7.2021

/* GUI.c */
extern void Make_EP_Window(short hor_win);
// extern void Close_EP_Window(void); // used locally only -> static, AF 25.7.2021
// extern void Handle_EP_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
extern void Make_DB_Window(short hor_win);
// extern void Close_DB_Window(void); // used locally only -> static, AF 25.7.2021
// extern void Handle_DB_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
extern void Make_DO_Window(short hor_win);
// extern void Close_DO_Window(void); // used locally only -> static, AF 25.7.2021
// extern void Handle_DO_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
extern void Make_RN_Window(short hor_win);
extern void Close_RN_Window(void);
// extern void Handle_RN_Window(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
extern struct WCSApp *WCS_App_New(void);
extern struct WCSApp *WCS_App_Startup(struct WCSApp *This);
extern short WCS_App_EventLoop(struct WCSApp *This);
extern void WCS_App_Del(struct WCSApp *This);
extern ULONG GetInput_ID(void);
extern ULONG CheckInput_ID(void);
extern USHORT User_Message(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey);
extern USHORT User_Message_Def(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey, int Default);
//extern void NoMod_Message(STRPTR mod); // used locally only -> static, AF 25.7.2021
extern USHORT CloseWindow_Query(STRPTR win);
extern USHORT NoGUI_Message(STRPTR mod);
extern void NoLoad_Message(CONST_STRPTR mod, CONST_STRPTR loaditem);
//extern USHORT FileExists_Message(STRPTR existsfile); // AF, not used 26.July 2021
extern short GetInputString(char *message, char *reject,
	 char *string);
// extern void Status_Log(STRPTR logtext, int Severity); // used locally only -> static, AF 25.7.2021
// extern void Make_Log_Window(int Severity); // used locally only -> static, AF 26.7.2021
// extern void Close_Log_Window(int Sticky); // used locally only -> static, AF 25.7.2021
// extern short Handle_APP_Windows(ULONG WCS_ID); // used locally only -> static, AF 25.7.2021
extern void Log(USHORT StdMesgNum, CONST_STRPTR LogTag);
extern void DisableKeyButtons(short group);
extern void Set_Param_Menu(short Group);
extern void settextint(APTR Obj, long Val);
extern void setfloat(APTR Obj, double Val);
extern void nnsetfloat(APTR Obj, double Val);

/* MapGUI.c */
//extern int MapGUI_New(struct MapData *MP); // used locally only -> static, AF 20.7.2021
extern void MapGUI_Update(struct MapData *MP);
extern void MapGUI_Message(short line, char *Message);
//extern void MapGUI_Del(struct MapData *MP); // used locally only -> static, AF 20.7.2021
extern void TrimZeros(char *String);
extern void UnderConst_New(void);
extern void UnderConst_Del(void);
extern short map(void);
extern void Handle_Map_Window(ULONG WCS_ID);
extern void Close_Map_Window(short ExitWCS);
extern void MapIDCMP_Restore(struct Window *win);
//extern short Make_Map_Menu(void); // used locally only -> static, AF 20.7.2021
extern short Make_MA_Window(struct MapData *MP);
// extern void Close_MA_Window(struct MapData *MP); // used locally only -> static, AF 20.7.2021
extern void Make_EL_Window(void);
extern void Close_EL_Window(void);
extern void Handle_EL_Window(ULONG WCS_ID);
extern void Update_EcoLegend(short Item, short Item2, short Operation);
extern void Contour_Export(void);
extern short Object_WriteXYZ(FILE *fObj, short Points, double *Lat, double *Lon,
	short *ElevPtr, float El, struct MaxMin3 *MM3);
extern struct MaxMin3 *MaxMin3_New(void);
extern void MaxMin3_Del(struct MaxMin3 *MM3);

/* MoreGUI.c */
extern void Make_DM_Window(void);
extern void Close_DM_Window(void);
extern void Handle_DM_Window(ULONG WCS_ID);
extern short Set_DM_Data(struct DEMExtractData *DEMExtract);
extern void Set_DM_HdrData(struct USGS_DEMHeader *Hdr);
extern void Set_DM_ProfData(struct USGS_DEMProfileHeader *ProfHdr);
extern void Make_PJ_Window(void);
extern void Close_PJ_Window(short Apply);
extern void Handle_PJ_Window(ULONG WCS_ID);
extern void Make_SC_Window(void);
extern void Close_SC_Window(void);
extern void Handle_SC_Window(ULONG WCS_ID);
// extern void ApplyImageScale(void); // used locally only -> static, AF 26.7.2021
extern void Make_PR_Window(void);
extern void Close_PR_Window(void);
extern void Handle_PR_Window(ULONG WCS_ID);

/* ParamsGUI.c */
// extern void Make_PS_Window(ULONG WCS_ID); // used locally only -> static, AF 24.7.2021
extern void Close_PS_Window(short apply);
extern void Handle_PS_Window(ULONG WCS_ID);
extern short Set_PS_List(char **List, short *ListID, short group,
	short ReqKeys, char *TruncateText);
// extern void Set_PS_Info(void); // used locally only -> static, AF 24.7.2021
extern void Make_LW_Window(void);
extern void Close_LW_Window(void);
extern void Handle_LW_Window(ULONG WCS_ID);
// extern void Set_LW_Info(void); // used locally only -> static, AF 24.7.2021
extern void Make_FM_Window(void);
extern void Close_FM_Window(void);
extern void Handle_FM_Window(ULONG WCS_ID);
// extern void Set_FM_List(short Update, short ActiveItem); // used locally only -> static, AF 24.7.2021
// extern short Add_FM_Item(void); // used locally only -> static, AF 19.7.2021
// extern void Remove_FM_Item(long item); // used locally only -> static, AF 24.7.2021
// extern void Unset_FM_Item(long item); // used locally only -> static, AF 24.7.2021
// extern short Load_FM_Win(void); // used locally only -> static, AF 24.7.2021
// extern short Save_FM_Win(void); // used locally only -> static, AF 24.7.2021
extern void Make_AN_Window(void);
extern void Close_AN_Window(void);
extern void Handle_AN_Window(ULONG WCS_ID);
extern void Init_Anim(short SaveAnim);

/* TimeLinesGUI.c */
extern void Close_EMTL_Window(short apply);
extern void Handle_EMTL_Window(ULONG WCS_ID);
extern void Make_EMTL_Window(void);
extern short Set_EMTL_Item(short item);
extern void Set_EMTL_Data(void);
extern void Close_ECTL_Window(short apply);
extern void Handle_ECTL_Window(ULONG WCS_ID);
extern void Make_ECTL_Window(void);
extern short Set_ECTL_Item(short item);
extern void Set_ECTL_Data(short subitem);
extern void Close_EETL_Window(short apply);
extern void Handle_EETL_Window(ULONG WCS_ID);
extern void Make_EETL_Window(void);
extern short Set_EETL_Item(short item);
extern void Set_EETL_Data(short subitem);
extern short GetInput_Pt(struct IClass *cl, Object *obj);
extern void ResetTimeLines(short NullGroup);

/* GenericTLGUI.c */
// extern short Set_TL_Item(struct TimeLineWindow *TL_Win, short item); // used locally only -> static, AF 25.7.2021
// extern void Set_TL_Data(struct TimeLineWindow *TL_Win, short subitem); // used locally only -> static, AF 25.7.2021
extern void TL_Redraw(struct TimeLineWindow *TL_Win);
extern void TL_Recompute(struct TimeLineWindow *TL_Win);
extern void Update_TL_Win(struct TimeLineWindow *TL_Win, short subitem);
extern void Make_TL_Window(char *NameStr, char **Titles,
	struct TimeLineWindow **TLPtr, APTR *ValStringGads,
	struct WindowKeyStuff *WKS, struct GUIKeyStuff *GKS,
	union KeyFrame **KFPtr, long *KFSizePtr, short *KeyFramesPtr,
	double *DblValue, float *FltValue, short *ShortValue);
// extern APTR Make_TLRegisterGroup(struct TimeLineWindow *TL_Win, short NumValues, 
//	char **Titles); // used locally only -> static, AF 25.7.2021
extern void Close_TL_Window(struct TimeLineWindow **TLWinPtr, short apply);
extern void Handle_TL_Window(ULONG WCS_ID);

/* WaveGUI.c */
extern void Make_WV_Window(short WindowNum, char *NameStr);
extern void Close_WV_Window(struct WaveWindow **WVWinPtr);
extern void Handle_WV_Window(ULONG WCS_ID);
extern short Wave_Load(char *filename, struct WaveData **WDPtr);
extern void GUIDisableKeyButtons(struct GUIKeyStuff *GKS, struct TimeLineWindow *TL,
	struct WindowKeyStuff *WKS);

#endif /* WCS_GUI_PROTO_H */


/* nngridr.c */
extern short            nngridr(void);
// extern struct 	NNGrid *NNGrid_New(void); // used locally only -> static, AF 26.7.2021
// extern void 		NNGrid_Init(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
// extern void 		NNGrid_Del(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
// extern short 		NNGridr_OptInit(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
/* extern void            GetOptions(struct NNGrid *NNG);*/

/* nncrunch.c */
extern short 		NNGrid_DataInit(struct NNGrid *NNG);
/*extern short            ReadData(struct NNGrid *NNG);*/
extern short            ChoroPleth(struct NNGrid *NNG);
// extern short            FindNeigh(struct NNGrid *NNG, int ipt); // used locally only -> static, AF 26.7.2021
// extern short            TriCentr(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
// extern short            TriNeigh(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
extern short            Gradient(struct NNGrid *NNG);
extern short            MakeGrid(struct NNGrid *NNG, short Units);
// extern short            Find_Prop(struct NNGrid *NNG, double wxd, double wyd); // used locally only -> static, AF 26.7.2021
extern double          Surface(struct NNGrid *NNG);
// extern double          Meld(struct NNGrid *NNG, double asurf, double wxd, double wyd); // used locally only -> static, AF 26.7.2021
// extern short           TooSteep(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
// extern short           TooShallow(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
// extern short           TooNarrow(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
extern struct datum    *Datum_New(void);
// extern struct simp     *Simp_New(void); // used locally only -> static, AF 26.7.2021
extern void	       Datum_Del(struct datum *CD);
extern void	       Simp_Del(struct simp *CS);
// extern struct temp     *Temp_New(void); // used locally only -> static, AF 26.7.2021
extern void	       Temp_Del(struct temp *CT);
// extern struct neig     *Neig_New(void); // used locally only -> static, AF 26.7.2021
extern void	       Neig_Del(struct neig *CN);
// extern int             *IntVect(int ncols); // used locally only -> static, AF 26.7.2021
extern void            FreeVecti(int *vectptr, int ncols);
// extern double          **DoubleMatrix(int nrows, int ncols); // used locally only -> static, AF 26.7.2021
extern void            FreeMatrixd(double **matptr, int nrows, int ncols);

