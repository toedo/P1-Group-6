#include <stdio.h>
#include <stdlib.h>

//A Single Node
typedef struct node {
    unsigned int node_id;
    double longitude;
    double latitude;
} Node;

//A Single Edge
typedef struct edge {
    unsigned int from_node;
    unsigned int to_node;
    unsigned int edge_id;
    unsigned int reverse_edge_id;
    double length_m;

} Edge;

//Ordered arrays for the taken path
typedef struct path {
    int edgesTraversed;
    Edge* edges;

    int nodesTraversed;
    Node* nodes;
} Path;

//Helper function to load a single node
Node LoadNodeFromCSV(FILE* nodesFile);
//Load all nodes from file
void LoadNodesFromCSV(FILE* nodesCSV, Node* nodes);

//Helper function to load a single edge
Edge LoadEdgeFromCSV(FILE* edgesFile);
//Load all edges from file
void LoadEdgesFromCSV(FILE* edgesCSV, Edge* edges);

