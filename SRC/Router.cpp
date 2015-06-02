// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Router.cpp
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the declaration of the Router class.
//					The purpose of the Router is to simulate the optical
//					routers in the network, passing messages on their way
//					to the destination and handling them when they get to
//					the final destination.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#include "ErrorCodes.h"
#include "Router.h"

#ifdef RUN_GUI
extern BITMAP *routersbmp;
extern BITMAP *popup;
extern BITMAP *routerinfo;
#endif

#include "Thread.h"

extern Thread* threadZero;

///////////////////////////////////////////////////////////////////
//
// Function Name:	Router
// Description:		Default constructor with no arguements.
//
///////////////////////////////////////////////////////////////////
Router::Router()
{
	adjacencyList = 0;
	
#ifdef RUN_GUI
	sprintf(name,"(no name)");
	rcolor = makecol(0,255,0);
	isSelected = false;
	numWorkstations = 0;
	routerpic = NULL;
	connAttemptsFromThis = 0;
	connAttemptsToThis = 0;
	connSuccessesFromThis = 0;
	connSuccessesToThis = 0;
	avgQTo = 0.0;
	avgQFrom = 0.0;
#endif

	destinationProbs = 0;
	acoProbs = 0;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	~Router
// Description:		Default destructor with no arguements.
//
///////////////////////////////////////////////////////////////////
Router::~Router()
{
	for(unsigned short int a = 0; a < edgeList.size(); ++a)
		delete edgeList[a];

	edgeList.clear();

	delete[] destinationProbs;
	delete[] acoProbs;

#ifdef RUN_GUI
	destroy_bitmap(routerpic);
#endif

	delete[] adjacencyList;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	addEdge
// Description:		Adds edge to the edge list and updates the 
//					adjacency list.
//
///////////////////////////////////////////////////////////////////
void Router::addEdge(Edge* e)
{
	if(adjacencyList == 0)
	{
		adjacencyList = new int[threadZero->getNumberOfRouters()];

		for(unsigned int a = 0; a < threadZero->getNumberOfRouters(); ++a)
			adjacencyList[a] = -1;
	}

	adjacencyList[e->getDestinationIndex()] = (int)edgeList.size();

	edgeList.push_back(e);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getEdgeByDestination
// Description:		Returns a point to the edge connecting to the
//					specific router.
//
///////////////////////////////////////////////////////////////////
Edge* Router::getEdgeByDestination(unsigned short int r)
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		if(edgeList[e]->getDestinationIndex() == r)
			return edgeList[e];
	}

	return 0;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateUsage
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Router::updateUsage()
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->updateUsage();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	generateACOProbabilities
// Description:		Updates the ACO probabilities based upon the
//					current pheremone levels.
//
///////////////////////////////////////////////////////////////////
void Router::generateACOProbabilities(unsigned int dest)
{
	double *destinationTaus = new double[getNumberOfEdges()];
	double *destinationEtas = new double[getNumberOfEdges()];

	if(acoProbs == 0)
	{
		acoProbs = new double[getNumberOfEdges()];
	}

	for(unsigned int e = 0; e < getNumberOfEdges(); ++e)
	{
		if(getEdgeByIndex(e)->getSourceIndex() == getIndex())
		{
			destinationTaus[e] = getEdgeByIndex(e)->getPheremone();

			if(getEdgeByIndex(e)->getDestinationIndex() != dest)
			{
				destinationEtas[e] = 1.0 / double(threadZero->getResourceManager()->span_distance[getEdgeByIndex(e)->getDestinationIndex() * threadZero->getNumberOfRouters() + dest]);
			}
			else
			{
				destinationEtas[e] = 1.0;
			}
		}
		else
		{
			destinationTaus[e] = 0.0;
			destinationEtas[e] = 0.0;
		}
	}

	double cumulativeProduct = 0.0;

	for(unsigned int e2 = 0; e2 < getNumberOfEdges(); ++e2)
	{
		destinationTaus[e2] = pow(destinationTaus[e2],double(threadZero->getQualityParams().ACO_alpha));
		destinationEtas[e2] = pow(destinationEtas[e2],double(threadZero->getQualityParams().ACO_beta));

		cumulativeProduct += destinationTaus[e2] * destinationEtas[e2];
	}

	for(unsigned int e3 = 0; e3 < getNumberOfEdges(); ++e3)
	{
		acoProbs[e3] = (destinationTaus[e3] * destinationEtas[e3]) / cumulativeProduct;

		if(e3 > 0)
			acoProbs[e3] += acoProbs[e3-1];
	}

	acoProbs[getNumberOfEdges() - 1] = 1.0;

	delete[] destinationTaus;
	delete[] destinationEtas;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	chooseEdge
// Description:		Randomly chooses an edge based upon the 
//					probabilities.
//
///////////////////////////////////////////////////////////////////
Edge* Router::chooseEdge(float p)
{
	if(p <= acoProbs[0])
		return getEdgeByIndex(0);

	for(unsigned int r = 0; r < getNumberOfEdges(); ++r)
	{
		if(p > acoProbs[r] && p <= acoProbs[r+1])
		{
			return getEdgeByIndex(r+1);
		}
	}

	return 0;
}

#ifdef RUN_GUI

///////////////////////////////////////////////////////////////////
//
// Function Name:	refreshedgebmps()
// Description:		calls refreshbmps for each edge, which recreates
//					edge bmps based on new pixel locations of routers
//
///////////////////////////////////////////////////////////////////
void Router::refreshedgebmps(bool useThread)
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->refreshbmps(useThread);
	}
}

void Router::scaleEdgesTo(int spns, int px)
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->scaleEdgesTo(spns,px);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateUsage
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Router::updateGUI()
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->updateGUI();
	}
}
#endif

///////////////////////////////////////////////////////////////////
//
// Function Name:	generateProbabilities
// Description:		Generates the probabilites as destionations.
//
///////////////////////////////////////////////////////////////////
void Router::generateProbabilities()
{
	double totalProbs = 0.0;
	double cumulativeProbs = 0.0;

	unsigned int pathSpans = 0;

	destinationProbs = new double[threadZero->getNumberOfRouters()];

	for(unsigned int r1 = 0; r1 < threadZero->getNumberOfRouters(); ++r1)
	{
		if(r1 != getIndex())
		{
			pathSpans = 0;

			kShortestPathReturn *kPath = threadZero->getResourceManager()->calculate_SP_path(r1,getIndex(),1,0);

			for(unsigned int r2 = 0; r2 < kPath->pathlen[0] - 1; ++r2)
				pathSpans += threadZero->getRouterAt(kPath->pathinfo[r2])->
					getEdgeByDestination(kPath->pathinfo[r2+1])->getNumberOfSpans();

			if(threadZero->getQualityParams().dest_dist == DISTANCE)
				destinationProbs[r1] = pathSpans;
			else if(threadZero->getQualityParams().dest_dist == INVERSE_DISTANCE)
				destinationProbs[r1] = 1.0 / pathSpans;
			else
				destinationProbs[r1] = 1.0;

			totalProbs += destinationProbs[r1];

			delete[] kPath->pathcost;
			delete[] kPath->pathinfo;
			delete[] kPath->pathlen;

			delete kPath;
		}
		else
		{
			destinationProbs[r1] = 0.0;
		}
	}

	for(unsigned int r2 = 0; r2 < threadZero->getNumberOfRouters(); ++r2)
	{
		destinationProbs[r2] = destinationProbs[r2] / totalProbs;

		cumulativeProbs += destinationProbs[r2];
		destinationProbs[r2] = cumulativeProbs;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	generateDestination
// Description:		Generates a destination based upon the
//					probability distribution.
//
///////////////////////////////////////////////////////////////////
unsigned int Router::generateDestination(double p)
{
	if(p < destinationProbs[0])
		return 0;

	for(unsigned int r = 0; r < threadZero->getNumberOfRouters() - 1; ++r)
	{
		if(p > destinationProbs[r] && p < destinationProbs[r+1])
		{
			return r + 1;
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetUsage
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Router::resetUsage()
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->resetAlgorithmUsage();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetFailures
// Description:		Resets the failure counts
//
///////////////////////////////////////////////////////////////////
void Router::resetFailures()
{
	qualityFailures = 0;
	waveFailures = 0;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	resetQMDegredation
// Description:		Updates the resetQMDegredation of the edges
//
///////////////////////////////////////////////////////////////////
void Router::resetQMDegredation()
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->resetQMDegredation();
	}
}

#ifdef RUN_GUI
///////////////////////////////////////////////////////////////////
//
// Function Name:	paintMaxUsage()
// Description:		Updates the usage of the edges
//
///////////////////////////////////////////////////////////////////
void Router::paintUsage(int p)
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		edgeList[e]->paintUsage(p);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintEdgeSpans()
// Description:		Paints the number of spans on each edge.
//
///////////////////////////////////////////////////////////////////
void Router::paintEdgeSpans()
{
	for(unsigned short int e = 0; e < edgeList.size(); ++e)
	{
		if(edgeList[e]->getSourceIndex() > edgeList[e]->getDestinationIndex())
			edgeList[e]->paintSpans();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintNumDests()
// Description:		Paints the router with the number of its numDests
//
///////////////////////////////////////////////////////////////////
void Router::paintNumDests(int n)
{
	int black = makecol(0,0,0);
	int number = 5; //whatever
	float quotient;
	
	if(n <= 0) //in this case, total active wks are passed in as a negative number
	{
		if(n == 0)
			quotient = 100.0;
		else{
			quotient = 100.0 *  float(numWorkstations)/float(-1 * n);
		}
		setRadius(quotient);
		number = routerIndex;
	}
	else if(n == 1) //1 Paints percentage of connection attempts which fail in reaching each router
	{	if(connAttemptsToThis != 0)
			quotient = 100 - (100.0 * ((double)connSuccessesToThis/(double)connAttemptsToThis));
		else
			quotient = 0;
		setRadius(quotient);
		number = quotient;
	}
	else if(n == 2)
	{	if(connAttemptsFromThis != 0)
			quotient = 100 - (100.0 * ((double)connSuccessesFromThis/(double)connAttemptsFromThis));
		else
			quotient = 0;
		setRadius(quotient);
		number = quotient;
	}
	else if(n == 3)
	{	quotient = 100.0 *  float(numWorkstations)/float(threadZero->getCurrentActiveWorkstations() );
		setRadius(quotient);
		number = numWorkstations;
	}
	else if(n == 4) //0 means paint indexes
	{
		quotient = 100.0 *  float(numWorkstations)/float(threadZero->getCurrentActiveWorkstations() );
		setRadius(quotient);
		number = routerIndex;}

	char bmpFile[40];
	sprintf(bmpFile,"BITMAPS");

	switch(radius){
		case 10:
			sprintf(bmpFile,"%s/router0_1",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router0_1plain.bmp",NULL);
			break;
		case 11:
			sprintf(bmpFile,"%s/router0_5",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router0_5plain.bmp",NULL);
			break;
		case 12:
			sprintf(bmpFile,"%s/router1_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router1_0plain.bmp",NULL);
			break;
		case 13:
			sprintf(bmpFile,"%s/router5_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router5_0plain.bmp",NULL);
			break;
		case 15:
			sprintf(bmpFile,"%s/router10_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router10_0plain.bmp",NULL);
			break;
		case 17:
			sprintf(bmpFile,"%s/router20_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router20_0plain.bmp",NULL);
			break;
		case 21:
			sprintf(bmpFile,"%s/router30_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router30_0plain.bmp",NULL);
			break;
		case 25:
			sprintf(bmpFile,"%s/router40_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router40_0plain.bmp",NULL);
			break;
		case 29:
			sprintf(bmpFile,"%s/router50_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router50_0plain.bmp",NULL);
			break;
		case 33:
			sprintf(bmpFile,"%s/router60_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router60_0plain.bmp",NULL);
			break;
		case 37:
			sprintf(bmpFile,"%s/router70_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router70_0plain.bmp",NULL);
			break;
		case 41:
			sprintf(bmpFile,"%s/router80_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router80_0plain.bmp",NULL);
			break;
		case 45:
			sprintf(bmpFile,"%s/router90_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router90_0plain.bmp",NULL);
			break;
		case 50:
			sprintf(bmpFile,"%s/router100_0",bmpFile);
			//routerpic = load_bitmap("BITMAPS/router100_0plain.bmp",NULL);
			break;
	}
	if(isSelected)
	{	sprintf(bmpFile,"%sselected.bmp",bmpFile);
		selectScreen();	}
	else
	{	sprintf(bmpFile,"%splain.bmp",bmpFile);}

	routerpic = load_bitmap(bmpFile,NULL);

	masked_stretch_blit(routerpic, routersbmp, 0,0, routerpic->w, routerpic->h, xPixel - (radius), yPixel - (radius), radius * 2, radius * 2);
	textprintf_centre_ex(routersbmp,font,xPixel,yPixel-3,black,-1,"%d", number);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	selectScreen()
// Description:		Draws popup box displaying router information
//
///////////////////////////////////////////////////////////////////
void Router::selectScreen()
{
	int startX = xPixel + 10;
	int startY = yPixel - 105;
	int numtopaint;
	int black = makecol(0,0,0);

	if((startX + 140) > SCREEN_W)
		startX = xPixel - 150;
	if((startY + 220) > SCREEN_H)
		startY = SCREEN_H - 220;
	if((startY < 15))
		startY = 15;

	masked_blit(routerinfo,popup,0,0,startX,startY,SCREEN_W,SCREEN_H);

	numtopaint = routerIndex;
	textprintf_ex(popup,font,startX+65,startY+10,black,-1,"%d", numtopaint);
	textprintf_ex(popup,font,startX+15,startY+35,black,-1,"\"%s\"", name);
	numtopaint = numWorkstations;
	textprintf_right_ex(popup,font,startX + 129,startY+57, black,-1,"%d", numtopaint);
	numtopaint = 100.0 *  float(numWorkstations)/float(threadZero->getCurrentActiveWorkstations() );
	textprintf_right_ex(popup,font,startX + 129,startY+72, black,-1,"%d", numtopaint);
	numtopaint = edgeList.size();
	textprintf_right_ex(popup,font,startX + 129,startY+90, black,-1,"%d", numtopaint);

	if(connAttemptsToThis == 0)
		numtopaint = 0;
	else
		numtopaint = 100 - (100.0 * ((double)connSuccessesToThis/(double)connAttemptsToThis));

	textprintf_right_ex(popup,font,startX + 129,startY+130, black,-1,"%d", numtopaint);

	//for percentage of failures by dest
	int totalfailures = 0;
	for(int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		totalfailures += threadZero->getRouterAt(r)->getConnAttemptsTo() - threadZero->getRouterAt(r)->getConnSuccessesTo();
	}
	if(totalfailures == 0)
		numtopaint = 0;
	else
		numtopaint = floor( 100.0 * ((double)(connAttemptsToThis - connSuccessesToThis)) / (double)totalfailures);
	textprintf_right_ex(popup,font,startX + 129,startY+158, black,-1,"%d", numtopaint);

	if(connAttemptsFromThis ==0)
		numtopaint = 0;
	else
		numtopaint = 100 - (100.0 * ((double)connSuccessesFromThis/(double)connAttemptsFromThis));
	textprintf_right_ex(popup,font,startX + 129,startY+191, black,-1,"%d", numtopaint);

	//for percentage of failures by source
	totalfailures = 0;
	for(int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		totalfailures += threadZero->getRouterAt(r)->getConnAttemptsFrom() - threadZero->getRouterAt(r)->getConnSuccessesFrom();
	}
	if(totalfailures == 0)
		numtopaint = 0;
	else
		numtopaint = floor( 100.0 * ((double)(connAttemptsFromThis - connSuccessesFromThis))/ (double)totalfailures);
	textprintf_right_ex(popup,font,startX + 129,startY+219, black,-1,"%d", numtopaint);

	textprintf_right_ex(popup,font,startX + 129,startY+248, black,-1,"%2.3f", avgQTo);
	textprintf_right_ex(popup,font,startX + 129,startY+263, black,-1,"%2.3f", avgQFrom);
	//numtopaint = avgQTo 
	//textprintf_ex(popup,font,startX+5,startY+170,black,-1,"Avg. Q Factor: ");
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setNumWorkstations(int w)
// Description:		Tells router how many active workstations are attached to 
//					it and sets its painting radius and color accordingly.
//
///////////////////////////////////////////////////////////////////
void Router::setNumWorkstations(int w)
{
	numWorkstations = w;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setRadius(float percentage)
// Description:		sets router radius according to percentage passed in.
//
///////////////////////////////////////////////////////////////////
void Router::setRadius(float pct)
{
	if(int(pct) == 0)
		radius = 10;
	else if(pct <= 0.1)
		radius = 10;
	else if(pct <= 0.5)
		radius = 11;
	else if(pct <= 1)
		radius = 12;
	else if(pct <= 5)
		radius = 13;
	else if(pct <= 10)
		radius = 15;
	else if(pct <= 20)
		radius = 17;
	else if(pct <= 30)
		radius = 21;
	else if(pct <= 40)
		radius = 25;
	else if(pct <= 50)
		radius = 29;
	else if(pct <= 60)
		radius = 33;
	else if(pct <= 70)
		radius = 37;
	else if(pct <= 80)
		radius = 41;
	else if(pct <= 90)
		radius = 45;
	else
		radius = 50;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paintProgress()
// Description:		Paints a progress bar.
//
///////////////////////////////////////////////////////////////////
void Router::paintProgress(int x,int y,int h, double PaintTime,double TimePerPx,int r, int g, int b)
{
	int prog = PaintTime / TimePerPx;
	rectfill(screen, x, y, x + prog, y + h, makecol(r,g,b));
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	saveData()
// Description:		saves all relevant data to recreate simulation
//					for exploring history later, to file given.
//
///////////////////////////////////////////////////////////////////
void Router::saveData(char* file)
{
	ofstream myFile;
	char buffer[7] = {' '};
	myFile.open (file, std::ios::app | std::ios::binary);
	myFile << numWorkstations << "|\n";
	myFile << connAttemptsFromThis << "|\n";
	myFile << connAttemptsToThis << "|\n";
	myFile << connSuccessesFromThis << "|\n";
	myFile << connSuccessesToThis << "|\n";
	avgQTo = avgQTo / (double)connSuccessesToThis;
	myFile << avgQTo << "\n";
	avgQFrom = avgQFrom / (double)connSuccessesFromThis;
	myFile << avgQFrom << "\n";

	myFile.close();
	for(unsigned int i = 0; i < edgeList.size(); i++)
	{
		edgeList[i]->saveData(file);
	}
	numWorkstations = 0;
	connAttemptsFromThis = 0;
	connAttemptsToThis = 0;
	connSuccessesFromThis = 0;
	connSuccessesToThis = 0;
	avgQTo = 0.0;
	avgQFrom = 0.0;
}
#endif
