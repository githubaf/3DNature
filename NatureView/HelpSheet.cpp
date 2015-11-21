// HelpSheet.cpp
// Help text array

#include "Types.h"

#include "IdentityDefines.h"
#include "KeyDefines.h"

char *HelpSheet[] =
	{
	NVW_NATUREVIEW_NAMETEXT " " NVW_VIEWER_VERSIONTEXT,
	"",
	"Menus: Right Mouse Button",
	"",
	"Keyboard Controls",
	"Help: "NV_KEY_HELPDESC,
	"Exit: "NV_KEY_EXITDESC,
	"Movement: "NV_KEY_MOVELEFTDESC", "NV_KEY_MOVERIGHTDESC", "NV_KEY_MOVEFWDDESC", "NV_KEY_MOVEBACKDESC,
	//"Move/Turn: "NV_KEY_MOVEFWD2DESC", "NV_KEY_MOVEBACK2DESC", "NV_KEY_TURNLEFTDESC", "NV_KEY_TURNRIGHTDESC,
	"Move/Turn: Up, Down, Left and Right arrow keys",
	"Turn: "NV_KEY_TURNLEFT2DESC", "NV_KEY_TURNRIGHT2DESC", "NV_KEY_TURNUPDESC", "NV_KEY_TURNDOWNDESC,
	"Up/Down: "NV_KEY_MOVEUPDESC", "NV_KEY_MOVEDOWNDESC,
	"Bank: "NV_KEY_TURNCLOCKDESC", "NV_KEY_TURNCCLOCKDESC,
	"Throttle Up/Down: "NV_KEY_THROTTLEUPDESC", "NV_KEY_THROTTLEDNDESC", Mousewheel",
	"Stop Moving: "NV_KEY_STOPMOVINGDESC", Middle Mouse Button",
	"Pause Movement: "NV_KEY_DISABLETOGGLEDESC,
	"Home: "NV_KEY_RETURNHOMEDESC,
	"Drive Mode: "NV_KEY_MOVEMODEDESC,
	"Rotate Mode: "NV_KEY_ROTMODEDESC,
	"Slide Mode: "NV_KEY_SLIDEMODEDESC,
	"Climb Mode: "NV_KEY_CLIMBMODEDESC,
	#ifdef NVW_SUPPORT_QUERYACTION
	"Action Mode: "NV_KEY_QUERYMODEDESC,
	#endif // NVW_SUPPORT_QUERYACTION
	"Undo Movement: "NV_KEY_UNDODESC,
	"Follow Terrain: "NV_KEY_TERRAINFOLLOWDESC,
	"Zoom In/Out: "NV_KEY_ZOOMINDESC "/" NV_KEY_ZOOMOUTDESC,
	"Next/Previous Viewpoint: "NV_KEY_NEXTCAMDESC "/" NV_KEY_PREVCAMDESC,
	"Viewpoint Tour: "NV_KEY_VIEWTOURDESC,
	#ifdef NVW_SUPPORT_QUERYACTION
	"Immediate Action: "NV_KEY_QUERYDESC,
	#endif // NVW_SUPPORT_QUERYACTION
	"Toggle Optimized Movement: \'"NV_KEY_MOVEOPTDESC"\'",
	"Toggle Fullscreen: \'"NV_KEY_FULLTOGGLEDESC"\'",
	"Toggle Framerate: \'"NV_KEY_STATSTOGGLEDESC"\'",
	"Stop Sound: \'"NV_KEY_SOUNDSTOPDESC"\'",
	//"Toggle Ocean: \'"NV_KEY_OCEANTOGGLEDESC"\'",
	//"Toggle Terrain: \'"NV_KEY_TERRAINTOGGLEDESC"\'",
	//"Toggle Vectors: \'"NV_KEY_VECTOGGLEDESC"\'",
	//"Toggle Foliage: \'"NV_KEY_FOLIAGETOGGLEDESC"\'",
	//"Toggle 3D Objects: \'"NV_KEY_OBJECTTOGGLEDESC"\'",
	//"Toggle Overlay: \'"NV_KEY_OVERLAYTOGGLEDESC"\'",
	//"Toggle Labels: \'"NV_KEY_LABELTOGGLEDESC"\'",
	NULL // must be here to end things
	}; // HelpSheet

