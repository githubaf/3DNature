
#include <math.h>
#include "vgl.h"
#include "vgl_internals.h"

#define FAST_vgl_dumb_set_pixel_noclip(pixmap,xxx,yyy)   \
	{*(((unsigned char *)(pixmap->pixdata->data)) + \
	   ((xxx) + (yyy)*pixmap->pixdata->x_size)) = \
	   pixmap->foreground;}


#define FAST_vgl_dumb_set_pixel(pixmap,xxx,yyy)		\
	{if( !pixmap->clip_on || gen_outcodes(pixmap,(xxx),(yyy))==0 ) \
   	*(((unsigned char *)(pixmap->pixdata->data)) + \
	  ((xxx) + (yyy)*pixmap->pixdata->x_size)) = \
	   pixmap->foreground;}

/* #define BRESENHAM_LINE */

static void vgl_dumb_fillrect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);  // used locally only -> static, AF 19.7.202
static void vgl_dumb_fillrect (PIXMAP * p, int x1, int y1, int x2, int y2); // used locally only -> static, AF 19.7.2021
static void vgl_dumb_set_pixel (PIXMAP * p, int x, int y); // used locally only -> static, AF 19.7.2021
static void vgl_dumb_rect_noclip (PIXMAP * p, int x1, int y1, int x2, int y2); // used locally only -> static, AF 20.7.2021
static void vgl_dumb_transbitblt_core (PIXMAP *source, int sx, int sy,
                                       int width, int height,
                                       PIXMAP *dest, int dx, int dy, int mask_color); // used locally only -> static, AF 20.7.2021
static void vgl_dumb_nothing (PIXMAP * p); // used locally only -> static, AF 20.7.2021
static void vgl_dumb_arc_noclip (PIXMAP * p, int x, int y, int x_axis, int y_axis, int start, int end); // used locally only -> static, AF 20.7.2021

#ifndef	INLINE_CHECK_MOUSE
unsigned long 
check_mouse1_internal (PIXMAP * pixmap, int x1, int y1, int x2, int y2)
{
  unsigned long temp;
/*
  check_mouse1 (pixmap, x1, y1, x2, y2, temp);
*/

  if( (pixmap)->board!=NULL)
    {
      tick_ss( (pixmap)->board);
      if( (pixmap)->mouse_nocheck_flag==0 && (pixmap)->board->mouse_ptr!=NULL)
	{
	  if( 1 /*(pixmap)->board->mouse_blanking_level==0*/)
	    {
	      if( (x1) <= (pixmap)->board->mouse_old_x)
		{
		  if( (pixmap)->board->mouse_old_x <= (x2))
		    {
		      if( (y1) <= (pixmap)->board->mouse_old_y)
			{
			  if( (pixmap)->board->mouse_old_y <= (y2))
			    {
			      vgl_mouse_ptr_off( (pixmap)->board);
			      temp=1;
			    }
			  else
			    temp=0;
			}
		      else
			{
			  if( (y1) < ((pixmap)->board->mouse_old_y + 
				      vgl_y_res((pixmap)->board->mouse_ptr)))
			    {
			      vgl_mouse_ptr_off( (pixmap)->board);
			      temp=1;
			    }
			  else
			    temp=0;
			}
		    }
		  else
		    temp=0;
		}
	      else
		{
		  if( (x1) < ((pixmap)->board->mouse_old_x +
			      vgl_x_res((pixmap)->board->mouse_ptr))
		    {
		      if( (y1) <= (pixmap)->board->mouse_old_y)
			{
			  if( (pixmap)->board->mouse_old_y <= (y2))
			    {
			      vgl_mouse_ptr_off( (pixmap)->board);
			      temp=1; 
			    }
			  else
			    temp=0;
			}
		      else 
			{ 
			  if( (y1) < ((pixmap)->board->mouse_old_y +
				      vgl_y_res((pixmap)->board->mouse_ptr)))
			    {
			      vgl_mouse_ptr_off( (pixmap)->board);
			      temp=1;
			    }
			  else
			    temp=0;
			}
		    }
		  else
		    temp=0;
		}
	    }
	  else /* Else mouse blanking level>0 */
	    temp=0;
	}
      else /* else p->mouse_nocheck_flag is set... */
	temp=0;
    }
  else  /* else p->board is not valid */
    temp=0;
  
  return (temp);
}
#endif


/***************************************************************************/
/******** Routines for dumb frame buffers and pixmaps in host RAM **********/
/***************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 19.July 2021
void 
vgl_dumb_set_clip (PIXMAP * p, int x1, int y1, int x2, int y2)
{
  /* Adjust for "auto selected" values */
  if (x1 < 0)
    x1 = 0;

  if (x2 < 0)
    x2 = p->pixdata->x_res;

  if (y1 < 0)
    y1 = 0;

  if (y2 < 0)
    y2 = p->pixdata->y_res;


  /* Adjust maximums and minimums */
  p->clip_x_min = min (x1, x2);
  p->clip_x_max = max (x1, x2);
  p->clip_y_min = min (y1, y2);
  p->clip_y_max = max (y1, y2);
}
#endif

/****************************************************************************/
void 
vgl_dumb_clip_off (PIXMAP * p)
{
  p->clip_setting = CLIP_OFF;

  p->set_pixel = vgl_dumb_set_pixel_noclip;
  p->get_pixel = vgl_dumb_get_pixel_noclip;
  p->line = vgl_dumb_line_noclip;
  p->hline = vgl_dumb_hline_noclip;
  p->vline = vgl_dumb_vline_noclip;
  p->rect = vgl_dumb_rect_noclip;
  p->fillrect = vgl_dumb_fillrect_noclip;
  p->circle = vgl_dumb_circle_noclip;
  p->fillcircle = vgl_dumb_fillcircle_noclip;
  p->ellipse = vgl_dumb_ellipse_noclip;
  p->fillellipse = vgl_dumb_fillellipse_noclip;
  p->arc = vgl_dumb_arc_noclip;
  p->fillarc = vgl_dumb_fillarc_noclip;
  p->poly = vgl_dumb_poly_noclip;
  p->fillpoly = vgl_dumb_fillpoly_noclip;
  p->bitblt = vgl_dumb_bitblt_noclip;
  p->transbitblt = vgl_dumb_transbitblt_noclip;
  p->setfont = vgl_dumb_setfont;
  p->text = vgl_dumb_text;
  p->text2 = vgl_dumb_text2;
  p->nothing = vgl_dumb_nothing;
  p->hlinelist = vgl_dumb_hlinelist_noclip;
}

/****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 19.July 2021
void 
vgl_dumb_clip_on (PIXMAP * p)
{
  p->clip_setting = CLIP_ON;

  p->set_pixel = vgl_dumb_set_pixel;
  p->get_pixel = vgl_dumb_get_pixel;
  p->line = vgl_dumb_line;
  p->hline = vgl_dumb_hline;
  p->vline = vgl_dumb_vline;
  p->rect = vgl_dumb_rect;
  p->fillrect = vgl_dumb_fillrect;
  p->circle = vgl_dumb_circle;
  p->fillcircle = vgl_dumb_fillcircle;
  p->ellipse = vgl_dumb_ellipse;
  p->fillellipse = vgl_dumb_fillellipse;
  p->arc = vgl_dumb_arc;
  p->fillarc = vgl_dumb_fillarc;
  p->poly = vgl_dumb_poly;
  p->fillpoly = vgl_dumb_fillpoly;
  p->bitblt = vgl_dumb_bitblt;
  p->transbitblt = vgl_dumb_transbitblt;
  p->setfont = vgl_dumb_setfont;
  p->text = vgl_dumb_text;
  p->text2 = vgl_dumb_text2;
  p->nothing = vgl_dumb_nothing;
  p->hlinelist = vgl_dumb_hlinelist;
}
#endif

/*****************************************************************************/
void 
vgl_dumb_setcur (PIXMAP * p, int fg, int bg)
{
  int i;

  static unsigned long expand_src[16] =
  {
    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
  };

  p->foreground = fg;
  p->background = bg;
  p->fore_8 = (p->foreground | (p->foreground << 8) | 
	       (p->foreground << 16) | (p->foreground << 24));
  p->back_8 = (p->background | (p->background << 8) | 
	       (p->background << 16) | (p->background << 24));

  for (i = 0; i < 16; i++)
    p->expand_8[i] = ((expand_src[i] & p->fore_8) | 
		      (~expand_src[i] & p->back_8));
}


/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 19.July 2021
int 
vgl_dumb_pixerror (PIXMAP * p)
{
  int err;

  err = p->error;
  p->error = VGL_ERR_NONE;
  return (err);
}
#endif

/*****************************************************************************/
static void 
vgl_dumb_nothing (PIXMAP * p) // used locally only -> static, AF 20.7.2021
{
}

/*****************************************************************************/
static void 
vgl_dumb_set_pixel (PIXMAP * p, int x, int y) // used locally only -> static, AF 19.7.2021
{
  unsigned long mouse;


  if (!p->clip_on || gen_outcodes (p, x, y) == 0)
    {
      check_mouse1 (p, x, y, x, y, mouse);
      *(((unsigned char *)(p->pixdata->data)) + 
	(x + y * p->pixdata->x_size)) = p->foreground;
      check_mouse2 (p, mouse);
    }
}


/*****************************************************************************/
int 
vgl_dumb_get_pixel (PIXMAP * p, int x, int y)
{
  int value;
  unsigned long mouse;

  if (!p->clip_on || gen_outcodes (p, x, y) == 0)
    {
      check_mouse1 (p, x, y, x, y, mouse);
      value = *(((unsigned char *) (p->pixdata->data)) + 
		(x + y * p->pixdata->x_size));
      check_mouse2 (p, mouse);
      return (value);
    }
  else
    return (-1);
}


/*****************************************************************************/
void 
vgl_dumb_set_pixel_noclip (PIXMAP * p, int x, int y)
{
  unsigned long mouse;

  check_mouse1 (p, x, y, x, y, mouse);
  *(((unsigned char *) (p->pixdata->data)) + 
    (x + y * p->pixdata->x_size)) = p->foreground;
  check_mouse2 (p, mouse);
}


/*****************************************************************************/
int 
vgl_dumb_get_pixel_noclip (PIXMAP * p, int x, int y)
{
  int value;
  unsigned long mouse;

  check_mouse1 (p, x, y, x, y, mouse);
  value = *(((unsigned char *) (p->pixdata->data)) + 
	    (x + y * p->pixdata->x_size));
  check_mouse2 (p, mouse);
  return (value);
}


/*****************************************************************************/
void 
vgl_dumb_line (PIXMAP * p, int x1, int y1, int x2, int y2)
{
  int out1, out2, dx, dy, x_orig, y_orig;

  /* This saves us from a division by zero later...  */
  /* This is also done by vgl_dumb_line_noclip */
  /*
     if( y1==y2)
     vgl_dumb_hline_noclip( p, x1, x2, y1, color);
     else if( x1==x2)
     vgl_dumb_vline_noclip( p, x1, y1, y2, color);
   */


  /* clip line here */
  out1 = gen_outcodes (p, x1, y1);
  out2 = gen_outcodes (p, x2, y2);

  if ((out1 & out2) != 0)
    {
      /* if line is entirely outside the clipping boundery, don't draw */
      return;
    }
  else if ((out1 | out2) != 0)	/* if line requires clipping... */
    {
      if(y1 > y2) /* sort/swap for better consistancy given same inp coords */
        {
        x_orig = out1;
        out1 = out2;
        out2 = x_orig;
        
        x_orig = x2;
        x2 = x1;
        x1 = x_orig;
        
        y_orig = y2;
        y2 = y1;
        y1 = y_orig;
        
        } /* if */
      else
        {
        x_orig = x1;
        y_orig = y1;
        } /* else */

      dx = x2 - x1;
      dy = y2 - y1;

      
      if (out1 & CLIP_X_MIN)
	{
	  x1 = p->clip_x_min;
	  y1 = y_orig + (dy * (x1 - x_orig)) / dx;

	  out1 = gen_outcodes (p, x1, y1);
	}
      else if (out2 & CLIP_X_MIN)
	{
	  x2 = p->clip_x_min;
	  y2 = y_orig + (dy * (x2 - x_orig)) / dx;

	  out2 = gen_outcodes (p, x2, y2);
	}

      if (out1 & CLIP_Y_MIN)
	{
	  y1 = p->clip_y_min;
	  x1 = x_orig + (dx * (y1 - y_orig)) / dy;

	  out1 = gen_outcodes (p, x1, y1);
	}
      else if (out2 & CLIP_Y_MIN)
	{
	  y2 = p->clip_y_min;
	  x2 = x_orig + (dx * (y2 - y_orig)) / dy;

	  out2 = gen_outcodes (p, x2, y2);
	}

      if (out1 & CLIP_X_MAX)
	{
	  x1 = p->clip_x_max - 1;
	  y1 = y_orig + (dy * (x1 - x_orig)) / dx;

	  out1 = gen_outcodes (p, x1, y1);
	}
      else if (out2 & CLIP_X_MAX)
	{
	  x2 = p->clip_x_max - 1;
	  y2 = y_orig + (dy * (x2 - x_orig)) / dx;

	  out2 = gen_outcodes (p, x2, y2);
	}

      if (out1 & CLIP_Y_MAX)
	{
	  y1 = p->clip_y_max - 1;
	  x1 = x_orig + (dx * (y1 - y_orig)) / dy;

	  out1 = gen_outcodes (p, x1, y1);
	}
      else if (out2 & CLIP_Y_MAX)
	{
	  y2 = p->clip_y_max - 1;
	  x2 = x_orig + (dx * (y2 - y_orig)) / dy;

	  out2 = gen_outcodes (p, x2, y2);
	}

      if ((out1 | out2) != 0)
	return;
    }


  /* Once line has been clipped, call vgl_dumb_line_noclip */
  vgl_dumb_line_noclip (p, x1, y1, x2, y2);
}


/*****************************************************************************/
#ifdef	BRESENHAM_LINE
void 
vgl_dumb_line_noclip (PIXMAP * p, int xa, int ya, int xb, int yb)
{
  int dx, dy, ady, adx, r, c, f, g, inc1, inc2, pos_slope, color;
  unsigned char *loc;

  if (xa == xb)
    {
      vgl_dumb_vline_noclip (p, xa, ya, yb);
      return;
    }
  else if (ya == yb)
    {
      vgl_dumb_hline_noclip (p, xa, xb, ya);
      return;
    }

  color = (unsigned char) p->foreground;
  dx = xb - xa;
  adx = abs (dx);
  dy = yb - ya;
  ady = abs (dy);

  pos_slope = (dx > 0);
  if (dy < 0)
    pos_slope = !pos_slope;

  if (adx > ady)
    {				/* Slope is more or less horizontal */
      if (dx > 0)
	{
	  c = xa;
	  r = ya;
	  f = xb;
	}
      else
	{
	  c = xb;
	  r = yb;
	  f = xa;
	}

      inc1 = 2 * ady;
      g = 2 * ady - adx;
      inc2 = 2 * (ady - adx);
      loc = (((unsigned char *) (p->pixdata->data)) + 
	     (c + r * p->pixdata->x_size));

      if (pos_slope)
	{
	  while (c <= f)
	    {
	      *loc = color;

	      c++;
	      loc++;

	      if (g >= 0)
		{
		  loc += p->pixdata->x_size;
		  g += inc2;
		}
	      else
		g += inc1;
	    }
	}
      else
	{
	  while (c <= f)
	    {
	      *loc = color;

	      c++;
	      loc++;

	      if (g > 0)
		{
		  loc -= p->pixdata->x_size;
		  g += inc2;
		}
	      else
		g += inc1;
	    }
	}
    }
  else
    {				/* Slope is more or less vertical */
      if (dy > 0)
	{
	  c = ya;
	  r = xa;
	  f = yb;
	}
      else
	{
	  c = yb;
	  r = xb;
	  f = ya;
	}

      inc1 = 2 * adx;
      g = 2 * adx - ady;
      inc2 = 2 * (adx - ady);
      loc = (((unsigned char *) (p->pixdata->data)) + 
	     (r + c * p->pixdata->x_size));

      if (pos_slope)
	{
	  while (c <= f)
	    {
	      *loc = color;

	      c++;
	      loc += p->pixdata->x_size;

	      if (g >= 0)
		{
		  loc++;
		  g += inc2;
		}
	      else
		g += inc1;
	    }
	}
      else
	{
	  while (c <= f)
	    {
	      *loc = color;

	      c++;
	      loc += p->pixdata->x_size;

	      if (g > 0)
		{
		  loc--;
		  g += inc2;
		}
	      else
		g += inc1;
	    }
	}
    }
}
#endif

/*****************************************************************************/
static unsigned long start_mask[] =
{0xFFFFFFFF, 0x00FFFFFF, 0x0000FFFF, 0x000000FF};
static unsigned long end_mask[] =
{0xFF000000, 0xFFFF0000, 0xFFFFFF00, 0xFFFFFFFF};
static unsigned long not_start_mask[] =
{0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00};
static unsigned long not_end_mask[] =
{0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000};


void 
vgl_dumb_hline (PIXMAP * p, int x1, int x2, int y)
{
  int out1, out2;

  out1 = gen_outcodes (p, x1, y);
  out2 = gen_outcodes (p, x2, y);

  if ((out1 & out2) != 0)
    return;
  else if ((out1 | out2) != 0)
    {
      if ((out1 & CLIP_X_MIN) != 0)
	x1 = p->clip_x_min;
      else if ((out1 & CLIP_X_MAX) != 0)
	x1 = p->clip_x_max - 1;

      if ((out2 & CLIP_X_MIN) != 0)
	x2 = p->clip_x_min;
      else if ((out2 & CLIP_X_MAX) != 0)
	x2 = p->clip_x_max - 1;
    }

  vgl_dumb_hline_noclip (p, x1, x2, y);
}


/*****************************************************************************/
void 
vgl_dumb_hline_noclip (PIXMAP * p, int x1, int x2, int y)
{
  int x1_trim, x2_trim;
  unsigned long *loc1, *loc2;
  unsigned long color_mask, mouse;

  color_mask = p->fore_8;

  /* Always draw left to right */
  if (x1 > x2)
    {
      x1_trim = x1;		/* use x1_trim as a temp variable */
      x1 = x2;
      x2 = x1_trim;
    }

  check_mouse1 (p, x1, y, x2, y, mouse);

    loc1 = (((unsigned long *) (p->pixdata->data)) + 
	    ((x1 + y * p->pixdata->x_size) / 4));
  /*
    loc2=  (((unsigned long *)(p->pixdata->data)) + 
            ((x2 + y*p->pixdata->x_size)/4));
   */
  loc2 = loc1 + ((x2 >> 2) - (x1 >> 2));

  x1_trim = x1 & 3;
  x2_trim = x2 & 3;


  if (loc1 == loc2)  /* If the line falls within a single 32 bit word */
    {
      *loc1 = (*loc1 & (not_start_mask[x1_trim] | not_end_mask[x2_trim]))
	| (color_mask & (start_mask[x1_trim] & end_mask[x2_trim]));
    }
  else
    {
      if (x1_trim)
	{
	  *loc1 = ((*loc1 & not_start_mask[x1_trim]) | 
		   (color_mask & start_mask[x1_trim]));
	  loc1++;
	}

      switch ((loc2 - loc1) & 15)
	{
	hline_loop:
	case 16:
	  *(loc1++) = color_mask;
	case 15:
	  *(loc1++) = color_mask;
	case 14:
	  *(loc1++) = color_mask;
	case 13:
	  *(loc1++) = color_mask;
	case 12:
	  *(loc1++) = color_mask;
	case 11:
	  *(loc1++) = color_mask;
	case 10:
	  *(loc1++) = color_mask;
	case 9:
	  *(loc1++) = color_mask;
	case 8:
	  *(loc1++) = color_mask;
	case 7:
	  *(loc1++) = color_mask;
	case 6:
	  *(loc1++) = color_mask;
	case 5:
	  *(loc1++) = color_mask;
	case 4:
	  *(loc1++) = color_mask;
	case 3:
	  *(loc1++) = color_mask;
	case 2:
	  *(loc1++) = color_mask;
	case 1:
	  *(loc1++) = color_mask;
	case 0:
	  if (loc1 < loc2)
	    goto hline_loop;
	}

      if (x2_trim == 3)
	*loc2 = color_mask;
      else
	*loc2 = ((*loc2 & not_end_mask[x2_trim]) | 
		 (color_mask & end_mask[x2_trim]));
    }

  check_mouse2 (p, mouse);
}

#ifdef VGL_DITHER
void 
vgl_dumb_hlinelist_noclip (PIXMAP * p, int count, struct span_list *list)
{ /* front end to vgl_dumb_hlinelist_noclip_root() if dithering is enabled */

vgl_dumb_hlinelist_noclip_root(p, count, list, (double)0);

} /* vgl_dumb_hlinelist_noclip() */

/*****************************************************************************/

void 
vgl_dumb_hlinelist_noclip_root (PIXMAP * p, int count, struct span_list *list, double DitherCol)

{
  unsigned char *pix_loc;
  int pix_count, trunc_col;
  double DitherRemain;
#else /* !VGL_DITHER: Same as it ever was... */
void 
vgl_dumb_hlinelist_noclip (PIXMAP * p, int count, struct span_list *list)

{
#endif /* VGL_DITHER */


  int i, x1, x2, x1_trim, x2_trim, y, linewords;
  unsigned long *loc_y, *loc1, *loc2;
  unsigned long color_mask;

  color_mask = p->fore_8;
  linewords = p->pixdata->x_size >> 2;
  y = 0;
  loc_y = (unsigned long *) p->pixdata->data;


  for (i = 0; i < count; i++, list++)
    {
      if (list->x1 < list->x2)
	{
	  x1 = list->x1;
	  x2 = list->x2;
	}
      else
	{
	  x1 = list->x2;
	  x2 = list->x1;
	}

      if (list->y == SPAN_INC)
	{
	  loc_y += linewords;
	  y++;
	}
      else if (list->y != SPAN_SAME)
	{
	  y++;
	  if (y == list->y)
	    loc_y += linewords;
	  else
	    {
	      y = list->y;
	      loc_y = ((unsigned long *) p->pixdata->data) + (y * linewords);
	    }
	}

#ifdef VGL_DITHER
      if(DitherCol != 0)
        {
          pix_loc = (unsigned char *)loc_y;
          for(pix_count = x1; pix_count <= x2; pix_count++)
            {
            trunc_col = (int)DitherCol;
            DitherRemain = (DitherCol - trunc_col) * 255;
            if(DitherRemain > p->DitherTable[(p->DTableIdx++ % p->DTableSize)])
              trunc_col++;
            pix_loc[pix_count] = trunc_col;
            } /* for */
        } /* if */
      else
        { /* must be a corresponding "}" below */
#endif

      loc1 = loc_y + (x1 >> 2);
      loc2 = loc_y + (x2 >> 2);

      x1_trim = x1 & 3;
      x2_trim = x2 & 3;


      if (loc1 == loc2)	  /* If the line falls within a single 32 bit word */
	{
	  *loc1 = (*loc1 & (not_start_mask[x1_trim] | not_end_mask[x2_trim]))
	    | (color_mask & (start_mask[x1_trim] & end_mask[x2_trim]));
	}
      else
	{
	  if (x1_trim)
	    {
	      *loc1 = ((*loc1  & not_start_mask[x1_trim]) | 
		       (color_mask & start_mask[x1_trim]));
	      loc1++;
	    }

	  switch ((loc2 - loc1) & 15)
	    {
	    hlinelist_noclip_loop:
	    case 16:
	      *(loc1++) = color_mask;
	    case 15:
	      *(loc1++) = color_mask;
	    case 14:
	      *(loc1++) = color_mask;
	    case 13:
	      *(loc1++) = color_mask;
	    case 12:
	      *(loc1++) = color_mask;
	    case 11:
	      *(loc1++) = color_mask;
	    case 10:
	      *(loc1++) = color_mask;
	    case 9:
	      *(loc1++) = color_mask;
	    case 8:
	      *(loc1++) = color_mask;
	    case 7:
	      *(loc1++) = color_mask;
	    case 6:
	      *(loc1++) = color_mask;
	    case 5:
	      *(loc1++) = color_mask;
	    case 4:
	      *(loc1++) = color_mask;
	    case 3:
	      *(loc1++) = color_mask;
	    case 2:
	      *(loc1++) = color_mask;
	    case 1:
	      *(loc1++) = color_mask;
	    case 0:
	      if (loc1 < loc2)
		goto hlinelist_noclip_loop;
	    }

	  if (x2_trim == 3)
	    *loc2 = color_mask;
	  else
	    *loc2 = ((*loc2 & not_end_mask[x2_trim]) | 
		     (color_mask & end_mask[x2_trim]));
	}
#ifdef VGL_DITHER
		  } /* here's the corresponding "}" */
#endif
    }
}


/*****************************************************************************/
void 
vgl_dumb_hlinelist (PIXMAP * p, int count, struct span_list *list)
{
  int i, x1, x2, x1_trim, x2_trim, y, linewords, out1, out2;
  unsigned long *loc_y, *loc1, *loc2;
  unsigned long color_mask;

  color_mask = p->fore_8;
  linewords = p->pixdata->x_size >> 2;
  y = INT_MIN;


  for (i = 0; i < count; i++, list++)
    {
      if (list->x1 < list->x2)
	{
	  x1 = list->x1;
	  x2 = list->x2;
	}
      else
	{
	  x1 = list->x2;
	  x2 = list->x1;
	}


#ifdef THIS_IS_BROKEN
      if (list->y == SPAN_INC)
	{
	  loc_y += linewords;
	  y++;
	}
      else if (list->y != SPAN_SAME)
	{
	  y++;
	  if (y == list->y)
	    loc_y += linewords;
	  else
	    {
#endif /* THIS_IS_BROKEN */
	      y = list->y;
	      loc_y = ((unsigned long *) p->pixdata->data) + (y * linewords);
#ifdef THIS_IS_BROKEN
	    }
	}
#endif /* THIS_IS_BROKEN */

      out1 = gen_outcodes (p, x1, y);
      out2 = gen_outcodes (p, x2, y);

      if ((out1 & out2) != 0)
	continue;
      else if ((out1 | out2) != 0)
	{
	  if ((out1 & CLIP_X_MIN) != 0)
	    x1 = p->clip_x_min;
	  else if ((out1 & CLIP_X_MAX) != 0)
	    x1 = p->clip_x_max - 1;

	  if ((out2 & CLIP_X_MIN) != 0)
	    x2 = p->clip_x_min;
	  else if ((out2 & CLIP_X_MAX) != 0)
	    x2 = p->clip_x_max - 1;
	}


      loc1 = loc_y + (x1 >> 2);
      loc2 = loc_y + (x2 >> 2);

      x1_trim = x1 & 3;
      x2_trim = x2 & 3;


      if (loc1 == loc2)	 /* If the line falls within a single 32 bit word */
	{
	  *loc1 = (*loc1 & (not_start_mask[x1_trim] | not_end_mask[x2_trim]))
	    | (color_mask & (start_mask[x1_trim] & end_mask[x2_trim]));
	}
      else
	{
	  if (x1_trim)
	    {
	      *loc1 = ((*loc1 & not_start_mask[x1_trim]) | 
		       (color_mask & start_mask[x1_trim]));
	      loc1++;
	    }

	  switch ((loc2 - loc1) & 15)
	    {
	    hline_list_loop:
	    case 16:
	      *(loc1++) = color_mask;
	    case 15:
	      *(loc1++) = color_mask;
	    case 14:
	      *(loc1++) = color_mask;
	    case 13:
	      *(loc1++) = color_mask;
	    case 12:
	      *(loc1++) = color_mask;
	    case 11:
	      *(loc1++) = color_mask;
	    case 10:
	      *(loc1++) = color_mask;
	    case 9:
	      *(loc1++) = color_mask;
	    case 8:
	      *(loc1++) = color_mask;
	    case 7:
	      *(loc1++) = color_mask;
	    case 6:
	      *(loc1++) = color_mask;
	    case 5:
	      *(loc1++) = color_mask;
	    case 4:
	      *(loc1++) = color_mask;
	    case 3:
	      *(loc1++) = color_mask;
	    case 2:
	      *(loc1++) = color_mask;
	    case 1:
	      *(loc1++) = color_mask;
	    case 0:
	      if (loc1 < loc2)
		goto hline_list_loop;
	    }

	  if (x2_trim == 3)
	    *loc2 = color_mask;
	  else
	    *loc2 = ((*loc2 & not_end_mask[x2_trim]) | 
		     (color_mask & end_mask[x2_trim]));
	}
    }
}



/*****************************************************************************/
void 
vgl_dumb_vline (PIXMAP * p, int x, int ya, int yb)
{
  int out1, out2;

  out1 = gen_outcodes (p, x, ya);
  out2 = gen_outcodes (p, x, yb);

  if ((out1 & out2) != 0)
    return;
  else if ((out1 | out2) != 0)
    {
      if ((out1 & CLIP_Y_MIN) != 0)
	ya = p->clip_y_min;
      else if ((out1 & CLIP_Y_MAX) != 0)
	ya = p->clip_y_max - 1;

      if ((out2 & CLIP_Y_MIN) != 0)
	yb = p->clip_y_min;
      else if ((out2 & CLIP_Y_MAX) != 0)
	yb = p->clip_y_max - 1;


      if ((out1 & CLIP_X_MIN) != 0)
	x = p->clip_x_min;
      else if ((out1 & CLIP_X_MAX) != 0)
	x = p->clip_x_max - 1;
    }

  vgl_dumb_vline_noclip (p, x, ya, yb);
}


void 
vgl_dumb_vline_noclip (PIXMAP * p, int x, int ya, int yb)
{
  unsigned char *l1, *l2;
  unsigned long mouse;

  if (ya > yb)
    swap (ya, yb);

  check_mouse1 (p, x, ya, x, yb, mouse);

  l1 = ((unsigned char *) (p->pixdata->data)) + (x + ya * p->pixdata->x_size);
  l2 = ((unsigned char *) (p->pixdata->data)) + (x + yb * p->pixdata->x_size);


  while (l1 <= l2)
    {
      *l1 = p->foreground;
      l1 += p->pixdata->x_size;
    }

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_rect (PIXMAP * p, int x1, int y1, int x2, int y2)
{
  vgl_dumb_hline (p, x1, x2, y1);
  vgl_dumb_vline (p, x2, y1, y2);
  vgl_dumb_hline (p, x2, x1, y2);
  vgl_dumb_vline (p, x1, y2, y1);
}


/*****************************************************************************/
static void 
vgl_dumb_rect_noclip (PIXMAP * p, int x1, int y1, int x2, int y2) // used locally only -> static, AF 20.7.2021
{
  vgl_dumb_hline_noclip (p, x1, x2, y1);
  vgl_dumb_vline_noclip (p, x2, y1, y2);
  vgl_dumb_hline_noclip (p, x2, x1, y2);
  vgl_dumb_vline_noclip (p, x1, y2, y1);
}


/*****************************************************************************/
static void 
vgl_dumb_fillrect (PIXMAP * p, int x1, int y1, int x2, int y2) // used locally only -> static, AF 19.7.2021
{
  int out1, out2;

  out1 = gen_outcodes (p, x1, y1);
  out2 = gen_outcodes (p, x2, y2);

  if ((out1 & out2) != 0)
    return;

  if ((out1 & CLIP_X_MIN) != 0)
    x1 = p->clip_x_min;
  else if ((out1 & CLIP_X_MAX) != 0)
    x1 = p->clip_x_max - 1;

  if ((out1 & CLIP_Y_MIN) != 0)
    y1 = p->clip_y_min;
  else if ((out1 & CLIP_Y_MAX) != 0)
    y1 = p->clip_y_max - 1;

  if ((out2 & CLIP_X_MIN) != 0)
    x2 = p->clip_x_min;
  else if ((out2 & CLIP_X_MAX) != 0)
    x2 = p->clip_x_max - 1;

  if ((out2 & CLIP_Y_MIN) != 0)
    y2 = p->clip_y_min;
  else if ((out2 & CLIP_Y_MAX) != 0)
    y2 = p->clip_y_max - 1;

  vgl_dumb_fillrect_noclip (p, x1, y1, x2, y2);
}


/*****************************************************************************/
static void 
vgl_dumb_fillrect_noclip (PIXMAP * p, int x1, int y1, int x2, int y2)  // used locally only -> static, AF 19.7.2021
{
  int line_size;
  int x1_trim, x2_trim;
  unsigned long *loc1, *loc2, *loc_end, *o1;
  unsigned long color_mask, mouse;

  if (y1 > y2)
    swap (y1, y2);

  if (x1 > x2)
    swap (x1, x2);

  check_mouse1 (p, x1, y1, x2, y2, mouse);

  o1 = loc1 = (((unsigned long *) (p->pixdata->data)) + 
	       ((x1 + y1 * p->pixdata->x_size) / 4));
  loc2 = (((unsigned long *) (p->pixdata->data)) + 
	  ((x2 + y1 * p->pixdata->x_size) / 4));
  loc_end = (((unsigned long *) (p->pixdata->data)) + 
	     ((x1 + y2 * p->pixdata->x_size) / 4));

  x1_trim = x1 % 4;
  x2_trim = x2 % 4;

  line_size = p->pixdata->x_size / 4;
  color_mask = p->fore_8;


  while (o1 <= loc_end)
    {
      if (loc1 == loc2)	/* If the line falls within a single 32 bit word */
	{
	  *loc1 = (*loc1 & (not_start_mask[x1_trim] | not_end_mask[x2_trim]))
	    | (color_mask & (start_mask[x1_trim] & end_mask[x2_trim]));
	}
      else
	{
	  if (x1_trim)
	    {
	      *loc1 = ((*loc1 & not_start_mask[x1_trim]) | 
		       (color_mask & start_mask[x1_trim]) );
	      loc1++;
	    }
	  /*
	     while( loc1<loc2)  
	     *(loc1++) = color_mask;
	   */
	  switch ((loc2 - loc1) & 15)
	    {
	    fillrect_loop:
	    case 16:
	      *(loc1++) = color_mask;
	    case 15:
	      *(loc1++) = color_mask;
	    case 14:
	      *(loc1++) = color_mask;
	    case 13:
	      *(loc1++) = color_mask;
	    case 12:
	      *(loc1++) = color_mask;
	    case 11:
	      *(loc1++) = color_mask;
	    case 10:
	      *(loc1++) = color_mask;
	    case 9:
	      *(loc1++) = color_mask;
	    case 8:
	      *(loc1++) = color_mask;
	    case 7:
	      *(loc1++) = color_mask;
	    case 6:
	      *(loc1++) = color_mask;
	    case 5:
	      *(loc1++) = color_mask;
	    case 4:
	      *(loc1++) = color_mask;
	    case 3:
	      *(loc1++) = color_mask;
	    case 2:
	      *(loc1++) = color_mask;
	    case 1:
	      *(loc1++) = color_mask;
	    case 0:
	      if (loc1 < loc2)
		goto fillrect_loop;
	    }

	  if (x2_trim == 3)
	    *loc2 = color_mask;
	  else
	    *loc2 = ((*loc2 & not_end_mask[x2_trim]) | 
		     (color_mask & end_mask[x2_trim]));
	}

      loc1 = (o1 += line_size);
      loc2 += line_size;
    }

  check_mouse2 (p, mouse);
}

/*****************************************************************************/
void 
vgl_dumb_circle_noclip (PIXMAP * p, int x, int y, int r)
{
  int d, de, dse, i;
  char *lc, *l1, *l2, *l3, *l4, *l5, *l6, *l7, *l8;
  unsigned long mouse;

  check_mouse1 (p, x - r, y - r, x + r, y + r, mouse);

  d = 1 - r;
  de = 3;
  dse = -2 * r + 5;

  lc = ((char *) p->pixdata->data) + y * p->pixdata->x_size + x;
  i = r * p->pixdata->x_size;
  l1 = lc + i;
  l2 = lc + i;
  l3 = lc - r;
  l4 = lc + r;
  l5 = lc - r;
  l6 = lc + r;
  l7 = lc - i;
  l8 = lc - i;

  i = p->foreground;

  *l1 = i;
  *l2 = i;
  *l3 = i;
  *l4 = i;
  *l5 = i;
  *l6 = i;
  *l7 = i;
  *l8 = i;

  while (l2 > l6)
    {
      if (d < 0)
	{
	  d += de;
	  de += 2;
	  dse += 2;

	  l1--;
	  l2++;
	  l3 -= p->pixdata->x_size;
	  l4 -= p->pixdata->x_size;
	  l5 += p->pixdata->x_size;
	  l6 += p->pixdata->x_size;
	  l7--;
	  l8++;
	}
      else
	{
	  d += dse;
	  de += 2;
	  dse += 4;

	  l1 = l1 - 1 - p->pixdata->x_size;
	  l2 = l2 + 1 - p->pixdata->x_size;
	  l3 = l3 + 1 - p->pixdata->x_size;
	  l4 = l4 - 1 - p->pixdata->x_size;
	  l5 = l5 + 1 + p->pixdata->x_size;
	  l6 = l6 - 1 + p->pixdata->x_size;
	  l7 = l7 - 1 + p->pixdata->x_size;
	  l8 = l8 + 1 + p->pixdata->x_size;
	}

      *l1 = i;
      *l2 = i;
      *l3 = i;
      *l4 = i;
      *l5 = i;
      *l6 = i;
      *l7 = i;
      *l8 = i;
    }

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_circle (PIXMAP * p, int x, int y, int r)
{
  int xo, yo, d, de, dse;
  unsigned long mouse;


  xo = gen_outcodes (p, x - r, y - r);
  yo = gen_outcodes (p, x + r, y + r);

  if ((xo & yo) != 0)	/* Can the entire circle be trivially removed? */
    return;
  else if ((xo | yo) == 0)	/* Is clipping required at all? */
    {
      vgl_dumb_circle_noclip (p, x, y, r);
      return;
    }

  check_mouse1 (p, x - r, y - r, x + r, y + r, mouse);

  xo = 0;
  yo = r;
  d = 1 - r;
  de = 3;
  dse = -2 * r + 5;

  FAST_vgl_dumb_set_pixel (p, x - xo, y + yo);
  FAST_vgl_dumb_set_pixel (p, x + xo, y + yo);
  FAST_vgl_dumb_set_pixel (p, x - yo, y - xo);
  FAST_vgl_dumb_set_pixel (p, x + yo, y - xo);
  FAST_vgl_dumb_set_pixel (p, x - yo, y + xo);
  FAST_vgl_dumb_set_pixel (p, x + yo, y + xo);
  FAST_vgl_dumb_set_pixel (p, x - xo, y - yo);
  FAST_vgl_dumb_set_pixel (p, x + xo, y - yo);

  while (xo < yo)
    {
      if (d < 0)
	{
	  d += de;
	  de += 2;
	  dse += 2;
	  xo++;
	}
      else
	{
	  d += dse;
	  de += 2;
	  dse += 4;
	  xo++;
	  yo--;
	}

      FAST_vgl_dumb_set_pixel (p, x - xo, y + yo);
      FAST_vgl_dumb_set_pixel (p, x + xo, y + yo);
      FAST_vgl_dumb_set_pixel (p, x - yo, y - xo);
      FAST_vgl_dumb_set_pixel (p, x + yo, y - xo);
      FAST_vgl_dumb_set_pixel (p, x - yo, y + xo);
      FAST_vgl_dumb_set_pixel (p, x + yo, y + xo);
      FAST_vgl_dumb_set_pixel (p, x - xo, y - yo);
      FAST_vgl_dumb_set_pixel (p, x + xo, y - yo);
    }

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_fillcircle (PIXMAP * p, int x, int y, int r)
{
  int xo, yo, d, de, dse, n_list;
  struct span_list *list, *l1, *l2, *l3, *l4;
  unsigned long mouse;


  n_list = (r << 1) + 1;
  if ((sizeof (list[0]) * n_list) > p->worksize)
    {
      if (p->workspace != NULL)
	vgl_free (p->workspace);
      p->worksize = sizeof (list[0]) * n_list;
      p->workspace = vgl_malloc (p->worksize);
      if (p->workspace == NULL)
	{
	  p->error = VGL_ERR_MEMORY;
	  p->worksize = 0;
	  return;
	}
    }
  list = p->workspace;


  xo = gen_outcodes (p, x - r, y - r);
  yo = gen_outcodes (p, x + r, y + r);

  if ((xo & yo) != 0)	/* Can the entire circle be trivially removed? */
    return;
  else if ((xo | yo) == 0)    /* Is clipping required at all? */
    {
      vgl_dumb_fillcircle_noclip (p, x, y, r);
      return;
    }

  xo = 0;
  yo = r;
  d = 1 - r;
  de = 3;
  dse = -2 * r + 5;

  l1 = list;
  l2 = list + r;
  l3 = list + r;
  l4 = list + (r << 1);

  l1->x1 = x;
  l1->x2 = x;
  l1->y = y - r;

  l2->x1 = x - r;
  l2->x2 = x + r;
  l2->y = SPAN_INC;

  l3->x1 = x - r;
  l3->x2 = x + r;
  l3->y = SPAN_INC;

  l4->x1 = x;
  l4->x2 = x;
  l4->y = SPAN_INC;

  while (yo > xo)
    {
      if (d < 0)
	{
	  d += de;
	  de += 2;
	  dse += 2;
	  xo++;

	  l2--;
	  l3++;
	}
      else
	{
	  d += dse;
	  de += 2;
	  dse += 4;
	  xo++;
	  yo--;

	  l1++;
	  l2--;
	  l3++;
	  l4--;
	}

      l1->x1 = x - xo;
      l1->x2 = x + xo;
      l1->y = SPAN_INC;

      l2->x1 = x - yo;
      l2->x2 = x + yo;
      l2->y = SPAN_INC;

      l3->x1 = x - yo;
      l3->x2 = x + yo;
      l3->y = SPAN_INC;

      l4->x1 = x - xo;
      l4->x2 = x + xo;
      l4->y = SPAN_INC;
    }

  check_mouse1 (p, x - r, y - r, x + r, y + r, mouse);

  list->y = y - r;
  vgl_dumb_hlinelist (p, n_list, list);

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_fillcircle_noclip (PIXMAP * p, int x, int y, int r)
{
  int xo, yo, d, de, dse, n_list;
  struct span_list *list, *l1, *l2, *l3, *l4;
  unsigned long mouse;


  n_list = (r << 1) + 1;
  if ((sizeof (list[0]) * n_list) > p->worksize)
    {
      if (p->workspace != NULL)
	vgl_free (p->workspace);
      p->worksize = sizeof (list[0]) * n_list;
      p->workspace = vgl_malloc (p->worksize);
      if (p->workspace == NULL)
	{
	  p->error = VGL_ERR_MEMORY;
	  p->worksize = 0;
	  return;
	}
    }
  list = p->workspace;

  xo = 0;
  yo = r;
  d = 1 - r;
  de = 3;
  dse = -2 * r + 5;

  l1 = list;
  l2 = list + r;
  l3 = list + r;
  l4 = list + (r << 1);

  l1->x1 = x;
  l1->x2 = x;
  l1->y = y - r;

  l2->x1 = x - r;
  l2->x2 = x + r;
  l2->y = SPAN_INC;

  l3->x1 = x - r;
  l3->x2 = x + r;
  l3->y = SPAN_INC;

  l4->x1 = x;
  l4->x2 = x;
  l4->y = SPAN_INC;

  while (yo > xo)
    {
      if (d < 0)
	{
	  d += de;
	  de += 2;
	  dse += 2;
	  xo++;

	  l2--;
	  l3++;
	}
      else
	{
	  d += dse;
	  de += 2;
	  dse += 4;
	  xo++;
	  yo--;

	  l1++;
	  l2--;
	  l3++;
	  l4--;
	}

      l1->x1 = x - xo;
      l1->x2 = x + xo;
      l1->y = SPAN_INC;

      l2->x1 = x - yo;
      l2->x2 = x + yo;
      l2->y = SPAN_INC;

      l3->x1 = x - yo;
      l3->x2 = x + yo;
      l3->y = SPAN_INC;

      l4->x1 = x - xo;
      l4->x2 = x + xo;
      l4->y = SPAN_INC;
    }


  check_mouse1 (p, x - r, y - r, x + r, y + r, mouse);

  list->y = y - r;
  vgl_dumb_hlinelist_noclip (p, n_list, list);

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_clear (PIXMAP * p)
{
  int count, size;
  unsigned long *loc, color;
  unsigned long mouse;

  check_mouse1 (p, 0, 0, vgl_x_res (p), vgl_y_res (p), mouse);

  size = (p->pixdata->x_size >> 2) * p->pixdata->y_res;
  loc = ((unsigned long *) (p->pixdata->data));
  color = p->back_8;

  count = 0;
  switch (size & 15)
    {
    clear_simple_loop:
    case 16:
      loc[count++] = color;
    case 15:
      loc[count++] = color;
    case 14:
      loc[count++] = color;
    case 13:
      loc[count++] = color;
    case 12:
      loc[count++] = color;
    case 11:
      loc[count++] = color;
    case 10:
      loc[count++] = color;
    case 9:
      loc[count++] = color;

    case 8:
      loc[count++] = color;
    case 7:
      loc[count++] = color;
    case 6:
      loc[count++] = color;
    case 5:
      loc[count++] = color;
    case 4:
      loc[count++] = color;
    case 3:
      loc[count++] = color;
    case 2:
      loc[count++] = color;
    case 1:
      loc[count++] = color;

    case 0:
      if (count < size)
	goto clear_simple_loop;
    }

  check_mouse2 (p, mouse);
}


/*****************************************************************************/
void 
vgl_dumb_dissolve (PIXMAP * dest, PIXMAP * source, int speed)
{
  unsigned long mask;
  unsigned long size, pos, line, loc;
  int n_bits;
  char *s, *d;
  unsigned long *sl, *dl;
  unsigned long mouse1, mouse2;


  static unsigned long mask_table[] =
    { 
      0,          0,          0x03,       0x06,       0x0c,       0x14, 
      0x30,       0x60,       0xb8,       0x0110,     0x0240,     0x0500,
      0x0ca0,     0x1b00,     0x3500,     0x6000,     0xB400,     0x012000,
      0x020400,   0x072000,   0x090000,   0x0140000,  0x00300000, 0x00400000,
      0x00D80000, 0x01200000, 0x03880000, 0x07200000, 0x09000000, 0x14000000, 
      0x32800000, 0x48000000, 0xA3000000
    };

  if ((dest->dissolve != source->dissolve) &&
      (dest->pixdata->x_size != source->pixdata->x_size) &&
      (dest->pixdata->y_res != source->pixdata->y_res))
    {
      dest->error = VGL_ERR_INVALID_OP;
      return;
    }

  check_mouse1 (source, 0, 0, vgl_x_res (source), vgl_y_res (source), mouse1);
  check_mouse1 (dest, 0, 0, vgl_x_res (dest), vgl_y_res (dest), mouse2);

  if (speed == 0)
    {
      line = dest->pixdata->x_size >> 2;
      size = line * (dest->pixdata->y_res >> 2);
      for (n_bits = 12; size > (1 << n_bits); n_bits++);
      mask = mask_table[n_bits];

      dl = (unsigned long *) dest->pixdata->data;
      sl = (unsigned long *) source->pixdata->data;

      dl[line * 0] = sl[line * 0];
      dl[line * 1] = sl[line * 1];
      dl[line * 2] = sl[line * 2];
      dl[line * 3] = sl[line * 3];

      pos = 34;
      do
	{
	  if (pos < size)
	    {
	      loc = ((pos / line) << 2) * line + (pos % line);

	      dl[loc] = sl[loc];
	      loc += line;
	      dl[loc] = sl[loc];
	      loc += line;
	      dl[loc] = sl[loc];
	      loc += line;
	      dl[loc] = sl[loc];
	    }

	  if (pos & 1)
	    pos = pos >> 1 ^ mask;
	  else
	    pos >>= 1;
	}
      while (pos != 34);
    }
  else
    {
      size = dest->pixdata->x_size * dest->pixdata->y_res;
      for (n_bits = 16; size > (1 << n_bits); n_bits++);
      mask = mask_table[n_bits];

      d = (char *) dest->pixdata->data;
      s = (char *) source->pixdata->data;

      *d = *s;

      pos = 743;
      do
	{
	  if (pos < size)
	    d[pos] = s[pos];

	  if (pos & 1)
	    pos = pos >> 1 ^ mask;
	  else
	    pos >>= 1;
	}
      while (pos != 743);
    }

  check_mouse2 (source, mouse1);
  check_mouse2 (dest, mouse2);
}


/*****************************************************************************/
void 
vgl_dumb_ellipse (PIXMAP * p, int x, int y, int a, int b)
{
}


/*****************************************************************************/
void 
vgl_dumb_ellipse_noclip (PIXMAP * p, int x, int y, int x_axis, int y_axis)
{
}


/*****************************************************************************/

void 
vgl_dumb_fillellipse (PIXMAP * p, int x, int y, int x_axis, int y_axis)
{
}


/*****************************************************************************/
void 
vgl_dumb_fillellipse_noclip (PIXMAP * p, int x, int y, int x_axis, int y_axis)
{
}


/*****************************************************************************/
void 
vgl_dumb_arc (PIXMAP * p, int x, int y, int x_axis, int y_axis, int start, int end)
{
}


/*****************************************************************************/
static void 
vgl_dumb_arc_noclip (PIXMAP * p, int x, int y, int x_axis, int y_axis, int start, int end) // used locally only -> static, AF 20.7.2021
{
}


/*****************************************************************************/
void 
vgl_dumb_fillarc (PIXMAP *p,
		  int x,
		  int y, 
		  int x_axis, 
		  int y_axis, 
		  int start, 
		  int end)
{
}


/*****************************************************************************/
void 
vgl_dumb_fillarc_noclip (PIXMAP *p,
			 int x,
			 int y,
			 int x_axis,
			 int y_axis, 
			 int start, 
			 int end)
{
}


/*****************************************************************************/
void 
vgl_dumb_bitblt (PIXMAP *source, int sx, int sy, int width, int height,
		 PIXMAP *dest, int dx, int dy)
{
  unsigned long temp1, temp2;


  if (width < 0)
    width = source->pixdata->x_res;

  if (height < 0)
    height = source->pixdata->y_res;


  if (sx < 0)
    {
      width += sx;
      dx -= sx;
      sx = 0;
    }

  if (sy < 0)
    {
      height += sy;
      dy -= sy;
      sy = 0;
    }

  if (sx + width > source->pixdata->x_res)
    width -= ((sx + width) - source->pixdata->x_res);

  if (sy + height > source->pixdata->y_res)
    height -= ((sy + height) - source->pixdata->y_res);



  if (dx < dest->clip_x_min)
    {
      sx += dest->clip_x_min - dx;
      width -= dest->clip_x_min - dx;
      dx = dest->clip_x_min;
    }

  if (dy < dest->clip_y_min)
    {
      sy += dest->clip_y_min - dy;
      height -= dest->clip_y_min - dy;
      dy = dest->clip_y_min;
    }

  if (dx + width > dest->clip_x_max)
    {
      width -= (dx + width) - dest->clip_x_max;
    }

  if (dy + height > dest->clip_y_max)
    {
      height -= (dy + height) - dest->clip_y_max;
    }

  if (width < 0 || height < 0)
    return;
  /*
     vgl_dumb_bitblt_noclip( source, sx, sy, width, height, dest, dx, dy);
   */

  check_mouse1 (source, sx, sy, sx + width, sy + height, temp1);
  check_mouse1 (dest, dx, dy, dx + width, dy + height, temp2);

  vgl_dumb_bitblt_core (source, sx, sy, width, height, dest, dx, dy);

  check_mouse2 (source, temp1);
  check_mouse2 (dest, temp2);
}

/*****************************************************************************/
void 
vgl_dumb_bitblt_noclip (PIXMAP *source, int sx, int sy, int width, int height,
			PIXMAP *dest, int dx, int dy)
{
  unsigned long temp1, temp2;

  if (width < 0)
    width = source->pixdata->x_res;

  if (height < 0)
    height = source->pixdata->y_res;

  check_mouse1 (source, sx, sy, sx + width, sy + height, temp1);
  check_mouse1 (dest, dx, dy, dx + width, dy + height, temp2);

  vgl_dumb_bitblt_core (source, sx, sy, width, height, dest, dx, dy);

  check_mouse2 (source, temp1);
  check_mouse2 (dest, temp2);
}



/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 20.July 2021
void 
vgl_dumb_transbitblt (PIXMAP *source, int sx, int sy, int width, int height,
		      PIXMAP * dest, int dx, int dy, int mask_color)
{
  unsigned long temp1, temp2;


  if (width < 0)
    width = source->pixdata->x_res;

  if (height < 0)
    height = source->pixdata->y_res;

  if (sx < 0)
    {
      width += sx;
      dx -= sx;
      sx = 0;
    }

  if (sy < 0)
    {
      height += sy;
      dy -= sy;
      sy = 0;
    }

  if (sx + width > source->pixdata->x_res)
    width -= ((sx + width) - source->pixdata->x_res);

  if (sy + height > source->pixdata->y_res)
    height -= ((sy + height) - source->pixdata->y_res);



  if (dx < dest->clip_x_min)
    {
      sx += dest->clip_x_min - dx;
      width -= dest->clip_x_min - dx;
      dx = dest->clip_x_min;
    }

  if (dy < dest->clip_y_min)
    {
      sy += dest->clip_y_min - dy;
      height -= dest->clip_y_min - dy;
      dy = dest->clip_y_min;
    }

  if (dx + width > dest->clip_x_max)
    {
      width -= (dx + width) - dest->clip_x_max;
    }

  if (dy + height > dest->clip_y_max)
    {
      height -= (dy + height) - dest->clip_y_max;
    }

  if (width <= 0 || height <= 0)
    return;


  check_mouse1 (source, sx, sy, sx + width, sy + height, temp1);
  check_mouse1 (dest, dx, dy, dx + width, dy + height, temp2);

  vgl_dumb_transbitblt_core (source, sx, sy, width, height, 
			     dest, dx, dy, mask_color);

  check_mouse2 (source, temp1);
  check_mouse2 (dest, temp2);
}
#endif

/*****************************************************************************/
void 
vgl_dumb_transbitblt_noclip (PIXMAP *source, int sx, int sy,
			     int width, int height,
			     PIXMAP *dest, int dx, int dy, int mask_color)
{
  unsigned long temp1, temp2;

  if (width < 0)
    width = source->pixdata->x_res;

  if (height < 0)
    height = source->pixdata->y_res;

  check_mouse1 (source, sx, sy, sx + width, sy + height, temp1);
  check_mouse1 (dest, dx, dy, dx + width, dy + height, temp2);

  vgl_dumb_transbitblt_core (source, sx, sy, width, height,
			     dest, dx, dy, mask_color);

  check_mouse2 (source, temp1);
  check_mouse2 (dest, temp2);
}


/*****************************************************************************/
static void 
vgl_dumb_transbitblt_core (PIXMAP *source, int sx, int sy,
			   int width, int height,
			   PIXMAP *dest, int dx, int dy, int mask_color) // used locally only -> static, AF 20.7.2021 
{
  int source_line, dest_line;
  char *sl, *sb, *sr, *s, *dl, *d;

  if (width < 0)
    width = source->pixdata->x_res;

  if (height < 0)
    height = source->pixdata->y_res;


  source_line = source->pixdata->x_size;
  dest_line = dest->pixdata->x_size;

  sl = ((char *) source->pixdata->data) + sy * source_line + sx;
  dl = ((char *) dest->pixdata->data) + dy * dest_line + dx;
  sb = sl + (height * source_line);
  sr = sl + width;

  while (sl < sb)
    {
      s = sl;
      d = dl;
      /*
         while( s<sr)
         {
         if( *s != mask_color)
         *(d++) = *(s++);
         else
         {
         d++;
         s++;
         }
         }
       */

      switch ((sr - s) & 7)
	{
	masked_loop_thing:
	case 8:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 7:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 6:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 5:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 4:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 3:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 2:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 1:
	  if (*s != mask_color)
	    *d = *s;
	  d++;
	  s++;

	case 0:
	  while (s < sr)
	    goto masked_loop_thing;
	}


      sl += source_line;
      sr += source_line;
      dl += dest_line;
    }
}


/****************************************************************************/
void 
vgl_dumb_setfont (PIXMAP * p, struct vgl_ffont *ffont)
{
  p->font = ffont;
}


/****************************************************************************/
int
vgl_dumb_test( PIXMAP *p, char *msg)
{

#ifdef AMIGA
  return (VGL_ERR_NONE);
#else
  int i, err;

  for( i=0; i<1; i++)
    {
      err = vgl_mem_test (p->pixdata->data,
			  p->pixdata->x_size * p->pixdata->y_res,
			  msg);

      if (err!=VGL_ERR_NONE)
	break;
    }
  return (err);
#endif
}

/****************************************************************************/
void vgl_dumb_quadlist (PIXMAP *p, int n_quads, struct vgl_quad_def *quad,
			int xoffset, int yoffset)
{
  int i, cur_color;
  int prev_f, prev_b;
  struct vgl_coord coords[4];

  prev_f = p->foreground;
  prev_b = p->background;
 
  cur_color = quad->color;
  vgl_setcur (p, cur_color, prev_b);

  for (i=0; i<n_quads; i++, quad++)
    {
      if (cur_color!=quad->color)
	{
	  cur_color = quad->color;
	  vgl_setcur (p, cur_color, prev_b);
	}

      coords[0].x = xoffset + ((quad->p1>>16) & 0xFFFF);
      coords[0].y = yoffset + ( quad->p1      & 0xFFFF);
      coords[1].x = xoffset + ((quad->p2>>16) & 0xFFFF);
      coords[1].y = yoffset + ( quad->p2      & 0xFFFF);
      coords[2].x = xoffset + ((quad->p3>>16) & 0xFFFF);
      coords[2].y = yoffset + ( quad->p3      & 0xFFFF);
      coords[3].x = xoffset + ((quad->p4>>16) & 0xFFFF);
      coords[3].y = yoffset + ( quad->p4      & 0xFFFF);

      vgl_fillpoly (p, 4, coords);
    }
      
  /* Set the color back to what it "should" be */
  vgl_setcur (p, prev_f, prev_b);
}


/****************************************************************************/
void vgl_dumb_linelist (PIXMAP *p, int n_lines, struct vgl_line_def *line,
			int xoffset, int yoffset)
{
  int i, cur_color;
  int prev_f, prev_b;
  int x1, y1, x2, y2;

  prev_f = p->foreground;
  prev_b = p->background;
 
  cur_color = line->color;
  vgl_setcur (p, cur_color, prev_b);

  for (i=0; i<n_lines; i++, line++)
    {
      if (cur_color!=line->color)
	{
	  cur_color = line->color;
	  vgl_setcur (p, cur_color, prev_b);
	}

      x1 = xoffset + ((line->p1>>16) & 0xFFFF);
      y1 = yoffset + ( line->p1      & 0xFFFF);
      x2 = xoffset + ((line->p2>>16) & 0xFFFF);
      y2 = yoffset + ( line->p2      & 0xFFFF);

      vgl_line (p, x1, y1, x2, y2);
    }
      
  /* Set the color back to what it "should" be */
  vgl_setcur (p, prev_f, prev_b);
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
