
#ifndef	_DB_180_H_
#define	_DB_180_H_

#include "vgl.h"
#include "vgl_internals.h"

/*  Defined in vgl.h
typedef unsigned long ulong;
*/

/*****************************************************************************/
/*****************************************************************************/
typedef struct
  {
    ulong	addr;
    ulong	mainlut;
    ulong	reg;
    ulong	overlaylut;
    ulong	rsv;
    ulong	config;
    ulong	ctrl;
    ulong	curlut;
  } BT445;


/*
   Note:  The names of the P9000 regs are (mostly) consistant with the
   Weitek P9000 manual.
*/
typedef struct
  {
    char             __unused[4];

    /* System Control Regs */
    unsigned long    sysconfig;                     /* 0x00100004-0x00100007 */
    unsigned long    interrupt_sr;                  /* 0x00100008-0x0010000B */
    unsigned long    interrupt_en;                  /* 0x0010000C-0x0010000F */
    char             __unused000[0xF4];             /* 0x00100010-0x00100103 */

    /* Video Control Register */
    unsigned long    hrzc;                          /* 0x00100104-0x00100107 */
    unsigned long    hrzt;                          /* 0x00100108-0x0010010B */
    unsigned long    hrzsr;                         /* 0x0010010C-0x0010010F */
    unsigned long    hrzbr;                         /* 0x00100110-0x00100113 */
    unsigned long    hrzbf;                         /* 0x00100114-0x00100117 */
    unsigned long    prehrzc;                       /* 0x00100118-0x0010011B */
    unsigned long    vrtc;                          /* 0x0010011C-0x0010011F */
    unsigned long    vrtt;                          /* 0x00100120-0x00100123 */
    unsigned long    vrtsr;                         /* 0x00100124-0x00100127 */
    unsigned long    vrtbr;                         /* 0x00100128-0x0010012B */
    unsigned long    vrtbf;                         /* 0x0010012C-0x0010012F */
    unsigned long    prevrtc;                       /* 0x00100130-0x00100133 */
    unsigned long    sraddr;                        /* 0x00100134-0x00100137 */
    unsigned long    srtctl;                        /* 0x00100138-0x0010013B */
    unsigned long    sraddr_inc;                    /* 0x0010013C-0x0010013F */
    char             __unused001[0x44];             /* 0x00100140-0x00100183 */

    /* VRAM Control Registers */
    unsigned long    mem_config;                    /* 0x00100184-0x00100187 */
    unsigned long    rfperiod;                      /* 0x00100188-0x0010018B */
    unsigned long    rfcount;                       /* 0x0010018C-0x0010018F */
    unsigned long    rlmax;                         /* 0x00100190-0x00100193 */
    unsigned long    rlcur;                         /* 0x00100194-0x00100197 */
    char             __unused002[0x68];             /* 0x00100198-0x001001FF */

    /* RAMDAC */
    BT445            bt445;                         /* 0x00100200-0x0010021F */
    char             __unused003[0x7FDE0];          /* 0x00100220-0x0017FFFF */

    /* Parameter Engine Registers */
    unsigned long    status;                        /* 0x00180000-0x00180003 */

    /* Commands */
    unsigned long    blit_command;                  /* 0x00180004-0x00180007 */
    unsigned long    quad_command;                  /* 0x00180008-0x0018000B */
    unsigned long    pixel8_command;                /* 0x0018000C-0x0018000F */
    char             __unused004[0x4];              /* 0x00180010-0x00180013 */
    unsigned long    next_pixels_command;           /* 0x00180014-0x00180017 */
    char             __unused005[0x68];             /* 0x00180018-0x0018007F */
    unsigned long    pixel1_base_command[32];       /* 0x00180080-0x001800FF */
    char             __unused006[0x84];             /* 0x00180100-0x00180183 */

    /* Parameter Engine Control and Condition Regs */
    unsigned long    oor;                           /* 0x00180184-0x00180187 */
    char             __unused007[0x4];              /* 0x00180188-0x0018018B */
    unsigned long    cindex;                        /* 0x0018018C-0x0018018F */
    unsigned long    w_off_xy;                      /* 0x00180190-0x00180193 */
    unsigned long    pe_w_min;                      /* 0x00180194-0x00180197 */
    unsigned long    pe_w_max;                      /* 0x00180198-0x0018019B */
    char             __unused008[0x4];              /* 0x0018019C-0x0018019F */
    unsigned long    yclip;                         /* 0x001801A0-0x001801A3 */
    unsigned long    xclip;                         /* 0x001801A4-0x001801A7 */
    unsigned long    xedge_lt;                      /* 0x001801A8-0x001801AB */
    unsigned long    xedge_gt;                      /* 0x001801AC-0x001801AF */
    unsigned long    yedge_lt;                      /* 0x001801B0-0x001801B3 */
    unsigned long    yedge_gt;                      /* 0x001801B4-0x001801B7 */
    char             __unused009[0x48];             /* 0x001801B8-0x001801FF */

    /* Drawing Engine Regs */
    unsigned long    fground;                       /* 0x00180200-0x00180203 */
    unsigned long    bground;                       /* 0x00180204-0x00180207 */
    unsigned long    pmask;                         /* 0x00180208-0x0018020B */
    unsigned long    draw_mode;                     /* 0x0018020C-0x0018020F */
    unsigned long    pat_originx;                   /* 0x00180210-0x00180213 */
    unsigned long    pat_originy;                   /* 0x00180214-0x00180217 */
    unsigned long    raster;                        /* 0x00180218-0x0018021B */
    unsigned long    pixel8_reg;                    /* 0x0018021C-0x0018021F */
    unsigned long    w_min;                         /* 0x00180220-0x00180223 */
    unsigned long    w_max;                         /* 0x00180224-0x00180227 */
    char             __unused010[0x58];             /* 0x00180228-0x0018027F */
    unsigned long    pattern[8];                    /* 0x00180280-0x0018029F */
    char             __unused011[0xD68];            /* 0x001802A0-0x00181007 */

    /* Parameter Engine Registers */
    unsigned long    x0_abs;                        /* 0x00181008-0x0018100B */
    char             __unused012[0x4];              /* 0x0018100C-0x0018100F */
    unsigned long    y0_abs;                        /* 0x00181010-0x00181013 */
    char             __unused013[0x4];              /* 0x00181014-0x00181017 */
    unsigned long    xy0_abs;                       /* 0x00181018-0x0018101B */
    char             __unused014[0xC];              /* 0x0018101C-0x00181027 */
    unsigned long    x0_rel;                        /* 0x00181028-0x0018102B */
    char             __unused015[0x4];              /* 0x0018102C-0x0018102F */
    unsigned long    y0_rel;                        /* 0x00181030-0x00181033 */
    char             __unused016[0x4];              /* 0x00181034-0x00181037 */
    unsigned long    xy0_rel;                       /* 0x00181038-0x0018103B */
    char             __unused017[0xC];              /* 0x0018103C-0x00181047 */
    unsigned long    x1_abs;                        /* 0x00181048-0x0018104B */
    char             __unused018[0x4];              /* 0x0018104C-0x0018104F */
    unsigned long    y1_abs;                        /* 0x00181050-0x00181053 */
    char             __unused019[0x4];              /* 0x00181054-0x00181057 */
    unsigned long    xy1_abs;                       /* 0x00181058-0x0018105B */
    char             __unused020[0xC];              /* 0x0018105C-0x00181067 */
    unsigned long    x1_rel;                        /* 0x00181068-0x0018106B */
    char             __unused021[0x4];              /* 0x0018106C-0x0018106F */
    unsigned long    y1_rel;                        /* 0x00181070-0x00181073 */
    char             __unused022[0x4];              /* 0x00181074-0x00181077 */
    unsigned long    xy1_rel;                       /* 0x00181078-0x0018107B */
    char             __unused023[0xC];              /* 0x0018107C-0x00181087 */
    unsigned long    x2_abs;                        /* 0x00181088-0x0018108B */
    char             __unused024[0x4];              /* 0x0018108C-0x0018108F */
    unsigned long    y2_abs;                        /* 0x00181090-0x00181093 */
    char             __unused025[0x4];              /* 0x00181094-0x00181097 */
    unsigned long    xy2_abs;                       /* 0x00181098-0x0018109B */
    char             __unused026[0xC];              /* 0x0018109C-0x001810A7 */
    unsigned long    x2_rel;                        /* 0x001810A8-0x001810AB */
    char             __unused027[0x4];              /* 0x001810AC-0x001810AF */
    unsigned long    y2_rel;                        /* 0x001810B0-0x001810B3 */
    char             __unused028[0x4];              /* 0x001810B4-0x001810B7 */
    unsigned long    xy2_rel;                       /* 0x001810B8-0x001810BB */
    char             __unused029[0xC];              /* 0x001810BC-0x001810C7 */
    unsigned long    x3_abs;                        /* 0x001810C8-0x001810CB */
    char             __unused030[0x4];              /* 0x001810CC-0x001810CF */
    unsigned long    y3_abs;                        /* 0x001810D0-0x001810D3 */
    char             __unused031[0x4];              /* 0x001810D4-0x001810D7 */
    unsigned long    xy3_abs;                       /* 0x001810D8-0x001810DB */
    char             __unused032[0xC];              /* 0x001810DC-0x001810E7 */
    unsigned long    x3_rel;                        /* 0x001810E8-0x001810EB */
    char             __unused033[0x4];              /* 0x001810EC-0x001810EF */
    unsigned long    y3_rel;                        /* 0x001810F0-0x001810F3 */
    char             __unused034[0x4];              /* 0x001810F4-0x001810F7 */
    unsigned long    xy3_rel;                       /* 0x001810F8-0x001810FB */
    char             __unused035[0x10C];            /* 0x001810FC-0x00181207 */

    /* Meta-Coordinate Pseudo-Registers */
    unsigned long    point_win_x;                   /* 0x00181208-0x0018120B */
    char             __unused036[0x4];              /* 0x0018120C-0x0018120F */
    unsigned long    point_win_y;                   /* 0x00181210-0x00181213 */
    char             __unused037[0x4];              /* 0x00181214-0x00181217 */
    unsigned long    point_win_xy;                  /* 0x00181218-0x0018121B */
    char             __unused038[0xC];              /* 0x0018121C-0x00181227 */
    unsigned long    point_vert_x;                  /* 0x00181228-0x0018122B */
    char             __unused039[0x4];              /* 0x0018122C-0x0018122F */
    unsigned long    point_vert_y;                  /* 0x00181230-0x00181233 */
    char             __unused040[0x4];              /* 0x00181234-0x00181237 */
    unsigned long    point_vert_xy;                 /* 0x00181238-0x0018123B */
    char             __unused041[0xC];              /* 0x0018123C-0x00181247 */
    unsigned long    line_win_x;                    /* 0x00181248-0x0018124B */
    char             __unused042[0x4];              /* 0x0018124C-0x0018124F */
    unsigned long    line_win_y;                    /* 0x00181250-0x00181253 */
    char             __unused043[0x4];              /* 0x00181254-0x00181257 */
    unsigned long    line_win_xy;                   /* 0x00181258-0x0018125B */
    char             __unused044[0xC];              /* 0x0018125C-0x00181267 */
    unsigned long    line_vert_x;                   /* 0x00181268-0x0018126B */
    char             __unused045[0x4];              /* 0x0018126C-0x0018126F */
    unsigned long    line_vert_y;                   /* 0x00181270-0x00181273 */
    char             __unused046[0x4];              /* 0x00181274-0x00181277 */
    unsigned long    line_vert_xy;                  /* 0x00181278-0x0018127B */
    char             __unused047[0xC];              /* 0x0018127C-0x00181287 */
    unsigned long    tri_win_x;                     /* 0x00181288-0x0018128B */
    char             __unused048[0x4];              /* 0x0018128C-0x0018128F */
    unsigned long    tri_win_y;                     /* 0x00181290-0x00181293 */
    char             __unused049[0x4];              /* 0x00181294-0x00181297 */
    unsigned long    tri_win_xy;                    /* 0x00181298-0x0018129B */
    char             __unused050[0xC];              /* 0x0018129C-0x001812A7 */
    unsigned long    tri_vert_x;                    /* 0x001812A8-0x001812AB */
    char             __unused051[0x4];              /* 0x001812AC-0x001812AF */
    unsigned long    tri_vert_y;                    /* 0x001812B0-0x001812B3 */
    char             __unused052[0x4];              /* 0x001812B4-0x001812B7 */
    unsigned long    tri_vert_xy;                   /* 0x001812B8-0x001812BB */
    char             __unused053[0xC];              /* 0x001812BC-0x001812C7 */
    unsigned long    quad_win_x;                    /* 0x001812C8-0x001812CB */
    char             __unused054[0x4];              /* 0x001812CC-0x001812CF */
    unsigned long    quad_win_y;                    /* 0x001812D0-0x001812D3 */
    char             __unused055[0x4];              /* 0x001812D4-0x001812D7 */
    unsigned long    quad_win_xy;                   /* 0x001812D8-0x001812DB */
    char             __unused056[0xC];              /* 0x001812DC-0x001812E7 */
    unsigned long    quad_vert_x;                   /* 0x001812E8-0x001812EB */
    char             __unused057[0x4];              /* 0x001812EC-0x001812EF */
    unsigned long    quad_vert_y;                   /* 0x001812F0-0x001812F3 */
    char             __unused058[0x4];              /* 0x001812F4-0x001812F7 */
    unsigned long    quad_vert_xy;                  /* 0x001812F8-0x001812FB */
    char             __unused059[0xC];              /* 0x001812FC-0x00181307 */
    unsigned long    rect_win_x;                    /* 0x00181308-0x0018130B */
    char             __unused060[0x4];              /* 0x0018130C-0x0018130F */
    unsigned long    rect_win_y;                    /* 0x00181310-0x00181313 */
    char             __unused061[0x4];              /* 0x00181314-0x00181317 */
    unsigned long    rect_win_xy;                   /* 0x00181318-0x0018131B */
    char             __unused062[0xC];              /* 0x0018131C-0x00181327 */
    unsigned long    rect_vert_x;                   /* 0x00181328-0x0018132B */
    char             __unused063[0x4];              /* 0x0018132C-0x0018132F */
    unsigned long    rect_vert_y;                   /* 0x00181330-0x00181333 */
    char             __unused064[0x4];              /* 0x00181334-0x00181337 */
    unsigned long    rect_vert_xy;                  /* 0x00181338-0x0018133B */
 
    char             __unused999[0x7ECC4];
  } P9000_BT445;


typedef struct
  {
    ulong	ivr;
    ulong	lcd_enable;
    ulong	lcd_disable;
    ulong	reset;
    char	__fill00[0x0ffff0];
    P9000_BT445	p9000_bt445;
    char	vram[0x200000];
  } board_db_180;



/*
   Note:  The names of the people involved have been changed to protect the
   guilty.  Any resemblace to persons living, dead, or unborn are purely
   coincidental.  Remember, always make sure the color of your shoes matches
   the color of your umbrella.
*/

/* P9000 Minterms and Other Flags */
#define IGM_F_MASK  (0xFF00)
#define IGM_B_MASK  (0xF0F0)
#define IGM_S_MASK  (0xCCCC)
#define IGM_D_MASK  (0xAAAA)
#define P9000_OVERSIZED    (1<<16)
#define P9000_X11          (0)
#define P9000_USE_PATTERN  (1<<17)


#define pack_xy(XXX,YYY)   ((((XXX)<<16)&0xFFFF0000) | ((YYY)&0x0000FFFF))

/* P9000 Status Reg Flags */
#define P9000_ISSUE      (1<<31)
#define P9000_BUSY       (1<<30)
#define P9000_PICKED     (1<<7)
#define P9000_PIXEL_SW   (1<<6)
#define P9000_BLIT_SW    (1<<5)
#define P9000_QUAD_SW    (1<<4)
#define P9000_CONCAVE    (1<<3)
#define P9000_HIDDEN     (1<<2)
#define P9000_VISIBLE    (1<<1)
#define P9000_INTERSECTS (1<<0)

#define P9000_busy_wait(vgboard)  {while ((((board_db_180 *)((vgboard)->addr))->p9000_bt445.status & P9000_BUSY)!=0);}

#define P9000_issue_wait(vgboard)  {while ((((board_db_180 *)((vgboard)->addr))->p9000_bt445.status & P9000_ISSUE)!=0);}



#endif

