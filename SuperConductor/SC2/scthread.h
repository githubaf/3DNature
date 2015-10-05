#ifndef SCTHREAD_H
#define SCTHREAD_H

#include <qthread.h>
#include "client.h"
#include "datatypes.h"

class SCServer;

/* SCThread class, used to control clients in a threaded manner, non-blocking
 * for GUI operations
 */
class SCThread : public QThread
{
public:
	SCThread();
	SCThread(SCClient*, SCServer*, SCProject*);

	void run();

private:

	// SCServer pointer is used as the receiver of events in postEvent method
	SCClient* client;
	SCServer* server;
	SCProject* project;
};

#endif // SCTHREAD_H
