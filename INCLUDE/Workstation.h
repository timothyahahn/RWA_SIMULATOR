// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Workstation.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the Workstation class.
//					The purpose of the Workstations are to generate traffic for
//					the optical network.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifndef WORKSTATION_H
#define WORKSTATION_H

class Workstation
{
	public:
		Workstation();
		~Workstation();

		inline unsigned short int getParentRouterIndex()
			{ return parentRouterIndex; };
		inline void setParentRouterIndex(unsigned short int p)
			{ parentRouterIndex = p; };

		inline void setActive(bool a)
			{ active = a; };
		inline bool getActive()
			{ return active; };

	private:
		unsigned short int parentRouterIndex;

		bool active;
};

#endif
