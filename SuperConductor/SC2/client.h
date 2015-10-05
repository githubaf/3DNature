// client.h

// It is important to note that this should be as memory efficient as possible
// because there will be a new instance of your derived client class created
// for every node that is used as a renderer, and in doing so will be able to
// chew up a bunch of memory quite quickly if it is allowed to, depending on
// the number of clients that you have connected

#ifndef CLIENT_H
#define CLIENT_H

// Qt includes
#include <qobject.h>
#include <qvariant.h>
#include <qptrlist.h>
#include <qthread.h>
#include <qstring.h>
#include <qhostaddress.h>
#include <qmessagebox.h>
#include <qsocket.h>
#include <qtimer.h>
#include <qstring.h>
#include <qlibrary.h>
#include <qmutex.h>

// local includes
#include "datatypes.h"

// client declarations
// this class is made to be inherited, so that it is overloadable
// by other clients
class SCClient : public QObject
{
	Q_OBJECT
public:
	// virtual constructor defined... we don't want any direct instances of this class
	virtual ~SCClient() { while (settings.current() != NULL) settings.remove(); }
	
	// returns a list of all jobs from the parsed file, which is passed in as
	// path + filename format, should only need to open the file and examine it
	virtual collectedSCProjects& parse(QString) = 0;
	
	// returns a pointer list to Setting objects, pairs
    // contains list of ALL possible settings, and initial values (if any)
    // take care of this in the constructor
    // SuperConductor will take care of the value
	virtual QPtrList<Setting> getConfig() = 0;

	// Set all internal client settings to be like passed in QPtrList
	virtual void setConfig(QPtrList<Setting>) = 0;
    
	// Identify plugin with a QString (e.g. "WCS/VNS" or something suitable for
    // a user to identify the plugin with, as well as able to be used for a sting
    // for internal matching in the program
    virtual QString identity() = 0;
    
	// CSV QString of valid extensions for your projects, (e.g. "proj,prj")
    virtual QString extensions() = 0;
	
	// load a project from a filename.  May or may not be needed, depending on
    // renderer.  Same format, etc. as parse.  Used by controller when assigning
	// a different job to the client, as it is sometimes needed
	virtual void load(QString) = 0;
	
	// returns the filename of the loaded project that the client is working on
	// used so that reloading of an already loaded file is not done
	virtual QString loadedFile() = 0;
	
	// starts the rendering on the client.  Require that all configuration
	// be set in the setConfig function
	// takes in a QString as the project name, and an integer frame number
	// Project name needed in case there are multiple projects in a file
	virtual void render(QString, long int) = 0;
	
	// halts the rendering on the client completely, flushes any cache, etc.
	virtual void halt() = 0;
	
	// pauses rendering on the client
	virtual void pause() = 0;
	
	// unpauses the rendering... can be just a pass-through to render
	virtual void unpause() = 0;

	// universal client functions implemented below... don't override

	void connectSignals(QObject* parent) {
		if (parent != NULL) {
			// connect the signals to the parent thread
			connect(this, SIGNAL(failed(const QString&, const SCClient*)),
					parent, SLOT(failed(const QString&, const SCClient*)));
			connect(this, SIGNAL(loaded(const QString&, const SCClient*)),
					parent, SLOT(loaded(const QString&, const SCClient*)));
			connect(this, SIGNAL(changed(const QString&, const SCClient*)),
					parent, SLOT(changed(const QString&, const SCClient*)));
			connect(this, SIGNAL(echo(const QString&, const SCClient*)),
					parent, SLOT(echo(const QString&, const SCClient*)));
			connect(this, SIGNAL(finished(const QString&, const SCClient*)),
					parent, SLOT(finished(const QString&, const SCClient*)));
			connect(this, SIGNAL(halted(const QString&, const SCClient*)),
					parent, SLOT(halted(const QString&, const SCClient*)));
		}
	}

	SCProject* getProject() { return proj; }
	void setProject( SCProject* p ) { mutex.lock(); proj = p; load_ok = false; mutex.unlock(); }
	QString getName() { return nameStr; }
	void setName( QString n ) { mutex.lock(); nameStr = n; mutex.unlock(); }
	// act acts like a spinlock, telling if the client is running or not
	void setActive( bool a ) { mutex.lock(); act = a; mutex.unlock(); }
	bool active() { return act; }
	bool irrecoverable() { return irreco; }
	bool load_status() { return load_ok; }
	void setFrame( SCFrame* frm ) { mutex.lock(); frame = frm; mutex.unlock(); }
	SCFrame* getFrame() { return frame; }
	
signals:
	
	// emitted when the rendering failed, AFTER the client has taken
	// appropriate steps like trying to reconnect to program, etc.
	void failed(const QString&, const SCClient*);

	// emitted when a loading finishes, so we know it's ok to ask a client to render
	void loaded(const QString&, const SCClient*);
	
	// If the status has changed for the client's render status, or another
	// similar status-style update (started rendering, percent done, etc.)
	void changed(const QString&, const SCClient*);
	
	// Used for general informational logging.  Say, a dropped connection, which
	// is then resumed, or whatever other diagnostic information you want, any 
	// information which is non-critical for program operation
	void echo(const QString&, const SCClient*);
	
	// emitted when the frame is finished, and the client is ready for another
	void finished(const QString&, const SCClient*);
	
	// emitted when the client successfully halts, to inform thread of status
	// so thread can do cleanup, get ready to start again, controller "knows" that
	// the node is no longer running and it can make changes to the client 
	// configuations without worrying about breaking
	void halted(const QString&, const SCClient*);
	
	// progress is a special signal emitted, used to update the rendering progress
	// of an individual client.  The first string is informational (say, "shadow map generation"
	// or the like) and the second is the percentage (per cent, as in it goes from 0-100)
	void progress(const QString&, int, const SCClient*);
	
	
protected slots:

protected:
	QMutex mutex;
	bool act, load_ok, irreco;
	SCProject* proj;
	SCFrame* frame;
	QPtrList<Setting> settings;
	QString nameStr;
};

/*
There are also 2 functions needed by Superconductor, being as this is a
library and there needs to be an interface of some sort
Functions need to be named "create" and "destroy"
Examples:

extern "C" SCClient* create() {
	return new yourClassInstance;
}

extern "C" void destroy(SCClient* s) {
	delete s;
}

See wcs-client.h in ./plugins for a more detailed example
*/

#endif // CLIENT_H
