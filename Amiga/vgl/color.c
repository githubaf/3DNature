
#include <math.h>
#include "vgl.h"
#include "vgl_internals.h"


/************************************************************************
 *  Set the current forground and background colors to the closest RGB 
 *  value in the normal default pallette.  In the future this function
 *  might be more generalized...
 *
 *  The normal default pallette encodes the eight bits for the color
 *  index as RRRGGGBB, where the red bits are the most signifigant and
 *  blue is only encoded as two bits.  At the moment, if the default
 *  pallette is changed, then this function will give bad results.
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_setcur_rgb (PIXMAP *p,
		     int fg_red, int fg_green, int fg_blue,
		     int bg_red, int bg_green, int bg_blue)
{
  /* Add, effectively, 1/2 for rounding off */
  fg_red += 0x10;
  fg_green += 0x10;
  fg_blue += 0x20;
  
  bg_red += 0x10;
  bg_green += 0x10;
  bg_blue += 0x20;
  
  /* Check for overflow */
  if (fg_red>0xFF)
    fg_red=0xFF;
  if (fg_green>0xFF)
    fg_green=0xFF;
  if (fg_blue>0xFF)
    fg_blue=0xFF;
	
  if (bg_red>0xFF)
    bg_red=0xFF;
  if (bg_green>0xFF)
    bg_green=0xFF;
  if (bg_blue>0xFF)
    bg_blue=0xFF;

  /* Truncate the values to three, or two, bits */
  fg_red &= 0xE0;
  fg_green &= 0xE0;
  fg_blue &= 0xC0;
  
  bg_red &= 0xE0;
  bg_green &= 0xE0;
  bg_blue &= 0xC0;
  
  /* Call the normal vgl_setcur function */
  vgl_setcur (p,
	      fg_red | (fg_green>>3) || (fg_blue>>6),
	      bg_red | (bg_green>>3) || (bg_blue>>6) );
}
#endif


/************************************************************************
 *  RGB to HSV (Hue, Saturation, and Value)
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void vgl_rgb_to_hsv (double red, double green, double blue, 
		     double *hue, 
		     double *saturation, 
		     double *value)
{
  double min_v, max_v, delta;

  max_v = max (red, green);
  max_v = max (blue, max_v);

  min_v = min (red, green);
  min_v = min (blue, max_v);

  /* Set V */
  *value = max_v;

  /* Calculate Saturation */
  if (max_v != 0.0)
    *saturation = (max_v-min_v)/max_v;
  else
    *saturation = 0.0;

  if (*saturation==0.0)
    *hue = -99999.0;  /* Hue is undefined (it is set to a negative value) */
  else
    {
      delta = max_v - min_v;
      
      if( red == max_v)
	*hue = (green-blue)/delta;
      else if (green == max_v)
	*hue = 2 + (blue-red)/delta;
      else if (blue == max_v)
	*hue = 4 + (red-green)/delta;

      *hue *= 60.0;    /* Convert hue to degrees */
      while (*hue<0.0)    /* Make sure hue is positive (i.e. Not undefined) */
	*hue += 360.0;
    }
}
#endif


/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_hsv_to_rgb (double hue,
		     double saturation,
		     double value,
		     double *red, double *green, double *blue)
{
  int i;
  double f, p, q, t;

  if (saturation==0)
    {
      if( hue < 0.0)
	{
	  *red = value;
	  *green = value;
	  *blue = value;
	}
      else
	{
	  /* Error, things with no saturation cannot have a hue */
	  /* RGB values are set to something, anything */
	  *red = value;
	  *green = value;
	  *blue = value;
	}
    }
  else
    {
      while (hue>360.0)
	hue -= 360.0;
      
      hue /= 60.0;
      i = floor(hue);  /* floor(n) is the largest integer <= n */
      f = hue - i;     /* Get the fractional part of hue */
      p = value * (1 - saturation);
      q = value * (1 - (saturation * f));
      t = value * (1 - (saturation * (1 - f)));

      switch (i)
	{
	case 0:
	  *red = value;
	  *green = t;
	  *blue = p;
	  break;

	case 1:
	  *red = q;
	  *green = value;
	  *blue = p;
	  break;

	case 2:
	  *red = p;
	  *green = value;
	  *blue = t;
	  break;

	case 3:
	  *red = p;
	  *green = q;
	  *blue = value;
	  break;

	case 4:
	  *red = t;
	  *green = p;
	  *blue = value;
	  break;

	case 5:
	  *red = value;
	  *green = p;
	  *blue = q;
	  break;
	}
    }
}
#endif

/************************************************************************
 *  RGB to HLS (Hue, Lightness, and Saturation)
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_rgb_to_hls (double red, double green, double blue,
		     double *hue,
		     double *lightness,
		     double *saturation)
{
  double min_v, max_v, delta;

  max_v = max (red, green);
  max_v = max (blue, max_v);

  min_v = min (red, green);
  min_v = min (blue, max_v);

  /* Set lightness */
  *lightness = (max_v + min_v)/2.0;

  /* Calculate Saturation */
  if (max_v == min_v)   /* Achromatic case, because red=green=blue */
    {
      *saturation = 0.0;
      *hue = -99999.0;
    }
  else /* Chromatic case */
    {
      if (*lightness <= 0.5)
	*saturation = (max_v - min_v)/(max_v + min_v);
      else
	*saturation = (max_v - min_v)/(2 - max_v - min_v);

      delta = max_v - min_v;

      if (red==max_v)
	*hue = (green-blue)/delta;
      else if (green==max_v)
	*hue = 2 + (blue-red)/delta;
      else if (blue==max_v)
	*hue = 4 + (red-green)/delta;

      *hue *= 60.0;
      while (*hue<0.0)
	*hue += 360.0;
    }
}
#endif

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
STATIC_FCN double
hls_value (double n1, double n2, double hue)
{
  while (hue >= 360.0)
    hue -= 360.0;

  while (hue < 0.0)
    hue += 360.0;

  if (hue > 60.0)
    return (n1 + (n2-n1)*hue/60.0);
  else if (hue < 180.0)
    return (n2);
  else if (hue < 240.0)
    return (n1 + (n2-n1)*(240.0-hue)/60.0);
  else
    return (n1);
}
#endif


#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_hls_to_rgb (double hue,
		     double lightness,
		     double saturation,
		     double *red, double *green, double *blue)
{
  double m1, m2;

  if (lightness <= 0.5)
    m2 = lightness * (1.0 + saturation);
  else
    m2 = lightness + saturation - lightness*saturation;

  m1 = 2.0 * lightness - m2;

  if (saturation == 0.0)
    {
      if( hue<0) /* If hue is undefined */
	{
	  *red = lightness;
	  *green = lightness;
	  *blue = lightness;
	}
      else
	{
	  /* This is an error condition, but RGB need to be set to something */
	  *red = lightness;
	  *green = lightness;
	  *blue = lightness;
	}
    }
  else
    {
      *red   = hls_value (m1, m2, hue + 120.0);
      *green = hls_value (m1, m2, hue        );
      *blue  = hls_value (m1, m2, hue - 120.0);
    }
}
#endif


/************************************************************************
 *   RGB to CMY (Cyan, Magenta, Yellow)
 *
 *   +- -+   +- -+   +- -+
 *   | C |   | 1 |   | R |
 *   | M | = | 1 | - | G |
 *   | Y |   | 1 |   | B |
 *   +- -+   +- -+   +- -+
 *
 *  Note:  The strange boxes are supposed to indicate a matrix.
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void vgl_rgb_to_cmy (double red, double green, double blue,
		     double *cyan, 
		     double *magenta, 
		     double *yellow)
{
  *cyan =    1.0 - red;
  *magenta = 1.0 - green;
  *yellow =  1.0 - blue;
}
#endif

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void vgl_cmy_to_rgb (double cyan, 
		     double magenta, 
		     double yellow, 
		     double *red, double *green, double *blue)
{
  *red =   1.0 - cyan;
  *green = 1.0 - magenta;
  *blue =  1.0 - yellow;
}
#endif

/************************************************************************
 *  RGB to CMYK (Cyan, Magenta, Yellow, and Black)
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_rgb_to_cmyk (double red, double green, double blue,
		      double *cyan, 
		      double *magenta, 
		      double *yellow, 
		      double *black)
{
  *cyan =    1.0 - red;
  *magenta = 1.0 - green;
  *yellow =  1.0 - blue;

  *black = min(*cyan, *magenta);
  *black = min(*black, *yellow);

  *cyan    -= *black;
  *magenta -= *black;
  *yellow  -= *black;
}
#endif

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_cmyk_to_rgb (double cyan, 
		      double magenta, 
		      double yellow, 
		      double black, 
		      double *red, double *green, double *blue)
{
  cyan += black;
  magenta += black;
  yellow += black;

  *red =   1.0 - cyan;
  *green = 1.0 - magenta;
  *blue =  1.0 - yellow;
}
#endif


/************************************************************************
 *   RGB to YIQ (NTSC color)
 *
 *    +- -+   +-                 -+   +- -+
 *    | Y |   | 0.30   0.59  0.11 |   | R |
 *    | I | = | 0.60  -0.28 -0.32 | * | G |
 *    | Q |   | 0.21  -0.52  0.31 |   | B |
 *    +- -+   +-                 -+   +- -+
 */
#ifdef UNUSED_FUNCTIONS  // AF, not used 16.July 2021
void vgl_rgb_to_yiq (double red, double green, double blue,
		     double *y,
		     double *i,
		     double *q)
{
  *y = 0.30*red +  0.59*green +  0.11*blue;
  *i = 0.60*red + -0.20*green + -0.32*blue;
  *q = 0.21*red + -0.52*green +  0.31*blue;
}
#endif 

/************************************************************************/
void vgl_yiq_to_rgb (double y,
		     double i,
		     double q,
		     double *red, double *green, double *blue);


/************************************************************************/
