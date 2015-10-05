/* Scheduler.h
 * This is used to contain the scheduler and the thread class that the
 * scheduler interfaces with.  The thread class contains instances of the
 * clients, and uses the API set out in client.h, so abstracts the interface
 * and allows more control for the scheduler
 */

#ifndef SERVER_H
#define SERVER_H

#include <qwidget.h>
#include <qapplication.h>
#include <qevent.h>
#include <qptrlist.h>
#include <qlibrary.h>
#include <qdir.h>
#include <qlibrary.h>
#include <qstring.h>
#include <qsettings.h>
#include <qfiledialog.h>
#include <qmap.h>

#include "client.h"
#include "datatypes.h"
#include "scthread.h"

//class SCThread;

/* SCServer class, used to interface with SCMain GUI and control threads
 * subclass of QWidget because we want it to have a QCustomEvent catcher
 */
class SCServer:public QWidget
{
    Q_OBJECT
	public:
		SCServer (SCMessageList * msgs = NULL);
		~SCServer ();

		// I/O type thingies
		QPtrList<SCProject> getProjects ();
		QPtrList<SCClient> getClients( SCProject* prj );
		void saveClients( SCProject* prj );
	
		bool removeProject ( SCProject* prj );

		bool run ( SCProject* prj );
		bool pause ( SCProject* prj );
		void unpause ( SCProject* prj );
		bool stop ( SCProject* prj );

		SCClient* newClient ( SCProject* prj , QString name, bool warn=true);
		bool removeClient ( SCProject* prj, SCClient* cli);

		bool onMultipleProjects ( SCClient *cli );
		void setMessages ( SCMessageList * ptr ) { messages = ptr; }

		QString stuff;		// very temporary storage

	signals:
		void projectFinished(SCProject*);
		void updateDisplay(SCProject*); // generic display update call, updates all fields

	protected slots:
		void failed(const QString&, const SCClient*);
		void loaded(const QString&, const SCClient*);
		void changed(const QString&, const SCClient*);
		void echo(const QString&, const SCClient*);
		void finished(const QString&, const SCClient*);
		void halted(const QString&, const SCClient*);

    private:
		void frameDone(const QString&, const SCClient*, bool);
		void balance();
		double score( SCProject* prj );
		bool moveClient( SCProject* prjA, SCProject* prjB);  // move first valid client from prjA to prjB

		QPtrList < QLibrary > plugins;	// list containing the plugins
		QPtrList < SCClient > clients;	// list of client objects
		QPtrList < SCProject > projects;	// this contains the list of jobs
		QMap < SCClient*, SCThread* > cliThreadMap; // maps client pointers to their current thread
		SCMessageList *messages;
};

#endif // SERVER_H
