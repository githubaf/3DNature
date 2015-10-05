// wcs-client.cpp

#include <iostream>

using namespace::std;

#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qtable.h>

#include "wcs-client.h"

extern "C" SCClient* create() {
	 return new wcsClient;
}

extern "C" void destroy(SCClient* cli) {
	delete cli;
}

wcsClient::wcsClient() {
	// simple initialization
	// assume that updated is connected
	settings.setAutoDelete(TRUE);
	settings.append(new Setting("port", "4242"));
	settings.append(new Setting("host", ""));
	
	theSocket = new QSocket;
	load_ok = false;
	irreco = false;
	// do the socket connections here, socket signals to local slots
	connect(theSocket, SIGNAL(error(int)), this, SLOT(error(int)));
	connect(theSocket, SIGNAL(connected()), this, SLOT(connected()));
	connect(theSocket, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()));
	connect(theSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

wcsClient::~wcsClient() {
	// need to purge all values in settings
	delete theSocket;
}

collectedSCProjects& wcsClient::parse(QString file)
{
	collectedSCProjects* jobs = new collectedSCProjects;
	QFile infile(file);
	//	int startFrame, endFrame, frameInterval;
	QString line;
	QString summaryStream;
	QString JobNameTemp, CamNameTemp, OptNameTemp;
	int JobEnabledTemp, FrameIntervalTemp, StartFrameTemp, EndFrameTemp;
	int WidthTemp, HeightTemp, SegmentsTemp, JobPriTemp;
	double JobFrameRateTemp, StartTimeTemp, EndTimeTemp;
	QTextStream* summary;
	QString piece;
	char *temp;
	unsigned long size;
	bool *ok, swapped;
	int version, revision;

	ok = new bool;
	if(infile.open(IO_ReadOnly))
	{
		QTextStream info(&infile);
		
		//reads in the first 8 bytes as first file format checking method
		temp = new char[8];
		info.readRawBytes(temp, 8);
		line.sprintf(temp);
		line.truncate(8);
		
		//if the first 8 bytes of the file are not the string
		//"WCS File" then the format is incorrect and 2 is returned
		if(line != "WCS File")
		{
			infile.close();
			return *jobs;
		}
		
		
		delete [] temp;
		line.truncate(0);
		
		//takes in the file version and revision bytes
		temp = new char[2];
		info.readRawBytes(temp, 2);
		
		version = temp[0];
		revision = temp[1];
		
		if(version < 4) {
			infile.close();
			// return 5;
			return *jobs;
		}
		
		delete [] temp;
		line.truncate(0);
		
		//reads in the 4 byte, byte swapping constant
		//if no swapping occurs, the value will be 0xaabbccdd
		temp = new char[4];
		info.readRawBytes(temp, 4);
		getSize(temp, size);
		
		
		//checks the byteswapping of the system
		//if the bytes are not swapped, then swapped = false
		//if the bytes are Endian swapped, then swapped is true
		//if the bytes are neither, 3 is returned as error message
		if(size == 0xaabbccdd)
		{
			swapped = false;
		}
		else if(size == 0xddccbbaa)
		{
			swapped = true;
		}
		else
		{
			infile.close();
			// return 3;
			return *jobs;
		}
		
		delete [] temp;
		line.truncate(0);
		
		
		//takes in the name of the first chunk
		temp = new char[8];
		info.readRawBytes(temp, 8);
		line.sprintf(temp);
		line.truncate(8);
		
		delete [] temp;
		
		//goes through the chunks until the summary chunk is found
		while(line != "Summary\0" && !info.atEnd())
		{
			temp = new char[4];
			info.readRawBytes(temp, 4);
			getSize(temp, size);
			if(swapped == true)
				byteSwap(size);
			
			goToNextChunk(info, size);
			
			delete [] temp;
			line.truncate(0);
			
			temp = new char[8];
			info.readRawBytes(temp, 8);
			line.sprintf(temp);
			line.truncate(8);
			
			
			if((line == "様様様様") || (info.atEnd())) {
				infile.close();
				// return 4;
			    return *jobs;
			}
			
			delete [] temp;
		}
		
		line.truncate(0);
		
		temp = new char[4];                     //reads in size of Summary
		info.readRawBytes(temp, 4);             //not necessary to save
		getSize(temp, size);					//get the size of the summary chunk
		delete [] temp;
		temp = new char[size];					// reallocate temp to hold whole array
		info.readRawBytes(temp, size);			// read the whole summary chunk into temp
		summaryStream = temp;					// make a string from the summary
		summary = new QTextStream(&summaryStream, IO_ReadOnly);
		
		// we need to parse according to value, not simply order
		// set a loop here, parse the lines, evaluate according to
		// their value
		// line = QString, info = QTextStream
		line = summary->readLine();
		// this first loop reads in all the options lines, deals with what they have
		while (line != "") {
			// we got a new line
			line = summary->readLine();
			// now figure out what is in it (no endline on string)
			// if the line is empty (but not null) then it is the break between the
			// two "sections", so just get the next two lines after that
		}
		// Now read one or more lines.  Parse the
		// project data, including things like frames, size, etc.
		// put them into the "incoming" parameters
		// int& startFrame, int& endFrame, int& frameInterval, int& port, QString &job
		
		// when we hit a blank line we're done
		for(line = summary->readLine(); line != "";) {
			int ThisField = 0;
			piece = line.section(",", ThisField, ThisField);
			JobNameTemp = piece.section("\"", 1, 1);
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			CamNameTemp = piece.section("\"", 1, 1);
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			OptNameTemp = piece.section("\"", 1, 1);
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			JobPriTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			JobEnabledTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			JobFrameRateTemp = piece.toDouble();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			FrameIntervalTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			StartFrameTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			EndFrameTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			StartTimeTemp = piece.toDouble();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			EndTimeTemp = piece.toDouble();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			WidthTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			HeightTemp = piece.toInt();
			ThisField++;
			
			piece = line.section(",", ThisField, ThisField);
			SegmentsTemp = piece.toInt();
			ThisField++;
			
			// read zero or more lines corresponding to output events
			for(line = summary->readLine(); line[0] == 0x09; line = summary->readLine())
			{
				// Output Event line (7 fields)
				// Output Format type, Enabled State, Path and File text
				// Number frame digits, Extension text (if used)
				// Temp Path, Save Before Post
				// ignored for now as we are not doing any sort of output validation
			} // for
			// 	renderInfo(int pri, int fi, int sf, int ef, QString jn, renderInfo* nextRI=NULL)
			if(JobEnabledTemp == 1) {
				jobs->addSCProject(*(new renderInfo(JobPriTemp, FrameIntervalTemp, StartFrameTemp, EndFrameTemp, JobNameTemp)));
			} // if
		} // for
	} // if infile close
	else {
		infile.close();
		return *jobs;
	}
	infile.close();
	return *jobs;                               //returns 0 if no problems with parsing
}

QPtrList<Setting> wcsClient::getConfig() {
	return settings;
}

void wcsClient::setConfig(QPtrList<Setting> s) {
	while (settings.current()) {
		settings.remove();
	}
	settings = s;
}

QString wcsClient::identity() {
	return (QString)"WCS/VNS";
}

QString wcsClient::extensions() {
	return (QString)"proj";
}

void wcsClient::load(QString fn) {
	// we need to connect to the 'server', IE, the WCS/VNS program now
	// get the address and such from settings
	// settings is a QPtrList<Setting> variable... loop through
	// to get the stuff we need
	load_ok = false;
	Setting* set = settings.first();
	QString host;
	uint port;
	while ( set ) {
		// settings necessary are port and host
		if ( set->name() == "host" )
			host = set->value();
		else if ( set->name() == "port" )
			port = set->value().toUInt();
		set = settings.next();
	}
	theSocket->connectToHost( host, port );
	fileName = fn;
	QTextStream os(theSocket);
	os << "project load " << fn << "\n";
	internalString = "Project load ";
//	emit changed("Project Loaded successfully", this);
}

QString wcsClient::loadedFile() {
	return fileName;
}

void wcsClient::render(QString projName, long int frame) {
	projectName = projName;
	currentFrame = frame;
	if (theSocket->state() == QSocket::Connected) {
			QTextStream os(theSocket);
		os << "render job=\"" << projName << "\" frame=" << frame << "\n";
		internalString = "Render job "+projName;
		emit changed( "Rendering started", this );
	}
	else
		emit echo("Trying to render...", this );
}

void wcsClient::halt() {
	if (theSocket->state() == QSocket::Connected) {
		QTextStream os(theSocket);
		os << escchar << "\n";
		outputFound = false;
		emit halted("Rendering halted", this);
	}
	else {
		emit halted("Halted.  Not connected to host.", this);
	}
}

void wcsClient::pause() {
	// just call halt... no pause function
	halt();
}

void wcsClient::unpause() {
   // just call render with the saved info
   render( projectName, currentFrame );
}

// If there is data waiting on the socket, do something about it
void wcsClient::readyRead()
{
	MessageType analysis;
	while (theSocket->canReadLine())
	{
		QString str = QString(theSocket->readLine());
		QString i;
		analysis=analyzeMessage(str,i);
		// analyzed, now switch on analysis in order to determine action
		switch(analysis) {
		case Result:
			// result of previous action was ok
			// FIXME: This could be from a Loaded message... what shall we do?
			if (!outputFound) {
				emit finished(internalString + " rendering finished", this);
			}
			else {
				outputFound = false;
				emit finished(internalString + " successful", this);
			}
		
			break;
		case Err:
			// result of previous action caused an error in WCS
			emit failed(internalString + " failed", this);
			break;
		case Loaded:
			load_ok = true;
			emit loaded(internalString + " successful", this);
			break;
		case Progress:
			// progress noted, time to send out
			emit progress(str, i.toInt(), this);
			break;
		case Output:
			// output file done, can parse/replace for "real" path in later
			// devel. iteration of program
			// also means that the output is finished
			outputFound = true;
			emit echo(str, this);
			break;
		default:
			// did something, but we don't care what.
			emit echo(str, this);
//			emit finished(str, this);
			break;
		}
	}
}

// Handle any other types of errors that can be on the socket
void wcsClient::error(int ErrNum) {
	switch (ErrNum) {
	case QSocket::ErrConnectionRefused:
		irreco = true;
		emit failed("Connection was refused", this);
		break;
	case QSocket::ErrHostNotFound:
		irreco = true;
		emit failed("Host not found", this);
		break;
	case QSocket::ErrSocketRead:
		emit failed("There was a read error on the socket", this);
		break;
	default:
		emit failed("Unknown (general) error on the socket", this);
	}
}

void wcsClient::connected() {
//	QString("Host ").append(peerAddress().toString().append(" found"));
	emit changed("Remote host found and connected", this);
}

void wcsClient::connectionClosed() {
	emit failed("Remote host has closed the connection", this);
}

// message parsing for control strings from WCS/VNS
wcsClient::MessageType wcsClient::analyzeMessage(QString &str, QString &i) {
	MessageType ret = Undefined;
	if (str.find("RESULT=\"1\"")!=-1 ) {
		if (load_message) {
			ret = Loaded;
			load_message = false;
		}
		else
			ret = Result;
	}
	else if (str.find("ERROR")!=-1)
		ret = Err;
	else if (str.find("project load")!=-1)
		load_message = true;
	else if (str.find("PROGRESS")!=-1) {
		i=str.mid(str.findRev(':')+1,str.findRev('%')-str.findRev(':')-1);
		str=str.mid(10,str.findRev(':')-str.find('\"')-1);
		ret = Progress;
	}
	else if (str.find("OUTPUT") != -1) {
		// then we have output to a WCS relative path
		str = str.section("\"", 1, 1);
		ret = Output;
	}
	return ret;
}

void wcsClient::goToNextChunk(QTextStream &infile, unsigned long size)
{
	char *temp;
	temp = new char[size];
	
	infile.readRawBytes(temp, size);              //skips specified number of bytes
	
	delete temp;
}

void wcsClient::getSize(char* tempChar, unsigned long &destination)
{
	char *temp = (char*)&destination;              //creates char* to casted destination long
	
	for(int i = 0; i < 4; i++)                     //all sizes are four bytes, sets four places
		temp[i] = tempChar[i];
}

void wcsClient::byteSwap(unsigned long &temp)
{
	unsigned long holder = temp;
	char *source = (char*)&holder;                 //creates char* to casted old long value
	char *destination = (char*)&temp;              //creates char* to casted new long value
	
	for(int i = 0; i < 4; i++)                     //all sizes are four bytes, swaps four places
		destination[i] = source[3 - i];
}
