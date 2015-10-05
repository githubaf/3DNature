/*----------------------- nncrunch.c ----------------------------*/
#include "nnchead.h"

int ReadData(void)
{
   double temp[3], minx, maxx, miny, maxy;
   int i0, i1;

   bigtri[0][0] = bigtri[0][1] = bigtri[1][1] = 
      bigtri[2][0] = -1;
   bigtri[1][0] = bigtri[2][1] = 5;

   if ((filep = fopen(dat_file,"r")) EQ NULL)
   {  fprintf(stderr,"\nUnable to open data file %s\n",
         dat_file);
      return 0;
   }
   if (rootdat EQ NULL) 
   {  rootdat = IMakeDatum();
      rootsimp = IMakeSimp();
      roottemp = IMakeTemp();
      rootneig = IMakeNeig();
      rootdat->values[0] = rootdat->values[1] = 
         rootdat->values[2] = 0;
   }
   else 
   {  FreeVecti(jndx);
      FreeMatrixd(points);
      FreeMatrixd(joints);
   }
   curdat = rootdat;
   datcnt = 0;
   if (mxmn)
   {  maxxy[0][0] = maxxy[0][1] = maxxy[0][2] = 
         -(maxxy[1][0] = maxxy[1][1] = maxxy[1][2] = 
         BIGNUM);
   }
   minx = xstart - horilap;   maxx = xterm + horilap;
   miny = ystart - vertlap;   maxy = yterm + vertlap;
   if (densi)   
   {  while (fscanf(filep, indform, &temp[0], &temp[1]) 
         == 2)
      {
         if (mxmn)
         {  for (i1=0; i1<2; i1++)
            {  if (maxxy[0][i1] < temp[i1]) 
                  maxxy[0][i1] = temp[i1]; 
               if (maxxy[1][i1] > temp[i1]) 
                  maxxy[1][i1] = temp[i1]; 
            }
         }
         if (temp[0]>minx AND temp[0]<maxx AND 
            temp[1]>miny AND temp[1]<maxy)
         {  if (curdat->nextdat EQ NULL) 
               curdat->nextdat = IMakeDatum();
            curdat = curdat->nextdat;
            datcnt++;
            for (i1=0; i1<2; i1++) 
               curdat->values[i1] = temp[i1];
            curdat->values[2] = 1;
         }
      }
   }
   else      
   {  while (fscanf(filep, infform, &temp[0], &temp[1], 
         &temp[2]) NE EOF)
      {
         if (mxmn)
         {  for (i1=0; i1<3; i1++)
            {  if (maxxy[0][i1] < temp[i1]) 
                  maxxy[0][i1] = temp[i1]; 
               if (maxxy[1][i1] > temp[i1]) 
                  maxxy[1][i1] = temp[i1]; 
            }
         }
         if (temp[0]>minx AND temp[0]<maxx AND 
            temp[1]>miny AND temp[1]<maxy)
         {  if (curdat->nextdat EQ NULL) 
               curdat->nextdat = IMakeDatum();
            curdat = curdat->nextdat;
            datcnt++;
            for (i1=0; i1<3; i1++) 
               curdat->values[i1] = temp[i1];
         }
      }
   }
   fclose(filep);
   if (mxmn)
   {  Leader(indent);
      printf("Data file maximums %14.5e  %14.5e",
         maxxy[0][0],maxxy[0][1]);
      if (!densi) printf("  %14.5e",maxxy[0][2]);
      Leader(indent);
      printf("Data file minimums %14.5e  %14.5e",
         maxxy[1][0],maxxy[1][1]);
      if (!densi) printf("  %14.5e",maxxy[1][2]);
      Leader(indent);
      printf("Data file differ's %14.5e  %14.5e",
         maxxy[0][0]-maxxy[1][0],
         maxxy[0][1]-maxxy[1][1]);
      if (!densi) printf("  %14.5e",
         maxxy[0][2]-maxxy[1][2]);
      printf("\n");
   }
   if (datcnt>3)
   {  datcnt3 = datcnt + 3;
      jndx = IntVect(datcnt3);
      sumx = sumy = sumz = sumx2 = sumy2 = 
         sumxy = sumxz = sumyz = 0;
      maxxy[0][0] = maxxy[0][1] = maxxy[0][2] = 
         -(maxxy[1][0] = maxxy[1][1] = maxxy[1][2] = 
         BIGNUM);
      if (igrad) points = DoubleMatrix(datcnt+4, 6);
      else       points = DoubleMatrix(datcnt+4, 3);
      joints = DoubleMatrix(datcnt3, 2); 
      curdat = rootdat->nextdat;
      rootdat->nextdat = NULL;
      for (i0=0; i0<datcnt; i0++)
      {  sumx += points[i0][0] = 
            curdat->values[0] * magx;
         sumx2 += SQ(points[i0][0]);
         if (maxxy[0][0] < points[i0][0]) 
            maxxy[0][0] = points[i0][0];  
         if (maxxy[1][0] > points[i0][0]) 
            maxxy[1][0] = points[i0][0];  
         sumy += points[i0][1] = 
            curdat->values[1] * magy;
         sumy2 += SQ(points[i0][1]);
         sumxy += points[i0][0] * points[i0][1];
         if (maxxy[0][1] < points[i0][1]) 
            maxxy[0][1] = points[i0][1];  
         if (maxxy[1][1] > points[i0][1]) 
            maxxy[1][1] = points[i0][1];  
/********************************************
The statement     if (!densi)
which was the fifth line from the bottom on page 106
has been replaced by the following two lines
6 June 1994
***********************************************/
         if (densi) points[i0][2] = 1;
         else
         {  sumz += points[i0][2] = 
               curdat->values[2] * magz;
            sumxz += points[i0][0] * points[i0][2];
            sumyz += points[i0][1] * points[i0][2];
            if (maxxy[0][2] < points[i0][2]) 
               maxxy[0][2] = points[i0][2]; 
            if (maxxy[1][2] > points[i0][2]) 
               maxxy[1][2] = points[i0][2]; 
         }
         holddat = curdat;
         curdat = curdat->nextdat;
         free(holddat);
      }
      det = (datcnt * (sumx2 * sumy2 - sumxy * sumxy))
          - (sumx * (sumx * sumy2 - sumy * sumxy))
          + (sumy * (sumx * sumxy - sumy * sumx2));
      aaa = ((sumz * (sumx2 * sumy2 - sumxy * sumxy))
          - (sumxz * (sumx * sumy2 - sumy * sumxy))
          + (sumyz * (sumx * sumxy - sumy * sumx2))) / 
         det;
      bbb = 
         ((datcnt * (sumxz * sumy2 - sumyz * sumxy))
          - (sumz * (sumx * sumy2 - sumy * sumxy))
          + (sumy * (sumx * sumyz - sumy * sumxz))) / 
         det;
      ccc = 
         ((datcnt * (sumx2 * sumyz - sumxy * sumxz))
          - (sumx * (sumx * sumyz - sumy * sumxz))
          + (sumz * (sumx * sumxy - sumy * sumx2))) / 
         det;
      if (mxmn)
      {  Leader(indent);
         printf("Grid region maximums %14.5e %14.5e",
            maxxy[0][0] / magx,maxxy[0][1] / magy);
         if (!(densi)) 
            printf(" %14.5e",maxxy[0][2] / magz);
         Leader(indent);
         printf("Grid region minimums %14.5e %14.5e",
            maxxy[1][0] / magx,maxxy[1][1] / magy);
         if (!(densi)) 
            printf(" %14.5e",maxxy[1][2] / magz);
         Leader(indent);
         printf("Grid region differ's %14.5e %14.5e",
            (maxxy[0][0] - maxxy[1][0]) / magx,
            (maxxy[0][1] - maxxy[1][1]) / magy);
         if (!(densi)) printf(" %14.5e",
            (maxxy[0][2] - maxxy[1][2]) / magz);
         printf("\n");
      }
      if (maxxy[0][0] < maxx * magx) 
         maxxy[0][0] = maxx * magx; 
      if (maxxy[1][0] > minx * magx) 
         maxxy[1][0] = minx * magx; 
      if (maxxy[0][1] < maxy * magy) 
         maxxy[0][1] = maxy * magy; 
      if (maxxy[1][1] > miny * magy) 
         maxxy[1][1] = miny * magy; 
      for (i0=0; i0<3; i0++) 
         maxxy[0][i0] -= maxxy[1][i0];
      maxhoriz = maxxy[0][0]; 
      if (maxhoriz < maxxy[0][1]) 
         maxhoriz = maxxy[0][1];
      wbit = maxhoriz * EPSILON;
      if (maxxy[0][0] / maxxy[0][1] > 2 
         OR maxxy[0][1] / maxxy[0][0] > 2) 
         TooNarrow();
      if (!ichoro AND igrad)
      {  if (maxxy[0][2] / maxxy[0][0] > 60
            OR maxxy[0][2] / maxxy[0][1] > 60) 
            TooSteep();
         if (maxxy[0][2] / maxxy[0][0] < .017
            OR maxxy[0][2] / maxxy[0][1] < .017) 
            TooShallow();
      }
      for (i0=0; i0<3; i0++)
      {  points[datcnt+i0][0] = maxxy[1][0] + 
            bigtri[i0][0] * maxxy[0][0] * NRANGE;
         points[datcnt+i0][1] = maxxy[1][1] + 
            bigtri[i0][1] * maxxy[0][1] * NRANGE;
         if (densi) points[datcnt+i0][2] = 1;
         else points[datcnt+i0][2] =
            aaa + bbb * points[datcnt+i0][0] + 
            ccc * points[datcnt+i0][1];
      }
      rootdat = NULL;
   }
   else
   {  fprintf(stderr,"\nInsufficient data in gridded region to triangulate");
      fprintf(stderr,"\n       ... increase the size of the gridded region");
      return 0;
   }
   srand(367);     
   for (i0=0; i0<datcnt; i0++) 
      for (i1=0; i1<2; i1++)
         points[i0][i1] += wbit * 
            (0.5 - (double)rand() / MAXRAND);
   if (sdip OR igrad)
   {  piby2 = 2 * atan(1.0);
      pi = piby2 * 2;
      piby32 = 3 * piby2;
      rad2deg = 90 / piby2;
   }
   return 1;
}

/***********************************************************************/

void ChoroPleth(void)
{
   int i0, i1;
   double cmax[2][2];
   maxxy[0][2] = -BIGNUM;
   maxxy[1][2] = BIGNUM;
   sumz = sumxz = sumyz = 0;
   for (i0=0; i0<datcnt; i0++)
   {  FindNeigh(i0);
      cmax[0][0] = cmax[0][1] = -BIGNUM;
      cmax[1][0] = cmax[1][1] = BIGNUM;
      cursimp = rootsimp;
      for (i1=0; i1<numtri; i1++)
      {  cursimp = cursimp->nextsimp;
         joints[i1][0] = cursimp->cent[0];
         if (cmax[0][0] < joints[i1][0]) 
            cmax[0][0] = joints[i1][0];
         if (cmax[1][0] > joints[i1][0]) 
            cmax[1][0] = joints[i1][0];
         joints[i1][1] = cursimp->cent[1];
         if (cmax[0][1] < joints[i1][1]) 
            cmax[0][1] = joints[i1][1];
         if (cmax[1][1] > joints[i1][1]) 
            cmax[1][1] = joints[i1][1];
      }
      cmax[0][0] -= cmax[1][0];
      cmax[0][1] -= cmax[1][1];
      for (i1=0; i1<3; i1++)
      {  joints[datcnt+i1][0] = cmax[1][0] + 
            bigtri[i1][0] * cmax[0][0] * NRANGE;
         joints[datcnt+i1][1] = cmax[1][1] + 
            bigtri[i1][1] * cmax[0][1] * NRANGE;
      }
      neicnt = numtri;
      TriCentr();
      cursimp = rootsimp;
      for (asum=0, i1=0; i1<numtri; i1++)
      {  cursimp = cursimp->nextsimp;
         if (cursimp->vert[0] < datcnt) asum +=
            fabs((joints[cursimp->vert[1]][0] - 
               joints[cursimp->vert[0]][0])
              *  (joints[cursimp->vert[2]][1] - 
               joints[cursimp->vert[0]][1])
              -  (joints[cursimp->vert[2]][0] - 
               joints[cursimp->vert[0]][0])
              *  (joints[cursimp->vert[1]][1] - 
               joints[cursimp->vert[0]][1])) / 2;
      }
      if (asum > 0) points[i0][2] /= asum; /* conditional added 4/4/95 */
      else points[i0][2] = 0;
      sumz += points[i0][2];
      sumxz += points[i0][0] * points[i0][2];
      sumyz += points[i0][1] * points[i0][2];
      if (maxxy[0][2] < points[i0][2]) 
         maxxy[0][2] = points[i0][2]; 
      if (maxxy[1][2] > points[i0][2]) 
         maxxy[1][2] = points[i0][2]; 
   }
   maxxy[0][2] -= maxxy[1][2];
   if (igrad)
   {   if (maxxy[0][2] / maxhoriz > 60
         OR maxxy[0][2] / maxxy[0][1] > 60) 
         TooSteep();
      if (maxxy[0][2] / maxhoriz < .017
         OR maxxy[0][2] / maxxy[0][1] < .017) 
         TooShallow();
   }
   det = (datcnt * (sumx2 * sumy2 - sumxy * sumxy))
       - (sumx * (sumx * sumy2 - sumy * sumxy))
       + (sumy * (sumx * sumxy - sumy * sumx2));
   aaa = ((sumz * (sumx2 * sumy2 - sumxy * sumxy))
       - (sumxz * (sumx * sumy2 - sumy * sumxy))
       + (sumyz * (sumx * sumxy - sumy * sumx2))) / 
      det;
   bbb = ((datcnt * (sumxz * sumy2 - sumyz * sumxy))
       - (sumz * (sumx * sumy2 - sumy * sumxy))
       + (sumy * (sumx * sumyz - sumy * sumxz))) / 
      det;
   ccc = ((datcnt * (sumx2 * sumyz - sumxy * sumxz))
       - (sumx * (sumx * sumyz - sumy * sumxz))
       + (sumz * (sumx * sumxy - sumy * sumx2))) / 
      det;
   for (i1=0; i1<3; i1++)
   {  points[datcnt+i1][2] =
         aaa + bbb * points[datcnt+i1][0] + 
         ccc * points[datcnt+i1][1];
      if (igrad)
      {  points[datcnt+i1][3] = -bbb;
         points[datcnt+i1][4] = -ccc;
         points[datcnt+i1][5] = 1;
      }
   }
}

/***********************************************************************/

void Gradient(void)
{
   int i0, i1, i2, i3;
   double u2, wxd, wyd, wxde, wydn, xc, xe, xn;
   for (i0=0; i0<datcnt; i0++)
   {  FindNeigh(i0);
      if (!ext) 
      {  TriNeigh();
         wxd = points[i0][0];
         wyd = points[i0][1];
         FindProp(wxd,wyd);
         xc = Surface();
         wxde = wxd + wbit;
         FindProp(wxde,wyd);
         xe = Surface();
         wydn = wyd + wbit;
         FindProp(wxd,wydn);
         xn = Surface();
         points[i0][3] = (xc - xe) / wbit;
         points[i0][4] = (xc - xn) / wbit;
         asum /= pi; 
         points[i0][5] = 1 - sqrt(asum / 
            (asum + SQ(points[i0][2] - xc)));
      }
      else     
      {  points[i0][3] = points[i0][4] = 
            points[i0][5] = xx = 0;
         cursimp = rootsimp;
         for (i1=0; i1<numtri; i1++)
         {  cursimp = cursimp->nextsimp;
            for (i2=0; i2<2; i2++) 
               for (i3=0; i3<3; i3++)
                  work3[i2][i3] = 
                     points[cursimp->vert[0]][i3] - 
                     points[cursimp->vert[i2+1]][i3];
            work3[2][0] = work3[0][1] * work3[1][2] - 
               work3[1][1] * work3[0][2];
            work3[2][1] = work3[0][2] * work3[1][0] - 
               work3[1][2] * work3[0][0];
            work3[2][2] = work3[0][0] * work3[1][1] - 
               work3[1][0] * work3[0][1];
            u2 = 1;
            if (work3[2][2]<0) u2 = -1;
            xx += sqrt(SQ(work3[2][0]) + 
               SQ(work3[2][1]) + SQ(work3[2][2]));
            for (i2=0; i2<3; i2++) points[i0][i2+3] += 
               work3[2][i2] * u2;
         }
         xx = 1 - sqrt(SQ(points[i0][3]) + 
            SQ(points[i0][4]) + 
            SQ(points[i0][5])) / xx;
         points[i0][3] /= points[i0][5];
         points[i0][4] /= points[i0][5];
         points[i0][5] = xx; 
      }
   }
   for (i0=0; i0<3; i0++)
   {  points[datcnt+i0][3] = -bbb;
      points[datcnt+i0][4] = -ccc;
      points[datcnt+i0][5] = 1;
   }
}

/***********************************************************************/

void FindNeigh(int ipt)
{
   int i0, i1, i2, i3, j1, j2, j3, j4, j5;
   if (rootsimp->nextsimp EQ NULL) 
      rootsimp->nextsimp = IMakeSimp();
   cursimp = rootsimp->nextsimp;
   cursimp->vert[0] = datcnt;
   cursimp->vert[1] = datcnt + 1;
   cursimp->vert[2] = datcnt + 2;
   cursimp->cent[0] = cursimp->cent[1] = 0.5;
   cursimp->cent[2] = BIGNUM;
   numtri = 1;
   lasttemp = roottemp;
   for (i2=0; i2<3; i2++)
   {  j1 = 0;
      if (j1 EQ i2) j1++;
      j2 = j1 + 1;
      if (j2 EQ i2) j2++;
      if (lasttemp->nexttemp EQ NULL) 
         lasttemp->nexttemp = IMakeTemp();
      lasttemp = lasttemp->nexttemp;
      lasttemp->end[0] = cursimp->vert[j1];
      lasttemp->end[1] = cursimp->vert[j2];
   }
   curtemp = roottemp;
   for (i1=0; i1<3; i1++)
   {  curtemp = curtemp->nexttemp;
      for (i2=0; i2<2; i2++)
      {  work3[i2][0] = points[curtemp->end[i2]][0] - 
            points[ipt][0];
         work3[i2][1] = points[curtemp->end[i2]][1] - 
            points[ipt][1];
         work3[i2][2] = work3[i2][0] * 
            (points[curtemp->end[i2]][0] + 
            points[ipt][0]) / 2 + work3[i2][1] * 
            (points[curtemp->end[i2]][1] + 
            points[ipt][1]) / 2;
      }
      xx = work3[0][0] * work3[1][1] - 
         work3[1][0] * work3[0][1];
      cursimp->cent[0] = (work3[0][2] * work3[1][1] - 
         work3[1][2] * work3[0][1]) / xx;
      cursimp->cent[1] = (work3[0][0] * work3[1][2] - 
         work3[1][0] * work3[0][2]) / xx;
      cursimp->cent[2] = SQ(points[ipt][0] - 
         cursimp->cent[0]) + SQ(points[ipt][1] - 
         cursimp->cent[1]);
      cursimp->vert[0] = curtemp->end[0];
      cursimp->vert[1] = curtemp->end[1];
      cursimp->vert[2] = ipt;
      lastsimp = cursimp;
      if (cursimp->nextsimp EQ NULL) 
         cursimp->nextsimp = IMakeSimp();
      cursimp = cursimp->nextsimp; 
   }
   numtri += 2;
   for (i0=0; i0<datcnt; i0++)
   {  if (i0 NE ipt)
      {  j4 = 0;
         j3 = -1;
         lasttemp = roottemp;
         cursimp = rootsimp;
         for (i1=0; i1<numtri; i1++)
         {  prevsimp = cursimp;
            cursimp = cursimp->nextsimp;
            xx = cursimp->cent[2] - 
               SQ(points[i0][0] - cursimp->cent[0]);
            if (xx > 0)
            {  xx -= SQ(points[i0][1] - 
                  cursimp->cent[1]);
               if (xx > 0)
               { j4--;
                 for (i2=0; i2<3; i2++)
                 { j1 = 0; 
                   if (j1 EQ i2) j1++; 
                   j2 = j1 + 1;    
                   if (j2 EQ i2) j2++;
                   if (j3>1)
                   { j5 = j3;
                     curtemp = roottemp;
                     for (i3=0; i3<=j5; i3++)
                     { prevtemp = curtemp;
                       curtemp =
                          curtemp->nexttemp;
                       if (cursimp->vert[j1] EQ 
                          curtemp->end[0])
                       { if (cursimp->vert[j2] EQ 
                            curtemp->end[1])
                         { if (curtemp EQ lasttemp) 
                              lasttemp = prevtemp;
                            else
                           { prevtemp->nexttemp = 
                                curtemp->nexttemp;
                             curtemp->nexttemp = 
                                lasttemp->nexttemp;
                             lasttemp->nexttemp = 
                                curtemp;
                           }
                           j3--;
                           goto NextOne;
                         }
                       }
                     }
                   }
                   if (lasttemp->nexttemp EQ NULL) 
                      lasttemp->nexttemp = 
                      IMakeTemp();
                   lasttemp = lasttemp->nexttemp;
                   j3++;
                   lasttemp->end[0] = 
                      cursimp->vert[j1];
                   lasttemp->end[1] = 
                      cursimp->vert[j2];
NextOne:; }
                  if (cursimp EQ lastsimp) 
                     lastsimp = prevsimp;
                  else
                  {  prevsimp->nextsimp = 
                        cursimp->nextsimp;
                     cursimp->nextsimp = 
                        lastsimp->nextsimp;
                     lastsimp->nextsimp = cursimp;
                     cursimp = prevsimp;
                  }
               }
            }
         }
         if (j3 > -1)
         {  curtemp = roottemp;
            cursimp = lastsimp->nextsimp;
            for (i1=0; i1<=j3; i1++)
            {  curtemp = curtemp->nexttemp;
               if (curtemp->end[0] EQ ipt OR 
                  curtemp->end[1] EQ ipt)
               {  for (i2=0; i2<2; i2++)
                  {  work3[i2][0] = 
                        points[curtemp->end[i2]][0] - 
                        points[i0][0];
                     work3[i2][1] = 
                        points[curtemp->end[i2]][1] - 
                        points[i0][1];
                     work3[i2][2] = work3[i2][0] * 
                        (points[curtemp->end[i2]][0] + 
                        points[i0][0]) / 2 + 
                        work3[i2][1] *
                        (points[curtemp->end[i2]][1] + 
                        points[i0][1]) / 2;
                  }
                  xx = work3[0][0] * work3[1][1] - 
                     work3[1][0] * work3[0][1];
                  cursimp->cent[0] = (work3[0][2] * 
                     work3[1][1] - work3[1][2] * 
                     work3[0][1]) / xx;
                  cursimp->cent[1] = (work3[0][0] * 
                     work3[1][2] - work3[1][0] * 
                     work3[0][2]) / xx;
                  cursimp->cent[2] = 
                     SQ(points[i0][0] - 
                     cursimp->cent[0]) +
                     SQ(points[i0][1] - 
                     cursimp->cent[1]);
                  cursimp->vert[0] = curtemp->end[0];
                  cursimp->vert[1] = curtemp->end[1];
                  cursimp->vert[2] = i0;
                  lastsimp = cursimp;
                  if (cursimp->nextsimp EQ NULL) 
                     cursimp->nextsimp = IMakeSimp();
                  cursimp = cursimp->nextsimp; 
                  j4++;
               }
            }
            numtri += j4;
         }
      }
   }
   for (i0=0; i0<datcnt; i0++) jndx[i0] = 0;
   cursimp = rootsimp;
   for (ext=0, i1=0; i1<numtri; i1++)
   {  cursimp = cursimp->nextsimp;
      for (i2=0; i2<3; i2++) 
      {  if (cursimp->vert[i2] < datcnt)
         {  if (cursimp->vert[i2] NE ipt) 
               jndx[cursimp->vert[i2]] = 1;
         }
         else ext = 1; 
      }
   }
}

/***********************************************************************/

void TriCentr(void)
{
   int i0, i1, i2, i3, j1, j2, j3, j4, j5;
   if (rootsimp->nextsimp EQ NULL) 
      rootsimp->nextsimp = IMakeSimp();
   lastsimp = cursimp = rootsimp->nextsimp;
   cursimp->vert[0] = datcnt;
   cursimp->vert[1] = datcnt + 1;
   cursimp->vert[2] = datcnt + 2;
   cursimp->cent[0] = cursimp->cent[1] = 0.5;
   cursimp->cent[2] = BIGNUM;
   numtri = 1;
   for (i0=0; i0<neicnt; i0++)
   {  j3 = -1;
      j4 = 0;
      lasttemp = roottemp;
      cursimp = rootsimp;
      for (i1=0; i1<numtri; i1++)
      {  prevsimp = cursimp;
         cursimp = cursimp->nextsimp;
         xx = cursimp->cent[2] - SQ(joints[i0][0] - 
            cursimp->cent[0]);
         if (xx > 0)
         {  xx -= SQ(joints[i0][1] - 
               cursimp->cent[1]);
            if (xx > 0)
            {  j4--;
               for (i2=0; i2<3; i2++)
               {  j1 = 0;
                  if (j1 EQ i2) j1++;
                  j2 = j1 + 1;
                  if (j2 EQ i2) j2++;
                  if (j3>1)
                  {  j5 = j3;
                     curtemp = roottemp;
                     for (i3=0; i3<=j5; i3++)
                     {  prevtemp = curtemp;
                        curtemp = curtemp->nexttemp;
                        if (cursimp->vert[j1] EQ 
                           curtemp->end[0])
                        {  if (cursimp->vert[j2] EQ 
                              curtemp->end[1])
                           {  if (curtemp EQ lasttemp) 
                                 lasttemp = prevtemp;
                              else
                              {  prevtemp->nexttemp = 
                                   curtemp->nexttemp;
                                 curtemp->nexttemp = 
                                   lasttemp->nexttemp;
                                 lasttemp->nexttemp = 
                                   curtemp;
                              }
                              j3--;
                              goto NextOne;
                           }
                        }
                     }
                  }
                  if (lasttemp->nexttemp EQ NULL) 
                     lasttemp->nexttemp = IMakeTemp();
                  lasttemp = lasttemp->nexttemp;
                  j3++;
                  lasttemp->end[0] = 
                     cursimp->vert[j1];
                  lasttemp->end[1] = 
                     cursimp->vert[j2];
NextOne:;      }
               if (cursimp EQ lastsimp) 
                  lastsimp = prevsimp;
               else
               {  prevsimp->nextsimp = 
                     cursimp->nextsimp;
                  cursimp->nextsimp = 
                     lastsimp->nextsimp;
                  lastsimp->nextsimp = cursimp;
                  cursimp = prevsimp;
               }
            }
         }
      }
      curtemp = roottemp;
      cursimp = lastsimp->nextsimp;
      for (i1=0; i1<=j3; i1++)
      {  curtemp = curtemp->nexttemp;
         for (i2=0; i2<2; i2++)
         {  work3[i2][0] = 
               joints[curtemp->end[i2]][0] - 
               joints[i0][0];
            work3[i2][1] = 
               joints[curtemp->end[i2]][1] - 
               joints[i0][1];
            work3[i2][2] = work3[i2][0] * 
               (joints[curtemp->end[i2]][0] + 
               joints[i0][0]) / 2 + 
               work3[i2][1] * 
               (joints[curtemp->end[i2]][1] + 
               joints[i0][1]) / 2;
         }
         xx = work3[0][0] * work3[1][1] - 
            work3[1][0] * work3[0][1];
         cursimp->cent[0] = 
            (work3[0][2] * work3[1][1] - 
            work3[1][2] * work3[0][1]) / xx;
         cursimp->cent[1] = 
            (work3[0][0] * work3[1][2] - 
            work3[1][0] * work3[0][2]) / xx;
         cursimp->cent[2] = 
            SQ(joints[i0][0] - cursimp->cent[0]) +
            SQ(joints[i0][1] - cursimp->cent[1]);
         cursimp->vert[0] = curtemp->end[0];
         cursimp->vert[1] = curtemp->end[1];
         cursimp->vert[2] = i0;
         lastsimp = cursimp;
         if (cursimp->nextsimp EQ NULL) 
            cursimp->nextsimp = IMakeSimp();
         cursimp = cursimp->nextsimp;   
         j4++;
      }
      numtri += j4;
   }
}

/***********************************************************************/

void TriNeigh(void)
{
   int i0, i1, i2, i3, j1, j2, j3, j4, j5;
   if (rootsimp->nextsimp EQ NULL) 
      rootsimp->nextsimp = IMakeSimp();
   lastsimp = cursimp = rootsimp->nextsimp;
   cursimp->vert[0] = datcnt;
   cursimp->vert[1] = datcnt + 1;
   cursimp->vert[2] = datcnt + 2;
   cursimp->cent[0] = cursimp->cent[1] = 0.5;
   cursimp->cent[2] = BIGNUM;
   numtri = 1;
   for (i0=0; i0<datcnt; i0++)
   {  if (jndx[i0])
      {  j3 = -1;
         lasttemp = roottemp;
         cursimp = rootsimp;
         for (i1=0; i1<numtri; i1++)
         {  prevsimp = cursimp;
            cursimp = cursimp->nextsimp;
            xx = cursimp->cent[2] - 
               SQ(points[i0][0] - cursimp->cent[0]);
            if (xx > 0)
            {  xx -= SQ(points[i0][1] - 
                  cursimp->cent[1]);
               if (xx > 0)
               {  for (i2=0; i2<3; i2++)
                  {  j1 = 0;
                     if (j1 EQ i2) j1++;
                     j2 = j1 + 1;
                     if (j2 EQ i2) j2++;
                     if (j3>1)
                     {  j5 = j3;
                        curtemp = roottemp;
                        for (i3=0; i3<=j5; i3++)
                        { prevtemp = curtemp;
                          curtemp = 
                             curtemp->nexttemp;
                          if (cursimp->vert[j1] EQ 
                             curtemp->end[0])
                          { if (cursimp->vert[j2] EQ 
                               curtemp->end[1])
                            { if (curtemp EQ lasttemp) 
                                 lasttemp = prevtemp;
                              else
                              { prevtemp->nexttemp = 
                                  curtemp->nexttemp;
                                curtemp->nexttemp = 
                                  lasttemp->nexttemp;
                                lasttemp->nexttemp = 
                                  curtemp;
                               }
                               j3--;
                               goto NextOne;
                             }
                           }
                        }
                     }
                     if (lasttemp->nexttemp EQ NULL)
                     lasttemp->nexttemp = IMakeTemp();
                     lasttemp = lasttemp->nexttemp;
                     j3++;
                     lasttemp->end[0] = 
                        cursimp->vert[j1];
                     lasttemp->end[1] = 
                        cursimp->vert[j2];
NextOne:; }
                  if (cursimp EQ lastsimp) 
                     lastsimp = prevsimp;
                  else
                  {  prevsimp->nextsimp = 
                        cursimp->nextsimp;
                     cursimp->nextsimp = 
                        lastsimp->nextsimp;
                     lastsimp->nextsimp = cursimp;
                     cursimp = prevsimp;
                  }
               }
            }
         }
         curtemp = roottemp;
         cursimp = lastsimp->nextsimp;
         for (i1=0; i1<=j3; i1++)
         {  curtemp = curtemp->nexttemp;
            for (i2=0; i2<2; i2++)
            {  work3[i2][0] = 
                  points[curtemp->end[i2]][0] - 
                  points[i0][0];
               work3[i2][1] = 
                  points[curtemp->end[i2]][1] - 
                  points[i0][1];
               work3[i2][2] = work3[i2][0] * 
                  (points[curtemp->end[i2]][0] + 
                  points[i0][0]) / 2 + work3[i2][1] * 
                  (points[curtemp->end[i2]][1] + 
                  points[i0][1]) / 2;
            }
            xx = work3[0][0] * work3[1][1] - 
               work3[1][0] * work3[0][1];
            cursimp->cent[0] = 
               (work3[0][2] * work3[1][1] - 
               work3[1][2] * work3[0][1]) / xx;
            cursimp->cent[1] = 
               (work3[0][0] * work3[1][2] - 
               work3[1][0] * work3[0][2]) / xx;
            cursimp->cent[2] = SQ(points[i0][0] - 
               cursimp->cent[0]) + SQ(points[i0][1] - 
               cursimp->cent[1]);
            cursimp->vert[0] = curtemp->end[0];
            cursimp->vert[1] = curtemp->end[1];
            cursimp->vert[2] = i0;
            lastsimp = cursimp;
            if (cursimp->nextsimp EQ NULL) 
               cursimp->nextsimp = IMakeSimp();
            cursimp = cursimp->nextsimp;
         }
         numtri += 2;
      }
   }
   cursimp = rootsimp;
   for (asum=0, i0=0; i0<numtri; i0++) 
   {  cursimp = cursimp->nextsimp;
      for (i1=0; i1<2; i1++)
      {  work3[0][i1] = points[cursimp->vert[1]][i1] - 
            points[cursimp->vert[0]][i1];
         work3[1][i1] = points[cursimp->vert[2]][i1] - 
            points[cursimp->vert[0]][i1];
      }
      xx = work3[0][0] * work3[1][1] - 
         work3[0][1] * work3[1][0];
      if (xx < 0)
      {  j4 = cursimp->vert[2];
         cursimp->vert[2] = cursimp->vert[1];
         cursimp->vert[1] = j4;
         if (cursimp->vert[0] < datcnt) 
            asum -= xx / 2;
      }
      else if (cursimp->vert[0] < datcnt) 
         asum += xx / 2;
   }
}

/***********************************************************************/

int MakeGrid(void)
{
   double wxd, wyd, wxde, wydn, surf, surfe, surfn,
          x_increm, y_increm, x_increm2, 
          aspect, slope;
   int i0, j7, j8, tx_nodes;

   if ((filep = fopen(grd_file,"w")) EQ NULL)
      fprintf(stderr,"\nUnable to open grid file %s",
         grd_file);
   else
   {  if (z_only)
      {  fprintf(filep,"# %3d %3d %3d %3d %3d %3d\n",
            x_nodes,y_nodes,tgrid,updir,sdip,rads);
         fprintf(filep,"# %13.4f %13.4f %13.4f %13.4f\n",
            xstart,ystart,xterm,yterm);
      }
      if (optim) 
      {  for (i0=0; i0<datcnt; i0++) jndx[i0] = 1;
         TriNeigh();
      }
      x_increm = ((.00000001 + xterm - xstart) * magx) /
         (float)(x_nodes-1);   /* add scale factor 24/4/95 */
      tx_nodes = x_nodes;
      if (tgrid)
      {  x_increm2 = ((.00000001 + xterm - xstart) * magx) /
         (float)(x_nodes-1);   /* add scale factor 24/4/95 */
         x_increm = x_increm2 * 2;
         tx_nodes = x_nodes / 2;
      }
      y_increm = ((.00000001 + yterm - ystart) * magy) /
         (float)(y_nodes-1);   /* add scale factor 24/4/95 */
      wyd = ystart * magy - y_increm;   /* add scale factor 24/4/95 */
      if (arriba<0) wyd = yterm * magy + y_increm;   /* add scale factor 24/4/95 */
      for (j8=0; j8<y_nodes; j8++) 
      {  wyd += y_increm * arriba;
         points[datcnt3][1] = wyd;
         wxd = xstart * magx - x_increm;   /* add scale factor 24/4/95 */
         if (gnup) fprintf(filep, "\n");
         if (tgrid AND j8 % 2) wxd += x_increm2;
         for (j7=0; j7<tx_nodes; j7++) 
         {  wxd += x_increm;
            points[datcnt3][0] = wxd;
            if (!optim)
            {  FindNeigh(datcnt3);
               TriNeigh();
            }
            FindProp(wxd,wyd);
            if (!extrap AND !goodflag) surf = nuldat;
            else
            {  surf = Surface();
               if (igrad>0) surf = Meld(surf,wxd,wyd);
               if (non_neg) if (surf < 0) surf = 0;
            }
            if (sdip)
            {  wxde = wxd + wbit;
               FindProp(wxde,wyd);
               surfe = Surface();
               if (igrad>0) surfe = 
                  Meld(surfe,wxde,wyd);
               if (non_neg) if (surfe < 0) surfe = 0;
               wydn = wyd + wbit;
               FindProp(wxd,wydn);
               surfn = Surface();
               if (igrad>0) surfn = 
                  Meld(surfn,wxd,wydn);
               if (non_neg) if (surfn < 0) surfn = 0;
               surfe = (surf - surfe) / wbit;
               surfn = (surf - surfn) / wbit;
               if (surfe > 0)
               {  if (surfn > 0) aspect = 
                     piby2 - atan(surfn / surfe);
                  else aspect = piby2 + 
                     atan(surfn / surfe) * -1;
               }
               else
               {  if (surfe < 0)
                  {  if (surfn > 0) aspect = piby32 + 
                        atan(surfn / surfe) * -1;
                     else aspect = piby32 - 
                        atan(surfn / surfe);
                  }
                  else
                  {  if (surfn > 0) aspect = 0;
                     else aspect = pi;
                  }
               }
               slope = atan(sqrt(SQ(surfe) + 
                  SQ(surfn)));
               if (!rads)
               {  aspect *= rad2deg;
                  slope *= rad2deg;
               }
               if (z_only) 
               {  if (!imag) fprintf(filep,out1aform,
                     surf,aspect,slope);
                  else 
                  {  if (densi) fprintf(filep,
                        out1aform,surf,
                        aspect,slope);
                     else  fprintf(filep,out1aform,
                        surf/magz,aspect,slope);
                  }
               }
               else
               {  if (!imag) fprintf(filep,
                     out3aform,wxd,wyd,
                     surf,aspect,slope);
                  else 
                  {  if (densi) fprintf(filep,
                        out3aform,wxd/magx,
                        wyd/magy,surf,
                        aspect,slope);
                     else  fprintf(filep,out3aform,
                        wxd/magx,wyd/magy,
                        surf/magz,aspect,slope);
                  }
               }
            }
            else
            {  if (z_only) 
               {  if (!imag) fprintf(filep,out1form,
                     surf);
                  else 
                  {  if (densi) fprintf(filep,
                        out1form,surf,
                        aspect,slope);
                     else  fprintf(filep,out1form,
                         surf/magz,aspect,slope);
                  }
               }
               else
               {  if (!imag) fprintf(filep,
                     out3form,wxd,wyd,surf);
                  else 
                  {  if (densi) 
                        fprintf(filep,out3form,
                           wxd/magx,wyd/magy,surf);
                     else  fprintf(filep,out3form,
                        wxd/magx,wyd/magy,surf/magz);
                  }
               }
            }
            fprintf(filep,"\n");
         }
      }
   }
   fclose(filep);
   return 1;
}

/***********************************************************************/

void FindProp(double wxd, double wyd)
{
   int i2, i3, i4, pos_count, inside;
   double xx, work3[3][3], work4[3][2];
   lastneig = rootneig;
   goodflag = 0;
   numnei = -1;
   cursimp = rootsimp;
   for (i2=0; i2<numtri; i2++)
   {  cursimp = cursimp->nextsimp;
      xx = cursimp->cent[2] - 
         SQ(wxd - cursimp->cent[0]);
      if (xx > 0)
      {  xx -= SQ(wyd - cursimp->cent[1]);
         if (xx > 0)
         {  inside = 0;
            if (cursimp->vert[0] < datcnt) inside = 1;
            for (i3=0; i3<3; i3++)
            {  for (i4=0; i4<2; i4++)
               {  work3[i4][0] = 
                     points[cursimp->
                     vert[scor[i3][i4]]][0] - wxd;
                  work3[i4][1] = 
                     points[cursimp->
                     vert[scor[i3][i4]]][1] - wyd;
                  work3[i4][2] = work3[i4][0] *
                     (points[cursimp->
                     vert[scor[i3][i4]]][0] + 
                     wxd) / 2 + work3[i4][1] *
                     (points[cursimp->
                     vert[scor[i3][i4]]][1] + 
                     wyd) / 2;
               }
               xx =  work3[0][0] * work3[1][1] - 
                  work3[1][0] * work3[0][1];
               work4[i3][0] = (work3[0][2] * 
                  work3[1][1] - work3[1][2] * 
                  work3[0][1]) / xx;
               work4[i3][1] = (work3[0][0] * 
                  work3[1][2] - work3[1][0] * 
                  work3[0][2]) / xx;
            }
            pos_count = 0;
            for (i3=0; i3<3; i3++)
            {  work3[2][i3] = 
                  ((work4[scor[i3][0]][0] - 
                  cursimp->cent[0]) *
                  (work4[scor[i3][1]][1] - 
                  cursimp->cent[1]) -
                  (work4[scor[i3][1]][0] - 
                  cursimp->cent[0]) *
                  (work4[scor[i3][0]][1] - 
                  cursimp->cent[1])) / 2;
               if (work3[2][i3]>0) pos_count++;
            }
            if (pos_count>2 AND inside) goodflag = 1;
            for (i3=0; i3<3; i3++)
            {  if (numnei>1)
               {  curneig = rootneig;
                  for (i4=0; i4<=numnei; i4++)
                  {  curneig = curneig->nextneig;
                     if (cursimp->vert[i3] EQ 
                        curneig->neinum)
                     {  curneig->narea += 
                           work3[2][i3];
                        goto GOTEM;
                     }
                  }
               }
               if (lastneig->nextneig EQ NULL) 
                  lastneig->nextneig = IMakeNeig();
               lastneig = lastneig->nextneig;
               numnei++;
               lastneig->neinum = cursimp->vert[i3];
               lastneig->narea = work3[2][i3];
GOTEM:;       }   
         }
      }
   }
}

/***********************************************************************/

double Surface(void)
{
   int i0;
   double xx, asurf;
   curneig = rootneig;
   for (xx=0, i0=0; i0<=numnei; i0++) 
   {  curneig = curneig->nextneig;
      xx += curneig->narea;
   }
   curneig = rootneig;
   for (asurf=0, i0=0; i0<=numnei; i0++)
   {  curneig = curneig->nextneig;
      curneig->narea /= xx;
      asurf += curneig->narea * 
         points[curneig->neinum][2];
   }
   return asurf;
}

/***********************************************************************/

double Meld(double asurf, double wxd, double wyd)
{
   int i0;
   double rS, rT, rB, bD, bB, hP;
   curneig = rootneig;
   for (i0=0; i0<=numnei; i0++)
   {  curneig = curneig->nextneig;
      curneig->coord = 0;
      if (curneig->narea>0.00001 AND 
         curneig->narea < 2)
      {  if (fabs(points[curneig->neinum][5]) > 
            0.00001)
         {  rS = fabs(points[curneig->neinum][5]) + 
               bI;
            rT = rS * bJ;
            rB = 1 / rT;
            bD = pow(curneig->narea, rT);
            bB = bD * 2;
            if (bD>0.5) bB = (1 - bD) * 2;
            bB = pow(bB, rS) / 2;
            if (bD>0.5) bB = 1 - bB;
            hP = pow(bB, rB);
            curneig->coord = 
               ((points[curneig->neinum][3] *
               points[curneig->neinum][0] + 
               points[curneig->neinum][4] *
               points[curneig->neinum][1] + 
               points[curneig->neinum][2] -
               points[curneig->neinum][3] * 
               wxd -
               points[curneig->neinum][4] * 
               wyd) - asurf) * hP;
         }
      }
   }
   curneig = rootneig;
   for (i0=0; i0<=numnei; i0++) 
   {  curneig = curneig->nextneig;
      asurf += curneig->coord;
   }
   return asurf; 
}

/***********************************************************************/

void TooSteep(void)
{
   Leader(indent);
   fprintf(stderr,"N.B. The ratio of vertical to horizontal scales is too large for");
   Leader(indent);
   fprintf(stderr,"gradient estimation.  Rescale the data if gradients are required.\n");
   igrad = 0;
}

/***********************************************************************/

void TooShallow(void)
{
   Leader(indent);
   fprintf(stderr,"N.B. The ratio of vertical to horizontal scales is too small for");
   Leader(indent);
   fprintf(stderr,"gradient estimation.  Rescale the data if gradients are required.\n");
   igrad = 0;
}

/***********************************************************************/

void TooNarrow(void)
{
   Leader(indent);
   fprintf(stderr,"W A R N I N G The ratio of width to length of this gridded region");
   Leader(indent);
   fprintf(stderr,"may be too extreme for good interpolation.  Changing the block");
   Leader(indent);
   fprintf(stderr,"proportions, or rescaling the x or y coordinate may be indicated.\n");
   igrad = 0;
}

/***********************************************************************/

int *IntVect(int ncols)
{
   int *vectptr;
   if ((vectptr = (int *) 
      malloc(ncols * sizeof(int))) EQ NULL)
   {  fprintf(stderr,
         "\nUnable to allocate storage for ivector");
      exit(-1);
   }
   return vectptr;
}

/***********************************************************************/

void FreeVecti(int *vectptr)
{
   free(vectptr);
}

/***********************************************************************/

double *DoubleVect(int ncols)
{
   double *vectptr;
   if ((vectptr = (double *) 
      malloc(ncols * sizeof(double))) EQ NULL)
   {  fprintf(stderr,
         "\nUnable to allocate storage for dvector");
      exit(-1);
   }
   return vectptr;
}

/***********************************************************************/

void FreeVectd(double *vectptr)
{
   free(vectptr);
}

/***********************************************************************/

int **IntMatrix(int nrows, int ncols)
{
   int i0;
   int **matptr;
   if (nrows<2) nrows = 2;
   if (ncols<2) ncols = 2;
   if ((matptr = (int **) 
      malloc(nrows * sizeof(int *))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for **imatrix");
      exit(-1);
   }
   if ((matptr[0] = (int *) 
      malloc(nrows * ncols * sizeof(int))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for imatrix[]");
      exit(-1);
   }
   for (i0=1; i0<nrows; i0++) 
      matptr[i0] = matptr[0] + i0 * ncols;
   return matptr;
}

/***********************************************************************/

void FreeMatrixi(int **matptr)
{
   free(matptr[0]);        /* added 1/1/95 */
   free(matptr);
}

/***********************************************************************/

float **FloatMatrix(int nrows, int ncols)
{
   int i0;
   float **matptr;
   if (nrows<2) nrows = 2;
   if (ncols<2) ncols = 2;
   if ((matptr = (float **) 
      malloc(nrows * sizeof(float *))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for **fmatrix");
      exit(-1);
   }
   if ((matptr[0] = (float *) 
      malloc(nrows * ncols * sizeof(float))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for fmatrix[]");
      exit(-1);
   }
   for (i0=1; i0<nrows; i0++) 
      matptr[i0] = matptr[0] + i0 * ncols;
   return matptr;
}

/***********************************************************************/

void FreeMatrixf(float **matptr)
{
   free(matptr[0]);        /* added 1/1/95 */
   free(matptr);
}

/***********************************************************************/

double **DoubleMatrix(int nrows, int ncols)
{
   int i0;
   double **matptr;
   if (nrows<2) nrows = 2;
   if (ncols<2) ncols = 2;
   if ((matptr = (double **) 
      malloc(nrows * sizeof(double *))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for **dmatrix");
      exit(-1);
   }
   if ((matptr[0] = (double *) 
      malloc(nrows * ncols * sizeof(double))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for dmatrix[]");
      exit(-1);
   }
   for (i0=1; i0<nrows; i0++) 
      matptr[i0] = matptr[0] + i0 * ncols;
   return matptr;
}

/***********************************************************************/

void FreeMatrixd(double **matptr)
{
   free(matptr[0]);        /* added 1/1/95 */
   free(matptr);
}

/***********************************************************************/

struct datum *IMakeDatum(void)
{
   struct datum *datptr;
   if ((datptr = (struct datum *) 
      malloc(sizeof(struct datum))) EQ NULL)
   {  fprintf(stderr,
         "\nUnable to allocate storage for raw data");
      return NULL;
   }
   datptr->nextdat = NULL;
   return datptr;
}

/***********************************************************************/

struct simp *IMakeSimp(void)
{
   struct simp *simpptr;
   if ((simpptr = (struct simp *) 
      malloc(sizeof(struct simp))) EQ NULL)
   {  fprintf(stderr,"\nUnable to allocate storage for a simplex");
      return NULL;
   }
   simpptr->nextsimp = NULL;
   return simpptr;
}

/***********************************************************************/

struct temp *IMakeTemp(void)
{
   struct temp *tempptr;
   if ((tempptr = (struct temp *) 
      malloc(sizeof(struct temp))) EQ NULL)
   {  fprintf(stderr,
         "\nUnable to allocate storage for temp");
      return NULL;
   }
   tempptr->nexttemp = NULL;
   return tempptr;
}

/***********************************************************************/

struct neig *IMakeNeig(void)
{
   struct neig *neigptr;
   if ((neigptr = (struct neig *) 
      malloc(sizeof(struct neig))) EQ NULL)
   {  fprintf(stderr,
         "\nUnable to allocate storage for neig");
      return NULL;
   }
   neigptr->nextneig = NULL;
   return neigptr;
}
