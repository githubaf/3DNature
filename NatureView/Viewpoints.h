

#ifndef NVW_VIEWPOINTS_H
#define NVW_VIEWPOINTS_H

enum
	{
	NVW_VIEWPOINTMENU_IDBASE = 58000,
	NVW_VIEWPOINTMENU_HIGHEST = 58100, // max of 100 viewpoints -- you crazy person
	NVW_VIEWPOINTMENU_VPTOUR = 58101,
	NVW_VIEWPOINTMENU_VPNEXT = 58102,
	NVW_VIEWPOINTMENU_VPPREV = 58103,
	NVW_VIEWPOINTMENU_MAX // no further items
	}; // menu IDs


int SelectNewCameraViaMenu(int XCoord, int YCoord);
void BuildViewpointMenu(void *MenuHandle);
int HandleViewpointMenuSelection(int MenuID);


#endif // !NVW_VIEWPOINTS_H

