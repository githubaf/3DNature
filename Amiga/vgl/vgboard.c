
#include "vgl.h"
#include "vgl_internals.h"

#if defined(VXWORKS)
#include <vme.h>
#elif defined(OS9)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <module.h>
#include <modes.h>
#include <procid.h>
#include <setsys.h>
#include <errno.h>

#include "../os9/common/sys_os9.h"
#endif


static VGBOARD default_board[] =
{
#ifdef SUPPORT_MMI_150
  {
    "Uninitalized", -1,
    MMI_150, (void *) 0x0100000, (void *) 0x00000000, 1024*1024,
    5, 176, NULL, 0, 0, 0, 0, 0, 5, 35, 8, 2, 0, 0, 0,
    mmi_150_board_init, mmi_150_board_kill,
    mmi_150_pixmap_map, mmi_150_pixmap_disp, mmi_150_boarderror,
    mmi_150_setrgb, mmi_150_getrgb,
    mmi_150_mouse_getcoords, mmi_150_mouse_setcoords, mmi_150_mouse_sethandler,
    mmi_150_mouse_setaccel, mmi_150_mouse_setpixmap,
    mmi_150_mouse_ptr_on, mmi_150_mouse_ptr_off,
    mmi_150_kbhit, mmi_150_getkey, mmi_150_kb_setled,
    mmi_150_ss_set, mmi_150_ss_tick, mmi_150_test,
    0, 0, {0}, 0, 0, 0,  VGL_ERR_NONE, {0}, 0, NULL, NULL, NULL, NULL, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, SS_ON, 0, 1, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0,
    NULL, 0, 0, NULL,
  },
#endif

#ifdef SUPPORT_MMI_250
  {
    "Uninitalized", -1,
    MMI_250, (void *) 0x0200000, (void *) 0x02F0000, 4*1024*1024,
    1, 180, NULL, 0, 0, 0, 0, 0, 5, 35, 8, 2, 0, 0, 0,
    mmi_250_board_init, mmi_250_board_kill,
    mmi_250_pixmap_map, mmi_250_pixmap_disp, mmi_250_boarderror,
    mmi_250_setrgb, mmi_250_getrgb, mmi_250_mouse_getcoords,
    mmi_250_mouse_setcoords, mmi_250_mouse_sethandler,
    mmi_250_mouse_setaccel, mmi_250_mouse_setpixmap,
    mmi_250_mouse_ptr_on, mmi_250_mouse_ptr_off,
    mmi_250_kbhit, mmi_250_getkey, mmi_250_kb_setled,
    mmi_250_ss_set, mmi_250_ss_tick, mmi_250_test,
    0, 0, {0}, 0, 0, 0,  VGL_ERR_NONE, {0}, 0, NULL, NULL, NULL, NULL, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, SS_ON, 0, 1, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0,
    NULL, 0, 0, NULL,
  },
#endif

#ifdef SUPPORT_DB_180
  {
    "Uninitalized", -1, 
    DB_180, (void *) 0xFD000000, (void *) 0xFD000000, 4*1024*1024,
    4, 180, NULL, 0, 0, 0, 0, 0, 5, 35, 8, 2, 0, 0, 0,
    db_180_board_init, db_180_board_kill,
    db_180_pixmap_map, db_180_pixmap_disp, db_180_boarderror,
    db_180_setrgb, db_180_getrgb, db_180_mouse_getcoords,
    db_180_mouse_setcoords, db_180_mouse_sethandler,
    db_180_mouse_setaccel, db_180_mouse_setpixmap,
    db_180_mouse_ptr_on, db_180_mouse_ptr_off,
    db_180_kbhit, db_180_getkey, db_180_kb_setled,
    db_180_ss_set, db_180_ss_tick, db_180_test,
    0, 0, {0}, 0, 0, 0, VGL_ERR_NONE, {0}, 0, NULL, NULL, NULL, NULL, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, SS_ON, 0, 1, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0,
    NULL, 0, 0, NULL,
  },
#endif

#ifdef SUPPORT_GDM_9000
  {
    "Uninitalized", -1, 
    GDM_9000, (void *) 0xFD000000, (void *) 0xFD000000, 4*1024*1024,
    4, 180, NULL, 0, 0, 0, 0, 0, 5, 35, 8, 2, 0, 0, 0,
    gdm_9000_board_init, gdm_9000_board_kill,
    gdm_9000_pixmap_map, gdm_9000_pixmap_disp, gdm_9000_boarderror,
    gdm_9000_setrgb, gdm_9000_getrgb, gdm_9000_mouse_getcoords,
    gdm_9000_mouse_setcoords, gdm_9000_mouse_sethandler,
    gdm_9000_mouse_setaccel, gdm_9000_mouse_setpixmap,
    gdm_9000_mouse_ptr_on, gdm_9000_mouse_ptr_off,
    gdm_9000_kbhit, gdm_9000_getkey, gdm_9000_kb_setled,
    gdm_9000_ss_set, gdm_9000_ss_tick, gdm_9000_test,
    0, 0, {0}, 0, 0, 0, VGL_ERR_NONE, {0}, 0, NULL, NULL, NULL, NULL, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, SS_ON, 0, 1, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0,
    NULL, 0, 0, NULL,
  },
#endif
};

#define	N_BOARD_TYPES	(sizeof(default_board)/sizeof(default_board[0]))

static int font_init = 1;
struct vgl_ffont *vgl_small_ffont = NULL, *vgl_large_ffont;

/*****************************************************************************/
/*****************************************************************************/
VGBOARD *
vgl_initboard (char *board_name, char *video_mode)
{
  int i;
  VGBOARD *b;
  void	*address;
  int   num;
  short type, irq_level, irq_vector;
#ifdef OS9
/*
  mh_com *module;
*/
  mod_exec *module;
#endif

  if (font_init == 1)
    {
      vgl_small_ffont = vgl_expand_font (&vgl_small_font);
      vgl_large_ffont = vgl_expand_font (&vgl_large_font);
      font_init = 0;
    }

  for( i=0; board_config[i].name[0]!='\0'; i++)
    if( vgl_stricmp( board_name, board_config[i].name)==0)
      break;
	  
  if( board_config[i].name[0]=='\0')
    return(NULL);
  else
    {
      num        = i;
      type       = board_config[i].type;
      address    = board_config[i].addr;
      irq_level  = board_config[i].irq_level;
      irq_vector = board_config[i].irq_vector;
    }

/*  Buggy IF statement...  (Kept here for historical/hysterical reasons.)
  if (type < 0 || type >= N_BOARD_TYPES)
    return (NULL);
*/

#ifdef OS9
  module = modlink (board_config[i].name, 0);
  if ((int)module == -1)
    b = NULL;
  else
    b = (VGBOARD *)((int)module + (int)module->_mexec);
#else
  b = vgl_malloc (sizeof (VGBOARD));
#endif

  if (b)
    {
      for (i=0; i<N_BOARD_TYPES; i++)
	if (default_board[i].type == type)
	  {
	    *b = default_board[i];
	    break;
	  }

      if( i>=N_BOARD_TYPES)
	{
	  vgl_free(b);
	  return(NULL);
	}

      vgl_strcpy (b->name, board_name);
      b->num = num;
      b->type = type;
      b->addr = address;
      b->irq_level = irq_level;
      b->irq_vector = irq_vector;
#if defined (OS9)
      b->pointer1 = module;
#endif

      for (i = 0; i < vgl_mode_list[i].name[0] != '\0'; i++)
	if (vgl_stricmp (video_mode, vgl_mode_list[i].name) == 0)
	  {
	    b->mode = vgl_mode_list + i;
	    break;
	  }

      if (vgl_mode_list[i].name[0] == '\0')
	{
	  vgl_free (b);
	  return (NULL);
	}

      b->addr = vgl_ext_to_int_address (b->addr, b->mem_size);

      vgl_mouse_queue_init (b);
      vgl_kb_queue_init (b);

      (*b->board_init) (b);

#ifdef VXWORKS
      sysIntEnable (b->irq_level);
#endif

      return (b);
    }
  else
    return (NULL);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
