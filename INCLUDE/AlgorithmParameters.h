// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      AlgorithmParameters.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the AlgoirthmParameters
//					struct, which are used to calculate the Q factor
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifndef ALGORITHM_PARAM_H
#define ALGORITHM_PARAM_H

enum RoutingAlgorithm {
	SHORTEST_PATH,
	PABR,
	LORA,
	IMPAIRMENT_AWARE,
	Q_MEASUREMENT,
	ADAPTIVE_QoS,
	DYNAMIC_PROGRAMMING,
	ACO,
	MAX_MIN_ACO,
	NUMBER_OF_ROUTING_ALGORITHMS
};

enum WavelengthAlgorithm {
	FIRST_FIT,
	FIRST_FIT_ORDERED,
	BEST_FIT,
	RANDOM_FIT,
	QUAL_FIRST_FIT,
	QUAL_FIRST_FIT_ORDERED,
	QUAL_RANDOM_FIT,
	LEAST_QUALITY,
	MOST_QUALITY,
	MOST_USED,
	QUAL_MOST_USED,
	NUMBER_OF_WAVELENGTH_ALGORITHMS
};

enum ProbeStyle {
	SINGLE,
	SERIAL,
	PARALLEL,
	NUMBER_OF_PROBE_STYLES
};

struct AlgorithmToRun
{
	RoutingAlgorithm ra;
	WavelengthAlgorithm wa;
	ProbeStyle ps;
	bool qa;
	unsigned int workstations;
};

#endif
