#include "pathAlgorithms.h"
const int INF = 1000000;


int parent(int i) { return (i - 1) / 2; }

// Returns index of left child
int leftChild(int i) { return 2 * i + 1; }

// Returns index of right child
int rightChild(int i) { return 2 * i + 2; }

void initHeap(minHeap *heap, int capacity) //Initializes heap
{
    heap->entry = malloc(capacity * sizeof(heapEntry));
    heap->size = 0;
    heap->capacity = capacity;
}

void push(minHeap *heap, int dist, Node* node) //Push new entry into heap to pop later
{
}

void pop(minHeap heap) //Pops entry; if entry is unmarked vertex, mark vertex and add the dist. If marked, ignore.
{

}



Path Dijkstras(Node *startNode, Node* endNode, Node* nodes, int node_count)
{
    minHeap heap;
    int heapCapacity = node_count;
    initHeap(&heap, heapCapacity);

    //Array for each vertex (node)
    //Stores the shortest route between start vertex d[s] and d[v]
    //initial value is infinity
    int *dist = malloc(node_count * sizeof(int));
    //boolean array u[], checks to see if each vertex is marked
    //initially all are unmarked (false)
    bool *marked = malloc(node_count * sizeof(bool));
    int *pred = malloc(node_count * sizeof(int)); //Array of predecessor vertices


    for (int i = 0; i < node_count; i++) {
        marked[i] = false;
        dist[i] = INF;
    }
    dist[startNode->nodeID] = 0;

    while (heap.size > 0) //CHANGE LATER!! DOESNT WORK LIKE THIS I DONT THINK!!
    {

    }

    free(dist);
    free(pred);
    free(marked);
}

//TODO: Vertex relaxation helper function
