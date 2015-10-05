// CustomModules.h
// Custom classes derived from AppModule for each interactive section of
// the program.
// Created from scratch on 26 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_CUSTOMMODULES_H
#define WCS_CUSTOMMODULES_H

class Database;
class WCSApp;
class WCSMapModule;
class GUIContext;

#include "AppModule.h"
#include "Fenetre.h"
#include "Requester.h"

class WCSMapModule : public WCSModule
	{
	private:
		double CenLat, CenLon, MapScale;
		GUIContext *OSGUI;
		DrawingFenetre MapWin, PalWin;
	
	public:
		FileReq *LoadMap;
		DEM Topo;

		WCSMapModule(GUIContext *Pane);
		~WCSMapModule();
		void Open(void);
		void SetView(double Lat, double Lon, double Zoom);
		void SetCenter(double Lat, double Lon);
		void SetZoom(double Zoom);
		void AutoCenter(Database *DB);
		void Clear(void);
		void Draw(Database *DB, unsigned short X = NULL, unsigned short Y = NULL,
 		 unsigned short W = NULL, unsigned short H = NULL);
		void DrawTopo(void);
		long HandleEvent(AppEvent *Activity, WCSApp *AppScope);
	}; // WCSMapModule

#endif // WCS_CUSTOMMODULES_H
