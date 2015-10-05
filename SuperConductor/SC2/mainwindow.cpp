/****************************************************************************
** Form implementation generated from reading ui file 'ui/BaseSC.ui'
**
** Created: Mon Jan 3 13:43:18 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mainwindow.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtable.h>
#include <qlistbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

#include <qinputdialog.h>
#include "mainwindow.h"
#include "message_dialog.h"

static const unsigned char image0_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a, 0xf4, 0x00, 0x00, 0x01,
    0x52, 0x49, 0x44, 0x41, 0x54, 0x58, 0x85, 0xed, 0x57, 0x3b, 0x6e, 0xc3,
    0x30, 0x0c, 0xa5, 0x8a, 0x4c, 0x19, 0x7d, 0x1b, 0x73, 0x0b, 0xd0, 0x29,
    0xf0, 0x8d, 0x0a, 0x4f, 0x46, 0x6e, 0x64, 0x74, 0xea, 0x4c, 0x9f, 0x21,
    0x97, 0xc8, 0x15, 0xd4, 0x49, 0xaa, 0x42, 0xd3, 0x12, 0x49, 0x23, 0x75,
    0x87, 0xbe, 0xd1, 0xb0, 0xf9, 0x1e, 0x7f, 0x4f, 0x72, 0x20, 0x22, 0x38,
    0x12, 0x6f, 0x87, 0xb2, 0xff, 0x05, 0x01, 0x27, 0xcf, 0x47, 0x38, 0x4c,
    0x11, 0x00, 0x00, 0x1e, 0x9f, 0x4f, 0xcf, 0x89, 0x96, 0x60, 0x8d, 0x15,
    0xac, 0x33, 0x80, 0xc3, 0x14, 0x39, 0x31, 0x87, 0x45, 0x88, 0xa9, 0x05,
    0x1a, 0x72, 0x00, 0x00, 0xc4, 0x3e, 0x6a, 0x63, 0xaa, 0x2b, 0x20, 0x91,
    0xf3, 0x4c, 0x39, 0xb1, 0xa6, 0x12, 0xae, 0x21, 0x24, 0x5a, 0x82, 0x14,
    0xbc, 0x25, 0xc8, 0x2d, 0xa0, 0xcc, 0xde, 0x33, 0x68, 0xbb, 0x05, 0x98,
    0xd0, 0x5d, 0x4d, 0xaf, 0x9b, 0xd7, 0x90, 0xaf, 0xe0, 0xde, 0x8a, 0x34,
    0x05, 0x64, 0xc2, 0x04, 0x36, 0x88, 0xa9, 0xcf, 0x5e, 0x21, 0x55, 0x01,
    0xb9, 0xf7, 0x45, 0x59, 0x7f, 0x6d, 0x06, 0x9e, 0x06, 0x6f, 0x1e, 0x83,
    0xa6, 0xb7, 0x7c, 0x55, 0xcf, 0x97, 0xbb, 0x5f, 0xc0, 0x56, 0x8f, 0x2d,
    0x26, 0xa3, 0x81, 0x28, 0x60, 0xd5, 0x77, 0x05, 0x6a, 0x2e, 0x89, 0xc3,
    0x14, 0xb7, 0x84, 0x57, 0xd7, 0xb0, 0xcc, 0x3e, 0xb7, 0xa1, 0xbb, 0xaa,
    0xaa, 0x70, 0xbe, 0xdc, 0xe1, 0xeb, 0xf6, 0x70, 0x3a, 0xe1, 0x46, 0x26,
    0x34, 0x8f, 0x62, 0x40, 0xa9, 0xf7, 0x25, 0x79, 0x12, 0x2f, 0x09, 0x37,
    0x1b, 0x51, 0x2d, 0x98, 0x07, 0xbe, 0xb3, 0x60, 0x1e, 0xf3, 0x59, 0xd0,
    0xca, 0x9e, 0x83, 0x0b, 0xdf, 0x65, 0xc5, 0xda, 0xe3, 0xb9, 0x06, 0xb7,
    0x00, 0x89, 0x5c, 0x3b, 0x78, 0x25, 0xea, 0x4e, 0x88, 0x7d, 0xdc, 0x34,
    0x20, 0x03, 0x79, 0xad, 0x52, 0xa2, 0x00, 0xa2, 0x25, 0xe4, 0x5e, 0x29,
    0x4a, 0x9c, 0x1c, 0x0f, 0xb1, 0x8f, 0x2b, 0xe3, 0x6a, 0xb4, 0xc9, 0x75,
    0x29, 0xe5, 0xe4, 0x29, 0x73, 0x44, 0xf8, 0x31, 0x9c, 0x54, 0xb9, 0x46,
    0x02, 0xcd, 0x2b, 0x99, 0xb4, 0x6e, 0xa5, 0xc7, 0xf3, 0xb2, 0xb7, 0xd6,
    0x93, 0x57, 0x48, 0x75, 0x27, 0x7c, 0xff, 0xe8, 0x56, 0x41, 0x2d, 0xab,
    0x26, 0x11, 0x9b, 0x04, 0xbc, 0x12, 0x87, 0xff, 0x19, 0xfd, 0x0b, 0x38,
    0x5c, 0xc0, 0x37, 0xee, 0x5f, 0xcd, 0x4c, 0x1c, 0xd5, 0x37, 0x27, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char image1_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x57, 0x02, 0xf9, 0x87, 0x00, 0x00, 0x00,
    0xc2, 0x49, 0x44, 0x41, 0x54, 0x68, 0x81, 0xed, 0xd7, 0xbb, 0x0a, 0x84,
    0x30, 0x14, 0x84, 0xe1, 0x75, 0xf1, 0x75, 0xac, 0xac, 0xf2, 0xfe, 0x90,
    0xad, 0xac, 0xce, 0x03, 0xc5, 0x22, 0x0c, 0xd8, 0xba, 0xe7, 0x32, 0x04,
    0xe6, 0x6f, 0xc4, 0x42, 0x98, 0x0f, 0x85, 0xe0, 0xd6, 0x7b, 0xff, 0xac,
    0xdc, 0x97, 0x3d, 0xc0, 0x9b, 0x00, 0xec, 0x04, 0x60, 0x27, 0x00, 0x3b,
    0x01, 0xd8, 0x09, 0xc0, 0x4e, 0x00, 0x76, 0x02, 0x98, 0xb5, 0x61, 0xd6,
    0x46, 0xc4, 0x98, 0x7f, 0xda, 0x3d, 0x0f, 0x9b, 0xb5, 0x71, 0x5d, 0xb8,
    0x9b, 0x88, 0xe3, 0xf8, 0x6d, 0xde, 0x51, 0x6f, 0x72, 0x01, 0x9e, 0xb1,
    0x20, 0x61, 0x00, 0x54, 0x0d, 0x09, 0x07, 0xa0, 0x2a, 0x48, 0x1a, 0x00,
    0x65, 0x43, 0xd2, 0x01, 0x28, 0x0b, 0x52, 0x06, 0x40, 0xd1, 0x90, 0x72,
    0x00, 0x7a, 0x42, 0x3c, 0x88, 0xe5, 0x4f, 0x62, 0xda, 0x1b, 0x38, 0xcf,
    0x79, 0x5d, 0xee, 0x13, 0x8a, 0x1a, 0x8e, 0xca, 0x00, 0xd1, 0xc3, 0x51,
    0x3a, 0x20, 0x6b, 0x38, 0x4a, 0x03, 0x64, 0x0f, 0x47, 0xe1, 0x80, 0xaa,
    0xe1, 0x28, 0x0c, 0x50, 0x3d, 0x1c, 0xb9, 0x00, 0x73, 0x2c, 0xe7, 0x3f,
    0x00, 0xb9, 0xdf, 0x00, 0x6b, 0x38, 0x5a, 0xfe, 0x24, 0x16, 0x80, 0x9d,
    0x00, 0xec, 0x04, 0x60, 0x27, 0x00, 0x3b, 0x01, 0xd8, 0x09, 0xc0, 0x6e,
    0x79, 0xc0, 0x0d, 0xf2, 0x42, 0x3a, 0x64, 0xbf, 0x54, 0xb8, 0x1c, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char image2_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x57, 0x02, 0xf9, 0x87, 0x00, 0x00, 0x00,
    0x8c, 0x49, 0x44, 0x41, 0x54, 0x68, 0x81, 0xed, 0xd9, 0x41, 0x0a, 0x80,
    0x20, 0x14, 0x00, 0xd1, 0x8c, 0xae, 0xd3, 0x01, 0xbc, 0x3f, 0xd8, 0xca,
    0xd5, 0x3f, 0x90, 0x2d, 0x22, 0x68, 0x11, 0x44, 0x16, 0x0c, 0xc2, 0xbc,
    0x9d, 0x7c, 0x10, 0x07, 0x5a, 0x28, 0xa5, 0x52, 0xca, 0x34, 0xb2, 0x99,
    0x3e, 0xc0, 0x57, 0x06, 0xd0, 0x0c, 0xa0, 0x19, 0x40, 0x33, 0x80, 0x66,
    0x00, 0xcd, 0x00, 0x9a, 0x01, 0x57, 0x11, 0xb9, 0x45, 0xe4, 0xd6, 0x3b,
    0xef, 0xb1, 0xfc, 0xb5, 0x51, 0x44, 0x6e, 0xb5, 0x9e, 0xab, 0xdc, 0xd6,
    0x75, 0x4b, 0x6f, 0xe6, 0xbd, 0xfc, 0x84, 0x68, 0x06, 0xd0, 0x0c, 0xa0,
    0x19, 0x40, 0x33, 0x80, 0x66, 0x00, 0xcd, 0x00, 0xda, 0xf0, 0x01, 0xbf,
    0xbd, 0x07, 0x8e, 0xfb, 0xfd, 0xf1, 0x58, 0xb9, 0xbb, 0xeb, 0x3f, 0xcd,
    0x7b, 0x25, 0x7f, 0x70, 0xc0, 0x0c, 0xa0, 0x19, 0x40, 0x33, 0x80, 0x66,
    0x00, 0xcd, 0x00, 0x9a, 0x01, 0xb4, 0xe1, 0x03, 0x76, 0xea, 0xd2, 0x25,
    0x1b, 0x6c, 0x96, 0xcb, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char image3_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x57, 0x02, 0xf9, 0x87, 0x00, 0x00, 0x00,
    0x87, 0x49, 0x44, 0x41, 0x54, 0x68, 0x81, 0xed, 0xd7, 0xb1, 0x09, 0x03,
    0x31, 0x10, 0x05, 0xd1, 0xd5, 0xe1, 0x76, 0xae, 0x00, 0xf5, 0x0f, 0x72,
    0x74, 0x91, 0x0a, 0x92, 0x03, 0x97, 0xf0, 0x0f, 0x86, 0x85, 0x99, 0x5c,
    0x8b, 0x1e, 0x6c, 0x20, 0x8d, 0xb5, 0x56, 0x75, 0xee, 0xa2, 0x2f, 0x90,
    0x26, 0x80, 0x4e, 0x00, 0x9d, 0x00, 0x3a, 0x01, 0x74, 0x02, 0xe8, 0x04,
    0xd0, 0xb5, 0x07, 0x7c, 0xd2, 0x01, 0x7b, 0xcf, 0x93, 0x9c, 0xbf, 0xef,
    0xef, 0x48, 0xce, 0x47, 0x80, 0xbd, 0xe7, 0x79, 0x9e, 0x64, 0x42, 0x55,
    0xd5, 0x3c, 0x09, 0xa2, 0xfd, 0x0a, 0x09, 0xa0, 0x13, 0x40, 0x27, 0x80,
    0x4e, 0x00, 0x9d, 0x00, 0x3a, 0x01, 0x74, 0x02, 0xe8, 0xa2, 0xff, 0xc0,
    0xff, 0x1d, 0xdf, 0xf8, 0x43, 0xf3, 0xc6, 0x05, 0xd2, 0xda, 0xaf, 0x90,
    0x00, 0x3a, 0x01, 0x74, 0x02, 0xe8, 0x04, 0xd0, 0x09, 0xa0, 0x13, 0x40,
    0xd7, 0x1e, 0xf0, 0x03, 0x4d, 0x88, 0x12, 0x2a, 0x56, 0xd1, 0x42, 0xbf,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char image4_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0x7f, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xed, 0x95, 0x51, 0x0a, 0x00,
    0x21, 0x08, 0x44, 0x75, 0xd9, 0xdb, 0x04, 0x1e, 0xc0, 0xe8, 0xfa, 0x5e,
    0xcb, 0xfd, 0xda, 0x85, 0x2d, 0x4b, 0x11, 0xfa, 0x6b, 0x20, 0x28, 0xa3,
    0x57, 0x0c, 0x53, 0xa1, 0x88, 0xc0, 0x0e, 0xdd, 0x7d, 0x81, 0x99, 0x35,
    0xb2, 0x50, 0x44, 0x70, 0x35, 0x7f, 0x59, 0x45, 0x55, 0x9d, 0x36, 0x00,
    0x00, 0x22, 0x72, 0x0f, 0x60, 0x82, 0x3d, 0x11, 0x91, 0x0b, 0x1f, 0xac,
    0x88, 0xa8, 0x94, 0xf2, 0xf5, 0x99, 0x59, 0x2d, 0x5b, 0x52, 0xe0, 0x5a,
    0xeb, 0x6f, 0x6c, 0xc1, 0x53, 0xe0, 0xd6, 0xda, 0x50, 0xeb, 0xe1, 0x29,
    0x30, 0xe2, 0x32, 0x10, 0x39, 0xf0, 0x9b, 0x0c, 0x6f, 0xb3, 0x54, 0x2a,
    0x22, 0x3a, 0xe0, 0x03, 0x3e, 0xe0, 0x85, 0xcc, 0x2b, 0x1d, 0x79, 0x0b,
    0x3c, 0xe1, 0xae, 0x3f, 0x6f, 0x9b, 0x15, 0x0f, 0x6d, 0x54, 0x33, 0xaf,
    0x47, 0xa4, 0xf4, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};


/*
 *  Constructs a SCMain as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
SCMain::SCMain( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    QImage img;
    img.loadFromData( image0_data, sizeof( image0_data ), "PNG" );
    image0 = img;
    img.loadFromData( image1_data, sizeof( image1_data ), "PNG" );
    image1 = img;
    img.loadFromData( image2_data, sizeof( image2_data ), "PNG" );
    image2 = img;
    img.loadFromData( image3_data, sizeof( image3_data ), "PNG" );
    image3 = img;
    img.loadFromData( image4_data, sizeof( image4_data ), "PNG" );
    image4 = img;
    if ( !name )
	setName( "SCMain" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 811, 533 ) );
    setSizeIncrement( QSize( 0, 0 ) );
    setBaseSize( QSize( 640, 480 ) );
    setIcon( image0 );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    SCMainLayout = new QGridLayout( centralWidget(), 1, 1, 11, 6, "SCMainLayout"); 

    GroupBox2 = new QGroupBox( centralWidget(), "GroupBox2" );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 6 );
    GroupBox2->layout()->setMargin( 11 );
    GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );

    PriorityLabel = new QLabel( GroupBox2, "PriorityLabel" );

    GroupBox2Layout->addMultiCellWidget( PriorityLabel, 3, 3, 0, 1 );

    Run = new QPushButton( GroupBox2, "Run" );
    Run->setPixmap( image1 );

    GroupBox2Layout->addWidget( Run, 0, 0 );

    PrioritySpin = new QSpinBox( GroupBox2, "PrioritySpin" );
    PrioritySpin->setMaxValue( 11 );

    GroupBox2Layout->addWidget( PrioritySpin, 3, 2 );

    Pause = new QPushButton( GroupBox2, "Pause" );
    Pause->setPixmap( image2 );

    GroupBox2Layout->addWidget( Pause, 0, 1 );

    Stop = new QPushButton( GroupBox2, "Stop" );
    Stop->setPixmap( image3 );

    GroupBox2Layout->addWidget( Stop, 0, 2 );

    RemoveProject = new QPushButton( GroupBox2, "RemoveProject" );

    GroupBox2Layout->addMultiCellWidget( RemoveProject, 2, 2, 0, 2 );

    AddProject = new QPushButton( GroupBox2, "AddProject" );

    GroupBox2Layout->addMultiCellWidget( AddProject, 1, 1, 0, 2 );

    SCMainLayout->addWidget( GroupBox2, 1, 1 );

    GroupBox3 = new QGroupBox( centralWidget(), "GroupBox3" );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 6 );
    GroupBox3->layout()->setMargin( 11 );
    GroupBox3Layout = new QVBoxLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );

    MessageButton = new QPushButton( GroupBox3, "MessageButton" );
    GroupBox3Layout->addWidget( MessageButton );

    SCMainLayout->addWidget( GroupBox3, 2, 1 );

    GroupBox4 = new QGroupBox( centralWidget(), "GroupBox4" );
    GroupBox4->setColumnLayout(0, Qt::Vertical );
    GroupBox4->layout()->setSpacing( 6 );
    GroupBox4->layout()->setMargin( 11 );
    GroupBox4Layout = new QGridLayout( GroupBox4->layout() );
    GroupBox4Layout->setAlignment( Qt::AlignTop );

    TextLabel2 = new QLabel( GroupBox4, "TextLabel2" );

    GroupBox4Layout->addWidget( TextLabel2, 0, 1 );

    AddClient = new QPushButton( GroupBox4, "AddClient" );
    QFont AddClient_font(  AddClient->font() );
    AddClient_font.setPointSize( 10 );
    AddClient->setFont( AddClient_font ); 

    GroupBox4Layout->addWidget( AddClient, 1, 2 );

    RemoveClient = new QPushButton( GroupBox4, "RemoveClient" );
    QFont RemoveClient_font(  RemoveClient->font() );
    RemoveClient_font.setPointSize( 10 );
    RemoveClient->setFont( RemoveClient_font ); 

    GroupBox4Layout->addWidget( RemoveClient, 2, 2 );

//    ApplySettings = new QPushButton( GroupBox4, "ApplySettings" );
//    QFont ApplySettings_font(  ApplySettings->font() );
//    ApplySettings_font.setPointSize( 10 );
//    ApplySettings->setFont( ApplySettings_font ); 

//    GroupBox4Layout->addWidget( ApplySettings, 5, 2 );

    LoadList = new QPushButton( GroupBox4, "LoadList" );
    QFont LoadList_font(  LoadList->font() );
    LoadList_font.setPointSize( 10 );
    LoadList->setFont( LoadList_font ); 

    GroupBox4Layout->addWidget( LoadList, 3, 2 );

    SaveList = new QPushButton( GroupBox4, "SaveList" );
    QFont SaveList_font(  SaveList->font() );
    SaveList_font.setPointSize( 10 );
    SaveList->setFont( SaveList_font ); 

    GroupBox4Layout->addWidget( SaveList, 4, 2 );
    spacer1 = new QSpacerItem( 81, 41, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox4Layout->addItem( spacer1, 0, 2 );

    TextLabel1 = new QLabel( GroupBox4, "TextLabel1" );

    GroupBox4Layout->addWidget( TextLabel1, 0, 0 );

    ClientSettingList = new QTable( GroupBox4, "ClientSettingList" );
    ClientSettingList->setNumCols( ClientSettingList->numCols() + 1 );
    ClientSettingList->horizontalHeader()->setLabel( ClientSettingList->numCols() - 1, tr( "Setting" ) );
    ClientSettingList->setNumCols( ClientSettingList->numCols() + 1 );
    ClientSettingList->horizontalHeader()->setLabel( ClientSettingList->numCols() - 1, tr( "Value" ) );
    ClientSettingList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, ClientSettingList->sizePolicy().hasHeightForWidth() ) );
    ClientSettingList->setNumRows( 0 );
    ClientSettingList->setNumCols( 2 );
    ClientSettingList->setShowGrid( FALSE );
    ClientSettingList->setSelectionMode( QTable::SingleRow );
   
    ClientSettingList->setColumnReadOnly( 0, TRUE );
    ClientSettingList->setLeftMargin( 0 );
    ClientSettingList->setColumnMovingEnabled( FALSE );
    ClientSettingList->setColumnStretchable( 0, FALSE );
    ClientSettingList->setColumnStretchable( 1, FALSE );

    GroupBox4Layout->addMultiCellWidget( ClientSettingList, 1, 5, 1, 1 );

    ClientList = new QListBox( GroupBox4, "ClientList" );

    GroupBox4Layout->addMultiCellWidget( ClientList, 1, 5, 0, 0 );

    SCMainLayout->addMultiCellWidget( GroupBox4, 1, 2, 0, 0 );

    GroupBox1 = new QGroupBox( centralWidget(), "GroupBox1" );
    GroupBox1->setFrameShape( QGroupBox::Box );
    GroupBox1->setFrameShadow( QGroupBox::Sunken );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 6 );
    GroupBox1->layout()->setMargin( 11 );
    GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );

    ProjectList = new QListView( GroupBox1, "ProjectList" );
    ProjectList->addColumn( tr( "Project Name" ) );
    ProjectList->addColumn( tr( "Frames" ) );
    ProjectList->addColumn( tr( "Avg. time/frame" ) );
    ProjectList->addColumn( tr( "Elapsed Time" ) );
    ProjectList->addColumn( tr( "Time Remaining(est.)" ) );
    ProjectList->addColumn( tr( "Priority" ) );
    ProjectList->addColumn( tr( "Status" ) );
	ProjectList->setAllColumnsShowFocus( true );
    GroupBox1Layout->addWidget( ProjectList );

    SCMainLayout->addMultiCellWidget( GroupBox1, 0, 0, 0, 1 );

    // actions
    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( image4 ) );
    fileExitAction = new QAction( this, "fileExitAction" );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    clientOpenAction = new QAction( this, "clientOpenAction" );
    clientSaveAction = new QAction( this, "clientSaveAction" );


    // toolbars


    // menubar
    menubar = new QMenuBar( this, "menubar" );


    Project = new QPopupMenu( this );
    fileNewAction->addTo( Project );
    Project->insertSeparator();
    Project->insertSeparator();
    fileExitAction->addTo( Project );
    menubar->insertItem( QString(""), Project, 1 );

    Client = new QPopupMenu( this );
    clientOpenAction->addTo( Client );
    clientSaveAction->addTo( Client );
    menubar->insertItem( QString(""), Client, 2 );

    helpMenu = new QPopupMenu( this );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    menubar->insertItem( QString(""), helpMenu, 3 );

    languageChange();
    resize( QSize(811, 725).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    connect( LoadList, SIGNAL( clicked() ), this, SLOT( clientOpen() ) );
    connect( SaveList, SIGNAL( clicked() ), this, SLOT( clientSave() ) );
    connect( Run, SIGNAL( clicked() ), this, SLOT( runClicked() ) );
    connect( Pause, SIGNAL( clicked() ), this, SLOT( pauseClicked() ) );
    connect( Stop, SIGNAL( clicked() ), this, SLOT( stopClicked() ) );
    connect( AddClient, SIGNAL( clicked() ), this, SLOT( clientAdd() ) );
    connect( RemoveClient, SIGNAL( clicked() ), this, SLOT( clientRemove() ) );
    connect( RemoveProject, SIGNAL( clicked() ), this, SLOT( projectRemove() ) );
    connect( AddProject, SIGNAL( clicked() ), this, SLOT( projectAdd() ) );
    connect( PrioritySpin, SIGNAL( valueChanged(int) ), this, SLOT( priorityChanged(int) ) );
    connect( MessageButton, SIGNAL( clicked() ), this, SLOT( showMessages() ) );
    connect( ClientList, SIGNAL( highlighted(QListBoxItem*) ), this, SLOT( clientHighlightChanged(QListBoxItem*) ) );
    connect( ProjectList, SIGNAL( selectionChanged() ), this, SLOT( projectSelectionChanged() ) );
    connect( fileNewAction, SIGNAL( activated() ), this, SLOT( projectAdd() ) );
    connect( ClientSettingList, SIGNAL( currentChanged(int,int) ), this, SLOT( settingSelectedChanged(int,int) ) );
	connect( ClientSettingList, SIGNAL( valueChanged(int,int) ), this, SLOT( applyClientSettings(int,int) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
SCMain::~SCMain()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SCMain::languageChange()
{
    setCaption( tr( "SuperConductor" ) );
    GroupBox2->setTitle( tr( "Project Control" ) );
    PriorityLabel->setText( tr( "Priority" ) );
    Run->setText( QString::null );
    QToolTip::add( Run, tr( "Run Selected Project" ) );
    Pause->setText( QString::null );
    QToolTip::add( Pause, tr( "Pause Selected Project" ) );
    Stop->setText( QString::null );
    QToolTip::add( Stop, tr( "Halt Selected Project" ) );
    RemoveProject->setText( tr( "Remove" ) );
    QToolTip::add( RemoveProject, tr( "Halt and Remove selected project" ) );
    AddProject->setText( tr( "Add New" ) );
    QToolTip::add( AddProject, tr( "Add a new project" ) );
    GroupBox3->setTitle( tr( "Diagnostics" ) );
    MessageButton->setText( tr( "View Messages" ) );
    GroupBox4->setTitle( tr( "Project Clients" ) );
    TextLabel2->setText( tr( "Client Settings" ) );
    AddClient->setText( tr( "Add Client" ) );
    QToolTip::add( AddClient, tr( "Add a new client to the selected project" ) );
    RemoveClient->setText( tr( "Remove Client" ) );
    QToolTip::add( RemoveClient, tr( "Remove selected client from the current project" ) );
//    ApplySettings->setText( tr( "Apply New\n"
//"Settings" ) );
//    QToolTip::add( ApplySettings, tr( "Apply the new settings to the current client" ) );
    LoadList->setText( tr( "Load Client List" ) );
    SaveList->setText( tr( "Save Client List" ) );
    TextLabel1->setText( tr( "Client List" ) );
    ClientSettingList->horizontalHeader()->setLabel( 0, tr( "Setting" ) );
    ClientSettingList->horizontalHeader()->setLabel( 1, tr( "Value" ) );
    GroupBox1->setTitle( tr( "Project Information" ) );
    ProjectList->header()->setLabel( 0, tr( "Project Name" ) );
    ProjectList->header()->setLabel( 1, tr( "Frames" ) );
    ProjectList->header()->setLabel( 2, tr( "Avg. time/frame" ) );
    ProjectList->header()->setLabel( 3, tr( "Elapsed Time" ) );
    ProjectList->header()->setLabel( 4, tr( "Time Remaining(est.)" ) );
    ProjectList->header()->setLabel( 5, tr( "Priority" ) );
    ProjectList->header()->setLabel( 6, tr( "Status" ) );
    fileNewAction->setText( tr( "New" ) );
    fileNewAction->setMenuText( tr( "&New Project" ) );
    fileNewAction->setAccel( tr( "Ctrl+N" ) );
    fileExitAction->setText( tr( "Exit" ) );
    fileExitAction->setMenuText( tr( "E&xit" ) );
    fileExitAction->setAccel( QString::null );
    helpAboutAction->setText( tr( "About" ) );
    helpAboutAction->setMenuText( tr( "&About..." ) );
    helpAboutAction->setAccel( QString::null );
    clientOpenAction->setText( tr( "Open List" ) );
    clientOpenAction->setMenuText( tr( "&Open List..." ) );
    clientSaveAction->setText( tr( "Save List" ) );
    clientSaveAction->setMenuText( tr( "&Save List" ) );
    if (menubar->findItem(1))
        menubar->findItem(1)->setText( tr( "Project" ) );
    if (menubar->findItem(2))
        menubar->findItem(2)->setText( tr( "Client" ) );
    if (menubar->findItem(3))
        menubar->findItem(3)->setText( tr( "&Help" ) );
}

void SCMain::fileExit()
{
    QMainWindow::close();
}

void SCMain::helpAbout()
{
    QString helpstring;
    QString part2;
    helpstring = "SuperConductor 2, from 3D Nature";
    part2 = "Design & Implementation:\n";
    part2 = "Project Lead, Design and Implementation: David \'Pita\' Kopp\n";
    part2 += "Additional Coding, Initial Concept: Chris \'Xenon\' Hanson\n";
    part2 += "\nhttp://www.Super-Conductor.org";
    QMessageBox::information(0, helpstring, part2);
}

void SCMain::projectAdd()
{
    // here we pop up a dialog window, ask for the new project file
    QPtrList<SCProject> s;
    SCProject* tempInfo = NULL;
    s = theServer->getProjects();
    QString one, two;
    // and then put the collected objects into the project list
    if (!s.isEmpty()) {
        tempInfo = s.first();
        while (tempInfo) {
            *messages << (QString)"SCProject name: " + tempInfo->name();
            projects.append(new SCProjectListItem( ProjectList,
                                tempInfo,
                                tempInfo->name(),
                                one.setNum(tempInfo->frames()),
                                tempInfo->timePerFrame(),
                                tempInfo->elapsedTime(),
                                tempInfo->estimate(),
								two.setNum(tempInfo->priority()),
                                tempInfo->status() ));
            tempInfo = s.next();
            ProjectList->setSelected( projects.getLast(), TRUE );
        }
    }
}

void SCMain::projectRemove()
{
    if (projects.isEmpty() || !ProjectList->selectedItem()) {
        // there's nothing to remove
        *messages << (QString)"No project removed... no project selected";
    }
    else {
        SCProjectListItem* lptr = (SCProjectListItem*)(ProjectList->selectedItem());
        SCProject* prj = lptr->project();
        temporaryStorage = prj->name();
        if ( theServer->removeProject( prj ) ) {
            // ProjectList doesn't own the items, projects does
            // emits selectionChanged()
            clients.clear();
            listboxList.clear();
            ClientList->clear();  // clear all things related to this project in GUI
            projects.remove( lptr ); /* remove and delete the list item */
            ProjectList->takeItem(lptr);
            *messages << (QString)"Project " + temporaryStorage + (QString)" removed";
        } // if removeProject
    }
}

void SCMain::clientOpen()
{
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if ( projects.count() && ProjectList->selectedItem() ) {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        QPtrList<SCClient> list;
        list = theServer->getClients( prj );
        // list now has a list of SCClients, time to build the client list
        SCClient* tmp = list.first();
        while ( tmp ) {
            clients.append( tmp );
            listboxList.append(new SCClientListItem( ClientList, tmp->getName(), tmp, prj ));
            tmp = list.next();
        }
        ClientList->setCurrentItem( 0 );
    } // if projects.count()
}

void SCMain::clientSave()
{
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if ( projects.count() && clients.count() ) {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        theServer->saveClients( prj );
    }
}

void SCMain::clientAdd()
{
    bool ok=false;
    QString message = "Enter a name for this client:";
    QString name;
    SCClient* newCli;
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if (ProjectList->selectedItem()) {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        name = QInputDialog::getText(
            "Client Name", message, QLineEdit::Normal,
            QString::null, &ok, this );
        if ( !name.isEmpty() ) {
            newCli = theServer->newClient( prj , name );
            if ( newCli ) {
                // the client was successfully created... add it to the list
                clients.append( newCli );
                listboxList.append(new SCClientListItem( ClientList, name, newCli, prj ));
                ClientList->setSelected( listboxList.getLast(), TRUE );
            }
            else {
                *messages << (QString)"Server failed to create client";
            }
        }
        // don't do anything here... all additions, etc. are done in the function
    }
}

void SCMain::clientRemove()
{
    if (clients.isEmpty() || !ClientList->selectedItem()) {
        *messages << (QString)"No client removed... none selected";
    }
    else {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        if ( prj->isStopped() ) {
            SCClient* harry = NULL;
            harry = ((SCClientListItem*)(ClientList->selectedItem()))->client();
            if (theServer->removeClient( prj, harry )) {
                clients.remove(harry);
                SCClientListItem* tmp = (SCClientListItem*)(ClientList->selectedItem());
                ClientList->takeItem( tmp );
                listboxList.remove( tmp );
                delete tmp;
                ClientList->setSelected( 0, TRUE );
            }
            else {
                *messages << (QString)"No client removed... server couldn't remove client";
            }
        }
        else {
            *messages << (QString)"No client removed... project must be stopped first";
        }
    }
}

void SCMain::runClicked()
{
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if (projects.isEmpty() || !ProjectList->selectedItem()) {
        *messages << (QString)"No project started... no project selected";
    }
    else {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        if (!prj) {
            *messages << (QString)"Project find failed!";
        }
        else if (theServer->run( prj )) {
           ClientList->clearSelection();
           ClientSettingList->clearSelection();
           ClientSettingList->setReadOnly( TRUE );
           ClientList->setSelectionMode( QListBox::NoSelection );
           ClientSettingList->setSelectionMode( QTable::NoSelection );
           ProjectList->selectedItem()->setText( 6, prj->status() );
		   prj->startMark();
        }
        else
           *messages << (QString)"Error running project";
    }
}

void SCMain::pauseClicked()
{
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if (projects.isEmpty() || !ProjectList->selectedItem()) {
        *messages << (QString)"No project paused... no project selected";
    }
    else {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        if (!prj) { *messages << (QString)"Project find failed!"; }
        else if ( prj->isRunning() ) {
           if ( theServer->pause( prj ) ) {
              ClientList->clearSelection();
              ClientSettingList->clearSelection();
              ClientSettingList->setReadOnly( TRUE );
              ClientList->setSelectionMode( QListBox::NoSelection );
              ClientSettingList->setSelectionMode( QTable::NoSelection );
              ProjectList->selectedItem()->setText( 6, prj->status() );
           }
           else
              *messages << (QString)"Error pausing project";
        }
        else {
           *messages << (QString)"Project not paused because it is not running";
        }
    }
}

void SCMain::stopClicked()
{
	if ( projects.count() == 1 )
		ProjectList->setSelected( ProjectList->firstChild(), true );
    if (projects.isEmpty() || !ProjectList->selectedItem()) {
        *messages << (QString)"No project stopped... no project selected";
    }
    else {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        if (!prj) {
            *messages << (QString)"Project find failed!";
        }
        else if ( theServer->stop( prj ) ) {
           ClientList->setSelectionMode( QListBox::Single );
           ClientSettingList->setSelectionMode( QTable::SingleRow );
           ClientList->setSelected( 0, TRUE );
           ClientSettingList->selectRow( 0 );
           ClientSettingList->setReadOnly( FALSE );
           ProjectList->selectedItem()->setText( 6, prj->status() );
        }
        else
           *messages << (QString)"Error stopping project";
    }
}

void SCMain::applyClientSettings( int row, int col ) {
	if (ProjectList->selectedItem()) {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        if ( prj->isStopped() ) {
            SCClient* harry = NULL;
            harry = ((SCClientListItem*)(ClientList->selectedItem()))->client();
            QPtrList<Setting> cliSettings;
            for (int i = (ClientSettingList->numRows() - 1); i >= 0; --i) {
                cliSettings.append( new Setting( ClientSettingList->text( i, 0 ),
                                                    ClientSettingList->text( i, 1 )) );
            }
            harry->setConfig( cliSettings );
        }
        else {
            *messages << (QString)"Project needs to be stopped to change client options";
        }
    }
}

void SCMain::priorityChanged(int priVal)
{
    if (ProjectList->selectedItem()) {
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        prj->setPriority( priVal );
		ProjectList->selectedItem()->setText( 5, temporaryStorage.setNum( priVal ) );
    }
}

void SCMain::projectSelectionChanged()
{
    clients.clear();
    ClientList->clear();
    listboxList.clear();
	while ( ClientSettingList->numRows() ) {
		ClientSettingList->removeRow( 0 );
	}
    if ( ProjectList->selectedItem() ) {
        while ( ClientSettingList->numRows() > 0 ) {
            ClientSettingList->removeRow(0);
        }
        SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
        clients = prj->clientList();
        SCClientListItem* tmpCLI;
        for (uint i = 0; i < clients.count(); ++i) {
            tmpCLI = new SCClientListItem( ClientList, clients.at(i)->getName(), clients.at(i), prj );
/*          if (clients.at(i)->getProject() != prj) {
                tmpCLI->setSelectable( FALSE );
            }*/
            listboxList.append(tmpCLI);
        }
        ClientList->setSelected( listboxList.getFirst(), TRUE );
        ClientList->update();
        PrioritySpin->setValue( prj->priority() );
        SCProject::statusType status = prj->statusValue();
        if ( status ) {
            ClientList->clearSelection();
            ClientSettingList->clearSelection();
            ClientList->setSelectionMode( QListBox::NoSelection );
            ClientSettingList->setSelectionMode( QTable::NoSelection );
            ClientSettingList->setReadOnly( TRUE );
        }
        else {
            ClientList->setSelectionMode( QListBox::Single );
            ClientSettingList->setSelectionMode( QTable::SingleRow );
            ClientList->clearSelection();
            ClientSettingList->clearSelection();
            ClientList->setSelected( 0, TRUE );
            ClientSettingList->selectRow( 0 );
            ClientSettingList->setReadOnly( FALSE );
        }
    }
}

void SCMain::clientHighlightChanged(QListBoxItem* theItem)
{
    while ( ClientSettingList->numRows() > 0 ) {
        ClientSettingList->removeRow(0);
    }
    if ( theItem ) {
        ClientList->setCurrentItem(theItem);
        SCClient* harry = NULL;
        QPtrList<Setting> settings;
        Setting* aSetting;
        harry = ((SCClientListItem*)theItem)->client();
        if (harry) { settings = harry->getConfig(); }
        for ( aSetting = settings.first(); aSetting; aSetting = settings.next() ) {
            // plug aSetting into the "ClientSettingList" listview
            ClientSettingList->insertRows( 0, 1 );
            ClientSettingList->setText( 0, 0, aSetting->name());
            ClientSettingList->setText( 0, 1, aSetting->value());
        }
        ClientSettingList->selectRow( 0 );
    }
}

void SCMain::showMessages()
{
    Superconductor_Messages msgBox( messages );
    msgBox.exec();
}

void SCMain::settingSelectedChanged(int row, int col)
{
   // FIXME make sure we CANNOT change a client that exists on multiple projects AND one of them is running
    // this sets the selection any time it's changed to only the editable column
    if (ProjectList->selectedItem()) {
      // Logic: check to see if client is on multiple projects.  If so, get list of projects, make sure none
      // of them are running.  If any of them are, disallow changes, like below
       SCProject::statusType status = ((SCProjectListItem*)(ProjectList->selectedItem()))->project()->statusValue();
       if ( !status && ClientSettingList->numSelections()) {  // status will be 0 if the project is running.  See enumeration.
           col = 1;
           ClientSettingList->setCurrentCell(row, 1);
       }
    }
}

void SCMain::projectFinished(SCProject* prj) {
	SCProjectListItem* pl_item = (SCProjectListItem*)projects.first();
	bool found = false;
	while (!found && pl_item) {
		if (pl_item->project() == prj)
			found = true;
		else
			pl_item = (SCProjectListItem*)projects.next();
	}
	// now we have the project list item... set the status
	pl_item->setText( 6, prj->status() );
	prj->finishMark();
}

void SCMain::updateDisplay(SCProject* prj) {
	SCProjectListItem* pl_item = (SCProjectListItem*)projects.first();
	bool found = false;
	while (!found && pl_item) {
		if (pl_item->project() == prj)
			found = true;
		else
			pl_item = (SCProjectListItem*)projects.next();
	}
	/*
    ProjectList->header()->setLabel( 0, tr( "Project Name" ) );
    ProjectList->header()->setLabel( 1, tr( "Frames" ) );
    ProjectList->header()->setLabel( 2, tr( "Avg. time/frame" ) );
    ProjectList->header()->setLabel( 3, tr( "Elapsed Time" ) );
    ProjectList->header()->setLabel( 4, tr( "Time Remaining(est.)" ) );
    ProjectList->header()->setLabel( 5, tr( "Priority" ) );
    ProjectList->header()->setLabel( 6, tr( "Status" ) );
	*/
	// update frames column to show how many frames done? (10/600 or something)
	QString a, b;
	pl_item->setText( 1, a.setNum(prj->finishCount())+(QString)"/"+b.setNum(prj->frames()) );
	pl_item->setText( 2, prj->timePerFrame() );
	pl_item->setText( 3, prj->elapsedTime() );
	pl_item->setText( 6, prj->status() );
}

void SCMain::updateTimer(SCProjectListItem* pl_item) {
	// update the display of the elapsed time ONLY (once per second)
}

void SCMain::setServer( SCServer* serv )
{
    theServer = serv;
    // here we connect the signals/slots that are necessary
	connect( theServer, SIGNAL( projectFinished(SCProject*) ), this, SLOT( projectFinished(SCProject*) ) );
	connect( theServer, SIGNAL( updateDisplay(SCProject*) ), this, SLOT( updateDisplay(SCProject*) ) );
}

void SCMain::setMessages( SCMessageList* ptr )
{
    messages = ptr;
    // here we connect the signals/slots that are necessary
}
