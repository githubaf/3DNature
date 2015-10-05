/****************************************************************************
** Form interface generated from reading ui file 'BaseSC.ui'
**
** Created: Fri Feb 28 15:25:30 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SCMAIN_H
#define SCMAIN_H

#include <qvariant.h>
#include <qmainwindow.h>
#include "datatypes.h"
#include "server.h"

class QTable;
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QGroupBox;
class QLabel;
class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;
class QPushButton;
class QSpinBox;

class SCMain : public QMainWindow
{ 
    Q_OBJECT

public:
    SCMain( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~SCMain();

    QGroupBox* GroupBox1;
    QGroupBox* GroupBox2;
    QGroupBox* GroupBox3;
    QGroupBox* GroupBox4;
    QPushButton* Run;
    QPushButton* Pause;
    QPushButton* Stop;
    QPushButton* AddProject;
    QPushButton* RemoveProject;
    QPushButton* LoadList;
    QPushButton* SaveList;
    QPushButton* AddClient;
    QPushButton* RemoveClient;
    QPushButton* MessageButton;
    QLabel* PriorityLabel;
    QLabel* TextLabel1;
    QLabel* TextLabel2;
    QSpinBox* PrioritySpin;
    QListView* ProjectList;
    QTable* ClientSettingList;
    QListBox* ClientList;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
	QPopupMenu *clientMenu;
    QPopupMenu *helpMenu;
    QAction* fileNewAction;
	QAction* fileExitAction;
    QAction* clientOpenAction;
    QAction* clientSaveAction;
    QAction* helpAboutAction;
	
	QStringList* messages;
	QPtrList<QListViewItem> projects;
	QPtrList<SCClient> clients;
	QPtrList<SCClientListItem> listboxList;
	
	bool close();
	void setServer( SCServer* );
	void setMessages( QStringList* );

public slots:
    virtual void fileNew();
	virtual void projectRemove();
    virtual void clientOpen();
    virtual void clientSave();
    virtual void fileExit();
    virtual void helpAbout();
	virtual void runPressed();
	virtual void pausePressed();
	virtual void stopPressed();
	virtual void projectSelectionChanged();
	virtual void applySettings();
	virtual void addClient();
	virtual void showMessages();
	virtual void removeClient();
	virtual void priorityChanged(int);
	virtual void ClientSettingSelect(int, int);
	virtual void ClientHighlightChanged( QListBoxItem* );

protected:
	SCServer* theServer;
	QString temporaryStorage;
};

#endif // SCMAIN_H
