#include "pathAlgorithms.h"
#include <math.h>

// Returns parent index
int parent(int i)
{
    return (i - 1) / 2;
}

// Returns index of left child
int leftChild(int i)
{
    return 2 * i + 1;
}

// Returns index of right child
int rightChild(int i)
{
    return 2 * i + 2;
}

int initHeap(minHeap *heap, int capacity) //Initializes heap
{
    heap->entry = malloc(capacity * sizeof(heapEntry));
    heap->size = 0;
    heap->capacity = capacity;
}

int push(minHeap *heap, double dist, Node* node) //Push new entry into heap to pop later
{
}

heapEntry pop(minHeap *heap) //Pops entry; if entry is unmarked vertex, mark vertex and add the dist. If marked, ignore.
{

}


//A helper function for Dijkstra and AStar pathfinding
Path findPath(Node* startNode, Node* endNode, Node* nodes, int node_count, bool usingAStar)
{
    //Initializes path
    Path path = {0};

    //Check if input is valid
    if (startNode == 0 || endNode == NULL || nodes == 0 || node_count <= 0)
    {
        return path;
    }


}

/*If this tries to use array indicies
 *without using the graph struct
 *(which stores the size of the nodes array and the size of the edges array)
 *this is going to cause problems
 *(most likely out-of-bounds memory read errors)
*/
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
        dist[i] = INFINITY;
    }
    dist[startNode->nodeID] = 0;

    while (heap.size > 0) //CHANGE LATER!! DOESNT WORK LIKE THIS I DONT THINK!!
    {

    }

    free(dist);
    free(pred);
    free(marked);
}

double heuristic (Node* currentNode, Node* endNode)
{
    //Not added yet for A star
    return 0;
}

Path AStar(Node* startNode, Node* endNode, Node* nodes, int node_count)
{

}


//TODO: Vertex relaxation helper function
