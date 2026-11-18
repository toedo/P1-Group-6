#ifndef CSV_IMPORTER_H
#define CSV_IMPORTER_H
#include <stdio.h>

typedef struct node Node;
typedef struct edge Edge;

//A Single Node
typedef struct node {
    int nodeID;
    double longitude;
    double latitude;
    //Array info
    int amountOfConnectedEdges;
    Edge* connectedEdges;
} Node;

//A Single Edge
typedef struct edge {
    Node* fromNode;
    Node* toNode;
    int edgeID;
    //No reverse edge means it is oneway
    int reverseEdgeID;
    Edge* reverseEdge;
    double lengthM;
    //Heuristics
    int serviceClass;
    //Traffic, Hospital proximity, Bus Route*,

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

//Dijkstras algorithm, finds shortest path between two points
//Could be replaced with A*
Path Dijkstras(Node startNode, Node endNode);
//The A* algorithm
Path AStar(Node startNode, Node endNode);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);
#endif // CSV_IMPORTER_H
