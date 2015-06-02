// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      EstablishedConnection.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains datatypes that are important for determining
//					the performance of the various algorithms.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  06/02/2009	v1.02	Initial Version.
//
// ____________________________________________________________________________

#ifndef ESTABLISHED_H
#define ESTABLISHED_H

#include "Edge.h"

struct EstablishedConnection
{
	Edge **connectionPath;
	unsigned short int connectionLength;
	short int wavelength;
	double connectionStartTime;
	double connectionEndTime;
	float initQFactor;
	float belowQFactor;
	float averageQFactor;
	vector<double> *QFactors;
	vector<double> *QTimes;
};

#endif
