
#include "vgl.h"
#include "vgl_internals.h"

#if defined(SUPPORT_GDM_9000)

#include "gdm_9000.h"


#define fast_req_drawbuffer(pixmap,fff)  gdm_9000_req_drawbuffer(pixmap,fff)
#define fast_free_drawbuffer(pixmap)     gdm_9000_free_drawbuffer(pixmap)



#define fast_point(pixmap,xxx,yyy)   \
	{*(((unsigned char *)(pixmap->pixdata->data))+ ((xxx) + (yyy)*pixmap->pixdata->x_size)) = \
	pixmap->foreground;}

#define fast_line(b,x1,y1,x2,y2) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_x = x1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_y = y1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_x = x2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_y = y2; \
    do_quad(b); \
  }

#define fast_line_packed(b,p1,p2) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_xy = p1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.line_win_xy = p2; \
    do_quad(b); \
  }

#define fast_tri(b,x1,y1,x2,y2,x3,y3) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_x = x1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_y = y1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_x = x2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_y = y2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_x = x3; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.tri_win_y = y3; \
    do_quad(b); \
  }

#define fast_rect(b,x1,y1,x2,y2) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.rect_win_x = x1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.rect_win_y = y1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.rect_win_x = x2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.rect_win_y = y2; \
    do_quad(b); \
  }

#define fast_quad(b,x1,y1,x2,y2,x3,y3,x4,y4) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_x = x1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_y = y1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_x = x2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_y = y2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_x = x3; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_y = y3; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_x = x4; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_y = y4; \
    do_quad(b); \
  }

#define fast_quad_packed(b,p1,p2,p3,p4) \
  { \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_xy = p1; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_xy = p2; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_xy = p3; \
    ((board_gdm_9000 *)((b)->addr))->p9000_bt445.quad_win_xy = p4; \
    do_quad(b); \
  }

#define do_quad(b) \
  {while (((board_gdm_9000*)((b)->addr))->p9000_bt445.quad_command&P9000_ISSUE);}

#define do_blit(b) \
  {while (((board_gdm_9000*)((b)->addr))->p9000_bt445.blit_command&P9000_BUSY);}



/*****************************************************************************/
/******* Routines for dumb frame buffers and pixmaps in host RAM *************/
/*****************************************************************************/
void 
gdm_9000_set_clip (PIXMAP *p, int x1, int y1, int x2, int y2)
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

  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (p->clip_x_min, p->clip_y_min);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (p->clip_x_max-1, p->clip_y_max-1);
}


/****************************************************************************/
void 
gdm_9000_clip_off (PIXMAP *p)
{
  p->clip_setting = CLIP_OFF;

  p->bitblt = db_180_bitblt_noclip;

  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (0,0);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (vgl_x_res(p)-1, vgl_y_res(p)-1);
}

/****************************************************************************/
void 
gdm_9000_clip_on (PIXMAP *p)
{
  p->clip_setting = CLIP_ON;

  p->bitblt = db_180_bitblt;

  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (p->clip_x_min, p->clip_y_min);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (p->clip_x_max-1, p->clip_y_max-1);
}


/*****************************************************************************/
void 
gdm_9000_setcur (PIXMAP *p, int fg, int bg)
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
  p->fore_8 = (p->foreground | (p->foreground << 8) | (p->foreground << 16) | (p->foreground << 24));
  p->back_8 = (p->background | (p->background << 8) | (p->background << 16) | (p->background << 24));

  for (i = 0; i < 16; i++)
    p->expand_8[i] = ((expand_src[i] & p->fore_8) | (~expand_src[i] & p->back_8));

  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.bground = p->background;
}


/*****************************************************************************/
int 
gdm_9000_pixerror (PIXMAP *p)
{
  int err;

  err = p->error;
  p->error = VGL_ERR_NONE;
  return (err);
}


/*****************************************************************************/
void 
gdm_9000_set_pixel (PIXMAP *p, int x, int y)
{
  fast_req_drawbuffer( p, 0);
  fast_point (p, x, y);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
int 
gdm_9000_get_pixel (PIXMAP *p, int x, int y)
{
  int	i;

  fast_req_drawbuffer( p, 0);
  i = vgl_dumb_get_pixel (p, x, y);
  fast_free_drawbuffer (p);

  return (i);
}


/*****************************************************************************/
void 
gdm_9000_line (PIXMAP *p, int xa, int ya, int xb, int yb)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);

  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, xa, ya, xb, yb);

  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_hline (PIXMAP *p, int x1, int x2, int y)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y, x2, y);

  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_hlinelist (PIXMAP *p, int count, struct span_list *list)
{
  int i, x1, x2, y;
  VGBOARD *b;

  b = p->board;
  fast_req_drawbuffer( p, 0);

  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  y = 0;

  for (i = 0; i < count; i++, list++)
    {
      if (list->y == SPAN_INC)
	y++;
      else if (list->y != SPAN_SAME)
	y = list->y;

      fast_line (b, list->x1, y, list->x2, y);
    }

  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_vline (PIXMAP *p, int x, int ya, int yb)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x, ya, x, yb);

  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_rect (PIXMAP *p, int x1, int y1, int x2, int y2)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y1, x2, y1);
  fast_line (b, x2, y1, x2, y2);
  fast_line (b, x2, y2, x1, y2);
  fast_line (b, x1, y2, x1, y1);

  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_fillrect (PIXMAP *p, int x1, int y1, int x2, int y2)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_rect (b, x1, y1, x2, y2);

  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_circle (PIXMAP *p, int x, int y, int r)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_circle (p, x, y, r);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_fillcircle (PIXMAP *p, int x, int y, int r)
{
  int xo, yo, d, de, dse;
  VGBOARD *b;

  b = p->board;

  xo = 0;
  yo = r;
  d = 1 - r;
  de = 3;
  dse = -2 * r + 5;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x-xo, y+yo, x+xo, y+yo);
  fast_line (b, x-yo, y-xo, x+yo, y-xo);
  fast_line (b, x-yo, y+xo, x+yo, y+xo);
  fast_line (b, x-xo, y-yo, x+xo, y-yo);

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

      fast_line (b, x-xo, y+yo, x+xo, y+yo);
      fast_line (b, x-yo, y-xo, x+yo, y-xo);
      fast_line (b, x-yo, y+xo, x+yo, y+xo);
      fast_line (b, x-xo, y-yo, x+xo, y-yo);
    }

  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_clear (PIXMAP *p)
{
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_B_MASK);
  fast_rect (b, 0, 0, vgl_x_res(p)-1, vgl_y_res(p)-1);

  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_dissolve (PIXMAP *dest, PIXMAP *source, int speed)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_dissolve (dest, source, speed);

  fast_free_drawbuffer (source);
  fast_free_drawbuffer (dest);
}


/*****************************************************************************/
void 
gdm_9000_ellipse (PIXMAP *p, int x, int y, int a, int b)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_ellipse (p, x, y, a, b);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_fillellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillellipse (p, x, y, x_axis, y_axis);
  fast_free_drawbuffer (p);
}



/*****************************************************************************/
void 
gdm_9000_arc (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_arc (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_fillarc (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillarc (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
gdm_9000_bitblt (PIXMAP *source, int sx, int sy, int width, int height, 
	       PIXMAP *dest, int dx, int dy)
{
  if (width < 0)
    width = vgl_x_res(source);

  if (height < 0)
    height = vgl_y_res(source);

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

  if (sx + width > vgl_x_res(source))
    width -= ((sx + width) - vgl_x_res(source));

  if (sy + height > vgl_y_res(source))
    height -= ((sy + height) - vgl_y_res(source));


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

  gdm_9000_bitblt_noclip (source, sx, sy, width, height, dest, dx, dy);
}


/*****************************************************************************/
void 
gdm_9000_bitblt_noclip (PIXMAP *source, int sx, int sy, int width, int height,
		      PIXMAP *dest, int dx, int dy)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_bitblt_noclip (source, sx, sy, width, height, 
			  dest, dx, dy);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);

#else

  unsigned long    *loc, *x1, *x2, *x3, i, *pixel8;
  unsigned int     x_word_size;
  VGBOARD *b;


  if (width < 0)
    width = vgl_x_res(source);

  if (height < 0)
    height = vgl_y_res(source);

  b = dest->board;
/*
  fast_req_drawbuffer( source, 0);
*/
  fast_req_drawbuffer( dest, 0);

  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
							 IGM_S_MASK);

  if (source->board == dest->board)
    {
      /* Source and destination are on the same board */
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.x0_abs = sx;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.y0_abs = sy;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.x1_abs = sx + width  - 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.y1_abs = sy + height - 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.x2_abs = dx;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.y2_abs = dy;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.x3_abs = dx + width  - 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.y3_abs = dy + height - 1;

      do_blit(b);
    }
  else
    {
      /* The source is not on the board */

      if (!(sx&3))  /* If the source is word aligned */
	{
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x0_abs = dx;
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x1_abs = dx;
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y1_abs = dy;
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x2_abs = dx + width;
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y3_abs = 1;
	  
	  if (sx==0                    && sy==0                      &&
	      width==vgl_x_res(source) && height==vgl_y_res(source))
	    {
	      /* The entire source pixmap is being moved */
	      loc = source->pixdata->data;
	      i = ((vgl_x_res(source)*vgl_y_res(source))>>2);
	      x1 = loc + i;
	      pixel8 = &(((board_gdm_9000 *)(b->addr))->p9000_bt445.pixel8_command);
	      P9000_issue_wait(b);

	      switch (i&31)
		{
		bitblt_loop1:
		case 32:  *pixel8 = *(loc++);
		case 31:  *pixel8 = *(loc++);
		case 30:  *pixel8 = *(loc++);
		case 29:  *pixel8 = *(loc++);
		case 28:  *pixel8 = *(loc++);
		case 27:  *pixel8 = *(loc++);
		case 26:  *pixel8 = *(loc++);
		case 25:  *pixel8 = *(loc++);
		case 24:  *pixel8 = *(loc++);
		case 23:  *pixel8 = *(loc++);
		case 22:  *pixel8 = *(loc++);
		case 21:  *pixel8 = *(loc++);
		case 20:  *pixel8 = *(loc++);
		case 19:  *pixel8 = *(loc++);
		case 18:  *pixel8 = *(loc++);
		case 17:  *pixel8 = *(loc++);
		case 16:  *pixel8 = *(loc++);
		case 15:  *pixel8 = *(loc++);
		case 14:  *pixel8 = *(loc++);
		case 13:  *pixel8 = *(loc++);
		case 12:  *pixel8 = *(loc++);
		case 11:  *pixel8 = *(loc++);
		case 10:  *pixel8 = *(loc++);
		case  9:  *pixel8 = *(loc++);
		case  8:  *pixel8 = *(loc++);
		case  7:  *pixel8 = *(loc++);
		case  6:  *pixel8 = *(loc++);
		case  5:  *pixel8 = *(loc++);
		case  4:  *pixel8 = *(loc++);
		case  3:  *pixel8 = *(loc++);
		case  2:  *pixel8 = *(loc++);
		case  1:  *pixel8 = *(loc++);
		case  0:
		  if (loc<x1)
		    goto bitblt_loop1;
		}
	    }
	  else
	    {
	      /* only part of the source is being moved */
	      x_word_size = source->pixdata->x_size>>2;

	      x1 = (((unsigned long *)source->pixdata->data) +
		    sy*x_word_size + (sx>>2));
	      x2 = x1 + ((width+3)>>2);
	      x3 = x1 + height*x_word_size;;
	      pixel8 = &(((board_gdm_9000 *)(b->addr))->p9000_bt445.pixel8_command);	      
	      P9000_issue_wait(b);

	      while (x1<x3)
		{
		  /* Do a line */
		  loc = x1;
/*  This is the original version 
		  while (loc<x2)
		    {
		      ((board_gdm_9000 *)(b->addr))->p9000_bt445.pixel8_command =
			*(loc++);
		    }
*/
		  switch ((x2-loc)&31)
		    {
		    bitblt_loop2:
		    case 32:  *pixel8 = *(loc++);
		    case 31:  *pixel8 = *(loc++);
		    case 30:  *pixel8 = *(loc++);
		    case 29:  *pixel8 = *(loc++);
		    case 28:  *pixel8 = *(loc++);
		    case 27:  *pixel8 = *(loc++);
		    case 26:  *pixel8 = *(loc++);
		    case 25:  *pixel8 = *(loc++);
		    case 24:  *pixel8 = *(loc++);
		    case 23:  *pixel8 = *(loc++);
		    case 22:  *pixel8 = *(loc++);
		    case 21:  *pixel8 = *(loc++);
		    case 20:  *pixel8 = *(loc++);
		    case 19:  *pixel8 = *(loc++);
		    case 18:  *pixel8 = *(loc++);
		    case 17:  *pixel8 = *(loc++);
		    case 16:  *pixel8 = *(loc++);
		    case 15:  *pixel8 = *(loc++);
		    case 14:  *pixel8 = *(loc++);
		    case 13:  *pixel8 = *(loc++);
		    case 12:  *pixel8 = *(loc++);
		    case 11:  *pixel8 = *(loc++);
		    case 10:  *pixel8 = *(loc++);
		    case  9:  *pixel8 = *(loc++);
		    case  8:  *pixel8 = *(loc++);
		    case  7:  *pixel8 = *(loc++);
		    case  6:  *pixel8 = *(loc++);
		    case  5:  *pixel8 = *(loc++);
		    case  4:  *pixel8 = *(loc++);
		    case  3:  *pixel8 = *(loc++);
		    case  2:  *pixel8 = *(loc++);
		    case  1:  *pixel8 = *(loc++);
		    case  0:
		      if (loc<x2)
			goto bitblt_loop2;
		    }

		  x1 += x_word_size;
		  x2 += x_word_size;
		}
	    }
	}
      else  /* If the source is not word aligned */
	{
	  vgl_dumb_bitblt_noclip (source, sx, sy, width, height, 
			   dest, dx, dy);
	}
    }

  fast_free_drawbuffer (dest);
/*
  fast_free_drawbuffer (source);
*/

#endif
}


/*****************************************************************************/
void 
gdm_9000_transbitblt (PIXMAP *source, int sx, int sy, int width, int height, PIXMAP *dest, int dx, int dy, int mask_color)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_transbitblt (source, sx, sy, width, height,
			dest, dx, dy, 
			mask_color);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);
}


/****************************************************************************/
void 
gdm_9000_setfont (PIXMAP *p, struct vgl_ffont *ffont)
{
  p->font = ffont;
}


/****************************************************************************/
int
gdm_9000_pixmap_test( PIXMAP *p, char *msg)
{
  int err;

  fast_req_drawbuffer( p, 0);

  err = vgl_mem_test (p->pixdata->data,
		      p->pixdata->x_size * p->pixdata->y_res,
		      msg);

  fast_free_drawbuffer (p);
  return (err);
}


/****************************************************************************/
void gdm_9000_text (PIXMAP *p, int x, int y, char *s)
{
  VGBOARD *b;
  unsigned long *command, *bitmap;

  b = p->board;
  command = (((board_gdm_9000 *)(b->addr))->p9000_bt445.pixel1_base_command +
	     (p->font->width-1));

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED |
     ((IGM_S_MASK & IGM_F_MASK) | (~IGM_S_MASK & IGM_B_MASK)));

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x0_abs = x;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x1_abs = x;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y1_abs = y;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x2_abs = x + p->font->width;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y2_abs = y;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y3_abs = 1;

  P9000_issue_wait(b);

  while (*s != '\0')
    {
      bitmap = p->font->bitmaps + (p->font->offsets[*s-p->font->min_char]);

      switch (p->font->height)
	{
	default:
	case 32:  *command = *(bitmap++);
	case 31:  *command = *(bitmap++);
	case 30:  *command = *(bitmap++);
	case 29:  *command = *(bitmap++);
	case 28:  *command = *(bitmap++);
	case 27:  *command = *(bitmap++);
	case 26:  *command = *(bitmap++);
	case 25:  *command = *(bitmap++);
	case 24:  *command = *(bitmap++);
	case 23:  *command = *(bitmap++);
	case 22:  *command = *(bitmap++);
	case 21:  *command = *(bitmap++);
	case 20:  *command = *(bitmap++);
	case 19:  *command = *(bitmap++);
	case 18:  *command = *(bitmap++);
	case 17:  *command = *(bitmap++);
	case 16:  *command = *(bitmap++);
	case 15:  *command = *(bitmap++);
	case 14:  *command = *(bitmap++);
	case 13:  *command = *(bitmap++);
	case 12:  *command = *(bitmap++);
	case 11:  *command = *(bitmap++);
	case 10:  *command = *(bitmap++);
	case  9:  *command = *(bitmap++);
	case  8:  *command = *(bitmap++);
	case  7:  *command = *(bitmap++);
	case  6:  *command = *(bitmap++);
	case  5:  *command = *(bitmap++);
	case  4:  *command = *(bitmap++);
	case  3:  *command = *(bitmap++);
	case  2:  *command = *(bitmap++);
	case  1:  *command = *(bitmap++);
	case  0:
	  break;
	}

      ((board_gdm_9000 *)(b->addr))->p9000_bt445.next_pixels_command =
	p->font->width;
      s++;
    }

  fast_free_drawbuffer (p);
}


/****************************************************************************/
void gdm_9000_text2 (PIXMAP *p, int x, int y, char *s, int len)
{
  VGBOARD *b;
  unsigned long *command, *bitmap;
  char *end;

  b = p->board;
  command = (((board_gdm_9000 *)(b->addr))->p9000_bt445.pixel1_base_command +
	     (p->font->width-1));
  end = s+len;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED |
     ((IGM_S_MASK & IGM_F_MASK) | (~IGM_S_MASK & IGM_B_MASK)));

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x0_abs = x;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x1_abs = x;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y1_abs = y;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.x2_abs = x + p->font->width;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y2_abs = y;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.y3_abs = 1;

  P9000_issue_wait(b);

  while (s!=end)
    {
      bitmap = p->font->bitmaps + (p->font->offsets[*s-p->font->min_char]);

      switch (p->font->height)
	{
	default:
	case 32:  *command = *(bitmap++);
	case 31:  *command = *(bitmap++);
	case 30:  *command = *(bitmap++);
	case 29:  *command = *(bitmap++);
	case 28:  *command = *(bitmap++);
	case 27:  *command = *(bitmap++);
	case 26:  *command = *(bitmap++);
	case 25:  *command = *(bitmap++);
	case 24:  *command = *(bitmap++);
	case 23:  *command = *(bitmap++);
	case 22:  *command = *(bitmap++);
	case 21:  *command = *(bitmap++);
	case 20:  *command = *(bitmap++);
	case 19:  *command = *(bitmap++);
	case 18:  *command = *(bitmap++);
	case 17:  *command = *(bitmap++);
	case 16:  *command = *(bitmap++);
	case 15:  *command = *(bitmap++);
	case 14:  *command = *(bitmap++);
	case 13:  *command = *(bitmap++);
	case 12:  *command = *(bitmap++);
	case 11:  *command = *(bitmap++);
	case 10:  *command = *(bitmap++);
	case  9:  *command = *(bitmap++);
	case  8:  *command = *(bitmap++);
	case  7:  *command = *(bitmap++);
	case  6:  *command = *(bitmap++);
	case  5:  *command = *(bitmap++);
	case  4:  *command = *(bitmap++);
	case  3:  *command = *(bitmap++);
	case  2:  *command = *(bitmap++);
	case  1:  *command = *(bitmap++);
	case  0:
	  break;
	}

      ((board_gdm_9000 *)(b->addr))->p9000_bt445.next_pixels_command =
	p->font->width;
      s++;
    }

  fast_free_drawbuffer (p);
}


/****************************************************************************/
void gdm_9000_poly (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_poly (p, n_vert, vert);
  fast_free_drawbuffer (p);
}


/****************************************************************************/
void gdm_9000_fillpoly (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
  int     i;
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  switch (n_vert)
    {
    case 1:
      fast_point (p, vert[0].x, vert[0].y);
      break;

    case 2:
      fast_line (b, vert[0].x, vert[0].y, vert[1].x, vert[1].y);
      break;

    case 3:
      fast_tri (b, 
		vert[0].x, vert[0].y, 
		vert[1].x, vert[1].y,
		vert[2].x, vert[2].y);
      break;

    case 4:
      fast_quad (b,
		 vert[0].x, vert[0].y, 
		 vert[1].x, vert[1].y,
		 vert[2].x, vert[2].y,
		 vert[3].x, vert[3].y);
      

      if ( ((board_gdm_9000 *)(b->addr))->p9000_bt445.status & P9000_QUAD_SW)
	vgl_dumb_fillpoly_noclip (p, n_vert, vert);

      break;

    default:
      vgl_dumb_fillpoly (p, n_vert, vert);
      return;
    }

  fast_free_drawbuffer (p);
}


/****************************************************************************/
/****************************************************************************/
void gdm_9000_quadlist (PIXMAP *p, int n_quads, struct vgl_quad_def *quad, int xoffset, int yoffset)
{
  VGBOARD *b;
  int i, cur_color;

  b = p->board;

  cur_color = quad->color;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED | IGM_F_MASK);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (xoffset, yoffset);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = 
    cur_color;

  i = 0;
  while (i<n_quads)
    {
      if (cur_color!=quad->color)
	{
	  cur_color = quad->color;
	  P9000_busy_wait(b);
	  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = cur_color;
	}

      fast_quad_packed (b, quad->p1, quad->p2, quad->p3, quad->p4);
      quad++;
      i++;
    }

  /* Set the color back to what it "should" be */
  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (0,0);

  fast_free_drawbuffer (p);
}

/****************************************************************************/
/****************************************************************************/
void gdm_9000_linelist (PIXMAP *p, int n_lines, struct vgl_line_def *line, int xoffset, int yoffset)
{
  VGBOARD *b;
  int i, cur_color;

  b = p->board;

  cur_color = line->color;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED | IGM_F_MASK);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (xoffset, yoffset);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = 
    cur_color;

  i = n_lines;
  while (i>0)
    {
      if (cur_color!=line->color)
	{
	  cur_color = line->color;
	  P9000_busy_wait(b);
	  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = cur_color;
	}

      fast_line_packed (b, line->p1, line->p2); 
      line++;
      i--;
    }

  /* Set the color back to what it "should" be */
  P9000_busy_wait(p->board);
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (0,0);

  fast_free_drawbuffer (p);
}


/****************************************************************************/
/****************************************************************************/

#endif
