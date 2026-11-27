#include "csv_importer.h"
#define MAX_STR_BUFFER 256

//Should only be run once at the start of the program.
/**
 * Should only be run once at the start of the program.
 * Loads in the edges into the graph's array from the csv file.
 * @param nodesCSV .csv file containing nodes
 * @param graph the graph where the edges are saved
 */
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


/**
 * Should only be run once at the start of the program.
 * Loads in the edges into the graph's array from the csv file.
 * @param edgesCSV .csv file containing edges
 * @param graph the graph where the edges are saved
 */
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

        //Fill out edge data
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
            Edge* temp = realloc(graph->edges, sizeof(Edge)*graph->edgeCount);
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
//TODO: Some king of system that murders a fucked up edge? Or maybe just brick the whole brogram if theres an error
/**
 * Assumes edgeID is equivalent to array index, assumes nodeID is equivalent to array index.
 * Because that's the way it is in the data we are working from.
 * Creates pointers between corresponding associated notes and edges
 * @param graph
 * @return returns 0 on success, otherwise an error has occurred
 */
int LinkNodesAndEdgesByCSVID(Graph* graph) {
    int retVal = 0;
    //loop over edges array
    for (int i = 0; i < graph->edgeCount; i++) {
        Edge* edge = &graph->edges[i];
        //Checks that the fromNodeID and the toNodeID are both valid indexes
        if (edge->fromNodeID < 0 || edge->fromNodeID >= graph->nodeCount
            || edge->toNodeID < 0 || edge->toNodeID >= graph->nodeCount) {
            retVal = -1;
            printf("Error: fromNodeID or toNodeID missing! Skipped adding edge.\n");
            continue;
        }
        //Since both of these indexes exist, both of the corresponding nodes have pointers added at once
        //This prevents any kind of "hanging chad" where an edge only has one of the proper pointers
        //For example if a graph was trimmed down and happened to "cut" between two connected nodes
        edge->fromNode = &graph->nodes[edge->fromNodeID];
        edge->toNode = &graph->nodes[edge->toNodeID];
        //Now the fromNode should have the current edge added to its outgoingEdges
        if (AppendOutgoingEdgeToNode(edge, edge->fromNode) != 0) {
            printf("Error: Failed to connect outgoing edge to start node.\n");
            retVal = -1;
            continue;
        }
        //And the toNode should have the current edge added to its incomingEdges
        if (AppendIncomingEdgeToNode(edge, edge->toNode) != 0) {
            //If the previous append operation succeeded, but the current one failed
            //Undo the previous append operation and skip the rest of this edge
            printf("Error: Failed to connect incoming edge to destination node.\n");
            edge->fromNode->amountOfOutgoingEdges--;
            retVal = -1;
            continue;
        }

        //reverse edge pointers
        if (!edge->isOneway) {
            if (edge->reverseEdgeID < 0 || edge->reverseEdgeID >= graph->edgeCount) {
                retVal = -1;
                continue;
            }
            edge->reverseEdge = &graph->edges[edge->reverseEdgeID];
        }
    }

    return retVal;
}

/**
 * Helper function that creates pointers between an edge that starts at a node
 * Returns 0 on success, -1 for an error
 * @param edge outgoing edge
 * @param node start node
 * @return returns 0 on success
 */
int AppendOutgoingEdgeToNode(Edge* edge, Node* node) {
    //Temporary values are used to prevent overwriting existing memory in case of a realloc failure.
    int newCount = node->amountOfOutgoingEdges + 1;
    Edge** temp = realloc(node->outgoingEdges,sizeof(Edge*)*newCount);
    if (temp == NULL) {
        printf("Memory allocation failed");
        return -1;
    }

    //Adds the outgoing edge to the node
    temp[newCount-1] = edge;
    node->outgoingEdges = temp;
    node->amountOfOutgoingEdges = newCount;

    return 0;
}

/**
 * Helper function that creates pointers between an edge that ends at a node
 * Returns 0 on success, -1 for an error
 * @param edge incoming edge
 * @param node end node
 * @return returns 0 on success
 */
int AppendIncomingEdgeToNode(Edge* edge, Node* node) {
    //Temporary values are used to prevent overwriting existing memory in case of a realloc failure.
    int newCount = node->amountOfIncomingEdges + 1;
    Edge** temp = realloc(node->incomingEdges,sizeof(*node->incomingEdges)*newCount);
    if (temp == NULL) {
        printf("Memory allocation failed");
        return -1;
    }

    //Adds the incoming edge to the node
    temp[newCount-1] = edge;
    node->incomingEdges = temp;
    node->amountOfIncomingEdges = newCount;

    return 0;
}

/**
 * Used to open to CSV files for nodes and edges respectively
 * @param nodesCSV Used as a return variable for the nodes
 * @param nodesCSVPath Absolute or local path from working directory to node csv file
 * @param edgeCSV Used as a return variable for the edges
 * @param edgeCSVPath Absolute or local path from working directory to edges csv file
 */
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


//TODO: Finish function that cleanly frees all allocated memory

/**
 *Frees a Graph, includes nodes array and edges array
 *Consequently, arrays of nodes and edges MUST be associated with only one graph
 *Otherwise other graphs could try to access freed memory
 * @param graph Pointer to the graph struct to be freed
 */
int FreeGraph(Graph* graph) {
    if (graph == NULL) {
        printf("Failed to free graph, as graph pointer was NULL\n");
        return -1;
    }
    return -1;
}