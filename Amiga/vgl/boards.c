
#include "vgl.h"
#include "vgl_internals.h"

/*****************************************************************************/
/*****************************************************************************/
/*  Board names under OS/9 must not contain dashes (-) and cannot be longer
 *  than 9 characters.
 */


struct board_config board_config[] =
  {  /* Name,   type,    address,    irq_level, irq_vector */
    {"MMI_150" , MMI_150,  (void *)0x08000000, 1,         176},
    {"MMI_250" , MMI_250,  (void *)0x09000000, 1,         180},
    {"GDM_9000", GDM_9000, (void *)0x0A000000, 1,         188},
    {"DB_180"  , DB_180,   (void *)0xFD000000, 4,         184},
    {"", -1,  NULL, 0, 0} /* Must be the last "board" */
  };
