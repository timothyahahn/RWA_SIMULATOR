// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Edge.cpp
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

#include "Edge.h"
#include "Thread.h"

#ifdef RUN_GUI

#include <stdlib.h>

extern BITMAP *graph;
extern BITMAP *graphbackground;
extern BITMAP *popup;
extern BITMAP *buffer;
extern BITMAP *edgesbmp;
extern BITMAP *edgespans;
extern BITMAP *mainbuf;

extern BITMAP *edgeOriginals[14];
extern BITMAP *arrowOriginals[14];
extern BITMAP *tailOriginals[14];

extern BITMAP *routersbmp;//take this out later.

#define PI 3.14159265

#endif

#include "EstablishedConnections.h"

extern Thread* threadZero;
extern Thread** threads;
extern vector<Router*> topoRouters;

///////////////////////////////////////////////////////////////////
//
// Function Name:	Edge
// Description:		Default constructor with no arguements.
//
///////////////////////////////////////////////////////////////////
Edge::Edge()
{

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	Edge
// Description:		Constructor with arguements to set the source,
//					destination, and number of spans.
//
///////////////////////////////////////////////////////////////////
Edge::Edge(unsigned short int src, unsigned short int dest, unsigned short int spans)
{
	sourceIndex = src;
	destinationIndex = dest;
	numberOfSpans = spans;

	status = new EdgeStatus[threadZero->getNumberOfWavelengths()];
	activeSession = new int[threadZero->getNumberOfWavelengths()];

	degredation = new double[threadZero->getNumberOfWavelengths()];

	resetQMDegredation();

	stats = new EdgeStats;

	resetEdgeStats();

	for(unsigned short int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
	{
		status[w] = EDGE_FREE;
		activeSession[w] = -1;
	}

	algorithmUsage = 0.0;
	actualUsage = 0;
	QMDegredation = 0.0;

#ifdef RUN_GUI
	max_actual_usage = 0;

	for(int x = 0; x < 14; x++)
		edgeBmps[x] = 0;

	refreshbmps(true);
#endif

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	Edge
// Description:		Default destructor with no arguements.
//
///////////////////////////////////////////////////////////////////
Edge::~Edge()
{
	delete[] status;
	delete[] activeSession;

	delete[] degredation;

	delete[] stats;

#ifdef RUN_GUI
	usageList.clear();

	for(int i = 0; i < 14; i++)
	{
		destroy_bitmap(edgeBmps[i]);
	}
#endif
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	evaporatePheremone
// Description:		Evaporates the pheremone
//
///////////////////////////////////////////////////////////////////
void Edge::evaporatePheremone(unsigned int ci)
{
	pheremone = (1.0 - threadZero->getQualityParams().ACO_rho) * pheremone;

	if(threads[ci]->getCurrentRoutingAlgorithm() == MAX_MIN_ACO)
	{
		if(pheremone < threadZero->getQualityParams().MM_ACO_gamma)
		{
			pheremone = threadZero->getQualityParams().MM_ACO_gamma;
		}
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetPheremone
// Description:		Resets the pheremone levels to the initial value
//
///////////////////////////////////////////////////////////////////
void Edge::resetPheremone(unsigned int ci, unsigned int spans)
{
	if(threads[ci]->getCurrentRoutingAlgorithm() == MAX_MIN_ACO)
	{
		pheremone = 1.0;
	}
	else if(threads[ci]->getCurrentRoutingAlgorithm() == ACO)
	{
		pheremone = double(threadZero->getQualityParams().ACO_ants) /
			double(spans);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	addPheremone
// Description:		Adds the pheremone level based upon the solution quality.
//
///////////////////////////////////////////////////////////////////
void Edge::addPheremone(unsigned int hops, unsigned int ci)
{
	pheremone = pheremone + 1.0 / float(hops);

	if(threads[ci]->getCurrentRoutingAlgorithm() == MAX_MIN_ACO)
	{
		if(pheremone > 1.0)
		{
			pheremone = 1.0;
		}
	}
}

#ifdef RUN_GUI
///////////////////////////////////////////////////////////////////
//
// Function Name:	refreshtopobmps
// Description:		Repaints the edges with new router coordinates.
//
///////////////////////////////////////////////////////////////////
void Edge::initializetopobmps()
{
	max_actual_usage = 0;

	for(int x = 0; x < 14; x++)
		edgeBmps[x] = 0;

	refreshbmps(false);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	refreshbmps
// Description:		Repaints the edges with new router coordinates.
//
///////////////////////////////////////////////////////////////////
void Edge::refreshbmps(bool useThread)
{
	float dist, doubR1X, doubR1Y, doubR2X, doubR2Y;
	painted_usage = 0.0;
	painted_percent = -1;

	currX = 0;
	currY = 0;
	if(useThread)
	{
		r1x = threadZero->getRouterAt(sourceIndex)->getXPixel(); //set coordinates of routers
		r1y = threadZero->getRouterAt(sourceIndex)->getYPixel();
		r2x = threadZero->getRouterAt(destinationIndex)->getXPixel();
		r2y = threadZero->getRouterAt(destinationIndex)->getYPixel();
	}
	else
	{
		r1x = topoRouters[sourceIndex]->getXPixel(); //set coordinates of routers
		r1y = topoRouters[sourceIndex]->getYPixel();
		r2x = topoRouters[destinationIndex]->getXPixel();
		r2y = topoRouters[destinationIndex]->getYPixel();
	}

	if(!useThread && r3x == r1x && r3y == r1y && r4x == r2x && r4y == r2y)
		return;
	else
	{
		r3x = r1x;
		r3y = r1y;
		r4x = r2x;
		r4y = r2y;

		if(r2x == r1x){ //straight up and down
			invunitX = 1.0;
			invunitY = 0.0;
			dist = abs(r2y - r1y);
		}
		else if(r2y == r1y){ //slope actually 0
			invunitX = 0.0;
			invunitY = 1.0;
			dist = abs(r2x - r1x);
		}
		else{
			doubR1X = r1x;
			doubR1Y = r1y;
			doubR2X = r2x;
			doubR2Y = r2y;
			dist = sqrt( ( doubR2X - doubR1X )*( doubR2X - doubR1X ) + (doubR2Y - doubR1Y)*(doubR2Y - doubR1Y) );//distance formula
			invunitX = -(r2y - r1y) / dist; //condense to inverse unit vector
			invunitY = (r2x - r1x) / dist ;			
		}

		float angle = atan((float)(r2y - r1y) / (float)(r2x - r1x)) * (256)/(2*PI);

		if( (r1x == r2x || r1y == r2y) && sourceIndex > destinationIndex)
		{
			r1x -= 5 * invunitX;
			r1y -= 5 * invunitY;
			r2x -= 5 * invunitX;
			r2y -= 5 * invunitY;
		}
		else{
			r1x += 5 * invunitX;
			r1y += 5 * invunitY;
			r2x += 5 * invunitX;
			r2y += 5 * invunitY;
		}

		if(r2x < r1x)
			angle += 128.0;
		
		//for bitmap edges

		int distInt = ceil(dist);
		BITMAP *temp_bmp = create_bitmap(distInt,13);
		BITMAP *temp_bmp2 = create_bitmap(distInt,distInt);

		if(r1x < r2x)
			putX = r1x;
		else
			putX = r2x;
		if(r1y < r2y)
			putY = r1y;
		else 
			putY = r2y;

		int copyX = ((distInt - abs(r2x - r1x)) / 2) - 7;
		int copyY = ((distInt - abs(r2y - r1y)) / 2) - 7;

		if(copyX < 0)
			copyX = 0;

		if(copyY < 0)
			copyY = 0;

		for(int b = 0; b < 14; b++)
		{
			clear_to_color(temp_bmp,makecol(255,0,255));
			stretch_blit(tailOriginals[b], temp_bmp, 0, 0, tailOriginals[b]->w, tailOriginals[b]->h, 15, 0, tailOriginals[b]->w, temp_bmp->h);
			stretch_blit(arrowOriginals[b], temp_bmp, 0, 0, arrowOriginals[b]->w, arrowOriginals[b]->h, distInt - 15 - arrowOriginals[b]->w, 0, arrowOriginals[b]->w, temp_bmp->h);
			stretch_blit(edgeOriginals[b], temp_bmp, 0, 0, edgeOriginals[b]->w, edgeOriginals[b]->h, 15+tailOriginals[b]->w, 0, distInt - 30 - arrowOriginals[b]->w - tailOriginals[b]->w, temp_bmp->h);
			clear_to_color(temp_bmp2,makecol(255,0,255));	
			rotate_sprite(temp_bmp2,temp_bmp,0,(temp_bmp2->h)/2,itofix(angle));
			destroy_bitmap(edgeBmps[b]);
			edgeBmps[b] = create_bitmap(abs(r2x - r1x)+14,abs(r2y - r1y)+14);
			clear_to_color(edgeBmps[b],makecol(255,0,255));
			blit(temp_bmp2, edgeBmps[b],copyX,copyY,0,0,edgeBmps[0]->w,edgeBmps[0]->h);
		}
		destroy_bitmap(temp_bmp);
		destroy_bitmap(temp_bmp2);
		//end for bitmap edges
	}
}

void Edge::scaleEdgesTo(int spns, int px)
{
	float dist,doubR1X, doubR1Y, doubR2X, doubR2Y;
	doubR1X = (float)topoRouters[sourceIndex]->getXPixel(); //set coordinates of routers
	doubR1Y = (float)topoRouters[sourceIndex]->getYPixel();
	doubR2X = (float)topoRouters[destinationIndex]->getXPixel();
	doubR2Y = (float)topoRouters[destinationIndex]->getYPixel();
	dist = sqrt( ( doubR2X - doubR1X )*( doubR2X - doubR1X ) + (doubR2Y - doubR1Y)*(doubR2Y - doubR1Y) );//distance formula
	unsigned short int newspans = (unsigned short int)ceil(dist * (float)spns / (float)px);
	numberOfSpans = newspans;
}
#endif
///////////////////////////////////////////////////////////////////
//
// Function Name:	updateUsage
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Edge::updateUsage()
{
	float newUsage = 0.0;

	if(threadZero->getQualityParams().q_factor_stats == true)
	{
		newUsage = float(establishedConnections.size());
	}
	else
	{
		for(unsigned short int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
		{
			if(status[w] == EDGE_USED)
				++newUsage;
		}
	}

	algorithmUsage = (algorithmUsage + newUsage) / float(2.0);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateQMDegredation
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Edge::updateQMDegredation(unsigned short int ci, unsigned int wavelength)
{	
	ResourceManager* rm = threadZero->getResourceManager();

#ifdef RUN_GUI
	thdIndx = ci; //TODO: put this in a location where it isn't repeated.
#endif

	double time = threads[ci]->getGlobalTime();

	for(list<void*>::iterator iter = establishedConnections.begin();
		iter != establishedConnections.end(); ++iter)
	{
		EstablishedConnection* ec = static_cast<EstablishedConnection*>(*iter);

		for(unsigned short int p = 0; p < ec->connectionLength; ++p)
		{
			if(ec->connectionPath[p] == this &&
				abs(ec->wavelength - int(wavelength)) <= threadZero->getQualityParams().nonlinear_halfwin)
			{
				double xpm = 0.0;
				double fwm = 0.0;
				double ase = 0.0;
			
				double source_noise = 0.0;
				double source_Q = 0.0;

				double dest_noise = 0.0;
				double dest_Q = 0.0;
			
				if(p == 0)
				{
					source_noise = threadZero->getQualityParams().ASE_perEDFA[threadZero->getQualityParams().halfwavelength];
				}
				else
				{
					source_Q = rm->estimate_Q(ec->wavelength,ec->connectionPath,p,&xpm,&fwm,&ase,ci);

					source_noise = pow(threadZero->getQualityParams().channel_power / pow(10.0,source_Q/10.0),2.0);
				}
			
				dest_Q = rm->estimate_Q(ec->wavelength,ec->connectionPath,p+1,&xpm,&fwm,&ase,ci);

				dest_noise = pow(threadZero->getQualityParams().channel_power / pow(10.0,dest_Q/10.0),2.0);
			
				degredation[ec->wavelength] = 10.0 * log10(sqrt(dest_noise) / sqrt(source_noise));


				if(threadZero->getQualityParams().q_factor_stats == true && 
					p == ec->connectionLength - 1)
				{
					if(ec->QFactors->size() == 0)
						ec->initQFactor = float(dest_Q);

					ec->QFactors->push_back(float(dest_Q));
					ec->QTimes->push_back(time);
				}

				break;
			}
		}
	}

	unsigned short int activeLightpaths = 0;
	double cumulativeDegradation = 0.0;

	for(unsigned short int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
	{
		if(degredation[w] != 0.0)
		{
			cumulativeDegradation += degredation[w];
			++activeLightpaths;
		}
	}

	if(activeLightpaths == threadZero->getNumberOfWavelengths())
		QMDegredation = std::numeric_limits<float>::infinity();
	else if(activeLightpaths == 0)
		QMDegredation = 2.0;
	else if(activeLightpaths > 0)
		QMDegredation = float(cumulativeDegradation) / float(activeLightpaths);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateQFactorStats
// Description:		Updates the Q Factor stats
//
///////////////////////////////////////////////////////////////////
void Edge::updateQFactorStats(unsigned short int ci, unsigned int wavelength)
{
	ResourceManager* rm = threadZero->getResourceManager();

#ifdef RUN_GUI
	thdIndx = ci; //TODO: put this in a location where it isn't repeated.
#endif

	double time = threads[ci]->getGlobalTime();

	for(list<void*>::iterator iter = establishedConnections.begin();
		iter != establishedConnections.end(); ++iter)
	{
		EstablishedConnection* ec = static_cast<EstablishedConnection*>(*iter);

		if(abs(ec->wavelength - int(wavelength)) <= threadZero->getQualityParams().nonlinear_halfwin)
		{
			if(ec->QTimes->size() == 0)
			{
				double xpm = 0.0;
				double fwm = 0.0;
				double ase = 0.0;

				ec->QFactors->push_back(float(rm->estimate_Q(
					ec->wavelength,ec->connectionPath,
					ec->connectionLength,&xpm,&fwm,&ase,ci)));

				ec->QTimes->push_back(time);

				ec->initQFactor = float(ec->QFactors->at(0));
			}
			else if(ec->QTimes->back() != time)
			{
				double xpm = 0.0;
				double fwm = 0.0;
				double ase = 0.0;

				ec->QFactors->push_back(float(rm->estimate_Q(
					ec->wavelength,ec->connectionPath,
					ec->connectionLength,&xpm,&fwm,&ase,ci)));

				ec->QTimes->push_back(time);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetQMDegredation
// Description:		Resets the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Edge::resetQMDegredation()
{
	for(unsigned int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
		degredation[w] = 0.0;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	removeEstablishedConnection
// Description:		Removes an established connection
//
///////////////////////////////////////////////////////////////////
void Edge::removeEstablishedConnection(void* dcpe_void)
{
	DestroyConnectionProbeEvent* dcpe = static_cast<DestroyConnectionProbeEvent*>(dcpe_void);
	EstablishedConnection* ec;

	for(list<void*>::iterator iter = establishedConnections.begin();
		iter != establishedConnections.end(); ++iter)
	{
		ec = static_cast<EstablishedConnection*>(*iter);

		if(ec->connectionPath == dcpe->connectionPath)
		{
			establishedConnections.erase(iter);
			break;
		}
	}

	degredation[dcpe->wavelength] = 0.0;

	if(this != dcpe->connectionPath[dcpe->connectionLength - 1])
		return;

	if(threadZero->getQualityParams().q_factor_stats == true)
	{
		if(ec->QFactors->size() !=	ec->QTimes->size())
		{
			threadZero->recordEvent("ERROR: Q Factor size is not equal to Q Times size.\n",true,0);
			exit(ERROR_QFACTOR_MONTIOR);
		}

		bool connectionDropped = false;

		double weight = 0.0;
		double timebelow = 0.0;
		
		unsigned int s = 0;

		for(s = 0; s < ec->QFactors->size() - 1; ++s)
		{
			weight += ec->QFactors->at(s) * 
				(ec->QTimes->at(s+1) - ec->QTimes->at(s));

			if(ec->QFactors->at(s) < threadZero->getQualityParams().TH_Q)
			{
				timebelow += ec->QTimes->at(s+1) - ec->QTimes->at(s);
				connectionDropped = true;
			}
		}

		if(connectionDropped == true)
		{
			++stats->droppedConnections;
		}

		weight += ec->QFactors->at(s) * 
			(ec->connectionEndTime - ec->QTimes->at(s));

		if(ec->QFactors->at(s) < threadZero->getQualityParams().TH_Q)
		{
			timebelow += ec->connectionEndTime - ec->QTimes->at(s);
		}

		ec->averageQFactor = float(weight / (ec->connectionEndTime - ec->connectionStartTime));

		ec->belowQFactor = float(timebelow / (ec->connectionEndTime - ec->connectionStartTime));

#ifdef RUN_GUI
		int src = dcpe->connectionPath[0]->getSourceIndex();
		threads[thdIndx]->getRouterAt(src)->addToAvgQFrom( ec->averageQFactor );
		int dest = dcpe->connectionPath[dcpe->connectionLength - 1]->getDestinationIndex();
		threads[thdIndx]->getRouterAt(dest)->addToAvgQTo( ec->averageQFactor );
#endif

		ec->QFactors->clear();
		ec->QTimes->clear();

		stats->count += 1;
		stats->totalTime += ec->connectionEndTime - ec->connectionStartTime;

		stats->totalAverageQFactor += ec->averageQFactor;
		stats->totalInitalQFactor += ec->initQFactor;
		stats->totalPercentQFactor += timebelow;

		if(ec->initQFactor < stats->minInitalQFactor)
			stats->minInitalQFactor = ec->initQFactor;
		else if(ec->initQFactor > stats->maxInitalQFactor)
			stats->maxInitalQFactor = ec->initQFactor;

		if(ec->averageQFactor < stats->minAverageQFactor)
			stats->minAverageQFactor = ec->averageQFactor;
		else if(ec->averageQFactor > stats->maxAverageQFactor)
			stats->maxAverageQFactor = ec->averageQFactor;

		if(ec->belowQFactor < stats->minPercentQFactor)
			stats->minPercentQFactor = ec->belowQFactor;
		else if(ec->belowQFactor > stats->maxPercentQFactor)
			stats->maxPercentQFactor = ec->belowQFactor;

		delete ec->QFactors;
		delete ec->QTimes;
	}

	delete ec;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetQMDegredation
// Description:		Resets the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Edge::resetEdgeStats()
{
	stats->minInitalQFactor = std::numeric_limits<float>::infinity();
	stats->minAverageQFactor = std::numeric_limits<float>::infinity();
	stats->minPercentQFactor = std::numeric_limits<float>::infinity();
	stats->maxInitalQFactor = 0.0;
	stats->maxAverageQFactor = 0.0;
	stats->maxPercentQFactor = 0.0;
	stats->totalInitalQFactor = 0.0;
	stats->totalAverageQFactor = 0.0;
	stats->totalPercentQFactor = 0.0;
	stats->totalTime = 0.0;
	stats->count = 0;
	stats->droppedConnections = 0;
}

#ifdef RUN_GUI
///////////////////////////////////////////////////////////////////
//
// Function Name:	updateGUI
// Description:		Updates the GUI for this edge
//
///////////////////////////////////////////////////////////////////
void Edge::updateGUI()
{
	if(threadZero->getQualityParams().q_factor_stats == true)
	{
		actualUsage = unsigned short int(establishedConnections.size());
	}
	else
	{
		actualUsage = 0.0;

		for(unsigned short int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
		{
			if(status[w] == EDGE_USED)
				++actualUsage;
		}
	}

	usageList.push_back(actualUsage); //put newUsage number into usage history vector

	if(actualUsage > max_actual_usage)
	{
		max_actual_usage = actualUsage; //used for end of simulation visual
	
		if(actualUsage > threadZero->getMaxMaxUsage())
			threadZero->setMaxMaxUsage(actualUsage);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	calculateAverageUsage()
// Description:		Uses vector to find the average usage of the edge
//
///////////////////////////////////////////////////////////////////
void Edge::calculateAverageUsage()
{
	float total = 0;

	for(unsigned short int u = threadZero->getUsageHistStartTime(); u < (threadZero->getUsageHistStartTime() + threadZero->getNumUsagesToDisplay()); ++u )
	{
		total += usageList[u];
	}

	average_usage = total / threadZero->getNumUsagesToDisplay(); 
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintSpans()
// Description:		paints the number of spans of the edge it its 
//					center.
//
///////////////////////////////////////////////////////////////////
void Edge::paintSpans()
{
	int x = ((r1x + r2x)/2);//+invunitX*15;
	int y = ((r1y + r2y)/2);//+invunitY*15;
	textprintf_centre_ex(edgespans,font,x,y,makecol(255,255,255),makecol(0,0,0),"%d",numberOfSpans);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintUsage()
// Description:		paints the network in terms of max usage
//
///////////////////////////////////////////////////////////////////
void Edge::paintUsage(int p)
{
	int percent = 0;
	if(p < 0){
		if(p == -1){ //p == -1 if called from update_usage()
		}
		else if(p == -2) 
		{	actualUsage = max_actual_usage;	}
		else if(p == -3 || p == -4)
		{	calculateAverageUsage();
			actualUsage = average_usage;	}
		else if(p == -5) //called from build topology
		{	actualUsage = 0;
			percent = 0;}
	}
	else{//if p is positive, print old history
		actualUsage = usageList[p];
	}

	if(p != -5)
	{
		percent = 1000 * actualUsage / threadZero->getNumberOfWavelengths();
		if(percent < 1)			//0.1%
			percent = 0;
		else if(percent < 5)	//0.5%
			percent = 1;
		else if(percent < 10)	//1%
			percent = 2;
		else if(percent < 50)	//5%
			percent = 3;
		else if(percent < 100)	//10%
			percent = 4;
		else if(percent < 200)	//20%
			percent = 5;
		else if(percent < 300)	//30%
			percent = 6;
		else if(percent < 400)	//40%
			percent = 7;
		else if(percent < 500)	//50%
			percent = 8;
		else if(percent < 600)	//60%
			percent = 9;
		else if(percent < 700)	//70%
			percent = 10;
		else if(percent < 800)	//80%
			percent = 11;
		else if(percent < 900)	//90%
			percent = 12;
		else //100%
			percent = 13;
	}
	if(percent != painted_percent || p == -5) //p == -4,-3,or-2 will be sign that we are painting over again.
	{	masked_blit(edgeBmps[percent],edgesbmp,0,0,putX-7,putY-7,edgeBmps[0]->w,edgeBmps[0]->h);
		painted_percent = percent;	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintUsageHistory(int x1,int y1, int x2,int y2)
// Description:		paints a graph displaying usage history for edge
//
///////////////////////////////////////////////////////////////////
void Edge::paintUsageHistory(int x1,int y1, int x2,int y2,int t)
{
	float bwidth = (x2 - x1 - 50); //calculates bar width
	float help = threadZero->getNumUsagesToDisplay();//15.0;//usageList.size();
	bwidth = bwidth/help;
	float realx = x1 + 40;
	int x = realx;
	int col = makecol(0,255,0);
	int bla = makecol(0,0,0);
	int pink = makecol(255,0,255);
	int timeFact = 1;
	if(threadZero->isPaintRealTime())
	{	timeFact = threadZero->getQualityParams().gui_update_interval;	}
	if(t > 0){	
		col = makecol(255,255,255);
		x = ceil(x + (t - threadZero->getUsageHistStartTime()) * bwidth);
		clear_to_color(buffer,pink);
		masked_blit(graph, buffer, 0,0,0,0,SCREEN_W,SCREEN_H);
		rectfill(buffer, x, y1+10, x, y2 - 20,col);//time scroller line
		textprintf_right_ex(buffer,font,x,y1+10,col,bla,"%d",usageList[t]);
		textprintf_ex(buffer,font,x+4,y1+10,col,bla,"t:%d",t * timeFact);
		masked_blit(buffer, popup, 0,0,0,0,SCREEN_W,SCREEN_H);
		masked_blit(popup,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	}
	else{
		clear_to_color(graph,makecol(255,0,255));//clear graph because we're making a new one.
		int timeUnitModder = 0;
		int truegreen = makecol(0,255,0);
		if(threadZero->getNumUsagesToDisplay() < 100)
		{	timeUnitModder = 10;	}
		else if(threadZero->getNumUsagesToDisplay() < 500)
		{	timeUnitModder = 50;	}
		else if(threadZero->getNumUsagesToDisplay() < 1000)
		{	timeUnitModder = 100;	}
		else if(threadZero->getNumUsagesToDisplay() < 5000)
		{	timeUnitModder = 500;	}
		else if(threadZero->getNumUsagesToDisplay() < 10000)
		{	timeUnitModder = 1000;	}
		else
		{	timeUnitModder = 5000;	}

		int bheight;
		float realBH;
		float mid = 0.40 * static_cast<float>(threadZero->getNumberOfWavelengths()); //drawing parameters
		int green, red, increment, unit;
		float use;
		//sets stretch and units of graph based on max_usage. 
		float realIncrement = (y2 - 20 - y1 - 10)/(threadZero->getMaxMaxUsage() + 0.5);
		increment = ceil(realIncrement); 
		unit = 0;
		rectfill(graph, x1, y1, x2, y2, bla); //paint graph black first
		//blit(graphbackground,graph,0,20,0,SCREEN_H-150,SCREEN_W,150);
		stretch_blit(graphbackground,graph,0,0,graphbackground->w,graphbackground->h,0,y1,x2-x1,y2-y1);
		realBH = y2 - 20.0;
		int drawnUnitPx = y2;
		bheight = ceil(realBH);//temporarily set to place unit markers
		rectfill(graph, x, bheight, x, y1+9, col);
		rectfill(graph, x, y2-19, x2-10, y2-19, col);
		while(bheight > y1 + 10) //add ten px just to make it cleaner
		{
			if(bheight < (drawnUnitPx - 15.0) ){//10 px used as safe distance to put between numbers
				rectfill(graph, x - 5, bheight+1, x, bheight, col); //unit mark (10 px by 1 px)
				textprintf_right_ex(graph,font,x-6,bheight-5,col,pink,"%d", unit);
				drawnUnitPx = bheight;
			}
			realBH -= realIncrement;
			bheight = realBH;
			unit++;
		}

		textprintf_right_ex(graph,font,x2-5,y1-10,bla,pink,"From %d to %d", sourceIndex, destinationIndex);
		
		//don't use all calls to threadZero in future
		for(unsigned short int u = threadZero->getUsageHistStartTime(); u < (threadZero->getUsageHistStartTime() + threadZero->getNumUsagesToDisplay()); u++)
		{
			//(for color) not really necessary
			use = usageList[u];
			if(use <= mid){ //we'll say 2 is our yellow point for now. TODO: make constant
				green = 255;
				red = 255 * (use/mid);
			}
			else{
				green = 255 - 255 * ((use - mid) / mid);
				if(green < 0)
					green = 0;
					red = 255;
				}

			if(u % timeUnitModder == 0)//if true, draw unit marker
			{
				rectfill(graph, x, y2 - 20, x+1, y2-15, truegreen);
				textprintf_centre_ex(graph,font,x,y2-10,truegreen,bla,"%d",u * timeFact);
			}

			col = makecol(red,green,0);
			//(for color) not really necessary
			realBH = (realIncrement * usageList[u]) + 20.0;
			bheight = realBH;
			if(u < usageList.size())
			{	rectfill(graph, x, y2 - bheight, x + bwidth, y2 - 20, col);	}
			realx += bwidth;
			x = realx;
		}
		x = x1 + 40;
		calculateAverageUsage();
		realBH = (realIncrement * average_usage) + 20.0;
		bheight = realBH;
		rectfill(graph, x, y2 - bheight, x2 - 10, y2 - bheight,col);//average line
		textprintf_right_ex(graph,font,x2 - 10,y2 - (bheight + 10),col,bla,"average = %f", average_usage);
		textprintf_right_ex(graph,font,x2-20,y1+25,truegreen,bla,"Maximum Usage: %d", max_actual_usage);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	saveUsageHistory
// Description:		saves entire usageList to file given by input
//					parameter.
//
///////////////////////////////////////////////////////////////////
void Edge::saveData(char* file)
{
	ofstream myFile;
	char buffer[12] = {' '};
	myFile.open (file, std::ios::app | std::ios::binary);
	for(unsigned int u = 0; u < usageList.size(); u++ )
	{
		myFile << usageList.at(u) << "|\n";
	}
	myFile << max_actual_usage <<"|\n";
	myFile.close();

	for(unsigned short int u = 0; u < usageList.size(); ++u)
		usageList.erase(usageList.begin());
	usageList.clear();
	max_actual_usage = 0;
	//directories.erase(directories.begin()+result);
}
#endif
