// SpiclopsSupport.cpp
// Code to support eLumens dome systems via their spiclops DLL
// Some code taken from osgspiclops.cpp by Farshid Lashkari


#ifdef NVW_SUPPORT_SPICLOPS
#include <string.h>

#include "SpiclopsSupport.h"


static SPIDLLLoader DLLLoader; // this will acquire the DLL linkage for us at runtime

bool QuerySPIDLLAvailable(void)
{
return(DLLLoader.GetInitOK());
} // QuerySPIDLLAvailable


// SPICLOPS settings
static unsigned long int _SPIChannels = 1, _SPIMode = SPI_PF_AUTO;
static bool _SPIEnabled = false;

unsigned long int GetSPIChannels(void)
{
return(_SPIChannels);
} // GetSPIChannels

unsigned long int GetSPIMode(void)
{
return(_SPIMode);
} // GetSPIMode

bool GetSPIEnabled(void)
{
return(_SPIEnabled);
} // GetSPIEnabled


void SetSPIChannels(unsigned long int NewChannels)
{
_SPIChannels = NewChannels;
} // SetSPIChannels


void SetSPIMode(unsigned long int NewMode)
{
_SPIMode = NewMode;
} // SetSPIMode


void SetSPIModeFromString(const char *ModeString)
{
if(!stricmp(ModeString, "NONE"))
	{
	SetSPIMode(SPI_PF_NONE);
	} // if
else if(!stricmp(ModeString, "BACKBUFFER"))
	{
	SetSPIMode(SPI_PF_BACKBUFFER);
	} // if
else if(!stricmp(ModeString, "NORMAL"))
	{
	SetSPIMode(SPI_PF_NORMAL);
	} // if
else if(!stricmp(ModeString, "PBUFFER"))
	{
	SetSPIMode(SPI_PF_PBUFFER);
	} // if
else if(!stricmp(ModeString, "TEXTURE"))
	{
	SetSPIMode(SPI_PF_TEXTURE);
	} // if
else if(!stricmp(ModeString, "AUTO"))
	{
	SetSPIMode(SPI_PF_AUTO);
	} // if
else if(!stricmp(ModeString, "STEREO"))
	{
	SetSPIMode(SPI_PF_STEREO);
	} // if
			
} // SetSPIModeFromString



void SetSPIEnabled(bool NewEnabled)
{
_SPIEnabled = NewEnabled;
} // SetSPIMode


// not  used currently
/*
unsigned long int MapChannelNumToChannelVal(unsigned long int ChannelNum)
{
switch(ChannelNum)
	{
	case 1: return(SPI_PF_1_CHAN);
	case 2: return(SPI_PF_2_CHAN);
	case 3: return(SPI_PF_3_CHAN);
	case 4: return(SPI_PF_4_CHAN);
	default: return(SPI_PF_1_CHAN);
	} // ChannelNum

return(SPI_PF_1_CHAN);
} // MapChannelNumToChannelVal
*/

class ElumensRenderCallback : public Producer::Camera::Callback
{
public:

	ElumensRenderCallback(Producer::Camera *cam, void *context, int channel, bool firstchannel, bool lastchannel)
		: m_camera(cam)
		, m_context(context)
		, m_channel(channel)
		, m_firstChannel(firstchannel)
		, m_lastChannel(lastchannel)
		, m_preDraw(true)
	{}

	void operator()(const Producer::Camera &cam );

protected:
	Producer::ref_ptr<Producer::Camera> m_camera;
	void *m_context;
	int m_channel;
	bool m_firstChannel, m_lastChannel;
	bool m_preDraw;
	osg::Matrixd m_lastProjectionMatrix;
	osg::Matrixd m_lastModelviewMatrix;
	float m_viewportX,m_viewportY,m_viewportW,m_viewportH;
};

void ElumensRenderCallback::operator()(const Producer::Camera &cam )
{
	static double projMx[16];
	int _x, _y;
	unsigned int _w, _h;

	if(m_preDraw) {

		//Get the window dimensions
		m_camera->getRenderSurface()->getWindowRectangle( _x, _y, _w, _h );

		//Save current projection/modelview matrix and viewport
		m_lastProjectionMatrix.set(m_camera->getProjectionMatrix());
		m_lastModelviewMatrix.set(m_camera->getViewMatrix());
		m_camera->getProjectionRectangle(m_viewportX,m_viewportY,m_viewportW,m_viewportH);

		if(m_firstChannel) {

			//Apply producer projection matrix to gl projection matrix
			glMatrixMode(GL_PROJECTION);
			glLoadMatrix(m_lastProjectionMatrix.ptr());

			//Apply producer view matrix to gl modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrix(m_lastModelviewMatrix.ptr());

			//Need to set viewport to fullscreen so that it is restored when spiFlush is called
			glViewport(0,0,_w,_h);

			//This is the first channel, so we need to call spiBegin
			DLLLoader.dllspiBegin( m_context );
		}

		//Prepare for rendering on this channel
		DLLLoader.dllspiPreRender( m_context, m_channel );

		//Need to apply modified gl projection matrix back to producer camera
		glGetDoublev( GL_PROJECTION_MATRIX, projMx );
		//I modified Producer::Lens so that you can manually set the projection matrix
		//If you want to get this compiled you'll have to modify it also, or find another
		//way of doing this.
		m_camera->getLens()->setMatrix(projMx);

		//Need to apply modified gl viewport to producer camera
		glGetDoublev( GL_VIEWPORT, projMx );
		//Producer adjusts the projection rectangle values using the current window rectangle,
		//so we need to account for it.
		m_camera->setProjectionRectangle((int)projMx[0]+_x,(int)projMx[1]+_y,(unsigned int)projMx[2],(unsigned int)projMx[3]);

	} else {

		//Cleanup rendering for this channel
		DLLLoader.dllspiPostRender( m_context, m_channel );

		if(m_lastChannel) {

			//This is the last channel, need to call spiEnd
			DLLLoader.dllspiEnd( m_context );

			//Clear color and depth buffer
			glClearColor( 0.0f, 0.0f, 0.4f, 1.0f);
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Flush all the channels onto the screen
			DLLLoader.dllspiFlush( m_context, SPI_ALL_CHAN );
		}

		//Restore projection/modelview matrix and viewport
		m_camera->getLens()->setMatrix(m_lastProjectionMatrix.ptr());
		m_camera->setViewByMatrix(m_lastModelviewMatrix.ptr());
		m_camera->setProjectionRectangle(m_viewportX,m_viewportY,m_viewportW,m_viewportH);

		//Update scene view projection/modelview matrix and viewport
		osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(m_camera->getSceneHandler());
		osgUtil::SceneView* sv = sh ? sh->getSceneView() : 0;
		if(sv != 0) {
			sv->setProjectionMatrix(m_lastProjectionMatrix);
			sv->setViewMatrix(m_lastModelviewMatrix);
			m_camera->getProjectionRectangle(_x, _y, _w, _h);
			sv->setViewport(_x, _y, _w, _h);
		}
	}

	//Toggle between predraw and postdraw
	m_preDraw = !m_preDraw;
}

//Convert 0 based index to spi channel ID
int GetChannel(int value, int numChannels)
{
	switch(numChannels) {
		case 1:
			return 1;
			break;
		case 2:
			return 2 << value;
			break;
		case 3:
		case 4:
			return 8 << value;
			break;
		default:
			break;
	}

	return 0;
}

void initSpiclops(void *context, int numChannels, osgProducer::Viewer *viewer)
{
	//Create specified number of channels
	for(unsigned int i = 0; i < (unsigned long int)numChannels; i++) {

		int channel = GetChannel(i,numChannels);

		// set the origin of the channels viewport in screen space
		DLLLoader.dllspiSetChanOrigin( context, channel, 0, 0 );

		// set the width & height of the channel's viewport in screenspace
		DLLLoader.dllspiSetChanSize( context, channel, 1024, 1024 );

		// set the TextureID
		DLLLoader.dllspiSetChanTextureID( context, channel, 999 - i);

		// set the channels field of view, horiz & vert.
		DLLLoader.dllspiSetChanFOV( context, channel, 90, 90 );

		// set the granularity of the channel's mesh.
		DLLLoader.dllspiSetChanTessLevel( context, channel, 21 );

		Producer::Camera *cam;

		if(i == 0) {
			//Get camera that is already created
			cam = viewer->getCamera(0);
			osgProducer::OsgSceneHandler *sh = dynamic_cast<osgProducer::OsgSceneHandler*>(cam->getSceneHandler());
			//Need to disable automatic clip plane computation
			//if(sh) sh->getSceneView()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR); // this doesn't seem to work
			viewer->getCullSettings().setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR); // this does! // THIS does!
		} else {
			//This is not the first camera
			cam = new Producer::Camera();
			
			//Get the original producer camera
			Producer::Camera *pcam1 = viewer->getCamera(0);

			//Set the new cameras render surface to the same render surface as the original camera
			cam->setRenderSurface(pcam1->getRenderSurface());

			//Set scene handler to original scene handler
			cam->setSceneHandler(pcam1->getSceneHandler());

			//Create a unique camera name, based on the current number of cameras
			char camName[20];
			sprintf(camName,"window%d",viewer->getNumberOfCameras());

			//Add the camera to the viewer
			viewer->getCameraConfig()->addCamera(camName,cam);

			//Cover entire screen
			cam->setProjectionRectangle(0.0f,1.0f,1.0f,0.0f);
		}

		//Determine whether this is the first,middle,or last channel
		bool First = false, Last = false;

		// if channels == 1, a channel can be both first and last
		if(i == 0) {
			First = true;
		}
		if(i == numChannels-1) {
			Last = true;
		}

		//Create a callback for this channel
		ElumensRenderCallback *cb = new ElumensRenderCallback( cam, context, channel, First, Last );

		//Add elumens pre/post render callbacks
		cam->addPreCullCallback( cb );
		cam->addPostDrawCallback( cb );
	}
}




void InitSpiclopsCallback::operator()( const Producer::RenderSurface &rs )
{
// create a spi context with specified number of channels and pixel format
m_context = DLLLoader.dllspiInitialize( NULL, 256 << m_numChannels | m_pixelFormat );
}


SPIDLLLoader::SPIDLLLoader()
{
InitOK = false;
SPIDLLHM = NULL;

dllspiInitialize = NULL;
dllspiPreRender = NULL;
dllspiPostRender = NULL;
dllspiFlush = NULL;
dllspiBegin = NULL;
dllspiEnd = NULL;
dllspiSetChanTextureID = NULL;
dllspiSetChanOrigin = NULL;
dllspiSetChanSize = NULL;
dllspiSetChanFOV = NULL;
dllspiSetChanTessLevel = NULL;

#ifdef _DEBUG
if(SPIDLLHM = LoadLibrary("spiclopsD.dll"))
#else //!_DEBUG
if(SPIDLLHM = LoadLibrary("spiclops.dll"))
#endif // !_DEBUG
	{
	dllspiInitialize = (DLLSPIINITIALIZE)GetProcAddress(SPIDLLHM, "spiInitialize");
	dllspiPreRender = (DLLSPIPRERENDER)GetProcAddress(SPIDLLHM, "spiPreRender");
	dllspiPostRender = (DLLSPIPOSTRENDER)GetProcAddress(SPIDLLHM, "spiPostRender");
	dllspiFlush = (DLLSPIFLUSH)GetProcAddress(SPIDLLHM, "spiFlush");
	dllspiBegin = (DLLSPIBEGIN)GetProcAddress(SPIDLLHM, "spiBegin");
	dllspiEnd = (DLLSPIEND)GetProcAddress(SPIDLLHM, "spiEnd");
	dllspiSetChanTextureID = (DLLSPISETCHANTEXTUREID)GetProcAddress(SPIDLLHM, "spiSetChanTextureID");

	dllspiSetChanOrigin = (DLLSPISETCHANORIGIN)GetProcAddress(SPIDLLHM, "spiSetChanOrigin");
	dllspiSetChanSize = (DLLSPISETCHANSIZE)GetProcAddress(SPIDLLHM, "spiSetChanSize");
	dllspiSetChanFOV = (DLLSPISETCHANFOV)GetProcAddress(SPIDLLHM, "spiSetChanFOV");
	dllspiSetChanTessLevel = (DLLSPISETCHANTESSLEVEL)GetProcAddress(SPIDLLHM, "spiSetChanTessLevel");
	
	if(dllspiInitialize && dllspiPreRender && dllspiPostRender && dllspiFlush && dllspiBegin && dllspiEnd &&
	 dllspiSetChanTextureID && dllspiSetChanOrigin && dllspiSetChanSize && dllspiSetChanFOV && dllspiSetChanTessLevel)
		{
		InitOK = true;
		} // if
	} // if

} // SPIDLLLoader::SPIDLLLoader

SPIDLLLoader::~SPIDLLLoader()
{
if(SPIDLLHM)
	{
	FreeLibrary(SPIDLLHM);
	SPIDLLHM = NULL;
	} // if

} // SPIDLLLoader::~SPIDLLLoader


#endif // NVW_SUPPORT_SPICLOPS
