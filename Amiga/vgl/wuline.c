
/*
 *  This function draws a line, based on work by Xialon Wu.  It is similar
 *  to the Bresenham's line drawing routine, but should be about four times
 *  faster for longer lines.
 *
 *  Pattern 1:    Pattern 2:       Pattern 3:       Pattern 4:
 *  . . .                 . . .            . . .            . . *
 *  . . .                 . . *            . * *            . * .
 *  # * *                 # * .            # . .            # . .
 *
 *    Legend:
 *       . = Unlit Pixel
 *       * = Lit Pixel
 *       # = Reference Pixel (already lit)    
 */

#include "vgl.h"
#include "vgl_internals.h"

#ifdef	WU_LINE

/*****************************************************************************/
/*****************************************************************************/
void 
vgl_dumb_line_noclip (PIXMAP * p, int a1, int b1, int a2, int b2)
{
  int dx, dy, incr1, incr2, D, xend, c, pixels_left, sign_x, sign_y, step,
    step1, x_size, i;
  unsigned char *loc, *loc1, color;
  int mx1, mx2, my1, my2;
  unsigned long mouse;

  if (a1 == a2)
    {
      vgl_dumb_vline_noclip (p, a1, b1, b2);
      return;
    }
  else if (b1 == b2)
    {
      vgl_dumb_hline_noclip (p, a1, a2, b1);
      return;
    }

/****** Do the mouse thing ******/
  if (a1 < a2)
    {
      mx1 = a1;
      mx2 = a2;
    }
  else
    {
      mx1 = a2;
      mx2 = a1;
    }

  if (b1 < b2)
    {
      my1 = b1;
      my2 = b2;
    }
  else
    {
      my1 = b2;
      my2 = b1;
    }

  check_mouse1 (p, mx1, my1, mx2, my2, mouse);


  color = (unsigned char) p->foreground;

  dx = a2 - a1;
  if (dx < 0)
    {
      sign_x = 1;
      dx = -dx;
    }
  else
    sign_x = 0;

  dy = b2 - b1;
  if (dy < 0)
    {
      sign_y = 1;
      dy = -dy;
    }
  else
    sign_y = 0;


/* Chooses axis of grestest movement (make dx) */
/* Note error check for dx==0 shoud be included here */
  if (dy <= dx)
    {
      /* decide incriment sign by the slope sign */
      if (sign_x == sign_y)
	step = p->pixdata->x_size;
      else
	step = -p->pixdata->x_size;

      step1 = step + 1;

      /* start from the smaller coordinate */
/*
   if( a1>a2)
 */
      if (sign_x)
	{
	  loc = (((unsigned char *) p->pixdata->data) + 
		 (p->pixdata->x_size * b2 + a2));
	  loc1 = (((unsigned char *) p->pixdata->data) + 
		  (p->pixdata->x_size * b1 + a1));
	}
      else
	{
	  loc = (((unsigned char *) p->pixdata->data) + 
		 (p->pixdata->x_size * b1 + a1));
	  loc1 = (((unsigned char *) p->pixdata->data) + 
		  (p->pixdata->x_size * b2 + a2));
	}

      /* Note dx=n implies 0 - n or (dx+1) pixels to be set    */
      /* Go round loop dx/4 times then plot last 0, 1, 2, or 3 pixels */
      /* In fact (dx-1)/4 as 2 pixels are already ploted.  */
      xend = (dx - 1) >> 2;
      pixels_left = (dx - 1) & 0x03;	/* Number of pixels over on the end */

      *loc = color;
      *loc1 = color;
/*
   incr2 = 4 * dy - 2 * dx;
 */
      incr2 = (dy << 2) - (dx << 1);

      if (incr2 < 0)
	{
/*
   c = 2*dy;
   incr1=2*c;
 */
	  c = dy << 1;
	  incr1 = c << 1;
	  D = incr1 - dx;

	  for (i = 0; i < xend; i++)	/* Plotting loop */
	    {
	      loc++;
	      loc1--;
	      if (D < 0)
		{
		  /* Pattern 1 forwards */
		  *(loc++) = color;
		  *loc = color;

		  /* Pattern 1 backwards */
		  *(loc1--) = color;
		  *loc1 = color;

		  D += incr1;
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2, forwards */
		      *loc = color;
		      loc += step1;
		      *loc = color;

		      /* Pattern 2, backwards */
		      *loc1 = color;
		      loc1 -= step1;
		      *loc1 = color;
		    }
		  else
		    {
		      /* Pattern 3, forwards */
		      loc += step;
		      *(loc++) = color;
		      *loc = color;

		      /* Pattern 3, reverse */
		      loc1 -= step;
		      *(loc1--) = color;
		      *loc1 = color;
		    }

		  D += incr2;
		}
	    }			/* End for() */

	  /* Plot last pattern */
	  if (pixels_left)
	    {
	      if (D < 0)
		{
		  /* Pattern 1 */
		  *(++loc) = color;

		  if (pixels_left > 1)
		    *(++loc) = color;

		  if (pixels_left > 2)
		    *(--loc1) = color;
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2 */
		      *(++loc) = color;

		      if (pixels_left > 1)
			{
			  loc += step1;
			  *loc = color;
			}

		      if (pixels_left > 2)
			*(--loc1) = color;
		    }
		  else
		    {
		      /* Pattern 3 */
		      loc += step1;
		      *loc = color;
		      if (pixels_left > 1)
			*(++loc) = color;

		      if (pixels_left > 2)
			{
			  loc1 -= step1;
			  *loc1 = color;
			}
		    }
		}
	    }			/* end if pixels_left */
	}			/* End slope < 1/2 */
      else
	{			/* Slope > 1/2 */
/*
   c = 2 * (dy - dx);
   incr1 = 2 * c;
 */
	  c = (dy - dx) << 1;
	  incr1 = c << 1;

	  D = incr1 + dx;
	  for (i = 0; i < xend; i++)
	    {
	      loc++;
	      loc1--;

	      if (D > 0)
		{
		  /* Pattern 4, forwards */
		  loc += step;
		  *loc = color;
		  loc += step1;
		  *loc = color;

		  /* Pattern 4, backwards */
		  loc1 -= step;
		  *loc1 = color;
		  loc1 -= step1;
		  *loc1 = color;

		  D += incr1;
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2, forwards */
		      *loc = color;
		      loc += step1;
		      *loc = color;

		      /* Pattern 2, backwards */
		      *loc1 = color;
		      loc1 -= step1;
		      *loc1 = color;
		    }
		  else
		    {
		      /* Pattern 3, forwards */
		      loc += step;
		      *(loc++) = color;
		      *loc = color;

		      /* Pattern 3, backwards */
		      loc1 -= step;
		      *(loc1--) = color;
		      *loc1 = color;
		    }
		  D += incr2;
		}
	    }			/* end for */

	  /* Plot Last Pattern */
	  if (pixels_left)
	    {
	      if (D > 0)
		{
		  /* Pattern 4 */
		  loc += step1;
		  *loc = color;
		  if (pixels_left > 1)
		    {
		      loc += step1;
		      *loc = color;
		    }

		  if (pixels_left > 2)
		    {
		      loc1 -= step1;
		      *loc1 = color;
		    }
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2 */
		      *(++loc) = color;
		      if (pixels_left > 1)
			{
			  loc += step1;
			  *loc = color;
			}

		      if (pixels_left > 2)
			{
			  *(--loc1) = color;
			}
		    }
		  else
		    {
		      /* Pattern 3 */
		      loc += step1;
		      *loc = color;
		      if (pixels_left > 1)
			{
			  *(++loc) = color;
			}
		      if (pixels_left > 2)
			{
			  if (D > c)	/* Step 3 */
			    {
			      loc1 -= step1;
			      *loc1 = color;
			    }
			  else
			    {
			      *(--loc1) = color;
			    }
			}
		    }
		}
	    }
	}			/* End slope > 1/2 */
    }
  else

/********************************/
    {
      /* decide incriment sign by the slope sign */
      if (sign_x == sign_y)
	step = 1;
      else
	step = -1;

      x_size = p->pixdata->x_size;
      step1 = step + x_size;

      /* start from the smaller coordinate */
/*
   if( b1>b2)
 */
      if (sign_y)
	{
	  loc = (((unsigned char *) p->pixdata->data) + 
		 (p->pixdata->x_size * b2 + a2));
	  loc1 = (((unsigned char *) p->pixdata->data) + 
		  (p->pixdata->x_size * b1 + a1));
	}
      else
	{
	  loc = (((unsigned char *) p->pixdata->data) + 
		 (p->pixdata->x_size * b1 + a1));
	  loc1 = (((unsigned char *) p->pixdata->data) + 
		  (p->pixdata->x_size * b2 + a2));
	}

      /* Note dx=n implies 0 - n or (dx+1) pixels to be set    */
      /* Go round loop dx/4 times then plot last 0, 1, 2, or 3 pixels */
      /* In fact (dx-1)/4 as 2 pixels are already ploted.  */
      xend = (dy - 1) >> 2;
      pixels_left = (dy - 1) & 0x03;	/* Number of pixels over on the end */

      *loc = color;
      *loc1 = color;
/*
   incr2 = 4 * dx - 2 * dy;
 */
      incr2 = (dx << 2) - (dy << 1);

      if (incr2 < 0)
	{
/*
   c = 2*dx;
   incr1=2*c;
 */
	  c = dx << 1;
	  incr1 = c << 1;
	  D = incr1 - dy;

	  for (i = 0; i < xend; i++)	/* Plotting loop */
	    {
	      loc += x_size;
	      loc1 -= x_size;
	      if (D < 0)
		{
		  /* Pattern 1 forwards */
		  *loc = color;
		  loc += x_size;
		  *loc = color;

		  /* Pattern 1 backwards */
		  *loc1 = color;
		  loc1 -= x_size;
		  *loc1 = color;

		  D += incr1;
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2, forwards */
		      *loc = color;
		      loc += step1;
		      *loc = color;

		      /* Pattern 2, backwards */
		      *loc1 = color;
		      loc1 -= step1;
		      *loc1 = color;
		    }
		  else
		    {
		      /* Pattern 3, forwards */
		      loc += step;
		      *loc = color;
		      loc += x_size;
		      *loc = color;

		      /* Pattern 3, reverse */
		      loc1 -= step;
		      *loc1 = color;
		      loc1 -= x_size;
		      *loc1 = color;
		    }

		  D += incr2;
		}
	    }			/* End for() */

	  /* Plot last pattern */
	  if (pixels_left)
	    {
	      if (D < 0)
		{
		  /* Pattern 1 */
		  loc += x_size;
		  *loc = color;

		  if (pixels_left > 1)
		    {
		      loc += x_size;
		      *loc = color;
		    }

		  if (pixels_left > 2)
		    {
		      loc1 -= x_size;
		      *loc1 = color;
		    }
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2 */
		      loc += x_size;
		      *loc = color;
		      if (pixels_left > 1)
			{
			  loc += step1;
			  *loc = color;
			}

		      if (pixels_left > 2)
			{
			  loc1 -= x_size;
			  *loc1 = color;
			}
		    }
		  else
		    {
		      /* Pattern 3 */
		      loc += step1;
		      *loc = color;
		      if (pixels_left > 1)
			{
			  loc += x_size; /* This one is suspect. -Xen */
			  *loc = color;
			}

		      if (pixels_left > 2)
			{
			  loc1 -= step1;
			  *loc1 = color;
			}
		    }
		}
	    }			/* end if pixels_left */
	}			/* End slope < 1/2 */
      else
	{			/* Slope > 1/2 */
/*
   c = 2 * (dx - dy);
   incr1 = 2 * c;
 */
	  c = (dx - dy) << 1;
	  incr1 = c << 1;
	  D = incr1 + dy;

	  for (i = 0; i < xend; i++)
	    {
	      loc += x_size;
	      loc1 -= x_size;

	      if (D > 0)
		{
		  /* Pattern 4, forwards */
		  loc += step;
		  *loc = color;
		  loc += step1;
		  *loc = color;

		  /* Pattern 4, backwards */
		  loc1 -= step;
		  *loc1 = color;
		  loc1 -= step1;
		  *loc1 = color;

		  D += incr1;
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2, forwards */
		      *loc = color;
		      loc += step1;
		      *loc = color;

		      /* Pattern 2, backwards */
		      *loc1 = color;
		      loc1 -= step1;
		      *loc1 = color;
		    }
		  else
		    {
		      /* Pattern 3, forwards */
		      loc += step;
		      *loc = color;
		      loc += x_size;
		      *loc = color;

		      /* Pattern 3, backwards */
		      loc1 -= step;
		      *loc1 = color;
		      loc1 -= x_size;
		      *loc1 = color;
		    }
		  D += incr2;
		}
	    }			/* end for */

	  /* Plot Last Pattern */
	  if (pixels_left)
	    {
	      if (D > 0)
		{
		  /* Pattern 4 */
		  loc += step1;
		  *loc = color;
		  if (pixels_left > 1)
		    {
		      loc += step1;
		      *loc = color;
		    }

		  if (pixels_left > 2)
		    {
		      loc1 -= step1;
		      *loc1 = color;
		    }
		}
	      else
		{
		  if (D < c)
		    {
		      /* Pattern 2 */
		      loc += x_size;
		      *loc = color;
		      if (pixels_left > 1)
			{
			  loc += step1;
			  *loc = color;
			}

		      if (pixels_left > 2)
			{
			  loc1 -= x_size;
			  *loc1 = color;
			}
		    }
		  else
		    {
		      /* Pattern 3 */
		      loc += step1;
		      *loc = color;
		      if (pixels_left > 1)
			{
			  loc += x_size; /* This is the line! -Xen */
			  *loc = color;
			}
		      if (pixels_left > 2)
			{
			  if (D > c)	/* Step 3 */
			    {
			      loc1 -= step1;
			      *loc1 = color;
			    }
			  else
			    {
			      loc1 -= x_size;
			      *loc1 = color;
			    }
			}
		    }
		}
	    }
	}			/* End slope > 1/2 */
    }

  check_mouse2 (p, mouse);
}


#endif

/*****************************************************************************/
/*****************************************************************************/
