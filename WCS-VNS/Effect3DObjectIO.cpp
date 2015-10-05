// Effect3DObjectIO.cpp
// Contains functions relating to reading and saving 3D Objects

#include "stdafx.h"

// file parts
// Part A: Properties user may manipulate in memory that affect rendering
	// scaling, position, shading, object bounds
// Part B: Primitives Count
	// num vertices, num polygons, num materials
// Part C: Geometry
	// vertices, polygons
// Part D: Material names

// when to read, when to write parts
// Part A:
	// Write: Project file, Component file
	// Read: Load Project, New Component
// Part B:
	// Write: Project file, Component file
	// Read: Load Project, New Component, New file selected, Load to render
// Part C:
	// Write: Translating from alien formats, Component file, Apply scaling
	// Read: New Component, New file selected, Apply scaling, Load to render
// Part D:
	// Write: Project file, Component file
	// Read: Load Project, New Component, New file selected

// which parts, which file
// Part A:
	// Write: .proj, .w3d
// Part B:
	// Write: .proj, .w3d
// Part C:
	// Write: .w3d, .w3o
// Part D:
	// Write: .w3d, .w3o

// .proj
	// Object3DEffect_Save() - WCS_EFFECTSBASE_DATA chunk tag
		// Save() - BrowseInfo
		//          Part A
		//          Part B
		//          Part D
// .w3o
	// SaveObjectW3O() - "WCS File", Project version, revision, byte order
	//                   "Object3D" chunk tag
		// ObjectData_Save() - 3d object version, revision, byte order, 
		//                     WCS_EFFECTSPOINT_OBJECT3D chunk tag
			// SaveData() - Part B
			//              Part C
			//              Part D
// .w3d
	// SaveFilePrep() - "WCS File", Project version, revision, byte order
	//                  BrowseInfo
		// SaveObject() - DEM Bounds 
		//                Images
		//                Waves
		//                Materials
		//                3D Objects "Object3D" chunks
		//                Current "Object3D" chunk
			// SaveWithGeometry() - BrowseInfo
			//                      Part A
			//                      Part B
			//                      Part C	// not found in early V5 component files
			//                      Part D

// which operation, which file, which parts
// Save project
	// .proj 
		// Save A, B, D
// Load project
	// .proj 
		// Load A, B, D
// Save component
	// .w3d 
		// Save A, B, C, D
// Load component
	// .w3d 
		// Load A, B, C, D
// Select new file
	// .w3d 
		// Load B, C, D - skip A
	// .w3o
		// Load B, C, D
	// .lwo, .3ds, .dxf
		// Load B, C, D
		// Save .w3o B, C, D
// Apply scaling
	// .w3o
		// Load B, C, D to backup object
		// Save .w3o B, C, D from backup object
		// copy C to current object
	// .w3d
		// Load A, B, C, D to backup object
		// Save .w3d A, B, C, D from backup object
		// copy C to current object
	// .lwo, .3ds, .dxf - .w3o or .w3d can't be found
		// Load B, C, D to backup object
		// Save .w3o B, C, D from backup object
		// copy C to current object
// Load to render
	// .w3o
		// Load B, C - skip D
	// .w3d
		// Load B, C -  skip A, D
	// .lwo, .3ds, .dxf - .w3o or .w3d can't be found
		// Load B, C, D to backup object
		// Save .w3o B, C, D from backup object
		// Copy B, C to current object

// which file, which parts, which operations, what function
// Saving
	// .proj
		// A, B, D  Save Project
		//virtual unsigned long Save(FILE *ffile);
			// does not save geometry
	// .w3o
		// B, C, D  Select new alien file, Apply scaling, Load to render alien file
		//int SaveObjectW3O(void);
			// opens file, writes project version #, byte order, "Object3D" chunk tag
	// .w3d
		// A, B, C, D  Save component
		// RasterAnimHost::SaveFilePrep()
		// A, B, C, D  Apply scaling
		//int SaveObjectW3D(void);
			// calls SaveFilePrep()

// Loading
	// .proj
		// A, B, D  Load project
		//virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
			// does not load geometry
	// .w3o
		// B, C, D  Select new file, Apply scaling
		// B, C     Load to render
		//int LoadObjectW3O(short LoadMaterials);
			// LoadMaterials instructs whether or not to load part D
	// .w3d
		// A, B, C, D  Load component
		// RasterAnimHost::LoadFilePrep()
		// A, B, C, D  Apply scaling
		// B, C, D     Select new file
		// B, C        Load to render
		//int LoadObjectW3D(short LoadParameters, LoadMaterials);
			// LoadParameters instructs whether or not to load part A
			// LoadMaterials instructs whether or not to load part D

#include "Application.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Project.h"
#include "Requester.h"
#include "Log.h"
#include "Raster.h"
#include "Lists.h"
#include "FeatureConfig.h"

/*===========================================================================*/
/*===========================================================================*/
// Project file IO

ULONG EffectsLib::Object3DEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Object3DEffect *Current;

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
					//case WCS_EFFECTSBASE_RESOLUTION:
					//	{
					//	BytesRead = ReadBlock(ffile, (char *)&Object3DBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
					//	break;
					//	}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Object3DEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Object3DEffect *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
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

} // EffectsLib::Object3DEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::Object3DEffect_Save(FILE *ffile)
{
Object3DEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

//if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
//	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
//	WCS_BLOCKTYPE_FLOAT, (char *)&Object3DBase.Resolution)) == NULL)
//	goto WriteError;
//TotalWritten += BytesWritten;

Current = Object3D;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if lake effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Object3DEffect *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::Object3DEffect_Save()

/*===========================================================================*/

unsigned long Object3DEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long NamesRead = 0, LatLoaded = 0;
char TempReceiveShadows;
#ifdef WCS_BUILD_VNS
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_BUILD_VNS
long UVWTableNum = 0, CPVTableNum = 0;

if (UVWTable)
	delete [] UVWTable;
UVWTable = NULL;
NumUVWMaps = 0;
if (CPVTable)
	delete [] CPVTable;
CPVTable = NULL;
NumCPVMaps = 0;

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
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_HIRESEDGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HiResEdge, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_USEGRADIENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseGradient, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROCCLUDED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderOccluded, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_LATITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].Load(ffile, Size, ByteFlip);
						LatLoaded = 1;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_LONGITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ELEVATION:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGXx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGYx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGZx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWINTENSx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWINTENS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWOFFSET:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTBIAS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZPLUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZMINUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_AAPASSES:
						{
						BytesRead = ReadBlock(ffile, (char *)&AAPasses, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_DRAWENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWSONLY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowsOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CASTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempReceiveShadows)
							{
							ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = 
								ReceiveShadowsCloudSM = 1;
							ReceiveShadowsVolumetric = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SELFSHADOWED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempReceiveShadows)
							{
							ReceiveShadowsTerrain = ReceiveShadowsFoliage =  
								ReceiveShadowsCloudSM = ReceiveShadowsVolumetric = 0;
							ReceiveShadows3DObject = 1;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSFOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS3D:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows3DObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSSM:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsCloudSM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSVOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsVolumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_UNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Units, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CALCNORMALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CalcNormals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTCOLORAVAILABLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertexColorsAvailable, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTUVWAVAILABLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertexUVWAvailable, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNHEADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlignHeading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTICAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlignVertical, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_HEADINGAXIS:
						{
						BytesRead = ReadBlock(ffile, (char *)&HeadingAxis, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTICALAXIS:
						{
						BytesRead = ReadBlock(ffile, (char *)&VerticalAxis, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_REVERSEHEADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReverseHeading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZESCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEROTATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEPOSITION:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEVERTEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeVert, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHOWDETAIL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowDetail, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ISOMETRIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Isometric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTVEC:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlignVertVec, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNSPECIALVEC:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlignSpecialVec, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_GEOINSTANCE:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeographicInstance, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FRAGMENTOPTIMIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FragmentOptimize, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}

					case WCS_EFFECTS_OBJECT3D_SHADOWMAPWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowMapWidth, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_USEMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_REGENMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RegenMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}

					case WCS_EFFECTS_OBJECT3D_NUMVERTICES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumVertices, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMPOLYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumPolys, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMMATERIALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumMaterials, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[3], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[4], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[5], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FILEPATH:
						{
						BytesRead = ReadBlock(ffile, (char *)FilePath, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FILENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)FileName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_MATERIALNAME:
						{
						if (! NameTable && NumMaterials > 0)
							NameTable = new ObjectMaterialEntry[NumMaterials];
						if (NameTable)
							{
							BytesRead = ReadBlock(ffile, (char *)NameTable[NamesRead++].Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMUVWTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumUVWMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumUVWMaps > 0)
							{
							if ((UVWTable = new ObjectPerVertexMap[NumUVWMaps]) == NULL)
								goto ReadError;
							UVWTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMCPVTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumCPVMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumCPVMaps > 0)
							{
							if ((CPVTable = new ObjectPerVertexMap[NumCPVMaps]) == NULL)
								goto ReadError;
							CPVTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_UVWTABLE:
						{
						if (UVWTableNum < NumUVWMaps)
							BytesRead = UVWTable[UVWTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						UVWTableNum ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CPVTABLE:
						{
						if (CPVTableNum < NumCPVMaps)
							BytesRead = CPVTable[CPVTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						CPVTableNum ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALE:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEX:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEY:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEZ:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATE:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEX:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEY:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEZ:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITION:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONX:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONY:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONZ:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITION:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONX:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONY:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONZ:
						{
						if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_QUERY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Search = (SearchQuery *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName);
							} // if
						break;
						}
					#endif // WCS_BUILD_VNS
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

// older file that did not offer geographic positioning
if (! LatLoaded)
	GeographicInstance = 0;

return (TotalRead);

ReadError:
return (0);

} // Object3DEffect::Load

/*===========================================================================*/
/*===========================================================================*/

unsigned long Object3DEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, NamesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_OBJECT3D_NUMANIMPAR] = {WCS_EFFECTS_OBJECT3D_LATITUDE,
																	WCS_EFFECTS_OBJECT3D_LONGITUDE,
																	WCS_EFFECTS_OBJECT3D_ELEVATION,
																	WCS_EFFECTS_OBJECT3D_ROTATIONX,
																	WCS_EFFECTS_OBJECT3D_ROTATIONY,
																	WCS_EFFECTS_OBJECT3D_ROTATIONZ,
																	WCS_EFFECTS_OBJECT3D_SCALINGX,
																	WCS_EFFECTS_OBJECT3D_SCALINGY,
																	WCS_EFFECTS_OBJECT3D_SCALINGZ,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONX,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONY,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONZ,
																	WCS_EFFECTS_OBJECT3D_SHADOWINTENS,
																	WCS_EFFECTS_OBJECT3D_SHADOWOFFSET,
																	WCS_EFFECTS_OBJECT3D_ALIGNVERTBIAS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZMINUS};
unsigned long TextureItemTag[WCS_EFFECTS_OBJECT3D_NUMTEXTURES] = {WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALE,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATE,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITION,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITION,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONZ};
#ifdef WCS_THEMATIC_MAP
unsigned long ThemeItemTag[WCS_EFFECTS_OBJECT3D_NUMTHEMES] = {WCS_EFFECTS_OBJECT3D_THEMEROTATIONX,
																	WCS_EFFECTS_OBJECT3D_THEMEROTATIONY,
																	WCS_EFFECTS_OBJECT3D_THEMEROTATIONZ,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGX,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGY,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGZ,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONX,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONY,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONZ};
#endif // WCS_THEMATIC_MAP

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_HIRESEDGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&HiResEdge)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_USEGRADIENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseGradient)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ABSOLUTE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Absolute)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROCCLUDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RenderOccluded)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block
					else
						goto WriteError;
					} // if texture saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

#ifdef WCS_THEMATIC_MAP
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetTheme(Ct))
		{
		if ((BytesWritten = PrepWriteBlock(ffile, ThemeItemTag[Ct], WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(GetTheme(Ct)->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)GetTheme(Ct)->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // for
#endif // WCS_THEMATIC_MAP

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_AAPASSES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AAPasses)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_DRAWENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHADOWSONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShadowsOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSFOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows3DObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSSM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsCloudSM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSVOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsVolumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_UNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Units)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_CALCNORMALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CalcNormals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTCOLORAVAILABLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertexColorsAvailable)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTUVWAVAILABLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertexUVWAvailable)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNHEADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignHeading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNVERTICAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignVertical)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_HEADINGAXIS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HeadingAxis)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTICALAXIS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VerticalAxis)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_REVERSEHEADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReverseHeading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZESCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEROTATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEPOSITION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEVERTEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeVert)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHOWDETAIL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowDetail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ISOMETRIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Isometric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNVERTVEC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignVertVec)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNSPECIALVEC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignSpecialVec)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_GEOINSTANCE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeographicInstance)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FRAGMENTOPTIMIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FragmentOptimize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHADOWMAPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowMapWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_USEMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_REGENMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RegenMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMVERTICES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumVertices)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMPOLYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumPolys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMMATERIALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumMaterials)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMUVWTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumUVWMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMCPVTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumCPVMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[3])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[4])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[5])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FILEPATH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FilePath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FilePath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FILENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FileName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FileName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (NumMaterials > 0 && NameTable)
	{
	for (NamesWritten = 0; NamesWritten < NumMaterials; NamesWritten ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_MATERIALNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(NameTable[NamesWritten].Name) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)NameTable[NamesWritten].Name)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // if

if (UVWTable)
	{
	for (Ct = 0; Ct < NumUVWMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_UVWTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = UVWTable[Ct].Save(ffile, FALSE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if UVWTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

if (CPVTable)
	{
	for (Ct = 0; Ct < NumCPVMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_CPVTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CPVTable[Ct].Save(ffile, FALSE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if CPVTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

#ifdef WCS_BUILD_VNS
if (Search)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_QUERY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Search->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Search->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Object3DEffect::Save

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
// .w3o file IO


int Object3DEffect::LoadObjectW3O(short LoadMaterials)
{
char TempFileName[256], Title[12], ReadBuf[32];
char Version, Revision;
short ByteFlip;
FILE *ffile;
//ULONG ItemTag = 0;
ULONG Size, BytesRead, TotalRead = 0, ByteOrder;
long Pt;
int Success = 0;

if (LoadMaterials)
	{
	if (NameTable)
		delete [] NameTable;
	NameTable = NULL;
	NumMaterials = 0;
	} // if
NumPolys = 0;
if (Vertices)
	{
	FreeVertices();
	} // if
if (Polygons)
	{
	FreePolygons();
	} // if
CalcNormals = 1;
VertexColorsAvailable = VertexUVWAvailable = 0;

if (FilePath[0] && FileName[0])
	{
	strmfp(TempFileName, FilePath, FileName);
	StripExtension(TempFileName);
	strcat(TempFileName, ".w3o");
	if (ffile = PROJ_fopen(TempFileName, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, ffile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Version, &ReadBuf[8], 1);
		memcpy(&Revision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (ByteOrder == 0xaabbccdd)
			ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			ByteFlip = 1;
		else
			goto ReadError;

		Title[11] = '\0';
		if (! strnicmp(Title, "WCS File", 8))
			{
			TotalRead = BytesRead = 14;

			while (BytesRead)
				{
				/* read block descriptor tag from file */
				if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
					WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
					{
					TotalRead += BytesRead;
					ReadBuf[8] = 0;
					// read block size from file 
					if (BytesRead = ReadBlock(ffile, (char *)&Size,
						WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
						{
						TotalRead += BytesRead;
						BytesRead = 0;
						if (! strnicmp(ReadBuf, "Object3D", 8))
							{
							if ((BytesRead = ObjectData_Load(ffile, Size, LoadMaterials)) == Size)
								Success = 1;
							} // else if images
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TotalRead += BytesRead;
						if (BytesRead != Size)
							{
							break;
							} // if error
						} // if size block read 
					else
						break;
					} // if tag block read 
				else
					break;
				} // while 

			} // if WCS File
		fclose(ffile);
		} // if file
	else
		{
//		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, TempFileName);
		return (0);
		} // else
	} // if name

if (Success && LoadMaterials && ! NameTable)
	{
	Success = 0;
	UserMessageOK("Load 3D Object", "This is an older WCS 3D Object file that does not contain materials. It cannot be loaded directly.\nYou must instead select the .lwo, .3ds or .dxf file that corresponds with it.");
	} // if
if (Success && NameTable)
	{
	for (Pt = 0; Pt < NumMaterials; Pt ++)
		{
		GlobalApp->AppEffects->SetMakeMaterial(&NameTable[Pt], NULL);
		} // for
	} // if

return (Success);

ReadError:

if (ffile)
	fclose(ffile);
sprintf(TempFileName, "3D Object, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, TempFileName);
return (0);

} // Object3DEffect::LoadObjectW3O

/*===========================================================================*/

ULONG Object3DEffect::ObjectData_Load(FILE *ffile, ULONG ReadSize, short LoadMaterials)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, ByteOrder;
union MultiVal MV;
short ByteFlip;
static char StructBuf[24];  // Big enuf to hold anything we currently fread...
char Version, Revision;

if (! ffile)
	{
	return (0);
	} // if need to open file

// header data is 6 bytes
if ((fread((char *)StructBuf, 6, 1, ffile)) != 1)
	return (0);

Version = StructBuf[0];
Revision = StructBuf[1];
memcpy(&ByteOrder, &StructBuf[2], 4);

// Endian flop if necessary

if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

TotalRead = 6;
while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
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
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSPOINT_OBJECT3D:
						{
						BytesRead = LoadData(ffile, Size, ByteFlip, LoadMaterials);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // Object3DEffect::ObjectData_Load

/*===========================================================================*/

unsigned long Object3DEffect::LoadData(FILE *ffile, unsigned long ReadSize, short ByteFlip, short LoadMaterials)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long TempNames, UVWTableNum = 0, CPVTableNum = 0, VertsRead = 0, PolysRead = 0, NamesRead = 0;

if (UVWTable)
	delete [] UVWTable;
UVWTable = NULL;
NumUVWMaps = 0;
if (CPVTable)
	delete [] CPVTable;
CPVTable = NULL;
NumCPVMaps = 0;

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
					case WCS_EFFECTS_OBJECT3D_NUMVERTICES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumVertices, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumVertices > 0)
							{
							if ((Vertices = new Vertex3D[NumVertices]) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMPOLYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumPolys, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumPolys > 0)
							{
							if ((Polygons = new Polygon3D[NumPolys]) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMMATERIALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempNames, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (LoadMaterials)
							{
							if (NameTable)
								{
								delete [] NameTable;
								NameTable = NULL;
								} // if
							NumMaterials = TempNames;
							} // if
						else if (TempNames != NumMaterials)
							goto ReadError;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMUVWTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumUVWMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumUVWMaps > 0)
							{
							if ((UVWTable = new ObjectPerVertexMap[NumUVWMaps]) == NULL)
								goto ReadError;
							UVWTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMCPVTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumCPVMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumCPVMaps > 0)
							{
							if ((CPVTable = new ObjectPerVertexMap[NumCPVMaps]) == NULL)
								goto ReadError;
							CPVTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTEX3D:
						{
						if (VertsRead < NumVertices)
							BytesRead = Vertices[VertsRead].Load(ffile, Size, ByteFlip, &CPVTable, NumVertices, NumCPVMaps, VertsRead);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						VertsRead ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_POLYGON3D:
						{
						if (PolysRead < NumPolys)
							BytesRead = Polygons[PolysRead].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						PolysRead ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_UVWTABLE:
						{
						if (UVWTableNum < NumUVWMaps)
							BytesRead = UVWTable[UVWTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						UVWTableNum ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CPVTABLE:
						{
						if (CPVTableNum < NumCPVMaps)
							BytesRead = CPVTable[CPVTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						CPVTableNum ++;
						break;
						}

					case WCS_EFFECTS_OBJECT3D_MATERIALNAME:
						{
						if (LoadMaterials)
							{
							if (! NameTable && NumMaterials > 0)
								NameTable = new ObjectMaterialEntry[NumMaterials];
							if (NameTable)
								{
								BytesRead = ReadBlock(ffile, (char *)NameTable[NamesRead++].Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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

CalcNormals = (Vertices && Vertices[0].Normalized && Vertices[NumVertices - 1].Normalized) ? 0: 1;
if (! CalcNormals)
	Isometric = 1;
VertexColorsAvailable = (CPVTable) ? 1: 0;
VertexUVWAvailable = (UVWTable) ? 1: 0;

return (TotalRead);

ReadError:
return (0);

} // Object3DEffect::LoadData

/*===========================================================================*/
/*===========================================================================*/

int Object3DEffect::SaveObjectW3O(void)
{
FILE *ffile = NULL;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;
char TempFileName[256], FileType[12], StrBuf[12];
char Version = WCS_PROJECT_CURRENT_VERSION;
char Revision = WCS_PROJECT_CURRENT_REVISION;

if (FilePath[0] && FileName[0])
	{
	strmfp(TempFileName, FilePath, FileName);
	StripExtension(TempFileName);
	strcat(TempFileName, ".w3o");
	if (ffile = PROJ_fopen(TempFileName, "wb"))
		{
		strcpy(FileType, "WCS File");

		// no tags or sizes for first four items: file descriptor, version, revision & byte order
		if ((BytesWritten = WriteBlock(ffile, (char *)FileType,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		memset(StrBuf, 0, 9);
		strcpy(StrBuf, "Object3D");

		if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ObjectData_Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} /* if wrote size of block */
					else
						goto WriteError;
					} /* if Paths saved */
				else
					goto WriteError;
				} /* if size written */
			else
				goto WriteError;
			} /* if tag written */
		else
			goto WriteError;

		fclose(ffile);
		} // if file
	else
		{
		sprintf(TempFileName, "3D Object %s", FileName);
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, TempFileName);
		return (0);
		} // else
	} // if name
else
	return (0);

return (1);

WriteError:

if (ffile)
	fclose(ffile);
sprintf(TempFileName, "3D Object, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, TempFileName);

return (0);

} // Object3DEffect::SaveObjectW3O

/*===========================================================================*/

unsigned long Object3DEffect::ObjectData_Save(FILE *ffile)
{
char Version = WCS_OBJECT3D_VERSION, Revision = WCS_OBJECT3D_REVISION;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;

if (ffile)
	{
	// no tags or sizes for first three items: version, revision & byte order
	if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_EFFECTSPOINT_OBJECT3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = SaveData(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if raster saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;

	ItemTag = WCS_PARAM_DONE;
	if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} /* if file */

return (TotalWritten);

WriteError:

return (0L);

} // Object3DEffect::ObjectData_Save

/*===========================================================================*/

unsigned long Object3DEffect::SaveData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, NamesWritten, Ct;
int SaveNormals;

SaveNormals = ! CalcNormals;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMVERTICES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumVertices)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMPOLYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumPolys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMMATERIALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumMaterials)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMUVWTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumUVWMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMCPVTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumCPVMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < NumVertices; Ct ++)
	{
	ItemTag = WCS_EFFECTS_OBJECT3D_VERTEX3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Vertices[Ct].Save(ffile, SaveNormals))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

for (Ct = 0; Ct < NumPolys; Ct ++)
	{
	ItemTag = WCS_EFFECTS_OBJECT3D_POLYGON3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Polygons[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

if (UVWTable)
	{
	for (Ct = 0; Ct < NumUVWMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_UVWTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = UVWTable[Ct].Save(ffile, TRUE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if UVWTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

if (CPVTable)
	{
	for (Ct = 0; Ct < NumCPVMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_CPVTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CPVTable[Ct].Save(ffile, TRUE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if CPVTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

if (NumMaterials > 0 && NameTable)
	{
	for (NamesWritten = 0; NamesWritten < NumMaterials; NamesWritten ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_MATERIALNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(NameTable[NamesWritten].Name) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)NameTable[NamesWritten].Name)) == NULL)
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

} // Object3DEffect::SaveData

/*===========================================================================*/
/*===========================================================================*/
// .w3d file IO

// used for loading component files for rendering or applying scaling or if selected as new file
int Object3DEffect::LoadObjectW3D(short LoadMaterials, short LoadParameters, short Queries)
{
char filename[512], Title[12], ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
char Version, Revision;
short ByteFlip;
int Success = 0, OldBoundsLoaded = 0, Count;
ULONG ByteOrder, Size, BytesRead = 1, TotalRead = 0;
//ULONG ItemTag = 0;
NotifyTag Changes[2];
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
CoordSys *CurrentCoordSys = NULL;
FILE *fFile;
DEMBounds OldBounds, CurBounds;


if (LoadMaterials)
	{
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

strmfp(filename, FilePath, FileName);
if (fFile = PROJ_fopen(filename, "rb"))
	{
	fread((char *)ReadBuf, 14, 1, fFile);

	memcpy(Title, &ReadBuf[0], 8);
	memcpy(&Version, &ReadBuf[8], 1);
	memcpy(&Revision, &ReadBuf[9], 1);
	memcpy(&ByteOrder, &ReadBuf[10], 4);

	if (ByteOrder == 0xaabbccdd)
		ByteFlip = 0;
	else if (ByteOrder == 0xddccbbaa)
		ByteFlip = 1;
	else
		goto ReadError;

	Title[11] = '\0';
	if (! strnicmp(Title, "WCS File", 8))
		{
		EffectsLib::LoadQueries = LoadParameters ? (Queries ? 1: 0): 0;	// suppresses copy messages

		if (! LoadMaterials || (LoadToEffects = new EffectsLib()))
			{
			if (! LoadMaterials || (LoadToImages = new ImageLib()))
				{
				// set some global pointers so that things know what libraries to link to
				if (LoadMaterials)
					{
					GlobalApp->LoadToEffectsLib = LoadToEffects;
					GlobalApp->LoadToImageLib = LoadToImages;
					} // if

				while (BytesRead)
					{
					// read block descriptor tag from file 
					if (BytesRead = ReadBlock(fFile, (char *)ReadBuf,
						WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
						{
						TotalRead += BytesRead;
						ReadBuf[8] = 0;
						// read block size from file 
						if (BytesRead = ReadBlock(fFile, (char *)&Size,
							WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
							{
							TotalRead += BytesRead;
							BytesRead = 0;
							if (LoadParameters && ! strnicmp(ReadBuf, "DEMBnds", 8))
								{
								if ((BytesRead = OldBounds.Load(fFile, Size, ByteFlip)) == Size)
									OldBoundsLoaded = 1;
								} // if DEMBnds
							else if (LoadToImages && ! strnicmp(ReadBuf, "Images", 8))
								{
								BytesRead = LoadToImages->Load(fFile, Size, NULL);
								} // if Images
							else if (LoadToEffects && ! strnicmp(ReadBuf, "CoordSys", 8))
								{
								if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
									{
									BytesRead = CurrentCoordSys->Load(fFile, Size, ByteFlip);
									}
								} // if CoordSys
							else if (LoadToEffects && ! strnicmp(ReadBuf, "Matl3D", 8))
								{
								if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
									{
									BytesRead = CurrentMaterial->Load(fFile, Size, ByteFlip);
									}
								} // if Matl3D
							else if (LoadToEffects && ! strnicmp(ReadBuf, "Wave", 8))
								{
								if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
									{
									BytesRead = CurrentWave->Load(fFile, Size, ByteFlip);
									}
								} // if Wave
							else if (! strnicmp(ReadBuf, "Object3D", 8))
								{
								if (LoadToEffects)
									{
									if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
										{
										if ((BytesRead = CurrentObj->LoadWithGeometry(fFile, Size, ByteFlip, LoadMaterials, LoadParameters)) == Size)
											Success = 1;	// we got our man, or at least a man. the last one should be our host
										}
									} // if
								else
									{
									if ((BytesRead = LoadWithGeometry(fFile, Size, ByteFlip, LoadMaterials, LoadParameters)) == Size)
										Success = 1;	// we got our man, or at least a man. the last one should be our host
									} // else
								} // if 3d object
							else if (! fseek(fFile, Size, SEEK_CUR))
								BytesRead = Size;
							TotalRead += BytesRead;
							if (BytesRead != Size)
								{
								Success = 0;
								break;
								} // if error
							} // if size block read 
						else
							break;
						} // if tag block read 
					else
						break;
					} // while 
				} // if image lib
			else
				Success = 0;
			} // if effects lib
		else
			Success = 0;

		if (Success == 1 && CurrentObj && LoadMaterials && LoadToEffects && LoadToImages)
			{
			// need to attach materials to name list
			for (Count = 0; Count < CurrentObj->NumMaterials; Count ++)
				{
				LoadToEffects->SetMakeMaterial(&CurrentObj->NameTable[Count], NULL);
				} // for
			GlobalApp->CopyFromEffectsLib = LoadToEffects;
			GlobalApp->CopyFromImageLib = LoadToImages;
			Copy(this, CurrentObj);
			strcpy(ReadBuf, Name);
			SetUniqueName(GlobalApp->AppEffects, ReadBuf);
			} // if

		if (LoadToEffects)
			delete LoadToEffects;
		if (LoadToImages)
			delete LoadToImages;
		GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
		GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
		GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
		GlobalApp->LoadToImageLib = GlobalApp->AppImages;

		} // if correct file type

ReadError:
	fclose(fFile);
	HeresYourNewFilePathIfYouCare(filename);
	} // if file

if (LoadMaterials)
	{
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

return (Success);

} // Object3DEffect::LoadObjectW3D

/*===========================================================================*/

void Object3DEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double NewWE, NewNS, NewUD, FirstVal, TempVal;
GraphNode *CurNode;

NewWE = (CurBounds->West + CurBounds->East) * 0.5;
NewNS = (CurBounds->North + CurBounds->South) * 0.5;
NewUD = GlobalApp->MainProj->Interactive->ElevationPoint(NewNS, NewWE);

// camera position
FirstVal = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].CurValue;
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetValue(NewNS);
if (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() - FirstVal + NewNS;
	if (TempVal > 90.0)
		TempVal = 90.0;
	if (TempVal < -90.0)
		TempVal = -90.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() - FirstVal + NewNS;
		if (TempVal > 90.0)
			TempVal = 90.0;
		if (TempVal < -90.0)
			TempVal = -90.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
FirstVal = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].CurValue;
AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetValue(NewWE);
if (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() - FirstVal + NewWE);
	while (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() - FirstVal + NewWE);
		} // while
	} // if
if (Absolute == WCS_EFFECT_ABSOLUTE)
	{
	FirstVal = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
	AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].SetValue(NewUD);
	if (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].GetFirstNode(0))
		{
		CurNode->SetValue(CurNode->GetValue() - FirstVal + NewUD);
		while (CurNode = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].GetNextNode(0, CurNode))
			{
			CurNode->SetValue(CurNode->GetValue() - FirstVal + NewUD);
			} // while
		} // if
	} // if

} // Camera::ScaleToDEMBounds

/*===========================================================================*/

// used for loading component files - called from SetRAHostProperties()
// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Object3DEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
MaterialEffect *CurrentMaterial = NULL;
Object3DEffect *CurrentObj = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
WaveEffect *CurrentWave = NULL;
//ULONG ItemTag = 0;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0, Count;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = LoadToImages->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if Matl3D
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if Wave
					else if (! strnicmp(ReadBuf, "Search", 8))
						{
						if (CurrentQuery = new SearchQuery(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentQuery->Load(ffile, Size, ByteFlip);
							}
						} // if search query
					else if (! strnicmp(ReadBuf, "ThemeMap", 8))
						{
						if (CurrentTheme = new ThematicMap(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip);
							}
						} // if thematic map
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentObj->LoadWithGeometry(ffile, Size, ByteFlip, TRUE, TRUE)) == Size)
								Success = 1;	// we got our man, or at least a man. the last one should be our host
							}
						} // if 3d object
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentObj)
	{
	// need to attach materials to name list
	for (Count = 0; Count < CurrentObj->NumMaterials; Count ++)
		{
		LoadToEffects->SetMakeMaterial(&CurrentObj->NameTable[Count], NULL);
		} // for
	// scaling the geographic position for objects that didn't have a geographic position in the file will result in 
	// moving the object from a useful place to one that is not. Instead we will offer to center it.
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load 3D Object", "Do you wish the loaded 3D Object's geographic position\n to be centered in current DEM bounds?"))
			{
			CurrentObj->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	CurrentObj->GeographicInstance = GeographicInstance;
	Copy(this, CurrentObj);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // Object3DEffect::LoadObject

/*===========================================================================*/

unsigned long Object3DEffect::LoadWithGeometry(FILE *ffile, unsigned long ReadSize, short ByteFlip, 
	short LoadMaterials, short LoadParameters)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long TempNames, UVWTableNum = 0, CPVTableNum = 0, VertsRead = 0, PolysRead = 0, NamesRead = 0, LatLoaded = 0;
char TempReceiveShadows;
#ifdef WCS_BUILD_VNS
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_BUILD_VNS

if (LoadMaterials)
	{
	if (NameTable)
		delete [] NameTable;
	NameTable = NULL;
	NumMaterials = 0;
	} // if
if (Vertices)
	FreeVertices();
if (Polygons)
	FreePolygons();
if (UVWTable)
	delete [] UVWTable;
UVWTable = NULL;
NumUVWMaps = 0;
if (CPVTable)
	delete [] CPVTable;
CPVTable = NULL;
NumCPVMaps = 0;

CalcNormals = 1;

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
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (LoadParameters)
							{
							if (BrowseInfo)
								BrowseInfo->FreeAll();
							else
								BrowseInfo = new BrowseData();
							if (BrowseInfo)
								BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_PRIORITY:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_HIRESEDGE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&HiResEdge, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_USEGRADIENT:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&UseGradient, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ABSOLUTE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_RENDEROCCLUDED:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RenderOccluded, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_LATITUDE:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						LatLoaded = 1;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_LONGITUDE:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ELEVATION:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONX:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONY:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ROTATIONZ:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGXx100:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].Load(ffile, Size, ByteFlip);
							AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].ScaleValues(.01);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGX:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGYx100:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].Load(ffile, Size, ByteFlip);
							AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].ScaleValues(.01);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGY:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGZx100:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].Load(ffile, Size, ByteFlip);
							AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].ScaleValues(.01);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SCALINGZ:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONX:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONY:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TRANSLATIONZ:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWINTENSx100:
						{
						if (LoadParameters)
							{
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].Load(ffile, Size, ByteFlip);
							AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].ScaleValues(.01);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWINTENS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWOFFSET:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTBIAS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZPLUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZMINUS:
						{
						if (LoadParameters)
							BytesRead = AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_AAPASSES:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&AAPasses, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_DRAWENABLED:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&DrawEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHADOWSONLY:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ShadowsOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CASTSHADOWS:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS:
						{
						if (LoadParameters)
							{
							BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (TempReceiveShadows)
								{
								ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = 
									ReceiveShadowsCloudSM = 1;
								ReceiveShadowsVolumetric = 0;
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SELFSHADOWED:
						{
						if (LoadParameters)
							{
							BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (TempReceiveShadows)
								{
								ReceiveShadowsTerrain = ReceiveShadowsFoliage =  
									ReceiveShadowsCloudSM = ReceiveShadowsVolumetric = 0;
								ReceiveShadows3DObject = 1;
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSTER:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSFOL:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS3D:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows3DObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSSM:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsCloudSM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSVOL:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsVolumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_UNITS:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&Units, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CALCNORMALS:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&CalcNormals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTCOLORAVAILABLE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&VertexColorsAvailable, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTUVWAVAILABLE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&VertexUVWAvailable, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNHEADING:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&AlignHeading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTICAL:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&AlignVertical, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_HEADINGAXIS:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&HeadingAxis, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTICALAXIS:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&VerticalAxis, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_REVERSEHEADING:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ReverseHeading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZESCALE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEROTATION:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEPOSITION:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RandomizeObj[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_RANDOMIZEVERTEX:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RandomizeVert, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SHOWDETAIL:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ShowDetail, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ISOMETRIC:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&Isometric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNVERTVEC:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&AlignVertVec, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_ALIGNSPECIALVEC:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&AlignSpecialVec, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_GEOINSTANCE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&GeographicInstance, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FRAGMENTOPTIMIZE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&FragmentOptimize, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}

					case WCS_EFFECTS_OBJECT3D_SHADOWMAPWIDTH:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&ShadowMapWidth, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_USEMAPFILE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&UseMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_REGENMAPFILE:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)&RegenMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}

					case WCS_EFFECTS_OBJECT3D_NUMVERTICES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumVertices, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (Vertices)
							{
							FreeVertices();
							} // if
						if (NumVertices > 0)
							{
							if ((Vertices = new Vertex3D[NumVertices]) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMPOLYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumPolys, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (Polygons)
							{
							FreePolygons();
							} // if
						if (NumPolys > 0)
							{
							if ((Polygons = new Polygon3D[NumPolys]) == NULL)
								goto ReadError;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMMATERIALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempNames, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (LoadMaterials)
							{
							if (NameTable)
								{
								delete [] NameTable;
								NameTable = NULL;
								} // if
							NumMaterials = TempNames;
							} // if
						else if (TempNames != NumMaterials)
							goto ReadError;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMUVWTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumUVWMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumUVWMaps > 0)
							{
							if ((UVWTable = new ObjectPerVertexMap[NumUVWMaps]) == NULL)
								goto ReadError;
							UVWTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_NUMCPVTABLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumCPVMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (NumCPVMaps > 0)
							{
							if ((CPVTable = new ObjectPerVertexMap[NumCPVMaps]) == NULL)
								goto ReadError;
							CPVTableNum = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_VERTEX3D:
						{
						if (VertsRead < NumVertices && Vertices)
							BytesRead = Vertices[VertsRead].Load(ffile, Size, ByteFlip, &CPVTable, NumVertices, NumCPVMaps, VertsRead);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						VertsRead ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_POLYGON3D:
						{
						if (PolysRead < NumPolys && Polygons)
							BytesRead = Polygons[PolysRead].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						PolysRead ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_UVWTABLE:
						{
						if (UVWTableNum < NumUVWMaps)
							BytesRead = UVWTable[UVWTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						UVWTableNum ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_CPVTABLE:
						{
						if (CPVTableNum < NumCPVMaps)
							BytesRead = CPVTable[CPVTableNum].Load(ffile, Size, ByteFlip, NumVertices);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						CPVTableNum ++;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[3], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZEHIZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[4], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_SIZELOZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectBounds[5], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FILEPATH:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)FilePath, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_FILENAME:
						{
						if (LoadParameters)
							BytesRead = ReadBlock(ffile, (char *)FileName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_MATERIALNAME:
						{
						if (LoadMaterials)
							{
							if (! NameTable && NumMaterials > 0)
								NameTable = new ObjectMaterialEntry[NumMaterials];
							if (NameTable)
								{
								BytesRead = ReadBlock(ffile, (char *)NameTable[NamesRead++].Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALE:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEX:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEY:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEZ:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATE:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEX:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEY:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEZ:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITION:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONX:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONY:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONZ:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITION:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONX:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONY:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONZ:
						{
						if (LoadParameters)
							{
							if (TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ] = new RootTexture(this, 0, 0, 0))
								{
								BytesRead = TexRoot[WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ]->Load(ffile, Size, ByteFlip);
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMESCALINGZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMEROTATIONZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONX:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONZ:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_QUERY:
						{
						if (LoadParameters)
							{
							BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (MatchName[0])
								{
								Search = (SearchQuery *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName);
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_VNS
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

CalcNormals = (Vertices && Vertices[0].Normalized && Vertices[NumVertices - 1].Normalized) ? 0: 1;
if (! CalcNormals)
	Isometric = 1;
VertexColorsAvailable = (CPVTable) ? 1: 0;
VertexUVWAvailable = (UVWTable) ? 1: 0;
ReadError:

// older file that did not offer geographic positioning
if (! LatLoaded && LoadParameters)
	GeographicInstance = 0;

return (TotalRead);

} // Object3DEffect::LoadWithGeometry

/*===========================================================================*/

int Object3DEffect::SaveObjectW3D(void)
{
char filename[512];
RasterAnimHostProperties Prop;

Prop.PropMask = 0;
Prop.FlagsMask = 0;
Prop.Name = NULL;
if ((strlen(FileName) < strlen("w3d") + 1) 
	|| (FileName[strlen(FileName) - strlen("w3d") - 1] != '.')
	|| (stricmp(&FileName[strlen(FileName) - strlen("w3d")], "w3d")))
	{
	strcat(FileName, ".");
	strcat(FileName, "w3d");
	} // if
strmfp(filename, FilePath, FileName);
Prop.Path = filename;
return (SaveFilePrep(&Prop));

} // Object3DEffect::SaveObjectW3D

/*===========================================================================*/

int Object3DEffect::ComponentOverwriteOK(RasterAnimHostProperties *Prop)
{

// load vertices if needed
if (! Vertices || ! Polygons)
	{
	if (! OpenInputFile(NULL, 0, 0, 0))
		return (0);
	if (! Vertices || ! Polygons)
		return (0);
	} // if

return (1);

} // Object3DEffect::ComponentOverwriteOK

/*===========================================================================*/

int Object3DEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

// load vertices if needed
if (! Vertices || ! Polygons)
	{
	if (! OpenInputFile(NULL, 0, 0, 0))
		return (0);
	} // if
// break supplied file name into parts and copy into object file name
BreakFileName((char *)SuppliedFileName, FilePath, 256, FileName, 64);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords)
	&& BuildFileComponentsList(&Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Queries;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Search");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SearchQuery *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if SearchQuery saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	#ifdef WCS_THEMATIC_MAP
	CurEffect = Themes;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "ThemeMap");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ThematicMap *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if ThemeMap saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_THEMATIC_MAP

	CurEffect = Waves;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Wave");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((WaveEffect *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if wave saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Material3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Matl3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((MaterialEffect *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if material saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Object3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Object3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((Object3DEffect *)CurEffect->Me)->SaveWithGeometry(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if 3D Object saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	while (Queries)
		{
		CurEffect = Queries;
		Queries = Queries->Next;
		delete CurEffect;
		} // while
	while (Themes)
		{
		CurEffect = Themes;
		Themes = Themes->Next;
		delete CurEffect;
		} // while
	while (Material3Ds)
		{
		CurEffect = Material3Ds;
		Material3Ds = Material3Ds->Next;
		delete CurEffect;
		} // while
	while (Object3Ds)
		{
		CurEffect = Object3Ds;
		Object3Ds = Object3Ds->Next;
		delete CurEffect;
		} // while
	while (Waves)
		{
		CurEffect = Waves;
		Waves = Waves->Next;
		delete CurEffect;
		} // while
	} // if

// Object3DEffect
strcpy(StrBuf, "Object3D");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SaveWithGeometry(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Object3DEffect saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // Object3DEffect::SaveObject

/*===========================================================================*/

unsigned long Object3DEffect::SaveWithGeometry(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, NamesWritten, Ct;
int SaveNormals;

unsigned long AnimItemTag[WCS_EFFECTS_OBJECT3D_NUMANIMPAR] = {WCS_EFFECTS_OBJECT3D_LATITUDE,
																	WCS_EFFECTS_OBJECT3D_LONGITUDE,
																	WCS_EFFECTS_OBJECT3D_ELEVATION,
																	WCS_EFFECTS_OBJECT3D_ROTATIONX,
																	WCS_EFFECTS_OBJECT3D_ROTATIONY,
																	WCS_EFFECTS_OBJECT3D_ROTATIONZ,
																	WCS_EFFECTS_OBJECT3D_SCALINGX,
																	WCS_EFFECTS_OBJECT3D_SCALINGY,
																	WCS_EFFECTS_OBJECT3D_SCALINGZ,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONX,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONY,
																	WCS_EFFECTS_OBJECT3D_TRANSLATIONZ,
																	WCS_EFFECTS_OBJECT3D_SHADOWINTENS,
																	WCS_EFFECTS_OBJECT3D_SHADOWOFFSET,
																	WCS_EFFECTS_OBJECT3D_ALIGNVERTBIAS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJSCALEZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJROTATEZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMOBJPOSITIONZMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZPLUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONXMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONYMINUS,
																	WCS_EFFECTS_OBJECT3D_RANDOMVERTPOSITIONZMINUS};
unsigned long TextureItemTag[WCS_EFFECTS_OBJECT3D_NUMTEXTURES] = {WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALE,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJSCALEZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATE,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJROTATEZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITION,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMOBJPOSITIONZ,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITION,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONX,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONY,
																	WCS_EFFECTS_OBJECT3D_TEXRANDOMVERTPOSITIONZ};
#ifdef WCS_THEMATIC_MAP
unsigned long ThemeItemTag[WCS_EFFECTS_OBJECT3D_NUMTHEMES] = {WCS_EFFECTS_OBJECT3D_THEMEROTATIONX,
																	WCS_EFFECTS_OBJECT3D_THEMEROTATIONY,
																	WCS_EFFECTS_OBJECT3D_THEMEROTATIONZ,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGX,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGY,
																	WCS_EFFECTS_OBJECT3D_THEMESCALINGZ,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONX,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONY,
																	WCS_EFFECTS_OBJECT3D_THEMETRANSLATIONZ};
#endif // WCS_THEMATIC_MAP

SaveNormals = ! CalcNormals;

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_HIRESEDGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&HiResEdge)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_USEGRADIENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseGradient)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ABSOLUTE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Absolute)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROCCLUDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RenderOccluded)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block
					else
						goto WriteError;
					} // if texture saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

#ifdef WCS_THEMATIC_MAP
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetTheme(Ct))
		{
		if ((BytesWritten = PrepWriteBlock(ffile, ThemeItemTag[Ct], WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(GetTheme(Ct)->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)GetTheme(Ct)->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // for
#endif // WCS_THEMATIC_MAP

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_AAPASSES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AAPasses)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_DRAWENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHADOWSONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShadowsOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSFOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWS3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows3DObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSSM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsCloudSM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RECEIVESHADOWSVOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsVolumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_UNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Units)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_CALCNORMALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CalcNormals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTCOLORAVAILABLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertexColorsAvailable)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTUVWAVAILABLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VertexUVWAvailable)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNHEADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignHeading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNVERTICAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignVertical)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_HEADINGAXIS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HeadingAxis)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_VERTICALAXIS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VerticalAxis)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_REVERSEHEADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReverseHeading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZESCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEROTATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEPOSITION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeObj[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_RANDOMIZEVERTEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeVert)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHOWDETAIL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowDetail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ISOMETRIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Isometric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNVERTVEC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignVertVec)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_ALIGNSPECIALVEC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignSpecialVec)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_GEOINSTANCE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeographicInstance)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FRAGMENTOPTIMIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FragmentOptimize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SHADOWMAPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowMapWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_USEMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_REGENMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RegenMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMVERTICES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumVertices)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMPOLYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumPolys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMMATERIALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumMaterials)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMUVWTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumUVWMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_NUMCPVTABLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumCPVMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[3])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZEHIZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[4])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_SIZELOZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ObjectBounds[5])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FILEPATH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FilePath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FilePath)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_FILENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FileName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FileName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < NumVertices; Ct ++)
	{
	ItemTag = WCS_EFFECTS_OBJECT3D_VERTEX3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Vertices[Ct].Save(ffile, SaveNormals))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

for (Ct = 0; Ct < NumPolys; Ct ++)
	{
	ItemTag = WCS_EFFECTS_OBJECT3D_POLYGON3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Polygons[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

if (NumMaterials > 0 && NameTable)
	{
	for (NamesWritten = 0; NamesWritten < NumMaterials; NamesWritten ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_OBJECT3D_MATERIALNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(NameTable[NamesWritten].Name) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)NameTable[NamesWritten].Name)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // if

if (UVWTable)
	{
	for (Ct = 0; Ct < NumUVWMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_UVWTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = UVWTable[Ct].Save(ffile, TRUE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if UVWTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

if (CPVTable)
	{
	for (Ct = 0; Ct < NumCPVMaps; Ct ++)
		{
		ItemTag = WCS_EFFECTS_OBJECT3D_CPVTABLE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CPVTable[Ct].Save(ffile, TRUE))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if CPVTable saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if

#ifdef WCS_BUILD_VNS
if (Search)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_QUERY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Search->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Search->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // Object3DEffect::SaveWithGeometry
