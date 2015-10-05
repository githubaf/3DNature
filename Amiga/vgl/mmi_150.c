
#include "vgl.h"
#include "vgl_internals.h"

#ifdef SUPPORT_MMI_150

#include "mmi_150.h"


#define	DEF_ISR_MASK	(0x03)
#define	ISR_TRANS	(0x01)
#define	ISR_RECV	(0x02)

/* Just change DIM_SHIFT, not DIM_MASK1 or DIM_MASK */
#define DIM_SHIFT	(2)
#define	DIM_MASK1	((256>>DIM_SHIFT)-1)
#define DIM_MASK	((DIM_MASK1)     | (DIM_MASK1<<8)  | \
			 (DIM_MASK1<<16) | (DIM_MASK1<<24))


static void mmi_150_mouse_init (int b_num, VGBOARD * b);
static void mmi_150_kb_init (int b_num, VGBOARD * b);

#if !defined(OS9)
static void mmi_150_video_isr (int board_index);
static void mmi_150_mouse_isr (int board_index);
static void mmi_150_kb_isr (int board_index);
#endif

#ifdef PSOS
static void mmi_150_video_init (int b_num,
				VGBOARD *b, 
				void (*handler) (void));
static void mmi_150_mouse_isr_psos( void);
static void mmi_150_kb_isr_psos( void);
#else
static void mmi_150_video_init (int b_num, 
				VGBOARD *b, 
				void (*handler) (int param));
#endif

static int mmi_150_accel (VGBOARD * b, int delta);


/* The maximum number of MMI-100/150's that can operate in one system.
 * Note: If this number is changed, the number of ISR_STUB()'s must be changed
 */
#define	MAX_MMI_150	(8)

static VGBOARD *board_list[MAX_MMI_150];
static int board_list_init = 0;

/* Look closely, these are the actual functions! */
#ifdef PSOS
static void mmi_150_isrstub0 (void){  mmi_150_video_isr (0);}
static void mmi_150_isrstub1 (void){  mmi_150_video_isr (1);}
static void mmi_150_isrstub2 (void){  mmi_150_video_isr (2);}
static void mmi_150_isrstub3 (void){  mmi_150_video_isr (3);}
static void mmi_150_isrstub4 (void){  mmi_150_video_isr (4);}
static void mmi_150_isrstub5 (void){  mmi_150_video_isr (5);}
static void mmi_150_isrstub6 (void){  mmi_150_video_isr (6);}
static void mmi_150_isrstub7 (void){  mmi_150_video_isr (7);}

static void (*mmi_150_stub_list[]) (void) =
	{
	  mmi_150_isrstub0,
	  mmi_150_isrstub1,
	  mmi_150_isrstub2,
	  mmi_150_isrstub3,
	  mmi_150_isrstub4,
	  mmi_150_isrstub5,
	  mmi_150_isrstub6,
	  mmi_150_isrstub7
	};

#endif /* PSOS */

static PIXMAP mmi_150_default_pixmap =
  {
    NULL, VGL_ERR_NONE,		/* pixdata, error */
    NULL,			/* board */
    NULL,			/* mouse_pixmap */
    CLIP_ON, 0, 0, 0, 0,	/* clip_flag, x_min, y_min, x_max, y_max */
    
    255, 0,			/* Forground & background */
    0, 0,
    {0},			/* Fore/Back_8, expand_8 */
    NULL,			/* ffont */
    NULL,			/* Workspace */
    0,				/* Worksize */
    0,				/* Mouse_nocheck_flag */
    
    0,
    
    vgl_dumb_set_clip,
    vgl_dumb_clip_off,
    vgl_dumb_clip_on,
    vgl_dumb_pixerror,
    vgl_dumb_set_pixel,
    vgl_dumb_get_pixel,
    vgl_dumb_line,
    vgl_dumb_hline,
    vgl_dumb_vline,
    vgl_dumb_rect,
    vgl_dumb_fillrect,
    vgl_dumb_circle,
    vgl_dumb_fillcircle,
    vgl_dumb_ellipse,
    vgl_dumb_fillellipse,
    vgl_dumb_arc,
    vgl_dumb_fillarc,
    vgl_dumb_poly,
    vgl_dumb_fillpoly,
    vgl_dumb_bitblt,
    vgl_dumb_transbitblt,
    vgl_dumb_setfont,
    vgl_dumb_text,
    vgl_dumb_text2,
    vgl_dumb_clear,
    vgl_dumb_dissolve,
    vgl_dumb_setcur,
    vgl_dumb_test,
    vgl_dumb_nothing,
    vgl_dumb_quadlist,
    vgl_dumb_linelist,
    vgl_dumb_hlinelist,
  };


#if !defined(OS9)
static struct kb_mod_table mod_table[] = {
  /* Code, State, Action, Set, Reset, */
  {0x2A, L_SHIFT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xAA, L_SHIFT, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0x36, R_SHIFT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xB6, R_SHIFT, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},

  {0xAA, EL_SHIFT, MOD_SET, PREFIX_E0, PREFIX_E1},
  {0x12, EL_SHIFT, MOD_RESET, PREFIX_E0, PREFIX_E1},
  {0x36, ER_SHIFT, MOD_SET, PREFIX_E0, PREFIX_E1},
  {0xB6, ER_SHIFT, MOD_RESET, PREFIX_E0, PREFIX_E1},

  {0x1D, L_CTRL, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0x9D, L_CTRL, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0x1D, R_CTRL, MOD_SET, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT},
  {0x9D, R_CTRL, MOD_RESET, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT},

  {0x38, L_ALT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xB8, L_ALT, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0x38, R_ALT, MOD_SET, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT},
  {0xB8, R_ALT, MOD_RESET, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT},

  {0x45, NUM_LOCK, MOD_TOGGLE, 0, (PREFIX_ALL|NUM_LOCK_DOWN|E_ALL) & ~E_SHIFT},
  {0x45, NUM_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xC5, NUM_LOCK_DOWN, MOD_RESET, 0, (PREFIX_ALL|PREFIX_E11D|E_ALL)&~E_SHIFT},

  {0x46, SCRL_LOCK, MOD_TOGGLE, 0, (SCRL_LOCK_DOWN|PREFIX_ALL|E_ALL)&~E_SHIFT},
  {0x46, SCRL_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xC6, SCRL_LOCK_DOWN, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},

  {0x3A, CAPS_LOCK, MOD_TOGGLE, 0, (CAPS_LOCK_DOWN|PREFIX_ALL|E_ALL)&~E_SHIFT},
  {0x3A, CAPS_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
  {0xBA, CAPS_LOCK_DOWN, MOD_RESET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT},
};

#define NUM_MOD_TABLE	(sizeof(mod_table)/sizeof(mod_table[0]))

#define NORMAL_KEY       0,         (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0
#define E0_NORMAL_KEY    PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0
#define CURSOR_KEY       PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0


static struct kb_key_table key_table[] =
{
  {1, 0x01, NORMAL_KEY},
  {KUP | 1, 0x81, NORMAL_KEY},
  {2, 0x3B, NORMAL_KEY},
  {KUP | 2, 0xBB, NORMAL_KEY},
  {3, 0x3C, NORMAL_KEY},
  {KUP | 3, 0xBC, NORMAL_KEY},
  {4, 0x3D, NORMAL_KEY},
  {KUP | 4, 0xBD, NORMAL_KEY},
  {5, 0x3E, NORMAL_KEY},
  {KUP | 5, 0xBE, NORMAL_KEY},
  {6, 0x3F, NORMAL_KEY},
  {KUP | 6, 0xBF, NORMAL_KEY},
  {7, 0x40, NORMAL_KEY},
  {KUP | 7, 0xC0, NORMAL_KEY},
  {8, 0x41, NORMAL_KEY},
  {KUP | 8, 0xC1, NORMAL_KEY},
  {9, 0x42, NORMAL_KEY},
  {KUP | 9, 0xC2, NORMAL_KEY},

  {10, 0x43, NORMAL_KEY},
  {KUP | 10, 0xC3, NORMAL_KEY},
  {11, 0x44, NORMAL_KEY},
  {KUP | 11, 0xC4, NORMAL_KEY},
  {12, 0x57, NORMAL_KEY},
  {KUP | 12, 0xD7, NORMAL_KEY},
  {13, 0x58, NORMAL_KEY},
  {KUP | 13, 0xD8, NORMAL_KEY},

  {14, 0x37, E0_NORMAL_KEY},
  {KUP | 14, 0xB7, E0_NORMAL_KEY},
  {14, 0x54, NORMAL_KEY},
  {KUP | 14, 0xD4, NORMAL_KEY},

  {15, 0x46, NORMAL_KEY},
  {KUP | 15, 0xC6, NORMAL_KEY},

  {16, 0x45, PREFIX_E11D, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {KUP | 16, 0xC5, PREFIX_E11D, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {16, 0x46, E0_NORMAL_KEY},
  {KUP | 16, 0xC6, E0_NORMAL_KEY},

  {17, 0x29, NORMAL_KEY},
  {KUP | 17, 0xA9, NORMAL_KEY},
  {18, 0x02, NORMAL_KEY},
  {KUP | 18, 0x82, NORMAL_KEY},
  {19, 0x03, NORMAL_KEY},
  {KUP | 19, 0x83, NORMAL_KEY},

  {20, 0x04, NORMAL_KEY},
  {KUP | 20, 0x84, NORMAL_KEY},
  {21, 0x05, NORMAL_KEY},
  {KUP | 21, 0x85, NORMAL_KEY},
  {22, 0x06, NORMAL_KEY},
  {KUP | 22, 0x86, NORMAL_KEY},
  {23, 0x07, NORMAL_KEY},
  {KUP | 23, 0x87, NORMAL_KEY},
  {24, 0x08, NORMAL_KEY},
  {KUP | 24, 0x88, NORMAL_KEY},
  {25, 0x09, NORMAL_KEY},
  {KUP | 25, 0x89, NORMAL_KEY},
  {26, 0x0A, NORMAL_KEY},
  {KUP | 26, 0x8A, NORMAL_KEY},
  {27, 0x0B, NORMAL_KEY},
  {KUP | 27, 0x8B, NORMAL_KEY},
  {28, 0x0C, NORMAL_KEY},
  {KUP | 28, 0x8C, NORMAL_KEY},
  {29, 0x0D, NORMAL_KEY},
  {KUP | 29, 0x8D, NORMAL_KEY},

  {30, 0x0E, NORMAL_KEY},
  {KUP | 30, 0x8E, NORMAL_KEY},
  {31, 0x52, CURSOR_KEY},
  {KUP | 31, 0xD2, CURSOR_KEY},
  {32, 0x47, CURSOR_KEY},
  {KUP | 32, 0xC7, CURSOR_KEY},
  {33, 0x49, CURSOR_KEY},
  {KUP | 33, 0xC9, CURSOR_KEY},

  {34, 0x45, 0, (PREFIX_Ex | E_ALL | PREFIX_E11D) & ~E_SHIFT, 0},
  {KUP | 34, 0xC5, 0, (PREFIX_Ex | E_ALL | PREFIX_E11D) & ~E_SHIFT, 0},

  {35, 0x35, E0_NORMAL_KEY},
  {KUP | 35, 0xB5, E0_NORMAL_KEY},

  {36, 0x37, NORMAL_KEY},
  {KUP | 36, 0xB7, NORMAL_KEY},
  {37, 0x4A, NORMAL_KEY},
  {KUP | 37, 0xCA, NORMAL_KEY},

  {38, 0x0F, NORMAL_KEY},
  {KUP | 38, 0x8F, NORMAL_KEY},
  {39, 0x10, NORMAL_KEY},
  {KUP | 39, 0x90, NORMAL_KEY},
  {40, 0x11, NORMAL_KEY},
  {KUP | 40, 0x91, NORMAL_KEY},
  {41, 0x12, NORMAL_KEY},
  {KUP | 41, 0x92, NORMAL_KEY},
  {42, 0x13, NORMAL_KEY},
  {KUP | 42, 0x93, NORMAL_KEY},
  {43, 0x14, NORMAL_KEY},
  {KUP | 43, 0x94, NORMAL_KEY},
  {44, 0x15, NORMAL_KEY},
  {KUP | 44, 0x95, NORMAL_KEY},
  {45, 0x16, NORMAL_KEY},
  {KUP | 45, 0x96, NORMAL_KEY},
  {46, 0x17, NORMAL_KEY},
  {KUP | 46, 0x97, NORMAL_KEY},
  {47, 0x18, NORMAL_KEY},
  {KUP | 47, 0x98, NORMAL_KEY},
  {48, 0x19, NORMAL_KEY},
  {KUP | 48, 0x99, NORMAL_KEY},
  {49, 0x1A, NORMAL_KEY},
  {KUP | 49, 0x9A, NORMAL_KEY},

  {50, 0x1B, NORMAL_KEY},
  {KUP | 50, 0x9B, NORMAL_KEY},
  {51, 0x2B, NORMAL_KEY},
  {KUP | 51, 0xAB, NORMAL_KEY},
  {52, 0x53, CURSOR_KEY},
  {KUP | 52, 0xD3, CURSOR_KEY},
  {53, 0x4F, CURSOR_KEY},
  {KUP | 53, 0xCF, CURSOR_KEY},
  {54, 0x51, CURSOR_KEY},
  {KUP | 54, 0xD1, CURSOR_KEY},
  {55, 0x47, NORMAL_KEY},
  {KUP | 55, 0xC7, NORMAL_KEY},
  {56, 0x48, NORMAL_KEY},
  {KUP | 56, 0xC8, NORMAL_KEY},
  {57, 0x49, NORMAL_KEY},
  {KUP | 57, 0xC9, NORMAL_KEY},
  {58, 0x4E, NORMAL_KEY},
  {KUP | 58, 0xCE, NORMAL_KEY},

  {59, 0x3A, NORMAL_KEY},
  {KUP | 59, 0xBA, NORMAL_KEY},
  {60, 0x1E, NORMAL_KEY},
  {KUP | 60, 0x9E, NORMAL_KEY},
  {61, 0x1F, NORMAL_KEY},
  {KUP | 61, 0x9F, NORMAL_KEY},
  {62, 0x20, NORMAL_KEY},
  {KUP | 62, 0xA0, NORMAL_KEY},
  {63, 0x21, NORMAL_KEY},
  {KUP | 63, 0xA1, NORMAL_KEY},
  {64, 0x22, NORMAL_KEY},
  {KUP | 64, 0xA2, NORMAL_KEY},
  {65, 0x23, NORMAL_KEY},
  {KUP | 65, 0xA3, NORMAL_KEY},
  {66, 0x24, NORMAL_KEY},
  {KUP | 66, 0xA4, NORMAL_KEY},
  {67, 0x25, NORMAL_KEY},
  {KUP | 67, 0xA5, NORMAL_KEY},
  {68, 0x26, NORMAL_KEY},
  {KUP | 68, 0xA6, NORMAL_KEY},
  {69, 0x27, NORMAL_KEY},
  {KUP | 69, 0xA7, NORMAL_KEY},

  {70, 0x28, NORMAL_KEY},
  {KUP | 70, 0xA8, NORMAL_KEY},
  {71, 0x1C, NORMAL_KEY},
  {KUP | 71, 0x9C, NORMAL_KEY},
  {72, 0x4B, NORMAL_KEY},
  {KUP | 72, 0xCB, NORMAL_KEY},
  {73, 0x4C, NORMAL_KEY},
  {KUP | 73, 0xCC, NORMAL_KEY},
  {74, 0x4D, NORMAL_KEY},
  {KUP | 74, 0xCD, NORMAL_KEY},
  {75, 0x2A, NORMAL_KEY},
  {KUP | 75, 0xAA, NORMAL_KEY},
  {76, 0x2C, NORMAL_KEY},
  {KUP | 76, 0xAC, NORMAL_KEY},
  {77, 0x2D, NORMAL_KEY},
  {KUP | 77, 0xAD, NORMAL_KEY},
  {78, 0x2E, NORMAL_KEY},
  {KUP | 78, 0xAE, NORMAL_KEY},
  {79, 0x2F, NORMAL_KEY},
  {KUP | 79, 0xAF, NORMAL_KEY},

  {80, 0x30, NORMAL_KEY},
  {KUP | 80, 0xB0, NORMAL_KEY},
  {81, 0x31, NORMAL_KEY},
  {KUP | 81, 0xB1, NORMAL_KEY},
  {82, 0x32, NORMAL_KEY},
  {KUP | 82, 0xB2, NORMAL_KEY},
  {83, 0x33, NORMAL_KEY},
  {KUP | 83, 0xB3, NORMAL_KEY},
  {84, 0x34, NORMAL_KEY},
  {KUP | 84, 0xB4, NORMAL_KEY},
  {85, 0x35, NORMAL_KEY},
  {KUP | 85, 0xB5, NORMAL_KEY},
  {86, 0x36, NORMAL_KEY},
  {KUP | 86, 0xB6, NORMAL_KEY},
  {87, 0x48, CURSOR_KEY},
  {KUP | 87, 0xC8, CURSOR_KEY},
  {88, 0x4F, NORMAL_KEY},
  {KUP | 88, 0xC4, NORMAL_KEY},
  {89, 0x50, NORMAL_KEY},
  {KUP | 89, 0xD0, NORMAL_KEY},

  {90, 0x51, NORMAL_KEY},
  {KUP | 90, 0xD1, NORMAL_KEY},
  {91, 0x1C, E0_NORMAL_KEY},
  {KUP | 91, 0x9C, E0_NORMAL_KEY},
  {92, 0x1D, NORMAL_KEY},
  {KUP | 92, 0x9D, NORMAL_KEY},
  {93, 0x38, NORMAL_KEY},
  {KUP | 93, 0xB8, NORMAL_KEY},
  {94, 0x39, NORMAL_KEY},
  {KUP | 94, 0xB9, NORMAL_KEY},
  {95, 0x38, E0_NORMAL_KEY},
  {KUP | 95, 0xB8, E0_NORMAL_KEY},
  {96, 0x1D, E0_NORMAL_KEY},
  {KUP | 96, 0x9D, E0_NORMAL_KEY},
  {97, 0x4B, CURSOR_KEY},
  {KUP | 97, 0xCB, CURSOR_KEY},
  {98, 0x50, CURSOR_KEY},
  {KUP | 98, 0xD0, CURSOR_KEY},
  {99, 0x4D, CURSOR_KEY},
  {KUP | 99, 0xCD, CURSOR_KEY},

  {100, 0x52, NORMAL_KEY},
  {KUP | 100, 0xD2, NORMAL_KEY},
  {101, 0x53, NORMAL_KEY},
  {KUP | 101, 0xD3, NORMAL_KEY},
};

#define NUM_KEY_TABLE	(sizeof(key_table)/sizeof(key_table[0]))
#endif

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_board_init (VGBOARD * b)
{
  int i;

  /***** Initalize the board list, if required *****/
  if (board_list_init == 0)
    {
      for (i = 0; i < MAX_MMI_150; i++)
	board_list[i] = NULL;

      board_list_init = 1;
    }

  /***** Find an empty board slot *****/
  for (i = 0; i < MAX_MMI_150; i++)
    {
      if (board_list[i] == NULL)
	break;
    }

  /***** Set error code and return, on error *****/
  if (i >= MAX_MMI_150)
    {
      b->error = VGL_ERR_MAX_BOARDS;
      return;
    }

  /***** Add new board to board list *****/
  board_list[i] = b;

  /***** Get permission to access the board *****/
  vgl_board_permit (b);

  /***** Set irq_vector *****/
  vgl_set_bytereg (b, VIVR, b->irq_vector);

  /***** Initalize board sub-systems *****/
#if defined (PSOS)
  mmi_150_video_init (i, b, mmi_150_stub_list[i]);
#elif defined (VXWORKS)
  mmi_150_video_init (i, b, mmi_150_video_isr);
#elif defined (OS9)
  mmi_150_video_init (i, b, NULL);
#endif
  mmi_150_mouse_init (i, b);
  mmi_150_kb_init (i, b);

  /***** Set irq masks *****/
  vgl_set_bytereg (b, VIMR, (1 << 3) | (1 << 1) | (1 << 0));
}


/*****************************************************************************/
/*****************************************************************************/
#ifdef PSOS
static void 
mmi_150_video_init (int b_num, VGBOARD * b, void (*handler) (void))
#else
static void 
mmi_150_video_init (int b_num, VGBOARD * b, void (*handler) (int param))
#endif
{
  vgl_set_bytereg (b, DUART_OUT_RESET, GRST);
  vgl_sleep ((1 * VGL_CLOCK) / 60);
  vgl_set_bytereg (b, DUART_OUT_SET, GRST);
  vgl_sleep ((1 * VGL_CLOCK) / 60);

  vgl_set_reg (b, G3BL, b->mode->mmi_150.bl);
  vgl_sleep ((1 * VGL_CLOCK) / 60);
  vgl_set_reg (b, G3CR, b->mode->mmi_150.cr & 0xFFFFFFFE);

  vgl_set_reg (b, G3HS, b->mode->mmi_150.hs);
  vgl_set_reg (b, G3BP, b->mode->mmi_150.bp);
  vgl_set_reg (b, G3HD, b->mode->mmi_150.hd);
  vgl_set_reg (b, G3SD, b->mode->mmi_150.sd);
  vgl_set_reg (b, G3BR, b->mode->mmi_150.br);
  vgl_set_reg (b, G3VS, b->mode->mmi_150.vs);
  vgl_set_reg (b, G3VB, b->mode->mmi_150.vb);
  vgl_set_reg (b, G3VD, b->mode->mmi_150.vd);
  vgl_set_reg (b, G3LT, b->mode->mmi_150.lt);
  vgl_set_reg (b, G3LS, b->mode->mmi_150.ls);
  vgl_set_reg (b, G3MI, b->mode->mmi_150.mi);
  vgl_set_reg (b, G3MR, 0xFF);
  vgl_set_reg (b, G3TD, b->mode->mmi_150.td);

  vgl_set_reg (b, G3TS, 0);

  vgl_setrgb (b, 0, 256, vgl_default_clut);
  if (b->mode->mmi_150.sync_pol < 0)
    vgl_set_bytereg (b, DUART_OUT_SET, SYNP);
  else
    vgl_set_bytereg (b, DUART_OUT_RESET, SYNP);

  vgl_set_reg (b, G3CR, (b->mode->mmi_150.cr & 0xFFFFFFFE) | 1);

#if !defined(OS9)
  vgl_install_isr (b->irq_vector, handler, b_num);
#endif
}

/*****************************************************************************/
/****************************************************************************/
#if !defined(OS9)
static void 
mmi_150_video_isr (int board_index)
{
  int i;
  unsigned long *clut;
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
      vgl_set_reg (b, G3TS, b->tos_value);
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
	}
      else if (b->ss_counter >= 0 && b->ss_istatus == 0)
	{
	  b->ss_istatus = 1;	/* Turn it on */
	  b->color_start = 0;
	  b->color_end = 256;
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
	}
    }
  else
    /* if(b->status==SS_ON) */
    /* If the screen is "turned on"... */
    {
      if (b->ss_istatus == 0)	/* if the screen is off, turn it on */
	{
	  b->ss_istatus = 1;
	  b->color_start = 0;
	  b->color_end = 256;
	}
    }



  /* Set the color look-up table (CLUT) */
  if (b->color_start != b->color_end)
    {
      clut = (unsigned long *) ((char *) (b->addr) + CLUT);

      i = b->color_start;
      if (b->ss_istatus != 0)
	{
	  switch ((b->color_end - b->color_start) & 0x07)
	    {
	    clut_loop:
	    case 8:
	      clut[i] = b->color_table[i];
	      i++;
	    case 7:
	      clut[i] = b->color_table[i];
	      i++;
	    case 6:
	      clut[i] = b->color_table[i];
	      i++;
	    case 5:
	      clut[i] = b->color_table[i];
	      i++;
	    case 4:
	      clut[i] = b->color_table[i];
	      i++;
	    case 3:
	      clut[i] = b->color_table[i];
	      i++;
	    case 2:
	      clut[i] = b->color_table[i];
	      i++;
	    case 1:
	      clut[i] = b->color_table[i];
	      i++;
	    case 0:
	      if (i < b->color_end)
		goto clut_loop;
	    }
	}
      else
	{
	  switch ((b->color_end - b->color_start) & 0x07)
	    {
	    clut_loop2:
	    case 8:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 7:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 6:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 5:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 4:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 3:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 2:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 1:
	      clut[i] = (b->color_table[i] >> DIM_SHIFT) & DIM_MASK;
	      i++;
	    case 0:
	      if (i < b->color_end)
		goto clut_loop2;
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

  if (b->mouse_blanking_level == 0 && 
      (b->mouse_visible_flag == 0 || b->mouse_update_flag != 0))
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

  vgl_set_reg (b, G3CLGI, 0);
}
#endif

/****************************************************************************/
/****************************************************************************/
void 
mmi_150_board_kill (VGBOARD * b)
{
  int i;

  /***** Set irq masks *****/
  vgl_set_bytereg (b, VIMR, 0);

  if (b->mouse_under != NULL)
    vgl_freepixmap (b->mouse_under);

  if (b->cur_pixmap->mouse_pixmap != NULL)
    vgl_freepixmap (b->cur_pixmap->mouse_pixmap);

  for (i = 0; i < MAX_MMI_150; i++)
    {
      if (board_list[i] == b)
	board_list[i] = NULL;
    }

#if defined(OS9)
  munlink (b->pointer1);
#else
  free (b);
#endif
}



/*****************************************************************************/
/*****************************************************************************/
PIXMAP *
mmi_150_pixmap_map (VGBOARD * b, int pagenum)
{
  PIXMAP *p;

  p = vgl_malloc (sizeof (PIXMAP));
  if (p)
    {
      *p = mmi_150_default_pixmap;
      p->pixdata = vgl_malloc (sizeof (p->pixdata[0]));
      if (p->pixdata == NULL)
	{
	  vgl_free (p);
	  return (NULL);
	}

      p->pixdata->x_res = b->mode->x_res;
      p->pixdata->y_res = b->mode->y_res;
      p->pixdata->x_size = b->mode->mmi_150.x_size;
      p->pixdata->users = 1;
      p->pixdata->free_flag = 0;
      p->pixdata->data = (((char *) b->addr) + 
			  (b->mode->mmi_150.page_size * pagenum));

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
	  /* An inline, modified verson of: mmi_150_pixmap_disp( p); */
	  b->mouse_blanking_level = 1;
	  b->mouse_visible_flag = 0;

	  b->cur_pixmap = p;
	  b->next_cur_pixmap = p;
	  b->tos_value = ((((char *) p->pixdata->data) - 
			   ((char *) p->board->addr)) >> 8) & 0xFFFFFFFC;

	  b->tos_reset = 1;
	  b->mouse_update_flag = 1;
	}

      return (p);
    }
  else
    return (NULL);
}


/****************************************************************************/
/****************************************************************************/
void 
mmi_150_pixmap_disp (PIXMAP *p)
{
  unsigned long temp;

  if (p->board != NULL)
    {
      temp = vgl_mask_irq (p->board);

      p->board->tos_value = ((((char *) p->pixdata->data) - 
			      ((char *) p->board->addr)) >> 8) & 0xFFFFFFFC;
      p->board->next_cur_pixmap = p;

      p->board->tos_reset = 1;
      p->board->mouse_update_flag = 1;

      vgl_unmask_irq (p->board, temp);
    }
  else
    p->error = VGL_ERR_BADOP;
}


/****************************************************************************/
/****************************************************************************/
int 
mmi_150_boarderror (VGBOARD * b)
{
  int err;

  err = b->error;
  b->error = VGL_ERR_NONE;
  return (err);
}


/****************************************************************************/
/****************************************************************************/
void 
mmi_150_setrgb (VGBOARD *b, 
		int color_num, 
		int n_colors, 
		struct vgl_color *colors)
{
  int i;
  unsigned long old_mode;

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

      vgl_unmask_irq (b, old_mode);
    }
}


/****************************************************************************/
/****************************************************************************/
void 
mmi_150_getrgb (VGBOARD * b, 
		int color_num, 
		int n_colors, 
		struct vgl_color *colors)
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
static void 
mmi_150_mouse_init (int b_num, VGBOARD *b)
{
#if defined(PSOS)
  vgl_install_isr (b->irq_vector + 2, mmi_150_mouse_isr_psos, b_num);
#elif !defined(OS9)
  vgl_install_isr (b->irq_vector + 2, mmi_150_mouse_isr, b_num);
#endif

  b->mouse_buf.in_start = 0;
  b->mouse_buf.in_end = 0;

  b->mouse_max_x = (b->mode->x_res - 1) * b->mouse_accel_base;
  b->mouse_max_y = (b->mode->y_res - 1) * b->mouse_accel_base;

  vgl_set_bytereg (b, DUART_CRA, 0x20);		/* Reset Receiver */
  vgl_set_bytereg (b, DUART_CRA, 0x30);		/* Reset Transmitter */
  vgl_set_bytereg (b, DUART_CRA, 0x40);		/* Reset Error Status */
  vgl_set_bytereg (b, DUART_CRA, 0x50);		/* Reset Break Change */

  vgl_set_bytereg (b, DUART_ACR, 0xE0);

  vgl_set_bytereg (b, DUART_CRA, 0x10);		/* Set pointer to MRA */
  vgl_set_bytereg (b, DUART_MRA, 0x12);
  vgl_set_bytereg (b, DUART_MRA, 0x0F);
  vgl_set_bytereg (b, DUART_CSRA, 0x66);
  vgl_set_bytereg (b, DUART_OPCR, 0);
  vgl_set_bytereg (b, DUART_IMR, ISR_RECV);
  vgl_set_bytereg (b, DUART_OUT_SET, RTSA);

  vgl_set_bytereg (b, DUART_CRA, 0x05);	 /* Enable receiver and transmitter */
}


/****************************************************************************/
/****************************************************************************/
#ifdef PSOS
static void
mmi_150_mouse_isr_psos( void)
{
  int	i;

  for( i=0; i<MAX_MMI_150; i++)
    mmi_150_mouse_isr( i);
}
#endif

#if !defined(OS9)
static void
mmi_150_mouse_isr (int j)
{
  unsigned char ch, err;
  VGBOARD *bd;
  int n_bytes, a, b, c;
  signed char dx, dy;

  if (board_list[j] != NULL)
    {
      bd = board_list[j];

      err = vgl_get_bytereg (bd, DUART_SRA);

      while (err & 0x01)
	{
	  /* Receiver Ready */
	  ch = vgl_get_bytereg (bd, DUART_RHRA);
	  
	  if (bd->mouse_buf.in_start != (bd->mouse_buf.in_end+1)%MOUSE_BUF_SIZE
	      && !(err & 0xF0))
	    {
	      bd->mouse_buf.err_in[bd->mouse_buf.in_end] = err & 0xF3;
	      bd->mouse_buf.in[bd->mouse_buf.in_end] = ch;
	      bd->mouse_buf.in_end = (bd->mouse_buf.in_end+1) % MOUSE_BUF_SIZE;
	    }
	  
	  if ((err & 0x10) != 0)	/* Reset Overrun error, if needed */
	    vgl_set_bytereg (bd, DUART_CRA, 0x40);
	  
	  err = vgl_get_bytereg (bd, DUART_SRA);
	}
      
      /* Call mouse handler */
      do
	{
	  /* Is there a hold over from the previous packet?
	     IE, a middle button? */
	  if (bd->mouse_buf.in_start != bd->mouse_buf.in_end && 
	      bd->mouse_sync == 3)
	    {
	      a = bd->mouse_buf.in[bd->mouse_buf.in_start] & 0xF0;
	      bd->mouse_sync = 0;
	      
	      if (a == 0x20)
		{
		  bd->mouse_middle = 1;
		  bd->mouse_buf.in_start =
		    (bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
		  bd->mouse_sync++;
		}
	      else if (a == 0x00)
		{
		  bd->mouse_middle = 0;
		  bd->mouse_buf.in_start = 
		    (bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
		  bd->mouse_sync++;
		}
	      else
		bd->mouse_sync = 0;
	      
	      bd->mouse_buttons = 
		(((bd->mouse_oa & 0x20) ? MOUSE_LEFT  : 0x00) |
		 ((bd->mouse_oa & 0x10) ? MOUSE_RIGHT : 0x00) |
		 (bd->mouse_middle ? MOUSE_MID : 0x00));
	      
	      /*  Generate a mouse event here */
	      vgl_mouse_enqueue (bd,
				 bd->mouse_x / bd->mouse_accel_base,
				 bd->mouse_y / bd->mouse_accel_base,
				 bd->mouse_buttons);
	      
	      bd->mouse_cur_x = 
		bd->mouse_x / bd->mouse_accel_base - bd->mouse_hot_x;
	      bd->mouse_cur_y = 
		bd->mouse_y / bd->mouse_accel_base - bd->mouse_hot_y;
	      bd->mouse_update_flag = 1;
	      
	      if ((bd->ss_flags & SS_MOUSE) != 0)
		{
		  /*bd->ss_tick(b); */
		  if (bd->ss_status == SS_NORMAL)
		    bd->ss_counter = bd->ss_timeout_value;
		}
	    }
	  
	      /* Throw away bytes until sync character */
	  while (bd->mouse_buf.in_start != bd->mouse_buf.in_end &&
		 (bd->mouse_buf.in[bd->mouse_buf.in_start] & 0x40) == 0)
	    {
	      bd->mouse_buf.in_start = 
		(bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
	      bd->mouse_sync = 0;
	    }
	  
	  
	  /* How many characters are built up? */
	  if (bd->mouse_buf.in_start <= bd->mouse_buf.in_end)
	    n_bytes = bd->mouse_buf.in_end - bd->mouse_buf.in_start;
	  else
	    n_bytes = 
	      (bd->mouse_buf.in_end+MOUSE_BUF_SIZE) - bd->mouse_buf.in_start;
	  
	  
	  if (n_bytes >= 3)
	    {
	      bd->mouse_oa = bd->mouse_buf.in[bd->mouse_buf.in_start];
	      bd->mouse_buf.in_start = 
		(bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
	      b = bd->mouse_buf.in[bd->mouse_buf.in_start];
	      bd->mouse_buf.in_start = 
		(bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
	      c = bd->mouse_buf.in[bd->mouse_buf.in_start];
	      bd->mouse_buf.in_start = 
		(bd->mouse_buf.in_start + 1) % MOUSE_BUF_SIZE;
	      bd->mouse_sync = 3;
	      n_bytes -= 3;
	      
	      dx = (bd->mouse_oa & 0x03) << 6 | (b & 0x3F);
		  dy = (bd->mouse_oa & 0x0C) << 4 | (c & 0x3F);
	      
	      bd->mouse_x += mmi_150_accel (bd, dx);
	      if (bd->mouse_x >= bd->mouse_max_x)
		bd->mouse_x = bd->mouse_max_x - 1;
	      else if (bd->mouse_x < 0)
		bd->mouse_x = 0;
	      
	      bd->mouse_y += mmi_150_accel (bd, dy);
	      if (bd->mouse_y >= bd->mouse_max_y)
		bd->mouse_y = bd->mouse_max_y - 1;
	      else if (bd->mouse_y < 0)
		bd->mouse_y = 0;
	      
	      bd->mouse_buttons = 
		(((bd->mouse_oa & 0x20) ? MOUSE_LEFT  : 0x00) |
		 ((bd->mouse_oa & 0x10) ? MOUSE_RIGHT : 0x00) |
		 (bd->mouse_middle ? MOUSE_MID : 0x00));
	      
	      /* Generate a mouse event here */
	      vgl_mouse_enqueue (bd,
				 bd->mouse_x / bd->mouse_accel_base,
				 bd->mouse_y / bd->mouse_accel_base,
				 bd->mouse_buttons);
	      
	      bd->mouse_cur_x = 
		bd->mouse_x / bd->mouse_accel_base - bd->mouse_hot_x;
	      bd->mouse_cur_y = 
		bd->mouse_y / bd->mouse_accel_base - bd->mouse_hot_y;
	      bd->mouse_update_flag = 1;
	      
	      if ((bd->ss_flags & SS_MOUSE) != 0)
		{
		  /*bd->ss_tick(b); */
		  if (bd->ss_status == SS_NORMAL)
		    bd->ss_counter = bd->ss_timeout_value;
		}
	      /*
		 if( bd->mouse_user_handler!=NULL)
		 (*bd->mouse_user_handler)( x, y, bd->mouse_cur_buttons);
		 */
	    }
	}
      while (n_bytes >= 3);
    }			/* If board is valid */
}
#endif

/****************************************************************************/
/****************************************************************************/
void 
mmi_150_mouse_getcoords (VGBOARD * b, 
			 int *x, 
			 int *y, 
			 int *buttons, 
			 int wait_flag)
{
  unsigned long old_mode;

  if (wait_flag)
    vgl_mouse_dequeue (b, x, y, buttons, 1);
  else
    {
      old_mode = vgl_mask_irq (b);

      *x = b->mouse_x / b->mouse_accel_base;
      *y = b->mouse_y / b->mouse_accel_base;
      *buttons = b->mouse_buttons;

      vgl_unmask_irq (b, old_mode);
    }
}


/****************************************************************************/
/****************************************************************************/
void 
mmi_150_mouse_setcoords (VGBOARD * b, int x, int y)
{
  unsigned long old_mode;

  old_mode = vgl_mask_irq (b);

  b->mouse_x = x * b->mouse_accel_base;
  b->mouse_y = y * b->mouse_accel_base;

  b->mouse_cur_x = x - b->mouse_hot_x;
  b->mouse_cur_y = y - b->mouse_hot_y;
  b->mouse_update_flag = 1;

  vgl_unmask_irq (b, old_mode);
}


/****************************************************************************/
/****************************************************************************/
void 
mmi_150_mouse_sethandler (VGBOARD *b, 
			  void (*fctn) (int x, int y, int buttons))
{
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_mouse_setpixmap (VGBOARD * b, PIXMAP * pixmap, int hot_x, int hot_y)
{
  if (b->mouse_ptr != NULL)
    vgl_freepixmap (b->mouse_ptr);
  if (b->mouse_under != NULL)
    vgl_freepixmap (b->mouse_under);

  b->mouse_ptr = vgl_dupepixmap (pixmap);
  b->mouse_under = vgl_makepixmap (vgl_x_res (pixmap), vgl_y_res (pixmap));
  b->mouse_hot_x = hot_x;
  b->mouse_hot_y = hot_y;
  b->mouse_cur_x = b->mouse_x / b->mouse_accel_base - b->mouse_hot_x;
  b->mouse_cur_y = b->mouse_y / b->mouse_accel_base - b->mouse_hot_y;

  vgl_set_clip (b->mouse_under, -1, -1, -1, -1);
  vgl_clip_on (b->mouse_under);
  b->mouse_update_flag = 1;
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_mouse_ptr_on (VGBOARD * b)
{
  unsigned long irq_flags;

  irq_flags = vgl_mask_irq (b);

  if (b->mouse_blanking_level > 0)
    b->mouse_blanking_level--;

  vgl_unmask_irq (b, irq_flags);
}

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_mouse_ptr_off (VGBOARD * b)
{
  unsigned long irq_flags;

  irq_flags = vgl_mask_irq (b);

  if (b->mouse_visible_flag == 1)
    {
#if !defined(OS9)
      if (b->cur_pixmap->mouse_pixmap != NULL && b->mouse_under != NULL)
	vgl_bitblt (b->mouse_under,
		    0, 0, -1, -1,
		    b->cur_pixmap->mouse_pixmap,
		    b->mouse_old_x, b->mouse_old_y);
#endif
      b->mouse_visible_flag = 0;
    }

  b->mouse_blanking_level++;

  vgl_unmask_irq (b, irq_flags);
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_mouse_setaccel (VGBOARD *b,
			int base_speed, 
			int accel_thresh, 
			int accel_num, 
			int accel_den)
{
  unsigned long old_mode;

  old_mode = vgl_mask_irq (b);

  b->mouse_accel_base = base_speed;
  b->mouse_accel_thresh = accel_thresh;
  b->mouse_accel_num = accel_num;
  b->mouse_accel_den = accel_den;
  b->mouse_max_x = (b->mode->x_res - 1) * b->mouse_accel_base;
  b->mouse_max_y = (b->mode->y_res - 1) * b->mouse_accel_base;

  vgl_unmask_irq (b, old_mode);
}


/*****************************************************************************/
/*****************************************************************************/
static int 
mmi_150_accel (VGBOARD * b, int delta)
{
  if (delta > 0)
    {
      if (delta >= b->mouse_accel_thresh)
	return (((delta - b->mouse_accel_thresh) * 
		 b->mouse_accel_num / 
		 b->mouse_accel_den) + 
		b->mouse_accel_thresh);
      else
	return (delta);
    }
  else
    {
      if (-delta >= b->mouse_accel_thresh)
	return (((delta + b->mouse_accel_thresh) * 
		 b->mouse_accel_num / 
		 b->mouse_accel_den) - 
		b->mouse_accel_thresh);
      else
	return (delta);
    }
}

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_ss_set (VGBOARD * b, short status, long timeout, short flags)
{
  unsigned long temp;

  temp = vgl_mask_irq (b);

  b->ss_status = status;
  b->ss_flags = flags;
  b->ss_counter = timeout;
  b->ss_timeout_value = timeout;

  vgl_unmask_irq (b, temp);
}

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_150_ss_tick (VGBOARD * b, short flags)
{
  if (b->ss_status == SS_NORMAL && (b->ss_flags & flags) != 0)
    b->ss_counter = b->ss_timeout_value;
}


/*****************************************************************************/
/*****************************************************************************/
static void 
mmi_150_kb_init (int b_num, VGBOARD * b)
{
#if defined(PSOS)
  vgl_install_isr (b->irq_vector + 3, mmi_150_kb_isr_psos, b_num);
#elif !defined(OS9)
  vgl_install_isr (b->irq_vector + 3, mmi_150_kb_isr, b_num);
#endif

  vgl_set_bytereg (b, KSR, 0);
}


/****************************************************************************/
/****************************************************************************/
#ifdef PSOS
static void
mmi_150_kb_isr_psos( void)
{
int	i;

for( i=0; i<MAX_MMI_150; i++)
  {
    mmi_150_kb_isr( i);
  }
}
#endif

#if !defined(OS9)
static void
mmi_150_kb_isr (int j)
{
  int i, c;
  VGBOARD *b;

  if (board_list[j] != NULL)
    {
      b = board_list[j];

      /* There is a scancode ready */
      while ((vgl_get_bytereg (b, KSR) & KINT) != 0)
	{
	  c = vgl_get_bytereg (b, KEYB);

	  if (c == 0xE0)
	    b->kb_state |= PREFIX_E0;
	  else if (c == 0xE1)
	    b->kb_state |= PREFIX_E1;
	  else if ((c == 0x1D || c == 0x9D) && (b->kb_state & PREFIX_E1) != 0)
	    {
	      b->kb_state |= PREFIX_E11D;
	      b->kb_state &= ~PREFIX_E1;
	    }
	  else
	    {
	      /* Decode state modifiers */
	      for (i = 0; i < NUM_MOD_TABLE; i++)
		{
		  if (c == mod_table[i].keycode &&
		      (b->kb_state & mod_table[i].set) == mod_table[i].set &&
		      (b->kb_state & mod_table[i].reset) == 0)
		    {
		      switch (mod_table[i].action)
			{
			case MOD_SET:
			  b->kb_state |= mod_table[i].state;
			  break;

			case MOD_RESET:
			  b->kb_state &= ~mod_table[i].state;
			  break;

			case MOD_TOGGLE:
			  b->kb_state ^= mod_table[i].state;
			  break;
			}

		      /* Don't "break;" here,
			 there can be more than one modifier per scancode */
		    } /* End if, scancode is a match */
		} /* End check for mod codes */

	      /* Decode into "real" key scancode */
	      for (i = 0; i < NUM_KEY_TABLE; i++)
		{
		  if (c == key_table[i].keycode &&
		      (b->kb_state & key_table[i].set) == key_table[i].set &&
		      (b->kb_state & key_table[i].reset) == 0 &&
		      (key_table[i].one_of == 0 || 
		       (b->kb_state & key_table[i].one_of) != 0))
		    {
		      vgl_kb_enqueue (b, b->kb_state | key_table[i].key_num);

		      if ((b->ss_flags & SS_KEYBOARD) != 0)
			{
			  /*b->ss_tick(b); */
			  if (b->ss_status == SS_NORMAL)
			    b->ss_counter = b->ss_timeout_value;
			}

		      break;	/* Only one key# per scancode */
		    }
		}

	      /* Reset b->state for next keycode */
	      b->kb_state = 
		(b->kb_state & (~SINGLE_KEY_RESET)) | SINGLE_KEY_SET;
	    }

	  vgl_set_bytereg (b, KSR, 0);
	} /* End, if scancode is waiting */
    } /* End, if valid board */
}
#endif

/****************************************************************************/
/****************************************************************************/
int 
mmi_150_kbhit (VGBOARD * b)
{
  return (vgl_kb_queue_check (b));
}


/***************************************************************************/
/***************************************************************************/
long 
mmi_150_getkey (VGBOARD * b, int wait_flag)
{
  unsigned long c;

  if (vgl_kb_dequeue (b, &c, wait_flag) == 0)
    return (c);
  else
    return (0);
}

/****************************************************************************/
/****************************************************************************/
void 
mmi_150_kb_setled (VGBOARD * b, unsigned long led_flags)
{
/* The MMI-150 does not allow the keyboard LED's to be set */
}


/****************************************************************************/
/****************************************************************************/
int
mmi_150_test (VGBOARD *b, char *msg)
{
  strcpy (msg, "Hardware test of MMI-150 unsupported.");
  return (VGL_TEST_UNSUPPORTED);
}


/****************************************************************************/
/****************************************************************************/

#endif /* SUPPORT_MMI_150 */
