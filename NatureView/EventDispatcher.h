// EventDispatcher.h

#include "Types.h"

class EventDispatcher
	{
	public:
		/*
		NVW_EventClass Major, NVW_EventSubClass Minor
			{
			NVW_DS_OK = 0, // 
			NVW_DS_ERROR,
			NVW_DS_MAXIMUM // to prevent comma propogation
			}; // NVW_DispatchStatus
		*/

		enum NVW_EventClass
			{
			NVW_EC_GLOBAL = 0, // systemwide stuff that doesn't go elsewhere
			NVW_EC_NAVIGATION,
			NVW_EC_UI, // opening windows, etc
			NVW_EC_SCENE, // things in the scene
			NVW_EC_LOD, // things to do with performance tuning, etc
			NVW_EC_ACTION, // things that are best categorized as an action, for 100, Alex
			NVW_EC_MEDIA, // Media-related stuff
			NVW_EC_DEBUG, // Debugging stuff
			NVW_EC_MISC, // those that don't go anywhere else
			NVW_EC_MAXIMUM // to prevent comma propogation
			}; // NVW_EventClass

		enum NVW_EventSubClass
			{
			NVW_ESC_NONE = 0,
			NVW_ESC_GLOBAL_EXIT, // systemwide stuff that doesn't go elsewhere

			// NAVIGATION
			NVW_ESC_NAVIGATION_MOVEMENT, // navigation on/off
			NVW_ESC_NAVIGATION_OPTIMIZEMOVE,
			NVW_ESC_NAVIGATION_TERRAINFOLLOW,
			NVW_ESC_NAVIGATION_ZOOM, // in or out
			NVW_ESC_NAVIGATION_VIEWPOINT, // next/prev, start/stop
			NVW_ESC_NAVIGATION_STOP,
			NVW_ESC_NAVIGATION_GOTO,
			NVW_ESC_NAVIGATION_ROTATE,
			NVW_ESC_NAVIGATION_QUERY,
			NVW_ESC_NAVIGATION_DRIVE,
			NVW_ESC_NAVIGATION_SLIDE,
			NVW_ESC_NAVIGATION_CLIMB,
			NVW_ESC_NAVIGATION_UNDO,
			NVW_ESC_NAVIGATION_PAUSE,
			NVW_ESC_NAVIGATION_HOME,
			NVW_ESC_NAVIGATION_MOVEIMMED,
			NVW_ESC_NAVIGATION_TURNIMMED,
			NVW_ESC_NAVIGATION_THROTTLE,

			// UI
			NVW_ESC_UI_HELP, // 
			NVW_ESC_UI_INFO, // 
			NVW_ESC_UI_DRIVE, // 
			NVW_ESC_UI_NAV, // 
			NVW_ESC_UI_MAINPOPUP, // 
			NVW_ESC_UI_FRAMESTATS, // 
			NVW_ESC_UI_CREDITS, // 
			NVW_ESC_UI_HIDEALLWIN, // 

			// SCENE
			NVW_ESC_SCENE_CATMENU,
			NVW_ESC_SCENE_ALTDRAPE,
			NVW_ESC_SCENE_VECTOR,
			NVW_ESC_SCENE_OBJECT,
			NVW_ESC_SCENE_FOLIAGE,
			NVW_ESC_SCENE_LABEL,
			NVW_ESC_SCENE_OCEAN,
			NVW_ESC_SCENE_TERRAIN,
			NVW_ESC_SCENE_OVERLAY,
			NVW_ESC_SCENE_SPLASH,

			// LOD
			NVW_ESC_LOD_MINFEATURESIZE, // 
			NVW_ESC_LOD_DETAIL, // 
			NVW_ESC_LOD_MAINTAINRATE, // 

			// NVW_EC_ACTION
			NVW_ESC_ACTION_QUERYHERE, // Query/Invoke object's action, please

			// MEDIA
			NVW_ESC_MEDIA_CANCELSOUNDS, // Cancel all playing sounds

			// DEBUG
			NVW_ESC_DEBUG_TEXTURES, // Toggle Textures
			NVW_ESC_DEBUG_WIREFRAME, // Toggle wireframe

			// MISC
			NVW_ESC_MISC_NONE, // 

			}; // NVW_EventSubClass

		enum NVW_ParamCode
			{
			NVW_PC_NONE = 0, // 
			NVW_PC_NEXT,
			NVW_PC_PREV,
			NVW_PC_ON,
			NVW_PC_OFF,
			NVW_PC_IN,
			NVW_PC_OUT,
			NVW_PC_MORE,
			NVW_PC_LESS,
			NVW_PC_LEFT,
			NVW_PC_RIGHT,
			NVW_PC_FORWARD,
			NVW_PC_BACKWARD,
			NVW_PC_UP,
			NVW_PC_DOWN,
			NVW_PC_CLOCKWISE,
			NVW_PC_COUNTERCLOCKWISE,
			NVW_PC_START,
			NVW_PC_STOP,
			NVW_PC_TOGGLE,
			NVW_PC_FULL,
			NVW_PC_ZERO,
			NVW_PC_MAXIMUM // to prevent comma propogation
			}; // NVW_EventSubClass

	private:
		static bool DispatchEventGLOBAL(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventSCENE(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventNAVIGATION(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventUI(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventLOD(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventACTION(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventMEDIA(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventDEBUG(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		static bool DispatchEventMISC(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode = NVW_PC_NONE, char *EventParam = NULL);
		
		static unsigned long int MousePosX, MousePosY;
		static float MousePosXf, MousePosYf;

	protected:
		// we don't implement so you can't create an EventDispatcher, just access its static methods
		EventDispatcher();
		EventDispatcher(const EventDispatcher&);
		EventDispatcher& operator= (const EventDispatcher&);
		//~EventDispatcher();

	public:

		static bool DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor);
		static bool DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor, char *EventParam);
		static bool DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam = NULL);
		static void SetMousPosf(float NewXf, float NewYf) {MousePosXf = NewXf; MousePosYf = NewYf;};
		

	}; // EventDispatcher


