// RLA.cpp
// Kinetix file support
// 05/21/99 FPW2 adapted from 3DS RLA code
// Hacked off unused stuff 12/04/00 FPW2
// Copyright 1999

#include "stdafx.h"
#include "Types.h"
#include "RLA.h"
#include "Useful.h"
#include "Render.h"
#include "Project.h"

// Characters to indicate presence of gbuffer channels in the 'program' string
static char gbufChars[] =   "ZEOUNRCBIGTVWM";
//GB_Z-----------------------||||||||||||||
//GB_MTL_ID-------------------|||||||||||||
//GB_NODE_ID-------------------||||||||||||
//GB_UV-------------------------|||||||||||
//GB_NORMAL----------------------||||||||||
//GB_REALPIX----------------------|||||||||
//GB_COVERAGE----------------------||||||||
//GB_BG-----------------------------|||||||
//GB_NODE_RENDER_ID -----------------||||||
//GB_COLOR----------------------------|||||
//GB_TRANSP----------------------------||||
//GB_VELOC------------------------------|||
//GB_WEIGHT------------------------------||
//GB_MASK---------------------------------|

// Other letters in "program" string
//  L  Layer data present
//  P  RenderInfo present
//  D  Node name table is present.

//lint -save -e704
//-----------------------------------------------------------------------------

#ifdef BYTEORDER_LITTLEENDIAN
long lswap(long x) {
	return ((x >> 24) & 0x000000ff) |
         ((x >>  8) & 0x0000ff00) |
         ((x <<  8) & 0x00ff0000) |
         ((x << 24) & 0xff000000);
	}
#else // !BYTEORDER_LITTLEENDIAN
long lswap(long x) {
	return (x);
	}
#endif // !BYTEORDER_LITTLEENDIAN

//-----------------------------------------------------------------------------

#ifdef BYTEORDER_LITTLEENDIAN
//lint -save -e702
short sswap(short x) {
	return ((x >> 8) & 0x00ff) |
         ((x << 8) & 0xff00);
	}
//lint -restore
#else // !BYTEORDER_LITTLEENDIAN
short sswap(short x) {
	return (x);
	}
#endif // !BYTEORDER_LITTLEENDIAN

//-----------------------------------------------------------------------------
#ifdef BYTEORDER_LITTLEENDIAN
unsigned short usswap(unsigned short x) {
	return ((x >> 8) & 0x00ff) |
         ((x << 8) & 0xff00);
	}
#else // !BYTEORDER_LITTLEENDIAN
unsigned short usswap(unsigned short x) {
	return (x);
	}
#endif // !BYTEORDER_LITTLEENDIAN
//lint -restore

inline void USSW(unsigned short &s) { s = usswap(s); }
inline void SSW(short &s) { s = sswap(s); }
inline void LSW(long &l) { l = lswap(l); }

// as Discreet defines the channel sizes
int GBDataSize[NUMGBCHAN] = {4, 1, 2, 8, 4, 4, 1, 3, 2, 3, 3, 8, 3, 2};

/*===========================================================================*/

void MakeRLAProgString(char *s, ULONG chan, BOOL rendInfo)
	{
#ifdef WCS_BUILD_VNS
	strcpy(s, "Visual Nature Studio : (");
#else // !VNS
	strcpy(s, "World Construction Set : (");
#endif // !VNS
	if (chan & BMM_CHAN_Z)			strcat(s," Z");
	if (chan & BMM_CHAN_MTL_ID)		strcat(s," E");
	if (chan & BMM_CHAN_NODE_ID)	strcat(s," O");
	if (chan & BMM_CHAN_UV)			strcat(s," U");
	if (chan & BMM_CHAN_NORMAL)		strcat(s," N");
	if (chan & BMM_CHAN_REALPIX)	strcat(s," R");
	if (chan & BMM_CHAN_COVERAGE)	strcat(s," C");
	if (chan & BMM_CHAN_BG)			strcat(s," B");
	if (rendInfo)					strcat(s," P");
	strcat(s, " )");
	}

/*===========================================================================*/

static int findChar(char *s, char c) {
	for (int i=0; s[i]!=0; i++)
		if (c==s[i])
			return i;
	return -1;
	}

static ULONG gbChannelsFromString(char *s, BOOL &gotRendInfo, BOOL &gotLayerData, BOOL &gotNodeNames) {
	ULONG chan = 0;
	gotRendInfo = FALSE;
	int i = findChar(s,'(');
	if (i<0)  
		return 0;
	for (i++; s[i]!=0; i++) {
		char c = s[i];
		int n = findChar(gbufChars, c);
		if (n>=0) 
			chan |= (1<<n);
		else switch(c) {
			case 'P': gotRendInfo = TRUE; break;
			case 'L': gotLayerData = TRUE; break;
			case 'D': gotNodeNames = TRUE; break;
			default: break;
			}
		}
	return chan;
	}

int MaxGBSize(ULONG gbChannels) {
	int sz = 0;
	for (int i=0; i<NUMGBCHAN; i++) {
		if (gbChannels&(1<<i)) {
			int s = GBDataSize[i];
			if (s>sz) sz = s;
			}
		}
	if (sz<2) sz = 2;
	return sz;
	}

/*===========================================================================*/

void SwapRLAHdrBytes(RLAHeader& h) {
	USSW(h.window.left);
	USSW(h.window.right);
	USSW(h.window.top);
	USSW(h.window.bottom);
	USSW(h.active_window.left);
	USSW(h.active_window.right);
	USSW(h.active_window.top);
	USSW(h.active_window.bottom);
	SSW(h.frame);
	SSW(h.storage_type);
	SSW(h.num_chan);
	SSW(h.num_matte);
	SSW(h.num_aux);
	USSW(h.revision);
	LSW(h.job_num);
	SSW(h.field);
	SSW(h.chan_bits);
	SSW(h.matte_type);
	SSW(h.matte_bits);
	SSW(h.aux_bits);
	LSW(h.next);
	}

/*===========================================================================*/

// Encodes one byte channel from input buffer.
int rla_encode(unsigned char *input, unsigned char *output, int xSize, int stride)
{
unsigned char *in = input;
unsigned char *out = output;
unsigned char *inend = in+(xSize*stride);
signed char *outid = (signed char*)out;
unsigned char lastval = ! *in;
int runcnt = 0;
int cnt;

out++;

while (in < inend)
	{
	unsigned char val = *in;
	*out++ = val;
	in += stride;
	if (val == lastval)
		{
		if (++runcnt == 3)
			{
			cnt = (signed char*)out - outid;
			if (cnt > 4)
				{
				*outid = -(cnt - 4);
				outid = (signed char*)out - 3;
				}
			while (in < inend)
				{
				val = *in;
				if (val == lastval)
					{
					runcnt++;
					in += stride;
					}
				else
					{
					break;
					}
        	   }
			out = (unsigned char *)outid + 1;
            while (runcnt)
				{
                int chunk = runcnt;
                if (chunk > 128) chunk = 128;
                *outid = chunk-1;
                outid += 2;
                *out = lastval;
	 			out += 2;
	 			runcnt -= chunk;
	            }
            if (in < inend)
				{
	 			*out++ = val;
                in += stride;
				}
            lastval = val;
            runcnt = 1;
        }
		else if ((cnt = (signed char*)out - outid) == 129)
			{
			*outid = -(cnt-1);
            outid = (signed char*)out;
            lastval = ! *in;
            out++;
            runcnt = 0;
	        }
    }
	else
		{
        cnt = ((signed char*)out-outid);
        if (cnt == 129)
			{
            *outid = -(cnt-1);
            outid = (signed char *)out;
            lastval = ! *in;
				out++;
            runcnt = 0;
			}
		else
			{
            lastval = val;
			runcnt = 1;
			}
		}
	}

if ((signed char*)out-outid > 1)
	{
	*outid = -((signed char*)out - outid - 1);
	}
else
	{
	out = (unsigned char*)outid;
	}

return (out - output);

} // rla_encode

/*===========================================================================*/

// Decodes one run-encoded channel from input buffer.
BYTE* decode(BYTE *input, BYTE *output, int xFile, int xImage, int stride)
{
int count, x = xFile;
int bytes = 0;
int useX  = 0;
int xMax  = xFile < xImage ? xFile : xImage;

BYTE  *out = (BYTE  *)output;
while (x > 0)
	{
	count = *(signed char *)input++;
	bytes++;
	if (count >= 0) {
		// Repeat pixel value (count + 1) times.
		while (count-- >= 0)
			{
			if (useX < xImage)
				{
				*out = *input;
				out += stride;
				}
			--x;
			useX++;
			}
		++input;
		bytes++;
		}
	else
		{
		// Copy (-count) unencoded values.
		for (count = -count; count > 0; --count)
			{
			if (useX < xImage)
				{
				*out = *input;
				out += stride;
				}
			input++;
			bytes++;
			--x;
			useX++;
			}
		}
	}

return input;

} // rla_decode

/*===========================================================================*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// RLA Reader
//-----------------------------------------------------------------------------

RLAReader::RLAReader(RLAHeader *hd, FILE *f, int w, int h) {
	hdr = hd;
	fd = f; 
	width = w; 
	height = h; 
	maxLength = 6 * ((width * 280) / 256); 
	offsets = NULL;
	inp = NULL;
	}

int RLAReader::Init() {
	offsets = new long[height];
	inp = new BYTE[maxLength];
	out = new BYTE[width];
	if (offsets==NULL||inp==NULL||out==NULL) return 0;
	// Read in table of offsets.
	int count = (int)fread(offsets, sizeof(long), height, fd);
	if (count != height) return 0;
	for (int i=0; i<height; i++) offsets[i] = lswap(offsets[i]);
	return 1;
	}

/***
#define RENDINFO_VERS1 1000
int RLAReader::ReadRendInfo(RenderInfo *ri) {
   short vers;
   if (fread(&vers,sizeof(short),1,fd)!=1) return 0;
   int size = sizeof(RenderInfo);
   if (vers != RENDINFO_VERS1) {
      // the old version didn't start with a version word, but
      // with projType which is 0 or 1.
      size -= sizeof(Rect); // The old record didn't have the region Rect.
      fseek(fd,-sizeof(short),SEEK_CUR); // Undo the version read
   }
   if (ri) {
      if (fread(ri,size,1,fd)!=1) return 0;
   }
   else
      fseek(fd, size, SEEK_CUR);
   return 1;
}
***/

ULONG RLAChannelsFromString(char *s, BOOL &gotRendInfo, BOOL &gotLayerData, BOOL &gotNodeNames)
	{
	return gbChannelsFromString(s, gotRendInfo, gotLayerData, gotNodeNames);
	}

RLAReader::~RLAReader() {
	delete [] offsets;
	delete [] inp;
	delete [] out;
	}

int RLAReader::BeginLine(int y) {
	int yy = (height - 1) - y;
	// position at beginning of line;
	fseek(fd, offsets[yy], SEEK_SET);
	return 1;
	}

int RLAReader::ReadNextChannel( BYTE *out, int stride, int w, BOOL longLength) {	
	WORD length = 0;
	if (w<0) w = width;
#ifdef RPF_DEBUG
	long fpos = ftell(fd);
	char msg[80];
	sprintf(msg, "@ %d\n", fpos);
	DEBUGOUT(msg);
#endif // RPF_DEBUG
	if (longLength) {
		int l = 0;
		if (fread(&l, 4, 1, fd)!=1)
			return 0;
		l = lswap(l);
		if (l>=maxLength)
			return 0;
		if (fread(inp, 1, l, fd) == 0)
			return 0;
		}
	else {
		if (fread(&length, 2, 1, fd)!=1)
			return 0;
		length = sswap(length);
		if (length>=maxLength)
			return 0;
		if (fread(inp, 1, length, fd) == 0)
			return 0;
		}
	curInp = decode(inp, out, w, w, stride);             
	return 1;
	}

int RLAReader::ReadLowByte( BYTE *out, int stride) {	
	curInp = decode(curInp, out, width, width, stride);             
	return 1;
	}

int RLAReader::ReadWordChannel(WORD *ptr, int stride, BOOL twoBytes) {
	int i;
	WORD *p = ptr;
	if (!ReadNextChannel(out, 1)) return 0;
	for (i=0; i<width; i++) { 	*p = out[i]<<8;	 p += stride; 	}
	if (twoBytes) {
		if (!ReadLowByte(out, 1)) return 0;
		p = ptr;
		for (i=0; i<width; i++) { *p += out[i];	p += stride; } 
		}
	return 1;
	}

int RLAReader::ReadRGBA( Max_Color_64 *pix) {
	BOOL do2 = (hdr->chan_bits==16)?1:0;
	if (!ReadWordChannel(&pix->r, 4, do2)) return 0;
	if (!ReadWordChannel(&pix->g, 4, do2)) return 0;
	if (!ReadWordChannel(&pix->b, 4, do2)) return 0;
	if (hdr->num_matte>0) 
		if (!ReadWordChannel(&pix->a, 4, hdr->matte_bits==16?1:0)) return 0;
	return 1;
	}

int RLAReader::ReadNChannels( BYTE *out, int n, int w, BOOL longLength) {	
	// read n byte channels starting with high byte
#ifdef RPF_DEBUG
	char msg[80];
	sprintf(msg, "Channels = %d\n", n);
	DEBUGOUT(msg);
#endif // RPF_DEBUG
	for (int i=n-1; i>=0; i--) 
		if (!ReadNextChannel( out+i, n, w, longLength)) 
			return 0;
	return 1;		
	}

int RLAReader::ReadNumLRecs(void) {
	long n;
	if (fread(&n, 4, 1, fd)!=1)
		return -1;
	return n;
	}


FragmentLoadTransfer::FragmentLoadTransfer()
{
FragZValid = FragMatIDValid = FragNodeIDValid = FragUVValid = FragNormalValid = FragRGBValid
 = Coverage8Valid = FragBGRGBValid = FragRenderIDValid = FragColorRGBValid = FragTransRGBValid = VelocValid
 = FragWeightRGBValid = FourByFourMaskValid = 0;
} // FragmentLoadTransfer::FragmentLoadTransfer


FragmentLoadTransfer::~FragmentLoadTransfer()
{
} // FragmentLoadTransfer::~FragmentLoadTransfer
