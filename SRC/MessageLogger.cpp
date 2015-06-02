// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      MessageLogger.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the implementation of the MessageLogger class,
//					used to record events to a file.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#include "MessageLogger.h"
#include "Thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern Thread* threadZero;
extern Thread** threads;

char* itoa( int value, char* result, int base );

///////////////////////////////////////////////////////////////////
//
// Function Name:	MessageLogger
// Description:		Default constructor
//
///////////////////////////////////////////////////////////////////
MessageLogger::MessageLogger(const char* topo, const char* lambda, const char* seed, const char* k,int runCount)
{
	pthread_mutex_init(&LogMutex,NULL);
	pthread_mutex_init(&PrintMutex,NULL);
	pthread_mutex_init(&ResultsMutex,NULL);

	char buffer[100];
	sprintf(buffer,"OUTPUT/EventLog-%s-%s-%s-%s-R%d.txt",topo,lambda,seed,k,runCount);

	eventLogger.open(buffer);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	~MessageLogger
// Description:		Default destructor
//
///////////////////////////////////////////////////////////////////
MessageLogger::~MessageLogger()
{
	pthread_mutex_destroy(&LogMutex);
	pthread_mutex_destroy(&PrintMutex);
	pthread_mutex_destroy(&ResultsMutex);

	eventLogger.close();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	recordEvent
// Description:		Writes the event to the log file with a timestamp.
//					Also prints the file to the console if the print
//					value is set to true (that is the default)
//
///////////////////////////////////////////////////////////////////
void MessageLogger::recordEvent(const string &e, bool print, unsigned short int ci)
{
	if(threadZero->getQualityParams().detailed_log == true || print == true)
	{
		pthread_mutex_lock(&LogMutex);

		struct tm *current;
		time_t now;
	
		time(&now);
		current = localtime(&now);

		eventLogger << ci << ":";
		
		if(current->tm_hour < 10)
			eventLogger << "0" << current->tm_hour << "::";
		else
			eventLogger << current->tm_hour << "::";

		if(current->tm_min < 10)
			eventLogger << "0" << current->tm_min << "::";
		else
			eventLogger << current->tm_min << "::";

		if(current->tm_sec < 10)
			eventLogger << "0" << current->tm_sec;
		else
			eventLogger << current->tm_sec;

		eventLogger << " " << e << endl;

		pthread_mutex_unlock(&LogMutex);
	}

	if(print == true)
	{
		pthread_mutex_lock(&PrintMutex);

		struct tm *current;
		time_t now;
	
		time(&now);
		current = localtime(&now);

		cout << ci << ":";
		
		if(current->tm_hour < 10)
			cout << "0" << current->tm_hour << "::";
		else
			cout << current->tm_hour << "::";

		if(current->tm_min < 10)
			cout << "0" << current->tm_min << "::";
		else
			cout << current->tm_min << "::";

		if(current->tm_sec < 10)
			cout << "0" << current->tm_sec;
		else
			cout << current->tm_sec;

		cout << " " << e << endl;

		pthread_mutex_unlock(&PrintMutex);
	}

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	convertToFormattedTime
// Description:		Takes the time in seconds and converts it to
//					the structured time format.
//
///////////////////////////////////////////////////////////////////
FormattedTime MessageLogger::convertToFormattedTime(double t)
{
	FormattedTime retval;

	//Get the number of minutes
	retval.minutes = static_cast<unsigned short>(t / 60.0);
	retval.seconds = float(t - retval.minutes * 60.0);

	//Get the number of hours
	retval.hours = static_cast<unsigned short>(static_cast<float>(retval.minutes) / 60.0);
	retval.minutes = retval.minutes - retval.hours * 60;

	return retval;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	convertToStringTime
// Description:		Takes the time in the structured format and
//					converts it to a string
//
///////////////////////////////////////////////////////////////////
string MessageLogger::convertToStringTime(const FormattedTime &t)
{
	string retval;
	char buffer[10];

	//Print the hours
	if(t.hours >= 10)
	{
		itoa(t.hours,buffer,10);
		retval.append(buffer);
	}
	else
	{
		itoa(t.hours,buffer,10);
		retval.append("0");
		retval.append(buffer);
	}

	retval.append(":");

	//Print the minutes
	if(t.minutes >= 10)
	{
		itoa(t.minutes,buffer,10);
		retval.append(buffer);
	}
	else
	{
		itoa(t.minutes,buffer,10);
		retval.append("0");
		retval.append(buffer);
	}

	retval.append(":");

	//Print the seconds
	if(t.seconds >= 10)
	{
		sprintf(buffer,"%2.3f",t.seconds);
		retval.append(buffer);
	}
	else
	{
		sprintf(buffer,"%2.3f",t.seconds);
		retval.append("0");
		retval.append(buffer);
	}
	return retval;
}

//Source: http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
char* itoa( int value, char* result, int base )
{	
	// check that the base if valid	
	if (base < 2 || base > 16)
	{ 
		*result = 0; 
		return result; 
	}
	
	char* out = result;
	int quotient = value;
	
	do {
	
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
	
		++out;
	
		quotient /= base;
	
	} while ( quotient );
	
	// Only apply negative sign for base 10
	if ( value < 0 && base == 10)
		*out++ = '-';
	
	std::reverse( result, out );
	
	*out = 0;
	
	return result;
}
