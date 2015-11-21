

#ifndef NVW_CATEGORY_H
#define NVW_CATEGORY_H

enum
	{
	NVW_CATEGORYMENU_IDBASE = 59000,
	NVW_CATEGORYMENU_OCEAN,
	NVW_CATEGORYMENU_TERRAIN,
	NVW_CATEGORYMENU_VECTORS,
	NVW_CATEGORYMENU_FOLIAGE,
	NVW_CATEGORYMENU_3DO,
	NVW_CATEGORYMENU_LABELS,
	NVW_CATEGORYMENU_OVERLAY,
	NVW_CATEGORYMENU_MAX // no further items
	}; // menu IDs


int SelectCategoryViaMenu(int XCoord, int YCoord);
void BuildCategoryMenu(void *MenuHandle);
int HandleCategoryMenuSelection(int MenuID);


#endif // !NVW_CATEGORY_H

