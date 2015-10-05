/* Menu.c (ne gismenu.c 14 Jan 1994 CXH)
** ASCII Menus to select modules and run GIS renderer.
** Not used if GUI is in use.
** Original code by Gary R. Huber, 11/8/93.
*/

#include "WCS.h"

#ifndef USE_GIS_GUI
void wcsmainmenu(void)
{
 short menuitem, abort = 0;

 do {
  if (dbaseloaded && paramsloaded) {
   printf("\n\n******* WCS Main: Database \"%s\", Params \"%s\" *******\n\n",
	dbasename, paramfile);
  } /* if */
  else if (dbaseloaded)
   printf("\n\n***** WCS Main: Database \"%s\", Please Load Params *****\n\n",
	dbasename);
  else 
   printf("\n\n******** WCS Main: Please Load A Database First ********\n\n");

  printf("  0. Database Operations\n");
  printf("  1. Data Manipulation\n");
  printf("  2. 2-D Mapping\n");
  printf("  3. Parameter Editing\n");
  printf("  4. 3-D Rendering\n");
  printf("  5. Preferences\n");
  printf("100. Exit WCS\n\n");

  printf("Your Choice: ");
  scanf("%hd", &menuitem);

  switch (menuitem) {
   case 0:
    abort = dbasemenu();
    break;
   case 1:
    abort = datamenu();
    break;
   case 2:
    abort = map();
    break;
   case 3:
    abort = editparam();
    break;
   case 4:
    abort = rendermenu();
    break;
   case 5:
    wcsprefsmenu();
    break;
   case 100:
    abort = 1;
    break;
  } /* switch menuitem */

 } while (! abort);

} /* mainmenu() */

short dbasemenu(void)
{
 short i, loadstatus, menuitem, abort = 0, menuenabled;

 do {
  if (dbaseloaded) {
   printf("\n\n******** WCS Database: Database \"%s\" ********\n\n",
	dbasename);
   menuenabled = 1;
  } /* if */
  else {
   printf("\n\n******** WCS Database: Please Load A Database ********\n\n");
   menuenabled = 0;
  } /* else */

  printf("  0. Load Database\n");
  printf("  1. Append Database\n");
  printf("  2. Create New Database\n");
  printf("  3. Edit Database\n");
  printf("  4. Save Database\n");
  printf(" 50. Load Parameters\n");
  printf("200. Return To Main Menu\n");
  printf("100. Exit WCS\n\n");

  printf("Your Choice: ");
  scanf("%hd", &menuitem);

  switch (menuitem) {
   case 0:
    if (loaddbase(0, NULL)) dbaseloaded = 1;
    if (dbaseloaded) {
     if ((loadstatus = loadparams(0x1111, -1, NULL)) == -1) paramsloaded = 0;
     else if (loadstatus == 1) {
      paramsloaded = 1;
      for (i=0; i<2; i++) {
       copytonewmoparams(i);
       copytonewcoparams(i);
       copytonewecoparams(i);
      } /* for */
     } /* else if */
    } /* if dbaseloaded */
    break;
   case 1:
    if (menuenabled) loaddbase(NoOfObjects, NULL);
    break;
   case 2:
    if (makedbase()) dbaseloaded = 1;
    break;
   case 3:
    if (menuenabled) abort = eddbase();
    break;
   case 4:
    if (menuenabled) savedbase(NULL);
    break;
   case 50:
    if ((loadstatus = loadparams(0x1111, -1, NULL)) == -1) paramsloaded = 0;
    else if (loadstatus == 1) {
     paramsloaded = 1;
     for (i=0; i<2; i++) {
      copytonewmoparams(i);
      copytonewcoparams(i);
      copytonewecoparams(i);
     } /* for */
    } /* else if */
    break;
   case 200:
    return (0);
    break;
   case 100:
    return (1);
    break;
  } /* switch menuitem */
 } while (! abort);

 return (abort);

} /* dbasemenu() */

short datamenu(void)
{
 short menuitem, abort = 0, menuenabled;

 do {
  if (dbaseloaded) {
   printf("\n\n******** WCS Data Ops: Database \"%s\" ********\n\n",
	dbasename);
   menuenabled = 1;
  } /* if */
  else {
   printf("\n\n******** WCS Data Ops: Please Load A Database ********\n\n");
   menuenabled = 0;
  } /* else */

  printf("  0. Load Database\n");
  printf("  1. Create face file(s)\n");
  printf("  2. Create relative elevation file(s)\n");
  printf("  3. Interpolate map\n");
  printf("200. Return To Main Menu\n");
  printf("100. Exit WCS\n\n");

  printf("Your Choice: ");
  scanf("%hd", &menuitem);

  switch (menuitem) {
   case 0:
    if (loaddbase(0, NULL)) dbaseloaded = 1;
    break;
   case 1:
    if (menuenabled) makefacefile();
    break;
   case 2:
    if (menuenabled) makerelelfile();
    break;
   case 3:
    if (menuenabled) interpmap();
    break;
   case 200:
    return (0);
    break;
   case 100:
    return (1);
    break;
  } /* switch menuitem */
 } while (! abort);

 return (abort);

} /* datamenu() */

short editparam(void)
{
 short ans, i, k, abort = 0, loadstatus, menuenabled;
 char num[32];

 do {
  if (paramsloaded) {
   printf("******** WCS Editing: Database \"%s\", Params \"%s\" ********\n\n",
	dbasename, paramfile);
   menuenabled = 1;
  } /* if */
  else {
   printf("****** WCS Editing: Database \"%s\", Please Load Params ******\n\n",
	dbasename);
   menuenabled = 0;
  } /* if */

  printf("  0. Return and Cancel Changes\n");
  printf("  1. Edit Motion Parameters\n"); 
  printf("  2. Edit Palette\n");
  printf("  3. Edit Ecosystem Parameters\n");
  printf("  4. Edit Surface parameters\n");
  printf("  5. Set First Frame\n");
  printf("  6. Set Last Frame\n");
  printf("  7. Set Quick Step\n");
  printf("  8. Set Next Key Frame\n");
  printf("  9. Edit Render Settings\n");
  printf(" 10. Edit Motion Interactively\n"); 
  printf(" 17. Reset All Original Parameters\n");
  printf(" 18. Fix All Parameters For Undo\n");
  printf(" 19. Undo All Changes To Last Fix\n\n");
  printf(" 50. Load All Parameters\n");
  printf(" 60. Save All Parameters\n");
  printf("200. Return And Use/Save Changes\n");
  printf("100. Exit WCS\n\n");

  printf(" Your choice: ");
  scanf("%s",&num);
  k=atoi(num);

  switch (k) {
   case 0:
    if (menuenabled) {
     copyfromnewmoparams(0);
     copyfromnewcoparams(0);
     copyfromnewecoparams(0);
    } /* if */
    return (0);
    break;
   case 1:
    if (menuenabled) edmopar();
    break;
   case 2:
    if (menuenabled) edcopar();
    break;
   case 3:
    if (menuenabled) edecopar();
    break; 
   case 4:
    if (menuenabled) edsurfpar();
    break; 
   case 5:
    if (menuenabled) { 
     printf("First frame: <%d> ",ParHdr.FirstFrame);
     scanf("%hd",&ParHdr.FirstFrame);
    } /* if */
    break;
   case 6:
    if (menuenabled) {
     printf("Last Frame: <%d> ",ParHdr.LastFrame);
     scanf("%hd",&ParHdr.LastFrame);
    } /* if */
    break;
   case 7:
    if (menuenabled) {
     printf("Quick frame step: <%d> ",ParHdr.QuickStep);
     scanf("%hd",&ParHdr.QuickStep);
    } /* if */
    break;
   case 8:
    if (menuenabled) {
     printf("Next key frame: <%d> ",ParHdr.NextKey);
     scanf("%hd",&ParHdr.NextKey);
    } /* if */
    break;
   case 9:
    if (menuenabled) abort = globemapmenu();
    break;
   case 10:
    if (menuenabled) interactiveview(0);
    break;
   case 17:
    if (menuenabled) {
     copyfromnewmoparams(0);
     copyfromnewcoparams(0);
     copyfromnewecoparams(0);
    } /* if */
    break;
   case 18:
    if (menuenabled) {
     copytonewmoparams(1);
     copytonewcoparams(1);
     copytonewecoparams(1);
    } /* if */
    break;
   case 19:
    if (menuenabled) {
     copyfromnewmoparams(1);
     copyfromnewcoparams(1);
     copyfromnewecoparams(1);
    } /* if */
    break;
   case 50:
    if ((loadstatus = loadparams(0x1111, -1, NULL)) == -1) paramsloaded = 0;
    else if (loadstatus == 1) {
     paramsloaded = 1;
     for (i=0; i<2; i++) {
      copytonewmoparams(i);
      copytonewcoparams(i);
      copytonewecoparams(i);
     } /* for */
    } /* else if */
    break;
   case 60:
    if (menuenabled) saveparams(0x1111, -1, NULL);
    break;
   case 100:
    return (1);
    break;
  } /* switch */
 } while (k != 200 && ! abort);

 if (k == 200) {
  printf("Save Parameters? (1= yes, 0= No): ");
  scanf("%hd", &ans);
  if (ans) saveparams(0x1111, -1, NULL);
 } /* if */
 
 return (abort);

} /* editparam() */

short rendermenu(void)
{
 short menuitem, abort = 0, loadstatus, menuenabled, i;

 do {
  if (dbaseloaded && paramsloaded) {
   printf("\n\n****** WCS Render: Database \"%s\", Params \"%s\" ******\n\n",
	dbasename, paramfile);
   menuenabled = 1;
  } /* if */
  else if (dbaseloaded) {
   printf("\n\n***** WCS Render: Database \"%s\", Please Load Params *****\n\n",
	dbasename);
   menuenabled = 0;
  } /* else if */
  else {
   printf("\n\n******** WCS Render: Please Load A Database First ********\n\n");
   menuenabled = 0;
  } /* else */

  printf("  0. Load Database\n");
  printf("  1. Load Parameters\n");
  printf("  2. Edit Settings\n");
  printf("500. Render Image/Animation\n");
  printf("200. Return To Main Menu\n");
  printf("100. Exit WCS\n\n");

  printf("Your Choice: ");
  scanf("%hd", &menuitem);

  switch (menuitem) {
   case 0:
    if (loaddbase(0, NULL)) dbaseloaded = 1;
    break;
   case 1:
    if (dbaseloaded) {
     if ((loadstatus = loadparams(0x1111, -1, NULL)) == -1) paramsloaded = 0;
     else if (loadstatus == 1) {
      paramsloaded = 1;
      for (i=0; i<2; i++) {
       copytonewmoparams(i);
       copytonewcoparams(i);
       copytonewecoparams(i);
      } /* for i=0... */
     } /* if loadstatus */
    } /* if dbaseloaded */
    else printf("ERROR: You must first load a database!\n");
    break;
   case 2:
    if (menuenabled) abort = globemapmenu();
    break;
   case 500:
    if (menuenabled) {
     render = settings.renderopts;
     if (settings.stepframes==2) settings.stepframes = (short)(ParHdr.QuickStep);
     else settings.stepframes = 1;
     printf("Thank you. Initializing parameters...\n\n");
     globemap();
    } /* if */
   case 200:
    return (0);
    break;
   case 100:
    return (1);
    break;
  } /* switch menuitem */
 } while (! abort);

 return (abort);

} /* rendermenu */


void wcsprefsmenu(void)
{

/*
** 
*/

} /* wcsprefsmenu() */


short globemapmenu(void)
{
 short menuitem, abort = 0;

 render = 0;
 do {
  printf("\n\n********* GISBase Render Settings: Params \"%s\" ********\n\n",
	paramfile);
  printf("500. Begin rendering\n");
  printf("  1. Frame to start mapping, frames to map: %d, %d\n",
    settings.startframe,settings.maxframes);
  printf("  2. Follow camera, focus paths, bank turns, banking: %d, %d, %d, %f\n",
    settings.campath,settings.focpath,settings.bankturn,settings.bankfactor);
  printf
    ("  3. Use color maps? Random borders? Trees? Color Match? G/Y Model? Stats?\n");
  printf("    (1=Yes, 0=No): %d, %d, %d, %d, %d, %d\n",
	settings.colrmap,settings.borderandom,settings.cmaptrees,
	settings.ecomatch,settings.treemodel,settings.statistics);
  printf("  4. Horizon fixed (1= Yes, 0= No), zenith (mi): %d, %f\n",
	settings.horfix, settings.zenith);
  printf("  5. Render mode (1= every frame, 2= every %dth frame): %d\n\n",
    (short)(ParHdr.QuickStep),settings.stepframes);

  printf("  6. Antialiasing level, zbuffered?, sky antialias: %d, %d, %d\n",
    (long)MoPar.mn[24].Value[0],settings.zbufalias,(long)settings.skyalias);
  printf("  7. Fractal depth, premap, fix fractal: %d, %d, %d\n",
    (long)MoPar.mn[23].Value[0],settings.premap,settings.fixfract);
  printf
    ("  8. Map world? clouds? (1= Yes, 0= No): %d, %d",settings.worldmap,settings.clouds);
  if (settings.worldmap) {
   if (settings.worldbase) {
    printf(", 5 arc-min, %d to %d deg\n",(long)settings.lowglobe,(long)settings.highglobe);
   }
   else printf(", 30 arc-min database\n");
  }
  else printf("\n");
  printf("  9. Fade line objects? (1= Yes, 0= No): %d\n",settings.linefade);
  printf(" 10. Line offset (miles): %f\n\n",settings.lineoffset);

  printf(" 11. Map topo objects as surfaces? (1= Yes. 0= No): %d\n",settings.mapassfc);
  printf(" 12. Alternate q: %d\n",settings.alternateq);
  printf(" 13. Draw surface grid? (1= Yes, 0= No), grid size: %d, %d\n",
      settings.drawgrid,settings.gridsize);
  printf(" 14. Tree factor: %f\n",settings.treefactor);
  printf
    (" 15. Draw line objects to screen, file (1= Scrn, 0= File, -1= None): %d\n\n",
      settings.linetoscreen);

  printf(" 16. Set rendering options <%x>\n", settings.renderopts);
  printf(" 17. Set screen size <%d x %d @ %f>\n",
	settings.scrnwidth, settings.scrnheight, settings.picaspect);
  printf(" 18. Set rendering segments <%d>\n", settings.rendersegs);
  printf(" 19. Set vertical overscan <%d>\n", settings.overscan);
  printf(" 20. Composite rendered segments? (1= Yes, 0= No): <%d>\n\n",
		settings.composite);

  printf(" 21. Save format (1= IFF, 0= RAW) Interleave? (1= Yes, 0=No): %d, %d\n",
	settings.saveIFF, settings.interleave);
  printf(" 22. Auto-Select maps to render (odd = use) %x  Selected: %d\n\n",
		settings.autoselect, RenderObjects);
  printf(" 25. Frame storage path: %s\n",framepath);
  printf(" 50. Load settings\n");
  printf(" 60. Save settings\n");
  printf("200. Return to Main Menu\n");
  printf("100. Abort Program\n\n");
  printf("Your choice: ");
  scanf("%hd",&menuitem);

  switch (menuitem) {
   case 500:				/* Begin rendering */
    render = settings.renderopts;
    if (settings.stepframes==2) settings.stepframes = (short)(ParHdr.QuickStep);
    else settings.stepframes = 1;
    printf("Thank you. Initializing parameters...\n\n");
    globemap();
    break;
   case 1:
    printf("Frame to start mapping: <%d> ",settings.startframe);
    scanf("%hd",&settings.startframe);
    printf("Number of frames to map: <%d>",settings.maxframes);
    scanf("%hd",&settings.maxframes);
    break;
   case 2:
    printf("Follow camera path? (1=Yes, 0=No): <%d>",settings.campath);
    scanf("%hd",&settings.campath);
    printf("Follow focus path? (1=Yes, 0=No): <%d>",settings.focpath);
    scanf("%hd",&settings.focpath);
    printf("Bank turns? (1=Yes, 0=No): <%d>",settings.bankturn);
    scanf("%hd",&settings.bankturn);
    if (settings.bankturn) {
     printf("Turn banking factor: <%f>",settings.bankfactor);
     scanf("%le",&settings.bankfactor);
    }
    break;
   case 3:
    printf("Use color maps? (1=Yes, 0=No): <%d>",settings.colrmap);
    scanf("%hd",&settings.colrmap);
    if (settings.colrmap) {
     printf("Default ecosystem: <%d>",settings.defaulteco);
     scanf("%hd", &settings.defaulteco);
     printf("Randomize borders? (1=Yes, 0=No): <%d>",settings.borderandom);
     scanf("%hd",&settings.borderandom);
     printf("Draw trees for color mapped area? (1=Yes, 0=No): <%d>",settings.cmaptrees);
     scanf("%hd",&settings.cmaptrees);
     printf("Use tree model? (1=Yes, 0=No): <%d>",settings.treemodel);
     scanf("%hd",&settings.treemodel);
     if (settings.treemodel) {
      loadtreemodel();
      settings.ecomatch = 0;
     } /* if settings.treemodel */
     else {
      printf("Match ecosystems to color map? (1= Yes, 0= No): <%d>",settings.ecomatch);
      scanf("%hd", &settings.ecomatch);
     } /* else */
     printf("Compute statistics? (1=Yes, 0=No): <%d>",settings.statistics);
     scanf("%hd",&settings.statistics);
     if (settings.statistics) {
      printf("Low elevation: ");
      scanf("%hd",&settings.lowsurfel);
      printf("High elevation: ");
      scanf("%hd",&settings.highsurfel);
      settings.surfelrange=settings.highsurfel-settings.lowsurfel;
     } /* if settings.statistics */
    } /* if settings.colrmap */
    break;
   case 4:
    printf("Fix horizon? (1= Yes, 0=No): <%d>",settings.horfix);
    scanf("%hd",&settings.horfix);
    if (! settings.horfix) {
     printf("Zenith altitude (miles above horizon): <%f>",settings.zenith);
     scanf("%le",&settings.zenith);
    }
    break;
   case 5:
    printf("Render mode (1= every frame, 2= every %dth frame): <%d>",
      (short)(ParHdr.QuickStep),settings.stepframes);
    scanf("%hd",&settings.stepframes);
    break;
   case 6:
    printf("Antialiasing (0= none, 1= one pixel, 2= two pixels...): <%d>",
      (long)MoPar.mn[24].Value[0]);
    scanf("%le",&MoPar.mn[24].Value[0]);
    if (MoPar.mn[24].Value[0]>0.0) {
     printf("Z-buffer antialiasing? (1= Yes, 0= No): <%d>",settings.zbufalias);
     scanf("%hd",&settings.zbufalias);
     if (settings.zbufalias) {
      printf("Z buffer differential (miles): <%f>",settings.zalias);
      scanf("%le",&settings.zalias);
     }
    }
    printf("Sky antialias color point range: <%d>",(long)settings.skyalias);
    scanf("%le",&settings.skyalias);
    break;
   case 7:
    printf("Fractal depth (0= none, 1= one, 2= two...): <%d>",
      (long)MoPar.mn[23].Value[0]);
    scanf("%le",&MoPar.mn[23].Value[0]);
    if (MoPar.mn[23].Value[0]>0.0) {
     settings.premap=1;
     printf("Pre-map (0= No, 1= Yes): <%d>",settings.premap);
     scanf("%hd",&settings.premap);
     printf("Fix fractal level? <%d>",settings.fixfract);
     scanf("%hd",&settings.fixfract);
    }
    else settings.premap=0;
    break;
   case 8:
    printf("Map world? (1= Yes, 0= No): <%d>",settings.worldmap);
    scanf("%hd",&settings.worldmap);
    if (settings.worldmap) {
     printf("World database (0= 30 minute, 1= 5 minute): ");
     scanf("%hd",&settings.worldbase);
     if (settings.worldbase) {
      printf("low latitude: ");
      scanf("%le",&settings.lowglobe);
      printf("high latitude: ");
      scanf("%le",&settings.highglobe);
      if (settings.lowglobe>=settings.highglobe) {
       settings.lowglobe=-90.0;
       settings.highglobe=90.0;
      }
     }
    }
    printf("Map clouds? (1= Yes, 0= No): <%d>",settings.clouds);
    scanf("%hd",&settings.clouds);
    break;
   case 9:
    printf("Fade line objects? (1= Yes, 0= No): <%d>",settings.linefade);
    scanf("%hd",&settings.linefade);
    break;
   case 10:
    printf("Offset for line visibility? (pos. float): <%f>",settings.lineoffset);
    scanf("%le",&settings.lineoffset);
    break;
   case 11:
    printf("Map topo objects as surfaces? (1= Yes. 0= No): <%d>",settings.mapassfc);
    scanf("%hd",&settings.mapassfc);
    break;
   case 12:
    printf("Use alternate q (1= Yes, 0= No): <%d>",settings.alternateq);
    scanf("%hd",&settings.alternateq);
    if (settings.alternateq) {
     printf("Latitude: <%f>",settings.altqlat);
     scanf("%le",&settings.altqlat);
     printf("Longitude: <%f>",settings.altqlon);
     scanf("%le",&settings.altqlon);
    }
    break;
   case 13:
    printf("Draw surface grid? (1= Yes, 0= No): <%d>",settings.drawgrid);
    scanf("%hd",&settings.drawgrid);
    if (settings.drawgrid) {
     printf("Grid size in cells: <%d>",settings.gridsize);
     scanf("%hd",&settings.gridsize);
    }
    break;
   case 14:
    printf("Tree factor: <%f>",settings.treefactor);
    scanf("%le",&settings.treefactor);
    printf("Randomize vertical offset? (1= Yes, 0= No): <%d>",settings.Yoffset);
    scanf("%hd",&settings.Yoffset);
    break;
   case 15:
    printf("Draw lines to screen, file or nil? (1= Screen, 0= File, -1= Nil): <%d>",
        settings.linetoscreen);
    scanf("%hd",&settings.linetoscreen);
    if (!settings.linetoscreen) {
     if (!getfilename(1,"Line File Base",linepath,linefile)) settings.linetoscreen=-1;
    }
    break;
   case 16:
    settings.renderopts = 0;
    printf("Render to screen (1= Yes, 0= No): ");
    scanf("%hd", &ans);
    if (ans) settings.renderopts = 0x10;
    printf("Render 24 bit (1= Yes, 0= No): ");
    scanf("%hd", &ans);
    if (ans) settings.renderopts |= 0x01;
    printf("Create ecosystem overlay (1 = Yes. 0= No): ");
    scanf("%hd", &ans);
    if (ans) settings.renderopts |= 0x100;
    break;
   case 17:
    printf("Screen width <%d>: ", settings.scrnwidth);
    scanf("%hd", &settings.scrnwidth);
    printf("Screen height <%d>: ", settings.scrnheight);
    scanf("%hd", &settings.scrnheight);
    printf("Pixel aspect (high/wide) <%f>: ", settings.picaspect);
    scanf("%le", &settings.picaspect);
    break;
   case 18:
    printf("Number of segments <%d>: ", settings.rendersegs);
    scanf("%hd", &settings.rendersegs);
    if (settings.rendersegs < 1) settings.rendersegs = 1;
    break;
   case 19:
    printf("Overscan bottom of image (pixels) <%d>: ", settings.overscan);
    scanf("%hd", &settings.overscan);
    if (settings.overscan < 0) settings.overscan = 0;
    break;
   case 20:
    printf("Composite rendered segments? (1= Yes, 0= No): <%d> ",
		settings.composite);
    scanf("%hd",&settings.composite);
    break;
   case 21:
    printf("File format (1= IFF, 0= RAW): <%d> ",settings.saveIFF);
    scanf("%hd",&settings.saveIFF);
    printf("Interleave frame files? (1= Yes, 0= No): <%d> ",settings.interleave);
    scanf("%hd",&settings.interleave);
    break;
   case 22:
    printf("Use automated render list? (1= Yes, 0= No): ");
    scanf("%hd", &settings.autoselect);
    if (! settings.autoselect) {
     printf("Create Render List now? (1= Yes, 0= No): ");
     scanf("%hd", &ans);
    } /* if */
    if (settings.autoselect || ans) {
     printf("Discard obscured maps? (1= Yes, 0= No): ");
     scanf("%hd", &ans);
     if (ans) settings.autoselect |= 0x1;
     printf("Compute optimal drawing direction? (1= Yes, 0= No): ");
     scanf("%hd", &ans);
     if (ans) settings.autoselect |= 0x10;
     printf("Recompute for each frame? (1= Yes, 0= No): ");
     scanf("%hd", &ans);
     if (ans) settings.autoselect |= 0x100;
     startframe = settings.startframe;
     constructview();
     CenterX = MoShift[6].Value[2];
     CenterY = MoShift[7].Value[2];
     if (autoactivate()) settings.autoselect = 0;
    } /* if */
    break;
   case 25:
    getframepath(1, framepath);
    break;
   case 50:
    loadparams(0x1000, 0, NULL);
    break;
   case 60:
    saveparams(0x1000, 0, NULL);
    break;
   case 100:
    return (1);
    break;
   case 200:
    return (0);
    break;
  }
 } while (! abort);

 return abort;

} /* globemapmenu() */

#endif /* USE_GIS_GUI */
