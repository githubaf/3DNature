
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define	SOURCE_FILE	"Videomodes"

typedef unsigned long ulong;

struct mode_def
  {
    char name[32];		/* Name  */
    int x_res;			/* X Axis Resolution (pixels)  */
    int y_res;			/* Y Axis Resolution (pixels)  */
    float h_refresh;		/* Horizontal Refresh Rate (kHz)  */
    float v_refresh;		/* Vertical Refresh Rate (Hz)  */
    float clock;		/* Pixel Clock (Mhz)  */
    char sync_style[32];	/* VGA/Mixed/OnVideo/Serated  */
    int hp;			/* Horizontal Period (pixels)  */
    int hb;			/* Horizontal Blanking (pixels)  */
    int hs;			/* Horizontal Sync Width (pixels)  */
    int hbp;			/* Horizontal Back Porch (pixels)  */
    int ha;			/* Horizontal Active (pixels)  */
    int hfp;			/* Horizontal Front Porch (pixels)  */
    int vp;			/* Vertical Period (lines)  */
    int vb;			/* Vertical Blanking (lines)  */
    int vs;			/* Vertical Sync Width (line  */
    int vbp;			/* Vertical Back Porch  */
    int va;			/* Vertical Active  */
    int vfp;			/* Vertical Front Porch  */
  };

int parse_mode (char *s, struct mode_def *mode);


struct mmi_150
  {
    unsigned long hs, bp, hd, sd, br, vs, vb, vd, lt, ls, mi, td, cr, bl;
    unsigned long x_size, page_size;
    int sync_pol;
  };

#define	MMI_150_CLOCK	(5)
int gen_mmi_150 (struct mmi_150 *mmi, struct mode_def *mode);




struct gdm_9000
  {
    /* System Control Regs */
    ulong	sysconfig;

    /* Video Control Regs */
    ulong	hrzt;
    ulong	hrzsr;
    ulong	hrzbr;
    ulong	hrzbf;
    ulong	prehrzc;
    ulong	vrtt;
    ulong	vrtsr;
    ulong	vrtbr;
    ulong	vrtbf;
    ulong       prevrtc;
    ulong	srtctl;

    /* VRAM Control Regs */
    ulong	mem_config;
    ulong	rfperiod;
    ulong	rlmax;

    unsigned long	x_size, page_size;

    long	bt445_ctrl[16];
  };


int gen_gdm_9000 (struct gdm_9000 *gdm, struct mode_def *mode);


typedef struct
  {
    char name[32];
    int x_res, y_res;
    struct mmi_150 mmi_150;
    struct gdm_9000 gdm_9000;
/*
   struct mmi_250       mmi_250;
   struct vgs           vgs;
 */
  }
VIDEO_MODE;


/*****************************************************************************/
int 
main ()
{
  int i;
  char s[1024];
  FILE *file;
  struct mode_def mode;
  struct mmi_150 mmi_150;
  struct gdm_9000 gdm_9000;
  int mmi_150_ok, gdm_9000_ok;

  file = fopen (SOURCE_FILE, "r");
  if (file)
    {
      printf ("VIDEO_MODE	mode_list[] =\n\t{\n");

      fgets (s, 1024, file);
      while (!feof (file))
	{
	  if (isalpha (s[0]))
	    {
	      if (parse_mode (s, &mode) == 0)
		{
		  mmi_150_ok = gen_mmi_150 (&mmi_150, &mode);
		  gdm_9000_ok = gen_gdm_9000 (&gdm_9000, &mode);

		  printf ("\t{ \"%s\", %d, %d, \n", 
			  mode.name, mode.x_res, mode.y_res);
#if 0
		  printf ("\t\t/* MMI-150 */\n");
		  if (mmi_150_ok==0)
		    {
		      printf ("\t\t{ %lu, %lu, %lu, %lu, %lu, %lu, "
			      "%lu, %lu, %lu, %lu, %lu, %lu, 0x%06lX, "
			      "%lu, %lu, %lu, %d }\n",
			      mmi_150.hs, mmi_150.bp, mmi_150.hd, mmi_150.sd, 
			      mmi_150.br, mmi_150.vs, mmi_150.vb, mmi_150.vd, 
			      mmi_150.lt, mmi_150.ls, mmi_150.mi, mmi_150.td, 
			      mmi_150.cr, mmi_150.bl, mmi_150.x_size, 
			      mmi_150.page_size, mmi_150.sync_pol);
		    }
		  else
		    printf ("\t\t{ 0 },\n\n");
#endif
		  printf ("\t\t/* GDM-9000 */\n");
		  if (gdm_9000_ok==0)
		    {
		      printf ("\t\t{\n");

		      printf ("\t\t0x%08lX, %lu, %lu, %lu, %lu, \n"
			      "%lu, %lu, %lu, %lu, %lu, %lu, 0x%lX, \n"
			      "%lu, %lu, %lu, %lu, %lu,\n",
			      gdm_9000.sysconfig, gdm_9000.hrzt, 
			      gdm_9000.hrzsr, gdm_9000.hrzbr, 
			      gdm_9000.hrzbf, gdm_9000.prehrzc, 
			      gdm_9000.vrtt, gdm_9000.vrtsr, gdm_9000.vrtbr, 
			      gdm_9000.vrtbf, gdm_9000.prevrtc, 
			      gdm_9000.srtctl, gdm_9000.mem_config, 
			      gdm_9000.rfperiod, gdm_9000.rlmax, 
			      gdm_9000.x_size, gdm_9000.page_size);
		      printf ("\t\t\t{ 0x%X", gdm_9000.bt445_ctrl[0]);
		      for (i=1; i<16; i++)
			printf (", 0x%X", gdm_9000.bt445_ctrl[i]);
		      printf (" }\n");
		      printf ("\t\t}\n");
		    }
		  else
		    printf ("\t\t{ 0 },\n");

		  printf ("\n\t},\n\n");
		}
	    }

	  fgets (s, 1024, file);
	}

      printf ("\t};\n");
      fclose (file);
    }
  else
    {
      printf ("Error opening source file, %s\n", SOURCE_FILE);
    }
}


/*****************************************************************************/
int 
parse_mode (char *s, struct mode_def *mode)
{
  int n_tokens;
  char *t[100], *u;

  n_tokens = 0;
  for (u = strtok (s, " \t,;:\r\n"); u != NULL; u = strtok (NULL, " \t,;:\r\n"))
    t[n_tokens++] = u;

  if (n_tokens != 19)
    return (1);

  strcpy (mode->name, t[0]);	/* Name  */
  mode->x_res = atoi (t[1]);	/* X Axis Resolution (pixels)  */
  mode->y_res = atoi (t[2]);	/* Y Axis Resolution (pixels)  */
  mode->h_refresh = atof (t[3]);	/* Horizontal Refresh Rate (kHz)  */
  mode->v_refresh = atof (t[4]);	/* Vertical Refresh Rate (Hz)  */
  mode->clock = atof (t[5]);	/* Pixel Clock (Mhz)  */
  strcpy (mode->sync_style, t[6]);	/* VGA/Mixed/OnVideo/Serated  */
  mode->hp = atoi (t[7]);	/* Horizontal Period (pixels)  */
  mode->hb = atoi (t[8]);	/* Horizontal Blanking (pixels)  */
  mode->hs = atoi (t[9]);	/* Horizontal Sync Width (pixels)  */
  mode->hbp = atoi (t[10]);	/* Horizontal Back Porch (pixels)  */
  mode->ha = atoi (t[11]);	/* Horizontal Active (pixels)  */
  mode->hfp = atoi (t[12]);	/* Horizontal Front Porch (pixels)  */
  mode->vp = atoi (t[13]);	/* Vertical Period (lines)  */
  mode->vb = atoi (t[14]);	/* Vertical Blanking (lines)  */
  mode->vs = atoi (t[15]);	/* Vertical Sync Width (line  */
  mode->vbp = atoi (t[16]);	/* Vertical Back Porch  */
  mode->va = atoi (t[17]);	/* Vertical Active  */
  mode->vfp = atoi (t[18]);	/* Vertical Front Porch  */

  return (0);
}


/*****************************************************************************/
/* A Screen Unit (SU) is four pixels */
#define	SU	(4)

int 
gen_mmi_150 (struct mmi_150 *mmi, struct mode_def *mode)
{
  int fp;

  mmi->bl = 0x20 & (int) (((float) mode->clock / (float) MMI_150_CLOCK) + 0.5);
  mmi->lt = mode->hp / SU;	/* Line time = Horizontal Period in SU */
  mmi->hs = (mode->hs / 2) / SU;	/* Half Sync = H Sync Width / 2 in SU */
  mmi->bp = mode->hbp / SU;	/* Back Porch = H Back Porch in SU */
  mmi->hd = mode->ha / SU;

  if (mode->hfp >= 0)
    fp = mode->hfp;
  else
    fp = mmi->lt - ((mmi->hs * 2) + mmi->bp + (mode->ha / SU));

  mmi->sd = (mmi->lt / 2) - (mmi->hs * 2) + mmi->bp + fp;
  mmi->br = (mmi->lt / 2) - fp;

  mmi->vd = 2 * mode->va;
  mmi->vs = 2 * mode->vs;
  mmi->vb = 2 * (mode->vbp - mode->vs);
  mmi->ls = 0;
  mmi->td = 10 + ((mode->clock * 680) / (SU * 1000));
  mmi->mi = 256 - mmi->td;

  if (strcmp (mode->sync_style, "NEG") == 0)
    {
      mmi->cr = 0x070039;
      mmi->sync_pol = -1;
    }
  else if (strcmp (mode->sync_style, "POS") == 0)
    {
      mmi->cr = 0x070039;
      mmi->sync_pol = 1;
    }

  mmi->x_size = mode->x_res;
  mmi->page_size = mmi->x_size * mode->y_res;

  
  /* Is there enough RAM for this mode? */
  if ( mmi->page_size > 1044480)
      return(1);

  return(0);
}

/*****************************************************************************/
#define GDM_9000_CLOCK (20.0)

int 
gen_gdm_9000 (struct gdm_9000 *gdm, struct mode_def *mode)
{
  int i, j, k, a;
  unsigned long l;
  float f;

  static int bt_445_ctrl[16] =
    {
      -1, 0x40, 0x35, 0x07, 0, 32, 0x44, 0xE0,
      0x04, 0x40, 0x00, 0x07, -1, 0x08, 0x00, 0x01
    };

  struct hor_shift
    {
      int res;
      int value;
      int fields;
    };

  static struct hor_shift field[] =
    {
      { 2048, 0x07, 4},
      { 1024, 0x06, 6},
      {  512, 0x05, 7},
      {  256, 0x04, 7},
      {  128, 0x03, 7},
      {   64, 0x02, 3},
      {   32, 0x01, 1},
      {    0,    0, 7},
    };

  struct clk_gen
    {
      float mhz;
      int l, m, n;
    };

  static struct clk_gen clk_table[1920];
  static int clk_table_gen = 0;


  /* Initalize the clock table */
  if (clk_table_gen==0)
    {
      clk_table_gen = 1;

      i=0; 
      for (j=24; j<=63; j++)
	{
	  for (k=4; k<=15; k++)
	    {
	      f = (GDM_9000_CLOCK*j)/k;
	      if ( f>=75.0 && f<=167.0)
		{
		  clk_table[i].l = 0;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = f;
		  i++;
		  
		  clk_table[i].l = 1;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = f/2.0;
		  i++;
		  
		  clk_table[i].l = 2;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = f/4.0;
		  i++;
		  
		  clk_table[i].l = 3;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = f/8.0;
		  i++;
		}
	      else
		{
		  clk_table[i].l = 0;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = 0.0;
		  i++;
		  
		  clk_table[i].l = 1;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = 0.0;
		  i++;
		  
		  clk_table[i].l = 2;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = 0.0;
		  i++;
		  
		  clk_table[i].l = 3;
		  clk_table[i].m = j;
		  clk_table[i].n = k;
		  clk_table[i].mhz = 0.0;
		  i++;
		}
	    }
	}
    }


  /* Initalize the "easy" registers */
  gdm->rfperiod = 515;
  gdm->rlmax = 330;
  gdm->prehrzc = 0;
  gdm->prevrtc = 0;
  gdm->srtctl = 0x01E4;

  /* Figure the memory configuration */
  gdm->x_size = mode->x_res;
  gdm->page_size = mode->y_res * gdm->x_size;

  if ( gdm->page_size <= 1024*1024)
    gdm->mem_config = 3;
  else if (gdm->page_size <= 2*1024*1024)
    gdm->mem_config = 2;
  else
    return(1);  /* Not enough RAM to support this mode */


  /* Initalize the bt_445 ctrl regs */
  for (i=0; i<16; i++)
    gdm->bt445_ctrl[i] = bt_445_ctrl[i];


  /* Calculate the value of the sysconfig register */
  k=1<<2;         /* The current bit flag of the field */
  j=gdm->x_size;  /* The remaining value to "figure out" */
  l=0;            /* The current value of the sysconfig register */
  a = 20;         /* The current "shift position" for the current field */
  for (i=0; j>0 && field[i].res!=0 && k!=0; i++)
    {
      if (j>=field[i].res && (field[i].fields & k)!=0)
	{
	  j -= field[i].res;
	  l |= field[i].value << a;
	  k >>= 1;
	  a -= 3;
	}
    }

  /* Was the desired resolution met? */
  if (j!=0)
    return(1);

  gdm->sysconfig = l;


  /* Video Control Regs */
  gdm->hrzt = mode->hp/8 - 1;
  gdm->hrzsr = mode->hs/8 - 1;
  gdm->hrzbr = (mode->hs + mode->hbp)/8 - 1;
  gdm->hrzbf = (mode->hs + mode->hbp + mode->ha)/8 - 1;
  gdm->vrtt = mode->vp;
  gdm->vrtsr = mode->vs;
  gdm->vrtbr = mode->vs + mode->vbp;
  gdm->vrtbf = mode->vs + mode->vbp + mode->va;

  /* Clock Generation in the BT445 */
  for (i=0, k=0, f=99999999.0; i<1920; i++)
    {
      if (fabs(clk_table[i].mhz - mode->clock) < f)
	{
	  f = fabs(clk_table[i].mhz - mode->clock);
	  k = i;
	}
    }

  gdm->bt445_ctrl[5] = clk_table[k].m;
  gdm->bt445_ctrl[6] = (clk_table[k].l<<6) | (clk_table[k].n-1);
  if (     mode->clock <   90)  gdm->bt445_ctrl[7] |= 7;
  else if (mode->clock <  100)  gdm->bt445_ctrl[7] |= 7;
  else if (mode->clock <  110)  gdm->bt445_ctrl[7] |= 6;
  else if (mode->clock <  120)  gdm->bt445_ctrl[7] |= 6;
  else if (mode->clock <  130)  gdm->bt445_ctrl[7] |= 5;
  else if (mode->clock <  140)  gdm->bt445_ctrl[7] |= 4;
  else if (mode->clock <  150)  gdm->bt445_ctrl[7] |= 2;
  else if (mode->clock <  167)  gdm->bt445_ctrl[7] |= 1;
  else
    return(1);  /* Pixel clock is too high */

  return(0);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
