#include "QYShortestPath.h"
#include "QYKShortestPaths.h"
#include "QYInclude.h"

using namespace std;
using namespace asu_emit_qyan;

#ifdef __GNUC__ 
	extern "C" void calc_k_shortest_paths(const kShortestPathParms &params, kShortestPathReturn* retVal);
#else
	extern "C" __declspec(dllexport) void calc_k_shortest_paths(const kShortestPathParms &params, kShortestPathReturn* retVal);
#endif

void copyResults(vector<CQYDirectedPath*> &topK_shortest_paths, const kShortestPathParms &params, kShortestPathReturn* retVal, bool dijkstra);

void calc_k_shortest_paths(const kShortestPathParms &params, kShortestPathReturn* retVal)
{
	CQYDirectedGraph dg(params);

	if(params.k_paths == 1)
	{
		vector<CQYDirectedPath*> topK_shortest_paths;

		CQYShortestPath sp(dg);
		
		topK_shortest_paths.push_back(sp.GetShortestPath(params.src_node, params.dest_node));

		copyResults(topK_shortest_paths, params, retVal, true);

		if(topK_shortest_paths.size() > 0)
			delete topK_shortest_paths[0];
	}
	else
	{
		CQYKShortestPaths ksp(dg, params.src_node, params.dest_node, params.k_paths);

		vector<CQYDirectedPath*> topK_shortest_paths = ksp.GetTopKShortestPaths();

		copyResults(topK_shortest_paths, params, retVal, false);
	}

	return;
}

void copyResults(vector<CQYDirectedPath*> &topK_shortest_paths, const kShortestPathParms &params, kShortestPathReturn* retVal, bool dijkstra)
{
	for(vector<CQYDirectedPath*>::iterator iter = topK_shortest_paths.begin(); iter != topK_shortest_paths.end(); ++iter)
	{
		if((*iter)->GetLength() <= 0 || (*iter)->GetLength() >= params.total_nodes)
		{
			if(dijkstra == true)
				delete *iter;

			topK_shortest_paths.erase(iter);
			
			if(topK_shortest_paths.size() == 0)
				break;
			else
				iter = topK_shortest_paths.begin();
		}
	}

	for(unsigned short int a = 0; a < topK_shortest_paths.size(); ++a)
	{
		retVal->pathcost[a] = float(topK_shortest_paths[a]->GetCost());
		retVal->pathlen[a] = topK_shortest_paths[a]->GetLength();

		for(unsigned short int b = 0; b < retVal->pathlen[a]; ++b)
		{
			retVal->pathinfo[a * (params.total_nodes - 1) + b] = topK_shortest_paths[a]->GetVertexList()[b];
		}
	}

	for(unsigned short int d = topK_shortest_paths.size(); d < params.k_paths; ++d)
	{
		retVal->pathcost[d] = std::numeric_limits<float>::infinity();
		retVal->pathlen[d] = std::numeric_limits<int>::infinity();
	}
}
