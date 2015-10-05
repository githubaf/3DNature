// ViewGUI.cpp
// Built from scratch and from ragged chunks of MapViewGUI.cpp/CamViewGUI.cpp
// Started on 10/22/99 (four days to my 28th birthday!) by CXH
// Copyright 1996-1999

#include "stdafx.h"
#include "ViewGUI.h"
#include "Conservatory.h"
#include "Project.h"
#include "Interactive.h"
#include "Toolbar.h"
#include "resource.h"
#include "Security.h"
#include "Joe.h"
#include "MathSupport.h"
#include "EffectsLib.h"
#include "Render.h"
#include "DEM.h"
#include "AppMem.h"
#include "Requester.h"
#include "DiagnosticGUI.h"
#include "DigitizeGUI.h"
#include "DEMEditGUI.h"
#include "DEMPaintGUI.h"
#include "WCSVersion.h"
#include "AppHelp.h"
#include "Raster.h"
#include "Realtime.h"
#include "Lists.h"
#include "DBFilterEvent.h"
#include "FeatureConfig.h"

//lint -save -e676

enum
	{
	WCS_X,
	WCS_Y,
	WCS_Z//,
	//WCS_W
	}; // 


#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif // GL_CLAMP_TO_EDGE


extern int GLClampEdgeAvailable;
extern NotifyTag ThawEvent[2];

//VertexDEM NullCenter;
int RTFLoaded;

// used in glQuadPolyUnitTexDefNorm()
static Point3d QPUTDFNormalA, QPUTDFNormalB, QPUTDFNormalC, QPUTDFNormalD, QPUTNormalFlat;


// fudging constants
#define WCS_VIEW_JOYSTICK_FULL_RANGE_PIXELS	50.0f
#define WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG 20000.0f

//#define WCS_TEST_WATERFLOW

#define WCS_VIEWGUI_SYNC_DELAY_MS	250 // 100ms = .1 sec

#ifdef WCS_TEST_WATERFLOW
#include "WaterFlow.h"
#endif // WCS_TEST_WATERFLOW

//#define WCS_VIEWGUI_STAGGER_POLYS
//#define WCS_VIEWGUI_BGREDRAW
#define WCS_VIEWGUI_INHIBIT_VEC_SIMPLIFY
#define WCS_VIEWGUI_INVERT
//#define WCS_VIEWGUI_3DOBJ_POLY_MAXPCT	0.45
//#define WCS_VIEWGUI_3DOBJ_POLY_MINPCT	0.25
#define WCS_VIEWGUI_VECTOR_DEPTHTEST
#define WCS_VIEWGUI_VECTOR_DEPTHSKEW
#define WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT 0.99995
//#define WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT 0.999	// Chris' original value - too low when near clip plane is closer

#define WCS_VIEWGUI_TEXGRAD_SIZE	128	// must be power of 2 for GL
#define WCS_VIEWGUI_TEXGRID_SIZE	16	// must be power of 2 for GL

//extern unsigned short Default8[];
//extern char BusyWinAbort;

static int DebugSelect, SuppressPlanAmbient;
static int GLWarned; // used to suppress all but the first warning about OpenGL issues

//char TempMsgBuf[80];

char *ViewPrefsGUI::TabNames[6] = {"General", "Terrain", "Land Cover/Water", "3D/Vec/Sky", "Light/Camera", "Overlay/Gradient"};
long ViewPrefsGUI::ActivePage;

char *ViewRTPrefsGUI::TabNames[2] = {"Display", "Output"};
long ViewRTPrefsGUI::ActivePage;

#define WCS_VIEWGUI_VPG_NUMLINES 12

WIDGETID VPGLabelIDs[13] = {NULL, IDC_LABEL1, IDC_LABEL2, IDC_LABEL3, IDC_LABEL4, IDC_LABEL5, IDC_LABEL6, IDC_LABEL7, IDC_LABEL8, IDC_LABEL9, IDC_LABEL10, IDC_LABEL11, IDC_LABEL12};
WIDGETID VPGItemIDs[13]  = {NULL, IDC_ITEM1, IDC_ITEM2, IDC_ITEM3, IDC_ITEM4, IDC_ITEM5, IDC_ITEM6, IDC_ITEM7, IDC_ITEM8, IDC_ITEM9, IDC_ITEM10, IDC_ITEM11, IDC_ITEM12};
WIDGETID VPGRealIDs[13]  = {NULL, IDC_CHECKREAL1, IDC_CHECKREAL2, IDC_CHECKREAL3, IDC_CHECKREAL4, IDC_CHECKREAL5, IDC_CHECKREAL6, IDC_CHECKREAL7, IDC_CHECKREAL8, IDC_CHECKREAL9, IDC_CHECKREAL10, IDC_CHECKREAL11, IDC_CHECKREAL12};
WIDGETID VPGRendIDs[13]  = {NULL, IDC_CHECKREND1, IDC_CHECKREND2, IDC_CHECKREND3, IDC_CHECKREND4, IDC_CHECKREND5, IDC_CHECKREND6, IDC_CHECKREND7, IDC_CHECKREND8, IDC_CHECKREND9, IDC_CHECKREND10, IDC_CHECKREND11, IDC_CHECKREND12};

static int CleanupNoSwap;
// Pick this up from Fenetre to clue us in on a different redraw strategy.
extern int GLReadPixelsFront;

#define gl_M_PI		Pi
#define gl_RAD(x) (((x)*gl_M_PI)/180.)

// this code is based on the same functions in billboard.c from
// http://www.sgi.com/software/opengl/advanced97/programs/programs.html

/*===========================================================================*/

void buildRot(float theta, float x, float y, float z, float m[16])
{
float d = x*x + y*y + z*z;
float ct = cosf((float)gl_RAD(theta)), st = sinf((float)gl_RAD(theta));

// normalize
if (d > 0) {
	d = 1/d;
	x *= d;
	y *= d;
	z *= d;
	} // if

m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;

/* R = uu' + cos(theta)*(I-uu') + sin(theta)*S
 *
 * S =  0  -z   y    u' = (x, y, z)
 *	    z   0  -x
 *	   -y   x   0
 */

 m[0] = x*x + ct*(1-x*x) + st*0;
 m[4] = x*y + ct*(0-x*y) + st*-z;
 m[8] = x*z + ct*(0-x*z) + st*y;

 m[1] = y*x + ct*(0-y*x) + st*z;
 m[5] = y*y + ct*(1-y*y) + st*0;
 m[9] = y*z + ct*(0-y*z) + st*-x;

 m[2] = z*x + ct*(0-z*x) + st*-y;
 m[6] = z*y + ct*(0-z*y) + st*x;
 m[10]= z*z + ct*(1-z*z) + st*0;

} // buildRot

/*===========================================================================*/

static void calcMatrix(float m[16])
{
float mat[16];

glGetFloatv(GL_MODELVIEW_MATRIX, mat);
buildRot((float)(-180*atan2f(mat[8], mat[10])/gl_M_PI), 0.0f, 1.0f, 0.0f, m);
//glMultMatrixf(m);

} // calcMatrix

/*===========================================================================*/

static void glTriPoly(Point3d A, Point3d B, Point3d C)
{
static Point3d Normal, Side[2];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(Normal, Side[0], Side[1]);

//SurfaceNormal(Normal, Vertices[0], Vertices[1], Vertices[2]);

glNormal3dv(Normal);

glVertex3dv(A);
glVertex3dv(B);
glVertex3dv(C);

} // glTriPoly

/*===========================================================================*/

/*** not used - FPW2 10/04/07
static void glTriPolySN(Point3d A, Point3d B, Point3d C, Point3d Normal)
{
glNormal3dv(Normal);

glVertex3dv(A);
glVertex3dv(B);
glVertex3dv(C);
} // glTriPolySN
***/

/*===========================================================================*/

static void glTriPolyVertexDEM(VertexDEM &A, VertexDEM &B, VertexDEM &C)
{
// use xyz for normal
glNormal3dv(A.xyz);
glVertex3dv(A.XYZ);

glNormal3dv(B.xyz);
glVertex3dv(B.XYZ);

glNormal3dv(C.xyz);
glVertex3dv(C.XYZ);

} // glTriPolyVertexDEM

/*===========================================================================*/

/*** not used - FPW2 10/04/07
static void glTriPolyVertexDEMTex(VertexDEM &A, VertexDEM &B, VertexDEM &C)
{
// use xyz for normal
// use ScrnXYZ[3] to store texture coords
glTexCoord3dv(A.ScrnXYZ);
glNormal3dv(A.xyz);
glVertex3dv(A.XYZ);

glTexCoord3dv(B.ScrnXYZ);
glNormal3dv(B.xyz);
glVertex3dv(B.XYZ);

glTexCoord3dv(C.ScrnXYZ);
glNormal3dv(C.xyz);
glVertex3dv(C.XYZ);
} // glTriPolyVertexDEMTex
***/

/*===========================================================================*/

static void glQuadPolyUnitTex(Point3d A, Point3d B, Point3d C, Point3d D, int FlipX)
{
// (numbers indicate texture coordinates)
//   0
//  A B
// 0   1
//  D C
//   1
//static Point3d Normal, Normal2, Side[3];

//FindPosVector(Side[0], B, A);
//FindPosVector(Side[1], C, A);
//FindPosVector(Side[2], D, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
//SurfaceNormal(Normal, Side[0], Side[1]);
//SurfaceNormal(Normal2, Side[2], Side[1]);

//AddPoint3d(Normal2, Normal);
//UnitVector(Normal);

//SurfaceNormal(Normal, Vertices[0], Vertices[1], Vertices[2]);

glNormal3dv(QPUTNormalFlat);

if (FlipX)
	{
	glTexCoord2f(1.0f, 0.0f);
	glVertex3dv(A);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3dv(B);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3dv(C);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3dv(D);
	} // if
else
	{
	glTexCoord2f(0.0f, 0.0f);
	glVertex3dv(A);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3dv(B);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3dv(C);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3dv(D);
	} // else

} // glQuadPolyUnitTex

/*===========================================================================*/

static void glQuadPolyUnitTexDefNorm(Point3d A, Point3d B, Point3d C, Point3d D, int FlipX)
{
// (numbers indicate texture coordinates)
//   0
//  A B
// 0   1
//  D C
//   1

// static normals are inited once, elsewhere

if (FlipX)
	{
	glTexCoord2f(1.0f, 0.0f);
	glNormal3dv(QPUTDFNormalA);
	glVertex3dv(A);
	glTexCoord2f(0.0f, 0.0f);
	glNormal3dv(QPUTDFNormalB);
	glVertex3dv(B);
	glTexCoord2f(0.0f, 1.0f);
	glNormal3dv(QPUTDFNormalC);
	glVertex3dv(C);
	glTexCoord2f(1.0f, 1.0f);
	glNormal3dv(QPUTDFNormalD);
	glVertex3dv(D);
	} // if
else
	{
	glTexCoord2f(0.0f, 0.0f);
	glNormal3dv(QPUTDFNormalA);
	glVertex3dv(A);
	glTexCoord2f(1.0f, 0.0f);
	glNormal3dv(QPUTDFNormalB);
	glVertex3dv(B);
	glTexCoord2f(1.0f, 1.0f);
	glNormal3dv(QPUTDFNormalC);
	glVertex3dv(C);
	glTexCoord2f(0.0f, 1.0f);
	glNormal3dv(QPUTDFNormalD);
	glVertex3dv(D);
	} // else

} // glQuadPolyUnitTexDefNorm

/*===========================================================================*/

/*** not used - FPW2 10/04/07
static void glQuadPoly(Point3d A, Point3d B, Point3d C, Point3d D)
{

// AB
// DC

static Point3d Normal, Normal2, Side[3];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
FindPosVector(Side[2], D, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(Normal, Side[0], Side[1]);
SurfaceNormal(Normal2, Side[2], Side[1]);

AddPoint3d(Normal2, Normal);
UnitVector(Normal2);

//SurfaceNormal(Normal, Vertices[0], Vertices[1], Vertices[2]);


glNormal3dv(Normal2);

glVertex3dv(A);
glVertex3dv(B);
glVertex3dv(C);
glVertex3dv(D);

} // glQuadPoly
***/

/*===========================================================================*/

static void glQuadPolyVertexDEMTex(VertexDEM &A, VertexDEM &B, VertexDEM &C, VertexDEM &D)
{
// AB
// DC
// use xyz for normal
// use ScrnXYZ[3] to store texture coords
glTexCoord3dv(A.ScrnXYZ);
glNormal3dv(A.xyz);
glVertex3dv(A.XYZ);

glTexCoord3dv(B.ScrnXYZ);
glNormal3dv(B.xyz);
glVertex3dv(B.XYZ);

glTexCoord3dv(C.ScrnXYZ);
glNormal3dv(C.xyz);
glVertex3dv(C.XYZ);

glTexCoord3dv(D.ScrnXYZ);
glNormal3dv(D.xyz);
glVertex3dv(D.XYZ);

} // glQuadPolyVertexDEMTex

/*===========================================================================*/

static void glQuadOutline(Point3d A, Point3d B, Point3d C, Point3d D)
{

// AB
// DC

//static Point3d Normal, Side[2];

//FindPosVector(Side[0], B, A);
//FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
//SurfaceNormal(Normal, Side[0], Side[1]);

//SurfaceNormal(Normal, Vertices[0], Vertices[1], Vertices[2]);


//glNormal3dv(Normal);

glVertex3dv(A);
glVertex3dv(B);
glVertex3dv(C);
glVertex3dv(D);

} // glQuadOutline

/*===========================================================================*/

static void gldrawW(double Size)
{

glScaled(Size, Size, Size);
glLineWidth(1.0f);
glBegin(GL_LINE_STRIP);
glVertex3d(-2.0,  2.0,  0.0);
glVertex3d(-1.0, -1.0,  0.0);
glVertex3d( 0.0,  0.0,  0.0);
glVertex3d( 1.0, -1.0,  0.0);
glVertex3d( 2.0,  2.0,  0.0);
glEnd();
glBegin(GL_LINE_STRIP);
glVertex3d( 0.0,  2.0, -2.0);
glVertex3d( 0.0, -1.0, -1.0);
glVertex3d( 0.0,  0.0,  0.0);
glVertex3d( 0.0, -1.0,  1.0);
glVertex3d( 0.0,  2.0,  2.0);
glEnd();

} // gldrawW

/*===========================================================================*/

void glStandUpright(double Lat, double Lon)
{

glRotated(Lon, 0.0, -1.0, 0.0);
glRotated(90 - Lat, 1.0, 0.0, 0.0);

} // glStandUpright

/*===========================================================================*/

// Calculate the cross product and return it
static void cross (float dst[3], float srcA[3], float srcB[3])
{
    dst[0] = srcA[1]*srcB[2] - srcA[2]*srcB[1];
    dst[1] = srcA[2]*srcB[0] - srcA[0]*srcB[2];
    dst[2] = srcA[0]*srcB[1] - srcA[1]*srcB[0];
}

/*===========================================================================*/

// Normalize the input vector
static void normalize (float vec[3])
{
    const float squaredLen = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
    const float invLen = 1.f / (float) sqrt (squaredLen);

    vec[0] *= invLen;
    vec[1] *= invLen;
    vec[2] *= invLen;
}

/*
    multLookAt -- Create a matrix to make an object, such as
	    a camera, "look at" another object or location, from
		a specified position.

	Parameters:
	    eye[x|y|x] Desired location of the camera object
		at[x|y|z]  Location for the camera to look at
		up[x|y|z]  Up vector of the camera

    Assumptions:
	    The camera geometry is defined to be facing
		the negative Z axis.

    Usage:
	    nultLookAt creates a matrix and transforms it onto the
		current matrix stack. Typical usage would be as follows:

		    glMatrixMode (GL_MODELVIEW);
			// Define the usual view transformation here using
			//   gluLookAt or whatever.
			glPushMatrix();
			multLookAt (orig[0], orig[1], orig[2],
			    at[0], at[1], at[2],
				up[0], up[1], up[2]);
			// Define "camera" object geometry here
			glPopMatrix();

    Warning: Results are undefined if (at-eye) is equal to 
	    or approximately equal to (up).

	Author:
		From: "Martz" <martz@frii.com>
		To: "Paul Martz" <martz@wslmail.fc.hp.com>
		Cc: "Paul Martz" <martz@fc.hp.com>
*/

/*===========================================================================*/

static void multLookAt (double deyex, double deyey, double deyez,
						double datx, double daty, double datz,
						double dupx, double dupy, double dupz)
{

	float eyex, eyey, eyez, atx, aty, atz, upx, upy, upz;
	float at[3], up[3];
	float xaxis[3];

	eyex = (float)deyex;
	eyey = (float)deyey;
	eyez = (float)deyez;
	atx  = (float)datx;
	aty  = (float)daty;
	atz  = (float)datz;
	upx  = (float)dupx;
	upy  = (float)dupy;
	upz  = (float)dupz;

	// Compute our new look at vector, which will be
	//   the new negative Z axis of our transformed object.
	at[0] = (float)(datx-deyex); at[1] = (float)(daty-deyey); at[2] = (float)(datz-deyez);
	normalize (at);

	// Make a useable copy of the current up vector.
	up[0] = upx; up[1] = upy; up[2] = upz;

	// Cross product of the new look at vector and the current
	//   up vector will produce a vector which is the new
	//   positive X axis of our transformed object.
	cross (xaxis, at, up);
	normalize (xaxis);

	// Calculate the new up vector, which will be the
	//   positive Y axis of our transformed object. Note
	//   that it will lie in the same plane as the new
	//   look at vector and the old up vector.
	cross (up, xaxis, at);

	// Create the matrix. The desired transformation
	//   is obtained with this 4x4 matrix:
	//       |  [xaxis] 0  |
	//       |    [up]  0  |
	//       |   [-at]  0  |
	//       |   [eye]  1  |
	{
		GLfloat m[16];
		m[0] = xaxis[0]; m[1] = xaxis[1];
		m[2] = xaxis[2]; m[3] = 0.f;
		m[4] = up[0]; m[5] = up[1];
		m[6] = up[2]; m[7] = 0.f;
		m[8] = -at[0]; m[9] = -at[1];
		m[10] = -at[2]; m[11] = 0.f;
		m[12] = eyex; m[13] = eyey;
		m[14] = eyez; m[15] = 1.f;
		glMultMatrixf (m);
	}
}

/*===========================================================================*/

static void multLookAtVec (float eyex, float eyey, float eyez,
						float atvecx, float atvecy, float atvecz,
						float upx, float upy, float upz)
{
	float at[3], up[3];
	float xaxis[3];

	// Compute our new look at vector, which will be
	//   the new negative Z axis of our transformed object.
	at[0] = atvecx; at[1] = atvecy; at[2] = atvecz;
	normalize (at);

	// Make a useable copy of the current up vector.
	up[0] = upx; up[1] = upy; up[2] = upz;

	// Cross product of the new look at vector and the current
	//   up vector will produce a vector which is the new
	//   positive X axis of our transformed object.
	cross (xaxis, at, up);
	normalize (xaxis);

	// Calculate the new up vector, which will be the
	//   positive Y axis of our transformed object. Note
	//   that it will lie in the same plane as the new
	//   look at vector and the old up vector.
	cross (up, xaxis, at);

	// Create the matrix. The desired transformation
	//   is obtained with this 4x4 matrix:
	//       |  [xaxis] 0  |
	//       |    [up]  0  |
	//       |   [-at]  0  |
	//       |   [eye]  1  |
	{
		GLfloat m[16];
		m[0] = xaxis[0]; m[1] = xaxis[1];
		m[2] = xaxis[2]; m[3] = 0.f;
		m[4] = up[0]; m[5] = up[1];
		m[6] = up[2]; m[7] = 0.f;
		m[8] = -at[0]; m[9] = -at[1];
		m[10] = -at[2]; m[11] = 0.f;
		m[12] = eyex; m[13] = eyey;
		m[14] = eyez; m[15] = 1.f;
		glMultMatrixf (m);
	}
}

/*===========================================================================*/

static void mgluLookAtVec( GLdouble eyex, GLdouble eyey, GLdouble eyez,
                         GLdouble centerx, GLdouble centery, GLdouble centerz,
                         GLdouble upx, GLdouble upy, GLdouble upz )
{
   GLdouble m[16];
   GLdouble x[3], y[3], z[3];
   GLdouble mag;

   /* Make rotation matrix */

   /* Z vector */
   //z[0] = eyex - centerx;
   //z[1] = eyey - centery;
   //z[2] = eyez - centerz;
   // We now supply lookat vector instead of lookat position
   z[0] = -centerx;
   z[1] = -centery;
   z[2] = -centerz;
   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
   if (mag) {  /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] =  y[1]*z[2] - y[2]*z[1];
   x[1] = -y[0]*z[2] + y[2]*z[0];
   x[2] =  y[0]*z[1] - y[1]*z[0];

   /* Recompute Y = Z cross X */
   y[0] =  z[1]*x[2] - z[2]*x[1];
   y[1] = -z[0]*x[2] + z[2]*x[0];
   y[2] =  z[0]*x[1] - z[1]*x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
#undef M
   glMultMatrixd( m );

   /* Translate Eye to Origin */
   glTranslated( -eyex, -eyey, -eyez );

} // mgluLookAtVec

/*===========================================================================*/

static void InvertZ(Point3d Flip)
{
#ifdef WCS_VIEWGUI_INVERT
Flip[2] = -Flip[2];
#endif // WCS_VIEWGUI_INVERT
} // InvertZ

/*** not used - FPW2 10/04/07
static void InvertZf(Point3f Flip)
{
#ifdef WCS_VIEWGUI_INVERT
Flip[2] = -Flip[2];
#endif // WCS_VIEWGUI_INVERT
} // InvertZf
***/

/*** not used - FPW2 10/04/07
static void InvertXYf(Point3f Flip)
{
#ifdef WCS_VIEWGUI_INVERT
Flip[0] = -Flip[0];
Flip[1] = -Flip[1];
#endif // WCS_VIEWGUI_INVERT
} // InvertXYf
***/

/*===========================================================================*/

static void InvertXYd(Point3d Flip)
{
#ifdef WCS_VIEWGUI_INVERT
Flip[0] = -Flip[0];
Flip[1] = -Flip[1];
#endif // WCS_VIEWGUI_INVERT
} // InvertXYd

/*** not used - FPW2 10/04/07
static void glMakeNormal(Point3d A, Point3d B, Point3d C)
{
static Point3d Normal, Side[2];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(Normal, Side[0], Side[1]);

glNormal3dv(Normal);
} // glMakeNormal
***/

/*===========================================================================*/

static void MakeNormal(Point3d NV, Point3d A, Point3d B, Point3d C)
{
static Point3d Side[2];

FindPosVector(Side[0], B, A);
FindPosVector(Side[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(NV, Side[0], Side[1]);
//InvertZ(NV);

} // MakeNormal

/*===========================================================================*/

static void fauxSolidSphere(GLdouble radius)
{
    GLUquadricObj *quadObj;
    //GLdouble *sizeArray;
    //GLuint displayList;

    //sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    //*sizeArray = radius;
    //displayList = findList (SPHERESOLID, sizeArray, 1);

//    if (displayList == 0) {
//	glNewList(makeModelPtr (SPHERESOLID, sizeArray, 1),
//	    GL_COMPILE_AND_EXECUTE);
	    quadObj = gluNewQuadric ();
	    gluQuadricDrawStyle (quadObj, GLU_FILL);
	    gluQuadricNormals (quadObj, GLU_SMOOTH);
	    gluSphere (quadObj, radius, 16, 16);
		gluDeleteQuadric(quadObj);
//	glEndList();
//    }
//    else {
//	glCallList(displayList);
//	free (sizeArray);
//    }
}

/*===========================================================================*/

static void fauxSolidCyl(GLdouble radiusbot, GLdouble radiustop, GLdouble len)
{
    GLUquadricObj *quadObj;
    //GLdouble *sizeArray;
    //GLuint displayList;

    //sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    //*sizeArray = radius;
    //displayList = findList (SPHERESOLID, sizeArray, 1);

//    if (displayList == 0) {
//	glNewList(makeModelPtr (SPHERESOLID, sizeArray, 1),
//	    GL_COMPILE_AND_EXECUTE);
	    quadObj = gluNewQuadric ();
	    gluQuadricDrawStyle (quadObj, GLU_FILL);
	    gluQuadricNormals (quadObj, GLU_SMOOTH);
	    //gluSphere (quadObj, radius, 16, 16);
		gluCylinder(quadObj, radiusbot, radiustop, len, 8, 1);
		gluDeleteQuadric(quadObj);
//	glEndList();
//    }
//    else {
//	glCallList(displayList);
//	free (sizeArray);
//    }
}

/*===========================================================================*/

static void fauxSolidDisk(GLdouble radius)
{
    GLUquadricObj *quadObj;
    //GLdouble *sizeArray;
    //GLuint displayList;

    //sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    //*sizeArray = radius;
    //displayList = findList (SPHERESOLID, sizeArray, 1);

//    if (displayList == 0) {
//	glNewList(makeModelPtr (SPHERESOLID, sizeArray, 1),
//	    GL_COMPILE_AND_EXECUTE);
	    quadObj = gluNewQuadric ();
	    gluQuadricDrawStyle (quadObj, GLU_FILL);
	    gluQuadricNormals (quadObj, GLU_SMOOTH);
	    //gluSphere (quadObj, radius, 16, 16);
		gluDisk(quadObj, 0.0, radius, 8, 1);
		gluDeleteQuadric(quadObj);
//	glEndList();
//    }
//    else {
//	glCallList(displayList);
//	free (sizeArray);
//    }
}

/*===========================================================================*/

static void gldrawbox(GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1,
	GLdouble z0, GLdouble z1, GLenum type)
{
    static GLdouble n[6][3] = {
	{-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
	{0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}
    };
    static GLint faces[6][4] = {
	{ 0, 1, 2, 3 }, { 3, 2, 6, 7 }, { 7, 6, 5, 4 },
	{ 4, 5, 1, 0 }, { 5, 6, 2, 1 }, { 7, 4, 0, 3 }
    };
    GLdouble v[8][3], tmp;
    GLint i;

    if (x0 > x1) {
		tmp = x0; x0 = x1; x1 = tmp;
		} // if
    if (y0 > y1) {
		tmp = y0; y0 = y1; y1 = tmp; 
		} // if
    if (z0 > z1) {
		tmp = z0; z0 = z1; z1 = tmp; 
		} // if
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = x0;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = x1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;

    for (i = 0; i < 6; i++) {
		glBegin(type);
		glNormal3dv(&n[i][0]);
		glVertex3dv(&v[faces[i][0]][0]);
		glNormal3dv(&n[i][0]);
		glVertex3dv(&v[faces[i][1]][0]);
		glNormal3dv(&n[i][0]);
		glVertex3dv(&v[faces[i][2]][0]);
		glNormal3dv(&n[i][0]);
		glVertex3dv(&v[faces[i][3]][0]);
		glEnd();
		} // if
} // gldrawbox

/*===========================================================================*/

ViewGUI::ViewGUI(Project *ProjSource, Database *DBStore, InterCommon *IC)
{
int InitLoop, InitSubLoop;
GradientCritter *GradNode;
unsigned char GradR, GradG, GradB;
static NotifyTag AllEvents[] = {MAKE_ID(0xff, 0xff, 0xff, 0xff), 0};
static NotifyTag DbEvents[] = {	MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB),
								MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN),
								MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH),
								MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE),
								MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE),
								0};
static NotifyTag AllIntercommonEvents[] = {	MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
											MAKE_ID(WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_GRANULARITY, WCS_INTERCAM_ITEM_GRIDSAMPLE, 0),
											MAKE_ID(WCS_INTERCLASS_VECTOR, 0xff, 0xff, 0xff),
								0};

LastActiveView = -1;
GlobeRadius = EARTHRAD;
CamSetup = NULL;
SetCursor(0.0, 0.0, 0.0);
GlobalFloor = StaticGlobalFloor = DynGlobalFloor = 0.0;
TexElRange = 1.0;
TexElMin = DBL_MAX;
TexElMax = -DBL_MAX;
ClearLastMouse();
ObjectManipulationMode = ViewManipulationMode = ViewManipulationEnable = 0;
HMotionScaleFactor = 1.0; // 1 meter
VMotionScaleFactor = 1.0; // 1 meter
ObjectUnit = 1.0;
//DeferredGrab = NULL;
//DeferredWin = NULL;
TempVec = NULL;
BGRedraw = 0;
BGRedrawCurView = -1;
QueuedRedraw = 0;
QueuedViewNum = -2;
JoeChanged = 0;
QuietDuringLoad = 0; // suppress handlenotify during loading

MotionControlEnable = MotionControlMode = 0;
DigitizeOneShot = 0;

EarthGradient = PrimaryGradient = GreyGradient = NULL;
EarthTexGrad = PrimaryTexGrad = GreyTexGrad = ContourTex = GridTex = NULL;

AxisEnable[WCS_X] = AxisEnable[WCS_Y] = AxisEnable[WCS_Z] = AxisEnable[3] = 1;

MeasureInProgress = 0;
MeasureString[0] = MeasureString[1] = MeasureString[2] = MeasureString[3] = MeasureString[4] = MeasureString[5] = -DBL_MAX;
ZoomCoord[0] = ZoomCoord[1] = -1;
ZoomInProgress = 0;
ZoomView = -1;
RegionCoord[0] = RegionCoord[1] = -1;
RegionInProgress = 0;
RegionView = -1;
ClearPointsPick = PickEnabled = 0;

SetInterLock(0);
LoadInProgress = 0;
OpenState = 0;
SyncDelayTicks = DelayedSync = 0;
SyncDelayTime = 0;

PrefsGUI = NULL;
RTPrefsGUI = NULL;

OriginThresh = 1000000;

ConstructError = 0;

LocalDB = DBStore;
LocalProject = ProjSource;

VectorListsAreReserved = 0;
RegenDEMs = RegenVecs = RegenDyn = 1;
RegenWalls = 0; // they will by themselves first time we draw
RegenSlopeTex = RegenFractTex = RegenEcoTex = RegenRelelTex = 0;
WallPerspList = WallPlanList = -1;
#ifdef WCS_BUILD_VNS
WallProjPlanList = -1;
#endif // WCS_BUILD_VNS
WallPerspOri[0] = WallPerspOri[1] = WallPerspOri[2] = 0.0;
DEMPolyPercent = ObjPolyPercent = 1.0;

InterStash = IC;

for (InitLoop = 0; InitLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; InitLoop++)
	{
	ViewWins[InitLoop] = NULL;
	ViewSpaces[InitLoop] = NULL;
	} // for

for (InitLoop = 0; InitLoop < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; InitLoop++)
	{
	PerspVectorLists[InitLoop] = 0;
	PlanVectorLists[InitLoop] = 0;
	#ifdef WCS_BUILD_VNS
	ProjPlanVectorLists[InitLoop] = 0;
	#endif // WCS_BUILD_VNS

	} // for

if (ContourTex = (unsigned char *)AppMem_Alloc(3 * WCS_VIEWGUI_TEXGRAD_SIZE, APPMEM_CLEAR))
	{
	unsigned char Grid;
	for (InitLoop = 0; InitLoop < WCS_VIEWGUI_TEXGRAD_SIZE; InitLoop++)
		{
		Grid = (InitLoop < 4);
		Grid = (Grid ? 0 : 255);
		ContourTex[InitLoop * 3 + 0] = 
		ContourTex[InitLoop * 3 + 1] = 
		ContourTex[InitLoop * 3 + 2] = Grid;
		} // for
	} // if

if (GridTex = (unsigned char *)AppMem_Alloc(WCS_VIEWGUI_TEXGRID_SIZE * WCS_VIEWGUI_TEXGRID_SIZE, APPMEM_CLEAR))
	{
	int RowIdx;
	for (InitLoop = 0; InitLoop < WCS_VIEWGUI_TEXGRID_SIZE; InitLoop++)
		{
		RowIdx = InitLoop * WCS_VIEWGUI_TEXGRID_SIZE;
		for (InitSubLoop = 0; InitSubLoop < WCS_VIEWGUI_TEXGRID_SIZE; InitSubLoop++)
			{
			if ((!InitLoop)||(!InitSubLoop))
				{
				GridTex[(RowIdx + InitSubLoop)] = 255;
				} // if
			} // if
		} // for
	} // if


if (GreyGradient = new AnimColorGradient(NULL, 0))
	{
	if (GreyTexGrad = (unsigned char *)AppMem_Alloc(3 * WCS_VIEWGUI_TEXGRAD_SIZE, APPMEM_CLEAR))
		{
		if (GradNode = GreyGradient->AddNode(0.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 0.0, 0.0);
				} // if
			} // if
		if (GradNode = GreyGradient->AddNode(1.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 1.0, 1.0);
				} // if
			} // if
		for (InitLoop = 0; InitLoop < WCS_VIEWGUI_TEXGRAD_SIZE; InitLoop++)
			{
			GreyGradient->GetBasicColor(GradR, GradG, GradB, (double)InitLoop / (double)(WCS_VIEWGUI_TEXGRAD_SIZE - 1));
			GreyTexGrad[InitLoop * 3 + 0] = GradR;
			GreyTexGrad[InitLoop * 3 + 1] = GradG;
			GreyTexGrad[InitLoop * 3 + 2] = GradB;
			} // for
		} // if
	} // if


if (EarthGradient = new AnimColorGradient(NULL, 0))
	{
	if (EarthTexGrad = (unsigned char *)AppMem_Alloc(3 * WCS_VIEWGUI_TEXGRAD_SIZE, APPMEM_CLEAR))
		{
		if (GradNode = EarthGradient->AddNode(0.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 5.0/15.0, 0.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(1.0/6.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 9.0/15.0, 0.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(2.0/6.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(9.0/15.0, 4.0/15.0, 0.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(3.0/6.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(13.0/15.0, 9.0/15.0, 5.0/15.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(4.0/6.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(7.0/15.0, 7.0/15.0, 7.0/15.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(5.0/6.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(11.0/15.0, 11.0/15.0, 11.0/15.0);
				} // if
			} // if
		if (GradNode = EarthGradient->AddNode(1.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 1.0, 1.0);
				} // if
			} // if
		for (InitLoop = 0; InitLoop < WCS_VIEWGUI_TEXGRAD_SIZE; InitLoop++)
			{
			EarthGradient->GetBasicColor(GradR, GradG, GradB, (double)InitLoop / (double)(WCS_VIEWGUI_TEXGRAD_SIZE - 1));
			EarthTexGrad[InitLoop * 3 + 0] = GradR;
			EarthTexGrad[InitLoop * 3 + 1] = GradG;
			EarthTexGrad[InitLoop * 3 + 2] = GradB;
			} // for
		} // if
	} // if

if (PrimaryGradient = new AnimColorGradient(NULL, 0))
	{
	if (PrimaryTexGrad = (unsigned char *)AppMem_Alloc(3 * WCS_VIEWGUI_TEXGRAD_SIZE, APPMEM_CLEAR))
		{
		if (GradNode = PrimaryGradient->AddNode(0.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 0.0, 0.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(1.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 0.0, 1.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(2.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 0.0, 1.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(3.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(0.0, 1.0, 0.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(4.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 1.0, 0.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(5.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 7.0/15.0, 0.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(6.0/7.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 0.0, 0.0);
				} // if
			} // if
		if (GradNode = PrimaryGradient->AddNode(1.0))
			{
			if (GradNode->GetThing())
				{
				((ColorTextureThing *)GradNode->GetThing())->Color.SetValue3(1.0, 1.0, 1.0);
				} // if
			} // if
		for (InitLoop = 0; InitLoop < WCS_VIEWGUI_TEXGRAD_SIZE; InitLoop++)
			{
			PrimaryGradient->GetBasicColor(GradR, GradG, GradB, (double)InitLoop / (double)(WCS_VIEWGUI_TEXGRAD_SIZE - 1));
			PrimaryTexGrad[InitLoop * 3 + 0] = GradR;
			PrimaryTexGrad[InitLoop * 3 + 1] = GradG;
			PrimaryTexGrad[InitLoop * 3 + 2] = GradB;
			} // for
		} // if
	} // if

GlobalApp->AppDB->RegisterClient(this, DbEvents);
GlobalApp->AppEx->RegisterClient(this, AllEvents);
GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
//GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);

if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("suppress_plan_ambient"))
	{
	SuppressPlanAmbient = 1;
	} // if

// init static normals for glQuadPolyUnitTexDefNorm()
QPUTDFNormalA[0] =  0.0;
QPUTDFNormalA[1] =  0.0;
QPUTDFNormalA[2] = -1.0;

QPUTDFNormalB[0] =  0.0;
QPUTDFNormalB[1] =  0.0;
QPUTDFNormalB[2] = -1.0;

QPUTDFNormalC[0] = -0.5;
QPUTDFNormalC[1] = 0.0;
QPUTDFNormalC[2] = -0.5;

QPUTDFNormalD[0] =  0.5;
QPUTDFNormalD[1] = 0.0;
QPUTDFNormalD[2] = -0.5;

UnitVector(QPUTDFNormalA);
UnitVector(QPUTDFNormalB);
UnitVector(QPUTDFNormalC);
UnitVector(QPUTDFNormalD);

// static "flat" normal for glQuadPolyUnitTex()
QPUTNormalFlat[0] =  0.0;
QPUTNormalFlat[1] =  0.0;
QPUTNormalFlat[2] = -1.0;

/*
QPUTDFNormalA[0] =  0.0;
QPUTDFNormalA[1] =  0.0;
QPUTDFNormalA[2] = -1.0;

QPUTDFNormalB[0] =  0.0;
QPUTDFNormalB[1] =  0.0;
QPUTDFNormalB[2] = -1.0;

QPUTDFNormalC[0] =  0.0;
QPUTDFNormalC[1] =  0.0;
QPUTDFNormalC[2] = -1.0;

QPUTDFNormalD[0] =  0.0;
QPUTDFNormalD[1] =  0.0;
QPUTDFNormalD[2] = -1.0;
*/ 

} // ViewGUI::ViewGUI

/*===========================================================================*/

ViewGUI::~ViewGUI()
{
int KillLoop;

if (PrefsGUI)
	{
	delete PrefsGUI;
	PrefsGUI = NULL;
	} // if

if (RTPrefsGUI)
	{
	delete RTPrefsGUI;
	RTPrefsGUI = NULL;
	} // if

GlobalApp->AppDB->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
//GlobalApp->MainProj->RemoveClient(this);

// remove ourselves from background processing
GlobalApp->RemoveBGHandlers(this);


if (EarthGradient) delete EarthGradient; EarthGradient = NULL;
if (PrimaryGradient) delete PrimaryGradient; PrimaryGradient = NULL;
if (GreyGradient) delete GreyGradient; GreyGradient = NULL;

if (EarthTexGrad) AppMem_Free(EarthTexGrad, 3 * WCS_VIEWGUI_TEXGRAD_SIZE); EarthTexGrad = NULL;
if (PrimaryTexGrad) AppMem_Free(PrimaryTexGrad, 3 * WCS_VIEWGUI_TEXGRAD_SIZE); PrimaryTexGrad = NULL;
if (GreyTexGrad) AppMem_Free(GreyTexGrad, 3 * WCS_VIEWGUI_TEXGRAD_SIZE); GreyTexGrad = NULL;
if (ContourTex) AppMem_Free(ContourTex, 3 * WCS_VIEWGUI_TEXGRAD_SIZE); ContourTex = NULL;
if (GridTex) AppMem_Free(GridTex, WCS_VIEWGUI_TEXGRID_SIZE * WCS_VIEWGUI_TEXGRID_SIZE); GridTex = NULL;

// DoClose will kill DEM and Vector displaylists as it closes the last GLDF

for (KillLoop = 0; KillLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; KillLoop++)
	{
	DoClose(KillLoop);
	} // for

if (CamSetup)
	{
	delete CamSetup;
	CamSetup = NULL;
	} // if

} // ViewGUI::~ViewGUI

/*===========================================================================*/

NativeGUIWin ViewGUI::Open(Project *Moi)
{

NativeGUIWin Ret = NULL;
LocalProject = Moi;
ConfigureWidgets();
OpenState = 1;
return(Ret);

} // ViewGUI::Open

/*===========================================================================*/

NativeGUIWin ViewGUI::Construct(void)
{

return(NULL);

} // ViewGUI::Construct

/*===========================================================================*/

void ViewGUI::ConfigureWidgets(void)
{
RasterAnimHost *ActObj = NULL;
RasterAnimHostProperties RAHP;
char ManipRot, IsVectorOrControlPt = 0, IsDEM = 0;
char Xen, Yen, Zen, Move, Rot, Scale, Points;

if (ActObj = RasterAnimHost::GetActiveRAHost())
	{
	RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	ActObj->GetRAHostProperties(&RAHP);
	IsVectorOrControlPt = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);
	IsDEM = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_DEM);
	} // if

if (!IsVectorOrControlPt)
	{
	// Points mode is invalid
	InterStash->SetParam(1, MAKE_ID(WCS_INTERCLASS_MAPVIEW, WCS_INTERMAP_SUBCLASS_MISC, WCS_INTERMAP_ITEM_POINTSMODE, 0),
	 0, 0);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_POINTSMODE, false);
	} // if

if (GetViewManipulationEnable())
	{
	ManipRot = (GetViewManipulationMode() == WCS_VIEWGUI_MANIP_ROT);
	Move = Rot = Scale = 1;
	Xen = Yen = Zen = 1;
	Points = 0;
	} // if
else
	{
	Points = Move = Rot = Scale = Xen = Yen = Zen = 0; // in case no active object
	ManipRot = (GetObjectManipulationMode() == WCS_VIEWGUI_MANIP_ROT);
	if (ActObj)
		{
		FetchObjectActionsAxes(ActObj, Move, Rot, Scale, Xen, Yen, Zen, Points);
		} // if
	} // else

EnableAxisButtons(Xen, Yen, Zen, (Xen || Yen || Zen));
EnableModeButtons(Move, Rot, Scale);
EnablePoints(Points);

GlobalApp->MCP->ConfigureToolbarAxis(ManipRot ? true : false);

UpdateAxes();
UpdateManipulationMode();

EnableJoeControls(IsVectorOrControlPt, IsDEM);

EnableViewButtons(IdentActiveView() != -1);

} // ViewGUI::ConfigureWidgets

/*===========================================================================*/

void ViewGUI::EnableViewButtons(int IsView) // IsView is not used
{
// Buttons are moving to each View's titlebar
for (int InitLoop = 0; InitLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; InitLoop++)
	{
	if (ViewWins[InitLoop])
		{
		ViewWins[InitLoop]->UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_PAN | WCS_FENETRE_WINSTATE_ROTATE | WCS_FENETRE_WINSTATE_ZOOM | WCS_FENETRE_WINSTATE_ZOOMBOX | WCS_FENETRE_WINSTATE_RENDER | WCS_FENETRE_WINSTATE_CONSTRAIN);
		} // if
	} // for
} // ViewGUI::EnableViewButtons

/*===========================================================================*/

void ViewGUI::EnableJoeControls(char IsVectorOrControlPt, char IsDEM)
{

GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_APPEND, !IsVectorOrControlPt);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_REPLACE, !IsVectorOrControlPt);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_UNDO, !IsVectorOrControlPt);

// create button ghosting is a little complex but probably worth it
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_CREATE, ! RasterAnimHost::IsDigitizeLegal(NULL, 0));

} // ViewGUI::EnableJoeControls

/*===========================================================================*/

void ViewGUI::EnableModeButtons(char M, char R, char S)
{

GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_MOVE, !M);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_ROTATE, !R);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_SCALEZOOM, !S);

} // ViewGUI::EnableModeButtons

/*===========================================================================*/

void ViewGUI::EnableAxisButtons(char X, char Y, char Z, char E)
{

GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_XAXIS, !X);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_YAXIS, !Y);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_ZAXIS, !Z);
GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_ELEVAXIS, !E);

} // ViewGUI::EnableAxisButtons

/*===========================================================================*/

void ViewGUI::EnablePoints(char Points)
{

GlobalApp->MCP->SetToolbarButtonDisabled(IDI_TB_POINTSMODE, !Points);
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_POINTSMODE, InterStash->MVPointsMode() ? true : false);

} // ViewGUI::EnablePoints

/*===========================================================================*/

void ViewGUI::FetchObjectActionsAxes(RasterAnimHost *Active, char &Move, char &Rot, char &Scale, char &Xen, char &Yen, char &Zen, char &Points)
{
RasterAnimHostProperties RAHP;

Move = Rot = Scale = Xen = Yen = Zen = 0;

if (Active)
	{
	RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	Active->GetRAHostProperties(&RAHP);

	Move   = RAHP.InterFlags & (WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ) ? 1 : 0;
	Rot    = RAHP.InterFlags & (WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_ROTATEY | WCS_RAHOST_INTERBIT_ROTATEZ) ? 1 : 0;
	Scale  = RAHP.InterFlags & (WCS_RAHOST_INTERBIT_SCALEX | WCS_RAHOST_INTERBIT_SCALEY | WCS_RAHOST_INTERBIT_SCALEZ) ? 1 : 0;

	Points = RAHP.InterFlags & (WCS_RAHOST_INTERBIT_POINTS) ? 1 : 0;

	switch(GetObjectManipulationMode())
		{
		case WCS_VIEWGUI_MANIP_MOVE:
			{
			Xen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_MOVEX ? 1 : 0;
			Yen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_MOVEY ? 1 : 0;
			Zen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_MOVEZ ? 1 : 0;
			break;
			} // 
		case WCS_VIEWGUI_MANIP_ROT:
			{
			Xen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_ROTATEX ? 1 : 0;
			Yen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_ROTATEY ? 1 : 0;
			Zen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_ROTATEZ ? 1 : 0;
			break;
			} // 
		case WCS_VIEWGUI_MANIP_SCALE:
			{
			Xen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_SCALEX ? 1 : 0;
			Yen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_SCALEY ? 1 : 0;
			Zen = RAHP.InterFlags & WCS_RAHOST_INTERBIT_SCALEZ ? 1 : 0;
			break;
			} // 
		} // switch
	} // if

} // ViewGUI::FetchObjectActions

/*===========================================================================*/

void ViewGUI::ReserveVectorDisplayLists(void)
{
int ListStart, ListInit;

// Perspective
if (ListStart = glGenLists(WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX))
	{
	for (ListInit = 0; ListInit < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; ListInit++)
		{
		PerspVectorLists[ListInit] = ListInit + ListStart;
		} // for
	} // if

// Plan
if (ListStart = glGenLists(WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX))
	{
	for (ListInit = 0; ListInit < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; ListInit++)
		{
		PlanVectorLists[ListInit] = ListInit + ListStart;
		} // for
	} // if

#ifdef WCS_BUILD_VNS
// Projected Plan
if (ListStart = glGenLists(WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX))
	{
	for (ListInit = 0; ListInit < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; ListInit++)
		{
		ProjPlanVectorLists[ListInit] = ListInit + ListStart;
		} // for
	} // if
#endif // WCS_BUILD_VNS


VectorListsAreReserved = 1;

} // ViewGUI::ReserveVectorDisplayLists

/*===========================================================================*/

void ViewGUI::KillVectorDisplayLists(void)
{
int KillLoop;

// Kill global displaylists
for (KillLoop = 0; KillLoop < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; KillLoop++)
	{
	if (PerspVectorLists[KillLoop])
		{
		glDeleteLists(PerspVectorLists[KillLoop], 1);
		PerspVectorLists[KillLoop] = 0;
		} // if

	// <<<>>> Should we be killing Plan (and Projected Plan) lists here?
	/*
	if (PlanVectorLists[KillLoop])
		{
		glDeleteLists(PlanVectorLists[KillLoop], 1);
		PlanVectorLists[KillLoop] = 0;
		} // if
	#ifdef WCS_BUILD_VNS
	if (ProjPlanVectorLists[KillLoop])
		{
		glDeleteLists(ProjPlanVectorLists[KillLoop], 1);
		ProjPlanVectorLists[KillLoop] = 0;
		} // if
	#endif // WCS_BUILD_VNS
	*/

	} // for


VectorListsAreReserved = 0;
RegenVecs = 1;

} // ViewGUI::KillVectorDisplayLists

/*===========================================================================*/

void ViewGUI::KillDEMDisplayLists(void)
{
Joe *Clip;
JoeDEM *MyDEM;
JoeViewTemp *JVT;

// Kill object displaylists
for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP);
				if (JVT)
					{
					if (JVT->PerspDisplayListNum)
						{
						glDeleteLists(JVT->PerspDisplayListNum, 1);
						JVT->PerspDisplayListNum = 0;
						} // if
					if (JVT->PlanDisplayListNum)
						{
						glDeleteLists(JVT->PlanDisplayListNum, 1);
						JVT->PlanDisplayListNum = 0;
						} // if
					#ifdef WCS_BUILD_VNS
					if (JVT->ProjPlanDisplayListNum)
						{
						glDeleteLists(JVT->ProjPlanDisplayListNum, 1);
						JVT->ProjPlanDisplayListNum = 0;
						} // if
					#endif // WCS_BUILD_VNS
					} // if
				} // 
			} // if
		} // if
	} // for

RegenDEMs = 1;

} // ViewGUI::KillDEMDisplayLists

/*===========================================================================*/

long ViewGUI::GetNumOpenViews(void)
{
int ViewCheck, NumOpenViews = 0;

// any views?
for (ViewCheck = 0; ViewCheck < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewCheck++)
	{
	if (ViewSpaces[ViewCheck] && ViewWins[ViewCheck])
		{
		NumOpenViews++;
		} // if
	} // for

return(NumOpenViews);

} // ViewGUI::GetNumOpenViews

/*===========================================================================*/

long ViewGUI::GetNumOpenLiveViews(void)
{
int ViewCheck, NumOpenViews = 0;

// any views?
for (ViewCheck = 0; ViewCheck < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewCheck++)
	{
	if (ViewSpaces[ViewCheck] && ViewSpaces[ViewCheck]->GetAlive())
		{
		NumOpenViews++;
		} // if
	} // for

return(NumOpenViews);

} // ViewGUI::GetNumOpenLiveViews

/*===========================================================================*/

Renderer *ViewGUI::FindNextLiveRenderer(long &CurViewNum)
{
Renderer *Rend;
int NumOpenViews = 0;

CurViewNum = CurViewNum < 0 ? 0: CurViewNum + 1;

// any views?
for ( ; CurViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; CurViewNum++)
	{
	if (ViewSpaces[CurViewNum] && ViewSpaces[CurViewNum]->GetAlive())
		{
		if (Rend = ViewSpaces[CurViewNum]->FetchRenderer())
			return (Rend);
		} // if
	} // for

return(NULL);

} // ViewGUI::FindNextLiveRenderer

/*===========================================================================*/

int ViewGUI::DrawForceRegen(int Vecs, int DEMs, int WhichView)
{

// Force regens
if (Vecs)
	{
	RegenVecs = 1;
	RegenWalls = 1;
	} // if
if (DEMs) RegenDEMs = 1;
return(Draw(WhichView));

} // ViewGUI::DrawForceRegen

/*===========================================================================*/

int ViewGUI::Draw(int WhichView)
{ // queue drawing until next spare moment

// bail out of BGredraw if in progress
EndBGRedraw();

// anything to redraw?
if (GetNumOpenViews() == 0) return(0);

SetQueuedRedraw();
if (QueuedViewNum == -2)
	{
	QueuedViewNum = WhichView;
	} // if
else
	{ // several are now flagged for redraw, do them all.
	QueuedViewNum = -1;
	} // else

if (!GlobalApp->IsBGHandler(this))
	{
	GlobalApp->AddBGHandler(this);
	} // if

return(0);

} // ViewGUI::Draw

/*===========================================================================*/

int ViewGUI::DrawBG(void)
{

#ifdef WCS_VIEWGUI_BGREDRAW

if (BGRedrawCurView == -1)
	{
	PrepForRedraw();
	BGRedrawCurView = 0;
	return(0);
	} // if
else
	{
	if (ViewSpaces[BGRedrawCurView] && ViewWins[BGRedrawCurView])
		{
		DrawView(BGRedrawCurView, ViewSpaces[BGRedrawCurView]);
		DrawSync(BGRedrawCurView, ViewSpaces[BGRedrawCurView]);
		} // if
	if (BGRedrawCurView < WCS_VIEWGUI_VIEWS_OPEN_MAX)
		{
		BGRedrawCurView ++;
		return(0);
		} // if
	} // else
#endif // WCS_VIEWGUI_BGREDRAW

EndBGRedraw();
return(1);

} // ViewGUI::DrawBG

/*===========================================================================*/

void ViewGUI::PrepForRedraw(void)
{
double N, S, E, W;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

GlobeRadius = GlobalApp->AppEffects->GetPlanetRadius();

if (LocalDB)
	{
	LocalDB->GetBounds(N, S, E, W);
	DBCenter.Lat  = (N + S) * 0.5;
	DBCenter.Lon  = (E + W) * 0.5;
	DBCenter.Elev = 0.0;
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&DBCenter);
	#else // WCS_BUILD_VNS
	DBCenter.DegToCart(GlobeRadius);
	#endif // WCS_BUILD_VNS
	} // if

} // ViewGUI::PrepForRedraw

/*===========================================================================*/

int ViewGUI::DrawImmediately(int WhichView)
{
int ViewNum;

if (LoadInProgress || GetInterLock()) return(0);

// bail out of BGredraw if in progress
EndBGRedraw();

PrepForRedraw();

if (WhichView < 0)
	{
	for (ViewNum = 0; ViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewNum++)
		{
		if (ViewSpaces[ViewNum] && ViewWins[ViewNum])
			{
			DrawView(ViewNum, ViewSpaces[ViewNum]);
			} // if
		} // for
	} // if
else
	{
	DrawView(WhichView, ViewSpaces[WhichView]);
	} // else

if (GetQueuedRedraw())
	{
	ClearQueuedRedraw();
	QueuedViewNum = -2;
	} // if

FinishRedraw();

return(0);

} // ViewGUI::DrawImmediately

/*===========================================================================*/

void ViewGUI::FinishRedraw(void)
{

// Don't perform sync yet.
SyncDelayTicks = 0;
SyncDelayTime = GetTickCount();
DelayedSync = 1;
if (!GlobalApp->IsBGHandler(this))
	{
	GlobalApp->AddBGHandler(this);
	} // if

} // ViewGUI::FinishRedraw

/*===========================================================================*/

void ViewGUI::DrawView(int ViewNum, ViewContext *VC)
{
VertexDEM NewOrigin;
//double LMLat, LMLon;
int SelectHits;
RasterAnimHost *NewActive;
double PolyWeight;
Point3d OriginShift;
PlanetOpt *DefPO;
GeneralEffect *CurEffect;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
unsigned short VWidth, VHeight;

glEnable(GL_NORMALIZE);
glSelectBuffer(WCS_VIEWGUI_SELBUF_SIZE, SelBuf);
glInitNames();

// prevent crash with view width = 0
ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);
if (VWidth == 0 || VHeight == 0)
	{
	return;
	} // if

/*
if (!(DefPO = (PlanetOpt *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT)))
	{
	return; // no planet
	} // if
*/

DefPO = NULL;
if (CurEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled)
			{
			DefPO = (PlanetOpt *)CurEffect;
			}// if
		CurEffect = CurEffect->Next;
		} // while
	} // if

if (!DefPO)
	{
	return; // no enabled planet
	} // if

if (VC)
	{
	VC->SetPlanet(DefPO);
	} // if

if (!ViewSpaces[ViewNum]->VCamera || !ViewSpaces[ViewNum]->RO)
	{
	if (!ViewSpaces[ViewNum]->GetAlive())
		{
		ViewWins[ViewNum]->SetupForDrawing();
		ViewWins[ViewNum]->Clear();
		ViewWins[ViewNum]->CleanupFromDrawing();
		} // if
	} // if

if (ViewSpaces[ViewNum]->GetAlive() && !ViewSpaces[ViewNum]->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
	{
	if (LocalDB && ViewSpaces[ViewNum] && ViewSpaces[ViewNum]->VCamera)
		{
		ViewSpaces[ViewNum]->FetchCurCamCoord();
		NewOrigin.XYZ[0] = ViewSpaces[ViewNum]->CamPos.XYZ[0];
		NewOrigin.XYZ[1] = ViewSpaces[ViewNum]->CamPos.XYZ[1];
		NewOrigin.XYZ[2] = ViewSpaces[ViewNum]->CamPos.XYZ[2];
		//NewOrigin.XYZ[0] = (DBCenter.XYZ[0] + ViewSpaces[ViewNum]->CamPos.XYZ[0]) / 2.0;
		//NewOrigin.XYZ[1] = (DBCenter.XYZ[1] + ViewSpaces[ViewNum]->CamPos.XYZ[1]) / 2.0;
		//NewOrigin.XYZ[2] = (DBCenter.XYZ[2] + ViewSpaces[ViewNum]->CamPos.XYZ[2]) / 2.0;
		//NewOrigin.XYZ[0] = DBCenter.XYZ[0];
		//NewOrigin.XYZ[1] = DBCenter.XYZ[1];
		//NewOrigin.XYZ[2] = DBCenter.XYZ[2];
		//if (PointDistance(NewOrigin.XYZ, *(ViewSpaces[ViewNum]->GetOriginCart())) > OriginThresh)
			{
			#ifdef WCS_BUILD_VNS
			DefCoords->CartToDeg(&NewOrigin);
			#else // WCS_BUILD_VNS
			NewOrigin.CartToDeg(ViewSpaces[ViewNum]->GlobeRadius);
			#endif // WCS_BUILD_VNS
			ViewSpaces[ViewNum]->SetOrigin(NewOrigin.Lat, NewOrigin.Lon, NewOrigin.Elev);
			} // if
		} // if
	else
		{
		return;
		} // else

	// enforce projected view singluarity
	#ifdef WCS_BUILD_VNS
	int DoneOneProj = 0;
	for (int ViewProjLoop = 0; ViewProjLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewProjLoop++)
		{
		if (ViewSpaces[ViewProjLoop] && ViewWins[ViewProjLoop])
			{
			if (ViewSpaces[ViewProjLoop]->GetProjected() && ViewSpaces[ViewProjLoop]->GetAlive())
				{
				if (DoneOneProj)
					{
					// only one projected view alive at a time, disable others
					ViewSpaces[ViewProjLoop]->SetAlive(0);
					ConfigureTitle(ViewProjLoop);
					} // if
				else
					{
					DoneOneProj = 1;
					} // else
				} // if
			} // if
		} // for

	#endif // WCS_BUILD_VNS


	SetupLightsAndCameras(ViewNum, VC);

	ViewSpaces[ViewNum]->CalcWorldBounds(LocalDB);

	DrawSetup(ViewNum, ViewSpaces[ViewNum]);

	if (PickEnabled)
		{
		if (DebugSelect)
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "glRenderMode(GL_SELECT);");
			} // if
		glRenderMode(GL_SELECT);
		glPushName(0);
		} // if

	if (RegenDyn)
		{
		ReboundDynamic(-1, VC);
		RegenDyn = 0;
		} // if

	if (RegenDEMs)
		{
		double PolyPercent, ObjPolyWeight, DEMPolyWeight;
		double PolyLimit;// = 30000.0;

		DEMPolyPercent = ObjPolyPercent = 1.0;

		// 3dobject decimation disabled by popular demand
		//ObjPolyWeight = Weigh3DObjects();
		ObjPolyWeight = 0;
		DEMPolyWeight = WeighDEMGeometry();
		PolyWeight = ObjPolyWeight + DEMPolyWeight;

		PolyLimit = (double)GlobalApp->MainProj->Interactive->GetGridSample();

		if ((PolyWeight > 0.0) && (PolyWeight > PolyLimit))
			{
			// WCS_VIEWGUI_3DOBJ_POLY_MAXPCT
			ObjPolyPercent  = ObjPolyWeight  / PolyWeight;
			// limit polygon allotment dedicated to 3d objects
			// 3dobject decimation disabled by popular demand
			//ObjPolyPercent  = min(ObjPolyPercent, WCS_VIEWGUI_3DOBJ_POLY_MAXPCT);
			//ObjPolyPercent  = max(ObjPolyPercent, WCS_VIEWGUI_3DOBJ_POLY_MINPCT);
			//DEMPolyPercent  = 1.0 - ObjPolyPercent; // the rest goes to terrain

			// calculate our max limit as a percent of our total polygon load
			PolyPercent = PolyLimit / PolyWeight;

			// restrict them both so their total adds up to our max limit
			DEMPolyPercent *= PolyPercent;
			//ObjPolyPercent *= PolyPercent;
			ObjPolyPercent = 1.0;

			} // if
		// sanity check...
		if (DEMPolyPercent <= 0.0) DEMPolyPercent = 0.1;
		if (DEMPolyPercent > 1.0)  DEMPolyPercent = 1.0;

		if (ObjPolyPercent <= 0.0) ObjPolyPercent = 0.1;
		if (ObjPolyPercent > 1.0)  ObjPolyPercent = 1.0;

		} // if

	if (RegenDEMs)
		{
		RegenDEMGeometry(-1, DEMPolyPercent, 1, ViewSpaces[ViewNum]);
		RegenDEMs = 0;
		} // if

	if (RegenVecs)
		{
		RegenVectorGeometry(-1, -1, 1, ViewSpaces[ViewNum]);
		RegenVectorGeometry(1, -1, 1, ViewSpaces[ViewNum]);
		#ifdef WCS_BUILD_VNS
		int DoneOneProj = 0;
		for (int ViewProjLoop = 0; ViewProjLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewProjLoop++)
			{
			if (ViewSpaces[ViewProjLoop] && ViewWins[ViewProjLoop])
				{
				if (ViewSpaces[ViewProjLoop]->GetProjected() && ViewSpaces[ViewProjLoop]->GetAlive())
					{
					if (DoneOneProj)
						{
						// only one projected view alive at a time, disable others
						ViewSpaces[ViewProjLoop]->SetAlive(0);
						ConfigureTitle(ViewProjLoop);
						} // if
					else
						{
						DoneOneProj = 1;
						RegenVectorGeometry(1, -1, 1, ViewSpaces[ViewNum], ViewSpaces[ViewProjLoop]->GetProjected()); // regen Projected vectors for this View
						} // else
					} // if
				} // if
			} // for

		#endif // WCS_BUILD_VNS
		RegenVecs = 0;
		} // if

	if (RegenWalls)
		{
#ifdef WCS_BUILD_VNS
		if ((WallProjPlanList != -1) && glIsList(WallProjPlanList))
			{
			glDeleteLists(WallProjPlanList, 1);
			WallProjPlanList = -1;
			} // if
#endif // WCS_BUILD_VNS
		if ((WallPlanList != -1) && glIsList(WallPlanList))
			{
			glDeleteLists(WallPlanList, 1);
			WallPlanList = -1;
			} // if
		if ((WallPerspList != -1) && glIsList(WallPerspList))
			{
			glDeleteLists(WallPerspList, 1);
			WallPerspList = -1;
			} // if
		} // if

	if (RegenSlopeTex)
		{
		GenTexMaps(DefPO, WCS_VIEWGUI_ENABLE_OVER_SLOPE);
		RegenSlopeTex = 0;
		} // if
	if (RegenFractTex)
		{
		GenTexMaps(DefPO, WCS_VIEWGUI_ENABLE_OVER_FRACTAL);
		RegenFractTex = 0;
		} // if
	if (RegenEcoTex)
		{
		GenTexMaps(DefPO, WCS_VIEWGUI_ENABLE_OVER_ECOSYS);
		RegenEcoTex = 0;
		} // if
	if (RegenRelelTex)
		{
		GenTexMaps(DefPO, WCS_VIEWGUI_ENABLE_OVER_RELEL);
		RegenRelelTex = 0;
		} // if


	DrawClear(ViewNum, ViewSpaces[ViewNum]);
	DrawCamera(ViewNum, ViewSpaces[ViewNum]);
	DrawGLProjection(ViewNum, ViewSpaces[ViewNum], 0, 1);
	if (!PickEnabled)
		{
		DrawLights(ViewNum, ViewSpaces[ViewNum]);
		} // if
	if (!VC->IsPlan())
		{
		DrawGLProjection(ViewNum, ViewSpaces[ViewNum], 1, 1);
		} // if
	if (PickEnabled != 2) // 2 = pick points
		{
		if (!VC->IsPlan())
			{
			DrawCelestial(ViewNum, ViewSpaces[ViewNum]);
			DrawGLProjection(ViewNum, ViewSpaces[ViewNum], 0, 1);
			} // if
		} // if (PickEnabled != 2) // 2 = pick points
	if (!PickEnabled)
		{
		ViewSpaces[ViewNum]->FetchGLProjection();
		} // if

	if (!PickEnabled)
		{
		DrawFog(ViewSpaces[ViewNum]);
		} // if

	if (PickEnabled != 2) // 2 = pick points
		{
		if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS))
			{
			DrawDEMObjects(ViewNum, ViewSpaces[ViewNum], WCS_DATABASE_STATIC);
			DrawDEMObjects(ViewNum, ViewSpaces[ViewNum], WCS_DATABASE_DYNAMIC);
			} // if
		} // if (PickEnabled != 2) // 2 = pick points
	if (PickEnabled)
		{
		// Draw active vector object for points-testing
		if (VC->IsPlan())
			{
			glDisable(GL_DEPTH_TEST);
			} // if
		else
			{
			#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
			glDepthRange(0.0, WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT);
			#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
			#ifndef WCS_VIEWGUI_VECTOR_DEPTHTEST
			glDisable(GL_DEPTH_TEST);
			#endif // WCS_VIEWGUI_VECTOR_DEPTHTEST
			} // else
		if (PickEnabled == 2) // 2 = pick points
			{
			RasterAnimHost *RAH;
			RasterAnimHostProperties RAHP;
			if (RAH = RasterAnimHost::GetActiveRAHost())
				{
				// Could be bad if RAH is not a Joe.
				RAHP.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
				RAH->GetRAHostProperties(&RAHP);
				if ((RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT))
					{
					PushAndTranslateOrigin(VC);
					GenOneVector(VC->IsPlan(), 0, VC, (Joe *)(RAH), &DBCenter, 1, VC->GetProjected());
					PopOrigin();
					} // if
				} // if
			} // if (PickEnabled == 2) // 2 = pick points
		else
			{
			if (VC->IsPlan())
				{
				glPushMatrix(); // on MODELVIEW
				OriginShift[0] = DBCenter.Lon;
				OriginShift[1] = DBCenter.Lat;
				OriginShift[2] = 0.0;
				glTranslated(OriginShift[0], OriginShift[1], OriginShift[2]);
				RegenVectorGeometry(ViewNum, -1, NULL, ViewSpaces[ViewNum], VC->GetProjected());
				glPopMatrix(); // on MODELVIEW
				} // if
			else
				{
				glPushMatrix(); // on MODELVIEW
				// these three lines were added to replace the one commented out below in order to select vectors
				// now that the vectors are offset from the DBCenter when drawn
				CopyPoint3d(OriginShift, DBCenter.XYZ); // get object in WCS world coords
				InvertZ(OriginShift); // Convert object local origin from WCS world coords to GL world coords
				glTranslated(VC->GLNegOrigin[0] + OriginShift[0], VC->GLNegOrigin[1] + OriginShift[1], VC->GLNegOrigin[2] + OriginShift[2]);
				//glTranslated(VC->GLNegOrigin[0], VC->GLNegOrigin[1], VC->GLNegOrigin[2]);
				RegenVectorGeometry(-1, -1, NULL, ViewSpaces[ViewNum], VC->GetProjected());
				glPopMatrix(); // on MODELVIEW
				} // if
			} // else
		glEnable(GL_DEPTH_TEST);
		#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
		glDepthRange(0.0, 1.0);
		#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
		} // if PickEnabled
	else
		{
		if (BGRedraw)
			{
			glDisable(GL_LIGHTING);
			if (VC->IsPlan())
				{
				glDisable(GL_DEPTH_TEST);
				} // if
			else
				{
				#ifndef WCS_VIEWGUI_VECTOR_DEPTHTEST
				glDisable(GL_DEPTH_TEST);
				#endif // WCS_VIEWGUI_VECTOR_DEPTHTEST
				#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
				glDepthRange(0.0, WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT);
				#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
				} // else
			glPushMatrix(); // on MODELVIEW
			if (VC->IsPlan())
				{
				// should these be added here too?
				OriginShift[0] = DBCenter.Lon;
				OriginShift[1] = DBCenter.Lat;
				OriginShift[2] = 0.0;
				glTranslated(OriginShift[0], OriginShift[1], OriginShift[2]);
				RegenVectorGeometry(1, 0, NULL, ViewSpaces[ViewNum], VC->GetProjected());
				} // if
			else
				{
				// should these be added here too?
				CopyPoint3d(OriginShift, DBCenter.XYZ); // get object in WCS world coords
				InvertZ(OriginShift); // Convert object local origin from WCS world coords to GL world coords
				glTranslated(VC->GLNegOrigin[0] + OriginShift[0], VC->GLNegOrigin[1] + OriginShift[1], VC->GLNegOrigin[2] + OriginShift[2]);
				//glTranslated(VC->GLNegOrigin[0], VC->GLNegOrigin[1], VC->GLNegOrigin[2]);
				RegenVectorGeometry(-1, 0, NULL, ViewSpaces[ViewNum], VC->GetProjected());
				} // else
			glPopMatrix(); // on MODELVIEW
			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
			glDepthRange(0.0, 1.0);
			#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
			} // if BGRedraw
		else
			{ // !BGRedraw
			if (VC->IsPlan())
				{
				if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS))
					{
					DrawVectorObjects(ViewNum, ViewSpaces[ViewNum]);
					} // if
				} // if
			else
				{
				DrawVectorObjects(ViewNum, ViewSpaces[ViewNum]);
				} // else !IsPlan
			} // else !BGRedraw
		} // else !PickEnabled
	if (!PickEnabled)
		{
		DrawDigitizeObject(ViewNum, ViewSpaces[ViewNum]);
		DrawCursor(ViewNum, ViewSpaces[ViewNum]);
		} // if !PickEnabled
	if (PickEnabled != 2) // 2 = pick points
		{
		#ifdef WCS_VIEWGUI_BGREDRAW
		Draw3DObjects(ViewNum, ViewSpaces[ViewNum], (BGRedraw ? 0.0 : ObjPolyPercent));
		DrawWalls(ViewNum, ViewSpaces[ViewNum]);
		DrawFoliage(ViewNum, ViewSpaces[ViewNum], (BGRedraw ? 0.0 : ObjPolyPercent), 0); // 0 = only foliage effects
		//DrawFoliage(ViewNum, ViewSpaces[ViewNum], (BGRedraw ? 0.0 : ObjPolyPercent), 1); // 1 = realtime foliage file
		#else // !WCS_VIEWGUI_BGREDRAW
		Draw3DObjects(ViewNum, ViewSpaces[ViewNum], ObjPolyPercent);
		DrawWalls(ViewNum, ViewSpaces[ViewNum]);
		//DrawFoliage(ViewNum, ViewSpaces[ViewNum], ObjPolyPercent, 0); // 0 = only foliage effects
		//DrawFoliage(ViewNum, ViewSpaces[ViewNum], ObjPolyPercent, 1); // 1 = realtime foliage file
		DrawFoliage(ViewNum, ViewSpaces[ViewNum], ObjPolyPercent, RTFLoaded && ViewSpaces[ViewNum]->GetEnabled(WCS_VIEWGUI_ENABLE_RTFOLFILE)); // 1 = realtime foliage file

		// <<<>>> Try turning off Z writes for the next few types of objects
		glDepthMask(0);

		#endif // !WCS_VIEWGUI_BGREDRAW
		if (!VC->GetProjected())
			{
			DrawTargetObjects(ViewNum, ViewSpaces[ViewNum]);
			DrawWaveObjects(ViewNum, ViewSpaces[ViewNum]);
			} // 
		if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS))
			{
			// <<<>>> Need depth write back on here
			glDepthMask(1);
			DrawDEMObjects(ViewNum, ViewSpaces[ViewNum], WCS_DATABASE_STATIC);
			DrawDEMObjects(ViewNum, ViewSpaces[ViewNum], WCS_DATABASE_DYNAMIC);
			// <<<>>> Turn depth write back off here
			glDepthMask(0);
			if (VC->IsPlan())
				{
				DrawVectorObjects(ViewNum, ViewSpaces[ViewNum]);
				} // if
			} // if
		DrawWater(ViewNum, ViewSpaces[ViewNum]);
		if (!VC->GetProjected())
			{
			DrawLightObjects(ViewNum, ViewSpaces[ViewNum]);
			DrawCameraObjects(ViewNum, ViewSpaces[ViewNum]);
			} // if
		DrawCMaps(ViewNum, ViewSpaces[ViewNum]);

// draw search query bounds
// we have no visibility control for them, so we only draw them
// if one is the active object
#ifdef WCS_BUILD_VNS
		DrawActiveSQ(ViewNum, ViewSpaces[ViewNum]);
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_RTX
		DrawExportBounds(ViewNum, ViewSpaces[ViewNum]);
#endif // WCS_BUILD_RTX
		DrawClouds(ViewNum, ViewSpaces[ViewNum]);

		if (PickEnabled != 2)
			{
			if (VC->IsPlan())
				{
				DrawCelestial(ViewNum, ViewSpaces[ViewNum]);
				} // if
			} // if

		} // if (PickEnabled != 2) // 2 = pick points
	if (!PickEnabled)
		{
		DrawVariousLines(ViewNum, ViewSpaces[ViewNum]);
		} // if

	// <<<>>> Turn depth write back on here
	glDepthMask(1);


	DrawCleanup(ViewNum, ViewSpaces[ViewNum]);

	
	if (PickEnabled)
		{
		SelectHits = 0;
		if (DebugSelect)
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "glRenderMode(GL_RENDER);");
			} // if
		SelectHits = glRenderMode(GL_RENDER);
		if (DebugSelect)
			{
			char DbgSel[200];
			sprintf(DbgSel, "%d hits.", SelectHits);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, DbgSel);
			} // if
		if (NewActive = ProcessSelectHits(ViewSpaces[ViewNum], SelectHits))
			{
			NewSelected = NewActive;
			} // if
		PickEnabled = 0;
		} // if
	ClearPointsPick = 0;

	} // if

// Nobody ever seems to set Deferred Grab, so this code is moot.
/*
if (DeferredGrab && DeferredWin && LastMouseValid())
	{
	LatLonFromXY(DeferredGrab, DeferredWin, LastMouseX, LastMouseY, LMLat, LMLon);
	SetLastMouseLatLon(LMLat, LMLon);
	DeferredGrab = NULL;
	DeferredWin = NULL;
	} // if
*/

} // ViewGUI::DrawView

/*===========================================================================*/

RasterAnimHost *ViewGUI::ProcessSelectHits(ViewContext *VC, int NumHits)
{
RasterAnimHost *NewRAH = NULL;
RasterAnimHost *TestRAH;
RasterAnimHostProperties RAHP;
int HitIdx, HitBuf, NumNames, IsPlan = 0;
unsigned long MinZ, MaxZ, NearestZ = UINT_MAX, NearestNonDEMZ = UINT_MAX, NearestName = 0, NearestNonDEMName = 0, FirstName;
	
if (IsPlan = VC->IsPlan())
	{
	NearestZ = UINT_MAX;
	NearestNonDEMZ = UINT_MAX;
	} // if

for (HitIdx = 0, HitBuf = 0; HitIdx < NumHits;)
	{
	FirstName = 0;
	NumNames  = SelBuf[HitBuf++];
	MinZ      = SelBuf[HitBuf++];
	MaxZ      = SelBuf[HitBuf++];
	if (NumNames)
		{
		// grab top name
		FirstName = SelBuf[HitBuf];
		//TestRAH = (RasterAnimHost *)FirstName;
		// skip the rest
		HitBuf += NumNames;
		} // if
	if (FirstName)
		{
		if (IsPlan)
			{
			// if picking points don't test object type
			if (PickEnabled == 2)
				{
				if (MaxZ < NearestZ)
					{
					NearestZ = MaxZ;
					NearestName = FirstName;
					} // if
				} // if
			else
				{
				TestRAH = (RasterAnimHost *)FirstName;
				RAHP.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
				TestRAH->GetRAHostProperties(&RAHP);
				if (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_DEM)
					{
					if (MaxZ < NearestZ)
						{
						NearestZ = MaxZ;
						NearestName = FirstName;
						} // if
					} // if
				else
					{
					if (MaxZ < NearestNonDEMZ)
						{
						NearestNonDEMZ = MaxZ;
						NearestNonDEMName = FirstName;
						} // if
					} // else
				} // if
			} // if
		else
			{
			if (MinZ < NearestZ)
				{
				NearestZ = MinZ;
				NearestName = FirstName;
				} // if
			} // else
		} // if
	HitIdx++;
	} // for

if (NearestName || NearestNonDEMName)
	{
	if (PickEnabled == 2)
		{
		if (ClearPointsPick)
			{
			InterStash->SetParam(1,
			 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0), WCS_VECTOR_PTOPERATE_MAPSELECTED, 
			 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SETPOINTSELECT, 0),
			 NearestName, 1, 0);
			} // if
		else
			{
			InterStash->SetParam(1,
			 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0), WCS_VECTOR_PTOPERATE_MAPSELECTED, 
			 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_TOGGLEPOINTSELECT, 0),
			 NearestName, 0);
			} // else
		Draw();
		} // if
	else
		{
		NewRAH = NearestNonDEMName ? (RasterAnimHost *)NearestNonDEMName: (RasterAnimHost *)NearestName;
		} // else
	} // if

return(NewRAH);

} // ViewGUI::ProcessSelectHits

/*===========================================================================*/

void ViewGUI::DrawSetup(int ViewNum, ViewContext *VC)
{

// Prepare fgl
ViewWins[ViewNum]->fglShow();
ViewWins[ViewNum]->fglDrawBuffer(GL_BACK);
ViewWins[ViewNum]->fglEnable(GL_DEPTH_TEST);
ViewWins[ViewNum]->fglDepthFunc(GL_LEQUAL);

} // ViewGUI::DrawSetup

/*===========================================================================*/

void ViewGUI::SetupLightsAndCameras(int ViewNum, ViewContext *VC)
{
Light *CurrentLight;
Camera *CurrentCam;

if (!CamSetup)
	{
	CamSetup = new RenderData(NULL);
	} // if

if (CamSetup)
	{
	// init cameras
	for (CurrentCam = (Camera *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA));
	 CurrentCam; CurrentCam = (Camera *)(CurrentCam->Next))
		{
		//if (CurrentCam->Enabled)
			{
			CamSetup->InitToView(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->MainProj->Interactive, NULL, CurrentCam, 320, 320);
			CurrentCam->InitToRender(NULL /* VC->RO */, NULL); // ignores RO
			CurrentCam->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
			} // if
		} // for

	// init lights
	for (CurrentLight = (Light *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT));
	 CurrentLight; CurrentLight = (Light *)(CurrentLight->Next))
		{
		if (CurrentLight->Enabled)
			{
			CurrentLight->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
			} // if
		} // for
	} // if

} // ViewGUI::SetupLightsAndCameras

/*===========================================================================*/

void ViewGUI::DrawClear(int ViewNum, ViewContext *VC)
{
Point3f SkyCol;
Point3d SkyGrab;
Sky *FirstSky;

// Clear operations do not seem to be displaylist-able

// look for most appropriate sky (first enabled)
if (VC) // && (!VC->VSky || (VC->VSky && !VC->VSky->Enabled))
	{
	FirstSky = (Sky *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_SKY);
	while (FirstSky)
		{
		if (FirstSky->Enabled)
			break;
		FirstSky = (Sky *)FirstSky->Next;
		} // while

	// Do we have a sky to use?
	VC->SetSky(FirstSky);
	} // if

if (VC && VC->VSky && VC->VSky->Enabled && VC->GetEnabled(WCS_VIEWGUI_ENABLE_SKY))
	{
	VC->VSky->SkyGrad.GetBasicColor(SkyGrab[0], SkyGrab[1], SkyGrab[2], 0.25);
	SkyCol[0] = (float)SkyGrab[0];
	SkyCol[1] = (float)SkyGrab[1];
	SkyCol[2] = (float)SkyGrab[2];
	} // if
else
	{
	SkyCol[0] = SkyCol[1] = SkyCol[2] = 0.0f;
	} // else


// Clear everyone
ViewWins[ViewNum]->fglClearColor(SkyCol[0], SkyCol[1], SkyCol[2], 1.0f);
if (VC->IsPlan())
	{
	ViewWins[ViewNum]->fglClearDepth(1.0);
	} // if
else
	{
	ViewWins[ViewNum]->fglClearDepth(1.0);
	} // else

glDisable(GL_FOG);
glDisable(GL_TEXTURE_1D);
glDisable(GL_TEXTURE_2D);

ViewWins[ViewNum]->fglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

} // ViewGUI::DrawClear

/*===========================================================================*/

void ViewGUI::DrawCamera(int ViewNum, ViewContext *VC)
{
double HFOV, TestFar, CenterX, CenterY, WidthRatio, HeightRatio;
Point3d DistTest;
VertexDEM ViewVec, CamTarget;
unsigned short VWidth, VHeight, SetupWidth;

ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);

if (VC->VCamera)
	{
	// May need to re-init this light properly for correct aspect and smoothing framerate
	if (CamSetup)
		{
		if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_SAFEAREA) && VC->RO)
			{
			// limit render to actual render aspect
			WidthRatio = (double)VWidth / VC->RO->OutputImageWidth;
			HeightRatio = (double)VHeight / VC->RO->OutputImageHeight;
			if (WidthRatio <= HeightRatio)
				{
				SetupWidth = VWidth;
				} // if
			else
				{
				SetupWidth = (unsigned short)(min(VWidth, VC->RO->OutputImageWidth * HeightRatio));
				} // else
			} // if displaying image frame
		else
			{
			SetupWidth = VWidth;
			} // else

		CamSetup->InitToView(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->MainProj->Interactive, VC->RO, VC->VCamera, VWidth, SetupWidth);
		VC->VCamera->InitToRender(VC->RO, NULL);
		VC->VCamera->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // if
	} // if
else
	{
	return;
	} // else


HFOV    = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue(0);
CenterX = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].GetCurValue(0);
CenterY = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].GetCurValue(0);

// multiply percents by image width & height
CenterX *= VWidth;
CenterY *= VHeight;

VC->VCamera->CenterX = CenterX;
VC->VCamera->CenterY = CenterY;

VC->VFOV = 15.0;
if ((VHeight > 0) && (VWidth > 0))
	{
	VC->PixAspect = (double)VWidth / (double)VHeight;
	if (VC->PixAspect > 0)
		{
		VC->VFOV = HFOV / VC->PixAspect;
		} // if
	} // if

// ScrnXYZ[3] to XYZ
// ConvertVtx indicates do CartToDeg
/*
		void ProjectVertexDEM(VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx);
		void UnProjectVertexDEM(VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx);
*/


VC->FetchCurCamCoord();
//CopyPoint3d(LiveCamPos, VC->CamPos.XYZ);
CopyPoint3d(VC->LiveCamPos, VC->VCamera->CamPos->XYZ);

/*
// TargPos is really a view vector, not a position, so add it to
// the camera position to make a position out of it.
double BalanceLen;
BalanceLen = VectorMagnitude(VC->CamPos.XYZ);
//CopyPoint3d(VC->LiveTargetPos, VC->VCamera->TargPos->XYZ);
MultiplyPoint3d(VC->LiveTargetPos, VC->VCamera->TargPos->XYZ, BalanceLen);
AddPoint3d(VC->LiveTargetPos, VC->CamPos.XYZ);
AddPoint3d(VC->LiveCamPos, VC->NegOrigin);
AddPoint3d(VC->LiveTargetPos, VC->NegOrigin);
*/

// Try TargPos as a view vector, not a position. Use with mgluLookAtVec();
{
CopyPoint3d(VC->LiveTargetPos, VC->VCamera->TargPos->XYZ);
AddPoint3d(VC->LiveCamPos, VC->NegOrigin);
}

VC->FarClip = 0.0;
// SceneBounds
CopyPoint3d(DistTest, VC->SceneBoundsPos);		// +,+,+
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[0] = VC->SceneBoundsNeg[0];			// -,+,+
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[1] = VC->SceneBoundsNeg[1];			// -,-,+
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[2] = VC->SceneBoundsNeg[2];			// -,-,-
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[0] = VC->SceneBoundsPos[0];			// +,-,-
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[1] = VC->SceneBoundsPos[1];			// +,+,-
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[0] = VC->SceneBoundsNeg[0];			// -,+,-
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

DistTest[0] = VC->SceneBoundsPos[0];
DistTest[1] = VC->SceneBoundsNeg[1];
DistTest[2] = VC->SceneBoundsPos[2];			// +,-,+
TestFar = PointDistance(DistTest, VC->CamPos.XYZ);
if (TestFar > VC->FarClip) VC->FarClip = TestFar;

// VC->VCamera
// calculate left, right, top, bottom
VC->FrustumLeft   = -CenterX;
VC->FrustumRight  =  (VWidth - CenterX);
VC->FrustumTop    =  CenterY;
VC->FrustumBottom = -(VHeight - CenterY);

VC->GLViewport[0] = 0;
VC->GLViewport[1] = 0;
VC->GLViewport[2] = VWidth;
VC->GLViewport[3] = VHeight;

//VC->FarClip = sqrt(VC->FarClip); // only do sqrt once

if (VC->IsPlan())
	{
	//VC->FarClip = 1.5 * VC->VCCalcExag(VC->SceneHigh + 2.0);
	//VC->FarClip = -10.0;	// Chris' original method
	//VC->NearClip  = max(10.0, (VC->VCCalcExag(VC->SceneHigh + 2.0)));	// Chris' original method
	VC->FarClip = -1000.0;
	if (VC->SceneHigh != -DBL_MAX)
		{
		VC->NearClip  = WCS_max(10.0, 5 * (VC->VCCalcExag(VC->SceneHigh + 2.0)));
		} // if
	else
		{
		VC->NearClip  = 10000.0;
		} // 
	//VC->NearClip = -2400;
	} // if
else
	{
//	VC->NearClip = 1.0;		// so far just using a constant seems OK
	VC->NearClip = VC->FarClip / 5000.0;
	VC->FarClip *= 1.5;		// just a hack to try and get all the DEM in plus a little more clouds
//	VC->NearClip = VC->FarClip / 500.0;	// Chris' original method
	} // else

} // ViewGUI::DrawCamera

/*===========================================================================*/

void ViewGUI::DrawGLProjection(int ViewNum, ViewContext *VC, char Celestial, char FirstPick)
{
double NearClip, FarClip, Invert = 1.0;
double XShift = 0.0, YShift = 0.0, UpP = 1.0, AspMult = 1.0, 
	WidthRatio, HeightRatio, RenderWidth;
unsigned short VWidth, VHeight;

#ifdef WCS_VIEWGUI_INVERT
Invert = -1.0;
#endif // WCS_VIEWGUI_INVERT

// Set up camera
//glViewport(VC->GLViewport[0], VC->GLViewport[1], VC->GLViewport[2], VC->GLViewport[3]);
ViewWins[ViewNum]->fglMatrixMode(GL_PROJECTION);
ViewWins[ViewNum]->fglLoadIdentity(); // on PROJECTION

if (Celestial && (VC->NearCeles != DBL_MAX))
	{
	NearClip = VC->NearCeles * 0.5;
	FarClip  = VC->FarCeles  * 1.5;
	} // if
else
	{
	NearClip = VC->NearClip;
	FarClip  = VC->FarClip;
	} // else

if (PickEnabled == 1)
	{
	gluPickMatrix(PickMatrix[0], PickMatrix[1], PickMatrix[2] + 1.0f, PickMatrix[3] + 1.0f, VC->GLViewport);
	} // if
else if (PickEnabled == 2) // more tolerant
	{
	gluPickMatrix(PickMatrix[0], PickMatrix[1], 8.0, 8.0, VC->GLViewport);
	} // if

//ViewWins[ViewNum]->fglPerspective(VC->VFOV, VC->PixAspect, NearClip, FarClip);

if (VC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	double PlanViewWidthM, PlanViewHeightM, PlanViewWidthDeg, PlanViewHeightDeg, RefLat, RefLon, RefLonScale, RefLatScale;
	double OLeft, ORight, OBot, OTop;

	if (VC->RO)
		{
		AspMult = VC->RO->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;
		if (AspMult == 0.0) AspMult = 1.0; // save idiots from themselves...
		} // if

	#ifdef WCS_BUILD_VNS
	// projected planimetric camera setup
	if (VC->VCamera && VC->VCamera->Projected && VC->VCamera->Coords)
		{
		double globeRadius = LatScale(VC->GlobeRadius);	// avoid macro with side effects warning
		VertexDEM Corner;

		RefLatScale = max(globeRadius, 1.0);	// avoid macro with side effects warning
		ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);

		// Determine edge coords of camera view within translated projected space
		// UL corner (OLeft, OTop)
		Corner.ScrnXYZ[0] = 0.5;
		Corner.ScrnXYZ[1] = 0.5;
		Corner.ScrnXYZ[2] = 1.0;
		VC->VCamera->UnProjectVertexDEM(GlobalApp->AppEffects->FetchDefaultCoordSys(), &Corner, RefLatScale, VC->GlobeRadius, 1);
		// convert to projected system
		VC->VCamera->Coords->DefDegToProj(&Corner);
		// offset by local origin in projected units
		Corner.xyz[0] -= DBCenter.XYZ[0];
		Corner.xyz[1] -= DBCenter.XYZ[1];
		OLeft = Corner.xyz[0];
		OTop  = Corner.xyz[1];

		// LR Corner (ORight, OBot)
		Corner.ScrnXYZ[0] = VWidth  - 0.5;
		Corner.ScrnXYZ[1] = VHeight - 0.5;
		Corner.ScrnXYZ[2] = 1.0;
		VC->VCamera->UnProjectVertexDEM(GlobalApp->AppEffects->FetchDefaultCoordSys(), &Corner, RefLatScale, VC->GlobeRadius, 1);
		// convert to projected system
		VC->VCamera->Coords->DefDegToProj(&Corner);
		// offset by local origin in projected units
		Corner.xyz[0] -= DBCenter.XYZ[0];
		Corner.xyz[1] -= DBCenter.XYZ[1];
		ORight = Corner.xyz[0];
		OBot  = Corner.xyz[1];

		//VC->ProjWidth  = ORight - OLeft;
		//VC->ProjHeight = OTop - OBot;

		glOrtho(OLeft, ORight, OBot, OTop, NearClip, FarClip);
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);

		WidthRatio = (double)VWidth / VC->RO->OutputImageWidth;
		HeightRatio = (double)VHeight / VC->RO->OutputImageHeight;
		if (WidthRatio > HeightRatio)
			{
			RenderWidth = min(VWidth, VC->RO->OutputImageWidth * HeightRatio);
			} // else
		else
			{
			RenderWidth = VWidth;
			} // else

		RefLat = (VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue(0));
		RefLon = (VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue(0));
		RefLonScale = LonScale(VC->GlobeRadius, RefLat);
		RefLonScale = max(RefLonScale, 1.0);
		RefLatScale = LatScale(VC->GlobeRadius);
		RefLatScale = max(RefLatScale, 1.0);
		PlanViewWidthM  = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0);
		PlanViewWidthM *= (double)VWidth / RenderWidth;
		PlanViewWidthDeg = (PlanViewWidthM / RefLonScale);
		UpP = PlanViewWidthM / (double)VWidth;
		PlanViewHeightM   = (double)VHeight * UpP / AspMult;
		PlanViewHeightDeg = (PlanViewHeightM / RefLatScale);
		XShift = VC->VCamera->CenterX - ((double)VWidth * 0.5);
		YShift = VC->VCamera->CenterY - ((double)VHeight * 0.5);
		OLeft  = RefLon + (PlanViewWidthDeg * 0.5);
		ORight = RefLon - (PlanViewWidthDeg * 0.5);
		OBot   = RefLat - (PlanViewHeightDeg * 0.5);
		OTop   = RefLat + (PlanViewHeightDeg * 0.5);
		glOrtho(OLeft, ORight, OBot, OTop, NearClip, FarClip);
		//glOrtho(OLeft, ORight, OBot, OTop, FarClip, NearClip);
		assert(PlanViewWidthDeg != 0.0);
		VC->PlanDegPerPix = PlanViewWidthDeg / (double)VWidth;
		assert(VC->PlanDegPerPix != 0.0);
		//glOrtho(OLeft, ORight, OBot, OTop, -1.0, 1.0);
		glTranslated(XShift * UpP, YShift * UpP, 0.0);
		//ViewWins[ViewNum]->fglDepthFunc(GL_GEQUAL);
		} // else
	} // if
else
	{
	if (VC->VCamera->Orthographic && !VC->VCamera->PanoCam)
		{
		double OrthoViewWidth, OrthoViewHeight;
		if (VC->RO)
			{
			AspMult = VC->RO->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;
			if (AspMult == 0.0) AspMult = 1.0; // save idiots from themselves...
			} // if
		ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);
		OrthoViewWidth  = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0);
		UpP = OrthoViewWidth / (double)VWidth;
		OrthoViewHeight = (double)VHeight * UpP / AspMult;
		XShift = VC->VCamera->CenterX - ((double)VWidth * 0.5);
		YShift = VC->VCamera->CenterY - ((double)VHeight * 0.5);
		glOrtho(OrthoViewWidth * -0.5, OrthoViewWidth * 0.5, OrthoViewHeight * -0.5, OrthoViewHeight * 0.5, NearClip, FarClip);
		glTranslated(XShift * UpP, YShift * UpP, 0.0);
		} // if
	else
		{
		ViewWins[ViewNum]->fglFrustum(
		 VC->FrustumLeft   * (NearClip / VC->VCamera->HorScale),
		 VC->FrustumRight  * (NearClip / VC->VCamera->HorScale),
		 VC->FrustumBottom * (NearClip / VC->VCamera->VertScale),
		 VC->FrustumTop    * (NearClip / VC->VCamera->VertScale),
		 NearClip, FarClip);
		} // else
	} // else

//ViewWins[ViewNum]->fglRotated(-ParamHost->GetMoShift(WCS_MOTION_BANK), 0.0, 0.0, 1.0);

// Setup scene
ViewWins[ViewNum]->fglMatrixMode(GL_MODELVIEW);
ViewWins[ViewNum]->fglLoadIdentity();// on MODELVIEW
//ViewWins[ViewNum]->fglLookAt(0.0, 0.0, 0.0, - RefPt[0], - RefPt[1], RefPt[2], 0.0, 1.0, 0.0);
if (VC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	glScaled(1.0, 1.0, -1.0); // Flip Z axis over for planview
	} // if
else
	{
/*
	ViewWins[ViewNum]->fglLookAt(VC->LiveCamPos[0], VC->LiveCamPos[1], VC->LiveCamPos[2] * Invert,
								 VC->LiveTargetPos[0], VC->LiveTargetPos[1], VC->LiveTargetPos[2] * Invert,
								 VC->VCamera->CamVertical->XYZ[0], VC->VCamera->CamVertical->XYZ[1], VC->VCamera->CamVertical->XYZ[2] * Invert);
*/
	mgluLookAtVec(VC->LiveCamPos[0], VC->LiveCamPos[1], VC->LiveCamPos[2] * Invert,
				  VC->LiveTargetPos[0], VC->LiveTargetPos[1], VC->LiveTargetPos[2] * Invert,
				  VC->VCamera->CamVertical->XYZ[0], VC->VCamera->CamVertical->XYZ[1], VC->VCamera->CamVertical->XYZ[2] * Invert);
	} // else

} // ViewGUI::DrawGLProjection

/*===========================================================================*/

void ViewGUI::DrawLights(int ViewNum, ViewContext *VC)
{
Point4f LightCol, AmbCol, LightPosCartf, LightVecCartf;
Point4d LightPosCart, LightVecCart;
VertexDEM LightPos;
Light *CurLight;
Atmosphere *AS;
float Intensity;
int FXIdx;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

ViewWins[ViewNum]->fglEnable(GL_LIGHTING);
ViewWins[ViewNum]->fglLightModeli(GL_LIGHT_MODEL_TWO_SIDE , 1);
/*	ViewWins[ViewNum]->fglLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER ,1); */

LightCol[0] = AmbCol[0] = 0.0f;
LightCol[1] = AmbCol[1] = 0.0f;
LightCol[2] = AmbCol[2] = 0.0f;
LightCol[3] = AmbCol[3] = 0.0f;

// Ambient is in atmospheres
for (FXIdx = 0, AS = (Atmosphere *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE));
 AS; AS = (Atmosphere *)(AS->Next))
	{
	if (AS->Enabled)
		{
		Intensity = (float)AS->TopAmbientColor.GetIntensity();
		AmbCol[0] += Intensity * (float)AS->TopAmbientColor.GetCurValue(0);
		AmbCol[1] += Intensity * (float)AS->TopAmbientColor.GetCurValue(1);
		AmbCol[2] += Intensity * (float)AS->TopAmbientColor.GetCurValue(2);

		Intensity = (float)AS->BottomAmbientColor.GetIntensity();
		LightCol[0] += Intensity * (float)AS->BottomAmbientColor.GetCurValue(0);
		LightCol[1] += Intensity * (float)AS->BottomAmbientColor.GetCurValue(1);
		LightCol[2] += Intensity * (float)AS->BottomAmbientColor.GetCurValue(2);
		FXIdx ++;
		} // if
	} // if

AmbCol[3]    = 1.0f;
LightCol[3]  = 1.0f;

// Setup ambient light (GL_LIGHT0) at center of planet to apply
// BottomAmbientColor as direct omni light
// TopAmbientColor is scene ambient light (GL_LIGHT_MODEL_AMBIENT)
LightPosCart[0] = LightPosCart[1] = LightPosCart[2] = 0.0;
AddPoint3d(LightPosCart, VC->NegOrigin);

glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AmbCol);

LightPosCartf[0] = (float)LightPosCart[0];
LightPosCartf[1] = (float)LightPosCart[1];
LightPosCartf[2] = (float)LightPosCart[2];
LightPosCartf[3] = 1.0f;

glLightfv(GL_LIGHT0, GL_POSITION, LightPosCartf);
glLightfv(GL_LIGHT0, GL_DIFFUSE, LightCol);
glLightfv(GL_LIGHT0, GL_SPECULAR, LightCol);

AmbCol[0] = 0.0f;
AmbCol[1] = 0.0f;
AmbCol[2] = 0.0f;
AmbCol[3] = 1.0f;

glLightfv(GL_LIGHT0, GL_AMBIENT, AmbCol);
glEnable(GL_LIGHT0);

if (VC->IsPlan() && !SuppressPlanAmbient)
	{
	// force some ambient illumination
	AmbCol[0] = 0.5f;
	AmbCol[1] = 0.5f;
	AmbCol[2] = 0.5f;
	AmbCol[3] = 1.0f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AmbCol);
	} // if

AmbCol[0] = 0.0f;
AmbCol[1] = 0.0f;
AmbCol[2] = 0.0f;
AmbCol[3] = 0.0f;

for (FXIdx = 0, CurLight = (Light *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT));
 CurLight; CurLight = (Light *)(CurLight->Next))
	{
	if (CurLight->Enabled)
		{
		Intensity = (float)CurLight->Color.GetIntensity();
		LightCol[0] = Intensity * (float)CurLight->Color.GetCurValue(0);
		LightCol[1] = Intensity * (float)CurLight->Color.GetCurValue(1);
		LightCol[2] = Intensity * (float)CurLight->Color.GetCurValue(2);
		LightCol[3] = 1.0f;

		LightPos.Lat  = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetCurValue(0);
		LightPos.Lon  = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetCurValue(0);
		LightPos.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetCurValue(0);
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&LightPos);
		#else // WCS_BUILD_VNS
		LightPos.DegToCart(VC->GlobeRadius);
		#endif // WCS_BUILD_VNS
		CopyPoint3d(LightPosCart, LightPos.XYZ);
		AddPoint3d(LightPosCart, VC->NegOrigin);
		InvertZ(LightPosCart);

		LightPosCartf[0] = (float)LightPosCart[0];
		LightPosCartf[1] = (float)LightPosCart[1];
		LightPosCartf[2] = (float)LightPosCart[2];
		LightPosCartf[3] = 1.0f;


		if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_PARALLEL)
			{
			LightPosCartf[3] = 0.0f;
			// Distant means points at center of planet
			if (CurLight->Distant)
				{
				CopyPoint3d(LightVecCart, LightPosCart);
				UnitVector(LightVecCart); // unify
				NegateVector(LightVecCart); // negate
				LightVecCartf[0] = (float)LightVecCart[0];
				LightVecCartf[1] = (float)LightVecCart[1];
				LightVecCartf[2] = (float)LightVecCart[2];
				} // if
			else if (CurLight->LightAim)
				{
				// Stored as negated, negate and use
				LightVecCartf[0] = (float)-CurLight->LightAim->XYZ[0];
				LightVecCartf[1] = (float)-CurLight->LightAim->XYZ[1];
				LightVecCartf[2] = (float)CurLight->LightAim->XYZ[2];
				} // if
			if (!(LightVecCartf[0] == 0.0 && LightVecCartf[1] == 0.0 && LightVecCartf[2] == 0.0))
				{
				glLightfv(GL_LIGHT1 + FXIdx, GL_SPOT_DIRECTION, LightVecCartf);
				} // if
			// NVIDIA: Parallel light sources with SPOT_CUTOFF other than 180.0 fail to work on NVIDIA GeForce cards with driver like .0522
			glLightf(GL_LIGHT1 + FXIdx, GL_SPOT_CUTOFF, 180.0f);
			} // if
		else if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
			{
			float SpotAng;
			if (CurLight->LightAim)
				{
				LightVecCartf[0] = (float)-CurLight->LightAim->XYZ[0];
				LightVecCartf[1] = (float)-CurLight->LightAim->XYZ[1];
				LightVecCartf[2] = (float)CurLight->LightAim->XYZ[2];
				LightVecCartf[3] = 0.0f; // ignored
				if (!(LightVecCartf[0] == 0.0 && LightVecCartf[1] == 0.0 && LightVecCartf[2] == 0.0))
					{
					glLightfv(GL_LIGHT1 + FXIdx, GL_SPOT_DIRECTION, LightVecCartf);
					} // if
				} // if
			SpotAng = (float)(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].GetCurValue(0) * 0.5); // divided by 2
			glLightf(GL_LIGHT1 + FXIdx, GL_SPOT_CUTOFF, SpotAng);
			} // if
		else // omni
			{
			glLightf(GL_LIGHT1 + FXIdx, GL_SPOT_CUTOFF, 180.0f);
			} // else

		// Setup lights
		glLightfv(GL_LIGHT1 + FXIdx, GL_POSITION, LightPosCartf);
		glLightfv(GL_LIGHT1 + FXIdx, GL_DIFFUSE, LightCol);
		glLightfv(GL_LIGHT1 + FXIdx, GL_SPECULAR, LightCol);
		glLightfv(GL_LIGHT1 + FXIdx, GL_AMBIENT, AmbCol);
		// Distant lights do not support falloff
		if (CurLight->Distant)
			{
			glLightf(GL_LIGHT1 + FXIdx, GL_QUADRATIC_ATTENUATION, 0.0f);
			glLightf(GL_LIGHT1 + FXIdx, GL_CONSTANT_ATTENUATION, 1.0f);
			} // if
		else
			{
			if (CurLight->FallOffExp == 0.0)
				{
				glLightf(GL_LIGHT1 + FXIdx, GL_QUADRATIC_ATTENUATION, 0.0f);
				glLightf(GL_LIGHT1 + FXIdx, GL_CONSTANT_ATTENUATION, 1.0f);
				} // if
			else if ((CurLight->FallOffExp > 0.0) && (CurLight->FallOffExp <= 2.0))
				{
				// GL can't modify the falloff exponent like WCS can, so we have
				// to hack it. This ViewGUI code sort of kuldge/hack/fakes the
				// equation 1/D^n (where n=[0.0...2.0] range) as 1/((n/2)*(D^2))
				// The error is most at n=(slightly greater than 0), and diminishes
				// to a perfect result where n=2.
				glLightf(GL_LIGHT1 + FXIdx, GL_QUADRATIC_ATTENUATION, (float)(CurLight->FallOffExp * 0.5));
				glLightf(GL_LIGHT1 + FXIdx, GL_CONSTANT_ATTENUATION, 0.0f);
				} // else if
			else
				{ // exponent greater than 2.0
				// GL can't modify the falloff exponent like WCS can, so we have
				// to hack it. This ViewGUI code sort of kuldge/hack/fakes the
				// equation 1/D^n (where n=[2.0...] range) as 1/(((n - 1.0)^2)*(D^2))
				// There is no error at n=2, and the error increases as n > 2.
				glLightf(GL_LIGHT1 + FXIdx, GL_QUADRATIC_ATTENUATION, (float)((CurLight->FallOffExp - 1.0) * (CurLight->FallOffExp - 1.0)) );
				glLightf(GL_LIGHT1 + FXIdx, GL_CONSTANT_ATTENUATION, 0.0f);
				} // else
			} // else


		glEnable (GL_LIGHT1 + FXIdx);
		FXIdx ++;
		} // if
	} // for

// turn off unneeded lights
GLint GLMaxLights;
glGetIntegerv(GL_MAX_LIGHTS, &GLMaxLights);
for (; FXIdx < (GLMaxLights - 1); FXIdx ++)
	{
	glDisable(GL_LIGHT1 + FXIdx);
	} // for

} // ViewGUI::DrawLights

/*===========================================================================*/

void ViewGUI::DrawCelestial(int ViewNum, ViewContext *VC)
{
Point4f CelesCol;
Point4d PosCart;
VertexDEM Pos;
CelestialEffect *Celest;
Point4f SurfSpecular;
float SurfShine;
float Intensity;
double SizeFac, Radius;
int FXIdx, SwitchBack = 0;
char AmIActive = 0, DoMore = 1, DoThis = 0, IsPlan = 0;
EffectList *EL = NULL;
Light *LinkedLight;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();


if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CELEST))) return;

IsPlan = VC->IsPlan();

ViewWins[ViewNum]->fglDisable(GL_FOG);
glDisable(GL_DEPTH_TEST);

if (!glIsEnabled(GL_CULL_FACE))
	{
	glEnable(GL_CULL_FACE);
	SwitchBack = 1;
	} // if

CelesCol[0] = 0.0f;
CelesCol[1] = 0.0f;
CelesCol[2] = 0.0f;
CelesCol[3] = 0.0f;
CelesCol[3]  = 1.0f;

for (FXIdx = 0, Celest = (CelestialEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CELESTIAL));
 Celest; Celest = (CelestialEffect *)(Celest->Next))
	{
	if (Celest->Enabled)
		{
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (Celest == RasterAnimHost::GetActiveRAHost()) || (Celest == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		Intensity = (float)Celest->Color.GetIntensity();
		CelesCol[0] = Intensity * (float)Celest->Color.GetCurValue(0);
		CelesCol[1] = Intensity * (float)Celest->Color.GetCurValue(1);
		CelesCol[2] = Intensity * (float)Celest->Color.GetCurValue(2);
		CelesCol[3] = 1.0f;

		if (IsPlan)
			{
			Radius = 10.0;
			} // if
		else
			{
			Radius   = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].GetCurValue(0);
			SizeFac  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].GetCurValue(0);
			Radius  *= SizeFac;
			} // else
		
		EL = Celest->Lights;
		do
			{
			DoMore = 0; DoThis = 0;
			if (EL)
				{
				if (LinkedLight = (Light *)EL->Me)
					{
					if (LinkedLight->Enabled)
						{
						Pos.Lat  = LinkedLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetCurValue(0);
						Pos.Lon  = LinkedLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetCurValue(0);
						Pos.Elev = LinkedLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetCurValue(0);
						EL = EL->Next;
						DoMore = 1;
						DoThis = 1;
						} // if
					else
						{
						EL = EL->Next;
						DoMore = 1;
						DoThis = 0;
						} // else
					} // if
				} // if
			else if (!Celest->Lights)
				{
				Pos.Lat  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].GetCurValue(0);
				Pos.Lon  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].GetCurValue(0);
				Pos.Elev = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].GetCurValue(0);
				DoMore = 0;
				DoThis = 1;
				} // if


			if (DoThis)
				{
				if (IsPlan)
					{
					PosCart[0] = Pos.Lon;
					PosCart[1] = Pos.Lat;
					PosCart[2] = 0.0;
					} // if
				else
					{
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&Pos);
					#else // WCS_BUILD_VNS
					Pos.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					CopyPoint3d(PosCart, Pos.XYZ);
					AddPoint3d(PosCart, VC->NegOrigin);
					InvertZ(PosCart);
					PosCart[3] = 0.0;
					} // else

				SurfSpecular[0] = 0.90f;
				SurfSpecular[1] = 0.90f;
				SurfSpecular[2] = 0.90f;
				SurfSpecular[2] = 1.0f;

				SurfShine = 64.0f;

				if (Celest->ShowPhase && !IsPlan) // Can't usefully show phase in plan
					{
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, CelesCol);
					glEnable(GL_BLEND);
					//glBlendFunc(GL_ONE, GL_ONE); // additive
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
					CelesCol[0] = CelesCol[1] = CelesCol[2] = 0.0f;
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, CelesCol);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, CelesCol);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SurfSpecular);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &SurfShine);
					} // if
				else
					{
					// No Phase: Luminous
					glDisable(GL_LIGHTING);
					//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, CelesCol);
					glColor4fv(CelesCol);
					CelesCol[0] = CelesCol[1] = CelesCol[2] = 0.0f;
					/*ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, CelesCol);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SurfSpecular);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &SurfShine);
					ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, CelesCol);
					*/
					} // else

				// Create Sphere
				glPushMatrix(); // on MODELVIEW
				glTranslated(PosCart[0], PosCart[1], PosCart[2]);
				// Draw a solid sphere near the origin
				if (AmIActive)
					{
					DrawSetupActiveObject(VC);
					} // if
				SetupPickName(Celest);
				fauxSolidSphere(Radius);
				ClearPickName();
				if (AmIActive)
					{
					DrawFinishActiveObject(VC);
					} // if
				ViewWins[ViewNum]->fglDisable(GL_FOG);
				glPopMatrix(); // on MODELVIEW
				} // if
			} while (DoMore);
		FXIdx ++;
		} // if
	} // for

CelesCol[0] = CelesCol[1] = CelesCol[2] = 0.0f;
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, CelesCol);

if (SwitchBack)
	{
	glDisable(GL_CULL_FACE);
	} // if
glDisable(GL_BLEND);
glEnable(GL_DEPTH_TEST);
glEnable(GL_LIGHTING);

} // ViewGUI::DrawCelestial

/*===========================================================================*/

//float LightStore[4];
//static int LightOn;
//static int FogOn;
static Point4f OldAmb;
void ViewGUI::DrawSetupActiveObject(ViewContext *VC, int Full)
{
Point4f HazeCol;

if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_ACTIVEOBJ)) return;

HazeCol[0] = 1.0f;
HazeCol[1] = 1.0f;
HazeCol[2] = 0.0f;
HazeCol[3] = 0.5f;

if (Full)
	{
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, -0.01f);
	glFogf(GL_FOG_END, 0.0f); // NVIDIA: if GL_FOG_START and GL_FOG_END are both 0.0, NVIDIA GeForce cards with driver like .0522 crash
	glFogfv(GL_FOG_COLOR, HazeCol);
	} // if
else // typically only used for terrain -- the pixel transfer ends up altering the texture on the DEM
	{
	glPixelTransferf(GL_RED_SCALE, 0.5f);
	glPixelTransferf(GL_GREEN_SCALE, 0.5f);
	glPixelTransferf(GL_BLUE_SCALE, 0.5f);
	glPixelTransferf(GL_RED_BIAS, 0.5f);
	glPixelTransferf(GL_GREEN_BIAS, 0.5f);
	glPixelTransferf(GL_BLUE_BIAS, 0.0f);
	glDisable(GL_FOG);
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, OldAmb);
	HazeCol[0] = 1.0f;
	HazeCol[1] = 1.0f;
	HazeCol[2] = 0.0f;
	HazeCol[3] = 1.0f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, HazeCol);
	} // if

} // ViewGUI::DrawSetupActiveObject

/*===========================================================================*/

void ViewGUI::DrawFinishActiveObject(ViewContext *VC, int Full)
{

if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_ACTIVEOBJ)) return;

if (Full)
	{
	glDisable(GL_FOG);
	DrawFog(VC);
	} // if
else
	{
	glPixelTransferf(GL_RED_SCALE, 1.0f);
	glPixelTransferf(GL_GREEN_SCALE, 1.0f);
	glPixelTransferf(GL_BLUE_SCALE, 1.0f);
	glPixelTransferf(GL_RED_BIAS, 0.0f);
	glPixelTransferf(GL_GREEN_BIAS, 0.0f);
	glPixelTransferf(GL_BLUE_BIAS, 0.0f);
	DrawFog(VC);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, OldAmb);
	} // if

} // ViewGUI::DrawFinishActiveObject

/*===========================================================================*/

void ViewGUI::DrawFog(ViewContext *VC)
{
Point4f HazeCol;
float HazeStart = 0.0f, HazeFull;
Atmosphere *Atmos;

if ((VC->IsPlan()) || (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_ATMOS))))
	{
	glDisable(GL_FOG);
	return;
	} // if


if (Atmos = (Atmosphere *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE)))
	{
	if (Atmos->Enabled && Atmos->HazeEnabled)
		{
		HazeCol[0] = (float)Atmos->HazeColor.GetClampedCompleteValue(0);
		HazeCol[1] = (float)Atmos->HazeColor.GetClampedCompleteValue(1);
		HazeCol[2] = (float)Atmos->HazeColor.GetClampedCompleteValue(2);
		HazeCol[3] = 1.0f; //(float)Atmos->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue;

		// Setup fog
		HazeStart = (float)Atmos->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue;
		HazeFull = (float)(Atmos->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue + HazeStart);

		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, HazeStart);
		glFogf(GL_FOG_END, HazeFull);
		glFogfv(GL_FOG_COLOR, HazeCol);
		} // if
	} // if

} // ViewGUI::DrawFog

/*===========================================================================*/

void ViewGUI::DrawDEMObjects(int ViewNum, ViewContext *VC, unsigned long DBFlags)
{
Point4f EarthSpecular, None, White, Grey, Black, Terrain;
VertexDEM NW, NE, SW, SE, ObjOrig;
Point3d DynamicShift, LocalShift, Up1, Up2, ProjRef;
float EarthShine;
Joe *Clip;
int DEMSteps = 0, ThisList, NoTexture = 0, FogOn = 0, OkToProceed, DrawingDynamic = 0;
JoeDEM *MyDEM;
JoeViewTemp *JVT;
int PlanView = 0;
char AmIActive = 0, Brighten, Filter, Trans, NoLight, Wrap;
double MyMin, ElScale, ExagMax, ExagMin, DEMOriLat, DEMOriLon, PlanCamOriLon, QuickLatScale, GridScaleModifier, TempLon;
int LatSegSteps, LonSegSteps, LatSeg, LonSeg;
double North, South, East, West, TexZScale, TexZTrans, TexZInt;
unsigned char *LoadTexture = NULL;
GLenum TexMode;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ProjPlanSys = NULL;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN) || VC->GetEnabled(WCS_VIEWGUI_ENABLE_DEMEDGE))) return;

if (DBFlags & WCS_DATABASE_DYNAMIC)
	DrawingDynamic = 1;

if (!DrawingDynamic)
	{ // skip static DB DEM items if DEM Paint is open and in Solo
	// we ignore Solo setting if DEMPaint Preview isn't on
	if (GlobalApp && GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview() && GlobalApp->GUIWins->DPG->ShowSolo())
		{
		return;
		} // if
	} // if

// Setup surface attributes
White[0] = 
White[1] = 
White[2] = 
White[3] = 1.0f;

Black[0] = 
Black[1] = 
Black[2] = 0.0f;
Black[3] = 1.0f;

Grey[0] = 
Grey[1] = 
Grey[2] = .5f;
Grey[3] = 1.0f;

None[0] = 
None[1] = 
None[2] = 0.0f;
None[3] = 1.0f;

EarthSpecular[0] = 
EarthSpecular[1] = 
EarthSpecular[2] = 0.25f;
EarthSpecular[3] = 1.0f;

EarthShine = 32.0f;

if (VC->IsPlan())
	{
	PlanView = 1;
	PlanCamOriLon = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
	ProjPlanSys = VC->GetProjected();
	} // if

QuickLatScale = LatScale(VC->GlobeRadius);

FogOn = glIsEnabled(GL_FOG);

ProjRef[0] = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y); // lat
ProjRef[1] = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X); // lon
//ProjRef[2] = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z); // elev

Brighten = 0;

if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_GREY] || VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_EARTH]
 || VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_PRIMARY] || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_CONTOURS])
 || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_SLOPE]) || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_RELEL])
 || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_FRACTAL]) || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_ECOSYS]))
	{
	if ((VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_SLOPE]) || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_RELEL])
	 || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_FRACTAL]) || (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_ECOSYS]))
		{
		TexMode = GL_TEXTURE_2D;
		glDisable(GL_TEXTURE_1D);
		glEnable(GL_TEXTURE_2D);
		if (GLClampEdgeAvailable)
			{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			} // if
		else
			{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			} // else
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Black);
		Brighten = 1;
		Filter = 1;
		LoadTexture = NULL;
		} // if
	else
		{
		TexMode = GL_TEXTURE_1D;
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_1D);
		Wrap = 0;
		glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, Black);
		Brighten = 1;
		Filter = 0;
		if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRADREPEAT])
			{
			Wrap = 1;
			} // Grey
		if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_GREY])
			{
			LoadTexture = GreyTexGrad;
			} // Grey
		else if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_EARTH])
			{
			LoadTexture = EarthTexGrad;
			} // Earth
		else if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_PRIMARY])
			{
			LoadTexture = PrimaryTexGrad;
			} // Primary
		else if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_CONTOURS])
			{
			LoadTexture = ContourTex;
			Brighten = 0;
			Filter = 1;
			Wrap = 1;
			} // Contours

		if (Wrap)
			{
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			} // if
		else
			{
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			} // else

		} // else
	if (Filter)
		{
		glTexParameteri(TexMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(TexMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} // if
	else
		{
		glTexParameteri(TexMode, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(TexMode, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} // else
/*
	if (LoadTexture)
		{
		if (TexMode == GL_TEXTURE_1D)		glTexImage1D(GL_TEXTURE_1D, 0, 3, WCS_VIEWGUI_TEXGRAD_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, LoadTexture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		} // if
*/
	} // if
else
	{
	Brighten = 0;
	NoTexture = 1;
	glTexImage1D(GL_TEXTURE_1D, 0, 3, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	} // Disable
if (Brighten)
	{
	Terrain[0] = 
	Terrain[1] = 
	Terrain[2] = .85f;
	Terrain[3] = 1.0f;
	} // if
else
	{
	Terrain[0] = 
	Terrain[1] = 
	Terrain[2] = .5f;
	Terrain[3] = 1.0f;
	} // else

GridScaleModifier = GlobalApp->MainProj->Interactive->GridOverlaySize.GetCurValue(0);
if (GridScaleModifier <= 0.0)
	{
	GridScaleModifier = ObjectUnit * 10;
	GlobalApp->MainProj->Interactive->GridOverlaySize.SetCurValue(0, GridScaleModifier);
	} // if
GridScaleModifier = 1.0 / GridScaleModifier;


ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Terrain);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, None);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, None);

ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, EarthSpecular);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &EarthShine);


if (Trans = VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS))
	{
	White[3] = 0.7f;
	Grey[3] = 0.7f;
	Terrain[3] = 0.7f;
	Black[3] = 0.7f;
	None[3] = 0.7f;
	EarthSpecular[3] = .7f;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} // if

glPushMatrix(); // on MODELVIEW

for (Clip = LocalDB->GetFirst(DBFlags); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	JVT = NULL;
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP);

				// draw floor
				// sanity check: floor would be completely obscured by terrain in opaque plan views
				if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_MEASURES)) && (!PlanView || (PlanView && VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS) || (PlanView && VC->GetEnabled(WCS_VIEWGUI_ENABLE_POLYEDGE)))))
					{
					if (FogOn) glDisable(GL_FOG);
					if (Trans) glDisable(GL_BLEND);
					// use ScrnXYZ[3] to store lat/lon/elev texture coords respectively
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, White); // max range for texture

					// setup texture
					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					ViewWins[ViewNum]->fglMatrixMode(GL_TEXTURE);
					glLoadIdentity(); // on TEXTURE
					glScaled(GridScaleModifier, GridScaleModifier, 1.0);
					ViewWins[ViewNum]->fglMatrixMode(GL_MODELVIEW);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Black);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // or GL_NEAREST
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					//glTexImage2D(GL_TEXTURE_2D, 0, 3, WCS_VIEWGUI_TEXGRID_SIZE, WCS_VIEWGUI_TEXGRID_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, GridTex);
					gluBuild2DMipmaps(GL_TEXTURE_2D, 1, WCS_VIEWGUI_TEXGRID_SIZE, WCS_VIEWGUI_TEXGRID_SIZE, GL_LUMINANCE, GL_UNSIGNED_BYTE, GridTex);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					

					North = fabs(Clip->NWLat - Clip->SELat);
					West  = fabs(Clip->NWLon - Clip->SELon);
					LatSegSteps = max(4, (int)North);
					LonSegSteps = max(4, (int)West);

					// North and West are initialized to Lat and Lon extent from above
					// Now South and East are calculated to degrees per segment
					South = North / (double)LatSegSteps;
					East  = West  / (double)LonSegSteps;

					if (PlanView)
						{
						NW.Elev = NE.Elev = SW.Elev = SE.Elev = -1.0;
						} // if
					else
						{
						NW.Elev = NE.Elev = SW.Elev = SE.Elev = GlobalFloor;
						} // else

					// We go SE to SW, and south to north.
					for (LonSeg = 0; LonSeg < LonSegSteps; LonSeg++)
						{
						for (LatSeg = 0; LatSeg < LatSegSteps; LatSeg++)
							{
							NE.Lat  = NW.Lat  = Clip->SELat + (double)(LatSeg + 1) * South;
							NW.Lon  = SW.Lon  = Clip->SELon + (double)(LonSeg + 1) * East;
							SW.Lat  = SE.Lat  = Clip->SELat + (double)LatSeg * South; 
							NE.Lon  = SE.Lon  = Clip->SELon + (double)LonSeg * East;

							#ifdef WCS_BUILD_VNS
							DefCoords->DegToCart(&NW);
							DefCoords->DegToCart(&NE);
							DefCoords->DegToCart(&SW);
							DefCoords->DegToCart(&SE);
							#else // WCS_BUILD_VNS
							NW.DegToCart(GlobeRadius);
							NE.DegToCart(GlobeRadius);
							SW.DegToCart(GlobeRadius);
							SE.DegToCart(GlobeRadius);
							#endif // WCS_BUILD_VNS

							UnitVectorCopy(NW.xyz, NW.XYZ);
							UnitVectorCopy(NE.xyz, NE.XYZ);
							UnitVectorCopy(SW.xyz, SW.XYZ);
							UnitVectorCopy(SE.xyz, SE.XYZ);

							// Texture coords
							InvertXYd(NW.xyz);
							NW.ScrnXYZ[0] = (NW.Lat - ProjRef[0]) * QuickLatScale;
							NW.ScrnXYZ[1] = (NW.Lon - ProjRef[1]) * LonScale(VC->GlobeRadius, NW.Lat);
							InvertXYd(NE.xyz);
							NE.ScrnXYZ[0] = (NE.Lat - ProjRef[0]) * QuickLatScale;
							NE.ScrnXYZ[1] = (NE.Lon - ProjRef[1]) * LonScale(VC->GlobeRadius, NE.Lat);
							InvertXYd(SW.xyz);
							SW.ScrnXYZ[0] = (SW.Lat - ProjRef[0]) * QuickLatScale;
							SW.ScrnXYZ[1] = (SW.Lon - ProjRef[1]) * LonScale(VC->GlobeRadius, SW.Lat);
							InvertXYd(SE.xyz);
							SE.ScrnXYZ[0] = (SE.Lat - ProjRef[0]) * QuickLatScale;
							SE.ScrnXYZ[1] = (SE.Lon - ProjRef[1]) * LonScale(VC->GlobeRadius, SE.Lat);

							if (PlanView)
								{
								#ifdef WCS_BUILD_VNS
								if (ProjPlanSys)
									{
									ProjectForViews(VC, &NW);
									ProjectForViews(VC, &NE);
									ProjectForViews(VC, &SW);
									ProjectForViews(VC, &SE);
									} // if
								else
								#endif // WCS_BUILD_VNS
									{
									NW.ProjectAsPlanView();
									NE.ProjectAsPlanView();
									SW.ProjectAsPlanView();
									SE.ProjectAsPlanView();
									} // else
								} // if
							else
								{
								AddPoint3d(NW.XYZ, VC->NegOrigin);
								InvertZ(NW.XYZ);
								AddPoint3d(NE.XYZ, VC->NegOrigin);
								InvertZ(NE.XYZ);
								AddPoint3d(SW.XYZ, VC->NegOrigin);
								InvertZ(SW.XYZ);
								AddPoint3d(SE.XYZ, VC->NegOrigin);
								InvertZ(SE.XYZ);
								} // else
							
							glBegin(GL_QUADS);
							glQuadPolyVertexDEMTex(NW, NE, SE, SW);
							glEnd();

							} // for
						} // for

					glDisable(GL_TEXTURE_2D);
					if (FogOn) glEnable(GL_FOG);
					} // if


				if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN)))
					{
					if (Trans) glEnable(GL_BLEND);
					OkToProceed = 0;
					if (JVT)
						{
						if (PlanView)
							{
							#ifdef WCS_BUILD_VNS
							if (ProjPlanSys)
								{
								ThisList = JVT->ProjPlanDisplayListNum;
								OkToProceed = 1;
								} // if
							else
							#endif // WCS_BUILD_VNS
								{
								ThisList = JVT->PlanDisplayListNum;
								OkToProceed = 1;
								} // else
							} // if
						else
							{
							ThisList = JVT->PerspDisplayListNum;
							OkToProceed = 1;
							} // else
						} // if
					else
						{
						// must not have JVT/displaylist
						if (DrawingDynamic)
							{
							if (MyDEM->Pristine)
								{
								OkToProceed = 1;
								} // if
							} // if
						} // else
					//if ((PlanView && (ThisList = JVT->PlanDisplayListNum)) || (!PlanView && (ThisList = JVT->PerspDisplayListNum)))
					if (OkToProceed)
						{
						char Textured = 1;
						if (RasterAnimHost::GetActiveRAHost())
							AmIActive = (Clip == RasterAnimHost::GetActiveRAHost());
						if (AmIActive)
							{
							DrawSetupActiveObject(VC, 0);
							} // if
						SetupPickName(Clip);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Terrain);

						if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_POLYEDGE)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						// do texturing
						// <<<>>> note, we're downloading the texture once per DEM per frame. Very inefficient.
						glEnable(TexMode);
						if (TexMode == GL_TEXTURE_2D) glDisable(GL_TEXTURE_1D);
						NoLight = 0;
						if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_SLOPE])
							{
							if (JVT && JVT->SlopeTex)
								{
								NoLight = 1;
								glTexImage2D(GL_TEXTURE_2D, 0, 1, JVT->TexMapWidth, JVT->TexMapHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, JVT->SlopeTex);
								} // if
							else
								{
								glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
								} // else
							} // WCS_VIEWGUI_ENABLE_OVER_SLOPE
						else if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_RELEL])
							{
							if (JVT && JVT->RelelTex)
								{
								NoLight = 1;
								glTexImage2D(GL_TEXTURE_2D, 0, 1, JVT->TexMapWidth, JVT->TexMapHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, JVT->RelelTex);
								} // if
							else
								{
								glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
								} // else
							} // WCS_VIEWGUI_ENABLE_OVER_RELEL
						else if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_FRACTAL])
							{
							if (JVT && JVT->FractTex)
								{
								NoLight = 1;
								glTexImage2D(GL_TEXTURE_2D, 0, 1, JVT->TexMapWidth, JVT->TexMapHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, JVT->FractTex);
								} // if
							else
								{
								glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
								} // else
							} // WCS_VIEWGUI_ENABLE_OVER_FRACTAL
						else if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_ECOSYS])
							{
							if (JVT && JVT->EcoTex)
								{
								NoLight = 0;
								glTexImage2D(GL_TEXTURE_2D, 0, 3, JVT->TexMapWidth, JVT->TexMapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, JVT->EcoTex);
								} // if
							else
								{
								glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
								} // else
							} // WCS_VIEWGUI_ENABLE_OVER_ECOSYS
						else
							{ // untextured, need to use the other "Full" Active Object highlighting method
							Textured = 0;
							} // else

						if (NoLight) glDisable(GL_LIGHTING);
						ViewWins[ViewNum]->fglMatrixMode(GL_TEXTURE);
						glLoadIdentity(); // on TEXTURE
						if (TexMode == GL_TEXTURE_1D) glRotated(90.0, 0.0, 1.0, 0.0);
						TexZScale = 1.0;
						TexZTrans = 0.0;
						TexZInt = VC->ContourOverlayInterval.GetCurValue(0);
						if (TexZInt <= 0.0) TexZInt = 100.0;
						if (VC->Enabled[WCS_VIEWGUI_ENABLE_OVER_CONTOURS])
							{
							TexZScale = TexElRange / TexZInt;
							TexZTrans = -TexElMin;
							} // Contours
						else if ((VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_GREY]) || (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_EARTH]) ||  (VC->Enabled[WCS_VIEWGUI_ENABLE_GRAD_PRIMARY]))
							{
							if (VC->Enabled[WCS_VIEWGUI_ENABLE_GRADREPEAT])
								{
								TexZScale = TexElRange / TexZInt;
								TexZTrans = -TexElMin;
								} // if
							} // if
						glTranslated(0.0, 0.0, TexZTrans);
						glScaled(1.0, 1.0, TexZScale);
						ViewWins[ViewNum]->fglMatrixMode(GL_MODELVIEW);
						if (LoadTexture)
							{
							if (TexMode == GL_TEXTURE_1D)		glTexImage1D(GL_TEXTURE_1D, 0, 3, WCS_VIEWGUI_TEXGRAD_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, LoadTexture);
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							Textured = 1;
							} // if
						if (!Textured)
							{
							if (AmIActive)
								{
								DrawFinishActiveObject(VC, 0); // clean up from not-full technique
								DrawSetupActiveObject(VC);
								} // if
							} // if
						if (BGRedraw)
							{
							//if (JVT->MyNextBGRedrawAction == 0)
								{
								LoadOneDEM(ViewNum, 1.0, VC, Clip, JVT);
								//JVT->MyNextBGRedrawAction = 1;
								//break;
								} // if
							//if (JVT->MyNextBGRedrawAction == 1)
								{
								GenOneDEM(PlanView, VC->NegOrigin, VC, Clip, MyDEM->ViewPref);
								FreeDEM(0, VC->NegOrigin, VC, Clip, JVT, MyDEM);
								//JVT->MyNextBGRedrawAction == 2; // done with this
								//break;
								} // if
							} // if
						else
							{
							OkToProceed = 0;
							if (JVT)
								{
								if (glIsList(ThisList))
									{
									OkToProceed = 1;
									} // if
								} // if
							else
								{
								if (DrawingDynamic)
									{
									if (MyDEM->Pristine)
										{
										OkToProceed = 1;
										} // if
									} // if
								} // else
							if (OkToProceed)
								{
								// Translate to local origin
								glPushMatrix(); // on MODELVIEW

								ElScale = MyDEM->FetchBestElScale();
								MyMin = MyDEM->MinEl * (ElScale / ELSCALE_METERS);
								MyMin = VC->VCCalcExag(MyMin);
								if (PlanView)
									{
									#ifdef WCS_BUILD_VNS
									if (ProjPlanSys)
										{
										if (DrawingDynamic)
											{
											GenOneDEM(1, DynamicShift, VC, Clip, MyDEM->Pristine, 1, VC->GetProjected());
											} // if
										else
											{
											if (GlobalApp && GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview() && GlobalApp->GUIWins->DPG->FetchJoe() == Clip)
												{ // skip this DEM, as DEMPaint has it in the Dynamic database
												} // if
											else
												{
												glCallList(ThisList);
												} // else
											} // else
										} // if
									else
									#endif // WCS_BUILD_VNS
										{
										DEMOriLat = (Clip->SELat + Clip->NWLat) * 0.5;
										DEMOriLon = (Clip->SELon + Clip->NWLon) * 0.5;
										TempLon = DEMOriLon - PlanCamOriLon;
										if (fabs(TempLon) > 180.0)
											{
											TempLon += 180.0;
											if (fabs(TempLon) > 360.0)
												TempLon = fmod(TempLon, 360.0);
											if (TempLon < 0.0)
												TempLon += 360.0;
											TempLon -= 180.0;
											DEMOriLon = TempLon + PlanCamOriLon;
											} // if
										// replaced by above
										//while (DEMOriLon - PlanCamOriLon > 180.0)
										//	{
										//	DEMOriLon -= 360.0;
										//	} // while
										//while (DEMOriLon - PlanCamOriLon < -180.0)
										//	{
										//	DEMOriLon += 360.0;
										//	} // while
										LocalShift[1]  = DEMOriLat;
										LocalShift[0]  = DEMOriLon;
										LocalShift[2]  = 0.0;
										glTranslated(LocalShift[0], LocalShift[1], LocalShift[2]);

										if (DrawingDynamic)
											{
											GenOneDEM(1, DynamicShift, VC, Clip, MyDEM->Pristine, 1, VC->GetProjected());
											} // if
										else
											{
											if (GlobalApp && GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview() && GlobalApp->GUIWins->DPG->FetchJoe() == Clip)
												{ // skip this DEM, as DEMPaint has it in the Dynamic database
												} // if
											else
												{
												glCallList(ThisList);
												} // else
											} // else
										//DrawDEMOutline(ViewNum, VC, Clip->NWLat, Clip->SELat, Clip->SELon, Clip->NWLon);
										} // else
									} // if
								else
									{
									// Calculate object local origin (MinEl at SE corner)
									ObjOrig.Lat  = Clip->SELat;
									ObjOrig.Lon  = Clip->SELon;
									ObjOrig.Elev = MyMin;
									#ifdef WCS_BUILD_VNS
									DefCoords->DegToCart(&ObjOrig);
									#else // WCS_BUILD_VNS
									ObjOrig.DegToCart(GlobeRadius);
									#endif // WCS_BUILD_VNS
									if (DrawingDynamic)
										{
										CopyPoint3d(DynamicShift, ObjOrig.XYZ);
										NegateVector(DynamicShift);
										} // if
									InvertZ(ObjOrig.XYZ); // Convert object local origin from WCS world coords to GL world coords
									CopyPoint3d(LocalShift, VC->NegOrigin); // get local origin in WCS world coords
									InvertZ(LocalShift); // convert it to GL world coords for glTranslated
									glTranslated(LocalShift[0] + ObjOrig.XYZ[0], LocalShift[1] + ObjOrig.XYZ[1], LocalShift[2] + ObjOrig.XYZ[2]);

									if (DrawingDynamic)
										{
										GenOneDEM(0, DynamicShift, VC, Clip, MyDEM->Pristine, 1, VC->GetProjected());
										} // if
									else
										{
										if (GlobalApp && GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview() && GlobalApp->GUIWins->DPG->FetchJoe() == Clip)
											{ // skip this DEM, as DEMPaint has it in the Dynamic database
											} // if
										else
											{
											glCallList(ThisList);
											} // else
										} // else
									} // else
								glPopMatrix(); // on MODELVIEW
								} // if
							} // else
						if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_POLYEDGE)) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						ClearPickName();
						if (AmIActive)
							{
							DrawFinishActiveObject(VC, !Textured);
							} // if
						glDisable(GL_TEXTURE_1D);
						if (NoLight) glEnable(GL_LIGHTING);
						} // if
					} // if


				glDisable(GL_LIGHTING);
				if (!PlanView && VC->GetEnabled(WCS_VIEWGUI_ENABLE_DEMEDGE)) // draw bounds
					{
					if (FogOn) glDisable(GL_FOG);
					ExagMax = VC->VCCalcExag((double)MyDEM->MaxEl);
					ExagMin = VC->VCCalcExag((double)MyDEM->MinEl);
					if (AmIActive)
						{
						DrawSetupActiveObject(VC);
						} // if
					SetupPickName(Clip);
					//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, White);
					glColor4fv(White);

					NW.Lat = NE.Lat = Clip->NWLat; 
					NW.Lon = SW.Lon = Clip->NWLon;
					NE.Lon = SE.Lon = Clip->SELon;
					SW.Lat = SE.Lat = Clip->SELat; 
			
					// North Edge
					NW.Elev = ExagMax;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NW);
					#else // WCS_BUILD_VNS
					NW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NW.XYZ, VC->NegOrigin);
					InvertZ(NW.XYZ);
					CopyPoint3d(Up1, NW.XYZ);

					NE.Elev = ExagMax;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NE);
					#else // WCS_BUILD_VNS
					NE.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NE.XYZ, VC->NegOrigin);
					InvertZ(NE.XYZ);
					CopyPoint3d(Up2, NE.XYZ);

					NW.Elev = ExagMin;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NW);
					#else // WCS_BUILD_VNS
					NW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NW.XYZ, VC->NegOrigin);
					InvertZ(NW.XYZ);

					NE.Elev = ExagMin;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NE);
					#else // WCS_BUILD_VNS
					NE.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NE.XYZ, VC->NegOrigin);
					InvertZ(NE.XYZ);

					glBegin(GL_TRIANGLES);
					glTriPoly(Up1, NW.XYZ, Up2);
					glTriPoly(NW.XYZ, NE.XYZ, Up2);
					glEnd();

					// East edge
					SE.Elev =  ExagMax;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SE);
					#else // WCS_BUILD_VNS
					SE.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(SE.XYZ, VC->NegOrigin);
					InvertZ(SE.XYZ);
					CopyPoint3d(Up1, SE.XYZ);
					
					SE.Elev =  ExagMin;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SE);
					#else // WCS_BUILD_VNS
					SE.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(SE.XYZ, VC->NegOrigin);
					InvertZ(SE.XYZ);
					
					glBegin(GL_TRIANGLES);
					glTriPoly(Up2, NE.XYZ, Up1);
					glTriPoly(NE.XYZ, SE.XYZ, Up1);
					glEnd();
					

					// South Edge
					SW.Elev = ExagMax;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SW);
					#else // WCS_BUILD_VNS
					SW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(SW.XYZ, VC->NegOrigin);
					InvertZ(SW.XYZ);
					CopyPoint3d(Up2, SW.XYZ);

					SW.Elev = ExagMin;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SW);
					#else // WCS_BUILD_VNS
					SW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(SW.XYZ, VC->NegOrigin);
					InvertZ(SW.XYZ);

					glBegin(GL_TRIANGLES);
					glTriPoly(Up1, SE.XYZ, Up2);
					glTriPoly(SE.XYZ, SW.XYZ, Up2);
					glEnd();

					// West Edge
					NW.Elev = ExagMax;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NW);
					#else // WCS_BUILD_VNS
					NW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NW.XYZ, VC->NegOrigin);
					InvertZ(NW.XYZ);
					CopyPoint3d(Up1, NW.XYZ);

					NW.Elev = ExagMin;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NW);
					#else // WCS_BUILD_VNS
					NW.DegToCart(GlobeRadius);
					#endif // WCS_BUILD_VNS
					AddPoint3d(NW.XYZ, VC->NegOrigin);
					InvertZ(NW.XYZ);

					glBegin(GL_TRIANGLES);
					glTriPoly(Up2, SW.XYZ, Up1);
					glTriPoly(SW.XYZ, NW.XYZ, Up1);
					glEnd();

					ClearPickName();
					if (AmIActive)
						{
						DrawFinishActiveObject(VC, 0);
						} // if
					if (FogOn) glEnable(GL_FOG);
					} // if

				glEnable(GL_LIGHTING);
				} // if
			} // if
		} // if
	} // for
glPopMatrix(); // on MODELVIEW

glDisable(GL_BLEND);
glDisable(GL_TEXTURE_1D);
glDisable(GL_TEXTURE_2D);

} // ViewGUI::DrawDEMObjects

/*===========================================================================*/

void ViewGUI::DrawVectorObjects(int ViewNum, ViewContext *VC)
{
RasterAnimHost *RAH;
RasterAnimHostProperties RAHP;
char AmIActive = 0;
int PlanView = 0;
Point3d OriginShift;

if (VC->VCamera && (VC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)) PlanView = 1;

glDisable(GL_LIGHTING);
if (VC->IsPlan())
	{
	glDisable(GL_DEPTH_TEST);
	} // if
else
	{
	#ifndef WCS_VIEWGUI_VECTOR_DEPTHTEST
	glDisable(GL_DEPTH_TEST);
	#endif // WCS_VIEWGUI_VECTOR_DEPTHTEST
	#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
	glDepthRange(0.0, WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT);
	#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
	} // else
glPushMatrix(); // on MODELVIEW

glPushMatrix(); // on MODELVIEW

if (PlanView)
	{

	#ifdef WCS_BUILD_VNS
	if ((VC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) && VC->VCamera->Projected && VC->VCamera->Coords)
		{
		// no need to mess with translating origin, already taken care of by glOrtho setup
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]				&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_PLAINVEC)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CONTROLPOINTS)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_AREATERRAFX)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CMAPS)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_GROUND)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_LAKES)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]		&& (VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAINSHADOWS) /* || VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJSHADOWS) */ )) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_SNOW)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_STREAMS)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAFX)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_WAVES)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]);
		if (ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS)) glCallList(ProjPlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]);
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		OriginShift[0] = DBCenter.Lon;
		OriginShift[1] = DBCenter.Lat;
		OriginShift[2] = 0.0;
		glTranslated(OriginShift[0], OriginShift[1], OriginShift[2]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]				&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_PLAINVEC)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CONTROLPOINTS)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_AREATERRAFX)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CMAPS)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_GROUND)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_LAKES)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]		&& (VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAINSHADOWS) /* || VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJSHADOWS) */ )) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_SNOW)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_STREAMS)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAFX)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_WAVES)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]);
		if (PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS)) glCallList(PlanVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]);
		} // else
	} // if
else
	{
	// the next three lines were added to replace the one commented out since vectors are now referenced to a closer origin
	CopyPoint3d(OriginShift, DBCenter.XYZ); // get object in WCS world coords
	InvertZ(OriginShift); // Convert object local origin from WCS world coords to GL world coords
	glTranslated(VC->GLNegOrigin[0] + OriginShift[0], VC->GLNegOrigin[1] + OriginShift[1], VC->GLNegOrigin[2] + OriginShift[2]);
	//glTranslated(VC->GLNegOrigin[0], VC->GLNegOrigin[1], VC->GLNegOrigin[2]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]				&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_PLAINVEC)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CONTROLPOINTS)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_AREATERRAFX)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CMAPS)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_GROUND)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_LAKES)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]		&& (VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAINSHADOWS)/* || VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJSHADOWS) */)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_SNOW)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_STREAMS)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]			&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAFX)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_WAVES)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES]);
	if (PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]		&& VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS)) glCallList(PerspVectorLists[WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS]);
	} // else
glPopMatrix(); // on MODELVIEW

if (RAH = RasterAnimHost::GetActiveRAHost())
	{
	RAHP.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	RAH->GetRAHostProperties(&RAHP);
	AmIActive = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);
	} // if

// Draw temporary vector
if (TempVec)
	{
	PushAndTranslateOrigin(VC);
	GenOneVector(VC->IsPlan(), 0, VC, TempVec, &DBCenter, 0, VC->GetProjected());
	PopOrigin();
	} // if

// redraw the active object
if (AmIActive && VC->GetEnabled(WCS_VIEWGUI_ENABLE_ACTIVEOBJ))
	{
	DrawSetupActiveObject(VC);
	PushAndTranslateOrigin(VC);
	GenOneVector(VC->IsPlan(), 0, VC, (Joe *)(RAH), &DBCenter, 0, VC->GetProjected());
	DrawFinishActiveObject(VC);
	if (InterStash->MVPointsMode())
		{
		GenOneVector(VC->IsPlan(), 0, VC, (Joe *)(RAH), &DBCenter, 2, VC->GetProjected());
		GenOneVector(VC->IsPlan(), 0, VC, (Joe *)(RAH), &DBCenter, 1, VC->GetProjected());
		} // if
	PopOrigin();
	} // if


glPopMatrix();
glEnable(GL_LIGHTING);
glEnable(GL_DEPTH_TEST);
#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
glDepthRange(0.0, 1.0);
#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW

} // ViewGUI::DrawVectorObjects

/*===========================================================================*/

void ViewGUI::DrawDigitizeObject(int ViewNum, ViewContext *VC)
{
Point4f None, Color, Partial;
VertexDEM Node;
float Shine;
int PointLoop;
unsigned char R, G, B, PlanView;
struct DigitizePoints *DI;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

// anything to display?
if (GlobalApp->GUIWins->DIG && GlobalApp->GUIWins->DIG->AddInfo.Point)
	{
	PlanView = VC->IsPlan();
	DI = &GlobalApp->GUIWins->DIG->AddInfo;
	None[0] = 0.0f;
	None[1] = 0.0f;
	None[2] = 0.0f;
	None[3] = 1.0f;

	Shine = 0.0f;

	Color[3] = 1.0f;

	glDisable(GL_LIGHTING);
	if (VC->IsPlan())
		{
		glDisable(GL_DEPTH_TEST);
		} // if
	else
		{
		#ifndef WCS_VIEWGUI_VECTOR_DEPTHTEST
		glDisable(GL_DEPTH_TEST);
		#endif // WCS_VIEWGUI_VECTOR_DEPTHTEST
		#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
		glDepthRange(0.0, WCS_VIEWGUI_VECTOR_DEPTHSKEW_AMOUNT);
		#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW
		} // else
	glPushMatrix(); // on MODELVIEW

	R = (unsigned char)DI->Red;
	G = (unsigned char)DI->Green;
	B = (unsigned char)DI->Blue;
/*	if ((R == 180) && (G == 180) && (B == 180))
		{ // try the view color
		R = RedPart  (Default8[DI->PenNum]);
		G = GreenPart(Default8[DI->PenNum]);
		B = BluePart (Default8[DI->PenNum]);
		} // if
*/
	Color[0] = ((float)(R) / 255.0f);
	Color[1] = ((float)(G) / 255.0f);
	Color[2] = ((float)(B) / 255.0f);
	//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Color);
	glColor4fv(Color);
	Partial[0] = Color[0] * .1f;
	Partial[1] = Color[1] * .1f;
	Partial[2] = Color[2] * .1f;
	Partial[3] = 1.0f;
	//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, None);
	glLineWidth((float)DI->LineWeight);

	glBegin(GL_LINE_STRIP);
	for (PointLoop = 0; PointLoop < DI->Point; PointLoop++)
		{
		if (PlanView)
			{
			#ifdef WCS_BUILD_VNS
			if (VC->GetProjected())
				{
				VertexDEM ProjConvert;

				ProjConvert.Lon  = DI->Lon[PointLoop];
				ProjConvert.Lat  = DI->Lat[PointLoop];
				ProjConvert.Elev = VC->VCCalcExag(DI->Elev[PointLoop]);
				ProjectForViews(VC, &ProjConvert);
				Node.XYZ[0] = ProjConvert.XYZ[0];
				Node.XYZ[1] = ProjConvert.XYZ[1];
				Node.XYZ[2] = ProjConvert.Elev;
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				Node.XYZ[0] = DI->Lon[PointLoop];
				Node.XYZ[1] = DI->Lat[PointLoop];
				Node.XYZ[2] = VC->VCCalcExag(DI->Elev[PointLoop]);
				} // else
			} // if
		else
			{
			Node.Lat  = DI->Lat[PointLoop];
			Node.Lon  = DI->Lon[PointLoop];
			Node.Elev = DI->Elev[PointLoop];

			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&Node);
			#else // WCS_BUILD_VNS
			Node.DegToCart(VC->GlobeRadius);
			#endif // WCS_BUILD_VNS
			// Adjust by local origin
			AddPoint3d(Node.XYZ, VC->NegOrigin);
			InvertZ(Node.XYZ);
			} // else
		glVertex3dv(&Node.XYZ[0]);
		} // for

	glEnd();

	glPopMatrix(); // on MODELVIEW
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	#ifdef WCS_VIEWGUI_VECTOR_DEPTHSKEW
	glDepthRange(0.0, 1.0);
	#endif // WCS_VIEWGUI_VECTOR_DEPTHSKEW

	} // if

} // ViewGUI::DrawDigitizeObject

/*===========================================================================*/

void ViewGUI::DrawCursor(int ViewNum, ViewContext *VC)
{
Point4f Diffuse;
Point3d PosCart, Upper, Lower;
int IsPlan = 0;
CoordSys *IsProj = NULL;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

IsPlan = VC->IsPlan();
IsProj = VC->GetProjected();

if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_CURSOR)) return;
glDisable(GL_LIGHTING);

// Setup surface attributes
// Cursor is blue for now.
Diffuse[0] = 0.0f;
Diffuse[1] = 0.25f;
Diffuse[2] = 1.0f;
Diffuse[3] = 1.0f;

glColor4fv(Diffuse);

if (IsPlan)
	{
	glDisable(GL_DEPTH_TEST);
	} // if
else
	{
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&Cursor);
	#else // WCS_BUILD_VNS
	Cursor.DegToCart(VC->GlobeRadius);
	#endif // WCS_BUILD_VNS
	CopyPoint3d(PosCart, Cursor.XYZ);

	AddPoint3d(PosCart, VC->NegOrigin);
	InvertZ(PosCart);

	glPushMatrix(); // on MODELVIEW

	glTranslated(PosCart[0], PosCart[1], PosCart[2]);
	glStandUpright(Cursor.Lat, Cursor.Lon);
	} // else

glBegin(GL_LINES);

if (IsPlan)
	{
	#ifdef WCS_BUILD_VNS
	if (IsProj)
		{
		double WidthFac;
		VertexDEM CurPosProj;

		WidthFac = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0) * .05;
		// Lat Axis
		CurPosProj.Lon  = Cursor.Lon;
		CurPosProj.Lat  = Cursor.Lat;
		CurPosProj.Elev = 0.0;
		ProjectForViews(VC, &CurPosProj);
		Lower[0] = CurPosProj.XYZ[0] - WidthFac;
		Lower[1] = CurPosProj.XYZ[1];
		Lower[2] = 0.0;
		glVertex3dv(Lower);

		CurPosProj.Lon  = Cursor.Lon;
		CurPosProj.Lat  = Cursor.Lat;
		CurPosProj.Elev = 0.0;
		ProjectForViews(VC, &CurPosProj);
		Upper[0] = CurPosProj.XYZ[0] + WidthFac;
		Upper[1] = CurPosProj.XYZ[1];
		Upper[2] = 0.0;
		glVertex3dv(Upper);

		// Lon Axis
		CurPosProj.Lon  = Cursor.Lon;
		CurPosProj.Lat  = Cursor.Lat;
		CurPosProj.Elev = 0.0;
		ProjectForViews(VC, &CurPosProj);
		Lower[0] = CurPosProj.XYZ[0];
		Lower[1] = CurPosProj.XYZ[1] - WidthFac;
		Lower[2] = 0.0;
		glVertex3dv(Lower);

		CurPosProj.Lon  = Cursor.Lon;
		CurPosProj.Lat  = Cursor.Lat;
		CurPosProj.Elev = 0.0;
		ProjectForViews(VC, &CurPosProj);
		Upper[0] = CurPosProj.XYZ[0];
		Upper[1] = CurPosProj.XYZ[1] + WidthFac;
		Upper[2] = 0.0;
		glVertex3dv(Upper);
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		// Lat Axis
		Lower[0] = Cursor.Lon - (25 * VC->PlanDegPerPix);
		Lower[1] = Cursor.Lat;
		Lower[2] = 0.0;
		glVertex3dv(Lower);

		Upper[0] = Cursor.Lon + (25 * VC->PlanDegPerPix);
		Upper[1] = Cursor.Lat;
		Upper[2] = 0.0;
		glVertex3dv(Upper);

		// Lon Axis
		Lower[0] = Cursor.Lon;
		Lower[1] = Cursor.Lat - (25 * VC->PlanDegPerPix);
		Lower[2] = 0.0;
		glVertex3dv(Lower);

		Upper[0] = Cursor.Lon;
		Upper[1] = Cursor.Lat + (25 * VC->PlanDegPerPix);
		Upper[2] = 0.0;
		glVertex3dv(Upper);
		} // else
	} // if
else
	{
	// X Axis
	Lower[0] = PosCart[0] - 10000.0f;
	Lower[1] = 0.0;
	Lower[2] = 0.0;
	glVertex3dv(Lower);

	Upper[0] = PosCart[0] + 10000.0f;
	Upper[1] = 0.0;
	Upper[2] = 0.0;
	glVertex3dv(Upper);

	// Z Axis
	Lower[0] = 0.0;
	Lower[1] = 0.0;
	Lower[2] = PosCart[2] - 10000.0f;
	glVertex3dv(Lower);

	Upper[0] = 0.0;
	Upper[1] = 0.0;
	Upper[2] = PosCart[2] + 10000.0f;
	glVertex3dv(Upper);
	} // else


if (!IsPlan)
	{
	Diffuse[0] = 1.0f;
	Diffuse[1] = 0.25f;
	Diffuse[2] = 1.0f;
	Diffuse[3] = 1.0f;
	glColor4fv(Diffuse);
	// Y Axis
	Lower[0] = 0.0;
	Lower[1] = PosCart[1] - 10000.0f;
	Lower[2] = 0.0;
	glVertex3dv(Lower);

	Upper[0] = 0.0;
	Upper[1] = PosCart[1] + 10000.0f;
	Upper[2] = 0.0;
	glVertex3dv(Upper);
	} // if

glEnd();

glEnable(GL_LIGHTING);

if (IsPlan)
	{
	glEnable(GL_DEPTH_TEST);
	} // if
else
	{
	glPopMatrix(); // on MODELVIEW
	} // else


//FinishDisplayList();

} // ViewGUI::DrawCursor

/*===========================================================================*/


#ifdef WCS_BUILD_VNS
void ViewGUI::DrawActiveSQ(int ViewNum, ViewContext *VC)
{
Point4f AddColor, RemoveColor;
Point3d /* PosCart, */ Upper, Lower;
int IsPlan = 0;
CoordSys *IsProj = NULL;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
SearchQuery *SQ = NULL;
VertexDEM GeoPoint, BoundNW, BoundNE, BoundSE, BoundSW;

IsPlan = VC->IsPlan();
IsProj = VC->GetProjected();

if (!VC->GetEnabled(WCS_VIEWGUI_ENABLE_ACTIVEOBJ)) return;

RasterAnimHost *ActObj = NULL;
if (ActObj = RasterAnimHost::GetActiveRAHost())
	{
	RasterAnimHostProperties RAHP;
	RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	ActObj->GetRAHostProperties(&RAHP);
	if (RAHP.TypeNumber != WCS_EFFECTSSUBCLASS_SEARCHQUERY)
		{
		return;
		} // if
	else
		{
		SQ = (SearchQuery *)ActObj;
		} // else
	if (!SQ->Filters) // no filters?
		{
		return;
		} // if
	} // if
else
	{
	return;
	} // else


glDisable(GL_LIGHTING);
if (IsPlan)
	{
	glDisable(GL_DEPTH_TEST);
	} // if

// Setup surface attributes
// Add Bounds are pale yellow for now.
// Remove Bounds are orange for now.
AddColor[0] = 1.0f;
AddColor[1] = 1.0f;
AddColor[2] = 0.5f;
AddColor[3] = 1.0f;

RemoveColor[0] = 1.0f;
RemoveColor[1] = 0.5f;
RemoveColor[2] = 0.0f;
RemoveColor[3] = 1.0f;

for (DBFilterEvent *CurFilter = SQ->Filters; CurFilter; CurFilter = CurFilter->Next)
	{
	// fast exits to next filter
	if (!CurFilter->Enabled) continue;
	if (!(CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside || CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)) continue;

	// must be something to do
	if (CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
		{
		glColor4fv(AddColor);
		} // if
	else
		{
		glColor4fv(RemoveColor);
		} // else

	if (CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)
		{ // draw a point
		GeoPoint.Lat = CurFilter->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
		GeoPoint.Lon = CurFilter->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
		GeoPoint.Elev = 0;

/*
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&GeoPoint);
		#else // WCS_BUILD_VNS
		GeoPoint.DegToCart(VC->GlobeRadius);
		#endif // WCS_BUILD_VNS
		CopyPoint3d(PosCart, GeoPoint.XYZ);

		AddPoint3d(PosCart, VC->NegOrigin);
		InvertZ(PosCart);
*/
		} // if

	if (CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside)
		{ // draw a box
		BoundNW.Lat = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
		BoundNW.Lon = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
		BoundNW.Elev = 0;

/*
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&BoundNW);
		#else // WCS_BUILD_VNS
		BoundNW.DegToCart(VC->GlobeRadius);
		#endif // WCS_BUILD_VNS
		CopyPoint3d(PosCart, BoundNW.XYZ);

		AddPoint3d(PosCart, VC->NegOrigin);
		InvertZ(PosCart);
*/
		BoundNE.Lat = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
		BoundNE.Lon = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
		BoundNE.Elev = 0;

		BoundSE.Lat = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		BoundSE.Lon = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
		BoundSE.Elev = 0;

		BoundSW.Lat = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		BoundSW.Lon = CurFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
		BoundSW.Elev = 0;

		// <<<>>> more projection here
		} // if

	if (!IsPlan)
		{

		glPushMatrix(); // on MODELVIEW

/*
		glTranslated(PosCart[0], PosCart[1], PosCart[2]);
		glStandUpright(Cursor.Lat, Cursor.Lon);
*/

		} // if

	glBegin(GL_LINES);

	if (IsPlan)
		{
		#ifdef WCS_BUILD_VNS
		if (IsProj)
			{
			double WidthFac;
			VertexDEM CurPosProj;

			WidthFac = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0) * .02;

			if (CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside)
				{
				// North Side
				CurPosProj.Lon  = BoundNW.Lon;
				CurPosProj.Lat  = BoundNW.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0];
				Lower[1] = CurPosProj.XYZ[1];
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = BoundNE.Lon;
				CurPosProj.Lat  = BoundNE.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0];
				Upper[1] = CurPosProj.XYZ[1];
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// West Side
				CurPosProj.Lon  = BoundNW.Lon;
				CurPosProj.Lat  = BoundNW.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0];
				Lower[1] = CurPosProj.XYZ[1];
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = BoundSW.Lon;
				CurPosProj.Lat  = BoundSW.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0];
				Upper[1] = CurPosProj.XYZ[1];
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// South Side
				CurPosProj.Lon  = BoundSW.Lon;
				CurPosProj.Lat  = BoundSW.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0];
				Lower[1] = CurPosProj.XYZ[1];
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = BoundSE.Lon;
				CurPosProj.Lat  = BoundSE.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0];
				Upper[1] = CurPosProj.XYZ[1];
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// East Side
				CurPosProj.Lon  = BoundNE.Lon;
				CurPosProj.Lat  = BoundNE.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0];
				Lower[1] = CurPosProj.XYZ[1];
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = BoundSE.Lon;
				CurPosProj.Lat  = BoundSE.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0];
				Upper[1] = CurPosProj.XYZ[1];
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				if (CurFilter->GeoBndsInside)
					{ // some kind of distinctive decoration
					} // if
				if (CurFilter->GeoBndsOutside)
					{ // some kind of distinctive decoration
					} // if
				} // if

			if (CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)
				{

				// Lat Axis
				CurPosProj.Lon  = GeoPoint.Lon;
				CurPosProj.Lat  = GeoPoint.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0] - WidthFac;
				Lower[1] = CurPosProj.XYZ[1];
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = GeoPoint.Lon;
				CurPosProj.Lat  = GeoPoint.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0] + WidthFac;
				Upper[1] = CurPosProj.XYZ[1];
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// Lon Axis
				CurPosProj.Lon  = GeoPoint.Lon;
				CurPosProj.Lat  = GeoPoint.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Lower[0] = CurPosProj.XYZ[0];
				Lower[1] = CurPosProj.XYZ[1] - WidthFac;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				CurPosProj.Lon  = GeoPoint.Lon;
				CurPosProj.Lat  = GeoPoint.Lat;
				CurPosProj.Elev = 0.0;
				ProjectForViews(VC, &CurPosProj);
				Upper[0] = CurPosProj.XYZ[0];
				Upper[1] = CurPosProj.XYZ[1] + WidthFac;
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				if (CurFilter->GeoPtContained)
					{ // some kind of distinctive decoration
					} // if
				if (CurFilter->GeoPtUncontained)
					{ // some kind of distinctive decoration
					} // if
				} // if
			} // if
		else
		#endif // WCS_BUILD_VNS
			{
			if (CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)
				{

				// Lat Axis
				Lower[0] = GeoPoint.Lon - (5 * VC->PlanDegPerPix);
				Lower[1] = GeoPoint.Lat;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = GeoPoint.Lon + (5 * VC->PlanDegPerPix);
				Upper[1] = GeoPoint.Lat;
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// Lon Axis
				Lower[0] = GeoPoint.Lon;
				Lower[1] = GeoPoint.Lat - (5 * VC->PlanDegPerPix);
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = GeoPoint.Lon;
				Upper[1] = GeoPoint.Lat + (5 * VC->PlanDegPerPix);
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				if (CurFilter->GeoPtContained)
					{ // some kind of distinctive decoration
					} // if
				if (CurFilter->GeoPtUncontained)
					{ // some kind of distinctive decoration
					} // if
				} // if
			if (CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside)
				{

				// North side
				Lower[0] = BoundNW.Lon;
				Lower[1] = BoundNW.Lat;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = BoundNE.Lon;
				Upper[1] = BoundNE.Lat;
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// West Side
				Lower[0] = BoundNW.Lon;
				Lower[1] = BoundNW.Lat;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = BoundSW.Lon;
				Upper[1] = BoundSW.Lat;
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// South side
				Lower[0] = BoundSW.Lon;
				Lower[1] = BoundSW.Lat;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = BoundSE.Lon;
				Upper[1] = BoundSE.Lat;
				Upper[2] = 0.0;
				glVertex3dv(Upper);

				// East Side
				Lower[0] = BoundNE.Lon;
				Lower[1] = BoundNE.Lat;
				Lower[2] = 0.0;
				glVertex3dv(Lower);

				Upper[0] = BoundSE.Lon;
				Upper[1] = BoundSE.Lat;
				Upper[2] = 0.0;
				glVertex3dv(Upper);


				if (CurFilter->GeoBndsInside)
					{ // some kind of distinctive decoration
					} // if
				if (CurFilter->GeoBndsOutside)
					{ // some kind of distinctive decoration
					} // if
				} // if
			} // else
		} // if
	else
		{
		// 3D Perspective view
		// is this even useful?

		if (CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside)
			{
			// <<<>>>
			if (CurFilter->GeoBndsInside)
				{ // some kind of distinctive decoration
				} // if
			if (CurFilter->GeoBndsOutside)
				{ // some kind of distinctive decoration
				} // if
			} // if

		if (CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)
			{
			// <<<>>>
			if (CurFilter->GeoPtContained)
				{ // some kind of distinctive decoration
				} // if
			if (CurFilter->GeoPtUncontained)
				{ // some kind of distinctive decoration
				} // if
			} // if

/*
// old Cursor drawing code
		// X Axis
		Lower[0] = PosCart[0] - 10000.0f;
		Lower[1] = 0.0;
		Lower[2] = 0.0;
		glVertex3dv(Lower);

		Upper[0] = PosCart[0] + 10000.0f;
		Upper[1] = 0.0;
		Upper[2] = 0.0;
		glVertex3dv(Upper);

		// Z Axis
		Lower[0] = 0.0;
		Lower[1] = 0.0;
		Lower[2] = PosCart[2] - 10000.0f;
		glVertex3dv(Lower);

		Upper[0] = 0.0;
		Upper[1] = 0.0;
		Upper[2] = PosCart[2] + 10000.0f;
		glVertex3dv(Upper);

*/
		} // else


	glEnd();
	} // for


glEnable(GL_LIGHTING);

if (IsPlan)
	{
	glEnable(GL_DEPTH_TEST);
	} // if
else
	{
	glPopMatrix(); // on MODELVIEW
	} // else

//FinishDisplayList();

} // ViewGUI::DrawCursor
#endif // WCS_BUILD_VNS

/*===========================================================================*/

void ViewGUI::DrawCameraObjects(int ViewNum, ViewContext *VC)
{
double Size, CamVA; // Half of actual size
Point4f Specular, Diffuse, Partial /*, PosCartf */;
Point3d PosCart, Zero, Upper, Lower;
float Shine;
//Light *CurrentObj;
Camera *CurrentObj;
VertexDEM Pos;
int FXIdx;
char AmIActive = 0, PlanView, FogOn;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CAMERAS))) return;

FogOn = glIsEnabled(GL_FOG);
glDisable(GL_FOG);

PlanView = VC->IsPlan();

Size = ObjectUnit;

// Setup surface attributes

// Cameras are red for now.
Diffuse[0] = 1.0f;
Diffuse[1] = 0.0f;
Diffuse[2] = 0.0f;
Diffuse[3] = 1.0f;

Partial[0] = Diffuse[0] * .25f;
Partial[1] = Diffuse[1] * .25f;
Partial[2] = Diffuse[2] * .25f;
Partial[3] = 1.0f;

Specular[0] = 0.30f;
Specular[1] = 0.20f;
Specular[2] = 0.20f;
Specular[3] = 1.0f;

Shine = 64.0f;

Zero[0] = 0.0;
Zero[1] = 0.0;
Zero[2] = 0.0;

Upper[0] = 0.0;
Upper[1] = 10.0;
Upper[2] = -20.0;

Lower[0] = 0.0;
Lower[1] = -10.0;
Lower[2] = -20.0;


ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Partial);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &Shine);
Diffuse[0] = 0.5f;
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Partial);

for (FXIdx = 0, CurrentObj = (Camera *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA));
 CurrentObj; CurrentObj = (Camera *)(CurrentObj->Next))
	{
	// Don't try to draw ourselves
	if (CurrentObj->Enabled && CurrentObj != VC->VCamera)
		{
		Point3f LookAtPoint, PlanUpVec;
		VertexDEM Trans;
		double CamMag;
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (CurrentObj == RasterAnimHost::GetActiveRAHost()) || (CurrentObj == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		if (AmIActive)
			{ // not active if a target parameter
			if ((RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]) ||
			 (RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]) ||
			 (RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]))
				{
				AmIActive = 0;
				} // if
			} // if
		CamVA    = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue(0);
		Pos.Lat  = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue(0);
		Pos.Lon  = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue(0);
		Pos.Elev = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue(0);

		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&Pos);
		#else // WCS_BUILD_VNS
		Pos.DegToCart(VC->GlobeRadius);
		#endif // WCS_BUILD_VNS
		CamMag = VectorMagnitude(Pos.XYZ);
		glPushMatrix(); // on MODELVIEW

		if (PlanView)
			{
			PosCart[0] = Pos.Lon;
			PosCart[1] = Pos.Lat;
			PosCart[2] = Pos.Elev;
			assert(VC->PlanDegPerPix != 0.0);
			Size = 4.0 * VC->PlanDegPerPix;
			assert(Size != 0.0);

			Trans.XYZ[0] = (Pos.XYZ[0] + (CurrentObj->TargPos->XYZ[0] * 1.0));
			Trans.XYZ[1] = (Pos.XYZ[1] + (CurrentObj->TargPos->XYZ[1] * 1.0));
			Trans.XYZ[2] = (Pos.XYZ[2] + (CurrentObj->TargPos->XYZ[2] * 1.0));
			#ifdef WCS_BUILD_VNS
			DefCoords->CartToDeg(&Trans);
			#else // WCS_BUILD_VNS
			Trans.CartToDeg(VC->GlobeRadius); // fabricate a target position
			#endif // WCS_BUILD_VNS

			LookAtPoint[0] = (float)(Trans.Lon  - Pos.Lon);
			LookAtPoint[1] = (float)(Trans.Lat  - Pos.Lat);
			LookAtPoint[2] = (float)((Trans.Elev - Pos.Elev) / LatScale(VC->GetRadius()));
			UnitVector(LookAtPoint);

			Trans.XYZ[0] = (Pos.XYZ[0] + (CurrentObj->CamVertical->XYZ[0] * 1.0));
			Trans.XYZ[1] = (Pos.XYZ[1] + (CurrentObj->CamVertical->XYZ[1] * 1.0));
			Trans.XYZ[2] = (Pos.XYZ[2] + (CurrentObj->CamVertical->XYZ[2] * 1.0));
			#ifdef WCS_BUILD_VNS
			DefCoords->CartToDeg(&Trans);
			#else // WCS_BUILD_VNS
			Trans.CartToDeg(VC->GlobeRadius); // fabricate a target position
			#endif // WCS_BUILD_VNS

			PlanUpVec[0] = (float)(Trans.Lon  - Pos.Lon);
			PlanUpVec[1] = (float)(Trans.Lat  - Pos.Lat);
			PlanUpVec[2] = (float)((Trans.Elev - Pos.Elev) / LatScale(VC->GetRadius()));
			UnitVector(PlanUpVec);

			multLookAtVec((float)PosCart[0], (float)PosCart[1], (float)PosCart[2],
			 LookAtPoint[0], LookAtPoint[1], -LookAtPoint[2],
			 PlanUpVec[0], PlanUpVec[1], -PlanUpVec[2]);
			} // if
		else
			{
			CopyPoint3d(PosCart, Pos.XYZ);
			AddPoint3d(PosCart, VC->NegOrigin);
			InvertZ(PosCart);
			if (CurrentObj->TargPos)
				{
				LookAtPoint[0] = (float)(PosCart[0] + (CurrentObj->TargPos->XYZ[0] * CamMag));
				LookAtPoint[1] = (float)(PosCart[1] + (CurrentObj->TargPos->XYZ[1] * CamMag));
				LookAtPoint[2] = (float)(PosCart[2] - (CurrentObj->TargPos->XYZ[2] * CamMag));
				multLookAt((double)PosCart[0], (double)PosCart[1], (double)PosCart[2],
				 (double)LookAtPoint[0], (double)LookAtPoint[1], (double)LookAtPoint[2],
				 (double)CurrentObj->CamVertical->XYZ[0], (double)CurrentObj->CamVertical->XYZ[1], (double)(-CurrentObj->CamVertical->XYZ[2]));
				} // if
			} // else
		glScaled(Size, Size, Size);
		if (AmIActive)
			{
			DrawSetupActiveObject(VC);
			} // if
		SetupPickName(CurrentObj);
		Diffuse[3] = 1.0f;
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
		gldrawbox(-.35, .35, -.5, .5, 0.0, 2.0, GL_QUADS);

		Diffuse[3] = 0.25f;
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// first frustum side
		glPushMatrix(); // on MODELVIEW
		glRotated(CamVA * 0.5, 0.0, 1.0, 0.0);

		glBegin(GL_TRIANGLES);
		glTriPoly(Zero, Upper, Lower);
		glEnd();
		glPopMatrix(); // on MODELVIEW

		glPushMatrix(); // on MODELVIEW
		// second frustum side
		glRotated(-CamVA * 0.5, 0.0, 1.0, 0.0);

		glBegin(GL_TRIANGLES);
		glTriPoly(Zero, Upper, Lower);
		glEnd();

		glDisable(GL_BLEND);
		glPopMatrix(); // on MODELVIEW
		glPopMatrix(); // on MODELVIEW

		ClearPickName();
		if (AmIActive)
			{
			DrawFinishActiveObject(VC);
			} // if
		FXIdx ++;
		} // if
	} // for

if (FogOn) glEnable(GL_FOG);

} // ViewGUI::DrawCameraObjects

/*===========================================================================*/

double ViewGUI::WeighDEMGeometry(void)
{
Joe *Clip;
JoeDEM *MyDEM;
JoeViewTemp *JVT;
double ApproxWeight = 0.0;

LocalDB->ResetGeoClip();

for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP);
				if (!JVT)
					{
					if (JVT = new JoeViewTemp)
						{
						Clip->AddAttribute(JVT);
						// load only the headers
						if (Clip->AttemptLoadDEM(0, LocalProject) && MyDEM->Pristine)
							{
							JVT->MaxPolys = MyDEM->Pristine->CalcPolys();
							MyDEM->Pristine->FreeRawElevs();
							} //  if
						} // if
					} // if
				if (JVT)
					{
					ApproxWeight += (double)JVT->MaxPolys;
					} // if
				} // 
			} // if
		} // if
	} // for

return(ApproxWeight);

} // ViewGUI::WeighDEMGeometry

/*===========================================================================*/

double ViewGUI::Weigh3DObjects(void)
{
long NumPolys, NumVerts;
Object3DEffect *Object3D;

Joe *Current = NULL;
VectorPoint *Point = NULL;

VertexDEM ObjectLocation;

double ApproxWeight = 0.0;

/*
if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ))) return(0.0);
*/

// loop through the initialized 3d objects
for (Object3D = (Object3DEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon); Object3D && Current && Point;
	Object3D = (Object3DEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon))
	{
	NumPolys = NumVerts = 0;
	if (Object3D->DrawEnabled)
		{
		if ((Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_CUBE))
			{
			NumVerts = 8;
			NumPolys = 6;
			} // if draw cube
		else
			{
			NumPolys = Object3D->NumPolys;
			NumVerts = Object3D->NumVertices;
			} // else if draw detail

		if (NumPolys > 0 && NumVerts > 0)
			{
			ApproxWeight += (double)NumPolys;
			} // if object loaded
		} // if draw enabled
	} // for

return(ApproxWeight);

} // ViewGUI::Weigh3DObjects

/*===========================================================================*/

long VGCubeCornerRef[8][3] = {1, 3, 5,  	// v0
							1, 3, 4,  	// v1
							0, 3, 4,  	// v2
							0, 3, 5,  	// v3
							1, 2, 5,  	// v4
							1, 2, 4,  	// v5
							0, 2, 4,  	// v6
							0, 2, 5};	// v7
long VGCubePolyRef[8][3] = {0, 2, 5,		// v0
						  0, 3, 5,		// v1
						  0, 3, 4,		// v2
						  0, 2, 4,		// v3
						  1, 2, 5,		// v4
						  1, 3, 5,		// v5
						  1, 3, 4,		// v6
						  1, 2, 4};		// v7
long VGCubeVertRef[6][4] = {0, 3, 2, 1,	// p0
						  4, 5, 6, 7,	// p1
						  0, 4, 7, 3,	// p2
						  1, 2, 6, 5,	// p3
						  2, 3, 7, 6,	// p4
						  0, 1, 5, 4};	// p5

static unsigned long StaggerNth;

//#define WCS_VIEWGUI_3DOBJ_SEARCHQUERY	draws 3d object instances based on search queries in views - 
// can be very slow if a lot of enabled vectors so it isn't enabled by default

void ViewGUI::Draw3DObjects(int ViewNum, ViewContext *VC, double Simplification)
{
Point3d RefPt;
Point4f VtxCol, Col, Black;
long Ct, Ct1, poly, p0, p1, p2, NumPolys, NumVerts, DrawDetail;
unsigned long Nth, LoopNth;
Object3DEffect *Object3D;
Joe *Current = NULL;
VectorPoint *Point = NULL;
Polygon3D *Poly, *Polygons;
Vertex3D *Vertices;
MaterialEffect *CurMat;
GLDrawingFenetre *ViewWin = NULL;
SearchQuery *ActiveSearch = NULL;
VertexDEM ObjectLocation;
PolygonData PolyDat;
char AmIActive = 0, SwitchBack = 0, ForceBox = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Atmosphere *AS;
Point4f AmbCol, SpecCol, LumCol;
float SpecEx, Intensity;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ))) return;

SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
SpecCol[3] = 1.0f;
LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
LumCol[3] = 1.0f;
AmbCol[0] = AmbCol[1] = AmbCol[2] = 0.0f;
AmbCol[3] = 1.0f;
VtxCol[0] = VtxCol[1] = VtxCol[2] = .5f;
VtxCol[3] = 1.0f;

// Ambient is in atmospheres
for (AS = (Atmosphere *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE));
 AS; AS = (Atmosphere *)(AS->Next))
	{
	if (AS->Enabled)
		{
		Intensity = (float)AS->TopAmbientColor.GetIntensity();
		AmbCol[0] += Intensity * (float)AS->TopAmbientColor.GetCurValue(0);
		AmbCol[1] += Intensity * (float)AS->TopAmbientColor.GetCurValue(1);
		AmbCol[2] += Intensity * (float)AS->TopAmbientColor.GetCurValue(2);
		} // if
	} // if

ViewWin = ViewWins[ViewNum];

if (Simplification < 0.0)
	{
	ForceBox = 1;
	Nth = 1;
	} // if
else if ((Simplification == 0.0) || (Simplification >= 1.0))
	{
	Nth = 1;
	} // if
else
	{
	Nth = (unsigned long int)(1.0 / Simplification);
	} // else

#ifdef WCS_VIEWGUI_STAGGER_POLYS
StaggerNth++;
#endif // WCS_VIEWGUI_STAGGER_POLYS

if (StaggerNth > (Nth - 1)) StaggerNth = 0;

CopyPoint3d(RefPt, VC->NegOrigin);
NegateVector(RefPt);

// init 3d object effects
for (Object3D = (Object3DEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D));
 Object3D; Object3D = (Object3DEffect *)(Object3D->Next))
	{
	if (Object3D->Enabled)
		{
		Object3D->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // if
	} // for

//ViewWin->fglColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//ViewWin->fglEnable(GL_COLOR_MATERIAL);
Black[0] = 
Black[1] = 
Black[2] = 0.0f;
Black[3] = 1.0f;

glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Black);

if (glIsEnabled(GL_CULL_FACE))
	{
	glDisable(GL_CULL_FACE);
	SwitchBack = 1;
	} // if

Col[3] = 1.0f;

// loop through the initialized 3d objects
Object3D = (Object3DEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon);
if (! Object3D)
	{
	Current = NULL;
	Point = NULL;
	#ifdef WCS_VIEWGUI_3DOBJ_SEARCHQUERY
	// search queries
	Object3D = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
	while (Object3D && ! Current)
		{
		if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->Search && Object3D->Search->Enabled)
			{
			if (Object3D->Search->OneFilterValid())
				{
				for (Current = LocalDB->GetFirst(); Current; Current = LocalDB->GetNext(Current))
					{
					if (Current->TestFlags(WCS_JOEFLAG_HASKIDS) || ! Current->TestFlags(WCS_JOEFLAG_ACTIVATED))
						{
						continue;
						} // if 
					if (Current->GetFirstRealPoint() && Object3D->Search->ApproveJoe(Current))
						{
						Point = Current->GetFirstRealPoint();
						ActiveSearch = Object3D->Search;
						break;
						} // if
					} // for
				} // if
			} // if
		if (! Current)
			Object3D = (Object3DEffect *)Object3D->Next;
		} // while
	#endif //
	if (! Object3D)
		{
		// geographic instances
		Object3D = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
		while (Object3D)
			{
			if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->GeographicInstance)
				{
				break;
				} // if
			Object3D = (Object3DEffect *)Object3D->Next;
			} // while
		} // if
	} // if
//for (Object3D = LocalDB->GetNextObject3D(Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon); Object3D && Current && Point;
//	Object3D = LocalDB->GetNextObject3D(Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon))

while (Object3D)
	{
	if (RasterAnimHost::GetActiveRAHost())
		AmIActive = (Object3D == RasterAnimHost::GetActiveRAHost());
	if (AmIActive)
		{
		DrawSetupActiveObject(VC);
		} // if
	SetupPickName(Object3D);
	NumPolys = NumVerts = DrawDetail = 0;
	if (Object3D->DrawEnabled)
		{
		Object3D->FindBasePosition(CamSetup, &ObjectLocation, &PolyDat, Current, Point);
		/*
		if (Object3D->Absolute == WCS_EFFECT_RELATIVETOJOE)	// relative to vector
			{
			ObjectLocation.Elev = Point->Elevation;
			ObjectLocation.Elev = VC->VCCalcExag(ObjectLocation.Elev);
			ObjectLocation.Elev = ObjectLocation.Elev + Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
			} // if
		else if (Object3D->Absolute == WCS_EFFECT_ABSOLUTE)	// absolute
			{
			ObjectLocation.Elev = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
			} // else if
		else						// relative to ground
			{
			ObjectLocation.Elev = InterStash->ElevationPoint(ObjectLocation.Lat, ObjectLocation.Lon);
			ObjectLocation.Elev = VC->VCCalcExag(ObjectLocation.Elev);
			//ObjectLocation.Elev += ((ParamHost->GetMoPar(WCS_MOTION_DATUM) - ObjectLocation.Elev) * ParamHost->GetMoPar(WCS_MOTION_FLATNG));
			ObjectLocation.Elev += Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV].CurValue;
			} // else
		*/
		if ((ForceBox) || (Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_CUBE))
			{
			if (Vertices = new Vertex3D[8])
				{
				NumVerts = 8;
				if (Polygons = new Polygon3D[6])
					{
					NumPolys = 6;
					for (Ct = 0; Ct < 8; Ct ++)
						{
						Vertices[Ct].PolyRef = (long *)AppMem_Alloc(3 * sizeof (long), 0);
						Vertices[Ct].NumPolys = 3;
						for (Ct1 = 0; Ct1 < 3; Ct1 ++)
							{
							Vertices[Ct].xyz[Ct1] = Object3D->ObjectBounds[VGCubeCornerRef[Ct][Ct1]];
							} // for
						for (Ct1 = 0; Ct1 < 3; Ct1 ++)
							{
							Vertices[Ct].PolyRef[Ct1] = VGCubePolyRef[Ct][Ct1];
							} // for
						} // for
					for (Ct = 0; Ct < 6; Ct ++)
						{
						Polygons[Ct].VertRef = (long *)AppMem_Alloc(4 * sizeof (long), 0);
						Polygons[Ct].NumVerts = 4;
						for (Ct1 = 0; Ct1 < 4; Ct1 ++)
							{
							Polygons[Ct].VertRef[Ct1] = VGCubeVertRef[Ct][Ct1];
							} // for
						} // for
					if (Object3D->NameTable && Object3D->NumMaterials > 0)
						{
						if (! Object3D->NameTable[0].Mat && Object3D->NameTable[0].Name[0])
							GlobalApp->AppEffects->SetMakeMaterial(&Object3D->NameTable[0], NULL);
						if (CurMat = Object3D->NameTable[0].Mat)
							{
							//Colr = CurMat->Color;
							Col[0] = (float)CurMat->DiffuseColor.GetCurValue(0);
							Col[1] = (float)CurMat->DiffuseColor.GetCurValue(1);
							Col[2] = (float)CurMat->DiffuseColor.GetCurValue(2);

							// WCS has monostimulus specularity/luminosity, GL has tristimulus specularity/luminosity
							SpecCol[0] = SpecCol[1] = SpecCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
							SpecCol[3] = 1.0f;
							SpecEx = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);

							LumCol[0] = LumCol[1] = LumCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0);
							LumCol[3] = 1.0f;
							} // if
						else
							{
							Col[0] = Col[1] = Col[2] = .5f;
							SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
							SpecCol[3] = 1.0f;
							SpecEx = 1.0f;
							LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
							LumCol[3] = 1.0f;
							} // else
						} // if
					else
						{
						Col[0] = Col[1] = Col[2] = .5f;
						SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
						SpecCol[3] = 1.0f;
						SpecEx = 1.0f;
						LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
						LumCol[3] = 1.0f;
						} // else
					} // if polygons
				} // if vertices
			LoopNth = 1; // draw all cube faces w/o decimation
			} // if draw cube
		else if ((Object3D->Vertices && Object3D->Polygons && Object3D->NameTable) || Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE))
			{
			NumPolys = Object3D->NumPolys;
			NumVerts = Object3D->NumVertices;
			Vertices = Object3D->Vertices;
			Polygons = Object3D->Polygons;
			DrawDetail = 1;
			LoopNth = Nth;
			} // else if draw detail

		if (NumPolys > 0 && NumVerts > 0)
			{
			VC->VCamera->ProjectVertexDEM(DefCoords, &ObjectLocation, CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 1);
			if (Object3D->Transform(Vertices, NumVerts, CamSetup, &PolyDat, &ObjectLocation, NULL, -1.0))
				{
				// project all the object vertices
				if (VC->IsPlan())
					{
					for (Ct = 0; Ct < NumVerts; Ct ++)
						{
						#ifdef WCS_BUILD_VNS
						DefCoords->CartToDeg(&Vertices[Ct]);
						#else // WCS_BUILD_VNS
						Vertices[Ct].CartToDeg(CamSetup->PlanetRad);
						#endif // WCS_BUILD_VNS
						VC->VCamera->ProjectVertexDEM(DefCoords, &Vertices[Ct], CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 0);
						} // for
					} // if
				for (Ct = 0; Ct < NumPolys; Ct ++)
					Polygons[Ct].Normalized = 0;
				if (LoopNth == 1)
					{
					Ct = 0;
					} // if
				else
					{
					Ct = (StaggerNth % LoopNth);
					} // else
				for (; Ct < NumPolys; Ct += LoopNth)
					{
					Poly = &Polygons[Ct];
					if (DrawDetail)
						{
						if (Object3D->NameTable[Poly->Material].Mat ||
							(Object3D->NameTable[Poly->Material].Name[0] && (Object3D->NameTable[Poly->Material].Mat = GlobalApp->AppEffects->SetMakeMaterial(&Object3D->NameTable[Poly->Material], NULL))))
							{
							CurMat = Object3D->NameTable[Poly->Material].Mat;
							if (CurMat->Shading == WCS_EFFECT_MATERIAL_SHADING_INVISIBLE ||
								! CurMat->Enabled)
								continue;
							// compute surface and vertex normals
							Poly->Normalize(Vertices);
							if (CurMat->FlipNormal)
								NegateVector(Poly->Normal);
							if (CurMat->Shading == WCS_EFFECT_MATERIAL_SHADING_PHONG)
								{
								for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
									{
									Vertices[Poly->VertRef[p0]].Normalized = 0;
									Vertices[Poly->VertRef[p0]].Normalize(Polygons, Vertices, Poly, Object3D->NameTable);
									} // for
								} // if gouraud or phong
							else
								{
								for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
									{
									Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
									Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
									Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
									} // for
								} // else
							Col[0] = (float)CurMat->DiffuseColor.GetCurValue(0);
							Col[1] = (float)CurMat->DiffuseColor.GetCurValue(1);
							Col[2] = (float)CurMat->DiffuseColor.GetCurValue(2);

							// WCS has monostimulus specularity/luminosity, GL has tristimulus specularity/luminosity
							// <<<>>> WCS/VNS now have a real specular color we could use
							SpecCol[0] = SpecCol[1] = SpecCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
							SpecCol[3] = 1.0f;
							SpecEx = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);

							LumCol[0] = LumCol[1] = LumCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0);
							LumCol[3] = 1.0f;
							} // if
						else
							{
							for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
								{
								Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
								Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
								Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
								} // for
							Col[0] = (float).5;
							Col[1] = (float).5;
							Col[2] = (float).5;
							SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
							SpecCol[3] = 1.0f;
							SpecEx = 1.0f;
							LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
							LumCol[3] = 1.0f;
							} // else
						} // if
					else
						{
						// compute surface and vertex normals
						Poly->Normalize(Vertices);
						for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
							{
							Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
							Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
							Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
							} // for
						} // else

					p0 = 0;
					p1 = 1;
					p2 = Poly->NumVerts - 1;
					poly = 0;
					while (p2 > p1)
						{
						ViewWin->fglBegin(GL_POLYGON);
						// do some ambient/specularity/transparency too
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Col);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbCol);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, LumCol);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecCol);
						glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS , SpecEx);
						ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p0]].Normal[0],
							-Vertices[Poly->VertRef[p0]].Normal[1], Vertices[Poly->VertRef[p0]].Normal[2]); 
						ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p1]].Normal[0],
							-Vertices[Poly->VertRef[p1]].Normal[1], Vertices[Poly->VertRef[p1]].Normal[2]); 
						ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p2]].Normal[0],
							-Vertices[Poly->VertRef[p2]].Normal[1], Vertices[Poly->VertRef[p2]].Normal[2]);
						if (VC->IsPlan())
							{
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p0]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p0]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p0]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p0]].Lon,
								Vertices[Poly->VertRef[p0]].Lat, Vertices[Poly->VertRef[p0]].Elev); 
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p1]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p1]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p1]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p1]].Lon,
								Vertices[Poly->VertRef[p1]].Lat, Vertices[Poly->VertRef[p1]].Elev); 
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p2]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p2]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p2]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p2]].Lon,
								Vertices[Poly->VertRef[p2]].Lat, Vertices[Poly->VertRef[p2]].Elev); 
							} // if plan view
						else
							{
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p0]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p0]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p0]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p0]].XYZ[0] - RefPt[0],
								Vertices[Poly->VertRef[p0]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p0]].XYZ[2] - RefPt[2])); 
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p1]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p1]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p1]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p1]].XYZ[0] - RefPt[0],
								Vertices[Poly->VertRef[p1]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p1]].XYZ[2] - RefPt[2])); 
//							if (CurMat && CurMat->UseVertexColor)
								{
//								VtxCol[0] = Vertices[Poly->VertRef[p2]].RGB[0];
//								VtxCol[1] = Vertices[Poly->VertRef[p2]].RGB[1];
//								VtxCol[2] = Vertices[Poly->VertRef[p2]].RGB[2];
//								ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VtxCol);
								} // if
							ViewWin->fglVertex3d(Vertices[Poly->VertRef[p2]].XYZ[0] - RefPt[0],
								Vertices[Poly->VertRef[p2]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p2]].XYZ[2] - RefPt[2])); 
							} // else
						ViewWin->fglEnd();

						if (poly %2)
							{
							p0 = p2;
							p2 --;
							} // if
						else
							{
							p0 = p1;
							p1 ++;
							} // else
						poly ++;
						} // while p2 > p1
					} // for each polygon
				} // if Transform - returns 0 if no vertices
			} // if object loaded
		if (Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_CUBE)
			{
			if (Polygons)
				delete [] Polygons;
			Polygons = NULL;
			if (Vertices)
				delete [] Vertices;
			Vertices = NULL;
			} // if
		} // if draw enabled
	ClearPickName();
	if (AmIActive)
		{
		DrawFinishActiveObject(VC);
		} // if
	if (Current && Point)
		{
		#ifdef WCS_VIEWGUI_3DOBJ_SEARCHQUERY
		if (! ActiveSearch)
			{
		#endif // WCS_VIEWGUI_3DOBJ_SEARCHQUERY
			Object3D = (Object3DEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon);
			if (! Object3D)
				{
				Current = NULL;
				Point = NULL;
				#ifdef WCS_VIEWGUI_3DOBJ_SEARCHQUERY
				// search queries
				Object3D = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
				while (Object3D && ! Current)
					{
					if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->Search && Object3D->Search->Enabled)
						{
						if (Object3D->Search->OneFilterValid())
							{
							for (Current = LocalDB->GetFirst(); Current; Current = LocalDB->GetNext(Current))
								{
								if (Current->TestFlags(WCS_JOEFLAG_HASKIDS) || ! Current->TestFlags(WCS_JOEFLAG_ACTIVATED))
									{
									continue;
									} // if 
								if (Current->GetFirstRealPoint() && Object3D->Search->ApproveJoe(Current))
									{
									Point = Current->GetFirstRealPoint();
									ActiveSearch = Object3D->Search;
									break;
									} // if
								} // for
							} // if
						} // if
					if (! Current)
						Object3D = (Object3DEffect *)Object3D->Next;
					} // while
				#endif // WCS_VIEWGUI_3DOBJ_SEARCHQUERY
				} // if
		#ifdef WCS_VIEWGUI_3DOBJ_SEARCHQUERY
			} // if
		else	// search query
			{
			Point = Point->Next;
			if (! Point)
				{
				for (Current = LocalDB->GetNext(Current); Current; Current = LocalDB->GetNext(Current))
					{
					if (Current->TestFlags(WCS_JOEFLAG_HASKIDS) || ! Current->TestFlags(WCS_JOEFLAG_ACTIVATED))
						{
						continue;
						} // if 
					if (Current->GetFirstRealPoint() && Object3D->Search->ApproveJoe(Current))
						{
						Point = Current->GetFirstRealPoint();
						break;
						} // if
					} // for
				if (! Current)
					Object3D = (Object3DEffect *)Object3D->Next;
				while (Object3D && ! Current)
					{
					if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->Search && Object3D->Search->Enabled)
						{
						if (Object3D->Search->OneFilterValid())
							{
							for (Current = LocalDB->GetFirst(); Current; Current = LocalDB->GetNext(Current))
								{
								if (Current->TestFlags(WCS_JOEFLAG_HASKIDS) || ! Current->TestFlags(WCS_JOEFLAG_ACTIVATED))
									{
									continue;
									} // if 
								if (Current->GetFirstRealPoint() && Object3D->Search->ApproveJoe(Current))
									{
									Point = Current->GetFirstRealPoint();
									ActiveSearch = Object3D->Search;
									break;
									} // if
								} // for
							} // if
						} // if
					if (! Current)
						Object3D = (Object3DEffect *)Object3D->Next;
					} // while
				if (! Object3D)
					ActiveSearch = NULL;
				} // if
			} // else
		#endif // WCS_VIEWGUI_3DOBJ_SEARCHQUERY
		} // if
	if (! Object3D)
		{
		Current = NULL;
		Point = NULL;
		Object3D = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
		while (Object3D)
			{
			if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->GeographicInstance)
				break;
			Object3D = (Object3DEffect *)Object3D->Next;
			} // while
		} // if no more vector instances
	else if (! Current || ! Point)
		{
		Object3D = (Object3DEffect *)Object3D->Next;
		while (Object3D)
			{
			if (Object3D->Enabled && Object3D->DrawEnabled && ! Object3D->ShadowsOnly && Object3D->GeographicInstance)
				break;
			Object3D = (Object3DEffect *)Object3D->Next;
			} // while
		} // else if we're testing for more geographic instances
	} // while

//ViewWin->fglDisable(GL_COLOR_MATERIAL);

if (SwitchBack)
	{
	glEnable(GL_CULL_FACE);
	} // if

} // ViewGUI::Draw3DObjects

/*===========================================================================*/

void ViewGUI::DrawWalls(int ViewNum, ViewContext *VC)
{
#ifdef WCS_VIEW_DRAWWALLS
Point3d RefPt;
Point4f VtxCol, Col, Black;
GLDrawingFenetre *ViewWin = NULL;
char AmIActive = 0, SwitchBack = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Atmosphere *AS;
Point4f AmbCol, SpecCol, LumCol;
float SpecEx, Intensity;
RenderJoeList *RJL = NULL, *CurJL = NULL;
FenceVertexList *ListElement;
int FenceVertexToRender, BuildList = 0;
int IsProj = 0;
#endif // WCS_VIEW_DRAWWALLS

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_WALLS))) return;

#ifndef WCS_VIEW_DRAWWALLS
return; // nothing to do
#else // WCS_VIEW_DRAWWALLS

// set up surfaces and color attributes
AmbCol[0] = AmbCol[1] = AmbCol[2] = 0.0f;
AmbCol[3] = 1.0f;
VtxCol[0] = VtxCol[1] = VtxCol[2] = .5f;
VtxCol[3] = 1.0f;

// Ambient is in atmospheres
for (AS = (Atmosphere *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE));
 AS; AS = (Atmosphere *)(AS->Next))
	{
	if (AS->Enabled)
		{
		Intensity = (float)AS->TopAmbientColor.GetIntensity();
		AmbCol[0] += Intensity * (float)AS->TopAmbientColor.GetCurValue(0);
		AmbCol[1] += Intensity * (float)AS->TopAmbientColor.GetCurValue(1);
		AmbCol[2] += Intensity * (float)AS->TopAmbientColor.GetCurValue(2);
		} // if
	} // if

Black[0] = 
Black[1] = 
Black[2] = 0.0f;
Black[3] = 1.0f;

glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Black);

if (glIsEnabled(GL_CULL_FACE))
	{
	glDisable(GL_CULL_FACE);
	SwitchBack = 1;
	} // if

Col[3] = 1.0f;
Col[0] = Col[1] = Col[2] = .5f;
SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
SpecCol[3] = 1.0f;
SpecEx = 1.0f;
LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
LumCol[3] = 1.0f;

CopyPoint3d(RefPt, VC->NegOrigin);
NegateVector(RefPt);

IsProj = VC->GetProjected() ? 1 : 0;

// see if we can just run with what we have displaylisted
if (VC->IsPlan())
	{
	#ifdef WCS_BUILD_VNS
	if (IsProj)
		{ // WallProjPlanList
		if ((WallProjPlanList != -1) && glIsList(WallProjPlanList))
			{
			// no local origin matrix translate for Plan walls which are built in pure lat/lon
			glCallList(WallProjPlanList);
			} // if
		else
			{
			BuildList = 1;
			} // else
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		if ((WallPlanList != -1) && glIsList(WallPlanList))
			{
			// no local origin matrix translate for Plan walls which are built in pure lat/lon
			glCallList(WallPlanList);
			} // if
		else
			{
			BuildList = 1;
			} // else
		} // else
	} // if
else
	{
	// preserve old matrix state
	glPushMatrix();
	if ((WallPerspList != -1) && glIsList(WallPerspList))
		{
		// setup local origin matrix translate using WallPerspOri
		glTranslatef((float)(WallPerspOri[0] - RefPt[0]), (float)(WallPerspOri[1] - RefPt[1]), (float)-(WallPerspOri[2] - RefPt[2]));
		glCallList(WallPerspList);
		} // if
	else
		{
		BuildList = 1;
		} // else
	} // else

if (BuildList)
	{
	// init walls
	if (RJL = GlobalApp->AppDB->CreateRenderJoeList(GlobalApp->AppEffects, WCS_EFFECTSSUBCLASS_FENCE))
		{
		GlobalApp->AppEffects->FnceBase.Init(GlobalApp->AppDB, RJL);
		} // if
	else
		{
		goto EndWall;
		} // else
	if (!VC->IsPlan())
		{
		// set up WallPerspOri to first vertex
		if (GlobalApp->AppEffects->FnceBase.VerticesToRender)
			{
			VertexDEM WPI;

			WPI.Lat  = GlobalApp->AppEffects->FnceBase.VertList[0].Lat;
			WPI.Lon  = GlobalApp->AppEffects->FnceBase.VertList[0].Lon;
			WPI.Elev = GlobalApp->AppEffects->FnceBase.VertList[0].Elev;
			VC->VCamera->ProjectVertexDEM(DefCoords, &WPI, CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 1);
			WallPerspOri[0] = WPI.XYZ[0];
			WallPerspOri[1] = WPI.XYZ[1];
			WallPerspOri[2] = WPI.XYZ[2];
			glTranslatef((float)(WallPerspOri[0] - RefPt[0]), (float)(WallPerspOri[1] - RefPt[1]), (float)-(WallPerspOri[2] - RefPt[2]));
			} // if
		else
			{
			WallPerspOri[0] = WallPerspOri[1] = WallPerspOri[2] = 0.0;
			} // else
		} // if
	} // if



if (BuildList)
	{
	if (VC->IsPlan())
		{
		#ifdef WCS_BUILD_VNS
		if (VC->GetProjected())
			{
			WallProjPlanList = glGenLists(1);
			// if list building fails, we'll just execute
			glNewList(WallProjPlanList, GL_COMPILE_AND_EXECUTE);
			} // if
		else
		#endif // WCS_BUILD_VNS
			{
			WallPlanList = glGenLists(1);
			// if list building fails, we'll just execute
			glNewList(WallPlanList, GL_COMPILE_AND_EXECUTE);
			} // else
		} // if
	else
		{
		WallPerspList = glGenLists(1);
		glNewList(WallPerspList, GL_COMPILE_AND_EXECUTE);
		} // else

	ViewWin = ViewWins[ViewNum];

	// This code stolen from int Renderer::RenderFence(FenceVertexList *ListElement)
	for (FenceVertexToRender = 0; FenceVertexToRender < GlobalApp->AppEffects->FnceBase.VerticesToRender; FenceVertexToRender++)
		{
		#ifndef WCS_FENCE_LIMITED
		double Area, Angle, RoofElev;
		int Clockwise, Legal, BreakToOuterLoop;
		long NumRoofPts, FirstCt, NextCt, NextNextCt, TestCt, NumToTest, Illegal;
		VectorPoint **RoofPts, *PLink, *PLinkPrev;
		#endif // WCS_FENCE_LIMITED
		double PolySide[2][3], TopElev, BotElev;
        #ifdef WCS_THEMATIC_MAP
        double ThemeValue[3];
        #endif // WCS_THEMATIC_MAP
		unsigned long Flags;
		long Ct;
		int Success = 1;
		JoeCoordSys *MyAttr;
		CoordSys *MyCoords;
		VertexData *VtxPtr[3];
		VertexData Vtx[4];
		VertexBase TempVert;
		PolygonData Poly;

		ListElement = &(GlobalApp->AppEffects->FnceBase.VertList[FenceVertexToRender]); // get pointer to current Vertex in array

		if (MyAttr = (JoeCoordSys *)ListElement->Vec->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;

		CamSetup->TexData.VDataVecOffsetsValid = 0;

		switch (ListElement->PieceType)
			{
			case WCS_FENCEPIECE_SPAN:
				{
				if (!VC->IsPlan())
					{
					// 0 and 2 are top, 1 and 3 are bottom
					ListElement->PointA->ProjToDefDeg(MyCoords, &Vtx[0]);
					Vtx[1].CopyLatLon(&Vtx[0]);
					ListElement->PointB->ProjToDefDeg(MyCoords, &Vtx[2]);
					Vtx[3].CopyLatLon(&Vtx[2]);

					TopElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue;
					BotElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue;
					#ifdef WCS_THEMATIC_MAP
					if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV) &&
						//ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)->VectorIsPointData &&
						ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)->Eval(ThemeValue, ListElement->Vec))
						TopElev = ThemeValue[0];
					if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV) &&
						//ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)->VectorIsPointData &&
						ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)->Eval(ThemeValue, ListElement->Vec))
						BotElev = ThemeValue[0];
					#endif // WCS_THEMATIC_MAP

					#ifdef WCS_FENCE_LIMITED
					if (TopElev > 5.0)
						TopElev = 5.0;
					if (BotElev > 5.0)
						BotElev = 5.0;
					if (TopElev < -10.0)
						TopElev = -10.0;
					if (BotElev < -10.0)
						BotElev = -10.0;
					#endif // WCS_FENCE_LIMITED

					if (ListElement->Fnce->Absolute == WCS_EFFECT_RELATIVETOJOE)
						{
						Vtx[0].Elev = TopElev + CamSetup->ElevDatum + (Vtx[0].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
						Vtx[1].Elev = BotElev + CamSetup->ElevDatum + (Vtx[1].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
						Vtx[2].Elev = TopElev + CamSetup->ElevDatum + (Vtx[2].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
						Vtx[3].Elev = BotElev + CamSetup->ElevDatum + (Vtx[3].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
						} // if
					else if (ListElement->Fnce->Absolute == WCS_EFFECT_ABSOLUTE)
						{
						Vtx[0].Elev = TopElev;
						Vtx[1].Elev = BotElev;
						Vtx[2].Elev = TopElev;
						Vtx[3].Elev = BotElev;
						} // else if
					else
						{
						Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
							WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
						CamSetup->Interactive->VertexDataPoint(CamSetup, &Vtx[0], &Poly, Flags);
						Vtx[1].Elev = Vtx[0].Elev + BotElev;
						Vtx[0].Elev += TopElev;
						CamSetup->Interactive->VertexDataPoint(CamSetup, &Vtx[2], &Poly, Flags);
						Vtx[3].Elev = Vtx[2].Elev + BotElev;
						Vtx[2].Elev += TopElev;
						} // else

					for (Ct = 0; Ct < 4; Ct ++)
						{
						// convert degrees to cartesian and project to screen
						// <<<>>> do we need to project?
						VC->VCamera->ProjectVertexDEM(DefCoords, &Vtx[Ct], CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 1);
						} // for

					Poly.Vector = ListElement->Vec;
					Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS;
					Poly.Lat = ListElement->Lat;
					Poly.Lon = ListElement->Lon;
					Poly.Elev = ListElement->Elev;
					Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2] + Vtx[3].ScrnXYZ[2]) * 0.25; // Optimized out division. Was / 4.0
					Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q + Vtx[3].Q) * 0.25; // Optimized out division. Was / 4.0
					Poly.Slope = 90.0;

					Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
					Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

					// compute surface normal - it is same for both triangles
					FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
					FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
					SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

					/*
					// test visibility and reverse normal if necessary
					ZeroPoint3d(Poly.ViewVec);
					// find view vector
					for (Ct = 0; Ct < 4; Ct ++)
						{
						AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
						} // for
					Poly.ViewVec[0] *= (1.0 / 4.0); // inverse multiply!
					Poly.ViewVec[1] *= (1.0 / 4.0); // inverse multiply!
					Poly.ViewVec[2] *= (1.0 / 4.0); // inverse multiply!

					Poly.ViewVec[0] -= VC->VCamera->CamPos->XYZ[0]; 
					Poly.ViewVec[1] -= VC->VCamera->CamPos->XYZ[1]; 
					Poly.ViewVec[2] -= VC->VCamera->CamPos->XYZ[2]; 
					
					// this seems to make bad normals as far as GL is concerned. I don't get it. -CXH
					if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
						{
						NegateVector(Poly.Normal);
						} // if reverse normals
					*/

					TempVert.XYZ[0] = Poly.Normal[0];
					TempVert.XYZ[1] = Poly.Normal[1];
					TempVert.XYZ[2] = Poly.Normal[2];
					TempVert.RotateY(-Poly.Lon);
					TempVert.RotateX(90.0 - Poly.Lat);
					Poly.Aspect = TempVert.FindRoughAngleYfromZ();
					if (Poly.Aspect < 0.0)
						Poly.Aspect += 360.0;

					Poly.Beach = &ListElement->Fnce->SpanMat;
					Poly.Fnce = ListElement->Fnce;
					Poly.FenceType = WCS_FENCEPIECE_SPAN;

					VtxPtr[0] = &Vtx[0];
					VtxPtr[1] = &Vtx[2];
					VtxPtr[2] = &Vtx[1];
					for (Ct = 0; Ct < 2; Ct ++)
						{
						ViewWin->fglBegin(GL_POLYGON);
						// do some ambient/specularity/transparency too
						TwinMaterials TwoMat;
						Poly.Fnce->SpanMat.GetRenderMaterial(&TwoMat, 0.0); // get leftmost material
						Col[0] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(0);
						Col[1] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(1);
						Col[2] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(2);
						Col[3] = 1.0f;
						SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
						SpecCol[3] = 1.0f;
						SpecEx = 1.0f;
						LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
						LumCol[3] = 1.0f;
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Col);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbCol);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, LumCol);
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecCol);
						glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS , SpecEx);
						if (Ct)
							{
							ViewWin->fglNormal3d(Poly.Normal[0], Poly.Normal[1], -Poly.Normal[2]); 
							ViewWin->fglNormal3d(Poly.Normal[0], Poly.Normal[1], -Poly.Normal[2]); 
							ViewWin->fglNormal3d(Poly.Normal[0], Poly.Normal[1], -Poly.Normal[2]);
							} // if
						else
							{
							ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]); 
							ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]); 
							ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]);
							} // else

						// only get here if non-Planimetric
						ViewWin->fglVertex3d(VtxPtr[0]->XYZ[0] - WallPerspOri[0], VtxPtr[0]->XYZ[1] - WallPerspOri[1], -(VtxPtr[0]->XYZ[2] - WallPerspOri[2])); 
						ViewWin->fglVertex3d(VtxPtr[1]->XYZ[0] - WallPerspOri[0], VtxPtr[1]->XYZ[1] - WallPerspOri[1], -(VtxPtr[1]->XYZ[2] - WallPerspOri[2])); 
						ViewWin->fglVertex3d(VtxPtr[2]->XYZ[0] - WallPerspOri[0], VtxPtr[2]->XYZ[1] - WallPerspOri[1], -(VtxPtr[2]->XYZ[2] - WallPerspOri[2])); 

						ViewWin->fglEnd();

						VtxPtr[0] = &Vtx[2];
						VtxPtr[1] = &Vtx[1];
						VtxPtr[2] = &Vtx[3];
						} // for
					} // if
				break;
				} // span
			#ifndef WCS_FENCE_LIMITED
			case WCS_FENCEPIECE_ROOF:
				{
				PLinkPrev = NULL;
				for (NumRoofPts = 0, PLink = ListElement->Vec->GetFirstRealPoint(); PLink; PLink = PLink->Next)
					{
					if (PLinkPrev && PLink->SamePoint(PLinkPrev))
						continue;
					if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
						continue;
					NumRoofPts ++;
					PLinkPrev = PLink;
					} // for
				//NumRoofPts = ListElement->Vec->GetNumRealPoints();
				if (NumRoofPts >= 3 && (RoofPts = (VectorPoint **)AppMem_Alloc(NumRoofPts * sizeof (VectorPoint *), APPMEM_CLEAR)))
					{
					// find out if vector is clockwise or countercw
					Area = ListElement->Vec->ComputeAreaDegrees();
					Clockwise = (Area >= 0.0);
					// build list of vector points
					PLinkPrev = NULL;
					if (Clockwise)
						{
						for (Ct = 0, PLink = ListElement->Vec->GetFirstRealPoint(); Ct < NumRoofPts && PLink; PLink = PLink->Next)
							{
							if (PLinkPrev && PLink->SamePoint(PLinkPrev))
								continue;
							if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
								continue;
							RoofPts[Ct ++] = PLink;
							PLinkPrev = PLink;
							} // for
						} // if
					else
						{
						for (Ct = NumRoofPts - 1, PLink = ListElement->Vec->GetFirstRealPoint(); Ct >= 0 && PLink; PLink = PLink->Next)
							{
							if (PLinkPrev && PLink->SamePoint(PLinkPrev))
								continue;
							if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
								continue;
							RoofPts[Ct --] = PLink;
							PLinkPrev = PLink;
							} // for
						} // else
					Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
						WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);

					RoofElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].CurValue;
					#ifdef WCS_THEMATIC_MAP
					if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV) &&
						//ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)->VectorIsPointData &&
						ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)->Eval(ThemeValue, ListElement->Vec))
						RoofElev = ThemeValue[0];
					#endif // WCS_THEMATIC_MAP

					NumToTest = NumRoofPts - 2;
					Illegal = 0;
					for (FirstCt = 0; Illegal < NumToTest && NumToTest > 0; )
						{
						BreakToOuterLoop = 0;
						if (RoofPts[FirstCt])
							{
							for (NextCt = FirstCt + 1 < NumRoofPts ? FirstCt + 1: 0; NextCt != FirstCt && ! BreakToOuterLoop; )
								{
								if (RoofPts[NextCt])
									{
									for (NextNextCt = NextCt + 1 < NumRoofPts ? NextCt + 1: 0; NextNextCt != FirstCt && NextNextCt != NextCt; )
										{
										if (RoofPts[NextNextCt])
											{
											Legal = 1;
											// test to see if it is a legal triangle
											// a roof triangle is legal and OK to draw if it fills clockwise space and
											// if no other point causes the triangle to be partly outside the roof outline
											// Is it clockwise?
											RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
											RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
											RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);
											Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
											Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
											Vtx[2].XYZ[0] = -(Vtx[2].Lon - Vtx[0].Lon);
											Vtx[2].XYZ[2] = Vtx[2].Lat - Vtx[0].Lat;
											Angle = Vtx[1].FindAngleYfromZ();
											Vtx[2].RotateY(-Angle);
											if (Vtx[2].XYZ[0] >= 0.0)
												{
												// rotate back to original position
												Vtx[2].RotateY(Angle);
												Angle = Vtx[2].FindAngleYfromZ();
												Vtx[2].RotateY(-Angle);
												for (TestCt = NextNextCt + 1 < NumRoofPts ? NextNextCt + 1: 0; TestCt != FirstCt && TestCt != NextCt && TestCt != NextNextCt; )
													{
													if (RoofPts[TestCt])
														{
														RoofPts[TestCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
														Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
														Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
														Vtx[1].RotateY(-Angle);
														if (Vtx[1].XYZ[2] > 0.0 && Vtx[1].XYZ[2] < Vtx[2].XYZ[2] && Vtx[1].XYZ[0] < 0.0)
															{
															Legal = 0;
															break;
															} // if
														} // if
													TestCt ++;
													if (TestCt >= NumRoofPts)
														TestCt = 0;
													} // for
												} // if clockwise triangle
											else
												Legal = 0;

											// if legal then fill it
											if (Legal)
												{
												RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
												RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
												RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);

												if (ListElement->Fnce->Absolute == WCS_EFFECT_RELATIVETOJOE)
													{
													Vtx[0].Elev = RoofElev + 
														CamSetup->ElevDatum + (Vtx[0].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
													Vtx[1].Elev = RoofElev + 
														CamSetup->ElevDatum + (Vtx[1].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
													Vtx[2].Elev = RoofElev + 
														CamSetup->ElevDatum + (Vtx[2].Elev - CamSetup->ElevDatum) * CamSetup->Exageration;
													} // if
												else if (ListElement->Fnce->Absolute == WCS_EFFECT_ABSOLUTE)
													{
													Vtx[0].Elev = Vtx[1].Elev = Vtx[2].Elev = RoofElev;
													} // else if
												else
													{
													CamSetup->Interactive->VertexDataPoint(CamSetup, &Vtx[0], &Poly, Flags);
													CamSetup->Interactive->VertexDataPoint(CamSetup, &Vtx[1], &Poly, Flags);
													CamSetup->Interactive->VertexDataPoint(CamSetup, &Vtx[2], &Poly, Flags);
													Vtx[0].Elev += RoofElev;
													Vtx[1].Elev += RoofElev;
													Vtx[2].Elev += RoofElev;
													} // else

												for (Ct = 0; Ct < 3; Ct ++)
													{
													// convert degrees to cartesian and project to screen
													VC->VCamera->ProjectVertexDEM(DefCoords, &Vtx[Ct], CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 1);
													} // for

												Poly.Vector = ListElement->Vec;
												Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS;
												Poly.Lat = ListElement->Lat;
												Poly.Lon = ListElement->Lon;
												Poly.Elev = ListElement->Elev;
												Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2]) * (1.0 / 3.0);
												Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q) * (1.0 / 3.0);
												Poly.Slope = 0.0;

												Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
												Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

												// compute surface normal - it is same for both triangles
												FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
												FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
												SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

												// test visibility and reverse normal if necessary
												ZeroPoint3d(Poly.ViewVec);
												// find view vector
												for (Ct = 0; Ct < 3; Ct ++)
													{
													AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
													} // for
												Poly.ViewVec[0] *= (1.0 / 3.0); // inverse multiply!
												Poly.ViewVec[1] *= (1.0 / 3.0); // inverse multiply!
												Poly.ViewVec[2] *= (1.0 / 3.0); // inverse multiply!

												Poly.ViewVec[0] -= VC->VCamera->CamPos->XYZ[0]; 
												Poly.ViewVec[1] -= VC->VCamera->CamPos->XYZ[1]; 
												Poly.ViewVec[2] -= VC->VCamera->CamPos->XYZ[2]; 
												if (!SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
													{
													NegateVector(Poly.Normal);
													} // if reverse normals

												Poly.Beach = ListElement->Fnce->SeparateRoofMat ? &ListElement->Fnce->RoofMat: &ListElement->Fnce->SpanMat;
												Poly.Fnce = ListElement->Fnce;
												Poly.FenceType = WCS_FENCEPIECE_ROOF;

												VtxPtr[0] = &Vtx[0];
												VtxPtr[1] = &Vtx[2];
												VtxPtr[2] = &Vtx[1];

												ViewWin->fglBegin(GL_POLYGON);
												// do some ambient/specularity/transparency too
												TwinMaterials TwoMat;
												if (ListElement->Fnce->SeparateRoofMat)
													{
													Poly.Fnce->RoofMat.GetRenderMaterial(&TwoMat, 0.0); // get leftmost ROOF material
													} // if
												else
													{
													Poly.Fnce->SpanMat.GetRenderMaterial(&TwoMat, 0.0); // get leftmost SPAN/PANEL material
													} // else
												Col[0] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(0);
												Col[1] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(1);
												Col[2] = (float)TwoMat.Mat[0]->DiffuseColor.GetClampedCompleteValue(2);
												Col[3] = 1.0f;
												SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
												SpecCol[3] = 1.0f;
												SpecEx = 1.0f;
												LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
												LumCol[3] = 1.0f;
												ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Col);
												ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbCol);
												ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, LumCol);
												ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecCol);
												glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS , SpecEx);
												ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]); 
												ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]); 
												ViewWin->fglNormal3d(-Poly.Normal[0], -Poly.Normal[1], Poly.Normal[2]);
												if (VC->IsPlan())
													{
													#ifdef WCS_BUILD_VNS
													if (IsProj)
														{
														VertexDEM ProjWallVertex;

														ProjWallVertex.Lon  = VtxPtr[0]->Lon;
														ProjWallVertex.Lat  = VtxPtr[0]->Lat;
														ProjWallVertex.Elev = VtxPtr[0]->Elev;
														ProjectForViews(VC, &ProjWallVertex);
														ViewWin->fglVertex3d(ProjWallVertex.XYZ[0], ProjWallVertex.XYZ[1], VtxPtr[0]->Elev); 

														ProjWallVertex.Lon  = VtxPtr[1]->Lon;
														ProjWallVertex.Lat  = VtxPtr[1]->Lat;
														ProjWallVertex.Elev = VtxPtr[1]->Elev;
														ProjectForViews(VC, &ProjWallVertex);
														ViewWin->fglVertex3d(ProjWallVertex.XYZ[0], ProjWallVertex.XYZ[1], VtxPtr[1]->Elev); 

														ProjWallVertex.Lon  = VtxPtr[2]->Lon;
														ProjWallVertex.Lat  = VtxPtr[2]->Lat;
														ProjWallVertex.Elev = VtxPtr[2]->Elev;
														ProjectForViews(VC, &ProjWallVertex);
														ViewWin->fglVertex3d(ProjWallVertex.XYZ[0], ProjWallVertex.XYZ[1], VtxPtr[2]->Elev); 
														} // if
													else
													#endif // WCS_BUILD_VNS
														{
														ViewWin->fglVertex3d(VtxPtr[0]->Lon, VtxPtr[0]->Lat, VtxPtr[0]->Elev); 
														ViewWin->fglVertex3d(VtxPtr[1]->Lon, VtxPtr[1]->Lat, VtxPtr[1]->Elev); 
														ViewWin->fglVertex3d(VtxPtr[2]->Lon, VtxPtr[2]->Lat, VtxPtr[2]->Elev); 
														} // else
													} // if
												else
													{
													ViewWin->fglVertex3d(VtxPtr[0]->XYZ[0] - WallPerspOri[0], VtxPtr[0]->XYZ[1] - WallPerspOri[1], -(VtxPtr[0]->XYZ[2] - WallPerspOri[2])); 
													ViewWin->fglVertex3d(VtxPtr[1]->XYZ[0] - WallPerspOri[0], VtxPtr[1]->XYZ[1] - WallPerspOri[1], -(VtxPtr[1]->XYZ[2] - WallPerspOri[2])); 
													ViewWin->fglVertex3d(VtxPtr[2]->XYZ[0] - WallPerspOri[0], VtxPtr[2]->XYZ[1] - WallPerspOri[1], -(VtxPtr[2]->XYZ[2] - WallPerspOri[2])); 
													} //else
												ViewWin->fglEnd();


												RoofPts[NextCt] = NULL;
												NumToTest --;
												Illegal = 0;
												BreakToOuterLoop = 1;
												break;
												} // if
											else
												{
												Illegal ++;
												BreakToOuterLoop = 1;
												break;
												} // else
											} // if
										NextNextCt ++;
										if (NextNextCt >= NumRoofPts)
											NextNextCt = 0;
										} // for
									} // if
								NextCt ++;
								if (NextCt >= NumRoofPts)
									NextCt = 0;
								} // for
							} // if
						FirstCt ++; 
						if (FirstCt >= NumRoofPts)
							FirstCt = 0;
						} // for
					AppMem_Free(RoofPts, NumRoofPts * sizeof (VectorPoint *));
					} // if list
				break;
				} // roof
			#endif // WCS_FENCE_LIMITED
			} // switch
		} // for
	glEndList();

	// dispose of the FenceVertexList
	GlobalApp->AppEffects->FnceBase.Destroy();

	// dispose of RenderJoeList
	while (RJL)
		{
		CurJL = (RenderJoeList *)RJL->Next;
		delete RJL;
		RJL = CurJL;
		} // while
	} // if

EndWall:

if (!VC->IsPlan())
	{
	glPopMatrix();
	} // if

//ViewWin->fglDisable(GL_COLOR_MATERIAL);

if (SwitchBack)
	{
	glEnable(GL_CULL_FACE);
	} // if

#endif // WCS_VIEW_DRAWWALLS

} // ViewGUI::DrawWalls

/*===========================================================================*/

// Move to ViewGUI object scope when ready for big recompile
RealtimeFoliageIndex RTFIndex;

static unsigned char GLTextureXfer[128 * 128 * 4]; // 128x128 RGBA

void ViewGUI::DrawFoliage(int ViewNum, ViewContext *VC, double Simplification, int FolType)
{
Point3d RefPt, PointA, PointB, PointC, PointD;
Point4f Col, Black, ReplColor, White;
long Ct, Ct1, poly, p0, p1, p2, NumPolys, NumVerts, DrawDetail;
unsigned long Nth, LoopNth;
FoliageEffect *Verdure;
Joe *Current = NULL;
VectorPoint *Point = NULL;
Polygon3D *Poly, *Polygons;
Vertex3D *Vertices;
MaterialEffect *CurMat;
GLDrawingFenetre *ViewWin = NULL;
VertexDEM ObjectLocation;
PolygonData PolyDat;
char SwitchBack = 0, ForceBox = 0, IsEnabled;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Atmosphere *AS;
Point4f AmbCol, SpecCol, LumCol;
float SpecEx, Intensity;
FoliagePreviewData PointData;
float ModelViewMatrix[16];
int LoopGood;
RasterAnimHost *TheActive;
RealtimeFoliageData *RTFD = NULL;
int RotateToFace = 1;
double MinDistSq, MaxDistSq;
double MinHt, MaxHt;

// this doesn't seem to work in plan views yet, so save the effort and bail quickly
//if (VC->IsPlan()) return; 

if (FolType && !RTFLoaded)
	{
	return; // nothing to do
	} // if

if (!((VC->GetEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX) && VC->GetEnabled(WCS_VIEWGUI_ENABLE_RTFOLIAGE)))) return;

MinHt = RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].GetCurValue();
MaxHt = RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].GetCurValue();
MinDistSq = RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].GetCurValue();
MinDistSq *= MinDistSq;
MaxDistSq = RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_FARDIST].GetCurValue();
MaxDistSq *= MaxDistSq;

AmbCol[0] = AmbCol[1] = AmbCol[2] = 0.0f;
AmbCol[3] = 1.0f;
SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
SpecCol[3] = 1.0f;
LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
LumCol[3] = 1.0f;

White[0] = 
White[1] = 
White[2] = 
White[3] = 1.0f;

// Ambient is in atmospheres
for (AS = (Atmosphere *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE));
 AS; AS = (Atmosphere *)(AS->Next))
	{
	if (AS->Enabled)
		{
		Intensity = (float)AS->TopAmbientColor.GetIntensity();
		AmbCol[0] += Intensity * (float)AS->TopAmbientColor.GetCurValue(0);
		AmbCol[1] += Intensity * (float)AS->TopAmbientColor.GetCurValue(1);
		AmbCol[2] += Intensity * (float)AS->TopAmbientColor.GetCurValue(2);
		} // if
	} // if

ViewWin = ViewWins[ViewNum];

if (Simplification < 0.0)
	{
	ForceBox = 1;
	Nth = 1;
	} // if
else if ((Simplification == 0.0) || (Simplification >= 1.0))
	{
	Nth = 1;
	} // if
else
	{
	Nth = (unsigned long int)(1.0 / Simplification);
	} // else

#ifdef WCS_VIEWGUI_STAGGER_POLYS
StaggerNth++;
#endif // WCS_VIEWGUI_STAGGER_POLYS

if (StaggerNth > (Nth - 1)) StaggerNth = 0;

CopyPoint3d(RefPt, VC->NegOrigin);
NegateVector(RefPt);

// init 3d object effects and foliage effects
for (PointData.Object3D = (Object3DEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D));
 PointData.Object3D; PointData.Object3D = (Object3DEffect *)(PointData.Object3D->Next))
	{
	if (PointData.Object3D->Enabled)
		{
		PointData.Object3D->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // if
	} // for
// build foliage chains
for (Verdure = (FoliageEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_FOLIAGE));
 Verdure; Verdure = (FoliageEffect *)(Verdure->Next))
	{
	if (Verdure->Enabled && Verdure->PreviewEnabled)
		{
		Verdure->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // if
	} // for

Black[0] = 
Black[1] = 
Black[2] = 0.0f;
Black[3] = 1.0f;

glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Black);

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Black);

if (glIsEnabled(GL_CULL_FACE))
	{
	glDisable(GL_CULL_FACE);
	SwitchBack = 1;
	} // if

Col[3] = 1.0f;

LoopGood = 0;
if (FolType)
	{
	if (RTFD = RTFIndex.FirstWalk())
		LoopGood = 1;
	} // if
else
	{
	// loop through the initialized foliage objects
	if (Verdure = (FoliageEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_FOLIAGE, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon))
		LoopGood = 1;
	} // else

while (LoopGood)
	{
	LoopGood = 0;
	IsEnabled = 0;
	TheActive = NULL;
	if (FolType)
		{
		RTFD->InterpretFoliageRecord(GlobalApp->AppEffects, GlobalApp->AppImages, &PointData);
		IsEnabled = 1;
		// We'll do filtering by height here as these criteria only apply to RTF files
		if (PointData.Height > MaxHt || PointData.Height < MinHt)
			{
			// suppress display
			IsEnabled = 0;
			} // if
		} // if
	else
		{
		IsEnabled = (char)Verdure->Enabled;
		} // else
	if (RasterAnimHost::GetActiveRAHost())
		{
		if (FolType)
			{
			// only can show 3d objects as active
			if ((PointData.Object3D == RasterAnimHost::GetActiveRAHost()))
				TheActive = PointData.Object3D;
			} // if
		else
			{
			if ((Verdure == RasterAnimHost::GetActiveRAHost()))
				TheActive = Verdure;
			} // else
		} // if
	if (TheActive)
		{
		DrawSetupActiveObject(VC);
		} // if
	NumPolys = NumVerts = DrawDetail = 0;

	if (FolType)
		{
		double DistSq, DeltaX, DeltaY, DeltaZ;
		// need to calculate World Coords now to do most efficient filtering by distance
		// Add back in Cell origin/offset to reaquire world coords
		ObjectLocation.XYZ[0] = (double)(RTFD->XYZ[0]) + (double)(RTFIndex.RefXYZ[0]);
		ObjectLocation.XYZ[1] = (double)(RTFD->XYZ[1]) + (double)(RTFIndex.RefXYZ[1]);
		ObjectLocation.XYZ[2] = (double)(RTFD->XYZ[2]) + (double)(RTFIndex.RefXYZ[2]);
		// We're going to have to unproject that XYX into Lat/Lon for glStandUpright() below.
		#ifdef WCS_BUILD_VNS
		DefCoords->CartToDeg(&ObjectLocation);
		#else // WCS_BUILD_VNS
		ObjectLocation.CartToDeg(VC->GetRadius());
		#endif // WCS_BUILD_VNS
		// Height filter	
		DeltaX = ObjectLocation.XYZ[0] - VC->CamPos.XYZ[0];
		DeltaY = ObjectLocation.XYZ[1] - VC->CamPos.XYZ[1];
		DeltaZ = ObjectLocation.XYZ[2] - VC->CamPos.XYZ[2];
		DistSq = (DeltaX * DeltaX) + (DeltaY * DeltaY) + (DeltaZ * DeltaZ);
		if (DistSq < MinDistSq || DistSq > MaxDistSq)
			{
			IsEnabled = 0;
			} // if
		} // if

	if (IsEnabled)
		{
		SetupPickName(TheActive);
		if (!FolType)
			{
			PointData.CurRast = NULL;
			PointData.Object3D = NULL;
			Verdure->FindBasePosition(CamSetup, &ObjectLocation, &PolyDat, Current, Point);
			#ifdef WCS_FORESTRY_WIZARD
			Verdure->SelectForestryImageOrObject(CamSetup, &PolyDat, &PointData);
			#endif // WCS_FORESTRY_WIZARD
			} // if
		// if FolType is 0 we ignore the RTFDispConf.Display
		if (PointData.Object3D && (!FolType || RTFDispConf.Display3DO))
			{
			if (FolType)
				{
				// World Coordinates are now calculated in advance for RTF Files
				} // if
			if ((ForceBox) || (PointData.Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_CUBE))
				{
				glDisable(GL_CULL_FACE);
				if (Vertices = new Vertex3D[8])
					{
					NumVerts = 8;
					if (Polygons = new Polygon3D[6])
						{
						NumPolys = 6;
						for (Ct = 0; Ct < 8; Ct ++)
							{
							Vertices[Ct].PolyRef = (long *)AppMem_Alloc(3 * sizeof (long), 0);
							Vertices[Ct].NumPolys = 3;
							for (Ct1 = 0; Ct1 < 3; Ct1 ++)
								{
								Vertices[Ct].xyz[Ct1] = PointData.Object3D->ObjectBounds[VGCubeCornerRef[Ct][Ct1]];
								} // for
							for (Ct1 = 0; Ct1 < 3; Ct1 ++)
								{
								Vertices[Ct].PolyRef[Ct1] = VGCubePolyRef[Ct][Ct1];
								} // for
							} // for
						for (Ct = 0; Ct < 6; Ct ++)
							{
							Polygons[Ct].VertRef = (long *)AppMem_Alloc(4 * sizeof (long), 0);
							Polygons[Ct].NumVerts = 4;
							for (Ct1 = 0; Ct1 < 4; Ct1 ++)
								{
								Polygons[Ct].VertRef[Ct1] = VGCubeVertRef[Ct][Ct1];
								} // for
							} // for
						if (PointData.Object3D->NameTable && PointData.Object3D->NumMaterials > 0)
							{
							if (! PointData.Object3D->NameTable[0].Mat && PointData.Object3D->NameTable[0].Name[0])
								GlobalApp->AppEffects->SetMakeMaterial(&PointData.Object3D->NameTable[0], NULL);
							if (CurMat = PointData.Object3D->NameTable[0].Mat)
								{
								//Colr = CurMat->Color;
								Col[0] = (float)CurMat->DiffuseColor.GetCurValue(0);
								Col[1] = (float)CurMat->DiffuseColor.GetCurValue(1);
								Col[2] = (float)CurMat->DiffuseColor.GetCurValue(2);

								// WCS has monostimulus specularity/luminosity, GL has tristimulus specularity/luminosity
								SpecCol[0] = SpecCol[1] = SpecCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
								SpecCol[3] = 1.0f;
								SpecEx = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);

								LumCol[0] = LumCol[1] = LumCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0);
								LumCol[3] = 1.0f;
								} // if
							else
								{
								Col[0] = Col[1] = Col[2] = .5f;
								SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
								SpecCol[3] = 1.0f;
								SpecEx = 1.0f;
								LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
								LumCol[3] = 1.0f;
								} // else
							} // if
						else
							{
							Col[0] = Col[1] = Col[2] = .5f;
							SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
							SpecCol[3] = 1.0f;
							SpecEx = 1.0f;
							LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
							LumCol[3] = 1.0f;
							} // else
						} // if polygons
					} // if vertices
				LoopNth = 1; // draw all cube faces w/o decimation
				} // if draw cube
			else if ((PointData.Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_DETAIL) && ((PointData.Object3D->Vertices && PointData.Object3D->Polygons && PointData.Object3D->NameTable) || PointData.Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
				{
				NumPolys = PointData.Object3D->NumPolys;
				NumVerts = PointData.Object3D->NumVertices;
				Vertices = PointData.Object3D->Vertices;
				Polygons = PointData.Object3D->Polygons;
				DrawDetail = 1;
				LoopNth = Nth;
				} // else if draw detail

			if (NumPolys > 0 && NumVerts > 0)
				{
				VC->VCamera->ProjectVertexDEM(DefCoords, &ObjectLocation, CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, FolType ? 0: 1);
				if (PointData.Object3D->Transform(Vertices, NumVerts, CamSetup, &PolyDat, &ObjectLocation, PointData.Rotate, PointData.Height))
					{
					// project all the object vertices
					if (VC->IsPlan())
						{
						for (Ct = 0; Ct < NumVerts; Ct ++)
							{
							#ifdef WCS_BUILD_VNS
							DefCoords->CartToDeg(&Vertices[Ct]);
							#else // WCS_BUILD_VNS
							Vertices[Ct].CartToDeg(CamSetup->PlanetRad);
							#endif // WCS_BUILD_VNS
							VC->VCamera->ProjectVertexDEM(DefCoords, &Vertices[Ct], CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 0);
							} // for
						} // if
					for (Ct = 0; Ct < NumPolys; Ct ++)
						Polygons[Ct].Normalized = 0;
					if (LoopNth == 1)
						{
						Ct = 0;
						} // if
					else
						{
						Ct = (StaggerNth % LoopNth);
						} // else
					for (; Ct < NumPolys; Ct += LoopNth)
						{
						Poly = &Polygons[Ct];
						if (DrawDetail)
							{
							if (PointData.Object3D->NameTable[Poly->Material].Mat ||
								(PointData.Object3D->NameTable[Poly->Material].Name[0] && (PointData.Object3D->NameTable[Poly->Material].Mat = GlobalApp->AppEffects->SetMakeMaterial(&PointData.Object3D->NameTable[Poly->Material], NULL))))
								{
								CurMat = PointData.Object3D->NameTable[Poly->Material].Mat;
								if (CurMat->Shading == WCS_EFFECT_MATERIAL_SHADING_INVISIBLE || ! CurMat->Enabled)
									continue;
								// compute surface and vertex normals
								Poly->Normalize(Vertices);
								if (CurMat->FlipNormal)
									NegateVector(Poly->Normal);
								if (CurMat->Shading == WCS_EFFECT_MATERIAL_SHADING_PHONG)
									{
									for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
										{
										Vertices[Poly->VertRef[p0]].Normalized = 0;
										Vertices[Poly->VertRef[p0]].Normalize(Polygons, Vertices, Poly, PointData.Object3D->NameTable);
										} // for
									} // if gouraud or phong
								else
									{
									for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
										{
										Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
										Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
										Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
										} // for
									} // else
								Col[0] = (float)CurMat->DiffuseColor.GetCurValue(0);
								Col[1] = (float)CurMat->DiffuseColor.GetCurValue(1);
								Col[2] = (float)CurMat->DiffuseColor.GetCurValue(2);

								// WCS has monostimulus specularity/luminosity, GL has tristimulus specularity/luminosity
								SpecCol[0] = SpecCol[1] = SpecCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
								SpecCol[3] = 1.0f;
								SpecEx = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);

								LumCol[0] = LumCol[1] = LumCol[2] = (float)CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0);
								LumCol[3] = 1.0f;
								} // if
							else
								{
								for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
									{
									Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
									Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
									Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
									} // for
								Col[0] = (float).5;
								Col[1] = (float).5;
								Col[2] = (float).5;
								SpecCol[0] = SpecCol[1] = SpecCol[2] = 0.0f;
								SpecCol[3] = 1.0f;
								SpecEx = 1.0f;
								LumCol[0] = LumCol[1] = LumCol[2] = 0.0f;
								LumCol[3] = 1.0f;
								} // else
							} // if
						else
							{
							// compute surface and vertex normals
							Poly->Normalize(Vertices);
							for (p0 = 0; p0 < Poly->NumVerts; p0 ++)
								{
								Vertices[Poly->VertRef[p0]].Normal[0] = Poly->Normal[0];
								Vertices[Poly->VertRef[p0]].Normal[1] = Poly->Normal[1];
								Vertices[Poly->VertRef[p0]].Normal[2] = Poly->Normal[2];
								} // for
							} // else

						p0 = 0;
						p1 = 1;
						p2 = Poly->NumVerts - 1;
						poly = 0;
						while (p2 > p1)
							{
							ViewWin->fglBegin(GL_POLYGON);
							// do some ambient/specularity/transparency too
							ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Col);
							ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbCol);
							ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, LumCol);
							ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecCol);
							glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS , SpecEx);
							ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p0]].Normal[0],
								-Vertices[Poly->VertRef[p0]].Normal[1], Vertices[Poly->VertRef[p0]].Normal[2]); 
							ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p1]].Normal[0],
								-Vertices[Poly->VertRef[p1]].Normal[1], Vertices[Poly->VertRef[p1]].Normal[2]); 
							ViewWin->fglNormal3d(-Vertices[Poly->VertRef[p2]].Normal[0],
								-Vertices[Poly->VertRef[p2]].Normal[1], Vertices[Poly->VertRef[p2]].Normal[2]); 
							if (VC->IsPlan())
								{
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p0]].Lon,
									Vertices[Poly->VertRef[p0]].Lat, Vertices[Poly->VertRef[p0]].Elev); 
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p1]].Lon,
									Vertices[Poly->VertRef[p1]].Lat, Vertices[Poly->VertRef[p1]].Elev); 
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p2]].Lon,
									Vertices[Poly->VertRef[p2]].Lat, Vertices[Poly->VertRef[p2]].Elev); 
								} // if plan view
							else
								{
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p0]].XYZ[0] - RefPt[0],
									Vertices[Poly->VertRef[p0]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p0]].XYZ[2] - RefPt[2])); 
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p1]].XYZ[0] - RefPt[0],
									Vertices[Poly->VertRef[p1]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p1]].XYZ[2] - RefPt[2])); 
								ViewWin->fglVertex3d(Vertices[Poly->VertRef[p2]].XYZ[0] - RefPt[0],
									Vertices[Poly->VertRef[p2]].XYZ[1] - RefPt[1], -(Vertices[Poly->VertRef[p2]].XYZ[2] - RefPt[2])); 
								} // else
							ViewWin->fglEnd();

							if (poly %2)
								{
								p0 = p2;
								p2 --;
								} // if
							else
								{
								p0 = p1;
								p1 ++;
								} // else
							poly ++;
							} // while p2 > p1
						} // for each polygon
					} // if Transform - returns 0 if no vertices
				} // if object loaded
			if (PointData.Object3D->DrawEnabled == WCS_EFFECTS_OBJECT3D_DRAW_CUBE)
				{
				if (Polygons)
					delete [] Polygons;
				Polygons = NULL;
				if (Vertices)
					delete [] Vertices;
				Vertices = NULL;
				} // if
			} // if 3d object
		// if FolType is 0 we ignore the RTFDispConf.Display
		// be sure to ignore Object3D entities that might have otherwise failed above test
		if (!PointData.Object3D && PointData.CurRast && (!FolType || RTFDispConf.DisplayImage))
			{
			if (FolType && (PointData.ColorImageOpacity < 1.0))
				{
				ReplColor[0] = (float)PointData.RGB[0];
				ReplColor[1] = (float)PointData.RGB[1];
				ReplColor[2] = (float)PointData.RGB[2];
				ReplColor[3] = 1.0f;
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ReplColor); // replace color
				} // if
			else
				{
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, White); // max range for texture
				} // else
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbCol);
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Black);
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Black);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
			// draw a foliage image object here
			// variables of interest are 
			// Raster *PointData.CurRast - largest raster in MIP Map chain for image to draw
			// double PointData.Height - height of image in meters
			// double PointData.Width - width of image in meters
			// double PointData.RGB[3] - replacement colors if needed (range is 0-1 normally but may exceed 1)
			// double PointData.ColorImageOpacity - opacity of image color, remainder is PointData.RGB color
			// char PointData.FlipX - reverse the X axis of the image if TRUE
			// PolyDat.Lat, Lon, Elev - position of base of foliage image
			// copy position to VertexDEM so you can call ProjectVertexDEM
			if (FolType)
				{
				// World coords are now calculated in advance for RTF files
				} // if
			else
				{
				ObjectLocation.Lat = PolyDat.Lat;
				ObjectLocation.Lon = PolyDat.Lon;
				ObjectLocation.Elev = PolyDat.Elev;
				VC->VCamera->ProjectVertexDEM(DefCoords, &ObjectLocation, CamSetup->EarthLatScaleMeters, CamSetup->PlanetRad, 1);
				} // else
			// ObjectLocation now has valid XYZ global position as well as screen coords including Z distance if you need them

			glPushMatrix(); // on MODELVIEW
			AddPoint3d(ObjectLocation.XYZ, VC->NegOrigin);
			InvertZ(ObjectLocation.XYZ);
			glTranslated(ObjectLocation.XYZ[0], ObjectLocation.XYZ[1], ObjectLocation.XYZ[2]); // on ModelView
			if (!VC->IsPlan())
				{
				glStandUpright(ObjectLocation.Lat, ObjectLocation.Lon);
				} // if
			// rotate billboard to face camera
			if (RotateToFace)
				{
				calcMatrix(ModelViewMatrix);
				glMultMatrixf(ModelViewMatrix);
				} // if

			// Foliage textures will be displaylisted
			if (PointData.CurRast && PointData.CurRast->Thumb && PointData.CurRast->Thumb->TNailsValid())
				{
				if (PointData.CurRast->ThumbDisplayListNum == ULONG_MAX)
					{ // need to make texture displaylist
					unsigned long GLTexX, GLTexY, ThumbX, ThumbY, GLTexInc, ThumbWidth, ThumbHeight, TNIdx;
					float ThumbXf, ThumbYf;
					unsigned char *ThumbRed, *ThumbGrn, *ThumbBlu;
					int ListGenInProgress = 0;
					PointData.CurRast->ThumbDisplayListNum = glGenLists(1);
					if (PointData.CurRast->ThumbDisplayListNum == 0)
						{
						PointData.CurRast->ThumbDisplayListNum = ULONG_MAX; // don't displaylist, run in immediate
						} // if
					else
						{
						glNewList(PointData.CurRast->ThumbDisplayListNum, GL_COMPILE_AND_EXECUTE); // run commands as we generate
						ListGenInProgress = 1;
						} // else
					ThumbRed = PointData.CurRast->Thumb->TNail[0];
					ThumbGrn = PointData.CurRast->Thumb->TNail[1];
					ThumbBlu = PointData.CurRast->Thumb->TNail[2];
					ThumbWidth  = 100 - (PointData.CurRast->Thumb->TNailPadX + PointData.CurRast->Thumb->TNailPadX);
					ThumbHeight = 100 - (PointData.CurRast->Thumb->TNailPadY + PointData.CurRast->Thumb->TNailPadY);

					GLTexInc = 0;
					for (GLTexY = 0; GLTexY < 128; GLTexY++)
						{
						for (GLTexX = 0; GLTexX < 128; GLTexX++)
							{
							ThumbXf = ((float)GLTexX / 128.0f);
							ThumbYf = ((float)GLTexY / 128.0f);
							ThumbX = (unsigned long int)(ThumbXf * ThumbWidth)  + PointData.CurRast->Thumb->TNailPadX;
							ThumbY = (unsigned long int)(ThumbYf * ThumbHeight) + PointData.CurRast->Thumb->TNailPadY;
							TNIdx = (ThumbY) * 100 + ThumbX;
							GLTextureXfer[GLTexInc++] = ThumbRed[TNIdx]; // r
							GLTextureXfer[GLTexInc++] = ThumbGrn[TNIdx]; // g
							GLTextureXfer[GLTexInc++] = ThumbBlu[TNIdx]; // b
							GLTextureXfer[GLTexInc++] = (ThumbRed[TNIdx] + ThumbGrn[TNIdx] + ThumbBlu[TNIdx] == 0) ? 0 : 255; // alpha (1 or 0)
							
							//GLTextureXfer[GLTexInc++] = (unsigned char)(ThumbXf * 255.0f); // red
							//GLTextureXfer[GLTexInc++] = (unsigned char)(ThumbYf * 255.0f); // green
							//GLTextureXfer[GLTexInc++] = (unsigned char)((1.0 - ThumbXf) * 255.0f); //blue

							//GLTextureXfer[GLTexInc++] = (unsigned char)(GLTexX + GLTexY); // red
							//GLTextureXfer[GLTexInc++] = (unsigned char)(GLTexX + GLTexY); // green
							//GLTextureXfer[GLTexInc++] = (unsigned char)(GLTexX + GLTexY); // blue

							} // for
						} // for
	/*
						{
						FILE *RGBOut;
						if (RGBOut = fopen("c:\\test.rgb", "wb"))
							{
							fwrite(GLTextureXfer, 1, 128 * 128 * 4, RGBOut);
							fclose(RGBOut);
							} // fi
						}
	*/
					//gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, GLTextureXfer);
					glTexImage2D(GL_TEXTURE_2D, 0, 4, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, GLTextureXfer);
					if (ListGenInProgress)
						{
						glEndList(); // end displaylist generation
						} // if
					} // if
				else
					{ // we can use pre-created texture displaylist
					glCallList(PointData.CurRast->ThumbDisplayListNum);
					} // if
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glDisable(GL_TEXTURE_1D);
				glEnable(GL_TEXTURE_2D);
				
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);

				ViewWins[ViewNum]->fglMatrixMode(GL_TEXTURE);
				glLoadIdentity(); // on TEXTURE
				glScalef(1.0f, 1.0f, 1.0f);
				ViewWins[ViewNum]->fglMatrixMode(GL_MODELVIEW);
				} // if
			else
				{
				glDisable(GL_TEXTURE_2D);
				} // else

			glDisable(GL_CULL_FACE);
/*
			glBegin(GL_TRIANGLES);
			// Pointy tree-like tripoly
			//  A
			// B C
			PointA[0] = 0;
			PointA[1] = PointData.Height;
			PointA[2] = 0;
			PointB[0] = -(PointData.Width * 0.5);
			PointB[1] = 0;
			PointB[2] = 0;
			PointC[0] =  (PointData.Width * 0.5);
			PointC[1] = 0;
			PointC[2] = 0;
			PointD[0] =  (PointData.Width * 0.5);
			PointD[1] = 0;
			PointD[2] = 0;
			glTriPoly(PointA, PointB, PointC);
*/
			// Rectangular Quad
			// A B
			// D C
			glBegin(GL_QUADS);
			PointA[0] = -(PointData.Width * 0.5);
			PointA[1] = PointData.Height;
			PointA[2] = 0;
			PointB[0] =  (PointData.Width * 0.5);
			PointB[1] = PointData.Height;
			PointB[2] = 0;
			PointC[0] =  (PointData.Width * 0.5);
			PointC[1] = 0;
			PointC[2] = 0;
			PointD[0] = -(PointData.Width * 0.5);
			PointD[1] = 0;
			PointD[2] = 0;

			// <<<>>> can we displaylist these buggers and just use transform matrices to scale them?
			if (PointData.Shade3D)
				{
				// use this for 3D shading
				glQuadPolyUnitTexDefNorm(PointA, PointB, PointC, PointD, PointData.FlipX);
				} // if
			else
				{
				// this for no 3D shading
				glQuadPolyUnitTex(PointA, PointB, PointC, PointD, PointData.FlipX);
				} // else
			glEnd();
/*
			{
			VertexDEM TempCursor;
			TempCursor = Cursor;
			Cursor.Lat = ObjectLocation.Lat;
			Cursor.Lon = ObjectLocation.Lon;
			Cursor.Elev = ObjectLocation.Elev;

			DrawCursor(ViewNum, VC);
			Cursor = TempCursor;
			}
*/
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_CULL_FACE);
			glDisable(GL_ALPHA_TEST);
			glPopMatrix(); // on MODELVIEW

			} // else if raster
		ClearPickName();
		} // if draw enabled
	if (TheActive)
		{
		DrawFinishActiveObject(VC);
		} // if
	if (FolType)
		{
		if (RTFD = RTFIndex.NextWalk())
			LoopGood = 1;
		} // if
	else
		{
		if (Current && Point)
			{
			if (Verdure = (FoliageEffect *)LocalDB->GetNextPointEffect(WCS_EFFECTSSUBCLASS_FOLIAGE, Current, Point, ObjectLocation.Elev, ObjectLocation.Lat, ObjectLocation.Lon))
				LoopGood = 1;
			} // if
		else
			{
			Verdure = NULL;
			LoopGood = 0;
			} // else
		} // else
	} // while

if (SwitchBack)
	{
	glEnable(GL_CULL_FACE);
	} // if

} // ViewGUI::DrawFoliage

/*===========================================================================*/

int ViewGUI::PrepRealtimeFoliage(char *IndexName)
{
int FileCt = 0;

// init image IDs so they match (hopefully) the images in the files
GlobalApp->AppImages->InitRasterIDs();

FileCt = RTFIndex.LoadFoliageIndex(IndexName, NULL);
RTFIndex.LoadAllFoliage();
RTFLoaded = 1;

return(FileCt);

} // ViewGUI::PrepRealtimeFoliage

/*===========================================================================*/

void ViewGUI::FreeRealtimeFoliage(void)
{

RTFIndex.FreeAllFoliage();

} // ViewGUI::FreeRealtimeFoliage

/*===========================================================================*/

/*
// I realize this is not the way things will be done but this shows all the steps needed to do realtime foliage
// from foliage list files saved by renderer in project default directory
void ViewGUI::AllInOneRealtimeFoliage(void)
{
RealtimeFoliageIndex Index;
long FileCt, DatPt;
char FileName[512], HitTest, TestFileVersion;
double CellCtrXYZ[3], CellRadius;
FILE *ffile;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;

// init image IDs so they match (hopefully) the images in the files
GlobalApp->AppImages->InitRasterIDs();

// find and open the index file
strmfp(FileName, GlobalApp->MainProj->dirname, "FoliageIndx.dat");
if (ffile = PROJ_fopen(FileName, "rb"))
	{
	// read file descriptor, no need to keep it around unless you want to
	fgets(FileName, 256, ffile);
	// version
	fread((char *)&Index.FileVersion, sizeof (char), 1, ffile);
	// number of files
	fread((char *)&Index.NumCells, sizeof (long), 1, ffile);
	// reference XYZ
	fread((char *)&Index.RefXYZ[0], sizeof (double), 1, ffile);
	fread((char *)&Index.RefXYZ[1], sizeof (double), 1, ffile);
	fread((char *)&Index.RefXYZ[2], sizeof (double), 1, ffile);

	if (Index.NumCells > 0)
		{
		// allocate cell data
		if (Index.CellDat = new RealtimeFoliageCellData[Index.NumCells])
			{
			// for each file
			for (FileCt = 0; FileCt < Index.NumCells; FileCt ++)
				{
				// file name
				fgets(Index.CellDat[FileCt].FileName, 64, ffile);
				// center XYZ
				fread((char *)&Index.CellDat[FileCt].CellXYZ[0], sizeof (double), 1, ffile);
				fread((char *)&Index.CellDat[FileCt].CellXYZ[1], sizeof (double), 1, ffile);
				fread((char *)&Index.CellDat[FileCt].CellXYZ[2], sizeof (double), 1, ffile);
				// half cube cell dimension
				fread((char *)&Index.CellDat[FileCt].CellRad, sizeof (double), 1, ffile);
				// number of trees in file
				if (fread((char *)&Index.CellDat[FileCt].DatCt, sizeof (long), 1, ffile) != 1)
					break;
				} // for
			} // if
		} // if some cells to read
	fclose(ffile);
	} // if index opened

// determine which files are viewable with some kind of hit test, load the data and draw it
for (FileCt = 0; FileCt < Index.NumCells; FileCt ++)
	{
	// hit test a box or sphere with center at cell center in world coords and half cube dimension of CellRad
	// CellRad may be different for different files
	CellCtrXYZ[0] = Index.RefXYZ[0] + Index.CellDat[FileCt].CellXYZ[0];
	CellCtrXYZ[1] = Index.RefXYZ[1] + Index.CellDat[FileCt].CellXYZ[1];
	CellCtrXYZ[2] = Index.RefXYZ[2] + Index.CellDat[FileCt].CellXYZ[2];

	// for sphere testing
	CellRadius = sqrt(Index.CellDat[FileCt].CellRad * Index.CellDat[FileCt].CellRad * 3.0);

	// do some OGL voodoo here instead of:
	HitTest = 1;

	if (HitTest)
		{
		strmfp(FileName, GlobalApp->MainProj->dirname, Index.CellDat[FileCt].FileName);
		if (ffile = PROJ_fopen(FileName, "rb"))
			{
			fgets(FileName, 64, ffile);
			// version
			fread((char *)&TestFileVersion, sizeof (char), 1, ffile);
			// test to see if same version file as index
			if (TestFileVersion == Index.FileVersion)
				{
				for (DatPt = 0; DatPt < Index.CellDat[FileCt].DatCt; DatPt ++)
					{
					if (FolData.ReadFoliageRecord(ffile))
						{
						if (FolData.InterpretFoliageRecord(GlobalApp->AppEffects, GlobalApp->AppImages, &PointData))
							{
							// draw a foliage item using info in PointData same as for foliage effects
							} // if
						} // if
					} // for
				} // if
			fclose(ffile);
			} // if
		} // if

	} // for

} // ViewGUI::AllInOneRealtimeFoliage

*/

/*===========================================================================*/

void ViewGUI::DrawTargetObjects(int ViewNum, ViewContext *VC)
{
double Size; // Half of actual size
double FractSize;
Point4f Specular, Diffuse, Partial /*, PosCartf */;
Point3d PosCart;
float Shine;
//Light *CurrentObj;
Camera *CurrentObj;
VertexDEM Pos;
int FXIdx;
char AmIActive = 0, IsPlan = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_TARGETS))) return;

if (IsPlan = VC->IsPlan())
	{
	Size = 4.0 * VC->PlanDegPerPix;
	} // if
else
	{
	Size = ObjectUnit;
	} // else

FractSize = Size * .075;

// Setup surface attributes

// Targets are green for now.
Diffuse[0] = 0.0f;
Diffuse[1] = 1.0f;
Diffuse[2] = 0.0f;
Diffuse[3] = 1.0f;

Partial[0] = Diffuse[0] * .25f;
Partial[1] = Diffuse[1] * .25f;
Partial[2] = Diffuse[2] * .25f;
Partial[3] = 1.0f;

Specular[0] = 0.90f;
Specular[1] = 0.90f;
Specular[2] = 0.90f;
Specular[3] = 1.0f;

Shine = 64.0f;

ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Partial);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &Shine);
Diffuse[0] = 0.5f;
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Partial);

for (FXIdx = 0, CurrentObj = (Camera *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA));
 CurrentObj; CurrentObj = (Camera *)(CurrentObj->Next))
	{
	// Don't try to draw ourselves
	if (CurrentObj->Enabled && CurrentObj->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
		{
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (CurrentObj == RasterAnimHost::GetActiveRAHost()) || (CurrentObj == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		if (AmIActive)
			{ // only active if a target parameter
			if ((RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]) ||
			 (RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]) ||
			 (RasterAnimHost::GetActiveRAHost() == &CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]))
				{
				AmIActive = 1;
				} // if
			else
				{
				AmIActive = 0;
				} // else
			} // if
		Pos.Lat  = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue(0);
		Pos.Lon  = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue(0);
		Pos.Elev = CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue(0);
		if (IsPlan)
			{
			PosCart[0] = Pos.Lon;
			PosCart[1] = Pos.Lat;
			PosCart[2] = Pos.Elev;
			} // if
		else
			{
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&Pos);
			#else // WCS_BUILD_VNS
			Pos.DegToCart(VC->GlobeRadius);
			#endif // WCS_BUILD_VNS
			CopyPoint3d(PosCart, Pos.XYZ);
			AddPoint3d(PosCart, VC->NegOrigin);
			InvertZ(PosCart);
			} // else

		glPushMatrix(); // on MODELVIEW
		glTranslated(PosCart[0], PosCart[1], PosCart[2]);
		if (!IsPlan)
			{
			glStandUpright(Pos.Lat, Pos.Lon);
			} // if
		if (AmIActive)
			{
			DrawSetupActiveObject(VC);
			} // if
		SetupPickName(&CurrentObj->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]);
		gldrawbox(-Size, Size, -FractSize, FractSize, -FractSize, FractSize, GL_QUADS);
		gldrawbox(-FractSize, FractSize, -FractSize, FractSize, -Size, Size, GL_QUADS);
		gldrawbox(-FractSize, FractSize, -Size, Size, -FractSize, FractSize, GL_QUADS);
		ClearPickName();
		if (AmIActive)
			{
			DrawFinishActiveObject(VC);
			} // if
		glPopMatrix(); // on MODELVIEW

		FXIdx ++;
		} // if
	} // for


} // ViewGUI::DrawTargetObjects

/*===========================================================================*/

void ViewGUI::DrawWaveObjects(int ViewNum, ViewContext *VC)
{
double Size; // Half of actual size
double FractSize;
Point4f Diffuse;
Point3d PosCart, BasePos;
CloudEffect *Puffy;
WaveEffect *WE;
WaveSource *Typhoon;
VertexDEM Pos;
int FXIdx;
char AmIActive = 0, IsPlan = 0, FogOn;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (IsPlan = VC->IsPlan())
	{
	Size = 8.0 * VC->PlanDegPerPix;
	glDisable(GL_DEPTH_TEST);
	} // if
else
	{
	Size = ObjectUnit;
	} // else

FractSize = Size * .5;

// Setup surface attributes

FogOn = glIsEnabled(GL_FOG);
glDisable(GL_FOG);
glDisable(GL_LIGHTING);


if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS)))
	{
	// CloudWaves are very light blue for now.
	Diffuse[0] = 0.75f;
	Diffuse[1] = 0.85f;
	Diffuse[2] = 1.0f;
	Diffuse[3] = 1.0f;
	glColor4fv(Diffuse);
	for (FXIdx = 0, Puffy = (CloudEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD));
	 Puffy; Puffy = (CloudEffect *)(Puffy->Next))
		{
		if (Puffy->Enabled)
			{
			BasePos[0] = Puffy->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].GetCurValue(0);
			BasePos[1] = Puffy->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].GetCurValue(0);
			BasePos[2] = Puffy->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].GetCurValue(0);
			for (Typhoon = Puffy->WaveSources; Typhoon; Typhoon = Typhoon->Next)
				{
				if (Typhoon->Enabled)
					{
					if (RasterAnimHost::GetActiveRAHost())
						AmIActive = (Typhoon == RasterAnimHost::GetActiveRAHost()) || (Typhoon == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));

					Pos.Lat  = BasePos[0] + (Typhoon->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].GetCurValue(0) / LatScale(VC->GetRadius()));
					Pos.Lon  = BasePos[1] - (Typhoon->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].GetCurValue(0) / LonScale(VC->GetRadius(), BasePos[0]));
					Pos.Elev = BasePos[2];
					glPushMatrix(); // on MODELVIEW
					if (IsPlan)
						{
						PosCart[0] = Pos.Lon;
						PosCart[1] = Pos.Lat;
						PosCart[2] = 0.0;
						glTranslated(PosCart[0], PosCart[1], PosCart[2]);
						} // if
					else
						{
						#ifdef WCS_BUILD_VNS
						DefCoords->DegToCart(&Pos);
						#else // WCS_BUILD_VNS
						Pos.DegToCart(VC->GlobeRadius);
						#endif // WCS_BUILD_VNS
						CopyPoint3d(PosCart, Pos.XYZ);
						AddPoint3d(PosCart, VC->NegOrigin);
						InvertZ(PosCart);

						glTranslated(PosCart[0], PosCart[1], PosCart[2]);
						glStandUpright(Pos.Lat, Pos.Lon);
						} // else

					if (AmIActive)
						{
						DrawSetupActiveObject(VC);
						} // if
					SetupPickName(Typhoon);
					gldrawW(FractSize);
					ClearPickName();
					if (AmIActive)
						{
						DrawFinishActiveObject(VC);
						} // if
					glPopMatrix(); // on MODELVIEW
					} // if
				} // for
			FXIdx ++;
			} // if
		} // for
	} // if

if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_WAVES)))
	{
	// Waves are blue for now.
	Diffuse[0] = 0.1f;
	Diffuse[1] = 0.1f;
	Diffuse[2] = 1.0f;
	Diffuse[3] = 1.0f;
	glColor4fv(Diffuse);

	for (FXIdx = 0, WE = (WaveEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE));
	 WE; WE = (WaveEffect *)(WE->Next))
		{
		if (WE->Enabled)
			{
			BasePos[0] = WE->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].GetCurValue(0);
			BasePos[1] = WE->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].GetCurValue(0);
			BasePos[2] = 0.0;
			for (Typhoon = WE->WaveSources; Typhoon; Typhoon = Typhoon->Next)
				{
				if (Typhoon->Enabled)
					{
					if (RasterAnimHost::GetActiveRAHost())
						AmIActive = (Typhoon == RasterAnimHost::GetActiveRAHost()) || (Typhoon == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));

					Pos.Lat  = BasePos[0] + (Typhoon->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].GetCurValue(0) / LatScale(VC->GetRadius()));
					Pos.Lon  = BasePos[1] - (Typhoon->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].GetCurValue(0) / LonScale(VC->GetRadius(), BasePos[0]));
					Pos.Elev = BasePos[2];

					glPushMatrix(); // on MODELVIEW
					if (IsPlan)
						{
						PosCart[0] = Pos.Lon;
						PosCart[0] = Pos.Lat;
						PosCart[0] = 0.0;
						} // if
					else
						{
						#ifdef WCS_BUILD_VNS
						DefCoords->DegToCart(&Pos);
						#else // WCS_BUILD_VNS
						Pos.DegToCart(VC->GlobeRadius);
						#endif // WCS_BUILD_VNS
						CopyPoint3d(PosCart, Pos.XYZ);
						AddPoint3d(PosCart, VC->NegOrigin);
						InvertZ(PosCart);

						glTranslated(PosCart[0], PosCart[1], PosCart[2]);
						glStandUpright(Pos.Lat, Pos.Lon);
						} // else

					if (AmIActive)
						{
						DrawSetupActiveObject(VC);
						} // if
					SetupPickName(Typhoon);
					gldrawW(FractSize);
					ClearPickName();
					if (AmIActive)
						{
						DrawFinishActiveObject(VC);
						} // if
					glPopMatrix(); // on MODELVIEW

					} // if
				} // for
			FXIdx ++;
			} // if
		} // for
	} // if

glEnable(GL_LIGHTING);
if (FogOn) glEnable(GL_FOG);
if (IsPlan) glEnable(GL_DEPTH_TEST);

} // ViewGUI::DrawWaveObjects

/*===========================================================================*/

void ViewGUI::DrawLightObjects(int ViewNum, ViewContext *VC)
{
double LightRad, Angle; // Half of actual size
Point4f Specular, Diffuse, Partial;
Point3d PosCart, TargVec, Zero;
float Shine;
Light *CurrentObj;
VertexDEM Pos;
int FXIdx;
double Size; // Half of actual size
char AmIActive = 0, Moved = 0, AimMe = 0, PlanView;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

Size = ObjectUnit;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_LIGHTS))) return;

PlanView = VC->IsPlan();

// Setup surface attributes

// lights are partially transparent
Diffuse[0] = 0.0f;
Diffuse[1] = 0.0f;
Diffuse[2] = 0.0f;
Diffuse[3] = 0.35f;

Specular[0] = 0.50f;
Specular[1] = 0.50f;
Specular[2] = 0.50f;
Specular[3] = 1.0f;

Shine = 64.0f;

Zero[0] = 0.0;
Zero[1] = 0.0;
Zero[2] = 0.0;


for (FXIdx = 0, CurrentObj = (Light *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT));
 CurrentObj; CurrentObj = (Light *)(CurrentObj->Next))
	{
	Moved = AimMe = 0;
	if (CurrentObj->Enabled)
		{
		//Point3f LookAtPoint, PlanUpVec;
		VertexDEM Trans;
		double CamMag;

		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (CurrentObj == RasterAnimHost::GetActiveRAHost()) || (CurrentObj == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		Diffuse[0] = (float)CurrentObj->Color.GetClampedCompleteValue(0);
		Diffuse[1] = (float)CurrentObj->Color.GetClampedCompleteValue(1);
		Diffuse[2] = (float)CurrentObj->Color.GetClampedCompleteValue(2);
		Angle    = CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].GetCurValue(0);
		if (PlanView)
			{
			Size = 6.0 * VC->PlanDegPerPix;
			} // if
		if (CurrentObj->SoftShadows && ! PlanView)
			{
			LightRad = CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].GetCurValue(0);
			} // if
		else
			{
			LightRad = Size * .5;
			} // else
		Pos.Lat  = CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetCurValue(0);
		Pos.Lon  = CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetCurValue(0);
		Pos.Elev = CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetCurValue(0);
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&Pos);
		#else // WCS_BUILD_VNS
		Pos.DegToCart(VC->GlobeRadius);
		#endif // WCS_BUILD_VNS

		CamMag = VectorMagnitude(Pos.XYZ);


		CopyPoint3d(PosCart, Pos.XYZ);
		AddPoint3d(PosCart, VC->NegOrigin);
		InvertZ(PosCart);

		glPushMatrix(); // on MODELVIEW
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (CurrentObj->LightType != WCS_EFFECTS_LIGHTTYPE_OMNI)
			{
			if (CurrentObj->LightAim)
				{
				CopyPoint3d(TargVec, CurrentObj->LightAim->XYZ);
				AimMe = 1;
				} // if
			if (CurrentObj->Distant)
				{ 
				// Distant means pointed at center of planet
				CopyPoint3d(TargVec, PosCart);
				NegateVector(TargVec);
				AimMe = 1;
				} // if
			if ((TargVec[0] == 0.0 && TargVec[1] == 0.0 && TargVec[2] == 0.0))
				{
				AimMe = 0;
				} // if
			if (AimMe)
				{
				if (PlanView)
					{
					// 
					} // if
				else
					{
					// it's a good bet that far-distant lights won't appear if drawn here,
					// because they'll be out of the near-far clip range. No clever tricks
					// come to mind for this. Maybe someday one will.
					multLookAtVec((float)PosCart[0], (float)PosCart[1], (float)PosCart[2],
					 (float)TargVec[0], (float)TargVec[1], (float)-TargVec[2],
					 (float)TargVec[0], (float)TargVec[2], (float)(-TargVec[1]));
					Moved = 1;
					} // else
				} // if
			} // if
		if (!Moved)
			{
			if (PlanView)
				{
				// keep lights within PlanView's vertical range
				if (Pos.Elev > VC->NearClip) Pos.Elev = VC->NearClip + ((VC->FarClip - VC->NearClip) / 4);
				glTranslatef((float)Pos.Lon, (float)Pos.Lat, (float)Pos.Elev);
				} // if
			else
				{
				glTranslatef((float)PosCart[0], (float)PosCart[1], (float)PosCart[2]);
				} // else
			Moved = 1;
			} // if

		//glScaled(LightRad, LightRad, LightRad);
		Diffuse[3] = 1.0f;
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
		//gldrawbox(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, GL_QUADS);
		//if (!CurrentObj->Distant)
			{
			Partial[0] = Diffuse[0] * .75f;
			Partial[1] = Diffuse[1] * .75f;
			Partial[2] = Diffuse[2] * .75f;
			Partial[3] = .60f;

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Partial);
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &Shine);
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Partial);

			if (AmIActive)
				{
				DrawSetupActiveObject(VC);
				} // if
			SetupPickName(CurrentObj);
			switch(CurrentObj->LightType)
				{
				case WCS_EFFECTS_LIGHTTYPE_OMNI:
					{
					fauxSolidSphere(LightRad);
					break;
					} // 
				case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
					{
					fauxSolidCyl(LightRad, LightRad, LightRad * 2.0);
					fauxSolidDisk(LightRad);
					break;
					} // 
				case WCS_EFFECTS_LIGHTTYPE_SPOT:
					{
					float ConeAng, ConeEndWidth;
					ConeAng = (float)CurrentObj->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].GetCurValue(0);
					//ConeEndWidth = LightRad;
					ConeEndWidth = (float)(tan(.5 * ConeAng * PiOver180) * LightRad * 2.0);
					fauxSolidCyl(0.0, (double)ConeEndWidth, LightRad * 2.0);
					break;
					} // 
				} // switch
			ClearPickName();
			if (AmIActive)
				{
				DrawFinishActiveObject(VC);
				} // if
			} // not distant

		glDisable(GL_BLEND);
		glPopMatrix(); // on MODELVIEW

		FXIdx ++;
		} // if
	} // for

//FinishDisplayList();

} // ViewGUI::DrawLightObjects

/*===========================================================================*/

void ViewGUI::DrawWater(int ViewNum, ViewContext *VC)
{
double North, South, East, West, TempLon, EastDiff, WestDiff;
double SeaLevel, DEMOriLon, PlanCamOriLon;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ProjectedPlan = NULL;
Joe *Clip;
JoeDEM *MyDEM;
LakeEffect *Ocean;
MaterialEffect *WaterMat;
GradientCritter *WaterNode;
Point4f WaterSpecular, WaterColor;
int DEMSteps = 0;
int LatSegSteps, LonSegSteps, LatSeg, LonSeg;
float WaterSpec, WaterSpecExp, WaterIntense;
VertexDEM NW, NE, SW, SE;
int FXIdx, DoOcean = 0;

// Setup surface attributes

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_LAKES))) return;

if (VC->IsPlan())
	{
	PlanCamOriLon = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
	LatSegSteps = LonSegSteps = 1;
	ProjectedPlan = VC->GetProjected();
	} // if

SeaLevel = 0.0;
WaterSpec = 0.10f;
WaterSpecExp = 64.0f;
WaterColor[0] = 0.25f;
WaterColor[1] = 0.35f;
WaterColor[2] = 1.0f;
WaterColor[3] = 0.5f;

for (FXIdx = 0, Ocean = (LakeEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LAKE));
 Ocean; Ocean = (LakeEffect *)(Ocean->Next))
	{
	// vector bounded lakes are not oceans. Search queries disqualify an object from being an ocean as well.
	if (Ocean->Enabled && ! Ocean->Joes && ! Ocean->Search)
		{
		DoOcean = 1;
		SetupPickName(Ocean);
		SeaLevel     = Ocean->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].GetCurValue(0);
		if (VC->Planet->EcoExageration)
			{
			SeaLevel    = VC->VCCalcExag(SeaLevel);
			} // if
		// get active water material
		if ((WaterNode = Ocean->WaterMat.GetActiveNode()) && (WaterMat = (MaterialEffect *)WaterNode->GetThing()))
			{
			WaterSpec    = (float)WaterMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
			WaterSpecExp = (float)WaterMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);
			WaterIntense = (float)WaterMat->DiffuseColor.GetIntensity();
			WaterColor[0] = WaterIntense * (float)WaterMat->DiffuseColor.GetCurValue(0);
			WaterColor[1] = WaterIntense * (float)WaterMat->DiffuseColor.GetCurValue(1);
			WaterColor[2] = WaterIntense * (float)WaterMat->DiffuseColor.GetCurValue(2);
			} // if
		else
			{
			WaterSpec    = 0.5f;
			WaterSpecExp = 10.0f;
			WaterIntense = 1.0f;
			WaterColor[0] = WaterIntense * 0.2f;
			WaterColor[1] = WaterIntense * 0.3f;
			WaterColor[2] = WaterIntense * 0.5f;
			} // else
		FXIdx ++;
		} // if
	} // for

if (!DoOcean) return;

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




WaterSpecular[0] = WaterSpecular[1] = WaterSpecular[2] = WaterSpec;
WaterSpecular[2] = 1.0f;

ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, WaterSpecular);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &WaterSpecExp);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, WaterColor);
WaterColor[0] = WaterColor[1] = WaterColor[2] = 0.0f;
WaterColor[3] = 1.0f;
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, WaterColor);
ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, WaterColor);

glDisable(GL_CULL_FACE);
glPushMatrix(); // on MODELVIEW

for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if (VC->IsPlan())
					{
					North = Clip->NWLat;
					South = Clip->SELat;
					East  = Clip->SELon;
					West  = Clip->NWLon;

					DEMOriLon = (West + East) * 0.5;
					TempLon = DEMOriLon - PlanCamOriLon;
					EastDiff = DEMOriLon - East;
					WestDiff = DEMOriLon - West;
					if (fabs(TempLon) > 180.0)
						{
						TempLon += 180.0;
						if (fabs(TempLon) > 360.0)
							TempLon = fmod(TempLon, 360.0);
						if (TempLon < 0.0)
							TempLon += 360.0;
						TempLon -= 180.0;
						DEMOriLon = TempLon + PlanCamOriLon;
						East = DEMOriLon - EastDiff;
						West = DEMOriLon - WestDiff;
						} // if
					// replaced by above
					//while (DEMOriLon - PlanCamOriLon > 180.0)
					//	{
					//	DEMOriLon -= 360.0;
					//	East -= 360.0;
					//	West -= 360.0;
					//	} // while
					//while (DEMOriLon - PlanCamOriLon < -180.0)
					//	{
					//	DEMOriLon += 360.0;
					//	East += 360.0;
					//	West += 360.0;
					//	} // while

					NW.XYZ[0] = West;
					NW.XYZ[1] = North;
					NW.XYZ[2] = SeaLevel;

					NE.XYZ[0] = East;
					NE.XYZ[1] = North;
					NE.XYZ[2] = SeaLevel;

					SW.XYZ[0] = West;
					SW.XYZ[1] = South;
					SW.XYZ[2] = SeaLevel;

					SE.XYZ[0] = East;
					SE.XYZ[1] = South;
					SE.XYZ[2] = SeaLevel;

					#ifdef WCS_BUILD_VNS
					if (ProjectedPlan)
						{
						// project all corners into View's CS, and replace into .XZY so code below is unchanged
						VertexDEM ProjConvert;

						// NW
						ProjConvert.Lon  = NW.XYZ[0];
						ProjConvert.Lat  = NW.XYZ[1];
						ProjConvert.Elev = 0.0;
						ProjectForViews(VC, &ProjConvert);
						NW.XYZ[0] = ProjConvert.XYZ[0];
						NW.XYZ[1] = ProjConvert.XYZ[1];

						// NE
						ProjConvert.Lon  = NE.XYZ[0];
						ProjConvert.Lat  = NE.XYZ[1];
						ProjConvert.Elev = 0.0;
						ProjectForViews(VC, &ProjConvert);
						NE.XYZ[0] = ProjConvert.XYZ[0];
						NE.XYZ[1] = ProjConvert.XYZ[1];

						// SW
						ProjConvert.Lon  = SW.XYZ[0];
						ProjConvert.Lat  = SW.XYZ[1];
						ProjConvert.Elev = 0.0;
						ProjectForViews(VC, &ProjConvert);
						SW.XYZ[0] = ProjConvert.XYZ[0];
						SW.XYZ[1] = ProjConvert.XYZ[1];

						// SE
						ProjConvert.Lon  = SE.XYZ[0];
						ProjConvert.Lat  = SE.XYZ[1];
						ProjConvert.Elev = 0.0;
						ProjectForViews(VC, &ProjConvert);
						SE.XYZ[0] = ProjConvert.XYZ[0];
						SE.XYZ[1] = ProjConvert.XYZ[1];
						} // if
					#endif // WCS_BUILD_VNS


					glBegin(GL_TRIANGLES);
					glTriPoly(SW.XYZ, NE.XYZ, SE.XYZ);
					glTriPoly(NW.XYZ, NE.XYZ, SW.XYZ);
					glEnd();
					} // if
				else
					{
					North = fabs(Clip->NWLat - Clip->SELat);
					West  = fabs(Clip->NWLon  -  Clip->SELon);
					LatSegSteps = max(4, (int)North);
					LonSegSteps = max(4, (int)West);

					NW.Elev = NE.Elev = SW.Elev = SE.Elev = SeaLevel;

					/*
					NW.Lat  = Clip->NWLat; 
					NW.Lon  = Clip->NWLon;
					NE.Lat  = Clip->NWLat; 
					NE.Lon  = Clip->SELon;
					SW.Lat  = Clip->SELat; 
					SW.Lon  = Clip->NWLon;
					SE.Lat  = Clip->SELat; 
					SE.Lon  = Clip->SELon;

					glBegin(GL_TRIANGLES);
					if (!((NW.Lat > 89.999) && (NW.Lat > 89.999)))
						{
						glTriPoly(NW.XYZ, NE.XYZ, SW.XYZ);
						} // if
					if (!((SE.Lat < -89.999) && (SW.Lat < -89.999)))
						{
						glTriPoly(NE.XYZ, SE.XYZ, SW.XYZ);
						} // if
					glEnd();

					*/

					// North and West are initialized to Lat and Lon extent from above
					// Now South and East are calculated to degrees per segment
					South = North / (double)LatSegSteps;
					East  = West  / (double)LonSegSteps;

					// We go SE to SW, and south to north.
					for (LonSeg = 0; LonSeg < LonSegSteps; LonSeg++)
						{
						for (LatSeg = 0; LatSeg < LatSegSteps; LatSeg++)
							{
							NE.Lat  = NW.Lat  = Clip->SELat + (double)(LatSeg + 1) * South;
							NW.Lon  = SW.Lon  = Clip->SELon + (double)(LonSeg + 1) * East;
							SW.Lat  = SE.Lat  = Clip->SELat + (double)LatSeg * South; 
							NE.Lon  = SE.Lon  = Clip->SELon + (double)LonSeg * East;

							#ifdef WCS_BUILD_VNS
							DefCoords->DegToCart(&NW);
							#else // WCS_BUILD_VNS
							NW.DegToCart(VC->GlobeRadius);
							#endif // WCS_BUILD_VNS
							UnitVectorCopy(NW.xyz, NW.XYZ);
							AddPoint3d(NW.XYZ, VC->NegOrigin);
							InvertZ(NW.XYZ);
							InvertXYd(NW.xyz);
							#ifdef WCS_BUILD_VNS
							DefCoords->DegToCart(&NE);
							#else // WCS_BUILD_VNS
							NE.DegToCart(VC->GlobeRadius);
							#endif // WCS_BUILD_VNS
							UnitVectorCopy(NE.xyz, NE.XYZ);
							AddPoint3d(NE.XYZ, VC->NegOrigin);
							InvertZ(NE.XYZ);
							InvertXYd(NE.xyz);
							#ifdef WCS_BUILD_VNS
							DefCoords->DegToCart(&SW);
							#else // WCS_BUILD_VNS
							SW.DegToCart(VC->GlobeRadius);
							#endif // WCS_BUILD_VNS
							UnitVectorCopy(SW.xyz, SW.XYZ);
							AddPoint3d(SW.XYZ, VC->NegOrigin);
							InvertZ(SW.XYZ);
							InvertXYd(SW.xyz);
							#ifdef WCS_BUILD_VNS
							DefCoords->DegToCart(&SE);
							#else // WCS_BUILD_VNS
							SE.DegToCart(VC->GlobeRadius);
							#endif // WCS_BUILD_VNS
							UnitVectorCopy(SE.xyz, SE.XYZ);
							AddPoint3d(SE.XYZ, VC->NegOrigin);
							InvertZ(SE.XYZ);
							InvertXYd(SE.xyz);

							glBegin(GL_TRIANGLES);
							//if (!((NW.Lat > 89.999) && (NW.Lat > 89.999)))
								{
								//glTriPoly(NW.XYZ, NE.XYZ, SW.XYZ);
								glTriPolyVertexDEM(NW, NE, SW);
								} // if
							//if (!((SE.Lat < -89.999) && (SW.Lat < -89.999)))
								{
								//glTriPoly(NE.XYZ, SE.XYZ, SW.XYZ);
								glTriPolyVertexDEM(NE, SE, SW);
								} // if
							glEnd();
							} // for
						} // for

					} // else
				} // 
			} // if
		} // if
	} // for
ClearPickName();
glPopMatrix(); // on MODELVIEW
glEnable(GL_CULL_FACE);
glDisable(GL_BLEND);

//FinishDisplayList();

} // ViewGUI::DrawWater

/*===========================================================================*/

// utility function, may move elsewhere
void MakeGeoBoundingBox(double MinX, double MaxX, double MinY, double MaxY, double &MinLon, double &MaxLon, double &MinLat, double &MaxLat, CoordSys *CS)
{
VertexDEM Convert;

if (CS)
	{
	// blow out extrema
	MinLon =  DBL_MAX;
	MaxLon = -DBL_MAX;
	MinLat =  DBL_MAX;
	MaxLat = -DBL_MAX;

	// LL corner
	Convert.xyz[0] = MinX;
	Convert.xyz[1] = MinY;
	Convert.xyz[2] = 0.0; // elev is irrelevant
	CS->ProjToDefDeg(&Convert);
	// no need to test, first one is always used initially
	MinLat = MaxLat = Convert.Lat;
	MinLon = MaxLon = Convert.Lon;

	// LR corner
	Convert.xyz[0] = MaxX;
	Convert.xyz[1] = MinY;
	Convert.xyz[2] = 0.0; // elev is irrelevant
	CS->ProjToDefDeg(&Convert);
	if (Convert.Lat > MaxLat) MaxLat = Convert.Lat;
	if (Convert.Lat < MinLat) MinLat = Convert.Lat;
	if (Convert.Lon > MaxLon) MaxLon = Convert.Lon;
	if (Convert.Lon < MinLon) MinLon = Convert.Lon;

	// UR corner
	Convert.xyz[0] = MaxX;
	Convert.xyz[1] = MaxY;
	Convert.xyz[2] = 0.0; // elev is irrelevant
	CS->ProjToDefDeg(&Convert);
	if (Convert.Lat > MaxLat) MaxLat = Convert.Lat;
	if (Convert.Lat < MinLat) MinLat = Convert.Lat;
	if (Convert.Lon > MaxLon) MaxLon = Convert.Lon;
	if (Convert.Lon < MinLon) MinLon = Convert.Lon;

	// UL corner
	Convert.xyz[0] = MinX;
	Convert.xyz[1] = MaxY;
	Convert.xyz[2] = 0.0; // elev is irrelevant
	CS->ProjToDefDeg(&Convert);
	if (Convert.Lat > MaxLat) MaxLat = Convert.Lat;
	if (Convert.Lat < MinLat) MinLat = Convert.Lat;
	if (Convert.Lon > MaxLon) MaxLon = Convert.Lon;
	if (Convert.Lon < MinLon) MinLon = Convert.Lon;
	} // if
else
	{
	MinLat = MinY;
	MinLon = MinX;
	MaxLat = MaxY;
	MaxLon = MaxX;
	} // else

} // MakeGeoBoundingBox

/*===========================================================================*/

void ViewGUI::DrawCMaps(int ViewNum, ViewContext *VC)
{
Point4f Specular, Color, EdgeColor;
int DEMSteps = 0;
CmapEffect *CM;
Raster *CMRas;
GeoRefShell *GRS;
double Bottom, CMapCenLon, PlanCamOriLon;
double HighY, LowY, HighX, LowX;
double HighLat, LowLat, HighLon, LowLon, ShiftLon = 0.0;
float CMSpecular, CMSpecularExp;// CloudIntense 
VertexDEM UL, UR, LL, LR;
int YSegSteps, XSegSteps, YSeg, XSeg;
double YRange, YStep, XStep, XRange, LatRange, LonRange, TempLon, ShiftLonDiff;
char AmIActive = 0, IsPlan = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ImgCoords = NULL;
RasterAttribute *RA;
CoordSys *ProjectedPlan = NULL;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CMAPS))) return;

if (IsPlan = VC->IsPlan())
	{
	glDisable(GL_DEPTH_TEST);
	ProjectedPlan = VC->GetProjected();
	} // if

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


// Setup surface attributes

Bottom = TexElMax;
if (Bottom == -DBL_MAX) Bottom = 0.0;
CMSpecular = 0.10f;
CMSpecularExp = 64.0f;
Color[0] = 0.5f;
Color[1] = 1.0f;
Color[2] = 0.5f;
Color[3] = 0.5f;

for (CM = (CmapEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CMAP));
 CM; CM = (CmapEffect *)(CM->Next))
	{
	if (CM->Enabled)
		{
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (CM == RasterAnimHost::GetActiveRAHost()) || (CM == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		CM->InitFrameToRender(GlobalApp->AppEffects, CamSetup);

		if (CM->Img && (CMRas = CM->Img->Rast))
			{
			if (RA = CMRas->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
				{
				if (GRS = (GeoRefShell *)RA->Shell)
					{
					ImgCoords = (CoordSys *)GRS->GetHost();
					HighY = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetCurValue(0);
					LowY  = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetCurValue(0);
					HighX = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetCurValue(0);
					LowX  = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetCurValue(0);

					MakeGeoBoundingBox(LowX, HighX, LowY, HighY, LowLon, HighLon, LowLat, HighLat, ImgCoords);
					} // if
				else
					continue;	// prevent trying to draw non-existent image area
				} // if
			else
				continue;	// prevent trying to draw non-existent image area
			} // if
		else
			continue;	// prevent trying to draw non-existent image area

		if (IsPlan)
			{
			PlanCamOriLon = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
			CMapCenLon = (HighLon + LowLon) * 0.5;
			TempLon = CMapCenLon - PlanCamOriLon;
			ShiftLonDiff = CMapCenLon - ShiftLon;
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180.0;
				if (fabs(TempLon) > 360.0)
					TempLon = fmod(TempLon, 360.0);
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				CMapCenLon = TempLon + PlanCamOriLon;
				ShiftLon = CMapCenLon - ShiftLonDiff;
				} // if
			// replaced by above
			//while (CMapCenLon - PlanCamOriLon > 180.0)
			//	{
			//	CMapCenLon -= 360.0;
			//	ShiftLon -= 360.0;
			//	} // while
			//while (CMapCenLon - PlanCamOriLon < -180.0)
			//	{
			//	CMapCenLon += 360.0;
			//	ShiftLon += 360.0;
			//	} // while
			} // if


		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Color);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Color);


//		EdgeColor[0] = (float)Color[0] * 0.5f;
//		EdgeColor[1] = (float)Color[1] * 0.5f;
//		EdgeColor[2] = (float)Color[2] * 0.5f;
//		EdgeColor[3] = Color[3];


		EdgeColor[0] = 1.0f;
		EdgeColor[1] = 0.0f;
		EdgeColor[2] = 0.0f;
		EdgeColor[3] = Color[3];

		Specular[0] = Specular[1] = Specular[2] = CMSpecular;
		Specular[2] = 1.0f;

		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &CMSpecularExp);

		if (AmIActive)
			{
			DrawSetupActiveObject(VC);
			} // if
		SetupPickName(CM);

		if (IsPlan)
			{
			if (ImgCoords)
				{
				UL.xyz[0] = LowX;
				UL.xyz[1] = HighY;
				ImgCoords->ProjToDefDeg(&UL);
				UL.XYZ[0] = UL.Lon;
				UL.XYZ[1] = UL.Lat;

				UR.xyz[0] = HighX;
				UR.xyz[1] = HighY;
				ImgCoords->ProjToDefDeg(&UR);
				UR.XYZ[0] = UR.Lon;
				UR.XYZ[1] = UR.Lat;

				LL.xyz[0] = LowX;
				LL.xyz[1] = LowY;
				ImgCoords->ProjToDefDeg(&LL);
				LL.XYZ[0] = LL.Lon;
				LL.XYZ[1] = LL.Lat;

				LR.xyz[0] = HighX;
				LR.xyz[1] = LowY;
				ImgCoords->ProjToDefDeg(&LR);
				LR.XYZ[0] = LR.Lon;
				LR.XYZ[1] = LR.Lat;
				} // if
			else
				{
				UL.XYZ[0] = LL.XYZ[0] = HighX;
				LR.XYZ[0] = UR.XYZ[0] = LowX;
				UL.XYZ[1] = UR.XYZ[1] = HighY;
				LL.XYZ[1] = LR.XYZ[1] = LowY;
				} // else
			UL.XYZ[2] = LL.XYZ[2] = UR.XYZ[2] = LR.XYZ[2] = 0.0;

			#ifdef WCS_BUILD_VNS
			if (ProjectedPlan)
				{
				// project all corners into View's CS, and replace into .XZY so code below is unchanged
				VertexDEM ProjConvert;

				// UL
				ProjConvert.Lon  = UL.XYZ[0];
				ProjConvert.Lat  = UL.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				UL.XYZ[0] = ProjConvert.XYZ[0];
				UL.XYZ[1] = ProjConvert.XYZ[1];

				// UR
				ProjConvert.Lon  = UR.XYZ[0];
				ProjConvert.Lat  = UR.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				UR.XYZ[0] = ProjConvert.XYZ[0];
				UR.XYZ[1] = ProjConvert.XYZ[1];

				// LL
				ProjConvert.Lon  = LL.XYZ[0];
				ProjConvert.Lat  = LL.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				LL.XYZ[0] = ProjConvert.XYZ[0];
				LL.XYZ[1] = ProjConvert.XYZ[1];

				// LR
				ProjConvert.Lon  = LR.XYZ[0];
				ProjConvert.Lat  = LR.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				LR.XYZ[0] = ProjConvert.XYZ[0];
				LR.XYZ[1] = ProjConvert.XYZ[1];
				} // if
			#endif // WCS_BUILD_VNS

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			glBegin(GL_TRIANGLES);
			glTriPoly(UL.XYZ, UR.XYZ, LL.XYZ);
			glTriPoly(UR.XYZ, LR.XYZ, LL.XYZ);
			glEnd();

			// draw outline
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);

			glBegin(GL_LINE_LOOP);
			glQuadOutline(UL.XYZ, UR.XYZ, LR.XYZ, LL.XYZ);
			glEnd();
			} // if
		else
			{
			YRange = (HighY - LowY);
			XRange  = (HighX - LowX);
			LatRange = fabs(HighLat - LowLat);
			LonRange  = fabs(HighLon - LowLon);
			YSegSteps = max(4, (int)LatRange);
			XSegSteps = max(4, (int)LonRange);

			// YRange and XRange are initialized to Y and X extent from above
			// Now YStep and XStep are calculated to distance per segment
			YStep = YRange / (double)YSegSteps;
			XStep  = XRange  / (double)XSegSteps;

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			// We go LR to LL, and south to north.
			for (XSeg = 0; XSeg < XSegSteps; XSeg++)
				{
				for (YSeg = 0; YSeg < YSegSteps; YSeg++)
					{
					double UX, LX, UY, LY;

					UL.Elev = LL.Elev = UR.Elev = LR.Elev = Bottom;
					UL.xyz[2] = LL.xyz[2] = UR.xyz[2] = LR.xyz[2] = Bottom;

					UY  = LowY + (double)(YSeg + 1) * YStep;
					LX  = LL.Lon  = LowX + (double)(XSeg + 1) * XStep;
					LY  = LowY + (double)YSeg * YStep; 
					UX  = LowX + (double)XSeg * XStep;

					if (!ImgCoords)
						{
						UR.Lat  = UL.Lat  = UY;
						UL.Lon  = LL.Lon  = LX;
						LL.Lat  = LR.Lat  = LY; 
						UR.Lon  = LR.Lon  = UX;
						} // if
					//DefCoords->ProjToCart(&UL);

					// UL
					#ifdef WCS_BUILD_VNS
					if (ImgCoords)
						{
						UL.xyz[0] = LX;
						UL.xyz[1] = UY;
						ImgCoords->ProjToCart(&UL);
						} // if
					else
						{
						DefCoords->DegToCart(&UL);
						} // else
					#else // WCS_BUILD_VNS
					UL.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(UL.xyz, UL.XYZ);
					AddPoint3d(UL.XYZ, VC->NegOrigin);
					InvertZ(UL.XYZ);
					InvertXYd(UL.xyz);

					// UR
					#ifdef WCS_BUILD_VNS
					if (ImgCoords)
						{
						UR.xyz[0] = UX;
						UR.xyz[1] = UY;
						ImgCoords->ProjToCart(&UR);
						} // if
					else
						{
						DefCoords->DegToCart(&UR);
						} // else
					#else // WCS_BUILD_VNS
					UR.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(UR.xyz, UR.XYZ);
					AddPoint3d(UR.XYZ, VC->NegOrigin);
					InvertZ(UR.XYZ);
					InvertXYd(UR.xyz);
					
					// LL
					#ifdef WCS_BUILD_VNS
					if (ImgCoords)
						{
						LL.xyz[0] = LX;
						LL.xyz[1] = LY;
						ImgCoords->ProjToCart(&LL);
						} // if
					else
						{
						DefCoords->DegToCart(&LL);
						} // else
					#else // WCS_BUILD_VNS
					LL.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(LL.xyz, LL.XYZ);
					AddPoint3d(LL.XYZ, VC->NegOrigin);
					InvertZ(LL.XYZ);
					InvertXYd(LL.xyz);
					
					// LR
					#ifdef WCS_BUILD_VNS
					if (ImgCoords)
						{
						LR.xyz[0] = UX;
						LR.xyz[1] = LY;
						ImgCoords->ProjToCart(&LR);
						} // if
					else
						{
						DefCoords->DegToCart(&LR);
						} // else
					#else // WCS_BUILD_VNS
					LR.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(LR.xyz, LR.XYZ);
					AddPoint3d(LR.XYZ, VC->NegOrigin);
					InvertZ(LR.XYZ);
					InvertXYd(LR.xyz);

					glBegin(GL_TRIANGLES);
					//if (!((UL.Lat > 89.999) && (UL.Lat > 89.999)))
						{
						//glTriPoly(UL.XYZ, UR.XYZ, LL.XYZ);
						glTriPolyVertexDEM(UL, UR, LL);
						} // if
					//if (!((LR.Lat < -89.999) && (LL.Lat < -89.999)))
						{
						//glTriPoly(UR.XYZ, LR.XYZ, LL.XYZ);
						glTriPolyVertexDEM(UR, LR, LL);
						} // if
					glEnd();

					// do we need to draw any edge lines?
					if ((YSeg == 0) || (XSeg == 0) || (YSeg == YSegSteps - 1) || (XSeg == XSegSteps - 1))
						{
						// draw outline
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);
						glBegin(GL_LINES);

						if (YSeg == 0) // south edge E-W
							{
							glVertex3dv(LR.XYZ);
							glVertex3dv(LL.XYZ);
							} // if
						if (XSeg == 0) // east edge S-N
							{
							glVertex3dv(LR.XYZ);
							glVertex3dv(UR.XYZ);
							} // if
						if (YSeg == YSegSteps - 1) // north edge E-W
							{
							glVertex3dv(UR.XYZ);
							glVertex3dv(UL.XYZ);
							} // if
						if (XSeg == XSegSteps - 1) // west edge S-N
							{
							glVertex3dv(LL.XYZ);
							glVertex3dv(UL.XYZ);
							} // if
						//glQuadOutline(UL.XYZ, UR.XYZ, LR.XYZ, LL.XYZ);
						glEnd();
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
						} // if

					} // for
				} // for
			} // else


		ClearPickName();
		if (AmIActive)
			{
			DrawFinishActiveObject(VC);
			} // if
		} // if
	} // for

glDisable(GL_BLEND);

if (IsPlan)
	{
	glEnable(GL_DEPTH_TEST);
	} // if

} // ViewGUI::DrawCMaps

/*===========================================================================*/

#ifdef WCS_BUILD_RTX
void ViewGUI::DrawExportBounds(int ViewNum, ViewContext *VC)
{
Point4f Specular, Color, EdgeColor;
int DEMSteps = 0;
SceneExporter *SE;
double Bottom, ExportCenLon, PlanCamOriLon;
double HighY, LowY, HighX, LowX;
double HighLat, LowLat, HighLon, LowLon, ShiftLon = 0.0;
float CMSpecular, CMSpecularExp;// CloudIntense 
VertexDEM UL, UR, LL, LR;
int YSegSteps, XSegSteps, YSeg, XSeg;
double YRange, YStep, XStep, XRange, LatRange, LonRange, TempLon, ShiftLonDiff;
char AmIActive = 0, IsPlan = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ExportCoords = NULL;
CoordSys *ProjectedPlan = NULL;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_EXPORTERS))) return;

if (IsPlan = VC->IsPlan())
	{
	glDisable(GL_DEPTH_TEST);
	ProjectedPlan = VC->GetProjected();
	} // if

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


// Setup surface attributes

Bottom = TexElMax;
if (Bottom == -DBL_MAX) Bottom = 0.0;
CMSpecular = 0.10f;
CMSpecularExp = 64.0f;
Color[0] = 0.5f;
Color[1] = 0.5f;
Color[2] = 1.0f;
Color[3] = 0.5f;

for (SE = (SceneExporter *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_EXPORTER));
 SE; SE = (SceneExporter *)(SE->Next))
	{
	if (SE->Enabled)
		{
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (SE == RasterAnimHost::GetActiveRAHost()) || (SE == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		SE->InitFrameToRender(GlobalApp->AppEffects, CamSetup);

		ExportCoords = (CoordSys *)SE->Coords;
		HighY = SE->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetCurValue(0);
		LowY  = SE->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetCurValue(0);
		HighX = SE->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetCurValue(0);
		LowX  = SE->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetCurValue(0);

		MakeGeoBoundingBox(LowX, HighX, LowY, HighY, LowLon, HighLon, LowLat, HighLat, ExportCoords);

		if (IsPlan)
			{
			PlanCamOriLon = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
			ExportCenLon = (HighLon + LowLon) * 0.5;
			TempLon = ExportCenLon - PlanCamOriLon;
			ShiftLonDiff = ExportCenLon - ShiftLon;
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180.0;
				if (fabs(TempLon) > 360.0)
					TempLon = fmod(TempLon, 360.0);
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				ExportCenLon = TempLon + PlanCamOriLon;
				ShiftLon = ExportCenLon - ShiftLonDiff;
				} // if
			} // if


		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Color);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Color);


//		EdgeColor[0] = (float)Color[0] * 0.5f;
//		EdgeColor[1] = (float)Color[1] * 0.5f;
//		EdgeColor[2] = (float)Color[2] * 0.5f;
//		EdgeColor[3] = Color[3];


		EdgeColor[0] = 0.0f;
		EdgeColor[1] = 0.0f;
		EdgeColor[2] = 1.0f;
		EdgeColor[3] = Color[3];

		Specular[0] = Specular[1] = Specular[2] = CMSpecular;
		Specular[2] = 1.0f;

		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &CMSpecularExp);

		if (AmIActive)
			{
			DrawSetupActiveObject(VC);
			} // if
		SetupPickName(SE);

		if (IsPlan)
			{
			if (ExportCoords)
				{
				UL.xyz[0] = LowX;
				UL.xyz[1] = HighY;
				ExportCoords->ProjToDefDeg(&UL);
				UL.XYZ[0] = UL.Lon;
				UL.XYZ[1] = UL.Lat;

				UR.xyz[0] = HighX;
				UR.xyz[1] = HighY;
				ExportCoords->ProjToDefDeg(&UR);
				UR.XYZ[0] = UR.Lon;
				UR.XYZ[1] = UR.Lat;

				LL.xyz[0] = LowX;
				LL.xyz[1] = LowY;
				ExportCoords->ProjToDefDeg(&LL);
				LL.XYZ[0] = LL.Lon;
				LL.XYZ[1] = LL.Lat;

				LR.xyz[0] = HighX;
				LR.xyz[1] = LowY;
				ExportCoords->ProjToDefDeg(&LR);
				LR.XYZ[0] = LR.Lon;
				LR.XYZ[1] = LR.Lat;
				} // if
			else
				{
				UL.XYZ[0] = LL.XYZ[0] = HighX;
				LR.XYZ[0] = UR.XYZ[0] = LowX;
				UL.XYZ[1] = UR.XYZ[1] = HighY;
				LL.XYZ[1] = LR.XYZ[1] = LowY;
				} // else
			UL.XYZ[2] = LL.XYZ[2] = UR.XYZ[2] = LR.XYZ[2] = 0.0;

			#ifdef WCS_BUILD_VNS
			if (ProjectedPlan)
				{
				// project all corners into View's CS, and replace into .XZY so code below is unchanged
				VertexDEM ProjConvert;

				// UL
				ProjConvert.Lon  = UL.XYZ[0];
				ProjConvert.Lat  = UL.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				UL.XYZ[0] = ProjConvert.XYZ[0];
				UL.XYZ[1] = ProjConvert.XYZ[1];

				// UR
				ProjConvert.Lon  = UR.XYZ[0];
				ProjConvert.Lat  = UR.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				UR.XYZ[0] = ProjConvert.XYZ[0];
				UR.XYZ[1] = ProjConvert.XYZ[1];

				// LL
				ProjConvert.Lon  = LL.XYZ[0];
				ProjConvert.Lat  = LL.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				LL.XYZ[0] = ProjConvert.XYZ[0];
				LL.XYZ[1] = ProjConvert.XYZ[1];

				// LR
				ProjConvert.Lon  = LR.XYZ[0];
				ProjConvert.Lat  = LR.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				LR.XYZ[0] = ProjConvert.XYZ[0];
				LR.XYZ[1] = ProjConvert.XYZ[1];
				} // if
			#endif // WCS_BUILD_VNS

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			glBegin(GL_TRIANGLES);
			glTriPoly(UL.XYZ, UR.XYZ, LL.XYZ);
			glTriPoly(UR.XYZ, LR.XYZ, LL.XYZ);
			glEnd();

			// draw outline
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);

			glBegin(GL_LINE_LOOP);
			glQuadOutline(UL.XYZ, UR.XYZ, LR.XYZ, LL.XYZ);
			glEnd();
			} // if
		else
			{
			YRange = (HighY - LowY);
			XRange  = (HighX - LowX);
			LatRange = fabs(HighLat - LowLat);
			LonRange  = fabs(HighLon - LowLon);
			YSegSteps = max(4, (int)LatRange);
			XSegSteps = max(4, (int)LonRange);

			// YRange and XRange are initialized to Y and X extent from above
			// Now YStep and XStep are calculated to distance per segment
			YStep = YRange / (double)YSegSteps;
			XStep  = XRange  / (double)XSegSteps;

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			// We go LR to LL, and south to north.
			for (XSeg = 0; XSeg < XSegSteps; XSeg++)
				{
				for (YSeg = 0; YSeg < YSegSteps; YSeg++)
					{
					double UX, LX, UY, LY;

					UL.Elev = LL.Elev = UR.Elev = LR.Elev = Bottom;
					UL.xyz[2] = LL.xyz[2] = UR.xyz[2] = LR.xyz[2] = Bottom;

					UY  = LowY + (double)(YSeg + 1) * YStep;
					LX  = LL.Lon  = LowX + (double)(XSeg + 1) * XStep;
					LY  = LowY + (double)YSeg * YStep; 
					UX  = LowX + (double)XSeg * XStep;

					if (!ExportCoords)
						{
						UR.Lat  = UL.Lat  = UY;
						UL.Lon  = LL.Lon  = LX;
						LL.Lat  = LR.Lat  = LY; 
						UR.Lon  = LR.Lon  = UX;
						} // if
					//DefCoords->ProjToCart(&UL);

					// UL
					#ifdef WCS_BUILD_VNS
					if (ExportCoords)
						{
						UL.xyz[0] = LX;
						UL.xyz[1] = UY;
						ExportCoords->ProjToCart(&UL);
						} // if
					else
						{
						DefCoords->DegToCart(&UL);
						} // else
					#else // WCS_BUILD_VNS
					UL.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(UL.xyz, UL.XYZ);
					AddPoint3d(UL.XYZ, VC->NegOrigin);
					InvertZ(UL.XYZ);
					InvertXYd(UL.xyz);

					// UR
					#ifdef WCS_BUILD_VNS
					if (ExportCoords)
						{
						UR.xyz[0] = UX;
						UR.xyz[1] = UY;
						ExportCoords->ProjToCart(&UR);
						} // if
					else
						{
						DefCoords->DegToCart(&UR);
						} // else
					#else // WCS_BUILD_VNS
					UR.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(UR.xyz, UR.XYZ);
					AddPoint3d(UR.XYZ, VC->NegOrigin);
					InvertZ(UR.XYZ);
					InvertXYd(UR.xyz);
					
					// LL
					#ifdef WCS_BUILD_VNS
					if (ExportCoords)
						{
						LL.xyz[0] = LX;
						LL.xyz[1] = LY;
						ExportCoords->ProjToCart(&LL);
						} // if
					else
						{
						DefCoords->DegToCart(&LL);
						} // else
					#else // WCS_BUILD_VNS
					LL.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(LL.xyz, LL.XYZ);
					AddPoint3d(LL.XYZ, VC->NegOrigin);
					InvertZ(LL.XYZ);
					InvertXYd(LL.xyz);
					
					// LR
					#ifdef WCS_BUILD_VNS
					if (ExportCoords)
						{
						LR.xyz[0] = UX;
						LR.xyz[1] = LY;
						ExportCoords->ProjToCart(&LR);
						} // if
					else
						{
						DefCoords->DegToCart(&LR);
						} // else
					#else // WCS_BUILD_VNS
					LR.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(LR.xyz, LR.XYZ);
					AddPoint3d(LR.XYZ, VC->NegOrigin);
					InvertZ(LR.XYZ);
					InvertXYd(LR.xyz);

					glBegin(GL_TRIANGLES);
					//if (!((UL.Lat > 89.999) && (UL.Lat > 89.999)))
						{
						//glTriPoly(UL.XYZ, UR.XYZ, LL.XYZ);
						glTriPolyVertexDEM(UL, UR, LL);
						} // if
					//if (!((LR.Lat < -89.999) && (LL.Lat < -89.999)))
						{
						//glTriPoly(UR.XYZ, LR.XYZ, LL.XYZ);
						glTriPolyVertexDEM(UR, LR, LL);
						} // if
					glEnd();

					// do we need to draw any edge lines?
					if ((YSeg == 0) || (XSeg == 0) || (YSeg == YSegSteps - 1) || (XSeg == XSegSteps - 1))
						{
						// draw outline
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);
						glBegin(GL_LINES);

						if (YSeg == 0) // south edge E-W
							{
							glVertex3dv(LR.XYZ);
							glVertex3dv(LL.XYZ);
							} // if
						if (XSeg == 0) // east edge S-N
							{
							glVertex3dv(LR.XYZ);
							glVertex3dv(UR.XYZ);
							} // if
						if (YSeg == YSegSteps - 1) // north edge E-W
							{
							glVertex3dv(UR.XYZ);
							glVertex3dv(UL.XYZ);
							} // if
						if (XSeg == XSegSteps - 1) // west edge S-N
							{
							glVertex3dv(LL.XYZ);
							glVertex3dv(UL.XYZ);
							} // if
						//glQuadOutline(UL.XYZ, UR.XYZ, LR.XYZ, LL.XYZ);
						glEnd();
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
						} // if

					} // for
				} // for
			} // else


		ClearPickName();
		if (AmIActive)
			{
			DrawFinishActiveObject(VC);
			} // if
		} // if
	} // for

glDisable(GL_BLEND);

if (IsPlan)
	{
	glEnable(GL_DEPTH_TEST);
	} // if

} // ViewGUI::DrawExportBounds
#endif // WCS_BUILD_RTX

/*===========================================================================*/

void ViewGUI::DrawClouds(int ViewNum, ViewContext *VC)
{
Point4f Specular, Color, EdgeColor;
Point3d BasicColor;
int DEMSteps = 0;
CloudEffect *Puffy;
double CloudBot, Density, DEMOriLon, PlanCamOriLon, TempLon, HighLonDiff, LowLonDiff;
float CloudSpec, CloudSpecExp /*, CloudIntense */;
VertexDEM NW, NE, SW, SE;
int LatSegSteps, LonSegSteps, LatSeg, LonSeg;
double North, South, East, West;
int FXIdx, IsPlan;
char AmIActive = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ProjectedPlan = NULL;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS))) return;

if (IsPlan = VC->IsPlan())
	{
	glDisable(GL_DEPTH_TEST);
	ProjectedPlan = VC->GetProjected();
	} // if

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


// Setup surface attributes

CloudBot = 0;
Density = 0.0;
CloudSpec = 0.10f;
CloudSpecExp = 64.0f;
Color[0] = 0.5f;
Color[1] = 0.5f;
Color[2] = 0.5f;
Color[3] = 1.0f;

for (FXIdx = 0, Puffy = (CloudEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD));
 Puffy; Puffy = (CloudEffect *)(Puffy->Next))
	{
	if (Puffy->Enabled)
		{
		if (RasterAnimHost::GetActiveRAHost())
			AmIActive = (Puffy == RasterAnimHost::GetActiveRAHost()) || (Puffy == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));
		Puffy->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		CloudBot     = Puffy->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].GetCurValue(0);

		if (IsPlan)
			{
			PlanCamOriLon = VC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
			DEMOriLon = (Puffy->HighLon + Puffy->LowLon) * .5;
			TempLon = DEMOriLon - PlanCamOriLon;
			HighLonDiff = DEMOriLon - Puffy->HighLon;
			LowLonDiff = DEMOriLon - Puffy->LowLon;
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180.0;
				if (fabs(TempLon) > 360.0)
					TempLon = fmod(TempLon, 360.0);
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				DEMOriLon = TempLon + PlanCamOriLon;
				Puffy->HighLon = DEMOriLon - HighLonDiff;
				Puffy->LowLon = DEMOriLon - LowLonDiff;
				} // if
			// replaced by above
			//while (DEMOriLon - PlanCamOriLon > 180.0)
			//	{
			//	DEMOriLon -= 360.0;
			//	Puffy->HighLon -= 360.0;
			//	Puffy->LowLon -= 360.0;
			//	} // while
			//while (DEMOriLon - PlanCamOriLon < -180.0)
			//	{
			//	DEMOriLon += 360.0;
			//	Puffy->HighLon += 360.0;
			//	Puffy->LowLon += 360.0;
			//	} // while
			} // if


		Density      = Puffy->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].GetCurValue(0);
		//CloudSpec    = (float)Puffy->WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
		//CloudSpecExp = (float)Puffy->WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0);

		//CloudIntense = (float)Puffy->WaterMat.DiffuseColor.GetIntensity();
		Puffy->ColorGrad.GetBasicColor(BasicColor[0], BasicColor[1], BasicColor[2], 0.25);
		Color[0] = Color[1] = Color[2] = 0.0f;
		Color[3] = 1.0f;
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Color);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Color);

		Color[0] = (float)BasicColor[0];
		Color[1] = (float)BasicColor[1];
		Color[2] = (float)BasicColor[2];
		Color[3] = 0.25f * (float)Density;

		EdgeColor[0] = (float)Color[0] * 0.5f;
		EdgeColor[1] = (float)Color[1] * 0.5f;
		EdgeColor[2] = (float)Color[2] * 0.5f;
		EdgeColor[3] = Color[3];


		Specular[0] = Specular[1] = Specular[2] = CloudSpec;
		Specular[2] = 1.0f;

		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
		ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &CloudSpecExp);

		if (AmIActive)
			{
			DrawSetupActiveObject(VC);
			} // if
		SetupPickName(Puffy);

		if (IsPlan)
			{
			NW.XYZ[2] = SW.XYZ[2] = NE.XYZ[2] = SE.XYZ[2] = 0.0;
			NW.XYZ[0] = SW.XYZ[0] = Puffy->HighLon;
			SE.XYZ[0] = NE.XYZ[0] = Puffy->LowLon;
			NW.XYZ[1] = NE.XYZ[1] = Puffy->HighLat;
			SW.XYZ[1] = SE.XYZ[1] = Puffy->LowLat;

			#ifdef WCS_BUILD_VNS
			if (ProjectedPlan)
				{
				// project all corners into View's CS, and replace into .XZY so code below is unchanged
				VertexDEM ProjConvert;

				// NW
				ProjConvert.Lon  = NW.XYZ[0];
				ProjConvert.Lat  = NW.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				NW.XYZ[0] = ProjConvert.XYZ[0];
				NW.XYZ[1] = ProjConvert.XYZ[1];

				// NE
				ProjConvert.Lon  = NE.XYZ[0];
				ProjConvert.Lat  = NE.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				NE.XYZ[0] = ProjConvert.XYZ[0];
				NE.XYZ[1] = ProjConvert.XYZ[1];

				// SW
				ProjConvert.Lon  = SW.XYZ[0];
				ProjConvert.Lat  = SW.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				SW.XYZ[0] = ProjConvert.XYZ[0];
				SW.XYZ[1] = ProjConvert.XYZ[1];

				// SE
				ProjConvert.Lon  = SE.XYZ[0];
				ProjConvert.Lat  = SE.XYZ[1];
				ProjConvert.Elev = 0.0;
				ProjectForViews(VC, &ProjConvert);
				SE.XYZ[0] = ProjConvert.XYZ[0];
				SE.XYZ[1] = ProjConvert.XYZ[1];
				} // if
			#endif // WCS_BUILD_VNS


			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			glBegin(GL_TRIANGLES);
			glTriPoly(NW.XYZ, NE.XYZ, SW.XYZ);
			glTriPoly(NE.XYZ, SE.XYZ, SW.XYZ);
			glEnd();

			// draw outline
			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);

			glBegin(GL_LINE_LOOP);
			glQuadOutline(NW.XYZ, NE.XYZ, SE.XYZ, SW.XYZ);
			glEnd();
			} // if
		else
			{
			North = fabs(Puffy->HighLat - Puffy->LowLat);
			West  = fabs(Puffy->HighLon - Puffy->LowLon);
			LatSegSteps = max(4, (int)North);
			LonSegSteps = max(4, (int)West);

			NW.Elev = SW.Elev = NE.Elev = SE.Elev = CloudBot;

			// North and West are initialized to Lat and Lon extent from above
			// Now South and East are calculated to degrees per segment
			South = North / (double)LatSegSteps;
			East  = West  / (double)LonSegSteps;

			ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
			// We go SE to SW, and south to north.
			for (LonSeg = 0; LonSeg < LonSegSteps; LonSeg++)
				{
				for (LatSeg = 0; LatSeg < LatSegSteps; LatSeg++)
					{
					NE.Lat  = NW.Lat  = Puffy->LowLat + (double)(LatSeg + 1) * South;
					NW.Lon  = SW.Lon  = Puffy->LowLon + (double)(LonSeg + 1) * East;
					SW.Lat  = SE.Lat  = Puffy->LowLat + (double)LatSeg * South; 
					NE.Lon  = SE.Lon  = Puffy->LowLon + (double)LonSeg * East;

					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NW);
					#else // WCS_BUILD_VNS
					NW.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(NW.xyz, NW.XYZ);
					AddPoint3d(NW.XYZ, VC->NegOrigin);
					InvertZ(NW.XYZ);
					InvertXYd(NW.xyz);
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&NE);
					#else // WCS_BUILD_VNS
					NE.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(NE.xyz, NE.XYZ);
					AddPoint3d(NE.XYZ, VC->NegOrigin);
					InvertZ(NE.XYZ);
					InvertXYd(NE.xyz);
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SW);
					#else // WCS_BUILD_VNS
					SW.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(SW.xyz, SW.XYZ);
					AddPoint3d(SW.XYZ, VC->NegOrigin);
					InvertZ(SW.XYZ);
					InvertXYd(SW.xyz);
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&SE);
					#else // WCS_BUILD_VNS
					SE.DegToCart(VC->GlobeRadius);
					#endif // WCS_BUILD_VNS
					UnitVectorCopy(SE.xyz, SE.XYZ);
					AddPoint3d(SE.XYZ, VC->NegOrigin);
					InvertZ(SE.XYZ);
					InvertXYd(SE.xyz);

					glBegin(GL_TRIANGLES);
					//if (!((NW.Lat > 89.999) && (NW.Lat > 89.999)))
						{
						//glTriPoly(NW.XYZ, NE.XYZ, SW.XYZ);
						glTriPolyVertexDEM(NW, NE, SW);
						} // if
					//if (!((SE.Lat < -89.999) && (SW.Lat < -89.999)))
						{
						//glTriPoly(NE.XYZ, SE.XYZ, SW.XYZ);
						glTriPolyVertexDEM(NE, SE, SW);
						} // if
					glEnd();

					// do we need to draw any edge lines?
					if ((LatSeg == 0) || (LonSeg == 0) || (LatSeg == LatSegSteps - 1) || (LonSeg == LonSegSteps - 1))
						{
						// draw outline
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, EdgeColor);
						glBegin(GL_LINES);

						if (LatSeg == 0) // south edge E-W
							{
							glVertex3dv(SE.XYZ);
							glVertex3dv(SW.XYZ);
							} // if
						if (LonSeg == 0) // east edge S-N
							{
							glVertex3dv(SE.XYZ);
							glVertex3dv(NE.XYZ);
							} // if
						if (LatSeg == LatSegSteps - 1) // north edge E-W
							{
							glVertex3dv(NE.XYZ);
							glVertex3dv(NW.XYZ);
							} // if
						if (LonSeg == LonSegSteps - 1) // west edge S-N
							{
							glVertex3dv(SW.XYZ);
							glVertex3dv(NW.XYZ);
							} // if
						//glQuadOutline(NW.XYZ, NE.XYZ, SE.XYZ, SW.XYZ);
						glEnd();
						ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);
						} // if

					} // for
				} // for
			} // else


		ClearPickName();
		if (AmIActive)
			{
			DrawFinishActiveObject(VC);
			} // if

		FXIdx ++;
		} // if
	} // for

glDisable(GL_BLEND);

if (IsPlan)
	{
	glEnable(GL_DEPTH_TEST);
	} // if

} // ViewGUI::DrawClouds

/*===========================================================================*/

void ViewGUI::DrawVariousLines(int ViewNum, ViewContext *VC)
{
Point4f Color;
char DrawImageBounds = 1, DrawLtdRegion = 1;
double WidthRatio, HeightRatio;
unsigned short Width, Height;
long RenderWidth = 0, RenderHeight = 0,
 LX = 0, UY = 0, RX = 0, LY = 0, RCX, RCY, HRW, HRH;
int DrawSafeTitle = 0, DrawSafeAction = 0;

if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION))) DrawLtdRegion = 0;
if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_SAFEAREA)))  DrawImageBounds = 0;

if (!VC->RO) DrawImageBounds = 0;

// These settings start defaulting to 0 and change to 1 if implemented
if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_SAFETITLE)))  DrawSafeTitle = 1;
if ((VC->GetEnabled(WCS_VIEWGUI_ENABLE_SAFEACTION)))  DrawSafeAction = 1;

if (!DrawImageBounds && !DrawLtdRegion && !DrawSafeTitle && !DrawSafeAction) return;

if (DrawLtdRegion) DrawImageBounds = 0;

glDisable(GL_DEPTH_TEST);
glDisable(GL_LIGHTING);

// Setup surface attributes

Color[0] = 0.5f;
Color[1] = 0.5f;
Color[2] = 0.5f;
Color[3] = 1.0f;
glColor4fv(Color);

ViewWins[ViewNum]->GetDrawingAreaSize(Width, Height);

if (DrawImageBounds)
	{
	// limit region by actual render aspect
	WidthRatio = (double)Width / VC->RO->OutputImageWidth;
	HeightRatio = (double)Height / VC->RO->OutputImageHeight;
	if (WidthRatio <= HeightRatio)
		{
		RenderWidth = Width;
		RenderHeight = (long)(min(Height, VC->RO->OutputImageHeight * WidthRatio));
		UY = (Height - RenderHeight) / 2;
		LX = -2;
		RX = Width + 2;
		LY = RenderHeight + UY;
		} // if
	else
		{
		RenderHeight = Height;
		RenderWidth = (long)(min(Width, VC->RO->OutputImageWidth * HeightRatio));
		LX = (Width - RenderWidth) / 2;
		UY = -1;
		RX = RenderWidth + LX;
		LY = Height + 1;
		} // else

	// Flip Screen Y into GL convention
	LY = Height - LY;
	UY = Height - UY;

	} // if displaying image frame


if (DrawLtdRegion)
	{
	ViewWins[ViewNum]->GetDrawingAreaSize(Width, Height);
	// limit render to partial region
	HRW = (RenderWidth = (long)(Width * VC->RenderRegion[2])) / 2;
	HRH = (RenderHeight = (long)(Height * VC->RenderRegion[3])) / 2;
	RCX = (long)(Width * VC->RenderRegion[0]);
	RCY = (long)(Height * VC->RenderRegion[1]);
	LX = RCX - HRW;
	UY = RCY - HRH;
	RX = RCX + HRW;
	LY = RCY + HRH;

	// Flip Screen Y into GL convention
	LY = Height - LY;
	UY = Height - UY;
	} // if displaying image frame

glMatrixMode(GL_MODELVIEW);
glPushMatrix(); // on MODELVIEW
glLoadIdentity(); // on MODELVIEW

glMatrixMode(GL_PROJECTION);
glPushMatrix(); // on PROJECTION
glLoadIdentity(); // on PROJECTION

gluOrtho2D(0.0, (double)Width, 0.0, (double)Height);


glBegin(GL_LINE_LOOP);
glVertex2i(LX, UY);
glVertex2i(RX, UY);
glVertex2i(RX, LY);
glVertex2i(LX, LY);
glEnd();

if (DrawSafeAction || DrawSafeTitle)
	{
	glLineWidth(5.0f);
	Color[0] = 0.5f;
	Color[1] = 0.5f;
	Color[2] = 0.75f;
	Color[3] = 0.5f;
	glColor4fv(Color);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} // if

if (DrawSafeAction)
	{
	if (DrawImageBounds)
		{
		// limit region by actual render aspect
		WidthRatio = (double)Width / VC->RO->OutputImageWidth;
		HeightRatio = (double)Height / VC->RO->OutputImageHeight;
		if (WidthRatio <= HeightRatio)
			{
			RenderWidth = Width;
			RenderHeight = (long)(min(Height, VC->RO->OutputImageHeight * WidthRatio));
			UY = (Height - RenderHeight) / 2;
			UY += (long)(RenderHeight * 0.05);
			LX = (long)((double)Width * 0.05);
			RX = (long)((double)Width * 0.95);
			LY = (long)(RenderHeight * 0.90) + UY;
			} // if
		else
			{
			RenderHeight = Height;
			RenderWidth = (long)(min(Width, VC->RO->OutputImageWidth * HeightRatio));
			LX = (Width - RenderWidth) / 2;
			LX += (long)(RenderWidth * 0.05);
			UY = (long)((double)Height * 0.05);
			RX = (long)(RenderWidth * 0.90) + LX;
			LY = (long)((double)Height * 0.95);
			} // else

		// Flip Screen Y into GL convention
		LY = Height - LY;
		UY = Height - UY;
		} // if DrawImageBounds
	else
		{
		LX = (long)((double)Width * 0.05);
		RX = (long)((double)Width * 0.95);
		UY = (long)((double)Height * 0.05);
		LY = (long)((double)Height * 0.95);
		} // else

	glBegin(GL_LINE_LOOP);
	glVertex2i(LX, UY);
	glVertex2i(RX, UY);
	glVertex2i(RX, LY);
	glVertex2i(LX, LY);
	glEnd();
	} // if

if (DrawSafeTitle)
	{
	if (DrawImageBounds)
		{
		// limit region by actual render aspect
		WidthRatio = (double)Width / VC->RO->OutputImageWidth;
		HeightRatio = (double)Height / VC->RO->OutputImageHeight;
		if (WidthRatio <= HeightRatio)
			{
			RenderWidth = Width;
			RenderHeight = (long)(min(Height, VC->RO->OutputImageHeight * WidthRatio));
			UY = (Height - RenderHeight) / 2;
			UY += (long)(RenderHeight * 0.10);
			LX = (long)((double)Width * 0.10);
			RX = (long)((double)Width * 0.90);
			LY = (long)(RenderHeight * 0.80) + UY;
			} // if
		else
			{
			RenderHeight = Height;
			RenderWidth = (long)(min(Width, VC->RO->OutputImageWidth * HeightRatio));
			LX = (Width - RenderWidth) / 2;
			LX += (long)(RenderWidth * 0.10);
			UY = (long)((double)Height * 0.10);
			RX = (long)(RenderWidth * 0.80) + LX;
			LY = (long)((double)Height * 0.90);
			} // else

		// Flip Screen Y into GL convention
		LY = Height - LY;
		UY = Height - UY;
		} // if DrawImageBounds
	else
		{
		LX = (long)((double)Width * 0.10);
		RX = (long)((double)Width * 0.90);
		UY = (long)((double)Height * 0.10);
		LY = (long)((double)Height * 0.90);
		} // else

	glBegin(GL_LINE_LOOP);
	glVertex2i(LX, UY);
	glVertex2i(RX, UY);
	glVertex2i(RX, LY);
	glVertex2i(LX, LY);
	glEnd();
	} // if

if (DrawSafeAction || DrawSafeTitle)
	{
	glLineWidth(1.0f);
	glDisable(GL_BLEND);
	} // if

glPopMatrix(); // on PROJECTION
glMatrixMode(GL_MODELVIEW);
glPopMatrix(); // on MODELVIEW

glEnable(GL_DEPTH_TEST);
glEnable(GL_LIGHTING);

} // ViewGUI::DrawVariousLines

/*===========================================================================*/

/*
void ViewGUI::DrawDEMOutline(int ViewNum, ViewContext *VC, double N, double S, double E, double W)
{
Point4f Color;
char DrawImageBounds = 1, DrawLtdRegion = 1;
double LX = 0, UY = 0, RX = 0, LY = 0;

glDisable(GL_DEPTH_TEST);
glDisable(GL_LIGHTING);
glDisable(GL_FOG);

// Setup surface attributes

glDisable(GL_TEXTURE_1D);

Color[0] = 1.0f;
Color[1] = 1.0f;
Color[2] = 0.0f;
Color[3] = 1.0f;
glColor4fv(Color);

UY = 90.0 - N;
LX = W;
RX = E;
LY = 90.0 - S;

glBegin(GL_LINE_LOOP);
glVertex2d(LX, UY);
glVertex2d(RX, UY);
glVertex2d(RX, LY);
glVertex2d(LX, LY);
glEnd();

glEnable(GL_DEPTH_TEST);
glEnable(GL_LIGHTING);

} // ViewGUI::DrawDEMOutline

*/

/*===========================================================================*/

void ViewGUI::DrawCleanup(int ViewNum, ViewContext *VC)
{
// Finish up and clean up
if (ViewWins[ViewNum])
	{
	ViewWins[ViewNum]->fglFlush();
	if (!CleanupNoSwap)
		{
		ViewWins[ViewNum]->fglSwapBuffers();
		} // if
	} // if
} // ViewGUI::DrawCleanup

/*===========================================================================*/

void ViewGUI::DrawSync(int ViewNum, ViewContext *VC)
{

// This syncs the foreground and backingstore.
if (ViewWins[ViewNum])
	{
	ViewWins[ViewNum]->SetupForDrawing();
	ViewWins[ViewNum]->CleanupFromDrawing();
	} // if

} // ViewGUI::DrawSync

/*===========================================================================*/

int ViewGUI::ReboundDynamic(int ViewNum, ViewContext *VC)
{

// Ask the regen code to just recalc bounds for us.
return(RegenDEMGeometry(ViewNum, 1.0, 0, VC, WCS_DATABASE_DYNAMIC));

} // ViewGUI::ReboundDynamic

/*===========================================================================*/

int ViewGUI::ReboundDEMGeometry(int ViewNum, unsigned long DisplayList, ViewContext *VC, unsigned long Flags,
								double &MaxEl, double &MinEl, double &CellSizeNS, double &CellSizeWE)
{
Joe *Clip;
JoeDEM *MyDEM;
JoeViewTemp *JVT;
double ElRange = 0.0, MyMax, MyMin, ElScale;
double NSDimension, WEDimension;
int DEMSteps = 0, Dynamic = 0;
//int PlanView = 0;
unsigned long TestList = 0;
VertexDEM ObjOrig;

if (Flags & WCS_DATABASE_DYNAMIC)
	Dynamic = 1;

LocalDB->ResetGeoClip();

for (Clip = LocalDB->GetFirst(Flags); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (Dynamic)
				Dynamic = 1;
			//DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP);
				if (!JVT && !Dynamic)
					{
					if (JVT = new JoeViewTemp)
						{
						Clip->AddAttribute(JVT);
						} // if
					} // if
				if (JVT)
					{
					// load only the headers
					if (Clip->AttemptLoadDEM(0, LocalProject) && MyDEM->Pristine)
						{
						JVT->MaxPolys = MyDEM->Pristine->CalcPolys();
						MyDEM->Pristine->FreeRawElevs();
						} //  if
					if (DisplayList)
						{
						#ifdef WCS_BUILD_VNS
						// projected plan
							{
							if (JVT->ProjPlanDisplayListNum)
								{
								glDeleteLists(JVT->ProjPlanDisplayListNum, 1);
								JVT->ProjPlanDisplayListNum = 0;
								} // if
							} // if
						#endif // WCS_BUILD_VNS
						//if (PlanView)
							{
							if (JVT->PlanDisplayListNum)
								{
								glDeleteLists(JVT->PlanDisplayListNum, 1);
								JVT->PlanDisplayListNum = 0;
								} // if
							} // if
						//else
							{
							if (JVT->PerspDisplayListNum)
								{
								glDeleteLists(JVT->PerspDisplayListNum, 1);
								JVT->PerspDisplayListNum = 0;
								} // if
							} // else
						} // if
					} // if
				// Load elevations now so we can get the units field for bounding
				//if (Clip->AttemptLoadDownSampled(GlobalApp->MainProj->Interactive->GetGridSample(), LocalProject))
				//if (Clip->AttemptLoadBetterDownSampled(1000, LocalProject))
					{
					if (MyDEM->Pristine)
						{
						/*
						NSDimension = (MyDEM->Pristine->Northest() - MyDEM->Pristine->Southest()) / (MyDEM->Pristine->LatEntries() - 1);
						WEDimension = (MyDEM->Pristine->Westest() - MyDEM->Pristine->Eastest()) / (MyDEM->Pristine->LonEntries() - 1);
						NSDimension *= LatScale(VC->GetRadius());
						WEDimension *= LonScale(VC->GetRadius(), (MyDEM->Pristine->Northest() + MyDEM->Pristine->Southest()) / 2.0);
						*/
						MyDEM->Pristine->GetDEMCellSizeMeters(NSDimension, WEDimension);
						if (WEDimension < CellSizeWE)
							CellSizeWE = WEDimension;
						if (NSDimension < CellSizeNS)
							CellSizeNS = NSDimension;
						ElScale = MyDEM->Pristine->pElScale;
						} // if
					else
						{
						ElScale = MyDEM->FetchBestElScale();
						} // else
					MyMax = ((double)MyDEM->MaxEl + 1.0) * (ElScale / ELSCALE_METERS);
					MyMax = VC->VCCalcExag(MyMax);

					MyMin = ((double)MyDEM->MinEl - 1.0) * (ElScale / ELSCALE_METERS);
					MyMin = VC->VCCalcExag(MyMin);

					if (MyMax > MaxEl)
						{
						MaxEl = MyMax;
						} // if
					if (MyMin < MinEl)
						{
						MinEl = MyMin;
						} // if
					} // if
				} // 
			} // if
		} // if
	} // for

return(0);

} // ViewGUI::ReboundDEMGeometry

/*===========================================================================*/

int ViewGUI::RegenDEMGeometry(int ViewNum, double PolyPercent, unsigned long DisplayList, ViewContext *VC, int JustReboundFlags)
{
Joe *Clip;
JoeDEM *MyDEM;
JoeViewTemp *JVT;
double DMaxEl = -DBL_MAX, DMinEl = DBL_MAX, SMaxEl = -DBL_MAX, SMinEl = DBL_MAX, MyMin;
double DCellSizeNS, DCellSizeWE, SCellSizeNS, SCellSizeWE;
int DEMSteps = 0, DValid = 1, SValid = 1;;
//int PlanView = 0;
unsigned long TestList = 0;
VertexDEM ObjOrig;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
CoordSys *ProjectedSys = NULL;
int DoneOneProj = 0;

DCellSizeNS = DCellSizeWE = 1000000.0;
SCellSizeNS = SCellSizeWE = 1000000.0;

//if (ViewNum != -1) PlanView = 1;

if (!VectorListsAreReserved) ReserveVectorDisplayLists();

LocalDB->ResetGeoClip();

if (!JustReboundFlags || (JustReboundFlags & WCS_DATABASE_STATIC))
	ReboundDEMGeometry(ViewNum, DisplayList, VC, WCS_DATABASE_STATIC, SMaxEl, SMinEl, SCellSizeNS, SCellSizeWE);
if (!JustReboundFlags || (JustReboundFlags & WCS_DATABASE_DYNAMIC))
	ReboundDEMGeometry(ViewNum, DisplayList, VC, WCS_DATABASE_DYNAMIC, DMaxEl, DMinEl, DCellSizeNS, DCellSizeWE);

/*
for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			//DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP);
				if (!JVT)
					{
					if (JVT = new JoeViewTemp)
						{
						Clip->AddAttribute(JVT);
						} // if
					} // if
				if (JVT)
					{
					// load only the headers
					if (Clip->AttemptLoadDEM(0, LocalProject) && MyDEM->Pristine)
						{
						JVT->MaxPolys = MyDEM->Pristine->CalcPolys();
						MyDEM->Pristine->FreeRawElevs();
						} //  if
					if (DisplayList)
						{
						//if (PlanView)
							{
							if (JVT->PlanDisplayListNum)
								{
								glDeleteLists(JVT->PlanDisplayListNum, 1);
								JVT->PlanDisplayListNum = 0;
								} // if
							} // if
						//else
							{
							if (JVT->PerspDisplayListNum)
								{
								glDeleteLists(JVT->PerspDisplayListNum, 1);
								JVT->PerspDisplayListNum = 0;
								} // if
							} // else
						} // if
					} // if
				// Load elevations now so we can get the units field for bounding
				//if (Clip->AttemptLoadDownSampled(GlobalApp->MainProj->Interactive->GetGridSample(), LocalProject))
				//if (Clip->AttemptLoadBetterDownSampled(1000, LocalProject))
					{
					if (MyDEM->Pristine)
						{
						NSDimension = (MyDEM->Pristine->Northest() - MyDEM->Pristine->Southest()) / (MyDEM->Pristine->LatEntries() - 1);
						WEDimension = (MyDEM->Pristine->Westest() - MyDEM->Pristine->Eastest()) / (MyDEM->Pristine->LonEntries() - 1);
						//LonScale = cos(((MyDEM->Pristine->Northest() + MyDEM->Pristine->Southest()) / 2.0) * PiOver180);
						//NSDimension *= (EARTHLATSCALE * 1000.0);
						//WEDimension *= (EARTHLATSCALE * LonScale * 1000.0);
						NSDimension *= LatScale(VC->GetRadius());
						//WEDimension *= (LatScale(VC->GetRadius()) * LonScale);
						WEDimension *= LonScale(VC->GetRadius(), (MyDEM->Pristine->Northest() + MyDEM->Pristine->Southest()) / 2.0);
						if (WEDimension < CellSizeWE)
							CellSizeWE = WEDimension;
						if (NSDimension < CellSizeNS)
							CellSizeNS = NSDimension;
						} // if

					ElScale = MyDEM->Pristine->pElScale;
					MyMax = (double)MyDEM->MaxEl * (ElScale / ELSCALE_METERS);
					MyMax = VC->VCCalcExag(MyMax);

					MyMin = (double)MyDEM->MinEl * (ElScale / ELSCALE_METERS);
					MyMin = VC->VCCalcExag(MyMin);

					if (MyMax > MaxEl)
						{
						MaxEl = MyMax;
						} // if
					if (MyMin < MinEl)
						{
						MinEl = MyMin;
						} // if
					} // if
				} // 
			} // if
		} // if
	} // for

ObjectUnit = 1.0;
if ((MaxEl == -DBL_MAX) || (MinEl == DBL_MAX))
	{
	HMotionScaleFactor = 1.0;
	VMotionScaleFactor = 1.0;
	// No DEMs
	return(0);
	} // if
else
	{
	HMotionScaleFactor = min(CellSizeWE, CellSizeNS);
	VMotionScaleFactor = min(CellSizeWE, CellSizeNS);
	ObjectUnit = ((CellSizeNS + CellSizeWE) / 2) * 5.0;
	} // else

ElRange = MaxEl - MinEl;

TexElMin = MinEl;
TexElMax = MaxEl;
TexElRange = TexElMax - TexElMin;

GlobalFloor = MinEl - (ElRange * 0.75);
*/

if ((DMaxEl == -DBL_MAX) || (DMinEl == DBL_MAX)) DValid = 0;
if ((SMaxEl == -DBL_MAX) || (SMinEl == DBL_MAX)) SValid = 0;

ObjectUnit = 1.0;
if (!DValid && !SValid)
	{
	// No DEMs
	double DBN, DBS, DBE, DBW;
	HMotionScaleFactor = 1.0;
	VMotionScaleFactor = 1.0;
	LocalDB->GetBounds(DBN, DBS, DBE, DBW);

	if ((DBE == 180) && (DBW == -180) && (DBN == -90) && (DBS == 90))
		return(0);

	// get NS and EW range
	DBW -= DBE;
	DBN -= DBS;

	//reuse DBE to hold average N/S
	DBE = (DBN + DBS) / 2;

	// make NS and EW range positive
	DBW = fabs(DBW);
	DBN = fabs(DBN);

	// Convert NS and EW ranges from geographic units to linear units (approx)
	DBW *= LonScale(GetRadius(), DBE); // DBE has average latitude
	DBN *= LatScale(GetRadius());

	// Divide by 300 on the harebrained assumption that vector data
	// would be turned into a 300x300 DEM, thereby approximating the same
	// responsiveness you'll get after gridding.
	HMotionScaleFactor = (DBW / 300.0);
	VMotionScaleFactor = (DBN / 300.0);

	ObjectUnit = ((HMotionScaleFactor + VMotionScaleFactor) / 2) * 5.0;

	return(0);
	} // if
else
	{
	HMotionScaleFactor = min(min(DCellSizeWE, DCellSizeNS), min(SCellSizeWE, SCellSizeNS));
	VMotionScaleFactor = min(min(DCellSizeWE, DCellSizeNS), min(SCellSizeWE, SCellSizeNS));
	ObjectUnit = ((min(DCellSizeNS, SCellSizeNS) + min(DCellSizeWE, SCellSizeWE)) / 2) * 5.0;

	if (DValid)
		{
		DynTexElMin = DMinEl;
		DynTexElMax = DMaxEl;
		DynTexElRange = DynTexElMax - DynTexElMin;
		DynGlobalFloor = DMinEl - max((DynTexElRange * 0.75), 25);
		} // if
	if (SValid)
		{
		StaticTexElMin = SMinEl;
		StaticTexElMax = SMaxEl;
		StaticTexElRange = StaticTexElMax - StaticTexElMin;
		StaticGlobalFloor = SMinEl - max((StaticTexElRange * 0.75), 25);
		} // if

	if (DValid && SValid)
		{
		GlobalFloor = min(DynGlobalFloor, StaticGlobalFloor);
		TexElMin = min(DynTexElMin, StaticTexElMin);
		TexElMax = max(DynTexElMax, StaticTexElMax);
		} // if
	else
		{
		if (DValid)
			{
			GlobalFloor = DynGlobalFloor;
			TexElMin = DynTexElMin;
			TexElMax = DynTexElMax;
			} // if
		else // if (SValid)
			{
			GlobalFloor = StaticGlobalFloor;
			TexElMin = StaticTexElMin;
			TexElMax = StaticTexElMax;
			} // else
		} // if
	TexElRange = TexElMax - TexElMin;
	} // else

if (JustReboundFlags) return(1);

#ifdef WCS_BUILD_VNS
// figure out the coordsys we will use for projected, and disable any duplicate projected views
for (int ViewProjLoop = 0; ViewProjLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewProjLoop++)
	{
	if (ViewSpaces[ViewProjLoop] && ViewWins[ViewProjLoop])
		{
		if (ViewSpaces[ViewProjLoop]->GetProjected() && ViewSpaces[ViewProjLoop]->GetAlive())
			{
			if (DoneOneProj)
				{
				// only one projected view alive at a time, disable others
				ViewSpaces[ViewProjLoop]->SetAlive(0);
				ConfigureTitle(ViewProjLoop);
				} // if
			else
				{
				DoneOneProj = 1;
				ProjectedSys = ViewSpaces[ViewProjLoop]->GetProjected();
				} // else
			} // if
		} // if
	} // for
#endif // WCS_BUILD_VNS

for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			DEMSteps++;

			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if (JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP))
					{
					LoadOneDEM(ViewNum, PolyPercent, VC, Clip, JVT);

					if (DisplayList)
						{
						// Calculate object local origin (MinEl at SE corner)
						MyMin = MyDEM->MinEl * (MyDEM->FetchBestElScale() / ELSCALE_METERS);
						MyMin = VC->VCCalcExag(MyMin);
						ObjOrig.Lat  = Clip->SELat;
						ObjOrig.Lon  = Clip->SELon;
						ObjOrig.Elev = MyMin;
						#ifdef WCS_BUILD_VNS
						DefCoords->DegToCart(&ObjOrig);
						#else // WCS_BUILD_VNS
						ObjOrig.DegToCart(GlobeRadius);
						#endif // WCS_BUILD_VNS
						// Negate into subtractive form
						NegateVector(ObjOrig.XYZ);
					
						//if (VC && VC->IsPlan())
							{
							if (TestList = glGenLists(1))
								{
								glNewList(TestList, GL_COMPILE);
								JVT->PlanDisplayListNum = TestList;
								GenOneDEM(1, ObjOrig.XYZ, VC, Clip, MyDEM->ViewPref);
								glEndList();
								} // if
							} // if
						//else
							{
							if (TestList = glGenLists(1))
								{
								//LoadOneDEM(ViewNum, PolyPercent, VC, Clip, JVT);
								glNewList(TestList, GL_COMPILE);
								JVT->PerspDisplayListNum = TestList;
								GenOneDEM(0, ObjOrig.XYZ, VC, Clip, MyDEM->ViewPref);
								glEndList();
								} // if
							} // else
						// projected plan
#ifdef WCS_BUILD_VNS
							{
							if (TestList = glGenLists(1))
								{
								//LoadOneDEM(ViewNum, PolyPercent, VC, Clip, JVT);
								glNewList(TestList, GL_COMPILE);
								JVT->ProjPlanDisplayListNum = TestList;
								GenOneDEM(1, ObjOrig.XYZ, VC, Clip, MyDEM->ViewPref, 0, ProjectedSys);
								glEndList();
								} // if
							} //
#endif // WCS_BUILD_VNS
						FreeDEM(1, ObjOrig.XYZ, VC, Clip, JVT, MyDEM);
						} // if
					else
						{
						GenOneDEM(0, VC->NegOrigin, VC, Clip, MyDEM->ViewPref, 0, VC->GetProjected());
						FreeDEM(0, VC->NegOrigin, VC, Clip, JVT, MyDEM);
						} // if
					} // if
				} // if
			} // if
		} // if
	} // for

return(DEMSteps);

} // ViewGUI::RegenDEMGeometry

/*===========================================================================*/

int ViewGUI::LoadOneDEM(int ViewNum, double PolyPercent, ViewContext *VC, Joe *Clip, JoeViewTemp *JVT)
{
int DEMPolys;

DEMPolys = (int)((double)JVT->MaxPolys * PolyPercent);
return(Clip->AttemptLoadBetterDownSampled(DEMPolys, LocalProject));

} // ViewGUI::LoadOneDEM

/*===========================================================================*/

int ViewGUI::FreeDEM(int ViewNum, Point3d UseOrigin, ViewContext *VC, Joe *Clip, JoeViewTemp *JVT, JoeDEM *MyDEM)
{

MyDEM->ViewPref->FreeVertices();
delete MyDEM->ViewPref;
MyDEM->ViewPref = NULL;

return(0);

} // ViewGUI::FreeDEM

/*===========================================================================*/

int ViewGUI::GenOneDEM(int ViewNum, Point3d UseOrigin, ViewContext *VC, Joe *Clip, DEM *Terrain, int ForceTFXPreviewOff, CoordSys *ProjectedSys)
{
double ElScaleMult, DEMTexElMin, DEMTexElMax, DEMTexElRange;
double LatRange, LonRange, South, East;
double invDEMTexElRange;	//invLatRange, invLonRange
//double NClamp, SClamp, EClamp, WClamp;
Point3d TexCoord;
GeoBounds GeoClip;
VertexDEM UnRot;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
unsigned long LastVertA, LastVertB, NextBaseVtx, LonCt, LatCt, BaseVtx, Ct, 
	TerrainLonEntries, TerrainLatEntries, limit = Terrain->LatEntries() * Terrain->LonEntries();
int Stripping;
bool adjustElev = false;
char LastVertValid, IsPlan, IsProj;
JoeDEM *MyDEM;

#ifdef WCS_VIEW_TERRAFFECTORS
VertexData VTD;
PolygonData PGD;
RenderData Rend(NULL);
int PreviewTFX = 0;

if (GlobalApp->MainProj->Interactive->GetTfxPreview()) PreviewTFX = 1;

// Terraffector preview blitzes Dynamic objects because it doesn't see them.
if (ForceTFXPreviewOff)
	{
	PreviewTFX = 0;
	} // if

#endif // WCS_VIEW_TERRAFFECTORS

//IsPlan = VC->IsPlan();
IsPlan = ViewNum;
if (ProjectedSys)
	{
	IsProj = 1;
	} // if
else
	{
	IsProj = 0;
	} // else

// This 'fix' is projection-aware
// It doesn't solve the seams problem on DEM edges in projected data, but that's a bigger issue
South = Clip->SELat;
East  = Clip->SELon;
LatRange = (Clip->NWLat - Clip->SELat);
if (Terrain->LatEntries()) // prevent /0 errors
	{
	LatRange += .5 * (LatRange / (Terrain->LatEntries())); // one extra LatStep worth (more or less)
	} // if
LonRange = (Clip->NWLon - Clip->SELon);


#ifdef WCS_VIEW_TERRAFFECTORS
// Setup for terraffector eval
if (PreviewTFX)
	{
	Rend.PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
	Rend.EarthLatScaleMeters = LatScale(Rend.PlanetRad);
	Rend.EffectsBase = GlobalApp->AppEffects;
	Rend.ElevDatum = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
	Rend.Exageration = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
	Rend.DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
	Rend.TexRefLon = InterStash->GetProjRefCoords(WCS_INTERVEC_COMP_X);
	Rend.TexRefLat = InterStash->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
	Rend.TexRefElev = InterStash->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
	Rend.RefLonScaleMeters = Rend.EarthLatScaleMeters * cos(Rend.TexRefLat * PiOver180);
	Rend.TexData.MetersPerDegLat = Rend.EarthLatScaleMeters;
	Rend.TexData.Datum = Rend.ElevDatum;
	Rend.TexData.Exageration = Rend.Exageration;
	Rend.ExagerateElevLines = VC->Planet->EcoExageration;
	Rend.DBase = LocalDB;
	if (! GlobalApp->AppEffects->SetTfxGeoClip(&GeoClip, GlobalApp->AppDB, Rend.EarthLatScaleMeters))
		PreviewTFX = 0;
	} // if
#endif // WCS_VIEW_TERRAFFECTORS

if (Terrain && Terrain->LonEntries() && Terrain->LatEntries() && (LatRange > 0.0) && (LonRange > 0.0))
	{
/*	DEMTexElMax = ElevDatum + ((double)MyDEM->MaxEl - ElevDatum) * Exageration;
	DEMTexElMin = ElevDatum + ((double)MyDEM->MinEl - ElevDatum) * Exageration;
	DEMTexElRange = DEMTexElMax - DEMTexElMin; */
	DEMTexElMax = TexElMax;
	DEMTexElMin = TexElMin;
	DEMTexElRange = TexElRange;
	// error handling -- if we're the only DEM in the project, we need to set the ranges to be us
	if (TexElMax == -DBL_MAX && TexElMin == DBL_MAX && TexElRange == 1.0)
		{
		DEMTexElMax = Terrain->MaxEl();
		DEMTexElMin = Terrain->MinEl();
		DEMTexElRange = DEMTexElMax - DEMTexElMin;
		} // if
	
	if (DEMTexElRange == 0.0)
		DEMTexElRange = 1.0; // no matter

	// compute inverses so we can multiply instead of divide
	//invLatRange = 1.0 / LatRange;
	//invLonRange = 1.0 / LonRange;
	invDEMTexElRange = 1.0 / DEMTexElRange;

	// Ensure that the point remains within the DEM after whatever precision errors occur during coord transforms
	// so that VertexDataPoint will find a valid elevation
	Terrain->TransferToVerticesLatLon(TRUE);
	ElScaleMult = (Terrain->ElScale() / ELSCALE_METERS);
	// precision errors cause FindAndLoadDEM to flush too often
	// This problem is believed fixed by ensuring the vertices fall within the DEM bounds in TransferToVerticesLatLon()
	//NClamp = Terrain->Northest();
	//SClamp = Terrain->Southest();
	//EClamp = Terrain->Eastest();
	//WClamp = Terrain->Westest();

	// Project vertices into Cartesian World space
	if ((ElScaleMult != 1.0) || (VC->VCCalcExag(2.0) != 2.0))
		adjustElev = true;
	for (Ct = 0; Ct < limit; ++Ct)
		{
		// adjust elevation
		#ifdef WCS_VIEW_TERRAFFECTORS
		if (PreviewTFX && GeoClip.PointContained(Terrain->Vertices[Ct].Lat, Terrain->Vertices[Ct].Lon))
			{
			VTD.Lat = Terrain->Vertices[Ct].Lat;
			VTD.Lon = Terrain->Vertices[Ct].Lon;
			//if (VTD.Lat < SClamp)
			//	VTD.Lat = SClamp;
			//if (VTD.Lat > NClamp)
			//	VTD.Lat = NClamp;
			//if (VTD.Lon < EClamp)
			//	VTD.Lon = EClamp;
			//if (VTD.Lon > WClamp)
			//	VTD.Lon = WClamp;
			InterStash->VertexDataPoint(&Rend, &VTD, &PGD,
			 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
			Terrain->Vertices[Ct].Elev = VTD.Elev;
			//CopyPoint3d(Terrain->Vertices[Ct].XYZ, VTD.XYZ);
			} // if
		else
		#endif // !WCS_VIEW_TERRAFFECTORS
			{
			if (adjustElev)
				{
				Terrain->Vertices[Ct].Elev = Terrain->Vertices[Ct].Elev * ElScaleMult;
				Terrain->Vertices[Ct].Elev = VC->VCCalcExag(Terrain->Vertices[Ct].Elev);
				} // if
			} // else
		
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&Terrain->Vertices[Ct]);
		#else // WCS_BUILD_VNS
		Terrain->Vertices[Ct].DegToCart(GlobeRadius);
		#endif // WCS_BUILD_VNS

		// Adjust by apropriate origin
		AddPoint3d(Terrain->Vertices[Ct].XYZ, UseOrigin);
		InvertZ(Terrain->Vertices[Ct].XYZ);

		// clear .xyz for use as normal
		ZeroPoint3d(Terrain->Vertices[Ct].xyz);
		} // for

	// Calculate summed normals
	LastVertA = LastVertB = 0;	// for Lint's benefit
	for (Ct = LonCt = 0; LonCt < Terrain->LonEntries() - 1; ++LonCt)
		{
		int NPoleCount, SPoleCount;
		LastVertValid = 0;
		BaseVtx = LonCt * Terrain->LatEntries();
		NextBaseVtx = BaseVtx + Terrain->LatEntries();
		for (LatCt = 0; LatCt < Terrain->LatEntries(); ++Ct, ++LatCt)
			{
			Point3d PolyNorm;
			if (LastVertValid)
				{
				NPoleCount = SPoleCount = 0;
				if (Terrain->Vertices[LastVertA].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[LastVertB].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[BaseVtx + LatCt].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[LastVertA].Lat < -89.999) ++SPoleCount;
				if (Terrain->Vertices[LastVertB].Lat < -89.999) ++SPoleCount;
				if (Terrain->Vertices[BaseVtx + LatCt].Lat < -89.999) ++SPoleCount;

				if ((SPoleCount < 2) && (NPoleCount< 2))
					{
					// Generate normal for polygon (order is very important)
					MakeNormal(PolyNorm,
					 Terrain->Vertices[LastVertA].XYZ,
					 Terrain->Vertices[LastVertB].XYZ,
					 Terrain->Vertices[BaseVtx + LatCt].XYZ);

					// Add normal to all vertices addressed by polygon
					AddPoint3d(Terrain->Vertices[LastVertA].xyz, PolyNorm);
					AddPoint3d(Terrain->Vertices[LastVertB].xyz, PolyNorm);
					AddPoint3d(Terrain->Vertices[BaseVtx + LatCt].xyz, PolyNorm);
					} // if

				NPoleCount = SPoleCount = 0;
				if (Terrain->Vertices[NextBaseVtx + LatCt].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[BaseVtx + LatCt].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[LastVertB].Lat > 89.999) ++NPoleCount;
				if (Terrain->Vertices[NextBaseVtx + LatCt].Lat < -89.999) ++SPoleCount;
				if (Terrain->Vertices[BaseVtx + LatCt].Lat < -89.999) ++SPoleCount;
				if (Terrain->Vertices[LastVertB].Lat < -89.999) ++SPoleCount;

				if ((SPoleCount < 2) && (NPoleCount < 2))
					{
					// Generate normal for polygon (order is very important)
					MakeNormal(PolyNorm,
					 Terrain->Vertices[NextBaseVtx + LatCt].XYZ,
					 Terrain->Vertices[BaseVtx + LatCt].XYZ,
					 Terrain->Vertices[LastVertB].XYZ);

					// Add normal to all vertices addressed by polygon
					AddPoint3d(Terrain->Vertices[LastVertB].xyz, PolyNorm);
					AddPoint3d(Terrain->Vertices[NextBaseVtx + LatCt].xyz, PolyNorm);
					AddPoint3d(Terrain->Vertices[BaseVtx + LatCt].xyz, PolyNorm);
					} // if
				} // if
			LastVertA = BaseVtx + LatCt;
			LastVertB = NextBaseVtx + LatCt;
			LastVertValid = 1;
			} // for
		} // for

	// Renormalize normals
#pragma omp parallel
	{
	#pragma omp for
	for (long sCt = 0; sCt < (signed)limit; ++sCt)
		{
		UnitVector(Terrain->Vertices[Ct].xyz);
		} // for
	}

	// need these for texmapping coords
	int TexMapWidth = 0, TexMapHeight = 0;
	if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		CalcTexMapSize(MyDEM, TexMapWidth, TexMapHeight);
		} // if
	TerrainLonEntries = Terrain->LonEntries();
	TerrainLatEntries = Terrain->LatEntries();
	// Make geometry
	double TexCoordZeroA, TexCoordZeroB; // loop-hoised TexCoords for first and second vertices (below)
	for (Ct = LonCt = 0; LonCt < Terrain->LonEntries() - 1; ++LonCt)
		{
		double DEMOriLat, DEMOriLon;
		int BaseVtxReject, NextBaseVtxReject;

		DEMOriLat = (Clip->SELat + Clip->NWLat) * 0.5;
		DEMOriLon = (Clip->SELon + Clip->NWLon) * 0.5;
		glBegin(GL_TRIANGLE_STRIP);
		Stripping = 1;
		BaseVtx = LonCt * Terrain->LatEntries();
		NextBaseVtx = BaseVtx + Terrain->LatEntries();
		
		// loop-hoisted TexCoords from inside LatCt loop below
		TexCoordZeroA = (double)LonCt / (TerrainLonEntries - 1);
		TexCoordZeroA = 1.0 - (.5 + TexCoordZeroA * (TexMapWidth - 1)) / TexMapWidth;
		TexCoordZeroB = (double)(LonCt + 1) / (TerrainLonEntries - 1);
		TexCoordZeroB = 1.0 - (.5 + TexCoordZeroB * (TexMapWidth - 1)) / TexMapWidth;
		
		for (LatCt = 0; LatCt < Terrain->LatEntries(); ++Ct, ++LatCt)
			{
			BaseVtxReject     = Terrain->TestNullReject(BaseVtx + LatCt);
			NextBaseVtxReject = Terrain->TestNullReject(NextBaseVtx + LatCt);
			if (BaseVtxReject || NextBaseVtxReject)
				{ // both vertices fail the test. End tristrip.
				if (Stripping)
					{
					glEnd();
					Stripping = 0;
					} // if
				} // if
			else
				{
				if (!Stripping)
					{
					glBegin(GL_TRIANGLE_STRIP);
					Stripping = 1;
					} // if
				} // else
			if (Stripping)
				{
				// VNS 2 code replaced 10/15/08 to match tex map generation code - GRH
				//TexCoord[0] = (Terrain->Vertices[BaseVtx + LatCt].Lon - East)  * invLonRange; // lon
				//TexCoord[1] = (Terrain->Vertices[BaseVtx + LatCt].Lat - South) * invLatRange; // lat
				// calculate terrain UV of point
				//TexCoord[0] = (double)LonCt / (TerrainLonEntries - 1); // lon (1/29/09 CXH hoisted to TexCoordZeroA outside LatCt loop at F2's suggestion)
				TexCoord[1] = (double)LatCt / (TerrainLatEntries - 1); // lat
				// convert terrain UV to image UV
				//TexCoord[0] = 1.0 - (.5 + TexCoord[0] * (TexMapWidth - 1)) / TexMapWidth; (1/29/09 CXH hoisted to TexCoordZeroA outside LatCt loop at F2's suggestion)
				TexCoord[0] = TexCoordZeroA;
				TexCoord[1] = (.5 + TexCoord[1] * (TexMapHeight - 1)) / TexMapHeight;
				TexCoord[2] = (Terrain->Vertices[BaseVtx + LatCt].Elev - DEMTexElMin) * invDEMTexElRange;
				glTexCoord3dv(TexCoord);
				if (IsPlan)
					{
					#ifdef WCS_BUILD_VNS
					if (ProjectedSys)
						{
						glNormal3dv((const double *)Terrain->Vertices[BaseVtx + LatCt].xyz);
						// need to briefly reproject the vertex into projected space
						// and subtract local offset, found in DBCenter
						// <<<>>> note to self, changing vectors, or creating them outside the existing bounds
						// may alter DBCenter without causing a DEM regen, resulting in misaligned
						// projected DEMs. Not much I can think of to solve this at the moment. Bleeding edge.
						
						// we're using .xyz for normals, so we need a temp copy for projecting
						VertexDEM ProjectedVtx;
						ProjectedVtx.Lat  = Terrain->Vertices[BaseVtx + LatCt].Lat;
						ProjectedVtx.Lon  = Terrain->Vertices[BaseVtx + LatCt].Lon;
						ProjectedVtx.Elev = Terrain->Vertices[BaseVtx + LatCt].Elev;
						ProjectedSys->DefDegToProj(&ProjectedVtx);
						
						// Adjust by local origin
						ProjectedVtx.xyz[0] -= DBCenter.XYZ[0];
						ProjectedVtx.xyz[1] -= DBCenter.XYZ[1];

						if (ProjectedSys->GetGeographic())
							{
							// not sure just yet
							/*
							Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
							TempLon = Vert->ScrnXYZ[0];
							if (fabs(TempLon) > 180.0)
								{
								TempLon += 180.0;
								if (fabs(TempLon) > 360.0)
									TempLon = fmod(TempLon, 360.0);	// retains the original sign
								if (TempLon < 0.0)
									TempLon += 360.0;
								TempLon -= 180.0;
								Vert->ScrnXYZ[0] = TempLon;
								} // if
							Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
							Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
							*/
							} // if

						glVertex3d(
							ProjectedVtx.xyz[0],
							ProjectedVtx.xyz[1],
							ProjectedVtx.Elev);
						} // if
					else
					#endif // WCS_BUILD_VNS
						{
						glNormal3dv((const double *)Terrain->Vertices[BaseVtx + LatCt].xyz);
						glVertex3d(
							Terrain->Vertices[BaseVtx + LatCt].Lon - DEMOriLon,
							Terrain->Vertices[BaseVtx + LatCt].Lat - DEMOriLat,
							Terrain->Vertices[BaseVtx + LatCt].Elev);
						} // else
					} // if
				else
					{
					glNormal3dv((const double *)Terrain->Vertices[BaseVtx + LatCt].xyz);
					glVertex3d(
						Terrain->Vertices[BaseVtx + LatCt].XYZ[0],
						Terrain->Vertices[BaseVtx + LatCt].XYZ[1],
						Terrain->Vertices[BaseVtx + LatCt].XYZ[2]); 
					} // else
				// VNS 2 code replaced 10/15/08 to match tex map generation code - GRH
				//TexCoord[0] = (Terrain->Vertices[NextBaseVtx + LatCt].Lon - East)  * invLonRange; // lon
				//TexCoord[1] = (Terrain->Vertices[NextBaseVtx + LatCt].Lat - South) * invLatRange; // lat
				// calculate terrain UV of point
				//TexCoord[0] = (double)(LonCt + 1) / (TerrainLonEntries - 1); // lon (1/29/09 CXH hoisted to TexCoordZeroA outside LatCt loop at F2's suggestion)
				// convert terrain UV to image UV
				//TexCoord[0] = 1.0 - (.5 + TexCoord[0] * (TexMapWidth - 1)) / TexMapWidth; // (1/29/09 CXH hoisted to TexCoordZeroA outside LatCt loop at F2's suggestion)
				TexCoord[0] = TexCoordZeroB;
				TexCoord[2] = (Terrain->Vertices[NextBaseVtx + LatCt].Elev - DEMTexElMin) * invDEMTexElRange;
				glTexCoord3dv(TexCoord);
				if (IsPlan)
					{
					#ifdef WCS_BUILD_VNS
					if (ProjectedSys)
						{
						glNormal3dv((const double *)Terrain->Vertices[NextBaseVtx + LatCt].xyz);
						// need to briefly reproject the vertex into projected space
						// and subtract local offset, found in DBCenter
						// <<<>>> note to self, changing vectors, or creating them outside the existing bounds
						// may alter DBCenter without causing a DEM regen, resulting in misaligned
						// projected DEMs. Not much I can think of to solve this at the moment. Bleeding edge.

						VertexDEM ProjectedVtx;
						ProjectedVtx.Lat  = Terrain->Vertices[NextBaseVtx + LatCt].Lat;
						ProjectedVtx.Lon  = Terrain->Vertices[NextBaseVtx + LatCt].Lon;
						ProjectedVtx.Elev = Terrain->Vertices[NextBaseVtx + LatCt].Elev;
						ProjectedSys->DefDegToProj(&ProjectedVtx);
						
						// Adjust by local origin
						ProjectedVtx.xyz[0] -= DBCenter.XYZ[0];
						ProjectedVtx.xyz[1] -= DBCenter.XYZ[1];
						
						if (ProjectedSys->GetGeographic())
							{
							// not sure just yet
							/*
							Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
							TempLon = Vert->ScrnXYZ[0];
							if (fabs(TempLon) > 180.0)
								{
								TempLon += 180.0;
								if (fabs(TempLon) > 360.0)
									TempLon = fmod(TempLon, 360.0);	// retains the original sign
								if (TempLon < 0.0)
									TempLon += 360.0;
								TempLon -= 180.0;
								Vert->ScrnXYZ[0] = TempLon;
								} // if
							Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
							Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
							*/
							} // if

						glVertex3d(
							ProjectedVtx.xyz[0],
							ProjectedVtx.xyz[1],
							ProjectedVtx.Elev);
						} // if
					else
					#endif // WCS_BUILD_VNS
						{
						glNormal3dv((const double *)Terrain->Vertices[NextBaseVtx + LatCt].xyz);
						glVertex3d(
							Terrain->Vertices[NextBaseVtx + LatCt].Lon - DEMOriLon,
							Terrain->Vertices[NextBaseVtx + LatCt].Lat - DEMOriLat,
							Terrain->Vertices[NextBaseVtx + LatCt].Elev);
						} // if
					} // if
				else
					{
					glNormal3dv((const double *)Terrain->Vertices[NextBaseVtx + LatCt].xyz);
					glVertex3d(
						Terrain->Vertices[NextBaseVtx + LatCt].XYZ[0],
						Terrain->Vertices[NextBaseVtx + LatCt].XYZ[1],
						Terrain->Vertices[NextBaseVtx + LatCt].XYZ[2]);
					} // else
				} // if
			} // for
		if (Stripping)
			{
			glEnd();
			} // if
		} // for
	} // if

return(0);

} // ViewGUI::GenOneDEM

/*===========================================================================*/

int ViewGUI::RegenVectorGeometry(int ViewNum, long Simplify, unsigned long DisplayList, ViewContext *VC, CoordSys *ProjectedSys)
{
double ElRange = 0.0;
VertexDEM Node;
Joe *Clip;
JoeLake *JLake;
LakeEffect *Lee, *CurLake, *FirstLake;
Point4f None, Color;
float Shine;
int Steps = 0, DisplayListNum, PlanView = 0;
unsigned long PolyLimit;
unsigned char Matches, DoIt, LakeIt;

PrepForRedraw();	// doesn't seem necessary but doesn't hurt much

#ifdef WCS_BUILD_VNS
// need to project DBCenter for use in projected plan lists
if (VC && VC->GetCamera() && VC->GetCamera()->Projected && VC->GetCamera()->Coords)
	{
	CoordSys *ProjCoords = VC->GetCamera()->Coords;
	VertexDEM TempVec;

	// DBCenter was set up in PrepForRedraw(), above
	TempVec.Lat = DBCenter.Lat;
	TempVec.Lon = DBCenter.Lon;
	TempVec.Elev = DBCenter.Elev;
	ProjCoords->DefDegToProj(&TempVec);
	// move into projected vars for DBCenter
	DBCenter.XYZ[0] = TempVec.xyz[0];
	DBCenter.XYZ[1] = TempVec.xyz[1];
	} // if
#endif // WCS_BUILD_VNS

if (Simplify == -1)
	{
	PolyLimit = GlobalApp->MainProj->Interactive->GetGridSample();
	if (PolyLimit < 10000)
		Simplify = 5;
	else if (PolyLimit < 20000)
		Simplify = 4;
	else if (PolyLimit < 40000)
		Simplify = 3;
	else if (PolyLimit < 60000)
		Simplify = 2;
	else
		Simplify = 0;
	} // if

TempVec = NULL;

if (ViewNum != -1) PlanView = 1;
if (!VectorListsAreReserved) ReserveVectorDisplayLists();

LocalDB->ResetGeoClip();

// Setup default surface attributes
None[0] = 0.0f;
None[1] = 0.0f;
None[2] = 0.0f;
None[3] = 1.0f;

Shine = 0.0f;

Color[3] = 1.0f;

FirstLake = (LakeEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LAKE);
while (FirstLake)
	{
	if (FirstLake->Search && FirstLake->Search->Enabled)
		break;
	FirstLake = (LakeEffect *)FirstLake->Next;
	} // while

for (DisplayListNum = WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS; DisplayListNum < WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX; DisplayListNum++)
	{
	Lee = NULL;
	JLake = NULL;
	// begin apropriate list
	if (DisplayList)
		{
		if (PlanView)
			{
			#ifdef WCS_BUILD_VNS
			if (ProjectedSys)
				{
				glNewList(ProjPlanVectorLists[DisplayListNum], GL_COMPILE);
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				glNewList(PlanVectorLists[DisplayListNum], GL_COMPILE);
				} // else
			} // if
		else
			{
			glNewList(PerspVectorLists[DisplayListNum], GL_COMPILE);
			} // else
		} // if
	else
		{
		switch(DisplayListNum)
			{
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS:				if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_PLAINVEC))) continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CONTROLPOINTS)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJ)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX:			if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_AREATERRAFX)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP:			if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CMAPS)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_ECOFX)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_GROUND)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE:			if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_LAKES)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAINSHADOWS) || VC->GetEnabled(WCS_VIEWGUI_ENABLE_3DOBJSHADOWS)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW:			if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_SNOW)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_STREAMS)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX:			if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_TERRAFX)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_WAVES)))  continue;
				break;
			case WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS:		if (!(VC->GetEnabled(WCS_VIEWGUI_ENABLE_CLOUDS)))  continue;
				break;
			} // switch
		} // else

	//glDisable(GL_LINE_STIPPLE);
	//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, None);
	//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &Shine);
	for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
			{
			LakeIt = Matches = DoIt = 0;
			// Probably a more efficient way to do this...
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_OBJECT3D))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX) DoIt = 1;
				} // if
// CMaps not vector-bounded
/*
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_CMAP))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP) DoIt = 1;
				} // if
*/
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_FOLIAGE))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_GROUND))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND) DoIt = 1;
				} // if
// no more illumination effects
/*
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ILLUM) DoIt = 1;
				} // if
*/
			if (JLake = (JoeLake *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_LAKE))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE)
					{
					DoIt = 1;
					if (JLake->Lake->Enabled)
						{
						LakeIt = 1;
						Lee = JLake->Lake;
						} // if
					} // if
				} // if
			else
				{
				for (CurLake = FirstLake; CurLake; CurLake = (LakeEffect *)CurLake->Next)
					{
					if (CurLake->Search && CurLake->Search->Enabled)
						{
						if (CurLake->Search->OneFilterValid())
							{
							if (CurLake->Search->ApproveJoe(Clip))
								{
								Matches++;
								if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE)
									{
									DoIt = 1;
									if (CurLake->Enabled)
										{
										LakeIt = 1;
										Lee = CurLake;
										break;
										} // if
									} // if
								} // if
							} // if
						} // if
					} // for
				} // else
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SHADOW))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_SNOW))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_STREAM))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX) DoIt = 1;
				} // if
// Terrain params not vector-bounded
/*
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TERRAINPAR) DoIt = 1;
				} // if
*/
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_WAVE))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES) DoIt = 1;
				} // if
			if (Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_CLOUD))
				{
				Matches++;
				if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS) DoIt = 1;
				} // if
			if (!Matches)
				{ // no attributes, it's a plain vector or control point
				if (Clip->TestFlags(WCS_JOEFLAG_ISCONTROL))
					{
					if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS) DoIt = 1;
					} // if
				else
					{
					if (DisplayListNum == WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS) DoIt = 1;
					} // else
				} // if
	
			if (DoIt)
				{
				if (BGRedraw)
					{
					DrawSetupActiveObject(VC);
					} // if
				// pass DBCenter coords
				GenOneVector(PlanView, Simplify, VC, Clip, &DBCenter, 0, ProjectedSys);
				if (LakeIt)
					{
					GenOneLake(PlanView, Simplify, VC, Clip, Lee, &DBCenter, ProjectedSys);
					} // if
				if (BGRedraw)
					{
					DrawFinishActiveObject(VC);
					} // if
				} // if
			} // if
		} // for

	if (DisplayList)
		{
		glEndList();
		} // if
	} // for

return(Steps);

} // ViewGUI::RegenVectorGeometry

/*===========================================================================*/

int ViewGUI::GenOneVector(int PlanView, long Simplify, ViewContext *VC, Joe *Clip, VertexDEM *OriginVert, int AsPoints, CoordSys *ProjectedSys)
{
Point4f Color, Partial;
VertexDEM Node;
VectorPoint *Point;
int Steps = 0;
float LineWidth;
unsigned char R, G, B, DoPoint;
unsigned short LineStyle;
unsigned long PointCount, Marker;
char PickE;
JoeCoordSys *MyAttr;
CoordSys *MyCoords, *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (MyAttr = (JoeCoordSys *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

PickE = PickEnabled;
//if (PickEnabled == 2) PickEnabled = 0;

R = Clip->Red();
G = Clip->Green();
B = Clip->Blue();

if (Simplify > (long int)Clip->NumPoints()) Simplify = 0;

#ifdef WCS_VIEWGUI_INHIBIT_VEC_SIMPLIFY
Simplify = 0;
#endif // WCS_VIEWGUI_INHIBIT_VEC_SIMPLIFY

/*
if ((R == 180) && (G == 180) && (B == 180))
	{ // try the view color
	R = RedPart  (Default8[Clip->PenNum()]);
	G = GreenPart(Default8[Clip->PenNum()]);
	B = BluePart (Default8[Clip->PenNum()]);
	} // if
*/
Color[3] = 1.0f;

if (AsPoints == 1)
	{
	Color[0] = 1.0f;
	Color[1] = 0.0f;
	Color[2] = 0.0f;
	} // if
else if (AsPoints == 2)
	{
	Color[0] = 1.0f;
	Color[1] = 1.0f;
	Color[2] = 1.0f;
	} // if
else
	{
	Color[0] = ((float)(R) / 255.0f);
	Color[1] = ((float)(G) / 255.0f);
	Color[2] = ((float)(B) / 255.0f);
	} // if

//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Color);
glColor4fv(Color);
Partial[0] = Color[0] * .1f;
Partial[1] = Color[1] * .1f;
Partial[2] = Color[2] * .1f;
Partial[3] = 1.0f;
//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, None);
LineStyle = Clip->GetLineStyle();
LineWidth = (float)Clip->GetLineWidth();

if (PickEnabled == 2)
	{
	glPointSize(6.0f);
	glLineWidth(6.0f);
	glDisable(GL_POINT_SMOOTH);
	} // if
else
	{
	if (AsPoints)
		{
		LineWidth = 4.0f;
		LineStyle = 1;
		} // if
	SetupPickName(Clip);

	switch(LineStyle)
		{
		case 0: // points
			{
			glBegin(GL_POINTS);
			break;
			} // 
		case 1: // circle
			{
			glPointSize(LineWidth);
			glEnable(GL_POINT_SMOOTH);
			glBegin(GL_POINTS);
			break;
			} // 
		case 2: // square
			{
			glPointSize(LineWidth);
			glBegin(GL_POINTS);
			break;
			} // 
		case 3: // cross
			{
			glPointSize(LineWidth);
			glBegin(GL_POINTS);
			break;
			} // 
		case 4: // solid
			{
			glLineWidth(LineWidth);
			glBegin(GL_LINE_STRIP);
			break;
			} // 
		case 5: // dotted
			{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xAAAA);
			glLineWidth(LineWidth);
			glBegin(GL_LINE_STRIP);
			break;
			} // 
		case 6: // dashed
			{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xf0f0);
			glLineWidth(LineWidth);
			glBegin(GL_LINE_STRIP);
			break;
			} // 
		case 7: // broken
			{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xf060);
			glLineWidth(LineWidth);
			glBegin(GL_LINE_STRIP);
			break;
			} // 
		} // switch
	} // else

PointCount = 0; // we don't do the label point
if ((AsPoints == 1) && !(PickEnabled == 2))
	{
	Marker = 0;
	Point = InterStash->GetFirstSelectedPt(Marker);
	} // if
else
	{
	Point = Clip->Points();
	} // else
while (Point)
	{
	#ifdef WCS_JOE_LABELPOINTEXISTS
	// skip label point
	if (Point != Clip->Points())
	#endif // WCS_JOE_LABELPOINTEXISTS
		{
		DoPoint = 1;
		if (Simplify)
			{
			if (PointCount % Simplify) DoPoint = 0; //lint !e573 // only do every Simplify points
			if ((PointCount == Joe::GetFirstRealPtNum()) || (Point->Next == NULL)) DoPoint = 1; // Always do first & last points
			} // if
		if (DoPoint && Point->ProjToDefDeg(MyCoords, &Node))
			{
			if (PlanView)
				{
				#ifdef WCS_BUILD_VNS
				if (ProjectedSys)
					{
					VertexDEM TempVert;

					TempVert.Lon  = Node.Lon;
					TempVert.Lat  = Node.Lat;
					TempVert.Elev = VC->VCCalcExag(Node.Elev);
					ProjectedSys->DefDegToProj(&TempVert);
					
					// Adjust by local origin
					Node.XYZ[0] = TempVert.xyz[0] - OriginVert->XYZ[0];
					Node.XYZ[1] = TempVert.xyz[1] - OriginVert->XYZ[1];
					Node.XYZ[2] = VC->VCCalcExag(Node.Elev);

					if (ProjectedSys->GetGeographic())
						{
						// not sure just yet
						/*
						Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
						TempLon = Vert->ScrnXYZ[0];
						if (fabs(TempLon) > 180.0)
							{
							TempLon += 180.0;
							if (fabs(TempLon) > 360.0)
								TempLon = fmod(TempLon, 360.0);	// retains the original sign
							if (TempLon < 0.0)
								TempLon += 360.0;
							TempLon -= 180.0;
							Vert->ScrnXYZ[0] = TempLon;
							} // if
						Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
						Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
						*/
						} // if

					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					Node.XYZ[0] = Node.Lon;
					Node.XYZ[1] = Node.Lat;
					Node.XYZ[2] = VC->VCCalcExag(Node.Elev);
					// Adjust by local origin
					Node.XYZ[0] -= OriginVert->Lon;
					Node.XYZ[1] -= OriginVert->Lat;
					//Node.XYZ[2] = VC->VCCalcExag(VC->SceneHigh);
					} // else, sorta
				} // if
			else
				{
				//Node.Lat  = CurVert.Lat;
				//Node.Lon  = CurVert.Lon;
				//Node.Elev = Point->Elevation;
				Node.Elev = VC->VCCalcExag(Node.Elev);

				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Node);
				#else // WCS_BUILD_VNS
				Node.DegToCart(VC->GlobeRadius);
				#endif // WCS_BUILD_VNS
				// Adjust by local origin
				FindPosVector(Node.XYZ, Node.XYZ, OriginVert->XYZ);
				//AddPoint3d(Node.XYZ, VC->NegOrigin);
				InvertZ(Node.XYZ);
				} // else
			if (PickEnabled == 2)
				{
				glLoadName((GLuint)PointCount);
				glBegin(GL_POINTS);
				} // if
			glVertex3dv(&Node.XYZ[0]);
			if (PickEnabled == 2)
				{
				glEnd();
				} // if
			} // if
		} // if
	if ((AsPoints == 1) && !(PickEnabled == 2))
		{
		Point = InterStash->GetNextSelectedPt(Marker);
		} // if
	else
		{
		Point = Point->Next;
		} // else
	PointCount ++;
	} // while

if (PickEnabled != 2)
	{
	glEnd();
	} // if
glLineWidth(1.0f);
glPointSize(1.0f);
glDisable(GL_LINE_STIPPLE);
glDisable(GL_POINT_SMOOTH);
ClearPickName();

PickEnabled = PickE;

return(1);

} // ViewGUI::GenOneVector

/*===========================================================================*/

int ViewGUI::GenOneLake(int PlanView, long Simplify, ViewContext *VC, Joe *Clip, LakeEffect *Lee, VertexDEM *OriginVert, CoordSys *ProjectedSys)
{
Point4f Color, Partial;
VertexDEM Node;
VectorPoint *Point;
int Steps = 0;
unsigned char R, G, B, DoPoint;
char AmIActive = 0, IsPlan = 0;
unsigned long PointCount;
double LeeElev = 0.0;
JoeCoordSys *MyAttr;
CoordSys *MyCoords, *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

// need at least three vertices to make a 3 point polygon. + labelpoint makes 4 minimum
if (Clip->GetNumRealPoints() < 3)
	return(1);

if (MyAttr = (JoeCoordSys *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

R = Clip->Red();
G = Clip->Green();
B = Clip->Blue();

if (Lee)
	{
	ThematicMap *ElevTheme;
	double Value[3];
	if ((ElevTheme = Lee->GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_ELEV)) && ElevTheme->Eval(Value, Clip))
		LeeElev = Value[0];
	else
		LeeElev = Lee->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].GetCurValue(0);
	if (VC->Planet && VC->Planet->EcoExageration)
		LeeElev = VC->VCCalcExag(LeeElev);
	} // if

if (RasterAnimHost::GetActiveRAHost())
	AmIActive = (Lee == RasterAnimHost::GetActiveRAHost()) || (Lee == ((RasterAnimHost::GetActiveRAHost())->GetRAHostRoot()));

if (Simplify > (long int)Clip->NumPoints()) Simplify = 0;
#ifdef WCS_VIEWGUI_INHIBIT_VEC_SIMPLIFY
Simplify = 0;
#endif // WCS_VIEWGUI_INHIBIT_VEC_SIMPLIFY
/*
if ((R == 180) && (G == 180) && (B == 180))
	{ // try the view color
	R = RedPart  (Default8[Clip->PenNum()]);
	G = GreenPart(Default8[Clip->PenNum()]);
	B = BluePart (Default8[Clip->PenNum()]);
	} // if
*/
Color[0] = ((float)(R) / 255.0f);
Color[1] = ((float)(G) / 255.0f);
Color[2] = ((float)(B) / 255.0f);
glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Color);
glColor4fv(Color);
Partial[0] = Color[0] * .1f;
Partial[1] = Color[1] * .1f;
Partial[2] = Color[2] * .1f;
Partial[3] = 1.0f;
//ViewWins[ViewNum]->fglMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, None);

if (IsPlan = (char)VC->IsPlan())
	{
	glEnable(GL_DEPTH_TEST);
	} // if

if (AmIActive)
	{
	// 012209 CXH: Disable highlighting of active lake surfaces of this kind, as they only get displayed from displaylists
	// and we don't want the active state burned into the displaylist
	//DrawSetupActiveObject(VC);
	} // if

SetupPickName(Lee);

glBegin(GL_POLYGON);

PointCount = 0; // we don't do the label point
Point = Clip->Points();
while (Point)
	{
	#ifdef WCS_JOE_LABELPOINTEXISTS
	// skip label point
	if (Point != Clip->Points())
	#endif // WCS_JOE_LABELPOINTEXISTS
		{
		DoPoint = 1;
		if (Simplify)
			{
			if (PointCount % Simplify) DoPoint = 0;  //lint !e573 // only do every Simplify points
			if ((PointCount == Joe::GetFirstRealPtNum()) || (Point->Next == NULL)) DoPoint = 1; // Always do first & last points
			} // if
		if (DoPoint && Point->ProjToDefDeg(MyCoords, &Node))
			{
			//Node.Lat  = Point->Latitude;
			//Node.Lon  = Point->Longitude;
			//Node.Elev = Point->Elevation;
			Node.Elev = LeeElev;

			if (IsPlan)
				{
				#ifdef WCS_BUILD_VNS
				if (ProjectedSys)
					{
					VertexDEM TempVert;

					TempVert.Lon  = Node.Lon;
					TempVert.Lat  = Node.Lat;
					TempVert.Elev = Node.Elev;
					ProjectedSys->DefDegToProj(&TempVert);
					
					// Adjust by local origin
					Node.XYZ[0] = TempVert.xyz[0] - OriginVert->XYZ[0];
					Node.XYZ[1] = TempVert.xyz[1] - OriginVert->XYZ[1];
					Node.XYZ[2] = Node.Elev;

					if (ProjectedSys->GetGeographic())
						{
						// not sure just yet
						/*
						Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
						TempLon = Vert->ScrnXYZ[0];
						if (fabs(TempLon) > 180.0)
							{
							TempLon += 180.0;
							if (fabs(TempLon) > 360.0)
								TempLon = fmod(TempLon, 360.0);	// retains the original sign
							if (TempLon < 0.0)
								TempLon += 360.0;
							TempLon -= 180.0;
							Vert->ScrnXYZ[0] = TempLon;
							} // if
						Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
						Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
						*/
						} // if

					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					Node.XYZ[0] = Node.Lon;
					Node.XYZ[1] = Node.Lat;
					Node.XYZ[2] = Node.Elev;
					// Adjust by local origin
					Node.XYZ[0] -= OriginVert->Lon;
					Node.XYZ[1] -= OriginVert->Lat;
					} // else
				} // if
			else
				{
				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Node);
				#else // WCS_BUILD_VNS
				Node.DegToCart(VC->GlobeRadius);
				#endif // WCS_BUILD_VNS
				// Adjust by local origin
				FindPosVector(Node.XYZ, Node.XYZ, OriginVert->XYZ);
				//AddPoint3d(Node.XYZ, VC->NegOrigin);
				InvertZ(Node.XYZ);
				} // if
			glVertex3dv(&Node.XYZ[0]);
			} // if
		} // if
	Point = Point->Next;
	PointCount ++;
	} // while

glEnd();
ClearPickName();

if (AmIActive)
	{
	// 012209 CXH: Disable highlighting of active lake surfaces of this kind, as they only get displayed from displaylists
	// and we don't want the active state burned into the displaylist
	//DrawFinishActiveObject(VC);
	} // if

if (IsPlan)
	{
	glDisable(GL_DEPTH_TEST);
	} // if

return(1);

} // ViewGUI::GenOneLake

/*===========================================================================*/

void ViewGUI::PushAndTranslateOrigin(ViewContext *VC)
{
Point3d OriginShift;

glPushMatrix();
if (VC->IsPlan())
	{
	#ifdef WCS_BUILD_VNS
	if (VC->GetCamera() && VC->GetCamera()->Projected && VC->GetCamera()->Coords) // handle projected planimetric Views
		{
		// calculate difference between Camera and Displaylist Local Origin (DBCenter)
		// I don't think this is necessary, due to our setting up our glOrtho matrix using already-translated coords
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		OriginShift[0] = DBCenter.Lon;
		OriginShift[1] = DBCenter.Lat;
		OriginShift[2] = 0.0;
		glTranslated(OriginShift[0], OriginShift[1], OriginShift[2]);
		} // else
	} // if
else
	{
	// these three lines were added to replace the one commented out below in order to select vectors
	// now that the vectors are offset from the DBCenter when drawn
	CopyPoint3d(OriginShift, DBCenter.XYZ); // get object in WCS world coords
	InvertZ(OriginShift); // Convert object local origin from WCS world coords to GL world coords
	glTranslated(VC->GLNegOrigin[0] + OriginShift[0], VC->GLNegOrigin[1] + OriginShift[1], VC->GLNegOrigin[2] + OriginShift[2]);
	//glTranslated(VC->GLNegOrigin[0], VC->GLNegOrigin[1], VC->GLNegOrigin[2]);
	} // if

} // ViewGUI::PushAndTranslateOrigin

/*===========================================================================*/

void ViewGUI::PopOrigin(void)
{

glPopMatrix();

} // ViewGUI::PopOrigin

/*===========================================================================*/

void ViewGUI::HandleNotifyEvent(void)
{
int UpdateObjLOD = 0, DoDEMRegen = 0, DoTFXRegen = 0, DoVecRegen = 0, DoWallRegen = 0, DoDraw = 1, DoImmediate = 0, ValidateCams = 0, ValidateSky = 0, ValidateOpts = 0, ValidateAllTitles = 0, ClearAll = 0, ViewLoop, NotValid;
ViewContext *OriVC = NULL;
DrawingFenetre *OriDF = NULL;
int ChangeLoop;
char Retitle, UpdatePrefs = 0, UpdateRTPrefs = 0, UpdateTitles = 0;

NotifyTag Changed, BufChanged, ThreshChanged, *Changes, OtherInterested[6], BufInterested[2], ThreshInterested[2];

if (QuietDuringLoad) return;

if (Activity->ChangeNotify->NotifyData)
	{
	if (! GlobalApp->AppEffects->IsEffectValid((GeneralEffect *)Activity->ChangeNotify->NotifyData, 0)
		&& ! GlobalApp->AppDB->ValidateJoe((Joe *)Activity->ChangeNotify->NotifyData))
		return;
	} // if

Changes = Activity->ChangeNotify->ChangeList;

OtherInterested[1] = OtherInterested[2] = NULL;


// 3d LOD change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, WCS_EFFECTS_OBJECT3D_NUMANIMPAR, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	// WCS_EFFECTS_OBJECT3D_NUMANIMPAR discriminates only preview/box/detail changes
	// 3dobject decimation disabled by popular demand
	//UpdateObjLOD = 1;
	} // if

// Vector change
// no need for special handling currently
/*
OtherInterested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	// nothing special
	} // if
*/

// Vector visual style changes -- regen displaylists
OtherInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB);
OtherInterested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN);
OtherInterested[2] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH);
OtherInterested[3] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE);
OtherInterested[4] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE);
OtherInterested[5] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoVecRegen = 1;
	} // if

// clear out the usuals (after that big match, above) so we don't have to set them to NULL for each below
OtherInterested[1] = OtherInterested[2] = OtherInterested[3] = NULL;


// Sky change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SKY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	ValidateSky = 1;
	} // if

// Time change
OtherInterested[0] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoImmediate = 1;
	UpdatePrefs = 1;
	} // if

// Camera addition/removal
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	ValidateCams = 1;
	UpdatePrefs = 1;
	} // if

// Renderopt change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	ValidateOpts = 1;
	UpdatePrefs = 1;
	} // if


// Camera or RenderOpt name change -- update title
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, WCS_EFFECTSSUBCLASS_CAMERA, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
OtherInterested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	UpdateTitles = 1;
	} // if

OtherInterested[1] = OtherInterested[2] = NULL;


// Less drastic Camera change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	UpdatePrefs = 1;
	UpdateTitles = 1; // an undo causes this event, which could change the camera name
	} // if

// Less drastic Renderopt change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	UpdatePrefs = 1;
	} // if

// Planetopt Vertical Exag (Vertical Scale) change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_PLANETOPT, WCS_SUBCLASS_ANIMDOUBLETIME, WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG, WCS_NOTIFYCOMP_ANIM_VALUECHANGED); // 0x1fa50264
OtherInterested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_PLANETOPT, WCS_SUBCLASS_ANIMDOUBLETIME, WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG, WCS_NOTIFYCOMP_ANIM_NODEREMOVED); // 0x1fa502??
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoDEMRegen = 1;
	DoVecRegen = 1;
	} // if

OtherInterested[1] = OtherInterested[2] = NULL;

// Planetopt default coordsys change
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_PLANETOPT, WCS_EFFECTSSUBCLASS_PLANETOPT, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED); // 
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoDEMRegen = 1;
	DoVecRegen = 1;
	} // if


// Coordsys change -- pretty serious
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	CoordSys *ChangedEntity;
	// can we minimize the impact of no-op CoordSys changes?
	if (ChangedEntity = (CoordSys *)Activity->ChangeNotify->NotifyData)
		{
		// does this CS have ANY DB objects affiliated with it?
		// if not, we can probably safely ignore it, as all non-joe-linked
		// CoordSys stuff (colormap images, etc) aren't displaylisted
		// and will be addressed during the normal view redraw
		if (ChangedEntity->Joes)
			{
			DoDEMRegen = 1;
			DoVecRegen = 1;
			} // if
		} // if
	} // if


// New TerraGenerator terrain available
OtherInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	RegenDyn = 1;
	// do something clever like render new terrain
	// assuming you wanted to get the float map as "TerrainMap" and the number of rows and columns as "TerrainPoints"
	// and you had already found a TerraGenerator named "TerraGenFX":

	//if (TerraGenFX->GetEnabled() && TerraGenFX->GetPreviewEnabled() && (TerrainMap = (float *)TerraGenFX->GetPreviewMap()))
	//	{
	//	TerrainPoints = (long)TerraGenFX->GetPreviewSize();
	//	} // if
	} // if


// Did everything change?
OtherInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	// disable views
	/* ClearAll = 1; */
	LoadInProgress = 1;
	if (PrefsGUI)
		{
		delete PrefsGUI;
		PrefsGUI = NULL;
		} // if
	if (RTPrefsGUI)
		{
		// let's not close, as this can be caused by
		// starting a render or generating a RTF file
		UpdateRTPrefs = 1;
		//delete RTPrefsGUI;
		//RTPrefsGUI = NULL;
		} // if
	return;
	} // if

// Did everything change?
OtherInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	LoadInProgress = 0;
	// Request validation
	ValidateSky = 1;
	ValidateCams = 1;
	ValidateOpts = 1;
	UpdatePrefs = 1;
	ValidateAllTitles = 1;
	} // if

// Regen vectors?
OtherInterested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, 0xff);
OtherInterested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	// WCS_NOTIFYCOMP_OBJECT_NAMECHANGED is just a name or layer change, no visual change, no regen needed
	if (NOTIFYCOMPONENTPART(Changed) != WCS_NOTIFYCOMP_OBJECT_NAMECHANGED)
		{
		DoVecRegen = 1;
		} // if
	// Terraffector preview AutoUpdate
	// (Change in terraffector itself)
	if (InterStash->GetTfxRealtime())
		{
		JoeAttribute *JA;
		GeneralEffect *GE;
		Joe *TFXJ;

		if (TFXJ = (Joe *)Activity->ChangeNotify->NotifyData)
			{
			if (JA = TFXJ->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR))
				{
				if (GE = TFXJ->GetAttributeEffect(JA))
					{
					if (GE->GetEnabled())
						{
						DoTFXRegen = 1;
						} // if
					} // if
				} // if
			if (JA = TFXJ->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR))
				{
				if (GE = TFXJ->GetAttributeEffect(JA))
					{
					if (GE->GetEnabled())
						{
						DoTFXRegen = 1;
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if
OtherInterested[1] = OtherInterested[2] = NULL;

// Terraffector preview AutoUpdate
// (Change in terraffector itself)
if (InterStash->GetTfxRealtime())
	{
	OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, 0xff, 0xff, 0xff);
	OtherInterested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_RASTERTA, 0xff, 0xff, 0xff);
	if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
		{
		GeneralEffect *TFXGE;
		if (NOTIFYCOMPONENTPART(Changed) == WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED) // linking or unlinking to a Joe
			{
			DoTFXRegen = 1;
			} // if
		if (TFXGE = (GeneralEffect *)Activity->ChangeNotify->NotifyData)
			{
			JoeList *EffectJoes;
			for (EffectJoes = TFXGE->Joes; EffectJoes; EffectJoes = EffectJoes->Next)
				{
				if (EffectJoes->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
					{
					DoTFXRegen = 1;
					} // if
				} // for
			} // if
		} // if
	} // if
OtherInterested[1] = OtherInterested[2] = NULL;
OtherInterested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoVecRegen = 1;
	} // if

OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, WCS_SUBCLASS_ANIMDOUBLETIME, 0, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
OtherInterested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
OtherInterested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
OtherInterested[3] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{ // catch attached-lake enable/disable notifications
	LakeEffect *EventLake = NULL;
	if (EventLake = (LakeEffect *)Activity->ChangeNotify->NotifyData)
		{
		// are there any vectors or SQs attached to this Lake?
		if (EventLake->Joes && EventLake->Joes->Me) // is there at least one hard-link joe?
			{
			DoVecRegen = 1;
			} // if
		// we don't yet even display lakes for SQ-linked vectors, so this is moot so far */
		/*
		if (EventLake->Search && EventLake->Search->OneFilterValid()) // may have some vectors, too slow to actually count them, so assume some qualify
			{
			DoVecRegen = 1;
			} // if
		*/
		} // if
	} // if

OtherInterested[1] = OtherInterested[2] = NULL;

// Regen DEMs?
OtherInterested[0] = MAKE_ID(WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_GRANULARITY, WCS_INTERCAM_ITEM_GRIDSAMPLE, 0);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoDEMRegen = 1;
	} // if
OtherInterested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoDEMRegen = 1;
	} // if
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_FENCE, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoWallRegen = 1;
	} // if
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_FENCE, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	DoWallRegen = 1;
	} // if

// handle thematic maps on Foliage Effects or Walls
OtherInterested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, 0xff);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(OtherInterested, Changes, 0))
	{
	ThematicMap *EventTM = NULL;
	if (EventTM = (ThematicMap *)Activity->ChangeNotify->NotifyData)
		{
		int FXIdx;
		// iterate Walls to see if any are using this TM for height
		Fence *WallComponent;
		for (FXIdx = 0, WallComponent = (Fence *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_FENCE));
		 WallComponent; WallComponent = (Fence *)(WallComponent->Next))
			{
			if (WallComponent->Enabled)
				{
				if ((EventTM == WallComponent->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)) ||
				 (EventTM == WallComponent->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)) ||
				 (EventTM == WallComponent->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)))
					{
					DoWallRegen = 1;
					DoDraw = 1;
					} // if
				++FXIdx;
				} // if
			} // for
		// catch Foliage FX themes below in broad all TMs because it's too ugly to dig into all foliage groups and items of all folFX
		// iterate Foliage effects to see if any are using this TM for height
		/*
		FoliageEffect *FolComponent;
		for (FXIdx = 0, FolComponent = (FoliageEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_FOLIAGE));
		 FolComponent; FolComponent = (FoliageEffect *)(FolComponent->Next))
			{
			if (FolComponent->Enabled)
				{
				if ((EventTM == FolComponent->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)) ||
				 (EventTM == FolComponent->GetEnabledTheme(WCS_ECOTYPE_THEME_DBH)) ||
				 (EventTM == FolComponent->GetEnabledTheme(WCS_ECOTYPE_THEME_BASALAREA)) ||
				 (EventTM == FolComponent->GetEnabledTheme(WCS_ECOTYPE_THEME_AGE)))
					{
					DoDraw = 1;
					} // if
				FXIdx ++;
				} // if
			} // for
		*/
		// iterate Lakes to see if any are using this TM for elevation
		LakeEffect *Ocean;
		for (FXIdx = 0, Ocean = (LakeEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LAKE));
		 Ocean; Ocean = (LakeEffect *)(Ocean->Next))
			{
			if (Ocean->Enabled)
				{
				if (EventTM == Ocean->GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_ELEV))
					{
					DoVecRegen = 1; // lake presentation is handled by displaylisted vectors
					} // if
				FXIdx ++;
				} // if
			} // for
		if (InterStash->GetTfxRealtime())
			{
			// iterate Area Terraffectors to see if any are using this TM for elevation
			RasterTerraffectorEffect *TFX;
			for (FXIdx = 0, TFX = (RasterTerraffectorEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RASTERTA));
			 TFX; TFX = (RasterTerraffectorEffect *)(TFX->Next))
				{
				if (TFX->Enabled)
					{
					if (EventTM == TFX->GetEnabledTheme(WCS_EFFECTS_RASTERTA_THEME_ELEV))
						{
						DoTFXRegen = 1;
						} // if
					FXIdx ++;
					} // if
				} // for
			} // if
		} // if
	DoDraw = 1; // trigger a basic redraw if nothing else
	} // if



if (UpdateObjLOD)
	{ // potentially have more or fewer polygons to sepnd on DEMs now
	DoDEMRegen = 1;
	} // if

if (ClearAll)
	{
	for (ViewLoop = 0; ViewLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewLoop++)
		{
		// disable views
		if (ViewSpaces[ViewLoop])
			{
			ViewSpaces[ViewLoop]->VCamera = NULL;
			ViewSpaces[ViewLoop]->RO = NULL;
			ViewSpaces[ViewLoop]->SetAlive(0);
			} // if
		if (ViewWins[ViewLoop]) ViewWins[ViewLoop]->SetTitle("Unconfigured View");
		} // for
	return;
	} // if

for (ViewLoop = 0; ViewLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewLoop++)
	{
	NotValid = 0;
	Retitle = 0;
	if (ViewWins[ViewLoop] && ViewSpaces[ViewLoop])
		{
		if (ValidateSky)
			{
			if (!GlobalApp->AppEffects->IsEffectValid(ViewSpaces[ViewLoop]->VSky, WCS_EFFECTSSUBCLASS_SKY, 0))
				{
				ViewSpaces[ViewLoop]->VSky = NULL;
				} // if
			} // if
		if (ValidateCams)
			{
			if ((Activity->ChangeNotify) && (ViewSpaces[ViewLoop]->VCamera == (Camera *)(Activity->ChangeNotify->NotifyData)))
				{
				Retitle = 1;
				} // if
			if (!GlobalApp->AppEffects->IsEffectValid(ViewSpaces[ViewLoop]->VCamera, WCS_EFFECTSSUBCLASS_CAMERA, 0))
				{
				ViewSpaces[ViewLoop]->VCamera = NULL;
				NotValid = 1;
				// handled later, now.
/*				if (PrefsGUI && (PrefsGUI->GetActiveView() == ViewLoop))
					{
					delete PrefsGUI;
					PrefsGUI = NULL;
					} // if
*/
				} // if
			} // if
		if (ValidateOpts)
			{
			if ((Activity->ChangeNotify) && (ViewSpaces[ViewLoop]->RO == (RenderOpt *)(Activity->ChangeNotify->NotifyData)))
				{
				Retitle = 1;
				} // if
			if (!GlobalApp->AppEffects->IsEffectValid(ViewSpaces[ViewLoop]->RO, WCS_EFFECTSSUBCLASS_RENDEROPT, 0))
				{
				ViewSpaces[ViewLoop]->RO = NULL;
				NotValid = 1;
				if (PrefsGUI && (PrefsGUI->GetActiveView() == ViewLoop))
					{
					delete PrefsGUI;
					PrefsGUI = NULL;
					} // if
				} // if
			} // if
		if (Retitle || ValidateAllTitles)
			{
			ConfigureTitle(ViewLoop);
			} // if
		} // if
	if (NotValid)
		{
		ViewSpaces[ViewLoop]->SetAlive(0);
		// handled later, now.
		//ViewWins[ViewLoop]->SetTitle("Unconfigured View");
		} // if
	} // for

BufInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF, 0xff, 0xff);
BufInterested[1] = NULL;
ThreshInterested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THRESHOLD, 0xff, 0xff);
ThreshInterested[1] = NULL;

BufChanged = GlobalApp->AppEx->MatchNotifyClass(BufInterested, Changes, 0);
ThreshChanged = GlobalApp->AppEx->MatchNotifyClass(ThreshInterested, Changes, 0);

if (BufChanged || ThreshChanged)
	{
	DoDraw = 0;
	for (ChangeLoop = 0; ChangeLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ChangeLoop++)
		{
		if (ViewSpaces[ChangeLoop] && ViewWins[ChangeLoop])
			{
			OriDF = ViewWins[ChangeLoop];
			OriVC = ViewSpaces[ChangeLoop];
			// SetDisplayBuffer/SetDisplayThreshold expect to be setup already
			OriDF->SetupForDrawing();
			if (OriVC->BigR && BufChanged)
				{
				OriVC->BigR->SetDisplayBuffer((unsigned char)(BufChanged & 0xff));
				} // if

			if (OriVC->BigR && ThreshChanged)
				{
				OriVC->BigR->SetDisplayThreshold((unsigned char)((ThreshChanged & 0xff00) >> 8), (unsigned char)(ThreshChanged & 0xff));
				} // if

			OriDF->CleanupFromDrawing();
			} // if
		} // for
	} // if

if (DoDEMRegen || DoTFXRegen)
	{
	RegenDEMs = 1;
	} // if

if (DoVecRegen)
	{
	RegenVecs = 1;
	RegenWalls = 1;
	} // if

if (DoWallRegen)
	{
	RegenWalls = 1;
	} // if


if (DoDraw)
	{
	if (DoImmediate)
		{
		DrawImmediately();
		} // if
	else
		{
		Draw();
		} // else
	} // if

if (PrefsGUI && UpdatePrefs)
	{
	PrefsGUI->ConfigureWidgets();
	} // if

if (RTPrefsGUI && UpdateRTPrefs)
	{
	RTPrefsGUI->ConfigureWidgets();
	} // if

if (UpdateTitles)
	{
	for (ChangeLoop = 0; ChangeLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ChangeLoop++)
		{
		if (ViewSpaces[ChangeLoop] && ViewWins[ChangeLoop])
			{
			ConfigureTitle(ChangeLoop);
			} // if
		} // for
	} // if

// retitling of views to say Unconfigured should be done after all other
// event processing, because it can cause re-entrant events that will
// trash later event processing
if (ValidateCams)
	{
	for (ViewLoop = 0; ViewLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewLoop++)
		{
		if (ViewWins[ViewLoop] && ViewSpaces[ViewLoop])
			{
			if (!GlobalApp->AppEffects->IsEffectValid(ViewSpaces[ViewLoop]->VCamera, WCS_EFFECTSSUBCLASS_CAMERA, 0))
				{
				ViewSpaces[ViewLoop]->VCamera = NULL;
				ViewWins[ViewLoop]->SetTitle("Unconfigured View");
				if (PrefsGUI && (PrefsGUI->GetActiveView() == ViewLoop))
					{
					delete PrefsGUI;
					PrefsGUI = NULL;
					} // if
				} // if
			} // if
		} // for
	} // if

} // ViewGUI::HandleNotifyEvent

/*===========================================================================*/

long ViewGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
int ViewOrigin;

CheckSpaceMode();

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	DrawImmediately(ViewOrigin);
	//Draw(ViewOrigin);
	} // if

return(0);

} // ViewGUI::HandleReSized

/*===========================================================================*/

int ViewGUI::IdentViewOrigin(AppEvent *Activity)
{
int ViewOrigin = -1, ViewScan;

// Determine which window the event came from
for (ViewScan = 0; ViewScan < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewScan++)
	{
	if ((Fenetre *)ViewWins[ViewScan] == Activity->Origin)
		{
		ViewOrigin = ViewScan;
		break;
		} // if
	} // for

return(ViewOrigin);

} // ViewGUI::IdentViewOrigin

/*===========================================================================*/

int ViewGUI::IdentActiveView(void)
{
int Active = -1, ViewScan;

// Determine which window the event came from
for (ViewScan = 0; ViewScan < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewScan++)
	{
	if (ViewWins[ViewScan] && ViewWins[ViewScan]->AreYouActive())
		{
		Active = ViewScan;
		break;
		} // if
	} // for

return(Active);

} // ViewGUI::IdentActiveView

/*===========================================================================*/

// The following method is safe to call with a NULL instance, or View = -1
char ViewGUI::GetEnabled(int View, int Item)
{

if (this && View >= 0)
	{
	if (ViewSpaces[View])
		{
		return(ViewSpaces[View]->GetEnabled(Item));
		} // if
	} // if
return(0);

} // ViewGUI::GetEnabled

/*===========================================================================*/

// The following method is safe to call with a NULL instance, or View = -1
char ViewGUI::GetViewAlive(int View)
{

if (this && View >= 0)
	{
	if (ViewSpaces[View])
		{
		return(ViewSpaces[View]->GetAlive());
		} // if
	} // if
return(0);

} // ViewGUI::GetViewAlive

/*===========================================================================*/

long ViewGUI::HandleKeyUp(int Key, char Alt, char Control, char Shift)
{
int ViewOrigin;

CheckSpaceMode();

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	// F4 is handled globally
	if (Key == VK_F5)
		{
		HandlePopupMenuSelect(ID_WINMENU_OPTIONS_SETAREA);
		} // if
	else if (Key == VK_F6)
		{
		HandlePopupMenuSelect(ID_WINMENU_OPTIONS_CONSTRAIN);
		} // if
	else if (Key == VK_F7)
		{
		HandlePopupMenuSelect(ID_WINMENU_SAVEIMAGE);
		} // if
	else if (Key == VK_F8)
		{
		HandlePopupMenuSelect(ID_WINMENU_SHOWPREV);
		} // if
	else if (Key == VK_F9)
		{
		HandlePopupMenuSelect(ID_WINMENU_RENDERPREV);
		} // if
	} // if

return(0);

} // ViewGUI::HandleKeyUp

/*===========================================================================*/

/*BusyWin *MCBW; */

void ViewGUI::PrepMotionControl(void)
{
if (GetMotionControl())
	{
/*	if (!MCBW)
		{
		// this never increments
		MCBW = new BusyWin("Joystick Control", 10, 'BUSY', 0);
		} // if
*/	
	if (GetMotionControlMode())
		{
		GlobalApp->MCP->SetCurrentStatusText("Joystick Control: MultiAxis."); // slide
		} // if
	else
		{
		GlobalApp->MCP->SetCurrentStatusText("Joystick Control: Drive.");
		} // else
	JOX = GlobalApp->WinSys->CheckInputControllerX(1);
	JOY = GlobalApp->WinSys->CheckInputControllerY(1);
	JOZ = GlobalApp->WinSys->CheckInputControllerZ(1);
	BeginInteractive();
	if (!GlobalApp->IsBGHandler(this))
		{
		GlobalApp->AddBGHandler(this);
		} // if
	} // if
else
	{
	GlobalApp->MCP->SetCurrentStatusText("Joystick Control Off.");
/*	if (MCBW)
		{
		delete MCBW;
		MCBW = NULL;
		} // if
*/
	EndInteractive();
	} // else

} // ViewGUI::PrepMotionControl

/*===========================================================================*/

long ViewGUI::HandleKeyPress(int Key, char Alt, char Control, char Shift)
{
int ViewOrigin;
ViewContext *OriVC = NULL;

CheckSpaceMode();

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	if ((Key == 'j') || (Key == 'J'))
		{
		if (Key == 'J')
			{ // multiaxis (slide)
			if (GetMotionControl())
				{
				if (GetMotionControlMode() == 1)
					{ // switch off
					SetMotionControl(!GetMotionControl());
					} // if
				else
					{ // stay on, switch modes
					SetMotionControlMode(1);
					} // else
				} // if
			else
				{ // switch on
				SetMotionControlMode(1);
				SetMotionControl(!GetMotionControl());
				} // else
			} // if
		else
			{ // drive
			if (GetMotionControl())
				{
				if (GetMotionControlMode() == 0)
					{ // switch off
					SetMotionControl(!GetMotionControl());
					} // if
				else
					{ // stay on, switch modes
					SetMotionControlMode(0);
					} // else
				} // if
			else
				{ // switch on
				SetMotionControlMode(0);
				SetMotionControl(!GetMotionControl());
				} // else
			} // if
		PrepMotionControl();
		} // if
	if (Key == VK_ESCAPE)
		{
		if (GetMotionControl())
			{
			SetMotionControl(!GetMotionControl());
			} // if
		PrepMotionControl();
		} // if
	if (Key == '?')
		{
		DoPrefs(ViewOrigin);
		} // if
	if (Key == '/')
		{
		DoRTPrefs();
		} // if
	if (Key == 'g')
		{
		DoGLVersion();
		} // if
	if (Key == '&')
		{
		DoGLBench(ViewOrigin);
		} // if
	if (Key == '+' || Key == '=')
		{
		OriVC = ViewSpaces[ViewOrigin];
		if (OriVC && OriVC->VCamera)
			{
			ViewSpaces[ViewOrigin]->DismissRendererIfPresent(false); // dismiss without asking
			OriVC->VCamera->Zoom(1.0f);
			} // if
		} // 
	if (Key == '-')
		{
		OriVC = ViewSpaces[ViewOrigin];
		if (OriVC && OriVC->VCamera)
			{
			ViewSpaces[ViewOrigin]->DismissRendererIfPresent(false); // dismiss without asking
			OriVC->VCamera->Zoom(-1.0f);
			} // if
		} // 
	} // if

return(0);

} // ViewGUI::HandleKeyPress

/*===========================================================================*/

long ViewGUI::HandleMouseWheelVert(long X, long Y, float Amount, char Alt, char Control, char Shift)
{
int ViewOrigin;
ViewContext *OriVC = NULL;

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	OriVC = ViewSpaces[ViewOrigin];
	if (OriVC && OriVC->VCamera)
		{
		ViewSpaces[ViewOrigin]->DismissRendererIfPresent(false); // dismiss without asking
		OriVC->VCamera->Zoom(Amount);
		} // if
	} // if

return(1);

} // ViewGUI::HandleMouseWheelVert

/*===========================================================================*/

long ViewGUI::HandleMouseWheelHoriz(long X, long Y, float Amount, char Alt, char Control, char Shift)
{

return(1);

} // ViewGUI::HandleMouseWheelHoriz

/*===========================================================================*/

// Hack
extern int GLInhibitReadPixels;

void ViewGUI::DoGLCompatCheck(void)
{
const char *Vend, *Rend, *Vers, *Exts;
char SetAvoidGLReadPixels = 0;

if (!GLWarned)
	{
	GLWarned = 1;
	Vend = (const char *)glGetString(GL_VENDOR);
	Rend = (const char *)glGetString(GL_RENDERER);
	Vers = (const char *)glGetString(GL_VERSION);
	Exts = (const char *)glGetString(GL_EXTENSIONS);

	if (Vend)
		{
		if (strstr(Vend, "Intel"))
			{
			if (strstr(Rend, "Graphics Media Accelerator"))
				{
				char *QResult;
				bool SuppressWarning = false;
				if (QResult = GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("debug_select"))
					{
					if (strstr(QResult, "INTELGMA"))
						{
						SuppressWarning = true;
						} // if
					} // if
				if (!SuppressWarning)
					{
					if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
						{
						GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#INTELGMA");
						GlobalApp->MainProj->Prefs.SetConfigOpt("suppress_glcompatcheckvendor", "INTELGMA"); // suppress the warning next time
						} // if
					} // if
				} // if
			} // if
/*
		if (strstr(Vend, "ELSA"))
			{
			if (strstr(Rend, "ELSA GLoria II-64"))
				{
				SetAvoidGLReadPixels = 1;
				if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
					{
					GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#GLORIA2");
					} // if
				} // if
			} // if

		if (strstr(Vend, "Evans & Sutherland"))
			{
			if (strstr(Rend, "E&S REALimage Technology"))
				{
				SetAvoidGLReadPixels = 1;
				if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
					{
					GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#TORNADO");
					} // if
				} // if
			} // if
*/
		if (strstr(Vend, "Matrox Graphics Inc."))
			{
			if (strstr(Rend, "Matrox G4"))
				{
				if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
					{
					GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#MATROX");
					} // if
				} // if
			} // if
/*
		if (strstr(Vend, "Intergraph Corporation"))
			{
			if (strstr(Rend, "GLZICD"))
				{
				SetAvoidGLReadPixels = 1;
				if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
					{
					GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#WILDCAT");
					} // if
				} // if
			} // if
// SGI Cobalt
		if (strstr(Vend, "SGI"))
			{
			if (strstr(Rend, "Cobalt"))
				{
				SetAvoidGLReadPixels = 1;
				if (UserMessageOKCAN("OpenGL Compatability Warning", "The graphics card detected in your system is on the list of cards known or suspected to have OpenGL compatability issues with "APP_TLA". Click Ok to visit the OpenGL Troubleshooting section of the 3D Nature website to learn more."))
					{
					GlobalApp->HelpSys->OpenURLIndirect("http://www.3dnature.com/glprobs.html#COBALT");
					} // if
				} // if
			} // if
*/	
		} // if
	if (SetAvoidGLReadPixels && !GLInhibitReadPixels)
		{
		if (UserMessageYN("OpenGL Workaround Suggestion", APP_TLA" has a workaround built in to counteract one of the known problems with your display card. Would you like this workaround to be enabled for you?"))
			{
			GlobalApp->MainProj->Prefs.SetConfigOpt("avoid_glreadpixels", "yes");
			GLInhibitReadPixels = 1;
			} // if
		} // if

	} // if

} // ViewGUI::DoGLCompatCheck

/*===========================================================================*/

void ViewGUI::DoGLVersion(void)
{
char GLVersionString[8192], *TokIn;
const char *Vend, *Rend, *Vers, *Exts, *OneExt;
Vend = (const char *)glGetString(GL_VENDOR);
Rend = (const char *)glGetString(GL_RENDERER);
Vers = (const char *)glGetString(GL_VERSION);
Exts = (const char *)glGetString(GL_EXTENSIONS);

if (!Vend) Vend = "No Vendor Info Available.";
if (!Rend) Rend = "No Renderer Info Available.";
if (!Vers) Vers = "No Version Info Available.";
if (!Exts) Exts = "No Extension Info Available.";

//sprintf(GLVersionString, "Vendor: %s\nRenderer: %s\nVersion: %s\nExtensions: %s", Vend, Rend, Vers, Exts);
//UserMessageOK("GL Information", GLVersionString);
sprintf(GLVersionString, "Vendor: %s", Vend);
GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Renderer: %s", Rend);
GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Version: %s", Vers);
GlobalApp->StatusLog->PostError(0, GLVersionString);

GlobalApp->StatusLog->PostError(0, "Extensions:");
strcpy(GLVersionString, Exts);
TokIn = GLVersionString;
while (OneExt = strtok(TokIn, " "))
	{
	TokIn = NULL;
	GlobalApp->StatusLog->PostError(0, OneExt);
	} // while

} // ViewGUI::DoGLVersion

/*===========================================================================*/

void ViewGUI::DoGLBench(int ViewNum)
{
double STime, ETime, ElTime, AvTimePerFrame, FPS = 0;
char GLVersionString[8192], *TokIn, SysName[100];
const char *Vend, *Rend, *Vers, *Exts, *OneExt, *Plat;
int FrameLoop, MaxFrames = 250;
unsigned short WWidth = 0, WHeight = 0;
int DoExts = 0, DoSave = 0;
FILE *LogFile = NULL;
FileReq *GetNew;

// error-protection
if ((ViewNum == -1) || (!ViewWins[ViewNum])) return;

Plat = " ";

// Fetch private GLBENCH Frames variable if provided
if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("glbench_testframes"))
	{
	MaxFrames = atoi(GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("glbench_testframes"));
	} // if

// Prep view by drawing once to ensure everything is loaded
// and set up, etc.
DrawImmediately(ViewNum);

// do benchmarking loop
STime = GetSystemTimeFP();

for (FrameLoop = 0; FrameLoop < MaxFrames; FrameLoop++)
	{
	DrawImmediately(ViewNum);
	} // for

ETime = GetSystemTimeFP();

ElTime = ETime - STime;
AvTimePerFrame = ElTime / (double)MaxFrames;
if (AvTimePerFrame > 0)
	{
	FPS = 1 / AvTimePerFrame;
	} // if

Vend = (const char *)glGetString(GL_VENDOR);
Rend = (const char *)glGetString(GL_RENDERER);
Vers = (const char *)glGetString(GL_VERSION);
Exts = (const char *)glGetString(GL_EXTENSIONS);

if (!Vend) Vend = "No Vendor Info Available.";
if (!Rend) Rend = "No Renderer Info Available.";
if (!Vers) Vers = "No Version Info Available.";
if (!Exts) Exts = "No Extension Info Available.";

DoExts = UserMessageYN("GL Benchmarking", "Report GL extension list?", 1);
if (DoSave = UserMessageYN("GL Benchmarking", "Write results to file?", 0))
	{
	if (GetNew = new FileReq)
		{
		GetNew->SetDefFile(APP_TLA"GLOG.txt");
		if (GetNew->Request(WCS_REQUESTER_FILE_SAVE))
			{
			LogFile = PROJ_fopen((char *)GetNew->GetFirstName(), "a");
			} // if
		delete GetNew;
		GetNew = NULL;
		} // if
	} // if

sprintf(GLVersionString, "%s V%s OpenGL Benchmark Log%c", ExtTitle, ExtVersion, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

GlobalApp->InquireOSDescString(GLVersionString, 8191);
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

GlobalApp->InquireCPUDescString(GLVersionString, 8191);
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

SysName[0] = 0;
GlobalApp->InquireCPUDescString(SysName, 99);
sprintf(GLVersionString, "System Name: %s%c", SysName, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "OpenGL Card Vendor: %s%c", Vend, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "OpenGL Renderer: %s%c", Rend, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "OpenGL Version: %s%c", Vers, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

if (DoExts)
	{
	sprintf(GLVersionString, "OpenGL Extensions:%c", (LogFile ? '\n' : 0));
	if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);
	strcpy(GLVersionString, Exts);
	TokIn = GLVersionString;
	while (OneExt = strtok(TokIn, " "))
		{
		TokIn = NULL;
		if (LogFile)
			{
			fprintf(LogFile, "   %s", OneExt);
			fputc('\n', LogFile);
			} // if
		else
			{
			GlobalApp->StatusLog->PostError(0, OneExt);
			} // else
		} // while
	} // if

GlobalApp->InquireDisplayResString(GLVersionString, 8191);
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

ViewWins[ViewNum]->GetDrawingAreaSize(WWidth, WHeight);
sprintf(GLVersionString, "GL Window Size: %dx%d%c", WWidth, WHeight, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Max Polygons: %d%c", GlobalApp->MainProj->Interactive->GetGridSample(), (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Frames Tested: %d%c", MaxFrames, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Total Elapsed Time: %f%c", ElTime, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Avg Time / Frame: %f%c", AvTimePerFrame, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

sprintf(GLVersionString, "Avg Frames / Second: %f%c", FPS, (LogFile ? '\n' : 0));
if (LogFile) fputs(GLVersionString, LogFile); else GlobalApp->StatusLog->PostError(0, GLVersionString);

if (LogFile) fclose(LogFile); LogFile = NULL;

} // ViewGUI::DoGLBench

/*===========================================================================*/

long ViewGUI::HandleKeyDown(int Key, char Alt, char Control, char Shift)
{
ViewContext *OriVC = NULL;
GLDrawingFenetre *GLDF;
char DoRenderDiag = 0;
unsigned long ShiftCode = 0;

// identify which view it came from
if (OriginFen && OriginFen->CustomDataVoid[0])
	{
	GLDF = (GLDrawingFenetre *)OriginFen;
	//GLDF->GetDrawingAreaSize(VWidth, VHeight);
	OriVC = (ViewContext *)OriginFen->CustomDataVoid[0];
	DoRenderDiag = (OriVC->BigR && OriVC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV));
	} // if

CheckSpaceMode();

if (!DoRenderDiag)
	{
	if (Shift) ShiftCode = WCS_DOINTER_CODE_SHIFT;
	switch(Key)
		{
		case VK_ESCAPE:
			{
			if (BusyWinAbort == 0)
				{
				BusyWinAbort = 1;
				} // if
			if (AppScope->GUIWins->DIG)
				{
				EndCreate();
				} // if
			break;
			} //
		case VK_RETURN:
			{
			//user pressed enter key
			GlobalApp->MCP->KeyFrameAdd(Shift);
			break;
			} // ENTER
		case VK_LEFT:
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, -1, 0, 0, 0, ShiftCode);
			EndInteractive();
			break;
			} // 
		case VK_RIGHT:
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, 1, 0, 0, 0, ShiftCode);
			EndInteractive();
			break;
			} // 
		case VK_UP:
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, 0, -1, 0, 0, ShiftCode);
			EndInteractive();
			break;
			} // 
		case VK_DOWN:
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, 0, 1, 0, 0, ShiftCode);
			EndInteractive();
			break;
			} // 
		case VK_PRIOR: // PAGEUP
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, 0, 0, -1, -1, ShiftCode);
			EndInteractive();
			break;
			} // 
		case VK_NEXT: // PAGEDOWN
			{
			BeginInteractive();
			DoInteractive(OriVC, GLDF, 0, 0, 1, 1, ShiftCode);
			EndInteractive();
			break;
			} // 
		case 'M': // 
		case '1': // 
		case VK_NUMPAD1: // 
			{
			if (GetViewManipulationEnable()) SetViewManipulationMode(WCS_VIEWGUI_MANIP_MOVE);
			else SetObjectManipulationMode(WCS_VIEWGUI_MANIP_MOVE);
			break;
			} // 
		case 'R': // 
		case '2': // 
		case VK_NUMPAD2: // 
			{
			if (GetViewManipulationEnable()) SetViewManipulationMode(WCS_VIEWGUI_MANIP_ROT);
			else SetObjectManipulationMode(WCS_VIEWGUI_MANIP_ROT);
			break;
			} // 
		case 'S': // 
		case '3': // 
		case VK_NUMPAD3: // 
			{
			if (GetViewManipulationEnable()) SetViewManipulationMode(WCS_VIEWGUI_MANIP_SCALE);
			else SetObjectManipulationMode(WCS_VIEWGUI_MANIP_SCALE);
			break;
			} // 
		case 'X': // 
		case '4': // 
		case VK_NUMPAD4: // 
			{
			SetAxisEnable(WCS_X, !GetAxisEnable(WCS_X));
			break;
			} // 
		case 'Y': // 
		case '5': // 
		case VK_NUMPAD5: // 
			{
			SetAxisEnable(WCS_Y, !GetAxisEnable(WCS_Y));
			break;
			} // 
		case 'Z': // 
		case '6': // 
		case VK_NUMPAD6: // 
			{
			SetAxisEnable(WCS_Z, !GetAxisEnable(WCS_Z));
			break;
			} // 
		case 'P': // 
		case '7': // 
		case VK_NUMPAD7: // 
			{
			// <<<>>>
			break;
			} // 
		case 'D': // 
		case '8': // 
		case VK_NUMPAD8: // 
			{
			// <<<>>>
			break;
			} // 
		case 'U': // 
		case '9': // 
		case VK_NUMPAD9: // 
			{
			// <<<>>>
			break;
			} // 
		case 'V': // 
			{
			HandlePopupMenuSelect(ID_WINMENU_VIEW_REFRESH);
			break;
			} // 
		} // switch
	} // if

return(0);

} // ViewGUI::HandleKeyDown

/*===========================================================================*/

long ViewGUI::HandlePopupMenuSelect(int MenuID)
{
int ViewOrigin;

if (((ViewOrigin = IdentViewOrigin(Activity)) >= 0) || ((ViewOrigin = LastActiveView) >= 0))
	{
	// moved this code out of switch/case statement to handle larger numbers of camera entities
	if (MenuID >= ID_WINMENU_VIEW_CAM1 && MenuID < (ID_WINMENU_VIEW_CAM1 + WCS_VIEWGUI_VIEWS_CAMSINPOPUP_MAX))
		{
		int ItemIdx;
		GeneralEffect *MyEffect;
		// identify the camera and set
		for (ItemIdx = 0, MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); MyEffect; MyEffect = MyEffect->Next)
			{
			if (MenuID == (ID_WINMENU_VIEW_CAM1 + (ItemIdx++)))
				{
				if (ViewSpaces[ViewOrigin])
					{
					ViewSpaces[ViewOrigin]->SetCamera((Camera *)MyEffect);
					//ViewWins[ViewOrigin]->SetTitle((((Camera *)(MyEffect))->GetName()));
					ConfigureTitle(ViewOrigin);
					if (PrefsGUI) PrefsGUI->ConfigureWidgets();
					ViewSpaces[ViewOrigin]->DismissRendererIfPresent(false); // dismiss without asking
					Draw(ViewOrigin);
					} // if
				} // if
			} // for
		} // if
	// moved this code out of switch/case statement to handle larger numbers of RenderOpt entities
	else if (MenuID >= ID_WINMENU_VIEW_ROPT1 && MenuID < (ID_WINMENU_VIEW_ROPT1 + WCS_VIEWGUI_VIEWS_ROPTINPOPUP_MAX))
		{
		int ItemIdx;
		GeneralEffect *MyEffect;
		// identify the options and set
		for (ItemIdx = 0, MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); MyEffect; MyEffect = MyEffect->Next)
			{
			if (MenuID == (ID_WINMENU_VIEW_ROPT1 + (ItemIdx++)))
				{
				if (ViewSpaces[ViewOrigin])
					{
					ViewSpaces[ViewOrigin]->SetRenderOpt((RenderOpt *)MyEffect);
					ConfigureTitle(ViewOrigin);
					Draw(ViewOrigin);
					} // if
				} // if
			} // for
		} // if
	else
		{ // any other codes
		switch(MenuID)
			{
			case ID_WINMENU_VIEW_REFRESH:
				{
				DrawForceRegen(1, 1);
				break;
				} // ID_WINMENU_VIEW_REFRESH
			case ID_WINMENU_OPTIONS_SETAREA:
				{
				if (!RegionInProgress)
					{
					switch(UserMessageCustom("Set Constrained Render Area","The next two points clicked in a View\n will become the new Render Area.\n\nPoints may be selected in any order.", "Ok", "Cancel", "Use Last Area", 0)) // OK, Cancel, LastArea
						{
						case 0:
							{ // cancel
							return(0);
							} // 
						case 1:
							{ // OK
							break;
							} // 
						case 2:
							{ // last area
							if (ViewSpaces[ViewOrigin])
								{
								ViewSpaces[ViewOrigin]->SetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION, 1);
								UpdatePrefs(ViewOrigin);
								Draw(ViewOrigin);
								} // if
							break;
							} // 
						} // switch
					StartRegion();
					} // if
				else
					{
					DoCancelRegion(0);
					} // else
				break;
				} // Copy
			case ID_WINMENU_SHOW_COPYRENDER:
				{
				DoCopy(ViewOrigin, 1);
				UpdatePrefs(ViewOrigin);
				break;
				} // Copy
			case ID_WINMENU_RENDER_COPYREALTIME:
				{
				DoCopy(ViewOrigin, 0);
				UpdatePrefs(ViewOrigin);
				break;
				} // Copy
			case ID_WINMENU_MAKE_QUICK:
				{
				DoAnim(ViewOrigin);
				break;
				} // make quickanim
			case ID_WINMENU_VIEW_EDITCAM:
				{
				if (ViewSpaces[ViewOrigin]->CheckCamValid())
					{
					ViewSpaces[ViewOrigin]->VCamera->Edit();
					} // if
				break;
				} // edit view's camera
			case ID_WINMENU_VIEW_EDITRO:
				{
				if (ViewSpaces[ViewOrigin]->CheckROValid())
					{
					ViewSpaces[ViewOrigin]->RO->Edit();
					} // if
				break;
				} // edit view's renderopt
			case ID_WINMENU_VIEW_WINENABLED:
				{
				// User is attempting to enable or disable a view
				// if disable, go aheqad and do it, if enable, must make sure it's safe to do
				if (ViewSpaces[ViewOrigin]->GetAlive())
					{
					// disable it
					ViewSpaces[ViewOrigin]->SetAlive(0);
					} // if
				else
					{
					// need to validate if camera and RO are ok
					if (ViewSpaces[ViewOrigin]->CheckImportantValid())
						{
						ViewSpaces[ViewOrigin]->SetAlive(1);
						#ifdef WCS_BUILD_VNS
						if (ViewSpaces[ViewOrigin]->GetProjected())
							{
							// make sure only one Projected View is active at a time.
							for (int ViewProjLoop = 0; ViewProjLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewProjLoop++)
								{
								if (ViewSpaces[ViewProjLoop] && ViewWins[ViewProjLoop])
									{
									if (ViewSpaces[ViewProjLoop]->GetProjected() && ViewSpaces[ViewProjLoop]->GetAlive())
										{
										if (ViewProjLoop != ViewOrigin)
											{
											// only one projected view alive at a time, disable others
											ViewSpaces[ViewProjLoop]->SetAlive(0);
											ConfigureTitle(ViewProjLoop);
											} // if
										} // if
									} // if
								} // for
							// we assume our Projected displaylists are probably obsolete
							RegenDEMs = 1;
							RegenVecs = 1;
							} // if

						#endif // WCS_BUILD_VNS
						Draw(ViewOrigin);
						} // if
					} // else
				ConfigureTitle(ViewOrigin);
				UpdatePrefs(ViewOrigin);
				break;
				} // Enable View
			case ID_WINMENU_VIEW_EDITORS:
				{
				if (ViewSpaces[ViewOrigin] && ViewWins[ViewOrigin])
					{
					DoClose(ViewOrigin);
					} // if
				break;
				} //
			case ID_WINMENU_VIEW_NEWPERSPEC:
			case ID_WINMENU_VIEW_NEWOVER:
			case ID_WINMENU_VIEW_NEWPLAN:
				{
				Camera *NewCam = NULL;
				char CamType = WCS_EFFECTS_CAMERATYPE_TARGETED;
				if (MenuID == ID_WINMENU_VIEW_NEWOVER)
					{
					CamType = WCS_EFFECTS_CAMERATYPE_OVERHEAD;
					} // if
				if (MenuID == ID_WINMENU_VIEW_NEWPLAN)
					{
					CamType = WCS_EFFECTS_CAMERATYPE_PLANIMETRIC;
					} // if
				if (NewCam = CreateNewCamera(CamType))
					{
					if (ViewSpaces[ViewOrigin])
						{
						ViewSpaces[ViewOrigin]->SetCamera(NewCam);
						//ViewWins[ViewOrigin]->SetTitle(NewCam->GetName());
						ConfigureTitle(ViewOrigin);
						if (PrefsGUI) PrefsGUI->ConfigureWidgets();
						Draw(ViewOrigin);
						} // if
					} // if
				break;
				} //
/*
			case ID_WINMENU_DIAGNOSTICS:
				{
				GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
							WCS_TOOLBAR_ITEM_RDG, 0);
				break;
				} // 
*/
			case ID_WINMENU_SAVEIMAGE:
				{
				if (ViewSpaces[ViewOrigin] && ViewSpaces[ViewOrigin]->BigR)
					ViewSpaces[ViewOrigin]->BigR->SaveDisplayedBuffers(0);
				else
					DoSaveImage(ViewOrigin);
				break;
				} // 
			case ID_WINMENU_JOY_DRIVE:
				{
				if (GetMotionControl())
					{
					if (GetMotionControlMode() == 0)
						{ // switch off
						SetMotionControl(!GetMotionControl());
						} // if
					else
						{ // stay on, switch modes
						SetMotionControlMode(0);
						} // else
					} // if
				else
					{ // switch on
					SetMotionControlMode(0);
					SetMotionControl(!GetMotionControl());
					} // else
				PrepMotionControl();
				break;
				} // 
			case ID_WINMENU_JOY_SLIDE:
				{ // multiaxis
				if (GetMotionControl())
					{
					if (GetMotionControlMode() == 1)
						{ // switch off
						SetMotionControl(!GetMotionControl());
						} // if
					else
						{ // stay on, switch modes
						SetMotionControlMode(1);
						} // else
					} // if
				else
					{ // switch on
					SetMotionControlMode(1);
					SetMotionControl(!GetMotionControl());
					} // else
				PrepMotionControl();
				break;
				} // 
			case ID_WINMENU_SHOWPREV:
				{
				if (ViewSpaces[ViewOrigin] && ViewSpaces[ViewOrigin]->CheckRendererPresent())
					{
					if (!ViewSpaces[ViewOrigin]->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
						{ // update window with rendered image
						ViewSpaces[ViewOrigin]->ShowRendererIfPresent();
						} // if
					else
						{
						ViewSpaces[ViewOrigin]->DismissRendererIfPresent(true); // ask, then dismiss
						Draw();
						} // else
					} // if
				else
					{ // could only be the F8 key, since the menu ghosts
					if (UserMessageYN("View", "No preview available. Render one?", 0))
						{
						DoRender(ViewOrigin);
						} // if
					} // else
				EnableViewButtons(0); // refresh pressed state of Render button
				break;
				} // 
			//case ID_WINMENU_SHOW_EDITMORE:
			case ID_WINMENU_RENDER_EDITMORE:
				{
				DoPrefs(ViewOrigin);
				break;
				} // 

			case ID_WINMENU_SHOW_CONTROLPOINTS: 
			case IS_WINMENU_SHOW_MEASURES:
			case ID_WINMENU_SHOW_ACTIVEOBJECT:
			case ID_WINMENU_SHOW_DEMEDGES:
			case ID_WINMENU_SHOW_TERRAIN:
			case ID_WINMENU_SHOW_TERRAIN_TRANS:
			case ID_WINMENU_SHOW_TERRAIN_POLY:
			case ID_WINMENU_SHOW_TFX:
			case ID_WINMENU_SHOW_ATFX:

			case ID_WINMENU_SHOW_SNOW:
			case ID_WINMENU_SHOW_FOLIAGEFX:
			case ID_WINMENU_SHOW_ECOFX:
			case ID_WINMENU_SHOW_COLORMAPS:
			case ID_WINMENU_SHOW_GROUND:

			case ID_WINMENU_SHOW_WATER:
			case ID_WINMENU_SHOW_STREAMS:
			case ID_WINMENU_SHOW_WAVES:

			case ID_WINMENU_SHOW_HAZE:
			case ID_WINMENU_SHOW_CLOUDS:
			case ID_WINMENU_SHOW_SKY:
			case ID_WINMENU_SHOW_CELESTIALOBJECTS:

			case ID_WINMENU_SHOW_SHADOWFX:
			case ID_WINMENU_SHOW_LIGHTS:

			case ID_WINMENU_SHOW_3DOBJS:
			case ID_WINMENU_SHOW_WALLS:
			case ID_WINMENU_SHOW_VECTORS:

			case ID_WINMENU_SHOW_CAMERAS:
			case ID_WINMENU_SHOW_TARGETS:
			case ID_WINMENU_SHOW_IMGBOUNDS:

			case ID_WINMENU_SHOW_CONTOURS:
			case ID_WINMENU_SHOW_SLOPE:
			case ID_WINMENU_SHOW_RELEL:
			case ID_WINMENU_SHOW_LOD:
			case ID_WINMENU_SHOW_ECOSYSTEMS:
			//case ID_WINMENU_SHOW_RENDEREDTEXTURE:
			case ID_WINMENU_SHOW_GREY:
			case ID_WINMENU_SHOW_EARTH:
			case ID_WINMENU_SHOW_COLOR:
			case ID_WINMENU_SHOW_MULTIGRAD:
			case ID_WINMENU_OPTIONS_CONSTRAIN:
			case ID_WINMENU_OPTIONS_SAFETITLE:
			case ID_WINMENU_OPTIONS_SAFEACTION:
			case ID_WINMENU_SHOW_CURSOR:
			case ID_WINMENU_SHOW_EXPORTERS:
			case ID_WINMENU_SHOW_TFXVIEW:
			case ID_WINMENU_SHOW_TFXVIEW_AUTO:
			case ID_WINMENU_SHOW_RTFOLIAGE:
			case ID_WINMENU_SHOW_RTFOLFILE:
				{
				ChangePrefs(ViewOrigin, MenuID);
				break;
				} // do something
			case ID_WINMENU_RENDER_TERRAIN:
			case ID_WINMENU_RENDER_SNOW:
			case ID_WINMENU_RENDER_TFX:
			case ID_WINMENU_RENDER_ATFX:
			case ID_WINMENU_RENDER_FOLIAGE:
			case ID_WINMENU_RENDER_FOLIAGEFX:
			case ID_WINMENU_RENDER_ECOFX:
			case ID_WINMENU_RENDER_COLORMAPS:
			case ID_WINMENU_RENDER_WATER:
			case ID_WINMENU_RENDER_STREAMS:
			case ID_WINMENU_RENDER_WAVES:
			case ID_WINMENU_RENDER_REFLECTIONS:
			case ID_WINMENU_RENDER_SKY:
			case ID_WINMENU_RENDER_HAZE:
			case ID_WINMENU_RENDER_CLOUDS:
			case ID_WINMENU_RENDER_VOLUMETRICS:
			case ID_WINMENU_RENDER_CELESTIALOBJECTS:
			case ID_WINMENU_RENDER_STARS:
			case ID_WINMENU_RENDER_LIGHTS:
			case ID_WINMENU_RENDER_CLOUDSHADOW:
			case ID_WINMENU_RENDER_SHADOWFX:
			case ID_WINMENU_RENDER_3DOBJSHADOW:
			case ID_WINMENU_RENDER_3DOBJS:
			case ID_WINMENU_RENDER_WALLS:
			case ID_WINMENU_RENDER_LABELS:
			case ID_WINMENU_RENDER_VECTORS:
			case ID_WINMENU_RENDER_DOF:
			case ID_WINMENU_RENDER_MULTIAA:
			case ID_WINMENU_RENDER_DIAG:
				{
				DoRenderOpt(ViewOrigin, MenuID);
				break;
				} // 

			case ID_WINMENU_RENDERPREV:
				{
				DoRender(ViewOrigin);
				break;
				} // RENDER
			case ID_WINMENU_SHOW_EDITRTF:
				{
				DoRTPrefs();
				break;
				} // ID_WINMENU_SHOW_EDITRTF
			case ID_WINMENU_CREATEFOLFILE:
				{
				BuildRealtimeFoliage(ViewOrigin);
				// reload new file if necessary
				if (RTFLoaded && !strcmp(RTFDispConf.BaseName, RTFWriteConf.BaseName))
					{
					FreeRealtimeFoliage();
					RTFLoaded = 0;
					PrepRealtimeFoliage(RTFDispConf.BaseName);
					Draw(-1);
					} // if
				break;
				} // ID_WINMENU_CREATEFOLFILE
			case ID_WINMENU_LOADFOLFILE:
				{
				if (!RTFLoaded)
					{
					PrepRealtimeFoliage(RTFDispConf.BaseName);
					} // if
				else
					{
					// unload
					FreeRealtimeFoliage();
					RTFLoaded = 0;
					} // else
				Draw(-1);
				break;
				} // ID_WINMENU_LOADFOLFILE

			} // switch
		} // else
	} // if

return(0);

} // ViewGUI::HandlePopupMenuSelect

/*===========================================================================*/

long ViewGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
int ViewNum;
ViewContext *OriVC = NULL;
NotifyTag DBUndoChanges[2];

CheckSpaceMode();

ViewNum = IdentActiveView();
if (ViewNum == -1) ViewNum = LastActiveView;

if (ButtonID == IDI_TB_LTDREGION)
	{
	#ifdef WCS_TEST_WATERFLOW
	DoCoolWaterStuff();
	#else // WCS_TEST_WATERFLOW
	if ((LastActiveView != -1) && (GetEnabled(LastActiveView, WCS_VIEWGUI_ENABLE_LTDREGION)))
		{
		HandlePopupMenuSelect(ID_WINMENU_OPTIONS_CONSTRAIN);
		} // if
	else
		{
		HandlePopupMenuSelect(ID_WINMENU_OPTIONS_SETAREA);
		} // else
	#endif // WCS_TEST_WATERFLOW
	} // else if
else if (ButtonID == IDI_TB_MEASURE)
	{
	if (GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_MEASURE))
		{
		StartMeasure();
		} // if
	else
		{
		DoCancelMeasure(0);
		} // else
	} // else if
else if (ButtonID == IDI_TB_UNDO)
	{ // UNDO
	// Warn everybody
	DBUndoChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
	DBUndoChanges[1] = NULL;
	LocalDB->GenerateNotify(DBUndoChanges, LocalDB->ActiveObj);
	// Do it
	InterStash->SetParam(1, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_BACKUP, WCS_INTERVEC_ITEM_UNDO, 0), 0);
	// update database
	LocalDB->ActiveObj->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
	LocalDB->ActiveObj->RecheckBounds();
	LocalDB->ActiveObj->ZeroUpTree();
	LocalDB->ReBoundTree(WCS_DATABASE_STATIC);
	// Tell 'em we did it.
	DBUndoChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	DBUndoChanges[1] = NULL;
	LocalDB->GenerateNotify(DBUndoChanges, LocalDB->ActiveObj);
	if (InterStash->GetPointOperate() == WCS_VECTOR_PTOPERATE_ALLPTS)
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);
	} // else
else if (ButtonID == IDI_TB_CREATE)
	{ // CREATE
	DoCreate(0);
	} // else
else if (ButtonID == IDI_TB_REPLACE)
	{ // REPLACE
	DoCreate(WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE);
	} // else
else if (ButtonID == IDI_TB_APPEND)
	{ // APPEND
	DoCreate(WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND);
	} // else
else if (ButtonID == IDI_TB_MOVE)
	{
	if (GetViewManipulationEnable())
		{
		SetViewManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_MOVE) ? WCS_VIEWGUI_MANIP_MOVE : 0);
		} // if
	else
		{
		SetObjectManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_MOVE) ? WCS_VIEWGUI_MANIP_MOVE : 0);
		} // else
	} // else
else if (ButtonID == IDI_TB_ROTATE)
	{
	if (GetViewManipulationEnable())
		{
		SetViewManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_ROTATE) ? WCS_VIEWGUI_MANIP_ROT : 0);
		} // if
	else
		{
		SetObjectManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_ROTATE) ? WCS_VIEWGUI_MANIP_ROT : 0);
		} // else
	} // else
else if (ButtonID == IDI_TB_SCALEZOOM)
	{
	if (GetViewManipulationEnable())
		{
		SetViewManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_SCALEZOOM) ? WCS_VIEWGUI_MANIP_SCALE : 0);
		} // if
	else
		{
		SetObjectManipulationMode(GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_SCALEZOOM) ? WCS_VIEWGUI_MANIP_SCALE : 0);
		} // else
	} // else
else if (ButtonID == IDI_TB_ZOOM)
	{
	if (ViewNum != -1)
		{
		OriVC = ViewSpaces[ViewNum];
		if (OriVC && OriVC->VCamera)
			{
			OriVC->VCamera->SetFloating(1);
			} // if
		} // if
	} // else
else if (ButtonID == IDI_TB_ZOOMBOX)
	{
	if (!ZoomInProgress)
		{
		StartZoom();
		} // if
	else
		{
		DoCancelZoom(0);
		} // else
	} // else
else if (ButtonID == IDI_TB_ZOOMWAYOUT)
	{
	if (ViewNum != -1)
		{
		OriVC = ViewSpaces[ViewNum];
		if (OriVC && OriVC->VCamera)
			{
			OriVC->VCamera->Zoom(-FLT_MAX);
			} // if
		} // if
	} // else
else if (ButtonID == IDI_TB_ZOOMRESTORE)
	{
	if (ViewNum != -1)
		{
		OriVC = ViewSpaces[ViewNum];
		if (OriVC && OriVC->VCamera)
			{
			if (OriVC->BigR && OriVC->BigR->ViewCamCopy)
				OriVC->BigR->ViewCamCopy->RestoreZoomProxy(OriVC->VCamera);
			else
				OriVC->VCamera->RestoreZoom();
			} // if
		} // if
	} // else
else if (ButtonID == IDI_TB_RENDERVIEW)
	{
	if (ViewNum != -1)
		{
		// if a rendering is already present, dismiss it
		if ((GetEnabled(LastActiveView, WCS_VIEWGUI_ENABLE_RENDERPREV)) && (ViewSpaces[LastActiveView]->BigR))
			{
			HandlePopupMenuSelect(ID_WINMENU_SHOWPREV);
			} // if
		else
			{ // otherwise make one!
			DoRender(-1);
			} // else
		} // if
	} // else
else if ((ButtonID == IDI_TB_XAXIS) || (ButtonID == IDI_TB_YAXIS) || (ButtonID == IDI_TB_ZAXIS) || (ButtonID == IDI_TB_ELEVAXIS))
	{
	AxisEnable[WCS_X] = GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_XAXIS); // x
	AxisEnable[WCS_Y] = GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_YAXIS); // y
	AxisEnable[WCS_Z] = GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_ZAXIS); // z
	AxisEnable[3] = GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_ELEVAXIS); // elev
	UpdateAxes();
	} // else
else if (ButtonID == IDI_TB_POINTSMODE)
	{
	InterStash->SetParam(1, MAKE_ID(WCS_INTERCLASS_MAPVIEW, WCS_INTERMAP_SUBCLASS_MISC, WCS_INTERMAP_ITEM_POINTSMODE, 0),
	 GlobalApp->MCP->GetToolbarButtonPressed(IDI_TB_POINTSMODE), 0);
	Draw();
	} // IDI_TB_POINTSMODE
else if (ButtonID == IDI_TB_APPEND)
	{
	DoDelete();
	} // IDI_TB_APPEND
else if ((ButtonID == IDI_CAP_MOVEVIEW) || (ButtonID == IDI_CAP_ROTVIEW) || (ButtonID == IDI_CAP_ZOOMVIEW))
	{
	if (ViewNum != -1)
		{
		unsigned long StateMask = NULL, InvStateMask = NULL;
		if (OriVC = ViewSpaces[ViewNum])
			{
			// determine which bit we're interested in
			if (ButtonID == IDI_CAP_MOVEVIEW) StateMask = WCS_VIEWGUI_VIEWSTATE_PANVIEW;
			if (ButtonID == IDI_CAP_ROTVIEW)  StateMask = WCS_VIEWGUI_VIEWSTATE_ROTVIEW;
			if (ButtonID == IDI_CAP_ZOOMVIEW) StateMask = WCS_VIEWGUI_VIEWSTATE_ZOOMVIEW;
			
			// determine current state of that bit
			bool CurState = (OriVC->ViewState & StateMask) ? true : false;
			
			// turn off all of the bits for mutual exclusion
			OriVC->ViewState &= ~(WCS_VIEWGUI_VIEWSTATE_VIEWMANIP);
			
			// alter only that state bit
			if (!CurState) // turn the bit off
				{ // turn the bit on
				OriVC->ViewState |= (StateMask);
				} // if
			} // if
		} // if
	ViewWins[ViewNum]->UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_PAN | WCS_FENETRE_WINSTATE_ROTATE | WCS_FENETRE_WINSTATE_ZOOM);
	} // IDI_CAP_MOVEVIEW/IDI_CAP_ROTVIEW/IDI_CAP_ZOOMVIEW

return(0);

} // ViewGUI::HandleButtonClick

/*===========================================================================*/

long ViewGUI::HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift)
{
ViewContext *OriVC = NULL;
unsigned short VWidth, VHeight;
GLDrawingFenetre *GLDF;
double LMLat, LMLon, Elev;
char DoRenderDiag = 0;
RasterAnimHost *NewActive;
RasterAnimHost *ActObj = NULL;


// identify which view it came from
if (OriginFen && OriginFen->CustomDataVoid[0])
	{
	GLDF = (GLDrawingFenetre *)OriginFen;
	GLDF->GetDrawingAreaSize(VWidth, VHeight);
	GLDF->CaptureInput();
	OriVC = (ViewContext *)OriginFen->CustomDataVoid[0];
	DoRenderDiag = (OriVC->BigR && OriVC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV));
	} // if

if (!OriVC || !OriVC->VCamera || !OriVC->GetAlive()) return(0);

CheckSpaceMode();

SetLastMouse(X, Y);
if (OriVC)
	{
	LatLonFromXY(OriVC, GLDF, X, Y, LMLat, LMLon);
	SetLastMouseLatLon(LMLat, LMLon);
	} // if

if (OriVC && ZoomInProgress)
	{
	DoZoomPoint(ViewNumFromVC(OriVC), X, Y);
	return(0);
	} // if
if (OriVC && MeasureInProgress)
	{
	double MLat, MLon, MElev;
	LatLonElevFromXY(OriVC, GLDF, X, Y, MLat, MLon, MElev);
	DoMeasurePoint(ViewNumFromVC(OriVC), MLat, MLon, MElev);
	return(0);
	} // if
if (OriVC && RegionInProgress)
	{
	DoRegionPoint(ViewNumFromVC(OriVC), X, Y);
	return(0);
	} // if

if (Alt && OriVC)
	{
	if (DoRenderDiag)
		{
		OriVC->BigR->SelectDiagnosticObject(X, Y);
		} // if
	else if (NewActive = DoSelectObject(OriVC, GLDF, X, Y, 0, 0))
		{
		RasterAnimHost::SetActiveRAHost(NewActive);
		} // if
	return(0);
	} // if

if (InterStash->MVPointsMode())
	{ // point select-deselect
	DoPointHit(OriVC, GLDF, X, Y, !Shift);
	Dif[0] = Dif[1] = Dif[2] = 0.0; // clear these to make sure they're not uninitialized if you do a move later (normally selectively done in BeginInteractive() below)
	JoeChanged = 0;
	return(0);
	} // if

if (Control && OriVC)
	{
	Elev = GlobalApp->MainProj->Interactive->ElevationPoint(LMLat, LMLon);
	Elev = OriVC->VCCalcExag(Elev);
	DoInteractivePlace(OriVC, GLDF, LMLat, LMLon, Elev, X, Y);
	return(0);
	} // if

ActObj = RasterAnimHost::GetActiveRAHost();

/*
if (GetViewManipulationEnable() && !DoRenderDiag)
	{
	} // if
*/
if (ActObj && (!GetViewManipulationEnable() && GetObjectManipulationMode() && !DoRenderDiag))
	{
	BeginInteractive();
	} // if
else
	{
	if (OriVC)
		{
		if (!Shift)
			{
			if ((DoRenderDiag) || ((!(GetViewManipulationEnable() && GetViewManipulationMode())) && !(OriVC->QueryViewState() & WCS_VIEWGUI_VIEWSTATE_VIEWMANIP)))
				{
				if (!InterStash->MVPointsMode())
					DoSampleDiag(OriVC, GLDF, X, Y, Alt, Control, Shift, 1);
				} // if
			} // if
		} // if
	} // if

return(0);

} // ViewGUI::HandleLeftButtonDown

/*===========================================================================*/

long ViewGUI::HandleRightButtonDown(long X, long Y, char Alt, char Control, char Shift)
{
ViewContext *OriVC = NULL;
//unsigned short VWidth, VHeight;
GLDrawingFenetre *GLDF;
double LMLat, LMLon;

// identify which view it came from
if (OriginFen && OriginFen->CustomDataVoid[0])
	{
	GLDF = (GLDrawingFenetre *)OriginFen;
	//GLDF->GetDrawingAreaSize(VWidth, VHeight);
	GLDF->CaptureInput();
	OriVC = (ViewContext *)OriginFen->CustomDataVoid[0];
	} // if

CheckSpaceMode();

SetLastMouse(X, Y);
if (OriVC && OriVC->GetAlive())
	{
	LatLonFromXY(OriVC, GLDF, X, Y, LMLat, LMLon);
	SetLastMouseLatLon(LMLat, LMLon);

	#ifdef WCS_VIEW_RIGHTCLICK_DRILLDOWN
	RasterAnimHost *ActObj = NULL;
	unsigned char ObjectManipActive = 0;
	char Xen, Yen, Zen, Move, Rot, Scale, Points;
	Points = Move = Rot = Scale = Xen = Yen = Zen = 0; // in case no active object
	if (ActObj = RasterAnimHost::GetActiveRAHost())
		{
		FetchObjectActionsAxes(ActObj, Move, Rot, Scale, Xen, Yen, Zen, Points);
		if (ObjectManipActive = GetObjectManipulationMode())
			{
			if ((ObjectManipActive == WCS_VIEWGUI_MANIP_MOVE && !Move) ||
			 (ObjectManipActive == WCS_VIEWGUI_MANIP_ROT && !Rot) ||
			 (ObjectManipActive == WCS_VIEWGUI_MANIP_SCALE && !Scale))
				ObjectManipActive = WCS_VIEWGUI_MANIP_NONE;
			} // if
		} // if
	if (! GetViewManipulationEnable() && ! ObjectManipActive && InterStash && LocalDB && ! AppScope->GUIWins->DIG && !(OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_VIEWMANIP))
		{
		#ifdef WCS_BUILD_VNS
		if (OriVC->IsPlan())
			{
			if (OriVC->GetProjected())
				{
				VertexDEM Original;
				Original.xyz[0] = LMLon + DBCenter.XYZ[0];
				Original.xyz[1] = LMLat + DBCenter.XYZ[1];
				Original.xyz[2] = 0.0;
				// convert to lat/lon
				OriVC->GetProjected()->ProjToDefDeg(&Original);
				// LMLat/LMLon will be real lat/lon after this
				LMLat = Original.Lat;
				LMLon = Original.Lon;
				} // if
			} // if Plan
		#endif // WCS_BUILD_VNS
		InterStash->DrillDownPoint(LMLat, LMLon);
		} // if
	#endif // WCS_VIEW_RIGHTCLICK_DRILLDOWN
	} // if

if (!GetViewManipulationEnable() && GetObjectManipulationMode())
	{
	BeginInteractive();
	} // if

return(0);

} // ViewGUI::HandleRightButtonDown

/*===========================================================================*/

long ViewGUI::HandleRightButtonUp(long X, long Y, char Alt, char Control, char Shift)
{
CheckSpaceMode();
DrawingFenetre::ReleaseInput();
ClearLastMouse();

if (GetViewManipulationEnable())
	{
	} // if
else
	{
	if (!GetViewManipulationEnable() && GetObjectManipulationMode())
		{
		EndInteractive();
		} // if
	if (AppScope->GUIWins->DIG)
		{
		EndCreate();
		} // if
	} // else

return(0);

} // ViewGUI::HandleRightButtonUp

/*===========================================================================*/

long ViewGUI::HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift)
{
CheckSpaceMode();
DrawingFenetre::ReleaseInput();
ClearLastMouse();

DigitizeOneShot = 0;

if (!GetViewManipulationEnable() && GetObjectManipulationMode())
	{
	EndInteractive();
	} // if

return(0);

} // ViewGUI::HandleLeftButtonUp

/*===========================================================================*/

long ViewGUI::HandleLeftButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift)
{
ViewContext *OriVC = NULL;
RasterAnimHost *EditMe = NULL;
GLDrawingFenetre *GLDF;

CheckSpaceMode();

/*
if (GetViewManipulationEnable())
	{
	} // if
else
*/
	{
	if (OriginFen && OriginFen->CustomDataVoid[0])
		{
		if ((OriVC = (ViewContext *)OriginFen->CustomDataVoid[0]) && (OriVC->GetAlive()))
			{
			GLDF = (GLDrawingFenetre *)OriginFen;
			if (OriVC->BigR && OriVC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
				{
				// suppress double-click-to-edit while digitizing or DEMPainting
				if (!((GlobalApp->GUIWins->DIG) || (GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview())))
					{
					OriVC->BigR->EditDiagnosticObject(X, Y);
					} // if
				} // if
			else
				{
				// not while DEMPainting
				if (!(GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->ShowPreview()))
					{
					if (EditMe = DoSelectObject(OriVC, GLDF, X, Y, 0, 0))
						{
						(EditMe->GetRAHostRoot())->EditRAHost();
						} // if
					} // if
				} // else
			} // if
		} // if
	} // else

return (0);

} // ViewGUI::HandleLeftButtonDoubleClick

/*===========================================================================*/

int ViewControlQualifier, ViewShiftQualifier;

long ViewGUI::HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
ViewContext *OriVC = NULL;
//unsigned short VWidth, VHeight;
long XDif, YDif;
GLDrawingFenetre *GLDF;
char DoRenderDiag = 0;
unsigned long ShiftCode = 0;
int ViewNum = -1;
unsigned short VWidth, VHeight;
RasterAnimHost *ActObj = NULL;

ViewControlQualifier = Control;
ViewShiftQualifier   = Shift;

// identify which view it came from
if (OriginFen && OriginFen->CustomDataVoid[0])
	{
	GLDF = (GLDrawingFenetre *)OriginFen;
	//GLDF->GetDrawingAreaSize(VWidth, VHeight);
	OriVC = (ViewContext *)OriginFen->CustomDataVoid[0];
	ViewNum = ViewNumFromVC(OriVC);
	DoRenderDiag = (OriVC->BigR && OriVC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV));
	} // if

if (ViewNum == -1 || ViewSpaces[ViewNum] == NULL || ViewWins[ViewNum] == NULL)
	{
	return(0);
	} // if

if (ZoomInProgress && OriVC && OriVC->GetAlive() && (ViewNum != -1) && (ViewNum == ZoomView))
	{
	if (ZoomCoord[0] != -1)
		{
		ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);
		ViewWins[ViewNum]->SyncBackground(0, 0, VWidth, VHeight);
		DrawTempBox(ViewNum, (unsigned short)ZoomCoord[0], (unsigned short)ZoomCoord[1], (unsigned short)X, (unsigned short)Y);
		} // if
	} // if
else if (RegionInProgress && OriVC && (ViewNum != -1) && (ViewNum == RegionView))
	{
	if (RegionCoord[0] != -1)
		{
		ViewWins[ViewNum]->GetDrawingAreaSize(VWidth, VHeight);
		ViewWins[ViewNum]->SyncBackground(0, 0, VWidth, VHeight);
		DrawTempBox(ViewNum, (unsigned short)RegionCoord[0], (unsigned short)RegionCoord[1], (unsigned short)X, (unsigned short)Y);
		} // if
	} // if

CheckSpaceMode();

ActObj = RasterAnimHost::GetActiveRAHost();

if (!DoRenderDiag)
	{
	if (Shift) ShiftCode = WCS_DOINTER_CODE_SHIFT;
	if (Alt) return(0);
	if (OriVC && OriVC->VCamera && OriVC->VCamera->CamPos)
		{
		if (Left)
			{
			if (MouseDif (XDif = X, YDif = Y))  //lint !e1058 // Watch those args carefully, they're not what they seem.
				{
				if ((ViewSpaces[ViewNum]->ViewState & WCS_VIEWGUI_VIEWSTATE_VIEWMANIP) || (ActObj && ((GetViewManipulationEnable() && GetViewManipulationMode()) || (!GetViewManipulationEnable() && GetObjectManipulationMode()))))
					{
					DoInteractive(OriVC, GLDF, XDif, YDif, 0, 0, ShiftCode);
					} // if
				else
					{
					if (!InterStash->MVPointsMode()) DoSampleDiag(OriVC, GLDF, X, Y, Alt, Control, Shift, 0);
					} // else
				} // if
			} // if
		else if (Right)
			{
			if (MouseDif (XDif = X, YDif = Y)) //lint !e1058 // Watch those args carefully, they're not what they seem.
				{
				DoInteractive(OriVC, GLDF, 0, 0, XDif, YDif, ShiftCode);
				} // if
			} // else if
		} // if
	SetLastMouse(X, Y);
	} // if
else
	{
	if (OriVC && Left)
		{
		if (Alt)
			{
			// too slow to allow this
			//OriVC->BigR->SelectDiagnosticObject(X, Y);
			} // if
		else
			{
			if (!InterStash->MVPointsMode()) DoSampleDiag(OriVC, GLDF, X, Y, Alt, Control, Shift, 0);
			} // if
		} // if
	} // else

ViewControlQualifier = 0;
ViewShiftQualifier   = 0;

return(0);

} // ViewGUI::HandleMouseMove

/*===========================================================================*/

void ViewGUI::ActiveItemSwitch(void)
{
int RestoreBackup = 0, StoreBackup = 0, Mode, ActiveView;
RasterAnimHost *NewActive = NULL;
Camera *ActiveCam;

if (! RasterAnimHost::GetActiveLock())
	{
	if (GetViewManipulationEnable())
		{
		if ((ActiveView = IdentActiveView()) >= 0)
			{
			Mode = GetViewManipulationMode();
			ActiveCam = ViewSpaces[ActiveView]->VCamera;

			if (Mode == WCS_VIEWGUI_MANIP_NONE)
				{
				RestoreBackup = 1;
				} // if
			else if (Mode == WCS_VIEWGUI_MANIP_MOVE)
				{
				StoreBackup = 1;
				if (ActiveCam)
					NewActive = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
				} // else if
			else if (Mode == WCS_VIEWGUI_MANIP_ROT)
				{
				StoreBackup = 1;
				if (ActiveCam)
					NewActive = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HEADING);
				} // else if
			else if (Mode == WCS_VIEWGUI_MANIP_SCALE)
				{
				StoreBackup = 1;
				if (ActiveCam)
					NewActive = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
				} // else if
			} // if
		} // if
	else
		{
		RestoreBackup = 1;
		} // else
	if (RestoreBackup && RasterAnimHost::GetBackupRAHost())
		{
		RasterAnimHost::RestoreBackupRAHost();
		} // if
	else if (StoreBackup && NewActive)
		{
		if (! RasterAnimHost::GetBackupRAHost())
			RasterAnimHost::SetBackupRAHost(NewActive);
		else
			RasterAnimHost::SetActiveRAHostNoBackup(NewActive);
		} // else
	} // if

} // ViewGUI::ActiveItemSwitch

/*===========================================================================*/

RasterAnimHost *ViewGUI::DoSelectObject(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, int PointHit, int ClearPoints)
{
unsigned short Width, Height;

{ // test for debug_select
char *QResult;
DebugSelect = 0;
if (QResult = GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("debug_select"))
	{
	if (tolower(QResult[0]) == 'y')
		{
		DebugSelect = 1;
		} // 
	} // if
} // test for debug_select

NewSelected = NULL;

if (DebugSelect) GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Entering DoSelect.");

if (GLDF && OriVC)
	{
	GLDF->GetDrawingAreaSize(Width, Height);

	PickMatrix[0] = (double)X;
	PickMatrix[1] = (double)(Height - Y);
	PickMatrix[2] = 1.0;
	PickMatrix[3] = 1.0;
	if (PointHit)
		{
		ClearPointsPick = ClearPoints;
		PickEnabled = 2;
		} // if
	else
		{
		PickEnabled = 1;
		} // else

	DrawView(ViewNumFromVC(OriVC), OriVC);
	} // if

ClearPointsPick = 0;
if (DebugSelect) GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Exiting DoSelect.");
DebugSelect = 0;
PickEnabled = 0;
return(NewSelected);

} // ViewGUI::DoSelectObject

/*===========================================================================*/

void ViewGUI::DoAnim(int ViewNum)
{
BusyWin *FrameGauge = NULL;
int Count = 0, MaxFrame;
char Ptrn[30], filenum[30], OutPath[500];

#ifdef WCS_BUILD_DEMO
UserMessageDemo("Quick Sequences are limited to 5 seconds.");
#endif // WCS_BUILD_DEMO


strcpy(Ptrn, WCS_REQUESTER_WILDCARD);
if (! GetFileNamePtrn(1, "Save Quick Sequence:", GlobalApp->MainProj->animpath, GlobalApp->MainProj->animfile, Ptrn, 64))
	{
	return;
	} // if

GlobalApp->MainProj->Interactive->SetActiveTime(0.0);
MaxFrame = GlobalApp->MCP->GetMaxFrame();

FrameGauge = new BusyWin("Quick Sequence", MaxFrame + 1, 'BUSY', 0);

//ViewWins[ViewNum]->JumpTop();

while (GlobalApp->MainProj->Interactive->GetActiveFrame() < MaxFrame + 1)
	{
	if (FrameGauge)
		if (FrameGauge->Update(Count))
			{
			break;
			} // if
		strmfp(OutPath, GlobalApp->MainProj->animpath, GlobalApp->MainProj->animfile);
		sprintf(filenum, "%04d.bmp", Count);
		strcat(OutPath, filenum);

#ifdef WCS_BUILD_DEMO
		{
		long NextQSFrame;
		double NextQSTime;

		NextQSFrame = (GlobalApp->MainProj->Interactive->GetActiveFrame() + 1);
		NextQSTime = (GlobalApp->MainProj->Interactive->ProjFrameRate > 0.0 ? NextQSFrame / GlobalApp->MainProj->Interactive->ProjFrameRate: 0.0);
		if (NextQSTime > 5.0)
			{
			break;
			} // if
		} // temp scope
#endif // WCS_BUILD_DEMO

	GlobalApp->MainProj->Interactive->SetActiveFrame(GlobalApp->MainProj->Interactive->GetActiveFrame() + 1);
	if (!ViewWins[ViewNum]->SaveContentsToFile(OutPath))
		{ // error
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to save Quick Sequence.");
		break;
		} // if
	Count ++;
	} // while

if (FrameGauge)
	{
	delete FrameGauge;
	FrameGauge = NULL;
	} // if

} // ViewGUI::DoAnim

/*===========================================================================*/

void ViewGUI::DoSaveImage(int ViewNum)
{
char Ptrn[30], filenum[30], OutPath[500];

strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD(".bmp"));

if (! GetFileNamePtrn(1, "Save Displayed Image:", GlobalApp->MainProj->animpath, GlobalApp->MainProj->animfile, Ptrn, 64))
	{
	return;
	} // if

//ViewWins[ViewNum]->JumpTop();

strmfp(OutPath, GlobalApp->MainProj->animpath, GlobalApp->MainProj->animfile);
sprintf(filenum, ".bmp");
strcat(OutPath, filenum);
if (!ViewWins[ViewNum]->SaveContentsToFile(OutPath))
	{ // error
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to save Quick Sequence.");
	} // if

} // ViewGUI::DoSaveImage

/*===========================================================================*/

int ViewGUI::VecCtrlScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
Joe *MiJo;
float MiJoMax = 0.0f, MiJoMin = 0.0f;
Point3d OrigLoc, MidPoint;

DiagnosticData *Diag;

if (MoveMe && Data)
	{
	Diag = Data;
	MiJo = (Joe *)MoveMe;
	// Establish our current 'midpoint' for distance purposes
	MiJo->GetElevRange(MiJoMax, MiJoMin);
	MidPoint[0] = MiJo->GetNSCenter();
	MidPoint[1] = MiJo->GetEWCenter();
	MidPoint[2] = (MiJoMax + MiJoMin) * 0.5;
	Diag->ValueValid[WCS_DIAGNOSTIC_LATITUDE]  = 1;
	Diag->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Diag->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	switch(Operation)
		{
		case WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV:
		case WCS_RAHOST_INTERACTIVEOP_MOVEXYZ:
			{
			OrigLoc[0] = Diag->Value[WCS_DIAGNOSTIC_LATITUDE]  = MidPoint[0];
			OrigLoc[1] = Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] = MidPoint[1];
			OrigLoc[2] = Diag->Value[WCS_DIAGNOSTIC_ELEVATION] = MidPoint[2];
			// Calculate how far to move
			ScaleMotion(Data);
			// Calculate how far we moved in each direction
			Dif[0] += Diag->Value[WCS_DIAGNOSTIC_LATITUDE]  - OrigLoc[0];
			Dif[1] += Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] - OrigLoc[1];
			Dif[2] += Diag->Value[WCS_DIAGNOSTIC_ELEVATION] - OrigLoc[2];

			if (!((Dif[0] == 0.0) && (Dif[1] == 0.0) && (Dif[2] == 0.0)))
				{
				InterStash->SetParam(1,
				 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0),
				 Dif[1], Dif[0], Dif[2], 0);
				TempVec = MiJo;
				JoeChanged = 1;
				Draw();
				} // if
			break;
			} // 
		case WCS_RAHOST_INTERACTIVEOP_SETPOS:
			{
			Dif[0] += Diag->Value[WCS_DIAGNOSTIC_LATITUDE]  - MidPoint[0];
			Dif[1] += Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] - MidPoint[1];
			Dif[2] += Diag->Value[WCS_DIAGNOSTIC_ELEVATION] - MidPoint[2];
			if (!((Dif[0] == 0.0) && (Dif[1] == 0.0)))
				{
				InterStash->SetParam(1,
				 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLAT, 0),
				 Dif[1], Dif[0], 0);
				TempVec = MiJo;
				JoeChanged = 1;
				Draw();
				} // if
			break;
			} // 
		case WCS_RAHOST_INTERACTIVEOP_ROTATE:
			{
			Dif[0] += Data->MoveX * .5;
			InterStash->SetParam(1,
			 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_ROTATEPTDEG, 0),
			 Dif[0], 0);
			TempVec = MiJo;
			JoeChanged = 1;
			Draw();
			break;
			} // 
		case WCS_RAHOST_INTERACTIVEOP_SCALE:
			{
			Dif[0] += (double)Data->MoveX * .1;
			Dif[1] += (double)Data->MoveY * .1;
			if (!((Dif[0] == 0) && (Dif[1] == 0)))
				{
				InterStash->SetParam(1,
				 MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SCALEPTLONLAT, 0),
				 Dif[0], Dif[1], 0);
				} // if
			TempVec = MiJo;
			JoeChanged = 1;
			Draw();
			break;
			} // 
		} // Operation
	} // if
return(0);

} // ViewGUI::VecCtrlScaleMoveRotate

/*===========================================================================*/

void ViewGUI::BeginInteractive(void)
{

Dif[2] = Dif[1] = Dif[0] = 0.0;
JoeChanged = 0;

} // ViewGUI::BeginInteractive

/*===========================================================================*/

void ViewGUI::EndInteractive(void)
{
NotifyTag GenericChanges[2];

if (JoeChanged)
	{
	GenericChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	GenericChanges[1] = NULL;
	LocalDB->ActiveObj->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
	LocalDB->ActiveObj->RecheckBounds();
	LocalDB->ActiveObj->ZeroUpTree();
	LocalDB->ReBoundTree(WCS_DATABASE_STATIC);
	LocalDB->GenerateNotify(GenericChanges, LocalDB->ActiveObj);

	if (InterStash->GetTfxRealtime())
		{
		JoeAttribute *JA;
		GeneralEffect *GE;
		if (LocalDB->ActiveObj)
			{
			if (JA = LocalDB->ActiveObj->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR))
				{
				if (GE = LocalDB->ActiveObj->GetAttributeEffect(JA))
					{
					if (GE->GetEnabled())
						{
						DrawForceRegen(1, 1);
						} // if
					} // if
				} // if
			if (JA = LocalDB->ActiveObj->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR))
				{
				if (GE = LocalDB->ActiveObj->GetAttributeEffect(JA))
					{
					if (GE->GetEnabled())
						{
						DrawForceRegen(1, 1);
						} // if
					} // if
				} // if
			} // if
		} // if
	else
		{
		DrawForceRegen(1, 0);
		} // if
	JoeChanged = 0;
	} // if

Dif[2] = Dif[1] = Dif[0] = 0.0;

} // ViewGUI::EndInteractive

/*===========================================================================*/

static DiagnosticData InterData;
void ViewGUI::DoInteractive(ViewContext *OriVC, GLDrawingFenetre *GLDF, long XDif, long YDif, long ZXDif, long ZYDif, unsigned long Code)
{
double CX, CY, CZ, MpD, MoveDegLat, MoveDegLon, MoveDist, ArcScale = 0.5, HFOV, TerrainElev, TerrainOffset;
int Changed = 0, IsVectorOrControlPt = 0, OrigLock, ForceElev = 0;
VertexDEM CurPos, NewPos;
RasterAnimHost *ActObj = NULL;
RasterAnimHostProperties RAHP;
unsigned short Width, Height;
long ViewNum;
char WeLocked = 0;
CoordSys *DefCoords;

DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (!OriVC || ! OriVC->VCamera || ! OriVC->VCamera->CamPos || !GLDF) return;

// Stash code here so ScaleMotion can use it shortly
InterCode = Code;

MpD = LatScale(OriVC->GetRadius());

// Lock out stutter during the blizzard of param chanes
if (! (OrigLock = RasterAnimHost::GetActiveLock()))
	RasterAnimHost::SetActiveLock(1);
if (!GetInterLock())
	{
	WeLocked = 1;
	SetInterLock(1);
	} // if

if (GetViewManipulationEnable() || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_VIEWMANIP))
	{
	if (GetViewManipulationMode() == WCS_VIEWGUI_MANIP_MOVE || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
		{
		CurPos.Lat  = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue(0);
		CurPos.Lon  = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue(0);
		CurPos.Elev = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue(0);
		if (OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && (ViewNum = ViewNumFromVC(OriVC)) >= 0)
			{
			ViewWins[ViewNum]->GetDrawingAreaSize(Width, Height);
			MoveDist = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0) / Width;
			if (fabs(CurPos.Lat) < 90.0)
				MoveDegLon = MoveDist / MpD * cos(CurPos.Lat * PiOver180);
			else
				MoveDegLon = 0.1;
			MoveDegLat = MoveDist / MpD;
			} // if
		else
			{
			MoveDist = HMotionScaleFactor * .1;
			MoveDegLon = MoveDegLat = MoveDist / MpD;
			} // else
		if ((GlobalApp->MainProj->Prefs.InteractiveStyle == WCS_INTERACTIVE_STYLE_LIGHTWAVE && ! (InterCode == WCS_DOINTER_CODE_SHIFT))
			|| (GlobalApp->MainProj->Prefs.InteractiveStyle == WCS_INTERACTIVE_STYLE_MAX && InterCode == WCS_DOINTER_CODE_SHIFT))
			{
			if (XDif)
				{
				Changed = 1;
				if (GetAxisEnable(WCS_X) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
					CurPos.Lon += ((double)XDif * MoveDegLon) * OriVC->VCamera->IARightVec2D[0];
				if (GetAxisEnable(WCS_Y) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
					CurPos.Lat += ((double)XDif * MoveDegLat) * OriVC->VCamera->IARightVec2D[1];
				} // if
			if (YDif)
				{
				Changed = 1;
				if (GetAxisEnable(WCS_X) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
					CurPos.Lon -= ((double)YDif * MoveDegLon) * OriVC->VCamera->IAUpVec2D[0];
				if (GetAxisEnable(WCS_Y) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
					CurPos.Lat -= ((double)YDif * MoveDegLat) * OriVC->VCamera->IAUpVec2D[1];
				} // if
			if (ZYDif)
				{
				Changed = 1;
				ForceElev = 1;
				if (GetAxisEnable(WCS_Z) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
					{
					if (OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
						{
						OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetCurValue(
							OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0) - ((double)ZYDif * MoveDist));
						} // if
					else
						{
						CurPos.Elev -= ((double)ZYDif * MoveDist); // add elevation
						} // else
					} // if
				} // if
			if (Changed)
				{
				if (OriVC->VCamera->InterElevFollow && ! ForceElev && OriVC->Planet)
					{
					TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
						OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
					TerrainElev = OriVC->VCCalcExag(TerrainElev);
					TerrainOffset = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - TerrainElev;
					} // if
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(CurPos.Lon);
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(CurPos.Lat);
				if (OriVC->VCamera->InterElevFollow && ! ForceElev && OriVC->Planet)
					{
					TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
						OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
					TerrainElev = OriVC->VCCalcExag(TerrainElev);
					OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(TerrainOffset + TerrainElev);
					} // if
				else
					OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(CurPos.Elev);
				} // if
			} // if
		else
			{
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&CurPos);
			#else // WCS_BUILD_VNS
			CurPos.DegToCart(OriVC->GetRadius());
			#endif // WCS_BUILD_VNS
			CopyPoint3d(NewPos.XYZ, CurPos.XYZ);
			if (XDif && GetAxisEnable(WCS_X) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
				{
				Changed = 1;
				NewPos.XYZ[0] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[0]; // add scaled CamRight vector
				NewPos.XYZ[1] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[1]; // add scaled CamRight vector
				NewPos.XYZ[2] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[2]; // add scaled CamRight vector
				} // if
			if (YDif && GetAxisEnable(WCS_Y) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
				{
				Changed = 1;
				NewPos.XYZ[0] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[0]; // add scaled CamVertical vector
				NewPos.XYZ[1] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[1]; // add scaled CamVertical vector
				NewPos.XYZ[2] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[2]; // add scaled CamVertical vector
				} // if
			if (ZYDif && GetAxisEnable(WCS_Z) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW))
				{
				Changed = 1;
				NewPos.XYZ[0] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[0]; // add scaled TargPos vector
				NewPos.XYZ[1] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[1]; // add scaled TargPos vector
				NewPos.XYZ[2] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[2]; // add scaled TargPos vector
				} // if
			if (Changed)
				{
				#ifdef WCS_BUILD_VNS
				DefCoords->CartToDeg(&NewPos);
				#else // WCS_BUILD_VNS
				NewPos.CartToDeg(OriVC->GetRadius());
				#endif // WCS_BUILD_VNS
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(NewPos.Lon);
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(NewPos.Lat);
				if (GetAxisEnable(3))
					{
					OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(NewPos.Elev);
					} // if
				} // if
			} // if
		} // if
	else if (GetViewManipulationMode() == WCS_VIEWGUI_MANIP_ROT || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_ROTVIEW))
		{
		HFOV = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue(0);
		ArcScale = (HFOV / 180.0);
		if (!(OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC))
			{
			if (XDif && (GetAxisEnable(WCS_X) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW)) && !(OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD))
				{
				Changed = 1;
				CX = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetCurValue(0);
				CX += (double)XDif * ArcScale; // heading on X
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetCurValue(CX);
				} // if
			if (YDif && (GetAxisEnable(WCS_Y) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW)) && !(OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD))
				{
				Changed = 1;
				CY = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetCurValue(0);
				CY += (double)YDif * ArcScale; // pitch on Y
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetCurValue(CY);
				} // if
			if (ZXDif && (GetAxisEnable(WCS_Z) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW)))
				{
				Changed = 1;
				CZ = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetCurValue(0);
				CZ += (double)ZXDif * ArcScale; // bank on X
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetCurValue(CZ);
				} // if
			} // if
		} // if
	else if (GetViewManipulationMode() == WCS_VIEWGUI_MANIP_SCALE || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_ZOOMVIEW))
		{
		if (YDif && (GetAxisEnable(WCS_Y) || (OriVC->ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW)))
			{
			Changed = 1;
			CY = YDif / 100.0;
			if (CY < 0.0)
				CY = 1.0 / (1.0 - CY);
			else
				CY += 1.0;
			if (CY < .5)
				CY = .5;
			if ((OriVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) || (OriVC->VCamera->Orthographic))
				{ // planimetric/ortho cameras, modify View Width instead of HFOV
				CY *= OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0);
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetCurValue(CY);
				} // if
			else
				{ // non-plan/ortho cameras
				CY *= OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue(0);
				OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetCurValue(CY);
				} // else
			} // if
		} // if
	} // if
else
	{
	InterData.RefLat = InterData.RefLon = 0.0;
	InterData.PixelX = InterData.PixelY = InterData.PixelZ = 0;
	InterData.MoveX = 0;
	InterData.MoveY = 0;
	InterData.MoveZ = 0;
	InterData.ViewSource = ViewNumFromVC(OriVC);
	Changed = 0;
	if (ActObj = RasterAnimHost::GetActiveRAHost())
		{
		RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
		ActObj->GetRAHostProperties(&RAHP);
		IsVectorOrControlPt = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);

		if ((GetObjectManipulationMode() == WCS_VIEWGUI_MANIP_MOVE) && (RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ)))
			{
			CurPos.Lat  = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue(0);
			CurPos.Lon  = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue(0);
			CurPos.Elev = OriVC->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue(0);
			// VMotionScaleFactor;
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&CurPos);
			#else // WCS_BUILD_VNS
			CurPos.DegToCart(OriVC->GetRadius());
			#endif // WCS_BUILD_VNS
			CopyPoint3d(NewPos.XYZ, CurPos.XYZ);
			if (XDif && GetAxisEnable(WCS_X) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_MOVEX))
				{
				InterData.MoveX = XDif; Changed = 1;
				} // if
			if (YDif && GetAxisEnable(WCS_Y) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_MOVEY))
				{
				InterData.MoveY = YDif; Changed = 1;
				} // if
			if (ZYDif && GetAxisEnable(WCS_Z) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_MOVEZ))
				{
				InterData.MoveZ = ZYDif; Changed = 1;
				} // if
			if (Changed)
				{
				if (Code == WCS_DOINTER_CODE_SHIFT)
					{ // move on global axes
					if (IsVectorOrControlPt)
						{
						VecCtrlScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV);
						} // if
					else
						{
						ActObj->ScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV);
						} // else
					} // if
				else
					{
					if (IsVectorOrControlPt)
						{
						VecCtrlScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_MOVEXYZ);
						} // if
					else
						{
						if ((ViewNum = ViewNumFromVC(OriVC)) >= 0)
							{
							ViewWins[ViewNum]->GetDrawingAreaSize(Width, Height);
							InterData.DimX = Width;
							InterData.DimY = Height;
							} // if
						ActObj->ScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_MOVEXYZ);
						} // else
					} // else
				} // if
			} // if
		else if ((GetObjectManipulationMode() == WCS_VIEWGUI_MANIP_ROT) && (RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_ROTATEY | WCS_RAHOST_INTERBIT_ROTATEZ)))
			{
			if (XDif && GetAxisEnable(WCS_X) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_ROTATEX))
				{
				InterData.MoveX = XDif; Changed = 1;
				} // if
			if (YDif && GetAxisEnable(WCS_Y) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_ROTATEY))
				{
				InterData.MoveY = YDif; Changed = 1;
				} // if
			if (ZXDif && GetAxisEnable(WCS_Z) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_ROTATEZ))
				{
				InterData.MoveZ = ZXDif; Changed = 1;
				} // if
			if (Changed)
				{
				if (IsVectorOrControlPt)
					{
					VecCtrlScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_ROTATE);
					} // if
				else
					{
					ActObj->ScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_ROTATE);
					} // else
				} // if
			} // if
		else if ((GetObjectManipulationMode() == WCS_VIEWGUI_MANIP_SCALE) && (RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_SCALEX | WCS_RAHOST_INTERBIT_SCALEY | WCS_RAHOST_INTERBIT_SCALEZ)))
			{
			// if user holds down SHIFT key, we will apply the XDif isometrically to all three axes (if enabled)
			if(GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_SHIFT))
				{
				// the positive-negative sense of these axes differ, so we must handle them separately
				ZYDif = ZXDif = XDif;
				YDif = -XDif;
				} // if
			if (XDif && GetAxisEnable(WCS_X) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_SCALEX))
				{
				InterData.MoveX = XDif; Changed = 1;
				} // if
			if (YDif && GetAxisEnable(WCS_Y) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_SCALEY))
				{
				InterData.MoveY = YDif; Changed = 1;
				} // if
			if (ZXDif && GetAxisEnable(WCS_Z) && RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_SCALEZ))
				{
				InterData.MoveZ = ZYDif; Changed = 1;
				} // if
			if (Changed)
				{
				if (IsVectorOrControlPt)
					{
					VecCtrlScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_SCALE);
					} // if
				else
					{
					ActObj->ScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_SCALE);
					} // else
				} // if
			} // if
		} // if
	} // else

if (WeLocked)
	{
	SetInterLock(0);
	} // if
if (! OrigLock)
	RasterAnimHost::SetActiveLock(0);

// redraw the stuff we neglected to redraw above
if (Changed && WeLocked)
	{
	DrawImmediately();
	} // if

} // ViewGUI::DoInteractive

/*===========================================================================*/

void ViewGUI::DoInteractivePlace(ViewContext *OriVC, GLDrawingFenetre *GLDF, double Lat, double Lon, double Elev, long X, long Y)
{
RasterAnimHost *ActObj = NULL;
RasterAnimHostProperties RAHP;
int Changed = 0, IsVectorOrControlPt = 0, OrigLock;
unsigned short Width, Height;

if (!OriVC || !GLDF) return;

// Lock out stutter during the blizzard of param chanes
if (! (OrigLock = RasterAnimHost::GetActiveLock()))
	RasterAnimHost::SetActiveLock(1);
SetInterLock(1);

InterData.PixelX = X;
InterData.PixelY = Y;
InterData.PixelZ = 0;
InterData.MoveX = 0;
InterData.MoveY = 0;
InterData.MoveZ = 0;
InterData.ViewSource = ViewNumFromVC(OriVC);
if (InterData.ViewSource >= 0)
	{
	ViewWins[InterData.ViewSource]->GetDrawingAreaSize(Width, Height);
	InterData.DimX = Width;
	InterData.DimY = Height;
	} // if
Changed = 0;

if (ActObj = RasterAnimHost::GetActiveRAHost())
	{
	RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	ActObj->GetRAHostProperties(&RAHP);
	IsVectorOrControlPt = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);
	// click-to-set
	if (RAHP.TestInterFlags(WCS_RAHOST_INTERBIT_CLICKTOPOS))
		{
		Elev = GlobalApp->MainProj->Interactive->ElevationPoint(Lat, Lon);
		Elev = OriVC->VCCalcExag(Elev);
		InterData.Value[WCS_DIAGNOSTIC_LATITUDE]  = Lat;  InterData.ValueValid[WCS_DIAGNOSTIC_LATITUDE]  = 1;
		InterData.Value[WCS_DIAGNOSTIC_LONGITUDE] = Lon;  InterData.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
		InterData.Value[WCS_DIAGNOSTIC_ELEVATION] = Elev; InterData.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
		if (IsVectorOrControlPt)
			{
			VecCtrlScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_SETPOS);
			} // if
		else
			{
			ActObj->ScaleMoveRotate(ActObj, &InterData, WCS_RAHOST_INTERACTIVEOP_SETPOS);
			} // else
		Changed = 1;
		} // if
	} // if

SetInterLock(0);
if (! OrigLock)
	RasterAnimHost::SetActiveLock(0);

// redraw the stuff we neglected to redraw above
if (Changed)
	{
	Draw();
	} // if

} // ViewGUI::DoInteractivePlace

/*===========================================================================*/

void ViewGUI::CheckSpaceMode(void)
{
char Space;

Space = (GetAsyncKeyState(VK_SPACE) & ~0x01 ? 1 : 0);

if (Space)
	{
	if (!GetViewManipulationEnable())
		{
		SetViewManipulationEnable(WCS_VIEWGUI_MANIPSTATE_VIEW);
		} // if
	} // if
else
	{
	if (GetViewManipulationEnable() == WCS_VIEWGUI_MANIPSTATE_VIEW) // won't transition off of WCS_VIEWGUI_MANIPSTATE_VIEW_STUCK
		{
		SetViewManipulationEnable(WCS_VIEWGUI_MANIPSTATE_OBJECT);
		} // if
	} // else

} // ViewGUI::CheckSpaceMode

/*===========================================================================*/

void ViewGUI::DoCancelMeasure(int Quietly)
{
MeasureInProgress = 0;
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_MEASURE, false);
} // ViewGUI::DoCancelMeasure

/*===========================================================================*/

void ViewGUI::StartMeasure(void)
{

DoCancelRegion(0);
DoCancelZoom(0);
SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
SetViewManipulationEnable(0);
SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
MeasureString[0] = MeasureString[1] = MeasureString[2] = MeasureString[3] = MeasureString[4] = MeasureString[5]= -DBL_MAX;
MeasureInProgress = 1;
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_MEASURE, true);

} // ViewGUI::StartMeasure

/*===========================================================================*/

void ViewGUI::DoMeasurePoint(int ViewNum, double PointLat, double PointLon, double PointElev)
{
VertexDEM Begin, End;
double LinearDist, GreatCircleDist, GlobeRad;
char DistanceString[255], LinearString[50], GreatCircleString[50];
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (MeasureInProgress)
	{
	if (ViewNum != -1)
		{
		if (ViewSpaces[ViewNum] && ViewWins[ViewNum])
			{
			if (MeasureString[0] == -DBL_MAX)
				{
				//DrawSync(BGRedrawCurView, ViewSpaces[BGRedrawCurView]);
				MeasureString[0] = PointLat;
				MeasureString[1] = PointLon;
				MeasureString[2] = PointElev;
				return;
				} // if
			else
				{
				MeasureString[3] = PointLat;
				MeasureString[4] = PointLon;
				MeasureString[5] = PointElev;

				GlobeRad = ViewSpaces[ViewNum]->GetRadius();

				GreatCircleDist = FindDistance(MeasureString[0], MeasureString[1], MeasureString[3], MeasureString[4], GlobeRad);
				Begin.Lat  = MeasureString[0];
				Begin.Lon  = MeasureString[1];
				Begin.Elev = MeasureString[2];
				End.Lat    = MeasureString[3];
				End.Lon    = MeasureString[4];
				End.Elev   = MeasureString[5];
				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Begin);
				DefCoords->DegToCart(&End);
				#else // WCS_BUILD_VNS
				Begin.DegToCart(GlobeRad);
				End.DegToCart(GlobeRad);
				#endif // WCS_BUILD_VNS
				LinearDist = PointDistance(Begin.XYZ, End.XYZ);
				FormatAsPreferredUnit(LinearString, WCS_ANIMDOUBLE_METRIC_DISTANCE, LinearDist, 1.0);
				FormatAsPreferredUnit(GreatCircleString, WCS_ANIMDOUBLE_METRIC_DISTANCE, GreatCircleDist, 1.0);
				sprintf(DistanceString, "%s Linear Distance\n%s Great Circle Distance", LinearString, GreatCircleString);
				UserMessageOK("Measure Distance", DistanceString);
				} // else
			} // if
		else
			{
			return;
			} // else
		} // if
	} // if

DoCancelMeasure(0);

} // ViewGUI::DoMeasurePoint

/*===========================================================================*/

void ViewGUI::DoCancelZoom(int Quietly)
{
ZoomInProgress = 0;
if (!Quietly)
	{
	if ((ZoomView != -1) && (ZoomCoord[0] != -1))
		{
		unsigned short VWidth, VHeight;
		ViewWins[ZoomView]->GetDrawingAreaSize(VWidth, VHeight);
		ViewWins[ZoomView]->SyncBackground(0, 0, VWidth, VHeight);
		} // if
	} // if
ZoomView = -1;
ZoomCoord[0] = ZoomCoord[1] = -1;
EnableViewButtons(1);
} // ViewGUI::DoCancelZoom

void ViewGUI::StartZoom(void)
{
DoCancelRegion(0);
DoCancelMeasure(0);
SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
SetViewManipulationEnable(0);
ZoomInProgress = 1;
ZoomView = -1;
ZoomCoord[0] = ZoomCoord[1] = -1;
EnableViewButtons(1);
} // ViewGUI::StartZoom

/*===========================================================================*/

void ViewGUI::DoZoomPoint(int ViewNum, int PointX, int PointY)
{
unsigned short OldWidth, OldHeight;
long NewWidth, NewHeight, NewCenterX, NewCenterY;

if (ZoomInProgress)
	{
	if (ViewNum != -1)
		{
		if (ViewSpaces[ViewNum] && ViewWins[ViewNum])
			{
			if (ZoomCoord[0] == -1)
				{
				DrawSync(BGRedrawCurView, ViewSpaces[BGRedrawCurView]);
				ZoomCoord[0] = PointX;
				ZoomCoord[1] = PointY;
				ZoomView = ViewNum;
				return;
				} // if
			else
				{
				if ((ViewNum == ZoomView) && (ViewSpaces[ViewNum]->VCamera))
					{
					ViewWins[ViewNum]->GetDrawingAreaSize(OldWidth, OldHeight);
					if ((ZoomCoord[0] >= 0) && (ZoomCoord[1] >= 0) && (PointX >= 0) && (PointY >= 0)
					 && (ZoomCoord[0] <= OldWidth) && (ZoomCoord[1] <= OldHeight) && (PointX <= OldWidth) && (PointY <= OldHeight))
						{
						NewWidth  = abs(ZoomCoord[0] - PointX);
						NewHeight = abs(ZoomCoord[1] - PointY);
						NewCenterX = (ZoomCoord[0] + PointX) / 2;
						NewCenterY = (ZoomCoord[1] + PointY) / 2;
						DoCancelZoom(1);
						if (ViewSpaces[ViewNum]->BigR)
							{
							if (ViewSpaces[ViewNum]->BigR->ViewCamCopy)
								ViewSpaces[ViewNum]->BigR->ViewCamCopy->ZoomBoxProxy(ViewSpaces[ViewNum]->VCamera, 
									OldWidth, OldHeight, NewWidth, NewHeight, NewCenterX, NewCenterY);
							} // if a rendering present
						else
							ViewSpaces[ViewNum]->VCamera->ZoomBox(OldWidth, OldHeight,
							 NewWidth, NewHeight, NewCenterX, NewCenterY);
						} // if
					} // if
				} // else
			} // if
		else
			{
			return;
			} // else
		} // if
	} // 

DoCancelZoom(0);

} // ViewGUI::DoZoomPoint

/*===========================================================================*/

void ViewGUI::DoCancelRegion(int Quietly)
{
RegionInProgress = 0;

if (!Quietly)
	{
	if ((RegionView != -1) && (RegionCoord[0] != -1))
		{
		unsigned short VWidth, VHeight;
		ViewWins[RegionView]->GetDrawingAreaSize(VWidth, VHeight);
		ViewWins[RegionView]->SyncBackground(0, 0, VWidth, VHeight);
		} // if
	} // if
RegionView = -1;
RegionCoord[0] = RegionCoord[1] = -1;

} // ViewGUI::DoCancelZoom

/*===========================================================================*/

void ViewGUI::StartRegion(void)
{

DoCancelZoom(0);
DoCancelMeasure(0);
RegionInProgress = 1;
RegionView = -1;
RegionCoord[0] = RegionCoord[1] = -1;

} // ViewGUI::StartRegion

/*===========================================================================*/

void ViewGUI::DoRegionPoint(int ViewNum, int PointX, int PointY)
{
double WPct, HPct, CXPct, CYPct;
long NewWidth, NewHeight, NewCenterX, NewCenterY;
unsigned short OldWidth, OldHeight;

if (RegionInProgress)
	{
	if (ViewNum != -1)
		{
		if (ViewSpaces[ViewNum] && ViewWins[ViewNum] && (ViewSpaces[ViewNum]->RO))
			{
			if (RegionCoord[0] == -1)
				{
				DrawSync(BGRedrawCurView, ViewSpaces[BGRedrawCurView]);
				RegionCoord[0] = PointX;
				RegionCoord[1] = PointY;
				RegionView = (char)ViewNum;
				return;
				} // if
			else
				{
				if ((ViewNum == RegionView) && (ViewSpaces[ViewNum]->VCamera))
					{
					ViewWins[ViewNum]->GetDrawingAreaSize(OldWidth, OldHeight);
					if ((RegionCoord[0] >= 0) && (RegionCoord[1] >= 0) && (PointX >= 0) && (PointY >= 0)
					 && (RegionCoord[0] <= OldWidth) && (RegionCoord[1] <= OldHeight) && (PointX <= OldWidth) && (PointY <= OldHeight))
						{
						NewWidth  = abs(RegionCoord[0] - PointX);
						NewHeight = abs(RegionCoord[1] - PointY);
						NewCenterX = (RegionCoord[0] + PointX) / 2;
						NewCenterY = (RegionCoord[1] + PointY) / 2;
						WPct = (double)NewWidth / (double)OldWidth;
						HPct = (double)NewHeight / (double)OldHeight;
						CXPct = (double)NewCenterX / (double)OldWidth;
						CYPct = (double)NewCenterY / (double)OldHeight;
						ViewSpaces[ViewNum]->RenderRegion[0] = CXPct;
						ViewSpaces[ViewNum]->RenderRegion[1] = CYPct;
						ViewSpaces[ViewNum]->RenderRegion[2] = WPct;
						ViewSpaces[ViewNum]->RenderRegion[3] = HPct;
						ViewSpaces[ViewNum]->SetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION, 1);
						EnableViewButtons(ViewNum);
						DoCancelRegion(1);
						Draw(ViewNum);
						/*ViewSpaces[ViewNum]->VCamera->ZoomBox(OldWidth, OldHeight,
						 NewWidth, NewHeight, NewCenterX, NewCenterY); */
						} // if
					} // if
				} // else
			} // if
		else
			{
			return;
			} // else
		} // if
	} // 

DoCancelRegion(0);

} // ViewGUI::DoRegionPoint

/*===========================================================================*/

void ViewGUI::DoCreate(short Mode, long ForceCategory)
{
#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO

if (!GlobalApp->Sentinal->CheckDongle()) return;

#endif // !WCS_BUILD_DEMO

if (GlobalApp->MainProj->VerifyProjectLoaded())
	{
	// Drop out of Manipulation modes
	SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
	SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
	SetViewManipulationEnable(0);
	if (!GlobalApp->GUIWins->DIG)
		{
		if (GlobalApp->GUIWins->DIG = new DigitizeGUI(GlobalApp->MainProj, GlobalApp->AppEffects, GlobalApp->AppDB, 0/*Page*/))
			{
			if (GlobalApp->GUIWins->DIG->ConstructError)
				{
				delete GlobalApp->GUIWins->DIG;
				GlobalApp->GUIWins->DIG = NULL;
				} // if
			} // if
		} // if
	if (GlobalApp->GUIWins->DIG && GlobalApp->GUIWins->SAG)
		{
		if (GlobalApp->GUIWins->DIG->ConfigureDigitize(Mode, ForceCategory))
			{
			GlobalApp->GUIWins->DIG->Open(GlobalApp->MainProj);
			} // if
		else
			{
			delete GlobalApp->GUIWins->DIG;
			GlobalApp->GUIWins->DIG = NULL;
			} // else
		} // if
	} // if


} // ViewGUI::DoCreate

/*===========================================================================*/

void ViewGUI::EndCreate(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_ENDDIGITIZE, 0, 0);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

// clear LastSample for all ViewContexts
for (int ViewCheck = 0; ViewCheck < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewCheck++)
	{
	if (ViewSpaces[ViewCheck])
		{
		ViewSpaces[ViewCheck]->LastSampleX = ViewSpaces[ViewCheck]->LastSampleY = -1;
		} // if
	} // for


} // ViewGUI::EndCreate

/*===========================================================================*/

void ViewGUI::DoCopy(int View, int Direction)
{
ViewContext *Realtime = NULL;
RenderOpt *Render = NULL;
NotifyTag Changes[2];
// Direction: 1 = ID_WINMENU_SHOW_COPYRENDER

if (View != -1)
	{
	Realtime = ViewSpaces[View];
	Render = ViewSpaces[View]->RO;
	} // if

if (Realtime && Render)
	{
	if (Direction)
		{ // Render to Realtime
		//Realtime->Enabled[] = RO->
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAIN] = Render->TerrainEnabled;
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_SNOW] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAFX] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_AREATERRAFX] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_FOLIAGEFX] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_ECOFX] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_CMAPS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_LAKES] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_STREAMS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_WAVES] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_3DOBJ] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_WALLS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_PLAINVEC] = Render->VectorsEnabled;
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_SKY] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_CLOUDS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_CELEST] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_LIGHTS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAINSHADOWS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW];
		Realtime->Enabled[WCS_VIEWGUI_ENABLE_ATMOS] = Render->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE];

		Draw(View);
		} // if
	else
		{ // Realtime to Render
		//Render-> = Realtime->Enabled[]
		Render->TerrainEnabled = Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAIN];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_SNOW];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAFX];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_AREATERRAFX];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_FOLIAGEFX];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_ECOFX];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_CMAPS];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_LAKES];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_STREAMS];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_WAVES];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_3DOBJ];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_WALLS];
		Render->VectorsEnabled = Realtime->Enabled[WCS_VIEWGUI_ENABLE_PLAINVEC];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_SKY];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_CLOUDS];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_CELEST];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_LIGHTS];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_TERRAINSHADOWS];
		Render->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = Realtime->Enabled[WCS_VIEWGUI_ENABLE_ATMOS];

		Changes[1] = 0;
		Changes[0] = MAKE_ID(Render->GetNotifyClass(), Render->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Render->GetRAHostRoot());
		} // else
	} // if

} // ViewGUI::DoCopy

/*===========================================================================*/

void ViewGUI::LatLonFromXY(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double &Lat, double &Lon)
{
double DummyElev;

LatLonElevFromXY(OriVC, GLDF, X, Y, Lat, Lon, DummyElev);

} // ViewGUI::LatLonFromXY

/*===========================================================================*/

void ViewGUI::LatLonElevFromXY(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double &Lat, double &Lon, double &Elev)
{
Point3d World, Screen;
VertexDEM Original;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

Screen[0] = X;
Screen[1] = OriVC->GLViewport[3] - Y;
// Get corresponding Z value from Z
Screen[2] = GLDF->GLGetPixelZ(X, OriVC->GLViewport[3] - Y);
// unproject to GL cartesian
OriVC->GLUnProject(World, Screen);
if (OriVC->IsPlan())
	{
	Lat  = World[1];
	Lon  = World[0];
	Elev = World[2];
	} // if
else
	{
	// unflip z
	InvertZ(World);
	// add local GL origin back in
	AddPoint3d(World, OriVC->LocalOrigin.XYZ);
	CopyPoint3d(Original.XYZ, World);
	#ifdef WCS_BUILD_VNS
	DefCoords->CartToDeg(&Original);
	#else // WCS_BUILD_VNS
	Original.CartToDeg(OriVC->GlobeRadius);
	#endif // WCS_BUILD_VNS
	Lat  = Original.Lat;
	Lon  = Original.Lon;
	Elev = Original.Elev;
	} // if

} // ViewGUI::LatLonElevFromXY

/*===========================================================================*/

void ViewGUI::XYZfromXYZ(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double Z, Point3d DestXYZ)
{
Point3d World, Screen;

Screen[0] = X;
Screen[1] = OriVC->GLViewport[3] - Y;
// Get corresponding Z value from Z
Screen[2] = Z;
// unproject to GL cartesian
OriVC->GLUnProject(World, Screen);
// unflip z
InvertZ(World);
// add local GL origin back in
AddPoint3d(World, OriVC->LocalOrigin.XYZ);
CopyPoint3d(DestXYZ, World);

} // ViewGUI::XYZfromXYZ

/*===========================================================================*/

void ViewGUI::DoPointHit(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, int ClearPoints)
{

DoSelectObject(OriVC, GLDF, X, Y, 1, ClearPoints);

} // ViewGUI::DoPointHit

/*===========================================================================*/

void ViewGUI::DoSampleDiag(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, char Alt, char Control, char Shift, char LeftMouseTransition)
{
Point3d World, Screen;
VertexDEM Original;
DiagnosticData Data;
NotifyTag Changes[2];
long ViewNum = -1;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
short InputMode;

if (OriVC)
	{
	if ((X == OriVC->LastSampleX) && (Y == OriVC->LastSampleY))
		{ // stutter, bail
		return;
		} // if
	if (GlobalApp->GUIWins->DIG) // suppress stutter during digitize, but permit streaming diag events otherwise
		{
		InputMode = GlobalApp->MainProj->Interactive->GetDigDrawMode();
		if ((InputMode == WCS_DIGITIZE_ADDPOINTS_SINGLE) && DigitizeOneShot)
			{ // stutter, bail
			return;
			} // if
		} // if
	DigitizeOneShot = 1; // no longer the first point created on this mousedown
	OriVC->LastSampleX = X;
	OriVC->LastSampleY = Y;
	if (OriVC->BigR && OriVC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
		{
		OriVC->BigR->SampleDiagnostics(X, Y, 0, 0);
		} // if
	else
		{
		ViewNum = OriVC->VCNum;
		if (OriVC->GLViewport[3] == 0) return; // not yet initialized
		Screen[0] = X;
		Screen[1] = OriVC->GLViewport[3] - Y;
		// Get corresponding Z value from Z
		Screen[2] = GLDF->GLGetPixelZ(X, OriVC->GLViewport[3] - Y);
		// unproject to GL cartesian
		OriVC->GLUnProject(World, Screen);

		Data.RefLat = 0.0;
		Data.RefLon = 0.0;
		Data.PixelX = X;
		Data.PixelY = Y;
		Data.MoveX = 0;
		Data.MoveY = 0;
		Data.ViewSource = ViewNum;


		if (OriVC->IsPlan())
			{
			#ifdef WCS_BUILD_VNS
			if (OriVC->GetProjected())
				{
				// add back in local offset (DBCenter.XYZ)
				Original.xyz[0] = World[0] + DBCenter.XYZ[0];
				Original.xyz[1] = World[1] + DBCenter.XYZ[1];
				Original.xyz[2] = 0.0;

				// convert to lat/lon
				OriVC->GetProjected()->ProjToDefDeg(&Original);

				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				Original.Lat = World[1];
				Original.Lon = World[0];
				} // else
			Original.Elev = InterStash->ElevationPoint(Original.Lat, Original.Lon);
			Original.Elev = OriVC->VCCalcExag(Original.Elev);
			Data.Value[WCS_DIAGNOSTIC_ELEVATION] = Original.Elev;
			Data.Value[WCS_DIAGNOSTIC_LATITUDE] = Original.Lat;
			Data.Value[WCS_DIAGNOSTIC_LONGITUDE] = Original.Lon;
			Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
			Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
			Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
			} // if
		else
			{
			if (Screen[2] == 1.0)
				{
				double LatOut, LonOut;
				//XYZfromXYZ(OriVC, GLDF, X, Y, Screen[2], DestXYZ);
				if (CollideCoord(ViewNum, LatOut, LonOut, X, Y, GlobalFloor))
					{
					Data.Value[WCS_DIAGNOSTIC_ELEVATION] = GlobalFloor;
					Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;

					Data.Value[WCS_DIAGNOSTIC_LATITUDE] = LatOut;
					Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;

					Data.Value[WCS_DIAGNOSTIC_LONGITUDE] = LonOut;
					Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
					} // if
				else
					{
					// Can't help you
					return;
					} // else
				} // if
			else
				{
				// unflip z
				InvertZ(World);
				// add local GL origin back in
				AddPoint3d(World, OriVC->LocalOrigin.XYZ);
				CopyPoint3d(Original.XYZ, World);
				#ifdef WCS_BUILD_VNS
				DefCoords->CartToDeg(&Original);
				#else // WCS_BUILD_VNS
				Original.CartToDeg(OriVC->GlobeRadius);
				#endif // WCS_BUILD_VNS

				//Data.Value[WCS_DIAGNOSTIC_Z] = GLGetScaledPixelZ(OriVC, GLDF, X, Y);
				// Skin this cat another way
				Data.Value[WCS_DIAGNOSTIC_Z] = SolveDistCart(Original.XYZ[0], Original.XYZ[1], Original.XYZ[2],
				 OriVC->VCamera->CamPos->XYZ[0], OriVC->VCamera->CamPos->XYZ[1], OriVC->VCamera->CamPos->XYZ[2]);
				Data.ValueValid[WCS_DIAGNOSTIC_Z] = 1;

				Data.Value[WCS_DIAGNOSTIC_ELEVATION] = Original.Elev;
				Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;

				Data.Value[WCS_DIAGNOSTIC_LATITUDE] = Original.Lat;
				Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;

				Data.Value[WCS_DIAGNOSTIC_LONGITUDE] = Original.Lon;
				Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;

				} // else
			} // else


		GLDF->GLGetPixelRGBA(X, Y, Data.DataRGB[0], Data.DataRGB[1], Data.DataRGB[2], Data.Alpha);
		Data.ValueValid[WCS_DIAGNOSTIC_RGB] = 1;
		Data.ValueValid[WCS_DIAGNOSTIC_ALPHA] = 1;



		Data.ValueValid[WCS_DIAGNOSTIC_SLOPE] = 0;
		Data.ValueValid[WCS_DIAGNOSTIC_ASPECT] = 0;
		Data.ValueValid[WCS_DIAGNOSTIC_ILLUMINATION] = 0;
		Data.ValueValid[WCS_DIAGNOSTIC_REFLECTION] = 0;

		//Data.Value[WCS_DIAGNOSTIC_RELEL] = RelElBuf[PixZip];
		Data.ValueValid[WCS_DIAGNOSTIC_RELEL] = 0;
		//Data.Value[WCS_DIAGNOSTIC_NORMALX] = NormalBuf[0][PixZip];
		Data.ValueValid[WCS_DIAGNOSTIC_NORMALX] = 0;
		//Data.Value[WCS_DIAGNOSTIC_NORMALY] = NormalBuf[1][PixZip];
		Data.ValueValid[WCS_DIAGNOSTIC_NORMALY] = 0;
		//Data.Value[WCS_DIAGNOSTIC_NORMALZ] = NormalBuf[2][PixZip];
		Data.ValueValid[WCS_DIAGNOSTIC_NORMALZ] = 0;
		//Data.Object = ObjectBuf[PixZip];
		Data.ValueValid[WCS_DIAGNOSTIC_OBJECT] = 0;

		Data.DisplayedBuffer = WCS_DIAGNOSTIC_RGB;
		Data.ThresholdValid = 0;
		// Use WCS_DIAGNOSTIC_ITEM_MOUSEDOWN = 1, WCS_DIAGNOSTIC_ITEM_MOUSEDRAG = 2
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, (LeftMouseTransition ? WCS_DIAGNOSTIC_ITEM_MOUSEDOWN : WCS_DIAGNOSTIC_ITEM_MOUSEDRAG), 0);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, &Data);

		SetCursor(Original.Lat, Original.Lon, Original.Elev);
		Draw();
		} // else
	} // if

} // ViewGUI::DoSampleDiag

/*===========================================================================*/

long ViewGUI::ViewNumFromVC(ViewContext *VC)
{

if (VC) return(VC->VCNum);
return(-1);

} // ViewGUI::ViewNumFromVC

/*===========================================================================*/

long ViewGUI::ViewNumFromFenetre(Fenetre *Candidate)
{
ViewContext *OriVC = NULL;

if (Candidate)
	{
	if (Candidate->TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW))
		{
		OriVC = (ViewContext *)Candidate->CustomDataVoid[0];
		return(ViewNumFromVC(OriVC));
		} // if
	} // if

return(-1);

} // ViewGUI::ViewNumFromFenetre

/*===========================================================================*/

int ViewGUI::CollideCoord(int ViewNum, double &LatOut, double &LonOut, long XIn, long YIn, double ElevIn)
{
double EarthCenter[3], CollideVec[3], Radius, Distance;
Point3d FetchRay;
VertexDEM NearestPosIntersection;
Camera *CurVCam;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

// replace these with real values
if (ViewSpaces[ViewNum] && (CurVCam = ViewSpaces[ViewNum]->VCamera))
	{
	// temporary initialization, remove when the real coords are in
	// put normalized ray direction in CollideVec[]. Vector should point from camera out through the pixel.
	XYZfromXYZ(ViewSpaces[ViewNum], ViewWins[ViewNum], XIn, YIn, .5, FetchRay);
	FetchRay[0] -= CurVCam->CamPos->XYZ[0];
	FetchRay[1] -= CurVCam->CamPos->XYZ[1];
	FetchRay[2] -= CurVCam->CamPos->XYZ[2];
	UnitVector(FetchRay);
	CollideVec[0] = FetchRay[0];
	CollideVec[1] = FetchRay[1];
	CollideVec[2] = FetchRay[2];

	// Radius is planet radius + elevation of interest since sphere is geocentric
	Radius = ElevIn + ViewSpaces[ViewNum]->GetRadius();
	EarthCenter[0] = EarthCenter[1] = EarthCenter[2] = 0.0;

	// if intersection takes place function returns 1, puts cartesian coords of nearest positive ray intersection point
	// in NearestPosIntersection.XYZ, and distance to intersection in Distance
	if (RaySphereIntersect(Radius, CurVCam->CamPos->XYZ, CollideVec, EarthCenter, NearestPosIntersection.XYZ, Distance))
		{
		// transform cartesian to geographic, transfer values and return
		#ifdef WCS_BUILD_VNS
		DefCoords->CartToDeg(&NearestPosIntersection);
		#else // WCS_BUILD_VNS
		NearestPosIntersection.CartToDeg(ViewSpaces[ViewNum]->GetRadius());
		#endif // WCS_BUILD_VNS
		LatOut = NearestPosIntersection.Lat;
		LonOut = NearestPosIntersection.Lon;
		return (1);
		} // if
	} // if

// no collision
return(0);

} // ViewGUI::CollideCoord

/*===========================================================================*/

//char DistDump[50];
int ViewGUI::ScaleMotion(DiagnosticData *Diag)
{
double DistToObj = 0.0, DegPerPix = 0, MoveDist = 0, LocalLatScale, LocalLonScale, MoveDegLat, MoveDegLon;
VertexDEM NewPos;
int WorldCoords = 0;
long XDif, YDif, ZXDif, ZYDif;
int Changed = 0;
unsigned short Width, Height;
ViewContext *OriVC;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

WorldCoords = (InterCode == WCS_DOINTER_CODE_SHIFT) ? 1 : 0;
WorldCoords = (((GlobalApp->MainProj->Prefs.InteractiveStyle == WCS_INTERACTIVE_STYLE_LIGHTWAVE) && ! WorldCoords) ||
	((GlobalApp->MainProj->Prefs.InteractiveStyle == WCS_INTERACTIVE_STYLE_MAX) && WorldCoords));

if (Diag && (Diag->ViewSource != -1) && ViewSpaces[Diag->ViewSource] && ViewSpaces[Diag->ViewSource]->VCamera)
	{
	// find the distance to the object we're manipulating
	NewPos.Lat  = Diag->Value[WCS_DIAGNOSTIC_LATITUDE];
	NewPos.Lon  = Diag->Value[WCS_DIAGNOSTIC_LONGITUDE];
	NewPos.Elev = Diag->Value[WCS_DIAGNOSTIC_ELEVATION];

	LocalLatScale = LatScale(ViewSpaces[Diag->ViewSource]->GetRadius());
	LocalLonScale = LocalLatScale * cos(NewPos.Lat * PiOver180);

	if (ViewSpaces[Diag->ViewSource]->IsPlan())
		{
		XDif  = Diag->MoveX;
		YDif  = Diag->MoveY;
		ZXDif = Diag->MoveZ;
		ZYDif = Diag->MoveZ;

		ViewWins[Diag->ViewSource]->GetDrawingAreaSize(Width, Height);
		MoveDist = ViewSpaces[Diag->ViewSource]->VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].GetCurValue(0) / Width;
		if (fabs(NewPos.Lat) < 90.0)
			MoveDegLon = MoveDist / LocalLonScale;
		else
			MoveDegLon = 0.1;
		MoveDegLat = MoveDist / LocalLatScale;

		if (XDif)
			{
			Changed = 1;
			if (GetAxisEnable(WCS_X))
				NewPos.Lon -= XDif * MoveDegLon;
			} // if
		if (YDif)
			{
			Changed = 1;
			if (GetAxisEnable(WCS_Y))
				NewPos.Lat -= YDif * MoveDegLat;
			} // if
		if (ZYDif)
			{
			Changed = 1;
			if (GetAxisEnable(WCS_Z))
				NewPos.Elev -= ZYDif * MoveDist;
			} // if
		if (Changed)
			{
			Diag->Value[WCS_DIAGNOSTIC_LATITUDE] = NewPos.Lat;
			Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] = NewPos.Lon;
			Diag->Value[WCS_DIAGNOSTIC_ELEVATION] = NewPos.Elev;
			Diag->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
			Diag->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
			Diag->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = GetAxisEnable(3);
			} // if
		} // if
	else
		{
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&NewPos);
		#else // WCS_BUILD_VNS
		NewPos.DegToCart(ViewSpaces[Diag->ViewSource]->GetRadius());
		#endif // WCS_BUILD_VNS
		DistToObj = SolveDistCart(ViewSpaces[Diag->ViewSource]->VCamera->CamPos->XYZ[0],
		 ViewSpaces[Diag->ViewSource]->VCamera->CamPos->XYZ[1],
		 ViewSpaces[Diag->ViewSource]->VCamera->CamPos->XYZ[2],
		 NewPos.XYZ[0],
		 NewPos.XYZ[1],
		 NewPos.XYZ[2]);
		OriVC = ViewSpaces[Diag->ViewSource];
		if (OriVC->GLViewport[3] > 0)
			{
			// calculate how much vertical view arc (in degrees) each screen pixel represents
			DegPerPix = OriVC->VFOV / (double)(OriVC->GLViewport[3]);
				{
				if ((DegPerPix > 0.0) && (DegPerPix < 90.0))
					{
					// use the distance-to-object to estimate what one pixel's worth of movement
					// covers at that distance.
					MoveDist = asin(DegPerPix * PiOver180) * DistToObj;
					MoveDist = min(MoveDist, 10000);
					if (MoveDist > 0.0)
						{
						XDif  = Diag->MoveX;
						YDif  = Diag->MoveY;
						ZXDif = Diag->MoveZ;
						ZYDif = Diag->MoveZ;

						if (WorldCoords)
							{
							if (XDif)
								{
								Changed = 1;
								if (GetAxisEnable(WCS_X))
									NewPos.Lon += ((double)XDif * MoveDist) * OriVC->VCamera->IARightVec2D[0] / LocalLonScale;
								if (GetAxisEnable(WCS_Y))
									NewPos.Lat += ((double)XDif * MoveDist) * OriVC->VCamera->IARightVec2D[1] / LocalLatScale;
								} // if
							if (YDif)
								{
								Changed = 1;
								if (GetAxisEnable(WCS_X))
									NewPos.Lon -= ((double)YDif * MoveDist) * OriVC->VCamera->IAUpVec2D[0] / LocalLonScale;
								if (GetAxisEnable(WCS_Y))
									NewPos.Lat -= ((double)YDif * MoveDist) * OriVC->VCamera->IAUpVec2D[1] / LocalLatScale;
								} // if
							if (ZYDif)
								{
								Changed = 1;
								if (GetAxisEnable(WCS_Z))
									NewPos.Elev -= ((double)ZYDif * MoveDist); // add elevation
								} // if
							if (Changed)
								{
								Diag->Value[WCS_DIAGNOSTIC_LATITUDE] = NewPos.Lat;
								Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] = NewPos.Lon;
								Diag->Value[WCS_DIAGNOSTIC_ELEVATION] = NewPos.Elev;
								Diag->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
								Diag->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
								Diag->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = GetAxisEnable(3);
								} // if
							} // if
						else
							{
							if (XDif && GetAxisEnable(WCS_X))
								{
								Changed = 1;
								NewPos.XYZ[0] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[0]; // add scaled CamRight vector
								NewPos.XYZ[1] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[1]; // add scaled CamRight vector
								NewPos.XYZ[2] += ((double)XDif * MoveDist) * OriVC->VCamera->CamRight->XYZ[2]; // add scaled CamRight vector
								} // if
							if (YDif && GetAxisEnable(WCS_Y))
								{
								Changed = 1;
								NewPos.XYZ[0] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[0]; // add scaled CamVertical vector
								NewPos.XYZ[1] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[1]; // add scaled CamVertical vector
								NewPos.XYZ[2] -= ((double)YDif * MoveDist) * OriVC->VCamera->CamVertical->XYZ[2]; // add scaled CamVertical vector
								} // if
							if (ZYDif && GetAxisEnable(WCS_Z))
								{
								Changed = 1;
								NewPos.XYZ[0] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[0]; // add scaled TargPos vector
								NewPos.XYZ[1] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[1]; // add scaled TargPos vector
								NewPos.XYZ[2] -= ((double)ZYDif * MoveDist) * OriVC->VCamera->TargPos->XYZ[2]; // add scaled TargPos vector
								} // if
							if (Changed)
								{
								#ifdef WCS_BUILD_VNS
								DefCoords->CartToDeg(&NewPos);
								#else // WCS_BUILD_VNS
								NewPos.CartToDeg(OriVC->GetRadius());
								#endif // WCS_BUILD_VNS
								Diag->Value[WCS_DIAGNOSTIC_LATITUDE] = NewPos.Lat;
								Diag->Value[WCS_DIAGNOSTIC_LONGITUDE] = NewPos.Lon;
								Diag->Value[WCS_DIAGNOSTIC_ELEVATION] = NewPos.Elev;
								Diag->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
								Diag->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
								Diag->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = GetAxisEnable(3);
								} // if
							} // else
						} // if
					} // if
				} // if
			} // if
		} // else not plan view
	} // if

return(0);

} // ViewGUI::ScaleMotion

/*===========================================================================*/

void ViewGUI::BeginBGRedraw(void)
{

BGRedraw = 1;
BGRedrawCurView = 0;

} // ViewGUI::BeginBGRedraw

/*===========================================================================*/

void ViewGUI::EndBGRedraw(void)
{

if (BGRedraw && (BGRedrawCurView != -1))
	{
	FinishRedraw();
	} // if

BGRedraw = 0;
BGRedrawCurView = -1; // inactive
// <<<>>> Strip NextBGRedrawAction values from database
//NextBGRedrawAction = 0;

} // ViewGUI::EndBGRedraw

/*===========================================================================*/

static int MotionControlIdle;

long ViewGUI::HandleBackgroundCrunch(int Siblings)
{
int SyncLoop;
unsigned long DiffTicks, NowTicks;

SECURITY_INLINE_CHECK(017, 17);

if (GetMotionControl())
	{
	// Only process motion control on alternating ticks to give system a moment of rest.
	MotionControlIdle = !MotionControlIdle;
	if (!MotionControlIdle)
		{
		ProcessMotionControl();
		} // if
	return(0);
	} // if

if (GetQueuedRedraw())
	{
	DrawImmediately(QueuedViewNum);
	QueuedViewNum = -1;
	//ClearQueuedRedraw();
	} // if

if (DelayedSync && SyncDelayTime != 0)
	{
	NowTicks = GetTickCount();
	if ((DiffTicks = NowTicks - SyncDelayTime) > WCS_VIEWGUI_SYNC_DELAY_MS)
		{
		for (SyncLoop = 0; SyncLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; SyncLoop++)
			{
			if (ViewSpaces[SyncLoop])
				{
				// perform a surreptitious redraw here.
				if (GLReadPixelsFront) // Fenetre.cpp will tell us.
					{
					// a little out-of-band signalling to tell cleanup not to swap the
					// buffers so we can read them undamaged from the BG buffer in Sync
					CleanupNoSwap = 1;
					DrawView(SyncLoop, ViewSpaces[SyncLoop]);
					CleanupNoSwap = 0;
					} // if
				DrawSync(SyncLoop, ViewSpaces[SyncLoop]);
				//BeginBGRedraw();
				} // if
			} // for
		SyncDelayTicks = DelayedSync = 0;
		SyncDelayTime = 0;
		} // if
	return(0);
	} // if

if (BGRedraw)
	{
	return(DrawBG());
	} // if

return(1);

} // ViewGUI::HandleBackgroundCrunch

/*===========================================================================*/

long ViewGUI::HandleCloseWin(NativeGUIWin NW)
{
int ViewOrigin;

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	//DoClose(ViewOrigin); // DoClose is not safe if called from within a Fenetre event handler!
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_VIW, ViewOrigin);
	} // if

return(1);

} // ViewGUI::HandleCloseWin

/*===========================================================================*/

void ViewGUI::DoClose(int ViewOrigin)
{
const char *VPID;

if (ViewWins[ViewOrigin])
	{
	// unmark dock cell
	VPID = ViewWins[ViewOrigin]->GetVPID();
	if (VPID[0] != 0)
		{
		GlobalApp->MainProj->ViewPorts.ClearFlag(VPID, WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
		} // if
	// Chris, calling this next line causes Jamie's machine and some users' to crash running VNS or 5.52
	// I can't reproduce it here but found the offending line by putting debug messages in and letting Jamie test it.
	// I'm not sure if ViewWins[ViewOrigin] is valid or not when it crashes but it seems to be a valid
	// GLDrawingFenetre for me and the destructor runs without a hitch. I don't know enough about the workings of the 
	// destructor to test it any further. The crash happens whenever a view is closed either by the user or by program 
	// control such as when loading a new project or quiting the program.
	delete ViewWins[ViewOrigin];
	ViewWins[ViewOrigin] = NULL;
	delete ViewSpaces[ViewOrigin];
	ViewSpaces[ViewOrigin] = NULL;
	if (PrefsGUI)
		{
		if (PrefsGUI->GetActiveView() == ViewOrigin)
			{
	/*
			Prev = PrefsGUI->GetPrevView();
			if ((Prev >= 0) && (ViewWins[Prev]))
				{
				DoPrefs(Prev);
				} // if
			else
	*/
				{
				delete PrefsGUI;
				PrefsGUI = NULL;
				} // else
			} // if
		} // if

	if (GLDrawingFenetre::LastOneOut() && VectorListsAreReserved)
		{
		// Kill global displaylists
		KillVectorDisplayLists();

		// Kill object displaylists
		KillDEMDisplayLists();

		if (WallPlanList != -1)
			{
			glDeleteLists(WallPlanList, 1);
			WallPlanList = -1;
			} // if

		if (WallPerspList != -1)
			{
			glDeleteLists(WallPerspList, 1);
			WallPerspList = -1;
			} // if

		} // if
	} // if
} // ViewGUI::DoClose

/*===========================================================================*/

long ViewGUI::HandleGainFocus(void)
{
int ViewOrigin;

if (PrefsGUI && !PrefsGUI->IsThisYou(NULL)) // is Pref window open?
	{
	if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
		{
		if (ViewOrigin != PrefsGUI->GetActiveView())
			{
			DoPrefs(ViewOrigin);
			}
		} // if
	} // if

if ((ViewOrigin = IdentViewOrigin(Activity)) >= 0)
	{
	LastActiveView = ViewOrigin;
	} // if

EnableViewButtons(1);
ActiveItemSwitch();

return(0);

} // ViewGUI::HandleGainFocus

/*===========================================================================*/

long ViewGUI::HandleLoseFocus(void)
{
EnableViewButtons(0);

// turn off View manipulation caption buttons
for (int InitLoop = 0; InitLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; InitLoop++)
	{
	if (ViewWins[InitLoop])
		{
		ViewSpaces[InitLoop]->ViewState &= ~(WCS_VIEWGUI_VIEWSTATE_VIEWMANIP);
		ViewWins[InitLoop]->UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_PAN | WCS_FENETRE_WINSTATE_ROTATE | WCS_FENETRE_WINSTATE_ZOOM);
		} // if
	} // for

return(0);

} // ViewGUI::HandleLoseFocus

/*===========================================================================*/

static char ViewTitle[540];
void ViewGUI::DoPrefs(int ViewNum)
{
int IsOpen = 0;
#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO
if (!GlobalApp->Sentinal->CheckDongle()) return;
#endif // !WCS_BUILD_DEMO

if (ViewNum != -1)
	{
	if (ViewSpaces[ViewNum])
		{
		if (!((ViewSpaces[ViewNum]->VCamera) && (ViewSpaces[ViewNum]->RO)))
			{
			ViewNum = -1;
			} // if
		} // if
	else
		{
		ViewNum = -1;
		} // else
	} // if

if (!PrefsGUI)
	{
	PrefsGUI = new ViewPrefsGUI();
	} // if
else
	{
	if (ViewNum == -1)
		{ // already open, bozo
		return;
		} // if
	if (!PrefsGUI->IsThisYou(NULL))
		{
		IsOpen = 1;
		} // if
	} // else

if (PrefsGUI)
	{
	if (ViewNum != -1)
		{
		sprintf(ViewTitle, "View Preferences %d (%s)", ViewNum, GetViewName(ViewNum));
		PrefsGUI->SetTitle(ViewTitle);
		} // if
	else
		{
		PrefsGUI->SetTitle("View Preferences");
		} // else
	//if (!IsOpen)
		{
		PrefsGUI->Open(GlobalApp->MainProj);
		} // if
	PrefsGUI->DoView(ViewNum);
	} // if

} // ViewGUI::DoPrefs

/*===========================================================================*/

void ViewGUI::UpdatePrefs(int ViewNum)
{

if (PrefsGUI && PrefsGUI->ActiveViewNum == ViewNum)
	{
	PrefsGUI->ConfigureWidgets();
	} // if

} // ViewGUI::UpdatePrefs

/*===========================================================================*/

void ViewGUI::ChangePrefs(int ViewNum, int ID)
{
ViewContext *ViewOrigin = NULL;

if ((ViewNum != -1) && (ViewOrigin = ViewSpaces[ViewNum]))
	{
	switch(ID)
		{
		case ID_WINMENU_SHOW_CONTROLPOINTS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CONTROLPOINTS); break;
		case IS_WINMENU_SHOW_MEASURES: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_MEASURES); break;
		case ID_WINMENU_SHOW_ACTIVEOBJECT: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_ACTIVEOBJ); break;
		case ID_WINMENU_SHOW_DEMEDGES: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_DEMEDGE); break;
		case ID_WINMENU_SHOW_TERRAIN: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_TERRAIN); break;
		case ID_WINMENU_SHOW_TERRAIN_TRANS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS); break;
		case ID_WINMENU_SHOW_TERRAIN_POLY: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_POLYEDGE); break;
		case ID_WINMENU_SHOW_TFX: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_TERRAFX); break;
		case ID_WINMENU_SHOW_ATFX: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_AREATERRAFX); break;

		case ID_WINMENU_SHOW_SNOW: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SNOW); break;
		case ID_WINMENU_SHOW_FOLIAGEFX: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_FOLIAGEFX); break;
		case ID_WINMENU_SHOW_RTFOLIAGE: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_RTFOLIAGE); break;
		case ID_WINMENU_SHOW_RTFOLFILE: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_RTFOLFILE); break;
		case ID_WINMENU_SHOW_ECOFX: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_ECOFX); break;
		case ID_WINMENU_SHOW_COLORMAPS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CMAPS); break;

		case ID_WINMENU_SHOW_WATER: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_LAKES); break;
		case ID_WINMENU_SHOW_STREAMS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_STREAMS); break;
		case ID_WINMENU_SHOW_WAVES: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_WAVES); break;

		case ID_WINMENU_SHOW_HAZE: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_ATMOS); break;
		case ID_WINMENU_SHOW_CLOUDS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CLOUDS); break;
		case ID_WINMENU_SHOW_GROUND: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_GROUND); break;
		case ID_WINMENU_SHOW_SKY: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SKY); break;
		case ID_WINMENU_SHOW_CELESTIALOBJECTS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CELEST); break;

		case ID_WINMENU_SHOW_SHADOWFX: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_TERRAINSHADOWS); break;
		case ID_WINMENU_SHOW_LIGHTS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_LIGHTS); break;

		case ID_WINMENU_SHOW_3DOBJS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_3DOBJ); break;
#ifdef WCS_VIEW_DRAWWALLS
		case ID_WINMENU_SHOW_WALLS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_WALLS); break;
#endif // WCS_VIEW_DRAWWALLS
		case ID_WINMENU_SHOW_VECTORS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_PLAINVEC); break;

		case ID_WINMENU_SHOW_CAMERAS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CAMERAS); break;
		case ID_WINMENU_SHOW_TARGETS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_TARGETS); break;
		case ID_WINMENU_SHOW_IMGBOUNDS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SAFEAREA); break;
		case ID_WINMENU_OPTIONS_CONSTRAIN: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_LTDREGION); break;
		case ID_WINMENU_OPTIONS_SAFETITLE: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SAFETITLE); break;
		case ID_WINMENU_OPTIONS_SAFEACTION: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SAFEACTION); break;

		case ID_WINMENU_SHOW_CONTOURS: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_CONTOURS, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_CONTOURS)); break;
		case ID_WINMENU_SHOW_SLOPE: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_SLOPE, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_SLOPE)); if (ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_SLOPE)) RegenSlopeTex = 1; else TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_SLOPE); break;
		case ID_WINMENU_SHOW_RELEL: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_RELEL, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_RELEL));  if (ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_RELEL)) RegenRelelTex = 1;  else TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_RELEL); break;
		case ID_WINMENU_SHOW_LOD: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_FRACTAL, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_FRACTAL));  if (ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_FRACTAL)) RegenFractTex = 1;  else TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_FRACTAL); break;
		case ID_WINMENU_SHOW_ECOSYSTEMS: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_ECOSYS, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_ECOSYS));  if (ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_ECOSYS)) RegenEcoTex = 1;  else TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_ECOSYS); break;
		//case ID_WINMENU_SHOW_RENDEREDTEXTURE: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_RENDER, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_RENDER)); break;
		case ID_WINMENU_SHOW_GREY: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_GREY, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_GRAD_GREY)); break;
		case ID_WINMENU_SHOW_EARTH: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_EARTH, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_GRAD_EARTH)); break;
		case ID_WINMENU_SHOW_COLOR: ViewOrigin->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_PRIMARY, !ViewOrigin->GetEnabled(WCS_VIEWGUI_ENABLE_GRAD_PRIMARY)); break;
		
		case ID_WINMENU_SHOW_MULTIGRAD: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_GRADREPEAT); break;

		case ID_WINMENU_SHOW_CURSOR: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_CURSOR); break;
		case ID_WINMENU_SHOW_EXPORTERS: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_EXPORTERS); break;
		case ID_WINMENU_SHOW_TFXVIEW_AUTO:
			{
			InterStash->SetParam(1, WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_SETTINGS, WCS_INTERCAM_ITEM_SETTINGS_TFXREALTIME, 0, !InterStash->GetTfxRealtime(), NULL);
			break;
			} // Terraffector Preview AutoUpdate
		case ID_WINMENU_SHOW_TFXVIEW:
			{
			InterStash->SetParam(1, WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_SETTINGS, WCS_INTERCAM_ITEM_SETTINGS_TFXPREVIEW, 0, !InterStash->GetTfxPreview(), NULL);
			RegenDEMs = 1;
			break;
			} // ID_WINMENU_SHOW_TFXVIEW
		} // ID
	} // if

UpdatePrefs(ViewNum);
Draw(ViewNum);

} // ViewGUI::ChangePrefs

/*===========================================================================*/

void ViewGUI::DoRTPrefs(void)
{
#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO
if (!GlobalApp->Sentinal->CheckDongle()) return;
#endif // !WCS_BUILD_DEMO

if (!RTPrefsGUI)
	{
	RTPrefsGUI = new ViewRTPrefsGUI();
	} // if

if (RTPrefsGUI)
	{
	RTPrefsGUI->Open(GlobalApp->MainProj);
	} // if

} // ViewGUI::DoRTPrefs

/*===========================================================================*/

void ViewGUI::UpdateRTPrefs(int ViewNum)
{
if (PrefsGUI && PrefsGUI->ActiveViewNum == ViewNum)
	{
	PrefsGUI->ConfigureWidgets();
	} // if
} // ViewGUI::UpdateRTPrefs

/*===========================================================================*/

int ViewGUI::DoDelete(void)
{
RasterAnimHost *ActObj = NULL;
RasterAnimHostProperties RAHP;
int IsVectorOrControlPt;
Joe *CurActive;
NotifyTag DeleteEditChanges[2];

if (ActObj = RasterAnimHost::GetActiveRAHost())
	{
	RAHP.PropMask = WCS_RAHOST_MASKBIT_INTERFLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	ActObj->GetRAHostProperties(&RAHP);
	IsVectorOrControlPt = (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR) || (RAHP.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);

	if (IsVectorOrControlPt)
		{
		CurActive = (Joe *)ActObj;
		if (InterStash->MVPointsMode())
			{ // Delete Points
			if (CurActive->Points()) // Does it _have_ points?
				{
				// Warn everyone
				DeleteEditChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
				DeleteEditChanges[1] = NULL;
				LocalDB->GenerateNotify(DeleteEditChanges, LocalDB->ActiveObj);

				// Do the foul deed.
				InterStash->DeleteSelectedPoints();
				InterStash->UnSelectAllPoints();

				// Announce the demise
				DeleteEditChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
				DeleteEditChanges[1] = NULL;
				LocalDB->GenerateNotify(DeleteEditChanges, LocalDB->ActiveObj);
				DrawForceRegen(1, 0);
				return(1);
				} // if
			} // if
		else
			{
			LocalDB->RemoveRAHost(ActObj, GlobalApp->MainProj, GlobalApp->AppEffects, 0, 0);
			DrawForceRegen(1, 0);
			return(1);
			} // else
		} // if
	} // if

return(0);

} // ViewGUI::DoDelete

/*===========================================================================*/

void ViewGUI::DoRenderOpt(int ViewNum, int ID)
{
ViewContext *ViewOrigin = NULL;
RenderOpt *RO = NULL;
NotifyTag Changes[2];

if ((ViewNum != -1) && (ViewOrigin = ViewSpaces[ViewNum]) && (RO = ViewOrigin->RO))
	{
	switch(ID)
		{
		case ID_WINMENU_RENDER_TERRAIN: RO->TerrainEnabled = !RO->TerrainEnabled; break;
		case ID_WINMENU_RENDER_SNOW: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW]; break;
		case ID_WINMENU_RENDER_TFX: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR]; break;
		case ID_WINMENU_RENDER_ATFX: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA]; break;
		case ID_WINMENU_RENDER_FOLIAGE: RO->FoliageEnabled = !RO->FoliageEnabled; break;
		case ID_WINMENU_RENDER_FOLIAGEFX: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE]; break;
		case ID_WINMENU_RENDER_ECOFX: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM]; break;
		case ID_WINMENU_RENDER_COLORMAPS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP]; break;
		case ID_WINMENU_RENDER_WATER: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE]; break;
		case ID_WINMENU_RENDER_STREAMS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM]; break;
		case ID_WINMENU_RENDER_WAVES: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE]; break;
		case ID_WINMENU_RENDER_REFLECTIONS: RO->ReflectionsEnabled = !RO->ReflectionsEnabled; break;
		case ID_WINMENU_RENDER_SKY: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY]; break;
		case ID_WINMENU_RENDER_HAZE: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE]; break;
		case ID_WINMENU_RENDER_CLOUDS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD]; break;
		case ID_WINMENU_RENDER_VOLUMETRICS: RO->VolumetricsEnabled = !RO->VolumetricsEnabled; break;
		case ID_WINMENU_RENDER_CELESTIALOBJECTS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL]; break;
		case ID_WINMENU_RENDER_STARS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD]; break;
		case ID_WINMENU_RENDER_LIGHTS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT]; break;
		case ID_WINMENU_RENDER_CLOUDSHADOW: RO->CloudShadowsEnabled = !RO->CloudShadowsEnabled; break;
		case ID_WINMENU_RENDER_SHADOWFX: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW]; break;
		case ID_WINMENU_RENDER_3DOBJSHADOW: RO->ObjectShadowsEnabled = !RO->ObjectShadowsEnabled; break;
		case ID_WINMENU_RENDER_3DOBJS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D]; break;
		case ID_WINMENU_RENDER_WALLS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE]; break;
		case ID_WINMENU_RENDER_LABELS: RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = !RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL]; break;
		case ID_WINMENU_RENDER_VECTORS: RO->VectorsEnabled = !RO->VectorsEnabled; break;
		case ID_WINMENU_RENDER_DOF: RO->DepthOfFieldEnabled = !RO->DepthOfFieldEnabled; break;
		case ID_WINMENU_RENDER_MULTIAA: RO->MultiPassAAEnabled = !RO->MultiPassAAEnabled; break;
		case ID_WINMENU_RENDER_DIAG: RO->RenderDiagnosticData = !RO->RenderDiagnosticData; break;
		} // ID
	Changes[0] = MAKE_ID(RO->GetNotifyClass(), RO->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = 0;
	GlobalApp->AppEx->GenerateNotify(Changes, RO->GetRAHostRoot());
	} // if

} // ViewGUI::DoRenderOpt

/*===========================================================================*/

int ViewGUI::DoRender(int ViewNum, int WithRTF)
{
ViewContext *VC;
RenderJob *Mical = NULL;
RenderOpt *Opt = NULL;
int Success = 0;
double WidthRatio, HeightRatio;
char TempFieldRender;
DrawingFenetre *Pane;
unsigned short Width, Height;

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO
if (!GlobalApp->Sentinal->CheckDongle()) return(Success);
#endif // !WCS_BUILD_DEMO

if (ViewNum == -1)
	{
	// attempt to identify the proper view to render
	ViewNum = LastActiveView;
	} // if

if (ViewNum >= 0 && ViewSpaces[ViewNum] && ViewSpaces[ViewNum]->RO && ViewSpaces[ViewNum]->VCamera)
	{
	VC = ViewSpaces[ViewNum];
	} // if
else
	{
	return(Success);
	} // else

if (VC)
	{
	if (VC->BigR)
		{
		if (VC->BigR->Opt) delete VC->BigR->Opt;
		VC->BigR->Opt = NULL;
		delete VC->BigR; VC->BigR = NULL;
		} // if
	VC->BigR = new Renderer;
	Mical = new RenderJob;
	Opt = new RenderOpt;
	Pane = ViewWins[ViewNum];
	Pane->SetFocusToMe(); // make sure any field changes that haven't gotten return/tab are accepted.
	Pane->GoModal(); // to keep us responsive to ESC during rendering
	Pane->GetDrawingAreaSize(Width, Height);
	if (Mical && VC->BigR && Opt)
		{
		Opt->Copy(Opt, VC->RO);
		// alter some settings...
		Opt->RenderImageSegments = 1;
		Opt->TilingEnabled = 0;
		Opt->MultiPassAAEnabled = 0;
		// stash values from camera
		TempFieldRender = VC->VCamera->FieldRender;
		VC->VCamera->FieldRender = 0;
		
		if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_SAFEAREA))
			{
			// limit render to actual render aspect
			WidthRatio = (double)Width / VC->RO->OutputImageWidth;
			HeightRatio = (double)Height / VC->RO->OutputImageHeight;
			if (WidthRatio <= HeightRatio)
				{
				Opt->CamSetupRenderWidth = Width;
				Opt->CamSetupRenderHeight = (long)(min(Height, VC->RO->OutputImageHeight * WidthRatio));
				Opt->RenderOffsetY = Opt->SetupOffsetY = (Height - Opt->CamSetupRenderHeight) / 2;
				} // if
			else
				{
				Opt->CamSetupRenderHeight = Height;
				Opt->CamSetupRenderWidth = (long)(min(Width, VC->RO->OutputImageWidth * HeightRatio));
				Opt->RenderOffsetX = Opt->SetupOffsetX = (Width - Opt->CamSetupRenderWidth) / 2;
				} // else
			} // if displaying image frame
		else
			{
			Opt->CamSetupRenderWidth = Width;
			Opt->CamSetupRenderHeight = Height;
			} // else

		if (VC->GetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION))
			{
			// limit rendering to limited region
			Opt->OutputImageWidth = (long)(Width * VC->RenderRegion[2]);
			Opt->OutputImageHeight = (long)(Height * VC->RenderRegion[3]);
			Opt->RenderOffsetX = (long)(VC->RenderRegion[0] * Width - Opt->OutputImageWidth * 0.5);
			Opt->RenderOffsetY = (long)(VC->RenderRegion[1] * Height - Opt->OutputImageHeight * 0.5);
			} // else if limited region
		else
			{
			Opt->OutputImageWidth = Opt->CamSetupRenderWidth;
			Opt->OutputImageHeight = Opt->CamSetupRenderHeight;
			} // else

		Opt->RenderDiagnosticData = 1;
		// disable output
		while (Opt->OutputEvents) Opt->RemoveOutputEvent(Opt->OutputEvents);

		Mical->SetCamera(VC->VCamera);
		Mical->SetRenderOpt(Opt);

		// RealtimeFoliageList
		VC->BigR->RealTimeFoliageWrite = NULL;
		if (WithRTF)
			{
			// enable Realtime Foliage list file writing in renderer
			VC->BigR->RealTimeFoliageWrite = &RTFWriteConf;

			// disable some things to speed generation
			Opt->VectorsEnabled = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = 0;

			Opt->CloudShadowsEnabled = 0;
			Opt->ObjectShadowsEnabled = 0;
			Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 0;
			Opt->ReflectionsEnabled = 0;
			} // if

		if (VC->BigR->Init(Mical, GlobalApp, GlobalApp->AppEffects, GlobalApp->AppImages, 
		 GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->StatusLog, NULL,
		 Pane, TRUE))
			{
			Pane->SetupForDrawing();
			Pane->Clear();
			//Pane->GoModal();
			VC->BigR->RenderFrame(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetActiveFrame());
			//Pane->EndModal();
			VC->SetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV, 1);
			GlobalApp->WinSys->ClearRenderTitle();
			GlobalApp->WinSys->DoBeep();
			if (!WithRTF) VC->BigR->LogElapsedTime(GlobalApp->MainProj->Interactive->GetActiveFrame(), true, false, true);
			Success = 1;
			Pane->CleanupFromDrawing();
			VC->BigR->Cleanup(true, false, 1, !GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("suppress_diagnosticwin"));
			GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);

			// Open diagnostics
			if (!WithRTF)
				{
				#ifdef WCS_BUILD_DEMO
				DONGLE_INLINE_CHECK()
				if (!GlobalApp->WinSys->InquireMinimized())
				#else // !WCS_BUILD_DEMO
				if (!GlobalApp->WinSys->InquireMinimized() && (GlobalApp->Sentinal->CheckDongle()))
				#endif // !WCS_BUILD_DEMO
					{
					if (!GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("suppress_diagnosticwin"))
						{
						GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
						 WCS_TOOLBAR_ITEM_RDG, 0);
						} // if
					} // if
				} // if
			} // if
		else
			{
			VC->BigR->Cleanup(false, false, FALSE, false);
			GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
			} // else
		// restore values to camera
		VC->VCamera->FieldRender = TempFieldRender;
		} // if
	if (Mical) delete Mical; Mical = NULL;
	// Keep renderer around for diagnostics
	//if (VC->BigR) delete VC->BigR; VC->BigR = NULL;
	// Keep Opt around too, for PostProc events to use. Careful to delete it when we kill BigR
	//if (Opt) delete Opt; Opt = NULL;
	
	Pane->EndModal();
	} // if

EnableViewButtons(IdentActiveView() != -1);

return(Success);

} // ViewGUI::DoRender

/*===========================================================================*/

void ViewGUI::DrawTempBox(int ViewNum, unsigned short XS, unsigned short YS, unsigned short XE, unsigned short YE)
{

ViewWins[ViewNum]->SetupForDrawing();
ViewWins[ViewNum]->DrawTempLine(XS, YS, XE, YS);
ViewWins[ViewNum]->DrawTempLine(XE, YS, XE, YE);
ViewWins[ViewNum]->DrawTempLine(XE, YE, XS, YE);
ViewWins[ViewNum]->DrawTempLine(XS, YE, XS, YS);
ViewWins[ViewNum]->CleanupFromDrawing();

} // ViewGUI::DrawTempBox

/*===========================================================================*/

char *ViewGUI::GetViewName(int View)
{

if (View >= 0 && ViewSpaces[View])
	{
	return(ViewSpaces[View]->VCamera->Name);
	} // if

return("");

} // ViewGUI::GetViewName

/*===========================================================================*/

int ViewGUI::ProjectForViews(ViewContext *VC, VertexDEM *Input)
{
#ifdef WCS_BUILD_VNS
CoordSys *ProjectedSys;

ProjectedSys = VC->GetProjected();

if (ProjectedSys == NULL) return(0); // error

VertexDEM TempVert;

TempVert.Lon  = Input->Lon;
TempVert.Lat  = Input->Lat;
TempVert.Elev = Input->Elev;
ProjectedSys->DefDegToProj(&TempVert);

// Adjust by local origin
Input->XYZ[0] = TempVert.xyz[0] - DBCenter.XYZ[0];
Input->XYZ[1] = TempVert.xyz[1] - DBCenter.XYZ[1];
Input->XYZ[2] = VC->VCCalcExag(Input->Elev);

if (ProjectedSys->GetGeographic())
	{
	// not sure just yet
	/*
	Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
	TempLon = Vert->ScrnXYZ[0];
	if (fabs(TempLon) > 180.0)
		{
		TempLon += 180.0;
		if (fabs(TempLon) > 360.0)
			TempLon = fmod(TempLon, 360.0);	// retains the original sign
		if (TempLon < 0.0)
			TempLon += 360.0;
		TempLon -= 180.0;
		Vert->ScrnXYZ[0] = TempLon;
		} // if
	Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
	Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
	*/
	} // if

return(1);
#else // !WCS_BUILD_VNS
return(0); // error
#endif // !WCS_BUILD_VNS
} // ViewGUI::ProjectForViews

/*===========================================================================*/
/*===========================================================================*/

ViewContext::ViewContext()
{
double RangeDefaults[3];

SetAlive(1);
ShowRender = 0;
NearCeles = FarCeles = VFOV = PixAspect = NearClip = FarClip = 0.0;
GlobeRadius = EARTHRAD;
FrustumLeft = FrustumRight = FrustumTop = FrustumBottom = 0.0;
SetOrigin(0.0, 0.0, 0.0);
SetTime(0.0);
SetCamera(NULL);
SetSky(NULL);
SetRenderOpt(NULL);
ViewState = NULL;

GLViewport[3] = GLViewport[2] = GLViewport[1] = GLViewport[0] = 0;

//ContourOverlayInterval
ContourOverlayInterval.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES|WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY|WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ContourOverlayInterval.SetCurValue(10.0);
ContourOverlayInterval.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 100.0;
ContourOverlayInterval.SetRangeDefaults(RangeDefaults);

BigR = NULL;
VCWinID = NULL;
VPID[0] = NULL;
LastSampleX = LastSampleY = -1;
RenderRegion[0] = RenderRegion[1] = // Position of center of region
RenderRegion[2] = RenderRegion[3] = 0.5; // fraction of whole image
memset(&Enabled[0], 1, WCS_VIEWGUI_ENABLE_MAX);
SetEnabled(WCS_VIEWGUI_ENABLE_POLYEDGE, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_TERRAIN_TRANS, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_DEMEDGE, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_GRADREPEAT, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_GRAD_GREY, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_GRAD_PRIMARY, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_CONTOURS, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_SLOPE, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_RELEL, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_FRACTAL, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_ECOSYS, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_OVER_RENDER, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_SAFETITLE, 0);
SetEnabled(WCS_VIEWGUI_ENABLE_SAFEACTION, 0);
#ifdef WCS_VIEW_DRAWWALLS
SetEnabled(WCS_VIEWGUI_ENABLE_WALLS, 1);
#endif // WCS_VIEW_DRAWWALLS
} // ViewContext::ViewContext()

/*===========================================================================*/

ViewContext::~ViewContext()
{
if (BigR)
	{
	if (BigR->Opt) delete BigR->Opt;
	BigR->Opt = NULL;
	delete BigR; BigR = NULL;
	} // if
} // ViewContext::~ViewContext()

/*===========================================================================*/

int ViewContext::CheckCamValid(void)
{

if (VCamera)
	{
	if (GlobalApp->AppEffects->IsEffectValid(VCamera, WCS_EFFECTSSUBCLASS_CAMERA, 0))
		{
		return(1);
		} // if
	} // if
return(0);

} // ViewContext::CheckCamValid

/*===========================================================================*/

int ViewContext::CheckROValid(void)
{
if (RO)
	{
	if (GlobalApp->AppEffects->IsEffectValid(RO, WCS_EFFECTSSUBCLASS_RENDEROPT, 0))
		{
		return(1);
		} // if
	} // if
return(0);

} // ViewContext::CheckROValid

/*===========================================================================*/

int ViewContext::CheckSkyValid(void)
{

if (VSky)
	{
	if (GlobalApp->AppEffects->IsEffectValid(VSky, WCS_EFFECTSSUBCLASS_SKY, 0))
		{
		return(1);
		} // if
	} // if
return(0);

} // ViewContext::CheckSkyValid

/*===========================================================================*/

void ViewContext::FetchCamCoord(double TheTime)
{
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

CamPos.Lat  = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, TheTime);  // TheTime
CamPos.Lon  = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, TheTime);  // TheTime
CamPos.Elev = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetValue(0, TheTime); // TheTime

#ifdef WCS_BUILD_VNS
DefCoords->DegToCart(&CamPos);
#else // WCS_BUILD_VNS
CamPos.DegToCart(GlobeRadius);
#endif // WCS_BUILD_VNS
} // ViewContext::FetchCamCoord

/*===========================================================================*/

void ViewContext::FetchCurCamCoord(void)
{
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

CamPos.Lat  = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue(0);
CamPos.Lon  = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue(0);
CamPos.Elev = VCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue(0);

#ifdef WCS_BUILD_VNS
DefCoords->DegToCart(&CamPos);
#else // WCS_BUILD_VNS
CamPos.DegToCart(GlobeRadius);
#endif // WCS_BUILD_VNS
} // ViewContext::FetchCurCamCoord

/*===========================================================================*/

void ViewContext::SetPlanet(PlanetOpt *NewPlanet)
{

Planet = NewPlanet;
GlobeRadius = GlobalApp->AppEffects->GetPlanetRadius();

}; // ViewContext::SetPlanet

/*===========================================================================*/

void ViewContext::CalcWorldBounds(Database *SceneDB)
{
Joe *Clip;
JoeDEM *MyDEM;
CelestialEffect *Celest;
VertexDEM Pos, Neg;
VertexDEM CelPos;
double MaxEl = -DBL_MAX, MinEl = DBL_MAX, ElRange = 0.0, Radius, SizeFac, TestFar;
//double NSUnit, EWUnit;
int DEMSteps = 0;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

NearCeles = DBL_MAX;
FarCeles = 0.0;

SceneBoundsPos[0] = -DBL_MAX;
SceneBoundsPos[1] = -DBL_MAX;
SceneBoundsPos[2] = -DBL_MAX;

SceneBoundsNeg[0] = DBL_MAX;
SceneBoundsNeg[1] = DBL_MAX;
SceneBoundsNeg[2] = DBL_MAX;

SceneLow  = DBL_MAX;
SceneHigh = -DBL_MAX;

SceneDB->ResetGeoClip();

//SceneDB->GetMinDEMCellSizeMeters(NSUnit, EWUnit);
//NSUnit = EWUnit = 30.0;
//ObjectUnit = ((NSUnit + EWUnit) / 2) * 5.0;

for (Clip = SceneDB->GetFirst(); Clip ; Clip = SceneDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				MaxEl = MyDEM->MaxEl;
				MinEl = MyDEM->MinEl;

				Pos.Lat  = Clip->NWLat;
				Pos.Lon  = Clip->NWLon;
				Pos.Elev = MaxEl;

				Neg.Lat  = Clip->SELat;
				Neg.Lon  = Clip->SELon;
				Neg.Elev = MinEl;

				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Pos);
				DefCoords->DegToCart(&Neg);
				#else // WCS_BUILD_VNS
				Pos.DegToCart(GlobeRadius);
				Neg.DegToCart(GlobeRadius);
				#endif // WCS_BUILD_VNS

				//SceneBoundsPos
				if (Pos.XYZ[0] > SceneBoundsPos[0]) SceneBoundsPos[0] = Pos.XYZ[0];
				if (Pos.XYZ[1] > SceneBoundsPos[1]) SceneBoundsPos[1] = Pos.XYZ[1];
				if (Pos.XYZ[2] > SceneBoundsPos[2]) SceneBoundsPos[2] = Pos.XYZ[2];
				//SceneBoundsNeg
				if (Neg.XYZ[0] < SceneBoundsNeg[0]) SceneBoundsNeg[0] = Neg.XYZ[0];
				if (Neg.XYZ[1] < SceneBoundsNeg[1]) SceneBoundsNeg[1] = Neg.XYZ[1];
				if (Neg.XYZ[2] < SceneBoundsNeg[2]) SceneBoundsNeg[2] = Neg.XYZ[2];

				if (MaxEl > SceneHigh)
					{
					SceneHigh = MaxEl;
					} // if
				if (MinEl < SceneLow)
					{
					SceneLow = MinEl;
					} // if
				} // 
			} // if
		else if (Clip->GetNumRealPoints() && Clip->GetFirstRealPoint())	// added this to avoid pitfall of default bounds too large
			{
				MaxEl = 0;
				MinEl = 0;

				// slight cheat
				MaxEl = Clip->GetFirstRealPoint()->Elevation *  2.0;
				MinEl = Clip->GetFirstRealPoint()->Elevation * -2.0;

				Pos.Lat  = Clip->NWLat;
				Pos.Lon  = Clip->NWLon;
				Pos.Elev = max(MaxEl, MinEl);

				Neg.Lat  = Clip->SELat;
				Neg.Lon  = Clip->SELon;
				Neg.Elev = min(MaxEl, MinEl);
				Neg.Elev = min(0.0, Neg.Elev);

				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Pos);
				DefCoords->DegToCart(&Neg);
				#else // WCS_BUILD_VNS
				Pos.DegToCart(GlobeRadius);
				Neg.DegToCart(GlobeRadius);
				#endif // WCS_BUILD_VNS

				//SceneBoundsPos
				if (Pos.XYZ[0] > SceneBoundsPos[0]) SceneBoundsPos[0] = Pos.XYZ[0];
				if (Pos.XYZ[1] > SceneBoundsPos[1]) SceneBoundsPos[1] = Pos.XYZ[1];
				if (Pos.XYZ[2] > SceneBoundsPos[2]) SceneBoundsPos[2] = Pos.XYZ[2];
				//SceneBoundsNeg
				if (Neg.XYZ[0] < SceneBoundsNeg[0]) SceneBoundsNeg[0] = Neg.XYZ[0];
				if (Neg.XYZ[1] < SceneBoundsNeg[1]) SceneBoundsNeg[1] = Neg.XYZ[1];
				if (Neg.XYZ[2] < SceneBoundsNeg[2]) SceneBoundsNeg[2] = Neg.XYZ[2];

				if (MaxEl > SceneHigh)
					{
					SceneHigh = MaxEl;
					} // if
				if (MinEl < SceneLow)
					{
					SceneLow = MinEl;
					} // if
			} // else
		} // if
	} // for

// exact copy of above with DYNAMIC
for (Clip = SceneDB->GetFirst(WCS_DATABASE_DYNAMIC); Clip ; Clip = SceneDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			DEMSteps++;
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				MaxEl = MyDEM->MaxEl;
				MinEl = MyDEM->MinEl;

				Pos.Lat  = Clip->NWLat;
				Pos.Lon  = Clip->NWLon;
				Pos.Elev = MaxEl;

				Neg.Lat  = Clip->SELat;
				Neg.Lon  = Clip->SELon;
				Neg.Elev = MinEl;

				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Pos);
				DefCoords->DegToCart(&Neg);
				#else // WCS_BUILD_VNS
				Pos.DegToCart(GlobeRadius);
				Neg.DegToCart(GlobeRadius);
				#endif // WCS_BUILD_VNS

				//SceneBoundsPos
				if (Pos.XYZ[0] > SceneBoundsPos[0]) SceneBoundsPos[0] = Pos.XYZ[0];
				if (Pos.XYZ[1] > SceneBoundsPos[1]) SceneBoundsPos[1] = Pos.XYZ[1];
				if (Pos.XYZ[2] > SceneBoundsPos[2]) SceneBoundsPos[2] = Pos.XYZ[2];
				//SceneBoundsNeg
				if (Neg.XYZ[0] < SceneBoundsNeg[0]) SceneBoundsNeg[0] = Neg.XYZ[0];
				if (Neg.XYZ[1] < SceneBoundsNeg[1]) SceneBoundsNeg[1] = Neg.XYZ[1];
				if (Neg.XYZ[2] < SceneBoundsNeg[2]) SceneBoundsNeg[2] = Neg.XYZ[2];

				if (MaxEl > SceneHigh)
					{
					SceneHigh = MaxEl;
					} // if
				if (MinEl < SceneLow)
					{
					SceneLow = MinEl;
					} // if
				} // 
			} // if
		else if (Clip->GetNumRealPoints() && Clip->GetFirstRealPoint())	// added this to avoid pitfall of default bounds too large
			{
				MaxEl = 0;
				MinEl = 0;

				// slight cheat
				MaxEl = Clip->GetFirstRealPoint()->Elevation *  2.0;
				MinEl = Clip->GetFirstRealPoint()->Elevation * -2.0;

				Pos.Lat  = Clip->NWLat;
				Pos.Lon  = Clip->NWLon;
				Pos.Elev = max(MaxEl, MinEl);

				Neg.Lat  = Clip->SELat;
				Neg.Lon  = Clip->SELon;
				Neg.Elev = min(MaxEl, MinEl);
				Neg.Elev = min(0.0, Neg.Elev);

				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&Pos);
				DefCoords->DegToCart(&Neg);
				#else // WCS_BUILD_VNS
				Pos.DegToCart(GlobeRadius);
				Neg.DegToCart(GlobeRadius);
				#endif // WCS_BUILD_VNS

				//SceneBoundsPos
				if (Pos.XYZ[0] > SceneBoundsPos[0]) SceneBoundsPos[0] = Pos.XYZ[0];
				if (Pos.XYZ[1] > SceneBoundsPos[1]) SceneBoundsPos[1] = Pos.XYZ[1];
				if (Pos.XYZ[2] > SceneBoundsPos[2]) SceneBoundsPos[2] = Pos.XYZ[2];
				//SceneBoundsNeg
				if (Neg.XYZ[0] < SceneBoundsNeg[0]) SceneBoundsNeg[0] = Neg.XYZ[0];
				if (Neg.XYZ[1] < SceneBoundsNeg[1]) SceneBoundsNeg[1] = Neg.XYZ[1];
				if (Neg.XYZ[2] < SceneBoundsNeg[2]) SceneBoundsNeg[2] = Neg.XYZ[2];

				if (MaxEl > SceneHigh)
					{
					SceneHigh = MaxEl;
					} // if
				if (MinEl < SceneLow)
					{
					SceneLow = MinEl;
					} // if
			} // else
		} // if
	} // for

for (Celest = (CelestialEffect *)(GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CELESTIAL));
 Celest; Celest = (CelestialEffect *)(Celest->Next))
	{
	if (Celest->Enabled)
		{
		Radius   = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].GetCurValue(0);
		SizeFac  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].GetCurValue(0);
		Radius  *= SizeFac;
		CelPos.Lat  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].GetCurValue(0);
		CelPos.Lon  = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].GetCurValue(0);
		CelPos.Elev = Celest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].GetCurValue(0);

		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(&CelPos);
		#else // WCS_BUILD_VNS
		CelPos.DegToCart(GlobeRadius);
		#endif // WCS_BUILD_VNS
		TestFar = PointDistance(CamPos.XYZ, CelPos.XYZ);
		if ((TestFar + 2 * Radius) > FarCeles)  FarCeles  = TestFar + 2 * Radius;
		if ((TestFar - 2 * Radius) < NearCeles) NearCeles = TestFar - 2 * Radius;
		} // if
	} // for

if ((SceneBoundsPos[0] == -DBL_MAX) || (SceneBoundsNeg[0] == DBL_MAX))
	{
	SceneBoundsPos[0] = 0;
	SceneBoundsPos[1] = 0;
	SceneBoundsPos[2] = 0;

	SceneBoundsNeg[0] = 0;
	SceneBoundsNeg[1] = 0;
	SceneBoundsNeg[2] = 0;
	} // if

} // ViewContext::CalcWorldBounds

/*===========================================================================*/

void ViewContext::SetOrigin(double Lat, double Lon, double Elev)
{
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

LocalOrigin.Lat  = Lat;
LocalOrigin.Lon  = Lon;
LocalOrigin.Elev = Elev;

#ifdef WCS_BUILD_VNS
DefCoords->DegToCart(&LocalOrigin);
#else // WCS_BUILD_VNS
LocalOrigin.DegToCart(GlobeRadius);
#endif // WCS_BUILD_VNS

// Build negated copy of local origin, for translating global cartesian coords to local system
CopyPoint3d(NegOrigin, LocalOrigin.XYZ);
NegateVector(NegOrigin);

// Build GL-notation negated copy of local origin, for glTranslated
CopyPoint3d(GLNegOrigin, NegOrigin);
InvertZ(GLNegOrigin);

} // ViewContext::SetOrigin

/*===========================================================================*/

void ViewContext::FetchGLProjection()
{

glGetDoublev (GL_MODELVIEW_MATRIX, GLModelMatrix);
glGetDoublev (GL_PROJECTION_MATRIX, GLProjMatrix);
glGetIntegerv(GL_VIEWPORT, GLViewport);

} // ViewContext::FetchGLProjection

/*===========================================================================*/

void ViewContext::GLProject(Point3d WorldXYZ, Point3d ScreenXYZ)
{

gluProject(WorldXYZ[0], WorldXYZ[1], WorldXYZ[2],
		   GLModelMatrix, GLProjMatrix, GLViewport,
		   &ScreenXYZ[0], &ScreenXYZ[1], &ScreenXYZ[2]);

} // ViewContext::GLProject

/*===========================================================================*/

void ViewContext::GLUnProject(Point3d WorldXYZ, Point3d ScreenXYZ)
{

gluUnProject(ScreenXYZ[0], ScreenXYZ[1], ScreenXYZ[2],
			 GLModelMatrix, GLProjMatrix, GLViewport,
			 &WorldXYZ[0], &WorldXYZ[1], &WorldXYZ[2]);

} // ViewContext::GLUnProject

/*===========================================================================*/

void ViewContext::EnableGradOverlay(int Which, int State)
{
int Sanify;

// turn off all but the item specified

for (Sanify = WCS_VIEWGUI_ENABLE_GRAD_GREY; Sanify <= WCS_VIEWGUI_ENABLE_OVER_ECOSYS; Sanify++)
	{
	if (Which == Sanify)
		{
		Enabled[Sanify] = (unsigned char)State;
		} // if
	else
		{
		Enabled[Sanify] = 0;
		} // if
	} // if

} // ViewContext::SanifyGradOverlay

/*===========================================================================*/

double ViewGUI::GLGetScaledPixelZ(ViewContext *OriVC, GLDrawingFenetre *GLDF, int X, int Y)
{
double DZ = 0.0;

if (GLDF && OriVC)
	{
	DZ = GLDF->GLGetPixelZ(X, Y);
	//glReadPixels(X, Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Z);
	DZ *= (OriVC->FarClip - OriVC->NearClip);
	DZ += OriVC->NearClip;
	} // if
return(DZ);

} // ViewGUI::GLGetScaledPixelZ

/*===========================================================================*/

Camera *ViewGUI::CreateNewCamera(char CamType)
{
int State = 0;
char *CamName = "Perspective Camera";
Camera *NewCam = NULL;

if (CamType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
	{
	CamName = "Overhead Camera";
	} // if
if (CamType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	CamName = "Planimetric Camera";
	} // if

if (NewCam = (Camera *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, CamName, NULL))
	{
	NewCam->CameraType = CamType;
	NewCam->SetFloating(1);
	} // if

return(NewCam);

} // ViewGUI::CreateNewCamera

/*===========================================================================*/

int ViewGUI::CreateNewViewAndCamera(char *DockCell, char CamType)
{
int State = 0;
Camera *NewCam = NULL;

if (NewCam = CreateNewCamera(CamType))
	{
	State = CreateNewView(DockCell, NewCam, NULL);
	} // if

return(State);

} // ViewGUI::CreateNewViewAndCamera

/*===========================================================================*/

static char FindPreview[50];

int ViewGUI::CreateNewView(char *DockCell, Camera *ViewCam, RenderOpt *ViewRO)
{
int InitLoop, EnableCt, ViewType, GoOn = 1;
unsigned long ViewWinID;
RenderOpt *Preview, *Scan;
RenderJob *RJ;
int ViewIsProjected = 0;

ViewWinID = 'VW00';

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO
if (!GlobalApp->Sentinal->CheckDongle()) return(NULL);
#endif // !WCS_BUILD_DEMO

if (!ViewRO)
	{ // default Render Preview Options
	Preview = NULL;
	for (Scan = (RenderOpt *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); Scan; Scan = (RenderOpt *)Scan->Next)
		{
		if (strstr(Scan->GetName(), "Preview Options"))
			{
			ViewRO = Preview = Scan;
			break;
			} // if
		} // for
	if (!Preview)
		{
		Scan = NULL;
		// Look through the render jobs, and see if we can find a RenderOpts to clone for
		// our previews
		for (RJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); RJ; RJ = (RenderJob *)RJ->Next)
			{
			if (RJ->Options)
				{
				Scan = RJ->Options;
				break;
				} // if
			} // for

		strncpy(FindPreview, GlobalApp->MainProj->projectname, 48);
		FindPreview[48] = NULL;
		StripExtension(FindPreview);
		strncat(FindPreview, " Preview Options", 48 - strlen(FindPreview));
		if (Preview = (RenderOpt *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, FindPreview, Scan))
			{
			ViewRO = Preview;
			} // if
		} // else
	} // else

#ifdef WCS_BUILD_VNS
// is this new View projected?
if (ViewCam->Projected && ViewCam->Coords)
	{
	ViewIsProjected = 1;
	} // if

#endif // WCS_BUILD_VNS


for (InitLoop = 0; GoOn && InitLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; InitLoop++)
	{
	if ((ViewWins[InitLoop] == NULL) && (ViewSpaces[InitLoop] == NULL))
		{
		GoOn = 0;
		ViewWinID += InitLoop;
		if (ViewSpaces[InitLoop] = new ViewContext)
			{
			ViewSpaces[InitLoop]->VCNum = InitLoop;
			if (ViewCam)
				{
				ViewSpaces[InitLoop]->SetCamera(ViewCam);
				// set enabled status and contour interval to user prefs for appropriate camera type
				ViewType = ViewCam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD ? WCS_VIEWGUI_VIEWTYPE_OVERHEAD:
					ViewCam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC ? WCS_VIEWGUI_VIEWTYPE_PLANIMETRIC: WCS_VIEWGUI_VIEWTYPE_PERSPECTIVE;
				for (EnableCt = 0; EnableCt < WCS_VIEWGUI_ENABLE_MAX; EnableCt ++)
					ViewSpaces[InitLoop]->SetEnabled(EnableCt, GlobalApp->MainProj->Prefs.ViewEnabled[ViewType][EnableCt]);
				ViewSpaces[InitLoop]->ContourOverlayInterval.SetCurValue(GlobalApp->MainProj->Prefs.ViewContOverlayInt[ViewType]);
				//if ((ViewCam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD) || (ViewCam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC))
				//	{
				//	ViewSpaces[InitLoop]->SetEnabled(WCS_VIEWGUI_ENABLE_ATMOS, 0);
				//	} // if
				} // if
			ViewSpaces[InitLoop]->SetSky((Sky *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_SKY));
			if (ViewRO)
				{
				ViewSpaces[InitLoop]->SetRenderOpt((RenderOpt *)ViewRO);
				} // if
			ViewSpaces[InitLoop]->SetPlanet((PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL));

			// We're going to force this to dock, so strip off any WCS_FENTRACK_FLAGS_UNDOCKED flags
			GlobalApp->MainProj->SetWindowFlags(ViewWinID, WCS_FENTRACK_FLAGS_UNDOCKED, WCS_FENTRACK_FLAGS_DISABLE);
			if (CreateNewViewFen(InitLoop, ViewWinID))
				{
				ConfigureTitle(InitLoop);
				LastActiveView = InitLoop;
				#ifdef WCS_BUILD_VNS
				if (ViewIsProjected)
					{
					// need to disable any other Views that are open and showing projected cameras
					for (int SafeLoop = 0; SafeLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; SafeLoop++)
						{
						// is there someone alive there?
						if (ViewWins[SafeLoop] && ViewSpaces[SafeLoop] && ViewSpaces[SafeLoop]->GetAlive())
							{
							// is it projected
							if (ViewSpaces[SafeLoop]->GetProjected())
								{
								// is it someone other than us?
								if (SafeLoop != InitLoop)
									{
									// switch him off, we win.
									ViewSpaces[SafeLoop]->SetAlive(0);
									ConfigureTitle(SafeLoop);
									} // if
								} // if
							} // if
						} // for

					// regen DEMs to pick up our projection.
					RegenDEMs = 1;
					} // if
				#endif // WCS_BUILD_VNS
				Draw(InitLoop);
				} // if
			else
				{
				delete ViewSpaces[InitLoop];
				ViewSpaces[InitLoop] = NULL;
				} // else
			} // if
		} // if
	} // for


return(0);
} // ViewGUI::CreateNewView

/*===========================================================================*/

GLDrawingFenetre *ViewGUI::CreateNewViewFen(int ViewSpaceNum, unsigned long ViewWinID)
{
char *ViewName = "Empty View";
const char *Str;

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO

if (!AppScope->Sentinal->CheckDongle()) return(NULL);

#endif // !WCS_BUILD_DEMO

if (ViewSpaces[ViewSpaceNum]->VCamera) ViewName = ViewSpaces[ViewSpaceNum]->VCamera->GetName();

if (ViewWins[ViewSpaceNum] = new GLDrawingFenetre(ViewWinID, this, ViewName))
	{
	ViewWins[ViewSpaceNum]->SetWinManFlags(WCS_FENETRE_WINMAN_ISVIEW|WCS_FENETRE_WINMAN_ISDIAG);
		{
		ViewWins[ViewSpaceNum]->CustomDataVoid[0] = ViewSpaces[ViewSpaceNum];
		if (ViewWins[ViewSpaceNum]->Open(GlobalApp->MainProj))
			{
			ViewWins[ViewSpaceNum]->fglInit();

			// Set GL config vars
			if (Str = (const char *)glGetString(GL_VENDOR))
				GlobalApp->SetGLVendorString(Str);
			if (Str = (const char *)glGetString(GL_RENDERER))
				GlobalApp->SetGLRenderString(Str);
			if (Str = (const char *)glGetString(GL_VERSION))
				GlobalApp->SetGLVersionString(Str);

			return(ViewWins[ViewSpaceNum]);
			} // if
		} // if
	delete ViewWins[ViewSpaceNum];
	ViewWins[ViewSpaceNum] = NULL;
	} // if
return(NULL);

} // ViewGUI::CreateNewViewFen

/*===========================================================================*/

void ViewGUI::ConfigureTitle(int ViewNum)
{
Camera *Cam;
RenderOpt *RendOpt;
char ViewTitleBuf[150];
bool ShowRONameInTitle = false;

if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("view_title_show_ro_name"))
	{
	ShowRONameInTitle = true;
	} // if

ViewTitleBuf[0] = NULL;

Cam = ViewSpaces[ViewNum]->GetCamera();
RendOpt = ViewSpaces[ViewNum]->GetRenderOpt();

if (Cam)
	{
	strncpy(ViewTitleBuf, Cam->GetName(), 145);
	} // if
if (RendOpt && ShowRONameInTitle)
	{
	strcat(ViewTitleBuf, " - ");
	strncat(ViewTitleBuf, RendOpt->GetName(), 143 - strlen(ViewTitleBuf));
	} // if
ViewTitleBuf[146] = NULL;
if (ViewTitleBuf[0] == NULL)
	{
	strcpy(ViewTitleBuf, "View");
	} // if

if (!ViewSpaces[ViewNum]->GetAlive())
	{
	strcat(ViewTitleBuf, " [Disabled]");
	} // if

ViewWins[ViewNum]->SetTitle(ViewTitleBuf);

} // ViewGUI::ConfigureTitle

/*===========================================================================*/

void ViewGUI::SetViewManipulationEnable(char NewState)
{
ViewManipulationEnable = NewState;
UpdateManipulationMode();
ActiveItemSwitch();

// clear per-view manipulation mode
// turn off View manipulation caption buttons
for (int InitLoop = 0; InitLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; InitLoop++)
	{
	if (ViewWins[InitLoop])
		{
		ViewSpaces[InitLoop]->ViewState &= ~(WCS_VIEWGUI_VIEWSTATE_VIEWMANIP);
		ViewWins[InitLoop]->UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_PAN | WCS_FENETRE_WINSTATE_ROTATE | WCS_FENETRE_WINSTATE_ZOOM);
		} // if
	} // for

ConfigureWidgets();
} // ViewGUI::SetViewManipulationEnable

/*===========================================================================*/

void ViewGUI::SetViewManipulationMode(char NewState)
{

ViewManipulationMode = NewState;
UpdateManipulationMode();
ActiveItemSwitch();
ConfigureWidgets();

} // ViewGUI::SetViewManipulationMode

/*===========================================================================*/

void ViewGUI::SetObjectManipulationMode(char NewState)
{

ObjectManipulationMode = NewState;
UpdateManipulationMode();
ConfigureWidgets();

} // ViewGUI::SetObjectManipulationMode

/*===========================================================================*/

void ViewGUI::UpdateManipulationMode(void)
{
char MMode;

// ObjectManipulationMode
if (ViewManipulationEnable) MMode = ViewManipulationMode;
else MMode = ObjectManipulationMode;

GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_MOVE,   MMode == WCS_VIEWGUI_MANIP_MOVE); // move
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_ROTATE, MMode == WCS_VIEWGUI_MANIP_ROT); // rotate
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_SCALEZOOM,  MMode == WCS_VIEWGUI_MANIP_SCALE); // scale

} // ViewGUI::UpdateManipulationMode

/*===========================================================================*/

void ViewGUI::UpdateAxes(void)
{
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_XAXIS,   AxisEnable[WCS_X] ? true : false); // x
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_YAXIS,	AxisEnable[WCS_Y] ? true : false); // y
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_ZAXIS,	AxisEnable[WCS_Z] ? true : false); // z
GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_ELEVAXIS,	AxisEnable[3] ? true : false); // elev
} // ViewGUI::UpdateAxes

/*===========================================================================*/

void ViewGUI::ActiveObjectChanged(RasterAnimHost *AO)
{
// <<<>>>
} // ViewGUI::ActiveObjectChanged

/*===========================================================================*/

void ViewGUI::CalcTexMapSize(JoeDEM *MyDEM, int &TexMapWidth, int &TexMapHeight)
{
GLint MaxTexSize, TestTexSize, TestDEMSize;

// Calculate max texmap size, limited by GL and the DEM
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTexSize);
TestDEMSize = MyDEM->Pristine->LonEntries();
/*
for (TestTexSize = 32; TestTexSize < MaxTexSize + 1; TestTexSize = TestTexSize << 1)
	{
	if ((TestTexSize > TestDEMSize) || (TestDEMSize - TestTexSize < (TestTexSize >> 1)))
		{
		break;
		} // if
	} // if
JVT->TexMapWidth = TestTexSize;
*/
TexMapWidth = TestTexSize = 32;
while (TestTexSize * 2 <= TestDEMSize)
	{
	TexMapWidth = (TestTexSize *= 2);
	} 

TestDEMSize = MyDEM->Pristine->LatEntries();
/*
for (TestTexSize = 32; TestTexSize < MaxTexSize + 1; TestTexSize = TestTexSize << 1)
	{
	if ((TestTexSize > TestDEMSize) || (TestDEMSize - TestTexSize < (TestTexSize >> 1)))
		{
		break;
		} // if
	} // if
JVT->TexMapHeight = TestTexSize;
*/
TexMapHeight = TestTexSize = 32;
while (TestTexSize * 2 <= TestDEMSize)
	{
	TexMapHeight = (TestTexSize *= 2);
	} 

} // ViewGUI::CalcTexMapSize

/*===========================================================================*/


int ViewGUI::GenTexMap(PlanetOpt *PO, Joe *Clip, JoeViewTemp *JVT, JoeDEM *MyDEM, int WhichMap, EnvironmentEffect *DefEnv, GroundEffect *DefGround, char CmapsExist)
{
double LatRange, LonRange, South, East;
double ElScale, ElScaleMult;
double Slope, TexCoordV, TexCoordU;
CoordSys *DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
MathAccellerators *mathTables = GlobalApp->MathTables;
unsigned char *TempGen = NULL, Unload = 0;
unsigned long LastVertA, LastVertB;	//lint -e530
unsigned long NextBaseVtx, LonCt, LatCt, BaseVtx, Ct;
long TexCoordX, TexCoordY;
int TexX, TexY;
VertexBase TempVert;
PolygonData Poly;	// creates and initializes members, needs to be initialized for each polygon rendered
unsigned char CompA, CompB, CompC;
char LastVertValid, PolyCt;

if (JVT->TexMapWidth == 0)
	{
	CalcTexMapSize(MyDEM, JVT->TexMapWidth, JVT->TexMapHeight);
	} // if

if ((JVT->TexMapWidth == 0) || (JVT->TexMapHeight == 0))
	{
	return(0);
	} // if


if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL)
	{
	if (JVT->RelelTex)
		{
		AppMem_Free(JVT->RelelTex, JVT->TexMapWidth * JVT->TexMapHeight); JVT->RelelTex = NULL;
		} // if
	if (JVT->RelelTex = TempGen = (unsigned char *)AppMem_Alloc(JVT->TexMapWidth * JVT->TexMapHeight, APPMEM_CLEAR))
		{
		if (MyDEM->Pristine)
			{
			if (Clip->AttemptLoadDEM(1, GlobalApp->MainProj))
				{
				MyDEM->Pristine->ScreenTransform(JVT->TexMapWidth, JVT->TexMapHeight);
				} //  if
			else
				{
				return(0);
				} // else
			} // if
		else
			{
			return(0);
			} // else
		} // if
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL)
	{
	if (JVT->FractTex)
		{
		AppMem_Free(JVT->FractTex, JVT->TexMapWidth * JVT->TexMapHeight); JVT->FractTex = NULL;
		} // if
	if (JVT->FractTex = TempGen = (unsigned char *)AppMem_Alloc(JVT->TexMapWidth * JVT->TexMapHeight, APPMEM_CLEAR))
		{
		if (MyDEM->Pristine)
			{
			#ifdef WCS_MULTIFRD
			MyDEM->Pristine->AttemptLoadFractalFile((char *)Clip->FileName(), GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, GlobalApp->AppDB)->GetName(), GlobalApp->MainProj);
			#else // WCS_MULTIFRD
			MyDEM->Pristine->AttemptLoadFractalFile((char *)Clip->FileName(), GlobalApp->MainProj);
			#endif // WCS_MULTIFRD
			MyDEM->Pristine->ScreenTransform(JVT->TexMapWidth, JVT->TexMapHeight);
			} // if
		else
			{
			return(0);
			} // else
		} // if
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE)
	{
	if (JVT->SlopeTex)
		{
		AppMem_Free(JVT->SlopeTex, JVT->TexMapWidth * JVT->TexMapHeight); JVT->SlopeTex = NULL;
		} // if
	if (JVT->SlopeTex = TempGen = (unsigned char *)AppMem_Alloc(JVT->TexMapWidth * JVT->TexMapHeight, APPMEM_CLEAR))
		{
		if (MyDEM->Pristine)
			{
			if (Clip->AttemptLoadDEM(1, GlobalApp->MainProj))
				{
				MyDEM->Pristine->ScreenTransform(JVT->TexMapWidth, JVT->TexMapHeight);
				} //  if
			else
				{
				return(0);
				} // else
			} // if
		else
			{
			return(0);
			} // else
		} // if
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
	{
	if (JVT->EcoTex)
		{
		AppMem_Free(JVT->EcoTex, 3 * JVT->TexMapWidth * JVT->TexMapHeight); JVT->EcoTex = NULL;
		} // if
	if (JVT->EcoTex = TempGen = (unsigned char *)AppMem_Alloc(3 * JVT->TexMapWidth * JVT->TexMapHeight, APPMEM_CLEAR))
		{
		if (MyDEM->Pristine)
			{
			if (Clip->AttemptLoadDEM(1, GlobalApp->MainProj))
				{
				MyDEM->Pristine->ScreenTransform(JVT->TexMapWidth, JVT->TexMapHeight);
				if (CmapsExist) GlobalApp->AppEffects->BuildDEMSpecificCmapList(MyDEM->Pristine);
				} //  if
			else
				{
				return(0);
				} // else
			} // if
		else
			{
			return(0);
			} // else
		} // if
	} // if
else
	{
	return(0);
	} // else

if (TempGen)
	{
	int RowIdx;
	double TexPosX, TexPosY;
	short FrD;

	if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL) || (WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL))
		{
		for (TexY = 0; TexY < JVT->TexMapHeight; TexY++)
			{
			RowIdx = TexY * JVT->TexMapWidth;
			for (TexX = 0; TexX < JVT->TexMapWidth; TexX++)
				{
				TexPosX = 1.0 - ((double)TexX / (double)JVT->TexMapWidth);
				TexPosY = 1.0 - ((double)TexY / (double)JVT->TexMapHeight);
				if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL)
					{
					//TempGen[(RowIdx + TexX)] = (unsigned char)(TexPosX * 255.0);
					TempGen[(RowIdx + TexX)] = 120 + (unsigned char)((double)(MyDEM->Pristine->GetRelElFromScreenCoord((JVT->TexMapWidth - 1) - TexX, (JVT->TexMapHeight - 1) - TexY, 250, -250)) * .45);
					} // if
				else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL)
					{
					//TempGen[(RowIdx + TexX)] = (unsigned char)(TexPosX * 255.0);
					FrD = MyDEM->Pristine->GetFractalFromScreenCoord((JVT->TexMapWidth - 1) - TexX, (JVT->TexMapHeight - 1) - TexY);
					TempGen[(RowIdx + TexX)] = (unsigned char)(FrD >= 0 ? 127 + FrD * 14: 0);
					} // if
/*
				else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE)
					{
					TempGen[(RowIdx + TexX)] = (unsigned char)(TexPosX * 255.0);
					} // if
				else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
					{
					TempGen[(RowIdx + TexX) * 3 + 0] = (unsigned char)(TexPosX * 255.0);
					TempGen[(RowIdx + TexX) * 3 + 1] = (unsigned char)(TexPosY * 255.0);
					TempGen[(RowIdx + TexX) * 3 + 2] = 255 - (unsigned char)(TexPosX * 255.0);
					} // if
*/
				} // for
			} // for
		} // if
	else
		{
		// Calculate some normals
		// no need to worry about vertices falling within the DEM if precision errors occur:
		// not sampling elevations using VertexDataPoint
		MyDEM->Pristine->TransferToVerticesLatLon(FALSE);
		ElScale = MyDEM->Pristine->pElScale;
		ElScaleMult = (ElScale / ELSCALE_METERS);
		CamSetup->IsTextureMap = 1;

		South = Clip->SELat;
		East  = Clip->SELon;
		LatRange = (Clip->NWLat - Clip->SELat);
		LonRange = (Clip->NWLon - Clip->SELon);
		// Project vertices into Cartesian World space
		for (Ct = LonCt = 0; LonCt < MyDEM->Pristine->LonEntries(); LonCt ++)
			{
			for (LatCt = 0; LatCt < MyDEM->Pristine->LatEntries(); Ct ++, LatCt ++)
				{
				// adjust elevation
				MyDEM->Pristine->Vertices[Ct].Elev = MyDEM->Pristine->Vertices[Ct].Elev * ElScaleMult;
				MyDEM->Pristine->Vertices[Ct].Elev = CalcExag(MyDEM->Pristine->Vertices[Ct].Elev, PO);
				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&MyDEM->Pristine->Vertices[Ct]);
				#else // WCS_BUILD_VNS
				MyDEM->Pristine->Vertices[Ct].DegToCart(GlobeRadius);
				#endif // WCS_BUILD_VNS
				} // for
			} // for

		for (Ct = LonCt = 0; LonCt < MyDEM->Pristine->LonEntries() - 1; LonCt ++)
			{
			LastVertValid = 0;
			BaseVtx = LonCt * MyDEM->Pristine->LatEntries();
			NextBaseVtx = BaseVtx + MyDEM->Pristine->LatEntries();
			for (LatCt = 0; LatCt < MyDEM->Pristine->LatEntries(); Ct ++, LatCt ++)
				{
				/*
				TexCoordLon = (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lon - East)  / LonRange; // lon
				TexCoordLat = (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat - South) / LatRange; // lat
				*/
				TexCoordU = 1.0 - ((double)LonCt / (double)(MyDEM->Pristine->LonEntries() - 1)); // lon
				TexCoordV = (double)LatCt / (double)(MyDEM->Pristine->LatEntries() - 1); // lat
				TexCoordX = (long)((TexCoordU * (JVT->TexMapWidth - 1)));
				TexCoordY = (long)((TexCoordV * (JVT->TexMapHeight - 1)));
				TexX = (JVT->TexMapWidth - 1) - TexCoordX;
				TexY = (JVT->TexMapHeight - 1) - TexCoordY;

				Slope = 0.0;
				CompA = CompB = CompC = 0;
				PolyCt = 0;
				Point3d PolyNorm;
				if (LastVertValid)
					{
					int NPoleCount = 0, SPoleCount = 0;
					if (MyDEM->Pristine->Vertices[LastVertA].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[LastVertB].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[LastVertA].Lat < -89.999) SPoleCount++;
					if (MyDEM->Pristine->Vertices[LastVertB].Lat < -89.999) SPoleCount++;
					if (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat < -89.999) SPoleCount++;

					if ((SPoleCount < 2) && (NPoleCount< 2))
						{
						// Generate normal for polygon (order is very important)
						// MakeNormal() creates an inverted normal compared to the Renderer
						MakeNormal(PolyNorm,
						 MyDEM->Pristine->Vertices[LastVertA].XYZ,
						 MyDEM->Pristine->Vertices[LastVertB].XYZ,
						 MyDEM->Pristine->Vertices[BaseVtx + LatCt].XYZ);
						// correct slope because of downward pointing normal
						//Slope += (180.0 - mathTables->ACosTab.Lookup(VectorAngle(MyDEM->Pristine->Vertices[LastVertA].XYZ, PolyNorm)) / PiOver180);	// in degrees
						Slope += (180.0 - mathTables->ACosTab.Lookup(VectorAngle(MyDEM->Pristine->Vertices[LastVertA].XYZ, PolyNorm)) * PiUnder180);	// in degrees
						PolyCt++;
						} // if

					NPoleCount = SPoleCount = 0;
					if (MyDEM->Pristine->Vertices[NextBaseVtx + LatCt].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[LastVertB].Lat > 89.999) NPoleCount++;
					if (MyDEM->Pristine->Vertices[NextBaseVtx + LatCt].Lat < -89.999) SPoleCount++;
					if (MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat < -89.999) SPoleCount++;
					if (MyDEM->Pristine->Vertices[LastVertB].Lat < -89.999) SPoleCount++;

					if ((SPoleCount < 2) && (NPoleCount < 2))
						{
						// Generate normal for polygon (order is very important)
						// MakeNormal() creates an inverted normal compared to the Renderer
						MakeNormal(PolyNorm,
						 MyDEM->Pristine->Vertices[NextBaseVtx + LatCt].XYZ,
						 MyDEM->Pristine->Vertices[BaseVtx + LatCt].XYZ,
						 MyDEM->Pristine->Vertices[LastVertB].XYZ);
						// correct slope because of downward pointing normal
						//Slope += (180.0 - mathTables->ACosTab.Lookup(VectorAngle(MyDEM->Pristine->Vertices[NextBaseVtx + LatCt].XYZ, PolyNorm)) / PiOver180);	// in degrees
						Slope += (180.0 - mathTables->ACosTab.Lookup(VectorAngle(MyDEM->Pristine->Vertices[NextBaseVtx + LatCt].XYZ, PolyNorm)) * PiUnder180);	// in degrees
						PolyCt++;
						} // if
					} // if
				if (PolyCt == 2)
					{
					// Average slope
					Slope = Slope / (double)PolyCt;
					} // if
				if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
					{
					if (PolyCt)
						{
						Poly.Lat = MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat;
						Poly.Lon = MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lon;
						Poly.Elev = MyDEM->Pristine->Vertices[BaseVtx + LatCt].Elev;
						//Poly.WaterElev = (Vtx[0]->WaterElev + Vtx[1]->WaterElev + Vtx[2]->WaterElev) / 3.0;
						//Poly.Z = (Vtx[0]->ScrnXYZ[2] + Vtx[1]->ScrnXYZ[2] + Vtx[2]->ScrnXYZ[2]) / 3.0;
						//Poly.Q = (Vtx[0]->Q + Vtx[1]->Q + Vtx[2]->Q) / 3.0;
						Poly.RelEl = MyDEM->Pristine->GetRelElFromScreenCoord(TexX, TexY);

						//Poly.LonSeed = (ULONG)(fabs((Vtx[0]->Map->pNorthWest.Lon - Poly.Lon) / Vtx[0]->Map->LonRange) * ULONG_MAX);
						//Poly.LatSeed = (ULONG)(fabs((Poly.Lat - Vtx[0]->Map->pSouthEast.Lat) / Vtx[0]->Map->LatRange) * ULONG_MAX);

						Poly.Slope = Slope;
						// MakeNormal() creates the opposite normal from the renderer so reverse it
						TempVert.XYZ[0] = -PolyNorm[0];
						TempVert.XYZ[1] = -PolyNorm[1];
						TempVert.XYZ[2] = -PolyNorm[2];
						TempVert.RotateY(-MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lon);
						TempVert.RotateX(90.0 - MyDEM->Pristine->Vertices[BaseVtx + LatCt].Lat);
						//Poly.Aspect = 0;
						Poly.Aspect = TempVert.FindRoughAngleYfromZ();
						Poly.Eco = NULL;
						Poly.Cmap = NULL;
						Poly.TintFoliage = NULL;
						

						if (CmapsExist)
							GlobalApp->AppEffects->EvalCmaps(CamSetup, &Poly);

						if (! Poly.Cmap || (! Poly.Eco && Poly.TintFoliage))
							{
							Poly.Env = DefEnv;
							Poly.Env->EvalEcosystem(&Poly);
							} // if
						if (Poly.Cmap)
							{
							if (Poly.Cmap && !Poly.Eco)
								{
								CompA = (unsigned char)(Poly.RGB[0] * 255.0);
								CompB = (unsigned char)(Poly.RGB[1] * 255.0);
								CompC = (unsigned char)(Poly.RGB[2] * 255.0);
								} // if
							} // if

						if (Poly.Eco)
							{
							Poly.Eco->EcoMat.GetBasicColor(CompA, CompB, CompC, 0.0);
							} // if
						else if (!Poly.Cmap)
							{
							// Fallthrough to default ground effect
							if (DefGround)
								{
								DefGround->EcoMat.GetBasicColor(CompA, CompB, CompC, 0.0);
								} // if
							} // else
						} // if
					} // if
				else
					{
					Slope *= 2.8;
					} // else
				// plot pixel
				if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
					{
					long Pos;
					Pos = TexCoordY * JVT->TexMapWidth + TexCoordX;
					if ((Pos < 3 * JVT->TexMapWidth * JVT->TexMapHeight) && (Pos >= 0))
						{
						TempGen[Pos * 3 + 0] = (unsigned char)CompA;
						TempGen[Pos * 3 + 1] = (unsigned char)CompB;
						TempGen[Pos * 3 + 2] = (unsigned char)CompC;
						} // if
					else
						{
						Pos = 1;
						} // else
					} // if
				else
					{
					long Pos;
					Pos = TexCoordY * JVT->TexMapWidth + TexCoordX;
					if ((Pos < JVT->TexMapWidth * JVT->TexMapHeight) && (Pos >= 0))
						{
						TempGen[Pos] = (unsigned char)Slope;
						} // if
					else
						{
						Pos = 1;
						} // else
					} // temp
				LastVertA = BaseVtx + LatCt;
				LastVertB = NextBaseVtx + LatCt;
				LastVertValid = 1;
				} // for
			} // for

		MyDEM->Pristine->FreeVertices();
		CamSetup->IsTextureMap = 0;
		} // else
	} // if

// per-object clean up
if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL)
	{
	if (MyDEM->Pristine)
		{
		MyDEM->Pristine->FreeRawElevs();
		MyDEM->Pristine->FreeRelElMap();
		} // if
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL)
	{
	if (MyDEM->Pristine)
		{
		MyDEM->Pristine->FreeFractalMap();
		} // if
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE)
	{
	} // if
else if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
	{
	} // if

return(1);

} // ViewGUI::GenTexMap

/*===========================================================================*/

int ViewGUI::GenTexMaps(PlanetOpt *PO, int WhichMap)
{
Joe *Clip;
JoeViewTemp *JVT;
JoeDEM *MyDEM;
EnvironmentEffect *CurrentEnv, *DefEnv= NULL;
EcosystemEffect *CurrentEco;
CmapEffect *CurrentCmap;
GroundEffect *CurrentGround, *DefGround = NULL;
char CmapsExist = 0;
BusyWin *Progress = NULL;
unsigned long DEMSteps, GenIt;

DEMSteps = 0;
if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS) && (!(DefEnv = (EnvironmentEffect *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0, NULL))))
	{
	UserMessageOK("Texture Mapper", "Unable to generate Ecosystem texture map.\nNo default Environment available.");
	return(0);
	} // if


if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
	{
	for (CurrentEnv = GlobalApp->AppEffects->Environment; CurrentEnv; CurrentEnv = (EnvironmentEffect *)CurrentEnv->Next)
		{
		CurrentEnv->InitToRender(NULL, NULL);
		CurrentEnv->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // for
	for (CurrentEco = GlobalApp->AppEffects->Ecosystem; CurrentEco; CurrentEco = (EcosystemEffect *)CurrentEco->Next)
		{
		CurrentEco->InitToRender(NULL, NULL);
		CurrentEco->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // for
	if (CmapsExist = (char)(GlobalApp->AppEffects->EnabledEffectExists(WCS_EFFECTSSUBCLASS_CMAP)))
		{
		for (CurrentCmap = GlobalApp->AppEffects->Cmap; CurrentCmap; CurrentCmap = (CmapEffect *)CurrentCmap->Next)
			{
			CurrentCmap->InitToRender(NULL, NULL);
			CurrentCmap->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
			} // for
		} // if
	// GroundEffect *CurrentGround, *DefGround;
	DefGround = (GroundEffect *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_GROUND, 0, NULL);
	for (CurrentGround = GlobalApp->AppEffects->Ground; CurrentGround; CurrentGround = (GroundEffect *)CurrentGround->Next)
		{
		CurrentGround->InitToRender(NULL, NULL);
		CurrentGround->InitFrameToRender(GlobalApp->AppEffects, CamSetup);
		} // for
	} // if

for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if (JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP))
					{
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS) && !(JVT->EcoTex))
						{
						DEMSteps++;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL) && !(JVT->FractTex))
						{
						DEMSteps++;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL) && !(JVT->RelelTex))
						{
						DEMSteps++;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE) && !(JVT->SlopeTex))
						{
						DEMSteps++;
						} // if
					} // if
				} // if
			} // if
		} // if
	} // for

if (!DEMSteps) return(0);

Progress = new BusyWin("Generate TexMaps", DEMSteps, 'BUSY', 0);

DEMSteps = 0;

for (Clip = LocalDB->GetFirst(); Clip && (!Progress || !Progress->CheckAbort()); Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if (JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP))
					{
					GenIt = 0;
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS) && !(JVT->EcoTex))
						{
						GenIt = 1;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL) && !(JVT->FractTex))
						{
						GenIt = 1;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL) && !(JVT->RelelTex))
						{
						GenIt = 1;
						} // if
					if ((WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE) && !(JVT->SlopeTex))
						{
						GenIt = 1;
						} // if
					if (GenIt)
						{
						GenTexMap(PO, Clip, JVT, MyDEM, WhichMap, DefEnv, DefGround, CmapsExist);
						} // if
					} // if
				} // if
			if (Progress) Progress->Update(DEMSteps++);
			} // if
		} // if
	} // for

if (Progress)
	{
	delete Progress;
	Progress = NULL;
	} // if

// Procedure Cleanup
if (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS)
	{
	if (CmapsExist)
		{
		for (CurrentCmap = GlobalApp->AppEffects->Cmap; CurrentCmap; CurrentCmap = (CmapEffect *)CurrentCmap->Next)
			{
			CurrentCmap->CleanupFromRender();
			} // for
		} // if
	} // if

return(1);

} // ViewGUI::GenTexMaps

/*===========================================================================*/

void ViewGUI::DeleteAllTexMaps(int WhichMap)
{
Joe *Clip;
JoeViewTemp *JVT;

for (Clip = LocalDB->GetFirst(); Clip ; Clip = LocalDB->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ACTIVATED) && Clip->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (JVT = (JoeViewTemp *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP))
				{
				if (WhichMap == -1)
					{
					JVT->DeleteTexMaps();
					} // if
				else
					{
					JVT->DeleteTexMap(WhichMap);
					} // else
				} // if
			} // if
		} // if
	} // for

} // ViewGUI::DeleteAllTexMaps

/*===========================================================================*/

void ViewGUI::TryFreeTex(int WhichMap)
{
int ViewNum, ShouldFree = 1;

// Make sure no one else is using this overlay map
for (ViewNum = 0; ViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewNum++)
	{
	if (ViewSpaces[ViewNum] && ViewWins[ViewNum])
		{
		if (ViewSpaces[ViewNum]->GetEnabled(WhichMap))
			{
			ShouldFree = 0;
			break; // no point in going on here
			} // if
		} // if
	} // for

if (ShouldFree)
	{
	DeleteAllTexMaps(WhichMap);
	} // if

} // ViewGUI::TryFreeTex

/*===========================================================================*/

void ViewGUI::Invalidate(RasterAnimHost *Invalid)
{
// make sure no views are relying on this camera/renderopt/sky/planetopt
int ViewNum, Clear;

for (ViewNum = 0; ViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewNum++)
	{
	Clear = 0;
	if (ViewSpaces[ViewNum])
		{
		if (ViewSpaces[ViewNum]->VCamera == Invalid)
			{
			ViewSpaces[ViewNum]->VCamera = NULL;
			Clear = 1;
			} // if
		if (ViewSpaces[ViewNum]->RO == Invalid)
			{
			ViewSpaces[ViewNum]->RO = NULL;
			Clear = 1;
			} // if
		if (ViewSpaces[ViewNum]->VSky == Invalid)
			{
			ViewSpaces[ViewNum]->VSky = NULL;
			Clear = 1;
			} // if
		if (ViewSpaces[ViewNum]->Planet == Invalid)
			{
			ViewSpaces[ViewNum]->Planet = NULL;
			Clear = 1;
			} // if
		if (Clear)
			{
			if (ViewWins[ViewNum])
				{
				ViewWins[ViewNum]->Clear();
				} // if
			} // if
		} // if
	} // for

} // ViewGUI::Invalidate

/*===========================================================================*/

void ViewGUI::CloseViewsForIO(int AlterQuiet)
{
int KillLoop;

if (AlterQuiet) QuietDuringLoad = 1;

if (PrefsGUI)
	{
	delete PrefsGUI;
	PrefsGUI = NULL;
	} // if

for (KillLoop = 0; KillLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; KillLoop++)
	{
	DoClose(KillLoop);
	} // for

} // ViewGUI::CloseViewsForIO

/*===========================================================================*/

extern int suppress_openviews;

void ViewGUI::ReopenIOComplete(int AlterQuiet)
{
int OpenLoop;
char CheckID[4];

if (AlterQuiet) QuietDuringLoad = 0;

if (suppress_openviews) return;

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO

if (!GlobalApp->Sentinal->CheckDongle()) return;
#endif // !WCS_BUILD_DEMO

for (OpenLoop = 0; OpenLoop < WCS_VIEWGUI_VIEWS_OPEN_MAX; OpenLoop++)
	{
	if (ViewSpaces[OpenLoop] && !ViewWins[OpenLoop])
		{
		if (ViewSpaces[OpenLoop]->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_SLOPE)) RegenSlopeTex = 1;
		if (ViewSpaces[OpenLoop]->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_RELEL)) RegenRelelTex = 1;
		if (ViewSpaces[OpenLoop]->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_FRACTAL)) RegenFractTex = 1;
		if (ViewSpaces[OpenLoop]->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_ECOSYS)) RegenEcoTex = 1;
		ViewSpaces[OpenLoop]->Enabled[WCS_VIEWGUI_ENABLE_RENDERPREV] = 0;
		ViewSpaces[OpenLoop]->SetPlanet((PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL));
		ViewSpaces[OpenLoop]->SetSky((Sky *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_SKY));
		GlobalApp->MainProj->InquireWindowCell(ViewSpaces[OpenLoop]->VCWinID, CheckID);
		GlobalApp->MainProj->ViewPorts.SetFlag(CheckID, WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
		CreateNewViewFen(OpenLoop, ViewSpaces[OpenLoop]->VCWinID);
		ConfigureTitle(OpenLoop);
		} // if
	} // for

} // ViewGUI::ReopenIOComplete

/*===========================================================================*/

unsigned long ViewGUI::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long LoadNum = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_VIEWINIT_FOLWRITECONFIG:
						{
						BytesRead = RTFWriteConf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAYCONFIG:
						{
						BytesRead = RTFDispConf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_VIEWCONTEXT:
						{
						if (LoadNum < WCS_VIEWGUI_VIEWS_OPEN_MAX && (ViewSpaces[LoadNum] = new ViewContext()))
							{
							BytesRead = ViewSpaces[LoadNum]->Load(ffile, Size, ByteFlip);
							ViewSpaces[LoadNum]->VCNum = (char)LoadNum;
							LoadNum ++;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // ViewGUI::Load

/*===========================================================================*/

unsigned long ViewContext::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char LoadName[256], Sanify, Hit;
long NumEnabled = 0, EnabledCt = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_VIEWCONTEXT_CAMERANAME:
						{
						BytesRead = ReadBlock(ffile, (char *)LoadName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (LoadName[0])
							{
							VCamera = (Camera *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, LoadName);
							} // if
						break;
						}
					case WCS_VIEWCONTEXT_OPTIONSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)LoadName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (LoadName[0])
							{
							RO = (RenderOpt *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_RENDEROPT, LoadName);
							} // if
						break;
						}
					case WCS_VIEWCONTEXT_VCWINID:
						{
						BytesRead = ReadBlock(ffile, (char *)&VCWinID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_FENID:
						{
						BytesRead = ReadBlock(ffile, (char *)&VPID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_NUMENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumEnabled, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_ENABLED:
						{
						if (EnabledCt < WCS_VIEWGUI_ENABLE_MAX && EnabledCt < NumEnabled)
							BytesRead = ReadBlock(ffile, (char *)&Enabled[EnabledCt ++], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_VIEWCONTEXT_LTDREGIONCTRX:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderRegion[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_LTDREGIONCTRY:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderRegion[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_LTDREGIONWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderRegion[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_LTDREGIONHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderRegion[3], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_VIEWCONTEXT_CONTOURINTERVAL:
						{
						BytesRead = ContourOverlayInterval.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

// Ensure only one of the mutex options is on at any given time
Hit = 0;
for (Sanify = WCS_VIEWGUI_ENABLE_GRAD_GREY; Sanify <=	WCS_VIEWGUI_ENABLE_OVER_RENDER; Sanify++)
	{
	if (Enabled[Sanify])
		{
		if (Hit)
			{
			Enabled[Sanify] = 0;
			} // if
		Hit = 1;
		} // if
	} // for

return (TotalRead);

} // ViewContext::Load

/*===========================================================================*/

unsigned long ViewGUI::Save(FILE *ffile)
{
ULONG Ct, ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_VIEWINIT_FOLWRITECONFIG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = RTFWriteConf.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if foliage write config saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_VIEWINIT_FOLDISPLAYCONFIG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = RTFDispConf.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if foliage display config saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

for (Ct = 0; Ct < WCS_VIEWGUI_VIEWS_OPEN_MAX; Ct++)
	{
	if (ViewWins[Ct] && ViewSpaces[Ct])
		{
		// update IDs
		ViewSpaces[Ct]->VCWinID = ViewWins[Ct]->FenID;
		strncpy(ViewSpaces[Ct]->VPID, ViewWins[Ct]->GetVPID(), 4);

		ItemTag = WCS_VIEWINIT_VIEWCONTEXT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ViewSpaces[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if view context saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ViewGUI::Save


unsigned long ViewContext::Save(FILE *ffile)
{
ULONG Ct, ItemTag, TotalWritten = 0, NumEnabled = WCS_VIEWGUI_ENABLE_MAX;
long BytesWritten;

if (VCamera)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_CAMERANAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(VCamera->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)VCamera->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (RO)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_OPTIONSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(RO->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)RO->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_VCWINID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&VCWinID)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_FENID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&VPID)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_NUMENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < NumEnabled; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_ENABLED, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&Enabled[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_LTDREGIONCTRX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&RenderRegion[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_LTDREGIONCTRY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&RenderRegion[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_LTDREGIONWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&RenderRegion[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWCONTEXT_LTDREGIONHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&RenderRegion[3])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_VIEWCONTEXT_CONTOURINTERVAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ContourOverlayInterval.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if contour interval saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ViewContext::Save

/*===========================================================================*/

bool ViewContext::DismissRendererIfPresent(bool AskRetainDiagnosticsFirst)
{
if (CheckRendererPresent())
	{
	if (GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
		{
		SetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV, false);
		if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("suppress_retain_diagdata"))
			{
			AskRetainDiagnosticsFirst = false;
			} // if
		if (!AskRetainDiagnosticsFirst || !UserMessageYN("View", "Retain diagnostic data?", 1))
			{
			// Clear Renderer
			if (BigR)
				{
				if (BigR->Opt) delete BigR->Opt;
				BigR->Opt = NULL;
				delete BigR;
				BigR = NULL;
				return(true);
				} // if
			} // if
		} // if
	} // if
return(false);

} // ViewContext::DismissRendererIfPresent

/*===========================================================================*/

void ViewContext::ShowRendererIfPresent(void)
{
NotifyTag Changes[2];
if (CheckRendererPresent())
	{
	if (!GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV))
		{ // update window with rendered image
		SetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV, true);
		Changes[1] = 0;
		if (GlobalApp->GUIWins->RDG)
			{
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF, 0, GlobalApp->GUIWins->RDG->DisplayBuffer);
			} // if
		else
			{
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF, 0, WCS_DIAGNOSTIC_RGB);
			} // else
		GlobalApp->AppEx->GenerateNotify(Changes);
		} // if
	} // if
} // ViewContext::ShowRendererIfPresent

/*===========================================================================*/

NativeGUIWin ViewPrefsGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	SECURITY_INLINE_CHECK(054, 54);
	WidgetTCSetCurSel(IDC_TAB1, ActivePage);
	ShowPanel(0, ActivePage == 0 ? 0 : 1);
	ConfigureWidgets();
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ViewPrefsGUI::Open

/*===========================================================================*/

NativeGUIWin ViewPrefsGUI::Construct(void)
{
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DISPPREFS, GlobalApp->WinSys->RootWin);
	CreateSubWinFromTemplate(IDD_DISPPREFS_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_DISPPREFS_ENABLE1, 0, 1);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < 6; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		} // if

	} // if
 
return(NativeWin);

} // ViewPrefsGUI::Construct

/*===========================================================================*/

ViewPrefsGUI::ViewPrefsGUI()
: GUIFenetre('VPRF', this, "View Preferences") // Yes, I know...
{
ConstructError = 0;
SetActiveView(0); SetActiveView(-1); // So Prev is set too
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // ViewPrefsGUI::ViewPrefsGUI

/*===========================================================================*/

ViewPrefsGUI::~ViewPrefsGUI()
{
// prefs can be closed by calling destructor if a view is closed 
// so need to remove from menu list here too.
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ViewPrefsGUI::~ViewPrefsGUI()

/*===========================================================================*/

long ViewPrefsGUI::HandleCloseWin(NativeGUIWin NW)
{

// no need to destroy window while ViewGUI is still around.
GlobalApp->MCP->RemoveWindowFromMenuList(this);
Close();
return(0);

} // ViewPrefsGUI::HandleCloseWin

/*===========================================================================*/

long ViewPrefsGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
TerrainParamEffect *TP;
ViewContext *TheVC = NULL;

switch (ButtonID)
	{
	case IDC_EDITMORE:
		{
		if (GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->VCamera)
			{
			GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->VCamera->Edit();
			} // if
		break;
		} // 
	case IDC_EDITMORE2:
		{
		if (GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->RO)
			{
			GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->RO->Edit();
			} // if
		break;
		} // 
	case IDC_EDITMORE3:
		{
		if (TP = (TerrainParamEffect *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, NULL))
			{
			TP->Edit();
			} // if
		break;
		} // 
	case IDC_EDITMORE4:
		{ // regen texture
		if (TheVC = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()])
			{
			if (TheVC->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_SLOPE))
				{
				GlobalApp->GUIWins->CVG->RegenSlopeTex = 1;
				GlobalApp->GUIWins->CVG->DeleteAllTexMaps(WCS_VIEWGUI_ENABLE_OVER_SLOPE);
				} // if
			else if (TheVC->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_RELEL))
				{
				GlobalApp->GUIWins->CVG->RegenRelelTex = 1;
				GlobalApp->GUIWins->CVG->DeleteAllTexMaps(WCS_VIEWGUI_ENABLE_OVER_RELEL);
				} // else if
			else if (TheVC->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_FRACTAL))
				{
				GlobalApp->GUIWins->CVG->RegenFractTex = 1;
				GlobalApp->GUIWins->CVG->DeleteAllTexMaps(WCS_VIEWGUI_ENABLE_OVER_FRACTAL);
				} // else if
			else if (TheVC->GetEnabled(WCS_VIEWGUI_ENABLE_OVER_ECOSYS))
				{
				GlobalApp->GUIWins->CVG->RegenEcoTex = 1;
				GlobalApp->GUIWins->CVG->DeleteAllTexMaps(WCS_VIEWGUI_ENABLE_OVER_ECOSYS);
				} // else if
			else return(0);
			GlobalApp->GUIWins->CVG->DrawView(GetActiveView(), TheVC);
			} // if
		break;
		} // 
	case IDC_EDITMORE5:
		{
		GlobalApp->GUIWins->CVG->DoRTPrefs();
		break;
		} // IDC_EDITMORE5
	case IDC_COPYPREFS:
		{
		char ViewType;
	
		if (TheVC = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()])
			{
			if (TheVC->VCamera)
				{
				ViewType = TheVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD ? WCS_VIEWGUI_VIEWTYPE_OVERHEAD:
					TheVC->VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC ? WCS_VIEWGUI_VIEWTYPE_PLANIMETRIC: WCS_VIEWGUI_VIEWTYPE_PERSPECTIVE;
				memcpy(&GlobalApp->MainProj->Prefs.ViewEnabled[ViewType][0], &TheVC->Enabled[0], WCS_VIEWGUI_ENABLE_MAX);
				GlobalApp->MainProj->Prefs.ViewContOverlayInt[ViewType] = TheVC->ContourOverlayInterval.CurValue;
				} // if
			} // if
		break;
		} //
	} // switch

return(0);

} // ViewPrefsGUI::HandleButtonClick

/*===========================================================================*/

void ViewPrefsGUI::HandleNotifyEvent(void)
{
} // ViewPrefsGUI::HandleNotifyEvent()

/*===========================================================================*/

long ViewPrefsGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

GlobalApp->GUIWins->CVG->Draw(-1);
return(0);

} // ViewPrefsGUI::HandleFIChange

/*===========================================================================*/

long ViewPrefsGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
Camera *ActiveCam = NULL;
RenderOpt *Active = NULL;
ViewGUI *VG;
long CurrentItem;
int AV;

if (!(VG = GlobalApp->GUIWins->CVG)) return(0);

if ((AV = GetActiveView()) != -1)
	{
	switch (CtrlID)
		{
		case IDC_OPTLIST:
			{
			CurrentItem = WidgetCBGetCurSel(IDC_OPTLIST);
			if (CurrentItem != CB_ERR)
				{
				Active = (RenderOpt *)WidgetCBGetItemData(IDC_OPTLIST, CurrentItem);
				if (Active && GlobalApp->AppEffects->IsEffectValid(Active, WCS_EFFECTSSUBCLASS_RENDEROPT, 0))
					{
					if (VG->ViewSpaces[AV])
						{
						VG->ViewSpaces[AV]->SetRenderOpt(Active);
						VG->ConfigureTitle(AV);
						ConfigureWidgets();
						VG->Draw(AV);
						} // if
					} // if
				} // if
			break;
			} // IDC_OPTLIST
		case IDC_CAMLIST:
			{
			CurrentItem = WidgetCBGetCurSel(IDC_CAMLIST);
			if (CurrentItem != CB_ERR)
				{
				ActiveCam = (Camera *)WidgetCBGetItemData(IDC_CAMLIST, CurrentItem);
				if (ActiveCam && GlobalApp->AppEffects->IsEffectValid(ActiveCam, WCS_EFFECTSSUBCLASS_CAMERA, 0))
					{
					if (VG->ViewSpaces[AV])
						{
						VG->ViewSpaces[AV]->SetCamera(ActiveCam);
						VG->ConfigureTitle(AV);
						ConfigureWidgets();
						VG->Draw(AV);
						} // if
					} // if
				} // if
			break;
			} // IDC_CAMLIST
		} // switch
	} // if

return(0);

} // ViewPrefsGUI::HandleCBChange

/*===========================================================================*/

long ViewPrefsGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
RenderOpt *Active = NULL;
int CurState = 0, AV;
ViewGUI *VG;

if (!(VG = GlobalApp->GUIWins->CVG)) return(0);

if ((AV = GetActiveView()) != -1)
	{
	Active = VG->ViewSpaces[GetActiveView()]->RO;
	} // if

Changes[1] = 0;

if ((AV >= 0) && (ActivePage == 5))
	{
	CurState = WidgetGetCheck(CtrlID);
	switch (CtrlID)
		{
		case IDC_CHECKREAL1: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_CONTOURS, CurState); break;
		case IDC_CHECKREAL2: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_SLOPE, CurState);  if (CurState) VG->RegenSlopeTex = 1;  else VG->TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_SLOPE); break;
		case IDC_CHECKREAL3: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_RELEL, CurState);  if (CurState) VG->RegenRelelTex = 1;  else VG->TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_RELEL); break;
		case IDC_CHECKREAL4: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_FRACTAL, CurState);  if (CurState) VG->RegenFractTex = 1;  else VG->TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_FRACTAL); break;
		case IDC_CHECKREAL5: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_OVER_ECOSYS, CurState); if (CurState) VG->RegenEcoTex = 1;  else VG->TryFreeTex(WCS_VIEWGUI_ENABLE_OVER_ECOSYS); break;
		case IDC_CHECKREAL6: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_GREY, CurState); break;
		case IDC_CHECKREAL7: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_EARTH, CurState); break;
		case IDC_CHECKREAL8: VG->ViewSpaces[AV]->EnableGradOverlay(WCS_VIEWGUI_ENABLE_GRAD_PRIMARY, CurState); break;
		} // switch
	ConfigureWidgets();
	} // if

switch (CtrlID)
	{
	case IDC_CHECKREND1:
	case IDC_CHECKREND2:
	case IDC_CHECKREND3:
	case IDC_CHECKREND4:
	case IDC_CHECKREND5:
	case IDC_CHECKREND6:
	case IDC_CHECKREND7:
	case IDC_CHECKREND8:
	case IDC_CHECKREND9:
	case IDC_CHECKREND10:
	case IDC_CHECKREND11:
	case IDC_CHECKREND12:
		{
		if (Active)
			{
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_CHECKTFXPREV:
		{
		GlobalApp->GUIWins->CVG->RegenDEMs = 1;
		GlobalApp->GUIWins->CVG->Draw(-1);
		break;
		} // IDC_CHECKTFXPREV
	default:
		{
		if (AV != -1)
			{
			GlobalApp->GUIWins->CVG->Draw(AV);
			} // if
		break;
		} // default
	} // switch CtrlID

if (AV != -1)
	{
	GlobalApp->GUIWins->CVG->ConfigureTitle(AV);
	} // if
return(0);

} // ViewPrefsGUI::HandleSCChange

/*===========================================================================*/

long ViewPrefsGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		if (ActiveViewNum == -1) // can't switch to another page if not attached to a view
			{
			NewPageID = 0;
			WidgetTCSetCurSel(IDC_TAB1, 0);
			} // if
		switch (NewPageID)
			{
			case 0:
				{
				ShowPanel(0, 0);
				break;
				} // 1
			default:
				{
				ShowPanel(0, 1);
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

ActivePage = NewPageID;
ConfigureWidgets();
return(0);

} // ViewPrefsGUI::HandlePageChange

/*===========================================================================*/

void ViewPrefsGUI::ShowLine(int Line, int Show)
{

WidgetShow(VPGItemIDs[Line], Show);
WidgetShow(VPGItemIDs[Line], Show);
WidgetShow(VPGRealIDs[Line], Show);
WidgetShow(VPGRendIDs[Line], Show);

} // ViewPrefsGUI::ShowLine

/*===========================================================================*/

void ViewPrefsGUI::ShowLines(int Lines)
{
int LineScan;

for (LineScan = 1; LineScan < WCS_VIEWGUI_VPG_NUMLINES + 1; LineScan++)
	{
	ShowLine(LineScan, (LineScan <= Lines));
	} // for

} // ViewPrefsGUI::ShowLines

/*===========================================================================*/

void ViewPrefsGUI::EnableChecks(int Line, int Real, int Rend)
{
//Rend = 0;
WidgetSetDisabled(VPGRealIDs[Line], !Real);
if (!Real)
	{
	WidgetSetCheck(VPGRealIDs[Line], Real);
	WidgetSCConfig(VPGRealIDs[Line], NULL, NULL);
	} // if
WidgetSetDisabled(VPGRendIDs[Line], !Rend);
if (!Rend)
	{
	WidgetSetCheck(VPGRendIDs[Line], 0);
	WidgetSCConfig(VPGRendIDs[Line], NULL, NULL);
	} // if

} // ViewPrefsGUI::EnableChecks

/*===========================================================================*/

void ViewPrefsGUI::DisableRendCheck(int Line)
{

WidgetSetDisabled(VPGRendIDs[Line], 1);
WidgetSetCheck(VPGRendIDs[Line], 0);
WidgetSCConfig(VPGRendIDs[Line], NULL, NULL);

} // ViewPrefsGUI::DisableRendCheck

/*===========================================================================*/

void ViewPrefsGUI::SetChecks(int Line, int Real, int Rend)
{

WidgetSetCheck(VPGRealIDs[Line], Real);
WidgetSetCheck(VPGRendIDs[Line], Rend);

} // ViewPrefsGUI::SetChecks

/*===========================================================================*/

void ViewPrefsGUI::SetupLine(int Line, char *Name, int Real, int Rend)
{

WidgetSetText(VPGItemIDs[Line], Name);
EnableChecks(Line, Real, Rend);

} // ViewPrefsGUI::SetupLine

/*===========================================================================*/

void ViewPrefsGUI::SetupLabel(int Line, char *Name)
{

WidgetSetText(VPGLabelIDs[Line], Name);
WidgetShow(VPGLabelIDs[Line], 1);

} // ViewPrefsGUI::SetupLabel

/*===========================================================================*/

void ViewPrefsGUI::SetupReal(int Line, int EnableItem)
{
ViewContext *ToConfig;
void *MV;

if (GlobalApp->GUIWins->CVG)
	{
	if (ToConfig = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()])
		{
		MV = &(ToConfig->Enabled[EnableItem]);
		WidgetSCConfig(VPGRealIDs[Line], MV, SCFlag_Char);
		} // if
	} // if

} // ViewPrefsGUI::SetupReal

/*===========================================================================*/

void ViewPrefsGUI::SetupRend(int Line, void *Item)
{

WidgetSCConfig(VPGRendIDs[Line], Item, SCFlag_Char);

} // ViewPrefsGUI::SetupRend

/*===========================================================================*/

void ViewPrefsGUI::ClearLabels(void)
{
int LineScan;

for (LineScan = 1; LineScan < WCS_VIEWGUI_VPG_NUMLINES; LineScan++)
	{
	WidgetShow(VPGLabelIDs[LineScan], 0);
	} // for

} // ViewPrefsGUI::ClearLabels

/*===========================================================================*/

void ViewPrefsGUI::ConfigureWidgets(void)
{
Camera *ListCam = NULL, *ActiveCam = NULL;
RenderOpt *ListRO = NULL, *Active = NULL;
int WorkingLine, ItemNum, CursorOk = 0, EnableOk = 0;

if (!NativeWin) return;

// Configure General page
ConfigureFI(NativeWin, IDC_VIEWGRID, NULL, 1000.0, 1000.0, 4000000.0, FIOFlag_Long, GlobalApp->MainProj->Interactive, MAKE_ID(WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_GRANULARITY, WCS_INTERCAM_ITEM_GRIDSAMPLE, 0));
WidgetSNConfig(IDC_GRIDUNIT, &GlobalApp->MainProj->Interactive->GridOverlaySize);

#ifdef WCS_VIEW_TERRAFFECTORS
ConfigureSC(NativeWin, IDC_CHECKTFXPREV, NULL, SCFlag_Char, GlobalApp->MainProj->Interactive, MAKE_ID(WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_SETTINGS, WCS_INTERCAM_ITEM_SETTINGS_TFXPREVIEW, 0));
ConfigureSC(NativeWin, IDC_CHECKTFXPREVAUTO, NULL, SCFlag_Char, GlobalApp->MainProj->Interactive, MAKE_ID(WCS_INTERCLASS_CAMVIEW, WCS_INTERCAM_SUBCLASS_SETTINGS, WCS_INTERCAM_ITEM_SETTINGS_TFXREALTIME, 0));
#else // !WCS_VIEW_TERRAFFECTORS
WidgetShow(IDC_CHECKTFXPREV, 0);
WidgetShow(IDC_CHECKTFXPREVAUTO, 0);
#endif // !WCS_VIEW_TERRAFFECTORS

if (ActiveViewNum != -1)
	{ // safegaurd
	if (GlobalApp->GUIWins->CVG->ViewSpaces[ActiveViewNum])
		{
		if (!((GlobalApp->GUIWins->CVG->ViewSpaces[ActiveViewNum]->VCamera) && (GlobalApp->GUIWins->CVG->ViewSpaces[ActiveViewNum]->RO)))
			{
			SetActiveView(-1);
			} // if
		} // if
	else
		{
		SetActiveView(-1);
		return;
		} // else
	} // if

if (GetActiveView() != -1)
	{
	Active = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->RO;
	ActiveCam = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->VCamera;

	// populate and set droplists
	WidgetCBClear(IDC_OPTLIST);
	WidgetCBSetCurSel(IDC_OPTLIST, -1);
	ItemNum = 0;
	for (ListRO = (RenderOpt *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); ListRO; ListRO = (RenderOpt *)ListRO->Next)
		{
		WidgetCBAddEnd(IDC_OPTLIST, ListRO->GetName());
		WidgetCBSetItemData(IDC_OPTLIST, ItemNum, ListRO);
		if (Active == ListRO) WidgetCBSetCurSel(IDC_OPTLIST, ItemNum);
		ItemNum++;
		} // for
	WidgetCBClear(IDC_CAMLIST);
	WidgetCBSetCurSel(IDC_CAMLIST, -1);
	ItemNum = 0;
	for (ListCam = (Camera *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); ListCam; ListCam = (Camera *)ListCam->Next)
		{
		WidgetCBAddEnd(IDC_CAMLIST, ListCam->GetName());
		WidgetCBSetItemData(IDC_CAMLIST, ItemNum, ListCam);
		if (ActiveCam == ListCam) WidgetCBSetCurSel(IDC_CAMLIST, ItemNum);
		ItemNum++;
		} // for

	if (ActiveCam)
		{
		WidgetSmartRAHConfig(IDC_LAT, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], ActiveCam);
		WidgetSmartRAHConfig(IDC_LON, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], ActiveCam);
		WidgetSmartRAHConfig(IDC_SCALE, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], ActiveCam);
		WidgetSNConfig(IDC_CONTINT, &GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->ContourOverlayInterval);
		ConfigureSC(NativeWin, IDC_CHECKFOLLOW, NULL, SCFlag_Char, GlobalApp->MainProj->Interactive, MAKE_ID(WCS_INTERCLASS_MAPVIEW, 0, WCS_INTERMAP_ITEM_FOLLOWTERRAIN, 0));
		if (GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->IsPlan() || GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()]->VCamera->Orthographic)
			{
			WidgetSetText(IDC_VIEWARC, "Width ");
			WidgetSmartRAHConfig(IDC_VIEWARC, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH], ActiveCam);
			} // if
		else
			{
			WidgetSetText(IDC_VIEWARC, "HFOV ");
			WidgetSmartRAHConfig(IDC_VIEWARC, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV], ActiveCam);
			} // if
		} // if
	if (Active)
		{
		ViewContext *ToConfig;
		void *MV;
		if (GlobalApp->GUIWins->CVG)
			{
			if (ToConfig = GlobalApp->GUIWins->CVG->ViewSpaces[GetActiveView()])
				{
				WidgetSetDisabled(IDC_CHECKCURSOR, 0);
				MV = &(ToConfig->Enabled[WCS_VIEWGUI_ENABLE_CURSOR]);
				WidgetSCConfig(IDC_CHECKCURSOR, MV, SCFlag_Char);
				CursorOk = 1;
				if (ActiveCam)
					{
					WidgetSetDisabled(IDC_CHECKVIEWENABLED, 0);
					MV = &(ToConfig->Alive);
					WidgetSCConfig(IDC_CHECKVIEWENABLED, MV, SCFlag_Char);
					EnableOk = 1;
					} // if
				} // if
			} // if
		} // if
	if (!CursorOk)
		{
		WidgetSetDisabled(IDC_CHECKCURSOR, 1);
		} // if
	if (!EnableOk)
		{
		WidgetSetDisabled(IDC_CHECKVIEWENABLED, 1);
		} // if
	WidgetSetDisabled(IDC_LAT, 0);
	WidgetSetDisabled(IDC_LON, 0);
	WidgetSetDisabled(IDC_VIEWARC, 0);
	WidgetSetDisabled(IDC_CHECKFOLLOW, 0);
	WidgetSetDisabled(IDC_SCALE, 0);
	WidgetSetDisabled(IDC_CONTINT, 0);
	WidgetSetDisabled(IDC_EDITMORE, 0);
	WidgetSetDisabled(IDC_EDITMORE3, 0);
	WidgetSetDisabled(IDC_EDITMORE2, Active ? 0: 1);
	WidgetSetDisabled(IDC_CAMLIST, 0);
	WidgetSetDisabled(IDC_OPTLIST, 0);
	} // if
else
	{ // can only show page one if all is not well
	ActivePage = 0;
	ShowPanel(0, 0);
	WidgetSetDisabled(IDC_CHECKVIEWENABLED, 1);
	WidgetSetDisabled(IDC_CHECKCURSOR, 1);
	WidgetTCSetCurSel(IDC_TAB1, 0);
	WidgetSetDisabled(IDC_LAT, 1);
	WidgetSetDisabled(IDC_LON, 1);
	WidgetSetDisabled(IDC_VIEWARC, 1);
	WidgetSetDisabled(IDC_CHECKFOLLOW, 1);
	WidgetSetDisabled(IDC_SCALE, 1);
	WidgetSetDisabled(IDC_CONTINT, 1);
	WidgetSetDisabled(IDC_EDITMORE, 1);
	WidgetSetDisabled(IDC_EDITMORE2, 1);
	WidgetSetDisabled(IDC_EDITMORE3, 1);
	WidgetSetDisabled(IDC_CAMLIST, 1);
	WidgetSetDisabled(IDC_OPTLIST, 1);
	} // else

// Label
switch (ActivePage)
	{
	case 0: break; 		// General
	case 1:
		{
		// Terrain
		WidgetSetText(IDC_PAGELABEL, "Terrain");
		ClearLabels();
		WorkingLine = 1;
		ShowLines(9);
		SetupLine(WorkingLine, "Control Points",				1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_CONTROLPOINTS);
		WorkingLine++;
		SetupLine(WorkingLine, "Reference Grid",				1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_MEASURES);
		WorkingLine++;
		SetupLine(WorkingLine, "Active Object",					1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_ACTIVEOBJ);
		WorkingLine++;
		SetupLine(WorkingLine, "DEM Edges",						1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_DEMEDGE);
		WorkingLine++;
		SetupLine(WorkingLine, "Terrain",						1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_TERRAIN); if (Active) SetupRend(WorkingLine, &Active->TerrainEnabled); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Terrain Transparency",			1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_TERRAIN_TRANS);
		WorkingLine++;
		SetupLine(WorkingLine, "Terrain Polygon Edges",			1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_POLYEDGE);
		WorkingLine++;
		#ifdef WCS_BUILD_VNS
		SetupLine(WorkingLine, "Terraffectors",				1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_TERRAFX); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Area Terraffectors",			1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_AREATERRAFX); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		#else // WCS_BUILD_VNS
		SetupLine(WorkingLine, "Terraffectors",				0, 1); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Area Terraffectors",			0, 1); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		#endif // WCS_BUILD_VNS
		WidgetShow(IDC_EDITMORE3, 1);
		WidgetShow(IDC_CONTINT, 0);
		WidgetShow(IDC_EDITMORE4, 0);
		WidgetShow(IDC_EDITMORE5, 0);
		break;
		} //
	case 2:
		{
		// LandCover & Water
		WidgetSetText(IDC_PAGELABEL, "Land Cover && Water"); 
		ClearLabels();
		SetupLabel(1, "Land Cover");
		SetupLabel(9, "Water");
		ShowLines(12);
		WorkingLine = 1;
		SetupLine(WorkingLine, "Ground Components",			1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_GROUND);
		WorkingLine++;
		SetupLine(WorkingLine, "Other Foliage",				0, 1); if (Active) SetupRend(WorkingLine, &Active->FoliageEnabled); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Foliage Effects",			1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_FOLIAGEFX); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "RT Foliage Images",			1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_RTFOLIAGE);
		WorkingLine++;
		SetupLine(WorkingLine, "Vector Ecosystem",			1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_ECOFX); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "RT Foliage File",			1, 0); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_RTFOLFILE);
		WorkingLine++;
		SetupLine(WorkingLine, "Color Maps",				1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_CMAPS); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Snow",						1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_SNOW); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Lakes",						1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_LAKES); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Streams",					1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_STREAMS); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Wave Models",				1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_WAVES); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Reflections",				0, 1); if (Active) SetupRend(WorkingLine, &Active->ReflectionsEnabled); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		WidgetShow(IDC_EDITMORE3, 0);
		WidgetShow(IDC_CONTINT, 0);
		WidgetShow(IDC_EDITMORE4, 0);
		WidgetShow(IDC_EDITMORE5, 1);
		WidgetSetText(IDC_EDITMORE5, "Prefs");
		break;
		} //
	case 3:
		{
		WidgetSetText(IDC_PAGELABEL, "3D Obj, Vectors && Sky Features");
		ClearLabels();
		SetupLabel(1, "3D Objects");
#ifdef WCS_LABEL
		SetupLabel(4, "Vectors");
		SetupLabel(5, "Sky Features");
		ShowLines(10);
#else // !WCS_LABEL
		SetupLabel(3, "Vectors");
		SetupLabel(4, "Sky Features");
		ShowLines(9);
#endif // !WCS_LABEL
		WorkingLine = 1;
		SetupLine(WorkingLine, "3D Objects",					1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_3DOBJ); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
#ifdef WCS_VIEW_DRAWWALLS
		SetupLine(WorkingLine, "Walls",						1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_WALLS); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE]); else DisableRendCheck(WorkingLine);
#else // !WCS_VIEW_DRAWWALLS
		SetupLine(WorkingLine, "Walls",						0, 1); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE]); else DisableRendCheck(WorkingLine);
#endif // !WCS_VIEW_DRAWWALLS
		WorkingLine++;

#ifdef WCS_LABEL
		SetupLine(WorkingLine, "Labels",					0, 1); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
#endif // WCS_LABEL


		SetupLine(WorkingLine, "Plain Vectors",				1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_PLAINVEC); if (Active) SetupRend(WorkingLine, &Active->VectorsEnabled); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Sky",							1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_SKY); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Atmosphere",					1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_ATMOS); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Cloud Models",				1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_CLOUDS); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Volumetrics",				0, 1); if (Active) SetupRend(WorkingLine, &Active->VolumetricsEnabled); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Celestial Objects",			1, 1); SetupReal(WorkingLine, WCS_VIEWGUI_ENABLE_CELEST); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		SetupLine(WorkingLine, "Starfields",					0, 1); if (Active) SetupRend(WorkingLine, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD]); else DisableRendCheck(WorkingLine);
		WorkingLine++;
		WidgetShow(IDC_EDITMORE3, 0);
		WidgetShow(IDC_CONTINT, 0);
		WidgetShow(IDC_EDITMORE4, 0);
		WidgetShow(IDC_EDITMORE5, 0);
		break;
		} //
	case 4:
		{
		// Light & Render
		WidgetSetText(IDC_PAGELABEL, "Light/Shadow && Camera");
		ClearLabels();
		SetupLabel(1, "Light/Shadow");
		SetupLabel(5, "Camera");
		ShowLines(12);
		SetupLine(1, "Lights",						1, 1); SetupReal(1, WCS_VIEWGUI_ENABLE_LIGHTS); if (Active) SetupRend(1, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT]); else DisableRendCheck(1);
		SetupLine(2, "Cloud Shadows",				0, 1); if (Active) SetupRend(2, &Active->CloudShadowsEnabled); else DisableRendCheck(2);
		SetupLine(3, "Terrain Shadows",				1, 1); SetupReal(3, WCS_VIEWGUI_ENABLE_TERRAINSHADOWS); if (Active) SetupRend(3, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW]); else DisableRendCheck(3);
		SetupLine(4, "3D Object Shadows",			0, 1); if (Active) SetupRend(4, &Active->ObjectShadowsEnabled); else DisableRendCheck(4);
		SetupLine(5, "Cameras",						1, 0); SetupReal(5, WCS_VIEWGUI_ENABLE_CAMERAS);
		SetupLine(6, "Targets",						1, 0); SetupReal(6, WCS_VIEWGUI_ENABLE_TARGETS);
		SetupLine(7,"Image Boundaries",				1, 0); SetupReal(7, WCS_VIEWGUI_ENABLE_SAFEAREA);

		SetupLine(8,"Safe Title",				1, 0); SetupReal(8, WCS_VIEWGUI_ENABLE_SAFETITLE);
		SetupLine(9,"Safe Action",				1, 0); SetupReal(9, WCS_VIEWGUI_ENABLE_SAFEACTION);

		SetupLine(10, "Depth-of-Field",				0, 1); if (Active) SetupRend(10, &Active->DepthOfFieldEnabled); else DisableRendCheck(10);
		SetupLine(11, "Multipass AA/MotionBlur",		0, 1); if (Active) SetupRend(11, &Active->MultiPassAAEnabled); else DisableRendCheck(11);
		SetupLine(12, "Render Diagnostic Data",		0, 1); if (Active) SetupRend(12, &Active->RenderDiagnosticData); else DisableRendCheck(12);
//		case ID_WINMENU_OPTIONS_SAFETITLE: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SAFETITLE); break;
//		case ID_WINMENU_OPTIONS_SAFEACTION: ViewOrigin->ToggleEnabled(WCS_VIEWGUI_ENABLE_SAFEACTION); break;
		WidgetShow(IDC_EDITMORE3, 0);
		WidgetShow(IDC_CONTINT, 0);
		WidgetShow(IDC_EDITMORE4, 0);
		WidgetShow(IDC_EDITMORE5, 0);
		break;
		} // 
	case 5:
		{
		WidgetSetText(IDC_PAGELABEL, "Overlay && Gradient");
		ClearLabels();
		SetupLabel(1, "Overlay");
		SetupLabel(6, "Gradient");
		ShowLines(9);
		SetupLine(1, "Contours",				1, 0); SetupReal(1, WCS_VIEWGUI_ENABLE_OVER_CONTOURS);
		SetupLine(2, "Slope Map",				1, 0); SetupReal(2, WCS_VIEWGUI_ENABLE_OVER_SLOPE);
		SetupLine(3, "Relative Elevation Map",		1, 0); SetupReal(3, WCS_VIEWGUI_ENABLE_OVER_RELEL);
		SetupLine(4, "Fractal Map",		1, 0); SetupReal(4, WCS_VIEWGUI_ENABLE_OVER_FRACTAL);
		SetupLine(5, "Ecosystem Map",			1, 0); SetupReal(5, WCS_VIEWGUI_ENABLE_OVER_ECOSYS);
		SetupLine(6, "Grey Elevation",		1, 0); SetupReal(6, WCS_VIEWGUI_ENABLE_GRAD_GREY);
		SetupLine(7, "Earth Elevation",		1, 0); SetupReal(7, WCS_VIEWGUI_ENABLE_GRAD_EARTH);
		SetupLine(8, "Rainbow Elevation",		1, 0); SetupReal(8, WCS_VIEWGUI_ENABLE_GRAD_PRIMARY);
		SetupLine(9,"Elev Gradient Repeat",		1, 0); SetupReal(9, WCS_VIEWGUI_ENABLE_GRADREPEAT);
		WidgetShow(IDC_EDITMORE3, 0);
		WidgetShow(IDC_CONTINT, 1);
		WidgetShow(IDC_EDITMORE4, 1);
		WidgetShow(IDC_EDITMORE5, 0);
		WidgetSetText(IDC_EDITMORE4, "Regen");
		break;
		} //
	} // switch

} // ViewPrefsGUI::ConfigureWidgets()

/*===========================================================================*/

void ViewPrefsGUI::DoView(int ViewNum)
{

SetActiveView(ViewNum);
ConfigureWidgets();

} // ViewPrefsGUI::DoView

/*===========================================================================*/
/*===========================================================================*/

// ViewRTPrefs

// currently code stolen from ViewPrefsGUI

NativeGUIWin ViewRTPrefsGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	SECURITY_INLINE_CHECK(054, 54);
	WidgetTCSetCurSel(IDC_TAB1, ActivePage);
	ShowPanel(0, ActivePage == 0 ? 0 : 1);
	ConfigureWidgets();
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ViewRTPrefsGUI::Open

/*===========================================================================*/

NativeGUIWin ViewRTPrefsGUI::Construct(void)
{
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DISPPREFS, GlobalApp->WinSys->RootWin);
	CreateSubWinFromTemplate(IDD_DISPPREFS_RTSHOW, 0, 0);
	CreateSubWinFromTemplate(IDD_DISPPREFS_RTWRITE, 0, 1);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < 2; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		} // if
	} // if
 
return(NativeWin);

} // ViewRTPrefsGUI::Construct

/*===========================================================================*/

ViewRTPrefsGUI::ViewRTPrefsGUI()
: GUIFenetre('VRTP', this, "View Realtime Preferences") // Yes, I know...
{

ConstructError = 0;
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // ViewRTPrefsGUI::ViewRTPrefsGUI

/*===========================================================================*/

ViewRTPrefsGUI::~ViewRTPrefsGUI()
{

// prefs can be closed by calling destructor if a view is closed 
// so need to remove from menu list here too.
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ViewRTPrefsGUI::~ViewRTPrefsGUI()

/*===========================================================================*/

long ViewRTPrefsGUI::HandleCloseWin(NativeGUIWin NW)
{

// no need to destroy window while ViewGUI is still around.
GlobalApp->MCP->RemoveWindowFromMenuList(this);
Close();
return(0);

} // ViewRTPrefsGUI::HandleCloseWin

/*===========================================================================*/

long ViewRTPrefsGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

// No basic buttons right now
return(0);

} // ViewRTPrefsGUI::HandleButtonClick

/*===========================================================================*/

void ViewRTPrefsGUI::HandleNotifyEvent(void)
{

// No basic events right now

} // ViewRTPrefsGUI::HandleNotifyEvent()

/*===========================================================================*/

long ViewRTPrefsGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RTDISP_MINHT:
	case IDC_RTDISP_MAXHT:
	case IDC_RTDISP_NEAR:
	case IDC_RTDISP_FAR:
		{
		// redraw all views
		GlobalApp->GUIWins->CVG->Draw(-1);
		} // View-reactive floatints
	} // switch
return(0);

} // ViewRTPrefsGUI::HandleFIChange

/*===========================================================================*/

long ViewRTPrefsGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// No comboboxes right now
return(0);

} // ViewRTPrefsGUI::HandleCBChange

/*===========================================================================*/

long ViewRTPrefsGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RTDISP_3DOBJECTS:
	case IDC_RTDISP_IMAGES:
		{
		// redraw all views
		GlobalApp->GUIWins->CVG->Draw(-1);
		} // View-reactive checkboxes
	} // switch
return(0);

} // ViewRTPrefsGUI::HandleSCChange

/*===========================================================================*/

long ViewRTPrefsGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 0:
				{
				ShowPanel(0, 0);
				break;
				} // 1
			case 1:
				{
				ShowPanel(0, 1);
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

ActivePage = NewPageID;
ConfigureWidgets();
return(0);

} // ViewRTPrefsGUI::HandlePageChange

/*===========================================================================*/

long ViewRTPrefsGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
ViewGUI *VG;
char NewName[WCS_EFFECT_MAXNAMELENGTH];

if (!NativeWin) return(0);
if (!(VG = GlobalApp->GUIWins->CVG)) return(0);

switch (CtrlID)
	{
	case IDC_RTDISP_INFILE:
		{
		if (WidgetGetModified(IDC_RTDISP_INFILE))
			{
			WidgetGetText(IDC_RTDISP_INFILE, WCS_EFFECT_MAXNAMELENGTH, NewName);
			WidgetSetModified(IDC_RTDISP_INFILE, FALSE);
			strncpy((char *)&VG->RTFDispConf.BaseName, NewName, WCS_FOLIAGELIST_BASENAME_LEN - 1);
			VG->RTFDispConf.BaseName[WCS_FOLIAGELIST_BASENAME_LEN - 1] = NULL;
			// Reload new file.
			if (RTFLoaded)
				{
				VG->FreeRealtimeFoliage();
				RTFLoaded = 0;
				VG->PrepRealtimeFoliage(VG->RTFDispConf.BaseName);
				VG->Draw(-1);
				} // if
			} // if 
		break;
		} // 
	case IDC_RTWRITE_OUTFILE:
		{
		if (WidgetGetModified(IDC_RTWRITE_OUTFILE))
			{
			WidgetGetText(IDC_RTWRITE_OUTFILE, WCS_EFFECT_MAXNAMELENGTH, NewName);
			WidgetSetModified(IDC_RTWRITE_OUTFILE, FALSE);
			strncpy((char *)&VG->RTFWriteConf.BaseName, NewName, WCS_FOLIAGELIST_BASENAME_LEN - 1);
			VG->RTFWriteConf.BaseName[WCS_FOLIAGELIST_BASENAME_LEN - 1] = NULL;
			} // if 
		break;
		} // 
	} // switch CtrlID

return (0);

} // ViewRTPrefsGUI::HandleStringLoseFocus

/*===========================================================================*/

void ViewRTPrefsGUI::ConfigureWidgets(void)
{
ViewGUI *VG;

if (!NativeWin) return;
if (!(VG = GlobalApp->GUIWins->CVG)) return;

// RTFDispConf
WidgetSNConfig(IDC_RTDISP_MINHT, &VG->RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT]);
WidgetSNConfig(IDC_RTDISP_MAXHT, &VG->RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT]);
WidgetSNConfig(IDC_RTDISP_NEAR, &VG->RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_NEARDIST]);
WidgetSNConfig(IDC_RTDISP_FAR, &VG->RTFDispConf.ConfigParams[WCS_REALTIME_CONFIG_FARDIST]);

ConfigureSC(NativeWin, IDC_RTDISP_3DOBJECTS, &VG->RTFDispConf.Display3DO, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_RTDISP_IMAGES, &VG->RTFDispConf.DisplayImage, SCFlag_Char, NULL, NULL);

WidgetSetModified(IDC_RTDISP_INFILE, FALSE);
WidgetSetText(IDC_RTDISP_INFILE, (char *)&VG->RTFDispConf.BaseName);

// RTFWriteConf
WidgetSNConfig(IDC_RTWRITE_MINHT, &VG->RTFWriteConf.ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT]);
WidgetSNConfig(IDC_RTWRITE_MAXHT, &VG->RTFWriteConf.ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT]);
WidgetSNConfig(IDC_RTWRITE_NEAR, &VG->RTFWriteConf.ConfigParams[WCS_REALTIME_CONFIG_NEARDIST]);
WidgetSNConfig(IDC_RTWRITE_FAR, &VG->RTFWriteConf.ConfigParams[WCS_REALTIME_CONFIG_FARDIST]);

ConfigureSC(NativeWin, IDC_RTWRITE_3DOBJECTS, &VG->RTFWriteConf.Include3DO, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_RTWRITE_IMAGES, &VG->RTFWriteConf.IncludeImage, SCFlag_Char, NULL, NULL);

WidgetSetModified(IDC_RTWRITE_OUTFILE, FALSE);
WidgetSetText(IDC_RTWRITE_OUTFILE, (char *)&VG->RTFWriteConf.BaseName);

ConfigureFI(NativeWin, IDC_RTWRITE_MAXSTEMS, &VG->RTFWriteConf.StemsPerCell, 1.0, 1.0, 1000000.0, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_RTWRITE_NUMFILES, &VG->RTFWriteConf.NumFiles, 1.0, 1.0, 1000.0, FIOFlag_Long, NULL, NULL);

} // ViewRTPrefsGUI::ConfigureWidgets()

/*===========================================================================*/
/*===========================================================================*/

// from Joe.cpp -- these deal with Joes in Views

signed long Joe::HowManyPolys(void)
{
signed long Polys = 0;
JoeViewTemp *JVT;
if (TestFlags(WCS_JOEFLAG_ISDEM))
	{
	if (JVT = (JoeViewTemp *)MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP))
		{
		if (JVT->MaxPolys > 0)
			{
			Polys = JVT->MaxPolys;
			} // if
		} // if
	} // if
return(Polys);

} // Joe::HowManyPolys

/*===========================================================================*/

void JoeViewTemp::InitClear(void)
{
NextAttrib = NULL;
MajorAttribType = WCS_JOE_ATTRIB_INTERNAL;
MinorAttribType = WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP;
AttribClassSize = sizeof(class JoeViewTemp);
FileSize = 0;
MyNextBGRedrawAction = 0;

TexMapWidth = TexMapHeight = 0;
SlopeTex = RelelTex = FractTex = EcoTex = NULL;

MaxPolys = 0;
PerspDisplayListNum = 0;
PlanDisplayListNum = 0;
#ifdef WCS_BUILD_VNS
ProjPlanDisplayListNum = 0;
#endif // WCS_BUILD_VNS
}; // JoeViewTemp::InitClear

/*===========================================================================*/

void JoeViewTemp::DeleteTexMaps(void)
{

DeleteTexMap(WCS_VIEWGUI_ENABLE_OVER_SLOPE);
DeleteTexMap(WCS_VIEWGUI_ENABLE_OVER_RELEL);
DeleteTexMap(WCS_VIEWGUI_ENABLE_OVER_FRACTAL);
DeleteTexMap(WCS_VIEWGUI_ENABLE_OVER_ECOSYS);

} // JoeViewTemp::DeleteTexMaps

/*===========================================================================*/

void JoeViewTemp::DeleteTexMap(int WhichMap)
{

if (SlopeTex && (WhichMap == WCS_VIEWGUI_ENABLE_OVER_SLOPE))
	{
	AppMem_Free(SlopeTex, TexMapWidth * TexMapHeight);
	SlopeTex = NULL;
	} // if
if (RelelTex && (WhichMap == WCS_VIEWGUI_ENABLE_OVER_RELEL))
	{
	AppMem_Free(RelelTex, TexMapWidth * TexMapHeight);
	RelelTex = NULL;
	} // if
if (FractTex && (WhichMap == WCS_VIEWGUI_ENABLE_OVER_FRACTAL))
	{
	AppMem_Free(FractTex, TexMapWidth * TexMapHeight);
	FractTex = NULL;
	} // if
if (EcoTex   && (WhichMap == WCS_VIEWGUI_ENABLE_OVER_ECOSYS))
	{
	AppMem_Free(EcoTex,   3 * TexMapWidth * TexMapHeight);
	EcoTex = NULL;
	} // if

} // JoeViewTemp::DeleteTexMap

/*===========================================================================*/

void ViewGUI::ProcessMotionControl(void)
{
float JX, JY /*, JZ */;
unsigned long JB;
int AV;
long XDif, YDif;
char OldVME, OldVMM;
ViewContext *OriVC = NULL;
GLDrawingFenetre *GLDF;
//unsigned long ShiftCode = 0;

if (!GetMotionControl()) return;

// identify which view it came from
if ((AV = IdentActiveView()) == -1) return;  // no active view window

GLDF = ViewWins[AV];
OriVC = ViewSpaces[AV];

if (!OriVC || !GLDF) return;

JX = GlobalApp->WinSys->CheckInputControllerX(1) - JOX;
JY = GlobalApp->WinSys->CheckInputControllerY(1) - JOY;
//JZ = GlobalApp->WinSys->CheckInputControllerZ(1) - JOZ;
JB = GlobalApp->WinSys->CheckInputControllerButtons(1, 0xffffffff);
//CLock = GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CAPSLOCK);

// clamp values to +- WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG
JX = min(JX, WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);
JX = max(JX, -WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);

JY = min(JY, WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);
JY = max(JY, -WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);

// Normalize to +-[0...1]
JX = (JX / WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);
JY = (JY / WCS_VIEW_JOYSTICK_FULL_RANGE_ANALOG);

// Square to use exponential sensitivity
if (JX < 0)
	{
	JX = -(JX * JX);
	} // if
else
	{
	JX = JX * JX;
	} // else
if (JY < 0)
	{
	JY = -(JY * JY);
	} // if
else
	{
	JY = JY * JY;
	} // else

XDif = (long)(JX * WCS_VIEW_JOYSTICK_FULL_RANGE_PIXELS);
YDif = (long)(JY * WCS_VIEW_JOYSTICK_FULL_RANGE_PIXELS);

//sprintf(JoyMsg, "X:%+08.2f Y:%+08.2f Z:%+08.2f B:%x", JX, JY, JZ, JB);
//GlobalApp->MCP->SetCurrentStatusText(JoyMsg);
/*
if (MCBW)
	{
	if (MCBW->CheckAbort())
		{
		SetMotionControl(0); // stop motioncontrol
		PrepMotionControl();
		EndInteractive();
		} // if
	} // if
*/
if (JB & 0x01)
	{
	SetMotionControl(0); // stop motioncontrol
	//GlobalApp->MCP->SetCurrentStatusText("Joystick Control Off.");
	PrepMotionControl();
	EndInteractive();
	} // if

if (GetMotionControlMode())
	{
	if (JB & 0x02)
		{ // move mode
		if (GetViewManipulationEnable())
			{
			// only switch if we're not already in MOVE
			if (GetViewManipulationMode() != WCS_VIEWGUI_MANIP_MOVE)
				{
				SetViewManipulationMode(WCS_VIEWGUI_MANIP_MOVE);
				} // if
			} // if
		else
			{
			// only switch if we're not already in MOVE
			if (GetObjectManipulationMode() != WCS_VIEWGUI_MANIP_MOVE)
				{
				SetObjectManipulationMode(WCS_VIEWGUI_MANIP_MOVE);
				} // if
			} // else
		} // if
	else
		{ // rotate mode
		if (GetViewManipulationEnable())
			{
			// only switch if we're not already in MOVE
			if (GetViewManipulationMode() != WCS_VIEWGUI_MANIP_ROT)
				{
				SetViewManipulationMode(WCS_VIEWGUI_MANIP_ROT);
				} // if
			} // if
		else
			{
			// only switch if we're not already in MOVE
			if (GetObjectManipulationMode() != WCS_VIEWGUI_MANIP_ROT)
				{
				SetObjectManipulationMode(WCS_VIEWGUI_MANIP_ROT);
				} // if
			} // else
		} // else
	} // if

//	if (Shift) ShiftCode = WCS_DOINTER_CODE_SHIFT;
//	if (Alt) return(0);
if (OriVC && OriVC->VCamera && OriVC->VCamera->CamPos)
	{
	// Can only handle one motion-axis set
	if (GetMotionControlMode())
		{
		if ((XDif != 0) || (YDif != 0))
			{
			if ((GetViewManipulationEnable() && GetViewManipulationMode()) || (!GetViewManipulationEnable() && GetObjectManipulationMode()))
				{
				if (GetViewManipulationEnable() && (GetViewManipulationMode() == WCS_VIEWGUI_MANIP_ROT))
					{
					DoInteractive(OriVC, GLDF, XDif, -YDif, 0, 0, 0);
					} // if
				else
					{
					DoInteractive(OriVC, GLDF, XDif, YDif, 0, 0, 0);
					} // else
				} // if
			} // if
		} // if
	else
		{ // move like airplane, F/B = move, L/R = rotate
		if ((XDif != 0) || (YDif != 0)) // Watch those args carefully, they're not what they seem.
			{
			// we're allowed to quietly futz with ViewManipulationEnable/ViewManipulationMode
			// because we're ViewGUI, we know what we're doing, and we put them right back
			// before the rest of the universe notices. Very quantum. Plus it's easier than
			// doing it the other (hard) way.
			OldVME = GetViewManipulationEnable();
			OldVMM = GetViewManipulationMode();
			ViewManipulationEnable = 1;

			// Do this atomically with no redraw between them
			SetInterLock(1);

			if (YDif != 0)
				{
				ViewManipulationMode = WCS_VIEWGUI_MANIP_MOVE;
				DoInteractive(OriVC, GLDF, 0, YDif, 0, 0, 0);
				} // if
			if (XDif != 0)
				{
				ViewManipulationMode = WCS_VIEWGUI_MANIP_ROT;
				DoInteractive(OriVC, GLDF, XDif, 0, 0, 0, 0);
				} // if

			SetInterLock(0);

			ViewManipulationEnable = OldVME;
			ViewManipulationMode   = OldVMM;

			DrawImmediately();
			} // if
		} // else
	} // if

} // ViewGUI::ProcessMotionControl

/*===========================================================================*/

int ViewGUI::BuildRealtimeFoliage(int ViewNum)
{
int NumStems = 0;

if (ViewNum != -1)
	{
	DoRender(ViewNum, 1); // WithRTF = 1
	} // if

return(NumStems);

} // ViewGUI::BuildRealtimeFoliage

/*===========================================================================*/

unsigned long ViewGUI::QueryViewWindowState(long ViewNum)
{
unsigned long Result = 0;

if (ViewNum != -1)
	{
	// NULL pointer error checking handled in other signature of this method, below
	return(QueryViewWindowState(ViewSpaces[ViewNum]));
	} // if

return(Result);

} // ViewGUI::QueryViewWindowState

/*===========================================================================*/

unsigned long ViewGUI::QueryViewWindowState(ViewContext *VC)
{
unsigned long Result = 0;

if (VC != NULL)
	{
	Result = VC->QueryViewState();
	} // if

return(Result);

} // ViewGUI::QueryViewWindowState

/*===========================================================================*/

unsigned long ViewContext::QueryViewState(void)
{
unsigned long Result = 0;

Result = ViewState;

if (RO && VCamera)
	{
	if (GetEnabled(WCS_VIEWGUI_ENABLE_LTDREGION)) Result |= WCS_VIEWGUI_VIEWSTATE_CONSTRAIN;
	if ((GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV)) && BigR) Result |= WCS_VIEWGUI_VIEWSTATE_RENDERPREV;
	if (GlobalApp->GUIWins->CVG->ZoomInProgress) Result |= WCS_VIEWGUI_VIEWSTATE_ZOOMBOX; // <<<>>> needs to be per-View, not global or they'll all pop in
	} // if

return(Result);
} // ViewContext::QueryViewState

/*===========================================================================*/

//lint -restore
