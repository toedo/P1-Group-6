#include "csv_importer.h"

Node LoadNodeFromCSV(FILE* nodesFile) {
    Node node;
    fscanf(nodesFile, "%d,%lf,%lf,%lf,%lf,%d", node.nodeID, node.longitude, node.latitude, node.UTMx_m, node.UTMy_m, node.OSMNodeID);
    return node;
}