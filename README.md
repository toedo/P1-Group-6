# Snow Plow and Road Salter Routing Algorithm
This project is by P1-Group-6 from Dat-26 at Aalborg University.
The purpose of this project is to make an algorithm that can optimize the path taken by a snow plow and road salting vehicle in Aalborg Municipality, to optimze resource use.

## Description
(More in-depth technical descriptions)

## Quickstart Guide
This project relies on several programming languages and dependencies to optimize data-fetching and visualization, while the core algorithm is written in C.

### Dependencies
The data used relies on OpenStreetMap (OSM) data and libraries used to import and perform operations on this data, to get street data for the area in which the route-optimization is desired. Here the following libraries are automatically downloaded by a bash scripts when fetching the data from GEOFABRIK.DE:
- Protozero (https://github.com/mapbox/protozero)
- LibOsmium (https://github.com/osmcode/libosmium)
- Osmium (https://github.com/osmcode/osmium-tool.git)

It is recommended to use Cond

### Installing
It is recommended to use Conda-Forge to run this project when using Windows.

To run this locally, copy the repo locally and first run the import-OSM bash script to import the map data.
Then the algorithm can be run on the area defined by the import-OSM script by running (insert final finale name).c.
Then to visualize this... (TBD)
