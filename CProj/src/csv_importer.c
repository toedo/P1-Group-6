#include "csv_importer.h"

//Loads nodes from CSV document
//Should be ran before LoadEdgesFromCSV()
Node* LoadNodesFromCSV(FILE* nodesCSV, Node* nodes, int size) {
    //skip first line
    fscanf(nodesCSV,"[^\n]\n", NULL);
    int i = 0;
    Node node;
    //node_id,lon,lat,x_m,y_m,osm_node_id
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
            nodes[size-1] = node;
        }
        i++;
    }
    //return array pointer
    return nodes;
}
//Loads edges into an array from a CSV document
//Do not run before LoadNodesFromCSV()
Edge* LoadEdgesFromCSV(FILE* edgesCSV, Edge* edges, int size) {
    //skip the first line
    fscanf(edgesCSV,"[^\n]\n", NULL);
    Edge edge;
    char boolBuffer[6];
    //calloc string pointers 
    int i = 0;
    //edge_id,u,v,length_m,speed_kmh,travel_time_s,priority,route,name,road_type,oneway,reverse_edge_id,geometry,osm_edge_id
    while (14 == fscanf(edgesCSV,"%d,%d,%d,%lf,%d,%lf,%d,%d,%s,%s,%s,%d,\"%s\",%s",
        &edge.edgeID, &edge.fromNodeID, &edge.toNodeID,&edge.lengthM,&edge.speedKMH,
        &edge.travelTimeS,&edge.serviceClassPriority,&edge.routeNumber,&edge.roadName,
        &edge.roadType,&boolBuffer,&edge.reverseEdgeID, &edge.linestringGeometry, &edge.OSMEdgeID)) {
        //Fill out edge data, like pointers to Nodes, and turning boolBuffer into a bool
        if (strcmp(boolBuffer, "True") == 0) {
            edge.isOneway = true;
        } else {
            edge.isOneway = false;
        }

        if (i<size) {
            //While not exceeding the size of Edge* edges, overwrite
            edges[i] = edge;
        } else {
            //Else, realloc Edge* edges
            size++;
            edges = realloc(edges, sizeof(Edge)*size);
            if (edges == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            edges[size-1] = edge;
        }

        i++;
    }
}