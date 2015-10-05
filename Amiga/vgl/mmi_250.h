#ifndef	_MMI_250_H_
#define	_MMI_250_H_

#include "vgl.h"
#include "vgl_internals.h"

/*****************************************************************************/
/*****************************************************************************/
#define	vgl_set_reg(BOARD,REG,VAL)	\
(*((unsigned long *)(((char *)((BOARD)->regaddr))+(REG)))=(VAL))

#define	vgl_get_reg(BOARD,REG)		\
(*((unsigned long *)(((char *)((BOARD)->regaddr))+(REG))))

#define	vgl_get_bytereg(BOARD,REG)	\
(*(((unsigned char *)((BOARD)->regaddr))+(REG)))

#define	vgl_set_bytereg(BOARD,REG,VAL)	\
(*(((unsigned char *)((BOARD)->regaddr))+(REG))=(VAL))


/* VME Interrupt registers */
#define	VMIMR	(0xF803)
#define	VMIVR	(0xF883)
#define VSIMR	(0xF903)
#define VSISR	(0xF983)

/* G300 register offsets.   Add to mmi_base to get 'real' address */
#define	CLUT	(0xE000)
#define	G3BC	(0xE400)
#define G3HS	(0xE484)
#define	G3BP	(0xE488)
#define	G3HD	(0xE48C)
#define	G3SD	(0xE490)
#define	G3BR	(0xE494)
#define	G3VS	(0xE498)
#define	G3VB	(0xE49C)
#define	G3VD	(0xE4A0)
#define	G3LT	(0xE4A4)
#define	G3LS	(0xE4A8)
#define	G3MI	(0xE4AC)
#define	G3TD	(0xE4B0)

#define	G3HI	(0xE4B4)
#define	G3VI	(0xE4B8)
#define	G3TBI	(0xE4BC)

#define	G3MR	(0xE500)
#define	G3CR	(0xE580)
#define	G3TS	(0xE600)
#define	G3BL	(0xE680)
#define G3SR	(0xFA03)


/* DUART offset from mmi_base */
enum DUART_OUTPORT
  {
    RES2 = 128, RES1 = 64, VPOL = 32, HPOL = 16, 
    GRST =   8, VBLK =  4, RTSB =  2, RTSA =  1
  };

/* Read regs */
#define	DUART_MRA	(0xF403)
#define	DUART_SRA	(0xF407)
#define	DUART_RES1	(0xF40B)
#define	DUART_RHRA	(0xF40F)
#define	DUART_IPCR	(0xF413)
#define	DUART_ISR	(0xF417)
#define	DUART_CTU	(0xF41B)
#define	DUART_CTL	(0xF41F)
#define	DUART_MRB	(0xF423)
#define	DUART_SRB	(0xF427)
#define	DUART_RES2	(0xF42B)
#define	DUART_RHRB	(0xF42F)
#define	DUART_IVR	(0xF433)
#define	DUART_IP	(0xF437)
#define	DUART_STCC	(0xF43B)
#define	DUART_SPCC	(0xF43F)

/* Write regs */
/*#define       DUART_MRA       (0xF4103) */
#define	DUART_CSRA	(0xF407)
#define	DUART_CRA	(0xF40B)
#define	DUART_THRA	(0xF40F)
#define	DUART_ACR	(0xF413)
#define	DUART_IMR	(0xF417)
#define	DUART_CRUR	(0xF41B)
#define	DUART_CTLR	(0xF41F)
/*#define       DUART_MRB       (0xF423) */
#define	DUART_CSRB	(0xF427)
#define	DUART_CRB	(0xF42B)
#define	DUART_THRB	(0xF42F)
/*#define       DUART_IVR       (0xF433) */
#define	DUART_OPCR	(0xF437)
#define	DUART_OUT_SET	(0xF43B)
#define	DUART_OUT_RESET	(0xF43F)

enum
  {
    MOUSE_MICROSOFT = 3, 
    MOUSE_LOGITECH = 2, 
    MOUSE_MOUSEMAN = 1, 
    MOUSE_TEST = 0
  };

/* Keyboard register offsets */
#define	KOUT	(0xF003)
#define	KSR	(0xF007)
#define	KDAT	(0xF003)
#define	KCOM	(0xF007)

/* Keyboard Command Bytes */
#define	KB_READ_CONFIG		(0x20)
#define	KB_WRITE_CONFIG		(0x60)
#define	KB_INIT			(0xAA)
#define	KB_TEST			(0xAB)
#define	KB_OFF			(0xAD)
#define KB_ON			(0xAE)
#define KB_READ_J11		(0xC0)

#define KB_SET_LED		(0xED)

/* 8042 Configuration Byte Defines */
#define	KB_CVRT		(1<<6)
#define	KB_PCXT		(1<<5)
#define	KB_KOFF		(1<<4)
#define	KB_EINT		(1<<0)
#define KB_BASE		(0x08)

/* 8042 Status Register Defines */
#define KB_PERR		(1<<7)
#define	KB_RXTO		(1<<6)
#define	KB_TXTO		(1<<5)
#define	KB_KIBF		(1<<1)
#define	KB_KOBF		(1<<0)

#define	KB_TIMEOUT	(1000000)

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
#define PREFIX_E114	(1<<9)
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
#define PREFIX_F0	KEY_UP

#define PREFIX_ALL	( PREFIX_E0 | PREFIX_E1 | PREFIX_F0 | PREFIX_E114)
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
    unsigned long led_update;
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
