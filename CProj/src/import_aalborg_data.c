//import_aalborg_data.c is linked to csv_importer.c in Cmake
#include "csv_importer.h"

int main (int argc, char *argv[])
{    //open the nodes file, open the edges file
    if (argc <3) {
        printf("Usage: %s <edge csv path> <nodes csv path>\n", argv[0]);
        exit(1);
    }
    //File path should be passed in as an argument when running the program
    const char *edge_csv_path = argv[1];
    FILE* edge_csv = fopen(edge_csv_path,"r");
    if (edge_csv == NULL) {
        printf("Error opening csv file\n");
        exit(1);
    }
    const char *nodes_csv_path = argv[2];
    FILE* nodes_csv = fopen(nodes_csv_path,"r");
    if (nodes_csv == NULL) {
        printf("Error opening csv file\n");
        exit(1);
    }
    //When making the Graph, it should 0-initialized
    //Something along the lines of Graph graph = {0};

    //allocate memory
    //Load data from files to memory

    //close files
    fclose(edge_csv);
    fclose(nodes_csv);
    //Do something with data in memory


    //free all the memory
    return 1;
 }
