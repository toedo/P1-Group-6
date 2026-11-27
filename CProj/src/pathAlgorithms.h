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

typedef struct
{
    int dist;
    Node* node;
} heapEntry;;

typedef struct
{
    int size;
    int capacity;
    heapEntry* entry;
} minHeap;

int rightChild(int i);
int leftChild(int i);
int parent(int i);

void push(minHeap *heap, int dist, Node* node);
void pop(minHeap heap);
void initHeap(minHeap *heap, int capacity);
void heapEntrySwap(heapEntry *a, heapEntry *b);

#endif