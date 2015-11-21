// NVLights.cpp
// Lighting code, broken out of main.cpp on 1/31/06 by CXH

#include "NVLights.h"



#include <osg/Light>
#include <osg/LightSource>


#include "NVAnimObject.h"
#include "NVScene.h"

extern NVScene MasterScene;
extern osg::ref_ptr<osg::Group> LightGroup;
extern osg::ref_ptr<osg::StateSet> LightingStateSet;

bool BuildLights(void)
{

LightGroup = new osg::Group;
if(MasterScene.GetNumLights() == 0)
	{
	// if we didn't get any lights, we need to create a default lighting rig
	CreateDefaultLights();
	} // if
if(MasterScene.GetNumLights() > 0)
	{
	// create Lighting stateset for non-vector objects
	LightingStateSet = new osg::StateSet;
	LightingStateSet->setGlobalDefaults();

	CreateLights(LightGroup.get(), LightingStateSet.get());
	} // else

return(true);
} // BuildLights



// this doesn't create the default lights themselves, it just pushes them into the scene so CreateLights will build them
int CreateDefaultLights(void)
{
NVAnimObject *CurAnimSubObject;
MiniKeyFrame *KF;

if(CurAnimSubObject = new NVAnimObject)
	{
	CurAnimSubObject->Type = NVAnimObject::LIGHT;
	CurAnimSubObject->SetEnabled(true);
	CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTPARALLEL);
	if(KF = new MiniKeyFrame)
		{
		KF->SetTimeValue(0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISX, 0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISY, 0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISZ, -1.0f); // straight down
		KF->SetChannelValue(NVAnimObject::CANONINTENSITYA, 1.0); // full intensity
		KF->SetChannelValue(NVAnimObject::CANONCOLORR, 1.0f); //R
		KF->SetChannelValue(NVAnimObject::CANONCOLORG, 1.0f); //G
		KF->SetChannelValue(NVAnimObject::CANONCOLORB, 1.0f); //B
		CurAnimSubObject->AddKeyFrame(KF);
		} // if
	MasterScene.Lights.push_back(CurAnimSubObject);
	} // if

if(CurAnimSubObject = new NVAnimObject)
	{
	CurAnimSubObject->Type = NVAnimObject::LIGHT;
	CurAnimSubObject->SetEnabled(true);
	CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTPARALLEL);
	if(KF = new MiniKeyFrame)
		{
		KF->SetTimeValue(0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISX, 0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISY, 0.7071068f); // 7071068 is squareroot of 1/2
		KF->SetChannelValue(NVAnimObject::CANONAXISZ, 0.7071068f); // angled up
		KF->SetChannelValue(NVAnimObject::CANONINTENSITYA, 0.3f); // lower intensity
		KF->SetChannelValue(NVAnimObject::CANONCOLORR, 1.0f); //R
		KF->SetChannelValue(NVAnimObject::CANONCOLORG, 1.0f); //G
		KF->SetChannelValue(NVAnimObject::CANONCOLORB, 1.0f); //B
		CurAnimSubObject->AddKeyFrame(KF);
		} // if
	MasterScene.Lights.push_back(CurAnimSubObject);
	} // if

if(CurAnimSubObject = new NVAnimObject)
	{
	CurAnimSubObject->Type = NVAnimObject::LIGHT;
	CurAnimSubObject->SetEnabled(true);
	CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTPARALLEL);
	if(KF = new MiniKeyFrame)
		{
		KF->SetTimeValue(0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISX, 0.5773503f); // .5773503 is squareroot of 1/3
		KF->SetChannelValue(NVAnimObject::CANONAXISY, -0.5773503f);
		KF->SetChannelValue(NVAnimObject::CANONAXISZ, 0.5773503f); // angled up
		KF->SetChannelValue(NVAnimObject::CANONINTENSITYA, 0.3f); // lower intensity
		KF->SetChannelValue(NVAnimObject::CANONCOLORR, 1.0f); //R
		KF->SetChannelValue(NVAnimObject::CANONCOLORG, 1.0f); //G
		KF->SetChannelValue(NVAnimObject::CANONCOLORB, 1.0f); //B
		CurAnimSubObject->AddKeyFrame(KF);
		} // if
	MasterScene.Lights.push_back(CurAnimSubObject);
	} // if

if(CurAnimSubObject = new NVAnimObject)
	{
	CurAnimSubObject->Type = NVAnimObject::LIGHT;
	CurAnimSubObject->SetEnabled(true);
	CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTPARALLEL);
	if(KF = new MiniKeyFrame)
		{
		KF->SetTimeValue(0.0f);
		KF->SetChannelValue(NVAnimObject::CANONAXISX, -0.5773503f); // .5773503 is squareroot of 1/3
		KF->SetChannelValue(NVAnimObject::CANONAXISY, -0.5773503f);
		KF->SetChannelValue(NVAnimObject::CANONAXISZ, 0.5773503f); // angled up
		KF->SetChannelValue(NVAnimObject::CANONINTENSITYA, 0.3f); // lower intensity
		KF->SetChannelValue(NVAnimObject::CANONCOLORR, 1.0f); //R
		KF->SetChannelValue(NVAnimObject::CANONCOLORG, 1.0f); //G
		KF->SetChannelValue(NVAnimObject::CANONCOLORB, 1.0f); //B
		CurAnimSubObject->AddKeyFrame(KF);
		} // if
	MasterScene.Lights.push_back(CurAnimSubObject);
	} // if


return(1);
} // CreateDefaultLights



int CreateLights(osg::Group *LightParent, osg::StateSet *LightingStateSet)
{
// for some reason, light slot 1 is misbehaving for us, so we start at 2
int LightsCreated = 2, AmbientLights = 0;
osg::Vec4 SkyAmbient(0.0, 0.0, 0.0, 1.0), GroundAmbient(0.0, 0.0, 0.0, 1.0);
/*
int RealLights = 0;

// determine if we have any non-ambient lights
for(LightList::iterator LightWalker = MasterScene.Lights.begin(); LightWalker != MasterScene.Lights.end(); ++LightWalker)
	{
	if(((*LightWalker)->GetGenericIntValue(NVAnimObject::LightTypeGenericIntDefs::LIGHTTYPE) == NVAnimObject::LightTypeGenericInt::LIGHTPARALLEL) ||
	 ((*LightWalker)->GetGenericIntValue(NVAnimObject::LightTypeGenericIntDefs::LIGHTTYPE) == NVAnimObject::LightTypeGenericInt::LIGHTSPOT) ||
	 ((*LightWalker)->GetGenericIntValue(NVAnimObject::LightTypeGenericIntDefs::LIGHTTYPE) == NVAnimObject::LightTypeGenericInt::LIGHTOMNI))
		{
		RealLights++;
		} // if
	} // for
*/

//if(RealLights)
if(1)
	{
	// enable lighting by default
	LightingStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	for(LightList::iterator LightWalker = MasterScene.Lights.begin(); LightWalker != MasterScene.Lights.end(); ++LightWalker)
		{
		osg::Vec4 SingleLight;
		double LightIntensity;
		double LightFalloffExp;
		LightIntensity = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONINTENSITYA, 0.0);
		LightFalloffExp = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONINTENSITYB, 0.0);
		if(LightIntensity > 0.0)
			{
			SingleLight.set((*LightWalker)->GetChannelValueTime(NVAnimObject::CANONCOLORR, 0.0), (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONCOLORG, 0.0), (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONCOLORB, 0.0), 0.0);
			SingleLight *= LightIntensity;

			if(((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTPARALLEL) ||
			 ((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTSPOT) ||
			 ((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTOMNI))
				{
				// install SingleLight into a osg Light
				PartialVertexDEM LightLoc;
				osg::Light* RealLight = new osg::Light;
				RealLight->setLightNum(LightsCreated);
				
				// handle Falloff Exponent as best we can
				// code taken from ViewGUI.cpp

				if(LightFalloffExp == 0.0) // no falloff
					{
					RealLight->setConstantAttenuation(1.0f);
					RealLight->setQuadraticAttenuation(0.0f);
					} // if
				else if((LightFalloffExp > 0.0) && (LightFalloffExp <= 2.0))
					{
					// GL can't modify the falloff exponent like WCS can, so we have
					// to hack it. This ViewGUI code sort of kuldge/hack/fakes the
					// equation 1/D^n (where n=[0.0...2.0] range) as 1/((n/2)*(D^2))
					// The error is most at n=(slightly greater than 0), and diminishes
					// to a perfect result where n=2.
					RealLight->setQuadraticAttenuation((float)(LightFalloffExp / 2.0));
					RealLight->setConstantAttenuation(0.0f);
					} // else if
				else
					{ // exponent greater than 2.0
					// GL can't modify the falloff exponent like WCS can, so we have
					// to hack it. This ViewGUI code sort of kuldge/hack/fakes the
					// equation 1/D^n (where n=[2.0...] range) as 1/(((n - 1.0)^2)*(D^2))
					// There is no error at n=2, and the error increases as n > 2.
					RealLight->setQuadraticAttenuation((float)((LightFalloffExp - 1.0) * (LightFalloffExp - 1.0)) );
					RealLight->setConstantAttenuation(0.0f);
					} // else
				
				// are we a spot/omni or parallel light
				if(((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) != NVAnimObject::LIGHTPARALLEL))
					{ // spot/omni
					float spot_cutoff = 180.0f; // default to omni
					LightLoc.Lat  = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONY, 0.0f);
					LightLoc.Lon  = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONX, 0.0f);
					LightLoc.Elev = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONZ, 0.0f);
					MasterScene.DegToCart(LightLoc);
					RealLight->setPosition(osg::Vec4(LightLoc.XYZ[0],LightLoc.XYZ[1],LightLoc.XYZ[2],1.0f));
					if(((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTSPOT))
						{ // spotlights have controlled angle
						float spot_soft;
						spot_cutoff = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONHANGLE, 0.0f) * .5; // WCS/VNS uses edge-to-edge angle, OGL uses center-to-edge
						spot_soft = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONB, 0.0f); // Additional 'soft' edge value
						if(spot_soft > 0.0)
							{
							spot_cutoff += spot_soft; // expand cone by soft region
							RealLight->setSpotExponent(100); // make it sorta soft
							} // if

						// handle spot direction
						float DirX, DirY, DirZ;
						DirX = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISX, 0.0f);
						DirY = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISY, 0.0f);
						DirZ = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISZ, 0.0f);
						RealLight->setDirection(osg::Vec3(DirX,DirY,DirZ));
						} // if
					RealLight->setSpotCutoff(spot_cutoff);
					} // if
				else // position of parallel light sources is unimportant, only direction
					{
					float DirX, DirY, DirZ;
					DirX = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISX, 0.0f);
					DirY = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISY, 0.0f);
					DirZ = (*LightWalker)->GetChannelValueTime(NVAnimObject::CANONAXISZ, 0.0f);
					RealLight->setPosition(osg::Vec4(-DirX, -DirY, -DirZ, 0.0f)); // position of parallel light sources is unimportant, direction is encoded in position
					} // else


				RealLight->setAmbient(osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // key lights don't get any ambient
				RealLight->setDiffuse(SingleLight);

				osg::LightSource* RealLightSource = new osg::LightSource;	
				RealLightSource->setLight(RealLight);
				RealLightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
				RealLightSource->setStateSetModes(*LightingStateSet,osg::StateAttribute::ON);
				LightParent->addChild(RealLightSource);

				LightsCreated++;
				} // if
			else
				{ // sum up ambients
				if((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTAMBIENTSKY)
					{
					SkyAmbient += SingleLight; // sum them up (no need to divide, as Alpha value is 0 in SingleLight)
					} // if
				else if((*LightWalker)->GetGenericIntValue(NVAnimObject::LIGHTTYPE) == NVAnimObject::LIGHTAMBIENTGROUND)
					{
					GroundAmbient += SingleLight; // sum them up (no need to divide, as Alpha value is 0 in SingleLight)
					} // else
				} // else
			} // if
		} // for

	// set up lights to simulate sky and ground ambient
	osg::Light* SkyAmbLight = new osg::Light;
	SkyAmbLight->setLightNum(LightsCreated++);
	SkyAmbLight->setPosition(osg::Vec4(0.0f, 0.0f, 1.0f, 0.0f)); // position of parallel light sources is unimportant, aim down
	SkyAmbLight->setDiffuse(SkyAmbient);

	osg::LightSource* SkyLightSource = new osg::LightSource;	
	SkyLightSource->setLight(SkyAmbLight);
	SkyLightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
	SkyLightSource->setStateSetModes(*LightingStateSet,osg::StateAttribute::ON);
	LightParent->addChild(SkyLightSource);

	// do ground
	osg::Light* GroundAmbLight = new osg::Light;
	GroundAmbLight->setLightNum(LightsCreated++);
	GroundAmbLight->setPosition(osg::Vec4(0.0f, 0.0f, -1.0f, 0.0f)); // position of parallel light sources is unimportant, aim up
	GroundAmbLight->setDiffuse(GroundAmbient);

	osg::LightSource* GroundLightSource = new osg::LightSource;	
	GroundLightSource->setLight(GroundAmbLight);
	GroundLightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
	GroundLightSource->setStateSetModes(*LightingStateSet,osg::StateAttribute::ON);
	LightParent->addChild(GroundLightSource);

	} // if
else
	{ // leave the skylight alone, and just set up ambient
	// not implemented right now. Don't see much need for it
	// if you want control of lighting, export lights
	// if not, you get default, and no control over ambient
	} // else

return(LightsCreated);
} // CreateLights



