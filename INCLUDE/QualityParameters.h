// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      QualityParameters.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the QualityParameters
//					struct, which are used to calculate the Q factor
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifndef QUALITY_PARAMS_H
#define QUALITY_PARAMS_H

enum DestinationDistribution
{
	UNIFORM = 1,
	DISTANCE = 2,
	INVERSE_DISTANCE = 3
};

struct QualityParameters
{
	float arrival_interval;		//the inter arrival time on each workstation
	float duration;				//the duration time of one connection
	int nonlinear_halfwin;		//the total nonlinear_win is 2*M+1
	int halfwavelength;			//the total wavelength is 2N+1
	float fc;					//center of the wavelength comb
	float f_step;				//step of the wavelength comb
	float channel_power;		//power per channel
	float L;					//length of the NZDSF in each span
	float alphaDB;				//attenuation of NZDSF in each span
	float alpha;				//attenuation of NZDSF in each span
	float D;					//dispersion of NZDSF in each span
	float S;					//disperison slop of NZDSF in each span
	float gamma;				//nonlinear coefficent of NZDSF in each span
	float QFactor_factor;		//Factor of max min to set the threshold
	float TH_Q;					//nonlinear coefficent of NZDSF in each span
	float EDFA_Noise_Figure;	//EDFA Noise Figure
	float EDFA_Gain;			//EDFA Gain
	float B_w;					//Optical bandwidth of the signal channel
	float *ASE_perEDFA;			//ASE noise per EDFA
	float usage_update_interval;	//interval for updating usage for PABR and LORA
	float beta;					//beta value for PABR and LORA
	int gui_update_interval;	//interval for updating the gui
	unsigned short int max_probes;	//max number of probes per connection request
	float refractive_index;		//refractive index of the optical links
	bool q_factor_stats;		//should the program calculate the Q-factor stats (1=yes,0=no)
	bool detailed_log;			//should the program keep a detailed log (1=yes,0=no)
	DestinationDistribution dest_dist;	//distribution of the destination
	float DP_alpha;				//Alpha value for Dynamic Programming
	int ACO_ants;				//number of ants in each ACO iteration
	float ACO_alpha;			//the pheromone power index for ACO
	float ACO_beta;				//the heuristic information power index for ACO
	float ACO_rho;				//the pheromone evaporation rate for ACO
	float MM_ACO_gamma;			//the min-max pheromone ratio for MM ACO
	int MM_ACO_N_iter;			//the number of iterations for stagnation for MM ACO
	int MM_ACO_N_reset;			//the number of reinitialization times for MM ACO
};

#endif
