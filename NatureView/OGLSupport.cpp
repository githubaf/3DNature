// OGLSupport.cpp
// Some misc support code for OpenGL that didn't fit elsewhere

#include <windows.h>
#include <gl/gl.h>
#include <string.h>

// Code from this cited message:
// Date: Tue, 21 Mar 2006 21:07:35 -0800
// From: "Farshid Lashkari" <flashk@gmail.com>
// To: "osg users" <osg-users@openscenegraph.net>
// Subject: Re: [osg-users] Can we toggle block on vsync using a RenderSurface and SceneView?

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = NULL;

// Enable (1) or disable (0) VSync
// I believe this must be called with a valid GL context
void setVSync(int interval)
{
const unsigned char *extensions = glGetString( GL_EXTENSIONS );

if( extensions == 0 || strstr( (const char *)extensions, "WGL_EXT_swap_control" ) == 0 ) return;

wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT" );
if( wglSwapIntervalEXT ) wglSwapIntervalEXT(interval);
} // setVSync