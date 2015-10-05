
#ifndef	_MMI_150_H_
#define	_MMI_150_H_

#define MMI_150

#include "vgl.h"
#include "vgl_internals.h"



/*****************************************************************************/
/*****************************************************************************/
#define	vgl_set_reg(BOARD,REG,VAL)	\
    (*((volatile unsigned long *)(((char *)((BOARD)->addr))+(REG)))=(VAL))

#define	vgl_get_reg(BOARD,REG)		\
    (*((volatile long *)(((char *)((BOARD)->addr))+(REG))))

#define	vgl_get_bytereg(BOARD,REG)	\
    (*(((volatile unsigned char *)((BOARD)->addr))+(REG)))

#define	vgl_set_bytereg(BOARD,REG,VAL)	\
    (*(((volatile unsigned char *)((BOARD)->addr))+(REG))=(VAL))


/* VME Interrupt registers */
#define	VIMR	(0xFF043)
#define	VIVR	(0xFF083)

/* G300 register offsets.   Add to mmi_base to get 'real' address */
#define	CLUT	(0xFF800)
#define	G3BC	(0xFFC00)
#define G3HS	(0xFFC84)
#define	G3BP	(0xFFC88)
#define	G3HD	(0xFFC8C)
#define	G3SD	(0xFFC90)
#define	G3BR	(0xFFC94)
#define	G3VS	(0xFFC98)
#define	G3VB	(0xFFC9C)
#define	G3VD	(0xFFCA0)
#define	G3LT	(0xFFCA4)
#define	G3LS	(0xFFCA8)
#define	G3MI	(0xFFCAC)
#define	G3TD	(0xFFCB0)
#define	G3HI	(0xFFCB4)
#define	G3VI	(0xFFCB8)
#define	G3TBI	(0xFFCBC)
#define	G3MR	(0xFFD00)
#define	G3CR	(0xFFD80)
#define	G3TS	(0xFFE00)
#define	G3BL	(0xFFE80)
#define	G3CLGI	(0xFF000)


/* DUART offset from mmi_base */
enum DUART_OUTPORT
  {
    GLED = 128, RLED = 64, DSPB = 32, SYNP = 16, 
    GRST =   8, DRST =  4, RTSB =  2, RTSA =  1
  };

/* Read regs */
#define	DUART_MRA	(0xFF103)
#define	DUART_SRA	(0xFF107)
#define	DUART_RES1	(0xFF10B)
#define	DUART_RHRA	(0xFF10F)
#define	DUART_IPCR	(0xFF113)
#define	DUART_ISR	(0xFF117)
#define	DUART_CTU	(0xFF11B)
#define	DUART_CTL	(0xFF11F)
#define	DUART_MRB	(0xFF123)
#define	DUART_SRB	(0xFF127)
#define	DUART_RES2	(0xFF12B)
#define	DUART_RHRB	(0xFF12F)
#define	DUART_IVR	(0xFF133)
#define	DUART_IP	(0xFF137)
#define	DUART_STCC	(0xFF13B)
#define	DUART_SPCC	(0xFF13F)

/* Write regs */
/*#define       DUART_MRA       (0xFF103) */
#define	DUART_CSRA	(0xFF107)
#define	DUART_CRA	(0xFF10B)
#define	DUART_THRA	(0xFF10F)
#define	DUART_ACR	(0xFF113)
#define	DUART_IMR	(0xFF117)
#define	DUART_CRUR	(0xFF11B)
#define	DUART_CTLR	(0xFF11F)
/*#define       DUART_MRB       (0xFF123) */
#define	DUART_CSRB	(0xFF127)
#define	DUART_CRB	(0xFF12B)
#define	DUART_THRB	(0xFF12F)
/*#define       DUART_IVR       (0xFF133) */
#define	DUART_OPCR	(0xFF137)
#define	DUART_OUT_SET	(0xFF13B)
#define	DUART_OUT_RESET	(0xFF13F)

enum
  {
    MOUSE_MICROSOFT = 3, 
    MOUSE_LOGITECH = 2, 
    MOUSE_MOUSEMAN = 1, 
    MOUSE_TEST = 0
  };


/* Keyboard register offsets */
#define	KEYB	(0xFF203)
#define	KSR	(0xFF207)

#define KINT	(1<<0)

/* Keyboard states/modes */
/*
   #define L_SHIFT              (1<<14)
   #define R_SHIFT              (1<<15)
   #define L_CTRL               (1<<18)
   #define R_CTRL               (1<<19)
   #define L_ALT                (1<<22)
   #define R_ALT                (1<<23)
   #define NUM_LOCK     (1<<26)
   #define CAPS_LOCK    (1<<27)
   #define SCRL_LOCK    (1<<28)
 */
#define PREFIX_E11D	(1<<9)
#define PAUSE_DOWN	(1<<10)
#define SCRL_LOCK_DOWN	(1<<11)
#define NUM_LOCK_DOWN	(1<<12)
#define CAPS_LOCK_DOWN	(1<<13)
#define EL_SHIFT	(1<<16)
#define ER_SHIFT	(1<<17)
#define EL_CTRL		(1<<20)
#define ER_CTRL		(1<<21)
#define EL_ALT		(1<<24)
#define ER_ALT		(1<<25)
#define PREFIX_E0	(1<<29)
#define PREFIX_E1	(1<<30)

#define KUP		KEY_UP

#define PREFIX_ALL	( PREFIX_E0 | PREFIX_E1 | PREFIX_E11D)
#define PREFIX_Ex	( PREFIX_E0 | PREFIX_E1 )
#define	E_SHIFT		( EL_SHIFT | ER_SHIFT)
#define E_ALT		( EL_ALT | ER_ALT)
#define E_CTRL		( EL_CTRL | ER_CTRL)
#define E_ALL		( E_SHIFT | E_ALT | E_CTRL)

/* Modes/states/flags that are set/reset after each key */
#define SINGLE_KEY_SET		(0)
#define SINGLE_KEY_RESET	(PREFIX_ALL)

enum kb_mod_action
  {
    MOD_SET, MOD_RESET, MOD_TOGGLE
  };

struct kb_mod_table
  {
    int keycode;
    unsigned long state;
    short action;
    unsigned long set;
    unsigned long reset;
  };

struct kb_key_table
  {
    int key_num;
    int keycode;
    unsigned long set;
    unsigned long reset;
    unsigned long one_of;
  };

#endif
