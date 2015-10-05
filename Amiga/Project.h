// Project.h
// Project Object
// Written from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_PROJECT_H
#define WCS_PROJECT_H

class Project; // Heh heh heh... You get a B- for your Class Project in C++ pun

class Project
	{
	public:
//	Project();
//	~Project();
//	Load();
//	Save();
	void InquireWindowCoords(unsigned long int WinID, unsigned short int &X,
	 unsigned short int &Y, unsigned short int &W, unsigned short int &H);
	}; // Project

#endif // WCS_PROJECT_H
