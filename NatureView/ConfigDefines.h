// ConfigDefines.h

#ifndef NVW_CONFIGDEFINES_H
#define NVW_CONFIGDEFINES_H

#define NV_FOLIAGE_STEMS_MAX_DEFAULT			100000
#define NV_MOVE_OPTIMIZE_SETTLE_TIME_SECONDS	.75
#define NV_TERRAIN_VERTPERCELL_MAX				512 // This number is empirical. This is a temporary tuning value until we have LOD terrain anyway

#define NV_FRAME_RATE_MULTIPLE  2  // This is the ratio of our maximum framerate to display refresh. Note this can be 1 !!

#define NV_LOD_SETTLE_TIME_SECONDS	.1 // start slowing down refresh after .1 second (or 6 frames at 60Hz)
#define NV_LOD_SETTLE_FPS	15 // we'll drop down to 5fps during settled times
#define NV_LOD_SETTLE_WINDOW_SECONDS	1 // settle to SETTLE_FPS over the course of 1 second


#define NVW_SPLASH_DISPLAY_SECONDS		5
#define NVW_HELP_DISPLAY_SECONDS		20

#define NVW_ZOOM_RATE_FACTOR			1.2f

#endif // NVW_CONFIGDEFINES_H
