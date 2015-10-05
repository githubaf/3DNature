// wcsclient.h

#include <qstring.h>
#include <qsocket.h>
#include <qtable.h>
#include "../client.h"
#include "../datatypes.h"

#ifndef ESCCHAR
#define ESCCHAR
const QChar escchar(27);
#endif

#ifndef WCS_CLIENT
#define WCS_CLIENT

class wcsClient : public SCClient {
	// COMMENT BEFORE Q_OBJECT
	Q_OBJECT
	// COMMENT AFTER Q_OBJECT
public:
	// needs a default constructor, no arguments (can't standardize on socket comm)
	wcsClient();
	// default destructor, for thoroughness
	~wcsClient();
	// returns a collectedJobs item, takes in a filename + path
	collectedSCProjects& parse(QString);
	// returns a pointer list to Setting objects, pairs
	QPtrList<Setting> getConfig();
	// should pop up a window so you can set client specific attributes
	void setConfig(QPtrList<Setting>);
	// starts the rendering on the client.  Require that all configuration
	// be set in the setConfig function, only take in frame number
	QString identity();
	QString extensions();
	void load(QString);
	QString loadedFile();
	void render(QString, long int);
	// halts the rendering on the client completely, flushes any cache, etc.
	void halt();
	void pause();
   void unpause();

private slots:
	void readyRead();
	void error(int);
	void connected();
	void connectionClosed();

private:
	// All of these functions are specific to the WCS/VNS client
	// There shouldn't be any public functions other than the ones
	// described in the Client class header file

	// Enumeration of message values
	enum MessageType { Undefined, Progress, Result, Err, Loaded, Quit, Output };
	// Analyse messages from taken in from client
	MessageType analyzeMessage(QString& str, QString &i);
	
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
	//Precondition:  The unsigned long contains a byte value
	//               Example: 0xddccbbaa
	//Postcondition: The long contains an (relatively) swapped byte value
	//               Example: 0xaabbccdd

	// private data members (setting is recommended for all client types)
	// however, you can choose your own method for storing settings, as long
	// as they can be converted into a QPtrList<Setting> type in getConfig()
	QPtrList<Setting> settings;
	QString internalString;
	QString fileName, projectName;
	bool outputFound;
	QSocket* theSocket;
   int currentFrame;
};

#endif // WCS_CLIENT
