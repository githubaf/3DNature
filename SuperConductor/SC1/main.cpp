// main.cpp

#include <qapplication.h>
#include "controller.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Controller *w = new Controller;
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
	w->setCaption("SuperConductor");
	a.setMainWidget(w);
    w->show();
    return a.exec();
}


