/*
 * MSG_Test.c
 *
 *  Created on: Jul 4, 2025
 *      Author: afritsch
 */

#include "MSG_Test.h"
#define CATCOMP_NUMBERS 1
#include "WCS.h"
#include "WCS_locale.h"
#include "Version.h"
#include "GenericParams.h"
#include "TimeLinesGUI.h"
#include "Wave.h"

#ifdef BETA_USER_MESSAGE_TEST   // is set in Version.h
void ShowTestNumbers(unsigned int TestNumber, unsigned int TotalNumber)
{
    struct RastPort *rp = &WCSScrn->RastPort;
    static char TextBuffer[32];   // Room for "604/604 (100.00%)"

    SetAPen(rp, 1); // Vordergrundfarbe

    sprintf(TextBuffer, "%u/%u (%.2f%%)", TestNumber, TotalNumber, (float)TestNumber/TotalNumber*100.0f); // Formatierter Text
    Move(rp, WCSScrn->Width/2-TextLength(rp,TextBuffer,12)/2, WCSScrn->BarHeight-WCSScrn->Font->ta_YSize/2 +1);
    Text(rp, TextBuffer, strlen(TextBuffer)); // Text ausgeben
} /* TestNumbers() */

void Test_User_Message(unsigned int StartTestNumber)
{
    unsigned int TotalTests=532; // TotalTests is the number of User_Message() calls in WCS.c

    if(StartTestNumber<1)
    {
        StartTestNumber=1;
    }

    switch(StartTestNumber)
    {
        default:
            printf("Test_User_Message: Invalid Test_User_Message Number %u\n", StartTestNumber);
            return; // Invalid StartTestNumber, exit the function

            // awk '/BEGIN USER_MESSAGE_TEST/{Start=1; next} /\/\/.*find/{next} /User_Message/{if(Start==1){print $0}}' "WCS.c" | wc -l
            // Count Test-numbers starting from here...   results in 604

            // find . -name "*.c" -exec grep -nHis "User_Message" {} \; | awk -F":" '{print $1}' | sort --unique
            //./AGUI.c
            // find . -name "AGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 1:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Parameters Module", "Out of memory!", "OK",

        case 2:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_AGUI_CREATEDEFAULTPARAMETERSFORDATABASEALLCURRENTPARAMETERS ) , "dbasename");  // "Create Default Parameters for Database %s? All current Parameters will be overwritten."
            User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) , (CONST_STRPTR)str, (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "Parameter Editing: Defaults", str, "OK|Cancel"


        case 3:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) ,  // "Parameter Editing: Defaults"
                    (CONST_STRPTR) GetString( MSG_AGUI_YOUMUSTFIRSTLOADADATABASEBEFOREDEFAULTPARAMETERSCANBEC ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "You must first load a Database before Default Parameters can be computed.", "OK"

        case 4:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Database Module", "Out of memory!", "OK"

        case 5:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_DATAOPSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "DataOps Module", "Out of memory!", "OK"

        case 6:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_PUBLICSCREENSTILLHASVISITORSTRYCLOSINGAGAIN ) ,  // "Public Screen still has visitors. Try closing again?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2);  // "Close|Warn|Cancel"

        case 7:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_QUITPROGRAMREYOUSURE ) ,  // )"Quit Program\nAre you sure?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ) , (CONST_STRPTR)"owc", 2);  // "Close|Warn|Cancel"

        case 8:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_WCSPROJECT ) ,  // "WCS Project"
                    (CONST_STRPTR) GetString( MSG_AGUI_PROJECTPATHSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Project paths have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

        case 9:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERMODULE ) ,  // "Parameter Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Parameters have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

        case 10:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_DATABASEHASBEENMODIFIEDSAVEITBEFORECLOSING ) ,  // "Database has been modified. Save it before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc", 1);  // "OK|Cancel"

        case 11:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_EXTRASMODULE ), (CONST_STRPTR) GetString( MSG_AGUI_NOTYETIMPLEMENTEDTAYTUNED ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "Not yet implemented.\nStay Tuned!", "OK"

        case 12:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def("", (CONST_STRPTR) GetString( MSG_AGUI_KEEPCHANGES ) , (CONST_STRPTR) GetString( MSG_AGUI_KEEPCANCEL ) , (CONST_STRPTR)"kc", 1);  // "Keep changes?", "Keep|Cancel"

        case 13:
            //IncAndShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_EXTRASMODULE ), (CONST_STRPTR)"loadmesg", (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,(CONST_STRPTR)"o");  // "OK"

        case 14:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def("",  GetString( MSG_AGUI_FILEALREADYEXISTSOYOUWISHTOOVERWRITEIT ) ,  // "File already exists.\nDo you wish to overwrite it?"
                    GetString( MSG_GLOBAL_OKCANCEL ) , "oc", 1);  // "OK|CANCEL"

        case 15:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGSTATUSMODULE ) , (CONST_STRPTR) GetString( MSG_AGUI_CANTOPENLOGSTATUSWINDOW ) ,  // "Log Status Module", "Can't Open Log Status Window!"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "OK"

        case 16:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_LOGWINDOW ) , (CONST_STRPTR) GetString( MSG_AGUI_OUTOFMEMORY ) , (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Log Window", "Out of memory!", "OK"

        case 17:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_AGUI_WCSSCREENMODE ) ,                           // "WCS: Screen Mode"
                    GetString( MSG_AGUI_INORDERTORESETTHESCREENMODEWCSWILLHAVETOCLOSEANDREOPEN ),  // "In order to reset the screen mode WCS will have to close and re-open. Any work in progress should be saved before invoking this command.\nDo you wish to proceed now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),   // "OK|Cancel"
                    (CONST_STRPTR)"oc");

            //./AutoSelfTest.c   // for test only. No Strings to checked

            //./BitMaps.c
            //find . -name "BitMaps.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 18:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"Image.iff",
                    GetString( MSG_BITMAPS_FILEALREADYEXISTSVERWRITEIT ) , GetString( MSG_GLOBAL_OKCANCEL ) , (CONST_STRPTR)"oc");  // "File already exists!\nOverwrite it?" "OK|CANCEL"

        case 19:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"Image.iff",
                    GetString( MSG_BITMAPS_CANTOPENIMAGEFILEFOROUTPUTPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Can't open image file for output!\nOperation terminated.", "OK"


        case 20:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"Image.iff", GetString( MSG_BITMAPS_ERRORSAVINGIMAGEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error saving image!\nOperation terminated." "OK"

        case 21:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERRORLOADINGZBUFFERPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error loading Z Buffer!\nOperation terminated." "OK"

        case 22:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_OUTOFMEMORYMERGINGZBUFFERPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Out of memory merging Z Buffer!\nOperation terminated." "OK"

        case 23:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERROROPENINGZBUFFERFILEFORINPUTPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error opening Z Buffer file for input!\nOperation terminated." "OK"

        case 24:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADZBUFFILENOTSINGLEPRECISIONFLOATINGPOI ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNot single precision floating point.\nOperation terminated." "OK"

        case 25:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADZBUFFILENOZBODCHUNKOPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNo ZBOD chunk.\nOperation terminated." "OK"

        case 26:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADZBUFFILENOZBUFCHUNKOPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nNo ZBUF chunk.\nOperation terminated." "OK"

        case 27:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERRORREADINGZBUFFERFILERONGSIZEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Z Buffer file!\nWrong Size.\nOperation terminated." "OK"

        case 28:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERRORLOADINGBACKGROUNDIMAGEPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error loading background image!\nOperation terminated." "OK"

        case 29:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_OUTOFMEMORYMERGINGBACKGROUNDPERATIONTERMINATED ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Out of memory merging background!\nOperation terminated." "OK"

        case 30:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERROROPENINGBACKGROUNDFILEFORINPUTPERATIONTERMINATE ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error opening Background file for input!\nOperation terminated." "OK"

        case 31:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADBCKGRNDWRONGSIZEPERATIONTERMINATE ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nWrong Size.\nOperation terminated." "OK"

        case 32:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADBCKGRNDNOBODYCHUNKPERATIONTERMINA ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nNo BODY Chunk.\nOperation terminated." "OK"

        case 33:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERREADBCKGRNDNOBMHDCHUNKPERATIONTERMINA ) , GetString( MSG_GLOBAL_OK ) , (CONST_STRPTR)"o");  // "Error reading Background file!\nNo BMHD Chunk.\nOperation terminated." "OK"

        case 34:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_BITMAPS_ERRORREADINGBACKGROUNDFILEOMPRESSIONERRORPERATIONTE ) , GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");  // "Error reading Background file!\nCompression error.\nOperation terminated." "OK"


            //./Cloud.c
            //   find . -name "Cloud.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 35:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR) GetString( MSG_CLOUD_CLOUDEDITORSETBOUNDS ) ,     // "Cloud Editor:Set Bounds"
                    (CONST_STRPTR) GetString( MSG_CLOUD_MAPVIEWMODULEMUSTBEOPEN ) ,  // "Map View Module must be open in order to use this function. Would you like to open it now?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ) ,                 // "OK|Cancel"
                    (CONST_STRPTR)"oc",1);

        case 36:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),               // "Mapping Module: Align"
                    GetString( MSG_CLOUD_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSET ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                      // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

            //./CloudGUI.c
            // find . -name "CloudGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 37:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR) GetString( MSG_CLOUDGUI_MAPVIEWCLOUDS ) ,  // "Map View: Clouds"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OUTOFMEMORY ) ,    // "Out of memory!"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,             // "OK"
                    (CONST_STRPTR)"o");

        case 38:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ) ,  // "Parameters Module: Model"
                    GetString( MSG_CLOUDGUI_THECURRENTCLOUDMODELHASBEENMODIFIEDDOYOUWISHTOSAVE ) ,  // "The current Cloud Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ) ,  // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 39:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,               // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ), // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 40:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,     // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_DELETEALLCLOUDKEYFRAMES ) ,  // "Delete all cloud key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ) ,                 // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 41:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ),                             // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 42:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ) ,                            // "Yes|No",
                    (CONST_STRPTR)"yn", 1);


            //./DEM.c
            //find . -name "DEM.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 43:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSDEMINTERPOLATE ) ,  // "Data Ops: DEM Interpolate"
                    GetString( MSG_DEM_NOFILESSELECTED ) ,             // "No file(s) selected!"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,           // "OK",
                    (CONST_STRPTR)"o");

        case 44:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"elevfile",
                    GetString( MSG_DEM_ERROROPENINGFILEFORINTERPOLATIONILENOTDEMORREMONTINUE ) ,  // "Error opening file for interpolation!\nFile not DEM or REM\nContinue?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 45:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"rootfile",
                    GetString( MSG_DEM_DEMNAMEISTOOLONGTOADDANEXTRACHARACTERTODOYOUWISHTOENTER ) ,  // "DEM name is too long to add an extra character to. Do you wish to enter a new base name for the DEM or abort the interpolation?"
                    GetString( MSG_DEM_NEWNAMEABORT ),                                              // "New Name|Abort"
                    (CONST_STRPTR)"na", 1);

        case 46:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,  // "Data Ops: Interpolate DEM"
                    GetString( MSG_DEM_ERRORREADINGELEVATIONFILEONTINUE ) ,  // "Error reading elevation file!\nContinue?",
                    GetString( MSG_GLOBAL_OKCANCEL ),                           // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 47:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,         // "Data Ops: Interpolate DEM"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 48:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                           // "Data Ops: Interpolate DEM"
                    GetString( MSG_DEM_ERROROPENINGDEMFILEFOROUTPUTPERATIONTERMINATED ) ,  // "Error opening DEM file for output!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                              // "OK",
                    (CONST_STRPTR)"o");

        case 49:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                  // "Data Ops: Interpolate DEM"
                    GetString( MSG_DEM_ERRORWRITINGDEMFILEPERATIONTERMINATED ) ,  // "Error writing DEM file!\nOperation terminated."
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OK ),                       // "OK"
                    (CONST_STRPTR)"o");

        case 50:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                 // "Database Module",
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK",
                    (CONST_STRPTR)"o");

        case 51:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                           // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK",
                    (CONST_STRPTR)"o");

        case 52:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEM_DATAOPSINTERPOLATEDEM ) ,                         // "Data Ops: Interpolate DEM"
                    GetString( MSG_DEM_ERROROPENINGOBJECTFILEFOROUTPUTPERATIONTERMINATED ) ,  // "Error opening Object file for output!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                                 // "OK"
                    (CONST_STRPTR)"o");

        case 53:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_OUTOFMEMALLOCDEMINFOHEADERPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Info Header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 54:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_OUTOFMEMALLOCDEMINFOHEADERPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Info Header!\nOperation terminated."
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OK ) ,                                     // "OK"
                    (CONST_STRPTR)"o");

        case 55:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_75MINUTEDEMSDONOTALLLIEWITHINSAMEUTMZONEPERATIONTERMINA ) ,  // "7.5 Minute DEMs do not all lie within same UTM Zone!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                                       // "OK"
                    (CONST_STRPTR)"o");

        case 56:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_OUTOFMEMORYALLOCATINGDEMARRAYSPERATIONTERMINATED ) ,  // "Out of memory allocating DEM Arrays!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                                // "OK"
                    (CONST_STRPTR)"o");

        case 57:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_CANTREADDEMPROFILEHEADERPERATIONTERMINATED ) ,  // "Can't read DEM profile header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                          // "OK"
                    (CONST_STRPTR)"o");

        case 58:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 59:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 60:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_CANTREADDEMPROFILEHEADERPERATIONTERMINATED ),  // "Can't read DEM profile header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 61:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_OUTOFMEMORYALLOCATINGTEMPORARYBUFFERPERATIONTERMINATED ),  // "Out of memory allocating temporary buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 62:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_ERRORREADINGDEMPROFILEPERATIONTERMINATED ),  // "Error reading DEM profile!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK",
                    (CONST_STRPTR)"o");

        case 63:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_ERRORREADINGDEMPROFILEHEADERPERATIONTERMINATED ),  // "Error reading DEM profile header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 64:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_IMPROPERDEMPROFILELENGTHPERATIONTERMINATED ),  // "Improper DEM profile length!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK",
                    (CONST_STRPTR)"o");

        case 65:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_OUTOFMEMORYALLOCATINGMAPBUFFERPERATIONTERMINATED ),  // "Out of memory allocating map buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                //  "OK"
                    (CONST_STRPTR)"o");

        case 66:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_ERRORCREATINGOUTPUTFILEPERATIONTERMINATED ),  // "Error creating output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 67:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_NOFILESSELECTED ),  // "No file(s) selected!",
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");

        case 68:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_CANTOPENDEMFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DEM file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 69:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"MsgHdr",
                    GetString( MSG_DEM_CANTREADDEMFILEHEADERPERATIONTERMINATED ),  // "Can't read DEM file header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 70:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"FileBase",
                    GetString( MSG_DEM_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 71:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"FileBase",
                    GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK",
                    (CONST_STRPTR)"o");

        case 72:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"FileBase",
                    GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 73:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                             // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK",
                    (CONST_STRPTR)"o");

        case 74:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                     // "Database Module"
                    GetString( MSG_DEM_OUTOFMEMORYEXPANDINGDATABASEEDITORLIST ),  // "Out of memory expanding Database Editor List!"
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 75:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"FileBase",
                    GetString( MSG_DEM_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 76:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
                    GetString( MSG_DEM_BADARRAYDIMENSIONSSOMETHINGDOESNTCOMPUTEPERATIONTERMINA ),  // "Bad array dimensions! Something doesn't compute.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                       // "OK"
                    (CONST_STRPTR)"o");

        case 77:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),     // "Mapping Module: Fix Flats"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 78:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                // "Mapping Module: Fix Flats"
                    GetString( MSG_DEM_NOFLATSPOTSTOOPERATEONPERATIONTERMINATED ),  // "No flat spots to operate on!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

            //./DEMGUI.c
            // find . -name "DEMGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 79:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),  // "Map View: Build DEM"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");

        case 80:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                       // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_THISWINDOWMUSTREMAINOPENWHILETHEDEMGRIDDERISOPENOYOU ),  // "This window must remain open while the DEM Gridder is open!\nDo you wish to close them both?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 81:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWDEMGRIDDER ),  // "Map View: DEM Gridder"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),        // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                 // "OK"
                    (CONST_STRPTR)"o");

        case 82:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,                         // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_SELECTCONTOUROBJECTSTOIMPORTANDRESELECT ),  // "Select contour objects to import and reselect \"Import\" when done."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 83:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWEXPORTCONTOURS ) ,                          // "Map View: Export Contours",
                    GetString( MSG_DEMGUI_CANTOPENDATABASEEDITORWINDOWPERATIONTERMINATED ),  // "Can't open Database Editor window!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                             // "OK",
                    (CONST_STRPTR)"o");

        case 84:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWEXPORTCONTOURS ) ,                                // "Map View: Export Contours"
                    GetString( MSG_DEMGUI_EXTRACTELEVATIONVALUESFROMOBJECTNAMESLABELFIELDSORUS ),  // "Extract elevation values from Object Names, Label fields or use the values embedded in the Objects themselves?"
                    GetString( MSG_DEMGUI_NAMELABELEMBEDDED ),                                     // "Name|Label|Embedded"
                    (CONST_STRPTR)"nle");

        case 85:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,  // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_ERRORIMPORTINGCONTOURDATAPERATIONTERMINATED ) ,  // "Error importing contour data!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,  // "OK",
                    (CONST_STRPTR)"o");

        case 86:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                       // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_ATLEASTONEOBJECTFAILEDTOLOADANDCOULDNOTBEIMPORTED ),  // "At least one Object failed to load and could not be imported."
                    GetString( MSG_GLOBAL_OK ),                                                 // "OK"
                    (CONST_STRPTR)"o");

        case 87:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWIMPORTCONTOURS ),         // "Map View: Import Contours"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK",
                    (CONST_STRPTR)"o");

        case 88:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                 // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_YOUDIDNOTSELECTAFILETOIMPORTPERATIONTERMINATED ),  // "You did not select a file to import!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK",
                    (CONST_STRPTR)"o");

        case 89:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                      // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_UTMZONESMAYBEFROM0TO60THESELECTEDZONEISOUTOFRANGEPER ),  // "UTM zones may be from 0 to 60! The selected zone is out of range.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    //  "OK"
                    (CONST_STRPTR)"o");

        case 90:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),  // "Map View: Build DEM"
                    GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),   // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                // "OK"
                    (CONST_STRPTR)"o");

        case 91:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ) ,                               // "Map View: Build DEM"
                    GetString( MSG_DEMGUI_ERROROPENINGXYZFILETOIMPORTPERATIONTERMINATED ),  // "Error opening XYZ file to import!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 92:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ) ,                                 // "Map View: XYZ Export"
                    GetString( MSG_DATAOPS_YOUMUSTSPECIFYANOUTPUTFILENAMEPERATIONTERMINATED ),  // "You must specify an output file name!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                // "OK"
                    (CONST_STRPTR)"o");

        case 93:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ),                                     // "Map View: XYZ Export"
                    GetString( MSG_DEMGUI_ERRORWRITINGTOXYZFILEPARTIALFILEWRITTENPERATIONTERMI ),  // "Error writing to XYZ file! Partial file written.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 94:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message( GetString( MSG_DEMGUI_MAPVIEWXYZEXPORT ),                                // "Map View: XYZ Export"
                    GetString( MSG_DEMGUI_UNABLETOOPENXYZFILEFOREXPORTPERATIONTERMINATED ),  // "Unable to open XYZ file for export!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK",
                    (CONST_STRPTR)"o");

            //./DLG.c
            //find . -name "DLG.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 95:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                                  // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK",
                    (CONST_STRPTR)"o");

        case 96:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),  // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_NOFILESSELECTED ),         // "No file(s) selected!"
                    GetString( MSG_GLOBAL_OK ),                      // "OK",
                    (CONST_STRPTR)"o");

        case 97:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                     // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_CANTOPENDLGFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DLG file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 98:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                     // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_FILENOTAUSGSOPTIONALDLGPERATIONTERMINATED ),  // "File not a USGS Optional DLG!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 99:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                  // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_INAPPROPRIATEUTMZONEPERATIONTERMINATED ),  // "Inappropriate UTM Zone!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 100:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),                                   // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_THISFILECONTAINSDATAINANUNSUPPORTEDREFERENCESYSTEMPERAT ),  // "This file contains data in an unsupported Reference System!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                       // "OK"
                    (CONST_STRPTR)"o");

        case 101:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                             // "Database Module",
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 102:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDLG ),  // "Data Ops Module: Import DLG"
                    GetString( MSG_DLG_ERRORSAVINGOBJECTFILEPERATIONTERMINATED ),  // "Error saving object file!\nOperation terminated"
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 103:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                             // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ) ,  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 104:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                                  // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 105:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),  // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),  // "No file(s) selected!"
                    GetString( MSG_GLOBAL_OK ),                      // "OK"
                    (CONST_STRPTR)"o");

        case 106:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                     // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_CANTOPENDXFFILEFORINPUTPERATIONTERMINATED ),  // "Can't open DXF file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 107:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message( GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),                               // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_IMPROPERCODEVALUEFOUNDPERATIONTERMINATEDPREMATURELY ),  // "Improper Code value found!\nOperation terminated prematurely."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 108:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 109:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ) ,                                           // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK
                    (CONST_STRPTR)"o");

        case 110:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),               // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_ERRORSAVINGOBJECTPERATIONTERMINATED ),  // "Error saving object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                   // "OK"
                    (CONST_STRPTR)"o");

        case 111:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module",
                    GetString( MSG_DLG_OUTOFMEMORYEXPANDINGDATABASEEDITORLISTASTITEMDOESNOTAPP ),  // "Out of memory expanding Database Editor List!\nLast item does not appear in list view."
                    GetString( MSG_GLOBAL_OK ),                                                       // "OK",
                    (CONST_STRPTR)"o");

        case 112:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTDXF ),              // "Data Ops Module: Import DXF"
                    GetString( MSG_DLG_ERRORSAVINGLASTOBJECTPERATIONTERMINATED ),  // "Error saving last object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 113:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),                                  // "Data Ops Module: Import WDB"
                    GetString( MSG_DLG_OUTOFMEMORYALLOCATINGTEMPORARYARRAYSPERATIONTERMINATED ),  // "Out of memory allocating temporary arrays!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),
                    (CONST_STRPTR)"o");

        case 114:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),  // "Data Ops Module: Import WDB"
                    GetString( MSG_DLG_NOFILESSELECTED ),              // "No file(s) selected!"
                    GetString( MSG_GLOBAL_OK ),                           // "OK"
                    (CONST_STRPTR)"o");

        case 115:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSMODULEIMPORTWDB ),                     // "Data Ops Module: Import WDB"
                    GetString( MSG_DLG_CANTOPENWDBFILEFORINPUTPERATIONTERMINATED ),  // "Can't open WDB file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 116:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 117:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                                       // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 118:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                            // "Database Module"
                    GetString( MSG_DLG_OUTOFMEMXPDBEDITORLISTOPERATIONTERMINATE ),  // "Out of memory expanding Database Editor List!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 119:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                // "Data Ops: Import WDB"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ) ,  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                              // "OK"
                    (CONST_STRPTR)"o");

        case 120:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                          // "Data Ops: Import WDB"
                    GetString( MSG_DLG_ERROROPENINGSOURCEFILEPERATIONTERMINATED ),  // "Error opening source file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 121:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                     // "Data Ops: Import WDB"
                    GetString( MSG_DLG_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 122:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                         // "Data Ops: Import WDB"
                    GetString( MSG_DLG_ERRORSAVINGOBJECTFILEPERATIONTERMINATED ),  // "Error saving object file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 123:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                            // "Data Ops: Import WDB"
                    GetString( MSG_DLG_UNSUPPORTEDATTRIBUTECODEPERATIONTERMINATED ),  // "Unsupported attribute code!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 124:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                               // "Data Ops: Import WDB"
                    GetString( MSG_DLG_OBJECTCONTAINSTOOMANYPOINTSPERATIONTERMINATED ),  // "Object contains too many points!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 125:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DLG_DATAOPSIMPORTWDB ),                                // "Data Ops: Import WDB"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

            //./DataBase.c
            // find . -name "DataBase.c" -exec grep -A3 -nHis "User_Message" {} \;

        case 126:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)dbasename, GetString( MSG_DB_ERRORSAVINGDATABASEELECTANEWDIRECTORY ),  //"Error saving database!\nSelect a new directory?"
                    GetString( MSG_DB_OKCANCEL ),  // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 127:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_DB_DATABASEMODULESAVE ), (CONST_STRPTR)"Database-File",                   // "Database Module: Save"
                    GetString( MSG_DB_OKCANCEL ),                                                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 128:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
                    GetString( MSG_DB_ILLEGALNUMBEROFDATABASERECORDSLESSTHANONEPERATIONTERMINA ), // "Illegal number of database records: less than one!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 129:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),  // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYANTUPDATEDATABASELIST ),  // "Out of memory!\nCan't update database list."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 130:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_DB_ATLEASTONEVECTORFILEWASFOUNDTOCONTAINANUMBEROFPOINTSDIFF ));  // "At least one vector file was found to contain a number of points different from that in its Database record!\nThe record has been updated.\nDatabase should be re-saved."
            User_Message(GetString( MSG_DB_MAPVIEWLOAD ),  // "Map View: Load"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),           // "OK",
                    (CONST_STRPTR)"o");

        case 131:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGELEVATIONSOBJECTNOTLOADED ), // "Error reading elevations! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 132:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGLATITUDESOBJECTNOTLOADED ),  // "Error reading latitudes! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 133:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGLONGITUDESOBJECTNOTLOADED ),  // "Error reading longitudes! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 134:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_OUTOFMEMORYOBJECTNOTLOADED ),              // "Out of memory! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 135:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_ERRORREADINGHEADEROBJECTNOTLOADED ),        // "Error reading header! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 136:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_UNSUPPORTEDFILEVERSIONOBJECTNOTLOADED ),     // "Unsupported file version! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 137:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name", GetString( MSG_DB_OUTOFMEMORYOBJECTNOTLOADED ),  // "Out of memory! Object not loaded."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 138:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_DB_DATABASEMODULENAME ),                            // "Database Module: Name"
                    GetString( MSG_DB_VECTORNAMEALREADYPRESENTINDATABASERYANEWNAME ),  // "Vector name already present in database!\nTry a new name?"
                    GetString( MSG_DB_OKCANCEL ),                                      // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 139:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 140:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
                    GetString( MSG_DB_NOMEMORYFORVECTORCOORDINATESEWOBJECTHASBEENCREATEDBUTCAN ),  // "No memory for vector coordinates!\nNew object has been created but can not be edited until memory is available."
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK"
                    (CONST_STRPTR)"o");

        case 141:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
                    GetString( MSG_DB_OUTOFMEXPDBASEEDITORLSTNEWOBJECTHASBEENCRE ),  // "Out of memory expanding Database Editor List!\nNew object has been created but will not appear in list view."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 142:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_DATABASEADDOBJECT ),  // "Database: Add Object"
                    GetString( MSG_DB_NOFILESSELECTED ),    // "No file(s) selected!"
                    GetString( MSG_GLOBAL_OK ),                 // "OK"
                    (CONST_STRPTR)"o");

        case 143:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"newfile", GetString( MSG_DB_OBJECTMUSTENDINSUFFIXOBJ ),  // "Object must end in suffix \"Obj\"!"
                    GetString( MSG_GLOBAL_OK ),  // (CONST_STRPTR)"OK"
                    (CONST_STRPTR)"o");

        case 144:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str,"DBase[j].Name");
            User_Message_Def((CONST_STRPTR)str,
                    GetString( MSG_DB_OBJECTNAMEALREADYPRESENTINDATABASEUPLICATEITEMSWILLBESKI ),  // "Object name already present in database!\nDuplicate items will be skipped."
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK"
                    (CONST_STRPTR)"o", 1);

        case 145:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                 // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ), // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 146:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_DATABASEMODULEEDITOR ),                                      // "Database Module: Editor"
                    GetString( MSG_DB_OUTOFMEMEXPDBEDITLSTNEWOBJADDED ),  // "Out of memory expanding Database Editor List!\nNew object has been added but will not appear in list view.",
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK"
                    (CONST_STRPTR)"o");

        case 147:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_DB_ERRORLOADINGTHISOBJECTPERATIONTERMINATED ),  // "Error loading this Object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 148:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_MAPVIEWSAVEALL ),                                  // "Map View: Save All"
                    GetString( MSG_DB_ERRORWRITINGMASTEROBJECTFILEPERATIONTERMINATED ),  // "Error writing Master Object file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 149:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                               // "Map View: Load"
                    GetString( MSG_DB_OUTOFMEMORYLOADINGMASTEROBJECTFILENABLEDOBJECTSWILLBELOA ),  // "Out of memory loading Master Object File!\nEnabled Objects will be loaded individually."
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK"
                    (CONST_STRPTR)"o");

        case 150:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                     // "Map View: Load"
                    GetString( MSG_DB_ERRORREADINGMASTEROBJECTFILEPERATIONTERMINATED ),  // "Error reading Master Object file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 151:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                                               // "Map View: Load"
                    GetString( MSG_DB_NUMBEROFOBJECTSINTHEMASTEROBJECTFILEDOESNOTMATCHTHENUMBE ),  // "Number of Objects in the Master Object file does not match the number of Objects in the current Database! Master Object file cannot be used. Objects will be loaded from individual files"
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK",
                    (CONST_STRPTR)"o");

        case 152:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DB_MAPVIEWLOAD ),                   // "Map View: Load"
                    GetString( MSG_DB_MDBISNOTAWCSMASTEROBJECTFILE ),  // ".MDB is not a WCS Master Object file!"
                    GetString( MSG_GLOBAL_OK ),                            // "OK"
                    (CONST_STRPTR)"o");

            //./DataOps.c
            // find . -name "DataOps.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 153:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                              // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_YOUMUSTSPECIFYAFILETOCONVERTPERATIONTERMINATED ),  // "You must specify a file to convert!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 154:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_YOUMUSTSPECIFYANOUTPUTFILENAMEPERATIONTERMINATED ),  // "You must specify an output file name!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                // "OK"
                    (CONST_STRPTR)"o");

        case 155:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_YOUMUSTSPECIFYINPUTROWSANDCOLUMNSPERATIONTERMINATED ),   // "You must specify input rows and columns!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 156:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,                                   // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_THEREISNODATABASETODIRECTOUTPUTENTITIESTOPERATIONTE ),  // "There is no Database to direct output entities to!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 157:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_DATAOPS_INPUTDATACANNOTBEEQUALLYDIVIDEDAMONGOUTPUTMAPSASTCO ),  // "Input data cannot be equally divided among output maps.\nLast Column of maps will have %ld columns.\nLast Row of maps will have %ld rows."
                    320, 200);  // Example values for columns and rows
            User_Message_Def(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),     // "Data Ops: Convert DEM"
                    (CONST_STRPTR)str,
                    GetString( MSG_DATAOPS_CONTINUETRUNCATECANCEL ),   // "Continue|Truncate|Cancel"
                    (CONST_STRPTR)"ntc", 1);

        case 158:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INCORRECTFILESIZEFORSPECIFIEDHEADERWIDTHANDHEIGHTRO ),  // "Incorrect file size for specified header, width and height!\nProceed anyway?."
                    GetString( MSG_GLOBAL_OKCANCEL ) ,                                            // "OK|Cancel",
                    (CONST_STRPTR)"oc");

        case 159:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),  // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INVERTDATAORDER ),    // "Invert Data order?"
                    GetString( MSG_GLOBAL_YESNO ),              // "Yes|No"
                    (CONST_STRPTR)"yn");

        case 160:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 161:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                           // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_UNABLETOOPENFILEFORINPUTPERATIONTERMINATED ),  // "Unable to open file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 162:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INCORRFILSIZFORSPECIFHEADERWDTHANDHIGHTPE ),  // "Incorrect file size for specified header, width and height!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 163:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                            // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_UNABLETOOPENFILEFOROUTPUTPERATIONTERMINATED ),  // "Unable to open file for output!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 164:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                              // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_ERRORWRITINGDESTINATIONFILEPERATIONTERMINATED ),  // "Error writing destination file!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 165:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                         // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_ERRORREADINGSOURCEFILEPERATIONTERMINATED ),  // "Error reading source file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 166:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                     // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_NOTACOMPRESSEDFILEPERATIONTERMINATED ),  // "Not a compressed file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                    // "OK"
                    (CONST_STRPTR)"o");

        case 167:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
                    (CONST_STRPTR)MSG_DATAOPS_EXTENDEDHEADEROPERATIONTERMINATED,  // "Extended header!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK",
                    (CONST_STRPTR)"o");

        case 168:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INPUTFILECONFIGURATIONNOTYETSUPPORTEDPERATIONTERMIN ),  // "Input file configuration not yet supported!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 169:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INPUTDATAFORMATNOTSUPPORTEDHECKYOURSETTINGSPERATION ),  // "Input data format not supported!\nCheck your settings.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK",
                    (CONST_STRPTR)"o");

        case 170:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 171:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                       // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_ERRORSAVINGOBJFILEOPERATIONTERMOINATED ),  // "Error saving \".Obj\" file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 172:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_INPUTFILENOTRECOGNIZEDASADTEDFILEPERATIONTERMINATED ),  // "Input file not recognized as a DTED file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 173:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_LLEGALSOURCEVALUEFORMATSIZECOMBINATIONPERATIONTERMI ),  // "!\nIllegal source value format/size combination!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 174:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                                    // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPS_LLEGALTARGETVALUEFORMATSIZECOMBINATIONPERATIONTERMI ),  // "!\nIllegal target value format/size combination!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

            //./DataOpsGUI.c
            // find . -name "DataOpsGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 175:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DEMCONVERTER ),  // "DEM Converter"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),            // "OK"
                    (CONST_STRPTR)"o");

        case 176:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)DC_Win->InFile,
                    GetString( MSG_DATAOPSGUI_UNABLETOOPENFILEFORINPUT ),  // "Unable to open file for input!\n"
                    GetString( MSG_GLOBAL_OK ),                        // "OK"
                    (CONST_STRPTR)"o");

        case 177:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)DC_Win->InFile,
                    GetString( MSG_DATAOPSGUI_UNABLETOREADFILESIZE ),  // "Unable to read file size!\n"
                    GetString( MSG_GLOBAL_OK ),                    // "OK"
                    (CONST_STRPTR)"o");

        case 178:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERT ) ,            // "Data Ops: Convert"
                    GetString( MSG_DATAOPSGUI_WARNINGILEISNOTAWCSDEMFILE ),  // "Warning!\nFile is not a WCS DEM file."
                    GetString( MSG_GLOBAL_OK ),                          // "OK"
                    (CONST_STRPTR)"o");

        case 179:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERT ),                   // "Data Ops: Convert"
                    GetString( MSG_DATAOPSGUI_WARNINGILEISNOTANIFFZBUFFERFILE ),  // "Warning!\nFile is not an IFF Z Buffer file."
                    GetString( MSG_GLOBAL_OK ),                               // "OK"
                    (CONST_STRPTR)"o");

        case 180:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),             // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_WARNINGILEISNOTAVISTADEMFILE ),  // "Warning\nFile is not a Vista DEM file."
                    GetString( MSG_GLOBAL_OK ),                            // "OK"
                    (CONST_STRPTR)"o");

        case 181:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),  // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_WARNINGILEISNOTACOMPRESSEDVISTAFILEANDCANNOTBEIM ),  // "Warning\nFile is not a compressed Vista file and cannot be imported."
                    GetString( MSG_GLOBAL_OK ),                                                // "OK"
                    (CONST_STRPTR)"o");

        case 182:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                 // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_ISTHISASMALLLARGEORHUGEVISTAFILE ),  // "Is this a Small, Large or Huge Vista file?"
                    GetString( MSG_DATAOPSGUI_SMALLLARGEHUGE ),                    // "Small|Large|Huge"
                    (CONST_STRPTR)"slh");

        case 183:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),         // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_WRNFILENOTIFF ),  // "Warning\nFile is not an IFF file."
                    GetString( MSG_GLOBAL_OK ),                        // "OK"
                    (CONST_STRPTR)"o");

        case 184:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),              // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_WRNFILENOTIFFIMAGFILE ),  // "Warning\nFile is not an IFF image file."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 185:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ) ,        // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_ERRORREADINGBITMAPHEADER ),  // "Error reading bitmap header."
                    GetString( MSG_GLOBAL_OK ),                        // "OK"
                    (CONST_STRPTR)"o");

        case 186:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),                     // "Data Ops: Convert DEM"
                    GetString( MSG_DATAOPSGUI_WARNINGILEISNOTRECOGNIZEDASADTEDFILE ),  // "Warning\nFile is not recognized as a DTED file."
                    GetString( MSG_GLOBAL_OK ),                                    // "OK"
                    (CONST_STRPTR)"o");

        case 187:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DATAOPSGUI_DEMINTERPOLATE ),  // "DEM Interpolate"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");


            //./DefaultParams.c
            //find . -name "DefaultParams.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 188:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                  // "Parameters Module: Defaults"
                    GetString( MSG_DEFPARM_PLEASEENABLEATLEASTONETOPODEMANDTRYAGAIN ),  // "Please enable at least one topo DEM and try again."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 189:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),                   // "Parameters Module: Defaults"
                    GetString( MSG_DEFPARM_PLEASECLOSEALLTIMELINESWINDOWSANDTRYAGAIN ),  // "Please close all Time Lines windows and try again."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 190:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DEFPARM_PARAMETERSMODULEDEFAULTS ),  // "Parameters Module: Defaults"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),               // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                        // "OK"
                    (CONST_STRPTR)"o");


            //./DiagnosticGUI.c
            // find . -name "DiagnosticGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 191:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DIAG_RENDERDATA ),   // "Render Data",
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK",
                    (CONST_STRPTR)"o");

            //./DispatchGUI.c
            //find . -name "DispatchGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 192:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str,"exampledirectory");
            User_Message_Def((CONST_STRPTR)str,
                    GetString( MSG_DISPGUI_MAKETHISTHEDEFAULTOBJECTDIRECTORY ),  // "Make this the default object directory?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                           // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 193:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                                // "Database: Load"
                    GetString( MSG_DISPGUI_ERROROPENINGDATABASEFILEPERATIONTERMINATED ),  // "Error opening Database file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 194:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                           // "Database: Load"
                    GetString( MSG_DISPGUI_NOTAWCSDATABASEFILEPERATIONTERMINATED ),  // "Not a WCS Database file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 195:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DISPGUI_DATABASELOAD ),                                // "Database: Load"
                    GetString( MSG_DISPGUI_ERRORREADINGDATABASEFILEPERATIONTERMINATED ),  // "Error reading Database file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 196:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_DISPGUI_DATABASEMODULELOAD ),                               // "Database Module: Load"
                    GetString( MSG_DISPGUI_OUTOFMEMORYALLOCATINGDATABASEPERATIONTERMINATED ),  // "Out of memory allocating Database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o");

            //./EdDBaseGUI.c
            // find . -name "EdDBaseGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 197:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDDB_DATABASEEDITOR ),                                         // "Database Editor"
                    GetString( MSG_EDDB_YOUMUSTFIRSTLOADORCREATEADATABASEBEFOREOPENINGTHEEDITO ), // "You must first load or create a database before opening the editor."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 198:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                    // "Database Module"
                    GetString( MSG_EDDB_OUTOFMEMORYANTOPENDATABASEWINDOW ),  // "Out of memory!\nCan't open database window.",
                    GetString( MSG_GLOBAL_OK ),                                // "OK"
                    (CONST_STRPTR)"o");

        case 199:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDDB_DATABASEEDITOR ),  // "Database Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");

        case 200:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_DB_DATABASEMODULENAME ),                             // "Database Module: Name"
                    GetString( MSG_EDDB_OBJECTNAMEALREADYPRESENTINDATABASERYANEWNAME ) ,  // "Object name already present in database!\nTry a new name?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 201:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDDB_DATABASEMODULEREMOVEITEM ),                                          // "Database Module: Remove Item
                    GetString( MSG_EDDB_DELETEOBJECTELEVATIONANDRELATIVEELEVATIONFILESFROMDISKASWELL ),      // "Delete object, elevation and relative elevation files from disk as well as remove their names from the Database?"
                    GetString( MSG_EDDB_FROMDISKDATABASEONLYCANCEL ),                                        // "From Disk|Database Only|Cancel",
                    (CONST_STRPTR)"fdc", 1);

        case 202:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                  // "Database Module"
                    GetString( MSG_EDDB_OUTOFMEMCANTOPENDBLIST ),          // "Out of memory!\nCan't open database list."
                    GetString( MSG_GLOBAL_OK ),                            // "OK"
                    (CONST_STRPTR)"o");

        case 203:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                         // "Database Module"
                    GetString( MSG_EDDB_OUTOFMEMORYANTOPENDIRECTORYLISTWINDOW ),  // "Out of memory!\nCan't open directory list window."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 204:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                         // "Database Module"
                    GetString( MSG_EDDB_OUTOFMEMORYANTOPENDIRECTORYLISTWINDOW ),  // "Out of memory!\nCan't open directory list window."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 205:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDDB_DIRECTORYLIST ),  // "Directory List"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),             // "OK"
                    (CONST_STRPTR)"o");

        case 206:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                    GetString( MSG_DB_OUTOFMEMORYEXPANDINGDATABASEOPERATIONTERMINATED ),  // "Out of memory expanding database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

            //./EdEcoGUI.c
            //find . -name "EdEcoGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 207:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),                                     // "Ecosystem Editor"
                    GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor.",
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 208:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),          // "Parameters Module: Ecosystem"
                    GetString( MSG_EDECOGUI_OUTOFMEMORYANTOPENECOSYSTEMEDITOR ),  // "Out of memory!\nCan't open Ecosystem Editor."
                    GetString( MSG_GLOBAL_OK ),                                 // "OK"
                    (CONST_STRPTR)"o");

        case 209:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMEDITOR ),  // "Ecosystem Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");

        case 210:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDMOGUI_DELETEALLKEYFRAMES ), PAR_NAME_ECO(EE_Win->EcoItem));  // "Delete all %s Key Frames?"
            User_Message_Def(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),                          // "Parameters Module: Ecosystem"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ),                                           // "OK|Cancel",
                    (CONST_STRPTR)"oc", 1);

        case 211:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDECOGUI_ECOSYSTEMPARAMETERSSWAP ) ,                         // "Ecosystem Parameters: Swap"
                    GetString( MSG_EDECOGUI_CANTSWAPWITHFIRST12ECOSYSTEMSPERATIONTERMINATED ),  // "Can't swap with first 12 ecosystems!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"oc");

            //./EdMoGUI.c
            //find . -name "EdMoGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 212:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),                                         // "Motion Editor"
                    GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 213:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_MOTIONEDITOR ),  // "Motion Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),            // "OK"
                    (CONST_STRPTR)"o");

        case 214:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_MOTIONEDITORAUTOCENTER ),                          // "Motion Editor: Auto Center"
                    GetString( MSG_EDMOGUI_INTERACTIVEMODULEMUSTBEOPENBEFOREAUTOCENTERING ),  // "Interactive module must be open before auto centering!"
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 215:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDMOGUI_DELETEALLKEYFRAMES ), varname[EM_Win->MoItem]);  // "Delete all %s Key Frames?"
            User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMOTION ),                       // "Parameters Module: Motion"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ),                                     // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 216:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EDMOGUI_MAKEKEYFRAMESFORCAMERAPARAMETERSALSO ) );  // "Make key frames for Camera Parameters also?"
            User_Message_Def(GetString( MSG_EDMOGUI_PARAMETERSMODULEMAKEKEY ),  // "Parameters Module: Make Key"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_YESNO ),                    // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 217:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),   // "Camera View"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 218:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEWASPECT ),                                     // "Camera View: Aspect"
                    GetString( MSG_EDMOGUI_COMPUTEDHEIGHTISLARGERTHANTHECURRENTSCREENHEIGHTDOY ),  // "Computed height is larger than the current screen height. Do you wish to use the screen height?"
                    GetString( MSG_GLOBAL_OKCANCEL),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 219:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_MOTIONPARAMLIST ),  // "Motion Param List"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");

            //./EdPar.c
            // find . -name "EdPar.c" -exec grep -A3 -nHis "User_Message" {} \;

        case 220:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEBANKKEYS ),             // "Parameters Module: Bank Keys"
                    GetString( MSG_EDPAR_KEYFRAMESEXISTFORTHEBANKPARAMETEROVERWRITETHEM ),  // "Key Frames exist for the "Bank" Parameter. Overwrite them?"
                    GetString( MSG_GLOBAL_OKCANCEL ), (CONST_STRPTR)"oc");                   // "OK|Cancel"

        case 221:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),              // "Parameters Module: Export"
                    GetString( MSG_EDPAR_ERRORCREATINGKEYFRAMEPERATIONTERMINATED ),  // "Error creating Key Frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 222:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),                    // "Parameters Module: Export"
                    GetString( MSG_EDPAR_NOCAMERAPATHLATLONKEYFRAMESPERATIONTERMINATED ),  // "No Camera Path Lat/Lon Key Frames!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 223:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERSMODULEEXPORT ),    // "Parameters Module: Export"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 224:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load",
                    GetString( MSG_EDPAR_UNSUPPORTEDPARAMETERFILETYPEORVERSIONPERATIONTERMINAT ),  // "Unsupported Parameter file type or version!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 225:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THISISANOLDV1FORMATFILEWOULDYOULIKETORESAVEITINTHENEW ),  // "This is an old V1 format file! Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 226:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THEPARAMETERFILEFORMATHASBEENCHANGEDSLIGHTLYSINCETHIS ),  // "The Parameter File format has been changed slightly since this file was saved. Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 227:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     //  "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO),                 // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 228:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 229:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_COLORITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_COLOR(0));  // "Color item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),  // "Color Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                      // "OK"
                    (CONST_STRPTR)"o");

        case 230:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_ECOSYSTEMITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_ECO(0));  // "Ecosystem item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
                    (CONST_STRPTR)"o");

        case 231:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 232:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     // "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO ),                // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 233:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),            // "Parameter Module: Load"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 234:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_COLORITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ),PAR_NAME_COLOR(0));  // "Color item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),                                                   // "Color Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                                                       // "OK",
                    (CONST_STRPTR)"o");

        case 235:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_COLORITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_COLOR(0));  // "Color item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_COLOREDITORLOADCURRENT ),                                                    // "Color Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                                                        // "OK"
                    (CONST_STRPTR)"o");

        case 236:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_ECOSYSTEMITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_ECO(0));  // "Ecosystem item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
                    (CONST_STRPTR)"o");

        case 237:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_ECOSYSTEMITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_ECO(0));  // "Ecosystem item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORLOADCURRENT ),                                                  // "Ecosystem Editor: Load Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                                                          // "OK"
                    (CONST_STRPTR)"o");

        case 238:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
                    GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 239:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
                    GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 240:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETEREDITINGMODULE ),                                 // "Parameter Editing Module"
                    GetString( MSG_EDPAR_PARTIALFILESMAYNOTBEWRITTENTOOLDFILEVERSIONSDOYOUWISH ),  // "Partial files may not be written to old file versions!\n\Do you wish to save the entire parameter file?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                               // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 241:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMFILE ),                         // "paramfile"
                    GetString( MSG_EDPAR_ERROROPENINGFILEFOROUTPUTRYAGAIN ),  // "Error opening file for output!\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                          // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 242:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_COLORITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ),  // "Color item %s not found in this file!\nOperation terminated."
                    PAR_NAME_COLOR(0));
            User_Message(GetString( MSG_EDPAR_COLOREDITORSAVECURRENT ),                         // "Color Editor: Save Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 243:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDPAR_ECOSYSTEMITEMNOTFOUNDINTHISFILEPERATIONTERMINATED ), PAR_NAME_ECO(0));  // "Ecosystem item %s not found in this file!\nOperation terminated."
            User_Message(GetString( MSG_EDPAR_ECOSYSTEMEDITORSAVECURRENT ),  // "Ecosystem Editor: Save Current"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),                          // "OK"
                    (CONST_STRPTR)"o");

        case 244:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMETERMODULESAVE ),     // "Parameter Module: Save"
                    GetString( MSG_EDPAR_SAVEALLKEYFRAMESASWELL ),  // "Save all key frames as well?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 245:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDPAR_PARAMFILE ),                                              // "paramfile"
                    GetString( MSG_EDPAR_ERRORWRITINGTOPARAMETERFILETHEOUTPUTFILEHASBEENMODIFI ),  // "Error writing to Parameter file!\n\The output file has been modified and may no longer be valid. Try resaving to a different device or freeing some disk space and saving again."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

            //./EdSetGUI.c
            //find . -name "EdSetGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 246:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_EDSETGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENIN ),  // "You must first load or create a parameter file before opening the Render Module."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK",
                    (CONST_STRPTR)"o");

        case 247:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDSETGUI_RENDERSETTINGSEDITOR ),  // "Render Settings Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                    // "OK"
                    (CONST_STRPTR)"o");

            //./EditGui.c
            // find . -name "EditGui.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 248:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),                                      // "Color Editor"
                    GetString( MSG_EDITGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENING ),  // "You must first load or create a parameter file before opening the Editor."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o");

        case 249:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDITGUI_COLOREDITOR ),  // "Color Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 250:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EDMOGUI_DELETEALLKEYFRAMES ), PAR_NAME_COLOR(EC_Win->PalItem));  // "Delete all %s Key Frames?"
            User_Message_Def(GetString( MSG_EDITGUI_PARAMETERSMODULECOLOR ),  // "Parameters Module: Color"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ),               // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 251:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDITGUI_COLOREDITORCOPY ) ,  // "Color Editor: Copy"
                    GetString( MSG_EDITGUI_COPYKEYFRAMESTOO ),   // "Copy Key Frames too?"
                    GetString( MSG_GLOBAL_YESNO ),              // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 252:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDITGUI_COLORPARAMETERSSWAP ),                          // "Color Parameters: Swap"
                    GetString( MSG_EDITGUI_CANTSWAPWITHFIRST24COLORSPERATIONTERMINATED ),  // "Can't swap with first 24 colors!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                          // "OK"
                    (CONST_STRPTR)"oc");

        case 253:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)PAR_NAME_ECO(1),
                    GetString( MSG_EDITGUI_THECURRENTCOLORISBEINGUSEDREMOVEITANYWAY ),  // "The current color is being used. Remove it anyway?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                  // "OK|Cancel"
                    (CONST_STRPTR)"oc", 0);

            //./EvenMoreGUI.c
            // find . -name "EvenMoreGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 254:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EVMORGUI_SUNTIMEWINDOW ) ,  // "Sun Time Window"
                    GetString( MSG_EVMORGUI_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");

        case 255:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_PROJECTNEWEDIT ),  // "Project: New/Edit"
                    GetString( MSG_EVMORGUI_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");

        case 256:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_YOUMUSTSUPPLYANEWPROJECTNAME ));  // "You must supply a new project name."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 257:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRORLOADINGPROJECTFILETOCLONE ) );  // "Error loading Project file to clone."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 258:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRORLOADINGWAVEFILETOCLONE ) );  // "Error loading Wave file to clone."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 259:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRORLOADINGCLOUDFILETOCLONE ) );  // "Error loading Cloud file to clone."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 260:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString ( MSG_EVMORGUI_ERRORCREATINGNEWPROJECTDIRECTORYITMAYALREADYEXISTO ), "examplefilename");  // "Error creating new Project Directory: %s. It may already exist or there may be a file with that name."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 261:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EVMORGUI_ERRORCREATINGNEWDATABASEDIRECTORYITMAYALREADYEXIST ), "examplefilename");  // "Error creating new Database Directory: %s. It may already exist or there may be a file with that name."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 262:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_EVMORGUI_ERRORCREATINGNEWDEFAULTDIRECTORYITMAYALREADYEXISTO ), "examplefilename");  // "Error creating new Default Directory: %s. It may already exist or there may be a file with that name."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 263:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRORSAVINGTHENEWPROJECTFILE ) );  // "Error saving the new Project file."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 264:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRORSAVINGTHECLONEDWAVEFILE ) );  // "Error saving the cloned Wave file."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 265:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_EVMORGUI_ERRSAVECLONEDCLOUDFILE ) );  // "Error saving the cloned Cloud file."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);

        case 266:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            strcpy(str, (char*)GetString( MSG_GLOBAL_OUTOFMEMORY ) );  // "Out of memory."
            User_Message_Def(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),          // "OK"
                    (CONST_STRPTR)"o", 0);


            //./FoliageGUI.c
            // find . -name "FoliageGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 267:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_FOLIGUI_PARAMETERSMODULEFOLIAGE ),         // "Parameters Module: Foliage"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYANTOPENFOLIAGEEDITOR ), // "Out of memory!\nCan't open Foliage Editor."
                    GetString( MSG_GLOBAL_OK ),                              // "OK"
                    (CONST_STRPTR)"o");

        case 268:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_FOLIGUI_PARAMETERSMODULEFOLIAGE ),          // "Parameters Module: Foliage"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYANTOPENFOLIAGEEDITOR ),  // b"Out of memory!\nCan't open Foliage Editor."
                    GetString( MSG_GLOBAL_OK ),                               // "OK"
                    (CONST_STRPTR)"o");

        case 269:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),             // "OK"
                    (CONST_STRPTR)"o");

        case 270:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
                    GetString( MSG_AGUI_KEEPCHANGES ),    // "Keep changes?"
                    GetString( MSG_GLOBAL_YESNO ),          // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 271:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_UNABLETOLOADIMAGEFILEFORVIEWINGPERATIONTERMINATED ),   // "Unable to load image file for viewing!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o", 0);

        case 272:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORLOADECOTYPE ),                   // "Foliage Editor: Load Ecotype"
                    GetString( MSG_FOLIGUI_ERRORLOADINGECOTYPEFILEPERATIONTERMINATED ),  // "Error loading Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o", 0);

        case 273:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                            // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o", 0);

        case 274:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                           // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_ERRORLOADINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error loading Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o", 0);

        case 275:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o", 0);

        case 276:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                                              // "OK"
                    (CONST_STRPTR)"o", 0);

        case 277:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEGROUP ),                         // "Foliage Editor: Save Group"
                    GetString( MSG_FOLIGUI_ERRORSAVINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error saving Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o", 0);

        case 278:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                             // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ) ,  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                // "OK"
                    (CONST_STRPTR)"o", 0);

        case 279:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                    // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_ERRORLOADINGIMAGEFILEPERATIONTERMINATED ),  // "Error loading image file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o", 0);

        case 280:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_THEIMAGELOADEDPROPERLYMAYBESOMEDAYTHEREWILLEVENBEAW ),  // "The image loaded properly. Maybe some day there will even be a way for you to see it!\n"
                    GetString( MSG_FOLIGUI_THATWOULDBENICE ), (CONST_STRPTR)"t", 0);               // "That would be nice"


        case 281:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEECOTYPE ),                  // "Foliage Editor: Save Ecotype"
                    GetString( MSG_FOLIGUI_ERRORSAVINGECOTYPEFILEPERATIONTERMINATED ),  // "Error saving Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o", 0);

            //./GUI.c
            // find . -name "GUI.c" -exec grep -A3 -nHis "User_Message" {} \;

            //.GenericParams.c
            // find . -name "GenericParams.c" -exec grep -A3 -nHis "User_Message" {} \;

        case 282:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARAMS_KEYFRAMEMODULE ),                                      // "Key Frame Module"
                    GetString( MSG_PARAMS_OUTOFMEMORYALLOCATINGNEWKEYFRAMEPERATIONTERMINATED ),  // "Out of memory allocating new key frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

            //.GenericTLGUI.c
            // find . -name "GenericTLGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 283:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                           // "Time Lines"
                    GetString( MSG_GENTLGUI_OKGARYYOUKNOWYOUCANTHAVEMORETHANTENVALUESPERTIMELI ),  // "OK, Gary! You know you can't have more than ten values per Time Line. Maybe now you will concede the value of dynamic allocation."
                    GetString( MSG_GENTLGUI_SUREANYTHINGYOUSAY ),                                  // "Sure, anything you say!"
                    (CONST_STRPTR)"s");

        case 284:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                              // "Time Lines"
                    GetString( MSG_GENTLGUI_YOUVEREACHEDTHELIMITOFOPENTIMELINEWINDOWSPLEASECLO ),  // "You've reached the limit of open Time Line windows. Please close one and try again."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 285:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_GENTLGUI_TIMELINE ),     // "Time Line"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 286:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_GENTLGUI_TIMELINES ),                                              // "Time Lines"
                    GetString( MSG_GENTLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");


            //.GlobeMap.c
            // find . -name "GlobeMap.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 287:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                   // "Render Module"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 288:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                // "Render Module"
                    GetString( MSG_GLMP_ERROROPENINGRENDERWINDOWPERATIONTERMINATED ),  // "Error opening render window!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 289:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                 // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGZBUFFERPERATIONTERMINATED ),  // "Out of memory opening Z buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK",
                    (CONST_STRPTR)"o");

        case 290:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                  // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGBITMAPSPERATIONTERMINATED ),   // "Out of memory opening bitmaps!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                            // "OK"
                    (CONST_STRPTR)"o");

        case 291:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory opening anti-alias buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   //"OK"
                    (CONST_STRPTR)"o");

        case 292:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGREFLECTIONBUFFERCONTINUEWITHOUTRE ),  // "Out of memory allocating Reflection buffer!\n\Continue without Reflections?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
                    (CONST_STRPTR)"oc");

        case 293:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module
                    GetString( MSG_GLMP_DIAGNOSTICBUFFERSCANTBEGENERATEDFORMULTIPLESEGMENTORMU ),  // "Diagnostic buffers can't be generated for multiple segment or multiple frame renderings! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
                    (CONST_STRPTR)"oc", 1);

        case 294:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGDIAGNOSTICBUFFERSPROCEEDRENDERINGWIT ),  // "Out of memory opening Diagnostic buffers! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
                    (CONST_STRPTR)"oc", 1);

        case 295:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ) ,                                       // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),   // "Out of memory opening key frame table!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 296:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                          // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGWAVEFILECONSTSTRPTRCONTINUEWITHOUTWAVES ),   // "Error loading Wave File!\n\Continue without Waves?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                        // "Continue|Cancel"
                    (CONST_STRPTR)"oc");

        case 297:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                  // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORLOADINGCLOUDMAPFILEONTINUEWITHOUTCLOUDSHADOWS ),  // "Error loading Cloud Map file!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 298:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                      // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGCLOUDMAPONTINUEWITHOUTCLOUDSHADOWS ),   // "Out of memory creating Cloud Map!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 299:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGMASTERCOLORMAPSEESTATUSLOGFORMOREINFORMATI ),  // "Error loading Master Color Map! See Status Log for more information.\n\Continue rendering without Color Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 300:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGSTRATADEFORMATIONMAPCONTINUERENDERINGWITHO ),  // "Error loading Strata Deformation Map!\n\Continue rendering without Deformation Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 301:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGNOISEMAPCONTINUERENDERINGWITHOUTTEX ),  // "Out of memory creating Noise Map!\n\Continue rendering without Texture Noise?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                          // "Continue|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 302:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 303:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 304:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)linefile,
                    GetString( MSG_GLMP_CANTOPENVECTORFILEFOROUTPUTONTINUERENDERINGWITHOUTVECT ),  // "Can't open vector file for output!\nContinue rendering without vectors?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                                // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 305:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                               // "Render Module"
                    GetString( MSG_GLMP_ERRORINTERLACINGFIELDSPERATIONTERMINATED ) ,  // "Error interlacing fields!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 306:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULESAVE ),                           // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGBITMAPPEDIMAGETRYANOTHERDEVICE ),  // "Error saving bitmapped image! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                   // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 307:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYSAVINGZBUFFER ),  // "Out of memory saving Z Buffer!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),               // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 308:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULESAVE ),                    // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGZBUFFERTRYANOTHERDEVICE ),  // "Error saving Z Buffer! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                            // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 309:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_GLMP_OUTOFMEMORYREADINGMAP ), "DBase[OBN].Name");  // "Out of memory reading map %s!"
            User_Message_Def(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                 // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ) ,                     // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 310:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                               // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 311:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGFRACTALMAPARRAYCONTINUEWITHOUTFRA ),  // "Out of memory allocating Fractal Map array!\n\Continue without Fractal Maps or retry?"
                    GetString( MSG_GLMP_CONTINUERETRYCANCEL ),                                     // "Continue|Retry|Cancel"
                    (CONST_STRPTR)"orc");

        case 312:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory allocating antialias buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 313:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 314:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                       // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGPOLYGONEDGEBUFFERS ),  // "Out of memory allocating polygon edge buffers!",
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                              // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 315:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                  // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 316:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                  // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 317:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                                          // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 318:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                     // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGCLOUDKEYFRAMESPERATIONTERMINATED ),  // "Out of memory allocating Cloud Key Frames!\nOperation terminated"
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o", 0);

        case 319:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),  // "Render Module"
                    GetString( MSG_GLMP_ERRORREADINGPAGEDOUTFILECANTRESTOREREFLECTIONBUFFERSOP),  // original in GlobeMap.c:1974 (CONST_STRPTR)ErrStr,
                    GetString( MSG_GLOBAL_OK ),            // "OK"
                    (CONST_STRPTR)"o");

        case 320:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                            // "Render Module"
                    GetString( MSG_GLMPSPRT_ERRORLOADINGSUNIMAGEPERATIONTERMINATED ),  // "Error loading Sun Image!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 321:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                             // "Render Module"
                    GetString( MSG_GLMPSPRT_ERRORLOADINGMOONIMAGEPERATIONTERMINATED ),  // "Error loading Moon Image!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");


            //.HelpGUI.c
            // find . -name "HelpGUI.c" -exec grep -A3 -nHis "User_Message" {} \;

            //.InteractiveDraw.c
            //find . -name "InteractiveDraw.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 322:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTDRW_INTERACTIVEMOTIONMODULE ),                  // "Interactive Motion Module"
                    GetString( MSG_INTDRW_OUTOFMEMORYIDDENLINEREMOVALNOTAVAILABLE ),  // "Out of memory!\nHidden line removal not available."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 323:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTDRW_PARAMETERSMODULEPATH ),                               // "Parameters Module: Path"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening key frame table!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                 // "OK"
                    (CONST_STRPTR)"o");

            //.InteractiveView.c
            // find . -name "InteractiveView.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 324:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),              // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_YOUMUSTFIRSTLOADACOMPLETEPARAMETERFILE ),  // "You must first load a complete Parameter file!"
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 325:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),                         // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_THEREARENOOBJECTSINTHISDATABASEPERATIONTERMINATED ),  // "There are no objects in this Database!\nOperation terminated"
                    GetString( MSG_GLOBAL_OK ),                                                 // "OK"
                    (CONST_STRPTR)"o");

        case 326:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTVIEW_EDITINGMODULEINTERACTIVE ),                  // "Editing Module: Interactive"
                    GetString( MSG_INTVIEW_CAMERAVIEWFAILEDTOOPENPERATIONTERMINATED ),  // "Camera View failed to open!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 327:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),                      // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYOPENINGCAMERAVIEWPERATIONTERMINATED ),  // "Out of memory opening Camera View!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 328:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),             // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYLOADINGDEMSNCREASEGRIDSIZE ),  // "Out of memory loading DEMs!\nIncrease grid size?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                               // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 329:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),            // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_NODEMOBJECTSACTIVEPERATIONTERMINATED ),  // "No DEM objects active!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                    // "OK"
                    (CONST_STRPTR)"o");

        case 330:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                   // "Camera View"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGZBUFFERPERATIONTERMINATED ),  // "Out of memory opening Z buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 331:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYOPENINGANTIALIASBUFFERPERATIONTERMINATED ),  // "Out of memory opening Antialias buffer!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 332:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_EDMOGUI_CAMERAVIEW ),                     // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYALLOCATINGDEMARRAY ),  // "Out of memory allocating DEM array!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                    // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 333:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI ),  // "Out of memory allocating Polygon Smoothing array!\nContinue without Polygon Smoothing?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                             // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 334:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 335:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                // "Camera View"
                    GetString( MSG_INTVIEW_GRIDMUSTBEPRESENTPLEASEREDRAWANDTRYAGAIN ),  // "Grid must be present, please redraw and try again."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 336:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                          // "Camera View"
                    GetString( MSG_INTVIEW_ERROROPENINGSMALLRENDERINGWINDOWPERATIONTERMINATED ),  // "Error opening Small Rendering Window!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 337:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, "%s.elev", "DBase[MapOBN].Name");
            User_Message((CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_ERROROPENINGDEMFILEFORINPUTPERATIONTERMINATED ),  // "Error opening DEM file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 338:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, "%s.elev", "DBase[MapOBN].Name");
            User_Message((CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_OUTOFMEMORYTRYASMALLERPREVIEWSIZEPERATIONTERMINATED ),  // "Out of memory! Try a smaller preview size.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 339:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_EDMOGUI_CAMERAVIEW ),                                           // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMALLOCPOLYSMOOTHARRAYCONTINUEWI ),  // "Out of memory allocating Polygon Smoothing array!\nContinue without Polygon Smoothing?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                             // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 340:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, "%s.relel", "DBase[MapOBN].Name");
            User_Message((CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 341:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                         // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMALLOCANTIALIASEDGEBUFFERSPERATIONTE ),  // "Out of memory allocating antialias and edge buffers!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 342:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_INTVIEW_PARAMETERSMODULEPREVIEW ),                      // "Parameters Module: Preview"
                    GetString( MSG_INTVIEW_RESTORETHEPARAMETERSUSEDTOCREATETHISPREVIEW ),  // "Restore the Parameters used to create this preview?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                     // GetString( MSG_INTVIEW_OKCANCEL )"OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

            //.LWSupport.c
            //find . -name "LWSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 343:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                  // "LightWave Motion: Export"
                    GetString( MSG_LWSPRT_NOKEYFRAMESTOEXPORTPERATIONTERMINATED ),  // "No Key Frames to export!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 344:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),          // "LightWave Motion: Export"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 345:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                        // "LightWave Motion: Export"
                    GetString( MSG_LWSPRT_ERROROPENINGFILEFOROUTPUTPERATIONTERMINATED ),  // "Error opening file for output!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 346:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LWSPRT_LIGHTWAVEMOTIONEXPORT ),                            // "LightWave Motion: Export"
                    GetString( MSG_LWSPRT_ERRORWRITINGTOFILEPERATIONTERMINATEDPREMATURELY ),  // "Error writing to file!\nOperation terminated prematurely."
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o");

        case 347:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"Example DEMName",
                    GetString( MSG_LWSPRT_ERRORLOADINGDEMOBJECTPERATIONTERMINATED ),  // "Error loading DEM Object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o", 0);

        case 348:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"Example DEMName",
                    GetString( MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED ),  // "Error loading DEM Object!\nObject not saved."
                    GetString( MSG_GLOBAL_OK ),                                  // "OK"
                    (CONST_STRPTR)"o", 0);

        case 349:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_LWSPRT_LWOBJECTEXPORT ),                 // "LW Object Export"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o", 0);

        case 350:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_APROBLEMOCCURREDSAVINGTHELWSCENEFAFILEWASCREATEDITWILLNOTBE ),  // "A problem occurred saving the LW scene.\nIf a file was created it will not be complete and may not load properly into LightWave."
                    GetString( MSG_GLOBAL_OK ),                                                           // "OK"
                    (CONST_STRPTR)"o", 0);

        case 351:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_THEOUTPUTIMAGESIZEISNOTASTANDARDLIGHTWAVEIMAGESIZETHEZOOMFA ),  // "The output image size is not a standard LightWave image size. The zoom factor and image dimensions may not be portrayed correctly in the scene file just created."
                    GetString( MSG_GLOBAL_OK ),                                                           // "OK"
                    (CONST_STRPTR)"o", 0);

            //.LineSupport.c
            // find . -name "LineSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 352:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),  // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ),                                     // "Active|New|Cancel"
                    (CONST_STRPTR)"anc", 1);

        case 353:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_DIAGNOSTICDIGITIZE ),                                  // "Diagnostic: Digitize"
                    GetString( MSG_MAP_ACTIVEOBJECTISADEMANDMAYNOTBEDIGITIZEDPERATIO ),  // "Active object is a DEM and may not be digitized!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 354:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_INTERACTIVEMODULEADDPOINTS ),    // "Interactive Module: Add Points"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ), // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ) ,                           // "OK"
                    (CONST_STRPTR)"o");

        case 355:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_LINESPRT_SAVEOBJECTPOINTS ),  // "Save object points?"
                    GetString( MSG_GLOBAL_OKCANCEL ),          // "OK|CANCEL"
                    (CONST_STRPTR)"oc", 1);

        case 356:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                     // "Mapping Module: Path"
                    GetString( MSG_MAP_ERRORLOADINGVECTOROBJECTPERATIONTERMINATED),  // "Error loading vector object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 357:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_CAMERAKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVAL ),  // "Camera Key Frames exist. Proceeding will delete current values!"
                    GetString( MSG_MOREGUI_PROCEEDCANCEL ),                                       // "Proceed|Cancel"
                    (CONST_STRPTR)"pc");

        case 358:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_FOCUSKEYFRAMESEXISTPROCEEDINGWILLDELETECURRENTVALU ),  // "Focus Key Frames exist. Proceeding will delete current values!"
                    GetString( MSG_MOREGUI_PROCEEDCANCEL ) ,                                      // "Proceed|Cancel"
                    (CONST_STRPTR)"pc");

        case 359:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, "%s.elev", "DBase[OBN].Name");
            User_Message_Def((CONST_STRPTR)str, GetString( MSG_LINESPRT_USEELEVATIONDATA ),  // "Use elevation data?"
                    GetString( MSG_GLOBAL_YESNO ),                                // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 360:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current flattening, datum and vertical exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 361:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                               // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_OUTOFMEMORYCREATINGKEYFRAMESPERATIONTERMINATED ),  // "Out of memory creating Key Frames!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 362:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_USEALLSPLINEDPOINTSORONLYKEYFRAMES ),  // "Use all splined points or only Key Frames?"
                    GetString( MSG_LINESPRT_ALLSPLINEDKEYFRAMES ),                 // "All Splined|Key Frames"
                    (CONST_STRPTR)"ak");

        case 363:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current Flattening, Datum and Vertical Exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ),                                               // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 364:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                  // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening Key Frame table!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),  // "OK"
                    (CONST_STRPTR)"o");

        case 365:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_THEREAREMOREFRAMESTHANALLOWABLEVECTORPOINTSPATHWIL ),  // "There are more frames than allowable vector points! Path will be truncated."
                    GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 366:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module"
                    GetString( MSG_LINESPRT_VECTORNAMEALREADYPRESENTINDATABASEVERWRITEITORTRYA ),  // "Vector name already present in Database!\nOverwrite it or try a new name?"
                    GetString( MSG_LINESPRT_OVERWRITENEWCANCEL ),                                  // "Overwrite|New|Cancel"
                    (CONST_STRPTR)"onc", 2);

        case 367:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_DATABASEMODULE ),                                  // "Database Module"
                    GetString( MSG_LINESPRT_OUTOFMEMORYEXPANDINGDATABASEPERATIONTERMINATED ),  // "Out of memory expanding Database!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 368:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_OUTOFMEMCREATNEWVECTOROBJECTPERATIONTERMINAT ),  // "Out of memory creating new vector object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

            //.Map.c
            // find . -name "Map.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 369:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
                    GetString( MSG_MAP_FIRSTSETOFALIGNMENTLATLONCOORDINATESMUSTBELAR ),  // "First set of alignment lat/lon coordinates must be larger than second and map scale must be greater than zero!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 370:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
                    GetString( MSG_MAP_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSETO ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 371:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPVIEWECOSYSTEMS ),                              // "Map View: Ecosystems"
                    GetString( MSG_MAP_THEREARENOPARAMETERSLOADEDECOSYSTEMMAPPINGISN ),  // "There are no Parameters loaded! Ecosystem mapping is not available until you load a Parameter file or create Default Parameters."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 372:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAP_MAPVIEWTOPODRAW ),                                // "Map View: Topo Draw"
                    GetString( MSG_MAP_MEMORYALLOCATIONFAILURECANNOTDRAWTOPOCONTINUE ),  // "Memory allocation failure, cannot draw topo. Continue?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                       // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 373:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPVIEWECOSYSTEMS ),                              // "Map View: Ecosystems"
                    GetString( MSG_MAP_OUTOFMEMORYLOADINGRELATIVEELEVATIONFILEECOSYS ),  // "Out of memory loading Relative Elevation file. Ecosystem mapping not available?"
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 374:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),  // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ),                   // "YES|NO"
                    (CONST_STRPTR)"yn", 1);

        case 375:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[i].Name", GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),   // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ),                                                 // "YES|NO"
                    (CONST_STRPTR)"yn", 1);

        case 376:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),   // "Mapping Module"
                    GetString( MSG_MAP_OBJECTNOTFOUND ),  // "Object not found!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");

        case 377:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAP_MAPVIEWMULTISELECT ),     // "Map View: Multi-Select"
                    GetString( MSG_MAP_SELECTORDESELECTITEMS ),  // "Select or de-select items?"
                    GetString( MSG_MAP_SELECTDESELECTCANCEL ),   // "Select|De-select|Cancel"
                    (CONST_STRPTR)"sdc", 1);

        case 378:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),   // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ), (CONST_STRPTR)"anc", 1);     // "Active|New|Cancel"

        case 379:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPVIEWDIGITIZE ),                                // "Map View: Digitize"
                    GetString( MSG_MAP_ACTIVEOBJECTISADEMANDMAYNOTBEDIGITIZEDPERATIO ),  // "Active object is a DEM and may not be digitized!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 380:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),   // "Mapping Module: Digitize"
                    GetString( MSG_MAP_ACCEPTNEWPOINTS ),         // "Accept new points?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 381:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                           // "Mapping Module: Digitize"
                    GetString( MSG_MAP_OUTOFMEMORYALLOCATINGNEWVECTORARRAYPERATIONTE ),   // "Out of memory allocating new vector array!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                              // "OK"
                    (CONST_STRPTR)"o");

        case 382:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 383:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEINSERTPOINTS ),   // "Mapping Module: Insert Points"
                    GetString( MSG_MAP_OUTOFMEMORYOPERATIONFAILED ),  // "Out of memory! Operation failed."
                    GetString( MSG_GLOBAL_OK ),                          // "OK"
                    (CONST_STRPTR)"o");

        case 384:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 385:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_CREATEVISUALSENSITIVITYMAPFORTHISOBJECT ),  // "Create Visual Sensitivity map for this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                 // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 386:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_DBASEOBNNAME ),                                // "DBase[OBN].Name"
                    GetString( MSG_MAP_ERRORLOADINGVECTOROBJECTPERATIONTERMINATED ),  // "Error loading vector object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                       // "OK"
                    (CONST_STRPTR)"o");

        case 387:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                 // "Mapping Module"
                    GetString( MSG_MAP_ERROROPENINGVIEWSHEDWINDOWXECUTIONTERMINATED ),  // "Error opening viewshed window!\nExecution terminated."
                    GetString( MSG_GLOBAL_OK ),                                            // "OK"
                    (CONST_STRPTR)"o");

        case 388:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),          // "Mapping Module: Viewshed"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 389:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),          // "Mapping Module: Viewshed"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 390:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),                   // "Mapping Module: Viewshed"
                    GetString( MSG_MAP_ERRORREADINGTOPOMAPSPERATIONTERMINATED ),  // "Error reading topo maps!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 391:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),                // "Mapping Module: Viewshed"
                    GetString( MSG_MAP_SMOOTHTHEMAPBEFORECOMPUTINGVIEWSHED ),  // "Smooth the map before computing viewshed?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                             // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 392:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAP_MAPPINGMODULEVIEWSHED ),           // "Mapping Module: Viewshed"
                    GetString( MSG_MAP_DRAWVECTORSONVIEWSHEDRENDERING ),  // "Draw vectors on viewshed rendering?"
                    GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                    (CONST_STRPTR)"yn");

        case 393:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CANTOPENSERIALDEVICEPERATIONTERMINATED ),  // "Can't open serial device!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                      // "OK"
                    (CONST_STRPTR)"o");

        case 394:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
                    GetString( MSG_MAP_DIGITIZENEWREGISTRATIONPOINTS ),  // "Digitize new registration points?"
                    GetString( MSG_GLOBAL_YESNO ),                          // "YES|NO"
                    (CONST_STRPTR)"yn");

        case 395:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                          // "Mapping Module: Digitize"
                    GetString( MSG_MAP_ILLEGALVALUEWOREGISTRATIONPOINTSMAYNOTBECOINC ),  // "Illegal value!\nTwo registration points may not be coincident.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK
                    (CONST_STRPTR)"o");

        case 396:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 397:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),          // "Mapping Module: Digitize"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

            //.MapExtra.c
            //find . -name "MapExtra.c" -exec grep -A3 -nHis "User_Message" {} \;

        case 398:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_OBJECTISNOTCLOSEDHEORIGINCANNOTBEMOVEDETLASTVERTEX ),  // "Object is not closed!\nThe origin cannot be moved.\nSet last vertex equal to first now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 399:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 400:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_OBJECTRESULTINGFROMTHISMATCHWOULDBELARGERTHANTHEMA ),  // "Object resulting from this match would be larger than the maximum of MAXOBJPTS !\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 401:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),                             // "Mapping Module: Point Match"
                    GetString( MSG_MAPEXTRA_ILLEGALNUMBEROFPOINTSFFIRSTANDLASTDESTINATIONPOINT ),  // "Illegal number of points!\nIf first and last destination points are the same, source points must be larger than zero.\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 402:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),  // "Mapping Module: Point Match"
                    GetString( MSG_MAPEXTRA_PROCEEDWITHRELOCATION ),    // "Proceed with relocation?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                 // "OK|CANCEL"
                    (CONST_STRPTR)"oc", 1);

        case 403:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),                        // "Mapping Module: Point Match"
                    GetString( MSG_MAPEXTRA_OUTOFMEMORYOTENOUGHFORNEWPOINTSPERATIONFAILED ),  // "Out of memory!\nNot enough for new points.\nOperation failed."
                    GetString( MSG_GLOBAL_OK ),                                             // "OK"
                    (CONST_STRPTR)"o");

        case 404:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ) ,                               // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 405:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 406:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_DUPLICATETHISOBJECT ),  // "Duplicate this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ),             // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 407:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                            // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_OUTOFMEMNOTENOUGHFORTEMPTOPOARRAYPERATIONFAIL ), // "Out of memory!\nNot enough for temporary topo array.\nOperation failed."
                    GetString( MSG_GLOBAL_OK ),                                                   // "OK"
                    (CONST_STRPTR)"o");

        case 408:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                   // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_POINTMAXIMUMHASBEENREACHEDAPPINGTERMINATED ),  // "Point maximum has been reached!\nMapping terminated"
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

        case 409:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str,
                    (char*)GetString( MSG_MAPEXTRA_REACHEDEDGEOFCURRENTMAPOINTSONTINUETONEXTMAP ), pts);                  // "Reached edge of current map!\nPoints = %d\nContinue to next map?"
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ), (CONST_STRPTR)str,  // "Mapping Module: Follow Stream"
                    GetString( MSG_GLOBAL_OKCANCEL ),  // "OK|CANCEL"
                    (CONST_STRPTR)"oc");

        case 410:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                           // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_INITIALPOINTNOTWITHINCURRENTLYLOADEDTOPOBOUNDARIES ),  // "Initial point not within currently loaded topo boundaries!\nObject points reduced to 1."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 411:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),                           // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_INITIALPOINTNOTWITHINCURRENTLYLOADEDTOPOBOUNDARIES ),  // "Initial point not within currently loaded topo boundaries!\nObject points reduced to 1."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 412:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),  // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_SAVEVECTOROBJECTNOW ),        // "Save vector object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                   // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 413:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_MAPEXTRA_SPLINELENGTHFKILOMETERSNTERVALFKMSEGMENT ), 20, 5);  // "Spline length = %f kilometers\nInterval = %f km/segment"
            User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULESPLINE ),                       // "Mapping Module: Spline"
                    (CONST_STRPTR)str,
                    GetString( MSG_MAPEXTRA_OKRESETCANCEL ),                             // "OK|Reset|Cancel"
                    (CONST_STRPTR)"orc", 1);

        case 414:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize",
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 415:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPVIEWMODULEINTERPOLATE ),                            // "Map View Module: Interpolate"
                    GetString( MSG_MAPEXTRA_OUTOFMEMORYCANTALLOCATENEWVECTORPERATIONTERMINATED ),  // "Out of memory! Can't allocate new vector.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 416:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),  // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_PROCEEDORRESETPOINTS ),   // "Proceed or reset points?"
                    GetString( MSG_MAPEXTRA_PROCEEDRESETCANCEL ),     // "Proceed|Reset|Cancel"
                    (CONST_STRPTR)"prc", 1);

        case 417:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),           // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_KEEPORSAVEDEMORRESETPARAMETERS ),  // "Keep or save DEM or reset parameters?"
                    GetString( MSG_MAPEXTRA_KEEPSAVERESETCANCEL ),             // "Keep|Save|Reset|Cancel"
                    (CONST_STRPTR)"ksrc", 1);

        case 418:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),          // "Mapping Module: Fix Flats"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 419:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_ALLCORNERPOINTSMUSTBEWITHINTOPOMAPBOUNDARIESPERATI ),  // "All corner points must be within topo map boundaries!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 420:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                          // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_ILLEGALDIMENSIONSTRYMAKINGTHERECTANGLELARGERPERATI ),  // "Illegal dimensions! Try making the rectangle larger.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 421:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message( GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),                               // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_ALLCORNERPNTSMUSTBEWITHINSAMEDEMPERATIONTERMINAT ),  // "All corner points must be within same DEM!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 422:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[TopoOBN[i]].Name",
                    GetString( MSG_MAPEXTRA_ERROROPENINGOUTPUTFILEPERATIONTERMINATED ),  // "Error opening output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                        // "OK"
                    (CONST_STRPTR)"o");

        case 423:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[TopoOBN[i]].Name",
                    GetString( MSG_MAPEXTRA_ERRORWRITINGTOOUTPUTFILEPERATIONTERMINATED ),  // "Error writing to output file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                          // "OK"
                    (CONST_STRPTR)"o");

            //.MapGUI.c
            // find . -name "MapGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 424:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                           // "Mapping Module"
                    GetString( MSG_MAPGUI_OUTOFMEMORYANTINITIALIZEMAPWINDOWPERATIONTERMINATED ),  // "Out of memory!\nCan't initialize map window!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");                               // "OK"

        case 425:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),  // "Mapping Module: Align"
                    GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal."
                    GetString( MSG_GLOBAL_OK ), (CONST_STRPTR)"o");  // "OK"

        case 426:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),     // "Mapping Module: Digitize"
                    GetString( MSG_MAPGUI_SETDIGITIZINGINPUTSOURCE ),  // "Set digitizing input source."
                    GetString( MSG_MAPGUI_BITPADSUMMAGRIDMOUSE ),      // "Bitpad|Summagrid|Mouse"
                    (CONST_STRPTR)"bsm");

        case 427:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                                      // "Map View: Build DEM"
                    (CONST_STRPTR) GetString( MSG_MAPGUI_ATLEASTONEENDCONTROLPOINTFORTHELINESEGMENTJUSTDRAWNC ),  // "At least one end control point for the line segment just drawn could not be found!\nDo you wish to use the current and minimum slider elevations for this segment or abort the operation?",
                    GetString( MSG_MAPGUI_SLIDERABORT ),                                                          // "Slider|Abort"
                    (CONST_STRPTR)"sa");

        case 428:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                                    // "Mapping Module: Align"
                    GetString( MSG_MAPGUI_ILLEGALREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUA ),  // "Illegal registration values! High and low X or Y values are equal.
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 429:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_DATABASESAVE ),                                          // "Database: Save"
                    GetString( MSG_MAPGUI_THEDATABASEHASBEENMODIFIEDSINCEITWASLOADEDOYOUWISHTO ),  // "The Database has been modified since it was loaded.\nDo you wish to save it or a Master Object file now?",
                    GetString( MSG_MAPGUI_DBASEOBJECTBOTHNEITHER ),                                // "D'base|Object|Both|Neither"
                    (CONST_STRPTR)"dmbn");

        case 430:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
                    GetString( MSG_MAPGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREOPENINGT ),  // "You must first load or create a parameter file before opening the Legend."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 431:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWECOSYSTEMLEGEND ),              // "Map View: Ecosystem Legend"
                    GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "Out of memory!\nCan't open Ecosystem Legend."
                    GetString( MSG_MAPGUI_OUTOFMEMORYANTOPENECOSYSTEMLEGEND ),  // "OK"
                    (CONST_STRPTR)"o");

        case 432:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_ECOSYSTEMLEGEND ),  // "Ecosystem Legend"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),      // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");


            //.MapLineObject.c
            // find . -name "MapLineObject.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 433:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                               // "Render Module"
                    GetString( MSG_MAPLINO_ERRORSAVINGLINEVERTICESTOFILEELECTNEWPATH ),  // "Error saving line vertices to file!\nSelect new path."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 434:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                          // "Render Module"
                    GetString( MSG_MAPLINO_ERROROPENINGLINESAVEFILEELECTNEWPATH ),  // "Error opening line save file!\nSelect new path?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                              // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

            //.MapSupport.c
            // find . -name "MapSupport.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 435:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[OBN].Name", GetString( MSG_MAPSUPRT_CANTOPENOBJECTFILEBJECTNOTSAVED ),  // "Can't open object file!\nObject not saved."
                    GetString( MSG_GLOBAL_OK ),                                                              // "OK"
                    (CONST_STRPTR)"o");

        case 436:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"DBase[OBN].Name", GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),  // "Error saving object file!\nObject not saved."
                    GetString( MSG_MAPSUPRT_ERRORSAVINGOBJECTFILEBJECTNOTSAVED ),                                 // "OK"
                    (CONST_STRPTR)"o");

        case 437:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPSUPRT_MAPPINGMODULETOPOMAPPING ),                            // "Mapping Module: Topo Mapping"
                    GetString( MSG_MAPSUPRT_NOTOPOMAPSFOUNDHECKOBJECTENABLEDSTATUSANDCLASSINDA ),  // "No topo maps found!\nCheck object Enabled Status and Class in database."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 438:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),               // "Map View: Load Topos"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 439:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, "%s.elev", "DBase[i].Name");
            User_Message((CONST_STRPTR)str, GetString( MSG_MAPSUPRT_ERRORLOADINGTOPOMAPCHECKSTATUSLOGTOSEEIFOUTOFMEMOR ),  // "Error loading topo map! Check Status Log to see if out of memory.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 440:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPSUPRT_MAPVIEWLOADTOPOS ),            // "Map View: Load Topos"
                    GetString( MSG_MAPSUPRT_ERRORLOADINGDEMSNONELOADED ),  // "Error loading DEMs! None loaded."
                    GetString( MSG_GLOBAL_OK ),                          // "OK"
                    (CONST_STRPTR)"o");

        case 441:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULE ),                                       // "Mapping Module"
                    GetString( MSG_MAP_OUTOFMEMORYALLOCATINGNEWVECTORARRAYPERATIONTE ),  // "Out of memory allocating new vector array!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 442:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAPSUPRT_VECTOROBJECTHASBEENMODIFIEDAVEITBEFORECLOSING ),  // "Vector object has been modified!\nSave it before closing?"
                    GetString( MSG_MAPSUPRT_SAVECANCEL ),                                     // "SAVE|CANCEL"
                    (CONST_STRPTR)"sc", 1);

        case 443:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_SELECTEDOBJECTMUSTBEATOPODEMEECLASSFIELDINDATABASE ),  // "Selected object must be a Topo DEM!\nSee Class field in Database Editor.\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 444:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_SELECTEDMAPISNOTCURRENTLYLOADEDOYOUWISHTOLOADTOPOM ),  // "Selected map is not currently loaded!\nDo you wish to load topo maps?",
                    GetString( MSG_GLOBAL_OKCANCEL ),                                            // "OK|CANCEL"
                    (CONST_STRPTR)"oc", 1);

        case 445:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                               // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_OUTOFMEMORYCREATINGBITMAPSPERATIONTERMINATED ),  // "Out of memory creating bitmaps!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                            // "OK"
                    (CONST_STRPTR)"o");

        case 446:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                    // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_INCLUDEDEMELEVATIONDATAINCOLORMAP ),  // "Include DEM elevation data in Color Map?"
                    GetString( MSG_GLOBAL_YESNO ),                              // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

            //.MapTopoObject.c
            // find . -name "MapTopoObject.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 447:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                               // "Retry|Cancel"
                    (CONST_STRPTR)"rc", 1);

        case 448:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                       // "Render Module"
                    GetString( MSG_MAPTOPOOB_ERRORALLOCATINGORREADINGFRACTALINDEXARRAYSCONTINU ),  // "Error allocating or reading Fractal Index arrays!\nContinue without Fractal Displacement Mapping?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                           // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 449:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                     // "Render Module"
                    GetString( MSG_MAPTOPOOB_ERRORSAVINGVECTORVERTICESTOFILE ),  // "Error saving vector vertices to file!"
                    GetString( MSG_GLOBAL_OK ),                               // "OK"
                    (CONST_STRPTR)"o");

        case 450:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                 // "Render Module"
                    GetString( MSG_MAPTOPOOB_CANTOPENVECTORFILEFOROUTPUT ),  // "Can't open vector file for output!"
                    GetString( MSG_GLOBAL_OK ),                           // "OK"
                    (CONST_STRPTR)"o");

        case 451:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str, (char*)GetString( MSG_GLMP_OUTOFMEMORYREADINGMAP ), "DBase[OBN].Name");  // "Out of memory reading map %s!"
            User_Message_Def(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                        // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ),                             //"Retry|Cancel",
                    (CONST_STRPTR)"rc", 1);

        case 452:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_AGUI_RENDERMODULE ),                                       // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGKEYFRAMETABLEPERATIONTERMINATED ),  // "Out of memory opening key frame table!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                 // "OK"
                    (CONST_STRPTR)"o");

            //.Memory.c
            // find . -name "Memory.c" -exec grep -A3 -nHis "User_Message" {} \;

            //.MoreGUI.c
            // find . -name "MoreGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 453:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_DEMEXTRACT ),   // "DEM Extract"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 454:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_DATAOPSMODULEDEMEXTRACT ),                       // "Data Ops Module: DEM Extract"
                    GetString( MSG_MOREGUI_PLEASEENTERTHELATITUDEANDLONGITUDEVALUESFORTHESOUTH ),  // "Please enter the latitude and longitude values for the southeast corner of the current DEM in the string gadgets near the top of the DEM Extract Window."
                    GetString( MSG_MOREGUI_PROCEEDCANCEL ), (CONST_STRPTR)"pc");                   // "Proceed|Cancel"


        case 455:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_PROJECTNEWEDIT ),  // "Project: New/Edit"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              //"OK"
                    (CONST_STRPTR)"o");

        case 456:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                    // "OK"
                    (CONST_STRPTR)"o");

        case 457:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MOREGUI_PARAMETERSIMAGESCALE ),  // "Parameters: Image Scale"
                    GetString( MSG_MOREGUI_APPLYCHANGES ),          // "Apply changes?"
                    GetString( MSG_GLOBAL_OKCANCEL ),              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 458:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MENU_PREFS ),  // "Preferences"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");


            //.Params.c
            // find . -name "Params.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 459:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            sprintf(str,
                    (char*)GetString( MSG_PARAMS_OUTOFMEMORYRESTORINGOLDKEYFRAMESOMEKEYSMAYBELOST ),  // "Out of memory restoring old key frames!\nSome %s keys may be lost."
                    "groupname");  // "groupname" should be replaced with the actual group name variable
            User_Message(GetString( MSG_PARAMS_KEYFRAMECANCEL ),  // "Key Frame: Cancel"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ),               // "OK"
                    (CONST_STRPTR)"o");

        case 460:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARAMS_KEYFRAMEMODULE ) ,                                     // "Key Frame Module"
                    GetString( MSG_PARAMS_OUTOFMEMORYALLOCATINGNEWKEYFRAMEPERATIONTERMINATED ),  // "Out of memory allocating new key frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 461:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARAMS_PARAMETERSMODULEVELOCITYDISTRIBUTION ),                 // "Parameters Module: Velocity Distribution"
                    GetString( MSG_PARAMS_EASEINPLUSEASEOUTFRAMEVALUESEXCEEDTOTALNUMBEROFANIMA ),  // "\"Ease In\" plus \"Ease Out\" frame values exceed total number of animated frames.\nThis is illegal! Do you wish to continue without Velocity Distribution?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

            //.ParamsGUI.c
            // find . -name "ParamsGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 462:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
                    GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 463:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
                    GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 464:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),  // "Parameters Module: Scale"
                    GetString( MSG_PARGUI_NOKEYFRAMESTOSCALE ),     // "No key frames to scale!"
                    GetString( MSG_GLOBAL_OK ),                     // "OK"
                    (CONST_STRPTR)"o");

        case 465:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULESCALE ),          // "Parameters Module: Scale"
                    GetString( MSG_PARGUI_OUTOFMEMORYANTOPENSCALEWINDOW ),  // "Out of memory!\nCan't open Scale window."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");

        case 466:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_SCALEKEYS ),    // "Scale Keys"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 467:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_LIGHTWAVEMOTIONIO ),                                     // "LightWave Motion I/O"
                    GetString( MSG_PARGUI_YOUMUSTFIRSTLOADORCREATEAPARAMETERFILEBEFOREUSINGTHI ),  // "You must first load or create a parameter file before using this feature."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 468:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_LIGHTWAVEMOTIONIO ),                                // "LightWave Motion I/O"
                    GetString( MSG_PARGUI_ERRORBUILDINGMOTIONVALUETABLEPERATIONTERMINATED ),  // "Error building motion value table\nOperation terminated",
                    GetString( MSG_GLOBAL_OK ),                                               // "OK"
                    (CONST_STRPTR)"o");

        case 469:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_LIGHTWAVEIO ),  // "LightWave I/O"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),  // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),           // "OK"
                    (CONST_STRPTR)"o");

        case 470:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                // "Parameters Module: Model"
                    GetString( MSG_PARGUI_OUTOFMEMORYANTOPENMODELDESIGNWINDOW ),  // "Out of memory!\nCan't open model design window."
                    GetString( MSG_GLOBAL_OK ),                                   // "OK"
                    (CONST_STRPTR)"o");

        case 471:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),  // "Parameters Module: Model"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),            // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                     // "OK"
                    (CONST_STRPTR)"o");

        case 472:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_THECURRENTECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_1 ),  // "The current Ecosystem Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_PARGUI_YESNOCANCEL ),                                           // "Yes|No|Cancel"
                    (CONST_STRPTR)"ync", 1);

        case 473:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2 ),  //" The current Ecosystem Model has been modified. Do you wish to save it before proceeding?"
                    GetString( MSG_PARGUI_YESNOCANCEL ),                                           // "Yes|No|Cancel"
                    (CONST_STRPTR)"ync", 1);

        case 474:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_ERROROPENINGECOSYSTEMMODELFILEFORINPUTPERATIONTERMI ),   // "Error opening Ecosystem Model file for input!\nOperation terminated." // AF: fixed, was "output"
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 475:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
                    GetString( MSG_PARGUI_ERRORREADINGFROMECOSYSTEMMODELFILEPERATIONTERMINATEDPR ),  // "Error reading from Ecosystem Model file!\nOperation terminated prematurely.", // AF: fixed, was "writing to"
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 476:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
                    GetString( MSG_PARGUI_NOTAWCSECOSYSTEMMODELFILEPERATIONTERMINATED ),  // "Not a WCS Ecosystem Model file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 477:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_UNSUPPORTEDWCSECOSYSTEMMODELFILEVERSIONPERATIONTERMI ),  // "Unsupported WCS Ecosystem Model file version!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 478:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),  // "Parameters Module: Model"
                    GetString( MSG_PARGUI_YOUHAVENOTSELECTEDAFILENAMEFORINPUTPERATIONTERMINATE ),  // "You have not selected a file name for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 479:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_ERROPENCOSYSMODELFILEFOROUTPUTPERATIONTERMI ),  // "Error opening Ecosystem Model file for output!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 480:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                  // "Parameters Module: Model"
                    GetString( MSG_PARGUI_ERRORWRITINGTOECOSYSTEMMODELFILEPERATIONTERMINATEDPR ),   // "Error writing to Ecosystem Model file!\nOperation terminated prematurely.",
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 481:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_NOTSELECTEDAFILENAMEFOROUTPUTPERATIONTERMINAT ),  // "You have not selected a file name for output!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                    // "OK"
                    (CONST_STRPTR)"o");

        case 482:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),  // "Parameters Module: Anim"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),           // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                    // "OK"
                    (CONST_STRPTR)"o");

        case 483:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),                                 // "Parameters Module: Anim"
                    GetString( MSG_PARGUI_SPECIFIEDWIDTHISLARGERTHANTHECURRENTSCREENWIDTHDOYOU ),  // "Specified width is larger than the current screen width. Do you wish to use the screen width?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 484:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEANIM ),                                  // "Parameters Module: Anim"
                    GetString( MSG_PARGUI_SPECIFIEDORCOMPUTEDHEIGHTISLARGERTHANTHECURRENTSCREE ),  // "Specified or computed height is larger than the current screen height. Do you wish to use the screen height?
                    GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc");

            //.Support.c
            // find . -name "Support.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 485:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_WCSCONFIGURATIONSAVE ),                         // "WCS Configuration: Save"
                    GetString( MSG_SUPPORT_CANTOPENCONFIGURATIONFILEPERATIONTERMINATED ),  // "Can't open configuration file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 486:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_WCSCONFIGURATIONLOAD ),                         // "WCS Configuration: Load"
                    GetString( MSG_SUPPORT_CANTOPENCONFIGURATIONFILEPERATIONTERMINATED ),  // "Can't open configuration file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 487:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_WCSPROJECTSAVE ),                         // "WCS Project: Save"
                    GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 488:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_SUPPORT_PROJECTSAVE ),                          // "Project: Save"
                    GetString( MSG_SUPPORT_SAVEDATABASEANDPARAMETERFILESASWELL ),  // "Save Database and Parameter files as well?"
                    GetString( MSG_SUPPORT_BOTHDBASEPARAMSNO ),                    // "Both|D'base|Params|No",
                    (CONST_STRPTR)"bdpn", 1);

        case 489:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_WCSPROJECTLOAD ),                         // "WCS Project: Load"
                    GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 490:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_PROJECTLOAD ),                           // "Project: Load"
                    GetString( MSG_SUPPORT_NOTAWCSPROJECTFILEPERATIONTERMINATED ),  // "Not a WCS Project file!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                    // "OK"
                    (CONST_STRPTR)"o");

        case 491:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                                        // "Mapping Module: Align"
                    GetString( MSG_SUPPORT_ILLEGALMAPREGISTRATIONVALUESHIGHANDLOWXORYVALUESAREEQUAL ),  // "Illegal map registration values! High and low X or Y values are equal."
                    GetString( MSG_GLOBAL_OK ),                                                        // "OK"
                    (CONST_STRPTR)"o");

        case 492:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_DIRECTORYLISTLOAD ),                      // "Directory List: Load"
                    GetString( MSG_SUPPORT_CANTOPENPROJECTFILEPERATIONTERMINATED ),  // "Can't open project file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                     // "OK"
                    (CONST_STRPTR)"o");

        case 493:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_SUPPORT_DIRECTORYLISTLOAD ),                     // "Directory List: Load"
                    GetString( MSG_SUPPORT_NOTAWCSPROJECTFILEPERATIONTERMINATED ),  // "Not a WCS Project file!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ),                                    // "OK"
                    (CONST_STRPTR)"o");

            //.TimeLinesGUI.c
            // find . -name "TimeLinesGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 494:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
                    GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
                    GetString( MSG_GLOBAL_OK ),                                // "OK"
                    (CONST_STRPTR)"o");

        case 495:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
                    GetString( MSG_TLGUI_NOMOTIONPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERM ),  // "No Motion Parameters with more than one Key Frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 496:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_MOTIONTIMELINE ),  // "Motion Time Line"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),     // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),              // "OK"
                    (CONST_STRPTR)"o");

        case 497:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_MOTIONEDITORTIMELINES ),                                  // "Motion Editor: Time Lines"
                    GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 498:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
                    GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
                    GetString( MSG_GLOBAL_OK ),                                // "OK"
                    (CONST_STRPTR)"o");

        case 499:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
                    GetString( MSG_TLGUI_NOCOLORPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONTERMI ),  // "No Color Parameters with more than one Key Frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 500:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_COLORTIMELINE ),  // "Color Time Line"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),    // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),             // "OK"
                    (CONST_STRPTR)"o");

        case 501:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_COLOREDITORTIMELINES ),                                   // "Color Editor: Time Lines"
                    GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 502:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                // "Parameters: Time Line"
                    GetString( MSG_TLGUI_OUTOFMEMORYANTOPENTIMELINEWINDOW ),  // "Out of memory!\nCan't open Time Line window."
                    GetString( MSG_GLOBAL_OK ),                                // "OK"
                    (CONST_STRPTR)"o");

        case 503:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_PARAMETERSTIMELINE ),                                     // "Parameters: Time Line"
                    GetString( MSG_TLGUI_NOECOSYSTEMPARAMETERSWITHMORETHANONEKEYFRAMEPERATIONT ),  // "No Ecosystem Parameters with more than one Key Frame!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

        case 504:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_ECOSYSTEMTIMELINE ),  // "Ecosystem Time Line"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),        // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),                 // "OK"
                    (CONST_STRPTR)"o");

        case 505:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_TLGUI_ECOSYSTEMEDITORTIMELINES ),                               // "Ecosystem Editor: Time Lines"
                    GetString( MSG_TLGUI_ATLEASTTWOKEYFRAMESFORTHISPARAMETERMUSTBECREATEDPRIOR ),  // "At least two key frames for this parameter must be created prior to opening the time line window"
                    GetString( MSG_GLOBAL_OK ),                                                     // "OK"
                    (CONST_STRPTR)"o");

            //.Tree.c
            // find . -name "Tree.c" -exec grep -A3 -nHis "User_Message" {} \;

        case 506:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
                    GetString( MSG_TREE_ERROROPENINGECOSYSTEMMODELFILEFORINPUTPERATIONTERMINAT ),  // "Error opening Ecosystem Model file for input!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK",
                    (CONST_STRPTR)"o");

        case 507:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
                    GetString( MSG_TREE_ERRORWRITINGTOECOSYSTEMMODELFILEPERATIONTERMINATEDPREM ),  // "Error writing to Ecosystem Model file!\nOperation terminated prematurely."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 508:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
                    GetString( MSG_PARGUI_NOTAWCSECOSYSTEMMODELFILEPERATIONTERMINATED ),  // "Not a WCS Ecosystem Model file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 509:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
                    GetString( MSG_PARGUI_UNSUPPORTEDWCSECOSYSTEMMODELFILEVERSIONPERATIONTERMI ),  // "Unsupported WCS Ecosystem Model file version!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 510:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                   // "Parameters Module: Model"
                    GetString( MSG_TREE_OUTOFMEMORYALLOCATINGECOSYSTEMMODELSPERATIONTERMINATED ),  // "Out of memory allocating Ecosystem Models!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                      // "OK"
                    (CONST_STRPTR)"o");

        case 511:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                        // "Parameters Module: Model"
                    GetString( MSG_TREE_NODATAINWCSECOSYSTEMMODELPERATIONTERMINATED ),  // "No data in WCS Ecosystem Model!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                           // "OK"
                    (CONST_STRPTR)"o");

        case 512:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)PAR_NAME_ECO(0), GetString( MSG_TREE_APROBLEMOCCURREDLOADINGATLEASTONEIMAGEFORTHISECOSYSTEM ),  // "A problem occurred loading at least one image for this ecosystem!\nContinue without it or them?"
                    GetString( MSG_GLOBAL_OKCANCEL ),   // "OK|Cancel"
                    (CONST_STRPTR)"oc");

        case 513:
            //                        IncAndShowTestNumbers(StartTestNumber++,TotalTests);
            //    User_Message(PAR_NAME_ECO(0), "No images found for this ecosystem!\n\
            ./Tree.c-1347-Continue without them?", "OK|Cancel", "oc");

            //.WCS.c  // ALEXANDER
            // find . -name "WCS.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 514:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message((CONST_STRPTR)"World Construction set",
                    GetString( MSG_WCS_BETAPERIODEXPIRED ),     // "Beta period expired..."
                    GetString( MSG_GLOBAL_OK ),                    // "OK"
                    (CONST_STRPTR)"o");

            //.Wave.c
            // find . -name "Wave.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 515:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),          // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTGENERALWAVECENTER ),  // "Select general wave center."
                    GetString( MSG_WAV_FOCUSPOINTCAMERAPOINT ),    // "Focus Point|Camera Point"
                    (CONST_STRPTR)"fc", 0);

        case 516:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Wave: Set Defaults"
                    GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Select wave speed."
                    GetString( MSG_WAV_FASTVERYFASTSLOW ),  //"Fast|Very Fast|Slow"
                    (CONST_STRPTR)"fvs", 1);

        case 517:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAV_WAVESETDEFAULTS ),      // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTWAVEDIRECTION ),  // "Select wave direction."
                    GetString( MSG_WAV_SPREADINGCONVERGING ),  // "Spreading|Converging"
                    (CONST_STRPTR)"sc", 1);;


            //.WaveGUI.c
            // find . -name "WaveGUI.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 518:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_WAVGUI_MAPVIEWWAVES ),  // "Map View: Waves"
                    GetString( MSG_GLOBAL_OUTOFMEMORY ),   // "Out of memory!"
                    GetString( MSG_GLOBAL_OK ),            // "OK"
                    (CONST_STRPTR)"o");

        case 519:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),  // "Wave Editor"
                    GetString( MSG_WAVGUI_THECURRENTWAVEMODELHASBEENMODIFIEDDOYOUWISHTOSAVEITB ),  // "The current Wave Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ),  // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 520:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 521:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ) ,             // "Wave Editor"
                    GetString( MSG_WAVGUI_DELETEALLWAVEKEYFRAMES ),  // "Delete all wave key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                // "OK|Cancel"
                    (CONST_STRPTR)"oc", 1);

        case 522:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 523:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ),                           // "Yes|No"
                    (CONST_STRPTR)"yn", 1);

        case 524:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message_Def(GetString( MSG_WAVGUI_ADDWAVE ),                                               // "Add Wave"
                    GetString( MSG_WAVGUI_MAPVIEWMODULEMUSTBEOPENINORDEROUSETHISFUNCTIONWOULDY ),  // "Map View Module must be open in order\ to use this function. Would you like to open it now?"
                    GetString( MSG_GLOBAL_OKCANCEL ),                                              // "OK|Cancel"
                    (CONST_STRPTR)"oc",1);

        case 525:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_WAVGUI_MAPVIEWWAVEADD ),                                     // "Map View: Wave Add"
                    GetString( MSG_WAVGUI_REMOVEALLCURRENTLYDEFINEDWAVESBEFOREADDINGNEWONES ),  // "Remove all currently defined waves before adding new ones?"
                    GetString( MSG_GLOBAL_YESNO ),                                              // "Yes|No"
                    (CONST_STRPTR)"yn");


            //.nncrunch.c
            // find . -name "nncrunch.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 526:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                     // "Map View: Build DEM"
                    GetString( MSG_NNCRUNCH_INSUFFICIENTDATAINGRIDDEDREGIONTOTRIANGULATEINCREA ),  // "Insufficient data in gridded region to triangulate! Increase the size of the gridded region or add more control points."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");

        case 527:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ),                                      // "Map View: Grid DEM"
                    GetString( MSG_NNCRUNCH_THERATIOOFVERTICALTOHORIZONTALMAPDIMENSIONSISTOOLA ),  // "The ratio of vertical to horizontal map dimensions is too large for gradient estimation. Scale the data if gradients are required.\nDo you wish to continue without gradient estimation?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
                    (CONST_STRPTR)"oc");

        case 528:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ) ,                                     // "Map View: Grid DEM"
                    GetString( MSG_NNCRUNCH_RATIOOFVERTTOHORIZMAPDIMENSIONSISTOOSM ),  // "The ratio of vertical to horizontal map dimensions is too small for gradient estimation. Scale the data if gradients are required.\nDo you wish to continue without gradient estimation?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
                    (CONST_STRPTR)"oc");

        case 529:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_NNCRUNCH_MAPVIEWGRIDDEM ),                                      // "Map View: Grid DEM"
                    GetString( MSG_NNCRUNCH_THERATIOOFWIDTHTOLENGTHOFTHISGRIDDEDREGIONMAYBETOO ),  // "The ratio of width to length of this gridded region may be too extreme for good interpolation.\nChanging the block proportions, or rescaling the x or y coordinate may be a good idea.\nContinue now with the present dimensions?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ),                                      // "Continue|Cancel"
                    (CONST_STRPTR)"oc");

        case 530:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                            // "Map View: Build DEM",
                    GetString( MSG_NNCRUNCH_OUTOFMEMORYDOUBLEMATRIXPERATIONTERMINATED ),  // "Out of memory Double Matrix!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                         // "OK"
                    (CONST_STRPTR)"o");

        case 531:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_MAPGUI_MAPVIEWBUILDDEM ),                                     // "Map View: Build DEM"
                    GetString( MSG_NNCRUNCH_OUTOFMEMORYALLOCATINGDOUBLEMATRIXPERATIONTERMINATE ),  // "Out of memory allocating Double Matrix!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                                                  // "OK"
                    (CONST_STRPTR)"o");


            //.nngridr.c
            // find . -name "nngridr.c" -exec grep -A3 -nHis "User_Message" {} \;
        case 532:
            ShowTestNumbers(StartTestNumber++,TotalTests);
            User_Message(GetString( MSG_NNGRIDR_MAPVIEWGRIDDEM ),                // "Map View: Grid DEM"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ),                             // "OK"
                    (CONST_STRPTR)"o");
    }
}

APTR  AF_MakeModControlWin (APTR BT_Database, APTR BT_DataOps, APTR BT_Mapping, APTR BT_Editing,
                         APTR BT_Render); // extracted from ModControl.c
APTR AF_Make_Credits_Window(void);
APTR AF_MakeAboutWin (APTR *BT_AboutOK);
APTR AF_Make_Info_Window(void);
APTR AF_Make_UM_Win(CONST_STRPTR outlinetxt, CONST_STRPTR message,APTR *UM_BTGroup);
APTR AF_Make_IS_Win(char *message, char *reject, char *string, APTR InputStr, APTR BT_OK, APTR BT_Cancel);
void Make_Log_Window(int Severity);
void Close_Log_Window(int StayClosed);
void Make_CL_Window(void);
void Make_MD_Window(void);
void Make_GR_Window(void);
void Make_DI_Window(void);
void Make_DC_Window(void);
void Open_Diagnostic_Window(struct Window *EcoWin, char *WinTitle);
void Close_EMPL_Window(void);
void Make_PN_Window(void);
void Make_FE_Window(void);
int MapGUI_New(struct MapData *MP);
void MapGUI_Del(struct MapData *MP);
void Close_MA_Window(struct MapData *MP);
APTR AF_MakeScaleWinObject(struct ScaleWindow *PS_Win);
APTR AF_MakeEETLWindow(struct TimeLineWindow *EETL_Win);
APTR AF_MakeECTLWindow(struct TimeLineWindow  *ECTL_Win);
APTR AF_MakeEMTLWindow(struct TimeLineWindow  *EMTL_Win);
APTR AF_MakeWV_Win(struct WaveWindow *WV_Win, short WinNum, char *NameStr, ULONG WinID);

void waitForRightClick(Object *MuiWindow)
{
    struct IntuiMessage *msg;
    BOOL waiting = TRUE;
    struct Window *winptr;
    get(MuiWindow, MUIA_Window_Window, &winptr);

    while (waiting) {
        WaitPort(winptr->UserPort);
        while ((msg = (struct IntuiMessage *)GetMsg(winptr->UserPort))) {
            if (msg->Class == IDCMP_MENUPICK /*&& msg->Code == 0xffff*/) {
                waiting = FALSE;
            }
            ReplyMsg((struct Message *)msg);
        }
    }
}

void Test_IS_Win(CONST STRPTR message)
{
    // Input String Window
    // should this called with all possible strings? (look for all latFixer() calls in the code)
    APTR InputStr=NULL, BT_OK=NULL, BT_Cancel=NULL, IS_Win=NULL;
    // Example strings from MapExtra.c void FlatFixer(struct Box *Bx)
    IS_Win=AF_Make_IS_Win((CONST_STRPTR)message, (CONST_STRPTR)"+-.,abcdefghijklmnopqrstuvwxyz9", "", &InputStr, &BT_OK, &BT_Cancel);
    DoMethod(app, OM_ADDMEMBER, IS_Win);
    set(IS_Win,MUIA_Window_Open, TRUE);
    waitForRightClick(IS_Win); // Wait for right mouse button to be pressed
    set(IS_Win,MUIA_Window_Open, FALSE);
    }

void Test_UM_Win(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons)
{
    APTR UM_Win;
    APTR UM_BTGroup;

    APTR UM_BT[11];
    USHORT numbuttons = 1, j = 0, i = 0, k = 0, error = 0;
    char buttontext[11][20];
    CONST_STRPTR buttonkey="abcdefghijklmno"; // 12 keys, 11 buttons

    while (buttons[i] && numbuttons<12)
    {
        if (buttons[i] == '|')
        {
            buttontext[j][k] = 0;
            j++;
            i++;
            k=0;
            continue;
        }
        buttontext[j][k] = buttons[i];
        i++;
        k++;
    }
    buttontext[j][k] = 0;
    numbuttons = j + 1;

    if (numbuttons > 10) numbuttons = 10;

    UM_Win=AF_Make_UM_Win(outlinetxt,
            message,
            &UM_BTGroup);


    for (j=0; j<numbuttons-1  && ! error; j++)
    {
        UM_BT[j + 1] = KeyButtonObject(buttonkey[j]),
                MUIA_Text_PreParse, "\33c",
                MUIA_Text_Contents, buttontext[j], End;
        if (! UM_BT[j + 1]) error = 1;
        else DoMethod(UM_BTGroup, OM_ADDMEMBER, UM_BT[j + 1]);
    } /* for j=0... */

    UM_BT[0] = KeyButtonObject(buttonkey[numbuttons - 1]),
            MUIA_Text_PreParse, "\33c",
            MUIA_Text_Contents, buttontext[numbuttons - 1], End;
    if (! UM_BT[0]) error = 1;
    else DoMethod(UM_BTGroup, OM_ADDMEMBER, UM_BT[0]);

    if (error)
    {
        MUI_DisposeObject(UM_Win);
        printf("Error in %s %s Line %d!\n",__FILE__,__func__,__LINE__);
        return; // exit if error creating buttons
        /*   break;*/
    } /* if error creating buttons */

    DoMethod(app, OM_ADDMEMBER, UM_Win);
    set(UM_Win,MUIA_Window_Open, TRUE);
    waitForRightClick(UM_Win); // Wait for right mouse button to be pressed
    set(UM_Win,MUIA_Window_Open, FALSE);
}


void Test_WindowObject(unsigned int StartTestNumber, int ShouldBreak)
{
    unsigned int TotalTests=332; // TotalTests is the number of cases here

    //   APTR BT_Database=NULL, BT_DataOps=NULL, BT_Mapping=NULL, BT_Editing=NULL,BT_Render=NULL,
    APTR BT_AboutOK=NULL;
    //   APTR UM_BTGroup=NULL;
    APTR UM_Win;

    dbaseloaded=1; // Set dbaseloaded to 1, so that the Windows do not complain
    paramsloaded=1; // Set paramsloaded to 1, so that the Windows do not complain


    set(ModControlWin,MUIA_Window_Open, FALSE);

    if(StartTestNumber<1)
    {
        StartTestNumber=1;
    }

    switch(StartTestNumber)
    {
        default:
        {
            printf("Test_WindowObject: Invalid Test_User_Message Number %u\n", StartTestNumber);
            return; // Invalid StartTestNumber, exit the function
        }

        case 1:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            set(ModControlWin,MUIA_Window_Open, TRUE);
            waitForRightClick(ModControlWin); // Wait for right mouse button to be pressed
            set(ModControlWin,MUIA_Window_Open, FALSE);
            if(ShouldBreak) break;
        }

        case 2:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            Make_EP_Window(0);   // Edit Parameters Window
            waitForRightClick(EP_Win->EditWindow); // Wait for right mouse button to be pressed
            Close_EP_Window();
            if(ShouldBreak) break;
        }

        case 3:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            Make_DB_Window(0); // Database Window
            waitForRightClick(DB_Win->DatabaseWindow); // Wait for right mouse button to be pressed
            Close_DB_Window();
            if(ShouldBreak) break;
        }

        case 4:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            Make_DO_Window(0); // Data Operations Window
            waitForRightClick(DO_Win->DataOpsWindow); // Wait for right mouse button to be pressed
            Close_DO_Window();
            if(ShouldBreak) break;
        }

        case 5:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            CreditWin=AF_Make_Credits_Window(); // Credits Window
            DoMethod(app, OM_ADDMEMBER, CreditWin);
            set(CreditWin,MUIA_Window_Open, TRUE);
            waitForRightClick(CreditWin); // Wait for right mouse button to be pressed
            set(CreditWin,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, CreditWin);
            MUI_DisposeObject(CreditWin);
            if(ShouldBreak) break;
        }

        case 6:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            AboutWin=AF_MakeAboutWin(&BT_AboutOK);
            DoMethod(app, OM_ADDMEMBER, AboutWin);
            set(AboutWin,MUIA_Window_Open, TRUE);
            waitForRightClick(AboutWin); // Wait for right mouse button to be pressed
            set(AboutWin,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, AboutWin);
            MUI_DisposeObject(AboutWin);
            if(ShouldBreak) break;
        }

        case 7:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            InfoWin=AF_Make_Info_Window();
            DoMethod(app, OM_ADDMEMBER, InfoWin);
            set(InfoWin,MUIA_Window_Open, TRUE);
            waitForRightClick(InfoWin); // Wait for right mouse button to be pressed
            set(InfoWin,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, InfoWin);
            MUI_DisposeObject(InfoWin);
            if(ShouldBreak) break;
        }

        case 8:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            //    UM_Win=AF_Make_UM_Win("Some outline text", "A message...\n(Not to be translated in this demo.)", UM_BTGroup);
            //    DoMethod(app, OM_ADDMEMBER, UM_Win);
            //    set(UM_Win,MUIA_Window_Open, TRUE);
            //    waitForRightClick(UM_Win); // Wait for right mouse button to be pressed
            //    set(UM_Win,MUIA_Window_Open, FALSE);

            //####### User Message Window ############
//            if(ShouldBreak) break;
        }
        case 9:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_OBJECTISNOTCLOSEDHEORIGINCANNOTBEMOVEDETLASTVERTEX ),  // "Object is not closed!\nThe origin cannot be moved.\nSet last vertex equal to first now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));
            if(ShouldBreak) break;
        }

        case 10:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 11:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),  // "Mapping Module: Point Match"
                    GetString( MSG_MAPEXTRA_PROCEEDWITHRELOCATION ),    // "Proceed with relocation?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|CANCEL"
            if(ShouldBreak) break;
        }

        case 12:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                               // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 13:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 14:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_DUPLICATETHISOBJECT ),  // "Duplicate this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ));             // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 15:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),  // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_SAVEVECTOROBJECTNOW ),        // "Save vector object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 16:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULESPLINE ),                       // "Mapping Module: Spline"
                    (CONST_STRPTR)str,
                    GetString( MSG_MAPEXTRA_OKRESETCANCEL ));                             // "OK|Reset|Cancel"
            if(ShouldBreak) break;
        }

        case 17:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize",
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 18:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),  // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_PROCEEDORRESETPOINTS ),   // "Proceed or reset points?"
                    GetString( MSG_MAPEXTRA_PROCEEDRESETCANCEL ));     // "Proceed|Reset|Cancel"
            if(ShouldBreak) break;
        }

        case 19:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),           // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_KEEPORSAVEDEMORRESETPARAMETERS ),  // "Keep or save DEM or reset parameters?"
                    GetString( MSG_MAPEXTRA_KEEPSAVERESETCANCEL ));             // "Keep|Save|Reset|Cancel"
            if(ShouldBreak) break;
        }

        case 20:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                          // "Render Module"
                    GetString( MSG_MAPLINO_ERROROPENINGLINESAVEFILEELECTNEWPATH ),  // "Error opening line save file!\nSelect new path?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                              // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 21:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),                          // "Parameters Module: Ecosystem"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));                                           // "OK|Cancel",
            if(ShouldBreak) break;
        }

        case 22:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module
                    GetString( MSG_GLMP_DIAGNOSTICBUFFERSCANTBEGENERATEDFORMULTIPLESEGMENTORMU ),  // "Diagnostic buffers can't be generated for multiple segment or multiple frame renderings! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                                // "OK|CANCEL"
            if(ShouldBreak) break;
        }

        case 23:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGDIAGNOSTICBUFFERSPROCEEDRENDERINGWIT ),  // "Out of memory opening Diagnostic buffers! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                                // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 24:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                  // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORLOADINGCLOUDMAPFILEONTINUEWITHOUTCLOUDSHADOWS ),  // "Error loading Cloud Map file!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                      // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 25:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                      // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGCLOUDMAPONTINUEWITHOUTCLOUDSHADOWS ),   // "Out of memory creating Cloud Map!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 26:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGMASTERCOLORMAPSEESTATUSLOGFORMOREINFORMATI ),  // "Error loading Master Color Map! See Status Log for more information.\n\Continue rendering without Color Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 27:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGSTRATADEFORMATIONMAPCONTINUERENDERINGWITHO ),  // "Error loading Strata Deformation Map!\n\Continue rendering without Deformation Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 28:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGNOISEMAPCONTINUERENDERINGWITHOUTTEX ),  // "Out of memory creating Noise Map!\n\Continue rendering without Texture Noise?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 29:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 30:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 31:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULESAVE ),                           // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGBITMAPPEDIMAGETRYANOTHERDEVICE ),  // "Error saving bitmapped image! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 32:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYSAVINGZBUFFER ),  // "Out of memory saving Z Buffer!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));               // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 33:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULESAVE ),                    // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGZBUFFERTRYANOTHERDEVICE ),  // "Error saving Z Buffer! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                            // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 34:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                 // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                     // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 35:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                               // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 36:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                       // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGPOLYGONEDGEBUFFERS ),  // "Out of memory allocating polygon edge buffers!",
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                              // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 37:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                  // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 38:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                  // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 39:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 40:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                     // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGCLOUDKEYFRAMESPERATIONTERMINATED ),  // "Out of memory allocating Cloud Key Frames!\nOperation terminated"
                    GetString( MSG_GLOBAL_OK ));                                                     // "OK"
            if(ShouldBreak) break;
        }

        case 41:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULENAME ),                             // "Database Module: Name"
                    GetString( MSG_EDDB_OBJECTNAMEALREADYPRESENTINDATABASERYANEWNAME ) ,  // "Object name already present in database!\nTry a new name?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 42:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDDB_DATABASEMODULEREMOVEITEM ),                                          // "Database Module: Remove Item
                    GetString( MSG_EDDB_DELETEOBJECTELEVATIONANDRELATIVEELEVATIONFILESFROMDISKASWELL ),      // "Delete object, elevation and relative elevation files from disk as well as remove their names from the Database?"
                    GetString( MSG_EDDB_FROMDISKDATABASEONLYCANCEL ));                                        // "From Disk|Database Only|Cancel",
            if(ShouldBreak) break;
        }

        case 43:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_CLOUD_CLOUDEDITORSETBOUNDS ) ,     // "Cloud Editor:Set Bounds"
                    (CONST_STRPTR) GetString( MSG_CLOUD_MAPVIEWMODULEMUSTBEOPEN ) ,  // "Map View Module must be open in order to use this function. Would you like to open it now?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 44:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),               // "Mapping Module: Align"
                    GetString( MSG_CLOUD_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSET ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                      // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 45:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                               // "Retry|Cancel"
            if(ShouldBreak) break;
        }

        case 46:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                        // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                             //"Retry|Cancel",
            if(ShouldBreak) break;
        }

        case 47:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ) ,  // "Parameters Module: Model"
                    GetString( MSG_CLOUDGUI_THECURRENTCLOUDMODELHASBEENMODIFIEDDOYOUWISHTOSAVE ) ,  // "The current Cloud Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ));  // "Yes|No"
            if(ShouldBreak) break;
        }

        case 48:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,               // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ), // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 49:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,     // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_DELETEALLCLOUDKEYFRAMES ) ,  // "Delete all cloud key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 50:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);
            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                             // "Yes|No"
            if(ShouldBreak) break;
        }
        case 51:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"

                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                            // "Yes|No",
            if(ShouldBreak) break;
        }

        case 52:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) ,
                    (CONST_STRPTR)str,
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "Parameter Editing: Defaults", str, "OK|Cancel"
            if(ShouldBreak) break;
        }

        case 53:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_PUBLICSCREENSTILLHASVISITORSTRYCLOSINGAGAIN ) ,  // "Public Screen still has visitors. Try closing again?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ));  // "Close|Warn|Cancel"
            if(ShouldBreak) break;
        }
        case 54:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_QUITPROGRAMREYOUSURE ) ,  // )"Quit Program\nAre you sure?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ));  // "Close|Warn|Cancel"
            if(ShouldBreak) break;
        }
        case 55:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_WCSPROJECT ) ,  // "WCS Project"
                    (CONST_STRPTR) GetString( MSG_AGUI_PROJECTPATHSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Project paths have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 56:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERMODULE ) ,  // "Parameter Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Parameters have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 57:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_DATABASEHASBEENMODIFIEDSAVEITBEFORECLOSING ) ,  // "Database has been modified. Save it before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 58:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win("", (CONST_STRPTR) GetString( MSG_AGUI_KEEPCHANGES ) , (CONST_STRPTR) GetString( MSG_AGUI_KEEPCANCEL ));  // "Keep changes?", "Keep|Cancel"
            if(ShouldBreak) break;
        }

        case 59:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win("",  GetString( MSG_AGUI_FILEALREADYEXISTSOYOUWISHTOOVERWRITEIT ) ,  // "File already exists.\nDo you wish to overwrite it?"
                    GetString( MSG_GLOBAL_OKCANCEL )); // "OK|CANCEL"
            if(ShouldBreak) break;
        }

        case 60:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_CLOUD_CLOUDEDITORSETBOUNDS ) ,     // "Cloud Editor:Set Bounds"
                    (CONST_STRPTR) GetString( MSG_CLOUD_MAPVIEWMODULEMUSTBEOPEN ) ,  // "Map View Module must be open in order to use this function. Would you like to open it now?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 61:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),               // "Mapping Module: Align"
                    GetString( MSG_CLOUD_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSET ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                      // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 62:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ) ,  // "Parameters Module: Model"
                    GetString( MSG_CLOUDGUI_THECURRENTCLOUDMODELHASBEENMODIFIEDDOYOUWISHTOSAVE ) ,  // "The current Cloud Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ));  // "Yes|No"
            if(ShouldBreak) break;
        }
        case 63:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,               // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ), // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 64:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,     // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_DELETEALLCLOUDKEYFRAMES ) ,  // "Delete all cloud key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 65:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                             // "Yes|No"
            if(ShouldBreak) break;
        }

        case 66:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_CLOUDGUI_CLOUDEDITOR ) ,                 // "Cloud Editor"
                    GetString( MSG_CLOUDGUI_MAKETHISFILETHEPROJECTCLOUDFILE ) ,  // "Make this file the Project Cloud File?"
                    GetString( MSG_GLOBAL_YESNO ));                            // "Yes|No",
            if(ShouldBreak) break;
        }
        case 67:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"rootfile",
                    GetString( MSG_DEM_DEMNAMEISTOOLONGTOADDANEXTRACHARACTERTODOYOUWISHTOENTER ) ,  // "DEM name is too long to add an extra character to. Do you wish to enter a new base name for the DEM or abort the interpolation?"
                    GetString( MSG_DEM_NEWNAMEABORT ));                                              // "New Name|Abort"
            if(ShouldBreak) break;
        }
        case 68:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULESAVE ), (CONST_STRPTR)"Database-File",                   // "Database Module: Save"
                    GetString( MSG_DB_OKCANCEL ));                                                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 69:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULENAME ),                            // "Database Module: Name"
                    GetString( MSG_DB_VECTORNAMEALREADYPRESENTINDATABASERYANEWNAME ),  // "Vector name already present in database!\nTry a new name?"
                    GetString( MSG_DB_OKCANCEL ));                                      // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 70:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str,
                    GetString( MSG_DB_OBJECTNAMEALREADYPRESENTINDATABASEUPLICATEITEMSWILLBESKI ),  // "Object name already present in database!\nDuplicate items will be skipped."
                    GetString( MSG_GLOBAL_OK ));                                                        // "OK"
            if(ShouldBreak) break;
        }
        case 71:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),     // "Data Ops: Convert DEM"
                    (CONST_STRPTR)str,
                    GetString( MSG_DATAOPS_CONTINUETRUNCATECANCEL ));   // "Continue|Truncate|Cancel"
            if(ShouldBreak) break;
        }
        case 72:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str,
                    GetString( MSG_DISPGUI_MAKETHISTHEDEFAULTOBJECTDIRECTORY ),  // "Make this the default object directory?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                           // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 73:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULENAME ),                             // "Database Module: Name"
                    GetString( MSG_EDDB_OBJECTNAMEALREADYPRESENTINDATABASERYANEWNAME ) ,  // "Object name already present in database!\nTry a new name?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 74:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDDB_DATABASEMODULEREMOVEITEM ),                                          // "Database Module: Remove Item
                    GetString( MSG_EDDB_DELETEOBJECTELEVATIONANDRELATIVEELEVATIONFILESFROMDISKASWELL ),      // "Delete object, elevation and relative elevation files from disk as well as remove their names from the Database?"
                    GetString( MSG_EDDB_FROMDISKDATABASEONLYCANCEL ));                                        // "From Disk|Database Only|Cancel",
            if(ShouldBreak) break;
        }
        case 75:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDECOGUI_PARAMETERSMODULEECOSYSTEM ),                          // "Parameters Module: Ecosystem"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));                                           // "OK|Cancel",
            if(ShouldBreak) break;
        }
        case 76:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_PARAMETERSMODULEMOTION ),                       // "Parameters Module: Motion"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));                                     // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 77:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_PARAMETERSMODULEMAKEKEY ),  // "Parameters Module: Make Key"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_YESNO ));                    // "Yes|No"
            if(ShouldBreak) break;
        }
        case 78:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THISISANOLDV1FORMATFILEWOULDYOULIKETORESAVEITINTHENEW ),  // "This is an old V1 format file! Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 79:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THEPARAMETERFILEFORMATHASBEENCHANGEDSLIGHTLYSINCETHIS ),  // "The Parameter File format has been changed slightly since this file was saved. Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 80:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     //  "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO));                 // "Yes|No"

            if(ShouldBreak) break;
        }
        case 81:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     // "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO ));                // "Yes|No"
            if(ShouldBreak) break;
        }
        case 82:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDITGUI_PARAMETERSMODULECOLOR ),  // "Parameters Module: Color"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 83:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDITGUI_COLOREDITORCOPY ) ,  // "Color Editor: Copy"
                    GetString( MSG_EDITGUI_COPYKEYFRAMESTOO ),   // "Copy Key Frames too?"
                    GetString( MSG_GLOBAL_YESNO ));              // "Yes|No"
            if(ShouldBreak) break;
        }
        case 84:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)PAR_NAME_ECO(1),
                    GetString( MSG_EDITGUI_THECURRENTCOLORISBEINGUSEDREMOVEITANYWAY ),  // "The current color is being used. Remove it anyway?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 85:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 86:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 87:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 88:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 89:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 90:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 91:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 92:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 93:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 94:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 95:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"
            if(ShouldBreak) break;
        }
        case 96:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
                    GetString( MSG_AGUI_KEEPCHANGES ),    // "Keep changes?"
                    GetString( MSG_GLOBAL_YESNO ));          // "Yes|No"
            if(ShouldBreak) break;
        }
        case 97:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_UNABLETOLOADIMAGEFILEFORVIEWINGPERATIONTERMINATED ),   // "Unable to load image file for viewing!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ));                                                  // "OK"
            if(ShouldBreak) break;
        }
        case 98:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORLOADECOTYPE ),                   // "Foliage Editor: Load Ecotype"
                    GetString( MSG_FOLIGUI_ERRORLOADINGECOTYPEFILEPERATIONTERMINATED ),  // "Error loading Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                         // "OK"
            if(ShouldBreak) break;
        }
        case 99:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                            // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                               // "OK"
            if(ShouldBreak) break;
        }
        case 100:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                           // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_ERRORLOADINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error loading Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                              // "OK"
            if(ShouldBreak) break;
        }
        case 101:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                               // "OK"
            if(ShouldBreak) break;
        }
        case 102:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                              // "OK"
            if(ShouldBreak) break;
        }
        case 103:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEGROUP ),                         // "Foliage Editor: Save Group"
                    GetString( MSG_FOLIGUI_ERRORSAVINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error saving Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                             // "OK"
            if(ShouldBreak) break;
        }
        case 104:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                             // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ) ,  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                                // "OK"
            if(ShouldBreak) break;
        }
        case 105:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                    // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_ERRORLOADINGIMAGEFILEPERATIONTERMINATED ),  // "Error loading image file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                       // "OK"
            if(ShouldBreak) break;
        }
        case 106:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_THEIMAGELOADEDPROPERLYMAYBESOMEDAYTHEREWILLEVENBEAW ),  // "The image loaded properly. Maybe some day there will even be a way for you to see it!\n"
                    GetString( MSG_FOLIGUI_THATWOULDBENICE ));               // "That would be nice"
            if(ShouldBreak) break;
        }
        case 107:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEECOTYPE ),                  // "Foliage Editor: Save Ecotype"
                    GetString( MSG_FOLIGUI_ERRORSAVINGECOTYPEFILEPERATIONTERMINATED ),  // "Error saving Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                        // "OK"
            if(ShouldBreak) break;
        }
        case 108:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module
                    GetString( MSG_GLMP_DIAGNOSTICBUFFERSCANTBEGENERATEDFORMULTIPLESEGMENTORMU ),  // "Diagnostic buffers can't be generated for multiple segment or multiple frame renderings! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                                // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 109:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYOPENINGDIAGNOSTICBUFFERSPROCEEDRENDERINGWIT ),  // "Out of memory opening Diagnostic buffers! Proceed rendering without them?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                                // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 110:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                  // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORLOADINGCLOUDMAPFILEONTINUEWITHOUTCLOUDSHADOWS ),  // "Error loading Cloud Map file!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                      // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 111:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                      // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGCLOUDMAPONTINUEWITHOUTCLOUDSHADOWS ),   // "Out of memory creating Cloud Map!\nContinue without cloud shadows?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 112:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGMASTERCOLORMAPSEESTATUSLOGFORMOREINFORMATI ),  // "Error loading Master Color Map! See Status Log for more information.\n\Continue rendering without Color Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 113:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_ERRORLOADINGSTRATADEFORMATIONMAPCONTINUERENDERINGWITHO ),  // "Error loading Strata Deformation Map!\n\Continue rendering without Deformation Map?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 114:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                                            // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYCREATINGNOISEMAPCONTINUERENDERINGWITHOUTTEX ),  // "Out of memory creating Noise Map!\n\Continue rendering without Texture Noise?"
                    GetString( MSG_GLOBAL_CONTINUECANCEL ));                                          // "Continue|Cancel"
            if(ShouldBreak) break;
        }
        case 115:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 116:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 117:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULESAVE ),                           // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGBITMAPPEDIMAGETRYANOTHERDEVICE ),  // "Error saving bitmapped image! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 118:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYSAVINGZBUFFER ),  // "Out of memory saving Z Buffer!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));               // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 119:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULESAVE ),                    // "Render Module: Save"
                    GetString( MSG_GLMP_ERRORSAVINGZBUFFERTRYANOTHERDEVICE ),  // "Error saving Z Buffer! Try another device?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                            // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 120:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                 // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                     // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 121:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                               // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 122:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                       // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGPOLYGONEDGEBUFFERS ),  // "Out of memory allocating polygon edge buffers!",
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                              // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 123:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                  // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 124:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),           // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMCREATCLOUDMAP ),  // "Out of memory creating Cloud Map!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                  // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 125:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                   // "Render Module: Clouds"
                    GetString( MSG_GLMP_ERRORCREATINGCLOUDMAPEITHEROUTOFMEMORYORUSERABORTED ),  // "Error creating Cloud Map! Either out of memory or user aborted."
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                                          // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 126:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_GLMP_RENDERMODULECLOUDS ),                                     // "Render Module: Clouds"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGCLOUDKEYFRAMESPERATIONTERMINATED ),  // "Out of memory allocating Cloud Key Frames!\nOperation terminated"
                    GetString( MSG_GLOBAL_OK ));                                                     // "OK"
            if(ShouldBreak) break;
        }
        case 127:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),             // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYLOADINGDEMSNCREASEGRIDSIZE ),  // "Out of memory loading DEMs!\nIncrease grid size?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 128:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_CAMERAVIEW ),                     // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYALLOCATINGDEMARRAY ),  // "Out of memory allocating DEM array!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                    // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 129:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_INTVIEW_PARAMETERSMODULEPREVIEW ),                      // "Parameters Module: Preview"
                    GetString( MSG_INTVIEW_RESTORETHEPARAMETERSUSEDTOCREATETHISPREVIEW ),  // "Restore the Parameters used to create this preview?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                     // GetString( MSG_INTVIEW_OKCANCEL )"OK|Cancel"
            if(ShouldBreak) break;
        }
        case 130:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"Example DEMName",
                    GetString( MSG_LWSPRT_ERRORLOADINGDEMOBJECTPERATIONTERMINATED ),  // "Error loading DEM Object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                       // "OK"
            if(ShouldBreak) break;
        }
        case 131:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"Example DEMName",
                    GetString( MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED ),  // "Error loading DEM Object!\nObject not saved."
                    GetString( MSG_GLOBAL_OK ));                                  // "OK"
            if(ShouldBreak) break;
        }
        case 132:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWOBJECTEXPORT ),                 // "LW Object Export"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                             // "OK"
            if(ShouldBreak) break;
        }
        case 133:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_APROBLEMOCCURREDSAVINGTHELWSCENEFAFILEWASCREATEDITWILLNOTBE ),  // "A problem occurred saving the LW scene.\nIf a file was created it will not be complete and may not load properly into LightWave."
                    GetString( MSG_GLOBAL_OK ));                                                           // "OK"
            if(ShouldBreak) break;
        }
        case 134:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_THEOUTPUTIMAGESIZEISNOTASTANDARDLIGHTWAVEIMAGESIZETHEZOOMFA ),  // "The output image size is not a standard LightWave image size. The zoom factor and image dimensions may not be portrayed correctly in the scene file just created."
                    GetString( MSG_GLOBAL_OK ));                                                           // "OK"
            if(ShouldBreak) break;
        }
        case 135:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),  // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ));                                     // "Active|New|Cancel"
            if(ShouldBreak) break;
        }
        case 136:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_LINESPRT_SAVEOBJECTPOINTS ),  // "Save object points?"
                    GetString( MSG_GLOBAL_OKCANCEL ));          // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 137:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str, GetString( MSG_LINESPRT_USEELEVATIONDATA ),  // "Use elevation data?"
                    GetString( MSG_GLOBAL_YESNO ));                                // "Yes|No"
            if(ShouldBreak) break;
        }
        case 138:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current flattening, datum and vertical exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ));                                               // "Yes|No"
            if(ShouldBreak) break;
        }
        case 139:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current Flattening, Datum and Vertical Exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ));                                               // "Yes|No"
            if(ShouldBreak) break;
        }
        case 140:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module"
                    GetString( MSG_LINESPRT_VECTORNAMEALREADYPRESENTINDATABASEVERWRITEITORTRYA ),  // "Vector name already present in Database!\nOverwrite it or try a new name?"
                    GetString( MSG_LINESPRT_OVERWRITENEWCANCEL ));                                  // "Overwrite|New|Cancel"
            if(ShouldBreak) break;
        }
        case 141:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
                    GetString( MSG_MAP_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSETO ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 142:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAP_MAPVIEWTOPODRAW ),                                // "Map View: Topo Draw"
                    GetString( MSG_MAP_MEMORYALLOCATIONFAILURECANNOTDRAWTOPOCONTINUE ),  // "Memory allocation failure, cannot draw topo. Continue?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 143:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),  // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ));                   // "YES|NO"
            if(ShouldBreak) break;
        }
        case 144:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name", GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),   // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ));                                                 // "YES|NO"
            if(ShouldBreak) break;
        }
        case 145:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAP_MAPVIEWMULTISELECT ),     // "Map View: Multi-Select"
                    GetString( MSG_MAP_SELECTORDESELECTITEMS ),  // "Select or de-select items?"
                    GetString( MSG_MAP_SELECTDESELECTCANCEL ));   // "Select|De-select|Cancel"
            if(ShouldBreak) break;
        }
        case 146:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),   // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ));     // "Active|New|Cancel"
            if(ShouldBreak) break;
        }
        case 147:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),   // "Mapping Module: Digitize"
                    GetString( MSG_MAP_ACCEPTNEWPOINTS ),         // "Accept new points?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 148:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 149:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 150:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_CREATEVISUALSENSITIVITYMAPFORTHISOBJECT ),  // "Create Visual Sensitivity map for this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 151:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_OBJECTISNOTCLOSEDHEORIGINCANNOTBEMOVEDETLASTVERTEX ),  // "Object is not closed!\nThe origin cannot be moved.\nSet last vertex equal to first now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                            // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 152:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 153:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEPOINTMATCH ),  // "Mapping Module: Point Match"
                    GetString( MSG_MAPEXTRA_PROCEEDWITHRELOCATION ),    // "Proceed with relocation?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                 // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 154:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 155:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize"
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save Object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 156:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAPEXTRA_DUPLICATETHISOBJECT ),  // "Duplicate this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ));             // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 157:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFOLLOWSTREAM ),  // "Mapping Module: Follow Stream"
                    GetString( MSG_MAPEXTRA_SAVEVECTOROBJECTNOW ),        // "Save vector object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 158:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULESPLINE ),                       // "Mapping Module: Spline"
                    (CONST_STRPTR)str,
                    GetString( MSG_MAPEXTRA_OKRESETCANCEL ));                             // "OK|Reset|Cancel"
            if(ShouldBreak) break;
        }
        case 159:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),                   // "Mapping Module: Digitize",
                    GetString( MSG_MAPEXTRA_CONFORMVECTORTOTERRAINANDSAVEOBJECTNOW ),  // "Conform vector to terrain and save object now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 160:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),  // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_PROCEEDORRESETPOINTS ),   // "Proceed or reset points?"
                    GetString( MSG_MAPEXTRA_PROCEEDRESETCANCEL ));     // "Proceed|Reset|Cancel"
            if(ShouldBreak) break;
        }
        case 161:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPEXTRA_MAPPINGMODULEFIXFLATS ),           // "Mapping Module: Fix Flats"
                    GetString( MSG_MAPEXTRA_KEEPORSAVEDEMORRESETPARAMETERS ),  // "Keep or save DEM or reset parameters?"
                    GetString( MSG_MAPEXTRA_KEEPSAVERESETCANCEL ));             // "Keep|Save|Reset|Cancel"
            if(ShouldBreak) break;
        }
        case 162:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                          // "Render Module"
                    GetString( MSG_MAPLINO_ERROROPENINGLINESAVEFILEELECTNEWPATH ),  // "Error opening line save file!\nSelect new path?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                              // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 163:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAPSUPRT_VECTOROBJECTHASBEENMODIFIEDAVEITBEFORECLOSING ),  // "Vector object has been modified!\nSave it before closing?"
                    GetString( MSG_MAPSUPRT_SAVECANCEL ));                                     // "SAVE|CANCEL"
            if(ShouldBreak) break;
        }
        case 164:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_SELECTEDMAPISNOTCURRENTLYLOADEDOYOUWISHTOLOADTOPOM ),  // "Selected map is not currently loaded!\nDo you wish to load topo maps?",
                    GetString( MSG_GLOBAL_OKCANCEL ));                                            // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 165:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                    // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_INCLUDEDEMELEVATIONDATAINCOLORMAP ),  // "Include DEM elevation data in Color Map?"
                    GetString( MSG_GLOBAL_YESNO ));                              // "Yes|No"
            if(ShouldBreak) break;
        }
        case 166:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_RENDERMODULE ),                              // "Render Module"
                    GetString( MSG_GLMP_OUTOFMEMORYALLOCATINGSMOOTHINGINDEXARRAY ),  // "Out of memory allocating Smoothing Index array!"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                               // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 167:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPTOPOOB_RENDERMODULETOPO ),                        // "Render Module: Topo"
                    (CONST_STRPTR)str,
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                             //"Retry|Cancel",
            if(ShouldBreak) break;
        }
        case 168:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_THECURRENTECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_1 ),  // "The current Ecosystem Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_PARGUI_YESNOCANCEL ));                                           // "Yes|No|Cancel"
            if(ShouldBreak) break;
        }
        case 169:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2 ),  //" The current Ecosystem Model has been modified. Do you wish to save it before proceeding?"
                    GetString( MSG_PARGUI_YESNOCANCEL ));                                           // "Yes|No|Cancel"
            if(ShouldBreak) break;
        }
        case 170:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_SUPPORT_PROJECTSAVE ),                          // "Project: Save"
                    GetString( MSG_SUPPORT_SAVEDATABASEANDPARAMETERFILESASWELL ),  // "Save Database and Parameter files as well?"
                    GetString( MSG_SUPPORT_BOTHDBASEPARAMSNO ));                    // "Both|D'base|Params|No",
            if(ShouldBreak) break;
        }
        case 171:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),          // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTGENERALWAVECENTER ),  // "Select general wave center."
                    GetString( MSG_WAV_FOCUSPOINTCAMERAPOINT ));    // "Focus Point|Camera Point"
            if(ShouldBreak) break;
        }
        case 172:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Wave: Set Defaults"
                    GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Select wave speed."
                    GetString( MSG_WAV_FASTVERYFASTSLOW ));  //"Fast|Very Fast|Slow"
            if(ShouldBreak) break;
        }
        case 173:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),      // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTWAVEDIRECTION ),  // "Select wave direction."
                    GetString( MSG_WAV_SPREADINGCONVERGING ));  // "Spreading|Converging"
            if(ShouldBreak) break;
        }
        case 174:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),  // "Wave Editor"
                    GetString( MSG_WAVGUI_THECURRENTWAVEMODELHASBEENMODIFIEDDOYOUWISHTOSAVEITB ),  // "The current Wave Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ));  // "Yes|No"
            if(ShouldBreak) break;
        }
        case 175:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 176:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ) ,             // "Wave Editor"
                    GetString( MSG_WAVGUI_DELETEALLWAVEKEYFRAMES ),  // "Delete all wave key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 177:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 178:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 179:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_ADDWAVE ),                                               // "Add Wave"
                    GetString( MSG_WAVGUI_MAPVIEWMODULEMUSTBEOPENINORDEROUSETHISFUNCTIONWOULDY ),  // "Map View Module must be open in order\ to use this function. Would you like to open it now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                              // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 180:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_INTVIEW_PARAMETERSMODULECAMERAVIEW ),             // "Parameters Module: Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYLOADINGDEMSNCREASEGRIDSIZE ),  // "Out of memory loading DEMs!\nIncrease grid size?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 181:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_CAMERAVIEW ),                     // "Camera View"
                    GetString( MSG_INTVIEW_OUTOFMEMORYALLOCATINGDEMARRAY ),  // "Out of memory allocating DEM array!\n"
                    GetString( MSG_INTVIEW_RETRYCANCEL ));                    // "Retry|Cancel"
            if(ShouldBreak) break;
        }
        case 182:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_INTVIEW_PARAMETERSMODULEPREVIEW ),                      // "Parameters Module: Preview"
                    GetString( MSG_INTVIEW_RESTORETHEPARAMETERSUSEDTOCREATETHISPREVIEW ),  // "Restore the Parameters used to create this preview?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                     // GetString( MSG_INTVIEW_OKCANCEL )"OK|Cancel"
            if(ShouldBreak) break;
        }
        case 183:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DATAOPSGUI_DATAOPSCONVERTDEM ),     // "Data Ops: Convert DEM"
                    (CONST_STRPTR)str,
                    GetString( MSG_DATAOPS_CONTINUETRUNCATECANCEL ));   // "Continue|Truncate|Cancel"
            if(ShouldBreak) break;
        }
        case 184:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDITGUI_PARAMETERSMODULECOLOR ),  // "Parameters Module: Color"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 185:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDITGUI_COLOREDITORCOPY ) ,  // "Color Editor: Copy"
                    GetString( MSG_EDITGUI_COPYKEYFRAMESTOO ),   // "Copy Key Frames too?"
                    GetString( MSG_GLOBAL_YESNO ));              // "Yes|No"
            if(ShouldBreak) break;
        }
        case 186:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"PAR_NAME_ECO(Eco)",
                    GetString( MSG_EDITGUI_THECURRENTCOLORISBEINGUSEDREMOVEITANYWAY ),  // "The current color is being used. Remove it anyway?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 187:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAPSUPRT_VECTOROBJECTHASBEENMODIFIEDAVEITBEFORECLOSING ),  // "Vector object has been modified!\nSave it before closing?"
                    GetString( MSG_MAPSUPRT_SAVECANCEL ));                                     // "SAVE|CANCEL"
            if(ShouldBreak) break;
        }
        case 188:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                                     // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_SELECTEDMAPISNOTCURRENTLYLOADEDOYOUWISHTOLOADTOPOM ),  // "Selected map is not currently loaded!\nDo you wish to load topo maps?",
                    GetString( MSG_GLOBAL_OKCANCEL ));                                            // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 189:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPSUPRT_MAPVIEWCOLORMAP ),                    // "Map View: Color Map"
                    GetString( MSG_MAPSUPRT_INCLUDEDEMELEVATIONDATAINCOLORMAP ),  // "Include DEM elevation data in Color Map?"
                    GetString( MSG_GLOBAL_YESNO ));                              // "Yes|No"
            Test_UM_Win((CONST_STRPTR)"DEMName",
                    GetString( MSG_LWSPRT_ERRORLOADINGDEMOBJECTPERATIONTERMINATED ),  // "Error loading DEM Object!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                       // "OK"
            if(ShouldBreak) break;
        }
        case 190:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DEMName",
                    GetString( MSG_LWSPRT_ERRLOADDEMOBJOBJNOTSAVED ),  // "Error loading DEM Object!\nObject not saved."
                    GetString( MSG_GLOBAL_OK ));                                  // "OK"
            if(ShouldBreak) break;
        }
        case 191:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWOBJECTEXPORT ),                 // "LW Object Export"
                    GetString( MSG_GLOBAL_OUTOFMEMORYOPERATIONTERMINATED ),  // "Out of memory!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                             // "OK"
            if(ShouldBreak) break;
        }
        case 192:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_APROBLEMOCCURREDSAVINGTHELWSCENEFAFILEWASCREATEDITWILLNOTBE ),  // "A problem occurred saving the LW scene.\nIf a file was created it will not be complete and may not load properly into LightWave."
                    GetString( MSG_GLOBAL_OK ));                                                           // "OK"
            if(ShouldBreak) break;
        }
        case 193:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LWSPRT_LWSCENEEXPORT ),                                                // "LW Scene Export"
                    GetString( MSG_LWSPRT_THEOUTPUTIMAGESIZEISNOTASTANDARDLIGHTWAVEIMAGESIZETHEZOOMFA ),  // "The output image size is not a standard LightWave image size. The zoom factor and image dimensions may not be portrayed correctly in the scene file just created."
                    GetString( MSG_GLOBAL_OK ));                                                           // "OK"
            if(ShouldBreak) break;
        }
        case 194:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_SUPPORT_PROJECTSAVE ),                          // "Project: Save"
                    GetString( MSG_SUPPORT_SAVEDATABASEANDPARAMETERFILESASWELL ),  // "Save Database and Parameter files as well?"
                    GetString( MSG_SUPPORT_BOTHDBASEPARAMSNO ));                    // "Both|D'base|Params|No",
            if(ShouldBreak) break;
        }
        case 195:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EVMORGUI_NEWPROJECT ),  // "New Project"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OK ));          // "OK"

            if(ShouldBreak) break;
        }
        case 196:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str,
                    GetString( MSG_DISPGUI_MAKETHISTHEDEFAULTOBJECTDIRECTORY ),  // "Make this the default object directory?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                           // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 197:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),  // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ));                                     // "Active|New|Cancel"
            if(ShouldBreak) break;
        }
        case 198:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_LINESPRT_SAVEOBJECTPOINTS ),  // "Save object points?"
                    GetString( MSG_GLOBAL_OKCANCEL ));          // "OK|CANCEL"
            if(ShouldBreak) break;
        }
        case 199:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str, GetString( MSG_LINESPRT_USEELEVATIONDATA ),  // "Use elevation data?"
                    GetString( MSG_GLOBAL_YESNO ));                                // "Yes|No"
            if(ShouldBreak) break;
        }
        case 200:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current flattening, datum and vertical exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ));                                               // "Yes|No"
            if(ShouldBreak) break;
        }
        case 201:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_LINESPRT_MAPPINGMODULEPATH ),                                   // "Mapping Module: Path"
                    GetString( MSG_LINESPRT_MODIFYALTITUDESWITHCURRENTFLATTENINGDATUMANDVERTIC ),  // "Modify altitudes with current Flattening, Datum and Vertical Exaggeration?"
                    GetString( MSG_GLOBAL_YESNO ));                                               // "Yes|No"
            if(ShouldBreak) break;
        }
        case 202:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_AGUI_DATABASEMODULE ),                                      // "Database Module"
                    GetString( MSG_LINESPRT_VECTORNAMEALREADYPRESENTINDATABASEVERWRITEITORTRYA ),  // "Vector name already present in Database!\nOverwrite it or try a new name?"
                    GetString( MSG_LINESPRT_OVERWRITENEWCANCEL ));                                  // "Overwrite|New|Cancel"
            if(ShouldBreak) break;
        }
        case 203:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),  // "Wave Editor"
                    GetString( MSG_WAVGUI_THECURRENTWAVEMODELHASBEENMODIFIEDDOYOUWISHTOSAVEITB ),  // "The current Wave Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_GLOBAL_YESNO ));  // "Yes|No"
            if(ShouldBreak) break;
        }
        case 204:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 205:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ) ,             // "Wave Editor"
                    GetString( MSG_WAVGUI_DELETEALLWAVEKEYFRAMES ),  // "Delete all wave key frames?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 206:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 207:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_WAVEEDITOR ),                      // "Wave Editor"
                    GetString( MSG_WAVGUI_MAKETHISFILETHEPROJECTWAVEFILE ),  // "Make this file the Project Wave File?"
                    GetString( MSG_GLOBAL_YESNO ));                           // "Yes|No"
            if(ShouldBreak) break;
        }
        case 208:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAVGUI_ADDWAVE ),                                               // "Add Wave"
                    GetString( MSG_WAVGUI_MAPVIEWMODULEMUSTBEOPENINORDEROUSETHISFUNCTIONWOULDY ),  // "Map View Module must be open in order\ to use this function. Would you like to open it now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                              // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 209:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_PARAMETERSMODULEMOTION ),                       // "Parameters Module: Motion"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_OKCANCEL ));                                     // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 210:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDMOGUI_PARAMETERSMODULEMAKEKEY ),  // "Parameters Module: Make Key"
                    (CONST_STRPTR)str,
                    GetString( MSG_GLOBAL_YESNO ));                    // "Yes|No"
            if(ShouldBreak) break;
        }
        case 211:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULESAVE ), (CONST_STRPTR)str,                   // "Database Module: Save"
                    GetString( MSG_DB_OKCANCEL ));                                                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 212:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_DB_DATABASEMODULENAME ),                            // "Database Module: Name"
                    GetString( MSG_DB_VECTORNAMEALREADYPRESENTINDATABASERYANEWNAME ),  // "Vector name already present in database!\nTry a new name?"
                    GetString( MSG_DB_OKCANCEL ));                                      // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 213:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)str,
                    GetString( MSG_DB_OBJECTNAMEALREADYPRESENTINDATABASEUPLICATEITEMSWILLBESKI ),  // "Object name already present in database!\nDuplicate items will be skipped."
                    GetString( MSG_GLOBAL_OK ));                                                        // "OK"
            if(ShouldBreak) break;
        }
        case 214:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_PARAMETEREDITINGDEFAULTS ) , (CONST_STRPTR)str, (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "Parameter Editing: Defaults", str, "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 215:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_PUBLICSCREENSTILLHASVISITORSTRYCLOSINGAGAIN ) ,  // "Public Screen still has visitors. Try closing again?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ));  // "Close|Warn|Cancel"
            if(ShouldBreak) break;
        }
        case 216:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"World Construction Set",
                    (CONST_STRPTR) GetString( MSG_AGUI_QUITPROGRAMREYOUSURE ) ,  // )"Quit Program\nAre you sure?"
                    (CONST_STRPTR) GetString( MSG_AGUI_CLOSEWARNCANCEL ));  // "Close|Warn|Cancel"
            if(ShouldBreak) break;
        }
        case 217:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_WCSPROJECT ) ,  // "WCS Project"
                    (CONST_STRPTR) GetString( MSG_AGUI_PROJECTPATHSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Project paths have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 218:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_PARAMETERMODULE ) ,  // "Parameter Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_PARAMETERSHAVEBEENMODIFIEDSAVETHEMBEFORECLOSING ) ,  // "Parameters have been modified. Save them before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 219:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR) GetString( MSG_AGUI_DATABASEMODULE ) ,  // "Database Module"
                    (CONST_STRPTR) GetString( MSG_AGUI_DATABASEHASBEENMODIFIEDSAVEITBEFORECLOSING ) ,  // "Database has been modified. Save it before closing?"
                    (CONST_STRPTR) GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 220:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"example-win", (CONST_STRPTR) GetString( MSG_AGUI_KEEPCHANGES ) , (CONST_STRPTR) GetString( MSG_AGUI_KEEPCANCEL ));  // "Keep changes?", "Keep|Cancel"
            if(ShouldBreak) break;
        }
        case 221:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"example existsfile",  GetString( MSG_AGUI_FILEALREADYEXISTSOYOUWISHTOOVERWRITEIT ) ,  // "File already exists.\nDo you wish to overwrite it?"
                    GetString( MSG_GLOBAL_OKCANCEL ));  // "OK|CANCEL"

            Test_UM_Win((CONST_STRPTR)"example rootfile",
                    GetString( MSG_DEM_DEMNAMEISTOOLONGTOADDANEXTRACHARACTERTODOYOUWISHTOENTER ) ,  // "DEM name is too long to add an extra character to. Do you wish to enter a new base name for the DEM or abort the interpolation?"
                    GetString( MSG_DEM_NEWNAMEABORT ));                                              // "New Name|Abort"
            if(ShouldBreak) break;
        }
        case 222:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEALIGN ),                             // "Mapping Module: Align"
                    GetString( MSG_MAP_ILLEGALVALUESHEREMUSTBEATLEASTONEPIXELOFFSETO ),  // "Illegal values!\nThere must be at least one pixel offset on both axes.\nTry again?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 223:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAP_MAPVIEWTOPODRAW ),                                // "Map View: Topo Draw"
                    GetString( MSG_MAP_MEMORYALLOCATIONFAILURECANNOTDRAWTOPOCONTINUE ),  // "Memory allocation failure, cannot draw topo. Continue?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                       // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 224:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name",
                    GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),  // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ));                   // "YES|NO"
            if(ShouldBreak) break;
        }
        case 225:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[i].Name", GetString( MSG_MAP_ISTHISTHECORRECTOBJECT ),   // "Is this the correct object?"
                    GetString( MSG_GLOBAL_YESNO ));                                                 // "YES|NO"
            if(ShouldBreak) break;
        }
        case 226:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAP_MAPVIEWMULTISELECT ),     // "Map View: Multi-Select"
                    GetString( MSG_MAP_SELECTORDESELECTITEMS ),  // "Select or de-select items?"
                    GetString( MSG_MAP_SELECTDESELECTCANCEL ));   // "Select|De-select|Cancel"
            if(ShouldBreak) break;
        }
        case 227:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_DIGITIZENEWPOINTSFORTHEACTIVEVECTOROBJECTORCR ),   // "Digitize new points for the active vector object or create a new object?"
                    GetString( MSG_GLOBAL_ACTIVENEWCANCEL ));  // "Active|New|Cancel"
            if(ShouldBreak) break;
        }
        case 228:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),   // "Mapping Module: Digitize"
                    GetString( MSG_MAP_ACCEPTNEWPOINTS ),         // "Accept new points?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 229:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 230:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_MAPGUI_MAPPINGMODULEDIGITIZE ),      // "Mapping Module: Digitize"
                    GetString( MSG_MAP_CONFORMVECTORTOTERRAINNOW ),  // "Conform vector to terrain now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                   // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 231:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win((CONST_STRPTR)"DBase[OBN].Name",
                    GetString( MSG_MAP_CREATEVISUALSENSITIVITYMAPFORTHISOBJECT ),  // "Create Visual Sensitivity map for this object?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                 // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 232:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITOR ),  // "Foliage Editor"
                    GetString( MSG_AGUI_KEEPCHANGES ),    // "Keep changes?"
                    GetString( MSG_GLOBAL_YESNO ));          // "Yes|No"
            if(ShouldBreak) break;
        }
        case 233:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_UNABLETOLOADIMAGEFILEFORVIEWINGPERATIONTERMINATED ),   // "Unable to load image file for viewing!\nOperation terminated.",
                    GetString( MSG_GLOBAL_OK ));                                                  // "OK"
            if(ShouldBreak) break;
        }
        case 234:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORLOADECOTYPE ),                   // "Foliage Editor: Load Ecotype"
                    GetString( MSG_FOLIGUI_ERRORLOADINGECOTYPEFILEPERATIONTERMINATED ),  // "Error loading Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                         // "OK"
            if(ShouldBreak) break;
        }
        case 235:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                            // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                               // "OK"
            if(ShouldBreak) break;
        }
        case 236:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDGROUP ),                           // "Foliage Editor: Add Group"
                    GetString( MSG_FOLIGUI_ERRORLOADINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error loading Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                              // "OK"
            if(ShouldBreak) break;
        }
        case 237:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                               // "OK"
            if(ShouldBreak) break;
        }
        case 238:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORNEWGROUP ),                            // "Foliage Editor: New Group"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ),  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                              // "OK"
            if(ShouldBreak) break;
        }
        case 239:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEGROUP ),                         // "Foliage Editor: Save Group"
                    GetString( MSG_FOLIGUI_ERRORSAVINGFOLIAGEGROUPFILEPERATIONTERMINATED ),  // "Error saving Foliage Group file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                             // "OK"
            if(ShouldBreak) break;
        }
        case 240:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                             // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_OUTOFMEMORYALLOCATINGNEWGROUPPERATIONTERMINATED ) ,  // "Out of memory allocating new group!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                                // "OK"
            if(ShouldBreak) break;
        }
        case 241:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORADDIMAGE ),                    // "Foliage Editor: Add Image"
                    GetString( MSG_FOLIGUI_ERRORLOADINGIMAGEFILEPERATIONTERMINATED ),  // "Error loading image file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                       // "OK"
            if(ShouldBreak) break;
        }
        case 242:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORVIEWIMAGE ),                              // "Foliage Editor: View Image"
                    GetString( MSG_FOLIGUI_THEIMAGELOADEDPROPERLYMAYBESOMEDAYTHEREWILLEVENBEAW ),  // "The image loaded properly. Maybe some day there will even be a way for you to see it!\n"
                    GetString( MSG_FOLIGUI_THATWOULDBENICE ));               // "That would be nice"
            if(ShouldBreak) break;
        }
        case 243:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_FOLIGUI_FOLIAGEEDITORSAVEECOTYPE ),                  // "Foliage Editor: Save Ecotype"
                    GetString( MSG_FOLIGUI_ERRORSAVINGECOTYPEFILEPERATIONTERMINATED ),  // "Error saving Ecotype file!\nOperation terminated."
                    GetString( MSG_GLOBAL_OK ));                                        // "OK"
            if(ShouldBreak) break;
        }
        case 244:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THISISANOLDV1FORMATFILEWOULDYOULIKETORESAVEITINTHENEW ),  // "This is an old V1 format file! Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 245:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),                                    // "Parameter Module: Load"
                    GetString( MSG_EDPAR_THEPARAMETERFILEFORMATHASBEENCHANGEDSLIGHTLYSINCETHIS ),  // "The Parameter File format has been changed slightly since this file was saved. Would you like to re-save it in the new format now?"
                    GetString( MSG_GLOBAL_OKCANCEL ));                                               // "OK|Cancel"
            if(ShouldBreak) break;
        }
        case 246:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     //  "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO));                 // "Yes|No"
            if(ShouldBreak) break;
        }
        case 247:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_EDPAR_PARAMETERMODULELOAD ),  // "Parameter Module: Load"
                    GetString( MSG_EDPAR_LOADALLKEYFRAMES ),     // "Load all key frames?"
                    GetString( MSG_GLOBAL_YESNO ));                // "Yes|No"

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_THECURRENTECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_1 ),  // "The current Ecosystem Model has been modified. Do you wish to save it before closing?"
                    GetString( MSG_PARGUI_YESNOCANCEL ));                                           // "Yes|No|Cancel"
            if(ShouldBreak) break;
        }
        case 248:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_PARGUI_PARAMETERSMODULEMODEL ),                                 // "Parameters Module: Model"
                    GetString( MSG_PARGUI_CURRECOSYSTEMMODELHASBEENMODIFIEDDOYOUWISHTO_2 ),  //" The current Ecosystem Model has been modified. Do you wish to save it before proceeding?"
                    GetString( MSG_PARGUI_YESNOCANCEL ));                                           // "Yes|No|Cancel"
            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),          // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTGENERALWAVECENTER ),  // "Select general wave center."
                    GetString( MSG_WAV_FOCUSPOINTCAMERAPOINT ));    // "Focus Point|Camera Point"
            if(ShouldBreak) break;
        }
        case 249:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Wave: Set Defaults"
                    GetString( MSG_WAV_WAVESETDEFAULTS ),   // "Select wave speed."
                    GetString( MSG_WAV_FASTVERYFASTSLOW ));  //"Fast|Very Fast|Slow"
            if(ShouldBreak) break;
        }
        case 250:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_UM_Win(GetString( MSG_WAV_WAVESETDEFAULTS ),      // "Wave: Set Defaults"
                    GetString( MSG_WAV_SELECTWAVEDIRECTION ),  // "Select wave direction."
                    GetString( MSG_WAV_SPREADINGCONVERGING ));  // "Spreading|Converging"

            //####### Input String Window ############

            if(ShouldBreak) break;
        }
        case 251:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPEXTRA_ENTERNUMBEROFOUTPUTVERTICES ));  // "Enter number of output vertices."

            if(ShouldBreak) break;
        }
        case 252:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPEXTRA_ENTERMINIMUMMATCHINGPOINTS ));  // "Enter minimum matching points."

            if(ShouldBreak) break;
        }
        case 253:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPEXTRA_ENTERELEVATIONTOLERANCE ));  // "Enter elevation tolerance."

            if(ShouldBreak) break;
        }
        case 254:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 255:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DB_ENTERNEWOBJECTNAME ));  // "Enter new object name."

            if(ShouldBreak) break;
        }
        case 256:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDDB_ENTERSEARCHSTRING ));  // "Enter search string."

            if(ShouldBreak) break;
        }
        case 257:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_CLOUD_ENTERFRAMENUMBER ));  // "Enter Frame Number."

            if(ShouldBreak) break;
        }
        case 258:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPTOPOOB_ENTERTHEMAXIMUMPIXELSIZEFORAPOLYGONTHESMALLERTHEN ));  // "Enter the maximum pixel size for a polygon. The smaller the number the longer image rendering will take!"

            if(ShouldBreak) break;
        }
        case 259:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPTOPOOB_ENTERTHEFIRSTFRAMETOSCAN ));  // "Enter the first frame to scan."

            if(ShouldBreak) break;
        }
        case 260:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPTOPOOB_ENTERTHELASTFRAMETOSCAN ));  // "Enter the last frame to scan."

            if(ShouldBreak) break;
        }
        case 261:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPTOPOOB_ENTERTHEFRAMEINTERVALTOSCANTHESMALLERTHENUMBERTHE ));  // "Enter the frame interval to scan. The smaller the number the longer this process will take!"

            if(ShouldBreak) break;
        }
        case 262:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 263:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 264:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DEMGUI_ENTERTHEUTMZONENUMBER060FORTHEDATAYOUAREABOUTTOIMPOR ));  // "Enter the UTM zone number (0-60) for the data you are about to import."

            if(ShouldBreak) break;
        }
        case 265:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 266:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAPGUI_ENTERELEVATIONVALUEFORNEWCONTROLPOINT ));  // "Enter elevation value for new control point."

            if(ShouldBreak) break;
        }
        case 267:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_LINESPRT_ENTERFRAMEINTERVALTOREPRESENTEACHVECTORSEGMENT ));  // "Enter frame interval to represent each vector segment."
            if(ShouldBreak) break;
        }
        case 268:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_LINESPRT_ENTERNAMEOFVECTORTOBECREATED ));  // "Enter name of vector to be created."

            if(ShouldBreak) break;
        }
        case 269:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 270:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_WAVGUI_ENTERWAVEAMPLITUDE ));  // "Enter Wave Amplitude."

            if(ShouldBreak) break;
        }
        case 271:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_WAVGUI_ENTERWAVELENGTHKM ));  // "Enter Wave Length (km)."

            if(ShouldBreak) break;
        }
        case 272:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_WAVGUI_ENTERWAVEVELOCITYKMHR ));  // "Enter Wave Velocity (km/hr)."

            if(ShouldBreak) break;
        }
        case 273:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DLG_ENTERUPTO3CHARACTERSASAPREFIXFORTHISDLGSETIFYOUDESIRE ));  // ,"Enter up to 3 characters as a prefix for this DLG set if you desire."

            if(ShouldBreak) break;
        }
        case 274:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DLG_ANENTITYHASBEENFOUNDWITHNONAMEIDENTIFIERPLEASEENTERADEF ));  // "An entity has been found with no name identifier. Please enter a default name."

            if(ShouldBreak) break;
        }
        case 275:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDITGUI_ENTERFRAMETOMAKEKEYFOR ));  // "Enter frame to make key for."

            if(ShouldBreak) break;
        }
        case 276:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DB_ENTERNEWOBJECTNAME ));  // "Enter new object name."

            if(ShouldBreak) break;
        }
        case 277:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DB_ENTERNEWOBJECTNAME ));  // "Enter new object name."

            if(ShouldBreak) break;
        }
        case 278:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_DEM_ENTERANAMEFORTHE30METERDEMOBJECT ));  // "Enter a name for the 30 meter DEM object."

            if(ShouldBreak) break;
        }
        case 279:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_MAP_ENTERVERTICALOFFSETINMETERS ));  // "Enter vertical offset in meters."

            if(ShouldBreak) break;
        }
        case 280:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_FOLIGUI_ENTERNEWGROUPNAME ));  // "Enter new group name."

            if(ShouldBreak) break;
        }
        case 281:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_FOLIGUI_ENTERNEWIMAGEPATHANDNAME ));  // "Enter new image path and name."

            if(ShouldBreak) break;
        }
        case 282:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Test_IS_Win(GetString( MSG_EDPAR_ENTERKEYFRAMEINTERVALORKFORCURRENTKEYFRAMES ));  // "Enter Key Frame interval or 'K' for current Key Frames."
            if(ShouldBreak) break;
        }
        // ###################################


        case 283:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            printf("Line %d\n", __LINE__);
            Make_Log_Window(128); // Log Window, 128, if >127, stays open
            waitForRightClick(Log_Win->LogWindow); // Wait for right mouse button to be pressed
            Close_Log_Window(2);  // Close Log Window, 2 = Shutdown, kill it all! */
            if(ShouldBreak) break;
        }



        case 284:
            // ####### CloudGui.c ##############
        {
            Make_CL_Window();   // ALEXANDER: Auch im normalen Test aufrufen!
            waitForRightClick(CL_Win->CloudWin); // Wait for right mouse button to be pressed
            Close_CL_Window();  // Close Cloud Window
            if(ShouldBreak) break;
        }


        case 285:
            // ####### DEMGui.c ##############
        {
            Make_MD_Window();   // Under Construction Window is opened automatically, too
            waitForRightClick(MD_Win->MakeDEMWin); // Wait for right mouse button to be pressed
            Close_MD_Window();  // Close DEM Window
            UnderConst_Del(); // Delete the Under Construction Window, too
            if(ShouldBreak) break;
        }

        case 286:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_GR_Window();
            waitForRightClick(GR_Win->NNGridWin); // Wait for right mouse button to be pressed
            Close_GR_Window();  // Close Grid Window
            if(ShouldBreak) break;
        }

        case 287:
            // ####### DataOpsGui.c ##############
        {
            Make_DI_Window();  // Make Dialog Window
            // eigener Message-Handler, OK klicken!
            if(ShouldBreak) break;
        }

        case 288:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DC_Window();
            waitForRightClick(DC_Win->ConvertWin); // Wait for right mouse button to be pressed
            Close_DC_Window();  // Close Convert Window
            if(ShouldBreak) break;
        }

        case 289:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DC_Window();
            set(DC_Win->IPRegister, MUIA_Group_ActivePage, 1);  // Input Page "Preprocessing"
            set(DC_Win->PGRegister, MUIA_Group_ActivePage, 1);  // Output Format "Value Format"
            waitForRightClick(DC_Win->ConvertWin); // Wait for right mouse button to be pressed
            Close_DC_Window();  // Close Convert Window
            if(ShouldBreak) break;
        }

        case 290:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DC_Window();
            set(DC_Win->IPRegister, MUIA_Group_ActivePage, 1);  // Input Page "Preprocessing"
            set(DC_Win->PGRegister, MUIA_Group_ActivePage, 1);  // Output Format "Value Format"
            set(DC_Win->VSRegister, MUIA_Group_ActivePage, 1);  // Output Format "One Value"
            waitForRightClick(DC_Win->ConvertWin); // Wait for right mouse button to be pressed
            Close_DC_Window();  // Close Convert Window
            if(ShouldBreak) break;
        }

        case 291:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DC_Window();
            set(DC_Win->IPRegister, MUIA_Group_ActivePage, 1);  // Input Page "Preprocessing"
            set(DC_Win->PGRegister, MUIA_Group_ActivePage, 1);  // Output Format "Value Format"
            set(DC_Win->VSRegister, MUIA_Group_ActivePage, 2);  // Output Format "Max-Min"
            waitForRightClick(DC_Win->ConvertWin); // Wait for right mouse button to be pressed
            Close_DC_Window();  // Close Convert Window
            if(ShouldBreak) break;
        }

        case 292:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            DIAG_Win=NULL;
            Open_Diagnostic_Window(NULL, "ExampleWinTitle");
            waitForRightClick(DIAG_Win->DiagnosticWin); // Wait for right mouse button to be pressed
            Close_Diagnostic_Window();  // Close Diagnostic Window
            if(ShouldBreak) break;
        }

        // ###### EdBaseGui.c ##############
        case 293:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DL_Window();  // Make DirList Window
            waitForRightClick(DL_Win->DirListWin); // Wait for right mouse button to be pressed
            Close_DL_Window(DL_Win->DLCopy);  // Close Dialog Window
            if(ShouldBreak) break;
        }



        case 294:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DE_Window();
            waitForRightClick(DE_Win->DatabaseEditWin); // Wait for right mouse button to be pressed
            Close_DE_Window();  // Close DatabaseEditWin
            if(ShouldBreak) break;
        }

        // ############ EdEcoGui.c ##############
        case 295:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            EE_Win=NULL;
            Make_EE_Window();
            waitForRightClick(EE_Win->EcosystemWin); // Wait for right mouse button to be pressed
            Close_EE_Window(0);  // Close Edit Ecosystem Window
            if(ShouldBreak) break;
        }


        // ############ EdEcoGui.c ##############
        case 296:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            AN_Win=NULL;  // Close_EMIA_Window() crashes if EMIA_Win is not NULL
            KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            Make_EMIA_Window();  // Make Edit Model Image Attributes Window
            waitForRightClick(EMIA_Win->IAMotionWin); // Wait for right mouse button to be pressed
            Close_EMIA_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 297:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // show 2nd Tab
            AN_Win=NULL;  // Close_EMIA_Window() crashes if EMIA_Win is not NULL
            KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            Make_EMIA_Window();  // Make Edit Model Image Attributes Window
            set(EMIA_Win->Register_Cycle_Page, MUIA_Group_ActivePage, 2);
            waitForRightClick(EMIA_Win->IAMotionWin); // Wait for right mouse button to be pressed
            Close_EMIA_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 298:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // this window ist empty in this test ?!
            Make_EMPL_Window();  // Make Edit Motion Parameter List Window
            waitForRightClick(EMPL_Win->ParListWin); // Wait for right mouse button to be pressed
            Close_EMPL_Window();  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 299:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_EM_Window();  // Make Edit Motion Window
            waitForRightClick(EM_Win->MotionWin ); // Wait for right mouse button to be pressed
            Close_EMPL_Window();  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        // ############ EdSetGUI.c ##############
        case 300:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 301:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Image Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 1);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 302:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Motion Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 2);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 303:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Color Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 3);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 304:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Surfaces Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 4);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 305:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Fractal Tab Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 5);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 306:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // EcoSys Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 6);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        case 307:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            // Misc Tab
            memset(&settings, 0, sizeof(settings)); // Clear settings structure
            settings.scrnwidth = 640;  // Set a valid screen width
            settings.scrnheight = 480; // Set a valid screen height
            settings.overscan = 1;
            settings.rendersegs=1; // Set a valid number of render segments

            Make_ES_Window();
            set(ES_Win->Pages , MUIA_Group_ActivePage, 7);
            waitForRightClick(ES_Win->SettingsWin); // Wait for right mouse button to be pressed
            Close_ES_Window(1);  // Close Edit Model Image Attributes Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        // ############ EdSetGUI.c ##############
        case 308:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            Make_EC_Window(); // Make Edit Color Window
            waitForRightClick(EC_Win->EcoPalWin); // Wait for right mouse button to be pressed
            Close_EC_Window(1); // Close Edit Color Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        // ############ EvenMoreGUI.c ###########
        case 309:
            Make_PN_Window(); // Project New Window
            waitForRightClick(PN_Win->NewProjWin); // Wait for right mouse button to be pressed
            Close_PN_Window(1);

        case 310:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            Make_TS_Window();
            waitForRightClick(TS_Win->TimeSetWin); // Wait for right mouse button to be pressed
            Close_TS_Window(1);  // Close Time Set Window, 1= Shutdown, kill it all!
            if(ShouldBreak) break;
        }

        // ####### FoliageGUI.c ##############
        case 311:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_FE_Window();
            waitForRightClick(FE_Win->FoliageWin); // Wait for right mouse button to be pressed
            Close_FE_Window(1);
            if(ShouldBreak) break;
        }


        case 312:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            //               // Dies hier funktioniert gerade so... nicht aendern!
            //               static const char *Titles[8];
            //               struct TimeLineWindow *TL=NULL;
            //               APTR FloatStr[7];
            //               struct WindowKeyStuff WKS;
            //               struct GUIKeyStuff GKS;
            //               union KeyFrame * CloudKey=NULL;
            //               long KFsize = (/*ParHdr.KeyFrames*/0 + 20) * (sizeof (union KeyFrame));  // we need a valid size
            //               short NumKeys=0;
            //               double Coverage=0;
            //
            //                WKS.Group = 3;
            //                WKS.Item = 0;
            //                WKS.NumValues = 7;
            //                WKS.Precision = WCS_KFPRECISION_FLOAT;
            //
            //
            //               memset(&GKS, 0, sizeof(GKS)); // Clear GUIKeyStuff structure
            //
            //               Titles[0] = (const char*)GetString( MSG_CLOUDGUI_COVERAGE );          // "Coverage"
            //               Titles[1] = (const char*)GetString( MSG_CLOUDGUI_DENSITY );           // "Density"
            //               Titles[2] = (const char*)GetString( MSG_CLOUDGUI_ROUGHNESS );         // "Roughness"
            //               Titles[3] = (const char*)GetString( MSG_CLOUDGUI_FRACTALDIMENSION );  // "Fractal Dimension"
            //               Titles[4] = (const char*)GetString( MSG_CLOUDGUI_ALTITUDE );          // "Altitude"
            //               Titles[5] = (const char*)GetString( MSG_CLOUDGUI_MOVELATITUDE );      // "Move Latitude"
            //               Titles[6] = (const char*)GetString( MSG_CLOUDGUI_MOVELONGITUDE );     // "Move Longitude"
            //               Titles[7] = NULL;
            //
            //               Make_TL_Window((char*)GetString( MSG_CLOUDGUI_CLOUDTIMELINES ),
            //                              (char **)Titles,
            //                              &TL,
            //                              FloatStr,
            //                              &WKS,
            //                              &GKS,
            //                              &CloudKey,
            //                              &KFsize,
            //                              &NumKeys,
            //                              &Coverage,
            //                              NULL, NULL);
            //
            //               // All Register pages look identically, no need to show them all
            //
            //               waitForRightClick(TL->TimeLineWin); // Wait for right mouse button to be pressed
            ////               Close_TL_Window(&TL, 0);  // Close Time Line Window, 1= Shutdown, kill it all! - It is closed automatically at the end of the program, so don't free twice!
            if(ShouldBreak) break;
        }

        // ################### MapGui.c ####################
        case 313:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_EL_Window();    // Window mit leeren Cyclebuttons
            waitForRightClick(EL_Win->EcoLegendWin); // Wait for right mouse button to be pressed
            Close_EL_Window();
            if(ShouldBreak) break;
        }

        case 314:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct MapData MP;
            MapGUI_New(&MP);
            waitForRightClick(MP.MAPC); // Wait for right mouse button to be pressed
            MapGUI_Del(&MP);

            if(ShouldBreak) break;
        }

        case 315:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct MapData MP;
            Make_MA_Window(&MP);
            waitForRightClick(MP.AlignWin); // Wait for right mouse button to be pressed
            Close_MA_Window(&MP);  // Close Map Align Window

            if(ShouldBreak) break;
        }

        case 316:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            UnderConst_New();
            waitForRightClick(UnderConst); // Wait for right mouse button to be pressed
            UnderConst_Del();
            if(ShouldBreak) break;
        }


        // ############ MoreGUI.c ##############
        case 317:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_SC_Window();
            waitForRightClick(SC_Win->ScaleWin); // Wait for right mouse button to be pressed
            Close_SC_Window();
            if(ShouldBreak) break;
        }

        case 318:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_PR_Window();
            waitForRightClick(PR_Win->PrefsWin); // Wait for right mouse button to be pressed
            Close_PR_Window();
            if(ShouldBreak) break;
        }

        case 319:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_DM_Window();
            waitForRightClick(DM_Win->ExtractWin); // Wait for right mouse button to be pressed
            Close_DM_Window();
            if(ShouldBreak) break;
        }

        case 320:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_PJ_Window();  // Page 1
            waitForRightClick(PJ_Win->ProjWin); // Wait for right mouse button to be pressed
            Close_PJ_Window(1);
            if(ShouldBreak) break;
        }

        case 321:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_PJ_Window();  // Page 2
            set(PJ_Win->RegGrp, MUIA_Group_ActivePage, 1);  // Set to Page 2
            waitForRightClick(PJ_Win->ProjWin); // Wait for right mouse button to be pressed
            Close_PJ_Window(1);
            if(ShouldBreak) break;
        }

        // ################### ParamsGui.c ####################

        case 322:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct ScaleWindow PS_Win= {0};    // Param ist leer ??
            PS_Win.ScaleWin=AF_MakeScaleWinObject(&PS_Win);
            if(PS_Win.ScaleWin == NULL)
            {
                printf("AF_MakeScaleWinObject() failed!\n");
                return; // Exit if the Scale Window could not be created
            }
            DoMethod(app, OM_ADDMEMBER, PS_Win.ScaleWin);
            set(PS_Win.ScaleWin,MUIA_Window_Open, TRUE);
            waitForRightClick(PS_Win.ScaleWin); // Wait for right mouse button to be pressed
            set(PS_Win.ScaleWin,MUIA_Window_Open, FALSE);
            if(ShouldBreak) break;
        }

        case 323:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_AN_Window();
            waitForRightClick(AN_Win->AnimWin); // Wait for right mouse button to be pressed
            Close_AN_Window();
            if(ShouldBreak) break;
        }

        case 324:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_LW_Window();
            waitForRightClick(LW_Win->IOWin); // Wait for right mouse button to be pressed
            Close_LW_Window();
            if(ShouldBreak) break;
        }

        case 325:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            Make_FM_Window();
            waitForRightClick(FM_Win->ModelWin); // Wait for right mouse button to be pressed
            Close_FM_Window();
            if(ShouldBreak) break;
        }
        // ####################### Requester_GUI.c #######################
        case 326:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct BusyWindow *BW;
            BW=BusyWin_New((char*)GetString( MSG_NNCRUNCH_CHOROPLETH ), 10, 60, MakeID('B','W','G','R'));  // Busy Window
            waitForRightClick(BW->BusyWin); // Wait for right mouse button to be pressed
            BusyWin_Del(BW);
            if(ShouldBreak) break;
        }


        // ############ ScreenModeGUI.c ##############
        case 327:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);


            struct WCSScreenMode *ScreenModes = ModeList_New();
            struct WCSScreenData *ScrnData={0};
            ModeList_Choose(ScreenModes,ScrnData);
            ModeList_Del(ScreenModes);
            if(ShouldBreak) break;
        }


        // ############ TimeLinesGUI.c ##############
        case 328:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct TimeLineWindow EETL_Win={0};
            APTR TimeLineWin=NULL;

            struct Data
            {
                    short x, y, sx, sy;
                    short left, right, top, bottom, textbottom, textzero, lowframe, highframe,
                    textwidthtop, textwidthbottom, textwidthzero, framegrid,
                    framegridfirst, framegridlg, drawgrid;
                    float texthighval, textlowval, valgrid, valgridfirst, framepixgrid,
                    valpixgrid, valpixpt, framepixpt;
                    struct KeyTable *SKT;
                    short group, activekey, activeitem, dataitems, baseitem;
                    long inputflags;
                    struct TimeLineWindow *win;
            };


            if ( ! (EETL_Win.SuperClass = MUI_GetClass(MUIC_Area)))
            {
                printf("MUI_GetClass(MUIC_Area) failed!\n");
                return;
            }

            /* create the new class */
            if (!(EETL_Win.TL_Class =
                    MakeClass(NULL, NULL, EETL_Win.SuperClass, sizeof(struct Data), 0)))
            {
                printf("MakeClass() failed!\n");
                return;
            }
            /* set the dispatcher for the new class */
            EETL_Win.TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
            EETL_Win.TL_Class->cl_Dispatcher.h_SubEntry = NULL;
            EETL_Win.TL_Class->cl_Dispatcher.h_Data     = NULL;


            TimeLineWin=AF_MakeEETLWindow(&EETL_Win);
            if(TimeLineWin == NULL)
            {
                printf("AF_MakeEETLWindow() failed!\n");
                return; // Exit if the Time Line Window could not be created
            }
            DoMethod(app, OM_ADDMEMBER, TimeLineWin);
            set(TimeLineWin,MUIA_Window_Open, TRUE);
            waitForRightClick(TimeLineWin); // Wait for right mouse button to be pressed
            set(TimeLineWin,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, TimeLineWin);
            MUI_FreeClass(EETL_Win.SuperClass);
            MUI_DisposeObject(TimeLineWin);
            if(ShouldBreak) break;
        }

        case 329:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct TimeLineWindow ECTL_Win={0};
            APTR Window=NULL;

            if ( ! (ECTL_Win.SuperClass = MUI_GetClass(MUIC_Area)))
            {
                printf("MUI_GetClass(MUIC_Area) failed!\n");
                return;
            }

            /* create the new class */
            if (!(ECTL_Win.TL_Class =
                    MakeClass(NULL, NULL, ECTL_Win.SuperClass, sizeof(struct Data), 0)))
            {
                printf("MakeClass() failed!\n");
                return;
            }
            /* set the dispatcher for the new class */
            ECTL_Win.TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
            ECTL_Win.TL_Class->cl_Dispatcher.h_SubEntry = NULL;
            ECTL_Win.TL_Class->cl_Dispatcher.h_Data     = NULL;



            Window=AF_MakeECTLWindow(&ECTL_Win);  // Make Ecosystem Time Line Window
            if(Window == NULL)
            {
                printf("AF_MakeECTL_Window() failed!\n");
                return; // Exit if the Ecosystem Time Line Window could not be created
            }
            DoMethod(app, OM_ADDMEMBER, Window);
            set(Window,MUIA_Window_Open, TRUE);
            waitForRightClick(Window); // Wait for right mouse button to be pressed
            set(Window,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, Window);
            MUI_FreeClass(ECTL_Win.SuperClass);
            MUI_DisposeObject(Window);
            if(ShouldBreak) break;
        }

        case 330:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct TimeLineWindow EMTL_Win={0};
            APTR Window=NULL;

            if ( ! (EMTL_Win.SuperClass = MUI_GetClass(MUIC_Area)))
            {
                printf("MUI_GetClass(MUIC_Area) failed!\n");
                return;
            }

            /* create the new class */
            if (!(EMTL_Win.TL_Class =
                    MakeClass(NULL, NULL, EMTL_Win.SuperClass, sizeof(struct Data), 0)))
            {
                printf("MakeClass() failed!\n");
                return;
            }
            /* set the dispatcher for the new class */
            EMTL_Win.TL_Class->cl_Dispatcher.h_Entry    = (APTR)TL_Dispatcher;
            EMTL_Win.TL_Class->cl_Dispatcher.h_SubEntry = NULL;
            EMTL_Win.TL_Class->cl_Dispatcher.h_Data     = NULL;



            Window=AF_MakeEMTLWindow(&EMTL_Win);  // Make Ecosystem Motion Time Line Window
            if(Window == NULL)
            {
                printf("AF_MakeEMTL_Window() failed!\n");
                return; // Exit if the Ecosystem Motion Time Line Window could not be created
            }
            DoMethod(app, OM_ADDMEMBER, Window);
            set(Window,MUIA_Window_Open, TRUE);
            waitForRightClick(Window); // Wait for right mouse button to be pressed
            set(Window,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, Window);
            MUI_FreeClass(EMTL_Win.SuperClass);
            MUI_DisposeObject(Window);
            if(ShouldBreak) break;
        }

        // ############ WaveGUI.c ##############
        case 331:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct WaveWindow WV_Win={0};
            short WinNum=0; // 0 = Wave Edit Window, 1 = Wave Control Window
            char *NameStr=(char*)GetString( MSG_WAVGUI_WAVEEDITOR ); // "Wave Editor"
            ULONG WinID = WinNum == 0 ? MakeID('W','V','E','D'): MakeID('W','V','E','C');

            APTR Window=NULL;

            Window=AF_MakeWV_Win(&WV_Win, WinNum, NameStr, WinID);  // Make Wave Window
            if(Window == NULL)
            {
                printf("AF_MakeWV_Win() failed!\n");
                return; // Exit if the Wave Window could not be created
            }

            DoMethod(app, OM_ADDMEMBER, Window);
            set(Window,MUIA_Window_Open, TRUE);
            waitForRightClick(Window); // Wait for right mouse button to be pressed
            set(Window,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, Window);
            MUI_DisposeObject(Window);

            if(ShouldBreak) break;
        }

        case 332:
        {
            ShowTestNumbers(StartTestNumber++,TotalTests);

            struct WaveWindow WV_Win={0};
            short WinNum=1; // 0 = Wave Edit Window, 1 = Wave Control Window
            char *NameStr=(char*)GetString( MSG_CLOUDGUI_CLOUDWAVEEDITOR ); // "Cloud Wave Editor"
            ULONG WinID = WinNum == 0 ? MakeID('W','V','E','D'): MakeID('W','V','E','C');

            APTR Window=NULL;

            Window=AF_MakeWV_Win(&WV_Win, WinNum, NameStr, WinID);  // Make Wave Window
            if(Window == NULL)
            {
                printf("AF_MakeWV_Win() failed!\n");
                return; // Exit if the Wave Window could not be created
            }

            DoMethod(app, OM_ADDMEMBER, Window);
            set(Window,MUIA_Window_Open, TRUE);
            waitForRightClick(Window); // Wait for right mouse button to be pressed
            set(Window,MUIA_Window_Open, FALSE);
            DoMethod(app, OM_REMMEMBER, Window);
            MUI_DisposeObject(Window);

            if(ShouldBreak) break;
        }
    } // switch (TestNumber)
}
#endif
