#include "dijkstras.h"

const int INF = 1000000;


Path Dijkstras(Node* startNode, Node* endNode, Node* nodes, int node_count) {
    //Array for each vertex (node)
    //Stores the shortest route between start vertex d[s] and d[v]
    //initial value is infinity
    int dist[node_count];
    //boolean array u[], checks to see if each vertex is marked
    //initially all are unmarked (false)
    bool marked[node_count];
    //TODO: array of predecessors p[] to store shortest paths

    for (int i = 0; i < node_count; i++) {
        if (nodes[i].nodeID != startNode->nodeID) {
            dist[i] = INF;
            marked[i] = false;
        }
        else {
            marked[i] = false;
            dist[i] = 0;
        }
    }

    for (int i = 0; i <= startNode->amountOfConnectedEdges; i++) {

    }
}

//TODO: Vertex relaxation helper function
