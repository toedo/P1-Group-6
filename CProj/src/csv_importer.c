#include "csv_importer.h"

#include <stdlib.h>

Node* LoadNodesFromCSV(FILE* nodesCSV, Node* nodes, int size) {
    //skip first line
    fscanf(nodesCSV,"[^\n]\n", NULL);
    int i = 0;
    Node node;

    while (6 == fscanf(nodesCSV, "%d,%lf,%lf,%lf,%lf,%d", &node.nodeID, &node.longitude, &node.latitude, &node.UTMx_m, &node.UTMy_m, &node.OSMNodeID)) {
        if (i<size) {
            //while not exceeding the size of Node* nodes, overwrite
            nodes[i] = node;
        } else {
            //if there are more nodes than the size of Node* nodes, realloc Node* nodes
            //realloc
            size++;
            nodes = realloc(nodes, sizeof(Node)*size);
            //exit cleanly in case of memory allocation failure
            if (nodes == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            //write node to newly allocated memory
            nodes[size] = node;
        }
        i++;
    }




    //return array pointer
}