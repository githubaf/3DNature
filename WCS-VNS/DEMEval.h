// DEMEval.h
// Code to support 'Freezing' of terraffectors into existing or new copies of DEMs
// Built from scratch and from ragged bits of ViewGUI and LWS/3DS export code
// on Mar 8 2002 by CXH to commemorate Nederland's Frozen Dead Guy Days.
// Copyright 2002, 3D Nature

#include "FeatureConfig.h"

#ifdef WCS_DEM_TFX_FREEZE


#ifndef WCS_DEMEVAL_H
#define WCS_DEMEVAL_H

class Database;
class DBEditGUI;
class SearchQuery;
class PlanetOpt;
class GeneralEffect;
class Joe;
class JoeDEM;
class DEM;
class InterCommon;

#include "Render.h"
#include "EffectsLib.h"

// seems like we'll want to preserve some state info here,
// so we'll make a class to handle it.
class DEMEval
	{
	private:
		char PrefixName[20];
		int MakeNewDEMs;
		DBEditGUI *SelectFromDBSelected;
		SearchQuery *SelectFromQuery;

		GeoBounds GeoClip;
		VertexData VTD;
		PolygonData PGD;
		RenderData Rend;
		PlanetOpt *DefPO;
		GeneralEffect *CurEffect;
		Joe *Clip;
		JoeDEM *MyDEM;
		DEM *Terrain;
		InterCommon *InterStash;

	public:
		DEMEval() : Rend(NULL) {PrefixName[0] = 0; MakeNewDEMs = 0; SelectFromDBSelected = NULL; SelectFromQuery = NULL;
		 DefPO = NULL; CurEffect = NULL; Clip = NULL; MyDEM = NULL; Terrain = NULL; InterStash = NULL;};
		// sets state and executes all in one
		int FreezeDEMTFXUI(DBEditGUI *SelectFromDB, SearchQuery *SelectFromQ);
		// sets state and executes all in one
		int FreezeDEMTFX(DBEditGUI *SelectFromDB, SearchQuery *SelectFromQ, int NewDEMs, char *DEMPrefixName);
		// executes from pre-set state
		int Freeze(int Notify);
		// prepares for Freeze
		int PrepFreeze(void);
		// Freezes one DEM with current settings
		int FreezeDEM(int Notify);
		// cleanup
		void CleanupFromFreeze(void);

		inline double DEUnCalcExag(double Elev);
	}; // DEMEval


inline double DEMEval::DEUnCalcExag(double Elev)
{
double Exag;
Exag = DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
if(Exag == 0.0) return(Elev);
return(DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue + (Elev - DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) / Exag);
} // DEMEval::DEUnCalcExag

#endif // WCS_DEMEVAL_H

#endif // WCS_DEM_TFX_FREEZE
