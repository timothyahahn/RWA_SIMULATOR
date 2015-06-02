// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Event.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of some data types
//					required by the EventQueue.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//  06/02/2009	v1.02	Minor optimizations and bug fixes.
//
// ____________________________________________________________________________

#ifndef EVENT_H
#define EVENT_H

#include "Edge.h"

#include <vector>

using std::vector;

#include "QYInclude.h"

enum EventType {
	ACTIVATE_WORKSTATIONS,
	DEACTIVATE_WORKSTATIONS,
	UPDATE_USAGE,
	UPDATE_GUI,
	CONNECTION_REQUEST,
	CREATE_CONNECTION_PROBE,
	CREATE_CONNECTION_CONFIRMATION,
	COLLISION_NOTIFICATION,
	DESTROY_CONNECTION_PROBE,
	NUMBER_OF_EVENTS
};

struct Event {
	EventType e_type;
	double e_time;
	void* e_data;
};

struct ConnectionRequestEvent
{
	unsigned short int sourceRouterIndex;
	unsigned short int destinationRouterIndex;
	short int wavelength;
	double connectionDuration;
	double requestBeginTime;
	unsigned int session;
	unsigned short int sequence;
	unsigned short int max_sequence;
	bool qualityFail;
};

struct CreateConnectionProbeEvent
{
	unsigned short int sourceRouterIndex;
	unsigned short int destinationRouterIndex;
	double connectionDuration;
	double requestBeginTime;
	double decisionTime;
	Edge **connectionPath;
	unsigned short int connectionLength;
	unsigned short int numberOfHops;
	short int wavelength;
	unsigned int session;
	unsigned short int sequence;
	unsigned short int max_sequence;
	kShortestPathReturn *kPaths;
	CreateConnectionProbeEvent **probes;
	bool atDestination;
	bool qualityFail;
};

struct CollisionNotificationEvent
{
	unsigned short int sourceRouterIndex;
	unsigned short int destinationRouterIndex;
	Edge **connectionPath;
	unsigned short int connectionLength;
	unsigned short int numberOfHops;
	unsigned int session;
	unsigned short int sequence;
	unsigned short int max_sequence;
	short int wavelength;
	CreateConnectionProbeEvent **probes;
	bool finalFailure;
};

struct CreateConnectionConfirmationEvent
{
	unsigned short int sourceRouterIndex;
	unsigned short int destinationRouterIndex;
	double connectionDuration;
	double requestBeginTime;
	Edge **connectionPath;
	unsigned short int connectionLength;
	unsigned short int numberOfHops;
	unsigned short int max_sequence;
	short int wavelength;
	short int originalWavelength;
	unsigned int session;
	unsigned short int sequence;
	kShortestPathReturn *kPaths;
	CreateConnectionProbeEvent **probes;
	bool finalFailure;
};

struct DestroyConnectionProbeEvent
{
	Edge **connectionPath;
	unsigned short int connectionLength;
	unsigned short int numberOfHops;
	unsigned int session;
	unsigned short int sequence;
	short int wavelength;
	CreateConnectionProbeEvent **probes;
};

#endif
