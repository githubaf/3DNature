// ResourceSupport.cpp
// bridge Win32 resources to C++

#include "ResourceSupport.h"
#include "InstanceSupport.h"

std::string LoadStringFromResource(unsigned long int ID)
{
// Apparently under Win32 resource strings are limited to
// 256 characters anyway
char StringBuffer[256];
std::string Result;

if(LoadString(GetHInstance(), ID, StringBuffer, 255))
	{
	Result = StringBuffer; // needless copy. Sigh.
	} // if

return(Result);
} // LoadStringFromResource
