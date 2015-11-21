// NVSky.cpp
// Code relating to the SkyDome
// mostly taken from main.cpp on 1/31/06 by CXH

#include <osg/ref_ptr> 
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osg/TexEnv>
#include <osg/Depth>


#include "NVScene.h"
#include "NVSky.h"
#include "NVSigCheck.h"
#include "NVNodeMasks.h"
extern NVScene MasterScene;
osg::ref_ptr<osg::ClearNode> clearNode; 


Node *makeFullSky( float SkyRadius, const char *SkyTexFileName, const char *SkyTexSig);

Node *makeSkyandStructure( float SkyRadius)
{
clearNode = new osg::ClearNode;
osg::Node *SkyNode;

clearNode->setRequiresClear(false); // we've got base and sky to do it.

// use a transform to make the sky and base around with the eye point.
osg::Transform* transform = new MoveSkyDomeWithEyePointTransform;

// transform's value isn't known until in the cull traversal so its bounding
// volume is can't be determined, therefore culling will be invalid,
// so switch it off, this causes all of our parents to switch culling
// off as well. But don't worry culling will be back on once underneath
// this node or any other branch above this transform.
transform->setCullingActive(false);

// add the sky-nadir dome to the transform
transform->addChild(SkyNode = makeFullSky(SkyRadius, MasterScene.GetSkyFileName(), MasterScene.GetSkySig()));  // bin number -2 so drawn first.
if(SkyNode)
	{
	SkyNode->setNodeMask(NVW_NODEMASK_INTANGIBLE | NVW_NODEMASK_SKY);
	} // if

// add the transform to the clear node
clearNode->addChild(transform);

// prevent skydome from being added to near/far bounding dimensions during cull
clearNode->setUpdateCallback(new MyCullCallback);

return(clearNode.get());
} // makeSkyandStructure


Node *makeFullSky( float SkyRadius, const char *SkyTexFileName, const char *SkyTexSig)
{
    int i, j;
    float lev[] = {-90.0, -75.0, -60.0, -45.0, -30.0, -15.0, 0.0, 15.0, 30.0, 45.0, 60.0, 75.0, 90.0 };
    float x, y, z;
    float alpha, theta;
    int nlev = sizeof( lev )/sizeof(float);


#ifdef NV_CHECK_SIGNATURES
if(!CheckDependentBinaryFileSignature(MasterScene.GetSkySig(), SkyTexFileName))
	{
	SetGlobalSigInvalid(true);
	// Hard to back out of creating a sky, so just don't texture it
	SkyTexFileName = "";
	//return(NULL);
	} // if

#endif // NV_CHECK_SIGNATURES


    Geometry *geom = new Geometry;

    Vec3Array& coords = *(new Vec3Array(19*nlev));
    Vec4Array& colors = *(new Vec4Array(1));
    Vec2Array& tcoords = *(new Vec2Array(19*nlev));

    colors[0][0] = 1.0;
    colors[0][1] = 1.0;
    colors[0][2] = 1.0;
    colors[0][3] = 1.0;    
    
    int ci = 0;

    for( i = 0; i < nlev; i++ )
    {
        for( j = 0; j <= 18; j++ )
        {
            alpha = osg::DegreesToRadians(lev[i]);
            theta = osg::DegreesToRadians((float)(j*20));

            x = SkyRadius * cosf( alpha ) * cosf( theta );
            y = SkyRadius * cosf( alpha ) * -sinf( theta );
            z = SkyRadius * sinf( alpha );

            coords[ci][0] = x;
            coords[ci][1] = y;
            coords[ci][2] = z;

            tcoords[ci][0] = (float)j/18.0;
            tcoords[ci][1] = (float)i/(float)(nlev-1);
            ci++;
        }

    }

    for( i = 0; i < nlev-1; i++ )
    {
        DrawElementsUShort* drawElements = new DrawElementsUShort(PrimitiveSet::TRIANGLE_STRIP);
        drawElements->reserve(38);

        for( j = 0; j <= 18; j++ )
        {
            drawElements->push_back((i+1)*19+j);
            drawElements->push_back((i+0)*19+j);
        }

        geom->addPrimitiveSet(drawElements);
    }
    
    geom->setVertexArray( &coords );
    geom->setTexCoordArray( 0, &tcoords );

    geom->setColorArray( &colors );
    geom->setColorBinding( Geometry::BIND_OVERALL );


    Texture2D *tex = new Texture2D;
    tex->setImage(osgDB::readImageFile(SkyTexFileName));
	tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
	tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE ); // Falls back to CLAMP if unavailable
	// sky texture compression seems to be a lot of ugliness for not much gain at the moment.
	//if(MasterScene.SceneLOD.CheckCompressTerrainTex())
	//	{
	//	tex->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
	//	} // if

    StateSet *dstate = new StateSet;

    dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );
    dstate->setTextureAttribute(0, new TexEnv );
    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
    dstate->setMode( GL_CULL_FACE, StateAttribute::ON );
    

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0,1.0);   
    dstate->setAttributeAndModes(depth,StateAttribute::ON );

    dstate->setRenderBinDetails(-2,"RenderBin");

    geom->setStateSet( dstate );

    Geode *geode = new Geode;
    geode->addDrawable( geom );

    geode->setName( "FullSky" );

    return geode;
} // makeFullSky

