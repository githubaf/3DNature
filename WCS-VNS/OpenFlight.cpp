// OpenFlight.cpp
// class implementations for MultiGen-Paradigm OpenFlight support
// Created from scratch 11/13/03 by Frank Weed II
// Copyright 2003 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "OpenFlight.h"
#include "UsefulIO.h"

static long OF_FormatRevision = 0;
static long OF_GroupCounter = 0;

//lint -save -e419

/*===========================================================================*/

long OF_GetRev(void)
{

return OF_FormatRevision;

} // OF_GetRev

/*===========================================================================*/

void OF_SetRev(long Rev)
{

OF_FormatRevision = Rev;

} // OF_SetRev

/*===========================================================================*/
/*===========================================================================*/

OF_Header::OF_Header()
{

memset(this, 0, sizeof(OF_Header));
Opcode = 1;
if (OF_FormatRevision == 1560)
	Length = 280;	// docs list this as 278, but Onyx needs record sizes that are multiples of 4
else if (OF_FormatRevision == 1580)
	Length = 324;
strcpy(ASCII_ID, "db");
FormatRev = OF_GetRev();
UnitMultiplier = 1;
//lint -save -e569
Flags = 0x80000000;
//lint -restore
VertexStorageType = 1;
DatabaseOrigin = 100;

}; // OF_Header::OF_Header

/*===========================================================================*/

OF_Header::~OF_Header()
{

}; // OF_Header::~OF_Header

/*===========================================================================*/

int OF_Header::WriteToFile(FILE *fOF)
{
int RC;
unsigned long i;
unsigned short pad = 0;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(FormatRev, fOF);
(void)PutB32S(EditRev, fOF);
(void)fwrite(DateAndTime, 1, 32, fOF);
(void)PutB16S(NextGroupID, fOF);
(void)PutB16S(NextLODID, fOF);
(void)PutB16S(NextObjectID, fOF);
(void)PutB16S(NextFaceID, fOF);
(void)PutB16S(UnitMultiplier, fOF);
(void)fwrite(&VertexCoordUnits, 1, 1, fOF);
(void)fwrite(&TexWhite, 1, 1, fOF);
(void)PutB32S(Flags, fOF);
for (i = 0; i < 6; i++)
	(void)PutB32S(Reserved1[i], fOF);
(void)PutB32S(ProjectionType, fOF);
for (i = 0; i < 7; i++)
	(void)PutB32S(Reserved2[i], fOF);
(void)PutB16S(NextDOFID, fOF);
(void)PutB16S(VertexStorageType, fOF);
(void)PutB32S(DatabaseOrigin, fOF);
(void)PutB64(&SW_DB_CoordX, fOF);
(void)PutB64(&SW_DB_CoordY, fOF);
(void)PutB64(&DB_DeltaX, fOF);
(void)PutB64(&DB_DeltaY, fOF);
(void)PutB16S(NextSoundID, fOF);
(void)PutB16S(NextPathID, fOF);
for (i = 0; i < 2; i++)
	(void)PutB32S(Reserved3[i], fOF);
(void)PutB16S(NextClipID, fOF);
(void)PutB16S(NextTextID, fOF);
(void)PutB16S(NextBSPID, fOF);
(void)PutB16S(NextSwitchID, fOF);
(void)PutB32S(Reserved4, fOF);
(void)PutB64(&SW_Corn_Lat, fOF);
(void)PutB64(&SW_Corn_Lon, fOF);
(void)PutB64(&NE_Corn_Lat, fOF);
(void)PutB64(&NE_Corn_Lon, fOF);
(void)PutB64(&OriginLat, fOF);
(void)PutB64(&OriginLon, fOF);
(void)PutB64(&LambertUL, fOF);
(void)PutB64(&LambertLL, fOF);
(void)PutB16S(NextLightSourceID, fOF);
(void)PutB16S(NextLightPointID, fOF);
(void)PutB16S(NextRoadID, fOF);
(void)PutB16S(NextCATID, fOF);
for (i = 0; i < 4; i++)
	(void)PutB16S(Reserved5[i], fOF);
(void)PutB32S(EllipsoidModel, fOF);
(void)PutB16S(NextAdaptiveID, fOF);
(void)PutB16S(NextCurveID, fOF);
(void)PutB16S(UTMZone, fOF);

if (OF_FormatRevision == 1560)
	{
	RC = PutB16U(pad, fOF);	// extra pad to make 4 byte alignment for Onyx systems
	} // rev 1560
else if (OF_FormatRevision >= 1580)
	{
	(void)fwrite(Reserved6, 1, 6, fOF);
	(void)PutB64(&DB_DeltaZ, fOF);
	(void)PutB64(&Radius, fOF);
	(void)PutB16S(NextMeshID, fOF);
	(void)PutB16S(NextLPSID, fOF);
	(void)PutB32S(Reserved7, fOF);
	(void)PutB64(&MajorAxis, fOF);
	RC = PutB64(&MinorAxis, fOF);
	} // rev 1580

return RC;

} // OF_Header::OF_WriteHeader

/*===========================================================================*/
/*===========================================================================*/

OF_ExtRef::OF_ExtRef()
{

memset(this, 0, sizeof(OF_ExtRef));
Opcode = 63;
Length = 216;

}; // OF_ExtRef::OF_ExtRef

/*===========================================================================*/

OF_ExtRef::~OF_ExtRef()
{

}; // OF_ExtRef::~OF_ExtRef

/*===========================================================================*/

int OF_ExtRef::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_Path, 1, 200, fOF);
(void)PutB32S(Reserved1, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB16S(ViewAsBB, fOF);
return (PutB16S(Reserved2, fOF));

}; // OF_ExtRef::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Face::OF_Face()
{

memset(this, 0, sizeof(OF_Face));
Opcode = 5;
Length = 80;
TexWhite = 1;
ColorNameIndex = -1;		// this seems to be standard - means unused?
AltColorNameIndex = -1;
DetailTexPatIndex = -1;
TexPatIndex = -1;
MatIndex = -1;
LightMode = 2;
PackedColorPrimary = 0;
PackedColorAlternate = 0;
TexMapIndex = -1;
PrimaryColorIndex = 127;			// color #0, intensity of 127
AlternateColorIndex = 0xffffffff;	// seems to be standard when unused

}; // OF_Face::OF_Face

/*===========================================================================*/

OF_Face::~OF_Face()
{

}; // OF_Face::~OF_Face

/*===========================================================================*/

int OF_Face::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(IRColorCode, fOF);
(void)PutB16S(RelativePri, fOF);
(void)fwrite(&DrawType, 1, 1, fOF);
(void)fwrite(&TexWhite, 1, 1, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(AltColorNameIndex, fOF);
(void)fwrite(&Reserved1, 1, 1, fOF);
(void)fwrite(&Template, 1, 1, fOF);
(void)PutB16S(DetailTexPatIndex, fOF);
(void)PutB16S(TexPatIndex, fOF);
(void)PutB16S(MatIndex, fOF);
(void)PutB16S(SurfaceMatCode, fOF);
(void)PutB16S(FeatureID, fOF);
(void)PutB32S(IRMatCode, fOF);
(void)PutB16S(Transparency, fOF);
(void)fwrite(&LODGenCtrl, 1, 1, fOF);
(void)fwrite(&LineStyleIndex, 1, 1, fOF);
(void)PutB32S(Flags, fOF);
(void)fwrite(&LightMode, 1, 1, fOF);
(void)fwrite(Reserved2, 1, 7, fOF);
(void)PutB32U(PackedColorPrimary, fOF);
(void)PutB32U(PackedColorAlternate, fOF);
(void)PutB16S(TexMapIndex, fOF);
(void)PutB16S(Reserved3, fOF);
(void)PutB32U(PrimaryColorIndex, fOF);
(void)PutB32U(AlternateColorIndex, fOF);

return (PutB32S(Reserved4, fOF));

}; // OF_Face::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Mesh::OF_Mesh()
{

memset(this, 0, sizeof(OF_Mesh));
Opcode = 84;
Length = 80;
ColorNameIndex = -1;
AltColorNameIndex = -1;
DetailTexPatIndex = -1;
TexPatIndex = -1;
MatIndex = -1;
TexMapIndex = -1;
PrimaryColorIndex = 127;
AlternateColorIndex = 0xffffffff;

}; // OF_Mesh::OF_Mesh

/*===========================================================================*/

OF_Mesh::~OF_Mesh()
{

}; // OF_Mesh::~OF_Mesh

/*===========================================================================*/

int OF_Mesh::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(IRColorCode, fOF);
(void)PutB16S(RelativePri, fOF);
(void)fwrite(&DrawType, 1, 1, fOF);
(void)fwrite(&TexWhite, 1, 1, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(AltColorNameIndex, fOF);
(void)fwrite(&Reserved1, 1, 1, fOF);
(void)fwrite(&Template, 1, 1, fOF);
(void)PutB16S(DetailTexPatIndex, fOF);
(void)PutB16S(TexPatIndex, fOF);
(void)PutB16S(MatIndex, fOF);
(void)PutB16S(SurfaceMatCode, fOF);
(void)PutB16S(FeatureID, fOF);
(void)PutB32S(IRMatCode, fOF);
(void)PutB16S(Transparency, fOF);
(void)fwrite(&LODGenCtrl, 1, 1, fOF);
(void)fwrite(&LineStyleIndex, 1, 1, fOF);
(void)PutB32S(Flags, fOF);
(void)fwrite(&LightMode, 1, 1, fOF);
(void)fwrite(Reserved2, 1, 7, fOF);
(void)PutB32U(PackedColorPrimary, fOF);
(void)PutB32U(PackedColorAlternate, fOF);
(void)PutB16S(TexMapIndex, fOF);
(void)PutB16S(Reserved3, fOF);
(void)PutB32U(PrimaryColorIndex, fOF);
(void)PutB32U(AlternateColorIndex, fOF);

return (PutB32S(Reserved4, fOF));

}; // OF_Mesh::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_MeshPrimitive::OF_MeshPrimitive()
{

Opcode = 86;

}; // OF_Mesh::OF_Mesh

/*===========================================================================*/

OF_MeshPrimitive::~OF_MeshPrimitive()
{

}; // OF_Mesh::~OF_Mesh

/*===========================================================================*/

int OF_MeshPrimitive::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(PrimitiveType, fOF);
(void)PutB16U(IndexSize, fOF);

return (PutB32U(VertexCount, fOF));

}; // OF_MeshPrimitive::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_LocalVertexPool::OF_LocalVertexPool()
{

Opcode = 85;

}; // OF_LocalVertexPool::OF_LocalVertexPool

/*===========================================================================*/

OF_LocalVertexPool::~OF_LocalVertexPool()
{

}; // OF_LocalVertexPool::~OF_LocalVertexPool

/*===========================================================================*/

int OF_LocalVertexPool::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32U(NumVerts, fOF);

return(PutB32U(AttributeMask, fOF));

}; // OF_LocalVertexPool::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_PushLevel::OF_PushLevel()
{

Opcode = 10;
Length = 4;

}; // OF_PushLevel::OF_PushLevel

/*===========================================================================*/

OF_PushLevel::~OF_PushLevel()
{

}; // OF_PushLevel::~OF_PushLevel

/*===========================================================================*/

int OF_PushLevel::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);

return (PutB16U(Length, fOF));

}; // OF_PushLevel::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_PopLevel::OF_PopLevel()
{

Opcode = 11;
Length = 4;

}; // OF_PopLevel::OF_PopLevel

/*===========================================================================*/

OF_PopLevel::~OF_PopLevel()
{

}; // OF_PopLevel::~OF_PopLevel

/*===========================================================================*/

int OF_PopLevel::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);

return (PutB16U(Length, fOF));

}; // OF_PopLevel::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Group::OF_Group()
{

memset(this, 0, sizeof(OF_Group));
Opcode = 2;
if (OF_FormatRevision == 1560)
	Length = 32;
else if (OF_FormatRevision == 1580)
	Length = 44;

}; // OF_Group::OF_Group

/*===========================================================================*/

OF_Group::~OF_Group()
{

}; // OF_Group::~OF_Group

/*===========================================================================*/

int OF_Group::WriteToFile(FILE *fOF)
{
int RC;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
OF_GroupCounter++;
memset(ASCII_ID, 0, 8);
(void)sprintf(ASCII_ID, "g%d", OF_GroupCounter);
(void)PutB16S(RelPri, fOF);
(void)PutB16S(Reserved1, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB16S(SFX_ID1, fOF);
(void)PutB16S(SFX_ID2, fOF);
(void)PutB16S(Significance, fOF);
(void)fwrite(&LayerCode, 1, 1, fOF);
(void)fwrite(&Reserved2, 1, 1, fOF);
RC = (int)fwrite(&Reserved3, 1, 4, fOF);

if (OF_FormatRevision >= 1580)
	{
	(void)PutB32S(LoopCount, fOF);
	(void)PutB32F(&LoopDuration, fOF);
	RC = PutB32F(&LastFrameDuration, fOF);
	} // rev 1580

return RC;

}; // OF_Group::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_ColorPalette::OF_ColorPalette()
{

Opcode = 32;
Length = 4228;
memset(Reserved, 0, 128);
memset(Color, 255, 1024 * 4);

}; // OF_ColorPalette::OF_ColorPalette

/*===========================================================================*/

OF_ColorPalette::~OF_ColorPalette()
{

}; // OF_ColorPalette::~OF_ColorPalette

/*===========================================================================*/

int OF_ColorPalette::WriteToFile(FILE *fOF)
{
int RC;
long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(Reserved, 1, 128, fOF);
for (i = 0; i < 1024; i++)
	RC = PutB32S(Color[i], fOF);

return RC;

}; // OF_ColorPalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Continuation::OF_Continuation()
{

memset(this, 0, sizeof(OF_Continuation));
Opcode = 23;

}; // OF_Continuation::OF_Continuation

/*===========================================================================*/

OF_Continuation::~OF_Continuation()
{

}; // OF_Continuation::~OF_Continuation

/*===========================================================================*/

int OF_Continuation::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);

return (PutB32S(ContinueLength, fOF));

}; // OF_Continuation::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Scale::OF_Scale()
{

memset(this, 0, sizeof(OF_Scale));
Opcode = 79;
Length = 44;
CenterX = CenterY = CenterZ = 0.0;
XScale = YScale = ZScale = 1.0f;

}; // OF_Scale::OF_Scale

/*===========================================================================*/

OF_Scale::~OF_Scale()
{

}; // OF_Scale::~OF_Scale

/*===========================================================================*/

int OF_Scale::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);
(void)PutB64(&CenterX, fOF);
(void)PutB64(&CenterY, fOF);
(void)PutB64(&CenterZ, fOF);
(void)PutB32F(&XScale, fOF);
(void)PutB32F(&YScale, fOF);

return (PutB32F(&ZScale, fOF));

}; // OF_Scale::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_TexturePalette::OF_TexturePalette()
{

memset(this, 0, sizeof(OF_TexturePalette));
Opcode = 64;
Length = 216;

}; // OF_TexturePalette::OF_TexturePalette

/*===========================================================================*/

OF_TexturePalette::~OF_TexturePalette()
{

}; // OF_TexturePalette::~OF_TexturePalette

/*===========================================================================*/

int OF_TexturePalette::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(Filename, 1, 200, fOF);
(void)PutB32S(PatternIndex, fOF);
(void)PutB32S(LocationX, fOF);

return (PutB32S(LocationY, fOF));

}; // OF_TexturePalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_TextureAttributeFile::OF_TextureAttributeFile()
{

memset(&TexelsU, 0, sizeof(OF_TextureAttributeFile));
// long TexelsU
// long TexelsV
// long Obsolete1;
// long Obsolete2;
UpX = 64;	// these seem to be the "standard" values
UpY = 0;	//
FileFormatType = -1;	// OSG says this means unused
MinFilter = 1;	// Bilinear
MagFilter = 1;	// Bilinear
WrapUV = 0;	// Repeat
WrapU = 3;	// None
WrapV = 3;	// None
// long Modified;
// long PivotX;
// long PivotY;
EnviroType = 2;	// Decal mode
// long AlphaWhite;
// long Reserved1[8];
RealWorldSizeU = 1.0;	// ???
RealWorldSizeV = 1.0;	// ???
OriginCode = 100;	// guessing that it's the same code as the Database Origin in Header Record
KernelVersion = OF_GetRev();	// ???
// long InternalFormat;
// long ExternalFormat;
// long UsingMIPMAPkernel;
// float MIPMAPkernel[8];
// long Send;
// float LOD0;
// float Scale0;
// float LOD1;
// float Scale1;
// float LOD2;
// float Scale2;
// float LOD3;
// float Scale3;
// float LOD4;
// float Scale4;
// float LOD5;
// float Scale5;
// float LOD6;
// float Scale6;
// float LOD7;
// float Scale7;
// float ControlClamp;
MagFilterAlpha = 2;	// None
MagFilterColor = 2;	// None
// float Reserved2;
// float Reserved3[8];
// double LamberCM;
// double LamberUL;
// double LamberLL;
// double Reserved4;
// float Reserved5[5];
// long UsingTX_DETAIL;
// long J_Arg;
// long K_Arg;
// long M_Arg;
// long N_Arg;
// long Scramble_Arg;
// long Using_TX_TILE;
LL_U = 0.0f;
LL_V = 0.0f;
UR_U = 1.0f;
UR_V = 1.0f;
Projection = 7;	// Undefined
EarthModel = 0;	// WGS84
// long Reserved6;
// long UTMzone;
ImageOrigin = 1;
// long GeospecificPointsUnits;
// long Reserved7;
// long Reserved8;
// long Hemisphere;
// long Reserved9;
// long Reserved10;
// long Reserved11[149];
// char Comments[512];
// long Reserved12[13];
AttributeFileVersion =  OF_GetRev();	// ???
// long NumGeospecificControlPoints;
// long NumSubtextures;

}; // OF_TextureAttributeFile::OF_TextureAttributeFile

/*===========================================================================*/

OF_TextureAttributeFile::~OF_TextureAttributeFile()
{

}; // OF_TextureAttributeFile::~OF_TextureAttributeFile

/*===========================================================================*/

// _VERY_ important note on TAF's!!!
// http://www.multigen-paradigm.com/ubb/Forum8/HTML/000253.html
int OF_TextureAttributeFile::WriteToFile(FILE *fATTR)
{
int RC;
unsigned long i, junk = 0;

(void)PutB32S(TexelsU, fATTR);
(void)PutB32S(TexelsV, fATTR);
(void)PutB32S(Obsolete1, fATTR);
(void)PutB32S(Obsolete2, fATTR);
(void)PutB32S(UpX, fATTR);
(void)PutB32S(UpY, fATTR);
(void)PutB32S(FileFormatType, fATTR);
(void)PutB32S(MinFilter, fATTR);
(void)PutB32S(MagFilter, fATTR);
(void)PutB32S(WrapUV, fATTR);
(void)PutB32S(WrapU, fATTR);
(void)PutB32S(WrapV, fATTR);
(void)PutB32S(Modified, fATTR);
(void)PutB32S(PivotX, fATTR);
(void)PutB32S(PivotY, fATTR);
(void)PutB32S(EnviroType, fATTR);
(void)PutB32S(AlphaWhite, fATTR);
for (i = 0; i < 8; i++)
	(void)PutB32S(Reserved1[i], fATTR);
(void)PutB32U(junk, fATTR);	// add undocumented padding
(void)PutB64(&RealWorldSizeU, fATTR);
(void)PutB64(&RealWorldSizeV, fATTR);
(void)PutB32S(OriginCode, fATTR);
(void)PutB32S(KernelVersion, fATTR);
(void)PutB32S(InternalFormat, fATTR);
(void)PutB32S(ExternalFormat, fATTR);
(void)PutB32S(UsingMIPMAPkernel, fATTR);
for (i = 0; i < 8; i++)
	(void)PutL32F(&MIPMAPkernel[i], fATTR);
(void)PutB32S(Send, fATTR);
(void)PutL32F(&LOD0, fATTR);	// read the html above
(void)PutL32F(&Scale0, fATTR);
(void)PutL32F(&LOD1, fATTR);
(void)PutL32F(&Scale1, fATTR);
(void)PutL32F(&LOD2, fATTR);
(void)PutL32F(&Scale2, fATTR);
(void)PutL32F(&LOD3, fATTR);
(void)PutL32F(&Scale3, fATTR);
(void)PutL32F(&LOD4, fATTR);
(void)PutL32F(&Scale4, fATTR);
(void)PutL32F(&LOD5, fATTR);
(void)PutL32F(&Scale5, fATTR);
(void)PutL32F(&LOD6, fATTR);
(void)PutL32F(&Scale6, fATTR);
(void)PutL32F(&LOD7, fATTR);
(void)PutL32F(&Scale7, fATTR);
(void)PutL32F(&ControlClamp, fATTR);
(void)PutB32S(MagFilterAlpha, fATTR);
(void)PutB32S(MagFilterColor, fATTR);
(void)PutL32F(&Reserved2, fATTR);
for (i = 0; i < 8; i++)
	(void)PutL32F(&Reserved3[i], fATTR);
(void)PutB64(&LamberCM, fATTR);
(void)PutB64(&LamberUL, fATTR);
(void)PutB64(&LamberLL, fATTR);
(void)PutB64(&Reserved4, fATTR);
for (i = 0; i < 5; i++)
	(void)PutL32F(&Reserved5[i], fATTR);
(void)PutB32S(UsingTX_DETAIL, fATTR);
(void)PutB32S(J_Arg, fATTR);
(void)PutB32S(K_Arg, fATTR);
(void)PutB32S(M_Arg, fATTR);
(void)PutB32S(N_Arg, fATTR);
(void)PutB32S(Scramble_Arg, fATTR);
(void)PutB32S(Using_TX_TILE, fATTR);
(void)PutL32F(&LL_U, fATTR);
(void)PutL32F(&LL_V, fATTR);
(void)PutL32F(&UR_U, fATTR);
(void)PutL32F(&UR_V, fATTR);
(void)PutB32S(Projection, fATTR);
(void)PutB32S(EarthModel, fATTR);
(void)PutB32S(Reserved6, fATTR);
(void)PutB32S(UTMzone, fATTR);
(void)PutB32S(ImageOrigin, fATTR);
(void)PutB32S(GeospecificPointsUnits, fATTR);
(void)PutB32S(Reserved7, fATTR);
(void)PutB32S(Reserved8, fATTR);
(void)PutB32S(Hemisphere, fATTR);
(void)PutB32S(Reserved9, fATTR);
(void)PutB32S(Reserved10, fATTR);
for (i = 0; i < 149; i++)
	(void)PutB32S(Reserved11[i], fATTR);
(void)fwrite(Comments, 1, 512, fATTR);
for (i = 0; i < 13; i++)
	(void)PutB32S(Reserved12[i], fATTR);
(void)PutB32U(junk, fATTR);	// add undocumented padding
(void)PutB32S(AttributeFileVersion, fATTR);
(void)PutB32S(NumGeospecificControlPoints, fATTR);
// Any GeoSpecificControlPoint records would need to be written now
RC =  PutB32S(NumSubtextures, fATTR);
// Any Subtexture records would need to be written now

return RC;

}; // OF_TextureAttributeFile::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_LOD::OF_LOD()
{

memset(this, 0, sizeof(OF_LOD));
Opcode = 73;
if (OF_FormatRevision == 1560)
	Length = 72;
else if (OF_FormatRevision == 1580)
	Length = 80;

}; // OF_LOD::OF_LOD

/*===========================================================================*/

OF_LOD::~OF_LOD()
{

}; // OF_LOD::~OF_LOD

/*===========================================================================*/

int OF_LOD::WriteToFile(FILE *fOF)
{
int RC;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(Reserved, fOF);
(void)PutB64(&SwitchIn, fOF);
(void)PutB64(&SwitchOut, fOF);
(void)PutB16S(SFX_ID1, fOF);
(void)PutB16S(SFX_ID2, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB64(&CenterCoordX, fOF);
(void)PutB64(&CenterCoordY, fOF);
(void)PutB64(&CenterCoordZ, fOF);

RC = PutB64(&TransitionRange, fOF);

if (OF_FormatRevision >= 1580)
	RC = PutB64(&SignificantSize, fOF);

return RC;

}; // OF_LOD::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_VertexList::OF_VertexList()
{

memset(this, 0, sizeof(OF_VertexList));
Opcode = 72;

}; // OF_VertexList::OF_VertexList

/*===========================================================================*/

OF_VertexList::~OF_VertexList()
{

}; // OF_VertexList::~OF_VertexList

/*===========================================================================*/

int OF_VertexList::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
return (PutB16U(Length, fOF));

}; // OF_VertexList::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_InstanceDefinition::OF_InstanceDefinition()
{

memset(this, 0, sizeof(OF_InstanceDefinition));
Opcode = 62;
Length = 8;

}; // OF_InstanceDefinition::OF_InstanceDefinition

/*===========================================================================*/

OF_InstanceDefinition::~OF_InstanceDefinition()
{

}; // OF_InstanceDefinition::~OF_InstanceDefinition

/*===========================================================================*/

int OF_InstanceDefinition::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(Reserved, fOF);

return (PutB16S(InstanceDefNum, fOF));

}; // OF_InstanceDefinition::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_InstanceReference::OF_InstanceReference()
{

memset(this, 0, sizeof(OF_InstanceReference));
Opcode = 61;
Length = 8;

}; // OF_InstanceReference::OF_InstanceReference

/*===========================================================================*/

OF_InstanceReference::~OF_InstanceReference()
{

}; // OF_InstanceReference::~OF_InstanceReference

/*===========================================================================*/

int OF_InstanceReference::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(Reserved, fOF);

return (PutB16S(InstanceDefNum, fOF));

}; // OF_InstanceReference::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_BoundingSphere::OF_BoundingSphere()
{

memset(this, 0, sizeof(OF_BoundingSphere));
Opcode = 105;
Length = 16;

}; // OF_BoundingSphere::OF_BoundingSphere

/*===========================================================================*/

OF_BoundingSphere::~OF_BoundingSphere()
{

}; // OF_BoundingSphere::~OF_BoundingSphere

/*===========================================================================*/

int OF_BoundingSphere::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);

return (PutB64(&Radius, fOF));

}; // OF_BoundingSphere::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_BoundingVolumeCenter::OF_BoundingVolumeCenter()
{

memset(this, 0, sizeof(OF_BoundingVolumeCenter));
Opcode = 108;
Length = 32;

}; // OF_BoundingVolumeCenter::OF_BoundingVolumeCenter

/*===========================================================================*/

OF_BoundingVolumeCenter::~OF_BoundingVolumeCenter()
{

}; // OF_BoundingVolumeCenter::~OF_BoundingVolumeCenter

/*===========================================================================*/

int OF_BoundingVolumeCenter::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);
(void)PutB64(&CenterX, fOF);
(void)PutB64(&CenterY, fOF);

return (PutB64(&CenterZ, fOF));

}; // OF_BoundingVolumeCenter::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_BoundingVolumeOrientation::OF_BoundingVolumeOrientation()
{

memset(this, 0, sizeof(OF_BoundingVolumeOrientation));
Opcode = 109;
Length = 32;

}; // OF_BoundingVolumeOrientation::OF_BoundingVolumeOrientation

/*===========================================================================*/

OF_BoundingVolumeOrientation::~OF_BoundingVolumeOrientation()
{

}; // OF_BoundingVolumeOrientation::~OF_BoundingVolumeOrientation

/*===========================================================================*/

int OF_BoundingVolumeOrientation::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);
(void)PutB64(&Yaw, fOF);
(void)PutB64(&Pitch, fOF);

return (PutB64(&Roll, fOF));

}; // OF_BoundingVolumeOrientation::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_EyepointFields::OF_EyepointFields()
{

memset(&RotCtrX, 0, sizeof(OF_EyepointFields));
Scale = 1.0f;

}; // OF_EyepointFields::OF_EyepointFields

/*===========================================================================*/

OF_EyepointFields::~OF_EyepointFields()
{

}; // OF_EyepointFields::OF_EyepointFields

/*===========================================================================*/

int OF_EyepointFields::WriteToFile(FILE *fOF)
{
long i;

(void)PutB64(&RotCtrX, fOF);
(void)PutB64(&RotCtrY, fOF);
(void)PutB64(&RotCtrZ, fOF);
(void)PutB32F(&Roll, fOF);
(void)PutB32F(&Pitch, fOF);
(void)PutB32F(&Yaw, fOF);
for (i = 0; i < 16; i++)
	(void)PutB32F(&RotationMatrix[i], fOF);
(void)PutB32F(&FieldOfView, fOF);
(void)PutB32F(&Scale, fOF);
(void)PutB32F(&NearClipPlane, fOF);
(void)PutB32F(&FarClipPlane, fOF);
for (i = 0; i < 16; i++)
	(void)PutB32F(&FlythroughMatrix[i], fOF);
(void)PutB32F(&EyepointX, fOF);
(void)PutB32F(&EyepointY, fOF);
(void)PutB32F(&EyepointZ, fOF);
(void)PutB32F(&FlythroughYaw, fOF);
(void)PutB32F(&FlythroughPitch, fOF);
(void)PutB32F(&EyepointDirectVecI, fOF);
(void)PutB32F(&EyepointDirectVecJ, fOF);
(void)PutB32F(&EyepointDirectVecK, fOF);
(void)PutB32S(NoFlythrough, fOF);
(void)PutB32S(OrthoView, fOF);
(void)PutB32S(ValidEyepoint, fOF);
(void)PutB32S(ImageOffsetX, fOF);
(void)PutB32S(ImageOffsetY, fOF);
(void)PutB32S(ImageZoom, fOF);
for (i = 0; i < 4; i++)
	(void)PutB64(&Reserved1[i], fOF);

return (PutB32S(Reserved2, fOF));

}; // OF_BoundingVolumeOrientation::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_TrackplaneFields::OF_TrackplaneFields()
{

memset(&ValidTrackplane, 0, sizeof(OF_TrackplaneFields));

}; // OF_TrackplaneFields::OF_TrackplaneFields

/*===========================================================================*/

OF_TrackplaneFields::~OF_TrackplaneFields()
{

}; // OF_TrackplaneFields::~OF_TrackplaneFields

/*===========================================================================*/

int OF_TrackplaneFields::WriteToFile(FILE *fOF)
{

(void)PutB32S(ValidTrackplane, fOF);
(void)PutB32S(Reserved1, fOF);
(void)PutB64(&TrackplaneOriginX, fOF);
(void)PutB64(&TrackplaneOriginY, fOF);
(void)PutB64(&TrackplaneOriginZ, fOF);
(void)PutB64(&TrackplaneAlignmentX, fOF);
(void)PutB64(&TrackplaneAlignmentY, fOF);
(void)PutB64(&TrackplaneAlignmentZ, fOF);
(void)PutB64(&TrackplanePlaneX, fOF);
(void)PutB64(&TrackplanePlaneY, fOF);
(void)PutB64(&TrackplnaePlaneZ, fOF);
(void)fwrite(&GridVisible, 1, 1, fOF);
(void)fwrite(&GridType, 1, 1, fOF);
(void)fwrite(&GridUnderFlag, 1, 1, fOF);
(void)fwrite(&Reserved2, 1, 1, fOF);
(void)PutB32F(&RadialGridAngle, fOF);
(void)PutB64(&GridSpacingX, fOF);
(void)PutB64(&GridSpacingY, fOF);
(void)fwrite(&RadialGridSpacingDirection, 1, 1, fOF);
(void)fwrite(&RectangularGridSpacingDirection, 1, 1, fOF);
(void)fwrite(&SnapCursorToGrid, 1, 1, fOF);
(void)fwrite(&Reserved3, 1, 1, fOF);
(void)PutB32S(Reserved4, fOF);
(void)PutB64(&GridSize, fOF);
(void)PutB32S(VisibleGridQuadrantsMask, fOF);

return (PutB32S(Reserved5, fOF));

}; // OF_TrackplaneFields::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_EyepointAndTrackplanePalette::OF_EyepointAndTrackplanePalette()
{

Opcode = 83;
Length = 4008;
Reserved = 0;

} // OF_EyepointAndTrackplanePalette::OF_EyepointAndTrackplanePalette

/*===========================================================================*/

OF_EyepointAndTrackplanePalette::~OF_EyepointAndTrackplanePalette()
{

}; // OF_EyepointAndTrackplanePalette::~OF_EyepointAndTrackplanePalette

/*===========================================================================*/

int OF_EyepointAndTrackplanePalette::WriteToFile(FILE *fOF)
{
int RC;
long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);
for (i = 0; i < 10; i++)
	(void)Eyepoint[i].WriteToFile(fOF);
for (i = 0; i < 10; i++)
	RC = Trackplane[i].WriteToFile(fOF);

return RC;

}; // OF_EyepointAndTrackplanePalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Vertex_withColor::OF_Vertex_withColor()
{

memset(this, 0, sizeof(OF_Vertex_withColor));
Opcode = 68;
Length = 40;
ColorNameIndex = -1;
Flags = 0x1000;	// Packed color
PackedColorABGR = -1;

}; // OF_Vertex_withColor::OF_Vertex_withColor

/*===========================================================================*/

OF_Vertex_withColor::~OF_Vertex_withColor()
{

}; // OF_Vertex_withColor::~OF_Vertex_withColor

/*===========================================================================*/

int OF_Vertex_withColor::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(Flags, fOF);
(void)PutB64(&VertexCoordX, fOF);
(void)PutB64(&VertexCoordY, fOF);
(void)PutB64(&VertexCoordZ, fOF);
(void)PutB32S(PackedColorABGR, fOF);

return (PutB32U(VertexColorIndex, fOF));

}; // OF_Vertex_withColor::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Vertex_withColorNormal::OF_Vertex_withColorNormal()
{

memset(this, 0, sizeof(OF_Vertex_withColorNormal));
Opcode = 69;
Length = 52;
ColorNameIndex = -1;
Flags = 0x1000;	// Packed color
PackedColorABGR = -1;

}; // OF_Vertex_withColorNormal::OF_Vertex_withColorNormal

/*===========================================================================*/

OF_Vertex_withColorNormal::~OF_Vertex_withColorNormal()
{

}; // OF_Vertex_withColorNormal::~OF_Vertex_withColorNormal

/*===========================================================================*/

int OF_Vertex_withColorNormal::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(Flags, fOF);
(void)PutB64(&VertexCoordX, fOF);
(void)PutB64(&VertexCoordY, fOF);
(void)PutB64(&VertexCoordZ, fOF);
(void)PutB32F(&VertexNormalI, fOF);
(void)PutB32F(&VertexNormalJ, fOF);
(void)PutB32F(&VertexNormalK, fOF);
(void)PutB32S(PackedColorABGR, fOF);

return (PutB32U(VertexColorIndex, fOF));

}; // OF_Vertex_withColorNormal::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Vertex_withColorUV::OF_Vertex_withColorUV()
{

memset(this, 0, sizeof(OF_Vertex_withColorUV));
Opcode = 71;
Length = 48;
ColorNameIndex = -1;
Flags = 0x1000;	// Packed color
PackedColorABGR = -1;

}; // OF_Vertex_withColorUV::OF_Vertex_withColorUV

/*===========================================================================*/

OF_Vertex_withColorUV::~OF_Vertex_withColorUV()
{

}; // OF_Vertex_withColorUV::~OF_Vertex_withColorUV

/*===========================================================================*/

int OF_Vertex_withColorUV::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(Flags, fOF);
(void)PutB64(&VertexCoordX, fOF);
(void)PutB64(&VertexCoordY, fOF);

// catch a NaN
assert(_finite(VertexCoordZ));

(void)PutB64(&VertexCoordZ, fOF);
(void)PutB32F(&TextureCoordU, fOF);
(void)PutB32F(&TextureCoordV, fOF);
(void)PutB32S(PackedColorABGR, fOF);

return (PutB32U(VertexColorIndex, fOF));

}; // OF_Vertex_withColorUV::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Vertex_withColorNormalUV::OF_Vertex_withColorNormalUV()
{

memset(this, 0, sizeof(OF_Vertex_withColorNormalUV));
Opcode = 70;
if (OF_FormatRevision == 1560)
	Length = 60;
else if (OF_FormatRevision == 1580)
	Length = 64;
ColorNameIndex = 0;
Flags = 0x2000;	// No color
//PackedColorABGR = 0xffffffff;

}; // OF_Vertex_withColorNormalUV::OF_Vertex_withColorNormalUV

/*===========================================================================*/

OF_Vertex_withColorNormalUV::~OF_Vertex_withColorNormalUV()
{

}; // OF_Vertex_withColorNormalUV::~OF_Vertex_withColorNormalUV

/*===========================================================================*/

int OF_Vertex_withColorNormalUV::WriteToFile(FILE *fOF)
{
int RC;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB16S(ColorNameIndex, fOF);
(void)PutB16S(Flags, fOF);
(void)PutB64(&VertexCoordX, fOF);
(void)PutB64(&VertexCoordY, fOF);
(void)PutB64(&VertexCoordZ, fOF);
(void)PutB32F(&VertexNormalI, fOF);
(void)PutB32F(&VertexNormalJ, fOF);
(void)PutB32F(&VertexNormalK, fOF);
(void)PutB32F(&TextureCoordU, fOF);
(void)PutB32F(&TextureCoordV, fOF);
(void)PutB32S(PackedColorABGR, fOF);
RC = PutB32U(VertexColorIndex, fOF);

if (OF_FormatRevision >= 1580)
	RC = PutB32S(Reserved, fOF);

return RC;

}; // OF_Vertex_withColorNormalUV::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_VertexPalette::OF_VertexPalette()
{

Opcode = 67;
Length = 8;
TotalLength = 8;

}; // OF_VertexPalette::OF_VertexPalette

/*===========================================================================*/

OF_VertexPalette::~OF_VertexPalette()
{

}; // OF_VertexPalette::~OF_VertexPalette

/*===========================================================================*/

int OF_VertexPalette::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);

return (PutB32S(TotalLength, fOF));

}; // OF_VertexPalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_LevelOfDetail::OF_LevelOfDetail()
{

memset(this, 0, sizeof(OF_LevelOfDetail));
Opcode = 73;
Length = 80;

}; // OF_LevelOfDetail::OF_LevelOfDetail

/*===========================================================================*/

OF_LevelOfDetail::~OF_LevelOfDetail()
{

}; // OF_LevelOfDetail::~OF_LevelOfDetail

/*===========================================================================*/

int OF_LevelOfDetail::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(Reserved, fOF);
(void)PutB64(&SwitchInDist, fOF);
(void)PutB64(&SwitchOutDist, fOF);
(void)PutB16S(SFX_ID1, fOF);
(void)PutB16S(SFX_ID2, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB64(&CenterCoordX, fOF);
(void)PutB64(&CenterCoordY, fOF);
(void)PutB64(&CenterCoordZ, fOF);
(void)PutB64(&TransitionRange, fOF);

return (PutB64(&SignificantSize, fOF));

}; // OF_LevelOfDetail::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Object::OF_Object()
{

memset(this, 0, sizeof(OF_Object));
Opcode = 4;
Length = 28;

}; // OF_Object::OF_Object

/*===========================================================================*/

OF_Object::~OF_Object()
{

}; // OF_Object::~OF_Object

/*===========================================================================*/

int OF_Object::WriteToFile(FILE *fOF)
{

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB16S(RelPri, fOF);
(void)PutB16U(Trans, fOF);
(void)PutB16S(SFX_ID1, fOF);
(void)PutB16S(SFX_ID2, fOF);
(void)PutB16S(Significance, fOF);

return (PutB16S(Reserved, fOF));

}; // OF_Object::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_MaterialPalette::OF_MaterialPalette()
{

memset(this, 0, sizeof(OF_MaterialPalette));
Opcode = 113;
Length = 84;
//lint -save -e569
Flags = 0x80000000;
//lint -restore

}; // OF_MaterialPalette::OF_MaterialPalette

/*===========================================================================*/

OF_MaterialPalette::~OF_MaterialPalette()
{

}; // OF_MaterialPalette::~OF_MaterialPalette

/*===========================================================================*/

int OF_MaterialPalette::WriteToFile(FILE *fOF)
{
unsigned long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(MatIndex, fOF);
(void)fwrite(MatName, 1, 12, fOF);
(void)PutB32S(Flags, fOF);
for (i = 0; i < 3; i++)
	(void)PutB32F(&Ambient[i], fOF);
for (i = 0; i < 3; i++)
	(void)PutB32F(&Diffuse[i], fOF);
for (i = 0; i < 3; i++)
	(void)PutB32F(&Specular[i], fOF);
for (i = 0; i < 3; i++)
	(void)PutB32F(&Emissive[i], fOF);
(void)PutB32F(&Shininess, fOF);
(void)PutB32F(&Alpha, fOF);

return (PutB32S(Reserved, fOF));

}; // OF_MaterialPalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Matrix::OF_Matrix()
{

memset(this, 0, sizeof(OF_Matrix));
Opcode = 49;
Length = 68;

}; // OF_Matrix::OF_Matrix

/*===========================================================================*/

OF_Matrix::~OF_Matrix()
{

}; // OF_Matrix::~OF_Matrix

/*===========================================================================*/

void OF_Matrix::Clear(void)
{
unsigned long i;

for (i = 0; i < 16; i++)
	Matrix[i] = 0.0f;

} // OF_Matrix::Clear

/*===========================================================================*/

int OF_Matrix::WriteToFile(FILE *fOF)
{
int RC;
unsigned long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
for (i = 0; i < 16; i++)
	RC = PutB32F(&Matrix[i], fOF);

return RC;

}; // OF_Matrix::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_LightSourcePalette::OF_LightSourcePalette()
{

memset(this, 0, sizeof(OF_LightSourcePalette));
Opcode = 102;
Length = 240;
PaletteIndex = 1;
AmbientRGBA[0] = 0.0f;
AmbientRGBA[1] = 0.0f;
AmbientRGBA[2] = 0.0f;
AmbientRGBA[3] = 1.0f;
DiffuseRGBA[0] = 1.0f;
DiffuseRGBA[1] = 1.0f;
DiffuseRGBA[2] = 1.0f;
DiffuseRGBA[3] = 1.0f;
SpecularRGBA[0] = 1.0f;
SpecularRGBA[1] = 1.0f;
SpecularRGBA[2] = 1.0f;
SpecularRGBA[3] = 1.0f;
//LightType;
SpotExpDropoff = 0.0f;
SpotCutoffAngle = 90.0f;	// in degrees
Yaw = 135.0f;
Pitch = 45.0f;
ConstantAttenuationCoeff = 1.0f;
LinearAttenuationCoeff = 0.0f;
QuadraticAttenuationCoeff = 0.0f;
ModelingLight = 1;

}; // OF_LightSourcePalette::OF_LightSourcePalette

/*===========================================================================*/

OF_LightSourcePalette::~OF_LightSourcePalette()
{

}; // OF_LightSourcePalette::~OF_LightSourcePalette

/*===========================================================================*/

int OF_LightSourcePalette::WriteToFile(FILE *fOF)
{
int RC;
unsigned long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(PaletteIndex, fOF);
for (i = 0; i < 2; i++)
	(void)PutB32S(Reserved0[i], fOF);
(void)fwrite(LightSourceName, 1, 20, fOF);
(void)PutB32S(Reserved1, fOF);
for (i = 0; i < 4; i++)
	(void)PutB32F(&AmbientRGBA[i], fOF);
for (i = 0; i < 4; i++)
	(void)PutB32F(&DiffuseRGBA[i], fOF);
for (i = 0; i < 4; i++)
	(void)PutB32F(&SpecularRGBA[i], fOF);
(void)PutB32S(LightType, fOF);
for (i = 0; i < 10; i++)
	(void)PutB32S(Reserved2[i], fOF);
(void)PutB32F(&SpotExpDropoff, fOF);
(void)PutB32F(&SpotCutoffAngle, fOF);	// in degrees
(void)PutB32F(&Yaw, fOF);
(void)PutB32F(&Pitch, fOF);
(void)PutB32F(&ConstantAttenuationCoeff, fOF);
(void)PutB32F(&LinearAttenuationCoeff, fOF);
(void)PutB32F(&QuadraticAttenuationCoeff, fOF);
(void)PutB32S(ModelingLight, fOF);
for (i = 0; i < 19; i++)
	RC = PutB32S(Reserved3[i], fOF);

return RC;

}; // OF_LightSourcePalette::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_LightSource::OF_LightSource()
{

memset(this, 0, sizeof(OF_LightSource));
Opcode = 101;
Length = 64;

}; // OF_LightSource::OF_LightSource

/*===========================================================================*/

OF_LightSource::~OF_LightSource()
{

}; // OF_LightSource::~OF_LightSource

/*===========================================================================*/

int OF_LightSource::WriteToFile(FILE *fOF)
{
unsigned long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)fwrite(ASCII_ID, 1, 8, fOF);
(void)PutB32S(Reserved0, fOF);
(void)PutB32S(LightPaletteIndex, fOF);
(void)PutB32S(Reserved1, fOF);
(void)PutB32S(Flags, fOF);
(void)PutB32S(Reserved2, fOF);
for (i = 0; i < 3; i++)
	(void)PutB64(&PositionXYZ[i], fOF);
(void)PutB32F(&Yaw, fOF);


return (PutB32F(&Pitch, fOF));

}; // OF_LightSource::WriteToFile

/*===========================================================================*/
/*===========================================================================*/

OF_Translate::OF_Translate()
{

memset(this, 0, sizeof(OF_Translate));
Opcode = 78;
Length = 56;

}; // OF_Translate::OF_Translate

/*===========================================================================*/

OF_Translate::~OF_Translate()
{

}; // OF_Translate::~OF_Translate

/*===========================================================================*/

int OF_Translate::WriteToFile(FILE *fOF)
{
int RC;
long i;

(void)PutB16S(Opcode, fOF);
(void)PutB16U(Length, fOF);
(void)PutB32S(Reserved, fOF);
for (i = 0; i < 3; i++)
	(void)PutB64(&FromXYZ[i], fOF);
for (i = 0; i < 3; i++)
	RC = PutB64(&DeltaXYZ[i], fOF);

return RC;

}; // OF_Translate::WriteToFile

/*===========================================================================*/

//lint -restore
