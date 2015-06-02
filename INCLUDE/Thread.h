// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Thread.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the thread class.
//					The purpose of the controller is to manage the simulation and
//					its associated events. This includes controlling the simulation
//					time, managing events, controlling the active/inactive work-
//					stations, and collecting/saving/printing the results of the
//					simulation.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//  06/02/2009	v1.02	Minor optimizations and bug fixes.
//
// ____________________________________________________________________________

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <vector>

using std::less;
using std::priority_queue;
using std::vector;

#include "AlgorithmParameters.h"
#include "EstablishedConnections.h"
#include "ErrorCodes.h"
#include "EventQueue.h"
#include "MessageLogger.h"
#include "QualityParameters.h"
#include "ResourceManager.h"
#include "Router.h"
#include "Stats.h"
#include "Workstation.h"

#include "boost/random.hpp"

using std::ifstream;
using std::cout;
using std::endl;
using std::ofstream;
using std::left;
using std::priority_queue;
using std::string;
using std::vector;

const int COMPLETED_ALL_SIMULATIONS = 2;
const int MORE_SIMULATIONS = 1;

class Thread
{
	public:
		Thread();
		Thread(int ci, int argc, const char* argv[], bool isLPS, int rc);
		~Thread();

		inline Router* getRouterAt(unsigned short int i)
			{ return routers[i]; };
		inline Workstation* getWorkstationAt(unsigned short int i)
			{ return workstations[i]; };

		inline void addRouter(Router* r)
			{ routers.push_back(r); };
		inline void addWorkstation(Workstation* w)
			{ workstations.push_back(w); };

		inline double getGlobalTime()
			{ return globalTime; };
		inline void setGlobalTime(double t)
			{ globalTime = t; };
		
		inline GlobalStats& getGlobalStats()
			{ return stats; };

		inline void setControllerIndex(unsigned short int ci)
			{ controllerIndex = ci; };

		void initThread(AlgorithmToRun* alg);

		int runThread(AlgorithmToRun* alg);

		void initPriorityQueue(unsigned short int w);
		void initResourceManager();

		inline QualityParameters getQualityParams()
			{ return qualityParams; };

		inline void recordEvent(const string &s, bool print, unsigned short int ci)
		{
			if(isLoadPrevious == true)
				return;
			else if(controllerIndex == 0)
				logger->recordEvent(s,print,ci);
			else
				exit(ERROR_RECORD_EVENT);
		};

		inline unsigned short int getNumberOfRouters()
			{ return numberOfRouters; };
		inline unsigned short int getNumberOfWorkstations()
			{ return numberOfWorkstations; };
		inline unsigned short int getNumberOfEdges()
			{ return numberOfEdges; };
		inline unsigned short int getNumberOfWavelengths()
			{ return numOfWavelengths; };
		inline void setNumberOfWavelengths(unsigned short int n)
			{	numOfWavelengths = n;	};
		inline unsigned short int getRandomSeed()
			{ return randomSeed; };

		inline ResourceManager* getResourceManager()
			{ return rm; };

		inline RoutingAlgorithm getCurrentRoutingAlgorithm()
			{ return CurrentRoutingAlgorithm; };
		inline WavelengthAlgorithm getCurrentWavelengthAlgorithm()
			{ return CurrentWavelengthAlgorithm; };
		inline ProbeStyle getCurrentProbeStyle()
			{ return CurrentProbeStyle; };
		inline bool getCurrentQualityAware()
			{ return CurrentQualityAware; };
		inline unsigned int getCurrentActiveWorkstations()
			{ return CurrentActiveWorkstations; };
		inline void setCurrentActiveWorkstations(unsigned int w)
			{ CurrentActiveWorkstations = w;	}

		float getBeta();

		inline double getMinDuration()
			{ return minDuration; };
		inline unsigned short int getMaxSpans()
			{ return maxSpans; };

		void setMinDuration(unsigned short int spans);
		void setQFactorMin(unsigned short int spans);

		inline const string* getRoutingAlgorithmName(unsigned short int a)
			{ return RoutingAlgorithmNames[a]; };
		inline const string* getWavelengthAlgorithmName(unsigned short int w)
			{ return WavelengthAlgorithmNames[w]; };
		inline const string* getProbeStyleName(unsigned short int p)
			{ return ProbeStyleNames[p]; };

#ifdef RUN_GUI
		void setTerminate()
			{ terminateProgram = true; };

		void saveThread(char* dir);

		inline void setMaxMaxUsage(float m)
			{ maxMaxUsage = m; };
		inline float getMaxMaxUsage()
			{ return maxMaxUsage; };

		inline void setPaintDir()
		{	if( paintDir )
			{	paintDir = false;	}
			else
			{	paintDir = true;	}
		};

		inline bool getPaintDir()
			{	return paintDir; };

		inline void setPaintSpans()//I don't think this should be here. Do something similar
		{	if( paintSpans )		//in GUI.cpp, instead
			{	paintSpans = false;	}
			else
			{	paintSpans = true;	}
		};
		inline bool getPaintSpans()
			{	return paintSpans;	};

		inline void setPaintRealTime()
		{
			if(paintRealTime)
			{	paintRealTime = false;	}
			else
			{	paintRealTime = true;	}
		};

		inline bool isPaintRealTime()
		{
			return paintRealTime;
		}

		inline void setNumUsagesToDisplay(int n)
			{	numUsagesToDisplay = n;	};
		inline int getNumUsagesToDisplay()
			{	return numUsagesToDisplay;	};
		inline void setUsageHistStartTime(int t)
			{	usageHistStartTime = t;	};
		inline int getUsageHistStartTime()
			{	return usageHistStartTime;	};
		
		bool terminateProgram;

		inline void setRouting(const char* rout)
		{	sprintf(routing,"%s",rout);	};
		inline void setTopology(const char* topo)
		{	sprintf(topology,"%s",topo);	};
		inline void setWavelength(const char* wave)
		{	sprintf(wavelength,"%s",wave);	};
		inline void setProbing(const char* prob)
		{	sprintf(probing,"%s",prob);	};
		inline void setQualityAware(int q)
		{	qualityAware = (bool)q;	};

		inline void setOverallBlocking(double b)
		{	overallBlocking = b;	};
		inline void setCollisions(double c)
		{	collisions = c;	};
		inline void setBadQuality(double b)
		{	badquality = b;	};
		inline void setNonResource(double n)
		{	nonresource = n;	};
		inline void setAvgProbesPerRequest(double a)
		{	avgprobesperrequest = a;	};
		inline void setAvgRequestDelayTime(double t)
		{	avgrequestdelaytime = t;	};
		inline void setAvgConnHopCount(double a)
		{	avgconnhopcount = a;	};
		inline void setAvgConnSpanCount(double a)
		{	avgconnspancount = a;	};
		inline void setAvgASEnoise(double a)
		{	avgASEnoise = a;	}
		inline void setAvgFWMnoise(double a)
		{	avgFWMnoise = a;	};
		inline void setAvgXPMnoise(double a)
		{	avgXPMnoise = a;	};
		
		void detailScreen();
#endif

		inline unsigned int getNumberOfConnections()
			{ return numberOfConnections; };

		//Random generator for zero to one
		boost::mt19937 rng4;
		boost::uniform_real<> *zo;
		boost::variate_generator<boost::mt19937&, boost::uniform_real<> > *generateZeroToOne;

		inline const char* getTopology()
			{ return topology; };

		inline MessageLogger* getLogger()
			{ return logger; };

	private:
		vector<Router*> routers;
		vector<Workstation*> workstations;

		void setTopologyParameters(const char* f);
		void setWorkstationParameters(const char* f);

		EventQueue* queue;

		double globalTime;

		MessageLogger* logger;

		ResourceManager* rm;

		const string* RoutingAlgorithmNames[NUMBER_OF_ROUTING_ALGORITHMS];
		const string* WavelengthAlgorithmNames[NUMBER_OF_WAVELENGTH_ALGORITHMS];
		const string* ProbeStyleNames[NUMBER_OF_PROBE_STYLES];

		void setAlgorithmParameters(const char* f, unsigned short int iterationCount);

		void activate_workstations();
		void deactivate_workstations();
		void generateTrafficEvent(unsigned int session);

		void update_link_usage();

#ifdef RUN_GUI
		void update_gui();
#endif
		void connection_request(ConnectionRequestEvent* cre);
		void create_connection_probe(CreateConnectionProbeEvent* ccpe);
		void create_connection_confirmation(CreateConnectionConfirmationEvent* ccce);
		void destroy_connection_probe(DestroyConnectionProbeEvent* dcpe);
		void collision_notification(CollisionNotificationEvent* cne);

		GlobalStats stats;

		bool isLoadPrevious;

		unsigned short int numOfWavelengths;

#ifdef RUN_GUI
		float maxMaxUsage;
		bool paintDir; //true if all edges must be repainted.
		bool paintSpans;
		bool paintRealTime;
		int progBarLengthpx; //length of full progress bar
		double ConnsPerPx;
		float multFactor;
		int paintedConns;
		int numUsagesToDisplay;
		int usageHistStartTime;
		char topoFile[100];
		char wkstFile[100];//TODO: do I ever use these?

		//SUMMARY variables to be saved to file
		char topology[20];//TODO: for this group of variables,
		char routing[20];//there may be other variables already 
		char wavelength[20];//fulfilled which serve the same purpose.
		char probing[20];//if possible, use those instead and
		bool qualityAware;//remember to get rid of all the set
		int totalActiveWKS;//methods used here
		int availableWaves;
		double overallBlocking;
		double collisions;
		double badquality;
		double nonresource;
		double avgprobesperrequest;
		double avgrequestdelaytime;
		double avgconnhopcount;
		double avgconnspancount;
		double avgASEnoise;
		double avgFWMnoise;
		double avgXPMnoise;

#endif

		unsigned short int controllerIndex;

		RoutingAlgorithm CurrentRoutingAlgorithm;
		WavelengthAlgorithm CurrentWavelengthAlgorithm;
		ProbeStyle CurrentProbeStyle;
		bool CurrentQualityAware;
		unsigned int CurrentActiveWorkstations;

		QualityParameters qualityParams;
		void setQualityParameters(const char* f);

		unsigned short int randomSeed;

		unsigned short int numberOfRouters;
		unsigned short int numberOfWorkstations;
		unsigned short int numberOfEdges;

		unsigned int numberOfConnections;

		bool order_init;
		unsigned short int* workstationOrder;

		double calculateDelay(unsigned short int spans);

		bool sendResponse(CreateConnectionProbeEvent* probe);
		void clearResponses(CreateConnectionProbeEvent* probe);
		bool moreProbes(CreateConnectionProbeEvent* probe);
		int otherResponse(CreateConnectionProbeEvent* probe);

		void updateQMDegredation(Edge **connectionPath, unsigned short int connectionLength, unsigned int wavelength);
		void updateQFactorStats(Edge **connectionPath, unsigned short int connectionLength, unsigned int wavelength);

		double minDuration;

		unsigned short int maxSpans;

		kShortestPathReturn* getKShortestPaths(ConnectionRequestEvent *cre, unsigned short int probesToSend);
		CreateConnectionProbeEvent** calcProbesToSend(ConnectionRequestEvent *cre, kShortestPathReturn *kPath, 
		unsigned short int &probesToSend, unsigned short int &probeStart, unsigned short int &probesSkipped);
		void sendProbes(ConnectionRequestEvent *cre, kShortestPathReturn *kPath, CreateConnectionProbeEvent** probesList,
		unsigned short int probesToSend, unsigned short int probeStart, unsigned short int probesSkipped);

		//Random generator for destination router
		boost::mt19937 rng;
		boost::uniform_int<> *rt;
		boost::variate_generator<boost::mt19937&, boost::uniform_int<> > *generateRandomRouter;

		//Random generator for duration time
		boost::mt19937 rng2;
		boost::exponential_distribution<> *dt;
		boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> > *generateRandomDuration;

		//Random generator for arrival interval
		boost::mt19937 rng3;
		boost::exponential_distribution<> *ai;
		boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> > *generateArrivalInterval;

		int getKthParameterInt(char *f);
		float getKthParameterFloat(char *f);

		int runCount;
		int maxRunCount;

		const char* topology;
};

#endif
