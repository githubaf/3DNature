// EffectObject3D.cpp
// For managing Object 3D Effects
// Built from EffectFoliage on 03/4/98 by Gary R. Huber
// Copyright 1998 Questar Productions

#include "stdafx.h"

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
#endif // FRANK or GARY

//#define WCS_SUPPORT_3DS
#define WCS_SUPPORT_OBJ

#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "Object3DEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "Points.h"
#include "MathSupport.h"
#include "Requester.h"
#include "Database.h"
#include "Render.h"
#include "Interactive.h"
#include "RenderControlGUI.h"
#include "ViewGUI.h"
#ifdef WCS_SUPPORT_3DS
#include "3dsftk.h"
#endif // WCS_SUPPORT_3DS
#include "Security.h"
#include "DXF.h"
#include "WCSVersion.h"
#include "DEM.h"
#include "Lists.h"

#ifndef min
#define   min(a,b)    ((a) <= (b) ? (a) : (b))
#endif

#define WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY	1023
#define WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY_TEXT	"1023"
#define WCS_EFFECTOBJECT3D_OBJ_MAXPOINTSPERPOLY	1023
#define WCS_EFFECTOBJECT3D_OBJ_MAXPOINTSPERPOLY_TEXT	"1023"

inline unsigned long CalcHash3D(double IndexFloatA, double IndexFloatB, double IndexFloatC);

class MaterialTextureInfo
	{
	public:
		char Enabled;
		Point3f Scale, Rot, Trans;
		float Opacity, BumpAmp;
		char MapName[512], ObjPath[512];
		char UVMapName[64], RGBMapName[64];
		char TileX, TileY, FlipX, FlipY, Negative, Decal, ClipMap;
		long TexType; // -1 indicates none, otherwise like WCS_TEXTURE_TYPE_PLANARIMAGE, etc...
		short AxisType;
		char UseColors; // use the fields below
		unsigned char CoordSpace;
		Point3f ColorA, ColorB;

		MaterialTextureInfo();
		void SetX(float NewScale) {Scale[0] = NewScale;};
		void SetY(float NewScale) {Scale[1] = NewScale;};
		void SetZ(float NewScale) {Scale[2] = NewScale;};
		float GetX(void) {return(Scale[0]);};
		float GetY(void) {return(Scale[1]);};
		float GetZ(void) {return(Scale[2]);};
		char *GetName(void) {return(MapName);};
		void SetName(char *NewName) {strncpy(MapName, NewName, 511); MapName[511] = NULL;};
		void SetNameTrimPath(char *NewName);

		char *GetPath(void) {return(ObjPath);};
		void SetPath(char *NewName) {strncpy(ObjPath, NewName, 511); ObjPath[511] = NULL;};
		char *GetUVName(void) {return(UVMapName);};
		void SetUVName(char *NewName) {strncpy(UVMapName, NewName, 63); UVMapName[63] = NULL;};
		char *GetRGBName(void) {return(RGBMapName);};
		void SetRGBName(char *NewName) {strncpy(RGBMapName, NewName, 63); RGBMapName[63] = NULL;};

		void SetType(long NewTexType) {TexType = NewTexType;};
		long GetType(void) {return(TexType);};
		void SetSpace(unsigned char NewSpace) {CoordSpace = NewSpace;};
		unsigned char GetSpace(void) {return(CoordSpace);};

	}; // class MaterialTextureInfo

class LWClipEntry
	{
	public:
		unsigned long ClipID;
		char ClipPath[1024];
		LWClipEntry *Next;

		LWClipEntry() {ClipPath[0] = NULL; ClipID = 0; Next = NULL;};
		void SetName(char *ClipNameAndPath);	// {strncpy(ClipPath, ClipNameAndPath, 1023); ClipNameAndPath[1023] = NULL;};
		LWClipEntry *FindInChain(unsigned long FindID); // call on head of list, returns object or NULL for failure
	}; // LWClipEntry

class LWVertMapEntry
	{
	public:
		LWVertMapEntry *Next;
		int NumVMAPEntries;
		int NumVMADEntries;
		char MapName[255];

		LWVertMapEntry() {Next = NULL; NumVMAPEntries = NumVMADEntries = 0;};
		~LWVertMapEntry();
		void SetName(char *NewMapName) {strncpy(MapName, NewMapName, 254); MapName[254] = NULL;};
		int CompareName(char *CompMapName) {return(strcmp(MapName, CompMapName));};

	}; // LWVertMapEntry


class LWTagEntry
	{
	public:
		unsigned char EntryName[WCS_EFFECT_MAXNAMELENGTH];
		long SurfID;
		ObjectMaterialEntry *OME;

		LWTagEntry() {InitClear();};
		void InitClear(void) {EntryName[0] = NULL; SurfID = -1; OME = NULL;};
	}; // LWTagEntry

class LWTagTable
	{
	private:
		LWTagEntry *TagTable;
		unsigned long TTEntries;
	public:
		LWTagTable() {TagTable = NULL; TTEntries = 0;};
		~LWTagTable();

		unsigned long SetEntries(unsigned long NewEntries);
		void DiscardEntries(void);
		void SetEntryName(unsigned long EntryNum, unsigned char *EntryName);
		void SetOME(unsigned long EntryNum, ObjectMaterialEntry *NewOME) {TagTable[EntryNum].OME = NewOME;};
		//void SetSurfID(unsigned long EntryNum, unsigned long SurfID) {TagTable[EntryNum].SurfID = SurfID;};
		void SetSurfID(unsigned long EntryNum, long SurfID);
		ObjectMaterialEntry *GetOME(unsigned long EntryNum) {if (EntryNum < TTEntries) return(TagTable[EntryNum].OME); else return(NULL);};
		//long GetSurfID(unsigned long EntryNum) {if (EntryNum < TTEntries) return(TagTable[EntryNum].SurfID); else return(-1);};
		long GetSurfID(unsigned long EntryNum);
		unsigned char *GetEntryName(unsigned long EntryNum) {if (EntryNum < TTEntries) return(&TagTable[EntryNum].EntryName[0]); else return(NULL);};
		long FindEntryByName(unsigned char *TagName); // returns -1 for failure
	}; // LWTagTable

class DynFileBlock
	{
	private:
		unsigned char *DynBlock;
		size_t DynBlockSize;
	public:
		DynFileBlock() {DynBlock = NULL; DynBlockSize = 0;};
		~DynFileBlock();
		unsigned char *ReadDynBlock(FILE *BlockFile, size_t BlockSize);
		void DiscardDynBlock(void);
		unsigned char *GetBlock(void) {return(DynBlock);};
		size_t GetBlockSize(void) {return(DynBlockSize);};
		unsigned char *GetBlockEnd(void) {return(&DynBlock[DynBlockSize]);}; // pointer one byte beyond last valid block byte
	}; // DynFileBlock

/*===========================================================================*/

int LWO2ChannelLookup(char *InQuad)
{
int Result = -1;
unsigned long CheckQuad;

memcpy(&CheckQuad, InQuad, 4);
switch(CheckQuad)
	{
	case 'COLR': return(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR);
	case 'DIFF': return(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY);
	case 'LUMI': return(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY);
	case 'SPEC': return(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY);
	case 'GLOS': return(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP);
	case 'REFL': return(WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY);
	case 'TRAN': return(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY);
	case 'RIND': return(-1); // index of refraction -- no match
	case 'TRNL': return(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY);
	case 'BUMP': return(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP);
	} // CheckQuad

return(Result);

} // LWO2ChannelLookup

/*===========================================================================*/

void MaterialTextureInfo::SetNameTrimPath(char *NewName)
{
char *StartofName;
int ScanLoop;

StartofName = NewName;
for (ScanLoop = 0; NewName[ScanLoop]; ScanLoop++)
	{
	if ((NewName[ScanLoop] == ':') || (NewName[ScanLoop] == '\\') || (NewName[ScanLoop] == '/'))
		{
		StartofName = &NewName[ScanLoop + 1];
		} // if
	} // if

strncpy(MapName, StartofName, 511); MapName[511] = NULL;

} // MaterialTextureInfo::SetNameTrimPath

/*===========================================================================*/
/*===========================================================================*/

// call on head of list, returns object or NULL for failure
LWClipEntry *LWClipEntry::FindInChain(unsigned long FindID)
{
LWClipEntry *Search;

for (Search = this; Search; Search = Search->Next)
	{
	if (Search->ClipID == FindID)
		{
		return(Search);
		} // if
	} // for

return(NULL);

} // LWClipEntry::FindInChain

/*===========================================================================*/

void LWClipEntry::SetName(char *ClipNameAndPath)
{

if (strlen(ClipNameAndPath) < 1024)
	{
	strcpy(ClipPath, ClipNameAndPath);
	} // if
else
	{
	strncpy(ClipPath, ClipNameAndPath, 1023);
	ClipPath[1023] = NULL;
	} // else

} // LWClipEntry::SetName

/*===========================================================================*/
/*===========================================================================*/

LWTagTable::~LWTagTable()
{

DiscardEntries();

} // LWTagTable::~LWTagTable

/*===========================================================================*/

unsigned long LWTagTable::SetEntries(unsigned long NewEntries)
{
DiscardEntries();

if (NewEntries > 0)
	{
	if (TagTable = (LWTagEntry *)AppMem_Alloc(NewEntries * sizeof(LWTagEntry), APPMEM_CLEAR))
		{
		TTEntries = NewEntries;
		// initialize them, as AppMem_Alloc doesn't call constructors for us
		for (unsigned long Constructor = 0; Constructor < NewEntries; Constructor++)
			{
			TagTable[Constructor].InitClear();
			} // for
		} // if
	} // if

return(0);

} // LWTagTable::SetEntries

/*===========================================================================*/

void LWTagTable::DiscardEntries(void)
{
if (TagTable)
	{
	AppMem_Free(TagTable, TTEntries * sizeof(LWTagEntry));
	TagTable = NULL;
	TTEntries = 0;
	} // if
} // LWTagTable::DiscardEntries

/*===========================================================================*/

void LWTagTable::SetEntryName(unsigned long EntryNum, unsigned char *EntryName)
{

if (EntryNum < TTEntries)
	{
	strncpy((char *)TagTable[EntryNum].EntryName, (char *)EntryName, 48);
	TagTable[EntryNum].EntryName[47] = NULL;
	} // if

} // LWTagTable::SetEntryName

/*===========================================================================*/

long LWTagTable::FindEntryByName(unsigned char *TagName)
{
unsigned long Search;

for (Search = 0; Search < TTEntries; Search++)
	{
	if (TagName && TagTable[Search].EntryName) // prevent blowup on NULL entries
		{
		if (!strcmp((char *)TagName, (char *)TagTable[Search].EntryName))
			{
			return(Search);
			} // if
		} // if
	} // for

return(-1);

} // LWTagTable::FindEntryByName

/*===========================================================================*/

void LWTagTable::SetSurfID(unsigned long EntryNum, long SurfID)
{

TagTable[EntryNum].SurfID = SurfID;

} // LWTagTable::SetSurfID

/*===========================================================================*/

long LWTagTable::GetSurfID(unsigned long EntryNum)
{

if (EntryNum < TTEntries)
	return(TagTable[EntryNum].SurfID);
else
	return(-1);

} // LWTagTable::SetSurfID

/*===========================================================================*/
/*===========================================================================*/

unsigned char *DynFileBlock::ReadDynBlock(FILE *BlockFile, size_t BlockSize)
{

DiscardDynBlock();
if (DynBlock = new unsigned char [BlockSize])
	{
	if (fread(DynBlock, BlockSize, 1, BlockFile) != 1)
		{ // read failed
		DiscardDynBlock();
		} // if
	else
		{ // success
		DynBlockSize = BlockSize;
		} // else
	} // if

return(DynBlock);

} // DynFileBlock::ReadDynBlock

/*===========================================================================*/

void DynFileBlock::DiscardDynBlock(void)
{

delete [] DynBlock;
DynBlock = NULL;

} // DynFileBlock::DiscardDynBlock

/*===========================================================================*/

DynFileBlock::~DynFileBlock(void)
{

DiscardDynBlock();

} // DynFileBlock::~DynFileBlock

/*===========================================================================*/
/*===========================================================================*/

MaterialAttributes::MaterialAttributes()
{

Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
Red = Green = Blue = 255;
FPRed = FPGreen = FPBlue = 0.0f;
UseFPGuns = 0;
DoubleSided = 1;
SmoothAngle = 45.0;
Luminosity = 0.0;
Diffuse = 1.0;
Transparency = 0.0;
Specularity = 0.0;
SpecExponent = 16.0;
Reflectivity = 0.0;
Translucency = 0.0;
BumpIntensity = 1.0;
DiffuseSharpness = 0.0;

for (unsigned int i = 0; i < WCS_EFFECTS_MATERIAL_NUMTEXTURES; i++)
	{
	MTI[i] = NULL;
	} // for

} // MaterialAttributes::MaterialAttributes

/*===========================================================================*/

MaterialAttributes::~MaterialAttributes()
{

for (unsigned int i = 0; i < WCS_EFFECTS_MATERIAL_NUMTEXTURES; i++)
	{
	if (MTI[i])
		{
		delete MTI[i];
		MTI[i] = NULL;
		} // if
	} // for

} // MaterialAttributes::~MaterialAttributes

/*===========================================================================*/
/*===========================================================================*/

MaterialTextureInfo::MaterialTextureInfo()
{

Enabled = 1;
Scale[2] = Scale[1] = Scale[0] = 1.0f;
Rot[2] = Rot[1] = Rot[0] = Trans[2] = Trans[1] = Trans[0] = 0.0f;
ObjPath[0] = UVMapName[0] = RGBMapName[0] = MapName[0] = NULL;
TexType = -1;
AxisType = 2;	// translates to Z axis in WCS
UseColors = 0;
TileX = TileY = 1;
FlipX = FlipY = Negative = Decal = ClipMap = 0;
Opacity = 100.0f;
BumpAmp = 1.0f;
CoordSpace = WCS_TEXTURE_COORDSPACE_NONE;
} // MaterialTextureInfo::MaterialTextureInfo

/*===========================================================================*/

MaterialTextureInfo *MaterialAttributes::GetCreateMaterialTextureInfo(int Attribute)
{

if (MTI[Attribute])
	{
	delete MTI[Attribute];
	MTI[Attribute] = NULL;
	} // if

MTI[Attribute] = new MaterialTextureInfo;

return(MTI[Attribute]);

} // MaterialAttributes::GetCreateMaterialTextureInfo

/*===========================================================================*/

ObjectMaterialEntry::ObjectMaterialEntry(void)
{

Name[0] = 0;
Mat = NULL;

} // ObjectMaterialEntry::ObjectMaterialEntry

/*===========================================================================*/

// MaterialAttributes *Mat can be NULL, which creates a default material
MaterialEffect *EffectsLib::SetMakeMaterial(ObjectMaterialEntry *Me, MaterialAttributes *Mat)
{
MaterialEffect *NameTest;
Raster *PlanarRaster;
NotifyTag Changes[2];

// first check and see if a material with the correct name already exists
if (Me && Me->Name[0])
	{
	NameTest = Material;
	while (NameTest)
		{
		if (! strcmp(NameTest->Name, Me->Name))
			{
			return (Me->Mat = NameTest);
			} // if
		NameTest = (MaterialEffect *)NameTest->Next;
		} // while
	} // if

// make a new material and give it the correct name
if (NameTest = new MaterialEffect(NULL, this, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
	{
	strncpy(NameTest->Name, Me->Name, WCS_EFFECT_MAXNAMELENGTH);
	NameTest->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	if (Mat)
		{
		MaterialTextureInfo *MTI;
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(Mat->Luminosity);
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetValue(Mat->Transparency);
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetValue(Mat->Specularity);
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].SetValue(Mat->SpecExponent);
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].SetValue(Mat->Translucency);
		NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].SetValue(Mat->BumpIntensity);
		// WCS doesn't support this param right now, but I've left this here in case we do later.
		//NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSESHARPNESS].SetValue(Mat->DiffuseSharpness);
		NameTest->Shading = Mat->Shading;
		NameTest->DoubleSided = Mat->DoubleSided;
		NameTest->SmoothAngle = Mat->SmoothAngle;
		NameTest->CosSmoothAngle = cos(NameTest->SmoothAngle * PiOver180);
		if (Mat->UseFPGuns)
			{
			NameTest->DiffuseColor.SetValue3((double)Mat->FPRed, (double)Mat->FPGreen, (double)Mat->FPBlue);
			} // if
		else
			{
			NameTest->DiffuseColor.SetValue3(Mat->Red / 255.0, Mat->Green / 255.0, Mat->Blue / 255.0);
			} // else
		NameTest->DiffuseColor.Intensity.SetValue(Mat->Diffuse);

		if (MTI = Mat->MTI[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR])
			{ // process extended texture info
			if (PlanarRaster = SetMakeTexture(MTI, NameTest, WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR, false, false))
				{
				if (! Mat->MTI[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY])
					{
					// only do this if there is no specific transparency texture
					if (PlanarRaster->AlphaAvailable)
						{
						PlanarRaster->SetAlphaEnabled(true);
						NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetValue(1.0);
						// make a transparency texture, negate image and alpha only
						SetMakeTexture(MTI, NameTest, WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY, true, true);
						} // if
					} // if
				} // if
			} // if
		if (MTI = Mat->MTI[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY])
			{ // process extended texture info, negate image
			SetMakeTexture(MTI, NameTest, WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY, true, false);
			NameTest->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetValue(1.0);
			} // if
		} // if
	Changes[0] = MAKE_ID(NameTest->GetNotifyClass(), NameTest->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NameTest->GetRAHostRoot());
	return (Me->Mat = NameTest);
	} // if

return (NULL);

} // EffectsLib::SetMakeMaterial

/*===========================================================================*/

Raster *EffectsLib::SetMakeTexture(MaterialTextureInfo *MTI, MaterialEffect *NameTest, int TexChannel, 
	bool NegateImage, bool AlphaOnly)
{
Raster *PlanarRaster = NULL;

if ((MTI->GetType() == WCS_TEXTURE_TYPE_PLANARIMAGE) || (MTI->GetType() == WCS_TEXTURE_TYPE_UVW))
	{
	if (MTI->GetType() == WCS_TEXTURE_TYPE_PLANARIMAGE)
		{
		if (NameTest->NewRootTexture(TexChannel))
			{
			// ...BumpAmp
			NameTest->Enabled = MTI->Enabled;
			PlanarImageTexture *PlanarImgTex;
			if (PlanarImgTex = (PlanarImageTexture *)NameTest->TexRoot[TexChannel]->AddNewTexture(NULL, WCS_TEXTURE_TYPE_PLANARIMAGE))
				{
				PlanarImgTex->CoordSpace = MTI->CoordSpace;
				PlanarImgTex->TileWidth = MTI->TileX;
				PlanarImgTex->TileHeight = MTI->TileY;
				PlanarImgTex->FlipWidth = MTI->FlipX;
				PlanarImgTex->FlipHeight = MTI->FlipY;
				PlanarImgTex->TexAxis = (unsigned char)MTI->AxisType;
				NameTest->TexRoot[TexChannel]->PreviewDirection = PlanarImgTex->TexAxis < 2 ? PlanarImgTex->TexAxis: 5;
				PlanarImgTex->ImageNeg = NegateImage ? ! MTI->Negative: MTI->Negative;
				PlanarImgTex->AlphaOnly = AlphaOnly;
				PlanarImgTex->TexCenter[0].SetCurValue(0, (double)MTI->Trans[0]);
				PlanarImgTex->TexCenter[1].SetCurValue(0, (double)MTI->Trans[1]);
				PlanarImgTex->TexCenter[2].SetCurValue(0, (double)MTI->Trans[2]);
				PlanarImgTex->TexRotation[0].SetCurValue(0, (double)MTI->Rot[0]);
				PlanarImgTex->TexRotation[1].SetCurValue(0, (double)MTI->Rot[1]);
				PlanarImgTex->TexRotation[2].SetCurValue(0, (double)MTI->Rot[2]);
				PlanarImgTex->TexSize[0].SetCurValue(0, (double)MTI->Scale[0]);
				PlanarImgTex->TexSize[1].SetCurValue(0, (double)MTI->Scale[1]);
				PlanarImgTex->TexSize[2].SetCurValue(0, (double)MTI->Scale[2]);
				PlanarImgTex->TexParam[WCS_TEXTURE_OPACITY].SetCurValue((double)MTI->Opacity);
				if (MTI->UVMapName[0]) 
					PlanarImgTex->SetVertexMap(MTI->UVMapName);
				if (PlanarRaster = GlobalApp->AppImages->AddRaster(MTI->ObjPath, MTI->GetName(), 1, 1, 0))
					{
					PlanarImgTex->SetRaster(PlanarRaster);
					if (MTI->Decal)
						{
						PlanarRaster->AlphaEnabled = 1;
						PlanarImgTex->SelfOpacity = 1;
						} // if
					if (MTI->ClipMap)
						{
						// <<<>>>
						} // if
					} // if
				} // if
			} // if
		} // if
	else if (MTI->GetType() == WCS_TEXTURE_TYPE_UVW)
		{
		if (NameTest->NewRootTexture(TexChannel))
			{
			UVImageTexture *PlanarImgTex;
			if (PlanarImgTex = (UVImageTexture *)NameTest->TexRoot[TexChannel]->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
				{
				// no need to set coordspace
				//PlanarImgTex->CoordSpace = MTI->CoordSpace;
				PlanarImgTex->TileWidth = MTI->TileX;
				PlanarImgTex->TileHeight = MTI->TileY;
				PlanarImgTex->FlipWidth = MTI->FlipX;
				PlanarImgTex->FlipHeight = MTI->FlipY;
				PlanarImgTex->ImageNeg = NegateImage ? ! MTI->Negative: MTI->Negative;
				PlanarImgTex->AlphaOnly = AlphaOnly;
				PlanarImgTex->TexAxis = (unsigned char)MTI->AxisType;
				NameTest->TexRoot[TexChannel]->PreviewDirection = PlanarImgTex->TexAxis < 2 ? PlanarImgTex->TexAxis: 5;
				// no need to set values below
				//PlanarImgTex->TexCenter[0].SetCurValue(0, MTI->Trans[0]);
				//PlanarImgTex->TexCenter[1].SetCurValue(0, MTI->Trans[1]);
				//PlanarImgTex->TexCenter[2].SetCurValue(0, MTI->Trans[2]);
				//PlanarImgTex->TexRotation[0].SetCurValue(0, MTI->Rot[0]);
				//PlanarImgTex->TexRotation[1].SetCurValue(0, MTI->Rot[1]);
				//PlanarImgTex->TexRotation[2].SetCurValue(0, MTI->Rot[2]);
				//PlanarImgTex->TexSize[0].SetCurValue(0, MTI->Scale[0]);
				//PlanarImgTex->TexSize[1].SetCurValue(0, MTI->Scale[1]);
				//PlanarImgTex->TexSize[2].SetCurValue(0, MTI->Scale[2]);
				PlanarImgTex->TexParam[WCS_TEXTURE_OPACITY].SetCurValue((double)MTI->Opacity);
				if (MTI->UVMapName[0]) 
					PlanarImgTex->SetVertexMap(MTI->UVMapName);
				//if (PlanarRaster = GlobalApp->AppImages->AddRaster(MTI->ObjPath, MTI->GetName(), 1, 1, 0))
				// turns out we do need to load n prep the image or we don't know about the alpha channel
				if (PlanarRaster = GlobalApp->AppImages->AddRaster(MTI->ObjPath, MTI->GetName(), 1, 1, 1))
					{
					PlanarImgTex->SetRaster(PlanarRaster);
					if (MTI->Decal)
						{
						PlanarRaster->AlphaEnabled = 1;
						PlanarImgTex->SelfOpacity = 1;
						} // if
					if (MTI->ClipMap)
						{
						// <<<>>>
						} // if
					} // if
				} // if
			} // if
		} // else if
	} // if

return (PlanarRaster);

} // EffectsLib::SetMakeTexture

/*===========================================================================*/

char *ObjectMaterialEntry::SetName(char *NewName)
{

if (NewName)
	{
	strncpy(Name, NewName, WCS_EFFECT_MAXNAMELENGTH);
	Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	return (Name);
	} // if
return (NULL);

} // ObjectMaterialEntry::SetName

/*===========================================================================*/
/*===========================================================================*/

ObjectPerVertexMap::ObjectPerVertexMap()
{

strcpy(Name, "Vertex Map");
NumNodes = 0; 
UVMapType = WCS_OBJPERVERTMAP_MAPTYPE_UV;
CoordsValid = NULL; 
CoordsArray[0] = CoordsArray[1] = CoordsArray[2] = NULL;

} // ObjectPerVertexMap::ObjectPerVertexMap

/*===========================================================================*/

// if you use this version of the constructor you must check to see if allocation worked OK
ObjectPerVertexMap::ObjectPerVertexMap(long NewNumNodes)
{

strcpy(Name, "Vertex Map");
NumNodes = 0; 
UVMapType = WCS_OBJPERVERTMAP_MAPTYPE_UV;
CoordsValid = NULL; 
CoordsArray[0] = CoordsArray[1] = CoordsArray[2] = NULL;
AllocMap(NewNumNodes);

} // ObjectPerVertexMap::ObjectPerVertexMap

/*===========================================================================*/

ObjectPerVertexMap::~ObjectPerVertexMap()
{

FreeMap();

} // ObjectPerVertexMap::~ObjectPerVertexMap

/*===========================================================================*/

char *ObjectPerVertexMap::SetName(char *NewName)
{

if (NewName)
	{
	strncpy(Name, NewName, WCS_EFFECT_MAXNAMELENGTH);
	Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	return (Name);
	} // if
return (NULL);

} // ObjectPerVertexMap::SetName

/*===========================================================================*/

int ObjectPerVertexMap::AllocMap(long NewNumNodes)
{

FreeMap();

if (NewNumNodes > 0)
	{
	NumNodes = NewNumNodes;
	CoordsValid = (char *)AppMem_Alloc(NumNodes * sizeof(char), APPMEM_CLEAR);
	CoordsArray[0] = (float *)AppMem_Alloc(NumNodes * sizeof(float), APPMEM_CLEAR);
	CoordsArray[1] = (float *)AppMem_Alloc(NumNodes * sizeof(float), APPMEM_CLEAR);
	CoordsArray[2] = (float *)AppMem_Alloc(NumNodes * sizeof(float), APPMEM_CLEAR);
	if (CoordsValid && CoordsArray[0] &&CoordsArray[1] &&CoordsArray[2])
		return (1);
	FreeMap();
	} // if

return (0);

} // ObjectPerVertexMap::AllocMap

/*===========================================================================*/

void ObjectPerVertexMap::FreeMap(void)
{

if (NumNodes > 0)
	{
	if (CoordsValid)
		AppMem_Free(CoordsValid, NumNodes * sizeof(char));
	CoordsValid = NULL;
	if (CoordsArray[0])
		AppMem_Free(CoordsArray[0], NumNodes * sizeof(float));
	CoordsArray[0] = NULL;
	if (CoordsArray[1])
		AppMem_Free(CoordsArray[1], NumNodes * sizeof(float));
	CoordsArray[1] = NULL;
	if (CoordsArray[2])
		AppMem_Free(CoordsArray[2], NumNodes * sizeof(float));
	CoordsArray[2] = NULL;
	NumNodes = 0;
	} // if

} // ObjectPerVertexMap::FreeMap

/*===========================================================================*/

unsigned long ObjectPerVertexMap::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, long NumVerts)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long Ct;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_OBJPERVERTMAP_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_BUILD_GARY
						// One of the 3do's in V6 box cover project has the wrong UV table name 
						// in its w3o file which makes it not be recognized by the textures and leaves render transparent.
						// Bark becomes the wrong color. Set the name back to what it should be.
						if (! strcmp(Name, "UV"))
							strcpy(Name, "Vertex Map");
						#endif //WCS_BUILD_GARY
						break;
						}
					case WCS_OBJPERVERTMAP_NUMNODES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumNodes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumNodes < NumVerts)
							{
							goto ReadError;
							} // if
						break;
						}
					case WCS_OBJPERVERTMAP_COORDSVALID:
						{
						if (! CoordsValid)
							AllocMap(NumNodes);
						if (CoordsValid)
							BytesRead = ReadLongBlock(ffile, (char *)CoordsValid, Size);
						else
							goto ReadError;
						break;
						}
					case WCS_OBJPERVERTMAP_COORDSARRAY0:
						{
						if (CoordsArray[0])
							{
							if ((BytesRead = ReadLongBlock(ffile, (char *)CoordsArray[0], Size)) == Size)
								{
								if (ByteFlip)
									{
									for (Ct = 0; Ct < NumNodes; Ct ++)
										{
										SimpleEndianFlip32F((void *)&CoordsArray[0][Ct], (float *)&CoordsArray[0][Ct]);
										} // for
									} // if
								} // if
							} // else
						else
							goto ReadError;
						break;
						}
					case WCS_OBJPERVERTMAP_COORDSARRAY1:
						{
						if (CoordsArray[1])
							{
							if ((BytesRead = ReadLongBlock(ffile, (char *)CoordsArray[1], Size)) == Size)
								{
								if (ByteFlip)
									{
									for (Ct = 0; Ct < NumNodes; Ct ++)
										{
										SimpleEndianFlip32F((void *)&CoordsArray[1][Ct], (float *)&CoordsArray[1][Ct]);
										} // for
									} // if
								} // if
							} // else
						else
							goto ReadError;
						break;
						}
					case WCS_OBJPERVERTMAP_COORDSARRAY2:
						{
						if (CoordsArray[2])
							{
							if ((BytesRead = ReadLongBlock(ffile, (char *)CoordsArray[2], Size)) == Size)
								{
								if (ByteFlip)
									{
									for (Ct = 0; Ct < NumNodes; Ct ++)
										{
										SimpleEndianFlip32F((void *)&CoordsArray[2][Ct], (float *)&CoordsArray[2][Ct]);
										} // for
									} // if
								} // if
							} // else
						else
							goto ReadError;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

ReadError:
return (0);

} // ObjectPerVertexMap::Load

/*===========================================================================*/

unsigned long ObjectPerVertexMap::Save(FILE *ffile, int SaveArrays)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_OBJPERVERTMAP_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (SaveArrays)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_OBJPERVERTMAP_NUMNODES, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&NumNodes)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if (CoordsValid)
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_OBJPERVERTMAP_COORDSVALID, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, NumNodes * sizeof(char),
			WCS_BLOCKTYPE_CHAR, (char *)CoordsValid)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	if (CoordsArray[0])
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_OBJPERVERTMAP_COORDSARRAY0, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, NumNodes * sizeof(float),
			WCS_BLOCKTYPE_CHAR, (char *)CoordsArray[0])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	if (CoordsArray[1])
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_OBJPERVERTMAP_COORDSARRAY1, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, NumNodes * sizeof(float),
			WCS_BLOCKTYPE_CHAR, (char *)CoordsArray[1])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	if (CoordsArray[2])
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_OBJPERVERTMAP_COORDSARRAY2, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, NumNodes * sizeof(float),
			WCS_BLOCKTYPE_CHAR, (char *)CoordsArray[2])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ObjectPerVertexMap::Save

/*===========================================================================*/
/*===========================================================================*/

VertexReferenceData::VertexReferenceData()
{

MapType = WCS_VERTREFDATA_MAPTYPE_UVW;
MapNumber = 0;
ObjVertNumber = VertRefNumber = 0;
NextVertex = NextMap = NULL;

} // VertexReferenceData::VertexReferenceData

/*===========================================================================*/

VertexReferenceData::VertexReferenceData(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber)
{

MapType = NewMapType;
MapNumber = NewMapNumber;
ObjVertNumber = NewObjVertNumber;
VertRefNumber = NewVertRefNumber;
NextVertex = NextMap = NULL;

} // VertexReferenceData::VertexReferenceData

/*===========================================================================*/

VertexReferenceData::~VertexReferenceData()
{
VertexReferenceData *TempRef;

while (NextMap)
	{
	TempRef = NextMap->NextMap;
	NextMap->NextMap = NULL;
	delete NextMap;
	NextMap = TempRef;
	} // while

} // VertexReferenceData::~VertexReferenceData

/*===========================================================================*/

VertexReferenceData *VertexReferenceData::AddVertRef(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber)
{
VertexReferenceData *LastRef = NULL, *CurRef = this;

while (CurRef)
	{
	if (CurRef->ObjVertNumber == NewObjVertNumber)
		{
		while (CurRef)
			{
			if (CurRef->MapType == NewMapType)
				{
				if (CurRef->MapNumber == NewMapNumber)
					{
					break;
					} // if
				} // if
			LastRef = CurRef;
			CurRef = CurRef->NextMap;
			} // while
		if (LastRef && ! CurRef)
			{
			LastRef->NextMap = CurRef = new VertexReferenceData(NewObjVertNumber, NewVertRefNumber, NewMapType, NewMapNumber);
			} // if
		return (CurRef);
		} // if
	LastRef = CurRef;
	CurRef = CurRef->NextVertex;
	} // while

if (LastRef && ! CurRef)
	{
	LastRef->NextVertex = CurRef = new VertexReferenceData(NewObjVertNumber, NewVertRefNumber, NewMapType, NewMapNumber);
	} // if

return (CurRef);

} // VertexReferenceData::AddVertRef

/*===========================================================================*/

long VertexReferenceData::GetMapIndex(unsigned char TestMapType, unsigned char TestMapNumber, long DefaultVertexNumber)
{
VertexReferenceData *CurRef = this;

while (CurRef)
	{
	if (CurRef->MapType == TestMapType)
		{
		if (CurRef->MapNumber == TestMapNumber)
			{
			return (CurRef->VertRefNumber);
			} // if
		} // if
	CurRef = CurRef->NextMap;
	} // while

return (DefaultVertexNumber);

} // VertexReferenceData::GetMapIndex

/*===========================================================================*/
/*===========================================================================*/

Vertex3D::Vertex3D(void)
{

Normal[2] = Normal[1] = Normal[0] = 0.0;
PolyRef = NULL;
NumPolys = 0;
Normalized = 0;

} // Vertex3D::Vertex3D

/*===========================================================================*/

Vertex3D::~Vertex3D(void)
{

if (PolyRef)
	AppMem_Free(PolyRef, NumPolys * sizeof(long));
PolyRef = NULL;

} // Vertex3D::Vertex3D

/*===========================================================================*/

void Vertex3D::Copy(Vertex3D *CopyTo, Vertex3D *CopyFrom)
{

CopyTo->NumPolys = CopyFrom->NumPolys;
CopyTo->Normalized = CopyFrom->Normalized;
CopyTo->Normal[0] = CopyFrom->Normal[0];
CopyTo->Normal[1] = CopyFrom->Normal[1];
CopyTo->Normal[2] = CopyFrom->Normal[2];
CopyTo->CopyVDEM(CopyFrom);

} // Vertex3D::Copy

/*===========================================================================*/

void Vertex3D::InterpolateVertex3D(Vertex3D *FromVtx, Vertex3D *ToVtx, double LerpVal)
{

//RGB[0] = FromVtx->RGB[0] + (float)LerpVal * (ToVtx->RGB[0] - FromVtx->RGB[0]);
//RGB[1] = FromVtx->RGB[1] + (float)LerpVal * (ToVtx->RGB[1] - FromVtx->RGB[1]);
//RGB[2] = FromVtx->RGB[2] + (float)LerpVal * (ToVtx->RGB[2] - FromVtx->RGB[2]);

Normal[0] = FromVtx->Normal[0] + LerpVal * (ToVtx->Normal[0] - FromVtx->Normal[0]);
Normal[1] = FromVtx->Normal[1] + LerpVal * (ToVtx->Normal[1] - FromVtx->Normal[1]);
Normal[2] = FromVtx->Normal[2] + LerpVal * (ToVtx->Normal[2] - FromVtx->Normal[2]);

InterpolateVertexDEM(FromVtx, ToVtx, LerpVal);

} // Vertex3D::InterpolateVertex3D

/*===========================================================================*/

void Vertex3D::Normalize(Polygon3D *Poly, Vertex3D *Vert, Polygon3D *RefPoly, ObjectMaterialEntry *Matl)
{
EffectsLib *AppEffects = GlobalApp->AppEffects;
long MatNum;
short Ct, Included = 0;

if (! Normalized)
	{
	ZeroPoint3d(Normal);
	for (Ct = 0; Ct < NumPolys; Ct ++)
		{
		MatNum = Poly[PolyRef[Ct]].Material;
		if ((Matl[MatNum].Mat || (Matl[MatNum].Mat = AppEffects->SetMakeMaterial(&Matl[MatNum], NULL)))
			&& (Matl[MatNum].Mat->Shading == WCS_EFFECT_MATERIAL_SHADING_FLAT))
			continue;
		Poly[PolyRef[Ct]].Normalize(Vert);
		if (Poly[PolyRef[Ct]].Normal[0] != 0.0 || Poly[PolyRef[Ct]].Normal[1] != 0.0 || Poly[PolyRef[Ct]].Normal[2] != 0.0)
			{
			if (VectorAngle(Poly[PolyRef[Ct]].Normal, RefPoly->Normal) < Matl[RefPoly->Material].Mat->CosSmoothAngle)
				continue;	// VectorAngle returns a cosine which is continuously declining over the angle range 0 - 180
			AddPoint3d(Normal, Poly[PolyRef[Ct]].Normal);
			++Included;
			} // if
		} // for
	if (Included > 0)
		{
		DividePoint3d(Normal, (double)Included);
		::UnitVector(Normal);	// :: calls the global method in Useful.cpp
		} // if
	else
		{
		Normal[0] = RefPoly->Normal[0];
		Normal[1] = RefPoly->Normal[1];
		Normal[2] = RefPoly->Normal[2];
		} // else
//	swmem(&Normal[1], &Normal[2], sizeof(double));
	Normalized = 1;
	} // if

} // Vertex3D::Normalize

/*===========================================================================*/

unsigned long Vertex3D::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, ObjectPerVertexMap **LoadToColors, long NumVerts, long &NumCPVMaps, long VertsRead)
{
ObjectPerVertexMap *VertMap;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long PolysRead = 0;
float TempFloat;

VertMap = *LoadToColors;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_VERTEX3D_NUMPOLYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumPolys, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (NumPolys > 0)
							{
							if ((PolyRef = (long *)AppMem_Alloc(NumPolys * sizeof(long), APPMEM_CLEAR)) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_VERTEX3D_X:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						xyz[0] = TempFloat;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_Y:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						xyz[1] = TempFloat;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_Z:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						xyz[2] = TempFloat;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_NX:
						{
						if (Size == 4)
							{
							BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
							Normal[0] = TempFloat;
							} // if
						else if (Size == 8)
							BytesRead = ReadBlock(ffile, (char *)&Normal[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						Normalized = 1;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_NY:
						{
						if (Size == 4)
							{
							BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
							Normal[1] = TempFloat;
							} // if
						else if (Size == 8)
							BytesRead = ReadBlock(ffile, (char *)&Normal[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_VERTEX3D_NZ:
						{
						if (Size == 4)
							{
							BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
							Normal[2] = TempFloat;
							} // if
						else if (Size == 8)
							BytesRead = ReadBlock(ffile, (char *)&Normal[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_VERTEX3D_RED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						if (! *LoadToColors)
							{
							if (*LoadToColors = new ObjectPerVertexMap[1])
								{
								NumCPVMaps = 1;
								VertMap = *LoadToColors;
								if (! VertMap[0].AllocMap(NumVerts))
									{
									VertMap = NULL;
									} // if
								} // if
							} // if
						if (VertMap && VertsRead < NumVerts)
							VertMap->CoordsArray[0][VertsRead] = TempFloat;
						else
							goto ReadError;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_GREEN:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						if (VertMap && VertsRead < NumVerts)
							VertMap->CoordsArray[1][VertsRead] = TempFloat;
						else
							goto ReadError;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_BLUE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFloat, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						if (VertMap && VertsRead < NumVerts)
							{
							VertMap->CoordsArray[2][VertsRead] = TempFloat;
							VertMap->CoordsValid[VertsRead] = 1;
							VertsRead ++;
							} // if
						else
							goto ReadError;
						break;
						}
					case WCS_EFFECTS_VERTEX3D_POLYREF:
						{
						if (PolysRead < NumPolys)
							BytesRead = ReadBlock(ffile, (char *)&PolyRef[PolysRead], WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						PolysRead ++;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

ReadError:
return (0);

} // Vertex3D::Load

/*===========================================================================*/

unsigned long Vertex3D::Save(FILE *ffile, int SaveNormals)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, PolysWritten;
float TempFloat;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_NUMPOLYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&NumPolys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
TempFloat = (float)xyz[0];
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_X, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
TempFloat = (float)xyz[1];
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_Y, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
TempFloat = (float)xyz[2];
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_Z, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (SaveNormals)
	{
	TempFloat = (float)Normal[0];
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_NX, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	TempFloat = (float)Normal[1];
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_NY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	TempFloat = (float)Normal[2];
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_NZ, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_FLOAT, (char *)&TempFloat)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (NumPolys > 0 && PolyRef)
	{
	for (PolysWritten = 0; PolysWritten < NumPolys; PolysWritten ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_VERTEX3D_POLYREF, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&PolyRef[PolysWritten])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Vertex3D::Save

/*===========================================================================*/

Polygon3D::Polygon3D(void)
{

NumVerts = 0;
Normalized = 0;
VertRef = NULL;
RefData = NULL;
Material = 0;
Normal[0] = Normal[1] = Normal[2] = 0.0;

// the rest doesn't need to be initialized by the constructor

} // Polygon3D::Polygon3D

/*===========================================================================*/

Polygon3D::~Polygon3D(void)
{
VertexReferenceData *TempRef;

if (VertRef)
	AppMem_Free(VertRef, NumVerts * sizeof(long));
VertRef = NULL;

while (RefData)
	{
	TempRef = RefData->NextVertex;
	delete RefData;
	RefData = TempRef;
	} // while

} // Polygon3D::Polygon3D

/*===========================================================================*/

void Polygon3D::Copy(Polygon3D *CopyTo, Polygon3D *CopyFrom)
{

CopyTo->NumVerts = CopyFrom->NumVerts;
CopyTo->Normalized = CopyFrom->Normalized;
CopyTo->Material = CopyFrom->Material;

} // Polygon3D::Copy

/*===========================================================================*/

VertexReferenceData *Polygon3D::AddVertRef(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber)
{

if (RefData)
	return (RefData->AddVertRef(NewObjVertNumber, NewVertRefNumber, NewMapType, NewMapNumber));

return (RefData = new VertexReferenceData(NewObjVertNumber, NewVertRefNumber, NewMapType, NewMapNumber));

} // Polygon3D::AddVertRef

/*===========================================================================*/

VertexReferenceData *Polygon3D::FindVertRefDataHead(long TestObjVertNumber)
{
VertexReferenceData *CurRef = RefData;

while (CurRef)
	{
	if (CurRef->ObjVertNumber == TestObjVertNumber)
		{
		return (CurRef);
		} // if
	CurRef = CurRef->NextVertex;
	} // while

return (NULL);

} // Polygon3D::FindVertRefDataHead

/*===========================================================================*/

void Polygon3D::Normalize(Vertex3D *Vert)
{
Point3d P1, P2;
long LastVert;

if (NumVerts > 2)
	{
	LastVert = NumVerts - 1;
	while (! Normalized)
		{
		// take first and last points and find normal to them
		FindPosVector(P1, Vert[VertRef[1]].XYZ, Vert[VertRef[0]].XYZ);
		FindPosVector(P2, Vert[VertRef[LastVert]].XYZ, Vert[VertRef[0]].XYZ);
		SurfaceNormal(Normal, P1, P2);
		if (Normal[0] != 0.0 || Normal[1] != 0.0 || Normal[2] != 0.0)	// points do not lie on a straight line
			{
			Normalized = 1;
			} // if
		else
			{
			LastVert --;
			if (LastVert <= 1)
				{
				Normalized = 1;
				break;
				} // if no more points to try
			} // else points lie on straight line
		} // if
	} // if
else
	{
	Normal[0] = Normal[1] = Normal[2] = 0.0;
	Normalized = 1;
	} // else

} // Polygon3D::Normalize

/*===========================================================================*/

unsigned long Polygon3D::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long VertsRead = 0, ObjVertNumber = 0, VertRefNumber = 0;
unsigned char VertMapType = 0, VertMapNumber = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_POLYGON3D_NUMVERTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumVerts, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						if (NumVerts > 0)
							{
							if ((VertRef = (long *)AppMem_Alloc(NumVerts * sizeof(long), APPMEM_CLEAR)) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_POLYGON3D_MATERIAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&Material, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POLYGON3D_VERTREF:
						{
						if (VertsRead < NumVerts)
							BytesRead = ReadBlock(ffile, (char *)&VertRef[VertsRead], WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						VertsRead ++;
						break;
						}
					case WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTMAPTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertMapType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTMAPNUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertMapNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POLYGON3D_VERTREFDATA_OBJVERTNUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjVertNumber, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTREFNUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertRefNumber, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (! AddVertRef(ObjVertNumber, VertRefNumber, VertMapType, VertMapNumber))
							goto ReadError;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

ReadError:
return (0);

} // Polygon3D::Load

/*===========================================================================*/

unsigned long Polygon3D::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, VertsWritten;
VertexReferenceData *CurRef, *TempRef;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_NUMVERTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&NumVerts)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_MATERIAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Material)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (NumVerts > 0 && VertRef)
	{
	for (VertsWritten = 0; VertsWritten < NumVerts; VertsWritten ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_VERTREF, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&VertRef[VertsWritten])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // if

CurRef = RefData;
while (CurRef)
	{
	TempRef = CurRef;
	while (TempRef)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTMAPTYPE, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&TempRef->MapType)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTMAPNUMBER, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&TempRef->MapNumber)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_VERTREFDATA_OBJVERTNUMBER, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&TempRef->ObjVertNumber)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POLYGON3D_VERTREFDATA_VERTREFNUMBER, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&TempRef->VertRefNumber)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		TempRef = TempRef->NextMap;
		} // while
	CurRef = CurRef->NextVertex;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Polygon3D::Save

/*===========================================================================*/
/*===========================================================================*/

long Object3DInstance::CountBoundedInstances(RasterBounds *RBounds)
{
double N, S, W, E, X, Y;
long SumCt = 0;
Object3DInstance *CurInstance = this;

if (RBounds && RBounds->CoordsValid)
	{
	if (RBounds->FetchBoundsEdgesGIS(N, S, W, E))
		{
		while (CurInstance)
			{
			RBounds->DefDegToRBounds(CurInstance->WCSGeographic[1], CurInstance->WCSGeographic[0], X, Y);
			if (X >= W && X <= E && Y >= S && Y <= N)
				SumCt ++;
			CurInstance = CurInstance->Next;
			} // while
		} // if
	} // if
else
	{
	while (CurInstance)
		{
		SumCt ++;
		CurInstance = CurInstance->Next;
		} // while
	} // else

return (SumCt);

} // Object3DInstance::CountBoundedInstances

/*===========================================================================*/

long Object3DInstance::IsBounded(RasterBounds *RBounds)
{
double N, S, W, E, X, Y;

if (RBounds && RBounds->CoordsValid)
	{
	if (RBounds->FetchBoundsEdgesGIS(N, S, W, E))
		{
		RBounds->DefDegToRBounds(WCSGeographic[1], WCSGeographic[0], X, Y);
		if (X <= E && X >= W && Y >= S && Y <= N)
			return (1);
		} // if
	return (0);
	} // if

return (1);

} // Object3DInstance::IsBounded

/*===========================================================================*/
/*===========================================================================*/

Object3DEffectBase::Object3DEffectBase(void)
{

SetDefaults();

} // Object3DEffectBase::Object3DEffectBase

/*===========================================================================*/

void Object3DEffectBase::SetDefaults(void)
{

//Resolution = (float)90.0;
//GeoRast = NULL;
VerticesToRender = 0;
VertList = NULL;

} // Object3DEffectBase::SetDefaults

/*===========================================================================*/

int Object3DEffectBase::Init(Object3DEffect *EffectList, RenderJoeList *JL)
{
long Ct;
RenderJoeList *CurJL;
VectorPoint *CurPt;
Object3DEffect *CurEffect;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;
CurJL = JL;
while (CurJL)
	{
	if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
		VerticesToRender += (CurJL->Me->GetNumRealPoints());
	CurJL = (RenderJoeList *)CurJL->Next;
	} // while
for (CurEffect = EffectList; CurEffect; CurEffect = (Object3DEffect *)CurEffect->Next)
	{
	if (CurEffect->GeographicInstance && CurEffect->Enabled && ! CurEffect->ShadowsOnly)
		{
		VerticesToRender ++;
		} // if
	} // for
if (VerticesToRender > 0)
	{
	if (VertList = new Object3DVertexList[VerticesToRender])
		{
		CurJL = JL;
		Ct = 0;
		while (CurJL)
			{
			if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
				{
				if (MyAttr = (JoeCoordSys *)CurJL->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				for (CurPt = CurJL->Me->GetFirstRealPoint(); CurPt; CurPt = CurPt->Next)
					{
					if (CurPt->ProjToDefDeg(MyCoords, &MyVert))
						{
						VertList[Ct].Obj = (Object3DEffect *)CurJL->Effect;
						VertList[Ct].Vec = CurJL->Me;
						VertList[Ct].Lat = MyVert.Lat;
						VertList[Ct].Lon = MyVert.Lon;
						VertList[Ct ++].Point = CurPt;
						} // if
					else
						VerticesToRender --;
					} // for
				} // if
			CurJL = (RenderJoeList *)CurJL->Next;
			} // while
		for (CurEffect = EffectList; CurEffect; CurEffect = (Object3DEffect *)CurEffect->Next)
			{
			if (CurEffect->GeographicInstance && CurEffect->Enabled && ! CurEffect->ShadowsOnly)
				{
				VertList[Ct].Obj = CurEffect;
				VertList[Ct].Vec = NULL;
				VertList[Ct].Lat = CurEffect->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue;
				VertList[Ct].Lon = CurEffect->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue;
				VertList[Ct ++].Point = NULL;
				} // if
			} // for
		} // if
	else
		return (0);
	} // if

return (1);

} // Object3DEffectBase::Init

/*===========================================================================*/

void Object3DEffectBase::Destroy(void)
{

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;

} // Object3DEffectBase::Destroy

/*===========================================================================*/

void Object3DEffectBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{
		ObjVert.Lat = VertList[VertCt].Lat;
		ObjVert.Lon = VertList[VertCt].Lon;
		if (! CurDEM || CurDEM->GeographicPointContained(Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon, TRUE))
			{
			VertList[VertCt].Obj->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			Rend->Cam->ProjectVertexDEM(Rend->DefCoords, &ObjVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_3DOBJECT;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = (float)(ObjVert.ScrnXYZ[2] - Rend->ShadowMapDistanceOffset);
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // Object3DEffectBase::FillRenderPolyArray

/*===========================================================================*/

void Object3DEffectBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt,
	CoordSys *MyCoords, GeoRegister *MyBounds)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

if (! MyBounds)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{
		ObjVert.Lat = VertList[VertCt].Lat;
		ObjVert.Lon = VertList[VertCt].Lon;
		if (MyBounds->GeographicPointContained(MyCoords, Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon))
			{
			VertList[VertCt].Obj->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->DegToCart(&ObjVert);
			#else // WCS_BUILD_VNS
			ObjVert.DegToCart(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_3DOBJECT;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = 1.0f;
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // Object3DEffectBase::FillRenderPolyArray

/*===========================================================================*/

int Object3DEffectBase::InitFrameToRender(void)
{
long VertCt;

if (VertList)
	{
	for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
		{
		VertList[VertCt].Rendered = 0;
		} // for
	} // if

return (1);

} // Object3DEffectBase::InitFrameToRender

/*===========================================================================*/
/*
short Object3DEffectBase::EvalFourPoints(VertexDEM *Vert[3], long &StartRastNum, VectorPoint *&StartPoint)
{
GeoRaster *TestRast = NULL;
double HiLat = -1000000.0, LoLat = 1000000.0, HiLon = -1000000.0, LoLon = 1000000.0;
short HiLatPt, LoLatPt, HiLonPt, LoLonPt, PtCt;
long RastNum = 0;
VectorPoint *TestPoint;

for (PtCt = 0; PtCt < 3; PtCt ++)
	{
	if (Vert[PtCt]->Lat > HiLat)
		{
		HiLat = Vert[PtCt]->Lat;
		HiLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lat < LoLat)
		{
		LoLat = Vert[PtCt]->Lat;
		LoLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon > HiLon)
		{
		HiLon = Vert[PtCt]->Lon;
		HiLonPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon < LoLon)
		{
		LoLon = Vert[PtCt]->Lon;
		LoLonPt = PtCt;
		} // if
	} // for

if (LoLat <= GeoRast->N && HiLat >= GeoRast->S)
	{
	if (LoLon <= GeoRast->W && HiLon >= GeoRast->E)
		{
		TestRast = (GeoRaster *)GeoRast->Next;
		} // if
	} // if
while (TestRast)
	{
	RastNum ++;
	if (LoLat <= TestRast->N && HiLat >= TestRast->S)
		{
		if (LoLon <= TestRast->W && HiLon >= TestRast->E)
			{
			for (TestPoint = TestRast->JLUT[0]->Points->Next; TestPoint; TestPoint = TestPoint->Next)
				{
				if (TestPoint->Latitude <= HiLat && TestPoint->Latitude >= LoLat && 
					TestPoint->Longitude <= HiLon && TestPoint->Longitude >= LoLon)
					{
					if (PointEnclosedPoly3(Vert, TestPoint->Latitude, TestPoint->Longitude))
						{
						StartRastNum = RastNum;
						StartPoint = TestPoint;
						return (1);
						} // if
					} // if
				} // if
			} // if
		} // if
	TestRast = (GeoRaster *)TestRast->Next;
	} // while

return (0);

} // Object3DEffectBase::EvalFourPoints
*/
/*===========================================================================*/
/*
long Object3DEffectBase::EvalThreePoints(RenderData *Rend, VertexDEM *PolyVert[3], long PointsAlreadyRendered, long StartRastNum, VectorPoint *StartPoint,
	VertexDEM *ObjVert, PolygonData *Poly, Object3DEffect *&Object3D)
{
GeoRaster *TestRast;
double HiLat = -1000000.0, LoLat = 1000000.0, HiLon = -1000000.0, LoLon = 1000000.0;
short HiLatPt, LoLatPt, HiLonPt, LoLonPt, PtCt;
long RastNum = 0, PointsFound = 0;
VectorPoint *TestPoint;

for (PtCt = 0; PtCt < 3; PtCt ++)
	{
	if (PolyVert[PtCt]->Lat > HiLat)
		{
		HiLat = PolyVert[PtCt]->Lat;
		HiLatPt = PtCt;
		} // if
	if (PolyVert[PtCt]->Lat < LoLat)
		{
		LoLat = PolyVert[PtCt]->Lat;
		LoLatPt = PtCt;
		} // if
	if (PolyVert[PtCt]->Lon > HiLon)
		{
		HiLon = PolyVert[PtCt]->Lon;
		HiLonPt = PtCt;
		} // if
	if (PolyVert[PtCt]->Lon < LoLon)
		{
		LoLon = PolyVert[PtCt]->Lon;
		LoLonPt = PtCt;
		} // if
	} // for

TestRast = (GeoRaster *)GeoRast->Next;
for (RastNum = 1; RastNum < StartRastNum; RastNum ++)
	TestRast = (GeoRaster *)TestRast->Next;

while (TestRast)
	{
	if (LoLat <= TestRast->N && HiLat >= TestRast->S)
		{
		if (LoLon <= TestRast->W && HiLon >= TestRast->E)
			{
			for (TestPoint = RastNum == StartRastNum ? StartPoint: TestRast->JLUT[0]->Points->Next; TestPoint; TestPoint = TestPoint->Next)
				{
				if (TestPoint->Latitude <= HiLat && TestPoint->Latitude >= LoLat && 
					TestPoint->Longitude <= HiLon && TestPoint->Longitude >= LoLon)
					{
					if (PointEnclosedPoly3(PolyVert, TestPoint->Latitude, TestPoint->Longitude))
						{
						// bingo!
						PointsFound ++;
						if (PointsFound > PointsAlreadyRendered)
							{
							Object3D = (Object3DEffect *)TestRast->LUT[0];
							Object3D->FindBasePosition(Rend, ObjVert, Poly, TestRast->JLUT[0], TestPoint);
							return (PointsFound);
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	TestRast = (GeoRaster *)TestRast->Next;
	RastNum ++;
	} // while

return (-1);

} // Object3DEffectBase::EvalThreePoints
*/
/*===========================================================================*/
/*===========================================================================*/

Object3DEffect::Object3DEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_OBJECT3D;
SetDefaults();

} // Object3DEffect::Object3DEffect

/*===========================================================================*/

Object3DEffect::Object3DEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_OBJECT3D;
SetDefaults();

} // Object3DEffect::Object3DEffect

/*===========================================================================*/

Object3DEffect::Object3DEffect(RasterAnimHost *RAHost, EffectsLib *Library, Object3DEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_OBJECT3D;
Prev = Library->LastObject3D;
if (Library->LastObject3D)
	{
	Library->LastObject3D->Next = this;
	Library->LastObject3D = this;
	} // if
else
	{
	Library->Object3D = Library->LastObject3D = this;
	} // else
Name[0] = NULL;
SetDefaults();	// there are dynamic elements which must be NULLed before calling Copy()
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "3D Object");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Object3DEffect::Object3DEffect

/*===========================================================================*/

Object3DEffect::~Object3DEffect(void)
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->OEG && GlobalApp->GUIWins->OEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->OEG;
		GlobalApp->GUIWins->OEG = NULL;
		} // if
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

FreeNameTable();
FreeUVWMaps();
FreeCPVMaps();
FreeVertices();
FreePolygons();
} // Object3DEffect::~Object3DEffect

/*===========================================================================*/

void Object3DEffect::FreePolygons(void)
{
if (VertRefBlock)
	{
	int PolyClear;
	delete [] VertRefBlock;
	VertRefBlock = NULL;
	if (Polygons)
		{
		// erase hanging references to VertRefBlock so the
		// Polygon3D destructor won't try to free them again
		for (PolyClear = 0; PolyClear < NumPolys; PolyClear++)
			{
			Polygons[PolyClear].VertRef = NULL;
			} // 
		} // if
	} // if
if (Polygons)
	delete [] Polygons;
Polygons = NULL;
NumPolys = 0;
} // Object3DEffect::FreePolygons

/*===========================================================================*/

void Object3DEffect::FreeVertices(void)
{
if (PolyRefBlock)
	{
	int PolyClear;
	delete [] PolyRefBlock;
	PolyRefBlock = NULL;
	if (Vertices)
		{
		// erase hanging references to VertRefBlock so the
		// Polygon3D destructor won't try to free them again
		for (PolyClear = 0; PolyClear < NumVertices; PolyClear++)
			{
			Vertices[PolyClear].PolyRef = NULL;
			} // 
		} // if
	} // if
if (Vertices)
	delete [] Vertices;
Vertices = NULL;
NumVertices = 0;
} // Object3DEffect::FreeVertices

/*===========================================================================*/

void Object3DEffect::FreeNameTable(void)
{

if (NameTable)
	delete [] NameTable;
NameTable = NULL;
NumMaterials = 0;

} // Object3DEffect::FreeNameTable

/*===========================================================================*/

void Object3DEffect::FreeUVWMaps(void)
{

if (UVWTable)
	delete [] UVWTable;
UVWTable = NULL;
NumUVWMaps = 0;

} // Object3DEffect::FreeUVWMaps

/*===========================================================================*/

void Object3DEffect::FreeCPVMaps(void)
{

if (CPVTable)
	delete [] CPVTable;
CPVTable = NULL;
NumCPVMaps = 0;

} // Object3DEffect::FreeCPVMaps

/*===========================================================================*/

void Object3DEffect::FreeDynamicStuff(void)
{

FreeNameTable();

FreeUVWMaps();
FreeCPVMaps();

FreeVertices();
FreePolygons();

} // Object3DEffect::FreeDynamicStuff

/*===========================================================================*/

void Object3DEffect::SetDefaults()
{
double EffectDefault[WCS_EFFECTS_OBJECT3D_NUMANIMPAR] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, .75, .05,
														.5, 
														1.1, 1.0, 1.0, .9, 1.0, 1.0, 0.0, 5.0, 0.0, 0.0, -5.0, 0.0,
														1.0, 0.0, 1.0, -1.0, 0.0, -1.0, .1, .1, .1, 0.0, 0.0, 0.0};
double RangeDefaults[WCS_EFFECTS_OBJECT3D_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, .000001,
															FLT_MAX, -FLT_MAX, .000001,
															FLT_MAX, -10000.0, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, .01,
															FLT_MAX, -FLT_MAX, .01,
															FLT_MAX, -FLT_MAX, .01,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															1.0, 0.0, .01,
															1000.0, 0.0, .01,
															1.0, 0.0, .01,	// vertical alignment bias
															100.0, 0.0, .01,	// scale object x plus
															100.0, 0.0, .01,	// scale object y plus
															100.0, 0.0, .01,	// scale object z plus
															100.0, 0.0, .01,	// scale object x minus
															100.0, 0.0, .01,	// scale object y minus
															100.0, 0.0, .01,	// scale object z minus
															180.0, -180.0, 1.0,	// rotate object x plus
															180.0, -180.0, 1.0,	// rotate object y plus
															180.0, -180.0, 1.0,	// rotate object z plus
															180.0, -180.0, 1.0,	// rotate object x minus
															180.0, -180.0, 1.0,	// rotate object y minus
															180.0, -180.0, 1.0,	// rotate object z minus
															FLT_MAX, -FLT_MAX, .1,	// move object x plus
															FLT_MAX, -FLT_MAX, .1,	// move object y plus
															FLT_MAX, -FLT_MAX, .1,	// move object z plus
															FLT_MAX, -FLT_MAX, .1,	// move object x minus
															FLT_MAX, -FLT_MAX, .1,	// move object y minus
															FLT_MAX, -FLT_MAX, .1,	// move object z minus
															FLT_MAX, -FLT_MAX, .1,	// move vertices x plus
															FLT_MAX, -FLT_MAX, .1,	// move vertices y plus
															FLT_MAX, -FLT_MAX, .1,	// move vertices z plus
															FLT_MAX, -FLT_MAX, .1,	// move vertices x minus
															FLT_MAX, -FLT_MAX, .1,	// move vertices y minus
															FLT_MAX, -FLT_MAX, .1,	// move vertices z minus
															};
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_OBJECT3D_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for

// Notify item WCS_EFFECTS_OBJECT3D_NUMANIMPAR is reserved for changing the DrawEnabled setting.
// Views listen for that item and a WCS_NOTIFYCOMP_OBJECT_VALUECHANGED component to know that 
// display lists may need rebuilding.

strcpy(FilePath, "WCSContent:3DObject");
FileName[0] = 0;
NumVertices = NumPolys = NumMaterials = 0;
AAPasses = 1;
DrawEnabled = WCS_EFFECTS_OBJECT3D_DRAW_CUBE;
ShadowsOnly = CastShadows = 0;
ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = ReceiveShadowsCloudSM = 1;
ReceiveShadowsVolumetric = 0;
ShadowMapWidth = 512;
UseMapFile = RegenMapFile = 0;
NameTable = NULL;
UVWTable = NULL;
CPVTable = NULL;
Vertices = NULL;
Polygons = NULL;
VertRefBlock = NULL;
PolyRefBlock = NULL;
ZeroMatrix4x4(WorldMatx);
ZeroMatrix4x4(InvWorldMatx);
ZeroMatrix3x3(WorldNormalMatx);
ObjectBounds[0] = ObjectBounds[1] = ObjectBounds[2] = ObjectBounds[3] = ObjectBounds[4] = ObjectBounds[5] = 0.0;
CalcNormals = 1;
VertexColorsAvailable = VertexUVWAvailable = 0;
Units = WCS_USEFUL_UNIT_METER;
Elev = 0.0;
AlignHeading = AlignVertical = 0;
HeadingAxis = WCS_EFFECTS_OBJECT3D_ALIGN_Z;
VerticalAxis = WCS_EFFECTS_OBJECT3D_ALIGN_Y;
ReverseHeading = 0;
RandomizeObj[0] = RandomizeObj[1] = RandomizeObj[2] = 0;
RandomizeVert = 0;
ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE;
AlignVertVec = WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR;
Isometric = 1;
AlignSpecialVec = 0;
AlignVec = NULL;
GeographicInstance = 1;
FragmentOptimize = WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPASS;
NumUVWMaps = NumCPVMaps = 0;
ShortName[0] = 0;
ClickQueryEnabled = 0;
lockScales = 1;

AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);

AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));

} // Object3DEffect::SetDefaults

/*===========================================================================*/

void Object3DEffect::Copy(Object3DEffect *CopyTo, Object3DEffect *CopyFrom)
{
long Ct, Result = -1;
MaterialEffect *NextMat, **ToMat;
NotifyTag Changes[2];

CopyTo->FreeNameTable();
CopyTo->FreeUVWMaps();
CopyTo->FreeCPVMaps();
CopyTo->FreeVertices();
CopyTo->FreePolygons();
strcpy(CopyTo->FilePath, CopyFrom->FilePath);
strcpy(CopyTo->FileName, CopyFrom->FileName);
strcpy(CopyTo->ShortName, CopyFrom->ShortName);
CopyTo->NumVertices = CopyFrom->NumVertices;
CopyTo->NumPolys = CopyFrom->NumPolys;
CopyTo->NumMaterials = CopyFrom->NumMaterials;
CopyTo->NumUVWMaps = CopyFrom->NumUVWMaps;
CopyTo->NumCPVMaps = CopyFrom->NumCPVMaps;
CopyTo->AAPasses = CopyFrom->AAPasses;
CopyTo->DrawEnabled = CopyFrom->DrawEnabled;
CopyTo->ShadowsOnly = CopyFrom->ShadowsOnly;
CopyTo->CastShadows = CopyFrom->CastShadows;
CopyTo->ReceiveShadowsTerrain = CopyFrom->ReceiveShadowsTerrain;
CopyTo->ReceiveShadowsFoliage = CopyFrom->ReceiveShadowsFoliage;
CopyTo->ReceiveShadows3DObject = CopyFrom->ReceiveShadows3DObject;
CopyTo->ReceiveShadowsCloudSM = CopyFrom->ReceiveShadowsCloudSM;
CopyTo->ReceiveShadowsVolumetric = CopyFrom->ReceiveShadowsVolumetric;
CopyTo->ShadowMapWidth = CopyFrom->ShadowMapWidth;
CopyTo->UseMapFile = CopyFrom->UseMapFile;
CopyTo->RegenMapFile = CopyFrom->UseMapFile ? 1: 0;
CopyTo->CalcNormals = CopyFrom->CalcNormals;
CopyTo->VertexColorsAvailable = CopyFrom->VertexColorsAvailable;
CopyTo->VertexUVWAvailable = CopyFrom->VertexUVWAvailable;
CopyTo->AlignHeading = CopyFrom->AlignHeading;
CopyTo->AlignVertical = CopyFrom->AlignVertical;
CopyTo->HeadingAxis = CopyFrom->HeadingAxis;
CopyTo->VerticalAxis = CopyFrom->VerticalAxis;
CopyTo->ReverseHeading = CopyFrom->ReverseHeading;
CopyTo->RandomizeObj[0] = CopyFrom->RandomizeObj[0];
CopyTo->RandomizeObj[1] = CopyFrom->RandomizeObj[1];
CopyTo->RandomizeObj[2] = CopyFrom->RandomizeObj[2];
CopyTo->RandomizeVert = CopyFrom->RandomizeVert;
CopyTo->ShowDetail = CopyFrom->ShowDetail;
CopyTo->Isometric = CopyFrom->Isometric;
CopyTo->AlignSpecialVec = CopyFrom->AlignSpecialVec;
CopyTo->AlignVertVec = CopyFrom->AlignVertVec;
CopyTo->GeographicInstance = CopyFrom->GeographicInstance;
CopyTo->FragmentOptimize = CopyFrom->FragmentOptimize;
CopyTo->Units = CopyFrom->Units;
CopyMatrix4x4(CopyTo->WorldMatx, CopyFrom->WorldMatx);
CopyMatrix4x4(CopyTo->InvWorldMatx, CopyFrom->InvWorldMatx);
CopyMatrix3x3(CopyTo->WorldNormalMatx, CopyFrom->WorldNormalMatx);
for (Ct = 0; Ct < 6; Ct ++)
	{
	CopyTo->ObjectBounds[Ct] = CopyFrom->ObjectBounds[Ct];
	} // for
if (CopyFrom->NameTable)
	{
	if (CopyTo->NameTable = new ObjectMaterialEntry[CopyFrom->NumMaterials])
		{
		for (Ct = 0; Ct < CopyFrom->NumMaterials; Ct ++)
			{
			ToMat = &CopyTo->NameTable[Ct].Mat;
			if (! CopyFrom->NameTable[Ct].Mat)
				CopyFrom->NameTable[Ct].Mat = (MaterialEffect *)GlobalApp->CopyFromEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CopyFrom->NameTable[Ct].Name);
			if (NextMat = CopyFrom->NameTable[Ct].Mat)
				{
				if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib || GlobalApp->CopyToEffectsLib != GlobalApp->AppEffects)
					{
					// special treatment for VNS SX exports which might be copying materials not in a library
					if (GlobalApp->CopyToEffectsLib != GlobalApp->AppEffects)
						(*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffectNoValidation(NextMat);
					else
						(*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextMat);
					} // if no need to make another copy, its all in the family
				else
					{
					if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextMat->EffectType, NextMat->Name))
						{
						Result = UserMessageCustom("Copy 3D Object", "How do you wish to resolve Material name collisions?\n\nLink to existing Materials, replace existing Materials, or create new Materials?",
							"Link", "Create", "Overwrite", 0);
						} // if
					if (Result <= 0)
						{
						(*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->AddEffect(NextMat->EffectType, NULL, NextMat);
						} // if create new
					else if (Result == 1)
						{
						(*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextMat);
						} // if link to existing
					else if ((*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->FindByName(NextMat->EffectType, NextMat->Name))
						{
						(*ToMat)->Copy((*ToMat), NextMat);
						Changes[0] = MAKE_ID((*ToMat)->GetNotifyClass(), (*ToMat)->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, (*ToMat));
						} // else if found and overwrite
					else
						{
						(*ToMat) = (MaterialEffect *)GlobalApp->CopyToEffectsLib->AddEffect(NextMat->EffectType, NULL, NextMat);
						} // else
					} // else better copy or overwrite it since its important to get just the right material
				} // if there is a material to copy
			if (*ToMat)
				strcpy(CopyTo->NameTable[Ct].Name, (*ToMat)->Name);
			else
				strcpy(CopyTo->NameTable[Ct].Name, CopyFrom->NameTable[Ct].Name);
			} // for
		} // if
	} // if
if (CopyFrom->UVWTable)
	{
	if (CopyTo->AllocUVWMaps(CopyFrom->NumUVWMaps))
		{
		for (Ct = 0; Ct < CopyFrom->NumUVWMaps; Ct ++)
			{
			strcpy(CopyTo->UVWTable[Ct].Name, CopyFrom->UVWTable[Ct].Name);
			} // for
		} // if
	} // if
if (CopyFrom->CPVTable)
	{
	if (CopyTo->AllocCPVMaps(CopyFrom->NumCPVMaps))
		{
		for (Ct = 0; Ct < CopyFrom->NumCPVMaps; Ct ++)
			{
			strcpy(CopyTo->CPVTable[Ct].Name, CopyFrom->CPVTable[Ct].Name);
			} // for
		} // if
	} // if
// note: there is no attempt to copy the alignment vector, neither is it set to NULL - it remains the same
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Object3DEffect::Copy

/*===========================================================================*/

void Object3DEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->OEG)
	{
	delete GlobalApp->GUIWins->OEG;
	}
GlobalApp->GUIWins->OEG = new Object3DEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if (GlobalApp->GUIWins->OEG)
	{
	GlobalApp->GUIWins->OEG->Open(GlobalApp->MainProj);
	}

} // Object3DEffect::Edit

/*===========================================================================*/

short Object3DEffect::AnimateShadows(void)
{

return (0);

} // Object3DEffect::AnimateShadows

/*===========================================================================*/

short Object3DEffect::AnimateShadow3D(void)
{
short Animate = 0;
long MatNum;
MaterialEffect *MyMat;

if (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].GetNumNodes(0) > 1)
	Animate = 1;
else
	{
	for (MatNum = 0; MatNum < NumMaterials; MatNum ++)
		{
		if (MyMat = GlobalApp->AppEffects->SetMakeMaterial(&NameTable[MatNum], NULL))
			{
			if (MyMat->AnimateShadow3D())
				{
				Animate = 1;
				break;
				} // if
			} // if
		} // for
	for (MatNum = 0; MatNum < GetNumTextures(); MatNum ++)
		{
		if (GetTexRootPtr(MatNum) && GetTexRootPtr(MatNum)->GetRAHostAnimated())
			{
			Animate = 1;
			break;
			} // if
		} // for
	} // else

return (Animate);

} // Object3DEffect::AnimateShadow3D

/*===========================================================================*/

char *Object3DEffectCritterNames[WCS_EFFECTS_OBJECT3D_NUMANIMPAR] = {"Latitude (deg)", "Longitude (deg)", "Elevation (m)", "Rotation X (deg)",
						"Rotation Y (deg)", "Rotation Z (deg)",
						"Scaling X", "Scaling Y",
						"Scaling Z", "Translation X",
						"Translation Y", "Translation Z",
						"Shadow Intensity (%)", "Shadow Offset (m)",
						"Alignment Bias (%)",
						"Scale X High (%)",
						"Scale Y High (%)",
						"Scale Z High (%)",
						"Scale X Low (%)",
						"Scale Y Low (%)",
						"Scale Z Low (%)",
						"Rotate X High (deg)",
						"Rotate Y High (deg)",
						"Rotate Z High (deg)",
						"Rotate X Low (deg)",
						"Rotate Y Low (deg)",
						"Rotate Z Low (deg)",
						"Position X High (m)",
						"Position Y High (m)",
						"Position Z High (m)",
						"Position X Low (m)",
						"Position Y Low (m)",
						"Position Z Low (m)",
						"Vtx Position X High (m)",
						"Vtx Position Y High (m)",
						"Vtx Position Z High (m)",
						"Vtx Position X Low (m)",
						"Vtx Position Y Low (m)",
						"Vtx Position Z Low (m)"
						};
char *Object3DEffectTextureNames[WCS_EFFECTS_OBJECT3D_NUMTEXTURES] = {
						"Scale Overall Range (%)",
						"Scale X Range (%)",
						"Scale Y Range (%)",
						"Scale Z Range (%)",
						"Rotate Overall Range (%)",
						"Rotate X Range (%)",
						"Rotate Y Range (%)",
						"Rotate Z Range (%)",
						"Position Overall Range (%)",
						"Position X Range (%)",
						"Position Y Range (%)",
						"Position Z Range (%)",
						"Vtx Position Overall Range (%)",
						"Vtx Position X Range (%)",
						"Vtx Position Y Range (%)",
						"Vtx Position Z Range (%)",
						};
char *Object3DEffectThemeNames[WCS_EFFECTS_OBJECT3D_NUMTHEMES] = {
						"Rotation X (deg)", "Rotation Y (deg)", "Rotation Z (deg)",
						"Scaling X", "Scaling Y",
						"Scaling Z", "Translation X",
						"Translation Y", "Translation Z",
						};

char *Object3DEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (Object3DEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (Object3DEffectTextureNames[Ct]);
		} // if
	} // for
return ("");

} // Object3DEffect::GetCritterName

/*===========================================================================*/

char *Object3DEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a 3D Object Texture! Remove anyway?");

} // Object3DEffect::OKRemoveRaster

/*===========================================================================*/

char *Object3DEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? Object3DEffectTextureNames[TexNumber]: (char*)"");

} // Object3DEffect::GetTextureName

/*===========================================================================*/

void Object3DEffect::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 0;
ApplyToDisplace = 0;

} // Object3DEffect::GetTextureApplication

/*===========================================================================*/

RootTexture *Object3DEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Object3DEffect::NewRootTexture

/*===========================================================================*/

char *Object3DEffect::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? Object3DEffectThemeNames[ThemeNum]: "");

} // Object3DEffect::GetThemeName

/*===========================================================================*/

int Object3DEffect::ObjectCartesianLegal(RootTexture *TexTest)
{
long Ct;

for (Ct = WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION; Ct <= WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ; Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct] == TexTest)
		return (1);
	} // for

return (0);

} // Object3DEffect::ObjectCartesianLegal

/*===========================================================================*/

void Object3DEffect::MaterialNameChanging(char *OldName, char *NewName)
{
long Ct;

for (Ct = 0; Ct < NumMaterials; Ct ++)
	{
	if (! NameTable[Ct].CompareName(OldName))
		NameTable[Ct].SetName(NewName);
	} // for


} // Object3DEffect::MaterialNameChanging

/*===========================================================================*/

int Object3DEffect::MatchMaterialName(char *MatchName)
{
long Ct;

for (Ct = 0; Ct < NumMaterials; Ct ++)
	{
	if (! NameTable[Ct].CompareName(MatchName))
		return (1);
	} // for

return (0);

} // Object3DEffect::MatchMaterialName

/*===========================================================================*/

ObjectPerVertexMap *Object3DEffect::MatchVertexMap(int MapType, char *MatchName, unsigned char &MapNumber)
{
unsigned char Ct;

if (MapType == 0)	// UVMap
	{
	if (UVWTable)
		{
		for (Ct = 0; Ct < NumUVWMaps; Ct ++)
			{
			if (! strcmp(UVWTable[Ct].Name, MatchName))
				{
				if (UVWTable[Ct].CoordsValid && UVWTable[Ct].CoordsArray[0] && UVWTable[Ct].CoordsArray[2] && UVWTable[Ct].CoordsArray[2])
					{
					MapNumber = Ct;
					return (&UVWTable[Ct]);
					} // if
				return (NULL);
				} // if
			} // for
		} // if
	} // if
else	// color per vertex
	{
	if (CPVTable)
		{
		for (Ct = 0; Ct < NumCPVMaps; Ct ++)
			{
			if (! strcmp(CPVTable[Ct].Name, MatchName))
				{
				if (CPVTable[Ct].CoordsValid && CPVTable[Ct].CoordsArray[0] && CPVTable[Ct].CoordsArray[2] && CPVTable[Ct].CoordsArray[2])
					{
					MapNumber = Ct;
					return (&CPVTable[Ct]);
					} // if
				return (NULL);
				} // if
			} // for
		} // if
	} // else

return (NULL);

} // Object3DEffect::MatchVertexMap

/*===========================================================================*/

ObjectPerVertexMap *Object3DEffect::GetVertexMapNumber(int MapType, char *MatchName, unsigned char &MapNumber)
{
unsigned char Ct;

if (MapType == 0)	// UVMap
	{
	if (UVWTable)
		{
		for (Ct = 0; Ct < NumUVWMaps; Ct ++)
			{
			if (! strcmp(UVWTable[Ct].Name, MatchName))
				{
				MapNumber = Ct;
				return (&UVWTable[Ct]);
				} // if
			} // for
		} // if
	} // if
else	// color per vertex
	{
	if (CPVTable)
		{
		for (Ct = 0; Ct < NumCPVMaps; Ct ++)
			{
			if (! strcmp(CPVTable[Ct].Name, MatchName))
				{
				MapNumber = Ct;
				return (&CPVTable[Ct]);
				} // if
			} // for
		} // if
	} // else

return (NULL);

} // Object3DEffect::GetVertexMapNumber

/*===========================================================================*/

int Object3DEffect::GetMaterialBoundsXYZ(char *MatchName, double &HighX, double &LowX, double &HighY, double &LowY, double &HighZ, double &LowZ)
{
long MatCt, PolyCt, VertCt;
int Found = 0;
Vertex3D *CurVert;

HighX = HighY = HighZ = -FLT_MAX;
LowX = LowY = LowZ = FLT_MAX;

for (MatCt = 0; MatCt < NumMaterials; MatCt ++)
	{
	// same material can be used more than once in name table list so don't quit after finding one instance
	if (! NameTable[MatCt].CompareName(MatchName))
		{
		if (! (Polygons && Vertices))
			{
			// load from file
			OpenInputFile(NULL, FALSE, FALSE, FALSE);
			} // if
		if (Polygons && Vertices)
			{
			for (PolyCt = 0; PolyCt < NumPolys; PolyCt ++)
				{
				if (Polygons[PolyCt].Material == MatCt && Polygons[PolyCt].VertRef)
					{
					for (VertCt = 0; VertCt < Polygons[PolyCt].NumVerts; VertCt ++)
						{
						CurVert = &Vertices[Polygons[PolyCt].VertRef[VertCt]];
						if (CurVert->xyz[0] > HighX)
							{
							HighX = CurVert->xyz[0];
							Found = 1;
							} // if
						if (CurVert->xyz[0] < LowX)
							LowX = CurVert->xyz[0];
						if (CurVert->xyz[1] > HighY)
							HighY = CurVert->xyz[1];
						if (CurVert->xyz[1] < LowY)
							LowY = CurVert->xyz[1];
						if (CurVert->xyz[2] > HighZ)
							HighZ = CurVert->xyz[2];
						if (CurVert->xyz[2] < LowZ)
							LowZ = CurVert->xyz[2];
						} // for
					} // if
				} // for
			} // if
		} // if
	} // for

if (! Found)
	{
	HighX = LowX = 0.0;
	HighY = LowY = 0.0;
	HighZ = LowZ = 0.0;
	} // if

return (Found);

} // Object3DEffect::GetMaterialBoundsXYZ

/*===========================================================================*/

int Object3DEffect::VectorNotInJoeList(Joe *TestVec)
{
JoeList *CurJoe;

CurJoe = Joes;
while (CurJoe)
	{
	if (CurJoe->Me == TestVec)
		{
		return (0);
		} // if
	CurJoe = CurJoe->Next;
	} // while

return (1);

} // Object3DEffect::VectorNotInJoeList

/*===========================================================================*/

void Object3DEffect::VectorAdded(void)
{
NotifyTag Changes[2];

if (GeographicInstance)
	{
	if (UserMessageYN(Name, "Disable Geographic Instance rendering?"))
		{
		GeographicInstance = 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

} // Object3DEffect::VectorAdded

/*===========================================================================*/

int Object3DEffect::SetAlignVec(Joe *NewVec)
{
NotifyTag Changes[2];
JoeAlign3DObj *Attr;

// test to see if it is a placement vector
if (NewVec && ! VectorNotInJoeList(NewVec))
	{
	UserMessageOK("Name", "Vector is already used as a placement vector for this 3D object and shouldn't be used for alignment too.");
	return (0);
	} // if

if (AlignVec)
	{
	if (AlignVec == NewVec)
		return (0);
	RemoveRAHost(AlignVec);
	AlignSpecialVec = 0;
	} // if

AlignVec = NewVec;
if (AlignVec)
	{
	if (Attr = new JoeAlign3DObj())
		{
		Attr->Obj = this;
		AlignVec->AddAttributeNotify(Attr);
		} // if
	AlignSpecialVec = 1;
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Object3DEffect::SetAlignVec

/*===========================================================================*/

int Object3DEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
long Ct, Removed = 0;
Joe *TempVec;
NotifyTag Changes[2];

if (AlignVec && AlignVec == RemoveMe)
	{
	TempVec = AlignVec;
	AlignVec = NULL;
	TempVec->RemoveRAHost(this);
	Removed = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	Removed = 1;
	} // if
for (Ct = 0; Ct < NumMaterials; Ct ++)
	{
	if (NameTable[Ct].Mat && NameTable[Ct].Mat == (MaterialEffect *)RemoveMe)
		{
		NameTable[Ct].Mat = NULL;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		Removed = 1;
		} // if
	} // for

return (GeneralEffect::RemoveRAHost(RemoveMe) || Removed);

} // Object3DEffect::RemoveRAHost

/*===========================================================================*/

char Object3DEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	)
	return (1);

return (0);

} // Object3DEffect::GetRAHostDropOK

/*===========================================================================*/

int Object3DEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success, Result, AlreadyJoes;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Object3DEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Object3DEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Success = SetQuery((SearchQuery *)DropSource->DropSource);
		} // if
	} // else if
#endif // WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s for position or alignment?", DropSource->Name, DropSource->Type, NameStr);
	AlreadyJoes = Joes ? 1: 0;
	if ((Result = UserMessageCustom(NameStr, QueryStr, "Position", "Cancel", "Alignment", 1)) == 1)
		{
		if (AlignVec && AlignVec == (Joe *)DropSource->DropSource)
			{
			UserMessageOK("Name", "Error: Vector is used as an alignment vector for this 3D Object and shouldn't be used for placement too.");
			} // if
		else if (((Joe *)DropSource->DropSource)->AddEffect(this, -1))
			{
			Success = 1;
			if (! AlreadyJoes)
				VectorAdded();
			} // if
		} // if position
	else if (Result == 2)
		{
		Success = SetAlignVec((Joe *)DropSource->DropSource);	// takes care of notification
		} // else if alignment
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // Object3DEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Object3DEffect::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_3DOBJECT | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Object3DEffect::GetRAFlags

/*===========================================================================*/

void Object3DEffect::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_MOVEELEV | WCS_RAHOST_INTERBIT_MOVEY |
					WCS_RAHOST_INTERBIT_SCALEX | WCS_RAHOST_INTERBIT_SCALEY |
					WCS_RAHOST_INTERBIT_SCALEZ |
					WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_ROTATEY |
					WCS_RAHOST_INTERBIT_ROTATEZ);
if (GeographicInstance)
	Prop->InterFlags |= (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEZ);

} // Object3DEffect::GetInterFlags

/*===========================================================================*/

int Object3DEffect::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
double NewVal, TerrainElev;
PlanetOpt *DefPlanetOpt = NULL;

if (! MoveMe)
	{
	return (0);
	} // if

if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS ||
	Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (GeographicInstance)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
		if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
		if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
			{
			if (Absolute == WCS_EFFECT_ABSOLUTE)
				AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
			} // if
		return (1);
		} // if
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue;
	if (Absolute == WCS_EFFECT_ABSOLUTE)
		Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
	else if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
		{
		TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue,
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue);
		TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
		Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue + TerrainElev;
		} // else
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (GeographicInstance)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
		if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
		} // if
	if (Data->MoveZ && Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		if (Absolute == WCS_EFFECT_ABSOLUTE)
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		else if (DefPlanetOpt)
			{
			TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue,
				AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue);
			TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
			AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION] - TerrainElev);
			} // else if
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SCALE)
	{
	// allow size factor
	NewVal = Data->MoveX / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue * NewVal);
	NewVal = -Data->MoveY / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue * NewVal);
	NewVal = Data->MoveZ / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue * NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETSIZE)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		{
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		{
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ])
		{
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_ROTATE)
	{
	// allow heading and pitch
	NewVal = Data->MoveX * .5;
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].CurValue + NewVal);
	NewVal = Data->MoveY * .5;
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].CurValue + NewVal);
	NewVal = Data->MoveZ * .5;
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].SetCurValue(AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].CurValue + NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETROTATION)
	{
	// allow heading and pitch
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALY]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ])
		AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALZ]);
	} // if 

return (0);	// return 0 if nothing changed

} // Object3DEffect::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *Object3DEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
long Ct, RepeatCt, ReturnIt, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < NumMaterials; Ct ++) 
	{
	if (! NameTable[Ct].Mat)
		NameTable[Ct].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable[Ct].Name);
	if (Found && NameTable[Ct].Mat)
		{
		// we must not return the same material twice or endless loops can result
		ReturnIt = 1;
		for (RepeatCt = 0; RepeatCt < Ct; RepeatCt ++)
			{
			if (NameTable[Ct].Mat == NameTable[RepeatCt].Mat)
				{
				ReturnIt = 0;
				break;
				} // if
			} // for
		if (ReturnIt)
			return (NameTable[Ct].Mat);
		} // if
	if (Current == NameTable[Ct].Mat)
		Found = 1;
	} // while
#ifdef WCS_BUILD_VNS
if (Found && Search)
	return (Search);
if (Search && Current == Search)
	Found = 1;
#endif // WCS_BUILD_VNS
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while
if (Found && AlignVec && VectorNotInJoeList(AlignVec))
	return (AlignVec);

return (NULL);

} // Object3DEffect::GetRAHostChild

/*===========================================================================*/

int Object3DEffect::RemoveRAHostQuery(RasterAnimHost *RemoveMe)
{
char QueryStr[256], NameStr[128];
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
RemoveMe->GetRAHostProperties(&Prop);
if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	{
	UserMessageOK(NameStr, "The material you are attempting to remove is used in this 3D Object and cannot be removed.");
	return (0);
	} // if

sprintf(QueryStr, "Remove %s %s from %s?", Prop.Name, Prop.Type, NameStr);
return (UserMessageCustom(NameStr, QueryStr, "Yes", "No", "Yes to All", 0));

} // Object3DEffect::RemoveRAHostQuery

/*===========================================================================*/

int Object3DEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_SCALINGX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_SCALINGY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetThemeAddr(Ct))
			{
			ThemeAffil = (ThematicMap **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_SCALINGX:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_SCALINGY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY);
					break;
					} // 
				case WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Object3DEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *Object3DEffect::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LON));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LON))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV))
	return (GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT));

return (NULL);

} // Object3DEffect::GetNextGroupSibling

/*===========================================================================*/

int Object3DEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
long Count;

for (Count = 0; Count < NumMaterials; Count ++)
	{
	if (! GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Count], NULL))
		return (0);
	} // for

ShadowFlags = 0;
if (ReceiveShadowsTerrain)
	ShadowFlags |= WCS_SHADOWTYPE_TERRAIN;
if (ReceiveShadowsFoliage)
	ShadowFlags |= WCS_SHADOWTYPE_FOLIAGE;
if (ReceiveShadows3DObject)
	ShadowFlags |= WCS_SHADOWTYPE_3DOBJECT;
if (ReceiveShadowsCloudSM)
	ShadowFlags |= WCS_SHADOWTYPE_CLOUDSM;
if (ReceiveShadowsVolumetric)
	ShadowFlags |= WCS_SHADOWTYPE_VOLUME;

return (1);

} // Object3DEffect::InitToRender

/*===========================================================================*/

int Object3DEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

if ((Absolute  == WCS_EFFECT_ABSOLUTE) && Rend->ExagerateElevLines)
	Elev = (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	Elev = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // Object3DEffect::InitFrameToRender

/*===========================================================================*/

int Object3DEffect::OpenInputFileRequest(void)
{
char Ptrn[64], *Ext;
int Success = 0;
int OOBCondition;
NotifyTag Changes[2];
Object3DEffect *Backup;

strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("w3o")";"WCS_REQUESTER_PARTIALWILD("w3d")";"WCS_REQUESTER_PARTIALWILD("lwo"));
#ifdef WCS_SUPPORT_3DS
strcat(Ptrn, ";"WCS_REQUESTER_PARTIALWILD("3ds"));
#endif // WCS_SUPPORT_3DS
strcat(Ptrn, ";"WCS_REQUESTER_PARTIALWILD("dxf"));
#ifdef WCS_SUPPORT_OBJ
strcat(Ptrn, ";"WCS_REQUESTER_PARTIALWILD("obj"));
#endif // WCS_SUPPORT_OBJ

if (Backup = new Object3DEffect())
	{
	Copy(Backup, this);
	if (GetFileNamePtrn(0, "Select 3D Object", Backup->FilePath, Backup->FileName, Ptrn, 64))
		{
		Ext = FindFileExtension(Backup->FileName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Loading 3D Object.");
		if (Success = Backup->OpenInputFile(Ext, TRUE, TRUE, TRUE))
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "3D Object loaded OK.");
			if (Backup->Vertices)
				{
				Backup->GetObjectBounds();
				if (OOBCondition = Backup->TestOriginOutOfBounds())
					{
					if (Backup->OutOfBoundsMessage(OOBCondition))
						{
						Backup->RecenterObject();
						} // if
					} // if
				} // if
			Copy(this, Backup);
			RegenMapFile = (UseMapFile || RegenMapFile);
			StripExtension(Backup->FileName);
			SetUniqueName(GlobalApp->AppEffects, Backup->FileName);
			Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		else
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, "3D Object load failed.");
		} // if
	delete Backup;
	} // if

return (Success);

} // Object3DEffect::OpenInputFileRequest

/*===========================================================================*/

/* obsolete
short Object3DEffect::OpenInputFile(short LoadIt, short SaveIt, char *Ext, short LoadMaterials)
{
char filename[256], ChunkTag[5];
short Format = 0, Success = 0;
FILE *fInput;

if (SaveIt || ! (Success = LoadObjectW3O(LoadMaterials)) || ! (Success = LoadObjectW3D(LoadMaterials)))
	{
	ChunkTag[4] = 0;
	if (FilePath[0] && FileName[0])
		{
		strmfp(filename, FilePath, FileName);
		if (! Ext)
			Ext = FindFileExtension(FileName);
		if (fInput = PROJ_fopen(filename, "rb"))
			{
			if (fread(ChunkTag, 8, 1, fInput) != 1)
				goto ReadError;
			// test to see if it's a LW file, if not IFF then check file extension to see what it might be
			if (! strncmp(ChunkTag, "FORM", 4))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE;
				} // could be Lightwave
			else if (! strnicmp(ChunkTag, "WCS File", 8))
				{
				if (Ext && ! stricmp(Ext, "w3o"))
					{
					Format = WCS_EFFECT_OBJECT3D_FORMAT_W3O;
					SaveIt = 0;
					} // else
				else if (Ext && ! stricmp(Ext, "w3d"))
					{
					Format = WCS_EFFECT_OBJECT3D_FORMAT_W3D;
					SaveIt = 0;
					} // else
				else
					UserMessageOK("Load 3D Object", "Not a recognized "APP_TLA" 3D Object file extension.");
				} // could be WCS
			else if (Ext && ! stricmp(Ext, "dxf"))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_DXF;
				} // else
			else if (Ext && ! stricmp(Ext, "3ds"))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_3DS;
				} // else
			else
				UserMessageOK("Load 3D Object", "Not a recognized 3D Object file format.");
			fclose(fInput);
			} // if
		else
			UserMessageOK("Load 3D Object", "Error opening file to read. Possible reason is invalid file name or path.");
		} // if
	else
		return (0);

	switch (Format)
		{
		case WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE:
			{
			Success = ReadLWObjectFile(filename, LoadIt);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE
		case WCS_EFFECT_OBJECT3D_FORMAT_DXF:
			{
			Success = ReadDXFObjectFile(filename, LoadIt);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_DXF
		#ifdef WCS_SUPPORT_3DS
		case WCS_EFFECT_OBJECT3D_FORMAT_3DS:
			{
			Success = Read3dsObjectFile(filename, LoadIt);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_3DS
		#endif // WCS_SUPPORT_3DS
		case WCS_EFFECT_OBJECT3D_FORMAT_W3O:
			{
			Success = LoadObjectW3O(LoadMaterials);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_W3O
		case WCS_EFFECT_OBJECT3D_FORMAT_W3D:
			{
			Success = LoadObjectW3D(LoadMaterials);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_W3D
		} // switch

	if (Success && SaveIt && Format != WCS_EFFECT_OBJECT3D_FORMAT_W3O && Format != WCS_EFFECT_OBJECT3D_FORMAT_W3D)
		{
		SaveObjectW3O();
		} // if
	} // if

return (Success);

ReadError:

UserMessageOK(Name, "Error reading 3D Object input file!");
return (0);

} // Object3DEffect::OpenInputFile
*/
/*===========================================================================*/

// Ext is only valid if this is a user selecting a new object
int Object3DEffect::OpenInputFile(char *Ext, short LoadMaterials, short LoadParameters, short Queries)
{
FILE *fInput;
NotifyTag Changes[3];
int Format = 0, Success = 0, SaveIt = 0;
char filename[256], ChunkTag[5], LoadWCSFile = 1;

// when user selects a new file to open and it is not a WCS file we need to look to the original 
// even if a WCS file exists. w3o files do not contain material colors like a LightWave or 3ds file do.
// Ext is valid only if this function is called by user request.
if (Ext)
	{
	if (stricmp(Ext, "w3o") && stricmp(Ext, "w3d"))
		LoadWCSFile = 0;
	} // if

if (! (LoadWCSFile && ((Success = LoadObjectW3O(LoadMaterials)) || (Success = LoadObjectW3D(LoadMaterials, LoadParameters, Queries)))))
	{
	ChunkTag[4] = 0;
	if (FilePath[0] && FileName[0])
		{
		strmfp(filename, FilePath, FileName);
		if (! Ext)
			Ext = FindFileExtension(FileName);
		if (fInput = PROJ_fopen(filename, "rb"))
			{
			if (fread(ChunkTag, 4, 1, fInput) != 1)
				goto ReadError;
			// test to see if it's a LW file, if not IFF then check file extension to see what it might be
			if (! strncmp(ChunkTag, "FORM", 4))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE;
				SaveIt = 1;
				} // could be Lightwave
			else if (Ext && ! stricmp(Ext, "dxf"))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_DXF;
				SaveIt = 1;
				} // else
			else if (Ext && ! stricmp(Ext, "3ds"))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_3DS;
				SaveIt = 1;
				} // else
			#ifdef WCS_SUPPORT_OBJ
			else if (Ext && ! stricmp(Ext, "obj"))
				{
				Format = WCS_EFFECT_OBJECT3D_FORMAT_OBJ;
				SaveIt = 1;
				} // else if
			#endif // WCS_SUPPORT_OBJ
			else
				{
				UserMessageOK(Name, "Not a recognized 3D Object file format.");
				Enabled = 0;
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, this);
				} // else
			fclose(fInput);
			} // if
		else
			{
			char PathStash[512];
			sprintf(PathStash, "Error opening file to read: %s", filename);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, PathStash);
			UserMessageOK(Name, "Error opening file to read. Possible reason is invalid file name or path.");
			Enabled = 0;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, this);
			} // else
		} // if
	else
		return (0);

	// Lock S@G while loading complex stuff
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);

	switch (Format)
		{
		case WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE:
			{
			Success = ReadLWObjectFile(filename, TRUE);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE
		case WCS_EFFECT_OBJECT3D_FORMAT_DXF:
			{
			Success = ReadDXFObjectFile(filename, TRUE);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_DXF
		#ifdef WCS_SUPPORT_3DS
		case WCS_EFFECT_OBJECT3D_FORMAT_3DS:
			{
			Success = Read3dsObjectFile(filename, TRUE);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_3DS
		#endif // WCS_SUPPORT_3DS
		#ifdef WCS_SUPPORT_OBJ
		case WCS_EFFECT_OBJECT3D_FORMAT_OBJ:
			{
			Success = ReadWF_OBJFile(filename, TRUE);
			break;
			} // WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE
		#endif // WCS_SUPPORT_OBJ
		} // switch

	if (Success && SaveIt)
		{
		SaveObjectW3O();
		} // if

	// Unlock S@G
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);

	} // if

return (Success);

ReadError:

UserMessageOK(Name, "Error reading 3D Object input file!");
return (0);

} // Object3DEffect::OpenInputFile

/*===========================================================================*/

short Object3DEffect::ReadLWObjectFile(char *filename, short LoadObj)
{
short Success = 1, SubPolys, PolyVertices, PolyMaterial, Vtx;
unsigned short *TempPoly;
unsigned long ChunkTagID;
long ChunkSize, NamePoolSize = 0, Count, SizeRead = 0, LWO2PassOneMarker, FileNumUVMaps = 0, FileNumRGBMaps = 0, FileNumClips = 0;
float FloatVert[3];
Point3f LayerPivot;
char ChunkTag[5], *NamePtr, *NamePool = NULL, LWO2 = 0, POLSOneShot, PNTSOneShot;
long NumTT = 0;
MaterialAttributes *TextureTable = NULL;
FILE *fInput;
DynFileBlock TDFB; // Tags DFB: automatically destroyed/freed when we leave scope
DynFileBlock CDFB; // Clips DFB: automatically destroyed/freed when we leave scope
LWTagTable LWTT;
ObjectPerVertexMap *VertMap = NULL;
LWVertMapEntry *LWVME = NULL;
LWClipEntry *ClipChain = NULL;

FreeDynamicStuff();
CalcNormals = 1;
VertexColorsAvailable = VertexUVWAvailable = 0;

LayerPivot[0] = LayerPivot[1] = LayerPivot[2] = 0.0f;

if (fInput = PROJ_fopen(filename, "rb"))
	{
	if (fread(ChunkTag, 4, 1, fInput) == 1)
		{
		if (! strncmp(ChunkTag, "FORM", 4))
			{
			if (fread(&ChunkSize, 4, 1, fInput) == 1)
				{
				if (fread(ChunkTag, 4, 1, fInput) == 1)
					{
					if ((! strncmp(ChunkTag, "LWOB", 4)) || (! strncmp(ChunkTag, "LWO2", 4)))
						{
						if (! strncmp(ChunkTag, "LWO2", 4))
							{
							LWO2 = 1;
							} // if
						if (LWO2)
							{
							unsigned long NextSurfID = 0;
							LWO2PassOneMarker = ftell(fInput);

							// make a first pass through the file building surface definitions
							while (Success && fread(ChunkTag, 4, 1, fInput) == 1)
								{
								if (fread(&ChunkSize, 4, 1, fInput) == 1)
									{
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip32S(ChunkSize, &ChunkSize);
									#endif // BYTEORDER_LITTLEENDIAN
									if (ChunkSize > 0)
										{
										ChunkTagID = MAKE_ID(ChunkTag[0], ChunkTag[1], ChunkTag[2], ChunkTag[3]);
										switch (ChunkTagID)
											{
											case 'CLIP':
												{
												unsigned char *ChunkData, *NextSubChunk;
												if (ChunkData = CDFB.ReadDynBlock(fInput, ChunkSize))
													{
													unsigned long ClipIndex;

													memcpy(&ClipIndex, ChunkData, sizeof(unsigned long));
													#ifdef BYTEORDER_LITTLEENDIAN
													SimpleEndianFlip32U(ClipIndex, &ClipIndex);
													#endif // BYTEORDER_LITTLEENDIAN
													ChunkData += sizeof(unsigned long);

													while (ChunkData < CDFB.GetBlockEnd())
														{
														unsigned short SubChunkSize;
														unsigned long SubChunkID;

														SubChunkID = MAKE_ID(ChunkData[0], ChunkData[1], ChunkData[2], ChunkData[3]);
														ChunkData += 4;
														memcpy(&SubChunkSize, ChunkData, sizeof(unsigned short));
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(SubChunkSize, &SubChunkSize);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short);
														NextSubChunk = ChunkData + SubChunkSize; // default skipahead handling

														switch(SubChunkID)
															{
															case 'STIL':
																{ // currently we only grok still images
																LWClipEntry *LWCE = NULL;
																// PassData is a sub-pointer into the CDFB object and therefore is
																// persistent until we exit this method
																// by that time we will have copied the image name/path into
																// a more persistant temporary object, the MaterialTextureInfo
																if (LWCE = new LWClipEntry)
																	{
																	// copy values in
																	LWCE->SetName((char *)ChunkData);
																	LWCE->ClipID = ClipIndex;
																	// add to singly-linked list
																	LWCE->Next = ClipChain;
																	ClipChain = LWCE;
																	} // if
																break;
																} // STIL
															case 'ISEQ':
																{
																break;
																} // ISEQ
															case 'ANIM':
																{
																break;
																} // ANIM
															case 'XREF':
																{
																break;
																} // XREF
															case 'STCC':
																{
																break;
																} // STCC
															} // switch
														ChunkData = NextSubChunk;
														} // while
													} // if
												break;
												} // CLIP
											case 'SURF':
												{
												unsigned char *ChunkData, *NextSubChunk, *SurfName, *SurfSource;
												DynFileBlock DFB; // automatically destroyed/freed when we leave scope
												long StringCount;
												unsigned short SubChunkSize;
												unsigned long SubChunkID, BLOKCount;
												long SurfIDX = -1;

												if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
													{
													// read surface name
													StringCount = 0;
													SurfName = SurfSource = NULL;
													if (ChunkData[0]) SurfName = ChunkData; // grab name
													// [S0]
													//do {ChunkData++; StringCount++; } while (ChunkData[0] != 0);
													do {StringCount++; } while (*(ChunkData++) != 0);
													if (StringCount & 0x01) // odd number of bytes?
														{
														ChunkData++; // eat pad byte
														} // if
													
													// Surface 'source' name (not really supported currently)
													StringCount = 0;
													if (ChunkData[0]) SurfSource = ChunkData; // grab name
													// [S0]
													//do {ChunkData++; StringCount++; } while (ChunkData[0] != 0);
													do {StringCount++; } while (*(ChunkData++) != 0);
													if (StringCount & 0x01) // odd number of bytes?
														{
														ChunkData++; // eat pad byte
														} // if

													// identify which surface index goes with this named surface
													for (int SurfScan = 0; SurfScan < NumTT; SurfScan++)
														{
														SurfIDX = LWTT.FindEntryByName(SurfName);
														if (SurfIDX != -1)
															{
															LWTT.SetSurfID(SurfIDX, NextSurfID++); // flag this one as being a surface
															break; // no more searching if found
															} // if
														} // if

													// process sub-chunks
													if (TextureTable && SurfIDX != -1)
														{ // setup LW defaults in case chunks are missing
														TextureTable[SurfIDX].SpecExponent = 64.0; // default LW "Medium"
														} // if
													BLOKCount = 0;
													while (ChunkData < DFB.GetBlockEnd())
														{
														SubChunkID = MAKE_ID(ChunkData[0], ChunkData[1], ChunkData[2], ChunkData[3]);
														ChunkData += 4;
														memcpy(&SubChunkSize, ChunkData, sizeof(unsigned short));
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(SubChunkSize, &SubChunkSize);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short);
														NextSubChunk = ChunkData + SubChunkSize; // default skipahead handling
														if (TextureTable && SurfIDX != -1)
															{
															switch(SubChunkID)
																{
																case 'COLR':
																	{
																	float FltGun;

																	TextureTable[SurfIDX].UseFPGuns = 1;
																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].FPRed = FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].FPGreen = FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].FPBlue = FltGun;
																	break;
																	} // 
																case 'DIFF':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].Diffuse = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'LUMI':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].Luminosity = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'SPEC':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].Specularity = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'REFL':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].Reflectivity = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'TRAN':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].Transparency = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'TRNL':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	// We don't seem to have an entry for translucency at the moment
																	TextureTable[SurfIDX].Translucency = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'GLOS':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	// convert glossyness as % between 0.0 and 1.0 into exponent
																	// between 4.0 and 1024
																	TextureTable[SurfIDX].SpecExponent = pow(2.0, 10.0 * FltGun + 2.0);

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'SHRP':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	// I don't think we have a diffuse sharpness right now
																	TextureTable[SurfIDX].DiffuseSharpness = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'BUMP':
																	{
																	float FltGun;

																	memcpy(&FltGun, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FltGun, &FltGun);
																	#endif // BYTEORDER_LITTLEENDIAN
																	// No bump intensity yet
																	TextureTable[SurfIDX].BumpIntensity = FltGun;

																	// ignore VX of EnvelopeID, our auto-subchunk-advance code will save us
																	break;
																	} // 
																case 'SIDE':
																	{
																	unsigned short Sided;

																	memcpy(&Sided, ChunkData, sizeof(unsigned short));
																	ChunkData += sizeof(unsigned short);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Sided, &Sided);
																	#endif // BYTEORDER_LITTLEENDIAN
																	if (Sided == 3)
																		{
																		TextureTable[SurfIDX].DoubleSided = 1;
																		} // if
																	else
																		{
																		TextureTable[SurfIDX].DoubleSided = 0;
																		} // else
																	break;
																	} // 
																case 'SMAN':
																	{ // smoothing angle
																	float AngFour;

																	TextureTable[SurfIDX].Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;

																	memcpy(&AngFour, ChunkData, sizeof(float));
																	ChunkData += sizeof(float);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&AngFour, &AngFour);
																	#endif // BYTEORDER_LITTLEENDIAN
																	TextureTable[SurfIDX].SmoothAngle = AngFour * PiUnder180; // LW represents it in radians
																	break;
																	} // 
																case 'VCOL':
																	{ // vertex coloration
																	break;
																	} // 
																case 'RFOP':
																	{ // reflection options
																	break;
																	} // 
																case 'RIMG':
																	{ // reflection image
																	break;
																	} // 
																case 'BLOK':
																	{ // Surface Blocks
																	if (BLOKCount == 0) // only handle first BLOK for now
																		{
																		unsigned short BLOKSize, BLOKIncSize = 0, BLOKSubChunkSize;
																		unsigned long BLOKSubChunkID;
																		char KeepBLOKing = 1, IMAPBLOK = 0;
																		MaterialTextureInfo *MTI = NULL;
																		unsigned char *EndChunkData;
																		
																		// use SubChunkSize here
																		//memcpy(&BLOKSize, ChunkData, sizeof(unsigned short));
																		// skip over BLOK size
																		//ChunkData += sizeof(unsigned short);
																		BLOKSize = SubChunkSize;

																		EndChunkData = &ChunkData[BLOKSize]; // for later safe skipahead

																		// handle heaps of subchunks
																		while (KeepBLOKing && (BLOKIncSize < BLOKSize))
																			{
																			unsigned char *SubChunkData;
																			BLOKSubChunkID = MAKE_ID(ChunkData[0], ChunkData[1], ChunkData[2], ChunkData[3]);
																			ChunkData += sizeof(unsigned long);
																			BLOKIncSize += sizeof(unsigned long);

																			memcpy(&BLOKSubChunkSize, ChunkData, sizeof(unsigned short));
																			#ifdef BYTEORDER_LITTLEENDIAN
																			SimpleEndianFlip16U(BLOKSubChunkSize, &BLOKSubChunkSize);
																			#endif // BYTEORDER_LITTLEENDIAN
																			ChunkData += sizeof(unsigned short);
																			BLOKIncSize += sizeof(unsigned short);

																			SubChunkData = ChunkData;

																			switch(BLOKSubChunkID)
																				{
																				case 'TMAP':
																					{
																					unsigned char *TMAPSubChunkData, *TMAPEndChunk;
																					unsigned long TMAPSubChunkID;
																					unsigned short TMAPSubChunkSize;
																					
																					TMAPEndChunk = &SubChunkData[BLOKSubChunkSize];
																					
																					while (SubChunkData < TMAPEndChunk)
																						{
																						TMAPSubChunkID = MAKE_ID(SubChunkData[0], SubChunkData[1], SubChunkData[2], SubChunkData[3]);
																						//memcpy(&TMAPSubChunkID, SubChunkData, sizeof(unsigned long));
																						SubChunkData += sizeof(unsigned long);

																						memcpy(&TMAPSubChunkSize, SubChunkData, sizeof(unsigned short));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(TMAPSubChunkSize, &TMAPSubChunkSize);
																						#endif // BYTEORDER_LITTLEENDIAN
																						SubChunkData += sizeof(unsigned short);

																						TMAPSubChunkData = SubChunkData;
																						
																						// attempt to parse this chunk
																						switch(TMAPSubChunkID)
																							{
																							case 'CNTR':
																								{ // center
																								if (MTI)
																									{
																									Point3f Vec12;
																									memcpy(&Vec12[0], TMAPSubChunkData, 3 * sizeof(float));
																									// endian flip
																									#ifdef BYTEORDER_LITTLEENDIAN
																									SimpleEndianFlip32F(&Vec12[0], &Vec12[0]);
																									SimpleEndianFlip32F(&Vec12[1], &Vec12[1]);
																									SimpleEndianFlip32F(&Vec12[2], &Vec12[2]);
																									#endif // BYTEORDER_LITTLEENDIAN
																									MTI->Trans[0] = Vec12[0]; MTI->Trans[1] = Vec12[1]; MTI->Trans[2] = Vec12[2];
																									} // if
																								// skip envelope portion
																								break;
																								} // CNTR
																							case 'SIZE':
																								{ // size
																								if (MTI)
																									{
																									Point3f Vec12;
																									memcpy(&Vec12[0], TMAPSubChunkData, 3 * sizeof(float));
																									// endian flip
																									#ifdef BYTEORDER_LITTLEENDIAN
																									SimpleEndianFlip32F(&Vec12[0], &Vec12[0]);
																									SimpleEndianFlip32F(&Vec12[1], &Vec12[1]);
																									SimpleEndianFlip32F(&Vec12[2], &Vec12[2]);
																									#endif // BYTEORDER_LITTLEENDIAN
																									MTI->Scale[0] = Vec12[0]; MTI->Scale[1] = Vec12[1]; MTI->Scale[2] = Vec12[2];
																									} // if
																								// skip envelope portion
																								break;
																								} // CNTR
																							case 'ROTA':
																								{ // rotation
																								if (MTI)
																									{
																									Point3f Vec12;
																									memcpy(&Vec12[0], TMAPSubChunkData, 3 * sizeof(float));
																									// endian flip
																									#ifdef BYTEORDER_LITTLEENDIAN
																									SimpleEndianFlip32F(&Vec12[0], &Vec12[0]);
																									SimpleEndianFlip32F(&Vec12[1], &Vec12[1]);
																									SimpleEndianFlip32F(&Vec12[2], &Vec12[2]);
																									#endif // BYTEORDER_LITTLEENDIAN
																									MTI->Rot[0] = Vec12[0]; MTI->Rot[1] = Vec12[1]; MTI->Rot[2] = Vec12[2];
																									} // if
																								// skip envelope portion
																								break;
																								} // CNTR
																							case 'FALL':
																								{ // falloff
																								break;
																								} // CNTR
																							case 'OREF':
																								{ // reference object -- ignored
																								break;
																								} // CNTR
																							case 'CSYS':
																								{
																								if (MTI)
																									{
																									unsigned short WorldCoordFlag;
																									memcpy(&WorldCoordFlag, TMAPSubChunkData, sizeof(unsigned short));
																									MTI->CoordSpace = ((WorldCoordFlag != 0) ? WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN : WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN);
																									} // if
																								break;
																								} // CNTR

																							} // switch
																						
																						// skip ahead past this chunk even if unparsed
																						SubChunkData += TMAPSubChunkSize;
																						} // while
																				
																					break;
																					} // TMAP
																				case 'IMAP':
																					{
																					unsigned char *IMAPSubChunkData, *IMAPEndChunk;
																					unsigned long IMAPSubChunkID;
																					unsigned short IMAPSubChunkSize;
																					
																					IMAPBLOK = 1;
																					IMAPEndChunk = &SubChunkData[BLOKSubChunkSize];
																					
																					// scan over ordinal -- ignored
																					while (SubChunkData[0] != 0) SubChunkData++;
																					SubChunkData++; // skip ending NULL

																					while (SubChunkData < IMAPEndChunk)
																						{
																						IMAPSubChunkID = MAKE_ID(SubChunkData[0], SubChunkData[1], SubChunkData[2], SubChunkData[3]);
																						//memcpy(&IMAPSubChunkID, SubChunkData, sizeof(unsigned long));
																						SubChunkData += sizeof(unsigned long);

																						memcpy(&IMAPSubChunkSize, SubChunkData, sizeof(unsigned short));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(IMAPSubChunkSize, &IMAPSubChunkSize);
																						#endif // BYTEORDER_LITTLEENDIAN
																						SubChunkData += sizeof(unsigned short);

																						IMAPSubChunkData = SubChunkData;
																						
																						// attempt to parse this chunk
																						switch(IMAPSubChunkID)
																							{
																							case 'CHAN':
																								{
																								int ChannelID;
																								unsigned long ChannelInput;
																								// identify channel

																								ChannelInput = MAKE_ID(IMAPSubChunkData[0], IMAPSubChunkData[1], IMAPSubChunkData[2], IMAPSubChunkData[3]);
																								ChannelID = LWO2ChannelLookup((char *)&ChannelInput);
																								if (ChannelID != -1)
																									{
																									MTI = TextureTable[SurfIDX].GetCreateMaterialTextureInfo(ChannelID);
																									} // if
																								break;
																								} // CHANnel
																							case 'ENAB':
																								{
																								if (MTI)
																									{
																									unsigned short EnFlag;
																									memcpy(&EnFlag, IMAPSubChunkData, sizeof(unsigned short));
																									MTI->Enabled = (EnFlag != 0);
																									} // if
																								break;
																								} // ENABle
																							case 'OPAC':
																								{
																								// Negative
																								if (MTI)
																									{
																									float Opac;
																									// skip Type field that we don't understand
																									IMAPSubChunkData += sizeof(unsigned short);
																									// read float opacity
																									memcpy(&Opac, IMAPSubChunkData, sizeof(float));
																									#ifdef BYTEORDER_LITTLEENDIAN
																									SimpleEndianFlip32F(&Opac, &Opac);
																									#endif // BYTEORDER_LITTLEENDIAN
																									MTI->Opacity = Opac * 100.0f;
																									} // if
																								break;
																								} // OPACity
																							case 'AXIS':
																								{
																								// ignored, axis of displacement
																								break;
																								} // AXIS
																							case 'NEGA':
																								{
																								// Negative
																								if (MTI)
																									{
																									unsigned short NegFlag;
																									memcpy(&NegFlag, IMAPSubChunkData, sizeof(unsigned short));
																									MTI->Negative = (NegFlag != 0);
																									} // if
																								break;
																								} // NEGA
																							} // switch
																						
																						// skip ahead past this chunk even if unparsed
																						SubChunkData += IMAPSubChunkSize;
																						} // while
																				
																					break;
																					} // ImageMAP
																				case 'PROJ':
																					{
																					// Projection type
																					if (MTI)
																						{
																						unsigned short ProjType;
																						memcpy(&ProjType, SubChunkData, sizeof(unsigned short));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(ProjType, &ProjType);
																						#endif // BYTEORDER_LITTLEENDIAN
																						switch(ProjType)
																							{
																							case 0: MTI->TexType = WCS_TEXTURE_TYPE_PLANARIMAGE; break; // planar
																							case 1: MTI->TexType = WCS_TEXTURE_TYPE_CYLINDRICALIMAGE; break; // cyl
																							case 2: MTI->TexType = WCS_TEXTURE_TYPE_SPHERICALIMAGE; break; // sphere
																							//case 3: MTI->TexType = WCS_TEXTURE_TYPE_CUBICIMAGE; break; // cubic
																							case 4: MTI->TexType = WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE; break; // front proj
																							case 5: MTI->TexType = WCS_TEXTURE_TYPE_UVW; break; // UV
																							} // ProjType
																						} // if
																					break;
																					} // PROJ
																				case 'AXIS':
																					{ // not supported yet
																					if (MTI)
																						{
																						unsigned short AxisType;
																						memcpy(&AxisType, SubChunkData, sizeof(unsigned short));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(AxisType, &AxisType);
																						#endif // BYTEORDER_LITTLEENDIAN
																						switch(AxisType)
																							{
																							case 0: MTI->AxisType = 0; break; // X
																							case 1: MTI->AxisType = 1; break; // Y
																							case 2: MTI->AxisType = 2; break; // Z
																							} // AxisType
																						} // if
																					break;
																					} // AXIS
																				case 'IMAG':
																					{ // IMAGe map index
																					// parse a VX here
																					unsigned long Dest4P;
																					unsigned short Dest2P;
																					if (SubChunkData[0] == 0xFF)
																						{ // long notation
																						memcpy(&Dest4P, SubChunkData, 4);
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip32U(Dest4P, &Dest4P);
																						#endif // BYTEORDER_LITTLEENDIAN
																						Dest4P &= 0x00ffffff; // mask off top 0xff signal byte
																						SubChunkData += sizeof(unsigned long); // advance by 4
																						} // if
																					else
																						{ // short notation
																						memcpy(&Dest2P, SubChunkData, 2);
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(Dest2P, &Dest2P);
																						#endif // BYTEORDER_LITTLEENDIAN
																						Dest4P = Dest2P; // upconvert for usage/storage
																						SubChunkData += sizeof(unsigned short); // advance by 2
																						} // else
																					// Dest4 is now the image Index, search for a matching ClipEntry
																					if (MTI)
																						{
																						LWClipEntry *FoundClip = NULL;

																						if (FoundClip = ClipChain->FindInChain(Dest4P))
																							{
																							MTI->SetNameTrimPath(FoundClip->ClipPath);
																							} // if
																						} // if
																					break;
																					} // IMAGe ID
																				case 'WRAP':
																					{ // wrap mode
																					// 0 no wrap
																					// 1 (default) tile
																					// 2 tile with flip
																					// 3 stretch edge pixel color
																					if (MTI)
																						{
																						unsigned short WrapX, WrapY;
																						memcpy(&WrapX, SubChunkData, sizeof(unsigned short));
																						SubChunkData += sizeof(unsigned short);
																						memcpy(&WrapY, SubChunkData, sizeof(unsigned short));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip16U(WrapX, &WrapX);
																						SimpleEndianFlip16U(WrapY, &WrapY);
																						#endif // BYTEORDER_LITTLEENDIAN
																						switch(WrapX)
																							{
																							case 0: MTI->TileX = 0; MTI->FlipX = 0; break; // reset
																							case 1: MTI->TileX = 1; MTI->FlipX = 0; break; // repeat
																							case 2: MTI->TileX = 1; MTI->FlipX = 1; break; // mirror
																							case 3: MTI->TileX = 0; MTI->FlipX = 0; break; // edge
																							} // Wrap
																						switch(WrapY)
																							{
																							case 0: MTI->TileY = 0; MTI->FlipY = 0; break; // reset
																							case 1: MTI->TileY = 1; MTI->FlipY = 0; break; // repeat
																							case 2: MTI->TileY = 1; MTI->FlipY = 1; break; // mirror
																							case 3: MTI->TileY = 0; MTI->FlipY = 0; break; // edge
																							} // Wrap
																						} // if
																					break;
																					} // WRAP modes
																				case 'WRPH':
																				case 'WRPW':
																					{ // wrap width/height how many times
																					break;
																					} // WRPH, WRPW
																				case 'VMAP':
																					{ // UVMap name
																					// copy [S0] to VMAP name
																					if (MTI)
																						{
																						MTI->SetUVName((char *)SubChunkData);
																						} // if
																					break;
																					} // VMAP
																				case 'AAST':
																					{ // AA strength
																					break;
																					} // AAST
																				case 'PIXB':
																					{ // pixel blending
																					break;
																					} // PIXB
																				case 'STCK':
																					{ // Front projection map 'sticky' moment
																					break;
																					} // STCK
																				case 'TAMP':
																					{ // Texture amplitude for bumpmapping
																					if (MTI)
																						{
																						float BAmp;
																						// read float amplitude
																						memcpy(&BAmp, SubChunkData, sizeof(float));
																						#ifdef BYTEORDER_LITTLEENDIAN
																						SimpleEndianFlip32F(&BAmp, &BAmp);
																						#endif // BYTEORDER_LITTLEENDIAN
																						MTI->BumpAmp = BAmp;
																						} // if
																					break;
																					} // TAMP
																				case 'VALU':
																					{ // procedural channel upper value
																					break;
																					} // VALU
																				case 'FUNC':
																					{ // procedural function name, data
																					break;
																					} // FUNC
																				case 'PROC':
																					{
																					KeepBLOKing = 0; // not implemented now
																					break;
																					} // PROCedural texture
																				case 'GRAD':
																					{
																					KeepBLOKing = 0; // not implemented now
																					break;
																					} // GRADient
																				case 'SHDR':
																					{
																					KeepBLOKing = 0; // not implemented now
																					break;
																					} // SHaDeR
																				} // switch
																			ChunkData += BLOKSubChunkSize; // skip CheckData ahead over any subchunk
																			BLOKIncSize += BLOKSubChunkSize;
																			} // while
																		} // if
																	BLOKCount++;
																	break;
																	} // 
																// no need for default case, skipahead already handled
																} // switch
															} // if
														ChunkData = NextSubChunk;
														} // while
													} // if
												break;
												} // SURF
											case 'TAGS':
												{ // 
												unsigned char *ChunkData;
												long StringCount = 0, EntryNum = 0, MaxStringLen, StringLen, Pass;
												if (ChunkData = TDFB.ReadDynBlock(fInput, ChunkSize)) // this block will persist until we leave function scope
													{
													for (Pass = 0; Pass < 2; Pass++)
														{
														MaxStringLen = 0;
														if (Pass == 1)
															{
															ChunkData = TDFB.GetBlock(); // reset to beginning of block for second pass
															LWTT.SetEntries(StringCount);
															} // if
														while (ChunkData < TDFB.GetBlockEnd())
															{
															StringLen = 0;
															if (Pass == 0)
																{
																// [S0]
																/* do
																	{
																	ChunkData++;
																	StringLen++;
																	} while (ChunkData[0] != 0); */
																do {StringCount++; } while (*(ChunkData++) != 0);
																if (StringLen & 0x01) // odd number of bytes?
																	{
																	ChunkData++; // eat pad byte
																	} // if
																if (StringLen > MaxStringLen) MaxStringLen = StringLen;
																StringCount++;
																} // if
															else
																{ // Pass == 1
																if (ChunkData[0] != 0)
																	{
																	LWTT.SetEntryName(EntryNum++, ChunkData);
																	} // if
																// [S0]
																/*do
																	{
																	// put into table here
																	ChunkData++;
																	StringLen++;
																	} while (ChunkData[0] != 0); */
																do {StringCount++; } while (*(ChunkData++) != 0);
																if (StringLen & 0x01) // odd number of bytes?
																	{
																	ChunkData++; // eat pad byte
																	} // if
																} // else
															} // while
														} // for
													if (TextureTable = new MaterialAttributes[EntryNum]) // all tags may be superset of surface/material definition
														{
														NumTT = EntryNum;
														} // if
													} // if
												break;
												} // TAGS
											case 'VMAP':
												{ // UV coords or RGB values
												if (LWO2)
													{
													unsigned char *ChunkData;
													char *VMAPName = NULL;
													DynFileBlock DFB; // automatically destroyed/freed when we leave scope
													unsigned long Dest4;
													unsigned short Dest2;
													float DestFloat;
													unsigned short Dimension;
													long StringCount, VertexCount = 0;
													LWVertMapEntry *NewVME = NULL;

													if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
														{
														if (!strcmp((char *)ChunkData, "TXUV"))
															{
															FileNumUVMaps++;
															ChunkData += 4; // advance over ID
															memcpy(&Dest2, ChunkData, 2);
															#ifdef BYTEORDER_LITTLEENDIAN
															SimpleEndianFlip16U(Dest2, &Dest2);
															#endif // BYTEORDER_LITTLEENDIAN
															ChunkData += sizeof(unsigned short); // advance by 2
															Dimension = Dest2;
															if (Dimension == 2)
																{
																// read name
																StringCount = 0;
																VMAPName = (char *)ChunkData; // just a pointer, not a copy!
																// [S0]
																do {StringCount++; } while (*(ChunkData++) != 0);
																if (StringCount & 0x01) // odd number of bytes?
																	{
																	ChunkData++; // eat pad byte
																	} // if

																while (ChunkData < DFB.GetBlockEnd())
																	{
																	if (ChunkData[0] == 0xFF)
																		{ // long notation
																		memcpy(&Dest4, ChunkData, 4);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip32U(Dest4, &Dest4);
																		#endif // BYTEORDER_LITTLEENDIAN
																		Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																		ChunkData += sizeof(unsigned long); // advance by 4
																		} // if
																	else
																		{ // short notation
																		memcpy(&Dest2, ChunkData, 2);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip16U(Dest2, &Dest2);
																		#endif // BYTEORDER_LITTLEENDIAN
																		Dest4 = Dest2; // upconvert for usage/storage
																		ChunkData += sizeof(unsigned short); // advance by 2
																		} // else
																	// read Values (discarded in first pass)
																	for (int DimCount = 0; DimCount < Dimension; DimCount++)
																		{
																		memcpy(&DestFloat, ChunkData, 4);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip32F(&DestFloat, &DestFloat);
																		#endif // BYTEORDER_LITTLEENDIAN
																		ChunkData += sizeof(float); // advance by 4
																		} // for
																	VertexCount++;
																	} // while
																} // if
															} // if
														if (!strncmp((char *)ChunkData, "RGB", 3)) // RGB or RGBA
															{
															FileNumRGBMaps++;
															ChunkData += 4; // advance over ID
															memcpy(&Dest2, ChunkData, 2);
															#ifdef BYTEORDER_LITTLEENDIAN
															SimpleEndianFlip16U(Dest2, &Dest2);
															#endif // BYTEORDER_LITTLEENDIAN
															ChunkData += sizeof(unsigned short); // advance by 2
															Dimension = Dest2;
															if ((Dimension == 3) || (Dimension == 4)) // 3:RGB 4:RGBA
																{
																// read name
																StringCount = 0;
																VMAPName = (char *)ChunkData; // just a pointer, not a copy!
																// [S0]
																do {StringCount++; } while (*(ChunkData++) != 0);
																if (StringCount & 0x01) // odd number of bytes?
																	{
																	ChunkData++; // eat pad byte
																	} // if

																while (ChunkData < DFB.GetBlockEnd())
																	{
																	if (ChunkData[0] == 0xFF)
																		{ // long notation
																		memcpy(&Dest4, ChunkData, 4);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip32U(Dest4, &Dest4);
																		#endif // BYTEORDER_LITTLEENDIAN
																		Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																		ChunkData += sizeof(unsigned long); // advance by 4
																		} // if
																	else
																		{ // short notation
																		memcpy(&Dest2, ChunkData, 2);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip16U(Dest2, &Dest2);
																		#endif // BYTEORDER_LITTLEENDIAN
																		Dest4 = Dest2; // upconvert for usage/storage
																		ChunkData += sizeof(unsigned short); // advance by 2
																		} // else
																	// read Values (discarded in first pass)
																	for (int DimCount = 0; DimCount < Dimension; DimCount++)
																		{
																		memcpy(&DestFloat, ChunkData, 4);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip32F(&DestFloat, &DestFloat);
																		#endif // BYTEORDER_LITTLEENDIAN
																		ChunkData += sizeof(float); // advance by 4
																		} // for
																	VertexCount++;
																	} // while
																} // if
															} // if
														} // if
													} // if
												break;
												} // VMAP
											default:
												{ // skip all other chunk types in first pass
												fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
												break;
												} // default
											} // switch chunktag ID
										} // if chunksize > 0
									// Don't fail out, just move on to the next chunk. Blender 2.44 is known to write zero-sized empty ICON chunks
									// http://web.archive.org/web/20010423140254/http://members.home.net/erniew2/lwsdk/docs/filefmts/lwo2.html#H_file
									// that were tripping us up.
									//else
									//	Success = 0;
									} // if read chunk size
								else
									Success = 0;
								} // while reading new chunktag
							// return file pointer to beginning of chunks for second pass
							fseek(fInput, LWO2PassOneMarker, SEEK_SET);
							} // if
						if (FileNumUVMaps)
							{
							if (! UVWTable && AllocUVWMaps(FileNumUVMaps))
								{
								NumUVWMaps = FileNumUVMaps;
								} // if
							FileNumUVMaps = 0; // re-init counter for second pass
							} // if
						if (FileNumRGBMaps)
							{
							if (! CPVTable && AllocCPVMaps(FileNumRGBMaps))
								{
								NumCPVMaps = FileNumRGBMaps;
								} // if
							FileNumRGBMaps = 0; // re-init counter for second pass
							} // if
						POLSOneShot = 1;
						PNTSOneShot = 1;
						while (Success && fread(ChunkTag, 4, 1, fInput) == 1)
							{
							if (fread(&ChunkSize, 4, 1, fInput) == 1)
								{
								#ifdef BYTEORDER_LITTLEENDIAN
								SimpleEndianFlip32S(ChunkSize, &ChunkSize);
								#endif // BYTEORDER_LITTLEENDIAN
								if (ChunkSize > 0)
									{
									ChunkTagID = MAKE_ID(ChunkTag[0], ChunkTag[1], ChunkTag[2], ChunkTag[3]);
									switch (ChunkTagID)
										{
										case 'PNTS':
											{
											if (LoadObj)
												{
												if (PNTSOneShot)
													{
													PNTSOneShot = 0; // prevent trying to read multiple PNTS chunks
													NumVertices = ChunkSize / 12;
													if (Vertices = new Vertex3D[NumVertices])
														{
														for (Count = 0; Count < NumVertices; Count ++)
															{
															if (fread(&FloatVert[0], 12, 1, fInput) == 1)
																{
																for (Vtx = 0; Vtx < 3; Vtx ++)
																	{
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&FloatVert[Vtx], &FloatVert[Vtx]);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Vertices[Count].xyz[Vtx] = (double)FloatVert[Vtx] + (double)LayerPivot[Vtx];
																	// sanity range check against unnormalized very small values (e-41)
																	if (fabs(Vertices[Count].xyz[Vtx]) < FLT_MIN)
																		{
																		Vertices[Count].xyz[Vtx] = 0.0;
																		} // if
																	} // for
																} // if
															else
																{
																//delete [] Vertices;
																//Vertices = NULL;
																FreeVertices();
																Success = 0;
																break;
																} // else
															} // for
														} // if
													} // if
												else
													{
													// set error
													Success = 0;
													// warn user
													UserMessageOK("LWO2 Loader", "Multilayer objects not currently supported.");
													// skip chunk
													if (fseek(fInput, ChunkSize, SEEK_CUR))
														Success = 0;
													} // else
												} // if
											else
												{
												if (fseek(fInput, ChunkSize, SEEK_CUR))
													Success = 0;
												else
													{
													NumVertices = ChunkSize / 12;
													} // else
												} // else
											break;
											} // PNTS
										case 'SRFS':
											{
											if (ChunkSize > 0)
												{
												if (NamePool = (char *)AppMem_Alloc(ChunkSize, 0))
													{
													NamePoolSize = ChunkSize;
													if (fread(NamePool, ChunkSize, 1, fInput) == 1)
														{
														NamePtr = NamePool;
														do 
															{
															NumMaterials ++;
															} while (NamePtr = GetNextName(NamePtr, NamePool + ChunkSize));
														if (NameTable = new ObjectMaterialEntry[NumMaterials])
															{
															NamePtr = NamePool;
															Count = 0;
															do 
																{
																NameTable[Count].SetName(NamePtr);
																Count ++;
																} while (NamePtr = GetNextName(NamePtr, NamePool + ChunkSize));
															TextureTable = new MaterialAttributes[NumMaterials];
															} // if
														} // if
													else
														{
														Success = 0;
														} // else
													AppMem_Free(NamePool, NamePoolSize);
													NamePool = NULL;
													NamePoolSize = 0;
													} // if
												} // if
											break;
											} // SRFS
										case 'POLS':
											{
											if (LWO2)
												{
												if (POLSOneShot)
													{
													int Pass = 0;
													NumPolys = 0; // calculated in pass 0, used in pass 1
													DynFileBlock DFB; // automatically destroyed/freed when we leave scope
													unsigned char *ChunkData;
													// read whole chunk to memory
													POLSOneShot = 0; // prevent reading multiple chunks
													ChunkData = DFB.ReadDynBlock(fInput, ChunkSize);
													if (!strcmp((char *)ChunkData, "FACE")) // only read regular poly faces (could also be CURV, PTCH, MBAL, BONE)
														{
														ChunkData += 4; // skip over ID
														// parse entire chunk
														for (Pass = 0; Pass < 2; Pass++)
															{
															if (Pass == 1)
																{ // data is already read, just grab fresh copy of block pointer
																if ((NumPolys != 0) && (!Polygons))
																	{
																	Polygons = new Polygon3D[NumPolys];
																	} // if
																ChunkData = DFB.GetBlock();
																ChunkData += 4; // skip TYPE id in second pass
																Count = 0;
																} // if
															if (ChunkData)
																{
																unsigned long Dest4;
																unsigned short Dest2;
																while (ChunkData < DFB.GetBlockEnd())
																	{
																	if (Pass == 0)
																		{
																		NumPolys++; // just count polygons on first pass
																		} // if
																	// read number of vertices
																	memcpy(&PolyVertices, ChunkData, sizeof(short));
																	ChunkData += sizeof(short);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16S(PolyVertices, &PolyVertices);
																	#endif // BYTEORDER_LITTLEENDIAN
																	if (Pass == 1)
																		{
																		if (PolyVertices > WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY)
																			{
																			UserMessageOK(Name, "Too many vertices per polygon!\nLimit of " WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY_TEXT " has been exceeded.\nThis object cannot be rendered in "APP_TLA".");
																			Success = 0;
																			break;
																			} // if
																		if ((Polygons[Count].VertRef = (long *)AppMem_Alloc(PolyVertices * sizeof(long), 0)) == NULL)
																			{
																			Success = 0;
																			break;
																			} // if
																		Polygons[Count].NumVerts = PolyVertices;
																		Polygons[Count].Material = 0;
																		} // if
																	for (Vtx = 0; Vtx < PolyVertices; Vtx ++)
																		{
																		if (ChunkData[0] == 0xFF)
																			{ // long notation
																			memcpy(&Dest4, ChunkData, 4);
																			#ifdef BYTEORDER_LITTLEENDIAN
																			SimpleEndianFlip32U(Dest4, &Dest4);
																			#endif // BYTEORDER_LITTLEENDIAN
																			Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																			ChunkData += sizeof(unsigned long); // advance by 4
																			} // if
																		else
																			{ // short notation
																			memcpy(&Dest2, ChunkData, 2);
																			#ifdef BYTEORDER_LITTLEENDIAN
																			SimpleEndianFlip16U(Dest2, &Dest2);
																			#endif // BYTEORDER_LITTLEENDIAN
																			Dest4 = Dest2; // upconvert for usage/storage
																			ChunkData += sizeof(unsigned short); // advance by 2
																			} // else
																		if (Pass == 1)
																			{
																			Polygons[Count].VertRef[Vtx] = Dest4;
																			if (Vertices)
																				Vertices[Dest4].NumPolys ++;
																			} // if
																		} // for
																	Count++;
																	} // while
																} // if
															} // for


														if (Vertices)
															{
															for (Count = 0; Count < NumVertices; Count ++)
																{
																if (Vertices[Count].NumPolys)
																	{
																	if ((Vertices[Count].PolyRef = (long *)AppMem_Alloc(Vertices[Count].NumPolys * sizeof(long), 0)) == NULL)
																		{
																		Success = 0;
																		break;
																		} // if
																	} // if
																Vertices[Count].NumPolys = 0;
																} // for
															if (Success)
																{
	/*
																long Ct;

																for (Count = 0; Count < NumPolys; Count ++)
																	{
																	for (Vtx = 0; Vtx < Polygons[Count].NumVerts; Vtx ++)
																		{
																		// this is the vertex number referred to by the polygon, it contains the vertex offset for this mesh
																		Ct = Polygons[Count].VertRef[Vtx];
																		// gh added sanity check 3/28/02
																		if (VertRef[Ct] >= 0)	// VertRef is only the size of this mesh's vertex count
																			{
																			// gh changed 3/26/02
																			//Vertices[Ct].PolyRef[VertRef[Ct]--] = Poly + ObjBasePolyNum;
																			// gh changed again 3/28/02 - subtracted vertex base offset to index
																			Vertices[Ct].PolyRef[VertRef[Ct]--] = Count;
																			} // if
																		} // for
																	} // for
	*/

																for (Count = 0; Count < NumPolys; Count ++)
																	{
																	for (Vtx = 0; Vtx < Polygons[Count].NumVerts; Vtx ++)
																		{
																		if (Vertices[Polygons[Count].VertRef[Vtx]].PolyRef)
																			{
																			Vertices[Polygons[Count].VertRef[Vtx]].PolyRef[Vertices[Polygons[Count].VertRef[Vtx]].NumPolys] = Count;
																			Vertices[Polygons[Count].VertRef[Vtx]].NumPolys ++;
																			} // if
																		} // for
																	} // for


																} // if
															} // if


														} // if
													else
														{ // CURV, PTCH, MBAL, BONE
														// should not need to do anything, as we've consumed the entire chunk
														} // else
													} // if POLSOneShot
												else
													{
													// set error
													Success = 0;
													// warn user
													UserMessageOK("LWO2 Loader", "Multilayer objects or objects with Spline Curves, Subdivision Patches, Metaballs and Bones are not currently supported.");
													// skip chunk
													if (fseek(fInput, ChunkSize, SEEK_CUR))
														Success = 0;
													} // else
												} // if
											else
												{ // LWOB
												NumPolys = ChunkSize / 10;	// this may be high due to subpolygons
												// TempPoly is allocated larger than need be in case we ever decide to support
												// more than 6 vertices per polygon. 200 is the largest number that LightWave 3.0 supports.
												// LW6.x documentation states this can now be as high as 1023
												// this number is defined in WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY at the top of the file
												if (TempPoly = (unsigned short *)AppMem_Alloc(WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY * sizeof(unsigned short), 0))
													{
													if (Polygons = new Polygon3D[NumPolys])
														{
														for (Count = 0; Count < NumPolys && SizeRead < ChunkSize; Count ++)
															{
															// read number of vertices
															if (fread(&PolyVertices, sizeof(short), 1, fInput) == 1)
																{
																SizeRead += sizeof(short);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip16S(PolyVertices, &PolyVertices);
																#endif // BYTEORDER_LITTLEENDIAN
																if (PolyVertices > WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY)
																	{
																	UserMessageOK(Name, "Too many vertices per polygon!\nLimit of " WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY_TEXT " has been exceeded.\nThis object cannot be rendered in "APP_TLA".");
																	Success = 0;
																	break;
																	} // if
																if ((Polygons[Count].VertRef = (long *)AppMem_Alloc(PolyVertices * sizeof(long), 0)) == NULL)
																	{
																	Success = 0;
																	break;
																	} // if
																Polygons[Count].NumVerts = PolyVertices;
																} // if
															else
																{
																Success = 0;
																break;
																} // else
															// read vertex indices
															if (fread((char *)TempPoly, sizeof(unsigned short) * PolyVertices, 1, fInput) == 1)
																{
																SizeRead += (sizeof(unsigned short) * PolyVertices);
																for (Vtx = 0; Vtx < PolyVertices; Vtx ++)
																	{
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(TempPoly[Vtx], &TempPoly[Vtx]);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Polygons[Count].VertRef[Vtx] = TempPoly[Vtx];
																	if (Vertices)
																		Vertices[TempPoly[Vtx]].NumPolys ++;
																	} // for
																} // if
															else
																{
																Success = 0;
																break;
																} // else
															// read material - indexed to 1 with neg number indicating subpolygons
															if (fread((char *)&PolyMaterial, sizeof(short), 1, fInput) == 1)
																{
																SizeRead += sizeof(short);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip16S(PolyMaterial, &PolyMaterial);
																#endif // BYTEORDER_LITTLEENDIAN
																if (PolyMaterial < 0)
																	{
																	if (fread(&SubPolys, sizeof(short), 1, fInput) == 1)
																		{
																		SizeRead += sizeof(short);
																		#ifdef BYTEORDER_LITTLEENDIAN
																		SimpleEndianFlip16S(SubPolys, &SubPolys);
																		#endif // BYTEORDER_LITTLEENDIAN
																		} // if
																	else
																		{
																		Success = 0;
																		break;
																		} // else
																	} // if negative material = subpolygons
																Polygons[Count].Material =  abs(PolyMaterial) - 1;
																} // if
															else
																{
																Success = 0;
																break;
																} // else
															} // for
														} // if
													if (SizeRead < ChunkSize)
														{
														if (fseek(fInput, ChunkSize - SizeRead, SEEK_CUR))
															Success = 0;
														} // if didn't read it all
													if (! Success)
														{
														FreePolygons();
														break;
														} // if
													else
														{
														NumPolys = Count;	// this may be high due to subpolygons
														if (Vertices)
															{
															for (Count = 0; Count < NumVertices; Count ++)
																{
																if (Vertices[Count].NumPolys)
																	{
																	if ((Vertices[Count].PolyRef = (long *)AppMem_Alloc(Vertices[Count].NumPolys * sizeof(long), 0)) == NULL)
																		{
																		Success = 0;
																		break;
																		} // if
																	} // if
																Vertices[Count].NumPolys = 0;
																} // for
															if (Success)
																{
																for (Count = 0; Count < NumPolys; Count ++)
																	{
																	for (Vtx = 0; Vtx < Polygons[Count].NumVerts; Vtx ++)
																		{
																		if (Vertices[Polygons[Count].VertRef[Vtx]].PolyRef)
																			{
																			Vertices[Polygons[Count].VertRef[Vtx]].PolyRef[Vertices[Polygons[Count].VertRef[Vtx]].NumPolys] = Count;
																			Vertices[Polygons[Count].VertRef[Vtx]].NumPolys ++;
																			} // if
																		} // for
																	} // for
																} // if
															} // if
														} // else
													AppMem_Free(TempPoly, WCS_EFFECTOBJECT3D_LWO_MAXPOINTSPERPOLY * sizeof(unsigned short));
													} // if TempPoly allocated
												} // LWOB
											break;
											} // POLS
										case 'SURF':
											{
											if (LWO2)
												{ // skip chunk, we handled it in first pass
												fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
												} // if
											else
												{
												if (ChunkSize > NamePoolSize)
													{
													if (NamePool)
														AppMem_Free(NamePool, NamePoolSize);
													if (NamePool = (char *)AppMem_Alloc(ChunkSize, 0))
														NamePoolSize = ChunkSize;
													} // if
												if (NamePool && TextureTable && NameTable)
													{
													if (fread(NamePool, ChunkSize, 1, fInput) == 1)
														{
														ProcessLWMaterial(NamePool, ChunkSize, NameTable, TextureTable);
														} // if
													else
														Success = 0;
													} // if
												else if (fseek(fInput, ChunkSize, SEEK_CUR))
													Success = 0;
												} // else LWOB
											break;
											} // SURF
										case 'LAYR':
											{ // Layers
											if (LWO2)
												{
												unsigned short LayerID, LayerFlags, ParentID;
												unsigned char *ChunkData;
												long StringCount;
												DynFileBlock DFB; // automatically destroyed/freed when we leave scope

												if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
													{
													memcpy(&LayerID, ChunkData, sizeof(unsigned short)); // flip below
													ChunkData += sizeof(unsigned short);  // LayerID
													memcpy(&LayerFlags, ChunkData, sizeof(unsigned short)); // flip below
													ChunkData += sizeof(unsigned short); // LayerFlags
													// parse layer pivot point (all we care about, really)
													memcpy(&LayerPivot[0], ChunkData, sizeof(float));
													ChunkData += sizeof(float);
													memcpy(&LayerPivot[1], ChunkData, sizeof(float));
													ChunkData += sizeof(float);
													memcpy(&LayerPivot[2], ChunkData, sizeof(float));
													ChunkData += sizeof(float);
													#ifdef BYTEORDER_LITTLEENDIAN
													SimpleEndianFlip16U(LayerID, &LayerID);
													SimpleEndianFlip16U(LayerFlags, &LayerFlags);
													SimpleEndianFlip32F(&LayerPivot[0], &LayerPivot[0]);
													SimpleEndianFlip32F(&LayerPivot[1], &LayerPivot[1]);
													SimpleEndianFlip32F(&LayerPivot[2], &LayerPivot[2]);
													#endif // BYTEORDER_LITTLEENDIAN
													// name
													StringCount = 0;
													// [S0]
													//do {ChunkData++; StringCount++; } while (ChunkData[0] != 0);
													do {StringCount++; } while (*(ChunkData++) != 0);
													if (StringCount & 0x01) // odd number of bytes?
														{
														ChunkData++; // eat pad byte
														} // if

													if (ChunkData < DFB.GetBlockEnd()) // is option parent value present?
														{
														memcpy(&ParentID, ChunkData, sizeof(unsigned short));
														ChunkData += sizeof(unsigned short); // Layer Parent ID
														} // if
													} // if
												} // if
											break;
											} // LAYR
										case 'PTAG':
											{ // 
											if (LWO2)
												{
												unsigned char *ChunkData;
												DynFileBlock DFB; // automatically destroyed/freed when we leave scope
												unsigned long Dest4;
												unsigned short Dest2, TagID;

												if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
													{
													if (!strcmp((char *)ChunkData, "SURF"))
														{
														ChunkData += 4; // advance over ID
														while (ChunkData < DFB.GetBlockEnd())
															{
															if (ChunkData[0] == 0xFF)
																{ // long notation
																memcpy(&Dest4, ChunkData, 4);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip32U(Dest4, &Dest4);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																ChunkData += sizeof(unsigned long); // advance by 4
																} // if
															else
																{ // short notation
																memcpy(&Dest2, ChunkData, 2);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip16U(Dest2, &Dest2);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 = Dest2; // upconvert for usage/storage
																ChunkData += sizeof(unsigned short); // advance by 2
																} // else
															// read TagID
															memcpy(&TagID, ChunkData, 2);
															#ifdef BYTEORDER_LITTLEENDIAN
															SimpleEndianFlip16U(TagID, &TagID);
															#endif // BYTEORDER_LITTLEENDIAN
															ChunkData += sizeof(unsigned short); // advance by 2
															if (Polygons)
																{
																long NewSurfID = 0;
																if (TagID < NumTT)
																	{
																	NewSurfID = LWTT.GetSurfID(TagID);
																	} // if
																else
																	{
																	NewSurfID = 0; // error!
																	} // else
																if (NewSurfID == -1)
																	{
																	NewSurfID = 0;
																	} // if
																Polygons[Dest4].Material = NewSurfID;
																} // if
															} // while
														} // if
													else if (!strcmp((char *)ChunkData, "PART")) // ignored for now
														{
														while (ChunkData < DFB.GetBlockEnd())
															{
															if (ChunkData[0] == 0xFF)
																{ // long notation
																memcpy(&Dest4, ChunkData, 4);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip32U(Dest4, &Dest4);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																ChunkData += sizeof(unsigned long); // advance by 4
																} // if
															else
																{ // short notation
																memcpy(&Dest2, ChunkData, 2);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip16U(Dest2, &Dest2);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 = Dest2; // upconvert for usage/storage
																ChunkData += sizeof(unsigned short); // advance by 2
																} // else
															// read TagID
															memcpy(&TagID, ChunkData, 2);
															#ifdef BYTEORDER_LITTLEENDIAN
															SimpleEndianFlip16U(TagID, &TagID);
															#endif // BYTEORDER_LITTLEENDIAN
															ChunkData += sizeof(unsigned short); // advance by 2

															// do nothing with data, currently
															} // while
														} // else if
													else if (!strcmp((char *)ChunkData, "SMGP")) // ignored for now
														{
														while (ChunkData < DFB.GetBlockEnd())
															{
															if (ChunkData[0] == 0xFF)
																{ // long notation
																memcpy(&Dest4, ChunkData, 4);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip32U(Dest4, &Dest4);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																ChunkData += sizeof(unsigned long); // advance by 4
																} // if
															else
																{ // short notation
																memcpy(&Dest2, ChunkData, 2);
																#ifdef BYTEORDER_LITTLEENDIAN
																SimpleEndianFlip16U(Dest2, &Dest2);
																#endif // BYTEORDER_LITTLEENDIAN
																Dest4 = Dest2; // upconvert for usage/storage
																ChunkData += sizeof(unsigned short); // advance by 2
																} // else
															// read TagID
															memcpy(&TagID, ChunkData, 2);
															#ifdef BYTEORDER_LITTLEENDIAN
															SimpleEndianFlip16U(TagID, &TagID);
															#endif // BYTEORDER_LITTLEENDIAN
															ChunkData += sizeof(unsigned short); // advance by 2

															// do nothing with data, currently
															} // while
														} // else if
													} // if
												} // if
											break;
											} // PTAG
										case 'VMAD':
											{
											if (0) // if (LWO2)
												{
												unsigned char *ChunkData;
												char *VMAPName = NULL;
												DynFileBlock DFB; // automatically destroyed/freed when we leave scope
												unsigned long Dest4, Dest4P;
												unsigned short Dest2, Dest2P;
												float DestFloat;
												unsigned short Dimension;
												long StringCount;

												if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
													{
													if (!strcmp((char *)ChunkData, "TXUV"))
														{
														ChunkData += 4; // advance over ID
														memcpy(&Dest2, ChunkData, 2);
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(Dest2, &Dest2);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short); // advance by 2
														Dimension = Dest2;
														if (Dimension == 2)
															{
															// read name
															StringCount = 0;
															VMAPName = (char *)ChunkData; // just a pointer, not a copy!
															// [S0]
															do {StringCount++; } while (*(ChunkData++) != 0);
															if (StringCount & 0x01) // odd number of bytes?
																{
																ChunkData++; // eat pad byte
																} // if
															VertMap = NULL; // until we search for it, below
															if (UVWTable)
																{
																// while LWO2 doesn't specifically require a VMAP exist for all VMADs, we do
																// Here, we search for our corresponding VMAP entry by name
																int VertMapSearch;
																for (VertMapSearch = 0; VertMapSearch < NumUVWMaps; VertMapSearch++)
																	{
																	if (!(UVWTable[VertMapSearch].CompareName(VMAPName)))
																		{
																		VertMap = &UVWTable[VertMapSearch];
																		break;
																		} // if
																	} // for
																} // if

															while (ChunkData < DFB.GetBlockEnd())
																{
																// read vertex VX
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4, &Dest4);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2, &Dest2);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 = Dest2; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Poly VX
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4P, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4P, &Dest4P);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4P &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2P, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2P, &Dest2P);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4P = Dest2P; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Values
																for (int DimCount = 0; DimCount < Dimension; DimCount++)
																	{
																	memcpy(&DestFloat, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&DestFloat, &DestFloat);
																	#endif // BYTEORDER_LITTLEENDIAN
																	ChunkData += sizeof(float); // advance by 4
																	// store the value
																	if (Vertices && VertMap && (DimCount < 3)) // prevent idiocy
																		{
																		VertMap->CoordsArray[DimCount][Dest4] = DestFloat;
																		VertMap->CoordsValid[Dest4] = 1;
																		} // if

																	} // for
																} // while
															} // if
														} // if
													if (!strncmp((char *)ChunkData, "RGB", 3)) // RGB or RGBA
														{
														ChunkData += 4; // advance over ID
														memcpy(&Dest2, ChunkData, 2);
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(Dest2, &Dest2);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short); // advance by 2
														Dimension = Dest2;
														if ((Dimension == 3) || (Dimension == 4)) // 3:RGB 4:RGBA
															{
															if (CPVTable && CPVTable[FileNumRGBMaps].AllocMap(NumVertices))
																{
																VertMap = &CPVTable[FileNumRGBMaps];
																VertexColorsAvailable = 1;
																FileNumRGBMaps++;
																} // if
															// read name
															StringCount = 0;
															VMAPName = (char *)ChunkData; // just a pointer, not a copy!
															// [S0]
															do {StringCount++; } while (*(ChunkData++) != 0);
															if (StringCount & 0x01) // odd number of bytes?
																{
																ChunkData++; // eat pad byte
																} // if

															while (ChunkData < DFB.GetBlockEnd())
																{
																// read vertex XV
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4, &Dest4);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2, &Dest2);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 = Dest2; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Poly VX
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4P, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4P, &Dest4P);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4P &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2P, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2P, &Dest2P);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4P = Dest2P; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Values
																for (int DimCount = 0; DimCount < Dimension; DimCount++)
																	{
																	memcpy(&DestFloat, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&DestFloat, &DestFloat);
																	#endif // BYTEORDER_LITTLEENDIAN
																	ChunkData += sizeof(float); // advance by 4
																	// store the value
																	if (Vertices && VertMap && (DimCount < 3)) // We don't support A in vertex-xolor mode
																		{
																		VertMap->CoordsArray[DimCount][Dest4] = DestFloat;
																		VertMap->CoordsValid[Dest4] = 1;
																		} // if

																	} // for
																} // while
															} // if
														} // if
													} // if
												} // if
											fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
											break;
											} // VMAD
										case 'VMAP':
											{ // UV coords or RGB values
											if (LWO2)
												{
												unsigned char *ChunkData;
												char *VMAPName = NULL;
												DynFileBlock DFB; // automatically destroyed/freed when we leave scope
												unsigned long Dest4;
												unsigned short Dest2;
												float DestFloat;
												unsigned short Dimension;
												long StringCount;

												if (ChunkData = DFB.ReadDynBlock(fInput, ChunkSize))
													{
													if (!strcmp((char *)ChunkData, "TXUV"))
														{
														ChunkData += 4; // advance over ID
														memcpy(&Dest2, ChunkData, 2);
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(Dest2, &Dest2);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short); // advance by 2
														Dimension = Dest2;
														if (Dimension == 2)
															{
															// read name
															StringCount = 0;
															VMAPName = (char *)ChunkData; // just a pointer, not a copy!
															// [S0]
															do {StringCount++; } while (*(ChunkData++) != 0);
															if (StringCount & 0x01) // odd number of bytes?
																{
																ChunkData++; // eat pad byte
																} // if
															if (UVWTable && UVWTable[FileNumUVMaps].AllocMap(NumVertices))
																{
																VertMap = &UVWTable[FileNumUVMaps];
																// SetName implemented now
																UVWTable[FileNumUVMaps].SetName(VMAPName);
																VertexUVWAvailable = 1;
																FileNumUVMaps++;
																} // if

															while (ChunkData < DFB.GetBlockEnd())
																{
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4, &Dest4);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2, &Dest2);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 = Dest2; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Values
																for (int DimCount = 0; DimCount < Dimension; DimCount++)
																	{
																	memcpy(&DestFloat, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&DestFloat, &DestFloat);
																	#endif // BYTEORDER_LITTLEENDIAN
																	ChunkData += sizeof(float); // advance by 4
																	// store the value
																	if (Vertices && VertMap && (DimCount < 3)) // prevent idiocy
																		{
																		VertMap->CoordsArray[DimCount][Dest4] = DestFloat;
																		VertMap->CoordsValid[Dest4] = 1;
																		} // if

																	} // for
																} // while
															} // if
														} // if
													if (!strncmp((char *)ChunkData, "RGB", 3)) // RGB or RGBA
														{
														ChunkData += 4; // advance over ID
														memcpy(&Dest2, ChunkData, 2);
														#ifdef BYTEORDER_LITTLEENDIAN
														SimpleEndianFlip16U(Dest2, &Dest2);
														#endif // BYTEORDER_LITTLEENDIAN
														ChunkData += sizeof(unsigned short); // advance by 2
														Dimension = Dest2;
														if ((Dimension == 3) || (Dimension == 4)) // 3:RGB 4:RGBA
															{
															if (CPVTable && CPVTable[FileNumRGBMaps].AllocMap(NumVertices))
																{
																VertMap = &CPVTable[FileNumRGBMaps];
																VertexColorsAvailable = 1;
																FileNumRGBMaps++;
																} // if
															// read name
															StringCount = 0;
															VMAPName = (char *)ChunkData; // just a pointer, not a copy!
															// [S0]
															do {StringCount++; } while (*(ChunkData++) != 0);
															if (StringCount & 0x01) // odd number of bytes?
																{
																ChunkData++; // eat pad byte
																} // if

															while (ChunkData < DFB.GetBlockEnd())
																{
																if (ChunkData[0] == 0xFF)
																	{ // long notation
																	memcpy(&Dest4, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32U(Dest4, &Dest4);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 &= 0x00ffffff; // mask off top 0xff signal byte
																	ChunkData += sizeof(unsigned long); // advance by 4
																	} // if
																else
																	{ // short notation
																	memcpy(&Dest2, ChunkData, 2);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip16U(Dest2, &Dest2);
																	#endif // BYTEORDER_LITTLEENDIAN
																	Dest4 = Dest2; // upconvert for usage/storage
																	ChunkData += sizeof(unsigned short); // advance by 2
																	} // else
																// read Values
																for (int DimCount = 0; DimCount < Dimension; DimCount++)
																	{
																	memcpy(&DestFloat, ChunkData, 4);
																	#ifdef BYTEORDER_LITTLEENDIAN
																	SimpleEndianFlip32F(&DestFloat, &DestFloat);
																	#endif // BYTEORDER_LITTLEENDIAN
																	ChunkData += sizeof(float); // advance by 4
																	// store the value
																	if (Vertices && VertMap && (DimCount < 3)) // We don't support A in vertex-xolor mode
																		{
																		VertMap->CoordsArray[DimCount][Dest4] = DestFloat;
																		VertMap->CoordsValid[Dest4] = 1;
																		} // if

																	} // for
																} // while
															} // if
														} // if
													} // if
												} // if
											break;
											} // VMAP
										case 'TAGS':
											{ // skip chunk, we handled it in first pass
											fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
											break;
											} // TAGS
										case 'BBOX':
											{ // 
											if (LWO2)
												{
												// not implemented, skip
												} // if
											fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
											break;
											} // BBOX
										default:
											{ // unknown chunk type
											fseek(fInput, ChunkSize, SEEK_CUR); // skip chunk
											break;
											} // default
										} // switch chunktag ID
									} // if chunksize > 0
								// Don't fail out, just move on to the next chunk. Blender 2.44 is known to write zero-sized empty ICON chunks
								// http://web.archive.org/web/20010423140254/http://members.home.net/erniew2/lwsdk/docs/filefmts/lwo2.html#H_file
								// that were tripping us up.
								//else
								//	Success = 0;
								} // if read chunk size
							else
								Success = 0;
							} // while reading new chunktag
						} // if LWOB
					else
						Success = 0;
					} // if read chunktag
				else
					Success = 0;
				} // if read FORM size
			else
				Success = 0;
			} // if FORM
		else
			Success = 0;
		} // if read chunktag
	else
		Success = 0;
	fclose(fInput);
	} // if file opened
else
	Success = 0;

if (! Success)
	UserMessageOK(Name, "Error reading LightWave 3D Object file");
else
	{
	int Found, RenameIt, RenameAll = 0, ShareAll = 0;
	MaterialEffect *TestMat;
	Object3DEffect *CurrentObj;
	if (LWO2)
		{
		// count number of materials used and allocate NameTable
		for (int SurfScan = 0; SurfScan < NumTT; SurfScan++)
			{
			if (LWTT.GetSurfID(SurfScan) != -1)
				{ // it's a real surface!
				NumMaterials++;
				} // if
			} // if
		if (NameTable = new ObjectMaterialEntry[NumMaterials])
			{
			int SurfMake, SurfScan, MatDupWarned = 0;

			for (SurfScan = 0; SurfScan < NumTT; SurfScan++)
				{
				if (LWTT.GetSurfID(SurfScan) != -1)
					{ // search for duplicates using case insensitivity
					int Duplicates = 0;
					char AppendedName[256];

					for (int Ct2 = SurfScan + 1; Ct2 < NumTT; Ct2++)
						{
						if (LWTT.GetSurfID(SurfScan) != -1)
							{
							if (! stricmp((char *)LWTT.GetEntryName(SurfScan), (char *)LWTT.GetEntryName(Ct2)))
								{
								int Unique = 0;

								if (! MatDupWarned)
									UserMessageOK((char *)LWTT.GetEntryName(SurfScan), "This name is used by more than one surface in this object. Some materials will have numerals appended to make their names unique.");
								MatDupWarned = 1;
								while (! Unique)
									{
									Unique = 1;
									sprintf(AppendedName, "%s.%d", (char *)LWTT.GetEntryName(SurfScan), ++Duplicates);
									LWTT.SetEntryName(Ct2, (unsigned char *)AppendedName);
									for (int Ct3 = 0; Ct3 < NumTT; Ct3 ++)
										{
										if (Ct2 == Ct3)
											continue;
										if (! stricmp((char *)LWTT.GetEntryName(Ct2), (char *)LWTT.GetEntryName(Ct3)))
											{
											Unique = 0;
											break;
											} // if
										} // for
									} // while
								} // if
							} // if
						} // for
					} // if
				} // for

			for (SurfScan = 0; SurfScan < NumTT; SurfScan++)
				{
				if ((SurfMake = LWTT.GetSurfID(SurfScan)) != -1)
					{ // Make it for real
					// test to see if the material name already exists and not for this object
					// if the material exists and is not used by another object then it can 
					// be assumed that it is for this object alone
					if (TestMat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, (char *)LWTT.GetEntryName(SurfScan)))
						{
						// see if the material is in use
						Found = 0;
						for (CurrentObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurrentObj; CurrentObj = (Object3DEffect *)CurrentObj->Next)
							{
							if (CurrentObj == this)
								continue;
							if (Found = CurrentObj->MatchMaterialName(TestMat->Name))
								break;
							} // for

						if (Found)
							{
							// see if user wants a unique material
							if (! ShareAll && (RenameAll || (RenameIt = UserMessageCustomQuad(TestMat->Name, "This material name is used by another object. Would you like to change the name of the material for the object being loaded or the share the material between different objects?", "Rename", "Share", "Rename All", "ShareAll", 0))))
								{
								if (RenameIt == 2)
									RenameAll = 1;
								if (RenameIt == 3)
									{
									RenameIt = 0;
									ShareAll = 1;
									} // if
								// modify the name to make it unique
								if (RenameIt)
									GlobalApp->AppEffects->CreateUniqueName(WCS_EFFECTSSUBCLASS_MATERIAL, (char *)LWTT.GetEntryName(SurfScan));
								} // if
							} // if
						} // if
					NameTable[SurfMake].SetName((char *)LWTT.GetEntryName(SurfScan));
					GlobalApp->AppEffects->SetMakeMaterial(&NameTable[SurfMake], &TextureTable[SurfScan]);
					} // if
				} // if
			} // if
		} // if
	else
		{ // LWOB
		for (Count = 0; Count < NumMaterials; Count ++)
			{
			if (TestMat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable[Count].Name))
				{
				// see if the material is in use
				Found = 0;
				for (CurrentObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurrentObj; CurrentObj = (Object3DEffect *)CurrentObj->Next)
					{
					if (CurrentObj == this)
						continue;
					if (Found = CurrentObj->MatchMaterialName(TestMat->Name))
						break;
					} // for

				if (Found)
					{
					// see if user wants a unique material
					if (! ShareAll && (RenameAll || (RenameIt = UserMessageCustomQuad(TestMat->Name, "This material name is used by another object. Would you like to change the name of the material for the object being loaded or the share the material between different objects?", "Rename", "Share", "Rename All", "ShareAll", 0))))
						{
						if (RenameIt == 2)
							RenameAll = 1;
						if (RenameIt == 3)
							{
							RenameIt = 0;
							ShareAll = 1;
							} // if
						// modify the name to make it unique
						if (RenameIt)
							GlobalApp->AppEffects->CreateUniqueName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable[Count].Name);
						} // if
					} // if
				} // if
			GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Count], &TextureTable[Count]);
			} // for
		} // else
	} // else
if (TextureTable)
	delete [] TextureTable;
if (NamePool)
	AppMem_Free(NamePool, NamePoolSize);


if (NumVertices == 0 || NumPolys == 0)
	{
	Success = 0;
	} // if

return (Success);

} // Object3DEffect::ReadLWObjectFile


/*===========================================================================*/

void Object3DEffect::ProcessLWMaterial(char *NamePool, long ChunkSize, ObjectMaterialEntry *Names, MaterialAttributes *Mat)
{
short ShortInt, SubChunkSize;
unsigned long SubChunkTagID;
long Found, BytesRead = 0;
float FloatVal;

BytesRead = (long)(strlen(NamePool) + 1);

if (BytesRead % 2)
	BytesRead ++;

for (Found = 0; Found < NumMaterials; Found ++)
	{
	if (! strcmp(Names[Found].Name, NamePool))
		break;
	} // for
if (Found < NumMaterials)
	{
	while (BytesRead < ChunkSize)
		{
		SubChunkTagID = MAKE_ID(NamePool[BytesRead], NamePool[BytesRead + 1], NamePool[BytesRead + 2], NamePool[BytesRead + 3]);
		BytesRead += 4;
		memcpy(&SubChunkSize, &NamePool[BytesRead], 2);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16S(SubChunkSize, &SubChunkSize);
		#endif // BYTEORDER_LITTLEENDIAN
		BytesRead += 2;
		if (SubChunkSize > 0)
			{
			switch (SubChunkTagID)
				{
				case 'COLR':
					{
					Mat[Found].Red = (unsigned char)NamePool[BytesRead];
					Mat[Found].Green = (unsigned char)NamePool[BytesRead + 1];
					Mat[Found].Blue = (unsigned char)NamePool[BytesRead + 2];
					BytesRead += SubChunkSize;
					break;
					} // COLR
				case 'FLAG':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Shading = (ShortInt & (1 << 2)) ? WCS_EFFECT_MATERIAL_SHADING_PHONG: WCS_EFFECT_MATERIAL_SHADING_FLAT;
					Mat[Found].DoubleSided = (ShortInt & (1 << 8)) ? 1: 0;
					BytesRead += SubChunkSize;
					break;
					} // FLAG
				case 'LUMI':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Luminosity = ((double)ShortInt / 256.0);
					BytesRead += SubChunkSize;
					break;
					} // LUMI
				case 'VLUM':
					{
					memcpy(&FloatVal, &NamePool[BytesRead], 4);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32F(&FloatVal, &FloatVal);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Luminosity = (double)FloatVal;
					BytesRead += SubChunkSize;
					break;
					} // DIFF
				case 'DIFF':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Diffuse = ((double)ShortInt / 256.0);
					BytesRead += SubChunkSize;
					break;
					} // DIFF
				case 'VDIF':
					{
					memcpy(&FloatVal, &NamePool[BytesRead], 4);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32F(&FloatVal, &FloatVal);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Diffuse = (double)FloatVal;
					BytesRead += SubChunkSize;
					break;
					} // DIFF
				case 'SPEC':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Specularity = ((double)ShortInt / 256.0);
					BytesRead += SubChunkSize;
					break;
					} // SPEC
				case 'VSPC':
					{
					memcpy(&FloatVal, &NamePool[BytesRead], 4);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32F(&FloatVal, &FloatVal);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Specularity = (double)FloatVal;
					BytesRead += SubChunkSize;
					break;
					} // DIFF
				case 'REFL':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Reflectivity = ((double)ShortInt / 256.0);
					BytesRead += SubChunkSize;
					break;
					} // REFL
				case 'TRAN':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].Transparency = ((double)ShortInt / 256.0);
					BytesRead += SubChunkSize;
					break;
					} // TRAN
				case 'GLOS':
					{
					memcpy(&ShortInt, &NamePool[BytesRead], 2);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(ShortInt, &ShortInt);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].SpecExponent = (double)ShortInt;
					BytesRead += SubChunkSize;
					break;
					} // GLOS
				case 'SMAN':
					{
					memcpy(&FloatVal, &NamePool[BytesRead], 4);
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32F(&FloatVal, &FloatVal);
					#endif // BYTEORDER_LITTLEENDIAN
					Mat[Found].SmoothAngle = (double)FloatVal;
					BytesRead += SubChunkSize;
					break;
					} // SMAN
				default:
					{
					BytesRead += SubChunkSize;
					break;
					} // chunk unknown
				} // switch SubChunkTagID
			} // if SubChunkSize
		} // while
	} // if Found

} // Object3DEffect::ProcessLWMaterial

/*===========================================================================*/

#ifdef WCS_SUPPORT_3DS

// This code was used for material assignment prior to MULTIMESH capability. It's here purely
// for reference now.
/*
											for (Ct = 0; Success && Ct < NumMaterials; Ct ++)
												{
												GetMaterialByName3ds(Database3ds, Mesh3ds->matarray[Ct].name, &Material3ds);
												if (Material3ds && ! ftkerr3ds)
													{
													NameTable[Ct].SetName(Mesh3ds->matarray[Ct].name);
													Process3dsMaterial(&TextureTable[Ct], Material3ds);
													for (Poly = 0; Poly < Mesh3ds->matarray[Ct].nfaces; Poly ++)
														{
														Polygons[Mesh3ds->matarray[Ct].faceindex[Poly]].Material = Ct;
														} // for
													} // if
												else
													Success = 0;
												if (Material3ds)
													ReleaseMaterial3ds(&Material3ds);
												} // for
*/


short Object3DEffect::Read3dsObjectFile(char *filename, short LoadObj)
{
short Success = 1, DefaultMaterial = 0, *VertRef;
long Poly, PolyVert, Ct; //, RefCt;
dbtype3ds DBType3ds;
ulong3ds NumMeshes3ds;
database3ds *Database3ds = NULL;
//meshset3ds *MeshSet3ds = NULL;
material3ds *Material3ds = NULL;
mesh3ds *Mesh3ds = NULL;
//kfsets3ds *KFSets3ds = NULL;
kfmesh3ds *KFMesh3ds = NULL;
//namelist3ds *NameList3ds = NULL;
file3ds *File3ds = NULL;
MaterialAttributes *TextureTable = NULL;
int AllocStuff = 1;
long TotalNumMaterials, NumTexVert;
long TotalNumVertices, TotalNumPolys, TotalNumMeshes;
long ObjBaseVertexNum = 0, ObjBasePolyNum = 0;
unsigned long MeshLoop;
int VertRefBlockIdx = 0, PolyRefBlockIdx = 0;
ObjectPerVertexMap *VertMap = NULL;

TotalNumVertices = TotalNumPolys = TotalNumMeshes = 0;
VertexUVWAvailable = VertexColorsAvailable = 0;

TotalNumMaterials = 0;

FreeDynamicStuff();
CalcNormals = 1;

//GlobalApp->MainProj->MungPath(filename);
if ((File3ds = OpenFile3ds((const char3ds *)GlobalApp->MainProj->MungPath(filename), "r")) != NULL)
	{
	InitDatabase3ds(&Database3ds);
	if (Database3ds && ! ftkerr3ds)
		{
		CreateDatabase3ds(File3ds, Database3ds);
		DBType3ds = GetDatabaseType3ds(Database3ds);
		if (DBType3ds == MeshFile && ! ftkerr3ds)
			{
			DisconnectDatabase3ds(Database3ds);
			if (ftkerr3ds)
				ftkerr3ds = 0;
			if ((NumMeshes3ds = GetMeshCount3ds(Database3ds)) > 0)
				{
				TotalNumMaterials = GetMaterialCount3ds(Database3ds);
				if (TotalNumMaterials == 0)
					{
					TotalNumMaterials = 1;
					DefaultMaterial = 1;
					} // if
				NameTable = new ObjectMaterialEntry[TotalNumMaterials];
				TextureTable = new MaterialAttributes[TotalNumMaterials];
				// create all materials in advance so we can utilize them as they occur
				if (!DefaultMaterial)
					{
					for (Ct = 0; Success && Ct < TotalNumMaterials; Ct ++)
						{
						GetMaterialByIndex3ds(Database3ds, Ct, &Material3ds);
						if (Material3ds && ! ftkerr3ds)
							{
							char PathDest[512], FileDest[512];
							NameTable[Ct].SetName(Material3ds->name);
							BreakFileName(filename, PathDest, 512, FileDest, 512);
							Process3dsMaterial(&TextureTable[Ct], Material3ds, PathDest);
							} // if
						else
							Success = 0;
						if (Material3ds)
							ReleaseMaterial3ds(&Material3ds);
						} // for
					} // if

				// now DefaultMaterial becomes a per-object indicator
				DefaultMaterial = 0;

				TotalNumMeshes = NumMeshes3ds;
				for (MeshLoop = 0; MeshLoop < NumMeshes3ds; MeshLoop++)
					{
					GetMeshByIndex3ds(Database3ds, MeshLoop, &Mesh3ds);
					if (Mesh3ds && ! ftkerr3ds && Mesh3ds->nvertices > 0 && Mesh3ds->nfaces > 0)
						{
						TotalNumVertices += Mesh3ds->nvertices;
						TotalNumPolys += Mesh3ds->nfaces;
						} // if
					RelMeshObj3ds(&Mesh3ds);
					} // for

				for (MeshLoop = 0, GetMeshByIndex3ds(Database3ds, MeshLoop, &Mesh3ds); (Mesh3ds && ! ftkerr3ds); MeshLoop++, GetMeshByIndex3ds(Database3ds, MeshLoop, &Mesh3ds))
					{
					if (Mesh3ds && ! ftkerr3ds)
						{
						GetObjectMotionByName3ds(Database3ds, Mesh3ds->name, &KFMesh3ds);

						if ((NumVertices = Mesh3ds->nvertices) > 0 && (NumPolys = Mesh3ds->nfaces) > 0)
							{
							if (NumTexVert = Mesh3ds->ntextverts)
								{ // flag object as having UVW available
								if (! UVWTable && AllocUVWMaps(1))
									{
									NumUVWMaps = 1;
									if (UVWTable[0].AllocMap(TotalNumVertices))
										{
										VertMap = &UVWTable[0];
										VertexUVWAvailable = 1;
										} // if
									} // if
								} // if
							if ((NumMaterials = Mesh3ds->nmats) == 0)
								{
								NumMaterials = 1;
								DefaultMaterial = 1;
								} // if
							if (AllocStuff)
								{
								Vertices = new Vertex3D[TotalNumVertices];
								Polygons = new Polygon3D[TotalNumPolys];
								// can no longer compile without VertRefBlock support, but who'd want to?
								AllocVertRef(TotalNumPolys * 3); // block pre-allocate
								//PolyRefBlock = new long [TotalNumPolys * 3]; // block pre-allocate
								if (Vertices && Polygons && VertRefBlock/* && PolyRefBlock*/)
									{
									AllocStuff = 0;
									} // if
								} // if
							if (!AllocStuff && Vertices && Polygons && VertRefBlock && /*PolyRefBlock && */NameTable && TextureTable)
								{
								// this next allocation is just temporary storage
								if (VertRef = (short *)AppMem_Alloc(NumVertices * sizeof(short), APPMEM_CLEAR))
									{
									for (Ct = 0; Ct < NumPolys; Ct ++)
										{
/*										if ((Polygons[ObjBasePolyNum + Ct].VertRef = (long *)AppMem_Alloc(3 * sizeof(long), APPMEM_CLEAR)) == NULL)
											{
											Success = 0;
											break;
											} // if
*/
										// can't fail if pre-allocated
										Polygons[ObjBasePolyNum + Ct].VertRef = &VertRefBlock[VertRefBlockIdx];
										Polygons[ObjBasePolyNum + Ct].VertRef[0] = Polygons[ObjBasePolyNum + Ct].VertRef[1] = Polygons[ObjBasePolyNum + Ct].VertRef[2] = 0;
										VertRefBlockIdx += 3; // We just used 3 longs
										VertRef[Mesh3ds->facearray[Ct].v1] ++;
										VertRef[Mesh3ds->facearray[Ct].v2] ++;
										VertRef[Mesh3ds->facearray[Ct].v3] ++;
										Polygons[ObjBasePolyNum + Ct].VertRef[0] = Mesh3ds->facearray[Ct].v3 + ObjBaseVertexNum;
										Polygons[ObjBasePolyNum + Ct].VertRef[1] = Mesh3ds->facearray[Ct].v2 + ObjBaseVertexNum;
										Polygons[ObjBasePolyNum + Ct].VertRef[2] = Mesh3ds->facearray[Ct].v1 + ObjBaseVertexNum;
										Polygons[ObjBasePolyNum + Ct].NumVerts = 3;
										} // for
									if (Success)
										{
										if (KFMesh3ds)
											{
											} // 
										// copy actual coords
										for (Ct = 0; Ct < NumVertices; Ct ++)
											{
											if (VertMap && Ct < NumTexVert)
												{
												VertMap->CoordsArray[0][ObjBaseVertexNum + Ct] = Mesh3ds->textarray[Ct].u;
												VertMap->CoordsArray[1][ObjBaseVertexNum + Ct] = Mesh3ds->textarray[Ct].v;
												VertMap->CoordsValid[ObjBaseVertexNum + Ct] = 1;
												} // if
											// Real coords
											if (KFMesh3ds)
												{
												Vertices[ObjBaseVertexNum + Ct].xyz[0] = (double)Mesh3ds->vertexarray[Ct].x - KFMesh3ds->pivot.x;
												Vertices[ObjBaseVertexNum + Ct].xyz[1] = (double)Mesh3ds->vertexarray[Ct].z - KFMesh3ds->pivot.z;	// y and z are reversed
												Vertices[ObjBaseVertexNum + Ct].xyz[2] = (double)Mesh3ds->vertexarray[Ct].y - KFMesh3ds->pivot.y;
												} // if
											else
												{
												Vertices[ObjBaseVertexNum + Ct].xyz[0] = (double)Mesh3ds->vertexarray[Ct].x;
												Vertices[ObjBaseVertexNum + Ct].xyz[1] = (double)Mesh3ds->vertexarray[Ct].z;	// y and z are reversed
												Vertices[ObjBaseVertexNum + Ct].xyz[2] = (double)Mesh3ds->vertexarray[Ct].y;
												} // else
											Vertices[ObjBaseVertexNum + Ct].NumPolys = VertRef[Ct];
											if (VertRef[Ct] > 0)
												{
												//Vertices[ObjBaseVertexNum + Ct].PolyRef = (long *)&PolyRefBlock[PolyRefBlockIdx];
												//PolyRefBlockIdx += VertRef[Ct]; // just used VertRef[Ct] PolyRefBlock entries
												
												if ((Vertices[ObjBaseVertexNum + Ct].PolyRef = (long *)AppMem_Alloc(VertRef[Ct] * sizeof(long), APPMEM_CLEAR)) == NULL)
													{
													Success = 0;
													break;
													} // if
												// gh added 3/26/02
												VertRef[Ct] --;	// used later as an index so don't fencepost the array
												} // if
											} // for
										// resolve Vertex<->Polygon linkage
										for (Poly = ObjBasePolyNum; Poly < ObjBasePolyNum + NumPolys; Poly ++)
											{
											for (PolyVert = 0; PolyVert < Polygons[Poly].NumVerts; PolyVert ++)
												{
												// this is the vertex number referred to by the polygon, it contains the vertex offset for this mesh
												Ct = Polygons[Poly].VertRef[PolyVert];
												// link up UV data
												// This is apparently not necessary with 1:1 geometry to texture vertex association
												//Polygons[Poly].AddVertRef(Ct, Ct, WCS_VERTREFDATA_MAPTYPE_UVW, 0);

												// gh added sanity check 3/28/02
												if (VertRef[Ct - ObjBaseVertexNum] >= 0)	// VertRef is only the size of this mesh's vertex count
													{
													// gh changed 3/26/02
													//Vertices[Ct].PolyRef[VertRef[Ct]--] = Poly + ObjBasePolyNum;
													// gh changed again 3/28/02 - subtracted vertex base offset to index
													Vertices[Ct].PolyRef[VertRef[Ct - ObjBaseVertexNum]--] = Poly;
													} // if
												} // for
											} // for
										} // if
									if (Success)
										{
										if (DefaultMaterial)
											{
											NameTable[0].SetName(Mesh3ds->name);
											for (Poly = 0; Poly < NumPolys; Poly ++)
												{
												Polygons[ObjBasePolyNum + Poly].Material = 0;
												} // for
											// polygons default to material 0 so don't need to set them
											} // if
										else
											{
											// assign materials in object to pre-created materials list
											for (Ct = 0; Success && Ct < Mesh3ds->nmats; Ct ++)
												{
												// Search for already-created material ID num matching this object's material's text name
												int MatScan, MatNum = 0;
												for (MatScan = 0; MatScan < TotalNumMaterials; MatScan++)
													{
													if (!NameTable[MatScan].CompareName(Mesh3ds->matarray[Ct].name))
														{
														MatNum = MatScan;
														break;
														} // if
													} // for
												for (Poly = 0; Poly < Mesh3ds->matarray[Ct].nfaces; Poly ++)
													{
													// I think this should work now.
													Polygons[ObjBasePolyNum + Mesh3ds->matarray[Ct].faceindex[Poly]].Material = MatNum;
													} // for
												} // for
											} // else
										} // if
									AppMem_Free(VertRef, NumVertices * sizeof(short));
									} // if VertRef
								} // if vertices and polygons allocated
							} // if
						else
							Success = 0;
						if (KFMesh3ds)
							ReleaseObjectMotion3ds(&KFMesh3ds);
						} // if
					else
						Success = 0;

					if (Mesh3ds)
						RelMeshObj3ds(&Mesh3ds);

					ObjBaseVertexNum += NumVertices;
					ObjBasePolyNum += NumPolys;

					} // for
				} // if
			else
				Success = 0;
			} // if correct file type
		else
			Success = 0;
		} // if Database3ds
	else
		Success = 0;
	if (Database3ds)
		ReleaseDatabase3ds(&Database3ds);
	CloseAllFiles3ds();
	} // if file open
else
	Success = 0;

NumVertices = TotalNumVertices;
NumPolys = TotalNumPolys;

if (! Success)
	UserMessageOK(Name, "Error reading 3DS Object file");
else
	{
	// make all material names non-case sensitive unique
	for (Ct = 0; Ct < TotalNumMaterials; Ct ++)
		{
		int Duplicates = 0;

		for (int Ct2 = Ct + 1; Ct2 < TotalNumMaterials; Ct2 ++)
			{
			if (! stricmp(NameTable[Ct].Name, NameTable[Ct2].Name))
				{
				int Unique = 0;

				while (! Unique)
					{
					Unique = 1;
					sprintf(NameTable[Ct2].Name, "%s.%d", NameTable[Ct].Name, ++Duplicates);
					for (int Ct3 = 0; Ct3 < TotalNumMaterials; Ct3 ++)
						{
						if (Ct2 == Ct3)
							continue;
						if (! stricmp(NameTable[Ct2].Name, NameTable[Ct3].Name))
							{
							Unique = 0;
							break;
							} // if
						} // for
					} // while
				} // if
			} // for
		} // for

	for (Ct = 0; Ct < TotalNumMaterials; Ct ++)
		{
		GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Ct], &TextureTable[Ct]);
		} // for
	NumMaterials = TotalNumMaterials;
	} // else
if (TextureTable)
	delete [] TextureTable;

return (Success);

} // Object3DEffect::Read3dsObjectFile

/*===========================================================================*/

void Object3DEffect::Process3dsMaterial(MaterialAttributes *Mat, material3ds *Material3ds, char *ObjectPath)
{

Mat->Red = (unsigned char)(Material3ds->diffuse.r * 255.99);
Mat->Green = (unsigned char)(Material3ds->diffuse.g * 255.99);
Mat->Blue = (unsigned char)(Material3ds->diffuse.b * 255.99);
if (Material3ds->shading == 1)
	Mat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
else if (Material3ds->shading == 2)
	Mat->Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
else
	Mat->Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
Mat->DoubleSided = Material3ds->twosided;
Mat->Luminosity = Material3ds->selfillum ? Material3ds->selfillumpct: 0.0;
if (Mat->Luminosity > 1.0) Mat->Luminosity *= .01;
Mat->Specularity = Material3ds->shininess * .01;
if (Mat->Specularity > 1.0) Mat->Specularity *= .01;
Mat->Transparency = Material3ds->transparency;
if (Mat->Transparency > 1.0) Mat->Transparency *= .01;
Mat->SpecExponent = pow(256.0, (double)Material3ds->shinstrength);

if (Material3ds->texture.map.name[0] && (Material3ds->texture.map.percent != 0.0))
	{ // seems to be a map present
	MaterialTextureInfo *MTI;
	bitmap3ds *bm3 = &Material3ds->texture.map; // shorthand
	// ensure extended data block available
	if (MTI = Mat->GetCreateMaterialTextureInfo(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
		{
		if (MTI->Scale[0] == 1.0 && MTI->Scale[1] == 1.0 && MTI->Scale[2] == 1.0 &&
		 MTI->Trans[0] == 0.0 && MTI->Trans[1] == 0.0 && MTI->Trans[2] == 0.0 &&
		 MTI->Rot[0] == 0.0 && MTI->Rot[1] == 0.0 && MTI->Rot[2] == 0.0)
			{
			MTI->SetType(WCS_TEXTURE_TYPE_UVW); // simpler element type
			} // if
		else
			{
			MTI->SetType(WCS_TEXTURE_TYPE_PLANARIMAGE); // must use more complex PLANARIMAGE with CoordSpace=UVW
			MTI->SetSpace(WCS_TEXTURE_COORDSPACE_VERTEX_UVW);
			} // else
		MTI->SetName(bm3->name);
		MTI->SetPath(ObjectPath);
		MTI->SetUVName("Vertex Map"); // only one UV set available in 3DS
		if (bm3->tiling == 2)
			{
			// no tile, just decal
			MTI->TileX = MTI->TileY = 0;
			MTI->Decal = 1;
			} // if
		else if (bm3->tiling == 3)
			{
			// tile and decal on
			MTI->Decal = 1;
			} // if
		else
			{
			// tiling defaults to on, decal defaults to off
			} // else
		if (bm3->mirror)
			{
			MTI->FlipX = 1;
			// I don't know if mirror means both axes or just X
			//MTI->FlipY = 1;
			} // if
		MTI->Negative = bm3->negative;
		MTI->Scale[0] = bm3->uscale;
		MTI->Scale[1] = bm3->vscale;
		MTI->Trans[0] = bm3->uoffset;
		MTI->Trans[1] = bm3->voffset;
		MTI->Rot[2] = bm3->rotation; // I think they mean rotation on Z.
		MTI->Opacity = bm3->percent * 100.0f;
		} // if
	} // if

if (Material3ds->opacity.map.name[0] && (Material3ds->opacity.map.percent != 0.0))
	{ // seems to be a map present
	MaterialTextureInfo *MTI;
	bitmap3ds *bm3 = &Material3ds->opacity.map; // shorthand
	// ensure extended data block available
	if (MTI = Mat->GetCreateMaterialTextureInfo(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
		{
		if (MTI->Scale[0] == 1.0 && MTI->Scale[1] == 1.0 && MTI->Scale[2] == 1.0 &&
		 MTI->Trans[0] == 0.0 && MTI->Trans[1] == 0.0 && MTI->Trans[2] == 0.0 &&
		 MTI->Rot[0] == 0.0 && MTI->Rot[1] == 0.0 && MTI->Rot[2] == 0.0)
			{
			MTI->SetType(WCS_TEXTURE_TYPE_UVW); // simpler element type
			} // if
		else
			{
			MTI->SetType(WCS_TEXTURE_TYPE_PLANARIMAGE); // must use more complex PLANARIMAGE with CoordSpace=UVW
			MTI->SetSpace(WCS_TEXTURE_COORDSPACE_VERTEX_UVW);
			} // else
		MTI->SetName(bm3->name);
		MTI->SetPath(ObjectPath);
		MTI->SetUVName("Vertex Map"); // only one UV set available in 3DS
		if (bm3->tiling == 2)
			{
			// no tile, just decal
			MTI->TileX = MTI->TileY = 0;
			MTI->Decal = 1;
			} // if
		else if (bm3->tiling == 3)
			{
			// tile and decal on
			MTI->Decal = 1;
			} // if
		else
			{
			// tiling defaults to on, decal defaults to off
			} // else
		if (bm3->mirror)
			{
			MTI->FlipX = 1;
			// I don't know if mirror means both axes or just X
			//MTI->FlipY = 1;
			} // if
		MTI->Negative = bm3->negative;
		MTI->Scale[0] = bm3->uscale;
		MTI->Scale[1] = bm3->vscale;
		MTI->Trans[0] = bm3->uoffset;
		MTI->Trans[1] = bm3->voffset;
		MTI->Rot[2] = bm3->rotation; // I think they mean rotation on Z.
		MTI->Opacity = bm3->percent * 100.0f;
		} // if
	} // if

} // Object3DEffect::Process3dsMaterial

#endif // WCS_SUPPORT_3DS

/*===========================================================================*/

short Object3DEffect::ReadDXFObjectFile(char *filename, short LoadObj)
{
short Success = 1;
FILE *fInput;

CalcNormals = 1;
FreeDynamicStuff();
if (fInput = PROJ_fopen(filename, "rb"))
	{
	// Import3DObjDXF is no longer a minion of the Database
	Success = (short)Import3DObjDXF(fInput, NULL, NULL, this);
	fclose(fInput);
	} // if

return (Success);

} // Object3DEffect::ReadDXFObjectFile

/*===========================================================================*/


static char WFOBJinBuf[1024];
//static char WFFaceParseBuf[50];
static char *WFOBJPolyBuf[WCS_EFFECTOBJECT3D_OBJ_MAXPOINTSPERPOLY];

short Object3DEffect::ReadWF_OBJFile(char *filename, short LoadObj)
{
short Success = 1, Vtx;
long WFPolyCount, WFVtxCount, WFUVWCount, WFMatlCount, WFCurMat, WFCurMap, WFNextVertex, WFNextPoly, WFNextMat, WFNextUV, FileObjectCount;
char FilePass;
char RotateLikeXFrog = 0, KeepCheckingComments = 1;
char ObjFilePath[512], ObjFileName[64];
long NumTT = 0, NumUVWs = 0;
MaterialAttributes *TextureTable = NULL;
FILE *fInput, *fMInput;
DynFileBlock TDFB; // Tags DFB: automatically destroyed/freed when we leave scope
LWTagTable LWTT;
ObjectPerVertexMap *VertMap = NULL;

FreeDynamicStuff();
CalcNormals = 1;
VertexColorsAvailable = VertexUVWAvailable = 0;

if (fInput = PROJ_fopen(filename, "r"))
	{
	BreakFileName(filename, ObjFilePath, 510, ObjFileName, 63);
	// make two passes through the file
	WFPolyCount = WFVtxCount = WFUVWCount = 0;
	FileObjectCount = 0;
	WFNextVertex = WFNextPoly = 0;
	WFMatlCount = WFCurMat = WFNextMat = 0;
	WFCurMap = -1;
	WFNextUV = 0;
	for (FilePass = 0; FilePass < 2; FilePass++)
		{
		rewind(fInput); // start afresh each time
		if (FilePass == 1)
			{ // allocate after first pass
			if (WFPolyCount > 0 && WFVtxCount > 0)
				{
				if (Vertices = new Vertex3D[WFVtxCount])
					{
					NumVertices = WFVtxCount;
					if (Polygons = new Polygon3D[WFPolyCount])
						{
						NumPolys = WFPolyCount;
						} // if
					} // if
				} // if
			if (WFUVWCount > 0)
				{
				if (AllocUVWMaps(1))
					{
					NumUVWMaps = 1;
					// allocate at least as many UV entries as there are vertices or the object will fail to load if saved as a component
					// In practice it has been seen where the number of UV coordinates exceeds the number of vertices - GRH
					if (UVWTable[0].AllocMap(max(WFUVWCount, WFVtxCount)))
						{
						VertMap = &UVWTable[0];
						VertexUVWAvailable = 1;
						NumUVWs = WFUVWCount;
						} // if
					} // if
				} // if
			if (WFMatlCount)
				{
				LWTT.SetEntries(WFMatlCount);
				TextureTable = new MaterialAttributes[WFMatlCount];
				} // if
			} // if
		while (Success && fgetline(WFOBJinBuf, 1000, fInput, 0, 0))
			{
			// skip comment lines
			if (WFOBJinBuf[0] != '#')
				{
				if (!strnicmp(WFOBJinBuf, "vt", 2))
					{ // texture coordinate
					if (FilePass == 0)
						{ // first pass
						WFUVWCount++;
						} // if
					else
						{ // second pass
						if (UVWTable)
							{
							char *SkipSpace;
							float FloatU, FloatV, FloatW;
							int Count;

							if (SkipSpace = SkipPastNextSpace(&WFOBJinBuf[2]))
								{
								if ((Count = sscanf(SkipSpace, "%f %f %f", &FloatU, &FloatV, &FloatW)) >= 1)
									{
									if (VertMap && WFNextUV < WFUVWCount)
										{
										int ObjBaseVertexNum = 0;
										if (Count > 0) VertMap->CoordsArray[0][ObjBaseVertexNum + WFNextUV] = FloatU; // ObjBaseVertexNum used as placeholder for multi-object offsets
										if (Count > 1) VertMap->CoordsArray[1][ObjBaseVertexNum + WFNextUV] = FloatV; // ObjBaseVertexNum used as placeholder for multi-object offsets
										if (Count > 2) VertMap->CoordsArray[2][ObjBaseVertexNum + WFNextUV] = FloatW; // ObjBaseVertexNum used as placeholder for multi-object offsets
										VertMap->CoordsValid[ObjBaseVertexNum + WFNextUV] = 1; // ObjBaseVertexNum used as placeholder for multi-object offsets
										WFNextUV++;
										} // if
									} // if
								} // if
							} // if
						} // else
					} // if
				else if (!strnicmp(WFOBJinBuf, "o", 1))
					{ // object name
					if (FilePass == 0)
						{ // first pass
						FileObjectCount++;
						if (FileObjectCount > 1)
							{
							UserMessageOK(Name, APP_TLA " does not support multiple objects in one file.\nPlease merge objects.");
							Success = 0;
							break;
							} // if
						} // if
					else
						{ // second pass
						// nothing really to do with it
						} // else
					} // else if
				else if ((!strnicmp(WFOBJinBuf, "v ", 2)) || (!strnicmp(WFOBJinBuf, "v\t", 2))) // space or tab delimiter ok
					{ // vertex xyz entity
					if (FilePass == 0)
						{ // first pass
						WFVtxCount++;
						} // if
					else
						{ // second pass
						if (Vertices && WFNextVertex < NumVertices)
							{
							char *SkipSpace;
							float FloatX, FloatY, FloatZ, FloatW;

							if (SkipSpace = SkipPastNextSpace(&WFOBJinBuf[1]))
								{
								if (sscanf(SkipSpace, "%f %f %f %f", &FloatX, &FloatY, &FloatZ, &FloatW) >= 3)
									{
									Vertices[WFNextVertex].xyz[0] = FloatX;
									Vertices[WFNextVertex].xyz[1] = FloatY;
									Vertices[WFNextVertex].xyz[2] = -FloatZ; // Wavefront flips Z axis, apparently
									WFNextVertex++;
									} // if
								} // if
							} // if
						} // else
					} // else if
				else if ((!strnicmp(WFOBJinBuf, "f ", 2)) || (!strnicmp(WFOBJinBuf, "f\t", 2))) // space or tab delimiter ok
					{ // vertex xyz entity
					// f  v1/vt1/vn1   v2/vt2/vn2   v3/vt3/vn3 . . .
					if (FilePass == 0)
						{ // first pass
						WFPolyCount++;
						} // if
					else
						{ // second pass
						if (Polygons && WFNextPoly < NumPolys)
							{
							char *SkipSpace, *ScanSpace, OnSpace;

							if (SkipSpace = SkipPastNextSpace(&WFOBJinBuf[1]))
								{
								long WFPolyNumVtx;
								ScanSpace = SkipSpace;
								OnSpace = 0;
								WFOBJPolyBuf[0] = ScanSpace; // first entry
								for (WFPolyNumVtx = 0; /* NULL termination checked below in loop body */ ; ScanSpace++)
									{
									if (OnSpace)
										{
										if (ScanSpace[0] == NULL)
											{
											break; // nothing further to do
											} // if
										if (!isspace(ScanSpace[0]))
											{
											OnSpace = 0;
											// mark beginning of this vertex's text data
											WFOBJPolyBuf[WFPolyNumVtx] = ScanSpace;
											} // if
										} // if
									else
										{
										if (ScanSpace[0] == NULL || isspace(ScanSpace[0]))
											{
											OnSpace = 1;
											// count vertex to advance to next entry in WFOBJPolyBuf table
											WFPolyNumVtx++;
											if (ScanSpace[0] == NULL)
												{
												break; // end here, now that we incremented WFPolyNumVtx
												} // if
											else
												{
												// Drop in a NULL to terminate previous substring and keep going
												ScanSpace[0] = NULL;
												} // else
											} // if
										} // else
									} // for
								if (WFPolyNumVtx >= 3)
									{
									long VtxScan, NumValues, ThisVertex, ThisTex;
									long VtxIn, TexIn, NormIn;

									if (WFPolyNumVtx > WCS_EFFECTOBJECT3D_OBJ_MAXPOINTSPERPOLY)
										{
										UserMessageOK(Name, "Too many vertices per polygon!\nLimit of " WCS_EFFECTOBJECT3D_OBJ_MAXPOINTSPERPOLY_TEXT " has been exceeded.\nThis object cannot be rendered in "APP_TLA".");
										Success = 0;
										break;
										} // if
									if ((Polygons[WFNextPoly].VertRef = (long *)AppMem_Alloc(WFPolyNumVtx * sizeof(long), 0)) == NULL)
										{
										Success = 0;
										break;
										} // if
									Polygons[WFNextPoly].NumVerts = (short)WFPolyNumVtx;
									if (WFCurMap != -1)
										{
										Polygons[WFNextPoly].Material = WFCurMap;
										} // if
									else
										{
										Polygons[WFNextPoly].Material = WFCurMat;
										} // if

									for (VtxScan = 0; VtxScan < WFPolyNumVtx; VtxScan++)
										{
										NumValues = sscanf(WFOBJPolyBuf[VtxScan], "%d/%d/%d", &VtxIn, &TexIn, &NormIn);
										if (NumValues > 0) // we at least need the first entry, Vertex ID
											{
											ThisVertex = VtxIn;
											ThisTex = TexIn;
											if (VtxIn < 0) // relative indexing
												{
												if (WFNextVertex + VtxIn >= 0)
													{
													ThisVertex = WFNextVertex + VtxIn;
													} // if
												else
													{ // sanity check to Vertex 0 to prevent underflow
													ThisVertex = 0;
													} // else
												} // if
											else
												{
												ThisVertex -= 1; // They use 1-based, we use 0-based
												} // else
											//ThisVertex = (WFPolyNumVtx - 1) - ThisVertex;
											Polygons[WFNextPoly].VertRef[VtxScan] = ThisVertex;
											if (Vertices)
												Vertices[ThisVertex].NumPolys ++;

											if (TexIn < 0) // relative indexing
												{
												if (WFNextUV + TexIn > 0)
													{
													ThisTex = WFNextUV + TexIn;
													} // if
												else
													{ // sanity check to Tex 0 to prevent underflow
													ThisTex = 0;
													} // else
												} // if
											ThisTex -= 1; // They use 1-based, we use 0-based
											//ThisTex = (WFNextUV - 1) - ThisTex;
											if (Vertices && VertMap && (ThisTex < WFUVWCount))
												{
												Polygons[WFNextPoly].AddVertRef(ThisVertex, ThisTex, WCS_VERTREFDATA_MAPTYPE_UVW, 0);
												} // if

											} // if
										} // for
									WFNextPoly++;
									} // if
								} // if
							} // if
						} // else
					} // else if
				else if ((!strnicmp(WFOBJinBuf, "usemtl", 6)) || (!strnicmp(WFOBJinBuf, "usemap", 6)))
					{ // use material/surface/map
					// "usemtl material_name" or "usemap Simple_copyMap" or "usemap off"
					if (FilePass == 0)
						{ // first pass
						WFMatlCount ++; // rough count only, could be duplicates
						} // if
					else
						{ // second pass
						char *SkipSpace, UseMap = 0;
						long MatID;

						WFCurMat = 0; // in case below fails
						if (!strnicmp(WFOBJinBuf, "usemap", 6))
							{
							UseMap = 1;
							} // if
						if (SkipSpace = SkipPastNextSpace(&WFOBJinBuf[6]))
							{
							long TrimJunk;
							// trim off trailing junk (starts on NULL, but it's safe)
							for (TrimJunk = (long)strlen(SkipSpace); TrimJunk > 0; TrimJunk--)
								{
								if ((SkipSpace[TrimJunk] == NULL) || (SkipSpace[TrimJunk] == 10) || (SkipSpace[TrimJunk] == 13))
									{
									SkipSpace[TrimJunk] = NULL;
									} // if
								else
									{
									// stop trimming, we've found real stuff
									break;
									} // else
								} // for
							if (UseMap)
								{
								if (!stricmp(SkipSpace, "off"))
									{
									WFCurMap = -1; // disable map
									} // if
								} // if


							if ((MatID = LWTT.FindEntryByName((unsigned char*)SkipSpace)) != -1)
								{
								if (UseMap)
									{ // old-style maps supercede materials
									WFCurMap = MatID;
									} // if
								else
									{
									WFCurMat = MatID;
									} // else
								} // if
							else
								{
								LWTT.SetEntryName(WFNextMat, (unsigned char *)SkipSpace);
								LWTT.SetSurfID(WFNextMat, WFNextMat);
								if (UseMap)
									{
									WFCurMap = WFNextMat;
									} // if
								else
									{
									WFCurMat = WFNextMat;
									} // else
								WFNextMat++;
								} // else
							} // if
						} // else
					} // else if
				} // if
			else
				{ // attempt to identify some file creators by parts of their first comment lines
				if (KeepCheckingComments)
					{
					// is it xfrog? [reenworks] or [frog]
					if ((strstr(WFOBJinBuf, "frog")) || (strstr(WFOBJinBuf, "reenworks")))
						{
						RotateLikeXFrog = 1;
						KeepCheckingComments = 0;
						} // if
					// for now, only check first comment line for efficiency
					KeepCheckingComments = 0;
					} // if
				} // else
			} // while
		} // for

	fclose(fInput);
	} // if file opened
else
	Success = 0;

// stitch poly<->vertex connections
if (NumVertices && NumPolys && Vertices && Polygons)
	{
	long Count;
	for (Count = 0; Count < NumVertices; Count ++)
		{
		if (Vertices[Count].NumPolys)
			{
			if ((Vertices[Count].PolyRef = (long *)AppMem_Alloc(Vertices[Count].NumPolys * sizeof(long), 0)) == NULL)
				{
				Success = 0;
				break;
				} // if
			} // if
		Vertices[Count].NumPolys = 0;
		} // for

	for (Count = 0; Count < NumPolys; Count ++)
		{
		for (Vtx = 0; Vtx < Polygons[Count].NumVerts; Vtx ++)
			{
			if (Vertices[Polygons[Count].VertRef[Vtx]].PolyRef)
				{
				Vertices[Polygons[Count].VertRef[Vtx]].PolyRef[Vertices[Polygons[Count].VertRef[Vtx]].NumPolys] = Count;
				Vertices[Polygons[Count].VertRef[Vtx]].NumPolys ++;
				} // if
			} // for
		} // for
	} // if

if (! Success)
	UserMessageOK(Name, "Error reading Wavefront OBJ 3D Object file");
else
	{
	char NeedToMakeDefault = 0;
	if (WFNextMat == 0) // no material references (usemap or usemtl) found, create a default
		{
		NeedToMakeDefault = 1;
		WFNextMat = 1;
		} // if
	// Allocate NameTable
	if (NameTable = new ObjectMaterialEntry[WFNextMat])
		{
		NumMaterials = WFNextMat;
		int SurfMake, SurfScan;
		unsigned long FNLen;

		// try to read MTL (Material) file
		if ((FNLen = (unsigned long)strlen(filename)) > 4)
			{
			// does it look like it has a .TLA?
			if (filename[FNLen - 4] == '.')
				{
				// make working copy temporarily
				strcpy(WFOBJinBuf, filename);
				// substitute in .mtl TLA
				WFOBJinBuf[FNLen - 3] = 'm';
				WFOBJinBuf[FNLen - 2] = 't';
				WFOBJinBuf[FNLen - 1] = 'l';
				if (fMInput = PROJ_fopen(WFOBJinBuf, "r"))
					{
					// try to read it
					ProcessOBJMaterialFile(fMInput, NameTable, TextureTable, &LWTT, ObjFilePath);
					fclose(fMInput);
					fMInput = NULL;
					} // if
				} // if
			} // if

		// it's possible to not find any materials or maps, in which case we need to create a default
		// material
		if (NeedToMakeDefault)
			{
			NameTable[0].SetName("OBJ Default");
			GlobalApp->AppEffects->SetMakeMaterial(&NameTable[0], NULL); // create a material with default attributes
			} // if
		else
			{
			for (SurfScan = 0; SurfScan < WFNextMat; SurfScan++)
				{
				if ((SurfMake = LWTT.GetSurfID(SurfScan)) != -1)
					{ // Make it for real
					NameTable[SurfMake].SetName((char *)LWTT.GetEntryName(SurfScan));
					GlobalApp->AppEffects->SetMakeMaterial(&NameTable[SurfMake], &TextureTable[SurfScan]);
					} // if
				} // for
			} // else
		} // if
	} // else
if (TextureTable)
	delete [] TextureTable;

// only do this if the X Rot is still at the default of 0
if (RotateLikeXFrog && AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].CurValue == 0.0)
	{
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].SetCurValue(90.0);
	} // if

return (Success);

} // Object3DEffect::ReadWF_OBJFile


/*===========================================================================*/


short Object3DEffect::ProcessOBJMaterialFile(FILE *MTLFile, ObjectMaterialEntry *Names, MaterialAttributes *Mat, LWTagTable *TT, char *FilePath)
{
char *SkipSpace, *DataSkip;
int MaterialsMade = 0;
int CurMatlIdx = -1;
float FloatRed, FloatGrn, FloatBlu;

while (fgetline(WFOBJinBuf, 1000, MTLFile, 0, 0))
	{
	long TrimJunk;
	// trim off trailing junk (starts on NULL, but it's safe)
	for (TrimJunk = (long)strlen(WFOBJinBuf); TrimJunk > 0; TrimJunk--)
		{
		if ((WFOBJinBuf[TrimJunk] == NULL) || (WFOBJinBuf[TrimJunk] == 10) || (WFOBJinBuf[TrimJunk] == 13))
			{
			WFOBJinBuf[TrimJunk] = NULL;
			} // if
		else
			{
			// stop trimming, we've found real stuff
			break;
			} // else
		} // for

	// skip comment lines
	if (WFOBJinBuf[0] != '#')
		{
		SkipSpace = WFOBJinBuf;
		if (isspace(WFOBJinBuf[0]))
			{
			SkipSpace = SkipPastNextSpace(WFOBJinBuf);
			} // if
		if (SkipSpace)
			{
			if (!strnicmp(SkipSpace, "newmtl", 6))
				{ // create material/surface
				// "newmtl Body"
				if (DataSkip = SkipPastNextSpace(&SkipSpace[6]))
					{
					if ((CurMatlIdx = TT->FindEntryByName((unsigned char*)DataSkip)) != -1)
						{
						// set up useful defaults
						Mat[CurMatlIdx].Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
						Mat[CurMatlIdx].SmoothAngle = 89.9;
						} // if
					} // if
				} // if
			else if (!strnicmp(SkipSpace, "newmap", 6))
				{ // create imagemapped surface
				// "newmap Simple_copyMap cherry_leaf2.png"
				char *DataTwo;
				if (DataSkip = SkipPastNextSpace(&SkipSpace[6]))
					{
					if (DataTwo = SkipPastNextSpace(DataSkip))
						{
						DataTwo[-1] = NULL; // this is legal -- we're stomping on the space delimiter
						if ((CurMatlIdx = TT->FindEntryByName((unsigned char*)DataSkip)) != -1)
							{
							MaterialTextureInfo *MTI;
							// set up useful defaults
							Mat[CurMatlIdx].Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
							Mat[CurMatlIdx].SmoothAngle = 89.9;
							// ensure extended data block available
							if (MTI = Mat[CurMatlIdx].GetCreateMaterialTextureInfo(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
								{
								MTI->SetType(WCS_TEXTURE_TYPE_UVW); // was planar image
								MTI->SetUVName("Vertex Map"); // only one UV set available in WF-OBJ
								if (strchr(DataSkip, ':') || strchr(DataSkip, '/') || strchr(DataSkip, '\\'))
									{
									// move path part to FilePath from Name Part
									char PathDest[512], FileDest[512], NewPath[2048];
									BreakFileName(DataTwo, PathDest, 512, FileDest, 512);
									strmfp(NewPath, FilePath, PathDest);
									MTI->SetName(FileDest);
									if (FilePath) MTI->SetPath(NewPath);
									} // if
								else
									{
									MTI->SetName(DataTwo);
									if (FilePath) MTI->SetPath(FilePath);
									} // else
								} // if
							} // if
						} // if
					} // if
				} // if
			else if (CurMatlIdx != -1) // only process these if material identified
				{
				if (!strnicmp(SkipSpace, "map_Kd", 6))
					{ // diffuse texture
					if (DataSkip = SkipPastNextSpace(&SkipSpace[6]))
						{
						MaterialTextureInfo *MTI;
						// ensure extended data block available
						if (MTI = Mat[CurMatlIdx].GetCreateMaterialTextureInfo(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
							{
							MTI->SetType(WCS_TEXTURE_TYPE_UVW); // was planar image
							MTI->SetUVName("Vertex Map"); // only one UV set available in WF-OBJ
							if (strchr(DataSkip, ':') || strchr(DataSkip, '/') || strchr(DataSkip, '\\'))
								{
								// move path part to FilePath from Name Part
								char PathDest[512], FileDest[512], NewPath[2048];
								BreakFileName(DataSkip, PathDest, 512, FileDest, 512);
								strmfp(NewPath, FilePath, PathDest);
								MTI->SetName(FileDest);
								if (FilePath) MTI->SetPath(NewPath);
								} // if
							else
								{
								MTI->SetName(DataSkip);
								if (FilePath) MTI->SetPath(FilePath);
								} // else
							MTI->ClipMap = 1; // seems to be in vogue in XFrog Wavefront OBJs
							} // if
						} // if
					} // if
				else if (!strnicmp(SkipSpace, "map_Ks", 6))
					{ // specular texture
					} // if
				else if (!strnicmp(SkipSpace, "map_Ka", 6))
					{ // ambient texture
					} // if
				else if (!strnicmp(SkipSpace, "illum", 5))
					{ //  0 to disable lighting, 1 for ambient & diffuse only (specular color set to black), 2 for full lighting. 
					// currently ignored
					} // if
				else if (!strnicmp(SkipSpace, "Ns", 2))
					{ // Shininess (clamped to 1.0 - 128.0).
					// map to specular exponent
					if (DataSkip = SkipPastNextSpace(&SkipSpace[2]))
						{
						if (sscanf(DataSkip, "%f", &FloatRed) == 1)
							{
							// WCS clamps spec exponent to 100, Wavefront allows up to 128 (or 1000, according to some)
							Mat[CurMatlIdx].SpecExponent = WCS_min((double)FloatRed, 100.0);
							} // if
						} // if
					} // if
				else if (!strnicmp(SkipSpace, "Kd", 2))
					{ // diffuse values
					if (DataSkip = SkipPastNextSpace(&SkipSpace[2]))
						{
						if (sscanf(DataSkip, "%f %f %f", &FloatRed, &FloatGrn, &FloatBlu) == 3)
							{
							Mat[CurMatlIdx].UseFPGuns = 1;
							Mat[CurMatlIdx].FPRed   = FloatRed;
							Mat[CurMatlIdx].FPGreen = FloatGrn;
							Mat[CurMatlIdx].FPBlue  = FloatBlu;
							} // if
						} // if
					} // if
				else if (!strnicmp(SkipSpace, "Ks", 2))
					{ // specular values
					// WCS is monostimulus specularity, take max of RGB gun values
					if (DataSkip = SkipPastNextSpace(&SkipSpace[2]))
						{
						if (sscanf(DataSkip, "%f %f %f", &FloatRed, &FloatGrn, &FloatBlu) == 3)
							{
							Mat[CurMatlIdx].Specularity = WCS_max((double)FloatRed, WCS_max((double)FloatGrn, (double)FloatBlu));
							} // if
						} // if
					} // if
				else if (!strnicmp(SkipSpace, "Ka", 2))
					{ // ambient values
					// WCS is monostimulus ambient, take max of RGB gun values
					if (DataSkip = SkipPastNextSpace(&SkipSpace[2]))
						{
						if (sscanf(DataSkip, "%f %f %f", &FloatRed, &FloatGrn, &FloatBlu) == 3)
							{
							// This is commented out as it seems erroneous
							//Mat[CurMatlIdx].Luminosity = WCS_max(FloatRed, WCS_max(FloatGrn, FloatBlu));
							} // if
						} // if
					} // if
				else if (!strnicmp(SkipSpace, "d", 1))
					{ // dissolve -- transparency
					if (DataSkip = SkipPastNextSpace(&SkipSpace[1]))
						{
						if (sscanf(DataSkip, "%f", &FloatRed) == 1)
							{
							// Seems like dissolve is really inverse of dissolve.
							Mat[CurMatlIdx].Transparency = 1.0 - FloatRed;
							} // if
						} // if
					} // if
				} // else
			} // if
		} // if
	} // while


return(MaterialsMade);
} // Object3DEffect::ProcessOBJMaterialFile

/*===========================================================================*/



static double HashBlast3d[3];

unsigned long CalcHash3D(double IndexFloatA, double IndexFloatB, double IndexFloatC)
{
unsigned long NewHash = 0, *HashRead;

HashBlast3d[0] = IndexFloatA;
HashBlast3d[1] = IndexFloatB;
HashBlast3d[2] = IndexFloatC;

// XOR bits from all three doubles into a ULONG
HashRead = (unsigned long *)&HashBlast3d[0];
NewHash ^= *HashRead++; NewHash ^= *HashRead++;
NewHash ^= *HashRead++; NewHash ^= *HashRead++;
NewHash ^= *HashRead++; NewHash ^= *HashRead++;

// Now fold and smush the ULONG into however many bits matter
return((NewHash & 0xffff) ^ (NewHash >> 16));
} // CalcHash3D

/*===========================================================================*/

short Object3DEffect::ProcessDXFInput(struct DXFImportPolygon *DXFPolygons, VectorPoint *DXFPoints, unsigned long NumDXFPolys, unsigned long NumDXFPts, short CheckRedundancy)
{
VectorPoint **VecPtSource, **VecPtDest, *PLinkA;
long Pt, Poly, Index[4], NameIndex = 0, RefCt, VLink; //, Exceed, Reduce;
short SkipIt, ValidVerts, PolyVert, Success = 1, *VertRef;
char **MaterialNames, LinkOk;
unsigned long HashPos;
signed long *VecPtHash, *VecPtCollide;
BusyWin *Position = NULL;

if (NumDXFPolys > 0 && NumDXFPts > 0)
	{
	FreePolygons();
	VecPtSource = NULL; VecPtDest = NULL; VertRef = NULL; MaterialNames = NULL;
	VecPtHash = VecPtCollide = NULL;

	Polygons = new Polygon3D[NumDXFPolys];
	VecPtSource = (VectorPoint **)AppMem_Alloc(NumDXFPts * sizeof(VectorPoint *), 0);
	VecPtDest = (VectorPoint **)AppMem_Alloc(NumDXFPts * sizeof(VectorPoint *), 0);

	if (CheckRedundancy)
		{
		VecPtHash		= (long *)AppMem_Alloc(0x10000 * sizeof(long), 0);
		VecPtCollide	= (long *)AppMem_Alloc(NumDXFPts * sizeof(long), 0);
		} // if

	VertRef = (short *)AppMem_Alloc(NumDXFPts * sizeof(short), APPMEM_CLEAR);
	MaterialNames = (char **)AppMem_Alloc(1000 * sizeof(char *), 0);
	if (Polygons && VecPtSource && VecPtDest && VertRef && MaterialNames)
		{
		Position = new BusyWin ("Process DXF", NumDXFPolys, 'IDLG', 0);
		for (Pt = 0, PLinkA = DXFPoints; Pt < (long)NumDXFPts && PLinkA; Pt ++, PLinkA = PLinkA->Next)
			{
			VecPtSource[Pt] = PLinkA;
			} // for
		if (CheckRedundancy)
			{
			// Clear Hash and Collision Tables
			for (Pt = 0; Pt < 0x10000; Pt++) VecPtHash[Pt] = -1;
			for (Pt = 0; Pt < (signed)NumDXFPts; Pt++) VecPtCollide[Pt] = -1;
			} // if
		//Reduce = 0;
		for (Poly = 0; Poly < (long)NumDXFPolys; Poly ++)
			{
			ValidVerts = 0;
			for (PolyVert = 0; PolyVert < 4; PolyVert ++)
				{
				if (DXFPolygons[Poly].Index[PolyVert] > 0)
					{
					Index[PolyVert] = DXFPolygons[Poly].Index[PolyVert] - 1;
					SkipIt = 0;
					if (CheckRedundancy)
						{
						HashPos = (CalcHash3D(VecPtSource[Index[PolyVert]]->Latitude,
						 VecPtSource[Index[PolyVert]]->Longitude, (double)VecPtSource[Index[PolyVert]]->Elevation) & 0xffff);
						if (VecPtHash[HashPos] != -1)
							{
							for (Pt = (long)VecPtHash[HashPos]; Pt != -1; Pt = VecPtCollide[Pt])
								{
								if (VecPtSource[Index[PolyVert]]->Latitude  == VecPtDest[Pt]->Latitude && 
									VecPtSource[Index[PolyVert]]->Longitude == VecPtDest[Pt]->Longitude && 
									VecPtSource[Index[PolyVert]]->Elevation == VecPtDest[Pt]->Elevation)
									{
									//sprintf(DXFMsg, "Reduced: %d:%d\n", Poly, PolyVert);
									//OutputDebugStr(DXFMsg);
									//Reduce++;
									SkipIt = 1;
									Index[PolyVert] = Pt;
									break;
									} // if
								else
									{
									if (VecPtCollide[Pt] == -1)
										{
										VecPtCollide[Pt] = NumVertices;
										break;
										} // if
									} // else
								} // if
							} // if
						else
							{
							VecPtHash[HashPos] = NumVertices;
							} // else
						} // if
					else if (Index[PolyVert] < NumVertices)
						SkipIt = 1;
					if (! SkipIt)
						{
						VecPtDest[NumVertices] = VecPtSource[Index[PolyVert]];
						Index[PolyVert] = NumVertices;
						NumVertices ++;
						} // if
					ValidVerts ++;
					} // if
				else
					{
					break;
					} // else found the end of vertices for this polygon
				} // for PolyVert 0-3
			if (ValidVerts)
				{
				if ((Polygons[NumPolys].VertRef = (long *)AppMem_Alloc(ValidVerts * sizeof(long), 0)) == NULL)
					{
					Success = 0;
					break;
					} // if
				Polygons[NumPolys].NumVerts = ValidVerts;
				for (PolyVert = 0; PolyVert < ValidVerts; PolyVert ++)
					{
					Polygons[NumPolys].VertRef[PolyVert] = Index[PolyVert];
					VertRef[Index[PolyVert]] ++;
					} // for
				SkipIt = 0;
				for (Pt = 0; Pt < NumMaterials; Pt ++)
					{
					if (! strcmp(MaterialNames[Pt], DXFPolygons[Poly].MaterialStr))
						{
						SkipIt = 1;
						NameIndex = Pt;
						} // if
					} // for
				if (! SkipIt)
					{
					MaterialNames[NumMaterials] = DXFPolygons[Poly].MaterialStr;
					NameIndex = NumMaterials;
					NumMaterials ++;
					} // if
				Polygons[NumPolys].Material = NameIndex;
				NumPolys ++;
				} // if
			if (Position)
				{
				Position->Update(Poly + 1);
				if (Position->CheckAbort())
					{
					Success = 0;
					break;
					} // if
				} // if
			} // for - de main processing loop
		//sprintf(DXFMsg, "Total Reduced: %d.\n", Reduce);
		//OutputDebugStr(DXFMsg);

		if (Success && NumVertices > 0 && (Vertices = new Vertex3D[NumVertices]))
			{
			// Allocate and initialize vertex backref links to -1
			for (Pt = 0; Pt < NumVertices; Pt ++)
				{
				Vertices[Pt].xyz[0] = VecPtDest[Pt]->Longitude;	// x
				Vertices[Pt].xyz[1] = VecPtDest[Pt]->Elevation;	// y
				Vertices[Pt].xyz[2] = VecPtDest[Pt]->Latitude;	// z
				if (VertRef[Pt] > 0)
					{
					if ((Vertices[Pt].PolyRef = (long *)AppMem_Alloc(VertRef[Pt] * sizeof(long), APPMEM_CLEAR)) == NULL)
						{
						Success = 0;
						break;
						} // if
					Vertices[Pt].NumPolys = VertRef[Pt];
					for (Poly = 0; Poly < VertRef[Pt]; Poly ++)
						{
						Vertices[Pt].PolyRef[Poly] = -1;
						} // for
					} // if
				} // for
			// Create the Vertex->Polygon links
			//Exceed = 0;
			for (Poly = 0; Poly < NumPolys; Poly ++)
				{
				if (Position)
					{
					Position->Update(Poly + 1);
					if (Position->CheckAbort())
						{
						Success = 0;
						break;
						} // if
					} // if
				for (PolyVert = 0; PolyVert < Polygons[Poly].NumVerts; PolyVert ++)
					{
					VLink = Polygons[Poly].VertRef[PolyVert];
					for (LinkOk = 0, RefCt = 0; RefCt < VertRef[VLink]; RefCt++)
						{
						if (Vertices[VLink].PolyRef[RefCt] == -1)
							{
							Vertices[VLink].PolyRef[RefCt] = Poly;
							LinkOk = 1;
							break;
							} // if
						} // if
					if (!LinkOk)
						{
						// problem -- no space for link!
						//UserMessageOK("DXF Relink", "Out of PolyRef slots.");
						//sprintf(DXFMsg, "Exceed: %d:%d.\n", Poly, PolyVert);
						//OutputDebugStr(DXFMsg);
						//Exceed++;
						} // if
					} // for
				} // for
			//if (Exceed)
			//	UserMessageOK("DXF Relink", "Out of PolyRef slots.");

			// Check to make sure no -1 unused slots remain
			for (RefCt = Pt = 0; Pt < NumVertices; Pt ++)
				{
				if (VertRef[Pt] > 0)
					{
					for (Poly = 0; Poly < VertRef[Pt]; Poly ++)
						{
						if (Vertices[Pt].PolyRef[Poly] == -1)
							{
							RefCt++;
							} // if
						} // for
					} // if
				} // for
			//if (RefCt) UserMessageOK("DXF Relink", "Unfilled PolyRef slots!");
			} // if
		else
			Success = 0;
		if (Success && NumMaterials > 0 && (NameTable = new ObjectMaterialEntry[NumMaterials]))
			{
			for (Pt = 0; Pt < NumMaterials; Pt ++)
				{
				NameTable[Pt].SetName(MaterialNames[Pt]);
				} // for
			} // if
		else
			Success = 0;
		if (Position)
			delete Position;
		} // if all is well
	else
		{
		Success = 0;
		} // else

	if (VecPtHash)		AppMem_Free(VecPtHash, 0x10000 * sizeof(signed long)); VecPtHash = NULL;
	if (VecPtCollide)	AppMem_Free(VecPtCollide, NumDXFPts * sizeof(signed long)); VecPtCollide = NULL;

	if (MaterialNames)	AppMem_Free(MaterialNames, 1000 * sizeof(char *)); MaterialNames = NULL;
	if (VertRef)		AppMem_Free(VertRef, NumDXFPts * sizeof(short)); VertRef = NULL;
	if (VecPtDest)		AppMem_Free(VecPtDest, NumDXFPts * sizeof(VectorPoint *)); VecPtDest = NULL;
	if (VecPtSource)	AppMem_Free(VecPtSource, NumDXFPts * sizeof(VectorPoint *)); VecPtSource = NULL;

	} // if

if (! Success)
	UserMessageOK(Name, "Error reading DXF Object file");
else
	{
	for (Pt = 0; Pt < NumMaterials; Pt ++)
		{
		GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Pt], NULL);
		} // for
	} // else
return (Success);

} // Object3DEffect::ProcessDXFInput

/*===========================================================================*/

char *Object3DEffect::GetNextName(char *Current, char *MaxPtr)
{

if (Current && MaxPtr)
	{
	while (++Current < MaxPtr && *Current != 0);	//lint !e722
	while (++Current < MaxPtr && *Current == 0);	//lint !e722
	return (Current < MaxPtr ? Current: NULL);
	} // if

return (NULL);

} // Object3DEffect::GetNextName

/*===========================================================================*/

void Object3DEffect::CopyCoordinatesToXYZ(void)
{
long VertCt;

if (Vertices)
	{
	for (VertCt = 0; VertCt < NumVertices; ++VertCt)
		{
		Vertices[VertCt].XYZ[0] = Vertices[VertCt].xyz[0];
		Vertices[VertCt].XYZ[1] = Vertices[VertCt].xyz[1];
		Vertices[VertCt].XYZ[2] = Vertices[VertCt].xyz[2];
		} // for
	} // if

} // Object3DEffect::CopyCoordinatesToXYZ

/*===========================================================================*/

// this version of Transform has all the transformation parameters already figured out
int Object3DEffect::TransformToWCSDefGeographic(RenderData *Rend, Object3DInstance *CurInstance)
{
long VertCt;
VertexDEM Vert;
Matx4x4 LocalMatx, TempWorldMatx, TotalMatx;

// Build rotation matrices
//CurInstance->WCSGeographic[0] = longitude of object origin in WCS convention
//CurInstance->WCSGeographic[1] = latitude of object origin
//CurInstance->WCSGeographic[2] = elevation in meters of object origin

// rotations are in CurInstance->Euler
// CurInstance->Euler[0] = x = pitch
// CurInstance->Euler[1] = y = heading
// CurInstance->Euler[2] = z = bank

if (Vertices)
	{
	Vert.Lat = CurInstance->WCSGeographic[1];
	Vert.Lon = CurInstance->WCSGeographic[0];
	Vert.Elev = CurInstance->WCSGeographic[2];
	#ifdef WCS_BUILD_VNS
	Rend->DefCoords->DegToCart(&Vert);
	#else // WCS_BUILD_VNS
	Vert.DegToCart(Rend->PlanetRad);
	#endif // WCS_BUILD_VNS

	BuildTransformationMatrix4x4(
		0.0, 0.0, 0.0,
		CurInstance->Scale[0], CurInstance->Scale[1], CurInstance->Scale[2],
		-CurInstance->Euler[0] * PiOver180, -CurInstance->Euler[1] * PiOver180, -CurInstance->Euler[2] * PiOver180, LocalMatx);

	BuildTransformationMatrix4x4(-Vert.XYZ[0], -Vert.XYZ[1], -Vert.XYZ[2], 1.0, 1.0, 1.0, 
		(90.0 - Vert.Lat) * PiOver180, -Vert.Lon * PiOver180, 0.0, TempWorldMatx);

	Multiply4x4Matrices(TempWorldMatx, LocalMatx, TotalMatx);

	for (VertCt = 0; VertCt < NumVertices; ++VertCt)
		{
		Vertices[VertCt].XYZ[0] = Vertices[VertCt].xyz[0];
		Vertices[VertCt].XYZ[1] = Vertices[VertCt].xyz[1];
		Vertices[VertCt].XYZ[2] = Vertices[VertCt].xyz[2];
		Transform3DPoint(&Vertices[VertCt], TotalMatx);
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->CartToDeg(&Vertices[VertCt]);
		#else // WCS_BUILD_VNS
		Vertices[VertCt].CartToDeg(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		} // for
	return (1);
	} // if

return (0);

} // Object3DEffect::Transform

/*===========================================================================*/

int Object3DEffect::Transform(Vertex3D *LocalVertices, long NumLocalVertices, RenderData *Rend, PolygonData *Poly, 
	VertexDEM *BaseVtx, double ExtraRotation[3], double Height)
{
long Count, NVerts, NeedTransformedVertexForTextures;
double HeightScale, UnitScale, Rotation[3], Translation[3], Scale[3], GroupVal, AxisVal, TexOpacity, Value[3], 
	Heading, Pitch, Bank, TerrainAzimuth, TerrainSlope, XRot, YRot, ZRot;
Joe *AlignToMe;
Vertex3D *TVerts;
Matx4x4 LocalMatx, InvLocalMatx, TempWorldMatx, InvTempWorldMatx, TempMatx, AxisAlignMatx, InvAxisAlignMatx;
Matx3x3 LocalNormalMatx, TempWorldNormalMatx, AxisAlignNormalMatx, TempMatx3x3;
VertexDEM Vert;

// seed random number generator
Rand.Seed64BitShift(Poly->LatSeed, Poly->LonSeed);

if (Vertices || LocalVertices)
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	XRot = YRot = ZRot = Heading = Pitch = Bank = TerrainAzimuth = TerrainSlope = 0.0;
	if (AlignHeading || AlignVertical)
		{
		if (AlignHeading || (AlignVertVec == WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR))
			{
			if (AlignToMe = AlignVec && AlignSpecialVec ? AlignVec: Poly->Vector)
				{
				AlignToMe->HeadingAndPitch(BaseVtx->Lat, BaseVtx->Lon, Rend->EarthLatScaleMeters, Heading, Pitch);
				} // if
			} // if
		if (AlignVertical && (AlignVertVec == WCS_EFFECTS_OBJECT3D_ALIGNVERT_TERRAIN))
			{
			Vert.XYZ[0] = Poly->Normal[0];
			Vert.XYZ[1] = Poly->Normal[1];
			Vert.XYZ[2] = Poly->Normal[2];
			Vert.Lat = BaseVtx->Lat;
			Vert.Lon = BaseVtx->Lon;
			Vert.Elev = BaseVtx->Elev;
			Vert.RotateToHome();
			Vert.RotateY(-Heading);
			Pitch = Vert.FindAngleXfromY();
			Vert.RotateX(-Pitch);
			Bank = Vert.FindAngleZfromY();
			} // if
		if (AlignHeading)
			{
			if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
				{
				if (AlignVertical)
					{
					if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
						{
						if (ReverseHeading)		// negative X axis
							YRot = 90.0 * PiOver180;
						else
							YRot = -90.0 * PiOver180;
						} // if
					else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
						{
						if (ReverseHeading)		// negative X axis
							YRot = 90.0 * PiOver180;
						else
							YRot = -90.0 * PiOver180;
						ZRot = -90.0 * PiOver180;
						} // if
					} // if
				else
					{
					if (ReverseHeading)		// negative X axis
						YRot = 90.0 * PiOver180;
					else
						YRot = -90.0 * PiOver180;
					} // else
				} // if
			else if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
				{
				if (AlignVertical)
					{
					if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
						{
						if (ReverseHeading)		// negative Y axis
							YRot = -90.0 * PiOver180;
						else
							YRot = 90.0 * PiOver180;
						ZRot = 90.0 * PiOver180;
						} // if
					else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
						{
						if (! ReverseHeading)		// negative Y axis
							YRot = 180.0 * PiOver180;
						XRot = -90.0 * PiOver180;
						} // if
					} // if
				else
					{
					if (ReverseHeading)		// negative Y axis
						XRot = -90.0 * PiOver180;
					else
						XRot = 90.0 * PiOver180;
					} // else
				} // if
			else if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
				{
				if (AlignVertical)
					{
					if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
						{
						if (ReverseHeading)		// negative Z axis
							YRot = 180.0 * PiOver180;
						ZRot = 90.0 * PiOver180;
						} // if
					else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
						{
						if (ReverseHeading)		// negative Z axis
							YRot = 180.0 * PiOver180;
						} // if
					} // if
				else
					{
					if (ReverseHeading)		// negative Z axis
						YRot = 180.0 * PiOver180;
					} // else
				} // if
			} // if
		else if (AlignVertical)
			{
			if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
				{
				ZRot = 90.0 * PiOver180;
				} // if
			else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
				{
				XRot = -90.0 * PiOver180;
				} // if
			} // if
		} // if

	if (Units != WCS_USEFUL_UNIT_METER)
		{
		UnitScale = ConvertToMeters(1.0, Units);
		} 
	else
		UnitScale = 1.0;
	if (Height >= 0.0)
		{
		HeightScale = fabs(ObjectBounds[2] - ObjectBounds[3]) * UnitScale;
		HeightScale = Height / HeightScale;
		} // if
	else
		HeightScale = 1.0;

	if (AlignVertical)
		{
		Pitch *= AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue;
		Bank *= AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue;
		} // if
	else
		{
		Pitch = 0.0;
		Bank = 0.0;
		} // else
	if (! AlignHeading)
		Heading = 0.0;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGX]->Eval(Value, Poly->Vector))
		Scale[0] = Value[0] * .01;
	else
		Scale[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGY]->Eval(Value, Poly->Vector))
		Scale[1] = Value[0] * .01;
	else
		Scale[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ]->Eval(Value, Poly->Vector))
		Scale[2] = Value[0] * .01;
	else
		Scale[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX]->Eval(Value, Poly->Vector))
		Rotation[0] = Value[0];
	else
		Rotation[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY]->Eval(Value, Poly->Vector))
		Rotation[1] = Value[0];
	else
		Rotation[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ]->Eval(Value, Poly->Vector))
		Rotation[2] = Value[0];
	else
		Rotation[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX]->Eval(Value, Poly->Vector))
		Translation[0] = Value[0];
	else
		Translation[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY]->Eval(Value, Poly->Vector))
		Translation[1] = Value[0];
	else
		Translation[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].CurValue;

	if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ]->Eval(Value, Poly->Vector))
		Translation[2] = Value[0];
	else
		Translation[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].CurValue;

	// restore for later use
	Value[0] = Value[1] = Value[2] = 0.0;

	if (! CalcNormals)
		Scale[0] = Scale[1] = Scale[2] = Scale[0] * UnitScale * HeightScale;
	else
		{
		Scale[0] *= UnitScale * HeightScale;
		Scale[1] *= UnitScale * HeightScale;
		Scale[2] *= UnitScale * HeightScale;
		} // else
	Rotation[0] += Pitch;
	Rotation[1] += Heading;
	Rotation[2] += Bank;
	if (ExtraRotation)
		{
		Rotation[0] += ExtraRotation[0];
		Rotation[1] += ExtraRotation[1];
		Rotation[2] += ExtraRotation[2];
		} // if

	// transfer texture data
	Rend->TransferTextureData(Poly);
	Rend->TexData.Elev = BaseVtx->Elev;
	Rend->TexData.Latitude = BaseVtx->Lat;
	Rend->TexData.Longitude = BaseVtx->Lon;
	Rend->TexData.ZDist = BaseVtx->ScrnXYZ[2];

	// handle randomization
	if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_SCALE])
		{
		// if texture for group
		GroupVal = 1.0;
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					GroupVal += (Value[0] - GroupVal * TexOpacity);
					} // if
				else
					GroupVal = Value[0];
				} // if
			} // if

		// if texture for X
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Scale[0] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].CurValue);

		if (! Isometric)
			{
			// if texture for Y
			AxisVal = Rand.GenPRN();
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						AxisVal += (Value[0] - AxisVal * TexOpacity);
						} // if
					else
						AxisVal = Value[0];
					} // if
				} // if
			Scale[1] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].CurValue);

			// if texture for Z
			AxisVal = Rand.GenPRN();
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						AxisVal += (Value[0] - AxisVal * TexOpacity);
						} // if
					else
						AxisVal = Value[0];
					} // if
				} // if
			Scale[2] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].CurValue);
			} // if ! Isometric
		else
			{
			Scale[1] = Scale[2] = Scale[0];
			Rand.GenPRN();
			Rand.GenPRN();
			} // else
		} // if
	else
		{
		Rand.GenPRN();
		Rand.GenPRN();
		Rand.GenPRN();
		} // else keep random # in step
	if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_ROTATE])
		{
		// if texture for group
		GroupVal = 1.0;
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					GroupVal += (Value[0] - GroupVal * TexOpacity);
					} // if
				else
					GroupVal = Value[0];
				} // if
			} // if

		// if texture for X
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Rotation[0] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].CurValue;

		// if texture for Y
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Rotation[1] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].CurValue;

		// if texture for Z
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Rotation[2] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].CurValue;
		} // if
	else
		{
		Rand.GenPRN();
		Rand.GenPRN();
		Rand.GenPRN();
		} // else keep random # in step
	if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_POSITION])
		{
		// if texture for group
		GroupVal = 1.0;
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					GroupVal += (Value[0] - GroupVal * TexOpacity);
					} // if
				else
					GroupVal = Value[0];
				} // if
			} // if

		// if texture for X
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Translation[0] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].CurValue;

		// if texture for Y
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Translation[1] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].CurValue;

		// if texture for Z
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Translation[2] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].CurValue;
		} // if
	else
		{
		Rand.GenPRN();
		Rand.GenPRN();
		Rand.GenPRN();
		} // else keep random # in step

	// notes about rotation:
	// The direction of rotation with the signs used here give the same results as LightWave for x (pitch) and y(heading).
	// However the sign for z (bank) is reversed from LightWave so as to conform with the WCS Camera Bank direction
	// which to me seems more intuitive than having it conform strictly with mathematical convention which says
	// that positive rotation is counterclockwise when looking from the negative end of an axis. Trouble is we normally
	// look down from above (from +y), from the right (+) end of x, and from the front (-) end of the z axis.
	// The rotation signs given here will result in clockwise rotation from all of these vantage points.
	// V5 addendum: the rotation about the z axis is now consistent with other axes and does not require
	// that the z rotation angle be negated.

	// I think this makes a transformation matrix that performs rotation in the wrong order for
	// 3D objects which should be z then x then y - fixed 3/5/01 now it is ZXY as it should be.
	BuildTransformationMatrix4x4(
		-Translation[0], -Translation[1], -Translation[2],
		Scale[0], Scale[1], Scale[2],
		-Rotation[0] * PiOver180, -Rotation[1] * PiOver180, -Rotation[2] * PiOver180, LocalMatx);
	BuildRotationMatrix(
		-Rotation[0] * PiOver180, -Rotation[1] * PiOver180, -Rotation[2] * PiOver180, LocalNormalMatx);
	BuildInvTransformationMatrix4x4(
		-Translation[0], -Translation[1], -Translation[2],
		Scale[0], Scale[1], Scale[2],
		-Rotation[0] * PiOver180, -Rotation[1] * PiOver180, -Rotation[2] * PiOver180, InvLocalMatx);

	BuildTransformationMatrix4x4(-BaseVtx->XYZ[0], -BaseVtx->XYZ[1], -BaseVtx->XYZ[2], 1.0, 1.0, 1.0, 
		(90.0 - BaseVtx->Lat) * PiOver180, -BaseVtx->Lon * PiOver180, 0.0, TempWorldMatx);
	BuildRotationMatrix((90.0 - BaseVtx->Lat) * PiOver180, -BaseVtx->Lon * PiOver180, 0.0, TempWorldNormalMatx);
	BuildInvTransformationMatrix4x4(-BaseVtx->XYZ[0], -BaseVtx->XYZ[1], -BaseVtx->XYZ[2], 1.0, 1.0, 1.0, 
		(90.0 - BaseVtx->Lat) * PiOver180, -BaseVtx->Lon * PiOver180, 0.0, InvTempWorldMatx);

	BuildTransformationMatrix4x4(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, -XRot, -YRot, -ZRot, AxisAlignMatx);
	BuildRotationMatrix(-XRot, -YRot, -ZRot, AxisAlignNormalMatx);
	BuildInvTransformationMatrix4x4(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, -XRot, -YRot, -ZRot, InvAxisAlignMatx);

	Multiply4x4Matrices(LocalMatx, AxisAlignMatx, TempMatx);
	Multiply4x4Matrices(TempWorldMatx, TempMatx, WorldMatx);

	Multiply3x3Matrices(LocalNormalMatx, AxisAlignNormalMatx, TempMatx3x3);
	Multiply3x3Matrices(TempWorldNormalMatx, TempMatx3x3, WorldNormalMatx);

	Multiply4x4Matrices(InvLocalMatx, InvTempWorldMatx, TempMatx);
	Multiply4x4Matrices(InvAxisAlignMatx, TempMatx, InvWorldMatx);

	NeedTransformedVertexForTextures =
		((TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION]->Enabled)
		|| (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX]->Enabled)
		|| (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY]->Enabled)
		|| (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ]->Enabled));

	if (LocalVertices)
		{
		TVerts = LocalVertices;
		NVerts = NumLocalVertices;
		} // if
	else
		{
		TVerts = Vertices;
		NVerts = NumVertices;
		} // else
	for (Count = 0; Count < NVerts; Count ++)
		{
		TVerts[Count].XYZ[0] = TVerts[Count].xyz[0];
		TVerts[Count].XYZ[1] = TVerts[Count].xyz[1];
		TVerts[Count].XYZ[2] = TVerts[Count].xyz[2];

		if (RandomizeVert)
			{
			if (NeedTransformedVertexForTextures)
				{
				Transform3DPoint(&TVerts[Count], WorldMatx);
				if (Rend->DefCoords)
					Rend->DefCoords->CartToDeg(&TVerts[Count]);
				Rend->TransferTextureData(&TVerts[Count]);
				} // if
			//Rend->TexData.LowX = Rend->TexData.HighX = TVerts[Count].XYZ[0];
			//Rend->TexData.LowY = Rend->TexData.HighY = TVerts[Count].XYZ[1];
			//Rend->TexData.LowZ = Rend->TexData.HighZ = TVerts[Count].XYZ[2];
			//Rend->TexData.Elev = BaseVtx->Elev + TVerts[Count].XYZ[1] * UnitScale * HeightScale;
			Rend->TexData.WaterDepth = Poly->WaterElev - Rend->TexData.Elev;

			// if texture for group
			GroupVal = 1.0;
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						GroupVal += (Value[0] - GroupVal * TexOpacity);
						} // if
					else
						GroupVal = Value[0];
					} // if
				} // if

			// if texture for X
			AxisVal = Rand.GenPRN();
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						AxisVal += (Value[0] - AxisVal * TexOpacity);
						} // if
					else
						AxisVal = Value[0];
					} // if
				} // if
			TVerts[Count].XYZ[0] = TVerts[Count].xyz[0] + AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].CurValue;

			// if texture for Y
			AxisVal = Rand.GenPRN();
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						AxisVal += (Value[0] - AxisVal * TexOpacity);
						} // if
					else
						AxisVal = Value[0];
					} // if
				} // if
			TVerts[Count].XYZ[1] = TVerts[Count].xyz[1] + AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].CurValue;

			// if texture for Z
			AxisVal = Rand.GenPRN();
			if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ]->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						AxisVal += (Value[0] - AxisVal * TexOpacity);
						} // if
					else
						AxisVal = Value[0];
					} // if
				} // if
			TVerts[Count].XYZ[2] = TVerts[Count].xyz[2] + AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].CurValue;
			} // if

		Transform3DPoint(&TVerts[Count], WorldMatx);
		} // for
	return (1);
	} // if

return (0);

} // Object3DEffect::Transform

/*===========================================================================*/

void Object3DEffect::CalcTransformParams(RenderData *Rend, PolygonData *Poly, 
	VertexDEM *BaseVtx, double ExtraRotation[3], double Height, 
	Point3d OutRotation, Point4d OutQuaternion, Point3d OutTranslation, Point3d OutScale)
{
Point4d Q1, Q2;
double HeightScale, UnitScale, Rotation[3], Translation[3], Scale[3], GroupVal, AxisVal, TexOpacity, Value[3], 
	Heading, Pitch, Bank, TerrainAzimuth, TerrainSlope, XRot, YRot, ZRot;
Joe *AlignToMe;
VertexDEM Vert;

// seed random number generator
Rand.Seed64BitShift(Poly->LatSeed, Poly->LonSeed);

Value[0] = Value[1] = Value[2] = 0.0;
XRot = YRot = ZRot = Heading = Pitch = Bank = TerrainAzimuth = TerrainSlope = 0.0;
if (AlignHeading || AlignVertical)
	{
	if (AlignHeading || (AlignVertVec == WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR))
		{
		if (AlignToMe = AlignVec && AlignSpecialVec ? AlignVec: Poly->Vector)
			{
			AlignToMe->HeadingAndPitch(BaseVtx->Lat, BaseVtx->Lon, Rend->EarthLatScaleMeters, Heading, Pitch);
			} // if
		} // if
	if (AlignVertical && (AlignVertVec == WCS_EFFECTS_OBJECT3D_ALIGNVERT_TERRAIN))
		{
		Vert.XYZ[0] = Poly->Normal[0];
		Vert.XYZ[1] = Poly->Normal[1];
		Vert.XYZ[2] = Poly->Normal[2];
		Vert.Lat = BaseVtx->Lat;
		Vert.Lon = BaseVtx->Lon;
		Vert.Elev = BaseVtx->Elev;
		Vert.RotateToHome();
		Pitch = Vert.FindAngleXfromY();
		Vert.RotateX(-Pitch);
		Bank = Vert.FindAngleZfromY();
		} // if
	if (AlignHeading)
		{
		if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
			{
			if (AlignVertical)
				{
				if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
					{
					if (ReverseHeading)		// negative X axis
						YRot = 90.0;
					else
						YRot = -90.0;
					} // if
				else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
					{
					if (ReverseHeading)		// negative X axis
						YRot = 90.0;
					else
						YRot = -90.0;
					ZRot = -90.0;
					} // if
				} // if
			else
				{
				if (ReverseHeading)		// negative X axis
					YRot = 90.0;
				else
					YRot = -90.0;
				} // else
			} // if
		else if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
			{
			if (AlignVertical)
				{
				if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
					{
					if (ReverseHeading)		// negative Y axis
						YRot = -90.0;
					else
						YRot = 90.0;
					ZRot = 90.0;
					} // if
				else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
					{
					if (! ReverseHeading)		// negative Y axis
						YRot = 180.0;
					XRot = -90.0;
					} // if
				} // if
			else
				{
				if (ReverseHeading)		// negative Y axis
					XRot = -90.0;
				else
					XRot = 90.0;
				} // else
			} // if
		else if (HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
			{
			if (AlignVertical)
				{
				if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
					{
					if (ReverseHeading)		// negative Z axis
						YRot = 180.0;
					ZRot = 90.0;
					} // if
				else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y)
					{
					if (ReverseHeading)		// negative Z axis
						YRot = 180.0;
					} // if
				} // if
			else
				{
				if (ReverseHeading)		// negative Z axis
					YRot = 180.0;
				} // else
			} // if
		} // if
	else if (AlignVertical)
		{
		if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X)
			{
			ZRot = 90.0;
			} // if
		else if (VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z)
			{
			XRot = -90.0;
			} // if
		} // if
	} // if

if (Units != WCS_USEFUL_UNIT_METER)
	{
	UnitScale = ConvertToMeters(1.0, Units);
	} 
else
	UnitScale = 1.0;
if (Height >= 0.0)
	{
	HeightScale = fabs(ObjectBounds[2] - ObjectBounds[3]) * UnitScale;
	HeightScale = Height / HeightScale;
	} // if
else
	HeightScale = 1.0;

if (AlignVertical)
	{
	Pitch *= AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue;
	Bank *= AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue;
	} // if
else
	{
	Pitch = 0.0;
	Bank = 0.0;
	} // else
if (! AlignHeading)
	Heading = 0.0;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGX]->Eval(Value, Poly->Vector))
	Scale[0] = Value[0] * .01;
else
	Scale[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGY]->Eval(Value, Poly->Vector))
	Scale[1] = Value[0] * .01;
else
	Scale[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ]->Eval(Value, Poly->Vector))
	Scale[2] = Value[0] * .01;
else
	Scale[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX]->Eval(Value, Poly->Vector))
	Rotation[0] = Value[0];
else
	Rotation[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY]->Eval(Value, Poly->Vector))
	Rotation[1] = Value[0];
else
	Rotation[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ]->Eval(Value, Poly->Vector))
	Rotation[2] = Value[0];
else
	Rotation[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX]->Eval(Value, Poly->Vector))
	Translation[0] = Value[0];
else
	Translation[0] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY]->Eval(Value, Poly->Vector))
	Translation[1] = Value[0];
else
	Translation[1] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].CurValue;

if (GetEnabledTheme(WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ) && Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ]->Eval(Value, Poly->Vector))
	Translation[2] = Value[0];
else
	Translation[2] = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].CurValue;

// restore for later use
Value[0] = Value[1] = Value[2] = 0.0;

if (! CalcNormals)
	Scale[0] = Scale[1] = Scale[2] = Scale[0] * UnitScale * HeightScale;
else
	{
	Scale[0] *= UnitScale * HeightScale;
	Scale[1] *= UnitScale * HeightScale;
	Scale[2] *= UnitScale * HeightScale;
	} // else
Rotation[0] += Pitch;
Rotation[1] += Heading;
Rotation[2] += Bank;
if (ExtraRotation)
	{
	Rotation[0] += ExtraRotation[0];
	Rotation[1] += ExtraRotation[1];
	Rotation[2] += ExtraRotation[2];
	} // if

// transfer texture data
Rend->TransferTextureData(Poly);
Rend->TexData.Elev = BaseVtx->Elev;
Rend->TexData.Latitude = BaseVtx->Lat;
Rend->TexData.Longitude = BaseVtx->Lon;

// handle randomization
if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_SCALE])
	{
	// if texture for group
	GroupVal = 1.0;
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				GroupVal += (Value[0] - GroupVal * TexOpacity);
				} // if
			else
				GroupVal = Value[0];
			} // if
		} // if

	// if texture for X
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Scale[0] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].CurValue);

	if (! Isometric)
		{
		// if texture for Y
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Scale[1] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].CurValue);

		// if texture for Z
		AxisVal = Rand.GenPRN();
		if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Eval(Value, &Rend->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					AxisVal += (Value[0] - AxisVal * TexOpacity);
					} // if
				else
					AxisVal = Value[0];
				} // if
			} // if
		Scale[2] *= (AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].CurValue);
		} // if ! Isometric
	else
		{
		Scale[1] = Scale[2] = Scale[0];
		Rand.GenPRN();
		Rand.GenPRN();
		} // else
	} // if
else
	{
	Rand.GenPRN();
	Rand.GenPRN();
	Rand.GenPRN();
	} // else keep random # in step
if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_ROTATE])
	{
	// if texture for group
	GroupVal = 1.0;
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				GroupVal += (Value[0] - GroupVal * TexOpacity);
				} // if
			else
				GroupVal = Value[0];
			} // if
		} // if

	// if texture for X
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Rotation[0] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].CurValue;

	// if texture for Y
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Rotation[1] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].CurValue;

	// if texture for Z
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Rotation[2] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].CurValue;
	} // if
else
	{
	Rand.GenPRN();
	Rand.GenPRN();
	Rand.GenPRN();
	} // else keep random # in step
if (RandomizeObj[WCS_EFFECTS_OBJECT3D_RANDOMIZE_POSITION])
	{
	// if texture for group
	GroupVal = 1.0;
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				GroupVal += (Value[0] - GroupVal * TexOpacity);
				} // if
			else
				GroupVal = Value[0];
			} // if
		} // if

	// if texture for X
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Translation[0] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].CurValue;

	// if texture for Y
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Translation[1] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].CurValue;

	// if texture for Z
	AxisVal = Rand.GenPRN();
	if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ] && TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Enabled)
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				AxisVal += (Value[0] - AxisVal * TexOpacity);
				} // if
			else
				AxisVal = Value[0];
			} // if
		} // if
	Translation[2] += AxisVal * GroupVal * (AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].CurValue - AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].CurValue) + AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].CurValue;
	} // if

OutScale[0] = Scale[0];
OutScale[1] = Scale[1];
OutScale[2] = Scale[2];
OutTranslation[0] = Translation[0];
OutTranslation[1] = Translation[1];
OutTranslation[2] = Translation[2];

// use this if all else fails
OutRotation[0] = Rotation[0] + XRot;
OutRotation[1] = Rotation[1] + YRot;
OutRotation[2] = Rotation[2] + ZRot;

EulerToQuaternion(Rotation, Q1);
Rotation[0] = XRot;
Rotation[1] = YRot;
Rotation[2] = ZRot;
EulerToQuaternion(Rotation, Q2);
MultiplyQuaternions(OutQuaternion, Q2, Q1);

// this gave some related but not always correct results
//QuaternionToEuler(Rotation, QuatCombined);

/* this gets the right values but angles are mirrored around +/-90 whenever the real angle is greater than fabs(90)
BuildRotationMatrix(
		Rotation[0] * PiOver180, Rotation[1] * PiOver180, Rotation[2] * PiOver180, LocalNormalMatx);

m02 = LocalNormalMatx[0][2];
m10 = LocalNormalMatx[1][0];
m12 = LocalNormalMatx[1][2];

Pitch = asin(m12);
Heading = asin(-m02 / cos(Pitch)) * PiUnder180;
Bank = asin(-m10 / cos(Pitch)) * PiUnder180;
Pitch *= PiUnder180;

Rotation[0] = Pitch;
Rotation[1] = Heading;
Rotation[2] = Bank;

OutRotation[0] = Rotation[0];
OutRotation[1] = Rotation[1];
OutRotation[2] = Rotation[2];
*/
} // Object3DEffect::CalcTransformParams

/*===========================================================================*/

void Object3DEffect::TransformNormals(void)
{
long Count;

if (Vertices)
	{
	for (Count = 0; Count < NumVertices; Count ++)
		{
		RotatePoint(Vertices[Count].Normal, WorldNormalMatx);
		} // for
	} // if

} // Object3DEffect::TransformNormals

/*===========================================================================*/

long CubeCornerRef[8][3] = {1, 3, 5,  	// v0
							1, 3, 4,  	// v1
							0, 3, 4,  	// v2
							0, 3, 5,  	// v3
							1, 2, 5,  	// v4
							1, 2, 4,  	// v5
							0, 2, 4,  	// v6
							0, 2, 5};	// v7

int Object3DEffect::ProjectBounds(Camera *Cam, RenderData *Rend, VertexDEM *BaseVtx, PolygonData *Poly, double *LimitsX, double *LimitsY, double *LimitsZ)
{
Vertex3D LocalVertices[8];
long Ct, Ct1, Found = 0;

for (Ct = 0; Ct < 8; Ct ++)
	{
	for (Ct1 = 0; Ct1 < 3; Ct1 ++)
		{
		LocalVertices[Ct].xyz[Ct1] = ObjectBounds[CubeCornerRef[Ct][Ct1]];
		} // for
	} // for
Cam->ProjectVertexDEM(Rend->DefCoords, BaseVtx, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
Transform(LocalVertices, 8, Rend, Poly, BaseVtx, NULL, -1.0);
//Transform(LocalVertices, 8, -BaseVtx->XYZ[0], -BaseVtx->XYZ[1], -BaseVtx->XYZ[2], 90.0 - BaseVtx->Lat, -BaseVtx->Lon, 0.0);
LimitsX[0] = LimitsY[0] = LimitsZ[0] = -FLT_MAX;
LimitsX[1] = LimitsY[1] = LimitsZ[1] = FLT_MAX;
for (Ct = 0; Ct < 8; Ct ++)
	{
	Cam->ProjectVertexDEM(Rend->DefCoords, &LocalVertices[Ct], Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
	if (LocalVertices[Ct].ScrnXYZ[2] > 0.0)
		{
		if (LocalVertices[Ct].ScrnXYZ[0] > LimitsX[0])
			LimitsX[0] = LocalVertices[Ct].ScrnXYZ[0];
		if (LocalVertices[Ct].ScrnXYZ[0] < LimitsX[1])
			LimitsX[1] = LocalVertices[Ct].ScrnXYZ[0];
		if (LocalVertices[Ct].ScrnXYZ[1] > LimitsY[0])
			LimitsY[0] = LocalVertices[Ct].ScrnXYZ[1];
		if (LocalVertices[Ct].ScrnXYZ[1] < LimitsY[1])
			LimitsY[1] = LocalVertices[Ct].ScrnXYZ[1];
		if (LocalVertices[Ct].ScrnXYZ[2] > LimitsZ[0])
			LimitsZ[0] = LocalVertices[Ct].ScrnXYZ[2];
		if (LocalVertices[Ct].ScrnXYZ[2] < LimitsZ[1])
			LimitsZ[1] = LocalVertices[Ct].ScrnXYZ[2];
		Found ++;
		} // if
	} // for

return (Found > 1);

} // Object3DEffect::ProjectBounds

/*===========================================================================*/

int Object3DEffect::FindCurrentCartesian(RenderData *Rend, VertexDEM *Vert, char CenterOnOrigin)
{
VectorPoint *CurVtx = NULL;
Joe *CurJoe = NULL;
Vertex3D *TempVertArray = NULL;
PolygonData Poly;
long Ct, NVerts;

// first choice is to focus on a geographic instance, otherwise on the first vertex of the first vector
if (! GeographicInstance && Joes && Joes->Me && Joes->Me->GetFirstRealPoint())
	{
	if (CurVtx = Joes->Me->GetFirstRealPoint())
		CurJoe = Joes->Me;
	} // if

FindBasePosition(Rend, Vert, &Poly, CurJoe, CurVtx);

if (CenterOnOrigin)
	{
	NVerts = 1;
	if (TempVertArray = new Vertex3D[NVerts])
		{
		TempVertArray[0].xyz[0] = TempVertArray[0].xyz[1] = TempVertArray[0].xyz[2] = 0.0;
		} // if
	} // if
else
	{
	NVerts = 8;
	if (TempVertArray = new Vertex3D[NVerts])
		{
		for (Ct = 0; Ct < NVerts; Ct ++)
			{
			TempVertArray[Ct].xyz[0] = ObjectBounds[CubeCornerRef[Ct][0]];
			TempVertArray[Ct].xyz[1] = ObjectBounds[CubeCornerRef[Ct][1]];
			TempVertArray[Ct].xyz[2] = ObjectBounds[CubeCornerRef[Ct][2]];
			} // for
		} // if vertices
	} // if draw cube

#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(Vert);
#else // WCS_BUILD_VNS
Vert->DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS
if (TempVertArray)
	{
	// transform may apply rotation, scaling, translation, vector or terrain alignment, etc.
	if (Transform(TempVertArray, NVerts, Rend, &Poly, Vert, NULL, -1.0))
		{
		if (CenterOnOrigin)
			{
			Vert->XYZ[0] = TempVertArray[0].XYZ[0];
			Vert->XYZ[1] = TempVertArray[0].XYZ[1];
			Vert->XYZ[2] = TempVertArray[0].XYZ[2];
			} // if
		else
			{
			Vert->XYZ[0] = Vert->XYZ[1] = Vert->XYZ[2] = 0.0;
			for (Ct = 0; Ct < NVerts; Ct ++)
				{
				Vert->XYZ[0] += TempVertArray[Ct].XYZ[0];
				Vert->XYZ[1] += TempVertArray[Ct].XYZ[1];
				Vert->XYZ[2] += TempVertArray[Ct].XYZ[2];
				} // for
			Vert->XYZ[0] /= NVerts;
			Vert->XYZ[1] /= NVerts;
			Vert->XYZ[2] /= NVerts;
			} // else
		} // if Joes

	if (TempVertArray)
		delete [] TempVertArray;
	return (1);
	} // if

return (0);

} // Object3DEffect::FindCurrentCartesian

/*===========================================================================*/

int Object3DEffect::FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, VectorPoint *CurVtx)
{
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexData VertData;
unsigned long Flags;

if (CurVtx)
	{
	if (Vector && (MyAttr = (JoeCoordSys *)Vector->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	if (! CurVtx->ProjToDefDeg(MyCoords, &VertData))
		return (0);
	} // if
else
	{
	VertData.Lat = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue;
	VertData.Lon = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue;
	} // else

if (CurVtx && Absolute == WCS_EFFECT_RELATIVETOJOE)			// relative to Joe - apply exaggeration to vector elevation
	{
	Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
		WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
		WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT | WCS_VERTEXDATA_FLAG_NORMAL);
	Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
	VertData.Elev = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue + 
		Rend->ElevDatum + (CurVtx->Elevation - Rend->ElevDatum) * Rend->Exageration;
	} // if relative to Joe
else if (Absolute == WCS_EFFECT_ABSOLUTE)		// absolute - no exaggeration
	{
	Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
		WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
		WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT | WCS_VERTEXDATA_FLAG_NORMAL);
	Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
	VertData.Elev = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
	} // if absolute
else
	{						// relative to ground - exageration applied in VertexDataPoint() to terrain elevation
	Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
		WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED | 
		WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
		WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT | WCS_VERTEXDATA_FLAG_NORMAL);
	Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
	VertData.Elev += AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
	} // else relative to ground
// copy relevant data to vertex and polygon
Vert->Lat = VertData.Lat;
Vert->Lon = VertData.Lon;
Vert->Elev = VertData.Elev;
Poly->Lat = VertData.Lat;
Poly->Lon = VertData.Lon;
Poly->Elev = VertData.Elev;
Poly->RelEl = VertData.RelEl;
Poly->WaterElev = VertData.WaterElev;
Poly->Object = this;
Poly->Vector = Vector;
Poly->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;

Poly->LonSeed = (ULONG)((Vert->Lon - WCS_floor(Vert->Lon)) * ULONG_MAX);
Poly->LatSeed = (ULONG)((Vert->Lat - WCS_floor(Vert->Lat)) * ULONG_MAX);
Rand.Seed64BitShift(Poly->LatSeed, Poly->LonSeed);

return (1);

} // Object3DEffect::FindBasePosition

/*===========================================================================*/

int Object3DEffect::TestOriginOutOfBounds(void)
{
int OutOfBounds = 0;

if ((ObjectBounds[0] > 0.0 && ObjectBounds[1] > 0.0) || (ObjectBounds[0] < 0.0 && ObjectBounds[1] < 0.0))
	{
	OutOfBounds += 1;
	} // if
if ((ObjectBounds[2] > 0.0 && ObjectBounds[3] > 0.0) || (ObjectBounds[2] < 0.0 && ObjectBounds[3] < 0.0))
	{
	OutOfBounds += 2;
	} // if
if ((ObjectBounds[4] > 0.0 && ObjectBounds[5] > 0.0) || (ObjectBounds[4] < 0.0 && ObjectBounds[5] < 0.0))
	{
	OutOfBounds += 4;
	} // if

return (OutOfBounds);

} // Object3DEffect::TestOriginInBounds

/*===========================================================================*/

int Object3DEffect::GetObjectBounds(void)
{
long Pt;

// ObjectBounds
// 0 = High X
// 1 = Low X
// 2 = High Y
// 3 = Low Y
// 4 = High Z
// 5 = Low Z

if (NumVertices > 0 && Vertices)
	{
	ObjectBounds[0] = ObjectBounds[2] = ObjectBounds[4] = -FLT_MAX;
	ObjectBounds[1] = ObjectBounds[3] = ObjectBounds[5] = FLT_MAX;
	for (Pt = 0; Pt < NumVertices; Pt ++)
		{
		if (Vertices[Pt].xyz[0] > ObjectBounds[0])
			ObjectBounds[0] = Vertices[Pt].xyz[0];
		if (Vertices[Pt].xyz[0] < ObjectBounds[1])
			ObjectBounds[1] = Vertices[Pt].xyz[0];

		if (Vertices[Pt].xyz[1] > ObjectBounds[2])
			ObjectBounds[2] = Vertices[Pt].xyz[1];
		if (Vertices[Pt].xyz[1] < ObjectBounds[3])
			ObjectBounds[3] = Vertices[Pt].xyz[1];

		if (Vertices[Pt].xyz[2] > ObjectBounds[4])
			ObjectBounds[4] = Vertices[Pt].xyz[2];
		if (Vertices[Pt].xyz[2] < ObjectBounds[5])
			ObjectBounds[5] = Vertices[Pt].xyz[2];
		} // for
	return (1);
	} // if

ObjectBounds[0] = ObjectBounds[2] = ObjectBounds[4] = ObjectBounds[1] = ObjectBounds[3] = ObjectBounds[5] = 0.0;
return (0);

} // Object3DEffect::GetObjectBounds

/*===========================================================================*/

int Object3DEffect::OutOfBoundsMessage(int Condition)
{
char MesgStr[128], OutX = 0, OutY = 0, OutZ = 0;

if (Condition)
	{
	if (Condition / 4)
		{
		OutZ = 1;
		Condition %= 4;
		} // if
	if (Condition / 2)
		{
		OutY = 1;
		Condition %= 2;
		} // if
	if (Condition)
		{
		OutX = 1;
		} // if

	if (OutX && OutY && OutZ)
		sprintf(MesgStr, "Object's origin falls outside object bounds on X, Y and Z axes. Recenter X and Z and place Y origin at base?");
	else if (OutX && OutY)
		sprintf(MesgStr, "Object's origin falls outside object bounds on X and Y axes. Recenter X and Z and place Y origin at base?");
	else if (OutX && OutZ)
		sprintf(MesgStr, "Object's origin falls outside object bounds on X and Z axes. Recenter X and Z and place Y origin at base?");
	else if (OutY && OutZ)
		sprintf(MesgStr, "Object's origin falls outside object bounds on Y and Z axes. Recenter X and Z and place Y origin at base?");
	else if (OutX)
		sprintf(MesgStr, "Object's origin falls outside object bounds on X axis. Recenter X and Z and place Y origin at base?");
	else if (OutY)
		sprintf(MesgStr, "Object's origin falls outside object bounds on Y axis. Recenter X and Z and place Y origin at base?");
	else if (OutZ)
		sprintf(MesgStr, "Object's origin falls outside object bounds on Z axis. Recenter X and Z and place Y origin at base?");

	return (UserMessageYN("3D Object Center", MesgStr));
	} // if

return (0);

} // Object3DEffect::OutOfBoundsMessage

/*===========================================================================*/

int Object3DEffect::RecenterObject(void)
{
double BoundsDifferential[3];
long VtxNum;

// bounds are in same units and scaling as vertices
if (Vertices)
	{
	BoundsDifferential[0] = (ObjectBounds[0] + ObjectBounds[1]) * .5;
	BoundsDifferential[1] = ObjectBounds[3];
	BoundsDifferential[2] = (ObjectBounds[4] + ObjectBounds[5]) * .5;
	for (VtxNum = 0; VtxNum < NumVertices; VtxNum ++)
		{
		Vertices[VtxNum].xyz[0] -= BoundsDifferential[0];
		Vertices[VtxNum].xyz[1] -= BoundsDifferential[1];
		Vertices[VtxNum].xyz[2] -= BoundsDifferential[2];
		} // for
	GetObjectBounds();
	// save object back to file
	if (! stricmp(&FileName[strlen(FileName) - 4], ".w3d"))
		SaveObjectW3D();	// careful, this can return negative values if it fails
	else
		SaveObjectW3O();
	return (1);
	} // if

return (0);

} // Object3DEffect::RecenterObject

/*===========================================================================*/

void Object3DEffect::ApplyUnitScaling(void)
{
double XScale, YScale, ZScale;
Object3DEffect *TempObj;
NotifyTag Changes[2];
long VtxNum;

if (! UserMessageOKCAN("Scale 3D Object", "This will cause new dimensions to be computed\n and saved to the \
Object's .w3o or .w3d file.\n Other 3D Objects that use this file will be affected.\n\nProceed with scaling?"))
	return;

// <<<>>>gh
// load to duplicate object
// scale and copy geometry
// resave in appropriate format

if (TempObj = new Object3DEffect(NULL))
	{
	Copy(TempObj, this);

	if (TempObj->OpenInputFile(NULL, TRUE, TRUE, FALSE))
		{
		if (Vertices)
			{
			//delete [] Vertices;
			FreeVertices();
			} // if
		if (Polygons)
			{
			//delete [] Polygons;
			FreePolygons();
			} // if
		if (TempObj->Vertices)
			{
			if (TempObj->Units != WCS_USEFUL_UNIT_METER)
				{
				for (VtxNum = 0; VtxNum < TempObj->NumVertices; VtxNum ++)
					{
					TempObj->Vertices[VtxNum].xyz[0] = ConvertToMeters(TempObj->Vertices[VtxNum].xyz[0], TempObj->Units);
					TempObj->Vertices[VtxNum].xyz[1] = ConvertToMeters(TempObj->Vertices[VtxNum].xyz[1], TempObj->Units);
					TempObj->Vertices[VtxNum].xyz[2] = ConvertToMeters(TempObj->Vertices[VtxNum].xyz[2], TempObj->Units);
					} // for
				TempObj->Units = WCS_USEFUL_UNIT_METER;
				} // if units not meters
			if (TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue != 1.0
				|| TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue != 1.0
				|| TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue != 1.0)
				{
				XScale = TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
				YScale = TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
				ZScale = TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
				// if normals are precalculated you can only scale isometrically
				if (! TempObj->CalcNormals)
					YScale = ZScale = XScale;
				for (VtxNum = 0; VtxNum < TempObj->NumVertices; VtxNum ++)
					{
					TempObj->Vertices[VtxNum].xyz[0] = (TempObj->Vertices[VtxNum].xyz[0] * XScale);
					TempObj->Vertices[VtxNum].xyz[1] = (TempObj->Vertices[VtxNum].xyz[1] * YScale);
					TempObj->Vertices[VtxNum].xyz[2] = (TempObj->Vertices[VtxNum].xyz[2] * ZScale);
					} // for
				// normalize all scale values
				if (TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue != 0.0)
					TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].ScaleValues(1.0 / TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue);
				if (TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue != 0.0)
					TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].ScaleValues(1.0 / TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue);
				if (TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue != 0.0)
					TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].ScaleValues(1.0 / TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue);
				AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].Copy(&AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX], &TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX]);
				AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].Copy(&AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY], &TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY]);
				AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].Copy(&AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ], &TempObj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ]);
				} // if

			TempObj->GetObjectBounds();

			// save object back to file
			if (! stricmp(&TempObj->FileName[strlen(TempObj->FileName) - 4], ".w3d"))
				TempObj->SaveObjectW3D();	// careful, this can return negative values if it fails
			else
				TempObj->SaveObjectW3O();

			Vertices = TempObj->Vertices;
			Polygons = TempObj->Polygons;
			VertRefBlock = TempObj->VertRefBlock;
			PolyRefBlock = TempObj->PolyRefBlock;
			NumVertices = TempObj->NumVertices;
			NumPolys = TempObj->NumPolys;
			TempObj->Vertices = NULL;
			TempObj->Polygons = NULL;
			TempObj->VertRefBlock = NULL;
			TempObj->PolyRefBlock = NULL;
			Units = WCS_USEFUL_UNIT_METER;

			GetObjectBounds();

			// name may have been changed to prevent overwriting existing file
			strcpy(TempObj->FilePath, FilePath);
			strcpy(TempObj->FileName, FileName);

			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	delete TempObj;
	} // if

} // Object3DEffect::ApplyUnitScaling

/*===========================================================================*/


long Object3DEffect::InitImageIDs(long &ImageID)
{
MaterialEffect *MyMat;
long Ct, ImagesFound = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		ImagesFound += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for
for (Ct = 0; Ct < NumMaterials; Ct ++)
	{
	if (MyMat = GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Ct], NULL))
		ImagesFound += MyMat->InitImageIDs(ImageID);
	} // for

return (ImagesFound);

} // Object3DEffect::InitImageIDs

/*===========================================================================*/

int Object3DEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
MaterialEffect *MyMat;
EffectList **ListPtr;
long Ct;

for (Ct = 0; Ct < NumMaterials; Ct ++)
	{
	if (MyMat = GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Ct], NULL))
		{
		ListPtr = Material3Ds;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == MyMat)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = MyMat;
			else
				return (0);
			} // if
		if (! MyMat->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	} // for

return (GeneralEffect::BuildFileComponentsList(Queries, Themes, Coords));

} // Object3DEffect::BuildFileComponentsList

/*===========================================================================*/

void Object3DEffect::HeresYourNewFilePathIfYouCare(char *NewFullPath)
{

BreakFileName(NewFullPath, FilePath, 256, FileName, 64);

} // Object3DEffect::HeresYourNewFilePathIfYouCare

/*===========================================================================*/

void Object3DEffect::CompleteInstanceInfo(RenderData *Rend, Object3DInstance *CurInstance, PolygonData *Poly, double RefLat, double RefLon, double RefElev,
	double PointLat, double PointLon, double PointElev, double ExtraRotation[3], double Height)
{
double Translation[3];
unsigned long Flags;
VertexData VertData;
Vertex3D Vert3D;

Poly->Lon = VertData.Lon = RefLon + PointLon;
Poly->Lat = VertData.Lat = RefLat + PointLat;
Poly->Elev = VertData.Elev = RefElev + PointElev;
Poly->LonSeed = (ULONG)((VertData.Lon - WCS_floor(VertData.Lon)) * ULONG_MAX);
Poly->LatSeed = (ULONG)((VertData.Lat - WCS_floor(VertData.Lat)) * ULONG_MAX);

Flags = (WCS_VERTEXDATA_FLAG_RELEL | 
	WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
	WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT | WCS_VERTEXDATA_FLAG_NORMAL);
Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);

Poly->RelEl = VertData.RelEl;
Poly->WaterElev = VertData.WaterElev;
// this might be set to terrain elev by VertexDataPoint
VertData.Elev = Poly->Elev;

#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(&VertData);
#else // WCS_BUILD_VNS
VertData.DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS

CalcTransformParams(Rend, Poly, &VertData, ExtraRotation, Height, 
	CurInstance->Euler, CurInstance->Quaternion, Translation, CurInstance->Scale);

// position is the translation component of the object
// we need the geographic position

// we could use the actual Transform function to transmogrify a single point at the object's origin
// and transform that coordinate into geographic. Seems the simplest to do even though it will duplicate
// some of the effort done to obtain parameters above.

Vert3D.xyz[0] = Vert3D.xyz[1] = Vert3D.xyz[2] = 0.0;

Transform(&Vert3D, 1, Rend, Poly, &VertData, ExtraRotation, Height);

#ifdef WCS_BUILD_VNS
Rend->DefCoords->CartToDeg(&Vert3D);
#else // WCS_BUILD_VNS
Vert3D.CartToDeg(Rend->PlanetRad);
#endif // WCS_BUILD_VNS

CurInstance->WCSGeographic[0] = Vert3D.Lon;
CurInstance->WCSGeographic[1] = Vert3D.Lat;
CurInstance->WCSGeographic[2] = Vert3D.Elev;
CurInstance->MyObj = this;

} // Object3DEffect::CompleteInstanceInfo

/*===========================================================================*/

void Object3DEffect::CompleteInstanceInfo(RenderData *Rend, Object3DInstance *CurInstance, RealtimeFoliageIndex *Index, 
	RealtimeFoliageData *FolData, FoliagePreviewData *PointData)
{
PolygonData Poly;

#ifdef WCS_BUILD_SX2
Poly.Vector = FolData->MyVec;
#endif // WCS_BUILD_SX2

CompleteInstanceInfo(Rend, CurInstance, &Poly, Index->RefXYZ[1], Index->RefXYZ[0], Index->RefXYZ[2],
	(double)FolData->XYZ[1], (double)FolData->XYZ[0], (double)FolData->XYZ[2], PointData->Rotate, PointData->Height);

} // Object3DEffect::CompleteInstanceInfo

/*===========================================================================*/

long Object3DEffect::CountMaterialPolygons(long MatNum)
{
long PolyCt, MatPolyCt = 0;

if (Polygons)
	{
	for (PolyCt = 0; PolyCt < NumPolys; PolyCt ++)
		{
		if (Polygons[PolyCt].NumVerts >= 3)
			{
			if (Polygons[PolyCt].Material == MatNum)
				{
				MatPolyCt += Polygons[PolyCt].NumVerts - 2;
				} // if
			} // if
		} // for
	} // if

return (MatPolyCt);

} // Object3DEffect::CountMaterialPolygons

/*===========================================================================*/

long Object3DEffect::Count3SidedPolygons(void)
{
long PolyCt, SumPolyCt = 0;

if (Polygons)
	{
	for (PolyCt = 0; PolyCt < NumPolys; PolyCt ++)
		{
		if (Polygons[PolyCt].NumVerts >= 3)
			{
			SumPolyCt += Polygons[PolyCt].NumVerts - 2;
			} // if
		} // for
	} // if

return (SumPolyCt);

} // Object3DEffect::Count3SidedPolygons

/*===========================================================================*/

INT Object3DEffect::AllMaterialsLuminous(void)
{
long MatCt;

if (NameTable)
	{
	for (MatCt = 0; MatCt < NumMaterials; MatCt ++)
		{
		if (NameTable[MatCt].Mat || (NameTable[MatCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable[MatCt].Name)))
			{
			if (NameTable[MatCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0) < .999)
				return (0);
			} // if
		else
			return (0);
		} // for
	return (1);
	} // if

return (0);

} // Object3DEffect::AllMaterialsLuminous

/*===========================================================================*/
/*===========================================================================*/

int CompareRenderPolygon3D(const void *elem1, const void *elem2)
{

return (
	((RenderPolygon3D *)elem1)->qqq > ((RenderPolygon3D *)elem2)->qqq ? 1:
	(((RenderPolygon3D *)elem1)->qqq < ((RenderPolygon3D *)elem2)->qqq ? -1: 0)
	);

} // CompareRenderPolygon3D
