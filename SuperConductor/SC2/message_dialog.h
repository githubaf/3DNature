/****************************************************************************
** Form interface generated from reading ui file 'superconductor_messages.ui'
**
** Created: Fri Jul 11 12:57:11 2003
**      by: The User Interface Compiler ($Id: message_dialog.h,v 1.2 2003/09/11 23:03:53 pitabred Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SUPERCONDUCTOR_MESSAGES_H
#define SUPERCONDUCTOR_MESSAGES_H

#include <qvariant.h>
#include <qdialog.h>
#include "datatypes.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QListBox;
class QListBoxItem;
class QPushButton;
class QStringList;

class Superconductor_Messages : public QDialog
{
    Q_OBJECT

public:
	Superconductor_Messages( SCMessageList* sl = 0, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Superconductor_Messages();

	QGridLayout* layout;
    QListBox* listBox;
	QPushButton* save;
	QPushButton* closeButton;
	SCMessageList* strings;

protected:

protected slots:
	virtual void closeDialog();
	virtual void saveOutput();
    virtual void languageChange();

};

#endif // SUPERCONDUCTOR_MESSAGES_H
