/* DEMobject.c
** DEM generation and manipulation functions for WCS.
** Written by Gary R. Huber, Jan 1995.
*/

#include "WCS.h"

struct DEMData *DEMData_New(void);
void DEMData_Del(struct DEMData *DEM);
short DEMData_Load(void);
short DEMData_Save(void);


/*********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 20.July 2021
struct DEMData *DEMData_New(void)
{

 return ((struct DEMData *)
	get_Memory(sizeof (struct DEMData), MEMF_CLEAR));

} /* DEMData_New() */
#endif
/*********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 20.July 2021
void DEMData_Del(struct DEMData *DEM)
{

 if (DEM)
  free_Memory(DEM, sizeof (struct Wave));

} /* DEMData_Del() */
#endif
/*********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 20.July 202
short DEMData_Load(void)
{

 return (0);

} /* DEMData_Load() */
#endif
/*********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 20.July 2021
short DEMData_Save(void)
{

 return (0);

} /* DEMData_Save() */
#endif
