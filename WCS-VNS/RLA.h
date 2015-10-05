// RLA.h
// Kinetix file support
// 05/21/99 FPW2 adapted from 3DS RLA code & headers
// Hacked off unused stuff 12/04/00 FPW2
// Copyright 1999

#include "stdafx.h"

#ifndef WCS_RLA_H
#define WCS_RLA_H

struct RLASWindow {
	USHORT left;
	USHORT right;
	USHORT bottom;
	USHORT top;
	};

const unsigned short RLA_MAGIC_OLD = 0xFFFE;
const unsigned short RLA_MAGIC  = 0xFFFD; // started using this with R3.1 
const int RLA_Y_PAGE_SIZE = 32;

// I think most/all of these should be unsigned
struct RLAHeader {
	RLASWindow window;
	RLASWindow active_window;
	short			frame;
	short			storage_type;
	short			num_chan;
	short			num_matte;
	short			num_aux;
	unsigned short	revision;
	char			gamma[16];
	char			red_pri[24];
	char			green_pri[24];
	char			blue_pri[24];
	char			white_pt[24];
	long			job_num;
	char			name[128];
	char			desc[128];
	char			program[64];
	char			machine[32];
	char			user[32];
	char			date[20];
	char			aspect[24];
	char			aspect_ratio[8];
	char			chan[32];
	short			field;
	char			time[12];
	char			filter[32];
	short			chan_bits;
	short			matte_type;
	short			matte_bits;
	short			aux_type;
	short			aux_bits;
	char			aux[32];
	char			space[36];
	long			next;
	};

/*=========================== GBuffer defines ===============================*/

#define NUMGBCHAN 14

// GBuffer channels (number of bytes in parenthesis)
#define GB_Z       			0  	// (4)  Z-Buffer depth, float
#define GB_MTL_ID  			1  	// (1)  ID assigned to mtl via mtl editor
#define GB_NODE_ID 			2  	// (2)  ID assigned to node via properties
#define GB_UV       		3 	// (8)  UV coordinates - Point2 
#define GB_NORMAL   		4 	// (4)  Normal vector in view space, compressed 
#define GB_REALPIX  		5 	// (4)  Non clamped colors in "RealPixel" format 
#define GB_COVERAGE 		6 	// (1)  Pixel coverage  
#define GB_BG 	     		7 	// (3)  RGB color of what's behind layer 
#define GB_NODE_RENDER_ID 	8 	// (2)  Node render index, word
#define GB_COLOR		 	9 	// (3)  Color (RGB)
#define GB_TRANSP		 	10 	// (3)  Transparency (RGB)
#define GB_VELOC		 	11 	// (8)  Velocity (Point2)
#define GB_WEIGHT		 	12 	// (3)  Weight of layers contribution to pixel color
#define GB_MASK			 	13 	// (2)  Sub pixal coverage mask

//CoreExport TCHAR *GBChannelName(int i);

// Recognized channel bits 

#define BMM_CHAN_NONE     0
#define BMM_CHAN_Z        (1<<GB_Z) 		//  Z-buffer depth, float 
#define BMM_CHAN_MTL_ID   (1<<GB_MTL_ID) 	//  ID assigned to mtl via mtl editor 
#define BMM_CHAN_NODE_ID  (1<<GB_NODE_ID) 	//  ID assigned to node via properties 
#define BMM_CHAN_UV       (1<<GB_UV) 		//  UV coordinates - Point2 
#define BMM_CHAN_NORMAL   (1<<GB_NORMAL) 	//  Normal vector in view space, compressed 
#define BMM_CHAN_REALPIX  (1<<GB_REALPIX) 	//  Non clamped colors in "RealPixel" format 
#define BMM_CHAN_COVERAGE (1<<GB_COVERAGE) 	//  Pixel coverage of front surface 
#define BMM_CHAN_BG 	  (1<<GB_BG) 		//  RGB color of what's behind front object 
#define BMM_CHAN_NODE_RENDER_ID (1<<GB_NODE_RENDER_ID) //  node render index 
#define BMM_CHAN_COLOR    (1<<GB_COLOR) 	//  Color (Color24) 
#define BMM_CHAN_TRANSP   (1<<GB_TRANSP) 	//  Transparency (Color24) 
#define BMM_CHAN_VELOC    (1<<GB_VELOC) 	//  Velocity ( Point2 ) 
#define BMM_CHAN_WEIGHT   (1<<GB_WEIGHT) 	//  Weight ( Color24 ) 
#define BMM_CHAN_MASK	  (1<<GB_MASK)   	//  Subpixel mask ( word ) 

// Recognized types of channels
#define BMM_CHAN_TYPE_UNKNOWN 0 
#define BMM_CHAN_TYPE_8   2 // 1 byte per pixel
#define BMM_CHAN_TYPE_16  3 // 1 word per pixel
#define BMM_CHAN_TYPE_24  8 // 3 bytes per pixel
#define BMM_CHAN_TYPE_32  4 // 2 words per pixel
#define BMM_CHAN_TYPE_48  5 // 3 words per pixel
#define BMM_CHAN_TYPE_64  6 // 4 words per pixel
#define BMM_CHAN_TYPE_96  7 // 6 words per pixel

/***
struct GBufData {
	float z;
	UBYTE mtl_id;
	UWORD node_id;
	Point2 uv;
	DWORD normal;
	RealPixel realpix;
	UBYTE coverage;
	UWORD rend_id;
	Color24 color;
	Color24 transp;
	Color24 weight;
	Point2 veloc;
	UWORD mask;
	};
***/

struct RLAUSERDATA {
	DWORD	version;		//-- Reserved
	DWORD	channels;		//-- Bitmap with channels to save
	BOOL	usealpha;		//-- Save Alpha (if present)
	BOOL 	rgb16;			//-- Use 16 bit channels (or 8 bits if FALSE)
	BOOL 	defaultcfg;		//-- Reserved
	char 	desc[128];		//-- Description (ASCII)
	char 	user[32];		//-- User Name (ASCII)
}; 

int rla_encode(unsigned char *input, unsigned char *output, int xSize, int stride);
BYTE* rla_decode(BYTE *input, BYTE *output, int xFile, int xImage, int stride);
void SwapRLAHdrBytes(RLAHeader& h);
void MakeRLAProgString(char *s, ULONG chan, BOOL rendInfo);

typedef struct {
   unsigned short r,g,b,a;
} Max_Color_64;

ULONG RLAChannelsFromString(char *s, BOOL &gotRendInfo, BOOL &gotLayerData, BOOL &gotNodeNames);
int MaxGBSize(ULONG gbChannels);

class RLAReader {
	FILE* fd;
	long *offsets;
	BYTE *inp,*curInp;
	BYTE *out;
	int width, height;
	int maxLength, inited;
	RLAHeader* hdr;
	public:
		RLAReader(RLAHeader *hd,FILE *f, int w, int h);
		int Init();
//		int ReadRendInfo( RenderInfo *ri );
//		int ReadNameTab(NameTab &nt);
		int BeginLine( int y );
		int ReadNextChannel( BYTE *out, int stride,int w = -1, BOOL longLength=0 );
		int ReadLowByte( BYTE *out, int stride );
		int ReadWordChannel(WORD *ptr, int stride, BOOL twoBytes);
		int ReadNChannels( BYTE *out, int n, int w=-1, BOOL longLength=0);
		int ReadRGBA(Max_Color_64 *pix);
		int ReadNumLRecs();
		~RLAReader();
	};

class FragmentLoadTransfer
	{
	public:
		FragmentLoadTransfer();
		~FragmentLoadTransfer();

		// BMM_CHAN_Z
		float FragZ;
		unsigned char FragZValid;

		// BMM_CHAN_MTL_ID
		unsigned char FragMatID;
		unsigned char FragMatIDValid;

		// BMM_CHAN_NODE_ID
		unsigned short FragNodeID;
		unsigned char FragNodeIDValid;

		// BMM_CHAN_UV
		float FragU, FragV;
		unsigned char FragUVValid;

		// BMM_CHAN_NORMAL
		float FragNormal[3]; // uncompressed
		unsigned char FragNormalValid;

		// BMM_CHAN_REALPIX
		float FragR, FragG, FragB; // uncompressed into float guns
		unsigned char FragRGBValid;

		// BMM_CHAN_COVERAGE
		unsigned char Coverage8; // expressed as 0...255
		unsigned char Coverage8Valid;

		// BMM_CHAN_BG
		unsigned char FragBGR, FragBGG, FragBGB;
		unsigned char FragBGRGBValid;

		// BMM_CHAN_NODE_RENDER_ID
		unsigned short FragRenderID;
		unsigned char FragRenderIDValid;

		// BMM_CHAN_COLOR
		unsigned char FragColorR, FragColorG, FragColorB;
		unsigned char FragColorRGBValid;

		// BMM_CHAN_TRANSP
		unsigned char FragTransR, FragTransG, FragTransB;
		unsigned char FragTransRGBValid;

		// BMM_CHAN_VELOC
		float VolcX, VelocY;
		unsigned char VelocValid;

		// BMM_CHAN_WEIGHT
		unsigned char FragWeightR, FragWeightG, FragWeightB;
		unsigned char FragWeightRGBValid;

		// BMM_CHAN_MASK
		unsigned short FourByFourMask;
		unsigned char FourByFourMaskValid;

	}; // FragmentLoadTransfer

/*** Notes from Max - these can all be stored in the fragment list:

Below is an overview of the image channels.  The number of bits per pixel occupied by the channel is listed.  The way the channel is accessed and
cast to the appropriate data type is also shown.  Note: 3ds max users may store the G-Buffer data in offline storage in RLA or RPF files.  For the
definition of the RLA or RPF format you can look at the source code in \MAXSDK\SAMPLES\IO\RLA\RLA.CPP.  Also Note: The term 'fragment' is used in
the descriptions of the channels below. A 'fragment' is the portion of a triangle of a mesh that's seen by a particular pixel being rendered.  It's
as if the pixel was a cookie-cutter and chopped a visible section of the triangle off for rendering -- that cut piece is called a fragment. 

BMM_CHAN_Z

Z-buffer, stored as a float. The size is 32 bits per pixel.  This is the channel that would be used by a depth of field blur routine for instance.
The Z value is at the center of the fragment that is foremost in the sorted list of a-buffer fragments.  The Z buffer is an array of float values
giving the Z-coordinate in camera space of the point where a ray from the camera through the pixel center first intersects a surface.  All Z values
are negative, with more negative numbers representing points that are farther from the camera.  The Z buffer is initialized with the value -1.0E30.
Note that this is a change over 3ds max 1.x where the Z buffer was previously initialized with 1.0E30.  The negative value is more appropriate since
more negative values represent points farther from the camera.

Note that for non-camera viewports (such as Front, User, Grid, Shape, etc.) the values may be both positive and negative.  In such cases the developer
may as well add a large value onto all the values to make them all positive.  This is because positive versus negative doesn't really mean anything.
It is just the distance between values that matters.  As noted above, the Z values in the A buffer are in camera space.  The projection for a point
in camera space to a point in screen space is:

Point2 RenderInfo::MapCamToScreen(Point3 p) {

return (projType==ProjPerspective)?

        Point2(xc + kx*p.x/p.z, yc + ky*p.y/p.z):
        Point2(xc + kx*p.x, yc + ky*p.y);
}

This function is supplied by the RenderInfo data structure which can be obtained from the bitmap output by the renderer using the function
Bitmap::GetRenderInfo().  Note that this outputs a Point2.  There is no projection for Z.  As noted before, the Z buffer just uses the camera space Z.

float *zbuffer = (float *)GetChannel(BMM_CHAN_Z,type);

BMM_CHAN_MTL_ID

The ID assigned to the material via the Material Editor. The size is 8 bits per pixel.  This channel is currently settable to a value between 0 and 8
by the 'Material Effects Channel' flyoff in the Material Editor.  A plug-in material can generated up to 255 different material ID's (since this is
an 8-bit quantity).  This channel would be used to apply an effect (i.e., a glow) to a specific material.

BYTE *bbuffer = (BYTE *)GetChannel(BMM_CHAN_MTL_ID,type);

BMM_CHAN_NODE_ID

This is the ID assigned to node via the Object Properties / G-buffer ID spinner. The size is 16 bits per pixel.  This channel would be used to
perform an effect (for example a flare) on a specific node.

WORD *wbuffer = (WORD *)GetChannel(BMM_CHAN_NODE_ID,type); 

BMM_CHAN_UV

UV coordinates, stored as a Point2. The size is 64 bits per pixel.  If you have UV Coordinates on your object this channel provides access to them.
This channel could be used by 3D paint programs or image processing routines to affect objects based on their UVs.  The UV coordinate is stored as
a Point2, using Point2::x for u and Point2::y for v.    The UV coordinates are values prior to applying the offset, tiling, and rotation associated
with specific texture maps.

Point2 *pbuffer = (Point2 *)GetChannel(BMM_CHAN_UV,type);

BMM_CHAN_NORMAL

Normal vector in view space, compressed. The size is 32 bits per pixel.  Object normals are available for image processing routines that take
advantage of the normal vectors to do effects based on curvature (for example), as well as for 3D paint programs. The normal value is at the center
of the fragment that is foremost in the sorted list of a-buffer fragments.

DWORD *dbuffer = (DWORD *)GetChannel(BMM_CHAN_NORMAL,type);

Note:  The following function is available to decompress this value to a standard normalized Point3 value (DWORD and ULONG are both 32 bit quantities):

Point3 DeCompressNormal(ULONG n);

The decompressed vector has absolute error < 0.001 in each component.

BMM_CHAN_REALPIX

Non clamped colors in "RealPixel" format. The size is 32 bits per pixel.  See Structure RealPixel.  These are 'real' colors that are available for
physically-correct image processing routines to provide optical effects that duplicate the way the retina works.

RealPixel *rbuffer =
	(RealPixel *)GetChannel(BMM_CHAN_REALPIX,type);

BMM_CHAN_COVERAGE
Pixel coverage of the front surface.  This provides an 8-bit value (0..255) that gives the coverage of the surface fragment from which the other
G-buffer values are obtained.  This channel is being written and read with RLA files, and shows up in the Virtual Frame Buffer.  This may be used
to make the antialiasing in 2.5D plug-ins such as Depth Of Field filters much better.

UBYTE *gbufCov = (UBYTE*)GetChannel(BMM_CHAN_COVERAGE,type); 

BMM_CHAN_BG
The RGB color of what's behind the front object.  The size is 24 bits per pixel.
If you have the image color at a pixel, and the Z coverage at the pixel, then when the Z coverage is < 255, this channel tells you the color of
the object that was partially obscured by the foreground object.  For example, this info will let you determine what the "real" color of the
foreground object was before it was blended (antialiased) into the background.

Color24 *bgbuffer = (Color24 *)GetChannel(BMM_CHAN_BG,type); 

BMM_CHAN_NODE_RENDER_ID
System node number (valid during a render).  The size is 16 bits per pixel.
The renderer will set the RenderID of all rendered nodes, and will set all non-rendered nodes to 0xffff.  Video Post plug-ins can use the
Interface::GetINodeFromRenderID() method to get a node pointer from an ID in this channel.  Note that this channel is NOT saved with RLA files,
because the IDs would not be meaningful unless the scene was the one rendered.   

UWORD *renderID = (UWORD *)GetChannel(BMM_CHAN_NODE_RENDER_ID,type);
INode *node = ip->GetINodeFromRenderID(*renderID);

BMM_CHAN_COLOR
This option is available in release 3.0 and later only.
This is the color returned by the material shader for the fragment. It is a 24 bit RGB color (3 bytes per pixel).   

Color24 *c1 = (Color24 *)GetChannel(BMM_CHAN_COLOR,type); 

BMM_CHAN_TRANSP
This option is available in release 3.0 and later only.
This is the transparency returned by the material shader for the fragment. It is a 24 bit RGB color (3 bytes per pixel). 

Color24 *transp = (Color24 *)GetChannel(BMM_CHAN_TRANSP,type); 

BMM_CHAN_VELOC
This option is available in release 3.0 and later only.
This gives the velocity vector of the fragment relative to the screen, in screen coordinates. It is a Point 2 (8 bytes per pixel). 

Point2 *src = (Point2 *)GetChannel(BMM_CHAN_VELOC,type); 

BMM_CHAN_WEIGHT
This option is available in release 3.0 and later only.
This is the sub-pixel weight of a fragment. It is a 24 bit RGB color (3 bytes per pixel).  It is the fraction of the total pixel color
contributed by the fragment.  The sum of (color *weight) for all the fragments should give the final pixel color.  The weight ( which is an
RGB triple) for a given fragment takes into account the coverage of the fragment and the transparency of any fragments which are in front of
the given fragment.   

If  c1, c2, c3.. etc are the fragment colors, and w2, w2, w3... etc are the fragment weights, then

pixel color = c1*w1 + c2*w2 +c3*w3 + ... + cN*wN;

The purpose of the sub-pixel weight is to allow post processes to weight the conribution of a post-effect from a particular fragment.  It may
also be necessary to multiply by the fragment's own transparency, which is not included in its weight.  Note that for fragments that have no
transparent fragments in front of them, the weight will be equal to the coverage.

Color24 *w1 = (Color24 *)GetChannel(BMM_CHAN_WEIGHT,type); 

BMM_CHAN_NONE

None of the channels above.

BMM_CHAN_MASK
This option is available in release 4.0 and later only.
The 4x4 (16 bits = 1 word) pixel coverage mask.

***/

#endif // WCS_RLA_H
