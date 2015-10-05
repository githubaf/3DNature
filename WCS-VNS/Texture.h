// Texture.cpp
// Stuff for the new material and ecosystem ground textures.
// Built from scratch on 9/11/98 by Gary R. Huber.
// Copyright 1998 by Questar productions. All rights reserved.

#ifndef WCS_TEXTURE_H
#define WCS_TEXTURE_H

#include "GraphData.h"
#include "Types.h"
#include "RasterAnimHost.h"
#include "GeoRegister.h"
#include "FeatureConfig.h"

class Raster;
class RasterShell;
class ImageLib;
class fBmNoise;
class fBmTurbulentNoise;
class MultiFractal;
class HybridMultiFractal;
class RidgedMultiFractal;
class HeteroTerrain;
class Texture;
class F1CellBasis3;
class F2CellBasis3;
class F2mF1CellBasis3;
class F3mF1CellBasis3;
class F1Manhattan3;
class F2Manhattan3;
class F2mF1Manhattan3;
class NotifyEx;
class Joe;
class PRNGX;
class RasterAnimHost;
class TextureData;
class CoordSys;
class IncludeExcludeList;
class IncludeExcludeTypeList;
class VertexDEM;
class VertexData;
class PolygonData;
class VertexReferenceData;
class MaterialTable;

#undef RGB	// stupid Microsoft makes macros with hazardously-common names

// notification subclasses
enum
	{
	WCS_SUBCLASS_ROOTTEXTURE = 180,
	WCS_SUBCLASS_TEXTURE
	}; // subclasses contained in effects that are not effects per se

enum
	{
	// bitmaps
	WCS_TEXTURE_TYPE_PLANARIMAGE,
	WCS_TEXTURE_TYPE_CYLINDRICALIMAGE,
	WCS_TEXTURE_TYPE_SPHERICALIMAGE,
//	WCS_TEXTURE_TYPE_CUBICIMAGE,
	WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE,
	WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE,
	// vertex maps
	WCS_TEXTURE_TYPE_UVW,
	WCS_TEXTURE_TYPE_COLORPERVERTEX,
	// patterns
	WCS_TEXTURE_TYPE_STRIPES,
	WCS_TEXTURE_TYPE_SOFTSTRIPES,
	WCS_TEXTURE_TYPE_SINGLESTRIPE,
	WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE,
	WCS_TEXTURE_TYPE_WOODGRAIN,
	WCS_TEXTURE_TYPE_MARBLE,
	WCS_TEXTURE_TYPE_BRICK,
	WCS_TEXTURE_TYPE_DOTS,
//	WCS_TEXTURE_TYPE_VEINS,
//	WCS_TEXTURE_TYPE_CRUST,
//	WCS_TEXTURE_TYPE_UNDERWATER,
	// fractal
	WCS_TEXTURE_TYPE_FRACTALNOISE,
	WCS_TEXTURE_TYPE_MULTIFRACTALNOISE,
	WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE,
//	WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE,
//	WCS_TEXTURE_TYPE_HETEROTERRAINNOISE,
	WCS_TEXTURE_TYPE_TURBULENCE,
	// cell basis
	WCS_TEXTURE_TYPE_F1CELLBASIS,
	WCS_TEXTURE_TYPE_F2CELLBASIS,
	WCS_TEXTURE_TYPE_F2MF1CELLBASIS,
//	WCS_TEXTURE_TYPE_F3MF1CELLBASIS,
	WCS_TEXTURE_TYPE_F1MANHATTAN,
	WCS_TEXTURE_TYPE_F2MANHATTAN,
	WCS_TEXTURE_TYPE_F2MF1MANHATTAN,
	WCS_TEXTURE_TYPE_BIRDSHOT,
	// mathematical
	WCS_TEXTURE_TYPE_ADD,
	WCS_TEXTURE_TYPE_SUBTRACT,
	WCS_TEXTURE_TYPE_MULTIPLY,
	WCS_TEXTURE_TYPE_COMPOSITE,
	WCS_TEXTURE_TYPE_CONTRAST,
	WCS_TEXTURE_TYPE_DARKEN,
	WCS_TEXTURE_TYPE_LIGHTEN,
	WCS_TEXTURE_TYPE_LEVELS,
	WCS_TEXTURE_TYPE_HSVMERGE,
	WCS_TEXTURE_TYPE_SKEW,
	WCS_TEXTURE_TYPE_BELLCURVE,
	WCS_TEXTURE_TYPE_SQUAREWAVE,
	WCS_TEXTURE_TYPE_SAWTOOTH,
	WCS_TEXTURE_TYPE_STEP,
	WCS_TEXTURE_TYPE_SLOPE,
	WCS_TEXTURE_TYPE_GAMMA,
	WCS_TEXTURE_TYPE_BIAS,
	WCS_TEXTURE_TYPE_GAIN,
	WCS_TEXTURE_TYPE_CUSTOMCURVE,
	WCS_TEXTURE_TYPE_MAXIMUM,
	WCS_TEXTURE_TYPE_MAXIMUMSWITCH,
	WCS_TEXTURE_TYPE_MINIMUM,
	WCS_TEXTURE_TYPE_MINIMUMSWITCH,
	WCS_TEXTURE_TYPE_THRESHOLD,
	// parameters
	WCS_TEXTURE_TYPE_TERRAINPARAM,
	// no inputs
	WCS_TEXTURE_TYPE_GRADIENT,
	WCS_TEXTURE_TYPE_INCLUDEEXCLUDE,
	WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE,
// <<<>>> ADD_NEW_TEXTURE
	WCS_TEXTURE_NUMTYPES
	}; // texture types

// temporarily disable these textures
#define WCS_TEXTURE_TYPE_CUBICIMAGE					100
#define WCS_TEXTURE_TYPE_VEINS						101
#define WCS_TEXTURE_TYPE_CRUST						102
#define WCS_TEXTURE_TYPE_UNDERWATER					103
#define WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE	104
#define WCS_TEXTURE_TYPE_HETEROTERRAINNOISE			105
#define WCS_TEXTURE_TYPE_F3MF1CELLBASIS				106

enum
	{
	WCS_TEXTURE_SHOW_SIZE,
	WCS_TEXTURE_SHOW_CENTER,
	WCS_TEXTURE_SHOW_FALLOFF,
	WCS_TEXTURE_SHOW_VELOCITY,
	WCS_TEXTURE_SHOW_ROTATION
	}; // options for radio buttons

enum
	{
	WCS_TEXTURE_PREVIEW_PLUSX,
	WCS_TEXTURE_PREVIEW_PLUSY,
	WCS_TEXTURE_PREVIEW_PLUSZ,
	WCS_TEXTURE_PREVIEW_MINUSX,
	WCS_TEXTURE_PREVIEW_MINUSY,
	WCS_TEXTURE_PREVIEW_MINUSZ,
	WCS_TEXTURE_PREVIEW_CUBE,
	WCS_TEXTURE_PREVIEW_SPHERE
	}; // options for radio buttons

enum
	{
	WCS_TEXTURE_TYPECLASS_IMAGE = 1,
	WCS_TEXTURE_TYPECLASS_PROCEDURAL,
	WCS_TEXTURE_TYPECLASS_PARAMETER,
	WCS_TEXTURE_TYPECLASS_OBJECT,
	WCS_TEXTURE_TYPECLASS_VERTEX
	}; // basic texture type classes

enum
	{
	WCS_TEXTURE_TERRAINPARAM_ELEV,
	WCS_TEXTURE_TERRAINPARAM_RELELEV,
	WCS_TEXTURE_TERRAINPARAM_SLOPE,
	WCS_TEXTURE_TERRAINPARAM_NORTHDEV,
	WCS_TEXTURE_TERRAINPARAM_EASTDEV,
	WCS_TEXTURE_TERRAINPARAM_LATITUDE,
	WCS_TEXTURE_TERRAINPARAM_LONGITUDE,
	WCS_TEXTURE_TERRAINPARAM_ILLUMINATION,
	WCS_TEXTURE_TERRAINPARAM_ZDISTANCE,
	WCS_TEXTURE_TERRAINPARAM_WATERDEPTH,
	WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE,
	WCS_TEXTURE_TERRAINPARAM_RED,
	WCS_TEXTURE_TERRAINPARAM_GREEN,
	WCS_TEXTURE_TERRAINPARAM_BLUE,
	WCS_TEXTURE_TERRAINPARAM_HUE,
	WCS_TEXTURE_TERRAINPARAM_SATURATION,
	WCS_TEXTURE_TERRAINPARAM_VALUE,
	WCS_TEXTURE_TERRAINPARAM_LUMINOSITY,
	WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY,
	WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA,
	WCS_TEXTURE_TERRAINPARAM_ASPECT,
	WCS_TEXTURE_TERRAINPARAM_MAXPARAMS
	}; // terrain parameters

enum
	{
	WCS_TEXTURE_PARAMTYPE_OPACITY = 1,	// 0 is reserved for unused parameters
	WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES,
	WCS_TEXTURE_PARAMTYPE_TURBULENCE,
	WCS_TEXTURE_PARAMTYPE_SPACING,
	WCS_TEXTURE_PARAMTYPE_SHARPNESS,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_OCTAVES,
	WCS_TEXTURE_PARAMTYPE_COVERAGE,
	WCS_TEXTURE_PARAMTYPE_LEDGELEVEL,
	WCS_TEXTURE_PARAMTYPE_LEDGEWIDTH,
	WCS_TEXTURE_PARAMTYPE_BUMPSTRENGTH,
	WCS_TEXTURE_PARAMTYPE_SOURCES,
	WCS_TEXTURE_PARAMTYPE_SPEED,
	WCS_TEXTURE_PARAMTYPE_LENGTH,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC,
	WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC,
	WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS,
	WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS,
	WCS_TEXTURE_PARAMTYPE_CONTRAST,
	WCS_TEXTURE_PARAMTYPE_CENTER,
	WCS_TEXTURE_PARAMTYPE_WIDTH,
	WCS_TEXTURE_PARAMTYPE_MASK,
	WCS_TEXTURE_PARAMTYPE_MEDIAN,
	WCS_TEXTURE_PARAMTYPE_SWITCH,
	WCS_TEXTURE_PARAMTYPE_THRESHOLD,
	WCS_TEXTURE_PARAMTYPE_SAMPLE,
	WCS_TEXTURE_PARAMTYPE_REPEATFLAG,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE,
	WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE,
	WCS_TEXTURE_PARAMTYPE_SKEW,
	WCS_TEXTURE_PARAMTYPE_OFFSET,
	WCS_TEXTURE_PARAMTYPE_GAIN,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES,
	WCS_TEXTURE_PARAMTYPE_SIZE,
	WCS_TEXTURE_PARAMTYPE_TERRAINPAR,
	WCS_TEXTURE_PARAMTYPE_SLOPE,
	WCS_TEXTURE_PARAMTYPE_GAMMA,
	WCS_TEXTURE_PARAMTYPE_BIAS,
	WCS_TEXTURE_PARAMTYPE_HEIGHT,
	WCS_TEXTURE_PARAMTYPE_FALLOFF,
	WCS_TEXTURE_PARAMTYPE_ROTATION
	}; // texture parameter type classes

/*
// not currently used
enum
	{
	WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_OBJECT_SCALED,
	WCS_TEXTURE_COORDSPACE_OBJECT_ROTATED,
	WCS_TEXTURE_COORDSPACE_OBJECT_TRANSLATED,
	WCS_TEXTURE_COORDSPACE_OBJECT_SCALEDROTATED,
	WCS_TEXTURE_COORDSPACE_OBJECT_SCALEDTRANSLATED,
	WCS_TEXTURE_COORDSPACE_OBJECT_ROTATEDTRANSLATED,
	WCS_TEXTURE_COORDSPACE_OBJECT_SCALEDROTATEDTRANSLATED,
	WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_GLOBAL_GEOGRAPHIC,
	WCS_TEXTURE_COORDSPACE_GLOBAL_GEOGRXYZ_PROJECT,
	WCS_TEXTURE_COORDSPACE_GLOBAL_GEOGRXYZ_OBJECT,
	WCS_TEXTURE_COORDSPACE_VECTOR_GEOGRXYZ_OBJECT,
	WCS_TEXTURE_COORDSPACE_VECTOR_GEOGRXYZ_POINT,
	WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS,
	WCS_TEXTURE_COORDSPACE_IMAGE_UNITWIDTH_UNITHEIGHT,
	WCS_TEXTURE_COORDSPACE_IMAGE_UNITWIDTH_PROPHEIGHT,
	WCS_TEXTURE_COORDSPACE_IMAGE_UNITHEIGHT_PROPWIDTH,
	WCS_TEXTURE_COORDSPACE_CAMERA_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_CAMERA_PERSPECTIVE,
	WCS_TEXTURE_COORDSPACE_CAMERA_POLAR,
	WCS_TEXTURE_COORDSPACE_CAMERA_VISIBILITY,
	WCS_TEXTURE_COORDSPACE_LIGHT_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_LIGHT_PERSPECTIVE,
	WCS_TEXTURE_COORDSPACE_LIGHT_POLAR,
	WCS_TEXTURE_COORDSPACE_LIGHT_ILLUMINATION,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_GLOBAL_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_CAMERA_TRANSFORM_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_CAMERA_ANGULARDEVIATION,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_LIGHT_TRANSFORM_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_LIGHT_ANGULARDEVIATION,
	WCS_TEXTURE_COORDSPACE_SFCNORMAL_CAMERALIGHT_SPECULARDEVIATION,
	WCS_TEXTURE_COORDSPACE_TERRAINPARAM,
	WCS_TEXTURE_MAX_COORDSPACES
	}; // texture coordinate spaces
*/
enum
	{
	WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN,
	WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ,
	WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED,
	WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED,
	WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE,
	WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ,
	WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS,
	WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ,
	WCS_TEXTURE_COORDSPACE_VERTEX_UVW,
	WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX,
	WCS_TEXTURE_COORDSPACE_NONE,
	WCS_TEXTURE_MAX_COORDSPACES
	}; // texture coordinate spaces

#define WCS_TEXTURE_MAXPARAMS	11

#define	WCS_TEXTURE_MAXPARAMTEXTURES	(WCS_TEXTURE_MAXPARAMS + 16)

#define WCS_TEXTURE_COLOR1				WCS_TEXTURE_MAXPARAMS
#define WCS_TEXTURE_COLOR2				(WCS_TEXTURE_MAXPARAMS + 1)
#define WCS_TEXTURE_STRATAFUNC			(WCS_TEXTURE_MAXPARAMS + 2)
#define WCS_TEXTURE_BLENDINGFUNC		(WCS_TEXTURE_MAXPARAMS + 3)
#define WCS_TEXTURE_ROOT			-1
#define WCS_TEXTURE_OPACITY			0
#define WCS_TEXTURE_LOW				1
#define WCS_TEXTURE_HIGH			2
#define WCS_TEXTURE_3DIMENSIONS		50
#define WCS_TEXTURE_SIZE1			(WCS_TEXTURE_MAXPARAMS + 4)
#define WCS_TEXTURE_SIZE2			(WCS_TEXTURE_SIZE1 + 1)
#define WCS_TEXTURE_SIZE3			(WCS_TEXTURE_SIZE1 + 2)
#define WCS_TEXTURE_CENTER1			(WCS_TEXTURE_MAXPARAMS + 7)
#define WCS_TEXTURE_CENTER2			(WCS_TEXTURE_CENTER1 + 1)
#define WCS_TEXTURE_CENTER3			(WCS_TEXTURE_CENTER1 + 2)
#define WCS_TEXTURE_FALLOFF1		(WCS_TEXTURE_MAXPARAMS + 10)
#define WCS_TEXTURE_FALLOFF2		(WCS_TEXTURE_FALLOFF1 + 1)
#define WCS_TEXTURE_FALLOFF3		(WCS_TEXTURE_FALLOFF1 + 2)
#define WCS_TEXTURE_ROTATION1		(WCS_TEXTURE_MAXPARAMS + 13)
#define WCS_TEXTURE_ROTATION2		(WCS_TEXTURE_ROTATION1 + 1)
#define WCS_TEXTURE_ROTATION3		(WCS_TEXTURE_ROTATION1 + 2)
#define WCS_TEXTURE_SIZE(x)			((x) >= WCS_TEXTURE_SIZE1 && (x) <= WCS_TEXTURE_SIZE3)
#define WCS_TEXTURE_CENTER(x)		((x) >= WCS_TEXTURE_CENTER1 && (x) <= WCS_TEXTURE_CENTER3)
#define WCS_TEXTURE_FALLOFF(x)		((x) >= WCS_TEXTURE_FALLOFF1 && (x) <= WCS_TEXTURE_FALLOFF3)
#define WCS_TEXTURE_ROTATION(x)		((x) >= WCS_TEXTURE_ROTATION1 && (x) <= WCS_TEXTURE_ROTATION3)
#define WCS_TEXTURE_INPUTPARAM(x)	((x) >= WCS_TEXTURE_SIZE1 && (x) <= WCS_TEXTURE_ROTATION3)

#define WCS_TEXTURE_BOUNDSNORTH		0
#define WCS_TEXTURE_BOUNDSSOUTH		1
#define WCS_TEXTURE_BOUNDSWEST		2
#define WCS_TEXTURE_BOUNDSEAST		3
#define WCS_TEXTURE_FEATHER		4

#define WCS_TEXTURE_POSSIZEX	0
#define WCS_TEXTURE_POSSIZEY	1
#define WCS_TEXTURE_POSSIZEZ	2

#define WCS_TEXTURE_ANIMTYPE_PARAM		0
#define WCS_TEXTURE_ANIMTYPE_MISC		1
#define WCS_TEXTURE_ANIMTYPE_POSSIZE	2
#define WCS_TEXTURE_ANIMTYPE_SIZE		(WCS_TEXTURE_ANIMTYPE_POSSIZE + WCS_TEXTURE_SHOW_SIZE)
#define WCS_TEXTURE_ANIMTYPE_CENTER		(WCS_TEXTURE_ANIMTYPE_POSSIZE + WCS_TEXTURE_SHOW_CENTER)
#define WCS_TEXTURE_ANIMTYPE_FALLOFF	(WCS_TEXTURE_ANIMTYPE_POSSIZE + WCS_TEXTURE_SHOW_FALLOFF)
#define WCS_TEXTURE_ANIMTYPE_VELOCITY	(WCS_TEXTURE_ANIMTYPE_POSSIZE + WCS_TEXTURE_SHOW_VELOCITY)
#define WCS_TEXTURE_ANIMTYPE_ROTATION	(WCS_TEXTURE_ANIMTYPE_POSSIZE + WCS_TEXTURE_SHOW_ROTATION)

#define WCS_TEXTURETYPE_ISBITMAP(x) ((x) <= WCS_TEXTURE_TYPE_UVW)
#define WCS_TEXTURETYPE_ISMATHEMATICAL(x) ((x) >= WCS_TEXTURE_TYPE_ADD && (x) <= WCS_TEXTURE_TYPE_TERRAINPARAM)
#define WCS_TEXTURETYPE_IS3DOPTIONAL(x) (((x) >= WCS_TEXTURE_TYPE_F1CELLBASIS && (x) <= WCS_TEXTURE_TYPE_BIRDSHOT) ||\
 ((x) == WCS_TEXTURE_TYPE_BRICK) || ((x) == WCS_TEXTURE_TYPE_DOTS))
#define WCS_TEXTURETYPE_ISPARAMETER(x) ((x) == WCS_TEXTURE_TYPE_TERRAINPARAM)
#define WCS_TEXTURETYPE_USESIZE(x) (((x) != WCS_TEXTURE_TYPE_SPHERICALIMAGE) || ((x) != WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE) || ((x) != WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE))
#define WCS_TEXTURETYPE_USEVELOCITY(x) (((x) != WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE) || ((x) != WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE))
#define WCS_TEXTURETYPE_USEROTATION(x) (((x) != WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE))
#define WCS_TEXTURETYPE_ECOILLEGAL(x) (((x) == WCS_TEXTURE_TYPE_CYLINDRICALIMAGE) || ((x) == WCS_TEXTURE_TYPE_SPHERICALIMAGE) ||\
 ((x) == WCS_TEXTURE_TYPE_UVW) || ((x) == WCS_TEXTURE_TYPE_COLORPERVERTEX))
#define WCS_TEXTURETYPE_OBJECT3DILLEGAL(x) (0)
#define WCS_TEXTURETYPE_OBJECT3DILLEGAL2(x) (0)
#define WCS_TEXTURETYPE_ALIGNTOVECTORILLEGAL(x) ((x) == WCS_TEXTURE_TYPE_CYLINDRICALIMAGE || (x) == WCS_TEXTURE_TYPE_SPHERICALIMAGE ||\
 (x) == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || (x) == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE || WCS_TEXTURETYPE_ISMATHEMATICAL(x) ||\
 (x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDE || (x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE ||\
 (x) == WCS_TEXTURE_TYPE_UVW || (x) == WCS_TEXTURE_TYPE_COLORPERVERTEX)
#define WCS_TEXTURETYPE_LATLONBNDSILLEGAL(x) ((x) != WCS_TEXTURE_TYPE_PLANARIMAGE)

#define WCS_TEXTURETYPE_ISCURVE(x) (((x) >= WCS_TEXTURE_TYPE_BELLCURVE) && ((x) <= WCS_TEXTURE_TYPE_CUSTOMCURVE))
#define WCS_TEXTURETYPE_USEAXIS(x) (((x) == WCS_TEXTURE_TYPE_PLANARIMAGE) || ((x) == WCS_TEXTURE_TYPE_CYLINDRICALIMAGE) ||\
 ((x) == WCS_TEXTURE_TYPE_SPHERICALIMAGE) || ((x) == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE) ||\
 ((x) == WCS_TEXTURE_TYPE_WOODGRAIN) || ((x) == WCS_TEXTURE_TYPE_MARBLE) ||\
 ((x) == WCS_TEXTURE_TYPE_STRIPES) || ((x) == WCS_TEXTURE_TYPE_SOFTSTRIPES) || ((x) == WCS_TEXTURE_TYPE_SINGLESTRIPE) ||\
 ((x) == WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE) || ((x) == WCS_TEXTURE_TYPE_GRADIENT) || WCS_TEXTURETYPE_IS3DOPTIONAL(x))
#define WCS_TEXTURETYPE_BLENDILLEGAL(x) (((x) != WCS_TEXTURE_TYPE_CONTRAST)\
 && ((x) != WCS_TEXTURE_TYPE_DARKEN)\
 && ((x) != WCS_TEXTURE_TYPE_LIGHTEN)\
 && ((x) != WCS_TEXTURE_TYPE_LEVELS)\
 && ((x) != WCS_TEXTURE_TYPE_SKEW)\
 && ((x) != WCS_TEXTURE_TYPE_BELLCURVE)\
 && ((x) != WCS_TEXTURE_TYPE_SQUAREWAVE)\
 && ((x) != WCS_TEXTURE_TYPE_SAWTOOTH)\
 && ((x) != WCS_TEXTURE_TYPE_STEP)\
 && ((x) != WCS_TEXTURE_TYPE_SLOPE)\
 && ((x) != WCS_TEXTURE_TYPE_GAMMA)\
 && ((x) != WCS_TEXTURE_TYPE_BIAS)\
 && ((x) != WCS_TEXTURE_TYPE_GAIN)\
 && ((x) != WCS_TEXTURE_TYPE_CUSTOMCURVE)\
 )
#define WCS_TEXTURETYPE_ISTWOVALUE(x) (((x) == WCS_TEXTURE_TYPE_STRIPES)\
 || ((x) == WCS_TEXTURE_TYPE_SOFTSTRIPES)\
 || ((x) == WCS_TEXTURE_TYPE_SINGLESTRIPE)\
 || ((x) == WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE)\
 || ((x) == WCS_TEXTURE_TYPE_WOODGRAIN)\
 || ((x) == WCS_TEXTURE_TYPE_MARBLE)\
 || ((x) == WCS_TEXTURE_TYPE_BRICK)\
 || ((x) == WCS_TEXTURE_TYPE_DOTS)\
 || ((x) == WCS_TEXTURE_TYPE_FRACTALNOISE)\
 || ((x) == WCS_TEXTURE_TYPE_MULTIFRACTALNOISE)\
 || ((x) == WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE)\
 || ((x) == WCS_TEXTURE_TYPE_TURBULENCE)\
 || ((x) == WCS_TEXTURE_TYPE_F1CELLBASIS)\
 || ((x) == WCS_TEXTURE_TYPE_F2CELLBASIS)\
 || ((x) == WCS_TEXTURE_TYPE_F2MF1CELLBASIS)\
 || ((x) == WCS_TEXTURE_TYPE_F1MANHATTAN)\
 || ((x) == WCS_TEXTURE_TYPE_F2MANHATTAN)\
 || ((x) == WCS_TEXTURE_TYPE_F2MF1MANHATTAN)\
 || ((x) == WCS_TEXTURE_TYPE_BIRDSHOT)\
 || ((x) == WCS_TEXTURE_TYPE_ADD)\
 || ((x) == WCS_TEXTURE_TYPE_SUBTRACT)\
 || ((x) == WCS_TEXTURE_TYPE_MULTIPLY)\
 || ((x) == WCS_TEXTURE_TYPE_COMPOSITE)\
 || ((x) == WCS_TEXTURE_TYPE_MAXIMUM)\
 || ((x) == WCS_TEXTURE_TYPE_MAXIMUMSWITCH)\
 || ((x) == WCS_TEXTURE_TYPE_MINIMUM)\
 || ((x) == WCS_TEXTURE_TYPE_MINIMUMSWITCH)\
 || ((x) == WCS_TEXTURE_TYPE_THRESHOLD)\
 || ((x) == WCS_TEXTURE_TYPE_TERRAINPARAM)\
 || ((x) == WCS_TEXTURE_TYPE_GRADIENT)\
 || ((x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDE)\
 || ((x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE)\
 )

#define WCS_TEXTURETYPE_PARENTPREVIEW(x)	((x) == WCS_TEXTURE_TYPE_LEVELS || WCS_TEXTURETYPE_ISCURVE(x))

// removed from ISTWOVALUE above until needed
// || ((x) == WCS_TEXTURE_TYPE_VEINS)
// || ((x) == WCS_TEXTURE_TYPE_CRUST)
// || ((x) == WCS_TEXTURE_TYPE_UNDERWATER)
// || ((x) == WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE)
// || ((x) == WCS_TEXTURE_TYPE_HETEROTERRAINNOISE)
// || ((x) == WCS_TEXTURE_TYPE_F3MF1CELLBASIS)
#define WCS_TEXTURETYPE_USEANTIALIAS(x) (! WCS_TEXTURETYPE_ISMATHEMATICAL(x))
#define WCS_TEXTURETYPE_USESUMTABLEANTIALIAS(x) (! WCS_TEXTURETYPE_ISBITMAP(x) && ! WCS_TEXTURETYPE_ISMATHEMATICAL(x))
#define WCS_TEXTURETYPE_QUANTIZEENABLED(x) ((((x) >= WCS_TEXTURE_TYPE_F1CELLBASIS) && ((x) <= WCS_TEXTURE_TYPE_BIRDSHOT)) ||\
 ((x) == WCS_TEXTURE_TYPE_BRICK) || ((x) == WCS_TEXTURE_TYPE_DOTS))
// <<<>>> ADD_NEW_TEXTURE

#define WCS_TEXTUREPARAM_ISCOLOR(x)	(((x) == WCS_TEXTURE_COLOR1) || ((x) == WCS_TEXTURE_COLOR2) ||\
 ((x) == WCS_TEXTURE_ROOT))

#define WCS_TEXTUREPARAM_USECOORDSPACE(x)	((! WCS_TEXTURETYPE_ISMATHEMATICAL(x)) &&\
 ((x) != WCS_TEXTURE_TYPE_INCLUDEEXCLUDE) && ((x) != WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE) &&\
 ((x) != WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE) && ((x) != WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE))

#define WCS_TEXTUREPARAM_NOSIZECENTERETC(x)	((WCS_TEXTURETYPE_ISMATHEMATICAL(x)) ||\
 ((x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDE) || ((x) == WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE) ||\
 ((x) == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE) || ((x) == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE) ||\
 ((x) == WCS_TEXTURE_TYPE_UVW) || ((x) == WCS_TEXTURE_TYPE_COLORPERVERTEX))

#define WCS_TEXTURETYPE_TILEABLE(x)	(((x) == WCS_TEXTURE_TYPE_PLANARIMAGE) ||\
 ((x) == WCS_TEXTURE_TYPE_STRIPES) ||\
 ((x) == WCS_TEXTURE_TYPE_SOFTSTRIPES) ||\
 ((x) == WCS_TEXTURE_TYPE_BRICK) ||\
 ((x) == WCS_TEXTURE_TYPE_DOTS) ||\
 (((x) >= WCS_TEXTURE_TYPE_ADD) && ((x) <= WCS_TEXTURE_TYPE_THRESHOLD)))

// Param Flags
#define WCS_TEXTURE_PARAMAVAIL		(1 << 0)
#define WCS_TEXTURE_TEXTUREAVAIL	(1 << 1)
#define WCS_TEXTURE_PARAMPERCENT	(1 << 2)
#define WCS_TEXTURE_PARAMBOOLEAN	(1 << 3)

#define WCS_UVMAP_MAXNAMELENGTH	60

#define WCS_SinglePoint2D(a, b, c, d)	(((a) == (b)) && ((c) == (d)))
#define WCS_SinglePoint3D(a, b, c, d, e, f)	(((a) == (b)) && ((c) == (d)) && ((e) == (f)))

// Notes:
// Textures can be applied to either 3D Object Material Effects or to Ecosystem ground polygons
// Textures can be applied to the color channel of a material or to any of the other material properties
//  including specularity, specular exponent, luminosity, transparency, translucency,
//  translucency exponent or bump.
// For an ecosystem, textures can be applied to the color, bump, reflectivity (specularity),
//  smoothness (specular exponent), transparency (optical depth in the case of water), luminosity 
//  or foliage (foliage will be applied according to the texture values).
// Depending on whether the texture is appied to an ecosystem or to a material, certain values
//  in the Texture object may not be available to the user. For instance the LatLonBounds only makes
//  sense for textures applied to the terrain (as ecosystems).
// In addition certain values are only applicable to those textures applied as colors to the material
//  or ecosystem. The obvious case is the Palette numbers of colors.
// Still other special availability cases are determined by whether the user selects a raster image
//  or a procedural for the type of a texture.
// The texture root stores a pointer to a linked list of textures. The list can be as long as desired
//  and will be evaluated from the head of the list downward until either the end of the list or until
//  the opacity exceeds 100%.
// Like LightWave, the opacity, reduced by any alpha channel or self-alpha, is applied to the remainder
//  of the unused opacity when a texture is evaluated in the chain. We'll see how this works and change
//  it later if necessary to achieve certain effects.
// The Opacity channel, if specified, is just the head of another texture chain. The only difference is
//  that it obviously is not a color application and so color palettes are not available.

class RootTextureParent
	{
	public:
		void Copy(RootTextureParent *CopyTo, RootTextureParent *CopyFrom);
		virtual RootTexture *NewRootTexture(long TexNum) = 0;
		virtual char *GetTextureName(long TexNum) = 0;
		virtual long GetNumTextures(void) = 0;
		virtual RootTexture *GetTexRootPtr(long TexNum) = 0;
		virtual RootTexture *GetEnabledTexture(long TexNum) = 0;
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) = 0;
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) = 0;
		long GetTexNumberFromAddr(RootTexture **Addr);
		void DeleteAllTextures(void);
		int AreThereEnabledTextures(void);

	}; // class RootTextureParent

// these are used by texture Analyze methods to determine how to process coordinates
#define WCS_SAMPLETEXTURE_LIVEDATA		0
#define WCS_SAMPLETEXTURE_PROJECTED		1
#define WCS_SAMPLETEXTURE_FLAT			2

// in Texture.cpp
class TextureData
	{
	public:
		unsigned char VectorEffectType, VectorOffsetsComputed, VDataVecOffsetsValid, SampleType, IsSampleThumbnail, 
			ObjectType, InputSuppliedInOutput, ExagerateElevLines;
		long VtxNum[3];
		float RelElev, VectorSlope;
		double TLowX, THighX, TLowY, THighY, TLowZ, THighZ, ThumbnailX, ThumbnailY;
		double Elev, Slope, Aspect, Latitude, Longitude, Illumination, ZDist, QDist, WaterDepth, 
			WaveHeight, Reflectivity, TLatRange[4], TLonRange[4], TElevRange[4], Normal[3], CamAimVec[3],
			PixelX[2], PixelY[2], PixelUnityX[2], PixelUnityY[2], VecOffset[3], VecOffsetPixel[6], MetersPerDegLat, MetersPerDegLon, Datum, Exageration,
			RGB[3], PixelWidth, ObjectPixelWidth, VtxPixCornerWt[4][3], VtxWt[3], TexRefLat, TexRefLon, TexRefElev;
		Joe *Vec;
		VertexDEM *VDEM[3];
		VertexData *VData[3];
		PolygonData *PData;
		RasterAnimHost *Object;
		Object3DEffect *Object3D;
		VertexReferenceData *VertRefData[3];

		TextureData();
		void ClearVectorInfo(void) {VectorEffectType = VectorOffsetsComputed = VDataVecOffsetsValid = 0; Vec = 0;};
		// in RenderData.cpp
		void CopyData(TextureData *CopyFrom);
		void ExtractMatTableData(MaterialTable *MatTable, long *VertIndex);

	}; // class TextureData

class TextureSampleData
	{
	public:
		unsigned char PreviewDirection, Running, AndChildren;
		long y, zip, MaxY;
		float *PreviewDataPtr, *XData, *YData, *ZData, *IllumData;
		double PreviewSize, SampleInc, SampleStart, 
			StartX, StartY, StartZ, XInc, YInc, ZInc, XVInc, XHInc, YVInc, YHInc, ZVInc, ZHInc;
		Thumbnail *Thumb;
		TextureData TexData;

		TextureSampleData()	{Running = 0;};

	}; // class TextureSampleData

class TextureExtractionData
	{
	public:
		double LatRange[4], LonRange[4], ElevRange[4], LowX, HighX, LowY, HighY, LowZ, HighZ;
		
		TextureExtractionData(TextureData *Data);
	}; // class TextureExtractionData
	
#define WCS_TEXTURE_VECTOREFFECTTYPE_AREA			0
#define WCS_TEXTURE_VECTOREFFECTTYPE_LINE			(1 << 0)
#define WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV		(1 << 1)
#define WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON	(1 << 2)
#define WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS	(1 << 3)
#define WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT	(1 << 4)

class RootTexture : public RasterAnimHost
	{
	public:
		char Name[60];
		unsigned char Enabled, ApplyToEcosys, ApplyToColor, ApplyToDisplace;
		unsigned char ShowSize, PreviewDirection, ShowRootNail, ShowCompChildNail, ShowComponentNail;
		AnimDoubleTime PreviewSize;
		Texture *Tex;
		static char *TextureNames[WCS_TEXTURE_NUMTYPES];
		static bool TextureRemapType[WCS_TEXTURE_NUMTYPES];
		static char *CoordSpaceNames[WCS_TEXTURE_MAX_COORDSPACES];
		static char *UserNames[WCS_TEXTURE_NUMTYPES];
		static float *CubePreviewData, *SpherePreviewData;
		static PRNGX TextureRand;

		RootTexture();
		RootTexture(RasterAnimHost *RAHost);
		RootTexture(RasterAnimHost *RAHost, unsigned char EcosysSource, unsigned char ColorSource, unsigned char DisplaceSource);
		~RootTexture();
		static void DestroyPreviewSubjects(void);
		void SetDefaults(void);
		void Copy(RootTexture *CopyTo, RootTexture *CopyFrom);
		Texture *MatchRasterShell(RasterShell *Shell);
		int IsAnimated(void);
		unsigned char GetEnabled(void)	{return(Enabled);};
		int AdjustTextureOrder(Texture *MoveMe, int Direction);
		Texture *AddNewTexture(AnimColorTime *DefaultColor);
		Texture *AddNewTexture(Texture *Proto, unsigned char TexType);
		static Texture *NewTexture(RasterAnimHost *RAHost, Texture *Proto, unsigned char NewTexType, long ParamNumber);
		Texture *RemoveTexture(Texture *RemoveMe);
		unsigned char GetViewDirection(void) {return (PreviewDirection);};
		int EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneSampleLine(TextureSampleData *Samp);
		double Eval(double Value[3], TextureData *Data);
		static char *GetTextureName(char TexType) {return (TextureNames[TexType]);};
		static char *GetUserName(char TexType) {return (UserNames[TexType]);};
		static char IdentifyTextureType(char *TypeStr);
		void BuildTextureRotationMatrices(void);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SaveObject(FILE *ffile);
		int ConfigureForEcosystem(void);
		int ConfigureForObject3D(void);
		int VertexUVWAvailable(void);
		int VertexCPVAvailable(void);
		Texture *GetValidTexture(Texture *TestActive);
		long InitImageIDs(long &ImageID);
		int InitAAChain(void);
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		long SetColorList(short *ColorList, long &ListPos);
		void PropagateCoordSpace(unsigned char SpaceNumber);
		static float *CubePreviewLoaded(void) {return(CubePreviewData);};
		static float *SpherePreviewLoaded(void) {return(SpherePreviewData);};
		static float *LoadPreviewFile(char PreviewType);
		static char *GetCoordSpaceName(unsigned char SpaceNumber)	{return (CoordSpaceNames[SpaceNumber]);};
		static unsigned char GetCoordSpaceNumberFromName(char *CoordSpaceName);
		void OffsetTexCenter(int Channel, double Offset);
		int IsNeedRendered(Object3DEffect *Object3D, int MultipleUVMappingsOK, unsigned char &PrevMapUsed);	// does the texture need to be rendered for exporting 3d objects
		int IsTextureTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY);

		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		bool GetRAHostAnimatedInclVelocity(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_ROOTTEXTURE);};
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);
		int BuildFileComponentsList(EffectList **Coords);
		void ResolveLoadLinkages(EffectsLib *Lib);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ROOTTEXTURE);};
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual char *GetCritterName(RasterAnimHost *TestMe);
		virtual char *GetRAHostTypeString(void)					{return ("(Texture)");};
		virtual void EditRAHost(void);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // class RootTexture

#define WCS_TEXTUREAA_PRESAMPLES	10000
#define WCS_TEXTUREAA_SAMPLEBINS	1000
#define WCS_TEXTUREAA_MAX_SAMPLES	100

class TextureAA
	{
	private:
		unsigned short BinCount[WCS_TEXTUREAA_SAMPLEBINS][3];
		float BinSum[WCS_TEXTUREAA_SAMPLEBINS][3];
		static char TableInitialized;
		void CreateTextureAASamplePRNTable(void);

	public:
		static double TextureAARNGTable[WCS_TEXTUREAA_MAX_SAMPLES * 3];

		void Init(Texture *AAMe);
		TextureAA();
		inline void ProcessSamplePoints(double *Output, double MinR, double MaxR, double MinG, double MaxG, double MinB, double MaxB);

	}; // class TextureAA

class Texture : public RasterAnimHost
	{
	public:
		// all textures
		Texture *Tex[WCS_TEXTURE_MAXPARAMTEXTURES], *Next, *Prev;
		unsigned char TexType, Enabled, Antialias, ApplyToEcosys, ApplyToColor, SelfOpacity, AASamples, Rotated, Misc,
			ThreeD, Quantize, AbsoluteOutput, /*UseUVWCoords, */ParamFlags[WCS_TEXTURE_MAXPARAMTEXTURES], CoordSpace;
		double North, South, East, West, LatRange, LonRange;	// used by renderer
		double TSize[3], TCenter[3], TFalloff[3], TRotation[3];	// used during evaluation
		char *ParamName[WCS_TEXTURE_MAXPARAMTEXTURES], MapName[WCS_UVMAP_MAXNAMELENGTH];
		RasterShell *Img;
		IncludeExcludeList *InclExcl;
		IncludeExcludeTypeList *InclExclType;
		TextureAA *AASampler;
		CoordSys *Coords;	// used by renderer
		Raster *Rast;	// used by renderer
		Matx3x3 RotMatx;


			// ecosystem textures only
			//unsigned char AlignToVector;

			// color textures only
			short PalColor[2];
			AnimColorGradient ColorGrad;

			// if not (bitmap and ecosystem and LatLonBounds)
			AnimDoubleTime TexCenter[3], TexFalloff[3];
			
			// if not (bitmap and Front Projection)
			AnimDoubleTime TexVelocity[3];

			// if not (bitmap and ((ecosystem and LatLonBounds) or FrontProjection))
			AnimDoubleTime TexRotation[3];

			// depending on type of projection for bitmaps
			AnimDoubleTime TexSize[3];

			// if texture type requires (most procedurals don't)
			unsigned char TexAxis;

		// bitmaps
		unsigned char ImageNeg, PixelBlend, ReverseWidth, ReverseHeight, AlphaOnly;

			// if cylindrical projection
			double WrapWidth;
			
			// if cylindrical and spherical projection;
			double WrapHeight;

			// if not (ecosystem and LatLonBounds)
				// if planar or cubic
				unsigned char TileWidth, FlipWidth;

				// if planar or cubic or cylindrical
				unsigned char TileHeight, FlipHeight;

			// ecosystems only
			//unsigned char LatLonBounds;

				// if LatLonBounds
				//GeoRegister GeoReg;
				AnimDoubleTime TexFeather;

		// procedurals
		AnimDoubleTime TexParam[WCS_TEXTURE_MAXPARAMS];		

		// common methods
		Texture(RasterAnimHost *RAHost, unsigned char SetTexType);
		Texture(RasterAnimHost *RAHost, unsigned char SetTexType, unsigned char EcosysSource, unsigned char ColorSource, AnimColorTime *DefaultColor);
		virtual ~Texture();
		void SetDefaults(unsigned char SetTexType, AnimColorTime *DefaultColor);
		void CopyAndTransfer(Texture *CopyFrom);
		void Copy(Texture *CopyTo, Texture *CopyFrom);
		void RepairApplyToColor(int ParentApplyColor);
		Texture *NewTexture(long ParamNum, Texture *Proto, unsigned char NewTexType);
		Texture *MatchRasterShell(RasterShell *Shell);
		void SetRaster(Raster *NewRast);
		int IsAnimated(void);
		int ParamAvailable(int ParamNum) {return (ParamFlags[ParamNum] & WCS_TEXTURE_PARAMAVAIL);};
		int TextureAvailable(int ParamNum) {return (ParamFlags[ParamNum] & WCS_TEXTURE_TEXTUREAVAIL);};
		AnimDoubleTime *GetParamPtr(int ParamNum)	{ return (ParamAvailable(ParamNum) ? &TexParam[ParamNum]: NULL);};
		char *GetParamLabel(int ParamNum)	{ return (ParamAvailable(ParamNum) ? ParamName[ParamNum]: NULL);};
		unsigned char GetTexType(void) {return (TexType);};
		Texture *RemoveTexture(Texture *RemoveMe);
		int EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneSampleLine(TextureSampleData *Samp);
		double Eval(double Output[3], TextureData *Data, int EvalChildren, int StrataBlend);
		void EvalStrataBlend(double &Value, TextureData *Data);
		double ExtractTexDataValues(TextureData *Data, TextureExtractionData *Extraction);
		//double ExtractDynamicValues(TextureData *Data);
		int FindTextureNumberInParent(void);
		Texture *FindTexture(Texture *FindMe);
		long FindTextureOwnerType(void);
		RootTexture *GetRoot(void);
		int GetApplicationLegal(unsigned char Ecosys);
		int ConfigureForEcosystem(void);
		int ConfigureForObject3D(void);
		long InitImageIDs(long &ImageID);
		long SetColorList(short *ColorList, long &ListPos);
		int TextureRotated(void) {return (TexRotation[0].CurValue != 0.0 || TexRotation[1].CurValue != 0.0 || TexRotation[2].CurValue != 0.0);};
		int TextureRotatedT(void) {return (TRotation[0] != 0.0 || TRotation[1] != 0.0 || TRotation[2] != 0.0);};
		void BuildTextureRotationMatrix(int EvalChildren);
		void BuildTextureRotationMatrixT(void);
		inline void ReCenterMoveRotate(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		inline void ReSize(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		inline void ReSizeT(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		inline double MaxSampleSize(double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ);
		inline double FractalSetup(double MaxOctaves, double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		inline double CellBasisSetup(double MaxOctaves, double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		inline double AnalyzeWrapup(double Output[3], double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], double Value, TextureData *Data, int EvalChildren);
		inline double AnalyzeImageWrapup(double Output[3], double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], double Value, TextureData *Data, int EvalChildren);
		inline void AdjustSampleForVelocity(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ);
		double AdjustSampleForLatLonBounds(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ, 
			TextureData *Data, TextureExtractionData *Extraction);
		int ComputeVectorOffsets(TextureData *Data);
		double AdjustSampleForAlignToVector(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ, 
			TextureData *Data, TextureExtractionData *Extraction);
		int TextureMoving(void) {return ((TexVelocity[0].CurValue != 0.0 || TexVelocity[1].CurValue != 0.0 || TexVelocity[2].CurValue != 0.0) && (TexType != WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE && TexType != WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE));};
		double ObtainCylindricalX(double LowX, double HighX, double LowZ, double HighZ, double &PtXLow, double &PtXHigh);
		double ObtainSphericalY(double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double &PtYLow, double &PtYHigh);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		int InitAAChain(void);
		int InitAAConditional(void);
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		int InitGeoReg(void);
		TextureAA *InitAA(void);
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		bool GetRAHostAnimatedInclVelocity(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_TEXTURE);};
		char *GetTextureName(long TexNumber); 
		int GetTexEnabled(long TexNumber)	{return (Tex[TexNumber] && Tex[TexNumber]->Enabled);};
		void SetBounds(double LatRange[2], double LonRange[2]);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);
		int GetAbsoluteOutputRange(long ParamNumber, double RangeDefaults[3], double &CurrentValue, double &Multiplier, unsigned char &MetricType);
		void SetDimensionUnits(void);
		int BuildFileComponentsList(EffectList **Coords);
		void SetupForInputParamTexture(long ParamNumber);
		int SetSizeToFitMaterial(void);
		int ApproveInclExclClass(long MyClass);
		void ResolveLoadLinkages(EffectsLib *Lib);
		void SetCoordSpace(unsigned char NewSpace, int SendNotify);
		void PropagateCoordSpace(unsigned char SpaceNumber);
		int ApproveCoordSpace(unsigned char TestSpace);
		unsigned char GetDefaultCoordSpace(unsigned char SetTexType);
		void SetVertexMap(char *NewMap);
		char **GetVertexMapNames(int MapType);	// 0 = UVW, 1 = ColorPerVertex
		int UsesSeed(void);
		void OffsetTexCenter(int Channel, double Offset);
		int IsTextureTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY,
			int &SizeDoesMatter);

		virtual char GetTextureParamType(int ParamNum) = 0;
		virtual char GetTextureTypeClass(void) = 0;
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, 
			int EvalChildren, int StrataBlend) {Output[0] = Output[1] = Output[2] = 0.0; return (0.0);};
		virtual void InitNoise(void) {return;};
		virtual void InitBasis(void) {return;};
		virtual int ReInitNoise(int ParamNum) {return (0);};
		virtual int ReInitBasis(int ParamNum) {return (0);};
		virtual void SetMiscDefaults(void) {return;};
		virtual int IsColorChild(int ParamNum)	{return (0);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_TEXTURE);};
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual char *GetRAHostTypeString(void)					{return ("(Texture)");};
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual char *GetCritterName(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // class Texture

class PlanarImageTexture : public Texture
	{
	public:
	
		PlanarImageTexture(RasterAnimHost *RAHost, long ParamNumber);
		PlanarImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~PlanarImageTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class PlanarImageTexture

class CylindricalImageTexture : public Texture
	{
	public:
	
		CylindricalImageTexture(RasterAnimHost *RAHost, long ParamNumber);
		CylindricalImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~CylindricalImageTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class CylindricalImageTexture

class SphericalImageTexture : public Texture
	{
	public:
	
		SphericalImageTexture(RasterAnimHost *RAHost, long ParamNumber);
		SphericalImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~SphericalImageTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SphericalImageTexture

class CubicImageTexture : public Texture
	{
	public:
	
		CubicImageTexture(RasterAnimHost *RAHost, long ParamNumber);
		CubicImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~SphericalImageTexture();

	}; // class CubicImageTexture

class FrontProjectionTexture : public Texture
	{
	public:
	
		FrontProjectionTexture(RasterAnimHost *RAHost, long ParamNumber);
		FrontProjectionTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~FrontProjectionTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class FrontProjectionTexture

class EnvironmentMapTexture : public Texture
	{
	public:
	
		EnvironmentMapTexture(RasterAnimHost *RAHost, long ParamNumber);
		EnvironmentMapTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~EnvironmentMapTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class EnvironmentMapTexture

class UVImageTexture : public Texture
	{
	public:
	
		UVImageTexture(RasterAnimHost *RAHost, long ParamNumber);
		UVImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_IMAGE);};
		//~UVImageTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class UVImageTexture

class ColorPerVertexTexture : public Texture
	{
	public:
	
		ColorPerVertexTexture(RasterAnimHost *RAHost, long ParamNumber);
		ColorPerVertexTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_VERTEX);};
		//~ColorPerVertexTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class ColorPerVertexTexture

#define WCS_TEXTURE_STRIPE_CENTER		3
#define WCS_TEXTURE_STRIPE_WIDTH		4
#define WCS_TEXTURE_STRIPE_BRIGHTNESS	7
#define WCS_TEXTURE_STRIPE_CONTRAST		8

class StripeTexture : public Texture
	{
	public:
	
		StripeTexture(RasterAnimHost *RAHost, long ParamNumber);
		StripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~StripeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class StripeTexture

class SoftStripeTexture : public Texture
	{
	public:
	
		SoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber);
		SoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SoftStripeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SoftStripeTexture

#define WCS_TEXTURE_SINGLESTRIPE_LENGTH		5
#define WCS_TEXTURE_SINGLESTRIPE_OFFSET		6

class SingleStripeTexture : public Texture
	{
	public:
	
		SingleStripeTexture(RasterAnimHost *RAHost, long ParamNumber);
		SingleStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SingleStripeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SingleStripeTexture


class SingleSoftStripeTexture : public Texture
	{
	public:
	
		SingleSoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber);
		SingleSoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SingleSoftStripeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SingleSoftStripeTexture


#define WCS_TEXTURE_WOODGRAIN_FREQUENCIES	3
#define WCS_TEXTURE_WOODGRAIN_TURBULENCE	4
#define WCS_TEXTURE_WOODGRAIN_RINGSPACING	5
#define WCS_TEXTURE_WOODGRAIN_RINGSHARPNESS	6
#define WCS_TEXTURE_WOODGRAIN_BRIGHTNESS	7
#define WCS_TEXTURE_WOODGRAIN_CONTRAST		8

class WoodGrainTexture : public Texture
	{
	public:
		fBmNoise *Noise;
	
		WoodGrainTexture(RasterAnimHost *RAHost, long ParamNumber);
		WoodGrainTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~WoodGrainTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_WOODGRAIN_FREQUENCIES);};

	}; // class WoodGrainTexture

class MarbleTexture : public Texture
	{
	public:
		fBmNoise *Noise;
	
		MarbleTexture(RasterAnimHost *RAHost, long ParamNumber);
		MarbleTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MarbleTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_WOODGRAIN_FREQUENCIES);};

	}; // class MarbleTexture

#define WCS_TEXTURE_BRICK_ROWSHIFT		3
#define WCS_TEXTURE_BRICK_XMORTARWIDTH	5
#define WCS_TEXTURE_BRICK_YMORTARWIDTH	6
#define WCS_TEXTURE_BRICK_BRIGHTNESS	7
#define WCS_TEXTURE_BRICK_CONTRAST		8

class BrickTexture : public Texture
	{
	public:
	
		BrickTexture(RasterAnimHost *RAHost, long ParamNumber);
		BrickTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~BrickTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class BrickTexture


#define WCS_TEXTURE_DOTS_ROWSHIFT		3
#define WCS_TEXTURE_DOTS_COVERAGE		5
#define WCS_TEXTURE_DOTS_DENSITY		6
#define WCS_TEXTURE_DOTS_BRIGHTNESS		7
#define WCS_TEXTURE_DOTS_CONTRAST		8

class DotsTexture : public Texture
	{
	public:
	
		DotsTexture(RasterAnimHost *RAHost, long ParamNumber);
		DotsTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~DotsTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class DotsTexture


#define WCS_TEXTURE_VEIN_OCTAVES		3

class VeinTexture : public Texture
	{
	public:
	
		VeinTexture(RasterAnimHost *RAHost, long ParamNumber);
		VeinTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~VeinTexture();

	}; // class VeinTexture

#define WCS_TEXTURE_CRUST_OCTAVES		3

class CrustTexture : public Texture
	{
	public:
	
		CrustTexture(RasterAnimHost *RAHost, long ParamNumber);
		CrustTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~CrustTexture();

	}; // class CrustTexture

#define WCS_TEXTURE_UNDERWATER_WAVES		3

class UnderwaterTexture : public Texture
	{
	public:
	
		UnderwaterTexture(RasterAnimHost *RAHost, long ParamNumber);
		UnderwaterTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~UnderwaterTexture();

	}; // class UnderwaterTexture

#define WCS_TEXTURE_FRACTALNOISE_OCTAVES		3
#define WCS_TEXTURE_FRACTALNOISE_INPUTSEED		4
#define WCS_TEXTURE_FRACTALNOISE_ROUGHNESS		5
#define WCS_TEXTURE_FRACTALNOISE_LACUNARITY		6
#define WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS		7
#define WCS_TEXTURE_FRACTALNOISE_CONTRAST		8

class FractalNoiseTexture : public Texture
	{
	private:
		fBmNoise *Noise;

	public:

		FractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber);
		FractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~FractalNoiseTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY);};

	}; // class FractalNoiseTexture

#define WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET	9

class MultiFractalNoiseTexture : public Texture
	{
	private:
		MultiFractal *Noise;

	public:

		MultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber);
		MultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MultiFractalNoiseTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY || ParamNum == WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET);};

	}; // class MultiFractalNoiseTexture

class HybridMultiFractalNoiseTexture : public Texture
	{
	private:
		HybridMultiFractal *Noise;

	public:

		HybridMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber);
		HybridMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~HybridMultiFractalNoiseTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY || ParamNum == WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET);};

	}; // class HybridMultiFractalNoiseTexture

#define WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN	10

class RidgedMultiFractalNoiseTexture : public Texture
	{
	private:
		RidgedMultiFractal *Noise;

	public:

		RidgedMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber);
		RidgedMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~RidgedMultiFractalNoiseTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY || ParamNum == WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET || ParamNum == WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN);};

	}; // class RidgedMultiFractalNoiseTexture

class HeteroTerrainNoiseTexture : public Texture
	{
	private:
		HeteroTerrain *Noise;

	public:

		HeteroTerrainNoiseTexture(RasterAnimHost *RAHost, long ParamNumber);
		HeteroTerrainNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~HeteroTerrainNoiseTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY || ParamNum == WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET);};

	}; // class HeteroTerrainNoiseTexture

class TurbulenceTexture : public Texture
	{
	private:
		fBmTurbulentNoise *Noise;

	public:

		TurbulenceTexture(RasterAnimHost *RAHost, long ParamNumber);
		TurbulenceTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~TurbulenceTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual int ReInitNoise(int ParamNum) {return (ParamNum == WCS_TEXTURE_FRACTALNOISE_OCTAVES || ParamNum == WCS_TEXTURE_FRACTALNOISE_ROUGHNESS || ParamNum == WCS_TEXTURE_FRACTALNOISE_LACUNARITY);};

	}; // class TurbulenceTexture

#define WCS_TEXTURE_CELLBASIS_OCTAVES		3
#define WCS_TEXTURE_CELLBASIS_BRIGHTNESS	7
#define WCS_TEXTURE_CELLBASIS_CONTRAST		8

class F1CellBasisTexture : public Texture
	{
	private:
		F1CellBasis3 *Basis;

	public:

		F1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber);
		F1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F1CellBasisTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_CELLBASIS_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F1CellBasisTexture

class F2CellBasisTexture : public Texture
	{
	private:
		F2CellBasis3 *Basis;

	public:

		F2CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber);
		F2CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F2CellBasisTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_CELLBASIS_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F2CellBasisTexture

class F2mF1CellBasisTexture : public Texture
	{
	private:
		F2mF1CellBasis3 *Basis;

	public:

		F2mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber);
		F2mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F2mF1CellBasisTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_CELLBASIS_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F2mF1CellBasisTexture

class F3mF1CellBasisTexture : public Texture
	{
	private:
		F3mF1CellBasis3 *Basis;

	public:

		F3mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber);
		F3mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F3mF1CellBasisTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_CELLBASIS_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F3mF1CellBasisTexture

#define WCS_TEXTURE_MANHATTAN_OCTAVES		3
#define WCS_TEXTURE_MANHATTAN_BRIGHTNESS	7
#define WCS_TEXTURE_MANHATTAN_CONTRAST		8

class F1ManhattanTexture : public Texture
	{
	private:
		F1Manhattan3 *Basis;

	public:

		F1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber);
		F1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F1ManhattanTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_MANHATTAN_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F1ManhattanTexture

class F2ManhattanTexture : public Texture
	{
	private:
		F2Manhattan3 *Basis;

	public:

		F2ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber);
		F2ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F2ManhattanTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_MANHATTAN_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F2ManhattanTexture

class F2mF1ManhattanTexture : public Texture
	{
	private:
		F2mF1Manhattan3 *Basis;

	public:

		F2mF1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber);
		F2mF1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~F2mF1ManhattanTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_MANHATTAN_OCTAVES || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class F2mF1ManhattanTexture

#define WCS_TEXTURE_BIRDSHOT_SHOTSIZE		3
#define WCS_TEXTURE_BIRDSHOT_DENSITY		4
#define WCS_TEXTURE_BIRDSHOT_COVERAGE		5
#define WCS_TEXTURE_BIRDSHOT_CELLSIZE		6

class BirdshotTexture : public Texture
	{
	private:
		fBmNoise *Noise;
		F1CellBasis3 *Basis;

	public:

		BirdshotTexture(RasterAnimHost *RAHost, long ParamNumber);
		BirdshotTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~BirdshotTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void InitNoise(void);
		virtual void InitBasis(void);
		virtual int ReInitBasis(int ParamNum) {return (ParamNum == WCS_TEXTURE_BIRDSHOT_DENSITY || ParamNum == WCS_TEXTURE_3DIMENSIONS);};

	}; // class BirdshotTexture

class AddTexture : public Texture
	{
	public:
	
		AddTexture(RasterAnimHost *RAHost, long ParamNumber);
		AddTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~AddTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class AddTexture

class SubtractTexture : public Texture
	{
	public:
	
		SubtractTexture(RasterAnimHost *RAHost, long ParamNumber);
		SubtractTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SubtractTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SubtractTexture

class MultiplyTexture : public Texture
	{
	public:
	
		MultiplyTexture(RasterAnimHost *RAHost, long ParamNumber);
		MultiplyTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MultiplyTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class MultiplyTexture

#define WCS_TEXTURE_COMPOSITE_MASK		3

class CompositeTexture : public Texture
	{
	public:
	
		CompositeTexture(RasterAnimHost *RAHost, long ParamNumber);
		CompositeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~CompositeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class CompositeTexture

#define WCS_TEXTURE_CONTRAST_CONTRAST		3
#define WCS_TEXTURE_CONTRAST_MEDIAN			5

class ContrastTexture : public Texture
	{
	public:
	
		ContrastTexture(RasterAnimHost *RAHost, long ParamNumber);
		ContrastTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~ContrastTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class ContrastTexture

#define WCS_TEXTURE_DARKEN_MASK		3

class DarkenTexture : public Texture
	{
	public:
	
		DarkenTexture(RasterAnimHost *RAHost, long ParamNumber);
		DarkenTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~DarkenTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class DarkenTexture

#define WCS_TEXTURE_LIGHTEN_MASK		3

class LightenTexture : public Texture
	{
	public:
	
		LightenTexture(RasterAnimHost *RAHost, long ParamNumber);
		LightenTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~LightenTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class LightenTexture


#define WCS_TEXTURE_LEVELS_LOW		3
#define WCS_TEXTURE_LEVELS_HIGH		4
#define WCS_TEXTURE_LEVELS_MID		5

class LevelsTexture : public Texture
	{
	public:
	
		LevelsTexture(RasterAnimHost *RAHost, long ParamNumber);
		LevelsTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~LevelsTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class LevelsTexture


#define WCS_TEXTURE_HSVMERGE_HUE		3
#define WCS_TEXTURE_HSVMERGE_SAT		4
#define WCS_TEXTURE_HSVMERGE_VAL		5

class HSVMergeTexture : public Texture
	{
	public:
	
		HSVMergeTexture(RasterAnimHost *RAHost, long ParamNumber);
		HSVMergeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~HSVMergeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual int IsColorChild(int ParamNum)	{return (ParamNum == WCS_TEXTURE_HSVMERGE_HUE ||
			ParamNum == WCS_TEXTURE_HSVMERGE_SAT || ParamNum == WCS_TEXTURE_HSVMERGE_VAL);};

	}; // class HSVMergeTexture


#define WCS_TEXTURE_SKEW_MASK			3
#define WCS_TEXTURE_SKEW_MEDIANIN		5
#define WCS_TEXTURE_SKEW_MEDIANOUT		6

class SkewTexture : public Texture
	{
	public:
	
		SkewTexture(RasterAnimHost *RAHost, long ParamNumber);
		SkewTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SkewTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SkewTexture

#define WCS_TEXTURE_BELLCURVE_REPEAT		2
#define WCS_TEXTURE_BELLCURVE_AMPLITUDE		3
#define WCS_TEXTURE_BELLCURVE_SHIFT			4
#define WCS_TEXTURE_BELLCURVE_FREQUENCY		5
#define WCS_TEXTURE_BELLCURVE_PHASE			6
#define WCS_TEXTURE_BELLCURVE_PHASESCALE	7
#define WCS_TEXTURE_BELLCURVE_SKEW			8
#define WCS_TEXTURE_BELLCURVE_LOWERCLAMP	9
#define WCS_TEXTURE_BELLCURVE_UPPERCLAMP	10

class BellCurveTexture : public Texture
	{
	public:
	
		BellCurveTexture(RasterAnimHost *RAHost, long ParamNumber);
		BellCurveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~BellCurveTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class BellCurveTexture

#define WCS_TEXTURE_SQUAREWAVE_WIDTH	8

class SquareWaveTexture : public Texture
	{
	public:
	
		SquareWaveTexture(RasterAnimHost *RAHost, long ParamNumber);
		SquareWaveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SquareWaveTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SquareWaveTexture

class SawtoothTexture : public Texture
	{
	public:
	
		SawtoothTexture(RasterAnimHost *RAHost, long ParamNumber);
		SawtoothTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SawtoothTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SawtoothTexture

#define WCS_TEXTURE_STEP_STEPS	5

class StepTexture : public Texture
	{
	public:
	
		StepTexture(RasterAnimHost *RAHost, long ParamNumber);
		StepTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~StepTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class StepTexture

class SlopeTexture : public Texture
	{
	public:
	
		SlopeTexture(RasterAnimHost *RAHost, long ParamNumber);
		SlopeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~SlopeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class SlopeTexture

#define WCS_TEXTURE_GAMMA_GAMMA		5

class GammaTexture : public Texture
	{
	public:
	
		GammaTexture(RasterAnimHost *RAHost, long ParamNumber);
		GammaTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~GammaTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ,
			 double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class GammaTexture

#define WCS_TEXTURE_BIAS_BIAS		5

class BiasTexture : public Texture
	{
	public:
	
		BiasTexture(RasterAnimHost *RAHost, long ParamNumber);
		BiasTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~BiasTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class BiasTexture

#define WCS_TEXTURE_GAIN_GAIN		5

class GainTexture : public Texture
	{
	public:
	
		GainTexture(RasterAnimHost *RAHost, long ParamNumber);
		GainTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~GainTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class GainTexture

class CustomCurveTexture : public Texture
	{
	public:
		AnimDoubleProfile *CurveADP;

		CustomCurveTexture(RasterAnimHost *RAHost, long ParamNumber);
		CustomCurveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~CustomCurveTexture();
		void DeleteSpecificResources(void);
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class CustomCurveTexture

class MaximumTexture : public Texture
	{
	public:
	
		MaximumTexture(RasterAnimHost *RAHost, long ParamNumber);
		MaximumTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MaximumTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class MaximumTexture

#define WCS_TEXTURE_MAXIMUMSWITCH_SWITCHA		3
#define WCS_TEXTURE_MAXIMUMSWITCH_SWITCHB		4

class MaximumSwitchTexture : public Texture
	{
	public:
	
		MaximumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber);
		MaximumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MaximumSwitchTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class MaximumSwitchTexture

class MinimumTexture : public Texture
	{
	public:
	
		MinimumTexture(RasterAnimHost *RAHost, long ParamNumber);
		MinimumTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MinimumTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class MinimumTexture

#define WCS_TEXTURE_MINIMUMSWITCH_SWITCHA		3
#define WCS_TEXTURE_MINIMUMSWITCH_SWITCHB		4

class MinimumSwitchTexture : public Texture
	{
	public:
	
		MinimumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber);
		MinimumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~MinimumSwitchTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class MinimumSwitchTexture

#define WCS_TEXTURE_THRESHOLD_THRESHOLD		3
#define WCS_TEXTURE_THRESHOLD_SAMPLE		4

class ThresholdTexture : public Texture
	{
	public:
	
		ThresholdTexture(RasterAnimHost *RAHost, long ParamNumber);
		ThresholdTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~ThresholdTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class ThresholdTexture

#define WCS_TEXTURE_TERRAINPARAM_INPUTLOW		3
#define WCS_TEXTURE_TERRAINPARAM_INPUTHIGH		4

class TerrainParameterTexture : public Texture
	{
	public:
	
		TerrainParameterTexture(RasterAnimHost *RAHost, long ParamNumber);
		TerrainParameterTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PARAMETER);};
		//~TerrainParameterTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);
		virtual void SetMiscDefaults(void);

	}; // class TerrainParameterTexture

class GradientTexture : public Texture
	{
	public:
	
		GradientTexture(RasterAnimHost *RAHost, long ParamNumber);
		GradientTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_PROCEDURAL);};
		//~GradientTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class GradientTexture

class IncludeExcludeTexture : public Texture
	{
	public:
	
		IncludeExcludeTexture(RasterAnimHost *RAHost, long ParamNumber);
		IncludeExcludeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_OBJECT);};
		//~IncludeExcludeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class IncludeExcludeTexture

class IncludeExcludeTypeTexture : public Texture
	{
	public:
	
		IncludeExcludeTypeTexture(RasterAnimHost *RAHost, long ParamNumber);
		IncludeExcludeTypeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor);
		void SetDefaults(long ParamNumber);
		virtual char GetTextureParamType(int ParamNum);
		virtual char GetTextureTypeClass(void) {return (WCS_TEXTURE_TYPECLASS_OBJECT);};
		//~IncludeExcludeTypeTexture();
		virtual double Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, 
			double HighZ, double Value[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend);

	}; // class IncludeExcludeTypeTexture

// <<<>>> ADD_NEW_TEXTURE

/*
// interface
// always display
'texture list
'add, remove, up-down texture
'texture type
'enabled
'antialias
'self alpha
'use alpha
'opacity

// if (color)
'palette colors
'mix range
// else
'output range (mix range)

// if (eco & bitmap)
'lat-lon bounds
'align to vector - ghost if (lat-lon bounds)

// if (eco & bitmap & lat-lon bounds)
'N, S, W, E, 
'texture feathering
// else
'texture falloff
'texture center XYZ/lat-lon - configure widgets and labels based on whether eco or not
'texture velocity - ghost if not needed
'texture size - ghost if not needed
'texture axis - ghost if not needed

// if (bitmap)
'wrap width - ghost if not cylindrical or sherical
'wrap height - ghost if not spherical
'tile width - ghost if (eco & lat-lon bounds) or (not (planar or cubic))
'tile height - ghost if (eco & lat-lon bounds) or (not (planar or cubic or cylindrical))
'flip width - ghost if (eco & lat-lon bounds) or (not (planar or cubic))
'flip height - ghost if (eco & lat-lon bounds) or (not (planar or cubic or cylindrical))
// else
'TexParam - change labels and ghost items not needed based on procedural type
*/

// file definitions
#define WCS_ROOTTEXTURE_NAME				0x00610000
#define WCS_ROOTTEXTURE_APPLYTOECOSYS		0x00620000
#define WCS_ROOTTEXTURE_APPLYTOCOLOR		0x00630000
#define WCS_ROOTTEXTURE_SHOWSIZE			0x00640000
#define WCS_ROOTTEXTURE_SHOWROOTNAIL		0x00650000
#define WCS_ROOTTEXTURE_SHOWCOMPCHILDNAIL	0x00660000
#define WCS_ROOTTEXTURE_SHOWCOMPONENTNAIL	0x00670000
#define WCS_ROOTTEXTURE_PREVIEWSIZE_DBL		0x00680000
#define WCS_ROOTTEXTURE_TEXTURETYPE			0x00690000
#define WCS_ROOTTEXTURE_PREVIEWDIRECTION	0x006a0000
#define WCS_ROOTTEXTURE_APPLYTODISPLACE		0x006b0000
#define WCS_ROOTTEXTURE_ENABLED				0x006c0000
#define WCS_ROOTTEXTURE_BROWSEDATA			0x006d0000
#define WCS_ROOTTEXTURE_TEXTURE				0x00700000
#define WCS_ROOTTEXTURE_PREVIEWSIZE			0x00710000

#define WCS_TEXTURE_SAVEDMAXPARAMS			0x00710000
#define WCS_TEXTURE_SAVEDMAXPARAMTEXTURES	0x00720000
#define WCS_TEXTURE_ENABLED					0x00730000
#define WCS_TEXTURE_ANTIALIAS				0x00740000
#define WCS_TEXTURE_APPLYTOECOSYS			0x00750000
#define WCS_TEXTURE_APPLYTOCOLOR			0x00760000
#define WCS_TEXTURE_ALIGNTOVECTOR			0x00770000
#define WCS_TEXTURE_PALCOLOR1				0x00780000
#define WCS_TEXTURE_PALCOLOR2				0x00790000
#define WCS_TEXTURE_TEXAXIS					0x007a0000
#define WCS_TEXTURE_IMAGENEG				0x007b0000
#define WCS_TEXTURE_PIXELBLEND				0x007c0000
#define WCS_TEXTURE_WRAPWIDTH				0x007d0000
#define WCS_TEXTURE_WRAPHEIGHT				0x007e0000
#define WCS_TEXTURE_TILEWIDTH				0x007f0000
#define WCS_TEXTURE_FLIPWIDTH				0x00810000
#define WCS_TEXTURE_TILEHEIGHT				0x00820000
#define WCS_TEXTURE_FLIPHEIGHT				0x00830000
#define WCS_TEXTURE_LATLONBOUNDS			0x00840000
#define WCS_TEXTURE_NORTH					0x00850000
#define WCS_TEXTURE_SOUTH					0x00860000
#define WCS_TEXTURE_WEST					0x00870000
#define WCS_TEXTURE_EAST					0x00880000
#define WCS_TEXTURE_TEXFEATHER				0x00890000
#define WCS_TEXTURE_SELFOPACITY				0x008a0000
#define WCS_TEXTURE_AASAMPLES				0x008b0000
#define WCS_TEXTURE_IMAGEID					0x008c0000
#define WCS_TEXTURE_THREED					0x008d0000
#define WCS_TEXTURE_V5MISC					0x008e0000
#define WCS_TEXTURE_REVERSEWIDTH			0x008f0000
#define WCS_TEXTURE_REVERSEHEIGHT			0x00910000
#define WCS_TEXTURE_COLORGRADIENT			0x00920000
#define WCS_TEXTURE_BROWSEDATA				0x00930000
#define WCS_TEXTURE_GEOREG					0x00940000
#define WCS_TEXTURE_QUANTIZE				0x00950000
#define WCS_TEXTURE_MISC					0x00960000
#define WCS_TEXTURE_CURVEADP				0x00970000
#define WCS_TEXTURE_ABSOLUTEOUTPUT			0x00980000
#define WCS_TEXTURE_USEUVWCOORDS			0x00990000
#define WCS_TEXTURE_INCLUDEEXCLUDE			0x009a0000
#define WCS_TEXTURE_INCLUDEEXCLUDETYPE		0x009b0000
#define WCS_TEXTURE_COORDSPACE				0x009c0000
#define WCS_TEXTURE_MAPNAME					0x009d0000
#define WCS_TEXTURE_ALPHAONLY				0x009e0000

// 3
#define WCS_TEXTURE_TEXCENTER				0x00a10000
#define WCS_TEXTURE_TEXFALLOFF				0x00a20000
#define WCS_TEXTURE_TEXVELOCITY				0x00a30000
#define WCS_TEXTURE_TEXSIZE					0x00a40000
#define WCS_TEXTURE_TEXROTATION				0x00a50000

// # params
#define WCS_TEXTURE_PARAMNAME				0x00a80000	// Stored in case ever needed
#define WCS_TEXTURE_PARAMFLAGS				0x00a90000
#define WCS_TEXTURE_TEXPARAM				0x00aa0000

// # textures
#define WCS_TEXTURE_TEXTURENUMBER			0x00b10000
#define WCS_TEXTURE_TEXTURETYPE				0x00b20000
#define WCS_TEXTURE_USETEXTURE				0x00b30000
#define WCS_TEXTURE_TEXTURE					0x00b40000



#endif // WCS_TEXTURE_H
