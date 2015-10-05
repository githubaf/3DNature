// ForestWiz.cpp
// Code file for ForestWiz
// Created from scratch on 2/4/04 by Gary R. Huber
// Copyright 2004 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ForestWiz.h"
#include "Requester.h"
#include "Project.h"
#include "Raster.h"
#include "AppMem.h"
#include "DBFilterEvent.h"
#include "resource.h"

static char *ForestWizPageText[WCS_FORESTWIZ_NUMPAGES] = 
	{
	/*IDD_FORESTWIZ_WELCOME*/"Welcome to the Visual Nature Studio Forestry Applications Wizard, or \"Forestry Wizard\" for short.\r\r\n\r\r\nThe Forestry Wizard may be used as a fast and easy way to get realistic results from VNS's powerful land cover rendering engine. You will be asked questions about your data and from your answers VNS will determine the most practical solution for mapping the data into land cover units that VNS can render.\r\r\n\r\r\nIn many cases there is more than one way to achieve the results you want because VNS has so many features. It can be very difficult to determine the absolute best way to achieve the desired results. This Wizard will do its best to analyze the options and provide a good solution.\r\r\n\r\r\nProceed from panel to panel pressing the \"Next-->\" button whenever you are finished answering the questions on each panel. At any time you can backtrack with the \"<--Back\" button without losing any settings you have made along the way. If at some point the \"Next-->\" button does not respond then there is some question unanswered on the current panel. It must be answered before you can continue. When all the questions have been answered on all the panels, you can choose to go through one more time to review all your answers before you finalize the operation.\r\r\n\r\r\nIt is important to note that if you cancel the Wizard or close the window before you execute the \"Finish\" command on the last panel, no changes will be made to your VNS project. Once you hit the \"Finish\" button there will be changes to your project even if the Wizard operation can not be completed successfully for some reason. With this in mind it is a good idea to save the project now, before you begin the Wizard operation.",
	/*IDD_FORESTWIZ_CANCEL*/"Are you sure you want to cancel the Wizard operation?",
	/*IDD_FORESTWIZ_CONFIRM*/"Thank you for using the Forestry Wizard. You have answered all the questions necessary for the Wizard to process your request.\r\r\n\r\r\nIf you are sure you made all the correct answers and have selected all the options you wish, hit \"Finish\" to complete the operation.\r\r\n\r\r\nIf you think you may have made a mistake or simply wish to review your answers to make sure they are correct, hit the \"Review Instructions\" button to go through each panel from the beginning. As long as you don't change any answers that cause some of your other supplied answers to be implausible, nothing will be lost by reviewing your instructions.",
	/*IDD_FORESTWIZ_COMPLETE*/"The operation of this Wizard is complete. To begin a new operation please close this window and re-open it.",
	/*IDD_FORESTWIZ_COMPLETEERROR*/"An error has occurred completing the Wizard operation. Would you like to review your instructions?",
	/*IDD_FORESTWIZ_PAGEERROR*/"A PAGE ERROR HAS OCCURRED. The requested page could not be found.",
	/*IDD_FORESTWIZ_CMAPORVEC*/"Do you want to match land cover units with colors in a land cover classification image (Color Map) or define land cover units based on the areal extents of vector polygons or do you want to define specific locations of foliage based on vector point data?\r\r\n\r\r\nIf you wish to use a Color Map please note that the Forestry Wizard is only capable of assigning specific colors in a classified image to land cover units. If you need to match land cover units to a range of colors in your source image you will need to perform additional steps outside the Wizard to select the full range of matching colors.",
	/*IDD_FORESTWIZ_IMGLOADED*/"Is the land cover classification image already loaded in the Image Object Library?",
	/*IDD_FORESTWIZ_SELIMG*/"Please select the Image Object you want to use for land cover classification. Preferably it should be an image with discrete colors representing each land cover class.\r\r\n\r\r\nIf your image has ranges of color to represent each class then not all steps can be performed in the Wizard but a good start can be made here. You will later need to refine the ranges of image color matching each cover class from within the Color Map editor.",
	/*IDD_FORESTWIZ_FINDIMG*/"Please specify the location of the land cover classification image on your hard drive. If you like you can use the disk icon to open a file requester for browsing your system. Preferably it should be an image with discrete colors representing each land cover class.\r\r\n\r\r\nIf your image has ranges of color to represent each class then not all steps can be performed in the Wizard but a good start can be made here. You will later need to refine the ranges of image color matching each cover class from within the Color Map editor.",
	/*IDD_FORESTWIZ_CLICKORNUM*/"Do you wish to define the matching colors by clicking on the image itself or by entering the color values numerically?",
	/*IDD_FORESTWIZ_CMAPCLICKIMG*/"When you hit the \"Show Image\" button a window will open with the land cover classification image displayed. You must then click on colors in the image, one at a time, and tell us what land cover name you want to give each color. Click \"Next Color\" if you want to define another color or \"Next -->\" when you have finished.",
	/*IDD_FORESTWIZ_CMAPNUMERIC*/"For each land cover unit you wish to identify, tell us what land cover name you want to give each color and tell us the RGB (red, green, blue) color values for each unit based on a color scale of 0 (black) to 255 (full color). Click \"Next Color\" if you want to define another color or \"Next -->\" when you have finished.",
	/*IDD_FORESTWIZ_ANALYZEIMG*/"Would you like us to analyze the image and check for any colors missed?",
	/*IDD_FORESTWIZ_UNIDENTCOLOR*/"There were some unidentified colors found in the image. Is each land cover unit represented by only one distinct color in the image or is a unit represented by a range of colors?",
	/*IDD_FORESTWIZ_NORANGES*/"This Wizard cannot let you designate ranges of colors. Once the rest of the Wizard operation is completed you can go to the Color Map Editor and supply the rest of the color range information.",
	/*IDD_FORESTWIZ_UNMATCHEDCOLOR*/"The first unmatched color is displayed below. Please supply a name for this color and hit the \"Next Color\" button to continue through the rest of the unidentified colors.\r\r\n\r\r\nName as many of the colors as you wish, you do not need to name them all. You must however name at least one of them. Any colors you do not supply names for will not be matched to land cover units and will be rendered in the unmatched color itself.\r\r\n\r\r\nWhen you come to a color that has all color values of 0 you have finished.",
	/*IDD_FORESTWIZ_NOCMAPMATCHES*/"No land cover units have been specified. Color Map placement of land cover units cannot be done unless at least one land cover unit is matched to a color in the land cover classification image. Use the \"Back\" button to go back and specify units to match or the \"Next\" button to exit.",
	/*IDD_FORESTWIZ_UNITBASICINFO*/"You must now supply information about each land cover unit. Step through the defined units using the \"Next Unit\" button and complete the information about each unit. If you skip this step VNS will construct generic land cover units which you can later add foliage and textures to to make them more realistic.\r\r\n\r\r\nEach land cover unit will become an Ecosystem in VNS. You can add as many foliage images to the Ecosystem as you like. First enable foliage by checking the \"Unit Has Foliage\" checkbox. Then make an image selection and use the \"Next Image\" button to record the selection and clear the file field for the next entry or make your selections in the Image Object Library and use the button labeled \"Grab selected images from Image Object Library\" to bring the selections into the current unit.\r\r\n\r\r\nThe \"Height\" and \"Stems per Unit Area\" fields can be used to vary the proportions of each image and their relative heights within the overall land cover unit.\r\r\n\r\r\nUse the \"Next-->\" button only when all of the land cover units and their foliage have been fully defined.",
	/*IDD_FORESTWIZ_VECLOADED*/"Is the vector data already loaded in the Database?",
	/*IDD_FORESTWIZ_LOADVEC*/"The Import Wizard should be used to load the vector data now. Click the \"Open Import Wizard\" button to open the Wizard. A file requester will be presented to you first. Select the file that contains the vector data that will be used to define the placement of your land cover units or foliage stems. When the Import Wizard has loaded your data hit the \"Next-->\" button to continue with the Forestry Wizard.\r\r\n\r\r\nIf you are planning to import a Shapefile make sure to import any attributes along with it that will be used to control land cover types, foliage species, foliage size and density.",
	/*IDD_FORESTWIZ_SPSCENARIO*/"Which scenario best describes the vector data attributes that you wish to use to define land cover or species types and the densities and sizes of foliage?",
	/*IDD_FORESTWIZ_ISTHEREVEG*/"Is there vegetation associated with any of the land cover units?",
	/*IDD_FORESTWIZ_HTDENSATTRIBS*/"Are either foliage size or density driven by database attributes? In other words, do you want attributes associated with database polygons or points to determine the sizes and densities of foliage?",
	/*IDD_FORESTWIZ_SELONESPFIELD*/"Select the database attribute that tells the species of vegetation or the land cover classification.",
	/*IDD_FORESTWIZ_SELONESIZEFIELD*/"Select the database attribute that tells the size of the vegetation. This can be a height attribute or another measurement that can be used to calculate height such as diameter or age.\r\r\n\r\r\nIf the size of vegetation is not controlled by a database attribute then leave the drop box blank and proceed to the next panel.",
	/*IDD_FORESTWIZ_SELONEDENSFIELD*/"Select the database attribute that tells the density (stocking) of the vegetation. Density is usually specified as number of stems per unit area but can also be specified as an amount of basal area or crown closure. You will be asked for the areal unit later.\r\r\n\r\r\nIf the density of vegetation is not controlled by a database attribute then leave the drop box blank and proceed to the next panel.",
	/*IDD_FORESTWIZ_SELFIRSTSPFIELD*/"Select the database attribute that names the first (dominant) species.",
	/*IDD_FORESTWIZ_SELSECONDSPFIELD*/"Select the database attribute that names the second (co-dominant) species.",
	/*IDD_FORESTWIZ_DENSITYMETHOD*/"VNS either needs foliage density to be specified as stems per unit area or some other units that can be converted to stems. You have already selected one or more attibutes to describe the density of your foliage. What type of data are those density attributes?\r\r\n\r\r\nBasal Area requires that the average diameter of trees in a unit also be known in order to compute the number of stems per unit area. Crown Closure can be given as a percentage (0-100) or a decimal fraction (0-1.0).",
	/*IDD_FORESTWIZ_AREAUNITS*/"What units of measurement is density (stocking) defined for in your database?",
	/*IDD_FORESTWIZ_BASALAREAUNITS*/"What units of measurement is the basal area field in your database?",
	/*IDD_FORESTWIZ_MULTIDBHFIELD*/"Is there a separate diameter attribute for each species?",
	/*IDD_FORESTWIZ_ONEORTWODBHFIELD*/"Is there a separate diameter attribute for first (dominant) and second (co-dominant) species?",
	/*IDD_FORESTWIZ_SELONEDBHFIELD*/"Select the database attribute that contains the diameter of foliage trunks. This may be known as DBH or diameter at breast height.",
	/*IDD_FORESTWIZ_SELFIRSTDBHFIELD*/"Select the database attribute that contains the diameter of foliage trunks of the first (dominant) species. This may be known as DBH or diameter at breast height.",
	/*IDD_FORESTWIZ_SELSECONDDBHFIELD*/"Select the database attribute that contains the diameter of foliage trunks of the second (co-dominant) species. This may be known as DBH or diameter at breast height.",
	/*IDD_FORESTWIZ_SELMULTIDBHFIELD*/"Select the database attribute that tells the diameter of each species. After you have made a selection hit the \"Next Species\" button and continue the selection.",
	/*IDD_FORESTWIZ_DBHUNITS*/"What units of measurement is the diameter field in your database?",
	/*IDD_FORESTWIZ_CLOSUREPRECISION*/"Is the crown closure specified as percentage values with a range of 0 to 100% or as a decimal value ranging from 0.0 to 1.0?",
	/*IDD_FORESTWIZ_SIZEMETHOD*/"VNS either needs foliage size to be specified as height in units of meters or some other units that can be converted to metric height. If your size data is not height or not in meters you need to specify some guidelines that can be used to convert your size attributes to meters of height.\r\r\n\r\r\nThe next questions asked will determine if a size conversion is necessary and what the conversion method should be.\r\r\n\r\r\nWhat data do you have that can be used to determine foliage size?",
	/*IDD_FORESTWIZ_SIZEUNITS*/"What are the units of measurement for foliage height?",
	/*IDD_FORESTWIZ_SIZEAGEEQUIV*/"How tall would a specimen be at four different ages? Consider the age values to be in the same units (such as years) as the age values in your database. What are the height units you are using for reference?\r\r\n\r\r\nA graph will be created from the measurements you give here. The graph is used to look up heights from the ages supplied by your database. The same graph will be used for all species. When the Wizard operation is complete you can modify the graphs for different species to achieve finer height control.",
	/*IDD_FORESTWIZ_SIZEDBHEQUIV*/"Figuring with the same units that the diameter is specified in your database, how tall would a specimen of four different diameters be? What units are you using for height? What units is diameter specified in, both here and in the database?\r\r\n\r\r\nA graph will be created from the measurements you give here. The graph is used to look up heights from the diameters supplied by your database. The same graph will be used for all species. When the Wizard operation is complete you can modify the graphs for different species to achieve finer height control.",
	/*IDD_FORESTWIZ_SIZEMULT*/"We have calculated a height multiplication factor for translating your foliage height data into the metric height VNS requires. That multiplier is shown below. This is a number that when multiplied times your height data will give a height in meters. If you feel that this is incorrect please enter the height factor you feel is correct.",
	/*IDD_FORESTWIZ_DBHMULT*/"We have calculated a diameter multiplication factor for translating your foliage diameter data into the metric diameter VNS requires. That multiplier is shown below. This is a number that when multiplied times your diameter data will give a diameter in meters. If you feel that this is incorrect please enter the diameter factor you feel is correct.",
	/*IDD_FORESTWIZ_HEIGHTTYPE*/"Are the foliage sizes specified in the database with some individuals being larger and some smaller than an average size, or are the maximum sizes specified with all individuals being that size or less within a percentage range, or do you know both the maximum and minimum sizes of the foliage?",
	/*IDD_FORESTWIZ_AVGHEIGHTTYPE*/"Is the percentage height variation found in a database attribute? If so, select the height variation attribute. Otherwise, what is the percentage height variation in either direction from the average? For intance if you specify a 20% variation above and below then individuals might be up to 20% shorter or 20% taller than the average.",
	/*IDD_FORESTWIZ_AVGDBHTYPE*/"Is the percentage diameter variation found in a database attribute? If so, select the diameter variation attribute. Otherwise, what is the percentage diameter variation in either direction from the average? For intance if you specify a 20% variation above and below then individuals might be up to 20% smaller or 20% larger than the average.",
	/*IDD_FORESTWIZ_AVGAGETYPE*/"Is the percentage age variation found in a database attribute? If so, select the age variation attribute. Otherwise, what is the percentage age variation in either direction from the average? For intance if you specify a 20% variation above and below then individuals might be up to 20% younger or 20% older than the average.",
	/*IDD_FORESTWIZ_MAXHEIGHTTYPE*/"Is the minimum foliage height percentage found in a database attribute? If so, select the minimum foliage height percentage attribute. Otherwise, what is the minimum foliage height expressed as a percentage of the maximum height? For instance if you specify a minimum height % of 80% then the smallest individuals will be 20% shorter than the largest.",
	/*IDD_FORESTWIZ_MAXDBHTYPE*/"Is the minimum foliage diameter percentage found in a database attribute? If so, select the minimum foliage diameter percentage attribute. Otherwise, what is the minimum foliage diameter expressed as a percentage of the maximum diameter? For instance if you specify a minimum diameter % of 80% then the smallest individuals will be 20% smaller than the largest.",
	/*IDD_FORESTWIZ_MAXAGETYPE*/"Is the minimum foliage age percentage found in a database attribute? If so, select the minimum foliage age percentage attribute. Otherwise, what is the minimum foliage age expressed as a percentage of the maximum age? For instance if you specify a minimum age % of 80% then the smallest individuals will be 20% younger than the largest.",
	/*IDD_FORESTWIZ_MAXMINHEIGHTTYPE*/"Is the minimum foliage height found in a database attribute? If so, select the minimum foliage height attribute. Otherwise, what is the minimum foliage height?",
	/*IDD_FORESTWIZ_MAXMINDBHTYPE*/"Is the minimum foliage diameter found in a database attribute? If so, select the minimum foliage diameter attribute. Otherwise, what is the minimum foliage diameter?",
	/*IDD_FORESTWIZ_MAXMINAGETYPE*/"Is the minimum foliage age found in a database attribute? If so, select the minimum foliage age attribute. Otherwise, what is the minimum foliage age?",
	/*IDD_FORESTWIZ_NUMSIZEFIELD*/"Is there a separate size attribute for the dominant and co-dominant species? Size attributes can be height, diameter or age.",
	/*IDD_FORESTWIZ_NUMDENSFIELD*/"Is there a separate density (stocking) attribute for the dominant and co-dominant species?",
	/*IDD_FORESTWIZ_SELFIRSTSIZEFIELD*/"Select the database attribute that tells the size of the first (dominant) species. This can be a height attribute or another measurement that can be used to calculate height such as diameter or age.",
	/*IDD_FORESTWIZ_SELSECONDSIZEFIELD*/"Select the database attribute that tells the size of the second (co-dominant) species. This can be a height attribute or another measurement that can be used to calculate height such as diameter or age.",
	/*IDD_FORESTWIZ_SELFIRSTDENSFIELD*/"Select the database attribute that tells the density (stocking) of the first (dominant) species. Density is usually specified as number of stems per unit area but can also be specified as an amount of basal area or crown closure. You will be asked for the areal unit later.",
	/*IDD_FORESTWIZ_SELSECONDDENSFIELD*/"Select the database attribute that tells the density (stocking) of the second (co-dominant) species. Density is usually specified as number of stems per unit area but can also be specified as an amount of basal area or crown closure. You will be asked for the areal unit later.",
	/*IDD_FORESTWIZ_SELMULTIDENSFIELD*/"Select each database attribute that tells the density (stocking) of a species of vegetation. Density is usually specified as number of stems per unit area but can also be specified as an amount of basal area or crown closure. Use the Control and Shift keys to multi-select attributes.",
	/*IDD_FORESTWIZ_MULTISIZEFIELD*/"Is there a separate size attribute for each species? Size attributes can be height, diameter or age.",
	/*IDD_FORESTWIZ_SELMULTISIZEFIELD*/"Select the database attribute that tells the size of each species. This can be a height attribute or another measurement that can be used to calculate height such as diameter or age. After you have made a selection hit the \"Next Species\" button and continue the selection.",
	/*IDD_FORESTWIZ_SELMULTISPFIELD*/"Select each database attribute that tells one of the species of vegetation. Use the Control and Shift keys to multi-select attributes.",
	/*IDD_FORESTWIZ_SELMULTISPDENSFIELD*/"Select the database attribute that tells the relative proportion of each species. This can be a percentage (0-100) or a decimal fraction (0-1.0). After you have made a selection hit the \"Next Species\" button and continue the selection.",
	/*IDD_FORESTWIZ_SPDENSPRECISION*/"Is the relative proportion of each species specified as percentage values with a range of 0 to 100% or as a decimal value ranging from 0 to 1.0?",
	/*IDD_FORESTWIZ_POLYBASICINFO*/"You must now supply information about each land cover class or species. Step through the defined classes using the \"Next Class\" button and complete the information about each class. If you do not wish to enter all foliage image information at this time you must at least indicate which of the Classes will eventually contain foliage by selecting the \
\"Class Has Foliage\" checkbox.\r\r\n\r\r\nYou can add as many foliage images to each Class as you like. First enable foliage by checking the \"Class Has Foliage\" checkbox. Then make an image selection and use the \"Next Image\" button to record the selection and clear the file field for the next entry or make your selections in the Image Object Library and use the button labeled \
\"Grab selected images from Image Object Library\" to bring the selections into the current Class.\r\r\n\r\r\nThe \"Height\" and \"Stems per Unit Area\" fields can be used to vary the proportions of each image and their relative heights within the overall Class.\r\r\n\r\r\nUse the \"Next-->\" button only when all of the Classes and their foliage have been fully defined."
	// WCS_FORESTRYWIZARD_ADDPAGE - add the text for the page in the same order it is in the list of page defines in ForestWiz.h
	};

unsigned short ForestWizPageResourceID[WCS_FORESTWIZ_NUMPAGES] = 
	{
	IDD_FORESTWIZ_WELCOME,
	IDD_FORESTWIZ_CANCEL,
	IDD_FORESTWIZ_CONFIRM,
	IDD_FORESTWIZ_COMPLETE,
	IDD_FORESTWIZ_COMPLETEERROR,
	IDD_FORESTWIZ_PAGEERROR,
	IDD_FORESTWIZ_CMAPORVEC,
	IDD_FORESTWIZ_IMGLOADED,	// ImageLoaded
	IDD_FORESTWIZ_SELIMG,		// CMapRast
	IDD_FORESTWIZ_FINDIMG,		// CMapRast
	IDD_FORESTWIZ_CLICKORNUM,	// ClickOrNumeric
	IDD_FORESTWIZ_CMAPCLICKIMG,	// CMapMatchNames, CMapMatchColors
	IDD_FORESTWIZ_CMAPNUMERIC,	// CMapMatchNames, CMapMatchColors
	IDD_FORESTWIZ_ANALYZEIMG,	// AnalyzeImage, UnmatchedColors, MaxUnitsExceeded, CMapMatchColors
	IDD_FORESTWIZ_UNIDENTCOLOR,	// CMapColorRange
	IDD_FORESTWIZ_NORANGES,
	IDD_FORESTWIZ_UNMATCHEDCOLOR,	// CMapMatchNames, CMapMatchColors
	IDD_FORESTWIZ_NOCMAPMATCHES,
	IDD_FORESTWIZ_UNITBASICINFO,	// CMapUnitInfo, FoliageBearingUnits
	IDD_FORESTWIZ_VECLOADED,
	IDD_FORESTWIZ_LOADVEC,
	IDD_FORESTWIZ_SPSCENARIO,
	IDD_FORESTWIZ_ISTHEREVEG,
	IDD_FORESTWIZ_HTDENSATTRIBS,
	IDD_FORESTWIZ_SELONESPFIELD,
	IDD_FORESTWIZ_SELONESIZEFIELD,
	IDD_FORESTWIZ_SELONEDENSFIELD,
	IDD_FORESTWIZ_SELFIRSTSPFIELD,
	IDD_FORESTWIZ_SELSECONDSPFIELD,
	IDD_FORESTWIZ_DENSITYMETHOD,
	IDD_FORESTWIZ_AREAUNITS,
	IDD_FORESTWIZ_BASALAREAUNITS,
	IDD_FORESTWIZ_MULTIDBHFIELD,
	IDD_FORESTWIZ_ONEORTWODBHFIELD,
	IDD_FORESTWIZ_SELONEDBHFIELD,
	IDD_FORESTWIZ_SELFIRSTDBHFIELD,
	IDD_FORESTWIZ_SELSECONDDBHFIELD,
	IDD_FORESTWIZ_SELMULTIDBHFIELD,
	IDD_FORESTWIZ_DBHUNITS,
	IDD_FORESTWIZ_CLOSUREPRECISION,
	IDD_FORESTWIZ_SIZEMETHOD,
	IDD_FORESTWIZ_SIZEUNITS,
	IDD_FORESTWIZ_SIZEAGEEQUIV,
	IDD_FORESTWIZ_SIZEDBHEQUIV,
	IDD_FORESTWIZ_SIZEMULT,
	IDD_FORESTWIZ_DBHMULT,
	IDD_FORESTWIZ_HEIGHTTYPE,
	IDD_FORESTWIZ_AVGHEIGHTTYPE,
	IDD_FORESTWIZ_AVGDBHTYPE,
	IDD_FORESTWIZ_AVGAGETYPE,
	IDD_FORESTWIZ_MAXHEIGHTTYPE,
	IDD_FORESTWIZ_MAXDBHTYPE,
	IDD_FORESTWIZ_MAXAGETYPE,
	IDD_FORESTWIZ_MAXMINHEIGHTTYPE,
	IDD_FORESTWIZ_MAXMINDBHTYPE,
	IDD_FORESTWIZ_MAXMINAGETYPE,
	IDD_FORESTWIZ_NUMSIZEFIELD,
	IDD_FORESTWIZ_NUMDENSFIELD,
	IDD_FORESTWIZ_SELFIRSTSIZEFIELD,
	IDD_FORESTWIZ_SELSECONDSIZEFIELD,
	IDD_FORESTWIZ_SELFIRSTDENSFIELD,
	IDD_FORESTWIZ_SELSECONDDENSFIELD,
	IDD_FORESTWIZ_SELMULTIDENSFIELD,
	IDD_FORESTWIZ_MULTISIZEFIELD,
	IDD_FORESTWIZ_SELMULTISIZEFIELD,
	IDD_FORESTWIZ_SELMULTISPFIELD,
	IDD_FORESTWIZ_SELMULTISPDENSFIELD,
	IDD_FORESTWIZ_SPDENSPRECISION,
	IDD_FORESTWIZ_POLYBASICINFO
	// WCS_FORESTRYWIZARD_ADDPAGE - add the panel resource ID in the same order it is in the list of page defines in ForestWiz.h
	};

ForestWiz::ForestWiz()
{
unsigned short PageCt;

CancelOrder = 0;
FinalOrderCancelled = 0;
ReviewOrder = 0;
CMapOrVec = WCS_FORESTWIZ_CMAPORVEC_CMAP;
ImageLoaded = GlobalApp->AppImages->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF) ? 1: 0;
CMapRast = NULL;
CMapImage.SetPath(GlobalApp->MainProj->imagepath);
ClickOrNumeric = WCS_FORESTWIZ_CLICKORNUMERIC_CLICK;
AnalyzeImage = 1;
UnmatchedColors = 0;
MaxUnitsExceeded = 0;
CMapColorRange = 0;
FoliageBearingUnits = 0;
MatchedCMapColors = 0;
CMapColorResponseEnabled = 0;
NumEcoUnits = 0;
EcoUnits = NULL;
SpeciesScenario = WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD;
NumSpeciesFields = 0;
NumSizeFields = 1;
NumDensFields = 1;
VegetationExists = 1;
HDAttribsExist = 0;
LinkViaAttribute = 0;
VectorsLoaded = GlobalApp->AppDB->AreThereVectors();
NumClassUnits = 0;
NumSpDensFields = 0;
ClassUnits = NULL;
SizeThemeMult = 1.0;
SizeUnits = WCS_FORESTWIZ_SIZEUNITS_FEET;
DBHUnits = WCS_FORESTWIZ_SIZEUNITS_INCHES;
SizeMeasurement = WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT;
AgeEquiv[0] = 19.0;
AgeEquiv[1] = 40.0;
AgeEquiv[2] = 56.0;
AgeEquiv[3] = 66.0;
DBHEquiv[0] = 23.0;
DBHEquiv[1] = 40.0;
DBHEquiv[2] = 56.0;
DBHEquiv[3] = 66.0;
DBHEquivUnits[0] = 5.0;
DBHEquivUnits[1] = 10.0;
DBHEquivUnits[2] = 20.0;
DBHEquivUnits[3] = 40.0;
HeightType = WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE;
AvgHeightRange = 20.0;
MinHeightRange = 80.0;
MinHeightAbs = 0.0;
HeightMult = 1.0;
ClosurePrecision = WCS_FORESTWIZ_PRECISION_PERCENTAGE;
DensityMethod = WCS_FORESTWIZ_DENSITYMETHOD_STEMS;
NumDBHFields = 1;
DBHThemeMult = DensityThemeMult = 1.0;
DBHThemeUnits = WCS_FORESTWIZ_SIZEUNITS_INCHES;
BasalAreaUnits = WCS_FORESTWIZ_SIZEUNITS_FEET;
EcoAreaUnits = WCS_FOLIAGE_DENSITY_ACRE;
RelativeDensityMult = .01;
SPDensPrecision = WCS_FORESTWIZ_PRECISION_PERCENTAGE;
ThematicMapsAllowed = 1;
MinSizeThemePresent = 0;

for (PageCt = 0; PageCt < WCS_FORESTWIZ_MAXSPECIES; PageCt ++)
	{
	SpeciesAttr[PageCt] = NULL;
	DensAttr[PageCt] = NULL;
	SizeAttr[PageCt] = NULL;
	DBHAttr[PageCt] = NULL;
	SpDensAttr[PageCt] = NULL;
	} // for

for (PageCt = 0; PageCt < WCS_FORESTWIZ_MAXNUMUNITS; PageCt ++)
	{
	CMapMatchNames[PageCt][0] = 0;
	CMapMatchColors[PageCt][0] = CMapMatchColors[PageCt][1] = CMapMatchColors[PageCt][2] = 0;
	} // for

for (PageCt = 0; PageCt < WCS_FORESTWIZ_NUMPAGES; PageCt ++)
	{
	ConfigurePage(&Wizzes[PageCt], PageCt);
	} // for
Wizzes[WCS_FORESTWIZ_WIZPAGE_WELCOME].AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CONFIRM]);
Wizzes[WCS_FORESTWIZ_WIZPAGE_WELCOME].AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CMAPORVEC]);

} // ForestWiz::ForestWiz

/*===========================================================================*/

ForestWiz::~ForestWiz()
{

if (EcoUnits)
	delete [] EcoUnits;
if (ClassUnits)
	delete [] ClassUnits;

} // ForestWiz::~ForestWiz

/*===========================================================================*/

WizardlyPage *ForestWiz::ProcessPage(WizardlyPage *ProcessMe, Project *CurProj)
{
long Ct;

if (CancelOrder && ! FinalOrderCancelled)
	{
	ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CANCEL]);
	return (ProcessMe->Next);
	} // if

switch (ProcessMe->WizPageID)
	{
	case WCS_FORESTWIZ_WIZPAGE_WELCOME:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_WELCOME
	case WCS_FORESTWIZ_WIZPAGE_CANCEL:
		{
		if (FinalOrderCancelled || ! ProcessMe->Prev || ! ProcessMe->Prev->Prev)
			{
			EndOrderCancel();
			return (NULL);
			} // if cancelled
		ProcessMe = ProcessMe->Prev;
		ProcessMe->Revert();
		return (ProcessMe);
		} // WCS_FORESTWIZ_WIZPAGE_CANCEL
	case WCS_FORESTWIZ_WIZPAGE_CONFIRM:
		{
		if (ReviewOrder)
			{
			ReviewOrder = 0;
			// return to second page (first page with selections to make)
			while (ProcessMe->Prev && ProcessMe->Prev->Prev)
				{
				ProcessMe = ProcessMe->Prev;
				ProcessMe->Revert();
				} // while
			return (ProcessMe);
			} // if
		else
			{
			if (! EndOrderComplete(CurProj))
				{
				CancelOrder = FinalOrderCancelled = 1;
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR]);
				return (ProcessMe->Next);
				} // if
			return (&Wizzes[WCS_FORESTWIZ_WIZPAGE_COMPLETE]);
			} // else
		} // WCS_FORESTWIZ_WIZPAGE_CONFIRM
	case WCS_FORESTWIZ_WIZPAGE_COMPLETE:
		{
		return (NULL);
		} // WCS_FORESTWIZ_WIZPAGE_COMPLETE
	case WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR:
		{
		if (ReviewOrder)
			{
			ReviewOrder = 0;
			CancelOrder = FinalOrderCancelled = 0;
			// return to second page (first page with selections to make)
			while (ProcessMe->Prev && ProcessMe->Prev->Prev)
				{
				ProcessMe = ProcessMe->Prev;
				ProcessMe->Revert();
				} // while
			return (ProcessMe);
			} // if
		else
			{
			EndOrderCancel();
			return (NULL);
			} // else
		} // WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR
	case WCS_FORESTWIZ_WIZPAGE_PAGEERROR:
		{
		return (NULL);
		} // WCS_FORESTWIZ_WIZPAGE_PAGEERROR
	case WCS_FORESTWIZ_WIZPAGE_CMAPORVEC:
		{
		if (CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_CMAP)
			{
			ThematicMapsAllowed = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_ANALYZEIMG]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CLICKORNUM]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_IMGLOADED]);
			} // if
		else if (CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_POLYGONS)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SPSCENARIO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_VECLOADED]);
			} // else
		else
			{
			VegetationExists = 1;
			NumSpeciesFields = 1;
			NumDensFields = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD]);
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_CMAPORVEC
	case WCS_FORESTWIZ_WIZPAGE_IMGLOADED:
		{
		if (ImageLoaded)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELIMG]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_FINDIMG]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_IMGLOADED
	case WCS_FORESTWIZ_WIZPAGE_SELIMG:
		{
		if (! CMapRast)
			{
			return (ProcessMe);
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELIMG
	case WCS_FORESTWIZ_WIZPAGE_FINDIMG:
		{
		if (! (CMapRast = AddCMapImageToLibrary()))
			{
			return (ProcessMe);
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_FINDIMG
	case WCS_FORESTWIZ_WIZPAGE_CLICKORNUM:
		{
		if (ClickOrNumeric == WCS_FORESTWIZ_CLICKORNUMERIC_CLICK)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_CLICKORNUM
	case WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG
	case WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC
	case WCS_FORESTWIZ_WIZPAGE_ANALYZEIMG:
		{
		if (AnalyzeImage)
			{
			if (AnalyzeImageColors())
				{
				if (UnmatchedColors)
					ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_UNIDENTCOLOR]);
				else if ((MatchedCMapColors = CountMatchedColors()) == 0)
					ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES]);
				} // if analysis successful
			else
				{
				CancelOrder = FinalOrderCancelled = 1;
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR]);
				} // else error analyzing image
			} // if
		else
			{
			if ((MatchedCMapColors = CountMatchedColors()) == 0)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES]);
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_ANALYZEIMG
	case WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES:
		{
		return (NULL);
		} // WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES
	case WCS_FORESTWIZ_WIZPAGE_UNIDENTCOLOR:
		{
		if (CMapColorRange)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NORANGES]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_UNIDENTCOLOR
	case WCS_FORESTWIZ_WIZPAGE_NORANGES:
		{
		if ((MatchedCMapColors = CountMatchedColors()) == 0)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_NORANGES
	case WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR:
		{
		if ((MatchedCMapColors = CountMatchedColors()) == 0)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR
	case WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO:
		{
		FoliageBearingUnits = CountFoliageBearingUnits();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO
	case WCS_FORESTWIZ_WIZPAGE_VECLOADED:
		{
		if (! VectorsLoaded)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_LOADVEC]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_VECLOADED
	case WCS_FORESTWIZ_WIZPAGE_LOADVEC:
		{
		VectorsLoaded = 1;
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_LOADVEC
	case WCS_FORESTWIZ_WIZPAGE_SPSCENARIO:
		{
		if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD)
			{
			NumSpeciesFields = 1;
			NumSpDensFields = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD]);
			} // if
		else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD)
			{
			VegetationExists = 1;
			NumSpeciesFields = 2;
			NumSpDensFields = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NUMSIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_NUMDENSFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELSECONDSPFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELFIRSTSPFIELD]);
			} // else
		else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_MULTISPECIESFIELD)
			{
			VegetationExists = 1;
			NumSpeciesFields = 0;
			NumSpDensFields = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELMULTIDENSFIELD]);
			} // else
		else //if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
			{
			VegetationExists = 1;
			NumSpeciesFields = 0;
			NumSpDensFields = 0;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SPDENSPRECISION]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELMULTISPFIELD]);
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_LOADVEC
	case WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG:
		{
		if (VegetationExists)
			{
			if (ThematicMapsAllowed)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_HTDENSATTRIBS]);
			else
				{
				NumSizeFields = 0;
				for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
					SizeAttr[Ct] = NULL;
				NumDensFields = 0;
				for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
					DensAttr[Ct] = NULL;
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD]);
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD]);
				} // if
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG
	case WCS_FORESTWIZ_WIZPAGE_HTDENSATTRIBS:
		{
		if (HDAttribsExist)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD]);
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_HTDENSATTRIBS
	case WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD:
		{
		if (! SpeciesAttr[0])
			return (ProcessMe);
		SetupForestWizClassData();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD:
		{
		if (! SizeAttr[0])
			{
			NumSizeFields = 0;
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				SizeAttr[Ct] = NULL;
			} // if
		else
			{
			NumSizeFields = 1;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD]);
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD:
		{
		if (! DensAttr[0])
			{
			NumDensFields = 0;
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				DensAttr[Ct] = NULL;
			} // if
		else
			{
			NumDensFields = 1;
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD]);
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELFIRSTSPFIELD:
		{
		if (! SpeciesAttr[0])
			return (ProcessMe);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELFIRSTSPFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELSECONDSPFIELD:
		{
		if (! SpeciesAttr[1])
			return (ProcessMe);
		SetupForestWizClassData();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELSECONDSPFIELD
	case WCS_FORESTWIZ_WIZPAGE_NUMSIZEFIELD:
		{
		if (NumSizeFields == 1)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD]);
		else if (NumSizeFields > 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELSECONDSIZEFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELFIRSTSIZEFIELD]);
			} // else
		else
			{
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				SizeAttr[Ct] = NULL;
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_NUMSIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_NUMDENSFIELD:
		{
		if (NumDensFields == 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD]);
			} // if
		else if (NumDensFields > 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELSECONDDENSFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELFIRSTDENSFIELD]);
			} // else
		else
			{
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				DensAttr[Ct] = NULL;
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_NUMDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD:
		{
		if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
			{
			if (ThematicMapsAllowed)
				{
				if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_MULTISPECIESFIELD)
					ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MULTIDBHFIELD]);
				else if (NumDensFields == 1)
					ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDBHFIELD]);
				else //if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD)
					ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_ONEORTWODBHFIELD]);
				} // if
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_BASALAREAUNITS]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_AREAUNITS]);
			} // if
		else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_CLOSUREPRECISION]);
		else // DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_AREAUNITS]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD
	case WCS_FORESTWIZ_WIZPAGE_BASALAREAUNITS:
		{
		ResolveDensityMultiplier();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_BASALAREAUNITS
	case WCS_FORESTWIZ_WIZPAGE_AREAUNITS:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_AREAUNITS
	case WCS_FORESTWIZ_WIZPAGE_ONEORTWODBHFIELD:
		{
		if (NumDBHFields == 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDBHFIELD]);
			} // if
		else if (NumDBHFields > 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELSECONDDBHFIELD]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELFIRSTDBHFIELD]);
			} // else if
		else
			{
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				DBHAttr[Ct] = NULL;
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_ONEORTWODBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_MULTIDBHFIELD:
		{
		if (NumDBHFields == 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONEDBHFIELD]);
			}
		else if (NumDBHFields > 1)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD]);
			} // else if
		else
			{
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				DBHAttr[Ct] = NULL;
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_MULTIDBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELONEDBHFIELD:
		{
		if (DBHAttr[0])
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DBHUNITS]);
		for (Ct = 1; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
			DBHAttr[Ct] = NULL;
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELONEDBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELFIRSTDBHFIELD:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELFIRSTDBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELSECONDDBHFIELD:
		{
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DBHUNITS]);
		if (! DBHAttr[1])
			NumDBHFields = 1;
		for (Ct = 2; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
			DBHAttr[Ct] = NULL;
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELSECONDDBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD:
		{
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DBHUNITS]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD
	case WCS_FORESTWIZ_WIZPAGE_DBHUNITS:
		{
		ResolveDBHMultiplier();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_DBHUNITS
	case WCS_FORESTWIZ_WIZPAGE_CLOSUREPRECISION:
		{
		ResolveDensityMultiplier();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_CLOSUREPRECISION
	case WCS_FORESTWIZ_WIZPAGE_SELFIRSTSIZEFIELD:
		{
		if (! SizeAttr[0])
			return (ProcessMe);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELFIRSTSIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELSECONDSIZEFIELD:
		{
		if (! SizeAttr[1])
			return (ProcessMe);
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELSECONDSIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELFIRSTDENSFIELD:
		{
		if (! DensAttr[0])
			return (ProcessMe);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELFIRSTDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELSECONDDENSFIELD:
		{
		if (! DensAttr[1])
			return (ProcessMe);
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELSECONDDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELMULTIDENSFIELD:
		{
		NumDensFields = 1;
		NumSpeciesFields = CountSpecies(FALSE);
		if (NumSpeciesFields == 0)
			return (ProcessMe);
		SetupForestWizClassData();
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELMULTIDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD:
		{
		if (NumSizeFields == 1)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD]);
		else if (NumSizeFields > 1)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD]);
		else
			{
			for (Ct = 0; Ct < WCS_FORESTWIZ_MAXSPECIES; Ct ++)
				SizeAttr[Ct] = NULL;
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD:
		{
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD
	case WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO
	case WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD:
		{
		if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEAGEEQUIV]);
		else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_DBHMULT]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV]);
			} // if
		else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
			{
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEMULT]);
			ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_SIZEUNITS]);
			} // if
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SIZEMETHOD
	case WCS_FORESTWIZ_WIZPAGE_SIZEUNITS:
		{
		ResolveSizeMultiplier();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SIZEUNITS
	case WCS_FORESTWIZ_WIZPAGE_SIZEAGEEQUIV:
		{
		ResolveSizeMultiplier();
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_HEIGHTTYPE]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SIZEAGEEQUIV
	case WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV:
		{
		ResolveSizeMultiplier();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV
	case WCS_FORESTWIZ_WIZPAGE_DBHMULT:
	case WCS_FORESTWIZ_WIZPAGE_SIZEMULT:
		{
		if (SizeThemeMult <= 0.0)
			return (ProcessMe);
		ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_HEIGHTTYPE]);
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SIZEMULT
	case WCS_FORESTWIZ_WIZPAGE_HEIGHTTYPE:
		{
		if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE)
			{
			if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_AVGHEIGHTTYPE]);
			else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_AVGDBHTYPE]);
			else
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_AVGAGETYPE]);
			} // if
		else if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
			{
			if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXHEIGHTTYPE]);
			else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXDBHTYPE]);
			else
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXAGETYPE]);
			} // else if
		else	// HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXMIN
			{
			if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXMINHEIGHTTYPE]);
			else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXMINDBHTYPE]);
			else
				ProcessMe->AddNext(&Wizzes[WCS_FORESTWIZ_WIZPAGE_MAXMINAGETYPE]);
			} // else
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_HEIGHTTYPE
	case WCS_FORESTWIZ_WIZPAGE_AVGHEIGHTTYPE:
	case WCS_FORESTWIZ_WIZPAGE_AVGDBHTYPE:
	case WCS_FORESTWIZ_WIZPAGE_AVGAGETYPE:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_AVGHEIGHTTYPE
	case WCS_FORESTWIZ_WIZPAGE_MAXHEIGHTTYPE:
	case WCS_FORESTWIZ_WIZPAGE_MAXDBHTYPE:
	case WCS_FORESTWIZ_WIZPAGE_MAXAGETYPE:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_MAXHEIGHTTYPE
	case WCS_FORESTWIZ_WIZPAGE_MAXMINHEIGHTTYPE:
	case WCS_FORESTWIZ_WIZPAGE_MAXMINDBHTYPE:
	case WCS_FORESTWIZ_WIZPAGE_MAXMINAGETYPE:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_MAXMINHEIGHTTYPE
	case WCS_FORESTWIZ_WIZPAGE_SELMULTISPFIELD:
		{
		NumSpeciesFields = CountSpecies(FALSE);
		if (NumSpeciesFields == 0)
			return (ProcessMe);
		SetupForestWizClassData();
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELMULTISPFIELD
	case WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD:
		{
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD
	case WCS_FORESTWIZ_WIZPAGE_SPDENSPRECISION:
		{
		RelativeDensityMult = SPDensPrecision == WCS_FORESTWIZ_PRECISION_PERCENTAGE ? .01: 1.0;
		return (ProcessMe->Next);
		} // WCS_FORESTWIZ_WIZPAGE_SPDENSPRECISION
	default:
		{
		CancelOrder = FinalOrderCancelled = 1;
		return (&Wizzes[WCS_FORESTWIZ_WIZPAGE_PAGEERROR]);
		} // default
	// WCS_FORESTRYWIZARD_ADDPAGE - add a case to handle the page when the page is being processed. If no case exists an error page will be displayed
	} // switch

// Lint points out that we can't ever hit the next line of code
//return (&Wizzes[WCS_FORESTWIZ_WIZPAGE_PAGEERROR]);

} // ForestWiz::ProcessPage

/*===========================================================================*/

int ForestWiz::ConfigurePage(WizardlyPage *ConfigMe, unsigned short ConfigPage)
{

if (ConfigPage < WCS_FORESTWIZ_NUMPAGES)
	{
	ConfigMe->WizPageID = ConfigPage;
	ConfigMe->Text = ForestWizPageText[ConfigPage];
	ConfigMe->WizPageResourceID = ForestWizPageResourceID[ConfigPage];
	return (1);
	} // if

return (0);

} // ForestWiz::ConfigurePage

/*===========================================================================*/

void ForestWiz::EndOrderCancel(void)
{

// clean up any resources used

} // ForestWiz::EndOrderCancel

/*===========================================================================*/

int ForestWiz::EndOrderComplete(Project *CurProj)
{
int Success = 0;

if (CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_CMAP)
	{
	Success = ProcessColorMap();
	} // if
else if (CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_POLYGONS)
	{
	Success = ProcessPolygons(CurProj);
	} // else if
else if (CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_POINTDATA)
	{
	Success = ProcessPointData();
	} // else

return (Success);

} // ForestWiz::EndOrderComplete

/*===========================================================================*/

void ForestWiz::ResolveSizeMultiplier(void)
{

SizeThemeMult = 1.0;

if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
	{
	switch (SizeUnits)
		{
		case WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS:
			{
			SizeThemeMult = .01;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS
		case WCS_FORESTWIZ_SIZEUNITS_FEET:
			{
			SizeThemeMult = .3048;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_FEET
		case WCS_FORESTWIZ_SIZEUNITS_INCHES:
			{
			SizeThemeMult = .0254;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_INCHES
		default:
			{
			SizeThemeMult = 1.0;
			break;
			} // default
		} // switch
	} // if
else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
	{
	// convert supplied diameter into meters
	switch (DBHUnits)
		{
		case WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS:
			{
			SizeThemeMult = .01;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS
		case WCS_FORESTWIZ_SIZEUNITS_FEET:
			{
			SizeThemeMult = .3048;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_FEET
		case WCS_FORESTWIZ_SIZEUNITS_INCHES:
			{
			SizeThemeMult = .0254;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_INCHES
		default:
			{
			SizeThemeMult = 1.0;
			break;
			} // default
		} // switch
	} // else if
if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER || SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
	{
	// convert height equivalents into meters
	switch (SizeUnits)
		{
		case WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS:
			{
			HeightMult = .01;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS
		case WCS_FORESTWIZ_SIZEUNITS_FEET:
			{
			HeightMult = .3048;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_FEET
		case WCS_FORESTWIZ_SIZEUNITS_INCHES:
			{
			HeightMult = .0254;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_INCHES
		default:
			{
			HeightMult = 1.0;
			break;
			} // default
		} // switch
	} // if

} // ForestWiz::ResolveSizeMultiplier

/*===========================================================================*/

void ForestWiz::ResolveDensityMultiplier(void)
{

DensityThemeMult = 1.0;

if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
	{
	switch (BasalAreaUnits)
		{
		case WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS:
			{
			DensityThemeMult = .01 * .01;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS
		case WCS_FORESTWIZ_SIZEUNITS_FEET:
			{
			DensityThemeMult = .3048 * .3048;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_FEET
		case WCS_FORESTWIZ_SIZEUNITS_INCHES:
			{
			DensityThemeMult = .0254 * .0254;
			break;
			} // WCS_FORESTWIZ_SIZEUNITS_INCHES
		default:
			{
			DensityThemeMult = 1.0;
			break;
			} // default
		} // switch
	} // if
else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
	{
	if (ClosurePrecision == WCS_FORESTWIZ_PRECISION_PERCENTAGE)
		{
		DensityThemeMult = .01;
		} // if WCS_FORESTWIZ_PRECISION_PERCENTAGE
	} // if

} // ForestWiz::ResolveDensityMultiplier

/*===========================================================================*/

void ForestWiz::ResolveDBHMultiplier(void)
{

DBHThemeMult = 1.0;

switch (DBHThemeUnits)
	{
	case WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS:
		{
		DBHThemeMult = .01;
		break;
		} // WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS
	case WCS_FORESTWIZ_SIZEUNITS_FEET:
		{
		DBHThemeMult = .3048;
		break;
		} // WCS_FORESTWIZ_SIZEUNITS_FEET
	case WCS_FORESTWIZ_SIZEUNITS_INCHES:
		{
		DBHThemeMult = .0254;
		break;
		} // WCS_FORESTWIZ_SIZEUNITS_INCHES
	default:
		{
		DBHThemeMult = 1.0;
		break;
		} // default
	} // switch

} // ForestWiz::ResolveDBHMultiplier

/*===========================================================================*/

int ForestWiz::AnalyzeImageColors(void)
{
long CurRow, CurCol, UnitNumber;
int Abort = 0, Found, Named;
unsigned char SampleRGB[3];

UnmatchedColors = 0;
MaxUnitsExceeded = 0;

for (CurRow = 0; CurRow < CMapRast->Rows && ! Abort; CurRow ++)
	{
	for (CurCol = 0; CurCol < CMapRast->Cols && ! Abort; CurCol ++)
		{
		if (CMapRast->SampleByteCell3(SampleRGB, CurCol, CurRow, Abort))
			{
			Found = Named = 0;
			for (UnitNumber = 0; UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS; UnitNumber ++)
				{
				if (CMapMatchColors[UnitNumber][0] || 
					CMapMatchColors[UnitNumber][1] ||
					CMapMatchColors[UnitNumber][2])
					{
					if (SampleRGB[0] == CMapMatchColors[UnitNumber][0] && 
						SampleRGB[1] == CMapMatchColors[UnitNumber][1] &&
						SampleRGB[2] == CMapMatchColors[UnitNumber][2])
						{
						Found = 1;
						if (CMapMatchNames[UnitNumber][0])
							Named = 1;
						break;
						} // if
					} // if
				else
					break;
				} // for
			if (! Named)
				{
				UnmatchedColors ++;
				if (! Found)
					{
					if (UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS) 
						{
						CMapMatchColors[UnitNumber][0] = SampleRGB[0];
						CMapMatchColors[UnitNumber][1] = SampleRGB[1];
						CMapMatchColors[UnitNumber][2] = SampleRGB[2];
						} // if
					else
						{
						MaxUnitsExceeded = 1;
						} // else
					} // if
				} // if
			} // if 
		} // for
	} // for

return (! Abort);

} // ForestWiz::AnalyzeImageColors

/*===========================================================================*/

long ForestWiz::CountMatchedColors(void)
{
long UnitNumber, ValidUnits = 0;

for (UnitNumber = 0; UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS; UnitNumber ++)
	{
	if (ValidateCMapUnit(UnitNumber))
		ValidUnits ++;
	} // for

return (ValidUnits);

} // ForestWiz::CountMatchedColors

/*===========================================================================*/

long ForestWiz::CountFoliageBearingUnits(void)
{
long UnitNumber, ValidUnits = 0;

if (EcoUnits)
	{
	for (UnitNumber = 0; UnitNumber < NumEcoUnits; UnitNumber ++)
		{
		if (EcoUnits[UnitNumber].HasFoliage && EcoUnits[UnitNumber].FolNames[0].GetName()[0])
			ValidUnits ++;
		} // for
	} // if

return (ValidUnits);

} // ForestWiz::CountFoliageBearingUnits

/*===========================================================================*/

Raster *ForestWiz::AddCMapImageToLibrary(void)
{
Raster *AddedRast = NULL;
NotifyTag Changes[2];

if (CMapImage.GetName()[0])
	{
#ifdef WCS_IMAGE_MANAGEMENT
	if (AddedRast = GlobalApp->AppImages->AddRaster((char *)CMapImage.GetPath(), (char *)CMapImage.GetName(), TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/,
		TRUE /*LoadnPrep*/, TRUE /*AllowImageManagement*/))
#else // !WCS_IMAGE_MANAGEMENT
	if (AddedRast = GlobalApp->AppImages->AddRaster((char *)CMapImage.GetPath(), (char *)CMapImage.GetName(), TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/,
		TRUE /*LoadnPrep*/, FALSE /*AllowImageManagement*/))
#endif // !WCS_IMAGE_MANAGEMENT
		{
		// notify of image added
		Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, AddedRast);

		// see if there is a georeference tag
		if (! AddedRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
			{
			AddedRast->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF, NULL, NULL);
			Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, AddedRast);
			if (UserMessageYN("Forestry Wizard", "The selected image must be georeferenced. Do you wish to set the georeference information now? If so, answer \"Yes\" and go to the GeoReference tab when the Image Object Library opens."))
				{
				AddedRast->EditRAHost();
				} // if
			} // if
		} // if
	} // if

return (AddedRast);

} // ForestWiz::AddCMapImageToLibrary

/*===========================================================================*/

long ForestWiz::CMapAddColor(long UnitNumber, char *CMapMatchName, unsigned char *CMapMatchRGB)
{
long UnitCt;

if (CMapColorResponseEnabled && UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS)
	{
	// check that the color hasn't already been used
	for (UnitCt = 0; UnitCt < UnitNumber; UnitCt ++)
		{
		if (! stricmp(CMapMatchNames[UnitCt], CMapMatchName))
			{
			CMapMatchColors[UnitCt][0] = CMapMatchRGB[0];
			CMapMatchColors[UnitCt][1] = CMapMatchRGB[1];
			CMapMatchColors[UnitCt][2] = CMapMatchRGB[2];
			return (ValidateCMapUnit(UnitCt));
			} // if name matched
		if (CMapMatchColors[UnitCt][0] == CMapMatchRGB[0] && CMapMatchColors[UnitCt][1] == CMapMatchRGB[1] && CMapMatchColors[UnitCt][2] == CMapMatchRGB[2])
			return (0);
		} // for
	strcpy(CMapMatchNames[UnitNumber], CMapMatchName);
	CMapMatchColors[UnitNumber][0] = CMapMatchRGB[0];
	CMapMatchColors[UnitNumber][1] = CMapMatchRGB[1];
	CMapMatchColors[UnitNumber][2] = CMapMatchRGB[2];

	return (ValidateCMapUnit(UnitNumber));
	} // if

return (0);

} // ForestWiz::CMapAddColor

/*===========================================================================*/

long ForestWiz::CMapFetchColor(long UnitNumber, char *CMapMatchName, unsigned char *CMapMatchRGB)
{

if (UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS)
	{
	strcpy(CMapMatchName, CMapMatchNames[UnitNumber]);
	CMapMatchRGB[0] = CMapMatchColors[UnitNumber][0];
	CMapMatchRGB[1] = CMapMatchColors[UnitNumber][1];
	CMapMatchRGB[2] = CMapMatchColors[UnitNumber][2];

	return (ValidateCMapUnit(UnitNumber));
	} // if

return (0);

} // ForestWiz::CMapFetchColor

/*===========================================================================*/

long ForestWiz::CountUnmatchedColors(void)
{
long UnitNumber, InValidUnits = 0;

for (UnitNumber = 0; UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS; UnitNumber ++)
	{
	if (! CMapMatchNames[UnitNumber][0] && (CMapMatchColors[UnitNumber][0] || CMapMatchColors[UnitNumber][1] || CMapMatchColors[UnitNumber][2]))
		InValidUnits ++;
	} // for

return (InValidUnits);

} // ForestWiz::CountUnmatchedColors

/*===========================================================================*/

ForestWizEcoData *ForestWiz::SetupForestWizEcoData(void)
{

if (EcoUnits)
	{
	if (NumEcoUnits != MatchedCMapColors)
		{
		delete [] EcoUnits;
		EcoUnits = NULL;
		NumEcoUnits = 0;
		} // if
	} // if

if (! EcoUnits)
	{
	if (EcoUnits = new ForestWizEcoData[MatchedCMapColors])
		{
		NumEcoUnits = MatchedCMapColors;
		} // if
	} // if

return (EcoUnits);

} // ForestWiz::SetupForestWizEcoData

/*===========================================================================*/

int ForestWiz::SetEcoData(long EcoNumber, ForestWizEcoData *CopyFrom)
{

if (EcoUnits && EcoNumber < NumEcoUnits)
	{
	EcoUnits[EcoNumber].Copy(CopyFrom);
	return (1);
	} // if

return (0);

} // ForestWiz::SetEcoData

/*===========================================================================*/

int ForestWiz::FetchEcoData(long EcoNumber, ForestWizEcoData *CopyTo)
{

if (EcoUnits && EcoNumber < NumEcoUnits)
	{
	CopyTo->Copy(&EcoUnits[EcoNumber]);
	return (1);
	} // if

return (0);

} // ForestWiz::FetchEcoData

/*===========================================================================*/

int ForestWiz::FetchEcoName(long UnitNumber, char *EcoName)
{

if (UnitNumber < MatchedCMapColors)
	{
	strcpy(EcoName, CMapMatchNames[UnitNumber]);
	return (1);
	} // if

return (0);

} // ForestWiz::FetchEcoName

/*===========================================================================*/

int ForestWiz::ProcessColorMap(void)
{
double MaxFolHt;
long EcoCt, UnitNumber, FolCt, NumFoliage;
EcosystemEffect *CurEco, *MadeEco;
GradientCritter *ActiveGrad;
MaterialEffect *EcoMat;
Ecotype *CurEcotp;
FoliageGroup *CurGrp;
Foliage *CurFol;
Raster *CurRast;
CmapEffect *CurCmap;
int Success = 1, Queried = 0, QueryResult = 0, Abort = 0, CMapQueried = 0, CMapQueryResult = 0, 
	SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0;
NotifyTag Changes[2];

if (! CMapRast)
	return (0);

Changes[1] = NULL;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

// for each eco unit, create an ecosystem and set it to cmap match
for (UnitNumber = EcoCt = 0; Success && UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS; UnitNumber ++)
	{
	if (ValidateCMapUnit(UnitNumber))
		{
		MadeEco = NULL;
		// add ecosystem
		// look for ecosystem already existing first
		CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, CMapMatchNames[UnitNumber], Queried, QueryResult, Abort, FALSE);
		if (Abort)
			{
			Success = 0;
			goto ThawSceneView;
			} // if
		if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
			{
			// set cmap match, colors
			CurEco->Enabled = 1;
			CurEco->CmapMatch = 1;
			CurEco->CmapMatchRange = 0;
			CurEco->MatchColor[0] = CMapMatchColors[UnitNumber][0];
			CurEco->MatchColor[1] = CMapMatchColors[UnitNumber][1];
			CurEco->MatchColor[2] = CMapMatchColors[UnitNumber][2];
			CurEco->MatchColor[3] = CurEco->MatchColor[4] = CurEco->MatchColor[5] = 0;
			if (MadeEco)
				{
				// change name
				CurEco->SetUniqueName(GlobalApp->AppEffects, CMapMatchNames[UnitNumber]);
				// copy modified name for use in color map
				strcpy(CMapMatchNames[UnitNumber], CurEco->GetName());
				// load material texture
				if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
					{
					if (! SetEcoProperties(EcoMat, EcoUnits[EcoCt].GroundTexture))
						{
						Success = 0;
						break;
						} // if
					if (EcoUnits[EcoCt].HasFoliage)
						{
						// add ecotype
						if (CurEcotp = EcoMat->NewEcotype(0))
							{
							CurEcotp->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
							CurEcotp->ConstDensity = 1;
							CurEcotp->DensityUnits = EcoAreaUnits;
							CurEcotp->AbsHeightResident = CurEcotp->AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;
							CurEcotp->SetAnimDefaults();

							// add fol grp
							if (CurGrp = CurEcotp->AddFoliageGroup(NULL, CMapMatchNames[UnitNumber]))
								{
								// find max fol height
								MaxFolHt = 0.0;
								NumFoliage = 0;
								for (FolCt = 0; FolCt < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE; FolCt ++)
									{
									if (EcoUnits[EcoCt].FolNames[FolCt].GetName()[0])
										{
										if (EcoUnits[EcoCt].FolHt[FolCt] > MaxFolHt)
											MaxFolHt = EcoUnits[EcoCt].FolHt[FolCt];
										NumFoliage += EcoUnits[EcoCt].FolCount[FolCt];
										} // if
									} // for
								MaxFolHt *= HeightMult;
								AvgHeightRange *= HeightMult;
								MinHeightRange *= HeightMult;
								MinHeightAbs *= HeightMult;
								CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
								CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
								CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].SetValue((double)NumFoliage);
								// set thematic maps for height and density
								// density
								if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
									{
									CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
									} // if
								else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
									{
									CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
									} // else if
								else // (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
									{
									CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
									} // else

								// size
								if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
									{
									CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
									// set up DBH curve
									SetupDBHCurve(&CurEcotp->DBHCurve);
									} // if
								else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
									{
									CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
									// set up Age curve
									SetupAgeCurve(&CurEcotp->AgeCurve);
									} // else if
								else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
									{
									CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
									} // else

								CurEcotp->SetAnimDefaults();

								// so we don't divide by 0
								if (MaxFolHt < .0001)
									MaxFolHt = .0001;
								if (NumFoliage == 0)
									NumFoliage = 1;
								for (FolCt = 0; FolCt < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE; FolCt ++)
									{
									if (EcoUnits[EcoCt].FolNames[FolCt].GetName()[0])
										{
										if (CurFol = CurGrp->AddFoliage(NULL))
											{
											CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)EcoUnits[EcoCt].FolCount[FolCt] / NumFoliage);
											CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(EcoUnits[EcoCt].FolHt[FolCt] / MaxFolHt);
											// add foliage images to library
											if (CurRast = GlobalApp->AppImages->AddRaster((char *)EcoUnits[EcoCt].FolNames[FolCt].GetPath(), (char *)EcoUnits[EcoCt].FolNames[FolCt].GetName(), 
												TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/, TRUE /*LoadnPrep*/, FALSE /*AllowImageManagement*/))
												{
												// notify of image added
												Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
												Changes[1] = NULL;
												GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
												// add foliage to  ecotype
												CurFol->SetRaster(CurRast);
												} // if image added
											} // if
										else
											{
											Success = 0;
											break;
											} // else
										} // if
									else
										break;
									} // for
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						else
							{
							Success = 0;
							break;
							} // else
						} // if
					} // if
				} // if
			} // if
		else
			{
			Success = 0;
			break;
			} // else
		EcoCt ++;
		} // if
	} // for

// create a cmap and add all the eco units to it
if (Success)
	{
	CurCmap = (CmapEffect *)FindComponent(WCS_EFFECTSSUBCLASS_CMAP, CMapRast->GetUserName(), CMapQueried, CMapQueryResult, Abort, TRUE);
	if (Abort)
		{
		Success = 0;
		goto ThawSceneView;
		} // if
	if (CurCmap = new CmapEffect(NULL, GlobalApp->AppEffects, NULL))
		{
		Changes[0] = MAKE_ID(CurCmap->GetNotifyClass(), CurCmap->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, CurCmap);
		CurCmap->SetUniqueName(GlobalApp->AppEffects, CMapRast->GetUserName());
		CurCmap->SetRaster(CMapRast);
		CurCmap->EvalByPixel = WCS_CMAP_EVAL_BYPOLYGON;
		for (UnitNumber = EcoCt = 0; UnitNumber < WCS_FORESTWIZ_MAXNUMUNITS; UnitNumber ++)
			{
			if (ValidateCMapUnit(UnitNumber))
				{
				if (CurEco = (EcosystemEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, CMapMatchNames[UnitNumber]))
					CurCmap->AddEcosystem(CurEco);
				} // if
			} // for
		} // if
	else
		Success = 0;
	} // if

ThawSceneView:

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

return (Success);

} // ForestWiz::ProcessColorMap

/*===========================================================================*/

int ForestWiz::SetEcoProperties(MaterialEffect *EcoMat, long GroundTex)
{
char *CompName = NULL;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char CompFileName[512];

switch (GroundTex)
	{
	case WCS_FORESTWIZECODATA_GROUNDTEX_RANDOM:
		{
		// color unchanged
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_GRAY:
		{
		EcoMat->DiffuseColor.SetValue3(.75, .75, .75);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_TAN:
		{
		EcoMat->DiffuseColor.SetValue3(.8, .75, .5);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_GREEN:
		{
		EcoMat->DiffuseColor.SetValue3(.5, .8, .3);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_RED:
		{
		EcoMat->DiffuseColor.SetValue3(.9, .4, .3);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_YELLOW:
		{
		EcoMat->DiffuseColor.SetValue3(.9, .9, .3);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_BLUE:
		{
		EcoMat->DiffuseColor.SetValue3(.3, .4, .9);
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_WATER:
		{
		EcoMat->DiffuseColor.SetValue3(153 / 255.0, 170 / 255.0, 214 / 255.0);
		CompName = "WizWater.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_EARTH:
		{
		EcoMat->DiffuseColor.SetValue3(220 / 255.0, 220 / 255.0, 208 / 255.0);
		CompName = "WizEarth.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_ROCK:
		{
		EcoMat->DiffuseColor.SetValue3(183 / 255.0, 176 / 255.0, 169 / 255.0);
		CompName = "WizRock.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_BOULDER:
		{
		EcoMat->DiffuseColor.SetValue3(183 / 255.0, 176 / 255.0, 169 / 255.0);
		CompName = "WizBoulders.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_GRAVEL:
		{
		EcoMat->DiffuseColor.SetValue3(196 / 255.0, 190 / 255.0, 185 / 255.0);
		CompName = "WizGravel.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_DRYGRASS:
		{
		EcoMat->DiffuseColor.SetValue3(221 / 255.0, 217 / 255.0, 168 / 255.0);
		CompName = "WizDryGrass.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_LAWNGRASS:
		{
		EcoMat->DiffuseColor.SetValue3(161 / 255.0, 201 / 255.0, 118 / 255.0);
		CompName = "WizLawnGrass.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_DECIDUOUSFOREST:
		{
		EcoMat->DiffuseColor.SetValue3(183 / 255.0, 182 / 255.0, 114 / 255.0);
		CompName = "WizDeciduousForest.mat";
		break;
		}
	case WCS_FORESTWIZECODATA_GROUNDTEX_CONIFERFOREST:
		{
		EcoMat->DiffuseColor.SetValue3(158 / 255.0, 156 / 255.0, 137 / 255.0);
		CompName = "WizConiferForest.mat";
		break;
		}
	default:
		break;
	} // switch
if (CompName)
	{
	strmfp(CompFileName, "WCSContent:3DObject", CompName);
	Prop.Path = CompFileName;
	Prop.Queries = 0;
	Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
	if (EcoMat->LoadFilePrep(&Prop) > 0)
		{
		Changes[0] = MAKE_ID(EcoMat->GetNotifyClass(), EcoMat->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, EcoMat->GetRAHostRoot());
		} // if
	else
		{
		if (! UserMessageYN(CompFileName, "File not found. Ecoystem texture can not be loaded. Proceed without texture?"))
			return (0);
		} // else
	} // if

return (1);

} // ForestWiz::SetEcoProperties

/*===========================================================================*/

long ForestWiz::CountSpecies(int ApplyNames)
{
long CurItem, NameCt, Duplicate, MaxNumClasses, AttrCt, NumClasses = 0;
Joe *VecWalk;
LayerStub *Stubby;
char *TempNames;
const char *LayerName, *LayerName2;
char AttrStr[256];

MaxNumClasses = ApplyNames ? NumClassUnits: WCS_FORESTWIZ_MAXSPECIES;

if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD)
	{
	if (SpeciesAttr[0])
		{
		if (TempNames = (char *)AppMem_Alloc(MaxNumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN, APPMEM_CLEAR))
			{
			// look at each vector that has an attribute for species
			LayerName = SpeciesAttr[0]->GetName();
			for (VecWalk = GlobalApp->AppDB->GetFirst(); VecWalk && NumClasses < MaxNumClasses; VecWalk = GlobalApp->AppDB->GetNext(VecWalk))
				{
				if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					if (Stubby = VecWalk->MatchEntryToStub(SpeciesAttr[0]))
						{
						if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
							strcpy(AttrStr, Stubby->GetTextAttribVal() ? Stubby->GetTextAttribVal(): "");
						else if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
							sprintf(AttrStr, "%g", Stubby->GetIEEEAttribVal());
						else
							continue;
						if (AttrStr[0])
							{
							Duplicate = 0;
							for (NameCt = 0; NameCt < NumClasses; NameCt ++)
								{
								if (! strcmp(&TempNames[NameCt * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr))
									{
									Duplicate = 1;
									break;
									} // if
								} // for
							if (! Duplicate)
								{
								if (ApplyNames)
									{
									strcpy(ClassUnits[NumClasses].PolyName, AttrStr);
									ClassUnits[NumClasses].OverUnder = 0;
									} // if
								strcpy(&TempNames[NumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr);
								NumClasses ++;
								} // if
							} // if
						} // if
					} // if not a group vector
				} // for
			AppMem_Free(TempNames);
			} // if
		} // if
	} // if
else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD)
	{
	// look at each vector that has an attribute for species
	if (SpeciesAttr[0] && SpeciesAttr[1])
		{
		if (TempNames = (char *)AppMem_Alloc(MaxNumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN, APPMEM_CLEAR))
			{
			// look at each vector that has an attribute for species
			LayerName = SpeciesAttr[0]->GetName();
			LayerName2 = SpeciesAttr[1]->GetName();
			for (VecWalk = GlobalApp->AppDB->GetFirst(); VecWalk && NumClasses < MaxNumClasses; VecWalk = GlobalApp->AppDB->GetNext(VecWalk))
				{
				if (Stubby = VecWalk->MatchEntryToStub(SpeciesAttr[0]))
					{
					if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
						strcpy(AttrStr, Stubby->GetTextAttribVal() ? Stubby->GetTextAttribVal(): "");
					else if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
						sprintf(AttrStr, "%g", Stubby->GetIEEEAttribVal());
					else
						continue;
					Duplicate = 0;
					for (NameCt = 0; NameCt < NumClasses; NameCt ++)
						{
						if (! strcmp(&TempNames[NameCt * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr))
							{
							Duplicate = 1;
							if (ApplyNames)
								ClassUnits[NameCt].OverUnder |= WCS_FORESTWIZ_OVERUNDER_OVERSTORY;
							break;
							} // if
						} // for
					if (! Duplicate)
						{
						if (ApplyNames)
							{
							strcpy(ClassUnits[NumClasses].PolyName, AttrStr);
							ClassUnits[NumClasses].OverUnder = WCS_FORESTWIZ_OVERUNDER_OVERSTORY;
							} // if
						strcpy(&TempNames[NumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr);
						NumClasses ++;
						} // if
					} // if
				if (NumClasses >= MaxNumClasses)
					break;
				if (Stubby = VecWalk->MatchEntryToStub(SpeciesAttr[1]))
					{
					if (LayerName2[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
						strcpy(AttrStr, Stubby->GetTextAttribVal() ? Stubby->GetTextAttribVal(): "");
					else if (LayerName2[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
						sprintf(AttrStr, "%g", Stubby->GetIEEEAttribVal());
					else
						continue;
					Duplicate = 0;
					for (NameCt = 0; NameCt < NumClasses; NameCt ++)
						{
						if (! strcmp(&TempNames[NameCt * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr))
							{
							Duplicate = 1;
							if (ApplyNames)
								ClassUnits[NameCt].OverUnder |= WCS_FORESTWIZ_OVERUNDER_UNDERSTORY;
							break;
							} // if
						} // for
					if (! Duplicate)
						{
						if (ApplyNames)
							{
							strcpy(ClassUnits[NumClasses].PolyName, AttrStr);
							ClassUnits[NumClasses].OverUnder = WCS_FORESTWIZ_OVERUNDER_UNDERSTORY;
							} // if
						strcpy(&TempNames[NumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr);
						NumClasses ++;
						} // if
					} // if
				} // for
			AppMem_Free(TempNames);
			} // if
		} // if
	} // else
else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	{
	NumSpDensFields = 0;
	while (SpeciesAttr[NumSpDensFields])
		{
		NumSpDensFields ++;
		} // while
	if (TempNames = (char *)AppMem_Alloc(MaxNumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN, APPMEM_CLEAR))
		{
		// look at each vector that has an attribute for species
		for (VecWalk = GlobalApp->AppDB->GetFirst(); VecWalk && NumClasses < MaxNumClasses; VecWalk = GlobalApp->AppDB->GetNext(VecWalk))
			{
			AttrCt = 0;
			while (SpeciesAttr[AttrCt])
				{
				LayerName = SpeciesAttr[AttrCt]->GetName();
				if (Stubby = VecWalk->MatchEntryToStub(SpeciesAttr[AttrCt]))
					{
					if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
						strcpy(AttrStr, Stubby->GetTextAttribVal() ? Stubby->GetTextAttribVal(): "");
					else if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
						sprintf(AttrStr, "%g", Stubby->GetIEEEAttribVal());
					else
						continue;
					if (AttrStr[0])
						{
						Duplicate = 0;
						for (NameCt = 0; NameCt < NumClasses; NameCt ++)
							{
							if (! strcmp(&TempNames[NameCt * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr))
								{
								Duplicate = 1;
								break;
								} // if
							} // for
						if (! Duplicate)
							{
							if (ApplyNames)
								{
								strcpy(ClassUnits[NumClasses].PolyName, AttrStr);
								ClassUnits[NumClasses].OverUnder = WCS_FORESTWIZ_OVERUNDER_OVERSTORY;
								} // if
							strcpy(&TempNames[NumClasses * WCS_FORESTWIZ_MAXUNITNAMELEN], AttrStr);
							NumClasses ++;
							} // if
						} // if
					} // if
				if (NumClasses >= MaxNumClasses)
					break;
				AttrCt ++;
				} // while
			} // for
		AppMem_Free(TempNames);
		} // if
	} // else if
else
	{
	for (CurItem = 0; CurItem < MaxNumClasses; CurItem ++)
		{
		if (SpeciesAttr[CurItem])
			{
			if (ApplyNames)
				{
				strcpy(ClassUnits[NumClasses].PolyName, &SpeciesAttr[CurItem]->GetName()[1]);
				ClassUnits[NumClasses].OverUnder = 0;
				} // if
			NumClasses ++;
			} // if
		} // for
	} // else

return (NumClasses);

} // ForestWiz::CountSpecies

/*===========================================================================*/

ForestWizClassData *ForestWiz::SetupForestWizClassData(void)
{
long NewNumClasses;

NewNumClasses = CountSpecies(FALSE);

if (ClassUnits && NewNumClasses != NumClassUnits)
	{
	delete [] ClassUnits;
	ClassUnits = NULL;
	NumClassUnits = 0;
	} // if

if (NewNumClasses && ! ClassUnits)
	{
	if (ClassUnits = new ForestWizClassData[NewNumClasses])
		{
		NumClassUnits = NewNumClasses;
		FillSpeciesNames();
		} // if
	} // if

return (ClassUnits);

} // ForestWiz::SetupForestWizClassData

/*===========================================================================*/

int ForestWiz::FillSpeciesNames(void)
{

if (ClassUnits)
	{
	if (CountSpecies(TRUE))
		return (1);
	} // if

return (0);

} // ForestWiz::FillSpeciesNames

/*===========================================================================*/

int ForestWiz::SetClassData(long ClassNumber, ForestWizClassData *CopyFrom)
{

if (ClassUnits && ClassNumber < NumClassUnits)
	{
	ClassUnits[ClassNumber].Copy(CopyFrom, VegetationExists);
	return (1);
	} // if

return (0);

} // ForestWiz::SetClassData

/*===========================================================================*/

int ForestWiz::FetchClassData(long ClassNumber, ForestWizClassData *CopyTo)
{

if (ClassUnits && ClassNumber < NumClassUnits)
	{
	CopyTo->Copy(&ClassUnits[ClassNumber], VegetationExists);
	return (1);
	} // if

return (0);

} // ForestWiz::FetchClassData

/*===========================================================================*/

int ForestWiz::FetchSpDensData(long AttrCt, ForestWizClassData *CopyTo)
{
const char *LayerName;

if (AttrCt < NumSpDensFields)
	{
	// set only the name of the attribute field
	LayerName = SpeciesAttr[AttrCt]->GetName();
	strcpy(CopyTo->PolyName, &LayerName[1]);
	return (1);
	} // if

return (0);

} // ForestWiz::FetchSpDensData

/*===========================================================================*/

int ForestWiz::ProcessPolygons(Project *CurProj)
{
int Success = 1;
NotifyTag Changes[2];

Changes[1] = NULL;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD)
	{
	Success = ProcessOneSpeciesField();
	} // if WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD
else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD)
	{
	Success = ProcessTwoSpeciesFields();
	} // else if WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD
else if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_MULTISPECIESFIELD)
	{
	Success = ProcessMultiSpeciesFields();
	} // else WCS_FORESTWIZ_SPSCENARIO_MULTISPECIESFIELD
else // if (SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	{
	Success = ProcessFARSITESpeciesFields();
	} // else WCS_FORESTWIZ_SPSCENARIO_FARSITE

if (Success)
	GlobalApp->AppEffects->EcosystemBase.SetFloating(TRUE, CurProj);		// this sends the valuechanged message

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

return (Success);

} // ForestWiz::ProcessPolygons

/*===========================================================================*/

int ForestWiz::ProcessOneSpeciesField(void)
{
double MaxFolHt;
long UnitNumber, FolCt, NumFoliage;
EcosystemEffect *CurEco, *MadeEco;
GradientCritter *ActiveGrad;
MaterialEffect *EcoMat;
Ecotype *CurEcotp;
FoliageGroup *CurGrp;
Foliage *CurFol;
Raster *CurRast;
SearchQuery *CurSQ, *MadeSQ;
ThematicMap *CurDensTheme = NULL, *CurHeightTheme = NULL, *CurMinHeightTheme = NULL, *CurDBHTheme = NULL, *MadeTheme;
DBFilterEvent *CurFilter;
int Success = 1, Queried = 0, QueryResult = 0, SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0, Abort = 0;
NotifyTag Changes[2];
char ThemeName[128];

// create an ecosystem for each class with an overstory ecotype
// populate with images
// create a search query for each ecosystem that links the ecosystem to the vectors via the species attr field
// create one thematic map for density and one for height, 
// link TM's to all vectors with height or density attribs and their values > 0
// attach TM's to overstory ecotype heights and densities

// create the density TM
// create the height TM
// create the TM Search Query
// look for TM's before creating them
if (DensAttr[0] && NumDensFields > 0)
	{
	MadeTheme = NULL;
	if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
		strcpy(ThemeName, "Ecosystem Foliage Density");
	else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
		strcpy(ThemeName, "Ecosystem Foliage Basal Area");
	else
		strcpy(ThemeName, "Ecosystem Foliage Crown Closure");
	CurDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurDensTheme || (CurDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			CurDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		CurDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		CurDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		CurDensTheme->NullConstant[0] = 0.0;
		CurDensTheme->AttribFactor[0] = DensityThemeMult;
		CurDensTheme->SetAttribute(DensAttr[0], 0);
		CurDensTheme->SetAttribute(NULL, 1);
		CurDensTheme->SetAttribute(NULL, 2);
		// create DBH theme if needed
		if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && DBHAttr[0])
			{
			MadeTheme = NULL;
			strcpy(ThemeName, "Ecosystem Foliage Diameter");
			CurDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
			if (Abort)
				{
				return (0);
				} // if
			if (CurDBHTheme || (CurDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
				{
				if (MadeTheme)
					{
					// change name
					CurDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
					} // if
				CurDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
				CurDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
				CurDBHTheme->NullConstant[0] = 1.0;
				CurDBHTheme->AttribFactor[0] = DBHThemeMult;
				CurDBHTheme->SetAttribute(DBHAttr[0], 0);
				CurDBHTheme->SetAttribute(NULL, 1);
				CurDBHTheme->SetAttribute(NULL, 2);
				} // if
			} // if
		} // if
	} // if
if (SizeAttr[0] && NumSizeFields > 0)
	{
	MadeTheme = NULL;
	if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
		strcpy(ThemeName, "Ecosystem Foliage Height");
	else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
		strcpy(ThemeName, "Ecosystem Foliage Diameter");
	else
		strcpy(ThemeName, "Ecosystem Foliage Age");
	CurHeightTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurHeightTheme || (CurHeightTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			CurHeightTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		CurHeightTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		CurHeightTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		CurHeightTheme->NullConstant[0] = 0.0;
		CurHeightTheme->SetAttribute(SizeAttr[0], 0);
		CurHeightTheme->SetAttribute(NULL, 1);
		CurHeightTheme->SetAttribute(NULL, 2);
		CurHeightTheme->AttribFactor[0] = SizeThemeMult;
		} // if
	} // if

if (SizeAttr[2] && MinSizeThemePresent)
	{
	MadeTheme = NULL;
	if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
		strcpy(ThemeName, "Ecosystem Foliage Minimum Height");
	else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
		strcpy(ThemeName, "Ecosystem Foliage Minimum Diameter");
	else
		strcpy(ThemeName, "Ecosystem Foliage Minimum Age");
	CurMinHeightTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurMinHeightTheme || (CurMinHeightTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			CurMinHeightTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		CurMinHeightTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		CurMinHeightTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		CurMinHeightTheme->NullConstant[0] = 0.0;
		CurMinHeightTheme->SetAttribute(SizeAttr[2], 0);
		CurMinHeightTheme->SetAttribute(NULL, 1);
		CurMinHeightTheme->SetAttribute(NULL, 2);
		if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
			CurMinHeightTheme->AttribFactor[0] = .01;
		else
			CurMinHeightTheme->AttribFactor[0] = SizeThemeMult;
		} // if
	} // if
	
for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
	{
	MadeEco = NULL;
	// add ecosystem
	// look for ecosystem already existing first
	CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, ClassUnits[UnitNumber].PolyName, Queried, QueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
		{
		if (MadeEco)
			{
			// change name
			CurEco->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
			// load material texture
			if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
				{
				if (! SetEcoProperties(EcoMat, ClassUnits[UnitNumber].GroundTexture))
					{
					Success = 0;
					break;
					} // if
				if (ClassUnits[UnitNumber].HasFoliage)
					{
					// add ecotype
					if (CurEcotp = EcoMat->NewEcotype(0))
						{
						CurEcotp->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
						CurEcotp->ConstDensity = 1;
						CurEcotp->DensityUnits = EcoAreaUnits;
						CurEcotp->AbsHeightResident = CurEcotp->AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;
						// set thematic maps for height and density
						// density
						if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
							{
							CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_DENSITY, CurDensTheme);
							} // if
						else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
							{
							CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_BASALAREA, CurDensTheme);
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_DBH, CurDBHTheme);
							} // else if
						else // (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
							{
							CurEcotp->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_CROWNCLOSURE, CurDensTheme);
							} // else

						// size
						if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
							{
							CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_DBH, CurHeightTheme);
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
							// set up DBH curve
							SetupDBHCurve(&CurEcotp->DBHCurve);
							} // if
						else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
							{
							CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_AGE, CurHeightTheme);
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
							// set up Age curve
							SetupAgeCurve(&CurEcotp->AgeCurve);
							} // else if
						else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
							{
							CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_HEIGHT, CurHeightTheme);
							CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
							} // else

						CurEcotp->SetAnimDefaults();

						// add fol grp
						if (CurGrp = CurEcotp->AddFoliageGroup(NULL, ClassUnits[UnitNumber].PolyName))
							{
							// find max fol height
							MaxFolHt = 0.0;
							NumFoliage = 0;
							for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
								{
								if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
									{
									if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxFolHt)
										MaxFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
									NumFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
									} // if
								} // for
							CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
							CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
							CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].SetValue((double)NumFoliage);

							// so we don't divide by 0
							if (MaxFolHt < .0001)
								MaxFolHt = .0001;
							if (NumFoliage == 0)
								NumFoliage = 1;
							for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
								{
								if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
									{
									if (CurFol = CurGrp->AddFoliage(NULL))
										{
										CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumFoliage);
										CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxFolHt);
										// add foliage images to library
										if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName(), 
											TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/, TRUE /*LoadnPrep*/, FALSE /*AllowImageManagement*/))
											{
											// notify of image added
											Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
											Changes[1] = NULL;
											GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
											// add foliage to  ecotype
											CurFol->SetRaster(CurRast);
											} // if image added
										} // if
									else
										{
										Success = 0;
										break;
										} // else
									} // if
								else
									break;
								} // for
							} // if
						else
							{
							Success = 0;
							break;
							} // else
						} // if
					else
						{
						Success = 0;
						break;
						} // else
					} // if
				} // if
			} // if
		MadeSQ = NULL;
		// add search query
		// look for search query already existing first
		CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ClassUnits[UnitNumber].PolyName, SQQueried, SQQueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
			{
			if (MadeSQ)
				{
				// change name
				CurSQ->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
				} // if
			else
				{
				// remove filters
				while (CurSQ->Filters)
					CurSQ->RemoveFilter(CurSQ->Filters);
				} // else remove filters
			// find first filter or add one
			if ((CurFilter = CurSQ->Filters) || (CurFilter = CurSQ->AddFilter(NULL)))
				{
				CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
				CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
				CurFilter->NewAttribute((char *)SpeciesAttr[0]->GetName());
				CurFilter->NewAttributeValue(ClassUnits[UnitNumber].PolyName);
				CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
				} // if
			CurEco->SetQuery(CurSQ);
			} // if
		} // if
	} // for

return (Success);

} // ForestWiz::ProcessOneSpeciesField

/*===========================================================================*/

int ForestWiz::ProcessTwoSpeciesFields(void)
{
double MaxOverFolHt, MaxUnderFolHt;
long UnitNumber, SubUnitNumber, FolCt, NumOverFoliage, NumUnderFoliage, TextureUnit = 0, EcosMade = 0;
EcosystemEffect *CurEco, *MadeEco;
GradientCritter *ActiveGrad;
MaterialEffect *EcoMat;
FoliageGroup *OverGrp, *UnderGrp;
Foliage *CurFol;
Raster *CurRast;
SearchQuery *CurSQ, *MadeSQ;
ThematicMap *OverDensTheme = NULL, *UnderDensTheme = NULL, *OverSizeTheme = NULL, *UnderSizeTheme = NULL, *OverMinSizeTheme = NULL, *UnderMinSizeTheme = NULL, 
	*OverDBHTheme = NULL, *UnderDBHTheme = NULL, *EcoDensTheme = NULL, *EcoDBHTheme = NULL, *EcoSizeTheme = NULL, *EcoMinSizeTheme = NULL, *MadeTheme;
DBFilterEvent *CurFilter;
Ecotype *Overstory = NULL, *Understory = NULL;
int Success = 1, Queried = 0, QueryResult = 0, SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0, Abort = 0;
bool ImagesAdded = false;
NotifyTag Changes[2];
char ThemeName[128];

// create one ecosystem for each class found in the dominant field
// for each class found in the subdominant field, clone the ecosystem.
// Create an overstory and understory ecotype for each eco
// in the overstory create a group for the class that is found in the dominant attribute
// populate with images
// in the understory create a group for each class that is found in the co-dominant attribute
// populate with images
// Create one thematic map for density and one for height, 
// attach TM's to overstory group heights and densities
// if two heights or densities are used create one thematic map for co-dominant density and 
// one for co-dominant height, 
// attach TM's to understory group heights and densities

// in certain circumstances, density themes are in the ecotype instead of the foliage group
if (NumDensFields == 1 || (NumDensFields > 1 && ! DensAttr[1]))
	{
	MadeTheme = NULL;
	if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
		strcpy(ThemeName, "Ecosystem Foliage Density");
	else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
		strcpy(ThemeName, "Ecosystem Foliage Basal Area");
	else
		strcpy(ThemeName, "Ecosystem Foliage Crown Closure");
	EcoDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (EcoDensTheme || (EcoDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			EcoDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		EcoDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		EcoDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		EcoDensTheme->NullConstant[0] = 0.0;
		EcoDensTheme->AttribFactor[0] = DensityThemeMult;
		EcoDensTheme->SetAttribute(DensAttr[0], 0);
		EcoDensTheme->SetAttribute(NULL, 1);
		EcoDensTheme->SetAttribute(NULL, 2);
		// create DBH theme if needed
		if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && DBHAttr[0])
			{
			MadeTheme = NULL;
			strcpy(ThemeName, "Ecosystem Foliage Diameter");
			EcoDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
			if (Abort)
				{
				return (0);
				} // if
			if (EcoDBHTheme || (EcoDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
				{
				if (MadeTheme)
					{
					// change name
					EcoDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
					} // if
				EcoDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
				EcoDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
				EcoDBHTheme->NullConstant[0] = 1.0;
				EcoDBHTheme->AttribFactor[0] = DBHThemeMult;
				EcoDBHTheme->SetAttribute(DBHAttr[0], 0);
				EcoDBHTheme->SetAttribute(NULL, 1);
				EcoDBHTheme->SetAttribute(NULL, 2);
				} // if
			} // if
		} // if
	} // if

// in certain circumstances, Size themes are in the ecotype instead of the foliage group
if (NumSizeFields == 1 || (NumSizeFields > 1 && ! SizeAttr[1]))
	{
	MadeTheme = NULL;
	if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
		strcpy(ThemeName, "Ecosystem Foliage Height");
	else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
		strcpy(ThemeName, "Ecosystem Foliage Diameter");
	else
		strcpy(ThemeName, "Ecosystem Foliage Age");
	EcoSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (EcoSizeTheme || (EcoSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			EcoSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		EcoSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		EcoSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		EcoSizeTheme->NullConstant[0] = 0.0;
		EcoSizeTheme->SetAttribute(SizeAttr[0], 0);
		EcoSizeTheme->SetAttribute(NULL, 1);
		EcoSizeTheme->SetAttribute(NULL, 2);
		EcoSizeTheme->AttribFactor[0] = SizeThemeMult;
		} // if

	if (SizeAttr[2] && MinSizeThemePresent)
		{
		MadeTheme = NULL;
		if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
			strcpy(ThemeName, "Ecosystem Foliage Minimum Height");
		else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
			strcpy(ThemeName, "Ecosystem Foliage Minimum Diameter");
		else
			strcpy(ThemeName, "Ecosystem Foliage Minimum Age");
		EcoMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (EcoMinSizeTheme || (EcoMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
			{
			if (MadeTheme)
				{
				// change name
				EcoMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
				} // if
			EcoMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
			EcoMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
			EcoMinSizeTheme->NullConstant[0] = 0.0;
			EcoMinSizeTheme->SetAttribute(SizeAttr[2], 0);
			EcoMinSizeTheme->SetAttribute(NULL, 1);
			EcoMinSizeTheme->SetAttribute(NULL, 2);
			if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
				EcoMinSizeTheme->AttribFactor[0] = .01;
			else
				EcoMinSizeTheme->AttribFactor[0] = SizeThemeMult;
			} // if
		} // if
	} // if

// first the ecosystems without vegetation
for (UnitNumber = 0; UnitNumber < NumClassUnits && Success; UnitNumber ++)
	{
	if (ClassUnits[UnitNumber].OverUnder & WCS_FORESTWIZ_OVERUNDER_OVERSTORY)
		{
		NumOverFoliage = 0;
		MaxOverFolHt = 0.0;
		if (ClassUnits[UnitNumber].HasFoliage)
			{
			for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
				{
				if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
					{
					if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxOverFolHt)
						MaxOverFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
					NumOverFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
					} // if
				} // for
			} // if
		for (SubUnitNumber = 0; SubUnitNumber < NumClassUnits && Success; SubUnitNumber ++)
			{
			if (ClassUnits[SubUnitNumber].OverUnder & WCS_FORESTWIZ_OVERUNDER_UNDERSTORY)
				{
				NumUnderFoliage = 0;
				MaxUnderFolHt = 0.0;
				sprintf(ThemeName, "%s/%s", ClassUnits[UnitNumber].PolyName, ClassUnits[SubUnitNumber].PolyName);
				MadeEco = NULL;
				// add ecosystem
				// look for ecosystem already existing first
				CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, ThemeName, Queried, QueryResult, Abort, FALSE);
				if (Abort)
					{
					return (0);
					} // if
				if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
					{
					if (MadeEco)
						{
						// change name
						CurEco->SetUniqueName(GlobalApp->AppEffects, ThemeName);
						// load material texture
						if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
							{
							if (! SetEcoProperties(EcoMat, ClassUnits[UnitNumber].GroundTexture))
								{
								Success = 0;
								break;
								} // if
							} // if
						} // if
					MadeSQ = NULL;
					// add search query
					// look for search query already existing first
					CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ThemeName, SQQueried, SQQueryResult, Abort, FALSE);
					if (Abort)
						{
						return (0);
						} // if
					if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
						{
						if (MadeSQ)
							{
							// change name
							CurSQ->SetUniqueName(GlobalApp->AppEffects, ThemeName);
							} // if
						else
							{
							// remove filters
							while (CurSQ->Filters)
								CurSQ->RemoveFilter(CurSQ->Filters);
							} // else remove filters
						// find first filter or add one
						if ((CurFilter = CurSQ->Filters) || (CurFilter = CurSQ->AddFilter(NULL)))
							{
							CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
							CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
							CurFilter->NewAttribute((char *)SpeciesAttr[0]->GetName());
							CurFilter->NewAttributeValue(ClassUnits[UnitNumber].PolyName);
							CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
							if (CurFilter = CurSQ->AddFilter(NULL))
								{
								CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
								CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
								CurFilter->EventType = WCS_DBFILTER_EVENTTYPE_SUB;
								CurFilter->NewAttribute((char *)SpeciesAttr[1]->GetName());
								CurFilter->NewAttributeValue(ClassUnits[SubUnitNumber].PolyName);
								CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
								CurFilter->AttributeNot = 1;
								} // if
							} // if
						CurEco->SetQuery(CurSQ);
						} // if
					} // if

				if (ClassUnits[SubUnitNumber].HasFoliage)
					{
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[SubUnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							if (ClassUnits[SubUnitNumber].Groups[0].FolHt[FolCt] > MaxUnderFolHt)
								MaxUnderFolHt = ClassUnits[SubUnitNumber].Groups[0].FolHt[FolCt];
							NumUnderFoliage += ClassUnits[SubUnitNumber].Groups[0].FolCount[FolCt];
							} // if
						} // for
					} // if

				// add ecotypes
				if (NumOverFoliage)
					Overstory = EcoMat->NewEcotype(0);
				if (NumUnderFoliage)
					Understory = EcoMat->NewEcotype(1);

				if (Overstory)
					{
					Overstory->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
					Overstory->ConstDensity = 1;
					Overstory->DensityUnits = EcoAreaUnits;
					Overstory->AbsHeightResident = 
						(EcoSizeTheme) ? WCS_ECOTYPE_ABSRESIDENT_ECOTYPE: WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;
					Overstory->AbsDensResident = 
						(EcoDensTheme) ? WCS_ECOTYPE_ABSRESIDENT_ECOTYPE: WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;

					// density
					if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
						{
						Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_CROWNCLOSURE, EcoDensTheme);
						} // else
					else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
						{
						Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_BASALAREA, EcoDensTheme);
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoDBHTheme);
						} // else if
					else //if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
						{
						Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DENSITY, EcoDensTheme);
						} // if
					// size
					if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
						{
						Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoSizeTheme);
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						// set up DBH curve
						SetupDBHCurve(&Overstory->DBHCurve);
						} // if
					else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
						{
						Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_AGE, EcoSizeTheme);
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						// set up Age curve
						SetupAgeCurve(&Overstory->AgeCurve);
						} // else if
					else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
						{
						Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_HEIGHT, EcoSizeTheme);
						Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						} // else
					Overstory->SetAnimDefaults();

					if (Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
						{
						Overstory->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxOverFolHt);
						Overstory->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
						} // if
					
					// add group to overstory as required
					OverGrp = NULL;
					if (! (OverGrp = Overstory->AddFoliageGroup(NULL, ClassUnits[UnitNumber].PolyName)))
						{
						Success = 0;
						break;
						} // if
					// create thematic map for height and density
					// make density thematic maps
					if (Overstory->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && ! OverDensTheme)
						{
						if (NumDensFields >= 1 && DensAttr[0])
							{
							MadeTheme = NULL;
							if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
								sprintf(ThemeName, NumDensFields == 1 ? "Density": "Dominant Density");
							else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
								sprintf(ThemeName, NumDensFields == 1 ? "Basal Area": "Dominant Basal Area");
							else
								sprintf(ThemeName, NumDensFields == 1 ? "Crown Closure": "Dominant Crown Closure");
							OverDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
							if (Abort)
								{
								return (0);
								} // if
							if (OverDensTheme || (OverDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
								{
								if (MadeTheme)
									{
									// change name
									OverDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
									} // if
								OverDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
								OverDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
								OverDensTheme->NullConstant[0] = 0.0;
								OverDensTheme->AttribFactor[0] = DensityThemeMult;
								OverDensTheme->SetAttribute(DensAttr[0], 0);
								OverDensTheme->SetAttribute(NULL, 1);
								OverDensTheme->SetAttribute(NULL, 2);
								// create DBH theme if needed
								if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && DBHAttr[0])
									{
									MadeTheme = NULL;
									sprintf(ThemeName, NumDBHFields == 1 || DBHAttr[1] == DBHAttr[0] ? "Diameter": "Dominant Diameter");
									OverDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
									if (Abort)
										{
										return (0);
										} // if
									if (OverDBHTheme || (OverDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
										{
										if (MadeTheme)
											{
											// change name
											OverDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
											} // if
										OverDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
										OverDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
										OverDBHTheme->NullConstant[0] = 1.0;
										OverDBHTheme->AttribFactor[0] = DBHThemeMult;
										OverDBHTheme->SetAttribute(DBHAttr[0], 0);
										OverDBHTheme->SetAttribute(NULL, 1);
										OverDBHTheme->SetAttribute(NULL, 2);
										} // if
									} // if
								} // if
							} // if
						} // if
					if (Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && ! OverSizeTheme)
						{
						// make size thematic maps
						if (NumSizeFields >= 1 && SizeAttr[0] && (OverGrp || NumSizeFields == 1))
							{
							MadeTheme = NULL;
							if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
								sprintf(ThemeName, NumSizeFields == 1 ? "Height": "Dominant Height");
							else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
								sprintf(ThemeName, NumSizeFields == 1 ? "Diameter": "Dominant Diameter");
							else
								sprintf(ThemeName, NumSizeFields == 1 ? "Age": "Dominant Age");
							OverSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
							if (Abort)
								{
								return (0);
								} // if
							if (OverSizeTheme || (OverSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
								{
								if (MadeTheme)
									{
									// change name
									OverSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
									} // if
								OverSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
								OverSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
								OverSizeTheme->NullConstant[0] = 0.0;
								OverSizeTheme->SetAttribute(SizeAttr[0], 0);
								OverSizeTheme->SetAttribute(NULL, 1);
								OverSizeTheme->SetAttribute(NULL, 2);
								OverSizeTheme->AttribFactor[0] = SizeThemeMult;
								} // if

							if (SizeAttr[2] && MinSizeThemePresent)
								{
								MadeTheme = NULL;
								if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
									strcpy(ThemeName, NumSizeFields == 1 ? "Minimum Height": "Dominant Minimum Height");
								else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
									strcpy(ThemeName, NumSizeFields == 1 ? "Minimum Diameter": "Dominant Minimum Diameter");
								else
									strcpy(ThemeName, NumSizeFields == 1 ? "Minimum Age": "Dominant Minimum Age");
								OverMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
								if (Abort)
									{
									return (0);
									} // if
								if (OverMinSizeTheme || (OverMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
									{
									if (MadeTheme)
										{
										// change name
										OverMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
										} // if
									OverMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
									OverMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
									OverMinSizeTheme->NullConstant[0] = 0.0;
									OverMinSizeTheme->SetAttribute(SizeAttr[2], 0);
									OverMinSizeTheme->SetAttribute(NULL, 1);
									OverMinSizeTheme->SetAttribute(NULL, 2);
									if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
										OverMinSizeTheme->AttribFactor[0] = .01;
									else
										OverMinSizeTheme->AttribFactor[0] = SizeThemeMult;
									} // if
								} // if
							} // if
						} // if
					// add images
					// size
					if (Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
						{
						OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(MaxOverFolHt);
						OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
						if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							// set up DBH curve
							SetupDBHCurve(&OverGrp->DBHCurve);
							} // if
						else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_AGE, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							// set up Age curve
							SetupAgeCurve(&OverGrp->AgeCurve);
							} // else if
						else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_HEIGHT, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							} // else
						} // if
					// density
					OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue((double)NumOverFoliage);
					if (Overstory->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
						{
						if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DENSITY, OverDensTheme);
							} // if
						else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_BASALAREA, OverDensTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, OverDBHTheme);
							} // else if
						else // (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE, OverDensTheme);
							} // else
						} // if
					OverGrp->SetAnimDefaults(Overstory);
					// so we don't divide by 0
					if (MaxOverFolHt < .0001)
						MaxOverFolHt = .0001;
					if (NumOverFoliage == 0)
						NumOverFoliage = 1;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							// add foliage images to library
							if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName(), 
								TRUE, //NonRedundant 
								TRUE, //ConfirmFile 
								TRUE, //LoadnPrep 
								FALSE)) //AllowImageManagement
								{
								// notify of image added
								ImagesAdded = true;
								} // if image added
							if (CurFol = OverGrp->AddFoliage(NULL))
								{
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumOverFoliage);
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxOverFolHt);
								// add foliage to  ecotype
								CurFol->SetRaster(CurRast);
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						else
							break;
						} // for FolCt
					} // if
				if (Understory)
					{
					Understory->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
					Understory->ConstDensity = 1;
					Understory->DensityUnits = EcoAreaUnits;
					Understory->AbsHeightResident = 
						(EcoSizeTheme) ? WCS_ECOTYPE_ABSRESIDENT_ECOTYPE: WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;
					Understory->AbsDensResident = 
						(EcoDensTheme) ? WCS_ECOTYPE_ABSRESIDENT_ECOTYPE: WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;

					// density
					if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
						{
						Understory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_CROWNCLOSURE, EcoDensTheme);
						} // else
					else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
						{
						Understory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_BASALAREA, EcoDensTheme);
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoDBHTheme);
						} // else if
					else //if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
						{
						Understory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_DENSITY, EcoDensTheme);
						} // if
					// size
					if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
						{
						Understory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoSizeTheme);
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						// set up DBH curve
						SetupDBHCurve(&Understory->DBHCurve);
						} // if
					else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
						{
						Understory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_AGE, EcoSizeTheme);
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						// set up Age curve
						SetupAgeCurve(&Understory->AgeCurve);
						} // else if
					else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
						{
						Understory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_HEIGHT, EcoSizeTheme);
						Understory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
						} // else
					Understory->SetAnimDefaults();

					if (Understory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
						{
						Understory->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxUnderFolHt);
						Understory->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
						} // if

					// add group to understory as required
					UnderGrp = NULL;
					if (! (UnderGrp = Understory->AddFoliageGroup(NULL, ClassUnits[SubUnitNumber].PolyName)))
						{
						Success = 0;
						break;
						} // if
					if (Understory->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && ! UnderDensTheme)
						{
						UnderDensTheme = OverDensTheme;	// may be overridden next but may make things easier later
						if (NumDensFields > 1 && DensAttr[1])
							{
							MadeTheme = NULL;
							if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
								sprintf(ThemeName, "Subdominant Density");
							else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
								sprintf(ThemeName, "Subdominant Basal Area");
							else
								sprintf(ThemeName, "Subdominant Crown Closure");
							UnderDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
							if (Abort)
								{
								return (0);
								} // if
							if (UnderDensTheme || (UnderDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
								{
								if (MadeTheme)
									{
									// change name
									UnderDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
									} // if
								UnderDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
								UnderDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
								UnderDensTheme->NullConstant[0] = 0.0;
								UnderDensTheme->AttribFactor[0] = DensityThemeMult;
								UnderDensTheme->SetAttribute(DensAttr[1], 0);
								UnderDensTheme->SetAttribute(NULL, 1);
								UnderDensTheme->SetAttribute(NULL, 2);
								// create DBH theme if needed
								// test for DBHAttr[0] but use DBHAttr[1] if it exists
								if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && DBHAttr[0])
									{
									MadeTheme = NULL;
									sprintf(ThemeName, NumDBHFields == 1 || DBHAttr[1] == DBHAttr[0] ? "Diameter": "Subdominant Diameter");
									UnderDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
									if (Abort)
										{
										return (0);
										} // if
									if (UnderDBHTheme || (UnderDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
										{
										if (MadeTheme)
											{
											// change name
											UnderDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
											} // if
										UnderDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
										UnderDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
										UnderDBHTheme->NullConstant[0] = 1.0;
										UnderDBHTheme->AttribFactor[0] = DBHThemeMult;
										UnderDBHTheme->SetAttribute(DBHAttr[1] ? DBHAttr[1]: DBHAttr[0], 0);
										UnderDBHTheme->SetAttribute(NULL, 1);
										UnderDBHTheme->SetAttribute(NULL, 2);
										} // if
									} // if
								} // if
							} // if
						} // if
					if (Understory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && ! UnderSizeTheme)
						{
						UnderSizeTheme = OverSizeTheme;	// may be overridden next but may make things easier later
						if (NumSizeFields > 1 && SizeAttr[1])
							{
							MadeTheme = NULL;
							if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
								sprintf(ThemeName, "Subdominant Height");
							else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
								sprintf(ThemeName, "Subdominant Diameter");
							else
								sprintf(ThemeName, "Subdominant Age");
							UnderSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
							if (Abort)
								{
								return (0);
								} // if
							if (UnderSizeTheme || (UnderSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
								{
								if (MadeTheme)
									{
									// change name
									UnderSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
									} // if
								UnderSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
								UnderSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
								UnderSizeTheme->NullConstant[0] = 0.0;
								UnderSizeTheme->SetAttribute(SizeAttr[1], 0);
								UnderSizeTheme->SetAttribute(NULL, 1);
								UnderSizeTheme->SetAttribute(NULL, 2);
								UnderSizeTheme->AttribFactor[0] = SizeThemeMult;
								} // if

							if (SizeAttr[2] && MinSizeThemePresent)
								{
								MadeTheme = NULL;
								if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
									strcpy(ThemeName, "Subdominant Minimum Height");
								else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
									strcpy(ThemeName, "Subdominant Minimum Diameter");
								else
									strcpy(ThemeName, "Subdominant Minimum Age");
								UnderMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
								if (Abort)
									{
									return (0);
									} // if
								if (UnderMinSizeTheme || (UnderMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
									{
									if (MadeTheme)
										{
										// change name
										UnderMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
										} // if
									UnderMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
									UnderMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
									UnderMinSizeTheme->NullConstant[0] = 0.0;
									UnderMinSizeTheme->SetAttribute(SizeAttr[2], 0);
									UnderMinSizeTheme->SetAttribute(NULL, 1);
									UnderMinSizeTheme->SetAttribute(NULL, 2);
									if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
										UnderMinSizeTheme->AttribFactor[0] = .01;
									else
										UnderMinSizeTheme->AttribFactor[0] = SizeThemeMult;
									} // if
								} // if
							} // if
						} // if
					// add images
					// size
					if (Understory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
						{
						UnderGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(MaxUnderFolHt);
						UnderGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
						if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, UnderSizeTheme);
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, UnderMinSizeTheme);
							// set up DBH curve
							SetupDBHCurve(&UnderGrp->DBHCurve);
							} // if
						else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_AGE, UnderSizeTheme);
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, UnderMinSizeTheme);
							// set up Age curve
							SetupAgeCurve(&UnderGrp->AgeCurve);
							} // else if
						else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_HEIGHT, UnderSizeTheme);
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, UnderMinSizeTheme);
							} // else
						} // if
					// density
					UnderGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue((double)NumUnderFoliage);
					if (Understory->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
						{
						if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DENSITY, UnderDensTheme);
							} // if
						else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_BASALAREA, UnderDensTheme);
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, UnderDBHTheme);
							} // else if
						else // (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
							{
							UnderGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE, UnderDensTheme);
							} // else
						} // if
					UnderGrp->SetAnimDefaults(Understory);
					// so we don't divide by 0
					if (MaxUnderFolHt < .0001)
						MaxUnderFolHt = .0001;
					if (NumUnderFoliage == 0)
						NumUnderFoliage = 1;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[SubUnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							// add foliage images to library
							if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[SubUnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[SubUnitNumber].Groups[0].FolNames[FolCt].GetName(), 
								TRUE, //NonRedundant 
								TRUE, //ConfirmFile 
								TRUE, //LoadnPrep 
								FALSE)) //AllowImageManagement
								{
								// notify of image added
								ImagesAdded = true;
								} // if image added
							if (CurFol = UnderGrp->AddFoliage(NULL))
								{
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumUnderFoliage);
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxUnderFolHt);
								// add foliage to  ecotype
								CurFol->SetRaster(CurRast);
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						else
							break;
						} // for FolCt
					} // if
				EcosMade ++;
				} // if
			} // for
		} // if
	} // for
	
if (ImagesAdded)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED); 
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
	} // if

return (Success);

} //ForestWiz::ProcessTwoSpeciesFields

/*===========================================================================*/

int ForestWiz::ProcessMultiSpeciesFields(void)
{
double MaxFolHt;
long UnitNumber, FolCt, NumFoliage, TextureUnit = 0;
EcosystemEffect *CurEco, *MadeEco;
GradientCritter *ActiveGrad;
MaterialEffect *EcoMat;
FoliageGroup *OverGrp;
Foliage *CurFol;
Raster *CurRast;
SearchQuery *CurSQ, *MadeSQ;
ThematicMap *OverDensTheme = NULL, *OverSizeTheme = NULL, *OverMinSizeTheme = NULL, *OverDBHTheme = NULL, *MadeTheme;
DBFilterEvent *CurFilter;
Ecotype *Overstory = NULL;
int Success = 1, Queried = 0, QueryResult = 0, SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0, Abort = 0;
NotifyTag Changes[2];
char ThemeName[128];

// create one ecosystem with an overstory ecotype
// create a group in the overstory for each class
// populate with images
// For each group create a density and height thematic map from the attribs specified
// create thematic map for density from SpeciesAttr[UnitNumber]
// create search query for density TM from SpeciesAttr[UnitNumber] > 0 && SizeAttr[UnitNumber] > 0
//	or SizeAttr[0] > 0 if only one size attrib. Do not use size if there are no size attributes
// if individual size fields create thematic map for size from SizeAttr[UnitNumber]
// use same search query as for density

// query to link the vegetated ecosystem
MadeSQ = NULL;
strcpy(ThemeName, "Foliage Ecosystem");
CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ThemeName, SQQueried, SQQueryResult, Abort, FALSE);
if (Abort)
	{
	return (0);
	} // if
if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
	{
	if (MadeSQ)
		{
		// change name
		CurSQ->SetUniqueName(GlobalApp->AppEffects, ThemeName);
		// remove filters and add one
		} // if
	else
		{
		while (CurSQ->Filters)
			CurSQ->RemoveFilter(CurSQ->Filters);
		} // else remove filters
	// find first filter
	if ((CurFilter = CurSQ->Filters) || (CurFilter = CurSQ->AddFilter(NULL)))
		{
		CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
		CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
		CurFilter->NewAttribute((char *)SpeciesAttr[0]->GetName());
		CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EXISTS;

		// add filter for each species
		for (UnitNumber = 1; UnitNumber < NumClassUnits; UnitNumber ++)
			{
			if (CurFilter = CurSQ->AddFilter(NULL))
				{
				CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
				CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
				CurFilter->NewAttribute((char *)SpeciesAttr[UnitNumber]->GetName());
				CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EXISTS;
				} // if
			} // for
		} // if
	} // if

// find first foliage unit and grab ground texture number
for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
	{
	TextureUnit = ClassUnits[UnitNumber].GroundTexture;
	break;
	} // for

// make one ecosystem and add an overstory
MadeEco = NULL;
strcpy(ThemeName, "Foliage Ecosystem");
// look for ecosystem already existing first, force removal
CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, ThemeName, Queried, QueryResult, Abort, TRUE);
if (Abort)
	{
	return (0);
	} // if
if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
	{
	CurEco->SetQuery(CurSQ);
	if (MadeEco)
		{
		// change name
		CurEco->SetUniqueName(GlobalApp->AppEffects, ThemeName);
		} // if
	// load material texture
	if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
		{
		if (! SetEcoProperties(EcoMat, TextureUnit))
			{
			return (0);
			} // if

		// add ecotypes
		if (Overstory = EcoMat->NewEcotype(0))
			{
			Overstory->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
			Overstory->ConstDensity = 1;
			Overstory->DensityUnits = EcoAreaUnits;
			Overstory->AbsHeightResident = Overstory->AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;

			if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
				Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
			else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
				Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
			else
				Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;

			if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
				Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
			else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
				Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
			else
				Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;

			Overstory->SetAnimDefaults();

			for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
				{
				if (ClassUnits[UnitNumber].HasFoliage)
					{
					OverDensTheme = OverSizeTheme = NULL;
					// add group to overstory and/or understory as required
					OverGrp = NULL;
					if (! (OverGrp = Overstory->AddFoliageGroup(NULL, ClassUnits[UnitNumber].PolyName)))
						{
						Success = 0;
						break;
						} // if

					// create thematic map for height and density
					// make density thematic maps
					MadeTheme = NULL;
					if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
						sprintf(ThemeName, "%s Density", ClassUnits[UnitNumber].PolyName);
					else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
						sprintf(ThemeName, "%s Basal Area", ClassUnits[UnitNumber].PolyName);
					else
						sprintf(ThemeName, "%s Crown Closure", ClassUnits[UnitNumber].PolyName);
					OverDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
					if (Abort)
						{
						return (0);
						} // if
					if (OverDensTheme || (OverDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
						{
						if (MadeTheme)
							{
							// change name
							OverDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
							} // if
						OverDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
						OverDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
						OverDensTheme->NullConstant[0] = 0.0;
						OverDensTheme->AttribFactor[0] = DensityThemeMult;
						OverDensTheme->SetAttribute(SpeciesAttr[UnitNumber], 0);
						OverDensTheme->SetAttribute(NULL, 1);
						OverDensTheme->SetAttribute(NULL, 2);
						} // if

					OverDBHTheme = NULL;
					if (OverDensTheme && DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && (DBHAttr[UnitNumber] || DBHAttr[0]))
						{
						MadeTheme = NULL;
						strcpy(ThemeName, "%s Diameter");
						OverDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
						if (Abort)
							{
							return (0);
							} // if
						if (OverDBHTheme || (OverDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
							{
							if (MadeTheme)
								{
								// change name
								OverDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
								} // if
							OverDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
							OverDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
							OverDBHTheme->NullConstant[0] = 1.0;
							OverDBHTheme->AttribFactor[0] = DBHThemeMult;
							OverDBHTheme->SetAttribute(DBHAttr[UnitNumber] ? DBHAttr[UnitNumber]: DBHAttr[0], 0);
							OverDBHTheme->SetAttribute(NULL, 1);
							OverDBHTheme->SetAttribute(NULL, 2);
							} // if
						} // if

					// make size thematic maps
					if (NumSizeFields > 0)
						{
						MadeTheme = NULL;
						if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
							sprintf(ThemeName, "%s Height", ClassUnits[UnitNumber].PolyName);
						else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
							sprintf(ThemeName, "%s Diameter", ClassUnits[UnitNumber].PolyName);
						else
							sprintf(ThemeName, "%s Age", ClassUnits[UnitNumber].PolyName);
						OverSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
						if (Abort)
							{
							return (0);
							} // if
						if (OverSizeTheme || (OverSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
							{
							if (MadeTheme)
								{
								// change name
								OverSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
								} // if
							OverSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
							OverSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
							OverSizeTheme->NullConstant[0] = 0.0;
							OverSizeTheme->SetAttribute(NumSizeFields > 1 ? SizeAttr[UnitNumber]: SizeAttr[0], 0);
							OverSizeTheme->SetAttribute(NULL, 1);
							OverSizeTheme->SetAttribute(NULL, 2);
							OverSizeTheme->AttribFactor[0] = SizeThemeMult;
							} // if

						if (SizeAttr[2] && MinSizeThemePresent)
							{
							MadeTheme = NULL;
							if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
								sprintf(ThemeName, "%s Minimum Height", ClassUnits[UnitNumber].PolyName);
							else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
								sprintf(ThemeName, "%s Minimum Diameter", ClassUnits[UnitNumber].PolyName);
							else
								sprintf(ThemeName, "%s Minimum Age", ClassUnits[UnitNumber].PolyName);
							OverMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
							if (Abort)
								{
								return (0);
								} // if
							if (OverMinSizeTheme || (OverMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
								{
								if (MadeTheme)
									{
									// change name
									OverMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
									} // if
								OverMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
								OverMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
								OverMinSizeTheme->NullConstant[0] = 0.0;
								OverMinSizeTheme->SetAttribute(SizeAttr[2], 0);
								OverMinSizeTheme->SetAttribute(NULL, 1);
								OverMinSizeTheme->SetAttribute(NULL, 2);
								if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
									OverMinSizeTheme->AttribFactor[0] = .01;
								else
									OverMinSizeTheme->AttribFactor[0] = SizeThemeMult;
								} // if
							} // if
						} // if

					// add images
					// find max fol height
					MaxFolHt = 0.0;
					NumFoliage = 0;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxFolHt)
								MaxFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
							NumFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
							} // if
						} // for

					if (OverGrp)
						{
						OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
						OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
						OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue((double)NumFoliage);
						// set thematic maps for height and density
						// density
						if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DENSITY, OverDensTheme);
							} // if
						else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_BASALAREA, OverDensTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, OverDBHTheme);
							} // else if
						else // (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE, OverDensTheme);
							} // else

						// size
						if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_HEIGHT, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							// set up DBH curve
							SetupDBHCurve(&OverGrp->DBHCurve);
							} // if
						else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_AGE, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							// set up Age curve
							SetupAgeCurve(&OverGrp->AgeCurve);
							} // else if
						else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
							{
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_HEIGHT, OverSizeTheme);
							OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
							} // else

						OverGrp->SetAnimDefaults(Overstory);
						} // if

					// so we don't divide by 0
					if (MaxFolHt < .0001)
						MaxFolHt = .0001;
					if (NumFoliage == 0)
						NumFoliage = 1;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							// add foliage images to library
							if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName(), 
								TRUE, //NonRedundant 
								TRUE, //ConfirmFile 
								TRUE, //LoadnPrep 
								FALSE //AllowImageManagement
								))
								{
								// notify of image added
								Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
								Changes[1] = NULL;
								GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
								} // if image added
							if (OverGrp)
								{
								if (CurFol = OverGrp->AddFoliage(NULL))
									{
									CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumFoliage);
									CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxFolHt);
									// add foliage to  ecotype
									CurFol->SetRaster(CurRast);
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								} // if
							} // if
						else
							break;
						} // for FolCt
					} // if no foliage
				} // for UnitNumber
			} // if Overstory && Understory
		} // if ActiveGrad
	} // if Ecosystem

return (Success);

} //ForestWiz::ProcessMultiSpeciesFields

/*===========================================================================*/

int ForestWiz::ProcessFARSITESpeciesFields(void)
{
double MaxFolHt;
long UnitNumber, SpFieldCt, FolCt, NumFoliage, TextureUnit = 0, EcosMade = 0;
EcosystemEffect *CurEco, *MadeEco;
GradientCritter *ActiveGrad;
MaterialEffect *EcoMat;
FoliageGroup *OverGrp;
Foliage *CurFol;
Raster *CurRast;
SearchQuery *CurSQ, *MadeSQ;
ThematicMap *OverDensTheme = NULL, *OverSizeTheme = NULL, *OverMinSizeTheme = NULL, *EcoDensTheme = NULL, *EcoDBHTheme = NULL, *EcoSizeTheme = NULL, *EcoMinSizeTheme = NULL, *MadeTheme;
DBFilterEvent *CurFilter;
Ecotype *Overstory = NULL;
const char *LayerName;
int Success = 1, Queried = 0, QueryResult = 0, SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0, Abort = 0;
NotifyTag Changes[2];
char ThemeName[128], CombinedName[256];

// ecosystems with no foliage
for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
	{
	if (! ClassUnits[UnitNumber].HasFoliage)
		{
		MadeEco = NULL;
		// add ecosystem
		// look for ecosystem already existing first
		CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, ClassUnits[UnitNumber].PolyName, Queried, QueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
			{
			if (MadeEco)
				{
				// change name
				CurEco->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
				// load material texture
				if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
					{
					if (! SetEcoProperties(EcoMat, ClassUnits[UnitNumber].GroundTexture))
						{
						Success = 0;
						break;
						} // if
					} // if
				} // if
			MadeSQ = NULL;
			// add search query
			// look for search query already existing first
			CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ClassUnits[UnitNumber].PolyName, SQQueried, SQQueryResult, Abort, FALSE);
			if (Abort)
				{
				return (0);
				} // if
			if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
				{
				if (MadeSQ)
					{
					// change name
					CurSQ->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
					} // if
				// remove filters
				while (CurSQ->Filters)
					CurSQ->RemoveFilter(CurSQ->Filters);
				// add one filter for each species field
				for (SpFieldCt = 0; SpFieldCt < NumSpDensFields && Success; SpFieldCt ++)
					{
					LayerName = &SpeciesAttr[SpFieldCt]->GetName()[1];
					if (CurFilter = CurSQ->AddFilter(NULL))
						{
						CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
						CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
						CurFilter->NewAttribute((char *)SpeciesAttr[SpFieldCt]->GetName());
						CurFilter->NewAttributeValue(ClassUnits[UnitNumber].PolyName);
						CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
						} // if
					} // for
				CurEco->SetQuery(CurSQ);
				} // if
			} // if
		EcosMade ++;
		} // if no foliage
	} // for

// one ecosystem for foliage units
if (EcosMade < NumClassUnits)
	{
	// query to link the vegetated ecosystem
	MadeSQ = NULL;
	strcpy(ThemeName, "Foliage Ecosystem");
	CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ThemeName, SQQueried, SQQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeSQ)
			{
			// change name
			CurSQ->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			// remove filters and add one
			} // if
		while (CurSQ->Filters)
			CurSQ->RemoveFilter(CurSQ->Filters);
		// create filters - one for each species field
		if (CurFilter = CurSQ->AddFilter(NULL))
			{
			for (SpFieldCt = 0; SpFieldCt < NumSpDensFields; SpFieldCt ++)
				{
				CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
				CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
				CurFilter->NewAttribute((char *)SpeciesAttr[SpFieldCt]->GetName());
				CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EXISTS;
				} // for

			// add additional subtractive filters for each ecosystem which has no foliage
			for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
				{
				if (! ClassUnits[UnitNumber].HasFoliage)
					{
					for (SpFieldCt = 0; SpFieldCt < NumSpDensFields; SpFieldCt ++)
						{
						if (CurFilter = CurSQ->AddFilter(NULL))
							{
							CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
							CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
							CurFilter->EventType = WCS_DBFILTER_EVENTTYPE_SUB;
							CurFilter->NewAttribute((char *)SpeciesAttr[SpFieldCt]->GetName());
							CurFilter->NewAttributeValue(ClassUnits[UnitNumber].PolyName);
							CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
							} // if
						} // for
					} // if
				} // for
			} // if
		} // if

	// absolute density is in ecotype
	// thematic map for ecotype density comes from DensAttr[0]
	if (DensAttr[0])
		{
		MadeTheme = NULL;
		if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
			strcpy(ThemeName, "Ecosystem Foliage Density");
		else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
			strcpy(ThemeName, "Ecosystem Foliage Basal Area");
		else
			strcpy(ThemeName, "Ecosystem Foliage Crown Closure");
		EcoDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (EcoDensTheme || (EcoDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
			{
			if (MadeTheme)
				{
				// change name
				EcoDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
				} // if
			EcoDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
			EcoDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
			EcoDensTheme->NullConstant[0] = 0.0;
			EcoDensTheme->AttribFactor[0] = DensityThemeMult;
			EcoDensTheme->SetAttribute(DensAttr[0], 0);
			EcoDensTheme->SetAttribute(NULL, 1);
			EcoDensTheme->SetAttribute(NULL, 2);
			// create DBH theme if needed
			if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA && DBHAttr[0])
				{
				MadeTheme = NULL;
				strcpy(ThemeName, "Ecosystem Foliage Diameter");
				EcoDBHTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
				if (Abort)
					{
					return (0);
					} // if
				if (EcoDBHTheme || (EcoDBHTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
					{
					if (MadeTheme)
						{
						// change name
						EcoDBHTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
						} // if
					EcoDBHTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
					EcoDBHTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
					EcoDBHTheme->NullConstant[0] = 1.0;
					EcoDBHTheme->AttribFactor[0] = DBHThemeMult;
					EcoDBHTheme->SetAttribute(DBHAttr[0], 0);
					EcoDBHTheme->SetAttribute(NULL, 1);
					EcoDBHTheme->SetAttribute(NULL, 2);
					} // if
				} // if
			} // if
		} // if

	// absolute size is in ecotype if only one size field is chosen, otherwise it is in foliage group
	// thematic map for size comes from SizeAttr[0] if one size field
	if (SizeAttr[0] && NumSizeFields == 1)
		{
		MadeTheme = NULL;
		if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
			strcpy(ThemeName, "Ecosystem Foliage Height");
		else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
			strcpy(ThemeName, "Ecosystem Foliage Diameter");
		else
			strcpy(ThemeName, "Ecosystem Foliage Age");
		EcoSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (EcoSizeTheme || (EcoSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
			{
			if (MadeTheme)
				{
				// change name
				EcoSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
				} // if
			EcoSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
			EcoSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
			EcoSizeTheme->NullConstant[0] = 0.0;
			EcoSizeTheme->SetAttribute(SizeAttr[0], 0);
			EcoSizeTheme->SetAttribute(NULL, 1);
			EcoSizeTheme->SetAttribute(NULL, 2);
			EcoSizeTheme->AttribFactor[0] = SizeThemeMult;
			} // if

		if (SizeAttr[2] && MinSizeThemePresent)
			{
			MadeTheme = NULL;
			if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
				strcpy(ThemeName, "Ecosystem Foliage Minimum Height");
			else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
				strcpy(ThemeName, "Ecosystem Foliage Minimum Diameter");
			else
				strcpy(ThemeName, "Ecosystem Foliage Minimum Age");
			EcoMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
			if (Abort)
				{
				return (0);
				} // if
			if (EcoMinSizeTheme || (EcoMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
				{
				if (MadeTheme)
					{
					// change name
					EcoMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
					} // if
				EcoMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
				EcoMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
				EcoMinSizeTheme->NullConstant[0] = 0.0;
				EcoMinSizeTheme->SetAttribute(SizeAttr[2], 0);
				EcoMinSizeTheme->SetAttribute(NULL, 1);
				EcoMinSizeTheme->SetAttribute(NULL, 2);
				if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
					EcoMinSizeTheme->AttribFactor[0] = .01;
				else
					EcoMinSizeTheme->AttribFactor[0] = SizeThemeMult;
				} // if
			} // if
		} // if

	// find first foliage unit and grab ground texture number
	for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
		{
		if (ClassUnits[UnitNumber].HasFoliage)
			{
			TextureUnit = ClassUnits[UnitNumber].GroundTexture;
			break;
			} // if no foliage
		} // for

	// find max fol height
	MaxFolHt = 0.0;
	NumFoliage = 0;
	for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
		{
		if (ClassUnits[UnitNumber].HasFoliage)
			{
			for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
				{
				if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
					{
					if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxFolHt)
						MaxFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
					NumFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
					} // if
				} // for
			} // if
		} // for

	// make one ecosystem and add an overstory
	MadeEco = NULL;
	strcpy(ThemeName, "Foliage Ecosystem");
	// look for ecosystem already existing first, force removal
	CurEco = (EcosystemEffect *)FindComponent(WCS_EFFECTSSUBCLASS_ECOSYSTEM, ThemeName, Queried, QueryResult, Abort, TRUE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurEco || (CurEco = MadeEco = new EcosystemEffect(NULL, GlobalApp->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
		{
		CurEco->SetQuery(CurSQ);
		if (MadeEco)
			{
			// change name
			CurEco->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		// load material texture
		if ((ActiveGrad = CurEco->EcoMat.GetActiveNode()) && (EcoMat = (MaterialEffect *)ActiveGrad->GetThing()))
			{
			if (! SetEcoProperties(EcoMat, TextureUnit))
				{
				return (0);
				} // if

			// add ecotypes
			Overstory = EcoMat->NewEcotype(0);

			if (Overstory)
				{
				Overstory->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
				Overstory->ConstDensity = 1;
				Overstory->DensityUnits = EcoAreaUnits;
				Overstory->AbsHeightResident = 
					(EcoSizeTheme || ! SizeAttr[0]) ? WCS_ECOTYPE_ABSRESIDENT_ECOTYPE: WCS_ECOTYPE_ABSRESIDENT_FOLGROUP;
				Overstory->AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;

				// density
				if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE)
					{
					Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_CLOSURE;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_CROWNCLOSURE, EcoDensTheme);
					} // else
				else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
					{
					Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_BASALAREA, EcoDensTheme);
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoDBHTheme);
					} // else if
				else //if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
					{
					Overstory->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DENSITY, EcoDensTheme);
					} // if
				// size
				if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
					{
					Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_DBH, EcoSizeTheme);
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
					// set up DBH curve
					SetupDBHCurve(&Overstory->DBHCurve);
					} // if
				else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
					{
					Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
					// set up Age curve
					SetupAgeCurve(&Overstory->AgeCurve);
					} // else if
				else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
					{
					Overstory->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_HEIGHT, EcoSizeTheme);
					Overstory->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, EcoMinSizeTheme);
					} // else

				Overstory->SetAnimDefaults();

				if (Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
					{
					Overstory->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
					Overstory->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
					} // if

				// one foliage group for each species and each field
				// for each species field
				for (SpFieldCt = 0; SpFieldCt < NumSpDensFields && Success; SpFieldCt ++)
					{
					LayerName = &SpeciesAttr[SpFieldCt]->GetName()[1];
					// for each unit
					for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
						{
						if (ClassUnits[UnitNumber].HasFoliage)
							{
							sprintf(CombinedName, "%s %s", LayerName, ClassUnits[UnitNumber].PolyName);
							// create foliage group named for the species field and the unit
							// this could result in a lot of thematic maps but if they are not needed they will not be initialized
							OverDensTheme = OverSizeTheme = NULL;
							// add group to overstory and/or understory as required
							OverGrp = NULL;
							if (! (OverGrp = Overstory->AddFoliageGroup(NULL, CombinedName)))
								{
								Success = 0;
								break;
								} // if

							// create thematic map for relative density from SpDensAttr[species field], correct for percentage if necessary
							if (SpDensAttr[SpFieldCt])
								{
								MadeTheme = NULL;
								if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_STEMS)
									sprintf(ThemeName, "%s Density", CombinedName);
								else if (DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA)
									sprintf(ThemeName, "%s Basal Area", CombinedName);
								else
									sprintf(ThemeName, "%s Crown Closure", CombinedName);
								OverDensTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
								if (Abort)
									{
									return (0);
									} // if
								if (OverDensTheme || (OverDensTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
									{
									if (MadeTheme)
										{
										// change name
										OverDensTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
										} // if
									OverDensTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
									OverDensTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
									OverDensTheme->NullConstant[0] = 0.0;
									OverDensTheme->AttribFactor[0] = RelativeDensityMult;
									OverDensTheme->SetAttribute(SpDensAttr[SpFieldCt], 0);
									OverDensTheme->SetAttribute(NULL, 1);
									OverDensTheme->SetAttribute(NULL, 2);
									OverDensTheme->Condition.NewAttribute((char *)SpeciesAttr[SpFieldCt]->GetName());
									OverDensTheme->Condition.NewAttributeValue(ClassUnits[UnitNumber].PolyName);
									OverDensTheme->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
									OverDensTheme->ConditionEnabled = 1;
									} // if
								} // if

							// create size thematic map if size is in foliage group from SizeAttr[species field]
							if (SizeAttr[SpFieldCt] && Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
								{
								MadeTheme = NULL;
								if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
									sprintf(ThemeName, "%s Height", CombinedName);
								else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
									sprintf(ThemeName, "%s Diameter", CombinedName);
								else
									sprintf(ThemeName, "%s Age", CombinedName);

								OverSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
								if (Abort)
									{
									return (0);
									} // if
								if (OverSizeTheme || (OverSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
									{
									if (MadeTheme)
										{
										// change name
										OverSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
										} // if
									OverSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
									OverSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
									OverSizeTheme->NullConstant[0] = 0.0;
									OverSizeTheme->SetAttribute(SizeAttr[SpFieldCt], 0);
									OverSizeTheme->SetAttribute(NULL, 1);
									OverSizeTheme->SetAttribute(NULL, 2);
									OverSizeTheme->AttribFactor[0] = SizeThemeMult;
									} // if

								if (SizeAttr[2] && MinSizeThemePresent)
									{
									MadeTheme = NULL;
									if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
										sprintf(ThemeName, "%s Minimum Height", CombinedName);
									else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
										sprintf(ThemeName, "%s Minimum Diameter", CombinedName);
									else
										sprintf(ThemeName, "%s Minimum Age", CombinedName);
									OverMinSizeTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
									if (Abort)
										{
										return (0);
										} // if
									if (OverMinSizeTheme || (OverMinSizeTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
										{
										if (MadeTheme)
											{
											// change name
											OverMinSizeTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
											} // if
										OverMinSizeTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
										OverMinSizeTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
										OverMinSizeTheme->NullConstant[0] = 0.0;
										OverMinSizeTheme->SetAttribute(SizeAttr[2], 0);
										OverMinSizeTheme->SetAttribute(NULL, 1);
										OverMinSizeTheme->SetAttribute(NULL, 2);
										if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
											OverMinSizeTheme->AttribFactor[0] = .01;
										else
											OverMinSizeTheme->AttribFactor[0] = SizeThemeMult;
										} // if
									} // if
								} // if

							// add images
							// find max fol height
							MaxFolHt = 0.0;
							NumFoliage = 0;
							for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
								{
								if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
									{
									if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxFolHt)
										MaxFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
									NumFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
									} // if
								} // for

							if (OverGrp)
								{
								if (Overstory->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
									{
									OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
									OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);
									if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
										{
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DBH, OverSizeTheme);
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
										// set up DBH curve
										SetupDBHCurve(&OverGrp->DBHCurve);
										} // if
									else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
										{
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_AGE, OverSizeTheme);
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
										// set up Age curve
										SetupAgeCurve(&OverGrp->AgeCurve);
										} // else if
									else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
										{
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_HEIGHT, OverSizeTheme);
										OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_MINHEIGHT, OverMinSizeTheme);
										} // else
									} // if
								OverGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue((double)NumFoliage);
								// set thematic map for density
								OverGrp->SetThemePtr(WCS_FOLIAGEGRP_THEME_DENSITY, OverDensTheme);
								OverGrp->SetAnimDefaults(Overstory);
								} // if

							// so we don't divide by 0
							if (MaxFolHt < .0001)
								MaxFolHt = .0001;
							if (NumFoliage == 0)
								NumFoliage = 1;
							for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
								{
								if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
									{
									// add foliage images to library
									if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName(), 
										TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/, TRUE /*LoadnPrep*/, FALSE /*AllowImageManagement*/))
										{
										// notify of image added
										Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
										Changes[1] = NULL;
										GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
										} // if image added
									if (OverGrp)
										{
										if (CurFol = OverGrp->AddFoliage(NULL))
											{
											CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumFoliage);
											CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxFolHt);
											// add foliage to  ecotype
											CurFol->SetRaster(CurRast);
											} // if
										else
											{
											Success = 0;
											break;
											} // else
										} // if
									} // if
								else
									break;
								} // for FolCt
							} // if unit has foliage
						} // for UnitNumber
					} // for SpFieldCt
				} // if Overstory
			} // if ActiveGrad
		} // if CurEco
	} // if EcosMade < NumClassUnits

return (Success);

} // ForestWiz::ProcessFARSITESpeciesFields

/*===========================================================================*/

int ForestWiz::ProcessPointData(void)
{
int Success = 1;
NotifyTag Changes[2];

Changes[1] = NULL;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

Success = ProcessOneSpeciesFieldPointData();

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

return (Success);

} // ForestWiz::ProcessPointData

/*===========================================================================*/

int ForestWiz::ProcessOneSpeciesFieldPointData(void)
{
double MaxFolHt;
long UnitNumber, FolCt, NumFoliage;
FoliageEffect *CurFolEfct, *MadeFolEfct;
Ecotype *CurEcotp;
FoliageGroup *CurGrp;
Foliage *CurFol;
Raster *CurRast;
SearchQuery *CurSQ, *MadeSQ;
ThematicMap *CurHeightTheme = NULL, *CurMinHeightTheme = NULL, *CurDBHTheme = NULL, *MadeTheme;
DBFilterEvent *CurFilter;
int Success = 1, Queried = 0, QueryResult = 0, SQQueried = 0, SQQueryResult = 0, TMQueried = 0, TMQueryResult = 0, Abort = 0;
NotifyTag Changes[2];
char ThemeName[128];

// create an ecosystem for each class with an overstory ecotype
// populate with images
// create a search query for each ecosystem that links the ecosystem to the vectors via the species attr field
// create one thematic map for density and one for height, 
// link TM's to all vectors with height or density attribs and their values > 0
// attach TM's to overstory ecotype heights and densities

// create the density TM
// create the height TM
// create the TM Search Query
// look for TM's before creating them
if (SizeAttr[0] && NumSizeFields > 0)
	{
	MadeTheme = NULL;
	if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
		strcpy(ThemeName, "Foliage Effect Height");
	else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
		strcpy(ThemeName, "Foliage Effect Diameter");
	else
		strcpy(ThemeName, "Foliage Effect Age");
	CurHeightTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurHeightTheme || (CurHeightTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			CurHeightTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		CurHeightTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		CurHeightTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		CurHeightTheme->NullConstant[0] = 0.0;
		CurHeightTheme->SetAttribute(SizeAttr[0], 0);
		CurHeightTheme->SetAttribute(NULL, 1);
		CurHeightTheme->SetAttribute(NULL, 2);
		CurHeightTheme->AttribFactor[0] = SizeThemeMult;
		} // if
	} // if

if (SizeAttr[2] && MinSizeThemePresent)
	{
	MadeTheme = NULL;
	if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
		strcpy(ThemeName, "Foliage Effect Minimum Height");
	else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
		strcpy(ThemeName, "Foliage Effect Minimum Diameter");
	else
		strcpy(ThemeName, "Foliage Effect Minimum Age");
	CurMinHeightTheme = (ThematicMap *)FindComponent(WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeName, TMQueried, TMQueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurMinHeightTheme || (CurMinHeightTheme = MadeTheme = new ThematicMap(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeTheme)
			{
			// change name
			CurMinHeightTheme->SetUniqueName(GlobalApp->AppEffects, ThemeName);
			} // if
		CurMinHeightTheme->OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
		CurMinHeightTheme->NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_CONSTANT;
		CurMinHeightTheme->NullConstant[0] = 0.0;
		CurMinHeightTheme->SetAttribute(SizeAttr[2], 0);
		CurMinHeightTheme->SetAttribute(NULL, 1);
		CurMinHeightTheme->SetAttribute(NULL, 2);
		if (HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE || HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM)
			CurMinHeightTheme->AttribFactor[0] = .01;
		else
			CurMinHeightTheme->AttribFactor[0] = SizeThemeMult;
		} // if
	} // if
	
for (UnitNumber = 0; UnitNumber < NumClassUnits; UnitNumber ++)
	{
	MadeFolEfct = NULL;
	// add ecosystem
	// look for ecosystem already existing first
	CurFolEfct = (FoliageEffect *)FindComponent(WCS_EFFECTSSUBCLASS_FOLIAGE, ClassUnits[UnitNumber].PolyName, Queried, QueryResult, Abort, FALSE);
	if (Abort)
		{
		return (0);
		} // if
	if (CurFolEfct || (CurFolEfct = MadeFolEfct = new FoliageEffect(NULL, GlobalApp->AppEffects, NULL)))
		{
		if (MadeFolEfct)
			{
			// change name
			CurFolEfct->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
			// set ecotype
			if (CurEcotp = &CurFolEfct->Ecotp)
				{
				CurEcotp->SecondHeightType = HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? WCS_ECOTYPE_SECONDHT_RANGEPCT: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? WCS_ECOTYPE_SECONDHT_MINPCT: WCS_ECOTYPE_SECONDHT_MINABS;
				CurEcotp->AbsHeightResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;
				// set thematic maps for height
				// size
				if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER)
					{
					CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_DBH;
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_DBH, CurHeightTheme);
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
					// set up DBH curve
					SetupDBHCurve(&CurEcotp->DBHCurve);
					} // if
				else if (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_AGE)
					{
					CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_AGE;
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_AGE, CurHeightTheme);
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
					// set up Age curve
					SetupAgeCurve(&CurEcotp->AgeCurve);
					} // else if
				else // (SizeMeasurement == WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT)
					{
					CurEcotp->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_HEIGHT, CurHeightTheme);
					CurEcotp->SetThemePtr(WCS_ECOTYPE_THEME_MINHEIGHT, CurMinHeightTheme);
					} // else

				CurEcotp->SetAnimDefaults();

				// add fol grp
				if (CurGrp = CurEcotp->AddFoliageGroup(NULL, ClassUnits[UnitNumber].PolyName))
					{
					// find max fol height
					MaxFolHt = 0.0;
					NumFoliage = 0;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							if (ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] > MaxFolHt)
								MaxFolHt = ClassUnits[UnitNumber].Groups[0].FolHt[FolCt];
							NumFoliage += ClassUnits[UnitNumber].Groups[0].FolCount[FolCt];
							} // if
						} // for
					CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetValue(MaxFolHt);
					CurEcotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(HeightType == WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE ? AvgHeightRange: HeightType == WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM ? MinHeightRange: MinHeightAbs);

					// so we don't divide by 0
					if (MaxFolHt < .0001)
						MaxFolHt = .0001;
					if (NumFoliage == 0)
						NumFoliage = 1;
					for (FolCt = 0; FolCt < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; FolCt ++)
						{
						if (ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName()[0])
							{
							if (CurFol = CurGrp->AddFoliage(NULL))
								{
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetValue((double)ClassUnits[UnitNumber].Groups[0].FolCount[FolCt] / NumFoliage);
								CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetValue(ClassUnits[UnitNumber].Groups[0].FolHt[FolCt] / MaxFolHt);
								// add foliage images to library
								if (CurRast = GlobalApp->AppImages->AddRaster((char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetPath(), (char *)ClassUnits[UnitNumber].Groups[0].FolNames[FolCt].GetName(), 
									TRUE /*NonRedundant*/, TRUE /*ConfirmFile*/, TRUE /*LoadnPrep*/, FALSE /*AllowImageManagement*/))
									{
									// notify of image added
									Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
									Changes[1] = NULL;
									GlobalApp->AppEx->GenerateNotify(Changes, CurRast);
									// add foliage to  ecotype
									CurFol->SetRaster(CurRast);
									} // if image added
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						else
							break;
						} // for
					} // if
				else
					{
					Success = 0;
					break;
					} // else
				} // if
			else
				{
				Success = 0;
				break;
				} // else
			} // if
		MadeSQ = NULL;
		// add search query
		// look for search query already existing first
		CurSQ = (SearchQuery *)FindComponent(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ClassUnits[UnitNumber].PolyName, SQQueried, SQQueryResult, Abort, FALSE);
		if (Abort)
			{
			return (0);
			} // if
		if (CurSQ || (CurSQ = MadeSQ = new SearchQuery(NULL, GlobalApp->AppEffects, NULL)))
			{
			if (MadeSQ)
				{
				// change name
				CurSQ->SetUniqueName(GlobalApp->AppEffects, ClassUnits[UnitNumber].PolyName);
				} // if
			else
				{
				// remove filters
				while (CurSQ->Filters)
					CurSQ->RemoveFilter(CurSQ->Filters);
				} // else remove filters
			// find first filter or add one
			if ((CurFilter = CurSQ->Filters) || (CurFilter = CurSQ->AddFilter(NULL)))
				{
				CurFilter->PassControlPt = CurFilter->PassDEM = CurFilter->PassDisabled = 0;
				CurFilter->PassVector = CurFilter->PassEnabled = CurFilter->PassLine = CurFilter->PassPoint = 1;
				CurFilter->NewAttribute((char *)SpeciesAttr[0]->GetName());
				CurFilter->NewAttributeValue(ClassUnits[UnitNumber].PolyName);
				CurFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
				} // if
			CurFolEfct->SetQuery(CurSQ);
			} // if
		} // if
	} // for

return (Success);

} // ForestWiz::ProcessOneSpeciesFieldPointData

/*===========================================================================*/

GeneralEffect *ForestWiz::FindComponent(long CompClass, char *CompName, int &Queried, int &QueryResult, int &Abort, int ForceRemoval)
{
GeneralEffect *CurComp;
char MesgStr[128];

if (ForceRemoval)
	{
	Queried = 1;
	QueryResult = 2;
	} // if

Abort = 0;
if (CurComp = GlobalApp->AppEffects->FindByName(CompClass, CompName))
	{
	if (! Queried)
		{
		sprintf(MesgStr, "%s already exist. Do you wish to use them, replace them or create a new ones?", GlobalApp->AppEffects->GetEffectTypeName(CompClass));
		QueryResult = UserMessageCustomQuad(CompName, MesgStr, "Use Them", "Replace Them", "Create New", "Cancel", 0);
		Queried = 1;
		} // if
	if (QueryResult == 1)
		{
		// use it
		} // if
	else if (QueryResult == 2)
		{
		// replace it
		GlobalApp->AppEffects->RemoveRAHost(CurComp, TRUE);
		CurComp = NULL;
		} // else if
	else if (QueryResult == 3)
		{
		// create new
		CurComp = NULL;
		} // else if
	else	// cancel
		Abort = 1;
	} // if

return (CurComp);

} // ForestWiz::FindComponent

/*===========================================================================*/

void ForestWiz::SetupDBHCurve(AnimDoubleCurve *ADC)
{
GraphNode *Node;

ADC->SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADC->ReleaseGraph(0);

ADC->AddNode(0.0, 0.0, 0.0);
ADC->AddNode(DBHEquivUnits[0], DBHEquiv[0] * HeightMult, 0.0);
ADC->AddNode(DBHEquivUnits[1], DBHEquiv[1] * HeightMult, 0.0);
ADC->AddNode(DBHEquivUnits[2], DBHEquiv[2] * HeightMult, 0.0);
if (Node = ADC->AddNode(DBHEquivUnits[3], DBHEquiv[3] * HeightMult, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if
ADC->ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // ForestWiz::SetupDBHCurve

/*===========================================================================*/

void ForestWiz::SetupAgeCurve(AnimDoubleCurve *ADC)
{
GraphNode *Node;

ADC->SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADC->ReleaseGraph(0);

ADC->AddNode(0.0, 0.0, 0.0);
ADC->AddNode(10.0, AgeEquiv[0] * HeightMult, 0.0);
ADC->AddNode(25.0, AgeEquiv[1] * HeightMult, 0.0);
ADC->AddNode(50.0, AgeEquiv[2] * HeightMult, 0.0);
if (Node = ADC->AddNode(100.0, AgeEquiv[3] * HeightMult, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if
ADC->ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // ForestWiz::SetupAgeCurve

/*===========================================================================*/
/*===========================================================================*/

ForestWizEcoData::ForestWizEcoData()
{
long Ct;

HasFoliage = 0;
GroundTexture = WCS_FORESTWIZECODATA_GROUNDTEX_EARTH;

for (Ct = 0; Ct < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE; Ct ++)
	{
	FolHt[Ct] = 10.0;
	FolCount[Ct] = 100;
	FolNames[Ct].SetPath(GlobalApp->MainProj->imagepath);
	} // for

} // ForestWizEcoData::ForestWizEcoData

/*===========================================================================*/

void ForestWizEcoData::Copy(ForestWizEcoData *CopyFrom)
{
long Ct;

HasFoliage = CopyFrom->HasFoliage;
GroundTexture = CopyFrom->GroundTexture;

for (Ct = 0; Ct < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE; Ct ++)
	{
	if (CopyFrom->FolNames[Ct].GetName()[0])
		{
		FolNames[Ct].Copy(&FolNames[Ct], &CopyFrom->FolNames[Ct]);
		FolCount[Ct] = CopyFrom->FolCount[Ct];
		FolHt[Ct] = CopyFrom->FolHt[Ct];
		} // if
	else
		{
		FolNames[Ct].SetPath(GlobalApp->MainProj->imagepath);
		FolNames[Ct].SetName("");
		FolCount[Ct] = 100;
		FolHt[Ct] = 10.0;
		} // else
	} // for

} // ForestWizEcoData::Copy

/*===========================================================================*/

long ForestWizEcoData::CountFoliageNames(void)
{
long Ct, SumNames = 0;

for (Ct = 0; Ct < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE; Ct ++)
	{
	if (FolNames[Ct].GetName()[0])
		{
		SumNames ++;
		} // if
	else
		break;
	} // for

return (SumNames);

} // ForestWizEcoData::CountFoliageNames

/*===========================================================================*/
/*===========================================================================*/

ForestWizClassData::ForestWizClassData()
{

NumGroups = 1;
HasFoliage = 0;
GroundTexture = WCS_FORESTWIZECODATA_GROUNDTEX_EARTH;
PolyName[0] = 0;
OverUnder = 0;

} // ForestWizClassData::ForestWizClassData

/*===========================================================================*/

void ForestWizClassData::Copy(ForestWizClassData *CopyFrom, int VegetationExists)
{
long Ct, GrpCt;

HasFoliage = CopyFrom->HasFoliage;
GroundTexture = CopyFrom->GroundTexture;
NumGroups = CopyFrom->NumGroups;
strcpy(PolyName, CopyFrom->PolyName);
OverUnder = CopyFrom->OverUnder;

for (GrpCt = 0; GrpCt < CopyFrom->NumGroups; GrpCt ++)
	{
	for (Ct = 0; Ct < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; Ct ++)
		{
		if (VegetationExists && CopyFrom->Groups[GrpCt].FolNames[Ct].GetName()[0])
			{
			Groups[GrpCt].FolNames[Ct].Copy(&Groups[GrpCt].FolNames[Ct], &CopyFrom->Groups[GrpCt].FolNames[Ct]);
			Groups[GrpCt].FolCount[Ct] = CopyFrom->Groups[GrpCt].FolCount[Ct];
			Groups[GrpCt].FolHt[Ct] = CopyFrom->Groups[GrpCt].FolHt[Ct];
			} // if
		else
			{
			Groups[GrpCt].FolNames[Ct].SetPath(GlobalApp->MainProj->imagepath);
			Groups[GrpCt].FolNames[Ct].SetName("");
			Groups[GrpCt].FolCount[Ct] = 100;
			Groups[GrpCt].FolHt[Ct] = 10.0;
			} // else
		} // for
	} // for
for ( ; GrpCt < WCS_FORESTWIZCLASSDATA_MAXNUMGROUPS; GrpCt ++)
	{
	for (Ct = 0; Ct < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; Ct ++)
		{
		Groups[GrpCt].FolNames[Ct].SetPath(GlobalApp->MainProj->imagepath);
		Groups[GrpCt].FolNames[Ct].SetName("");
		Groups[GrpCt].FolCount[Ct] = 100;
		Groups[GrpCt].FolHt[Ct] = 10.0;
		} // for
	} // for

} // ForestWizClassData::Copy

/*===========================================================================*/
/*===========================================================================*/

ForestWizClassGroupData::ForestWizClassGroupData()
{
long Ct;

GroupName[0] = 0;
for (Ct = 0; Ct < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; Ct ++)
	{
	FolHt[Ct] = 10.0;
	FolCount[Ct] = 100;
	FolNames[Ct].SetPath(GlobalApp->MainProj->imagepath);
	} // for

} // ForestWizClassGroupData::ForestWizClassGroupData

/*===========================================================================*/

long ForestWizClassGroupData::CountFoliageNames(void)
{
long Ct, SumNames = 0;

for (Ct = 0; Ct < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE; Ct ++)
	{
	if (FolNames[Ct].GetName()[0])
		{
		SumNames ++;
		} // if
	else
		break;
	} // for

return (SumNames);

} // ForestWizClassGroupData::CountFoliageNames
