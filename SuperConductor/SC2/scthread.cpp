#include "scthread.h"

// First the thread class
SCThread::SCThread() {
	// no need for any initialization here, shouldn't do this in general usage
}

SCThread::SCThread(SCClient* cli, SCServer* srv, SCProject* prj ) {
	client = cli;
	server = srv;
	project = prj;
}

void SCThread::run() {
	// load a project and start it running... we can to the halting, re-loading, etc. in customEvent
	// get info from client for first render
	if ( client && server && project ) {
		if (!client->load_status())
			client->load( project->fn() );
		else
			client->render( project->name(), client->getFrame()->number());
		// when this call returns, frame has been rendered
	}
}
