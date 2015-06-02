// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      MessageLogger.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the MessageLogger class,
//					used to record events to a file.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifndef MESSAGE_LOGGER_H
#define MESSAGE_LOGGER_H

#include "pthread.h"

#include "RaptorTime.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using std::cout;
using std::endl;
using std::ofstream;
using std::string;

class MessageLogger
{
	public:
		MessageLogger(const char*, const char*, const char*, const char*,int runCount);
		~MessageLogger();

		void recordEvent(const string &e, bool print, unsigned short int ci);

		inline void LockResultsMutex()
			{ pthread_mutex_lock(&ResultsMutex); };

		inline void UnlockResultsMutex()
			{ pthread_mutex_unlock(&ResultsMutex); };

	private:
		ofstream eventLogger;

		FormattedTime convertToFormattedTime(double t);

		string convertToStringTime(const FormattedTime &t);

		pthread_mutex_t LogMutex;
		pthread_mutex_t PrintMutex;
		pthread_mutex_t ResultsMutex;
};

#endif
