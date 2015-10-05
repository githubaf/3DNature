// client.h

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
#include "scheduler.h"
#include "wcs-client.h"
#ifndef CLIENT_H
#define CLIENT_H

// client declarations
class Client : public QObject
{
	Q_OBJECT
public:
	Client();
	Client(int sock, QThread* th=NULL, QObject* parent=NULL, const char* name=NULL);
	~Client() {}

	void render(const QString&);
	void connectTo (QString, int);
	//bool parse(int&, int&, int&, QString&, QString);
	//bool parse(int&, int&, int&, QString&, QString);
	QHostAddress address() { return client->address(); }
	QHostAddress peerAddress() {return client->peerAddress();}
	void stillAlive();

signals:
	void changed(const QString&, const QThread*, int);
	void continueRender(const QString&, QThread*);
	void echo(const QString&, QThread*);
	void jobFailed(const QString&, QThread*);
	void jobDone(const QString&, QThread*);
	void connected(const QString&, const QThread*);
	void disconnect(const QString&, const QThread*);
	void couldntConnect(const QString&, const QThread*);
	
private slots:
	void updated();
	void connectStall();

private:
	bool jobkilled;
	bool Loaded;
	wcsClient* client;
	QString statusBuffer;
	QString projectName;
	QThread* thread;
	QTimer stalled;
};

class Thread : public QThread
{
public:
	Thread();
	Thread(QObject*, int);			// creates a thread with a "parent" object
									// needed to connect the socket to something
									// and a number for the socket
	~Thread() {}

	void mutlock();
	void mutunlock();

	// Client access/configuration functions
	void setHost(const QString&);	// hostname
	void setInfo(WorkItem*);	// sets the renderinfo variable
	WorkItem *getInfo();
	bool isReady();
	void setReady(bool);
	void setJobId(uint jobid){ JobId=jobid; }
	uint getJobId(){ return JobId; }
	QString getJobName() { return JobName; }
	void setJobName(QString jobName){ JobName = jobName; }
	Client* getClient() { return socket; }	// returns address for connection information
	QString getHost() {return host;}

	void run();
	void stop();
	QString FilePath;

private:
	QObject* parent;
	Client* socket;
	QMutex mutex;
	WorkItem *renderinfo;
	QString host;
	QString JobName;
	
	uint JobId;
	int sockNum;
	bool stopped;
};

#endif
