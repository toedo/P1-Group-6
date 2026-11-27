#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "csv_importer.h"
//The pathfinding algorithms Djikstras and A* are (to a degree) interchangable (they use the same start and end nodes)

//Dijkstras algorithm, finds shortest path between two points
Path Dijkstras(Node* startNode, Node* endNode, Node* nodes, int node_count);
//The A* algorithm
Path AStar(Node* startNode, Node* endNode, Node* nodes, int node_count);

//Implementation of the chinese postman problem
Path ChinesePostman(Node* startNode, Node* nodes, Edge* edges);



//Structure of heaps (double because travel times have decimal values)
typedef struct
{
    double dist;
    Node* node;
} heapEntry;

typedef struct
{
    int size;
    int capacity;
    heapEntry* entry;
} minHeap;

int rightChild(int i);
int leftChild(int i);
int parent(int i);

//Function returns -1 if memory allocation for the heap entry failed
int push(minHeap *heap, double dist, Node* node);
//The function removes and then returns the minimum heap entry, so that the algorithm can access the node and its priority(distance)
heapEntry pop(minHeap *heap);
//Function returns -1 if memory allocation for the heap failed
int initHeap(minHeap *heap, int capacity);
void heapEntrySwap(heapEntry *a, heapEntry *b);

#endif