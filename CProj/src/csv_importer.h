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
    //UTM Zone 32 - meter data. Is good for calculating the distance in meters between two nodes
    //Can be used for heuristics or distance estimation.
    double UTMx_m;
    double UTMy_m;

    int OSMNodeID;
    //Array info used for adjacency arrays
    int amountOfOutgoingEdges;
    Edge** outgoingEdges;
    int amountOfIncomingEdges;
    Edge** incomingEdges;
} Node;

//A Single Edge, must be allocated with calloc
typedef struct edge {
    int edgeID;
    //The actual graph / path data
    //Is mostly used to create adjacency arrays and for fast lookup
    int fromNodeID;
    Node* fromNode;
    int toNodeID;
    Node* toNode;

    //This is mostly used to calculate the "cost" of traversing an edge
    double lengthM;
    int speedKMH;
    double travelTimeS;
    //Aalborg municipality road service class priority (1-4)
    int serviceClassPriority;
    //TODO: Double check what route number in the csv actually is
    //Bus route number, if there is an associated bus route?
    int routeNumber;

    //pointer to road name string
    char* roadName;
    //pointer to road type string
    char* roadType;

    //No reverse edge means it is oneway, this is also relevant for path finding
    bool isOneway;
    int reverseEdgeID;
    Edge* reverseEdge;

    //Pointer to linestring geometry string
    char* linestringGeometry;
    //This is a string of the form "(int)_(int)" where the two ints are two OSM node ids.
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

    int pathCount;
    Path* paths;
}Graph;

//Ordered arrays for the taken path
/**
 * Arrays of nodes and edges traversed (in order) inside an associated graph
 */
typedef struct path {
    //If the associatedGraph is a NULL pointer that means all the edges and nodes were deleted as well
    //This means that the path is invalid as it would be referencing now freed memory
    Graph* associatedGraph;

    int edgesTraversed;
    Edge** edges;

    int nodesTraversed;
    Node** nodes;
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
Graph* TrimGraphByLonLat(Graph* originalGraph, float lon1, float lon2, float lat1, float lat2);

#endif // CSV_IMPORTER_H
