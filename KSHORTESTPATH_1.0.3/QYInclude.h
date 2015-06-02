#ifndef _QYINCLUDE_H_
#define _QYINCLUDE_H_

struct kShortestPathEdges {
	unsigned short int src_node;
	unsigned short int dest_node;
	float edge_cost;
};

struct kShortestPathParms {
	unsigned short int src_node;
	unsigned short int dest_node;
	unsigned short int k_paths;
	unsigned short int total_nodes;
	unsigned short int total_edges;
	kShortestPathEdges *edge_list;	
};

struct kShortestPathReturn {
	unsigned short int *pathinfo;
	float *pathcost;
	unsigned short int *pathlen;
};

#endif
