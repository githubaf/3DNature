
#include "vgl.h"
#include "vgl_internals.h"


/*****************************************************************************/
/*****************************************************************************/
#include "fastblt.h"
/*****************************************************************************/
/*****************************************************************************/
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

/*****************************************************************************/
/*****************************************************************************/


/* Text drawing routines for a framebuffer. */
/* Processor tuning parameters.  Risc machines usually have both of
   these. */
#undef  LARGE_INSTRUCTION_CACHE
#define FAST_CONSTANT_OFFSET_MODE


/* Below are all of the pixel manipulation routines. */

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


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Copy a region to another region.  The regions must have the screen
   image bit depth.  The region may be in the same image and overlap. */
/*
   copy_blt (PIXMAP *src_image,
   PIXMAP *dest_image,
   int srcx,
   int srcy,
   int destx,
   int desty,
   int width,
   int height)
 */
#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
void 
vgl_dumb_bitblt_core (PIXMAP * src_image, int srcx, int srcy, int width, int height, PIXMAP * dest_image, int destx, int desty)
{
  ulong *src, *dest, *src_line, *dest_line;
  int src_linelen, dest_linelen;
  int xdir;			/* 1 = left right, -1 = right left. */
  int ydir;			/* 1 = top down, -1 = bottom up. */
  int careful;
  ulong startmask, endmask;
  int nl, nlMiddle;
  int xoff_src, xoff_dest;
  int left_shift, right_shift;
  ulong bits, bits1;

  src_linelen = src_image->pixdata->x_size / 4;
  dest_linelen = dest_image->pixdata->x_size / 4;

  careful = (src_image->pixdata == dest_image->pixdata);

  if (careful && srcx < destx)
    xdir = -1;
  else
    xdir = 1;

  if (careful && srcy < desty)
    {
      ydir = -1;
      /* Start a last scanline of rectangle. */
      src_line = ((unsigned long *) src_image->pixdata->data) + ((srcy + height - 1) * src_linelen);
      dest_line = ((unsigned long *) dest_image->pixdata->data) + ((desty + height - 1) * dest_linelen);
      src_linelen = -src_linelen;
      dest_linelen = -dest_linelen;
    }
  else
    {
      ydir = 1;
      src_line = ((unsigned long *) src_image->pixdata->data) + srcy * src_linelen;
      dest_line = ((unsigned long *) dest_image->pixdata->data) + desty * dest_linelen;
    }

  if ((destx & PIM) + width <= PPW)
    {
      mask_partial_bits (destx, width, startmask);
      endmask = 0;
      nlMiddle = 0;
    }
  else
    {
      mask_bits (destx, width, startmask, endmask, nlMiddle);
    }

  if (xdir == 1)
    {
      xoff_src = srcx & PIM;
      xoff_dest = destx & PIM;
      src_line += srcx >> PWSH;
      dest_line += destx >> PWSH;
      if (xoff_src == xoff_dest)
	{
	  while (height--)
	    {
	      src = src_line;
	      dest = dest_line;
	      src_line += src_linelen;
	      dest_line += dest_linelen;
	      if (startmask)
		{
		  *dest = (*dest & ~startmask) |(*src & startmask);
		  dest++;
		  src++;
		}

	      nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE
	      src += nl & (UNROLL - 1);
	      dest += nl & (UNROLL - 1);
#define BodyOdd(n) dest[-n] = src[-n];
#define BodyEven(n) BodyOdd(n)
#define LoopReset   dest += UNROLL; src += UNROLL;
#else
#define BodyOdd(n)  *dest++ = *src++;
#define BodyEven(n)  BodyOdd(n)
#define LoopReset   ;
#endif
	      /* The semicolon doesn't hurt anything, and lets this
	         indent ok. */
	      PackedLoop;
#undef BodyOdd
#undef BodyEven
#undef LoopReset
#else
	      DuffL (nl, label1, *dest++ = *src++;
		);
#endif

	      if (endmask)
		{
		  *dest = (*dest & ~endmask) | (*src & endmask);
		}
	    }
	}
      else
	{
	  /* The bits don't line up so do the shifts. */
	  if (xoff_src > xoff_dest)
	    {
	      left_shift = (xoff_src - xoff_dest) << (5 - PWSH);
	      right_shift = 32 - left_shift;
	    }
	  else
	    {
	      right_shift = (xoff_dest - xoff_src) << (5 - PWSH);
	      left_shift = 32 - right_shift;
	    }

	  while (height--)
	    {
	      src = src_line;
	      dest = dest_line;
	      src_line += src_linelen;
	      dest_line += dest_linelen;
	      bits = 0;
	      if (xoff_src > xoff_dest)
		bits = *src++;
	      if (startmask)
		{
		  bits1 = bits << left_shift;
		  bits = *src++;
		  bits1 |= bits >> right_shift;
		  *dest = (*dest & ~startmask) | (bits1 & startmask);
		  dest++;
		}
	      nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
	      bits1 = bits;

#ifdef FAST_CONSTANT_OFFSET_MODE
	      src += nl & (UNROLL - 1);
	      dest += nl & (UNROLL - 1);
#define BodyOdd(n) \
	      bits = src[-n]; \
	      dest[-n] = (bits1 << left_shift) | (bits >> right_shift);
#define BodyEven(n) \
	      bits1 = src[-n]; \
	      dest[-n] = (bits << left_shift) | (bits1 >> right_shift);
#define LoopReset  dest += UNROLL; src += UNROLL;
#else
#define BodyOdd(n) \
	      bits = *src++; \
	      *dest++ = (bits1 << left_shift) | (bits >> right_shift);
#define BodyEven(n) \
	      bits1 = *src++; \
	      *dest++ = (bits << left_shift) | (bits1 >> right_shift);
#define LoopReset   ;
#endif
	      /* The semicolon doesn't hurt anything, and lets this
	         indent ok. */
	      PackedLoop;
#undef BodyOdd
#undef BodyEven
#undef LoopReset
#else
	      DuffL (nl, label2,
		     bits1 = bits << left_shift;
		     bits = *src++;
		     *dest = bits1 | (bits >> right_shift);
		     dest++;
		);
#endif

	      if (endmask)
		{
		  bits1 = bits << left_shift;
		  if (endmask << right_shift)
		    {
		      bits = *src;
		      bits1 |= bits >> right_shift;
		    }
		  *dest = (*dest & ~endmask) | (bits1 & endmask);
		}
	    }
	}
    }
  else
    {
      /* xdir == -1 */
      xoff_src = (srcx + width - 1) & PIM;
      xoff_dest = (destx + width - 1) & PIM;
      dest_line += ((destx + width - 1) >> PWSH) + 1;
      src_line += ((srcx + width - 1) >> PWSH) + 1;

      if (xoff_src == xoff_dest)
	{
	  while (height--)
	    {
	      src = src_line;
	      dest = dest_line;
	      src_line += src_linelen;
	      dest_line += dest_linelen;
	      if (endmask)
		{
		  dest--;
		  src--;
		  *dest = (*dest & ~endmask) | (*src & endmask);
		}

	      nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE
	      src -= nl & (UNROLL - 1);
	      dest -= nl & (UNROLL - 1);
#define BodyOdd(n) dest[n-1] = src[n-1];
#define BodyEven(n) BodyOdd(n)
#define LoopReset   dest -= UNROLL; src -= UNROLL;
#else
#define BodyOdd(n)  *--dest = *--src;
#define BodyEven(n)  BodyOdd(n)
#define LoopReset   ;
#endif
	      /* The semicolon doesn't hurt anything, and lets this
	         indent ok. */
	      PackedLoop;
#undef BodyOdd
#undef BodyEven
#undef LoopReset
#else
	      DuffL (nl, label3, *--dest = *--src;
		);
#endif

	      if (startmask)
		{
		  dest--;
		  src--;
		  *dest = (*dest & ~startmask) | (*src & startmask);
		}
	    }
	}
      else
	{
	  /* The transfers aren't aligned. */
	  if (xoff_dest > xoff_src)
	    {
	      right_shift = (xoff_dest - xoff_src) << (5 - PWSH);
	      left_shift = 32 - right_shift;
	    }
	  else
	    {
	      left_shift = (xoff_src - xoff_dest) << (5 - PWSH);
	      right_shift = 32 - left_shift;
	    }

	  while (height--)
	    {
	      src = src_line;
	      dest = dest_line;
	      src_line += src_linelen;
	      dest_line += dest_linelen;
	      bits = 0;
	      if (xoff_dest > xoff_src)
		bits = *--src;
	      if (endmask)
		{
		  bits1 = bits >> right_shift;
		  bits = *--src;
		  bits1 |= bits << left_shift;
		  dest--;
		  *dest = (*dest & ~endmask) | (bits1 & endmask);
		}
	      nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
	      bits1 = bits;

#ifdef FAST_CONSTANT_OFFSET_MODE
	      src -= nl & (UNROLL - 1);
	      dest -= nl & (UNROLL - 1);
#define BodyOdd(n) \
	      bits = src[n-1]; \
	      dest[n-1] = (bits1 >> right_shift) | (bits << left_shift);
#define BodyEven(n) \
	      bits1 = src[n-1]; \
	      dest[n-1] = (bits >> right_shift) | (bits1 << left_shift);
#define LoopReset  dest -= UNROLL; src -= UNROLL;
#else
#define BodyOdd(n) \
	      bits = *--src; \
	      *--dest = (bits1 << left_shift) | (bits >> right_shift);
#define BodyEven(n) \
	      bits1 = *--src; \
	      *--dest = (bits << left_shift) | (bits1 >> right_shift);
#define LoopReset   ;
#endif
	      /* The semicolon doesn't hurt anything, and lets this
	         indent ok. */
	      PackedLoop;
#undef BodyOdd
#undef BodyEven
#undef LoopReset
#else
	      DuffL (nl, label4,
		     bits1 = bits >> right_shift;
		     bits = *--src;
		     --dest;
		     *dest = bits1 | (bits << left_shift);
		);
#endif

	      if (startmask)
		{
		  bits1 = bits >> right_shift;
		  if (startmask >> left_shift)
		    {
		      bits = *--src;
		      bits1 |= bits << left_shift;
		    }
		  --dest;
		  *dest = (*dest & ~startmask) | (bits1 & startmask);
		}
	    }
	}
    }
}
#endif
