#include "csv_importer.h"
#define MAX_STR_BUFFER 256

//Should only be ran once at the start of the program.
void LoadNodesFromCSVToGraph(FILE* nodesCSV, Graph* graph) {
    //Skip the first line
    char c = '-';
    while (c != '\n') {
        printf("%c", c);
        c = fgetc(nodesCSV);
    }
    printf("\n");

    Node* node = calloc(1, sizeof(Node));
    if (node == NULL) {
        printf("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    //node_id,lon,lat,x_m,y_m,osm_node_id
    while (6 == fscanf(nodesCSV, "%d,%lf,%lf,%lf,%lf,%d", &node->nodeID, &node->longitude, &node->latitude, &node->UTMx_m, &node->UTMy_m, &node->OSMNodeID)) {
        if (i<graph->nodeCount) {
            //while not exceeding the size of Node* nodes, overwrite
            memcpy(&graph->nodes[i],node,sizeof(Node));
        } else {
            //if there are more nodes than the size of Node* nodes, realloc Node* nodes
            //realloc
            graph->nodeCount++;
            Node* temp = realloc(graph->nodes, sizeof(Node)*graph->nodeCount);
            //exit cleanly in case of memory allocation failure
            if (temp == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            graph->nodes = temp;
            //write node to newly allocated memory
            memcpy(&graph->nodes[graph->nodeCount-1],node,sizeof(Node));
        }
        i++;
    }
    free(node);
}

//Should only be ran once at the start of the program.
void LoadEdgesFromCSVToGraph(FILE* edgesCSV, Graph* graph) {
    //skip the first line
    char c = '-';
    while (c != '\n') {
        printf("%c", c);
        c = fgetc(edgesCSV);
    }
    printf("\n");
    //edge struct for holding and formatting data while its being imported
    Edge* edge = calloc(1, sizeof(Edge));
    if (edge == NULL) {
        printf("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    //Buffers for string values
    char roadNameBuffer[MAX_STR_BUFFER];
    char roadTypeBuffer[MAX_STR_BUFFER];
    char isOnewayBuffer[6];
    char LSGBuffer[MAX_STR_BUFFER];
    char OSMEdgeIDBuffer[MAX_STR_BUFFER];

    int i = 0;
    //The number before the scansets, except for the isOnewayBuffer in the fscanf
    //should be the size of MAX_STR_BUFFER - 1
    //edge_id,u,v,length_m,speed_kmh,travel_time_s,priority,route,name,road_type,oneway,reverse_edge_id,geometry,osm_edge_id
    while (14 == fscanf(edgesCSV,
    "%d,%d,%d,%lf,%d,%lf,%d,%d,%255[^,],%255[^,],%5[^,],%d,\"%255[^\"]\",%255[^\r\n]",
    &edge->edgeID,
    &edge->fromNodeID,
    &edge->toNodeID,
    &edge->lengthM,
    &edge->speedKMH,
    &edge->travelTimeS,
    &edge->serviceClassPriority,
    &edge->routeNumber,
    roadNameBuffer,
    roadTypeBuffer,
    isOnewayBuffer,
    &edge->reverseEdgeID,
    LSGBuffer,
    OSMEdgeIDBuffer)) {

        //Fill out edge data, like pointers to Nodes, and turning boolBuffer into a bool
        //NumOfElements is strlen + 1 to account for string terminator.
        edge->roadName = calloc(strlen(roadNameBuffer)+1, sizeof(char));
        if (edge->roadName == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(edge->roadName, roadNameBuffer);
        edge->roadType = calloc(strlen(roadTypeBuffer)+1, sizeof(char));
        if (edge->roadType == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(edge->roadType, roadTypeBuffer);
        if (strcmp(isOnewayBuffer, "True") == 0) {
            edge->isOneway = true;
        } else {
            edge->isOneway = false;
        }
        edge->linestringGeometry = calloc(strlen(LSGBuffer)+1, sizeof(char));
        if (edge->linestringGeometry == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(edge->linestringGeometry, LSGBuffer);
        edge->OSMEdgeID = calloc(strlen(OSMEdgeIDBuffer)+1, sizeof(char));
        if (edge->OSMEdgeID == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(edge->OSMEdgeID, OSMEdgeIDBuffer);

        if (i<graph->edgeCount) {
            //While not exceeding the size of Edge* edges, overwrite
            memcpy(&graph->edges[i],edge,sizeof(Edge));
        } else {
            //Else, realloc Edge* edges
            graph->edgeCount++;
            Edge* temp = graph->edges;
            temp = realloc(graph->edges, sizeof(Edge)*graph->edgeCount);
            if (temp == NULL) {
                printf("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            graph->edges = temp;
            memcpy(&graph->edges[graph->edgeCount-1],edge,sizeof(Edge));
        }

        i++;
    }
    free(edge);
}
//Assumes edgeID is equivalent to index, assumes nodeID is equivalent to index
//Because that's the way it is in the data we are working from
//TODO: Ideally both the toNodeID and fromNodeID should be checked BEFORE appending
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
            } else {
                return-1;
            }
        } else {
            return -1;
        }


        //Same as above but from Incoming Edges to a Node
        if (graph->edges[i].toNodeID >= 0 && graph->edges[i].toNodeID < graph->nodeCount) {
            graph->edges[i].toNode = &graph->nodes[graph->edges[i].toNodeID];
            if (AppendIncomingEdgeToNode(&graph->edges[i],&graph->nodes[graph->edges[i].toNodeID])==0) {
                //Write here for good message
            } else {
                return -1;
            }
        } else {
            return -1;
        }


        //reverse edge pointers
        if (graph->edges[i].isOneway == false
            && graph->edges[i].reverseEdgeID >= 0
            && graph->edges[i].reverseEdgeID < graph->edgeCount) {
            graph->edges[i].reverseEdge = &graph->edges[graph->edges[i].reverseEdgeID];
        }
    }

    return 0;
}
//TODO: Maybe add function that links using OSM IDs

//TODO: Maybe return -1 on failure instead of just exiting the program
int AppendOutgoingEdgeToNode(Edge* edge, Node* node) {
    //should be 0 by default
    node->amountOfOutgoingEdges++;
    if (node->outgoingEdges == NULL) {
        node->outgoingEdges = calloc(node->amountOfOutgoingEdges, sizeof(*node->outgoingEdges));
        if (node->outgoingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->outgoingEdges[node->amountOfOutgoingEdges-1] = edge;
    } else {
        Edge** temp = realloc(node->outgoingEdges,sizeof(*node->outgoingEdges)*node->amountOfOutgoingEdges);
        if (temp == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->outgoingEdges = temp;
        node->outgoingEdges[node->amountOfOutgoingEdges-1] = edge;
    }

    return 0;
}
//TODO: Maybe return -1 on failure instead of just exiting the program
int AppendIncomingEdgeToNode(Edge* edge, Node* node) {
    //should be 0 by default
    node->amountOfIncomingEdges++;
    if (node->incomingEdges == NULL) {
        node->incomingEdges = calloc(node->amountOfIncomingEdges, sizeof(*node->outgoingEdges));
        if (node->incomingEdges == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->incomingEdges[node->amountOfIncomingEdges-1] = edge;
    } else {
        Edge** temp = realloc(node->incomingEdges,sizeof(*node->outgoingEdges)*node->amountOfIncomingEdges);
        if (temp == NULL) {
            printf("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        node->incomingEdges = temp;
        node->incomingEdges[node->amountOfIncomingEdges-1] = edge;
    }
    return 0;
}

void OpenCSVsFromPath(FILE** nodesCSV, const char *nodesCSVPath, FILE** edgeCSV, const char *edgeCSVPath) {
    *nodesCSV = fopen(nodesCSVPath,"r");
    if (nodesCSV == NULL) {
        printf("Error opening csv file\n");
        exit(EXIT_FAILURE);
    }
    //File path should be passed in as an argument when running the program
    *edgeCSV = fopen(edgeCSVPath,"r");
    if (edgeCSV == NULL) {
        printf("Error opening csv file\n");
        exit(EXIT_FAILURE);
    }
}

//TODO: Add function that cleanly frees all allocated memory