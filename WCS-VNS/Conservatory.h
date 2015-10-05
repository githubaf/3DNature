// One structure to hold all the GUI window class pointers so that Application.h doesn't
// have to be changed every time a new window class is added

#ifndef WCS_CONSERVATORY_H
#define WCS_CONSERVATORY_H

class ColorEditGUI;
class CloudEditGUI;
class WaveEditGUI;
class SunPosGUI;
class DBEditGUI;
class SceneExportGUI;
class EcotypeEditGUI;
class ProjectPrefsGUI;
class InterpDEMGUI;
class ProjNewGUI;
class DigitizeGUI;
class ViewGUI;
class VersionGUI;
class ProjUpdateGUI;
class AuthorizeGUI;
class DiagnosticGUI;
class SceneImportGUI;
class EffectsLibGUI;
class LakeEditGUI;
class EcosystemEditGUI;
class RasterTAEditGUI;
class TerraffectorEditGUI;
class ShadowEditGUI;
class FoliageEffectEditGUI;
class StreamEditGUI;
class Object3DEditGUI;
class MaterialEditGUI;
class LightEditGUI;
class CameraEditGUI;
class TerrainParamEditGUI;
class SkyEditGUI;
class CelestialEditGUI;
class VectorEditGUI;
class VectorProfileGUI;
class DefaultGUI;
class DBExportGUI;
class ImageLibGUI;
class AnimGraphGUI;
class TextureEditGUI;
class SceneViewGUI;
class DragnDropListGUI;
class AtmosphereEditGUI;
class CmapEditGUI;
class EnvironmentEditGUI;
class GroundEditGUI;
class PlanetOptEditGUI;
class RenderJobEditGUI;
class RenderOptEditGUI;
class SnowEditGUI;
class StarfieldEditGUI;
class MaterialStrataEditGUI;
class GalleryGUI;
class BrowseDataGUI;
class ImageViewGUI;
class RenderControlGUI;
class EffectListGUI;
class ImportWizGUI;
class KeyScaleDeleteGUI;
class NumericEntryGUI;
class TerraGridderEditGUI;
class TerraGeneratorEditGUI;
class SearchQueryEditGUI;
class ThematicMapEditGUI;
class CoordSysEditGUI;
class TableListGUI;
class DEMEditGUI;
class DEMPaintGUI;
class FenceEditGUI;
class TemplateManGUI;
class PathTransferGUI;
class EDSSControlGUI;
class PostProcEditGUI;
class DEMMergeGUI;
class ScenarioEditGUI;
class VecProfExportGUI;
class CoordsCalculatorGUI;
class DrillDownInfoGUI;
class ExporterEditGUI;
class ExportControlGUI;
class GridderWizGUI;
class ForestWizGUI;
class MergerWizGUI;
class LabelEditGUI;
class FoliageEffectFolFileEditGUI;
// <<<>>> ADD_NEW_EFFECTS the GUI class

class Conservatory
	{
	public:
		Conservatory() {CEG = NULL; TXG = NULL; RCG = NULL; NUM = NULL; TGN = NULL;
						CLG = NULL; WEG = NULL; ILG = NULL; DKG = NULL;
						DBE = NULL; LWG = NULL; MSG = NULL; IVG = NULL; SQU = NULL;
						FEG = NULL; PPG = NULL; GRD = NULL; DSS = NULL; PPR = NULL;
						IDG = NULL; PNG = NULL; DIG = NULL; FCG = NULL; PTH = NULL;
						CVG = NULL; VER = NULL; SAG = NULL; DEM = NULL; TPM = NULL;
						RDG = NULL; VEG = NULL; DBO = NULL; TBG = NULL;
						SIM = NULL; LEG = NULL; EFG = NULL; COS = NULL;
						ECG = NULL; RTG = NULL; TAG = NULL; SPG = NULL;
						SHG = NULL; NPG = NULL; CPG = NULL; TPG = NULL; KPG = NULL;
						LPG = NULL; FLG = NULL; SEG = NULL; BRD = NULL; GWZ = NULL;
						VPG = NULL; DFG = NULL; OEG = NULL; MAG = NULL; THM = NULL;
						AEG = NULL; CMG = NULL; ENG = NULL; GNG = NULL; POG = NULL;
						RJG = NULL; ROG = NULL; SNG = NULL; STG = NULL; LVE = NULL;
						EFL = NULL; IWG = NULL; DPG = NULL; MRG = NULL; SCN = NULL;
						VPX = NULL; CSC = NULL; DRL = NULL; EXP = NULL; EXG = NULL;
						AUT = NULL; FWZ = NULL; MWZ = NULL; LBL = NULL; FFG = NULL;
						PUG = NULL;

// <<<>>> ADD_NEW_EFFECTS NULL the pointer to the GUI instance
						DDL[0] = DDL[1] = DDL[2] = DDL[3] = DDL[4] = NULL;
						GRG[0] = GRG[1] = GRG[2] = GRG[3] = GRG[4] = NULL;
						GRG[5] = GRG[6] = GRG[7] = GRG[8] = GRG[9] = NULL;
						};

		// Actual destructor is in toolbar.cpp
		~Conservatory();

		ColorEditGUI *CEG;
		CloudEditGUI *CLG;
		WaveEditGUI *WEG;
		SunPosGUI *SPG;
		DBEditGUI *DBE;
		SceneExportGUI *LWG;
		EcotypeEditGUI *FEG;
		ProjectPrefsGUI *PPG;
		InterpDEMGUI *IDG;
		ProjNewGUI *PNG;
		DigitizeGUI *DIG;
		ViewGUI *CVG;
		VersionGUI *VER;
		ProjUpdateGUI *PUG;
		AuthorizeGUI *AUT;
		DiagnosticGUI *RDG;
		SceneImportGUI *SIM;
		LakeEditGUI *LEG;
		EcosystemEditGUI *ECG;
		RasterTAEditGUI *RTG;
		TerraffectorEditGUI *TAG;
		ShadowEditGUI *SHG;
		FoliageEffectEditGUI *FLG;
		StreamEditGUI *SEG;
		Object3DEditGUI *OEG;
		MaterialEditGUI *MAG;
		EffectsLibGUI *EFG;
		LightEditGUI *NPG;
		CameraEditGUI *CPG;
		TerrainParamEditGUI *TPG;
		SkyEditGUI *KPG;
		CelestialEditGUI *LPG;
		VectorEditGUI *VEG;
		VectorProfileGUI *VPG;
		DefaultGUI *DFG;
		DBExportGUI *DBO;
		ImageLibGUI *ILG;
		KeyScaleDeleteGUI *DKG;
		TextureEditGUI *TXG;
		SceneViewGUI *SAG;
		AtmosphereEditGUI *AEG;
		CmapEditGUI *CMG;
		EnvironmentEditGUI *ENG;
		GroundEditGUI *GNG;
		PlanetOptEditGUI *POG;
		RenderJobEditGUI *RJG;
		RenderOptEditGUI *ROG;
		SnowEditGUI *SNG;
		StarfieldEditGUI *STG;
		MaterialStrataEditGUI *MSG;
		GalleryGUI *LVE;
		BrowseDataGUI *BRD;
		RenderControlGUI *RCG;
		ImageViewGUI *IVG;
		EffectListGUI *EFL;
		ImportWizGUI *IWG;
		NumericEntryGUI *NUM;
		TerraGridderEditGUI *GRD;
		TerraGeneratorEditGUI *TGN;
		SearchQueryEditGUI *SQU;
		ThematicMapEditGUI *THM;
		CoordSysEditGUI *COS;
		TableListGUI *TBG;
		DEMEditGUI *DEM;
		DEMPaintGUI *DPG;
		FenceEditGUI *FCG;
		TemplateManGUI *TPM;
		PathTransferGUI *PTH;
		EDSSControlGUI *DSS;
		PostProcEditGUI *PPR;
		DEMMergeGUI *MRG;
		ScenarioEditGUI *SCN;
		VecProfExportGUI *VPX;
		CoordsCalculatorGUI *CSC;
		DrillDownInfoGUI *DRL;
		ExporterEditGUI *EXP;
		ExportControlGUI *EXG;
		GridderWizGUI *GWZ;
		ForestWizGUI *FWZ;
		MergerWizGUI *MWZ;
		LabelEditGUI *LBL;
		FoliageEffectFolFileEditGUI *FFG;
// <<<>>> ADD_NEW_EFFECTS add pointer to an instance of the GUI module
		DragnDropListGUI *DDL[5];
		AnimGraphGUI *GRG[10];

	}; // Conservatory

#endif // WCS_CONSERVATORY_H
