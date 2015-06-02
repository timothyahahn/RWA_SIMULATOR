// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      AllegroWrapper.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the allegro includes, but undefined
//					some data structures so that Boost can use them.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  06/02/2009	v1.02	Initial Verison.
//
// ____________________________________________________________________________

#ifndef ALLEGROWRAPPER_H_INCLUDED
#define ALLEGROWRAPPER_H_INCLUDED

#include <allegro.h>

#ifndef __GNUC__ 
#include <winalleg.h>
#endif

#undef int8_t
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef intptr_t
#undef uintptr_t
#undef intmax_t
#undef uintmax_t
#undef int64_t
#undef int_least64_t
#undef int_fast64_t
#undef uint64_t
#undef uint_least64_t
#undef uint_fast64_t

#endif
