#ifndef WCS_STRINGS_H
#define WCS_STRINGS_H 1

/* Locale Catalog Source File
 *
 * Automatically created by SimpleCat V3
 * Do NOT edit by hand!
 *
 * SimpleCat ©1992-2013 Guido Mersmann
 *
 */



/****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif



/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_MENU_PROJECT 0
#define MSG_MENU_PR_NEW 1
#define MSG_MENU_PR_EDIT 2
#define MSG_MENU_PR_OPEN 3
#define MSG_MENU_PR_SAVE 4
#define MSG_MENU_PR_SAVEAS 5
#define MSG_MENU_PR_LOADCONFIG 6
#define MSG_MENU_PR_SAVECONFIG 7
#define MSG_MENU_PR_SAVESCREEN 8
#define MSG_MENU_PR_INFO 9
#define MSG_MENU_PR_VERSION 10
#define MSG_MENU_PR_CREDITS 11
#define MSG_MENU_PR_LOG 12
#define MSG_MENU_PR_QUIT 13
#define MSG_MENU_PR_ICONIFY 14
#define MSG_MENU_MODULES 15
#define MSG_MENU_MOD_DATABASE 16
#define MSG_MENU_MOD_DATAOPS 17
#define MSG_MENU_MOD_MAPVIEW 18
#define MSG_MENU_MOD_PARAMETERS 19
#define MSG_MENU_MOD_RENDER 20
#define MSG_MENU_MOD_MOTIONEDITOR 21
#define MSG_MENU_MOD_COLOREDITOR 22
#define MSG_MENU_MOD_ECOSYSEDITOR 23
#define MSG_MENU_PREFS 24
#define MSG_MENU_PREF_PREFERENCES 25
#define MSG_MENU_PREF_SCREENMODE 26
#define MSG_MENU_PARAMETERS 27
#define MSG_MENU_PAR_LOADALL 28
#define MSG_MENU_PAR_SAVEALL 29
#define MSG_MENU_PAR_FREEZE 30
#define MSG_MENU_PAR_RESTORE 31

#define CATCOMP_LASTID 31

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_MENU_PROJECT_STR "Project"
#define MSG_MENU_PR_NEW_STR "New..."
#define MSG_MENU_PR_EDIT_STR "Edit..."
#define MSG_MENU_PR_OPEN_STR "Open..."
#define MSG_MENU_PR_SAVE_STR "Save"
#define MSG_MENU_PR_SAVEAS_STR "Save As..."
#define MSG_MENU_PR_LOADCONFIG_STR "Load Config"
#define MSG_MENU_PR_SAVECONFIG_STR "Save Config"
#define MSG_MENU_PR_SAVESCREEN_STR "Save Screen..."
#define MSG_MENU_PR_INFO_STR "Info..."
#define MSG_MENU_PR_VERSION_STR "Version..."
#define MSG_MENU_PR_CREDITS_STR "Credits..."
#define MSG_MENU_PR_LOG_STR "Log..."
#define MSG_MENU_PR_QUIT_STR "Quit..."
#define MSG_MENU_PR_ICONIFY_STR "Iconify..."
#define MSG_MENU_MODULES_STR "Modules"
#define MSG_MENU_MOD_DATABASE_STR "DataBase"
#define MSG_MENU_MOD_DATAOPS_STR "Data Ops"
#define MSG_MENU_MOD_MAPVIEW_STR "Map View"
#define MSG_MENU_MOD_PARAMETERS_STR "Parameters"
#define MSG_MENU_MOD_RENDER_STR "Render"
#define MSG_MENU_MOD_MOTIONEDITOR_STR "Motion Editor"
#define MSG_MENU_MOD_COLOREDITOR_STR "Color Editor"
#define MSG_MENU_MOD_ECOSYSEDITOR_STR "Ecosys Editor"
#define MSG_MENU_PREFS_STR "Preferences"
#define MSG_MENU_PREF_PREFERENCES_STR "Preferences..."
#define MSG_MENU_PREF_SCREENMODE_STR "Screen Mode..."
#define MSG_MENU_PARAMETERS_STR "Parameters"
#define MSG_MENU_PAR_LOADALL_STR "Load All..."
#define MSG_MENU_PAR_SAVEALL_STR "Save All..."
#define MSG_MENU_PAR_FREEZE_STR "Freeze"
#define MSG_MENU_PAR_RESTORE_STR "Restore"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

const char CatCompBlock[] =
{
    "\x00\x00\x00\x00\x00\x08"
    MSG_MENU_PROJECT_STR "\x00"
    "\x00\x00\x00\x01\x00\x08"
    MSG_MENU_PR_NEW_STR "\x00\x00"
    "\x00\x00\x00\x02\x00\x08"
    MSG_MENU_PR_EDIT_STR "\x00"
    "\x00\x00\x00\x03\x00\x08"
    MSG_MENU_PR_OPEN_STR "\x00"
    "\x00\x00\x00\x04\x00\x06"
    MSG_MENU_PR_SAVE_STR "\x00\x00"
    "\x00\x00\x00\x05\x00\x0C"
    MSG_MENU_PR_SAVEAS_STR "\x00\x00"
    "\x00\x00\x00\x06\x00\x0C"
    MSG_MENU_PR_LOADCONFIG_STR "\x00"
    "\x00\x00\x00\x07\x00\x0C"
    MSG_MENU_PR_SAVECONFIG_STR "\x00"
    "\x00\x00\x00\x08\x00\x10"
    MSG_MENU_PR_SAVESCREEN_STR "\x00\x00"
    "\x00\x00\x00\x09\x00\x08"
    MSG_MENU_PR_INFO_STR "\x00"
    "\x00\x00\x00\x0A\x00\x0C"
    MSG_MENU_PR_VERSION_STR "\x00\x00"
    "\x00\x00\x00\x0B\x00\x0C"
    MSG_MENU_PR_CREDITS_STR "\x00\x00"
    "\x00\x00\x00\x0C\x00\x08"
    MSG_MENU_PR_LOG_STR "\x00\x00"
    "\x00\x00\x00\x0D\x00\x08"
    MSG_MENU_PR_QUIT_STR "\x00"
    "\x00\x00\x00\x0E\x00\x0C"
    MSG_MENU_PR_ICONIFY_STR "\x00\x00"
    "\x00\x00\x00\x0F\x00\x08"
    MSG_MENU_MODULES_STR "\x00"
    "\x00\x00\x00\x10\x00\x0A"
    MSG_MENU_MOD_DATABASE_STR "\x00\x00"
    "\x00\x00\x00\x11\x00\x0A"
    MSG_MENU_MOD_DATAOPS_STR "\x00\x00"
    "\x00\x00\x00\x12\x00\x0A"
    MSG_MENU_MOD_MAPVIEW_STR "\x00\x00"
    "\x00\x00\x00\x13\x00\x0C"
    MSG_MENU_MOD_PARAMETERS_STR "\x00\x00"
    "\x00\x00\x00\x14\x00\x08"
    MSG_MENU_MOD_RENDER_STR "\x00\x00"
    "\x00\x00\x00\x15\x00\x0E"
    MSG_MENU_MOD_MOTIONEDITOR_STR "\x00"
    "\x00\x00\x00\x16\x00\x0E"
    MSG_MENU_MOD_COLOREDITOR_STR "\x00\x00"
    "\x00\x00\x00\x17\x00\x0E"
    MSG_MENU_MOD_ECOSYSEDITOR_STR "\x00"
    "\x00\x00\x00\x18\x00\x0C"
    MSG_MENU_PREFS_STR "\x00"
    "\x00\x00\x00\x19\x00\x10"
    MSG_MENU_PREF_PREFERENCES_STR "\x00\x00"
    "\x00\x00\x00\x1A\x00\x10"
    MSG_MENU_PREF_SCREENMODE_STR "\x00\x00"
    "\x00\x00\x00\x1B\x00\x0C"
    MSG_MENU_PARAMETERS_STR "\x00\x00"
    "\x00\x00\x00\x1C\x00\x0C"
    MSG_MENU_PAR_LOADALL_STR "\x00"
    "\x00\x00\x00\x1D\x00\x0C"
    MSG_MENU_PAR_SAVEALL_STR "\x00"
    "\x00\x00\x00\x1E\x00\x08"
    MSG_MENU_PAR_FREEZE_STR "\x00\x00"
    "\x00\x00\x00\x1F\x00\x08"
    MSG_MENU_PAR_RESTORE_STR "\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/



#endif /* WCS_STRINGS_H */

