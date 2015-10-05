// FeatureConfig.h
// Contains a large list of difficult-to-manage defines for various build configurations

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_FEATURE_CONFIG_H
#define WCS_FEATURE_CONFIG_H

#define WCS_SUPPORT_3DS

// VNS 1.0
#ifdef WCS_BUILD_VNS
	#define WCS_SUPPORT_GENERIC_ATTRIBS
	#define WCS_SEARCH_QUERY
	#define WCS_THEMATIC_MAP
	#define WCS_COORD_SYSTEM
	#define WCS_VIEW_TERRAFFECTORS
#endif // WCS_BUILD_VNS

// WCS 6
#ifdef WCS_BUILD_W6
	#define WCS_FENCE_LIMITED
#endif // WCS_BUILD_W6

// VNS 2
#ifdef WCS_BUILD_V2
	#define WCS_EXPRESSIONS
	#define WCS_VIEW_DRAWWALLS
	#define WCS_DEM_TFX_FREEZE
	#define WCS_DEM_MERGE
	#define WCS_RENDER_SCENARIOS
	#define WCS_RENDER_TILES
	#define WCS_ISLAND_EFFECTS
	#define WCS_IMAGE_MANAGEMENT
	#define WCS_IMAGE_MANAGEMENT_VIRTRES
	#define WCS_FORESTRY_WIZARD
	#define WCS_LABEL
	#define WCS_THEMATICMAP_POINTSTYLE
	#define WCS_VIEW_RIGHTCLICK_DRILLDOWN
#endif // WCS_BUILD_V2

// GARY
#ifdef WCS_BUILD_GARY
#include "GaryConfig.h"
#endif // WCS_BUILD_GARY

// CHRIS
#ifdef WCS_BUILD_CHRIS
#endif // WCS_BUILD_CHRIS

// FRANK
#ifdef WCS_BUILD_FRANK
#endif // WCS_BUILD_FRANK

#ifdef WCS_BUILD_RTX
#define WCS_BUILD_IMAGEFORMATSTL_SUPPORT
#endif // WCS_BUILD_RTX

#ifdef WCS_BUILD_SX2
#define WCS_BUILD_FBX
#endif // WCS_BUILD_SX2


#endif // !WCS_FEATURE_CONFIG_H
