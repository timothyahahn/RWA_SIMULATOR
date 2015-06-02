// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      Thread.cpp
//  Author:         Timothy Hahn, Montana State University
//  Project:        RWASimulator
//
//  Description:    The file contains the implementation of the controller class.
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

#include "Thread.h"

#ifdef RUN_GUI

#include "AllegroWrapper.h"
extern BITMAP *mainbuf;
extern BITMAP *progbarbmp;
extern BITMAP *detailinfo;
extern BITMAP *popup;

#endif

extern Thread* threadZero;
extern Thread** threads;
extern unsigned short int threadCount;

extern vector<AlgorithmToRun*> algParams;

extern char* itoa( int value, char* result, int base );

///////////////////////////////////////////////////////////////////
//
// Function Name:	Thread
// Description:		Default constructor with no arguements, terminates
//					the program as command line arguments are
//					required.
//
///////////////////////////////////////////////////////////////////
Thread::Thread()
{
	threadZero->recordEvent(string("Unable to initialize the controller without command line arguements.\n"),true,controllerIndex);
	exit(ERROR_THREAD_INIT);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	Thread
// Description:		Constructor that takes the command line arguements
//					and initializes the controller.
//
///////////////////////////////////////////////////////////////////
Thread::Thread(int ci, int argc, const char* argv[], bool isLPS, int rc)
{
	isLoadPrevious = isLPS;

	runCount = rc;
	maxRunCount = 0;

	if(!isLoadPrevious)
	{	threads[ci] = this;	}

	if(ci == 0)
		threadZero = this;
	else
		setMinDuration(static_cast<unsigned short>(ceil(threadZero->maxSpans * threadZero->getQualityParams().QFactor_factor)));

#ifdef RUN_GUI
	sprintf(topology,"%s",argv[1]);
	maxMaxUsage = 0.0;
	paintDir = false;
	paintSpans = false;
	paintRealTime = false;
	terminateProgram = false;
	numUsagesToDisplay = 0; //initialize this to zero, but should later be set to usageList.size();
	usageHistStartTime = 0; //initialize this to zero. First hist graph should start at t = 0
	progBarLengthpx = 600;
	paintedConns = 0;
#endif

	controllerIndex = ci;
	setGlobalTime(0.0);

	char fileName[200];

	if(controllerIndex == 0 && isLoadPrevious == false)
	{
		logger = new MessageLogger(argv[1],argv[2],argv[3],argv[6],rc);

		sprintf(fileName,"INPUT/Quality-%s-%s.txt",argv[1],argv[2]);
		setQualityParameters(fileName);
	}

	topology = argv[1];

	sprintf(fileName,"INPUT/Topology-%s.txt",argv[1]);
	setTopologyParameters(fileName);

	sprintf(fileName,"INPUT/Workstation-%s-%s.txt",argv[1],argv[2]);
	setWorkstationParameters(fileName);

	if(controllerIndex == 0 && isLoadPrevious == false)
	{
		qualityParams.max_probes = atoi(argv[6]);

		sprintf(fileName,"INPUT/Algorithm.txt");
		setAlgorithmParameters(fileName,atoi(argv[5]));

		numberOfConnections = static_cast<unsigned int>(TEN_HOURS) / 
			static_cast<unsigned int>(threadZero->getQualityParams().arrival_interval);
	}
	else
	{
		logger = 0;
	}

	if(isLoadPrevious == false)
	{
		queue = new EventQueue();

		randomSeed = atoi(argv[3]);

		order_init = false;
		workstationOrder = new unsigned short int[getNumberOfWorkstations()];
	}

	return;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	~Thread
// Description:		Closes the Event Logger and deletes all of the
//					memory allocated by the controller
//
///////////////////////////////////////////////////////////////////
Thread::~Thread()
{
	if(isLoadPrevious == false){
		delete queue;	}

	if(controllerIndex == 0 && isLoadPrevious == false)
	{
		delete[] qualityParams.ASE_perEDFA;

		delete logger;
		delete rm;

		for(unsigned short int a = 0; a < NUMBER_OF_ROUTING_ALGORITHMS; ++a)
			delete RoutingAlgorithmNames[a];

		for(unsigned short int w = 0; w < NUMBER_OF_WAVELENGTH_ALGORITHMS; ++w)
			delete WavelengthAlgorithmNames[w];

		for(unsigned short int p = 0; p < NUMBER_OF_PROBE_STYLES; ++p)
			delete ProbeStyleNames[p];
	}

	for(unsigned short int r = 0; r < routers.size(); ++r)
		delete routers[r];

	for(unsigned short int w = 0; w < workstations.size(); ++w)
		delete workstations[w];

	routers.clear();
	workstations.clear();
	
	if(isLoadPrevious == false)
	{
		delete[] workstationOrder;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	initPriorityQueue
// Description:		Creates a new Event Queue and initalizes several
//					events in it.
//
///////////////////////////////////////////////////////////////////
void Thread::initPriorityQueue(unsigned short int w)
{
	Event* activate = new Event;
	activate->e_type = ACTIVATE_WORKSTATIONS;
	activate->e_time = 0.0;
	activate->e_data = 0;

	Event* deactivate = new Event;
	deactivate->e_type = DEACTIVATE_WORKSTATIONS;
	deactivate->e_time = HUNDRED_HOURS - 1.0;
	deactivate->e_data = 0;

	queue->addEvent(*deactivate);
	queue->addEvent(*activate);

	delete deactivate;
	delete activate;

	if(CurrentRoutingAlgorithm == PABR || CurrentRoutingAlgorithm == LORA)
	{
		Event* event = new Event();

		event->e_type = UPDATE_USAGE;
		event->e_time = 0.0;
		event->e_data = 0;

		queue->addEvent(*event);

		delete event;
	}

#ifdef RUN_GUI
	Event* event = new Event();

	event->e_type = UPDATE_GUI;
	event->e_time = 0.0;
	event->e_data = 0;

	queue->addEvent(*event);

	delete event;
#endif

}

///////////////////////////////////////////////////////////////////
//
// Function Name:	initResourceManager
// Description:		Creates a new Resource Manager and initalizes several
//					events in it.
//
///////////////////////////////////////////////////////////////////
void Thread::initResourceManager()
{
	if(controllerIndex == 0)
		rm = new ResourceManager();
	else
		rm = 0;

	for(unsigned int r = 0; r < getNumberOfRouters(); ++r)
	{
		if(threadZero->getQualityParams().dest_dist != UNIFORM)
		{
			getRouterAt(r)->generateProbabilities();
		}
	}
}
///////////////////////////////////////////////////////////////////
//
// Function Name:	initThread
// Description:		Runs the simulation for the program until there
//					are no more events to run.
//
///////////////////////////////////////////////////////////////////
void Thread::initThread(AlgorithmToRun* alg)
{
	CurrentRoutingAlgorithm = alg->ra;
	CurrentWavelengthAlgorithm = alg->wa;
	CurrentProbeStyle = alg->ps;
	CurrentQualityAware = alg->qa;
	CurrentActiveWorkstations = alg->workstations;

#ifdef RUN_GUI
	rectfill(mainbuf, 0, 50*controllerIndex+85-1, SCREEN_W, 50*(controllerIndex+1)+85-1, makecol(0,0,0));
	char buffer[200];
	sprintf(buffer,"THREAD %d: RA = %s, WA = %s, PS = %s, QA = %d, N = %d",controllerIndex,
		threadZero->getRoutingAlgorithmName(CurrentRoutingAlgorithm)->c_str(),
		threadZero->getWavelengthAlgorithmName(CurrentWavelengthAlgorithm)->c_str(),
		threadZero->getProbeStyleName(CurrentProbeStyle)->c_str(),
		(int)CurrentQualityAware,CurrentActiveWorkstations);
	textprintf_ex(mainbuf,font,40,50*controllerIndex+85,makecol(0,255,0),-1,buffer); 
#endif

	initPriorityQueue(alg->workstations);

	delete alg;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	runThread
// Description:		Runs the simulation for the program until there
//					are no more events to run.
//
///////////////////////////////////////////////////////////////////
int Thread::runThread(AlgorithmToRun* alg)
{
	initThread(alg);

#ifdef RUN_GUI
	rectfill(mainbuf, 49, controllerIndex * 50 + 99, 651, controllerIndex * 50 + 99 + 26, makecol(0,0,255));
#endif

	while(queue->getSize() > 0)
	{
#ifdef RUN_GUI
		if(terminateProgram == true)
		{
			threadZero->recordEvent("ERROR: User clicked on the close button, exiting simulation.",true,controllerIndex);
			exit(ERROR_USER_CLOSED);
		}
#endif

		Event event = queue->getNextEvent();

		setGlobalTime(event.e_time);

#ifdef RUN_GUI //PROGRESS BAR
		
		int prog = stats.ConnectionRequests * multFactor;
		if(prog >= 1)
		{
			if((stats.ConnectionRequests - paintedConns) > (ConnsPerPx*5) && stats.ConnectionRequests > 0)
			{
				stretch_blit(progbarbmp, mainbuf,0,0,progbarbmp->w,progbarbmp->h,50,controllerIndex * 50 + 100,prog,progbarbmp->h);//new
				masked_blit(mainbuf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
				paintedConns = stats.ConnectionRequests;
			}
		}
#endif

		switch (event.e_type)
		{
			case ACTIVATE_WORKSTATIONS:
				activate_workstations();
				break;
			case DEACTIVATE_WORKSTATIONS:
				deactivate_workstations();
				break;
			case UPDATE_USAGE:
				update_link_usage();
				break;
#ifdef RUN_GUI
			case UPDATE_GUI:
				update_gui();
				break;
#endif
			case CONNECTION_REQUEST:
				connection_request(static_cast<ConnectionRequestEvent*>(event.e_data));
				delete static_cast<ConnectionRequestEvent*>(event.e_data);
				break;
			case COLLISION_NOTIFICATION:
				collision_notification(static_cast<CollisionNotificationEvent*>(event.e_data));
				break;
			case CREATE_CONNECTION_PROBE:
				create_connection_probe(static_cast<CreateConnectionProbeEvent*>(event.e_data));
				break;
			case CREATE_CONNECTION_CONFIRMATION:
				create_connection_confirmation(static_cast<CreateConnectionConfirmationEvent*>(event.e_data));
				break;
			case DESTROY_CONNECTION_PROBE:
				destroy_connection_probe(static_cast<DestroyConnectionProbeEvent*>(event.e_data));
				break;
			default:
				threadZero->recordEvent("ERROR: Unknown event type, exiting simulation.",true,controllerIndex);
				exit(ERROR_THREAD_EVENT_TYPE);
				break;
		}
	}
	
	if(runCount != maxRunCount)
	{
		return MORE_SIMULATIONS;
	}
	else
	{
		return COMPLETED_ALL_SIMULATIONS;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	activateWorkstations
// Description:		Activates the specified number of workstations
//					randomly
//
///////////////////////////////////////////////////////////////////
void Thread::activate_workstations()
{
#ifdef RUN_GUI
	ConnsPerPx = (threadZero->getNumberOfConnections() * getCurrentActiveWorkstations()) / progBarLengthpx;
	multFactor = 600.0 / float(getCurrentActiveWorkstations() * threadZero->getNumberOfConnections());
#endif

	stats.ConnectionRequests = 0;
	stats.ConnectionSuccesses = 0;
	stats.CollisionFailures = 0;
	stats.NoPathFailures = 0;
	stats.QualityFailures = 0;
	stats.DroppedFailures = 0;
	stats.ProbeSentCount = 0;

	stats.totalSetupDelay = 0.0;
	stats.totalHopCount = 0;
	stats.totalSpanCount = 0;

	stats.aseNoiseTotal = 0.0;
	stats.fwmNoiseTotal = 0.0;
	stats.xpmNoiseTotal = 0.0;
	stats.raRunTime = 0.0;

	//Random generator for destination router
	rng.seed(boost::uint32_t(getRandomSeed()));
	rt = new boost::uniform_int<>(0,getNumberOfRouters() - 1);
	generateRandomRouter = new boost::variate_generator<boost::mt19937&, boost::uniform_int<> >(rng, *rt);

	//Random generator for duration time
	rng2.seed(boost::uint32_t(getRandomSeed()));
	dt = new boost::exponential_distribution<>(1.0 / float(threadZero->getQualityParams().duration));
	generateRandomDuration = new boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> >(rng2, *dt);

	//Random generator for arrival interval
	rng3.seed(boost::uint32_t(getRandomSeed() * getRandomSeed()));
	ai = new boost::exponential_distribution<>(1.0 / float(threadZero->getQualityParams().arrival_interval));
	generateArrivalInterval = new boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> >(rng3, *ai);

	//Random generator for zero to one
	rng4.seed(boost::uint32_t(getRandomSeed()));
	zo = new boost::uniform_real<>(0,1);
	generateZeroToOne = new boost::variate_generator<boost::mt19937&, boost::uniform_real<> >(rng4, *zo);

	if(CurrentRoutingAlgorithm == SHORTEST_PATH)
	{
		threadZero->getResourceManager()->initSPMatrix();
	}
	else
	{
		bool SP_active = false;

		for(unsigned int t = 0; t < threadCount; ++t)
		{
			if(threads[t]->getCurrentRoutingAlgorithm() == SHORTEST_PATH)
			{
				SP_active = true;
				break;
			}
		}

		if(SP_active == false)
			threadZero->getResourceManager()->freeSPMatrix();
	}

	if(CurrentRoutingAlgorithm == ADAPTIVE_QoS)
	{
		for(unsigned int r = 0; r < getNumberOfRouters(); ++r)
		{
			getRouterAt(r)->resetFailures();
		}
	}

	if(order_init == false)
	{
		boost::mt19937 rng;
		rng.seed(boost::uint32_t(getRandomSeed()));
		boost::uniform_int<> wk(0,getNumberOfWorkstations() - 1);
		boost::variate_generator<boost::mt19937&, boost::uniform_int<> > generateRandomWorkstation(rng, wk);
		
		bool* init = new bool[getNumberOfWorkstations()];

		for(unsigned short int w = 0; w <getNumberOfWorkstations(); ++w)
		{
			init[w] = false;
			workstationOrder[w] = 0;
		}

		unsigned short int numberFound = 0;

		while(numberFound < getNumberOfWorkstations())
		{
			int wkstn = generateRandomWorkstation();

			if(init[wkstn] == false)
			{
				workstationOrder[numberFound] = wkstn;
				init[wkstn] = true;

				numberFound++;
			}
		}

		delete[] init;

		order_init = true;
	}

	char buffer[50];
	sprintf(buffer,"Activate %d workstations.",getCurrentActiveWorkstations());
	threadZero->recordEvent(buffer,false,controllerIndex);

	for(unsigned short int w = 0; w < getCurrentActiveWorkstations(); ++w)
	{
		getWorkstationAt(workstationOrder[w])->setActive(true);
#ifdef RUN_GUI
		getRouterAt(getWorkstationAt(workstationOrder[w])->getParentRouterIndex())->incNumWorkstations();
#endif
		char buffer[50];
		sprintf(buffer,"\tActivate workstation %d",workstationOrder[w]);
		threadZero->recordEvent(buffer,false,controllerIndex);
	}

	if(CurrentRoutingAlgorithm == PABR || CurrentRoutingAlgorithm == LORA)
	{
		for(unsigned short int r = 0; r < getNumberOfRouters(); ++r)
		{
			getRouterAt(r)->resetUsage();
		}
	}
	else if(CurrentRoutingAlgorithm == Q_MEASUREMENT || CurrentRoutingAlgorithm == ADAPTIVE_QoS)
	{
		for(unsigned short int r = 0; r < getNumberOfRouters(); ++r)
		{
			getRouterAt(r)->resetQMDegredation();
		}
	}

	for(unsigned int w = 0; w < getNumberOfWorkstations(); ++w)
		if(getWorkstationAt(w)->getActive() == true)
			generateTrafficEvent(w * threadZero->getNumberOfConnections());
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	deactivateWorkstations
// Description:		Deactivates all of the workstations and prints
//					statistics on their performance.
//
///////////////////////////////////////////////////////////////////
void Thread::deactivate_workstations()
{
	char buffer[200];

	threadZero->getLogger()->LockResultsMutex();

	sprintf(buffer,"**ALGORITHM = %s-%s, WORKS = %d, PROBE = %s, QA = %d**",threadZero->getRoutingAlgorithmName(CurrentRoutingAlgorithm)->c_str(),
		threadZero->getWavelengthAlgorithmName(CurrentWavelengthAlgorithm)->c_str(),getCurrentActiveWorkstations(),threadZero->getProbeStyleName(CurrentProbeStyle)->c_str(),getCurrentQualityAware());
	threadZero->recordEvent(buffer,true,controllerIndex);

#ifdef RUN_GUI
	strcpy(routing,threadZero->getRoutingAlgorithmName(CurrentRoutingAlgorithm)->c_str());
	strcpy(wavelength,threadZero->getWavelengthAlgorithmName(CurrentWavelengthAlgorithm)->c_str());
	strcpy(probing,threadZero->getProbeStyleName(CurrentProbeStyle)->c_str());
	qualityAware = getCurrentQualityAware();
	
	overallBlocking = float(stats.ConnectionRequests - stats.ConnectionSuccesses) / float(stats.ConnectionRequests);
	collisions = float(stats.CollisionFailures) / float(stats.ConnectionRequests);
	badquality = float(stats.QualityFailures) / float(stats.ConnectionRequests);
	nonresource = float(stats.NoPathFailures) / float(stats.ConnectionRequests);
	avgprobesperrequest = float(stats.ProbeSentCount) / float(stats.ConnectionRequests);
	avgrequestdelaytime = stats.totalSetupDelay / float(stats.ConnectionSuccesses);
	avgconnhopcount = float(stats.totalHopCount) / float(stats.ConnectionSuccesses);
	avgconnspancount = float(stats.totalSpanCount) / float(stats.ConnectionSuccesses);
	avgASEnoise = stats.aseNoiseTotal / double(stats.ConnectionSuccesses);
	avgFWMnoise = stats.fwmNoiseTotal / double(stats.ConnectionSuccesses);
	avgXPMnoise = stats.xpmNoiseTotal / double(stats.ConnectionSuccesses);
#endif

	sprintf(buffer,"OVERALL BLOCKING (%d/%d) = %f", stats.ConnectionRequests - stats.ConnectionSuccesses, stats.ConnectionRequests, 
		float(stats.ConnectionRequests - stats.ConnectionSuccesses) / float(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);
	
	sprintf(buffer,"COLLISIONS (%d/%d) = %f", stats.CollisionFailures, 
		stats.ConnectionRequests, float(stats.CollisionFailures) / float(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"BAD QUALITY (%d/%d) = %f", stats.QualityFailures, stats.ConnectionRequests, 
		float(stats.QualityFailures) / float(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"NON RESOURCES (%d/%d) = %f", stats.NoPathFailures, stats.ConnectionRequests, 
		float(stats.NoPathFailures) / float(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE PROBES PER REQUEST (%d/%d) = %f", stats.ProbeSentCount, stats.ConnectionRequests, 
		float(stats.ProbeSentCount) / float(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE REQUEST DELAY TIME (%f/%d) = %f", stats.totalSetupDelay, stats.ConnectionSuccesses, 
		stats.totalSetupDelay / float(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE CONNECTION HOP COUNT (%d/%d) = %f", stats.totalHopCount, stats.ConnectionSuccesses, 
		float(stats.totalHopCount) / float(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE CONNECTION SPAN COUNT (%d/%d) = %f", stats.totalSpanCount, stats.ConnectionSuccesses, 
		float(stats.totalSpanCount) / float(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE ASE NOISE (%f/%d) = %e", stats.aseNoiseTotal, stats.ConnectionSuccesses,
		stats.aseNoiseTotal / double(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE FWM NOISE (%f/%d) = %e", stats.fwmNoiseTotal, stats.ConnectionSuccesses,
		stats.fwmNoiseTotal / double(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE XPM NOISE (%f/%d) = %e", stats.xpmNoiseTotal, stats.ConnectionSuccesses,
		stats.xpmNoiseTotal / double(stats.ConnectionSuccesses));
	threadZero->recordEvent(buffer,true,controllerIndex);

	sprintf(buffer,"AVERAGE RA RUN TIME (%f/%d) = %e", stats.raRunTime, stats.ConnectionRequests,
		stats.raRunTime / double(stats.ConnectionRequests));
	threadZero->recordEvent(buffer,true,controllerIndex);

	if(threadZero->getQualityParams().q_factor_stats == true)
	{
		double worstInitQ = std::numeric_limits<float>::infinity();
		double bestInitQ = 0.0;
		double averageInitQ = 0.0;

		double worstAvgQ = std::numeric_limits<float>::infinity();
		double bestAvgQ = 0.0;
		double averageAvgQ = 0.0;

		double worstPerQ = std::numeric_limits<float>::infinity();
		double bestPerQ = 0.0;
		double averagePerQ = 0.0;

		double timeTotal = 0.0;
		double countTotal = 0.0;
		double droppedTotal = 0.0;

		for(unsigned int r = 0; r < threadZero->getNumberOfRouters(); ++r)
		{
			Router* router = getRouterAt(r);

			for(unsigned int e = 0; e < router->getNumberOfEdges(); ++e)
			{
				EdgeStats* stats = router->getEdgeByIndex(e)->getEdgeStats();

				averageInitQ += stats->totalInitalQFactor;
				averageAvgQ += stats->totalAverageQFactor;
				averagePerQ += stats->totalPercentQFactor;

				timeTotal += stats->totalTime;
				countTotal += stats->count;

				droppedTotal += stats->droppedConnections;

				if(stats->minInitalQFactor < worstInitQ)
					worstInitQ = stats->minInitalQFactor;
				else if(stats->maxInitalQFactor > bestInitQ)
					bestInitQ = stats->maxInitalQFactor;

				if(stats->minAverageQFactor < worstAvgQ)
					worstAvgQ = stats->minAverageQFactor;
				else if(stats->maxAverageQFactor > bestAvgQ)
					bestAvgQ = stats->maxAverageQFactor;

				if(stats->minPercentQFactor < worstPerQ)
					worstPerQ = stats->minPercentQFactor;
				else if(stats->maxPercentQFactor > bestPerQ)
					bestPerQ = stats->maxPercentQFactor;

				router->getEdgeByIndex(e)->resetEdgeStats();
			}
		}

		sprintf(buffer,"DROPPED CONNECTIONS (%d/%d) = %f", int(droppedTotal), stats.ConnectionRequests, 
			droppedTotal / float(stats.ConnectionRequests));
		threadZero->recordEvent(buffer,true,controllerIndex);

		sprintf(buffer,"OVERALL W/DROPPED (%d/%d) = %f", int(stats.ConnectionRequests - stats.ConnectionSuccesses + droppedTotal), stats.ConnectionRequests, 
			float(stats.ConnectionRequests - stats.ConnectionSuccesses + droppedTotal) / float(stats.ConnectionRequests));
		threadZero->recordEvent(buffer,true,controllerIndex);

		sprintf(buffer,"INITIAL Q: MIN = %f, MAX = %f, AVG = %f",worstInitQ,bestInitQ,averageInitQ / countTotal);
		threadZero->recordEvent(buffer,true,controllerIndex);

		sprintf(buffer,"AVERAGE Q: MIN = %f, MAX = %f, AVG = %f",worstAvgQ,bestAvgQ,averageAvgQ / countTotal);
		threadZero->recordEvent(buffer,true,controllerIndex);

		sprintf(buffer,"%% TIME Q BELOW: MIN = %f, MAX = %f, AVG = %f",worstPerQ,bestPerQ,averagePerQ / timeTotal);
		threadZero->recordEvent(buffer,true,controllerIndex);
	}

	sprintf(buffer,"***********************************************\n");
	threadZero->recordEvent(buffer,true,controllerIndex);

	threadZero->getLogger()->UnlockResultsMutex();

	for(unsigned short int r1 = 0; r1 < getNumberOfRouters(); ++r1)
	{
		for(unsigned short int r2 = 0; r2 < getNumberOfRouters(); ++r2)
		{
			Edge* edge = getRouterAt(r1)->getEdgeByDestination(r2);

			if(edge != 0)
			{
				for(unsigned short int k = 0; k < threadZero->getNumberOfWavelengths(); ++k)
				{
					if(edge->getStatus(k) != EDGE_FREE)
					{
						threadZero->recordEvent("ERROR: Edge is still used when all edges should be free.",true,controllerIndex);
						exit(ERROR_EDGE_IS_USED);
					}
				}
			}
		}
	}

	threadZero->recordEvent("Deactivate workstations.",false,controllerIndex);

	for(unsigned short int w = 0; w < getNumberOfWorkstations(); ++w)
	{
		if(getWorkstationAt(w)->getActive() == true)
		{
			char buffer[50];
			sprintf(buffer,"\tDeactivate workstation %d",w);
			threadZero->recordEvent(buffer,false,controllerIndex);

			getWorkstationAt(w)->setActive(false);
		}
	}

	delete rt;
	delete generateRandomRouter;

	delete dt;
	delete generateRandomDuration;

	delete ai;
	delete generateArrivalInterval;

	delete zo;
	delete generateZeroToOne;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	generateTrafficEvent
// Description:		Generates connection request events to setup requests for new
//					connections.
//
///////////////////////////////////////////////////////////////////
void Thread::generateTrafficEvent(unsigned int session)
{
	unsigned int workstation = session / threadZero->getNumberOfConnections();

	Event* tr = new Event();
	ConnectionRequestEvent* tr_data = new ConnectionRequestEvent();

	tr->e_time = getGlobalTime() + float((*generateArrivalInterval)());
	tr->e_type = CONNECTION_REQUEST;
	tr->e_data = tr_data;

	tr_data->connectionDuration = float((*generateRandomDuration)());

	if(tr_data->connectionDuration < threadZero->getMinDuration())
		tr_data->connectionDuration = threadZero->getMinDuration();

	tr_data->requestBeginTime = tr->e_time;
	tr_data->sourceRouterIndex = getWorkstationAt(workstation)->getParentRouterIndex();
	tr_data->destinationRouterIndex = tr_data->sourceRouterIndex;
	tr_data->session = session;
	tr_data->sequence = 0;
	tr_data->max_sequence = 0;
	tr_data->qualityFail = false;

	while(tr_data->sourceRouterIndex == tr_data->destinationRouterIndex)
	{
		if(threadZero->getQualityParams().dest_dist == UNIFORM)
			tr_data->destinationRouterIndex = (*generateRandomRouter)();
		else if(threadZero->getQualityParams().dest_dist == DISTANCE ||
				threadZero->getQualityParams().dest_dist == INVERSE_DISTANCE)
			tr_data->destinationRouterIndex = getRouterAt(tr_data->sourceRouterIndex)->generateDestination((*generateZeroToOne)());
	}

//ifdef RUN_GUI, increment the number of connection attempts FROM the source
//and increment the number of connection attempts TO the destination
#ifdef RUN_GUI
	routers[tr_data->sourceRouterIndex]->incConnAttemptsFrom();
	routers[tr_data->destinationRouterIndex]->incConnAttemptsTo();
#endif

	queue->addEvent(*tr);

	delete tr;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	connection_request
// Description:		Handles a connection request event
//
///////////////////////////////////////////////////////////////////
void Thread::connection_request(ConnectionRequestEvent* cre)
{
	if(getGlobalTime() < TEN_HOURS)
	{
		if((cre->session+1) % threadZero->getNumberOfConnections() != 0)
			generateTrafficEvent(cre->session+1);
		else
			generateTrafficEvent(cre->session + 1 - threadZero->getNumberOfConnections());
	}

	++stats.ConnectionRequests;

	if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING || CurrentRoutingAlgorithm  == IMPAIRMENT_AWARE)
	{
		++stats.ProbeSentCount;
	}

	unsigned short int probesToSend = 0;
	unsigned short int probeStart = 0;
	unsigned short int probesSkipped = 0;

	if(CurrentProbeStyle == SINGLE)
		probesToSend = 1;
	else
		probesToSend = threadZero->getQualityParams().max_probes;

	kShortestPathReturn* kPath = getKShortestPaths(cre,probesToSend);

	CreateConnectionProbeEvent** probesList = calcProbesToSend(cre,kPath,probesToSend,probeStart,probesSkipped);

	sendProbes(cre,kPath,probesList,probesToSend,probeStart,probesSkipped);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	create_connection_probe
// Description:		Handles the probe message for creating a new
//					connection.
//
///////////////////////////////////////////////////////////////////
void Thread::create_connection_probe(CreateConnectionProbeEvent* ccpe)
{
	++ccpe->numberOfHops;

	if(ccpe->numberOfHops == ccpe->connectionLength)
	{
		ccpe->atDestination = true;

		if(CurrentProbeStyle != PARALLEL || sendResponse(ccpe) == true)
		{
			double q_factor = 0.0;
			double xpm_noise = 0.0;
			double fwm_noise = 0.0;
			double ase_noise = 0.0;

			Event *event = new Event();
			CreateConnectionConfirmationEvent *ccce = new CreateConnectionConfirmationEvent;

			ccce->connectionDuration = ccpe->connectionDuration;
			ccce->requestBeginTime = ccpe->requestBeginTime;
			ccce->connectionLength = ccpe->connectionLength;
			ccce->connectionPath = ccpe->connectionPath;
			ccce->destinationRouterIndex = ccpe->destinationRouterIndex;
			ccce->numberOfHops = 0;
			ccce->sourceRouterIndex = ccpe->sourceRouterIndex;
			ccce->kPaths = ccpe->kPaths;
			ccce->session = ccpe->session;
			ccce->sequence = ccpe->sequence;
			ccce->max_sequence = ccpe->max_sequence;
			ccce->originalWavelength = ccpe->wavelength;
			ccce->probes = ccpe->probes;
			ccce->finalFailure = false;

			ccpe->decisionTime = getGlobalTime();
			
			//Again...some algorithms have to be treated differently because they
			//use a forward reservation scheme.
			if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE || CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
			{
				double q_factor = 0.0;
				double xpm_noise = 0.0;
				double fwm_noise = 0.0;
				double ase_noise = 0.0;

				if(ccpe->wavelength >= 0)
				{
					for(unsigned int p = 0; p < ccpe->connectionLength; ++p)
					{
						if(ccpe->connectionPath[p]->getStatus(ccpe->wavelength) != EDGE_FREE)
						{
							ccpe->wavelength = NO_PATH_FAILURE;
							break;
						}
					}

					if(ccpe->wavelength != NO_PATH_FAILURE)
					{
						q_factor = threadZero->getResourceManager()->estimate_Q(
							ccpe->wavelength,ccpe->connectionPath,ccpe->connectionLength,&xpm_noise,
							&fwm_noise,&ase_noise,controllerIndex);

						if(getCurrentQualityAware() == true && q_factor < threadZero->getQualityParams().TH_Q)
							ccpe->wavelength = QUALITY_FAILURE;
					}
				}

				if(ccpe->wavelength >= 0 || CurrentRoutingAlgorithm == IMPAIRMENT_AWARE)
				{
					threadZero->getResourceManager()->print_connection_info(ccpe,q_factor,
						ase_noise,fwm_noise,xpm_noise,controllerIndex);
				}
				else
				{
					time_t start;
					time_t end;

					time(&start);

					ccpe->wavelength = threadZero->getResourceManager()->choose_wavelength(ccpe,controllerIndex);

					time(&end);
					stats.raRunTime += difftime(end,start);
				}
			}
			else
			{
				time_t start;
				time_t end;

				time(&start);

				ccpe->wavelength = threadZero->getResourceManager()->choose_wavelength(ccpe,controllerIndex);

				time(&end);
				stats.raRunTime += difftime(end,start);
			}
		
			if(CurrentProbeStyle == PARALLEL && ccpe->wavelength == NO_PATH_FAILURE)
			{
				for(unsigned int p = 0; p < ccce->max_sequence; ++p)
				{
					if(ccce->probes[p]->wavelength == QUALITY_FAILURE)
					{
						ccpe->wavelength = QUALITY_FAILURE;
						break;
					}
				}
			}
			else if(CurrentProbeStyle == SERIAL && ccpe->wavelength == NO_PATH_FAILURE)
			{
				if(ccpe->qualityFail == true)
					ccpe->wavelength = QUALITY_FAILURE;
			}

			ccce->wavelength = ccpe->wavelength;

			if((ccpe->wavelength == QUALITY_FAILURE || ccpe->wavelength == NO_PATH_FAILURE)
				&& otherResponse(ccpe) != -1)
			{
				int sequence = otherResponse(ccpe);

				--ccpe->probes[sequence]->numberOfHops;

				create_connection_probe(ccpe->probes[sequence]);

				delete ccce;
			}
			else if((ccpe->wavelength == QUALITY_FAILURE || ccpe->wavelength == NO_PATH_FAILURE)
				&& moreProbes(ccpe) == true)
			{
				//Don't send rejection, just wait for more probes
				delete ccce;
			}
			else
			{
				event->e_type = CREATE_CONNECTION_CONFIRMATION;
				event->e_time = getGlobalTime() + calculateDelay(ccpe->connectionPath[ccpe->numberOfHops-1]->getNumberOfSpans());
				event->e_data = ccce;

				queue->addEvent(*event);
			}

			if(CurrentProbeStyle != PARALLEL)
				delete ccpe;
			
			delete event;
		}
	}
	else
	{
		//We are not at our destination yet, so continue with the probe.
		Event *event = new Event();

		event->e_type = CREATE_CONNECTION_PROBE;
		event->e_time = getGlobalTime() + calculateDelay(ccpe->connectionPath[ccpe->numberOfHops]->getNumberOfSpans());
		event->e_data = ccpe;

		queue->addEvent(*event);

		delete event;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	destroy_connection_probe
// Description:		Handles the probe message for creating a new
//					connection.
//
///////////////////////////////////////////////////////////////////
void Thread::destroy_connection_probe(DestroyConnectionProbeEvent* dcpe)
{
	if(dcpe->numberOfHops == 0)
	{
		if(CurrentRoutingAlgorithm == Q_MEASUREMENT || CurrentRoutingAlgorithm == ADAPTIVE_QoS)
		{
			for(unsigned int p = 0; p < dcpe->connectionLength; ++p)
				dcpe->connectionPath[p]->removeEstablishedConnection(dcpe);

			updateQMDegredation(dcpe->connectionPath, dcpe->connectionLength, dcpe->wavelength);
		}
		else if(threadZero->getQualityParams().q_factor_stats == true)
		{
			for(unsigned int p = 0; p < dcpe->connectionLength; ++p)
				dcpe->connectionPath[p]->removeEstablishedConnection(dcpe);

			updateQFactorStats(dcpe->connectionPath, dcpe->connectionLength, dcpe->wavelength);
		}
	}

	Edge* edge = dcpe->connectionPath[dcpe->numberOfHops];

	if(edge->getStatus(dcpe->wavelength) == EDGE_USED)
	{
		edge->setFree(dcpe->wavelength);
	}
	else
	{
		unsigned short int srcIndex = edge->getSourceIndex();
		unsigned short int destIndex = edge->getDestinationIndex();

		char buffer[100];
		sprintf(buffer,"ERROR: Something is wrong, an edge from %d to %d on wave %d should be marked as used is actually free already.",srcIndex,destIndex,dcpe->wavelength);
		threadZero->recordEvent(buffer,true,controllerIndex);
		exit(ERROR_EDGE_IS_FREE);
	}

	++dcpe->numberOfHops;

	if(dcpe->numberOfHops == dcpe->connectionLength)
	{
		char buffer[100];
		string line;

		sprintf(buffer,"DESTROY CONNECTION: Routers[%d]:",dcpe->connectionLength);
		line.append(buffer);

		for(unsigned short int r = 0; r < dcpe->connectionLength; ++r)
		{
			itoa(dcpe->connectionPath[r]->getSourceIndex(),buffer,10);
			line.append(buffer);

			if(r < dcpe->connectionLength)
				line.append(",");
		}

		itoa(dcpe->connectionPath[dcpe->connectionLength - 1]->getDestinationIndex(),buffer,10);
		line.append(buffer);

		sprintf(buffer," Wavelength = %d ",dcpe->wavelength);
		line.append(buffer);

		sprintf(buffer,"Session = %d\n",dcpe->session);
		line.append(buffer);

		threadZero->recordEvent(line,false,controllerIndex);

		if(CurrentProbeStyle == PARALLEL)
			clearResponses(dcpe->probes[dcpe->sequence]);

		delete[] dcpe->connectionPath;
		delete dcpe;
	}
	else
	{
		//We are not at our destination yet, so continue with the probe.
		Event *event = new Event();

		event->e_type = DESTROY_CONNECTION_PROBE;
		event->e_time = getGlobalTime() + calculateDelay(dcpe->connectionPath[dcpe->numberOfHops]->getNumberOfSpans());
		event->e_data = dcpe;

		queue->addEvent(*event);

		delete event;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	create_connection_confirmation
// Description:		Handles the probe message for creating a new
//					connection.
//
///////////////////////////////////////////////////////////////////
void Thread::create_connection_confirmation(CreateConnectionConfirmationEvent* ccce)
{
#ifdef RUN_GUI
	int indx;
#endif

	Edge* edge = ccce->connectionPath[ccce->connectionLength - ccce->numberOfHops - 1];

	if(ccce->wavelength >= 0 && edge->getStatus(ccce->wavelength) == EDGE_FREE)
	{
		//No collision yet, so reserve the link.
		edge->setUsed(ccce->session,ccce->wavelength);
	}
	else if(ccce->wavelength >= 0 && edge->getStatus(ccce->wavelength) == EDGE_USED)
	{
		//Oops...a collision. We need to send a response upstream to notify the routers to release the
		//resources that were scheduled for this event.
		char buffer[200];		
		sprintf(buffer,"COLLISION: Session = %d, Sequence = %d\n",ccce->session, ccce->sequence);

		threadZero->recordEvent(buffer,false,controllerIndex);

		Event* event = new Event();
		CollisionNotificationEvent* cne = new CollisionNotificationEvent();

		event->e_type = COLLISION_NOTIFICATION;
		event->e_time = getGlobalTime() + calculateDelay(edge->getNumberOfSpans());
		event->e_data = cne;

		cne->sourceRouterIndex = edge->getSourceIndex();
		cne->destinationRouterIndex = ccce->destinationRouterIndex;
		cne->wavelength = ccce->wavelength;
		cne->numberOfHops = ccce->connectionLength - ccce->numberOfHops - 1;
		cne->connectionLength = ccce->connectionLength;
		cne->connectionPath = new Edge *[cne->connectionLength];
		cne->session = ccce->session;
		cne->sequence = ccce->sequence;
		cne->probes = ccce->probes;

		if(CurrentProbeStyle == PARALLEL)
			cne->max_sequence = ccce->probes[ccce->sequence]->max_sequence;
		else
			cne->max_sequence = 1;

		for(unsigned short int p = 0; p < ccce->connectionLength; ++p)
			cne->connectionPath[p] = ccce->connectionPath[p];

		cne->finalFailure = true;

		if(CurrentProbeStyle  == PARALLEL)
		{
			for(unsigned int p = 0; p < ccce->probes[ccce->sequence]->max_sequence; ++p)
			{
				if(p != ccce->sequence)
				{
					if(ccce->probes[p]->atDestination == false)
					{
						cne->finalFailure = false;
						break;
					}
					else
					{
						if(ccce->probes[p]->decisionTime == 0.0 ||
						   ccce->probes[p]->decisionTime > ccce->probes[ccce->sequence]->decisionTime)
						{
							cne->finalFailure = false;
							break;
						}
					}
				}
			}
		}
		
		ccce->finalFailure = cne->finalFailure;

		queue->addEvent(*event);

		delete event;

		ccce->wavelength = COLLISION_FAILURE;
	}

	++ccce->numberOfHops;

	if(ccce->connectionLength > ccce->numberOfHops)
	{
		//Forward the confirmation downstream.
		Event* event = new Event();

		event->e_type = CREATE_CONNECTION_CONFIRMATION;
		event->e_time = getGlobalTime() + calculateDelay(edge->getNumberOfSpans());
		event->e_data = ccce;

		queue->addEvent(*event);

		delete event;
	}
	else if(ccce->connectionLength == ccce->numberOfHops)
	{
		if(ccce->wavelength >= 0)
		{
			++stats.ConnectionSuccesses;

			if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING || CurrentRoutingAlgorithm  == IMPAIRMENT_AWARE)
			{
				stats.totalSetupDelay += (threadZero->getResourceManager()->span_distance[ccce->sourceRouterIndex * threadZero->getNumberOfRouters()] * 
					threadZero->getQualityParams().L * 1000) / (SPEED_OF_LIGHT / threadZero->getQualityParams().refractive_index);
			}
			else
			{
				stats.totalSetupDelay += getGlobalTime() - ccce->requestBeginTime;
			}

#ifdef RUN_GUI
			indx = ccce->destinationRouterIndex; //this code keeps track of how many times each router is the destination for a connection.
			routers[indx]->incConnSuccessesTo();
			indx = ccce->sourceRouterIndex;
			routers[indx]->incConnSuccessesFrom();//i think this stuff works.
#endif

			//Yeah, we have made it back to the destination and everything worked great.
			Event* event = new Event();
			DestroyConnectionProbeEvent* dcpe = new DestroyConnectionProbeEvent();

			event->e_type = DESTROY_CONNECTION_PROBE;
			event->e_time = getGlobalTime() + ccce->connectionDuration;
			event->e_data = dcpe;

			dcpe->connectionLength = ccce->connectionLength;
			dcpe->connectionPath = ccce->connectionPath;
			dcpe->numberOfHops = 0;
			dcpe->wavelength = ccce->wavelength;
			dcpe->session = ccce->session;
			dcpe->sequence = ccce->sequence;
			dcpe->probes = ccce->probes;

			queue->addEvent(*event);

			delete event;

			if(threadZero->getQualityParams().q_factor_stats == true ||
			   CurrentRoutingAlgorithm == Q_MEASUREMENT || CurrentRoutingAlgorithm == ADAPTIVE_QoS)
			{
				EstablishedConnection* ec = new EstablishedConnection();

				ec->connectionLength = ccce->connectionLength;
				ec->connectionPath = ccce->connectionPath;
				ec->wavelength = ccce->wavelength;
				ec->connectionStartTime = getGlobalTime();
				ec->connectionEndTime = getGlobalTime() + ccce->connectionDuration;

				if(threadZero->getQualityParams().q_factor_stats == true)
				{
					ec->QFactors = new vector<double>;
					ec->QTimes = new vector<double>;
				}

				for(unsigned int p = 0; p < ec->connectionLength; ++p)
				{
					ec->connectionPath[p]->insertEstablishedConnection(ec);
				}
			}

			stats.totalHopCount += ccce->connectionLength;

			for(unsigned short int p = 0; p < ccce->connectionLength; ++p)
			{
				stats.totalSpanCount += ccce->connectionPath[p]->getNumberOfSpans();
			}

			if(CurrentRoutingAlgorithm == Q_MEASUREMENT || CurrentRoutingAlgorithm == ADAPTIVE_QoS)
			{
				updateQMDegredation(ccce->connectionPath, ccce->connectionLength, ccce->wavelength);
			}
			else if(threadZero->getQualityParams().q_factor_stats == true)
			{
				updateQFactorStats(ccce->connectionPath, ccce->connectionLength, ccce->wavelength);
			}

			if(CurrentProbeStyle == SERIAL && CurrentRoutingAlgorithm != SHORTEST_PATH)
			{
				delete[] ccce->kPaths->pathcost;
				delete[] ccce->kPaths->pathinfo;
				delete[] ccce->kPaths->pathlen;

				delete ccce->kPaths;
			}

			delete ccce;
		}
		else if(CurrentProbeStyle == SERIAL && ccce->sequence < ccce->max_sequence - 1)
		{
			unsigned short int probesToSend = 0;
			unsigned short int probeStart = 0;
			unsigned short int probesSkipped = ccce->sequence + 1;

			ConnectionRequestEvent *cre = new ConnectionRequestEvent;

			cre->connectionDuration = ccce->connectionDuration;
			cre->destinationRouterIndex = ccce->destinationRouterIndex;
			cre->requestBeginTime = ccce->requestBeginTime;
			cre->session = ccce->session;
			cre->sequence = ccce->sequence + 1;
			cre->sourceRouterIndex = ccce->sourceRouterIndex;
			cre->wavelength = ccce->originalWavelength;
			cre->max_sequence = ccce->max_sequence;

			if(ccce->wavelength == QUALITY_FAILURE)
				cre->qualityFail = true;
			else
				cre->qualityFail = false;

			CreateConnectionProbeEvent** probesList = calcProbesToSend(cre,ccce->kPaths,probesToSend,probeStart,probesSkipped);

			sendProbes(cre,ccce->kPaths,probesList,probesToSend,probeStart,probesSkipped);

			delete[] ccce->connectionPath;
			delete ccce;

			delete cre;
		}
		else if(ccce->wavelength == COLLISION_FAILURE)
		{			
			if(CurrentProbeStyle == PARALLEL && ccce->finalFailure == true)
			{
				clearResponses(ccce->probes[ccce->sequence]);
				++stats.CollisionFailures;
			}
			else if(CurrentProbeStyle != PARALLEL)
			{
				++stats.CollisionFailures;

				if(CurrentProbeStyle == SERIAL && CurrentRoutingAlgorithm != SHORTEST_PATH)
				{
					delete[] ccce->kPaths->pathcost;
					delete[] ccce->kPaths->pathinfo;
					delete[] ccce->kPaths->pathlen;

					delete ccce->kPaths;
				}

				delete[] ccce->connectionPath;
			}

			delete ccce;
		}
		else if(ccce->wavelength == QUALITY_FAILURE)
		{
			if(CurrentRoutingAlgorithm == ADAPTIVE_QoS)
			{
				getRouterAt(ccce->sourceRouterIndex)->incrementQualityFailures();
			}

			if(CurrentProbeStyle == PARALLEL)
			{
				clearResponses(ccce->probes[ccce->sequence]);
			}

			++stats.QualityFailures;

			if(CurrentProbeStyle == SERIAL && CurrentRoutingAlgorithm != SHORTEST_PATH)
			{
				delete[] ccce->kPaths->pathcost;
				delete[] ccce->kPaths->pathinfo;
				delete[] ccce->kPaths->pathlen;

				delete ccce->kPaths;
			}

			delete[] ccce->connectionPath;
			delete ccce;            
		}
		else if(ccce->wavelength == NO_PATH_FAILURE)
		{
			if(CurrentRoutingAlgorithm == ADAPTIVE_QoS)
			{
				getRouterAt(ccce->sourceRouterIndex)->incrementWaveFailures();
			}

			if(CurrentProbeStyle == PARALLEL)
			{
				clearResponses(ccce->probes[ccce->sequence]);
			}

			++stats.NoPathFailures;

			if(CurrentProbeStyle == SERIAL && CurrentRoutingAlgorithm != SHORTEST_PATH)
			{
				delete[] ccce->kPaths->pathcost;
				delete[] ccce->kPaths->pathinfo;
				delete[] ccce->kPaths->pathlen;

				delete ccce->kPaths;
			}

			delete[] ccce->connectionPath;
			delete ccce;
		}
	}
	else
	{
		threadZero->recordEvent("ERROR: Invalid connection confirmation, something has really gone wrong here.",true,controllerIndex);
		exit(ERROR_INVALID_CONFIRMATION);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	collision_notification
// Description:		Handles the probe message for creating a new
//					connection.
//
///////////////////////////////////////////////////////////////////
void Thread::collision_notification(CollisionNotificationEvent* cne)
{
	if(cne->connectionPath[cne->numberOfHops]->getSourceIndex() != cne->sourceRouterIndex)
	{
		if(cne->connectionPath[cne->numberOfHops]->getStatus(cne->wavelength) == EDGE_USED)
		{
			cne->connectionPath[cne->numberOfHops]->setFree(cne->wavelength);
		}
		else
		{
			threadZero->recordEvent("ERROR: Something has gone wrong here...this edge should be used and it is free.",true,controllerIndex);
			exit(ERROR_EDGE_IS_FREE);
		}
	}

	++cne->numberOfHops;

	if(cne->numberOfHops == cne->connectionLength)
	{	
		if(cne->max_sequence > 1)
		{
			if(cne->finalFailure == false)
			{
				cne->probes[cne->sequence]->wavelength = COLLISION_FAILURE;

				if(otherResponse(cne->probes[cne->sequence]) != -1)
				{
					int sequence = otherResponse(cne->probes[cne->sequence]);

					--cne->probes[sequence]->numberOfHops;

					create_connection_probe(cne->probes[sequence]);
				}
			}
		}

		delete[] cne->connectionPath;
		delete cne;
	}
	else
	{
		Event* event = new Event();

		event->e_type = COLLISION_NOTIFICATION;
		event->e_time = getGlobalTime() + calculateDelay(cne->connectionPath[cne->numberOfHops]->getNumberOfSpans());
		event->e_data = cne;

		queue->addEvent(*event);

		delete event;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getKthParameterFloat
// Description:		Gets the kth parameter and returns it as a float.
//
///////////////////////////////////////////////////////////////////
float Thread::getKthParameterFloat(char* f)
{
	if(strchr(f,',') == 0)
	{
		return atof(f);
	}
	else
	{
		char* token = strtok(f,",");
		float retVal;
		int t = 0;

		while(token != 0)
		{
			if(t == runCount)
				retVal = atof(token);

			token = strtok(NULL,",");

			if(t > maxRunCount)
				maxRunCount = t;

			++t;
		}

		return retVal;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getKthParameterInt
// Description:		Gets the kth parameter and returns it as an int.
//
///////////////////////////////////////////////////////////////////
int Thread::getKthParameterInt(char* f)
{
	if(strchr(f,',') == 0)
	{
		return atoi(f);
	}
	else
	{
		char* token = strtok(f,",");
		int retVal;
		int t = 0;

		while(token != 0)
		{
			if(t == runCount)
				retVal = atoi(f);

			++t;
			token = strtok(NULL,",");

			if(t > maxRunCount)
				maxRunCount = t;
		}

		return retVal;
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setQualityParameters
// Description:		Opens up the Quality Paremeters file that was
//					specified by the command line arguement
//
///////////////////////////////////////////////////////////////////
void Thread::setQualityParameters(const char* f)
{
	//Default setting is uniform. Can be modifed using the parameter file.
	qualityParams.dest_dist = UNIFORM;

	char buffer[200];
	sprintf(buffer,"Reading Quality Parameters from %s file.",f);
	threadZero->recordEvent(buffer,true,0);

	ifstream inFile(f);

	while(inFile.getline(buffer,199))
	{
		char *param;
		char *value;
		
		param = strtok(buffer,"=");
		value = strtok(NULL,"\t");

		if(strcmp(param,"arrival_interval") == 0)
		{
			qualityParams.arrival_interval = getKthParameterFloat(value);
			sprintf(buffer,"\tarrival_interval = %f",qualityParams.arrival_interval);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"duration") == 0)
		{
			qualityParams.duration = getKthParameterFloat(value);
			sprintf(buffer,"\tduration = %f",qualityParams.duration);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"nonlinear_halfwin") == 0)
		{
			qualityParams.nonlinear_halfwin = getKthParameterInt(value);
			sprintf(buffer,"\tnonlinear_halfwin = %d",qualityParams.nonlinear_halfwin);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"halfwavelength") == 0)
		{
			qualityParams.halfwavelength = getKthParameterInt(value);
			sprintf(buffer,"\thalfwavelength = %d",qualityParams.halfwavelength);
			threadZero->recordEvent(buffer,true,0);

			numOfWavelengths = 2 * qualityParams.halfwavelength + 1;
			sprintf(buffer,"\tsys_wavelength = %d",threadZero->getNumberOfWavelengths());
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"fc") == 0)
		{
			qualityParams.fc = getKthParameterFloat(value);
			sprintf(buffer,"\tfc = %f",qualityParams.fc);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"f_step") == 0)
		{
			qualityParams.f_step = getKthParameterFloat(value);
			sprintf(buffer,"\tf_step = %f",qualityParams.f_step);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"channel_power") == 0)
		{
			qualityParams.channel_power = getKthParameterFloat(value);
			sprintf(buffer,"\tchannel_power = %f",qualityParams.channel_power);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"L") == 0)
		{
			qualityParams.L = getKthParameterFloat(value);
			sprintf(buffer,"\tL = %f",qualityParams.L);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"alphaDB") == 0)
		{
			qualityParams.alphaDB = getKthParameterFloat(value);
			qualityParams.alpha = float(qualityParams.alphaDB * 0.1 / log10(exp(1.0)));
			sprintf(buffer,"\talphaDB = %f",qualityParams.alphaDB);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"D") == 0)
		{
			qualityParams.D = getKthParameterFloat(value);
			sprintf(buffer,"\tD = %f",qualityParams.D);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"S") == 0)
		{
			qualityParams.S = getKthParameterFloat(value);
			sprintf(buffer,"\tS = %f",qualityParams.S);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"gamma") == 0)
		{
			qualityParams.gamma = getKthParameterFloat(value);
			sprintf(buffer,"\tgamma = %f",qualityParams.gamma);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"QFactor_factor") == 0)
		{
			qualityParams.QFactor_factor = getKthParameterFloat(value);
			sprintf(buffer,"\tQFactor_factor = %f",qualityParams.QFactor_factor);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"EDFA_Noise_Figure") == 0)
		{
			qualityParams.EDFA_Noise_Figure = getKthParameterFloat(value);
			sprintf(buffer,"\tEDFA_Noise_Figure = %f",qualityParams.EDFA_Noise_Figure);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"EDFA_Gain") == 0)
		{
			qualityParams.EDFA_Gain = getKthParameterFloat(value);
			sprintf(buffer,"\tEDFA_Gain = %f",qualityParams.EDFA_Gain);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"B_w") == 0)
		{
			qualityParams.B_w = getKthParameterFloat(value);
			sprintf(buffer,"\tB_w = %e",qualityParams.B_w);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"usage_update_interval") == 0)
		{
			qualityParams.usage_update_interval = getKthParameterFloat(value);
			sprintf(buffer,"\tusage_update_interval = %f",qualityParams.usage_update_interval);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"beta") == 0)
		{
			qualityParams.beta = getKthParameterFloat(value);
			sprintf(buffer,"\tbeta = %f",qualityParams.beta);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"gui_update_interval") == 0)
		{
			qualityParams.gui_update_interval = getKthParameterInt(value);
			sprintf(buffer,"\tgui_update_interval = %d",qualityParams.gui_update_interval);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"refractive_index") == 0)
		{
			qualityParams.refractive_index = getKthParameterFloat(value);
			sprintf(buffer,"\trefractive_index = %f",qualityParams.refractive_index);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"q_factor_stats") == 0)
		{
			if(getKthParameterInt(value) == 1)
				qualityParams.q_factor_stats = true;
			else if(getKthParameterInt(value) == 0)
				qualityParams.q_factor_stats = false;
			else
			{
				sprintf(buffer,"Unexpected value input for q_factor_stats.");
				threadZero->recordEvent(buffer,true,0);
				qualityParams.q_factor_stats = false;
			}

			sprintf(buffer,"\tq_factor_stats = %d",qualityParams.q_factor_stats);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"detailed_log") == 0)
		{
			if(getKthParameterInt(value) == 1)
				qualityParams.detailed_log = true;
			else if(getKthParameterInt(value) == 0)
				qualityParams.detailed_log = false;
			else
			{
				sprintf(buffer,"Unexpected buffer input for detailed_log.");
				threadZero->recordEvent(buffer,true,0);
				qualityParams.detailed_log = false;
			}

			sprintf(buffer,"\tdetailed_log = %d",qualityParams.detailed_log);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"dest_dist") == 0)
		{
			if(getKthParameterInt(value) == 1)
				qualityParams.dest_dist = UNIFORM;
			else if(getKthParameterInt(value) == 2)
				qualityParams.dest_dist = DISTANCE;
			else if(getKthParameterInt(value) == 3)
				qualityParams.dest_dist = INVERSE_DISTANCE;
			else
			{
				sprintf(buffer,"Unexpected value input for dest_dist.");
				threadZero->recordEvent(buffer,true,0);
				qualityParams.detailed_log = false;
			}

			sprintf(buffer,"\tdest_dist = %d",qualityParams.dest_dist);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"DP_alpha") == 0)
		{
			qualityParams.DP_alpha = getKthParameterFloat(value);
			sprintf(buffer,"\tDP_alpha = %f",qualityParams.DP_alpha);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"ACO_ants") == 0)
		{
			qualityParams.ACO_ants = getKthParameterInt(value);
			sprintf(buffer,"\tACO_ants = %d",qualityParams.ACO_ants);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"ACO_alpha") == 0)
		{
			qualityParams.ACO_alpha = getKthParameterFloat(value);
			sprintf(buffer,"\tACO_alpha = %f",qualityParams.ACO_alpha);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"ACO_beta") == 0)
		{
			qualityParams.ACO_beta = getKthParameterFloat(value);
			sprintf(buffer,"\tACO_beta = %f",qualityParams.ACO_beta);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"ACO_rho") == 0)
		{
			qualityParams.ACO_rho = getKthParameterFloat(value);
			sprintf(buffer,"\tACO_rho = %f",qualityParams.ACO_rho);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"MM_ACO_gamma") == 0)
		{
			qualityParams.MM_ACO_gamma = getKthParameterFloat(value);
			sprintf(buffer,"\tMM_ACO_gamma = %f",qualityParams.MM_ACO_gamma);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"MM_ACO_N_iter") == 0)
		{
			qualityParams.MM_ACO_N_iter = getKthParameterInt(value);
			sprintf(buffer,"\tMM_ACO_N_iter = %d",qualityParams.MM_ACO_N_iter);
			threadZero->recordEvent(buffer,true,0);
		}
		else if(strcmp(param,"MM_ACO_N_reset") == 0)
		{
			qualityParams.MM_ACO_N_reset = getKthParameterInt(value);
			sprintf(buffer,"\tMM_ACO_N_reset = %d",qualityParams.MM_ACO_N_reset);
			threadZero->recordEvent(buffer,true,0);
		}
		else
		{
			threadZero->recordEvent("ERROR: Invalid line in the input file!!!",true,0);
			inFile.close();
			exit(ERROR_QUALITY_INPUT);
		}
	}

	qualityParams.ASE_perEDFA = new float[getNumberOfWavelengths()];

	for(unsigned int w = 0; w < getNumberOfWavelengths(); ++w)
	{
		double F_n = pow(10.0, (qualityParams.EDFA_Noise_Figure / 10.0));
		double h = 6.6260689633e-34;
		double f_c;

		if(w < qualityParams.halfwavelength)
		{
			f_c = qualityParams.fc - (qualityParams.halfwavelength - w) * qualityParams.f_step;
		}
		else if(w > qualityParams.halfwavelength)
		{
			f_c = qualityParams.fc + (w - qualityParams.halfwavelength) * qualityParams.f_step;
		}
		else
		{
			f_c = qualityParams.fc;
		}

		double G = pow(10.0, (qualityParams.EDFA_Gain / 10.0));
		double B_w = qualityParams.B_w;
		double P_ch = qualityParams.channel_power;

		qualityParams.ASE_perEDFA[w] = 2.0 * P_ch * (F_n * h * f_c * (G - 1.0) * B_w);
	}

	inFile.close();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setTopologyParameters
// Description:		Opens up the Topology Paremeters file that was
//					specified by the command line arguement
//
///////////////////////////////////////////////////////////////////
void Thread::setTopologyParameters(const char * f)
{
	char buffer[200];
	sprintf(buffer,"Reading Topology Parameters from %s file.",f);
	threadZero->recordEvent(buffer,false,0);

#ifdef RUN_GUI
	strcpy(topoFile,f);
#endif

	ifstream inFile(f);

	char *param;
		
	numberOfRouters = 0;
	numberOfEdges = 0;

	while(inFile.getline(buffer,199))
	{
		param = strtok(buffer,"=");

		if(strcmp(param,"Router") == 0)
		{
			Router* r = new Router;

			r->setIndex(numberOfRouters);

#ifdef RUN_GUI
			r->setXPercent(atoi(strtok(NULL,",")));
			r->setYPercent(atoi(strtok(NULL,",")));
#endif
			addRouter(r);

			++numberOfRouters;
		}
		else if(strcmp(param,"Edge") == 0)
		{
			unsigned short int from = atoi(strtok(NULL,","));
			unsigned short int to = atoi(strtok(NULL,","));
			unsigned short int spans = atoi(strtok(NULL,","));

			Edge* e1 = new Edge(from,to,spans);
			Edge* e2 = new Edge(to,from,spans);

			getRouterAt(from)->addEdge(e1);
			getRouterAt(to)->addEdge(e2);

			numberOfEdges += 2;
		}
		else
		{
			threadZero->recordEvent("ERROR: Invalid line in the input file!!!",true,0);
			inFile.close();
			exit(ERROR_TOPOLOGY_INPUT_EDGES);
		}
	}

	sprintf(buffer,"\tCreated %d routers.",numberOfRouters);
	threadZero->recordEvent(buffer,false,0);

	sprintf(buffer,"\tCreated %d edges.",numberOfEdges);
	threadZero->recordEvent(buffer,false,0);

	inFile.close();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setWorkstationParameters
// Description:		Opens up the Workstation Paremeters file that was
//					specified by the command line arguement
//
///////////////////////////////////////////////////////////////////
void Thread::setWorkstationParameters(const char * f)
{
	char buffer[200];
	sprintf(buffer,"Reading Workstation Parameters from %s file.",f);
	threadZero->recordEvent(buffer,false,0);

#ifdef RUN_GUI
	strcpy(wkstFile,f);
#endif

	ifstream inFile(f);

	inFile.getline(buffer,199);

	char *param;
	char *parent;
	char *traffic;
		
	param = strtok(buffer,"=");
	parent = strtok(NULL," ,\t");

	if(strcmp(param,"NumberOfWorkstations") == 0)
		numberOfWorkstations = atoi(parent);
	else
	{
		threadZero->recordEvent("ERROR: Invalid line in the input file!!!",true,0);
		exit(ERROR_WORKSTATION_INPUT_QUANTITY);
	}

	//Add the specific configuration of workstations, if specified.
	while(inFile.getline(buffer,199))
	{
		Workstation* w = new Workstation();

		param = strtok(buffer,"=");
		parent = strtok(NULL," ,\t");
		traffic = strtok(NULL," ,\t");

		if(strcmp(param,"Workstation") == 0)
		{
			w->setParentRouterIndex(atoi(parent));
		}
		else
		{
			threadZero->recordEvent("ERROR: Invalid line in the input file!!!",true,0);
			inFile.close();
			exit(ERROR_WORKSTATION_INPUT_PARENT);
		}

		addWorkstation(w);
	}

	//If workstations are not specified, then just uniformly distribute them amongst the routers.
	for(unsigned short int n = static_cast<unsigned short int>(workstations.size()); n < numberOfWorkstations; ++n)
	{
		Workstation* w = new Workstation();

		w->setParentRouterIndex(n % getNumberOfRouters());

		addWorkstation(w);
	}

	inFile.close();

	sprintf(buffer,"\tCreated %d workstations.",numberOfWorkstations);
	threadZero->recordEvent(buffer,false,0);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setAlgorithmParameters
// Description:		Opens up the Quality Paremeters file that was
//					specified by the command line arguement
//
///////////////////////////////////////////////////////////////////
void Thread::setAlgorithmParameters(const char * f, unsigned short int iterationCount)
{
	RoutingAlgorithmNames[SHORTEST_PATH] = new std::string("SP");
	RoutingAlgorithmNames[PABR] = new std::string("PABR");
	RoutingAlgorithmNames[LORA] = new std::string("LORA");
	RoutingAlgorithmNames[IMPAIRMENT_AWARE] = new std::string("IA");
	RoutingAlgorithmNames[Q_MEASUREMENT] = new std::string("QM");
	RoutingAlgorithmNames[ADAPTIVE_QoS] = new std::string("AQoS");
	RoutingAlgorithmNames[DYNAMIC_PROGRAMMING] = new std::string("DP");
	RoutingAlgorithmNames[ACO] = new std::string("ACO");
	RoutingAlgorithmNames[MAX_MIN_ACO] = new std::string("MM-ACO");

	WavelengthAlgorithmNames[FIRST_FIT] = new std::string("FF");
	WavelengthAlgorithmNames[FIRST_FIT_ORDERED] = new std::string("FFwO");
	WavelengthAlgorithmNames[BEST_FIT] = new std::string("BF");
	WavelengthAlgorithmNames[RANDOM_FIT] = new std::string("RP");
	WavelengthAlgorithmNames[QUAL_FIRST_FIT] = new std::string("Q-FF");
	WavelengthAlgorithmNames[QUAL_FIRST_FIT_ORDERED] = new std::string("Q-FFwO");
	WavelengthAlgorithmNames[QUAL_RANDOM_FIT] = new std::string("Q-RP");
	WavelengthAlgorithmNames[LEAST_QUALITY] = new std::string("LQ");
	WavelengthAlgorithmNames[MOST_QUALITY] = new std::string("MQ");
	WavelengthAlgorithmNames[MOST_USED] = new std::string("MU");
	WavelengthAlgorithmNames[QUAL_MOST_USED] = new std::string("Q-MU");

	ProbeStyleNames[SINGLE] = new std::string("SINGLE");
	ProbeStyleNames[SERIAL] = new std::string("SERIAL");
	ProbeStyleNames[PARALLEL] = new std::string("PARALLEL");

	char buffer[200];
	sprintf(buffer,"Reading Algorithm Parameters from %s file.",f);
	threadZero->recordEvent(buffer,false,0);

	ifstream inFile(f);

	while(inFile.getline(buffer,199))
	{
		char *ra = strtok(buffer,",") + strlen("RA=") * sizeof(char);
		char *wa = strtok(NULL,",") + strlen("WA=") * sizeof(char);
		char *ps = strtok(NULL,",") + strlen("PS=") * sizeof(char);
		char *qa = strtok(NULL,",") + strlen("QA=") * sizeof(char);
		char *run = strtok(NULL,",") + strlen("RUN=") * sizeof(char);

		if(strcmp(run,"1") == 0)
		{
			CurrentRoutingAlgorithm = NUMBER_OF_ROUTING_ALGORITHMS;
			CurrentWavelengthAlgorithm = NUMBER_OF_WAVELENGTH_ALGORITHMS;
			CurrentProbeStyle = NUMBER_OF_PROBE_STYLES;
			CurrentQualityAware = true;

			for(unsigned short int r = 0; r < NUMBER_OF_ROUTING_ALGORITHMS; ++r)
			{
				if(strcmp(ra,RoutingAlgorithmNames[r]->c_str()) == 0)
				{
					CurrentRoutingAlgorithm = static_cast<RoutingAlgorithm>(r);
					break;
				}
			}

			for(unsigned short int w = 0; w < NUMBER_OF_WAVELENGTH_ALGORITHMS; ++w)
			{
				if(strcmp(wa,WavelengthAlgorithmNames[w]->c_str()) == 0)
				{
					CurrentWavelengthAlgorithm = static_cast<WavelengthAlgorithm>(w);
					break;
				}
			}

			for(unsigned short int p = 0; p < NUMBER_OF_PROBE_STYLES; ++p)
			{
				if(strcmp(ps,ProbeStyleNames[p]->c_str()) == 0)
				{
					CurrentProbeStyle = static_cast<ProbeStyle>(p);
					break;
				}
			}

			if(strcmp(qa,"1") == 0)
			{
				CurrentQualityAware = true;
			}
			else
			{
				CurrentQualityAware = false;
			}

			if(CurrentRoutingAlgorithm == NUMBER_OF_ROUTING_ALGORITHMS ||
			   CurrentWavelengthAlgorithm == NUMBER_OF_WAVELENGTH_ALGORITHMS ||
			   CurrentProbeStyle == NUMBER_OF_PROBE_STYLES)
			{
				threadZero->recordEvent("ERROR: Invalid line in the input file!!!",true,0);
				exit(ERROR_ALGORITHM_INPUT);
			}
			else
			{
				unsigned short int iterationWorkstationDelta = static_cast<unsigned short int>(float(1.0) / float(iterationCount) *
					float(threadZero->getNumberOfWorkstations()));

				for(unsigned short int i = 0; i < iterationCount; ++i)
				{
					AlgorithmToRun* ap = new AlgorithmToRun;

					ap->ra = CurrentRoutingAlgorithm;
					ap->wa = CurrentWavelengthAlgorithm;
					ap->ps = CurrentProbeStyle;
					ap->qa = CurrentQualityAware;
					ap->workstations = (i + 1) * iterationWorkstationDelta;

					algParams.push_back(ap);
				}
			}
		}
	}

	inFile.close();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	update_link_usage
// Description:		Updates the link usage to be used for the 
//					LORA and PABR algorithms
//
///////////////////////////////////////////////////////////////////
void Thread::update_link_usage()
{
	time_t start;
	time_t end;

	time(&start);

	if(getGlobalTime() >= TEN_HOURS)
	{
		return;
	}

	for(unsigned short int r = 0; r < getNumberOfRouters(); ++r)
	{
		getRouterAt(r)->updateUsage();
	}

	Event* event = new Event();

	event->e_type = UPDATE_USAGE;
	event->e_time = getGlobalTime() + threadZero->getQualityParams().usage_update_interval;
	event->e_data = 0;

	queue->addEvent(*event);

	delete event;

	time(&end);
	stats.raRunTime += difftime(end,start);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getKShortestPaths
// Description:		Calculates the k shortest paths based upon
//					the current routing algorithm.
//
///////////////////////////////////////////////////////////////////
kShortestPathReturn* Thread::getKShortestPaths(ConnectionRequestEvent *cre, unsigned short int probesToSend)
{
	kShortestPathReturn* kPath;

	time_t start;
	time_t end;

	time(&start);

	if(CurrentRoutingAlgorithm == PABR)
	{
		kPath = threadZero->getResourceManager()->
			calculate_PAR_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == SHORTEST_PATH)
	{
		kPath = threadZero->getResourceManager()->
			calculate_SP_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == LORA)
	{
		kPath = threadZero->getResourceManager()->
			calculate_LORA_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE)
	{
		kPath = threadZero->getResourceManager()->calculate_IA_path(
			cre->sourceRouterIndex,cre->destinationRouterIndex,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == Q_MEASUREMENT)
	{
		kPath = threadZero->getResourceManager()->
			calculate_QM_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == ADAPTIVE_QoS)
	{
		if(getRouterAt(cre->sourceRouterIndex)->getQualityFailures() >=
		getRouterAt(cre->sourceRouterIndex)->getWaveFailures())
		{
			kPath = threadZero->getResourceManager()->
				calculate_QM_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
				probesToSend,controllerIndex);
		}
		else
		{
			kPath = threadZero->getResourceManager()->
				calculate_AQoS_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
				probesToSend,controllerIndex);
		}
	}
	else if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
	{
		kPath = threadZero->getResourceManager()->
			calculate_DP_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == ACO)
	{
		kPath = threadZero->getResourceManager()->
			calculate_ACO_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else if(CurrentRoutingAlgorithm == MAX_MIN_ACO)
	{
		kPath = threadZero->getResourceManager()->
			calculate_MM_ACO_path(cre->sourceRouterIndex,cre->destinationRouterIndex,
			probesToSend,controllerIndex);
	}
	else
	{
		char buffer[200];
		sprintf(buffer,"Invalid values for CurrentAlgorithm (%d) and CurrentWavelengthAlgorithm (%d)\n",
			CurrentRoutingAlgorithm,CurrentWavelengthAlgorithm);
		exit(ERROR_ALGORITHM_INPUT);
	}

	time(&end);

	stats.raRunTime += difftime(end,start);

	return kPath;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	calcProbesToSend
// Description:		Calculates which probes to send based upon the
//					kPaths and the Routing/Wavelength Algorithms
//
///////////////////////////////////////////////////////////////////
CreateConnectionProbeEvent** Thread::calcProbesToSend(ConnectionRequestEvent *cre, kShortestPathReturn *kPath, unsigned short int &probesToSend, unsigned short int &probeStart, unsigned short int &probesSkipped)
{
	CreateConnectionProbeEvent** probesList = 0;

	//We have to handle some algorithms differently because it uses a forward reservation
	//scheme, while the others use a backward reservation scheme.
	if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE || CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
	{
		unsigned int probesTotal = 0;
		int probeFirst = -1;

		if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE)
		{
			for(unsigned short int w = probesSkipped; w < threadZero->getNumberOfWavelengths(); ++w)
			{
				if(kPath->pathcost[w] != std::numeric_limits<float>::infinity())
				{
					++probesTotal;

					if(probeFirst == -1)
						probeFirst = w;
				}
			}
		}
		else if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
		{
			unsigned int max_probes = threadZero->getQualityParams().max_probes;

			if(CurrentProbeStyle == SINGLE)
				max_probes = 1;

			for(unsigned short int w = probesSkipped; w < max_probes; ++w)
			{
				if(kPath->pathcost[w] != std::numeric_limits<float>::infinity())
				{
					++probesTotal;

					if(probeFirst == -1)
						probeFirst = w;
				}
			}
		}

		if(probeFirst == -1)
		{
			probesToSend = 0;
		}
		else
		{
			if(CurrentProbeStyle == PARALLEL)
			{
				probesToSend = probesTotal;
				
				probesList = new CreateConnectionProbeEvent*[probesToSend];
			}
			else if(CurrentProbeStyle == SINGLE || CurrentProbeStyle == SERIAL)
			{
				probesToSend = 1;

				if(CurrentWavelengthAlgorithm == FIRST_FIT || CurrentWavelengthAlgorithm == QUAL_FIRST_FIT)
				{
					probeStart = probeFirst;
				}
				else if(CurrentWavelengthAlgorithm == BEST_FIT)
				{
					time_t start;
					time_t end;

					time(&start);

					float minCost = std::numeric_limits<float>::infinity();

					for(unsigned short int w = 0; w < threadZero->getNumberOfWavelengths(); ++w)
					{
						if(kPath->pathcost[w] < minCost)
						{
							minCost = kPath->pathcost[w];
							probeStart = w;
						}
					}

					if(minCost == std::numeric_limits<float>::infinity())
						probesToSend = 0;

					time(&end);

					stats.raRunTime += difftime(end,start);
				}
				else
				{
					threadZero->recordEvent("ERROR: Invalid Wavelength Algorithm for Impairment Aware.",true,controllerIndex);
					exit(ERROR_WAVELENGTH_ALGORITHM_IA);
				}
			}

		}			
	}
	else if(CurrentProbeStyle == SINGLE)
	{
		probeStart = 0;

		if(kPath->pathcost[0] != std::numeric_limits<float>::infinity())
		{
			probesToSend = 1;
		}
		else
		{
			probesToSend = 0;
		}
	}
	else if(CurrentProbeStyle == SERIAL)
	{
		probesToSend = 0;

		for(unsigned int a = probesSkipped; a < threadZero->getQualityParams().max_probes; ++a)
		{
			if(kPath->pathcost[a] != std::numeric_limits<float>::infinity())
			{
				probeStart = a;
				probesToSend = 1;
				break;
			}
		}
	}
	else if(CurrentProbeStyle == PARALLEL)
	{
		probeStart = 0;
		probesToSend = 0;

		for(unsigned int p = 0; p < threadZero->getQualityParams().max_probes; ++p)
		{
			if(kPath->pathcost[p] != std::numeric_limits<float>::infinity())
				++probesToSend;
		}

		if(probesToSend > 0)
		{
			probesList = new CreateConnectionProbeEvent*[probesToSend];
		}
	}

	return probesList;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	sendProbes
// Description:		Generates the probe events based upon the
//					input parameters.
//
///////////////////////////////////////////////////////////////////
void Thread::sendProbes(ConnectionRequestEvent *cre, kShortestPathReturn *kPath, CreateConnectionProbeEvent** probesList, unsigned short int probesToSend, unsigned short int probeStart, unsigned short int probesSkipped)
{
	if(probesToSend == 0)
	{
		if(CurrentProbeStyle == SERIAL)
		{
			if(cre->wavelength >= 0 && CurrentRoutingAlgorithm != DYNAMIC_PROGRAMMING)
			{
				++stats.QualityFailures;
			}
			else
			{
				++stats.NoPathFailures;
			}
		}
		else
		{
			++stats.NoPathFailures;
		}
	}
	else
	{
		unsigned int newProbesSkipped = 0;

		for(unsigned short int p = probeStart; p < probeStart + newProbesSkipped + probesToSend; ++p)
		{
			if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE || CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
			{
				while(kPath->pathcost[p] == std::numeric_limits<float>::infinity())
				{
					++newProbesSkipped;
					++p;
				}
			}

			CreateConnectionProbeEvent* probe = new CreateConnectionProbeEvent();

			probe->sourceRouterIndex = cre->sourceRouterIndex;
			probe->destinationRouterIndex = cre->destinationRouterIndex;
			probe->connectionDuration = cre->connectionDuration;
			probe->requestBeginTime = cre->requestBeginTime;
			probe->numberOfHops = 0;
			probe->connectionLength = kPath->pathlen[p] - 1;
			probe->connectionPath = new Edge*[probe->connectionLength];
			probe->session = cre->session;
			probe->qualityFail = cre->qualityFail;

			if(CurrentProbeStyle == SERIAL)
				probe->sequence = cre->sequence;
			else if(CurrentProbeStyle == PARALLEL)
				probe->sequence = cre->sequence++;
			else if(CurrentProbeStyle == SINGLE)
				probe->sequence = 0;

			probe->atDestination = false;
			probe->decisionTime = 0.0;
			probe->kPaths = kPath;
			
			if(CurrentProbeStyle == PARALLEL)
			{
				probe->probes = probesList;
				probesList[probe->sequence] = probe;
			}

			if(CurrentProbeStyle == SERIAL)
			{
				if(cre->max_sequence == 0)
				{
					if(CurrentRoutingAlgorithm != IMPAIRMENT_AWARE && CurrentRoutingAlgorithm != DYNAMIC_PROGRAMMING)
					{
						for(unsigned int a = 0; a < threadZero->getQualityParams().max_probes; ++a)
						{
							if(kPath->pathcost[a] != std::numeric_limits<float>::infinity())
								++cre->max_sequence;
						}
					}
					else if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE)
					{
						for(unsigned int a = 0; a < threadZero->getNumberOfWavelengths(); ++a)
						{
							if(kPath->pathcost[a] != std::numeric_limits<float>::infinity())
								++cre->max_sequence;
						}
					}
					else if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
					{
						for(unsigned int a = 0; a < threadZero->getQualityParams().max_probes; ++a)
						{
							if(kPath->pathcost[a] != std::numeric_limits<float>::infinity())
								++cre->max_sequence;
						}
					}
				}

				probe->max_sequence = cre->max_sequence;
			}
			else
			{
				probe->max_sequence = probesToSend;
			}

			if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE)
			{
				probe->wavelength = p;
			}
			else if(CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
			{
				probe->wavelength = static_cast<short>(kPath->pathcost[p]);
			}
			else
			{
				probe->wavelength = 0;
			}

			for(unsigned short int r = 0; r < probe->connectionLength; ++r)
			{
				probe->connectionPath[r] = getRouterAt(kPath->pathinfo[p * (threadZero->getNumberOfRouters() - 1) + r])->
					getEdgeByDestination(kPath->pathinfo[p * (threadZero->getNumberOfRouters() - 1) + r + 1]);
			}

			if(CurrentRoutingAlgorithm != SHORTEST_PATH)
			{
				kPath->pathcost[p] = std::numeric_limits<float>::infinity();
				kPath->pathlen[p] = std::numeric_limits<int>::infinity();
			}

			Event* event = new Event();

			event->e_type = CREATE_CONNECTION_PROBE;
			event->e_time = getGlobalTime() + calculateDelay(probe->connectionPath[0]->getNumberOfSpans());
			event->e_data = probe;

			queue->addEvent(*event);

			delete event;

			if(CurrentRoutingAlgorithm != IMPAIRMENT_AWARE && CurrentRoutingAlgorithm != DYNAMIC_PROGRAMMING)
			{
				++stats.ProbeSentCount;
			}
		}
	}

	if((CurrentProbeStyle != SERIAL && CurrentRoutingAlgorithm != SHORTEST_PATH) || probesToSend == 0)
	{
		delete[] kPath->pathcost;
		delete[] kPath->pathinfo;
		delete[] kPath->pathlen;

		delete kPath;
	}
}

#ifdef RUN_GUI
///////////////////////////////////////////////////////////////////
//
// Function Name:	update_gui
// Description:		Updates the gui with the most recent usage levels
//
///////////////////////////////////////////////////////////////////
void Thread::update_gui()
{
	if(getGlobalTime() >= TEN_HOURS)
	{
		return;
	}

	for(unsigned short int r = 0; r < getNumberOfRouters(); ++r)
	{
		getRouterAt(r)->updateGUI();
	}

	Event* event = new Event();

	event->e_type = UPDATE_GUI;
	event->e_time = getGlobalTime() + threadZero->getQualityParams().gui_update_interval;
	event->e_data = 0;

	queue->addEvent(*event);

	delete event;
}
#endif

///////////////////////////////////////////////////////////////////
//
// Function Name:	calculateDelay
// Description:		Calculates the propogation delay on the optical
//					link.
//
///////////////////////////////////////////////////////////////////
double Thread::calculateDelay(unsigned short int spans)
{
	if(CurrentRoutingAlgorithm == IMPAIRMENT_AWARE || CurrentRoutingAlgorithm == DYNAMIC_PROGRAMMING)
	{
		return 0.0;
	}
	else
	{
		return double(spans * threadZero->getQualityParams().L * 1000) / 
			(SPEED_OF_LIGHT / threadZero->getQualityParams().refractive_index);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	sendResponse
// Description:		Determines if the router needs to respond to
//					this probe. (i.e. Either no response has been sent
//					yet or the sent responses were declined.)
//
///////////////////////////////////////////////////////////////////
bool Thread::sendResponse(CreateConnectionProbeEvent* probe)
{
	for(unsigned int p = 0; p < probe->max_sequence; ++p)
	{
		if(p != probe->sequence)
		{
			if(probe->probes[p]->atDestination == true)
			{
				if(probe->probes[p]->decisionTime != 0 &&
				   probe->probes[p]->wavelength >= 0)
				{
					return false;
				}
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	clearResponses
// Description:		Clears the pending responses as the connection has
//					been setup succesfully.
//
///////////////////////////////////////////////////////////////////
void Thread::clearResponses(CreateConnectionProbeEvent* probe)
{
	for(unsigned int p = 0; p < probe->max_sequence; ++p)
	{
		if(p != probe->sequence)
		{
			delete[] probe->probes[p]->connectionPath;
			delete probe->probes[p];
		}
	}

	delete[] probe->probes;
	delete probe;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	moreProbes
// Description:		Returns true if more probes will be arriving
//					at the destination.
//
///////////////////////////////////////////////////////////////////
bool Thread::moreProbes(CreateConnectionProbeEvent* probe)
{
	if(CurrentProbeStyle != PARALLEL)
		return false;

	for(unsigned int p = 0; p < probe->max_sequence; ++p)
	{
		if(p != probe->sequence)
		{
			if(probe->probes[p]->atDestination == false)
			{
				return true;
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setMinDuration
// Description:		Sets the minimum duration of a connection to
//					three times the maximum minumum distance between
//					two difference routers.
//
///////////////////////////////////////////////////////////////////
void Thread::setMinDuration(unsigned short int spans)
{
	//Routing algorithm should be a non centralized algorithm to ensure
	//the delay is not zero.
	RoutingAlgorithm origAlg = CurrentRoutingAlgorithm;
	CurrentRoutingAlgorithm = SHORTEST_PATH;

	minDuration = 3.0 * calculateDelay(spans);

	CurrentRoutingAlgorithm = origAlg;

	maxSpans = threadZero->getMaxSpans();
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	setQFactorMin
// Description:		Sets the value of the Q-factor threshold to
//					within a specified factor of the max min
//
///////////////////////////////////////////////////////////////////
void Thread::setQFactorMin(unsigned short int spans)
{
	qualityParams.TH_Q = static_cast<float>(qualityParams.QFactor_factor * 10.0 * 
		log10(qualityParams.channel_power / sqrt(qualityParams.ASE_perEDFA[qualityParams.halfwavelength] * float(spans))));

	double Q = qualityParams.TH_Q;
	maxSpans = spans;

	while(Q >= qualityParams.TH_Q)
	{
		++maxSpans;

		Q = 10.0 * log10(qualityParams.channel_power / sqrt(qualityParams.ASE_perEDFA[qualityParams.halfwavelength] * float(maxSpans+1)));
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	otherResponse
// Description:		Determines if another probe stored could be used 
//					to responsd to this probe.
//
///////////////////////////////////////////////////////////////////
int Thread::otherResponse(CreateConnectionProbeEvent* probe)
{
	if(CurrentProbeStyle != PARALLEL)
		return  -1;

	for(unsigned int p = 0; p < probe->max_sequence; ++p)
	{
		if(p != probe->sequence)
		{
			if(probe->probes[p]->atDestination == true)
			{
				if(probe->probes[p]->decisionTime == 0.0)
				{
					return p;
				}
			}
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateQMDegredation
// Description:		Updates the link costs when a connection is either added
//					or dropped.
//
///////////////////////////////////////////////////////////////////
void Thread::updateQMDegredation(Edge **connectionPath, unsigned short int connectionLength, unsigned int wavelength)
{
	time_t start;
	time_t end;

	time(&start);

	for(unsigned short int p = 0; p < connectionLength; ++p)
	{
		connectionPath[p]->updateQMDegredation(controllerIndex, wavelength);
	}

	time(&end);
	stats.raRunTime += difftime(end,start);
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	updateQFactorStats
// Description:		Updates the q-factor stats when a connection is either added
//					or dropped.
//
///////////////////////////////////////////////////////////////////
void Thread::updateQFactorStats(Edge **connectionPath, unsigned short int connectionLength, unsigned int wavelength)
{
	for(unsigned short int p = 0; p < connectionLength ; ++p)
	{
		connectionPath[p]->updateQFactorStats(controllerIndex, wavelength);
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	getBeta
// Description:		Returns the value of beta for threadZero
//
///////////////////////////////////////////////////////////////////
float Thread::getBeta()
{
	return threadZero->getQualityParams().beta;
}

#ifdef RUN_GUI
///////////////////////////////////////////////////////////////////
//
// Function Name:	detailScreen()
// Description:		displays configuration info as well as overall
//					results for the given simulation.
//
///////////////////////////////////////////////////////////////////
void Thread::detailScreen()
{
	int startX = ceil((double)SCREEN_W/2.0 - (double)detailinfo->w/2.0);
	int startY = 100;
	int numtopaint;
	int black = makecol(0,0,0);

	masked_blit(detailinfo,popup,0,0,startX,startY,SCREEN_W,SCREEN_H);
	textprintf_right_ex(popup,font,startX+270,startY+35,black,-1,"%s",topology);
	textprintf_right_ex(popup,font,startX+270,startY+52,black,-1,"%s",routing);
	textprintf_right_ex(popup,font,startX+270,startY+69,black,-1,"%s",wavelength);
	textprintf_right_ex(popup,font,startX+270,startY+86,black,-1,"%s",probing);
	if(qualityAware)
		textprintf_right_ex(popup,font,startX+270,startY+103,black,-1,"Yes");
	else
		textprintf_right_ex(popup,font,startX+270,startY+103,black,-1,"No");
	textprintf_right_ex(popup,font,startX+270,startY+130,black,-1,"%d",CurrentActiveWorkstations);
	textprintf_right_ex(popup,font,startX+270,startY+147,black,-1,"%d",numOfWavelengths);

	textprintf_right_ex(popup,font,startX+270,startY+202,black,-1,"%2.3f",overallBlocking);
	textprintf_right_ex(popup,font,startX+270,startY+219,black,-1,"%2.3f",collisions);
	textprintf_right_ex(popup,font,startX+270,startY+238,black,-1,"%2.3f",badquality);
	textprintf_right_ex(popup,font,startX+270,startY+255,black,-1,"%2.3f",nonresource);
	textprintf_right_ex(popup,font,startX+270,startY+285,black,-1,"%2.3f",avgprobesperrequest);
	textprintf_right_ex(popup,font,startX+270,startY+303,black,-1,"%2.3f",avgrequestdelaytime);
	textprintf_right_ex(popup,font,startX+270,startY+321,black,-1,"%2.3f",avgconnhopcount);
	textprintf_right_ex(popup,font,startX+270,startY+339,black,-1,"%2.3f",avgconnspancount);
	textprintf_right_ex(popup,font,startX+270,startY+357,black,-1,"%2.3e",avgASEnoise);
	textprintf_right_ex(popup,font,startX+270,startY+375,black,-1,"%2.3e",avgFWMnoise);
	textprintf_right_ex(popup,font,startX+270,startY+393,black,-1,"%2.3e",avgXPMnoise);

	while(!(mouse_b & 1))
	{
	}
}

///////////////////////////////////////////////////////////////////
//
// Function Name:	saveThread
// Description:		Creates appropriately-named .bin file and stores
//					all data needed to explore history of thread later.
//
///////////////////////////////////////////////////////////////////
void Thread::saveThread(char* dir)
{
	char allName[75];
	char fileName[40];
	ofstream myFile;

	//routing algorithm - wavelength algorithm - probe style - num workstations - quality aware (1 if true) - maxprobes
	sprintf(fileName,"%s-%s-%s-%d-%d-%d",threadZero->getRoutingAlgorithmName(CurrentRoutingAlgorithm)->c_str(),
		threadZero->getWavelengthAlgorithmName(CurrentWavelengthAlgorithm)->c_str(), threadZero->getProbeStyleName(CurrentProbeStyle)->c_str(),
		getCurrentActiveWorkstations(), (int)getCurrentQualityAware(), threadZero->getQualityParams().max_probes);

	sprintf(allName,"%s/index.idx",dir);
	myFile.open(allName, std::ios::out | std::ios::app | std::ios::binary);
	myFile << fileName << "|\n";
	myFile.close();

	sprintf(allName,"%s/%s.thd",dir,fileName);
	
	myFile.open(allName, std::ios::out | std::ios::app | std::ios::binary);
	myFile << topology <<"|\n";
	myFile << routing << "|\n";
	myFile << wavelength << "|\n";
	myFile << probing << "|\n";//need
	myFile << (int)qualityAware << "|\n";//need
	
	myFile << overallBlocking << "|\n";
	myFile << collisions << "|\n";
	myFile << badquality << "|\n";
	myFile << nonresource << "|\n";
	myFile << avgprobesperrequest << "|\n";
	myFile << avgrequestdelaytime << "|\n";
	myFile << avgconnhopcount << "|\n";
	myFile << avgconnspancount << "|\n";
	myFile << avgASEnoise << "|\n";
	myFile << avgFWMnoise << "|\n";
	myFile << avgXPMnoise << "|\n";

	myFile << threadZero->getNumberOfWavelengths() << "|\n";
	myFile << routers[0]->getEdgeByIndex(0)->getUsageNums() << "|\n";
	myFile << getCurrentActiveWorkstations() << "|\n";
	myFile.close();

	for(unsigned short int r = 0; r < routers.size(); ++r)
		routers[r]->saveData(allName);
}
#endif
