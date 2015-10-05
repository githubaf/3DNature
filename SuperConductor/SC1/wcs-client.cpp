// wcs-client.cpp

#include <iostream.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include "wcs-client.h"
#include "clientDataTransfer.h"

#include <qmessagebox.h>

wcsClient::wcsClient() : QSocket(0,0) {
	// this is the default constructor.  Leaves the object "broken"
}

wcsClient::wcsClient(QString hostName, int sockNum, QObject* parent)
: QSocket(parent, 0) {
	connect(this, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
	connect(this, SIGNAL(connected()), this, SLOT(connected()));
	connect(this, SIGNAL(error(int)), this, SLOT(error(int)));
	
	// assume that updated is connected
	
	host = hostName;
	sock = sockNum;
	statusBuffer = "";
	outputFound = false;
}

wcsClient::~wcsClient() {
}

QString wcsClient::status() {
	return statusBuffer;
}

// If there is data waiting on the socket, do something about it
void wcsClient::socketReadyRead()
{
	MessageType analysis;
	while (this->canReadLine())
	{
		QString str = QString(this->readLine());
		QString i;
		analysis=analyzeMessage(str,i);
		// analyzed, now switch on analysis in order to determine action
		switch(analysis) {
		case Result:
			// result of previous action was ok
			if (!outputFound) {
				statusBuffer = "RESULT?TRUE";
			}
			else {
				outputFound = false;
				statusBuffer = "OUTPUT?" + str;
			}
			emit updated();
			break;
		case Err:
			// result of previous action caused an error in WCS
			statusBuffer = "ERROR?FATAL";
			emit updated();
			break;
		case Progress:
			// progress noted, time to send out
			statusBuffer = "PROGRESS?" + str + "?" + i;
			emit updated();
			break;
		case Output:
			// output file done, can parse/replace for "real" path in later
			// devel. iteration of program
			// also means that the output is finished
			statusBuffer = "ECHO?" + str;
			outputFound = true;
			emit updated();
			break;
		default:
			// did something, but who knows what.
			statusBuffer = "ECHO?" + str;
			emit updated();
			break;
		}
	}
}

// WCS specific message parsing, to allow for update of the display and
// administration of the connection
wcsClient::MessageType wcsClient::analyzeMessage(QString &str, QString &i) {
	MessageType ret = Undefined;
	if (str.find("RESULT=\"1\"")!=-1 ) ret = Result;
	else if (str.find("ERROR")!=-1) ret = Err;
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

void wcsClient::render(QString projName, int frame) {
    QTextStream os(this);
	os << "render job=\"" << projName << "\" frame=" << frame << "\n";
}

void wcsClient::load(QString fileName, QString relativePath) {
    QTextStream os(this);
	if (relativePath != "")
		os << "project load " << relativePath << "\n";
	else
		os << "project load " << fileName << "\n";
}

void wcsClient::halt() {
	QTextStream os(this);
	os << escchar << "\n";
	outputFound = false;
}

void wcsClient::connectTo(QString host, int socket) {
	// using locally-passed args 'host' and 'socket' not object-scope vars 'host' and 'sock' -CXH
	QSocket::connectToHost(host, socket);
}

// nothing uses this method yet, but it seemed orthogonally necessary -CXH
void wcsClient::connectToStored(void) {
	// using object-scope vars 'host' and 'sock' -CXH
	QSocket::connectToHost(host, sock);
}

void wcsClient::connectionClosed() {
	statusBuffer = "ERROR?FATAL?";
	statusBuffer.append(peerAddress().toString().append(" has closed the connection"));
	emit updated();
}

void wcsClient::connected() {
	statusBuffer = "CONNECTED?";
	statusBuffer.append(QString("Host ").append(peerAddress().toString().append(" found")));
	emit updated();
}

// Handle any other types of errors that can be on the socket
void wcsClient::error(int ErrNum) {
	switch (ErrNum) {
	case QSocket::ErrConnectionRefused:
		statusBuffer = "ERROR?FATAL?";
		statusBuffer.append("Connection was refused");
		emit updated();
		break;
	case QSocket::ErrHostNotFound:
		statusBuffer = "ERROR?FATAL?";
		statusBuffer.append("Host not found");
		emit updated();
		break;
	case QSocket::ErrSocketRead:
		statusBuffer = "ERROR?FATAL?";
		statusBuffer.append("There was a read error on the socket");
		emit updated();
		break;
	default:
		statusBuffer = "ERROR?FATAL?";
		statusBuffer.append("Unknown (general) error on the socket");
		emit updated();
	}
}

// code inserted from wcsparser.h on July 7 2002 by CXH

// bool wcsClient::parse(const QString file, int& startFrame, int& endFrame, int& frameInterval, QString &job)
collectedJobs wcsClient::parse(QString file)
{
	collectedJobs jobs;
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
			collectedJobs t;
			return t;
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
			collectedJobs t;
			return t;
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
			collectedJobs t;
			return t;
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
				collectedJobs t;
			    return t;
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
				jobs.addJob(renderInfo(JobPriTemp, FrameIntervalTemp, StartFrameTemp, EndFrameTemp, JobNameTemp));
			} // if
		} // for
	} // if infile close
	else {
		infile.close();
		collectedJobs t;
		return t;
	}
	infile.close();
	return jobs;                               //returns 0 if no problems with parsing
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
