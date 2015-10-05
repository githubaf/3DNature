
#include "vgl.h"
#include "vgl_internals.h"


#ifdef SUPPORT_VOGL

#include "vogl.h"


/*   This is the VOGL device name for the DevEntry structure.
 *   Defined here for your convienence.
 */
#define VGL_VOGL_DEV_NAME	"vogl_for_vgl"


/*   This is the DevEntry name.   Note, however, that the exact name does
 *   not matter, since it is defined as static and only referenced in this
 *   file anyway.
 */
#define VGL_VOGL_DEV_ENTRY_NAME vgl_vogl_dev


/*   This is the name of the function to copy the VGL device into vdevice.dev.
 */
#define VGL_VOGL_DEVCPY_NAME	vgl_vogl_devcpy


/*   The board name to use.  This is used in the call to vgl_initboard().
 */
#define BOARD_NAME		"DB_180"


/*   The video mode name to use.   Also used in the call to vgl_initboard().
 */
#define	MODE_NAME		"VGA_640x480@60Hz"


/*   The font names for the small and large fonts in VGL.  These are quite
 *   arbitrary and could be named Fred and Wilma if need be.
 */
#define VGL_VOGL_LARGE_FONT	"large"
#define VGL_VOGL_SMALL_FONT	"small"


/*   These are the values to be returned from these functions upon a good
 *   or bad outcome.
 */
#define VGL_VOGL_GOOD	(1)
#define VGL_VOGL_ERROR	(-1)


/*   These are the button flag values used in calls to vgl_vogl_locator.
 *   It is used to translate VGL button values to VOGL values.  They could
 *   be changed to aid in compatability.
 */
#define VGL_VOGL_MOUSE_LEFT	(1<<0)
#define VGL_VOGL_MOUSE_MID	(1<<1)
#define VGL_VOGL_MOUSE_RIGHT	(1<<2)


/*   The default colors for VOGL.  New colors may be added to the end and the
 *   rest of the code will just cope with it.
 */
static struct vgl_color vogl_colors[] =
  {
    {   0,   0,   0},	/*   0 - black   */
    { 255,   0,   0},	/*   1 - red     */	 
    {   0, 255,   0},	/*   2 - green   */
    { 255, 255,   0},	/*   3 - yellow  */
    {   0,   0, 255},	/*   4 - blue    */
    { 255,   0, 255},	/*   5 - magenta */
    {   0, 255, 255},	/*   6 - cyan    */
    { 255, 255, 255},	/*   7 - white   */
  };


/*   The keyboard table is used to decode keyboard events into something
 *   meaningful for VOGL.  Each entry contains a key number, a set of flags,
 *   and a value to return.  When a key is struck, the key value and flags
 *   are looked for in the table.  If a match is found, the integer value is
 *   returned.
 */
struct keyboard_table
  {
    int		  key_num;
    unsigned long flags;
    int		  return_code;
  };

static struct keyboard_table keyboard_table[] =
  {
    { 1, 0, 0x1B},  /* Esc */

    /* 2-13, Function Keys */
    /* Undefined */

    /* 14-16, Print Screen, Scroll Lock, and Pause */
    /* Undefined */

    { 17, 0, '`'},
    { 17, L_SHIFT, '~'},
    { 17, R_SHIFT, '~'},
    { 17, CAPS_LOCK, '~'},

    { 18, 0, '1'},
    { 18, L_SHIFT, '!'},
    { 18, R_SHIFT, '!'},
    { 18, CAPS_LOCK, '1'},

    { 19, 0, '2'},
    { 19, L_SHIFT, '@'},
    { 19, R_SHIFT, '@'},
    { 19, CAPS_LOCK, '2'},

    { 20, 0, '3'},
    { 20, L_SHIFT, '#'},
    { 20, R_SHIFT, '#'},
    { 20, CAPS_LOCK, '3'},

    { 21, 0, '4'},
    { 21, L_SHIFT, '$'},
    { 21, R_SHIFT, '$'},
    { 21, CAPS_LOCK, '4'},

    { 22, 0, '5'},
    { 22, L_SHIFT, '%'},
    { 22, R_SHIFT, '%'},
    { 22, CAPS_LOCK, '5'},

    { 23, 0, '6'},
    { 23, L_SHIFT, '^'},
    { 23, R_SHIFT, '^'},
    { 23, CAPS_LOCK, '6'},

    { 24, 0, '7'},
    { 24, L_SHIFT, '&'},
    { 24, R_SHIFT, '&'},
    { 24, CAPS_LOCK, '7'},

    { 25, 0, '8'},
    { 25, L_SHIFT, '*'},
    { 25, R_SHIFT, '*'},
    { 25, CAPS_LOCK, '8'},

    { 26, 0, '9'},
    { 26, L_SHIFT, '('},
    { 26, R_SHIFT, '('},
    { 26, CAPS_LOCK, '9'},

    { 27, 0, '0'},
    { 27, L_SHIFT, ')'},
    { 27, R_SHIFT, ')'},
    { 27, CAPS_LOCK, '0'},

    { 28, 0, '-'},
    { 28, L_SHIFT, '_'},
    { 28, R_SHIFT, '_'},
    { 28, CAPS_LOCK, '-'},

    { 29, 0, '='},
    { 29, L_SHIFT, '+'},
    { 29, R_SHIFT, '+'},
    { 29, CAPS_LOCK, '='},

    { 30, 0, 0x08},

    /* 31-33, Insert, Home, and Page Up. */
    /* Undefined */

    /* 34, Num Lock */
    /* Undefined, and does not need to be. */

    { 35, 0, '/'},
    { 35, NUM_LOCK, '/'},
    { 36, 0, '*'},
    { 36, NUM_LOCK, '*'},
    { 37, 0, '-'},
    { 37, NUM_LOCK, '-'},

    { 38, 0, '\t'},

    { 39, 0, 'q'},
    { 39, L_SHIFT, 'Q'},
    { 39, R_SHIFT, 'Q'},
    { 39, CAPS_LOCK, 'Q'},

    { 40, 0, 'w'},
    { 40, L_SHIFT, 'W'},
    { 40, R_SHIFT, 'W'},
    { 40, CAPS_LOCK, 'W'},

    { 41, 0, 'e'},
    { 41, L_SHIFT, 'E'},
    { 41, R_SHIFT, 'E'},
    { 41, CAPS_LOCK, 'E'},

    { 42, 0, 'r'},
    { 42, L_SHIFT, 'R'},
    { 42, R_SHIFT, 'R'},
    { 42, CAPS_LOCK, 'R'},

    { 43, 0, 't'},
    { 43, L_SHIFT, 'T'},
    { 43, R_SHIFT, 'T'},
    { 43, CAPS_LOCK, 'T'},

    { 44, 0, 'y'},
    { 44, L_SHIFT, 'Y'},
    { 44, R_SHIFT, 'Y'},
    { 44, CAPS_LOCK, 'Y'},

    { 45, 0, 'u'},
    { 45, L_SHIFT, 'U'},
    { 45, R_SHIFT, 'U'},
    { 45, CAPS_LOCK, 'U'},

    { 46, 0, 'i'},
    { 46, L_SHIFT, 'I'},
    { 46, R_SHIFT, 'I'},
    { 46, CAPS_LOCK, 'I'},

    { 47, 0, 'o'},
    { 47, L_SHIFT, 'O'},
    { 47, R_SHIFT, 'O'},
    { 47, CAPS_LOCK, 'O'},

    { 48, 0, 'p'},
    { 48, L_SHIFT, 'P'},
    { 48, R_SHIFT, 'P'},
    { 48, CAPS_LOCK, 'P'},

    { 49, 0, '['},
    { 49, L_SHIFT, '{'},
    { 49, R_SHIFT, '{'},
    { 49, CAPS_LOCK, '{'},

    { 50, 0, ']'},
    { 50, L_SHIFT, '}'},
    { 50, R_SHIFT, '}'},
    { 50, CAPS_LOCK, '}'},

    { 51, 0, '\\'},
    { 51, L_SHIFT, '|'},
    { 51, R_SHIFT, '|'},
    { 51, CAPS_LOCK, '|'},

    { 52, 0, 0x7F},

    /* 53-54, End and Page Down. */
    /* Undefined */

    { 55, NUM_LOCK, '7'},
    { 56, NUM_LOCK, '8'},
    { 57, NUM_LOCK, '9'},

    { 58, 0, '+'},
    { 58, NUM_LOCK, '+'},

    /* 59, Caps Lock */
    /* Undefined (and does not need to be, really). */

    { 60, 0, 'a'},
    { 60, L_SHIFT, 'A'},
    { 60, R_SHIFT, 'A'},
    { 60, CAPS_LOCK, 'A'},

    { 61, 0, 's'},
    { 61, L_SHIFT, 'S'},
    { 61, R_SHIFT, 'S'},
    { 61, CAPS_LOCK, 'S'},

    { 62, 0, 'd'},
    { 62, L_SHIFT, 'D'},
    { 62, R_SHIFT, 'D'},
    { 62, CAPS_LOCK, 'D'},

    { 63, 0, 'f'},
    { 63, L_SHIFT, 'F'},
    { 63, R_SHIFT, 'F'},
    { 63, CAPS_LOCK, 'F'},

    { 64, 0, 'g'},
    { 64, L_SHIFT, 'G'},
    { 64, R_SHIFT, 'G'},
    { 64, CAPS_LOCK, 'G'},

    { 65, 0, 'h'},
    { 65, L_SHIFT, 'H'},
    { 65, R_SHIFT, 'H'},
    { 65, CAPS_LOCK, 'H'},

    { 66, 0, 'j'},
    { 66, L_SHIFT, 'J'},
    { 66, R_SHIFT, 'J'},
    { 66, CAPS_LOCK, 'J'},

    { 67, 0, 'k'},
    { 67, L_SHIFT, 'K'},
    { 67, R_SHIFT, 'K'},
    { 67, CAPS_LOCK, 'K'},

    { 68, 0, 'l'},
    { 68, L_SHIFT, 'L'},
    { 68, R_SHIFT, 'L'},
    { 68, CAPS_LOCK, 'L'},

    { 69, 0, ';'},
    { 69, L_SHIFT, ':'},
    { 69, R_SHIFT, ':'},
    { 69, CAPS_LOCK, ':'},

    { 70, 0, '\''},
    { 70, L_SHIFT, '\"'},
    { 70, R_SHIFT, '\"'},
    { 70, CAPS_LOCK, '\"'},

    { 72, NUM_LOCK, '4'},
    { 73, NUM_LOCK, '5'},
    { 74, NUM_LOCK, '6'},

    /* 75, Left Shift */
    /* Undefined, and does not need to be. */

    { 76, 0, 'z'},
    { 76, L_SHIFT, 'Z'},
    { 76, R_SHIFT, 'Z'},
    { 76, CAPS_LOCK, 'Z'},

    { 77, 0, 'x'},
    { 77, L_SHIFT, 'X'},
    { 77, R_SHIFT, 'X'},
    { 77, CAPS_LOCK, 'X'},

    { 78, 0, 'c'},
    { 78, L_SHIFT, 'C'},
    { 78, R_SHIFT, 'C'},
    { 78, CAPS_LOCK, 'C'},

    { 79, 0, 'v'},
    { 79, L_SHIFT, 'V'},
    { 79, R_SHIFT, 'V'},
    { 79, CAPS_LOCK, 'V'},

    { 80, 0, 'b'},
    { 80, L_SHIFT, 'B'},
    { 80, R_SHIFT, 'B'},
    { 80, CAPS_LOCK, 'B'},

    { 81, 0, 'n'},
    { 81, L_SHIFT, 'N'},
    { 81, R_SHIFT, 'N'},
    { 81, CAPS_LOCK, 'N'},

    { 82, 0, 'm'},
    { 82, L_SHIFT, 'M'},
    { 82, R_SHIFT, 'M'},
    { 82, CAPS_LOCK, 'M'},

    { 83, 0, ','},
    { 83, L_SHIFT, '<'},
    { 83, R_SHIFT, '<'},
    { 83, CAPS_LOCK, '<'},

    { 84, 0, '.'},
    { 84, L_SHIFT, '>'},
    { 84, R_SHIFT, '>'},
    { 84, CAPS_LOCK, '>'},

    { 85, 0, '/'},
    { 85, L_SHIFT, '?'},
    { 85, R_SHIFT, '?'},
    { 85, CAPS_LOCK, '?'},

    /* 86, Right Shift */
    /* Undefined, and does not need to be. */

    /* 87, Cursor up */
    /* Undefined */

    { 88, NUM_LOCK, '1'},
    { 89, NUM_LOCK, '2'},
    { 90, NUM_LOCK, '3'},

    { 91, 0, '\r'},
    { 91, NUM_LOCK, '\r'},

    /* 92 and 93, Left Control and Alt. */
    /* Undefined and not required. */

    { 94, 0, ' '},
    { 94, L_SHIFT, ' '},
    { 94, R_SHIFT, ' '},
    { 94, CAPS_LOCK, ' '},
    { 94, NUM_LOCK, ' '},
    { 94, L_SHIFT | NUM_LOCK, ' '},
    { 94, R_SHIFT | NUM_LOCK, ' '},

    /* 95 and 96, Right Alt and Control. */
    /* Undefined and does not need to be. */

    /* 97-99, Cursor left, down, and right. */
    /* Undefined */

    { 100, NUM_LOCK, '0'},

    { 101, 0, 0x7F},
    { 101, NUM_LOCK, '.'}
  };



/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *  Note:  Anything below this line should not be changed in the normal
 *         use of these functions.  If a bug is found in this code, please
 *         report it to David Kessner at Vigra (619) 597-0737.
 *
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */



/*   Function prototypes for the VOGL for VGL functions.  Since these are
 *   only required in this file, they will not clutter up a header file.
 *   At some time in the future, they should be defined as static.
 */
int	vgl_vogl_init (void);
int	vgl_vogl_exit (void);
int	vgl_vogl_move (int x, int y);
int	vgl_vogl_clear (void);
int	vgl_vogl_color (int index);
int	vgl_vogl_draw (int x, int y);
int	vgl_vogl_fill (int n, int *x, int *y);
int	vgl_vogl_mapcolor (int index, int red, int green, int blue);
int	vgl_vogl_backbuf (void);
int	vgl_vogl_frontbuf (void);
int	vgl_vogl_swapbuf (void);
int	vgl_vogl_sync (void);
int	vgl_vogl_checkkey (void);
int	vgl_vogl_getkey (void);
int	vgl_vogl_locator (int *x, int *y);
int	vgl_vogl_char (char c);
int	vgl_vogl_string (char *s);
int	vgl_vogl_font (char *fontfile);
int	vgl_vogl_setls (int ls);
int	vgl_vogl_setlw (int lw);

int	vgl_vogl_translate_key (unsigned long event);


/*   The DevEntry structure...  Useful for drawing, chearing, and other
 *   playful things.  It's also useful for informing VOGL what functions
 *   to draw with...
 */
static DevEntry VGL_VOGL_DEV_ENTRY_NAME =
  {
    VGL_VOGL_DEV_NAME,
    VGL_VOGL_LARGE_FONT,
    VGL_VOGL_SMALL_FONT,
    vgl_vogl_backbuf,
    vgl_vogl_char,
    vgl_vogl_checkkey,
    vgl_vogl_clear,
    vgl_vogl_color,
    vgl_vogl_draw,
    vgl_vogl_exit,
    vgl_vogl_fill,
    vgl_vogl_font,
    vgl_vogl_frontbuf,
    vgl_vogl_getkey,
    vgl_vogl_init,
    vgl_vogl_locator,
    vgl_vogl_mapcolor,
    vgl_vogl_setls,
    vgl_vogl_setlw,
    vgl_vogl_string,
    vgl_vogl_swapbuf,
    vgl_vogl_sync
  };


/*   The VGBOARD structure used to identify which board we are using...
 */
static VGBOARD	*b;


/*   The two PIXMAPs on the VGBOARD for double buffering.
 */
static PIXMAP	*p[2];


/*   A copy of p[0] or p[1] for quick reference during drawing.
 *   cur_pixmap = p[cur_pixmap_num]
 *   These variables are here for speed, not to save space.
 */
static PIXMAP   *cur_pixmap;
static int	cur_pixmap_num;


/*   The number of the currently displayed p[?].
 */
static int	cur_pixmap_disp;


/*  VGL uses a coodinate system where the origin is at the upper left corner
 *  of the screen, with values increasing to the right and down.
 *  VOGL, however, appears to place the origin at the lower left corner, 
 *  with values increasing to the right and up.
 *
 *  The following four macros transform VOGL coordinates to VGL coordinates,
 *  then back again.
 *
 *  These macros fully account for the screen resolution and offer literally
 *  no speed penalty when used (other than the actual math, which would be
 *  done anyway).  While four functions rather than one would seem redundant,
 *  it offers flexability that would otherwise be missed.
 */

/* Transform VOGL coordinates to VGL coordinates. */
#define transform_x(xxx)	(xxx)
#define	transform_y(yyy)	(vgl_y_res(cur_pixmap)-(yyy)-1)

/* Transform VGL coordinates back into VOGL. */
#define transform_x_back(xxx)	(xxx)
#define transform_y_back(yyy)	(vgl_y_res(cur_pixmap)-(yyy)-1)


/*****************************************************************************/
/*   This function copies the VOGL for VGL device definition into the 
 *   vdevice.dev structure.  The name for this function is defined near
 *   the top of this file.   Note:  There is no function prototype and
 *   this function is not used from within VGL or VOGL for VGL.
 */
int
VGL_VOGL_DEVCPY_NAME (void)
{
  vdevice.dev = VGL_VOGL_DEV_ENTRY_NAME;
  return (0);
}


/*****************************************************************************/
/*   Initalize the VOGL functions, set up video, and initialize the VOGL
 *   device structure.
 */
int
vgl_vogl_init (void)
{
  b=vgl_initboard (BOARD_NAME, MODE_NAME);
  if (b)
    {
      p[0] = vgl_pixmap_map (b, 0);
      if (p[0] == NULL)
	{
	  vgl_killboard (b);
	  return (VGL_VOGL_ERROR);
	}
      
      p[1] = vgl_pixmap_map (b, 1);
      if (p[1] == NULL)
	{
	  vgl_freepixmap (p[0]);
	  vgl_killboard (b);
	  return (VGL_VOGL_ERROR);
	}
      
      cur_pixmap_num = 0;
      cur_pixmap_disp = 0;
      cur_pixmap = p[cur_pixmap_num];
      
      vgl_setrgb (b, 0, sizeof(vogl_colors)/sizeof(vogl_colors[0]),
		  vogl_colors);
      
      vgl_setcur (p[0], 0, 0);
      vgl_setcur (p[1], 0, 0);
      
      
      /*   The following code is copied from Sharon T. Casaletto's code.     
       *   Certain parts were modified, however, to demove device dependancies.
       */
      
      /* Initalize the current graphics position. */
      /* Take this out when linking with VOGL.    */
      /*   Note from DK:  Shouldn't this be #ifdef'd out?
       *   Also, should this set the current position to an invalid location?
       *   If the origin is location 0,0 then this will set the current
       *   position to just one pixel outside the screen.  Clipping will
       *   adjust for this  but I consider it wrong.
       */
      vdevice.cpVx = vgl_x_res (cur_pixmap);    /* =640 at a res of 640x480 */
      vdevice.cpVy = vgl_y_res (cur_pixmap);    /* =480 at a res of 640x480 */
      
      /* Initalize the display dimensions. */
      vdevice.sizeSx = vgl_x_res (cur_pixmap);  /* =640 at a res of 640x480 */
      vdevice.sizeSy = vgl_y_res (cur_pixmap);  /* =480 at a res of 640x480 */
      
      /* Initalise the size of the square on the screen. */
      /*   Note from DK:  This sets sizeX/Y to be the largest square posible 
       *   at the given resolution.  It works for any resolution and aspect.
       *   At 640x480, it sets it to 480x480.
       */
      vdevice.sizeX = min (vgl_x_res (cur_pixmap), vgl_y_res (cur_pixmap));
      vdevice.sizeY = vdevice.sizeX;
      
      /* Initalize the number of bitplanes on the screen. */
      /* Don't know if this will work, took from VGA */
      /*   Note from DK:  The VOGL functions for VGL support up to 8 bitplanes.
       *   However, the vogl_colors structure might have to be expanded for it
       *   to look correct.  For that matter, the current vogl_colors
       *   structure is half the size it should be for 4 bitplanes.
       */
      vdevice.depth = 4;
      
      /*   This is the end of Sharon T. Casaletto's code, as relavent to this
       *   part of the program.
       */

      /*   Set the current dimensions of the current font */
      vdevice.hwidth = vgl_font_width(cur_pixmap);
      vdevice.hheight = vgl_font_heightcur_pixmap);
      
      return (VGL_VOGL_GOOD);
    }
  else
    return (VGL_VOGL_ERROR);
}


/*****************************************************************************/
/*   Shut down everything, free whatever needs freeing, etc.                 */
int	
vgl_vogl_exit (void)
{
  vgl_setcur (cur_pixmap, 0, 0);
  vgl_clear (cur_pixmap);
  vgl_freepixmap (p[0]);
  vgl_freepixmap (p[1]);
  vgl_killboard (b);

  return (VGL_VOGL_GOOD);
}

/*****************************************************************************/
/*   This is not a VOGL function, per-se, but included anyway...
 *   It moves the current drawing position to a new point.
 */
int	
vgl_vogl_move (int x, int y)
{
  vdevice.cpVx = x;
  vdevice.cpVy = y;
  
  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*    Clear the screen to the current background color.
 *
 *    Note:  There is no equivalent to vgl_vogl_color() for the background
 *    color.  Thus, this function will always change things to color index
 *    zero.  If the default color set is used (for VGL or VOGL) this is 
 *    black.
 */
int	
vgl_vogl_clear (void)
{
  vgl_clear (cur_pixmap);
  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*    Set the current drawing FOREGROUND color.                              */
int    
vgl_vogl_color (int index)
{
  vgl_setcur (p[0], index, 0);
  vgl_setcur (p[1], index, 0);

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*    Draw a line from the current position to the specified position
 *    Move the cursor to the new position.
 */
int	
vgl_vogl_draw (int x, int y)
{
  vgl_line (cur_pixmap,  
	    transform_x (vdevice.cpVx), transform_y (vdevice.cpVy),
	    transform_x (x), transform_y (y) );

  vdevice.cpVx = x;
  vdevice.cpVy = y;

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Draw a filled polygon of up to 128 points.                              */
int	
vgl_vogl_fill (int n, int *x, int *y)
{
  int	i;
  struct vgl_coord v[128];

  if (n > 128)
    return (VGL_VOGL_ERROR);

  for (i=0; i<n; i++)
    {
      v[i].x = transform_x (x[i]);
      v[i].y = transform_y (y[i]);
    }

  vgl_fillpoly (cur_pixmap, n, v);

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Set the RGB value of CLUT entry (color look-up table)                   */
int
vgl_vogl_mapcolor (int index, int red, int green, int blue)
{
  struct vgl_color c;

  c.red = red;
  c.green = green;
  c.blue = blue;

  vgl_setrgb (b, index, 1, &c);

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/* Set the drawing buffer to the one that is NOT being displayed             */
int
vgl_vogl_backbuf (void)
{
  cur_pixmap_num = (cur_pixmap_disp==1) ? 0:1;
  cur_pixmap = p[cur_pixmap_num];

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/* Set the drawing buffer to the one that IS being displayed                 */
int	
vgl_vogl_frontbuf (void)
{
  cur_pixmap_num = cur_pixmap_disp;
  cur_pixmap = p[cur_pixmap_num];

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Swap buffers for double buffering-- both displayed and drawn buffers.   */
int	
vgl_vogl_swapbuf (void)
{
  /* Set the currently displayed pixmap to be the currently DRAWN pixmap */
  vgl_pixmap_disp (cur_pixmap);
  cur_pixmap_disp = cur_pixmap_num;
  
  /* Now, swap the drawing pixmaps */
  cur_pixmap_num = (cur_pixmap_disp==1) ? 0:1;
  cur_pixmap = p[cur_pixmap_num];

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Sync the display.
 *
 *   Note:  VGL always keeps the display sync'd so this function always
 *   returns immidiately.
 */
int	
vgl_vogl_sync (void)
{
  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Get a key from the keyboard.  If no key is available, return zero.      */
int	
vgl_vogl_checkkey (void)
{
  unsigned long	event;
  int key;


  /*   Keep looping while there is something in the buffer AND the keycode
   *   does not translate into anything...
   */
  while (vgl_kbhit (b) )
    {
      event = vgl_getkey (b, 0);
      vgl_kb_setled (b, event);
      key = vgl_vogl_translate_key (event);

      if (key!=0)
	return (key);
    }

  return (0);
}


/*****************************************************************************/
/*    Get a key from the keyboard-- with blocking (it waits for a key)       */
int	
vgl_vogl_getkey (void)
{
  unsigned long event;
  int		key_code;


  /*  Loop until there is a key_code that's not zero... */
  do
    {
      event = vgl_getkey (b, 1);  /* This does proper blocking under any OS */
      vgl_kb_setled (b, event);
      key_code = vgl_vogl_translate_key (event);
    } while (key_code == 0);  /* Some events "never happened". (key-up's) */

  return (key_code);
}


/*****************************************************************************/
/*   Get the current position of the mouse.  Button positions are returned
 *   by the function, while x and y values are returned via pointers.
 *   See the definitions for VGL_VOGL_MOUSE_LEFT/MID/RIGHT near the top of
 *   this file for more info.
 */
int	
vgl_vogl_locator (int *wx, int *wy)
{
  int	x, y, buttons, vogl_buttons;

  /* Get the mouse coords, without blocking */
  vgl_mouse_getcoords (b, &x, &y, &buttons, 0);

  /* Translate them back to VOGL coords */
  *wx = transform_x_back (x);
  *wy = transform_y_back (y);

  /* Translate button flags into VOGL mouse flags */
  vogl_buttons = (((buttons&MOUSE_LEFT )?VGL_VOGL_MOUSE_LEFT :0) |
		  ((buttons&MOUSE_MID  )?VGL_VOGL_MOUSE_MID  :0) |
		  ((buttons&MOUSE_RIGHT)?VGL_VOGL_MOUSE_RIGHT:0)   );

  return (vogl_buttons);
}


/*****************************************************************************/
/*   Display a character at the current position, and update the current
 *   position.  The current position refers to the lower left of the 
 *   character cell.
 */
int	
vgl_vogl_char (char c)
{
  int	x, y;

  x = transform_x (vdevice.cpVx);
  y = transform_y (vdevice.cpVy);

  /* Address the upper left of the text, rather than the lower left. */
  x -= vdevice.hheight - 1;

  vgl_text2 (cur_pixmap, x, y, 1, &c);

  vdevice.cpVx += vdevice.hwidth;

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Display a string of text at the current cursor position, and update
 *   the current position.  Again, the current position refers to the lower
 *   left of the character.
 */
int	
vgl_vogl_string (char *s)
{
  int	x, y, len;

  x = transform_x (vdevice.cpVx);
  y = transform_y (vdevice.cpVy);

  /* Address the upper left of the text, rather than the lower left. */
  x -= vdevice.hheight - 1;
  
  len = strlen (s);

  vgl_text2 (cur_pixmap, x, y, len, s);

  vdevice.cpVx += len * vdevice.hwidth;

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Set the current font to be used.  The valid names for "fontfiles" are
 *   those defined in the beginning of this file-- namely, VGL_VOGL_LARGE_FONT
 *   and VGL_VOGL_SMALL_FONT.
 */
int	vgl_vogl_font (char *fontfile)
{
  if (strcmp (fontfile, VGL_VOGL_LARGE_FONT) == 0)
    {
      vgl_setfont (p[0], vgl_large_ffont);
      vgl_setfont (p[1], vgl_large_ffont);
    }
  else if (strcmp (fontfile, VGL_VOGL_SMALL_FONT) == 0)
    {
      vgl_setfont (p[0], vgl_small_ffont);
      vgl_setfont (p[1], vgl_small_ffont);
    }
  else
    return (VGL_VOGL_ERROR);


  vdevice.hwidth = vgl_font_width(cur_pixmap);
  vdevice.hheight = vgl_font_heightcur_pixmap);

  return (VGL_VOGL_GOOD);
}


/*****************************************************************************/
/*   Set the current line style to be drawn.
 *
 *   This function is not implimented at this time.
 */
int
vgl_vogl_setls (int ls)
{
  return (VGL_VOGL_ERROR);
}


/*****************************************************************************/
/*   Set the current line width to be drawn.
 *
 *   This function is not implimented at this time.
 */
int	
vgl_vogl_setlw (int lw)
{
  return (VGL_VOGL_ERROR);
}


/*****************************************************************************/
/*   Translate a VGL keyboard event into something VOGL can use (namely, 
 *   ASCII).  It uses the table at the beginning of this file.
 */
int	vgl_vogl_translate_key (unsigned long event)
{
  int i;

  /* Filter out unwanted flags */
  event &= (0xFF     |
	    L_SHIFT  | R_SHIFT   | 
	    L_ALT    | R_ALT     | 
	    L_CTRL   | R_CTRL    | 
	    NUM_LOCK | CAPS_LOCK | /* SCRL_LOCK | */
	    KEY_UP);

  for (i=0; i<(sizeof(keyboard_table)/sizeof(keyboard_table[0])); i++)
    {
      if( event == (keyboard_table[i].key_num | keyboard_table[i].flags))
	return (keyboard_table[i].return_code);
    }
  
  return (0);
}


/*****************************************************************************/
/*****************************************************************************/

#endif /* not SUPPORT_VOGL */

