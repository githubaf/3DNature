// serverobject.cpp

#include "serverobject.h"
#include <qmessagebox.h>

#ifndef ESCCHAR
#define ESCCHAR
const QChar escchar(27);
#endif

ServerObject::ServerObject(QStringList *addys, Scheduler *joblist)

{
	if (addys) clientlist=*addys;
	if (joblist)
		jobs=joblist;
	else jobs=new Scheduler;
	// this loop sets up a thread for each host in clientlist/addresses
	QString temp;
	QString hostname;
	int socket;
	if (!clientlist.isEmpty()) {
		while ((!clientlist.isEmpty())&&(threadlist.count() < 400)){
			temp=clientlist.front();
			clientlist.pop_front();
			socket = temp.section(":", 1, 1).toInt();
			hostname = temp.section(":", 0, 0);
			temp.append(suffix);
			if (clientlist.grep(temp).isEmpty())
			{
				threadlist.append(new Thread(this, socket));
				threadlist.current()->setHost(hostname);
				if (clientlist.grep(temp).isEmpty()) clientlist.push_back(temp);
			}
		} 
	}
	started = false;
	unpaused=true;
	connect(jobs, SIGNAL(changed(Job*)), this, SLOT( statusUpdate(Job*)));
	
}

ServerObject::~ServerObject() { delete jobs; }

/*
bool ServerObject::parser(int& stFrame, int& enFrame, int& frInterval, QString& projectName, QString fn) {
	Client steve;
	return (steve.parse(stFrame, enFrame, frInterval, projectName, fn));
}
*/

void ServerObject::setClients(QStringList addys)
{
	QStringList connected_list;
	if (!threadlist.isEmpty())
	{
		threadlist.at(0);
		do
		{
			threadlist.current()->setHost(threadlist.current()->getHost());
			connected_list.push_back(threadlist.current()->getHost());
		}
		while(threadlist.next());
	}
	QString temp;
	QString hostname;
	int socket;
	if (!addys.isEmpty()) {
		while ((!addys.isEmpty())&&(threadlist.count() < 400)){
			temp=addys.front();
			addys.pop_front();
			socket = temp.section(":", 1, 1).toInt();
			hostname = temp.section(":", 0, 0);
			hostname.append(suffix);
			if (clientlist.grep(temp).isEmpty() || connected_list.grep(temp).isEmpty())
			{
				threadlist.append(new Thread(this, socket));
				threadlist.current()->setHost(hostname);
				if (clientlist.grep(temp).isEmpty()) clientlist.push_back(temp);
			}
		} 
	}
}

bool ServerObject::AddJobToScheduler(Job * job)
{	
	return jobs->addJob(job);
}

void ServerObject::startJobs()
{
	started = true;
}

void ServerObject::setPriority(uint JobId,int newPriority)
{
	jobs->setPriority(JobId,newPriority);
}

void ServerObject::pauseJob()
{
	if(started)
	{
		if (unpaused)
		{
			if (!threadlist.isEmpty())
			{
				threadlist.at(0);
				do
				{
					threadlist.current()->getClient()->render(escchar);
					threadlist.current()->stop();
				}
				while(threadlist.next());
			}
			unpaused=false;
		}
		else
		{
			threadlist.at(0);
			if (!threadlist.isEmpty())
			{
				do
				{
					threadlist.current()->start();					
				}
				while(threadlist.next());
			}
			unpaused=true;
		}
	}
}

bool ServerObject::removeJob(uint JobId)
{
	if (jobs->endJob(JobId))
	{
		if (!threadlist.isEmpty())
		{
			threadlist.at(0);
			do
			{
				if (JobId==threadlist.current()->getJobId())
				{
					threadlist.current()->getClient()->render(escchar);
				}
			}while(threadlist.next());
		}
		return true;
	}
	return false;
}

void ServerObject::continueRender(const QString& str, QThread* thread)
{
	Thread* thr = (Thread*)thread;
	emit finished(thr->getHost().append(" ").append(str));
	thr->start();
}

void ServerObject::echo(const QString& str, QThread* thread)
{
	Thread* thr = (Thread*)thread;
	emit finished(thr->getHost().append(" ").append(str));
}

void ServerObject::jobFailed(const QString& str, QThread* thread)
{
	Thread* thr = (Thread*)thread;
	WorkItem *nextItem;
	if (unpaused) 
	{
		//jobs->renderFailed(((Thread*)thread)->getInfo());
		uint jobid;
		nextItem = jobs->getItem(&jobid);
		
		
		// here is where we queue in the next job in the list
		if (nextItem) {
			emit finished(thr->getHost().append(" ").append(str));
			thr->setJobId(jobid);
			thr->setJobName(nextItem->getJob()->getJobName());
			thr->setInfo(nextItem);
			thr->start();
		}
		else{
			emit finished(thr->getHost().append(" ").append(str));
			emit finished(tr("All jobs finished on ").append(thr->getHost()));
		}
	}
	else {
		emit finished(thr->getHost().append(" ").append("paused"));
	}
}

void ServerObject::jobDone(const QString& str, QThread* thread)
{
	if (unpaused) {		
		Thread* thr = (Thread*)thread;
		WorkItem *nextItem;
		jobs->renderFinished(((Thread*)thread)->getInfo());
		uint jobid;
		nextItem=jobs->getItem(&jobid);
		//thr->FilePath(jobs->get
		// here is where we queue in the next job in the list
		if (nextItem) {
			emit finished(thr->getHost().append(" ").append(str));
			thr->setJobId(jobid);
			thr->setJobName(nextItem->getJob()->getJobName());
			thr->setInfo(nextItem);
			thr->start();
		}
		else {
			emit finished(thr->getHost().append(" ").append(str));
			emit finished(tr("All jobs finished on ").append(thr->getHost()));
		}
	}
}

void ServerObject::statusUpdate(Job* job){
	emit changed(job, job->getJobName(), QString("done"), 100);
}

void ServerObject::statusUpdate(const QString& stat, const QThread* thr,int per){
	if (stat.find("Aborted")==-1)
	{
		if(((Thread*)thr)->getInfo()->getJob())
		{
			QString a=((Thread*)thr)->getClient()->peerAddress().toString();
			QString send=((Thread*)thr)->getInfo()->getJob()->getJobName()+'?'+stat;
			emit changed(NULL,a,send,per);
		}
	}	
}

void ServerObject::couldntConnect(const QString& str, const QThread* thr)
{
	if (((Thread*)thr)->getInfo())
	{
		jobs->renderFailed(((Thread*)thr)->getInfo());
	}
	emit connectChange(str);
	threadlist.removeRef((Thread*)thr);
}

void ServerObject::disconnect(const QString& str, const QThread* thr)
{
	if (((Thread*)thr)->getInfo())
	{
		jobs->renderFailed(((Thread*)thr)->getInfo());
	}
	emit connectChange(str);
	threadlist.removeRef((Thread*)thr);
}

void ServerObject::connected(const QString& str, const QThread* thr)
{
	emit connectChange(str);
	Thread* thread = (Thread*)thr;
	WorkItem *nextItem;
	if (started) {
		thread->setReady(true);
		uint jobid;
		nextItem = jobs->getItem(&jobid);
		if (nextItem) {
			thread->setJobId(jobid);
			thread->setJobName(nextItem->getJob()->getJobName());
			thread->setInfo(nextItem);
			thread->start();
		}
	}
	else
		thread->setReady(true);
}


