
#include "Viewer.h"

osgProducer::Viewer *viewer;

osgProducer::Viewer *GetGlobalViewer(void)
{
return(viewer);
} // GetGlobalViewer

void SetGlobalViewer(osgProducer::Viewer *NewViewer)
{
viewer = NewViewer;
} // SetGlobalViewer


HWND GetGlobalViewerHWND(void)
{
HWND ViewerNativeWindow = NULL;
// Find Viewer's HWND
for( unsigned int i = 0; i <GetGlobalViewer()->getCameraConfig()->getNumberOfCameras(); ++i )
	{
    Producer::Camera* cam = GetGlobalViewer()->getCameraConfig()->getCamera(i);
    Producer::RenderSurface* rs = cam->getRenderSurface();
	ViewerNativeWindow = rs->getWindow();
	} // for

return(ViewerNativeWindow);
} // GetGlobalViewerHWND

bool GetGlobalViewerDims(unsigned long int &Width, unsigned long int &Height)
{
HWND ViewerHWND;
RECT WindowRect;

if(ViewerHWND = GetGlobalViewerHWND())
	{
	if(GetWindowRect(ViewerHWND, &WindowRect))
		{
		Width = WindowRect.right - WindowRect.left;
		Height = WindowRect.bottom - WindowRect.top;
		return(true);
		} // if
	} // if
return(false);

} // GetGlobalViewerDims

unsigned int GetGlobalViewerWidth(void)
{
unsigned long int Width = 0, Height = 0;
GetGlobalViewerDims(Width, Height);
return(Width);
} // GetGlobalViewerWidth

unsigned int GetGlobalViewerHeight(void)
{
unsigned long int Width = 0, Height = 0;
GetGlobalViewerDims(Width, Height);
return(Height);
} // GetGlobalViewerHeight



bool GetGlobalViewerCoords(signed long int &X, signed long int &Y)
{
HWND ViewerHWND;

if(ViewerHWND = GetGlobalViewerHWND())
	{
	POINT MapPt;
	MapPt.x = MapPt.y = 0;
	if(ClientToScreen(ViewerHWND, &MapPt))
		{
		X = MapPt.x;
		Y = MapPt.y;
		return(true);
		} // if
	} // if
return(false);

} // GetGlobalViewerCoords

signed int GetGlobalViewerX(void)
{
signed long int X = 0, Y = 0;
GetGlobalViewerCoords(X, Y);
return(X);
} // GetGlobalViewerX

signed int GetGlobalViewerY(void)
{
signed long int X = 0, Y = 0;
GetGlobalViewerCoords(X, Y);
return(Y);
} // GetGlobalViewerY



void SignalApplicationExit(void) {if(viewer) viewer->setDone(true);};
