#ifndef CSV_IMPORTER_H
#define CSV_IMPORTER_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct node Node;
typedef struct edge Edge;
typedef struct graph Graph;
typedef struct path Path;

//A Single Node, must be allocated with calloc
typedef struct node {
    int nodeID;
    //Position data in lon/lat
    double longitude;
    double latitude;
    //UTM Zone 32 - meter data. Is good for calculating the distance in meters between two arbitrary nodes
    //Can be used for heuristics or distance estimation.
    double UTMx_m;
    double UTMy_m;
    //OSM Data
    int OSMNodeID;
    //Array info
    int amountOfOutgoingEdges;
    Edge* outgoingEdges;
} Node;

//A Single Edge, must be allocated with calloc
typedef struct edge {
    //The actual graph / path data
    int edgeID;
    int fromNodeID;
    Node* fromNode;
    int toNodeID;
    Node* toNode;

    //This is mostly used to calculate the "cost" of traversing an edge
    double lengthM;
    double speedKMH;
    double travelTimeS;
    //Road data, mostly for display
    int serviceClassPriority;
    int routeNumber;
    //pointer to string
    char* roadName;
    //pointer to string
    char* roadType;
    //No reverse edge means it is oneway, this is also relevant for path finding
    bool isOneway;
    int reverseEdgeID;
    Edge* reverseEdge;

    //These two *probably* aren't necessary
    char* linestringGeometry;
    //This is on the form "(int)_(int)" where the two ints are two OSM node ids.
    char* OSMEdgeID;

} Edge;

//TODO: Finish graph struct
typedef struct graph {
    Node* head;
    //References to nodes and edges arrays?
    int edgeCount;
    Edge* edges;
    int nodeCount;
    Node* nodes;
}Graph;

//Ordered arrays for the taken path
typedef struct path {
    int edgesTraversed;
    Edge* edges;

    int nodesTraversed;
    Node* nodes;
} Path;

//Loads nodes from CSV document
//Should be ran before LoadEdgesFromCSV()
Node* LoadNodesFromCSV(FILE* nodesCSV, Node* nodes, int size);
//Loads edges into an array from a CSV document
//Do not run before LoadNodesFromCSV()
Edge* LoadEdgesFromCSV(FILE* edgesCSV, Edge* edges, int size);
//returns 0 for successful linking of node and edge pointers
int LinkNodesAndEdges(Edge* edges, int edgeCount, Node* nodes, int nodeCount);
//callocs or reallocs node.outgoingEdges array, returns 0 on success
int AppendOutgoingEdgeToNode(Edge* edge, Node* node);


//Dijkstras algorithm, finds shortest path between two points
//Could be replaced with A*
Path Dijkstras(Node* startNode, Node* endNode);
//The A* algorithm
Path AStar(Node* startNode, Node* endNode);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);
#endif // CSV_IMPORTER_H
