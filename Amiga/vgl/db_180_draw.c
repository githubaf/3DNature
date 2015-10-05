
#include "vgl.h"
#include "vgl_internals.h"

#if defined(SUPPORT_DB_180)

#include "db_180.h"

#define USE_ACCEL


/* This is a macro version of db_180_req_drawbuffer() */
#define fast_req_drawbuffer(pixmap,fff) \
  {  \
     if ((pixmap)->board->type == DB_180) \
       { \
	 if(fff) \
	   (pixmap)->board->draw_irq_flag = 1; \
	 (pixmap)->board->n_users++; \
	 if ((pixmap)->data1!=(pixmap)->board->cur_draw_buffer) \
	   { \
	     P9000_busy_wait((pixmap)->board); \
	     if ((pixmap)->data1) \
	       { \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.sysconfig =  \
		   (pixmap)->board->mode->db_180.sysconfig | (1<<9) | (1<<10); \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.draw_mode = 0x03; \
	       } \
	     else \
	       { \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.sysconfig =  \
		   (pixmap)->board->mode->db_180.sysconfig & ~((1<<9) | (1<<10)); \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.draw_mode = 0x02; \
	       } \
	     (pixmap)->board->cur_draw_buffer = (pixmap)->data1; \
	     ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.fground = \
	       (pixmap)->foreground; \
	     ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.bground = \
	       (pixmap)->background; \
	     if ((pixmap)->clip_setting==CLIP_ON) \
	       { \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.w_min = \
		   pack_xy ((pixmap)->clip_x_min, (pixmap)->clip_y_min); \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.w_max = \
		   pack_xy ((pixmap)->clip_x_max-1, (pixmap)->clip_y_max-1); \
	       } \
	     else \
	       { \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.w_min = \
		   pack_xy (0,0); \
		 ((board_db_180 *)((pixmap)->board->addr))->p9000_bt445.w_max = \
		   pack_xy (vgl_x_res((pixmap))-1, vgl_y_res((pixmap))-1); \
	       } \
	   } \
       } \
   } \


/* This is a macro version of db_180_free_drawbuffer() */
#define fast_free_drawbuffer(pixmap) \
  {  \
    if ((pixmap)->board->type == DB_180) \
      { \
	if( (pixmap)->board->n_users>0) \
	  { \
	    if( (pixmap)->board->draw_irq_flag) \
	      (pixmap)->board->draw_irq_flag = 0; \
	    (pixmap)->board->n_users--; \
	  } \
      } \
  }




#define fast_point(pixmap,xxx,yyy)   \
	{*(((unsigned char *)(pixmap->pixdata->data))+ ((xxx) + (yyy)*pixmap->pixdata->x_size)) = \
	pixmap->foreground;}

#define fast_line(b,x1,y1,x2,y2) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_x = x1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_y = y1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_x = x2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_y = y2; \
    do_quad(b); \
  }

#define fast_line_packed(b,p1,p2) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_xy = p1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.line_win_xy = p2; \
    do_quad(b); \
  }

#define fast_tri(b,x1,y1,x2,y2,x3,y3) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_x = x1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_y = y1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_x = x2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_y = y2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_x = x3; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.tri_win_y = y3; \
    do_quad(b); \
  }

#define fast_rect(b,x1,y1,x2,y2) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.rect_win_x = x1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.rect_win_y = y1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.rect_win_x = x2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.rect_win_y = y2; \
    do_quad(b); \
  }

#define fast_quad(b,x1,y1,x2,y2,x3,y3,x4,y4) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_x = x1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_y = y1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_x = x2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_y = y2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_x = x3; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_y = y3; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_x = x4; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_y = y4; \
    do_quad(b); \
  }

#define fast_quad_packed(b,p1,p2,p3,p4) \
  { \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_xy = p1; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_xy = p2; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_xy = p3; \
    ((board_db_180 *)((b)->addr))->p9000_bt445.quad_win_xy = p4; \
    do_quad(b); \
  }

#define do_quad(b) \
  {while (((board_db_180*)((b)->addr))->p9000_bt445.quad_command&P9000_ISSUE);}

#define do_blit(b) \
  {while (((board_db_180*)((b)->addr))->p9000_bt445.blit_command&P9000_BUSY);}



/*****************************************************************************/
/******* Routines for dumb frame buffers and pixmaps in host RAM *************/
/*****************************************************************************/
void 
db_180_set_clip (PIXMAP *p, int x1, int y1, int x2, int y2)
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

#ifdef USE_ACCEL
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (p->clip_x_min, p->clip_y_min);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (p->clip_x_max-1, p->clip_y_max-1);
#endif
}


/****************************************************************************/
void 
db_180_clip_off (PIXMAP *p)
{
  p->clip_setting = CLIP_OFF;

  p->set_pixel = db_180_set_pixel_noclip;
  p->get_pixel = db_180_get_pixel_noclip;
  p->line = db_180_line_noclip;
  p->hline = db_180_hline_noclip;
  p->vline = db_180_vline_noclip;
  p->rect = db_180_rect_noclip;
  p->fillrect = db_180_fillrect_noclip;
  p->circle = db_180_circle_noclip;
  p->fillcircle = db_180_fillcircle_noclip;
  p->ellipse = db_180_ellipse_noclip;
  p->fillellipse = db_180_fillellipse_noclip;
  p->arc = db_180_arc_noclip;
  p->fillarc = db_180_fillarc_noclip;
  p->poly = db_180_poly_noclip;
  p->fillpoly = db_180_fillpoly_noclip;
  p->bitblt = db_180_bitblt_noclip;
  p->transbitblt = db_180_transbitblt_noclip;
  p->setfont = db_180_setfont;
  p->text = db_180_text;
  p->text2 = db_180_text2;

#ifdef USE_ACCEL
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (0,0);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (vgl_x_res(p)-1, vgl_y_res(p)-1);
#endif
}

/****************************************************************************/
void 
db_180_clip_on (PIXMAP *p)
{
  p->clip_setting = CLIP_ON;

  p->set_pixel = db_180_set_pixel;
  p->get_pixel = db_180_get_pixel;
  p->line = db_180_line;
  p->hline = db_180_hline;
  p->vline = db_180_vline;
  p->rect = db_180_rect;
  p->fillrect = db_180_fillrect;
  p->circle = db_180_circle;
  p->fillcircle = db_180_fillcircle;
  p->ellipse = db_180_ellipse;
  p->fillellipse = db_180_fillellipse;
  p->arc = db_180_arc;
  p->fillarc = db_180_fillarc;
  p->poly = db_180_poly;
  p->fillpoly = db_180_fillpoly;
  p->bitblt = db_180_bitblt;
  p->transbitblt = db_180_transbitblt;
  p->setfont = db_180_setfont;
  p->text = db_180_text;
  p->text2 = db_180_text2;

#ifdef USE_ACCEL
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_min =
    pack_xy (p->clip_x_min, p->clip_y_min);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_max =
    pack_xy (p->clip_x_max-1, p->clip_y_max-1);
#endif
}


/*****************************************************************************/
void 
db_180_setcur (PIXMAP *p, int fg, int bg)
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

#ifdef USE_ACCEL
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_db_180 *)(p->board->addr))->p9000_bt445.bground = p->background;
#endif
}


/*****************************************************************************/
int 
db_180_pixerror (PIXMAP *p)
{
  int err;

  err = p->error;
  p->error = VGL_ERR_NONE;
  return (err);
}


/*****************************************************************************/
void 
db_180_set_pixel (PIXMAP *p, int x, int y)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_set_pixel (p, x, y);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);

  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  ((board_db_180 *)(b->addr))->p9000_bt445.point_win_x = x;
  ((board_db_180 *)(b->addr))->p9000_bt445.point_win_y = y;

  while ( ((board_db_180 *)(b->addr))->p9000_bt445.quad_command & P9000_BUSY)
    /* Do Nothing */;

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
int 
db_180_get_pixel (PIXMAP *p, int x, int y)
{
  int	i;

  fast_req_drawbuffer( p, 0);
  i = vgl_dumb_get_pixel (p, x, y);
  fast_free_drawbuffer (p);

  return (i);
}


/*****************************************************************************/
void 
db_180_set_pixel_noclip (PIXMAP *p, int x, int y)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_set_pixel_noclip (p, x, y);
  fast_free_drawbuffer (p);

#else

  fast_req_drawbuffer( p, 0);
  fast_point (p, x, y);
  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
int 
db_180_get_pixel_noclip (PIXMAP *p, int x, int y)
{
  int	i;

  fast_req_drawbuffer( p, 0);
  i = vgl_dumb_get_pixel_noclip (p, x, y);
  fast_free_drawbuffer (p);

  return (i);
}


/*****************************************************************************/
void 
db_180_line (PIXMAP *p, int xa, int ya, int xb, int yb)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_line (p, xa, ya, xb, yb);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);

  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, xa, ya, xb, yb);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_line_noclip (PIXMAP *p, int xa, int ya, int xb, int yb)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_line_noclip (p, xa, ya, xb, yb);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, xa, ya, xb, yb);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_hline (PIXMAP *p, int x1, int x2, int y)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_hline (p, x1, x2, y);
  fast_free_drawbuffer (p);

#else
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y, x2, y);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_hline_noclip (PIXMAP *p, int x1, int x2, int y)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_hline_noclip (p, x1, x2, y);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y, x2, y);

  fast_free_drawbuffer (p);

#endif
}

/*****************************************************************************/
void 
db_180_hlinelist_noclip (PIXMAP *p, int count, struct span_list *list)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_hlinelist_noclip (p, count, list);
  fast_free_drawbuffer (p);

#else

  int i, x1, x2, y;
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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

#endif
}


/*****************************************************************************/
void 
db_180_hlinelist (PIXMAP *p, int count, struct span_list *list)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_hlinelist (p, count, list);
  fast_free_drawbuffer (p);

#else

  int i, x1, x2, y;
  VGBOARD *b;

  b = p->board;
  fast_req_drawbuffer( p, 0);

  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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

#endif
}



/*****************************************************************************/
void 
db_180_vline (PIXMAP *p, int x, int ya, int yb)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_vline (p, x, ya, yb);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x, ya, x, yb);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_vline_noclip (PIXMAP *p, int x, int ya, int yb)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_vline_noclip (p, x, ya, yb);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x, ya, x, yb);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_rect (PIXMAP *p, int x1, int y1, int x2, int y2)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_rect (p, x1, y1, x2, y2);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y1, x2, y1);
  fast_line (b, x2, y1, x2, y2);
  fast_line (b, x2, y2, x1, y2);
  fast_line (b, x1, y2, x1, y1);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_rect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_rect_noclip (p, x1, y1, x2, y2);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_line (b, x1, y1, x2, y1);
  fast_line (b, x2, y1, x2, y2);
  fast_line (b, x2, y2, x1, y2);
  fast_line (b, x1, y2, x1, y1);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_fillrect (PIXMAP *p, int x1, int y1, int x2, int y2)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillrect (p, x1, y1, x2, y2);
  fast_free_drawbuffer (p);

#else
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_rect (b, x1, y1, x2, y2);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_fillrect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillrect_noclip (p, x1, y1, x2, y2);
  fast_free_drawbuffer (p);

#else
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  fast_rect (b, x1, y1, x2, y2);

  fast_free_drawbuffer (p);

#endif
}


/****************************************************************************/
void 
db_180_circle_noclip (PIXMAP *p, int x, int y, int r)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_circle_noclip (p, x, y, r);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_circle (PIXMAP *p, int x, int y, int r)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_circle (p, x, y, r);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_fillcircle (PIXMAP *p, int x, int y, int r)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillcircle (p, x, y, r);
  fast_free_drawbuffer (p);

#else

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
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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

#endif
}


/*****************************************************************************/
void 
db_180_fillcircle_noclip (PIXMAP *p, int x, int y, int r)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillcircle_noclip (p, x, y, r);
  fast_free_drawbuffer (p);

#else

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
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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

#endif
}


/*****************************************************************************/
void 
db_180_clear (PIXMAP *p)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_clear (p);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_B_MASK);
  fast_rect (b, 0, 0, vgl_x_res(p)-1, vgl_y_res(p)-1);

  fast_free_drawbuffer (p);

#endif
}


/*****************************************************************************/
void 
db_180_dissolve (PIXMAP *dest, PIXMAP *source, int speed)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_dissolve (dest, source, speed);

  fast_free_drawbuffer (source);
  fast_free_drawbuffer (dest);
}


/*****************************************************************************/
void 
db_180_ellipse (PIXMAP *p, int x, int y, int a, int b)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_ellipse (p, x, y, a, b);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_ellipse_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_ellipse_noclip (p, x, y, x_axis, y_axis);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_fillellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillellipse (p, x, y, x_axis, y_axis);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_fillellipse_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillellipse_noclip (p, x, y, x_axis, y_axis);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_arc (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_arc (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_arc_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_arc_noclip (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_fillarc (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillarc (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_fillarc_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis, int start, int end)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_fillarc_noclip (p, x, y, x_axis, y_axis, start, end);
  fast_free_drawbuffer (p);
}


/*****************************************************************************/
void 
db_180_bitblt (PIXMAP *source, int sx, int sy, int width, int height, 
	       PIXMAP *dest, int dx, int dy)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_bitblt (source, sx, sy, width, height, 
			  dest, dx, dy);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);

#else

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

  db_180_bitblt_noclip (source, sx, sy, width, height, dest, dx, dy);

#endif
}


/*****************************************************************************/
void 
db_180_bitblt_noclip (PIXMAP *source, int sx, int sy, int width, int height,
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
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
							 IGM_S_MASK);

  if (source->board == dest->board)
    {
      /* Source and destination are on the same board */
      ((board_db_180 *)(b->addr))->p9000_bt445.x0_abs = sx;
      ((board_db_180 *)(b->addr))->p9000_bt445.y0_abs = sy;
      ((board_db_180 *)(b->addr))->p9000_bt445.x1_abs = sx + width  - 1;
      ((board_db_180 *)(b->addr))->p9000_bt445.y1_abs = sy + height - 1;
      ((board_db_180 *)(b->addr))->p9000_bt445.x2_abs = dx;
      ((board_db_180 *)(b->addr))->p9000_bt445.y2_abs = dy;
      ((board_db_180 *)(b->addr))->p9000_bt445.x3_abs = dx + width  - 1;
      ((board_db_180 *)(b->addr))->p9000_bt445.y3_abs = dy + height - 1;

      do_blit(b);
    }
  else
    {
      /* The source is not on the board */

      if (!(sx&3))  /* If the source is word aligned */
	{
	  ((board_db_180 *)(b->addr))->p9000_bt445.x0_abs = dx;
	  ((board_db_180 *)(b->addr))->p9000_bt445.x1_abs = dx;
	  ((board_db_180 *)(b->addr))->p9000_bt445.y1_abs = dy;
	  ((board_db_180 *)(b->addr))->p9000_bt445.x2_abs = dx + width;
	  ((board_db_180 *)(b->addr))->p9000_bt445.y3_abs = 1;
	  
	  if (sx==0                    && sy==0                      &&
	      width==vgl_x_res(source) && height==vgl_y_res(source))
	    {
	      /* The entire source pixmap is being moved */
	      loc = source->pixdata->data;
	      i = ((vgl_x_res(source)*vgl_y_res(source))>>2);
	      x1 = loc + i;
	      pixel8 = &(((board_db_180 *)(b->addr))->p9000_bt445.pixel8_command);
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
	      pixel8 = &(((board_db_180 *)(b->addr))->p9000_bt445.pixel8_command);	      
	      P9000_issue_wait(b);

	      while (x1<x3)
		{
		  /* Do a line */
		  loc = x1;
/*  This is the original version 
		  while (loc<x2)
		    {
		      ((board_db_180 *)(b->addr))->p9000_bt445.pixel8_command =
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
db_180_transbitblt (PIXMAP *source, int sx, int sy, int width, int height, PIXMAP *dest, int dx, int dy, int mask_color)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_transbitblt (source, sx, sy, width, height,
			dest, dx, dy, 
			mask_color);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);
}


/*****************************************************************************/
void 
db_180_transbitblt_noclip (PIXMAP *source, int sx, int sy, int width, int height, PIXMAP *dest, int dx, int dy, int mask_color)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_transbitblt_noclip (source, sx, sy, width, height,
			       dest, dx, dy, 
			       mask_color);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);
}


/*****************************************************************************/
void 
db_180_transbitblt_core (PIXMAP *source, int sx, int sy, int width, int height, PIXMAP *dest, int dx, int dy, int mask_color)
{
  fast_req_drawbuffer( source, 0);
  fast_req_drawbuffer( dest, 0);

  vgl_dumb_transbitblt_core (source, sx, sy, width, height,
			     dest, dx, dy,
			     mask_color);

  fast_free_drawbuffer (dest);
  fast_free_drawbuffer (source);
}


/****************************************************************************/
void 
db_180_setfont (PIXMAP *p, struct vgl_ffont *ffont)
{
  p->font = ffont;
}


/****************************************************************************/
int
db_180_pixmap_test( PIXMAP *p, char *msg)
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
void db_180_text (PIXMAP *p, int x, int y, char *s)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_text (p, x, y, s);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;
  unsigned long *command, *bitmap;

  b = p->board;
  command = (((board_db_180 *)(b->addr))->p9000_bt445.pixel1_base_command +
	     (p->font->width-1));

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED |
     ((IGM_S_MASK & IGM_F_MASK) | (~IGM_S_MASK & IGM_B_MASK)));

  ((board_db_180 *)(b->addr))->p9000_bt445.x0_abs = x;
  ((board_db_180 *)(b->addr))->p9000_bt445.x1_abs = x;
  ((board_db_180 *)(b->addr))->p9000_bt445.y1_abs = y;
  ((board_db_180 *)(b->addr))->p9000_bt445.x2_abs = x + p->font->width;
  ((board_db_180 *)(b->addr))->p9000_bt445.y2_abs = y;
  ((board_db_180 *)(b->addr))->p9000_bt445.y3_abs = 1;

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

      ((board_db_180 *)(b->addr))->p9000_bt445.next_pixels_command =
	p->font->width;
      s++;
    }

  fast_free_drawbuffer (p);
  
#endif
}


/****************************************************************************/
void db_180_text2 (PIXMAP *p, int x, int y, char *s, int len)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer( p, 0);
  vgl_dumb_text2 (p, x, y, s, len);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;
  unsigned long *command, *bitmap;
  char *end;

  b = p->board;
  command = (((board_db_180 *)(b->addr))->p9000_bt445.pixel1_base_command +
	     (p->font->width-1));
  end = s+len;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED |
     ((IGM_S_MASK & IGM_F_MASK) | (~IGM_S_MASK & IGM_B_MASK)));

  ((board_db_180 *)(b->addr))->p9000_bt445.x0_abs = x;
  ((board_db_180 *)(b->addr))->p9000_bt445.x1_abs = x;
  ((board_db_180 *)(b->addr))->p9000_bt445.y1_abs = y;
  ((board_db_180 *)(b->addr))->p9000_bt445.x2_abs = x + p->font->width;
  ((board_db_180 *)(b->addr))->p9000_bt445.y2_abs = y;
  ((board_db_180 *)(b->addr))->p9000_bt445.y3_abs = 1;

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

      ((board_db_180 *)(b->addr))->p9000_bt445.next_pixels_command =
	p->font->width;
      s++;
    }

  fast_free_drawbuffer (p);
  
#endif
}


/****************************************************************************/
void db_180_poly (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_poly (p, n_vert, vert);
  fast_free_drawbuffer (p);
}


/****************************************************************************/
void db_180_fillpoly (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_fillpoly (p, n_vert, vert);
  fast_free_drawbuffer (p);

#else

  int     i;
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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
      

      if ( ((board_db_180 *)(b->addr))->p9000_bt445.status & P9000_QUAD_SW)
	vgl_dumb_fillpoly_noclip (p, n_vert, vert);

      break;

    default:
      vgl_dumb_fillpoly (p, n_vert, vert);
      return;
    }

  fast_free_drawbuffer (p);

#endif
}


/****************************************************************************/
void db_180_poly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
  fast_req_drawbuffer( p, 0);
  vgl_dumb_poly_noclip (p, n_vert, vert);
  fast_free_drawbuffer (p);
}


/****************************************************************************/
void db_180_fillpoly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_fillpoly_noclip (p, n_vert, vert);
  fast_free_drawbuffer (p);

#else

  int     i;
  VGBOARD *b;

  b = p->board;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
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

      if ( ((board_db_180 *)(b->addr))->p9000_bt445.status & P9000_QUAD_SW)
	vgl_dumb_fillpoly_noclip (p, n_vert, vert);

      break;

    default:
      vgl_dumb_fillpoly (p, n_vert, vert);
      return;
    }

  fast_free_drawbuffer (p);

#endif
}


/****************************************************************************/
/****************************************************************************/
void db_180_quadlist (PIXMAP *p, int n_quads, struct vgl_quad_def *quad, int xoffset, int yoffset)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_quadlist (p, n_quads, quad);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;
  int i, cur_color;

  b = p->board;

  cur_color = quad->color;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED | IGM_F_MASK);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (xoffset, yoffset);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = 
    cur_color;

  i = 0;
  while (i<n_quads)
    {
      if (cur_color!=quad->color)
	{
	  cur_color = quad->color;
	  P9000_busy_wait(b);
	  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = cur_color;
	}

      fast_quad_packed (b, quad->p1, quad->p2, quad->p3, quad->p4);
      quad++;
      i++;
    }

  /* Set the color back to what it "should" be */
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (0,0);

  fast_free_drawbuffer (p);

#endif
}

/****************************************************************************/
/****************************************************************************/
void db_180_linelist (PIXMAP *p, int n_lines, struct vgl_line_def *line, int xoffset, int yoffset)
{
#ifndef USE_ACCEL

  fast_req_drawbuffer(p, 0);
  vgl_dumb_linelist (p, n_lines, line);
  fast_free_drawbuffer (p);

#else

  VGBOARD *b;
  int i, cur_color;

  b = p->board;

  cur_color = line->color;

  fast_req_drawbuffer( p, 0);
  P9000_busy_wait(b);
  ((board_db_180 *)(b->addr))->p9000_bt445.raster =
    (P9000_OVERSIZED | IGM_F_MASK);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (xoffset, yoffset);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = 
    cur_color;

  i = n_lines;
  while (i>0)
    {
      if (cur_color!=line->color)
	{
	  cur_color = line->color;
	  P9000_busy_wait(b);
	  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = cur_color;
	}

      fast_line_packed (b, line->p1, line->p2); 
      line++;
      i--;
    }

  /* Set the color back to what it "should" be */
  P9000_busy_wait(p->board);
  ((board_db_180 *)(p->board->addr))->p9000_bt445.fground = p->foreground;
  ((board_db_180 *)(p->board->addr))->p9000_bt445.w_off_xy = 
    pack_xy (0,0);
  fast_free_drawbuffer (p);
  
#endif
}


/****************************************************************************/
/****************************************************************************/

#endif
