// WindowDefaults.h

#ifndef NVW_WINDOWDEFAULTS_H
#define NVW_WINDOWDEFAULTS_H

enum ToolWindowLocation
	{
	Bottom = 0, // default
	Top,
	Left, // not used
	Right, // not used
	Last // always last
	}; // ToolWindowLocation

bool GetShowInfoWindowByDefault(void);
bool GetShowNavWindowByDefault(void);
bool GetShowDriveWindowByDefault(void);
bool GetShowMapWindowByDefault(void);
bool GetShowViewFullscreenByDefault(void);
ToolWindowLocation GetToolWindowLocation(void);

size_t ParseWindowDefaultOptions(const char *DefOpts);


#endif // NVW_WINDOWDEFAULTS_H

