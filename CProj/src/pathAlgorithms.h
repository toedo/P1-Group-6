#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "csv_importer.h"
//Djikstras and A* are (to a degree) interchangable (they use the same start and end nodes)

//Dijkstras algorithm, finds shortest path between two points
Path Dijkstras(Node* startNode, Node* endNode, Node* nodes, int node_count);
//The A* algorithm
Path AStar(Node* startNode, Node* endNode);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);

//Creates new subsets of Nodes* and Edges* that are inside a square
//it also "links" the new arrays
Graph TrimGraphByCoords(Graph originalGraph, float lon1, float lat1, float lon2, float lat2);



#endif