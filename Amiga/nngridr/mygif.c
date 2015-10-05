/*----------------------- mygif.c ----------------------------*/
#include "mygif.h"

void GifMaker(void)
{
   double z0, z1, z2;
   int i1, i2, i3, i4, row, col, xextra1, xextra2, 
       oldrow = 0, newrow = 1;
   char ch;

   if ((dfile = fopen(grd_file,"r")) EQ NULL)
   {  fprintf(stderr,"\nCannot open grid file %s",
         grd_file);
      exit(1);
   }
   if (idisplay EQ 0) DataMaxMin();
   else
   {  if (idisplay EQ 1)
      {  if (rads) top = PIE;
         else top = 180;
         top2 = 2 * top;
      }
      else 
      {  if (rads) top = PIEBY2;
         else top = 90;
      }
      bottom = 0;
   }
   if (z_only)  
   {  while ((ch = fgetc(dfile)) NE '\n');
      while ((ch = fgetc(dfile)) NE '\n');
   }
   span = WEEBIT + 30 * (top - bottom) / 31;
   rootnode = IMakeCodenode(); 
   rootnode->nodecode = -367; /* dummy */
   for (i1=0; i1<CLEAR; i1++) 
   {  rootnode->nextnode[i1] = IMakeCodenode();
      rootnode->nextnode[i1]->nodecode = i1;
   }
   numrows = y_nodes + (y_nodes - 1) * y_extra;
   numcols = x_nodes + (x_nodes - 1) * x_extra;
   tworows = DoubleMatrix(2, numcols);
   MakeHeader();
   InitTable();
   WriteHeader();
   if (x_extra > 0)
   {  xprop = DoubleMatrix(x_extra, 2);
      xhold = (double) x_extra + 1;
      for (i1=0; i1<x_extra; i1++)
      {  xprop[i1][1] = (double)(i1 + 1) / xhold;
         xprop[i1][0] = 1 - xprop[i1][1];
      }
   }
   if (y_extra > 0)
   {  yprop = DoubleMatrix(y_extra, 2);
      xhold = (double) y_extra + 1;
      for (i1=0; i1<y_extra; i1++)
      {  yprop[i1][0] = (double)(i1 + 1) / xhold;
         yprop[i1][1] = 1 - yprop[i1][0];
      }
   }
   switch (tgrid)
   {  case 0:  /* rectangular grid */    
      {  z0 = ReadDatum();
         if (idisplay EQ 1) if (z0 > top) 
            z0 = top2 - z0;
         tworows[0][0] = z0;
         PostColor(z0);
         i4 = 1 + (i3 = 0);
         for (col=1; col<x_nodes; col++) 
         {  for (i2=-1; i2<x_extra; i2++) i3++;
            z1 = ReadDatum();
            if (idisplay EQ 1) if (z1 > top) 
               z1 = top2 - z1;
            tworows[0][i3] = z1;
            if (x_extra > 0)
            {  for (i2=0; i2<x_extra; i2++)
               {  z2 = xprop[i2][0] * z0 + 
                     xprop[i2][1] * z1;
                  tworows[0][i4 + i2] = z2;
                  PostColor(z2);
               }
               i4 = 1 + i3;
               z0 = z1;
            }
            PostColor(z1);
         }
         for (row=1; row<y_nodes; row++)
         {  z0 = ReadDatum();
            if (idisplay EQ 1) if (z0 > top) 
               z0 = top2 - z0;
            tworows[newrow][0] = z0;
            i4 = 1 + (i3 = 0);
            for (i1=1; i1<x_nodes; i1++) 
            {  for (i2=-1; i2<x_extra; i2++) i3++;
               z1 = ReadDatum();
               if (idisplay EQ 1) if (z1 > top) 
                  z1 = top2 - z1;
               tworows[newrow][i3] = z1;
               if (x_extra > 0)
               {  for (i2=0; i2<x_extra; i2++)
                     tworows[newrow][i4 + i2] = 
                        xprop[i2][0] * z0 + 
                        xprop[i2][1] * z1;
                  i4 = 1 + i3;
                  z0 = z1;
               }
            }
            if (y_extra > 0)
            {  for (i1=0; i1<y_extra; i1++)
               {  for (col=0; col<numcols; col++)
                  {  z2 = yprop[i1][0] * 
                        tworows[newrow][col] + 
                        yprop[i1][1] * 
                        tworows[oldrow][col];
                     PostColor(z2);
                  }
               }
            }
            for (col=0; col<numcols; col++) 
               PostColor(tworows[newrow][col]);
            if (y_extra > 0)
            {  i4 = oldrow;
               oldrow = newrow;
               newrow = i4;
            }
         }
         break;
      }
      case 1: /* triangular grid */
      {  xnodes2 = x_nodes / 2;
         xextra1 = 1 + x_extra;
         xextra2 = 1 + 2 * x_extra;
         xprop2 = DoubleMatrix(xextra2, 2);
         xhold2 = (double) xextra2 + 1;
         for (i1=0; i1<xextra2; i1++)
         {  xprop2[i1][1] = (double)(i1 + 1) / xhold2;
            xprop2[i1][0] = 1 - xprop2[i1][1];
         }
         i4 = 1 + (i3 = 0);  /* first row, offset */
         z0 = ReadDatum();
         if (idisplay EQ 1) if (z1 > top) 
            z0 = top2 - z0;
         tworows[oldrow][i3] = z0;
         for (col=1; col<xnodes2; col++) 
         {  for (i2=-1; i2<xextra2; i2++) i3++;
            z1 = ReadDatum();
            if (idisplay EQ 1) if (z1 > top) 
               z1 = top2 - z1;
            tworows[oldrow][i3] = z1;
            for (i2=0; i2<xextra2; i2++)
            {  z2 = xprop2[i2][0] * z0 + 
                  xprop2[i2][1] * z1;
               tworows[oldrow][i4 + i2] = z2;
            }
            i4 = i3 + 1;
            z0 = z1;
         }
         i4 = 1 + (i3 = xextra1);  /* offset, second row */
         z0 = ReadDatum();          
         if (idisplay EQ 1) if (z0 > top) 
            z0 = top2 - z0;
         tworows[newrow][i3] = z0;
         for (col=1; col<xnodes2; col++) 
         {  for (i2=-1; i2<xextra2; i2++) i3++;
            z1 = ReadDatum();
            if (idisplay EQ 1) if (z1 > top) 
               z1 = top2 - z1;
            tworows[newrow][i3] = z1;
            for (i2=0; i2<xextra2; i2++)
            {  z2 = xprop2[i2][0] * z0 + 
                  xprop2[i2][1] * z1;
               tworows[newrow][i4 + i2] = z2;
            }
            i4 = i3 + 1;
            z0 = z1;
         }
         i4 = 1 + (i3 = numcols - xextra1 - 1);  /* offset, second row */
         z1 = tworows[oldrow][numcols - 1] 
            = tworows[newrow][numcols - 1] + 
            tworows[oldrow][i3] - tworows[newrow][i3];
         if (x_extra > 0)
         {  for (i2=0; i2<x_extra; i2++)
            {  z2 = xprop[i2][0] * tworows[oldrow][i3] 
                  + xprop[i2][1] * z1;
               tworows[oldrow][i4 + i2] = z2;
            }
         }
         z0 = tworows[newrow][0]  /* first on second row */
            = tworows[oldrow][0] + 
            tworows[newrow][xextra1] - 
            tworows[oldrow][xextra1];
         if (x_extra > 0)
         {  for (i2=0; i2<x_extra; i2++)
            {  z2 = xprop[i2][0] * z0 + xprop[i2][1] * 
                  tworows[newrow][xextra1];
               tworows[newrow][1 + i2] = z2;
            }
         }
         for (col=0; col<numcols; col++) 
            PostColor(tworows[oldrow][col]); /* output first input row */
         if (y_extra > 0)
         {  for (i1=0; i1<y_extra; i1++)
            {  for (col=0; col<numcols; col++)
               {  z2 = yprop[i1][0] * 
                     tworows[newrow][col] + 
                     yprop[i1][1] * 
                     tworows[oldrow][col];
                  PostColor(z2); /* output intermediate row(s) */                    
               }
            }
         }
         for (col=0; col<numcols; col++) 
            PostColor(tworows[newrow][col]); /* output second input row */
         for (row=2; row<y_nodes; row++)
         {  i4 = oldrow;
            oldrow = newrow;
            newrow = i4;
            if (oldrow)   /* newrow is row, offset */
            {  i4 = 1 + (i3 = 0);  
               z1 = ReadDatum();
               if (idisplay EQ 1) if (z1 > top) 
                  z1 = top2 - z1;
               tworows[newrow][i3] = z1;
               for (col=1; col<xnodes2; col++) 
               {  for (i2=-1; i2<xextra2; i2++) i3++;
                  z1 = ReadDatum();
                  if (idisplay EQ 1) if (z1 > top) 
                     z1 = top2 - z1;
                  tworows[newrow][i3] = z1;
                  for (i2=0; i2<xextra2; i2++)
                  {  z2 = xprop2[i2][0] * z0 + 
                        xprop2[i2][1] * z1;
                     tworows[newrow][i4 + i2] = z2;
                  }
                  i4 = i3 + 1;
                  z0 = z1;
               }
               i4 = 1 + (i3 = numcols - xextra1 - 1);
               z1 = tworows[newrow][numcols - 1] 
                  = tworows[oldrow][numcols - 1] + 
                  tworows[newrow][i3] - 
                  tworows[oldrow][i3];
               if (x_extra > 0)
               {  for (i2=0; i2<x_extra; i2++)
                  {  z2 = xprop[i2][0] * 
                        tworows[newrow][i3] + 
                        xprop[i2][1] * z1;
                     tworows[newrow][i4 + i2] = z2;
                  }
               }
            }
            else     /* newrow is offset, row */
            {  i4 = 1 + (i3 = xextra1);  
               z0 = ReadDatum();          
               if (idisplay EQ 1) if (z0 > top) 
                  z0 = top2 - z0;
               tworows[newrow][i3] = z0;
               for (col=1; col<xnodes2; col++) 
               {  for (i2=-1; i2<xextra2; i2++) i3++;
                  z1 = ReadDatum();
                  if (idisplay EQ 1) if (z1 > top) 
                     z1 = top2 - z1;
                  tworows[newrow][i3] = z1;
                  for (i2=0; i2<xextra2; i2++)
                  {  z2 = xprop2[i2][0] * z0 + 
                        xprop2[i2][1] * z1;
                     tworows[newrow][i4 + i2] = z2;
                  }
                  i4 = i3 + 1;
                  z0 = z1;
               }
               z0 = tworows[newrow][0] 
                  = tworows[oldrow][0] + 
                  tworows[newrow][xextra1] - 
                  tworows[oldrow][xextra1];
               if (x_extra > 0)
               {  for (i2=0; i2<x_extra; i2++)
                  {  z2 = xprop[i2][0] * z0 + 
                        xprop[i2][1] * 
                        tworows[newrow][xextra1];
                     tworows[newrow][1 + i2] = z2;
                  }
               }
            }
            if (y_extra > 0)
            {  for (i1=0; i1<y_extra; i1++)
               {  for (col=0; col<numcols; col++)
                  {  z2 = yprop[i1][0] * 
                        tworows[newrow][col] + 
                        yprop[i1][1] * 
                        tworows[oldrow][col];
                     PostColor(z2); /* output intermediate row(s) */                   
                  }
               }
            }
            for (col=0; col<numcols; col++) 
               PostColor(tworows[newrow][col]);
         }
         break;
      }
      default:;
   }
   Terminator();
   fclose(dfile);
   fclose(gfile);
   FreeMatrixd(tworows);
   if (x_extra > 0) FreeMatrixd(xprop);
   if (y_extra > 0) FreeMatrixd(yprop);
   if (tgrid) FreeMatrixd(xprop2);
}

/***********************************************************************/

void PostColor(double zz)
{
   short int color;
   if (idisplay EQ 1) if (southhemi) zz = top - zz;
   color = 1 + (short int) 30 * (zz - bottom) / span; 
   EncodeColor(color);
}

/***********************************************************************/

double ReadDatum(void)
{
   double v, w, x, y, z;
   switch (togif)
   {  case 1:
      {  fscanf(dfile,"%lf",&z);
         break;
      }
      case 2:
      {  fscanf(dfile,"%lf%lf%lf",&z,&x,&y);
         break;
      }
      case 3:
      {  fscanf(dfile,"%lf%lf%lf",&y,&z,&x);
         break;
      }
      case 4:
      {  fscanf(dfile,"%lf%lf%lf",&x,&y,&z);
         break;
      }
      case 5:
      {  fscanf(dfile,"%lf%lf%lf%lf%lf",
            &x,&y,&z,&v,&w);
         break;
      }
      case 6:
      {  fscanf(dfile,"%lf%lf%lf%lf%lf",
            &w,&x,&y,&z,&v);
         break;
      }
      case 7:
      {  fscanf(dfile,"%lf%lf%lf%lf%lf",
            &v,&w,&x,&y,&z);
         break;
      }
      default:;
   }
   return z;
}

/***********************************************************************/

void InitTable(void)
{
   codesize = CODSTA;
   codelim = CODLIM;
   topcode = STACOD;
   curnode = rootnode;
}

/***********************************************************************/

void EncodeColor(short color)
{
   if (curnode->nextnode[color] NE NULL) 
      curnode = curnode->nextnode[color];
   else
   {  SendCode(curnode->nodecode);
      curnode->nextnode[color] = IMakeCodenode();
      if (topcode > codelim)
      {  codelim = 2 * (codelim + 1) - 1;
         codesize++;
      }
      curnode->nextnode[color]->nodecode = topcode;
      topcode++;
      if (topcode > 4095) StartNewTable();
      curnode = rootnode->nextnode[color];
   }
}

/***********************************************************************/

void SendCode(short codenum)
{
   int i1;
   short int anum = 0x0000, bnum = 0x0001, 
             cnum = 0x0001;
   anum = thebyte; 
   for (i1=0; i1<byte_index; i1++) cnum <<= 1;
   for (i1=0; i1<codesize; i1++)
   {  if (codenum & bnum) anum |= cnum;
      bnum <<= 1;
      cnum <<= 1;
      if (byte_index < 7) byte_index++;
      else
      {  thebyte = (char) anum; 
         block[block_index] = thebyte;
         if (block_index > 253)
         {  block[0] = (char) block_index;
            SendBlock2File();
            block_index = 1;
         }
         else block_index++;
         byte_index = 0;
         anum = 0x0000;
         cnum = 0x0001;
      }
   }
   thebyte = (char) anum;
   bitcount += codesize;
}

/***********************************************************************/

void SendBlock2File(void)
{
   int i1;
   block_index++;
   for (i1=0; i1<block_index; i1++) 
      fputc(block[i1],gfile);
}

/***********************************************************************/

void Terminator(void)
{
   int i0;
   SendCode(curnode->nodecode);
   SendCode(EOI);
   if (byte_index > 0) 
   {  codesize = 8 - byte_index;
      SendCode(0x0000);
   }
   block_index--;
   if (block_index > 0)
   {  block[0] = (char) block_index;
      SendBlock2File();
   }
   block_index = 0;
   codesize = 8;
   SendCode(0x0000);
   SendCode(0x003b);
   for (i0=0; i0<9; i0++) SendCode(0x001a); 
   block_index--;
   SendBlock2File();
}

/***********************************************************************/

void StartNewTable(void)
{
   int i1, i2;
   SendCode(CLEAR);
   for (i1=0; i1<CLEAR; i1++)
   {  curnode = rootnode->nextnode[i1];
      FreeCodeNodes(curnode); 
      for (i2=0; i2<CLEAR; i2++) 
         rootnode->nextnode[i1]->nextnode[i2] = NULL;
   }
   InitTable();
}

/***********************************************************************/

struct codenode *IMakeCodenode(void)
{
   int i1;
   struct codenode *nodeptr;
   if ((nodeptr = (struct codenode *) 
      malloc(sizeof(struct codenode))) EQ NULL)
   {  fprintf(stderr,"\nMemory exhausted");
      exit(1);
   }
   for (i1=0; i1<CLEAR; i1++) 
      nodeptr->nextnode[i1] = NULL;
   return nodeptr;
}

/***********************************************************************/

void FreeCodeNodes(struct codenode *nodeptr)
{
   int i1;
   struct codenode *thisnode;
   for (i1=0; i1<CLEAR; i1++)
   {  thisnode = nodeptr->nextnode[i1];
      if (thisnode NE NULL) 
      {  FreeCodeNodes(thisnode);
         free(thisnode);
      }
   }
}

/***********************************************************************/

void DataMaxMin(void) 
{
   double v, w, x, y, z;
   char ch;
   bottom = BIGNUM;
   top = -BIGNUM;
   if (z_only) 
   {  while ((ch = fgetc(dfile)) NE '\n');
      while ((ch = fgetc(dfile)) NE '\n');
   }
   switch (togif)
   {  case 1:
      {  while (fscanf(dfile,"%lf",&z) NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 2:
      {  while (fscanf(dfile,"%lf%lf%lf",&z,&x,&y) 
            NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 3:
      {  while (fscanf(dfile,"%lf%lf%lf",&y,&z,&x) 
            NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 4:
      {  while (fscanf(dfile,"%lf%lf%lf",&x,&y,&z) 
            NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 5:
      {  while (fscanf(dfile,"%lf%lf%lf%lf%lf",
            &x,&y,&z,&v,&w) NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 6:
      {  while (fscanf(dfile,"%lf%lf%lf%lf%lf",
            &w,&x,&y,&z,&v) NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      case 7:
      {  while (fscanf(dfile,"%lf%lf%lf%lf%lf",
            &v,&w,&x,&y,&z) NE EOF)
         {  if (z > top) top = z;
            if (z < bottom) bottom = z;
         }
         break;
      }
      default:;
   }
   rewind(dfile);
}

/***********************************************************************/

void MakeHeader(void)
{
   int i1, i2, i3, j0, j1, j2;
   char hold;
   FILE *cfile;
   if ((cfile = fopen("colormap.dat","r")) NE NULL)
   {  for (i1=0; i1<CLEAR; i1++)
      {  fscanf(cfile,"%d %d %d",&j0,&j1,&j2);
         color_map[i1][0] = j0;
         color_map[i1][1] = j1;
         color_map[i1][2] = j2;
      }
      fclose(cfile);
   }
   for (i1=0; i1<6; i1++) block[i1] = signature[i1];
   if (numcols > 255)
   {  block[6] = LowNum(numcols);
      block[7] = HighNum(numcols);
   }
   else
   {  hold = (char) numcols;
      block[6] = hold;
      block[7] = 0x00;
   }
   if (numrows > 255)
   {  block[8] = LowNum(numrows);
      block[9] = HighNum(numrows);
   }
   else
   {  hold = (char) numrows;
      block[8] = hold;
      block[9] = 0x00;
   }
   block[10] = LowNum(196); 
   block[11] = 0x00;
   block[12] = 0x00;
   for (i3=13, i1=0; i1<CLEAR; i1++) 
   {  for (i2=0; i2<3; i2++)
      {  hold = (char) color_map[i1][i2];
         block[i3] = hold;
         i3++;
      }
   }
   block[109] = 0x2c;
   block[110] = 0x00;
   block[111] = 0x00;
   block[112] = 0x00;
   block[113] = 0x00;
   block[114] = block[6];
   block[115] = block[7];
   block[116] = block[8];
   block[117] = block[9];
   block[118] = 0x00;
   block[119] = 0x05;
}

/***********************************************************************/

char LowNum(short anum)
{
   int i0;
   char abyte;
   short int bnum = 0x0001, dnum = 0x0000;
   for (i0=0; i0<8; i0++)
   {  if (anum & bnum) dnum |= bnum;
      bnum <<= 1;
   }
   abyte = (char) dnum;
   return abyte;
}

/***********************************************************************/

char HighNum(short anum)
{
   int i0;
   char abyte;
   short int bnum = 0x0100, cnum = 0x0001, 
             dnum = 0x0000;
   for (i0=0; i0<8; i0++)
   {  if (anum & bnum) dnum |= cnum;
      bnum <<= 1;
      cnum <<= 1;
   }
   abyte = (char) dnum;
   return abyte;
}

/***********************************************************************/

void WriteHeader(void) 
{
   char *giftag = ".gif";  /* changed from 'char giftag[] = ".gif"' 9/9/94 */
   int i0 = 0, i1 = 0;
   if (grd_file[0] EQ '.') i1++;
   while (grd_file[i1] NE '.' AND 
      grd_file[i1] NE '\0') 
   {  gif_file[i0] = grd_file[i1];
      i0++;
      i1++;
   }
   gif_file[i0] = '\0';
   strcat(gif_file, giftag);
   if ((gfile = fopen(gif_file,"w")) EQ NULL)
   {  fprintf(stderr,"\nCannot open *.gif file %s",
         gif_file);
      exit(1);
   } 
   block_index = 119;
   SendBlock2File();
   block_index = 1;
   byte_index = 0;
   thebyte = 0x00;
   SendCode(CLEAR);
}
