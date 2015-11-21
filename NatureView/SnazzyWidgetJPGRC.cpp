// SnazzyWidgetJPGRC.cpp
// Interface between JPEG-in-Resource and SnazzyWidgetImageCollection

// support for loading from in-memory JPEG resources
#include <setjmp.h>
#include <stdio.h>
#include <windows.h>

extern "C" {
#include "jpeglib.h"
#include "png.h"
}


#include "SnazzyWidgetJPGRC.h"

// only accessed internally from within this file
int LoadJpegResourceAsRGB(unsigned short ImageID, unsigned char *&Red, unsigned char *&Green, unsigned char *&Blue, unsigned long int &Width, unsigned long int &Height);
int LoadPNGResourceAsRGBA(unsigned short ImageID, unsigned char *&Red, unsigned char *&Green, unsigned char *&Blue, unsigned char *&Alpha, unsigned long int &Width, unsigned long int &Height);


bool LoadPNGImageToSnazzyWidgetNormal(unsigned short ImageID, SnazzyWidgetImage *Destination)
{
return(LoadPNGImageToQuadBand(ImageID, Destination->GetR_Normal(), Destination->GetG_Normal(), Destination->GetB_Normal(), Destination->GetA_Normal()));
} // LoadPNGImageToSnazzyWidgetNormal


bool LoadJPGImageToSnazzyWidgetNormal(unsigned short ImageID, SnazzyWidgetImageCollection *Destination)
{
Destination->SetNormalValid(true);
return(LoadJPGImageToTripleBand(ImageID, Destination->GetR_Normal(), Destination->GetG_Normal(), Destination->GetB_Normal()));
} // LoadJPGImageToSnazzyWidgetNormal

bool LoadJPGImageToSnazzyWidgetNormalHover(unsigned short ImageID, SnazzyWidgetImageCollection *Destination)
{
Destination->SetNormalHoverValid(true);
return(LoadJPGImageToTripleBand(ImageID, Destination->GetR_NormalHover(), Destination->GetG_NormalHover(), Destination->GetB_NormalHover()));
} // LoadJPGImageToSnazzyWidgetNormalHover

bool LoadJPGImageToSnazzyWidgetSelected(unsigned short ImageID, SnazzyWidgetImageCollection *Destination)
{
Destination->SetSelectedValid(true);
return(LoadJPGImageToTripleBand(ImageID, Destination->GetR_Selected(), Destination->GetG_Selected(), Destination->GetB_Selected()));
} // LoadJPGImageToSnazzyWidgetSelected

bool LoadJPGImageToSnazzyWidgetSelectedHover(unsigned short ImageID, SnazzyWidgetImageCollection *Destination)
{
Destination->SetSelectedHoverValid(true);
return(LoadJPGImageToTripleBand(ImageID, Destination->GetR_SelectedHover(), Destination->GetG_SelectedHover(), Destination->GetB_SelectedHover()));
} // LoadJPGImageToSnazzyWidgetSelectedHover

bool LoadPNGImageToSnazzyWidgetIndex(unsigned short ImageID, SnazzyWidgetImageCollection *Destination)
{
return(LoadPNGImageToSingleBand(ImageID, Destination->Get_Index()));
} // LoadPNGImageToSnazzyWidgetIndex



// loads RGB bands
bool LoadJPGImageToTripleBand(unsigned short ImageID, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB)
{
unsigned char *R, *G, *B;
unsigned long int W, H;

if(ImageID && DestR && DestG && DestB)
	{
	if(LoadJpegResourceAsRGB(ImageID, R, G, B, W, H))
		{
		unsigned char *PixelDataR, *PixelDataG, *PixelDataB;
		PixelDataR = R;
		PixelDataG = G;
		PixelDataB = B;
		for(unsigned long int YLoop = 0; YLoop < H; YLoop++)
			{
			for(unsigned long int XLoop = 0; XLoop < W; XLoop++)
				{
				*DestR++ = *PixelDataR++;
				*DestG++ = *PixelDataG++;
				*DestB++ = *PixelDataB++;
				} // for
			} // for
		delete [] R; R = NULL;
		delete [] G; G = NULL;
		delete [] B; B = NULL;
		return(true);
		} // if
	} // if

return(false);
} // LoadJPGImageToTripleBand


// only loads R band to destination (used for index)
bool LoadPNGImageToSingleBand(unsigned short ImageID, unsigned char *DestBand)
{
unsigned char *R, *G, *B, *A;
unsigned long int W, H;

if(ImageID && DestBand)
	{
	if(LoadPNGResourceAsRGBA(ImageID, R, G, B, A, W, H))
		{
		unsigned char *PixelData;
		PixelData = R;
		for(unsigned long int YLoop = 0; YLoop < H; YLoop++)
			{
			for(unsigned long int XLoop = 0; XLoop < W; XLoop++)
				{
				*DestBand++ = *PixelData++;
				} // for
			} // for
		delete [] R; R = NULL;
		delete [] G; G = NULL;
		delete [] B; B = NULL;
		delete [] A; A = NULL;
		return(true);
		} // if
	} // if
return(false);
} // LoadPNGImageToSingleBand


// loads RGBA bands (PNG only, used for slider knobs)
bool LoadPNGImageToQuadBand(unsigned short ImageID, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB, unsigned char *DestA)
{
unsigned char *R, *G, *B, *A;
unsigned long int W, H;

if(ImageID && DestR && DestG && DestB && DestA)
	{
	if(LoadPNGResourceAsRGBA(ImageID, R, G, B, A, W, H))
		{
		unsigned char *PixelDataR, *PixelDataG, *PixelDataB, *PixelDataA;
		PixelDataR = R;
		PixelDataG = G;
		PixelDataB = B;
		PixelDataA = A;
		for(unsigned long int YLoop = 0; YLoop < H; YLoop++)
			{
			for(unsigned long int XLoop = 0; XLoop < W; XLoop++)
				{
				*DestR++ = *PixelDataR++;
				*DestG++ = *PixelDataG++;
				*DestB++ = *PixelDataB++;
				if(A)
					{
					*DestA++ = *PixelDataA++;
					} // if
				else
					{
					*DestA++ = 255; // full-on opaque
					} // else
				} // for
			} // for
		delete [] R; R = NULL;
		delete [] G; G = NULL;
		delete [] B; B = NULL;
		delete [] A; A = NULL;
		return(true);
		} // if
	} // if

return(false);
} // LoadPNGImageToQuadBand


// *********************************************************************
//
// Code to support loading and decompressing a JPEG out of resource data
//
// *********************************************************************



struct jpeg_source_mgr JPEGMemLoader;
static void *ImageJPGRsc;
static int ImageJPGSize;

void mem_init_source(j_decompress_ptr cinfo)
{
jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

// We reset the empty-input-file flag for each image,
// but we don't clear the input buffer.
// This is correct behavior for reading a series of images from one source.
src->next_input_byte = (unsigned char *)ImageJPGRsc;
src->bytes_in_buffer = ImageJPGSize;
} // mem_init_source()


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

// X: I don't believe this will ever be called as we fill the entire buffer in init_source above
boolean mem_fill_input_buffer(j_decompress_ptr cinfo)
{
  jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

  src->next_input_byte = (unsigned char *)ImageJPGRsc;
  src->bytes_in_buffer = ImageJPGSize;

  return TRUE;
} // mem_fill_input_buffer()


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

void mem_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

src->next_input_byte += (size_t) num_bytes;
src->bytes_in_buffer -= (size_t) num_bytes;
} // mem_skip_input_data


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */

unsigned char mem_jpeg_resync_to_restart (j_decompress_ptr cinfo, int desired)
{
return(jpeg_resync_to_restart(cinfo, desired));
} // mem_jpeg_resync_to_restart 


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

void mem_term_source(j_decompress_ptr cinfo)
{
  /* no work necessary here */
} // mem_term_source


/*
 * Here's the routine that will replace the standard error_exit method:
 */

struct SW_my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct SW_my_error_mgr * my_error_ptr;

METHODDEF(void)
mem_my_JPEGLOADER_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  //(*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


// code taken from WCS/VNS VersionGUI.cpp
// memory is allocated with operator new[], caller is responsible for freeing
int LoadJpegResourceAsRGB(unsigned short ImageID, unsigned char *&Red, unsigned char *&Green, unsigned char *&Blue, unsigned long int &Width, unsigned long int &Height)
{
int Success = 0;
HRSRC  ResHandle = NULL;
struct jpeg_decompress_struct cinfo;
struct SW_my_error_mgr jerr;
int row_stride = 0;
unsigned long int InScan, PixelCol;
short Cols, Rows;
unsigned char *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf;
WORD LOGOID = ImageID;

Width = Height = 0;
Red = Green = Blue = NULL;

// passing NULL as Instance means 'me you moron'
if(ImageJPGRsc = LockResource(LoadResource(NULL, ResHandle = FindResource(NULL, MAKEINTRESOURCE(LOGOID), "JPEGIMAGE"))))
	{
	ImageJPGSize = SizeofResource(NULL, ResHandle);

	cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);
	jerr.pub.error_exit = mem_my_JPEGLOADER_error_exit;

	if (setjmp(jerr.setjmp_buffer))
		{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		if(InterleaveBuf) delete [] InterleaveBuf;
		InterleaveBuf = NULL;
		if(!Success)
			{
			delete [] Red; Red = NULL;
			delete [] Green; Green = NULL;
			delete [] Blue; Blue = NULL;
			} // if
		jpeg_destroy_decompress(&cinfo);
		return(Success);
		} // if

	jpeg_create_decompress(&cinfo);
	cinfo.src = &JPEGMemLoader;
	cinfo.src->init_source = mem_init_source;
	cinfo.src->fill_input_buffer = mem_fill_input_buffer;
	cinfo.src->skip_input_data = mem_skip_input_data;
	cinfo.src->resync_to_restart = mem_jpeg_resync_to_restart; /* use default method */
	cinfo.src->term_source = mem_term_source;
	cinfo.src->bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	cinfo.src->next_input_byte = NULL; /* until buffer loaded */

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	cinfo.dct_method = JDCT_FLOAT;
	jpeg_start_decompress(&cinfo);

	Width = Cols = (short)cinfo.output_width;
	Height = Rows = (short)cinfo.output_height;
	Red = new unsigned char[Cols * Rows];
	Green = new unsigned char[Cols * Rows];
	Blue = new unsigned char[Cols * Rows];
	row_stride = cinfo.output_width * 3;
	InterleaveBuf = new unsigned char[row_stride];
	if(InterleaveBuf && Red && Green && Blue) // everything ok?
		{
		// Clear bitmaps
		memset(Red, 0, Cols * Rows);
		memset(Green, 0, Cols * Rows);
		memset(Blue, 0, Cols * Rows);

		while (cinfo.output_scanline < cinfo.output_height)
			{
			RBuf = &Red[cinfo.output_scanline * Cols];
			GBuf = &Green[cinfo.output_scanline * Cols];
			BBuf = &Blue[cinfo.output_scanline * Cols];
			if(jpeg_read_scanlines(&cinfo, &InterleaveBuf, 1) != 1)
				{
				jpeg_abort_decompress(&cinfo);
				break;
				} // if
			else
				{ // deinterleave
				for(InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
					{
					RBuf[PixelCol] = InterleaveBuf[InScan++];
					GBuf[PixelCol] = InterleaveBuf[InScan++];
					BBuf[PixelCol] = InterleaveBuf[InScan++];
					} // for
				} // else
			} // while

		if(cinfo.output_scanline == cinfo.output_height)
			{
			Success = 1;
			jpeg_finish_decompress(&cinfo);
			} // if
		} // if

	jpeg_destroy_decompress(&cinfo);

	if(InterleaveBuf) delete [] InterleaveBuf;
	InterleaveBuf = NULL;
	} // if


if(ImageJPGRsc)
	{
	FreeResource(ImageJPGRsc);
	ImageJPGRsc = NULL;
	} // if

return(Success);
} // LoadJpegResourceAsRGB


// *********************************************************************
//
// Code to support loading and decompressing a PNG out of resource data
//
// *********************************************************************



static void *ImagePNGRsc;
unsigned char *ImagePNGRscCur;
static int ImagePNGSize;

void PNG_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
// no error-checking done here!
memcpy(data, ImagePNGRscCur, length);
ImagePNGRscCur += length;
} // PNG_user_read_data



// code taken from LoadJpegResourceAsRGB and WCS's ImageInputFormat.cpp
// memory is allocated with operator new[], caller is responsible for freeing
int LoadPNGResourceAsRGBA(unsigned short ImageID, unsigned char *&Red, unsigned char *&Green, unsigned char *&Blue, unsigned char *&Alpha, unsigned long int &Width, unsigned long int &Height)
{
int Success = 0;
HRSRC  ResHandle = NULL;
png_structp png_ptr;
png_infop info_ptr, end_info;
png_uint_32 width, height;
int bit_depth, color_type, interlace_type, number_of_passes = 1, Pass;
int row_stride = 0;
short Cols, Rows;
unsigned char *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf, *ABuf, BytesPerPixel;
WORD LOGOID = ImageID;
bool AllIsWell = false, DoAlpha = false, AlphaFailed = false;
unsigned long int InScan, PixelCol, WorkRow;



Width = Height = 0;
Red = Green = Blue = Alpha = NULL;

// passing NULL as Instance means 'me you moron'
if(ImagePNGRsc = LockResource(LoadResource(NULL, ResHandle = FindResource(NULL, MAKEINTRESOURCE(LOGOID), "PNGIMAGE"))))
	{
	ImagePNGSize = SizeofResource(NULL, ResHandle);
	ImagePNGRscCur = (unsigned char *)ImagePNGRsc; // use as byte stream

	if(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL))
		{
		if(info_ptr = png_create_info_struct(png_ptr))
			{
		    if(end_info = png_create_info_struct(png_ptr))
				{
				AllIsWell = true;
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

	if(!AllIsWell)
		{
		FreeResource(ImagePNGRsc);
		ImagePNGRsc = NULL;
		return(0);
		} // if


    if (setjmp(png_jmpbuf(png_ptr)))
		{
		// If we get here, the PNG code has signaled an error.
		// We need to clean up the PNG object, close the input file, and return.
		if(InterleaveBuf) delete [] InterleaveBuf; // AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
		InterleaveBuf = NULL;
		if(!Success)
			{
			delete [] Red; Red = NULL;
			delete [] Green; Green = NULL;
			delete [] Blue; Blue = NULL;
			delete [] Alpha; Alpha = NULL;
			} // if
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return(Success);
		} // if

    //png_init_io(png_ptr, fh); // for FILE IO
    png_set_read_fn(png_ptr, ImagePNGRsc, PNG_user_read_data); // for custom memory IO (read only)
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
        DoAlpha = true;
		BytesPerPixel = 4;
		} // if
	else
		{
		DoAlpha = false;
		BytesPerPixel = 3;
		} // else

    if (interlace_type == PNG_INTERLACE_ADAM7)
        {
		//number_of_passes = png_set_interlace_handling(png_ptr);
		//GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Interlaced PNG currently not supported.");
		AllIsWell = 0;
		} // if

	if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
		png_set_gray_to_rgb(png_ptr);
		} // if


    if(AllIsWell)
		{
		png_read_update_info(png_ptr, info_ptr);

		Width = Cols = (short)width;
		Height = Rows = (short)height;
		Red = new unsigned char[Cols * Rows];
		Green = new unsigned char[Cols * Rows];
		Blue = new unsigned char[Cols * Rows];

		if(DoAlpha)
			{
			Alpha = new unsigned char[Cols * Rows];
			if(!Alpha)
				{
				AlphaFailed = true;
				} // if
			} // if

		row_stride = width * BytesPerPixel;
		InterleaveBuf = new unsigned char[row_stride];
		if(InterleaveBuf && Red && Green && Blue && !AlphaFailed) // everything ok?
			{
			// Clear bitmaps
			memset(Red, 0, Cols * Rows);
			memset(Green, 0, Cols * Rows);
			memset(Blue, 0, Cols * Rows);
			if(DoAlpha && Alpha)
				{
				memset(Alpha, 0, Cols * Rows);
				} // if

			for(Pass = 0; Pass < number_of_passes; Pass++)
				{
				for(WorkRow = 0; WorkRow < height; WorkRow++)
					{
					RBuf = &Red[WorkRow * Cols];
					GBuf = &Green[WorkRow * Cols];
					BBuf = &Blue[WorkRow * Cols];
					if(DoAlpha) ABuf = &Alpha[WorkRow * Cols];

					if(number_of_passes != 1)
						{
						// interleave requires read-in and interleaving from our
						// buffers into the InterleaveBuf so the png code can add
						// to exisitng data
						for(InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
							{
							InterleaveBuf[InScan++] = RBuf[PixelCol];
							InterleaveBuf[InScan++] = GBuf[PixelCol];
							InterleaveBuf[InScan++] = BBuf[PixelCol];
							if(DoAlpha) InterleaveBuf[InScan++] = ABuf[PixelCol];
							} // for
						} // if
					png_read_row(png_ptr, InterleaveBuf, NULL);
					for(InScan = PixelCol = 0; PixelCol < (unsigned)Cols; PixelCol++)
						{
						RBuf[PixelCol] = InterleaveBuf[InScan++];
						GBuf[PixelCol] = InterleaveBuf[InScan++];
						BBuf[PixelCol] = InterleaveBuf[InScan++];
						if(DoAlpha) ABuf[PixelCol] = InterleaveBuf[InScan++];
						} // for
					} // for
				} // for

			if((WorkRow == height) && (Pass == number_of_passes))
				{
				Success = 1;
				png_read_end(png_ptr, end_info);
				} // if
			} // if
		} // if AllIsWell

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	if(InterleaveBuf) delete [] InterleaveBuf;
	InterleaveBuf = NULL;
	} // if


if(ImagePNGRsc)
	{
	FreeResource(ImagePNGRsc);
	ImagePNGRsc = NULL;
	} // if

return(Success);
} // LoadPNGResourceAsRGBA
