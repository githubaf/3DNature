/* Tree.c (ne gistree.c 14 Jan 1994 CXH)
** Tree-related functions
** Built/cut from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. the Huber.
*/

#include "WCS.h"
#include "Foliage.h"
#include "Useful.h"

STATIC_VAR double VertSunFact, HorSunFact, HorSunAngle;

STATIC_FCN void SetBitmapImageSpan(struct BitmapImage *BMI); // used locally only -> static, AF 19.7.2021
STATIC_FCN void BitmapImage_Del(struct BitmapImage *BMI); // used locally only -> static, AF 23.7.2021
STATIC_FCN void Image_Paste(struct BitmapImage *SBMI, UBYTE **Bitmap,
        double Dw, double Dh, double Dx, double Dy,
        double Distance, struct ColorComponents *CC,
        struct ColorComponents *AM, double ElStart,
        double ElIncr, struct Window *win, struct QCvalues *QC); // used locally only -> static, AF 23.7.2021
STATIC_FCN void ColorToGray(UBYTE **Bitmap, long MaxZip); // used locally only -> static, AF 23.7.2021
STATIC_FCN struct BitmapImage *BitmapImage_New(void); // used locally only -> static, AF 26.7.2021
STATIC_FCN void BitmapImage_Scale(struct BitmapImage *SBMI, struct BitmapImage *DBMI,
        double Dx, double Dy); // used locally only -> static, AF 26.7.2021
STATIC_FCN void BitmapImage_DelSingle(struct BitmapImage *BMI); // used locally only -> static, AF 26.7.2021

#define MODE_REPLACE 0
#define MODE_AVERAGE 1

struct TreeDimension {
 float 	X[3], Y[3], OX, OY;
 short 	StartX, EndX,
	WT[4];
};
/*
struct ILBMHeader {
 UBYTE ChunkID[4];
 LONG ChunkSize;
};
*/
/*
struct WcsBitMapHeader {
 USHORT Width, Height;
 SHORT XPos, YPos;
 UBYTE Planes, Masking, Compression, Pad;
 USHORT Transparent;
 UBYTE XAspect, YAspect;
 SHORT PageWidth, PageHeight;
};
*/

/* use of random variables

treerand	random, forest model item, treeheight w/model or without,
		 randomint for cmap, color matching for cmap
treerand2	density determinant for tree presence
treerand3	center y position of trees, image selection
treerand4	center x position of trees
random		seadepthfact, WaterDepth, ElDepth, relfactor, color perturbation
*/

void DetailTree(struct Window *win, short eco, struct QCvalues *QC,
	short ComputeHt, struct ColorComponents *CC,
	struct ColorComponents *AM, double *Elev)
{
 short PtrnWidth, PtrnHeight, PtrnCtr, Mode, Class,
	LastRow, LastCol, PtrnCtrCol, IntEdgeWt, PixWt;
 long zip, x, y, j, l, MinX, MaxX, colval, TreePixWidth, TreePixHeight,
	NewSubPixArraySize, NewTreePixArraySize, WtAvg;
 double treewidth, FMaxX, FMinX, polywidth, LeftWt, RightWt, BottomWt,
	EdgeWt, dist, QIncr, QStart, QCurrent, TrueHeight, ElStart, ElIncr,
	ElPt, fred, fgrn, fblu;
 struct TreeDimension Tree;
 UBYTE *PixPtr, *TreePtr;
 const UBYTE *PtrnPtr;

 if (ComputeHt)
  {
  TrueHeight = treerand * PARC_RNDRHT_ECO(eco);	/* ht in meters */
  treeheight = TrueHeight * treehtfact / 21.34;
  Class = (PAR_TYPE_ECO(eco) & 0x00ff);
  } /* if */
 else
  {
  TrueHeight = treeheight * 21.34 / treehtfact;
  Class = EcoClass;
  }

 if (Class >= 100)
  return;

/*printf("CL %d HT %f\n", Class, treeheight);*/

 FMaxX = max(polyx[b][0], polyx[b][1]);
 FMaxX = max(polyx[b][2], FMaxX);
 FMinX = min(polyx[b][0], polyx[b][1]);
 FMinX = min(polyx[b][2], FMinX);
 polywidth = FMaxX - FMinX;
 Tree.X[0] = treerand4 * polywidth + FMinX;

 treewidth = .5 * polywidth * (maxfract + 1);

/* set floating point Y values */

 if (Class >= 50)
  {
  if (EcoShift[eco].BMImage)
   {
   struct BitmapImage *BMI;
   double Heightx2;
   
   RotatePoint(&PP[3], NoBankMatx);
/*   VectorMagnitude(&PP[3]); this is done in RotatePoint() */
   VectorMagnitude(&PP[4]);
   VertSunFact = VectorAngle(&PP[4], &SP);
   HorSunFact = sqrt(1.0 - VertSunFact * VertSunFact);

   HorSunAngle = -SignedVectorAngle2D(&SNB, &PP[3], 2);

   BMI = EcoShift[eco].BMImage;
   while (BMI->Next)
    {
    if (BMI->DensityPct > treerand3)	/* this should have an independent random variable */
     break;
    BMI = BMI->Next;
    } /* while */
   
   treeheight *= BMI->HeightPct;
   treewidth = (treeheight * BMI->Width) / BMI->Height;
   Heightx2 = treeheight * 2.0;

   while (BMI->Height > Heightx2 && BMI->Smaller)
    {
    BMI = BMI->Smaller;
    } /* while */

   Tree.Y[2] = polyy[b][0] + treerand3 * (polyy[b][2] - polyy[b][0]);
   Tree.Y[0] = Tree.Y[2] - treeheight;
   ElStart = TrueHeight + Elev[0] + (Elev[2] - Elev[0]) * treerand3;
   if (treeheight > 0.0)
    ElIncr = TrueHeight / treeheight;
   else
    return;

   FloatCol = 7.0 + 4.99 * sunfactor;
   ColMax = COL_CONIF_MAX;

   if (BMI->Colors < 3)
    {
    CC->Red = PARC_RNDR_COLOR(BMI->PalCol, 0);
    CC->Red -= sunshade * CC->Red;
    CC->Grn = PARC_RNDR_COLOR(BMI->PalCol, 1);
    CC->Grn -= sunshade * CC->Grn;
    CC->Blu = PARC_RNDR_COLOR(BMI->PalCol, 2);
    CC->Blu -= sunshade * CC->Blu;
    } /* if gray scale image */

   Image_Paste(BMI, bitmap,
	(double)treewidth, (double)treeheight,
	Tree.X[0] - treewidth * .5, (double)Tree.Y[0],
	(double)qqq, CC, AM, (double)ElStart, (double)ElIncr, win, QC);
   } /* if */
  return;
  } /* if image class */

 if (Class >= 4 && Class != 6)
  {
  Tree.Y[2] = polyy[b][0] + treerand3 * (polyy[b][2] - polyy[b][0]);
  Tree.Y[0] = Tree.Y[2] - treeheight;
  ElStart = TrueHeight + Elev[0] + (Elev[2] - Elev[0]) * treerand3;
  if (treeheight != 0.0)
   ElIncr = TrueHeight / treeheight;
  else
   ElIncr = 0.0;
  } /* if a tree */
 else
  {
  Tree.Y[0] = polyy[b][0] - treeheight;
  Tree.Y[2] = polyy[b][2];
  ElStart = TrueHeight + Elev[0];
  ElIncr = (Elev[2] - ElStart) / (Tree.Y[2] - Tree.Y[0]);
  } /* else not tree */

/* set floating point X values. FMinX and FMaxX are the outermost excursions
   of the polygon */

 if (Class == 5)
  {
  FMaxX += (2.0 * treewidth);
  FMinX -= (2.0 * treewidth);
  } /* if deciduous */
 else if (Class == 4)
  {
  FMaxX += (1.5 * treewidth);
  FMinX -= (1.5 * treewidth);
  } /* else if conifer */
 else
  {
  FMaxX += (treewidth / 2.0);
  FMinX -= (treewidth / 2.0);
  } /* else */

 switch (Class)
  {
  case 4:				/* conifer */
   {
   PtrnPtr = &evergreen[0][0];
   PtrnWidth = 20;
   PtrnHeight = 59;			/* actual array height - 1 */
   PtrnCtr = 9;
   FloatCol = 7.0 + 4.99 * sunfactor;
   ColMax = COL_CONIF_MAX;
   break;
   }

  case 5:				/* deciduous */
   {
   PtrnPtr = &deciduous[0][0];
   PtrnWidth = 40;
   PtrnHeight = 59;			/* actual array height - 1 */
   PtrnCtr = 19;
   FloatCol = 6.0 + 5.99 * sunfactor;
   ColMax = COL_DECID_MAX;
   break;
   }

  case 6:				/* low vegetation */
   {
   PtrnPtr = &grass[0][0];
   PtrnWidth = 40;
   PtrnHeight = 39;			/* actual array height - 1 */
   PtrnCtr = 19;
   FloatCol = 6.0 + 5.99 * sunfactor;
   ColMax = COL_DECID_MAX;
   break;
   }

  case 7:
   {
   PtrnPtr = &snag[0][0];
   PtrnWidth = 12;
   PtrnHeight = 59;			/* actual array height - 1 */
   PtrnCtr = 6;
   FloatCol = 13.0 + 2.99 * sunfactor;
   ColMax = COL_ROCK_MAX;
   break;
   }

  case 9:
   {
   PtrnPtr = &stump[0][0];
   PtrnWidth = 4;
   PtrnHeight = 9;			/* actual array height - 1 */
   PtrnCtr = 1;
   FloatCol = 13.0 + 2.99 * sunfactor;
   ColMax = COL_ROCK_MAX;
   break;
   }

  default:				/* water, snow, rock, ground */
   {
   PtrnPtr = &rockbrush[0][0];
   PtrnWidth = 40;
   PtrnHeight = 59;			/* actual array height - 1 */
   PtrnCtr = 19;
/* use same pen as for polygon rendering */
   break;
   }

  } /* switch */

/* subtract one pixel if the point is negative so truncation goes to lower value 
    (eg. -3.5 -> -4) to maintain proper width */
 xx[0] = Tree.X[0] < 0.0 ? Tree.X[0] - 1.0: Tree.X[0];
 yy[0] = Tree.Y[0] < 0.0 ? Tree.Y[0] - 1.0: Tree.Y[0];
 yy[2] = Tree.Y[2] < 0.0 ? Tree.Y[2] - 1.0: Tree.Y[2];

 MinX = FMinX;
 MaxX = FMaxX;			/* if right edge is neg.
						 it won't be plotted anyway */
 if(MaxX < 0) return;
 if(MinX < 0) MinX--;

/* if (MinX < 0) MinX = 0;
 if (MaxX > wide - 1) MaxX = wide - 1; */
 

/* determine Z buffer increment before adjustment made for edge of image */

 if (FMaxX != FMinX)
  QIncr = 2.0 * HalfPtrnOffset / (FMaxX - FMinX);
 else
  QIncr = 0.0;

/* determine leftmost pattern column and float offsets for weighting pixels */

 Tree.OX = Tree.X[0] - xx[0];
 Tree.OY = Tree.Y[0] - yy[0];
 Tree.StartX = PtrnCtr - (xx[0] - MinX);
 if (Tree.StartX < 0)
  {
  Tree.StartX = 0;
  MinX = xx[0] - PtrnCtr;
  }
 Tree.EndX = PtrnCtr + (MaxX - xx[0]);
 if (Tree.EndX > PtrnWidth - 1)
  {
  Tree.EndX = PtrnWidth - 1;
  MaxX = xx[0] + PtrnWidth / 2;
  }
 PtrnCtrCol = PtrnCtr - Tree.StartX;

/* Determine weighting for leftmost, rightmost and bottom pixel rows.
   Bounds checking is necessary since at image sides strange conditions apply */

 LeftWt = Tree.X[0] - FMinX;
 RightWt = FMaxX - Tree.X[0];
 LeftWt = LeftWt - (int)LeftWt;
 RightWt = RightWt - (int)RightWt;
 BottomWt = Tree.Y[2] - Tree.Y[0];
 BottomWt = BottomWt - (int)BottomWt;
 if (LeftWt > 1.0) LeftWt = 1.0;
 else if (LeftWt < 0.0) LeftWt = 0.0;
 if (RightWt > 1.0) RightWt = 1.0;
 else if (RightWt < 0.0) RightWt = 0.0;
 
 if (Tree.OY < .25)
  {
  if (Tree.OX < .25)
   {  
   Tree.WT[0] = 16;
   Tree.WT[1] = 0;
   Tree.WT[2] = 0;
   Tree.WT[3] = 0;
   }
  else if (Tree.OX < .5)
   {  
   Tree.WT[0] = 12;
   Tree.WT[1] = 4;
   Tree.WT[2] = 0;
   Tree.WT[3] = 0;
   }
  else if (Tree.OX < .75)
   {  
   Tree.WT[0] = 8;
   Tree.WT[1] = 8;
   Tree.WT[2] = 0;
   Tree.WT[3] = 0;
   }
  else 
   {  
   Tree.WT[0] = 4;
   Tree.WT[1] = 12;
   Tree.WT[2] = 0;
   Tree.WT[3] = 0;
   }
  }
 else if (Tree.OY < .5)
  {
  if (Tree.OX < .25)
   {  
   Tree.WT[0] = 12;
   Tree.WT[1] = 0;
   Tree.WT[2] = 4;
   Tree.WT[3] = 0;
   }
  else if (Tree.OX < .5)
   {  
   Tree.WT[0] = 9;
   Tree.WT[1] = 3;
   Tree.WT[2] = 3;
   Tree.WT[3] = 1;
   }
  else if (Tree.OX < .75)
   {  
   Tree.WT[0] = 6;
   Tree.WT[1] = 6;
   Tree.WT[2] = 2;
   Tree.WT[3] = 2;
   }
  else 
   {  
   Tree.WT[0] = 3;
   Tree.WT[1] = 9;
   Tree.WT[2] = 1;
   Tree.WT[3] = 3;
   }
  }
 else if (Tree.OY < .75)
  {
  if (Tree.OX < .25)
   {  
   Tree.WT[0] = 8;
   Tree.WT[1] = 0;
   Tree.WT[2] = 8;
   Tree.WT[3] = 0;
   }
  else if (Tree.OX < .5)
   {  
   Tree.WT[0] = 6;
   Tree.WT[1] = 2;
   Tree.WT[2] = 6;
   Tree.WT[3] = 2;
   }
  else if (Tree.OX < .75)
   {  
   Tree.WT[0] = 4;
   Tree.WT[1] = 4;
   Tree.WT[2] = 4;
   Tree.WT[3] = 4;
   }
  else 
   {  
   Tree.WT[0] = 2;
   Tree.WT[1] = 6;
   Tree.WT[2] = 2;
   Tree.WT[3] = 6;
   }
  }
 else
  {
  if (Tree.OX < .25)
   {  
   Tree.WT[0] = 4;
   Tree.WT[1] = 0;
   Tree.WT[2] = 12;
   Tree.WT[3] = 0;
   }
  else if (Tree.OX < .5)
   {  
   Tree.WT[0] = 3;
   Tree.WT[1] = 1;
   Tree.WT[2] = 9;
   Tree.WT[3] = 3;
   }
  else if (Tree.OX < .75)
   {  
   Tree.WT[0] = 2;
   Tree.WT[1] = 2;
   Tree.WT[2] = 6;
   Tree.WT[3] = 6;
   }
  else 
   {  
   Tree.WT[0] = 1;
   Tree.WT[1] = 3;
   Tree.WT[2] = 3;
   Tree.WT[3] = 9;
   }
  } /* if/else */

 TreePixWidth = (2 + Tree.EndX - Tree.StartX);
 if (TreePixWidth > PtrnWidth + 1 /*21*/) TreePixWidth = PtrnWidth + 1;/*21;*/
 TreePixHeight = (2 + yy[2] - yy[0]);
 NewSubPixArraySize = TreePixWidth * TreePixHeight;
 NewTreePixArraySize = NewSubPixArraySize;

 if (NewTreePixArraySize > TreePixArraySize)
  {
  free_Memory(TreePix, TreePixArraySize);
  TreePixArraySize = 0;
  if ((TreePix = get_Memory(NewTreePixArraySize, MEMF_CLEAR)) == NULL)
   {
   return;
   }
  else
   {
   TreePixArraySize = NewTreePixArraySize;
   }
  } /* if need larger array */
 else
  {
  memset(TreePix, 0, NewTreePixArraySize);
  } /* else clear as much of array as needed */
 if (NewSubPixArraySize > SubPixArraySize)
  {
  free_Memory(SubPix, SubPixArraySize);
  SubPixArraySize = 0;
  if ((SubPix = get_Memory(NewSubPixArraySize, MEMF_CLEAR)) == NULL)
   {
   return;
   }
  else
   {
   SubPixArraySize = NewSubPixArraySize;
   }
  } /* if need larger array */
 else
  {
  memset(SubPix, 0, NewSubPixArraySize);
  } /* else clear as much of array as needed */

 PtrnPtr += Tree.StartX;
 PixPtr = SubPix;
 TreePtr = TreePix;
 LastRow = TreePixHeight - 2;
 LastCol = TreePixWidth - 2;
 for (y=0; y<=LastRow; y++)
  {
  for (x=0; x<=LastCol; x++)
   {
   if ((unsigned int)PtrnPtr[x] > 0)
    {
    j = x;
    if (x != PtrnCtrCol)
     {
     if (x == 0)
      EdgeWt = LeftWt;
     else if (x == LastCol)
      EdgeWt = RightWt;
     else
      EdgeWt = 1.0;
     if (y == LastRow)
      EdgeWt *= BottomWt;
     } /* if not center column */
    else
     EdgeWt = 1.0;
    IntEdgeWt = EdgeWt * Tree.WT[0];
    PixPtr[j] += IntEdgeWt;
    TreePtr[j] += ((unsigned int)PtrnPtr[x] * IntEdgeWt);
    j += 1;
    IntEdgeWt = EdgeWt * Tree.WT[1];
    PixPtr[j] += IntEdgeWt;
    TreePtr[j] += ((unsigned int)PtrnPtr[x] * IntEdgeWt);
    j += TreePixWidth;
    IntEdgeWt = EdgeWt * Tree.WT[3];
    PixPtr[j] += IntEdgeWt;
    TreePtr[j] += ((unsigned int)PtrnPtr[x] * IntEdgeWt);
    j -= 1;
    IntEdgeWt = EdgeWt * Tree.WT[2];
    PixPtr[j] += IntEdgeWt;
    TreePtr[j] += ((unsigned int)PtrnPtr[x] * IntEdgeWt);

    } /* if */
   } /* for x=0... */
  PixPtr += TreePixWidth;
  TreePtr += TreePixWidth;
  if (y < PtrnHeight) PtrnPtr += PtrnWidth;
  } /* for y=0... */

 QStart = qqq + (xx[0] - MinX) * QIncr;

 PixPtr = SubPix;
 TreePtr = TreePix;
 for (y=yy[0], ElPt=ElStart; y<=yy[2]; y++, ElPt+=ElIncr)
  {
  if (y < 0)
   {
   PixPtr += TreePixWidth;
   TreePtr += TreePixWidth;
   continue;
   } /* if go to next row */
  if (y > high) break;
  zip = scrnrowzip[y] + MinX;
  
  QCurrent = QStart;

  for (x=MinX, l=0; x<=MaxX+1; x++, l++, zip++)
   {
   if (x < 0)
    continue;
   if (x > wide)
    {
    PixPtr += TreePixWidth;
    TreePtr += TreePixWidth;
    break;
    }
   dist = *(zbuf + zip);
   if (PixPtr[l])
    {
    if (QCurrent <= dist || bytemap[zip] < 100)
     {
     if (bytemap[zip] && (QCurrent >= dist ||
	 (((unsigned int)PixPtr[l] < 16) && (dist - QCurrent < PtrnOffset))))
      Mode = MODE_AVERAGE;
     else
      Mode = MODE_REPLACE;

     if (render & 0x01)
      {
      WtAvg = 15 * (unsigned int)PixPtr[l];
      fred = (CC->Red * (unsigned int)TreePtr[l]) / WtAvg;
      fgrn = (CC->Grn * (unsigned int)TreePtr[l]) / WtAvg;
      fblu = (CC->Blu * (unsigned int)TreePtr[l]) / WtAvg;
      fred += AM->Red;	/* ambient */
      fgrn += AM->Grn;
      fblu += AM->Blu;
      fred += (PARC_RNDR_COLOR(2, 0) - fred) * fade;
      fgrn += (PARC_RNDR_COLOR(2, 1) - fgrn) * fade;
      fblu += (PARC_RNDR_COLOR(2, 2) - fblu) * fade;
      fred += (PARC_RNDR_COLOR(2, 0) - fred) * fog;
      fgrn += (PARC_RNDR_COLOR(2, 1) - fgrn) * fog;
      fblu += (PARC_RNDR_COLOR(2, 2) - fblu) * fog;
      fred *= redsun;
      fgrn *= greensun;
      fblu *= bluesun;
      if (fred > 255.) fred = 255.;
      else if (fred < 0.) fred = 0.;
      if (fgrn > 255.) fgrn = 255.;
      else  if (fgrn < 0.) fgrn = 0.;
      if (fblu > 255.) fblu = 255.;
      else if (fblu < 0.) fblu = 0.;
      if (Mode)
       {
       PixWt = (unsigned int)PixPtr[l] * 6.25;	/* based on 100 / 16 */
       WtAvg = (unsigned int)bytemap[zip] + PixWt;
       colval = (*(bitmap[0] + zip) * (unsigned int)bytemap[zip] + fred * PixWt)
	 / WtAvg;
       *(bitmap[0] + zip) = (UBYTE)colval;

       colval = (*(bitmap[1] + zip) * (unsigned int)bytemap[zip] + fgrn * PixWt)
	 / WtAvg;
       *(bitmap[1] + zip) = (UBYTE)colval;

       colval = (*(bitmap[2] + zip) * (unsigned int)bytemap[zip] + fblu * PixWt)
	 / WtAvg;
       *(bitmap[2] + zip) = (UBYTE)colval;
       } /* if already values here and need to average */
      else
       {
       PixWt = 100;
       *(bitmap[0] + zip) = (UBYTE)fred;
       *(bitmap[1] + zip) = (UBYTE)fgrn;
       *(bitmap[2] + zip) = (UBYTE)fblu;
       } /* else this is the first value for this pixel or no averaging */
      } /* if render to bitmap */
     else
      PixWt = 100;

     if (QCurrent < dist)
      {
      if (render & 0x10)
       {
       if (render & 0x01)
        {
        ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
	} /* if bitmaps */
       else
        {
        NoRGBScreenPixelPlot(win, FloatCol, ColMax, x + drawoffsetX, y + drawoffsetY);
	} /* else no bitmaps */
       } /* if render to screen */
      if (render & 0x100)
       {
       *(QCmap[0] + zip) = QC->compval1;
       *(QCmap[1] + zip) = QC->compval2;
       *(QCmap[2] + zip) = QC->compval3;
       *(QCcoords[0] + zip) = facelat;
       *(QCcoords[1] + zip) = facelong;
       } /* if render to QC buffers */
      *(zbuf + zip) = QCurrent;
      if (ElevationMap)
       ElevationMap[zip] = ElPt;
      } /* if lower QCurrent value */
     bytemap[zip] += PixWt;
     } /* if QCurrent is closer or subpixel count not yet full */
    } /* if PixPtr */
   if (x < xx[0])
    QCurrent -= QIncr;
   else
    QCurrent += QIncr;
   } /* for x=MinX... */
  PixPtr += TreePixWidth;
  TreePtr += TreePixWidth;
  } /* for y=0... */

} /* DetailTree() */

/************************************************************************/
#ifdef GHJGFJHDGJAHSDG
// Converts gray scale raw image file into an ascii array for use as
//	 a texture brush
void ConvertTree(void)
{
 char filename[256], Path[256] = {0}, Filename[32] = {0};
 UBYTE *TreeArray;
 short i, j, k, width, height;
 FILE *ftree;

 if (! getfilename(0, "Input Image Path/Name", Path, Filename))
  return;

 strmfp(filename, Path, Filename);

 if ((ftree = fopen(filename, "rb")) == NULL)
  {
  User_Message("Tree Convert", "Error opening file for input!\nOperation terminated.", "OK", "c");
  return;
  }
 sprintf(str, "%d", "40");
 if (! GetInputString("Enter tree width in pixels.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
 {
     fclose (ftree);  // do not leave this resource open
     return;
 }
 width = atoi(str);
 if (! GetInputString("Enter tree height in pixels.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
 {
     fclose (ftree);  // do not leave this resource open
     return;
 }
 height = atoi(str);

 TreeArray = (UBYTE *)get_Memory(width * height, MEMF_ANY);
 if (TreeArray == NULL)
  {
     User_Message("Tree Convert", "Out of memory!\nOperation terminated.", "OK", "c");
     fclose (ftree);  // do not leave this resource open
     return;
  }
 
 if (fread(TreeArray, width * height, 1, ftree) != 1)
  {
     fclose (ftree);  // do not leave this resource open
     User_Message("Tree Convert", "Error reading file!\nOperation terminated.", "OK", "c");
     return;
  }

 fclose(ftree);

 if (! getfilename(0, "Output Image Path/Name", Path, Filename))
  return;

 strmfp(filename, Path, Filename);

 if ((ftree = fopen(filename, "w")) == NULL)
  {
  User_Message("Tree Convert", "Error opening file for output!\nOperation terminated.", "OK", "c");
  goto EndTree;
  }

 for (i=0, k=0; i<height*2; i++)
  {
  for (j=0; j<width/2; j++, k++)
   {
   fprintf(ftree, "%d, ", TreeArray[k] / 16);
   }
  fputc(10, ftree);
  }

 fclose (ftree);

EndTree:
 if (TreeArray)
  free_Memory(TreeArray, width * height);

} /* ConvertTree() */
#endif
/************************************************************************/

short LoadForestModels(void)
{
 short i, j, error = 0, Version, rendersegs;
 long sum;
 char filename[256], name[32], IDStr[32], Class[12];
 FILE *fModel;

 if (! settings.rendertrees)
  return (1);

 for (i=0; i<ECOPARAMS; i++)
  {
  if (EcoPar.en[i].Model[0] == 0)
   continue;
  if ((PAR_TYPE_ECO(i) & 0x00ff) >= 50)
   continue;

/* fix name if animated */
  strcpy(name, PAR_MODEL_ECO(i));
  rendersegs = settings.rendersegs;
  settings.rendersegs = 1;
  ModFileName(name, 0, 0, 0);
  settings.rendersegs = rendersegs;

  strmfp(filename, modelpath, name);
  if ((fModel = fopen(filename, "r")) == NULL)
   {
   error = 1;
   break;
   }
  fscanf(fModel, "%31s", IDStr);
  if (strcmp(IDStr, "WCSModel"))
   {
   error = 3;
   fclose(fModel);
   break;
   } /* if wrong type */

  fscanf(fModel, "%hd", &Version);
  if (Version != 1)
   {
   error = 4;
   fclose(fModel);
   break;
   } /* wrong version */

  fscanf(fModel, "%ld", &FM[i].Items);
  if (FM[i].Items < 1)
   {
   error = 6;
   break;
   } /* no data */

  if ((FM[i].TM = (struct TreeModel *)
	get_Memory(FM[i].Items * sizeof (struct TreeModel), MEMF_ANY)) == NULL)
   {
   error = 5;
   break;
   } /* out of memory */

  sum = 0;
  for (j=0; j<FM[i].Items; j++)
   {
   if (fscanf(fModel, "%ld%ld%11s%hd%hd%hd", &FM[i].TM[j].Ht, &FM[i].TM[j].Stems,
	Class, &FM[i].TM[j].Red, &FM[i].TM[j].Grn, &FM[i].TM[j].Blu) != 6)
    {
    error = 2;
    break;
    } /* if read error */

   if (! strcmp(Class, "Water"))
    FM[i].TM[j].Class = 0;
   else if (! strcmp(Class, "Snow"))
    FM[i].TM[j].Class = 1;
   else if (! strcmp(Class, "Rock"))
    FM[i].TM[j].Class = 2;
   else if (! strcmp(Class, "Strat"))	/* short-lived version 2.0 Param file */
    FM[i].TM[j].Class = 2;
   else if (! strcmp(Class, "Bare"))
    FM[i].TM[j].Class = 3;
   else if (! strcmp(Class, "Conif"))
    FM[i].TM[j].Class = 4;
   else if (! strcmp(Class, "Decid"))
    FM[i].TM[j].Class = 5;
   else if (! strcmp(Class, "LowVg"))
    FM[i].TM[j].Class = 6;
   else if (! strcmp(Class, "Snag"))
    FM[i].TM[j].Class = 7;
   else
    FM[i].TM[j].Class = 8;

   sum += FM[i].TM[j].Stems;

   } /* for j=0... */
  fclose(fModel);

  if (error) break;

  for (j=0; j<FM[i].Items; j++)
   {
   FM[i].TM[j].Pct = (float)FM[i].TM[j].Stems / (float)sum;
   } /* for j=0... */
  for (j=1; j<FM[i].Items; j++)
   {
   FM[i].TM[j].Pct += FM[i].TM[j - 1].Pct;
   } /* for j=0... */

  } /* for i=0... */

//EndLoad:

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"Error opening Ecosystem Model file for input!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)name);
   break;
   } /* open fail */
  case 2:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"Error writing to Ecosystem Model file!\nOperation terminated prematurely.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRITE_FAIL, (CONST_STRPTR)name);
   break;
   } /* write fail */
  case 3:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"Not a WCS Ecosystem Model file!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_TYPE, (CONST_STRPTR)name);
   break;
   } /* wrong type */
  case 4:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"Unsupported WCS Ecosystem Model file version!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_VER, (CONST_STRPTR)name);
   break;
   } /* wrong version */
  case 5:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"Out of memory allocating Ecosystem Models!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* out of memory */
  case 6:
   {
   User_Message((CONST_STRPTR)"Parameters Module: Model",
           (CONST_STRPTR)"No data in WCS Ecosystem Model!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_ILL_VAL, (CONST_STRPTR)name);
   break;
   } /* no data */
  } /* switch */

 if (error)
  {
  for (i=2; i<ECOPARAMS; i++)
   {
   if (FM[i].TM)
    free_Memory(FM[i].TM, FM[i].Items * sizeof (struct TreeModel));
   FM[i].TM = NULL;
   } /* for i=2... */
  } /* if error */

 return ((short)(! error));

} /* LoadForestModels() */

/**********************************************************************/

short BitmapImage_Load(void)
{
char filename[256];
short error, success = 1, Planes, MaxImgHt;
long i, j, fh;
double SumGrpDens, SumImgDens, SumDensity;
struct BitmapImage **BMI, *BMICur, *BMIPrev, *BMISmall;
struct FoliageGroup *FolGp;
struct Foliage *Fol;
struct ILBMHeader Hdr;
struct WcsBitMapHeader BMHdr;

 if (! settings.rendertrees)
  return (1);

 for (i=0, error=0; i<ECOPARAMS && success; i++, error=0)
  {

  if ((PAR_TYPE_ECO(i) & 0x00ff) >= 50 && (PAR_TYPE_ECO(i) & 0x00ff) < 100)
   {

   if (EcoShift[i].Ecotype && EcoShift[i].Ecotype->FolGp && EcoShift[i].Ecotype->FolGp->Fol)
    {
    BMI = &EcoShift[i].BMImage;
    SumDensity = 0.0;
/* sum group densities */

    FolGp = EcoShift[i].Ecotype->FolGp;
    SumGrpDens = 0.0;
    while (FolGp)
     {
     SumGrpDens += Rootstock_GetFloatValue(&FolGp->Root, WCS_ECOTYPE_DENSITY);
     FolGp = FolGp->Next;
     } /* while */
    if (SumGrpDens <= 0.0)
     SumGrpDens = 1.0;

/* for each group */

    FolGp = EcoShift[i].Ecotype->FolGp;
    while (FolGp)
     {
/* sum image densities */

     Fol = FolGp->Fol;
     SumImgDens = 0.0;
     while (Fol)
      {
      SumImgDens += Rootstock_GetFloatValue(&Fol->Root, WCS_ECOTYPE_DENSITY);
      Fol = Fol->Next;
      } /* while */
     if (SumImgDens <= 0.0)
      SumImgDens = 1.0;

     Fol = FolGp->Fol;
     while (Fol)
      {
      strcpy(filename, Rootstock_GetName(&Fol->Root));
      if ((fh = open(filename, O_RDONLY)) >= 0)
       {
       if (CheckIFF(fh, &Hdr))
        {
        if (FindIFFChunk(fh, &Hdr, "BMHD"))
         {
         if ((read(fh, (char *)&BMHdr, sizeof (struct WcsBitMapHeader))) ==
		 sizeof (struct WcsBitMapHeader))
          {
             ENDIAN_CHANGE_IF_NEEDED(
             SimpleEndianFlip16U(BMHdr.Width,&BMHdr.Width);
             SimpleEndianFlip16U(BMHdr.Height,&BMHdr.Height);
             SimpleEndianFlip16S(BMHdr.XPos,&BMHdr.XPos);
             SimpleEndianFlip16S(BMHdr.YPos,&BMHdr.YPos);
             // UBYTE Planes, Masking, Compression, Pad;
             SimpleEndianFlip16U(BMHdr.Transparent,&BMHdr.Transparent);
             // UBYTE XAspect, YAspect;
             SimpleEndianFlip16S(BMHdr.PageWidth,&BMHdr.PageWidth);
             SimpleEndianFlip16S(BMHdr.PageHeight,&BMHdr.PageHeight);
             )
          close (fh);
          fh = -1;
          if ((*BMI = BitmapImage_New()))
           {
           BMICur = *BMI;
           if (BMHdr.Planes == 8)
            BMICur->Colors = 1;
           else if (BMHdr.Planes == 24)
            BMICur->Colors = 3;
           else
            error = 1;
           if (! error)
            {
            BMICur->Width = BMHdr.Width;
            BMICur->Height = BMHdr.Height;
            for (j=0; j<BMICur->Colors; j++)
             {
             if ((BMICur->Bitmap[j] = 
		(UBYTE *)get_Memory(BMICur->Width * BMICur->Height, MEMF_ANY)) == NULL)
              {
              error = 1;
              break;
	      } /* if no memory */
	     } /* for j=0... */
            if (! error)
             {
             if ((BMICur->MidPt =
		(float *)get_Memory(BMICur->Height * sizeof (float), MEMF_ANY)))
              {
              if ((BMICur->Span =
		(float *)get_Memory(BMICur->Height * sizeof (float), MEMF_ANY)))
               {
               if (LoadImage(filename, BMICur->Colors > 1, BMICur->Bitmap,
		BMICur->Width, BMICur->Height, 0, &BMICur->Width, &BMICur->Height,
		&Planes))
                {
                if (BMICur->Colors == 3 && ! Rootstock_GetShortValue(&Fol->Root,
			WCS_ECOTYPE_USEIMGCOL))
                 {
                 ColorToGray(BMICur->Bitmap, BMICur->Width * BMICur->Height);
                 BMICur->Colors = 1;
		 } /* if don't need color */
                SetBitmapImageSpan(BMICur);
                BMICur->HeightPct = Rootstock_GetFloatValue(&Fol->Root, WCS_ECOTYPE_HEIGHT)
			* Rootstock_GetFloatValue(&FolGp->Root, WCS_ECOTYPE_HEIGHT);
		SumDensity +=
			Rootstock_GetFloatValue(&Fol->Root, WCS_ECOTYPE_DENSITY) *
			Rootstock_GetFloatValue(&FolGp->Root, WCS_ECOTYPE_DENSITY) /
			(SumGrpDens * SumImgDens);
                BMICur->DensityPct = SumDensity;
                BMICur->PalCol = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_PALCOL);
                EcoShift[i].BitmapImages ++;

                BMISmall = BMICur;

                while (BMISmall->Height > 10 && BMISmall->Width > 10 && ! error)
                 {
                 if ((BMISmall->Smaller = BitmapImage_New()))
                  {
                  BMIPrev = BMISmall;
                  BMISmall = BMISmall->Smaller;
                  BMISmall->Height = (BMIPrev->Height + 1) / 2;
                  BMISmall->Width = (BMIPrev->Width + 1) / 2;
                  BMISmall->Colors = BMIPrev->Colors;
                  BMISmall->HeightPct = BMIPrev->HeightPct;
                  BMISmall->DensityPct = BMIPrev->DensityPct;
                  BMISmall->PalCol = BMIPrev->PalCol;
                  for (j=0; j<BMISmall->Colors; j++)
                   {
                   if ((BMISmall->Bitmap[j] = 
			(UBYTE *)get_Memory(BMISmall->Width * BMISmall->Height, MEMF_CLEAR)) == NULL)
                    {
                    error = 1;
                    break;
                    } /* if no memory */
                   } /* for j=0... */
                  if (! error)
                   {
                   if ((BMISmall->Covg = (float *)
			get_Memory(BMISmall->Width * BMISmall->Height * sizeof (float),
			MEMF_CLEAR)))
                    {
                    if ((BMISmall->MidPt = (float *)
			get_Memory(BMISmall->Height * sizeof (float),
			MEMF_ANY)))
                     {
                     if ((BMISmall->Span = (float *)
			get_Memory(BMISmall->Height * sizeof (float),
			MEMF_ANY)))
                      {
                      BitmapImage_Scale(BMIPrev, BMISmall, 0.0, 0.0);
	              }
                     else
                      error = 1;
	             }
                    else
                     error = 1;
                    } /* if coverage array memory allocated */
                   else
                    error = 1;
                   } /* if bitmaps allocated */
                  if (error)
                   {
                   BitmapImage_Del(BMISmall);
                   BMIPrev->Smaller = NULL;
	           } /* if error */
                  } /* if new bitmap image structure allocated */
                 else
                  error = 1;
                 } /* while */
                MaxImgHt = Rootstock_GetShortValue(&Fol->Root, WCS_ECOTYPE_MAXIMGHT);
                while (BMICur->Smaller && BMICur->Height > MaxImgHt)
                 {
                 BMISmall = BMICur->Smaller;
                 BitmapImage_DelSingle(BMICur);
                 BMICur = BMISmall;
		 } /* while */
                *BMI = BMICur;
                BMI = &BMICur->Next;
	        } /* if image loaded */
               else
                error = 1;
	       } /* if span */
              else
               error = 1;
	      } /* if midpt */
             else
              error = 1;
	     } /* if not allocation error */
	    } /* if correct number of bit planes */
           if (error)
            {
            BitmapImage_Del(BMICur);
            *BMI = NULL;
	    }
	   } /* if new BitmapImage structure allocated */
          else
          {
           //KPrintF("AF: BitmapImage_New() failed\n");
           error = 1;
          }
          } /* if bitmap header read */
         else
         {
          //KPrintF("AF: read WCSBitMapHeader failed\n");
          error = 1;
         }
         } /* if IFF image file */
        else
        {
            //KPrintF("AF: FindIFFChunk() failed\n");
         error = 1;
        }
        } /* if IFF file */
       else
       {
           //KPrintF("AF: CheckIff() failed\n",filename);
        error = 1;
       }
       close(fh);
       } /* if file opened */
      else
      {
          //KPrintF("AF: open(%s) failed\n",filename);
       error = 1;
      }
      if (error)
       break;
      Fol = Fol->Next;
      } /* while Fol */
     if (error)
      break;
     FolGp = FolGp->Next;
     } /* while FolGp */
    } /* if */
   if (EcoShift[i].BitmapImages <= 0 || error)
    {
    if (! User_Message((CONST_STRPTR)PAR_NAME_ECO(i), (CONST_STRPTR)"A problem occurred loading at least one image\
 for this ecosystem!\n\
Continue without it or them?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
     success = 0;
    } /* if no images found and loaded */
   } /* if image ecosystem */
  } /* for i=0... */

/* some test stuff
for (i=0; i<ECOPARAMS; i++)
 {
 if (EcoShift[i].BMImage)
  {
  BMICur = EcoShift[i].BMImage;
  while (BMICur)
   {
   printf("%s %f %f %d %d %d\n", PAR_NAME_ECO(i), BMICur->HeightPct,
	 BMICur->DensityPct, BMICur->Height, BMICur->Width, BMICur->Colors);
   BMISmall = BMICur->Smaller;
   while (BMISmall)
    {
    printf("  %s %f %f %d %d %d\n", PAR_NAME_ECO(i), BMISmall->HeightPct,
	 BMISmall->DensityPct, BMISmall->Height, BMISmall->Width, BMISmall->Colors);
    BMISmall = BMISmall->Smaller;
    }
   BMICur = BMICur->Next;
   }
  }
 }
*/

 return (success);

} /* BitmapImage_Load() */

/**********/
#ifdef GHJDJSDHGJSGFJSDg
// obsolete
short BitmapImage_Load(void)
{
char filename[256];
short error, success = 1, Planes;
long i, j, k, fh;
struct BitmapImage **BMI, *BMICur, *BMIBase, *BMIPrev;
struct ILBMHeader Hdr;
struct WcsBitMapHeader BMHdr;
FILE *fImageList;

 for (i=0, error=0; i<ECOPARAMS && success; i++, error=0)
  {
  if (PAR_TYPE_ECO(i) == 10)
   {
   BMI = &EcoShift[i].BMImage;
 
   strmfp(filename, imagepath, PAR_MODEL_ECO(i));
   if (fImageList = fopen(filename, "r"))
    {
    for (k=0; ; k++, error=0)
     {
     if ((fgets(filename, 256, fImageList)) != NULL)
      {
      filename[strlen(filename) - 1] = 0;

      if ((fh = open(filename, O_RDONLY)) >= 0)
       {
       if (CheckIFF(fh, &Hdr))
        {
        if (FindIFFChunk(fh, &Hdr, "BMHD"))
         {
         if ((read(fh, (char *)&BMHdr, sizeof (struct WcsBitMapHeader))) ==
		 sizeof (struct WcsBitMapHeader))
          {
          close (fh);
          fh = -1;
          if (*BMI = BitmapImage_New())
           {
           BMICur = *BMI;
           if (BMHdr.Planes == 8)
            BMICur->Colors = 1;
           else if (BMHdr.Planes == 24)
            BMICur->Colors = 3;
           else
            error = 1;
           if (! error)
            {
            BMICur->Width = BMHdr.Width;
            BMICur->Height = BMHdr.Height;
            for (j=0; j<BMICur->Colors; j++)
             {
             if ((BMICur->Bitmap[j] = 
		(UBYTE *)get_Memory(BMICur->Width * BMICur->Height, MEMF_ANY)) == NULL)
              {
              error = 1;
              break;
	      } /* if no memory */
	     } /* for j=0... */
            if (! error)
             {
             if (BMICur->MidPt = 
		(float *)get_Memory(BMICur->Height * sizeof (float), MEMF_ANY))
              {
              if (BMICur->Span = 
		(float *)get_Memory(BMICur->Height * sizeof (float), MEMF_ANY))
               {
               if (LoadImage(filename, BMICur->Colors > 1, BMICur->Bitmap,
		BMICur->Width, BMICur->Height, 0, &BMICur->Width, &BMICur->Height,
		&Planes))
                {
                SetBitmapImageSpan(BMICur);
                EcoShift[i].BitmapImages ++;
                BMI = &BMICur->Next;
	        } /* if image loaded */
               else
                error = 1;
	       } /* if span */
              else
               error = 1;
	      } /* if midpt */
             else
              error = 1;
	     } /* if not allocation error */
	    } /* if correct number of bit planes */
           if (error)
            {
            BitmapImage_Del(BMICur);
            *BMI = NULL;
	    }
	   } /* if new BitmapImage structure allocated */
          else
           error = 1;
          } /* if bitmap header read */
         else
          error = 1;
         } /* if IFF image file */
        else
         error = 1;
        } /* if IFF file */
       else
        error = 1;
       if (fh >= 0)
        close(fh);
       } /* if file opened */
      else
       error = 1;
      } /* if an item is read OK */
     else
      break;
     } /* for k=0... */
    fclose(fImageList);
    } /* if image list file opened */
   if (EcoShift[i].BitmapImages <= 0)
    {
    if (! User_Message(PAR_NAME_ECO(i), "No images found for this ecosystem!\n\
Continue without them?", "OK|Cancel", "oc"))
     success = 0;
    } /* if no images found and loaded */
   else
    EcoShift[i].ImageIncr = 1.0 / EcoShift[i].BitmapImages;   
   } /* if image ecosystem */
  } /* for i=0... */ 

/* Scale the images for faster rendering */

 if (success)
  {
  for (i=0, error=0; i<ECOPARAMS && ! error; i++)
   {
   if (EcoShift[i].BMImage)
    {
    BMIBase = EcoShift[i].BMImage;

    while (BMIBase)
     {
     BMICur = BMIBase;

     while (BMICur->Height > 10 && BMICur->Width > 10 && ! error)
      {
      if (BMICur->Smaller = BitmapImage_New())
       {
       BMIPrev = BMICur;
       BMICur = BMICur->Smaller;
       BMICur->Height = (BMIPrev->Height + 1) / 2;
       BMICur->Width = (BMIPrev->Width + 1) / 2;
       BMICur->Colors = BMIPrev->Colors;
       for (j=0; j<BMICur->Colors; j++)
        {
        if ((BMICur->Bitmap[j] = 
		(UBYTE *)get_Memory(BMICur->Width * BMICur->Height, MEMF_CLEAR)) == NULL)
         {
         error = 1;
         break;
         } /* if no memory */
        } /* for j=0... */
       if (! error)
        {
        if (BMICur->Covg = (float *)
		get_Memory(BMICur->Width * BMICur->Height * sizeof (float),
		MEMF_CLEAR))
         {
         if (BMICur->MidPt = (float *)
		get_Memory(BMICur->Height * sizeof (float),
		MEMF_ANY))
          {
          if (BMICur->Span = (float *)
		get_Memory(BMICur->Height * sizeof (float),
		MEMF_ANY))
           {
           BitmapImage_Scale(BMIPrev, BMICur, 0.0, 0.0);
	   }
          else
           error = 1;
	  }
         else
          error = 1;
         } /* if coverage array memory allocated */
        else
         error = 1;
        } /* if bitmaps allocated */
       if (error)
        {
        BitmapImage_Del(BMICur);
        BMIPrev->Smaller = NULL;
	} /* if error */
       } /* if new bitmap image structure allocated */
      else
       error = 1;
      } /* while */
     BMIBase = BMIBase->Next;
     } /* while another image */
    } /* if bitmap image ecosystem */
   } /* for i=0... */
  } /* if sweet success */

 return (success);

} /* BitmapImage_Load() */
#endif
/**********************************************************************/

void BitmapImage_Unload(void)
{
long i;

 for (i=0; i<ECOPARAMS; i++)
  {
  if (EcoShift[i].BMImage)
   {
   BitmapImage_Del(EcoShift[i].BMImage);
   }
  EcoShift[i].BMImage = NULL;
  EcoShift[i].BitmapImages = 0;
  } /* for i=0... */

} /* BitmapImage_Unload() */

/**********************************************************************/

STATIC_FCN struct BitmapImage *BitmapImage_New(void) // used locally only -> static, AF 26.7.2021
{

 return ((struct BitmapImage *)get_Memory(sizeof (struct BitmapImage), MEMF_CLEAR));

} /* BitmapImage_New() */

/**********************************************************************/

STATIC_FCN void BitmapImage_Del(struct BitmapImage *BMI) // used locally only -> static, AF 23.7.2021
{
struct BitmapImage *BMINext;

 while (BMI)
  {
  BMINext = BMI->Next;
  if (BMI->Bitmap[0])
   free_Memory(BMI->Bitmap[0], BMI->Width * BMI->Height);
  if (BMI->Bitmap[1])
   free_Memory(BMI->Bitmap[1], BMI->Width * BMI->Height);
  if (BMI->Bitmap[2])
   free_Memory(BMI->Bitmap[2], BMI->Width * BMI->Height);
  if (BMI->Covg)
   free_Memory(BMI->Covg, BMI->Width * BMI->Height * sizeof (float));
  if (BMI->MidPt)
   free_Memory(BMI->MidPt, BMI->Height * sizeof (float));
  if (BMI->Span)
   free_Memory(BMI->Span, BMI->Height * sizeof (float));
  BitmapImage_Del(BMI->Smaller);
  free_Memory(BMI, sizeof (struct BitmapImage));
  BMI = BMINext;
  } /* while */

} /* BitmapImage_Del() */

/***********************************************************************/

STATIC_FCN void BitmapImage_DelSingle(struct BitmapImage *BMI) // used locally only -> static, AF 26.7.2021
{

 if (BMI)
  {
  if (BMI->Bitmap[0])
   free_Memory(BMI->Bitmap[0], BMI->Width * BMI->Height);
  if (BMI->Bitmap[1])
   free_Memory(BMI->Bitmap[1], BMI->Width * BMI->Height);
  if (BMI->Bitmap[2])
   free_Memory(BMI->Bitmap[2], BMI->Width * BMI->Height);
  if (BMI->Covg)
   free_Memory(BMI->Covg, BMI->Width * BMI->Height * sizeof (float));
  if (BMI->MidPt)
   free_Memory(BMI->MidPt, BMI->Height * sizeof (float));
  if (BMI->Span)
   free_Memory(BMI->Span, BMI->Height * sizeof (float));
  free_Memory(BMI, sizeof (struct BitmapImage));
  } /* if */

} /* BitmapImage_DelSingle() */

/***********************************************************************/

STATIC_FCN void ColorToGray(UBYTE **Bitmap, long MaxZip) // used locally only -> static, AF 23.7.2021
{
long zip;

 for (zip=0; zip<MaxZip; zip++)
  {
  Bitmap[0][zip] = ((unsigned int)Bitmap[0][zip] + (unsigned int)Bitmap[1][zip]
	 + (unsigned int)Bitmap[2][zip]) / 3;
  } /* for zip=0... */

 free_Memory(Bitmap[1], MaxZip);
 free_Memory(Bitmap[2], MaxZip);
 Bitmap[1] = Bitmap[2] = NULL;

} /* ColorToGray() */

/***********************************************************************/

STATIC_FCN void SetBitmapImageSpan(struct BitmapImage *BMI) // used locally only -> static, AF 19.7.2021
{
long x, y, zip = 0, FirstPt, LastPt;

 for (y=0; y<BMI->Height; y++)
  {
  LastPt = 0;
  FirstPt = -1;
  for (x=0; x<BMI->Width; x++, zip++)
   {
   if (BMI->Colors == 3)
    {
    if (BMI->Bitmap[0][zip] || BMI->Bitmap[1][zip] || BMI->Bitmap[2][zip])
     {
     if (FirstPt == -1)
      FirstPt = x;
     LastPt = x;
     } /* if */
    } /* if three colors */
   else
    {
    if (BMI->Bitmap[0][zip])
     {
     if (FirstPt == -1)
      FirstPt = x;
     LastPt = x;
     } /* if */
    } /* else */
   } /* for x=0... */
  BMI->Span[y] = 1 + LastPt - FirstPt;
  BMI->MidPt[y] = (1 + LastPt + FirstPt) / 2.0;
  } /* for y=0... */

} /* SetBitmapImageSpan() */

/**********************************************************************/

STATIC_FCN void BitmapImage_Scale(struct BitmapImage *SBMI, struct BitmapImage *DBMI,
	double Dx, double Dy) // used locally only -> static, AF 26.7.2021
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtys, wtye, wty, wtxs, wtxe, wt, PixWt, MaxWt,
	StartWt[4], EndWt[4], SumFirstPt, SumLastPt, SumFirstWt, SumLastWt;
long Px, Py, Pxp1, Pyp1, x, y, DRows, DCols, DxStart, DyStart, PixVal[3],
	zip, SourceZip, i, j, suby, StartPt[4], EndPt[4];

 Dox = Dx;
 Dex = Dx + DBMI->Width;
 Doy = Dy;
 Dey = Dy + DBMI->Height;

 dX = (double)SBMI->Width / (double)DBMI->Width;
 dY = (double)SBMI->Height / (double)DBMI->Height;

 MaxWt = dX * dY;

 Sox = ((int)Dox - Dox) * dX;
 Soy = ((int)Doy - Doy) * dY;

 DxStart = Dox; 
 DyStart = Doy; 
 DCols = Dex - DxStart + 1.0;
 DRows = Dey - DyStart + 1.0;

 for (y=DyStart, Coy=Soy, Cey=Soy+dY, j=0, PixWt=0.0,
	PixVal[0]=PixVal[1]=PixVal[2]=0;
	j<DRows; j++, y++, Coy+=dY, Cey+=dY)
  {
  if (y < 0)
   continue;
  if (y >= DBMI->Height)
   break;
  zip = y * DBMI->Width + DxStart;
  SumFirstPt = SumLastPt = SumFirstWt = SumLastWt = 0.0;
  for (suby=0; suby<4; suby++)
   {
   StartPt[suby] = -1;
   } /* for suby=0... */

  for (x=DxStart, Cox=Sox, Cex=Sox+dX, i=0; i<DCols;
	i++, x++, Cox+=dX, Cex+=dX, PixVal[0]=PixVal[1]=PixVal[2]=0,
	PixWt=0.0, zip++)
   {
   if (x < 0)
    continue;
   if (x >= DBMI->Width)
    break;

   for (Py=Coy, Pyp1=Coy+1, suby=0; Py<Cey && Py<SBMI->Height; Py++, Pyp1++, suby++)
    {
    if (Py < 0)
     continue;
    wtys = Py > Coy ? 1.0: Pyp1 - Coy; 
    wtye = Pyp1 < Cey ? 1.0: Cey - Py;
    wty = wtys * wtye;
    for (Px=Cox, Pxp1=Cox+1; Px<Cex && Px<SBMI->Width; Px++, Pxp1++)
     {
     if (Px < 0)
      continue;
     wtxs = Px > Cox ? 1.0: Pxp1 - Cox; 
     wtxe = Pxp1 < Cex ? 1.0: Cex - Px;
     wt = wty * wtxs * wtxe;
     SourceZip = Py * SBMI->Width + Px;
     if (SBMI->Covg)
      wt *= SBMI->Covg[SourceZip];
     if (DBMI->Colors == 3)
      {
      if (SBMI->Bitmap[0][SourceZip] || SBMI->Bitmap[1][SourceZip] || SBMI->Bitmap[2][SourceZip])
       {
       PixWt += wt;
       PixVal[0] += wt * (unsigned int)SBMI->Bitmap[0][SourceZip];
       PixVal[1] += wt * (unsigned int)SBMI->Bitmap[1][SourceZip];
       PixVal[2] += wt * (unsigned int)SBMI->Bitmap[2][SourceZip];
       if (StartPt[suby] < 0)
        {
        StartPt[suby] = Px;
        StartWt[suby] = wt;
	} /* if */
       EndPt[suby] = Px;
       EndWt[suby] = wt;
       } /* if */
      } /* if 24 bit */
     else
      {
      if (SBMI->Bitmap[0][SourceZip])
       {
       PixWt += wt;
       PixVal[0] += wt * (unsigned int)SBMI->Bitmap[0][SourceZip];
       if (StartPt[suby] < 0)
        {
        StartPt[suby] = Px;
        StartWt[suby] = wt;
	} /* if */
       EndPt[suby] = Px;
       EndWt[suby] = wt;
       } /* if */
      } /* else only gray scale */
     } /* for Px=... */
    } /* for Py=... */

   if (DBMI->Colors == 3)
    {
    if ((PixVal[0] || PixVal[1] || PixVal[2]) && PixWt > 0.0)
     {
     PixVal[0] /= PixWt;
     PixVal[1] /= PixWt;
     PixVal[2] /= PixWt;
     PixWt /= MaxWt;
     if (PixWt > 1.0)
      PixWt = 1.0;
     DBMI->Bitmap[0][zip] = PixVal[0];
     DBMI->Bitmap[1][zip] = PixVal[1];
     DBMI->Bitmap[2][zip] = PixVal[2];
     DBMI->Covg[zip] = PixWt;
     }
    else
     {
     DBMI->Covg[zip] = 0.0;
     } /* else */
    } /* if 3 colors */
   else
    {
    if (PixVal[0] && PixWt > 0.0)
     {
     PixVal[0] /= PixWt;
     PixWt /= MaxWt;
     if (PixWt > 1.0)
      PixWt = 1.0;
     DBMI->Bitmap[0][zip] = PixVal[0];
     DBMI->Covg[zip] = PixWt;
     } /* if */
    else
     {
     DBMI->Covg[zip] = 0.0;
     } /* else */
    } /* else */
   } /* for x=... */
  for (suby=0; suby<4; suby++)
   {
   if (StartPt[suby] >= 0)
    {
    SumFirstPt += (StartPt[suby] * StartWt[suby]);
    SumFirstWt += StartWt[suby];
    SumLastPt += (EndPt[suby] * EndWt[suby]);
    SumLastWt += EndWt[suby];
    } /* if a point found */
   } /* for suby=0... */
  if (SumFirstWt > 0.0)
   SumFirstPt /= SumFirstWt;
  if (SumLastWt > 0.0)
   SumLastPt /= SumLastWt;
  DBMI->Span[y] = ((1.0 + SumLastPt - SumFirstPt) * DBMI->Width) / SBMI->Width;
  DBMI->MidPt[y] = (((1.0 + SumLastPt + SumFirstPt) / 2.0) * DBMI->Width)
	 / SBMI->Width;
  } /* for y=... */

} /* BitmapImage_Scale() */

/**********************************************************************/

STATIC_FCN void Image_Paste(struct BitmapImage *SBMI, UBYTE **Bitmap,
	double Dw, double Dh, double Dx, double Dy,
	double Distance, struct ColorComponents *CC, 
	struct ColorComponents *AM, double ElStart,
	double ElIncr, struct Window *win, struct QCvalues *QC) // used locally only -> static, AF 23.7.2021
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtys, wtye, wty, wtxs, wtxe, wt, PixWt, InvPixWt, MaxWt, ElPt,
	VertSun, HorSun, VertSunIncr, MidX, Span, Angle;
long Px, Py, Pxp1, Pyp1, x, y, DRows, DCols, DxStart, DyStart, PixVal[3],
	zip, SourceZip, i, j, Mode;

 Dox = Dx;
 Dex = Dx + Dw;
 Doy = Dy;
 Dey = Dy + Dh;

 dX = (double)SBMI->Width / Dw;
 dY = (double)SBMI->Height / Dh;

 MaxWt = dX * dY;

 Sox = ((int)Dox - Dox) * dX;
 Soy = ((int)Doy - Doy) * dY;

 DxStart = Dox; 
 DyStart = Doy; 
 DCols = Dex - DxStart + 1.0;
 DRows = Dey - DyStart + 1.0;

 VertSunIncr = VertSunFact / (SBMI->Height - 1);

 for (y=DyStart, Coy=Soy, Cey=Soy+dY, j=0, PixWt= 0.0,
	PixVal[0]=PixVal[1]=PixVal[2]=0, ElPt=ElStart;
	j<DRows; j++, y++, Coy+=dY, Cey+=dY, ElPt+=ElIncr)
  {
  if (y < 0)
   continue;
  if (y >= high)
   break;
  zip = scrnrowzip[y] + DxStart;
  for (x=DxStart, Cox=Sox, Cex=Sox+dX, i=0; i<DCols;
	i++, x++, Cox+=dX, Cex+=dX, PixVal[0]=PixVal[1]=PixVal[2]=0,
	PixWt=0.0, zip++)
   {
   if (x < 0)
    continue;
   if (x >= wide)
    break;
   for (Py=Coy, Pyp1=Coy+1; Py<Cey && Py<SBMI->Height; Py++, Pyp1++)
    {
    if (Py < 0)
     continue;
    VertSun = VertSunFact - Py * VertSunIncr;
    VertSun *= VertSun;
    wtys = Py > Coy ? 1.0: Pyp1 - Coy; 
    wtye = Pyp1 < Cey ? 1.0: Cey - Py;
    wty = wtys * wtye;
    MidX = SBMI->MidPt[Py];
    Span = SBMI->Span[Py]; 
    for (Px=Cox, Pxp1=Cox+1; Px<Cex && Px<SBMI->Width; Px++, Pxp1++)
     {
     if (Px < 0)
      continue;
     wtxs = Px > Cox ? 1.0: Pxp1 - Cox; 
     wtxe = Pxp1 < Cex ? 1.0: Cex - Px;
     wt = wty * wtxs * wtxe;
     SourceZip = Py * SBMI->Width + Px;
     if (SBMI->Covg)
      wt *= SBMI->Covg[SourceZip];
     if (SBMI->Colors == 3)
      {
      if (SBMI->Bitmap[0][SourceZip] || SBMI->Bitmap[1][SourceZip] || SBMI->Bitmap[2][SourceZip])
       {
       Angle = (Px + .5 - MidX) / Span;
       if (Angle > 1.0)
        Angle = 1.0;
       else if (Angle < -1.0)
        Angle = -1.0;
       Angle = HorSunAngle + ASin_Table(Angle);
       if (fabs(Angle) > HalfPi)
        HorSun = 0.0;
       else
        {
        HorSun = Cos_Table(Angle);
        HorSun *= HorSunFact;
        HorSun *= HorSun;
        } /* else */
       HorSun += VertSun;
       HorSun += ((1.0 - HorSun) * .5);
       HorSun = 1.0 - (1.0 - HorSun) * PARC_RNDR_MOTION(22);
       PixWt += wt;
       PixVal[0] += HorSun * wt * (unsigned int)SBMI->Bitmap[0][SourceZip];
       PixVal[1] += HorSun * wt * (unsigned int)SBMI->Bitmap[1][SourceZip];
       PixVal[2] += HorSun * wt * (unsigned int)SBMI->Bitmap[2][SourceZip];
       }
      }
     else
      {
      if (SBMI->Bitmap[0][SourceZip])
       {
       Angle = (Px + .5 - MidX) / Span;
       if (Angle > 1.0)
        Angle = 1.0;
       else if (Angle < -1.0)
        Angle = -1.0;
       Angle = HorSunAngle + ASin_Table(Angle);
       if (fabs(Angle) > HalfPi)
        HorSun = 0.0;
       else
        {
        HorSun = Cos_Table(Angle);
        HorSun *= HorSunFact;
        HorSun *= HorSun;
        } /* else */
       HorSun += VertSun;
       HorSun += ((1.0 - HorSun) * .5);
       HorSun = 1.0 - (1.0 - HorSun) * PARC_RNDR_MOTION(22);
       PixWt += wt;
       PixVal[0] += HorSun * wt * (unsigned int)SBMI->Bitmap[0][SourceZip];
       }
      }
     } /* for Px=... */
    } /* for Py=... */
   if (SBMI->Colors == 3)
    {
    if ((PixVal[0] || PixVal[1] || PixVal[2]) && PixWt > 0.0)
     {
     if (Distance <= zbuf[zip] || bytemap[zip] < 100)
      {
      PixVal[0] /= PixWt;
      PixVal[1] /= PixWt;
      PixVal[2] /= PixWt;
      PixVal[0] -= sunshade * PixVal[0];			/* shading */
      PixVal[1] -= sunshade * PixVal[1];
      PixVal[2] -= sunshade * PixVal[2];
      PixVal[0] += AM->Red;					/* ambient */
      PixVal[1] += AM->Grn;
      PixVal[2] += AM->Blu;
      PixVal[0] += (PARC_RNDR_COLOR(2, 0) - PixVal[0]) * fade;
      PixVal[1] += (PARC_RNDR_COLOR(2, 1) - PixVal[1]) * fade;
      PixVal[2] += (PARC_RNDR_COLOR(2, 2) - PixVal[2]) * fade;
      PixVal[0] += (PARC_RNDR_COLOR(2, 0) - PixVal[0]) * fog;
      PixVal[1] += (PARC_RNDR_COLOR(2, 1) - PixVal[1]) * fog;
      PixVal[2] += (PARC_RNDR_COLOR(2, 2) - PixVal[2]) * fog;
      PixVal[0] *= redsun;
      PixVal[1] *= greensun;
      PixVal[2] *= bluesun;
      if (PixVal[0] > 255) PixVal[0] = 255;
      else if (PixVal[0] < 0) PixVal[0] = 0;
      if (PixVal[1] > 255) PixVal[1] = 255;
      else  if (PixVal[1] < 0) PixVal[1] = 0;
      if (PixVal[2] > 255) PixVal[2] = 255;
      else if (PixVal[2] < 0) PixVal[2] = 0;

      PixWt /= MaxWt;
      if (PixWt > 1.0)
       PixWt = 1.0;
      if (render & 0x01)
       {
       InvPixWt = 1.0 - PixWt;
       if (bytemap[zip] && (Distance >= zbuf[zip] ||
	 ((PixWt <= .999) && (zbuf[zip] - Distance < PtrnOffset))))
        Mode = MODE_AVERAGE;
       else
        Mode = MODE_REPLACE;
       if (Mode)
        {
        Bitmap[0][zip] = PixWt * PixVal[0] + Bitmap[0][zip] * InvPixWt;
        Bitmap[1][zip] = PixWt * PixVal[1] + Bitmap[1][zip] * InvPixWt;
        Bitmap[2][zip] = PixWt * PixVal[2] + Bitmap[2][zip] * InvPixWt;
	}
       else
        {
        Bitmap[0][zip] = PixVal[0];
        Bitmap[1][zip] = PixVal[1];
        Bitmap[2][zip] = PixVal[2];
	}
       } /* if render to bitmaps */
      if (Distance < zbuf[zip])
       {
       if (render & 0x10)
        {
        if (render & 0x01)
         {
         ScreenPixelPlot(win, Bitmap, x + drawoffsetX, y + drawoffsetY, zip);
	 } /* if */
        else
         {
         NoRGBScreenPixelPlot(win, FloatCol, ColMax, x + drawoffsetX, y + drawoffsetY);
         } /* if */
	} /* if */
       if (render & 0x100)
        {
        *(QCmap[0] + zip) = QC->compval1;
        *(QCmap[1] + zip) = QC->compval2;
        *(QCmap[2] + zip) = QC->compval3;
        *(QCcoords[0] + zip) = facelat;
        *(QCcoords[1] + zip) = facelong;
        } /* if render to QC buffers */
       zbuf[zip] = Distance;
       if (ElevationMap)
        ElevationMap[zip] = ElPt;
       } /* if */
      bytemap[zip] += (101.0 * PixWt);
      } /* if */
     } /* if */
    } /* if 3 colors */
   else
    {
    if (PixVal[0] && PixWt > 0.0)
     {
     if (Distance <= zbuf[zip] || bytemap[zip] < 100)
      {
      PixVal[0] /= PixWt;
      PixWt /= MaxWt;
      if (PixWt > 1.0)
       PixWt = 1.0;
      if (render & 0x01)
       {
       InvPixWt = 1.0 - PixWt;
       PixVal[1] = PixVal[0] * (CC->Grn / 255.0);
       PixVal[2] = PixVal[0] * (CC->Blu / 255.0);
       PixVal[0] *= (CC->Red / 255.0);
/*
       PixVal[0] += AM->Red;					// ambient
       PixVal[1] += AM->Grn;
       PixVal[2] += AM->Blu;
       PixVal[0] += (PARC_RNDR_COLOR(2, 0) - PixVal[0]) * fade;
       PixVal[1] += (PARC_RNDR_COLOR(2, 1) - PixVal[1]) * fade;
       PixVal[2] += (PARC_RNDR_COLOR(2, 2) - PixVal[2]) * fade;
       PixVal[0] += (PARC_RNDR_COLOR(2, 0) - PixVal[0]) * fog;
       PixVal[1] += (PARC_RNDR_COLOR(2, 1) - PixVal[1]) * fog;
       PixVal[2] += (PARC_RNDR_COLOR(2, 2) - PixVal[2]) * fog;
       PixVal[0] *= redsun; 
       PixVal[1] *= greensun;
       PixVal[2] *= bluesun;
*/
       if (PixVal[0] > 255) PixVal[0] = 255;
       else if (PixVal[0] < 0) PixVal[0] = 0;
       if (PixVal[1] > 255) PixVal[1] = 255;
       else  if (PixVal[1] < 0) PixVal[1] = 0;
       if (PixVal[2] > 255) PixVal[2] = 255;
       else if (PixVal[2] < 0) PixVal[2] = 0;

       if (bytemap[zip] && (Distance >= zbuf[zip] ||
	 ((PixWt <= .999) && (zbuf[zip] - Distance < PtrnOffset))))
        Mode = MODE_AVERAGE;
       else
        Mode = MODE_REPLACE;
       if (Mode)
        {
        Bitmap[0][zip] = PixWt * PixVal[0] + Bitmap[0][zip] * InvPixWt;
        Bitmap[1][zip] = PixWt * PixVal[1] + Bitmap[1][zip] * InvPixWt;
        Bitmap[2][zip] = PixWt * PixVal[2] + Bitmap[2][zip] * InvPixWt;
	}
       else
        {
        Bitmap[0][zip] = PixVal[0];
        Bitmap[1][zip] = PixVal[1];
        Bitmap[2][zip] = PixVal[2];
	}
       } /* if */
      if (Distance < zbuf[zip])
       {
       if (render & 0x10)
        {
        if (render & 0x01)
         {
         ScreenPixelPlot(win, Bitmap, x + drawoffsetX, y + drawoffsetY, zip);
	 } /* if */
        else
         {
         NoRGBScreenPixelPlot(win, FloatCol, ColMax, x + drawoffsetX, y + drawoffsetY);
         } /* if */
	} /* if render to screen */
       if (render & 0x100)
        {
        *(QCmap[0] + zip) = QC->compval1;
        *(QCmap[1] + zip) = QC->compval2;
        *(QCmap[2] + zip) = QC->compval3;
        *(QCcoords[0] + zip) = facelat;
        *(QCcoords[1] + zip) = facelong;
        } /* if render to QC buffers */
       zbuf[zip] = Distance;
       if (ElevationMap)
        ElevationMap[zip] = ElPt;
       } /* if */
      bytemap[zip] += (101.0 * PixWt);
      } /* if */
     } /* if */
    } /* else */
   } /* for Tox=... */
  } /* for Toy=... */

} /* Image_Paste() */

