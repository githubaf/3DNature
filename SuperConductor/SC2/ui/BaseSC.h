/****************************************************************************
** Form interface generated from reading ui file 'BaseSC.ui'
**
** Created: Wed Dec 29 15:55:13 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SCMAIN_H
#define SCMAIN_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

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
    QListBox* ClientList;
    QListView* ClientSettingList;
    QLabel* TextLabel2;
    QPushButton* AddClient;
    QPushButton* RemoveClient;
    QPushButton* ApplySettings;
    QPushButton* LoadList;
    QPushButton* SaveList;
    QLabel* TextLabel1;
    QGroupBox* GroupBox1;
    QListView* ProjectList;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QPopupMenu *helpMenu;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* fileExitAction;
    QAction* helpAboutAction;

public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void filePrint();
    virtual void fileExit();
    virtual void helpIndex();
    virtual void helpContents();
    virtual void helpAbout();

protected:
    QGridLayout* SCMainLayout;
    QGridLayout* GroupBox2Layout;
    QVBoxLayout* GroupBox3Layout;
    QGridLayout* GroupBox4Layout;
    QSpacerItem* spacer1;
    QVBoxLayout* GroupBox1Layout;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;
    QPixmap image6;

};

#endif // SCMAIN_H
