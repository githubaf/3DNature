//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  datatypes.cpp
//
//  class to group and manipulate Work Items.  Work Queue will have
//  a list of jobs. Each job has a priority for scheduling purposes.
//
//--------------------------------------------------------------------

#include "datatypes.h"
#include "client.h"
#include "scthread.h"

#ifndef USE_PLUGINS
#include "wcs-client.h"
#endif

/*
*
* collectedSCProjects class function definitions
*
*/

void collectedSCProjects::purge()
{
	renderInfo *temp;
	while(head != NULL)
	{
		temp = head;
		head = head->next;
		delete temp;
	}
	head = current = NULL;
}

collectedSCProjects::collectedSCProjects()
{
	head = NULL;
	current = NULL;
	size = 0;
}

collectedSCProjects::collectedSCProjects(collectedSCProjects& b)
{
	current = head = NULL;
	b.reset();
	renderInfo *d;
	d = b.getNextSCProject();
	while (d != NULL)
	{
		addSCProject (*d);
		d = b.getNextSCProject();
	}
	b.reset();
}

collectedSCProjects::~collectedSCProjects()
{
	// need to deallocate all the mem here
	purge();
}

collectedSCProjects& collectedSCProjects::operator=(collectedSCProjects& b)
{
	purge();
	b.reset();
	renderInfo *d;
	d = b.getNextSCProject();
	while (d != NULL)
	{
		addSCProject(*d);
		d = b.getNextSCProject();
	}
	b.reset();
	this->reset();
	return *this;
}

bool collectedSCProjects::isEmpty()
{
	if(head != NULL)
		return false;
	return true;
}

bool collectedSCProjects::isAtEnd()
{
	if (current != NULL)
		return false;
	return true;
}

void collectedSCProjects::addSCProject(renderInfo& obj)
{
	// plugs an item in at the head of the list
	// since order is unimportant
	renderInfo *temp = new renderInfo(obj);
	temp->next = head;
	head = temp;
	current = head;
	++size;
}

renderInfo* collectedSCProjects::getNextSCProject()
{
	if (current == NULL)
	{
		return current;
	}
	else
	{
		renderInfo *temp = current;
		current = current->next;
		return temp;
	}
}

void collectedSCProjects::reset()
{
	current = head;
}

/*
*
* SCProjectListItem class function definitions
*
*/
/*SCProjectListItem::SCProjectListItem( QListView* parent, SCProject* pro, QString label1, QString label2 = QString::null,
						  QString label3 = QString::null, QString label4 = QString::null,
						  QString label5 = QString::null, QString label6 = QString::null,
						  QString label7 = QString::null, QString label8 = QString::null )*/
SCProjectListItem::SCProjectListItem( QListView* parent, SCProject* pro, QString label1, QString label2,
						  QString label3, QString label4, QString label5,
						  QString label6, QString label7, QString label8 )
			: QListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
	{
	myProject = pro;
}

SCProjectListItem::~SCProjectListItem() {
	/* don't need to delete any vars, done in SCServer */
}

SCProject* SCProjectListItem::project() {
	return myProject;
}


/*
*
* SCProject class function definitions
*
*/

SCProject::SCProject() {
	myLib = NULL;
	current = NULL;
	statusVal = stopped;
	startFrame = 1;
	endFrame = 1;
	frameInterval = 1;
	priorityVal = 0;
	projectName = "BAD - DON'T USE ME";
}

SCProject::SCProject( renderInfo* ri, QLibrary* lib, QString fn ) {
	fileName = fn;
	myLib = lib;
	statusVal = stopped;
	startFrame = ri->startFrame;
	endFrame = ri->endFrame;
	frameInterval = ri->frameInterval;
	priorityVal = ri->priority;
	projectName = ri->jobName;
	current = NULL; // no rendering going on yet
	frameCount = 0;
	// now we need a loop to fill the newFrames object
	for ( uint i = startFrame; i <= endFrame; i += frameInterval ) {
		newFrames.append( new SCFrame( i ));
		++frameCount;
	}
	// once the frames get to finishedFrames, it "owns" them, so
	// it can delete them
	finishedFrames.setAutoDelete( true );
	#ifdef USE_PLUGINS
	createProto cr;
	destroyProto ds;
	cr = (createProto) lib->resolve( "create" );
	ds = (destroyProto) lib->resolve( "destroy" );
	SCClient* tmp = cr();
	libName = tmp->identity();
	ds (tmp);
	#else
	wcsClient* tmp = new wcsClient;
	libName = tmp->identity();
	delete tmp;
	#endif
}

SCProject::~SCProject() {
	// kill all memory used/created
	newFrames.setAutoDelete(true);
	newFrames.clear();
	// finishedFrames clears it's own, and we don't own clients or lib
	// all dynamically allocated mem is cleared
}
	
QString SCProject::timePerFrame() {
	// no days calculations here... it better not be
	// more than a day/unit
	if (finishedFrames.count()) {
		QTime t;
		QDateTime dt = QDateTime::currentDateTime();
		t.addSecs(startTime.secsTo(dt)/finishedFrames.count());
		return t.toString();
	}
	return "No Time";
}

QString SCProject::elapsedTime() {
	QTime t;
	int days = 0; // should not need more than days in the time elapsed
	if (statusVal == finished) {
		days = startTime.daysTo(endTime);
		// next line subtracts total days seconds, so we only have a
		// "remainder" of seconds in t, and if days is 0, it's simply
		// ignored
		t = t.addSecs(startTime.secsTo(endTime) - (days*24*60*60));
	}
	else if ( !startTime.isNull() ) {
		QDateTime dt = QDateTime::currentDateTime();
		days = startTime.daysTo(dt);
		t = t.addSecs(startTime.secsTo(dt) - (days*24*60*60));
	}
	QString ret = "";
	if (days) { ret.setNum(days); ret += " days, "; }
	ret += t.toString();
	return ret;
}

QString SCProject::estimate() {
	// this will return an estimate dependent on all currently finished frames
	QString ret;
	QTime t;
	if (finishedFrames.count() && newFrames.count() && (statusVal != finished)) {
		// we can make a semi-reliable estimate only if there are finished frames
		QDateTime dta = QDateTime::currentDateTime();
		QDateTime dtb(dta);
		// number frames left times the average time/frame until now
		int estSecs = newFrames.count() * startTime.secsTo(dta) / finishedFrames.count();
		dtb.addSecs(estSecs);
		int estDays = dta.daysTo(dtb);
		t = t.addSecs(dta.secsTo(dtb) - (estDays*24*60*60));
		if (estDays) { ret.setNum(estDays); ret += " days, "; }
		ret += t.toString();
	}
	else if (statusVal == finished ) {
		ret = "Finished";
	}
	else {
		ret = "No Estimate";
	}
	return ret;
}

void SCProject::setPriority( int newPri ) {
	priorityVal = newPri;
}

void SCProject::setStatus( statusType stat ) {
	statusVal = stat;
}

SCFrame* SCProject::getNextFrame() {
	SCFrame* tmp = NULL;
	mutex.lock();
	if (tmp = newFrames.first())
		newFrames.removeRef( tmp );
	return tmp;
	mutex.unlock();
}

void SCProject::frameFailed( SCFrame* fail ) {
	// put it at the top of the range again
	mutex.lock();
	fail->failed();
//	if ((fail->failures() < 3 )&&(newFrames.find(fail) == -1))
	if (fail->failures() < 3 )
			newFrames.append( fail );
//	else if (failedFrames.find(fail) == -1)
	else
		failedFrames.append ( fail );
	mutex.unlock();
}

void SCProject::frameFinished( SCFrame* finish ) {
	mutex.lock();
	if (finishedFrames.find(finish) == -1)
		finishedFrames.append( finish );
	mutex.unlock();
}
	
void SCProject::addClient( SCClient* newCli ) {
	if (!hasClient( newCli )) {
		clients.append( newCli );
	}
}

void SCProject::dropClient( SCClient* deadCli ) {
	clients.removeRef( deadCli );
}

bool SCProject::hasClient( SCClient* aCli ) {
	if (clients.findRef( aCli ) != -1) {
		return true;
	}
	return false;
}

bool SCProject::hasClient( QString cliName ) {
	SCClient* aCli;
	for ( aCli = clients.first(); aCli; aCli = clients.next()) {
		if ( aCli->getName() == cliName) {
			return true;
		}
	}
	return false;
}

SCClient* SCProject::getNamedClient( QString cliName ) {
	SCClient* aCli;
	for ( aCli = clients.first(); aCli; aCli = clients.next()) {
		if ( aCli->getName() == cliName) {
			return aCli;
		}
	}
	return NULL;
}

QPtrList<SCClient> SCProject::clientList() {
	QPtrList<SCClient> ret;
	clients.first();
	if (clients.count()) {
		do {
			ret.append(clients.current());
		} while ( clients.next());
	}
	return ret;
}

QString SCProject::status() {
	switch ( statusVal ) {
		case stopped:
			return QString("Stopped");
		case running:
			return QString("Running");
		case paused:
			return QString("Paused");
		case finished:
			return QString("Finished");
		case error:
			return QString("Error");
	}
	return QString("Undefined");
}

QString SCProject::libString() {
	return libName;
}
