#include "csv_importer.h"
#define MAX_STR_BUFFER 256


void LoadNodesFromCSVToGraph(FILE* nodesCSV, Graph* graph) {
    //skip first line
    fscanf(nodesCSV,"[^\n]\n", NULL);
    int i = 0;
    Node* node = calloc(1, sizeof(Node));
    //node_id,lon,lat,x_m,y_m,osm_node_id
    while (0 < fscanf(nodesCSV, "%d,%lf,%lf,%lf,%lf,%d", &node->nodeID, &node->longitude, &node->latitude, &node->UTMx_m, &node->UTMy_m, &node->OSMNodeID)) {
        if (i<graph->nodeCount) {
            //while not exceeding the size of Node* nodes, overwrite
            memcpy(&graph->nodes[i],node,sizeof(Node));
        } else {
            //if there are more nodes than the size of Node* nodes, realloc Node* nodes
            //realloc
            graph->nodeCount++;
            graph->nodes = realloc(graph->nodes, sizeof(Node)*graph->nodeCount);
            //exit cleanly in case of memory allocation failure
            if (graph->nodes == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            //write node to newly allocated memory
            memcpy(&graph->nodes[graph->nodeCount-1],node,sizeof(Node));
        }
        i++;
    }

}

void LoadEdgesFromCSVToGraph(FILE* edgesCSV, Graph* graph) {
    //skip the first line
    fscanf(edgesCSV,"[^\n]\n", NULL);
    //edge struct for holding and formatting data while its being imported
    Edge* edge = calloc(1, sizeof(Edge));

    //Buffers for string values
    char roadNameBuffer[MAX_STR_BUFFER];
    char roadTypeBuffer[MAX_STR_BUFFER];
    char isOnewayBuffer[6];
    char LSGBuffer[MAX_STR_BUFFER];
    char OSMEdgeIDBuffer[MAX_STR_BUFFER];

    int i = 0;
    //edge_id,u,v,length_m,speed_kmh,travel_time_s,priority,route,name,road_type,oneway,reverse_edge_id,geometry,osm_edge_id
    while (0 < fscanf(edgesCSV,"%d,%d,%d,%lf,%d,%lf,%d,%d,%s,%s,%s,%d,\"%s\",%s",
        &edge->edgeID, &edge->fromNodeID, &edge->toNodeID,&edge->lengthM,&edge->speedKMH,
        &edge->travelTimeS,&edge->serviceClassPriority,&edge->routeNumber,roadNameBuffer,roadTypeBuffer,
        isOnewayBuffer,&edge->reverseEdgeID, LSGBuffer, OSMEdgeIDBuffer)) {

        //Fill out edge data, like pointers to Nodes, and turning boolBuffer into a bool
        //NumOfElements is strlen + 1 to account for string terminator.
        edge->roadName = calloc(strlen(roadNameBuffer)+1, sizeof(char));
        strcpy(edge->roadName, roadNameBuffer);
        edge->roadType = calloc(strlen(roadTypeBuffer)+1, sizeof(char));
        strcpy(edge->roadType, roadTypeBuffer);
        if (strcmp(isOnewayBuffer, "True") == 0) {
            edge->isOneway = true;
        } else {
            edge->isOneway = false;
        }
        edge->linestringGeometry = calloc(strlen(LSGBuffer)+1, sizeof(char));
        strcpy(edge->linestringGeometry, LSGBuffer);
        edge->OSMEdgeID = calloc(strlen(OSMEdgeIDBuffer)+1, sizeof(char));
        strcpy(edge->OSMEdgeID, OSMEdgeIDBuffer);

        if (i<graph->edgeCount) {
            //While not exceeding the size of Edge* edges, overwrite
            memcpy(&graph->edges[i],edge,sizeof(Edge));
        } else {
            //Else, realloc Edge* edges
            graph->edgeCount++;
            graph->edges = realloc(graph->edges, sizeof(Edge)*graph->edgeCount);
            if (graph->edges == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            memcpy(&graph->edges[graph->edgeCount-1],edge,sizeof(Edge));
        }

        i++;
    }
}

int LinkNodesAndEdgesByCSVID(Graph* graph) {
    //loop over edges array
    for (int i = 0; i < graph->edgeCount; i++) {
        //find start-node pointer by CSV ID, put node pointer in Node* fromNode
        //Avoid out-of-bounds read
        if (graph->edges[i].fromNodeID >= 0 && graph->edges[i].fromNodeID < graph->nodeCount) {
            //The fromNodeID is equivalent to the array index of that node in Node* nodes
            graph->edges[i].fromNode = &graph->nodes[graph->edges[i].fromNodeID];
            //put edge pointer in from-nodes's Edge* outgoingEdges array.
            if (AppendOutgoingEdgeToNode(&graph->edges[i],&graph->nodes[graph->edges[i].fromNodeID])==0) {
                //printf for logging here?
            }
        }
        //Same as above but from Incoming Edges to a Node
        if (graph->edges[i].toNodeID >= 0 && graph->edges[i].toNodeID < graph->nodeCount) {
            graph->edges[i].toNode = &graph->nodes[graph->edges[i].toNodeID];
            if (AppendIncomingEdgeToNode(&graph->edges[i],&graph->nodes[graph->edges[i].toNodeID])==0) {
                //Write here for good message
            }
        }
        //find end-node pointer by CSV ID, put node pointer in Node* toNode
        if (graph->edges[i].toNodeID >= 0 && graph->edges[i].toNodeID < graph->nodeCount) {
            graph->edges[i].toNode = &graph->nodes[graph->edges[i].toNodeID];
        }
        //reverse edge pointers
        if (graph->edges[i].isOneway == false) {
            graph->edges[i].reverseEdge = &graph->edges[graph->edges[i].reverseEdgeID];
        }
    }

    return 0;
}
//TODO: Maybe add function that links using OSM IDs

int AppendOutgoingEdgeToNode(Edge* edge, Node* node) {
    //should be 0 by default
    node->amountOfOutgoingEdges++;
    if (node->outgoingEdges == NULL) {
        node->outgoingEdges = calloc(node->amountOfOutgoingEdges, sizeof(Edge));
        if (node->outgoingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->outgoingEdges[node->amountOfOutgoingEdges-1] = *edge;
    } else {
        node->outgoingEdges = realloc(node->outgoingEdges,sizeof(Edge)*node->amountOfOutgoingEdges);
        if (node->outgoingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->outgoingEdges[node->amountOfOutgoingEdges-1] = *edge;
    }

    return 0;
}

int AppendIncomingEdgeToNode(Edge* edge, Node* node) {
    //should be 0 by default
    node->amountOfIncomingEdges++;
    if (node->incomingEdges == NULL) {
        node->incomingEdges = calloc(node->amountOfIncomingEdges, sizeof(Edge));
        if (node->incomingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->incomingEdges[node->amountOfIncomingEdges-1] = *edge;
    } else {
        node->incomingEdges = realloc(node->incomingEdges,sizeof(Edge)*node->amountOfIncomingEdges);
        if (node->incomingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->incomingEdges[node->amountOfIncomingEdges-1] = *edge;
    }
    return 0;
}

//TODO: Add function that cleanly frees all allocated memory