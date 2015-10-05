
#include "vgl.h"
#include "vgl_internals.h"

#ifdef SUPPORT_MMI_250

#include "mmi_250.h"


#define	DEF_ISR_MASK	(0x03)
#define	ISR_TRANS	(0x01)
#define	ISR_RECV	(0x02)

/* Just change DIM_SHIFT, not DIM_MASK1 or DIM_MASK */
#define DIM_SHIFT	(2)
#define	DIM_MASK1	((256>>DIM_SHIFT)-1)
#define DIM_MASK	((DIM_MASK1) | (DIM_MASK1<<8) |     \
			 (DIM_MASK1<<16) | (DIM_MASK1<<24))


static void mmi_250_mouse_init ( int b_num, VGBOARD *b);
static void mmi_250_kb_init ( int b_num, VGBOARD *b);

#if !defined(OS9)
static void mmi_250_video_isr (int board_index);
static void mmi_250_mouse_isr (int board_index);
static void mmi_250_kb_isr (int board_index);
#endif


#ifdef PSOS
static void mmi_250_video_init (int b_num, VGBOARD *b, void (*handler) (void));
static void mmi_250_mouse_isr_psos (void);
static void mmi_250_kb_isr_psos (void);
#else
static void mmi_250_video_init (int b_num,
				VGBOARD *b,
				void (*handler) (int b_num));
#endif

static int mmi_250_accel (VGBOARD *b, int delta);


/* The maximum number of MMI-250's that can operate in one system.
 * Note: If this number is changed, the number of ISR_STUB()'s must be changed
 */
#define	MAX_MMI_250	(8)

static VGBOARD *board_list[MAX_MMI_250];
static int board_list_init = 0;

/* Look closely, these are the actual functions! */
#ifdef PSOS
void mmi_250_isrstub0 (void){  mmi_250_video_isr (0);}
void mmi_250_isrstub1 (void){  mmi_250_video_isr (1);}
void mmi_250_isrstub2 (void){  mmi_250_video_isr (2);}
void mmi_250_isrstub3 (void){  mmi_250_video_isr (3);}
void mmi_250_isrstub4 (void){  mmi_250_video_isr (4);}
void mmi_250_isrstub5 (void){  mmi_250_video_isr (5);}
void mmi_250_isrstub6 (void){  mmi_250_video_isr (6);}
void mmi_250_isrstub7 (void){  mmi_250_video_isr (7);}

static void (*mmi_250_stub_list[]) (void) =
{
  mmi_250_isrstub0,
  mmi_250_isrstub1,
  mmi_250_isrstub2,
  mmi_250_isrstub3,
  mmi_250_isrstub4,
  mmi_250_isrstub5,
  mmi_250_isrstub6,
  mmi_250_isrstub7
};

#endif /* PSOS */

static PIXMAP mmi_250_default_pixmap =
  {
    NULL, VGL_ERR_NONE,		/* pixdata, error */
    NULL,			/* Free_flag,  board */
    NULL,			/* mouse_pixmap */
    CLIP_ON, 0, 0, 0, 0,	/* clip_flag, x_min, y_min, x_max, y_max */
    
    255, 0,			/* Forground & background */
    0, 0,
    {0},
    NULL,
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
static struct kb_mod_table mod_table[] =
{
  /* Code, State, Action, Set, Reset, LED Update  */
  {0x12, L_SHIFT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x12, L_SHIFT, MOD_RESET, PREFIX_F0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {0x59, R_SHIFT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x59, R_SHIFT, MOD_RESET, PREFIX_F0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},

  {0x12, EL_SHIFT, MOD_SET, PREFIX_E0, PREFIX_E1 | PREFIX_F0, 0},
  {0x12, EL_SHIFT, MOD_RESET, PREFIX_E0 | PREFIX_F0, PREFIX_E1, 0},
  {0x59, ER_SHIFT, MOD_SET, PREFIX_E0, PREFIX_E1 | PREFIX_F0, 0},
  {0x59, ER_SHIFT, MOD_RESET, PREFIX_E0 | PREFIX_F0, PREFIX_E1, 0},

  {0x14, L_CTRL, MOD_SET, 0, (PREFIX_Ex | PREFIX_F0 | E_ALL) & ~E_SHIFT, 0},
  {0x14, L_CTRL, MOD_RESET, PREFIX_F0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {0x14, R_CTRL, MOD_SET, PREFIX_E0, (PREFIX_E1|PREFIX_F0|E_ALL)&~E_SHIFT, 0},
  {0x14, R_CTRL, MOD_RESET, PREFIX_E0|PREFIX_F0,(PREFIX_E1|E_ALL)&~E_SHIFT, 0},
  {0x14, ER_CTRL, MOD_SET, PREFIX_E1, PREFIX_E0 | PREFIX_F0 | PAUSE_DOWN, 0},
  {0x14, ER_CTRL, MOD_RESET, PREFIX_E1 | PREFIX_F0, PREFIX_E0 | PAUSE_DOWN, 0},

  {0x11, L_ALT, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x11, L_ALT, MOD_RESET, PREFIX_F0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {0x11, R_ALT, MOD_SET, PREFIX_E0, (PREFIX_E1|PREFIX_F0|E_ALL) & ~E_SHIFT, 0},
  {0x11, R_ALT, MOD_RESET, PREFIX_E0|PREFIX_F0, (PREFIX_E1|E_ALL)&~E_SHIFT, 0},

  {0x77, NUM_LOCK, MOD_TOGGLE, 0, (NUM_LOCK_DOWN|PREFIX_ALL|E_ALL)&~E_SHIFT,1},
  {0x77, NUM_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x77, NUM_LOCK_DOWN, MOD_RESET, PREFIX_F0,
     (PREFIX_Ex | PREFIX_E114 | E_ALL) & ~E_SHIFT, 0},

  {0x7E, SCRL_LOCK, MOD_TOGGLE,0,(SCRL_LOCK_DOWN|PREFIX_ALL|E_ALL)&~E_SHIFT,1},
  {0x7E, SCRL_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x7E, SCRL_LOCK_DOWN, MOD_RESET, PREFIX_F0, (PREFIX_Ex|E_ALL)&~E_SHIFT, 0},

  {0x58, CAPS_LOCK, MOD_TOGGLE,0,(CAPS_LOCK_DOWN|PREFIX_ALL|E_ALL)&~E_SHIFT,1},
  {0x58, CAPS_LOCK_DOWN, MOD_SET, 0, (PREFIX_ALL | E_ALL) & ~E_SHIFT, 0},
  {0x58, CAPS_LOCK_DOWN, MOD_RESET, PREFIX_F0, (PREFIX_Ex|E_ALL)&~E_SHIFT, 0},
};

#define NUM_MOD_TABLE	(sizeof(mod_table)/sizeof(mod_table[0]))

static struct kb_key_table key_table[] =
{
	  /* Key#  Scancode, Set,                    Reset,                                         One_of */
  {1, 0x76, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {2, 0x05, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {3, 0x06, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {4, 0x04, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {5, 0x0C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {6, 0x03, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {7, 0x0B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {8, 0x83, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {9, 0x0A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {10, 0x01, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {11, 0x09, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {12, 0x78, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {13, 0x07, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {14, 0x7C, PREFIX_E0 | EL_SHIFT, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {14, 0x7C, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {14, 0x84, 0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {15, 0x7E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},

  {16, 0x7E, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~ER_CTRL, 0},
  {16, 0x77, PREFIX_E114, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},

  {17, 0x0E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {18, 0x16, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {19, 0x1E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {20, 0x26, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {21, 0x25, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {22, 0x2E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {23, 0x36, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {24, 0x3D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {25, 0x3E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {26, 0x46, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {27, 0x45, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {28, 0x4E, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {29, 0x55, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {30, 0x66, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {31, 0x70, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {32, 0x6C, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {33, 0x7D, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {34, 0x77, 0, (PREFIX_Ex | E_ALL | PREFIX_E114) & ~E_SHIFT, 0},
  {35, 0x4A, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {36, 0x7C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {37, 0x7B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},

  {38, 0x0D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {39, 0x15, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {40, 0x1D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {41, 0x24, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {42, 0x2D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {43, 0x2C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {44, 0x35, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {45, 0x3C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {46, 0x43, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {47, 0x44, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {48, 0x4D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {49, 0x54, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {50, 0x5B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {51, 0x5D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {52, 0x71, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {53, 0x69, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {54, 0x7A, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {55, 0x6C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {56, 0x75, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {57, 0x7D, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {58, 0x79, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},

  {59, 0x58, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {60, 0x1C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {61, 0x1B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {62, 0x23, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {63, 0x2B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {64, 0x34, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {65, 0x33, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {66, 0x3B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {67, 0x42, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {68, 0x4B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {69, 0x4C, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {70, 0x52, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {71, 0x5A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {72, 0x6B, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {73, 0x73, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {74, 0x74, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},

  {75, 0x12, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {76, 0x1A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {77, 0x22, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {78, 0x21, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {79, 0x2A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {80, 0x32, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {81, 0x31, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {82, 0x3A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {83, 0x41, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {84, 0x49, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {85, 0x4A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {86, 0x59, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {87, 0x75, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {88, 0x69, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {89, 0x72, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {90, 0x7A, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {91, 0x5A, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},

  {92, 0x14, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {93, 0x11, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {94, 0x29, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {95, 0x11, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {96, 0x14, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {97, 0x6B, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {98, 0x72, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {99, 0x74, PREFIX_E0, (PREFIX_E1 | E_ALL) & ~E_SHIFT, 0},
  {100, 0x70, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0},
  {101, 0x71, 0, (PREFIX_Ex | E_ALL) & ~E_SHIFT, 0}
};

#define NUM_KEY_TABLE	(sizeof(key_table)/sizeof(key_table[0]))
#endif


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_board_init (VGBOARD *b)
{
  int i, j;

  /***** Initalize the board list, if required *****/
  if (board_list_init == 0)
    {
      for (i = 0; i < MAX_MMI_250; i++)
	board_list[i] = NULL;

      board_list_init = 1;
    }

  /***** Find an empty board slot *****/
  for (i = 0; i < MAX_MMI_250; i++)
    {
      if (board_list[i] == NULL)
	break;
    }

  /***** Set error code and return, on error *****/
  if (i >= MAX_MMI_250)
    {
      b->error = VGL_ERR_MAX_BOARDS;
      return;
    }

  /***** Get permission to access the board *****/
  if (vgl_board_permit (b))
    {
      b->error = VGL_ERR_VME_PERMIT;
      return;
    }

  /* Determine size of board */
  b->regaddr = ((char *) b->addr) + 0x1F0000;
  b->mem_size = 2*1024*1024;
  j = vgl_get_bytereg (b, DUART_SRA);
  vgl_set_bytereg (b, DUART_CSRA, 0xAA);
  j = vgl_get_bytereg (b, DUART_SRA);
  if (j == 0xAA)
    {				/* _MIGHT_ be a four meg board */
      vgl_set_bytereg (b, DUART_CSRA, 0x17);
      j = vgl_get_bytereg (b, DUART_SRA);
      if (j == 0x17)
	{			/* it is a FOUR meg board */
	  b->regaddr = ((char *) b->addr) + 0x3F0000;
	  b->mem_size = 4*1024*1024;
	}
      /* Else it is a two meg board */
    }
  /* Else it is a two meg board */


  /***** Add new board to board list *****/
  board_list[i] = b;

  /***** Clear irq masks *****/
  vgl_set_bytereg (b, VMIMR, 0);

  /***** Set irq_vector *****/
  vgl_set_bytereg (b, VMIVR, b->irq_vector);

  /***** Initalize board sub-systems *****/
#if defined (PSOS)
  mmi_250_video_init (i, b, mmi_250_stub_list[i]);
#elif defined (VXWORKS)
  mmi_250_video_init (i, b, mmi_250_video_isr);
#elif defined (OS9)
  mmi_250_video_init (i, b, NULL);
#endif
  mmi_250_mouse_init (i, b);
  mmi_250_kb_init (i, b);

  /***** Set irq masks *****/
  vgl_set_bytereg (b, VMIMR, (1 << 2) | (1 << 1) | (1 << 0));
}


/*****************************************************************************/
/*****************************************************************************/
#ifdef PSOS
static void 
mmi_250_video_init (int b_num, VGBOARD *b, void (*handler) (void))
#else
static void 
mmi_250_video_init (int b_num, VGBOARD *b, void (*handler) (int param))
#endif
{
  int i;
  long *j;

  vgl_set_bytereg (b, DUART_OUT_RESET, GRST);
  vgl_sleep ((1 * VGL_CLOCK) / 60);
  vgl_set_bytereg (b, DUART_OUT_SET, GRST);
  vgl_sleep ((1 * VGL_CLOCK) / 60);

  vgl_set_reg (b, G3BL, b->mode->mmi_250.bl);
  vgl_sleep ((1 * VGL_CLOCK) / 60);
  vgl_set_reg (b, G3CR, b->mode->mmi_250.cr & 0xFFFFFFFE);

  vgl_set_reg (b, G3HS, b->mode->mmi_250.hs);
  vgl_set_reg (b, G3BP, b->mode->mmi_250.bp);
  vgl_set_reg (b, G3HD, b->mode->mmi_250.hd);
  vgl_set_reg (b, G3SD, b->mode->mmi_250.sd);
  vgl_set_reg (b, G3BR, b->mode->mmi_250.br);
  vgl_set_reg (b, G3VS, b->mode->mmi_250.vs);
  vgl_set_reg (b, G3VB, b->mode->mmi_250.vb);
  vgl_set_reg (b, G3VD, b->mode->mmi_250.vd);
  vgl_set_reg (b, G3LT, b->mode->mmi_250.lt);
  vgl_set_reg (b, G3LS, b->mode->mmi_250.ls);
  vgl_set_reg (b, G3MI, b->mode->mmi_250.mi);
  vgl_set_reg (b, G3MR, 0xFF);
  vgl_set_reg (b, G3TD, b->mode->mmi_250.td);

  b->tos_reset = 0;
  if (b->mem_size == (2*1024*1024) )
    b->tos_value = 2*1024*1024;
  else
    b->tos_value = 0;

  vgl_set_reg (b, G3TS, b->tos_value);	/* Set Top Of Screen Reg */


  b->color_end = 0;
  b->color_start = 0;

  j = (long *) (((char *) b->regaddr) + CLUT);
  for (i = 0; i < 256; i++)
    j[i] = i | (i << 8) | (i << 16);

  vgl_setrgb (b, 0, 256, vgl_default_clut);

  if (b->mode->mmi_250.hs_pol < 0)
    vgl_set_bytereg (b, DUART_OUT_SET, HPOL);
  else
    vgl_set_bytereg (b, DUART_OUT_RESET, HPOL);

  if (b->mode->mmi_250.vs_pol < 0)
    vgl_set_bytereg (b, DUART_OUT_SET, VPOL);
  else
    vgl_set_bytereg (b, DUART_OUT_RESET, VPOL);

  vgl_set_reg (b, G3CR, (b->mode->mmi_250.cr & 0xFFFFFFFE) | 1);

#if !defined(OS9)
  vgl_install_isr (b->irq_vector, handler, b_num);
#endif

  vgl_set_bytereg (b, DUART_OUT_SET, VBLK);
}

/*****************************************************************************/
/*****************************************************************************/
#if !defined(OS9)
static void 
mmi_250_video_isr (int board_index)
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
      /* Set Top Of Screen Reg */
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
      clut = (unsigned long *) ((char *) (b->regaddr) + CLUT);

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
		    b->mouse_old_x, 
		    b->mouse_old_y,
		    vgl_x_res (b->mouse_under), 
		    vgl_y_res (b->mouse_under),
		    b->mouse_under,
		    0, 0);

      if (b->cur_pixmap->mouse_pixmap != NULL && b->mouse_ptr != NULL)
	vgl_transbitblt (b->mouse_ptr,
			 0, 0, -1, -1,
			 b->cur_pixmap->mouse_pixmap, 
			 b->mouse_old_x,
			 b->mouse_old_y,
			 0);

      b->mouse_visible_flag = 1;
    }

  b->mouse_update_flag = 0;
  b->frame_count++;

  i=vgl_get_bytereg (b, G3SR);
  i++;
  vgl_set_bytereg (b, G3SR, i);
}
#endif


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_board_kill (VGBOARD *b)
{
  int i;

  /***** Set irq masks *****/
  vgl_set_bytereg (b, VMIMR, 0);

  if (b->mouse_under != NULL)
    vgl_freepixmap (b->mouse_under);

  if (b->cur_pixmap->mouse_pixmap != NULL)
    vgl_freepixmap (b->cur_pixmap->mouse_pixmap);

  for (i = 0; i < MAX_MMI_250; i++)
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
mmi_250_pixmap_map (VGBOARD *b, int pagenum)
{
  PIXMAP *p;

  p = vgl_malloc (sizeof (PIXMAP));
  if (p)
    {
      *p = mmi_250_default_pixmap;
      p->pixdata = vgl_malloc (sizeof (p->pixdata[0]));
      if (p->pixdata == NULL)
	{
	  vgl_free (p);
	  return (NULL);
	}

      p->pixdata->x_res = b->mode->x_res;
      p->pixdata->y_res = b->mode->y_res;
      p->pixdata->x_size = b->mode->mmi_250.x_size;
      p->pixdata->users = 1;
      p->pixdata->free_flag = 0;
      p->pixdata->data = 
	((char *) b->addr) + (b->mode->mmi_250.page_size * pagenum);

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
	{ /* An inline, modified verson of: mmi_250_pixmap_disp( p); */
	  b->mouse_blanking_level = 1;
	  b->mouse_visible_flag = 0;

	  b->cur_pixmap = p;
	  b->next_cur_pixmap = p;

	  b->tos_value = (((char *) p->pixdata->data) -
			  ((char *) p->board->addr)) & 
			    0xFFFFFFFC;
	  
	  if (b->mem_size == (2*1024*1024) )
	    b->tos_value += 2*1024*1024;
	  
/*
	  b->tos_value = ((((char *) p->pixdata->data) - 
			   ((char *) p->board->addr)) & 0xFFFFFFFC);
*/
	  b->tos_reset = 1;
	  b->mouse_update_flag = 1;
	}

      return (p);
    }
  else
    return (NULL);
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_pixmap_disp (PIXMAP * p)
{
  unsigned long temp;


  if (p->board != NULL)
    {
      temp = vgl_mask_irq (p->board);

      p->board->tos_value = (((char *) p->pixdata->data) -
			     ((char *) p->board->addr)) & 
			       0xFFFFFFFC;

      if (p->board->mem_size == (2*1024*1024) )
	p->board->tos_value += 2*1024*1024;

      p->board->next_cur_pixmap = p;

      p->board->tos_reset = 1;
      p->board->mouse_update_flag = 1;

      vgl_unmask_irq (p->board, temp);
    }
  else
    p->error = VGL_ERR_BADOP;
}

/*****************************************************************************/
/*****************************************************************************/
int 
mmi_250_boarderror (VGBOARD *b)
{
  int err;

  err = b->error;
  b->error = VGL_ERR_NONE;
  return (err);
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_setrgb (VGBOARD *b, 
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


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_getrgb (VGBOARD *b,
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

/*****************************************************************************/
/*****************************************************************************/
static void 
mmi_250_mouse_init (int b_num, VGBOARD *b)
{
#ifdef PSOS
  vgl_install_isr (b->irq_vector + 1, mmi_250_mouse_isr_psos, b_num);
#elif !defined(OS9)
  vgl_install_isr (b->irq_vector + 1, mmi_250_mouse_isr, b_num);
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


/*****************************************************************************/
/*****************************************************************************/
#ifdef PSOS
static void
mmi_250_mouse_isr_psos( void)
{
  int	i;

  for( i=0; i<MAX_MMI_250; i++)
    mmi_250_mouse_isr( i);
}
#endif


#if !defined(OS9)
static void 
mmi_250_mouse_isr (int j)
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

	  if (bd->mouse_buf.in_start != 
	      (bd->mouse_buf.in_end + 1) % MOUSE_BUF_SIZE && !(err & 0xF0))
	    {
	      bd->mouse_buf.err_in[bd->mouse_buf.in_end] = err & 0xF3;
	      bd->mouse_buf.in[bd->mouse_buf.in_end] = ch;
	      bd->mouse_buf.in_end = (bd->mouse_buf.in_end+1) % MOUSE_BUF_SIZE;
	    }

	  if ((err & 0x10) != 0) /* Reset Overrun error, if needed */
	    vgl_set_bytereg (bd, DUART_CRA, 0x40);

	  err = vgl_get_bytereg (bd, DUART_SRA);
	}

      /* Call mouse handler */
      do
	{
	  /*  Is there a hold over from the previous packet.
	   *  IE, middle button? 
	   */
	  if (bd->mouse_buf.in_start!=bd->mouse_buf.in_end && 
	      bd->mouse_sync==3)
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
		(((bd->mouse_oa & 0x20) ? MOUSE_LEFT : 0x00) |
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
	      (bd->mouse_buf.in_end + MOUSE_BUF_SIZE) - bd->mouse_buf.in_start;


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

	      bd->mouse_x += mmi_250_accel (bd, dx);
	      if (bd->mouse_x >= bd->mouse_max_x)
		bd->mouse_x = bd->mouse_max_x - 1;
	      else if (bd->mouse_x < 0)
		bd->mouse_x = 0;

	      bd->mouse_y += mmi_250_accel (bd, dy);
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
    } /* If board is valid */
}
#endif


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_mouse_getcoords (VGBOARD *b, int *x, int *y, int *buttons, int wait_flag)
{
  unsigned long old_mode;

  if (wait_flag)
    vgl_mouse_dequeue (b, x, y, buttons, 1);
  else
    {
      if (vgl_mouse_dequeue (b, x, y, buttons, 0) != 0)
	{
	  old_mode = vgl_mask_irq (b);

	  *x = b->mouse_x / b->mouse_accel_base;
	  *y = b->mouse_y / b->mouse_accel_base;
	  *buttons = b->mouse_buttons;

	  vgl_unmask_irq (b, old_mode);
	}
    }
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_mouse_setcoords (VGBOARD *b, int x, int y)
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


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_mouse_sethandler (VGBOARD *b, void (*fctn) (int x, int y, int buttons))
{
}


/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_mouse_setpixmap (VGBOARD *b, PIXMAP *pixmap, int hot_x, int hot_y)
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
mmi_250_mouse_ptr_on (VGBOARD *b)
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
mmi_250_mouse_ptr_off (VGBOARD *b)
{
  unsigned long irq_flags;

  irq_flags = vgl_mask_irq (b);

  if (b->mouse_visible_flag == 1)
    {
#if !defined (OS9)
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
mmi_250_mouse_setaccel (VGBOARD *b, 
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
#if !defined(OS9)
static int 
mmi_250_accel (VGBOARD *b, int delta)
{
  if (delta > 0)
    {
      if (delta >= b->mouse_accel_thresh)
	return (((delta - b->mouse_accel_thresh) * 
		 b->mouse_accel_num / b->mouse_accel_den) + 
		b->mouse_accel_thresh);
      else
	return (delta);
    }
  else
    {
      if (-delta >= b->mouse_accel_thresh)
	return (((delta + b->mouse_accel_thresh) * 
		 b->mouse_accel_num / b->mouse_accel_den) - 
		b->mouse_accel_thresh);
      else
	return (delta);
    }
}
#endif

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_ss_set (VGBOARD *b, short status, long timeout, short flags)
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
mmi_250_ss_tick (VGBOARD *b, short flags)
{
  if (b->ss_status == SS_NORMAL && (b->ss_flags & flags) != 0)
    b->ss_counter = b->ss_timeout_value;
}


/*****************************************************************************/
/*****************************************************************************/
static void 
mmi_250_kb_init (int b_num, VGBOARD *b)
{
  unsigned long j;

#ifdef PSOS
  vgl_install_isr (b->irq_vector + 2, mmi_250_kb_isr_psos, b_num);
#elif !defined(OS9)
  vgl_install_isr (b->irq_vector + 2, mmi_250_kb_isr, b_num);
#endif

  for (j = 0; j<KB_TIMEOUT && (vgl_get_bytereg (b, KSR) & KB_KIBF) != 0; j++);
  vgl_set_bytereg (b, KCOM, KB_INIT);

  for (j = 0; j<KB_TIMEOUT && (vgl_get_bytereg (b, KSR) & KB_KOBF) == 0; j++);
  if (vgl_get_bytereg (b, KOUT) == 0x55)
    {
      for (j=0; j<KB_TIMEOUT && (vgl_get_bytereg (b, KSR) & KB_KIBF)!=0; j++);
      vgl_set_bytereg (b, KCOM, KB_WRITE_CONFIG);

      for (j=0; j<KB_TIMEOUT && (vgl_get_bytereg (b, KSR) & KB_KIBF)!=0; j++);
      vgl_set_bytereg (b, KDAT, KB_BASE | KB_EINT);
    }
}


/*****************************************************************************/
/*****************************************************************************/
#ifdef PSOS
static void
mmi_250_kb_isr_psos( void)
{
int	i;

for( i=0; i<MAX_MMI_250; i++)
  {
    mmi_250_kb_isr( i);
  }
}
#endif

#if !defined(OS9)
static void 
mmi_250_kb_isr (int j)
{
  int i, c, led_flag;
  VGBOARD *b;

  if (board_list[j] != NULL)
    {
      b = board_list[j];

      /* There is a scancode ready */
      while ((vgl_get_bytereg (b, KSR) & KB_KOBF) != 0)
	{
	  c = vgl_get_bytereg (b, KOUT);

	  if (c == 0xE0)
	    b->kb_state |= PREFIX_E0;
	  else if (c == 0xE1)
	    b->kb_state |= PREFIX_E1;
	  else if (c == 0xF0)
	    b->kb_state |= PREFIX_F0;
	  else if (c == 0x14 && (b->kb_state & PREFIX_E1) != 0)
	    {
	      b->kb_state |= PREFIX_E114;
	      b->kb_state &= ~PREFIX_E1;
	    }
	  else
	    {
	      led_flag = 0;

	      /* Decode state modifiers */
	      for (i = 0; i < NUM_MOD_TABLE; i++)
		{
		  if (c == mod_table[i].keycode &&
		      (b->kb_state & mod_table[i].set) == mod_table[i].set &&
		      (b->kb_state & mod_table[i].reset) == 0)
		    {
		      led_flag = led_flag || mod_table[i].led_update;

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

		      /* Don't "break;" here, there can be more than one
			 modifier per scancode */
		    } /* End if, scancode is a match */
		} /* End check for mod codes */
	     /*
	      if( led_flag!=0)
		{
		  b->kb_buf_out[b->kb_end_out] = KB_SET_LED;
		  b->kb_end_out = (b->kb_end_out+1)%KB_BUF_OUT_SIZE;
		  b->kb_buf_out[b->kb_end_out] = 
		    (((b->kb_state & SCRL_LOCK)?(1<<0):0) | 
		     ((b->kb_state &  NUM_LOCK)?(1<<1):0) | 
		     ((b->kb_state & CAPS_LOCK)?(1<<2):0));
		  b->kb_end_out = (b->kb_end_out+1)%KB_BUF_OUT_SIZE;
		}
	      */
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
	} /* End, if scancode is waiting */

      /* The keyboard is awaiting input (Send something to the keyboard) */
      while ((vgl_get_bytereg (b, KSR) & KB_KIBF) == 0 && 
	     b->kb_start_out != b->kb_end_out)
	{
	  vgl_set_bytereg (b, KDAT, b->kb_buf_out[b->kb_start_out]);
	  b->kb_start_out = (b->kb_start_out + 1) % KB_BUF_OUT_SIZE;
	}

    } /* End, if valid board */
}
#endif


/*****************************************************************************/
/*****************************************************************************/
int 
mmi_250_kbhit (VGBOARD *b)
{
  return (vgl_kb_queue_check (b));
}


/*****************************************************************************/
/*****************************************************************************/
long 
mmi_250_getkey (VGBOARD *b, int wait_flag)
{
  unsigned long c;

  if (vgl_kb_dequeue (b, &c, wait_flag) == 0)
    return (c);
  else
    return (0);
}

/*****************************************************************************/
/*****************************************************************************/
void 
mmi_250_kb_setled (VGBOARD *b, unsigned long led_flags)
{
/*
   unsigned long temp;

   temp = vgl_mask_irq(b);

   b->kb_buf_out[b->kb_end_out] = KB_SET_LED;
   b->kb_end_out = (b->kb_end_out+1)%KB_BUF_OUT_SIZE;
   b->kb_buf_out[b->kb_end_out] = ((led_flags & SCRL_LOCK)?(1<<0):0) | 
   ((led_flags &  NUM_LOCK)?(1<<1):0) | 
   ((led_flags & CAPS_LOCK)?(1<<2):0);
   b->kb_end_out = (b->kb_end_out+1)%KB_BUF_OUT_SIZE;

   vgl_unmask_irq (b, temp);
 */
}

/****************************************************************************/
/****************************************************************************/
int
mmi_250_test (VGBOARD *b, char *msg)
{
  strcpy (msg, "Hardware test of MMI-250 unsupported.");
  return (VGL_TEST_UNSUPPORTED);
}


#endif /* SUPPORT_MMI_250 */
