
#include <osg/Geode>
#include <osgText/Text>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/PositionAttitudeTransform>

#include "IdentityDefines.h"
#include "CreateHelpSplashWatermark.h"
#include "InternalImage.h"
#include "NVSigCheck.h"
#include "NVScene.h"

extern NVScene MasterScene;

using namespace osg;

// Pick up externally-compiled logo image
// believe it or not, this extern "C" is required...
extern "C" {
#define NVELOGO256X128_PNG_RAWDATA_SIZE		5692
extern unsigned char NVELogo256x128_png_rawData[NVELOGO256X128_PNG_RAWDATA_SIZE];
} // extern "C"


bool SplashDisplayed = false;

void SetSplashDisplayed(bool NewState)
{
SplashDisplayed = NewState;
} // SetSplashDisplayed
 
bool GetSplashDisplayed(void)
{
return(SplashDisplayed);
} // GetSplashDisplayed




osg::Node* createSplash()
{
int HideNameText = 1;

	float XMin = 0, XMax = 1280, XMid = 640;
	float YMin = 0, YMax = 1024, YMid = 600;
	osg::ref_ptr<osg::Image> LogoImage;
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

	LogoImage = ExportAndLoadImage("NVELogo256x128.png", NVELogo256x128_png_rawData, NVELOGO256X128_PNG_RAWDATA_SIZE);

	if(LogoImage.valid() && LogoImage->s() > 1 && LogoImage->t() > 1)
		{
		int ImageW, ImageH, HalfImageW, HalfImageH;
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array;
		ImageW = LogoImage->s();
		ImageH = LogoImage->t();
		HalfImageW = ImageW / 2;
		HalfImageH = ImageH / 2;
		texcoord->push_back(osg::Vec2(0.0f, 1.0f)); // UL
		texcoord->push_back(osg::Vec2(0.0f, 0.0f)); // LL 
		texcoord->push_back(osg::Vec2(1.0f, 0.0f)); // LR
		texcoord->push_back(osg::Vec2(1.0f, 1.0f)); // UR
		vertices->push_back(osg::Vec3(XMid - HalfImageW,YMid + HalfImageH,-0.1f)); // UL
		vertices->push_back(osg::Vec3(XMid - HalfImageW,YMid - HalfImageH,-0.1f)); // LL
		vertices->push_back(osg::Vec3(XMid + HalfImageW,YMid - HalfImageH,-0.1f)); // LR
		vertices->push_back(osg::Vec3(XMid + HalfImageW,YMid + HalfImageH,-0.1f)); // UR
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
	else
		{
		HideNameText = 0;
		} // else


    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // disable depth test, and make sure that the watermark is drawn after everything 
    // else so that it always appears ontop.
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11,"RenderBin");

    osg::Vec3 position(640.0f,550.0f,0.0f);
    osg::Vec3 shadowdelta(2.0f,-2.0f,-0.1f);
	osg::Vec3 delta(0.0f,-36.0f,0.0f);

char *SplashSheet[] =
	{
	NVW_NATUREVIEW_NAMETEXT, // we may start at 1 if HideNameText is 1
	"Version "NVW_VIEWER_VERSIONTEXT,
	NVW_NATUREVIEW_COPYRIGHTTEXT,
	//NVW_NATUREVIEW_FLAVORTEXT,
	"",
	"Press H for help, Escape to exit.",
	"",
	"Mouse Controls",
	"Hold down left mouse button",
	"and move mouse to drive.",
	"",
	NULL, // [11] might be replaced by blank line if Sigs invalid
	NULL, // [12] might be replaced by blank line if Sigs invalid
	NULL, // must be here to end things
	}; // SplashSheet

	if(GetGlobalSigInvalid())
		{
		SplashSheet[11] = "";
		SplashSheet[12] = "(Note: Some signatures are invalid.)";
		} // if

// if HideNameText == 1, we skip the first entry (the textual name)
for(int SplashLine = HideNameText; SplashSheet[SplashLine]; SplashLine++)
	{
		{ // add main text on top
			osgText::Text* text = new  osgText::Text;
			geode->addDrawable( text );
			text->setCharacterSize(32);
			text->setAlignment(osgText::Text::CENTER_TOP);
			text->setFont(timesFont);
			text->setPosition(position);
			text->setColor(osg::Vec4 (1.0, 1.0, 1.0, 1.0));
			text->setText(SplashSheet[SplashLine]);
		}    

		{ // add drop shadow behind for background contrast
			osgText::Text* text = new  osgText::Text;
			geode->addDrawable( text );
			text->setCharacterSize(32);
			text->setAlignment(osgText::Text::CENTER_TOP);
			text->setFont(timesFont);
			text->setPosition(position + shadowdelta);
			text->setColor(osg::Vec4 (0.0, 0.0, 0.0, 1.0));
			text->setText(SplashSheet[SplashLine]);
			position += delta;
		}    
	} // for

    // create the splash.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;

} // createSplash



osg::Node* createWatermark()
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // disable depth test, and make sure that the watermark is drawn after everything 
    // else so that it always appears ontop.
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11,"RenderBin");

    osg::Vec3 position(640.0f,800.0f,0.0f);
    osg::Vec3 delta(0.0f,-60.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        text->setCharacterSize(32);
		text->setAlignment(osgText::Text::CENTER_TOP);
        text->setFont(timesFont);
        text->setPosition(position);
		text->setColor(osg::Vec4 (1.0, 0.0, 0.0, 1.0));
		text->setText(MasterScene.Watermark.GetWatermarkText());
        position += delta;
    }    

    {
        osg::BoundingBox bb;
        for(unsigned int i=0;i<geode->getNumDrawables();++i)
        {
            bb.expandBy(geode->getDrawable(i)->getBound());
        }

        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* vertices = new osg::Vec3Array;
        float depth = bb.zMin()-0.1;
		float XMargin = 50;
		float YMargin = 50;
        vertices->push_back(osg::Vec3(bb.xMin() - XMargin,bb.yMax() + YMargin,depth));
        vertices->push_back(osg::Vec3(bb.xMin() - XMargin,bb.yMin() - YMargin,depth));
        vertices->push_back(osg::Vec3(bb.xMax() + XMargin,bb.yMin() - YMargin,depth));
        vertices->push_back(osg::Vec3(bb.xMax() + XMargin,bb.yMax() + YMargin,depth));
        geom->setVertexArray(vertices);

        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
        geom->setNormalArray(normals);
        geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0,1.0f,0.1f));
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
        
        osg::StateSet* stateset = geom->getOrCreateStateSet();
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

        geode->addDrawable(geom);
    }

    // create the watermark.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;

} // createWatermark

