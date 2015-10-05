// ProjectIODetail.cpp
// Code for working with ProjectIODetail
// Needed for DEMCore to work stand-alone

#include "stdafx.h"
#include "ProjectIODetail.h"

struct ProjectIODetail *ProjectIODetailSearchStandAlone(struct ProjectIODetail *This, char *Search)
{
struct ProjectIODetail *Found = NULL;

if (Search)
	{
	while (This)
		{
		if (! strnicmp(This->ChunkID, Search, 8))
			{
			Found = This;
			break;
			} // if found
		This = This->Next;
		} // while
	} // if

return (Found);

} // ProjectIODetailSearchStandAlone()
