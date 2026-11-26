//import_aalborg_data.c is linked to csv_importer.c in Cmake
#include "csv_importer.h"

int main (int argc, char *argv[])
{    //open the nodes file, open the edges file
    if (argc <3) {
        printf("Usage: %s <nodes csv path> <edges csv path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *nodesCSVPath = argv[1];
    FILE* nodesCSV = fopen(nodesCSVPath,"r");
    if (nodesCSV == NULL) {
        printf("Error opening csv file\n");
        exit(EXIT_FAILURE);
    }
    //File path should be passed in as an argument when running the program
    const char *edgeCSVPath = argv[2];
    FILE* edgeCSV = fopen(edgeCSVPath,"r");
    if (edgeCSV == NULL) {
        printf("Error opening csv file\n");
        exit(EXIT_FAILURE);
    }

    //allocate
    //When making the Graph, it should 0-initialized
    //Something along the lines of Graph graph = {0};
    Graph* aalborgGraph = calloc(1, sizeof(Graph));
    if (aalborgGraph == NULL) {
        printf("Error allocating graph\n");
        exit(EXIT_FAILURE);
    }
    //Load data from files to memory
    LoadNodesFromCSVToGraph(nodesCSV, aalborgGraph);
    LoadEdgesFromCSVToGraph(edgeCSV, aalborgGraph);

    //close files now that the data from them have been loaded into memory
    fclose(nodesCSV);
    fclose(edgeCSV);
    //Do something with data in memory


    //free all the memory
    return 1;
 }
