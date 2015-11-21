#include <string>
#include <osg/ref_ptr> 

class NVWatermark
	{
		bool WatermarkState;
		std::string WatermarkText;
		osg::ref_ptr<osgText::Text> WatermarkEntity;

	public:

		NVWatermark() {WatermarkState = 0; WatermarkEntity = NULL;};
		bool GetWatermarkOn(void) {return(WatermarkState);};
		void SetWatermarkState(bool NewState) {WatermarkState = NewState;};
		void ToggleWatermarkState(void) {WatermarkState = !WatermarkState;};
		std::string GetWatermarkText(void) {return(WatermarkText);};
		void SetWatermarkText(std::string NewText) {WatermarkText = NewText; if(WatermarkEntity.valid()) WatermarkEntity->setText(WatermarkText);};
		void SetWatermarkEntity(osgText::Text* NewEntity) {WatermarkEntity = NewEntity; if(WatermarkEntity.valid()) WatermarkEntity->setText(WatermarkText);};

	}; // NVWatermark

