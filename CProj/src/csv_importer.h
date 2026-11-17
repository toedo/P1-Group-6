#include <stdlib.h>

typedef struct node {
    unsigned int node_id;
    double longitude;
    double latitude;
} Node_t;

typedef struct edge {
    unsigned int edge_id;
    unsigned int reverse_edge_id;
    unsigned int from_node;
    unsigned int to_node;
    double length_m;

} Edge_t;