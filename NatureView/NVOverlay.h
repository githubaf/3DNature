
#ifndef NVW_NVOVERLAY
#define NVW_NVOVERLAY

#define NVW_NVOVERLAY_PSEUDO_SCREENSIZE_X	1280
#define NVW_NVOVERLAY_PSEUDO_SCREENSIZE_Y	1024

#include <osgText/Text>
#include <osg/PositionAttitudeTransform>
#include "InfoDlg.h"

class NVOverlay
	{
		bool OverlayState;
		std::string OverlayText, OverlayTextTemplate;
		int NumLines;
		bool MapState;
		osg::ref_ptr<osg::PositionAttitudeTransform> MapPointerTransform;
		osg::ref_ptr<osg::Image> MapNailImage;
		unsigned long int MapTop, MapLeft, MapBot, MapRight;

	public:

		NVOverlay() {OverlayState = false; NumLines = 4; SetMapState(false); MapTop = MapLeft = MapBot = MapRight = 0;};
		bool GetOverlayOn(void) {return(OverlayState);};
		void SetOverlayState(bool NewState) {OverlayState = NewState;};
		bool GetMapOn(void) {return(MapState);};
		void SetMapState(bool NewState) {MapState = NewState;};
		void SetNumLines(int NewValue) {NumLines = NewValue;};
		void ToggleOverlayState(void) {OverlayState = !OverlayState;};
		std::string GetOverlayText(void) {return(OverlayText);};
		std::string GetOverlayTextTemplate(void) {return(OverlayTextTemplate);};
		std::string GetProcessedText(void);
		static std::string NVOverlay::GetProcessedText(std::string InputText); // can be used by others, like Watermark code
		void SetOverlayText(std::string NewText);
		void SetOverlayTextTemplate(std::string NewText) {OverlayTextTemplate = NewText;};
		void SetMapPointerTransform(osg::PositionAttitudeTransform* NewPAT) {MapPointerTransform = NewPAT;};
		void SetMapNailImage(osg::Image *NewMapNailImage) {MapNailImage = NewMapNailImage;};
		osg::Image *GetMapNailImage(void) {return(MapNailImage.get());};
		void UpdateMapPoint(double WorldXCoord, double WorldYCoord, float Heading); // heading is in radians
		int GetNumLines(void) {return(NumLines);}
		void SetMapHUDBounds(unsigned long int NewMapTop, unsigned long int NewMapLeft, unsigned long int NewMapBot, unsigned long int NewMapRight)
		 {MapTop = NewMapTop; MapLeft = NewMapLeft; MapBot = NewMapBot; MapRight = NewMapRight;};
		bool HandlePotentialMapClick(float RawMouseX, float RawMouseY);

	}; // NVOverlay

osg::Node* createHUD();


#endif // NVW_NVOVERLAY