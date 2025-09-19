/* LineSupport.c (ne gislinesupport.c 14 Jan 1994 CXH)
** Line drawing functions and support code
** Built from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson
** original code by Gary R. Huber
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"

STATIC_VAR UBYTE ptred,ptgreen,ptblue;

void FadeLine(long el)
{

 if (settings.linefade)
  {
  fade = (qqq - PARC_RNDR_MOTION(20)) / PARC_RNDR_MOTION(21);
  if (fade > 1.0) fade = 1.0;
  else if (fade < 0.0) fade = 0.0;
  if (fogrange == 0.0) fog = 0.0;
  else
   {
   fog = (el - PARC_RNDR_MOTION(23)) / fogrange;
   if (fog > 1.0) fog = 1.0;
   else if (fog < 0.0) fog = 0.0;
   } /* else */
  aliasred=altred;
  aliasred +=(PARC_RNDR_COLOR(2, 0))*fade;
  aliasred +=(PARC_RNDR_COLOR(2, 0))*fog;
  aliasgreen=altgreen;
  aliasgreen +=(PARC_RNDR_COLOR(2, 1)-aliasgreen)*fade;
  aliasgreen +=(PARC_RNDR_COLOR(2, 1)-aliasgreen)*fog;
  aliasblue=altblue;
  aliasblue +=(PARC_RNDR_COLOR(2, 2)-aliasblue)*fade;
  aliasblue +=(PARC_RNDR_COLOR(2, 2)-aliasblue)*fog;
  aliasred *=redsun;
  aliasgreen *=greensun;
  aliasblue *=bluesun;
  if (aliasred>255) red=255;
  else red = (aliasred<0) ? 0:aliasred;
  if (aliasgreen>255) green=255;
  else  green = aliasgreen<0 ? 0:aliasgreen;
  if (aliasblue>255) blue=255;
  else blue = aliasblue<0 ? 0:aliasblue;
  } /* if fade lines */
 else
  {
  red=altred;
  green=altgreen;
  blue=altblue;
  } /* else no fade */

} /* linefade() */

/***********************************************************************/

void EndDrawLine(struct Window *win, short zbufplot, short Color)
{
 long a, b, x, y, zip, scrnrow;
 double m;

 if (yy[1] < yy[0])
  {
  swmem(&xx[0], &xx[1], 4);
  swmem(&yy[0], &yy[1], 4);
  } /* if */
 if (yy[1] == yy[0]) m = 0.0;
 else m = ((float)xx[1] - xx[0]) / (yy[1] - yy[0]);
 for (a=0; a<DBase[OBN].LineWidth; a++)
  {
  for (b=0; b<DBase[OBN].LineWidth; b++)
   {
   scrnrow = b + yy[0] - 1;
   for (y=0; y<=yy[1]-yy[0]; y++)
    {
    scrnrow ++;
    if (scrnrow > high) break;
    if (scrnrow < 0) continue;
    x = m * y + xx[0] + a;
    if (x < 0 || x > wide) continue;
    zip = scrnrowzip[scrnrow] + x;
    if (zbufplot)
     {
     if (qqq < *(zbuf + zip))
      {
      if (render & 0x01)
       {
       *(bitmap[0] + zip) = red;
       *(bitmap[1] + zip) = green;
       *(bitmap[2] + zip) = blue;
       } /* if */
      if (render & 0x10)
       {
       if (render & 0x01)
        {
        ScreenPixelPlot(win, bitmap, x + drawoffsetX, scrnrow + drawoffsetY, zip);
	} /* if render to bitmaps */
       else
        NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, scrnrow + drawoffsetY);
       } /* if */
      *(zbuf + zip) = qqq;
      *(bytemap + zip) = 100;
      } /* if */
     else if (qqq < *(zbuf + zip) + .005)
      {
      if (render & 0x01)
       {
       *(bitmap[0] + zip) = ((UBYTE)*(bitmap[0] + zip) + red) / 2;
       *(bitmap[1] + zip) = ((UBYTE)*(bitmap[1] + zip) + green) / 2;
       *(bitmap[2] + zip) = ((UBYTE)*(bitmap[2] + zip) + blue) / 2;
       } /* if */
      if (render & 0x10)
       {
       if (render & 0x01)
        {
        ScreenPixelPlot(win, bitmap, x + drawoffsetX, scrnrow + drawoffsetY, zip);
	} /* if render to bitmaps */
       else
        NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, scrnrow + drawoffsetY);
       } /* if */
      *(zbuf + zip) = qqq;
      *(bytemap + zip) = 100;
      } /* else if */
     } /* if zbufplot */
    else
     {
     NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, scrnrow + drawoffsetY);
     } /* else no z buffer */
    } /* for y=0... */
   } /* for b=0... */
  } /* for a=0... */
 if (xx[1] < xx[0])
  {
  swmem(&xx[0], &xx[1], 4);
  swmem(&yy[0], &yy[1], 4);
  } /* if */
 if (xx[1] == xx[0]) m = 0.0;
 else m = ((float)yy[1] - yy[0]) / (xx[1] - xx[0]);
 for (a=0; a<DBase[OBN].LineWidth; a++)
  {
  for (b=0; b<DBase[OBN].LineWidth; b++)
   {
   scrnrow = b + xx[0] - 1;
   for (x=0; x<=xx[1]-xx[0]; x++)
    {
    scrnrow ++;
    if (scrnrow > wide) break;
    if (scrnrow < 0) continue;
    y = m * x + yy[0] + a;
    if (y < 0 || y >= high) continue;
    zip = scrnrowzip[y] + scrnrow;
    if (zbufplot)
     {
     if (qqq < *(zbuf + zip))
      {
      if (render & 0x01)
       {
       *(bitmap[0] + zip) = red;
       *(bitmap[1] + zip) = green;
       *(bitmap[2] + zip) = blue;
       } /* if */
      if (render & 0x10)
       {
       if (render & 0x01)
        {
        ScreenPixelPlot(win, bitmap, scrnrow + drawoffsetX, y + drawoffsetY, zip);
	} /* if render to bitmaps */
       else
        NoDitherScreenPixelPlot(win, Color, scrnrow + drawoffsetX, y + drawoffsetY);
       } /* if */
      *(zbuf + zip) = qqq;
      *(bytemap + zip) = 100;
      } /* if */
     else if (qqq < *(zbuf + zip) + .005)
      {
      if (render & 0x01)
       {
       *(bitmap[0] + zip) = ((UBYTE) * (bitmap[0] + zip) + red) / 2;
       *(bitmap[1] + zip) = ((UBYTE) * (bitmap[1] + zip) + green) / 2;
       *(bitmap[2] + zip) = ((UBYTE) * (bitmap[2] + zip) + blue) / 2;
       } /* if */
      if (render & 0x10)
       {
       if (render & 0x01)
        {
        ScreenPixelPlot(win, bitmap, scrnrow + drawoffsetX, y + drawoffsetY, zip);
	} /* if render to bitmaps */
       else
        NoDitherScreenPixelPlot(win, Color, scrnrow + drawoffsetX, y + drawoffsetY);
       } /* if */
      *(zbuf + zip) = qqq;
      *(bytemap + zip) = 100;
      } /* else if */
     } /* if zbufplot */
    else
     {
     NoDitherScreenPixelPlot(win, Color, scrnrow + drawoffsetX, y + drawoffsetY);
     } /* else no z buffer */
    } /* for x=0... */
   } /* for b=0... */
  } /* for a=0... */

} /* EndDrawLine() */

/************************************************************************/

void EndDrawPoint(struct Window *win, short zbufplot, short Color)
{
 long a, b, x, y, zip;

 for (a=0; a<DBase[OBN].LineWidth; a++)
  {
  for (b=0; b<DBase[OBN].LineWidth; b++)
   {
   y = b + yy[0] - 1;
   if (y > high) break;
   if (y < 0) continue;
   x = xx[0] + a;
   if (x < 0 || x > wide) continue;
   zip = scrnrowzip[y] + x;
   ptred =  ((UBYTE)*(bitmap[0] + zip)) * transpar + (1.0 - transpar) * red;
   ptgreen =((UBYTE)*(bitmap[1] + zip)) * transpar + (1.0 - transpar) * green;
   ptblue = ((UBYTE)*(bitmap[2] + zip)) * transpar + (1.0 - transpar) * blue;
   if (zbufplot)
    {
    if (qqq < *(zbuf + zip))
     {
     if (render & 0x01)
      {
      *(bitmap[0] + zip) = ptred;
      *(bitmap[1] + zip) = ptgreen;
      *(bitmap[2] + zip) = ptblue;
      } /* if */
     if (render & 0x10)
      {
      if (render & 0x01)
       ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
      else
       NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, y + drawoffsetY);
      } /* if */
     *(zbuf + zip) = qqq;
     *(bytemap + zip) = 100;
     } /* if */
    else if (qqq < *(zbuf + zip) + .005)
     {
     if (render & 0x01)
      {
      *(bitmap[0] + zip) = ((UBYTE)*(bitmap[0] + zip) + ptred) / 2;
      *(bitmap[1] + zip) = ((UBYTE)*(bitmap[1] + zip) + ptgreen) / 2;
      *(bitmap[2] + zip) = ((UBYTE)*(bitmap[2] + zip) + ptblue) / 2;
      } /* if */
     if (render & 0x10)
      {
      if (render & 0x01)
       ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
      else
       NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, y + drawoffsetY);
      } /* if */
     *(zbuf + zip) = qqq;
     *(bytemap + zip) = 100;
     } /* else if */
    } /* if zbufplot */
   else
    {
    NoDitherScreenPixelPlot(win, Color, x + drawoffsetX, y + drawoffsetY);
    } /* else no z buffer */
   } /* for b=0... */
  } /* for a=0... */

} /* EndDrawPoint() */

/***********************************************************************/

short writelinefile(struct elmapheaderV101 *map, long mode)
{
 short error=0;
 long i, z, Lr, Lc;

 switch (mode)
  {
  case 0:
   {
   fprintf(fvector,"%s %s %d %d %d\n", DBase[OBN].Name, DBase[OBN].Layer2,
      DBase[OBN].Points, DBase[OBN].LineWidth, DBase[OBN].Color);
   for (i=1; i<=DBase[OBN].Points && error==0; i++)
    {
    if (fprintf(fvector, "%ld %ld\n",
	(long)*(map->scrnptrx + i), (long)*(map->scrnptry + i)) < 0)
     error = 1;
    } /* for i=1... */ 
   break;
   } /* case 0: vector object */
  case 1:
   {
   for (Lr=0; Lr<=map->rows && error==0; Lr+=settings.gridsize)
    {
    if (fprintf(fvector,"%s %s %ld %d %hu\n", DBase[OBN].Name, DBase[OBN].Layer2,
	map->columns, DBase[OBN].LineWidth, DBase[OBN].Color) < 0)
     error = 1;
    for (z=1; z<map->columns && error==0; z++)
     {
     map->facept[0] = z + Lr * map->columns;
     map->facept[1] = z + 1 + Lr * map->columns;
     if (fprintf(fvector,"%d %d\n",
	*(map->scrnptrx + map->facept[0]), *(map->scrnptry + map->facept[0])) < 0)
      error = 1;
     } /* for z=1... */
    if (fprintf(fvector,"%d %d\n",
	*(map->scrnptrx + map->facept[1]), *(map->scrnptry + map->facept[1])) < 0)
     error = 1;
    } /* for Lr=0... */
   for (Lc=0; Lc<map->columns && error==0; Lc+=settings.gridsize)
    {
    if (fprintf(fvector,"%s %s %ld %d %hu\n", DBase[OBN].Name, DBase[OBN].Layer2,
       map->rows + 1, DBase[OBN].LineWidth, DBase[OBN].Color) < 0)
	error = 1;
    for (z=0; z<map->rows && error==0; z++)
     {
     map->facept[0] = Lc + z * map->columns;
     map->facept[1] = Lc + (z + 1) * map->columns;
     if (fprintf(fvector,"%d %d\n",
	*(map->scrnptrx + map->facept[0]), *(map->scrnptry + map->facept[0])) < 0)
	error = 1;
     } /* for z=0... */
    if (fprintf(fvector,"%d %d\n",
	*(map->scrnptrx + map->facept[1]), *(map->scrnptry + map->facept[1])) < 0)
	error = 1;
    } /* for Lc=1... */
   break;
   } /* case 1: terrain grid */
  } /* switch mode: 0=line objects, 1=surface grid */

 return (error);

} /* writelinefile() */

/************************************************************************/

short InitDigPerspective(void)
{
 short NewArray, NewObj;

 if ((NewObj = User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
                                GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),  // "Digitize new points for the active vector object or create a new object?"
                                GetString( MSG_GLOBAL_ACTIVENEWCANCEL ),                                     // "Active|New|Cancel"
                                (CONST_STRPTR)"anc", 1)) == 0)
  return (0);
 if (NewObj == 2)
  DBaseObject_New();

 if (! strcmp(DBase[OBN].Special, "TOP") || 
     ! strcmp(DBase[OBN].Special, "SFC"))
  {
  User_Message(GetString( MSG_LINESPRT_DIAGNOSTICDIGITIZE ),                                  // "Diagnostic: Digitize"
               GetString( MSG_MAP_ACTIVEOBJECTISADEMANDMAYNOTBEDIGITIZEDPERATIO ),  // "Active object is a DEM and may not be digitized!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                  // "OK"
               (CONST_STRPTR)"o");
  return (0);
  } /* if DEM */ 

 NewArray = DBase[OBN].Lat ? 0: 1;

 if (! allocvecarray(OBN, MAXOBJPTS, NewArray))
  {
  User_Message(GetString( MSG_LINESPRT_INTERACTIVEMODULEADDPOINTS ),    // "Interactive Module: Add Points"
               GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ), // "Out of memory!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ) ,                           // "OK"
               (CONST_STRPTR)"o");
  return (0);
  } /* if out of memory allocating point array */

 DBase[OBN].Points = 0;

/* sprintf(str, "%s Pt 1", DBase[OBN].Name);
 SetWindowTitles(win, str, (UBYTE *)(-1));
*/
 return (1);
} /* InitDigPerspective() */

/***********************************************************************/

void QuitDigPerspective(void)
{
 short error;

 DBase[OBN].Flags |= 1;	/* set modification flag */

 if (User_Message_Def((CONST_STRPTR)DBase[OBN].Name,
                      GetString( MSG_LINESPRT_SAVEOBJECTPOINTS ),  // "Save object points?"
                      GetString( MSG_GLOBAL_OKCANCEL ),          // "OK|CANCEL"
                      (CONST_STRPTR)"oc", 1))
  {
  error = saveobject(OBN, NULL, DBase[OBN].Lon, DBase[OBN].Lat, DBase[OBN].Elev);
  if (! error) error = savedbase(1);
  } /* if save object */
 
 if (! MapWind0)
  freevecarray(OBN);
 else
  DBase[OBN].Flags &= (255 ^ 1);	/* unset modification flag */

} /* QuitDigPerspective() */

/***********************************************************************/

short PerspectivePoint(short addpoint, double lat, double lon, long elevation)
{

 if (! addpoint)
  {
  if (DBase[OBN].Points > 0) DBase[OBN].Points --;
  } /* else remove point */
 else if (DBase[OBN].Points == 0)
  {
  DBase[OBN].Lat[0] = DBase[OBN].Lat[1] = lat;
  DBase[OBN].Lon[0] = DBase[OBN].Lon[1] = lon;
  DBase[OBN].Elev[0] = DBase[OBN].Elev[1] = elevation;
  DBase[OBN].Points = 1;
  } /* if new object */
 else if (DBase[OBN].Lat[DBase[OBN].Points] != lat ||
		DBase[OBN].Lon[DBase[OBN].Points] != lon)
  {
  DBase[OBN].Points ++;
  DBase[OBN].Lat[DBase[OBN].Points] = lat;
  DBase[OBN].Lon[DBase[OBN].Points] = lon;
  DBase[OBN].Elev[DBase[OBN].Points] = (short)elevation;
  } /* if addpoint */

 if (DBase[OBN].Points < MAXOBJPTS)
  {
/*  sprintf(str, "%s Pt %d", DBase[OBN].Name, DBase[OBN].Points + 1);
  SetWindowTitles(win, str, (UBYTE *)(-1));
*/
  return (1);
  } /* if not at point maximum */

/* if max pts reached close and save */
 QuitDigPerspective();
 return (0);

} /* PerspectivePoint() */

/**************************************************************************/

short VectorToPath(short item)
{
 short i, Frame, StepFrames, OpenOK = 0, Flatten = 0, TempEM = 0, OldMoItem;


 if (! dbaseloaded)
  {
  NoLoad_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),  // "Mapping Module: Path"
                 GetString( MSG_DATAOPSGUI_ADATABASE ));         // "a Database"
  return (0);
  } /* if no database */
 if (! paramsloaded)
  {
  NoLoad_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ), //  "Mapping Module: Path"
                 GetString( MSG_LINESPRT_APARAMETERFILE ));   // "a Parameter file")
  return (0);
  } /* if no database */

 if (! DBase[OBN].Lat || ! DBase[OBN].Lon)
  {
  if (Load_Object(OBN, NULL) > 0)
   {
   User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                     // "Mapping Module: Path"
                GetString( MSG_MAP_ERRORLOADINGVECTOROBJECTPERATIONTERMINATED),  // "Error loading vector object!\nOperation terminated."
                GetString( MSG_GLOBAL_OK ),                                      // "OK"
                (CONST_STRPTR)"o");
   return (0);
   } /* if load error */
  } /* if object not in memory */

 if (! item)
  {
  if (CountKeyFrames(0, 0) || CountKeyFrames(0, 1) || CountKeyFrames(0, 2))
   {
/* This seems to work OK now when TL is open.
   if (EMTL_Win)
    {
    User_Message("Mapping Module: Path",
	"Motion Time Line window must be closed for this operation. Please close and try again.",
	 "OK", "o");
    return (0);
    }
*/
   if (! User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                      GetString( MSG_LINESPRT_CAMERAKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVAL ),  // "Camera Key Frames exist. Proceeding will delete current values!"
                      GetString( MSG_MOREGUI_PROCEEDCANCEL ),                                       // "Proceed|Cancel"
                      (CONST_STRPTR)"pc"))
    return (0);
   } /* if cancel */
  } /* if camera path */
 else
  {
  if (CountKeyFrames(0, 3) || CountKeyFrames(0, 4) || CountKeyFrames(0, 5))
   {
/* This seems to work OK now when TL is open.
   if (EMTL_Win)
    {
    User_Message("Mapping Module: Path",
	"Motion Time Line window must be closed for this operation. Please close and try again.",
	 "OK", "o");
    return (0);
    }
*/
   if (! User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                      GetString( MSG_LINESPRT_FOCUSKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVALU ),  // "Focus Key Frames exist. Proceeding will delete current values!"
                      GetString( MSG_MOREGUI_PROCEEDCANCEL ) ,                                      // "Proceed|Cancel"
                      (CONST_STRPTR)"pc"))
    return (0);
   } /* if cancel */
  } /* if focus path */
  
/* determine frame step equivalent for each vector point */
 sprintf(str, "%d", 1);
 if (! GetInputString((char*)GetString( MSG_LINESPRT_ENTERFRAMEINTERVALTOREPRESENTEACHVECTORSEGMENT ),  // "Enter frame interval to represent each vector segment."
	 "+-.,abcdefghijklmnopqrstuvwxyz", str))
  return (0);
 StepFrames = atoi(str);

/* delete current key frames */
 for (i=ParHdr.KeyFrames-1; i>=0; i--)
  {
  if (KF[i].MoKey.Group == 0)
   {
   if (KF[i].MoKey.Item == item * 3)
    DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, item * 3, 0, 0);
   else if (KF[i].MoKey.Item == item * 3 + 1)
    DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, item * 3 + 1, 0, 0);
   else if (KF[i].MoKey.Item == item * 3 + 2)
    DeleteKeyFrame(KF[i].MoKey.KeyFrame, 0, item * 3 + 2, 0, 0);
   } /* if group match */
  } /* for i=0... */

 sprintf(str, "%s.elev", DBase[OBN].Name);
 if (User_Message_Def((CONST_STRPTR)str, GetString( MSG_LINESPRT_USEELEVATIONDATA ),  // "Use elevation data?"
                      GetString( MSG_GLOBAL_YESNO ),                                // "Yes|No"
                      (CONST_STRPTR)"yn", 1))
  {
  OpenOK = 1;
  Flatten = User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                             GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current flattening, datum and vertical exaggeration?"
                             GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
                             (CONST_STRPTR)"yn", 1);
  } /* if use elev */

 SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);

/* initialize parameters */
 initmopar();

/* allocate EM_Win structure if nesessary */
 if (! EM_Win)
  {
  if ((EM_Win = (struct MotionWindow *)
	get_Memory(sizeof (struct MotionWindow), MEMF_CLEAR)) == NULL)
   {
   Frame = 0;
   goto EndVec;
   } /* if out of memory */
  TempEM = 1;
  }
 OldMoItem = EM_Win->MoItem;
 EM_Win->TCB[0] = 0.0;
 EM_Win->TCB[1] = 0.0;
 EM_Win->TCB[2] = 0.0;

/* add new key frames: one for every point in vector */
 for (Frame=1; Frame<=DBase[OBN].Points; Frame++)
  {
  if (OpenOK)
   {
   if (Flatten)
    {
    PAR_FIRST_MOTION(item * 3) = DBase[OBN].Elev[Frame];
    PAR_FIRST_MOTION(item * 3) += (PARC_RNDR_MOTION(13)
	 - PAR_FIRST_MOTION(item * 3)) * PARC_RNDR_MOTION(12);
    PAR_FIRST_MOTION(item * 3) *= PARC_RNDR_MOTION(14);
    PAR_FIRST_MOTION(item * 3) *= ELSCALE_METERS;
    }
   else
    {
    PAR_FIRST_MOTION(item * 3) = DBase[OBN].Elev[Frame] * ELSCALE_METERS;
    }
   EM_Win->MoItem = item * 3;
   if (! MakeKeyFrame(Frame * StepFrames - StepFrames + 1, 0, item * 3))
    break;  
   } /* if use elevations */
  PAR_FIRST_MOTION(item * 3 + 1) = DBase[OBN].Lat[Frame];
  PAR_FIRST_MOTION(item * 3 + 2) = DBase[OBN].Lon[Frame];
  EM_Win->MoItem = item * 3 + 1;
  if (! MakeKeyFrame(Frame * StepFrames - StepFrames + 1, 0, item * 3 + 1))
   break;  
  EM_Win->MoItem = item * 3 + 2;
  if (! MakeKeyFrame(Frame * StepFrames - StepFrames + 1, 0, item * 3 + 2))
   break;  
  } /* for i=0... */

EndVec:

 EM_Win->MoItem = OldMoItem;
 if (TempEM && EM_Win)
  {
  free_Memory(EM_Win, sizeof (struct MotionWindow));
  EM_Win = NULL;
  } /* if temporary EM_Win structure created */
 ClearPointer(MapWind0);

 if (Frame <= DBase[OBN].Points)
  {
  User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                               // "Mapping Module: Path"
               GetString( MSG_LINESPRT_OUTOFMEMORYCREATINGKEYFRAMESPERATIONTERMINATED ),  // "Out of memory creating Key Frames!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                              // "OK"
               (CONST_STRPTR)"o");
  return (0);
  } /* if error */

 Par_Mod |= 0x0001;

 return (1);

} /* VectorToPath() */

/**************************************************************************/

short PathToVector(short item)
{
 short i, j, UseAll, Frames, Fr1, Fr2, Fr3, LastDoneKey, OverWrite = 0,
	Flatten;
 float TempElev;

 if (! dbaseloaded)
  {
  NoLoad_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),  // "Mapping Module: Path"
                 GetString( MSG_DATAOPSGUI_ADATABASE ));         // "a Database"
  return (0);
  } /* if no database */
 if (! paramsloaded)
  {
  NoLoad_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),  // "Mapping Module: Path"
                 GetString( MSG_LINESPRT_APARAMETERFILE ));    // "a Parameter file"
  return (0);
  } /* if no database */

 UseAll = User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                   // "Mapping Module: Path"
                       GetString( MSG_LINESPRT_USEALLSPLINEDPOINTSORONLYKEYFRAMES ),  // "Use all splined points or only Key Frames?"
                       GetString( MSG_LINESPRT_ALLSPLINEDKEYFRAMES ),                 // "All Splined|Key Frames"
         (CONST_STRPTR)"ak");
 Flatten = User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                            GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current Flattening, Datum and Vertical Exaggeration?"
                            GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
                            (CONST_STRPTR)"yn", 1);

 if (! BuildKeyTable())
  {
  User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                  // "Mapping Module: Path"
               GetString( MSG_LINESPRT_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening Key Frame table!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),  // "OK"
               (CONST_STRPTR)"o");
  } /* if no key table */

 if (UseAll)
  {
  Frames = KT_MaxFrames;
  } /* if use all points */
 else
  {
  Fr1 = CountKeyFrames(0, item * 3);
  Fr2 = CountKeyFrames(0, item * 3 + 1);
  Fr3 = CountKeyFrames(0, item * 3 + 2);
  Frames = Fr1 + Fr2 + Fr3;
  } /* else */

 if (Frames >= MAXOBJPTS)
  {
  if (! User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                     GetString( MSG_LINESPRT_THEREAREMOREFRAMESTHANALLOWABLEVECTORPOINTSPATHWIL ),  // "There are more frames than allowable vector points! Path will be truncated."
                     GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|Cancel"
                     (CONST_STRPTR)"oc"))
   return (0);
  Frames = MAXOBJPTS - 1;
  } /* if more than max allowable object points */

/* assign values to new database record */
 if (! item)
  strcpy(str, (char*)GetString( MSG_LINESPRT_CAMERAPATH ));  // "CameraPath"
 else
  strcpy(str, (char*)GetString( MSG_LINESPRT_FOCUSPATH ));   // "FocusPath"
NewName:
 if (! GetInputString((char*)GetString( MSG_LINESPRT_ENTERNAMEOFVECTORTOBECREATED ), ":;*/?`#%", str))  // "Enter name of vector to be created."
  return (0);

 while (strlen(str) < length[0])
  strcat(str, " ");

 for (i=0; i<NoOfObjects; i++)
  {
  if (! strnicmp(str, DBase[i].Name, length[0]))
   {
   short ans;

   ans = User_Message_Def(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module"
                          GetString( MSG_LINESPRT_VECTORNAMEALREADYPRESENTINDATABASEVERWRITEITORTRYA ),  // "Vector name already present in Database!\nOverwrite it or try a new name?"
                          GetString( MSG_LINESPRT_OVERWRITENEWCANCEL ),                                  // "Overwrite|New|Cancel"
                          (CONST_STRPTR)"onc", 2);
   if (ans == 0)
    return (0);
   if (ans == 2)
    goto NewName;
   freevecarray(i);
   OBN = i;
   OverWrite = 1;
   break;
   } /* if name already in use */
  } /* for i=0... */

/* expand database for new object if necessary */
 if (! OverWrite)
  {
  if (NoOfObjects + 1 > DBaseRecords)
   {
   struct database *NewBase;

   if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
		 DBaseRecords + 20)) == NULL)
    {
    User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                 GetString( MSG_LINESPRT_OUTOFMEMORYEXPANDINGDATABASEPERATIONTERMINATED ),  // "Out of memory expanding Database!\nOperation terminated."
                 GetString( MSG_GLOBAL_OK ),                                              // "OK"
                 (CONST_STRPTR)"o");
    return (0);
    } /* if new database allocation fails */
   else
    {
    DBase = NewBase;
    DBaseRecords += 20;
    } /* else new database allocated and copied */
   } /* if need larger database memory */
  OBN = NoOfObjects;
  } /* if new object */

 strncpy(DBase[OBN].Name, str, length[0]);
 if (item)
  strcpy(DBase[OBN].Layer1, "FP");
 else
  strcpy(DBase[OBN].Layer1, "CP");
 strcpy(DBase[OBN].Layer2, "PTH");
 DBase[OBN].Color = item ? 3: 7;
 DBase[OBN].LineWidth = 1;
 strncpy(DBase[OBN].Label, str, length[0]);
 for (i=length[0]; i<length[5]; i++)
   strcat(DBase[OBN].Label, " ");
 DBase[OBN].Pattern[0] = 'L';
 DBase[OBN].Points = 0;
 DBase[OBN].Mark[0] = 'Y';
 DBase[OBN].Enabled = '*';
 DBase[OBN].Red = (AltColors[DBase[OBN].Color] & 0xf00) / 256;
 DBase[OBN].Red += DBase[OBN].Red * 16;
 DBase[OBN].Grn = (AltColors[DBase[OBN].Color] & 0x0f0) / 16;
 DBase[OBN].Grn += DBase[OBN].Grn * 16;
 DBase[OBN].Blu = (AltColors[DBase[OBN].Color] & 0x00f);
 DBase[OBN].Blu += DBase[OBN].Blu * 16;
 DBase[OBN].Flags = 6;
 strcpy(DBase[OBN].Special, "VEC");
 if (! OverWrite)
  NoOfObjects ++;

/* get some point memory */
 if (! allocvecarray(OBN, Frames, 1))
  {
  User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
               GetString( MSG_LINESPRT_OUTOFMEMCREATNEWVECTOROBJECTPERATIONTERMINAT ),  // "Out of memory creating new vector object!\nOperation terminated."
               GetString( MSG_GLOBAL_OK ),                                                  // "OK"
               (CONST_STRPTR)"o");
  return (0);
  } /* if memory fails */

 SetPointer(MapWind0, WaitPointer, 16, 16, -6, 0);

/* just in case there are no key frames for some axes */
 initmopar();

/* convert points */
 if (UseAll)
  {
  for (i=1; i<=Frames; i++)
   {
   if (KT[item * 3].Key)				/* altitude */
    TempElev = KT[item * 3].Val[0][i] / ELSCALE_METERS;
   else
    TempElev = PARC_RNDR_MOTION(item * 3) / ELSCALE_METERS;
   if (Flatten)
    {
    TempElev /= PARC_RNDR_MOTION(14);
    DBase[OBN].Elev[i] = (TempElev - PARC_RNDR_MOTION(13) * PARC_RNDR_MOTION(12))
	 / (1.0 - PARC_RNDR_MOTION(12));
    }
   else
    DBase[OBN].Elev[i] = TempElev;
   if (KT[item * 3 + 1].Key)			/* latitude */
    DBase[OBN].Lat[i] = KT[item * 3 + 1].Val[0][i];
   else
    DBase[OBN].Lat[i] = PARC_RNDR_MOTION(item * 3 + 1);
   if (KT[item * 3 + 2].Key)			/* longitude */
    DBase[OBN].Lon[i] = KT[item * 3 + 2].Val[0][i];
   else
    DBase[OBN].Lon[i] = PARC_RNDR_MOTION(item * 3 + 2);
   } /* for i=0... */
  } /* if Use All frames */
 else
  {
  LastDoneKey = 0;
  for (j=0, i=1; j<ParHdr.KeyFrames && i<=Frames; j++)
   {
   if (KF[j].MoKey.KeyFrame == LastDoneKey)
    continue;
   if (KF[j].MoKey.Group == 0)
    {
    if (KF[j].MoKey.Item == item * 3 || KF[j].MoKey.Item == item * 3 + 1
	|| KF[j].MoKey.Item == item * 3 + 2)
     {
     if (KT[item * 3].Key)				/* altitude */
      TempElev = KT[item * 3].Val[0][KF[j].MoKey.KeyFrame] / ELSCALE_METERS;
     else
      TempElev = PARC_RNDR_MOTION(item * 3) / ELSCALE_METERS;
     if (Flatten)
      {
      TempElev /= PARC_RNDR_MOTION(14);
      DBase[OBN].Elev[i] = (TempElev - PARC_RNDR_MOTION(13) * PARC_RNDR_MOTION(12))
	 / (1.0 - PARC_RNDR_MOTION(12));
      }
     else
      DBase[OBN].Elev[i] = TempElev;
     if (KT[item * 3 + 1].Key)			/* latitude */
      DBase[OBN].Lat[i] = KT[item * 3 + 1].Val[0][KF[j].MoKey.KeyFrame];
     else
      DBase[OBN].Lat[i] = PARC_RNDR_MOTION(item * 3 + 1);
     if (KT[item * 3 + 2].Key)			/* longitude */
      DBase[OBN].Lon[i] = KT[item * 3 + 2].Val[0][KF[j].MoKey.KeyFrame];
     else
      DBase[OBN].Lon[i] = PARC_RNDR_MOTION(item * 3 + 2);

     i ++;
     LastDoneKey = KF[j].MoKey.KeyFrame;
     } /* if time to spit out some values */
    } /* if motion group */
   } /* for j=0... */
  } /* else only key frames */

 DBase[OBN].Points = i - 1;
 DBase[OBN].Elev[0] = DBase[OBN].Elev[1];
 DBase[OBN].Lat[0] = DBase[OBN].Lat[1];
 DBase[OBN].Lon[0] = DBase[OBN].Lon[1];

 if (KT)
  FreeKeyTable();

 ClearPointer(MapWind0);

 if (MapWind0)
  {
  struct clipbounds cb;

  setclipbounds(MapWind0, &cb);
  outline(MapWind0, OBN, DBase[OBN].Color, &cb);
  } /* if map window open */
 if (DE_Win)
  Set_DE_Item(OBN);

 if (! saveobject(OBN, NULL, DBase[OBN].Lon, DBase[OBN].Lat, DBase[OBN].Elev))
  {
  savedbase(1);
  }

 if (! MapWind0)
  freevecarray(OBN);

 return (1);

} /* PathToVector() */

/**************************************************************************/
