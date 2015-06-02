// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      ErrorCodes.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file defines the various error codes returned by the
//					program at a critical error.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

enum ErrorCodes
{
	ERROR_INVALID_PARAMETERS = -1,
	ERROR_TOO_MANY_EDGES = -2,
	ERROR_CHOOSE_WAVELENGTH_1 = -3,
	ERROR_CHOOSE_WAVELENGTH_2 = -4,
	ERROR_INVALID_EDGE = -5,
	ERROR_THREAD_INIT = -6,
	ERROR_THREAD_EVENT_TYPE = -7,
	ERROR_EDGE_IS_USED = -8,
	ERROR_EDGE_IS_FREE = -9,
	ERROR_INVALID_CONFIRMATION = -10,
	ERROR_QUALITY_INPUT = -11,
	ERROR_TOPOLOGY_INPUT_ROUTERS = -12,
	ERROR_TOPOLOGY_INPUT_EDGES = -13,
	ERROR_WORKSTATION_INPUT_QUANTITY = -14,
	ERROR_WORKSTATION_INPUT_PARENT = -15,
	ERROR_ALGORITHM_INPUT = -16,
	ERROR_RECORD_EVENT = -17,
	ERROR_PROBES_RECEIVED = -18,
	ERROR_USER_CLOSED = -19,
	ERROR_QFACTOR_MONTIOR = -20,
	ERROR_WAVELENGTH_ALGORITHM_IA = -21,
	ERROR_PRIORITY_QUEUE = -22
};

#endif
