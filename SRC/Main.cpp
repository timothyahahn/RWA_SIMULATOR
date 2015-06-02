// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Main.cpp
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the implementation of main(), whose only
//					purpose is to pass the command line arguements to the controller.
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  05/20/2009	v1.0	Initial Version.
//
// ____________________________________________________________________________

#include "ErrorCodes.h"
#include "Thread.h"

#include "nonlinear.h"
#include "pthread.h"

#include <iostream>
#include <vector>

#ifdef RUN_GUI

#include "AllegroWrapper.h"

extern void close_button_handler();
extern void flash();
extern void explore_history();

extern void switcher( int argc, const char* argv[] );

extern int color,color2,white,explore_time;

BITMAP *buffer;
BITMAP *pointer;
BITMAP *graph;
BITMAP *graphbuttons;
BITMAP *topbuttons;
BITMAP *edgespans;
BITMAP *routersbmp;
BITMAP *routerinfo;
BITMAP *editrouterinfo;
BITMAP *editedgeinfo;
BITMAP *edgesbmp;
BITMAP *mainbuf;
BITMAP *progbarbmp;
BITMAP *routerpic;
BITMAP *graphbackground;
BITMAP *topobackground;
BITMAP *topomenu;
BITMAP *popup;
BITMAP *detailinfo;

BITMAP *colorkey;

char foldName[25];
char folder[50];

#define SCRNWID 1300
#define SCRNHEI 700
#endif

using std::vector;

//Hate to make a global instance of the threads, but the other classes
//need access to the threads information. They can't just include a 
//pointer to the class due to a circular reference.
Thread* threadZero;
Thread** threads;
unsigned short int threadCount;

vector<AlgorithmToRun*> algParams;

void *runThread(void* n);
void runSimulation(int argc, const char* argv[]);

pthread_mutex_t ScheduleMutex;

int main( int argc, const char* argv[] )
{
	if(argc != 7)
	{
		std::cout << "Usage: " << argv[0] << " <Topology> <Wavelengths> <Random Seed> <Thread Count> <Iteration Count> <Probe Count>" << std::endl;
		return ERROR_INVALID_PARAMETERS;
	}

#ifdef RUN_GUI
	allegro_init();
	set_close_button_callback(close_button_handler);
	set_color_depth(16);

	install_keyboard();
	install_mouse();
	install_timer();
	set_window_title("RAPTOR (Route Assignment Program for Transparent Optical Routes)");
	graph = create_bitmap(SCRNWID,SCRNHEI);
	graphbuttons = create_bitmap(SCRNWID,16);
	topbuttons = create_bitmap(SCRNWID,31);
	buffer = create_bitmap(SCRNWID,SCRNHEI);
	mainbuf = create_bitmap(SCRNWID,SCRNHEI);
	routersbmp = create_bitmap(SCRNWID,SCRNHEI);
	edgesbmp = create_bitmap(SCRNWID,SCRNHEI);
	edgespans = create_bitmap(SCRNWID,SCRNHEI);
	popup = create_bitmap(SCRNWID,SCRNHEI);
	graphbackground = load_bitmap("BITMAPS/graphbackground.bmp",NULL);
	topobackground = load_bitmap("BITMAPS/topobackground2.bmp",NULL);
	progbarbmp = load_bitmap("BITMAPS/progressbar.bmp",NULL);
	routerinfo = load_bitmap("BITMAPS/routerinfo.bmp",NULL);
	editrouterinfo = load_bitmap("BITMAPS/editrouterinfo.bmp",NULL);
	editedgeinfo = load_bitmap("BITMAPS/editedgeinfo.bmp",NULL);
	detailinfo = load_bitmap("BITMAPS/configurationinfo.bmp",NULL);
	colorkey = load_bitmap("BITMAPS/colorkey.bmp",NULL);
	pointer = load_bitmap("BITMAPS/pointer2.bmp",NULL);
	topomenu = load_bitmap("BITMAPS/topomenu.bmp",NULL);

	flash();

	set_gfx_mode(GFX_AUTODETECT_WINDOWED,SCRNWID,SCRNHEI,0,0);

	color = makecol(0,0,0);
	color2 = makecol(0,255,0);
	white = makecol(255,255,255);
	set_mouse_sprite(pointer);
	set_mouse_sprite_focus(15,15);
	show_mouse(screen);
	set_display_switch_mode(SWITCH_BACKGROUND);

	bool doMenu = true;
	//flash();
	switcher(argc, argv);

	set_mouse_sprite(NULL);
	destroy_bitmap(buffer);
	destroy_bitmap(graph);
	destroy_bitmap(pointer);
	destroy_bitmap(topobackground);
	destroy_bitmap(popup);
	destroy_bitmap(routerinfo);
	allegro_exit();
#else
	runSimulation(argc,argv);
#endif

	return 0;
}

#ifdef RUN_GUI
END_OF_MAIN()
#endif

void runSimulation(int argc, const char* argv[])
{
	const char *opt1 = "-nojvm";
	const char *opt2 = "-nojit";
	const char **pStrings = new const char *[2];

	pStrings[0] = opt1;
	pStrings[1] = opt2;

	mclInitializeApplication(pStrings,2);
	nonlinearInitialize();

	int* threadZeroReturn = 0;
	int runCount = 0;

	while(threadZeroReturn == 0 || *threadZeroReturn == MORE_SIMULATIONS)
	{
		delete threadZeroReturn;

		vector<pthread_t*> pThreads;

		threadCount = atoi(argv[4]);

		threads = new Thread*[threadCount];

		unsigned short int iterationCount = atoi(argv[5]);

#ifdef RUN_GUI
	rectfill(screen, 0, 0, SCREEN_W, 40, color);
	textprintf_ex(screen,font,20,15,color2,color,"Building XPM Database, please wait..."); 
#endif

		Thread* thread = new Thread(0,argc,argv,false,runCount);

		threadZero->initResourceManager();

#ifdef RUN_GUI
		rectfill(screen, 0, 0, SCREEN_W, 40, color);
#endif

		pthread_mutex_init(&ScheduleMutex,NULL);

		if(threadCount > algParams.size())
			threadCount = static_cast<unsigned short int>(algParams.size());

		for(unsigned short int t = 1; t < threadCount; ++t)
		{
			Thread* thread = new Thread(t,argc,argv,false,runCount);
			pThreads.push_back(new pthread_t);

			thread->initResourceManager();
		}

		char buffer[25];
		sprintf(buffer,"Created %d threads.\n",threadCount);
		threadZero->recordEvent(buffer,true,0);

		for(unsigned short int t = 1; t < threadCount; ++t)
		{
			pthread_create(pThreads[t-1],NULL,runThread,new unsigned short int(t));
		}

		threadZeroReturn = static_cast<int*>(runThread(new unsigned short int(0)));

		for(unsigned short int t = 1; t < threadCount; ++t)
		{
			pthread_join(*pThreads[t-1],NULL);

#ifdef RUN_GUI
		textprintf_ex(screen,font,20,500,makecol(0,255,0),makecol(0,0,0),"Thread %5d",t-1);
#endif
		}

		for(unsigned short int t = 0; t < threadCount; ++t)
		{
			delete threads[t];

			if(t != 0)
				delete pThreads[t-1];
		}

		delete[] threads;

		pThreads.clear();

		pthread_mutex_destroy(&ScheduleMutex);

		++runCount;
	}

	delete threadZeroReturn;

	nonlinearTerminate();
	mclTerminateApplication();

	delete[] pStrings;

#ifdef RUN_GUI

	explore_time = 0;
	while(!key[KEY_ESC])
	{
	}
#endif
}

void *runThread(void* n)
{
	unsigned short int* t_id = static_cast<unsigned short int *>(n);

	int *retVal = new int;

	while(algParams.size() > 0)
	{
		pthread_mutex_lock(&ScheduleMutex);

		AlgorithmToRun* alg = algParams.back();
		algParams.pop_back();

		pthread_mutex_unlock(&ScheduleMutex);

		*retVal = threads[*t_id]->runThread(alg);

#ifdef RUN_GUI
		textprintf_ex(screen,font,20,SCREEN_H-30,color2,color,"%s",folder); 
		threads[*t_id]->saveThread(folder);
#endif
	}

	delete t_id;

	return retVal;
}
