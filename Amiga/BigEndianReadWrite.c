/*
 * BigEndianReadWrite.c
 *
 *  Created on: Mar 22, 2023
 *      Author: Alexander Fritsch, selco, HGW
 *
 *      Read/write basic datatypes and special structures from/into big endian format
 */

#include "BigEndianReadWrite.h"

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteParHeader_BE(const struct ParHeader *ParHdr,FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct ParHeader TempParHdr;

    TempParHdr=*ParHdr;

    SimpleEndianFlip32F(TempParHdr.Version,&TempParHdr.Version);
    SimpleEndianFlip32S(TempParHdr.ByteOrder,&TempParHdr.ByteOrder);
    SimpleEndianFlip16S(TempParHdr.KeyFrames,&TempParHdr.KeyFrames);
    SimpleEndianFlip32S(TempParHdr.MotionParamsPos,&TempParHdr.MotionParamsPos);
    SimpleEndianFlip32S(TempParHdr.ColorParamsPos,&TempParHdr.ColorParamsPos);
    SimpleEndianFlip32S(TempParHdr.EcoParamsPos,&TempParHdr.EcoParamsPos);
    SimpleEndianFlip32S(TempParHdr.SettingsPos,&TempParHdr.SettingsPos);
    SimpleEndianFlip32S(TempParHdr.KeyFramesPos,&TempParHdr.KeyFramesPos);

    return (fwrite(&TempParHdr, sizeof(struct ParHeader), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(ParHdr, sizeof(struct ParHeader), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteMotion_BE(const struct Motion *Value, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Motion TempMotion;
    TempMotion=*Value;
    SimpleEndianFlip64(TempMotion.Value,&TempMotion.Value);
    return (fwrite(&TempMotion, sizeof (struct Motion), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(Value, sizeof (struct Motion), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}



// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteMoPar_BE(const struct Animation *MoPar, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Animation TempMoPar;
    TempMoPar=*MoPar;

    for(unsigned int i=0;i<MOTIONPARAMS;i++)
    {
        SimpleEndianFlip64(TempMoPar.mn[i].Value,&TempMoPar.mn[i].Value);
    }
    return (fwrite(&TempMoPar, sizeof (TempMoPar), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)MoPar, sizeof (struct Animation), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteCoPar_BE(const struct Palette *CoPar, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Palette TempCoPar;
    TempCoPar=*CoPar;

    for(unsigned int i=0;i<COLORPARAMS;i++)
    {
        SimpleEndianFlip16S(TempCoPar.cn[i].Value[0],&TempCoPar.cn[i].Value[0]);
        SimpleEndianFlip16S(TempCoPar.cn[i].Value[1],&TempCoPar.cn[i].Value[1]);
        SimpleEndianFlip16S(TempCoPar.cn[i].Value[2],&TempCoPar.cn[i].Value[2]);
    }
    return (fwrite(&TempCoPar, sizeof (TempCoPar), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)CoPar, sizeof (struct Palette), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

//    if ((fwrite((char *)&CoPar.cn[saveitem], sizeof (struct Color), 1, fparam))!=1)
// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteColorItem_BE(const struct Color *CoItem, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Color TempColor;
    TempColor=*CoItem;

    SimpleEndianFlip16S(TempColor.Value[0],&TempColor.Value[0]);
    SimpleEndianFlip16S(TempColor.Value[1],&TempColor.Value[1]);
    SimpleEndianFlip16S(TempColor.Value[2],&TempColor.Value[2]);

    return (fwrite(&TempColor, sizeof (struct Color), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)CoItem, sizeof (struct Color), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteEcoPar_BE(const union Environment *EcoPar, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static union Environment TempEcoPar;
    TempEcoPar=*EcoPar;

    for(unsigned int i=0;i<ECOPARAMS;i++)
    {
        SimpleEndianFlip32F(TempEcoPar.en[i].Line,          &TempEcoPar.en[i].Line    );
        SimpleEndianFlip32F(TempEcoPar.en[i].Skew,          &TempEcoPar.en[i].Skew    );
        SimpleEndianFlip32F(TempEcoPar.en[i].SkewAz,        &TempEcoPar.en[i].SkewAz  );
        SimpleEndianFlip32F(TempEcoPar.en[i].RelEl,         &TempEcoPar.en[i].RelEl   );
        SimpleEndianFlip32F(TempEcoPar.en[i].MaxRelEl,      &TempEcoPar.en[i].MaxRelEl);
        SimpleEndianFlip32F(TempEcoPar.en[i].MinRelEl,      &TempEcoPar.en[i].MinRelEl);
        SimpleEndianFlip32F(TempEcoPar.en[i].MaxSlope,      &TempEcoPar.en[i].MaxSlope);
        SimpleEndianFlip32F(TempEcoPar.en[i].MinSlope,      &TempEcoPar.en[i].MinSlope);
        SimpleEndianFlip32F(TempEcoPar.en[i].Density,       &TempEcoPar.en[i].Density );
        SimpleEndianFlip32F(TempEcoPar.en[i].Height,        &TempEcoPar.en[i].Height  );
        SimpleEndianFlip16S(TempEcoPar.en[i].Type,          &TempEcoPar.en[i].Type    );
        SimpleEndianFlip16S(TempEcoPar.en[i].Color,         &TempEcoPar.en[i].Color   );
        SimpleEndianFlip16S(TempEcoPar.en[i].UnderEco,      &TempEcoPar.en[i].UnderEco);
        SimpleEndianFlip16S(TempEcoPar.en[i].MatchColor[0], &TempEcoPar.en[i].MatchColor[0]);
        SimpleEndianFlip16S(TempEcoPar.en[i].MatchColor[1], &TempEcoPar.en[i].MatchColor[1]);
        SimpleEndianFlip16S(TempEcoPar.en[i].MatchColor[2], &TempEcoPar.en[i].MatchColor[2]);
    }
    return (fwrite(&TempEcoPar, sizeof (union Environment), 1, file));



#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)EcoPar, sizeof (union Environment), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteEcoParItem_BE(const struct Ecosystem *EcoParItem, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Ecosystem TempEcoParItem;
    TempEcoParItem=*EcoParItem;

    SimpleEndianFlip32F(TempEcoParItem.Line,          &TempEcoParItem.Line    );
    SimpleEndianFlip32F(TempEcoParItem.Skew,          &TempEcoParItem.Skew    );
    SimpleEndianFlip32F(TempEcoParItem.SkewAz,        &TempEcoParItem.SkewAz  );
    SimpleEndianFlip32F(TempEcoParItem.RelEl,         &TempEcoParItem.RelEl   );
    SimpleEndianFlip32F(TempEcoParItem.MaxRelEl,      &TempEcoParItem.MaxRelEl);
    SimpleEndianFlip32F(TempEcoParItem.MinRelEl,      &TempEcoParItem.MinRelEl);
    SimpleEndianFlip32F(TempEcoParItem.MaxSlope,      &TempEcoParItem.MaxSlope);
    SimpleEndianFlip32F(TempEcoParItem.MinSlope,      &TempEcoParItem.MinSlope);
    SimpleEndianFlip32F(TempEcoParItem.Density,       &TempEcoParItem.Density );
    SimpleEndianFlip32F(TempEcoParItem.Height,        &TempEcoParItem.Height  );
    SimpleEndianFlip16S(TempEcoParItem.Type,          &TempEcoParItem.Type    );
    SimpleEndianFlip16S(TempEcoParItem.Color,         &TempEcoParItem.Color   );
    SimpleEndianFlip16S(TempEcoParItem.UnderEco,      &TempEcoParItem.UnderEco);
    SimpleEndianFlip16S(TempEcoParItem.MatchColor[0], &TempEcoParItem.MatchColor[0]);
    SimpleEndianFlip16S(TempEcoParItem.MatchColor[1], &TempEcoParItem.MatchColor[1]);
    SimpleEndianFlip16S(TempEcoParItem.MatchColor[2], &TempEcoParItem.MatchColor[2]);

    return (fwrite(&TempEcoParItem, sizeof (struct Ecosystem), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)EcoParItem, sizeof (struct Ecosystem), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

//  if ((fwrite((char *)&settings, sizeof settings, 1, fparam)) != 1)
// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteSettings_BE(const struct Settings *settings, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static struct Settings TempSettings;
    TempSettings=*settings;

    SimpleEndianFlip16S(TempSettings.startframe,  &TempSettings.startframe   );
    SimpleEndianFlip16S(TempSettings.maxframes,   &TempSettings.maxframes    );
    SimpleEndianFlip16S(TempSettings.startseg,    &TempSettings.startseg     );
    SimpleEndianFlip16S(TempSettings.smoothfaces, &TempSettings.smoothfaces  );
    SimpleEndianFlip16S(TempSettings.bankturn,    &TempSettings.bankturn     );
    SimpleEndianFlip16S(TempSettings.colrmap,     &TempSettings.colrmap      );
    SimpleEndianFlip16S(TempSettings.borderandom, &TempSettings.borderandom  );
    SimpleEndianFlip16S(TempSettings.cmaptrees,   &TempSettings.cmaptrees    );
    SimpleEndianFlip16S(TempSettings.rendertrees, &TempSettings.rendertrees  );
    SimpleEndianFlip16S(TempSettings.statistics,  &TempSettings.statistics   );
    SimpleEndianFlip16S(TempSettings.stepframes,  &TempSettings.stepframes   );
    SimpleEndianFlip16S(TempSettings.zbufalias,   &TempSettings.zbufalias    );
    SimpleEndianFlip16S(TempSettings.horfix,      &TempSettings.horfix       );
    SimpleEndianFlip16S(TempSettings.horizonmax,  &TempSettings.horizonmax   );
    SimpleEndianFlip16S(TempSettings.clouds,      &TempSettings.clouds       );
    SimpleEndianFlip16S(TempSettings.linefade,    &TempSettings.linefade     );
    SimpleEndianFlip16S(TempSettings.drawgrid,    &TempSettings.drawgrid     );
    SimpleEndianFlip16S(TempSettings.gridsize,    &TempSettings.gridsize     );
    SimpleEndianFlip16S(TempSettings.alternateq,  &TempSettings.alternateq   );
    SimpleEndianFlip16S(TempSettings.linetoscreen,&TempSettings.linetoscreen );
    SimpleEndianFlip16S(TempSettings.mapassfc,    &TempSettings.mapassfc     );
    SimpleEndianFlip16S(TempSettings.cmapluminous,&TempSettings.cmapluminous );
    SimpleEndianFlip16S(TempSettings.surfel[0],   &TempSettings.surfel[0]    );
    SimpleEndianFlip16S(TempSettings.surfel[1],   &TempSettings.surfel[1]    );
    SimpleEndianFlip16S(TempSettings.surfel[2],   &TempSettings.surfel[2]    );
    SimpleEndianFlip16S(TempSettings.surfel[3],   &TempSettings.surfel[3]    );

    SimpleEndianFlip16S(TempSettings.worldmap,       &TempSettings.worldmap        );
    SimpleEndianFlip16S(TempSettings.flatteneco,     &TempSettings.flatteneco      );
    SimpleEndianFlip16S(TempSettings.fixfract,       &TempSettings.fixfract        );
    SimpleEndianFlip16S(TempSettings.vecsegs,        &TempSettings.vecsegs         );
    SimpleEndianFlip16S(TempSettings.reliefshade,    &TempSettings.reliefshade     );
    SimpleEndianFlip16S(TempSettings.renderopts,     &TempSettings.renderopts      );
    SimpleEndianFlip16S(TempSettings.scrnwidth,      &TempSettings.scrnwidth       );
    SimpleEndianFlip16S(TempSettings.scrnheight,     &TempSettings.scrnheight      );
    SimpleEndianFlip16S(TempSettings.rendersegs,     &TempSettings.rendersegs      );
    SimpleEndianFlip16S(TempSettings.overscan,       &TempSettings.overscan        );
    SimpleEndianFlip16S(TempSettings.lookahead,      &TempSettings.lookahead       );
    SimpleEndianFlip16S(TempSettings.composite,      &TempSettings.composite       );
    SimpleEndianFlip16S(TempSettings.defaulteco,     &TempSettings.defaulteco      );
    SimpleEndianFlip16S(TempSettings.ecomatch,       &TempSettings.ecomatch        );
    SimpleEndianFlip16S(TempSettings.Yoffset,        &TempSettings.Yoffset         );
    SimpleEndianFlip16S(TempSettings.saveIFF,        &TempSettings.saveIFF         );
    SimpleEndianFlip16S(TempSettings.background,     &TempSettings.background      );
    SimpleEndianFlip16S(TempSettings.zbuffer,        &TempSettings.zbuffer         );
    SimpleEndianFlip16S(TempSettings.antialias,      &TempSettings.antialias       );
    SimpleEndianFlip16S(TempSettings.scaleimage,     &TempSettings.scaleimage      );
    SimpleEndianFlip16S(TempSettings.fractal,        &TempSettings.fractal         );
    SimpleEndianFlip16S(TempSettings.aliasfactor,    &TempSettings.aliasfactor     );
    SimpleEndianFlip16S(TempSettings.scalewidth,     &TempSettings.scalewidth      );
    SimpleEndianFlip16S(TempSettings.scaleheight,    &TempSettings.scaleheight     );

    SimpleEndianFlip16S(TempSettings.exportzbuf,        &TempSettings.exportzbuf          );
    SimpleEndianFlip16S(TempSettings.zformat,           &TempSettings.zformat             );
    SimpleEndianFlip16S(TempSettings.fieldrender,       &TempSettings.fieldrender         );
    SimpleEndianFlip16S(TempSettings.lookaheadframes,   &TempSettings.lookaheadframes     );
    SimpleEndianFlip16S(TempSettings.velocitydistr,     &TempSettings.velocitydistr       );
    SimpleEndianFlip16S(TempSettings.easein,            &TempSettings.easein              );
    SimpleEndianFlip16S(TempSettings.easeout,           &TempSettings.easeout             );
    SimpleEndianFlip16S(TempSettings.displace,          &TempSettings.displace            );
    SimpleEndianFlip16S(TempSettings.mastercmap,        &TempSettings.mastercmap          );
    SimpleEndianFlip16S(TempSettings.cmaporientation,   &TempSettings.cmaporientation     );
    SimpleEndianFlip16S(TempSettings.fielddominance,    &TempSettings.fielddominance      );
    SimpleEndianFlip16S(TempSettings.fractalmap,        &TempSettings.fractalmap          );
    SimpleEndianFlip16S(TempSettings.perturb,           &TempSettings.perturb             );
    SimpleEndianFlip16S(TempSettings.realclouds,        &TempSettings.realclouds          );
    SimpleEndianFlip16S(TempSettings.reflections,       &TempSettings.reflections         );
    SimpleEndianFlip16S(TempSettings.waves,             &TempSettings.waves               );
    SimpleEndianFlip16S(TempSettings.colorstrata,       &TempSettings.colorstrata         );
    SimpleEndianFlip16S(TempSettings.cmapsurface,       &TempSettings.cmapsurface         );
    SimpleEndianFlip16S(TempSettings.deformationmap,    &TempSettings.deformationmap      );
    SimpleEndianFlip16S(TempSettings.moon,              &TempSettings.moon                );
    SimpleEndianFlip16S(TempSettings.sun,               &TempSettings.sun                 );
    SimpleEndianFlip16S(TempSettings.tides,             &TempSettings.tides               );
    SimpleEndianFlip16S(TempSettings.sunhalo,           &TempSettings.sunhalo             );
    SimpleEndianFlip16S(TempSettings.moonhalo,          &TempSettings.moonhalo            );

   for(unsigned int i=0;i<EXTRASHORTSETTINGS;i++)
   {
       SimpleEndianFlip16S(TempSettings.extrashorts[i], &TempSettings.extrashorts[i] );
   }

   for(unsigned int i=0;i<EXTRADOUBLESETTINGS;i++)
   {
       SimpleEndianFlip64(TempSettings.extradoubles[i], &TempSettings.extradoubles[i] );
   }

   SimpleEndianFlip64(TempSettings.deformscale,   &TempSettings.deformscale   );
   SimpleEndianFlip64(TempSettings.stratadip,     &TempSettings.stratadip     );
   SimpleEndianFlip64(TempSettings.stratastrike,  &TempSettings.stratastrike  );
   SimpleEndianFlip64(TempSettings.dispslopefact, &TempSettings.dispslopefact );
   SimpleEndianFlip64(TempSettings.globecograd,   &TempSettings.globecograd   );
   SimpleEndianFlip64(TempSettings.globsnowgrad,  &TempSettings.globsnowgrad  );
   SimpleEndianFlip64(TempSettings.globreflat,    &TempSettings.globreflat    );
   SimpleEndianFlip64(TempSettings.zalias,        &TempSettings.zalias        );
   SimpleEndianFlip64(TempSettings.bankfactor,    &TempSettings.bankfactor    );
   SimpleEndianFlip64(TempSettings.skyalias,      &TempSettings.skyalias      );
   SimpleEndianFlip64(TempSettings.lineoffset,    &TempSettings.lineoffset    );
   SimpleEndianFlip64(TempSettings.altqlat,       &TempSettings.altqlat       );
   SimpleEndianFlip64(TempSettings.altqlon,       &TempSettings.altqlon       );
   SimpleEndianFlip64(TempSettings.treefactor,    &TempSettings.treefactor    );
   SimpleEndianFlip64(TempSettings.displacement,  &TempSettings.displacement  );
   SimpleEndianFlip64(TempSettings.unused3,       &TempSettings.unused3       );
   SimpleEndianFlip64(TempSettings.picaspect,     &TempSettings.picaspect     );
   SimpleEndianFlip64(TempSettings.zenith,        &TempSettings.zenith        );

   return (fwrite(&TempSettings, sizeof (struct Settings), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)settings, sizeof (struct Settings), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 9.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteKeyFrames_BE(const union KeyFrame *KeyFrames, short NumKeyframes, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    union KeyFrame *TempKeyFrames;

    TempKeyFrames=malloc(sizeof(union KeyFrame)*NumKeyframes);
    if(!TempKeyFrames)
    {
        return 0;
    }

    memcpy(TempKeyFrames,KeyFrames,NumKeyframes*sizeof(union KeyFrame));

/*
    struct MotionKey MoKey;      // 3x short, 3x float, 1x short,  1x double
    struct MotionKey2 MoKey2;    // 3x short, 3x float, 1x short,  1x double[1]
    struct ColorKey CoKey;       // 3x short, 3x float, 1x short,  3x short

    struct EcosystemKey EcoKey;  // 3x short, 3x float, 1x short, 10x float
    struct EcosystemKey2 EcoKey2;// 3x short, 3x float, 1x short,  1x float[10]
    struct CloudKey CldKey;      // 3x short, 3x float, 1x short,  1x float[7]
    struct WaveKey WvKey;        // 3x short, 3x float, 1x short,  1x float[4]
*/


    // The first 7 Values are identical for all structs in the union
    for(unsigned int i=0;i<NumKeyframes;i++)
    {
       SimpleEndianFlip16S(TempKeyFrames[i].MoKey.KeyFrame,&TempKeyFrames[i].MoKey.KeyFrame);
       SimpleEndianFlip16S(TempKeyFrames[i].MoKey.Group,   &TempKeyFrames[i].MoKey.Group);
       SimpleEndianFlip16S(TempKeyFrames[i].MoKey.Item,    &TempKeyFrames[i].MoKey.Item);
       SimpleEndianFlip32F(TempKeyFrames[i].MoKey.TCB[0],  &TempKeyFrames[i].MoKey.TCB[0]);
       SimpleEndianFlip32F(TempKeyFrames[i].MoKey.TCB[1],  &TempKeyFrames[i].MoKey.TCB[1]);
       SimpleEndianFlip32F(TempKeyFrames[i].MoKey.TCB[2],  &TempKeyFrames[i].MoKey.TCB[2]);
       SimpleEndianFlip16S(TempKeyFrames[i].MoKey.Linear,  &TempKeyFrames[i].MoKey.Linear);

       // now some special handling...
       switch (KeyFrames[i].MoKey.Group)  // AF: 13.Jan.23 Das Switch/Case geht ueber die native Group, also ungeswappt!
       {
           case 0:
           {
               SimpleEndianFlip64(TempKeyFrames[i].MoKey.Value,&TempKeyFrames[i].MoKey.Value);
               break;
           } /* Motion */
           case 1:
           {
               SimpleEndianFlip16S(TempKeyFrames[i].CoKey.Value[0], &TempKeyFrames[i].CoKey.Value[0]);
               SimpleEndianFlip16S(TempKeyFrames[i].CoKey.Value[1], &TempKeyFrames[i].CoKey.Value[1]);
               SimpleEndianFlip16S(TempKeyFrames[i].CoKey.Value[2], &TempKeyFrames[i].CoKey.Value[2]);
               break;
           } /* Color */
           case 2:
           {
               for (unsigned int j=0; j<10; j++)  // we have 4 to 10 float values, flip them all
               {
                   SimpleEndianFlip32F(TempKeyFrames[i].EcoKey2.Value[j],&TempKeyFrames[i].EcoKey2.Value[j]);
               }
               break;
           }/* Ecosystem */
       }
    }
    // -------------------

   int result=fwrite(TempKeyFrames, NumKeyframes*sizeof (union KeyFrame), 1, file);
   free(TempKeyFrames);
   return (result);

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)KeyFrames, NumKeyframes*sizeof (union KeyFrame), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 9.Jan23, Write short in Big-Endian (i.e. native Amiga-) format
int fwrite_short_BE(const short *Value, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    short TempValue = *Value;

    SimpleEndianFlip16S(TempValue,  &TempValue);

    return (fwrite(&TempValue, sizeof (short), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)Value, sizeof (short), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 16.Feb23, Write LONG in Big-Endian (i.e. native Amiga-) format
int fwrite_LONG_BE(const LONG *Value, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    LONG TempValue = *Value;

    SimpleEndianFlip32S(TempValue,  &TempValue);

    return (fwrite(&TempValue, sizeof (LONG), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite((char *)Value, sizeof (LONG), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}


// AF: 21.Mar23, Write float in Big-Endian (i.e. native Amiga-) format
int write_float_BE(int fh, const float *Value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    float TempValue = *Value;

    SimpleEndianFlip32F(TempValue,  &TempValue);

    return write(fh,&TempValue, sizeof (float));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return write(fh, Value, sizeof (float));
#else
    #error "Unsupported Byte-Order"
#endif
}

// AF: 22.Mar23, Write float in Big-Endian (i.e. native Amiga-) format
int fwrite_float_BE(const float *Value, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    float TempValue = *Value;

    SimpleEndianFlip32F(TempValue,  &TempValue);

    return (fwrite(&TempValue, sizeof (float), 1, file));

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(Value, sizeof (float), 1, file));
#else
    #error "Unsupported Byte-Order"
#endif
}


// size in Bytes, not floats!
// returns number of Bytes written
ssize_t write_float_Array_BE(int filehandle, float *FloatArray, size_t size) // AF, HGW, 19.Jan23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_FLOAT_ARR_ENTRIES 128
    static float TempArray[TEMP_FLOAT_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t FloatsToDo=size/(sizeof(float));

    unsigned int i=0;
    for(i=0; i<FloatsToDo/TEMP_FLOAT_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_FLOAT_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip32F(FloatArray[i*TEMP_FLOAT_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_FLOAT_ARR_ENTRIES*sizeof(float));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_FLOAT_ARR_ENTRIES*sizeof(float))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<FloatsToDo%TEMP_FLOAT_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip32F(FloatArray[i*TEMP_FLOAT_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (FloatsToDo%TEMP_FLOAT_ARR_ENTRIES)*sizeof(float));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_FLOAT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, FloatArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}

// returns number of Blocks written
// the fwrite()-version
ssize_t fwrite_float_Array_BE(float *FloatArray, size_t size, FILE *file) // AF, HGW, 16.Feb23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_FLOAT_ARR_ENTRIES 128
    static float TempArray[TEMP_FLOAT_ARR_ENTRIES];

    size_t FloatsToDo=size/(sizeof(float));

    unsigned int i=0;
    for(i=0; i<FloatsToDo/TEMP_FLOAT_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_FLOAT_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip32F(FloatArray[i*TEMP_FLOAT_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=fwrite(TempArray, TEMP_FLOAT_ARR_ENTRIES*sizeof(float),1,file);
        if(Result!=1)
        {
            return 0;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<FloatsToDo%TEMP_FLOAT_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip32F(FloatArray[i*TEMP_FLOAT_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=fwrite(TempArray, (FloatsToDo%TEMP_FLOAT_ARR_ENTRIES)*sizeof(float),1,file);  // Result is 0 (error) or 1 (all bytes written)
    return Result;
#undef TEMP_FLOAT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(FloatArray, size,1,file));
#else
#error "Unsupported Byte-Order"
#endif
}


// size in Bytes, not SHORTs!
// returns 1 if all bytes written, otherwise 0
ssize_t fwrite_SHORT_Array_BE(SHORT *SHORTArray, size_t size, FILE *file) // AF, HGW, 16.Feb23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_SHORT_ARR_ENTRIES 128
    static SHORT TempArray[TEMP_SHORT_ARR_ENTRIES];

    size_t SHORTsToDo=size/(sizeof(SHORT));

    unsigned int i=0;
    for(i=0; i<SHORTsToDo/TEMP_SHORT_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_SHORT_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip16S(SHORTArray[i*TEMP_SHORT_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=fwrite(TempArray, TEMP_SHORT_ARR_ENTRIES*sizeof(SHORT),1,file);  // Result is 0 (error) or 1 (all bytes written)

        if(Result!=1)
        {
            return 0;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<SHORTsToDo%TEMP_SHORT_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip16S(SHORTArray[i*TEMP_SHORT_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=fwrite(TempArray, (SHORTsToDo%TEMP_SHORT_ARR_ENTRIES)*sizeof(SHORT),1,file);

    return Result;
#undef TEMP_SHORT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(SHORTArray, size,1,file));
#else
#error "Unsupported Byte-Order"
#endif
}

// size in Bytes, not doubles!
// returns number of Bytes written
ssize_t write_double_Array_BE(int filehandle, double *DoubleArray, size_t size) // AF, HGW, 20.Mar23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_DOUBLE_ARR_ENTRIES 128
    static double TempArray[TEMP_DOUBLE_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t DoublesToDo=size/(sizeof(double));

    unsigned int i=0;
    for(i=0; i<DoublesToDo/TEMP_DOUBLE_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_DOUBLE_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip64(DoubleArray[i*TEMP_DOUBLE_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_DOUBLE_ARR_ENTRIES*sizeof(double));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_DOUBLE_ARR_ENTRIES*sizeof(double))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<DoublesToDo%TEMP_DOUBLE_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip64(DoubleArray[i*TEMP_DOUBLE_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (DoublesToDo%TEMP_DOUBLE_ARR_ENTRIES)*sizeof(double));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_DOUBLE_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, DoubleArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}

// the fwrite version
ssize_t fwrite_double_Array_BE(double *DoubleArray, size_t size, FILE *file) // AF, HGW, 22.Feb23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_DOUBLE_ARR_ENTRIES 128
    static double TempArray[TEMP_DOUBLE_ARR_ENTRIES];

    size_t DoublesToDo=size/(sizeof(double));

    unsigned int i=0;
    for(i=0; i<DoublesToDo/TEMP_DOUBLE_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_DOUBLE_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip64(DoubleArray[i*TEMP_DOUBLE_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=fwrite(TempArray, TEMP_DOUBLE_ARR_ENTRIES*sizeof(DOUBLE),1,file);
        if(Result!=1)
        {
            return 0;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<DoublesToDo%TEMP_DOUBLE_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip64(DoubleArray[i*TEMP_DOUBLE_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=fwrite(TempArray, (DoublesToDo%TEMP_DOUBLE_ARR_ENTRIES)*sizeof(double),1,file);  // Result is 0 (error) or 1 (all bytes written)
    return Result;
#undef TEMP_FLOAT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (fwrite(DoubleArray, size,1,file));
#else
#error "Unsupported Byte-Order"
#endif
}


// size in Bytes, not short!
// returns number of Bytes written
long write_short_Array_BE(int filehandle, short *ShortArray, size_t size) // AF, HGW, 20.Mar23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_SHORT_ARR_ENTRIES 128
    static short TempArray[TEMP_SHORT_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t ShortsToDo=size/(sizeof(short));

    unsigned int i=0;
    for(i=0; i<ShortsToDo/TEMP_SHORT_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_SHORT_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip16S(ShortArray[i*TEMP_SHORT_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_SHORT_ARR_ENTRIES*sizeof(short));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_SHORT_ARR_ENTRIES*sizeof(short))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<ShortsToDo%TEMP_SHORT_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip16S(ShortArray[i*TEMP_SHORT_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (ShortsToDo%TEMP_SHORT_ARR_ENTRIES)*sizeof(short));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_SHORT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, ShortArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}

long write_ushort_Array_BE(int filehandle, short *UShortArray, size_t size) // AF, HGW, 20.Mar23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_USHORT_ARR_ENTRIES 128
    static unsigned short TempArray[TEMP_USHORT_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t UShortsToDo=size/(sizeof(unsigned short));

    unsigned int i=0;
    for(i=0; i<UShortsToDo/TEMP_USHORT_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_USHORT_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip16U(UShortArray[i*TEMP_USHORT_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_USHORT_ARR_ENTRIES*sizeof(unsigned short));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_USHORT_ARR_ENTRIES*sizeof(unsigned short))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<UShortsToDo%TEMP_USHORT_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip16U(UShortArray[i*TEMP_USHORT_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (UShortsToDo%TEMP_USHORT_ARR_ENTRIES)*sizeof(unsigned short));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_USHORT_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, UShortArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}

// size in Bytes, not LONG!
// returns number of Bytes written
long write_LONG_Array_BE(int filehandle, LONG *LongArray, size_t size) // AF, HGW, 20.Mar23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_LONG_ARR_ENTRIES 128
    static LONG TempArray[TEMP_LONG_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t LongsToDo=size/(sizeof(LONG));

    unsigned int i=0;
    for(i=0; i<LongsToDo/TEMP_LONG_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_LONG_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip32S(LongArray[i*TEMP_LONG_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_LONG_ARR_ENTRIES*sizeof(LONG));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_LONG_ARR_ENTRIES*sizeof(LONG))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<LongsToDo%TEMP_LONG_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip32S(LongArray[i*TEMP_LONG_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (LongsToDo%TEMP_LONG_ARR_ENTRIES)*sizeof(LONG));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_LONG_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, LongArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}

// size in Bytes, not ULONG!
// returns number of Bytes written
long write_ULONG_Array_BE(int filehandle, ULONG *ULongArray, size_t size) // AF, HGW, 20.Mar23
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEMP_ULONG_ARR_ENTRIES 128
    static ULONG TempArray[TEMP_ULONG_ARR_ENTRIES];
    ssize_t TotalBytesWritten=0;

    size_t ULongsToDo=size/(sizeof(ULONG));

    unsigned int i=0;
    for(i=0; i<ULongsToDo/TEMP_ULONG_ARR_ENTRIES;i++)
    {
        for(unsigned int k=0;k<TEMP_ULONG_ARR_ENTRIES;k++)
        {
            SimpleEndianFlip32U(ULongArray[i*TEMP_ULONG_ARR_ENTRIES+k],&TempArray[k]);
        }
        ssize_t Result=write(filehandle, TempArray, TEMP_ULONG_ARR_ENTRIES*sizeof(ULONG));
        TotalBytesWritten+=Result;
        if(Result!=TEMP_ULONG_ARR_ENTRIES*sizeof(ULONG))
        {
            return TotalBytesWritten;
        }
    }

    // now the rest that did not fill a complete TempArray
    for(unsigned int k=0;k<ULongsToDo%TEMP_ULONG_ARR_ENTRIES;k++)
    {
        SimpleEndianFlip32U(ULongArray[i*TEMP_ULONG_ARR_ENTRIES+k],&TempArray[k]);
    }
    ssize_t Result=write(filehandle, TempArray, (ULongsToDo%TEMP_ULONG_ARR_ENTRIES)*sizeof(ULONG));
    TotalBytesWritten+=Result;
    return TotalBytesWritten;
#undef TEMP_ULONG_ARR_ENTRIES

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, ULongArray, size));
#else
#error "Unsupported Byte-Order"
#endif
}


// AF, 20.Mar23 writes the DEM-Buffer in Big Endian Format, cares for int, unsigned and float, 1,2,4,8 Bytes size
long writeDemArray_BE(long fOutput,void *OutputData,long OutputDataSize,short outvalue_format,short outvalue_size)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// defines copied from DataOps.c, should be moved to a common header
#define DEM_DATA_FORMAT_SIGNEDINT	0
#define DEM_DATA_FORMAT_UNSIGNEDINT	1
#define DEM_DATA_FORMAT_FLOAT		2
#define DEM_DATA_FORMAT_UNKNOWN		3

#define DEM_DATA_VALSIZE_BYTE		0  //1 byte
#define DEM_DATA_VALSIZE_SHORT		1  //2 bytes
#define DEM_DATA_VALSIZE_LONG		2  //4 bytes
#define DEM_DATA_VALSIZE_DOUBLE		3  //8 bytes
#define DEM_DATA_VALSIZE_UNKNOWN	4

	// AF: we are always called with valid Format-Type / Format-Length combinations

	switch(outvalue_format)
	{
	case DEM_DATA_FORMAT_SIGNEDINT:
		switch (outvalue_size)
				{
					case DEM_DATA_VALSIZE_BYTE:
						return write(fOutput, (char *)OutputData, OutputDataSize); // just plain write
						break;
					case DEM_DATA_VALSIZE_SHORT:
						return write_short_Array_BE(fOutput,OutputData,OutputDataSize);
						break;
					case DEM_DATA_VALSIZE_LONG:
						return write_LONG_Array_BE(fOutput,OutputData,OutputDataSize);
						break;
					default:
						return 0;
				}
		break;
	case DEM_DATA_FORMAT_UNSIGNEDINT:
		switch (outvalue_size)
				{
					case DEM_DATA_VALSIZE_BYTE:
						return write(fOutput, (char *)OutputData, OutputDataSize); // just plain write
						break;
					case DEM_DATA_VALSIZE_SHORT:
						return write_ushort_Array_BE(fOutput,OutputData,OutputDataSize);
						break;
					case DEM_DATA_VALSIZE_LONG:
						return write_ULONG_Array_BE(fOutput,OutputData,OutputDataSize);
						break;
					default:
						return 0;
				}
		break;
	case DEM_DATA_FORMAT_FLOAT:
		switch (outvalue_size)
		{
			case DEM_DATA_VALSIZE_LONG:
				return (write_float_Array_BE(fOutput,OutputData,OutputDataSize));
				break;
			case DEM_DATA_VALSIZE_DOUBLE:
				return (write_double_Array_BE(fOutput,OutputData,OutputDataSize));
				break;
			default:
			return 0;
		}
		break;
	default:
		return 0;
	}

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(fOutput, (char *)OutputData, OutputDataSize));
#else
#error "Unsupported Byte-Order"
#endif
}



ssize_t writeILBMHeader_BE(int filehandle, struct ILBMHeader *Hdr)  // AF, 19.Jan23, always write BigEndian
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    struct ILBMHeader TempHdr=*Hdr;
    SimpleEndianFlip32S(TempHdr.ChunkSize, &TempHdr.ChunkSize);
    return(write(filehandle,&TempHdr,8));
#else
    // just write as it is
    return (write(filehandle, Hdr, 8));
#endif
}

ssize_t writeZBufferHeader_BE(int filehandle, struct ZBufferHeader *ZBufHdr)  // AF, 19.Jan23, always write BigEndian
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    struct ZBufferHeader TempHdr=*ZBufHdr;

    SimpleEndianFlip32U(TempHdr.Width, &TempHdr.Width);
    SimpleEndianFlip32U(TempHdr.Height, &TempHdr.Height);
    SimpleEndianFlip16U(TempHdr.VarType, &TempHdr.VarType);
    SimpleEndianFlip16U(TempHdr.Compression, &TempHdr.Compression);
    SimpleEndianFlip16U(TempHdr.Sorting, &TempHdr.Sorting);
    SimpleEndianFlip16U(TempHdr.Units, &TempHdr.Units);
    SimpleEndianFlip32F(TempHdr.Min, &TempHdr.Min);
    SimpleEndianFlip32F(TempHdr.Max, &TempHdr.Max);
    SimpleEndianFlip32F(TempHdr.Bkgrnd, &TempHdr.Bkgrnd);
    SimpleEndianFlip32F(TempHdr.ScaleFactor, &TempHdr.ScaleFactor);
    SimpleEndianFlip32F(TempHdr.ScaleBase, &TempHdr.ScaleBase);

    return(write(filehandle,&TempHdr,sizeof(struct ZBufferHeader)));
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return (write(filehandle, ZBufHdr, sizeof (struct ZBufferHeader)));
#else
#error "Unsupported Byte-Order"
#endif
}

// AF, HGW, 20.Jan23
ssize_t readZBufHdr_BE(int filehandle, struct ZBufferHeader *ZBufHdr)
{
    int Result=read(filehandle, ZBufHdr, sizeof (struct ZBufferHeader));

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Flip Endian if host is not Big Endian
    SimpleEndianFlip32U(ZBufHdr->Width      , &ZBufHdr->Width);
    SimpleEndianFlip32U(ZBufHdr->Height     , &ZBufHdr->Height);
    SimpleEndianFlip16U(ZBufHdr->VarType    , &ZBufHdr->VarType);
    SimpleEndianFlip16U(ZBufHdr->Compression, &ZBufHdr->Compression);
    SimpleEndianFlip16U(ZBufHdr->Sorting    , &ZBufHdr->Sorting);
    SimpleEndianFlip16U(ZBufHdr->Units      , &ZBufHdr->Units);
    SimpleEndianFlip32F(ZBufHdr->Min        , &ZBufHdr->Min);
    SimpleEndianFlip32F(ZBufHdr->Max        , &ZBufHdr->Max);
    SimpleEndianFlip32F(ZBufHdr->Bkgrnd     , &ZBufHdr->Bkgrnd);
    SimpleEndianFlip32F(ZBufHdr->ScaleFactor, &ZBufHdr->ScaleFactor);
    SimpleEndianFlip32F(ZBufHdr->ScaleBase  , &ZBufHdr->ScaleBase);
    return Result;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // nothing to change
#else
#error "Unsupported Byte-Order"
#endif
    return Result;
}

// AF: 20-Mar.23 read and correct endian if necessary
long readElMapHeaderV101_BE(int fh, struct elmapheaderV101 *Hdr)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	long Result=read(fh, Hdr, ELEVHDRLENV101);
	SimpleEndianFlip32S(Hdr->rows, &Hdr->rows);
	SimpleEndianFlip32S(Hdr->columns, &Hdr->columns);
	SimpleEndianFlip64(Hdr->lolat,&Hdr->lolat);
	SimpleEndianFlip64(Hdr->lolong,&Hdr->lolong);
	SimpleEndianFlip64(Hdr->steplat,&Hdr->steplat);
	SimpleEndianFlip64(Hdr->steplong,&Hdr->steplong);
	SimpleEndianFlip64(Hdr->elscale,&Hdr->elscale);
	SimpleEndianFlip16S(Hdr->MaxEl,&Hdr->MaxEl);
	SimpleEndianFlip16S(Hdr->MinEl,&Hdr->MinEl);
	SimpleEndianFlip32S(Hdr->Samples,&Hdr->Samples);
	SimpleEndianFlip32F(Hdr->SumElDif,&Hdr->SumElDif);
	SimpleEndianFlip32F(Hdr->SumElDifSq,&Hdr->SumElDifSq);
	SimpleEndianFlip32S(Hdr->size,&Hdr->size);
	SimpleEndianFlip32S(Hdr->scrnptrsize,&Hdr->scrnptrsize);
	SimpleEndianFlip32S(Hdr->fractalsize,&Hdr->fractalsize);
	SimpleEndianFlip32S(Hdr->facept[0],&Hdr->facept[0]);
	SimpleEndianFlip32S(Hdr->facept[1],&Hdr->facept[1]);
	SimpleEndianFlip32S(Hdr->facept[2],&Hdr->facept[2]);
	SimpleEndianFlip32S(Hdr->facect,&Hdr->facect);
	SimpleEndianFlip32S(Hdr->fracct,&Hdr->fracct);
	SimpleEndianFlip32S(Hdr->Lr,&Hdr->Lr);
	SimpleEndianFlip32S(Hdr->Lc,&Hdr->Lc);
	SimpleEndianFlip16S(Hdr->MapAsSFC,&Hdr->MapAsSFC);
	SimpleEndianFlip16S(Hdr->ForceBath,&Hdr->ForceBath);
	SimpleEndianFlip32F(Hdr->LonRange,&Hdr->LonRange);
    SimpleEndianFlip32F(Hdr->LatRange,&Hdr->LatRange);
    return Result;

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just read as it is
    return read(fh, Hdr, ELEVHDRLENV101);
#else
#error "Unsupported Byte-Order"
#endif

}

// AF: 21.Mar.23 read and correct endian if necessary
long writeElMapHeaderV101_BE(int fh, struct elmapheaderV101 *Hdr)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	struct elmapheaderV101 TempHdr=*Hdr;

	SimpleEndianFlip32S(TempHdr.rows, &TempHdr.rows);
	SimpleEndianFlip32S(TempHdr.columns, &TempHdr.columns);
	SimpleEndianFlip64(TempHdr.lolat,&TempHdr.lolat);
	SimpleEndianFlip64(TempHdr.lolong,&TempHdr.lolong);
	SimpleEndianFlip64(TempHdr.steplat,&TempHdr.steplat);
	SimpleEndianFlip64(TempHdr.steplong,&TempHdr.steplong);
	SimpleEndianFlip64(TempHdr.elscale,&TempHdr.elscale);
	SimpleEndianFlip16S(TempHdr.MaxEl,&TempHdr.MaxEl);
	SimpleEndianFlip16S(TempHdr.MinEl,&TempHdr.MinEl);
	SimpleEndianFlip32S(TempHdr.Samples,&TempHdr.Samples);
	SimpleEndianFlip32F(TempHdr.SumElDif,&TempHdr.SumElDif);
	SimpleEndianFlip32F(TempHdr.SumElDifSq,&TempHdr.SumElDifSq);
	SimpleEndianFlip32S(TempHdr.size,&TempHdr.size);
	SimpleEndianFlip32S(TempHdr.scrnptrsize,&TempHdr.scrnptrsize);
	SimpleEndianFlip32S(TempHdr.fractalsize,&TempHdr.fractalsize);
	SimpleEndianFlip32S(TempHdr.facept[0],&TempHdr.facept[0]);
	SimpleEndianFlip32S(TempHdr.facept[1],&TempHdr.facept[1]);
	SimpleEndianFlip32S(TempHdr.facept[2],&TempHdr.facept[2]);
	SimpleEndianFlip32S(TempHdr.facect,&TempHdr.facect);
	SimpleEndianFlip32S(TempHdr.fracct,&TempHdr.fracct);
	SimpleEndianFlip32S(TempHdr.Lr,&TempHdr.Lr);
	SimpleEndianFlip32S(TempHdr.Lc,&TempHdr.Lc);
	SimpleEndianFlip16S(TempHdr.MapAsSFC,&TempHdr.MapAsSFC);
	SimpleEndianFlip16S(TempHdr.ForceBath,&TempHdr.ForceBath);
	SimpleEndianFlip32F(TempHdr.LonRange,&TempHdr.LonRange);
    SimpleEndianFlip32F(TempHdr.LatRange,&TempHdr.LatRange);
	return write(fh, &TempHdr, ELEVHDRLENV101);

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return write(fh, Hdr, ELEVHDRLENV101);
#else
#error "Unsupported Byte-Order"
#endif

}

// AF: 22.Mar.23 correct endian if necessary and write
long fwriteVectorheaderV100_BE(struct vectorheaderV100 *Hdr, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    struct vectorheaderV100 TempHdr=*Hdr;

    SimpleEndianFlip32S(TempHdr.points, &TempHdr.points);
    SimpleEndianFlip16S(TempHdr.elevs, &TempHdr.elevs);
    SimpleEndianFlip64(TempHdr.avglat ,&TempHdr.avglat);
    SimpleEndianFlip64(TempHdr.avglon ,&TempHdr.avglon);
    SimpleEndianFlip64(TempHdr.avgelev ,&TempHdr.avgelev);
    SimpleEndianFlip64(TempHdr.elscale ,&TempHdr.elscale);
    SimpleEndianFlip16S(TempHdr.MaxEl, &TempHdr.MaxEl);
    SimpleEndianFlip16S(TempHdr.MinEl, &TempHdr.MinEl);

    return fwrite(&TempHdr, sizeof (struct vectorheaderV100), 1, file);

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return fwrite(Hdr, sizeof (struct vectorheaderV100), 1, file);
#else
#error "Unsupported Byte-Order"
#endif
}


// AF, HGW, 22.Jan23
int fread_double_BE(double *Value, FILE *file)
{
    int Result=fread(Value, sizeof (double),1,file);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Flip Endian if host is not Big Endian
    SimpleEndianFlip64(*Value,Value);
#endif
    return Result;
}

// AF, HGW, 22.Jan23
int fread_short_BE(short *Value, FILE *file)
{
    int Result=fread(Value, sizeof (short),1,file);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Flip Endian if host is not Big Endian
    SimpleEndianFlip16S(*Value,Value);
#endif
    return Result;
}

// AF, HGW, 29.Mar23
ssize_t read_float_Array_BE(int filehandle, float *FloatArray, ssize_t size)
{

    ssize_t Result=read(filehandle, FloatArray, size);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    unsigned int i=0;
    for(i=0; i<Result/sizeof(float);i++)  // swap all floats we could read
    {
    	SimpleEndianFlip32F(FloatArray[i],&FloatArray[i]);
    }
    return Result;

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just  return
    return Result;
#else
#error "Unsupported Byte-Order"
#endif
}
