
#include <osg/ref_ptr> 

class NVFolSpecies
	{
	public:
		std::string ImgFileName;
		float Proportion;
		unsigned char Flags; // Flag 0x01: Is actually a Label
		osg::ref_ptr<osg::Image> SpeciesImage;
		osg::ref_ptr<osg::StateSet> SpeciesState;
		TreeList SpeciesInstances;
		osg::ref_ptr<Forest> SpeciesForest;

		NVFolSpecies() {Flags = 0; Proportion = 1.0;};

		void SetName(char *NewName) {ImgFileName = NewName;};
		std::string GetName(void) {return(ImgFileName);};
		int CheckName(void) {return(!ImgFileName.empty());};

		void SetIsLabel(bool IsLabel) {if(IsLabel) Flags |= 0x01; else Flags &= (~0x01);};
		bool GetIsLabel(void) {return(Flags & 0x01 ? 1 : 0);};
		
	}; // NVFolSpecies

