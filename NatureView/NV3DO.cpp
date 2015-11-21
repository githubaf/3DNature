// NV3DO.cpp
// 3D Object instance loader
// created by CXH on 11/17/05 from code from main.cpp

#include <time.h>
#include <sstream>

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osg/ShapeDrawable>


#include "NV3DO.h"
#include "PartialVertexDEM.h"
#include "Types.h"
#include "NVAnimObject.h"
#include "NVScene.h"
#include "ExtentsVisitor.h"
#include "SwitchBox.h"
#include "NVSigCheck.h"
#include "NVNodeMasks.h"


extern NVScene MasterScene;
extern SwitchBox *MasterSwitches;


osg::Node *CreateObj(NVAnimObject *ObjInstance, unsigned long int ObjVerifiedCount, double ObjClipDist, double ObjBoxDist)
{
osg::MatrixTransform * ModelTransform = new osg::MatrixTransform;
osg::Matrix RotMat;
PartialVertexDEM Location;
Point3f Scale;
bool EnableNormalize = false;

#ifdef NV_CHECK_SIGNATURES
double VerifyProbability, VerifyTest;
int VerifyResult;

VerifyProbability = 1.0 / (double)ObjVerifiedCount;
VerifyResult = rand();
VerifyTest = (double)VerifyResult / (double)RAND_MAX;

if(VerifyTest < VerifyProbability) // Should we do a verify? (probability decreases for every additional object)
	{
	if(!CheckDependentBinaryFileSignature(ObjInstance->GetOverallSig(), ObjInstance->GetName()))
		{
		SetGlobalSigInvalid(true);
		return(NULL);
		} // if
	} // if
#endif // NV_CHECK_SIGNATURES


Location.Lat  = ObjInstance->GetChannelValueTime(NVAnimObject::CANONY, 0.0f);
Location.Lon  = ObjInstance->GetChannelValueTime(NVAnimObject::CANONX, 0.0f);
Location.Elev = ObjInstance->GetChannelValueTime(NVAnimObject::CANONZ, 0.0f);

MasterScene.DegToCart(Location);

ModelTransform->setDataVariance(osg::Object::STATIC);

/*
RotMat = 
	osg::Matrix::rotate(osg::inDegrees(-ObjInstance->GetChannelValueTime(NVAnimObject::CANONB, 0.0f)),1.0f,0.0f,0.0f) * // sense (+- of bank?) and axis?
	osg::Matrix::rotate(osg::inDegrees(ObjInstance->GetChannelValueTime(NVAnimObject::CANONP, 0.0f)),0.0f,1.0f,0.0f) * // sense (+- of pitch?) and axis?
	osg::Matrix::rotate(osg::inDegrees(-ObjInstance->GetChannelValueTime(NVAnimObject::CANONH, 0.0f)),0.0f,0.0f,1.0f);  // sense (+- of heading?) and axis?
*/

double AxisX, AxisY, AxisZ, AxisAngle;

AxisX = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAXISX, 0.0f);
AxisY = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAXISY, 0.0f);
AxisZ = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAXISZ, 0.0f);
AxisAngle = ObjInstance->GetChannelValueTime(NVAnimObject::CANONB, 0.0f);

if(AxisX + AxisY + AxisZ + AxisAngle == 0.0)
	{
	AxisX = 1.0;
	} // if

RotMat = osg::Matrix::rotate(osg::DegreesToRadians(180.0f), 0.0f, 0.0f, 1.0f);  // sense (+- of angle?) and axis (stored as bank)?

RotMat *= osg::Matrix::rotate(AxisAngle, AxisX, -AxisY, AxisZ);  // sense (+- of angle?) and axis (stored as bank)?


Scale[0] = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAUXX, 0.0f);
Scale[1] = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAUXY, 0.0f);
Scale[2] = ObjInstance->GetChannelValueTime(NVAnimObject::CANONAUXZ, 0.0f);

if(Scale[0] == 0.0 && Scale[1] == 0.0 && Scale[2] == 0.0)
	{
	Scale[0] = 1.0;
	Scale[1] = 1.0;
	Scale[2] = 1.0;
	} // if

if(Scale[0] != 1.0 || Scale[1] != 1.0 || Scale[2] != 1.0)
	{
	EnableNormalize = true;
	} // if

ModelTransform->setMatrix(osg::Matrix::scale(Scale[0], Scale[1], Scale[2]) * RotMat *
	osg::Matrix::translate(Location.XYZ[0],Location.XYZ[1],Location.XYZ[2]));

osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(ObjInstance->GetName());
if(loadedModel.valid())
	{
	if(EnableNormalize)
		{
		osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
		stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
		}// if

	if(ObjClipDist < FLT_MAX || ObjBoxDist < FLT_MAX)
		{ // add LOD node
		osg::LOD* LODNode = new osg::LOD;
		ModelTransform->addChild(loadedModel.get());

		if(ObjBoxDist < ObjClipDist)
			{ // add intermediate grey boundingbox version
			float Radius = 10000;
			ExtentsVisitor ev;
			osg::Vec3 BBCenter;

			loadedModel->accept(ev);

			osg::BoundingBox extents = ev.GetBound();
			BBCenter = extents.center();

			osg::Geode* geode = new osg::Geode();
			osg::TessellationHints* hints = new osg::TessellationHints;
			hints->setDetailRatio(0.5f);
			geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(Location.XYZ[0] + BBCenter.x(),Location.XYZ[1] + BBCenter.y(),Location.XYZ[2] + BBCenter.z()),
			Scale[0] * (extents.xMax() - extents.xMin()),
			Scale[1] * (extents.yMax() - extents.yMin()),
			Scale[2] * (extents.zMax() - extents.zMin())),hints));
			// add the box object
			LODNode->addChild(geode, ObjBoxDist, ObjClipDist);
			// add the real object
			LODNode->addChild(ModelTransform, 0, ObjBoxDist);
			} // if
		else
			{
			LODNode->addChild(ModelTransform, 0, ObjClipDist);
			} // else

		return(LODNode);
		} // if
	else
		{
		ModelTransform->addChild(loadedModel.get());
		return(ModelTransform);
		} // else
	} // if
else
	{
	// didn't load, fail silently
	} // else

return(NULL);
} // CreateObj


static unsigned long int ObjVerifiedCount; // only visible in this file

void InitObjectCreationState(void)
{
// for object verification probability randomization
ObjVerifiedCount = 1; // never start at 0
time_t now;
time(&now);
srand(now);
} // InitObjectCreationState


int CreateObjects(double MinXCoord, double MaxXCoord, double MinYCoord, double MaxYCoord, osg::Group *ObjVecParent)
{
int ObjectsCreated = 0;
PartialVertexDEM Location;
osg::Group* ObjGroup = NULL;
osg::Group* VecGroup = NULL;
double ObjClipDist, ObjBoxDist;

ObjClipDist = MasterScene.SceneLOD.GetObjClip();
ObjBoxDist = MasterScene.SceneLOD.GetObjBox();

// maybe should use a real iterator here
for(int InstLoop = 0; InstLoop < MasterScene.GetNum3DOInstances(); InstLoop++)
	{
	NVAnimObject *CurInst;
	osg::Node *CurNode;

	CurInst = MasterScene.Get3DOInstance(InstLoop);
	Location.Lat  = CurInst->GetChannelValueTime(NVAnimObject::CANONY, 0.0f);
	Location.Lon  = CurInst->GetChannelValueTime(NVAnimObject::CANONX, 0.0f);
	if(Location.Lat < MaxYCoord && Location.Lat >= MinYCoord && Location.Lon < MaxXCoord && Location.Lon >= MinXCoord)
		{
		if(CurNode = CreateObj(CurInst, ObjVerifiedCount, ObjClipDist, ObjBoxDist))
			{
			ObjectsCreated++;
			ObjVerifiedCount++; // to make signature check skipping accelerate the more objects we have
			// is it a vector? Vectors look like VO000000.3ds
			const char *Name;
			int NameLen;
			bool IsVec = false;
			Name = CurInst->GetName();
			NameLen = strlen(Name);
			
			#ifdef NVW_SUPPORT_QUERYACTION
			// Add ACTIONID data for Query/Action
			if(CurInst->CheckGenericIntValuePresent(NVAnimObject::ACTIONID))
				{
				unsigned long int ActionID;
				// ActionID can be 0 and still be valid
				ActionID = CurInst->GetGenericIntValue(NVAnimObject::ACTIONID);
				std::ostringstream TempNumberAsString;
				TempNumberAsString << ActionID << std::endl; // just like sprintf %d
				CurNode->addDescription(TempNumberAsString.str());
				} // if
			#endif // NVW_SUPPORT_QUERYACTION
			
			if(NameLen >= 12) // VO000000.3ds
				{
				if(Name[NameLen - 12] == 'V' && Name[NameLen - 11] == 'O' && isdigit(Name[NameLen - 10]))
					{ // name looks like a vector to me, add it to the Vector group where it will become fully lit
					if(!VecGroup)
						{
						if(VecGroup = new osg::Group())
							{
							VecGroup->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_VECTOR);
							VecGroup->setStateSet(MasterSwitches->GetSharedVecLightState());
							} // if
						} // if
					if(VecGroup)
						{
						VecGroup->addChild(CurNode);
						} // if
					else
						{ // error, dispose
						//delete CurNode;
						} // else
					IsVec = true;
					} // if
				} // if
			if(!IsVec)
				{
				if(!ObjGroup)
					{
					if(ObjGroup = new osg::Group())
						{
						ObjGroup->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_STRUCT);
						ObjGroup->setStateSet(MasterSwitches->GetSharedObjLightState());
						} // if
					} // if
				if(ObjGroup)
					{
					ObjGroup->addChild(CurNode);
					} // if
				else
					{ // error, dispose
					//delete CurNode;
					} // else
				} // if
			} // if
		} // if
	} // for

if(ObjGroup)
	{
	ObjVecParent->addChild(ObjGroup);
	} // if
if(VecGroup)
	{
	ObjVecParent->addChild(VecGroup);
	} // if

return(ObjectsCreated);
} // CreateObjects
