// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      EventQueue.cpp
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the EventQueue.h, which
//					is intended to store the events in ascending order so that
//					they can be handled in the appropriate order.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#include "EventQueue.h"

///////////////////////////////////////////////////////////////////
//
// Function Name:	EventQueue
// Description:		Default constructor
//
///////////////////////////////////////////////////////////////////
EventQueue::EventQueue()
{

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	~EventQueue
// Description:		Default destructor
//
///////////////////////////////////////////////////////////////////
EventQueue::~EventQueue()
{

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getNextEvent()
// Description:		Removes the top element from the priority queue
//					and returns it.
//
///////////////////////////////////////////////////////////////////
Event EventQueue::getNextEvent()
{
	Event retVal = pq.top();

	pq.pop();

	return retVal;
}
