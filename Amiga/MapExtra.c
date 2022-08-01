/* MapExtra.c
** Bunches of good stuff to make Map work.
** original code by Gary R. Huber
*/

#include "WCS.h"
#include "GUIDefines.h"

#ifdef UNUSED_VARIABLES_GC  // AF, not used 17.May 2022 found with -gc
STATIC_VAR short frontpen=2;
#endif
#ifdef UNUSED_VARIABLES_GC  // AF, not used 17.May 2022 found with -gc
STATIC_VAR USHORT graphtype;
#endif
STATIC_VAR short ptstore[8];


STATIC_FCN void markpt(short edpt, short col); // used locally only -> static, AF 19.7.2021
STATIC_FCN double SolveDistCart(double XJ, double YJ, double ZJ,
        double XK, double YK, double ZK); // used locally only -> static, AF 23.7.2021
// void ZeroMatrix3x3(Matx3x3 A); // AF, not used 26.July 2021
STATIC_FCN void MakeTempCart(double Lat, double Lon, double SphereRad,
        double *X, double *Y, double *Z); // used locally only -> static, AF 23.7.2021
STATIC_FCN double SolveArcAng(double CartDist, double SphereRad); // used locally only -> static, AF 23.7.2021
STATIC_FCN void InitGauss(struct Gauss *Gauss); // used locally only -> static, AF 23.7.2021
STATIC_FCN double DoGauss(struct Gauss *Gauss); // used locally only -> static, AF 23.7.2021
STATIC_FCN void unmarkpt(short edpt); // used locally only -> static, AF 23.7.2021
// void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C); // AF, not used 26.July 2021




/* ALEXANDER now in WCS.h struct Gauss {
	double Arand, Nrand, Add, Fac;
	long Seed;
};*/

#define f3(delta,x0,x1,x2) 	(((x0 + x1 + x2) / 3.0) + delta * DoGauss(&Gauss))
#define f4(delta,x0,x1,x2,x3)	(((x0 + x1 + x2 + x3) / 4.0) + delta * DoGauss(&Gauss))


#ifdef ENABLE_STATISTICS

double *loadstat(double *dataptr,long type)
{
 char filename[255];
 FILE *fstat;
 short i, j;
 long zip, newstatrows, newstatcols;

RepeatLoad: 
 if ((AslRequestTags(frstat,
     ASL_Hail, (ULONG)"Statistics File Loader",
     TAG_DONE)) == NULL) {
  return(dataptr);
 }

 strcpy(statpath, frstat->rf_Dir);
 strcpy(statname, frstat->rf_File);
 strmfp(filename, statpath, statname);
 if ((fstat = fopen(filename, "r")) == NULL) {
  Log(ERR_OPEN_FAIL, statname);
  User_Message(statname, "Can't open file!", "OK", "o");
  return(dataptr);
 }
 
 fgets(filetype, 5, fstat);
 if (strncmp(filetype, "STAT\n", 5)) {
  fclose(fstat);
  Log(ERR_WRONG_TYPE, statname);
  if (User_Message(statname, "File not a WCS statistics file!\nSelect another?",
	"OK|CANCEL", "oc"))
   goto RepeatLoad;
  return(dataptr);
 }
 fgets(str, 30, fstat);
 newstatcols = atoi(str);
 fgets(str, 30, fstat);
 newstatrows = atoi(str);
 if (type == 1) {
  if (newstatrows != statrows || newstatcols != statcols) {
   fclose(fstat);
   Log(ERR_WRONG_SIZE, statname);
   User_Message(statname, "File is of incorrect size!\nOperation terminated.",
		"OK", "o");
   return(dataptr);
  }
 }  
 else {
  statcols = newstatcols;
  statrows = newstatrows;
  statsize = statrows * statcols * 8;
 }
 if (dataptr && (type==0)) free_Memory(dataptr, statsize);
 else if (dataptr && (type==1)) free_Memory(dataptr, normsize);
 if ((dataptr = (double *)get_Memory(statsize, MEMF_CLEAR)) == NULL) {
  fclose(fstat);
  User_Message(statname, "Out of memory!\n Operation terminated.", "OK", "o");
  graphptr = NULL;
  return(dataptr);
 }
 for (i=0; i<statcols; i++) {
  for (j=0; j<statrows; j++) {
   zip = (i * statrows + j);
   fgets(str, 30, fstat);
   *(dataptr+zip) = (double)atof(str);
  }
 }
 fclose(fstat);
 graphptr=dataptr;
 return(dataptr);

} /* loadstat() */

/************************************************************************/

void normalize(void)
{
 short error = 0, i, j;
 long zip;
 double avg,sum=0.0;

 if (!statptr) {
  User_Message("Mapping Module: Statistics",
	"No statistical data has been loaded to normalize!", "OK", "o");
  return;
 }
 if (normptr && (statrows == normrows) && (statcols == normcols)) {
  if (! User_Message("Mapping Module: Statistics",
		"Load new normalizing data?", "YES|NO", "yn"))
   goto UseOldData;
 }

 normptr = loadstat(normptr, 1);
 if (! normptr || error) {
  graphptr = statptr;
  return;
 }
 normrows = statrows;
 normcols = statcols;
 normsize = statrows * statcols * 8;
 
 if ((normvalptr = (double *)get_Memory(normsize, MEMF_CLEAR)) == NULL) {
  User_Message("Mapping Module: Statistics",
		"Out of memory!\n Operation failed.", "OK", "o");
  return;
 }
 if ((normstatptr = (double *)get_Memory(normsize, MEMF_CLEAR)) == NULL) {
  User_Message("Mapping Module: Statistics",
		"Out of memory!\nOperation failed.", "OK", "o");
  return;
 }

 Log(MSG_NULL, (CONST_STRPTR)"New normalizing data loaded.");
  
UseOldData:
 for (i=0; i<statcols; i++) {
  for (j=0; j<statrows; j++) {
   zip = (i * statrows + j);
   sum += *(normptr + zip);
  }
 }
 avg = sum / (statcols * statrows);
 for (i=0; i<statcols; i++) {
  for (j=0; j<statrows; j++) {
   zip = (i * statrows + j);
   if ((*(normptr+zip)) != 0) *(normvalptr + zip) = avg / (*(normptr + zip));
   else *(normvalptr + zip) = 0.0;
   *(normstatptr + zip) = *(normvalptr + zip) * (*(statptr + zip));
  }
 }
 graphptr = normstatptr;

} /* normalize() */

/************************************************************************/

void graphset(void)
{
/* allows selection of graph type (rose, histogram, xy frequency),
** and data to be graphed (original, normalized or normalizing).
** allows colors to be selected for graphing.
** allows scale for graph to be set with mouse (NW & SE corners).
** allows graph to be positioned (NW corner).
*/
 ULONG flags, iflags;
 UBYTE c0, c1;
 char RequestName[80];
 long done = 0;
 struct clipbounds cb;

 strcpy(RequestName, "Graphing Parameters");
 flags = WINDOWCLOSE | ACTIVATE | SMART_REFRESH | REPORTMOUSE;
 iflags = NULL;
 c0 = 0x01;
 c1 = 0x03;
 MapWind1 = (struct Window *)make_window(82, 80, 280, 190, RequestName,
     flags, iflags, c0, c1, WCSScrn);
 if (! MapWind1) return; 
 SetAPen(MapWind1->RPort, 1);
 strcpy(str, "Basic Data:");
 Move(MapWind1->RPort, 38, 40);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 30, 10, 10);
 if (graphptr && graphptr == statptr) checkbox(MapWind1->RPort, 175, 30, 10, 10);
 strcpy(str, "Normalized Data:");
 Move(MapWind1->RPort, 38, 60);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 50, 10, 10);
 if (graphptr && graphptr == normstatptr)
	 checkbox(MapWind1->RPort, 175, 50, 10, 10);
 strcpy(str, "Normalizing Data:");
 Move(MapWind1->RPort, 38, 80);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 70, 10, 10);
 if (graphptr && graphptr == normptr) checkbox(MapWind1->RPort, 175, 70, 10, 10);

 strcpy(str, "XY Plot:");
 Move(MapWind1->RPort, 38, 100);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 90, 10, 10);
 if (graphtype == 0) checkbox(MapWind1->RPort, 175, 90, 10, 10);
 strcpy(str, "Rose Diagram:");
 Move(MapWind1->RPort, 38, 120);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 110, 10, 10);
 if (graphtype == 1) checkbox(MapWind1->RPort, 175, 110, 10, 10);
 strcpy(str, "Histogram:");
 Move(MapWind1->RPort, 38, 140);
 Text(MapWind1->RPort, str, strlen(str));
 makebox(MapWind1->RPort, 175, 130, 10, 10);
 if (graphtype == 2) checkbox(MapWind1->RPort, 175, 130, 10, 10);

 strcpy(str, " X       Y       Wide    High");
 Move(MapWind1->RPort, 30, 160);
 Text(MapWind1->RPort, str, strlen(str));
 SetBPen(MapWind1->RPort, 3);
 SetAPen(MapWind1->RPort, 0);
 sprintf(str, "%d", cornerx);
 Move(MapWind1->RPort, 32, 172);
 Text(MapWind1->RPort, str, strlen(str));
 sprintf(str, "%d", cornery);
 Move(MapWind1->RPort, 97, 172);
 Text(MapWind1->RPort, str, strlen(str));
 sprintf(str, "%d", grwidth);
 Move(MapWind1->RPort, 165, 172);
 Text(MapWind1->RPort, str, strlen(str));
 sprintf(str,"%d",grheight);
 Move(MapWind1->RPort, 235, 172);
 Text(MapWind1->RPort, str, strlen(str));


 setclipbounds(MapWind0, &cb);
 ModifyIDCMP(MapWind1, CLOSEWINDOW | MOUSEBUTTONS | VANILLAKEY);

 while (! done) {
  FetchEvent(MapWind1, &Event);
  if (Event.Class == CLOSEWINDOW) done = 1;
  else if (Event.Class == MOUSEBUTTONS) {
   if (Event.Code == SELECTUP) {
    if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 30 && Event.MouseY < 40) {
     clearchecks(0);
     graphptr = statptr;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 30, 10, 10);
    }
    else if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 50 && Event.MouseY < 60) {
     clearchecks(0);
     graphptr = normstatptr;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 50, 10, 10);
    }
    else if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 70 && Event.MouseY < 80) {
     clearchecks(0);
     graphptr = normptr;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 70, 10, 10);
    }

    else if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 90 && Event.MouseY < 100) {
     clearchecks(1);
     graphtype = 0;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 90, 10, 10);
    }
    else if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 110 && Event.MouseY < 120) {
     clearchecks(1);
     graphtype = 1;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 110, 10, 10);
    }
    else if (Event.MouseX > 175 && Event.MouseX < 185 && Event.MouseY > 130 && Event.MouseY < 140) {
     clearchecks(1);
     graphtype = 2;
     SetAPen(MapWind1->RPort, 1);
     checkbox(MapWind1->RPort, 175, 130, 10, 10);
    }
    else if (Event.MouseY > 165) {
     SetBPen(MapWind1->RPort, 2);
     SetAPen(MapWind1->RPort, 0);
     sprintf(str, "%d", cornerx);
     Move(MapWind1->RPort, 32, 172);
     Text(MapWind1->RPort, str, 1);
     Move(MapWind1->RPort, 32, 172);
     getval(MapWind1, str, 32, 62, 1);
     if (str[0]) cornerx = atoi(str);
     if (cornerx < cb.lowx) cornerx = cb.lowx;
     else if (cornerx>cb.highx) cornerx=cb.highx;
     SetBPen(MapWind1->RPort, 3);
     sprintf(str, "%d", cornerx);
     Move(MapWind1->RPort, 32, 172);
     Text(MapWind1->RPort, str, strlen(str));

     SetBPen(MapWind1->RPort, 2);
     SetAPen(MapWind1->RPort, 0);
     sprintf(str, "%d", cornery);
     Move(MapWind1->RPort, 97, 172);
     Text(MapWind1->RPort, str, 1);
     Move(MapWind1->RPort, 97, 172);
     getval(MapWind1,str,97,127,1);
     if (str[0]) cornery = atoi(str);
     if (cornery < cb.lowy) cornery = cb.lowy;
     else if (cornery > cb.highy) cornery = cb.highy;
     SetBPen(MapWind1->RPort, 3);
     sprintf(str, "%d", cornery);
     Move(MapWind1->RPort, 97, 172);
     Text(MapWind1->RPort, str, strlen(str));

     SetBPen(MapWind1->RPort, 2);
     SetAPen(MapWind1->RPort, 0);
     sprintf(str, "%d", grwidth);
     Move(MapWind1->RPort, 165, 172);
     Text(MapWind1->RPort, str, 1);
     Move(MapWind1->RPort, 165, 172);
     getval(MapWind1, str, 165, 195, 1);
     if (str[0]) grwidth = atoi(str);
     if (grwidth<0) grwidth = 0;
     if (grwidth + cornerx > cb.highx) {
      DisplayBeep(WCSScrn);
      grwidth = cb.highx - cornerx;
     }
     SetBPen(MapWind1->RPort, 3);
     sprintf(str, "%d", grwidth);
     Move(MapWind1->RPort, 165, 172);
     Text(MapWind1->RPort, str, strlen(str));

     SetBPen(MapWind1->RPort, 2);
     SetAPen(MapWind1->RPort, 0);
     sprintf(str, "%d", grheight);
     Move(MapWind1->RPort, 235, 172);
     Text(MapWind1->RPort, str, 1);
     Move(MapWind1->RPort, 235, 172);
     done=getval(MapWind1, str, 235, 265, 1);
     if (str[0]) grheight = atoi(str);
     if (grheight < 0) grheight = 0;
     if (grheight + cornery > cb.highy) {
      DisplayBeep(WCSScrn);
      grheight = cb.highy - cornery;
     }
     SetBPen(MapWind1->RPort, 3);
     sprintf(str, "%d", grheight);
     Move(MapWind1->RPort, 235, 172);
     Text(MapWind1->RPort, str, strlen(str));
    }
   }
  }
 } /* while !done */
 ModifyIDCMP(MapWind1, CLOSEWINDOW);
 clearmesg(MapWind1);
 CloseWindow(MapWind1);

} /* graphset */

/************************************************************************/

void clearchecks(short j)
{
 short i;

 SetAPen(MapWind1->RPort, 0);
 for (i=0; i<3; i++) {
  checkbox(MapWind1->RPort, 175, 30 + j * 60 + i * 20, 10, 10);
 }
} /* clearchecks() */

/************************************************************************/

void graphdraw(double *dataptr)
{
/*
** checks to see if graph parameters are set and appropriate and if
** data is present of the type specified.
** if no: return(error)
** draws graph.
*/
 short i, j;
 long zip,col,ptx,pty,centerx=320,centery=200;
 double radius,angle,aspect=1.2,sum=0.0,tempsum,maxrad=0.0,secondmax=0.0,
        itemwidth,itemheight;

 if (!dataptr) {
  User_Message("Mapping Module: Statistics",
	"No statistical data has been loaded to graph!", "OK", "o");
  return;
 }
 
 for (i=0; i<statcols; i++) {
  for (j=0; j<statrows; j++) {
   zip=(i*statrows+j);
   sum +=*(dataptr+zip);
  }
 }
 if (graphtype==0) {
  LoadRGB4(&WCSScrn->ViewPort,AltColors,16);

  for (i=0; i<statcols; i++) {
   for (j=0; j<statrows; j++) {
    zip=(i*statrows+j);
    if (*(dataptr+zip)>secondmax) {
     if (*(dataptr+zip)>maxrad) {
      secondmax=maxrad;
      maxrad=*(dataptr+zip);
     }
     else secondmax=*(dataptr+zip);
    }
   }
  }
  maxrad=(maxrad+secondmax)/2.0;
  itemheight=(double)grheight/statrows;
  itemwidth=(double)grwidth/(statcols);
  for (i=0; i<statcols; i++) {
   for (j=0; j<statrows; j++) {
    zip=(i*statrows+j);
    if ((*(dataptr+zip))==0) col=0;
    else col=1.0+14.99*(*(dataptr+zip))/maxrad;
    if (col>15) col=15;
    ptx=cornerx+i*itemwidth;
    pty=cornery+grheight-j*itemheight;
    SetAPen(MapWind0->RPort,col);
    RectFill(MapWind0->RPort,ptx,pty-(long)itemheight,ptx+(long)itemwidth,pty);
   }
  }
  SetAPen(MapWind0->RPort,15);
  makebox(MapWind0->RPort,cornerx+(long)itemwidth,cornery,(grwidth-(long)itemwidth)/4,
            grheight);
  makebox(MapWind0->RPort,cornerx+(long)itemwidth,cornery,(grwidth-(long)itemwidth)/2,
            grheight);
  makebox(MapWind0->RPort,cornerx+(long)itemwidth,cornery,3*(grwidth-(long)itemwidth)/4,
            grheight);
  makebox(MapWind0->RPort,cornerx,cornery,grwidth,grheight);
 }  
  
 else if (graphtype==1) {
  SetAPen(MapWind0->RPort,frontpen);
  Move(MapWind0->RPort,centerx,centery);
  for (i=1,tempsum=0; i<statcols; i++,tempsum=0) {
   for (j=0; j<statrows; j++) {
    zip=(i*statrows+j);
    tempsum +=*(dataptr+zip); 
   }
   radius=tempsum/sum;
   if (radius>maxrad) maxrad=radius;
  }
  maxrad=200.0/maxrad;
  for (i=1,tempsum=0; i<statcols; i++,tempsum=0) {
   for (j=0; j<statrows; j++) {
    zip=(i*statrows+j);
    tempsum +=*(dataptr+zip); 
   }
   radius=maxrad*tempsum/sum;
   angle=((i-1)*10)/DTOR;
   Draw(MapWind0->RPort,(long)(centerx-cos(angle)*radius),(long)(centery-
     sin(angle)*radius/aspect));
   angle=(i*10)/DTOR;
   Draw(MapWind0->RPort,(long)(centerx-cos(angle)*radius),(long)(centery-
     sin(angle)*radius/aspect));
   Draw(MapWind0->RPort,centerx,centery);
  } /* for i=1 */
 } /* graphtype==1  */

} /* graphdraw() */

/************************************************************************/

void grapherase(void)
{
 struct Box Bx;
 
 if (GetBounds(&Bx))
  {
  if (Bx.Low.X > Bx.High.X)
   swmem(&Bx.Low.X, &Bx.High.X, sizeof (short)); 
  if (Bx.Low.Y > Bx.High.Y)
   swmem(&Bx.Low.Y, &Bx.High.Y, sizeof (short)); 
  SetAPen(MapWind0->RPort, backpen);
  RectFill(MapWind0->RPort, Bx.Low.X, Bx.Low.Y, Bx.High.X, Bx.High.Y);
  SetAPen(MapWind0->RPort, 1);
  } /* if */

} /* grapherase() */
#endif /* ENABLE_STATISTICS */
/************************************************************************/
#ifdef JHKJSHKASJHDKASHD
// Obsolete - replaced 4/17/95 by the code below

void findarea(short OBN)
{
 short i, error=0, found = 0;
 double area=0.0;

 if (! DBase[OBN].Lat) return;

 for (i=0; i<=DBase[OBN].Points; i++)
  {		/* convert to miles */
  mlat[i] =DBase[OBN].Lat[i] * LATSCALE;
  mlon[i] =DBase[OBN].Lon[i] * LATSCALE * cos(DBase[OBN].Lat[i] * PiOver180);
  } /* for i=0... */
 
 for (i=2; i<=DBase[OBN].Points; i++)
  {		/* compute area */
  area += (((mlat[i] + mlat[i-1]) / 2.0) * (mlon[i] - mlon[i-1]));
  } /* for i=2... */

/* if object not closed, close back to origin */
 if ((mlat[DBase[OBN].Points] != mlat[1])
	|| (mlon[DBase[OBN].Points] != mlon[1]))
  {
  error = 1;
  area += (((mlat[1] + mlat[DBase[OBN].Points]) / 2.0) *
            (mlon[1] - mlon[DBase[OBN].Points]));
  } /* if */
 area = abs(area);				/* area in sq km */

 if ((strcmp(DBase[OBN].Special, "TOP") && strcmp(DBase[OBN].Special, "SFC"))
	|| ! topoload)
  sprintf(str, "%s: A = %f sq km.", DBase[OBN].Name, area);
 else
  {
  for (i=0; i<topomaps; i++)
   {
   if (OBN == TopoOBN[i])
    {
    found = 1;
    break;
    } /* if map found */
   } /* for i=0... */
  if (found)
   sprintf(str, "%s (%dc x %dr): A = %f sq km.", DBase[OBN].Name, 
	mapelmap[i].rows + 1, mapelmap[i].columns, area);
  else
   sprintf(str, "%s: A = %f sq km.", DBase[OBN].Name, area);

  } /* else object is DEM and topos are loaded */
 if (error) strcat(str, " Not closed!");
 MapGUI_Message(0, str);

} /* findarea() */
#endif

/**********************************************************************/

void findarea(short OBN)
{
 short i, error=0, found = 0;
 double area=0.0;
 float Length = 0.0;
 
 double XOne, YOne, ZOne, XTwo = 0, YTwo = 0, ZTwo = 0;

 if (! DBase[OBN].Lat) return;

 for (i=0; i<=DBase[OBN].Points; i++)
  {		/* convert to miles */
  mlat[i] =DBase[OBN].Lat[i] * LATSCALE;
  mlon[i] =DBase[OBN].Lon[i] * LATSCALE * cos(DBase[OBN].Lat[i] * PiOver180);
  } /* for i=0... */
 
 for (i=2; i<=DBase[OBN].Points; i++)
  {		/* compute area */
  area += (((mlat[i] + mlat[i-1]) / 2.0) * (mlon[i] - mlon[i-1]));
	MakeTempCart(DBase[OBN].Lat[i], DBase[OBN].Lon[i], EARTHRAD,
	 &XOne, &YOne, &ZOne);
	MakeTempCart(DBase[OBN].Lat[i - 1], DBase[OBN].Lon[i - 1], EARTHRAD,
	 &XTwo, &YTwo, &ZTwo);
	
/*	Length += SolveArcAng(SolveDistCart(XOne, YOne, ZOne, XTwo, YTwo, ZTwo), EARTHRAD); */
	Length += (LATSCALE * PiUnder180 * 2.0 * SolveArcAng(
	 SolveDistCart(XOne, YOne, ZOne, XTwo, YTwo, ZTwo), EARTHRAD));
/*	Length += SolveDistCart(XOne, YOne, ZOne, XTwo, YTwo, ZTwo); */

  } /* for i=2... */

/* if object not closed, close back to origin */
 if ((mlat[DBase[OBN].Points] != mlat[1])
	|| (mlon[DBase[OBN].Points] != mlon[1]))
  {
  error = 1;
  area += (((mlat[1] + mlat[DBase[OBN].Points]) / 2.0) *
            (mlon[1] - mlon[DBase[OBN].Points]));
  } /* if */
 area = fabs(area);				/* area in sq km */

 if ((strcmp(DBase[OBN].Special, "TOP") && strcmp(DBase[OBN].Special, "SFC"))
	|| ! topoload)
  sprintf(str, "%s: A=%f sq km, L=%fkm.", DBase[OBN].Name, area, Length);
 else
  {
  for (i=0; i<topomaps; i++)
   {
   if (OBN == TopoOBN[i])
    {
    found = 1;
    break;
    } /* if map found */
   } /* for i=0... */
  if (found)
   sprintf(str, "%s (%ldc x %ldr): A=%f sq km.", DBase[OBN].Name, 
	mapelmap[i].rows + 1, mapelmap[i].columns, area);
  else
   sprintf(str, "%s: A=%f sq km.", DBase[OBN].Name, area);

  } /* else object is DEM and topos are loaded */
 if (error) strcat(str, "[Not closed]");
 MapGUI_Message(0, str);

} /* findarea() */

/***********************************************************************/

void FindDistance(void)
{
 double Length, XOne, YOne, ZOne, XTwo = 0, YTwo = 0, ZTwo = 0, TempLat, TempLon;
 struct Box Bx;

 MapGUI_Message(0, "\0338Set origin point.");
 SetWindowTitles(MapWind0, (STRPTR) "Set origin point", (UBYTE *)-1);

 if (! MousePtSet(&Bx.Low, NULL, 0))
  goto EndShift;

 MapGUI_Message(0, "\0338Set destination point. ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Set destination point", (UBYTE *)-1);
	
 if (! MousePtSet(&Bx.High, &Bx.Low, 1))
  goto EndShift;


 TempLon = X_Lon_Convert((long)Bx.Low.X);
 TempLat = Y_Lat_Convert((long)Bx.Low.Y);
 MakeTempCart(TempLat, TempLon, EARTHRAD, &XOne, &YOne, &ZOne);

 TempLon = X_Lon_Convert((long)Bx.High.X);
 TempLat = Y_Lat_Convert((long)Bx.High.Y);
 MakeTempCart(TempLat, TempLon, EARTHRAD, &XTwo, &YTwo, &ZTwo);
	
 Length = (LATSCALE * PiUnder180 * 2.0 * SolveArcAng(
	 SolveDistCart(XOne, YOne, ZOne, XTwo, YTwo, ZTwo), EARTHRAD));
 sprintf(str, "Length: %f km", Length);
 MapGUI_Message(0, str);

 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
 return;

EndShift:
 MapIDCMP_Restore(MapWind0);
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

} /* FindDistance() */

/***********************************************************************/

STATIC_FCN void MakeTempCart(double Lat, double Lon, double SphereRad,
	double *X, double *Y, double *Z) // used locally only -> static, AF 23.7.2021
{
double TempCos;

 TempCos = cos(Lat * Pi / 180) * SphereRad;
 *X = sin(Lon * Pi / 180) * TempCos;
 *Y = sin(Lat * Pi / 180) * SphereRad;
 *Z = cos(Lon * Pi / 180) * TempCos;

} /* MakeTempCart() */

/***********************************************************************/

STATIC_FCN double SolveDistCart(double XJ, double YJ, double ZJ,
	double XK, double YK, double ZK) // used locally only -> static, AF 23.7.2021
{

/* printf("XJ=%f, YJ=%f, ZJ=%f, XK=%f, YK=%f, ZK=%f.\n", XJ, YJ, ZJ, XK, YK, ZK);
 printf("DX=%f, DY=%f, DZ=%f.\n", XK - XJ, YK - YJ, ZK - ZJ);
 printf("Sum of squares=%f.\n", (((XK-XJ)*(XK-XJ)) + ((YK-YJ)*(YK-YJ)) + ((ZK-ZJ) * (ZK-ZJ))));
 printf("Cartesian distance=%f.\n", sqrt(((XK-XJ)*(XK-XJ)) + ((YK-YJ)*(YK-YJ)) + ((ZK-ZJ) * (ZK-ZJ)))); */

 return(sqrt(((XK-XJ)*(XK-XJ)) + ((YK-YJ)*(YK-YJ)) + ((ZK-ZJ) * (ZK-ZJ))));

} /* SolveDistCart() */

/***********************************************************************/

STATIC_FCN double SolveArcAng(double CartDist, double SphereRad) // used locally only -> static, AF 23.7.2021
{

 return(asin((CartDist / 2) / SphereRad));

} /* SolveArcArg() */


/************************************************************************/

void setorigin(void)
{
 short i, j, selectpoint, error;
 struct clipbounds cb;

 if (! DBase[OBN].Lat) return;

 if (DBase[OBN].Lat[1] != DBase[OBN].Lat[DBase[OBN].Points] ||
       DBase[OBN].Lon[1] != DBase[OBN].Lon[DBase[OBN].Points])
  {
  if (User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
          (CONST_STRPTR)"Object is not closed!\nThe origin cannot be moved.\nSet last vertex equal to first now?",
          (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
   {
   DBase[OBN].Lat[DBase[OBN].Points] = DBase[OBN].Lat[1];
   DBase[OBN].Lon[DBase[OBN].Points] = DBase[OBN].Lon[1];
   } /* if close */
  else
   return;
  } /* if */

 setclipbounds(MapWind0, &cb);

 MapGUI_Message(0, "\0338Select new origin. Q=done, Uu=up, Dd=down, ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Select new origin", (UBYTE *)-1);
 selectpoint = modpoints(0);
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
 if (selectpoint == 0)
  return;

 for (i=1; i<=DBase[OBN].Points; i++)
  {
  mlon[i] = DBase[OBN].Lon[i];
  mlat[i] = DBase[OBN].Lat[i];
  } /* for i=1... */
 for (i=selectpoint,j=1; i<DBase[OBN].Points; i++,j++)
  {
  DBase[OBN].Lon[j] = mlon[i];
  DBase[OBN].Lat[j] = mlat[i];
  } /* for i=... */
 for (i=1; j<DBase[OBN].Points; i++,j++)
  {
  DBase[OBN].Lon[j] = mlon[i];
  DBase[OBN].Lat[j] = mlat[i];
  } /* for i=1... */
 DBase[OBN].Lon[DBase[OBN].Points] = DBase[OBN].Lon[1];
 DBase[OBN].Lat[DBase[OBN].Points] = DBase[OBN].Lat[1];

 if (DBase[OBN].Color == 2) outline(MapWind0, OBN, 7, &cb);
 else outline(MapWind0, OBN, 2, &cb);
 sprintf(str, "Reset vector origin: %s.\n", DBase[OBN].Name);
 Log(MSG_NULL, (CONST_STRPTR)str);
 DBase[OBN].Flags |= 1;

 if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
         (CONST_STRPTR)"Conform vector to terrain and save Object now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
  {
  if (! topoload)
   error = loadtopo();
  if (! error)
   {
   if (! maptotopo(OBN))
    {
    sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
    Log(MSG_NULL, (CONST_STRPTR)str);
    } /* if conform to terrain */
   } /* if topos loaded */
  } /* if map to topo */

} /*setorigin() */

/************************************************************************/

void matchpoints(void)
{
 short i, error = 0;
 long matchA, matchB, matchC, matchD, matchU, matchL, match1, match2, match,
      firstobj, matchobj = 0;
 struct clipbounds cb;

 if (! DBase[OBN].Lat) return;

 setclipbounds(MapWind0, &cb);
 firstobj = OBN;

 MapGUI_Message(0, "\0338Select first source vertex. Q=done Uu=up Dd=down ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Select first source vertex", (UBYTE *)-1);
 if ((matchC = modpoints(0)) == 0)
  goto AbortMatch;

 MapGUI_Message(0, "\0338Select last source vertex. Q=done Uu=up Dd=down ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Select last source vertex", (UBYTE *)-1);
 if ((matchD = modpoints(0)) == 0)
  goto AbortMatch;
 match2 = matchD - matchC;

 findmouse(0, 0, 0);
 outline(MapWind0, OBN, DBase[OBN].Color, &cb);
 matchobj = OBN; 
 if (matchobj == firstobj)
  goto AbortMatch;

 MapGUI_Message(0, "\0338Select first dest'n vertex. Q=done Uu=up Dd=down ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Select first destination vertex", (UBYTE *)-1);
 if ((matchA = modpoints(0)) == 0)
  goto AbortMatch;

 MapGUI_Message(0, "\0338Select last dest'n vertex. Q=done Uu=up Dd=down ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Set last destination vertex", (UBYTE *)-1);
 if ((matchB = modpoints(0)) == 0)
  goto AbortMatch;

 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

 match1 = matchB - matchA;
 if (match1 > match2)
  {
  if (DBase[firstobj].Points + (match1 - match2) > MAXOBJPTS - 1)
   {
   User_Message((CONST_STRPTR)DBase[OBN].Name,
           (CONST_STRPTR)"Object resulting from this match would be larger than the maximum of MAXOBJPTS !\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto AbortMatch;
   } /* if larger than allowed */
  } /* if destination pts greater */
 if (match1 >= 0)
  {
  for (i=0; i<=match1; i++)
   {
   mlon[i]=DBase[OBN].Lon[i+matchA];
   mlat[i]=DBase[OBN].Lat[i+matchA];
   } /* for i=0... */
  } /* if destination pts > 0 */
 else
  {
  for (i=0; i<=-match1; i++)
   {
   mlon[i]=DBase[OBN].Lon[i+matchB];
   mlat[i]=DBase[OBN].Lat[i+matchB];
   } /* for i=0... */
  } /* else destination pts = 0 */

 OBN = firstobj;
 match = abs(abs(match2) - abs(match1));
 if (! match2 && match1)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Point Match",
          (CONST_STRPTR)"Illegal number of points!\nIf first and last destination points are the same, source points must be larger than zero.\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  outline(MapWind0, OBN, 2, &cb);
  return;
  } /* if */
 if (! User_Message_Def((CONST_STRPTR)"Mapping Module: Point Match",
         (CONST_STRPTR)"Proceed with relocation?", (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc", 1))
   return;
 if (match2 > 0)
  {
  matchU = matchD;
  matchL = matchC;
  } /* if source pts > 0 */
 else
  {
  matchU = matchC;
  matchL = matchD;
  } /* else source pts = 0 */
 if (abs(match2) > abs(match1))
  {
  for (i=matchU+1; i<=DBase[OBN].Points; i++)
   {
   DBase[OBN].Lon[i - match] = DBase[OBN].Lon[i];
   DBase[OBN].Lat[i - match] = DBase[OBN].Lat[i];
   } /* for i=... */
  DBase[OBN].Points -= match;
  } /* if more source pts than destination */
 else if (abs(match2) < abs(match1))
  {
  if (allocvecarray(OBN, DBase[OBN].Points + match, 0))
   {
   for (i=0; i<DBase[OBN].Points-matchU; i++)
    {
    DBase[OBN].Lon[DBase[OBN].Points-i + match]
	 = DBase[OBN].Lon[DBase[OBN].Points - i];
    DBase[OBN].Lat[DBase[OBN].Points-i + match]
	 = DBase[OBN].Lat[DBase[OBN].Points - i];
    } /* for i=0... */
   DBase[OBN].Points += match;
   } /* if memory allocated for larger array */
  else error = 1;
  } /* else destination pts greater than source */
 if (! error)
  {
  if (match1 * match2 == 0)
   {
   DBase[OBN].Lon[matchL] = mlon[0];
   DBase[OBN].Lat[matchL] = mlat[0];
   } /* if one or the other pts is zero */
  else if (match1 * match2 > 0)
   {
   for (i=0; i<=abs(match1); i++)
    {
    DBase[OBN].Lon[i+matchL] = mlon[i];
    DBase[OBN].Lat[i+matchL] = mlat[i];
    } /* for i=0... */
   } /* else if both match directions are the same */
  else
   {
   for (i=0; i<=abs(match1); i++)
    {
    DBase[OBN].Lon[i + matchL] = mlon[abs(match1) - i];
    DBase[OBN].Lat[i + matchL] = mlat[abs(match1) - i];
    } /* for i=0... */
   } /* else if match directions are different */
  } /* if not allocation error */
 outline(MapWind0, OBN, 2, &cb);
 if (error )
  {
  User_Message((CONST_STRPTR)"Mapping Module: Point Match",
          (CONST_STRPTR)"Out of memory!\nNot enough for new points.\nOperation failed.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if memory allocation error */

 sprintf(str, "Matched vector %s to %s.\n", DBase[OBN].Name, DBase[matchobj].Name);
 Log(MSG_NULL, (CONST_STRPTR)str);
 DBase[OBN].Flags |= 1;

 if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
         (CONST_STRPTR)"Conform vector to terrain and save Object now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
  {
  if (! topoload)
   error = loadtopo();
  if (! error)
   {
   if (! maptotopo(OBN))
    {
    sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
    Log(MSG_NULL, (CONST_STRPTR)str);
    } /* if conform to terrain */
   } /* if topos loaded */
  } /* if map to topo */

 return;

AbortMatch:
 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
 if (matchobj)
  outline(MapWind0, matchobj, DBase[matchobj].Color, &cb);
 OBN = firstobj;
 if (DBase[OBN].Color == 2)
  outline(MapWind0, OBN, 7, &cb);
 else
  outline(MapWind0, OBN, 2, &cb);

} /* matchpoints() */

/************************************************************************/

short modpoints(short modify)
{
 short j, firstmark = 0, lastmark = 0, done = 0, found, edpt = 1,
	xlow, xhigh, ylow, yhigh;

 ModifyIDCMP(MapWind0, VANILLAKEY | MOUSEBUTTONS);

 if (! DBase[OBN].Lat) return (0);

 while (! done)
  {
  markpt(edpt, 7);
  sprintf(str, "Vertex %d  Latitude %f  Longitude %f", edpt, 
	DBase[OBN].Lat[edpt], DBase[OBN].Lon[edpt]);
  MapGUI_Message(1, str);

  FetchEvent(MapWind0, &Event);

  unmarkpt(edpt);
  switch (Event.Class)
   {
   case VANILLAKEY:
    {
    switch (Event.Code)
     {
     case ESC:
      {
      done = -1;
      break;
      } /* abort */
     case UP:
      {
      edpt++;
      break;
      }
     case SHIFTUP:
      {
      edpt += 10;
      break;
      }
     case DOWN:
      {
      edpt --;
      break;
      }
     case SHIFTDOWN:
      {
      edpt -= 10;
      break;
      }
     case MARKFIRSTPT:
      {
      firstmark = edpt;
      break;
      }
     case MARKLASTPT:
      {
      lastmark = edpt;
      break;
      }
     case DELPT:
      {
      if (modify)
       {
       SetAPen(MapWind0->RPort, backpen);
       if (edpt > 1) Move(MapWind0->RPort, mapxx[edpt - 1], mapyy[edpt - 1]);
       else Move(MapWind0->RPort, mapxx[edpt], mapyy[edpt]);
       Draw(MapWind0->RPort, mapxx[edpt], mapyy[edpt]);
       if (edpt < DBase[OBN].Points)
        Draw(MapWind0->RPort, mapxx[edpt + 1], mapyy[edpt + 1]);
       if (edpt > 1 && edpt < DBase[OBN].Points)
        {
        if (DBase[OBN].Color == 2)
         SetAPen(MapWind0->RPort, 7);
        else
         SetAPen(MapWind0->RPort, 2);
        Draw(MapWind0->RPort,mapxx[edpt-1],mapyy[edpt-1]);
        } /* if not first or last vertex */
       for (j=edpt+1; j<=DBase[OBN].Points; j++)
        {
        mapxx[j-1] = mapxx[j];
        mapyy[j-1] = mapyy[j];
        DBase[OBN].Lon[j - 1] = DBase[OBN].Lon[j];
        DBase[OBN].Lat[j - 1] = DBase[OBN].Lat[j];
        DBase[OBN].Elev[j - 1] = DBase[OBN].Elev[j];
        } /* for j=... */
       DBase[OBN].Points --;
       DBase[OBN].Flags |= 1;
       }
      break;
      }
     case DELRANGE:
      {
      if (modify)
       {
       if (! firstmark || ! lastmark) break;
       } /* if */
      break;
      }
     case ADDPT:
      {
      break;
      }
     case SAVEPTS:
      {
      break;
      }
     case 'q':
     case QUITPTS:
      {
      done = 1;
      break;
      }
     } /* switch Event.Code */
    break;
    } /* VANILLAKEY */  
   case MOUSEBUTTONS:
    {
    if (Event.Code == SELECTUP)
     {
     xlow  = Event.MouseX - 5;
     xhigh = Event.MouseX + 5;
     ylow  = Event.MouseY - 5;
     yhigh = Event.MouseY + 5;
     for (j=1, found=0; j<=DBase[OBN].Points && !found; j++)
      {
         if(j>=MAXOBJPTS)
         {
         sprintf (str, "Arrysize exceeded in %s Line %d. %s %u (AF, 8.July2021)",__FILE__,__LINE__, DBase[OBN].Name,DBase[OBN].Points);
         Log(ERR_WRONG_SIZE, (CONST_STRPTR)str);
         break;
         }

      if (mapxx[j] < xlow || mapxx[j] > xhigh || mapyy[j] < ylow || mapyy[j] > yhigh) continue;
      found = 1;
      } /* for j=1... */
     if (found) edpt = j - 1;
     } /* if SELECTUP */
    break;
    } /* MOUSEBUTTONS */
   } /* switch Event.Class */

  if (edpt < 1)
   edpt = DBase[OBN].Points;
  else if (edpt > DBase[OBN].Points)
   edpt = 1;

  } /* while ! done */

 MapIDCMP_Restore(MapWind0);
 MapGUI_Message(1, " ");

 if (modify && (DBase[OBN].Flags & 1))
  {
  if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
          (CONST_STRPTR)"Conform vector to terrain and save Object now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
   {
   short error = 0;

   if (! topoload)
    error = loadtopo();
   if (! error)
    {
    if (! maptotopo(OBN))
     {
     sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
     Log(MSG_NULL, (CONST_STRPTR)str);
     } /* if conform to terrain */
    } /* if topos loaded */
   } /* if map to topo */
  } /* if no error */

 if (done < 0)
  edpt = 0;
 return (edpt);   

} /* modpoints */

/************************************************************************/

STATIC_FCN void markpt(short edpt, short col) // used locally only -> static, AF 19.7.2021
{

 ptstore[0] = ReadPixel(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt] - 1);
 ptstore[1] = ReadPixel(MapWind0->RPort, mapxx[edpt],    mapyy[edpt] - 1);
 ptstore[2] = ReadPixel(MapWind0->RPort, mapxx[edpt] + 1,mapyy[edpt] - 1);
 ptstore[3] = ReadPixel(MapWind0->RPort, mapxx[edpt] + 1,mapyy[edpt]    );
 ptstore[4] = ReadPixel(MapWind0->RPort, mapxx[edpt] + 1,mapyy[edpt] + 1);
 ptstore[5] = ReadPixel(MapWind0->RPort, mapxx[edpt],    mapyy[edpt] + 1);
 ptstore[6] = ReadPixel(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt] + 1);
 ptstore[7] = ReadPixel(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt]    );
 SetAPen(MapWind0->RPort, col);
 Move(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt] - 1);
 Draw(MapWind0->RPort, mapxx[edpt] + 1,mapyy[edpt] - 1);
 Draw(MapWind0->RPort, mapxx[edpt] + 1,mapyy[edpt] + 1);
 Draw(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt] + 1);
 Draw(MapWind0->RPort, mapxx[edpt] - 1,mapyy[edpt] - 1);

} /* markpt() */

/************************************************************************/

STATIC_FCN void unmarkpt(short edpt) // used locally only -> static, AF 23.7.2021
{
 if (ptstore[0]>-1) {
  SetAPen(MapWind0->RPort,ptstore[0]);
  WritePixel(MapWind0->RPort,mapxx[edpt]-1,mapyy[edpt]-1);
 }
 if (ptstore[1]>-1) {
  SetAPen(MapWind0->RPort,ptstore[1]);
  WritePixel(MapWind0->RPort,mapxx[edpt],  mapyy[edpt]-1);   
 }
 if (ptstore[2]>-1) {
  SetAPen(MapWind0->RPort,ptstore[2]);
  WritePixel(MapWind0->RPort,mapxx[edpt]+1,mapyy[edpt]-1);   
 }
 if (ptstore[3]>-1) {
  SetAPen(MapWind0->RPort,ptstore[3]);
  WritePixel(MapWind0->RPort,mapxx[edpt]+1,mapyy[edpt]  );   
 }
 if (ptstore[4]>-1) {
  SetAPen(MapWind0->RPort,ptstore[4]);
  WritePixel(MapWind0->RPort,mapxx[edpt]+1,mapyy[edpt]+1);   
 }
 if (ptstore[5]>-1) {
  SetAPen(MapWind0->RPort,ptstore[5]);
  WritePixel(MapWind0->RPort,mapxx[edpt],  mapyy[edpt]+1);   
 }
 if (ptstore[6]>-1) {
  SetAPen(MapWind0->RPort,ptstore[6]);
  WritePixel(MapWind0->RPort,mapxx[edpt]-1,mapyy[edpt]+1);   
 }
 if (ptstore[7]>-1) {
  SetAPen(MapWind0->RPort,ptstore[7]);
  WritePixel(MapWind0->RPort,mapxx[edpt]-1,mapyy[edpt]  );   
 }

} /* unmarkpt() */

/************************************************************************/

short CloneObject(void)
{
short OldObj;
struct clipbounds cb;

 if (! User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
         (CONST_STRPTR)"Duplicate this object?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
  return (0);

 OldObj = OBN;
 setclipbounds(MapWind0, &cb);

 if (DBaseObject_New())
  {
  if (allocvecarray(OBN, DBase[OldObj].Points, 0))
   {
   DBase[OBN].Points = DBase[OldObj].Points;
   memcpy(DBase[OBN].Lat, DBase[OldObj].Lat, DBase[OBN].VecArraySize);
   memcpy(DBase[OBN].Lon, DBase[OldObj].Lon, DBase[OBN].VecArraySize);
   memcpy(DBase[OBN].Elev, DBase[OldObj].Elev, DBase[OBN].VecArraySize / 4);
   }
  outline(MapWind0, OBN, 2, &cb);
  return (1);
  } /* if new object created */

 return (0);

} /* CloneObject() */

/************************************************************************/

void makestream(short lowj)
{
 short error, done=0, trend, el[9], lowel,
	next, n, pts, lastopo = -1, superdone = 0, topoct;
 long Lr, Lc;
 float last_lat, last_lon, sintrend, costrend, tantrend, h, Elev;
 struct elmapheaderV101 tempel;
 struct clipbounds cb;
 struct Box Bx;

 if (! DBase[OBN].Lat) return;

 error = (topoload == 0) ? loadtopo(): 0;
 if (error) return;

 setclipbounds(MapWind0, &cb);

 if (lowj == 0)
  {
  MapGUI_Message(0, "\0338Select stream start point. ESC=abort");
  SetWindowTitles(MapWind0, (STRPTR) "Select stream start point", (UBYTE *)-1);

  SetAPen(MapWind0->RPort, 1);

  if (! MousePtSet(&Bx.Low, NULL, 0))
   goto EndStream;
  mapxx[0] = mapxx[1] = Bx.Low.X;
  mapyy[0] = mapyy[1] = Bx.Low.Y;
  DBase[OBN].Lon[0] = X_Lon_Convert((long)mapxx[0]);
  DBase[OBN].Lat[0] = Y_Lat_Convert((long)mapyy[0]);
  DBase[OBN].Lon[1] = DBase[OBN].Lon[0];
  DBase[OBN].Lat[1] = DBase[OBN].Lat[0];
  } /* if */
 else
  {
  Bx.Low.X = mapxx[lowj - 1];
  Bx.Low.Y = mapyy[lowj - 1];
  } /* else */

 MapGUI_Message(0, "\0338Select approximate stream end point. ESC=abort");
 SetWindowTitles(MapWind0, (STRPTR) "Select approximate stream end point", (UBYTE *)-1);
 strcpy(str,"Making Stream: click on stream end point. ESC=abort");
 if (! MousePtSet(&Bx.High, &Bx.Low, 1))
  {
  if (lowj == 0) DBase[OBN].Points = 1;
  goto EndModify;
  } /* if abort */

 last_lon = X_Lon_Convert((long)Bx.High.X);
 last_lat = Y_Lat_Convert((long)Bx.High.Y);
 if (last_lon == DBase[OBN].Lon[lowj])
  {
  if (last_lat > DBase[OBN].Lat[lowj]) trend = 1;
  else if (last_lat < DBase[OBN].Lat[lowj]) trend = 5;
  else
   {
   if (lowj == 0) DBase[OBN].Points = 1;
   goto EndModify;
   } /* else */
  } /* if */
 else
  {
  h = sqrt((last_lat - DBase[OBN].Lat[lowj]) * (last_lat - DBase[OBN].Lat[lowj])
	+ (last_lon - DBase[OBN].Lon[lowj]) * (last_lon - DBase[OBN].Lon[lowj]));
  sintrend = (last_lat - DBase[OBN].Lat[lowj]) / h;
  costrend = (last_lon - DBase[OBN].Lon[lowj]) / h;
  tantrend = fabs((last_lat - DBase[OBN].Lat[lowj]) / (last_lon - DBase[OBN].Lon[lowj]));
  if (sintrend > 0.0)
   {
   trend = 1;
   if (costrend > 0.0)
    {
    if (tantrend < .4142) trend = 7;
    else if (tantrend < 2.4142) trend = 8;
    } /* if */
   else if (costrend < 0.0)
    {
    if (tantrend < .4142) trend = 3;
    else if (tantrend < 2.4142) trend = 2;
    } /* else if */
   } /* if */
  else
   {
   trend = 5;
   if (costrend > 0.0)
    {
    if (tantrend < .4142) trend = 7;
    else if (tantrend < 2.4142) trend = 6;
    } /* if */
   else if (costrend < 0.0)
    {
    if (tantrend < .4142) trend = 3;
    else if (tantrend < 2.4142) trend = 4;
    } /* else if */
   } /* else */
  } /* else */

 pts = lowj==0 ? 1: lowj; 
 while (! superdone && pts < MAXOBJPTS)
  {
  topoct = 0;
  while (! done && topoct < topomaps)
   {
   if (topoct == lastopo)
    {
    topoct ++;
    continue;
    } /* if */
   if (DBase[OBN].Lat[pts] < mapcoords[topoct].C[0] || 
       DBase[OBN].Lat[pts] > mapcoords[topoct].C[2] || 
       DBase[OBN].Lon[pts] > mapcoords[topoct].C[1] || 
       DBase[OBN].Lon[pts] < mapcoords[topoct].C[3]) 
       topoct ++;
   else done = 1;
   } /* while !done */
  if (topoct == topomaps) break;
  lastopo = topoct;

/* copy elevations to temporary array */   

  memcpy(&tempel, &mapelmap[topoct], sizeof (struct elmapheaderV101));
  if ((tempel.map = (short *)get_Memory(tempel.size, MEMF_ANY)) == NULL ||
	! allocvecarray(OBN, MAXOBJPTS, 0) )
   {
   User_Message((CONST_STRPTR)"Mapping Module: Follow Stream",
           (CONST_STRPTR)"Out of memory!\nNot enough for temporary topo array.\nOperation failed.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   goto EndModify;
   } /* if out of memory */
  memcpy(tempel.map, mapelmap[topoct].map, tempel.size);
  
/* find out where in the array the current point is */
  Lc = (DBase[OBN].Lat[pts] - mapcoords[topoct].C[0]) / tempel.steplat;
  Lr = (mapcoords[topoct].C[1] - DBase[OBN].Lon[pts]) / tempel.steplong;

/* store point coordinates as they are found */  
  for (; pts<MAXOBJPTS; pts++)
   {
   DBase[OBN].Lat[pts] = mapcoords[topoct].C[0] + Lc * tempel.steplat;
   DBase[OBN].Lon[pts] = mapcoords[topoct].C[1] - Lr * tempel.steplong;
   Elev = *(mapelmap[topoct].map + Lr * tempel.columns + Lc);
   DBase[OBN].Elev[pts] = Elev * mapelmap[topoct].elscale / ELSCALE_METERS;
   
   if (DBase[OBN].Lat[pts] > last_lat - tempel.steplat
	&& DBase[OBN].Lat[pts] < last_lat + tempel.steplat
	&& DBase[OBN].Lon[pts] > last_lon - tempel.steplong
	&& DBase[OBN].Lon[pts] < last_lon + tempel.steplong)
    {
    superdone = 1;
    break;
    } /* if */
   lowel = el[0] = *(tempel.map + Lr *tempel.columns + Lc);
   *(tempel.map + Lr * tempel.columns + Lc) = 30000;
   el[1] = (Lc < tempel.columns - 1) ?
	*(tempel.map + Lr * tempel.columns + Lc + 1): 0;
   el[2] = (Lr < tempel.rows && Lc < tempel.columns - 1) ?
	*(tempel.map + (Lr + 1) * tempel.columns + Lc + 1): 0;
   el[3] = (Lr < tempel.rows) ? *(tempel.map + (Lr + 1) * tempel.columns + Lc): 0;
   el[4] = (Lr < tempel.rows && Lc > 0) ?
	*(tempel.map + (Lr+1) * tempel.columns + Lc - 1): 0;
   el[5] = (Lc > 0) ? *(tempel.map + Lr * tempel.columns + Lc - 1): 0;
   el[6] = (Lr > 0 && Lc > 0) ?
	*(tempel.map + (Lr - 1) * tempel.columns + Lc - 1): 0;
   el[7] = (Lr > 0) ? *(tempel.map + (Lr - 1) * tempel.columns + Lc): 0;
   el[8] = (Lr > 0 && Lc < tempel.columns - 1) ?
	*(tempel.map + (Lr - 1) * tempel.columns + Lc + 1): 0;

/* loop until out of hole */
HoleLoop:
   next = 0;
   n = trend + 1;
   do
    {
    if (n > 8) n = 1;
    if (el[n] == 0) done = 1;     /* at edge of map */
    else if (el[n] <= lowel)
     {
     lowel = el[n];
     next = n;
     } /* else if */
    n ++;
    } while (n != trend + 1);
   if (done && ! next) break;     /* last point on current map */
   else done = 0;
   switch (next)
    {
    case 1: Lc ++; break;
    case 2: Lr ++; Lc ++; break;
    case 3: Lr ++; break;
    case 4: Lr ++; Lc --; break;
    case 5: Lc --; break;
    case 6: Lr --; Lc --; break;
    case 7: Lr --; break;
    case 8: Lr --; Lc ++; break;
    default: lowel ++; goto HoleLoop;   /* no points lower or equal */
    } /* switch next */

   } /* for pts<MAXOBJPTS... */
  if (superdone) break;

  if (pts == MAXOBJPTS)
   {
   User_Message((CONST_STRPTR)"Mapping Module: Follow Stream",
           (CONST_STRPTR)"Point maximum has been reached!\nMapping terminated", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   superdone = 1;
   } /* if */
  else
   {
   DBase[OBN].Points = pts;
   outline(MapWind0, OBN, 2, &cb);
   sprintf(str,
	 "Reached edge of current map!\nPoints = %d\nContinue to next map?", pts);
   superdone = 1 - User_Message((CONST_STRPTR)"Mapping Module: Follow Stream", (CONST_STRPTR)str,
           (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc");
   } /* else */

  free_Memory(tempel.map, tempel.size);

  } /* while !superdone */

 if (! superdone && pts == 1)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Follow Stream",
          (CONST_STRPTR)"Initial point not within currently loaded topo boundaries!\nObject points reduced to 1.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  DBase[OBN].Points = 1;
  DBase[OBN].Flags |= 1;
  return;
  } /* if */
 else if (! superdone && pts == lowj)
  {
  User_Message((CONST_STRPTR)"Mapping Module: Follow Stream",
          (CONST_STRPTR)"Initial point not within currently loaded topo boundaries!\nObject points reduced to 1.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* else if */

 if (pts >= MAXOBJPTS) pts = MAXOBJPTS - 1;
 DBase[OBN].Points = pts;
 outline(MapWind0, OBN, 2, &cb);

EndModify:
 DBase[OBN].Flags |= 1;

 if (User_Message_Def((CONST_STRPTR)"Mapping Module: Follow Stream",
         (CONST_STRPTR)"Save vector object now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
  saveobject(OBN, NULL, DBase[OBN].Lon, DBase[OBN].Lat, DBase[OBN].Elev);

EndStream:
 MapGUI_Message(0, " ");
 MapIDCMP_Restore(MapWind0);
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);

} /* makestream() */

/************************************************************************/

void interpolatepath(void)
{ 
 short i, points, frames, UseResult, error = 0;
 double x1, x2, x3, x4, y1, y2, y3, y4, xc, yc, angle,
        rotangle, r, sum, lonscale[2], shift1;
 struct clipbounds cb;
 struct VelocityDistr Vel;

 if (! DBase[OBN].Lat) return;

/* Initialize mlon/mlat arrays with object lat/lon pairs */
 setclipbounds(MapWind0, &cb);
 points = DBase[OBN].Points;
 for (i=0; i<=points; i++)
  {
  mlat[i] = DBase[OBN].Lat[i];
  mlon[i] = DBase[OBN].Lon[i];
  } /* for i=0... */

/* Iterate spline function until max number of points achieved */

 while (points * 2 - 1 < MAXOBJPTS)
  {
  for (i=points; i>0; i--)
   {
   mlat[i * 2 - 1] = mlat[i];
   mlon[i * 2 - 1] = mlon[i];
   } /* for i=... */
  points = points * 2 - 1;
  mlon[2] = (mlon[1] + mlon[3]) * .5;
  mlat[2] = (mlat[1] + mlat[3]) * .5;
  for (i=1; i<=points-4; i=i+2)
   {
   lonscale[0] = cos(((mlat[i + 2] + mlat[i]) * .5) * PiOver180) * LATSCALE;
   x2 = (mlon[i + 2] - mlon[i]) * lonscale[0];
   y2 = (mlat[i + 2] - mlat[i]) * LATSCALE;
   lonscale[1] = cos(((mlat[i + 2] + mlat[i + 4]) *.5) * PiOver180)* LATSCALE;
   x4 = x2 + (mlon[i + 4] - mlon[i + 2]) * lonscale[1];
   y4 = (mlat[i + 4] - mlat[i]) * LATSCALE;
   rotangle = findangle3(x4, y4);
   rotate2(&x4, &y4, rotangle, rotangle);
   rotate2(&x2, &y2, rotangle, findangle3(x2, y2));
   if (y2 == 0.0)
    {
    x1 = x2 / 2.0;
    y1 = 0.0;
    x3 = (x2 + x4) / 2.0;
    y3 = 0.0;
    } /* if */
   else
    {
    xc = x4 / 2.0;
    yc = (x2 * x2 + y2 * y2 - x2 * x4) /(2.0 * y2);
    r = sqrt(xc * xc + yc * yc);
    angle = findangle3((x2 / 2.0) - xc, (y2 / 2.0) - yc);
    x1 = xc + r * cos(angle);
    y1 = yc + r * sin(angle);
    angle = findangle3(((x4 + x2) / 2.0) - xc,((y4 + y2) / 2.0) - yc);
    x3 = xc + r * cos(angle);
    y3 = yc + r * sin(angle);
    } /* else */
   rotate2(&x1, &y1, -rotangle, findangle3(x1, y1));
   rotate2(&x3, &y3, -rotangle, findangle3(x3, y3));
   y1 = mlat[i] + y1 / LATSCALE;
   lonscale[0] = cos(y1 * PiOver180) * LATSCALE;
   x1 = mlon[i] + x1 / lonscale[0];
   mlon[i + 1] = (mlon[i + 1] + x1) * .5;
   mlat[i + 1] = (mlat[i + 1] + y1) * .5;
   y3 = mlat[i] + y3 / LATSCALE;
   lonscale[1] = cos(y3 * PiOver180) * LATSCALE;
   x3 = mlon[i] + x3 / lonscale[1];
   mlon[i + 3] = x3;
   mlat[i + 3] = y3;
   } /* for i=1 */
  } /* while points<MAXOBJPTS */
 tlat[1] = 0.0;
 tlon[1] = 0.0;
 for (i=2; i<=points; i++)
  {
  tlat[i] = tlat[i - 1] + (mlat[i] - mlat[i - 1]) * LATSCALE;
  lonscale[0] = cos(((mlat[i] + mlat[i - 1]) * .5) * PiOver180) * LATSCALE;
  tlon[i] = tlon[i - 1] + (mlon[i] - mlon[i - 1]) * lonscale[0];
  } /* for i=2... */
 sum = 0.0;
 for (i=1; i<points; i++)
  { 
  sum += sqrt((tlon[i + 1] - tlon[i]) * (tlon[i + 1] - tlon[i]) +
       (tlat[i + 1] - tlat[i]) * (tlat[i + 1] - tlat[i]));
  } /* for i=1... */

 frames = DBase[OBN].Points;
SetFrameCount:
 sprintf(str, "%hd", frames);
 if (! GetInputString("Enter number of output vertices.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
  return;
 frames = atoi(str);

 shift1 = sum / (frames - 1);
 sprintf(str, "Spline length = %f kilometers\nInterval = %f km/segment", sum, shift1);
 if ((UseResult = User_Message_Def((CONST_STRPTR)"Mapping Module: Spline", (CONST_STRPTR)str,
         (CONST_STRPTR)"OK|Reset|Cancel", (CONST_STRPTR)"orc", 1)) == 2)
  goto SetFrameCount;
 if (UseResult == 0)
 {
     return;
 }

  Vel.PtsIn = points;
  Vel.PtsOut = frames;
  Vel.EaseIn = 0;
  Vel.EaseOut = 0;
  Vel.Base = 1;
  Vel.Lat = &mlat[0];
  Vel.Lon = &mlon[0];
  Vel.Alt = NULL;
  DistributeVelocity(&Vel);

 if (allocvecarray(OBN, frames, 0))
  {
  outline(MapWind0, OBN, backpen, &cb);
  for (i=1; i<=frames; i++)
   {
   DBase[OBN].Lat[i] = mlat[i];
   DBase[OBN].Lon[i] = mlon[i];
   }
  DBase[OBN].Points = frames;
  outline(MapWind0, OBN, DBase[OBN].Color, &cb);
  DBase[OBN].Flags |= 1;

  if (User_Message_Def((CONST_STRPTR)"Mapping Module: Digitize",
          (CONST_STRPTR)"Conform vector to terrain and save object now?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc", 1))
   {
   if (! topoload)
    error = loadtopo();
   if (! error)
    {
    if (! maptotopo(OBN))
     {
     sprintf (str, "Vector %s conformed to topography.", DBase[OBN].Name);
     Log(MSG_NULL, (CONST_STRPTR)str);
     } /* if conform to terrain */
    } /* if topos loaded */
   } /* if map to topo */
  } /* if memory allocated for larger array */
 else
  User_Message((CONST_STRPTR)"Map View Module: Interpolate",
          (CONST_STRPTR)"Out of memory! Can't allocate new vector.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");

} /* interpolatepath() */

/************************************************************************/

void SetSurface_Map(ULONG surface)
{
 short i, mapwidth, mapheight;
 long Lr, Lc;
 struct Vertex Vtx;

 sprintf(str, "\0338Select Surface %lu Elevation. ESC=Abort", surface + 1);
 MapGUI_Message(0, str);
 sprintf(str, "Select Surface %lu Elevation", surface + 1);
 SetWindowTitles(MapWind0, (STRPTR) str, (UBYTE *)-1);

 if (MousePtSet(&Vtx, NULL, 0))
  {
/* determine map containing selected point */
  for (i=0; i<topomaps; i++)
   {
   latlon_XY(TopoOBN[i]);
   if (Vtx.X >= mapxx[2] && Vtx.X <= mapxx[1]
	 && Vtx.Y >= mapyy[3] && Vtx.Y <= mapyy[2])
    break;
   } /* for i=0... */
   mapwidth = mapxx[1] - mapxx[2] > 0 ? mapxx[1] - mapxx[2]: 1;
   mapheight = mapyy[1] - mapyy[4] > 0 ? mapyy[1] - mapyy[4]: 1;

/* determine row and column of selected point */
  Lr = (mapelmap[i].rows * (Vtx.X - mapxx[2])) / mapwidth;
  Lc = ((mapelmap[i].columns - 1) * (mapyy[1] - Vtx.Y)) / mapheight;
  settings.surfel[surface] 
	= *(mapelmap[i].map + Lr * mapelmap[i].columns + Lc);
  if (ES_Win)
   {
   set(ES_Win->IntStr[9 + surface], MUIA_String_Integer, settings.surfel[surface]);
   } /* if settings window open */
  } /* if point selected */

 MapGUI_Message(0, " ");
 SetWindowTitles(MapWind0, (STRPTR) "Map View", (UBYTE *)-1);
 MapIDCMP_Restore(MapWind0);

} /* SetSurface_Map() */

/***********************************************************************/

void FlatFixer(struct Box *Bx)
{
 char filename[256];
 short i, j, *Data, *Backup, Rows, Cols, Lr, Lc, LowRow, LowCol, HighRow, HighCol,
	OpenOK, EqualPts, error = 0, ans, Pts, PtRange;
 double LowLat, HighLat, LowLon, HighLon, SubLat, SubLon;
 LONG 	fhelev, zip,
	protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE,
	flags = O_WRONLY | O_CREAT | O_TRUNC;
 float Version = DEM_CURRENT_VERSION, T;
 struct DirList *DLItem;

 LowLon = X_Lon_Convert((long)Bx->Low.X);
 LowLat = Y_Lat_Convert((long)Bx->Low.Y);
 HighLon = X_Lon_Convert((long)Bx->High.X);
 HighLat = Y_Lat_Convert((long)Bx->High.Y);

 if (LowLon > HighLon)
  swmem(&LowLon, &HighLon, sizeof (double));
 if (LowLat > HighLat)
  swmem(&LowLat, &HighLat, sizeof (double));

 for (i=0; i<topomaps; i++)
  {
  if (((DBase[TopoOBN[i]].Flags & 2) == 0) || (DBase[TopoOBN[i]].Lat == 0))
   continue;
  if (! strcmp(DBase[TopoOBN[i]].Special, "TOP")
	 || ! strcmp(DBase[TopoOBN[i]].Special, "SFC"))
   {
   if(DBase[TopoOBN[i]].Lon[2] < LowLon || DBase[TopoOBN[i]].Lon[1] > LowLon ||
          DBase[TopoOBN[i]].Lat[2] > LowLat || DBase[TopoOBN[i]].Lat[3] < LowLat) continue;
       /* found the right one... */
   break;
   } /* if */
  } /* for i=0... */

 if (i >= topomaps)
  {
  error = 2;
  goto EndFix;
  }

 for (j=0; j<topomaps; j++)
  {
  if (((DBase[TopoOBN[j]].Flags & 2) == 0) || (DBase[TopoOBN[j]].Lat == 0))
   continue;
  if (! strcmp(DBase[TopoOBN[j]].Special, "TOP")
	 || ! strcmp(DBase[TopoOBN[j]].Special, "SFC"))
   {
   if(DBase[TopoOBN[j]].Lon[2] < HighLon || DBase[TopoOBN[j]].Lon[1] > HighLon ||
          DBase[TopoOBN[j]].Lat[2] > HighLat || DBase[TopoOBN[j]].Lat[3] < HighLat) continue;
       /* found the right one... */
   break;
   } /* if */
  } /* for i=0... */

 if (j >= topomaps)
  {
  error = 2;
  goto EndFix;
  }

 if (i != j)
  {
  error = 4;
  goto EndFix;
  }

 SubLat = LowLat - mapelmap[i].lolat;
 SubLon = mapelmap[i].lolong - LowLon;
     
 LowCol = SubLat / mapelmap[i].steplat;
 HighRow = SubLon / mapelmap[i].steplong;

 SubLat = HighLat - mapelmap[i].lolat;
 SubLon = mapelmap[i].lolong - HighLon;
     
 HighCol = SubLat / mapelmap[i].steplat;
 LowRow = SubLon / mapelmap[i].steplong;
     
 if (LowRow == HighRow || LowCol == HighCol)
  {
  error = 3;
  goto EndFix;
  }

 if (LowRow > HighRow)
  swmem(&LowRow, & HighRow, sizeof (short));
 if (LowCol > HighCol)
  swmem(&LowCol, & HighCol, sizeof (short));

 Rows = HighRow - LowRow;
 Cols = HighCol - LowCol;

 Data = (short *)get_Memory(Rows * Cols * sizeof (short), MEMF_ANY);
 Backup = (short *)get_Memory(Rows * Cols * sizeof (short), MEMF_ANY);

 if (! Data || ! Backup)
  {
  error = 1;
  goto EndFix;
  }

 for (Lr=0; Lr<Rows; Lr++)
  {
  for (Lc=0; Lc<Cols; Lc++)
   {
   Data[Lr * Cols + Lc] =
	 mapelmap[i].map[(LowRow + Lr) * mapelmap[i].columns + LowCol + Lc];
   } /* for Lc=0... */
  } /* for Lr=0... */

 memcpy(Backup, Data, Rows * Cols * sizeof (short));

/* Find points of equal elevation and set to -30000 */

 EqualPts = 2;
 PtRange = 3;
 T = 0.0;

ResetPoints:

 sprintf(str, "%d", EqualPts);
 if (! GetInputString("Enter minimum matching points.",
	 "+-.,abcdefghijklmnopqrstuvwxyz9", str))
  {
  error = 10;
  goto EndFix;
  } /* if cancel */
 EqualPts = atoi(str);
 sprintf(str, "%d", PtRange);
 if (! GetInputString("Enter elevation tolerance.",
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
  {
  error = 10;
  goto EndFix;
  } /* if cancel */
 PtRange = atoi(str);

 for (Lr=2; Lr<Rows-2; Lr++)
  {
  zip = Lr * Cols + 2;
  for (Lc=2; Lc<Cols-2; Lc++, zip++)
   {
   Pts = 0;
   if (abs(Data[zip] - Data[zip + Cols]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip + Cols + 1]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip + 1]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip - Cols + 1]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip - Cols]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip - Cols - 1]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip - 1]) <= PtRange)
       Pts ++;
   if (abs(Data[zip] - Data[zip + Cols - 1]) <= PtRange)
       Pts ++;
   if (Pts >= EqualPts)
       Data[zip] = -30000;
   } /* for Lc=0... */
  } /* for Lr=0... */

/* copy modified data back to map and display */

 for (Lr=0; Lr<Rows; Lr++)
  {
  for (Lc=0; Lc<Cols; Lc++)
   {
   mapelmap[i].map[(LowRow + Lr) * mapelmap[i].columns + LowCol + Lc] =
     Data[Lr * Cols + Lc];
   } /* for Lc=0... */
  } /* for Lr=0... */
 makemap(MapWind0, (long)Bx->Low.X, (long)Bx->Low.Y, (long)Bx->High.X, (long)Bx->High.Y,
 (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0) | MMF_REFINE | MMF_FLATSPOTS);
/* ask if ready to continue or reset points */

 if ((ans = User_Message_Def((CONST_STRPTR)"Mapping Module: Fix Flats",
         (CONST_STRPTR)"Proceed or reset points?", (CONST_STRPTR)"Proceed|Reset|Cancel", (CONST_STRPTR)"prc", 1)) == 0)
  {
  error = 10;
  goto EndFix;
  } /* if cancel */
 if (ans == 2)
  {
  memcpy(Data, Backup, Rows * Cols * sizeof (short));
  goto ResetPoints;
  } /* if reset points */

/* Go fix the flat spots and redraw */
/* This didn't seem to contribute much and it was one more unneeded requester
 sprintf(str, "%1.1f", T);
 if (! GetInputString("Enter real number for tension value.",
	 ",abcdefghijklmnopqrstuvwxyz", str))
  {
  error = 10;
  goto EndFix;
  } // if cancel
 T = atof(str);
*/
 if (FixFlatSpots(Data, Rows, Cols, T))
  {
  for (Lr=0; Lr<Rows; Lr++)
   {
   for (Lc=0; Lc<Cols; Lc++)
    {
    mapelmap[i].map[(LowRow + Lr) * mapelmap[i].columns + LowCol + Lc] =
     Data[Lr * Cols + Lc];
    } /* for Lc=0... */
   } /* for Lr=0... */
  makemap(MapWind0, (long)Bx->Low.X, (long)Bx->Low.Y, (long)Bx->High.X, (long)Bx->High.Y,
   (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0) | MMF_REFINE);

  if ((ans = User_Message_Def((CONST_STRPTR)"Mapping Module: Fix Flats",
          (CONST_STRPTR)"Keep or save DEM or reset parameters?", (CONST_STRPTR)"Keep|Save|Reset|Cancel", (CONST_STRPTR)"ksrc", 1)) > 0)
   {
   if (ans == 3)
    {
    memcpy(Data, Backup, Rows * Cols * sizeof (short));
    goto ResetPoints;
    } /* if reset and try again */
   if (ans == 2)
    {
    OpenOK = 0;

    DLItem = DL;
    while (DLItem)
     {
     strmfp(filename, DLItem->Name, DBase[TopoOBN[i]].Name);
     strcat(filename, ".elev");
     if ((fhelev = open(filename, O_RDONLY, 0)) == -1)
      {
      DLItem = DLItem->Next;
      } /* if open/read fails */
     else
      {
      close (fhelev);
      if ((fhelev = open(filename, flags, protflags)) != -1)
       OpenOK = 1;
      break;
      } /* else open succeeds */
     } /* while */

    if (! OpenOK)
     {
     error = 5;
     } /* if file not found */
    else
     {
     FindElMaxMin(&mapelmap[i], mapelmap[i].map);
     write(fhelev, (char *)&Version, 4);
     if (write(fhelev, (char *)&mapelmap[i], ELEVHDRLENV101) != ELEVHDRLENV101)
      {
      error = 6;
      } /* if error writing header to file */
     else
      {
      if (write(fhelev,(char *)mapelmap[i].map, mapelmap[i].size) != mapelmap[i].size)
       {
       error = 6;
       } /* if error writing map to file */
      else
       Log(MSG_DEM_SAVE, (CONST_STRPTR)DBase[TopoOBN[i]].Name);
      } /* else no header write error */
     close(fhelev);
     } /* else file found */
    } /* if save */
   } /* if keep or save */
  else
   {
   error = 10;
   } /* else not keep */
  } /* if fixed OK */

EndFix:

 if (Data)
  free_Memory(Data, Rows * Cols * sizeof (short));
 if (Backup)
  {
  if (error == 10)
   {
   for (Lr=0; Lr<Rows; Lr++)
    {
    for (Lc=0; Lc<Cols; Lc++)
     {
     mapelmap[i].map[(LowRow + Lr) * mapelmap[i].columns + LowCol + Lc] =
      Backup[Lr * Cols + Lc];
     } /* for Lc=0... */
    } /* for Lr=0... */
   makemap(MapWind0, (long)Bx->Low.X, (long)Bx->Low.Y, (long)Bx->High.X, (long)Bx->High.Y,
    (topo ? MMF_TOPO : 0) | (ecoenabled ? MMF_ECO : 0) | MMF_REFINE);
   } /* if changes cancelled */
  free_Memory(Backup, Rows * Cols * sizeof (short));
  } /* if Backup */

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* out of memory */
  case 2:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"All corner points must be within topo map boundaries!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* one corner outside mapped area */
  case 3:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"Illegal dimensions! Try making the rectangle larger.\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* low = high row or column */
  case 4:
   {
   User_Message((CONST_STRPTR)"Mapping Module: Fix Flats",
           (CONST_STRPTR)"All corner points must be within same DEM!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* corners no on same map */
  case 5:
   {
   User_Message((CONST_STRPTR)DBase[TopoOBN[i]].Name,
           (CONST_STRPTR)"Error opening output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)DBase[TopoOBN[i]].Name);
   break;
   } /* file open fail */
  case 6:
   {
   User_Message((CONST_STRPTR)DBase[TopoOBN[i]].Name,
           (CONST_STRPTR)"Error writing to output file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRITE_FAIL, (CONST_STRPTR)DBase[TopoOBN[i]].Name);
   break;
   } /* file write fail */
  case 10:
   {
   break;
   } /* changes cancelled */
  } /* switch */

} /* FlatFixer() */

/***********************************************************************/

#define PATH_VECTOR 		0x0000
#define PATH_MOTION 		0x0001
#define PATH_MOTION_CAMERA	0x0011
#define PATH_MOTION_FOCUS	0x0021

short BuildElevTable(double **ElTable, short Frames, float *MaxMin,
	short PathSource)
{
 short i, j, LatPts = 0, LonPts = 0, Base;
 long Row, Col;
 double *Lat, *Lon, *Table, Flat, Datum, Exag;

 if (! topoload)
  if (loadtopo())
   return (0);

 if (topomaps < 1)
  return (0);

 if (PathSource == PATH_VECTOR)
  {
  if (! (DBase[OBN].Lat && DBase[OBN].Lon))
   return (0);
  Lat = DBase[OBN].Lat;
  Lon = DBase[OBN].Lon;
  LatPts = LonPts = 1;
  Base = 1;
  }
 else if (PathSource & PATH_MOTION)
  {
  if (! BuildKeyTable())
   return (0);
  Base = 0;
  if (PathSource == PATH_MOTION_CAMERA)
   {
   if (KT[1].Key)
    {
    Lat = KT[1].Val[0];
    LatPts = 1;
    }
   else
    Lat = &PAR_FIRST_MOTION(1);
   if (KT[2].Key)
    {
    Lon = KT[2].Val[0];
    LonPts = 1;
    }
   else
    Lon = &PAR_FIRST_MOTION(2);
   } /* camera path */
  else
   {
   if (KT[4].Key)
    {
    Lat = KT[4].Val[0];
    LatPts = 1;
    }
   else
    Lat = &PAR_FIRST_MOTION(4);
   if (KT[5].Key)
    {
    Lon = KT[5].Val[0];
    LonPts = 1;
    }
   else
    Lon = &PAR_FIRST_MOTION(5);
   } /* focus path */
  }
 else
  return (0);
  
 if ((Table = (double *)get_Memory((Frames + 1) * sizeof (double), MEMF_CLEAR))
	== NULL)
  return (0);

 MaxMin[0] = -30000.0;
 MaxMin[1] = 30000.0;

 for (i=Base; i<=Frames; i++)
  {
  for (j=0; j<topomaps; j++)
   {
   if (LonPts)
    Row = (mapelmap[j].lolong - Lon[i]) / mapelmap[j].steplong;
   else
    Row = (mapelmap[j].lolong - *Lon) / mapelmap[j].steplong;
   if (LatPts) 
    Col = (Lat[i] - mapelmap[j].lolat) / mapelmap[j].steplat;
   else 
    Col = (*Lat - mapelmap[j].lolat) / mapelmap[j].steplat;

   if (Row >= 0 && Row <= mapelmap[j].rows
	 && Col >= 0 && Col < mapelmap[j].columns)
    {
    Table[i] = mapelmap[j].map[Row * mapelmap[j].columns + Col]
		* mapelmap[j].elscale;
    if (PathSource & PATH_MOTION)
     {
     if (KT[12].Key)
      Flat = KT[12].Val[0][i];
     else
      Flat = PAR_FIRST_MOTION(12);
     if (KT[13].Key)
      Datum = KT[13].Val[0][i];
     else
      Datum = PAR_FIRST_MOTION(13);
     if (KT[14].Key)
      Exag = KT[14].Val[0][i];
     else
      Exag = PAR_FIRST_MOTION(14);

     Table[i] += ((Datum - Table[i]) * Flat);
     Table[i] *= Exag;
     } /* if motion path */
    break;
    } /* if point within map area */
   } /* for j=0... */
  if (Table[i] > MaxMin[0])
   MaxMin[0] = Table[i];
  if (Table[i] < MaxMin[1])
   MaxMin[1] = Table[i];
  } /* for i=... */

 *ElTable = Table;

 if (KT)
  FreeKeyTable();

 return (1);

} /* BuildElevTable() */

/**********************************************************************/

short BuildVelocityTable(short Foc, double **ElTable, short Frames, float *MaxMin)
{
 short i, LatPts = 0, LonPts = 0, AltPts = 0, Base;
 float Max, Min;
 double *Lat = NULL, *Lon = NULL, *Alt = NULL, *Table, lonscale, xxx, yyy, zzz
/*	, ScaleFactor = 1.0*/;

 if (! BuildKeyTable())
  return (0);

/* assuming function only operates on Key Framed Motion Paths, not Vectors */
 Base = 0;

 Foc *=3;

 if (KT[0 + Foc].Key)
  {
  Alt = KT[0 + Foc].Val[0];
  AltPts = 1;
  }
 if (KT[1 + Foc].Key)
  {
  Lat = KT[1 + Foc].Val[0];
  LatPts = 1;
  }
 else
  lonscale = LATSCALE * cos(PAR_FIRST_MOTION(1 + Foc) * PiOver180);
 if (KT[2 + Foc].Key)
  {
  Lon = KT[2 + Foc].Val[0];
  LonPts = 1;
  }

 if (*ElTable == NULL)
  {  
  if ((Table = (double *)get_Memory((Frames + 1) * sizeof (double), MEMF_CLEAR))
	== NULL)
   return (0);
  } /* if new table */
 else
  Table = *ElTable;

 Max = -30000.0;
 Min = 30000.0;

 for (i=Base; i<Frames; i++)
  {
  if (LatPts)
   lonscale = LATSCALE * cos(((Lat[i] + Lat[i + 1]) / 2.0) * PiOver180);

  if (LonPts)
   xxx = (Lon[i + 1] - Lon[i]) * lonscale;
  else
   xxx = 0.0;
  if (AltPts) 
   yyy = Alt[i + 1] - Alt[i];
  else 
   yyy = 0.0;
  if (LatPts) 
   zzz = (Lat[i + 1] - Lat[i]) * LATSCALE;
  else 
   zzz = 0.0;

  Table[i] = sqrt(xxx * xxx + yyy * yyy + zzz * zzz);

  if (Table[i] > Max)
   Max = Table[i];
  if (Table[i] < Min)
   Min = Table[i];
  } /* for i=... */
/*
 if (Max != Min && MaxMin[0] != MaxMin[1])
  ScaleFactor = (MaxMin[0] - MaxMin[1]) / (Max - Min);
*/
 for (i=Base; i<Frames; i++)
  {
  Table[i] = (Table[i] - Min)/* * ScaleFactor*/ + MaxMin[1];
  if (Table[i] > MaxMin[0])
   MaxMin[0] = Table[i];
  } /* for i=... */

 Table[Frames] = Table[Frames - 1];

 *ElTable = Table;

 if (KT)
  FreeKeyTable();

 return (1);

} /* BuildVelocityTable() */

/************************************************************************/

short DistributeVelocity(struct VelocityDistr *Vel) 
{
 short i, error = 0, pt;
 long TSize;
 double *TLat, *TLon, *TAlt, *Lngth, sum, lonscale, DeltaLat, DeltaLon, DeltaAlt,
	distance, pathdist, framedist, remainder, shift0, shift1, portion;

 if (! Vel->Lat && ! Vel->Lon && ! Vel->Alt)
  return (0);
 if (Vel->PtsIn < 2 || Vel->PtsOut < 2)
  return (0);

/* get memory */
 TSize = (Vel->PtsIn + 1) * sizeof (double);
 TLat = (double *)get_Memory(TSize, MEMF_CLEAR);
 TLon = (double *)get_Memory(TSize, MEMF_CLEAR);
 TAlt = (double *)get_Memory(TSize, MEMF_CLEAR);
 Lngth = (double *)get_Memory(TSize, MEMF_CLEAR);

 if (! TLat || ! TLon || ! TAlt || ! Lngth)
  {
  error = 1;
  goto EndDist;
  }

/* TLat & TLon & TAlt are lat/lon/alt distances relative to pt 1 */
 TLat[Vel->Base] = TLon[Vel->Base] = TAlt[Vel->Base] = 0.0;
 lonscale = cos(PAR_FIRST_MOTION(1) * PiOver180) * LATSCALE;
 for (i=Vel->Base+1; i<=Vel->PtsIn; i++)
  {
  if (Vel->Lat)
   {
   TLat[i] = TLat[i - 1] + (Vel->Lat[i] - Vel->Lat[i - 1]) * LATSCALE;
   lonscale = cos(((Vel->Lat[i] + Vel->Lat[i - 1]) * .5) * PiOver180) * LATSCALE;
   }
  if (Vel->Lon)
   TLon[i] = TLon[i - 1] + (Vel->Lon[i] - Vel->Lon[i - 1]) * lonscale;
  if (Vel->Alt)
   TAlt[i] = TAlt[i - 1] + (Vel->Alt[i] - Vel->Alt[i - 1]);
  } /* for i=2... */

 sum = 0.0;
 for (i=Vel->Base; i<Vel->PtsIn; i++)
  {
  DeltaLat = TLat[i + 1] - TLat[i];
  DeltaLon = TLon[i + 1] - TLon[i];
  DeltaAlt = TAlt[i + 1] - TAlt[i];
  Lngth[i] = sqrt(DeltaLat * DeltaLat + DeltaLon * DeltaLon + DeltaAlt * DeltaAlt);
  sum += Lngth[i];
  } /* for i=1... */

 shift1 = sum / (Vel->PtsOut - Vel->Base - Vel->EaseIn / 2.0 - Vel->EaseOut / 2.0);

 framedist = 0.0;
 pathdist = 0.0;
 remainder = 0.0;
 pt = 0;
 for (i=Vel->Base+1; i<Vel->PtsOut; i++)
  {
  if (i < Vel->Base + Vel->EaseIn)
   shift0 = i / ((float)Vel->EaseIn + Vel->Base * 1.0);
  else if (i >= Vel->PtsOut - Vel->EaseOut)
   shift0 = ((float)Vel->PtsOut - i) / ((float)Vel->EaseOut + Vel->Base * 1.0);
  else
   shift0 = 1.0;
  framedist += shift0 * shift1;
  if (framedist > sum) break;
  distance = pathdist + remainder;
  if (distance >= framedist)
   {
   portion = (framedist - pathdist) / (distance - pathdist);
   if (Vel->Lon)
    Vel->Lon[i] = Vel->Lon[i - 1] + (TLon[pt + 1] - Vel->Lon[i - 1]) * portion;
   if (Vel->Lat)
    Vel->Lat[i] = Vel->Lat[i - 1] + (TLat[pt + 1] - Vel->Lat[i - 1]) * portion;
   if (Vel->Alt)
    Vel->Alt[i] = Vel->Alt[i - 1] + (TAlt[pt + 1] - Vel->Alt[i - 1]) * portion;
   remainder = (distance - pathdist) * (1.0 - portion);
   pathdist = framedist;
   } /* if */
  else
   {
   while (distance < framedist && pt < Vel->PtsIn - 1)
    {
    pt ++;
    pathdist = distance;
    distance += Lngth[pt];
    } /* while */
   portion = (framedist - pathdist) / Lngth[pt];
   if (Vel->Lon)
    Vel->Lon[i] = TLon[pt] + (TLon[pt + 1] - TLon[pt]) * portion;
   if (Vel->Lat)
    Vel->Lat[i] = TLat[pt] + (TLat[pt + 1] - TLat[pt]) * portion;
   if (Vel->Alt)
    Vel->Alt[i] = TAlt[pt] + (TAlt[pt + 1] - TAlt[pt]) * portion;
   remainder = Lngth[pt] * (1.0 - portion);
   pathdist = framedist;
   } /* else */
  } /* for i=2... */
 if (Vel->Lon)
  Vel->Lon[Vel->PtsOut] = TLon[Vel->PtsIn];
 if (Vel->Lat)
  Vel->Lat[Vel->PtsOut] = TLat[Vel->PtsIn];
 if (Vel->Alt)
  Vel->Alt[Vel->PtsOut] = TAlt[Vel->PtsIn];

 for (i=Vel->Base+1; i<=Vel->PtsOut; i++)
  {
  if (Vel->Lat)
   {
   Vel->Lat[i] = Vel->Lat[Vel->Base] + Vel->Lat[i] / LATSCALE;
   lonscale = LATSCALE * cos(Vel->Lat[i] * PiOver180);
   }
  if (Vel->Lon)
   Vel->Lon[i] = Vel->Lon[Vel->Base] + Vel->Lon[i] / lonscale;
  if (Vel->Alt)
   Vel->Alt[i] = Vel->Alt[Vel->Base] + Vel->Alt[i];
  } /* for i=2... */

EndDist:

 if (TLat)
  free_Memory(TLat, TSize);
 if (TLon)
  free_Memory(TLon, TSize);
 if (TAlt)
  free_Memory(TAlt, TSize);
 if (Lngth)
  free_Memory(Lngth, TSize);

 return( (short)(! error));

} /* DistributeVelocity() */

/*************************************************************************/

STATIC_FCN void InitGauss(struct Gauss *Gauss) // used locally only -> static, AF 23.7.2021
{

 Gauss->Nrand = 4;
 Gauss->Arand = 1.0;
 Gauss->Add = sqrt(3.0 * Gauss->Nrand);				/* = 3.46410 */
 Gauss->Fac = 2.0 * Gauss->Add / (Gauss->Nrand * Gauss->Arand); /* = 1.73205 */
AF_DEBUG_f_f("Gauss->Add Gauss->Fac",Gauss->Add,Gauss->Fac);
AF_DEBUG_ld("Gauss->Seed",Gauss->Seed);
 srand48(Gauss->Seed);

} /* InitGauss() */

/************************************************************************/

STATIC_FCN double DoGauss(struct Gauss *Gauss) // used locally only -> static, AF 23.7.2021
{
short i;
double sum = 0.0;

 for (i=0; i<Gauss->Nrand; i++)
  {
  sum += drand48();
  } /* for i=0... */
 return (Gauss->Fac * sum - Gauss->Add);

} /* DoGauss() */

/***********************************************************************/

short Raster_Fract(float *Rast, long N1, long N2, long Seed, double delta,
	double H, short MaxStage)
{ /* N1 = width, N2 = height */
short success = 1;
long Np1, Stage, Addition, x, y, D, d;
struct Gauss Gauss;
struct BusyWindow *BWMD;

AF_DEBUG("");

 Gauss.Seed = Seed;

 Addition = 1;

 InitGauss(&Gauss);

/* begin */
 
 Np1 = N1;
 N1 --;
 N2 --;

 D = pow(2.0, (double)MaxStage);
 d = D / 2;

 BWMD = BusyWin_New("Computing...", MaxStage, 0, MakeID('B','W','M','D'));

 for (Stage=0; Stage<MaxStage; Stage++)
  {

  delta *= pow(.5, .5 * H);
  for (x=d; x<=N2-d; x+=D)
   {
   for (y=d; y<=N1-d; y+=D)
    {
    Rast[x * Np1 + y] = f4(delta, Rast[(x + d) * Np1 + y + d],
				 Rast[(x + d) * Np1 + y - d],
				 Rast[(x - d) * Np1 + y + d],
				 Rast[(x - d) * Np1 + y - d]);
    } /* for y=d... */
   } /* for x=d... */

  if (Addition)
   {
   for (x=0; x<=N2; x+=D)
    {
    for (y=0; y<=N1; y+=D)
     {
     Rast[x * Np1 + y] += (delta * DoGauss(&Gauss));
     } /* for y=0... */
    } /* for x=0... */
   } /* if */

  delta *= pow(.5, .5 * H);
  for (x=d; x<=N2-d; x+=D)
   {
   Rast[x * Np1] = 	f3(delta, Rast[(x + d) * Np1],
				  Rast[(x - d) * Np1],
				  Rast[(x    ) * Np1 + d]);
   Rast[x * Np1 + N1] = f3(delta, Rast[(x + d) * Np1 + N1],
				  Rast[(x - d) * Np1 + N1],
				  Rast[(x    ) * Np1 + N1 - d]);
   } /* for x=d... */

  for (y=d; y<=N1-d; y+=D)
   {
   Rast[y] = 		f3(delta, Rast[y + d],
				  Rast[y - d],
				  Rast[(d    ) * Np1 + y]);
   Rast[N2 * Np1 + y] = f3(delta, Rast[(N2) * Np1 + y + d],
				  Rast[(N2) * Np1 + y - d],
				  Rast[(N2 - d) * Np1 + y]);
   } /* for x=d... */

  for (x=d; x<=N2-d; x+=D)
   {
   for (y=D; y<=N1-D; y+=D)
    {
    Rast[x * Np1 + y] = f4(delta, Rast[(x) * Np1 + y + d],
				  Rast[(x) * Np1 + y - d],
				  Rast[(x + d) * Np1 + y],
				  Rast[(x - d) * Np1 + y]);
    } /* for y=D... */
   } /* for x=d... */

  for (x=D; x<=N2-D; x+=D)
   {
   for (y=d; y<=N1-d; y+=D)
    {
    Rast[x * Np1 + y] = f4(delta, Rast[(x) * Np1 + y + d],
				  Rast[(x) * Np1 + y - d],
				  Rast[(x + d) * Np1 + y],
				  Rast[(x - d) * Np1 + y]);
    } /* for y=d... */
   } /* for x=D... */
  /* ALEXANDER: Addition ist nicht das Problem. */
  if (Addition)
   {
   for (x=0; x<=N2; x+=D)
    {
    for (y=0; y<=N1; y+=D)
     {
     Rast[x * Np1 + y] += (delta * DoGauss(&Gauss));
     } /* for y=0... */
    } /* for x=0... */

   for (x=d; x<=N2-d; x+=D)
    {
    for (y=d; y<=N1-d; y+=D)
     {
     Rast[x * Np1 + y] += (delta * DoGauss(&Gauss));
     } /* for y=d... */
    } /* for x=d... */
   } /* if */

  D /= 2;
  d /= 2;

  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   success = 0;
   break;
   } /* if user abort */
  BusyWin_Update(BWMD, Stage + 1);
  } /* for Stage=0... */

 BusyWin_Del(BWMD);

 return (success);

} /* Raster_Fract() */

/***********************************************************************/

void FractalMap_Draw(void)
{
char filename[256];
short i, OpenOK, col;
long x, y, Low_X, Low_Y, High_X, High_Y, zip, RowZip, Row, Col;
double DataRow, DataCol, LatStep, LonStep;
FILE *fFrd;
struct BusyWindow *BWMD;
struct clipbounds cb;
struct DirList *DLItem;

 setclipbounds(MapWind0, &cb);

 BWMD = BusyWin_New("Drawing...", topomaps, 0, MakeID('B','W','M','D'));

 for (i=0; i<topomaps; i++)
  {
  OpenOK = 0;

  DLItem = DL;
  while (DLItem)
   {
   strmfp(filename, DLItem->Name, DBase[TopoOBN[i]].Name);
   strcat(filename, ".elev");
   if ((fFrd = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    } /* if DEM open fails */
   else
    {
    fclose(fFrd);
    OpenOK = 1;
    break;
    } /* else open succeeds */
   } /* while */

  if (OpenOK)
   {
   strmfp(filename, dirname, DBase[TopoOBN[i]].Name);
   strcat(filename, ".frd");
   if ((fFrd = fopen(filename, "rb")))
    {
    mapelmap[i].fractalsize = mapelmap[i].rows * (mapelmap[i].columns - 1) * 2;
    if ((mapelmap[i].fractal = (BYTE *)get_Memory(mapelmap[i].fractalsize, MEMF_ANY)))
     {
     if (fread(mapelmap[i].fractal, mapelmap[i].fractalsize, 1, fFrd) == 1)
      {
      latlon_XY(TopoOBN[i]);
      Low_X = mapxx[3];
      Low_Y = mapyy[3];
      High_X = mapxx[1];
      High_Y = mapyy[1];
      LatStep = ((double)(mapelmap[i].columns - 2)) / ((double)(High_Y - Low_Y));
      LonStep = ((double)(mapelmap[i].rows - 1)) / ((double)(High_X - Low_X));
      DataRow = mapelmap[i].columns - 2;
      for (y=Low_Y; y<=High_Y; y++, DataRow-=LatStep)
       {
       if (y < cb.lowy)
        continue;
       if (y > cb.highy)
        break;
       Row = DataRow;
       RowZip = Row * 2;
       DataCol = 0.0;
       for (x=Low_X; x<=High_X; x++, DataCol+=LonStep)
        {
        if (x < cb.lowx)
         continue;
        if (x > cb.highx)
         break;
        Col = DataCol;
        zip = RowZip + Col * (mapelmap[i].columns-1) * 2;
        col = 14 - mapelmap[i].fractal[zip];
        SetAPen(MapWind0->RPort, col);
        WritePixel(MapWind0->RPort, x, y);
        } /* for x=... */
       } /* for y=... */
      } /* if read OK */
     free_Memory(mapelmap[i].fractal, mapelmap[i].fractalsize);
     mapelmap[i].fractal = NULL;
     } /* if memory OK */
    fclose(fFrd);
    } /* if fractal file opened */
   } /* if DEM file found */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   break;
   } /* if user abort */
  if (BWMD)
   BusyWin_Update(BWMD, i + 1);

  } /* for i=0... */

 if (BWMD)
  BusyWin_Del(BWMD);

} /* FractalMap_Draw() */

/***********************************************************************/
#ifdef JHGSJDGSHGDJSDGJS
typedef double TDA[3];
typedef double Matx3x3[3][3];

struct Branch {
 double ScrnX, ScrnY;
 TDA N;
 Matx3x3 Rot;
 double L, D, A, Ar, Ad;
 struct Branch *Prev;
 struct Branch *Next;
};

struct TreeData {
 double BaseDiam, MinDiam, BranchAngle, NodeDivg, DiamLength,
	RangeVar, ForkTend, DieOff, Attrition;
 short Branches;
};

void ParticleTree(void);
short NodeBranch(struct Branch *BR, struct TreeData *TR, struct Window *win,
	struct clipbounds *cb, struct lineseg *ls,
	double ScrnBaseX, double ScrnBaseY);
struct Branch *Branch_New(void);
void Branch_Del(struct Branch *BR);
void RotationMatrix(double Ry, double Rz, Matx3x3 RMatx);
void Rotate3D(short m, double theta, Matx3x3 A);
//void ZeroMatrix3x3(Matx3x3 A);
void ZeroTDA(TDA A);
//void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C);
void RotateTreeNode(struct Branch *BR);
void ConvolveMatrices(TDA A, Matx3x3 M);
double RandomizeDimension(double CentralVal, double Variability,
	double MaxSize, double MinSize);

void ParticleTree(void)
{
double ScrnBaseX, ScrnBaseY;
struct TreeData TR;
struct Branch *BR = NULL;
struct clipbounds cb;
struct lineseg ls;

 if (! MapWind0)
  return;

/* set user variables - initial trunk diameter, minimum branch diameter,
	branching angle, node divergence, avg branch/trunk area fraction,
	random variability */

 TR.BaseDiam = 1.0;
 TR.MinDiam = .01;
 TR.BranchAngle = 20.0;
 TR.NodeDivg = 5.0;
 TR.DiamLength = 10.0;
 TR.RangeVar = .25;
 TR.ForkTend = .05;
 TR.Branches = 2;
 TR.DieOff = .1;
 TR.Attrition = .1;

	/* convert angles to radians */

 TR.NodeDivg *= PiOver180;
 TR.BranchAngle *= PiOver180;

	/* NodeDistance is measured in branch diameters */


/* Initialize first branch structure */

 if ((BR = Branch_New()) != NULL)
  {

  while (1)
   {

/* find start point and initial angle */

   BR->ScrnX = (MapWind0->Width / 2);
   BR->ScrnX += (BR->ScrnX * (2 * drand48() - 1.0));

   BR->ScrnY = MapWind0->Height - 1 - MapWind0->BorderBottom;
   ScrnBaseX = BR->ScrnX;
   ScrnBaseY = BR->ScrnY;
   BR->N[0] = BR->N[1] = BR->N[2] = 0.0;
   BR->Ad = RandomizeDimension(TR.NodeDivg, TR.RangeVar, HalfPi / 2.0, 0.0);
   BR->Ar = TwoPi * drand48();
   BR->D = RandomizeDimension(TR.BaseDiam, TR.RangeVar, 1000.0, 0.0);
   BR->A = BR->D * .5;
   BR->A = BR->A * BR->A * Pi;

/* recursively add nodes and draw segments */

   NodeBranch(BR, &TR, MapWind0, &cb, &ls, ScrnBaseX, ScrnBaseY);

   if (! User_Message_Def("Particle Tree", "Do another tree?", "Yes|No", "yn", 1))
    break;
   } /* while */

/* free memory */

  Branch_Del(BR);
  } /* if memory allocated */
 else
  {
  User_Message_Def("Particle Tree", 
	"Out of memory allocating new branch!\nOperation terminated.",
	"OK", "o", 0);
  } /* else */

} /* ParticleTree() */

/**********************************************************************/

short NodeBranch(struct Branch *BR, struct TreeData *TR, struct Window *win,
	struct clipbounds *cb, struct lineseg *ls,
	double ScrnBaseX, double ScrnBaseY)
{
short i, BranchAlive;

/* inputs include the xyz position of the node, vector to next node,
	length of branch, diameter - compute the next node */

 if (BR->D > TR->MinDiam)
  {
  if (BR->Next = Branch_New())
   {
   struct Branch *TB;
   short Fork;
   double AreaRem;

   TB = BR->Next;
   TB->Prev = BR;

   AreaRem = BR->A;

   Fork = (drand48() < TR->ForkTend);

   for (i=0; i<TR->Branches; i++)
    {
    if (i == 0 || Fork)
     {
     TB->A = RandomizeDimension(BR->A * (1.0 - TR->Attrition), TR->RangeVar,
		AreaRem, .5 * AreaRem);
     AreaRem -= TB->A;
     if (BranchAlive = (drand48() > TR->DieOff * .1))
      {
      TB->Ar = TwoPi * drand48();
      TB->Ad = RandomizeDimension(TR->NodeDivg, TR->RangeVar,
		HalfPi, 0.0);
      } /* if */
     } /* if main branch */
    else
     {
     TB->A = RandomizeDimension(AreaRem, TR->RangeVar,
		AreaRem, 0.0);
     AreaRem -= TB->A;
     if (BranchAlive = (drand48() > TR->DieOff))
      {
      TB->Ar = TwoPi * drand48();
      TB->Ad = RandomizeDimension(TR->BranchAngle, TR->RangeVar,
		HalfPi, 5.0);
      } /* if */
     } /* else auxiliary branch */
    if (BranchAlive)
     {
     TB->D = 2.0 * sqrt(TB->A / Pi);
     TB->L = RandomizeDimension(TB->D * TR->DiamLength, TR->RangeVar,
		1000.0, 0.0);
     ZeroTDA(TB->N);
     RotationMatrix(TB->Ar, TB->Ad, TB->Rot);
     RotateTreeNode(TB);
/* compute screen coords of new branch node */

     TB->ScrnX = ScrnBaseX + TB->N[0] * 2.0;
     TB->ScrnY = ScrnBaseY - TB->N[1] * 2.0;
     
/* draw the new branch */
     setclipbounds (win, cb);
     setlineseg(ls, BR->ScrnX, BR->ScrnY, TB->ScrnX, TB->ScrnY);
     SetAPen(win->RPort, 12);
     ClipDraw(win, cb, ls);

/* recurse to add new branches */
     if (! NodeBranch(TB, TR, win, cb, ls, ScrnBaseX, ScrnBaseY))
      {
      Branch_Del(TB);
      return (0);
      } /* if memory failed */
     } /* if alive */
    } /* for i=0... */
   Branch_Del(TB);
   } /* if new branch created */
  else
   return (0);
  } /* if need another branch */
 else
  {
  return (1);
  } /* else */

} /* NodeBranch() */

/**********************************************************************/

struct Branch *Branch_New(void)
{

 return ((struct Branch *)get_Memory(sizeof (struct Branch), MEMF_CLEAR));

} /* Branch_New() */

/**********************************************************************/

void Branch_Del(struct Branch *BR)
{

 if (BR)
  free_Memory(BR, sizeof (struct Branch));

} /* Branch_Del() */

/**********************************************************************/

void RotationMatrix(double Ry, double Rz, Matx3x3 RMatx)
{
Matx3x3 YRot, ZRot;

 Rotate3D(2, Ry, YRot);
 Rotate3D(3, Rz, ZRot);
 Multiply3x3Matrices(ZRot, YRot, RMatx);

} /* Rotationmatrix() */

/**********************************************************************/

void Rotate3D(short m, double theta, Matx3x3 A)
{

 short m1, m2;
 double c, s;

 ZeroMatrix3x3(A);
 A[m - 1][m - 1] = 1.0;
 m1 = (m % 3) + 1;
 m2 = m1 % 3;
 m1 --;
 c = cos(theta);
 s = sin(theta);
 A[m1][m1] = c;
 A[m1][m2] = s;
 A[m2][m2] = c;
 A[m2][m1] = -s;

} /* Rotate3D() */

/***********************************************************************/

STATIC_FCN void ZeroMatrix3x3(Matx3x3 A) // used locally only -> static, AF 23.7.2021
{
short i, j;

 for (i=0; i<3; i++)
  {
  for (j=0; j<3; j++)
   A[i][j] = 0.0;
  } /* for i=0... */

} /* ZeroMatx3x3() */

/***********************************************************************/

void ZeroTDA(TDA A)
{

 A[0] = A[1] = A[2] = 0.0;

} /* ZeroTDA() */

/***********************************************************************/

STATIC_FCN void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C) // used locally only -> static, AF 23.7.2021
{
short i, j, k;
double ab;

 for (i=0; i<3; i++)
  {
  for (j=0; j<3; j++)
   {
   ab = 0.0;
   for (k=0; k<3; k++)
    ab += A[i][k] * B[k][j];
   C[i][j] = ab;
   } /* for j=0... */
  } /* for i=0... */

} /* Multiply3x3matrices() */

/**********************************************************************/

void RotateTreeNode(struct Branch *BR)
{
struct Branch *Cur;

 Cur = BR;

 while (Cur->Prev)
  {
  BR->N[1] += Cur->L;
  ConvolveMatrices(BR->N, Cur->Rot);
  Cur = Cur->Prev;
  } /* while */

} /* RotateTreeNode() */

/**********************************************************************/

void ConvolveMatrices(TDA A, Matx3x3 M)
{
TDA B;

 B[0] = A[0] * M[0][0] + A[1] * M[0][1] + A[2] * M[0][2];
 B[1] = A[0] * M[1][0] + A[1] * M[1][1] + A[2] * M[1][2];
 B[2] = A[0] * M[2][0] + A[1] * M[2][1] + A[2] * M[2][2];

 memcpy(&A[0], &B[0], sizeof (TDA));

} /* ConvolveMatrices() */

/**********************************************************************/

double RandomizeDimension(double CentralVal, double Variability,
	double MaxSize, double MinSize)
{
double Dimension;

 Dimension = CentralVal + CentralVal * Variability * GaussRand();
 if (Dimension < MinSize)
  Dimension = MinSize;
 else if (Dimension > MaxSize)
  Dimension = MaxSize;

 return (Dimension);

} /* RandomizeDimension() */
#endif
