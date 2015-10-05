// serverobject.h

#include <qapplication.h>
#include <qvbox.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <stdlib.h>
#include <qlayout.h>

#include "client.h"

#ifndef SERVERPLUG_H
#define SERVERPLUG_H

// ServerObject declaration, which will contain the sockets, a pointer to
// the main ServerWidgets list of client names, and do the management of
// the socketlist object.  No actual network connections are made to or from
// ServerObject class instance
class ServerObject : public QObject
{
	Q_OBJECT
public:
	ServerObject(QStringList* addys=NULL, Scheduler* joblist=NULL);
	~ServerObject();
	void setClients(QStringList clientlist);
	bool AddJobToScheduler(Job *job);
	void setPriority(uint JobId, int newPriority);
	bool removeJob(uint JobId);
	void pauseJob();
	//bool parser(int&,int&,int&,QString&,QString);

signals:
	void finished(const QString&);
	void connectChange(const QString&);
	void changed(Job*,QString,QString,int);

private slots:
	void startJobs();
	void couldntConnect(const QString&, const QThread*);
	void disconnect(const QString&, const QThread*);
	void connected(const QString&, const QThread*);
	void jobDone(const QString&, QThread*);
	void continueRender(const QString&, QThread*);
	void echo(const QString&, QThread*);
	void jobFailed(const QString&, QThread*);
	void statusUpdate(Job*);
	void statusUpdate(const QString&, const QThread*,int);

private:
	void nextJob(Thread*);  // moves the thread the next job;
	QString suffix;
	QStringList clientlist;
	Scheduler *jobs;
	QPtrList<Thread> threadlist;  // make the new threadlist
	bool started;  // this variable holds whether the jobs are started or not
	bool unpaused;
};






#endif


