/* Defines.h
** Chock full of delicious, nutritious #define statements that probably
** are bad for you. (And your code.)
** Built from map.h and gis.h and gisam.c and gis.c and all that
** on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent perturbations by Gary R. Huber.
*/

#ifndef GIS_DEFINES_H
#define GIS_DEFINES_H

#ifdef __SASC_60
	#define AMIGA_GUI
	/* #define ROUNDING */
#else /* __SASC_60 */
	#ifdef LATTICE_50
		#define AMIGA_GUI
		#define SAS_5
		#define ROUNDING
	#else /* !LATTICE_50 */
		/* #define AMIGA_GUI */
		/* #define ROUNDING */
	#endif /* LATTICE_50 */
#endif /* __SASC_60 */

#ifdef ROUNDING 
 #define ROUNDING_KLUDGE -.5
#else
 #define ROUNDING_KLUDGE 0
#endif /* ifdef ROUNDING */

#ifdef SAS_5
 #define FGETS_KLUDGE 0
#else
 #define FGETS_KLUDGE 1
#endif /* ifdef LATTICE_50 */

/*
#define FRACTAL_BUILDER
#define DEM_CONTROL_BUILDER
#define CLOUD_FRACTAL_BUILDER
#define DEM_FRACTAL_BUILDER
#define VOLUMETRIC_TEXTURES
#define ANTIALIAS_TEST
*/
#define DBASE_SAVE_COMPOSITE
/*#define OLD_SUN_CALC*/

#define UNDER_CONST

/*#define MEMTRACK*/

#define EXTRASHORTSETTINGS 22
#define EXTRADOUBLESETTINGS 17
#define GRMAPSIZE 64000
#define PROJPLANE 1000
#define MAXSPECIES 3
#define MAXDBH 50
#define HORSCALEFACTOR 90.219

#define INTUITION_REV 37
#define DEM_CURRENT_VERSION	1.02
#define VEC_CURRENT_VERSION	1.00
#define PAR_CURRENT_VERSION	2.10
#define MOTIONPARAMSV1   	30
#define MOTIONPARAMS     	50
#define USEDMOTIONPARAMS 	33
#define COLORPARAMSV1    	30
#define COLORPARAMS      	60
#define ECOPARAMSV1      	20
#define ECOPARAMS        	50
#define SIZE 91000
#define DEPTH 4
#define MODES (HIRES | LACE)
#define ELEVHDRLENV100 48
#define ELEVHDRLENV101 64
#define MAXOBJPTS 2000
#define MAXLOGITEMS 100
#define LARGENUM 32000		/* 10E+12 */
#define FLLARGENUM 10E+6
#define LATSCALE 111.049771	/* 69.003 we're goin' metric! */
#define EARTHRAD 6362.683195	/* 3953.580673 we done gone metric! */
#define Pi 3.14159265
#define HalfPi 1.57079633
#define TwoPi 6.28318531
#define OneAndHalfPi 4.71238898
#define PiOver180 1.74532925199433E-002
#define PiUnder180 5.72957795130823E+001
#define COMP_RGB 0
#define COMP_HSV 1
#define ELSCALE_KILOM	 1.0
#define ELSCALE_METERS	 1.0E-003
#define ELSCALE_CENTIM	 1.0E-005
#define ELSCALE_MILES	 1.6093471
#define ELSCALE_FEET	 3.0480058E-004
#define ELSCALE_INCHES	 2.5400048E-005

/* screen/ecosystem color limits */
#define COL_WATER_MAX 	5
#define COL_WATER_MIN 	2
#define COL_SNOW_MAX 	5
#define COL_SNOW_MIN 	1
#define COL_ROCK_MAX 	15
#define COL_ROCK_MIN 	13
#define COL_BARE_MAX 	15
#define COL_BARE_MIN 	12
#define COL_CONIF_MAX 	11
#define COL_CONIF_MIN 	7
#define COL_DECID_MAX 	11
#define COL_DECID_MIN 	6
#define COL_LOWVEG_MAX 	9
#define COL_LOWVEG_MIN 	6
#define COL_SFC_MAX 	15
#define COL_SFC_MIN 	8

/* colors for illuminated objects */
#define ILLUMVECRED	235
#define ILLUMVECGRN	235
#define ILLUMVECBLU	210

 
/* Standard Messages */
#define ERR_MEM_FAIL	0
#define ERR_OPEN_FAIL	1
#define ERR_READ_FAIL	2
#define ERR_WRITE_FAIL	3
#define ERR_WRONG_TYPE	4
#define ERR_ILL_INST	5
#define ERR_ILL_VAL	6
#define ERR_NO_LOAD	7
#define WNG_OPEN_FAIL	8	
#define WNG_READ_FAIL	9
#define WNG_WRONG_TYPE	10
#define WNG_ILL_INSTR	11
#define WNG_ILL_VAL	12
#define MSG_NO_MOD	13
#define MSG_NO_GUI	14
#define MSG_PAR_LOAD	15
#define MSG_PAR_SAVE	16
#define MSG_DBS_LOAD	17
#define MSG_DBS_SAVE	18
#define MSG_DEM_LOAD	19
#define MSG_DEM_SAVE	20
#define MSG_VEC_LOAD	21
#define MSG_VEC_SAVE	22
#define MSG_IMG_LOAD	23
#define MSG_IMG_SAVE	24
#define MSG_CMP_LOAD	25
#define MSG_CMP_SAVE	26
#define MSG_NO_LOAD	27
#define MSG_UTIL_TAB	28
#define MSG_MPG_MOD	29
#define ERR_DIR_FAIL	30
#define ERR_WIN_FAIL	31
#define MSG_NULL	32
#define DTA_NULL	33
#define ERR_WRONG_SIZE	34
#define WNG_WIN_FAIL	35
#define WNG_WRONG_SIZE	36
#define WNG_WRONG_VER	37
#define MSG_RELEL_SAVE	38
#define WNG_NULL	39
#define MSG_VCS_LOAD	40
#define MSG_PROJ_LOAD	41
#define MSG_PROJ_SAVE	42
#define MSG_DIRLST_LOAD	43
#define ERR_WRONG_VER	44
#define MSG_TIME_ELAPSE	45
#define MSG_TOTAL_ELAPS	46
#define ERR_NULL	47

/* configuration file */
#define CONFIG_DB_HORWIN	0
#define CONFIG_DB		1
#define CONFIG_DO_HORWIN	2
#define CONFIG_DO		3
#define CONFIG_EP_HORWIN	4
#define CONFIG_EP		5
#define CONFIG_DE		8
#define CONFIG_EE		9
#define CONFIG_EC		10
#define CONFIG_EM		11
#define CONFIG_ES		12
#define CONFIG_EMIA_SIZE	13
#define CONFIG_EMIA		14
#define CONFIG_EETL		15
#define CONFIG_ECTL		16
#define CONFIG_EMTL		17
#define CONFIG_MAP_SIZE		18
#define CONFIG_MAP		19
#define CONFIG_EMIA_COMPSIZE	20
#define CONFIG_EMPL		21
#define CONFIG_DC		22
#define CONFIG_DL		23
#define CONFIG_DM		24
#define CONFIG_PS		25
#define CONFIG_FM		26
#define CONFIG_LW		27
#define CONFIG_AN		28
#define CONFIG_EL		29
#define CONFIG_PJ		30
#define CONFIG_SC		31
#define CONFIG_PR		32
#define CONFIG_MA		33
#define CONFIG_DI		34
#define CONFIG_TS		35

#endif /* GIS_DEFINES_H */

#ifndef MAP_DEFINES_H
#define MAP_DEFINES_H

#define EOLN 1
#define BACK_SP 0x08
#define CARRIAGE_RET 0x0d
#define ESC 0x1b
#define UP 0x75
#define SHIFTUP 0x55
#define DOWN 0x64
#define SHIFTDOWN 0x44
#define DELPT 0x2d
#define DELRANGE 0x7f
#define ADDPT 0x2b
#define MARKFIRSTPT 0x6d
#define MARKLASTPT 0x4d
#define SAVEPTS (0x53 || 0x73)
#define QUITPTS 0x51
#define DTOR 57.296
#define VERTPIX 48.177	/* 77.58 */
#define HORPIX 	56.026	/* 90.219 */
#define MAP_INTER_MASK 0x07 /* see mapsupport.c and interactiveview.c */


#endif /* MAP_DEFINES_H */

/* From support.c */
#define QuickCheckEvent(win) (win->UserPort->mp_MsgList.lh_Head->ln_Succ)

/* Re-do of parameters */
#define PAR_FIRST_MOTION(x)	MoPar.mn[x].Value
#define PAR_FIRST_COLOR(x, y)	CoPar.cn[x].Value[y]

#define PAR_NAME_COLOR(x)	CoPar.cn[x].Name

#define PAR_FIRSTLN_ECO(x)	EcoPar.en[x].Line
#define PAR_FIRSTSK_ECO(x)	EcoPar.en[x].Skew
#define PAR_FIRSTSA_ECO(x)	EcoPar.en[x].SkewAz
#define PAR_FIRSTRE_ECO(x)	EcoPar.en[x].RelEl
#define PAR_FIRSTXR_ECO(x)	EcoPar.en[x].MaxRelEl
#define PAR_FIRSTNR_ECO(x)	EcoPar.en[x].MinRelEl
#define PAR_FIRSTXS_ECO(x)	EcoPar.en[x].MaxSlope
#define PAR_FIRSTNS_ECO(x)	EcoPar.en[x].MinSlope
#define PAR_FIRSTDN_ECO(x)	EcoPar.en[x].Density
#define PAR_FIRSTHT_ECO(x)	EcoPar.en[x].Height

#define PAR_COLR_ECO(x)		EcoPar.en[x].Color
#define PAR_UNDER_ECO(x)	EcoPar.en[x].UnderEco
#define PAR_MTCH_ECO(x, y)	EcoPar.en[x].MatchColor[y]
#define PAR_NAME_ECO(x)		EcoPar.en[x].Name
#define PAR_TYPE_ECO(x)		EcoPar.en[x].Type
/*
#define PAR_TREE_ECO(x, 0)	EcoPar.en[x].Density
#define PAR_TREE_ECO(x, 1)	EcoPar.en[x].Height
*/
#define PAR_MODEL_ECO(x)	EcoPar.en[x].Model

#define PARC_RNDR_MOTION(x)	MoShift[x].Value
#define PARC_RNDR_COLOR(x, y)	CoShift[x].Value[y]
#define PARC_RNDRLN_ECO(x)	EcoShift[x].Line
#define PARC_RNDRSK_ECO(x)	EcoShift[x].Skew
#define PARC_RNDRSA_ECO(x)	EcoShift[x].SkewAz
#define PARC_RNDRRE_ECO(x)	EcoShift[x].RelEl
#define PARC_RNDRXR_ECO(x)	EcoShift[x].MaxRelEl
#define PARC_RNDRNR_ECO(x)	EcoShift[x].MinRelEl
#define PARC_RNDRXS_ECO(x)	EcoShift[x].MaxSlope
#define PARC_RNDRNS_ECO(x)	EcoShift[x].MinSlope
#define PARC_RNDRDN_ECO(x)	EcoShift[x].Density
#define PARC_RNDRHT_ECO(x)	EcoShift[x].Height

#define PARC_SKLT_ECO(x)	EcoShift[x].SkewLat
#define PARC_SKLN_ECO(x)	EcoShift[x].SkewLon
#define PARC_MXSL_ECO(x)	EcoShift[x].MXSlope
#define PARC_MNSL_ECO(x)	EcoShift[x].MNSlope

#define PARC_MCOL_ECO(x, y)	EcoShift[x].MainCol[y]
#define PARC_SCOL_ECO(x, y)	EcoShift[x].SubCol[y]

#define UNDOPAR_FIRST_MOTION(y, x)	UndoMoPar[y].mn[x].Value

/* Note: TLSupportGUI.c uses a variation of the EcoPar structure with .en2
**  members to allow array indexing of parameters. This must be modified if
**  the structure is changed: There are no macros!
*/

/* Switches for makemap() in map.c */
#define MMF_REFINE	1 << 1
#define MMF_TOPO	1 << 2
#define MMF_ECO		1 << 3
#define MMF_FLATSPOTS	1 << 4

/* stuff for nngridr */
#define SQ(x)   (x) * (x)
#define BIGNUM  1E37
#define EPSILON 0.00001           
#define MAXRAND 0x7fffffff
#define NRANGE  10
#define EQ      ==
#define NE      !=
#define AND     &&
#define OR      ||
#define RANSEED        367367
#define WEEBIT         0.0001
#define MAXPATHLEN     81
#define LINESIZE       81

#define WCS_MAX_TLWINDOWS	10
#define WCS_MAX_TLVALUES	10

#define WCS_ECOCOLOR_SWAP	1
#define WCS_ECOCOLOR_INCREMENT	2
#define WCS_ECOCOLOR_DECREMENT	3
