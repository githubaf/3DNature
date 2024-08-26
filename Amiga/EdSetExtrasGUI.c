/* EdSetExtrasGUI.c (ne gisedsetgui.c 14 Jan 1994 CXH)
** World Construction Set GUI for Settings Editing module.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"
#include <stdarg.h>

STATIC_FCN void Disable_ES_Window(void); // used locally only -> static, AF 26.7.2021

void Set_ES_Window(void)
{

/* Put values in strings */
  set(ES_Win->Str[0], MUIA_String_Contents, (IPTR)framepath); /* frame path */
  set(ES_Win->Str[1], MUIA_String_Contents, (IPTR)linepath); /* vector path */
  set(ES_Win->Str[2], MUIA_String_Contents, (IPTR)backgroundpath); /* background path */
  set(ES_Win->Str[3], MUIA_String_Contents, (IPTR)zbufferpath); /* z buffer path */
  set(ES_Win->Str[4], MUIA_String_Contents, (IPTR)colormappath); /* cmap path */
  set(ES_Win->Str[5], MUIA_String_Contents, (IPTR)backgroundfile); /* background file */
  set(ES_Win->Str[6], MUIA_String_Contents, (IPTR)zbufferfile); /* z buffer file */
  set(ES_Win->Str[7], MUIA_String_Contents, (IPTR)temppath); /* temp path */
  set(ES_Win->Str[8], MUIA_String_Contents, (IPTR)framefile); /* frame file */
  set(ES_Win->Str[10], MUIA_String_Contents, (IPTR)linefile); /* vector file */
  set(ES_Win->Str[11], MUIA_String_Contents, (IPTR)modelpath); /* forest model path */
  set(ES_Win->Str[12], MUIA_String_Contents, (IPTR)colormapfile); /* cmap file */
  set(ES_Win->Str[13], MUIA_String_Contents, (IPTR)deformpath); /* strata deformation map path */
  set(ES_Win->Str[14], MUIA_String_Contents, (IPTR)deformfile); /* strata deformation map file */

  set(ES_Win->IntStr[0], MUIA_String_Integer, settings.startframe); /* start frame */
  set(ES_Win->IntStr[1], MUIA_String_Integer, settings.maxframes); /* end frame */
  set(ES_Win->IntStr[2], MUIA_String_Integer, settings.stepframes); /* step frame */
  set(ES_Win->IntStr[3], MUIA_String_Integer, settings.rendersegs); /* segments */
  set(ES_Win->IntStr[4], MUIA_String_Integer, settings.scrnwidth); /* screen width */
  set(ES_Win->IntStr[5], MUIA_String_Integer, settings.scrnheight); /* screen height */
  set(ES_Win->IntStr[6], MUIA_String_Integer, settings.overscan); /* vert oscan */
  set(ES_Win->IntStr[7], MUIA_String_Integer, settings.fractal); /* fractal depth */
  set(ES_Win->IntStr[8], MUIA_String_Integer, settings.gridsize); /* grid spacing */
  set(ES_Win->IntStr[9], MUIA_String_Integer, settings.surfel[0]); /* sfc el 1 */
  set(ES_Win->IntStr[10], MUIA_String_Integer, settings.surfel[1]); /* sfc el 2 */
  set(ES_Win->IntStr[11], MUIA_String_Integer, settings.surfel[2]); /* sfc el 3 */
  set(ES_Win->IntStr[12], MUIA_String_Integer, settings.surfel[3]); /* sfc el 4 */
  set(ES_Win->IntStr[13], MUIA_String_Integer, (long)settings.skyalias); /* sky dither points */
  set(ES_Win->IntStr[14], MUIA_String_Integer, settings.aliasfactor); /* antialias factor */
#ifdef ENABLE_SCALING
  set(ES_Win->IntStr[15], MUIA_String_Integer, settings.scalewidth); /* scaled image width */
  set(ES_Win->IntStr[16], MUIA_String_Integer, settings.scaleheight); /* scaled image height */
#endif /* ENABLE_SCALING */
  set(ES_Win->IntStr[17], MUIA_String_Integer, settings.vecsegs); /* vector segments */
  set(ES_Win->IntStr[18], MUIA_String_Integer, settings.lookaheadframes); /* look ahead frames */
  set(ES_Win->IntStr[19], MUIA_String_Integer, settings.easein); /* vel distr ease in */
  set(ES_Win->IntStr[20], MUIA_String_Integer, settings.easeout); /* vel distr ease out */
  set(ES_Win->IntStr[21], MUIA_String_Integer, settings.startseg); /* start segment to render */
  set(ES_Win->IntStr[22], MUIA_String_Integer, 
	settings.startframe + (settings.maxframes - 1) * settings.stepframes);

  setfloat(ES_Win->FloatStr[0], settings.picaspect);
  setfloat(ES_Win->FloatStr[1], settings.bankfactor);
  setfloat(ES_Win->FloatStr[2], settings.lineoffset);
  setfloat(ES_Win->FloatStr[3], settings.zenith);
  setfloat(ES_Win->FloatStr[4], settings.treefactor);
  setfloat(ES_Win->FloatStr[5], settings.altqlat);
  setfloat(ES_Win->FloatStr[6], settings.altqlon);
  setfloat(ES_Win->FloatStr[7], settings.zalias);
  setfloat(ES_Win->FloatStr[8], settings.globecograd);
  setfloat(ES_Win->FloatStr[9], settings.globsnowgrad);
  setfloat(ES_Win->FloatStr[10], settings.globreflat);
  setfloat(ES_Win->FloatStr[11], settings.displacement);
  setfloat(ES_Win->FloatStr[12], settings.dispslopefact);
  setfloat(ES_Win->FloatStr[13], settings.stratadip);
  setfloat(ES_Win->FloatStr[14], settings.stratastrike);
  setfloat(ES_Win->FloatStr[15], settings.deformscale);

/* set active cycle items */
  set(ES_Win->Cycle[0], MUIA_Cycle_Active, (settings.renderopts & 0x01)); /* Render RGB */
  //set(ES_Win->Cycle[1], MUIA_Cycle_Active, 1); // (settings.renderopts & 0x10));    /* Render Screen */
  if(settings.renderopts & 0x10) {set(ES_Win->Cycle[1], MUIA_Cycle_Active, 1);}       /* Render Screen Gray Scale */
  else if(settings.renderopts & 0x20) {set(ES_Win->Cycle[1], MUIA_Cycle_Active, 2);}  /* Render Screen Colored */

  set(ES_Win->Cycle[2], MUIA_Cycle_Active, (settings.renderopts & 0x100)); /* Render Data */
  set(ES_Win->Cycle[3], MUIA_Cycle_Active, settings.worldmap); /* Global Gradients */
  set(ES_Win->Cycle[4], MUIA_Cycle_Active, settings.saveIFF); /* Save Format */
  set(ES_Win->Cycle[5], MUIA_Cycle_Active, settings.composite); /* Concatenate segments */
  set(ES_Win->Cycle[6], MUIA_Cycle_Active,
	 (settings.displace ? 2: settings.smoothfaces)); /* Render Style */
  set(ES_Win->Cycle[8], MUIA_Cycle_Active, settings.bankturn); /* Bank turns */
  set(ES_Win->Cycle[9], MUIA_Cycle_Active,
	 (settings.linetoscreen == -1 ? 2: settings.linetoscreen)); /* Render vectors */
  set(ES_Win->Cycle[10], MUIA_Cycle_Active, settings.linefade); /* Vector Haze */
  set(ES_Win->Cycle[11], MUIA_Cycle_Active,
	 (settings.fractalmap ? 2: settings.fixfract)); /* Fix Fractals */
  set(ES_Win->Cycle[12], MUIA_Cycle_Active, settings.horizonmax); /* No render beyond horizon */
  set(ES_Win->Cycle[13], MUIA_Cycle_Active, settings.colrmap); /* Use Color Maps */
  set(ES_Win->Cycle[14], MUIA_Cycle_Active, settings.borderandom); /* Randomize borders */
  set(ES_Win->Cycle[15], MUIA_Cycle_Active, settings.cmaptrees); /* CMap Trees */
  set(ES_Win->Cycle[16], MUIA_Cycle_Active, settings.ecomatch); /* Color Match */
  set(ES_Win->Cycle[17], MUIA_Cycle_Active, settings.cmapluminous); /* Luminous Colors */
  set(ES_Win->Cycle[18], MUIA_Cycle_Active, settings.fielddominance); /* Field Dominance */
  set(ES_Win->Cycle[19], MUIA_Cycle_Active, settings.drawgrid); /* Grid Surface */
  set(ES_Win->Cycle[20], MUIA_Cycle_Active, settings.horfix); /* Fix Horizon */
  set(ES_Win->Cycle[21], MUIA_Cycle_Active, settings.alternateq); /* Alternate Q */
  set(ES_Win->Cycle[22], MUIA_Cycle_Active, settings.clouds); /* Cloud Shadows */
  set(ES_Win->Cycle[23], MUIA_Cycle_Active, settings.flatteneco); /* Ecosystem flattening */
  set(ES_Win->Cycle[24], MUIA_Cycle_Active, settings.lookahead); /* Look ahead */
  set(ES_Win->Cycle[25], MUIA_Cycle_Active, settings.background); /* Pre-Load Background */
  set(ES_Win->Cycle[26], MUIA_Cycle_Active, settings.zbuffer); /* Pre-Load Z buffer */
  set(ES_Win->Cycle[27], MUIA_Cycle_Active, settings.antialias); /* Antialias */
  set(ES_Win->Cycle[28], MUIA_Cycle_Active, settings.zbufalias); /* Antialias */
  set(ES_Win->Cycle[30], MUIA_Cycle_Active, settings.mapassfc); /* Topos as SFC */
  set(ES_Win->Cycle[31], MUIA_Cycle_Active, settings.rendertrees); /* Trees and Textures */
  set(ES_Win->Cycle[32], MUIA_Cycle_Active, settings.velocitydistr); /* velocity distribution */
  set(ES_Win->Cycle[34], MUIA_Cycle_Active, settings.exportzbuf); /* Export Z buffer */
  set(ES_Win->Cycle[35], MUIA_Cycle_Active, settings.zformat); /* Z buffer format*/
  set(ES_Win->Cycle[36], MUIA_Cycle_Active, settings.fieldrender); /* Field Rendering */
  set(ES_Win->Cycle[37], MUIA_Cycle_Active, settings.mastercmap); /* Master Color Map */
  set(ES_Win->Cycle[38], MUIA_Cycle_Active, settings.cmaporientation); /* Color Map Orientation */
  set(ES_Win->Cycle[39], MUIA_Cycle_Active, settings.perturb); /* Perturb fractal surface color  */
  set(ES_Win->Cycle[40], MUIA_Cycle_Active, settings.realclouds); /* 3D clouds  */
  set(ES_Win->Cycle[41], MUIA_Cycle_Active, settings.waves); /* Waves  */
  set(ES_Win->Cycle[42], MUIA_Cycle_Active, settings.reflections); /* Reflections  */
  set(ES_Win->Cycle[43], MUIA_Cycle_Active, settings.deformationmap); /* Strata Deformation map  */
  set(ES_Win->Cycle[44], MUIA_Cycle_Active, settings.colorstrata); /* Strata Colors  */
  set(ES_Win->Cycle[45], MUIA_Cycle_Active, settings.cmapsurface); /* Surface Color maps  */
  set(ES_Win->Cycle[46], MUIA_Cycle_Active, settings.sun); /* Sun */
  set(ES_Win->Cycle[47], MUIA_Cycle_Active, settings.moon); /* Moon */
  set(ES_Win->Cycle[48], MUIA_Cycle_Active, settings.tides); /* Tides */
  set(ES_Win->Cycle[49], MUIA_Cycle_Active, settings.sunhalo); /* Sun Halo */
  set(ES_Win->Cycle[50], MUIA_Cycle_Active, settings.moonhalo); /* Moon Halo */

/* set text items */
  set(ES_Win->Txt[0], MUIA_Text_Contents, (IPTR)PAR_NAME_ECO(settings.defaulteco)); /* Default Ecosystem */

/* disable gadgets */
  Disable_ES_Window();

} /* Set_ES_Window() */

/**********************************************************************/

STATIC_FCN void Disable_ES_Window(void) // used locally only -> static, AF 26.7.2021
{

 Disable_ES_Gads((settings.maxframes == 1) && (settings.rendersegs == 1),
	ES_Win->Cycle[2], NULL);

 Disable_ES_Gads(settings.rendersegs > 1, ES_Win->Cycle[5], ES_Win->IntStr[21], NULL);

 Disable_ES_Gads(settings.renderopts & 0x01, ES_Win->Cycle[4], ES_Win->Cycle[36],
	ES_Win->Str[0], ES_Win->Str[8], NULL);

 Disable_ES_Gads(settings.fieldrender && (settings.renderopts & 0x01),
	ES_Win->Cycle[18], NULL);

 Disable_ES_Gads(settings.worldmap, ES_Win->FloatStr[8], ES_Win->FloatStr[9],
	ES_Win->FloatStr[10], NULL);

 Disable_ES_Gads(settings.linetoscreen >= 0, ES_Win->IntStr[17], ES_Win->FloatStr[2], 
	ES_Win->Str[1], ES_Win->Str[10], ES_Win->Cycle[10], NULL);
 Disable_ES_Gads(settings.linetoscreen == 1, ES_Win->Cycle[10], NULL);

 Disable_ES_Gads(settings.colrmap, ES_Win->Cycle[37],
	ES_Win->Cycle[14], ES_Win->Cycle[15],
	ES_Win->Cycle[16], ES_Win->Cycle[17],
	ES_Win->Str[4], NULL);

 Disable_ES_Gads(settings.drawgrid, ES_Win->IntStr[8], NULL);

 Disable_ES_Gads(! settings.horfix, ES_Win->FloatStr[3], NULL);

 Disable_ES_Gads(settings.alternateq, ES_Win->FloatStr[5], ES_Win->FloatStr[6], NULL);

 Disable_ES_Gads(settings.lookahead, ES_Win->IntStr[18], NULL);

 Disable_ES_Gads(settings.background, ES_Win->Str[2], ES_Win->Str[5], NULL);

 Disable_ES_Gads(settings.zbuffer, ES_Win->Str[3], ES_Win->Str[6], NULL);

 Disable_ES_Gads(settings.displace < 1, ES_Win->Cycle[39], NULL);
 Disable_ES_Gads(settings.displace,
	ES_Win->FloatStr[11], ES_Win->FloatStr[12], ES_Win->Cycle[41], NULL);

 Disable_ES_Gads(settings.antialias, ES_Win->IntStr[14], ES_Win->Cycle[28], NULL);
 Disable_ES_Gads(settings.antialias && settings.zbufalias,
	ES_Win->FloatStr[7], NULL);

#ifdef ENABLE_SCALING
 Disable_ES_Gads(settings.scaleimage, ES_Win->IntStr[15], ES_Win->IntStr[16], NULL);
#endif /* ENABLE_SCALING */

 Disable_ES_Gads(settings.exportzbuf, ES_Win->Cycle[35], NULL);
 Disable_ES_Gads(settings.fieldrender && (settings.renderopts & 0x01),
	ES_Win->Str[7], NULL);
 Disable_ES_Gads(settings.velocitydistr, ES_Win->IntStr[19], ES_Win->IntStr[20],
	NULL);

 Disable_ES_Gads(settings.mastercmap && settings.colrmap,
	ES_Win->Cycle[38], ES_Win->Str[12], NULL);

 Disable_ES_Gads(settings.deformationmap,
	ES_Win->Str[13], ES_Win->Str[14], ES_Win->FloatStr[15], NULL);

 Disable_ES_Gads(settings.sun, ES_Win->Cycle[49], NULL);
 Disable_ES_Gads(settings.moon, ES_Win->Cycle[50], NULL);

} /* Disable_ES_Window() */

/**********************************************************************/

void Disable_ES_Gads(short status, ...)
{
va_list VarA;
APTR Target;

va_start(VarA, status);

while (1)
 {
 Target = va_arg(VarA, APTR);
 if (Target == NULL)
  break;
 set(Target, MUIA_Disabled, 1 - status);
 } /* while */

va_end(VarA);

} /* Disable_ES_Gads() */

/**********************************************************************/

void Set_FloatStringValue(short i, short factor, double *value)
{
double change;

 switch (i)
  {
  case 2:
   {
   change = .01 * factor;
   break;
   }
  case 0:
  case 7:
  case 12:
   {
   change = .1 * factor;
   break;
   }
  case 1:
  case 4:
  case 5:
  case 6:
  case 10:
  case 11:
  case 13:
  case 15:
   {
   change = 1.0 * factor;
   break;
   }
  case 3:
  case 8:
  case 9:
  case 14:
   {
   change = 10.0 * factor;
   break;
   }
  } /* switch */

 *value += change;

 if (i == 0 && *value < .1)
  *value = .1;
 else if ((i == 4 || i == 7 || i == 12 || i == 13) && *value < 0.0)
  *value = 0.0;
 else if ((i == 13) && *value > 89.0)
  *value = 89.0;

} /* Set_FloatStringValue() */

/*********************************************************************/

void Set_IntStringValue(short i, short factor, long *value)
{
long change = 0;

 switch (i)
  {
  case 0:
  case 6:
  case 7:
  case 13:
  case 19:
  case 20:
  case 21:
  case 22:
   {
   if (factor > 0 || *value > 0)
    change = factor;
   break;
   } /* minimum 0 */
  case 1:
  case 3:
  case 4:
  case 5:
  case 8:
  case 14:
  case 17:
  case 18:
   {
   if (factor > 0 || *value > 1)
    change = factor;
   break;
   } /* minimum 1 */
  case 2:
  case 9:
  case 10:
  case 11:
  case 12:
   {
   change = factor;
   break;
   } /* no minimum */
  } /* switch */

 *value += change;

} /* Set_IntStringValue() */

/**********************************************************************/

void SetMemoryReqTxt(void)
{
char UseStr[32];
long Size, BMapSize, ZBufSize, QCBufSize, TotalUse = 0, MaxUse = 0, TempUse,
	Gigas, Megas, Kilos;

 BMapSize = settings.scrnwidth * (settings.overscan
		+ settings.scrnheight / settings.rendersegs);
 QCBufSize = BMapSize * 4;
 ZBufSize = BMapSize * 4;

/* once per animation */

 TotalUse += 10000 + ZBufSize + 5 * BMapSize;	/* KFsize + ... */

 if (settings.reflections)
  {
  TotalUse += BMapSize * 4;
  TotalUse += BMapSize * 4;
  TotalUse += BMapSize;
  }

 if (settings.renderopts & 0x100)
  {
  TotalUse += 5 * QCBufSize;
  }

 TotalUse += 20000; /* key table */

 TotalUse += 10000; /* forest models */

 TotalUse += 100000; /* bitmap images */

 if (settings.waves)
  TotalUse += 10000; /* waves */

 if (settings.clouds)
  TotalUse += 10000; /* cloud data struct */

 if (settings.clouds)
  {
  TotalUse += 6 * 301 * 301;
  }

 if (settings.colrmap && settings.mastercmap)
  {
  TotalUse += 3 * 301 * 301;
  TotalUse += sizeof (struct Color_Map);
  }

 if (settings.deformationmap)
  {
  TotalUse += 2 * 301 * 301;
  TotalUse += 65536;
  }

/* DEMs */
 Size = 2 * 301 * 301;
 TempUse = 0;

 TempUse += Size + 4 * 2 * Size; 

 if (settings.smoothfaces)
  {
  TempUse += 6 * 301 * sizeof (struct faces);
  }
 if (settings.fractalmap)
  {
  TempUse += 2 * 301 * 300;
  }
 if (settings.colrmap && ! settings.mastercmap)
  {
  TempUse += 3 * 301 * 301;
  }

 TempUse += 2 * 301 * 301;	/* relel */

 TempUse += 10000;		/* TreePix */
 TempUse += 10000;		/* SubPix */
 TempUse += 2 * 1000; 	/* Edges */

 if (settings.displace && ! settings.smoothfaces)
  {
  if (settings.fractal > 0)
   TempUse += 714;		/* 15 Vertices */ 
  if (settings.fractal > 1) 
   TempUse += 2922;		/* 63 */
  if (settings.fractal > 2)
   TempUse += 11754;		/* 255 */
  if (settings.fractal > 3)
   TempUse += 47082;		/* 1023 */
  if (settings.fractal > 4)
   TempUse += 188394;		/* 4095 */
  if (settings.fractal > 5)
   TempUse += 753642;		/* 16383 */
  }

 MaxUse = TotalUse + TempUse;

/* Vectors */

 TempUse = 3000 + 4 * 6000;

 if (TotalUse + TempUse > MaxUse)
  MaxUse = TotalUse + TempUse;

/* background */

 TempUse = 0;

 if (settings.zbuffer)
  TempUse += ZBufSize;
 if (settings.background)
  TempUse += 3 * BMapSize;

 if (TotalUse + TempUse > MaxUse)
  MaxUse = TotalUse + TempUse;

/* add celestial objects */

 TempUse = 0;

 TempUse += 622 * 622;

 if (TotalUse + TempUse > MaxUse)
  MaxUse = TotalUse + TempUse;

/* add clouds */

 TempUse = 0;

 if (settings.reflections)
  {
  TotalUse -= BMapSize * 5;
  }

 TempUse += 2 * 1000;

 TempUse += 3 * 4 * 301 * 301;
 if (! settings.clouds)
  {
  TempUse += 4 * 301 * 301;
  TempUse += 2 * 301 * 301;
  }

 if (TotalUse + TempUse > MaxUse)
  MaxUse = TotalUse + TempUse;

 if (settings.reflections)
  {
  TotalUse += BMapSize * 5;
  }

/* apply reflections - no additional memory required */

/* set MUI text gadget */

 Gigas = MaxUse / 1000000000;
 if (Gigas)
  MaxUse %= (Gigas * 1000000000);
 Megas = MaxUse / 1000000;
 if (Megas)
  MaxUse %= (Megas * 1000000);
 Kilos = MaxUse / 1000;
 if (Kilos)
  MaxUse %= (Kilos * 1000);

 str[0] = 0;
 if (Gigas)
  {
  sprintf(UseStr, "%ld,", Gigas);
  strcat(str, UseStr);
  }
 if (Megas)
  {
  if (Gigas)
   sprintf(UseStr, "%03ld,", Megas);
  else
   sprintf(UseStr, "%ld,", Megas);
  strcat(str, UseStr);
  }
 if (Kilos)
  {
  if (Megas)
   sprintf(UseStr, "%03ld,", Kilos);
  else
   sprintf(UseStr, "%ld,", Kilos);
  strcat(str, UseStr);
  }
 if (Kilos)
  sprintf(UseStr, "%03ld", MaxUse);
 else
  sprintf(UseStr, "%ld", MaxUse);
 strcat(str, UseStr);

 set(ES_Win->UseTxt, MUIA_Text_Contents, (IPTR)str);

} /* SetMemoryReqTxt() */

/***********************************************************************/

void SetRenderSpeedGauge(void)
{
double FracDepth, RenderTime = 1.0;
long IntRenderTime;

 if (settings.displace && settings.fractal > 6)
  FracDepth = 6.0;
 else
  FracDepth = settings.fractal;

 if (settings.displace)
  RenderTime *= 1.1;
 else if (settings.perturb)
  RenderTime *= 1.05;

 if (settings.fractalmap || ! settings.fixfract)
  RenderTime *= pow(2.0, FracDepth);
 else
  RenderTime *= pow(3.0, FracDepth);

 if (RenderTime > 275.0)
  RenderTime = 275.0;

 IntRenderTime = 10000.0 * RenderTime / 275.0;

 set(ES_Win->SpeedGauge, MUIA_Gauge_Current, IntRenderTime);

} /* SetRenderSpeedGauge() */

/***********************************************************************/

void Set_PJ_Window(void)
{

 nnset(PJ_Win->Str[0], MUIA_String_Contents, (IPTR)projectpath);
 nnset(PJ_Win->Str[1], MUIA_String_Contents, (IPTR)projectname);
 nnset(PJ_Win->Str[2], MUIA_String_Contents, (IPTR)dbasepath);
 nnset(PJ_Win->Str[3], MUIA_String_Contents, (IPTR)dbasename);
 nnset(PJ_Win->Str[4], MUIA_String_Contents, (IPTR)parampath);
 nnset(PJ_Win->Str[5], MUIA_String_Contents, (IPTR)paramfile);
 nnset(PJ_Win->Str[6], MUIA_String_Contents, (IPTR)framepath);
 nnset(PJ_Win->Str[7], MUIA_String_Contents, (IPTR)framefile);
 nnset(PJ_Win->Str[8], MUIA_String_Contents, (IPTR)temppath);
 nnset(PJ_Win->Str[9], MUIA_String_Contents, (IPTR)colormapfile);
 nnset(PJ_Win->Str[10], MUIA_String_Contents, (IPTR)linepath);
 nnset(PJ_Win->Str[11], MUIA_String_Contents, (IPTR)linefile);
 nnset(PJ_Win->Str[12], MUIA_String_Contents, (IPTR)zbufferpath);
 nnset(PJ_Win->Str[13], MUIA_String_Contents, (IPTR)zbufferfile);
 nnset(PJ_Win->Str[14], MUIA_String_Contents, (IPTR)backgroundpath);
 nnset(PJ_Win->Str[15], MUIA_String_Contents, (IPTR)backgroundfile);
 nnset(PJ_Win->Str[16], MUIA_String_Contents, (IPTR)graphpath);
 nnset(PJ_Win->Str[17], MUIA_String_Contents, (IPTR)graphname);
 nnset(PJ_Win->Str[18], MUIA_String_Contents, (IPTR)colormappath);
 nnset(PJ_Win->Str[19], MUIA_String_Contents, (IPTR)modelpath);
 nnset(PJ_Win->Str[20], MUIA_String_Contents, (IPTR)dirname);
 nnset(PJ_Win->Str[21], MUIA_String_Contents, (IPTR)cloudpath);
 nnset(PJ_Win->Str[22], MUIA_String_Contents, (IPTR)cloudfile);
 nnset(PJ_Win->Str[23], MUIA_String_Contents, (IPTR)wavepath);
 nnset(PJ_Win->Str[24], MUIA_String_Contents, (IPTR)wavefile);
 nnset(PJ_Win->Str[25], MUIA_String_Contents, (IPTR)deformpath);
 nnset(PJ_Win->Str[26], MUIA_String_Contents, (IPTR)deformfile);
 nnset(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
 nnset(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
 nnset(PJ_Win->Str[28], MUIA_String_Contents, (IPTR)sunfile);
 nnset(PJ_Win->Str[27], MUIA_String_Contents, (IPTR)imagepath);
 nnset(PJ_Win->Str[29], MUIA_String_Contents, (IPTR)moonfile);
 nnset(PJ_Win->Str[30], MUIA_String_Contents, (IPTR)pcprojectpath);
 nnset(PJ_Win->Str[31], MUIA_String_Contents, (IPTR)pcframespath);

} /* Set_PJ_Window() */

/***********************************************************************/
