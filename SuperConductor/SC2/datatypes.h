//--------------------------------------------------------------------
//  Author: David Kopp, CSM Superconductor team
//  Date: 8/28/03
//
//  datatypes.h
//
//  Basic datatypes and definitions that are essential to
//  Superconductor working. Included in almost all of the source files.
//
//--------------------------------------------------------------------

#ifndef DATATYPES_H
#define DATATYPES_H

// This tells us to use the plugins, or just statically compile in only the WCS support
// plugins don't seem to work under Windows.
//#define USE_PLUGINS

//#ifndef USE_PLUGINS
//#include <wcs-client.h>
//#endif

#include <iostream>

using namespace::std;

#include <qobject.h>
#include <qptrqueue.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qlibrary.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmutex.h>

class SCProject;
class collectedSCProjects;
class Setting;
class SCFrame;
class SCClient;

#ifdef USE_PLUGINS
typedef SCClient* (*createProto)();
typedef void (*destroyProto)(SCClient*);
#else
class wcsClient;
#endif // USE_PLUGINS

const QString APP_KEY = "/Superconductor/";

// renderInfo is a non-inheriting class
// that just has public data members
// Information:
class renderInfo
{
public:
	int priority;
	int frameInterval;
	int startFrame;
	int endFrame;
	QString jobName;
	renderInfo *next;
	renderInfo(renderInfo * nextRI = NULL)
	{
		next = nextRI;
		priority = -1;
		frameInterval = 0;
		startFrame = 0;
		endFrame = 0;
		jobName = "";
	}
	renderInfo(int pri, int fi, int sf, int ef, QString jn, renderInfo * nextRI = NULL)
	  :priority (pri), frameInterval (fi), startFrame (sf), endFrame (ef), jobName (jn), next (nextRI)
	{}
	// equals and copy constructor so that
	// it can be used to make a real/deep
	// copy in the collection class easier
	// they also act as nodes in the
	// collectedSCProjects class
	renderInfo(renderInfo& b)
	{
		priority = b.priority;
		frameInterval = b.frameInterval;
		startFrame = b.startFrame;
		endFrame = b.endFrame;
		jobName = b.jobName;
	}

	renderInfo& operator=(renderInfo & b)
	{
		priority = b.priority;
		frameInterval = b.frameInterval;
		startFrame = b.startFrame;
		endFrame = b.endFrame;
		jobName = b.jobName;
		return *this;
	}
};

// name and value pairs
class Setting
{
    public:
		Setting () { nm = ""; val = ""; }
		Setting (QString n, QString v):nm (n), val (v) { }
		Setting (Setting & s) { nm = s.nm; val = s.val; }
		~Setting () { }
		Setting & operator = (Setting & s) {
			nm = s.nm;
			val = s.val;
			return *this;
		}
		QString name () { return nm; }
		QString value () { return val; }
		QString serial() { return nm + "," + val; }
	private:
		QString nm;
		QString val;
};

class SCClientListItem : public QListBoxText
{
	public:
		SCClientListItem( QListBox* parent, QString cliName, SCClient* cli, SCProject* pro )
			: QListBoxText( parent, cliName ) { myClient = cli; myProject = pro; }
		~SCClientListItem() { /* don't need to delete any vars, done in SCServer */ }
		SCClient* client() { return myClient; }
		SCProject* project() { return myProject; }
	private:
		SCClient* myClient;
		SCProject* myProject;
};

class SCMessageList : public QStringList
{
	public:
		QStringList& operator <<(const QString& x) {
			QTime aTime = QTime::currentTime();
			append(aTime.toString("hh:mm:ss ap: ") + x );
			return *this;
		}
	private:
};

class SCProjectListItem : public QListViewItem
{
	public:
		SCProjectListItem( QListView* parent, SCProject* pro, QString label1, QString label2 = QString::null,
						  QString label3 = QString::null, QString label4 = QString::null,
						  QString label5 = QString::null, QString label6 = QString::null,
						  QString label7 = QString::null, QString label8 = QString::null );
/*		SCProjectListItem( QListView* parent, SCProject* pro, QString label1, QString label2,
						  QString label3, QString label4, QString label5,
						  QString label6, QString label7, QString label8 );*/
		~SCProjectListItem();
		SCProject* project();
	private:
		SCProject* myProject;
		QTimer* timer;
};

class SCFrame {
	// holds a single frame info
	public:
		SCFrame( uint n ) { num = n; failcount = 0; }
		uint number() { return num; }
		void startMark() { start = QTime::currentTime();}
		void endMark() { end = QTime::currentTime(); }
		int frameTime() { return start.secsTo(end); }
		void failed() { failcount++; }
		uint failures() { return failcount; }
	private:
		QTime start;
		QTime end;
		uint num;
		uint failcount;
};

// this class is basically a linked list class
// holding all of the renderInfo's you send to
// it, deep copying all their data
class collectedSCProjects
{
private:
	// current is an iterator, head
	// is pretty self explanatory
	renderInfo* head;
	renderInfo* current;
	void purge();
	uint size;
public:
	// only one constructor
	collectedSCProjects();
	collectedSCProjects(collectedSCProjects& b);
	~collectedSCProjects();
	collectedSCProjects& operator = (collectedSCProjects& b);
	bool isEmpty();
	bool isAtEnd();
	uint count() { return size; }
	void addSCProject(renderInfo& obj);
	renderInfo* getNextSCProject();
	// this is in case you need to reset the list for
	// whatever reason... error in reporting job or
	// some such thing
	void reset();
	QString filename;
};


class SCProject {
	// decided this needs to be its own class as well
	public:
		enum statusType { stopped = 0, running, paused, finished, error };
	
		SCProject();
		SCProject( renderInfo* ri, QLibrary* lib, QString fn );
		~SCProject();
	
		QString timePerFrame();
		QString elapsedTime();
		QString estimate();
		void startMark() { startTime = QDateTime::currentDateTime(); }
		void finishMark() { endTime = QDateTime::currentDateTime(); }
		
		// modifiers
		void setPriority( int newPri );
		void setStatus( statusType stat );
	
		// these return null if there is nothing available
		SCFrame* getNextFrame();
		SCFrame* getFailedFrame();
		void frameFailed( SCFrame* fail );
		void frameFinished( SCFrame* finish );

		void addClient( SCClient* newCli );
		void dropClient( SCClient* deadCli );
		bool hasClient( SCClient* aCli );
		bool hasClient(QString cliName);
		SCClient* getNamedClient( QString cliName );
		QPtrList<SCClient> clientList();
		QString status();
		QString libString();
		
		// inline implemented functions
		bool isStopped() { return ( statusVal == stopped ); }
		bool isRunning() { return ( statusVal == running ); }
		long int priority() { return priorityVal; }
		statusType statusValue() { return statusVal; }
		QString name() { return projectName; }
		QString fn() { return fileName; }
		uint frames() { return frameCount; }
		uint currentFrame() { return current->number(); }
		QLibrary* lib() { return myLib; }

		uint failCount() { return failedFrames.count(); }
		uint finishCount() { return finishedFrames.count(); }
		uint newCount() { return newFrames.count(); }

	private:
		QString projectName;
		QString fileName;
		QString libName;
		QLibrary* myLib;
	
		// values allow for ( if (!project.statusValue()) {
		statusType statusVal;
		uint startFrame;
		uint endFrame;
		uint frameInterval;
		uint frameCount;
		long int priorityVal;
		QDateTime startTime; // not QTime in case it takes > 24 hours
		QDateTime endTime; // not applicable until project finishes
		QMutex mutex;
		// SCProject has it's own lists of objects clients is a list of
		// applicable clients... the SCServer has the master list
		SCFrame* current;
		QPtrList< SCFrame > newFrames;
		QPtrList< SCFrame > finishedFrames;
		QPtrList< SCFrame > failedFrames;
		QPtrList< SCClient > clients;
};

#endif // DATATYPES_H
