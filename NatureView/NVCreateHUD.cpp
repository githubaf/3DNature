
#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgDB/ReadFile>
#include <osg/TexEnv>

using namespace osg;

#include "NVOverlay.h"
#include "InternalImage.h"
#include "NVScene.h"
#include "NVSigCheck.h"

extern NVScene MasterScene;

// Image data for the you-are-here marker on the HUD map
// believe it or not, this extern "C" is required...
extern "C" {
#define REDARROW38X38_PNG_RAWDATA_SIZE		735
extern unsigned char RedArrow38x38_png_rawData[REDARROW38X38_PNG_RAWDATA_SIZE];
#define REDDOT7X7_PNG_RAWDATA_SIZE		296
extern unsigned char RedDot7x7_png_rawData[REDDOT7X7_PNG_RAWDATA_SIZE];
} // extern "C"


osg::Node* createHUD()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<PositionAttitudeTransform> PAT;

	float FinalLogoWidth = 0.0;
	unsigned long int LogoRight, LogoLeft;
	unsigned long int MapRight;

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // disable depth test, and make sure that the hud is drawn after everything 
    // else so that it always appears on top.
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(15,"HUD1");

	float TextHeight = 28, YMargin = 28, HalfText;
	HalfText = TextHeight * 0.5f;
	float XMin = 0, XMax = NVW_NVOVERLAY_PSEUDO_SCREENSIZE_X;
	float YMin = 0, YMax = TextHeight * (MasterScene.Overlay.GetNumLines() + 1);


LogoRight = LogoLeft = XMax;
MapRight  = LogoLeft - HalfText; // calculated here by default, in case we don't have a LOGO (below)

if(MasterScene.CheckLogoImageName())
	{
	bool SigInvalid = false;

	// check signature so it can't be changed
 	#ifdef NV_CHECK_SIGNATURES
	SigInvalid = true;
	if(CheckDependentBinaryFileSignature(MasterScene.GetLogoImageSig(), MasterScene.GetLogoImageName()))
		{
		SigInvalid = false;
		} // if
	#endif // NV_CHECK_SIGNATURES
	if(!SigInvalid)
		{
		osg::ref_ptr<osg::Image> LogoImage;
		LogoImage = osgDB::readImageFile(MasterScene.GetLogoImageName());
		if(LogoImage->s() > 1 && LogoImage->t() > 1)
			{
			unsigned long int LogoTop, LogoBottom;
			osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
			osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array;
			float ImageWidthScaled = (float)LogoImage->s() / (float)LogoImage->t();
			ImageWidthScaled *= (float)(YMax);
			FinalLogoWidth = ImageWidthScaled; 
			texcoord->push_back(osg::Vec2(0.0f, 1.0f));
			texcoord->push_back(osg::Vec2(0.0f, 0.0f));
			texcoord->push_back(osg::Vec2(1.0f, 0.0f));
			texcoord->push_back(osg::Vec2(1.0f, 1.0f));
			
			LogoTop    = YMax;
			LogoBottom = YMin;
			if(MasterScene.GetLogoAlign() == NVScene::Left)
				{
				LogoLeft   = XMin + HalfText;
				LogoRight  = LogoLeft + (ImageWidthScaled);
				MapRight  = XMax - HalfText;
				} // if
			else
				{
				LogoLeft   = XMax - (ImageWidthScaled);
				LogoRight  = XMax;
				MapRight  = LogoLeft - HalfText;
				} // else
			
			vertices->push_back(osg::Vec3(LogoLeft,LogoTop,-1.0f));
			vertices->push_back(osg::Vec3(LogoLeft,LogoBottom,-1.0f));
			vertices->push_back(osg::Vec3(LogoRight,LogoBottom,-1.0f));
			vertices->push_back(osg::Vec3(LogoRight,LogoTop,-1.0f));
			
			geom->setVertexArray(vertices.get());
			geom->setTexCoordArray( 0, texcoord.get() );

			osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
			normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
			geom->setNormalArray(normals.get());
			geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

			osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
			colors->push_back(osg::Vec4(1.0f,1.0,1.0f,1.0f));
			geom->setColorArray(colors.get());
			geom->setColorBinding(osg::Geometry::BIND_OVERALL);

			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
        
			osg::StateSet* stateset = geom->getOrCreateStateSet();
			stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
			stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

			osg::ref_ptr<Texture2D> tex = new Texture2D;
			tex->setImage(LogoImage.get());
			tex->setWrap( Texture2D::WRAP_S, Texture2D::CLAMP );
			tex->setWrap( Texture2D::WRAP_T, Texture2D::CLAMP );
			stateset->setTextureAttributeAndModes(0, tex.get(), StateAttribute::ON );
			stateset->setTextureAttribute(0, new TexEnv );

			geode->addDrawable(geom.get());
			} // if
		} // if
    } // if creating logo image

if(MasterScene.Overlay.GetMapOn())
	{
	unsigned long int MapLeft, MapTop, MapBottom, MapWidth, MapHeight;
	if(MasterScene.Overlay.GetMapNailImage() && MasterScene.Overlay.GetMapNailImage()->s() > 1 && MasterScene.Overlay.GetMapNailImage()->t() > 1)
		{
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array;
		double XR, YR;
		XR = MasterScene.GetTerXRange();
		YR = MasterScene.GetTerYRange();
		MapHeight = YMax - YMargin; // leave margin around map on all sides
		float ImageWidthScaled = (float)XR / (float)YR;
		ImageWidthScaled *= (float)(MapHeight);
		MapWidth  = ImageWidthScaled;

		texcoord->push_back(osg::Vec2(0.0f, 1.0f));
		texcoord->push_back(osg::Vec2(0.0f, 0.0f));
		texcoord->push_back(osg::Vec2(1.0f, 0.0f));
		texcoord->push_back(osg::Vec2(1.0f, 1.0f));

		//MapRight  = LogoLeft - HalfText; // calculated above, so it can adjust to LOGO placement
		MapLeft   = MapRight - MapWidth;
		MapTop    = YMax - HalfText; // leave margin around map on all sides
		MapBottom = YMin + HalfText; // leave margin around map on all sides

		vertices->push_back(osg::Vec3(MapLeft,MapTop,-0.2f));
		vertices->push_back(osg::Vec3(MapLeft,MapBottom,-0.2f));
		vertices->push_back(osg::Vec3(MapRight,MapBottom,-0.2f));
		vertices->push_back(osg::Vec3(MapRight,MapTop,-0.2f));
		geom->setVertexArray(vertices.get());
		geom->setTexCoordArray( 0, texcoord.get() );

		osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
		geom->setNormalArray(normals.get());
		geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f,1.0,1.0f,1.0f));
		geom->setColorArray(colors.get());
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);

		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
		osg::StateSet* stateset = geom->getOrCreateStateSet();
		stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
		stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

		osg::ref_ptr<Texture2D> tex = new Texture2D;
		tex->setImage(MasterScene.Overlay.GetMapNailImage());
		tex->setWrap( Texture2D::WRAP_S, Texture2D::CLAMP );
		tex->setWrap( Texture2D::WRAP_T, Texture2D::CLAMP );
		stateset->setTextureAttributeAndModes(0, tex.get(), StateAttribute::ON );
		stateset->setTextureAttribute(0, new TexEnv );

		geode->addDrawable(geom.get());

		// add a marker image
		osg::ref_ptr<osg::Image> MarkerImage;
		MarkerImage = ExportAndLoadImage("RedArrow38x38.png", RedArrow38x38_png_rawData, REDARROW38X38_PNG_RAWDATA_SIZE);
		//MarkerImage = ExportAndLoadImage("RedDot7x7.png", RedDot7x7_png_rawData, REDDOT7X7_PNG_RAWDATA_SIZE);
		if(MarkerImage->s() > 1 && MarkerImage->t() > 1)
			{
			float ImageHeight, MarkerWidth, MarkerHeight, MarkerX, MarkerY, MarkerXPct = 0, MarkerYPct = 0;
			osg::ref_ptr<osg::Geometry> mgeom = new osg::Geometry;
			osg::ref_ptr<osg::Vec3Array> mvertices = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec2Array> mtexcoord = new osg::Vec2Array;
			MarkerWidth = MarkerHeight = 15;

			mtexcoord->push_back(osg::Vec2(0.0f, 1.0f));
			mtexcoord->push_back(osg::Vec2(0.0f, 0.0f));
			mtexcoord->push_back(osg::Vec2(1.0f, 0.0f));
			mtexcoord->push_back(osg::Vec2(1.0f, 1.0f));

			ImageHeight = (YMax - HalfText) - (YMin + HalfText);
			MarkerX = MarkerXPct * ImageWidthScaled;
			MarkerY = MarkerYPct * ImageHeight;

			mvertices->push_back(osg::Vec3(-MarkerWidth * .5, MarkerHeight * .5,-0.2f));
			mvertices->push_back(osg::Vec3(-MarkerWidth * .5,-MarkerHeight * .5,-0.2f));
			mvertices->push_back(osg::Vec3( MarkerWidth * .5,-MarkerHeight * .5,-0.2f));
			mvertices->push_back(osg::Vec3( MarkerWidth * .5, MarkerHeight * .5,-0.2f));

			mgeom->setVertexArray(mvertices.get());
			mgeom->setTexCoordArray( 0, mtexcoord.get() );

			osg::ref_ptr<osg::Vec3Array> mnormals = new osg::Vec3Array;
			mnormals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
			mgeom->setNormalArray(mnormals.get());
			mgeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

			osg::ref_ptr<osg::Vec4Array> mcolors = new osg::Vec4Array;
			mcolors->push_back(osg::Vec4(1.0f,1.0,1.0f,1.0f));
			mgeom->setColorArray(mcolors.get());
			mgeom->setColorBinding(osg::Geometry::BIND_OVERALL);

			mgeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
			osg::StateSet* mstateset = mgeom->getOrCreateStateSet();
			mstateset->setMode(GL_BLEND,osg::StateAttribute::ON);
		    mstateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
			mstateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
			mstateset->setRenderBinDetails(18,"HUD3");

			osg::ref_ptr<Texture2D> mtex = new Texture2D;
			mtex->setImage(MarkerImage.get());
			mtex->setWrap( Texture2D::WRAP_S, Texture2D::CLAMP );
			mtex->setWrap( Texture2D::WRAP_T, Texture2D::CLAMP );
			mstateset->setTextureAttributeAndModes(0, mtex.get(), StateAttribute::ON );
			mstateset->setTextureAttribute(0, new TexEnv );

			PAT = new osg::PositionAttitudeTransform;

		    osg::ref_ptr<osg::Geode> markergeode = new osg::Geode();
			markergeode->addDrawable(mgeom.get());
			PAT->addChild(markergeode.get());

			// add PAT below
			//PAT->setPosition(osg::Vec3( MapLeft, MapTop, 0.0));
			PAT->setPivotPoint(osg::Vec3( 0, 0, 0.0));
			MasterScene.Overlay.SetMapPointerTransform(PAT.get());
			MasterScene.Overlay.SetMapHUDBounds(MapTop, MapLeft, MapBottom, MapRight);
			//PAT->setAttitude(osg::Quat(-osg::DegreesToRadians(90.0f), osg::Vec3(0, 0, 1)));
			} // if
		}
    } // if creating map on overlay


    // create the hud.
    osg::ref_ptr<osg::MatrixTransform> modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
	// do something with PAT (marker transform)
	if(PAT.valid())
		{
		modelview_abs->addChild(PAT.get());
		} // if

    modelview_abs->addChild(geode.get());


    osg::Projection * projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,NVW_NVOVERLAY_PSEUDO_SCREENSIZE_X,0,NVW_NVOVERLAY_PSEUDO_SCREENSIZE_Y));
    projection->addChild(modelview_abs.get());

    return projection;

} // createHUD

