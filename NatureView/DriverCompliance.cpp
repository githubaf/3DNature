// DriverCompliance.cpp
// Code for warning users about stupid graphics subsystems that don't work well
// Created from scratch on 7/19/06 by CXH

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>
#include "DriverCompliance.h"
#include "RequesterBasic.h"
#include "MediaSupport.h" // to open web pages

// returns TRUE if we have warned the user
int CheckAndWarnDriverCompliance(void)
{
int Warned = 0;

// ATI
const char *Vend, *Rend;
Vend = (const char *)glGetString(GL_VENDOR);
Rend = (const char *)glGetString(GL_RENDERER);
if(Vend && !strcmp(Vend, "ATI Technologies Inc."))
	{
	// Radeon
	glGetString(GL_RENDERER);
	if(Rend && strstr(Rend, "RADEON"))
		{
		const char *Vers;
		char Tokened[256], *Iter, *ThisToken;
		double Version = 0.0;
		// versions prior to 2.0.5885
		Vers = (const char *)glGetString(GL_VERSION);
		if(Vers)
			{
			strcpy(Tokened, Vers);
			for(Iter = Tokened; ThisToken = strtok(Iter, "."); Iter = NULL)
				{
				Version *= 10000.0;
				Version += atoi (ThisToken);
				} // for
			
			if(Version != 0.0 && Version < 200005885.0)
				{
				char ComplianceWarning[400];
				sprintf(ComplianceWarning, "The graphics card and driver detected on this computer are known to have problems with NatureView.\n\"%s %s %s\"\nA newer version is available from the manufacturer.\nWould you like to visit their web site to obtain the latest driver?", Vend, Rend, Vers);
				if(UserMessageYN("OpenGL Card/Driver Warning", ComplianceWarning))
					{
					OpenURLExternally("http://www.ati.com");
					} // if
				Warned = 1;
				} // if
			} // if
		} // if
	} // if

return(Warned);
} // CheckAndWarnDriverCompliance