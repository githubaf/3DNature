// AppHelp.h
// Header file for OnlineHelp support.
// Created from Scratch on 1/10/98 by Chris 'Xenon' Hanson

#ifndef WCS_APPHELP_H
#define WCS_APPHELP_H

#include "PathAndFile.h"
#include "WCSVersion.h"

// like VNS1REF or WCS6REF
#define WCS_APPHELP_FILEPREFIX APP_TLA APP_VERS "ref"

#ifdef WCS_BUILD_DEMO
#define WCS_APPHELP_DEFAULTBASE	"wcs5demo"
#else // !WCS_BUILD_DEMO
#ifdef WCS_BUILD_VNS
#define WCS_APPHELP_DEFAULTBASE	WCS_APPHELP_FILEPREFIX
#else // !VNS
#define WCS_APPHELP_DEFAULTBASE	WCS_APPHELP_FILEPREFIX
#endif // !VNS
#endif // !WCS_BUILD_DEMO

#define WCS_APPHELP_HTMLSUFFIX	".html"
#define WCS_APPHELP_PDFSUFFIX	".pdf"
#define WCS_APPHELP_PDFDESTPREFIX	"M8.newlink."
#define WCS_APPHELP_PATH				"Help\\%s%s"
#define WCS_APPHELP_PATH_TUTORIALS		"Help\\Tutorials\\"

#define WCS_APPHELP_TUTORIALBASE "index"WCS_APPHELP_HTMLSUFFIX

class Raster;
class Thumbnail;

class AppHelp
	{
	friend class HelpFileMetaData;
	friend class HelpFileIndex;
	private:
		char FullHelpPathBuf[1024], HelpBasePath[512], TempScratch[1024];

		char *AssociateIDtoFile(unsigned long TopicID);
		char *FormatIDforPDFDest(const char *NamedDest);
		char *AttemptOpenHelpFile(char *HelpBaseName, int AddSuffix);
	public:
		int ShellOpenHelpFile(char *FullFileName);
		int ShellOpenHelpFilePDF(char *FullFileName, char *NamedDestination = NULL);
		int OpenHelpTopic(unsigned long TopicID);
		int OpenHelpFile(char *HelpFile, char *TopicName = NULL);
		static int OpenURLIndirect(char *URL);
		int DoOnlineUpdate(void);
		int DoOnlineReport(void);
		int DoOnlineRegister(void);
		int DoOnlinePurchase(void);
		int OpenCredits(void); // replaces credits window with HTML
	}; // AppHelp

class HelpFileMetaData
	{
	friend class HelpFileIndex;
	private:
		char Title[50], Brief[100], IconFileName[100], Keywords[300], Level[30], Set[100], Product[50], *Abstract;
		int Priority;
		PathAndFile PAF;
		Thumbnail *TN;
		Raster *ThumbHostRaster;
		HelpFileMetaData *Next;

	public:
		HelpFileMetaData();
		~HelpFileMetaData();

		void SetFileName(char *NewPath, char *NewName); // drops straight into PAF

		int ReadMetaTags(void); // uses path/name from PAF
		int ReadMetaTags(FILE *HelpFile);
		const char *GetTitle(void) {return(Title);};
		const char *GetBrief(void) {return(Brief);};
		const char *GetIconFileName(void) {return(IconFileName);};
		const char *GetKeywords(void) {return(Keywords);};
		const char *GetLevel(void) {return(Level);};
		const char *GetSet(void) {return(Set);};
		const char *GetProduct(void) {return(Product);};
		const char *GetAbstract(void) {return(Abstract ? Abstract : "");}; // never returns NULL
		int GetPriority(void) {return(Priority);};

		Thumbnail *AttemptLoadThumb(void);
		Raster *GetThumbRaster(void) {return(ThumbHostRaster);};
		int Launch(void);
	}; // HelpFileMetaData

class HelpFileIndex
	{
	private:
		int NumHelpFiles;
		HelpFileMetaData *MDChain;

	public:
		HelpFileIndex();
		~HelpFileIndex();

		int BuildFileIndex(char *BaseDir);
		void DiscardFileIndex(void);
		int GetNumHelpFiles(void) {return(NumHelpFiles);};
		HelpFileMetaData *GetFirst(void) {return(MDChain);};
		HelpFileMetaData *GetNext(HelpFileMetaData *Me) {if(Me) return(Me->Next); else return(NULL);};
		HelpFileMetaData *GetNum(int FileNum);
		int FindHighestPriorityNum(void);
	}; // class HelpFileIndex

#endif // WCS_APPHELP_H
