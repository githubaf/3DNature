#undef STATIC_FCN
#define STATIC_FCN

#include "vgl.h"
#include "vgl_internals.h"


/*****************************************************************************/
/*****************************************************************************/
/************************************************************************/
/*                         font.h                                       */
/************************************************************************/
#if 0
struct font
  {
    short width, height;	/* Width and height in pixels of font. */
    unsigned char *bitmaps;	/* Bitmap data of the font. */
    short min_char, max_char;	/* Minimum and maximum character. */
    unsigned short *offsets;	/* Offset table for each character. */
    char line_length;		/* Bytes per scanline in the font. */
  };

/* A fast font is less compact in memory, but much faster to draw.
   The font must be 32 pixels or less wide. */

struct ffont
  {
    short width, height;	/* Width and height in pixels of font. */
    unsigned long *bitmaps;	/* Bitmap data of the font. */
    short min_char, max_char;	/* Minimum and maximum character. */
    unsigned short *offsets;	/* Offset table for each character. */
  };
#endif

STATIC_FCN void vgl_dumb_text (PIXMAP * p, int x, int y, char *string); // used locally only -> static, AF 30.7.2021
STATIC_FCN void vgl_dumb_text2 (PIXMAP * p, int x, int y, char *string, int length); // used locally only -> static, AF 30.7.2021

/* An image. */

struct image
  {
    ulong *data;		/* Bit data of image. */
    int width;
    int height;
    int depth;
    int words_per_line;
  };

struct gc
  {
    ulong foreground;
    ulong background;
    ulong fore_8;
    ulong back_8;
    ulong expand_8[16];	/* Expand for expanding 4 pixels into
				   a longword. */
  };


static void copy_plane_1_to_8 (PIXMAP * p, struct image *src_image,
			       int srcx,
			       int srcy,
			       int destx,
			       int desty,
			       int width,
			       int height);


/************************************************************************/
/* Processor tuning parameters.  Risc machines usually have both of
   these. */
#undef LARGE_INSTRUCTION_CACHE
#undef FAST_CONSTANT_OFFSET_MODE

/************************************************************************/
/*                      #include "fastblt.h"                            */
/************************************************************************/
/*
 * Fast bitblt macros for certain hardware.  If your machine has an addressing
 * mode of small constant + register, you'll probably want this magic specific
 * code.  It's 25% faster for the R2000.  I haven't studied the Sparc
 * instruction set, but I suspect it also has this addressing mode.  Also,
 * unrolling the loop by 32 is possibly excessive for mfb. The number of times
 * the loop is actually looped through is pretty small.
 */

/*
 * WARNING:  These macros make *a lot* of assumptions about
 * the environment they are invoked in.  Plenty of implicit
 * arguments, lots of side effects.  Don't use them casually.
 */

#define SwitchOdd(n) case n: BodyOdd(n)
#define SwitchEven(n) case n: BodyEven(n)

/* to allow mfb and cfb to share code... */
#ifndef BitRight
#define BitRight(a,b) SCRRIGHT(a,b)
#define BitLeft(a,b) SCRLEFT(a,b)
#endif

#ifdef LARGE_INSTRUCTION_CACHE
#define UNROLL 8
#define PackedLoop \
    switch (nl & (UNROLL-1)) { \
    SwitchOdd( 7) SwitchEven( 6) SwitchOdd( 5) SwitchEven( 4) \
    SwitchOdd( 3) SwitchEven( 2) SwitchOdd( 1) \
    } \
    while ((nl -= UNROLL) >= 0) { \
	LoopReset \
	BodyEven( 8) \
    	BodyOdd( 7) BodyEven( 6) BodyOdd( 5) BodyEven( 4) \
    	BodyOdd( 3) BodyEven( 2) BodyOdd( 1) \
    }
#else
#define UNROLL 4
#define PackedLoop \
    switch (nl & (UNROLL-1)) { \
    SwitchOdd( 3) SwitchEven( 2) SwitchOdd( 1) \
    } \
    while ((nl -= UNROLL) >= 0) { \
	LoopReset \
    	BodyEven( 4) \
    	BodyOdd( 3) BodyEven( 2) BodyOdd( 1) \
    }
#endif

#define DuffL(counter,label,body) \
    switch (counter & 3) { \
    label: \
        body \
    case 3: \
	body \
    case 2: \
	body \
    case 1: \
	body \
    case 0: \
	if ((counter -= 4) >= 0) \
	    goto label; \
    }


/************************************************************************/
/* Make a fast font from a regular font. */
#ifdef UNUSED_FUNCTIONS  // AF, not used 19.July 2021
struct vgl_ffont *
vgl_expand_font (struct vgl_font *input)
{
  int nchars = input->max_char - input->min_char + 1;
  int line_length = input->line_length;
  int ch, line;
  unsigned char *src;
  unsigned long *dest;
  int shift;
  int offset;
  int byte;
  struct vgl_ffont *ff = vgl_malloc (sizeof (struct vgl_ffont));

  if(!ff)
  {
      return NULL;  // vgl_malloc() can fail, AF, 14.July 2021
  }

  if (input->width > 32)
    {
      /*
         fprintf (stderr, "Fast fonts must be <= 32 pixels wide.\n");
         exit (1);
       */
      if(ff)
      {
          vgl_free(ff);   // avaif memory leak, AF 24.July 2021
      }
      return (NULL);
    }

  ff->width = input->width;
  ff->height = input->height;
  ff->min_char = input->min_char;
  ff->max_char = input->max_char;
  ff->offsets = vgl_malloc ((sizeof (short *)) * nchars);
  ff->bitmaps = vgl_malloc ((sizeof (long)) * ff->height * nchars);

  offset = 0;
  for (ch = 0; ch < nchars; ch++)
    {
      src = &input->bitmaps[input->offsets[ch]];
      ff->offsets[ch] = offset;
      dest = &ff->bitmaps[offset];

      for (line = 0; line < ff->height; line++)
	{
	  *dest = 0;
	  for (byte = 0, shift = 24;
	       byte < line_length;
	       byte++, shift -= 8)
	    *dest |= *src++ << shift;
	  dest++;
	  offset++;
	}
    }

  return ff;
}
#endif
/************************************************************************/
/* Below are all of the pixel manipulation routines. */

/* First some macros to make this slightly portable. */
#define BITS_TO_LONGS(x)  (((x) + 31) >> 5)
#define LONGS_TO_BYTES(x) ((x) << 2)

/* Desired alignment (in pixels) of the destination framebuffer.
   Needs to be a power of 2. */
#define PPW     4
#define PIM     0x03
#define PMSK    0xff
#define PWSH	2
#define StartMaskTab cfbstarttab
#define EndMaskTab cfbendtab
#define StartMaskPartial cfbstartpartial
#define EndMaskPartial cfbendpartial

#define mask_partial_bits(x, width, startmask)			\
   (startmask) = (StartMaskPartial[(x) & PIM]			\
		  & EndMaskPartial[((x) + (width)) & PIM]);
#define mask_bits(x, width, startmask, endmask, nl)		\
   (startmask) = (StartMaskTab[(x) & PIM]);			\
   (endmask) = (EndMaskTab[((x) + (width)) & PIM]);		\
   if (startmask)						\
     (nl) = ((width) - (PPW - ((x) & PIM))) >> PWSH;		\
   else								\
     (nl) = (width) >> PWSH;

/************************************************************************/
/* Draw a string of text at a given position on the display.  No
   clipping is done. */
STATIC_FCN void 
vgl_dumb_text2 (PIXMAP * p, int x, int y, char *string, int length) // used locally only -> static, AF 30.7.2021
{
  int tmp_bitmap_length;	/* Words in one line of tmp_bitmap. */
  int tmp_bitmap_size;		/* Number of longs in the tmp_bitmap. */
  ulong *tmp_bitmap;		/* The temporary bitmap. */
  int bit_left;			/* Left edge of the bitmap. */
  int i;
  struct vgl_ffont *font;

  font = p->font;

  /* Allocate a temporary bitmap to hold the rendered characters. */
  tmp_bitmap_length = BITS_TO_LONGS (length * font->width + PPW);
  tmp_bitmap_size = LONGS_TO_BYTES (tmp_bitmap_length * font->height);
  tmp_bitmap = vgl_malloc (tmp_bitmap_size);
  vgl_bzero (tmp_bitmap, tmp_bitmap_size);

  /* This saves so little time it's hardly worth it. */
  /*bit_left = (x & (PPW - 1)); */
  bit_left = 0;

  /* Blt each character into the temporary bitmap. */
  {
    int col, line;
    int height = font->height;
    ulong *src;
    ulong *dest;
    int shift, lshift;

    col = bit_left;
    for (i = 0; i < length; i++)
      {
	if (string[i] < font->min_char || string[i] > font->max_char)
	  src = &font->bitmaps[font->offsets[' ' - font->min_char]];
	else
	  src = &font->bitmaps[font->offsets[string[i] - font->min_char]];

	dest = tmp_bitmap + (col >> 5);
	if ((col & 31) + font->width <= 32)
	  {
	    shift = col & 31;
	    /* The character fits within a longword. */
	    line = height;
	    DuffL (line, label1,
		   *dest |= *src >> shift;
		   dest += tmp_bitmap_length;
		   src++;
	      );
	  }
	else
	  {
	    shift = col & 31;
	    lshift = 32 - shift;
	    /* The character spans 2 longwords. */
	    line = height;
	    DuffL (line, label2,
		   *dest |= *src >> shift;
		   dest[1] |= *src << lshift;
		   dest += tmp_bitmap_length;
		   src++;
	      );
	  }

	col += font->width;
      }
  }
  {
    struct image src_image;

    src_image.data = tmp_bitmap;
    src_image.width = 100;	/* Doesn't matter, doesn't clip. */
    src_image.height = font->height;
    src_image.words_per_line = tmp_bitmap_length;

    copy_plane_1_to_8 (p, &src_image,
		     bit_left, 0, x, y, length * font->width, font->height);
  }

  vgl_free (tmp_bitmap);
}

/************************************************************************/
/* 8 bit framebuffer support. */
/* NOTE:
   the first element in starttab could be 0xffffffff.  making it 0
   lets us deal with a full first word in the middle loop, rather
   than having to do the multiple reads and masks that we'd
   have to do if we thought it was partial.
 */
static unsigned int cfbstarttab[] =
{
  0x00000000,
  0x00FFFFFF,
  0x0000FFFF,
  0x000000FF
};

static unsigned int cfbendtab[] =
{
  0x00000000,
  0xFF000000,
  0xFFFF0000,
  0xFFFFFF00
};

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
 */
static unsigned int cfbstartpartial[] =
{
  0xFFFFFFFF,
  0x00FFFFFF,
  0x0000FFFF,
  0x000000FF
};

static unsigned int cfbendpartial[] =
{
  0xFFFFFFFF,
  0xFF000000,
  0xFFFF0000,
  0xFFFFFF00
};



/************************************************************************/
/* Copy from a 1 bit image to an 8 bit image. */
static void 
copy_plane_1_to_8 (PIXMAP * p, struct image *src_image,
		   int srcx,
		   int srcy,
		   int destx,
		   int desty,
		   int width,
		   int height)
{
  ulong *src;
  ulong *dest;
  ulong *src_line, *dest_line;
  ulong bits, tmp;
  int left_shift;
  int right_shift;
  int nl;
  ulong startmask, endmask;
  int src_linelen, dest_linelen;
  int xoff_src, xoff_dest;
  int firstoff, secondoff;
  int nlMiddle;
  unsigned long mouse;


  if (p->clip_setting == CLIP_ON)
    {
      if (destx < p->clip_x_min)
	{
	  srcx += p->clip_x_min - destx;
	  width -= p->clip_x_min - destx;
	  destx = p->clip_x_min;
	}

      if (desty < p->clip_y_min)
	{
	  srcy += p->clip_y_min - desty;
	  height -= p->clip_y_min - desty;
	  desty = p->clip_y_min;
	}

      if (destx + width > p->clip_x_max)
	{
	  width -= (destx + width) - p->clip_x_max;
	}

      if (desty + height > p->clip_y_max)
	{
	  height -= (desty + height) - p->clip_y_max;
	}

      if (width <= 0 || height <= 0)
	return;
    }

  check_mouse1 (p, destx, desty, destx + width, destx + height, mouse);

#define Get_Four_Pixels(x)  p->expand_8[(ulong) (x) >> 28]
#define Next_Four_Bits(x)  ((x) <<= 4)

  src_linelen = src_image->words_per_line;
  dest_linelen = p->pixdata->x_size >> 2;

  src_line = src_image->data + srcy * src_linelen + (srcx >> 5);
  dest_line = ((ulong *) p->pixdata->data) + desty*dest_linelen + (destx>>2);

  xoff_src = srcx & 0x1f;
  xoff_dest = destx & 0x3;

  if (xoff_dest + width < 4)
    {
      /* maskpartialbits (xoff_dest, width, startmask); */
      mask_partial_bits (xoff_dest, width, startmask);
      endmask = 0;
      nlMiddle = 0;
    }
  else
    {
      mask_bits (xoff_dest, width, startmask, endmask, nlMiddle);
    }

  if (startmask)
    {
      firstoff = xoff_src - xoff_dest;
      if (firstoff > 28)
	secondoff = 32 - firstoff;
      if (xoff_dest)
	{
	  srcx += (4 - xoff_dest);
	  destx += (4 - xoff_dest);
	  xoff_dest = srcx & 0x3;
	}
    }

  left_shift = xoff_dest;
  right_shift = 32 - left_shift;
  while (height--)
    {
      src = src_line;
      dest = dest_line;
      src_line += src_linelen;
      dest_line += dest_linelen;
      bits = *src++;

      if (startmask)
	{
	  if (firstoff < 0)
	    tmp = bits >> -firstoff;
	  else
	    {
	      tmp = bits << firstoff;
	      if (firstoff >= 28)
		{
		  bits = *src++;
		  if (firstoff != 28)
		    tmp |= bits >> secondoff;
		}
	    }
	  *dest = (*dest & ~startmask) | (Get_Four_Pixels (tmp) & startmask);
	  dest++;
	}

      nl = nlMiddle;
      while (nl >= 8)
	{
	  nl -= 8;
	  tmp = bits << left_shift;
	  bits = *src++;
	  if (right_shift != 32)
	    tmp |= bits >> right_shift;
	  dest[0] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[1] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[2] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[3] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[4] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[5] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[6] = Get_Four_Pixels (tmp);
	  Next_Four_Bits (tmp);
	  dest[7] = Get_Four_Pixels (tmp);
	  dest += 8;
	}

      if (nl || endmask)
	{
	  tmp = bits << left_shift;
	  if (right_shift != 32)
	    {
	      bits = *src++;
	      tmp |= bits >> right_shift;
	    }

	  dest += nl;
	  switch (nl)
	    {
	    case 7:
	      dest[-7] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 6:
	      dest[-6] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 5:
	      dest[-5] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 4:
	      dest[-4] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 3:
	      dest[-3] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 2:
	      dest[-2] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    case 1:
	      dest[-1] = Get_Four_Pixels (tmp);
	      Next_Four_Bits (tmp);
	    }
	  if (endmask)
	    {
	      *dest = (*dest & ~endmask) | (Get_Four_Pixels (tmp) & endmask);
	    }
	}
    }

  check_mouse2 (p, mouse);
}


/***************************************************************************/
STATIC_FCN void 
vgl_dumb_text (PIXMAP * p, int x, int y, char *string) // used locally only -> static, AF 30.7.2021
{
  char *s;

  s = string;
  while (*s != '\0')
    s++;

  vgl_dumb_text2 (p, x, y, string, (int) (s - string));
}
