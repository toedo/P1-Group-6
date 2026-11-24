#include "csv_importer.h"
#define MAX_STR_BUFFER 256

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
    //edge struct for holding and formatting data while its being imported
    Edge edge;

    //Buffers for string values
    char roadNameBuffer[MAX_STR_BUFFER];
    char roadTypeBuffer[MAX_STR_BUFFER];
    char isOnewayBuffer[6];
    char LSGBuffer[MAX_STR_BUFFER];
    char OSMEdgeIDBuffer[MAX_STR_BUFFER];

    int i = 0;
    //edge_id,u,v,length_m,speed_kmh,travel_time_s,priority,route,name,road_type,oneway,reverse_edge_id,geometry,osm_edge_id
    while (14 == fscanf(edgesCSV,"%d,%d,%d,%lf,%d,%lf,%d,%d,%s,%s,%s,%d,\"%s\",%s",
        &edge.edgeID, &edge.fromNodeID, &edge.toNodeID,&edge.lengthM,&edge.speedKMH,
        &edge.travelTimeS,&edge.serviceClassPriority,&edge.routeNumber,&roadNameBuffer,&roadTypeBuffer,
        &roadTypeBuffer,&isOnewayBuffer,&edge.reverseEdgeID, &LSGBuffer, &OSMEdgeIDBuffer)) {

        //Fill out edge data, like pointers to Nodes, and turning boolBuffer into a bool
        //TODO:Might be missing end of string (EOS) character. NumOfElements might also be 1 too low.
        edge.roadName = calloc(strlen(roadNameBuffer), sizeof(char));
        strcpy(edge.roadName, roadNameBuffer);
        edge.roadType = calloc(strlen(roadTypeBuffer), sizeof(char));
        strcpy(edge.roadType, roadTypeBuffer);
        if (strcmp(isOnewayBuffer, "True") == 0) {
            edge.isOneway = true;
        } else {
            edge.isOneway = false;
        }
        edge.linestringGeometry = calloc(strlen(edge.roadType), sizeof(char));
        strcpy(edge.linestringGeometry, edge.roadType);
        edge.OSMEdgeID = calloc(strlen(edge.roadName), sizeof(char));
        strcpy(edge.OSMEdgeID, edge.roadName);

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

int LinkNodesAndEdges(Edge* edges, int edgeCount, Node* nodes, int nodeCount) {
    //loop over edges array
    for (int i = 0; i < edgeCount; i++) {
        //find start-node pointer by CSV ID, put node pointer in Node* fromNode
        //Avoid out-of-bounds read
        if (edges[i].fromNodeID >= 0 && edges[i].fromNodeID < nodeCount) {
            //The fromNodeID is equivalent to the array index of that node in Node* nodes
            edges[i].fromNode = &nodes[edges[i].fromNodeID];
            //put edge pointer in from-nodes's Edge* outgoingEdges array.
            //TODO: Add exiting edge to node.outgoingEdges
        }
        //find end-node pointer by CSV ID, put node pointer in Node* toNode
        if (edges[i].toNodeID >= 0 && edges[i].toNodeID < nodeCount) {
            edges[i].toNode = &nodes[edges[i].toNodeID];
        }

    }

    return 0;
}
int AppendOutgoingEdgeToNode(Edge* edge, Node* node) {
    node->amountOfOutgoingEdges++;
    //TODO: Write the code

}

//TODO: Add function that cleanly frees allocated memory