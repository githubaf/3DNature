
#include "vgl.h"
#include "vgl_internals.h"

#ifdef SUPPORT_GDM_9000

#define USE_IRQ

#include "gdm_9000.h"


/* Just change DIM_SHIFT, not DIM_MASK1 or DIM_MASK */
#define DIM_SHIFT	(2)
#define	DIM_MASK1	((256>>DIM_SHIFT)-1)
#define DIM_MASK	((DIM_MASK1) | (DIM_MASK1<<8) | (DIM_MASK1<<16) | (DIM_MASK1<<24))


static void gdm_9000_video_isr (int board_index);


#ifdef PSOS
static void gdm_9000_video_init ( int b_num, VGBOARD *b, void (*handler) (void));
#else
static void gdm_9000_video_init ( int b_num, VGBOARD *b, void (*handler) (int b_num));
#endif


/* 
   The maximum number of DB-180's that can operate in one system.
   Note: If this number is changed, the number of ISR_STUB()'s must be changed.
*/
#define	MAX_GDM_9000	(8)

static VGBOARD *board_list[MAX_GDM_9000];
static int board_list_init = 0;

/* Look closely, these are the actual functions! */
#ifdef PSOS
void gdm_9000_isrstub0 (void){  gdm_9000_video_isr (0);}
void gdm_9000_isrstub1 (void){  gdm_9000_video_isr (1);}
void gdm_9000_isrstub2 (void){  gdm_9000_video_isr (2);}
void gdm_9000_isrstub3 (void){  gdm_9000_video_isr (3);}
void gdm_9000_isrstub4 (void){  gdm_9000_video_isr (4);}
void gdm_9000_isrstub5 (void){  gdm_9000_video_isr (5);}
void gdm_9000_isrstub6 (void){  gdm_9000_video_isr (6);}
void gdm_9000_isrstub7 (void){  gdm_9000_video_isr (7);}

static void (*gdm_9000_stub_list[]) (void) =
{
  gdm_9000_isrstub0,
  gdm_9000_isrstub1,
  gdm_9000_isrstub2,
  gdm_9000_isrstub3,
  gdm_9000_isrstub4,
  gdm_9000_isrstub5,
  gdm_9000_isrstub6,
  gdm_9000_isrstub7,
};

#endif /* PSOS */

static PIXMAP gdm_9000_default_pixmap =
  {
    NULL,			/* pixdata */
    VGL_ERR_NONE,		/* error */
    NULL,			/* board */
    NULL,		        /* mouse_pixmap */
    CLIP_ON, 0, 0, 0, 0,	/* clip_flag, x_min, y_min, x_max, y_max */
    
    255, 0,			/* Forground & background */
    0, 0,			/* fore_8, back_8 */
    {0},			/* expand_8[] */
    NULL,			/* Font */

    NULL,			/* Workspace */
    0,				/* Worksize */

    0,				/* Mouse_nocheck_flag */

    0,				/* data1 (DB-180=Buffer number) */
    
    gdm_9000_set_clip,
    gdm_9000_clip_off,
    gdm_9000_clip_on,
    gdm_9000_pixerror,
    gdm_9000_set_pixel,
    gdm_9000_get_pixel,
    gdm_9000_line,
    gdm_9000_hline,
    gdm_9000_vline,
    gdm_9000_rect,
    gdm_9000_fillrect,
    gdm_9000_circle,
    gdm_9000_fillcircle,
    gdm_9000_ellipse,
    gdm_9000_fillellipse,
    gdm_9000_arc,
    gdm_9000_fillarc,
    gdm_9000_poly,
    gdm_9000_fillpoly,
    gdm_9000_bitblt,
    gdm_9000_transbitblt,
    gdm_9000_setfont,
    gdm_9000_text,
    gdm_9000_text2,
    gdm_9000_clear,
    gdm_9000_dissolve,
    gdm_9000_setcur,
    gdm_9000_pixmap_test,
    vgl_dumb_nothing,
    gdm_9000_quadlist,
    gdm_9000_linelist,
    gdm_9000_hlinelist,
  };



/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_board_init (VGBOARD *b)
{
  int i, j;

  /***** Initalize the board list, if required *****/
  if (board_list_init == 0)
    {
      for (i = 0; i < MAX_GDM_9000; i++)
	board_list[i] = NULL;

      board_list_init = 1;
    }

  /***** Find an empty board slot *****/
  for (i = 0; i < MAX_GDM_9000; i++)
    {
      if (board_list[i] == NULL)
	break;
    }

  /***** Set error code and return, on error *****/
  if (i >= MAX_GDM_9000)
    {
      b->error = VGL_ERR_MAX_BOARDS;
      return;
    }

  /***** Add new board to board list *****/
  board_list[i] = b;

  /***** Clear irq masks *****/
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.interrupt_en = 0xAA;

  /***** Set irq_vector *****/
#ifdef USE_IRQ
  ((board_gdm_9000 *)(b->addr))->ivr = b->irq_vector;
#endif

  /***** Initalize board sub-systems *****/
#if defined (PSOS)
  gdm_9000_video_init (i, b, gdm_9000_stub_list[i]);
#elif defined (VXWORKS) || defined (OS9)
  gdm_9000_video_init (i, b, gdm_9000_video_isr);
#endif

  /***** Set irq masks *****/
#ifdef USE_IRQ
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.interrupt_sr = 0x2A;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.interrupt_en = 0xF0;

  ((board_gdm_9000 *)(b->addr))->intr_en = 0;
#endif

  /***** Enable the +12 Volts *****/
/*
  ((board_gdm_9000 *)(b->addr))->lcd_enable = 0;
*/
}


/****************************************************************************/
/****************************************************************************/
#ifdef PSOS
static void 
gdm_9000_video_init (int b_num, VGBOARD *b, void (*handler) (void))
#else
static void 
gdm_9000_video_init (int b_num, VGBOARD *b, void (*handler) (int param))
#endif
{
  int i;
  long *j;
/*
  ((board_gdm_9000 *)(b->addr))->reset = 0;
  vgl_sleep ((20 * VGL_CLOCK) / 60);
*/
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig =
    b->mode->gdm_9000.sysconfig;

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.hrzt = b->mode->gdm_9000.hrzt;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.hrzsr = b->mode->gdm_9000.hrzsr;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.hrzbr = b->mode->gdm_9000.hrzbr;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.hrzbf = b->mode->gdm_9000.hrzbf;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.prehrzc=b->mode->gdm_9000.prehrzc;

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.vrtt = b->mode->gdm_9000.vrtt;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.vrtsr = b->mode->gdm_9000.vrtsr;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.vrtbr = b->mode->gdm_9000.vrtbr;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.vrtbf = b->mode->gdm_9000.vrtbf;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.prevrtc=b->mode->gdm_9000.prevrtc;

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.srtctl = b->mode->gdm_9000.srtctl;

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.mem_config =
    b->mode->gdm_9000.mem_config;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.rfperiod = 
    b->mode->gdm_9000.rfperiod;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.rlmax =
    b->mode->gdm_9000.rlmax;

  /* Initalize the bt445 ctrl reg set */
  for( i=0; i<16; i++)
    {
      if( b->mode->gdm_9000.bt445_ctrl[i] != -1)
	{
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;

	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.ctrl =
	    b->mode->gdm_9000.bt445_ctrl[i];
	}
    }

  /* Reset the bt445 pipeline */
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 1;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.ctrl = 
    b->mode->gdm_9000.bt445_ctrl[1] | 1;

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 1;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.ctrl =
    b->mode->gdm_9000.bt445_ctrl[1] & 0xFE;

  b->tos_reset = 0;
  b->tos_value = 0;
  b->color_end = 0;
  b->color_start = 0;
  b->cur_draw_context = NULL;
  b->draw_irq_flag = 0;
  b->n_users = 0;

  /* Force the CLUT to a greyscale */
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 0;
  for (i = 0; i < 256; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = i;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = i;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = i;
    }

  /* Now, nicely set the CLUT to the default */
  vgl_setrgb (b, 0, 256, vgl_default_clut);

  
  P9000_busy_wait(b);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.pmask = 0xFF;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x0A;
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.w_off_xy = pack_xy(0,0);
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.raster = (P9000_OVERSIZED |
						     IGM_F_MASK);

  vgl_install_isr (b->irq_vector, handler, b_num);
}

/****************************************************************************/
/****************************************************************************/
static void 
gdm_9000_video_isr (int board_index)
{
  int i, k;
  unsigned long j;
  VGBOARD *b;

  b = board_list[board_index];
  if (b == NULL)
    return;


  /* Undraw the mouse pointer */
  if (b->mouse_visible_flag != 0 && b->mouse_update_flag != 0)
    {
      if (b->cur_pixmap->mouse_pixmap != NULL && b->mouse_under != NULL)
	vgl_bitblt (b->mouse_under,
		    0, 0, -1, -1,
	       b->cur_pixmap->mouse_pixmap, b->mouse_old_x, b->mouse_old_y);
    }

  /* Set top of screen */
  if (b->tos_reset)
    {
/*  This part is done before, but is shown here for "completeness"...
      if ((b->tos_value & 1)!=0)
	((board_gdm_9000 *)(b->addr))->p9000_bt445.srtctl |= 1<<3;
      else
	((board_gdm_9000 *)(b->addr))->p9000_bt445.srtctl &= ~(1<<3);
*/

      /* Turn off mouse checking for old pixmap */
      b->cur_pixmap->mouse_nocheck_flag = 1;	

      /* Switch pixmaps */
      b->cur_pixmap = b->next_cur_pixmap;

      /* Turn on mouse checking for new pixmap */
      b->cur_pixmap->mouse_nocheck_flag = 0;

      b->next_cur_pixmap = NULL;
      b->tos_reset = 0;
    }


  /* Do Something with the screen saver */
  if (b->ss_status == SS_NORMAL)
    {
      if (b->ss_counter < 0 && b->ss_istatus != 0)
	{
	  b->ss_istatus = 0;	/* Turn it off */
	  b->color_start = 0;
	  b->color_end = 256;
/*
	  ((board_gdm_9000 *)(b->addr))->lcd_disable=0;
*/
	}
      else if (b->ss_counter >= 0 && b->ss_istatus != 1)
	{
	  b->ss_istatus = 1;	/* Turn it on */
	  b->color_start = 0;
	  b->color_end = 256;
/*
	  ((board_gdm_9000 *)(b->addr))->lcd_enable=0;
*/
	}
      else if (b->ss_counter >= 0)
	b->ss_counter--;
    }
  else if (b->ss_status == SS_OFF)	/* If the screen is "turned off" */
    {
      if (b->ss_istatus != 0)	/* Is the screen already off? */
	{
	  b->ss_istatus = 0;	/* Turn it off */
	  b->color_start = 0;
	  b->color_end = 256;
/*
	  ((board_gdm_9000 *)(b->addr))->lcd_disable=0;
*/
	}
    }
  else
    /* if(b->status == SS_ON) */
    /* If the screen is "turned on"... */
    {
      if (b->ss_istatus != 1)	/* if the screen is off, turn it on */
	{
	  b->ss_istatus = 1;
	  b->color_start = 0;
	  b->color_end = 256;
/*
	  ((board_gdm_9000 *)(b->addr))->lcd_enable=0;
*/
	}
    }


  /* Set the color look-up table (CLUT) */
  if (b->color_start != b->color_end)
    {
      i = b->color_start;

      if ( b->ss_istatus != 0)
	{
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
	  for (; i < b->color_end; i++)
	    {
	      j = b->color_table[i];
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = j & 0xFF;
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 8) & 0xFF;
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 16)  &0xFF;
	    }
	}
      else
	{
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
	  for (; i < b->color_end; i++)
	    {
	      j = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = j & 0xFF;
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 8) & 0xFF;
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 16)  & 0xFF;
	    }
	}

      b->color_start = 0;
      b->color_end = 0;
    }

  /* Update mouse pointer coords if needed */
  if (b->mouse_update_flag != 0)
    {
      b->mouse_old_x = b->mouse_cur_x;
      b->mouse_old_y = b->mouse_cur_y;
    }

  if (b->mouse_blanking_level == 0 && (b->mouse_visible_flag == 0 || b->mouse_update_flag != 0))
    {
      if (b->mouse_under != NULL && b->cur_pixmap->mouse_pixmap != NULL)
	vgl_bitblt (b->cur_pixmap->mouse_pixmap,
		    b->mouse_old_x, b->mouse_old_y,
		    vgl_x_res (b->mouse_under), vgl_y_res (b->mouse_under),
		    b->mouse_under,
		    0, 0);

      if (b->cur_pixmap->mouse_pixmap != NULL && b->mouse_ptr != NULL)
	vgl_transbitblt (b->mouse_ptr,
			 0, 0, -1, -1,
	    b->cur_pixmap->mouse_pixmap, b->mouse_old_x, b->mouse_old_y, 0);

      b->mouse_visible_flag = 1;
    }

  b->mouse_update_flag = 0;
  b->frame_count++;

  /* Reset interrupt */
#ifdef USE_IRQ
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.interrupt_sr = 0x20;
  ((board_gdm_9000 *)(b->addr))->intr_en = 0;
#endif
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_board_kill (VGBOARD *b)
{
  int i;

  /***** Disable the +12 Volts *****/
/*
  ((board_gdm_9000 *)(b->addr))->lcd_disable = 0;
*/

  /***** Clear irq masks *****/
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.interrupt_en = 0xAA;
  ((board_gdm_9000 *)(b->addr))->intr_dis = 0;

  if (b->mouse_under != NULL)
    vgl_freepixmap (b->mouse_under);

  if (b->cur_pixmap->mouse_pixmap != NULL)
    vgl_freepixmap (b->cur_pixmap->mouse_pixmap);

  for (i = 0; i < MAX_GDM_9000; i++)
    {
      if (board_list[i] == b)
	board_list[i] = NULL;
    }

  free (b);
}



/****************************************************************************/
/****************************************************************************/
PIXMAP *
gdm_9000_pixmap_map (VGBOARD *b, int pagenum)
{
  PIXMAP *p;

  p = vgl_malloc (sizeof (PIXMAP));
  if (p)
    {
      *p = gdm_9000_default_pixmap;
      p->pixdata = vgl_malloc (sizeof (p->pixdata[0]));
      if (p->pixdata == NULL)
	{
	  vgl_free (p);
	  return (NULL);
	}

      p->pixdata->x_res = b->mode->x_res;
      p->pixdata->y_res = b->mode->y_res;
      p->pixdata->x_size = b->mode->gdm_9000.x_size;
      p->pixdata->users = 1;
      p->pixdata->free_flag = 0;

      /* All DB-180 buffers start at the same address. */
      p->pixdata->data = ((board_gdm_9000 *)(b->addr))->vram;
      p->data1 = pagenum;	/* The buffer number is stored in data1 */

      p->clip_x_max = p->pixdata->x_res;
      p->clip_y_max = p->pixdata->y_res;
      p->clip_x_min = 0;
      p->clip_y_min = 0;

      p->board = b;
      p->font = vgl_large_ffont;

      vgl_setcur (p, p->foreground, p->background);

      p->mouse_pixmap = vgl_dupepixmap (p);
      vgl_set_clip (p->mouse_pixmap, -1, -1, -1, -1);
      vgl_clip_on (p->mouse_pixmap);
      p->mouse_pixmap->mouse_nocheck_flag = 1;

      if (b->cur_pixmap == NULL && pagenum == 0)
	{
	  /* An inline, modified verson of: gdm_9000_pixmap_disp( p); */
	  b->mouse_blanking_level = 1;
	  b->mouse_visible_flag = 0;

	  b->cur_pixmap = p;
	  b->next_cur_pixmap = p;
	  p->board->tos_value = pagenum;

	  b->tos_reset = 1;
	  b->mouse_update_flag = 1;

	  if ((b->tos_value & 1)!=0)
	    ((board_gdm_9000 *)(b->addr))->p9000_bt445.srtctl |= 1<<3;
	  else
	    ((board_gdm_9000 *)(b->addr))->p9000_bt445.srtctl &= ~(1<<3);
	}

      return (p);
    }
  else
    return (NULL);
}


/****************************************************************************/
/*   Request a drawing buffer.  This makes the buffer accessable to the
 *   host CPU (and other things).  It does blocking so that two buffers are
 *   not accessed at the same time.  If the irq_flag is set, the buffer is
 *   handed over immidiately, without blocking.  Each call to
 *   gdm_9000_req_drawbuffer() should be followed with a call to
 *   gdm_9000_free_drawbuffer().
 */
/*****************************************************************************/
void
gdm_9000_req_drawbuffer (PIXMAP *p, int irq_flag)
{
  VGBOARD *b;
  int buffer;

  b = p->board;

  if (b->type == GDM_9000)
    {
      /* "get" the board */
      vgl_access_p (b);

      b->n_users++;
      buffer = p->data1 & 1;	/* There are only two buffers... */

      if (p != b->cur_draw_context)
	{
	  P9000_busy_wait(b);

	  /* Select the current drawing buffer */
	  if (buffer!=0)
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig = 
		b->mode->gdm_9000.sysconfig | (1<<9) | (1<<10);
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x03;
	    }
	  else
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig = 
		b->mode->gdm_9000.sysconfig & ~((1<<9) | (1<<10));
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x02;
	    }

	  /* Set current forground and background colors */
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.fground = 
	    p->foreground;
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bground = 
	    p->background;

	  /* Set current clipping paramaters */
	  if (p->clip_setting==CLIP_ON)
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.w_min =
		pack_xy (p->clip_x_min, p->clip_y_min);
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.w_max =
		pack_xy (p->clip_x_max-1, p->clip_y_max-1);
	    }
	  else
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.w_min =
		pack_xy (0,0);
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.w_max =
		pack_xy (vgl_x_res(p)-1, vgl_y_res(p)-1);
	    }

	  b->cur_draw_context = p;
	}
    }
}


/****************************************************************************/
/****************************************************************************/
void
gdm_9000_free_drawbuffer (PIXMAP *p)
{
  if (p->board->type == GDM_9000)
    {
      p->board->n_users--;
      if (p->board->n_users==0)
	vgl_access_v (p->board);
    }
}

/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_pixmap_disp (PIXMAP *p)
{
  unsigned long temp;

  if (p->board != NULL)
    {
      temp = vgl_mask_irq (p->board);

      p->board->tos_value = p->data1;

      p->board->next_cur_pixmap = p;

      p->board->tos_reset = 1;
      p->board->mouse_update_flag = 1;

#ifndef USE_IRQ
      /* Copied from ISR */
      /* Set top of screen */
      if (p->board->tos_reset)
	{
	  if ((p->board->tos_value & 1)!=0)
	    ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.srtctl |= 1<<3;
	  else
	    ((board_gdm_9000 *)(p->board->addr))->p9000_bt445.srtctl &= ~(1<<3);
	  
	  /* Turn off mouse checking for old pixmap */
	  p->board->cur_pixmap->mouse_nocheck_flag = 1;	
	  
	  /* Switch pixmaps */
	  p->board->cur_pixmap = p->board->next_cur_pixmap;
	  
	  /* Turn on mouse checking for new pixmap */
	  p->board->cur_pixmap->mouse_nocheck_flag = 0;
	  
	  p->board->next_cur_pixmap = NULL;
	  p->board->tos_reset = 0;
	}
#else

      if ((p->board->tos_value & 1)!=0)
	((board_gdm_9000 *)(p->board->addr))->p9000_bt445.srtctl |= 1<<3;
      else
	((board_gdm_9000 *)(p->board->addr))->p9000_bt445.srtctl &= ~(1<<3);

#endif

      vgl_unmask_irq (p->board, temp);
    }
  else
    p->error = VGL_ERR_BADOP;
}

/****************************************************************************/
/****************************************************************************/
int 
gdm_9000_boarderror (VGBOARD *b)
{
  int err;

  err = b->error;
  b->error = VGL_ERR_NONE;
  return (err);
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_setrgb (VGBOARD *b, int color_num, int n_colors, struct vgl_color *colors)
{
  int i;
  unsigned long old_mode;

#ifndef USE_IRQ
  int k;
  unsigned long j;
#endif

  if (color_num < 0 || color_num > 255 || color_num + n_colors > 256)
    b->error = VGL_ERR_RANGE;
  else
    {
      old_mode = vgl_mask_irq (b);

      for (i = 0; i < n_colors; i++)
	{
	  b->color_table[color_num + i] =
	    (colors[i].blue & 0xFF) << 16 |
	    (colors[i].green & 0xFF) << 8 |
	    (colors[i].red & 0xFF);
	}

      b->color_start = min (color_num, b->color_start);
      i = color_num + n_colors;
      b->color_end = max (i, b->color_end);

#ifndef USE_IRQ

      /* This was cut right out of gdm_9000_video_isr() function */
      if (b->color_start != b->color_end)
	{
	  i = b->color_start;
	  
	  if ( b->ss_istatus != 0)
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
	      for (; i < b->color_end; i++)
		{
		  j = b->color_table[i];
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = j & 0xFF;
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 8) & 0xFF;
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 16)  &0xFF;
		}
	    }
	  else
	    {
	      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
	      for (; i < b->color_end; i++)
		{
		  j = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = j & 0xFF;
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 8) & 0xFF;
		  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = (j >> 16)  & 0xFF;
		}
	    }	  
	  b->color_start = 0;
	  b->color_end = 0;
	}
#endif

      vgl_unmask_irq (b, old_mode);
    }
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_getrgb (VGBOARD *b, int color_num, int n_colors, struct vgl_color *colors)
{
  int i;

  if (color_num < 0 || color_num > 255 || color_num + n_colors > 256)
    b->error = VGL_ERR_RANGE;
  else
    {
      for (i = 0; i < n_colors; i++)
	{
	  colors[i].red = b->color_table[color_num + i] & 0xFF;
	  colors[i].green = (b->color_table[color_num + i] >> 8) & 0xFF;
	  colors[i].blue = (b->color_table[color_num + i] >> 16) & 0xFF;
	}
    }
}

/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_getcoords (VGBOARD *b, int *x, int *y, int *buttons, int wait_flag)
{
  b->error = VGL_ERR_NOSUPPORT;
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_setcoords (VGBOARD *b, int x, int y)
{
  b->error = VGL_ERR_NOSUPPORT;
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_sethandler (VGBOARD *b, void (*fctn) (int x, int y, int buttons))
{
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_setpixmap (VGBOARD *b, PIXMAP * pixmap, int hot_x, int hot_y)
{
  b->error = VGL_ERR_NOSUPPORT;
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_ptr_on (VGBOARD *b)
{
  b->error = VGL_ERR_NOSUPPORT;
}

/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_ptr_off (VGBOARD *b)
{
  b->error = VGL_ERR_NOSUPPORT;
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_mouse_setaccel (VGBOARD *b, int base_speed, int accel_thresh, int accel_num, int accel_den)
{
  b->error = VGL_ERR_NOSUPPORT;
}

/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_ss_set (VGBOARD *b, short status, long timeout, short flags)
{
  unsigned long temp;

  temp = vgl_mask_irq (b);

  b->ss_status = status;
  b->ss_flags = flags;
  b->ss_counter = timeout;
  b->ss_timeout_value = timeout;

  vgl_unmask_irq (b, temp);
}


/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_ss_tick (VGBOARD *b, short flags)
{
  if (b->ss_status == SS_NORMAL && (b->ss_flags & flags) != 0)
    b->ss_counter = b->ss_timeout_value;
}

/****************************************************************************/
/****************************************************************************/
int 
gdm_9000_kbhit (VGBOARD *b)
{
  b->error = VGL_ERR_NOSUPPORT;
  return (0);
}


/****************************************************************************/
/****************************************************************************/
long 
gdm_9000_getkey (VGBOARD *b, int wait_flag)
{
  b->error = VGL_ERR_NOSUPPORT;
  return (0);
}

/****************************************************************************/
/****************************************************************************/
void 
gdm_9000_kb_setled (VGBOARD *b, unsigned long led_flags)
{
  b->error = VGL_ERR_NOSUPPORT;
}


/****************************************************************************/
/****************************************************************************/
static int gdm_9000_clut_test (VGBOARD *b, int pass, char *msg);

int
gdm_9000_test (VGBOARD *b, char *msg)
{
  int i, err, n_buf;
  unsigned long buf_size;

  switch (b->mode->gdm_9000.mem_config)
    {
    case 0: /* 1 buffer  of 1 meg */
    case 1: /* 1 buffer  of 1 meg */
      n_buf = 1;
      buf_size = 1024*1024;
      break;

    case 2: /* 1 buffer  of 2 meg */
      n_buf = 1;
      buf_size = 2*1024*1024;
      break;

    case 3: /* 2 buffers of 1 meg */
      n_buf = 2;
      buf_size = 1024*1024;
      break;

    case 4: /* 2 buffers of 2 meg */
      n_buf = 2;
      buf_size = 2*1024*1024;
      break;

    default:
      n_buf = 1;
      buf_size = 1024*1024;
    }


  for( i=0; i<1; i++)
    {
      /* Test Buffer 0 */
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig = 
	b->mode->gdm_9000.sysconfig & ~((1<<9) | (1<<10));
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x02;
      err = vgl_mem_test (((board_gdm_9000 *)(b->addr))->vram, buf_size, msg);
      if (err!=VGL_TEST_OK)
	{
	  vgl_strcat (msg, ", Buf #0");
	  return (err);
	}

      /* Test Buffer 1 */
      if (n_buf>1)
	{
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig = 
	    b->mode->gdm_9000.sysconfig | (1<<9) | (1<<10);
	  ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x03;
	  err = vgl_mem_test (((board_gdm_9000 *)(b->addr))->vram, buf_size, msg);
	  if (err!=VGL_TEST_OK)
	    {
	      vgl_strcat (msg, ", Buf #1");
	      return (err);
	    }
	}


      /* Test the CLUT */
      err = gdm_9000_clut_test (b, i, msg);
      if (err!=VGL_TEST_OK)
	return (err);

      /* Test the CLUT, again */
      err = gdm_9000_clut_test (b, i, msg);
      if (err!=VGL_TEST_OK)
	return (err);
    }

  ((board_gdm_9000 *)(b->addr))->p9000_bt445.sysconfig = 
    b->mode->gdm_9000.sysconfig & ~((1<<9) | (1<<10));
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.draw_mode = 0x02;
  b->cur_draw_buffer = 0;

  return (VGL_TEST_OK);
}


static int
gdm_9000_clut_test (VGBOARD *b, int pass, char *msg)
{
  int i, v, err;

  err = 0;

  /* Generate static for no real good reason */
/*
  v = 1;
  for ( i=0; i < 100000; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i & 0xFF;

      if ((v) & 1)
	v = v >> 1 ^ 0xB8;
      else
	v >>= 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = v;

      if ((v) & 1)
	v = v >> 1 ^ 0xB8;
      else
	v >>= 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = v;

      if ((v) & 1)
	v = v >> 1 ^ 0xB8;
      else
	v >>= 1;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = v;
    }
*/

  /* Test individual CLUT entries */
  for ( i=0; i < 256; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x55;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xFF;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xAA;

      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x55)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, RED #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x55, v);
	  err = 1;
	  break;
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xFF)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, GREEN #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xFF, v);

	  err = 1;
	  break;
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xAA)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, BLUE #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xAA, v);

	  err = 1;
	  break;
	}
    }

  for ( i=0; i < 256; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xAA;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x00;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x55;

      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = i;
      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xAA)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, RED #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xAA, v);
	  err = 1;
	  break;
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x00)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, GREEN #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x00, v);

	  err = 1;
	  break;
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x55)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, individual, pass %d, BLUE #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x55, v);

	  err = 1;
	  break;
	}
    }


  /* Check for a stuck clut bit */
  /* Write the entire CLUT */
#if 1
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 0;
  for ( i=0; i < 256; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xFF;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xAA;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x55;
    }

  /* Write the entire CLUT */
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 0;
  for ( i=0; i < 256; i++)
    {
      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xFF)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, RED #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xFF, v);
	  return (VGL_TEST_BADCLUT);
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xAA)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, GREEN #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xAA, v);
	  return (VGL_TEST_BADCLUT);
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x55)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, BLUE #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x55, v);
	  return (VGL_TEST_BADCLUT);
	}
    }


  /* Write the entire CLUT */
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 0;
  for ( i=0; i < 256; i++)
    {
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x00;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0x55;
      ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut = 0xAA;
    }

  /* Write the entire CLUT */
  ((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.addr = 0;
  for ( i=0; i < 256; i++)
    {
      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x00)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, RED #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x00, v);
	  return (VGL_TEST_BADCLUT);
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0x55)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, GREEN #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0x55, v);
	  return (VGL_TEST_BADCLUT);
	}

      if( (v=((board_gdm_9000 *)(b->addr))->p9000_bt445.bt445.mainlut) != 0xAA)
	{
	  *(char *)(((board_gdm_9000 *)(b->addr))->vram) = 0x00;

	  sprintf (msg, "CLUT, auto-inc, pass %d, BLUE #0x%02X.  "
		   "Wrote 0x%02X, read 0x%02X\n",
		   pass, i, 0xAA, v);
	  return (VGL_TEST_BADCLUT);
	}
    }
#endif


  return (VGL_TEST_OK);
}


/****************************************************************************/
/****************************************************************************/

#endif /* SUPPORT_GDM_9000  */
