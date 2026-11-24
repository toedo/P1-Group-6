#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdio.h>
#include "csv_importer.h"
//Djikstras and A* are (to a degree) interchangable (they use the same start and end nodes)

//Dijkstras algorithm, finds shortest path between two points
Path Dijkstras(Node* startNode, Node* endNode, Node* nodes, int node_count);
//The A* algorithm
Path AStar(Node* startNode, Node* endNode);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);


#endif