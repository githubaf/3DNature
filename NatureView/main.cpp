// NatureView Express main.cpp
// Created from scratch on Aug 12, 2003 by CXH

//#define NV_WATERMARK_BUILD
//#define NV_BUILD_PLUGINS


unsigned long int DebugVarTotalTreeCountFile = 0;
unsigned long int DebugVarTotalTreeCountLoaded = 0;

extern char ThreadDebugMsg[160];

#include "IdentityDefines.h"
#include "KeyDefines.h"
#include "ConfigDefines.h"
#include "InstanceSupport.h"

#ifdef WIN32
#include <windows.h>


#if _MSC_VER < 1300 // VC++7 (1300) doesn't seem to understand these options
#pragma comment(linker,"/RELEASE")
#pragma comment(linker,"/merge:.reloc=.data")

#if _MSC_VER >= 1000
#pragma comment(linker,"/FILEALIGN:0x200")
#endif // MSVC6 or higher

#endif // MSC_VER < 1300

#if _MSC_VER < 1400
#pragma comment(linker,"/merge:.rdata=.data")
#pragma comment(linker,"/merge:.text=.data")
#endif // VC7 or earlier


#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <direct.h>
#include <ctype.h>

#include <iostream>
#include <sstream>

#include <GL/gl.h>
#include <GL/glu.h>

#include <osg/StateAttribute>
#include <osg/ref_ptr> 
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LOD>
#include <osg/StateSet>
#include <osg/Fog>
#include <osgUtil/SceneView>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/SharedStateManager>
#include <osgProducer/Viewer>
#include <osgProducer/OsgSceneHandler>
#include <osgProducer/OsgCameraGroup>
//#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>
#include <osgUtil/Statistics>

using namespace Producer;
using namespace osg;

#include "3DNCrypt.h"
#include "PanoManipulator.h"
#include "RequesterBasic.h"
#include "Types.h"
#include "ScriptArg.h"
#include "DEMCore.h"
#include "Useful.h"
#include "UsefulMath.h"
#include "NVMathSupport.h"
#include "EventDispatcher.h"
#include "Viewer.h"
#include "NVTerrain.h"
#include "NVTerrainSupport.h" // for CleanupOpenECWFiles()
#include "NVSky.h"
#include "NVLights.h"
#include "NVRequest.h"
#include "NVParsing.h"
#include "CameraSupport.h"
#include "NVMiscGlobals.h"
#include "InternalImage.h"
#include "NVScene.h"
#include "NVEventHandler.h"
#include "DecompressZipScene.h"
#include "CreateHelpSplashWatermark.h"
#include "NVSigCheck.h"
#include "SwitchBox.h"
#include "BusyWin.h"
#include "FileAssociation.h"
#include "NVNodeMasks.h"
#include "WindowDefaults.h"
#include "NV3DO.h"
#include "MediaSupport.h"
#include "NVWidgets.h"
#include "CPUSupport.h"
#include "FrameStats.h"
#include "OGLSupport.h"


#include "SnazzyWidget.h"
#include "NavDlg.h"
#include "InfoDlg.h"
#include "HelpDlg.h"
#include "HTMLDlg.h"
#include "DriveDlg.h"
#include "DataDlg.h"
#include "ToolTips.h"
#include "Credits.h"

#include "DriverCompliance.h"

#ifdef NV_BUILD_PLUGINS
#include "PluginTest.h"
#endif // NV_BUILD_PLUGINS

#ifdef NVW_SUPPORT_SPICLOPS
#include "SpiclopsSupport.h"
#endif // NVW_SUPPORT_SPICLOPS


NativeAnyWin ViewerNativeWindow;
InfoDlg *GlobalInfoWindow;
ToolTipSupport *GlobalTipSupport;
SnazzyWidgetContainerFactory *SWCF;
HelpDlg *GlobalHelpDlg;
extern NVFrameStats FrameStatisticsCounter;
extern NVFrameStats DrawStatisticsCounter;
bool Terminate = false;

osg::ref_ptr<PanoManipulator> PM;

NativeLoadHandle GlobalInstance;

char ProgDir[500]; // startup directory

time_t HelpAppear;

osg::ref_ptr<osg::Group> LightGroup;
osg::ref_ptr<osg::StateSet> LightingStateSet;
SwitchBox *MasterSwitches;

double TargetFramerate;
double CurrentInstantFrameRate = 30.0f; // safe default

enum
	{
	NVW_BUILDSTAGE_TERRAIN = 1,
	NVW_BUILDSTAGE_OCEAN,
	NVW_BUILDSTAGE_SKY,
	NVW_BUILDSTAGE_VEG,
	NVW_BUILDSTAGE_3DO,
	NVW_BUILDSTAGE_MAX // no comma
	}; // enum


#define NVW_BUILDSTAGE_FULLSTEP (1.0f / (float)(NVW_BUILDSTAGE_MAX))
#define NVW_BUILDSTAGE_HALFSTEP (.5f * NVW_BUILDSTAGE_FULLSTEP)
#define NVW_BUILDSTAGE_FRAC(a) ((a) * NVW_BUILDSTAGE_FULLSTEP)

/*
class NVLabel
	{
	public:
		std::string LabelText;
		MiniKeyFrame KeyNode;

		//void AddKeyFrame(class MiniKeyFrame *NewKey) {if(NewKey) KeyNode.push_back(NewKey);};
		void SetLabel(char *NewLabel) {LabelText = NewLabel;};
		const char *GetName(void) {return(LabelText.c_str());};

	}; // NVLabel
*/

//Node *makeOcean(float Elevation, const char *OceanTexFileName);

NVScene MasterScene;


class Scene
{
    private:

        bool SceneGraphBuilt;

    public:
		osg::ref_ptr<osg::Group> MasterGroup;
		osg::ref_ptr<osg::Group> SceneGroup;

        Scene(): SceneGraphBuilt(false) { MasterGroup = new osg::Group; }
		bool QuerySceneGraphBuilt(void) {return(SceneGraphBuilt);};

		osg::Group *GetRootNode(void) {return(MasterGroup.get());};
		osg::Group *GetSceneRootNode(void) {return(SceneGroup.get());};

		void BuildSceneGraph(void)
        {
			BusyWin *ProgressDisplay;

			SceneGroup = new osg::Group;
			MasterGroup->addChild(SceneGroup.get());

			MasterSwitches = new SwitchBox; // <<<>>> Currently, this never gets deleted
			MasterSwitches->AddSwitchBoxToSceneGroups(MasterGroup.get());

			float SkyRadius = 1000;

			ProgressDisplay = new BusyWin("Building", "", 0, 1);

			// add some objects here
			if(MasterScene.CheckDEMName())
				{
				if(ProgressDisplay)
					{
					ProgressDisplay->SetText("Terrain");
					ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_TERRAIN) - NVW_BUILDSTAGE_HALFSTEP);
					} // if

				osg::ref_ptr<osg::Group> LODFrameworkRoot;
				LODFrameworkRoot = new osg::Group;
				if(LODFrameworkRoot.valid())
					{
					SceneGroup->addChild(LODFrameworkRoot.get());
					if(!CreateTerrainNew(LODFrameworkRoot.get()))
						{
						Terminate = true;
						} // if
					} // if
				} // if

			if(ProgressDisplay)
				{
				ProgressDisplay->SetText("");
				ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_TERRAIN));
				} // if

// ocean disabled 11/17/05 because it's not paging-aware and you can't create it from VNS anyway
/*
			if(MasterScene.CheckOcean() && MasterScene.CheckOceanFileName()) // OceanElev defaults to 0 if unspecified
				{
				osg::ref_ptr<osg::Node> OceanNode;

				if(ProgressDisplay)
					{
					ProgressDisplay->SetText("Ocean");
					ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_OCEAN) - NVW_BUILDSTAGE_HALFSTEP);
					} // if

				OceanNode = makeOcean(MasterScene.GetOceanElev(), MasterScene.GetOceanFileName());
				MasterSwitches->AddOcean(OceanNode.get());
				} // if

			if(ProgressDisplay)
				{
				ProgressDisplay->SetText("");
				ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_OCEAN));
				} // if
*/

			const osg::BoundingSphere& boundingSphere=GetSceneRootNode()->getBound(); // don't do entire scenegraph, just 'displayed' scene
	        SkyRadius = boundingSphere._radius * 6;

			if(MasterScene.CheckSky() && MasterScene.CheckSkyFileName())
				{
				if(ProgressDisplay)
					{
					ProgressDisplay->SetText("Sky");
					ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_SKY) - NVW_BUILDSTAGE_HALFSTEP);
					} // if

				// add clearnode-transform-skydome heirarchy to the scene.
				MasterGroup->addChild(makeSkyandStructure( SkyRadius)); // add to Master, not Scene
				} // if

			if(ProgressDisplay)
				{
				ProgressDisplay->SetText("");
				ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_SKY));
				} // if



			// do lights, if provided (primarily used for 3DOs, below)
			BuildLights();
			SceneGroup->addChild(LightGroup.get());

			// add objects
			if(MasterScene.GetNum3DOInstances())
				{
				if(ProgressDisplay)
					{
					ProgressDisplay->SetText("3D Objects");
					ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_3DO) - NVW_BUILDSTAGE_HALFSTEP);
					} // if
				// create a local light to illuminate vectors
				osg::Light* myLight2 = new osg::Light;
				myLight2->setLightNum(1);
				myLight2->setPosition(osg::Vec4(0.0,0.0,0.0,-1.0f));
				myLight2->setAmbient(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
				myLight2->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

				osg::ref_ptr<osg::StateSet> LuminousVecsStateSet; // Override lighting on vectors
				LuminousVecsStateSet = new osg::StateSet;
				LuminousVecsStateSet->setGlobalDefaults();

				osg::LightSource* lightS2 = new osg::LightSource;	
				lightS2->setLight(myLight2);
				lightS2->setLocalStateSetModes(osg::StateAttribute::ON); 
				lightS2->setStateSetModes(*LuminousVecsStateSet.get(),osg::StateAttribute::ON);

				MasterScene.SceneLOD.EnableObjects(true);
				if(LightingStateSet.valid())
					{
					MasterSwitches->SetSharedObjLightState(LightingStateSet.get());
					MasterGroup->addChild(LightGroup.get()); // this step doesn't matter much if the lights' position is non-critical
					} // if

				MasterScene.SceneLOD.EnableVecs(true);
				MasterSwitches->SetSharedVecLightState(LightingStateSet.get());
				MasterGroup->addChild(lightS2); // this step doesn't matter much if the light's position is non-critical

				InitObjectCreationState();
				if(ProgressDisplay)
					{
					ProgressDisplay->SetText("");
					ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_3DO));
					} // if
				} // if

			if(ProgressDisplay)
				{
				ProgressDisplay->SetText("Complete");
				ProgressDisplay->UpdateAndCheckAbort(NVW_BUILDSTAGE_FRAC(NVW_BUILDSTAGE_MAX));
				} // if

		    // add the HUD subgraph.
			// HUD must be built after terrain in order to display Map (if required)
			osg::ref_ptr<osg::Node> HUDNode;

			HUDNode = createHUD();
			MasterSwitches->AddHUD(HUDNode.get());
			if(!MasterScene.Overlay.GetOverlayOn())
				{
				MasterSwitches->EnableHUD(false);
				} // if

		    // add the watermark subgraph.    
		    #ifdef NV_WATERMARK_BUILD
			// force watermark
			MasterScene.Watermark.SetWatermarkText(NVW_NATUREVIEW_NAMETEXT " Prototype " __DATE__ " " __TIME__ "\n" NVW_NATUREVIEW_COPYRIGHTTEXT " NOT FOR RELEASE!");
			MasterScene.Watermark.SetWatermarkState(true); // force it on, cannot be turned off by key control
			#endif // NV_WATERMARK_BUILD
			if(MasterScene.Watermark.GetWatermarkOn())
				{
				MasterGroup->addChild(createWatermark()); // add to master, not scene
				} // if

			// add Haze
			NVAnimObject *FirstHaze;
			if((MasterScene.CheckHaze()) && (FirstHaze = MasterScene.GetHaze()) && MasterScene.GetHaze()->GetEnabled() && MasterScene.GetHaze()->CheckKeyFrame(0.0f))
				{
				osg::ref_ptr<osg::Fog> SceneHaze;
				SceneHaze = new osg::Fog;
				SceneHaze->setMode(osg::Fog::LINEAR);
				SceneHaze->setDensity(FirstHaze->GetChannelValueTime(NVAnimObject::CANONINTENSITYA, 0.0f));
				SceneHaze->setStart(FirstHaze->GetChannelValueTime(NVAnimObject::CANONDISTANCEA, 0.0f));
				SceneHaze->setEnd(FirstHaze->GetChannelValueTime(NVAnimObject::CANONDISTANCEA, 0.0f) + FirstHaze->GetChannelValueTime(NVAnimObject::CANONDISTANCEB, 0.0f));
				SceneHaze->setColor(osg::Vec4(FirstHaze->GetChannelValueTime(NVAnimObject::CANONCOLORR, 0.0f), FirstHaze->GetChannelValueTime(NVAnimObject::CANONCOLORG, 0.0f), FirstHaze->GetChannelValueTime(NVAnimObject::CANONCOLORB, 0.0f), 1.0));

				osg::ref_ptr<StateSet> fogstate = new StateSet;

				fogstate->setAttributeAndModes(SceneHaze.get(), StateAttribute::ON);
				SceneGroup->setStateSet(fogstate.get()); // add to scene -- don't affect stuff in MasterGroup (like sky and HUD and watermark)
				} // if


			// create splash window last
			if(MasterScene.GetSplashTime() > 0.0f)
				{
				osg::ref_ptr<osg::Node> SplashNode;

				SplashNode = createSplash();
				MasterSwitches->AddSplash(SplashNode.get());
				SetSplashDisplayed(true);
				} // if

			//osgUtil::Optimizer Opt;
			//Opt.optimize(MasterGroup.get(), osgUtil::Optimizer::OptimizationOptions::ALL_OPTIMIZATIONS);

            SceneGraphBuilt = true;
		if(ProgressDisplay) delete ProgressDisplay;
		ProgressDisplay = NULL;
        } // BuildSceneGraph

}; // class Scene


Scene MainScene;

// called from PanoManipulator, who doesn't want to understand a full Scene class
// these currently have to go wherever class Scene (above) lives
bool GetSceneGraphBuilt(void)
{
return(MainScene.QuerySceneGraphBuilt());
} // GetSceneGraphBuilt

osg::Group *GetSceneRootNodeGlobal(void)
{
return(MainScene.GetSceneRootNode());
} // GetSceneRootNodeGlobal




/*
Node *makeOcean(float Elevation, const char *OceanTexFileName)
{
int c = 0;
PartialVertexDEM Corner;

Vec3Array *coords = new Vec3Array(4);
Vec2Array *tcoords = new Vec2Array(4);
Vec4Array *colors = new Vec4Array(1);

(*colors)[0].set(1.0f,1.0f,1.0f,0.75f); // white

Corner.Elev = Elevation;

// SW
Corner.Lat = MasterScene.GetCoordYMin();
Corner.Lon = MasterScene.GetCoordXMax();
MasterScene.DegToCart(Corner);
// use .xyz as texcoord
Corner.xyz[0] = 0.0f;
Corner.xyz[1] = 0.0f;
// store the results
(*coords)[c].set(Corner.XYZ[0],Corner.XYZ[1],Corner.XYZ[2]);
(*tcoords)[c++].set(Corner.xyz[0], Corner.xyz[1]); // I think that's the first time I've ever actually done that ("c++") in real code...

// SE
Corner.Lon = MasterScene.GetCoordXMin();
Corner.Lat = MasterScene.GetCoordYMin();
MasterScene.DegToCart(Corner);
// use .xyz as texcoord
Corner.xyz[0] = 1.0f;
Corner.xyz[1] = 0.0f;
// store the results
(*coords)[c].set(Corner.XYZ[0],Corner.XYZ[1],Corner.XYZ[2]);
(*tcoords)[c++].set(Corner.xyz[0], Corner.xyz[1]); // I think that's the first time I've ever actually done that ("c++") in real code...

// NE
Corner.Lon = MasterScene.GetCoordXMin();
Corner.Lat = MasterScene.GetCoordYMax();
MasterScene.DegToCart(Corner);
// use .xyz as texcoord
Corner.xyz[0] = 1.0f;
Corner.xyz[1] = 1.0f;
// store the results
(*coords)[c].set(Corner.XYZ[0],Corner.XYZ[1],Corner.XYZ[2]);
(*tcoords)[c++].set(Corner.xyz[0], Corner.xyz[1]); // I think that's the first time I've ever actually done that ("c++") in real code...

// NW
Corner.Lon = MasterScene.GetCoordXMax();
Corner.Lat = MasterScene.GetCoordYMax();
MasterScene.DegToCart(Corner);
// use .xyz as texcoord
Corner.xyz[0] = 0.0f;
Corner.xyz[1] = 1.0f;
// store the results
(*coords)[c].set(Corner.XYZ[0],Corner.XYZ[1],Corner.XYZ[2]);
(*tcoords)[c++].set(Corner.xyz[0], Corner.xyz[1]); // I think that's the first time I've ever actually done that ("c++") in real code...

Geometry *geom = new Geometry;

geom->setVertexArray( coords );

geom->setTexCoordArray( 0, tcoords );

geom->setColorArray( colors );
geom->setColorBinding( Geometry::BIND_OVERALL );

geom->addPrimitiveSet( new DrawArrays(PrimitiveSet::QUADS,0,4) );

Texture2D *tex = new Texture2D;

tex->setImage(osgDB::readImageFile(OceanTexFileName));
tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
if(MasterScene.SceneLOD.CheckCompressTerrainTex())
	{
	tex->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
	} // if

StateSet *dstate = new StateSet;
dstate->setMode(GL_BLEND,osg::StateAttribute::ON);
dstate->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );

dstate->setTextureAttribute(0, new TexEnv );


// add reflectivity

osg::Image* imageref = osgDB::readImageFile("BassTrack_Ref.png");
if (imageref)
{
    osg::Texture2D* textureref = new osg::Texture2D;
    textureref->setImage(imageref);

    osg::TexGen* texgenref = new osg::TexGen;
    texgenref->setMode(osg::TexGen::SPHERE_MAP);

    osg::TexEnv* texenvref = new osg::TexEnv;
    texenvref->setMode(osg::TexEnv::BLEND);
    texenvref->setColor(osg::Vec4(.5f,0.5f,0.5f,0.5f));

    dstate->setTextureAttributeAndModes(1,textureref,osg::StateAttribute::ON);
    dstate->setTextureAttributeAndModes(1,texgenref,osg::StateAttribute::ON);
    dstate->setTextureAttribute(1,texenvref);
    
} // if


osg::ref_ptr<osg::Material> material = new osg::Material;
material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.f));
material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.f));
material->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.f));
dstate->setAttributeAndModes( material.get(), osg::StateAttribute::ON );

geom->setStateSet( dstate );

Geode *geode = new Geode;
geode->addDrawable( geom );
geode->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_OCEAN);

return geode;
} // makeOcean

*/



















// callbacks

struct MyNearFarClampCallback : public osgUtil::CullVisitor::ClampProjectionMatrixCallback
{
    osgUtil::CullVisitor* _cv;

    MyNearFarClampCallback(osgUtil::CullVisitor* cv):_cv(cv) {}
    

    virtual bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double &znear, double &zfar) const
		{
        //std::cout<<"virtual bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const"<<std::endl;
        return _cv->clampProjectionMatrixImplementation(projection,znear, zfar);
		}
    
    virtual bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double &znear, double &zfar) const
    {
        //std::cout<<"virtual bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const"<<std::endl;
		//if(znear < MasterScene.SceneLOD.GetNearClip()) znear = MasterScene.SceneLOD.GetNearClip();
		if (zfar>0.0f)
			{
			if (projection(0,3)==0.0f && projection(1,3)==0.0f && projection(2,3)==0.0f)
				{ // orthographic -- leave alone
				} // if
			else
				{
				znear = MasterScene.SceneLOD.GetNearClip();
				if(zfar  > MasterScene.SceneLOD.GetFarClip())  zfar  = MasterScene.SceneLOD.GetFarClip();
				} // else
			} // if
        return _cv->clampProjectionMatrixImplementation(projection,znear, zfar);
    }
}; // MyNearFarClampCallback



class MyRealizeCallback : public osgProducer::OsgCameraGroup::RealizeCallback
{
	public:
	    void operator()( osgProducer::OsgCameraGroup& cg, osgProducer::OsgSceneHandler& sh, const Producer::RenderSurface & rs)
			{
			//std::cout << glGetString(GL_VENDOR)  << std::endl;
			//std::cout << glGetString(GL_RENDERER) << std::endl;
			setVSync(1); // enable VSync, if the driver lets us control it

			// we have to do this once things get going and the graphics context is valid
			static bool OneShotCheckFired;
			if(!OneShotCheckFired)
				{
				CheckAndWarnDriverCompliance();
				OneShotCheckFired = true;
				} // if

			MainScene.BuildSceneGraph();
			sh.init();
			}

}; // MyRealizeCallback



char *RipTemplate = "SCENE/DFLT SPLASH WINDOW SPICHANNELS SPIMODE";


#ifndef NVW_USE_MAIN
#ifdef _CONSOLE
#define NVW_USE_MAIN
#endif // _CONSOLE
#endif // !NVW_USE_MAIN


extern "C" {
#ifdef NVW_USE_MAIN
int main(int Count, char **Vector);
#else // !NVW_USE_MAIN
int PASCAL WinMain(NativeLoadHandle Instance, NativeLoadHandle PrevInst, LPSTR Param,
	int Show);
#endif // !NVW_USE_MAIN

#ifdef NVW_USE_MAIN
int main(int Count, char **Vector)
{
NativeLoadHandle Instance = NULL;
const char *Param = Vector[0];
#else // !NVW_USE_MAIN
int PASCAL WinMain(NativeLoadHandle Instance, NativeLoadHandle PrevInst, LPSTR Param,
	int Show)
{ // } matching bracket to balance brace-match
SetHInstance(Instance);
#endif // !NVW_USE_MAIN

#ifdef NV_BUILD_PLUGINS

PluginManager PIM;

#endif // NV_BUILD_PLUGINS

getcwd(ProgDir, 250);

RegisterNVWidgets(Instance);

#ifdef NV_CHECK_SIGNATURES
// Clear signature caching
InitSigCaches();
#endif // NV_CHECK_SIGNATURES

// process the startup args
char ButcherableArg[1024];
int NumArgsParsed, Count;;
char *Result;

// associate our file extension with the current executable
SetupFileAssociation("."NVW_NATUREVIEW_FILEEXTTEXT, false); // don't force NVW association if someone other than us has it already
SetupFileAssociation("."NVW_NATUREVIEW_COMPFILEEXTTEXT, true); // .nvz as well, and force it, since no-one else should have that association

// lock in path to DLLs
char LineBuffer[1000];
getcwd(LineBuffer, 999);
osgDB::Registry::instance()->setLibraryFilePathList(LineBuffer);
osgDB::Registry::instance()->getOrCreateSharedStateManager()->setShareMode(osgDB::SharedStateManager::SHARE_TEXTURES);

UpdateLastMovementMoment(); // initialize time of last movement for movement optimization

strncpy(ButcherableArg, Param, 1000); ButcherableArg[1000] = NULL;
ArgRipper JackThe(RipTemplate);

if(NumArgsParsed = JackThe.Rip(ButcherableArg))
	{
	for(Count = 0; Count < NumArgsParsed;)
		{
		if(Result = JackThe.GetArg(0, Count)) // 0=SCENE
			{
			MasterScene.SetSceneName(Result);
			continue;
			} // if
		else Count++;
		} // for
	for(Count = 0; Count < NumArgsParsed;)
		{
		if(Result = JackThe.GetArg(1, Count)) // 1=SPLASH
			{
			MasterScene.SetSplashTime(atof(Result));
			continue;
			} // if
		else Count++;
		} // for
	for(Count = 0; Count < NumArgsParsed;)
		{
		if(Result = JackThe.GetArg(2, Count)) // 2=WINDOW
			{
			ParseWindowDefaultOptions(Result);
			continue;
			} // if
		else Count++;
		} // for
#ifdef NVW_SUPPORT_SPICLOPS
	for(Count = 0; Count < NumArgsParsed;)
		{
		if(Result = JackThe.GetArg(3, Count)) // 3=SPICHANNELS
			{
			SetSPIChannels(atof(Result));
			SetSPIEnabled(true);
			continue;
			} // if
		else Count++;
		} // for
	for(Count = 0; Count < NumArgsParsed;)
		{
		if(Result = JackThe.GetArg(4, Count)) // 4=SPIMODE
			{
			SetSPIModeFromString(Result);
			SetSPIEnabled(true);
			continue;
			} // if
		else Count++;
		} // for
#endif NVW_SUPPORT_SPICLOPS
	} // if

// request file via file requester if not supplied
if(!MasterScene.CheckSceneName())
	{
	char *NewSceneName;
	if(NewSceneName = SimpleRequestFileName("Open NatureView Scene File"))
		{
		MasterScene.SetSceneName(NewSceneName);
		} // if
	else
		{
		return(0); // nothing to do.
		} // else
	} // if


int NumEntities = 0;
// read the scene file if provided
if(MasterScene.CheckSceneName())
	{
	NumEntities = ReadScene(MasterScene.GetSceneName());
	} // if

#ifdef NVW_SUPPORT_SPICLOPS
if(GetSPIEnabled())
	{
	if(!QuerySPIDLLAvailable())
		{
		SetSPIEnabled(false);
		} // if
	} // if

if(GetSPIEnabled())
	{
	// disable some UI decorations that will interfere
	if(GetSPIChannels() > 1)
		{ // splash screen in orthographic coords looks dumb on multi-channel displays
		MasterScene.SetSplashTime(0.0f);
		} // if
	MasterScene.Overlay.SetOverlayState(0);
	} // if
#endif // NVW_SUPPORT_SPICLOPS

// Notice interferes with splash, so disable splash
if(MasterScene.GetNoticeTime() > 0 && (!MasterScene.GetNoticeFile().empty() || !MasterScene.GetNoticeText().empty()))
	{
	MasterScene.SetSplashTime(0.0f);
	} // if

if(NumEntities > 0)
	{
	unsigned int LightOptions = NULL;
	osgProducer::Viewer *tempviewer = new osgProducer::Viewer;

	MasterScene.SyncAnimFlag(); // this will trigger animation on right at the start if camera 0 is animated

	SetGlobalViewer(tempviewer);

	// initialize the viewer
	// (both osgProducer::Viewer::VIEWER_MANIPULATOR and osgProducer::Viewer::STATS_MANIPULATOR are required to get a framerate counter
	if(MasterScene.GetNumLights() == 0)
		{
		// we'll make our own lighting rig
		//LightOptions = osgProducer::Viewer::SKY_LIGHT_SOURCE;
		} // if
	//GetGlobalViewer()->setUpViewer(osgProducer::Viewer::STATS_MANIPULATOR | osgProducer::Viewer::ESCAPE_SETS_DONE | osgProducer::Viewer::VIEWER_MANIPULATOR | /* osgProducer::Viewer::SKY_LIGHT_SOURCE | */ osgProducer::Viewer::STATE_MANIPULATOR);
	GetGlobalViewer()->setUpViewer(osgProducer::Viewer::STATS_MANIPULATOR | osgProducer::Viewer::ESCAPE_SETS_DONE | osgProducer::Viewer::VIEWER_MANIPULATOR | LightOptions | osgProducer::Viewer::STATE_MANIPULATOR);
	//GetGlobalViewer()->setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
	GetGlobalViewer()->getCamera(0)->getRenderSurface()->setWindowName(NVW_NATUREVIEW_NAMETEXT);
	
	if(!GetShowViewFullscreenByDefault())
		{
		GetGlobalViewer()->getCamera(0)->getRenderSurface()->fullScreen(0);
		GetGlobalViewer()->getCamera(0)->getRenderSurface()->useBorder(0);
		} // if

	GetGlobalViewer()->getEventHandlerList().push_front(new NVEventHandler(GetGlobalViewer()));

	PM = new PanoManipulator();
	unsigned int pos = GetGlobalViewer()->addCameraManipulator(PM.get());
	GetGlobalViewer()->selectCameraManipulator(pos);

    GetGlobalViewer()->setRealizeCallback( new MyRealizeCallback );


	GetGlobalViewer()->setClearColor(osg::Vec4(0.6, 0.6, 1.0, 1.0));

#ifdef NVW_SUPPORT_SPICLOPS

	//Add rendersurface realize callback, this creates the spi context within a valid OpenGL context
	Producer::ref_ptr<InitSpiclopsCallback> isc;
	if(GetSPIEnabled())
		{
		isc  = new InitSpiclopsCallback(GetSPIChannels(),GetSPIMode());
		GetGlobalViewer()->getCamera(0)->getRenderSurface()->addRealizeCallback( isc.get() );
		} // if

#endif // NVW_SUPPORT_SPICLOPS


	// set the scene to render
	GetGlobalViewer()->setSceneData(MainScene.GetRootNode());

	// create the windows and run the threads.
	// by doing this before creating the scene, we can ensure we have a valid context
#ifdef NVW_SUPPORT_SPICLOPS
	if(GetSPIEnabled() && GetSPIChannels() > 1)
		{
		GetGlobalViewer()->realize(Producer::CameraGroup::SingleThreaded); // Single-threaded apparently still needed for SPIclops
		} // if
	else
		{
		GetGlobalViewer()->realize();
		} // else
#else // !NVW_SUPPORT_SPICLOPS
	GetGlobalViewer()->realize();
#endif // !NVW_SUPPORT_SPICLOPS


#ifdef NVW_SUPPORT_SPICLOPS
	//If spi context was created then initialize channels
	if(GetSPIEnabled())
		{
		if(isc->getContext())
			{
			initSpiclops(isc->getContext(),GetSPIChannels(),GetGlobalViewer());
			} // if
		else
			{
			printf("Failed to initialize spi context\n");
			} // else
		} // if
#endif // NVW_SUPPORT_SPICLOPS


    // Find Viewer's HWND
	ViewerNativeWindow = GetGlobalViewerHWND();

	ToolTipSupport IconCaptions;
	GlobalTipSupport = &IconCaptions;

	// need to create a SnazzyWidgetContainerFactory to use SnazzyWidgets
	SWCF = new SnazzyWidgetContainerFactory(GetHInstance());
	
	// DriveDlg must be instantiated first, as other two snap to it by default
	DriveDlg DashWindow(GetShowDriveWindowByDefault());
	InfoDlg InfoWindow(GetShowInfoWindowByDefault());
	NavDlg NavToolbar(GetShowNavWindowByDefault());
	DataDlg DataTable(false);
	HTMLDlg WebBrowser(false);
	// Map Window is currently part of Overlay -- set it now
	MasterScene.Overlay.SetOverlayState(GetShowMapWindowByDefault());


	GlobalInfoWindow = &InfoWindow;


	// Make the Scene object and initialize
	//MainScene.BuildSceneGraph();
	// currently done in realize callback (above) so that we have a valid gl context, which a number
	// of image-related glu operations require

	if(MasterScene.SceneLOD.CheckNearClip() || MasterScene.SceneLOD.CheckFarClip() || MasterScene.SceneLOD.CheckMinFeatureSizePixels())
		{
		osgProducer::OsgSceneHandler* scenehandler = GetGlobalViewer()->getSceneHandlerList()[0].get();
		osgUtil::SceneView* sv = scenehandler->getSceneView();

		if(sv)
			{
			if(MasterScene.SceneLOD.CheckMinFeatureSizePixels())
				{
				sv->setSmallFeatureCullingPixelSize(MasterScene.SceneLOD.GetMinFeatureSizePixels());
				} // if

			if(MasterScene.SceneLOD.CheckNearClip() || MasterScene.SceneLOD.CheckFarClip())
				{
				sv->getCullVisitor()->setClampProjectionMatrixCallback(new MyNearFarClampCallback(sv->getCullVisitor()));
				} // if
			} // if
		} // if

	time_t SplashAppear, now;
	time(&SplashAppear);

	// display Notice now, if necesary
	if(MasterScene.GetNoticeTime() > 0 && (!MasterScene.GetNoticeFile().empty() || !MasterScene.GetNoticeText().empty()))
		{
		// Not used yet until we have some sort of safe text-substitution method
		//PrepCreditsImage(); // make logo image available in temp dir. Will be cleaned up in HTMLDlg::Show(false)
		//HTMLDlg::SetHTMLText("<HTML><HEAD></HEAD><BODY><center><h2>You have been given notice.</h2><br><br>Click the close button.</center></BODY></HTML>");
		if(!MasterScene.GetNoticeText().empty())
			{
			HTMLDlg::SetCenterOnOpen(true);
			if(MasterScene.GetNoticeModal()) SetMovementLockoutGlobal(true); // HTMLDlg will clear this on exit
			HTMLDlg::SetForceTitle("Notice");
			HTMLDlg::SetHTMLText(MasterScene.GetNoticeText().c_str());
			SetNoticeDisplayed(true);
			} // if
		else if(!MasterScene.GetNoticeFile().empty())
			{
			// check sig
			#ifdef NV_CHECK_SIGNATURES
			if(!MasterScene.GetNoticeSig().empty() && CheckDependentBinaryFileSignature(MasterScene.GetNoticeSig().c_str(), MasterScene.GetNoticeFile().c_str()))
			#else // !NV_CHECK_SIGNATURES
			if(1)
			#endif // !NV_CHECK_SIGNATURES
				{
				HTMLDlg::SetCenterOnOpen(true);
				if(MasterScene.GetNoticeModal()) SetMovementLockoutGlobal(true); // HTMLDlg will clear this on exit
				HTMLDlg::SetForceTitle("Notice");
				HTMLDlg::SetHTMLTextFromFile(MasterScene.GetNoticeFile().c_str());
				SetNoticeDisplayed(true);
				} // if
			else
				{
				SetGlobalSigInvalid(true);
				} // else
			} // if
		} // if

	// tweak some settings
	osgDB::DatabasePager* dp = osgDB::Registry::instance()->getDatabasePager();
	if(dp)
		{
		//_putenv( "OSG_DATABASE_PAGER_GEOMETRY=VertexArrays" );
		dp->setMaximumNumOfObjectsToCompilePerFrame(1000);
		dp->setMinimumTimeAvailableForGLCompileAndDeletePerFrame(.1);
		dp->setExpiryDelay(1.5); // cull unused data rapidly -- doesn't entirely seem to be working.
		//dp->setThreadPriorityOutwithFrame(OpenThreads::Thread::THREAD_PRIORITY_NOMINAL); // NECESSARY, or we get lousy load performance
		} // if
	
	// ensure framerate counting is working properly
	GetGlobalViewer()->setInstrumentationMode(true);


    DEVMODE dm;
    memset(&dm,0,sizeof(dm));
    dm.dmSize = sizeof(dm);

    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    // Frame rate limiter code is originally courtesy of "Norman Vine" <nhv@cape.com> via osg list, Date: Wed, 26 Oct 2005 08:15:13 -0400
    // calculate max desired refresh rate
    int MaxDesiredFramerate = dm.dmDisplayFrequency / NV_FRAME_RATE_MULTIPLE;

	// <<<>>> debugging -- have framerate counter on to start
	//GetGlobalViewer()->setInstrumentationMode(!GetGlobalViewer()->getInstrumentationMode());
	
	float BGAvailableFrac = .75f; // keep 25% of effort available for bg loading
	if(QueryNumberOfRealCPUs() > 1)
		{
		// only need 10% available if we have another CPU to help with loading
		// ("front" CPU still needed for compile/download
		BGAvailableFrac = .9f;
		} // if

	int LastElapsed = 0; // used to determine last frame framerate
	while( !GetGlobalViewer()->done() && !Terminate)
		{
		// start timing how long we've spent on this frame
        osg::Timer_t start_tick = osg::Timer::instance()->tick();

		// average framerate counters
		static Producer::Camera::TimeStamp LastStamp, ThisStamp; // really a double
		double TimeDelta;
		double AvgTime = 0.001f; // not 0 to prevent divide by zero
		Producer::CameraGroup::FrameStats Stats;
		Stats = GetGlobalViewer()->getFrameStats();
		ThisStamp = Stats.getStartOfFrame();
		TimeDelta = ThisStamp - LastStamp;
		if(TimeDelta > 0 && TimeDelta < 3) // eliminate wacked math
			{ // this updates the framerate counter, used for UI displays
			FrameStatisticsCounter.AddNewSample(TimeDelta);
			} // if

		// but this is the DRAW time counter, updated elsewhere
		if(DrawStatisticsCounter.QueryAdequateSamples())
			{
			AvgTime = DrawStatisticsCounter.FetchAverageTime();
			} // if
		LastStamp = ThisStamp;



		// wait for all cull and draw threads to complete.
		GetGlobalViewer()->sync();
		MasterScene.UpdateHUD();
		
		// Adjust LODScale to maintain target framerate
		if(MasterScene.SceneLOD.GetFramerateMaintain())
			{
			double AimFactor;
			// calculate current framerate
			//if(LastElapsed > 0.0)
			if(AvgTime > 0.0)
				{
				//CurrentInstantFrameRate = (double)(1.0 / ((double)LastElapsed * .000001)); // LastElapsed is in usecs
				CurrentInstantFrameRate = (double)(1.0 / ((double)AvgTime)); // AvgTime is in decimal seconds already
				// modify by BGAvailableFrac to allot for BG loading, etc
				CurrentInstantFrameRate *= BGAvailableFrac;
				} // if
			
			// compare against MaxDesiredFramerate and modify
			// AutoFactor to try to get current rate closer to MaxDesiredFramerate
			AimFactor = CurrentInstantFrameRate - (double)MaxDesiredFramerate;
			if(AimFactor > 2.0)
				{ // more detail = decrease LOD factor
				if(!(dp->getFileRequestListSize() || dp->getDataToCompileListSize())) // don't increase detail while we're already loading stuff
					{
					if(MasterScene.SceneLOD.GetLODAutoFactor() > .0001)
						{
						double ExcessFrames = (AimFactor - 2.0); // ExcessFrames goes from >0 to infinity
						double AdjustFactor = ExcessFrames / 30.0;
						if(AdjustFactor > 1.0) AdjustFactor = 1.0;
						double LODAdjustAmount = flerp(AdjustFactor, .999, .75); // if we have a small excess, we adjust by .999, if we have a large excess, we adjust by .75
						MasterScene.SceneLOD.SetLODAutoFactor(MasterScene.SceneLOD.GetLODAutoFactor() * LODAdjustAmount); // adjust for more detail
						} // if
					} // if
				} // if
			else if(AimFactor < 0.0)
				{ // less detail = increase LOD factor
				if(MasterScene.SceneLOD.GetLODAutoFactor() < 1000)
					{
					double Undershoot = (-AimFactor); // UndershootFrames goes from >0 to infinity
					double AdjustFactor = Undershoot / 30.0;
					if(AdjustFactor > 1.0) AdjustFactor = 1.0;
					double LODAdjustAmount = flerp(AdjustFactor, (1.0f / (.999f)), (1.0f / (.75f))); // if we have a small excess, we adjust by .999, if we have a large excess, we adjust by .75
					MasterScene.SceneLOD.SetLODAutoFactor(MasterScene.SceneLOD.GetLODAutoFactor() * LODAdjustAmount); // adjust by inverse for less detail
					} // if
				} // else
			
			// load modified LODScale
			GetGlobalViewer()->setLODScale(MasterScene.SceneLOD.GetLODScalingFactor() * MasterScene.SceneLOD.GetLODAutoFactor());
			} // if
	
		// Adjust target framerate lower when we're not moving
		double SettledTime = MasterScene.SceneLOD.GetSceneSettledTime();
		TargetFramerate = MaxDesiredFramerate;
		if(!GetTransitionInProgress() && !GetAnimationInProgress() && !GetTourInProgress() && SettledTime > NV_LOD_SETTLE_TIME_SECONDS)
			{
			// after 1 second of settle, drop refresh rate to 5fps
			// which should allow us to react pretty quickly
			// should the user start moving again
			double SettledRange;
			SettledRange = SettledTime * (1.0 / NV_LOD_SETTLE_WINDOW_SECONDS);
			if (SettledRange > 1.0) SettledRange = 1.0;
			// commented out because it's not helping the displaylist compile bottleneck, it's making it WORSE!
			//TargetFramerate = flerp(SettledRange, MaxDesiredFramerate, NV_LOD_SETTLE_FPS);
			} // if
		
		// calculate amount of time we can spend each frame and still achieve our target framerate
		int desired_elapsed = (1000000 / (int)TargetFramerate)  - 1000;

		dp->setTargetFrameRate(TargetFramerate);

		// handle movement optimization
		if(MasterScene.SceneLOD.CheckOptimizeMove())
			{
			if((GetSystemTimeFP() - GetLastMovementMoment()) > NV_MOVE_OPTIMIZE_SETTLE_TIME_SECONDS)
				{ // turn back on the foliage
				MasterScene.SceneLOD.HideFoliageOptimizeMove(false);
				} // if
			else
				{ // turn off the foliage temporarily until we stop moving
				MasterScene.SceneLOD.HideFoliageOptimizeMove(true);
				} // else
			} // if

		// configure the current NodeMask for update() and frame()
		GetGlobalViewer()->getSceneHandlerList().at(0)->getSceneView()->setInheritanceMask((osgUtil::SceneView::ALL_VARIABLES & ~osgUtil::SceneView::CULL_MASK));
		GetGlobalViewer()->getSceneHandlerList().at(0)->getSceneView()->setCullMask(MasterScene.SceneLOD.GetCurrentEnabledNodeMask());
		
		// update the scene by traversing it with the the update visitor which will
		// call all node update callbacks and animations.
		GetGlobalViewer()->update();
		// fire off the cull and draw traversals of the scene.
		GetGlobalViewer()->frame();

		// count polygons if framerate counter is on
		unsigned int totalNumTriangles = 0;
		if(GetGlobalViewer()->getInstrumentationMode())
			{
			osgUtil::RenderStage *stage = GetGlobalViewer()->getSceneHandlerList().at(0)->getSceneView()->getRenderStage();
			osgUtil::Statistics stats;
			stage->getStats(stats);	// was getPrims(&stats)
			stats.setType(osgUtil::Statistics::STAT_PRIMS);
			osgUtil::Statistics::PrimitiveValueMap::iterator pItr;
			for(pItr=stats._primitiveCount.begin(); pItr!=stats._primitiveCount.end();++pItr)
				{
				switch(pItr->first)
					{
					case GL_TRIANGLES:
						totalNumTriangles += pItr->second.second / 3; break;
					case GL_TRIANGLE_STRIP:
					case GL_TRIANGLE_FAN:
					case GL_QUAD_STRIP:
					case GL_POLYGON:
						totalNumTriangles += pItr->second.second - (2 * pItr->second.first);
						break;  //fix1
					case GL_QUADS:
						totalNumTriangles += pItr->second.second / 2; break;  //fix2
					} // switch
				} // for
			} // if
			
		SetTotalNumTriangles(totalNumTriangles);

		if(GetSplashDisplayed())
			{ // count out splash display time
			time(&now);
			if(now - SplashAppear >= MasterScene.GetSplashTime())
				{
				MasterSwitches->EnableSplash(false);
				SetSplashDisplayed(false);
				} // if
			} // if
		if(GetNoticeDisplayed())
			{ // count out notice display time
			time(&now);
			if(now - SplashAppear >= MasterScene.GetNoticeTime())
				{
				HTMLDlg::Show(false);
				SetNoticeDisplayed(false);
				SetMovementLockoutGlobal(false); // ends a modal Notice window
				} // if
			} // if
		if(MasterScene.SceneLOD.CheckHelpEnabled())
			{ // count out Help display time
			time(&now);
			if(now - HelpAppear >= NVW_HELP_DISPLAY_SECONDS)
				{
				MasterScene.SceneLOD.EnableHelp(0);
				} // if
			} // if

		NavToolbar.ProcessAndHandleEvents();


#ifdef NV_BUILD_PLUGINS

		if(PIM.QueryPluginsLoaded())
			{
			static char TextBuf[1000];
			PIM.SetTime(0.0);
			PIM.SetHeight(1.0);
			PIM.SetX(2.0);
			PIM.SetY(3.0);
			PIM.SetZ(4.0);
			PIM.QueryText(TextBuf);
			MasterScene.Overlay.SetOverlayTextTemplate(TextBuf);
			} // if

#endif // NV_BUILD_PLUGINS

		// twiddle thumbs for remaining available time to let other tasks run
		osg::Timer_t end_tick = osg::Timer::instance()->tick();
		LastElapsed = osg::Timer::instance()->delta_u(start_tick, end_tick);
		if(TimeDelta > 0 && TimeDelta < 3) // eliminate wacked math
			{
			// this is the average DRAW time counter
			DrawStatisticsCounter.AddNewSample((double)LastElapsed * .000001);
			} // if

		if ( LastElapsed < desired_elapsed )
			{
			// do some more compiling of GL objects if required
			double AvailableCompileTime, UsedCompileTime;
			AvailableCompileTime = ((double)(desired_elapsed-LastElapsed)) * .000001; // AvailableCompileTime is in decimal time, not microseconds

			dp->compileGLObjects(*(GetGlobalViewer()->getSceneHandlerList().at(0)->getSceneView()->getState()),AvailableCompileTime);

			// update timing after compile
			end_tick = osg::Timer::instance()->tick();
			LastElapsed = osg::Timer::instance()->delta_u(start_tick, end_tick);

			UsedCompileTime = AvailableCompileTime - (((double)(desired_elapsed-LastElapsed)) * .000001);

			/*
			char LocalDebugMsg[100];
			memset(ThreadDebugMsg, 0, 160);
			sprintf(LocalDebugMsg, "Avail: %f\nUsed: %f", AvailableCompileTime, UsedCompileTime);
			strcpy(ThreadDebugMsg, LocalDebugMsg);
			*/

			// do we still have some time left?
			// If so, some drivers waste CPU time if we let VSYNC do its job.
			// We'll predict how long we should twiddle our thumbs and do most of the waiting ourselves
			// to save time for background loading
			// However, if we aren't background loading or compiling, don't twiddle as it may cause us to lose the CPU
			// and lose performance
			//if (LastElapsed < desired_elapsed && (dp->getFileRequestListSize() || dp->getDataToCompileListSize()))
/*			if (LastElapsed < desired_elapsed)
				{ // really twiddle
				double TwiddleTime;
				TwiddleTime = desired_elapsed-LastElapsed;
				TwiddleTime *= .95; // want to undershoot our target rather than overshoot
				OpenThreads::Thread::microSleep((unsigned int)TwiddleTime); // takes microseconds instead of milliseconds
				} // if
	*/		} // if
           
		} // while !Done

	// wait for all cull and draw threads to complete before exit.
	GetGlobalViewer()->sync();

	osgDB::DatabasePager* dpkill = osgDB::Registry::instance()->getDatabasePager();
	if(dpkill)
		{
		dp->setAcceptNewDatabaseRequests(false);
		dp->clear();
		dp->cancel();
		} // if

	delete SWCF;
	} // if NumEntities


// clean up delay-unloaded DEM
if(MasterScene.GetTerrain())
	{
	delete MasterScene.GetTerrain();
	MasterScene.SetTerrain(NULL);
	} // if

CleanupOpenECWFiles();

// stop and clean up any playing sounds
CleanupSounds();

UnRegisterNVWidgets(Instance);

return 0;
} // main / WinMain

} // extern "C"
