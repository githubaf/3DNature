// this file holds the definitions for the client data
// transfer structures

#ifndef CLIENTDATATRANSFER_H
#define CLIENTDATATRANSFER_H

#include <qstring.h>

// renderInfo is a non-inheriting class
// that just has public data members
// Information:
class renderInfo {
public:
	int priority;
	int frameInterval;
	int startFrame;
	int endFrame;
	QString jobName;
	renderInfo* next;
	renderInfo(renderInfo* nextRI=NULL) {
		next = nextRI;
		priority = -1; frameInterval = 0;
		startFrame = 0; endFrame = 0;
		jobName = "";
	}
	renderInfo(int pri, int fi, int sf, int ef, QString jn, renderInfo* nextRI=NULL)
		: priority(pri), frameInterval(fi), startFrame(sf), endFrame(ef), jobName(jn), next(nextRI)
	{ }
	// equals and copy constructor so that
	// it can be used to make a real/deep
	// copy in the collection class easier
	// they also act as nodes in the
	// collectedJobs class
	renderInfo(renderInfo& b) {
		priority = b.priority;
		frameInterval = b.frameInterval;
		startFrame = b.startFrame;
		endFrame = b.endFrame;
		jobName = b.jobName;
	}
	renderInfo& operator=(renderInfo& b) {
		priority = b.priority;
		frameInterval = b.frameInterval;
		startFrame = b.startFrame;
		endFrame = b.endFrame;
		jobName = b.jobName;
		return *this;
	}
};

// this class is basically a linked list class
// holding all of the renderInfo's you send to
// it, deep copying all their data
class collectedJobs {
private:
	// current is an iterator, head
	// is pretty self explanatory
	renderInfo* head;
	renderInfo* current;
public:
	// only one constructor
	collectedJobs() {
		head = NULL;
		current = NULL;
	}
	collectedJobs (collectedJobs& b) {
		b.reset();
		current = NULL; head = NULL;
		while(!b.isAtEnd()) {
			addJob(*b.getNextJob());
		}
	}

	~collectedJobs() {
		// need to deallocate all the mem here
		renderInfo* temp;
		while(head != NULL) {
			temp = head;
			head = head->next;
			delete temp;
		}
		head = current = NULL;
	}
	
	collectedJobs& operator=(collectedJobs b) {
		b.reset();
		current = NULL; head = NULL;
		while(!b.isAtEnd()) {
			addJob(*b.getNextJob());
		}
		return *this;
	}
	bool isEmpty() {
		if(head != NULL)
			return false;
		return true;
	}
	bool isAtEnd() {
		if(current != NULL)
			return false;
		else {
			reset();
			return true;
		}
	}
	void addJob(renderInfo obj) {
		// plugs an item in at the head of the list
		// since order is unimportant
		renderInfo* temp = new renderInfo(obj);
		temp->next = head;
		head = temp;
		current = temp;
	}
	// get the next job, 
	renderInfo* getNextJob() {
		if (current == NULL) {
			return current;
		}
		else {
			renderInfo* temp = current;
			current = current->next;
			return temp;
		}
	}
	// this is in case you need to reset the list for
	// whatever reason... error in reporting job or
	// some such thing
	void reset() {
		current = head;
	}
};

#endif // CLIENTDATATRANSFER_H
