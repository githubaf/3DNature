
#include "vgl.h"
#include "vgl_internals.h"

#ifdef VGL_DITHER
#include <time.h>
#include <math.h>
#endif

#define	MAX_EDGE	128

struct edge
  {
    int x1, y1, x2, y2;
    int v1, v2;
    long dx, dy, c1;
    int x, maxy;
    struct edge *next;
  };

/* The edge table, and the start of the active edge table (aet) */
static struct edge edge_table[MAX_EDGE], *aet;
/*static struct edge *et[MAX_EDGE];*/


//STATIC_FCN void sort_edge (struct edge *e, int n);
//STATIC_FCN void sort_aet (void);

//int vgl_dumb_fillpoly_root (PIXMAP * p, int nvert, struct vgl_coord *vert);  // AF, not used 26.July 2021

#ifdef VGL_DITHER
void vgl_dumb_fillpoly_convex (PIXMAP * p, int n_vert, struct vgl_coord *vert, double DitherCol);
#else
void vgl_dumb_fillpoly_convex (PIXMAP * p, int n_vert, struct vgl_coord *vert);
#endif
STATIC_FCN int vgl_dumb_fillpoly_convex_root (PIXMAP * p, int n_vert, struct vgl_coord *vert); // used locally only -> static, AF 26.7.2021
STATIC_FCN void vgl_dumb_poly (PIXMAP * p, int n_vert, struct vgl_coord *vert); // used locally only -> static, AF 30.7.2021
STATIC_FCN void vgl_dumb_poly_noclip (PIXMAP * p, int n_vert, struct vgl_coord *vert); // used locally only -> static, AF 30.7.2021

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN void 
vgl_dumb_poly (PIXMAP * p, int n_vert, struct vgl_coord *vert) // used locally only -> static, AF 30.7.2021
{
  int i;

  for (i = 0; i < n_vert - 1; i++)
    vgl_line (p, vert[i].x, vert[i].y, vert[i + 1].x, vert[i + 1].y);

  vgl_dumb_line (p, vert[i].x, vert[i].y, vert[0].x, vert[0].y);
}
#endif

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN void 
vgl_dumb_poly_noclip (PIXMAP * p, int n_vert, struct vgl_coord *vert) // used locally only -> static, AF 30.7.2021
{
  int i;

  for (i = 0; i < n_vert - 1; i++)
    vgl_line (p, vert[i].x, vert[i].y, vert[i + 1].x, vert[i + 1].y);

  vgl_line (p, vert[i].x, vert[i].y, vert[0].x, vert[0].y);
}
#endif

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void 
vgl_dumb_fillpoly (PIXMAP * p, int n_vert, struct vgl_coord *vert)
{
  int n_spans;

  n_spans = vgl_dumb_fillpoly_root (p, n_vert, vert);
/*
  vgl_dumb_hlinelist (p, n_spans, p->workspace);
*/
#ifdef AMIGA
  vgl_dumb_hlinelist (p, n_spans, p->workspace);
#endif
}
#endif

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void 
vgl_dumb_fillpoly_noclip (PIXMAP * p, int n_vert, struct vgl_coord *vert)
{
  int n_spans;

  n_spans = vgl_dumb_fillpoly_root (p, n_vert, vert);
/*
  vgl_dumb_hlinelist_noclip (p, n_spans, p->workspace);
*/
  vgl_hlinelist (p, n_spans, p->workspace);
}
#endif

/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
int
vgl_dumb_fillpoly_root (PIXMAP * p, int nvert, struct vgl_coord *vert)
{
  int i, j, k, l, nedge, max_spans, n_spans, w;
  struct edge *edge, *e2;
  struct span_list *list;

  if (nvert >= MAX_EDGE)
    {
      p->error = VGL_ERR_RANGE;
      nvert = MAX_EDGE;
    }

  max_spans = p->worksize / sizeof (list[0]);

  if (max_spans < p->pixdata->y_res)
    {
      max_spans = p->pixdata->y_res;
      if (p->workspace != NULL)
	vgl_free (p->workspace);

      p->worksize = sizeof (list[0]) * max_spans;
      p->workspace = vgl_malloc (p->worksize);
      if (p->workspace == NULL)
	{
	  p->worksize = 0;
	  p->error = VGL_ERR_MEMORY;
	  return 0;
	}
    }

  n_spans = 0;
  list = p->workspace;


  nvert = (nvert < MAX_EDGE) ? nvert : MAX_EDGE;
  aet = NULL;

/*      Initalize edge table, sort end points according to increasing Y */
  for (edge = edge_table, nedge = 0, i = 0; i < nvert; i++)
    {
      j = (i + 1) % nvert;
      k = vert[i].y - vert[j].y;
      if (k < 0)
	{
	  edge->v1 = i;
	  edge->v2 = j;
	  edge->x1 = vert[i].x;
	  edge->y1 = vert[i].y;
	  edge->x2 = vert[j].x;
	  edge->y2 = vert[j].y;

	  /* Figure out if the edge needs to be shortened by one */
	  k = (j + 1) % nvert;
	  if (vert[k].y >= edge->y2)
	    edge->maxy = edge->y2 - 1;
	  else
	    edge->maxy = edge->y2;
	}
      else if (k > 0)
	{
	  edge->v1 = j;
	  edge->v2 = i;
	  edge->x1 = vert[j].x;
	  edge->y1 = vert[j].y;
	  edge->x2 = vert[i].x;
	  edge->y2 = vert[i].y;

	  /* Figure out if the edge needs to be shortened by one */
	  k = i - 1;
	  if (k < 0)
	    k += nvert;
	  if (vert[k].y >= edge->y2)
	    edge->maxy = edge->y2 - 1;
	  else
	    edge->maxy = edge->y2;
	}
      else
	{
	  /*      We do not want to think about horizontal lines */
	  continue;
	}

      edge->dx = edge->x2 - edge->x1;
      edge->dy = edge->y2 - edge->y1;
      edge->c1 = edge->dy * edge->x1;
      edge->x = edge->x1;
      edge->next = NULL;

      edge++;
      nedge++;
    }

/*      Sort the edge table in order of increasing Y.  If two Y's are the
   same, then it is sorted by increasing X.

   qsort( edge_table, nedge, sizeof( edge_table[0]), cmp_edge);
 */
  sort_edge (edge_table, nedge);

/*      Unroll the "initalization sequence" of the first loop.
   Basically, set up the inital values of the AET.
 */
  j = 0;
  edge = aet = edge_table + j;
  j++;
  i = edge->y1;
  e2 = edge_table + 1;
  while (e2->y1 == i && j < nedge)
    {
      edge->next = e2;
      edge = e2;
      e2++;
      j++;
    }
  edge->next = NULL;


/*      j = next entry in the edge table
   i = current Y
 */

/*      While... There is something to draw  */
  while (aet != NULL)
    {
      /* Sort aet according to x */
      sort_aet ();

      /*      Draw horizontal lines */
      edge = aet;
      while (edge != NULL)
	{
	  k = edge->x;
	  edge = edge->next;
	  if (edge)
	    {
	      l = edge->x;
	      edge = edge->next;

	      if (n_spans >= max_spans)
		{
		  max_spans += p->pixdata->y_res;
		  p->worksize = sizeof (list[0]) * max_spans;
		  p->workspace = vgl_malloc (p->worksize);
		  if (p->workspace == NULL)
		    {
		      p->worksize = 0;
		      p->error = VGL_ERR_MEMORY;
		      return 0;
		    }

		  for (w = 0; w < n_spans; w++)
		    ((struct span_list *) p->workspace)[w] = list[w];

		  vgl_free (list);
		  list = p->workspace;
		}

	      list[n_spans].x1 = k;
	      list[n_spans].x2 = l;
	      list[n_spans].y = i;
	      n_spans++;
	      /*poly_hline( k, l, i); */
	    }
	}

      /* Prune old edges */
      edge = aet;
      while (edge != NULL)
	{
	  /* Should we remove this line? */
	  if (edge->maxy <= i)
	    {			/* Yes! */
	      if (edge == aet)
		{
		  aet = edge->next;
		  edge = aet;
		}
	      else
		{
		  /* e2 is not a valid pointer until one iteration
		     through the loop.  But this is OK since the first
		     iteration will not fall into this condition.
		   */
		  e2->next = edge->next;
		  edge = edge->next;
		}
	    }
	  else
	    {
	      e2 = edge;
	      edge = edge->next;
	    }
	}

      /* Update/Increment 'i' and the X coord's */
      i++;
      for (edge = aet; edge != NULL; edge = edge->next)
	{
	  edge->c1 += edge->dx;
	  edge->x = (int) (edge->c1 / edge->dy);
	}

      /* Add new lines, if needed */
      if (i == edge_table[j].y1 && j < nedge)
	{
	  /* Unroll the first loop
	     Do an insertion sort
	     Seek inside of aet to insert new line
	   */
	  if (aet == NULL)	/* Granted, this _could_ be folded into the following if, but... */
	    {
	      aet = edge_table[j].next = NULL;
	      aet = edge_table + j;
	      j++;
	    }
	  else if (edge_table[j].x1 > aet->x)
	    {
	      /* Insert new edge into aet */
	      edge_table[j].next = aet;
	      aet = edge_table + j;
	      j++;
	    }
	  edge = aet;

	  while (i == edge_table[j].y1 && j < nedge)
	    {
	      /* Do an insertion sort
	         Seek inside of aet to insert new line
	       */
	      while (edge->next != NULL)
		{
		  if (edge_table[j].x1 > edge->next->x)
		    break;
		  edge = edge->next;
		}

	      /* Insert new edge into aet */
	      edge_table[j].next = edge->next;
	      edge->next = edge_table + j;

	      j++;
	    }
	}
    }

  return (n_spans);
}
#endif
/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
STATIC_FCN int
cmp_edge (const void *a, const void *b)
{
  int i;

  i = ((struct edge *) a)->y1 - ((struct edge *) b)->y1;
  if (i == 0)
    {
      i = ((struct edge *) a)->x1 - ((struct edge *) b)->x1;
      if (i == 0)
	{
	  i = (int) ((((struct edge *) a)->c1 + ((struct edge *) a)->dx * 100) / ((struct edge *) a)->dy -
		     (((struct edge *) b)->c1 + ((struct edge *) b)->dx * 100) / ((struct edge *) b)->dy);
	}
    }

  return (i);
}
#endif

#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
STATIC_FCN void
sort_edge (struct edge *e, int n)
{
  int i, swap;
  struct edge temp;

  n--;

  do
    {
      swap = 0;
      for (i = 0; i < n; i++)
	{
	  if (cmp_edge (e + i, e + (i + 1)) > 0)
	    {
	      temp = e[i];
	      e[i] = e[i + 1];
	      e[i + 1] = temp;
	      swap = 1;
	    }
	}
    }
  while (swap);
}
#endif



/*****************************************************************************/
/*****************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
STATIC_FCN void 
sort_aet (void)
{
  int k;
  struct edge *t, *t2, *t3;

  if (aet->next == NULL)
    return;
  else if (aet->next->next == NULL)
    {
      if (aet->x <= aet->next->x)
	return;
      else
	{
	  t = aet->next;
	  t->next = aet;
	  aet->next = NULL;
	  aet = t;
	}
    }
  else
    {

      do			/* Do a bubble sort on aet */
	{
	  k = 0;

	  /* Swap the first two elements, if needed */
	  if (aet->x > aet->next->x)
	    {
	      t = aet->next;
	      aet->next = t->next;
	      t->next = aet;
	      aet = t;
	      k = 1;
	    }

	  t3 = aet;
	  t2 = t3->next;
	  t = t2->next;
	  while (t != NULL)
	    {
	      if (t->x < t2->x)
		{
		  k = 1;
		  t2->next = t->next;
		  t->next = t2;
		  t3->next = t;

		  t3 = t;
		  t = t2->next;
		}
	      else
		{
		  t3 = t2;
		  t2 = t;
		  t = t->next;
		}
	    }

	}
      while (k == 1);
    }
}
#endif
/*****************************************************************************/
/*****************************************************************************/

/* This next function was hacked by Chris "Xenon" Hanson
** (xenon@arcticus.burner.com) on 06 Mar 1994 as an adaptation
** of the vgl_dumbfillpoly_root() optimized for convex polygons.
*/


/*****************************************************************************/
/*****************************************************************************/
#ifdef VGL_DITHER
void 
vgl_dumb_fillpoly_convex (PIXMAP * p, int n_vert, struct vgl_coord *vert, double DitherCol)

{

  int init_dtable;
#else
void 
vgl_dumb_fillpoly_convex (PIXMAP * p, int n_vert, struct vgl_coord *vert)

{
#endif
  int n_spans, spare_foreground;

  n_spans = vgl_dumb_fillpoly_convex_root (p, n_vert, vert);


#ifdef VGL_DITHER
  spare_foreground = p->foreground;

  if(DitherCol != 0)
    {
      if(p->DTableSize == 0)
        {
        if((p->DitherTable = vgl_malloc(VGL_DITHERTABLE_SIZE)))
          {
          srand48(time(NULL)); /* set top 32 bits of seed to current time */
          for(init_dtable = 0; init_dtable < VGL_DITHERTABLE_SIZE; init_dtable++)
            {
              p->DitherTable[init_dtable] = (int)(255 * drand48());
            } /* for */
          p->DTableSize = VGL_DITHERTABLE_SIZE;
          } /* if */
        else
          {
          p->foreground = (int)DitherCol; /* No dithering today. */
          } /* if */
        } /* if */
      if(p->DTableSize) /* Might have changed since we last checked, because above */
        {
        vgl_dumb_hlinelist_noclip_root (p, n_spans, p->workspace, DitherCol);
        } /* else */
    } /* if */
  else
#endif
  vgl_dumb_hlinelist (p, n_spans, p->workspace);
#ifdef VGL_DITHER
  p->foreground = spare_foreground;
#endif
}


/*****************************************************************************/
/*****************************************************************************/

STATIC_FCN int
vgl_dumb_fillpoly_convex_root (PIXMAP * p, int n_vert, struct vgl_coord *vert) // used locally only -> static, AF 26.7.2021
{
  int max_spans, n_spans, xa, xb, dx, adx, y, ya, yb, dy, ady,
   min_y, max_y, scratch, pos_slope, c, r, f, inc1, inc2, g, loop;
   /* Yeah, freak the registerizer... */
  struct span_list *list;

  max_spans = p->worksize / sizeof (list[0]);

  if (max_spans < p->pixdata->y_res)
    {
      max_spans = p->pixdata->y_res;
      if (p->workspace != NULL)
   vgl_free (p->workspace);

      p->worksize = sizeof (list[0]) * max_spans;
      p->workspace = vgl_malloc (p->worksize);
      if (p->workspace == NULL)
   {
     p->worksize = 0;
     p->error = VGL_ERR_MEMORY;
     return 0;
   }
    }

  list = p->workspace;

  /* Determine vertical extent of poly */

  min_y = p->pixdata->y_res;
  max_y = 0;

  for(loop = 0; loop < n_vert; loop++)
    {
    if(vert[loop].y < min_y)
      min_y = vert[loop].y;
    if(vert[loop].y > max_y)
      max_y = vert[loop].y;
    } /* for */

  if(min_y < 0)
    min_y = 0;
  
  if(max_y > p->pixdata->y_res)
    max_y = p->pixdata->y_res;

  /* We already know, though some will be clipped */
  n_spans = (max_y - min_y) + 1;


  /* Init "safe" clipped coords into span list */

  for(loop = 0; loop < n_spans; loop++)
    {
    list[loop].y = loop + min_y;

    /* set x defaults to clipped values. Note: possible algorithm failure,
    ** if pixmap->data->x_res == MAX_INT. [shrug] I'm not worried. */

    list[loop].x1 = p->pixdata->x_res + 1; /* x1 is leftmost endpoint of span */
    list[loop].x2 = -1; /* x2 is rightmost endpoint. */
    /* (If x2 is still -1 after edge completion, the span will be tossed.) */
    } /* for */


  /* Iterate each edge */

  for(loop = 0; loop < n_vert; loop++)
    {
      xa = vert[loop].x;
      ya = vert[loop].y;
      if(loop == n_vert - 1)
        {
          xb = vert[0].x;
          yb = vert[0].y;
        } /* if */
      else
        {
          xb = vert[loop + 1].x;
          yb = vert[loop + 1].y;
        } /* else */

      if (xa == xb)
        {
          if(yb < ya)
            {
              /* Yes, David, I know I could do this wih an XOR. But that's
              ** probably patented. ;) */
              scratch = ya;
              ya = yb;
              yb = scratch;
            } /* for */
          for(y = ya; y <= yb; y++)
            {
              if((y >= 0) && (y < p->pixdata->y_res))
                {
                  list[y - min_y].y = y;
                  if(xa < list[y - min_y].x1)
                    list[y - min_y].x1 = xa; /* x1 is left endpoint of span */
                  if(xa > list[y - min_y].x2)
                    list[y - min_y].x2 = xa; /* x2 is right endpoint */
                } /* if */
            } /* for */
        } /* if */
      else if (ya == yb)
        {
          if((ya >= 0) && (ya < p->pixdata->y_res))
            {
              if(xa > xb)
                {
                  scratch = xb;
                  xb = xa;
                  xa = scratch;
                } /* if */
              
              list[ya - min_y].y = ya;
              if(xa < list[ya - min_y].x1)
                list[ya - min_y].x1 = xa;
              if(xb > list[ya - min_y].x2)
                list[ya - min_y].x2 = xb;
            } /* if */
        } /* else if */
    
      else
        {
        /* Code here was horked and hacked from BRESENHAM line in dumb.c */
        /* ...and still needs much work. [09 Mar 1994 -- should be done!] */
        /* [17 Mar 1994, Hah! You only THOUGHT so!] */

        dx = xb - xa;
        adx = abs (dx);
        dy = yb - ya;
        ady = abs (dy);
      
        pos_slope = (dx > 0);
        if (dy < 0)
          pos_slope = !pos_slope;
      
        if (adx > ady)
          {          /* Slope is more or less horizontal */
            if (dx > 0)
              {
                c = xa;
                r = ya;
                f = xb;
              } /* if */
            else
              {
                c = xb;
                r = yb;
                f = xa;
              } /* else */

            inc1 = 2 * ady;
            g = 2 * ady - adx;
            inc2 = 2 * (ady - adx);
      
            if (pos_slope)
              {
                while (c <= f) /* Checked, 13 Mar 1994 */
                  {
                    if((r >= 0) && (r < p->pixdata->y_res))
                      {
                        if(c < list[r - min_y].x1)
	                       list[r - min_y].x1 = c;
	                     if(c > list[r - min_y].x2)
	                       list[r - min_y].x2 = c;
                      } /* if */

                    c++; /* Ho, we make clever language pun, yes? */
      
                    if (g >= 0)
                      {
                        r++;
                        g += inc2;
                      }
                    else
                      g += inc1;
                  } /* while */
              } /* if pos slope */
            else
              {
                while (c <= f) /* Checked 13 Mar 1994 */
                  {
                    if((r >= 0) && (r < p->pixdata->y_res))
                      {
                        if(c < list[r - min_y].x1)
                          list[r - min_y].x1 = c;
                        if(c > list[r - min_y].x2)
                          list[r - min_y].x2 = c;
                      } /* if */

                    c++;

                    if (g > 0)
                      {
                      r--;
                      g += inc2;
                      } /* if */
                    else
                      g += inc1;
                  } /* while */
              } /* else neg slope */
          } /* slope if ~ horiz */
        else
          {          /* Slope is more or less vertical */
            if (dy > 0)
              {
                c = ya;
                r = xa;
                f = yb;
              } /* if */
            else
              {
                c = yb;
                r = xb;
                f = ya;
              } /* else */
      
            inc1 = 2 * adx;
            g = 2 * adx - ady;
            inc2 = 2 * (adx - ady);

            if (pos_slope)
              {
                while (c <= f) /* Checked 13 Mar 1994 */
                  {
                    if((c >= 0) && (c < p->pixdata->y_res))
                      {
               	      if(r < list[c - min_y].x1)
               	      {
	               	     list[c - min_y].x1 = r;
               	      }
	                  if(r > list[c - min_y].x2)
	                  {
	               	     list[c - min_y].x2 = r;
	                  }
               	    } /* if */
      
                    c++;
      
                    if (g >= 0)
                    {
                        r++;
                        g += inc2;
                    } /* if */
                    else
                    {
                        g += inc1;
                    }
                  } /* while */
              } /* if pos_slope */
            else
              {
                while (c <= f)
                  {
                    if((c >= 0) && (c < p->pixdata->y_res))
               	    {
               	      if(r < list[c - min_y].x1)
               	      {
	               	     list[c - min_y].x1 = r;
               	      }
	                  if(r > list[c - min_y].x2)
	                  {
	               	     list[c - min_y].x2 = r;
	                  }
               	    } /* if */

                    c++;

                    if (g > 0)
                    {
                        r--;
                        g += inc2;
                    } /* if */
                    else
                    {
                        g += inc1;
                    }
                  } /* while */
              } /* else neg slope */
          } /* else slope is vert */
        } /* else */
    } /* for */


  return (n_spans);
}
/*****************************************************************************/
/*****************************************************************************/

