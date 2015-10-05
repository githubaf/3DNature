#include <qapplication.h>
#include <qstringlist.h>
#include "mainwindow.h"
#include "server.h"
#include "datatypes.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
	SCMessageList* programMessages = new SCMessageList;
    *programMessages << (QString)"Superconductor Log initialized";
	SCServer* aServer = new SCServer( programMessages );
    SCMain* mw = new SCMain();
	mw->setMessages( programMessages );
    mw->setCaption( "SuperConductor 2" );
	mw->setServer( aServer );
    mw->show();
	a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
