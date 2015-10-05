
/*
  #include <stdio.h>
*/

#include "vgl.h"
#include "vgl_internals.h"

static int vgl_mem_test_stuck_bit (void *addr, unsigned long size, char *msg);
static int vgl_mem_test_addr_lines (void *addr, unsigned long size, char *msg);
static int vgl_mem_test_random (void *addr, unsigned long size, char *msg);

static unsigned long vgl_gen_rand( unsigned long *seed, unsigned long mask);


/*****************************************************************************/
/*****************************************************************************/
int vgl_mem_test (void *addr, unsigned long size, char *msg)
{
  int err;

  err = vgl_mem_test_stuck_bit (addr, size, msg);
  if( err!=VGL_TEST_OK)
    return (err);

  err = vgl_mem_test_addr_lines (addr, size, msg);
  if( err!=VGL_TEST_OK)
    return (err);

  err = vgl_mem_test_random (addr, size, msg);
  if( err!=VGL_TEST_OK)
    return (err);

  msg[0]='\0';
  return (VGL_TEST_OK);
}

/*****************************************************************************/
/*****************************************************************************/
static int vgl_mem_test_stuck_bit (void *addr, unsigned long size, char *msg)
{
  unsigned long count,  *a;
  unsigned char	*b;

  /* Long word check */
  /* Fill with one pattern */
  for( count=size/4, a=addr; count>0; count--)
    *(a++)=0x55555555;
  
  /* Verify the pattern  */
  for( count=size/4, a=addr; count>0; count--)
    if( *(a++) != 0x55555555)
      {
	a--;
	sprintf (msg,
		 "Error at %08lX.  Expected 0x55555555, Read 0x%08X.",
		 (unsigned long)a,
		 *a);
	return (VGL_TEST_STUCK32);	
      }

  /* Fill with another pattern */
  for( count=size/4, a=addr; count>0; count--)
    *(a++)=0xAAAAAAAA;

  /* Verify the pattern  */
  for( count=size/4, a=addr; count>0; count--)
    if( *(a++) != 0xAAAAAAAA)
      {
	a--;
	sprintf (msg,
		 "Error at %08lX.  Expected 0xAAAAAAAA, Read 0x%08X.",
		 (unsigned long)a,
		 *a);
	return( VGL_TEST_STUCK32);	
      }


  /* byte check */
  /* Fill with one pattern */
  for( count=size, b=addr; count>0; count--)
    *(b++)=0x55;

  /* Verify the pattern  */
  for( count=size, b=addr; count>0; count--)
    if( *(b++) != 0x55)
      {
	b--;
	sprintf (msg,
		 "Error at %08lX.  Expected 0x55, Read 0x%02X.",
		 (unsigned long)b,
		 *b);
	return( VGL_TEST_STUCK8);	
      }

  /* Fill with another pattern */
  for( count=size, b=addr; count>0; count--)
    *(b++)=0xAA;

  /* Verify the pattern  */
  for( count=size, b=addr; count>0; count--)
    if( *(b++) != 0xAA)
      {
	b--;
	sprintf (msg,
		 "Error at %08lX.  Expected 0xAA, Read 0x%02X.",
		 (unsigned long)b,
		 (int)*b);
	return( VGL_TEST_STUCK8);	
      }


  return(VGL_TEST_OK);
}


/*****************************************************************************/
static int vgl_mem_test_addr_lines (void *addr, unsigned long size, char *msg)
{
  unsigned long count, value, start;
  unsigned char *b;
  unsigned long *a;
  static unsigned long	starting_seed=42;

  /* Do a byte check */
  /* Fill with one pattern */
  start = value = vgl_gen_rand( &starting_seed, 0x00D80000);
  for( count=size, b=addr; count>0; count--)
    *(b++) = vgl_gen_rand( &value, 0xA3000000) & 0xFF;

  /* Verify the pattern  */
  value = start;
  for( count=size, b=addr; count>0; count--)
    if( *(b++) != (vgl_gen_rand( &value, 0xA3000000) & 0xFF))
	return( VGL_TEST_STUCK8);


  /* Do a long word check */
  /* Fill with one pattern */
  start = value = vgl_gen_rand( &starting_seed, 0x00D80000);
  for( count=size/4, a=addr; count>0; count--)
    *(a++) = vgl_gen_rand( &value, 0xA3000000);

  /* Verify the pattern  */
  value = start;
  for( count=size/4, a=addr; count>0; count--)
    if( *(a++) != vgl_gen_rand( &value, 0xA3000000))
	return( VGL_TEST_STUCK32);

  return(VGL_TEST_OK);
}


/*****************************************************************************/
static int vgl_mem_test_random (void *addr, unsigned long size, char *msg)
{
  unsigned long mask, start, value, value_start, pos;
  int n_bits;
  unsigned char *d;
  static unsigned long start_seed = 92569;

  static unsigned long mask_table[] =
    {
      0, 0, 0x03, 0x06, 0x0c, 0x14, 0x30, 0x60, 0xb8, 0x0110, 0x0240, 0x0500,
      0x0ca0, 0x1b00, 0x3500, 0x6000, 0xB400, 0x012000, 0x020400, 0x072000,
      0x090000, 0x0140000, 0x00300000, 0x00400000, 0x00D80000, 0x01200000,
      0x03880000, 0x07200000, 0x09000000, 0x14000000, 0x32800000, 0x48000000,
      0xA3000000
    };

  for (n_bits = 16; size > (1 << n_bits); n_bits++)
    ;
  mask = mask_table[n_bits];
  
  start = vgl_gen_rand( &start_seed, 0x00012000);
  value_start = vgl_gen_rand( &start_seed, 0x00012000);
      
  d = addr;

  /* Write a screen full */
  value = value_start;
  pos = start;
  do
    {
      if (pos < size)
	d[pos] = value & 0xFF;
     
      vgl_gen_rand( &value, 0xA3000000);
      vgl_gen_rand( &pos, mask);
    }  while (pos != start);

  /* Read a screen full */
  value = value_start;
  pos = start;
  do
    {
      if (pos < size)
	{
	  if( d[pos] != (value & 0xFF))
	    return( VGL_TEST_RANDOM);
	}
      
      vgl_gen_rand( &value, 0xA3000000);
      vgl_gen_rand( &pos, mask);
    }  while (pos != start);

  return (VGL_TEST_OK);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
static unsigned long vgl_gen_rand( unsigned long *seed, unsigned long mask)
{
if ((*seed) & 1)
  *seed = (*seed) >> 1 ^ mask;
else
  *seed >>= 1;

return(*seed);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#ifdef VGL_TEST_APP

#ifdef VXWORKS
#include <taskLib.h>
#endif

char	*ultoa( char *s, unsigned long v);

void	do_keys( void);
void	do_circles( void);
void	do_mouse( void);
long	ihypot( long x, long y);

static unsigned long	rand_last;
#define		RAND_A	(1664525UL)
#define		RAND_C	(32767UL)
#define	random(M)	(((rand_last=(RAND_A * rand_last + RAND_C))>>4)%M)
#define	srand(SEED)	(rand_last=SEED)


struct button_table
	{
	  int	         w, h;
	  PIXMAP	*up, *down;
	};

static struct button_table button_table[] =
	{
	  { 2, 1, NULL, NULL},
	  { 3, 1, NULL, NULL},
	  { 4, 1, NULL, NULL},
	  { 5, 1, NULL, NULL},
	  {13, 1, NULL, NULL},
	  { 2, 2, NULL, NULL}
	};

#define MAX_BUTTON_TABLE	(sizeof(button_table)/sizeof(button_table[0]))


struct key_table
	{
	  int	num;
	  int	r, c, button_num;
	  char	label[8];
	};

static struct key_table key_table[] =
	{
	  {   1, 0,  0, 0, "Esc"},
	  {   2, 0,  4, 0, "F1"},
	  {   3, 0,  6, 0, "F2"},
	  {   4, 0,  8, 0, "F3"},
	  {   5, 0, 10, 0, "F4"},
	  {   6, 0, 13, 0, "F5"},
	  {   7, 0, 15, 0, "F6"},
	  {   8, 0, 17, 0, "F7"},
	  {   9, 0, 19, 0, "F8"},
	  {  10, 0, 22, 0, "F9"},
	  {  11, 0, 24, 0, "F10"},
	  {  12, 0, 26, 0, "F11"},
	  {  13, 0, 28, 0, "F12"},
	  {  14, 0, 31, 0, "PSn"},
	  {  15, 0, 33, 0, "SLk"},
	  {  16, 0, 35, 0, "Brk"},

	  {  17, 2,  0, 0, "~ `"},
	  {  18, 2,  2, 0, "! 1"},
	  {  19, 2,  4, 0, "@ 2"},
	  {  20, 2,  6, 0, "# 3"},
	  {  21, 2,  8, 0, "$ 4"},
	  {  22, 2, 10, 0, "% 5"},
	  {  23, 2, 12, 0, "^ 6"},
	  {  24, 2, 14, 0, "& 7"},
	  {  25, 2, 16, 0, "* 8"},
	  {  26, 2, 18, 0, "( 9"},
	  {  27, 2, 20, 0, ") 0"},
	  {  28, 2, 22, 0, "_ -"},
	  {  29, 2, 24, 0, "+ ="},
	  {  30, 2, 26, 2, "Bksp"},
	  {  31, 2, 31, 0, "Ins"},
	  {  32, 2, 33, 0, "Hme"},
	  {  33, 2, 35, 0, "PUp"},
	  {  34, 2, 38, 0, "Num"},
	  {  35, 2, 40, 0, "/"},
	  {  36, 2, 42, 0, "*"},
	  {  37, 2, 44, 0, "-"},

	  {  38, 3,  0, 1, "Tab"},
	  {  39, 3,  3, 0, "Q"},
	  {  40, 3,  5, 0, "W"},
	  {  41, 3,  7, 0, "E"},
	  {  42, 3,  9, 0, "R"},
	  {  43, 3, 11, 0, "T"},
	  {  44, 3, 13, 0, "Y"},
	  {  45, 3, 15, 0, "U"},
	  {  46, 3, 17, 0, "I"},
	  {  47, 3, 19, 0, "O"},
	  {  48, 3, 21, 0, "P"},
	  {  49, 3, 23, 0, "{ ["},
	  {  50, 3, 25, 0, "} ]"},
	  {  51, 3, 27, 1, "| \\"},
	  {  52, 3, 31, 0, "Del"},
	  {  53, 3, 33, 0, "End"},
	  {  54, 3, 35, 0, "PDn"},
	  {  55, 3, 38, 0, "7"},
	  {  56, 3, 40, 0, "8"},
	  {  57, 3, 42, 0, "9"},
	  {  58, 3, 44, 5, "+"},

	  {  59, 4,  0, 2, "Caps"},
	  {  60, 4,  4, 0, "A"},
	  {  61, 4,  6, 0, "S"},
	  {  62, 4,  8, 0, "D"},
	  {  63, 4, 10, 0, "F"},
	  {  64, 4, 12, 0, "G"},
	  {  65, 4, 14, 0, "H"},
	  {  66, 4, 16, 0, "J"},
	  {  67, 4, 18, 0, "K"},
	  {  68, 4, 20, 0, "L"},
	  {  69, 4, 22, 0, ": ;"},
	  {  70, 4, 24, 0, "\" '"},
	  {  71, 4, 26, 2, "Enter"},
	  {  72, 4, 38, 0, "4"},
	  {  73, 4, 40, 0, "5"},
	  {  74, 4, 42, 0, "6"},

	  {  75, 5,  0, 3, "Shift"},
	  {  76, 5,  5, 0, "Z"},
	  {  77, 5,  7, 0, "X"},
	  {  78, 5,  9, 0, "C"},
	  {  79, 5, 11, 0, "V"},
	  {  80, 5, 13, 0, "B"},
	  {  81, 5, 15, 0, "N"},
	  {  82, 5, 17, 0, "M"},
	  {  83, 5, 19, 0, "< ,"},
	  {  84, 5, 21, 0, "> ."},
	  {  85, 5, 23, 0, "? /"},
	  {  86, 5, 25, 3, "Shift"},
	  {  87, 5, 33, 0, "Up"},
	  {  88, 5, 38, 0, "1"},
	  {  89, 5, 40, 0, "2"},
	  {  90, 5, 42, 0, "3"},
	  {  91, 5, 44, 5, "Ret"},

	  {  92, 6,  0, 1, "Ctrl"},
	  {  93, 6,  6, 1, "Alt"},
	  {  94, 6,  9, 4, "Space"},
	  {  95, 6, 22, 1, "Alt"},
	  {  96, 6, 27, 1, "Ctrl"},
	  {  97, 6, 31, 0, "Lft"},
	  {  98, 6, 33, 0, "Dn"},
	  {  99, 6, 35, 0, "Rt"},
	  { 100, 6, 38, 2, "0"},
	  { 101, 6, 42, 0, "."}
	};


#define MAX_KEY_TABLE	(sizeof(key_table)/sizeof(key_table[0]))


struct flag_table
	{
	  unsigned long	flag;
	  int		x, y, d;
	  char		label[32];
	};

static struct flag_table flag_table[] =
	{
	  {  CAPS_LOCK,  0, 0, 1, "Caps Lock"},
	  {  NUM_LOCK,  0, 1, 1, "Num Lock"},
	  {  SCRL_LOCK,  0, 2, 1, "Scroll Lock"},
	  {  L_SHIFT,  1, 0, 1, "Left Shift"},
	  {  R_SHIFT,  1, 1, 1, "Right Shift"},
	  {  L_CTRL,  2, 0, 1, "Left Control"},
	  {  R_CTRL,  2, 1, 1, "Right Control"},
	  {  L_ALT,  3, 0, 1, "Left Alt"},
	  {  R_ALT,  3, 1, 1, "Right Alt"},
/*
	  {  1<<20,  2, 2, 1, "Extended Left Control"},
	  {  1<<21,  2, 3, 1, "Extended Right Control"},
	  {  1<<16,  1, 2, 1, "Extended Left Shift"},
	  {  1<<17,  1, 3, 1, "Extended Right Shift"},
	  {  1<<9,   3, 3, 1, "PREFIX_Exxx"},
*/
	};

#define MAX_FLAG_TABLE	(sizeof(flag_table)/sizeof(flag_table[0]))

#define key_size(p)	(vgl_x_res(p)/46)

#define rgb(rrr,ggg,bbb) ((((rrr)&0x7)<<5)|(((ggg)&0x07)<<2)|(((bbb)&0x06)>>1))

#define BACKGROUND   rgb(4,4,4)

#define	T0  rgb( 7, 7, 7)
#define T1  rgb( 6, 6, 6)
#define T2  rgb( 5, 5, 5)

#define M2  rgb( 5, 5, 5)
#define M1  rgb( 4, 4, 4)

#define A2  rgb( 4, 4, 4)
#define A1  rgb( 3, 3, 3)
#define A0  rgb( 2, 2, 2)


void make_button( struct button_table *b, PIXMAP *p);
void draw_key( PIXMAP *p, unsigned long key_num, unsigned long state);
void draw_flag( PIXMAP *p, int flag_num, int state);

VGBOARD	*b;
PIXMAP	*p;
unsigned long	tid, targ[4];


#define MED_RES

#ifdef LOW_RES
#define	MODE		"VGA_640x480@60Hz"
#define BOARD		"default"
#define BORDER		(3)
#define CORNER		(3)
#define LED_SIZE	(3)
#define SHADOW		(2)
#define FONT		(vgl_small_ffont)
#endif

#ifdef MED_RES
#define	MODE		"VGA_1024x768@60Hz"
#define BOARD		"default"
#define BORDER		(4)
#define CORNER		(6)
#define LED_SIZE	(7)
#define SHADOW		(3)
#define FONT		(vgl_large_ffont)
#endif

#ifdef HIGH_RES
#define	MODE		"VGA_1280x1024@60Hz"
#define BOARD		"MMI-250"
#define BORDER		(5)
#define CORNER		(7)
#define LED_SIZE	(8)
#define SHADOW		(3)
#define FONT		(vgl_large_ffont)
#endif

void vgl_test(void)
{
  PIXMAP		*mouse;
  unsigned long		pri;


  b=vgl_initboard( BOARD, MODE);
  if( b)
    {
      p=vgl_pixmap_map( b, 0);

      if( p)
	{
	  mouse=vgl_makepixmap( 24, 24);
	  vgl_clear(mouse);
	  vgl_set_clip( mouse, 0, 0, -1, -1);
	  vgl_setcur( mouse, 0xC0, 0);
	  vgl_fillcircle( mouse, 8, 8, 8);
	  vgl_setcur( mouse, 0, 0);
	  vgl_fillcircle( mouse, 8, 8, 3);
	  vgl_setcur( mouse, 0xFF, 0);
	  vgl_circle( mouse, 8, 8, 8);
	  vgl_mouse_setpixmap( b, mouse, 8, 8);
	  vgl_mouse_ptr_on( b);
	
	  vgl_ss_set( b, SS_NORMAL, 18000, SS_ALL ^ SS_DRAW);

	  vgl_clear( p);
	  vgl_setfont( p, FONT);

#ifdef VXWORKS
	  taskPriorityGet( taskIdSelf(), &pri);
	  taskSpawn ("do_keys",    
		     pri, 0, 10240, do_keys,
		     0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	  taskSpawn ("do_circles", 
		     pri+1, 0, 10240, do_circles,
		     0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
/*
	  taskSpawn ("do_mouse",
		     pri, 0, 10240, do_mouse,
		     0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
*/
	  do_mouse();

#endif

#ifdef PSOS
	  t_setpri( 0, 0, &pri);

	  t_create( "ABCD", pri, 10240, 10240, T_GLOBAL | T_NOFPU, &tid);
	  t_start( tid, T_PREEMPT | T_TSLICE | T_USER, do_keys, targ);

	  t_create( "ABCD", pri, 10240, 10240, T_GLOBAL | T_NOFPU, &tid);
	  t_start( tid, T_PREEMPT | T_TSLICE | T_USER, do_circles, targ);

	  t_create( "ABCD", pri, 10240, 10240, T_GLOBAL | T_NOFPU, &tid);
	  t_start( tid, T_PREEMPT | T_TSLICE | T_USER, do_mouse, targ);

	  t_suspend( 0);
#endif

	  vgl_freepixmap( p);
	}

      vgl_killboard( b);
    }
}

/***********************************************************************/
void   	do_keys( void)
{
  int	i;
  PIXMAP	*p1;
  unsigned long	v;


  p1=vgl_dupepixmap(p);

  for( i=0; i<MAX_BUTTON_TABLE; i++)
    make_button( button_table+i, p1);

  for( i=0; i<MAX_KEY_TABLE; i++)
    draw_key( p1, key_table[i].num, 1);

  for( i=0; i<MAX_FLAG_TABLE; i++)
    draw_flag( p1, i, 0);

  for(;;)
    {
      v=vgl_getkey( b, 1);
      
      draw_key( p1, v, (v&KEY_UP));
    
      for( i=0; i<MAX_FLAG_TABLE; i++)
	draw_flag( p1, i, flag_table[i].flag&v);

      vgl_kb_setled( b, v);
    }
}


/***********************************************************************/
void	do_circles( void)
{
  int	c, x_offset, y_offset, x_range, y_range, size;
  PIXMAP	*p1;

  p1=vgl_dupepixmap( p);

  x_offset = vgl_x_res(p1)/2;
  y_offset = vgl_y_res(p1)/2;
  x_range = vgl_x_res(p1)/2;
  y_range = vgl_y_res(p1)/2;
  size = vgl_y_res(p1)/4;
  vgl_set_clip( p1, x_offset, y_offset, x_offset+x_range, y_offset+y_range);
  vgl_clip_on( p1);

  c=0;
  for(;;c++)
    {
      if( (c%1000)==0)
	{
	  vgl_setcur( p1, 0, 0);
	  vgl_fillrect (p1, x_offset, y_offset, 
		        x_offset+x_range, y_offset+y_range);
	}
      vgl_setcur (p1, c&0xFF, 0);
      vgl_circle (p1, 
		  x_offset + random(x_range), 
		  y_offset + random(y_range), 
		  random(size));
    }
}

/***********************************************************************/
void	do_mouse( void)
{
  int	x, y, ox, oy, dx, dy, buttons, dist, rad;
  int	x_offset, y_offset, x_range, y_range, x_center, y_center, size;
  PIXMAP	*p1;

  p1=vgl_dupepixmap( p);

  x_offset = 0;
  y_offset = vgl_y_res(p1)/2;
  x_range = vgl_x_res(p1)/2;
  y_range = vgl_y_res(p1)/2;

  x_center = x_offset + x_range/2;
  y_center = y_offset + y_range/2;

  size = vgl_y_res(p1)/6;
  vgl_set_clip( p1, x_offset, y_offset, x_offset+x_range, y_offset+y_range);
  vgl_clip_on( p1);

  vgl_setcur( p1, rgb( 3, 3, 3), 0);
  vgl_fillcircle( p1, x_center, y_center, size);
  vgl_setcur( p1, rgb( 6, 6, 6), 0);
  vgl_fillcircle( p1, x_center, y_center, size-(size/10));

  ox=0;
  oy=0;
  rad = (size - (3*size)/10);

  for(;;)
    {
      vgl_mouse_getcoords( b, &x, &y, &buttons, 1);

      vgl_setcur( p1, rgb( 6, 6, 6), 0);
      vgl_fillcircle( p1, ox, oy, size/10);

      dx = x - x_center;
      dy = y - y_center;

      dist = ihypot( dx, dy);
      if( dist>rad)
	{
	  ox = x_center + (rad*dx)/dist;
	  oy = y_center + (rad*dy)/dist;
	}
      else
	{
	  ox = x;
	  oy = y;
	}

      vgl_setcur( p1, rgb( 0, 0, 0), 0);
      vgl_fillcircle( p1, ox, oy, size/10);
    }
}


/***********************************************************************/
long	ihypot( long x, long y)
{
  long	s, c, o;

  s = x*x + y*y;

  c=0;
  o=1;

  for(;;)
    {
      if( s==0)
	return(c);
      else if( s<0)
	return(c-1);
      else
	{
	  s-=o;
	  o+=2;
	  c++;
	}
    }
}


/***********************************************************************/
#define BASE		(16)
#define MAX_A_LEN	(8)
static char digits[] = "0123456789ABCDEF";

char	*ultoa( char *s, unsigned long v)
{
  int		i, j;
  static char	b[MAX_A_LEN+1];

  i=MAX_A_LEN;
  b[i--]=NULL;

  do
    {
      b[i--]=digits[v%BASE];
      v/=BASE;
    } while( v!=0 && i>=0);

  while( i>=0)
    b[i--]=' ';

  i++;

  if( s)
    {
      for( j=i; b[j]!=NULL; j++)
	*(s++)=b[j];

      *s=NULL;
    }

  return(b+i);
}


/***********************************************************************/
void draw_key( PIXMAP *p, unsigned long key_num, unsigned long state)
{
  int	i, x, y, h, w, tw, th;
  char	*s;
  static PIXMAP *temp=NULL;

  if( temp==NULL)
    {
      temp=vgl_makepixmap( p->font->width*30+1, p->font->height+1);
      vgl_setfont( temp, FONT);
    }

  for( i=0; i<MAX_KEY_TABLE; i++)
    {
      if( key_table[i].num == (0xFF & key_num) )
	{
	  x=key_table[i].c * key_size(p);
	  y=key_table[i].r * key_size(p) * 2;
	  w=button_table[key_table[i].button_num].w * key_size(p) - BORDER;
	  h=button_table[key_table[i].button_num].h * key_size(p) * 2 - BORDER;
	
	  for( s=key_table[i].label; *s!=NULL; s++);
	  tw= p->font->width * (s - key_table[i].label);
	  th= p->font->height;
	
	
	  if( state!=0)
	    {			/* Up */
	      vgl_bitblt (button_table[key_table[i].button_num].up, 
			  0, 0, -1, -1, p, x, y);

	      vgl_setcur( p, 0x00, BACKGROUND);
	      vgl_text( p, 1+x+(w-tw)/2, 1+y+(h-th)/2, key_table[i].label);

	      vgl_setcur( temp, 0xDB, 0x00);
	      vgl_clear(temp);
	      vgl_text( temp, 0, 0, key_table[i].label);
	      vgl_transbitblt (temp, 0, 0, tw, th, 
			       p, x+(w-tw)/2, y+(h-th)/2, 0);
	    }
	  else
	    {			/* Down */
	      vgl_bitblt (button_table[key_table[i].button_num].down, 
			  0, 0, -1, -1, p, x, y);

	      vgl_setcur( p, 0x00, BACKGROUND);
	      vgl_text (p, SHADOW+x+(w-tw)/2, SHADOW+y+(h-th)/2, 
			key_table[i].label);

	      vgl_setcur( temp, 0xDB, 0x00);
	      vgl_clear( temp);
	      vgl_text( temp, 0, 0, key_table[i].label);
	      vgl_transbitblt (temp, 0, 0, tw, th, 
			       p, x+(w-tw)/2, y+(h-th)/2, 0);
	    }
	}
    }
}


/***********************************************************************/

void	make_button( struct button_table *b, PIXMAP *p)
{
  int	x, y, w, h;

  x=0;
  y=0;
  w=b->w * key_size(p)     - BORDER;
  h=b->h * key_size(p) * 2 - BORDER;
	

  b->up = vgl_makepixmap( w+1, h+1);
  b->down = vgl_makepixmap( w+1, h+1);

  vgl_setfont( b->up, FONT);
  vgl_setfont( b->down, FONT);


  /* Key Up */
  vgl_setcur( b->up, BACKGROUND, 0);
  vgl_clear( b->up);
  vgl_fillcircle( b->up, x+  CORNER, y+  CORNER, CORNER);
  vgl_fillcircle( b->up, x+w-CORNER, y+  CORNER, CORNER);
  vgl_fillcircle( b->up, x+  CORNER, y+h-CORNER, CORNER);
  vgl_fillcircle( b->up, x+w-CORNER, y+h-CORNER, CORNER);
    
  vgl_setcur( b->up, T0, BACKGROUND);
  vgl_circle( b->up, x+  CORNER, y+  CORNER, CORNER);
  vgl_setcur( b->up, M1, BACKGROUND);
  vgl_circle( b->up, x+w-CORNER, y+  CORNER, CORNER);
  vgl_circle( b->up, x+  CORNER, y+h-CORNER, CORNER);
  vgl_setcur( b->up, A0, BACKGROUND);
  vgl_circle( b->up, x+w-CORNER, y+h-CORNER, CORNER);

  vgl_setcur( b->up, T2, BACKGROUND);
  vgl_circle( b->up, x+  CORNER, y+  CORNER, CORNER-1);
  vgl_setcur( b->up, M2, BACKGROUND);
  vgl_circle( b->up, x+w-CORNER, y+  CORNER, CORNER-1);
  vgl_circle( b->up, x+  CORNER, y+h-CORNER, CORNER-1);
  vgl_setcur( b->up, A2, BACKGROUND);
  vgl_circle( b->up, x+w-CORNER, y+h-CORNER, CORNER-1);

  vgl_setcur( b->up, T1, BACKGROUND);
  vgl_line( b->up, x+CORNER, y,        x+w-CORNER, y);
  vgl_line( b->up, x,        y+CORNER, x,          y+h-CORNER);
  vgl_setcur( b->up, A1, BACKGROUND);
  vgl_line( b->up, x+CORNER, y+h,      x+w-CORNER, y+h);
  vgl_line( b->up, x+w,      y+CORNER, x+w,        y+h-CORNER);

  vgl_setcur( b->up, T2, BACKGROUND);
  vgl_line( b->up, x+CORNER, y+1,      x+w-CORNER, y+1);
  vgl_line( b->up, x+1,      y+CORNER, x+1,        y+h-CORNER);
  vgl_setcur( b->up, A2, BACKGROUND);
  vgl_line( b->up, x+CORNER, y+h-1,    x+w-CORNER, y+h-1);
  vgl_line( b->up, x+w-1,    y+CORNER, x+w-1,      y+h-CORNER);

  vgl_setcur( b->up, BACKGROUND, 0);
  vgl_fillrect( b->up, x+2, y+CORNER, x+w-2, y+h-CORNER);
  vgl_fillrect( b->up, x+CORNER, y+2, x+w-CORNER, y+h-2);


  /* Key Down */
  vgl_setcur( b->down, BACKGROUND, 0);
  vgl_clear( b->down);
  vgl_fillcircle( b->down, x+  CORNER, y+  CORNER, CORNER);
  vgl_fillcircle( b->down, x+w-CORNER, y+  CORNER, CORNER);
  vgl_fillcircle( b->down, x+  CORNER, y+h-CORNER, CORNER);
  vgl_fillcircle( b->down, x+w-CORNER, y+h-CORNER, CORNER);
    
  vgl_setcur( b->down, A0, BACKGROUND);
  vgl_circle( b->down, x+  CORNER, y+  CORNER, CORNER);
  vgl_setcur( b->down, M2, BACKGROUND);
  vgl_circle( b->down, x+w-CORNER, y+  CORNER, CORNER);
  vgl_circle( b->down, x+  CORNER, y+h-CORNER, CORNER);
  vgl_setcur( b->down, T0, BACKGROUND);
  vgl_circle( b->down, x+w-CORNER, y+h-CORNER, CORNER);

  vgl_setcur( b->down, A2, BACKGROUND);
  vgl_circle( b->down, x+  CORNER, y+  CORNER, CORNER-1);
  vgl_setcur( b->down, M1, BACKGROUND);
  vgl_circle( b->down, x+w-CORNER, y+  CORNER, CORNER-1);
  vgl_circle( b->down, x+  CORNER, y+h-CORNER, CORNER-1);
  vgl_setcur( b->down, T2, BACKGROUND);
  vgl_circle( b->down, x+w-CORNER, y+h-CORNER, CORNER-1);

  vgl_setcur( b->down, A1, BACKGROUND);
  vgl_line( b->down, x+CORNER, y,        x+w-CORNER, y);
  vgl_line( b->down, x,        y+CORNER, x,          y+h-CORNER);
  vgl_setcur( b->down, T1, BACKGROUND);
  vgl_line( b->down, x+CORNER, y+h,      x+w-CORNER, y+h);
  vgl_line( b->down, x+w,      y+CORNER, x+w,        y+h-CORNER);

  vgl_setcur( b->down, A2, BACKGROUND);
  vgl_line( b->down, x+CORNER, y+1,      x+w-CORNER, y+1);
  vgl_line( b->down, x+1,      y+CORNER, x+1,        y+h-CORNER);
  vgl_setcur( b->down, T2, BACKGROUND);
  vgl_line( b->down, x+CORNER, y+h-1,    x+w-CORNER, y+h-1);
  vgl_line( b->down, x+w-1,    y+CORNER, x+w-1,      y+h-CORNER);

  vgl_setcur( b->down, BACKGROUND, 0);
  vgl_fillrect( b->down, x+2, y+CORNER, x+w-2, y+h-CORNER);
  vgl_fillrect( b->down, x+CORNER, y+2, x+w-CORNER, y+h-2);
}



/***********************************************************************/
void	draw_flag( PIXMAP *p, int flag_num, int state)
{
  int	x, y;

  x = 20 + vgl_x_res(p)/4*flag_table[flag_num].x;
  y = key_size(p)*15 + 1*p->font->height*flag_table[flag_num].y;

  vgl_setcur( p, 0, 0);
  vgl_fillcircle( p, x, y+(p->font->height/2), LED_SIZE);

  vgl_setcur( p, 0x1C, 0);
  if(state)
    vgl_fillcircle( p, x, y+(p->font->height/2), LED_SIZE);
  else
    vgl_circle( p, x, y+(p->font->height/2), LED_SIZE);

  if( flag_table[flag_num].d!=0)
    {
      vgl_setcur( p, 0xDB, 0);
      vgl_text( p, x+2*LED_SIZE, y, flag_table[flag_num].label);
      flag_table[flag_num].d=0;
    }
}


#endif /* VGL_TEST_APP */
