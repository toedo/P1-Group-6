//import_aalborg_data.c is linked to csv_importer.c in Cmake
#include "csv_importer.h"

int main (int argc, char *argv[])
{
    FILE* nodesCSV = NULL;
    FILE* edgesCSV = NULL;
    //open the nodes file, open the edges file
    if (argc != 1 && argc != 3) {
        printf("Usage: %s [<nodes csv path> <edges csv path>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 3) {
        OpenCSVsFromPath(&nodesCSV, argv[2], &edgesCSV, argv[2]);
    }
    //TODO: Make default path separator take executing OS path separator into account
    if (argc == 1) {

        OpenCSVsFromPath(&nodesCSV, "dataset\\nodes.csv",&edgesCSV,"dataset\\edges.csv");
    }
    printf("Opened the two csv files successfully!\n");
    //allocate
    //When making the Graph, it should 0-initialized
    //Something along the lines of Graph graph = {0};
    Graph* aalborgGraph = calloc(1, sizeof(Graph));
    if (aalborgGraph == NULL) {
        printf("Error allocating graph\n");
        exit(EXIT_FAILURE);
    }
    printf("Successfully allocated graph!\n");

    printf("Begun loading the two csv files into memory!\n");
    //Load data from files to memory
    LoadNodesFromCSVToGraph(nodesCSV, aalborgGraph);
    LoadEdgesFromCSVToGraph(edgesCSV, aalborgGraph);
    printf("Successfully loaded the two CSV files into memory!\n");


    //close files now that the data from them have been loaded into memory
    printf("Closing the csv files!\n");
    fclose(nodesCSV);
    fclose(edgesCSV);
    printf("Successfully closed the two csv files\n");
    //Do something with data in memory
    //It MIGHT just be time to try and link nodes and edges
    printf("Attempting to connect nodes and edges together using CSVID!\n");
    int linkResult = LinkNodesAndEdgesByCSVID(aalborgGraph);
    printf("LinkNodesAndEdgesByCSVID(aalborgGraph) gave the following result: %d\n",linkResult);

    //free all the memory
    printf("Attempting to free aalborgGraph\n");
    int freeResult = FreeGraph(aalborgGraph);
    printf("FreeGraph(aalborgGraph) gave the following result: %d\n", freeResult);

    return 0;
 }
