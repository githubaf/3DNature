
#ifndef NVW_NVLOD_H
#define NVW_NVLOD_H

#include <float.h>

#include "NVFoliage.h"
#include "ConfigDefines.h"
#include "UsefulTime.h"

#include <osg/Texture2D> 


class NVLOD
	{
	private:
		double _NearClip, _FarClip, _TerrainClip, _FolClip, _ObjClip, _ObjBox, _LabelClip;
		float _MinFeatureSizePixels, _MaxFoliageStems;
		float _LODScalingFactor, _LODAutoFactor;
		bool _CompressTerrainTex, _CompressFolTex;
		bool _FoliageQuality;
		bool _OptimizeMove;
		bool _FramerateMaintain;
		bool _ShowFramerate;
		osg::Texture2D::FilterMode _MipMapMode;

		bool _SkyHidden, _TerrainHidden, _OceanHidden, _FoliageHidden, _LabelsHidden, _FoliageOptimizeHidden, _HelpShown, _ObjectsShown, _VecsShown;
		double _SceneSettledMoment;


	public:
		NVLOD() {_NearClip = 0.0; _FarClip = FLT_MAX; _MinFeatureSizePixels = 0.0f; _MaxFoliageStems = NV_FOLIAGE_STEMS_MAX_DEFAULT; _LODScalingFactor = 1.0f, _LODAutoFactor = 1.0f;
		 _TerrainClip = _FolClip = _ObjClip = _ObjBox = _LabelClip = FLT_MAX;
		 _CompressTerrainTex = _CompressFolTex = false; _FoliageQuality = true; _OptimizeMove = false;
		 _SkyHidden = _TerrainHidden = _OceanHidden = _FoliageHidden = _LabelsHidden = _FoliageOptimizeHidden = _HelpShown = _ObjectsShown = false; _VecsShown = true; _MipMapMode=osg::Texture2D::LINEAR_MIPMAP_LINEAR;
		 _FramerateMaintain = false; _ShowFramerate = false;
		 SetMipmapMode(osg::Texture2D::NEAREST);}
		void SetNearClip(double NewValue) {_NearClip = NewValue;};
		void SetFarClip(double NewValue) {_FarClip = NewValue;};
		void SetMinFeatureSizePixels(float NewValue) {_MinFeatureSizePixels = NewValue;};
		void SetLODScalingFactor(float NewValue) {_LODScalingFactor = NewValue;};
		void SetLODAutoFactor(float NewValue) {_LODAutoFactor = NewValue;};
		void SetMaxFoliageStems(float NewValue) {_MaxFoliageStems = NewValue;};
		void SetTerrainClip(double NewValue) {_TerrainClip = NewValue;};
		void SetFolClip(double NewValue) {_FolClip = NewValue;};
		void SetObjClip(double NewValue) {_ObjClip = NewValue;};
		void SetObjBox(double NewValue) {_ObjBox = NewValue;};
		void SetLabelClip(double NewValue) {_LabelClip = NewValue;};

		void SetCompressTerrainTex(bool NewValue) {_CompressTerrainTex = NewValue;};
		void SetCompressFolTex(bool NewValue) {_CompressFolTex = NewValue;};
		void SetFoliageQuality(bool NewValue) {_FoliageQuality = NewValue;};

		void SetOptimizeMove(bool NewValue) {_OptimizeMove = NewValue;};
		void ToggleOptimizeMove(void) {SetOptimizeMove(!_OptimizeMove);};
		
		void SetFramerateMaintain(bool NewValue) {_FramerateMaintain = NewValue;};
		bool GetFramerateMaintain(void) const {return(_FramerateMaintain);};

		void SetShowFramerate(bool NewValue) {_ShowFramerate = NewValue;};
		bool GetShowFramerate(void) const {return(_ShowFramerate);};

		void SetMipmapMode(osg::Texture2D::FilterMode NewValue) {_MipMapMode = NewValue;};
		osg::Texture2D::FilterMode GetMipmapMode(void) const {return(_MipMapMode);};


		double GetNearClip(void) const {return(_NearClip);};
		double GetFarClip(void) const {return(_FarClip);};
		float GetMinFeatureSizePixels(void) const {return(_MinFeatureSizePixels);};
		float GetLODScalingFactor(void) const {return(_LODScalingFactor);};
		float GetLODAutoFactor(void) const {return(_LODAutoFactor);};
		float GetMaxFoliageStems(void) const {return(_MaxFoliageStems);};
		double GetTerrainClip(void) const {return(_TerrainClip);};
		double GetFolClip(void) const {return(_FolClip);};
		double GetObjClip(void) const {return(_ObjClip);};
		double GetObjBox(void) const {return(_ObjBox);};
		double GetLabelClip(void) const {return(_LabelClip);};

		bool CheckNearClip(void) const {return(_NearClip != 0.0);};
		bool CheckFarClip(void) const {return(_FarClip != FLT_MAX);};
		bool CheckMinFeatureSizePixels(void) const {return(_MinFeatureSizePixels != 0.0f);};
		bool CheckLODScalingFactor(void) const {return(_MinFeatureSizePixels != 1.0f);};
		bool CheckTerrainClip(void) const {return(_TerrainClip != FLT_MAX);};
		bool CheckFolClip(void) const {return(_FolClip != FLT_MAX);};
		bool CheckObjClip(void) const {return(_ObjClip != FLT_MAX);};
		bool CheckObjBox(void) const {return(_ObjBox != FLT_MAX);};
		bool CheckLabelClip(void) const {return(_LabelClip != FLT_MAX);};

		bool CheckCompressTerrainTex(void) const {return(_CompressTerrainTex);};
		bool CheckCompressFolTex(void) const {return(_CompressFolTex);};
		bool CheckFoliageQuality(void) const {return(_FoliageQuality);};

		bool CheckOptimizeMove(void) const {return(_OptimizeMove);};

		void EnableSky(bool NewState) {_SkyHidden = !NewState;};
		bool CheckSkyEnabled(void) const {return(!_SkyHidden);}; // negative sense
		void ToggleSky(void) {EnableSky(!CheckSkyEnabled());};

		void EnableTerrain(bool NewState) {_TerrainHidden = !NewState;};
		bool CheckTerrainEnabled(void) const {return(!_TerrainHidden);}; // negative sense
		void ToggleTerrain(void) {EnableTerrain(!CheckTerrainEnabled());};

		void EnableOcean(bool NewState) {_OceanHidden = !NewState;};
		bool CheckOceanEnabled(void) const {return(!_OceanHidden);}; // negative sense
		void ToggleOcean(void) {EnableOcean(!CheckOceanEnabled());};

		void HideFoliageOptimizeMove(bool NewState) {_FoliageOptimizeHidden = NewState;};
		void EnableFoliage(bool NewState) {_FoliageHidden = !NewState;};
		bool CheckFoliageEnabled(void) const {return(!_FoliageHidden);}; // negative sense
		void ToggleFoliage(void) {EnableFoliage(!CheckFoliageEnabled());};

		void EnableLabels(bool NewState) {_LabelsHidden = !NewState;};
		bool CheckLabelsEnabled(void) const {return(!_LabelsHidden);}; // negative sense
		void ToggleLabels(void) {EnableLabels(!CheckLabelsEnabled());};

		void EnableObjects(bool NewState) {_ObjectsShown = NewState;};
		bool CheckObjectsEnabled(void) const {return(_ObjectsShown);}; // positive sense
		void ToggleObjects(void) {EnableObjects(!CheckObjectsEnabled());};

		void EnableVecs(bool NewState) {_VecsShown = NewState;};
		bool CheckVecsEnabled(void) const {return(_VecsShown);}; // positive sense
		void ToggleVecs(void) {EnableVecs(!CheckVecsEnabled());};

		void EnableHelp(bool NewState);
		bool CheckHelpEnabled(void) const {return(_HelpShown);}; // positive sense
		void ToggleHelp(void) {EnableHelp(!CheckHelpEnabled());};

		osg::Node::NodeMask GetCurrentEnabledNodeMask(void);

		double GetSceneSettledTime(void);



	}; // NVLOD

#endif // !NVW_NVLOD_H
