// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Stats.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains datatypes that are important for determining
//					the performance of the various algorithms.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//  06/02/2009	v1.02	Minor optimizations and bug fixes.
//
// ____________________________________________________________________________

#ifndef STATS_H
#define STATS_H

#include "Edge.h"

struct GlobalStats
{
	unsigned int ConnectionRequests;
	unsigned int ConnectionSuccesses;
	unsigned int CollisionFailures;
	unsigned int QualityFailures;
	unsigned int NoPathFailures;
	unsigned int DroppedFailures;
	unsigned int ProbeSentCount;
	unsigned int totalHopCount;
	unsigned int totalSpanCount;
	double aseNoiseTotal;
	double xpmNoiseTotal;
	double fwmNoiseTotal;
	double totalSetupDelay;
	double raRunTime;
};

struct EdgeStats
{
	unsigned int droppedConnections;
	double minInitalQFactor;
	double minAverageQFactor;
	double minPercentQFactor;
	double maxInitalQFactor;
	double maxAverageQFactor;
	double maxPercentQFactor;
	double totalInitalQFactor;
	double totalAverageQFactor;
	double totalPercentQFactor;
	double totalTime;
	double count;
};

enum FailureTypes
{
	COLLISION_FAILURE = -1,
	QUALITY_FAILURE = -2,
	NO_PATH_FAILURE = -3
};

#endif
