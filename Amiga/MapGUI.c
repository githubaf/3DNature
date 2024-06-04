/* MapGUI.c
**
** MUI user interface stuff for 2D mapping module
** Build on 27 Apr 1994 by Chris "Xenon" Hanson from code originally
** written into Map.c in April 1994.
** Copyright 1994 by Questar
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "Defines.h"
#include "WCS.h"
#include "GUIDefines.h"
#include "GUIExtras.h"

#define MED_HACK

STATIC_VAR ULONG MapWind0_Sig;
STATIC_VAR APTR UnderConstOK;

STATIC_VAR USHORT PrimaryColors[16]=
{
#ifndef DAVE_WARNER
 0x89b, /* 0, gray-blue */
 0x000, /* 1, black */
 0xddd, /* 2, almost white */
#else /* DAVE_WARNER */
 0x000, /* 0, gray-blue, now black */
 0xfff, /* 1, black, now white */
 0xbbb, /* 2, almost white, now greyish */
#endif /* DAVE_WARNER */
 0xb10,
 0x348,
 0x392,
 0x37c,
 0xdd2,
 0xfff,
 0xf00,
 0xf70,
 0xff0,
 0x0f0,
 0x00f,
 0xf0f,
 0x000
 };

#ifdef VISTA_SCHEME

STATIC_VAR USHORT PrintColors[16]=
{
#ifndef DAVE_WARNER
 0x89b,   /* 0, gray-blue */
 0x000,   /* 1, black */
 0xddd,   /* 2, almost white */
#else /* DAVE_WARNER */
 0x000,   /* 0, gray-blue, now black */
 0xfff,   /* 1, black, now white */
 0xbbb,   /* 2, almost white, now greyish */
#endif /* DAVE_WARNER */
 0xb10,
 0x348,
 0x392,
 0x37c,
 0xdd2,
 0xfff,   /* VistaLike color scheme */
 0xbbb,
 0x777,
 0xd95,
 0x940,
 0x7b7,
 0x090,
 0x060
 }
;

#endif /* VISTA_SCHEME */


STATIC_VAR USHORT PrintColors[16]=
{
#ifndef DAVE_WARNER
 0x89b,   /* 0, gray-blue */
 0x000,   /* 1, black */
 0xddd,   /* 2, almost white */
#else /* DAVE_WARNER */
 0x000,   /* 0, gray-blue, now black */
 0xfff,   /* 1, black, now white */
 0xbbb,   /* 2, almost white, now greyish */
#endif /* DAVE_WARNER */
 0xb10,
 0x348,
 0x392,
 0x37c,
 0xdd2,
 0xfff,   /* lighten color gradient */
 0xeee,
 0xddd,
 0xccc,
 0xbbb,
 0xaaa,
 0x999,
 0x888
 }
;


STATIC_FCN void MapGUI_Del(struct MapData *MP); // used locally only -> static, AF 20.7.2021
STATIC_FCN short Make_Map_Menu(void); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Close_MA_Window(struct MapData *MP); // used locally only -> static, AF 20.7.2021
STATIC_FCN int MapGUI_New(struct MapData *MP); // used locally only -> static, AF 20.7.2021


extern void ParticleTree(void);

STATIC_FCN struct NewMenu MapNewMenus[] =
	{
	{ NM_TITLE, (CONST_STRPTR)"empty", 	 0 , 0, 0, 0 },                                  // "Database"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0 , 0, 0, 0 },                                  // "Load Objects"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"L", 0, 0, (APTR)(ID_MC_LOADALL) },      // "All"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"K", 0, 0, (APTR)(ID_MC_LOADACTIVE) },   // "Active"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"J", 0, 0, (APTR)(ID_MC_LOADTOPOS) },    // "Load Topos"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0,  0, 0, 0 },                                  // "Save Objects"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"S", 0, 0, (APTR)(ID_MC_SAVEALL) },      // "All"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"W", 0, 0, (APTR)(ID_MC_SAVEACTIVE) },   // "Active"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"B", 0, 0, (APTR)(ID_DE_WINDOW) },      // "Database"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"C", 0, 0, (APTR)(ID_MC_CLEAR) },       // "Clear Window"
	{  NM_ITEM, (CONST_STRPTR)"empty", (STRPTR)"/", 0, 0, (APTR)(ID_MC_PRINT) },         // "Print"
	{  NM_ITEM, (CONST_STRPTR)"empty", (STRPTR)"Q", 0, 0, (APTR)(ID_MC_QUIT) },          // "Close Map"

	{ NM_TITLE, (CONST_STRPTR)"empty",  0 , 0, 0, 0 },                                   // "View"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"H", 0, 0, (APTR)(ID_MC_CTRLTOFR) },// "Controls"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)",", 0, 0, (APTR)(ID_EL_WINDOW) },       // "Eco Legend"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"G", 0, 0, (APTR)(ID_MC_ALIGN) },        // "Align Map"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"E", 0, 0, (APTR)(ID_MC_CENTER) },       // "Center"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"A", 0, 0, (APTR)(ID_MC_AUTO) },         // "Auto Center"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"Z", 0, 0, (APTR)(ID_MC_ZOOM) },         // "Zoom"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"P", 0, 0, (APTR)(ID_MC_PAN) },          // "Pan"
	{  NM_ITEM, (CONST_STRPTR)"empty",  0 , 0, 0, 0 },                                   // "Move"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"+", 0, 0, (APTR)(ID_MC_ZOOMIN) },       // "In"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 (STRPTR)"-", 0, 0, (APTR)(ID_MC_ZOOMOUT) },     // "Out"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_MOVELEFT) },             // "Left"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_MOVERIGHT) },            // "Right"
	{   NM_SUB, (CONST_STRPTR)"empty",  0 , 0, 0, (APTR)(ID_MC_MOVEUP) },                // "Up"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_MOVEDOWN) },             // "Down"

	{ NM_TITLE, (CONST_STRPTR)"empty", 	 0 , 0, 0, 0 },                                  // "Draw"
	{  NM_ITEM, (CONST_STRPTR)"empty", 		 0,  0, 0, 0 },                              // "Draw Map"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 (STRPTR)"D", 0, 0, (APTR)(ID_MC_DRAW) },        // "Normal"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 (STRPTR)"R", 0, 0, (APTR)(ID_MC_REFINE) },      // "Refine"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_FRACTALMAP) },           // "Fractal Map"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"M", 0, 0, (APTR)(ID_MC_COLORMAP) },    // "Color Map"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty", (STRPTR)"_", 0, 0, (APTR)(ID_MC_FIXFLATS) },      // "Fix Flats"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_FINDDIST) },             // "Find Distance"

	{ NM_TITLE, (CONST_STRPTR)"empty",  0 , 0, 0, 0 },                                   // "Object"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"N", 0, 0, (APTR)(ID_MC_NEWOBJECT) },    // "New Object"
	{  NM_ITEM, (CONST_STRPTR)"empty",  0 , 0, 0, 0 },                                   // "Find Object"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"F", 0, 0, (APTR)(ID_MC_FINDMOUSE) },    // "Single"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"U", 0, 0, (APTR)(ID_MC_FINDMULTI) },    // "Multi"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"?", 0, 0, (APTR)(ID_MC_OUTLINE) },      // "Highlight"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty",  0,  0, 0, 0 },                                   // "Add Points"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"X", 0, 0, (APTR)(ID_MC_ADDPTSNEW) },    // "New"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"Y", 0, 0, (APTR)(ID_MC_ADDPTSAPPEND) }, // "Append"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"I", 0, 0, (APTR)(ID_MC_ADDPTSINSERT) }, // "Insert"
	{  NM_ITEM, (CONST_STRPTR)"empty",	 0,  0, 0, 0 },                                  // "Create Stream"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"~", 0, 0, (APTR)(ID_MC_STREAMNEW) },    // "New"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"`", 0, 0, (APTR)(ID_MC_STREAMAPPEND) }, // "Append"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)".", 0, 0, (APTR)(ID_MC_MODPTS) },       // "Modify Points"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"#", 0, 0, (APTR)(ID_MC_INPUT) },       // "Input Source"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0,  0, 0, 0 },                                  // "Conform Topo"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"!", 0, 0, (APTR)(ID_MC_CONFORMALL) },   // "All"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"T", 0, 0, (APTR)(ID_MC_CONFORMACT) },   // "Active"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"=", 0, 0, (APTR)(ID_MC_MATCHPTS) },    // "Match Points"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"O", 0, 0, (APTR)(ID_MC_MOVEORIG) },     // "Move Origin"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"&", 0, 0, (APTR)(ID_MC_DUPLICATE) },    // "Duplicate"
#ifdef ENABLE_STATISTICS
	{ NM_TITLE, (STRPTR)"Graph",		 0 , 0, 0, 0 },
	{  NM_ITEM, (STRPTR)"Load Stats",	 (STRPTR)"$", 0, 0, (APTR)(ID_MC_LOADSTATS) },
	{  NM_ITEM, (STRPTR)"Normalize",	 (STRPTR)"%", 0, 0, (APTR)(ID_MC_NORMALIZE) },
	{  NM_ITEM, (STRPTR)"Graph Style",	 (STRPTR)"^", 0, 0, (APTR)(ID_MC_GRAPHSTYLE) },
	{  NM_ITEM, (STRPTR)"Draw Graph",	 0,  0, 0, (APTR)(ID_MC_GRAPH) },
	{  NM_ITEM, (STRPTR)"Erase",		 (STRPTR)"*", 0, 0, (APTR)(ID_MC_ERASE) },
#endif /* ENABLE_STATISTICS */
	{ NM_TITLE, (CONST_STRPTR)"empty",  0 , 0, 0, 0 },  // "Motion"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)">", 0, 0, (APTR)(ID_MC_SETCAM) },       // "Set Camera"
	{  NM_ITEM, (CONST_STRPTR)"empty",  (STRPTR)"<", 0, 0, (APTR)(ID_MC_SETFOC) },       // "Set Focus"
	{  NM_ITEM, (CONST_STRPTR)"empty",  0,  0, 0, 0 },                                   // "Path"
	{   NM_SUB, (CONST_STRPTR)"empty", 	 (STRPTR)"5", 0, 0, (APTR)(ID_MC_INTERP) },      // "Interpolate"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"6", 0, 0, (APTR)(ID_MC_VECCAM) },       // "Vect-Camera"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"7", 0, 0, (APTR)(ID_MC_VECFOC) },       // "Vect-Focus"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"8", 0, 0, (APTR)(ID_MC_CAMVEC) },       // "Camera-Vect"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"9", 0, 0, (APTR)(ID_MC_FOCVEC) },       // "Focus-Vect"
	{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
	{  NM_ITEM, (CONST_STRPTR)"empty",  0,  0, 0, 0 },                                   // "Surface El"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"1", 0, 0, (APTR)(ID_MC_SURFONE) },      // "One"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"2", 0, 0, (APTR)(ID_MC_SURFTWO) },      // "Two"
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"3", 0, 0, (APTR)(ID_MC_SURFTHREE) },    // "Three",
	{   NM_SUB, (CONST_STRPTR)"empty",  (STRPTR)"4", 0, 0, (APTR)(ID_MC_SURFFOUR) },     // "Four"

	{ NM_TITLE, (CONST_STRPTR)"empty", 	 0 , 0, 0, 0 },                                  // "Windows"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_MC_BUILDDEM) },             // "DEM Designer"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 0 , 0, 0, (APTR)(ID_CL_WINDOW) },               // "Cloud Editor"
	{  NM_ITEM, (CONST_STRPTR)"empty",  0 , 0, 0, (APTR)(ID_WV_WINDOW(0)) },             // "Wave Editor"
	{  NM_ITEM, (CONST_STRPTR)"empty", 	 (STRPTR)"V", 0, 0, (APTR)(ID_MC_VIEWSHED) },    // "Viewshed"
/*
	{  NM_ITEM, (STRPTR)"Particle Tree",	 0 , 0, 0, (APTR)(ID_MC_PARTICLETREE) },
*/
	{ NM_END,   NULL,		 0,  0, 0, 0 }
	};

/* Fills the MapNewMenus structure with the real locale strings */
/* AF, HGW, 30.Mar.2024                                         */
/* ------------------------------------------------------------ */
STATIC_FCN void InitMapNewMenus(void)
{
	unsigned int i=0;
	static int Init=TRUE;
	if(Init)
	{
		Init=FALSE;
		MapNewMenus[i++].nm_Label= GetString( MSG_DB_DATABASE );     // "Database"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_LOADOBJECTS );  // "Load Objects"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ALL );          // "All"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ACTIVE );       // "Active"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_LOADTOPOS );    // "Load Topos"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_SAVEOBJECTS );  // "Save Objects"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ALL );          // "All"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ACTIVE );       // "Active"
		i++;   // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label= GetString( MSG_DB_DATABASE );     // "Database"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_CLEARWINDOW );  // "Clear Window"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_PRINT );        // "Print"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_CLOSEMAP );     // "Close Map"

		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_VIEW );         // "View"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_CONTROLS );     // "Controls"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ECOLEGEND );    // GetString( MSG_MAPGUI_ECOLEGEND )
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ALIGNMAP );     // "Align Map"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_CENTER );       // "Center"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_AUTOCENTER );   // "Auto Center"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_ZOOM );         // "Zoom"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_PAN );          // "Pan"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_MOVE );         // "Move"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_IN );           // "In"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_OUT );          // "Out"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_LEFT );         // "Left"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_RIGHT );        // "Right"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_UP );           // "Up"
		MapNewMenus[i++].nm_Label= GetString( MSG_MAPGUI_DOWN );         // "Down"

		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_DRAW );        // "Draw"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_DRAWMAP );     // "Draw Map"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_NORMAL );      // "Normal"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_REFINE );      // "Refine"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FRACTALMAP );  // "Fractal Map"
		i++;  // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  GetString( MSG_DATAOPSGUI_COLORMAP );    // "Color Map"
		i++;  // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FIXFLATS );    // "Fix Flats"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FINDDISTANCE );// "Find Distance"

		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_OBJECT );      // "Object"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_NEWOBJECT );   // "New Object"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FINDOBJECT );  // "Find Object"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_SINGLE );      // "Single"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_MULTI );       // "Multi"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_HIGHLIGHT );   // "Highlight"
		i++;  // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_ADDPOINTS );   // "Add Points"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_NEW );         // "New"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_APPEND );      // "Append"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_INSERT );      // "Insert"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_CREATESTREAM );// "Create Stream"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_NEW );         // "New"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_APPEND );      // "Append"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_MODIFYPOINTS );// "Modify Points"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_INPUTSOURCE ); // "Input Source"
		i++;  // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_CONFORMTOPO ); // "Conform Topo"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_ALL );         // "All"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_ACTIVE );      // "Active"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_MATCHPOINTS ); // "Match Points"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_MOVEORIGIN );  // "Move Origin"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_DUPLICATE );   // "Duplicate"
#ifdef ENABLE_STATISTICS
		MapNewMenus[i++].nm_Label=  { NM_TITLE, (STRPTR)"Graph",		 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Load Stats",	 (STRPTR)"$", 0, 0, (APTR)(ID_MC_LOADSTATS) },
		MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Normalize",	 (STRPTR)"%", 0, 0, (APTR)(ID_MC_NORMALIZE) },
		MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Graph Style",	 (STRPTR)"^", 0, 0, (APTR)(ID_MC_GRAPHSTYLE) },
		MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Draw Graph",	 0,  0, 0, (APTR)(ID_MC_GRAPH) },
		MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Erase",		 (STRPTR)"*", 0, 0, (APTR)(ID_MC_ERASE) },
#endif /* ENABLE_STATISTICS */
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_MOTION );      // "Motion"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_SETCAMERA );   // "Set Camera"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_SETFOCUS );    // "Set Focus"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_PATH );        // "Path"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_INTERPOLATE ); // "Interpolate"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_VECTCAMERA );  // "Vect-Camera"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_VECTFOCUS );   // "Vect-Focus"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_CAMERAVECT );  // "Camera-Vect"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FOCUSVECT );   // "Focus-Vect"
		i++;  // {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_SURFACEEL );   // "Surface El"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_ONE );         // "One"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_TWO );         // "Two"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_THREE );       // "Three",
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_FOUR );        // "Four"

		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_WINDOWS );     // "Windows"
		MapNewMenus[i++].nm_Label=  GetString( MSG_DEMGUI_DEMDESIGNER ); // "DEM Designer"
		MapNewMenus[i++].nm_Label=  GetString( MSG_CLOUDGUI_CLOUDEDITOR ); // "Cloud Editor"
		MapNewMenus[i++].nm_Label=  GetString( MSG_WAVGUI_WAVEEDITOR );  // "Wave Editor"
		MapNewMenus[i++].nm_Label=  GetString( MSG_MAPGUI_VIEWSHED );    // "Viewshed"
		/*
MapNewMenus[i++].nm_Label=  {  NM_ITEM, (STRPTR)"Particle Tree",	 0 , 0, 0, (APTR)(ID_MC_PARTICLETREE) },
		 */
	}
}


/***********************************************************************/

STATIC_FCN short Make_Map_Menu(void) // used locally only -> static, AF 20.7.2021
{
 short OpenOK = 0;

 InitMapNewMenus(); // Fills the MapNewMenus structure with the real locale strings

 if ((MP->VisualInfo = GetVisualInfo(MapWind0->WScreen, TAG_END)) != NULL)
  {
  if ((MP->MenuStrip = CreateMenus(MapNewMenus, GTMN_FrontPen, (ULONG)(0), TAG_END)) != NULL)
   {
   if (LayoutMenus(MP->MenuStrip, MP->VisualInfo, TAG_END))
    {
    if (SetMenuStrip(MapWind0, MP->MenuStrip))
     OpenOK = 1;
    } /* if LayoutMenus */
   } /* if MenuStrip */
  } /* if VisualInfo */

 return (OpenOK);

} /* Make_Map_Menu() */

/************************************************************************/

void MapIDCMP_Restore(struct Window *win)
{

 ModifyIDCMP(win, IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW
	| IDCMP_ACTIVEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS
	| IDCMP_RAWKEY | IDCMP_VANILLAKEY
#ifdef USE_WCS_HELP
#ifdef USE_MAP_HELP
	| IDCMP_MENUHELP
#endif /* USE_MAP_HELP */
#endif /* USE_WCS_HELP */
 );

} /* MapIDCMP_Restore() */

/***********************************************************************/

STATIC_FCN int MapGUI_New(struct MapData *MP) // used locally only -> static, AF 20.7.2021
{
 long open;
 static const char *StyleCycle[7]={NULL};// = {GetString( MSG_MAPGUI_SINGLE ),   // "Single"
                                         //  GetString( MSG_MAPGUI_MULTI ),    // "Multi"
                                         //  GetString( MSG_MAPGUI_SURFACE ),  // "Surface"
                                         //  GetString( MSG_MAPGUI_EMBOSS ),   // "Emboss"
                                         //  GetString( MSG_MAPGUI_SLOPE ),    // "Slope"
                                         //  GetString( MSG_MAPGUI_CONTOUR ),  // "Contour"
                                         //  NULL};
 static const char *ColorCycle[4]={NULL};// = {GetString( MSG_MAPGUI_GREY ),       // "Grey"
                                         //  GetString( MSG_MAPGUI_LIGHTGREY ),  // "Light Grey"
                                         //  GetString( MSG_MAPGUI_COLOR ),      // "Color"
                                         //   NULL};

static int Init=TRUE;

if(Init)
{
	unsigned int i=0;
	Init=FALSE;
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_SINGLE );   // "Single"
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_MULTI );    // "Multi"
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_SURFACE );  // "Surface"
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_EMBOSS );   // "Emboss"
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_SLOPE );    // "Slope"
	StyleCycle[i++] = (char*)GetString( MSG_MAPGUI_CONTOUR );  // "Contour"
	StyleCycle[i++] = NULL;

	i=0;
	ColorCycle[i++] = (char*)GetString( MSG_MAPGUI_GREY );       // "Grey"
	ColorCycle[i++] = (char*)GetString( MSG_MAPGUI_LIGHTGREY );  // "Light Grey"
	ColorCycle[i++] = (char*)GetString( MSG_MAPGUI_COLOR );      // "Color"
	ColorCycle[i++] = NULL;
}

 Set_Param_Menu(10);

 MP->MAPC = WindowObject,
   MUIA_Window_Title      , GetString( MSG_MAPGUI_MAPVIEWCONTROL ),  // "Map View Control"
   MUIA_Window_ID         , MakeID('M','A','P','C'),
   MUIA_Window_Screen    , WCSScrn,

   WindowContents, VGroup,
         Child, HGroup,
	   Child, VGroup,
             Child, MP->ScaleStuff = ColGroup(5), MUIA_VertWeight, 0, MUIA_Group_HorizSpacing, 0,

       /* Row 1 */
               Child, Label2(GetString( MSG_MAPGUI_SCALE )),  // "Scale "
               Child, MP->Scale = StringObject, StringFrame,
		MUIA_String_Contents, "       ",
        	MUIA_String_Accept, " 0123456789.",
		MUIA_FixWidthTxt, "012345", End,
               Child, MP->ScaleLess = ImageButtonWCS(MUII_ArrowLeft),
               Child, MP->ScaleMore = ImageButtonWCS(MUII_ArrowRight),
               Child, MP->MapZoom   = KeyButtonObject('z'), MUIA_Text_Contents, GetString( MSG_MAPGUI_33CZOOM ),  // "\33cZoom"
         	MUIA_HorizWeight, 0, End,

       /* Row 2 */
               Child, Label2(GetString( MSG_MAPGUI_LAT)),  // "Lat "
               Child, MP->Lat = StringObject, StringFrame,
        	MUIA_String_Contents, "      ",
        	MUIA_String_Accept, " -0123456789.",
		MUIA_FixWidthTxt, "012345", End,
               Child, MP->LatLess = ImageButtonWCS(MUII_ArrowLeft),
               Child, MP->LatMore = ImageButtonWCS(MUII_ArrowRight),
               Child, MP->MapPan    = KeyButtonObject('p'), MUIA_Text_Contents, GetString( MSG_MAPGUI_33CPAN ),  // "\33cPan"
        	MUIA_HorizWeight, 0, End,

       /* Row 3 */
               Child, Label2(GetString( MSG_MAPGUI_LON )),  // "Lon "
               Child, MP->Lon = StringObject, StringFrame,
        	MUIA_String_Contents, "       ",
         	MUIA_String_Accept, " -0123456789.",
		MUIA_FixWidthTxt, "012345", End,
               Child, MP->LonLess = ImageButtonWCS(MUII_ArrowLeft),
               Child, MP->LonMore = ImageButtonWCS(MUII_ArrowRight),
               Child, HGroup, MUIA_Group_HorizSpacing, 0, MUIA_HorizWeight, 0,
                 Child, MP->MapAuto   = KeyButtonFunc('a', (char*)GetString( MSG_MAPGUI_AUTO ) ),      // "\33cAuto"
                 Child, MP->MapCenter = KeyButtonFunc('e', (char*)GetString( MSG_MAPGUI_33CCENTER )),  // "\33cCenter"
                 End, /* HGroup */

               End, /* ColGroup */

       /* Row 4 */
             Child, ColGroup(5), MUIA_VertWeight, 0, MUIA_Group_HorizSpacing, 0,
               Child, Label2(GetString( MSG_MAPGUI_EXAG )),  // " Exag "
               Child, MP->Exag = StringObject, StringFrame,
        	MUIA_String_Contents, "       ",
        	MUIA_String_Accept, " 0123456789.",
		MUIA_FixWidthTxt, "012345", End,
               Child, MP->ExagLess = ImageButtonWCS(MUII_ArrowLeft),
               Child, MP->ExagMore = ImageButtonWCS(MUII_ArrowRight),
               Child, MP->MapObject = KeyButtonObject('b'),
			MUIA_Text_Contents, GetString( MSG_MAPGUI_33CDATABASE ),  // "\33c Database "
        		MUIA_HorizWeight, 0, End,
               End, /* ColGroup */
	     End, /* VGroup */

           Child, RectangleObject, MUIA_Rectangle_VBar, TRUE, MUIA_HorizWeight, 0, End,
           Child, VGroup,
             Child, HGroup, MUIA_VertWeight, 0,
               Child, HGroup,
                 Child, MP->MapTopo = CheckMark(0),
                 Child, Label1(GetString( MSG_MAPGUI_TOPO )),  // "Topo"
                 End, /* HGroup */
               Child, RectangleObject, End,
               Child, HGroup,
                 Child, MP->MapVec = CheckMark(0),
                 Child, Label1(GetString( MSG_MAPGUI_VEC )),  // "Vec"
                 End, /* HGroup */
               Child, RectangleObject, End,
               Child, HGroup, MUIA_Group_HorizSpacing, 0,
                 Child, MP->MapEco = CheckMark(0),
                 Child, MP->BT_EcoLegend = KeyButtonFunc(',', (char*)GetString( MSG_MAPGUI_ECO )),  // "\33cEco "
                 End, /* HGroup */
               Child, RectangleObject, End,
               Child, HGroup,
                 Child, MP->MapInter = CheckMark(0),
                 Child, Label1(GetString( MSG_MAPGUI_INTER )),  // "Inter"
                 End, /* HGroup */
               End, /* HGroup */

             Child, HGroup, MUIA_VertWeight, 0,
               Child, HGroup,
                 Child, Label2(GetString( MSG_MAPGUI_STYLE )),  // "Style:"
                 Child, MP->MapStyle = CycleObject, MUIA_Cycle_Entries, StyleCycle, End,
                 End, /* HGroup */
               Child, RectangleObject, End,
               Child, HGroup,
                 Child, MP->MapDither = CheckMark(0),
                 Child, Label2(GetString(MSG_MAPGUI_DITHER)),  // "Dither"
                 End, /* HGroup */
               End, /* HGroup */

             Child, HGroup, MUIA_VertWeight, 0,
	       Child, HGroup, MUIA_Group_HorizSpacing, 0,
                 Child, MP->AlignCheck = CheckMark(align),
                 Child, MP->MapAlign  = KeyButtonFunc('g', (char*)GetString( MSG_MAPGUI_ALIGN )),  // "\33cAlign "
	         End, /* HGroup */
               Child, MP->MapColor = CycleObject, MUIA_Cycle_Entries, ColorCycle, End,
               End, /* HGroup */

             Child, HGroup, MUIA_Group_SameWidth, TRUE, MUIA_VertWeight, 0,
               Child, MP->ClearFrame = HGroup, MUIA_Group_HorizSpacing, 0, /* Checkmark and Button */
                 Child, MP->MapAutoClear = CheckMark(0),
                 Child, MP->MapClr    = KeyButtonFunc('c', (char*)GetString( MSG_MAPGUI_CLEAR )),  // "\33cClear"
                 End, /* HGroup */
               Child, MP->MapDraw   = KeyButtonFunc('d', (char*)GetString( MSG_MAPGUI_33CDRAW )),  // "\33cDraw"
               Child, MP->MapRefine = KeyButtonFunc('r', (char*)GetString( MSG_MAPGUI_33CREFINE )),   // "\33cRefine"
               End, /* HGroup */
             End, /* VGroup */
           End, /* HGroup */

     Child, VGroup, ReadListFrame,
       Child, MP->MapMsg[0] = TextObject, End,
       Child, MP->MapMsg[1] = TextObject, End,
       End, /* VGroup */
     End, /* VGroup */
   End; /* Window object */
 
 if(MP->MAPC)
   {
   DoMethod(app, OM_ADDMEMBER, MP->MAPC);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   
   DoMethod(MP->MAPC, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_MC_QUIT);
   DoMethod(MP->MapAutoClear, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_AUTOCLEAR);
   DoMethod(MP->AlignCheck, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_ALIGNCHECK);

   DoMethod(MP->MapTopo, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_TOPO);
   DoMethod(MP->MapVec, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_VEC);
   DoMethod(MP->MapEco, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_ECO);
   DoMethod(MP->MapInter, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_INTER);
   DoMethod(MP->MapDither, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_DITHER);
   DoMethod(MP->MapStyle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_MAPSTYLE);
   DoMethod(MP->MapColor, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_MAPCOLOR);

   DoMethod(MP->Scale, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_SCALE);
   DoMethod(MP->Lat, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_LAT);
   DoMethod(MP->Lon, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_LON);
   DoMethod(MP->Exag, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
    app, 2, MUIM_Application_ReturnID, ID_MC_EXAG);

   MUI_DoNotiPresFal(app, MP->MapClr, ID_MC_CLEAR,
    MP->MapDraw, ID_MC_DRAW, MP->MapRefine, ID_MC_REFINE,
    MP->MapObject, ID_DE_WINDOW, MP->MapZoom, ID_MC_ZOOM,
    MP->MapPan, ID_MC_PAN, MP->MapCenter, ID_MC_CENTER,
    MP->MapAuto, ID_MC_AUTO, MP->ScaleLess, ID_MC_SCALELESS,
    MP->ScaleMore, ID_MC_SCALEMORE, MP->LatLess, ID_MC_LATLESS,
    MP->LatMore, ID_MC_LATMORE, MP->LonLess, ID_MC_LONLESS,
    MP->LonMore, ID_MC_LONMORE, MP->ExagLess, ID_MC_EXAGLESS,
    MP->MapAlign, ID_MC_MAPALIGN, MP->ExagMore, ID_MC_EXAGMORE,
    MP->BT_EcoLegend, ID_EL_WINDOW, NULL);

/*   set(MP->MapEco, MUIA_Disabled, TRUE);*/
   
   DoMethod(MP->MAPC, MUIM_Window_SetCycleChain,
    MP->MapAuto, MP->MapCenter, MP->MapObject, MP->MapTopo, MP->MapVec,
    MP->MapEco, MP->BT_EcoLegend, MP->MapInter, MP->AlignCheck, MP->MapAlign,
    MP->MapColor, MP->MapStyle, MP->MapDither, MP->MapAutoClear, MP->MapClr,
    MP->MapDraw, MP->MapRefine, MP->Scale, MP->ScaleLess, MP->ScaleMore,
    MP->Lat, MP->LatLess, MP->LatMore, MP->Lon, MP->LonLess, MP->LonMore,
    MP->Exag, MP->ExagLess, MP->ExagMore, MP->MapZoom,
    MP->MapPan, NULL);


   set(MP->MAPC, MUIA_Window_ActiveObject, (IPTR)MP->MapAuto);
   
   set(MP->MAPC, MUIA_Window_Open, TRUE);
   get(MP->MAPC, MUIA_Window_Open, &open);
   if (! open)
    return (0);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Get Window structure pointer */
   get(MP->MAPC, MUIA_Window_Window, &MP->MAPCWin);

   MapGUI_Update(MP);

   /* Do stuff <<<>>> */
   } /* if */
 else
   {
   return(0); /* failed */
   } /* else */

return(1); /* success */
} /* MapGUI_New() */

/*************************************************************************/

void MapGUI_Update(struct MapData *MP)
{
char StringTrans[20];

set(MP->MapTopo, MUIA_Selected, topo);
set(MP->AlignCheck, MUIA_Selected, align);
set(MP->MapVec, MUIA_Selected, vectorenabled);
set(MP->MapEco, MUIA_Selected, ecoenabled);
set(MP->MapDither, MUIA_Selected, MapDither);
set(MP->MapInter, MUIA_Selected, InterStuff);
set(MP->MapStyle, MUIA_Cycle_Active, ContInt);
set(MP->MapAutoClear, MUIA_Selected, AutoClear);
	
if(mapscale == 0)
	{
	mapscale = 100;
	} /* if */
	
setfloat(MP->Scale, mapscale);
setfloat(MP->Lat, maplat);
setfloat(MP->Lon, maplon);
if(ContInt == 1 || ContInt == 5)
 sprintf(StringTrans, "%d", ContInterval);
if(ContInt == 3 || ContInt == 4)
 sprintf(StringTrans, "%f", MaxElevDiff);

if(ContInt == 0 || ContInt == 2)
 StringTrans[0] = 0;
if(ContInt == 3 || ContInt == 4)
/*else, what about 1*/
 TrimZeros(StringTrans);

set(MP->Exag, MUIA_String_Contents, (IPTR)StringTrans);
set(MP->Exag, MUIA_String_DisplayPos, 0);
set(MP->Exag, MUIA_String_BufferPos, 0);

if(!topo)
 {
 set(MP->Exag, MUIA_Disabled, TRUE);
 set(MP->ExagLess, MUIA_Disabled, TRUE);
 set(MP->ExagMore, MUIA_Disabled, TRUE);
 set(MP->MapStyle, MUIA_Disabled, TRUE);
 set(MP->MapDither, MUIA_Disabled, TRUE);
 } /* if */
else
 {
 set(MP->MapStyle, MUIA_Disabled, FALSE);
 set(MP->MapDither, MUIA_Disabled, FALSE);
 if(ContInt == 0 || ContInt == 2)
  {
  set(MP->Exag, MUIA_Disabled, TRUE);
  set(MP->ExagLess, MUIA_Disabled, TRUE);
  set(MP->ExagMore, MUIA_Disabled, TRUE);
  } /* if */
 else
  {
  set(MP->Exag, MUIA_Disabled, FALSE);
  set(MP->ExagLess, MUIA_Disabled, FALSE);
  set(MP->ExagMore, MUIA_Disabled, FALSE);
  } /* else */
 } /* else */

return;
} /* MapGUI_Update() */

/**********************************************************************/

void MapGUI_Message(short line, char *Message)
{
if(MP)
	{
	set(MP->MapMsg[line], MUIA_Text_Contents, (IPTR)Message);
	} /* if */

return;
} /* MapGUI_Message() */

/***********************************************************************/

STATIC_FCN void MapGUI_Del(struct MapData *MP) // used locally only -> static, AF 20.7.2021
{
if(MP->MAPC)
	{
   set(MP->MAPC, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
	DoMethod(app, OM_REMMEMBER, MP->MAPC);
   MUI_DisposeObject(MP->MAPC);
	MP->MAPC = NULL;
	} /* if */
} /* MapGUI_Del() */

/***********************************************************************/

short Make_MA_Window(struct MapData *MP)
{
 short i;
 long open;

 if (MP->AlignWin)
  {
  DoMethod(MP->AlignWin, MUIM_Window_ToFront);
  set(MP->AlignWin, MUIA_Window_Activate, TRUE);
  return (1);
  } /* if window already exists */

 Set_Param_Menu(10);

 MP->AlignWin = WindowObject,
   MUIA_Window_Title      , GetString( MSG_MAPGUI_MAPALIGNMENT ),  // "Map Alignment"
   MUIA_Window_ID         , MakeID('M','A','A','L'),
   MUIA_Window_Screen    , WCSScrn,
   MUIA_Window_Menu	, MapNewMenus,

   WindowContents, VGroup,
	 Child, TextObject, MUIA_Text_Contents, GetString( MSG_MAPGUI_EOGRAPHICCOORDS ), End,  // "\33c\0334Geographic Coords"
         Child, HGroup,

           Child, Label2(GetString( MSG_MAPGUI_NWLAT )),  // "NW Lat"
           Child, MP->FloatStr[0] = StringObject, StringFrame,
        	MUIA_String_Accept, "-.0123456789",
		MUIA_FixWidthTxt, "012345789012", End,
           Child, Label2(GetString( MSG_MAPGUI_LON )),  // " Lon"
           Child, MP->FloatStr[1] = StringObject, StringFrame,
        	MUIA_String_Accept, "-.0123456789",
		MUIA_FixWidthTxt, "012345789012", End,
           End, /* HGroup */

         Child, HGroup,

           Child, Label2(GetString( MSG_MAPGUI_SELAT )),  // "SE Lat"
           Child, MP->FloatStr[2] = StringObject, StringFrame,
        	MUIA_String_Accept, "-.0123456789",
		MUIA_FixWidthTxt, "012345789012", End,
           Child, Label2(GetString( MSG_MAPGUI_LON )),  // " Lon"
           Child, MP->FloatStr[3] = StringObject, StringFrame,
        	MUIA_String_Accept, "-.0123456789",
		MUIA_FixWidthTxt, "012345789012", End,
           End, /* HGroup */

	 Child, RectangleObject, MUIA_Rectangle_HBar, TRUE, End,

	 Child, TextObject, MUIA_Text_Contents, GetString( MSG_MAPGUI_CREENCOORDS ), End,  // "\33c\0334Screen Coords"
         Child, HGroup,

           Child, Label2(GetString( MSG_MAPGUI_UPPERLEFTX )),  // " Upper Left X"
           Child, MP->IntStr[0] = StringObject, StringFrame,
        	MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
           Child, Label2("   Y"),
           Child, MP->IntStr[1] = StringObject, StringFrame,
        	MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
           End, /* HGroup */

         Child, HGroup,

           Child, Label2(GetString( MSG_MAPGUI_LOWERRIGHTX )),  // "Lower Right X"
           Child, MP->IntStr[2] = StringObject, StringFrame,
        	MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
           Child, Label2("   Y"),
           Child, MP->IntStr[3] = StringObject, StringFrame,
        	MUIA_String_Accept, "0123456789",
		MUIA_FixWidthTxt, "012345", End,
           End, /* HGroup */

         Child, MP->Register = KeyButtonFunc('r', (char*)GetString( MSG_MAPGUI_SETREGISTRATION )),  // "\33cSet Registration"

     End, /* VGroup */
   End; /* Window object */
 
 if(! MP->AlignWin)
  {
  Close_MA_Window(MP);
  return (0);
  } /* if fail */

 DoMethod(app, OM_ADDMEMBER, MP->AlignWin);
   
 DoMethod(MP->AlignWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_MA_CLOSE);

 DoMethod(MP->Register, MUIM_Notify, MUIA_Pressed, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_MC_REGISTER);

 setfloat(MP->FloatStr[0], rlat[0]);
 setfloat(MP->FloatStr[1], rlon[0]);
 setfloat(MP->FloatStr[2], rlat[1]);
 setfloat(MP->FloatStr[3], rlon[1]);
 set(MP->IntStr[0], MUIA_String_Integer, AlignBox.Low.X);
 set(MP->IntStr[1], MUIA_String_Integer, AlignBox.Low.Y);
 set(MP->IntStr[2], MUIA_String_Integer, AlignBox.High.X);
 set(MP->IntStr[3], MUIA_String_Integer, AlignBox.High.Y);

 for (i=0; i<4; i++)
  {
  DoMethod(MP->FloatStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_MC_FLOATSTR(i));
  DoMethod(MP->IntStr[i], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
     app, 2, MUIM_Application_ReturnID, ID_MC_INTSTR(i));
  }

 DoMethod(MP->AlignWin, MUIM_Window_SetCycleChain,
    MP->FloatStr[0], MP->FloatStr[1], MP->FloatStr[2], MP->FloatStr[3],
    MP->IntStr[0], MP->IntStr[1], MP->IntStr[2], MP->IntStr[3],
    MP->Register, NULL);

 DoMethod(MP->FloatStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->FloatStr[1]);
 DoMethod(MP->FloatStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->FloatStr[2]);
 DoMethod(MP->FloatStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->FloatStr[3]);
 DoMethod(MP->FloatStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->IntStr[0]);
 DoMethod(MP->IntStr[0], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->IntStr[1]);
 DoMethod(MP->IntStr[1], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->IntStr[2]);
 DoMethod(MP->IntStr[2], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->IntStr[3]);
 DoMethod(MP->IntStr[3], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
   MP->AlignWin, 3, MUIM_Set, MUIA_Window_ActiveObject, MP->FloatStr[0]);

 set(MP->AlignWin, MUIA_Window_ActiveObject, (IPTR)MP->FloatStr[0]);
   
 set(MP->AlignWin, MUIA_Window_Open, TRUE);
 get(MP->AlignWin, MUIA_Window_Open, &open);
 if (! open)
  {
  Close_MA_Window(MP);
  return (0);
  }

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

 return (1);

} /* Make_MA_Window() */

/*************************************************************************/

STATIC_FCN void Close_MA_Window(struct MapData *MP) // used locally only -> static, AF 20.7.2021
{

if (MP->AlignWin)
   {
   set(MP->AlignWin, MUIA_Window_Open, FALSE);

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

   DoMethod(app, OM_REMMEMBER, MP->AlignWin);
   MUI_DisposeObject(MP->AlignWin);
   MP->AlignWin = NULL;
   } /* if */

} /* Close_MA_Window() */

/*************************************************************************/

void TrimZeros(char *String)
{
int i;

for(i = strlen(String) - 1; i > -1; i--)
	{
	switch (String[i])
		{
		case '.':
			{
			String[i] = 0;
			i = -1; /* end loop */
			break;
			} /* '.' */
		case '0':
			{
			String[i] = 0;
			break;
			} /* '0' */
		default:
			{
			i = -1; /* end loop */
			break;
			} /* default */
		} /* switch */
	} /* for */

return;
} /* TrimZeros */

/************************************************************************/

#ifdef UNDER_CONST

void UnderConst_New(void)
{

if(!UnderConst)
	{
	UnderConst = WindowObject,
   MUIA_Window_Title      , GetString( MSG_MAPGUI_MAPCONTROLWINDOWNOTICE ),  // "Map Control Window Notice"
   MUIA_Window_ID         , MakeID('N','O','T','C'),
/*   MUIA_Window_SizeGadget  , FALSE, */
   MUIA_Window_Screen    , WCSScrn,
   WindowContents, VGroup,
	 Child, ImageObject, MUIA_Frame, MUIV_Frame_Text,
	  MUIA_Image_OldImage, &DangerSign, MUIA_Weight, 1,
	  MUIA_InnerRight, 0, MUIA_InnerLeft, 0,
	  MUIA_InnerTop, 0, MUIA_InnerBottom, 0, End,
	 Child, RectangleObject, End,
	 Child, TextObject, MUIA_Text_Contents, GetString( MSG_MAPGUI_THISAREAISNDERCONSTRUCTIONARDHATREQUIRED ), End,  // "\33cThis area is\nUnder Construction.\nHard hat required."
	 Child, RectangleObject, End,
	 Child, UnderConstOK = KeyButtonFunc('o', (char*)GetString( MSG_MAPGUI_OKAY )),  // "\33cOkay"
    End,
   End;
   
   if(UnderConst)
   	{
  	   DoMethod(app, OM_ADDMEMBER, UnderConst);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   
	   DoMethod(UnderConst, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
   	 app, 2, MUIM_Application_ReturnID, ID_UNDERCONST);

	   MUI_DoNotiPresFal(app, UnderConstOK, ID_UNDERCONST, NULL);

	   set(UnderConst, MUIA_Window_ActiveObject, (IPTR)UnderConstOK);
      set(UnderConst, MUIA_Window_Open, TRUE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

   	} /* if */
   
	} /* if */

} /* UnderConst_New() */

/***********************************************************************/

void UnderConst_Del(void)
{

if(UnderConst)
	{
   set(UnderConst, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, UnderConst);
   MUI_DisposeObject(UnderConst);
   UnderConst = NULL;
	} /* if */

} /* UnderConst_Del() */

#endif /* UNDER_CONST */

/*********************************************************************/


short map(void)
{
 short PrefsLoaded = 0;
 ULONG iflags;
 struct clipbounds cb;
 
 if (MapWind0)
  {
  WindowToFront(MapWind0);
  return(0);
  } /* if map already open */

#ifdef MED_HACK
 MaxElevDiff = 0;
#endif
 if (MP_Width + MP_Left > WCSScrn->Width) MP_Left = 0;
 if (MP_Height + MP_Top > WCSScrn->Height) MP_Top = 0;
 if (MP_Width + MP_Left > WCSScrn->Width) MP_Width = 0;
 if (MP_Height + MP_Top > WCSScrn->Height) MP_Height = 0;
 if (! MP_Width || ! MP_Height)
  {
  MP_Width = WCSScrn->Width * .75;
  MP_Height = WCSScrn->Height * .75;
  MP_Left = (WCSScrn->Width - MP_Width) / 2;
  MP_Top = (WCSScrn->Height - MP_Height) / 2;
  } /* if no width or height previously specified */
 iflags = IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW
	| IDCMP_ACTIVEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY
	| IDCMP_VANILLAKEY
#ifdef USE_WCS_HELP
#ifdef USE_MAP_HELP
 | IDCMP_MENUHELP
#endif /* USE_MAP_HELP */
#endif /* USE_WCS_HELP */
  ;

 MapWind0 = OpenWindowTags(NULL,
	WA_Left,	MP_Left,
	WA_Top,		MP_Top,
	WA_Width,	MP_Width,
	WA_Height,	MP_Height,
	WA_DragBar,	TRUE,
	WA_CloseGadget,	TRUE,
	WA_SmartRefresh,TRUE,
	WA_SizeGadget,	TRUE,
	WA_SizeBBottom,	TRUE,
	WA_DepthGadget,	TRUE,
	WA_ReportMouse,	TRUE,
	WA_AutoAdjust,	TRUE,
#ifdef USE_WCS_HELP
#ifdef USE_MAP_HELP
	WA_MenuHelp,	TRUE,
#endif /* USE_MAP_HELP */
#endif /* USE_WCS_HELP */
	WA_Activate,	TRUE,
	WA_MinWidth,	50,
	WA_MinHeight,	30,
	WA_MaxWidth,	(~0),
	WA_MaxHeight,	(~0),
	WA_IDCMP,	iflags,
	WA_Title,	(IPTR)GetString( MSG_MAPGUI_MAPVIEW ),  // "Map View"
	WA_CustomScreen,(IPTR)WCSScrn,
	TAG_DONE);

 if (! MapWind0)
  {
  Log(ERR_WIN_FAIL, GetString( MSG_MAPGUI_MAPPINGMODULE ));  // "Mapping module"
  return (0);
  }

 MapWind0_Sig = 1L << MapWind0->UserPort->mp_SigBit;
 WCS_Signals |= MapWind0_Sig;

 setclipbounds(MapWind0, &cb);
 backpen = 1;
 SetAPen(MapWind0->RPort, backpen);
 RectFill(MapWind0->RPort, cb.lowx, cb.lowy, cb.highx, cb.highy);
 SetAPen(MapWind0->RPort,2);

 if ((MP = (struct MapData *)
	get_Memory(sizeof (struct MapData), MEMF_CLEAR)) == NULL)
  {
  User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                           // "Mapping Module"
		  GetString( MSG_MAPGUI_OUTOFMEMORYANTINITIALIZEMAPWINDOWPERATIONTERMINATED ),  // "Out of memory!\nCan't initialize map window!\nOperation terminated."
		  GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");                               // "OK"
  Close_Map_Window(1);
  return(0);
  }

 if (! Make_Map_Menu())
  {
  Close_Map_Window(1);
  return (0);
  }
 /*
 OBN = 0;
*/
 MapGUI_New(MP);

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

 vectorenabled = 1;
 InterStuff = 1;
 AutoClear = 1;

 if (! dbaseloaded)
 {
  Database_LoadDisp(0, 1, NULL, 0);
 }

 if (dbaseloaded)
 {
  PrefsLoaded = loadmapbase(0, 1);
 }

 valueset();

 if (PrefsLoaded)
 {
  Handle_Map_Window(ID_MC_DRAW);
 }
 else
 {
  Handle_Map_Window(ID_MC_AUTO);
 }

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

 return (1);

} /* map() */

/************************************************************************/

void Handle_Map_Window(ULONG WCS_ID)
{
short abort = 0, InternalEvent = 0, FindObject = 0, error;
struct clipbounds Clip;
LONG CheckState;
int FieldUpdate = 0, ReDraw = 0;
char *FloatStr;

if (! MP || ! MP->MAPC || ! MapWind0)
 return;

if(WCS_ID)
 {
 switch(WCS_ID)
  {
  case ID_MC_QUIT:
   {
   abort = 1;
   break;
   } /* case 0 */
  case ID_MC_AUTOCLEAR:
   {
   get(MP->MapAutoClear, MUIA_Selected, &CheckState);
   AutoClear = CheckState;
   break;
   } /* ID_MC_AUTOCLEAR */
  case ID_MC_CLEAR:
   {
   ClearWindow(MapWind0, backpen);
   MP->ptsdrawn = 0;
   break;
   } /* ID_MC_CLEAR */
  case ID_MC_DRAW:
   {
   ReDraw = 1;
   break;
   } /* ID_MC_DRAW */
  case ID_MC_REFINE:
   {
   struct Box Bx;

   if (GetBounds(&Bx))
	{
	makemap(MapWind0, (long)Bx.Low.X, (long)Bx.Low.Y, (long)Bx.High.X, (long)Bx.High.Y,
	 (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0) | MMF_REFINE);
	} /* if */
   break;
   } /* ID_MC_REFINE */
  case ID_MC_TOPO:
   {
   get(MP->MapTopo, MUIA_Selected, &CheckState);
   topo = CheckState;
   break;
   } /* ID_MC_TOPO */
  case ID_MC_VEC:
   {
   get(MP->MapVec, MUIA_Selected, &CheckState);
   vectorenabled = CheckState;
   break;
   } /* ID_MC_VEC */
  case ID_MC_ECO:
   {
   get(MP->MapEco, MUIA_Selected, &CheckState);
   ecoenabled = CheckState;
   break;
   } /* ID_MC_ECO */
  case ID_MC_INTER:
   {
   get(MP->MapInter, MUIA_Selected, &CheckState);
   InterStuff = CheckState;
   break;
   } /* ID_MC_INTER */
  case ID_MC_DITHER:
   {
   get(MP->MapDither, MUIA_Selected, &CheckState);
   MapDither = CheckState;
   break;
   } /* ID_MC_DITHER */
  case ID_MC_MAPSTYLE:
   {
   get(MP->MapStyle, MUIA_Cycle_Active, &CheckState);
   ContInt = CheckState;
   FieldUpdate = 1;
   break;
   } /* ID_MC_MAPSTYLE */
  case ID_MC_MAPCOLOR:
   {
   get(MP->MapColor, MUIA_Cycle_Active, &CheckState);
   if (CheckState == 2)
    {
    LoadRGB4(&WCSScrn->ViewPort, &PrimaryColors[0], 16);
    }
   else if (CheckState == 1)
    {
    LoadRGB4(&WCSScrn->ViewPort, &PrintColors[0], 16);
    }
   else
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    }
   break;
   } /* ID_MC_MAPCOLOR */
  case ID_MC_SCALE:
   {
   get(MP->Scale, MUIA_String_Contents, &FloatStr);
   mapscale = atof(FloatStr);
   FieldUpdate = 1;
   break;
   } /* ID_MC_SCALE */
  case ID_MC_SCALELESS:
   {
   mapscale /= 2.0;
   FieldUpdate = 1;
   break;
   } /* ID_MC_SCALELESS */
  case ID_MC_SCALEMORE:
   {
   mapscale *= 2.0;
   FieldUpdate = 1;
   break;
   } /* ID_MC_SCALEMORE */
  case ID_MC_LAT:
   {
   get(MP->Lat, MUIA_String_Contents, &FloatStr);
   maplat = atof(FloatStr);
   FieldUpdate = 1;
   break;
   } /* ID_MC_LAT */
  case ID_MC_LATLESS:
   {
   maplat -= ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop - MapWind0->BorderBottom) / 2));
   FieldUpdate = 1;
   break;
   } /* ID_MC_LATLESS */
  case ID_MC_LATMORE:
   {
   maplat += ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop - MapWind0->BorderBottom) / 2));
   FieldUpdate = 1;
   break;
   } /* ID_MC_LATMORE */
  case ID_MC_LON:
   {
   get(MP->Lon, MUIA_String_Contents, &FloatStr);
   maplon = atof(FloatStr);
   FieldUpdate = 1;
   break;
   } /* ID_MC_LON */
  case ID_MC_LONLESS:
   {
   maplon -= ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft - MapWind0->BorderRight) / 2));
   FieldUpdate = 1;
   break;
   } /* ID_MC_LONLESS */
  case ID_MC_LONMORE:
   {
   maplon += ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft - MapWind0->BorderRight) / 2));
   FieldUpdate = 1;
   break;
   } /* ID_MC_LONMORE */
  case ID_MC_EXAG:
   {
   get(MP->Exag, MUIA_String_Contents, &FloatStr);
   if(ContInt == 3 || ContInt == 4)
    MaxElevDiff = atof(FloatStr);
   if(ContInt == 1 || ContInt == 5)
    {
    int DecimalMustDie;

/* Note Chris: I think this is a questionable practice-changing the string
**  directly without copying it first. At least Stefan says you shouldn't.
**  We'll keep this in mind if some strange bug appears. <<<>>> */
    for(DecimalMustDie = 0; DecimalMustDie < strlen(FloatStr); DecimalMustDie++)
     {
     if(FloatStr[DecimalMustDie] == '.')
      FloatStr[DecimalMustDie] = 0;
     } /* for */

    ContInterval = atoi(FloatStr);
    }
   FieldUpdate = 1;
   break;
   } /* ID_MC_EXAG */
  case ID_MC_EXAGLESS:
   {
   if(ContInt == 3 || ContInt == 4)
    MaxElevDiff /= 2;
   if(ContInt == 1 || ContInt == 5)
    ContInterval /= 2;
   FieldUpdate = 1;
   break;
   } /* ID_MC_EXAGLESS */
  case ID_MC_EXAGMORE:
   {
   if(ContInt == 3 || ContInt == 4)
    MaxElevDiff *= 2;
   if(ContInt == 1 || ContInt == 5)
    ContInterval *= 2;
   FieldUpdate = 1;
   break;
   } /* ID_MC_EXAGMORE */
  case ID_MC_PAN:
   {
   if (mapscale==0.0) break;
   if (shiftmap(0, 0, 0))
    {
    MP->ptsdrawn = 0;
    FieldUpdate = 1;
    ReDraw = 1;
    } /* if */
   break;
   } /* ID_MC_PAN */
  case ID_MC_CENTER:
   {
   if (mapscale==0.0) break;
   if (shiftmap(1, 0, 0))
    {
    MP->ptsdrawn = 0;
    FieldUpdate = 1;
    ReDraw = 1;
    } /* if */
   break;
   } /* ID_MC_CENTER */
  case ID_MC_ZOOM:
   {
   struct Box Bx;

   if (GetBounds(&Bx))
    {
    short HalfHeight, HalfWidth;
    double lonscale, scale1, scale2;

    HalfHeight = (MapWind0->BorderTop + MapWind0->Height
	 - MapWind0->BorderBottom) / 2;
    HalfWidth = (MapWind0->BorderLeft + MapWind0->Width
	 - MapWind0->BorderRight) / 2;
    maplon = X_Lon_Convert((Bx.Low.X + Bx.High.X) / 2);
    maplat = Y_Lat_Convert((Bx.Low.Y + Bx.High.Y) / 2);
    lonzero = X_Lon_Convert((long)Bx.Low.X);
    latzero = Y_Lat_Convert((long)Bx.Low.Y);
    lonscale = LATSCALE * cos(maplat * PiOver180);
    scale1 = lonscale * HORPIX * (lonzero - maplon) / HalfWidth;
    scale2 = LATSCALE * VERTPIX * (latzero - maplat) / HalfHeight;
    mapscale = max(scale1, scale2);
    valueset();
    lonzero += MapWind0->BorderLeft * x_lon /lonscalefactor;
    latzero += MapWind0->BorderTop * y_lat /latscalefactor;
    scale1 = lonscale * HORPIX * (lonzero - maplon) / HalfWidth;
    scale2 = LATSCALE * VERTPIX * (latzero - maplat) / HalfHeight;
    mapscale = max(scale1, scale2);
    FieldUpdate = 1;
    ReDraw = 1;
    } /* if */
   break;
   } /* ID_MC_ZOOM */
  case ID_MC_AUTO:
   {
   FindCenter(&maplat, &maplon);
   ClearWindow(MapWind0, backpen);
   FieldUpdate = 1;
   ReDraw = 1;
   break;
   } /* ID_MC_AUTO */
  case ID_MC_FLOATSTR(0):
   {
   char *floatstr;

   get(MP->FloatStr[0], MUIA_String_Contents, &floatstr);
   rlat[0] = atof(floatstr);
   break;
   } /* ID_MC_FLOATSTR(0) */
  case ID_MC_FLOATSTR(1):
   {
   char *floatstr;

   get(MP->FloatStr[1], MUIA_String_Contents, &floatstr);
   rlon[0] = atof(floatstr);
   break;
   } /* ID_MC_FLOATSTR(1) */
  case ID_MC_FLOATSTR(2):
   {
   char *floatstr;

   get(MP->FloatStr[2], MUIA_String_Contents, &floatstr);
   rlat[1] = atof(floatstr);
   break;
   } /* ID_MC_FLOATSTR(2) */
  case ID_MC_FLOATSTR(3):
   {
   char *floatstr;

   get(MP->FloatStr[3], MUIA_String_Contents, &floatstr);
   rlon[1] = atof(floatstr);
   break;
   } /* ID_MC_FLOATSTR(3) */
  case ID_MC_INTSTR(0):
   {
   long data;

   get(MP->IntStr[0], MUIA_String_Integer, &data);
   AlignBox.Low.X = data;
   break;
   } /* ID_MC_INTSTR(0) */
  case ID_MC_INTSTR(1):
   {
   long data;

   get(MP->IntStr[1], MUIA_String_Integer, &data);
   AlignBox.Low.Y = data;
   break;
   } /* ID_MC_INTSTR(1) */
  case ID_MC_INTSTR(2):
   {
   long data;

   get(MP->IntStr[2], MUIA_String_Integer, &data);
   AlignBox.High.X = data;
   break;
   } /* ID_MC_INTSTR(2) */
  case ID_MC_INTSTR(3):
   {
   long data;

   get(MP->IntStr[3], MUIA_String_Integer, &data);
   AlignBox.High.Y = data;
   break;
   } /* ID_MC_INTSTR(3) */
  case ID_MC_REGISTER:
   {
   alignmap(&AlignBox);
   set(MP->IntStr[0], MUIA_String_Integer, AlignBox.Low.X);
   set(MP->IntStr[1], MUIA_String_Integer, AlignBox.Low.Y);
   set(MP->IntStr[2], MUIA_String_Integer, AlignBox.High.X);
   set(MP->IntStr[3], MUIA_String_Integer, AlignBox.High.Y);
   break;
   } /* ID_MC_REGISTER */
  case ID_MC_MAPALIGN:
   {
   Make_MA_Window(MP);
   break;
   } /* ID_MC_MAPALIGN */
  case ID_MC_ALIGNCHECK:
   {
   long State;

   get(MP->AlignCheck, MUIA_Selected, &State);
   align = State;
   set(MP->ScaleStuff, MUIA_Disabled, State);
   break;
   } /* ID_MC_REGISTER */
  case ID_MA_CLOSE:
   {
   Close_MA_Window(MP);
   break;
   }
  default:
   {
   return;
   break;
   } /* default */


  } /* switch WCS_ID */

 if (abort)
  {
  Close_Map_Window(0);
  return;
  } /* if time to quit map */

 if(FieldUpdate)
 	{ /* check and update all fields */
 	if(mapscale <= 0)
 		mapscale = 100;
 	if(maplat < -89.999)
 		maplat = -89.999;
 	if(maplat > 89.999)
 		maplat = 89.999;
/* 	if(maplon < 0)
 		maplon = 0;
 	if(maplon > 360)
 		maplon = 360;*/
 	if(ContInterval > 6144)
 		ContInterval = 12288;
 	if(ContInterval < 6)
 		ContInterval = 3;
 	} /* if */
 	
 MapGUI_Update(MP);

 if(ReDraw)
	{
        valueset();
        if (align)
         {
         if  ((AlignBox.High.Y != AlignBox.Low.Y) && (AlignBox.High.X - AlignBox.Low.Y))
          valuesetalign();
         else
          User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),  // "Mapping Module: Align"
                       GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal."
                       GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");  // "OK"
         } /* if align */
	if(AutoClear)
		{
		ClearWindow(MapWind0, backpen);
		MP->ptsdrawn = 0;
		} /* if */
	setclipbounds(MapWind0, &Clip);
	makemap(MapWind0, Clip.lowx, Clip.lowy, Clip.highx, Clip.highy,
	 (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0));
	if(InterStuff)
   		{
	   	ShowView_Map(&Clip);
		ShowPaths_Map(&Clip);
	   	} /* if */
	MapGUI_Update(MP); /* In case Exag changed as a result of loading Topos */
	} /* if */ 

 } /* if (WCS_ID) */

while (QuickFetchEvent(MapWind0, &Event))
 {
 FieldUpdate = ReDraw = 0;


HandleEvent:
 
 switch (Event.Class)
  {
  case IDCMP_CLOSEWINDOW:
   {
   abort = 1;
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   break;
   } /* CLOSEWINDOW */

#ifdef USE_WCS_HELP
#ifdef USE_MAP_HELP
  case IDCMP_MENUHELP:
   {
   struct MenuItem *Item;
   ULONG ReturnID;

   if (Event.Code == MENUNULL)
    break;
   Item = ItemAddress(MP->MenuStrip, Event.Code);
   ReturnID = (ULONG)(GTMENUITEM_USERDATA(Item));

   Help_Map_Window(ReturnID);
   break;
   } /* MENUHELP */
#endif /* USE_MAP_HELP */
#endif /* USE_WCS_HELP */

  case IDCMP_INTUITICKS:
   {
   LatLonElevScan(&Event, NULL, 0);
   break;
   } /* IDCMP_INTUITICKS */

  case IDCMP_ACTIVEWINDOW:
   {
/*   WindowToFront(MapWind0);*/
   get(MP->MapColor, MUIA_Cycle_Active, &CheckState);
   if (CheckState == 2)
    {
    LoadRGB4(&WCSScrn->ViewPort, &PrimaryColors[0], 16);
    }
   else if (CheckState == 1)
    {
    LoadRGB4(&WCSScrn->ViewPort, &PrintColors[0], 16);
    }
   else
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    }
   ModifyIDCMP(MapWind0, MapWind0->IDCMPFlags | IDCMP_INTUITICKS);
   break;
   } /* ACTIVEWINDOW */
  
  case IDCMP_INACTIVEWINDOW:
   {
   ModifyIDCMP(MapWind0, MapWind0->IDCMPFlags & (~IDCMP_INTUITICKS));
   break;
   } /* INACTIVEWINDOW */

  case IDCMP_REFRESHWINDOW:
   {
   struct clipbounds cb;

   setclipbounds(MapWind0, &cb);
   BeginRefresh(MapWind0);
   SetAPen(MapWind0->RPort, backpen);
   RectFill(MapWind0->RPort, cb.lowx, cb.lowy, cb.highx, cb.highy);
   SetAPen(MapWind0->RPort, 2);
   EndRefresh(MapWind0, TRUE);
   break;
   } /* REFRESHWINDOW */


  case IDCMP_MENUPICK:
   {
   struct MenuItem *Item;
   ULONG ReturnID;
   UWORD MenuNumber = Event.Code;

   while (MenuNumber != MENUNULL)
    {
    FieldUpdate = ReDraw = 0;
    Item = ItemAddress(MP->MenuStrip, MenuNumber);
    ReturnID =(IPTR)(GTMENUITEM_USERDATA(Item));

    switch (ReturnID)
     {
     case ID_MC_QUIT:
      {
      abort = 1;
      break;
      } /* case 0 */
     case ID_MC_CTRLTOFR:
      {
      DoMethod(MP->MAPC, MUIM_Window_ToFront);
      break;
      } /* ID_MC_CLEAR */
     case ID_DE_WINDOW:
      {
      Make_DE_Window();
      break;
      } /* ID_MC_CLEAR */
     case ID_EL_WINDOW:
      {
      Make_EL_Window();
      break;
      } /* ID_MC_CLEAR */
     case ID_MC_CLEAR:
      {
      ClearWindow(MapWind0, backpen);
      MP->ptsdrawn = 0;
      break;
      } /* ID_MC_CLEAR */
     case ID_MC_DRAW:
      {
      ReDraw = 1;
      break;
      } /* ID_MC_DRAW */
     case ID_MC_REFINE:
      {
      struct Box Bx;

      if (GetBounds(&Bx))
	{
	makemap(MapWind0, (long)Bx.Low.X, (long)Bx.Low.Y, (long)Bx.High.X, (long)Bx.High.Y,
	 (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0) | MMF_REFINE);
	} /* if */
      break;
      } /* ID_MC_REFINE */
     case ID_MC_PAN:
      {
      if (mapscale==0.0) break;
      if (shiftmap(0, 0, 0))
       {
       MP->ptsdrawn = 0;
       FieldUpdate = 1;
       ReDraw = 1;
       } /* if */
      break;
      } /* ID_MC_PAN */
     case ID_MC_ZOOMIN:
      {
      mapscale /= 2.0;
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_ZOOMIN */
     case ID_MC_ZOOMOUT:
      {
      mapscale *= 2.0;
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_ZOOMOUT */
     case ID_MC_MOVERIGHT:
      {
      maplon += ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft - MapWind0->BorderRight) / 2));
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_MOVERIGHT */
     case ID_MC_MOVELEFT:
      {
      maplon -= ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft - MapWind0->BorderRight) / 2));
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_MOVELEFT */
     case ID_MC_MOVEDOWN:
      {
      maplat += ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop - MapWind0->BorderBottom) / 2));
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_MOVEDOWN */
     case ID_MC_MOVEUP:
      {
      maplat -= ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop - MapWind0->BorderBottom) / 2));
      ReDraw = FieldUpdate = 1;
      break;
      } /* ID_MC_MOVEUP */
     case ID_MC_CENTER:
      {
      if (mapscale==0.0) break;
      if (shiftmap(1, 0, 0))
       {
       MP->ptsdrawn = 0;
       FieldUpdate = 1;
       ReDraw = 1;
       } /* if */
      break;
      } /* ID_MC_CENTER */
     case ID_MC_ZOOM:
      {
      struct Box Bx;

      if (GetBounds(&Bx))
	{
        short HalfHeight, HalfWidth;
        double lonscale, scale1, scale2;

        HalfHeight = (MapWind0->BorderTop + MapWind0->Height
	 - MapWind0->BorderBottom) / 2;
        HalfWidth = (MapWind0->BorderLeft + MapWind0->Width
	 - MapWind0->BorderRight) / 2;
        maplon = X_Lon_Convert((Bx.Low.X + Bx.High.X) / 2);
        maplat = Y_Lat_Convert((Bx.Low.Y + Bx.High.Y) / 2);
        lonzero = X_Lon_Convert((long)Bx.Low.X);
        latzero = Y_Lat_Convert((long)Bx.Low.Y);
        lonscale = LATSCALE * cos(maplat * PiOver180);
        scale1 = lonscale * HORPIX * (lonzero - maplon) / HalfWidth;
        scale2 = LATSCALE * VERTPIX * (latzero - maplat) / HalfHeight;
        mapscale = max(scale1, scale2);
        valueset();
        lonzero += MapWind0->BorderLeft * x_lon /lonscalefactor;
        latzero += MapWind0->BorderTop * y_lat /latscalefactor;
        scale1 = lonscale * HORPIX * (lonzero - maplon) / HalfWidth;
        scale2 = LATSCALE * VERTPIX * (latzero - maplat) / HalfHeight;
        mapscale = max(scale1, scale2);
        FieldUpdate = 1;
        ReDraw = 1;
	} /* if */
      break;
      } /* ID_MC_ZOOM */
     case ID_MC_AUTO:
      {
      FindCenter(&maplat, &maplon);
      ClearWindow(MapWind0, backpen);
      FieldUpdate = 1;
      ReDraw = 1;
      break;
      } /* MC_AUTO */
     case ID_MC_LOADALL:
      {
      if (! dbaseloaded) Database_LoadDisp(0, 1, NULL, 0);
      if (dbaseloaded)
          {
          loadmapbase(0, 1);

          }
      break;
      } /* LOADALL */
     case ID_MC_LOADACTIVE:
      {
      Load_Object(OBN, NULL);
      break;
      } /* LOADACTIVE */
     case ID_MC_LOADTOPOS:
      {
      loadtopo();
      break;
      } /* LOADTOPOS */
     case ID_MC_SAVEALL:
      {
#ifdef DBASE_SAVE_COMPOSITE
      DBase_SaveComposite();
#else
      short i;
      struct BusyWindow *BWMT;

      BWMT = BusyWin_New("Save Vectors", NoOfObjects, 1, MakeID('B','W','M','T'));

      for (i=0; i<NoOfObjects; i++)
       {
       if (DBase[i].Lat)
        saveobject(i, NULL, DBase[i].Lon, DBase[i].Lat, DBase[i].Elev);
       if (CheckInput_ID() == ID_BW_CLOSE)
        break;
       if (BWMT)
        BusyWin_Update(BWMT, i + 1);
       } /* for i=0... */
      if (BWMT)
       BusyWin_Del(BWMT);
      savedbase(1);
#endif /* DBASE_SAVE_COMPOSITE */
      break;
      } /* SAVEALL */
     case ID_MC_SAVEACTIVE:
      {
      if (! saveobject(OBN, NULL, DBase[OBN].Lon, DBase[OBN].Lat,
		DBase[OBN].Elev))
       savedbase(1);
      break;
      } /* SAVEACTIVE */
     case ID_MC_PRINT:
      {
      LoadRGB4(&WCSScrn->ViewPort, &PrintColors[0], 16);
      SetRGB4(&WCSScrn->ViewPort, 1, 15, 15, 15);
      PrintScreen(WCSScrn, MapWind0->TopEdge, /* + MapWind0->BorderTop,*/
	MapWind0->LeftEdge,/* + MapWind0->BorderLeft,*/
	MapWind0->Width,/* - MapWind0->BorderLeft - MapWind0->BorderRight,*/
	MapWind0->Height,/* - MapWind0->BorderTop - MapWind0->BorderBottom,*/
	0, (SPECIAL_FULLCOLS | SPECIAL_ASPECT));
      if (CheckState == 2)
        {
        LoadRGB4(&WCSScrn->ViewPort, &PrimaryColors[0], 16);
        }
      else if (CheckState == 1)
        {
        LoadRGB4(&WCSScrn->ViewPort, &PrintColors[0], 16);
        }
      else
        {
        LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
        }
      break;
      } /* PRINT */
     case ID_MC_ALIGN:
      {
      Make_MA_Window(MP);
      } /*  */
     case ID_MC_FRACTALMAP:
      {
      FractalMap_Draw();
      break;
      } /*  */
     case ID_MC_FINDDIST:
      {
      FindDistance();
      break;
      } /*  */
     case ID_MC_COLORMAP:
      {
      if (dbaseloaded) MakeColorMap();
      break;
      } /*  */
     case ID_MC_VIEWSHED:
      {
      if (mapscale != 0.0) Viewshed_Map(OBN);
      break;
      } /*  */
     case ID_MC_FIXFLATS:
      {
      struct Box Bx;

      if (GetBounds(&Bx))
       FlatFixer(&Bx);
      break;
      } /*  */
     case ID_MC_BUILDDEM:
      {
      Make_MD_Window();
      break;
      } /* DEM function */
     case ID_CL_WINDOW:
      {
      Handle_CL_Window(WCS_ID);
      break;
      } /* open cloud editor */
     case ID_WV_WINDOW(0):
      {
      Handle_WV_Window(WCS_ID);
      break;
      } /* open wave editor */
     case ID_MC_NEWOBJECT:
      {
      DBaseObject_New();
      break;
      } /* ID_MC_CLEAR */
     case ID_MC_FINDMOUSE:
      {
      if (mapscale == 0.0) break;
      if (findmouse(0, 0, 0) >= 0)
       findarea(OBN);
      break;
      } /*  */
     case ID_MC_FINDMULTI:
      {
      if (mapscale == 0.0) break;
      if (findmulti() >= 0)
       findarea(OBN);
      break;
      } /*  */
     case ID_MC_OUTLINE:
      {
      struct clipbounds cb; 

      if (mapscale == 0.0) break;
      setclipbounds(MapWind0, &cb);
      if (DBase[OBN].Color == 2) outline(MapWind0, OBN, 7, &cb);
      else outline(MapWind0, OBN, 2, &cb);
      break;
      } /*  */
     case ID_MC_ADDPTSNEW:
      {
      addpoints(0, 0);
      break;
      } /*  */
     case ID_MC_ADDPTSAPPEND:
      {
      addpoints((long)DBase[OBN].Points + 1, 0);
      break;
      } /*  */
     case ID_MC_ADDPTSINSERT:
      {
      short selectpoint;

      MapGUI_Message(0, (char*)GetString( MSG_MAPGUI_SETVERTEXTOINSERTAFTERQDONEUUUPDDDOWNESCABORT ));  // "Set vertex to insert after. Q=done Uu=up Dd=down ESC=abort"
      selectpoint = modpoints(0);
      MapGUI_Message(0, " ");
      if (selectpoint)
       addpoints(selectpoint + 1, 1);
      break;
      } /*  */
     case ID_MC_STREAMNEW:
      {
      makestream(0);
      break;
      } /*  */
     case ID_MC_STREAMAPPEND:
      {
      makestream((long)DBase[OBN].Points);
      break;
      } /*  */
     case ID_MC_MODPTS:
      {
      MapGUI_Message(0, (char*)GetString( MSG_MAPGUI_QDONEUUUPDDDOWNDELETE ));  // "Q=done Uu=up Dd=down -=delete"
      modpoints(1);
      MapGUI_Message(0, " ");
      break;
      } /*  */
     case ID_MC_CONFORMACT:
      {
      error = (topoload == 0) ? loadtopo(): 0;
      if (!error)
       {
       if ((DBase[OBN].Flags & 2) && strcmp(DBase[OBN].Special, "TOP")
		&& strcmp(DBase[OBN].Special, "SFC"))
        {
        error = maptotopo(OBN);
        if (! error)
         {
         sprintf (str, (char*)GetString( MSG_MAPGUI_VECTORCONFORMEDTOTOPOGRAPHY ), DBase[OBN].Name);  // "Vector %s conformed to topography."
         Log(MSG_NULL, (CONST_STRPTR)str);
	 }
        else
         Log(MSG_NULL, GetString( MSG_MAPGUI_VECTORTOPOCONFORMATIONABORTED ));  // "Vector topo conformation aborted."
        }
       }
      break;
      } /*  */
     case ID_MC_CONFORMALL:
      {
      if ((error = (topoload == 0) ? loadtopo(): 0) == 0)
       {
       short i;
       struct BusyWindow *BWMT;

       BWMT = BusyWin_New((char*)GetString( MSG_MAPGUI_CONFORM ), NoOfObjects, 1, MakeID('B','W','M','T'));  // "Conform"
       for (i=0; i<NoOfObjects; i++)
        {
        if ((DBase[i].Flags & 2) && DBase[i].Special[0] == 'V')
         {
         if ((error = maptotopo(i)) == 1)
          break;
         }  
        if (CheckInput_ID() == ID_BW_CLOSE)
         {
         error = 1;
         break;
         } /* if user abort */
        if (BWMT)
         BusyWin_Update(BWMT, i + 1);
        } /* for i=0... */
       if (BWMT)
        BusyWin_Del(BWMT);
       if (! error)
        {
        Log(MSG_NULL, GetString( MSG_MAPGUI_ALLENABLEDVECTORSCONFORMEDTOTOPOGRAPHY ));  // "All enabled vectors conformed to topography."
        } /* if ! error */
       else
        {
        sprintf(str, (char*)GetString( MSG_MAPGUI_VECTORTOPOCONFORMATIONABORTEDOBJECTSCOMPLETED ),i);  // "Vector topo conformation aborted! %d objects completed."
        Log(MSG_NULL, (CONST_STRPTR)str);
        } /* else aborted */
       } /* if topos loaded OK */
      break;
      } /*  */
     case ID_MC_MATCHPTS:
      {
      matchpoints();
      break;
      } /*  */
     case ID_MC_MOVEORIG:
      {
      setorigin();
      break;
      } /*  */
     case ID_MC_INPUT:
      {
      MP_DigMode = User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),     // "Mapping Module: Digitize"
                                GetString( MSG_MAPGUI_SETDIGITIZINGINPUTSOURCE ),  // "Set digitizing input source."
                                GetString( MSG_MAPGUI_BITPADSUMMAGRIDMOUSE ),      // "Bitpad|Summagrid|Mouse"
                                (CONST_STRPTR)"bsm");
      break;
      } /*  */
     case ID_MC_DUPLICATE:
      {
      CloneObject();
      break;
      } /*  */
#ifdef ENABLE_STATISTICS
     case ID_MC_LOADSTATS:
      {
      statptr=loadstat(statptr, 0);
      break;
      } /*  */
     case ID_MC_NORMALIZE:
      {
      normalize();
      break;
      } /*  */
     case ID_MC_GRAPHSTYLE:
      {
      graphset();
      break;
      } /*  */
     case ID_MC_GRAPH:
      {
      if (graphptr) graphdraw(graphptr);
      break;
      } /*  */
     case ID_MC_ERASE:
      {
      grapherase();
      break;
      } /*  */
#endif /* ENABLE_STATISTICS */

     case ID_MC_SETCAM:
      {
      if (mapscale != 0.0)
       SetView_Map(1);
      break;
      } /*  */
     case ID_MC_SETFOC:
      {
      if (mapscale != 0.0)
       SetView_Map(0);
      break; 
      } /*  */
     case ID_MC_SURFONE:
      {
      if (mapscale != 0.0)
       SetSurface_Map(0);
      break;
      } /*  */
     case ID_MC_SURFTWO:
      {
      if (mapscale != 0.0)
       SetSurface_Map(1);
      break;
      } /*  */
     case ID_MC_SURFTHREE:
      {
      if (mapscale != 0.0)
       SetSurface_Map(2);
      break;
      } /*  */
     case ID_MC_SURFFOUR:
      {
      if (mapscale != 0.0)
       SetSurface_Map(3);
      break;
      } /*  */
     case ID_MC_INTERP:
      {
      interpolatepath();
      break;
      } /*  */
     case ID_MC_VECCAM:
     case ID_MC_VECFOC:
      {
      if (ReturnID == ID_MC_VECCAM)
       VectorToPath(0);
      else
       VectorToPath(1);
      if (MP->ptsdrawn)
       {
       struct clipbounds cb;

       setclipbounds(MapWind0, &cb);
       ShowView_Map(&cb);
       }
      if (EM_Win)
       {
       UnsetKeyFrame(EM_Win->Frame, 0, EM_Win->MoItem, 0);
       GetKeyTableValues(0, EM_Win->MoItem, 1);
       Set_EM_List(1);
       if (EMPL_Win)
        DoMethod(EMPL_Win->LS_List, MUIM_List_Redraw, MUIV_List_Redraw_All);
       Set_EM_Item(EM_Win->MoItem);
       Set_Radial_Txt(2);
       ResetTimeLines(-1);
       DisableKeyButtons(0);
       } /* if motion editor open */
      break;
      } /*  */
     case ID_MC_CAMVEC:
      {
      PathToVector(0);
      break;
      } /*  */
     case ID_MC_FOCVEC:
      {
      PathToVector(1);
      break;
      } /*  */
/*
     case ID_MC_PARTICLETREE:
      {
      ParticleTree();
      break;
      }
*/
     } /* switch */
    MenuNumber = Item->NextSelect;

    } /* while another menu item */
   break;
   } /* MENUPICK */

  case IDCMP_MOUSEBUTTONS:
   {
   switch (Event.Code)
    {
    case SELECTDOWN:
     {
     if (Event.Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
      {
      if (findmouse(Event.MouseX, Event.MouseY, 0) >= 0)
       {
       findarea(OBN);
       if (DE_Win)
        DoMethod(DE_Win->LS_List, MUIM_List_Select, OBN,
		MUIV_List_Select_On, NULL);
       } /* if item found */
      } /* if shift key depressed */
     else if (Event.Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
      {
      if (findmouse(Event.MouseX, Event.MouseY, 0) >= 0)
       {
       Map_DB_Object(OBN, 0, 0);
       DBase[OBN].Mark[0] = 'N';
       DBase[OBN].Flags &= (255 ^ 2);
       DBase[OBN].Enabled = ' ';
       if (DE_Win)
        set(DE_Win->Check, MUIA_Selected, FALSE);
       } /* if object found */
      } /* if alt key depressed */
     else if (Event.Qualifier & IEQUALIFIER_CONTROL)
      {
      shiftmap(2, Event.MouseX, Event.MouseY);
       
      mapscale = mapscale / 2;
      valueset();
       
      if(AutoClear)
       {
       ClearWindow(MapWind0, backpen);
       MP->ptsdrawn = 0;
       } /* if */
      setclipbounds(MapWind0, &Clip);
      makemap(MapWind0, Clip.lowx, Clip.lowy, Clip.highx, Clip.highy,
       (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0));
      if(InterStuff)
       {
       ShowView_Map(&Clip);
       ShowPaths_Map(&Clip);
       } /* if */
      MapGUI_Update(MP); /* In case Exag changed as a result of loading Topos */
      } /* if CTRL key depressed */
     else if (MD_Win)
      {
      if (MD_Win->PMode)
       {
       short LastX = -10000, LastY = -10000, Displace = 0;
       long PtCoord[4], PtsAdded = 0, CurPt = 0;
       double m, a, b, X1, X2, X3, Y1, Y2, Y3, ElCalc, LastElCalc, LastEl,
		ElCalcDif, Displacement;
       struct datum *DT, *NT, *FT;

       PtCoord[0] = Event.MouseX - 5;
       PtCoord[1] = Event.MouseX + 5;
       PtCoord[2] = Event.MouseY - 5;
       PtCoord[3] = Event.MouseY + 5;

       switch (MD_Win->PMode)
        {
        case 1:
         {
         DT = MD_Win->CurDat;
         
         switch (MD_Win->DMode)
          {
          case 0:	/* isoline */
           SetAPen(MapWind0->RPort, 7);
           break;
          case 1:	/* gradient */
           SetAPen(MapWind0->RPort, 5);
           break;
          case 2:	/* concave */
           SetAPen(MapWind0->RPort, 6);
           break;
          case 3:	/* convex */
           SetAPen(MapWind0->RPort, 2);
           break;
          } /* switch */

         while (Event.Class != IDCMP_MOUSEBUTTONS ||
		(Event.Class == IDCMP_MOUSEBUTTONS && Event.Code != SELECTUP))
          {
          if (abs(LastX - Event.MouseX) >= MD_Win->MinDist
		|| abs(LastY - Event.MouseY) >= MD_Win->MinDist)
           {
           if ((MD_Win->CurDat->nextdat = Datum_New()))
            {
            WritePixel(MapWind0->RPort, Event.MouseX, Event.MouseY);
            MD_Win->CurDat = MD_Win->CurDat->nextdat;
            MD_Win->CurDat->values[0] = X_Lon_Convert((long)Event.MouseX);
            MD_Win->CurDat->values[1] = Y_Lat_Convert((long)Event.MouseY);
            PtsAdded ++;
            MD_Win->ControlPts ++; 
            sprintf(str, "%d", MD_Win->ControlPts);
            set(MD_Win->Text, MUIA_Text_Contents, (IPTR)str);
            LastX = Event.MouseX;
            LastY = Event.MouseY;
	    } /* if */
           else
            break;
	   } /* if greater than minimum distance between points */
          FetchEvent(MapWind0, &Event);
	  } /* while */
         if (PtsAdded)
          {
          str[0] = 0;
          X1 = 0.0;
          X2 = PtsAdded - 1;
          X3 = (X1 + X2) / 2.0;
          if (MD_Win->ElSource == 1)
           {
           short NotFound = 0;

/* find XY coords of start and end points and search for proximal control pts */
           PtCoord[0] = Event.MouseX - 5;
           PtCoord[1] = Event.MouseX + 5;
           PtCoord[2] = Event.MouseY - 5;
           PtCoord[3] = Event.MouseY + 5;
           if ((NT = Datum_MVFind(MD_Win->TC, DT->nextdat, PtCoord, 0)))
            Y2 = NT->values[2];
           else 
            NotFound = 1;

           Event.MouseX = Lon_X_Convert(DT->nextdat->values[0]);
           Event.MouseY = Lat_Y_Convert(DT->nextdat->values[1]);
           PtCoord[0] = Event.MouseX - 5;
           PtCoord[1] = Event.MouseX + 5;
           PtCoord[2] = Event.MouseY - 5;
           PtCoord[3] = Event.MouseY + 5;
           if ((NT = Datum_MVFind(MD_Win->TC, DT->nextdat, PtCoord, 0)))
            Y1 = NT->values[2];
           else
            NotFound += 2;

/* if end points not found warn and undraw points if user wishes */
           if (NotFound)
            {
            if (User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                                      // "Map View: Build DEM"
                             (CONST_STRPTR) GetString( MSG_MAPGUI_ATLEASTONEENDCONTROLPOINTFORTHELINESEGMENTJUSTDRAWNC ),  // "At least one end control point for the line segment just drawn could not be found!\nDo you wish to use the current and minimum slider elevations for this segment or abort the operation?",
                             GetString( MSG_MAPGUI_SLIDERABORT ),                                                          // "Slider|Abort"
                             (CONST_STRPTR)"sa"))
             {
             if (NotFound >= 2)
              Y1 = MD_Win->CurEl;
             if (NotFound % 2)
              Y2 = MD_Win->MinEl;
	     } /* if use slider values for elevation control */
            else
             {
             MD_Win->CurDat = DT;
             DT = DT->nextdat;
             SetAPen(MapWind0->RPort, 1);
             while (DT)
              {
              Event.MouseX = Lon_X_Convert(DT->values[0]);
              Event.MouseY = Lat_Y_Convert(DT->values[1]);
              WritePixel(MapWind0->RPort, Event.MouseX, Event.MouseY);
              MD_Win->ControlPts --;
              DT = DT->nextdat;
	      } /*  while */
             Datum_Del(MD_Win->CurDat->nextdat);
             MD_Win->CurDat->nextdat = NULL;
             sprintf(str, "%d", MD_Win->ControlPts);
             set(MD_Win->Text, MUIA_Text_Contents, (IPTR)str);
             break;
	     } /* else user wishes to abort the operation - remove added points */
	    } /* if one end point at least not found within 5 pixels of the line seg */
	   } /* if get elevation range from existing control points */
          else
           {
           Y1 = MD_Win->CurEl;
           Y2 = MD_Win->MinEl;
	   } /* else not end point source */
          if (PtsAdded > 1)
           m = (Y2 - Y1) / (X2 - X1);
          else
           m = 0.0;
          if (MD_Win->DMode == 2)	/* concave up */
           Y3 = (1.0 + MD_Win->NonLin) * m * X3 + Y1;
          else				/* convex up */
           Y3 = (1.0 - MD_Win->NonLin) * m * X3 + Y1;
          if (PtsAdded > 1)
           {
           a = (((Y3 - Y1) / X3) - ((Y2 - Y1) / X2)) / (X3 - X2);
           b = ((Y2 - Y1) / X2) - a * X2;
	   } /* if */
          else
           a = b = 0.0;
          if ((MD_Win->Displace == 1 || (PtsAdded == 1 && MD_Win->Displace == 3)
		|| (PtsAdded > 1 && MD_Win->Displace == 2)) && MD_Win->ElSource <= 1)
           Displace = 1;
          NT = DT;
          FT = DT = DT->nextdat;
          while (DT)
           {
           if (MD_Win->ElSource == 0 || MD_Win->ElSource == 1)
		/* from slider or control points */
            {
            if (MD_Win->DMode == 0)		/* isoline */
             ElCalc = Y1;
            else if (MD_Win->DMode == 1)	/* gradient */	
             ElCalc = Y1 + CurPt * m;
            else				/* non-linear gradient */
             ElCalc = a * CurPt * CurPt + b * CurPt + Y1;

            if (PtsAdded == 1)
             LastEl = LastElCalc = ElCalc;

            if (Displace && (DT != FT || PtsAdded == 1))
             {
             ElCalcDif = ElCalc - LastElCalc;
             Displacement =
		(1.0 - ((drand48() + drand48() + drand48() + drand48()) / 2.0))
		* MD_Win->StdDev;
             LastEl = DT->values[2] = LastEl + ElCalcDif + Displacement;
             LastElCalc = ElCalc;
	     } /* if */
            else
             LastEl = LastElCalc = DT->values[2] = ElCalc;
	    } /* if source is slider */
           else if (MD_Win->ElSource == 2)	/* from DEM */
            {
            Event.MouseX = Lon_X_Convert(DT->values[0]);
            Event.MouseY = Lat_Y_Convert(DT->values[1]);
            DT->values[2] = LatLonElevScan(&Event, NULL, 1); 
	    }
           else					/* numeric input */
            {
            if (! GetInputString((char*)GetString( MSG_MAPGUI_ENTERELEVATIONVALUEFORNEWCONTROLPOINT ),  // "Enter elevation value for new control point."
		    "abcdefghijklmnopqrstuvwxyz", str))
             {
             MD_Win->CurDat = NT;
             SetAPen(MapWind0->RPort, 1);
             while (DT)
              {
              Event.MouseX = Lon_X_Convert(DT->values[0]);
              Event.MouseY = Lat_Y_Convert(DT->values[1]);
              WritePixel(MapWind0->RPort, Event.MouseX, Event.MouseY);
              MD_Win->ControlPts --;
              DT = DT->nextdat;
	      } /*  while */
             Datum_Del(MD_Win->CurDat->nextdat);
             MD_Win->CurDat->nextdat = NULL;
             sprintf(str, "%d", MD_Win->ControlPts);
             set(MD_Win->Text, MUIA_Text_Contents, (IPTR)str);
             break;
	     } /* if user cancelled remove rest of points */
            DT->values[2] = atof(str);
	    }/* numeric input */
           NT = DT;
           DT = DT->nextdat;
           CurPt ++;
	   } /* while */
/* normalize displaced values to end at correct elevation and elliminate
	reversals if desired */
          if (Displace && PtsAdded > 1)
           {
           ElCalcDif = (LastElCalc - LastEl) / (PtsAdded - 1);
           NT = FT;
           DT = FT->nextdat;
           CurPt = 1;
           while (DT)
            {
            DT->values[2] += (ElCalcDif * CurPt);
            if (MD_Win->NoReversal)
             {
             if (DT->values[2] > NT->values[2])
              DT->values[2] = NT->values[2];
             if (DT->values[2] < Y2)
              DT->values[2] = Y2;
	     } /* if one of the gradient modes */
            DT = DT->nextdat;
            NT = NT->nextdat;
            CurPt ++;
	    } /* while */
	   } /* if */
	  } /* if */
         break;
	 } /* add DEM control point */
        case 2:
         {
         if ((DT = Datum_MVFind(MD_Win->TC, NULL, PtCoord, 0)))
          {
          LastX = Lon_X_Convert(DT->values[0]);
          LastY = Lat_Y_Convert(DT->values[1]);
          while (1)
           {
           FetchEvent(MapWind0, &Event);
           SetAPen(MapWind0->RPort, 1);
           WritePixel(MapWind0->RPort, LastX, LastY);
           SetAPen(MapWind0->RPort, 7);
           WritePixel(MapWind0->RPort, Event.MouseX, Event.MouseY);
           LastX = Event.MouseX;
           LastY = Event.MouseY;
           if (Event.Class == IDCMP_MOUSEBUTTONS && Event.Code == SELECTUP)
            break;
	   } /* while not mouse button up and datum found */
          DT->values[0] = X_Lon_Convert((long)Event.MouseX);
          DT->values[1] = Y_Lat_Convert((long)Event.MouseY);
	  } /* if datum found */
         break;
	 } /* move DEM control point */
        case 3:
         {
         if ((DT = Datum_MVFind(MD_Win->TC, NULL, PtCoord, -1)))
          {
          LastX = Lon_X_Convert(DT->nextdat->values[0]);
          LastY = Lat_Y_Convert(DT->nextdat->values[1]);
          SetAPen(MapWind0->RPort, 1);
          WritePixel(MapWind0->RPort, LastX, LastY);
          NT = DT->nextdat->nextdat;
          free_Memory(DT->nextdat, sizeof (struct datum));
          DT->nextdat = NT;
          if (DT->nextdat == NULL)
           MD_Win->CurDat = DT;
          MD_Win->ControlPts --;
          sprintf(str, "%d", MD_Win->ControlPts);
          set(MD_Win->Text, MUIA_Text_Contents, (IPTR)str);
	  } /* if point found */
         break;
	 } /* delete control point */
        case 4:
         {
         if ((DT = Datum_MVFind(MD_Win->TC, NULL, PtCoord, 0)))
          {
          sprintf(str, (char*)GetString( MSG_MAPGUI_DEMCTRLPTLATFLONFELEV2F ), DT->values[1],  // "DEM Ctrl Pt: Lat %f Lon %f Elev %.2f"
		DT->values[0], DT->values[2]);
          MapGUI_Message(0, str);
	  } /* if datum found */
         break;
	 } /* DEM control point information */
        case 5:
         {
         if ((DT = Datum_MVFind(MD_Win->TC, NULL, PtCoord, 0)))
          {
          MD_Win->CurEl = DT->values[2];
          set(MD_Win->IntStr[1], MUIA_String_Integer, MD_Win->CurEl);
	  } /* if datum found */
         break;
	 } /* set elevation to a control point */
	} /* switch */
       } /* if mode */
      else
       FindObject = 1;
      } /* else DEM builder window open */
     else
      FindObject = 1;

     if (FindObject)
      {
      short item;
      
      if (! SetIAView_Map(&Event))
       {
       if ((item = findmouse(Event.MouseX, Event.MouseY, 1)) >= 0)
        {
        findarea(item);
        } /* if object found */
       MP->LastSeconds = Event.Seconds;
       MP->LastMicros = Event.Micros;
       } /* identify object */
      } /* if not interactive event */
     break;
     } /* SELECTDOWN */
    } /* switch Event.Code */
   break;
   } /* MOUSEBUTTONS */
  case IDCMP_VANILLAKEY:
   {
   switch (Event.Code)
    {
    case '+':
     {
     mapscale /= 2.0;
     ReDraw = FieldUpdate = 1;
     break;
     }
    case '-':
     {
     mapscale *= 2.0;
     ReDraw = FieldUpdate = 1;
     break;
     }
    } /* switch */
   case 13:
   case 10:
    {
    if (Event.Qualifier & IEQUALIFIER_NUMERICPAD)
     MakeMotionKey();
    break;
    } /* enter */
   break;
   } /* VANILLAKEY */
  case IDCMP_RAWKEY:
   {
   double mult = 1.0;

   if (Event.Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
    mult = 4.0;
   switch (Event.Code)
    {
    case 0x4c:
     {
     maplat += (mult * ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop -
	 MapWind0->BorderBottom) / 2)) / 4.0);
     ReDraw = FieldUpdate = 1;
     break;
     } /* arrow up */
    case 0x4d:
     {
     maplat -= (mult * ((mapscale / (LATSCALE * VERTPIX)) * 
	((MapWind0->Height - MapWind0->BorderTop -
	 MapWind0->BorderBottom) / 2)) / 4.0);
     ReDraw = FieldUpdate = 1;
     break;
     } /* arrow down */
    case 0x4e:
     {
     maplon -= (mult * ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft -
	 MapWind0->BorderRight) / 2)) / 4.0);
     ReDraw = FieldUpdate = 1;
     break;
     } /* arrow right */
    case 0x4f:
     {
     maplon += (mult * ((mapscale / ((LATSCALE * cos(maplat * PiOver180)) * HORPIX)) * 
	((MapWind0->Width - MapWind0->BorderLeft -
	 MapWind0->BorderRight) / 2)) / 4.0);
     ReDraw = FieldUpdate = 1;
     break;
     } /* arrow left */
    } /* switch Event.Code */
   break;
   } /* RAWKEY */
  } /* switch Event.Class */
 
 if(FieldUpdate)
 	{ /* check and update all fields */
 	if(mapscale <= 0.0)
 		mapscale = 100.0;
 	if(maplat < -89.999)
 		maplat = -89.999;
 	if(maplat > 89.999)
 		maplat = 89.999;
/* 	if(maplon < 0.0)
 		maplon = 0.0;
 	if(maplon > 360.0)
 		maplon = 360.0;*/
 	if(ContInterval > 6144)
 		ContInterval = 12288;
 	if(ContInterval < 6)
 		ContInterval = 3;

	MapGUI_Update(MP);
 	} /* if */
 	
 if(ReDraw)
	{
        valueset();
        if (align)
         {
         if  ((AlignBox.High.Y != AlignBox.Low.Y) && (AlignBox.High.X - AlignBox.Low.Y))
          valuesetalign();
         else
          User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                                    // "Mapping Module: Align"
                       GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal.
                       GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                     (CONST_STRPTR)"o");
         } /* if align */
	if(AutoClear)
		{
		ClearWindow(MapWind0, backpen);
		MP->ptsdrawn = 0;
		} /* if */
	setclipbounds(MapWind0, &Clip);
	makemap(MapWind0, Clip.lowx, Clip.lowy, Clip.highx, Clip.highy,
	 (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0));
	if(InterStuff)
   		{
	   	ShowView_Map(&Clip);
		ShowPaths_Map(&Clip);
	   	} /* if */
	MapGUI_Update(MP); /* In case Exag changed as a result of loading Topos */
	} /* if */ 

 if (abort)
  {
  Close_Map_Window(0);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
  break;
  } /* if time to quit map */

 if (InternalEvent)
  {
  InternalEvent = 0;
  goto HandleEvent;
  } /* if IntuiMessage already read in an internal function */
 else
  {
  MapGUI_Update(MP);
  } /* else */

 } /* while QuickFetchEvent() */

} /* Handle_Map_Window() */

/************************************************************************/

void Close_Map_Window(short ExitWCS)
{
 short i, SaveIt;

 if (DB_Mod)
  {
  SaveIt = User_Message(GetString( MSG_MAPGUI_DATABASESAVE ),                                          // "Database: Save"
                        GetString( MSG_MAPGUI_THEDATABASEHASBEENMODIFIEDSINCEITWASLOADEDOYOUWISHTO ),  // "The Database has been modified since it was loaded.\nDo you wish to save it or a Master Object file now?",
                        GetString( MSG_MAPGUI_DBASEOBJECTBOTHNEITHER ),                                // "D'base|Object|Both|Neither"
                        (CONST_STRPTR)"dmbn");
  if (SaveIt == 1 || SaveIt == 3)
   savedbase(1);
  if (SaveIt == 2 || SaveIt == 3)
   DBase_SaveComposite();
  } /* if */
 if (FreeAllVectors())
  savedbase(1);

 if (dbaseloaded && mapscale != 0.0)
  savemapprefs();
 
 if (topoload)
  {
  if (mapelmap)
   {
   for (i=0; i<topomaps; i++)
    {
    if (mapelmap[i].map) free_Memory(mapelmap[i].map, mapelmap[i].size);
    } /* for i=0... */
   free_Memory(mapelmap, MapElmapSize);
   mapelmap = NULL;
   } /* if mapelmap */
  if (mapcoords) free_Memory(mapcoords, MapCoordSize);
  mapcoords = NULL;
  if (TopoOBN) free_Memory(TopoOBN, TopoOBNSize);
  TopoOBN = NULL;
  topoload = 0;
  topomaps = 0;
  } /* if topos loaded previously */

 if (MP->AlignWin)
  Close_MA_Window(MP);

 if(MP->MAPC)
 	{
 	MapGUI_Del(MP);
 	} /* if */

#ifdef UNDER_CONST
 if(UnderConst)
 	{
 	UnderConst_Del();
 	} /* if */
#endif /* UNDER_CONST */

#ifdef ENABLE_STATISTICS
 if (statptr) free_Memory(statptr,statsize);
 statptr = NULL;
 if (normptr) free_Memory(normptr,normsize);
 normptr = NULL;
 if (normvalptr) free_Memory(normvalptr,normsize);
 normvalptr = NULL;
 if (normstatptr) free_Memory(normstatptr,normsize);
 normstatptr = NULL;
#endif /* ENABLE_STATISTICS */

 if (MapWind0)
  {
  MP_Top = MapWind0->TopEdge;
  MP_Left = MapWind0->LeftEdge;
  MP_Width = MapWind0->Width;
  MP_Height = MapWind0->Height;
  WCS_Signals ^= MapWind0_Sig;
  MapWind0_Sig = 0;
#ifndef NEW_MAP_MENUS
  ClearMenuStrip(MapWind0);
#else
  if (MP->VisualInfo)
   {
   if (MP->MenuStrip)
    {
    ClearMenuStrip(MapWind0);
    FreeMenus(MP->MenuStrip);
    } /* if MenuStrip */
   FreeVisualInfo(MP->VisualInfo);
   } /* if VisualInfo */
#endif /* NEW_MAP_MENUS */
  closesharedwindow(MapWind0, 0);
  MapWind0 = NULL;
  } /* if map window open */
 if (MP) free_Memory(MP, sizeof (struct MapData));
 MP = NULL;
 DoMethod(app, MUIM_Application_SetMenuCheck, (GA_GADNUM(1) | MO_MAPPING), MapWind0);

#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */


} /* Close_Map_Window() */


/*********************************************************************/

void Make_EL_Window(void)
{
 short i;
 long open;

 if (EL_Win)
  {
  DoMethod(EL_Win->EcoLegendWin, MUIM_Window_ToFront);
  set(EL_Win->EcoLegendWin, MUIA_Window_Activate, TRUE);
  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
  return;
  } /* if window already exists */

 if (! paramsloaded)
  {
  User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
               GetString( MSG_MAPGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENINGT ),  // "You must first load or create a parameter file before opening the Legend."
               GetString( MSG_GLOBAL_OK ),                                                    // "OK"
               (CONST_STRPTR)"o");
  return;
  } /* if no params */

 if ((EL_Win = (struct EcoLegendWindow *)
	get_Memory(sizeof (struct EcoLegendWindow), MEMF_CLEAR)) == NULL)
   return;

 Set_Param_Menu(2);

 EL_Win->EEListSize = (ECOPARAMS + 1) * (sizeof (char *));

 EL_Win->EEList = (char **)get_Memory(EL_Win->EEListSize, MEMF_CLEAR);

 if (! EL_Win->EEList)
  {
  User_Message(GetString( MSG_MAPGUI_MAPVIEWECOSYSTEMLEGEND ),              // "Map View: Ecosystem Legend"
		GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "Out of memory!\nCan't open Ecosystem Legend."
                GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "OK"
                (CONST_STRPTR)"o");
  Close_EL_Window();
  return;
  } /* if out of memory */

 Set_PS_List(EL_Win->EEList, NULL, 2, 0, NULL);

     EL_Win->EcoLegendWin = WindowObject,
      MUIA_Window_Title		, GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
      MUIA_Window_ID		, MakeID('E','L','E','G'),
      MUIA_Window_Screen	, WCSScrn,
      MUIA_Window_Menu		, WCSNewMenus,

      WindowContents, VGroup,
	Child, HGroup,
	  Child, EL_Win->ColBut[0] = SmallImageButton(&EC_Button2),
	  Child, EL_Win->Cycle[0] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[0], End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, EL_Win->ColBut[4] = SmallImageButton(&EC_Button6),
	  Child, EL_Win->Cycle[4] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[4], End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, EL_Win->ColBut[2] = SmallImageButton(&EC_Button4),
	  Child, EL_Win->Cycle[2] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[2], End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, EL_Win->ColBut[3] = SmallImageButton(&EC_Button5),
	  Child, EL_Win->Cycle[3] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[3], End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, EL_Win->ColBut[5] = SmallImageButton(&EC_Button7),
	  Child, EL_Win->Cycle[5] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[5], End,
	  End, /* HGroup */
	Child, HGroup,
	  Child, EL_Win->ColBut[1] = SmallImageButton(&EC_Button3),
	  Child, EL_Win->Cycle[1] = CycleObject,
		MUIA_Cycle_Entries, EL_Win->EEList,
		MUIA_Cycle_Active, EcoLegend[1], End,
	  End, /* HGroup */
        End, /* EL_Group */
      End; /* WindowObject EL_Win->EcoLegendWin */

  if (! EL_Win->EcoLegendWin)
   {
   Close_EL_Window();
   User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
                GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                GetString( MSG_GLOBAL_OK ),               // "OK"
                (CONST_STRPTR)"o");
   return;
   } /* out of memory */

  DoMethod(app, OM_ADDMEMBER, EL_Win->EcoLegendWin);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* ReturnIDs */
  DoMethod(EL_Win->EcoLegendWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EL_CLOSE);  

/* link color buttons to application */
  for (i=0; i<6; i++)
   {
   set(EL_Win->ColBut[i], MUIA_Selected, EcoUse[i]);
   DoMethod(EL_Win->ColBut[i], MUIM_Notify, MUIA_Selected, FALSE,
    app, 2, MUIM_Application_ReturnID, ID_EL_COLBUT(i));
   DoMethod(EL_Win->ColBut[i], MUIM_Notify, MUIA_Selected, TRUE,
    app, 2, MUIM_Application_ReturnID, ID_EL_COLBUT(i));
   } /* for i=0... */

/* link cycle gadgets to application */
  for (i=0; i<6; i++)
   DoMethod(EL_Win->Cycle[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
	app, 2, MUIM_Application_ReturnID, ID_EL_CYCLE(i));

/* Set tab cycle chain */
  DoMethod(EL_Win->EcoLegendWin, MUIM_Window_SetCycleChain,
	EL_Win->ColBut[0], EL_Win->Cycle[0],
	EL_Win->ColBut[1], EL_Win->Cycle[1],
	EL_Win->ColBut[2], EL_Win->Cycle[2],
	EL_Win->ColBut[3], EL_Win->Cycle[3],
	EL_Win->ColBut[4], EL_Win->Cycle[4],
	EL_Win->ColBut[5], EL_Win->Cycle[5], NULL);

/* Open window */
  set(EL_Win->EcoLegendWin, MUIA_Window_Open, TRUE);
  get(EL_Win->EcoLegendWin, MUIA_Window_Open, &open);
  if (! open)
   {
   Close_EL_Window();
   return;
   } /* if no window */
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */

/* Notify of window activation */
  DoMethod(EL_Win->EcoLegendWin, MUIM_Notify, MUIA_Window_Activate, TRUE,
	app, 2, MUIM_Application_ReturnID, ID_EL_ACTIVATE);

} /* Make_EL_Window() */

/*********************************************************************/

void Close_EL_Window(void)
{
 if (EL_Win)
  {
  if (EL_Win->EcoLegendWin)
   {
   set(EL_Win->EcoLegendWin, MUIA_Window_Open, FALSE);
#ifdef WCS_MUI_2_HACK
	   MUI2_MenuCheck_Hack();
#endif /* WCS_MUI_2_HACK */
   DoMethod(app, OM_REMMEMBER, EL_Win->EcoLegendWin);
   MUI_DisposeObject(EL_Win->EcoLegendWin);
   } /* if window created */
  if (EL_Win->EEList) free_Memory(EL_Win->EEList, EL_Win->EEListSize);
  free_Memory(EL_Win, sizeof (struct EcoLegendWindow));
  EL_Win = NULL;
  } /* if */

} /* Close_EL_Window() */

/*********************************************************************/

void Handle_EL_Window(ULONG WCS_ID)
{
 short i;
 long data;

  if ((WCS_ID & 0x0000ff00) == GP_OPEN_WINDOW)
   {
   Make_EL_Window();
   return;
   } /* Open Ecosystem Legend Window */

  if (! EL_Win)
   return;

  switch (WCS_ID & 0x0000ff00)
   {
   case GP_ACTIVEWIN:
    {
    LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    break;
    } /* Activate Settings Editing Window */

   case GP_BUTTONS1:
    {
    switch (WCS_ID)
     {
     case ID_EL_CLOSE:
      {
      Close_EL_Window();
      break;
      } /* close */
     } /* switch */
    break;
    } /* BUTTONS1 */

   case GP_BUTTONS2:
    {
    i = WCS_ID - ID_EL_COLBUT(0);

    get(EL_Win->ColBut[i], MUIA_Selected, &data);
    EcoUse[i] = data;
    break;
    } /* BUTTONS2 */

   case GP_CYCLE1:
    {
    i = WCS_ID - ID_EL_CYCLE(0);

    get(EL_Win->Cycle[i], MUIA_Cycle_Active, &data);
    EcoLegend[i] = data;
    break;
    } /* CYCLE1 */
   } /* switch WCS_ID */

} /* Handle_EL_Window() */

/************************************************************************/

void Update_EcoLegend(short Item, short Item2, short Operation)
{
short i;

if (! EL_Win)
 return;

switch (Operation)
 {
 case 2:
  {
  for (i=0; i<6; i++)
   {
   if (EcoLegend[i] == Item)
    {
    set(EL_Win->Cycle[i], MUIA_Cycle_Active, Item2); 
    EcoLegend[i] = Item2; 
    }
   else if (EcoLegend[i] == Item2)
    {
    set(EL_Win->Cycle[i], MUIA_Cycle_Active, Item);
    EcoLegend[i] = Item; 
    }
   } /* for i=0... */
  break;
  } /* swap */
 case 3:
  {
  for (i=0; i<6; i++)
   {
   if (EcoLegend[i] >= Item)
    {
    if (EcoLegend[i] < ECOPARAMS - 1)
     {
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, EcoLegend[i] + 1); 
     EcoLegend[i] += 1;
     }
    else
     {
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, 0); 
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, EcoLegend[i]); 
     }
    }
   } /* for i=0... */
  break;
  } /* insert */
 case 4:
  {
  for (i=0; i<6; i++)
   {
   if (EcoLegend[i] == Item)
    {
    if (Item > 0)
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, 0);
    else 
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, 1);
    set(EL_Win->Cycle[i], MUIA_Cycle_Active, Item);
    EcoLegend[i] = Item; 
    }
   else if (EcoLegend[i] > Item)
    {
    if (EcoLegend[i] > 0)
     set(EL_Win->Cycle[i], MUIA_Cycle_Active, EcoLegend[i] - 1);
    EcoLegend[i] -= 1; 
    }
   } /* for i=0... */
  break;
  } /* remove */
 } /* switch */

} /* Update_EcoLegend() */

/**********************************************************************/
#ifdef JHKALSJDAHKLSJDALKSDH
void Contour_Export(void)
{
char filename[256], XYZDir[32], XYZFile[256];
short ElSource, ActiveItem, j, *ElevPtr;
long state;
float El;
FILE *fCont;
struct MaxMin3 *MM3;

 MM3 = MaxMin3_New();

 if (! DE_Win)
  {
  Make_DE_Window();
  if (DE_Win)
   User_Message("Map View: Export Contours",
	"Select contour objects to export and reselect Export Contours\
 from the menu  when done.",
	"OK", "o");
  else
   User_Message("Map View: Export Contours",
	"Can't open Database Editor window!\nOperation terminated.","OK", "o");
  return;
  } /* if Database Editor not open */


 ElSource = User_Message("Map View: Export Contours",
	"Extract elevation values from Object Names, Label fields\
 or use the values embedded in the Objects themselves?",
	"Name|Label|Embedded", "nle");
 strcpy(XYZDir, dirname);
 XYZFile[0] = '\0';
 if (getfilename(1, "XYZ File Path/Name", XYZDir, XYZFile))
  {
  strmfp(filename, XYZDir, XYZFile);
  if (fCont = fopen(filename, "w"))
   {
   get(DE_Win->LS_List, MUIA_List_Active, &ActiveItem);
   for (j=NoOfObjects-1; j>=0; j--)
    {
    DoMethod(DE_Win->LS_List, MUIM_List_Select, j,
		 MUIV_List_Select_Ask, &state);
    if (state || j == ActiveItem)
     {
     if (ElSource)
      {
      ElevPtr = NULL;
      if (ElSource == 1)
       El = atof(DBase[j].Name);
      else
       El = atof(DBase[j].Label);
      } /* if derive elevation */
     else
      ElevPtr = DBase[j].Elev;
     if (! Object_WriteXYZ(fCont, DBase[j].Points, DBase[j].Lat,
		DBase[j].Lon, ElevPtr, El, MM3))
      {
      User_Message("Map View: Export Contours",
		"Error writing to file!\nOperation terminated.","OK", "o");
      Log(ERR_WRITE_FAIL, XYZFile);
      break;
      } /* if error writing to file */
     } /* if selected or active */
    } /* for j=0... */
   fclose(fCont);
   strcat(XYZFile, ".hdr");
   strmfp(filename, XYZDir, XYZFile);
   if (fCont = fopen(filename, "w"))
    {
    if (fprintf(fCont, "Max Lon %f\nMin Lon %f\nMax Lat %f\nMin Lat %f\nMax Elev %f\nMin Elev %f\n",
	MM3->MxLon, MM3->MnLon, MM3->MxLat, MM3->MnLat, MM3->MxEl, MM3->MnEl)
	< 0)
     {
     User_Message("Map View: Export Contours",
		"Error writing to XYZ header file!","OK", "o");
     Log(ERR_WRITE_FAIL, XYZFile);
     } /* if write fail */
    fclose(fCont);
    } /* if header file opened */
   else
    {
    User_Message("Map View: Export Contours",
	"Error opening XYZ header file for output!","OK", "o");
    Log(ERR_OPEN_FAIL, XYZFile);
    } /* else */
   } /* if file opened */
  else
   {
   User_Message("Map View: Export Contours",
	"Error opening XYZ file for output!\nOperation terminated.","OK", "o");
   Log(ERR_OPEN_FAIL, XYZFile);
   } /* else */
  } /* if file name selected */

 MaxMin3_Del(MM3);

} /* Contour_Export() */

/***********************************************************************/

short Object_WriteXYZ(FILE *fObj, short Points, double *Lat, double *Lon,
	short *ElevPtr, float El, struct MaxMin3 *MM3)
{
short pt, success = 1;

 if (fObj)
  {
  for (pt = 1; pt<=Points; pt++)
   {
   if (ElevPtr)
    El = ElevPtr[pt];
   if (fprintf(fObj, "%f %f %f\n", Lon[pt], Lat[pt], El) < 0)
    {
    success = 0;
    break;
    }
   if (MM3)
    {
    if (Lon[pt] > MM3->MxLon)
     MM3->MxLon = Lon[pt];
    if (Lon[pt] < MM3->MnLon)
     MM3->MnLon = Lon[pt];
    if (Lat[pt] > MM3->MxLat)
     MM3->MxLat = Lat[pt];
    if (Lat[pt] < MM3->MnLat)
     MM3->MnLat = Lat[pt];
    if (El > MM3->MxEl)
     MM3->MxEl = El;
    if (El < MM3->MnEl)
     MM3->MnEl = El;
    } /* if */
   } /* for pt=1... */
  } /* if file */
 else
  success = 0;

 return (success);

} /* Object_WriteXYZ() */

/*************************************************************************/

struct MaxMin3 *MaxMin3_New(void)
{
struct MaxMin3 *MM3;

 if ((MM3 = (struct MaxMin3 *)
	get_Memory(sizeof (struct MaxMin3), MEMF_ANY)) != NULL)
  {
  MM3->MxLon = MM3->MxLat = MM3->MxEl = -1.0E30;
  MM3->MnLon = MM3->MnLat = MM3->MnEl = 1.0E30;
  } /* if memory OK */

 return (MM3);

} /* MaxMin3_New() */

/*************************************************************************/

void MaxMin3_Del(struct MaxMin3 *MM3)
{

 if (MM3)
  free_Memory(MM3, sizeof (struct MaxMin3));

} /* MaxMin3_Del() */
#endif
