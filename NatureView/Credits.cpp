// Credits.cpp
// Required and recommended credits
// Created from scratch on 2/17/05 by CXH

#include "Credits.h"
#include "HTMLDlg.h"
#include "NVMiscGlobals.h"
#include "InternalImage.h" // to be able to export and clean up NV logo

// Pick up externally-compiled logo image
// believe it or not, this extern "C" is required...
extern "C" {
#define NVELOGO256X128_PNG_RAWDATA_SIZE		5692
extern unsigned char NVELogo256x128_png_rawData[NVELOGO256X128_PNG_RAWDATA_SIZE];
} // extern "C"


// Used for Notice UI, which uses same framework (HTMLDlg) as Credits
bool NoticeDisplayed = false;

void SetNoticeDisplayed(bool NewState)
{
NoticeDisplayed = NewState;
} // SetNoticeDisplayed
 
bool GetNoticeDisplayed(void)
{
return(NoticeDisplayed);
} // GetNoticeDisplayed


void PrepCreditsImage(char *TempFilePath)
{
ExportImage("NVELogo256x128.png", NVELogo256x128_png_rawData, NVELOGO256X128_PNG_RAWDATA_SIZE, TempFilePath);
} // PrepCreditsImage

int DisplayCredits(void)
{
int Success = 0;
char TempFilePath[1024], TempHTMLText[4096];

PrepCreditsImage(TempFilePath);

HTMLDlg::SetCenterOnOpen(true);

HTMLDlg::SetForceTitle("About");

// this ugly BGCOLOR hack is because IE apparently ignores the PNG transparency, and shows the
// underlying color (which is sort of light blue). We just make the whole BG light blue so that
// the image BG isn't visible. Browsers that aren't stupid, will show the BG blue through the
// PNG, and it will look the same.

sprintf(TempHTMLText, "<HTML><HEAD></HEAD><BODY BGCOLOR=\"#D4E6EA\"><center>\
<img src=\"%s\"><br>\
Version "NVW_VIEWER_VERSIONTEXT"<br>"NVW_NATUREVIEW_COPYRIGHTTEXT_HTML"<br><br>\
Conceived and implemented by Chris &quot;Xenon&quot; Hanson\
<br><br>\
"NVW_COMPANY_NAMETEXT_HTML" gratefully uses and acknowledges contributions from the following people and projects:<br>\
<a href=\"http://www.openscenegraph.com/\">OpenSceneGraph.org</a><br>\
<a href=\"http://www.freetype.org/\">The Freetype Project</a><br>\
<a href=\"http://www.ijg.org/\">Independent JPEG Group</a><br>\
<a href=\"http://www.libpng.org/pub/png/\">PNG Group</a><br>\
<a href=\"http://www.gzip.org/zlib/\">ZLIB Group</a><br>\
<br>\
Bob Maple (bobm@retardedlogic.com) -- &quot;Alabaster&quot; UI Skin<br>\
Paul Martz -- OSG static linking<br>\
Jeff Glatt (@borg.com) -- Embedded Web Browser<br>\
Farshid Lashkari -- Elumens SPICLOPS assistance<br>\
University of East Anglia Norwich -- Elumens sponsorship<br>\
</center></BODY></HTML>", TempFilePath);

Success = HTMLDlg::SetHTMLText(TempHTMLText);

return(Success);
} // DisplayCredits

void CleanupCredits(void)
{
CleanupExportedImage("NVELogo256x128.png");
} // CleanupCredits