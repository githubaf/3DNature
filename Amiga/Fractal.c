//#undef STATIC_FCN
//#define STATIC_FCN
/* Fractal.c
** Functions to subdivide polygons
** Original code pulled from MapTopoObject.c and MapUtil.c 8/28/95
** by Gary R. Huber
*/

#include "WCS.h"

STATIC_FCN void Vertex_Sum(struct faces *Vertex, struct faces *Face); // used locally only -> static, AF 19.7.2021
STATIC_FCN void FractPoly_Divide(struct elmapheaderV101 *map, struct VertexIndex *Vtx); // used locally only -> static, AF 23.7.2021
STATIC_FCN void Poly_Divide(void); // used locally only -> static, AF 23.7.2021
STATIC_FCN void polydivide(void); // used locally only -> static, AF 23.7.2021
STATIC_FCN long MinInt5(long Num1, long Num2, long Num3, long Num4, long Num5); // used locally only -> static, AF 23.7.2021

#define DISPLACE_FRACT_SLOPEFACT 10.0

#define FRAMES_PER_HOUR		108000.0

void Recurse(struct elmapheaderV101 *map, struct Window *win, short MapAsSFC,
	struct CloudData *CD, struct FaceData *Data)
{

 for (polyct[b]=0; polyct[b]<4; polyct[b]++)
  {
  Poly_Divide();
  if (maxfract > b)
   {
   b ++;
   Recurse(map, win, MapAsSFC, CD, Data);
   } /* if */
  else
   {
   Point_Sort();
   xx[0] = polyx[b][0]; yy[0] = polyy[b][0];
   xx[1] = polyx[b][1]; yy[1] = polyy[b][1];
   xx[2] = polyx[b][2]; yy[2] = polyy[b][2];
   el = (polyel[b][0] + polyel[b][1] + polyel[b][2]) / 3.0;
   qqq = (polyq[b][0] + polyq[b][1] + polyq[b][2]) / 3.0;
   facelat = (polylat[b][0] + polylat[b][1] + polylat[b][2]) / 3.0;
   facelong = (polylon[b][0] + polylon[b][1] + polylon[b][2]) / 3.0;
   slope = (polyslope[b][0] + polyslope[b][1] + polyslope[b][2]) / 3.0;
   relel = polyrelel[b][0] + polyrelel[b][1] + polyrelel[b][2];
   diplat = (polydiplat[b][0] + polydiplat[b][1] + polydiplat[b][2]) / 3.0;
   diplong = (polydiplon[b][0] + polydiplon[b][1] + polydiplon[b][2]) / 3.0;
   cloudcover = (polycld[b][0] + polycld[b][1] + polycld[b][2]) / 3.0;
   treerand4 = drand48();
   treerand3 = drand48();
   treerand2 = drand48();
   treerand = drand48();
   Random = -.2 * treerand + .1;
   Data->El[0] = polyel[0][0];
   Data->El[1] = polyel[0][1];
   Data->El[2] = polyel[0][2];

  if (settings.perturb)
    {
/* questionable - if this is enabled the dlat... variables must be set in Face_Render()
    if (random > .066)
     {			
     diplat = dlat;
     diplong = dlong;
     slope = dslope;
     } // if
end questionable */
    if (dir <= 1) diplat += (Random + Random * slope * 4.0);
    if (diplat > maxdlat) diplat = maxdlat;
    else if (diplat < mindlat) diplat = mindlat;
    if (dir)   diplong += (Random + Random * slope * 4.0);
    if (diplong > maxdlong) dlong = maxdlong;
    else if (diplong < mindlong) diplong = mindlong;
/* this is for creating lakes and you don't want no slope areas changed
** 0.0174 corresponds to 1 in radians */
    if (slope > 0.0174)
     {
     if (dir==1) slope += ((Random + Random * slope * 4.0) * .7);
     else  slope += ((Random + Random * slope * 4.0) * .35);
     slope = abs(slope) < HalfPi ? abs(slope): HalfPi - .001;
     } /* if */
    } /* if */

   dir = dir < 2 ? dir + 1: 0;
   MapTopo(map, win, MapAsSFC, 0, 1, &Data->El[0]);
   } /* else */
  } /* for polyct[b] = 0... */

 b--;

} /* Recurse() */

/***********************************************************************/

void FractRecurse(struct Window *win, struct elmapheaderV101 *map, short MapAsSFC,
	struct FaceData *Data, struct faces *Face, struct VertexIndex *Vtx,
	struct CloudData *CD)
{

 for (polyct[b]=0; polyct[b]<4; polyct[b]++)
  {
  FractPoly_Divide(map, Vtx);
  if (maxfract > b)
   {
   b ++;
   FractRecurse(win, map, MapAsSFC, Data, Face, Vtx, CD);
   } /* if */
  else
   {
   short i = 0, MakeWater = 0, MakeBeach = 0;
   ULONG Seed;
   char *SeedBytes;
   double Elev[3];

   ZeroCoords(&PP[3]);

   for (; i<3; i++)
    {
/* Wave stuff */
    if (polyel[b][i] <= MaxSeaLevel)
     {
     WaveAmp_Compute(&Elev[i], &DP.alt, &polyel[b][i], &polylat[b][i],
		&polylon[b][i], Tsunami, &MakeWater, &MakeBeach,
		(double)frame / (double)(settings.fieldrender + 1));
     } /* if point at or below sea level */
    else
     {
     Elev[i] = polyel[b][i];
     DP.alt = polyel[b][i] * PARC_RNDR_MOTION(14) * .001;
     }
    DP.lat = polylat[b][i];
    DP.lon = polylon[b][i];
    getdblscrncoords(&polyx[b][i], &polyy[b][i], &polyq[b][i], &PP[i], &PP[3]);
    } /* for i=0... */

/* for use in tree shading */

   memcpy(&PP[4], &PP[0], sizeof (struct coords));

   findposvector(&PP[1], &PP[0]);
   findposvector(&PP[2], &PP[0]);
   SurfaceNormal(&PP[0], &PP[2], &PP[1]);

   if (MakeWater >= 2)
    {
    Data->El[0]  = Elev[0];
    Data->El[1]  = Elev[1];
    Data->El[2]  = Elev[2];
    }
   else
    {
    Data->El[0]  = polyel[b][0];
    Data->El[1]  = polyel[b][1];
    Data->El[2]  = polyel[b][2];
    }
   Data->Lat[0] = polylat[b][0];
   Data->Lat[1] = polylat[b][1];
   Data->Lat[2] = polylat[b][2];
   Data->Lon[0] = polylon[b][0];
   Data->Lon[1] = polylon[b][1];
   Data->Lon[2] = polylon[b][2];

   FacePt_Switch(map, Data, Face);
   
   if (Tsunami)
    {
    if (MakeWater >= 3)	/* 2 */
     MakeWater = 1;
    else if (MakeWater + MakeBeach >= 2)
     MakeWater = 2;
    else
     MakeWater = 0;
    } /* if */
   else
    MakeWater = 0;
#ifdef GHJAGDJSAHD
   for (i=0; i<3; i++)
    {
    if (polyx[b][i] < 0.0)
     polyx[b][i] = 0.0;
    else if (polyx[b][i] > wide)
     polyx[b][i] = wide;
    if (polyy[b][i] < -30.0)
     polyy[b][i] = -30.0;
    else if (polyy[b][i] > oshigh)
     polyy[b][i] = oshigh;
    } /* for i=0... */
#endif
   FractPoint_Sort(&Data->El[0]);

   xx[0] = polyx[b][0]; yy[0] = polyy[b][0];
   xx[1] = polyx[b][1]; yy[1] = polyy[b][1];
   xx[2] = polyx[b][2]; yy[2] = polyy[b][2];
   el = (Elev[0] + Elev[1] + Elev[2]) / 3.0;
   qqq = (polyq[b][0] + polyq[b][1] + polyq[b][2]) / 3.0;
   facelat = (polylat[b][0] + polylat[b][1] + polylat[b][2]) / 3.0;
   facelong = (polylon[b][0] + polylon[b][1] + polylon[b][2]) / 3.0;
   relel = polyrelel[b][0] + polyrelel[b][1] + polyrelel[b][2];
   cloudcover = (polycld[b][0] + polycld[b][1] + polycld[b][2]) / 3.0;
   slope = Face->slope;
   aspect = Face->aspect;
   diplat = Face->diplat;
   diplong = Face->diplong;
   Seed = (fabs((map->lolong - facelong) / map->LonRange) * USHRT_MAX);
   Seed <<= 16;
   Seed += fabs((facelat - map->lolat) / map->LatRange) * USHRT_MAX;
   SeedBytes = (char *)&Seed;
   swmem(&SeedBytes[0], &SeedBytes[1], 1);
   swmem(&SeedBytes[2], &SeedBytes[3], 1);
   Seed += hseed;

   srand48(Seed);
   treerand4 = drand48();
   treerand3 = drand48();
   treerand2 = drand48();
   treerand = drand48();
   Random = -.2 * treerand + .1;

/* shading calculation */

   sunangle = VectorAngle(&PP[0], &SP);	/* gives you the cosine of the angle */
   if (sunangle < 0.0)
    sunangle = 0.0;
   else if (sunangle > 0.0)
    {
    double LonDiff;

/* these need to be lon mod 360 */

    LonDiff = fabs(facelong - PARC_RNDR_MOTION(16));
    while (LonDiff > 180.0)
     {
     LonDiff -= 360.0;
     LonDiff = fabs(LonDiff);
     } /* while */
    if (LonDiff > 85.0)
     {
     if (LonDiff > 105.0)
      LonDiff = 105.0;
     sunangle -= (sunangle * ((LonDiff - 85.0) / 30.0));
     } /* if surface is positioned near 90 from the sun in longitude */
    } /* if surface is lit at all */
   sunfactor = 1.0 - sunangle;
   sunshade = sunfactor * PARC_RNDR_MOTION(22);
   sunshade += ((1.0 - sunshade) * cloudcover);
   sunangle = ACos_Table(sunangle);

   dir = dir < 2 ? dir + 1: 0;

   MapTopo(map, win, MapAsSFC, MakeWater, 1, &Data->El[0]);
   } /* else */
  } /* for polyct[b] = 0... */

 b--;

} /* FractRecurse() */

/*********************************************************************/

STATIC_FCN void FractPoly_Divide(struct elmapheaderV101 *map, struct VertexIndex *Vtx) // used locally only -> static, AF 23.7.2021
{
 double ElDif[3], MaxDif;
 short Pert[3], Done[3];
 long i, j;
 ULONG Seed;
 char *SeedBytes;

 j = VtxNum;
 for (i=0; i<3; i++, j++)
  {
  if (Vtx->Use[j] >= 0)
   {
   polyel[b][i] = Vtx->El[Vtx->Use[j]];
   polyrelel[b][i] = Vtx->RelEl[Vtx->Use[j]];
   polylat[b][i] = Vtx->Lat[Vtx->Use[j]];
   polylon[b][i] = Vtx->Lon[Vtx->Use[j]];
   polycld[b][i] = Vtx->Cld[Vtx->Use[j]];
   Done[i] = 1;
   Pert[i] = (Vtx->Pert[j] && b <= Vtx->Max[Vtx->Edge[j]]);
   } /* if */
  else
   {
   Done[i] = 0;
   Pert[i] = (b <= Vtx->Max[Vtx->Edge[j]]);
   } /* else */
  } /* for i=0... */
 switch (polyct[b])
  {
  case 0:
   {
   if (! Done[2])
    {
    polyel[b][2]    =    (polyel[b-1][0] +    polyel[b-1][1]) / 2.0;
    polyrelel[b][2] = (polyrelel[b-1][0] + polyrelel[b-1][1]) / 2.0;
    polylat[b][2]   =   (polylat[b-1][0] +   polylat[b-1][1]) / 2.0;
    polylon[b][2]   =   (polylon[b-1][0] +   polylon[b-1][1]) / 2.0;
    polycld[b][2]   =   (polycld[b-1][0] +   polycld[b-1][1]) / 2.0;
    }
   if (! Done[1])
    {
    polyel[b][1]    =    (polyel[b-1][0] +    polyel[b-1][2]) / 2.0;
    polyrelel[b][1] = (polyrelel[b-1][0] + polyrelel[b-1][2]) / 2.0;
    polylat[b][1]   =   (polylat[b-1][0] +   polylat[b-1][2]) / 2.0;
    polylon[b][1]   =   (polylon[b-1][0] +   polylon[b-1][2]) / 2.0;
    polycld[b][1]   =   (polycld[b-1][0] +   polycld[b-1][2]) / 2.0;
    }
   if (! Done[0])
    {
    polyel[b][0]    =    (polyel[b-1][1] +    polyel[b-1][2]) / 2.0;
    polyrelel[b][0] = (polyrelel[b-1][1] + polyrelel[b-1][2]) / 2.0;
    polylat[b][0]   =   (polylat[b-1][1] +   polylat[b-1][2]) / 2.0;
    polylon[b][0]   =   (polylon[b-1][1] +   polylon[b-1][2]) / 2.0;
    polycld[b][0]   =   (polycld[b-1][1] +   polycld[b-1][2]) / 2.0;
    }
   if (Pert[2])
    ElDif[2] = fabs(polyel[b-1][0] - polyel[b-1][1]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[1])
    ElDif[1] = fabs(polyel[b-1][0] - polyel[b-1][2]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[0])
    ElDif[0] = fabs(polyel[b-1][1] - polyel[b-1][2]) / DISPLACE_FRACT_SLOPEFACT;
   break;
   } /* case 0 */
  case 1:
   {
   if (! Done[0])
    {
    polyel[b][0]    =    polyel[b-1][0];
    polyrelel[b][0] = polyrelel[b-1][0];
    polylat[b][0]   =   polylat[b-1][0];
    polylon[b][0]   =   polylon[b-1][0];
    polycld[b][0]   =   polycld[b-1][0];
    }
   if (! Done[1])
    {
    polyel[b][1]    =    (polyel[b-1][0] +    polyel[b-1][1]) / 2.0;
    polyrelel[b][1] = (polyrelel[b-1][0] + polyrelel[b-1][1]) / 2.0;
    polylat[b][1]   =   (polylat[b-1][0] +   polylat[b-1][1]) / 2.0;
    polylon[b][1]   =   (polylon[b-1][0] +   polylon[b-1][1]) / 2.0;
    polycld[b][1]   =   (polycld[b-1][0] +   polycld[b-1][1]) / 2.0;
    }
   if (! Done[2])
    {
    polyel[b][2]    =    (polyel[b-1][0] +    polyel[b-1][2]) / 2.0;
    polyrelel[b][2] = (polyrelel[b-1][0] + polyrelel[b-1][2]) / 2.0;
    polylat[b][2]   =   (polylat[b-1][0] +   polylat[b-1][2]) / 2.0;
    polylon[b][2]   =   (polylon[b-1][0] +   polylon[b-1][2]) / 2.0;
    polycld[b][2]   =   (polycld[b-1][0] +   polycld[b-1][2]) / 2.0;
    }
   if (Pert[0])
    ElDif[0] = 0.0;
   if (Pert[1])
    ElDif[1] = fabs(polyel[b-1][0] - polyel[b-1][1]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[2])
    ElDif[2] = fabs(polyel[b-1][0] - polyel[b-1][2]) / DISPLACE_FRACT_SLOPEFACT;
   break;
   } /* case 1 */
  case 2:
   {
   if (! Done[0])
    {
    polyel[b][0]    =    (polyel[b-1][2] +    polyel[b-1][0]) / 2.0;
    polyrelel[b][0] = (polyrelel[b-1][2] + polyrelel[b-1][0]) / 2.0;
    polylat[b][0]   =   (polylat[b-1][2] +   polylat[b-1][0]) / 2.0;
    polylon[b][0]   =   (polylon[b-1][2] +   polylon[b-1][0]) / 2.0;
    polycld[b][0]   =   (polycld[b-1][2] +   polycld[b-1][0]) / 2.0;
    }
   if (! Done[1])
    {
    polyel[b][1]    =    (polyel[b-1][2] +    polyel[b-1][1]) / 2.0;
    polyrelel[b][1] = (polyrelel[b-1][2] + polyrelel[b-1][1]) / 2.0;
    polylat[b][1]   =   (polylat[b-1][2] +   polylat[b-1][1]) / 2.0;
    polylon[b][1]   =   (polylon[b-1][2] +   polylon[b-1][1]) / 2.0;
    polycld[b][1]   =   (polycld[b-1][2] +   polycld[b-1][1]) / 2.0;
    }
   if (! Done[2])
    {
    polyel[b][2]    =    polyel[b-1][2];
    polyrelel[b][2] = polyrelel[b-1][2];
    polylat[b][2]   =   polylat[b-1][2];
    polylon[b][2]   =   polylon[b-1][2];
    polycld[b][2]   =   polycld[b-1][2];
    }
   if (Pert[0])
    ElDif[0] = fabs(polyel[b-1][2] - polyel[b-1][0]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[1])
    ElDif[1] = fabs(polyel[b-1][2] - polyel[b-1][1]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[2])
    ElDif[2] = 0.0;
   break;
   } /* case 2 */
  case 3:
   {
   if (! Done[0])
    {
    polyel[b][0]    =    (polyel[b-1][1] +    polyel[b-1][0]) / 2.0;
    polyrelel[b][0] = (polyrelel[b-1][1] + polyrelel[b-1][0]) / 2.0;
    polylat[b][0]   =   (polylat[b-1][1] +   polylat[b-1][0]) / 2.0;
    polylon[b][0]   =   (polylon[b-1][1] +   polylon[b-1][0]) / 2.0;
    polycld[b][0]   =   (polycld[b-1][1] +   polycld[b-1][0]) / 2.0;
    }
   if (! Done[1])
    {
    polyel[b][1]    =    polyel[b-1][1];
    polyrelel[b][1] = polyrelel[b-1][1];
    polylat[b][1]   =   polylat[b-1][1];
    polylon[b][1]   =   polylon[b-1][1];
    polycld[b][1]   =   polycld[b-1][1];
    }
   if (! Done[2])
    {
    polyel[b][2]    =    (polyel[b-1][1] +    polyel[b-1][2]) / 2.0;
    polyrelel[b][2] = (polyrelel[b-1][1] + polyrelel[b-1][2]) / 2.0;
    polylat[b][2]   =   (polylat[b-1][1] +   polylat[b-1][2]) / 2.0;
    polylon[b][2]   =   (polylon[b-1][1] +   polylon[b-1][2]) / 2.0;
    polycld[b][2]   =   (polycld[b-1][1] +   polycld[b-1][2]) / 2.0;
    }
   if (Pert[0])
    ElDif[0] = fabs(polyel[b-1][1] - polyel[b-1][0]) / DISPLACE_FRACT_SLOPEFACT;
   if (Pert[1])
    ElDif[1] = 0.0;
   if (Pert[2])
    ElDif[2] = fabs(polyel[b-1][1] - polyel[b-1][2]) / DISPLACE_FRACT_SLOPEFACT;
   break;
   } /* case 3 */
  } /* switch */

 for (i=0; i<3; i++, VtxNum++)
  {
  if (Pert[i])
   {
   MaxDif = 4.0 * ElDif[i];
   ElDif[i] = pow(ElDif[i], settings.dispslopefact);
   if (ElDif[i] > MaxDif)
    ElDif[i] = MaxDif;
	/* dispslopefact = 1.2 for grand canyon, 1.5 for RMNP area */
/* high 16 bits for random seed composed of latitude, low 16 of longitude */
   Seed = (fabs((map->lolong - polylon[b][i]) / map->LonRange) * USHRT_MAX);
   Seed <<= 16;
   Seed += fabs((polylat[b][i] - map->lolat) / map->LatRange) * USHRT_MAX;
   SeedBytes = (char *)&Seed;
   swmem(&SeedBytes[0], &SeedBytes[1], 1);
   swmem(&SeedBytes[2], &SeedBytes[3], 1);

   srand48(Seed);
   polyel[b][i] += (GaussRand() * fractperturb[b]);
   if (ElDif[i] != 0.0)
    polyel[b][i] += (GaussRand() * ElDif[i]);
   } /* if needs to be perturbed */
  Vtx->El[VtxNum] = polyel[b][i];
  Vtx->RelEl[VtxNum] = polyrelel[b][i];
  Vtx->Lat[VtxNum] = polylat[b][i];
  Vtx->Lon[VtxNum] = polylon[b][i];
  Vtx->Cld[VtxNum] = polycld[b][i];

#ifdef HJDFHAKJHLKAJSD
// temporary code to compute new Use, Pert and Edge values

  if (StudyVertex == 0)
   {  
   Vtx->Use[VtxNum] = -1;
   Vtx->Pert[VtxNum] = 1;
   for (j=0; j<OVtx; j++)
    {
    if (fabs(Vtx->Lat[VtxNum] - Vtx->Lat[j]) < .00000001
	 && fabs(Vtx->Lon[VtxNum] - Vtx->Lon[j]) < .00000001)
     {
     Vtx->Use[VtxNum] = j;
     if (fabs(Vtx->El[VtxNum] - Vtx->El[j]) < .00000001)
      {
      Vtx->Pert[VtxNum] = 0;
      break;
      }
     } /* if found same lat/lon */
    } /* for j=... */

   if (Vtx->Lat[VtxNum] == Vtx->Lat[0])
    {
    if (Vtx->Lon[VtxNum] == Vtx->Lon[0])
     Vtx->Edge[VtxNum] = 0;
    else if (Vtx->Lon[VtxNum] == Vtx->Lon[1])
     Vtx->Edge[VtxNum] = 1;
    else
     Vtx->Edge[VtxNum] = 3;
    }
   else if (Vtx->Lon[VtxNum] == Vtx->Lon[0])
    {
    if (Vtx->Lat[VtxNum] == Vtx->Lat[2])
     Vtx->Edge[VtxNum] = 2;
    else
     Vtx->Edge[VtxNum] = 4;
    }
   else if (fabs(fabs((Vtx->Lon[VtxNum] - Vtx->Lon[0]) / (Vtx->Lon[1] - Vtx->Lon[0]))
	+ fabs((Vtx->Lat[VtxNum] - Vtx->Lat[0]) / (Vtx->Lat[2] - Vtx->Lat[0])) - 1.0)
	< .0082)
    Vtx->Edge[VtxNum] = 5;
   else
    {
    Vtx->Edge[VtxNum] = 6;
    }

   if (StudyVertex == 0)
    {
    printf("%d %d %5d %4d %4d %4d %f %f %f\n", b, polyct[b], VtxNum, Vtx->Use[VtxNum],
	Vtx->Pert[VtxNum], Vtx->Edge[VtxNum], Vtx->El[VtxNum], Vtx->Lat[VtxNum], Vtx->Lon[VtxNum]);
    } /* if */

   } /* if StudyVertex */
#endif
  } /* for i=0... */

} /* FractPoly_Divide() */

/*********************************************************************/

double GaussRand(void)
{
short i;
double sum = 0.0;

 for (i=0; i<4; i++)
  {
  sum += drand48();
  } /* for i=0... */

 return (1.73205 * sum - 3.46410);

} /* GaussRand() */

/*********************************************************************/

STATIC_FCN void Poly_Divide(void) // used locally only -> static, AF 23.7.2021
{

 switch (polyct[b])
  {
  case 0:
   {
   polyx[b][0] = (polyx[b-1][1] + polyx[b-1][2]) / 2.0;
   polyx[b][1] = (polyx[b-1][0] + polyx[b-1][2]) / 2.0;
   polyx[b][2] = (polyx[b-1][0] + polyx[b-1][1]) / 2.0;
   polyy[b][0] = (polyy[b-1][1] + polyy[b-1][2]) / 2.0;
   polyy[b][1] = (polyy[b-1][0] + polyy[b-1][2]) / 2.0;
   polyy[b][2] = (polyy[b-1][0] + polyy[b-1][1]) / 2.0;
   polyel[b][0] = (polyel[b-1][1] + polyel[b-1][2]) / 2.0;
   polyel[b][1] = (polyel[b-1][0] + polyel[b-1][2]) / 2.0;
   polyel[b][2] = (polyel[b-1][0] + polyel[b-1][1]) / 2.0;
   polyq[b][0] = (polyq[b-1][1] + polyq[b-1][2]) / 2.0;
   polyq[b][1] = (polyq[b-1][0] + polyq[b-1][2]) / 2.0;
   polyq[b][2] = (polyq[b-1][0] + polyq[b-1][1]) / 2.0;
   polyslope[b][0] = (polyslope[b-1][1] + polyslope[b-1][2]) / 2.0;
   polyslope[b][1] = (polyslope[b-1][0] + polyslope[b-1][2]) / 2.0;
   polyslope[b][2] = (polyslope[b-1][0] + polyslope[b-1][1]) / 2.0;
   polyrelel[b][0] = (polyrelel[b-1][1] + polyrelel[b-1][2]) / 2.0;
   polyrelel[b][1] = (polyrelel[b-1][0] + polyrelel[b-1][2]) / 2.0;
   polyrelel[b][2] = (polyrelel[b-1][0] + polyrelel[b-1][1]) / 2.0;
   polydiplat[b][0] = (polydiplat[b-1][1] + polydiplat[b-1][2]) / 2.0;
   polydiplat[b][1] = (polydiplat[b-1][0] + polydiplat[b-1][2]) / 2.0;
   polydiplat[b][2] = (polydiplat[b-1][0] + polydiplat[b-1][1]) / 2.0;
   polydiplon[b][0] = (polydiplon[b-1][1] + polydiplon[b-1][2]) / 2.0;
   polydiplon[b][1] = (polydiplon[b-1][0] + polydiplon[b-1][2]) / 2.0;
   polydiplon[b][2] = (polydiplon[b-1][0] + polydiplon[b-1][1]) / 2.0;
   polylat[b][0] = (polylat[b-1][1] + polylat[b-1][2]) / 2.0;
   polylat[b][1] = (polylat[b-1][0] + polylat[b-1][2]) / 2.0;
   polylat[b][2] = (polylat[b-1][0] + polylat[b-1][1]) / 2.0;
   polylon[b][0] = (polylon[b-1][1] + polylon[b-1][2]) / 2.0;
   polylon[b][1] = (polylon[b-1][0] + polylon[b-1][2]) / 2.0;
   polylon[b][2] = (polylon[b-1][0] + polylon[b-1][1]) / 2.0;
   polycld[b][0] = (polycld[b-1][1] + polycld[b-1][2]) / 2.0;
   polycld[b][1] = (polycld[b-1][0] + polycld[b-1][2]) / 2.0;
   polycld[b][2] = (polycld[b-1][0] + polycld[b-1][1]) / 2.0;
   break;
   } /* case 0 */
  case 1:
   {
   polyx[b][0] = polyx[b-1][0];
   polyx[b][1] = (polyx[b-1][0] + polyx[b-1][1]) / 2.0;
   polyx[b][2] = (polyx[b-1][0] + polyx[b-1][2]) / 2.0;
   polyy[b][0] = polyy[b-1][0];
   polyy[b][1] = (polyy[b-1][0] + polyy[b-1][1]) / 2.0;
   polyy[b][2] = (polyy[b-1][0] + polyy[b-1][2]) / 2.0;
   polyel[b][0] = polyel[b-1][0];
   polyel[b][1] = (polyel[b-1][0] + polyel[b-1][1]) / 2.0;
   polyel[b][2] = (polyel[b-1][0] + polyel[b-1][2]) / 2.0;
   polyq[b][0] = polyq[b-1][0];
   polyq[b][1] = (polyq[b-1][0] + polyq[b-1][1]) / 2.0;
   polyq[b][2] = (polyq[b-1][0] + polyq[b-1][2]) / 2.0;
   polyslope[b][0] = polyslope[b-1][0];
   polyslope[b][1] = (polyslope[b-1][0] + polyslope[b-1][1]) / 2.0;
   polyslope[b][2] = (polyslope[b-1][0] + polyslope[b-1][2]) / 2.0;
   polyrelel[b][0] = polyrelel[b-1][0];
   polyrelel[b][1] = (polyrelel[b-1][0] + polyrelel[b-1][1]) / 2.0;
   polyrelel[b][2] = (polyrelel[b-1][0] + polyrelel[b-1][2]) / 2.0;
   polydiplat[b][0] = polydiplat[b-1][0];
   polydiplat[b][1] = (polydiplat[b-1][0] + polydiplat[b-1][1]) / 2.0;
   polydiplat[b][2] = (polydiplat[b-1][0] + polydiplat[b-1][2]) / 2.0;
   polydiplon[b][0] = polydiplon[b-1][0];
   polydiplon[b][1] = (polydiplon[b-1][0] + polydiplon[b-1][1]) / 2.0;
   polydiplon[b][2] = (polydiplon[b-1][0] + polydiplon[b-1][2]) / 2.0;
   polylat[b][0] = polylat[b-1][0];
   polylat[b][1] = (polylat[b-1][0] + polylat[b-1][1]) / 2.0;
   polylat[b][2] = (polylat[b-1][0] + polylat[b-1][2]) / 2.0;
   polylon[b][0] = polylon[b-1][0];
   polylon[b][1] = (polylon[b-1][0] + polylon[b-1][1]) / 2.0;
   polylon[b][2] = (polylon[b-1][0] + polylon[b-1][2]) / 2.0;
   polycld[b][0] = polycld[b-1][0];
   polycld[b][1] = (polycld[b-1][0] + polycld[b-1][1]) / 2.0;
   polycld[b][2] = (polycld[b-1][0] + polycld[b-1][2]) / 2.0;
   break;
   } /* case 1 */
  case 2:
   {
   polyx[b][0] = (polyx[b-1][2] + polyx[b-1][0]) / 2.0;
   polyx[b][1] = (polyx[b-1][2] + polyx[b-1][1]) / 2.0;
   polyx[b][2] = polyx[b-1][2];
   polyy[b][0] = (polyy[b-1][2] + polyy[b-1][0]) / 2.0;
   polyy[b][1] = (polyy[b-1][2] + polyy[b-1][1]) / 2.0;
   polyy[b][2] = polyy[b-1][2];
   polyel[b][0] = (polyel[b-1][2] + polyel[b-1][0]) / 2.0;
   polyel[b][1] = (polyel[b-1][2] + polyel[b-1][1]) / 2.0;
   polyel[b][2] = polyel[b-1][2];
   polyq[b][0] = (polyq[b-1][2] + polyq[b-1][0]) / 2.0;
   polyq[b][1] = (polyq[b-1][2] + polyq[b-1][1]) / 2.0;
   polyq[b][2] = polyq[b-1][2];
   polyslope[b][0] = (polyslope[b-1][2] + polyslope[b-1][0]) / 2.0;
   polyslope[b][1] = (polyslope[b-1][2] + polyslope[b-1][1]) / 2.0;
   polyslope[b][2] = polyslope[b-1][2];
   polyrelel[b][0] = (polyrelel[b-1][2] + polyrelel[b-1][0]) / 2.0;
   polyrelel[b][1] = (polyrelel[b-1][2] + polyrelel[b-1][1]) / 2.0;
   polyrelel[b][2] = polyrelel[b-1][2];
   polydiplat[b][0] = (polydiplat[b-1][2] + polydiplat[b-1][0]) / 2.0;
   polydiplat[b][1] = (polydiplat[b-1][2] + polydiplat[b-1][1]) / 2.0;
   polydiplat[b][2] = polydiplat[b-1][2];
   polydiplon[b][0] = (polydiplon[b-1][2] + polydiplon[b-1][0]) / 2.0;
   polydiplon[b][1] = (polydiplon[b-1][2] + polydiplon[b-1][1]) / 2.0;
   polydiplon[b][2] = polydiplon[b-1][2];
   polylat[b][0] = (polylat[b-1][2] + polylat[b-1][0]) / 2.0;
   polylat[b][1] = (polylat[b-1][2] + polylat[b-1][1]) / 2.0;
   polylat[b][2] = polylat[b-1][2];
   polylon[b][0] = (polylon[b-1][2] + polylon[b-1][0]) / 2.0;
   polylon[b][1] = (polylon[b-1][2] + polylon[b-1][1]) / 2.0;
   polylon[b][2] = polylon[b-1][2];
   polycld[b][0] = (polycld[b-1][2] + polycld[b-1][0]) / 2.0;
   polycld[b][1] = (polycld[b-1][2] + polycld[b-1][1]) / 2.0;
   polycld[b][2] = polycld[b-1][2];
   break;
   } /* case 2 */
  case 3:
   {
   polyx[b][0] = (polyx[b-1][1] + polyx[b-1][0]) / 2.0;
   polyx[b][1] = polyx[b-1][1];
   polyx[b][2] = (polyx[b-1][1] + polyx[b-1][2]) / 2.0;
   polyy[b][0] = (polyy[b-1][1] + polyy[b-1][0]) / 2.0;
   polyy[b][1] = polyy[b-1][1];
   polyy[b][2] = (polyy[b-1][1] + polyy[b-1][2]) / 2.0;
   polyel[b][0] = (polyel[b-1][1] + polyel[b-1][0]) / 2.0;
   polyel[b][1] = polyel[b-1][1];
   polyel[b][2] = (polyel[b-1][1] + polyel[b-1][2]) / 2.0;
   polyq[b][0] = (polyq[b-1][1] + polyq[b-1][0]) / 2.0;
   polyq[b][1] = polyq[b-1][1];
   polyq[b][2] = (polyq[b-1][1] + polyq[b-1][2]) / 2.0;
   polyslope[b][0] = (polyslope[b-1][1] + polyslope[b-1][0]) / 2.0;
   polyslope[b][1] = polyslope[b-1][1];
   polyslope[b][2] = (polyslope[b-1][1] + polyslope[b-1][2]) / 2.0;
   polyrelel[b][0] = (polyrelel[b-1][1] + polyrelel[b-1][0]) / 2.0;
   polyrelel[b][1] = polyrelel[b-1][1];
   polyrelel[b][2] = (polyrelel[b-1][1] + polyrelel[b-1][2]) / 2.0;
   polydiplat[b][0] = (polydiplat[b-1][1] + polydiplat[b-1][0]) / 2.0;
   polydiplat[b][1] = polydiplat[b-1][1];
   polydiplat[b][2] = (polydiplat[b-1][1] + polydiplat[b-1][2]) / 2.0;
   polydiplon[b][0] = (polydiplon[b-1][1] + polydiplon[b-1][0]) / 2.0;
   polydiplon[b][1] = polydiplon[b-1][1];
   polydiplon[b][2] = (polydiplon[b-1][1] + polydiplon[b-1][2]) / 2.0;
   polylat[b][0] = (polylat[b-1][1] + polylat[b-1][0]) / 2.0;
   polylat[b][1] = polylat[b-1][1];
   polylat[b][2] = (polylat[b-1][1] + polylat[b-1][2]) / 2.0;
   polylon[b][0] = (polylon[b-1][1] + polylon[b-1][0]) / 2.0;
   polylon[b][1] = polylon[b-1][1];
   polylon[b][2] = (polylon[b-1][1] + polylon[b-1][2]) / 2.0;
   polycld[b][0] = (polycld[b-1][1] + polycld[b-1][0]) / 2.0;
   polycld[b][1] = polycld[b-1][1];
   polycld[b][2] = (polycld[b-1][1] + polycld[b-1][2]) / 2.0;
   break;
   } /* case 3 */
  } /* switch */

} /* Poly_Divide() */

/***********************************************************************/

void recurse(struct elmapheaderV101 *map, struct Window *win, short MapAsSFC,
	struct CloudData *CD, struct FaceData *Data)
{

 for (polyct[b]=0; polyct[b]<4; polyct[b]++)
  {
  polydivide();
  if (maxfract > b)
   {
   b ++;
   recurse(map, win, MapAsSFC, CD, Data);
   } /* if */
  else
   {
   PointSort2();
   xx[0] = polyx[b][0]; yy[0] = polyy[b][0];
   xx[1] = polyx[b][1]; yy[1] = polyy[b][1];
   xx[2] = polyx[b][2]; yy[2] = polyy[b][2];
   el = (polyel[b][0] + polyel[b][1] + polyel[b][2]) / 3.0;
   qqq = (polyq[b][0] + polyq[b][1] + polyq[b][2]) / 3.0;
   facelat = (polylat[b][0] + polylat[b][1] + polylat[b][2]) / 3.0;
   facelong = (polylon[b][0] + polylon[b][1] + polylon[b][2]) / 3.0;
   cloudcover = (polycld[b][0] + polycld[b][1] + polycld[b][2]) / 3.0;
   treerand4 = drand48();
   treerand3 = drand48();
   treerand2 = drand48();
   treerand = drand48();
   Random = -.2 * treerand + .1;
   Data->El[0] = polyel[0][0];
   Data->El[1] = polyel[0][1];
   Data->El[2] = polyel[0][2];

   if (settings.perturb)
    {
    if (Random > .066)
     {			
     diplat = dlat;
     diplong = dlong;
     slope = dslope;
     } /* if */
    if (dir <= 1) diplat += (Random + Random * slope * 2.0);
    if (diplat > maxdlat) diplat = maxdlat;
    else if (diplat < mindlat) diplat = mindlat;
    if (dir)   diplong += (Random + Random * slope * 2.0);
    if (diplong > maxdlong) dlong = maxdlong;
    else if (diplong < mindlong) diplong = mindlong;
/* this is for creating lakes and you don't want no slope areas changed
** 0.0174 corresponds to 1 in radians */
    if (slope > 0.0174)
     {
     if (dir==1) slope += ((Random + Random * slope * 2.0) * .7);
     else  slope += ((Random + Random * slope * 2.0) * .35);
     slope = abs(slope) < HalfPi ? abs(slope): HalfPi - .001;
     } /* if */
    } /* if */
   dir = dir < 2 ? dir + 1: 0;
   MapTopo(map, win, MapAsSFC, 0, 1, &Data->El[0]);
   } /* else */
  } /* for polyct[b] = 0... */

 b--;

} /* recurse() */

/***********************************************************************/

STATIC_FCN void polydivide(void) // used locally only -> static, AF 23.7.2021
{

 switch (polyct[b])
  {
  case 0:
   {
   polyx[b][2] = (polyx[b-1][0] + polyx[b-1][1]) / 2.0;
   polyx[b][1] = (polyx[b-1][0] + polyx[b-1][2]) / 2.0;
   polyx[b][0] = (polyx[b-1][1] + polyx[b-1][2]) / 2.0;
   polyy[b][2] = (polyy[b-1][0] + polyy[b-1][1]) / 2.0;
   polyy[b][1] = (polyy[b-1][0] + polyy[b-1][2]) / 2.0;
   polyy[b][0] = (polyy[b-1][1] + polyy[b-1][2]) / 2.0;
   polyel[b][2] = (polyel[b-1][0] + polyel[b-1][1]) / 2.0;
   polyel[b][1] = (polyel[b-1][0] + polyel[b-1][2]) / 2.0;
   polyel[b][0] = (polyel[b-1][1] + polyel[b-1][2]) / 2.0;
   polyq[b][2] = (polyq[b-1][0] + polyq[b-1][1]) / 2.0;
   polyq[b][1] = (polyq[b-1][0] + polyq[b-1][2]) / 2.0;
   polyq[b][0] = (polyq[b-1][1] + polyq[b-1][2]) / 2.0;
   polylat[b][2] = (polylat[b-1][0] + polylat[b-1][1]) / 2.0;
   polylat[b][1] = (polylat[b-1][0] + polylat[b-1][2]) / 2.0;
   polylat[b][0] = (polylat[b-1][1] + polylat[b-1][2]) / 2.0;
   polylon[b][2] = (polylon[b-1][0] + polylon[b-1][1]) / 2.0;
   polylon[b][1] = (polylon[b-1][0] + polylon[b-1][2]) / 2.0;
   polylon[b][0] = (polylon[b-1][1] + polylon[b-1][2]) / 2.0;
   polycld[b][2] = (polycld[b-1][0] + polycld[b-1][1]) / 2.0;
   polycld[b][1] = (polycld[b-1][0] + polycld[b-1][2]) / 2.0;
   polycld[b][0] = (polycld[b-1][1] + polycld[b-1][2]) / 2.0;
   break;
   } /* case 0 */
  case 1:
   {
   polyx[b][0] = polyx[b-1][0];
   polyx[b][1] = (polyx[b-1][0] + polyx[b-1][1]) / 2.0;
   polyx[b][2] = (polyx[b-1][0] + polyx[b-1][2]) / 2.0;
   polyy[b][0] = polyy[b-1][0];
   polyy[b][1] = (polyy[b-1][0] + polyy[b-1][1]) / 2.0;
   polyy[b][2] = (polyy[b-1][0] + polyy[b-1][2]) / 2.0;
   polyel[b][0] = polyel[b-1][0];
   polyel[b][1] = (polyel[b-1][0] + polyel[b-1][1]) / 2.0;
   polyel[b][2] = (polyel[b-1][0] + polyel[b-1][2]) / 2.0;
   polyq[b][0] = polyq[b-1][0];
   polyq[b][1] = (polyq[b-1][0] + polyq[b-1][1]) / 2.0;
   polyq[b][2] = (polyq[b-1][0] + polyq[b-1][2]) / 2.0;
   polylat[b][0] = polylat[b-1][0];
   polylat[b][1] = (polylat[b-1][0] + polylat[b-1][1]) / 2.0;
   polylat[b][2] = (polylat[b-1][0] + polylat[b-1][2]) / 2.0;
   polylon[b][0] = polylon[b-1][0];
   polylon[b][1] = (polylon[b-1][0] + polylon[b-1][1]) / 2.0;
   polylon[b][2] = (polylon[b-1][0] + polylon[b-1][2]) / 2.0;
   polycld[b][0] = polycld[b-1][0];
   polycld[b][1] = (polycld[b-1][0] + polycld[b-1][1]) / 2.0;
   polycld[b][2] = (polycld[b-1][0] + polycld[b-1][2]) / 2.0;
   break;
   } /* case 1 */
  case 2:
   {
   polyx[b][0] = (polyx[b-1][2] + polyx[b-1][0]) / 2.0;
   polyx[b][1] = (polyx[b-1][2] + polyx[b-1][1]) / 2.0;
   polyx[b][2] = polyx[b-1][2];
   polyy[b][0] = (polyy[b-1][2] + polyy[b-1][0]) / 2.0;
   polyy[b][1] = (polyy[b-1][2] + polyy[b-1][1]) / 2.0;
   polyy[b][2] = polyy[b-1][2];
   polyel[b][0] = (polyel[b-1][2] + polyel[b-1][0]) / 2.0;
   polyel[b][1] = (polyel[b-1][2] + polyel[b-1][1]) / 2.0;
   polyel[b][2] = polyel[b-1][2];
   polyq[b][0] = (polyq[b-1][2] + polyq[b-1][0]) / 2.0;
   polyq[b][1] = (polyq[b-1][2] + polyq[b-1][1]) / 2.0;
   polyq[b][2] = polyq[b-1][2];
   polylat[b][0] = (polylat[b-1][2] + polylat[b-1][0]) / 2.0;
   polylat[b][1] = (polylat[b-1][2] + polylat[b-1][1]) / 2.0;
   polylat[b][2] = polylat[b-1][2];
   polylon[b][0] = (polylon[b-1][2] + polylon[b-1][0]) / 2.0;
   polylon[b][1] = (polylon[b-1][2] + polylon[b-1][1]) / 2.0;
   polylon[b][2] = polylon[b-1][2];
   polycld[b][0] = (polycld[b-1][2] + polycld[b-1][0]) / 2.0;
   polycld[b][1] = (polycld[b-1][2] + polycld[b-1][1]) / 2.0;
   polycld[b][2] = polycld[b-1][2];
   break;
   } /* case 2 */
  case 3:
   {
   polyx[b][0] = (polyx[b-1][1] + polyx[b-1][0]) / 2.0;
   polyx[b][1] = polyx[b-1][1];
   polyx[b][2] = (polyx[b-1][1] + polyx[b-1][2]) / 2.0;
   polyy[b][0] = (polyy[b-1][1] + polyy[b-1][0]) / 2.0;
   polyy[b][1] = polyy[b-1][1];
   polyy[b][2] = (polyy[b-1][1] + polyy[b-1][2]) / 2.0;
   polyel[b][0] = (polyel[b-1][1] + polyel[b-1][0]) / 2.0;
   polyel[b][1] = polyel[b-1][1];
   polyel[b][2] = (polyel[b-1][1] + polyel[b-1][2]) / 2.0;
   polyq[b][0] = (polyq[b-1][1] + polyq[b-1][0]) / 2.0;
   polyq[b][1] = polyq[b-1][1];
   polyq[b][2] = (polyq[b-1][1] + polyq[b-1][2]) / 2.0;
   polylat[b][0] = (polylat[b-1][1] + polylat[b-1][0]) / 2.0;
   polylat[b][1] = polylat[b-1][1];
   polylat[b][2] = (polylat[b-1][1] + polylat[b-1][2]) / 2.0;
   polylon[b][0] = (polylon[b-1][1] + polylon[b-1][0]) / 2.0;
   polylon[b][1] = polylon[b-1][1];
   polylon[b][2] = (polylon[b-1][1] + polylon[b-1][2]) / 2.0;
   polycld[b][0] = (polycld[b-1][1] + polycld[b-1][0]) / 2.0;
   polycld[b][1] = polycld[b-1][1];
   polycld[b][2] = (polycld[b-1][1] + polycld[b-1][2]) / 2.0;
   break;
   } /* case 3 */
  } /* switch */

} /* polydivide() */

/**************************************************************************/

void Point_Sort(void)
{

 if (polyy[b][1] < polyy[b][0])
  {
  swmem(&polyx[b][0], &polyx[b][1], 8);
  swmem(&polyy[b][0], &polyy[b][1], 8);
  swmem(&polyel[b][0], &polyel[b][1], 8);
  swmem(&polyq[b][0], &polyq[b][1], 8);
  swmem(&polyslope[b][0], &polyslope[b][1], 8);
  swmem(&polyrelel[b][0], &polyrelel[b][1], 8);
  swmem(&polydiplat[b][0], &polydiplat[b][1], 8);
  swmem(&polydiplon[b][0], &polydiplon[b][1], 8);
  swmem(&polylat[b][0], &polylat[b][1], 8);
  swmem(&polylon[b][0], &polylon[b][1], 8);
  swmem(&polycld[b][0], &polycld[b][1], 8);
  } /* if */
 if (polyy[b][2] < polyy[b][1])
  {
  swmem(&polyx[b][1], &polyx[b][2], 8);
  swmem(&polyy[b][1], &polyy[b][2], 8);
  swmem(&polyel[b][1], &polyel[b][2], 8);
  swmem(&polyq[b][1], &polyq[b][2], 8);
  swmem(&polyslope[b][1], &polyslope[b][2], 8);
  swmem(&polyrelel[b][1], &polyrelel[b][2], 8);
  swmem(&polydiplat[b][1], &polydiplat[b][2], 8);
  swmem(&polydiplon[b][1], &polydiplon[b][2], 8);
  swmem(&polylat[b][1], &polylat[b][2], 8);
  swmem(&polylon[b][1], &polylon[b][2], 8);
  swmem(&polycld[b][1], &polycld[b][2], 8);
  if (polyy[b][1] < polyy[b][0])
   {
   swmem(&polyx[b][0], &polyx[b][1], 8);
   swmem(&polyy[b][0], &polyy[b][1], 8);
   swmem(&polyel[b][0], &polyel[b][1], 8);
   swmem(&polyq[b][0], &polyq[b][1], 8);
   swmem(&polyslope[b][0], &polyslope[b][1], 8);
   swmem(&polyrelel[b][0], &polyrelel[b][1], 8);
   swmem(&polydiplat[b][0], &polydiplat[b][1], 8);
   swmem(&polydiplon[b][0], &polydiplon[b][1], 8);
   swmem(&polylat[b][0], &polylat[b][1], 8);
   swmem(&polylon[b][0], &polylon[b][1], 8);
   swmem(&polycld[b][0], &polycld[b][1], 8);
   } /* if */
  } /* if */

}/* Point_Sort() */

/**************************************************************************/

void FractPoint_Sort(double *Elev)
{

 if (polyy[b][1] < polyy[b][0])
  {
  swmem(&polyx[b][0], &polyx[b][1], 8);
  swmem(&polyy[b][0], &polyy[b][1], 8);
  swmem(&polyq[b][0], &polyq[b][1], 8);
  swmem(&polyel[b][0], &polyel[b][1], 8);
  swmem(&polyrelel[b][0], &polyrelel[b][1], 8);
  swmem(&polylat[b][0], &polylat[b][1], 8);
  swmem(&polylon[b][0], &polylon[b][1], 8);
  swmem(&polycld[b][0], &polycld[b][1], 8);
  swmem(&Elev[0], &Elev[1], 8);
  } /* if */
 if (polyy[b][2] < polyy[b][1])
  {
  swmem(&polyx[b][1], &polyx[b][2], 8);
  swmem(&polyy[b][1], &polyy[b][2], 8);
  swmem(&polyq[b][1], &polyq[b][2], 8);
  swmem(&polyel[b][1], &polyel[b][2], 8);
  swmem(&polyrelel[b][1], &polyrelel[b][2], 8);
  swmem(&polylat[b][1], &polylat[b][2], 8);
  swmem(&polylon[b][1], &polylon[b][2], 8);
  swmem(&polycld[b][1], &polycld[b][2], 8);
  swmem(&Elev[1], &Elev[2], 8);
  if (polyy[b][1] < polyy[b][0])
   {
   swmem(&polyx[b][0], &polyx[b][1], 8);
   swmem(&polyy[b][0], &polyy[b][1], 8);
   swmem(&polyq[b][0], &polyq[b][1], 8);
   swmem(&polyel[b][0], &polyel[b][1], 8);
   swmem(&polyrelel[b][0], &polyrelel[b][1], 8);
   swmem(&polylat[b][0], &polylat[b][1], 8);
   swmem(&polylon[b][0], &polylon[b][1], 8);
   swmem(&polycld[b][0], &polycld[b][1], 8);
   swmem(&Elev[0], &Elev[1], 8);
   } /* if */
  } /* if */

}/* FractPoint_Sort() */

/**********************************************************************/

void PointSort2(void)
{

 if (polyy[b][1] < polyy[b][0])
  {
  swmem(&polyx[b][0], &polyx[b][1], 8);
  swmem(&polyy[b][0], &polyy[b][1], 8);
  swmem(&polyel[b][0], &polyel[b][1], 8);
  swmem(&polyq[b][0], &polyq[b][1], 8);
  swmem(&polylat[b][0], &polylat[b][1], 8);
  swmem(&polylon[b][0], &polylon[b][1], 8);
  swmem(&polycld[b][0], &polycld[b][1], 8);
  } /* if */
 if (polyy[b][2] < polyy[b][1])
  {
  swmem(&polyx[b][1], &polyx[b][2], 8);
  swmem(&polyy[b][1], &polyy[b][2], 8);
  swmem(&polyel[b][1], &polyel[b][2], 8);
  swmem(&polyq[b][1], &polyq[b][2], 8);
  swmem(&polylat[b][1], &polylat[b][2], 8);
  swmem(&polylon[b][1], &polylon[b][2], 8);
  swmem(&polycld[b][1], &polycld[b][2], 8);
  if (polyy[b][1] < polyy[b][0])
   {
   swmem(&polyx[b][0], &polyx[b][1], 8);
   swmem(&polyy[b][0], &polyy[b][1], 8);
   swmem(&polyel[b][0], &polyel[b][1], 8);
   swmem(&polyq[b][0], &polyq[b][1], 8);
   swmem(&polylat[b][0], &polylat[b][1], 8);
   swmem(&polylon[b][0], &polylon[b][1], 8);
   swmem(&polycld[b][0], &polycld[b][1], 8);
   } /* if */
  } /* if */

}/* PointSort2() */

/**********************************************************************/

void SmoothFace_ColSet(struct elmapheaderV101 *map, struct faces **Face,
	long FaceColSize)
{
long i;

 Face[0] = map->face;
 Face[1] = map->face + FaceColSize;
 Face[2] = map->face + 2 * FaceColSize;

 for (i=0; i<FaceColSize; i++)
  {
  Face[0][i].slope = FLT_MAX;
  Face[1][i].slope = FLT_MAX;
  Face[2][i].slope = FLT_MAX;
  } /* for i=0... */

} /* SmoothFace_ColSet() */

/*********************************************************************/

void SmoothFace_IncrColSwap(struct elmapheaderV101 *map, struct faces **Face,
	struct faces *MaxBase, long FaceColSize)
{
long i;

 Face[0] += FaceColSize;	/* map->columns * 2 */
 Face[1] += FaceColSize;
 Face[2] += FaceColSize;
 if (Face[0] > MaxBase)		/* map->face + map->columns * 4) */
  Face[0] = map->face;
 else if (Face[1] > MaxBase)
  Face[1] = map->face;
 else if (Face[2] > MaxBase)
  Face[2] = map->face;

 for (i=0; i<FaceColSize; i++)
  Face[2][i].slope = FLT_MAX;

} /* SmoothFace_IncrColSwap() */

/**********************************************************************/

void SmoothFace_DecrColSwap(struct elmapheaderV101 *map, struct faces **Face,
	struct faces *MaxBase, long FaceColSize)
{
long i;

 Face[0] -= FaceColSize;	/* map->columns * 2 */
 Face[1] -= FaceColSize;
 Face[2] -= FaceColSize;
 if (Face[0] < map->face)		/* map->face + map->columns * 4) */
  Face[0] = MaxBase;
 else if (Face[1] < map->face)
  Face[1] = MaxBase;
 else if (Face[2] < map->face)
  Face[2] = MaxBase;

 for (i=0; i<FaceColSize; i++)
  Face[0][i].slope = FLT_MAX;

} /* SmoothFace_DecrColSwap() */

/*********************************************************************/

void FaceIndex_Set(struct faces **Face,	struct faces **FaceIndex, long Lc,
	short WhichFace)
{
long BaseFace;

 BaseFace = 2 * (Lc - WhichFace);

 FaceIndex[1] = Face[0] + BaseFace;
 FaceIndex[0] = FaceIndex[1] - 1;
 FaceIndex[2] = FaceIndex[1] + 1;
 FaceIndex[3] = FaceIndex[2] + 1; 
 FaceIndex[4] = FaceIndex[3] + 1;

 FaceIndex[7] = Face[1] + BaseFace;
 FaceIndex[6] = FaceIndex[7] - 1;
 FaceIndex[5] = FaceIndex[6] - 1;
 FaceIndex[8] = FaceIndex[7] + 1;
 FaceIndex[9] = FaceIndex[8] + 1;
 FaceIndex[10] = FaceIndex[9] + 1;

 FaceIndex[13] = Face[2] + BaseFace;
 FaceIndex[11] = FaceIndex[13] - 2;
 FaceIndex[12] = FaceIndex[11] + 1;
 FaceIndex[14] = FaceIndex[13] + 1;
 FaceIndex[15] = FaceIndex[14] + 1;

} /* FaceIndex_Set() */

/**********************************************************************/

STATIC_FCN void Vertex_Sum(struct faces *Vertex, struct faces *Face) // used locally only -> static, AF 19.7.2021
{

 Vertex->slope += Face->slope;
 Vertex->aspect += Face->aspect;
 Vertex->diplat += Face->diplat;
 Vertex->diplong += Face->diplong;

} /* Vertex_Sum() */

/**********************************************************************/

void VertexOne_Set(struct elmapheaderV101 *map, struct faces **FaceIndex,
	struct faces *Vertex)
{
short i, Pts[3] = {0, 0, 0}, lastcol;

 lastcol = map->columns - 2;

 for (i=0; i<3; i++)
  {
  Vertex[i].slope = 0.0;
  Vertex[i].aspect = 0.0;
  Vertex[i].diplat = 0.0;
  Vertex[i].diplong = 0.0;
  } /* for */

 if (map->Lr > 0)
  {
  if (map->Lc > 0)
   {
   if (FaceIndex[0]->slope == FLT_MAX) /* 0 */
    FaceTwo_Setup(map, FaceIndex[0], map->Lr - 1, map->Lc);
   Vertex_Sum(&Vertex[0], FaceIndex[0]);
   Pts[0] ++;
   } /* if not first column */

  if (FaceIndex[1]->slope == FLT_MAX) /* 0 */
   FaceOne_Setup(map, FaceIndex[1], map->Lr - 1, map->Lc);
  Vertex_Sum(&Vertex[0], FaceIndex[1]);
  Pts[0] ++;

  if (FaceIndex[2]->slope == FLT_MAX) /* 0 2 */
   FaceTwo_Setup(map, FaceIndex[2], map->Lr - 1, map->Lc + 1);
  Vertex_Sum(&Vertex[0], FaceIndex[2]);
  Vertex_Sum(&Vertex[2], FaceIndex[2]);
  Pts[0] ++;
  Pts[2] ++;

  if (map->Lc < lastcol)
   {
   if (FaceIndex[3]->slope == FLT_MAX) /* 2 */
    FaceOne_Setup(map, FaceIndex[3], map->Lr - 1, map->Lc + 1);
   Vertex_Sum(&Vertex[2], FaceIndex[3]);
   Pts[2] ++;

   if (FaceIndex[4]->slope == FLT_MAX) /* 2 */
    FaceTwo_Setup(map, FaceIndex[4], map->Lr - 1, map->Lc + 2);
   Vertex_Sum(&Vertex[2], FaceIndex[4]);
   Pts[2] ++;
   } /* if not last column */
  } /* if not first row of map */


 if (map->Lc > 0)
  {
  if (FaceIndex[5]->slope == FLT_MAX) /* 0 */
   FaceOne_Setup(map, FaceIndex[5], map->Lr, map->Lc - 1);
  Vertex_Sum(&Vertex[0], FaceIndex[5]);
  Pts[0] ++;
  
  if (FaceIndex[6]->slope == FLT_MAX) /* 0 1 */
   FaceTwo_Setup(map, FaceIndex[6], map->Lr, map->Lc);
  Vertex_Sum(&Vertex[0], FaceIndex[6]);
  Vertex_Sum(&Vertex[1], FaceIndex[6]);
  Pts[0] ++;
  Pts[1] ++;
  } /* if not first column */

 if (FaceIndex[7]->slope == FLT_MAX) /* 0 2 1 */
  FaceOne_Setup(map, FaceIndex[7], map->Lr, map->Lc);
 Vertex_Sum(&Vertex[0], FaceIndex[7]);
 Vertex_Sum(&Vertex[1], FaceIndex[7]);
 Vertex_Sum(&Vertex[2], FaceIndex[7]);
 Pts[0] ++;
 Pts[1] ++;
 Pts[2] ++;
  
 if (FaceIndex[8]->slope == FLT_MAX) /* 2 1 */
  FaceTwo_Setup(map, FaceIndex[8], map->Lr, map->Lc + 1);
 Vertex_Sum(&Vertex[1], FaceIndex[8]);
 Vertex_Sum(&Vertex[2], FaceIndex[8]);
 Pts[1] ++;
 Pts[2] ++;

 if (map->Lc < lastcol)
  {
  if (FaceIndex[9]->slope == FLT_MAX) /* 2 */
   FaceOne_Setup(map, FaceIndex[9], map->Lr, map->Lc + 1);
  Vertex_Sum(&Vertex[2], FaceIndex[9]);
  Pts[2] ++;
  } /* if not last column */


 if (map->Lr < map->rows - 1)
  {
  if (map->Lc > 0)
   {
   if (FaceIndex[11]->slope == FLT_MAX) /* 1 */
    FaceOne_Setup(map, FaceIndex[11], map->Lr + 1, map->Lc - 1);
   Vertex_Sum(&Vertex[1], FaceIndex[11]);
   Pts[1] ++;

   if (FaceIndex[12]->slope == FLT_MAX) /* 1 */
    FaceTwo_Setup(map, FaceIndex[12], map->Lr + 1, map->Lc);
   Vertex_Sum(&Vertex[1], FaceIndex[12]);
   Pts[1] ++;
   } /* if not first column */

  if (FaceIndex[13]->slope == FLT_MAX) /* 1 */
   FaceOne_Setup(map, FaceIndex[13], map->Lr + 1, map->Lc);
  Vertex_Sum(&Vertex[1], FaceIndex[13]);
  Pts[1] ++;
  } /* if not last row */

 for (i=0; i< 3; i++)
  {
  Vertex[i].slope /= Pts[i];
  Vertex[i].aspect /= Pts[i];
  Vertex[i].diplat /= Pts[i];
  Vertex[i].diplong /= Pts[i];
  } /* for i=0... */

} /* VertexOne_Set() */

/**********************************************************************/

void VertexTwo_Set(struct elmapheaderV101 *map, struct faces **FaceIndex,
	struct faces *Vertex)
{
short i, Pts[3] = {0, 0, 0}, lastcol;

 lastcol = map->columns - 1;

 for (i=0; i<3; i++)
  {
  Vertex[i].slope = 0.0;
  Vertex[i].aspect = 0.0;
  Vertex[i].diplat = 0.0;
  Vertex[i].diplong = 0.0;
  } /* for */

 if (map->Lr > 0)
  {
  if (FaceIndex[2]->slope == FLT_MAX) /* 1 */
   FaceTwo_Setup(map, FaceIndex[2], map->Lr - 1, map->Lc);
  Vertex_Sum(&Vertex[1], FaceIndex[2]);
  Pts[1] ++;

  if (map->Lc < lastcol)
   {
   if (FaceIndex[3]->slope == FLT_MAX) /* 1 */
    FaceOne_Setup(map, FaceIndex[3], map->Lr - 1, map->Lc);
   Vertex_Sum(&Vertex[1], FaceIndex[3]);
   Pts[1] ++;

   if (FaceIndex[4]->slope == FLT_MAX) /* 1 */
    FaceTwo_Setup(map, FaceIndex[4], map->Lr -1, map->Lc + 1);
   Vertex_Sum(&Vertex[1], FaceIndex[4]);
   Pts[1] ++;
   } /* if not last column */
  } /* if not first row of map */

 if (map->Lc > 1)
  {
  if (FaceIndex[6]->slope == FLT_MAX) /* 2 */
   FaceTwo_Setup(map, FaceIndex[6], map->Lr, map->Lc - 1);
  Vertex_Sum(&Vertex[2], FaceIndex[6]);
  Pts[2] ++;
  } /* if not first column */

 if (FaceIndex[7]->slope == FLT_MAX) /* 1 2 */
  FaceOne_Setup(map, FaceIndex[7], map->Lr, map->Lc - 1);
 Vertex_Sum(&Vertex[1], FaceIndex[7]);
 Vertex_Sum(&Vertex[2], FaceIndex[7]);
 Pts[1] ++;
 Pts[2] ++;

 if (FaceIndex[8]->slope == FLT_MAX) /* 1 2 0 */
  FaceTwo_Setup(map, FaceIndex[8], map->Lr, map->Lc);
 Vertex_Sum(&Vertex[0], FaceIndex[8]);
 Vertex_Sum(&Vertex[1], FaceIndex[8]);
 Vertex_Sum(&Vertex[2], FaceIndex[8]);
 Pts[0] ++;
 Pts[1] ++;
 Pts[2] ++;

 if (map->Lc < lastcol)
  {
  if (FaceIndex[9]->slope == FLT_MAX) /* 1 0 */
   FaceOne_Setup(map, FaceIndex[9], map->Lr, map->Lc);
  Vertex_Sum(&Vertex[0], FaceIndex[9]);
  Vertex_Sum(&Vertex[1], FaceIndex[9]);
  Pts[0] ++;
  Pts[1] ++;

  if (FaceIndex[10]->slope == FLT_MAX) /* 0 */
   FaceTwo_Setup(map, FaceIndex[10], map->Lr, map->Lc + 1);
  Vertex_Sum(&Vertex[0], FaceIndex[10]);
  Pts[0] ++;
  } /* if not last column */


 if (map->Lr < map->rows - 1)
  {
  if (map->Lc > 1)
   {
   if (FaceIndex[11]->slope == FLT_MAX) /* 2 */
    FaceOne_Setup(map, FaceIndex[11], map->Lr + 1, map->Lc - 2);
   Vertex_Sum(&Vertex[2], FaceIndex[11]);
   Pts[2] ++;

   if (FaceIndex[12]->slope == FLT_MAX) /* 2 */
    FaceTwo_Setup(map, FaceIndex[12], map->Lr + 1, map->Lc - 1);
   Vertex_Sum(&Vertex[2], FaceIndex[12]);
   Pts[2] ++;
   }

  if (FaceIndex[13]->slope == FLT_MAX) /* 2 0 */
   FaceOne_Setup(map, FaceIndex[13], map->Lr + 1, map->Lc - 1);
  Vertex_Sum(&Vertex[0], FaceIndex[13]);
  Vertex_Sum(&Vertex[2], FaceIndex[13]);
  Pts[0] ++;
  Pts[2] ++;

  if (FaceIndex[14]->slope == FLT_MAX) /* 0 */
   FaceTwo_Setup(map, FaceIndex[14], map->Lr + 1, map->Lc);
  Vertex_Sum(&Vertex[0], FaceIndex[14]);
  Pts[0] ++;

  if (map->Lc < lastcol)
   {
   if (FaceIndex[15]->slope == FLT_MAX) /* 0 */
    FaceOne_Setup(map, FaceIndex[15], map->Lr + 1, map->Lc);
   Vertex_Sum(&Vertex[0], FaceIndex[15]);
   Pts[0] ++;
   } /* if not last column */
  } /* if not last row */

 for (i=0; i< 3; i++)
  {
  Vertex[i].slope /= Pts[i];
  Vertex[i].aspect /= Pts[i];
  Vertex[i].diplat /= Pts[i];
  Vertex[i].diplong /= Pts[i];
  } /* for i=0... */

} /* VertexTwo_Set() */

/**********************************************************************/

void VertexIndexFaceOne_EdgeSet(struct elmapheaderV101 *map,
	struct VertexIndex *Vtx, long MaxFract)
{
long lastcol, lastrow, ColSize, CurPoly, CurPolyPC, CurPolyMC;
short Index[12];

 lastcol = map->columns - 2;
 lastrow = map->rows - 1;

 if (! map->fractal)
  {
  Vtx->Max[5] = Vtx->Max[6] = MaxFract;
  if (map->Lr > 0)
   {
   Vtx->Max[4] = MaxFract;
   if (map->Lc > 0)
    {
    Vtx->Max[0] = MaxFract;
    }
   else
    {
    Vtx->Max[0] = 0;
    }
   if (map->Lc < lastcol)
    {
    Vtx->Max[2] = MaxFract;
    }
   else
    {
    Vtx->Max[2] = 0;
    }
   }
  else
   {
   Vtx->Max[0] = Vtx->Max[2] = Vtx->Max[4] = 0;
   }

  if (map->Lc > 0)
   {
   Vtx->Max[3] = MaxFract;
   if (map->Lr < lastrow)
    {
    Vtx->Max[1] = MaxFract;
    }
   else
    {
    Vtx->Max[1] = 0;
    }
   }
  else
   {
   Vtx->Max[1] = Vtx->Max[3] = 0;
   }
  } /* if no fractal map - still need to set edges of map to 0 displacement */
 else
  {
  ColSize = (map->columns - 1) * 2;
  CurPoly = map->fracct;
  CurPolyPC = map->fracct + ColSize;
  CurPolyMC = map->fracct - ColSize;

/* a */
  if (map->Lr > 0)
   {
   Index[1] = map->fractal[CurPolyMC] > 0 ? map->fractal[CurPolyMC]: 0;
   Index[2] = map->fractal[CurPolyMC + 1] > 0 ? map->fractal[CurPolyMC + 1]: 0;
/* ac */
   if (map->Lc > 0)
    {
    Index[0] = map->fractal[CurPolyMC - 1] > 0 ? map->fractal[CurPolyMC - 1]: 0;
    } /* if not first column */
   else
    {
    Index[0] = 0;
    } /* else */
/* ad */
   if (map->Lc < lastcol)
    {
    Index[3] = map->fractal[CurPolyMC + 2] > 0 ? map->fractal[CurPolyMC + 2]: 0;
    Index[4] = map->fractal[CurPolyMC + 3] > 0 ? map->fractal[CurPolyMC + 3]: 0;
    } /* if not last column */
   else
    {
    Index[3] = 0;
    Index[4] = 0;
    } /* else */
   } /* if not first row */
  else
   {
   Index[0] = 0;
   Index[1] = 0;
   Index[2] = 0;
   Index[3] = 0;
   Index[4] = 0;
   } /* else first row */

/* b */
  if (map->Lr < lastrow)
   {
   Index[11] = map->fractal[CurPolyPC] > 0 ? map->fractal[CurPolyPC]: 0;
/* bc */
   if (map->Lc > 0)
    {
    Index[9] = map->fractal[CurPolyPC - 2] > 0 ? map->fractal[CurPolyPC - 2]: 0;
    Index[10] = map->fractal[CurPolyPC - 1] > 0 ? map->fractal[CurPolyPC - 1]: 0;
    } /* if not first column */
   else
    {
    Index[9] = 0;
    Index[10] = 0;
    } /* else */
   } /* if not last row */
  else
   {
   Index[9] = 0;
   Index[10] = 0;
   Index[11] = 0;
   } /* else last row */

/* c */
  if (map->Lc > 0)
   {
   Index[5] = map->fractal[CurPoly - 2] > 0 ? map->fractal[CurPoly - 2]: 0;
   Index[6] = map->fractal[CurPoly - 1] > 0 ? map->fractal[CurPoly - 1]: 0;
   } /* if not first column */
  else
   {
   Index[5] = 0;
   Index[6] = 0;
   } /* else first column */

/* d */
  if (map->Lc < lastcol)
   {
   Index[8] = map->fractal[CurPoly + 2] > 0 ? map->fractal[CurPoly + 2]: 0;
   } /* if not last column */
  else
   {
   Index[8] = 0;
   } /* else last column */

  Index[7] = map->fractal[CurPoly + 1] > 0 ? map->fractal[CurPoly + 1]: 0;

  Vtx->Max[0] = MinInt5(Index[0], Index[1], Index[2], Index[5], Index[6]);
  Vtx->Max[1] = MinInt5(Index[6], Index[7], Index[9], Index[10], Index[11]);
  Vtx->Max[2] = MinInt5(Index[2], Index[3], Index[4], Index[7], Index[8]);
  Vtx->Max[3] = Index[6];
  Vtx->Max[4] = Index[2];
  Vtx->Max[5] = Index[7];
  Vtx->Max[6] = 6;	/* don't have fractal tool for higher values */
  } /* else use fractal map */

} /* VertexIndexFaceOne_EdgeSet() */

/**********************************************************************/

void VertexIndexFaceTwo_EdgeSet(struct elmapheaderV101 *map,
	struct VertexIndex *Vtx, long MaxFract)
{
long lastcol, lastrow, ColSize, CurPoly, CurPolyPC, CurPolyMC;
short Index[12];

 lastcol = map->columns - 1;
 lastrow = map->rows - 1;

 if (! map->fractal)
  {
  Vtx->Max[5] = Vtx->Max[6] = MaxFract;
  if (map->Lr < lastrow)
   {
   Vtx->Max[4] = MaxFract;
   if (map->Lc < lastcol)
    {
    Vtx->Max[0] = MaxFract;
    }
   else
    {
    Vtx->Max[0] = 0;
    }
   if (map->Lc > 0)
    {
    Vtx->Max[2] = MaxFract;
    }
   else
    {
    Vtx->Max[2] = 0;
    }
   }
  else
   {
   Vtx->Max[0] = Vtx->Max[2] = Vtx->Max[4] = 0;
   }

  if (map->Lc < lastcol)
   {
   Vtx->Max[3] = MaxFract;
   if (map->Lr > 0)
    {
    Vtx->Max[1] = MaxFract;
    }
   else
    {
    Vtx->Max[1] = 0;
    }
   }
  else
   {
   Vtx->Max[1] = Vtx->Max[3] = 0;
   }
  } /* if no fractal map */
 else
  {
  ColSize = (map->columns - 1) * 2;
  CurPoly = map->fracct;
  CurPolyPC = map->fracct + ColSize;
  CurPolyMC = map->fracct - ColSize;

/* a */
  if (map->Lr > 0)
   {
   Index[0] = map->fractal[CurPolyMC] > 0 ? map->fractal[CurPolyMC]: 0;
/* ad */
   if (map->Lc < lastcol)
    {
    Index[1] = map->fractal[CurPolyMC + 1] > 0 ? map->fractal[CurPolyMC + 1]: 0;
    Index[2] = map->fractal[CurPolyMC + 2] > 0 ? map->fractal[CurPolyMC + 2]: 0;
    } /* if not last column */
   else
    {
    Index[1] = 0;
    Index[2] = 0;
    } /* else last column */
   } /* if not first row */
  else
   {
   Index[0] = 0;
   Index[1] = 0;
   Index[2] = 0;
   } /* else first row */

/* b */
  if (map->Lr < lastrow)
   {
   Index[9] = map->fractal[CurPolyPC - 1] > 0 ? map->fractal[CurPolyPC - 1]: 0;
   Index[10] = map->fractal[CurPolyPC] > 0 ? map->fractal[CurPolyPC]: 0;
/* bc */
   if (map->Lc > 1)
    {
    Index[7] = map->fractal[CurPolyPC - 3] > 0 ? map->fractal[CurPolyPC - 3]: 0;
    Index[8] = map->fractal[CurPolyPC - 2] > 0 ? map->fractal[CurPolyPC - 2]: 0;
    } /* if not first column */
   else
    {
    Index[7] = 0;
    Index[8] = 0;
    } /* else first column */
/* bd */
   if (map->Lc < lastcol)
    {
    Index[11] = map->fractal[CurPolyPC + 1] > 0 ? map->fractal[CurPolyPC + 1]: 0;
    } /* if not last column */
   else
    {
    Index[11] = 0;
    } /* else last column */
   } /* if not last row */
  else
   {
   Index[9] = 0;
   Index[7] = 0;
   Index[8] = 0;
   Index[10] = 0;
   Index[11] = 0;
   } /* else last row */

/* c */
  if (map->Lc > 1)
   {
   Index[3] = map->fractal[CurPoly - 2] > 0 ? map->fractal[CurPoly - 2]: 0;
   } /* if not first column */
  else
   {
   Index[3] = 0;
   } /* else first column */

/* d */
  if (map->Lc < lastcol)
   {
   Index[5] = map->fractal[CurPoly + 1] > 0 ? map->fractal[CurPoly + 1]: 0;
   Index[6] = map->fractal[CurPoly + 2] > 0 ? map->fractal[CurPoly + 2]: 0;
   } /* if not last column */
  else
   {
   Index[5] = 0;
   Index[6] = 0;
   } /* else last column */

  Index[4] = map->fractal[CurPoly - 1] > 0 ? map->fractal[CurPoly - 1]: 0;

  Vtx->Max[0] = MinInt5(Index[5], Index[6], Index[9], Index[10], Index[11]);
  Vtx->Max[1] = MinInt5(Index[0], Index[1], Index[2], Index[4], Index[5]);
  Vtx->Max[2] = MinInt5(Index[3], Index[4], Index[7], Index[8], Index[9]);
  Vtx->Max[3] = Index[5];
  Vtx->Max[4] = Index[9];
  Vtx->Max[5] = Index[4];
  Vtx->Max[6] = 6;	/* don't have fractal tool for higher values */
  } /* else use fractal map */

} /* VertexIndexFaceTwo_EdgeSet() */

/**********************************************************************/

STATIC_FCN long MinInt5(long Num1, long Num2, long Num3, long Num4, long Num5) // used locally only -> static, AF 23.7.2021
{
long MinVal;

 MinVal = min(Num1,   Num2);
 MinVal = min(MinVal, Num3);
 MinVal = min(MinVal, Num4);
 MinVal = min(MinVal, Num5);

 return(MinVal);

} /* MinInt5() */

/***********************************************************************/

void WaveAmp_Compute(double *Elev, double *Alt, double *PolyEl, double *PolyLat,
	double *PolyLon, struct WaveData *WD, short *MakeWater, short *MakeBeach,
	double Frame)
{
double dist, avglat, lonscale, d1, d2, WaveAmp = 0.0;
struct Wave *WV;

 if (WD)
  {
  WV = WD->Wave;

  while (WV)
   { /* compute wave height in meters */
   avglat = (WV->Pt.Lat + WD->LatOff + *PolyLat) / 2.0;
   lonscale = LATSCALE * cos(avglat * PiOver180);
   d1 = (WV->Pt.Lat + WD->LatOff - *PolyLat) * LATSCALE;
   d2 = (WV->Pt.Lon + WD->LonOff - *PolyLon) * lonscale;
   dist = sqrt(d1 * d1 + d2 * d2);
   WaveAmp += (WV->Amp * WD->Amp * sin(TwoPi * ((dist / WV->Length)
	- (Frame * WV->Velocity) / (WV->Length * FRAMES_PER_HOUR))));
   WV = WV->Next;
   } /* while */
  } /* if */
 *Elev = SeaLevel + WaveAmp;
 if (*Elev >= *PolyEl)
  {
  (*MakeWater) ++;
  *Alt = (WaveAmp + SeaLevel * PARC_RNDR_MOTION(14)) * .001;
  } /* if elevation is greater than or equal to water bottom */
 else
  {
  *Elev = *PolyEl;
  (*MakeBeach) ++;
  *Alt = *PolyEl * PARC_RNDR_MOTION(14) * .001;
  } /* else water bottom is higher than water */

} /* WaveAmp_Compute() */

/***********************************************************************/
