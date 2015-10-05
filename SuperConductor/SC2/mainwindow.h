/****************************************************************************
** Form interface generated from reading ui file 'ui/BaseSC.ui'
**
** Created: Mon Jan 3 13:43:06 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SCMAIN_H
#define SCMAIN_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

#include "datatypes.h"
#include "server.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTable;
class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;

class SCMain : public QMainWindow
{
    Q_OBJECT

public:
    SCMain( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~SCMain();
    void setServer( SCServer* );
    void setMessages( SCMessageList* );
    
    QGroupBox* GroupBox2;
    QLabel* PriorityLabel;
    QPushButton* Run;
    QSpinBox* PrioritySpin;
    QPushButton* Pause;
    QPushButton* Stop;
    QPushButton* RemoveProject;
    QPushButton* AddProject;
    QGroupBox* GroupBox3;
    QPushButton* MessageButton;
    QGroupBox* GroupBox4;
    QLabel* TextLabel2;
    QPushButton* AddClient;
    QPushButton* RemoveClient;
    QPushButton* LoadList;
    QPushButton* SaveList;
    QLabel* TextLabel1;
    QTable* ClientSettingList;
    QListBox* ClientList;
    QGroupBox* GroupBox1;
    QListView* ProjectList;
    QMenuBar *menubar;
    QPopupMenu *Project;
    QPopupMenu *Client;
    QPopupMenu *helpMenu;
    QAction* fileNewAction;
    QAction* fileExitAction;
    QAction* helpAboutAction;
    QAction* clientOpenAction;
    QAction* clientSaveAction;
    
	SCMessageList* messages;
    QPtrList<QListViewItem> projects;
    QPtrList<SCClient> clients;
    QPtrList<SCClientListItem> listboxList;

public slots:
    virtual void fileExit();
    virtual void helpAbout();
    virtual void projectAdd();
    virtual void projectRemove();
    virtual void clientOpen();
    virtual void clientSave();
    virtual void clientAdd();
    virtual void clientRemove();
    virtual void runClicked();
    virtual void pauseClicked();
    virtual void stopClicked();
	virtual void applyClientSettings(int, int);
    virtual void priorityChanged(int newVal);
    virtual void projectSelectionChanged();
    virtual void clientHighlightChanged(QListBoxItem* theItem);
    virtual void showMessages();
    virtual void settingSelectedChanged(int row, int col);

	virtual void projectFinished(SCProject*);
	virtual void updateDisplay(SCProject*); // generic display update call, updates all fields
	virtual void updateTimer(SCProjectListItem*);

protected:
    QGridLayout* SCMainLayout;
    QGridLayout* GroupBox2Layout;
    QVBoxLayout* GroupBox3Layout;
    QGridLayout* GroupBox4Layout;
    QSpacerItem* spacer1;
    QVBoxLayout* GroupBox1Layout;
    
    SCServer* theServer;
    QString temporaryStorage;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;

};

#endif // SCMAIN_H
