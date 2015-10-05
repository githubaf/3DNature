
#include "vgl.h"
#include "vgl_internals.h"


/*****************************************************************************/
/*****************************************************************************/
#ifdef USE_VGL_STRICMP
int 
vgl_stricmp (char *a, char *b)
{
  int i;

  while (1)
    {
      if (*a == '\0' && *b == '\0')
	return (0);
      else if (*a == '\0')
	return (-1);
      else if (*b == '\0')
	return (1);
      else
	{
	  i = *(a++) - *(b++);
	  if (i != 0)
	    return (i);
	}
    }
}
#endif


/*****************************************************************************/
/*****************************************************************************/
#ifdef USE_VGL_STRCPY
char *
vgl_strcpy (char *dest, char *source)
{
  while (*source != '\0')
      *(dest++) = *(source++);

  *dest = *source;

  return (dest);
}
#endif


/*****************************************************************************/
/*****************************************************************************/
#ifdef USE_VGL_BZERO
void 
vgl_bzero (void *s, int len)
{
  char *d;

  d = (char *) s;
  for (; len > 0; len--)
    *d++ = '\0';
}
#endif


/****************************************************************************/
/****************************************************************************/
#ifdef USE_VGL_STRCAT
char *vgl_strcat (char *a, char *b)
{
  for (;*a!=NULL; a++)
    ; /* Do nothing but incriment */

  while (*b!=NULL)
    *(a++) = *(b++);

  *a = NULL;

  return(a);
}

#endif


/****************************************************************************/
/****************************************************************************/
#ifdef USE_VGL_MEMCPY
void *vgl_memcpy (void *dest, void *source, size_t size)
{
  int i;
  char *a, *b, *c;

  a = dest;
  b = source;
  c = dest + size;

  /*  Don't try this at home kids.  These are trained professionals who
   *  dont mind getting hurt!
   */
  switch (size&7)
    {
    memcpyloop:
    case 8:  *(a++) = *(b++);
    case 7:  *(a++) = *(b++);
    case 6:  *(a++) = *(b++);
    case 5:  *(a++) = *(b++);
    case 4:  *(a++) = *(b++);
    case 3:  *(a++) = *(b++);
    case 2:  *(a++) = *(b++);
    case 1:  *(a++) = *(b++);
    case 0:
      if (a < c)
	goto memcpyloop;
    }

}

#endif


/****************************************************************************/
/****************************************************************************/
unsigned long 
vgl_isqrt (unsigned long n)
{
  unsigned long next, current;

  next = n >> 1;

  if (n<=1)
    {
      return (n);
    }

  do
    {
      current = next;
      next = (next + n/next) >> 1;
    } while (next<current);

  return (current);
}



/****************************************************************************/
/****************************************************************************/
unsigned long 
vgl_ihypot (unsigned long x, unsigned long y)
{
  unsigned long next, current, n;

  n = x*x + y*y;

  next = n >> 1;

  if (n<=1)
    {
      return (n);
    }

  do
    {
      current = next;
      next = (next + n/next) >> 1;
    } while (next<current);

  return (current);
}



/****************************************************************************/
/****************************************************************************/
/*  This is a random number generator, taken from Numerical Recipies in C
 *  In that book, the function is called ran2(), but has been renamed here.
 *
 *  Long period (>2e18) random number generator of L'Ecuyer with Bays-Durham
 *  shuffle and added safeguards.  Returns a uniform random deviate between
 *  0.0 and 1.0 (exclusive of the endpoint values).  Call with idum a
 *  negative integer to initalize;  thereafter, do not alter idum between
 *  sucessive deviates in a sequence.  RNMX should approximate the largest
 *  floating point value that is less than 1.0.
 */

#define RAND2_IM1  (2147483563)
#define RAND2_IM2  (2147483399)
#define RAND2_AM   (1.0/RAND2_IM1)
#define RAND2_IMM1 (RAND2_IM1-1)
#define RAND2_IA1  (40014)
#define RAND2_IA2  (40692)
#define RAND2_IQ1  (53668)
#define RAND2_IQ2  (52774)
#define RAND2_IR1  (12211)
#define RAND2_IR2  (3791)
#define RAND2_NTAB (32)
#define RAND2_NDIV (1+RAND2_IMM1/RAND2_NTAB)
#define RAND2_EPS  (1.2e-7)
#define RAND2_RNMX (1.0-RAND2_EPS)

float
vgl_random (long *idum)
{
  int j;
  long k;
  static long idum2 = 123456789;
  static long iy = 0;
  static long iv[RAND2_NTAB];
  float temp;

  if (*idum <= 0)  /* Initalize */
    {
      if (-(*idum) < 1)   /* Prevent idum == 0 */
	*idum = 1;
      else
	*idum = -(*idum);

      idum2 = *idum;

      for (j=RAND2_NTAB+7; j>=0; j--)  /* Load the shuffle table */
	{                              /* After 8 warmups */
	  k = *idum / RAND2_IQ1;
	  *idum = RAND2_IA1 * (*idum - k * RAND2_IQ1) - k *RAND2_IR1;

	  if (*idum < 0)
	    *idum += RAND2_IM1;

	  if (j<RAND2_NTAB)
	    iv[j] = *idum;
	}
    }

  k = *idum / RAND2_IQ1;   /* Compute idum=(IA1*idum)%IM1 without overflow */
  *idum = RAND2_IA1 * (*idum - k * RAND2_IQ1) - k * RAND2_IR1;
  if (*idum < 0)
    *idum += RAND2_IM1;

  k = idum2/RAND2_IQ2;    /* Compute idum2=(IA2*idum2)%IM2 without overflow */
  idum2 = RAND2_IA2 * (idum2 - k * RAND2_IQ2) - k * RAND2_IR2;
  if (idum2 < 0)
    idum2 += RAND2_IM2;

  j = iy / RAND2_NDIV;    /* j will be in the range of 0..NTAB-1 */
  iy = iv[j]-idum2;       /* Here idum is shuffles, idum and idum2 are */
  iv[j] = *idum;          /* combined to genreate output. */
  if (iy < 1)
    iy += RAND2_IMM1;

  if ((temp=RAND2_AM*iy) > RAND2_RNMX)  /* Because the user doesn't expect */
    return(RAND2_RNMX);                 /* endpoint values */
  else
    return (temp);
}

/****************************************************************************/
/****************************************************************************/
/*  This is the "current" random value for vgl_random_quick().  The function
 *  is implimented as a macro in vgl.h but requires one static external
 *  variable.
 */
extern unsigned long _vgl_rand_last=394857;


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


