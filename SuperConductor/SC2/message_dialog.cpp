/****************************************************************************
** Form implementation generated from reading ui file 'superconductor_messages.ui'
**
** Created: Fri Jul 11 12:57:52 2003
**      by: The User Interface Compiler ($Id: message_dialog.cpp,v 1.2 2003/09/11 23:03:53 pitabred Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include <qvariant.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "message_dialog.h"

/* 
 *  Constructs a Superconductor_Messages as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
Superconductor_Messages::Superconductor_Messages( SCMessageList* sl, QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
	if ( sl )
	strings = sl;
	
    if ( !name )
	setName( "Superconductor_Messages" );

	layout = new QGridLayout( this, 2, 2 );
//	layout->setAutoAdd( TRUE );
	layout->setResizeMode( QLayout::FreeResize );

    listBox = new QListBox( this, "listBox" );
    listBox->setGeometry( QRect( 10, 10, 291, 461 ) );
    listBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, listBox->sizePolicy().hasHeightForWidth() ) );
    listBox->setSelectionMode( QListBox::NoSelection );
    listBox->setVariableWidth( TRUE );	
    languageChange();
    resize( QSize(314, 480).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
	
	save = new QPushButton( "Save", this );
	closeButton = new QPushButton( "Close", this );
	
	layout->addMultiCellWidget( listBox, 0, 0, 0, 1 );
	layout->addWidget( save, 1, 0 );
	layout->addWidget( closeButton, 1, 1 );

	connect( save, SIGNAL( clicked() ), this, SLOT( saveOutput() ) );
	connect( closeButton, SIGNAL( clicked() ), this, SLOT( closeDialog() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Superconductor_Messages::~Superconductor_Messages()
{
    // no need to delete child widgets, Qt does it all for us
}

void Superconductor_Messages::closeDialog() {
	this->close();
}

/*
 *  Saves the selected messages into a file, all if none selected
 */
void Superconductor_Messages::saveOutput()
{
	QString s = QFileDialog::getSaveFileName(
                    "/home",
                    "Log files (*.log)",
                    this,
                    "save file dialog"
                    "Choose a filename to save under" );
	if ( s ) {
		QFile aFile( s );
		QTextStream stream;
		QString output;
		if ( strings ) {
			output = strings->join( "\n" );
			output += "\n";
		}
		else {
			output = " ";
		}
		if ( aFile.exists() ) {
			switch( QMessageBox::warning( this, "Log Save",
	        "This file already exists. Overwrite?\n",
			"Yes",
	        "No", 0, 0, 1 ) ) {
 			case 0:  // user clicked ok... overwrite file
				aFile.open( IO_WriteOnly | IO_Truncate | IO_Raw );
				aFile.at( 0 );
				stream.setDevice( &aFile );
				stream << output;
				stream.unsetDevice();
				aFile.close();
				this->close();
				break;
			case 1: // The user clicked the Quit or pressed Escape
				break;
			default:
				break;
			}
		}
		else {  // file doesn't exist, just write it
			if ( !s.endsWith( (QString)".log" )) {
				s.append( ".log" );
				aFile.setName( s );
			}
			aFile.open( IO_WriteOnly | IO_Truncate | IO_Raw );
			aFile.at( 0 );
			stream.setDevice( &aFile );
			stream << output;
			stream.unsetDevice();
			aFile.close();
			this->close();
		}
	}
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Superconductor_Messages::languageChange()
{
    setCaption( tr( "Superconductor Messages" ) );
    listBox->clear();
	if ( strings ) { listBox->insertStringList( *strings ); }
}

