// wcsclient.h

#ifndef WCS_CLIENT
#define WCS_CLIENT

#define WCS_NO_DLL

#ifndef WCS_NO_DLL
#define WCS_API __declspec(dllexport)
#else
#define WCS_API

#endif // WCS_NO_DLL

#include <qstring.h>
#include <qsocket.h>
#include "clientDataTransfer.h"

#ifndef ESCCHAR
#define ESCCHAR

const QChar escchar(27);

#endif

/*
// connect to filename however it needs done
bool load (filename) // return true if ok

// stop the execution of the render
bool halt () // return true as well if ok

// start the render on projectname and frame
bool render (projectname, frame) // return true if sent alright

// return the current status updates in string form
QString status() // return string containing information in CSV form

// parse the data file filename, returns ok or not if parsed
bool parse (filename, &startframe, &endframe, &framestep, &projname)
*/

class wcsClient : public QSocket {
	Q_OBJECT
public:
	// clientName = client name it clients dialog (+ DNS suffix)
	// sockNum is purely for TCP/IP support.  Ignore if not needed.
	WCS_API wcsClient();
	WCS_API wcsClient(QString hostName, int sockNum, QObject* parent);
	WCS_API ~wcsClient();

	// issue the load command to your client on the file filename(includes path)
	// relativePath is the relative path put into the relative pathname box
	// in the SuperConductor main interface
	void load(QString filename, QString relativePath);
	// halt execution of the current render job on the client
	void halt();
	// renders the frame number on the project name
	void render(QString projName, int frame);
	// connects to the given host and on the socket
	void connectTo(QString host, int socket);
	// connects to the stored host and on the socket
	void connectToStored(void);
	// parses a file of the local type
//	bool parse (QString filename, int &startframe, int &endframe, int &frameInterval, QString& projectName);
	WCS_API collectedJobs parse (QString);
	//Precondition:  Four integers and a QString are passed by reference along with a QString
	//               which corrisponds to the name of the file to be parsed
	//Postcondition: The four integers contain the values of the starting frame,
	//               ending frame, frame interval, and port number listened to respectively
	//				 The refereced QString contains the name of the render job
	//               The return integer tells the user the success of the file parsing
	//               return 0: parsing was successful
	//               return 1: file was not opened successfully
	//               return 2: not a WCS File format
	//               return 3: byte swapping technique is not recognized
	//               return 4: bad file type
	//               return 5: version number is less than 4



	// status returns the current statusBuffer
	WCS_API QString status();
	// address returns the address of the current machine interface it is using
	QHostAddress address() { return QSocket::address(); }
	// peerAddress returns the address that it is connected to
	QHostAddress peerAddress() { return QSocket::peerAddress(); }

signals:
	// Status string samples (status called in main program after updated() issued):
	// PROGRESS?ACTION?STATUS
	// RESULT?TRUE/FALSE
	// CONNECTED?(system text)
	// ERROR?FATAL/RECOVERABLE?(error text)
	// ECHO?(any text you want echoed to system message screen)
	// OUTPUT?(filepath)  This also means that the current frame is finished
	void updated();

private slots:
	void error(int);
	void connected();
	void connectionClosed();
	void socketReadyRead();

private:
	enum MessageType { Undefined, Progress, Result, Err, Loaded, Quit, Output };
	//functions
	MessageType analyzeMessage(QString& str, QString &i);
	//data members
	QString statusBuffer;
	QString host;
	int sock;
	bool outputFound;

	void goToNextChunk(QTextStream&, unsigned long);
	//Precondition:  A QTextStream object that is linked to an open file is passed
	//               by reference, the unsigned long corrisponds to the number of bytes
	//               to be skipped to get to the next chunk
	//Postcondition: The QTextStream is read in the first byte of the next chunks text
	//               identifier in the next readRawBytes call

	void getSize(char*, unsigned long&);
	//Precondition:  The char* contains four bytes that denote a size of a chunk
	//Postcondition: The unsigned long contains the chunk size

	void byteSwap(unsigned long&);
	//Precondition:  The unsigned long contains a swapped byte value
	//               Example: 0xddccbbaa
	//Postcondition: The long contains an unswapped byte value
	//               Example: 0xaabbccdd

};

#endif // WCS_CLIENT
