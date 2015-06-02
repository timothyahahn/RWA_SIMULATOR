// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Edge.h
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the Edge class.
//					The purpose of the Edge class is to connect the routers
//					together.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//  06/02/2009	v1.02	Minor optimizations and bug fixes.
//
// ____________________________________________________________________________

#ifndef EDGE_H
#define EDGE_H

#include <list>
#include <vector>

#include "Stats.h"

#ifdef RUN_GUI
#include "AllegroWrapper.h"
#endif

using std::list;
using std::vector;

enum EdgeStatus {
	EDGE_FREE,
	EDGE_USED
};

class Edge
{
	public:
		Edge();
		Edge(unsigned short int src, unsigned short int dest, unsigned short int spans);

		~Edge();

		inline unsigned short int getSourceIndex() 
			{ return sourceIndex; };
		inline unsigned short int getDestinationIndex() 
			{ return destinationIndex; };
		inline void setSourceIndex(short int s)
			{	sourceIndex = s;	};
		inline void setDestinationIndex(short int d)
			{	destinationIndex = d;	};

		inline unsigned short int getNumberOfSpans() 
			{ return numberOfSpans; };
		inline void setNumberOfSpans(int s) 
			{ numberOfSpans = s; };

		inline EdgeStatus getStatus(unsigned short int w)
			{ return status[w]; };
		inline int getActiveSession(unsigned short int w)
			{ return activeSession[w]; };

		inline void setUsed(int session, unsigned short int w)
			{ status[w] = EDGE_USED; activeSession[w] = session; };
		inline void setFree(unsigned short int w)
			{ status[w] = EDGE_FREE; activeSession[w] = -1; degredation[w] = 0.0; };

		void updateUsage();
		
		void updateQMDegredation(unsigned short int ci, unsigned int wavelength);
		void updateQFactorStats(unsigned short int ci, unsigned int wavelength);

		list <void*> establishedConnections;

#ifdef RUN_GUI
		void updateGUI();
		void refreshbmps(bool useThread);
		void initializetopobmps();
#endif

		inline float getAlgorithmUsage()
			{ return algorithmUsage; };
		inline void resetAlgorithmUsage()
			{ algorithmUsage = 0.0; };
		void resetQMDegredation();

		inline float getQMDegredation()
			{ return QMDegredation; };

		inline float getPheremone()
			{ return pheremone; };
		 void resetPheremone(unsigned int ci, unsigned int spans);

		void evaporatePheremone(unsigned int ci);
		void addPheremone(unsigned int hops, unsigned int ci);

#ifdef RUN_GUI
		inline unsigned short int getMaxActualUsage()
			{ return max_actual_usage; };
		inline int getUsageNums()
			{ return static_cast<int>(usageList.size()); };
		inline void addUsage(unsigned short int u)
			{ usageList.push_back(u); }
		inline void setMaxUsage(unsigned short int u)
			{ max_actual_usage = u; }
		void scaleEdgesTo(int spns, int px);
		void paintSpans();
		void paintUsage(int p);
		void paintUsageHistory(int x1,int y1, int x2,int y2,int t);
		void calculateAverageUsage();
		void saveData(char* file);
#endif	
		inline EdgeStats* getEdgeStats()
			{ return stats; };

		void resetEdgeStats();

		inline void insertEstablishedConnection(void* ec_void)
			{ establishedConnections.push_back(ec_void); };
		void removeEstablishedConnection(void* dcpe_void);

private:
		unsigned short int sourceIndex;
		unsigned short int destinationIndex;
		unsigned short int numberOfSpans;

		int *activeSession;
		EdgeStatus *status;

		float algorithmUsage;
		unsigned short int actualUsage;

		float QMDegredation;

		double* degredation;

#ifdef RUN_GUI
		int r1x,r1y,r2x,r2y; //coordinates of routers
		int r3x,r3y,r4x,r4y; //points to draw to for edge width (these change)
		int putX,putY;
		unsigned short int thdIndx;
		float invunitX, invunitY, currX, currY, oldX, oldY, average_usage, painted_usage;//inverse unit vectors
		int painted_percent; 
		unsigned short int max_actual_usage;
		vector<unsigned short int> usageList;
		BITMAP* edgeBmps[14];
		BITMAP* edgeBmp;
#endif
		EdgeStats *stats;

		float pheremone;
};

#endif
