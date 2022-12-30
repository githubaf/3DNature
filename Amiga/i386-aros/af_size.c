#ifdef __AROS__
#define USHORT unsigned short
#define SHORT short
//#define EXTERN
#define __far
#define __chip
#endif

#include "WCS.h"

int main(void)
{

 KPrintF("AF: sizeof(struct AlbLatLonCoords)=%ld\n",sizeof(struct AlbLatLonCoords));
 KPrintF("AF: sizeof(struct AnimWindow)=%ld\n",sizeof(struct AnimWindow));
 KPrintF("AF: sizeof(struct Animation)=%ld\n",sizeof(struct Animation));
 KPrintF("AF: sizeof(struct AnimationV1)=%ld\n",sizeof(struct AnimationV1));
 KPrintF("AF: sizeof(struct BitmapImage)=%ld\n",sizeof(struct BitmapImage));
 KPrintF("AF: sizeof(struct Box)=%ld\n",sizeof(struct Box));
 KPrintF("AF: sizeof(struct BusyWindow)=%ld\n",sizeof(struct BusyWindow));
 KPrintF("AF: sizeof(struct CloudData)=%ld\n",sizeof(struct CloudData));
 KPrintF("AF: sizeof(struct CloudData2)=%ld\n",sizeof(struct CloudData2));
 KPrintF("AF: sizeof(struct CloudKey)=%ld\n",sizeof(struct CloudKey));
 KPrintF("AF: sizeof(struct CloudLayer)=%ld\n",sizeof(struct CloudLayer));
 KPrintF("AF: sizeof(struct CloudWindow)=%ld\n",sizeof(struct CloudWindow));
 KPrintF("AF: sizeof(struct Color)=%ld\n",sizeof(struct Color));
 KPrintF("AF: sizeof(struct ColorComponents)=%ld\n",sizeof(struct ColorComponents));
 KPrintF("AF: sizeof(struct ColorKey)=%ld\n",sizeof(struct ColorKey));
 KPrintF("AF: sizeof(struct ColorShift)=%ld\n",sizeof(struct ColorShift));
 KPrintF("AF: sizeof(struct ColorV1)=%ld\n",sizeof(struct ColorV1));
 KPrintF("AF: sizeof(struct Color_Map)=%ld\n",sizeof(struct Color_Map));
 KPrintF("AF: sizeof(struct DEMConvertData)=%ld\n",sizeof(struct DEMConvertData));
 KPrintF("AF: sizeof(struct DEMConvertWindow)=%ld\n",sizeof(struct DEMConvertWindow));
 KPrintF("AF: sizeof(struct DEMData)=%ld\n",sizeof(struct DEMData));
 KPrintF("AF: sizeof(struct DEMExtractData)=%ld\n",sizeof(struct DEMExtractData));
 KPrintF("AF: sizeof(struct DEMExtractWindow)=%ld\n",sizeof(struct DEMExtractWindow));
 KPrintF("AF: sizeof(struct DEMInfo)=%ld\n",sizeof(struct DEMInfo));
 KPrintF("AF: sizeof(struct DEMInterpolateData)=%ld\n",sizeof(struct DEMInterpolateData));
 KPrintF("AF: sizeof(struct DEMInterpolateWindow)=%ld\n",sizeof(struct DEMInterpolateWindow));
 KPrintF("AF: sizeof(struct DLGInfo)=%ld\n",sizeof(struct DLGInfo));
 KPrintF("AF: sizeof(struct DataOpsModWindow)=%ld\n",sizeof(struct DataOpsModWindow));
 KPrintF("AF: sizeof(struct DatabaseEditWindow)=%ld\n",sizeof(struct DatabaseEditWindow));
 KPrintF("AF: sizeof(struct DatabaseModWindow)=%ld\n",sizeof(struct DatabaseModWindow));
 KPrintF("AF: sizeof(struct DiagnosticWindow)=%ld\n",sizeof(struct DiagnosticWindow));
 KPrintF("AF: sizeof(struct DirList)=%ld\n",sizeof(struct DirList));
 KPrintF("AF: sizeof(struct DirListWindow)=%ld\n",sizeof(struct DirListWindow));
 KPrintF("AF: sizeof(struct EcoLegendWindow)=%ld\n",sizeof(struct EcoLegendWindow));
 KPrintF("AF: sizeof(struct EcoPalWindow)=%ld\n",sizeof(struct EcoPalWindow));
 KPrintF("AF: sizeof(struct Ecosystem)=%ld\n",sizeof(struct Ecosystem));
 KPrintF("AF: sizeof(struct Ecosystem2)=%ld\n",sizeof(struct Ecosystem2));
 KPrintF("AF: sizeof(struct Ecosystem2V1)=%ld\n",sizeof(struct Ecosystem2V1));
 KPrintF("AF: sizeof(struct EcosystemKey)=%ld\n",sizeof(struct EcosystemKey));
 KPrintF("AF: sizeof(struct EcosystemKey2)=%ld\n",sizeof(struct EcosystemKey2));
 KPrintF("AF: sizeof(struct EcosystemKey2V1)=%ld\n",sizeof(struct EcosystemKey2V1));
 KPrintF("AF: sizeof(struct EcosystemKeyV1)=%ld\n",sizeof(struct EcosystemKeyV1));
 KPrintF("AF: sizeof(struct EcosystemShift)=%ld\n",sizeof(struct EcosystemShift));
 KPrintF("AF: sizeof(struct EcosystemShiftV1)=%ld\n",sizeof(struct EcosystemShiftV1));
 KPrintF("AF: sizeof(struct EcosystemV1)=%ld\n",sizeof(struct EcosystemV1));
 KPrintF("AF: sizeof(struct EcosystemWindow)=%ld\n",sizeof(struct EcosystemWindow));
 KPrintF("AF: sizeof(struct EditModWindow)=%ld\n",sizeof(struct EditModWindow));
 KPrintF("AF: sizeof(struct FaceData)=%ld\n",sizeof(struct FaceData));
 KPrintF("AF: sizeof(struct FocusProfiles)=%ld\n",sizeof(struct FocusProfiles));
 KPrintF("AF: sizeof(struct FoliageWindow)=%ld\n",sizeof(struct FoliageWindow));
 KPrintF("AF: sizeof(struct ForestModel)=%ld\n",sizeof(struct ForestModel));
 KPrintF("AF: sizeof(struct ForestModelWindow)=%ld\n",sizeof(struct ForestModelWindow));
 KPrintF("AF: sizeof(struct GUIKeyStuff)=%ld\n",sizeof(struct GUIKeyStuff));
 KPrintF("AF: sizeof(struct Gauss)=%ld\n",sizeof(struct Gauss));
 KPrintF("AF: sizeof(struct GeoPoint)=%ld\n",sizeof(struct GeoPoint));
 KPrintF("AF: sizeof(struct IAMotionWindow)=%ld\n",sizeof(struct IAMotionWindow));
 KPrintF("AF: sizeof(struct ILBMHeader)=%ld\n",sizeof(struct ILBMHeader));
 KPrintF("AF: sizeof(struct InterAction)=%ld\n",sizeof(struct InterAction));
 KPrintF("AF: sizeof(struct InterpPoints)=%ld\n",sizeof(struct InterpPoints));
 KPrintF("AF: sizeof(struct KeyTable)=%ld\n",sizeof(struct KeyTable));
 KPrintF("AF: sizeof(struct LightWaveIOWindow)=%ld\n",sizeof(struct LightWaveIOWindow));
 KPrintF("AF: sizeof(struct LightWaveInfo)=%ld\n",sizeof(struct LightWaveInfo));
 KPrintF("AF: sizeof(struct LightWaveMotion)=%ld\n",sizeof(struct LightWaveMotion));
 KPrintF("AF: sizeof(struct MakeDEMWindow)=%ld\n",sizeof(struct MakeDEMWindow));
 KPrintF("AF: sizeof(struct MapCoords)=%ld\n",sizeof(struct MapCoords));
 KPrintF("AF: sizeof(struct MapData)=%ld\n",sizeof(struct MapData));
 KPrintF("AF: sizeof(struct MaxMin3)=%ld\n",sizeof(struct MaxMin3));
 KPrintF("AF: sizeof(struct MinimumDistance)=%ld\n",sizeof(struct MinimumDistance));
 KPrintF("AF: sizeof(struct Motion)=%ld\n",sizeof(struct Motion));
 KPrintF("AF: sizeof(struct MotionKey)=%ld\n",sizeof(struct MotionKey));
 KPrintF("AF: sizeof(struct MotionKey2)=%ld\n",sizeof(struct MotionKey2));
 KPrintF("AF: sizeof(struct MotionShift)=%ld\n",sizeof(struct MotionShift));
 KPrintF("AF: sizeof(struct MotionV1)=%ld\n",sizeof(struct MotionV1));
 KPrintF("AF: sizeof(struct MotionWindow)=%ld\n",sizeof(struct MotionWindow));
 KPrintF("AF: sizeof(struct NNGrid)=%ld\n",sizeof(struct NNGrid));
 KPrintF("AF: sizeof(struct NNGridWindow)=%ld\n",sizeof(struct NNGridWindow));
 KPrintF("AF: sizeof(struct NewProjectWindow)=%ld\n",sizeof(struct NewProjectWindow));
 KPrintF("AF: sizeof(struct NoLinearColorKey)=%ld\n",sizeof(struct NoLinearColorKey));
 KPrintF("AF: sizeof(struct NoLinearEcosystemKey)=%ld\n",sizeof(struct NoLinearEcosystemKey));
 KPrintF("AF: sizeof(struct NoLinearEcosystemKey2)=%ld\n",sizeof(struct NoLinearEcosystemKey2));
 KPrintF("AF: sizeof(struct NoLinearMotionKey)=%ld\n",sizeof(struct NoLinearMotionKey));
 KPrintF("AF: sizeof(struct OldEcosystemV1)=%ld\n",sizeof(struct OldEcosystemV1));
 KPrintF("AF: sizeof(struct Palette)=%ld\n",sizeof(struct Palette));
 KPrintF("AF: sizeof(struct PaletteItem)=%ld\n",sizeof(struct PaletteItem));
 KPrintF("AF: sizeof(struct PaletteV1)=%ld\n",sizeof(struct PaletteV1));
 KPrintF("AF: sizeof(struct ParHeader)=%ld\n",sizeof(struct ParHeader));
 KPrintF("AF: sizeof(struct ParHeaderV1)=%ld\n",sizeof(struct ParHeaderV1));
 KPrintF("AF: sizeof(struct ParListWindow)=%ld\n",sizeof(struct ParListWindow));
 KPrintF("AF: sizeof(struct PrefsWindow)=%ld\n",sizeof(struct PrefsWindow));
 KPrintF("AF: sizeof(struct ProjectWindow)=%ld\n",sizeof(struct ProjectWindow));
 KPrintF("AF: sizeof(struct QCvalues)=%ld\n",sizeof(struct QCvalues));
 KPrintF("AF: sizeof(struct RenderAnim)=%ld\n",sizeof(struct RenderAnim));
 KPrintF("AF: sizeof(struct ScaleImageWindow)=%ld\n",sizeof(struct ScaleImageWindow));
 KPrintF("AF: sizeof(struct ScaleKeyInfo)=%ld\n",sizeof(struct ScaleKeyInfo));
 KPrintF("AF: sizeof(struct ScaleWindow)=%ld\n",sizeof(struct ScaleWindow));
 KPrintF("AF: sizeof(struct Settings)=%ld\n",sizeof(struct Settings));
 KPrintF("AF: sizeof(struct SettingsV1)=%ld\n",sizeof(struct SettingsV1));
 KPrintF("AF: sizeof(struct SettingsWindow)=%ld\n",sizeof(struct SettingsWindow));
 KPrintF("AF: sizeof(struct SmallWindow)=%ld\n",sizeof(struct SmallWindow));
 KPrintF("AF: sizeof(struct StatusLogWindow)=%ld\n",sizeof(struct StatusLogWindow));
 KPrintF("AF: sizeof(struct TerrainControl)=%ld\n",sizeof(struct TerrainControl));
 KPrintF("AF: sizeof(struct TimeLineWindow)=%ld\n",sizeof(struct TimeLineWindow));
 KPrintF("AF: sizeof(struct TimeSetWindow)=%ld\n",sizeof(struct TimeSetWindow));
 KPrintF("AF: sizeof(struct TreeModel)=%ld\n",sizeof(struct TreeModel));
 KPrintF("AF: sizeof(struct USGS_DEMHeader)=%ld\n",sizeof(struct USGS_DEMHeader));
 KPrintF("AF: sizeof(struct USGS_DEMProfileHeader)=%ld\n",sizeof(struct USGS_DEMProfileHeader));
 KPrintF("AF: sizeof(struct UTMLatLonCoords)=%ld\n",sizeof(struct UTMLatLonCoords));
 KPrintF("AF: sizeof(struct VelocityDistr)=%ld\n",sizeof(struct VelocityDistr));
 KPrintF("AF: sizeof(struct Vertex)=%ld\n",sizeof(struct Vertex));
 KPrintF("AF: sizeof(struct VertexIndex)=%ld\n",sizeof(struct VertexIndex));
 KPrintF("AF: sizeof(struct ViewCenter)=%ld\n",sizeof(struct ViewCenter));
 KPrintF("AF: sizeof(struct Viewshed)=%ld\n",sizeof(struct Viewshed));
 KPrintF("AF: sizeof(struct WCSApp)=%ld\n",sizeof(struct WCSApp));
 KPrintF("AF: sizeof(struct WCSScreenData)=%ld\n",sizeof(struct WCSScreenData));
 KPrintF("AF: sizeof(struct WCSScreenMode)=%ld\n",sizeof(struct WCSScreenMode));
 KPrintF("AF: sizeof(struct Wave)=%ld\n",sizeof(struct Wave));
 KPrintF("AF: sizeof(struct WaveData)=%ld\n",sizeof(struct WaveData));
 KPrintF("AF: sizeof(struct WaveData2)=%ld\n",sizeof(struct WaveData2));
 KPrintF("AF: sizeof(struct WaveKey)=%ld\n",sizeof(struct WaveKey));
 KPrintF("AF: sizeof(struct WcsBitMapHeader)=%ld\n",sizeof(struct WcsBitMapHeader));
 KPrintF("AF: sizeof(struct WindowKeyStuff)=%ld\n",sizeof(struct WindowKeyStuff));
 KPrintF("AF: sizeof(struct ZBufferHeader)=%ld\n",sizeof(struct ZBufferHeader));
 KPrintF("AF: sizeof(struct boundingbox)=%ld\n",sizeof(struct boundingbox));
 KPrintF("AF: sizeof(struct clipbounds)=%ld\n",sizeof(struct clipbounds));
 KPrintF("AF: sizeof(struct coords)=%ld\n",sizeof(struct coords));
 KPrintF("AF: sizeof(struct database)=%ld\n",sizeof(struct database));
 KPrintF("AF: sizeof(struct datum)=%ld\n",sizeof(struct datum));
 KPrintF("AF: sizeof(struct elmapheaderV101)=%ld\n",sizeof(struct elmapheaderV101));
 KPrintF("AF: sizeof(struct faces)=%ld\n",sizeof(struct faces));
 KPrintF("AF: sizeof(struct lineseg)=%ld\n",sizeof(struct lineseg));
 KPrintF("AF: sizeof(struct neig)=%ld\n",sizeof(struct neig));
 KPrintF("AF: sizeof(struct poly3)=%ld\n",sizeof(struct poly3));
 KPrintF("AF: sizeof(struct poly4)=%ld\n",sizeof(struct poly4));
 KPrintF("AF: sizeof(struct simp)=%ld\n",sizeof(struct simp));
 KPrintF("AF: sizeof(struct temp)=%ld\n",sizeof(struct temp));
 KPrintF("AF: sizeof(struct vectorheaderV100)=%ld\n",sizeof(struct vectorheaderV100));


 KPrintF("AF: sizeof(union Environment)=%ld\n",sizeof(union Environment));
 KPrintF("AF: sizeof(union EnvironmentV1)=%ld\n",sizeof(union EnvironmentV1));
 KPrintF("AF: sizeof(union KeyFrame)=%ld\n",sizeof(union KeyFrame));
 KPrintF("AF: sizeof(union KeyFrameV1)=%ld\n",sizeof(union KeyFrameV1));
 KPrintF("AF: sizeof(union MapProjection)=%ld\n",sizeof(union MapProjection));
 KPrintF("AF: sizeof(union NoLinearKeyFrame)=%ld\n",sizeof(union NoLinearKeyFrame));

	return 0;
}
