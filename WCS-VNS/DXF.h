// DXF.h

//class Project;
//class Database;
//class MessageLog;
class ImportWizGUI;
struct ImportData;

#include "Application.h"
#include "Database.h"
#include "Fenetre.h"
#include "ImportThing.h"
#include "Points.h"
#include "NNGrid.h"
#include "Requester.h"
#include "EffectsLib.h"
#include "Log.h"
#include "Exports.h"
#include "ImportWizGUI.h"

// These are no longer minions of the ImportWiz or Database object.
VectorPoint *IncreaseCopyPointAllocation(VectorPoint *OldPoints, unsigned long &NumPts, VectorPoint *&CurrentPoint);
DXFImportPolygon *IncreaseCopyPolygonAllocation(DXFImportPolygon *OldPolys, unsigned long &NumPolys);
Joe *PrepnSaveDXFObject(Joe *FileParent, Joe *AfterGuy, ImportHook *IH, struct ImportData *ScaleHook,
	VectorPoint *Points, char *ObjName, char *ObjLayer, unsigned long NumPts, float DefaultElev, short ElevDataValid,
	short RefSys, char ConnectPoints, short ElevUnits, short ReverseLon, double *MaxBounds, ImportWizGUI *IWG,
	struct DXFLayerPens *LayerPens = NULL, short PenNum = 2);
Joe *SetDXFObject(char *Name, long Pairs, char Style, unsigned char Color, VectorPoint *Points, ImportWizGUI *IWG);
static void SetObjectLayer(Joe *AverageGuy, char *Layer);
static void SetDXFObjectOneLayer(Joe *AverageGuy, char *Layer);
#ifdef FOOBAR
BOOL TestForUTM(double *MaxVal, double *MinVal);
#endif // FOOBAR
unsigned long Import3DObjDXF(FILE *fDXF, Joe *FileParent, ImportHook *IH, Object3DEffect *Hook3D);
unsigned long ImportGISDXF(FILE *fDXF, Joe *FileParent, ImportHook *IH, struct ImportData *ScaleHook, Object3DEffect *Hook3D, ImportWizGUI *IWG);

