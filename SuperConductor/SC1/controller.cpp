// controller.cpp

#include <fstream.h>
#include <qvariant.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "controller.h"
#include "client.h"
#include "serverobject.h"
#include "wcs-client.h"
#include "clientDataTransfer.h"

static QPixmap uic_load_pixmap_Controller( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );	
    if ( !m )
		return QPixmap();
    QPixmap pix;	
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
*  Constructs a Controller which is a child of 'parent', with the   
*  name 'name' and widget flags set to 'f'.	
*
*/

Controller::Controller( QWidget* parent,  const char* name, WFlags fl )
: QMainWindow( parent, name, fl )
{
	ID = 0;
    (void)statusBar();
    //if ( !name )
//	setName( "Super Conductor" );
	setCaption("Super Conductor");
	setIconText("SuperConductor");
	layout()->setResizeMode( QLayout::FreeResize );
    resize( 640, 470 );
    setMinimumSize( QSize( 640, 470 ) );
    setMaximumSize( QSize( 640, 470 ) );
    setCursor( QCursor( 0 ) );
    
    setIcon( uic_load_pixmap_Controller( "" ) );
    setIconText( trUtf8( "" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
	
    Tab = new QTabWidget( centralWidget(), "Tab" );
    Tab->setGeometry( QRect( 0, 0, 640, 430 ) ); 
    QFont Tab_font(  Tab->font() );
    Tab_font.setFamily( "Arial" );
    Tab->setFont( Tab_font ); 
    Tab->setTabPosition( QTabWidget::Top );
    Tab->setTabShape( QTabWidget::Rounded );
	
    tab = new QWidget( Tab, "tab" );
	
    TextLabel4 = new QLabel( tab, "Active Projects" );
    TextLabel4->setGeometry( QRect( 10, 10, 80, 21 ) ); 
    TextLabel4->setText( trUtf8( "Active Projects" ) );
	
    TextLabel3 = new QLabel( tab, "Source File" );
    TextLabel3->setGeometry( QRect( 10, 250, 60, 21 ) ); 
    TextLabel3->setText( trUtf8( "Source File" ) );
	
    TextLabel6 = new QLabel( tab, "Destination Path" );
    TextLabel6->setGeometry( QRect( 10, 320, 80, 21 ) ); 
    TextLabel6->setText( trUtf8( "Destination Path" ) );
	
    TextLabel5 = new QLabel( tab, "Priority" );
    TextLabel5->setGeometry( QRect( 471, 260, 50, 21 ) ); 
    TextLabel5->setText( trUtf8( "Priority" ) );
	
    DestinationFile = new QLineEdit( tab, "DestinationFile" );
    DestinationFile->setGeometry( QRect( 10, 350, 250, 21 ) ); 
    DestinationFile->setDisabled(1);
	
    SourceFile = new QLineEdit( tab, "SourceFile" );
    SourceFile->setGeometry( QRect( 10, 280, 250, 21 ) ); 
	
    DestinationBrowse = new QPushButton( tab, "DestinationBrowse" );
    DestinationBrowse->setGeometry( QRect( 270, 350, 90, 21 ) ); 
    DestinationBrowse->setCursor( QCursor( 0 ) );
    DestinationBrowse->setText( trUtf8( "Browse . . ." ) );
    DestinationBrowse->setDisabled(1);
	
    SourceBrowse = new QPushButton( tab, "SourceBrowse" );
    SourceBrowse->setGeometry( QRect( 270, 280, 90, 21 ) ); 
    SourceBrowse->setCursor( QCursor( 0 ) );
    SourceBrowse->setText( trUtf8( "Browse . . ." ) );
    SourceBrowse->setFlat( FALSE );
    Tab->insertTab( tab, trUtf8( "Projects" ) );
	
    Update = new QPushButton( tab, "Update" );
    Update->setGeometry( QRect( 370, 280, 90, 21 ) ); 
    Update->setCursor( QCursor( 0 ) );
    Update->setText( trUtf8( "Add Project" ) );
	
	DeleteProject = new QPushButton( tab,"DeleteProject");
	DeleteProject->setGeometry(QRect( 370, 315, 90, 21 ));
	DeleteProject->setCursor(QCursor(0));
	DeleteProject->setText(trUtf8("Delete Project"));
	
	PauseProject = new QPushButton(tab, "PauseProject");
	PauseProject->setGeometry(QRect( 370, 350, 90 , 21));
	PauseProject->setCursor(QCursor(0));
	PauseProject->setText(trUtf8("Pause Project"));
	PauseProject->setToggleButton( TRUE );
	
	start = new QPushButton(tab, "Start Render" );
	start->setText(trUtf8("Start Render!"));
	start->setGeometry( QRect( 470, 315, 125, 56));
	
	PrioritySpin = new QSpinBox( tab, "PrioritySpin" );
    PrioritySpin->setGeometry( QRect( 471, 280, 44, 21 ) ); 
    PrioritySpin->setCursor( QCursor( 0 ) );
	PrioritySpin->setValue(1);
	PrioritySpin->setMinValue(0);
	PrioritySpin->setMaxValue(10);
	
    ProjectList = new QListView( tab, "ProjectList" );
    ProjectList->addColumn( trUtf8( "Project   " ) );
    ProjectList->addColumn( trUtf8( "Priority  " ) );
    ProjectList->addColumn( trUtf8( "Frame  " ) );
    ProjectList->addColumn( trUtf8( " Avg Frame Time" ) );
	ProjectList->addColumn( trUtf8( " Total Job Time" ));
    ProjectList->addColumn( trUtf8( " Time Left Estimation"));
	ProjectList->addColumn( trUtf8( " ID" ));
	ProjectList->setColumnWidth(7, 0);
	ProjectList->setGeometry( QRect( 10, 40, 615, 190 ) ); 
    ProjectList->setVScrollBarMode( QListView::Auto );
    ProjectList->setHScrollBarMode( QListView::Auto );
	ProjectList->setAllColumnsShowFocus(true);
	
    tab_2 = new QWidget( Tab, "tab_2" );
	
    TextLabel7 = new QLabel( tab_2, "Add Client" );
    TextLabel7->setGeometry( QRect( 10, 270, 60, 21 ) ); 
    TextLabel7->setText( trUtf8( "Add Client" ) );
	
    TextLabel2 = new QLabel( tab_2, "Available Clients" );
    TextLabel2->setGeometry( QRect( 10, 10, 82, 21 ) ); 
    QFont TextLabel2_font(  TextLabel2->font() );
    TextLabel2->setFont( TextLabel2_font ); 
    TextLabel2->setText( trUtf8( "Available Clients" ) );
	
	DNSLabel = new QLabel( tab_2, "Address Suffix" );
    DNSLabel->setGeometry( QRect( 10, 331, 60, 21 ) ); 
    DNSLabel->setText( trUtf8( "Address Suffix" ) );
	
	DNSSuffix = new QLineEdit(tab_2, "DNS Suffix");
	DNSSuffix->setGeometry( QRect( 10, 362, 220 ,21 ) );
	DNSSuffix->setText(Domain);
	
	SocketLabel = new QLabel( tab_2, "Port" );
    SocketLabel->setGeometry( QRect( 240, 331, 60, 21 ) ); 
    SocketLabel->setText( trUtf8( "Port" ) );
	
	SocketNumber = new QSpinBox( tab_2, "SocketNumber" );
    SocketNumber->setGeometry( QRect( 240, 362, 220 ,21 ) );
    SocketNumber->setCursor( QCursor( 0 ) );
	SocketNumber->setMinValue(1024);
	SocketNumber->setMaxValue(65535);
	SocketNumber->setValue(4242);
	
    FindClientName = new QLineEdit( tab_2, "FindClientName" );
    FindClientName->setGeometry( QRect( 10, 300, 220, 21 ) ); 
	
    TextLabel8 = new QLabel( tab_2, "Clients Used" );
    TextLabel8->setGeometry( QRect( 320, 10, 63, 21 ) ); 
    TextLabel8->setText( trUtf8( "Clients Used" ) );
	
    UsedClients = new QListBox( tab_2, "UsedClients" );
    UsedClients->setGeometry( QRect( 320, 40, 171, 211 ) ); 
    UsedClients->setFocusPolicy( QListBox::ClickFocus );
    UsedClients->setSelectionMode( QListBox::Extended );
	
    AvailableClients = new QListBox( tab_2, "AvailableClients" );
    AvailableClients->setGeometry( QRect( 10, 40, 171, 211 ) ); 
    AvailableClients->setFocusPolicy( QListBox::ClickFocus );
    AvailableClients->setSelectionMode( QListBox::Extended );
	
    AddClient = new QPushButton( tab_2, "AddClient" );
    AddClient->setGeometry( QRect( 200, 40, 101, 21 ) ); 
    AddClient->setCursor( QCursor( 0 ) );
    AddClient->setText( trUtf8( "Use Client >>" ) );
	
    RemoveClient = new QPushButton( tab_2, "RemoveClient" );
    RemoveClient->setGeometry( QRect( 200, 70, 101, 21 ) ); 
    RemoveClient->setCursor( QCursor( 0 ) );
    RemoveClient->setText( trUtf8( "<< Unuse Client" ) );
	
    AddAll = new QPushButton( tab_2, "AddAll" );
    AddAll->setGeometry( QRect( 200, 120, 101, 21 ) ); 
    AddAll->setCursor( QCursor( 0 ) );
    AddAll->setText( trUtf8( "Use All" ) );
	
    RemoveAll = new QPushButton( tab_2, "RemoveAll" );
    RemoveAll->setGeometry( QRect( 200, 150, 101, 21 ) ); 
    RemoveAll->setCursor( QCursor( 0 ) );
    RemoveAll->setText( trUtf8( "Unuse All" ) );
	
    OpenList = new QPushButton( tab_2, "OpenList" );
    OpenList->setGeometry( QRect( 200, 200, 101, 21 ) ); 
    OpenList->setCursor( QCursor( 0 ) );
    OpenList->setText( trUtf8( "Load Client List" ) );
	
    SaveList = new QPushButton( tab_2, "SaveList" );
    SaveList->setGeometry( QRect( 200, 230, 101, 21 ) ); 
    SaveList->setCursor( QCursor( 0 ) );
    SaveList->setText( trUtf8( "Save Client List" ) );
	
    FindClientButton = new QPushButton( tab_2, "FindClientButton" );
    FindClientButton->setGeometry( QRect( 240, 300, 41, 21 ) ); 
    FindClientButton->setCursor( QCursor( 0 ) );
    FindClientButton->setText( trUtf8( "OK" ) );
    FindClientButton->setDefault( FALSE );
	/*	
    TextLabel1 = new QLabel( tab_2, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 320, 270, 141, 21 ) ); 
    TextLabel1->setText( trUtf8( "Search For Available Clients" ) );
	
	  ClientSearch = new QPushButton( tab_2, "ClientSearch" );
	  ClientSearch->setGeometry( QRect( 320, 300, 40, 21 ) ); 
	  ClientSearch->setCursor( QCursor( 0 ) );
    ClientSearch->setText( trUtf8( "GO" ) );*/
    Tab->insertTab( tab_2, trUtf8( "Resources" ) );
	
    tab_3 = new QWidget( Tab, "tab_3" );
	
	infoText = new QTextView( tab_3,"infor text" );
    infoText->setGeometry(QRect(10,20,615,350));
    
	aServer = new ServerObject(NULL);
	connect(this, SIGNAL(rendering()), aServer, SLOT(startJobs()));
	connect(aServer, SIGNAL(finished(const QString&)),this, SLOT(socketJobDone(const QString&)));
	connect(aServer, SIGNAL(connectChange(const QString&)),this, SLOT(connectChange(const QString&)));
	Tab->insertTab( tab_3, trUtf8( "Server Messages" ) );
	
	tab_4 = new QWidget( Tab, "tab_4");
	
	ClientList = new QListView( tab_4, "ClientList" );
    ClientList->addColumn( trUtf8( "Clients   " ) );
    ClientList->addColumn( trUtf8( "Project  " ) );
    ClientList->addColumn( trUtf8( " Action"));
	ClientList->addColumn( trUtf8( " Status"));
	ClientList->setGeometry( QRect( 10, 20, 615, 350 ) ); 
    ClientList->setVScrollBarMode( QListView::Auto );
    ClientList->setHScrollBarMode( QListView::Auto );
	ClientList->setAllColumnsShowFocus(true);
	
	Tab->insertTab( tab_4, trUtf8( "Client Status"));
    // actions
	//fileNewAction = new QAction( this, "fileNewAction" );
	//fileNewAction->setIconSet( QIconSet( uic_load_pixmap_Controller( "" ) ) );
	//fileNewAction->setText( trUtf8( "New" ) );
	//fileNewAction->setMenuText( trUtf8( "&New" ) );
	//fileNewAction->setAccel( 4194382 );
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( uic_load_pixmap_Controller( "" ) ) );
    fileOpenAction->setText( trUtf8( "Open" ) );
    fileOpenAction->setMenuText( trUtf8( "&Open Job..." ) );
    fileOpenAction->setAccel( 4194383 );
    fileExitAction = new QAction( this, "fileExitAction" );
    fileExitAction->setText( trUtf8( "Exit" ) );
    fileExitAction->setMenuText( trUtf8( "&Exit" ) );
    fileExitAction->setAccel( 0 );
    helpContentsAction = new QAction( this, "helpContentsAction" );
    helpContentsAction->setText( trUtf8( "Contents" ) );
    helpContentsAction->setMenuText( trUtf8( "&Contents..." ) );
    helpContentsAction->setAccel( 0 );
    helpIndexAction = new QAction( this, "helpIndexAction" );
    helpIndexAction->setText( trUtf8( "Index" ) );
    helpIndexAction->setMenuText( trUtf8( "&Index..." ) );
    helpIndexAction->setAccel( 0 );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    helpAboutAction->setText( trUtf8( "About" ) );
    helpAboutAction->setMenuText( trUtf8( "&About..." ) );
    helpAboutAction->setAccel( 0 );
    fileRender = new QAction( this, "fileRender" );
    fileRender->setText( trUtf8( "Render" ) );
    fileRender->setMenuText( trUtf8( "&Render" ) );
    fileRender->setAccel( 4194386 );
	
	
    // toolbars
	
	
    // menubar
    menubar = new QMenuBar( this, "menubar" );
	
    menubar->setFrameShape( QMenuBar::MenuBarPanel );
    menubar->setFrameShadow( QMenuBar::Raised );
    fileMenu = new QPopupMenu( this ); 
    //fileNewAction->addTo( fileMenu );
    fileOpenAction->addTo( fileMenu );
    fileRender->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    menubar->insertItem( trUtf8( "&File" ), fileMenu );
	
    helpMenu = new QPopupMenu( this ); 
    helpContentsAction->addTo( helpMenu );
    helpIndexAction->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    menubar->insertItem( trUtf8( "&Help" ), helpMenu );
	
	
	
    // signals and slots connections
	connect( Update, SIGNAL( clicked() ), this, SLOT( jobUpdate() ) ); 
    connect( DeleteProject, SIGNAL(clicked()),this,SLOT( deleteJob()));
	connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( close() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( SourceBrowse, SIGNAL( clicked() ), this, SLOT( SourceBrowse_clicked() ) );
    connect( SourceBrowse, SIGNAL( pressed() ), SourceFile, SLOT( selectAll() ) );
    connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( SourceBrowse_clicked() ) );
    connect( DestinationBrowse, SIGNAL( pressed() ), DestinationFile, SLOT( selectAll() ) );
    connect( DestinationBrowse, SIGNAL( clicked() ), this, SLOT( DestinationBrowse_clicked() ) );
    connect( SaveList, SIGNAL( pressed() ), this, SLOT( SaveList_clicked() ) );
    connect( OpenList, SIGNAL( pressed() ), this, SLOT( OpenList_clicked() ) );
    connect( FindClientButton, SIGNAL( clicked() ), FindClientName, SLOT( selectAll() ) );
    connect( FindClientButton, SIGNAL( clicked() ), this, SLOT( FindClientButton_clicked() ) );
    connect( AddClient, SIGNAL( clicked() ), this, SLOT( AddClient_clicked() ) );
    connect( RemoveClient, SIGNAL( pressed() ), this, SLOT( RemoveClient_clicked() ) );
    connect( AddAll, SIGNAL( clicked() ), this, SLOT( AddAll_clicked() ) );
    connect( RemoveAll, SIGNAL( clicked() ), this, SLOT( RemoveAll_clicked() ) );
    connect( fileRender, SIGNAL( activated() ), this, SLOT( fileRender_activated() ) );
    //connect( fileNewAction, SIGNAL( activated() ), this, SLOT( SourceBrowse_clicked() ) );
	connect( DNSSuffix, SIGNAL( textChanged(const QString &)),this, SLOT( DNSSuffix_changed(const QString &) ) );
	connect( SocketNumber, SIGNAL( valueChanged(int)), this, SLOT(SocketNumber_changed(int)));
	connect( PrioritySpin, SIGNAL( valueChanged(int)), this, SLOT(UpdatePriority(int)));
	connect( start, SIGNAL( clicked() ),this, SLOT(	fileRender_activated() ) );
    connect( aServer, SIGNAL(changed(Job*,QString,QString,int)), this, SLOT(statusUpdate(Job*,QString,QString,int)));
    connect( this, SIGNAL( ClientAdded(QString)),this,SLOT(ClientAdd(QString)));
	connect( PauseProject, SIGNAL( clicked() ),this,SLOT(pauseJob()));
	// tab order
    setTabOrder( Tab, ProjectList );
    setTabOrder( ProjectList, Update );
    setTabOrder( Update, PrioritySpin );
    setTabOrder( PrioritySpin, SourceFile );
    setTabOrder( SourceFile, SourceBrowse );
    setTabOrder( SourceBrowse, DestinationFile );
    setTabOrder( DestinationFile, DestinationBrowse );
    setTabOrder( DestinationBrowse, AvailableClients );
    setTabOrder( AvailableClients, UsedClients );
    setTabOrder( UsedClients, AddClient );
    setTabOrder( AddClient, RemoveClient );
    setTabOrder( RemoveClient, AddAll );
    setTabOrder( AddAll, RemoveAll );
    setTabOrder( RemoveAll, OpenList );
    setTabOrder( OpenList, SaveList );
    setTabOrder( SaveList, FindClientName );
    setTabOrder( FindClientName, FindClientButton );
	//    setTabOrder( FindClientButton, ClientSearch );
}


/*  
*  Destroys the object and frees any allocated resources
*/
Controller::~Controller()
{
    // no need to delete child widgets, Qt does it all for us
}

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void Controller::DNSSuffix_changed(const QString &str)
{
	Domain=str;
}

void Controller::SocketNumber_changed(int socket)
{
	if ((socket>65535 || socket<1024))
	{
		SocketNumber->setValue(4242);
	}
	for(int i = 0; i < int(AvailableClients->count()); i++)
    {
		if(AvailableClients->isSelected(i))
		{
			// find the selected items
			QListBoxItem* item = AvailableClients->item(i);
			QString temp = item->text();
			AvailableClients->takeItem(item);
			// update the client to the correct/new setting
			temp = temp.section(":", 0, 0);
			temp.append(":");
			temp.append(SocketNumber->text());
			// put the client back in
			AvailableClients->insertItem(temp, 0);
			AvailableClients->sort(TRUE);
			// and reselect the client so that you can just spin the
			// box up and down without having to reselect the client
			item = AvailableClients->findItem(temp, Qt::ExactMatch);
			if (item)
				AvailableClients->setSelected(item, true);
		}
    }
}

void Controller::UpdatePriority(int newPriority)
{
	QListViewItem *current=ProjectList->selectedItem();
	if (current)
	{
		aServer->setPriority((current->text(6).toInt()), newPriority);
		int i=Projects.find(current);
		Projects.at(i)->setText(1,QVariant(newPriority).toString());
	}
}

void Controller::jobUpdate()
{
	Job *job = NULL;
	collectedJobs collection;
	QString name,status;
	QString file=SourceFile->text();
	if (!file.isEmpty())
	{
		if (int(file.find(".proj"))!=-1)
		{
			wcsClient TempClient;
			int startframe=0,endframe=0,frameInterval=1, pri=0;
			// bool result = aServer->parser(startframe,endframe,frameInterval,name,file);
			// bool result = TempClient.parse(file, startframe, endframe, frameInterval, name);
			collection = TempClient.parse(file);
			// result is a string of CSV's, separated by semicolons for each job
			// (assuming more than one job in the parse, otherwise it's unnecessary)
			// format:
			// job name, total frames, start frame, end frame, priority
			if (!collection.isEmpty()) {
				while(!collection.isAtEnd()) {
					renderInfo* temp = collection.getNextJob();
					// ID++; // increment Id.  Never decrement, keeps everything unique
					name = temp->jobName;
					startframe = temp->startFrame;
					endframe = temp->endFrame;
					frameInterval = temp->frameInterval;
					pri = temp->priority;
					if(pri < 0) { pri = PrioritySpin->value(); }
					//					if((job=new Job(PrioritySpin->value(), ID, name, file)))
					if((job=new Job(pri, ID, name, file)))
					{
						for(int x=startframe;x<=endframe;x+=frameInterval)
							job->add(QVariant(x).toString());
						if (aServer->AddJobToScheduler(job)) {
							QString tmp;
							Projects.append(new QListViewItem(ProjectList, name, QVariant(pri).toString(),
								status, "", "", "", tmp.setNum(ID)));
						}
						else
							QMessageBox::warning(0,0,"Project already exists");
						ID++;
					} // if
				} // while
			} // if
			else
			{
				QMessageBox::warning(0,0,"File type not recognized");
				return;
			} // else
			status=" 0 of "+QVariant(job->totalLength()).toString();
		}
		else return;
		SourceFile->clear();
	}
	
	else
	{
		return;
	}
	
	Projects.setAutoDelete(true);
}

void Controller::pauseJob()
{
	if (PauseProject->text()==QString("Pause Project") && !Projects.isEmpty())
	{
		PauseProject->setText("Unpause Project");
		aServer->pauseJob();
	}
	else
	{
		PauseProject->setText("Pause Project");
		aServer->pauseJob();
		PauseProject->setOn( FALSE );
	}
}

void Controller::deleteJob()
{
	QListViewItem *current = ProjectList->selectedItem();
	if (current)
	{
		if (aServer->removeJob(current->text(6).toInt()))
			Projects.remove(current);
	}
}

void Controller::ClientAdd(QString addy)
{
	if (!Clients.isEmpty())
	{
		Clients.at(0);
		do
		{
			if(Clients.current()->text(0) == addy)
				return;
		}while(Clients.next());
	}
	Clients.append(new QListViewItem(ClientList,addy));
	
}

void Controller::helpAbout()
{
    QMessageBox::information(0, "Super Conductor from 3D Nature", "Design & Implementation:\nColorado School of Mines 2002 SuperConductor Team\n Cedar Cranson\t David Kopp\n Andrew Warner\t Michael Widener\nAdditional Coding, Initial Concept: Chris \'Xenon\' Hanson\n\nwww.Super-Conductor.org");
}

void Controller::fileRender_activated()
{
    if(UsedClients->count() == 0)
    {
		QMessageBox::information(0,  "Error", "Please Select Clients");
		return;
    }
	
	if(Projects.isEmpty())
	{
		QMessageBox::information(0,  "Error", "Please Create a Project");
		return;
	}
    QStringList Clients;
    for(int i = 0; i < int(UsedClients->count()); i++)
    {
		//UsedClients
		Clients << UsedClients->text(i);
    }
	aServer->setClients(Clients);
	infoText->append( "Render signal sent to all clients\n" );
	emit rendering();
}

void Controller::SourceBrowse_clicked()
{  
    QString fn = QFileDialog::getOpenFileName( QString::null , QString::null);
    
    if ( !fn.isEmpty() )
    {
		SourceFile->insert(fn);
    }
	
}

void Controller::DestinationBrowse_clicked()
{
	QString fn = QFileDialog::getExistingDirectory();
    
    if(!fn.isEmpty())
    {
		DestinationFile->insert(fn);
    }
}

void Controller::OpenList_clicked()
{
    QString extension("Client Lists (*.lst)");
    QString fn = QFileDialog::getOpenFileName(QString::null, extension);
    char temp[50];
	
    if(!fn.isEmpty())
    {
		ifstream infile;
		infile.open(fn.data());
		
		for(int i = 0; i < int(UsedClients->count()); )
		{
			QListBoxItem* tempItem = UsedClients->item(0);
			
			UsedClients->takeItem(tempItem);
			AvailableClients->insertItem(tempItem, 0);
			AvailableClients->sort(TRUE);
		}
		
		while(infile.getline(temp, 50))
		{
			extension.truncate(0);
			extension.append(temp);
			
			UsedClients->insertItem(temp, 0);
			UsedClients->sort(TRUE);
		}
	}
	
	for(int i = 0; i < int(AvailableClients->count()); i++)
	{
		for(int j = 0; j < int(UsedClients->count()); j++)
		{
			if(AvailableClients->text(i) == UsedClients->text(j))
			{
				AvailableClients->removeItem(i);
			}
		}
	}
}


void Controller::SaveList_clicked()
{
    QString extension("Client Lists (*.lst)");
    QString fn = QFileDialog::getSaveFileName(QString::null, extension);
	int test = fn.find(".",0);
	
	fn.truncate(test);
	
	fn.append(".lst");
	
    if(!fn.isEmpty() && int(UsedClients->count()) != 0)
    {
		ofstream outfile;
		outfile.open(fn.data());
		
		//		outfile << Domain.data() << endl;
		
		for(int i = 0; i < int(UsedClients->count()); i++)
		{
			QString temp = UsedClients->text(i);
			outfile << temp.data() << endl;
		}
		
		outfile.close();
	}
}


void Controller::FindClientButton_clicked()
{
	QString fn = FindClientName->text();
	if (!fn.isEmpty()) {
		if (!Domain.isEmpty()) {
			fn.append(".");
			fn.append(Domain);
		}
		fn.append(":");
		fn.append(SocketNumber->text());
		FindClientName->clear();
		bool repeat = FALSE;
		int i;
		
		for(i = 0; i < int(AvailableClients->count()); i++)
		{
			if(AvailableClients->text(i) == fn)
				repeat = TRUE;
		}
		
		for(i = 0; i < int(UsedClients->count()); i++)
		{
			if(UsedClients->text(i) == fn)
				repeat = TRUE;
		}
		
		if(!repeat)
		{
			AvailableClients->insertItem(fn, 0);
			AvailableClients->sort(TRUE);
		}
	}
}


void Controller::AddClient_clicked()
{
    
    for(int i = 0; i < int(AvailableClients->count()); i++)
    {
		if(AvailableClients->isSelected(i))
		{
			QListBoxItem* temp = AvailableClients->item(i);
			AvailableClients->takeItem(temp);
			UsedClients->insertItem(temp);
			UsedClients->sort(TRUE);
		}
    }
}


void Controller::RemoveClient_clicked()
{
	
    for(int i = 0; i < int(UsedClients->count()); i++)
    {
		if(UsedClients->isSelected(i))
		{
			QListBoxItem* temp = UsedClients->item(i);
			
			UsedClients->takeItem(temp);
			AvailableClients->insertItem(temp);
			AvailableClients->sort(TRUE);
		}
    }
}


void Controller::AddAll_clicked()
{
    for(int i = 0; i < int(AvailableClients->count()); )
    {
		QListBoxItem* temp = AvailableClients->item(0);
		
		AvailableClients->takeItem(temp);
		UsedClients->insertItem(temp);
		UsedClients->sort(TRUE);
    }
}

void Controller::RemoveAll_clicked()
{
    for(int i = 0; i < int(UsedClients->count()); )
    {
		QListBoxItem* temp = UsedClients->item(0);
		
		UsedClients->takeItem(temp);
		AvailableClients->insertItem(temp, 0);
		AvailableClients->sort(TRUE);
    }
}

void Controller::statusUpdate(Job* job,QString client,QString stat,int percent)
{
	QString name;
	QString status;
	if (job)
	{
		int id=job->getJobID();
		
		status=QVariant(job->totalLength()-job->length()).toString()+" of "+QVariant(job->totalLength()).toString();
		for (unsigned int i=0;i<Projects.count();i++)
		{
			if (Projects.at(i)->text(6).toInt()==id)
			{
				Projects.at(i)->setText(2,status);
				int days=0;
				Projects.at(i)->setText(3,job->avgRenderTime(days).toString("hh:mm:ss.zzz"));
				QTime Est=job->getEstimatedTime(days);
				if (job->isFinished() || Est>QTime(0,0,0,0)) Projects.at(i)->setText(5,Est.toString("hh:mm:ss:zzz"));
				else if (days>0) Projects.at(i)->setText(5,QVariant(days).toString());
				else Projects.at(i)->setText(5,"No time estimate available");
				if (job->isFinished())
				{
					Projects.at(i)->setText(4,(job->getFinish() - job->getStart()).toString("hh:mm:ss.zzz")+" ");
				}
				break;
			}
		}
	}
	else
	{
		name=client;
		for (unsigned int i=0;i<Clients.count();i++)
		{
			if (Clients.at(i)->text(0).find(name)!=-1)
			{
				QString s=stat.section("?",-1,-1);
				//s.setLength(s.length()-1);
				Clients.at(i)->setText(1,stat.section("?",0,0));
				Clients.at(i)->setText(2,s);
				Clients.at(i)->setText(3,QVariant(percent).toString()+'%');
			}
		}
	}
}

void Controller::connectChange(const QString& str)
{
	infoText->append(str);
	if (str.find("found")!=-1) emit ClientAdded(str.section(" ",1,1));
}



void Controller::socketJobDone(const QString& str)
{
	infoText->append(str);
}

