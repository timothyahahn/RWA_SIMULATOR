// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      GUI.cpp
//  Author:         Andrew Albers, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains many of the functions necessary for the
//					GUI.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#ifdef RUN_GUI
#include "AllegroWrapper.h"
#include <stdlib.h>
#include "Edge.h"
#include "Thread.h"

extern Thread* threadZero;
extern Thread** threads;

extern void runSimulation(int argc, const char* argv[]);

void paint_link_usage(int p);
void paint_router_nums();
void paint_num_dests();
void paint_num_prob_fails();
void paint_num_qual_fails();
void explore_history();

void show_spans();
void show_maximums();
void show_averages();
void show_indexes();
void show_workstations();
void show_failures_from();
void show_failures_to();
void show_key();
void show_detailScreen();

void topo_zoomin();
void topo_zoomout();
void topo_move();

void graphleft();
void graphright();
void graphpoint();
void graphzoomin();
void graphzoomout();
void graphzoomwindow();
void graphmove();

void buildTopoSetFunction(int tf);
void buildtopoplacerouter();
void buildtopoplaceedge();
void buildtopomoverouter();
void buildtopoeditrouter();
const char* edit_inputter(bool alignleft,int xpx,int ypx,int wid,int hei);
int edit_inputint(bool alignleft,int edit, int xpx,int ypx,int wid,int hei);
void buildtopoeditedge();
void scaleAllEdgesTo(int spns, int px);
void paint_topo_edges(int p);
void topoRefresh();

void switcher( int argc, const char* argv[] );

bool userDisplayEdge();
void initialize_topbuttons();
void initialize_graphbuttons();
bool buildTopoMenu();

void userBuildTopo();
void userLoadPrevSim(int argc, const char* argv[]);
void userEditConfigs();
void userRunNewSim( int argc, const char* argv[] );

void menu();

int functionactive; //codes: -1 = nonechosen, 0 = exit, 1 = runNewSim, 2 = loadPrevSim, 3 = buildTopo, 4 = editConfig
int mx,my,mb,mr,color,color2,white,explore_time,graphX1,graphY1,graphX2,graphY2,paintNum, usageWindow,startTime,nums;
int topoX1,topoX2,topoY1,topoY2;
int realTopoWidthPx,realTopoHeightPx,bkgrndX,bkgrndY,numTopoWorkstations,numTopoEdges;
int showrouternum;//codes : showindexes (1), showwks (2), showfailurefrom (3), showfailureto (4) are all represented by values for showrouternum
int graphfunction; //codes: 1 = pointer, 2 = zoomin 3 = zoomout 4 = zoomwindow, 5 = move //graphfunction
int topofunction; //codes: 1 = addrouter, 2 = addedge, 3 = moverouter, 4 = editrouter, 5 = editedge
bool is_exploring, showcolorkey,showmaximums,showaverages;//showindexes,showwks, showfailurefrom,showfailureto

vector<Router*> topoRouters; //for building topology

float clickdist,rx,ry,dmx,dmy,shrinkConstant;
Edge* edge;

BITMAP* flashscreen;
BITMAP* edgeOriginals[14];
BITMAP* arrowOriginals[14];
BITMAP* tailOriginals[14];
BITMAP* topbutton[10];
BITMAP* topobutton[7];
BITMAP* topobuttonover[5];
BITMAP* topbuttonpress[9];
BITMAP* graphbutton[8];
BITMAP* graphbuttonpress[7];
BITMAP* menubutton[5];
BITMAP* menubuttonover[5];
BITMAP* topomenuover[2];

extern BITMAP *buffer;
extern BITMAP *pointer;
extern BITMAP *edgespans;
extern BITMAP *graph;
extern BITMAP *graphbuttons;
extern BITMAP *topbuttons;
extern BITMAP *routersbmp;
extern BITMAP *editrouterinfo;
extern BITMAP *editedgeinfo;
extern BITMAP *edgesbmp;
extern BITMAP *mainbuf;
extern BITMAP *popup;
extern BITMAP *graphbackground;
extern BITMAP *topobackground;
extern BITMAP *colorkey;
extern BITMAP *topomenu;

#define SCRNHEI 700
#define SCRNWID 1300

extern char foldName[25];
extern char folder[50];

///////////////////////////////////////////////////////////////////
//
// Function Name:	paint_link_usage
// Description:		Paints all links according to certian usage from 
//					simulation.
//
///////////////////////////////////////////////////////////////////
void paint_link_usage(int p)
{
	for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		threadZero->getRouterAt(r)->paintUsage(p);
	}
	clear_to_color(mainbuf,makecol(255,0,255));
	blit(edgesbmp, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	masked_blit(routersbmp,mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	masked_blit(topbuttons,mainbuf,0,0,0,0,SCRNWID,topbuttons->h);
	if(showcolorkey)
	{	blit(colorkey,mainbuf,0,0,SCRNWID-colorkey->w - 10,graphY1-16-colorkey->h -10,SCRNWID,SCRNHEI);	}
	if(threadZero->getPaintSpans())
	{	masked_blit(edgespans, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);	}	
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paint_router_nums
// Description:		Displays certain number on each router given by input
//					parameter n. 0 = index, 1 = probfails... the possibilities
//					are endless.
//
///////////////////////////////////////////////////////////////////
void paint_router_nums()
{
	clear_to_color(routersbmp,makecol(255,0,255));
	clear_to_color(mainbuf,makecol(0,0,0));
	
	for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		threadZero->getRouterAt(r)->paintNumDests(paintNum);
	}
	stretch_blit(topobackground,mainbuf,0,0,topobackground->w,topobackground->h,bkgrndX,bkgrndY,realTopoWidthPx,realTopoHeightPx);
	masked_blit(edgesbmp, mainbuf, topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);//makes it black
	if(threadZero->getPaintSpans())
	{	masked_blit(edgespans, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);	}
	masked_blit(routersbmp,mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	masked_blit(topbuttons,mainbuf,0,0,0,0,SCRNWID,SCRNHEI);
	if(edge == NULL)
	{
		stretch_blit(graphbackground,mainbuf,0,0,graphbackground->w,graphbackground->h,graphX1,graphY1,graphX2-graphX1,graphY2-graphY1);
	}
	masked_blit(graphbuttons,mainbuf,0,0,0,graphY1-16,SCRNWID,SCRNHEI);
	masked_blit(graph,mainbuf,graphX1,graphY1-topbuttons->h,graphX1,graphY1-topbuttons->h,SCRNWID,graphY2-(graphY1-topbuttons->h));//size of graph BITMAP should really be changed
	if(showcolorkey)
	{	blit(colorkey,mainbuf,0,0,SCRNWID-colorkey->w - 10,graphY1-16-colorkey->h -10,colorkey->w,colorkey->h);	}
	masked_blit(popup,mainbuf,0,0,0,0,SCRNWID,SCRNHEI);
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	paint_topo_edges
// Description:		Paints all edges with 0% activity for building
//					topology
//
///////////////////////////////////////////////////////////////////
void paint_topo_edges(int p)
{
	clear_to_color(edgesbmp,makecol(255,0,255));
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		topoRouters[r]->paintUsage(p);
	}
	clear_to_color(mainbuf,makecol(255,0,255));
	masked_blit(edgesbmp, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	masked_blit(routersbmp,mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	/*if(threadZero->getPaintSpans())
	{	masked_blit(edgespans, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);	}*/
	masked_blit(mainbuf,screen,0,topbuttons->h,0,topbuttons->h,topoX2-topoX1,topoY2-topoY1);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	topoRefresh
// Description:		refreshes images based on changes to topology
//
///////////////////////////////////////////////////////////////////
void topoRefresh()
{
	clear_to_color(routersbmp,makecol(255,0,255));
	clear_to_color(mainbuf,makecol(0,0,0));
	
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		topoRouters[r]->paintNumDests(-1 * numTopoWorkstations);
	}
	stretch_blit(topobackground,mainbuf,0,0,topobackground->w,topobackground->h,bkgrndX,bkgrndY,realTopoWidthPx,realTopoHeightPx);
	//makes it black
	/*if(threadZero->getPaintSpans())
	{	masked_blit(edgespans, mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);	}*/
	masked_blit(edgesbmp, mainbuf, topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	masked_blit(routersbmp,mainbuf,topoX1,topoY1,topoX1,topoY1,topoX2-topoX1,topoY2-topoY1);
	if(edge == NULL)
	{
		stretch_blit(graphbackground,mainbuf,0,0,graphbackground->w,graphbackground->h,graphX1,graphY1,graphX2-graphX1,graphY2-graphY1);
	}
	masked_blit(graphbuttons,mainbuf,0,0,0,graphY1-16,SCRNWID,SCRNHEI);
	/*if(showcolorkey)
	{	blit(colorkey,mainbuf,0,0,SCRNWID-colorkey->w - 10,graphY1-16-colorkey->h -10,colorkey->w,colorkey->h);	}*/
	masked_blit(popup,mainbuf,0,0,0,0,SCRNWID,SCRNHEI);
	blit(mainbuf,screen,0,topbuttons->h,0,topbuttons->h,topoX2-topoX1,topoY2-topoY1);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	refreshedges()
// Description:		redraws edgesbmp and edgespans to adjust for 
//					some change to the topology viewer, usually 
//					due to user moving or zooming in.
//
///////////////////////////////////////////////////////////////////
void refreshedges()
{
	for(unsigned short int s = 0; s < threadZero->getNumberOfRouters(); ++s)
	{
		threadZero->getRouterAt(s)->refreshedgebmps(true);
	}
	clear_to_color(edgespans,makecol(255,0,255));
	for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		threadZero->getRouterAt(r)->paintEdgeSpans();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	explore_history()
// Description:		Prompts user to choose edge. From here it prompts
//					user to click on graph to choose time of interest.
//					Runs while loop, updating all edges to show usage
//					levels as user pushes arrows.
//
///////////////////////////////////////////////////////////////////
void explore_history()
{
	initialize_topbuttons();
	float aPercent = 0.0;
	paintNum = 4; //initialize to paint router indexes
	bool showSpans = false;
	bool whatev;//does't matter, but this should be false.
	shrinkConstant = 0.05;
	
	showcolorkey = true;	//store to display pressed/unpressed buttons
	showmaximums = false;
	showaverages = false;
	showrouternum = 1;
	graphfunction = 0;
	graphpoint();

	is_exploring = true;

	//initializes number of usages to be displayed in usage history graphs
	threadZero->setNumUsagesToDisplay(500);
	int timeMov = 0;
	nums = threadZero->getRouterAt(0)->getEdgeByIndex(0)->getUsageNums();
	usageWindow = nums;
	threadZero->setNumUsagesToDisplay(usageWindow);//normally
	
	//we could put a lot of this stuff in an initialize function...
	clear_to_color(edgesbmp,makecol(255,0,255));//initialize buffers to magic pink
	clear_to_color(routersbmp,makecol(255,0,255));
	clear_to_color(edgespans,makecol(255,0,255));
	clear_to_color(mainbuf,makecol(255,0,255));
	clear_to_color(graph,makecol(255,0,255));
	explore_time = 1;

	for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		threadZero->getRouterAt(r)->setXYPixels();
		threadZero->getRouterAt(r)->moveXYPixels(0,topbuttons->h);//initially translate below top button bar
	}
	refreshedges();

	//reset all 
	paint_link_usage(-5);
	
	//paint_link_usage(explore_time);
	paint_router_nums();

	while(mouse_b & 1)
	{}

	masked_blit(edgesbmp, mainbuf, 0,0,0,0,SCRNWID,SCRNHEI);
	masked_blit(routersbmp,mainbuf,0,0,0,0,SCRNWID,SCRNHEI);
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
	
	while(!key[KEY_TAB] && !key[KEY_ESC])
	{
		mb = (mouse_b & 1);
		mr = (mouse_b & 2);
		my = mouse_y;
		mx = mouse_x;
		if(mb)
		{
			if(my < topbuttons->h)//pressed on top button bar
			{
				graphpoint(); //set to point
				if(mx < 38)
				{	menu();
					if(functionactive != -1)
					{	return;	}
				}
				else if(mx < 80)
				{	show_spans();
					while(mouse_b & 1)
					{}			}
				else if(mx < 122)
				{	show_maximums();
					while(mouse_b & 1)
					{}			}
				else if(mx < 164)
				{	show_averages();
					while(mouse_b & 1)
					{}			}
				else if(mx < 206)
				{	show_indexes();
					while(mouse_b & 1)
					{}			}
				else if(mx < 248)
				{	show_workstations();
					while(mouse_b & 1)
					{}			}
				else if(mx < 290)
				{	show_failures_from();	
					while(mouse_b & 1)
					{}			}
				else if(mx < 332)
				{	show_failures_to();		
					while(mouse_b & 1)
					{}			}
				else if(mx < 374)
				{	show_key();
					while(mouse_b & 1)
					{}			}
				else if(mx < 416)
				{	show_detailScreen();
					//I don't think we need a mouse_b & 1 loop here.
					//It should be handled in thread->detailScreen()
				}
			}
			else if(my < (graphY1 - topbuttons->h))//clicked in topology area
			{
				if(graphfunction == 5)
				{
					topo_move();
				}
				else
				{
					graphpoint();//set to point
					whatev = userDisplayEdge();
				}	
			}
			else if(my < (graphY1))//clicked on graph button bar
			{
				if(mx < graphbutton[0]->w)
				{	graphleft();		}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w)
				{	graphright();		}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w + graphbutton[2]->w)
				{	graphpoint();		}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w + 2* graphbutton[2]->w) //clicked on zoom in
				{	graphzoomin();		}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w + 3 * graphbutton[2]->w) //clicked on zoom out
				{	graphzoomout();		}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w + 4 * graphbutton[2]->w) //clicked on zoom window
				{	graphzoomwindow();	}
				else if(mx < graphbutton[0]->w + graphbutton[1]->w + 5 * graphbutton[2]->w)
				{	graphmove();		}
			}
			else if((graphfunction == 1) && my < (SCRNHEI)){//if user clicked on graph with pointer, go to that time.
				if(mx < 40)
				{	if(threadZero->getUsageHistStartTime() > 0)
					{	threadZero->setUsageHistStartTime(threadZero->getUsageHistStartTime()-1);
						explore_time = threadZero->getUsageHistStartTime();	}
					else
					{	mx = 40;
						explore_time = threadZero->getUsageHistStartTime() + (mx - 40) * usageWindow / (SCRNWID - 50);	}
				}
				else{
					explore_time = threadZero->getUsageHistStartTime() + (mx - 40) * usageWindow / (SCRNWID - 50);
				}//Think about this
				if(explore_time >= threadZero->getUsageHistStartTime() + usageWindow)
				{	if(explore_time >= nums)
					{	explore_time = threadZero->getUsageHistStartTime() + usageWindow -1;	}
					else
					{	threadZero->setUsageHistStartTime(threadZero->getUsageHistStartTime()+1);	}//Replace the call to threadZero with something quicker
				}
				paint_link_usage(explore_time);
				if(edge != NULL)
				{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
					edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
			}
		}
		else if(key[KEY_RIGHT])
		{
			graphright();
		}
		else if(key[KEY_LEFT])
		{	
			graphleft();
		}
		if(mr || (mb && graphfunction == 5)) //moving the graph
		{
			my = mouse_y;
			mx = mouse_x;
			if(my > (SCRNHEI - 150) && mx > 40 && mx < SCRNWID - 10 && edge != NULL)
			{	explore_time = threadZero->getUsageHistStartTime() + (mx - 40) * usageWindow / (SCRNWID - 50);	
				paint_link_usage(explore_time);
				while((mr || (mb && graphmove)) && mx > 40 && mx < SCRNWID - 10)
				{
					edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
					mx = mouse_x;
					timeMov = explore_time - (threadZero->getUsageHistStartTime() + (mx - 40) * usageWindow / (SCRNWID - 50));
					if(threadZero->getUsageHistStartTime() <= 0 && timeMov < 0)
					{	timeMov = 0;
						threadZero->setUsageHistStartTime(0);	
					}
					else if(threadZero->getUsageHistStartTime() >= (threadZero->getRouterAt(0)->getEdgeByIndex(0)->getUsageNums() - usageWindow) && timeMov > 0)
					{	timeMov = 0;
						threadZero->setUsageHistStartTime(threadZero->getRouterAt(0)->getEdgeByIndex(0)->getUsageNums() - usageWindow);	
					}
					else{
						threadZero->setUsageHistStartTime(threadZero->getUsageHistStartTime() + timeMov);
					}
					mr = (mouse_b & 2);
					mb = (mouse_b & 1);
					edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
					edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);
				}
			}
		}
		else if(key[KEY_UP])
		{	graphzoomin();	}
		else if(key[KEY_DOWN])
		{	graphzoomout();	}
		else if(key[KEY_B])
		{
			show_failures_to();
		}
		else if(key[KEY_C])
		{
			show_failures_from();
		}
		else if(key[KEY_D])
		{
			show_indexes();
		}
		else if(key[KEY_E])
		{
			show_averages();
		}
		else if(key[KEY_F])
		{
			show_maximums();
		}
		else if(key[KEY_K])
		{
			show_key();
			while(key[KEY_K])
			{}
		}
		else if(key[KEY_M])
		{
			topo_zoomin();
		}
		else if(key[KEY_N])
		{
			topo_zoomout();
		}
		else if(key[KEY_R])
		{
			threadZero->setPaintRealTime();
			if(edge != NULL)
			{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
				edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
			while(key[KEY_R])
			{}
		}
		else if(key[KEY_S])
		{
			show_spans();
			while(key[KEY_S])
			{}
		}
		else if(key[KEY_W])
		{
			show_workstations();
		}
		else if(key[KEY_Z])//toggles between showing and hiding directions.
		{
			threadZero->setPaintDir();
			paint_link_usage(-5);
			paint_link_usage(explore_time);
			while(key[KEY_Z])
			{}
		}
	}
	paint_link_usage(-3);//RETURN TO AVERAGES
	is_exploring = false;//no longer exploring history
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_spans()
// Description:		sets topology to show span numbers on edges
//
///////////////////////////////////////////////////////////////////
void show_spans()
{
	threadZero->setPaintSpans();
	paint_link_usage(explore_time);
	if(threadZero->getPaintSpans())
		blit(topbuttonpress[1],topbuttons,0,0,topbuttonpress[0]->w,0,SCRNWID,SCRNHEI);
	else
		blit(topbutton[1],topbuttons,0,0,topbuttonpress[0]->w,0,SCRNWID,SCRNHEI);
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_key()
// Description:		displays bitmap showing color legend for edges
//					toggles on/off
//
///////////////////////////////////////////////////////////////////
void show_key()
{
	int overx = 0;
	for(int i = 0; i < 8; i++)
	{	overx += topbutton[i]->w;	}
	if(showcolorkey)
	{
		showcolorkey = false;
		blit(topbutton[8],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	}
	else
	{
		showcolorkey = true;
		blit(topbuttonpress[8],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	}

	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_detailScreen()
// Description:		calls thread to display its various info on pretty
//					grey screen.
//
///////////////////////////////////////////////////////////////////
void show_detailScreen()
{
	threadZero->detailScreen();
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_maximums()
// Description:		sets topology to show usage maximums on edges
//
///////////////////////////////////////////////////////////////////
void show_maximums()
{
	int overx = 0;
	for(int i = 0; i < 2; i++)
	{	overx += topbutton[i]->w;	}
	if(showmaximums && showaverages)
	{	showmaximums = false;
		blit(topbutton[2],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	showmaximums = true;
		if(showaverages)
		{	show_averages();	}
		blit(topbuttonpress[2],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	paint_link_usage(-2);
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_averages()
// Description:		sets topology to show average usages on edges
//
///////////////////////////////////////////////////////////////////
void show_averages()
{
	int overx = 0;
	for(int i = 0; i < 3; i++)
	{	overx += topbutton[i]->w;	}
	if(showaverages && showmaximums)
	{	showaverages = false;
		blit(topbutton[3],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	showaverages = true;
		if(showmaximums)
		{	show_maximums();	}
		blit(topbuttonpress[3],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	paint_link_usage(-3);
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_indexes()
// Description:		sets topology to show router indexes
//
///////////////////////////////////////////////////////////////////
void show_indexes()
{
	int overx = 0;
	for(int i = 0; i < 4; i++)
	{	overx += topbutton[i]->w;	}
	if(showrouternum == 1)
	{	blit(topbutton[4],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(showrouternum == 3)
		{	show_failures_from();	}
		else if(showrouternum == 4)
		{	show_failures_to();	}
		else if(showrouternum == 2)
		{	show_workstations();	}
		showrouternum = 1;
		blit(topbuttonpress[4],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		paintNum = 4;
		paint_router_nums();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_workstations()
// Description:		sets topology to show router workstation #s
//
///////////////////////////////////////////////////////////////////
void show_workstations()
{//showindexes (1), showwks (2), showfailurefrom (3), showfailureto (4)
	int overx = 0;
	for(int i = 0; i < 5; i++)
	{	overx += topbutton[i]->w;	}
	if(showrouternum == 2)
	{	blit(topbutton[5],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(showrouternum == 3)
		{	show_failures_from();	}
		else if(showrouternum == 4)
		{	show_failures_to();	}
		else if(showrouternum == 1)
		{	show_indexes();	}
		showrouternum = 2;
		blit(topbuttonpress[5],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		paintNum = 3;
		paint_router_nums();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_failures_from()
// Description:		sets topology to show connection failures from 
//					routers
//
///////////////////////////////////////////////////////////////////
void show_failures_from()
{//showindexes (1), showwks (2), showfailurefrom (3), showfailureto (4)
	int overx = 0;
	for(int i = 0; i < 6; i++)
	{	overx += topbutton[i]->w;	}
	if(showrouternum == 3)
	{	blit(topbutton[6],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(showrouternum == 2)
		{	show_workstations();	}
		else if(showrouternum == 4)
		{	show_failures_to();	}
		else if(showrouternum == 1)
		{	show_indexes();	}
		showrouternum = 3;
		blit(topbuttonpress[6],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		paintNum = 2;
		paint_router_nums();
	}
	//Paints percentage of connection attempts which succeeded from this router
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	show_failures_to()
// Description:		sets topology to show connection failures to
//					routers
//
///////////////////////////////////////////////////////////////////
void show_failures_to()
{//showindexes (1), showwks (2), showfailurefrom (3), showfailureto (4)
	int overx = 0;
	for(int i = 0; i < 7; i++)
	{	overx += topbutton[i]->w;	}
	if(showrouternum == 4)
	{	blit(topbutton[7],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(showrouternum == 2)
		{	show_workstations();	}
		else if(showrouternum == 3)
		{	show_failures_from();	}
		else if(showrouternum == 1)
		{	show_indexes();	}
		showrouternum = 4;
		blit(topbuttonpress[7],topbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		paintNum = 1;
		paint_router_nums();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	topo_move
// Description:		Pans the topology around
//
///////////////////////////////////////////////////////////////////
void topo_move()
{
	int diffx = 0;
	int diffy = 0;
	my = mouse_y;
	mx = mouse_x;
	while(mouse_b & 1)
	{
		clear_to_color(edgesbmp,makecol(255,0,255));
		for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)//TODO: much of this is also in explore_history. Don't copy/paste code.
		{
			threadZero->getRouterAt(r)->moveXYPixels(diffx,diffy);//initially translate below top button bar
		}
		bkgrndX += diffx;//reset painting coordinates for topobackground.
		bkgrndY += diffy;
		refreshedges();

		paint_link_usage(-5);
		paint_link_usage(explore_time);
		paint_router_nums();

		diffy = mouse_y - my;
		my = mouse_y;
		diffx = mouse_x - mx;
		mx = mouse_x;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	topo_zoomin
// Description:		Zooms the topology in
//
///////////////////////////////////////////////////////////////////
void topo_zoomin()
{
	while(key[KEY_M])
	{
		if(realTopoWidthPx < 2000)
		{
			clear_to_color(edgesbmp,makecol(255,0,255));
			realTopoWidthPx += (0.25 * realTopoWidthPx);
			realTopoHeightPx += (0.25 * realTopoHeightPx);
			for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)//TODO: much of this is also in explore_history. Don't copy/paste code.
			{
				threadZero->getRouterAt(r)->setXYPixels();
				threadZero->getRouterAt(r)->moveXYPixels(0,topbuttons->h);//initially translate below top button bar
			}
			refreshedges();
			
			paint_link_usage(explore_time);
			paint_router_nums();
		}
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	topo_zoomout
// Description:		Zooms the topology out
//
///////////////////////////////////////////////////////////////////
void topo_zoomout()
{
	while(key[KEY_N])
	{
		clear_to_color(edgesbmp,makecol(255,0,255));
		realTopoWidthPx -= (0.20 * realTopoWidthPx);
		realTopoHeightPx -= (0.20 * realTopoHeightPx);
		if(realTopoWidthPx < (topoX2-topoX1))
			realTopoWidthPx = topoX2-topoX1;
		if(realTopoHeightPx < (topoY2 - topoY1))
			realTopoHeightPx = topoY2 - topoY1;
		for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)//TODO: much of this is also in explore_history. Don't copy/paste code.
		{
			threadZero->getRouterAt(r)->setXYPixels();
			threadZero->getRouterAt(r)->moveXYPixels(0,topbuttons->h);//initially translate below top button bar
		}
		bkgrndX = 0;
		bkgrndY = topbuttons->h;
		refreshedges();
			
		paint_link_usage(explore_time);
		paint_router_nums();
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphleft
// Description:		Moves the graph left
//
///////////////////////////////////////////////////////////////////
void graphleft()
{
	int overx = 0;
	for(int i = 0; i < 0; i++)
	{	overx += graphbutton[i]->w;	}
	blit(graphbuttonpress[0],graphbuttons,0,0,overx,0,graphbuttons->w,graphbuttons->h);	
	blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	while(key[KEY_LEFT] || (mouse_b & 1))
	{
		explore_time--;	
		if(explore_time < threadZero->getUsageHistStartTime())
		{	if(explore_time > 0)
			{	threadZero->setUsageHistStartTime(explore_time);
				if(edge != NULL)
				{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);	}
			}
			else
			{	explore_time = threadZero->getUsageHistStartTime();	}
		}//Replace the call to threadZero with something quicker
		paint_link_usage(explore_time);
		if(edge != NULL)
		{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
	}
	blit(graphbutton[0],graphbuttons,0,0,overx,0,graphbuttons->w,graphbuttons->h);
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphright
// Description:		Moves the graph right
//
///////////////////////////////////////////////////////////////////
void graphright()
{
	int overx = 0;
	for(int i = 0; i < 1; i++)
	{	overx += graphbutton[i]->w;	}
	blit(graphbuttonpress[1],graphbuttons,0,0,overx,0,graphbuttons->w,graphbuttons->h);
	blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	while(key[KEY_RIGHT] || (mouse_b & 1))
	{
		explore_time++;	
		if(explore_time >= threadZero->getUsageHistStartTime() + usageWindow)
		{	if(explore_time < nums)
			{	startTime = threadZero->getUsageHistStartTime();
				threadZero->setUsageHistStartTime(startTime + 1);
				if(edge != NULL)
				{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);	}
			}
			else
			{	explore_time = threadZero->getUsageHistStartTime() + usageWindow -1;	}
		}
		paint_link_usage(explore_time);
		if(edge != NULL)
		{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
	}
	blit(graphbutton[1],graphbuttons,0,0,overx,0,graphbuttons->w,graphbuttons->h);
	paint_router_nums();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphpoint
// Description:		Turns on graph pointer tool and turns off
//					others if necessary. Paints over buttons.
//
///////////////////////////////////////////////////////////////////
void graphpoint()
{
	int overx = 0;
	for(int i = 0; i < 2; i++)
	{	overx += graphbutton[i]->w;	}
	if(graphfunction == 1)
	{	blit(graphbutton[2],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(graphfunction == 4)
		{	graphzoomwindow();	}
		else if(graphfunction == 5)
		{	graphmove();	}
		graphfunction = 1;
		blit(graphbuttonpress[2],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphzoomin
// Description:		Turns on zoomin tool and turns off
//					others if necessary. Paints over buttons.
//
///////////////////////////////////////////////////////////////////
void graphzoomin()
{
	float aPercent;
	int overx = 0;
	for(int i = 0; i < 3; i++)
	{	overx += graphbutton[i]->w;	}

	if(graphfunction == 1)//unpress other buttons
	{	graphpoint();	}
	else if(graphfunction == 4)
	{	graphzoomwindow();	}
	else if(graphfunction == 5)
	{	graphmove();	}

	blit(graphbuttonpress[3],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	while(key[KEY_UP] || (mouse_b & 1))
	{
		if(usageWindow > 50)
		{
			aPercent = float(explore_time - threadZero->getUsageHistStartTime()) / float(usageWindow);
			startTime = ceil(threadZero->getUsageHistStartTime() + aPercent * shrinkConstant * usageWindow);
			threadZero->setUsageHistStartTime(startTime);
			usageWindow -= ceil(shrinkConstant * usageWindow);
			threadZero->setNumUsagesToDisplay(usageWindow);
			if(edge != NULL)
			{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
				edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	
			}
		}
	}
	blit(graphbutton[3],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	paint_router_nums();
	graphfunction = 0;//default back to pointer
	graphpoint();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphzoomout
// Description:		Turns on zoomout tool and turns off
//					others if necessary. Paints over buttons.
//
///////////////////////////////////////////////////////////////////
void graphzoomout()//turns itself on/off. turns point/move off
{
	int overx = 0;
	float aPercent;
	mx = mouse_x;
	for(int i = 0; i < 4; i++)
	{	overx += graphbutton[i]->w;	}

	if(graphfunction == 1)//unpress other buttons
	{	graphpoint();	}
	else if(graphfunction == 4)
	{	graphzoomwindow();	}
	else if(graphfunction == 5)
	{	graphmove();	}

	blit(graphbuttonpress[4],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	while(key[KEY_DOWN] || (mouse_b & 1))
	{
		aPercent = float(explore_time - threadZero->getUsageHistStartTime()) / float(usageWindow);
		startTime = ceil(threadZero->getUsageHistStartTime() - aPercent * (shrinkConstant) * usageWindow);
		if(startTime < 0)
		{	startTime = 0;	}
		usageWindow += ceil(shrinkConstant * usageWindow);
		if(usageWindow > nums)
		{	usageWindow = nums;	}
		if((startTime + usageWindow) > nums)
		{	startTime = nums - usageWindow; }
		threadZero->setUsageHistStartTime(startTime);
		threadZero->setNumUsagesToDisplay(usageWindow);
		if(edge != NULL)
		{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
			edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
	}
	blit(graphbutton[4],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	paint_router_nums();
	graphfunction = 0;//default back to pointer
	graphpoint();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphzoomwindow
// Description:		Turns on zoom window tool and turns off
//					others if necessary. Paints over buttons.
//
///////////////////////////////////////////////////////////////////
void graphzoomwindow()
{
	int overx = 0;
	int choseA, choseB;
	mx = mouse_x;
	for(int i = 0; i < 5; i++)
	{	overx += graphbutton[i]->w;	}
	if(graphfunction == 4)
	{	blit(graphbutton[5],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(graphfunction == 1)
		{	graphpoint();	}
		else if(graphfunction == 5)
		{	graphmove();	}
		graphfunction = 0;
		blit(graphbuttonpress[5],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);

		while(mouse_b & 1)
		{}
		while(!(mouse_b & 1))//wait for user to click
		{}
		mx = mouse_x;
		if(mouse_y > graphY1)//if clicked in graph area
		{
			if(mx < 40)
			{	choseA = 40;	}
			else if(mx > SCRNHEI - 10)
			{	choseA = SCRNHEI - 10;	}
			else
			{	choseA = mx;	}
			while(mouse_b & 1)
			{}

			mx = mouse_x;
			if(mx < 40)
			{	choseB = 40;	}
			else if(mx > SCRNHEI - 10)
			{	choseB = SCRNHEI - 10;	}
			else
			{	choseB = mx;	}

			if(abs(choseB - choseA) > 15)
			{
				if(choseB < choseA)
				{
					int temp = choseB;
					choseB = choseA;
					choseA = temp;
				}
				choseA = threadZero->getUsageHistStartTime() + (choseA - 40) * usageWindow / (SCRNWID - 50);
				choseB = threadZero->getUsageHistStartTime() + (choseB - 40) * usageWindow / (SCRNWID - 50);
				if(choseB >= threadZero->getUsageHistStartTime() + usageWindow)
				{	choseB = threadZero->getUsageHistStartTime() + usageWindow - 1;	}
				if(choseA < threadZero->getUsageHistStartTime())
				{	choseA = threadZero->getUsageHistStartTime();	}
				threadZero->setUsageHistStartTime(choseA);
				usageWindow = choseB-choseA;
				threadZero->setNumUsagesToDisplay(usageWindow);
				explore_time = threadZero->getUsageHistStartTime();
			}
			else
			{
				blit(graphbutton[5],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
				graphpoint();
				return;
			}
		}
		else
		{
			graphfunction = 4;
			graphpoint();
			return;
		}

		paint_link_usage(choseA);
		if(edge != NULL)
		{	edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,-1);
			edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);	}
	}
	blit(graphbutton[5],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
	graphfunction = 0;
	graphpoint();//default return to pointer
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	graphmove
// Description:		Turns on graph move tool and turns off
//					others if necessary. Paints over buttons.
//
///////////////////////////////////////////////////////////////////
void graphmove()
{
	int overx = 0;
	mx = mouse_x;
	for(int i = 0; i < 6; i++)
	{	overx += graphbutton[i]->w;	}
	if(graphfunction == 5)
	{	blit(graphbutton[6],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);	}
	else
	{	if(graphfunction == 1)
		{	graphpoint();	}
		else if(graphfunction == 4)
		{	graphzoomwindow();	}
		graphfunction = 5;
		blit(graphbuttonpress[6],graphbuttons,0,0,overx,0,SCRNWID,SCRNHEI);
		blit(graphbuttons,screen,0,0,0,graphY1-graphbutton[0]->h,SCRNWID,SCRNHEI);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	userDisplayEdge()
// Description:		function which lets user choose an edge and paints
//					its history graph. returns time if desired.
//
///////////////////////////////////////////////////////////////////
bool userDisplayEdge()//return true if it should go to explore edge next
{
	int source = -1;
	mx = mouse_x;
	my = mouse_y;
	mb = (mouse_b & 1);
	for(unsigned short int r = 0; r < threadZero->getNumberOfRouters(); ++r)
	{
		dmx = mx;
		dmy = my;
		rx = threadZero->getRouterAt(r)->getXPixel(); //set coordinates of routers
		ry = threadZero->getRouterAt(r)->getYPixel();
		clickdist = sqrt( ( dmx - rx )*( dmx - rx ) + ( dmy - ry )*( dmy - ry) );//distance formula
		if(clickdist < threadZero->getRouterAt(r)->getRadius())
		{	source = r;	}
	}

	if(source == -1)//if nothing clicked, go back
		return false;

	while(mouse_b & 1)//wait for unclick
	{}

	mr = (mouse_b & 2);
	mb = (mouse_b & 1);

	while(!mr && !mb){
		mr = (mouse_b & 2);
		mb = (mouse_b & 1);
		mx = mouse_x;
		my = mouse_y;
	}
	if(mb && source > -1){//if we've clicked again and we've already selected one router.
		for(unsigned short int r2 = 0; r2 < threadZero->getNumberOfRouters(); ++r2)
		{
			dmx = mx;
			dmy = my;
			rx = threadZero->getRouterAt(r2)->getXPixel(); //set coordinates of routers
			ry = threadZero->getRouterAt(r2)->getYPixel();
			clickdist = sqrt( ( dmx - rx )*( dmx - rx ) + ( dmy - ry )*( dmy - ry) );//distance formula
			if(clickdist < threadZero->getRouterAt(r2)->getRadius()){ //this should eventually be a global constant I think.
				if(threadZero->getRouterAt(source)->isAdjacentTo(r2) != -1){
						edge = threadZero->getRouterAt(source)->getEdgeByDestination(r2);
						edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,0);
						//edge->paintUsageHistory(graphX1,graphY1,graphX2,graphY2,explore_time);
						clear_to_color(popup,makecol(255,0,255));
						paint_router_nums();
						while(mouse_b & 1)
						{}
						return false;
						//ADDED FOR EXPLORING HISTORY
				}
				else if(r2 == source)
				{
					//clear_to_color(graph,makecol(255,0,255)); //taken out to fix graph drawing bug
					//blit(graphbackground,graph,0,20,0,SCRNHEI-150,SCRNWID,150);

					threadZero->getRouterAt(source)->setSelected();
					paint_router_nums();
					while(mouse_b & 1)//wait for unclick
					{}
					while(!(mouse_b & 1))//wait for click again
					{}
					threadZero->getRouterAt(source)->setSelected();
					clear_to_color(popup,makecol(255,0,255));
					paint_router_nums();
					return false;
					//paint_router_nums(); //this is all it was before, after clear_to_color()
				}
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	close_button_handler
// Description:		Sets the terminate flag in the first thread
//					to terminate the program.
//
///////////////////////////////////////////////////////////////////
void close_button_handler()
{
	threadZero->setTerminate();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	flash
// Description:		Loads the flash screen and loads bitmaps
//
///////////////////////////////////////////////////////////////////
void flash()
{
	flashscreen = load_bitmap("BITMAPS/flashscreen.bmp",NULL); //flashscreen for ALL-GUI
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,flashscreen->w + 100,flashscreen->h + 100,0,0);
	blit(flashscreen, screen, 0,0,ceil((double)SCREEN_W/2 - (double)flashscreen->w / 2.0),ceil((double)SCREEN_H/2 - (double)flashscreen->h / 2.0),flashscreen->w,flashscreen->h);///this makes screen black too
	rest(1000);
	destroy_bitmap(flashscreen);
	
	clear_to_color(edgesbmp,makecol(255,0,255));

	//probably move this somewhere more sensible later.
	edgeOriginals[0] = load_bitmap("bitmaps/edge0001.bmp",NULL);
	edgeOriginals[1] = load_bitmap("bitmaps/edge0005.bmp",NULL);
	edgeOriginals[2] = load_bitmap("bitmaps/edge0010.bmp",NULL);
	edgeOriginals[3] = load_bitmap("bitmaps/edge0050.bmp",NULL);
	edgeOriginals[4] = load_bitmap("bitmaps/edge0100.bmp",NULL);
	edgeOriginals[5] = load_bitmap("bitmaps/edge0200.bmp",NULL);
	edgeOriginals[6] = load_bitmap("bitmaps/edge0300.bmp",NULL);
	edgeOriginals[7] = load_bitmap("bitmaps/edge0400.bmp",NULL);
	edgeOriginals[8] = load_bitmap("bitmaps/edge0500.bmp",NULL);
	edgeOriginals[9] = load_bitmap("bitmaps/edge0600.bmp",NULL);
	edgeOriginals[10] = load_bitmap("bitmaps/edge0700.bmp",NULL);
	edgeOriginals[11] = load_bitmap("bitmaps/edge0800.bmp",NULL);
	edgeOriginals[12] = load_bitmap("bitmaps/edge0900.bmp",NULL);
	edgeOriginals[13] = load_bitmap("bitmaps/edge1000.bmp",NULL);

	arrowOriginals[0] = load_bitmap("bitmaps/arrow0001.bmp",NULL);
	arrowOriginals[1] = load_bitmap("bitmaps/arrow0005.bmp",NULL);
	arrowOriginals[2] = load_bitmap("bitmaps/arrow0010.bmp",NULL);
	arrowOriginals[3] = load_bitmap("bitmaps/arrow0050.bmp",NULL);
	arrowOriginals[4] = load_bitmap("bitmaps/arrow0100.bmp",NULL);
	arrowOriginals[5] = load_bitmap("bitmaps/arrow0200.bmp",NULL);
	arrowOriginals[6] = load_bitmap("bitmaps/arrow0300.bmp",NULL);
	arrowOriginals[7] = load_bitmap("bitmaps/arrow0400.bmp",NULL);
	arrowOriginals[8] = load_bitmap("bitmaps/arrow0500.bmp",NULL);
	arrowOriginals[9] = load_bitmap("bitmaps/arrow0600.bmp",NULL);
	arrowOriginals[10] = load_bitmap("bitmaps/arrow0700.bmp",NULL);
	arrowOriginals[11] = load_bitmap("bitmaps/arrow0800.bmp",NULL);
	arrowOriginals[12] = load_bitmap("bitmaps/arrow0900.bmp",NULL);
	arrowOriginals[13] = load_bitmap("bitmaps/arrow1000.bmp",NULL);

	tailOriginals[0] = load_bitmap("bitmaps/tail0001.bmp",NULL);
	tailOriginals[1] = load_bitmap("bitmaps/tail0005.bmp",NULL);
	tailOriginals[2] = load_bitmap("bitmaps/tail0010.bmp",NULL);
	tailOriginals[3] = load_bitmap("bitmaps/tail0050.bmp",NULL);
	tailOriginals[4] = load_bitmap("bitmaps/tail0100.bmp",NULL);
	tailOriginals[5] = load_bitmap("bitmaps/tail0200.bmp",NULL);
	tailOriginals[6] = load_bitmap("bitmaps/tail0300.bmp",NULL);
	tailOriginals[7] = load_bitmap("bitmaps/tail0400.bmp",NULL);
	tailOriginals[8] = load_bitmap("bitmaps/tail0500.bmp",NULL);
	tailOriginals[9] = load_bitmap("bitmaps/tail0600.bmp",NULL);
	tailOriginals[10] = load_bitmap("bitmaps/tail0700.bmp",NULL);
	tailOriginals[11] = load_bitmap("bitmaps/tail0800.bmp",NULL);
	tailOriginals[12] = load_bitmap("bitmaps/tail0900.bmp",NULL);
	tailOriginals[13] = load_bitmap("bitmaps/tail1000.bmp",NULL);

	initialize_topbuttons();
	initialize_graphbuttons();	
	
	graphX1 = 0;
	graphY1 = SCRNHEI - 150;
	graphX2 = SCRNWID;
	graphY2 = SCRNHEI;
	topoX1 = 0;
	topoY1 = topbuttons->h;
	topoX2 = SCRNWID;
	topoY2 = graphY1 - graphbuttons->h;
	realTopoWidthPx = topoX2 - topoX1;
	realTopoHeightPx = topoY2 - topoY1;
	bkgrndX = 0;
	bkgrndY = topbuttons->h;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	initialize_topbuttons
// Description:		Initializes the top buttons bar
//
///////////////////////////////////////////////////////////////////
void initialize_topbuttons()
{
	int xplace = 0;
	topbutton[0] = load_bitmap("BITMAPS/buttonlogo.bmp",NULL);
	blit(topbutton[0], topbuttons, 0,0,xplace,0,topbutton[0]->w,topbutton[0]->h);
	xplace += topbutton[0]->w;

	topbutton[1] = load_bitmap("BITMAPS/buttonspans.bmp",NULL);
	blit(topbutton[1], topbuttons, 0,0,xplace,0,topbutton[1]->w,topbutton[1]->h);
	xplace += topbutton[1]->w;

	topbutton[2] = load_bitmap("BITMAPS/buttonmax.bmp",NULL);
	blit(topbutton[2], topbuttons, 0,0,xplace,0,topbutton[2]->w,topbutton[2]->h);
	xplace += topbutton[2]->w;

	topbutton[3] = load_bitmap("BITMAPS/buttonavg.bmp",NULL);
	blit(topbutton[3], topbuttons, 0,0,xplace,0,topbutton[3]->w,topbutton[3]->h);
	xplace += topbutton[3]->w;

	topbutton[4] = load_bitmap("BITMAPS/buttonindexes.bmp",NULL);
	blit(topbutton[4], topbuttons, 0,0,xplace,0,topbutton[4]->w,topbutton[4]->h);
	xplace += topbutton[4]->w;

	topbutton[5] = load_bitmap("BITMAPS/buttonwks.bmp",NULL);
	blit(topbutton[5], topbuttons, 0,0,xplace,0,topbutton[5]->w,topbutton[5]->h);
	xplace += topbutton[5]->w;

	topbutton[6] = load_bitmap("BITMAPS/buttonfailsrc.bmp",NULL);
	blit(topbutton[6], topbuttons, 0,0,xplace,0,topbutton[6]->w,topbutton[6]->h);
	xplace += topbutton[6]->w;

	topbutton[7] = load_bitmap("BITMAPS/buttonfaildest.bmp",NULL);
	blit(topbutton[7], topbuttons, 0,0,xplace,0,topbutton[7]->w,topbutton[7]->h);
	xplace += topbutton[7]->w;

	topbutton[8] = load_bitmap("BITMAPS/buttonkey.bmp",NULL);
	blit(topbutton[8], topbuttons, 0,0,xplace,0,topbutton[8]->w,topbutton[8]->h);
	xplace += topbutton[8]->w;

	topbutton[9] = load_bitmap("BITMAPS/buttontopstretch.bmp",NULL);
	stretch_blit(topbutton[9], topbuttons, 0,0,topbutton[9]->w,topbutton[9]->h,xplace,0,SCRNWID-xplace,topbutton[9]->h);

	topbuttonpress[0] = load_bitmap("BITMAPS/buttonlogopress.bmp",NULL);
	topbuttonpress[1] = load_bitmap("BITMAPS/buttonspanspress.bmp",NULL);
	topbuttonpress[2] = load_bitmap("BITMAPS/buttonmaxpress.bmp",NULL);
	topbuttonpress[3] = load_bitmap("BITMAPS/buttonavgpress.bmp",NULL);
	topbuttonpress[4] = load_bitmap("BITMAPS/buttonindexespress.bmp",NULL);
	topbuttonpress[5] = load_bitmap("BITMAPS/buttonwkspress.bmp",NULL);
	topbuttonpress[6] = load_bitmap("BITMAPS/buttonfailsrcpress.bmp",NULL);
	topbuttonpress[7] = load_bitmap("BITMAPS/buttonfaildestpress.bmp",NULL);
	topbuttonpress[8] = load_bitmap("BITMAPS/buttonkeypress.bmp",NULL);

	menubutton[0] = load_bitmap("BITMAPS/menurun.bmp",NULL);
	menubuttonover[0] = load_bitmap("BITMAPS/menurunover.bmp",NULL);
	menubutton[1] = load_bitmap("BITMAPS/menuload.bmp",NULL);
	menubuttonover[1] = load_bitmap("BITMAPS/menuloadover.bmp",NULL);
	menubutton[2] = load_bitmap("BITMAPS/menubuild.bmp",NULL);
	menubuttonover[2] = load_bitmap("BITMAPS/menubuildover.bmp",NULL);
	menubutton[3] = load_bitmap("BITMAPS/menuedit.bmp",NULL);
	menubuttonover[3] = load_bitmap("BITMAPS/menueditover.bmp",NULL);
	menubutton[4] = load_bitmap("BITMAPS/menuexit.bmp",NULL);
	menubuttonover[4] = load_bitmap("BITMAPS/menuexitover.bmp",NULL);

	topomenuover[0] = load_bitmap("BITMAPS/topobuildnewpress.bmp",NULL);
	topomenuover[1] = load_bitmap("BITMAPS/topoeditexistingpress.bmp",NULL);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	initialize_topbuttons
// Description:		Initializes the building topology buttons bar
//
///////////////////////////////////////////////////////////////////
void initialize_topobuttons()
{
	int xplace = 0;
	topobutton[0] = load_bitmap("BITMAPS/buttonlogo.bmp",NULL);
	blit(topobutton[0], topbuttons, 0,0,xplace,0,topobutton[0]->w,topobutton[0]->h);
	xplace += topobutton[0]->w;
	
	topobutton[1] = load_bitmap("BITMAPS/buttonplacerouter.bmp",NULL);
	topobuttonover[0] = load_bitmap("BITMAPS/buttonplacerouterover.bmp",NULL);
	blit(topobutton[1], topbuttons, 0,0,xplace,0,topobutton[1]->w,topobutton[1]->h);
	xplace += topobutton[1]->w;

	topobutton[2] = load_bitmap("BITMAPS/buttonplaceedge.bmp",NULL);
	topobuttonover[1] = load_bitmap("BITMAPS/buttonplaceedgeover.bmp",NULL);
	blit(topobutton[2], topbuttons, 0,0,xplace,0,topobutton[2]->w,topobutton[2]->h);
	xplace += topobutton[2]->w;

	topobutton[3] = load_bitmap("BITMAPS/buttonmoverouter.bmp",NULL);
	topobuttonover[2] = load_bitmap("BITMAPS/buttonmoverouterover.bmp",NULL);
	blit(topobutton[3], topbuttons, 0,0,xplace,0,topobutton[3]->w,topobutton[3]->h);
	xplace += topobutton[3]->w;

	topobutton[4] = load_bitmap("BITMAPS/buttoneditrouter.bmp",NULL);
	topobuttonover[3] = load_bitmap("BITMAPS/buttoneditrouterover.bmp",NULL);
	blit(topobutton[4], topbuttons, 0,0,xplace,0,topobutton[4]->w,topobutton[4]->h);
	xplace += topobutton[4]->w;

	topobutton[5] = load_bitmap("BITMAPS/buttoneditedge.bmp",NULL);
	topobuttonover[4] = load_bitmap("BITMAPS/buttoneditedgeover.bmp",NULL);
	blit(topobutton[5], topbuttons, 0,0,xplace,0,topobutton[5]->w,topobutton[5]->h);
	xplace += topobutton[5]->w;

	topobutton[6] = load_bitmap("BITMAPS/buttontopstretch.bmp",NULL);
	stretch_blit(topobutton[6], topbuttons, 0,0,topobutton[6]->w,topobutton[6]->h,xplace,0,SCRNWID-xplace,topobutton[6]->h);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	initialize_graphbuttons
// Description:		Initializes the graph buttons bar
//
///////////////////////////////////////////////////////////////////
void initialize_graphbuttons()
{
	int xplace = 0;
	graphbutton[0] = load_bitmap("BITMAPS/buttonleft.bmp",NULL);
	blit(graphbutton[0], graphbuttons, 0,0,xplace,0,graphbutton[0]->w,graphbutton[0]->h);
	xplace += graphbutton[0]->w;
	graphbutton[1] = load_bitmap("BITMAPS/buttonright.bmp",NULL);
	blit(graphbutton[1], graphbuttons, 0,0,xplace,0,graphbutton[1]->w,graphbutton[1]->h);
	xplace += graphbutton[1]->w;
	graphbutton[2] = load_bitmap("BITMAPS/buttonpointer.bmp",NULL);
	blit(graphbutton[2], graphbuttons, 0,0,xplace,0,graphbutton[2]->w,graphbutton[2]->h);
	xplace += graphbutton[2]->w;
	graphbutton[3] = load_bitmap("BITMAPS/buttonzoomin.bmp",NULL);
	blit(graphbutton[3], graphbuttons, 0,0,xplace,0,graphbutton[3]->w,graphbutton[3]->h);
	xplace += graphbutton[3]->w;
	graphbutton[4] = load_bitmap("BITMAPS/buttonzoomout.bmp",NULL);
	blit(graphbutton[4], graphbuttons, 0,0,xplace,0,graphbutton[4]->w,graphbutton[4]->h);
	xplace += graphbutton[4]->w;
	graphbutton[5] = load_bitmap("BITMAPS/buttonzoomw.bmp",NULL);
	blit(graphbutton[5], graphbuttons, 0,0,xplace,0,graphbutton[5]->w,graphbutton[5]->h);
	xplace += graphbutton[5]->w;
	graphbutton[6] = load_bitmap("BITMAPS/buttonmove.bmp",NULL);
	blit(graphbutton[6], graphbuttons, 0,0,xplace,0,graphbutton[6]->w,graphbutton[6]->h);
	xplace += graphbutton[6]->w;
	graphbutton[7] = load_bitmap("BITMAPS/buttonstretch.bmp",NULL);
	stretch_blit(graphbutton[7], graphbuttons, 0,0,graphbutton[7]->w,graphbutton[7]->h,xplace,0,SCRNWID-xplace,graphbutton[7]->h);

	graphbuttonpress[0] = load_bitmap("BITMAPS/buttonleftpress.bmp",NULL);
	graphbuttonpress[1] = load_bitmap("BITMAPS/buttonrightpress.bmp",NULL);
	graphbuttonpress[2] = load_bitmap("BITMAPS/buttonpointerpress.bmp",NULL);
	graphbuttonpress[3] = load_bitmap("BITMAPS/buttonzoominpress.bmp",NULL);
	graphbuttonpress[4] = load_bitmap("BITMAPS/buttonzoomoutpress.bmp",NULL);
	graphbuttonpress[5] = load_bitmap("BITMAPS/buttonzoomwpress.bmp",NULL);
	graphbuttonpress[6] = load_bitmap("BITMAPS/buttonmovepress.bmp",NULL);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	userBuildTopo
// Description:		Lets the user build a topology
//
///////////////////////////////////////////////////////////////////
void userBuildTopo(int argc, const char* argv[])
{
	numTopoWorkstations = 0;
	numTopoEdges = 0;
	topofunction = 0;//codes: 1 = addrouter, 2 = addedge, 3 = moverouter, 4 = editrouter, 5 = editedge
	initialize_topobuttons();
	clear_to_color(routersbmp,makecol(255,0,255));
	clear_to_color(mainbuf,makecol(0,0,0));
	masked_blit(topbuttons,mainbuf,0,0,0,0,SCRNWID,SCRNHEI);
	stretch_blit(graphbackground,mainbuf,0,0,graphbackground->w,graphbackground->h,graphX1,graphY1,graphX2-graphX1,graphY2-graphY1);
	masked_blit(graphbuttons,mainbuf,0,0,0,graphY1-16,SCRNWID,SCRNHEI);
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);

	bool buildNew = buildTopoMenu();
	topoRefresh();

	if(buildNew)
	{}//no need to initialize routers
	else
	{}//this should initialize routers/edges/wks
	
	while(true)//stay in this function until user clicks menu to get out
	{
		if(mouse_y < topbuttons->h)//mouse is on top buttons
		{
			if(mouse_x < topobutton[0]->w)//on menu
			{
				if(mouse_b & 1)//clicked on menu
				{	menu();
					if(functionactive != -1)
						return;	}
			}
			else if(mouse_x < topobutton[0]->w + topobutton[1]->w)
			{
				buildTopoSetFunction(1);//code: 1 = addrouter
			}
			else if(mouse_x < topobutton[0]->w + topobutton[1]->w * 2)
			{
				buildTopoSetFunction(2);//code: 2 = addedge
			}
			else if(mouse_x < topobutton[0]->w + topobutton[1]->w * 3)
			{
				buildTopoSetFunction(3);//code: 3 = moverouter
			}
			else if(mouse_x < topobutton[0]->w + topobutton[1]->w * 4)
			{
				buildTopoSetFunction(4);//code: 4 = editrouter
			}
			else if(mouse_x < topobutton[0]->w + topobutton[1]->w * 5)
			{
				buildTopoSetFunction(5);//code: 5 = editedge
			}
		}
		else if(mouse_y < graphY1-16)//mouse in topology area
		{
			if(mouse_b & 1)
			{
				switch(topofunction)
				{
					case 1:
						buildtopoplacerouter();
						break;
					case 2:
						buildtopoplaceedge();
						break;
					case 3:
						buildtopomoverouter();
						break;
					case 4:
						buildtopoeditrouter();
						break;
					case 5:
						buildtopoeditedge();
						break;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoMenu
// Description:		Initial menu for building a topology
//
///////////////////////////////////////////////////////////////////
bool buildTopoMenu()
{
	int startX = ceil((double)SCRNWID/2.0 - (double)topomenu->w/2.0);
	int startY = ceil((double)SCRNHEI/2.0 - (double)topomenu->h/2.0);
	bool exit = false;
	int y1 = 23;
	int y2 = 52;
	
	while(true)
	{
		blit(topomenu,mainbuf,0,0,startX,startY,topomenu->w,topomenu->h);
		if(mouse_x > startX && mouse_x < startX + topomenu->w)
		{
			if(mouse_y > (startY + y1) && mouse_y < (startY + y2))
			{
				blit(topomenuover[0],mainbuf,0,0,startX,startY + y1,topomenuover[0]->w,topomenuover[0]->h);
				if(mouse_b & 1)
					return true;
			}
			else if(mouse_y >= (startY + y2) && mouse_y < (startY + topomenu->h))
			{
				blit(topomenuover[1],mainbuf,0,0,startX,startY + y2,topomenuover[1]->w,topomenuover[1]->h);
				if(mouse_b & 1)
					return false;
			}
		}
		masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		rolls over appropriate topobutton and sets topofunction
//					number to corresponding tool (given by tf).
//
///////////////////////////////////////////////////////////////////
void buildTopoSetFunction(int tf)
{
	blit(topbuttons,mainbuf,0,0,0,0,topbuttons->w,topbuttons->h);
	blit(topobuttonover[tf-1],mainbuf,0,0,topobutton[0]->w + topobutton[1]->w * (tf - 1),0,topobuttonover[tf-1]->w,topobuttonover[tf-1]->h);
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
	if(mouse_b & 1)
		topofunction = tf;//codes: 1 = addrouter, 2 = addedge, 3 = moverouter, 4 = editrouter, 5 = editedge
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		rolls over appropriate topobutton and sets topofunction
//					number to corresponding tool (given by tf).
//
///////////////////////////////////////////////////////////////////
void buildtopoplacerouter()
{
	int mosx = mouse_x;
	int mosy = mouse_y;
	bool freespot = true;
	//go through all routers. If clicked outside the radius of all routers, add new router at this location.
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		rx = topoRouters[r]->getXPixel(); //set coordinates of routers
		ry = topoRouters[r]->getYPixel();
		clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
		if(clickdist < topoRouters[r]->getRadius())
		{	freespot = false;	}
	}
	if(freespot)
	{
		Router* r = new Router;
		int sz = topoRouters.size();
		r->setIndex(sz);
		r->setXPercent(ceil(100.0 * ( (float)mosx - (float)bkgrndX )/ ((float)realTopoWidthPx) ) );
		r->setYPercent(ceil(100.0 * ( (float)mosy - (float)bkgrndY )/ ((float)realTopoHeightPx) ) );
		topoRouters.push_back(r);
		topoRouters[sz]->setXYPixels();
		topoRouters[sz]->moveXYPixels(0,topbuttons->h);//initially translate below top button bar
		//for testing workstation expansion
		topoRouters[sz]->setNumWorkstations(1);
		numTopoWorkstations++;
	}
	topoRefresh();//build this painting function
	while(mouse_b & 1)
	{}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		rolls over appropriate topobutton and sets topofunction
//					number to corresponding tool (given by tf).
//
///////////////////////////////////////////////////////////////////
void buildtopoplaceedge()
{
	short int from = -1;
	short int to = -1;
	short int spans = 0;
	int mosx = mouse_x;
	int mosy = mouse_y;
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		rx = topoRouters[r]->getXPixel(); //set coordinates of routers
		ry = topoRouters[r]->getYPixel();
		clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
		if(clickdist < topoRouters[r]->getRadius())
		{	from = r;	}
	}
	if(from == -1)
	{	return;	}
	
	while(mouse_b & 1)
	{}
	while(!(mouse_b & 1))
	{}
	mosx = mouse_x;
	mosy = mouse_y;

	if(mosy < graphY1 && mosy > topbuttons->h)
	{
		for(unsigned short int r = 0; r < topoRouters.size(); ++r)
		{
			rx = topoRouters[r]->getXPixel(); //set coordinates of routers
			ry = topoRouters[r]->getYPixel();
			clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
			if(clickdist < topoRouters[r]->getRadius())
			{	to = r;	}
		}

		if(to != -1 && to != from && topoRouters[from]->getTopoEdgeByDest(to) == 0)
		{
			Edge* e1 = new Edge();
			e1->setSourceIndex(from);
			e1->setDestinationIndex(to);
			e1->initializetopobmps();
			e1->setNumberOfSpans(1);
			topoRouters[from]->addTopoEdge(e1);
			Edge* e2 = new Edge();
			e2->setSourceIndex(to);
			e2->setDestinationIndex(from);
			e2->initializetopobmps();
			e2->setNumberOfSpans(1);
			topoRouters[to]->addTopoEdge(e2);
			numTopoEdges += 2;
			paint_topo_edges(-5);
			topoRefresh();
		}
	}
	while(mouse_b & 1)
	{}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		rolls over appropriate topobutton and sets topofunction
//					number to corresponding tool (given by tf).
//
///////////////////////////////////////////////////////////////////
void buildtopomoverouter()
{
	int chosen = -1;
	int mosx = mouse_x;
	int mosy = mouse_y;
	int diffx = 0;
	int diffy = 0;
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		rx = topoRouters[r]->getXPixel(); //set coordinates of routers
		ry = topoRouters[r]->getYPixel();
		clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
		if(clickdist < topoRouters[r]->getRadius())
		{	chosen = r;	}
	}
	if(chosen == -1)
	{	return;	}
	while(mouse_b & 1)
	{
		clear_to_color(edgesbmp,makecol(255,0,255));
		topoRouters[chosen]->moveXYPixels(diffx,diffy);	//remember, because we only change the pixels, we must recalculate
			//x,yPercents before saving this topology
		for(unsigned short int adj = 0; adj < topoRouters.size(); adj++)
		{
			topoRouters[adj]->refreshedgebmps(false);
		}
		
		paint_topo_edges(-5);
		topoRefresh();

		diffy = mouse_y - mosy;
		mosy = mouse_y;
		diffx = mouse_x - mosx;
		mosx = mouse_x;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		lets user click on router to edit workstation number
//					and 
//
///////////////////////////////////////////////////////////////////
void buildtopoeditrouter()
{
	int chosen = -1;//TODO: These first fifteen or so lines of code are reused in about 3 such functions.
	int mosx = mouse_x;//As one function, they would only need to return a single integer (index)
	int mosy = mouse_y;
	int diffx = 0;
	int diffy = 0;
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		rx = topoRouters[r]->getXPixel(); //set coordinates of routers
		ry = topoRouters[r]->getYPixel();
		clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
		if(clickdist < topoRouters[r]->getRadius())
		{	chosen = r;	}
	}
	if(chosen == -1)
	{	return;	}

	//COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY for the most part
	clear_to_color(popup,makecol(255,0,255));
	int startX = topoRouters[chosen]->getXPixel() + 10;
	int startY = topoRouters[chosen]->getYPixel() - 50;
	int numtopaint;
	int black = makecol(0,0,0);

	if((startX + 140) > SCRNWID)
		startX = topoRouters[chosen]->getXPixel() - 150;
	if((startY + 100) > graphY1)
		startY = graphY1 - 100;
	if((startY < 15))
		startY = 15;
	masked_blit(editrouterinfo,popup,0,0,startX,startY,editrouterinfo->w,editrouterinfo->h);
	numtopaint = chosen;
	textprintf_ex(popup,font,startX+65,startY+10,black,-1,"%d", numtopaint);
	textprintf_ex(popup,font,startX+15,startY+35,black,-1,"\"%s\"", topoRouters[chosen]->getName());
	numtopaint = topoRouters[chosen]->getNumWorkstations();
	textprintf_right_ex(popup,font,startX + 129,startY+57, black,-1,"%d", numtopaint);
	numtopaint = 100.0 *  float(topoRouters[chosen]->getNumWorkstations())/float(numTopoWorkstations);
	textprintf_right_ex(popup,font,startX + 129,startY+72, black,-1,"%d", numtopaint);
	numtopaint = topoRouters[chosen]->getNumberOfEdges();
	textprintf_right_ex(popup,font,startX + 129,startY+90, black,-1,"%d", numtopaint);
	topoRefresh();
	//COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY
	//paint editrouterinfo to mainbuf,then to screen
	while(mouse_b & 1)
	{}
	while(!(mouse_b & 1))//wait for another click
	{}
	if(mouse_x > startX && mouse_x < startX + editrouterinfo->w && mouse_y > startY && mouse_y < startY + editrouterinfo->h)
	{
		if(mouse_y > startY + 25 && mouse_y < startY + 47)
		{	topoRouters[chosen]->setName(edit_inputter(false,startX,startY+25,editrouterinfo->w,47-25));	}
		else if(mouse_y > startY + 47 && mouse_y < startY + 68)
		{
			int numw = topoRouters[chosen]->getNumWorkstations();
			topoRouters[chosen]->setNumWorkstations(edit_inputint(false,topoRouters[chosen]->getNumWorkstations(),
				startX+90,startY+47,editrouterinfo->w-94,68-47));
			numTopoWorkstations -= numw - topoRouters[chosen]->getNumWorkstations(); 
		}
	}
	else
	{
		clear_to_color(popup,makecol(255,0,255));
		topoRefresh();
	}
	clear_to_color(popup,makecol(255,0,255));
	topoRefresh();
	//wait for a new mouse click
	//did user click in editrouterinfo area?
		//where?
			//well, wherever it is, let user type in appropriate thing (shown on popup)
			//keep refreshing popup
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	buildTopoSetFunction(int tf)
// Description:		rolls over appropriate topobutton and sets topofunction
//					number to corresponding tool (given by tf).
//
///////////////////////////////////////////////////////////////////
void buildtopoeditedge()
{
	short int from = -1;//this is copied too. Again, TODO: we should replace with a return index function
	short int to = -1;
	short int spans = 0;
	int mosx = mouse_x;
	int mosy = mouse_y;
	for(unsigned short int r = 0; r < topoRouters.size(); ++r)
	{
		rx = topoRouters[r]->getXPixel(); //set coordinates of routers
		ry = topoRouters[r]->getYPixel();
		clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
		if(clickdist < topoRouters[r]->getRadius())
		{	from = r;	}
	}
	if(from == -1)
	{	return;	}
	
	while(mouse_b & 1)
	{}
	while(!(mouse_b & 1))
	{}
	mosx = mouse_x;
	mosy = mouse_y;

	if(mosy < graphY1 && mosy > topbuttons->h)
	{
		for(unsigned short int r = 0; r < topoRouters.size(); ++r)
		{
			rx = topoRouters[r]->getXPixel(); //set coordinates of routers
			ry = topoRouters[r]->getYPixel();
			clickdist = sqrt( ( mosx - rx )*( mosx - rx ) + ( mosy - ry )*( mosy - ry) );//distance formula
			if(clickdist < topoRouters[r]->getRadius())
			{	to = r;	}
		}

		edge = topoRouters[from]->getTopoEdgeByDest(to);
		if(edge != 0)
		{
			clear_to_color(popup,makecol(255,0,255));
			int startX = ceil(float(topoRouters[edge->getSourceIndex()]->getXPixel() 
				+ topoRouters[edge->getDestinationIndex()]->getXPixel())/2.0 - float(editedgeinfo->w)/2.0);
			int startY = ceil(float(topoRouters[edge->getSourceIndex()]->getYPixel() 
				+ topoRouters[edge->getDestinationIndex()]->getYPixel())/2.0 - float(editedgeinfo->h)/2.0);
			int numtopaint;
			int black = makecol(0,0,0);

			if((startX + editedgeinfo->w) > SCRNWID)
				startX = SCRNWID - editedgeinfo->w;
			if(startX < 0)
				startX = 0;
			if((startY + 50) > graphY1)
				startY = graphY1 - 50;
			if((startY < 15))
				startY = 15;

			masked_blit(editedgeinfo,popup,0,0,startX,startY,editedgeinfo->w,editedgeinfo->h);
			numtopaint = edge->getSourceIndex();
			textprintf_centre_ex(popup,font,startX+50,startY+10,black,-1,"%d", numtopaint);
			numtopaint = edge->getDestinationIndex();
			textprintf_centre_ex(popup,font,startX+85,startY+10,black,-1,"%d",numtopaint);
			numtopaint = edge->getNumberOfSpans();
			textprintf_right_ex(popup,font,startX+editedgeinfo->w - 7,startY+32,black,-1,"%3d",numtopaint);
			topoRefresh();
			//COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY COPY
			//paint editrouterinfo to mainbuf,then to screen
			while(mouse_b & 1)
			{}
			while(!(mouse_b & 1))//wait for another click
			{}
			if(mouse_x > startX && mouse_x < startX + editedgeinfo->w && mouse_y > startY && mouse_y < startY + editedgeinfo->h)
			{
				if(mouse_y > startY + 25 && mouse_y < startY + 46)
				{	int num_spans = edit_inputint(false,edge->getNumberOfSpans(),
						startX+60,startY+25,editedgeinfo->w-64,46-25);
					edge->setNumberOfSpans(num_spans);
					edge = edge = topoRouters[to]->getTopoEdgeByDest(from);
					edge->setNumberOfSpans(num_spans);
				}
				else if(mouse_y > startY + 47 && mouse_y < startY + editedgeinfo->h)
				{
					int sor = edge->getSourceIndex();
					int des = edge->getDestinationIndex();
					float x1 = (float)topoRouters[sor]->getXPixel();
					float x2 = (float)topoRouters[des]->getXPixel();
					float y1 = (float)topoRouters[sor]->getYPixel();
					float y2 = (float)topoRouters[des]->getYPixel();
					int px = ceil(sqrt( (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1) ));
					scaleAllEdgesTo(edge->getNumberOfSpans(), px);
				}
			}
			else
			{
				clear_to_color(popup,makecol(255,0,255));
				topoRefresh();
			}
			clear_to_color(popup,makecol(255,0,255));
			topoRefresh();
		}
	}
	return;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	scaleAllEdgesTo
// Description:		sets spans of all edges proportional to the spans/
//					pixel ratio represented by input parameters
//
///////////////////////////////////////////////////////////////////
void scaleAllEdgesTo(int spns, int px)
{

	for(int r = 0; r < topoRouters.size(); ++r)
	{
		topoRouters[r]->scaleEdgesTo(spns,px);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	edit_inputter
// Description:		draws grey square and takes user input for editing
//					router or edge info. returns char array
//
///////////////////////////////////////////////////////////////////
const char* edit_inputter(bool alignleft,int xpx,int ypx,int wid,int hei)
{
	int place = 0;
	int black = makecol(0,0,0);
	int newkey;
	char ASCII = 0;
	int scancode; 
	char getter[20] = {' '};
	sprintf(getter,"");
	rectfill(screen, xpx, ypx, xpx+wid, ypx + hei, makecol(175,175,175));
	while(mouse_b & 1)
	{}
	while(!key[KEY_ENTER] && !(mouse_b & 1)){//loop puts name of directory together.
		if(keypressed())
		 {
			rectfill(screen, xpx, ypx, xpx+wid, ypx + hei, makecol(175,175,175));
			newkey = ureadkey(&scancode);
			ASCII = scancode_to_ascii(scancode);
			if(place < 19)
			{
				getter[place] = (char)ASCII;
				place++;
				if(alignleft)
					textprintf_right_ex(screen,font,xpx+50,ypx+5,black,-1,"%s",getter);
				else
					textprintf_right_ex(screen,font,xpx+wid-10,ypx+5,black,-1,"%s",getter);
			}
			else if(key[KEY_BACKSPACE] || key[KEY_DEL])
			{
				if(place > 0){
					
					getter[place-1] = ' ';

					if(alignleft)
						textprintf_right_ex(screen,font,xpx+50,ypx+5,black,-1,"%s",getter);
					else
						textprintf_right_ex(screen,font,xpx+wid-10,ypx+5,black,-1,"%s",getter);

					place--;	}
				else
					place = 0;
			}
		}
	}
	return(getter);
}


int edit_inputint(bool alignleft,int edit, int xpx,int ypx,int wid,int hei)
{
	int num = edit;
	int black = makecol(0,0,0);
	rectfill(screen, xpx, ypx, xpx+wid, ypx + hei, makecol(175,175,175));

	if(alignleft)
		textprintf_ex(screen,font,xpx+50,ypx+7,black,-1,"%d",num);
	else
		textprintf_right_ex(screen,font,xpx+wid-10,ypx+7,black,-1,"%d",num);
	while(mouse_b & 1)
	{}
	while(!key[KEY_ENTER] && !(mouse_b & 1))
	{//loop lets user push up/down arrows to increase value of edit
		if(key[KEY_UP] || key[KEY_RIGHT])
		{
			rectfill(screen, xpx, ypx, xpx+wid, ypx + hei, makecol(175,175,175));
			num++;
			if(alignleft)
				textprintf_ex(screen,font,xpx+50,ypx+7,black,-1,"%d",num);
			else
				textprintf_right_ex(screen,font,xpx+wid-10,ypx+7,black,-1,"%d",num);
			while(key[KEY_UP] || key[KEY_RIGHT])
			{}
		}
		else if(key[KEY_DOWN] || key[KEY_LEFT])
		{
			rectfill(screen, xpx, ypx, xpx+wid, ypx + hei, makecol(175,175,175));
			num--;
			if(num < 0)
				num = 0;
			if(alignleft)
				textprintf_ex(screen,font,xpx+50,ypx+7,black,-1,"%d",num);
			else
				textprintf_right_ex(screen,font,xpx+wid-10,ypx+7,black,-1,"%d",num);
			while(key[KEY_DOWN] || key[KEY_LEFT])
			{}
		}
	}
	return(num);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	userLoadPrevSim
// Description:		Load a previous saved simulation and starts
//					explore history.
//
///////////////////////////////////////////////////////////////////
void userLoadPrevSim(int argc, const char* argv[])
{
	FILE * fileCheck;				//for checking to see if such a file exists
	ifstream inputFile;				//used for all file input in this function
	ifstream inputFile2;
	ifstream inputFile3;
	ofstream outputFile;			//used for correcting index problems
	bool folderselected = false;	//says whether directory has been successfully appended
	int levels = 0;					//currently set to go down two levels: index.idx -> folder/index.idx -> folder/selection.thd
	int black = makecol(0,0,0);		//define color: black
	int green = makecol(0,255,0);	//define color: green
	white = makecol(255,255,255);

	string indexName = "index.idx"; //string used to name whatever folder/file we currently want
	string indexName2;
	string directory;				//string used to hold names of directories taken from input file.
	char directory1[25];				//used to append second directory to first
	char buffer[50];
	char taker[30];
	string taker2;
	vector <string> directories;	//vector to hold names of things in currently viewed index.idx file
	int result;						//used to keep track of the length of directories
	int mb, mx, my, spcng = 15, printHdSt = 100;	//mouse variables; spacing, printHdSt, and printHd for painting list
	int printHd;
	int num_r, num_e, num_h,loadusage;
	unsigned int connTaker;
	double connDouble;
	
	while(levels < 2)
	{
		rectfill(screen, 0, 0, SCRNWID, SCRNHEI, black);
		rectfill(screen, SCRNWID - 50, SCRNHEI - 50, SCRNWID, SCRNHEI, makecol(255,0,0));
		printHd = printHdSt;
		result = 0;

		if(levels == 0)
		{
			inputFile.open(indexName.c_str(), std::ios::in);
			//populate directories
			//clear_to_color(screen,makecol(214,214,214));
			rectfill(screen, 75, 50, 400, SCRNHEI-100,makecol(214,214,214));
			while(inputFile.getline(buffer,40))//Before bed... having trouble understanding why it can't read this file the second time around.
			{
				directory = strtok(buffer,"|");
				directories.push_back(directory);
				textprintf_ex(screen,font,100,printHd,black,-1,"%s",directories.at(result).c_str());
				printHd += spcng; //go down for next 
				result++;
				directory.clear();
			}
			inputFile.close();
		}
		else if(levels == 1)
		{
			inputFile2.open(indexName2.c_str(), std::ios::in);
			//clear_to_color(screen,makecol(214,214,214));
			rectfill(screen, 75, 50, 400, SCRNHEI-100,makecol(214,214,214));
			while(inputFile2.getline(buffer,40))
			{
				directory = strtok(buffer,"|");
				directories.push_back(directory);
				textprintf_ex(screen,font,100,printHd,black,-1,"%s",directories.at(result).c_str());
				printHd += spcng; //go down for next 
				result++;
				directory.clear();
			}
			inputFile2.close();
		}
		
		rest(300);

		//textprintf_ex(screen,font,400,15,black,-1,"result=%d",result);
		folderselected = false;

		while(!folderselected)
		{
			mb = mouse_b & 1;
			my = mouse_y;
			mx = mouse_x;
			if(my > printHdSt && my < printHd)//if on one of the directory options.
			{
				result = (my - printHdSt) / spcng;
				for(unsigned int p = 0; p < directories.size(); p++)
				{
					if(p != result)
						textprintf_ex(screen,font,100,p*spcng + printHdSt,black,makecol(214,214,214),"%s",directories.at(p).c_str());
					else
						textprintf_ex(screen,font,100,result*spcng + printHdSt,black,makecol(175,175,175),"%s",directories.at(p).c_str());
				}
				if(mb){//mouse clicked, see if directory is legit.	
					directory = directories.at(result).c_str();
					if(levels == 0){
						sprintf(buffer,"%s/%s",directory.c_str(),indexName.c_str());
					}
					else if(levels == 1){
						sprintf(buffer,"%s/%s.thd",directory1,directory.c_str());
					}
					fileCheck = fopen( buffer, "r" );
					if ( fileCheck == NULL)
					{
						textprintf_ex(screen,font,100,result*spcng + printHdSt,black,green,"Directory Not Found");
						directories.erase(directories.begin()+ result);
						printHd -= spcng;
						//clear_to_color(screen,makecol(214,214,214));
						rectfill(screen, 75, 50, 400, SCRNHEI-100,makecol(214,214,214));
						outputFile.open(indexName.c_str(), std::ios::out);
						for(unsigned int d = 0; d < directories.size(); d++){
							textprintf_ex(screen,font,100,d*spcng + printHdSt,black,-1,"%s",directories.at(d).c_str());
							outputFile << directories.at(d).c_str() << "|\n";
						}
						outputFile.close();
					}
					else //directory is cool
					{
						//textprintf_ex(screen,font,100,result*spcng + printHdSt,white,black,"Found");
						rest(200);
						fclose (fileCheck);
						if(levels == 0) //sets it up to open the right index file next time around
						{	//directory1 = directory; //bad
							sprintf(directory1, directory.c_str());
							sprintf(buffer, "%s/%s",directory1,indexName.c_str());
							directory.clear();
							indexName2 = buffer;
						}	
						folderselected = true;
					}	
				}
			}
		}	
		result = 0;
		directories.clear();
		levels++;
	}

	Thread* thread = new Thread(0,argc,argv,true);
	
	//go through selected file, set up threadZero and explore!
	inputFile3.open(buffer, std::ios::in);

	num_r = threadZero->getNumberOfRouters();
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");			
	threadZero->setTopology(taker2.c_str());		//topology
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setRouting(taker2.c_str());			//routing alg
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setWavelength(taker2.c_str());			//wavelength alg
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setProbing(taker2.c_str());				//probing alg
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setQualityAware(atoi(taker2.c_str()));	//qualityAware?
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setOverallBlocking(atof(taker2.c_str()));//overallBlocking
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setCollisions(atof(taker2.c_str()));	//collisions
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setBadQuality(atof(taker2.c_str()));	//bad quality
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setNonResource(atof(taker2.c_str()));	//non resource
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgProbesPerRequest(atof(taker2.c_str()));//avg probes per request
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgRequestDelayTime(atof(taker2.c_str()));	//setAvgRequestDelayTime
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgConnHopCount(atof(taker2.c_str()));	//setAvgConnHopCount
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgConnSpanCount(atof(taker2.c_str()));	//setAvgConnSpanCount
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgASEnoise(atof(taker2.c_str()));		//avg ASE noise
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgFWMnoise(atof(taker2.c_str()));		//avg FWM noise
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setAvgXPMnoise(atof(taker2.c_str()));		//avg XPM noise

	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setNumberOfWavelengths(atoi(taker2.c_str()));//available wavelengths
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	num_h = atoi(taker2.c_str());
	threadZero->setMaxMaxUsage(threadZero->getNumberOfWavelengths());
	inputFile3.getline(taker,30);
	taker2 = strtok(taker,"|");
	threadZero->setCurrentActiveWorkstations(atoi(taker2.c_str()));
	textprintf_ex(screen,font,300,225,white,black,"Loading Edge History");
	for(int r = 0; r < num_r; r++){
		if(inputFile3.getline(taker,30))
		{
			taker2.clear();
			taker2 = strtok(taker,"|");
			loadusage = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->setNumWorkstations(loadusage);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connTaker = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->setConnAttemptsFrom(connTaker);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connTaker = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->setConnAttemptsTo(connTaker);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connTaker = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->setConnSuccessesFrom(connTaker);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connTaker = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->setConnSuccessesTo(connTaker);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connDouble = atof(taker2.c_str());
			threadZero->getRouterAt(r)->addToAvgQTo(connDouble);

			inputFile3.getline(taker,30);
			taker2 = strtok(taker,"|");
			connDouble = atof(taker2.c_str());
			threadZero->getRouterAt(r)->addToAvgQFrom(connDouble);
		}
		num_e = threadZero->getRouterAt(r)->getNumberOfEdges();
		textprintf_ex(screen,font,400,240,white,black,"Router = %d", r);
		for(int e = 0; e < num_e; e++){
			for(int h = 0; h < num_h; h++)
			{
				if(inputFile3.getline(taker,30))
				{
					taker2.clear();
					taker2 = strtok(taker,"|");
					loadusage = atoi(taker2.c_str());
					threadZero->getRouterAt(r)->getEdgeByIndex(e)->addUsage(loadusage);
				}
			}
			taker2.clear();
			if(inputFile3.getline(taker,30)){
			taker2 = strtok(taker,"|");
			loadusage = atoi(taker2.c_str());
			threadZero->getRouterAt(r)->getEdgeByIndex(e)->setMaxUsage(loadusage);
			}
		}
	}
	textprintf_ex(screen,font,300,240,white,black,"Done");

	inputFile3.close();
	explore_history();

	delete threadZero;
	delete[] threads;

	return;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	userEditConfigs
// Description:		Allows the user to edit the configuration files
//
///////////////////////////////////////////////////////////////////
void userEditConfigs()
{
	int mb, mx, my;
	int green = makecol(0,255,0);
	rectfill(screen, 0, 0, SCRNWID, SCRNHEI, color);
	rectfill(screen, SCRNWID - 50, SCRNHEI - 50, SCRNWID, SCRNHEI, makecol(255,0,0));
	textprintf_ex(screen,font,200,300,green,-1,"In Editing Configuration.");
	rest(300);
	while(true)
	{	mb = mouse_b & 1;
		my = mouse_y;
		mx = mouse_x;
		if(mb && mx > SCRNWID - 50 && my > SCRNHEI - 50)
		{	return;	}
		else if(mb)
			return;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	switcher
// Description:		Controls which of the 4 main functions
//					the user is using.
//
///////////////////////////////////////////////////////////////////
void switcher( int argc, const char* argv[] )
{
	functionactive = -1;
	clear_to_color(mainbuf,makecol(0,0,0));
	blit(topbuttons,mainbuf,0,0,0,0,topbuttons->w,topbuttons->h);
	masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
	while(functionactive == -1)
	{
		my = mouse_y;
		mx = mouse_x;
		if((mouse_b & 1) && mx < 38 && my < 31)
		{
			menu();
		}
	}
	while(functionactive != 0)
	{
		if(functionactive == 1)
		{	userRunNewSim(argc,argv);	}
		else if(functionactive == 2)
		{	userLoadPrevSim(argc,argv);	}
		else if(functionactive == 3)
		{	userBuildTopo(argc,argv);	}
		else if(functionactive == 4)
		{	userEditConfigs();	}
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	menu
// Description:		Sets value of fuctionactive to set which function
//					is active.
//
///////////////////////////////////////////////////////////////////
void menu()
{
	BITMAP* menu = create_bitmap(menubutton[0]->w,5 * menubutton[0]->h);
	int mx, my, mb;
	bool esc = false;
	functionactive = -1;

	while(mouse_b & 1)
	{}

	while(!esc){
		blit(menubutton[0], menu,0,0,0,0,menubutton[0]->w,menubutton[0]->h);
		blit(menubutton[1], menu,0,0,0,menubutton[0]->h,menu->w,menu->h);
		blit(menubutton[2], menu,0,0,0,2 * menubutton[0]->h,menu->w,menu->h);
		blit(menubutton[3], menu,0,0,0,3 * menubutton[0]->h,menu->w,menu->h);
		blit(menubutton[4], menu,0,0,0,4 * menubutton[0]->h,menu->w,menu->h);
		mx = mouse_x;
		my = mouse_y;
		mb = (mouse_b & 1);
		clear_to_color(popup,makecol(255,0,255));//make popup invisible pink
		if(mx < 100 && my > topbuttons->h && my < topbuttons->h + 5*menubutton[0]->h)
		{
			if(mb)
				esc = true;
			if(my < topbuttons->h + menubutton[0]->h)//blit(menu,mainbuf,0,0,0,topbuttons->h,menu->w,menu->h);
			{	blit(menubuttonover[0],menu,0,0,0,0,menubutton[0]->w,menubutton[0]->h);
				if(mb)
				{	functionactive = 1;	}
			}
			else if(my < topbuttons->h + 2*menubutton[0]->h)
			{	blit(menubuttonover[1],menu,0,0,0,menubutton[1]->h,menubutton[1]->w,menubutton[1]->h);
				if(mb)
				{	functionactive = 2; }
			}
			else if(my < topbuttons->h + 3*menubutton[0]->h)
			{	blit(menubuttonover[2],menu,0,0,0,2* menubutton[2]->h,menubutton[2]->w,menubutton[2]->h);
				if(mb)
				{	functionactive = 3;	}
			}
			else if(my < topbuttons->h + 4*menubutton[0]->h)
			{	blit(menubuttonover[3],menu,0,0,0,3 * menubutton[3]->h,menubutton[3]->w,menubutton[3]->h);
				if(mb)
				{	functionactive = 4;	}
			}
			else if(my < topbuttons->h + 5*menubutton[0]->h)
			{	blit(menubuttonover[4],menu,0,0,0,4 * menubutton[4]->h,menubutton[4]->w,menubutton[4]->h);
				if(mb)
				{	functionactive = 0;	}
			}
		}
		else if(mb && (mx > menu->w || my > topbuttons->h + menu->h))
		{
			clear_to_color(menu,makecol(255,0,255));
			functionactive = -1;
			esc = true;
		}
		blit(topbuttons,mainbuf,0,0,0,0,topbuttons->w,topbuttons->h);
		blit(menu,mainbuf,0,0,0,topbuttons->h,SCRNWID,SCRNHEI);
		masked_blit(mainbuf,screen,0,0,0,0,SCRNWID,SCRNHEI);
	}
	destroy_bitmap(menu);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	userRunNewSim
// Description:		Runs a new simulation from the argc and argv
//					inputs.
//
///////////////////////////////////////////////////////////////////
void userRunNewSim( int argc, const char* argv[] )
{
	ofstream indexer;
	int place = 0;
	int newkey;
	char ASCII = key[KEY_A];
	char scancode; 
	string str;
	for(int i = 0; i < 25; i++){
		foldName[i] = ASCII;
	}
	sprintf(foldName,"OUTPUT/");
	place = 7;
	rectfill(screen, 0, 0, SCRNWID, SCRNHEI, color);
	textprintf_ex(screen,font,20,15,color2,color,"Choose a name for the new simulation:");

	while(!key[KEY_ENTER]){//loop puts name of directory together.
		if(keypressed())
		 {
			newkey   = readkey();
			ASCII    = newkey;// & 0xff;
			scancode = newkey >> 8;

			if(ASCII >= 32 && ASCII <= 126 && place < 24)
			{
				foldName[place] = ASCII;
				place++;
				textprintf_ex(screen,font,20,25,color2,color,"%s",foldName);
			}
			else if(key[KEY_BACKSPACE] || key[KEY_DEL])
			{
				if(place > 0){
					
					foldName[place-1] = ' ';
					textprintf_ex(screen,font,20,25,color2,color,"%s",foldName);
					place--;	}
				else
					place = 0;
			}
			rest(50);
		}
	}
	
	sprintf(folder,"%s",foldName);
	int fldcreate = mkdir(folder);
	if(fldcreate != 0)
	{
		textprintf_ex(screen,font,20,35,color2,color,"Impossible. Returning to Menu."); 
		rest(3000);
		return;
	}
	else
	{	indexer.open("index.idx", std::ios::out | std::ios::app | std::ios::binary);
		indexer << foldName << "|\n";
		indexer.close();
	}

	runSimulation(argc,argv);

	return;//go back to menu when done

}

#endif
