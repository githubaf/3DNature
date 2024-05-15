/* nngridr.c
** Adapted from code by David F. Watson.
** Modified and Incorporated into World Construction Set in June, 1995 by GRH
** by permission of the author.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"

/* The DEM creation and gridding process

From Map View Menu select Make DEM.

  Make DEM window opens offering the following options:
	Set current elevation value with slider or string
	Set limit values for slider
	Set minimum distance between points for drawing mode - in the MP
		structure there will be a Last_X and Last_Y value set every
		time a new control point is digitized.
	Load an XYZ file of control points. File could be in XYZ or DXF format.
		Data could be in Lat/Lon or UTM projection.
	Save the control points as an XYZ file or DXF file in UTM or Lat/Lon.
	Import control points from Database objects. Only selected Objects
		would be used. These Objects would have to have an elevation
		embedded in either their Name or Label fields. The elevation
		would be extracted and the XY coordinates would be taken
		from the lat/lon locations of the Objects' vertices.
		This option supports the use of contours and streams digitized
		in Map View. Note: For digitizing streams where there is
		no elevation control it would be useful to have an Object
		Profile Elevation editor so gradients can be created.
	Drawing modes - for line drawing.
		Isoline - All digitized points fall on the defined elevation
		Gradient - Points constructed in a single drawing operation
			will be distributed along an elevation profile. The
			gradient will be the Elevation Slider range from the
			current elevation to the lower limit.
			Gradient can be linear or non-linear to simulate
			a stream that flattens along its course.
	Point modes - Points can be added, moved or deleted by clicking on
		them or clicking and dragging them.
	Gaussian displacement - Points can be vertically displaced according
		to a Gaussian distribution around the defined elevation.
		When using the gradient drawing mode the gradient will vary
		in slope but elevations will always be decreasing.
	Elevation Source - from slider and current elevation string or from
		underlying DEM if there is one.
	Open Grid Window		

  While the Make DEM window is open you can:
	Digitize control points to your heart's content.
	Load files of control points.
	Save files of control points.
	Convert	contour and other Database objects to control points.
	Grid the data.

  	Elevation control points can be added by clicking or drawing with the
		mouse while a control key is held on the keyboard. The
		control key allows other Map View mouse operations to be valid
		such as clicking on a point to identify an object or moving
		the mouse to determine the elevation from an existing DEM.
  	The elevation at control points can be determined from:
		The value set by the Elevation Slider OR
		The elevation of underlying DEMs - value taken from the slider
			if outside the area of loaded DEMs

Selecting the Grid button in the Make DEM window opens the Natural Neighbor
	(NN) Grid window.

  An NNGrid structure is allocated at this time with all the "global"
	variables used by the nngridr functions. The nngridr code does not
	load any files it simply executes the gridding based on a set of
	data passed to it from the Make DEM window and the parameters
	established prior to gridding in the NN Grid window.

  In the NN Grid window you can:
	Load a set of gridding parameters from a file.
	Save the current gridding parameters to a file.
	Restore the default gridding parameters created when the window
		opened.
	Set the following parameters:
		Data type - function, choropleth
		Choropleth data mapped as density
		Use gradients in estimation
		Number of output nodes in x and y dimensions
		Disallow negative output values
		Extrapolate values beyond convex hull
		Data is in which hemisphere
		Output data in north to south or south to north direction
			note: this may not be useful
		Tautness parameters #1 & #2
		Scaling factors for x, y, z
			note: these maybe should be computed automatically
			when they are necessary
		High and Low latitude and longitude
			note: these will be supplied when the window opens
			based on the dimensions of the input data but can
			be changed as desired.
		Overlap between rows and columns
			note: I don't know whether this is useful
		Null data value for areas outside the convex hull when
			extrapolation is disabled.
		Fractal level - gridded data can be interpolated using
			Brownian motion fractal interpolation to an additional
			level of detail corresponding to 2, 4, 8, 16, 32...
			times the lineal resolution of the primary gridded data.
		Standard Deviation - for use in fractalization. Larger numbers
			result in greater amplitude deviation in the output
			model.
		Seed for the random number generator used in fractalization.
		Fractal dimension - affects the randomness of the output if
			fractalization is enabled.
		Output Object name

 Output format will be a WCS DEM file with addition of the object to the
	Database. If you wish to change the format that can be done in the
	DEM Convert window in the Data Ops Module. In fact it would be a
	good idea to support output in XYZ, .grd, and .bil formats.

 Selecting the Grid button in the NN Grid window will initiate gridding.
	There will be progress gauges that can be cancelled at any time.
	While gridding is in progress you have the option to display the
	map as it progresses in the Map View window. If	gridding is cancelled
	before the operation is complete, the temporary	elevation file created
	during gridding will be deleted but the Obj and elev files from
	previous gridding operations will not be touched.


Global Models:
  Since creating an entire globe introduces some unique constraints the
    following points should be kept in mind when creating global models:
	Major topographic features should be confined to regions well
		away from the polar regions and from longitudes close to the
		seam meridian. This will prevent features from being distended
		due to forshortening close to the poles and from being
		truncated unnaturally at the seam.
	A series of control points should be created along the edges of the map
		(later to become the poles and seam meridian) that are of
		a constant elevation so that there are no joints or
		discontinuities at poles or seam meridian.
		This could be an option to have WCS set these for the user.
		It could also be accomplished by prohibiting extrapolation
		outside the convex hull and having a null data value that
		would be applied in these areas. This might not be quite
		foolproof however in that discontinuities could arise at the
		edges of the convex hull.

Additional terrain modification or editing could be done with the Terrain
  Editor function which hopefully will be written soon. Such things as
  adding craters or lakes or craggy spires or smoothing the edges of terrain
  flat spots could be done in the Editor better than in the DEM Maker.

*/



STATIC_FCN void NNGrid_Del(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
STATIC_FCN short NNGridr_OptInit(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
STATIC_FCN void NNGrid_Init(struct NNGrid *NNG); // used locally only -> static, AF 26.7.2021
STATIC_FCN struct NNGrid *NNGrid_New(void); // used locally only -> static, AF 26.7.2021


short nngridr(void)
{
short success = 0, NewNNG = 0;
struct NNGrid *NNG;

 if (GR_Win)
  {
  NNG = &GR_Win->NNG;
  if (NNG->Grid)
   free_Memory(NNG->Grid, NNG->GridSize);
  NNG->Grid = NULL;
  } /* if */
 else
  {
  if ((NNG = NNGrid_New()))
   NewNNG = 1;
  } /* else */

 if (NNG)
  {
  NNGrid_Init(NNG);
  if (NNGridr_OptInit(NNG))
      {
          ;
      }
   {
   if (NNGrid_DataInit(NNG))
    {
    if (NNG->ichoro)
     {
     if (! ChoroPleth(NNG))
      goto EndGrid;
     } /* if choropleth data */
    if (NNG->igrad)
     {
     if (! Gradient(NNG))
      goto EndGrid;
     } /* if use gradients */
    if (MakeGrid(NNG, MD_Win->Units))
     success = 1;
    } /* if data initialized OK */
   } /* if option initialization OK */
EndGrid:
  if (NewNNG)
   NNGrid_Del(NNG);
  } /* if NNGrid structure allocated */
 else
  User_Message(GetString( MSG_NNGRIDR_MAPVIEWGRIDDEM ),                // "Map View: Grid DEM"
               GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                             // "OK"
               (CONST_STRPTR)"o");

 return (success);

} /* nngridr() */

/************************************************************************/

STATIC_FCN struct NNGrid *NNGrid_New(void) // used locally only -> static, AF 26.7.2021
{

 return ((struct NNGrid *)get_Memory(sizeof (struct NNGrid), MEMF_CLEAR));

} /* NNGrid_New() */

/*************************************************************************/

STATIC_FCN void NNGrid_Init(struct NNGrid *NNG) // used locally only -> static, AF 26.7.2021
{

  NNG->magx = NNG->magy = NNG->magz = 1.0;
  NNG->scor[0][0] = NNG->scor[2][1] = 1;
  NNG->scor[0][1] = NNG->scor[1][0] = 2;
  NNG->scor[1][1] = NNG->scor[2][0] = 0;

} /* NNGrid_Init() */

/*************************************************************************/

STATIC_FCN void NNGrid_Del(struct NNGrid *NNG) // used locally only -> static, AF 26.7.2021
{

 if (NNG)
  {
  Datum_Del(NNG->rootdat);
  Simp_Del(NNG->rootsimp);
  Temp_Del(NNG->roottemp);
  Neig_Del(NNG->rootneig);
  FreeVecti(NNG->jndx, NNG->IntVectCols);
  FreeMatrixd(NNG->points, NNG->PointMatxRows, NNG->PointMatxCols);
  FreeMatrixd(NNG->joints, NNG->JointMatxRows, NNG->JointMatxCols);
  if (NNG->Grid)
   free_Memory(NNG->Grid, NNG->GridSize);
  free_Memory(NNG, sizeof (struct NNGrid));
  } /* if */

} /* NNGrid_Del() */

/*************************************************************************/

STATIC_FCN short NNGridr_OptInit(struct NNGrid *NNG) // used locally only -> static, AF 26.7.2021
{
char *floatdata, *OutFile;
long data;

/* output file and path */
 get(GR_Win->Str[0], MUIA_String_Contents, &OutFile);

/* drop in a user_message here */
 if (! OutFile[0])
  return (0);

 strcpy(NNG->grd_file, OutFile);

/* use gradient estimation */
 get(GR_Win->Check[0], MUIA_Selected, &data);
 NNG->igrad = data;
/* disallow negative values in output */ 
 get(GR_Win->Check[1], MUIA_Selected, &data);
 NNG->non_neg = data;
/* data is choropleth data */ 
 get(GR_Win->Check[2], MUIA_Selected, &data);
 NNG->ichoro = data;
/* choropleth is density data */ 
 get(GR_Win->Check[3], MUIA_Selected, &data);
 NNG->densi = data;
/* extrapolate values beyond convex hull */ 
 get(GR_Win->Check[4], MUIA_Selected, &data);
 NNG->extrap = data;
/* data is in southern hemisphere */
 get(GR_Win->Check[5], MUIA_Selected, &data);
 NNG->southhemi = data;

/* columns and rows for output */
 get(GR_Win->IntStr[0], MUIA_String_Integer, &data);
 NNG->x_nodes = data; 
 get(GR_Win->IntStr[1], MUIA_String_Integer, &data);
 NNG->y_nodes = data; 

/* drop in a user_message here */
 if (NNG->x_nodes * NNG->y_nodes == 0)
  return (0);

/* tautness parameters for gradient interpretation */
 get(GR_Win->FloatStr[6], MUIA_String_Contents, &floatdata);
 NNG->bI = atof(floatdata); 
 get(GR_Win->FloatStr[7], MUIA_String_Contents, &floatdata);
 NNG->bJ = atof(floatdata); 

/* scaling parameters */
 get(GR_Win->FloatStr[9], MUIA_String_Contents, &floatdata);
 NNG->magx = atof(floatdata);
 get(GR_Win->FloatStr[10], MUIA_String_Contents, &floatdata);
 NNG->magy = atof(floatdata);
 get(GR_Win->FloatStr[11], MUIA_String_Contents, &floatdata);
 NNG->magz = atof(floatdata);

/* reverse east and west edges and change signs for nngridr convention */
 get(GR_Win->FloatStr[3], MUIA_String_Contents, &floatdata);
 NNG->xstart = -atof(floatdata);	/* west */
 get(GR_Win->FloatStr[2], MUIA_String_Contents, &floatdata);
 NNG->xterm = -atof(floatdata);		/* east */
 get(GR_Win->FloatStr[1], MUIA_String_Contents, &floatdata);
 NNG->ystart = atof(floatdata);		/* south */
 get(GR_Win->FloatStr[0], MUIA_String_Contents, &floatdata);
 NNG->yterm = atof(floatdata);		/* north */

/* overlap between grid cells */
 get(GR_Win->FloatStr[4], MUIA_String_Contents, &floatdata);
 NNG->horilap = atof(floatdata);
 get(GR_Win->FloatStr[5], MUIA_String_Contents, &floatdata);
 NNG->vertlap = atof(floatdata);

/* null data value for areas outside convex hull if extrapolate is disabled */
 get(GR_Win->FloatStr[8], MUIA_String_Contents, &floatdata);
 NNG->nuldat = atof(floatdata);

 NNG->arriba = -1.0; 

 NNG->updir = (int) NNG->arriba;
 if (NNG->magx EQ 1) NNG->ixmag = 0;
 else NNG->ixmag = 1;
 if (NNG->magy EQ 1) NNG->iymag = 0;
 else NNG->iymag = 1;
 if (NNG->magz EQ 1) NNG->izmag = 0;
 else NNG->izmag = 1;
 if (NNG->ixmag OR NNG->iymag OR NNG->izmag) NNG->imag = 1;
 NNG->xspace = (NNG->xterm - NNG->xstart) / (NNG->x_nodes - 1);
 NNG->yspace = (NNG->yterm - NNG->ystart) / (NNG->y_nodes - 1);

 return (1);

} /* NNGrid_OptInit() */

/*************************************************************************/
#ifdef HJGJHDGJSGHJDGS
// Obsolete - options now come from NNGrid window and are set with NNGrid_Init()

void GetOptions(struct NNGrid *NNG)
{
   FILE *filep;
   int badfile = 0;

/* temp values - not used by WCS */
   int tgrid, z_only, gnup, sdip, rads, optim, igif, idisplay,
	x_extra, y_extra;

   if ((filep = fopen("nngridr.ini","rt")) NE NULL)
   {  if (fscanf(filep, "%s", NNG->dat_file) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%s", NNG->grd_file) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->igrad) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->ichoro) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->densi) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &tgrid) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->x_nodes) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->y_nodes) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->non_neg) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &z_only) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &gnup) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &sdip) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &rads) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &optim) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->arriba) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->bI) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->bJ) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->magx) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->magy) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->magz) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->xstart) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->ystart) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->xterm) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->yterm) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->horilap) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->vertlap) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%lf", &NNG->nuldat) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->extrap) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &igif) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &idisplay) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &x_extra) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &y_extra) EQ EOF) 
         badfile = 1;
      if (fscanf(filep, "%d", &NNG->southhemi) EQ EOF) 
         badfile = 1;

      if (! badfile) NNG->ioK = 1;
      fclose(filep);
   }
   else badfile = 1;
   if (badfile)
   {
      strcpy(NNG->dat_file, "???????????");
      strcpy(NNG->grd_file, "scratch.grd");
      NNG->igrad = NNG->ichoro = NNG->densi = NNG->non_neg = 0;
      NNG->arriba = NNG->magx = NNG->magy = NNG->magz = 1;
      NNG->bI = 1.5;
      NNG->bJ = 7.0;
      NNG->xstart = NNG->ystart = NNG->horilap = NNG->vertlap   = 1.0;
      NNG->xterm = NNG->yterm = 10.0;
      NNG->x_nodes = NNG->y_nodes   = 5;
   }
   NNG->updir = (int) NNG->arriba; /* removed 'if (!extrap)' 8 July 94 */
   if (NNG->magx EQ 1) NNG->ixmag = 0;
   else NNG->ixmag = 1;
   if (NNG->magy EQ 1) NNG->iymag = 0;
   else NNG->iymag = 1;
   if (NNG->magz EQ 1) NNG->izmag = 0;
   else NNG->izmag = 1;
   if (NNG->ixmag OR NNG->iymag OR NNG->izmag) NNG->imag = 1;
   NNG->xspace = (NNG->xterm - NNG->xstart) / (NNG->x_nodes - 1);
   NNG->yspace = (NNG->yterm - NNG->ystart) / (NNG->y_nodes - 1);

} /* GetOptions() */
#endif
