/****************************************************************************
** Form implementation generated from reading ui file 'BaseSC.ui'
**
** Created: Fri Feb 28 15:20:05 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
// #include "ui/BaseSC.ui.h"

#include <qvariant.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtable.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qinputdialog.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmessagebox.h>

#include "mainwindow.h"
#include "message_dialog.h"

static const char* const image0_data[] = { 
"32 32 4 1",
". c None",
"a c #0000ff",
"# c #001249",
"b c #0a41d8",
"................................",
"................................",
"................................",
".............##aaaa.............",
"............#aaaaaaaa...........",
"...........#aaaaaaaaaa..........",
"..........#aaaa....aaaa.........",
"..........#aa.......aaaa........",
".........#aa........aaaa........",
".........#aa........#aaa........",
".........#aa##aa....#aaa........",
"........##aaaaaaaa..#aaa........",
".......#a#aa........#aaa........",
"......#aa.#a.......#aaab........",
"......aa..#aa......#aaab........",
".....#aa..#aa.....#aaaab.#a.....",
".....#a....##a....#aaab..#a.....",
".....aa.....#a...#aaab....#a....",
".....aa......#a..#aaab....#a....",
".....aa.........#aaab.....#aa...",
".....aa........#aaaab.....#aa...",
".....aa.......#aaaab......#aa...",
".....aaa######aaaab......#aaa...",
"......aaaaaaaaaaabba....#aaaa...",
"......aaaaaaaaaab..aaa##aaaaa...",
"........aaaaabbb....aaaaaaaa....",
".........bbbbb.......aaaaa......",
"................................",
"................................",
"................................",
"................................",
"................................"};

static const char* const image1_data[] = { 
"48 48 3 1",
". c None",
"a c #6666ff",
"# c #9999ff",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................##..............................",
"...............#aa##............................",
"...............#aaaa##..........................",
"...............#aaaaaa##........................",
"...............#aaaaaaaa##......................",
"...............#aaaaaaaaaa##....................",
"...............#aaaaaaaaaaaa##..................",
"...............#aaaaaaaaaaaaaa##................",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaa##................",
"...............#aaaaaaaaaaaa##..................",
"...............#aaaaaaaaaa##....................",
"...............#aaaaaaaa##......................",
"...............#aaaaaa##........................",
"...............#aaaa##..........................",
"...............#aa##............................",
"................##..............................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................"};

static const char* const image2_data[] = { 
"48 48 3 1",
". c None",
"# c #9999ff",
"b c #6666ff",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"...................##......##...................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"..................#bb#....#bb#..................",
"...................##......##...................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................"};

static const char* const image3_data[] = { 
"48 48 3 1",
". c None",
"a c #6666ff",
"# c #9999ff",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................################................",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"...............#aaaaaaaaaaaaaaaa#...............",
"................################................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................"};

static const char* const image4_data[] = { 
"22 22 7 1",
". c None",
"# c #000000",
"b c #2e2e2e",
"c c #5c5c5c",
"d c #878787",
"e c #c2c2c2",
"a c #ffffff",
"......................",
"....##########........",
"....#aaaaaaa#b#.......",
"....#aaaaaaa#cb#......",
"....#aaaaaaa#dcb#.....",
"....#aaaaaaa#edcb#....",
"....#aaaaaaa#aedcb#...",
"....#aaaaaaa#######...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....###############...",
"......................",
"......................"};

static const char* const image5_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"c c #848200",
"a c #ffff00",
"b c #ffffff",
"......................",
"......................",
"......................",
"............####....#.",
"...........#....##.##.",
"..................###.",
".................####.",
".####...........#####.",
"#abab##########.......",
"#babababababab#.......",
"#ababababababa#.......",
"#babababababab#.......",
"#ababab###############",
"#babab##cccccccccccc##",
"#abab##cccccccccccc##.",
"#bab##cccccccccccc##..",
"#ab##cccccccccccc##...",
"#b##cccccccccccc##....",
"###cccccccccccc##.....",
"##cccccccccccc##......",
"###############.......",
"......................"};

static const char* const image6_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"a c #848200",
"b c #c1c1c1",
"c c #cab5d1",
"......................",
".####################.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbcbb####.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aaa############aaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaa#############aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
"..##################..",
"......................"};


/* 
 *  Constructs a SCMain which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
SCMain::SCMain( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{	
    (void)statusBar();
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    QPixmap image2( ( const char** ) image2_data );
    QPixmap image3( ( const char** ) image3_data );
    QPixmap image4( ( const char** ) image4_data );
    QPixmap image5( ( const char** ) image5_data );
    QPixmap image6( ( const char** ) image6_data );
    if ( !name )
	setName( "SCMain" );
	resize( 651, 500 );
	setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 640, 480 ) );
    setSizeIncrement( QSize( 0, 0 ) );
    setBaseSize( QSize( 640, 480 ) );
	// sets to sizeHint()... sizehint corrupted or something?
//	layout()->setResizeMode( QLayout::Fixed );
    setCaption( trUtf8( "SuperConductor 2" ) );
    setIcon( image0 );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );

    GroupBox2 = new QGroupBox( centralWidget(), "GroupBox2" );
    GroupBox2->setGeometry( QRect( 490, 180, 150, 190 ) ); 
    GroupBox2->setTitle( trUtf8( "Project Control" ) );

    Run = new QPushButton( GroupBox2, "Run" );
    Run->setGeometry( QRect( 20, 30, 30, 30 ) ); 
    Run->setText( trUtf8( "" ) );
    Run->setPixmap( image1 );
    QToolTip::add( Run, trUtf8( "Run Selected Project" ) );

    Pause = new QPushButton( GroupBox2, "Pause" );
    Pause->setGeometry( QRect( 60, 30, 30, 30 ) ); 
    Pause->setText( trUtf8( "" ) );
    Pause->setPixmap( image2 );
    QToolTip::add( Pause, trUtf8( "Pause Selected Project" ) );

    Stop = new QPushButton( GroupBox2, "Stop" );
    Stop->setGeometry( QRect( 100, 30, 30, 30 ) ); 
    Stop->setText( trUtf8( "" ) );
    Stop->setPixmap( image3 );
    QToolTip::add( Stop, trUtf8( "Halt Selected Project" ) );

    AddProject = new QPushButton( GroupBox2, "AddProject" );
    AddProject->setGeometry( QRect( 20, 80, 110, 24 ) ); 
    AddProject->setText( trUtf8( "Add New" ) );
    QToolTip::add( AddProject, trUtf8( "Add a new project" ) );

    RemoveProject = new QPushButton( GroupBox2, "RemoveProject" );
    RemoveProject->setGeometry( QRect( 20, 120, 110, 24 ) ); 
    RemoveProject->setText( trUtf8( "Remove" ) );
    QToolTip::add( RemoveProject, trUtf8( "Halt and Remove selected project" ) );

    PriorityLabel = new QLabel( GroupBox2, "PriorityLabel" );
    PriorityLabel->setGeometry( QRect( 20, 160, 65, 20 ) ); 
    PriorityLabel->setText( trUtf8( "Priority" ) );

    PrioritySpin = new QSpinBox( GroupBox2, "PrioritySpin" );
    PrioritySpin->setGeometry( QRect( 90, 160, 40, 21 ) ); 
//	PrioritySpin->setMaxValue( 11 );
//	PrioritySpin->setMinValue( 0 );

    GroupBox3 = new QGroupBox( centralWidget(), "GroupBox3" );
    GroupBox3->setGeometry( QRect( 490, 371, 151, 70 ) ); 
    GroupBox3->setTitle( trUtf8( "Diagnostics" ) );

    MessageButton = new QPushButton( GroupBox3, "MessageButton" );
    MessageButton->setGeometry( QRect( 20, 30, 110, 24 ) ); 
    MessageButton->setText( trUtf8( "View Messages" ) );

    GroupBox1 = new QGroupBox( centralWidget(), "GroupBox1" );
    GroupBox1->setGeometry( QRect( 10, 0, 630, 180 ) ); 
    GroupBox1->setFrameShape( QGroupBox::Box );
    GroupBox1->setFrameShadow( QGroupBox::Sunken );
    GroupBox1->setTitle( trUtf8( "Project Information" ) );

    ProjectList = new QListView( GroupBox1, "ProjectList" );
    ProjectList->addColumn( trUtf8( "Project Name" ) );
    ProjectList->addColumn( trUtf8( "Frames" ) );
    ProjectList->addColumn( trUtf8( "Avg. time/frame" ) );
    ProjectList->addColumn( trUtf8( "Elapsed Time" ) );
    ProjectList->addColumn( trUtf8( "Time Remaining(est.)" ) );
    ProjectList->addColumn( trUtf8( "Status" ) );
    ProjectList->setGeometry( QRect( 10, 20, 611, 151 ) );
	ProjectList->setAllColumnsShowFocus( TRUE );

    GroupBox4 = new QGroupBox( centralWidget(), "GroupBox4" );
    GroupBox4->setGeometry( QRect( 10, 180, 470, 260 ) ); 
    GroupBox4->setTitle( trUtf8( "Project Clients" ) );

    TextLabel2 = new QLabel( GroupBox4, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 151, 20, 110, 20 ) ); 
    TextLabel2->setText( trUtf8( "Client Settings" ) );

    TextLabel1 = new QLabel( GroupBox4, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 10, 20, 85, 20 ) ); 
    TextLabel1->setText( trUtf8( "Client List" ) );

    LoadList = new QPushButton( GroupBox4, "LoadList" );
    LoadList->setGeometry( QRect( 370, 140, 91, 31 ) ); 
    QFont LoadList_font(  LoadList->font() );
    LoadList_font.setPointSize( 10 );
    LoadList->setFont( LoadList_font ); 
    LoadList->setText( trUtf8( "Load Client List" ) );

    SaveList = new QPushButton( GroupBox4, "SaveList" );
    SaveList->setGeometry( QRect( 370, 180, 91, 31 ) ); 
    QFont SaveList_font(  SaveList->font() );
    SaveList_font.setPointSize( 10 );
    SaveList->setFont( SaveList_font ); 
    SaveList->setText( trUtf8( "Save Client List" ) );

    AddClient = new QPushButton( GroupBox4, "AddClient" );
    AddClient->setGeometry( QRect( 370, 40, 90, 31 ) ); 
    QFont AddClient_font(  AddClient->font() );
    AddClient_font.setPointSize( 10 );
    AddClient->setFont( AddClient_font ); 
    AddClient->setText( trUtf8( "Add Client" ) );
    QToolTip::add( AddClient, trUtf8( "Add a new client to the selected project" ) );

    RemoveClient = new QPushButton( GroupBox4, "RemoveClient" );
    RemoveClient->setGeometry( QRect( 370, 80, 90, 30 ) ); 
    QFont RemoveClient_font(  RemoveClient->font() );
    RemoveClient_font.setPointSize( 10 );
    RemoveClient->setFont( RemoveClient_font ); 
    RemoveClient->setText( trUtf8( "Remove Client" ) );
    QToolTip::add( RemoveClient, trUtf8( "Remove selected client from the current project" ) );

    ClientSettingList = new QTable( 0, 2, GroupBox4, "ClientSettingList" );
	ClientSettingList->horizontalHeader()->setLabel( 0, tr( "Setting" ) );
	ClientSettingList->horizontalHeader()->setLabel( 1, tr( "Value" ) );
    ClientSettingList->setGeometry( QRect( 151, 40, 210, 211 ) );
	ClientSettingList->setColumnReadOnly( 0, TRUE );
	ClientSettingList->setLeftMargin( 0 );
	ClientSettingList->setSelectionMode( QTable::SingleRow );
	ClientSettingList->setColumnWidth( 0, 88 );
	ClientSettingList->setColumnWidth( 1, 118 );
	ClientSettingList->setColumnMovingEnabled( FALSE );
	ClientSettingList->setColumnStretchable( 0, FALSE );
	ClientSettingList->setColumnStretchable( 1, FALSE );

    ClientList = new QListBox( GroupBox4, "ClientList" );
    ClientList->setGeometry( QRect( 10, 40, 130, 211 ) );
	ClientList->setSelectionMode(QListBox::Single);

    // actions
    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( image4 ) );
    fileNewAction->setText( trUtf8( "New Project" ) );
    fileNewAction->setMenuText( trUtf8( "&New" ) );
    fileNewAction->setAccel( 4194382 );
    clientOpenAction = new QAction( this, "fileOpenAction" );
    clientOpenAction->setIconSet( QIconSet( image5 ) );
    clientOpenAction->setText( trUtf8( "Open list" ) );
    clientOpenAction->setMenuText( trUtf8( "&Open List..." ) );
    clientOpenAction->setAccel( 4194383 );
    clientSaveAction = new QAction( this, "fileSaveAction" );
    clientSaveAction->setIconSet( QIconSet( image6 ) );
    clientSaveAction->setText( trUtf8( "Save list" ) );
    clientSaveAction->setMenuText( trUtf8( "&Save List" ) );
    clientSaveAction->setAccel( 4194387 );
	clientSaveAction->setIconSet( QIconSet( image6 ) );
    fileExitAction = new QAction( this, "fileExitAction" );
    fileExitAction->setText( trUtf8( "Exit" ) );
    fileExitAction->setMenuText( trUtf8( "E&xit" ) );
    fileExitAction->setAccel( 0 );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    helpAboutAction->setText( trUtf8( "About" ) );
    helpAboutAction->setMenuText( trUtf8( "&About..." ) );
    helpAboutAction->setAccel( 0 );


    // toolbars


    // menubar
    menubar = new QMenuBar( this, "menubar" );

    fileMenu = new QPopupMenu( this ); 
    fileNewAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    menubar->insertItem( trUtf8( "&Project" ), fileMenu );
	
	clientMenu = new QPopupMenu( this );
    clientOpenAction->addTo( clientMenu );
    clientSaveAction->addTo( clientMenu );
	menubar->insertItem( trUtf8( "&Client" ), clientMenu );
	
    helpMenu = new QPopupMenu( this );
    helpAboutAction->addTo( helpMenu );
    menubar->insertItem( trUtf8( "&Help" ), helpMenu );

	clients.setAutoDelete( FALSE );
	listboxList.setAutoDelete( FALSE );
//	projects.setAutoDelete( TRUE );     // ProjectList doesn't own list items
	projects.setAutoDelete( FALSE );

    // signals and slots connections
    connect( fileNewAction, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    connect( clientOpenAction, SIGNAL( activated() ), this, SLOT( clientOpen() ) );
    connect( clientSaveAction, SIGNAL( activated() ), this, SLOT( clientSave() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
	
	connect( MessageButton, SIGNAL( clicked() ), this, SLOT( showMessages() ) );
    connect( LoadList, SIGNAL( clicked() ), this, SLOT( clientOpen() ) );
    connect( SaveList, SIGNAL( clicked() ), this, SLOT( clientSave() ) );
	connect( AddProject, SIGNAL( clicked() ), this, SLOT( fileNew() ) );
	connect( RemoveProject, SIGNAL( clicked() ), this, SLOT( projectRemove() ) );
	connect( AddClient, SIGNAL( clicked() ), this, SLOT( addClient() ) );
	connect( RemoveClient, SIGNAL( clicked() ), this, SLOT( removeClient() ) );
	connect( Run, SIGNAL( clicked() ), this, SLOT( runPressed() ) );
	connect( Pause, SIGNAL( clicked() ), this, SLOT( pausePressed() ) );
	connect( Stop, SIGNAL( clicked() ), this, SLOT( stopPressed() ) );
	connect( PrioritySpin, SIGNAL( valueChanged( int ) ), this, SLOT( priorityChanged( int ) ) );
	connect( ProjectList, SIGNAL( selectionChanged() ), this, SLOT( projectSelectionChanged() ) );
	connect( ClientSettingList, SIGNAL( valueChanged(int, int) ), this, SLOT( applySettings() ) );
	connect( ClientSettingList, SIGNAL( currentChanged(int, int) ), this, SLOT( ClientSettingSelect(int, int) ) );
	connect( ClientList, SIGNAL( highlighted( QListBoxItem* )), this, SLOT( ClientHighlightChanged( QListBoxItem* )) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SCMain::~SCMain()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Will add a new project
 */
void SCMain::fileNew()
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
								tempInfo->status() ));
			tempInfo = s.next();
			ProjectList->setSelected( projects.getLast(), TRUE );
		}
	}
}

void SCMain::projectRemove() {
	if (projects.isEmpty() || !ProjectList->selectedItem()) {
		// there's nothing to remove
		*messages << (QString)"No project removed... no project selected";
	}
	else {
		// column 5 is the job ID
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
	if ( projects.count() && clients.count() ) {
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		theServer->saveClients( prj );
	}
}
	
void SCMain::fileExit()
{
	this->close();
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

void SCMain::runPressed() {
	if (projects.isEmpty() || !ProjectList->selectedItem()) {
		*messages << (QString)"No project started... no project selected";
	}
	else {
		ClientList->clearSelection();
		ClientSettingList->clearSelection();
		ClientSettingList->setReadOnly( TRUE );
		ClientList->setSelectionMode( QListBox::NoSelection );
		ClientSettingList->setSelectionMode( QTable::NoSelection );
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		if (!prj) { *messages << (QString)"Conversion failed"; }
		theServer->run( prj );
		ProjectList->selectedItem()->setText( 5, prj->status() );
	}
}

void SCMain::pausePressed() {
	if (projects.isEmpty() || !ProjectList->selectedItem()) {
		*messages << (QString)"No project paused... no project selected";
	}
	else {
		ClientList->clearSelection();
		ClientSettingList->clearSelection();
		ClientSettingList->setReadOnly( TRUE );
		ClientList->setSelectionMode( QListBox::NoSelection );
		ClientSettingList->setSelectionMode( QTable::NoSelection );
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		if (!prj) { *messages << (QString)"Conversion failed"; }
		theServer->pause( prj );
		ProjectList->selectedItem()->setText( 5, prj->status() );
	}
}

void SCMain::stopPressed() {
	if (projects.isEmpty() || !ProjectList->selectedItem()) {
		*messages << (QString)"No project stopped... no project selected";
	}
	else {
		ClientList->setSelectionMode( QListBox::Single );
		ClientSettingList->setSelectionMode( QTable::SingleRow );
		ClientList->setSelected( 0, TRUE );
		ClientSettingList->selectRow( 0 );
		ClientSettingList->setReadOnly( FALSE );
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		theServer->stop( prj );
		ProjectList->selectedItem()->setText( 5, prj->status() );
	}
}

void SCMain::projectSelectionChanged() {
	clients.clear();
	ClientList->clear();
	listboxList.clear();
	if ( ProjectList->selectedItem() ) {
		while ( ClientSettingList->numRows() > 0 ) {
			ClientSettingList->removeRow(0);
		}
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		clients = prj->clientList();
		SCClientListItem* tmpCLI;
		for (uint i = 0; i < clients.count(); ++i) {
			tmpCLI = new SCClientListItem( ClientList, clients.at(i)->getName(), clients.at(i), prj );
/*			if (clients.at(i)->getProject() != prj) {
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

void SCMain::applySettings() {
	// take settings from table and push them into client
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

void SCMain::addClient() {
	bool ok=false;
	QString message = "Enter a name for this client:";
	QString name;
	SCClient* newCli;
	if (ProjectList->selectedItem()) {
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
 		name = QInputDialog::getText(
			"Client Name", message, QLineEdit::Normal,
			QString::null, &ok, this );
		if ( !name.isEmpty() ) {
			newCli = theServer->newClient( prj , name);
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

void SCMain::removeClient() {
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

void SCMain::priorityChanged( int priVal ) {
	if (ProjectList->selectedItem()) {
		SCProject* prj = ((SCProjectListItem*)(ProjectList->selectedItem()))->project();
		prj->setPriority( priVal );
	}
}

void SCMain::ClientSettingSelect(int row, int col) {
	// this sets the selection any time it's changed to only the editable column
	SCProject::statusType status = ((SCProjectListItem*)(ProjectList->selectedItem()))->project()->statusValue();
	if ( !status ) {
		col = 1;
		ClientSettingList->setCurrentCell(row, 1);
	}
}

void SCMain::ClientHighlightChanged( QListBoxItem* theItem ) {
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

void SCMain::showMessages() {
	Superconductor_Messages msgBox( messages );
	msgBox.exec();
}

bool SCMain::close() {
	// do cleanup ops in here, if necessary
//	cerr << messages->join("\n");
	return QMainWindow::close();
}

void SCMain::setServer( SCServer* serv ) {
	theServer = serv;
	// here we connect the signals/slots that are necessary
}

void SCMain::setMessages( QStringList* ptr ) {
	messages = ptr;
	// here we connect the signals/slots that are necessary
}
