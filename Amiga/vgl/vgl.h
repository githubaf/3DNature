
#ifndef	_VGL_H_
#define _VGL_H_

#ifdef AMIGA
#include <stdlib.h>   /* ALEXANDER */
/* Rounds number a up to the next multiple of b */
#ifndef ROUNDUP
#define ROUNDUP(a,b)		(((a + (b - 1)) / b) * b)
#endif

#define VGL_DITHER
#define VGL_DITHERTABLE_SIZE 4096 /* Nice, even number */

#endif /* AMIGA */

/******  Things Specific to PSOS ******/
#if defined(PSOS)

#include <psos.h>
#include <limits.h>
typedef unsigned long QUEUE_ID;
typedef unsigned long SEMAPHORE_ID;
typedef unsigned long ulong;

#define vgl_access_p(vgboard) sm_p((vgboard)->access_semaphore, SM_WAIT, 0)
#define vgl_access_v(vgboard) sm_v((vgboard)->access_semaphore)



/******  (else) Things Specific to VX WORKS *****/
#elif defined (VXWORKS)

#include <vxWorks.h>
#include <msgQLib.h>
#include <limits.h>

typedef MSG_Q_ID QUEUE_ID;
typedef SEM_ID   SEMAPHORE_ID;
typedef unsigned long	ulong;


/******  (else) Things for OS/9 ******/
#elif defined (OS9)

#include <stdlib.h>
#include <limits.h>
#include "queue.h"
typedef unsigned long	ulong;
typedef VGL_Q *QUEUE_ID;

/******  (else) Things for the generic, unspecific OS ******/
#else

#include <limits.h>
typedef unsigned long	ulong;
typedef unsigned long QUEUE_ID;
typedef unsigned long SEMAPHORE_ID;

/******  End of OS specific declarations *****/
#endif


struct vgl_color
  {
    short red, green, blue;
  };



struct vgl_coord
  {
    int x, y;
  };


#define vgl_pack_xy(xxx,yyy)   ((((xxx)<<16)&0xFFFF0000) | ((yyy)&0x0000FFFF))

struct vgl_quad_def
  {
    int color;
    int p1, p2, p3, p4;
  };

#define vgl_set_quad(quad,x1,y1,x2,y2,x3,y3,x4,y4,ccc) \
  { \
      (quad)->p1 = vgl_pack_xy(x1,y1); \
      (quad)->p2 = vgl_pack_xy(x2,y2); \
      (quad)->p3 = vgl_pack_xy(x3,y3); \
      (quad)->p4 = vgl_pack_xy(x4,y4); \
      (quad)->color = (ccc);         \
  }

#define vgl_set_tri(quad,x1,y1,x2,y2,x3,y3,ccc) \
  { \
      (quad)->p1 = vgl_pack_xy(x1,y1); \
      (quad)->p2 = vgl_pack_xy(x2,y2); \
      (quad)->p3 = (quad)->p4 = vgl_pack_xy(x3,y3); \
      (quad)->color = (ccc);         \
  }

struct vgl_line_def
  {
    int color;
    int p1, p2;
  };

#define vgl_set_line(line,x1,y1,x2,y2,color) \
  { \
      (line)->p1 = vgl_pack_xy(x1,y1); \
      (line)->p2 = vgl_pack_xy(x2,y2); \
      (line)->color = (color);         \
  }

#define	SPAN_INC	(INT_MIN)
#define SPAN_SAME	(INT_MIN+1)

struct span_list
  {
    int x1, x2;
    int y;	/* -1 = Same as prev, -2 if inc'd from prev value */
  };



struct vgl_font
  {
    short width, height;	/* Width and height in pixels of font. */
    unsigned char *bitmaps;	/* Bitmap data of the font. */
    short min_char, max_char;	/* Minimum and maximum character. */
    unsigned short *offsets;	/* Offset table for each character. */
    char line_length;		/* Bytes per scanline in the font. */
  };


/* A fast font is less compact in memory, but much faster to draw.
   The font must be 32 pixels or less wide. */
struct vgl_ffont
  {
    short width, height;	/* Width and height in pixels of font. */
    unsigned long *bitmaps;	/* Bitmap data of the font. */
    short min_char, max_char;	/* Minimum and maximum character. */
    unsigned short *offsets;	/* Offset table for each character. */
  };

struct vgl_ffont *vgl_expand_font (struct vgl_font *input);


#define	MOUSE_BUF_SIZE	(32)

struct mouse_buffer
  {
    unsigned short in_start, in_end;
    unsigned char in[MOUSE_BUF_SIZE];
    unsigned char err_in[MOUSE_BUF_SIZE];
  };


/*****************************************************************************/
/**************************  STRUCT for PIXMAP *******************************/
/*****************************************************************************/
#define	PIXMAP_FREE_DATA	(1<<0)
#define	PIXMAP_FREE_STRUCT	(1<<1)

struct vgl_pixdata
  {
    int x_res, y_res, x_size, users, free_flag;
    void *data;
  };

typedef struct vgl_pixmap
  {
    struct vgl_pixdata *pixdata;

    int error;

    struct vgl_vgboard *board;
    struct vgl_pixmap *mouse_pixmap;

    int clip_setting;
    int clip_x_min, clip_y_min;
    int clip_x_max, clip_y_max;

    int foreground, background;
    unsigned long fore_8, back_8;
    unsigned long expand_8[16];
    struct vgl_ffont *font;

    /*   Workspace is an area of RAM set aside for temp data, used for drawing
     *   without the normal overhead of malloc-ing and freeing.
     */
    void *workspace;
    unsigned long worksize;
    
    #ifdef VGL_DITHER
    /* DitherTable is like workspace, but is filled with random numbers upon
    ** creation. This allows the dithering routine to quickly fetch a pseudo-
    ** random number for pixel-dithering, incurring only the overhead of a
    ** table-lookup, rather than a full-blown function call to drand(). This
    ** makes the dithering less random, but random enough for most uses. Use
    ** larger table sizes (VGL_DITHERTABLE_SIZE) for larger dithered polys.
    ** DTableIdx is used for keeping track of the current index into the
    ** DitherTable, and is always used mod DTableSize to wrap around.
    */
    
    unsigned char *DitherTable;
    unsigned long DTableSize, DTableIdx;
    
    #endif /* VGL_DITHER */

    /* Set when the mouse ptr check should not be done */
    int mouse_nocheck_flag;

    /*   An unsigned long integer used for implimentation defined things.
     *   Unused on the MMI-150/250.
     *   Buffer number on the DB-180.
     *   Unused on host RAM pixmaps.
     */
    unsigned long data1;

    /* Graphic Primitive functions and relavent data */
    void (*set_clip) (struct vgl_pixmap *p, int x1, int y1, int x2, int y2);
    void (*clip_off) (struct vgl_pixmap *p);
    void (*clip_on) (struct vgl_pixmap *p);

    int (*pixerror) (struct vgl_pixmap *p);

    void (*set_pixel) (struct vgl_pixmap *p, int x, int y);
    int (*get_pixel) (struct vgl_pixmap *p, int x, int y);

    void (*line) (struct vgl_pixmap *p, int x1, int y1, int x2, int y2);
    void (*hline) (struct vgl_pixmap *p, int x1, int x2, int y);
    void (*vline) (struct vgl_pixmap *p, int x, int y1, int y2);

    void (*rect) (struct vgl_pixmap *p, int x1, int y1, int x2, int y2);
    void (*fillrect) (struct vgl_pixmap *p, int x1, int y1, int x2, int y2);

    void (*circle) (struct vgl_pixmap *p, int x, int y, int radius);
    void (*fillcircle) (struct vgl_pixmap *p, int x, int y, int radius);

    void (*ellipse) (struct vgl_pixmap *p,
		     int x, int y, int x_axis, int y_axis);
    void (*fillellipse) (struct vgl_pixmap *p,
			 int x, int y, int x_axis, int y_axis);

    void (*arc) (struct vgl_pixmap *p,
		 int x, int y, int x_axis, int y_axis, int start, int end);
    void (*fillarc) (struct vgl_pixmap *p,
		     int x, int y, int x_axis, int y_axis, int start, int end);

    void (*poly) (struct vgl_pixmap *p,
		  int n_vert, struct vgl_coord * vert);
    void (*fillpoly) (struct vgl_pixmap *p,
		      int n_vert, struct vgl_coord * vert);

    void (*bitblt) (struct vgl_pixmap *source, int sx, int sy,
		    int width, int height,
		    struct vgl_pixmap *dest, int dx, int dy);
    void (*transbitblt) (struct vgl_pixmap *source, int sx, int sy,
			 int width, int height,
			 struct vgl_pixmap *dest, int dx, int dy,
			 int mask_color);

    void (*setfont) (struct vgl_pixmap *p, struct vgl_ffont * ffont);
    void (*text) (struct vgl_pixmap *p, int x, int y, char *s);
    void (*text2) (struct vgl_pixmap *p, int x, int y, char *s, int len);

    void (*clear) (struct vgl_pixmap *p);
    void (*dissolve) (struct vgl_pixmap *dest,
		      struct vgl_pixmap *source,
		      int speed);

    void (*setcur) (struct vgl_pixmap *p, int forground, int background);

    int  (*test) (struct vgl_pixmap *p, char *msg);

    void (*nothing) (struct vgl_pixmap *p);

    void (*quadlist)(struct vgl_pixmap *p,
		     int n_quads, 
		     struct vgl_quad_def *q, 
		     int xoffset, int yoffset);
    void (*linelist)(struct vgl_pixmap *p,
		     int n_lines, 
		     struct vgl_line_def *l,
		     int xoffset, int yoffset);
    void (*hlinelist)(struct vgl_pixmap *p,
		      int count,
		      struct span_list *list);
  } PIXMAP;


/*****************************************************************************/
/**********************  Structures for video paramaters  ********************/
/*****************************************************************************/
struct mmi_150
  {
    unsigned long hs, bp, hd, sd, br, vs, vb, vd, lt, ls, mi, td, cr, bl;
    unsigned long x_size, page_size;
    int sync_pol;
  };

struct mmi_250
  {
    unsigned long hs, bp, hd, sd, br, vs, vb, vd, lt, ls, mi, td, cr, bl;
    unsigned long x_size, page_size;
    int hs_pol, vs_pol;
  };

struct db_180
  {
    /* System Control Regs */
    ulong	sysconfig;

    /* Video Control Regs */
    ulong	hrzt;
    ulong	hrzsr;
    ulong	hrzbr;
    ulong	hrzbf;
    ulong	prehrzc;
    ulong	vrtt;
    ulong	vrtsr;
    ulong	vrtbr;
    ulong	vrtbf;
    ulong       prevrtc;
    ulong	srtctl;

    /* VRAM Control Regs */
    ulong	mem_config;
    ulong	rfperiod;
    ulong	rlmax;

    unsigned long	x_size, page_size;

    long	bt445_ctrl[16];
  };

struct gdm_9000
  {
    /* System Control Regs */
    ulong	sysconfig;

    /* Video Control Regs */
    ulong	hrzt;
    ulong	hrzsr;
    ulong	hrzbr;
    ulong	hrzbf;
    ulong	prehrzc;
    ulong	vrtt;
    ulong	vrtsr;
    ulong	vrtbr;
    ulong	vrtbf;
    ulong       prevrtc;
    ulong	srtctl;

    /* VRAM Control Regs */
    ulong	mem_config;
    ulong	rfperiod;
    ulong	rlmax;

    unsigned long	x_size, page_size;

    long	bt445_ctrl[16];
  };

#define VGL_NAME_LEN  (32)


typedef struct
  {
    char name[VGL_NAME_LEN];
    int x_res, y_res;
    struct mmi_150 mmi_150;
    struct mmi_250 mmi_250;
    struct db_180  db_180;
    struct gdm_9000 gdm_9000;
/*
    struct vgs_882 vgs_882;
*/
  } VIDEO_MODE;


/* These are in the file boards.c */
struct board_config
  {
    char  name[VGL_NAME_LEN];
    short type;
    void  *addr;
    int   irq_level;
    int   irq_vector;
  };

extern struct board_config board_config[];


/*****************************************************************************/
/*****************************  Struct for VGBOARD  **************************/
/*****************************************************************************/
enum { MMI_150 = 0, MMI_250, DB_180, GDM_9000, VGS_882};

#define SS_EXTERNAL	(1<<0)
#define SS_DRAW		(1<<1)
#define SS_MOUSE	(1<<2)
#define SS_KEYBOARD	(1<<3)
#define	SS_ALL		(0xFF)

enum VGL_SS_STATUS
  {
    SS_NORMAL, SS_ON, SS_OFF
  };

#define KB_BUF_OUT_SIZE	(16)
#define L_SHIFT		(1<<14)
#define R_SHIFT		(1<<15)
#define L_CTRL		(1<<18)
#define R_CTRL		(1<<19)
#define L_ALT		(1<<22)
#define R_ALT		(1<<23)
#define NUM_LOCK	(1<<26)
#define CAPS_LOCK	(1<<27)
#define SCRL_LOCK	(1<<28)
#define KEY_UP		(1<<31)

#define	MOUSE_LEFT	(0x01)
#define	MOUSE_MID	(0x02)
#define	MOUSE_RIGHT	(0x04)



typedef struct vgl_vgboard
  {
    char name[VGL_NAME_LEN];
    int  num;
    short type;
    void *addr, *regaddr;
    long mem_size;
    short irq_level, irq_vector;
    VIDEO_MODE *mode;

    long mouse_x, mouse_y, mouse_buttons;
    long mouse_max_x, mouse_max_y;
    int mouse_accel_base, mouse_accel_thresh, mouse_accel_num, mouse_accel_den;
    int mouse_middle, mouse_sync, mouse_oa;


    void (*board_init) (struct vgl_vgboard *b);
    void (*board_kill) (struct vgl_vgboard *b);
    PIXMAP *(*pixmap_map) (struct vgl_vgboard *b, int pagenum);
    void (*pixmap_disp) (PIXMAP *p);

    int (*boarderror) (struct vgl_vgboard *b);

    void (*setrgb) (struct vgl_vgboard *b,
		    int color_num, int n_colors, struct vgl_color * colors);
    void (*getrgb) (struct vgl_vgboard *b,
		    int color_num, int n_colors, struct vgl_color * colors);

    void (*mouse_getcoords) (struct vgl_vgboard *b,
			     int *x, int *y, int *buttons, int wait_flag);
    void (*mouse_setcoords) (struct vgl_vgboard *b, int x, int y);
    void (*mouse_sethandler) (struct vgl_vgboard *b,
			      void (*fctn) (int x, int y, int buttons));
    void (*mouse_setaccel) (struct vgl_vgboard *b,
			    int base_speed, int accel_thresh,
			    int accel_num, int accel_den);
    void (*mouse_setpixmap) (struct vgl_vgboard *b, PIXMAP *pointer,
			     int hot_x, int hot_y);
    void (*mouse_ptr_on) (struct vgl_vgboard *b);
    void (*mouse_ptr_off) (struct vgl_vgboard *b);

    int (*kbhit) (struct vgl_vgboard *b);
    long (*getkey) (struct vgl_vgboard *b, int wait_flag);
    void (*kb_setled) (struct vgl_vgboard *b, unsigned long led_flags);

    void (*ss_set) (struct vgl_vgboard *b,
		    short status, long timeout, short flags);
    void (*ss_tick) (struct vgl_vgboard *b, short flags);

    int (*test) (struct vgl_vgboard *b, char *msg);

    /* Structure members that would be private, if this were C++ */
    short color_start, color_end;
    long color_table[256];

    int tos_reset;
    long tos_value;

    unsigned long frame_count;

    short error;
    struct mouse_buffer mouse_buf;
    QUEUE_ID mouse_queue_id;

    /* The currently displayed pixmap, and the next one */
    PIXMAP *cur_pixmap, *next_cur_pixmap;
    /* The pixmap for the ptr and what's under */
    PIXMAP *mouse_ptr, *mouse_under;
    int mouse_hot_x, mouse_hot_y;  /* The x and y coords for the hot spot   */
    int mouse_cur_x, mouse_cur_y;  /* Current coords for PIXMAP mouse_ptr   */
    int mouse_old_x, mouse_old_y;  /* The last coords of PIXMAP mouse_ptr   */
    int mouse_update_flag;         /* Set when mouse ptr needs redrawing    */
    int mouse_blanking_level;      /* 0=Mouse ptr visible, >0 = not visible */
    int mouse_visible_flag;        /* Set when the mouse ptr is visible     */

    /* Screen saver stuff */
    long ss_counter;	     /* Normal when ss_counter>0, blanked otherwise */
    long ss_timeout_value;   /* # of frames for ss timeout                  */
    short ss_status;	     /* On, off, or running                         */
    short ss_flags;	     /* Which 'things' affect the screen saver      */
    short ss_istatus;	     /* Internal Stat.  0 = screen off, 1 = On      */

    /* Keyboard Stuff */
    QUEUE_ID kb_queue_id;
    int kb_start, kb_end;
    unsigned char kb_buf_out[KB_BUF_OUT_SIZE];
    int kb_start_out, kb_end_out;
    unsigned long kb_state;

    /* Buffer-blocking stuff */
    /* This stuff is only used on the DB-180 and GDM-9000. */
    unsigned int cur_draw_buffer;
    unsigned int draw_irq_flag;
    unsigned int n_users;

    /* This is a general use pointer.
     * It holds the module address on OS/9 (application side of things).
     * It is unused under pSOS+ and VxWorks.
     */
    void *pointer1;

    /* This flag is used under OS9 to indicate if IRQ's should be disabled. */
    int  irq_masked;

    /* This is an exclusion semaphore to control access to the P9000 and etc */
    /* It is unused on the MMI-150 and MMI-250 */
    SEMAPHORE_ID access_semaphore;
    PIXMAP       *cur_draw_context;
  } VGBOARD;



/*****************************************************************************/
/************  Stuff that does not require a pixmap or vgboard  **************/
/*****************************************************************************/
VGBOARD *vgl_initboard (char *board_name, char *video_mode);
PIXMAP *vgl_makepixmap (int x_res, int y_res);
PIXMAP *vgl_dupepixmap (PIXMAP *pixmap);
void vgl_copypixmap (PIXMAP *dest, PIXMAP *source);
void vgl_freepixmap (PIXMAP *p);

#define vgl_x_res(pixmap)	((pixmap)->pixdata->x_res)
#define vgl_y_res(pixmap)	((pixmap)->pixdata->y_res)

#define vgl_font_height(pixmap) ((pixmap)->font->height)
#define vgl_font_width(pixmap)  ((pixmap)->font->width)




/*****************************************************************************/
/**************  Functions and definitions in a 'useable' form  **************/
/*****************************************************************************/
/************** Board Level ( & Misc.) **************/
#define	vgl_pixmap_map(vgboard,pagenum) \
  ((*(vgboard)->pixmap_map)( vgboard, pagenum))
#define vgl_pixmap_disp(pixmap) ((*((pixmap)->board->pixmap_disp))(pixmap))

#define vgl_pixerror(pixmap)	((*(pixmap)->pixerror)(pixmap))
#define vgl_boarderror(vgboard) ((*(vgboard)->boarderror)(vgboard))

#define	vgl_killboard(vgboard)	((*(vgboard)->board_kill)( vgboard))

#define vgl_fcount(vgboard)	((vgboard)->frame_count)
#define vgl_board_test(vgboard,msg)	((*(vgboard)->test) (vgboard, msg))

/************** Color Mapping **************/
#define	vgl_setrgb(VGBOARD,color_num,N_COLORS,COLORS)	\
  ((*(VGBOARD)->setrgb)( VGBOARD, color_num, N_COLORS, COLORS))
#define	vgl_getrgb(vgboard,color_num,n_colors,colors)	\
  ((*(vgboard)->getrgb)( vgboard, color_num, n_colors, color))


/************** Graphic Primitives **************/
#define vgl_set_clip(pixmap,x1,y1,x2,y2) \
  ((*(pixmap)->set_clip)( pixmap, x1, y1, x2, y2))
#define vgl_clip_off(pixmap)	        ((*(pixmap)->clip_off)( pixmap))
#define vgl_clip_on(pixmap)		((*(pixmap)->clip_on)( pixmap))

#define vgl_nothing(pixmap)             ((*(pixmap)->nothing)(pixmap))

#define vgl_set_pixel(pixmap,xxx,yyy) ((*(pixmap)->set_pixel)(pixmap,xxx,yyy))
#define vgl_get_pixel(pixmap,xxx,yyy) ((*(pixmap)->get_pixel)(pixmap,xxx,yyy))

#define vgl_line(pixmap,x1,y1,x2,y2) ((*(pixmap)->line)(pixmap,x1,y1,x2,y2))
#define vgl_hline(pixmap,x1,x2,yyy)  ((*(pixmap)->hline)(pixmap,x1,x2,yyy))
#define vgl_vline(pixmap,xxx,y1,y2)  ((*(pixmap)->vline)(pixmap,xxx,y1,y2))

#define vgl_rect(pixmap,x1,y1,x2,y2) ((*(pixmap)->rect)(pixmap,x1,y1,x2,y2))
#define vgl_fillrect(pixmap,x1,y1,x2,y2) \
  ((*(pixmap)->fillrect)(pixmap, x1, y1, x2, y2))

#define vgl_circle(pixmap,xxx,yyy,radius) \
  ((*(pixmap)->circle)( pixmap, xxx, yyy, radius))
#define vgl_fillcircle(pixmap,xxx,yyy,radius) \
  ((*(pixmap)->fillcircle)( pixmap, xxx, yyy, radius))

#define vgl_ellipse(pixmap,xxx,yyy,x_axis,y_axis)  \
  ((*(pixmap)->ellipse)( pixmap, xxx, yyy, x_axis, y_axis))
#define vgl_fillellipse(pixmap,xxx,yyy,x_axis,y_axis)	\
  ((*(pixmap)->fillellipse)( pixmap, xxx, yyy, x_axis, y_axis))

#define vgl_arc(pixmap,xxx,yyy,x_axis,y_axis,start,end)		\
  ((*(pixmap)->arc)( pixmap, xxx, yyy, x_axis, y_axis, start, end))
#define vgl_fillarc(pixmap,xxx,yyy,x_axis,y_axis,start,end)	\
  ((*(pixmap)->fillarc)( pixmap, xxx, yyy, x_axis, y_axis, start, end))

#define vgl_poly(pixmap,n_vert,vertexes) \
  ((*(pixmap)->poly)( pixmap, n_vert, vertexes))
#define vgl_fillpoly(pixmap,n_vert,vertexes) \
  ((*(pixmap)->fillpoly)( pixmap, n_vert, vertexes))

#define vgl_bitblt(source,sx,sy,width,height,dest,dx,dy) \
  ((*((dest)->bitblt))( source, sx, sy, width, height, dest, dx, dy))
#define vgl_transbitblt(source,sx,sy,width,height,dest,dx,dy,mask)	\
  ((*(dest)->transbitblt)( source, sx, sy, width, height, dest, dx, dy, mask))

#define vgl_setfont(pixmap,ffont) \
  ((*(pixmap)->setfont)( pixmap, ffont))
#define vgl_text(pixmap,xxx,yyy,sss) \
  ((*(pixmap)->text)( pixmap, xxx, yyy, sss))
#define vgl_text2(pixmap,xxx,yyy,len,sss) \
  ((*(pixmap)->text2)( pixmap, xxx, yyy, sss, len))

#define vgl_clear(pixmap)			((*(pixmap)->clear)( pixmap))
#define vgl_dissolve(dest,source,speed)	\
  ((*(dest)->dissolve)( dest, source, speed))

#define vgl_setcur(pixmap,fg,bg)      ((*(pixmap)->setcur)( pixmap, fg, bg))

#define vgl_pixmap_test(pixmap,msg)   ((*(pixmap)->test)( pixmap, msg))

#define vgl_quadlist(pixmap,n_quads,quads,xoffset,yoffset)   ((*(pixmap)->quadlist) (pixmap, n_quads, quads, xoffset, yoffset))
#define vgl_linelist(pixmap,n_lines,lines,xoffset,yoffset)   ((*(pixmap)->linelist) (pixmap, n_lines, lines, xoffset, yoffset))
#define vgl_hlinelist(pixmap,count,spans)  ((*(pixmap)->hlinelist) (pixmap, count, spans))

/************** Mouse **************/
#define vgl_mouse_getcoords(vgboard,xxx,yyy,buttons,wait_flag)	\
  ((*(vgboard)->mouse_getcoords)( vgboard, xxx, yyy, buttons, wait_flag))
#define vgl_mouse_setcoords(vgboard,xxx,yyy) \
  ((*(vgboard)->mouse_setcoords)( vgboard, xxx, yyy))
#define vgl_mouse_setaccel(vgboard,base,accel_thresh,accel_num,accel_den) \
  ((*((vgboard)->mouse_setaccel))(vgboard, base, accel_thresh, \
				  accel_num, accel_den))

#define vgl_mouse_sethandler(vgboard,fctn) \
  ((*((vgboard)->mouse_sethandler))( vgboard, fctn))
#define vgl_mouse_setpixmap(vgboard,pixmap,hot_x,hot_y)	\
  ((*((vgboard)->mouse_setpixmap))( vgboard, pixmap, hot_x, hot_y))
#define vgl_mouse_ptr_on(vgboard)    ((*(vgboard)->mouse_ptr_on)( vgboard))
#define vgl_mouse_ptr_off(vgboard)   ((*(vgboard)->mouse_ptr_off)( vgboard))


/************** Keyboard **************/
#define vgl_kbhit(vgboard) \
  ((*(vgboard)->kbhit)(vgboard))
#define vgl_getkey(vgboard,wait_flag) \
  ((*(vgboard)->getkey)( vgboard, wait_flag))
#define vgl_kb_setled(vgboard,led_flags) \
  ((*(vgboard)->kb_setled)( vgboard, led_flags))


/************** Screen Saver ***********/
#define vgl_ss_set(vgboard,status,timeout,flags) \
  ((*((vgboard)->ss_set))( vgboard, status, timeout, flags))
#define vgl_ss_tick(vgboard,flags) \
  ((*((vgboard)->ss_tick))( vgboard, flags))


/************* Color Conversion Functions and other color things ************/
void vgl_setcur_rgb (PIXMAP *p,
		     int fg_red, int fg_green, int fg_blue,
		     int bg_red, int bg_green, int bg_blue);

void vgl_rgb_to_hsv (double red, double green, double blue, 
		     double *hue, 
		     double *saturation, 
		     double *value);

void vgl_hsv_to_rgb (double hue,
		     double saturation,
		     double value,
		     double *red, double *green, double *blue);

void vgl_rgb_to_hls (double red, double green, double blue,
		     double *hue,
		     double *lightness,
		     double *saturation);

void vgl_hls_to_rgb (double hue,
		     double lightness,
		     double saturation,
		     double *red, double *green, double *blue);

void vgl_rgb_to_cmy (double red, double green, double blue,
		     double *cyan, 
		     double *magenta, 
		     double *yellow);

void vgl_cmy_to_rgb (double cyan, 
		     double magenta, 
		     double yellow, 
		     double *red, double *green, double *blue);

void vgl_rgb_to_cmyk (double red, double green, double blue,
		      double *cyan, 
		      double *magenta, 
		      double *yellow, 
		      double *black);

void vgl_cmyk_to_rgb (double cyan, 
		      double magenta, 
		      double yellow, 
		      double black, 
		      double *red, double *green, double *blue);

void vgl_rgb_to_yiq (double red, double green, double blue,
		     double *y,
		     double *i,
		     double *q);

void vgl_yiq_to_rgb (double y,
		     double i,
		     double q,
		     double *red, double *green, double *blue);

/********************* Misc useful functions *************************/
unsigned long vgl_isqrt (unsigned long n);
unsigned long vgl_ihypot (unsigned long x, unsigned long y);
float         vgl_random (long *idum);

#define RANDQ_A  (1664525)
#define RANDQ_C  (1013904223)
extern unsigned long _vgl_rand_last;
#define vgl_random_quick(highest) \
       ((_vgl_rand_last=(RANDQ_A * _vgl_rand_last + RANDQ_C))%(highest))
#define vgl_random_quick_seed(seed)  (_vgl_rand_last=seed)


/************** Error Handling Functions and Constants ***************/
enum vgl_error
  {
    VGL_ERR_NONE = 0,	   /* No error */
    VGL_ERR_MEMORY,	   /* Memory errors, usually a failed malloc() */
    VGL_ERR_RANGE,	   /* Number out of range */
    VGL_ERR_BADOP,	   /* Bad operation */
    VGL_ERR_MAX_BOARDS,	   /* Too many video controller boards */
    VGL_ERR_INVALID_OP,	   /* Invalid Operation */
    VGL_ERR_KEYBOARD,	   /* Hardware related problems with keyboard */
    VGL_ERR_NOSUPPORT,	   /* An operation was tried that is not supported */
    VGL_ERR_BADRAM,	   /* A memory error has occured (a hardware error */
    VGL_ERR_VME_PERMIT,    /* The call to vgl_board_permit() failed */
    VGL_ERR_OTHER
  };

enum vgl_test_result
  {
    VGL_TEST_OK,
    VGL_TEST_STUCK8,
    VGL_TEST_STUCK32,
    VGL_TEST_ADDR8,
    VGL_TEST_ADDR32,
    VGL_TEST_RANDOM,
    VGL_TEST_UNSUPPORTED,
    VGL_TEST_BADCLUT,
    VGL_TEST_BADREG
  };

extern struct vgl_ffont *vgl_small_ffont, *vgl_large_ffont;
extern struct vgl_font vgl_small_font, vgl_large_font;
extern struct vgl_color vgl_default_clut[];

#endif
