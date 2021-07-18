
#include "vgl.h"
#include "vgl_internals.h"


static PIXMAP default_pixmap =
  {
    NULL,			/* pixdata */
    VGL_ERR_NONE,		/*  error */
    NULL,
    NULL,			/* mouse_pixmap */
    CLIP_ON, 0, 0, 0, 0,	/* clip_flag, x_min, y_min, x_max, y_max */
    
    255, 0,			/* Forground & background */
    0, 0,
    {0},
    NULL,
    NULL,			/* Workspace */
    0,				/* Worksize */
#ifdef VGL_DITHER
    NULL,
    0, 0,
#endif /* VGL_DITHER */
    0,				/* Mouse_nocheck_flag */
    
    0,
    
#ifdef AMIGA
/* We use VGL as a link library. These symbol references cause the entire
** VGL to be linked to the executable, whether functions are utilized or
** not. Calling the VGL functions directly fixes this. */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#else
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
#endif
  };


/*****************************************************************************/
/**************** PIXMAP *vgl_makepixmap( int x_res, int y_res) **************/
/*****************************************************************************/
PIXMAP *
vgl_makepixmap (int x_res, int y_res)
{
  PIXMAP *p;

  p = vgl_malloc (sizeof (PIXMAP));
  if (p)
    {
      *p = default_pixmap;
      p->pixdata = vgl_malloc (sizeof (p->pixdata[0]));
      if (p->pixdata == NULL)
	{
	  vgl_free (p);
	  return (NULL);
	}

      p->pixdata->x_res = x_res;
      p->pixdata->y_res = y_res;
#ifdef AMIGA
      /* For no obvious reason, Xenon's corner-turner (Tina Turner?)
      ** likes to have byteplanes padded to 32 pixels */
      p->pixdata->x_size = ROUNDUP(x_res, 32);
#else /* !AMIGA */
      /* x_size = x_res rounded up to a mult of 4 */
      p->pixdata->x_size = (x_res & 0xFFFFFFFC) + ((x_res & 0x03) ? 4 : 0);
#endif /* !AMIGA */
      p->pixdata->users = 1;
      p->pixdata->free_flag = 1;

      p->clip_x_max = p->pixdata->x_res;
      p->clip_y_max = p->pixdata->y_res;
      p->clip_x_min = 0;
      p->clip_y_min = 0;

      p->board = NULL;
#ifndef AMIGA
      p->font = vgl_large_ffont;
#endif

      p->pixdata->data = 
	vgl_malloc (sizeof (char) * p->pixdata->x_size * p->pixdata->y_res);
      if (p->pixdata->data == NULL)
	{
	  vgl_free (p->pixdata);
	  vgl_free (p);
	  return (NULL);
	}

#ifdef AMIGA
      vgl_dumb_setcur(p, p->foreground, p->background);
#else
      vgl_setcur (p, p->foreground, p->background);
#endif

      return (p);
    }
  else
    return (NULL);
}


/*****************************************************************************/
/******************************  Free Pixmap  ********************************/
/*****************************************************************************/
void 
vgl_freepixmap (PIXMAP * p)
{
  if (p)
    {
      /* Decriment the number of users of this pixdata */
      p->pixdata->users--;

      /* If there are no more users of this pixmap, free whatever */
      if (p->pixdata->users <= 0)
	{
	  /* If the pixdata->map is free-able, free it */
	  if (p->pixdata->free_flag != 0)
	    vgl_free (p->pixdata->data);

	  free (p->pixdata);	/* Free the pixdata struct */
	}

      if (p->workspace != NULL)
	vgl_free (p->workspace);

#ifdef VGL_DITHER
      if (p->DitherTable != NULL)
	vgl_free (p->DitherTable);

#endif /* VGL_DITHER */

      vgl_free (p);		/* Free the pixmap */
    }

  return;
}


/*****************************************************************************/
/*****************************************************************************/
PIXMAP *
vgl_dupepixmap (PIXMAP * pixmap)
{
  PIXMAP *p;

  p = vgl_malloc (sizeof (PIXMAP));
  if (p)
    {
      *p = *pixmap;
      p->pixdata->users++;
      p->worksize = 0;		/* Workspace is not shared */
      p->workspace = NULL;
#ifdef VGL_DITHER
      p->DTableSize = 0;		/* DitherTable is not shared either */
      p->DTableIdx = 0;
      p->DitherTable = NULL;
#endif /* VGL_DITHER */
      return (p);
    }
  else
    return (NULL);
}

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void 
vgl_copypixmap (PIXMAP * dest, PIXMAP * source)
{
  if (source && dest)
    {
      /* Decriment the number of users of this pixdata */
      dest->pixdata->users--;

      /* If there are no more users of this pixmap, free whatever */
      if (dest->pixdata->users <= 0)
	{
	  /* If the pixdata->map is free-able, free it */
	  if (dest->pixdata->free_flag != 0)
	    vgl_free (dest->pixdata->data);

	  free (dest->pixdata);	/* Free the pixdata struct */
	}

      if (dest->workspace != NULL)
	vgl_free (dest->workspace);

#ifdef VGL_DITHER
      if (dest->DitherTable != NULL)
	vgl_free (dest->DitherTable);
      dest->DTableSize = 0;
      dest->DTableIdx = 0;
      dest->DitherTable = NULL;
#endif
      
      *dest = *source;
      dest->pixdata->users++;
      dest->worksize = 0;
      dest->workspace = NULL;
    }
}
#endif
/*****************************************************************************/
/*****************************************************************************/
