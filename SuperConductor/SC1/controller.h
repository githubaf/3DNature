// controller.h

/****************************************************************************
** Form interface generated from reading ui file 'Controller.ui'
**
** Created: Thu May 23 15:44:18 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define WCS_NO_DLL

#include <qvariant.h>
#include <qmainwindow.h>
#include "client.h"
#include "serverobject.h"
#include "qfiledialog.h"
#include "qmessagebox.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QWidget;
class ServerObject;

class Controller : public QMainWindow
{ 
    Q_OBJECT

public:
    Controller( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~Controller();

    QTabWidget* Tab;
    QWidget* tab;
    QLabel* TextLabel4;
    QLabel* TextLabel3;
    QLabel* TextLabel6;
    QLabel* TextLabel5;
    QLineEdit* DestinationFile;
    QLineEdit* SourceFile;
	QLineEdit* DNSSuffix;
	QSpinBox* SocketNumber;
	QPtrList<QListViewItem> Projects;
	QLabel* DNSLabel;
	QLabel* SocketLabel;
	QPushButton *PauseProject;
	QPushButton *start;
    QPushButton* DestinationBrowse;
    QPushButton* Update;
	QPushButton* DeleteProject;
    QSpinBox* PrioritySpin;
    QListView* ProjectList;
    QPushButton* SourceBrowse;
    QWidget* tab_2;
    QLabel* TextLabel7;
    QLabel* TextLabel2;
    QLineEdit* FindClientName;
    QLabel* TextLabel8;
    QListBox* UsedClients;
    QListBox* AvailableClients;
    QPushButton* AddClient;
    QPushButton* RemoveClient;
    QPushButton* AddAll;
    QPushButton* RemoveAll;
    QPushButton* OpenList;
    QPushButton* SaveList;
    QPushButton* FindClientButton;
//    QLabel* TextLabel1;
//    QPushButton* ClientSearch;
    QWidget* tab_3;
	QTextView *infoText;
	ServerObject *aServer;

	
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QPopupMenu *helpMenu;
    //QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileExitAction;
    QAction* helpContentsAction;
    QAction* helpIndexAction;
    QAction* helpAboutAction;
    QAction* fileRender;
	QString Domain;
	QWidget* tab_4;
	QListView* ClientList;
	QPtrList<QListViewItem> Clients;


signals:
    void clicked();
	void ClientAdded(QString);
	void rendering();

public slots:
	void socketJobDone(const QString& str);
	void connectChange(const QString&);
	//    virtual void fileNew();
	//    virtual void fileOpen();
	//    virtual void fileSave();
	//    virtual void fileSaveAs();
	//    virtual void filePrint();
    virtual void fileExit() {};
    virtual void helpIndex() {};
    virtual void helpContents() {};
    virtual void helpAbout();
	void statusUpdate(Job *,QString,QString,int);
    void fileRender_activated();
    virtual void SourceBrowse_clicked();
    virtual void DestinationBrowse_clicked();
    virtual void OpenList_clicked();
    virtual void SaveList_clicked();
    virtual void FindClientButton_clicked();
    virtual void AddClient_clicked();
    virtual void RemoveClient_clicked();
    virtual void AddAll_clicked();
    virtual void RemoveAll_clicked();
	void UpdatePriority(int);
	void jobUpdate();
	void pauseJob();
	void deleteJob();
	void DNSSuffix_changed(const QString &);
	void SocketNumber_changed(int);
	void ClientAdd(QString);

protected:
private:
	uint ID;  // identification variable for job creation
};

#endif // CONTROLLER_H


