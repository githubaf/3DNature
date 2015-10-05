// client.cpp

#include "client.h"

#ifndef ESCCHAR
#define ESCCHAR
const QChar escchar(27);
#endif

/*
The Client class provides a socket that is connected with a client.
For each client in list, the server attempts to create a new connection object
from this class to the client app (actually a server socket that accepts connections)
*/
Client::Client() {
	// this default constructor leaves the client "broken"
	// do not use for any "real" client declarations
}

Client::Client( int sock, QThread* th, QObject *parent, const char *name )
{
	Loaded = false; jobkilled = false;
	thread = th;
	client = new wcsClient(name, sock, this);
	// connect all relevant signals, sometimes to the "parent" object ServerObject
	// when created by a thread
	// Connecting outside of Client for status updates
	connect(client, SIGNAL(updated()), this, SLOT(updated()));

	// local program connections
	connect(this, SIGNAL(jobDone(const QString&, QThread*)),
		parent, SLOT(jobDone(const QString&, QThread*)));
	connect(this, SIGNAL(continueRender(const QString&, QThread*)),
		parent, SLOT(continueRender(const QString&, QThread*)));
	connect(this, SIGNAL(echo(const QString&, QThread*)),
		parent, SLOT(echo(const QString&, QThread*)));
	connect(this, SIGNAL(jobFailed(const QString&, QThread*)),
		parent, SLOT(jobFailed(const QString&, QThread*)));
	connect(this, SIGNAL(disconnect(const QString&, const QThread*)),
		parent, SLOT(disconnect(const QString&, const QThread*)));
	connect(this, SIGNAL(couldntConnect(const QString&, const QThread*)),
		parent, SLOT(couldntConnect(const QString&, const QThread*)));
	connect(this, SIGNAL(connected(const QString&, const QThread*)),
		parent, SLOT(connected(const QString&, const QThread*)));
	connect(this, SIGNAL(changed(const QString&, const QThread*, int)),
		parent,SLOT(statusUpdate(const QString&, const QThread*,int)));
}

// this function is wholly in charge of output to the WCS6 program
// Loads a project if the variable is true, otherwise it sends the
// message to WCS about the project (render a frame or whatnot)
void Client::render(const QString& aString)
{
	if (aString == escchar) {
		client->halt();
		Loaded = false;
	}
	else {
	// here we make sense of the aString, then send it on to the client as needed
	// format fileName?projName?frameNum
		QString fn = aString.section("?", 0, 0);
		QString pn = aString.section("?", 1, 1);
		int fNum = aString.section("?", 2, 2).toInt();
	// check to see if job is loaded, if not, then load it
		if ((Loaded)&&(pn == projectName)) {
			client->render(pn, fNum);
		}
		else {
			client->load(fn, "");
			projectName = pn;
			Loaded = false;
		}
	}
}

void Client::connectTo(QString host, int port) {
	client->connectTo(host, port);
}

// this will parse the file and put it into the correct format for
// render, thereby making the project load option no longer a concern
/*
bool Client::parse(int& stFrame, int& enFrame, int& frInterval, QString& projectName, QString fn) {
	// now we can create a client according to the filename/registered fn's
	// for right now, WCS is only file type, so checking not necessary
  //wcsClient aClient;
  //if (aClient.parse(fn, stFrame, enFrame, frInterval, projectName))
  //	return true;
	return false;
}
*/
void Client::stillAlive() {
	stalled.stop();
	connect (&stalled, SIGNAL(timeout()), this, SLOT(connectStall()));
	stalled.start(10*60*1000, TRUE);  // start the 10-minute timeout
}

void Client::connectStall() {
	client->halt();
	emit jobFailed("Apparent error on render client", thread);
}

void Client::updated() {
	stillAlive();
	// get the information in statusBuffer
	statusBuffer = client->status();
	// and deal with what it says
	if(statusBuffer.section("?", 0, 0) == "ERROR") {
		if (statusBuffer.section("?", 1, 1) == "FATAL") {
			emit disconnect(statusBuffer.section("?", 2, 2), thread);
		}
		else if (statusBuffer.section("?", 1, 1) == "RECOVERABLE") {
			emit couldntConnect(statusBuffer.section("?", 2, 2), thread);
		}
		else
			emit jobFailed("Bad plugin error string.", thread);
	}
	else if (statusBuffer.section("?", 0, 0) == "PROGRESS") {
		// here is the status update code
		emit changed(statusBuffer.section("?", 1, 1), thread, statusBuffer.section("?", 2, 2).toInt());
	}
	else if (statusBuffer.section("?", 0, 0) == "CONNECTED") {
		// yay!  It connected... now get to it, slacker
		emit connected(statusBuffer.section("?", 1, 1), thread);
	}
	else if (statusBuffer.section("?", 0, 0) == "RESULT") {
		if (statusBuffer.section("?", 1, 1) == "TRUE") {
			if (!Loaded) {
				Loaded = true;
				emit continueRender("Loaded file successfully", thread);
			}
		}
		else
			// emit to say that the result was bad
			emit jobFailed("Job Failed", thread);
	}
	else if (statusBuffer.section("?", 0, 0) == "ECHO") {
		emit echo(statusBuffer.section("?",1,-1), thread);
	}
	else if (statusBuffer.section("?", 0, 0) == "OUTPUT") {
		// can change later to check for successful creation of filename
		emit echo(statusBuffer.section("?", 1, 1), thread);
		emit jobDone("Job finished", thread);
	}
	else
		emit echo(statusBuffer, thread);
}

/*
The Thread class encloses all threaded operations for the server
Each one contains a socket, so the processes can run independently
*/
// default thread constructor
Thread::Thread() {
     socket = NULL;
	 stopped = TRUE;
	 renderinfo=NULL;
}

// Make a new client
// We will need to pass this an object type of some sort
// in order for us to allow for plugins
Thread::Thread(QObject* obj, int num)
:parent(obj), sockNum(num) {
	renderinfo=NULL;
	stopped = TRUE;
	// create the client
	socket = new Client(sockNum, this, parent);
}

// lock the data that the thread is accessing
void Thread::mutlock() {
    mutex.lock();
}

// unlock the data that the thread was accessing
void Thread::mutunlock() {
    mutex.unlock();
}

// Change the host of the socket
void Thread::setHost(const QString& str) {
	host = str;
	socket->connectTo(str, sockNum);
}

// set the render information
void Thread::setInfo(WorkItem* str) {
	renderinfo = str;
}

// Get the info from the thread
WorkItem * Thread::getInfo()
{
	return renderinfo;
}

bool Thread::isReady()
{
	return !stopped;
}

void Thread::setReady(bool k)
{
	stopped = !k;
}

// run the thread, start the connection, etc.
void Thread::run()
{
	QString send;
	stopped = FALSE;
	send=renderinfo->getJob()->getFileName()+'?'+renderinfo->getJob()->getJobName()+'?'+renderinfo->getFrameInfo();
	socket->render(send);
}

// stop the thread.  Not used...
void Thread::stop()
{
	mutex.lock();
	stopped=TRUE;
	mutex.unlock();
}

