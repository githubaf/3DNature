// ImageInputFormat.cpp
// built from ragged Image Loader hunks of Bitmaps.cpp on 12/06/00 by CXH
// headers from Bitmaps.cpp follow:
/* BitMaps.cpp (ne gisbitmaps.h 14 Jan 1994 CXH)
** The functions relating to 24-bit maps, creating, saving and closing.
** Original code by Gary R. Huber
** Butchered on 9/8/95 for V2_VTUX edition by Chris "Xenon" Hanson
// TIFF routines moved to ImageInputFromatTIFFs on 05/09/06 by FW2
*/

#include "stdafx.h"
#include "ImageFormatConfig.h"
#include "AppMem.h"
#include "Useful.h"
#include "Application.h"
#include "Project.h"
#include "Requester.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "ImageFormatIFF.h"
#include "ImageFormatTIFFs.h"
#include "Raster.h"
#include "RLA.h"
#include "WCSVersion.h"
#include "PostProcessEvent.h"
#include "CoordSys.h"

#ifdef WCS_BUILD_JPEG_SUPPORT
#include <setjmp.h>
extern "C" {
#include "jpeglib.h"
// This doesn't operate well enough to enable at this time
#define WCS_IMAGE_MANAGEMENT_JPEG
}
#endif // WCS_BUILD_JPEG_SUPPORT

#ifdef WCS_BUILD_PNG_SUPPORT
extern "C" {
#include "png.h"
} // extern C
#endif // WCS_BUILD_PNG_SUPPORT

extern int GBDataSize[NUMGBCHAN];

const char *FileExtension[] = {"", ".iff", ".iff24", ".tga", ".bmp", ".pct", ".pic", ".pict", ".red", ".raw", ".elev", ".jpg", ".jpeg", ".tif", ".tiff", ".png", ".ecw", ".sid", ".jp2", NULL};

static PICTdata Loaddat;

/*===========================================================================*/

short CheckExistUnknownImageExtension(char *Name)
{
FILE *fTest;
short Length, i = 0;
char TestName[256];

strncpy(TestName, Name, 250);
Length = (short)strlen(TestName);

while (FileExtension[i])
	{
	strcpy(&TestName[Length], FileExtension[i]);
	if (fTest = PROJ_fopen(TestName, "rb"))
		{
		fclose(fTest);
		//return (LoadRasterImage(TestName, LoadRas, SupressWng));
		return(1);
		} // if
	i ++;
	} // while

return (0);

} // CheckExistUnknownImageExtension

/*===========================================================================*/

// This one attempts to automatically determine the type of the image,
// and then calls the appropriate Bitmaps.cpp/Load* function to do the
// real work.
short LoadRasterImage(char *Name, Raster *LoadRas, short SupressWng, ImageCacheControl *ICC)
{
FILE *fh = NULL;
short error = 0;
// Really, it's not polite to modify the filename in the buffer passed
// to the function, so we'll make a local copy to hack on.
char LILocalName[256];

// Ensure stale georeferencing info is tossed.
LoadRas->DiscardLoaderGeoRefShell();

if (Name)
	{
	strncpy(LILocalName, Name, 250);
	LILocalName[250] = NULL;
	while (fh == NULL)
		{
		if (fh = PROJ_fopen(LILocalName, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/))
			{
			fclose(fh);
			fh=NULL;
			switch(IdentImage(Name))
				{
				default:
				case WCS_BITMAPS_IDENTIMAGE_ERROR:
					{
					error = 1;
					break;
					} // ERROR
				case WCS_BITMAPS_IDENTIMAGE_UNKNOWN:
					{ // Can't load Sculpt files from here anymore
					//if ((Width != 0) && Height != 0)
					//	{
					//	return(LoadRas->LoadIFForSculpt(LILocalName, SupressWng));
					//	} // if
					error = 1;
					break;
					} // UNKNOWN
				case WCS_BITMAPS_IDENTIMAGE_IFFZBUF:
					{
					//return(LoadRas->LoadIFFZBUF(LILocalName, SupressWng));
					return(0); // error, should not load this from auto identify
					break;	//lint !e527
					} // IFFILBM
				case WCS_BITMAPS_IDENTIMAGE_IFFILBM:
					{
					return(LoadRas->LoadIFFILBM(LILocalName, SupressWng, ICC));
					break;	//lint !e527
					} // IFFILBM
				case WCS_BITMAPS_IDENTIMAGE_TARGA:
					{
					return(LoadRas->LoadTGA(LILocalName, SupressWng, ICC));
					break;	//lint !e527
					} // TARGA
				case WCS_BITMAPS_IDENTIMAGE_BMP:
					{
					return(LoadRas->LoadBMP(LILocalName, SupressWng, ICC));
					break;	//lint !e527
					} // BMP
				case WCS_BITMAPS_IDENTIMAGE_ECW:
					{
					#ifdef WCS_BUILD_ECW_SUPPORT
					return(LoadRas->LoadECW(LILocalName, SupressWng, ICC));
					#else // !WCS_BUILD_ECW_SUPPORT
					return(0);
					#endif // WCS_BUILD_ECW_SUPPORT
					break;	//lint !e527
					} // ECW
				case WCS_BITMAPS_IDENTIMAGE_JP2:
					{
					#ifdef WCS_BUILD_ECW_SUPPORT
					#ifdef WCS_BUILD_JP2_SUPPORT // we get JP2 as part of ECW 3.x and later
					return(LoadRas->LoadECW(LILocalName, SupressWng, ICC));
					#endif // WCS_BUILD_JP2_SUPPORT
					#else // !WCS_BUILD_ECW_SUPPORT
					return(0);
					#endif // WCS_BUILD_ECW_SUPPORT
					break;	//lint !e527
					} // JPEG2000
				case WCS_BITMAPS_IDENTIMAGE_MRSID:
					{
					#ifdef WCS_BUILD_MRSID_SUPPORT
					return(LoadRas->LoadMRSID(LILocalName, SupressWng, ICC));
					#else // !WCS_BUILD_MRSID_SUPPORT
					return(0);
					#endif // WCS_BUILD_MRSID_SUPPORT
					break;	//lint !e527
					} // MRSID
				case WCS_BITMAPS_IDENTIMAGE_PNG:
					{
					#ifdef WCS_BUILD_PNG_SUPPORT
					return(LoadRas->LoadPNG(LILocalName, SupressWng, ICC));
					#else // !WCS_BUILD_PNG_SUPPORT
					return(0);
					#endif // WCS_BUILD_PNG_SUPPORT
					break;	//lint !e527
					} // PNG
				case WCS_BITMAPS_IDENTIMAGE_JPEG:
				case WCS_BITMAPS_IDENTIMAGE_JPG:
					{
					#ifdef WCS_BUILD_JPEG_SUPPORT
					return(LoadRas->LoadJPG(LILocalName, SupressWng, ICC));
					#else // !WCS_BUILD_JPEG_SUPPORT
					return(0);
					#endif // WCS_BUILD_JPEG_SUPPORT
					break;	//lint !e527
					} // JPG
				case WCS_BITMAPS_IDENTIMAGE_TIFF:
				case WCS_BITMAPS_IDENTIMAGE_TIF:
					{
					#ifdef WCS_BUILD_TIFF_SUPPORT
					return(LoadRas->LoadTIFF(LILocalName, SupressWng, ICC));
					#else // !WCS_BUILD_TIFF_SUPPORT
					return(0);
					#endif // !WCS_BUILD_TIFF_SUPPORT
					break;	//lint !e527
					} // TIFF
				case WCS_BITMAPS_IDENTIMAGE_PIC:
				case WCS_BITMAPS_IDENTIMAGE_PCT:
				case WCS_BITMAPS_IDENTIMAGE_PICT:
					{
					return(LoadRas->LoadPICT(LILocalName, SupressWng));
					break;	//lint !e527
					} // PICT
				case WCS_BITMAPS_IDENTIMAGE_RPF:
				case WCS_BITMAPS_IDENTIMAGE_RLA:
					{
					return(LoadRas->LoadRPF_RLA(LILocalName, SupressWng));
					break;	//lint !e527
					}
				case WCS_BITMAPS_IDENTIMAGE_WCSDEM:
					{
					break;
					} // WCSDEM
				case WCS_BITMAPS_IDENTIMAGE_SGIRGB:
					{
					return(LoadRas->LoadSGIRGB(LILocalName, SupressWng));
					break;	//lint !e527
					} // SGI RGB
				} // switch on image type
			break; // out of the while loop
			} // if
		else
			{
#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
			if (0) // I know
				{
				} // if
#ifdef WCS_BUILD_ECW_SUPPORT
			else if (!strnicmp(LILocalName, "ecwp:", 5))
				{
				return(LoadRas->LoadECW(LILocalName, SupressWng, ICC));
				} // if
#endif // WCS_BUILD_ECW_SUPPORT

#ifdef WCS_BUILD_MRSID_SUPPORT
			else if /*(!strnicmp(LILocalName, "mrsid:", 5))*/ (0) // disabled until they do it
				{
				} // if
#endif // WCS_BUILD_MRSID_SUPPORT
			else
#endif // WCS_BUILD_REMOTEFILE_SUPPORT
				{
				long namelen;

				namelen = (long)strlen(LILocalName);
				if (namelen > 1)
					{
					LILocalName[namelen - 1] = NULL;
					} // if
				else
					{
					error = 1;
					break;
					} // else
				} // else
			} // else
		} // while
	} // if
else
	{
	// no name supplied
	return(0);
	} // else

if (error == 1)
	{
	if (! SupressWng) GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, Name);
	return(0);
	} // if

return (1);

} // LoadRasterImage()

/*===========================================================================*/

short Raster::LoadTGA(char *Name, short SupressWng, ImageCacheControl *ICC)
{
unsigned char IDBUF[30], FlipVert = 0, FlipHoriz = 0, Success = 0,
 BytesPerPixel, BytesPerPal = 0, GoodLine,
 *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf, *ABuf;
unsigned long Compression = 0, WorkRow, InScan, PixelCol;
unsigned char TGA_IDLen, TGA_CMType, TGA_IMType, TGA_CMDeep, TGA_PixDepth, TGA_ImgDesc,
 TR, TG, TB, TA, DoAlpha = 0, AlphaFailed = 0;
unsigned short TGA_CMStart, TGA_CMLen, TGA_Width, TGA_Height;
unsigned char *Alpha = NULL;
FILE *fh = NULL;

if (ICC) ICC->QueryOnlyFlags = NULL; // clear all flags


if (fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/))
	{
	// We're going to disable the TRUEVISION-XFILE checking to allow us to use
	// "old" flavor TGAs generated by ImageFX for example.
	memset(IDBUF, 0, sizeof(IDBUF));
	fseek(fh, -18, SEEK_END);
	fread(IDBUF, 1, 18, fh);
	//if (memcmp(IDBUF, "TRUEVISION-XFILE.", 17) == 0)
	if (1)
		{
		fseek(fh, 0, SEEK_SET);
		fread(&TGA_IDLen, 1, 1, fh);
		fread(&TGA_CMType, 1, 1, fh);
		fread(&TGA_IMType, 1, 1, fh);
		fread(&TGA_CMStart, 1, 2, fh); // Intel Byte-order, ignorable
		fread(&TGA_CMLen, 1, 2, fh); // Intel Byte-order
		fread(&TGA_CMDeep, 1, 1, fh);
		fread(&TGA_Width, 1, 2, fh); // really X-Origin, ignored
		fread(&TGA_Height, 1, 2, fh); // really Y-Origin, ignored
		fread(&TGA_Width, 1, 2, fh); // Intel Byte-order
		fread(&TGA_Height, 1, 2, fh); // Intel Byte-order
		fread(&TGA_PixDepth, 1, 1, fh);
		fread(&TGA_ImgDesc, 1, 1, fh);
		#ifdef BYTEORDER_BIGENDIAN
		SimpleEndianFlip16U(TGA_CMStart, &TGA_CMStart);
		SimpleEndianFlip16U(TGA_CMLen, &TGA_CMLen);
		SimpleEndianFlip16U(TGA_Width, &TGA_Width);
		SimpleEndianFlip16U(TGA_Height, &TGA_Height);
		#endif // BYTEORDER_BIGENDIAN

		if (((TGA_IMType == 2) || (TGA_IMType == 10)) && ((TGA_PixDepth == 24) || (TGA_PixDepth == 32)))
			{
			if (TGA_IMType == 10)
				Compression = 1;
			if (TGA_PixDepth == 32)
				{
				BytesPerPixel = 4;
				DoAlpha = 1;
				} // if
			else
				{
				BytesPerPixel = 3;
				} // else

			FlipVert = 1;
			if (TGA_ImgDesc & 0x20)
				FlipVert = 0;
			if (TGA_ImgDesc & 0x10)
				FlipHoriz = 1;

			// skip over any imageID
			if (TGA_IDLen)
				fseek(fh, TGA_IDLen, SEEK_CUR);

			// skip over any palette present
			if (TGA_CMType && TGA_CMLen)
				{
				BytesPerPal = ROUNDUP(TGA_CMDeep, 8) / 8;
				fseek(fh, TGA_CMLen * BytesPerPal, SEEK_CUR);
				} // if
			Cols = TGA_Width;
			Rows = TGA_Height;
			ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TGA Loader Bitmaps");
			ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TGA Loader Bitmaps");
			ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TGA Loader Bitmaps");
			if (DoAlpha)
				{
				if (!(Alpha = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TGA Loader Alpha Bitmaps")))
					{
					AlphaFailed = 1;
					} // if
				} // if
			InterleaveBuf = (UBYTE *)AppMem_Alloc(ROUNDUP((Cols * BytesPerPixel), 4), 0, "TGA DeInterleave Buffer");
			if (ByteMap[0] && ByteMap[1] && ByteMap[2] && InterleaveBuf && !AlphaFailed) // everything ok?
				{
				// Clear bitmaps
				memset(ByteMap[0], 0, Cols * Rows);
				memset(ByteMap[1], 0, Cols * Rows);
				memset(ByteMap[2], 0, Cols * Rows);

				if (DoAlpha && Alpha)
					{
					memset(Alpha, 0, Cols * Rows);
					} // if

				// Start reading the pixels
				for (WorkRow = 0; WorkRow < (unsigned)Rows; WorkRow++)
					{
					GoodLine = 0;
					// Set up pointers to indicate R G and B destination buffers for
					// either top-down or bottom-up behavior
					if (FlipVert)
						{ // bottom-up
						RBuf = &ByteMap[0][((Rows - 1) - WorkRow) * Cols];
						GBuf = &ByteMap[1][((Rows - 1) - WorkRow) * Cols];
						BBuf = &ByteMap[2][((Rows - 1) - WorkRow) * Cols];
						if (DoAlpha) ABuf = &(Alpha)[((Rows - 1) - WorkRow) * Cols];
						} // if
					else
						{ // top-down
						RBuf = &ByteMap[0][WorkRow * Cols];
						GBuf = &ByteMap[1][WorkRow * Cols];
						BBuf = &ByteMap[2][WorkRow * Cols];
						if (DoAlpha) ABuf = &(Alpha)[WorkRow * Cols];
						} // if
					if (Compression)
						{
						for (InScan = 0; InScan < (unsigned)Cols;)
							{
							// I reuse TGA_IDLen and TGA_CMLen for
							// decompression variables here, since at this point we
							// don't give a hoot about palettes.
							unsigned char TGA_DCTemp;
							if (fread(&TGA_DCTemp, 1, 1, fh)) // CMStart has RepCount field
								{
								TGA_IDLen = TGA_DCTemp & 0x7f; // IDLen is run/raw length -- low 7 bits of RepCount
								TGA_IDLen++; // We add one to the RepCount, because a 0-byte run/raw makes no sense
								if ((TGA_IDLen + InScan) > (unsigned)Cols)
									{
									GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "TGA: Decompression error.");
									} // if
								else
									{
									if (TGA_DCTemp & 0x80) // Check bit 8 of RepCount field for run/raw indicator
										{ // run-length packet
										fread(&TB, 1, 1, fh);
										fread(&TG, 1, 1, fh);
										fread(&TR, 1, 1, fh);
										if (BytesPerPixel == 4)
											{
											fread(&TA, 1, 1, fh);
											} // if
										// Reuse PixelCol as an offset precalculation
										PixelCol = InScan * BytesPerPixel;
										for (TGA_CMLen = 0; TGA_CMLen < TGA_IDLen; TGA_CMLen++)
											{
											InterleaveBuf[PixelCol++] = TB;
											InterleaveBuf[PixelCol++] = TG;
											InterleaveBuf[PixelCol++] = TR;
											if (BytesPerPixel == 4)
												{
												InterleaveBuf[PixelCol++] = TA; 
												} // if
											} // for
										} // if
									else
										{ // raw packet, TGA_IDLen has already been adjusted for "one-less" philosophy
										if (fread(&InterleaveBuf[InScan * BytesPerPixel], 1, TGA_IDLen * BytesPerPixel, fh) != (unsigned)(TGA_IDLen * BytesPerPixel))
											{
											GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "TGA: Decompression error -- unexpected end of file.");
											break;
											} // if
										} // else
									InScan += TGA_IDLen; // We did that many pixels
									} // else
								} // if
							} // for
						if (InScan == (unsigned)Cols)
							{
							GoodLine = 1;
							} // if
						} // if
					else
						{
						if (fread(InterleaveBuf, 1, (Cols * BytesPerPixel), fh) == (unsigned)(Cols * BytesPerPixel))
							{
							GoodLine = 1; // Uncompressed is pretty easy, eh?
							} // if
						} // else
					if (GoodLine) // Did we get a complete scanline?
						{
						// DeInterleave the BGR data into the apropriate R, G and B planes
						for (InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
							{
							BBuf[PixelCol] = InterleaveBuf[InScan++];
							GBuf[PixelCol] = InterleaveBuf[InScan++];
							RBuf[PixelCol] = InterleaveBuf[InScan++];
							if (DoAlpha)
								{
								ABuf[PixelCol] = InterleaveBuf[InScan++];
								} // if
							else
								{
								if (BytesPerPixel == 4)
									InScan++; // Skip over alpha channel byte
								} // else
							} // for
						if (FlipHoriz) // do we need to flip L-R?
							{
							SimpleDataFlip(RBuf, Cols);
							SimpleDataFlip(GBuf, Cols);
							SimpleDataFlip(BBuf, Cols);
							if (DoAlpha)
								{
								SimpleDataFlip(ABuf, Cols);
								} // if
							} // if
						} // if
					else
						{
						GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "End of file while reading TGA image data.");
						break; // bail out of scanline loop
						} // else
					} // for
				if (WorkRow == (unsigned)Rows) // Did we make it all the way through the image?
					{
					Success = 1;
					} // if
				} // if
			if (InterleaveBuf) AppMem_Free(InterleaveBuf, ROUNDUP((Cols * BytesPerPixel), 4)); InterleaveBuf = NULL;
			if (!Success)
				{
				if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
				if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
				if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
				if (DoAlpha && Alpha) AppMem_Free(Alpha, Cols * Rows); Alpha = NULL;
				} // if
			} // if
		else
			{
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, APP_TLA" TGA loader only supports 24/32-bit compressed/uncompressed TARGA formats.");
			} // else
		} // if
	if (!Success)
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Image load aborted.");
		} // if
	fclose(fh);
	} // if

if (Success)
	{
	//*NewPlanes = 24;
	ByteBands = 3;
	ByteBandSize = Rows * Cols;
	if (DoAlpha && Alpha)
		{
		AlphaAvailable = 1;
		if (AlphaEnabled)
			{
			CopyAlphaToCoverage(Alpha);
			} // if
		AppMem_Free(Alpha, Cols * Rows);
		Alpha = NULL;
		} // if
	else
		{
		AlphaAvailable = 0;
		AlphaEnabled = 0;
		} // else
	} // if

return(Success);

} // Raster::LoadTGA

/*===========================================================================*/

short Raster::LoadRPF_RLA(char *Name, short SupressWng)
{
class RLAReader *rla_rdr = NULL;
struct RLAHeader *hdr = NULL;
Max_Color_64 *line64 = NULL, *p64;
FILE *fin;
//FILE *fout = fopen("D:/rpf_rla.raw", "wb");
float inv64k = 1.0f / 65535.0f, *fr, *fg, *fb, *fa, *fa2, *z2;
unsigned long gbChannels;
unsigned short *m2;
long x, y;
int rsize_b, rsize_f;
BOOL gotLayerData = FALSE, gotNodeNames = FALSE, gotRendInfo = FALSE, DidComposite = FALSE;
short success = 0;
BYTE *gbChan[NUMGBCHAN];
int h, maxgbsz, sz, w;	// height, max gbuffer size, size, width
RLASampleData Samp;
unsigned char *br, *bg, *bb, *r2, *g2, *b2;

if ((fin = PROJ_fopen(Name, "rb")) && (hdr = (RLAHeader *)AppMem_Alloc(sizeof(RLAHeader), 0)))
	{
	if (fread(hdr, sizeof(RLAHeader), 1, fin) == 0)
		goto bailout;
#ifdef BYTEORDER_LITTLEENDIAN
	SwapRLAHdrBytes(*hdr);
#endif // BYTEORDER_LITTLEENDIAN
	BOOL oldver = (hdr->revision == (unsigned short)RLA_MAGIC_OLD) ? 1 : 0;
	w = hdr->active_window.right - hdr->active_window.left + 1;
	h = hdr->active_window.top - hdr->active_window.bottom + 1;

	// now we expect to find 3 RGB channels (storage type 0 = int instead of float?!?)
	if ((hdr->num_chan == 3) && (hdr->storage_type == 0) && (stricmp(hdr->chan, "rgb") == 0))
		{
		Cols = hdr->active_window.right - hdr->active_window.left + 1;
		Rows = hdr->active_window.top - hdr->active_window.bottom + 1;
		rsize_b = Cols * Rows;
		rsize_f = Cols * Rows * sizeof(float);
		ByteMap[WCS_RASTER_IMAGE_BAND_RED] = (unsigned char *)AppMem_Alloc(rsize_b, 0);
		ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] = (unsigned char *)AppMem_Alloc(rsize_b, 0);
		ByteMap[WCS_RASTER_IMAGE_BAND_BLUE] = (unsigned char *)AppMem_Alloc(rsize_b, 0);
		if (!ByteMap[WCS_RASTER_IMAGE_BAND_RED] || !ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] ||
			!ByteMap[WCS_RASTER_IMAGE_BAND_BLUE])
			goto bailout;
		FloatMap[WCS_RASTER_IMAGE_FBAND_RED] = (float *)AppMem_Alloc(rsize_f, 0);
		FloatMap[WCS_RASTER_IMAGE_FBAND_GREEN] = (float *)AppMem_Alloc(rsize_f, 0);
		FloatMap[WCS_RASTER_IMAGE_FBAND_BLUE] = (float *)AppMem_Alloc(rsize_f, 0);
		if (!FloatMap[WCS_RASTER_IMAGE_FBAND_RED] || !FloatMap[WCS_RASTER_IMAGE_FBAND_GREEN] ||
			!FloatMap[WCS_RASTER_IMAGE_FBAND_BLUE])
			goto bailout;
		if (hdr->num_matte != 0)
			{
			FloatMap[WCS_RASTER_IMAGE_FBAND_ALPHA] = (float *)AppMem_Alloc(rsize_f, 0);
			if (!FloatMap[WCS_RASTER_IMAGE_FBAND_ALPHA])
				goto bailout;
			} // if matte channels
		if (!(line64 = (Max_Color_64 *)AppMem_Alloc(sizeof(Max_Color_64) * Cols, 0)))
			goto bailout;

		gbChannels = RLAChannelsFromString(hdr->program, gotRendInfo, gotLayerData, gotNodeNames);

		/***
		GBuffer *gb = NULL;
		if (gbChannels) {
			s->CreateChannels(gbChannels);
			if (s->ChannelsPresent()==gbChannels) {
				ULONG ctype;
				for (int i=0; i<NUMGBCHAN; i++) 
					gbChan[i] = (BYTE *)s->GetChannel(1<<i,ctype);
				}
			gb = s->GetGBuffer();
			maxgbsz = MaxGBSize(gbChannels);
			gb->InitBuffer();
			}
		***/

		if (gbChannels)
			{
			for (int i = 0; i < NUMGBCHAN; i++)
				{
				if ((1 << i) & gbChannels)
					{
					sz = GBDataSize[i];
					gbChan[i] = (BYTE *)AppMem_Alloc(w * h * sz, APPMEM_CLEAR);
					}
				else
					gbChan[i] = NULL;
				} // for i
			maxgbsz = MaxGBSize(gbChannels);
			} // if gbChannels

		RLAReader rla_rdr(hdr, fin, Cols, Rows);

		// This reads in the offsets table and positions read head after it.
		if (!rla_rdr.Init())
			goto bailout;

		/***
		if (gotRendInfo)
			{
			RenderInfo* ri = s->AllocRenderInfo();
			if (!rla.ReadRendInfo(ri)) goto bailout;
			}

		if (gotNodeNames)
			{
			assert(gb);
			if (!rla.ReadNameTab(gb->NodeRenderIDNameTab())) goto bailout;
			}
		***/

		br = ByteMap[WCS_RASTER_IMAGE_BAND_RED];
		bg = ByteMap[WCS_RASTER_IMAGE_BAND_GREEN];
		bb = ByteMap[WCS_RASTER_IMAGE_BAND_BLUE];
		fr = FloatMap[WCS_RASTER_IMAGE_FBAND_RED];
		fg = FloatMap[WCS_RASTER_IMAGE_FBAND_GREEN];
		fb = FloatMap[WCS_RASTER_IMAGE_FBAND_BLUE];
		fa = FloatMap[WCS_RASTER_IMAGE_FBAND_ALPHA];

		Samp.Channels = gbChannels;
		Samp.ImageWidth = Cols;
		Samp.ImageHeight = Rows;

		if (LoadCompositer)
			Samp.weight[0] = Samp.weight[1] = Samp.weight[2] = 255;

		for (y = 0; y < Rows; y++)
			{
			rla_rdr.BeginLine(y);
			if (!rla_rdr.ReadRGBA(line64))
				goto bailout;
			// copy their scanline data into our float channels
			p64 = &line64[0];
			if (LoadCompositer)
				Samp.Y = y;
			r2 = br;
			g2 = bg;
			b2 = bb;
			fa2 = fa;
			for (x = 0; x < Cols; x++, p64++)
				{
				*br++ = p64->r >> 8;
				*fr++ = p64->r * inv64k;
				*bg++ = p64->g >> 8;
				*fg++ = p64->g * inv64k;
				*bb++ = p64->b >> 8;
				*fb++ = p64->b * inv64k;
				if (fa)
					*fa++ = p64->a * inv64k;
				}

			if (gbChannels)
				{
				long xx;
				for (int i = 0; i < NUMGBCHAN; i++)
					{
					if (gbChan[i])
						{
						sz = GBDataSize[i];
						// hack to get around problem if weight channel is present
						if (i == GB_WEIGHT)
							continue;
						else
							{
							if (!rla_rdr.ReadNChannels(&gbChan[i][w*y*sz], sz))
								goto bailout;
							if (i == GB_Z)
								z2 = (float *)(&gbChan[i][w*y*sz]);
							if (i == GB_MASK)
								m2 = (unsigned short *)(&gbChan[i][w*y*sz]);
							} // else
						} // if gbChan[i]
					} // for i
				if (LoadCompositer)
					{
					for (xx = 0; xx < Cols; xx++)
						{
						Samp.X = xx;
						if (gbChan[GB_Z])
							{
							#ifdef BYTEORDER_BIGENDIAN
							BlindSimpleEndianFlip32F(z2, z2);
							#endif // BYTEORDER_BIGENDIAN
							Samp.Z = -*z2++;
							} // if
						else
							Samp.Z = 0.0f;
						Samp.color[0] = *r2++;
						Samp.color[1] = *g2++;
						Samp.color[2] = *b2++;
						if (fa)
							Samp.Alpha = (unsigned char)(*fa2++ * 255 + 0.5);
						else
							Samp.Alpha = 255;
						if (gbChan[GB_MASK])
							{
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip16U(*m2, m2);
							#endif // BYTEORDER_BIGENDIAN
							Samp.mask = *m2++;
							} // if
						LoadCompositer->EvalOneRLASample(&Samp);
						} // for xx
					DidComposite = TRUE;
					} // if LoadCompositer
				if (gotLayerData)
					{
					int j;
					int nlrecs = rla_rdr.ReadNumLRecs();
					if (nlrecs < 0)
						goto bailout;
					if (nlrecs > 0)
						{
						// F2: hack for per pixel processing
						int sumsz = 2;	// x array {shorts}
						for (j = 0; j < NUMGBCHAN; j++)
							{
							if (gbChan[j])
								sumsz += GBDataSize[j];
							}
						/***
						gb->CreateLayerRecords(y, nlrecs);
						***/
						char *lbuf = (char *)malloc((nlrecs + 4) * sumsz);		// F2: hack for per pixel processing
						char *origlbuf = lbuf;									// F2: hack for per pixel processing
						//char *lbuf = (char *)malloc((nlrecs + 4) * maxgbsz);
						if (!rla_rdr.ReadNChannels((UBYTE*)lbuf, 2, nlrecs, TRUE))	 //Read X values
							goto bailout;
						lbuf += 2 * nlrecs;		// F2: hack for per pixel processing
						/***
						gb->SetLayerChannel(y, -1, lbuf);  // set array of X values
						***/
						for (int i = 0; i < NUMGBCHAN; i++)
							{
							// DS 10/1/99: R3 didn't write out the weight channel correctly for the layers:
							// oldver indicates R3 wrote the file.
							if (i == GB_WEIGHT && oldver)
								if (!gbChan[i]) continue;
							if (gbChannels&(1<<i))
								{
							//if (gbChan[i]) {
								sz = GBDataSize[i];
								if (!rla_rdr.ReadNChannels((UBYTE*)lbuf, sz, nlrecs, TRUE))	 //AAAA
									goto bailout;
								lbuf += sz * nlrecs;
								/***
								gb->SetLayerChannel(y, i, lbuf);
								***/
								}
							} // for i

						// per-fragment compositing processor
						if (LoadCompositer)
							{
							for (int k = 0; k < nlrecs; k++)
								{
								lbuf = origlbuf;
								Samp.X = *((short *)(lbuf + k * 2));
								lbuf += 2 * nlrecs;
								Samp.Y = y;
								Samp.Alpha = 255;
								if (gbChannels & BMM_CHAN_Z)
									{
									lbuf += GBDataSize[GB_Z] * k;
									Samp.Z = -*((float *)lbuf);
									lbuf += GBDataSize[GB_Z] * (nlrecs - k);
									}
								else
									Samp.Z = 0.0f;
								if (gbChannels & BMM_CHAN_MTL_ID)
									{
									lbuf += GBDataSize[GB_MTL_ID] * k;
									Samp.mtl_id = *((unsigned char *)lbuf);
									lbuf += GBDataSize[GB_MTL_ID] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_NODE_ID)
									{
									lbuf += GBDataSize[GB_NODE_ID] * k;
									Samp.node_id = *((unsigned short *)lbuf);
									lbuf += GBDataSize[GB_NODE_ID] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_UV)
									{
									lbuf += GBDataSize[GB_UV] * k;
									Samp.u = *((float *)lbuf);
									Samp.v = *((float *)(lbuf + 4));
									lbuf += GBDataSize[GB_UV] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_NORMAL)
									{
									lbuf += GBDataSize[GB_NORMAL] * k;
									Samp.normal = *((unsigned long *)lbuf);
									lbuf += GBDataSize[GB_NORMAL] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_REALPIX)
									{
									lbuf += GBDataSize[GB_REALPIX] * k;
									//Samp.realpix = *((unsigned long *)lbuf);
									Samp.realpix[0] = *((unsigned char *)lbuf);
									Samp.realpix[1] = *((unsigned char *)(lbuf + 1));
									Samp.realpix[2] = *((unsigned char *)(lbuf + 2));
									Samp.realpix[3] = *((unsigned char *)(lbuf + 3));
									lbuf += GBDataSize[GB_REALPIX] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_COVERAGE)
									{
									lbuf += GBDataSize[GB_COVERAGE] * k;
									Samp.coverage = *((unsigned char *)lbuf);
									lbuf += GBDataSize[GB_COVERAGE] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_BG)
									{
									// ???
									lbuf += GBDataSize[GB_BG] * nlrecs;
									}
								if (gbChannels & BMM_CHAN_NODE_RENDER_ID)
									{
									lbuf += GBDataSize[GB_NODE_RENDER_ID] * k;
									Samp.node_id = *((unsigned short *)lbuf);
									lbuf += GBDataSize[GB_NODE_RENDER_ID] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_COLOR)
									{
									lbuf += GBDataSize[GB_COLOR] * k;
									Samp.color[0] = *((unsigned char *)lbuf);
									Samp.color[1] = *((unsigned char *)(lbuf + 1));
									Samp.color[2] = *((unsigned char *)(lbuf + 2));
									lbuf += GBDataSize[GB_COLOR] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_TRANSP)
									{
									lbuf += GBDataSize[GB_TRANSP] * k;
									Samp.transp[0] = *((unsigned char *)lbuf);
									Samp.transp[1] = *((unsigned char *)(lbuf + 1));
									Samp.transp[2] = *((unsigned char *)(lbuf + 2));
									lbuf += GBDataSize[GB_TRANSP] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_VELOC)
									{
									lbuf += GBDataSize[GB_VELOC] * k;
									Samp.veloc_x = *((float *)lbuf);
									Samp.veloc_y = *((float *)(lbuf + 4));
									lbuf += GBDataSize[GB_VELOC] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_WEIGHT)
									{
									lbuf += GBDataSize[GB_WEIGHT] * k;
									Samp.weight[0] = *((unsigned char *)lbuf);
									Samp.weight[1] = *((unsigned char *)(lbuf + 1));
									Samp.weight[2] = *((unsigned char *)(lbuf + 2));
									lbuf += GBDataSize[GB_WEIGHT] * (nlrecs - k);
									}
								if (gbChannels & BMM_CHAN_MASK)
									{
									lbuf += GBDataSize[GB_MASK] * k;
									Samp.mask = *((unsigned short *)lbuf);
									lbuf += GBDataSize[GB_MASK] * (nlrecs - k);
									}

								LoadCompositer->EvalOneRLASample(&Samp);
								} // for k
							} // if

						lbuf = origlbuf;
						free(lbuf);
						} // if nlrecs > 0
					} // if gotLayerData
				} // if gbChannels


			/***
			if (!s->PutPixels(0,y,x,l64)) goto bailout;
			if (gbufZ) 
				if (!rla.ReadNChannels((BYTE *)&gbufZ[w*y],			4))	goto bailout;
			if (gbufMtlID) 
				if (!rla.ReadNChannels((BYTE *)&gbufMtlID[w*y],		1))	goto bailout;
			if (gbufNodeID) 
				if (!rla.ReadNChannels((BYTE *)&gbufNodeID[w*y],	2))	goto bailout;
			if (gbufUV) 
				if (!rla.ReadNChannels((BYTE *)&gbufUV[w*y],		8))	goto bailout;
			if (gbufNorm) 
				if (!rla.ReadNChannels((BYTE *)&gbufNorm[w*y],		4))	goto bailout;
			if (gbufRealPix)
				if (!rla.ReadNChannels((BYTE *)&gbufRealPix[w*y].r,	4))	goto bailout;
			if (gbufCov)
				if (!rla.ReadNChannels((BYTE *)&gbufCov[w*y],	1))	goto bailout;
			if (gbufBg)
				if (!rla.ReadNChannels((BYTE *)&gbufBg[w*y],	3))	goto bailout;
			***/
			} // for line

		/***
		for (int y=0; y<h; y++) {
			rla.BeginLine(y);
			if (!rla.ReadRGBA(l64))
				goto bailout;
			if (!s->PutPixels(0,y,w,l64)) 
				goto bailout;
			if (gbChannels) {
				for (int i=0; i<NUMGBCHAN; i++) {
					if (gbChan[i]) {
						int sz = GBDataSize(i);
						if (!rla.ReadNChannels(&gbChan[i][w*y*sz], sz))	
							goto bailout;
						}
					}
				if (gotLayerData)
					{
					int nlrecs = rla.ReadNumLRecs();
					if (nlrecs<0) 
						goto bailout;
					if (nlrecs>0) {
						gb->CreateLayerRecords(y,nlrecs);
						char *lbuf = (char *)malloc((nlrecs+4)*maxgbsz);
						if (!rla.ReadNChannels((UBYTE*)lbuf, 2, nlrecs, TRUE))	 //Read X values
							goto bailout;
						gb->SetLayerChannel(y,-1,lbuf);  // set array of X values
						for (int i=0; i<NUMGBCHAN; i++) {
							// DS 10/1/99: R3 didn't write out the weight channel correctly for the layers:
							// oldver indicates R3 wrote the file.
							if (i==GB_WEIGHT&&oldver) 
								if (!gbChan[i]) continue;
							if (gbChannels&(1<<i)) {
							//if (gbChan[i]) {
								int sz = GBDataSize(i);
								if (!rla.ReadNChannels((UBYTE*)lbuf, sz, nlrecs, TRUE))	 //AAAA
									goto bailout;
								gb->SetLayerChannel(y,i,lbuf);
								}
							}
						free(lbuf);
						}
					}
					}
			}
		if (gb)
			gb->UpdateChannelMinMax();
		return s;
		}
		***/

		ByteBands = 3;
		ByteBandSize = rsize_b;
		FloatBands = 4;
		FloatBandSize = rsize_f;
		} // if we have 3 RGB channels

		// we should have copied whatever we needed from these
		if (gbChannels)
			{
			for (int i = 0; i < NUMGBCHAN; i++)
				{
				if ((1 << i) & gbChannels)
					{
					sz = GBDataSize[i];
					AppMem_Free(gbChan[i], w * h * sz);
					gbChan[i] = NULL;
					}
				} // for i
			} // if gbChannels

	} // if file opened & hdr allocated

success = 1;
if (DidComposite)
	LoadCompositer = NULL;

/*** for testing
if (fout)
	{
	float *fptr, fval;
	long i;
	unsigned char val;
	fptr = FloatMap[WCS_RASTER_IMAGE_FBAND_RED];
	for (i = 0; i < Cols * Rows; i++, fptr++)
		{
		fval = *fptr * 255.0f;
		if (fval > 255.0f)
			fval = 255.0f;
		val = (unsigned char)fval;
		fwrite(&val, 1, 1, fout);
		}
	fptr = FloatMap[WCS_RASTER_IMAGE_FBAND_GREEN];
	for (i = 0; i < Cols * Rows; i++, fptr++)
		{
		fval = *fptr * 255.0f;
		if (fval > 255.0f)
			fval = 255.0f;
		val = (unsigned char)fval;
		fwrite(&val, 1, 1, fout);
		}
	fptr = FloatMap[WCS_RASTER_IMAGE_FBAND_BLUE];
	for (i = 0; i < Cols * Rows; i++, fptr++)
		{
		fval = *fptr * 255.0f;
		if (fval > 255.0f)
			fval = 255.0f;
		val = (unsigned char)fval;
		fwrite(&val, 1, 1, fout);
		}
	fclose(fout);
	}
***/

bailout:
if (line64)
	AppMem_Free(line64, sizeof(Max_Color_64) * Cols);
if (rla_rdr)
	delete rla_rdr;
if (hdr)
	AppMem_Free(hdr, sizeof(RLAHeader));

if (fin)
	{
	fclose(fin);
	fin = NULL;
	} // if

return success;

} // Raster::LoadRPF_RLA

/*===========================================================================*/

#ifdef WCS_BUILD_JPEG_SUPPORT

/*
 * Here's the routine that will replace the standard error_exit method:
 */

struct load_my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct load_my_error_mgr * my_error_ptr;

METHODDEF(void)
my_JPEGLOADER_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a load_my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1); //lint !e115
}

short Raster::LoadJPG(char *Name, short SupressWng, ImageCacheControl *ICC)
{
unsigned char Success = 0, *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf;
unsigned long InScan, PixelCol;
struct jpeg_decompress_struct cinfo;
struct load_my_error_mgr jerr;
struct ArcWorldInfo awi;
int row_stride = 0;
int JPEG_REALLY_NO_ERROR = 0;

FILE *fh = NULL;
#ifdef WCS_BUILD_GEOTIFF_SUPPORT
// We get JPG Worldfile support with GeoTIFF
int WFValid = 0, WFLen, FlipLon = 0;
#endif // WCS_BUILD_GEOTIFF_SUPPORT
int LoadingSubTile = 0;
long LoadOffsetX = 0, LoadOffsetY = 0;

if (ICC)
	{
	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
	ICC->QueryOnlyFlags = NULL; // clear all flags
	ICC->QueryOnlyFlags = WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE; // even stripped reports as short wide tiles
	LoadingSubTile = ICC->LoadingSubTile;
	#else // !WCS_IMAGE_MANAGEMENT_JPEG
	ICC->QueryOnlyFlags = NULL; // clear all flags
	#endif // !WCS_IMAGE_MANAGEMENT_JPEG
	} // if

if (fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/))
	{
#ifdef WCS_BUILD_GEOTIFF_SUPPORT

	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
	if (!LoadingSubTile)
	#endif // WCS_IMAGE_MANAGEMENT_JPEG
		{
		WFValid = ReadWorldFile(awi, Name);
		} // if

#endif // WCS_BUILD_GEOTIFF_SUPPORT

	cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);

	jerr.pub.error_exit = my_JPEGLOADER_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer))
		{
		#ifdef WCS_IMAGE_MANAGEMENT_JPEG
		// this ugly hack is here to work around the JPEG library's mistaken error
		// reporting when Image Management doesn't load the file initially.
		if (JPEG_REALLY_NO_ERROR)
			{
			goto JPEG_NO_ERROR_RESUME; // GOTO HELL. GOTO DIRECTLY TO HELL. DO NOT USE GOTO.
			} // if
		#endif // WCS_IMAGE_MANAGEMENT_JPEG
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		if (InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
		if (!Success)
			{
			if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
			if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
			if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
			} // if

		jpeg_destroy_decompress(&cinfo);
		fclose(fh);
		return(Success);
		} // if

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fh);
	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	cinfo.dct_method = JDCT_FLOAT;
	jpeg_start_decompress(&cinfo);

	Cols = cinfo.output_width;
	Rows = cinfo.output_height;

	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
		if (ICC)
			{
			if (ICC->LoadingSubTile)
				{ // loading a subtile, don't fiddle with settings
				Rows = ICC->LoadHeight;
				Cols = ICC->LoadWidth;
				LoadOffsetX = ICC->LoadXOri;
				LoadOffsetY = ICC->LoadYOri;
				} // if
			else
				{ // loading a whole file, pre-set settings
				unsigned long int AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_THRESH; // use higher AutoEnable threshold by default
				if(WFValid) AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_GEO_THRESH; // use lower threshold if we know it's georeferenced
				if ((cinfo.output_width >= AutoEnableThreshold) || (cinfo.output_height >= AutoEnableThreshold))
					{
					ICC->ImageManagementEnable = 1;
					} // if
				} // else
			} // if
	#endif // WCS_IMAGE_MANAGEMENT_JPEG

	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
	if (!(ICC && ICC->ImageManagementEnable))
	#endif // WCS_IMAGE_MANAGEMENT_JPEG
		{
		ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "JPG Loader Bitmaps");
		ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "JPG Loader Bitmaps");
		ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "JPG Loader Bitmaps");
		} // if

	row_stride = cinfo.output_width * 3;
	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
	if (!(ICC && ICC->ImageManagementEnable))
	#endif // WCS_IMAGE_MANAGEMENT_JPEG
		{
		InterleaveBuf = (UBYTE *)AppMem_Alloc(row_stride, 0, "JPG DeInterleave Buffer");
		if (InterleaveBuf && ByteMap[0] && ByteMap[1] && ByteMap[2]) // everything ok?
			{
			// Clear bitmaps
			#ifdef WCS_IMAGE_MANAGEMENT_JPEG
			if (!(ICC && ICC->ImageManagementEnable))
			#endif // WCS_IMAGE_MANAGEMENT_JPEG
				{
				memset(ByteMap[0], 0, Cols * Rows);
				memset(ByteMap[1], 0, Cols * Rows);
				memset(ByteMap[2], 0, Cols * Rows);
				} // if
			} // if
		} // if

	#ifdef WCS_IMAGE_MANAGEMENT_JPEG
	if (!(ICC && ICC->ImageManagementEnable)) // skip loading the real image if we've autoswitched IM on
	#else // WCS_IMAGE_MANAGEMENT_JPEG
	if (1)
	#endif // !WCS_IMAGE_MANAGEMENT_JPEG
		{
		while (cinfo.output_scanline < cinfo.output_height)
			{
			#ifdef WCS_IMAGE_MANAGEMENT_JPEG
			// do we even need this strip?
			if (ICC && ICC->LoadingSubTile)
				{
				if ((cinfo.output_scanline < (unsigned)LoadOffsetY) || (cinfo.output_scanline >= (unsigned)LoadOffsetY + Rows))
					{ // don't need it at all, skip it
					//cinfo.output_scanline++;
					if (cinfo.output_scanline >= (unsigned)(LoadOffsetY + Rows))
						{ // overshot, end processing
						cinfo.output_scanline = cinfo.output_height;
						break;
						} // if
					jpeg_read_scanlines(&cinfo, &InterleaveBuf, 1);
					continue;
					} // if

				// setup for row to tile decoding accounting for Y offset
				RBuf = &ByteMap[0][(cinfo.output_scanline - LoadOffsetY) * Cols];
				if (ByteMap[1]) GBuf = &ByteMap[1][(cinfo.output_scanline - LoadOffsetY) * Cols];
				if (ByteMap[2]) BBuf = &ByteMap[2][(cinfo.output_scanline - LoadOffsetY) * Cols];
				} // if
			else
			#endif // WCS_IMAGE_MANAGEMENT_JPEG
				{ // normal row to image decoding
				RBuf = &ByteMap[0][cinfo.output_scanline * Cols];
				GBuf = &ByteMap[1][cinfo.output_scanline * Cols];
				BBuf = &ByteMap[2][cinfo.output_scanline * Cols];
				} // else
			cinfo.input_scan_number = cinfo.output_scan_number = cinfo.output_scanline;
			if (jpeg_read_scanlines(&cinfo, &InterleaveBuf, 1) != 1)
				{
				jpeg_abort_decompress(&cinfo);
				break;
				} // if
			else
				{ // deinterleave
				for (InScan = PixelCol = 0; PixelCol < (unsigned)cinfo.output_width; PixelCol++)
					{
					#ifdef WCS_IMAGE_MANAGEMENT_JPEG
					if ((PixelCol < (unsigned)(LoadOffsetX + Cols)) && (PixelCol >= (unsigned)(LoadOffsetX)))
					#else // !WCS_IMAGE_MANAGEMENT_JPEG
					if (1) // always within range in absence of Image Management
					#endif // !WCS_IMAGE_MANAGEMENT_JPEG
						{
						RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
						GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
						BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
						} // if
					else
						{
						InScan += 3; // skip pixel
						} // 
					} // for
				} // else
			} // while
		} // if
	else
		{ // fake success without loading anything
		Success = 1;
		JPEG_REALLY_NO_ERROR = 1; // this next line will throw a (bogus) error, and then we'll detect it and GOTO back here.
		jpeg_abort_decompress(&cinfo);
		jpeg_finish_decompress(&cinfo);
		} // else

	if (cinfo.output_scanline == cinfo.output_height)
		{
		Success = 1;
		jpeg_finish_decompress(&cinfo);
		} // if

JPEG_NO_ERROR_RESUME:

	jpeg_destroy_decompress(&cinfo);

	if (InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
	if (!Success)
		{
		if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
		if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
		if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
		} // if
	fclose(fh);
	} // if

#ifdef WCS_BUILD_GEOTIFF_SUPPORT
if (Success && WFValid)
	{
	if (LoaderGeoRefShell = new GeoRefShell)
		{
		CoordSys *TempCS;
		double TempNorth = 1.0, TempSouth = 0.0, TempEast = 1.0, TempWest = 0.0;
		int IsUTM = 1;

		LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS; // typical in GIS imagery like (Geo)TIFF

		TempEast = (awi.originX + ((Cols - 1) * awi.cellSizeX));
		TempSouth = (awi.originY + ((Rows - 1)* awi.cellSizeY));

		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(TempNorth = awi.originY);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(TempWest = awi.originX);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(TempEast);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(TempSouth);

		// check to see if all bounds are within geographic limits -- unlikely for a UTM/stateplane
		if (TempNorth <= 90.0 && TempSouth >= -90.0 && fabs(TempEast) <= 360.0 && fabs(TempWest) <= 360.0)
			{
			// probably a WCS geographic
			IsUTM = 0;
			} // if
		// Set up a coordsys
		if (LoaderGeoRefShell->Host = TempCS = new CoordSys())
			{
			FILE *WorldFileHandle = NULL;
			int PrjGood = 0;
			char WorldFileName[1024];

			// Now try for PRJ file
			strcpy(WorldFileName, Name);
			WFLen = (int)strlen(WorldFileName);
			if ((WFLen > 4) && (WorldFileName[WFLen - 4] == '.'))
				{
				// modify TLA extension to .PRJ File
				WorldFileName[WFLen - 3] = 'p';
				WorldFileName[WFLen - 2] = 'r';
				WorldFileName[WFLen - 1] = 'j';
				if (WorldFileHandle = PROJ_fopen(WorldFileName, "r"))
					{
					if (LoaderGeoRefShell)
						{
						CSLoadInfo *CSLI = NULL;

						if (CSLI = TempCS->LoadFromArcPrj(WorldFileHandle))
							{
							PrjGood = 1;
							delete CSLI;
							CSLI = NULL;
							} // if
						} // if
					fclose(WorldFileHandle);
					WorldFileHandle = NULL;
					} // if
				} // if
			if (!PrjGood)
				{
				if (IsUTM)
					{ // guess at UTM
					TempCS->SetSystem("UTM - NAD 27");
					} // if
				else
					{
					// if geographic, we ought to negate the sign of the east and west bounds
					// World file stores GIS PosEast notation
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
					} // else
				} // if
			else
				{
				if (TempCS && TempCS->GetGeographic()) // are we GIS-style geographic notation
					{
					// if geographic, we ought to negate the sign of the east and west bounds
					// World file stores GIS PosEast notation
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
					} // if
				} // else
			} // if
		} // if
	} // if
#endif // WCS_BUILD_GEOTIFF_SUPPORT


if (Success)
	{
	//*NewPlanes = 24;
	ByteBands = 3;
	ByteBandSize = Rows * Cols;
	AlphaAvailable = 0;
	AlphaEnabled = 0;
	} // if

return(Success);

} // Raster::LoadJPG()
#endif // WCS_BUILD_JPEG_SUPPORT

/*===========================================================================*/

// TIFF support is now in ImageFormatTIFFs.cpp

// ECW support is now in ImageFormatECW.cpp

// MrSID support is now in ImageFormatMRSID.cpp

/*===========================================================================*/

#ifdef WCS_BUILD_PNG_SUPPORT
short Raster::LoadPNG(char *Name, short SupressWng, ImageCacheControl *ICC)
{
unsigned char AllIsWell = 0, Success = 0,
 *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf, *ABuf;
unsigned long InScan, PixelCol, WorkRow;
int row_stride = 0;
png_structp png_ptr;
png_infop info_ptr, end_info;
png_uint_32 width, height;
int bit_depth, color_type, interlace_type, number_of_passes = 1, Pass;
unsigned char DoGrey = 0, DoAlpha = 0, AlphaFailed = 0, BytesPerPixel;
unsigned char *Alpha = NULL;

FILE *fh = NULL;

if (ICC) ICC->QueryOnlyFlags = NULL; // clear all flags

if (fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/))
	{
	if (png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL))
		{
		if (info_ptr = png_create_info_struct(png_ptr))
			{
		    if (end_info = png_create_info_struct(png_ptr))
				{
				AllIsWell = 1;
				} // if
			else
				{
		        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			    } // else
			} // if
		else
			{
	        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			} // else
		} // if
	if (!AllIsWell)
		{
		fclose(fh); fh = NULL;
		return(0);
		} // if

	/* Establish the error handler */
    if (setjmp(png_jmpbuf(png_ptr)))
		{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		// If we get here, the PNG code has signaled an error.
		// We need to clean up the PNG object, close the input file, and return.
		if (InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
		if (!Success)
			{
			if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
			if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
			if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
			if (Alpha) AppMem_Free(Alpha, Cols * Rows); Alpha = NULL;
			// There's a remote possibility that we could misplace an Alpha buffer here if we fail, because
			// at this stage we can't tell if we allocated it ourself or if it was provided.
			} // if

		fclose(fh);
		return(0);
		}


    png_init_io(png_ptr, fh);
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,
       NULL, NULL);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY &&
        bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr,
        PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (color_type & PNG_COLOR_MASK_ALPHA)
		{
        DoAlpha = 1;
		BytesPerPixel = 4;
		} // if
	else
		{
		DoAlpha = 0;
		BytesPerPixel = 3;
		} // else

    if (interlace_type == PNG_INTERLACE_ADAM7)
        {
		number_of_passes = png_set_interlace_handling(png_ptr);
		//GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Interlaced PNG currently not supported.");
		//AllIsWell = 0;
		} // if

    // retire this in favor of direct Grey loading
	if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
		//png_set_gray_to_rgb(png_ptr);
		DoGrey = 1;
		} // if

    if (AllIsWell)
		{
		png_read_update_info(png_ptr, info_ptr);

		Cols = width;
		Rows = height;
		ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "PNG Loader Bitmaps");
		if (!DoGrey)
			{
			ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "PNG Loader Bitmaps");
			ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "PNG Loader Bitmaps");
			} // if
		if (DoAlpha && (Alpha == NULL))
			{
			if (!(Alpha = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "PNG Loader Alpha Bitmaps")))
				{
				AlphaFailed = 1;
				} // if
			} // if

		row_stride = width * BytesPerPixel;
		InterleaveBuf = (UBYTE *)AppMem_Alloc(row_stride, 0, "PNG DeInterleave Buffer");
		if (InterleaveBuf && ByteMap[0] && (DoGrey || (ByteMap[1] && ByteMap[2])) && !AlphaFailed) // everything ok?
			{
			// Clear bitmaps
			memset(ByteMap[0], 0, Cols * Rows);
			if (ByteMap[1]) memset(ByteMap[1], 0, Cols * Rows);
			if (ByteMap[2]) memset(ByteMap[2], 0, Cols * Rows);

			if (DoAlpha && Alpha)
				{
				memset(Alpha, 0, Cols * Rows);
				} // if

			for (Pass = 0; Pass < number_of_passes; Pass++)
				{
				for (WorkRow = 0; WorkRow < height; WorkRow++)
					{
					RBuf = &ByteMap[0][WorkRow * Cols];
					if (!DoGrey)
						{
						GBuf = &ByteMap[1][WorkRow * Cols];
						BBuf = &ByteMap[2][WorkRow * Cols];
						} // if
					if (DoAlpha) ABuf = &(Alpha)[WorkRow * Cols];
					if (number_of_passes != 1)
						{
						// interleave requires read-in and interleaving from our
						// buffers into the InterleaveBuf so the png code can add
						// to exisitng data
						for (InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
							{
							InterleaveBuf[InScan++] = RBuf[PixelCol];
							if (!DoGrey)
								{
								InterleaveBuf[InScan++] = GBuf[PixelCol];
								InterleaveBuf[InScan++] = BBuf[PixelCol];
								} // if
							if (DoAlpha) InterleaveBuf[InScan++] = ABuf[PixelCol];
							} // for
						} // if
					png_read_row(png_ptr, InterleaveBuf, NULL);
					for (InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
						{
						RBuf[PixelCol] = InterleaveBuf[InScan++];
						if (!DoGrey)
							{
							GBuf[PixelCol] = InterleaveBuf[InScan++];
							BBuf[PixelCol] = InterleaveBuf[InScan++];
							} // if
						if (DoAlpha) ABuf[PixelCol] = InterleaveBuf[InScan++];
						} // for
					} // for
				} // for

			if ((WorkRow == height) && (Pass == number_of_passes))
				{
				Success = 1;
				png_read_end(png_ptr, end_info);
				} // if
			} // if

		} // if

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	if (InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
	if (!Success)
		{
		if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
		if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
		if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
		if (Alpha) AppMem_Free(Alpha, Cols * Rows); Alpha = NULL;
		} // if
 
	fclose(fh);
	} // if

if (Success)
	{
	//*NewPlanes = 24;
	if (DoGrey)
		{
		ByteBands = 1;
		} // if
	else
		{
		ByteBands = 3;
		} // else
	ByteBandSize = Rows * Cols;
	if (DoAlpha && Alpha)
		{
		AlphaAvailable = 1;
		if (AlphaEnabled)
			{
			CopyAlphaToCoverage(Alpha);
			} // if
		AppMem_Free(Alpha, Cols * Rows);
		Alpha = NULL;
		} // if
	else
		{
		AlphaAvailable = 0;
		AlphaEnabled = 0;
		} // else
	} // if

return(Success);
} // Raster::LoadPNG()
#endif // WCS_BUILD_PNG_SUPPORT

/*===========================================================================*/


short Raster::LoadBMP(char *Name, short SupressWng, ImageCacheControl *ICC)
{
struct ArcWorldInfo awi;
unsigned char IDBUF[8], FlipVert = 0, Success = 0,
 *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf;
unsigned long BodyOffset, HeaderSize, Compression = 0, WorkRow,
 InScan, PixelCol;
signed long FWidth, FHeight;
signed short FWidthSixteen, FHeightSixteen;
unsigned short FPlanes, FBPP;
int PrjGood = 0;
FILE *fh = NULL;
int LoadingSubTile = 0;
long LoadOffsetX = 0, LoadOffsetY = 0;

#ifdef WCS_BUILD_GEOTIFF_SUPPORT
// We get BMP Worldfile support with GeoTIFF
char WorldFileName[1024];
FILE *WorldFileHandle = NULL;
int WFValid = 0, WFLen, FlipLon = 0;
#endif // WCS_BUILD_GEOTIFF_SUPPORT

if (ICC)
	{
	#ifdef WCS_IMAGE_MANAGEMENT
	//ICC->QueryOnlyFlags = NULL; // clear all flags
	ICC->QueryOnlyFlags = WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE; // even stripped reports as short wide tiles
	LoadingSubTile = ICC->LoadingSubTile;
	#else // !WCS_IMAGE_MANAGEMENT
	ICC->QueryOnlyFlags = NULL; // clear all flags
	#endif // !WCS_IMAGE_MANAGEMENT
	} // if


if (fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/))
	{
#ifdef WCS_BUILD_GEOTIFF_SUPPORT
	#ifdef WCS_IMAGE_MANAGEMENT
	if (!LoadingSubTile)
	#endif // WCS_IMAGE_MANAGEMENT
		{
		WFValid = ReadWorldFile(awi, Name);
		} // if !LoadingSubTile
#endif // WCS_BUILD_GEOTIFF_SUPPORT


	memset(IDBUF, 0, sizeof(IDBUF));
	fread(IDBUF, 1, 2, fh);
	if ((IDBUF[0] == 'B') && (IDBUF[1] == 'M'))
		{
		fread(IDBUF, 1, 8, fh); // skip filesize, reserved1 & reserved2
		if (fread(&BodyOffset, 1, 4, fh) == 4)
			{
			#ifdef BYTEORDER_BIGENDIAN
			SimpleEndianFlip32U(BodyOffset, &BodyOffset);
			#endif // BYTEORDER_BIGENDIAN
			if (fread(&HeaderSize, 1, 4, fh) == 4)
				{
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip32U(HeaderSize, &HeaderSize);
				#endif // BYTEORDER_BIGENDIAN

				if (HeaderSize > 12)
					{
					fread(&FWidth, 1, 4, fh);
					fread(&FHeight, 1, 4, fh);
					#ifdef BYTEORDER_BIGENDIAN
					SimpleEndianFlip32S(FWidth, &FWidth);
					SimpleEndianFlip32S(FHeight, &FHeight);
					#endif // BYTEORDER_BIGENDIAN
					} // if
				else
					{
					fread(&FWidthSixteen, 1, 2, fh);
					fread(&FHeightSixteen, 1, 2, fh);
					#ifdef BYTEORDER_BIGENDIAN
					SimpleEndianFlip16S(FWidthSixteen, &FWidthSixteen);
					SimpleEndianFlip16S(FHeightSixteen, &FHeightSixteen);
					#endif // BYTEORDER_BIGENDIAN
					FWidth = FWidthSixteen;
					FHeight = FHeightSixteen;
					} // else

				fread(&FPlanes, 1, 2, fh);
				fread(&FBPP, 1, 2, fh);
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip16U(FPlanes, &FPlanes);
				SimpleEndianFlip16U(FBPP, &FBPP);
				#endif // BYTEORDER_BIGENDIAN

				if (HeaderSize > 12)
					{
					if (fread(&Compression, 1, 4, fh) == 4)
						{
						#ifdef BYTEORDER_BIGENDIAN
						SimpleEndianFlip32U(Compression, &Compression);
						#endif // BYTEORDER_BIGENDIAN
						} // if
					} // if
				if ((Compression == 0) && (FBPP == 24) && (FPlanes == 1))
					{ // There are no supported compression methods for 24-bit, and we only do 24-bit/1-plane
					if (FHeight < 0)
						{ // top-down BMP?
						FHeight = 0 - FHeight; // Make height positive
						} // if
					else
						{
						FlipVert = 1;
						} // else
					fseek(fh, BodyOffset, SEEK_SET);
					if ((unsigned)ftell(fh) == BodyOffset) // did we land where we were supposed to?
						{
						Cols = FWidth;
						Rows = FHeight;

					#ifdef WCS_IMAGE_MANAGEMENT
						if (ICC)
							{
							if (ICC->LoadingSubTile)
								{ // loading a subtile, don't fiddle with settings
								Rows = ICC->LoadHeight;
								Cols = ICC->LoadWidth;
								LoadOffsetX = ICC->LoadXOri;
								LoadOffsetY = ICC->LoadYOri;
								} // if
							else
								{ // loading a whole file, pre-set settings
								signed long int AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_THRESH; // use higher AutoEnable threshold by default
								if(WFValid) AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_GEO_THRESH; // use lower threshold if we know it's georeferenced
								if ((FWidth >= AutoEnableThreshold) || (FHeight >= AutoEnableThreshold))
									{
									// autoswitch on
									ICC->ImageManagementEnable = 1;
									} // if
								} // else
							} // if
					#endif // WCS_IMAGE_MANAGEMENT

						#ifdef WCS_IMAGE_MANAGEMENT
						if (!(ICC && ICC->ImageManagementEnable))
						#endif // WCS_IMAGE_MANAGEMENT
							{
							ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "BMP Loader Bitmaps");
							ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "BMP Loader Bitmaps");
							ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "BMP Loader Bitmaps");
							} // if-ish

						
						#ifdef WCS_IMAGE_MANAGEMENT
						if (!(ICC && ICC->ImageManagementEnable))
						#endif // WCS_IMAGE_MANAGEMENT
							{
							InterleaveBuf = (UBYTE *)AppMem_Alloc(ROUNDUP((FWidth * 3), 4), 0, "BMP DeInterleave Buffer");
							if (ByteMap[0] && ByteMap[1] && ByteMap[2] && InterleaveBuf) // everything ok?
								{
								// Clear bitmaps
								#ifdef WCS_IMAGE_MANAGEMENT
								if (!(ICC && ICC->ImageManagementEnable))
								#endif // WCS_IMAGE_MANAGEMENT
									{
									memset(ByteMap[0], 0, Cols * Rows);
									memset(ByteMap[1], 0, Cols * Rows);
									memset(ByteMap[2], 0, Cols * Rows);
									} // if

								// Start reading the pixels
								#ifdef WCS_IMAGE_MANAGEMENT
								if (!(ICC && ICC->ImageManagementEnable)) // skip loading the real image if we've autoswitched IM on
								#else // WCS_IMAGE_MANAGEMENT
								if (1)
								#endif // !WCS_IMAGE_MANAGEMENT
									{
									unsigned long ImageDataSeek;
									ImageDataSeek = ftell(fh);
									for (WorkRow = 0; WorkRow < (unsigned)FHeight; WorkRow++)
										{
										#ifdef WCS_IMAGE_MANAGEMENT
										int RealRow;
										// do we even need this strip?
										if (ICC && ICC->LoadingSubTile)
											{
											if (FlipVert) RealRow = (FHeight - 1) - WorkRow;
											else RealRow = WorkRow;
											if ((RealRow < LoadOffsetY) || (RealRow >= LoadOffsetY + Rows))
												{ // don't need it at all, skip it
												if (FlipVert)
													{
													if (RealRow < LoadOffsetY)
														{ // undershot, end processing
														WorkRow = FHeight;
														break;
														} // if
													} // if
												else
													{
													if (RealRow >= LoadOffsetY + Rows)
														{ // overshot, end processing
														WorkRow = FHeight;
														break;
														} // if
													} // else
												//fseek(fh, ROUNDUP((FWidth * 3), 4), SEEK_CUR); // skip over the data for this scan line
												continue; // to next WorkRow
												} // if

											// setup for row to tile decoding accounting for Y offset
											// Set up pointers to indicate R G and B destination buffers for
											// either top-down or bottom-up behavior
											fseek(fh, ImageDataSeek + (WorkRow * ROUNDUP((FWidth * 3), 4)), SEEK_SET); // skip to the data for this scanline
											if (FlipVert)
												{ // bottom-up
												RBuf = &ByteMap[0][(RealRow - LoadOffsetY) * Cols];
												GBuf = &ByteMap[1][(RealRow - LoadOffsetY) * Cols];
												BBuf = &ByteMap[2][(RealRow - LoadOffsetY) * Cols];
/*
												// validate pointer insanity
												if ((&RBuf[0] < &ByteMap[0][0]) || (&RBuf[Cols] > &ByteMap[0][Rows * Cols]))
													FlipVert = 1;
												if ((&GBuf[0] < &ByteMap[1][0]) || (&GBuf[Cols] > &ByteMap[1][Rows * Cols]))
													FlipVert = 1;
												if ((&BBuf[0] < &ByteMap[2][0]) || (&BBuf[Cols] > &ByteMap[2][Rows * Cols]))
													FlipVert = 1;
*/
												//RBuf = &ByteMap[0][((Rows - 1) - WorkRow) * Cols];
												//GBuf = &ByteMap[1][((Rows - 1) - WorkRow) * Cols];
												//BBuf = &ByteMap[2][((Rows - 1) - WorkRow) * Cols];
												} // if
											else
												{ // top-down
												RBuf = &ByteMap[0][(WorkRow - LoadOffsetY) * Cols];
												GBuf = &ByteMap[1][(WorkRow - LoadOffsetY) * Cols];
												BBuf = &ByteMap[2][(WorkRow - LoadOffsetY) * Cols];
												//RBuf = &ByteMap[0][WorkRow * Cols];
												//GBuf = &ByteMap[1][WorkRow * Cols];
												//BBuf = &ByteMap[2][WorkRow * Cols];
												} // if
											} // if
										else
										#endif // WCS_IMAGE_MANAGEMENT
											{ // normal row to image decoding
											// Set up pointers to indicate R G and B destination buffers for
											// either top-down or bottom-up behavior
											if (FlipVert)
												{ // bottom-up
												RBuf = &ByteMap[0][((Rows - 1) - WorkRow) * Cols];
												GBuf = &ByteMap[1][((Rows - 1) - WorkRow) * Cols];
												BBuf = &ByteMap[2][((Rows - 1) - WorkRow) * Cols];
												} // if
											else
												{ // top-down
												RBuf = &ByteMap[0][WorkRow * Cols];
												GBuf = &ByteMap[1][WorkRow * Cols];
												BBuf = &ByteMap[2][WorkRow * Cols];
												} // if
											} // else normal row decoding
										if (fread(InterleaveBuf, 1, ROUNDUP((FWidth * 3), 4), fh) == (unsigned)ROUNDUP((FWidth * 3), 4))
										//if (1)
											{
											// DeInterleave the BGR data into the apropriate R, G and B planes
											for (InScan = PixelCol = 0; PixelCol < (unsigned)FWidth; PixelCol++)
												{
												#ifdef WCS_IMAGE_MANAGEMENT
												if ((PixelCol < (unsigned)(LoadOffsetX + Cols)) && (PixelCol >= (unsigned)LoadOffsetX))
												//if (0)
												#else // !WCS_IMAGE_MANAGEMENT
												if (1) // always within range in absence of Image Management
												#endif // !WCS_IMAGE_MANAGEMENT
													{
													BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
													GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
													RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
/*
													if ((&RBuf[PixelCol - LoadOffsetX] < &ByteMap[0][0]) || (&RBuf[PixelCol - LoadOffsetX] > &ByteMap[0][Rows * Cols]))
														FlipVert = 1;
													if ((&GBuf[PixelCol - LoadOffsetX] < &ByteMap[1][0]) || (&GBuf[PixelCol - LoadOffsetX] > &ByteMap[1][Rows * Cols]))
														FlipVert = 1;
													if ((&BBuf[PixelCol - LoadOffsetX] < &ByteMap[2][0]) || (&BBuf[PixelCol - LoadOffsetX] > &ByteMap[2][Rows * Cols]))
														FlipVert = 1;
*/
													} // if
												else
													{
													InScan += 3; // skip pixel
													} // else
												} // for
											} // if
										else
											{
											GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "End of file while reading BMP image data.");
											break;
											} // else
										} // for
									if (WorkRow == (unsigned)FHeight) // Did we make it all the way through the image?
										{
										Success = 1;
										} // if
									} // if
								else
									{ // fake success without loading anything
									Success = 1;
									} // else
								} // if
							if (InterleaveBuf) AppMem_Free(InterleaveBuf, ROUNDUP((FWidth * 3), 4)); InterleaveBuf = NULL;
							} // if
#ifdef WCS_IMAGE_MANAGEMENT
						else
							{ // fake success without loading anything
							Success = 1;
							} // else
#endif // WCS_IMAGE_MANAGEMENT
						if (!Success)
							{
							if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
							if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
							if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
							} // if
						} // if
					else
						{
						GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "File too short.");
						} // else
					} // if
				else
					{
					GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, APP_TLA" BMP loader only supports 24-bit uncompressed BMP.");
					} // else
				} // if
			} // if
		} // if
	if (!Success)
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Image load aborted.");
		} // if
	fclose(fh);
	} // if

#ifdef WCS_BUILD_GEOTIFF_SUPPORT
// World file support
if (Success && WFValid)
	{
	if (LoaderGeoRefShell = new GeoRefShell)
		{
		CoordSys *TempCS;
		double TempNorth = 1.0, TempSouth = 0.0, TempEast = 1.0, TempWest = 0.0;
		int IsUTM = 1;

		LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS; // typical in GIS imagery like (Geo)TIFF

		TempEast = (awi.originX + ((Cols - 1) * awi.cellSizeX));
		TempSouth = (awi.originY + ((Rows - 1)* awi.cellSizeY));

		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(TempNorth = awi.originY);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(TempWest = awi.originX);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(TempEast);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(TempSouth);
		// check to see if all bounds are within geographic limits -- unlikely for a UTM/stateplane
		if (TempNorth <= 90.0 && TempSouth >= -90.0 && fabs(TempEast) <= 360.0 && fabs(TempWest) <= 360.0)
			{
			// probably a WCS geographic
			IsUTM = 0;
			} // if
		// Set up a coordsys
		if (LoaderGeoRefShell->Host = TempCS = new CoordSys())
			{
			// Now try for PRJ file
			strcpy(WorldFileName, Name);
			WFLen = (int)strlen(WorldFileName);
			if ((WFLen > 4) && (WorldFileName[WFLen - 4] == '.'))
				{
				// modify TLA extension to .PRJ File
				WorldFileName[WFLen - 3] = 'p';
				WorldFileName[WFLen - 2] = 'r';
				WorldFileName[WFLen - 1] = 'j';
				if (WorldFileHandle = PROJ_fopen(WorldFileName, "r"))
					{
					if (!LoaderGeoRefShell) LoaderGeoRefShell = new GeoRefShell;
					if (LoaderGeoRefShell)
						{
						CSLoadInfo *CSLI = NULL;

						if (CSLI = TempCS->LoadFromArcPrj(WorldFileHandle))
							{
							PrjGood = 1;
							delete CSLI;
							CSLI = NULL;
							} // if
						} // if
					fclose(WorldFileHandle);
					WorldFileHandle = NULL;
					} // if
				} // if

			if (!PrjGood)
				{
				if (IsUTM)
					{ // guess at UTM
					TempCS->SetSystem("UTM - NAD 27");
					} // if
				else
					{
					// if geographic, we ought to negate the sign of the east and west bounds
					// World file stores GIS PosEast notation
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
					} // else
				} // if
			else
				{
				if (TempCS && TempCS->GetGeographic()) // are we GIS-style geographic notation?
					{
					// if geographic, we ought to negate the sign of the east and west bounds
					// World file stores GIS PosEast notation
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
					} // if
				} // else
			} // if
		} // if
	} // if
#endif // WCS_BUILD_GEOTIFF_SUPPORT


if (Success)
	{
	//*NewPlanes = 24;
	ByteBands = 3;
	ByteBandSize = Rows * Cols;
	AlphaAvailable = 0;
	AlphaEnabled = 0;
	} // if

return(Success);

} // Raster::LoadBMP()

/*===========================================================================*/

short Raster::LoadPICT(char *Name, short SupressWng)
{
ImLoaderLocal ILL;
short Success = 0;

ILL.filename = Name;
ILL.result = PICT_IPSTAT_OK;
ILL.priv_data = NULL;
ILL.begin = PICTHostBegin;
ILL.done = PICTHostDone;
ILL.monitor = NULL;

Loaddat.DoAlpha = 0;
Loaddat.Abuf = NULL;
Loaddat.d_width  = 0;
Loaddat.d_height = 0;
Loaddat.d_depth  = 0;
Loaddat.Rbuf = NULL;
Loaddat.Gbuf = NULL;
Loaddat.Bbuf = NULL;

PICTLoader(&ILL);

if (ILL.result != PICT_IPSTAT_OK) return(Success = 0);

if (Loaddat.Rbuf && Loaddat.Gbuf && Loaddat.Bbuf)
	{
	ByteMap[0] = Loaddat.Rbuf;
	ByteMap[1] = Loaddat.Gbuf;
	ByteMap[2] = Loaddat.Bbuf;
	Cols = Loaddat.d_width;
	Rows = Loaddat.d_height;
	ByteBands = 3;
	ByteBandSize = Rows * Cols;

	if (Loaddat.Abuf)
		{
		AlphaAvailable = 1;
		if (AlphaEnabled)
			{
			CopyAlphaToCoverage(Loaddat.Abuf);
			} // if
		AppMem_Free(Loaddat.Abuf, Cols * Rows);
		Loaddat.Abuf = NULL;
		} // if
	else
		{
		AlphaAvailable = 0;
		AlphaEnabled = 0;
		} // else
	return(Success = 1);

	} // if

return(Success);

} // Raster::LoadPICT()

/*===========================================================================*/

short Raster::LoadSGIRGB(char *Name, short SupressWng)
{
FILE *fh;
unsigned short xsize, ysize, zsize;
short Success = 0;

if (fh = PROJ_fopen(Name, "rb"))
	{
	fseek(fh, 6, SEEK_SET);
	xsize = GetB16U(fh);
	ysize = GetB16U(fh);
	zsize = GetB16U(fh);
	if (ferror(fh) == 0)
		{
		Cols = xsize;
		Rows = ysize;
		ByteBands = 3;
		if (zsize == 3)
			{
			AlphaAvailable = 0;
			AlphaEnabled = 0;
			Success = 1;
			} // zsize == 3
		else if (zsize == 4)
			{
			AlphaAvailable = 1;
			AlphaEnabled = 1;
			Success = 1;
			} // zsize == 4
		} // if sizes read
	} // if fh

return(Success);

} // Raster::LoadSGIRGB()

/*===========================================================================*/

static unsigned char FileIDTestBufA[20], FileIDTestBufB[20];
// Attempts to identify the format of an image. Kludge.
// return value corresponds to WCS_BITMAPS_IDENTIMAGE_ enums in Bitmaps.h
short IdentImage(char *Name)
{
FILE *Test;
short RetVal = WCS_BITMAPS_IDENTIMAGE_ERROR;
char *ext;

if (Test = PROJ_fopen(Name, "rb"))
	{
	ext = FindFileExtension(Name);
	if (ext && (stricmp(ext, "rla") == 0))
		RetVal = WCS_BITMAPS_IDENTIMAGE_RLA;
	else if (ext && (stricmp(ext, "rpf") == 0))
		RetVal = WCS_BITMAPS_IDENTIMAGE_RPF;
	else if (fread(FileIDTestBufA, 1, 20, Test) == 20)
		{
		if (memcmp(FileIDTestBufA, "FORM", 4) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_IFFILBM;
			if (memcmp(&FileIDTestBufA[8], "FPBM", 4) == 0)
				{
				RetVal = WCS_BITMAPS_IDENTIMAGE_FPBM;
				} // if "FPBM"
			else // WCS_BITMAPS_IDENTIMAGE_IFFZBUF
			if (memcmp(&FileIDTestBufA[8], "ILBMZBUF", 8) == 0)
				{
				RetVal = WCS_BITMAPS_IDENTIMAGE_IFFZBUF;
				} // if "ZBUF"
			} // if "FORM"
		// if JPEG/PNG/TIFF etc aren't compiled in, we want to continue to act oblivious to
		// them so we don't show our cards yet. Recognising but refusing to load them
		// would be suspicious.
		#ifdef WCS_BUILD_MRSID_SUPPORT
		else if (memcmp(FileIDTestBufA, "\x6d\x73\x69\x64", 4) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_MRSID;
			} // else
		#endif // WCS_BUILD_MRSID_SUPPORT
		#ifdef WCS_BUILD_ECW_SUPPORT
		else if (memcmp(FileIDTestBufA, "\x65\x02\x01", 3) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_ECW;
			} // else
		#endif // WCS_BUILD_ECW_SUPPORT
		#ifdef WCS_BUILD_JP2_SUPPORT
		// 0000 000C 6A50 2020 0D0A 870A per http://www.faqs.org/rfcs/rfc3745.html
		else if (memcmp(FileIDTestBufA, "\x00\x00\x00\x0C\x6A\x50\x20\x20\x0D\x0A\x87\x0A", 12) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_JP2;
			} // else
		#endif // WCS_BUILD_JP2_SUPPORT
		#ifdef WCS_BUILD_JPEG_SUPPORT
		else if ((memcmp(FileIDTestBufA, "\xFF\xD8\xFF\xE0", 4) == 0) && (memcmp(&FileIDTestBufA[6], "JFIF", 4) == 0))
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_JPEG;
			} // else
		else if ((memcmp(FileIDTestBufA, "\xFF\xD8\xFF\xE1", 4) == 0) && (memcmp(&FileIDTestBufA[6], "Exif", 4) == 0))
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_JPEG;
			} // else
		#endif // WCS_BUILD_JPEG_SUPPORT
		#ifdef WCS_BUILD_PNG_SUPPORT
		else if (memcmp(FileIDTestBufA, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_PNG;
			} // else
		#endif // WCS_BUILD_PNG_SUPPORT
		#ifdef WCS_BUILD_TIFF_SUPPORT
		else if ((memcmp(FileIDTestBufA, "II\x2A\x00", 4) == 0) || (memcmp(FileIDTestBufA, "MM\x00\x2A", 4) == 0))
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_TIF;
			} // else
		#endif // WCS_BUILD_TIFF_SUPPORT
		else if (memcmp(FileIDTestBufA, "BM", 2) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_BMP;
			} // else
		else if ((memcmp(FileIDTestBufA, "WCS File", 8) == 0) &&
			strlen(Name) > 5 && ! stricmp(&Name[strlen(Name) - 5], ".elev"))
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_WCSDEM;
			} // else
		else if (memcmp(FileIDTestBufA, "\x01\xDA\x0", 3) == 0)
			{
			RetVal = WCS_BITMAPS_IDENTIMAGE_SGIRGB;
			} // else
		else // Look elsewhere...
			{
			if (fseek(Test, 512, SEEK_SET) == 0)
				{
				if (fread(FileIDTestBufB, 1, 14, Test) == 14)
					{
					if ((FileIDTestBufB[10] == 0x00) && (FileIDTestBufB[11] == 0x11)
					 && (FileIDTestBufB[12] == 0x02) && (FileIDTestBufB[13] == 0xff))
						{
						RetVal = WCS_BITMAPS_IDENTIMAGE_PICT;
						} // if
					} // if
				} // if

			// Still unidentified?
			if (RetVal == WCS_BITMAPS_IDENTIMAGE_ERROR)
				{
				// Look at ending bytes...
				if (fseek(Test, -18, SEEK_END) == 0)
					{
					if (fread(FileIDTestBufB, 1, 18, Test) == 18)
						{
						if (memcmp(FileIDTestBufB, "TRUEVISION-XFILE.", 17) == 0)
							{
							RetVal = WCS_BITMAPS_IDENTIMAGE_TARGA;
							} // if
						else
							{
							RetVal = WCS_BITMAPS_IDENTIMAGE_UNKNOWN; // Maybe Sculpt?
							} // else
						} // if
					} // if
				} // if
			} // if

		// No conclusive recognition at this point, let's try a few wild hairs and
		// see if it fits the field-values we want for an 'old' style TGA file
		if (FileIDTestBufA[1] == 0) // is TGA colormap type 0?
			{
			if ((FileIDTestBufA[2] == 2) || (FileIDTestBufA[2] == 10)) // is ImageType TrueColor, compressed or not?
				{
				// Are Cmap First Entry, CMap Len, XOrig and YOrig all 0?
				// (We no longer check CMapEntrySize, since some writers put odd values there...)
				if ((FileIDTestBufA[3] == 0) && (FileIDTestBufA[4] == 0) && (FileIDTestBufA[5] == 0)
				 && (FileIDTestBufA[6] == 0) && (FileIDTestBufA[8] == 0) && (FileIDTestBufA[9] == 0))
					{
					if ((FileIDTestBufA[16] == 24) || (FileIDTestBufA[16] == 32)) // is depth field 24 or 32?
						{
						if ((FileIDTestBufA[17] & 0xc0) == 0) // are top two bits of ImageDescriptor clear?
							{ // Ok, we'll call it a Targa file
							RetVal = WCS_BITMAPS_IDENTIMAGE_TARGA;
							} // if
						} // if
					} // if
				} // if
			} // if
		} // else if
	fclose(Test); Test = NULL;
	} // if

return(RetVal);
} // IdentImage

/*===========================================================================*/

// 03/22/02 F2: This code seems unused. I've commented it out.
/***
short IdentImageByExtension(char *Name)
{
short i = 1;

if (Name)
	{
	if (strlen(Name) >= 4)
		{
		while (FileExtension[i])
			{
			if (! stricmp(&Name[strlen(Name) - (strlen(FileExtension[i]))], FileExtension[i]))
				return (i + 1);		// + 1 makes it sync up with the defines in Bitmaps.h
			i ++;
			} // while
		} // if
	return (WCS_BITMAPS_IDENTIMAGE_UNKNOWN);
	} // if

return (WCS_BITMAPS_IDENTIMAGE_ERROR);

} // IdentImageByExtension
***/

/*===========================================================================*/

// 11/29/00 CXH: This code seems unused. I've commented it out.
/*
char *AppendImageExtension(char *Name, short Format, short Caps)
{

switch (Format)
	{
	case WCS_BITMAPS_IDENTIMAGE_IFFILBM:
		{
		if (Caps)
			strcat(Name, ".iff");
		else
			strcat(Name, ".IFF");
		break;
		} // WCS_BITMAPS_IDENTIMAGE_IFFILBM
	case WCS_BITMAPS_IDENTIMAGE_TARGA:
		{
		if (Caps)
			strcat(Name, ".tga");
		else
			strcat(Name, ".TGA");
		break;
		} // WCS_BITMAPS_IDENTIMAGE_TARGA
	case WCS_BITMAPS_IDENTIMAGE_BMP:
		{
		if (Caps)
			strcat(Name, ".bmp");
		else
			strcat(Name, ".BMP");
		break;
		} // WCS_BITMAPS_IDENTIMAGE_BMP
	case WCS_BITMAPS_IMAGE_RAW:
		{
		if (Caps)
			strcat(Name, ".red");
		else
			strcat(Name, ".RED");
		break;
		} // WCS_BITMAPS_IMAGE_RAW
	case WCS_BITMAPS_IMAGE_RAWINTER:
		{
		if (Caps)
			strcat(Name, ".raw");
		else
			strcat(Name, ".RAW");
		break;
		} // WCS_BITMAPS_IMAGE_RAWINTER
	case WCS_BITMAPS_IDENTIMAGE_WCSDEM:
		{
		if (Caps)
			strcat(Name, ".elev");
		else
			strcat(Name, ".ELEV");
		break;
		} // WCS_BITMAPS_IMAGE_WCSDEM
	} // if

return (Name);

} // AppendImageExtension

*/

#ifdef JJ_PICT_CODE
//lint -save -e702

/*===========================================================================*/
// PICT code from JJones

void PICTputShort(FILE *fd, int i)
{
	(void) putc((i >> 8) & 0xff, fd);
	(void) putc(i & 0xff, fd);
} // PICTputShort

/*===========================================================================*/

int PICTPackBits(unsigned char *rowpixels, unsigned char *packed, int cols, int rowBytes, FILE *fd)
{
	int i;
	int packcols, count, run, rep, oc;
	unsigned char *pP;
	unsigned char lastp;
	unsigned char *p;

	run = count = 0;
	for (cols--, i = cols, pP = rowpixels + cols, p = packed, lastp = *pP;
		i >= 0;
		i--, lastp = *pP, pP--)
	{
		if (lastp == *pP) run++;
		else if (run < PICT_RUN_THRESH)
		{
			while (run > 0)
			{
				*p++ = lastp;
				run--;
				count++;
				if (count == PICT_MAX_COUNT)
				{
					*p++ = PICTcounttochar(PICT_MAX_COUNT);
					count -= PICT_MAX_COUNT;
				}
			}
			run = 1;
		}
		else
		{
			if (count > 0) *p++ = PICTcounttochar(count);
			count = 0;
			while (run > 0)
			{
				rep = run > PICT_MAX_RUN ? PICT_MAX_RUN : run;
				*p++ = lastp;
				*p++ = PICTruntochar(rep);
				run -= rep;
			}
			run = 1;
		}
	}
	if (run < PICT_RUN_THRESH)
	{
		while (run > 0)
		{
			*p++ = lastp;
			run--;
			count++;
			if (count == PICT_MAX_COUNT)
			{
				*p++ = PICTcounttochar(PICT_MAX_COUNT);
				count -= PICT_MAX_COUNT;
			}
		}
	}
	else
	{
		if (count > 0) *p++ = PICTcounttochar(count);
		count = 0;
		while (run > 0)
		{
			rep = run > PICT_MAX_RUN ? PICT_MAX_RUN : run;
			*p++ = lastp;
			*p++ = PICTruntochar(rep);
			run -= rep;
		}
		run = 1;
	}
	if (count > 0) *p++ = PICTcounttochar(count);

	packcols = p - packed;
	if (cols > 250)
	{
		PICTputShort(fd, packcols);
		oc = packcols + 2;
	}
	else
	{
		(void) putc(packcols, fd);
		oc = packcols + 1;
	}

	while (p != packed)
	{
		--p;
		(void) putc(*p, fd);
	}

	return(oc);
} // PICTPackBits

/*===========================================================================*/

int PICTUnPackBits(FILE *f, unsigned char* buf, int rowBytes)
{
	int i, cCount, destBytes = 0;
	short wCount;
	unsigned char bCount, b, v, *bufPtr = buf;

	if (rowBytes > 250)
	{
		fread(&wCount, 1, 2, f);	BSWAP_W(wCount);	// it's a word
		cCount = wCount;
	}
	else
	{
		bCount = fgetc(f);		// it's a byte
		cCount = bCount;
	}

	for (i=0; i<cCount; )		// for each input byte
	{
		b = fgetc(f); i++;		// get count
		if (b == 128) continue;	// NOP
		if (b > 128)				// repeat run
		{
			b = 257 - b;		// two's complement
			v = fgetc(f); i++;	// get repeated value
			do { *bufPtr++ = v; destBytes++; } while (--b > 0);
		}
		else // b < 128
		{
			b++;	// count = b + 1
			do { *bufPtr++ = fgetc(f); i++; destBytes++; } while (--b > 0);
		}
	}
	return destBytes;

} // PICTUnPackBits

/*===========================================================================*/

int PICTUnPackBits16(FILE *f, short *buf, int rowBytes)
{
int i, cCount, destWords = 0;
short v, wCount, *bufPtr = buf;
unsigned char bCount, b;

if (rowBytes > 250)
	{
	fread(&wCount, 1, 2, f);		// it's a word
	BSWAP_W(wCount);
	cCount = wCount;
	}
else
	{
	bCount = fgetc(f);				// it's a byte
	cCount = bCount;
	}

for (i = 0; i < cCount; )			// for each input byte
	{
	b = fgetc(f); i++;				// get count
	if (b == 128)					// NOP
		continue;
	if (b > 128)					// repeat run
		{
		b = 257 - b;				// two's complement
		fread(&v, 1, 2, f);	i+=2;	// get repeated value
		BSWAP_W(v);
		do
			{
			*bufPtr++ = v;
			destWords++;
			} while (--b > 0);
		}
	else // b < 128
	{
		b++;	// count = b + 1
		do
			{
			fread(&v, 1, 2, f);
			BSWAP_W(v);
			*bufPtr++ = v;
			i+=2; destWords++;
			} while (--b > 0);
		}
	}

return destWords;

} // PICTUnPackBits16

/*===========================================================================*/

void WritePICTHeader(PICTdata *dat)
{
	FILE *f = dat->d_file;
	PICTrect R;
	short W;
	long DW;
	int i;

	R.top = 0;
	R.left = 0;
	R.bottom = dat->d_height;	BSWAP_W(R.bottom);
	R.right = dat->d_width;		BSWAP_W(R.right);

	// leading 512 bytes
	for (i=0; i<512; i++) fputc(0, f);

	// pict header
	W = 0;	fwrite(&W, 1, 2, f);						// picSize (will write this last)
	fwrite(&R, 1, 8, f);								// picFrame
	dat->d_out += 10;

	// pict version
	W = 0x0011;	BSWAP_W(W);	fwrite(&W, 1, 2, f);		// VersionOp
	W = 0x02FF; BSWAP_W(W);	fwrite(&W, 1, 2, f);		// Version
	dat->d_out += 4;

	// header op
	W = 0x0C00; BSWAP_W(W);	fwrite(&W, 1, 2, f);		// HeaderOp
	DW = 0xFFFE0000; BSWAP_L(DW); fwrite(&DW, 1, 4, f);	//lint !e569 // Version (Extended version 2 PICT)
	DW = 0x00480000; BSWAP_L(DW); fwrite(&DW, 1, 4, f);	// HRes
	fwrite(&DW, 1, 4, f);								// VRes
	fwrite(&R, 1, 8, f);								// srcRect
	DW = 0; fwrite(&DW, 1, 4, f);						// reserved
	dat->d_out += 26;

	// clip region
	W = 0x0001; BSWAP_W(W);	fwrite(&W, 1, 2, f);		// ClipRgnOp
	W = 0x000A; BSWAP_W(W);	fwrite(&W, 1, 2, f);		// rgnSize (10)
	fwrite(&R, 1, 8, f);								// the rect again
	dat->d_out += 12;

	// pixmap
	if (dat->d_depth == 1)	// grayscale
	{
		W = 0x0098;	BSWAP_W(W); fwrite(&W, 1, 2, f);	// PackBitsRectOp (for index color images)
		// no dummy base address with this op
		dat->d_out += 2;
	}
	else	// RGB or RGBA image
	{
		W = 0x009A; BSWAP_W(W); fwrite(&W, 1, 2, f);	// DirectBitsOp
		DW = 0x000000FF; BSWAP_L(DW); fwrite(&DW, 1, 4, f);	// dummy base address
		dat->d_out += 6;
	}
	W = (dat->d_width * ((dat->d_depth==1)? 1 : 4));	// rowBytes (width * 1 or 4)
	W += dat->d_padding;								// add padding, if any
	W |= 0x8000; BSWAP_W(W); fwrite(&W, 1, 2, f);		// bit 15 ON = pixMap
	fwrite(&R, 1, 8, f);								// the rect yet again
	W = 0; fwrite(&W, 1, 2, f);							// pixmap version
	W = (dat->d_depth == 1)? 0 : 4; BSWAP_W(W); fwrite(&W, 1, 2, f);	// packType
	DW = 0; fwrite(&DW, 1, 4, f);						// packSize (always zero)
	DW = 0x00480000; BSWAP_L(DW); fwrite(&DW, 1, 4, f);	// HRes again
	fwrite(&DW, 1, 4, f);								// VRes again
	W = (dat->d_depth == 1)? 0 : 16; BSWAP_W(W); fwrite(&W, 1, 2, f);	// pixelType
	W = (dat->d_depth == 1)? 8 : 32; BSWAP_W(W); fwrite(&W, 1, 2, f);	// pixelSize
	W = dat->d_depth; BSWAP_W(W); fwrite(&W, 1, 2, f);	// cmpCount
	W = 8;	BSWAP_W(W);	fwrite(&W, 1, 2, f);			// cmpSize
	DW = 0;
	fwrite(&DW, 1, 4, f);								// planeBytes
	fwrite(&DW, 1, 4, f);								// pmTable
	fwrite(&DW, 1, 4, f);								// pmReserved
	dat->d_out += 46;
	
	// color table?
	if (dat->d_depth == 1)	// grayscale
	{
		DW = 0x00000028; BSWAP_L(DW); fwrite(&DW, 1, 4, f);	// ctSeed (?)
		W = 0; fwrite(&W, 1, 2, f);							// ctFlags
		W = 255; BSWAP_W(W); fwrite(&W, 1, 2, f);			// ctSize (numColors - 1)

		for (i=0; i<256; i++)	// write color table
		{
			W = i; BSWAP_W(W);
			fwrite(&W, 1, 2, f);	// color index (0 -> 255)

			((unsigned char*)&W)[0] = i;		// add color value to high byte too (why? beat's me...)
			fwrite(&W, 1, 2, f);	// R
			fwrite(&W, 1, 2, f);	// G
			fwrite(&W, 1, 2, f);	// B
		}
		dat->d_out += 2056;
	}
	
	fwrite(&R, 1, 8, f);			// srcRect
	fwrite(&R, 1, 8, f);			// destRect
	W = (dat->d_depth == 1)? 0 : 0x0040; BSWAP_W(W); fwrite(&W, 1, 2, f);	// mode (?)
	dat->d_out += 18;
} // WritePICTHeader

/*===========================================================================*/

void PICTSetSize(void *vdat, int w, int h, int flags)
{
PICTdata *dat;

dat = (PICTdata *)vdat;

	dat->d_file = PROJ_fopen(dat->d_name, "wb");

	//if (SaveWithAlpha) SaveWithAlpha = ((flags & PICT_IMGF_ALPHA) != 0);
	dat->d_depth = dat->DoAlpha ? 4 : 3;

	dat->d_width = w;
	dat->d_height = h;
	dat->d_rowBytes = w * ((dat->d_depth == 1)? 1 : 4);
	if (dat->d_rowBytes % 2) // if odd with grayscale
	{
		dat->d_padding = (4 - (dat->d_rowBytes % 4));	// must be multiple of 4
	}
	else dat->d_padding = 0;

	WritePICTHeader(dat);

	// Get output line buffer
	dat->d_LineBuf = (unsigned char *)AppMem_Alloc(dat->d_rowBytes + dat->d_padding, NULL);
	if (!dat->d_LineBuf) { dat->d_result = PICT_IPSTAT_FAILED; return; }

	// Get compression buffer
	dat->d_CompBuf = (unsigned char *)AppMem_Alloc((dat->d_width * dat->d_depth) + 100, NULL);
	if (!dat->d_CompBuf) { dat->d_result = PICT_IPSTAT_FAILED; return; }
} // PICTSetSize

/*===========================================================================*/

int PICTSendLine(void *vdat, int line, const ImageValue *rgbline, const ImageValue *alphaline)
{
PICTdata *dat;

dat = (PICTdata *)vdat;

	int i, c;
	unsigned char *LinePtr = dat->d_LineBuf;
	const unsigned char *rgbPtr = rgbline;
	int fullwidth = (dat->d_width * dat->d_depth) + dat->d_padding;

	if (dat->d_result != PICT_IPSTAT_OK) return -1;

	memset(dat->d_LineBuf, 0, (dat->d_rowBytes + dat->d_padding));	// clear buffer

	if (dat->d_depth == 1)
	{
		memcpy(dat->d_LineBuf, rgbline, dat->d_width);
	}
	else // RGB or RGBA
	{
		if (dat->d_depth == 4)
		{
			memcpy(LinePtr, alphaline, dat->d_width);
			LinePtr += dat->d_width;
		}

		for (c=0; c<3; c++)	// change chunky to planar
		{
			for (i=0; i<dat->d_width*3; i+=3) // get each channel
			{
				*LinePtr++ = rgbPtr[i];
			}
			rgbPtr++;	// next channel
		}
	}
	dat->d_out += PICTPackBits(dat->d_LineBuf, dat->d_CompBuf, fullwidth, dat->d_rowBytes, dat->d_file);

	return PICT_OK;
} // PICTSendLine

/*===========================================================================*/

int PICTDone(void *vdat, int error)
{
PICTdata *dat;

dat = (PICTdata *)vdat;

	short tempW;
	int pos;

	if (error) { dat->d_result = PICT_IPSTAT_FAILED; return 1; }

	pos = ftell(dat->d_file);
	if (pos % 2)
	{
		fputc(0, dat->d_file);	// add pad byte if not word aligned
		dat->d_out += 1;		// duh
	}

	tempW = 0x00FF; BSWAP_W(tempW); fwrite(&tempW, 1, 2, dat->d_file);		// end of picture op
	dat->d_out += 2;

	fseek(dat->d_file, 512, SEEK_SET);										// get back
	tempW = dat->d_out; BSWAP_W(tempW); fwrite(&tempW, 1, 2, dat->d_file);	// write pictSize
	// yes, that just lost the high byte of the int, but them's pict files for 'ya.
	
	AppMem_Free(dat->d_LineBuf, dat->d_LineBufSize);
	AppMem_Free(dat->d_CompBuf, dat->d_CompBufSize);
	fclose(dat->d_file);			// close file

	return (dat->d_result != PICT_IPSTAT_OK);
} // PICTDone

/*===========================================================================*/

int PICTMainSaver(ImSaverLocal *local)
{
	PICTdata *dat;
	ImageProtocol prot;

	//SaveWithAlpha = FALSE;

	dat = (PICTdata *)local->priv_data;
	dat->d_name = (char*) local->filename;
	dat->d_type = local->type;

	dat->d_result = PICT_IPSTAT_OK;
	//dat->d_LineBufSize = 0;
	dat->d_CompBufSize = 0;
	//dat->d_LineBuf = NULL;
	dat->d_CompBuf = NULL;
	dat->d_out = 0;

	prot.type = dat->d_type;
	prot.color.priv_data = dat;
	prot.color.setSize = PICTSetSize;
	prot.color.sendLine = PICTSendLine;
	prot.color.done = PICTDone;

	// get the data
	(*local->sendData) (local->priv_data, &prot, 0);

	local->result = dat->d_result;
	return PICT_OK;
}

/*===========================================================================*/

void PICTGetRect(FILE* f, PICTrect *R)
{
	fread(&(R->top), 1, 2, f);		BSWAP_W(R->top);	// TOP
	fread(&(R->left), 1, 2, f);		BSWAP_W(R->left);	// LEFT
	fread(&(R->bottom), 1, 2, f);	BSWAP_W(R->bottom);	// BOTTOM
	fread(&(R->right), 1, 2, f);	BSWAP_W(R->right);	// RIGHT
}

/*===========================================================================*/

int PICTLoader(ImLoaderLocal *local)
{
	// To the tune of "Several Species Of Small Furry Animals Gathered Together In A Cave And Grooving With A PICT."
	FILE* f;
	ImageProtocolID ip;
	ColorProtocol *cp;
	IndexProtocol *indexp;
	int i, j;
	int width, height, width2, width3, UPB=0, aBufSize=0, inBufSize, outBufSize;
	unsigned char *inBuf = NULL, *inPtr, *outBuf = NULL, *outPtr, *aBuf = NULL, *aPtr;
	short w, *inWBuf, *inWPtr;
	unsigned char b, colors[768];

	// PICT stuff
	BOOL scanning = TRUE;
	PICTrect pmRect;
	short pVersionOp, pVersion, hOpCode, tempOp, tempW;
	short pmRowBytes, pmVersion, pmPackType, pmPixelType, pmPixelSize, pmCmpCount, pmCmpSize;
	short ctFlags, ctSize = 0, tempWord;
	long hVersion, pmPackSize, ctSeed;

	// open the file...
	f = PROJ_fopen(local->filename, "rb");
	if (!f) { local->result = PICT_IPSTAT_BADFILE; return PICT_OK; }

	// it it a PICT file?
	fseek(f, 512, SEEK_SET);		// skip first 512 bytes
	fseek(f, 10, SEEK_CUR);			// skip size (2) and boundsRect (8)
	fread(&pVersionOp, 1, 2, f);	BSWAP_W(pVersionOp);
	if (pVersionOp != 0x0011)
		{
		fclose(f);
		local->result = PICT_IPSTAT_NOREC;
		return PICT_OK;
		}
	fread(&pVersion, 1, 2, f);	BSWAP_W(pVersion);
	if (pVersion != 0x02FF)
		{
		fclose(f);
		local->result = PICT_IPSTAT_NOREC;
		return PICT_OK;
		}
	fread(&hOpCode, 1, 2, f);	BSWAP_W(hOpCode);
	if (hOpCode != 0x0C00)
		{
		fclose(f);
		local->result = PICT_IPSTAT_NOREC;
		return PICT_OK;
		}
	fread(&hVersion, 1, 4, f);	BSWAP_L(hVersion);
//	if (hVersion != 0xFFFE0000 && hVersion != 0xFFFFFFFF) { fclose(f); local->result = PICT_IPSTAT_NOREC; return PICT_OK; }
//	(Killed the above because of many files written with bogus info here rather than version num.)
	// OK, it's a PICT file
	fseek(f, 20, SEEK_CUR);			// skip hRes, vRes, srcRect and reserved

	while (scanning)
	{
		fread(&tempOp, 1, 2, f);	BSWAP_W(tempOp);	// read OpCode
		switch(tempOp)
		{
		case 0x00A1:	// long comment: kind(2bytes), length(2bytes), data
						// length = data + 4;
			fseek(f, 2, SEEK_CUR);								// skip 'kind'
			fread(&tempWord, 1, 2, f);	BSWAP_W(tempWord);		// read size
			fseek(f, tempWord, SEEK_CUR);						// skip comment data
			break;
		case 0x0000:	break;									// NOP (0 bytes)
		case 0x0001:	fseek(f, 10, SEEK_CUR);		break;		// clip
		case 0x001A:
		case 0x001B:
		case 0x001D:
		case 0x001F:	fseek(f, 6, SEEK_CUR);		break;		// RGB fg/bg/hilite/op colors
		case 0x001E:	break;									// DefHilite (0 bytes)
		case 0x0098:	// PackBitsRect (used for color-mapped images and grayscale)
			// PixMap
			fread(&pmRowBytes, 1, 2, f);	BSWAP_W(pmRowBytes);	// bytes per row, and flags
			PICTGetRect(f, &pmRect);									// bounds rect
			fread(&pmVersion, 1, 2, f);		BSWAP_W(pmVersion);		// pixmap version
			fread(&pmPackType, 1, 2, f);	BSWAP_W(pmPackType);	// 0 <-> 4
			fread(&pmPackSize, 1, 4, f);	BSWAP_L(pmPackSize);
			fseek(f, 8, SEEK_CUR);			// skip hRes and vRes
			fread(&pmPixelType, 1, 2, f);	BSWAP_W(pmPixelType);	// pixel type
			fread(&pmPixelSize, 1, 2, f);	BSWAP_W(pmPixelSize);	// pixel size
			fread(&pmCmpCount, 1, 2, f);	BSWAP_W(pmCmpCount);	// number of components
			fread(&pmCmpSize, 1, 2, f);		BSWAP_W(pmCmpSize);		// component size
			fseek(f, 12, SEEK_CUR);			// skip planebytes, pmtable and pmreserved
			// Color Table
			fread(&ctSeed, 1, 4, f);		BSWAP_L(ctSeed);		// WTFIT?
			fread(&ctFlags, 1, 2, f);		BSWAP_W(ctFlags);		// ditto
			fread(&ctSize, 1, 2, f);		BSWAP_W(ctSize);		// entries - 1
			ctSize++;	// correct
			// Load colors
			for (i=0; i<(ctSize*3); i+=3)
			{
				fseek(f, 2, SEEK_CUR);		// skip ctValue (enum of color indexes)
				fread(&tempW, 1, 2, f);		BSWAP_W(tempW);		// Red
				colors[i] = (tempW >> 8) & 0xFF;	// get high byte
				fread(&tempW, 1, 2, f);		BSWAP_W(tempW);		// Green
				colors[i+1] = (tempW >> 8) & 0xFF;	// get high byte
				fread(&tempW, 1, 2, f);		BSWAP_W(tempW);		// Blue
				colors[i+2] = (tempW >> 8) & 0xFF;	// get high byte
			}
			fseek(f, 18, SEEK_CUR);			// skip srcRect, destRect and mode
			// now at start of image data
			scanning = FALSE;
			break;
		case 0x009A:	// DirectBitsRect (for 24 and 32 bit images)
			// PixMap
			fseek(f, 4, SEEK_CUR);			// skip dummy base pointer (0x000000FF)
			fread(&pmRowBytes, 1, 2, f);	BSWAP_W(pmRowBytes);	// bytes per row, and flags
			PICTGetRect(f, &pmRect);									// bounds rect
			fread(&pmVersion, 1, 2, f);		BSWAP_W(pmVersion);		// pixmap version
			fread(&pmPackType, 1, 2, f);	BSWAP_W(pmPackType);	// 0 <-> 4
			fread(&pmPackSize, 1, 4, f);	BSWAP_L(pmPackSize);
			fseek(f, 8, SEEK_CUR);			// skip hRes and vRes
			fread(&pmPixelType, 1, 2, f);	BSWAP_W(pmPixelType);	// pixel type
			fread(&pmPixelSize, 1, 2, f);	BSWAP_W(pmPixelSize);	// pixel size
			fread(&pmCmpCount, 1, 2, f);	BSWAP_W(pmCmpCount);	// number of components
			fread(&pmCmpSize, 1, 2, f);		BSWAP_W(pmCmpSize);		// component size
			fseek(f, 12, SEEK_CUR);			// skip planebytes, pmtable and pmreserved
			fseek(f, 18, SEEK_CUR);			// skip srcRect, destRect and mode
			// now at start of image data
			scanning = FALSE;
			break;
		default: { fclose(f); local->result = PICT_IPSTAT_NOREC; return PICT_OK; }
		}	// end switch
	}	// end while (scanning)

	pmRowBytes = pmRowBytes & 0x3FFF;	// nix the flag bits
	width = pmRect.right - pmRect.left;
	height = pmRect.bottom - pmRect.top;

	if (pmPixelType == 0 && ctSize > 0 && pmPixelSize <= 8 && pmCmpCount == 1 ) // indexed image
	{
		ip = (*local->begin) (local->priv_data, PICT_IMG_INDEX8); // grayscale is also index8
		if (!ip) { fclose(f); local->result = PICT_IPSTAT_FAILED; return PICT_OK; }
		indexp = &ip->index;
		PICT_IP_SETSIZE(indexp, width, height, 0);
		PICT_IP_NUMCOLORS(indexp, ctSize);
		for (i=0; i<ctSize; i++) PICT_IP_SETMAP(indexp, i, (unsigned char *)&colors[i*3]); //lint !e645
	}
	else if (pmPixelType == 16 && pmPixelSize >= 16 && pmCmpCount >= 3) // direct pixel image
	{
		ip = (*local->begin) (local->priv_data, PICT_IMG_RGB24);
		if (!ip) { fclose(f); local->result = PICT_IPSTAT_FAILED; return PICT_OK; }
		cp = &ip->color;
		PICT_IP_SETSIZE(cp, width, height, (pmCmpCount==4)? PICT_IMGF_ALPHA : 0);
	}
	else { fclose(f); local->result = PICT_IPSTAT_BADFILE; return PICT_OK; } // really bad file

	// get buffers
	inBuf = (unsigned char *)AppMem_Alloc(inBufSize = pmRowBytes, NULL);
	if (!inBuf) { local->result = PICT_IPSTAT_FAILED; goto cleanup; }
	inWPtr = inWBuf = (short*) inBuf;	// in case of 16-bit pixels

	outBuf = (unsigned char *)AppMem_Alloc(outBufSize = (width * ((pmCmpCount == 4)? 3 : pmCmpCount)), NULL); // for rgb or index pixels
	if (!outBuf) { local->result = PICT_IPSTAT_FAILED; goto cleanup; }

	if (pmCmpCount == 4)	// for alpha channel, if any
	{
		aBuf = (unsigned char *)AppMem_Alloc(aBufSize = width, NULL);
		if (!aBuf) { local->result = PICT_IPSTAT_FAILED; goto cleanup; }
	}

	width2 = width * 2;
	width3 = width * 3;

	// read in each line
	for (i=0; i<height; i++)
		{
		switch(pmPackType)
			{
			case 0:	// default packing
				if (pmPixelSize == 16)
				{
					UPB = PICTUnPackBits16(f, inWBuf, pmRowBytes);
				}
				else // the docs _imply_ that this is correct...
				{
					UPB = PICTUnPackBits(f, inBuf, pmRowBytes);
				}
				break;
			case 1:	// no packing
				fread(inBuf, 1, pmRowBytes, f);
				break;
			case 2:	// the old drop the pad byte trick
				fread(inBuf, 1, (pmRowBytes * 3) / 4, f);	// RGB only
				break;
			case 3:	// 16-bit packbits
				UPB = PICTUnPackBits16(f, inWBuf, pmRowBytes);
				break;
			case 4:
				UPB = PICTUnPackBits(f, inBuf, pmRowBytes);
				break;
			} // switch

		inPtr = inBuf;
		inWPtr = inWBuf;

		// decode pixels
		switch(pmPixelSize)
		{
		case 1:
			outPtr = outBuf;
			for (j=0; j<width; j+=8)
			{
				b = *inPtr++;
				*outPtr++ = (b & 128) >> 7;
				*outPtr++ = (b & 64) >> 6;
				*outPtr++ = (b & 32) >> 5;
				*outPtr++ = (b & 16) >> 4;
				*outPtr++ = (b & 8) >> 3;
				*outPtr++ = (b & 4) >> 2;
				*outPtr++ = (b & 2) >> 1;
				*outPtr++ = b & 1;
			}
			if (PICT_IP_SENDLINE(indexp, i, (unsigned char *)outBuf, 0)) goto cleanup;
			break;
		case 2:
			outPtr = outBuf;
			for (j=0; j<width; j+=4)
			{
				b = *inPtr++;
				*outPtr++ = (b & 192) >> 6;
				*outPtr++ = (b & 48) >> 4;
				*outPtr++ = (b & 12) >> 2;
				*outPtr++ = b & 3;
			}
			if (PICT_IP_SENDLINE(indexp, i, (unsigned char *)outBuf, 0)) goto cleanup;
			break;
		case 4:
			outPtr = outBuf;
			for (j=0; j<width; j+=2)
			{
				b = *inPtr++;
				*outPtr++ = (b & 0xF0) >> 4;
				*outPtr++ = b & 0x0F;
			}
			if (PICT_IP_SENDLINE(indexp, i, (unsigned char *)outBuf, 0)) goto cleanup;
			break;
		case 8:
			if (PICT_IP_SENDLINE(indexp, i, (unsigned char *)inBuf, 0)) goto cleanup;
			break;
		case 16:    // Bits: 15(unused), R(14-10), G(9-5), B(4-0)
			outPtr = outBuf;
			for (j=0; j<width; j++)
			{
				w = *inWPtr++;
				*outPtr++ = (w >> 7) & 0xF8;  // R
				*outPtr++ = (w >> 2) & 0xF8;  // G
				*outPtr++ = (w << 3) & 0xF8;  // B
			}
			if (PICT_IP_SENDLINE(cp, i, outBuf, 0)) goto cleanup;
			break;
		case 32:
			outPtr = outBuf;
			aPtr = aBuf;
			for (j=0; j<width; j++)
			{
				if (aBuf)	// alpha? (aBuf == NULL if not)
				{
					*aPtr++ = inPtr[0];				// A
					*outPtr++ = inPtr[width];		// R
					*outPtr++ = inPtr[width2];		// G
					*outPtr++ = inPtr[width3];		// B
				}
				else
				{
					if (UPB && UPB > pmRowBytes)	// CmpCount == 3, but 4 stored.
					{
						//*aPtr++ = inPtr[0];			// A
						*outPtr++ = inPtr[width];		// R
						*outPtr++ = inPtr[width2];		// G
						*outPtr++ = inPtr[width3];		// B
					}
					else
					{
						*outPtr++ = inPtr[0];			// R
						*outPtr++ = inPtr[width];		// G
						*outPtr++ = inPtr[width2];		// B
					}
				}
				inPtr++;
			}
			if (PICT_IP_SENDLINE(cp, i, (unsigned char *)outBuf, (unsigned char *)((aBuf)? aBuf : 0))) goto cleanup;
			break;
		}
	}

	// call PICT_IP_DONE
	if (pmPixelType == 0)
	{
		if (PICT_IP_DONE(indexp, 0)) { local->result = PICT_IPSTAT_ABORT; goto cleanup; }
	}
	else
	{
		if (PICT_IP_DONE(cp, 0)) { local->result = PICT_IPSTAT_ABORT; goto cleanup; }
	}

	// call ImLoaderLocal DONE
	(*local->done) (local->priv_data, ip);

	local->result = PICT_IPSTAT_OK;

cleanup:

	if (inBuf) AppMem_Free(inBuf, inBufSize);
	if (outBuf) AppMem_Free(outBuf, outBufSize);
	if (aBuf) AppMem_Free(aBuf, aBufSize);
	fclose(f);
	return PICT_OK;
}

/*===========================================================================*/

// Called from PictLoader from LoadPict
ImageProtocolID PICTHostBegin(void *, int type)
{
static ImageProtocol HostLoadIP;

if (type == PICT_IMG_RGB24)
	{
	HostLoadIP.type = HostLoadIP.color.type = PICT_IMG_RGB24;
	HostLoadIP.color.setSize = PICTHostSetSize;
	HostLoadIP.color.sendLine = PICTHostSendLine;
	HostLoadIP.color.done = PICTHostColorDone;
	HostLoadIP.color.priv_data = &Loaddat;
	return(&HostLoadIP);
	} // if

return(NULL);
} // PICTHostBegin

/*===========================================================================*/

// Called from PictLoader via PICT_IP_SETSIZE
void PICTHostSetSize(void *vdat, int w, int h, int flags)
{
PICTdata *pdat;

pdat = (PICTdata *)vdat;

if ((w > 0) && (h > 0))
	{
	pdat->Rbuf = (UBYTE *)AppMem_Alloc(w * h, APPMEM_CLEAR, "PICT Loader Bitmaps");
	pdat->Gbuf = (UBYTE *)AppMem_Alloc(w * h, APPMEM_CLEAR, "PICT Loader Bitmaps");
	pdat->Bbuf = (UBYTE *)AppMem_Alloc(w * h, APPMEM_CLEAR, "PICT Loader Bitmaps");
	if (pdat->Rbuf && pdat->Gbuf && pdat->Bbuf)
		{
		pdat->d_width = w;
		pdat->d_height = h;
		pdat->d_depth = (flags & PICT_IMGF_ALPHA ? 32 : 24);
		} // if
	else
		{
		if (pdat->Rbuf) AppMem_Free(pdat->Rbuf, w * h); pdat->Rbuf = NULL;
		if (pdat->Gbuf) AppMem_Free(pdat->Gbuf, w * h); pdat->Gbuf = NULL;
		if (pdat->Bbuf) AppMem_Free(pdat->Bbuf, w * h); pdat->Bbuf = NULL;
		} // if
	if (flags & PICT_IMGF_ALPHA)
		{
		pdat->Abuf = (UBYTE *)AppMem_Alloc(w * h, APPMEM_CLEAR, "PICT Loader Alpha Bitmaps");
		} // if
	} // if
else
	{
	pdat->Rbuf = NULL;
	pdat->Gbuf = NULL;
	pdat->Bbuf = NULL;
	pdat->Abuf = NULL;
	} // else

} // PICTHostSetSize

/*===========================================================================*/

// called from PictLoader via PICT_IP_SENDLINE
int PICTHostSendLine(void *vdat, int line, const ImageValue *rgbline, const ImageValue *alphaline)
{
PICTdata *pdat;
int LineWid, LineSub, ASub, LineOffset;

pdat = (PICTdata *)vdat;

if (pdat->Rbuf && pdat->Gbuf && pdat->Bbuf)
	{
	LineOffset = (line * pdat->d_width);
	// Deinterleave RGB data into seperate buffers
	for (LineWid = LineSub = ASub = 0; LineWid < pdat->d_width; LineWid++)
		{
		pdat->Rbuf[LineOffset + LineWid] = rgbline[LineSub++];
		pdat->Gbuf[LineOffset + LineWid] = rgbline[LineSub++];
		pdat->Bbuf[LineOffset + LineWid] = rgbline[LineSub++];
		if (pdat->Abuf)
			{
			pdat->Abuf[LineOffset + LineWid] = alphaline[ASub++];
			} // if
		} // for
	return(0);
	} // if
else
	{
	return(-1);
	} // else

} // PICTHostSendLine

/*===========================================================================*/

int PICTHostColorDone(void *vdat, int error)
{
PICTdata *pdat;

pdat = (PICTdata *)vdat;

if (pdat->Rbuf && pdat->Gbuf && pdat->Bbuf)
	{
	return(0);
	} // if
else
	{
	return(-1);
	} // else

} // PictHostColorDone

/*===========================================================================*/

void PICTHostDone(void *vdat, ImageProtocolID)
{

return;

} // PictHostDone

/*===========================================================================*/

// This is only used by the PICT saver, apparently
int PICTHostSendData(void *vdat, ImageProtocolID IPID, int flags)
{
int LineLoop, LineTerm, LineStep, LineOff, /* ReverseMode = 0,*/ MultiCount, MonoCount, LineX;
unsigned char *MultiplexBuf, *NewR, *NewG, *NewB, *NewA, Failed = 0;
BufferNode *RBN, *GBN, *BBN, *ABN;
PICTdata *PD;
ColorProtocol *cp;

PD = (PICTdata *)vdat;
cp = &IPID->color;
MultiplexBuf = (unsigned char *)PD->d_LineBuf;

if (cp->setSize)
	{
	PICT_IP_SETSIZE(cp, PD->d_width, PD->d_height, 0);
	} //
else
	{
	return(1);
	} // else


if (flags & PICT_IMGF_REVERSE)
	{
	//ReverseMode = 1;
	LineLoop = PD->d_height - 1;
	LineStep = -1;
	LineTerm = -1;
	} // if
else
	{
	LineLoop = 0;
	LineStep = 1;
	LineTerm = PD->d_height;
	} // else

if (PD->NewMode)
	{
	RBN = (BufferNode *)PD->Rbuf;
	GBN = (BufferNode *)PD->Gbuf;
	BBN = (BufferNode *)PD->Bbuf;
	ABN = (BufferNode *)PD->Abuf;
	} // if

for (; LineLoop != LineTerm; LineLoop += LineStep)
	{
	LineOff = PD->d_width * LineLoop;

	if (PD->NewMode)
		{
		NewR = (unsigned char *)(RBN->GetDataLine(LineLoop, WCS_RASTER_BANDSET_BYTE));
		NewG = (unsigned char *)(GBN->GetDataLine(LineLoop, WCS_RASTER_BANDSET_BYTE));
		NewB = (unsigned char *)(BBN->GetDataLine(LineLoop, WCS_RASTER_BANDSET_BYTE));
		if (PD->DoAlpha && ABN)
			{
			NewA = (unsigned char *)(ABN->GetDataLine(LineLoop, WCS_RASTER_BANDSET_BYTE));
			} // if
		else
			{
			NewA = NULL;
			} // else
		} // if
	else
		{
		NewA = NULL;
		} // else

	for (MonoCount = MultiCount = LineX = 0; LineX < PD->d_width; LineX++)
		{
		if (PD->NewMode)
			{
			MultiplexBuf[MultiCount++] = NewR[MonoCount];
			MultiplexBuf[MultiCount++] = NewG[MonoCount];
			MultiplexBuf[MultiCount++] = NewB[MonoCount];
			} // if
		else
			{
			MultiplexBuf[MultiCount++] = PD->Rbuf[LineOff + MonoCount];
			MultiplexBuf[MultiCount++] = PD->Gbuf[LineOff + MonoCount];
			MultiplexBuf[MultiCount++] = PD->Bbuf[LineOff + MonoCount];
			} // else
		MonoCount++;
		} // for
	if (PICT_IP_SENDLINE(cp, LineLoop, (unsigned char *)MultiplexBuf, NewA) != 0)
		{
		Failed = 1;
		break;
		} // if
	} // for

if (cp->done)
	{
	PICT_IP_DONE(cp, Failed);
	} //

return(Failed);

} // PICTHostSendData

//lint -restore
#endif // JJ_PICT_CODE
