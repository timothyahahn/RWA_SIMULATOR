// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      EventQueue.h
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

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <queue>
#include <vector>

#include "ErrorCodes.h"
#include "Event.h"

using std::less;
using std::priority_queue;
using std::vector;

class EventQueue
{
	public:
		EventQueue();
		~EventQueue();
	
		Event getNextEvent();

		inline void addEvent(const Event &e)
			{ pq.push(e); };

		inline unsigned int getSize()
			{ return static_cast<unsigned int>(pq.size()); };

	private:
		priority_queue<Event, vector<Event>,less<vector<Event>::value_type> > pq;
};

static bool operator< (const Event& event1, const Event &event2)
{
	if(event1.e_time != event2.e_time)
	{
		return event1.e_time > event2.e_time;
	}
	else if(event1.e_type == event2.e_type)
	{
		if(event1.e_type ==	CONNECTION_REQUEST)
		{
			ConnectionRequestEvent *cre1 = static_cast<ConnectionRequestEvent*>(event1.e_data);
			ConnectionRequestEvent *cre2 = static_cast<ConnectionRequestEvent*>(event2.e_data);

			return cre1->session > cre2->session;
		}
		else if(event1.e_type == CREATE_CONNECTION_PROBE)
		{
			CreateConnectionProbeEvent *ccp1 = static_cast<CreateConnectionProbeEvent*>(event1.e_data);
			CreateConnectionProbeEvent *ccp2 = static_cast<CreateConnectionProbeEvent*>(event2.e_data);

			if(ccp1->session != ccp2->session)
				return ccp1->session > ccp1->session;
			else
				return ccp1->sequence > ccp2->sequence;
		}
		else if(event1.e_type == CREATE_CONNECTION_CONFIRMATION)
		{
			CreateConnectionConfirmationEvent *ccc1 = static_cast<CreateConnectionConfirmationEvent*>(event1.e_data);
			CreateConnectionConfirmationEvent *ccc2 = static_cast<CreateConnectionConfirmationEvent*>(event2.e_data);

			return ccc1->session > ccc2->session;
		}
		else if(event1.e_type == COLLISION_NOTIFICATION)
		{
			CollisionNotificationEvent *cn1 = static_cast<CollisionNotificationEvent*>(event1.e_data);
			CollisionNotificationEvent *cn2 = static_cast<CollisionNotificationEvent*>(event2.e_data);

			return cn1->session > cn2->session;
		}
		else if(event1.e_type == DESTROY_CONNECTION_PROBE)
		{
			DestroyConnectionProbeEvent *dcp1 = static_cast<DestroyConnectionProbeEvent*>(event1.e_data);
			DestroyConnectionProbeEvent *dcp2 = static_cast<DestroyConnectionProbeEvent*>(event2.e_data);

			return dcp1->session > dcp2->session;
		}
		else
		{
			exit(ERROR_PRIORITY_QUEUE);
		}
	}
	else
	{
		return event1.e_type > event2.e_type;
	}
}

#endif
