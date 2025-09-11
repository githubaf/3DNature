/*
# AF, selco, 3.Mar2025 HGW
rm -f test_locale_redefinitions.o WCS_locale.o test_locale_gcc
m68k-amigaos-gcc -noixemul test_locale_redefinitions.c WCS_locale.c -o test_locale_gcc -I .
vamos test_locale_gcc

rm -f test_locale_redefinitions.o WCS_locale.o test_locale_sasc
vamos -q sc NOGST NOOPT NODEBUG DATA=FAR test_locale_redefinitions.c WCS_locale.c IGNORE=51 
vamos -q slink LIB:c.o test_locale_redefinitions.o  WCS_locale.o  WITH lib:utillib.with LIB LIB:sc.lib LIB:amiga.lib TO test_locale_sasc ND
vamos test_locale_sasc
*/

/* SAS/C only used the first 31 characters of a macro name. 35 MSG_... produced refinition warnings on SAS/C therefore. I manually changed those defines.
 * this tests the correct strings are returned for the changed MSG-numbers
*/

#include <stdio.h>
#include <string.h>
#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

//#include <libraries/locale.h>
#ifndef __SASC
   struct LocaleBase  *LocaleBase=NULL;
#endif

typedef struct
{
   long MsgNumber;
   char *MsgName;
   char *MsgEnglishText;
}Data;

int main(int argc, char **argv)
{
  unsigned int i=0;
  unsigned int Error=0;

static Data TestData[]={ 
   { MSG_BITMAPS_ERREADZBUFFILENOZBODCHUNKOPERATIONTERMINATED       ,"MSG_BITMAPS_ERREADZBUFFILENOZBODCHUNKOPERATIONTERMINATED"       ,"Error reading Z Buffer file!\nNo ZBOD chunk.\nOperation terminated." },
   { MSG_BITMAPS_ERREADZBUFFILENOZBUFCHUNKOPERATIONTERMINATED       ,"MSG_BITMAPS_ERREADZBUFFILENOZBUFCHUNKOPERATIONTERMINATED"       ,"Error reading Z Buffer file!\nNo ZBUF chunk.\nOperation terminated." },
   { MSG_BITMAPS_ERRORREADINGZBUFFERFILERONGSIZEPERATIONTERMINATED  ,"MSG_BITMAPS_ERRORREADINGZBUFFERFILERONGSIZEPERATIONTERMINATED"  ,"Error reading Z Buffer file!\nWrong Size.\nOperation terminated."  }, 
   { MSG_BITMAPS_ERREADBCKGRNDNOBODYCHUNKPERATIONTERMINA            ,"MSG_BITMAPS_ERREADBCKGRNDNOBODYCHUNKPERATIONTERMINA"            ,"Error reading Background file!\nNo BODY Chunk.\nOperation terminated."  },
   { MSG_BITMAPS_ERREADBCKGRNDNOBMHDCHUNKPERATIONTERMINA            ,"MSG_BITMAPS_ERREADBCKGRNDNOBMHDCHUNKPERATIONTERMINA"            ,"Error reading Background file!\nNo BMHD Chunk.\nOperation terminated."  },
   { MSG_BITMAPS_ERRORREADINGBACKGROUNDFILEOMPRESSIONERRORPERATIONTE,"MSG_BITMAPS_ERRORREADINGBACKGROUNDFILEOMPRESSIONERRORPERATIONTE","Error reading Background file!\nCompression error.\nOperation terminated."  },
   { MSG_DEM_OUTOFMEMORYALLOCATINGDEMARRAYSPERATIONTERMINATED       ,"MSG_DEM_OUTOFMEMORYALLOCATINGDEMARRAYSPERATIONTERMINATED"       ,"Out of memory allocating DEM Arrays!\nOperation terminated."  },
   { MSG_DLG_OUTOFMEMORYEXPANDINGDATABASEEDITORLISTASTITEMDOESNOTAPP,"MSG_DLG_OUTOFMEMORYEXPANDINGDATABASEEDITORLISTASTITEMDOESNOTAPP","Out of memory expanding Database Editor List!\nLast item does not appear in list view."  },
   { MSG_DB_OUTOFMEXPDBASEEDITORLSTNEWOBJECTHASBEENCRE              ,"MSG_DB_OUTOFMEXPDBASEEDITORLSTNEWOBJECTHASBEENCRE"              ,"Out of memory expanding Database Editor List!\nNew object has been created but will not appear in list view."  },
   { MSG_DB_OUTOFMEMEXPDBEDITLSTNEWOBJADDED                         ,"MSG_DB_OUTOFMEMEXPDBEDITLSTNEWOBJADDED"                         ,"Out of memory expanding Database Editor List!\nNew object has been added but will not appear in list view."  },
   { MSG_DATAOPS_INCORRFILSIZFORSPECIFHEADERWDTHANDHIGHTPE          ,"MSG_DATAOPS_INCORRFILSIZFORSPECIFHEADERWDTHANDHIGHTPE"          ,"Incorrect file size for specified header, width and height!\nOperation terminated."  },
   { MSG_DATAOPSGUI_WRNFILENOTIFF                                   ,"MSG_DATAOPSGUI_WRNFILENOTIFF"                                   ,"Warning\nFile is not an IFF file."  },
   { MSG_DATAOPSGUI_WRNFILENOTIFFIMAGFILE                           ,"MSG_DATAOPSGUI_WRNFILENOTIFFIMAGFILE"                           ,"Warning\nFile is not an IFF image file."  },
   { MSG_EDDB_OUTOFMEMCANTOPENDBLIST                                ,"MSG_EDDB_OUTOFMEMCANTOPENDBLIST"                                ,"Out of memory!\nCan't open database list."  }, 
   { MSG_EVMORGUI_ERRSAVECLONEDCLOUDFILE                            ,"MSG_EVMORGUI_ERRSAVECLONEDCLOUDFILE"                            ,"Error saving the cloned Cloud file."  },
   { MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE           ,"MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE"           ,"Out of memory allocating antialias and edge buffers!\nOperation terminated."  },
   { MSG_GLMP_OUTOFMEMCREATCLOUDMAP                                 ,"MSG_GLMP_OUTOFMEMCREATCLOUDMAP"                                 ,"Out of memory creating Cloud Map!"  },
   { MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI             ,"MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI"             ,"Out of memory allocating Polygon Smoothing array!\nContinue without Polygon Smoothing?"  },
   { MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED                            ,"MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED"                            ,"Error loading DEM Object!\nObject not saved."  },
   { MSG_LINESPRT_OUTOFMEMCREATNEWVECTOROBJECTPERATIONTERMINAT      ,"MSG_LINESPRT_OUTOFMEMCREATNEWVECTOROBJECTPERATIONTERMINAT"      ,"Out of memory creating new vector object!\nOperation terminated."  },
   { MSG_MAP_SETNORTHWESTREFPOINT                                   ,"MSG_MAP_SETNORTHWESTREFPOINT"                                   ,"Set northwest reference point"  },
   { MSG_MAP_SETSOUTHEASTREFPOINT                                   ,"MSG_MAP_SETSOUTHEASTREFPOINT"                                   ,"Set southeast reference point"  },
   { MSG_MAPEXTRA_SETDESTINATIONPT                                  ,"MSG_MAPEXTRA_SETDESTINATIONPT"                                  ,"Set destination point"  },
   { MSG_MAPEXTRA_SELECTFIRSTSRCVERTEX                              ,"MSG_MAPEXTRA_SELECTFIRSTSRCVERTEX"                              ,"Select first source vertex"  },
   { MSG_MAPEXTRA_SELECTLASTSRCVERTEX                               ,"MSG_MAPEXTRA_SELECTLASTSRCVERTEX"                               ,"Select last source vertex"  },
   { MSG_MAPEXTRA_SELSTREAMSTARTPT                                  ,"MSG_MAPEXTRA_SELSTREAMSTARTPT"                                  ,"Select stream start point"  },
   { MSG_MAPEXTRA_SELAPPRSTREAMENDPT                                ,"MSG_MAPEXTRA_SELAPPRSTREAMENDPT"                                ,"Select approximate stream end point"  },
   { MSG_MAPEXTRA_SELSURFELEVATION                                  ,"MSG_MAPEXTRA_SELSURFELEVATION"                                  ,"Select Surface %lu Elevation"  },
   { MSG_MAPEXTRA_ALLCORNERPNTSMUSTBEWITHINSAMEDEMPERATIONTERMINAT  ,"MSG_MAPEXTRA_ALLCORNERPNTSMUSTBEWITHINSAMEDEMPERATIONTERMINAT"  ,"All corner points must be within same DEM!\nOperation terminated."  },
   { MSG_MAPEXTRA_OUTOFMEMNOTENOUGHFORTEMPTOPOARRAYPERATIONFAIL     ,"MSG_MAPEXTRA_OUTOFMEMNOTENOUGHFORTEMPTOPOARRAYPERATIONFAIL"     ,"Out of memory!\nNot enough for temporary topo array.\nOperation failed."  },
   { MSG_MAPGUI_VECTTOPOCONFABORTEDOBJECTSCOMPLETED                 ,"MSG_MAPGUI_VECTTOPOCONFABORTEDOBJECTSCOMPLETED"                 ,"Vector topo conformation aborted! %d objects completed."  },
   { MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2      ,"MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2"      ,"The current Ecosystem Model has been modified. Do you wish to save it before proceeding?"  },
   { MSG_PARGUI_ERROPENCOSYSMODELFILEFOROUTPUTPERATIONTERMI         ,"MSG_PARGUI_ERROPENCOSYSMODELFILEFOROUTPUTPERATIONTERMI"         ,"Error opening Ecosystem Model file for output!\nOperation terminated."  },
   { MSG_PARGUI_NOTSELECTEDAFILENAMEFOROUTPUTPERATIONTERMINAT       ,"MSG_PARGUI_NOTSELECTEDAFILENAMEFOROUTPUTPERATIONTERMINAT"       ,"You have not selected a file name for output!\nOperation terminated."  },
   { MSG_NNCRUNCH_RATIOOFVERTTOHORIZMAPDIMENSIONSISTOOSM            ,"MSG_NNCRUNCH_RATIOOFVERTTOHORIZMAPDIMENSIONSISTOOSM"            ,"The ratio of vertical to horizontal map dimensions is too small for gradient estimation. Scale the data if gradients are required.\nDo you wish to continue without gradient estimation?"  },
   { 0                                                              ,NULL                                                             ,NULL}  
};

Locale_Open((STRPTR)"WCS.catalog",1/*,1*/);  // Version/*, revision*/  - Simplecat Doc says: There is no need to check any result.

 while(TestData[i].MsgNumber)
 {
   if(strcmp(GetString(TestData[i].MsgNumber),TestData[i].MsgEnglishText))
   {
     printf("Error in MsgNumber %u, i.e. %s\n",TestData[i].MsgNumber,TestData[i].MsgName);

     printf("Is       :\n%s\n",GetString(TestData[i].MsgNumber));
     printf("---------------------------------------------------------------------------------------\n");
     printf("Should be:\n%s\n",TestData[i].MsgEnglishText);
     printf("---------------------------------------------------------------------------------------\n");
     Error=1;
     break;
   }
   i++;
 }



 printf("\n\n");
 if(Error)
 {
 	printf("Error! %s failed.\n",argv[0]);
 }
 else
 {
    printf("OK. %s passed.\n",argv[0]);
 }
 Locale_Close();
 return Error;
}
