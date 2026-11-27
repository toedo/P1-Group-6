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
    Edge** outgoingEdges;
    int amountOfIncomingEdges;
    Edge** incomingEdges;
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
    int speedKMH;
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

/**
 * The graph struct containing references to the nodes and edges
 * Should be dynamically allocated using calloc, due to the way FreeGraph() works
 */
typedef struct graph {
    //References to nodes and edges arrays
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
//Should be run before LoadEdgesFromCSVToGraph()
void LoadNodesFromCSVToGraph(FILE* nodesCSV, Graph* graph);
void LoadEdgesFromCSVToGraph(FILE* edgesCSV, Graph* graph);
int LinkNodesAndEdgesByCSVID(Graph* graph);
int AppendOutgoingEdgeToNode(Edge* edge, Node* node);
int AppendIncomingEdgeToNode(Edge* edge, Node* node);
void OpenCSVsFromPath(FILE** nodesCSV, const char *nodesCSVPath, FILE** edgeCSV, const char *edgeCSVPath);
int FreeGraph(Graph* graph);

#endif // CSV_IMPORTER_H
