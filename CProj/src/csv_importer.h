#ifndef CSV_IMPORTER_H
#define CSV_IMPORTER_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct node Node;
typedef struct edge Edge;

//A Single Node
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
    int amountOfConnectedEdges;
    Edge* connectedEdges;
} Node;

//A Single Edge
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
    char* roadName;
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

//Ordered arrays for the taken path
typedef struct path {
    int edgesTraversed;
    Edge* edges;

    int nodesTraversed;
    Node* nodes;
} Path;

//Load all nodes from file
Node* LoadNodesFromCSV(FILE* nodesCSV, Node* nodes, int size);
//Helper function to load a single edge
Edge LoadEdgeFromCSV(FILE* edgesFile);
//Load all edges from file
Edge* LoadEdgesFromCSV(FILE* edgesCSV, Edge* edges, int size);

//Dijkstras algorithm, finds shortest path between two points
//Could be replaced with A*
Path Dijkstras(Node* startNode, Node* endNode);
//The A* algorithm
Path AStar(Node* startNode, Node* endNode);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);
#endif // CSV_IMPORTER_H
