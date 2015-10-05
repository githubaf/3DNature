// This is the implementation file for the SCServer

#include "client.h"
#include "datatypes.h"
#include "server.h"
#include "scthread.h"

#ifndef USE_PLUGINS
#include "wcs-client.h"
#endif

// the server class
SCServer::SCServer( SCMessageList* msgs ) {
	// in here we need to (on instantiation) find all the plugins in the
	// ./plugins directory
	if ( msgs ) { messages = msgs; }
	else { messages = new SCMessageList; }
	stuff = "Starting server initialization...";
	*messages << stuff;
	QSettings setting;
	setting.setPath( "3DNature.com", "Superconductor" );
	QString s = setting.readEntry( APP_KEY + "plugins", QDir::currentDirPath() + "/plugins/" );
	QString prev = s;
	QDir d;
	d.setFilter( QDir::Files | QDir::Hidden );
	d.setSorting( QDir::Size | QDir::Reversed );
	QString suffix;
	#if defined(Q_OS_WIN32)
		suffix = ".dll";
	#elif defined(Q_OS_MAC)
		suffix = ".dylib";
	#else // otherwise it's another *nix OS
		suffix = ".so";
	#endif
	*messages << "suffix: " + suffix;
	QFileInfoListIterator* it = NULL;
	const QFileInfoList* list = NULL;
	
	#ifdef USE_PLUGINS
	QFileInfo* fi;
	QLibrary* libPtr;
	while (plugins.count() == 0) {
		d.setPath( s );
		list = d.entryInfoList();
		if (list && (d.exists( s ))) {
			prev = s;
			setting.writeEntry( APP_KEY + "plugins", s );
			it = new QFileInfoListIterator( *list );
			*messages << "Finding libraries...";
			while ((fi = it->current()) != 0 ) {
				if (fi->fileName().endsWith(suffix)) {
					*messages << "Checking: " + fi->fileName();
					// if it's a dynamic lib, try loading it
					libPtr = new QLibrary( fi->filePath() );
					// if not, toss an error to the message list
					createProto cr = (createProto) libPtr->resolve( "create" );
					destroyProto ds = (destroyProto) libPtr->resolve( "destroy" );
					if (!cr) {
						*messages << "Problem resolving \"create\" function in library " + fi->filePath();
						}
					if (!ds) {
						*messages << "Problem resolving \"destroy\" function in library " + fi->filePath();
						}
					if (!ds || !cr) {
						*messages << fi->fileName() + " not working or not a SuperConductor plugin.  Disabled";
						delete libPtr;
					}
					else {
						// print a msg out to cerr (debugging info)
						SCClient* aClient = cr();
						*messages << aClient->identity() + " plugin functioning properly";
						plugins.append(libPtr);
						ds(aClient);
					}
				} // if
				++(*it);
			} // while
			delete it;
		}
		if (!d.exists( s ) || (plugins.count() == 0)) {
			// no plugins found... see if settings object exists
			QMessageBox::warning( NULL, "Superconductor",
				    "No plugins were found in the directory:\n" + s,
				    QMessageBox::Ok, QMessageBox::NoButton );
			s = setting.readEntry( APP_KEY + "plugins", "blank" );
			if ( ( s == prev ) || ( s == "blank" ) ) {
				*messages << "No plugins found";
				s = QFileDialog::getExistingDirectory(
					QDir::homeDirPath(),
					this,
					"Choose plugin directory",
					"Choose a directory containing Superconductor plugins",
					TRUE );
				prev = s;
				if (!s.length()) {
					// no plugin directory selected...
					exit(1);
				}
				else {
					setting.writeEntry( APP_KEY + "plugins", s );
				}
			}
		} // if plugins.count
	} // while (plugins.count() list)
	#endif // USE_PLUGINS
	// libraries are loaded now.  Everything should be good to go.
	// the server contains the master list of the clients
	// resolves destroy in lib and kills the client ref
	clients.setAutoDelete(false);
	*messages << "Server initialized";
}

SCServer::~SCServer() {
	while(!plugins.isEmpty()) {
		plugins.remove();
	}
}

QPtrList<SCProject> SCServer::getProjects() {
	// we need to go through the projects list, getting the extensions
	QString extensions;
	QString csvExt;
	QStringList expanded;
	QString analogy;
	SCClient* temp;
//	QString* extensionArray[plugins.count()];

	#ifdef USE_PLUGINS
	QString** extensionArray = new QString*[plugins.count()];
	createProto cr;
	destroyProto ds;
	QLibrary* lib = plugins.first();
	while (lib) {
		cr = (createProto) lib->resolve( "create" );
		ds = (destroyProto) lib->resolve( "destroy" );
		temp = cr();
		csvExt = temp->extensions();
		extensions += temp->identity() + " (";
		analogy += csvExt + ";";
		// put the string in where it needs to be in this array
		extensionArray[plugins.at()] = new QString(csvExt);
		expanded = QStringList::split(',', csvExt);
		for ( QStringList::Iterator it = expanded.begin(); it != expanded.end(); ++it ) {
			extensions += "*." + *it + " ";
//			"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
		}
		ds(temp);
		lib = plugins.next();
		if (lib) { extensions += ");;"; }
	}
	#else
	temp = (SCClient*)new wcsClient;
	csvExt = temp->extensions();
	extensions += temp->identity() + " (";
	analogy += csvExt + ";";
	// put the string in where it needs to be in this array
	expanded = QStringList::split(',', csvExt);
	for ( QStringList::Iterator it = expanded.begin(); it != expanded.end(); ++it ) {
		extensions += "*." + *it + " ";
	}
	delete temp;
	#endif // USE_PLUGINS
	extensions += ")";
	QSettings setting;
	setting.setPath( "3DNature.com", "Superconductor" );
	QString s = setting.readEntry( APP_KEY + "fileopen", QDir::homeDirPath() );
	s = QFileDialog::getOpenFileName(
                    s,
                    extensions,
                    this,
                    "open file dialog"
                    "Choose a file" );
	*messages << "Project filename: " + s;
	// we now have the filename... we just need to know what plugin to use it with
	extensions = s.section('.', -1);
	bool found = false;
	unsigned int index = 0;

	#ifdef USE_PLUGINS
	while ((!found) && (index < plugins.count())) {
		if (extensionArray[index]->contains(extensions)) {
			found = true;
		}
		else {
			++index;
		}
	}
	#endif // USE_PLUGINS
	collectedSCProjects* c;
	QPtrList<SCProject> returnSCProjects;
	c = new collectedSCProjects;
	#ifdef USE_PLUGINS
	if ((index < plugins.count()) && !s.isEmpty()) {
		// there's an ending slash, so use -2 instead of -1 as end
		setting.writeEntry( APP_KEY + "fileopen", s.section(QDir::separator(), 0, -2 ));
		lib = plugins.at(index);
		cr = (createProto) lib->resolve( "create" );
		ds = (destroyProto) lib->resolve( "destroy" );
		temp = cr();
		*c = temp->parse(s);
		if (c->isEmpty()) {
			*messages << "No projects returned";
			}
		else {
			renderInfo* ri;
			while ((ri = c->getNextSCProject())) {
				SCProject* newSCProject = new SCProject( ri, lib, s );
				projects.append( newSCProject );
				returnSCProjects.append( newSCProject );
			}
		}
		ds(temp);
	}
	#else
	setting.writeEntry( APP_KEY + "fileopen", s.section(QDir::separator(), 0, -2 ));
	temp = (SCClient*) new wcsClient;
	*c = temp->parse(s);
	if (c->isEmpty()) {
		*messages << "No projects returned";
	}
	else {
		renderInfo* ri;
		while ((ri = c->getNextSCProject())) {
			SCProject* newSCProject = new SCProject( ri, NULL, s );
			projects.append( newSCProject );
			returnSCProjects.append( newSCProject );
		}
	}
	delete temp;
	#endif // USE_PLUGINS
	delete c;
	// a collected SCProjects object is the collection of all the projects existing in a file
	return returnSCProjects;
}

QPtrList<SCClient> SCServer::getClients( SCProject* prj ) {
	QPtrList<SCClient> retClients;
	QSettings setting;
	setting.setPath( "3DNature.com", "Superconductor" );
	QString fp = setting.readEntry( APP_KEY + "clsopen", QDir::homeDirPath() );
	QString s = QFileDialog::getOpenFileName(
			fp,
			"Client Lists (*.cls)",
			this,
			"open file dialog",
			"Choose a file to open" );
	if ( s && prj ) {
		// filename returned, and project exists
		setting.writeEntry( APP_KEY + "clsopen", s.section(QDir::separator(), 0, -2 ));
		QFile aFile( s );
		QString buffer( "" );
		QString section;
		QStringList fileLine;
		if(aFile.open(IO_ReadOnly))	{
			QTextStream stream( &aFile );
			buffer = stream.readLine();
			section = prj->libString();
			// section will be null because of QString constructor if not
			// manipulated in if block
			if ( buffer != section ) {
				// no addition of clients... incorrect list
				QMessageBox::warning ( this, "Client mismatch", "The project type does not match the saved client list type"
										, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
			}
			else {
				// list ok... load them in
				buffer = stream.readLine();
				while( !buffer.isNull() ) {
					// clientName;;settingName;setting;;settingName;setting;;...
					fileLine = fileLine.split( ";;", buffer );
					// loop on next entries
					SCClient* newCli = NULL;
					// first entry is client name
					newCli = newClient( prj, fileLine[0], false);
					QPtrList<Setting> cliSettings;
					if (newCli) {
						for (uint i = 1; i <= fileLine.count()-1; ++i) {
							if ( fileLine[i].section(';',0,0) != "") {
								cliSettings.append( new Setting( fileLine[i].section(';', 0, 0),
															fileLine[i].section(';', 1, 1) ));
							}
						}
						newCli->setConfig( cliSettings );
						buffer = fileLine.first();
						newCli->setName( buffer );
						clients.append( newCli );
						prj->addClient( newCli );
						retClients.append( newCli );
					}
					buffer = stream.readLine();
				} // while( buffer )
			} // buffer != section
		} // aFile.open
	} // if s
	return retClients;
}

void SCServer::saveClients( SCProject* prj ) {
	QString s = QFileDialog::getSaveFileName(
				"/home",
				"Client Lists (*.cls)",
				this,
				"save file dialog"
				"Choose a filename to save under" );
	if ( s ) {
		bool prompt = TRUE;
		QFile aFile( s );
		QTextStream stream;
		QString output = "";
		SCClient* clientPtr = NULL;
		QPtrList<Setting> settings;
		Setting* aSetting;
		QPtrList<SCClient> pcli = prj->clientList();
		clientPtr = pcli.first();
		if ( clientPtr ) {
			output = clientPtr->identity() + (QString)"\n";
		}
		while ( clientPtr ) {
			settings = clientPtr->getConfig();
			output += clientPtr->getName() + (QString)";;";
			for ( aSetting = settings.first(); aSetting; aSetting = settings.next() ) {
				output += aSetting->name() + (QString)";" + aSetting->value() + (QString)";;";
			}
			output += (QString)"\n";
			clientPtr = pcli.next();
		}
		if ( aFile.exists() ) {
			switch( QMessageBox::warning( this, "Client List Save",
	        "This file already exists. Overwrite?\n",
			"Yes",
    	    "No", 0, 0, 1 ) ) {
				case 0:  // user clicked ok... overwrite file
				aFile.open( IO_WriteOnly | IO_Truncate );
				stream.setDevice( &aFile );
				stream << output;
				stream.unsetDevice();
				aFile.close();
				break;
			case 1: // The user clicked the Quit or pressed Escape
			default:
				prompt = FALSE;
				break;
			}
		}
		if (( output == "" )&& prompt ) {
			switch( QMessageBox::warning( this, "Client List Save",
	        "This file will be BLANK.  Continue?\n",
			"Yes",
    	    "No", 0, 0, 1 ) ) {
				case 0:  // user clicked ok... overwrite file
				aFile.open( IO_WriteOnly | IO_Truncate );
				stream.setDevice( &aFile );
				stream << output;
				stream.unsetDevice();
				aFile.close();
				break;
			case 1: // The user clicked the Quit or pressed Escape
				break;
			default:
				break;
			}
		}
		if (!aFile.exists() && (output != "")) {  // file doesn't exist, just write it
			if ( !s.endsWith( (QString)".cls" )) {
				s.append( ".cls" );
				aFile.setName( s );
			}
			aFile.open( IO_WriteOnly | IO_Truncate );
			stream.setDevice( &aFile );
			stream << output;
			stream.unsetDevice();
			aFile.close();
		}
		else {
			*messages << "No file saved";
		}
	}
}

bool SCServer::removeProject( SCProject* prj ) {
	// need to see if job is currently running, stop it, THEN remove
	if ( prj->isStopped() ) {
		projects.removeRef( prj );
		// go through and remove all non-duplicated clients fully
		QPtrList<SCClient> thclients = prj->clientList();
		SCClient* cli = thclients.first();
		while ( cli ) {
			removeClient( prj, cli );
			cli = thclients.next();
		}
		delete prj;
		return true;
	}
	return false;
}

bool SCServer::run( SCProject* prj ) {
	// Threads are cheap.  Make a new one with the info in it for the current frame each iteration
	if ( prj->status() == SCProject::paused ) {
		unpause( prj );
	}
	else if ((prj->clientList().count() > 0)&&( prj->statusValue() == SCProject::stopped) ) {
		balance();
		// create a new thread for each client in the list, put it into the thread list
		QPtrList< SCClient > clients = prj->clientList();
		SCClient* tempClient = clients.first();
		while (tempClient) {
			if (!tempClient->active()) {
				tempClient->setActive( true );
				// client has current frame?
				SCFrame* aFrame;
				aFrame = prj->getNextFrame();
				tempClient->setFrame( aFrame );
				if ( cliThreadMap.find(tempClient) != cliThreadMap.end() ) {
					delete cliThreadMap[tempClient];
					cliThreadMap.erase( tempClient );
				}
				SCThread* tthrd = new SCThread( tempClient, this, prj );
				// Gotta map 'em all!
				cliThreadMap.insert( tempClient, tthrd );
				aFrame->startMark();
				tthrd->run();
			}
			tempClient = clients.next();
		}
		prj->setStatus( SCProject::running );
		*messages << (QString)"Project " + prj->name() + (QString)" started";
	}
	else {
		*messages << (QString)"Project " + prj->name() + (QString)" not started because there are no clients attached";
		return false;
	}
	return true;
}

bool SCServer::pause( SCProject* prj ) {
	// pause project... reassign clients if necessary
	if ( prj->status() == SCProject::paused ) {
		unpause( prj );
	}
	else if ((prj->clientList().count() > 0)&&( prj->statusValue() == SCProject::running) ) {
		// go through the clients pausing them.  Leave the threads alive
		prj->setStatus( SCProject::paused );
		*messages << (QString)"Project " + prj->name() + (QString)" paused";
	}
	else {
		return false;
	}
	return true;
}

void SCServer::unpause( SCProject* prj ) {
	// start all clients back up
	// threads should still be running
	prj->setStatus( SCProject::running );
	*messages << (QString)"Project " + prj->name() + (QString)" unpaused";
}

bool SCServer::stop( SCProject* prj ) {
	// stop the threads related to the clients in this project
	// kill/delete all the threads
	// Should this reset all frames?
	if (prj->statusValue() == SCProject::finished) {
		return false;
	}
	if ((prj->clientList().count() > 0)&&( prj->statusValue() != SCProject::stopped)) {
		// create a new thread for each client in the list, put it into the thread list
		QPtrList< SCClient > clients = prj->clientList();
		SCClient* tempClient = clients.first();
		while (tempClient) {
			tempClient->halt();
			prj->frameFailed(tempClient->getFrame());
			tempClient->setActive( false );
			if ( cliThreadMap.find(tempClient) != cliThreadMap.end() ) {
				cliThreadMap.erase( tempClient );
				delete cliThreadMap[tempClient];
			}
			tempClient = clients.next();
		}
	}
	prj->setStatus( SCProject::stopped );
	*messages << (QString)"Project " + prj->name() + (QString)" stopped";
	return true;
}

SCClient* SCServer::newClient( SCProject* prj , QString name, bool warn ) {
	SCClient* myNewClient = NULL;
	if( prj && !prj->hasClient( name )) {
		SCProject* tprj;
		bool addIt = true;
		bool tst;
		for (tprj = projects.first(); tprj; tprj = projects.next()) {
			// test to see if client name exists on any other project
			// if so, ask to add, break
			if (tprj->hasClient( name ) && tprj != prj ) {
				addIt = false; // automatic short-circuit... no adding unless in dialog
				if (warn) { // just skip adding the client if warnings are enabled
					tst = !QMessageBox::question(
			            this,
        			    tr("Client Exists - Superconductor 2"),
            			tr(QString("This client exists on another project:\n")+
							tprj->name() +
							QString("\nAdd it to ")+ prj->name() + QString( " as well?")),
						tr("&Yes"), tr("&No"),
						QString::null, 0, 1 );
				}
				else { tst = true; }
				if (tst) {
					// append to the current project, no new client
					myNewClient = tprj->getNamedClient( name );
					if (myNewClient) {
						prj->addClient( myNewClient);
						// no need to add to client list or threads
						// but we do need to set prj as it's active project
						// FIXME
						// here is where we add a load-balancing call
						myNewClient->setProject( prj );
					}
                    else {
                        *messages << (QString)"Error getting named client";
                    }
				}
			} // hasClient(name)
		}  // for
		if (addIt) {
			// add the client to this project
			#ifdef USE_PLUGINS
            createProto cr = (createProto) prj->lib()->resolve( "create" );
			myNewClient = cr();
			#else
			myNewClient = (SCClient*) new wcsClient;
			#endif
			myNewClient->setProject( prj );
			myNewClient->setName( name );
			myNewClient->setActive( false );
			prj->addClient( myNewClient );
			// connect the client's signals to the server's slots
			myNewClient->connectSignals( (QObject*)this );
			// and created a new client, added it to the list
			clients.append( myNewClient );
		}
	}  // if prj && hasClient
	else {
		myNewClient = NULL;
		if ( prj->hasClient( name )) {
			*messages << (QString)"Client already exists on this project";
		}
		else {
			*messages << (QString)"Bad project";
		}
	}
	return myNewClient;
}

bool SCServer::removeClient( SCProject* prj, SCClient* client) {
	// see if any project has this client in it
	// this logic is here for when multiple clients can "share" between projects
	bool multPrj = false;
	if (projects.count() && prj->hasClient( client ) && prj->isStopped()) {
		projects.first();
		do {
			if (( prj != projects.current() )&&(projects.current()->hasClient( client ))) {
				// we found a client in multiple projects... set it to the other project
				client->setProject( projects.current() );
				multPrj = true;
			}
		} while (projects.next());
		if ( multPrj ) {
			// just remove it from project prj
			prj->dropClient( client );
		}
		else {
			// only instance of the client... kill it all the way (and it's thread)
			prj->dropClient( client );
			// find the associated thread ( it will be stopped )
			if ( cliThreadMap[client] ) {
				// if we found the thread for this client
				delete cliThreadMap[client];
			}
			#ifdef USE_PLUGINS
			destroyProto ds = (destroyProto) prj->lib()->resolve( "destroy" );
			ds(client);
			#else
			delete client;
			#endif // USE_PLUGINS
		}
		return true;
	}
	return false;
}

bool SCServer::onMultipleProjects( SCClient* cli ) {
	bool ret = false;
	SCProject* prjTmp;
	for( prjTmp = projects.first(); prjTmp; prjTmp = projects.next() ) {
		if (prjTmp->hasClient( cli ) ) {
			for ( ; prjTmp; prjTmp = projects.next() ) {
				if ( prjTmp->hasClient( cli )) {
					// found on multiple projects
					ret = true;
					// short circuits for loop
					prjTmp = NULL;
				} // if
			} // for
			// if the project was found once, no more loops needed
			// just the internal one
			prjTmp = NULL;
		} // if
	} // for
	return ret;
}


void SCServer::balance() {
}

double SCServer::score( SCProject* prj ) {
	// calculates a relative heuristic score for a given project
	// number of assigned clients/priority
	// This number should be LARGER for aq higher-priority project (when testing)
	int count = 0;
	SCClient* tmp;
	QPtrList<SCClient> cl = prj->clientList();
	for ( tmp = cl.first(); tmp; tmp = cl.next()) {
		if( tmp->getProject() == prj)
			++count;
	}
	// priority could be 0!!  FIXME
	return ((double)count/(double)prj->priority()+1);
}

bool SCServer::moveClient( SCProject* prjA, SCProject* prjB) {
	return true;
}

void SCServer::loaded(const QString& string, const SCClient* constclient) {
	SCClient* client = (SCClient*)constclient;
	// now we need to start the first thread
	*messages << client->getName() + (QString)": " + string;
	SCProject* project = client->getProject();
	SCFrame* frame = client->getFrame();
	if (project->statusValue() == SCProject::running ) {
		if ( cliThreadMap.find(client) != cliThreadMap.end() ) {
			delete cliThreadMap[client];
			cliThreadMap.erase( client );
		}
		if (!frame)
			frame = project->getNextFrame();
		if ( frame ) {
			// we have a valid frame, make the thread and all that
			client->setFrame( frame );
			SCThread* tthrd = new SCThread( client, this, project );
			frame->startMark();
			cliThreadMap.insert( client, tthrd );
			tthrd->run();  // start the just created thread
		}
		else {
			// no more frames, project has scheduled all the frames it can
			client->setActive( false );
			// no frames active in project... 'tis done!
			project->setStatus( SCProject::finished );
		}
	} // SCProject::running
}

void SCServer::changed(const QString& string, const SCClient* constclient) {
	// should just be a status update... similar to echo
	SCClient* client = (SCClient*)constclient;
	*messages << client->getName() + (QString)": " + string;
}

void SCServer::echo(const QString& string, const SCClient* constclient) {
	// dump the message to messages stream
	SCClient* client = (SCClient*)constclient;
	*messages << client->getName() + (QString)": " + string;
}

void SCServer::halted(const QString& string, const SCClient* constclient) {
	// client has successfully halted... do we need to do anything?
	SCClient* client = (SCClient*)constclient;
	*messages << client->getName() + (QString)": " + string;
}

void SCServer::finished(const QString& string, const SCClient* constclient) {
	frameDone( string, constclient, false ); // false on fail frame
}

void SCServer::failed(const QString& string, const SCClient* constclient) {
	frameDone( string, constclient, true ); // true on fail frame
}

void SCServer::frameDone(const QString& string, const SCClient* constclient, bool failed) {
	// a frame finished, kill the thread, start a new one with the next frame
	// FIXME: update the time estimate and such in the main window
	SCClient* client = (SCClient*)constclient;
	*messages << client->getName() + (QString)": " + string;
	SCProject* project = client->getProject();
	SCFrame* frame = client->getFrame();
	frame->endMark();
	if (client->active()) {
		if (failed)
			project->frameFailed( frame );
		else
			project->frameFinished( frame );
	}
	if (!client->irrecoverable()) {
		if (project->statusValue() == SCProject::running ) {
			if ( cliThreadMap.find(client) != cliThreadMap.end() ) {
				delete cliThreadMap[client];
				cliThreadMap.erase( client );
			}
			frame = project->getNextFrame();
			if ( (int)frame != 0 ) {
				// we have a valid frame, make the thread and all that
				client->setFrame( frame );
				SCThread* tthrd = new SCThread( client, this, project );
				frame->startMark();
				cliThreadMap.insert( client, tthrd );
				tthrd->run();  // start the just created thread
			}
			else {
				// no more frames, project has scheduled all the frames it can
				client->setActive( false );
				// no frames active in project... 'tis done!
				*messages << project->name() + (QString)": Rendering finished";
				// FIXME: Throw failed frames to messages (if they exist)
				*messages << project->name() + (QString)" failed frames: " + stuff.setNum(project->failCount());
				*messages << project->name() + (QString)" finished frames: " + stuff.setNum(project->finishCount());
				*messages << project->name() + (QString)" new  frames: " + stuff.setNum(project->newCount());
				project->setStatus( SCProject::finished );
				emit projectFinished( project );
			}
		} // SCProject::running
		emit updateDisplay(project);
	} // irrecoverable
	else {
		client->setActive( false );
		QPtrList<SCClient> t_list = project->clientList();
		bool active = false;
		// make client unused, possibly stop project
		if ( cliThreadMap.find(client) != cliThreadMap.end() ) {
			delete cliThreadMap[client];
			cliThreadMap.erase( client );
		}
		for( t_list.first(); t_list.current(); t_list.next() ) {
			if (t_list.current()->active())
				active = true;
		}
		if (!active) {
			project->setStatus( SCProject::error );
		}
		emit updateDisplay( project );
	}
	// otherwise, the client is just in limbo
}