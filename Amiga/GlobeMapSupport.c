/* GlobeMapSupport.c (ne gisglobemapsupport.c 14 Jan 1994 CXH)
** Some auxiliary calculation and IO routines for mapping in GIS renderer.
** Built (ripped) from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by Gary R. Huber.
*/

#include "WCS.h"
#include <math.h>

/*********************************************************************/
static short HaloEffect(UBYTE **Bitmap, long Width, long Height,
        double Dr, double Dx, double Dy, double Intensity, double NoHaloDist,
        struct ColorComponents *CC, char *NameStr, struct Window *win); // used locally only -> static, AF 19.7.2021
static short Image_Composite(UBYTE **Bitmap, UBYTE **Source, long Iw, long Ih,
        short Sw, short Sh, double Dr, double Dx, double Dy, double Distance,
        double *Visible, double *Luminosity, struct ColorComponents *CC,
        char *NameStr, struct Window *win); // used locally only -> static, AF 23.7.2021
static void getskypt(void); // used locally only -> static, AF 23.7.2021

double ComputeBanking(short frame)
{
 short i, j, ffr, lfr, compfr;
 double angledif, nextx = 0.0, nexty = 0.0, lastx = 0.0, lasty = 0.0,
	lonscale, Banking;

 if (! KT || KT_MaxFrames < 3)		/* no auto banking allowed */
  return (0.0);
 if (frame <= 1)
  compfr = 2;
 else if (frame >= KT_MaxFrames)
  compfr = KT_MaxFrames - 1;
 else
  compfr = frame; 
 ffr = compfr - 6;
 lfr = compfr + 6;
 if (ffr < 2)
  ffr = 2;
 if (lfr > KT_MaxFrames - 1)
  lfr = KT_MaxFrames - 1;
 Banking = 0.0;
 lonscale = LATSCALE * cos(KT[1].Val[0][frame] * PiOver180);

 for (i=ffr; i<=lfr; i++)
  {
  for (j=1; j<9; j++)
   {
   if (i > j)
    {
    if (KT[2].Key)
     lastx = (KT[2].Val[0][i] - KT[2].Val[0][i - j]) * lonscale;
    if (KT[1].Key)
     lasty = (KT[1].Val[0][i] - KT[1].Val[0][i - j]) * LATSCALE;
    } /* if */
   else
    {
    if (KT[2].Key)
     lastx = (KT[2].Val[0][i] - KT[2].Val[0][1]) * lonscale;
    if (KT[1].Key)
     lasty = (KT[1].Val[0][i] - KT[1].Val[0][1]) * LATSCALE;
    } /* else */
   if (i <= KT_MaxFrames - j)
    {
    if (KT[2].Key)
     nextx = (KT[2].Val[0][i + j] - KT[2].Val[0][i]) * lonscale;
    if (KT[1].Key)
     nexty = (KT[1].Val[0][i + j] - KT[1].Val[0][i]) * LATSCALE;
    } /* if */
   else
    {
    if (KT[2].Key)
     nextx = (KT[2].Val[0][KT_MaxFrames] - KT[2].Val[0][i]) * lonscale;
    if (KT[1].Key)
     nexty = (KT[1].Val[0][KT_MaxFrames] - KT[1].Val[0][i]) * LATSCALE;
    } /* else */
   angledif = findangle2(nextx, nexty) - findangle2(lastx, lasty);
   if (angledif > Pi) angledif -= TwoPi;
   else if (angledif < -Pi) angledif += TwoPi;
   Banking += settings.bankfactor * angledif * PiUnder180;
  } /* for j=1... */
  Banking /= 8.0;
 } /* for i=ParHdr.FirstFrame+1... */

 return (Banking);

} /* ComputeBanking() */

/*********************************************************************/

void setview(void)
{

 VP.lat = PARC_RNDR_MOTION(1);
 VP.lon = PARC_RNDR_MOTION(2);
 VP.alt = PARC_RNDR_MOTION(0);
 if (settings.bankturn && KT)
  {
  short i;
  double BankFactor = 0.0;

  for (i=-5; i<=5; i++)
   {
   BankFactor += ComputeBanking(frame + i);
   }
  PARC_RNDR_MOTION(8) += (BankFactor / 11.0);
  } /* if automatic turn banking - multiple frames to smooth out the wobblies */

/* set these values for fixfocus = 0 before converting view point */
 FP.lat = VP.lat;
 FP.lon = VP.lon;
 FP.alt = -EARTHRAD;

 if ((settings.lookahead && KT) || fixfocus)
  {
  if (settings.lookahead && KT)
   {
   short AheadFrame;

   if (frame + settings.lookaheadframes > KT_MaxFrames)
    AheadFrame = KT_MaxFrames;
   else
    AheadFrame = frame + settings.lookaheadframes;
   if (AheadFrame == frame && frame > 1)
    {
    if (KT[1].Key)
     FP.lat = KT[1].Val[0][frame] + (KT[1].Val[0][frame] - KT[1].Val[0][frame - 1]);
    else
     FP.lat = PARC_RNDR_MOTION(1);
    if (KT[2].Key)
     FP.lon = KT[2].Val[0][frame] + (KT[2].Val[0][frame] - KT[2].Val[0][frame - 1]);
    else
     FP.lon = PARC_RNDR_MOTION(2);
    if (KT[0].Key)
     FP.alt = KT[0].Val[0][frame] + (KT[0].Val[0][frame] - KT[0].Val[0][frame - 1]);
    else
     FP.alt = PARC_RNDR_MOTION(0);
    } /* if focus and camera same pt */
   else
    {
    if (KT[1].Key)
     FP.lat = KT[1].Val[0][AheadFrame];
    else
     FP.lat = PARC_RNDR_MOTION(1);
    if (KT[2].Key)
     FP.lon = KT[2].Val[0][AheadFrame];
    else
     FP.lon = PARC_RNDR_MOTION(2);
    if (KT[0].Key)
     FP.alt = KT[0].Val[0][AheadFrame];
    else
     FP.alt = PARC_RNDR_MOTION(0);
    } /* else normal look ahead */
   } /* if lookahead */
  else
   {
   FP.lat = PARC_RNDR_MOTION(4);
   FP.lon = PARC_RNDR_MOTION(5);
   FP.alt = PARC_RNDR_MOTION(3);
   } /* if fixfocus && !settings.focpath && !lookahead */
  } /* if settings.lookahead || settings.focpath */
 if ((FP.lat == VP.lat && FP.lon == VP.lon) || ! fixfocus)
  {
  convertpt(&VP);
  convertpt(&FP);
  findposvector(&FP, &VP);
  yrot = findangle2(FP.x, FP.z);
  rotate(&FP.x, &FP.z, yrot, yrot);
  xrot = findangle2(FP.y, FP.z);
  rotate(&FP.y, &FP.z, xrot, xrot);
  zrot = 0.0;
  } /* if same camera and focus lat/lon */
 else
  {
  DP.lat = FP.lat;
  DP.lon = FP.lon;
  DP.alt = FP.alt + 1000;
  convertpt(&VP);
  convertpt(&FP);
  findposvector(&FP, &VP);
  yrot = findangle2(FP.x, FP.z);
  rotate(&FP.x, &FP.z, yrot, yrot);
  xrot = findangle2(FP.y, FP.z);
  rotate(&FP.y, &FP.z, xrot, xrot);
  convertpt(&DP);
  findposvector(&DP, &VP);
  rotate(&DP.x, &DP.z, yrot, findangle2(DP.x, DP.z));
  rotate(&DP.y, &DP.z, xrot, findangle2(DP.y, DP.z));
  zrot=findangle2(DP.x, DP.y);
  rotate(&DP.x, &DP.y, zrot, zrot);
  } /* else fix focus */

 BuildRotationMatrix(-xrot, -yrot, zrot, NoBankMatx);

/* FP.q = sqrt(FP.x * FP.x + FP.y * FP.y + FP.z * FP.z);*/
 zrot -= (PARC_RNDR_MOTION(8) * PiOver180);

 cosxrot = cos(-xrot);
 cosyrot = cos(-yrot);
 coszrot = cos(-zrot);
 sinxrot = sin(-xrot);
 sinyrot = sin(-yrot);
 sinzrot = sin(-zrot);

 BuildRotationMatrix(-xrot, -yrot, zrot, ScrRotMatx);

/* Sun position */

 DP.alt = 149.0E+6;
 DP.lat = PARC_RNDR_MOTION(15);
 DP.lon = PARC_RNDR_MOTION(16);

/* This call sets the cartesian coordinates of the sun in SP */

 getscrnpt(&SP, NULL);
 VectorMagnitude(&SP);
 UnitVector(&SP);
 memcpy(&SNB, &SP, sizeof (struct coords));
 RotatePoint(&SNB, NoBankMatx);
 NegateVector(&SNB);

 sprintf(str, "Frame %hd  VP.lat=%f, VP.lon=%f, VP.alt=%f\n",
		frame, VP.lat * PiUnder180, VP.lon * PiUnder180, VP.alt);
 Log(DTA_NULL, str);
 sprintf(str, "FP.lat=%f, FP.lon=%f, FP.alt=%f\n",
		FP.lat * PiUnder180, FP.lon * PiUnder180, FP.alt);
 Log(MSG_UTIL_TAB, str);
 sprintf(str, "SP.x=%f, SP.y=%f, SP.z=%f\n",
		SP.x, SP.y, SP.z);
 Log(MSG_UTIL_TAB, str);
 sprintf(str, "Y rot=%f, X rot=%f, Z rot=%f\n",
		yrot * PiUnder180, xrot * PiUnder180, zrot * PiUnder180);
 Log(MSG_UTIL_TAB, str);
 sprintf(str, "Q max=%f, Q focus=%f, Banking=%f\n",
		qmax, FP.q, PARC_RNDR_MOTION(8));
 Log(MSG_UTIL_TAB, str);

} /* setview() */

/*********************************************************************/

short setquickview(void)
{

 VP.lat = PARC_RNDR_MOTION(1);
 VP.lon = PARC_RNDR_MOTION(2);
 VP.alt = PARC_RNDR_MOTION(0);
 if (settings.bankturn && KT)
  {
  short i;
  double BankFactor = 0.0;

  for (i=-5; i<=5; i++)
   {
   BankFactor += ComputeBanking(frame + i);
   }
  PARC_RNDR_MOTION(8) += (BankFactor / 11.0);
  } /* if automatic turn banking - multiple frames to smooth out the wobblies */

 FP.lat = VP.lat;
 FP.lon = VP.lon;
 FP.alt = -EARTHRAD;

 if ((settings.lookahead && KT) || fixfocus)
  {
  if (settings.lookahead && KT)
   {
   short AheadFrame;

   if (frame + settings.lookaheadframes > KT_MaxFrames)
    AheadFrame = KT_MaxFrames;
   else
    AheadFrame = frame + settings.lookaheadframes;
   if (AheadFrame == frame && frame > 1)
    {
    if (KT[1].Key)
     FP.lat = KT[1].Val[0][frame] + (KT[1].Val[0][frame] - KT[1].Val[0][frame - 1]);
    else
     FP.lat = PARC_RNDR_MOTION(1);
    if (KT[2].Key)
     FP.lon = KT[2].Val[0][frame] + (KT[2].Val[0][frame] - KT[2].Val[0][frame - 1]);
    else
     FP.lon = PARC_RNDR_MOTION(2);
    if (KT[0].Key)
     FP.alt = KT[0].Val[0][frame] + (KT[0].Val[0][frame] - KT[0].Val[0][frame - 1]);
    else
     FP.alt = PARC_RNDR_MOTION(0);
    } /* if focus and camera same pt */
   else
    {
    if (KT[1].Key)
     FP.lat = KT[1].Val[0][AheadFrame];
    else
     FP.lat = PARC_RNDR_MOTION(1);
    if (KT[2].Key)
     FP.lon = KT[2].Val[0][AheadFrame];
    else
     FP.lon = PARC_RNDR_MOTION(2);
    if (KT[0].Key)
     FP.alt = KT[0].Val[0][AheadFrame];
    else
     FP.alt = PARC_RNDR_MOTION(0);
    } /* else normal look ahead */
   } /* if lookahead */
  else
   {
   FP.lat = PARC_RNDR_MOTION(4);
   FP.lon = PARC_RNDR_MOTION(5);
   FP.alt = PARC_RNDR_MOTION(3);
   } /* if fixfocus && !lookahead */
  } /* if lookahead or fixfocus */

 if ((FP.lat == VP.lat && FP.lon == VP.lon) || ! fixfocus)
  {
/* Jamie didn't like this
  if (fixfocus)
   {
   if (! User_Message("Parameters Module",
 "Warning!\nCamera and focus at same latitude\
 and longitude coordinates (possibly a result of \"Look Ahead\" Render Setting\
 enabled).\n\
View may be different than expected.", "OK|Cancel", "oc"))
    return (0);
   }
*/
  convertpt(&VP);
  convertpt(&FP);
  findposvector(&FP, &VP);
  yrot = findangle2(FP.x, FP.z);
  rotate(&FP.x, &FP.z, yrot, yrot);
  xrot = findangle2(FP.y, FP.z);
  rotate(&FP.y, &FP.z, xrot, xrot);
  zrot = 0.0;
  } /* if !fixfocus */
 else
  {
  DP.lat = FP.lat;
  DP.lon = FP.lon;
  DP.alt = FP.alt + 1000;
  convertpt(&VP);
  convertpt(&FP);
  findposvector(&FP, &VP);
  yrot = findangle2(FP.x, FP.z);
  rotate(&FP.x, &FP.z, yrot, yrot);
  xrot = findangle2(FP.y, FP.z);
  rotate(&FP.y, &FP.z, xrot, xrot);
  convertpt(&DP);
  findposvector(&DP, &VP);
  rotate(&DP.x, &DP.z, yrot, findangle2(DP.x, DP.z));
  rotate(&DP.y, &DP.z, xrot, findangle2(DP.y, DP.z));
  zrot = findangle2(DP.x, DP.y);
  rotate(&DP.x, &DP.y, zrot, zrot);
  } /* if fixfocus */

 BuildRotationMatrix(-xrot, -yrot, zrot, NoBankMatx);

/* FP.q = sqrt(FP.x * FP.x + FP.y * FP.y + FP.z * FP.z);*/
 zrot -= PARC_RNDR_MOTION(8) * PiOver180;
 cosxrot = cos(-xrot);
 cosyrot = cos(-yrot);
 coszrot = cos(-zrot);
 sinxrot = sin(-xrot);
 sinyrot = sin(-yrot);
 sinzrot = sin(-zrot);

 BuildRotationMatrix(-xrot, -yrot, zrot, ScrRotMatx);

 DP.alt = 149.0E+6;
 DP.lat = PARC_RNDR_MOTION(15);
 DP.lon = PARC_RNDR_MOTION(16);
 getscrnpt(&SP, NULL);
 VectorMagnitude(&SP);
 UnitVector(&SP);
 memcpy(&SNB, &SP, sizeof (struct coords));
 RotatePoint(&SNB, NoBankMatx);
 NegateVector(&SNB);

 return (1);

} /*  setquickview() */

/*********************************************************************/
#ifdef GHGJAHKAJSHGDSH
// Obsolete - see Tree.c
void loadtreemodel(void)
{
/* reads standard GENGYM ascii output */

 char filename[255];
 short error, spp, dbh, i, fractlevel, dataread = 0;
 long mapyear, year, species;
 double overlap,latitude,polyarea,polyperacre, sum, runsum;
 FILE *fdat;

 for (spp=0; spp<MAXSPECIES; spp++) {
  for (dbh=0; dbh<MAXDBH; dbh++) standdata[spp][dbh].tpa=0.0;
 }
 gets(str);
 printf("Name of file to be read: ");
 gets(filename);
 printf("Year to read: ");
 gets(str);
 mapyear=atoi(str);
 if ((fdat=fopen(filename,"r"))==NULL) {
  printf("Can't open file %s to read\n",filename);
  settings.treemodel=0;
  return;
 }
 for (i=1; i<10; i++) fgets(str,119,fdat);
 while (1) {
  error=fscanf(fdat,"%hd",&year);
  if (error==EOF || error==0) break;
  if (year>mapyear) break;
  if (year==mapyear) {
   fscanf(fdat,"%d",&species);
   switch (species) {
    case 93:			/* englemann spruce */
     spp=0;
     break;
    case 108:			/* lodgepole pine */
     spp=1;
     break;
    case 19:			/* subalpine fir */
     spp=2;
     break;
    default:
     spp=0;
     break;
   } 
   fscanf(fdat,"%hd",&dbh);
   if (dbh>=MAXDBH) {
    fgets(str,119,fdat);
    continue;
   }
   fscanf(fdat,"%le %hd %hd",&standdata[spp][dbh].tpa,
      &standdata[spp][dbh].ht,&standdata[spp][dbh].pctcrn);
   dataread=1;
  }
  else fgets(str,119,fdat);
 }
 fclose(fdat);
 if (!dataread) {
  printf("No tree model data read!\n");
  settings.treemodel=0;
  return;
 }
 printf("Tree model read\n");
 sum=0.0;
 for (dbh=0; dbh<MAXDBH; dbh++) treearea[dbh]=
       Pi*((4.344+1.029*(dbh+.5))/2.0)*((4.344+1.029*(dbh+.5))/2.0);
 for (spp=0; spp<MAXSPECIES; spp++) {
  for (dbh=0; dbh<MAXDBH; dbh++) {   
   sum +=(standdata[spp][dbh].tpa*treearea[dbh]);
  }
 }
 if (sum<=0.0) {
  printf("Error reading model data\n");
  settings.treemodel=0;
  return;
 }
 printf("Canopy closure= %f sq ft/acre = %f%%\n",sum,sum/435.60);
EnterStock:
 printf("Clustering level in percent (overlap): ");
 scanf("%le",&overlap);
 printf("Fractal level (level 0 = 1201 pts per degree): ");
 scanf("%d",&fractlevel);
 printf("Latitude of mapped area (degrees): ");
 scanf("%le",&latitude);
 polyarea=.5*(LATSCALE*5280.0/1200.0)*(LATSCALE*5280.0/1200.0)*
               cos(latitude*PiOver180);
 for (i=1; i<=fractlevel; i++) polyarea /=4.0;
 polyperacre=43560.0/polyarea;
 if (overlap<0.0 || overlap>=100.0) goto EnterStock;
 overlap *=.01;
 overlap=1.0-overlap;
 runsum=0.0;
 for (spp=0; spp<MAXSPECIES; spp++) {
  for (dbh=0; dbh<MAXDBH; dbh++) {
   standdata[spp][dbh].tpa *=overlap;
   standdata[spp][dbh].tpa /=(polyarea/treearea[dbh]);   
   runsum +=standdata[spp][dbh].tpa/polyperacre;
   standdata[spp][dbh].percent=runsum;
  }
 }
 printf("Tree model percents computed. Occupied sites = %f\n",runsum);
 if (runsum>1.0)
   printf("Warning occupatipon > 100%% will result in loss of high dbh items\n");
 printf("Species 93 (Englemann Spruce) ecosystem: ");
 scanf("%hd", &treeeco[0]);
 printf("Species 108 (Lodgepole Pine) ecosystem: ");
 scanf("%hd", &treeeco[1]);
 printf("Species 19 (Subalpine Fir) ecosystem: ");
 scanf("%hd", &treeeco[2]);
 printf("Default ecosystem (no trees): ");
 scanf("%hd", &settings.defaulteco); 

} /* loadtreemodel() */
#endif
/*********************************************************************/

short makesky(short renderseg, struct Window *win)
{
 short rotatesky = 0, zenithline = 0, MergePts, red, green, blue, error = 0;
 long x, y, zip, colval;
 double angle, xxx, yyy, halfsky, skyfact, maxskyfact, lonscale;
 struct BusyWindow *BWDE;

 BWDE = BusyWin_New("Sky", settings.scrnheight, 0, 'BWDE');

 if (! settings.horfix)
  {
  lonscale = cos(FP.lat) * LATSCALE;
  xxx = (FP.lon - VP.lon) * PiUnder180 * lonscale;
  yyy = (FP.lat - VP.lat) * PiUnder180 * LATSCALE;
  angle = findangle2(xxx, yyy);
  xxx = qmax * sin(angle);
  yyy = qmax * cos(angle);
  DP.lat = VP.lat * PiUnder180 + yyy / LATSCALE;
  DP.lon = VP.lon * PiUnder180 + xxx / lonscale;
  DP.alt = 0.0;
  getscrnpt(NULL, NULL);
  horline = PARC_RNDR_MOTION(7) - (DP.y * PROJPLANE / DP.z) / vertscale;
  DP.lat = VP.lat * PiUnder180 + yyy / LATSCALE;
  DP.lon = VP.lon * PiUnder180 + xxx / lonscale;
  DP.alt = settings.zenith;					/* qmax; */
  convertpt(&DP);
  findposvector(&DP, &VP);
  rotate(&DP.x, &DP.z, yrot, findangle2(DP.x, DP.z));
  rotate(&DP.y, &DP.z, xrot, findangle2(DP.y, DP.z));
  rotate(&DP.x, &DP.y, zrot - PARC_RNDR_MOTION(8) * PiOver180,
             findangle2(DP.x, DP.y));
  zenithline = PARC_RNDR_MOTION(7) - (DP.y * PROJPLANE / DP.z) / vertscale;
  } /* if ! horfix, horizon and zenith computed at qmax from viewer */

 sprintf(str, "horline=%d, zenithline=%d\n",horline,zenithline);
 Log(MSG_UTIL_TAB, str);
 zenithline = horline - zenithline;

 if (horpt >= settings.scrnwidth / 2)
  {
  maxskyfact = sqrt(zenithline * zenithline +
     (horpt * horstretch) * (horpt * horstretch));
  } /* if horpt is right of center */
 else
  {
  maxskyfact = sqrt(zenithline * zenithline +
     ((wide - horpt) * horstretch) * ((wide - horpt) * horstretch));
  } /* else */

 redsky =   (PARC_RNDR_COLOR(3, 0) - (float)PARC_RNDR_COLOR(4, 0)) / maxskyfact;
 greensky = (PARC_RNDR_COLOR(3, 1) - (float)PARC_RNDR_COLOR(4, 1)) / maxskyfact;
 bluesky =  (PARC_RNDR_COLOR(3, 2) - (float)PARC_RNDR_COLOR(4, 2)) / maxskyfact;
 srand48(1010);
 halfsky = settings.skyalias * .5;
 if (fabs(PARC_RNDR_MOTION(8)) > .0001)
  rotatesky = 1;
 renderseg *= settings.scrnheight;

 zip = 0;
 for (y=0; y<settings.scrnheight; y++)
  {
  for (x=0; x<settings.scrnwidth; x++, zip++)
   {
   if (((unsigned int)bytemap[zip]) < 100)
    {
    DP.y = (double)y + renderseg - horline;
    DP.x = (double)x - horpt;

    if (rotatesky) getskypt();

    skyfact = sqrt(DP.y * DP.y + (DP.x * horstretch) * (DP.x * horstretch));
    skyfact = skyfact < maxskyfact ? skyfact: maxskyfact;
    flred = PARC_RNDR_COLOR(3, 0) - redsky * skyfact;
    flgreen = PARC_RNDR_COLOR(3, 1) - greensky *  skyfact;
    flblue = PARC_RNDR_COLOR(3, 2) - bluesky * skyfact;
    Random = settings.skyalias * drand48() - halfsky;
    aliasred = flred + Random * flred / 255.0;
    aliasgreen = flgreen + Random * flgreen / 255.0;
    aliasblue = flblue + Random * flblue / 255.0;
    if (aliasred > 255) aliasred = 255;
    if (aliasgreen > 255) aliasgreen = 255;
    if (aliasblue > 255) aliasblue = 255;
    red = aliasred < 0 ? 0: aliasred;
    green = aliasgreen < 0 ? 0: aliasgreen;
    blue = aliasblue < 0 ? 0: aliasblue;

    if (((unsigned int)bytemap[zip]) > 0)
     {
     MergePts = 100 - (unsigned int)bytemap[zip];
     colval = (*(bitmap[0] + zip) * (unsigned int)bytemap[zip] + red * MergePts) / 100;
     *(bitmap[0] + zip) = (UBYTE)colval;
     colval = (*(bitmap[1] + zip) * (unsigned int)bytemap[zip] + green * MergePts) / 100;
     *(bitmap[1] + zip) = (UBYTE)colval;
     colval = (*(bitmap[2] + zip) * (unsigned int)bytemap[zip] + blue * MergePts) / 100;
     *(bitmap[2] + zip) = (UBYTE)colval;
     } /* values already present */
    else
     {
     *(bitmap[0] + zip) = (UBYTE)red;
     *(bitmap[1] + zip) = (UBYTE)green;
     *(bitmap[2] + zip) = (UBYTE)blue;
     } /* no values present */
    if ((render & 0x11) == 0x11)
     {
     ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
     }

    if (y > 2)
     if (zbuf[zip - 3 * settings.scrnwidth] == FLT_MAX)
      {
      zbuf[zip] = FLT_MAX;	/* this is so partial sky pixels can receive
					 clouds and aren't picked up in
					 reflections where they don't belong */
      if (ReflectionMap)
       ReflectionMap[zip] = 0;
      } /* if */
    } /* if bytemap[zip] not full */
   } /* for x=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWDE, y + 1);
  } /* for y=0... */

 if (BWDE) BusyWin_Del(BWDE);

 return (error);

} /* makesky() */

/*********************************************************************/

static void getskypt(void) // used locally only -> static, AF 23.7.2021
{
 double angle,newangle,length;

 if (DP.x == 0.0) {
  angle = DP.y > 0.0 ? HalfPi: OneAndHalfPi;
 }
 else {
  angle = atan(DP.y / DP.x);
  if (DP.x < 0.0) angle += Pi;
 }
 length = sqrt(DP.x * DP.x + DP.y * DP.y);
 newangle = angle - PARC_RNDR_MOTION(8) * PiOver180;
 DP.x = length * cos(newangle);
 DP.y = length * sin(newangle);
}

/*********************************************************************/

void antialias(void)
{
 long x, y, j, pts, zip;

 if (settings.zbufalias) {
  for (j=0; j<settings.aliasfactor; j++) {
   for (y=0; y<high; y++) {
    for (x=pts=0; x<wide; x++,pts=0) {
     zip=y*settings.scrnwidth+x;
     red=(UBYTE)(*(bitmap[0]+zip));
     green=(UBYTE)(*(bitmap[1]+zip));
     blue=(UBYTE)(*(bitmap[2]+zip));

     if (fabs( *(zbuf + zip) - *(zbuf + zip + 1) ) > settings.zalias) { 
      altred=(UBYTE)(*(bitmap[0]+zip+1));
      altgreen=(UBYTE)(*(bitmap[1]+zip+1));
      altblue=(UBYTE)(*(bitmap[2]+zip+1));
      pts=1;
     } 
     if (fabs( *(zbuf + zip) - *(zbuf + zip + settings.scrnwidth)) >
		settings.zalias) {
      if (pts) {
       aliasred=((long)red*2+altred+(UBYTE)(*(bitmap[0]+zip+settings.scrnwidth)))/4;
       aliasgreen=((long)green*2+altgreen+(UBYTE)(*(bitmap[1]+zip+settings.scrnwidth)))/4;
       aliasblue=((long)blue*2+altblue+(UBYTE)(*(bitmap[2]+zip+settings.scrnwidth)))/4;
      }
      else {    
       aliasred=((long)red*2+(UBYTE)(*(bitmap[0]+zip+settings.scrnwidth)))/3;
       aliasgreen=((long)green*2+(UBYTE)(*(bitmap[1]+zip+settings.scrnwidth)))/3;
       aliasblue=((long)blue*2+(UBYTE)(*(bitmap[2]+zip+settings.scrnwidth)))/3;
       pts=1;
      }
     }
     else {
      if (pts) {
       aliasred=((long)red*2+altred)/3;
       aliasgreen=((long)green*2+altgreen)/3;
       aliasblue=((long)blue*2+altblue)/3;
      }
     }
     if (pts) {
      *(bitmap[0]+zip)=(UBYTE)aliasred;
      *(bitmap[1]+zip)=(UBYTE)aliasgreen;
      *(bitmap[2]+zip)=(UBYTE)aliasblue;   
     }
    } /* for x=0... */
   } /* for y=0... */
  } /* for j=0... */
 } /* if settings.zbufalias */
 else {
  for (j=0; j<settings.aliasfactor; j++) {
   for (y=0; y<high; y++) {
    for (x=0; x<wide; x++) {
     zip=y*settings.scrnwidth+x;
     red=(UBYTE)(*(bitmap[0]+zip));
     altred=((UBYTE)(*(bitmap[0]+zip+1))+(UBYTE)(*(bitmap[0]+zip+settings.scrnwidth)))/2;
     green=(UBYTE)(*(bitmap[1]+zip));
     altgreen=((UBYTE)(*(bitmap[1]+zip+1))+(UBYTE)(*(bitmap[1]+zip+settings.scrnwidth)))/2;
     blue=(UBYTE)(*(bitmap[2]+zip));
     altblue=((UBYTE)(*(bitmap[2]+zip+1))+(UBYTE)(*(bitmap[2]+zip+settings.scrnwidth)))/2;
     aliasred=((int)red+(int)altred)/2;
     aliasgreen=((int)green+(int)altgreen)/2;
     aliasblue=((int)blue+(int)altblue)/2;
     *(bitmap[0]+zip)=(UBYTE)aliasred;
     *(bitmap[1]+zip)=(UBYTE)aliasgreen;
     *(bitmap[2]+zip)=(UBYTE)aliasblue;   
    } /* for x=0... */
   } /* for y=0... */
  } /* for j=0 */
 } /* if !settings.zbufalias */
} /* antialias() */

/**********************************************************************/

#define HOR_JITTER_AMP 1.0
#define VERT_JITTER_AMP 30.0

short Reflection_Render(struct Window *win)
{
short error = 0, Red, Grn, Blu, Banking = 0;
long x, y, sX, sY, zip, sZip, csZip;
double A, D, Alpha, Beta, dAodX, Aprpr, Apr, Dpr, AoD, dsX, dsY, oX, oY,  /* strange chars changed to Alpha and Beta*/
	Wx[3], Wy[3], dOffset, HalfWidth, BankAngle;
struct BusyWindow *BWDE;

 if (! ReflectionMap || ! ElevationMap || ! bitmap[0] || ! bitmap[1] || ! bitmap[2])
  return (0);
 
 BWDE = BusyWin_New("Reflections", settings.scrnheight, 0, 'BWDE');

 HalfWidth = wide / 2.0;

 if (fabs(PARC_RNDR_MOTION(8)) > .0001)
  {
  BankAngle = PARC_RNDR_MOTION(8) * PiOver180;
  Banking = 1;
  } /* if gotta bank those reflections */

 for (y=1, zip=settings.scrnwidth; y<settings.scrnheight; y++)
  {
  for (x=0; x<settings.scrnwidth; x++, zip++)
   {
   if (ReflectionMap[zip] && ReflectionMap[zip] != 255)
    {
    sX = x;

    dOffset = (double)(x - HalfWidth) / (double)(y - high - 4 * settings.scrnwidth);
    if (Banking)
     {
     dOffset = atan(dOffset) - BankAngle;
     if (fabs(dOffset - HalfPi) < .0001)
      continue;
     dOffset = tan(dOffset);
     } /* if Banking */

    A = PARC_RNDR_MOTION(0) - ElevationMap[zip] * PARC_RNDR_MOTION(14) * .001;
/*  Uncomment this if you don't want reflections when below the water surface
    if (A < 0.0)
     continue;
*/
    D = sqrt(zbuf[zip] * zbuf[zip] - A * A);
    if (D <= 0.0)
     continue;
    AoD = A / D;

    if (SlopeMap)
     {
     dAodX = SlopeMap[zip] * zbuf[zip] / D;
     Beta  = atan(dAodX);
     Alpha = atan(AoD);
     AoD = tan(Beta * 2.0 + Alpha);
     } /* if waves */

    if (AoD > 1.0)
     AoD = 1.0;

    if (AoD > 0.0)
     ReflectionMap[zip] *= (1.0 - AoD);

    for (sY=y-1, dsX=sX-dOffset; sY>=0; sY--, dsX-=dOffset)
     {
     sX = dsX;
     if (sX < 0)
      {
      dOffset *= (-1.0);
      sX = 0;
      }
     else if (sX > wide)
      {
      dOffset *= (-1.0);
      sX = wide;
      }
     sZip = sY * settings.scrnwidth + sX;
     if (ElevationMap[sZip] < ElevationMap[zip] - MaxWaveAmp)
      continue;
     Aprpr = PARC_RNDR_MOTION(0) - ElevationMap[sZip] * PARC_RNDR_MOTION(14) * .001;
     Dpr = sqrt(zbuf[sZip] * zbuf[sZip] - Aprpr * Aprpr) - D;
     Apr = (ElevationMap[sZip] - ElevationMap[zip]) * PARC_RNDR_MOTION(14) * .001;
     if (Dpr > 0.0)
      {
      if (Apr / Dpr > AoD)
       break;
      } /* if */
     } /* for sY=... */

/* horizontal perturbation */

    if (sY < 0)
     {
     sY = 0;
     } /* if */
    dsY = sY + .5;

    dsX += ((HOR_JITTER_AMP / (2.0 * zbuf[zip]))
		* sin((((x - frame / 5.0) * zbuf[zip]) * TwoPi) / 10.0));

    sX = dsX;
    if (sX <= 0)
     {
     sX = 0;
     dsX = 0.5;
     }
    else if (sX >= wide)
     {
     sX = wide;
     dsX = wide + .5;
     }

/* vertical perturbation */

    sY = dsY;
    if (sY <= 0)
     {
     sY = 0;
     dsY = .5;
     }
    else if (sY >= high)
     {
     sY = high;
     dsY = high + .5;
     }

    sZip = sY * settings.scrnwidth + sX;

/* antialias the source pixel */

    oY = dsY - sY;
    oX = dsX - sX;

    if (oY >= .5)
     {
     Wy[0] = 0.0;
     if (sY < high)
      {
      Wy[1] = .5 + 1.0 - oY;
      Wy[2] = 1.0 - Wy[1];
      }
     else
      {
      Wy[1] = 1.0;
      Wy[2] = 0.0;
      }
     }
    else
     {
     Wy[2] = 0.0;
     if (sY > 0)
      {
      Wy[1] = .5 + oY;
      Wy[0] = 1.0 - Wy[1];
      }
     else
      {
      Wy[1] = 1.0;
      Wy[0] = 0.0;
      }
     }

    if (oX >= .5)
     {
     Wx[0] = 0.0;
     if (sX < wide)
      {
      Wx[1] = .5 + 1.0 - oX;
      Wx[2] = 1.0 - Wx[1];
      }
     else
      {
      Wx[1] = 1.0;
      Wx[2] = 0.0;
      }
     }
    else
     {
     Wx[2] = 0.0;
     if (sX > 0)
      {
      Wx[1] = .5 + oX;
      Wx[0] = 1.0 - Wx[1];
      }
     else
      {
      Wx[1] = 1.0;
      Wx[0] = 0.0;
      }
     }

    Red = Grn = Blu = 0;
    csZip = sZip - 1 - settings.scrnwidth;
    if (csZip >= 0)
     {
     Red += *(bitmap[0] + csZip) * Wx[0] * Wy[0];
     Grn += *(bitmap[1] + csZip) * Wx[0] * Wy[0];
     Blu += *(bitmap[2] + csZip) * Wx[0] * Wy[0];
     }
    csZip ++;
    if (csZip >= 0)
     {
     Red += *(bitmap[0] + csZip) * Wx[1] * Wy[0];
     Grn += *(bitmap[1] + csZip) * Wx[1] * Wy[0];
     Blu += *(bitmap[2] + csZip) * Wx[1] * Wy[0];
     }
    csZip ++;
    if (csZip >= 0)
     {
     Red += *(bitmap[0] + csZip) * Wx[2] * Wy[0];
     Grn += *(bitmap[1] + csZip) * Wx[2] * Wy[0];
     Blu += *(bitmap[2] + csZip) * Wx[2] * Wy[0];
     }
    csZip += (settings.scrnwidth - 2);
    if (csZip >= 0)
     {
     Red += *(bitmap[0] + csZip) * Wx[0] * Wy[1];
     Grn += *(bitmap[1] + csZip) * Wx[0] * Wy[1];
     Blu += *(bitmap[2] + csZip) * Wx[0] * Wy[1];
     }
    csZip ++;
    Red += *(bitmap[0] + csZip) * Wx[1] * Wy[1];
    Grn += *(bitmap[1] + csZip) * Wx[1] * Wy[1];
    Blu += *(bitmap[2] + csZip) * Wx[1] * Wy[1];
    csZip ++;
    if (csZip < bmapsize)
     {
     Red += *(bitmap[0] + csZip) * Wx[2] * Wy[1];
     Grn += *(bitmap[1] + csZip) * Wx[2] * Wy[1];
     Blu += *(bitmap[2] + csZip) * Wx[2] * Wy[1];
     }
    csZip += (settings.scrnwidth - 2);
    if (csZip < bmapsize)
     {
     Red += *(bitmap[0] + csZip) * Wx[0] * Wy[2];
     Grn += *(bitmap[1] + csZip) * Wx[0] * Wy[2];
     Blu += *(bitmap[2] + csZip) * Wx[0] * Wy[2];
     }
    csZip ++;
    if (csZip < bmapsize)
     {
     Red += *(bitmap[0] + csZip) * Wx[1] * Wy[2];
     Grn += *(bitmap[1] + csZip) * Wx[1] * Wy[2];
     Blu += *(bitmap[2] + csZip) * Wx[1] * Wy[2];
     }
    csZip ++;
    if (csZip < bmapsize)
     {
     Red += *(bitmap[0] + csZip) * Wx[2] * Wy[2];
     Grn += *(bitmap[1] + csZip) * Wx[2] * Wy[2];
     Blu += *(bitmap[2] + csZip) * Wx[2] * Wy[2];
     }

/* bitmap averaging */

    *(bitmap[0] + zip) = (*(bitmap[0] + zip) * (254 - ReflectionMap[zip]) +
	((Red * 8) / 10) * ReflectionMap[zip]) / 254;
    *(bitmap[1] + zip) = (*(bitmap[1] + zip) * (254 - ReflectionMap[zip]) +
	((Grn * 8) / 10) * ReflectionMap[zip]) / 254;
    *(bitmap[2] + zip) = (*(bitmap[2] + zip) * (254 - ReflectionMap[zip]) +
	((Blu * 8) / 10) * ReflectionMap[zip]) / 254;

    if ((render & 0x11) == 0x11)
     {
     ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
     }
    } /* if reflection */
   } /* for x=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWDE, y + 1);
  } /* for y=0... */

/* do a little blurring to smooth out reflections */

 for (y=0; y<high; y++)
  {
  zip = y * settings.scrnwidth;
  for (x=0; x<wide; x++, zip++)
   {
   if (ReflectionMap[zip])
    {
    *(bitmap[0] + zip) = ((unsigned int)(*(bitmap[0] + zip)) + 
	(unsigned int)(*(bitmap[0] + zip + 1)) + 
	(unsigned int)(*(bitmap[0] + zip + settings.scrnwidth)) +
	(unsigned int)(*(bitmap[0] + zip + settings.scrnwidth + 1))) / 4;
    *(bitmap[1] + zip) = ((unsigned int)(*(bitmap[1] + zip)) + 
	(unsigned int)(*(bitmap[1] + zip + 1)) + 
	(unsigned int)(*(bitmap[1] + zip + settings.scrnwidth)) +
	(unsigned int)(*(bitmap[1] + zip + settings.scrnwidth + 1))) / 4;
    *(bitmap[2] + zip) = ((unsigned int)(*(bitmap[2] + zip)) + 
	(unsigned int)(*(bitmap[2] + zip + 1)) + 
	(unsigned int)(*(bitmap[2] + zip + settings.scrnwidth)) +
	(unsigned int)(*(bitmap[2] + zip + settings.scrnwidth + 1))) / 4;
    } /* if reflection */
   } /* for x=0... */
  } /* for y=0... */

 if (BWDE) BusyWin_Del(BWDE);

 return (error);

} /* Reflection_Render() */

/***********************************************************************/

short Celestial_Bodies(UBYTE **Bitmap, long Width, long Height, struct Window *win)
{
char filename[256];
short error = 0, Colors, Sw, Sh;
float fDx, fDy, fDq;
double Dr, Luminosity, Visible;
UBYTE *Image[3];
struct ColorComponents CC;

/* Sun */
 if (settings.sun)
  {
  Luminosity = Visible = 0.0;
  Image[0] = Image[1] = Image[2] = NULL;
  Sw = Sh = 0;
  Dr = PARC_RNDR_MOTION(28);
  CC.Red = PARC_RNDR_COLOR(19, 0);
  CC.Grn = PARC_RNDR_COLOR(19, 1);
  CC.Blu = PARC_RNDR_COLOR(19, 2);

  DP.alt = 149.0E+6;
  DP.lat = PARC_RNDR_MOTION(26);
  DP.lon = PARC_RNDR_MOTION(27);
  getscrncoords(&fDx, &fDy, &fDq, NULL, NULL);

  strmfp(filename, imagepath, sunfile);

  if (LoadImage(filename, 0, Image, Sw, Sh, 0, &Sw, &Sh, &Colors))
   {
   if (! (error = Image_Composite(Bitmap, Image, Width, Height,
	Sw, Sh, Dr, (double)fDx, (double)fDy, (double)fDq,
	&Visible, &Luminosity, &CC, "Sun", win)))
    {
    if (settings.sunhalo)
     error = HaloEffect(Bitmap, Width, Height, Dr * 10.0, (double)fDx, (double)fDy,
	Luminosity * Visible, (double)fDq, &CC, "Sun Halo", win);
    } /* if */
   } /* if image loaded */
  else
   {
   error = 1;
   User_Message("Render Module", "Error loading Sun Image!\n\
Operation terminated.", "OK", "o");
   } /* else */
  if (Image[0])
   free_Memory(Image[0], Sw * Sh);
  if (Image[1])
   free_Memory(Image[1], Sw * Sh);
  if (Image[2])
   free_Memory(Image[2], Sw * Sh);
  } /* if sun */

 if (error)
  goto EndCelest;

/* Moon */
 if (settings.moon)
  {
  Luminosity = Visible = 0.0;
  Image[0] = Image[1] = Image[2] = NULL;
  Sw = Sh = 0;
  Dr = PARC_RNDR_MOTION(31);
  CC.Red = PARC_RNDR_COLOR(20, 0);
  CC.Grn = PARC_RNDR_COLOR(20, 1);
  CC.Blu = PARC_RNDR_COLOR(20, 2);

  DP.alt = 38.0E+4;
  DP.lat = PARC_RNDR_MOTION(29);
  DP.lon = PARC_RNDR_MOTION(30);
  getscrncoords(&fDx, &fDy, &fDq, NULL, NULL);

  strmfp(filename, imagepath, moonfile);
  if (LoadImage(filename, 0, Image, Sw, Sh, 0, &Sw, &Sh, &Colors))
   {
   if (! (error = Image_Composite(Bitmap, Image, Width, Height,
	Sw, Sh, Dr, (double)fDx, (double)fDy, (double)fDq,
	&Visible, &Luminosity, &CC, "Moon", win)))
    {
    if (settings.moonhalo)
     error = HaloEffect(Bitmap, Width, Height, Dr * 3.0, (double)fDx, (double)fDy,
	Luminosity * Visible, (double)fDq, &CC, "Moon Halo", win);
    } /* if */
   } /* if image loaded */
  else
   {
   error = 1;
   User_Message("Render Module", "Error loading Moon Image!\n\
Operation terminated.", "OK", "o");
   } /* else */
  if (Image[0])
   free_Memory(Image[0], Sw * Sh);
  if (Image[1])
   free_Memory(Image[1], Sw * Sh);
  if (Image[2])
   free_Memory(Image[2], Sw * Sh);
  } /* if moon */

EndCelest:

 return (error);
 
} /* Celestial_Bodies() */

/***********************************************************************/

static short HaloEffect(UBYTE **Bitmap, long Width, long Height,
	double Dr, double Dx, double Dy, double Intensity, double NoHaloDist,
	struct ColorComponents *CC, char *NameStr, struct Window *win) // used locally only -> static, AF 19.7.2021
{
short error = 0;
long x, y, zip, CheckByteMap;
double HaloStr, BGStr, Dist, Distx, Disty, DistQMax;
struct BusyWindow *BWDE;

 DistQMax = qmax / 4.0;

 BWDE = BusyWin_New(NameStr, Height, 0, 'BWDE');

 zip = CheckByteMap = 0;
 for (y=0; y<Height; y++)
  {
  for (x=0; x<Width; x++, zip++, CheckByteMap=0)
   {
   if (fabs(zbuf[zip] - NoHaloDist) < 1.0)
    {
    if (bytemap[zip] >= 99)
     continue;
    CheckByteMap = 1;
    } /* if this is the object we're haloing */
   Distx = Dx - x;
   Disty = (Dy - y) * settings.picaspect;
   Dist = sqrt(Distx * Distx + Disty * Disty);
   if (Dr > Dist)
    {
    HaloStr = 1.0 - Dist / Dr;
    HaloStr *= HaloStr;
    HaloStr *= Intensity;
    if (CheckByteMap)
     HaloStr *= ((100.0 - (unsigned int)bytemap[zip]) / 100.0);
    if (zbuf[zip] < DistQMax)
     HaloStr -= (HaloStr * (DistQMax - zbuf[zip]) / DistQMax);
    BGStr = 1.0 - HaloStr;
    Bitmap[0][zip] = (unsigned int)Bitmap[0][zip] * BGStr + CC->Red * HaloStr;
    Bitmap[1][zip] = (unsigned int)Bitmap[1][zip] * BGStr + CC->Grn * HaloStr;
    Bitmap[2][zip] = (unsigned int)Bitmap[2][zip] * BGStr + CC->Blu * HaloStr;
    if ((render & 0x11) == 0x11)
     {
     ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
     }
    } /* if in area of effect */
   } /* for x=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, y + 1);
  } /* for y=0... */

 if (BWDE) BusyWin_Del(BWDE);

 return (error);

} /* HaloEffect() */

/***********************************************************************/

static short Image_Composite(UBYTE **Bitmap, UBYTE **Source, long Iw, long Ih,
	short Sw, short Sh, double Dr, double Dx, double Dy, double Distance,
	double *Visible, double *Luminosity, struct ColorComponents *CC,
	char *NameStr, struct Window *win) // used locally only -> static, AF 23.7.2021
{
short error = 0;
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtys, wtye, wty, wtxs, wtxe, wt, PixWt, MaxWt;
long Px, Py, Pxp1, Pyp1, x, y, DRows, DCols, DxStart, DyStart, PixVal,
	zip, SourceZip, i, j, PtsSeen = 0, PtsExist = 0, Wt;
struct BusyWindow *BWDE;

 if (Dr <= 0.0 || Sw == 0 || Sh == 0)
  {
  *Visible = *Luminosity = 0.0;
  return (0);
  } /* if something not kosher */

 Dox = Dx - Dr;
 Dex = Dx + Dr;
 Doy = Dy - Dr / settings.picaspect;
 Dey = Dy + Dr / settings.picaspect;

 dX = Sw / (Dex - Dox);
 dY = Sh / (Dey - Doy);

 MaxWt = dX * dY;

 Sox = ((int)Dox - Dox) * dX;
 Soy = ((int)Doy - Doy) * dY;

 DxStart = Dox; 
 DyStart = Doy; 
 DCols = Dex - DxStart + 1.0;
 DRows = Dey - DyStart + 1.0;

 BWDE = BusyWin_New(NameStr, DRows, 0, 'BWDE');

 for (y=DyStart, Coy=Soy, Cey=Soy+dY, j=0, PixWt = 0, PixVal=0.0; j<DRows;
	j++, y++, Coy+=dY, Cey+=dY)
  {
  if (y < 0)
   continue;
  if (y >= Ih)
   break;
  zip = y * Iw + DxStart;
  for (x=DxStart, Cox=Sox, Cex=Sox+dX, i=0; i<DCols;
	i++, x++, Cox+=dX, Cex+=dX, PixVal=0, PixWt=0.0, zip++)
   {
   if (x < 0)
    continue;
   if (x >= Iw)
    break;
   for (Py=Coy, Pyp1=Coy+1; Py<Cey && Py<Sh; Py++, Pyp1++)
    {
    if (Py < 0)
     continue;
    wtys = Py > Coy ? 1.0: Pyp1 - Coy; 
    wtye = Pyp1 < Cey ? 1.0: Cey - Py;
    wty = wtys * wtye; 
    for (Px=Cox, Pxp1=Cox+1; Px<Cex && Px<Sw; Px++, Pxp1++)
     {
     if (Px < 0)
      continue;
     wtxs = Px > Cox ? 1.0: Pxp1 - Cox; 
     wtxe = Pxp1 < Cex ? 1.0: Cex - Px;
     wt = wty * wtxs * wtxe;
     SourceZip = Py * Sw + Px;
     if (Source[0][SourceZip])
      {
      PixWt += wt;
      PixVal += wt * (unsigned int)Source[0][SourceZip];
      }
     } /* for Px=... */
    } /* for Py=... */
   if (PixVal && PixWt > 0.0)
    {
    PtsExist ++;
    if (Distance < zbuf[zip])
     {
     PtsSeen ++;
     PixVal /= PixWt;
     *Luminosity += PixVal;
     PixWt /= MaxWt;
     zbuf[zip] = Distance;
     bytemap[zip] = 100 * PixWt;
     Wt = PixWt * PixVal;
     Bitmap[0][zip] += ((Wt * (CC->Red - Bitmap[0][zip])) / 255);
     Bitmap[1][zip] += ((Wt * (CC->Grn - Bitmap[1][zip])) / 255);
     Bitmap[2][zip] += ((Wt * (CC->Blu - Bitmap[2][zip])) / 255);
     if ((render & 0x11) == 0x11)
      {
      ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
      } /* if */
     } /* if */
    } /* if */
   } /* for Tox=... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, j + 1);
  } /* for Toy=... */

 *Visible = PtsExist ? (double)PtsSeen / (double)PtsExist: 0.0;
 *Luminosity = PtsSeen ? (*Luminosity / PtsSeen) / 255.0: 0.0;

 if (BWDE) BusyWin_Del(BWDE);

 return (error);

} /* Image_Composite() */
