
#ifndef	_VGL_INTERNALS_H_
#define	_VGL_INTERNALS_H_

#include "vgl.h"
#include "vgl_config.h"


/*****************************************************************************/
/**************************  Things Specific to PSOS *************************/
/*****************************************************************************/
#if defined(PSOS)

/*  include prepc.h insted.
 *  #include <stdlib.h>
 */
#include <prepc.h>

void vgl_install_isr (short vector, void (*fctn) (void), int param);

#define USE_VGL_BZERO
#define USE_VGL_MALLOC
#define USE_VGL_STRICMP
#undef  USE_VGL_STRCPY
#undef  USE_VGL_STRCAT
#undef  USE_VGL_MEMCPY
#undef  USE_VGL_QUEUE
#undef  DONT_CHECK_MOUSE
#define INLINE_CHECK_MOUSE

#define vgl_board_permit(vgboard)  (0)


/*****************************************************************************/
/********************  (else) Things Specific to VX WORKS ********************/
/*****************************************************************************/
#elif defined (VXWORKS)

#include <vxWorks.h>

/* Get the INUM_TO_IVEC macro */
#ifdef TARGET_MVME167
#include <arch/mc68k/ivMc68k.h>
#elif TARGET_SUN2CE
#include <arch/sparc/ivSparc.h>
#endif

void vgl_install_isr (short vector, void (*fctn) (int param), int param);

#undef  USE_VGL_BZERO
#define USE_VGL_MALLOC
#define USE_VGL_STRICMP
#undef  USE_VGL_STRCPY
#undef  USE_VGL_STRCAT
#undef  USE_VGL_MEMCPY
#undef  USE_VGL_QUEUE
#undef  DONT_CHECK_MOUSE
#define INLINE_CHECK_MOUSE

#define vgl_board_permit(vgboard)   (0)


/*****************************************************************************/
/**************************  (else) Things for OS/9 **************************/
/*****************************************************************************/
#elif defined (OS9)

#include <stdlib.h>
#include <process.h>
#include <types.h>
#include <modes.h>
#include <procid.h>
#include <setsys.h>
#include <errno.h>

void vgl_install_isr (short vector, void (*fctn) (int param), int param);

/* We don't like UltraC's version of a NULL */
#undef NULL
#define NULL ((void *)0)

#define USE_VGL_BZERO
#undef  USE_VGL_MALLOC
#define USE_VGL_STRICMP
#undef  USE_VGL_STRCPY
#undef  USE_VGL_STRCAT
#undef  USE_VGL_MEMCPY
#define USE_VGL_QUEUE
#undef  DONT_CHECK_MOUSE
#undef  INLINE_CHECK_MOUSE

#define MAX_RW_ACCESS (  MP_OWNER_READ | MP_OWNER_WRITE | MP_GROUP_READ \
		       | MP_GROUP_WRITE | MP_WORLD_READ | MP_WORLD_WRITE )

#define vgl_board_permit(vgboard)  \
   _os_permit ((vgboard)->addr, (vgboard)->mem_size, MAX_RW_ACCESS, getpid())


/*****************************************************************************/
/*************************  (else) Things for Amiga OS ***********************/
/*****************************************************************************/
#elif defined (AMIGA)

typedef unsigned int size_t;
/* void vgl_install_isr (short vector, void (*fctn) (int param), int param); */
#define USE_VGL_BZERO
#undef USE_VGL_MALLOC
#undef USE_VGL_STRICMP
#undef  USE_VGL_STRCPY
#undef  USE_VGL_STRCAT
#undef  USE_VGL_MEMCPY
#define  DONT_CHECK_MOUSE
#define INLINE_CHECK_MOUSE /* There's a bug in !INLINE_CHECK_MOUSE */



/*****************************************************************************/
/**************  (else) Things for the generic, unspecific OS ****************/
/*****************************************************************************/
#else

typedef unsigned int size_t;
void vgl_install_isr (short vector, void (*fctn) (int param), int param);
#undef USE_VGL_BZERO
#undef USE_VGL_MALLOC
#undef USE_VGL_STRICMP
#undef  USE_VGL_STRCPY
#undef  USE_VGL_STRCAT
#undef  USE_VGL_MEMCPY
#undef  DONT_CHECK_MOUSE
#define INLINE_CHECK_MOUSE

#define vgl_board_permit(vgboard)  (0)

/*****************************************************************************/
/*********************  End of OS specific declarations **********************/
/*****************************************************************************/
#endif



#ifndef NULL
#define NULL	((void *)0)
#endif


#ifndef USE_VGL_MALLOC
#define vgl_malloc(size)  malloc(size)
#define vgl_free(ptr)     free(ptr)
#else
void *vgl_malloc (size_t size);
int vgl_free (void *ptr);
#endif

#ifndef USE_VGL_STRICMP
#define vgl_stricmp(aaa,bbb)  stricmp((aaa),(bbb))
#else
int vgl_stricmp (char *a, char *b);
#endif

#ifndef USE_VGL_STRCPY
#define vgl_strcpy(aaa,bbb)  strcpy((aaa),(bbb))
#else
char *vgl_strcpy (char *a, char *b);
#endif

#ifndef USE_VGL_BZERO
#define vgl_bzero(sss,len)  bzero((sss),(len))
#else
void vgl_bzero (void *s, int len);
#endif

#ifndef  USE_VGL_STRCAT
#define vgl_strcat(aaa,bbb)  strcat((aaa),(bbb))
#else
char *vgl_strcat (char *a, char *b);
#endif

#ifndef USE_VGL_MEMCPY
#define vgl_memcpy(aaa,bbb,ccc)  memcpy((aaa),(bbb),(ccc))
#else
void *vgl_memcpy (void *dest, void *source, size_t size);
#endif

#ifdef USE_VGL_QUEUE
#include "queue.h"
#endif


void *vgl_ext_to_int_address (void *address, size_t size);
unsigned long vgl_mask_irq (VGBOARD *b);
void vgl_unmask_irq (VGBOARD *b, unsigned long);
void vgl_sleep (unsigned long time);
#define	VGL_CLOCK	(1000)


void vgl_mouse_queue_init (VGBOARD *b);
void vgl_mouse_queue_free (VGBOARD *b);
int vgl_mouse_queue_check (VGBOARD *b);
void vgl_mouse_enqueue (VGBOARD *b, int x, int y, int buttons);
int vgl_mouse_dequeue (VGBOARD *b,
		       int *x, int *y, int *buttons,
		       int wait_flag);

void vgl_kb_queue_init (VGBOARD *b);
void vgl_kb_queue_free (VGBOARD *b);
int vgl_kb_queue_check (VGBOARD *b);
void vgl_kb_enqueue (VGBOARD *b, unsigned long key_code);
int vgl_kb_dequeue (VGBOARD *b, unsigned long *key_code, int wait_flag);

void vgl_access_init (VGBOARD *b);
void vgl_access_free (VGBOARD *b);

int vgl_mem_test( void *addr, unsigned long size, char *msg);


#undef	BRESENHAM_LINE
#define	WU_LINE

extern VIDEO_MODE vgl_mode_list[];

#ifndef abs
#define	abs(xx)		(((xx)<0)?-(xx):(xx))
#endif

#ifndef min
#define min(aaa,bbb)	(((aaa)<(bbb))?(aaa):(bbb))
#endif

#ifndef max
#define max(aaa,bbb)	(((aaa)>(bbb))?(aaa):(bbb))
#endif

#ifndef swap
#define swap(aaa,bbb)	{aaa^=bbb;bbb^=aaa;aaa^=bbb;}
#endif

#define	CLIP_OFF	(0)
#define	CLIP_ON		(1)

#define	CLIP_X_MIN	(1<<0)
#define CLIP_X_MAX	(1<<1)
#define CLIP_Y_MIN	(1<<2)
#define CLIP_Y_MAX	(1<<3)
#define gen_outcodes(pixmap,xxx,yyy) \
  ((((xxx) <  (pixmap)->clip_x_min)?CLIP_X_MIN:0) | \
   (((xxx) >= (pixmap)->clip_x_max)?CLIP_X_MAX:0) | \
   (((yyy) <  (pixmap)->clip_y_min)?CLIP_Y_MIN:0) | \
   (((yyy) >= (pixmap)->clip_y_max)?CLIP_Y_MAX:0)     )


/****************************************************************************/
/****************************************************************************/

#if (defined(MMI_150) || defined (MMI_250))
#define tick_ss(vgboard) \
	{ \
	  if( ((vgboard)->ss_flags & SS_DRAW)!=0) \
            { \
	      if( (vgboard)->ss_status == SS_NORMAL) \
	        (vgboard)->ss_counter = (vgboard)->ss_timeout_value; \
	    } \
	}
#else
#define tick_ss(vgboard) \
	{ \
	  if( ((vgboard)->ss_flags & SS_DRAW)!=0) \
	    vgl_ss_tick(vgboard, SS_DRAW); \
	}
#endif


#if defined(DONT_CHECK_MOUSE)

/* Define dummy macros when there is no mouse checking */
#define check_mouse1(pixmap,x1,y1,x2,y2,temp)
#define check_mouse2(pixmap,temp)

#else

/* Do mouse checking...  */
#ifdef INLINE_CHECK_MOUSE
#define	check_mouse1(pixmap,x1,y1,x2,y2,temp) \
	{ \
	    if( (pixmap)->board!=NULL) \
	      { \
		tick_ss( (pixmap)->board); \
		if((pixmap)->mouse_nocheck_flag==0 && \
		   (pixmap)->board->mouse_ptr!=NULL) \
		  { \
		    if( 1 /*(pixmap)->board->mouse_blanking_level==0*/) \
		      { \
			if( (x1) <= (pixmap)->board->mouse_old_x) \
			  { \
			    if( (pixmap)->board->mouse_old_x <= (x2)) \
			      { \
				if( (y1) <= (pixmap)->board->mouse_old_y) \
				  { \
				    if( (pixmap)->board->mouse_old_y <= (y2)) \
				      { \
					vgl_mouse_ptr_off( (pixmap)->board); \
					temp=1; \
				      } \
				    else \
				      temp=0; \
				  } \
				else \
				  { \
				    if( (y1) < \
				       (pixmap)->board->mouse_old_y + \
				       vgl_y_res((pixmap)->board->mouse_ptr)) \
				      { \
					vgl_mouse_ptr_off( (pixmap)->board); \
					temp=1; \
				      } \
				    else \
				      temp=0; \
				  } \
			      } \
			    else \
			      temp=0; \
			  }  \
			else \
			  { \
			    if( (x1) < \
			       (pixmap)->board->mouse_old_x + \
			       vgl_x_res((pixmap)->board->mouse_ptr)) \
			      { \
				if( (y1) <= (pixmap)->board->mouse_old_y) \
				  { \
				    if( (pixmap)->board->mouse_old_y <= (y2)) \
				      { \
					vgl_mouse_ptr_off( (pixmap)->board); \
					temp=1;  \
				      } \
				    else \
				      temp=0; \
				  } \
				else  \
				  {  \
				    if( (y1) < \
				       (pixmap)->board->mouse_old_y + \
				       vgl_y_res((pixmap)->board->mouse_ptr)) \
				      { \
					vgl_mouse_ptr_off( (pixmap)->board); \
					temp=1; \
				      } \
				    else \
				      temp=0; \
				  } \
			      } \
			    else \
			      temp=0; \
			  } \
		      } \
		    else /* Else mouse blanking level>0 */ \
		      temp=0; \
		  } \
		else /* else p->mouse_nocheck_flag is set... */ \
		  temp=0; \
	      } \
	    else  /* else p->board is not valid */ \
	      temp=0; \
	  }

#else  /* Else if mouse checking is done through a function */

/* Define the function prototype and a macro for it's use */
unsigned long check_mouse1_internal (PIXMAP *p, 
				     int x1, int y1,
				     int x2, int y2);
#define	check_mouse1(pixmap,x1,y1,x2,y2,temp)	\
  {temp=check_mouse1_internal(pixmap,x1,y1,x2,y2);}

#endif

/* mouse_check2() is always a macro */
#define check_mouse2(pixmap,temp) \
  { if( temp!=0) vgl_mouse_ptr_on( (pixmap)->board);}

#endif  /* End if !defined(DONT_CHECK_MOUSE)  */


/*****************************************************************************/
/******** Routines for dumb frame buffers and pixmaps in host RAM ************/
/*****************************************************************************/
int vgl_dumb_pixerror (PIXMAP *p);

void vgl_dumb_set_clip (PIXMAP *p, int x_min, int y_min, int x_max, int y_max);
void vgl_dumb_clip_on (PIXMAP *p);
void vgl_dumb_clip_off (PIXMAP *p);

// void vgl_dumb_nothing (PIXMAP *p); // used locally only -> static, AF 20.7.2021

//void vgl_dumb_set_pixel (PIXMAP *p, int x, int y); // used locally only -> static, AF 19.7.2021
int vgl_dumb_get_pixel (PIXMAP *p, int x, int y);
void vgl_dumb_set_pixel_noclip (PIXMAP *p, int x, int y);
int vgl_dumb_get_pixel_noclip (PIXMAP *p, int x, int y);

void vgl_dumb_line (PIXMAP *p, int x1, int y1, int x2, int y2);
void vgl_dumb_hline (PIXMAP *p, int x1, int x2, int y);
void vgl_dumb_vline (PIXMAP *p, int x, int y1, int y2);
void vgl_dumb_line_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);
void vgl_dumb_hline_noclip (PIXMAP *p, int x1, int x2, int y);
void vgl_dumb_vline_noclip (PIXMAP *p, int x, int y1, int y2);

void vgl_dumb_hlinelist (PIXMAP *p, int count, struct span_list *list);

#ifdef VGL_DITHER
void vgl_dumb_hlinelist_noclip (PIXMAP *p, int count, struct span_list *list);
void vgl_dumb_hlinelist_noclip_root (PIXMAP *p, int count, struct span_list *list, double DitherCol);
#else
void vgl_dumb_hlinelist_noclip (PIXMAP *p, int count, struct span_list *list);
#endif

void vgl_dumb_rect (PIXMAP *p, int x1, int y1, int x2, int y2);
//void vgl_dumb_fillrect (PIXMAP *p, int x1, int y1, int x2, int y2);  // used locally only -> static, AF 19.7.2021
// void vgl_dumb_rect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2); // used locally only -> static, AF 20.7.2021
// void vgl_dumb_fillrect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);  // used locally only -> static, AF 19.7.2021

void vgl_dumb_circle (PIXMAP *p, int x, int y, int radius);
void vgl_dumb_fillcircle (PIXMAP *p, int x, int y, int radius);
void vgl_dumb_circle_noclip (PIXMAP *p, int x, int y, int radius);
void vgl_dumb_fillcircle_noclip (PIXMAP *p, int x, int y, int radius);

void vgl_dumb_ellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void vgl_dumb_fillellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void vgl_dumb_ellipse_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void vgl_dumb_fillellipse_noclip (PIXMAP *p,
				  int x, int y,
				  int x_axis, int y_axis);

void vgl_dumb_arc (PIXMAP *p,
		   int x, int y,
		   int x_axis, int y_axis,
		   int start, int end);
void vgl_dumb_fillarc (PIXMAP *p,
		       int x, int y,
		       int x_axis, int y_axis,
		       int start, int end);
/*void vgl_dumb_arc_noclip (PIXMAP *p,
			  int x, int y,
			  int x_axis, int y_axis,
			  int start, int end);*/ // used locally only -> static, AF 20.7.2021
void vgl_dumb_fillarc_noclip (PIXMAP *p,
			      int x, int y,
			      int x_axis, int y_axis,
			      int start, int end);

// void vgl_dumb_poly (PIXMAP *p, int n_vert, struct vgl_coord *vert); // used locally only -> static, AF 30.7.2021
void vgl_dumb_fillpoly (PIXMAP *p, int n_vert, struct vgl_coord *vert);
// void vgl_dumb_poly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert); // used locally only -> static, AF 30.7.2021
void vgl_dumb_fillpoly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert);

void vgl_dumb_bitblt (PIXMAP *source,
		      int sx, int sy,
		      int width, int height,
		      PIXMAP *dest,
		      int dx, int dy);
void vgl_dumb_bitblt_noclip (PIXMAP *source,
			     int sx, int sy, 
			     int width, int height,
			     PIXMAP *dest,
			     int dx, int dy);
void vgl_dumb_bitblt_core (PIXMAP *source,
			   int sx, int sy,
			   int width, int height,
			   PIXMAP *dest,
			   int dx, int dy);
void vgl_dumb_transbitblt (PIXMAP *source,
			   int sx, int sy,
			   int width, int height,
			   PIXMAP *dest,
			   int dx, int dy,
			   int mask_color);
void vgl_dumb_transbitblt_noclip (PIXMAP *source,
				  int sx, int sy,
				  int width, int height,
				  PIXMAP *dest,
				  int dx, int dy,
				  int mask_color);
/*void vgl_dumb_transbitblt_core (PIXMAP *source,
				int sx, int sy,
				int width, int height,
				PIXMAP *dest,
				int dx, int dy,
				int mask_color);*/ // used locally only -> static, AF 20.7.2021

void vgl_dumb_setfont (PIXMAP *p, struct vgl_ffont *ffont);
// void vgl_dumb_text (PIXMAP *p, int x, int y, char *s); // used locally only -> static, AF 30.7.2021
// void vgl_dumb_text2 (PIXMAP *p, int x, int y, char *s, int len); // used locally only -> static, AF 30.7.2021

void vgl_dumb_clear (PIXMAP *p);
void vgl_dumb_dissolve (PIXMAP *dest, PIXMAP *source, int speed);
void vgl_dumb_setcur (PIXMAP *p, int fg, int bg);

int  vgl_dumb_test (PIXMAP *p, char *msg);

void vgl_dumb_quadlist (PIXMAP *p,
			int n_quads,
			struct vgl_quad_def *quads,
			int xoffset, int yoffset);
void vgl_dumb_linelist (PIXMAP *p,
			int n_lines,
			struct vgl_line_def *lines,
			int xoffset, int yoffset);


/*****************************************************************************/
/*****************************************************************************/
#ifdef SUPPORT_MMI_150
void mmi_150_board_init (VGBOARD *b);
void mmi_150_board_kill (VGBOARD *b);
PIXMAP *mmi_150_pixmap_map (VGBOARD *b, int pagenum);
void mmi_150_pixmap_disp (PIXMAP *p);
int mmi_150_boarderror (VGBOARD *b);
void mmi_150_setrgb (VGBOARD *b, int color_num, int n_colors,
		     struct vgl_color *colors);
void mmi_150_getrgb (VGBOARD *b, int color_num, int n_colors,
		     struct vgl_color *colors);
void mmi_150_mouse_getcoords (VGBOARD *b,
			      int *x, int *y, int *buttons,
			      int wait_flag);
void mmi_150_mouse_setcoords (VGBOARD *b, int x, int y);
void mmi_150_mouse_sethandler (VGBOARD *b,
			       void (*fctn) (int x, int y, int buttons));
void mmi_150_mouse_setaccel (VGBOARD *b, int base_speed,
			     int accel_thresh, int accel_num, int accel_den);
void mmi_150_mouse_setpixmap (VGBOARD *b, PIXMAP *p, int hot_x, int hot_y);
void mmi_150_mouse_ptr_on (VGBOARD *b);
void mmi_150_mouse_ptr_off (VGBOARD *b);
int mmi_150_kbhit (VGBOARD *b);
long mmi_150_getkey (VGBOARD *b, int wait_flag);
void mmi_150_kb_setled (VGBOARD *b, unsigned long led_flags);
void mmi_150_ss_set (VGBOARD *b, short status, long timeout, short flags);
void mmi_150_ss_tick (VGBOARD *b, short flags);
int mmi_150_test (VGBOARD *b, char *msg);
#endif

/*****************************************************************************/
/*****************************************************************************/
#ifdef SUPPORT_MMI_250
void mmi_250_board_init (VGBOARD *b);
void mmi_250_board_kill (VGBOARD *b);
PIXMAP *mmi_250_pixmap_map (VGBOARD *b, int pagenum);
void mmi_250_pixmap_disp (PIXMAP *p);
int mmi_250_boarderror (VGBOARD *b);
void mmi_250_setrgb (VGBOARD *b, int color_num, int n_colors,
		     struct vgl_color *colors);
void mmi_250_getrgb (VGBOARD *b, int color_num, int n_colors,
		     struct vgl_color *colors);
void mmi_250_mouse_getcoords (VGBOARD *b,
			      int *x, int *y, int *buttons,
			      int wait_flag);
void mmi_250_mouse_setcoords (VGBOARD *b, int x, int y);
void mmi_250_mouse_sethandler (VGBOARD *b,
			       void (*fctn) (int x, int y, int buttons));
void mmi_250_mouse_setaccel (VGBOARD *b, int base_speed,
			     int accel_thresh, int accel_num, int accel_den);
void mmi_250_mouse_setpixmap (VGBOARD *b, PIXMAP *p, int hot_x, int hot_y);
void mmi_250_mouse_ptr_on (VGBOARD *b);
void mmi_250_mouse_ptr_off (VGBOARD *b);
int mmi_250_kbhit (VGBOARD *b);
long mmi_250_getkey (VGBOARD *b, int wait_flag);
void mmi_250_kb_setled (VGBOARD *b, unsigned long led_flags);
void mmi_250_ss_set (VGBOARD *b, short status, long timeout, short flags);
void mmi_250_ss_tick (VGBOARD *b, short flags);
int mmi_250_test (VGBOARD *b, char *msg);
#endif

/*****************************************************************************/
/************************** DB-180 Specific Functions ************************/
/*****************************************************************************/
#ifdef SUPPORT_DB_180
void db_180_board_init (VGBOARD *b);
void db_180_board_kill (VGBOARD *b);
PIXMAP *db_180_pixmap_map (VGBOARD *b, int pagenum);
void db_180_pixmap_disp (PIXMAP *p);
int db_180_boarderror (VGBOARD *b);
void db_180_setrgb (VGBOARD *b, int color_num, int n_colors,
		    struct vgl_color *colors);
void db_180_getrgb (VGBOARD *b, int color_num, int n_colors, 
		    struct vgl_color *colors);
void db_180_mouse_getcoords (VGBOARD *b, int *x, int *y,
			     int *buttons, int wait_flag);
void db_180_mouse_setcoords (VGBOARD *b, int x, int y);
void db_180_mouse_setaccel (VGBOARD *b, int base_speed, int accel_thresh,
			    int accel_num, int accel_den);
void db_180_mouse_sethandler (VGBOARD *b,
			      void (*fctn) (int x, int y, int buttons));
void db_180_mouse_setpixmap (VGBOARD *b, PIXMAP *p, int hot_x, int hot_y);
void db_180_mouse_ptr_on (VGBOARD *b);
void db_180_mouse_ptr_off (VGBOARD *b);
int db_180_kbhit (VGBOARD *b);
long db_180_getkey (VGBOARD *b, int wait_flag);
void db_180_kb_setled (VGBOARD *b, unsigned long led_flags);
void db_180_ss_set (VGBOARD *b, short status, long timeout, short flags);
void db_180_ss_tick (VGBOARD *b, short flags);

void db_180_req_drawbuffer (PIXMAP *p, int irq_flag);
void db_180_free_drawbuffer (PIXMAP *p);

/*   Drawing functions for the DB-180
 */
int db_180_pixerror (PIXMAP *p);

void db_180_set_clip (PIXMAP *p, int x_min, int y_min, int x_max, int y_max);
void db_180_clip_on (PIXMAP *p);
void db_180_clip_off (PIXMAP *p);

void db_180_set_pixel (PIXMAP *p, int x, int y);
int db_180_get_pixel (PIXMAP *p, int x, int y);
void db_180_set_pixel_noclip (PIXMAP *p, int x, int y);
int db_180_get_pixel_noclip (PIXMAP *p, int x, int y);

void db_180_line (PIXMAP *p, int x1, int y1, int x2, int y2);
void db_180_hline (PIXMAP *p, int x1, int x2, int y);
void db_180_vline (PIXMAP *p, int x, int y1, int y2);
void db_180_line_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);
void db_180_hline_noclip (PIXMAP *p, int x1, int x2, int y);
void db_180_vline_noclip (PIXMAP *p, int x, int y1, int y2);

void db_180_hlinelist (PIXMAP *p, int count, struct span_list *list);
void db_180_hlinelist_noclip (PIXMAP *p, int count, struct span_list *list);

void db_180_rect (PIXMAP *p, int x1, int y1, int x2, int y2);
void db_180_fillrect (PIXMAP *p, int x1, int y1, int x2, int y2);
void db_180_rect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);
void db_180_fillrect_noclip (PIXMAP *p, int x1, int y1, int x2, int y2);

void db_180_circle (PIXMAP *p, int x, int y, int radius);
void db_180_fillcircle (PIXMAP *p, int x, int y, int radius);
void db_180_circle_noclip (PIXMAP *p, int x, int y, int radius);
void db_180_fillcircle_noclip (PIXMAP *p, int x, int y, int radius);

void db_180_ellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void db_180_fillellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void db_180_ellipse_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void db_180_fillellipse_noclip (PIXMAP *p, int x, int y, 
				int x_axis, int y_axis);

void db_180_arc (PIXMAP *p, int x, int y, int x_axis, int y_axis,
		 int start, int end);
void db_180_fillarc (PIXMAP *p, int x, int y, int x_axis, int y_axis,
		     int start, int end);
void db_180_arc_noclip (PIXMAP *p, int x, int y, int x_axis, int y_axis,
			int start, int end);
void db_180_fillarc_noclip (PIXMAP *p, int x, int y, int x_axis,
			    int y_axis, int start, int end);

void db_180_poly (PIXMAP *p, int n_vert, struct vgl_coord *vert);
void db_180_fillpoly (PIXMAP *p, int n_vert, struct vgl_coord *vert);
void db_180_poly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert);
void db_180_fillpoly_noclip (PIXMAP *p, int n_vert, struct vgl_coord *vert);

void db_180_bitblt (PIXMAP *source,
		    int sx, int sy, 
		    int width, int height,
		    PIXMAP *dest,
		    int dx, int dy);

void db_180_bitblt_noclip (PIXMAP *source, 
			   int sx, int sy, 
			   int width, int height, 
			   PIXMAP *dest, 
			   int dx, int dy);

void db_180_bitblt_core (PIXMAP *source, 
			 int sx, int sy, 
			 int width, int height, 
			 PIXMAP *dest, 
			 int dx, int dy);

void db_180_transbitblt (PIXMAP *source, 
			 int sx, int sy, 
			 int width, int height, 
			 PIXMAP *dest, 
			 int dx, int dy, 
			 int mask_color);

void db_180_transbitblt_noclip (PIXMAP *source,
				int sx, int sy, 
				int width, int height, 
				PIXMAP *dest, 
				int dx, int dy, 
				int mask_color);

void db_180_transbitblt_core (PIXMAP *source, 
			      int sx, int sy, 
			      int width, int height, 
			      PIXMAP *dest, 
			      int dx, int dy, 
			      int mask_color);

void db_180_setfont (PIXMAP *p, struct vgl_ffont *ffont);
void db_180_text (PIXMAP *p, int x, int y, char *s);
void db_180_text2 (PIXMAP *p, int x, int y, char *s, int len);

void db_180_clear (PIXMAP *p);
void db_180_dissolve (PIXMAP *dest, PIXMAP *source, int speed);
void db_180_setcur (PIXMAP *p, int fg, int bg);

int  db_180_pixmap_test (PIXMAP *p, char *msg);
int  db_180_test (VGBOARD *b, char *msg);

void db_180_quadlist (PIXMAP *p,
		      int n_quads,
		      struct vgl_quad_def *quads,
		      int xoffset, int yoffset);
void db_180_linelist (PIXMAP *p,
		      int n_lines,
		      struct vgl_line_def *lines,
		      int xoffset, int yoffset);


#endif

/*****************************************************************************/
/************************ GDM-9000 Specific Functions ************************/
/*****************************************************************************/
#ifdef SUPPORT_GDM_9000
void gdm_9000_board_init (VGBOARD *b);
void gdm_9000_board_kill (VGBOARD *b);
PIXMAP *gdm_9000_pixmap_map (VGBOARD *b, int pagenum);
void gdm_9000_pixmap_disp (PIXMAP *p);
int gdm_9000_boarderror (VGBOARD *b);
void gdm_9000_setrgb (VGBOARD *b, int color_num, int n_colors,
		      struct vgl_color *colors);
void gdm_9000_getrgb (VGBOARD *b, int color_num, int n_colors, 
		      struct vgl_color *colors);
void gdm_9000_mouse_getcoords (VGBOARD *b, int *x, int *y,
			       int *buttons, int wait_flag);
void gdm_9000_mouse_setcoords (VGBOARD *b, int x, int y);
void gdm_9000_mouse_setaccel (VGBOARD *b, int base_speed, int accel_thresh,
			      int accel_num, int accel_den);
void gdm_9000_mouse_sethandler (VGBOARD *b,
				void (*fctn) (int x, int y, int buttons));
void gdm_9000_mouse_setpixmap (VGBOARD *b, PIXMAP *p, int hot_x, int hot_y);
void gdm_9000_mouse_ptr_on (VGBOARD *b);
void gdm_9000_mouse_ptr_off (VGBOARD *b);
int gdm_9000_kbhit (VGBOARD *b);
long gdm_9000_getkey (VGBOARD *b, int wait_flag);
void gdm_9000_kb_setled (VGBOARD *b, unsigned long led_flags);
void gdm_9000_ss_set (VGBOARD *b, short status, long timeout, short flags);
void gdm_9000_ss_tick (VGBOARD *b, short flags);

void gdm_9000_req_drawbuffer (PIXMAP *p, int irq_flag);
void gdm_9000_free_drawbuffer (PIXMAP *p);

/*   Drawing functions for the DB-180
 */
int gdm_9000_pixerror (PIXMAP *p);

void gdm_9000_set_clip (PIXMAP *p, int x_min, int y_min, int x_max, int y_max);
void gdm_9000_clip_on (PIXMAP *p);
void gdm_9000_clip_off (PIXMAP *p);

void gdm_9000_set_pixel (PIXMAP *p, int x, int y);
int gdm_9000_get_pixel (PIXMAP *p, int x, int y);

void gdm_9000_line (PIXMAP *p, int x1, int y1, int x2, int y2);
void gdm_9000_hline (PIXMAP *p, int x1, int x2, int y);
void gdm_9000_vline (PIXMAP *p, int x, int y1, int y2);

void gdm_9000_hlinelist (PIXMAP *p, int count, struct span_list *list);

void gdm_9000_rect (PIXMAP *p, int x1, int y1, int x2, int y2);
void gdm_9000_fillrect (PIXMAP *p, int x1, int y1, int x2, int y2);

void gdm_9000_circle (PIXMAP *p, int x, int y, int radius);
void gdm_9000_fillcircle (PIXMAP *p, int x, int y, int radius);

void gdm_9000_ellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);
void gdm_9000_fillellipse (PIXMAP *p, int x, int y, int x_axis, int y_axis);

void gdm_9000_arc (PIXMAP *p, int x, int y, int x_axis, int y_axis,
		   int start, int end);
void gdm_9000_fillarc (PIXMAP *p, int x, int y, int x_axis, int y_axis,
		       int start, int end);

void gdm_9000_poly (PIXMAP *p, int n_vert, struct vgl_coord *vert);
void gdm_9000_fillpoly (PIXMAP *p, int n_vert, struct vgl_coord *vert);

void gdm_9000_bitblt (PIXMAP *source,
		      int sx, int sy, 
		      int width, int height,
		      PIXMAP *dest,
		      int dx, int dy);

void gdm_9000_bitblt_noclip (PIXMAP *source,
			     int sx, int sy, 
			     int width, int height,
			     PIXMAP *dest,
			     int dx, int dy);

void gdm_9000_transbitblt (PIXMAP *source, 
			   int sx, int sy, 
			   int width, int height, 
			   PIXMAP *dest, 
			   int dx, int dy, 
			   int mask_color);

void gdm_9000_transbitblt_noclip (PIXMAP *source,
				  int sx, int sy, 
				  int width, int height, 
				  PIXMAP *dest, 
				  int dx, int dy, 
				  int mask_color);

void gdm_9000_transbitblt_core (PIXMAP *source, 
				int sx, int sy, 
				int width, int height, 
				PIXMAP *dest, 
				int dx, int dy, 
				int mask_color);

void gdm_9000_setfont (PIXMAP *p, struct vgl_ffont *ffont);
void gdm_9000_text (PIXMAP *p, int x, int y, char *s);
void gdm_9000_text2 (PIXMAP *p, int x, int y, char *s, int len);

void gdm_9000_clear (PIXMAP *p);
void gdm_9000_dissolve (PIXMAP *dest, PIXMAP *source, int speed);
void gdm_9000_setcur (PIXMAP *p, int fg, int bg);

int  gdm_9000_pixmap_test (PIXMAP *p, char *msg);
int  gdm_9000_test (VGBOARD *b, char *msg);

void gdm_9000_quadlist (PIXMAP *p,
			int n_quads,
			struct vgl_quad_def *quads,
			int xoffset, int yoffset);
void gdm_9000_linelist (PIXMAP *p,
			int n_lines,
			struct vgl_line_def *lines,
			int xoffset, int yoffset);
#endif

/*****************************************************************************/
/*****************************************************************************/

#endif

