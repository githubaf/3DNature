// OpenFlight.h
// header file for MultiGen-Paradigm OpenFlight support
// Created from scratch 11/13/03 by Frank Weed II
// Copyright 2003 3D Nature, LLC. All rights reserved.

#include "stdafx.h"

long OF_GetRev(void);
void OF_SetRev(long Rev);

struct InstanceInfo
	{
	double	X, Y, Z;
	long	FlipInUse, InUse;
	short	DefNum, FlipDefNum;
	};

class OF_Header
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long FormatRev;
		long EditRev;
		char DateAndTime[32];
		short NextGroupID;
		short NextLODID;
		short NextObjectID;
		short NextFaceID;
		short UnitMultiplier;
		char VertexCoordUnits;
		char TexWhite;
		long Flags;
		long Reserved1[6];
		long ProjectionType;
		long Reserved2[7];
		short NextDOFID;
		short VertexStorageType;
		long DatabaseOrigin;
		double SW_DB_CoordX;
		double SW_DB_CoordY;
		double DB_DeltaX;
		double DB_DeltaY;
		short NextSoundID;
		short NextPathID;
		long Reserved3[2];
		short NextClipID;
		short NextTextID;
		short NextBSPID;
		short NextSwitchID;
		long Reserved4;
		double SW_Corn_Lat;
		double SW_Corn_Lon;
		double NE_Corn_Lat;
		double NE_Corn_Lon;
		double OriginLat;
		double OriginLon;
		double LambertUL;
		double LambertLL;
		short NextLightSourceID;
		short NextLightPointID;
		short NextRoadID;
		short NextCATID;
		short Reserved5[4];
		long EllipsoidModel;
		short NextAdaptiveID;
		short NextCurveID;
		short UTMZone;
		char Reserved6[6];
		double DB_DeltaZ;
		double Radius;
		short NextMeshID;
		short NextLPSID;
		long Reserved7;
		double MajorAxis;
		double MinorAxis;

		OF_Header();
		~OF_Header();
		int WriteToFile(FILE *fOF);

	}; // class OF_Header

class OF_ExtRef
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_Path[200];
		long Reserved1;
		long Flags;
		short ViewAsBB;
		short Reserved2;

		OF_ExtRef();
		~OF_ExtRef();
		int WriteToFile(FILE *fOF);

	}; // class OF_ExtRef

class OF_Face
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long IRColorCode;
		short RelativePri;
		char DrawType;
		char TexWhite;
		short ColorNameIndex;
		short AltColorNameIndex;
		char Reserved1;
		char Template;	// billboard
		short DetailTexPatIndex;
		short TexPatIndex;
		short MatIndex;
		short SurfaceMatCode;
		short FeatureID;
		long IRMatCode;
		short Transparency;
		unsigned char LODGenCtrl;
		unsigned char LineStyleIndex;
		long Flags;
		unsigned char LightMode;
		char Reserved2[7];
		unsigned long PackedColorPrimary;	// abgr
		unsigned long PackedColorAlternate;	// abgr
		short TexMapIndex;
		short Reserved3;
		unsigned long PrimaryColorIndex;
		unsigned long AlternateColorIndex;
		long Reserved4;

		OF_Face();
		~OF_Face();
		int WriteToFile(FILE *fOF);

	}; // class OF_Face

class OF_Mesh
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long IRColorCode;
		short RelativePri;
		char DrawType;
		char TexWhite;
		short ColorNameIndex;
		short AltColorNameIndex;
		char Reserved1;
		char Template;	// billboard
		short DetailTexPatIndex;
		short TexPatIndex;
		short MatIndex;
		short SurfaceMatCode;
		short FeatureID;
		long IRMatCode;
		short Transparency;
		unsigned char LODGenCtrl;
		unsigned char LineStyleIndex;
		long Flags;
		unsigned char LightMode;
		char Reserved2[7];
		unsigned long PackedColorPrimary;	// abgr
		unsigned long PackedColorAlternate;	// abgr
		short TexMapIndex;
		short Reserved3;
		unsigned long PrimaryColorIndex;
		unsigned long AlternateColorIndex;
		long Reserved4;

		OF_Mesh();
		~OF_Mesh();
		int WriteToFile(FILE *fOF);

	}; // class OF_Mesh

class OF_MeshPrimitive
	{
	public:
		short Opcode;
		unsigned short Length;
		short PrimitiveType;
		unsigned short IndexSize;
		long VertexCount;

		OF_MeshPrimitive();
		~OF_MeshPrimitive();
		int WriteToFile(FILE *fOF);

	}; // class OF_MeshPrimitive

class OF_LocalVertexPool
	{
	public:
		short Opcode;
		unsigned short Length;
		unsigned long NumVerts;
		unsigned long AttributeMask;

		OF_LocalVertexPool();
		~OF_LocalVertexPool();
		int WriteToFile(FILE *fOF);

	}; // class OF_LocalVertexPool

class OF_PushLevel
	{
	public:
		short Opcode;
		unsigned short Length;

		OF_PushLevel();
		~OF_PushLevel();
		int WriteToFile(FILE *fOF);

	}; // class OF_PushLevel

class OF_PopLevel
	{
	public:
		short Opcode;
		unsigned short Length;

		OF_PopLevel();
		~OF_PopLevel();
		int WriteToFile(FILE *fOF);

	}; // class OF_PopLevel

class OF_Group
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		short RelPri;
		short Reserved1;
		long Flags;
		short SFX_ID1;
		short SFX_ID2;
		short Significance;
		char LayerCode;
		char Reserved2;
		long Reserved3;
		long LoopCount;
		float LoopDuration;
		float LastFrameDuration;

		OF_Group();
		~OF_Group();
		int WriteToFile(FILE *fOF);

	}; // class OF_Group

class OF_ColorPalette
	{
	public:
		short Opcode;
		unsigned short Length;
		char Reserved[128];
		long Color[1024];

		OF_ColorPalette();
		~OF_ColorPalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_ColorPalette

class OF_Continuation
	{
	public:
		short Opcode;
		unsigned short Length;
		long ContinueLength;

		OF_Continuation();
		~OF_Continuation();
		int WriteToFile(FILE *fOF);

	}; // class OF_Continuation

class OF_LOD
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long Reserved;
		double SwitchIn;
		double SwitchOut;
		short SFX_ID1;
		short SFX_ID2;
		long Flags;
		double CenterCoordX;
		double CenterCoordY;
		double CenterCoordZ;
		double TransitionRange;
		double SignificantSize;

		OF_LOD();
		~OF_LOD();
		int WriteToFile(FILE *fOF);

	}; // class OF_LOD

class OF_VertexList
	{
	public:
		short Opcode;
		unsigned short Length;

		OF_VertexList();
		~OF_VertexList();
		int WriteToFile(FILE *fOF);

	}; // class OF_VertexList

class OF_InstanceDefinition
	{
	public:
		short Opcode;
		unsigned short Length;
		short Reserved;
		short InstanceDefNum;

		OF_InstanceDefinition();
		~OF_InstanceDefinition();
		int WriteToFile(FILE *fOF);

	}; // class OF_InstanceDefinition

class OF_InstanceReference
	{
	public:
		short Opcode;
		unsigned short Length;
		short Reserved;
		short InstanceDefNum;

		OF_InstanceReference();
		~OF_InstanceReference();
		int WriteToFile(FILE *fOF);

	}; // class OF_InstanceReference

class OF_BoundingSphere
	{
	public:
		short Opcode;
		unsigned short Length;
		long Reserved;
		double Radius;

		OF_BoundingSphere();
		~OF_BoundingSphere();
		int WriteToFile(FILE *fOF);

	}; // class OF_BoundingSphere

class OF_BoundingVolumeCenter
	{
	public:
		short Opcode;
		unsigned short Length;
		long Reserved;
		double CenterX;
		double CenterY;
		double CenterZ;

		OF_BoundingVolumeCenter();
		~OF_BoundingVolumeCenter();
		int WriteToFile(FILE *fOF);

	}; // class OF_BoundingVolumeCenter

class OF_BoundingVolumeOrientation
	{
	public:
		short Opcode;
		unsigned short Length;
		long Reserved;
		double Yaw;
		double Pitch;
		double Roll;

		OF_BoundingVolumeOrientation();
		~OF_BoundingVolumeOrientation();
		int WriteToFile(FILE *fOF);

	}; // class OF_BoundingVolumeOrientation

class OF_EyepointFields
	{
	public:
		double RotCtrX;
		double RotCtrY;
		double RotCtrZ;
		float Roll;		// Note: this is the apparent order these are really stored in!
		float Pitch;
		float Yaw;
		float RotationMatrix[16];
		float FieldOfView;
		float Scale;
		float NearClipPlane;
		float FarClipPlane;
		float FlythroughMatrix[16];
		float EyepointX;
		float EyepointY;
		float EyepointZ;
		float FlythroughYaw;
		float FlythroughPitch;
		float EyepointDirectVecI;
		float EyepointDirectVecJ;
		float EyepointDirectVecK;
		long NoFlythrough;
		long OrthoView;
		long ValidEyepoint;
		long ImageOffsetX;
		long ImageOffsetY;
		long ImageZoom;
		double Reserved1[4];
		long Reserved2;

		OF_EyepointFields();
		~OF_EyepointFields();
		int WriteToFile(FILE *fOF);

	}; // class OF_EyepointFields

class OF_TrackplaneFields
	{
	public:
		long ValidTrackplane;
		long Reserved1;
		double TrackplaneOriginX;
		double TrackplaneOriginY;
		double TrackplaneOriginZ;
		double TrackplaneAlignmentX;
		double TrackplaneAlignmentY;
		double TrackplaneAlignmentZ;
		double TrackplanePlaneX;
		double TrackplanePlaneY;
		double TrackplnaePlaneZ;
		char GridVisible;
		char GridType;
		char GridUnderFlag;
		char Reserved2;
		float RadialGridAngle;
		double GridSpacingX;
		double GridSpacingY;
		char RadialGridSpacingDirection;
		char RectangularGridSpacingDirection;
		char SnapCursorToGrid;
		char Reserved3;
		long Reserved4;
		double GridSize;	// a power of 2
		long VisibleGridQuadrantsMask;
		long Reserved5;

		OF_TrackplaneFields();
		~OF_TrackplaneFields();
		int WriteToFile(FILE *fOF);

	}; // class OF_TrackplaneFields

class OF_EyepointAndTrackplanePalette
	{
	public:
		short Opcode;
		unsigned short Length;
		long Reserved;
		class OF_EyepointFields Eyepoint[10];
		class OF_TrackplaneFields Trackplane[10];

		OF_EyepointAndTrackplanePalette();
		~OF_EyepointAndTrackplanePalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_EyepointAndTrackplanePalette

class OF_LevelOfDetail
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long Reserved;
		double SwitchInDist;
		double SwitchOutDist;
		short SFX_ID1;
		short SFX_ID2;
		long Flags;
		double CenterCoordX;
		double CenterCoordY;
		double CenterCoordZ;
		double TransitionRange;
		double SignificantSize;

		OF_LevelOfDetail();
		~OF_LevelOfDetail();
		int WriteToFile(FILE *fOF);

	}; // class OF_LevelOfDetail

class OF_LightSourcePalette
	{
	public:
		short Opcode;
		unsigned short Length;
		long PaletteIndex;
		long Reserved0[2];
		char LightSourceName[20];
		long Reserved1;
		float AmbientRGBA[4];
		float DiffuseRGBA[4];
		float SpecularRGBA[4];
		long LightType;
		long Reserved2[10];
		float SpotExpDropoff;
		float SpotCutoffAngle;	// in degrees
		float Yaw;
		float Pitch;
		float ConstantAttenuationCoeff;
		float LinearAttenuationCoeff;
		float QuadraticAttenuationCoeff;
		long ModelingLight;
		long Reserved3[19];

		OF_LightSourcePalette();
		~OF_LightSourcePalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_LightSourcePalette

class OF_LightSource
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long Reserved0;
		long LightPaletteIndex;
		long Reserved1;
		long Flags;
		long Reserved2;
		double PositionXYZ[3];
		float Yaw;
		float Pitch;

		OF_LightSource();
		~OF_LightSource();
		int WriteToFile(FILE *fOF);

	}; // class OF_LightSource

class OF_MaterialPalette
	{
	public:
		short Opcode;
		unsigned short Length;
		long MatIndex;
		char MatName[12];
		long Flags;
		float Ambient[3];	// rgb values between 0.0 & 1.0 inclusive
		float Diffuse[3];	// ""
		float Specular[3];	// ""
		float Emissive[3];	// ""
		float Shininess;	// 0.0 to 128.0
		float Alpha;		// 0.0 to 1.0 where 1.0 is opaque
		long Reserved;

		OF_MaterialPalette();
		~OF_MaterialPalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_MaterialPalette

class OF_Matrix
	{
	public:
		short Opcode;
		unsigned short Length;
		float Matrix[16];

		OF_Matrix();
		~OF_Matrix();
		void Clear(void);
		int WriteToFile(FILE *fOF);

	}; // class OF_Matrix

class OF_Object
	{
	public:
		short Opcode;
		unsigned short Length;
		char ASCII_ID[8];
		long Flags;
		short RelPri;
		unsigned short Trans;
		short SFX_ID1;
		short SFX_ID2;
		short Significance;
		short Reserved;

		OF_Object();
		~OF_Object();
		int WriteToFile(FILE *fOF);

	}; // class OF_Object

class OF_Scale
	{
	public:
		short	Opcode;
		unsigned short Length;
		long	Reserved;
		double	CenterX;
		double	CenterY;
		double	CenterZ;
		float	XScale;
		float	YScale;
		float	ZScale;

		OF_Scale();
		~OF_Scale();
		int WriteToFile(FILE *fOF);

	}; // class OF_Scale

// _VERY_ important note on TAF's!!!
// http://www.multigen-paradigm.com/ubb/Forum8/HTML/000253.html
class OF_TextureAttributeFile
	{
	public:
		long TexelsU;
		long TexelsV;
		long Obsolete1;
		long Obsolete2;
		long UpX;
		long UpY;
		long FileFormatType;
		long MinFilter;
		long MagFilter;
		long WrapUV;
		long WrapU;
		long WrapV;
		long Modified;
		long PivotX;
		long PivotY;
		long EnviroType;
		long AlphaWhite;
		long Reserved1[8];
		double RealWorldSizeU;
		double RealWorldSizeV;
		long OriginCode;
		long KernelVersion;
		long InternalFormat;
		long ExternalFormat;
		long UsingMIPMAPkernel;
		float MIPMAPkernel[8];
		long Send;
		float LOD0;
		float Scale0;
		float LOD1;
		float Scale1;
		float LOD2;
		float Scale2;
		float LOD3;
		float Scale3;
		float LOD4;
		float Scale4;
		float LOD5;
		float Scale5;
		float LOD6;
		float Scale6;
		float LOD7;
		float Scale7;
		float ControlClamp;
		long MagFilterAlpha;
		long MagFilterColor;
		float Reserved2;
		float Reserved3[8];
		double LamberCM;
		double LamberUL;
		double LamberLL;
		double Reserved4;
		float Reserved5[5];
		long UsingTX_DETAIL;
		long J_Arg;
		long K_Arg;
		long M_Arg;
		long N_Arg;
		long Scramble_Arg;
		long Using_TX_TILE;
		float LL_U;
		float LL_V;
		float UR_U;
		float UR_V;
		long Projection;
		long EarthModel;
		long Reserved6;
		long UTMzone;
		long ImageOrigin;
		long GeospecificPointsUnits;
		long Reserved7;
		long Reserved8;
		long Hemisphere;
		long Reserved9;
		long Reserved10;
		long Reserved11[149];
		char Comments[512];
		long Reserved12[13];
		long AttributeFileVersion;
		long NumGeospecificControlPoints;
		long NumSubtextures;

		OF_TextureAttributeFile();
		~OF_TextureAttributeFile();
		int WriteToFile(FILE *fATTR);

	}; // OF_TextureAttributeFile

class OF_TexturePalette
	{
	public:
		short Opcode;
		unsigned short Length;
		char Filename[200];
		long PatternIndex;
		long LocationX;
		long LocationY;

		OF_TexturePalette();
		~OF_TexturePalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_TexturePalette

class OF_Translate
	{
	public:
		short Opcode;
		unsigned short Length;
		long Reserved;
		double FromXYZ[3];
		double DeltaXYZ[3];

		OF_Translate();
		~OF_Translate();
		int WriteToFile(FILE *fOF);

	}; // class OF_Translate

class OF_Vertex_withColor
	{
	public:
		short Opcode;
		unsigned short Length;
		short ColorNameIndex;	// I think docs are wrong, as value of FFFF seems to be common in files (-1 if signed)
		short Flags;
		double VertexCoordX;
		double VertexCoordY;
		double VertexCoordZ;
		long PackedColorABGR;
		unsigned long VertexColorIndex;

		OF_Vertex_withColor();
		~OF_Vertex_withColor();
		int WriteToFile(FILE *fOF);

	}; // class OF_Vertex_withColor

class OF_Vertex_withColorNormal
	{
	public:
		short Opcode;
		unsigned short Length;
		short ColorNameIndex;	// I think docs are wrong, as value of FFFF seems to be common in files (-1 if signed)
		short Flags;
		double VertexCoordX;
		double VertexCoordY;
		double VertexCoordZ;
		float VertexNormalI;
		float VertexNormalJ;
		float VertexNormalK;
		long PackedColorABGR;
		unsigned long VertexColorIndex;

		OF_Vertex_withColorNormal();
		~OF_Vertex_withColorNormal();
		int WriteToFile(FILE *fOF);

	}; // class OF_Vertex_withColorNormal

class OF_Vertex_withColorUV
	{
	public:
		short Opcode;
		unsigned short Length;
		short ColorNameIndex;	// I think docs are wrong, as value of FFFF seems to be common in files (-1 if signed)
		short Flags;
		double VertexCoordX;
		double VertexCoordY;
		double VertexCoordZ;
		float TextureCoordU;
		float TextureCoordV;
		long PackedColorABGR;
		unsigned long VertexColorIndex;

		OF_Vertex_withColorUV();
		~OF_Vertex_withColorUV();
		int WriteToFile(FILE *fOF);

	}; // class OF_Vertex_withColorUV

class OF_Vertex_withColorNormalUV
	{
	public:
		short Opcode;
		unsigned short Length;
		short ColorNameIndex;	// I think docs are wrong, as value of FFFF seems to be common in files (-1 if signed)
		short Flags;
		double VertexCoordX;
		double VertexCoordY;
		double VertexCoordZ;
		float VertexNormalI;
		float VertexNormalJ;
		float VertexNormalK;
		float TextureCoordU;
		float TextureCoordV;
		long PackedColorABGR;
		unsigned long VertexColorIndex;
		long Reserved;

		OF_Vertex_withColorNormalUV();
		~OF_Vertex_withColorNormalUV();
		int WriteToFile(FILE *fOF);

	}; // class OF_Vertex_withColorNormalUV

class OF_VertexPalette
	{
	public:
		short Opcode;
		unsigned short Length;
		long TotalLength;

		OF_VertexPalette();
		~OF_VertexPalette();
		int WriteToFile(FILE *fOF);

	}; // class OF_VertexPalette

