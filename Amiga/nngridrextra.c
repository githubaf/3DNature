/* Functions removed from nngridr code
*/

#ifdef GJHGFJDGf
//
void ShowState(void)
{
   int i0, i1;
   InitScrn(); 
   Leader(indent);
   printf("Execution parameters are -");
   Leader(indent);
   printf("............................. Files ..............................");
   Leader(indent);
   printf("Input data set: %s",dat_file);
   i0 = strlen(dat_file) + strlen(grd_file);
   Spacer(33 - i0);
   printf("Output grid set: %s",grd_file);
   Leader(indent);
   if (densi)
   {  printf("Input format: %s",indform);
      i0 = strlen(indform);
   }
   else
   {  printf("Input format: %s",infform);
      i0 = strlen(infform);
   }
   magcnt = 0;
   if (mxmn)
   {  Spacer(20 - i0);
      printf("Print data maximums and minimums");
      magcnt++;
   }
   if (imag)
   {  if (ixmag)
      {  i1 = GetSize(magx);
         i0 += i1;
         if (magcnt EQ 0) Spacer(25 - i0);
         else Leader(indent);
         printf("Scale east-west data by %6.4f",magx);
         magcnt++;
      }
      if (iymag)
      {  i1 = GetSize(magy);
         i0 += i1;
         if (magcnt EQ 0) Spacer(23 - i0);
         if (magcnt EQ 1) Leader(indent);
         if (magcnt EQ 2) Spacer(2);
         printf("Scale north-south data by %6.4f",magy);
         magcnt++;
      }
      if (izmag)
      {  i1 = GetSize(magz);
         i0 += i1;
         switch (magcnt)
         {  case 0:
            {  Spacer(26 - i0);
               break;
            }
            case 1:
            {  Leader(indent);
               break;
            }
            case 2:
            {  Spacer(2);
               break;
            }
            case 3:
            {  Leader(indent);
               break;
            }
            default:;
         }
         printf("Scale vertical data by %6.4f",magz);
         magcnt++;
      }
   }
   if (atime)
   {  switch (magcnt)
      {  case 0:
         {  Spacer(31 - i0);
            break;
         }
         case 1:
         {  Leader(indent);
            break;
         }
         case 2:
         {  Spacer(13);
            break;
         }
         case 3:
         {  Leader(indent);
            break;
         }
         case 4:
         {  Spacer(13);
            break;
         }
         default:;
      }
      printf("Print execution times");
   }
   Leader(indent);
   printf("..................... Interpolation Controls .....................");
   Leader(indent);
   printf("%s interpolation",intstr);
   i0 = strlen(intstr);
   if (igrad)
   {  Spacer(27 - i0);
      printf("Using estimated gradients");
      Leader(indent);
      printf("Tautness parameter #1 %5.2f",bI);
      Spacer(12);
      printf("Tautness parameter #2 %5.2f",bJ);
      Leader(indent);                /* these four lines added 8 July 94 */
      if (non_neg)
         printf("Negative interpolation not allowed");
      else printf("Negative interpolation is allowed");
   }
   else
   {  i0 = strlen(intstr);
      Spacer(33 - i0);
      printf("Don't use gradients");
   }
   Leader(indent);
   if (extrap) printf("Extrapolation allowed");
   else 
   {  printf("No extrapolation");
      Spacer(30);
      printf("Null value is %6.1f",nuldat);
   }
   if (sdip)
   {  Leader(indent);
      printf("Calculate aspect and slope");
      Spacer(21);
      if (rads) printf("%s",radians);
      else printf("%s",degrees);
   }
   Leader(indent);
   printf("Optimized for");
   if (optim) printf(" speed");
   else printf(" memory");
   Leader(indent);
   printf("................... Gridded Region Parameters ....................");
   Leader(indent);
   printf("Nodes along x: %6d",x_nodes);
   Spacer(24);
   printf("Nodes along y: %6d",y_nodes);

   Leader(indent);
   printf("Least x: ");
   i1 = GetSize(xstart);
   i0 = 0;
   if (i1 > 4)
   {  i0 = i1 - 4; /* offset for y */
      if (i1 > 5) i1 = 5;
   }
   Spacer(7 - i1);
   printf("%8.4f",xstart);
   i1 = GetSize(ystart);
   if (i1 > 5) i1 = 5;
   Spacer(24 - i0);
   printf("Least y: ");
   Spacer(7 - i1);
   printf("%8.4f",ystart);

   Leader(indent);
   printf("x spacing: %10.4f",xspace);
   Spacer(24);
   printf("y spacing: %10.4f",yspace);

   Leader(indent);
   printf("Greatest x: ");
   i1 = GetSize(xterm);
   i0 = 0;
   if (i1 > 3) i0 = i1 - 3; /* offset for y */
   printf("%9.4f",xterm);
   Spacer(24 - i0);
   printf("Greatest y: ");
   printf("%9.4f",yterm);
   Leader(indent);
   printf("x overlaps: %9.4f",horilap);
   Spacer(24);
   printf("y overlaps: %9.4f",vertlap);
   Leader(indent);
   printf("....................... Output Parameters ........................");
   Leader(indent);
   printf("Output: %s",z_only ? 
      "f(x,y) only" : "x,y,f(x,y) ");
   if (sdip)
   {  printf(" w/A&S ");
      Spacer(8);
   }
   else Spacer(15);
   if (updir > 0) printf("Output order from south to north");
   else printf("Output order from north to south");
   Leader(indent);
   if (z_only)
   {  if (sdip)
      {  printf("Output format: %s",out1aform);
         i0 = strlen(out1aform);
      }
      else
      {  printf("Output format: %s",out1form);
         i0 = strlen(out1form);
      }
   }
   else
   {  if (sdip)
      {  printf("Output format: %s",out3aform);
         i0 = strlen(out3aform);
      }
      else
      {  printf("Output format: %s",out3form);
         i0 = strlen(out3form);
      }
   }
   if (!tgrid)
   {  Spacer(35 - i0);
      printf("Rectangular grid");
   }
   else
   {  Spacer(36 - i0);
      printf("Triangular grid");
   }
   if (gnup)
   {  Leader(indent);
      printf("Gnuplot compatible output");
   }
   if (igif)
   {  if (gnup) 
      {  if (idisplay EQ 2) Spacer(8);
         else Spacer(7);
      }
      else Leader(indent);
      printf("Make a *.gif file ");
      switch (idisplay)
      {  case 0: printf("of height values");
         break;
         case 1: printf("of aspect values");
         break;
         case 2: printf("of slope values");
         break;
         default:;
      }
      Leader(indent);
      printf("Extra gif pixels per x cell: %d      Extra gif pixels per y cell: %d\n",
         x_extra,y_extra);
   }
   saygo = 1;
   Leader(0);
   Leader(indent);
   printf("Change parameters or Make the grid? (C or M)  ");
   InString();
   if (strcmp(inbuf,"c") EQ 0 OR 
      strcmp(inbuf,"C") EQ 0) saygo = 0;
}
#endif
/***********************************************************************/
/*
void InitScrn(void)
{
   Leader(0);
   Leader(indent);
   printf("%s",titlstr);
   printf(" %s",cpwtstr);
   Leader(0);
}
*/
/***********************************************************************/
/*
void ShowMenu(void)
{
   double bignum;
   Leader(indent);
   printf("N.B. Press ENTER to accept existing parameter or right hand choice\n");
   Leader(indent);
   printf("Path and name of input data set ");
   InString();
   if (strlen(inbuf)>0) sprintf(dat_file,"%s",inbuf);
   Spacer(indent);
   printf("Path and name of output grid set ");
   InString();
   if (strlen(inbuf)>0) sprintf(grd_file,"%s",inbuf);
   Spacer(indent);
   printf("Change any other parameters? (N or Y) ");
   InString();
   if (strcmp(inbuf,"n") EQ 0 OR 
      strcmp(inbuf,"N") EQ 0) goto DONE;
   Spacer(indent);
   printf("Print data max and min to screen? (Y or N) ");
   InString();
   if (strlen(inbuf)<1 OR strcmp(inbuf,"n") EQ 
      0 OR strcmp(inbuf,"N") EQ 0) mxmn = 0;
   else mxmn = 1;
   Spacer(indent);
   printf("Change input data scaling? (Y or N) ");
   InString();
   if (strcmp(inbuf,"y") EQ 0 OR 
      strcmp(inbuf,"Y") EQ 0)
   {  Spacer(indent+2);
      printf("N.B. Scale factor of 1 means no scaling");
      Leader(indent+2);
      printf("Change east-west scale? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0)
      {  Spacer(indent+4);
         printf("Scale factor? ");
         InString();
         if (strlen(inbuf)>0)
         {  bignum = atof(inbuf);
            magx = bignum;
         }
         if (magx EQ 1) ixmag = 0;
         else ixmag = 1;
      }
      else
      {  Spacer(indent+2);
         printf("Change north-south scale? (Y or N) ");
         InString();
         if (strcmp(inbuf,"y") EQ 0 OR 
            strcmp(inbuf,"Y") EQ 0)
         {  Spacer(indent+4);
            printf("Scale factor? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atof(inbuf);
               magy = bignum;
            }
            if (magy EQ 1) iymag = 0;
            else iymag = 1;
         }
      }
      Spacer(indent+2);
      printf("Change vertical scale? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0)
      {  Spacer(indent+4);
         printf("Scale factor? ");
         InString();
         if (strlen(inbuf)>0)
         {  bignum = atof(inbuf);
            magz = bignum;
         }
         if (magz EQ 1) izmag = 0;
         else izmag = 1;
      }
      if (ixmag OR iymag OR izmag) imag = 1;
      else imag = 0;
   }
   Spacer(indent);
   printf("Print execution time? (Y or N) ");
   InString();
   if (strlen(inbuf)<1 OR strcmp(inbuf,"n") EQ 
      0 OR strcmp(inbuf,"N") EQ 0) atime = 0;
   else atime = 1;
   Spacer(indent);
   printf("Change type of interpolation? (Y or N) ");
   InString();
   if (strcmp(inbuf,"y") EQ 
      0 OR strcmp(inbuf,"Y") EQ 0)
   {  Spacer(indent+2);
      printf("Functional interpolation? (N or Y) ");
      InString();
      if (strcmp(inbuf,"n") EQ 
         0 OR strcmp(inbuf,"N") EQ 0)
      {  ichoro = 1;
         Spacer(indent+2);
         printf("Density or Choropleth? (D or C) ");
         InString();
         if (strcmp(inbuf,"d") EQ 0 OR 
            strcmp(inbuf,"D") EQ 0) densi = 1;
      }
      else densi = ichoro = 0;
      InterpStr();
      Spacer(indent+2);
      printf("Input format? ");
      InString();
      if (strlen(inbuf)>0)
      {  if (densi) sprintf(indform,"%s",inbuf);
         else sprintf(infform,"%s",inbuf);
      }
      Spacer(indent+2);
      non_neg = 0;                  // added 8 July 94
      printf("Use estimated gradients? (N or Y) ");
      InString();
      if (strlen(inbuf)<1 OR strcmp(inbuf,"y") EQ 
         0 OR strcmp(inbuf,"Y") EQ 0)
      {  igrad = 1;
         Spacer(indent+2);
         printf("Change tautness parameters? (Y or N) ");
         InString();
         if (strcmp(inbuf,"y") EQ 0 OR 
            strcmp(inbuf,"Y") EQ 0)
         {  Spacer(indent+4);
            printf("Tautness#1? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atof(inbuf);
               bI = bignum;
               if (bI < 1.0) bI = 1;
               if (bI > 3.0) bI = 3;
            }
            Spacer(indent+4);
            printf("Tautness#2? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atof(inbuf);
               bJ = bignum;
               if (bJ < 3.0) bJ = 3;
               if (bJ > 9.0) bJ = 9;
            }
         }
         Spacer(indent+2);         // these five line added 8 July 94
         printf("Negative values allowed? (N or Y) ");
         InString();
         if (strcmp(inbuf,"n") EQ 0 OR 
            strcmp(inbuf,"N") EQ 0) non_neg = 1;
      }
      else igrad = 0;
      Spacer(indent+2);
      printf("Change extrapolation option? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR strcmp(inbuf,"Y") EQ 0)
      {  Spacer(indent+4);
         printf("Allow extrapolation? (N or Y) ");    // reversed order 8 July 94
         InString();
         if (strcmp(inbuf,"n") EQ 0 OR strcmp(inbuf,"N") EQ 0) // replaced y with n
         {  extrap = 0;                     // 1 for 0, 180894
            Spacer(indent+6);
            printf("Null value? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atof(inbuf);
               nuldat = bignum;
            }
         }
         else extrap = 1; // removed '= gnup = sdip = igif', 080794; 0 for 1, 180894
      }
      Spacer(indent+2);    // removed 'if (extrap)' 8 July 94
      printf("Calculate aspect and slope? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0)
      {  sdip = 1;
         Spacer(indent+4);
         printf("Use Radians or Degrees? (R or D) ");
         InString();
         if (strcmp(inbuf,"r") EQ 0 OR 
            strcmp(inbuf,"R") EQ 0) rads = 1;
         else rads = 0;
         Spacer(indent+4);
         printf("Reset hemisphere? (Y or N) ");
         InString();
         if (strcmp(inbuf,"y") EQ 0 OR 
            strcmp(inbuf,"Y") EQ 0)
         {  Spacer(indent+4);
            printf("Southern or Northern hemisphere? (S or N) ");
            InString();
            southhemi = 0;
            if (strcmp(inbuf,"s") EQ 0 OR
                strcmp(inbuf,"S") EQ 0) southhemi = 1;
         }
      }
      else sdip = 0;
   }
   Spacer(indent);
   printf("Change optimization? (Y or N) ");
   InString();
   if (strcmp(inbuf,"y") EQ 0 OR 
      strcmp(inbuf,"Y") EQ 0)
   {  Spacer(indent+2);
      printf("Optimize for Memory or Speed? (M or S) ");
      InString();
      if (strcmp(inbuf,"m") EQ 0 OR 
         strcmp(inbuf,"M") EQ 0) optim = 0;
      else optim = 1;
   }
   Spacer(indent);
   printf("Change gridded region limits? (N or Y) ");
   InString();
   if (strlen(inbuf)<1 OR strcmp(inbuf,"y") EQ 
      0 OR strcmp(inbuf,"Y") EQ 0)
   {  Spacer(indent+2);
      printf("Least easting? ");
      InString();
      if (strlen(inbuf)>0)
      {  bignum = atof(inbuf);
         xstart = bignum;
      }
      Spacer(indent+2);
      printf("Least northing? ");
      InString();
      if (strlen(inbuf)>0)
      {  bignum = atof(inbuf);
         ystart = bignum;
      }
      Spacer(indent+2);
      printf("Greatest easting? ");
      InString();
      if (strlen(inbuf)>0)
      {  bignum = atof(inbuf);
         xterm = bignum;
      }
      Spacer(indent+2);
      printf("Greatest northing? ");
      InString();
      if (strlen(inbuf)>0)
      {  bignum = atof(inbuf);
         yterm = bignum;
      }
      Spacer(indent+2);
      printf("Change gridded region overlaps? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0)
      {  Spacer(indent+4);
         printf("Easting overlaps? ");
         InString();
         if (strlen(inbuf)>0)
         {  bignum = atof(inbuf);
            horilap = bignum;
         }
         Spacer(indent+4);
         printf("Northing overlaps? ");
         InString();
         if (strlen(inbuf)>0)
         {  bignum = atof(inbuf);
            vertlap = bignum;
         }
      }
      Spacer(indent+2);
      printf("Change grid mesh proportions? (Y or N) ");
      InString();
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0)
      {  Spacer(indent+4);
         printf("Alter Number of nodes or Spacing? (N or S) ");
         InString();
         if (strcmp(inbuf,"n") EQ 0 OR 
            strcmp(inbuf,"N") EQ 0)
         {  Spacer(indent+6);
            printf("Number of nodes, west to east? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atoi(inbuf);
               x_nodes = (int) bignum;
               xspace = (xterm - xstart) / 
                  (x_nodes - 1);
            }
            Spacer(indent+6);
            printf("Number of nodes, south to north? ");
            InString();
            if (strlen(inbuf)>0)
            {  bignum = atoi(inbuf);
               y_nodes = (int) bignum;
               yspace = (yterm - ystart) / 
                  (y_nodes - 1);
            }
         }
         else
         {  Spacer(indent+6);
            printf("Spacing of nodes, west to east? ");
            InString();
            if (strlen(inbuf)>0)
            {  xspace = atof(inbuf);
               x_nodes = 1 + (WEEBIT + xterm - 
                  xstart) / xspace;
               bignum = xstart + (x_nodes - 1) * 
                  xspace;
               if (fabs(bignum - xterm) > WEEBIT)
               {  Spacer(indent+6);
                  printf("N.B. Greatest x: has been altered to suit this spacing\n");
                  xterm = bignum;
               }
            }
            Spacer(indent+6);
            printf("Spacing of nodes, south to north? ");
            InString();
            if (strlen(inbuf)>0)
            {  yspace = atof(inbuf);
               y_nodes = 1 + (WEEBIT + yterm - 
                  ystart) / yspace;
               bignum = ystart + (y_nodes - 1) * 
                  yspace;
               if (fabs(bignum - yterm) > WEEBIT)
               {  Spacer(indent+6);
                  printf("N.B. Greatest y: has been altered to suit this spacing\n");
                  yterm = bignum;
               }
            }
         }
         if (x_nodes % 2 OR y_nodes % 2) tgrid = 0;
      }
      xspace = (xterm - xstart) / (x_nodes - 1);
      yspace = (yterm - ystart) / (y_nodes - 1);
   } 
   Spacer(indent);
   printf("Change output parameters? (Y or N) ");
   InString();
   if (strcmp(inbuf,"y") EQ 0 OR strcmp(inbuf,"Y") EQ 0)
   {  Spacer(indent+2);
      printf("Save x,y,f(x,y) or f(x,y) only? (x or f) ");
      z_only = 1;
      InString();
      if (strcmp(inbuf,"x") EQ 0 OR 
         strcmp(inbuf,"X") EQ 0) z_only = 0;
      Spacer(indent+4);
      if (z_only)
      {  if (sdip)
         {  printf("Output format for f(x,y) w/A&S? ");
            InString();
            if (strlen(inbuf)>0) 
               sprintf(out1aform,"%s",inbuf);
         }
         else
         {  printf("Output format for f(x,y)? ");
            InString();
            if (strlen(inbuf)>0) 
               sprintf(out1form,"%s",inbuf);
         }
      }
      else
      {  if (sdip)
         {  printf("Output format for x,y,f(x,y) w/A&S? ");
            InString();
            if (strlen(inbuf)>0) 
               sprintf(out3aform,"%s",inbuf);
         }
         else
         {  printf("Output format for x,y,f(x,y)? ");
            InString();
            if (strlen(inbuf)>0) 
               sprintf(out3form,"%s",inbuf);
         }
      }
      Spacer(indent+2);
      printf("Triangular or Rectangular grid? (T or R) ");
      InString();
      tgrid = 0;
      if (strcmp(inbuf,"t") EQ 0 OR 
         strcmp(inbuf,"T") EQ 0)
      {  tgrid = 1;
         gnup = 0;
         if (x_nodes % 2) x_nodes++;
         if (y_nodes % 2) y_nodes++;
         xspace = (xterm - xstart) / (x_nodes - 1);
         yspace = (yterm - ystart) / (y_nodes - 1);
      }
      else
      {  Spacer(indent+2);  // removed 'if (extrap)' 8 July 94
         printf("Gnuplot compatible output? (N or Y) ");
         InString();
         if (strlen(inbuf) < 1 OR strcmp(inbuf,"y") EQ 
            0 OR strcmp(inbuf,"Y") EQ 0)
            gnup = 1;
         else gnup = 0;
      }
      Spacer(indent+2);
      printf("Generate *.gif file of grid? (Y or N) ");
      InString();
      igif = 0;
      if (strcmp(inbuf,"y") EQ 0 OR 
         strcmp(inbuf,"Y") EQ 0) 
      {  igif = 1;
         idisplay = 0;
         if (sdip)
         {  Spacer(indent+4);
            printf("Show aspect, slope, or height, values? (A, S, or H) ");
            InString();
            if (strcmp(inbuf,"a") EQ 0 OR 
               strcmp(inbuf,"A") EQ 0) idisplay = 1;
            if (strcmp(inbuf,"s") EQ 0 OR 
               strcmp(inbuf,"S") EQ 0) idisplay = 2;
         }
         Spacer(indent+4);
         printf("Extra pixels per cell in x direction? ");
         InString();
         x_extra = 0;
         if (strlen(inbuf)>0) 
         {  x_extra = atoi(inbuf);
            if (x_extra < 0) x_extra = 0;
         }
         Spacer(indent+4);
         printf("Extra pixels per cell in y direction? ");
         InString();
         y_extra = 0;
         if (strlen(inbuf)>0) 
         {  y_extra = atoi(inbuf);
            if (y_extra < 0) y_extra = 0;
         }
      }
      Spacer(indent+2);
      printf("Output order North to south?");
      Leader(indent+12);
      printf("or South to north? (N or S) ");
      InString();
      arriba = 1;
      if (strcmp(inbuf,"n") EQ 0 OR 
         strcmp(inbuf,"N") EQ 0) arriba = -1;
      updir = (int) arriba;
   }
DONE:;
}
*/
/***********************************************************************/
/*
void InString(void)
{
   register char *i;
   for (i=inbuf;;)
   {  *i = getc(stdin);
      if (*i  EQ '\n')
      {  *i = '\0';
         break;
      }
      else *i++; 
   }
}
*/
/***********************************************************************/
/*
void Leader(int num)
           // 9/9/94
{
   int i0;
   printf("\n");
   for (i0=0; i0<num; i0++) printf(" ");
}
*/
/***********************************************************************/
/*
void Spacer(int num)
           // 9/9/94
{
   int i0;
   for (i0=0; i0<num; i0++) printf(" ");
}
*/
/***********************************************************************/
/*
void MakeGnufl(void) 
{
   FILE   *fileq;
   char *astring, *file_nam = "nngnufl.asc";
   astring = (char *) malloc(64 * sizeof(char));
   if ((fileq = fopen(file_nam,"w"))!=NULL)
   {  sprintf(astring,"# command file for plotting isolines from %s",grd_file);
      fprintf(fileq,"%s\n",astring);
      fprintf(fileq,"set xlabel \"Easting\" -5,-2    # label and position for x-axis\n");
      fprintf(fileq,"set ylabel \"Northing\" 4,-1   # label and position for y-axis\n");
      fprintf(fileq,"set zlabel \"Height\"          # label for z-axis\n");
      fprintf(fileq,"set contour base\n");
      fprintf(fileq,"set surface\n");
      fprintf(fileq,"set nokey\n");
      if (maxxy[0][0] > maxxy[0][1]) 
         fprintf(fileq,"set size 1,%2.1f\n",
         maxxy[0][1]/maxxy[0][0]);
      else fprintf(fileq,"set size %2.1f,1\n",
         maxxy[0][0]/maxxy[0][1]);
      fprintf(fileq,"set cntrparam levels 10\n");
      fprintf(fileq,"set mapping cartesian\n");
      fprintf(fileq,"set autoscale\n"); 
      fprintf(fileq,"set data style lines\n");
      fprintf(fileq,"set view 60,30,1,1\n");
      sprintf(astring,"set title \"Perspective view of %s\"",dat_file);
      fprintf(fileq,"%s\n",astring);
      if (z_only) fprintf(fileq,
         "set noparametric\n");
      else      fprintf(fileq,"set parametric\n");
      sprintf(astring,"splot \"%s\"",grd_file);
      if (sdip AND z_only) strcat(astring,
         " using 1");
      fprintf(fileq,"%s\n",astring);
      fprintf(fileq,"# load \"rotate.gnu\"\n");
      fprintf(fileq,"pause -1 \"Press return to see plan view\"\n");
      fprintf(fileq,"set view 0,0,1\n");
      fprintf(fileq,"set nosurface\n");
      sprintf(astring,"set title \"Plan view of %s\"",dat_file);
      fprintf(fileq,"%s\n",astring);
      fprintf(fileq,"replot\n");
      fprintf(fileq,"pause -1 \"Press return to close display\"\n");
      fprintf(fileq,"quit");
      fclose(fileq);
      Leader(indent);
      printf("To view contours type: gnuplot %s",
         file_nam);
   }
}
*/
/***********************************************************************/
/*
void WriteInit(void)
{
FILE *filep;

   if ((filep = fopen("nngridr.ini","w")) NE NULL)
   {  fprintf(filep, "%s\n",     dat_file);
      fprintf(filep, "%s\n",     grd_file);
      fprintf(filep, "%5d\n",    igrad);
      fprintf(filep, "%5d\n",    ichoro);
      if (!ichoro) densi = 0;
      fprintf(filep, "%5d\n",    densi);
      fprintf(filep, "%5d\n",    tgrid);
      fprintf(filep, "%5d\n",    x_nodes);
      fprintf(filep, "%5d\n",    y_nodes);
      fprintf(filep, "%5d\n",    non_neg);
      fprintf(filep, "%5d\n",    z_only);
      fprintf(filep, "%5d\n",    gnup);
      fprintf(filep, "%5d\n",    sdip);
      fprintf(filep, "%5d\n",    rads);
      fprintf(filep, "%5d\n",    optim);
      fprintf(filep, "%e\n",     arriba);
      fprintf(filep, "%e\n",     bI);
      fprintf(filep, "%e\n",     bJ);
      fprintf(filep, "%e\n",     magx);
      fprintf(filep, "%e\n",     magy);
      fprintf(filep, "%e\n",     magz);
      fprintf(filep, "%e\n",     xstart);
      fprintf(filep, "%e\n",     ystart);
      fprintf(filep, "%e\n",     xterm);
      fprintf(filep, "%e\n",     yterm);
      fprintf(filep, "%e\n",     horilap);
      fprintf(filep, "%e\n",     vertlap);
      fprintf(filep, "%e\n",     nuldat);
      fprintf(filep, "%5d\n",    extrap);
      fprintf(filep, "%5d\n",    igif);
      fprintf(filep, "%5d\n",    idisplay);
      fprintf(filep, "%5d\n",    x_extra);
      fprintf(filep, "%5d\n",    y_extra);
      fprintf(filep, "%5d\n",    southhemi);
      fprintf(filep, "%s\n",     indform);
      fprintf(filep, "%s\n",     infform);
      fprintf(filep, "%s\n",     out1form);
      fprintf(filep, "%s\n",     out1aform);
      fprintf(filep, "%s\n",     out3form);
      fprintf(filep, "%s\n",     out3aform);
      fclose(filep);
   }
}
*/
/***********************************************************************/
/*
void InterpStr(void)
{
   if (ichoro)
   {  if (densi) strcpy(intstr,densstr);
      else strcpy(intstr,chorstr);
   }
   else strcpy(intstr,funcstr);
}
*/
/***********************************************************************/
/*
void MakeToGif(void)  
{
   if (z_only)
   {  if (sdip) 
      {  if (idisplay EQ 0) togif = 2;
         else 
         {  if (idisplay EQ 1) togif = 3;
            else togif = 4;
         }
      }
      else togif = 1;
   }
   else
   {  if (sdip) 
      {  if (idisplay EQ 0) togif = 5;
         else 
         {  if (idisplay EQ 1) togif = 6;
            else togif = 7;
         }
      }
      else togif = 4;
   }
}
*/
/***********************************************************************/
/*
int GetSize(double anum)
{
   int i1 = 3;
   anum = fabs(anum);
   if (anum < 10000) return i1;
   else i1 = (int) log10(anum);
   return i1;
}
*/
