# Aalborg road network for winter salting

A directed road network of Aalborg Kommune where every road knows its **winter-service
priority**: how important it is to salt it. It is made for route-planning exercises
such as the **Chinese Postman Problem**: find a short closed tour that drives
every road that must be salted at least once.

| File | What it is |
|---|---|
| `edges.csv` / `edges.parquet` | one row per **arc** (a road you can drive in one direction) |
| `nodes.csv` / `nodes.parquet` | one row per **node** (an intersection or a dead end) |
| `network_map.html` | interactive map – open it in a browser |
| `roadmap.py` | draws the map from Python, also with your own tour on top |
| `example_usage.ipynb` | loading the data, building a graph, plotting a tour |
| `dataset_info.json` | counts, and when the data was downloaded |

CSV and Parquet hold exactly the same data. CSV opens in Excel; Parquet is faster
in pandas (`pd.read_parquet`, needs `pyarrow`).

---

## 1. The network in one minute

* The network is a **directed graph**: `14,647` nodes and `32,346` arcs.
* A **two-way street is two arcs**, one in each direction. They point at each other
  through `reverse_edge_id`. A **one-way street is one arc** (`oneway = True`,
  `reverse_edge_id = -1`).
* Arcs run from intersection to intersection (or to a dead end). One street can
  therefore consist of many arcs.
* The graph is **strongly connected**: from every node you can drive to every other
  node. A closed tour that covers any set of arcs always exists.
* Node and arc ids are small integers (`0 … N-1` and `0 … M-1`), so they can be used
  directly as list or array indices.

```
   node 17  ── arc 40 (→) ──>  node 18          two-way street:
            <── arc 41 (←) ──                 arcs 40 and 41, reverse_edge_id of each other

   node 18  ── arc 52 (→) ──>  node 19          one-way street:
                                                 arc 52, reverse_edge_id = -1
```

## 2. Columns

### `edges` – one row per arc

| column | type | meaning |
|---|---|---|
| `edge_id` | int | id of the arc, `0 … M-1` |
| `u` | int | node the arc **starts** at |
| `v` | int | node the arc **ends** at |
| `length_m` | float | length in meters |
| `speed_kmh` | int | speed limit in the driving direction (see *limitations*) |
| `travel_time_s` | float | `length_m / speed` – seconds to drive the arc at the speed limit |
| `priority` | int | winter-service priority **1 – 4**, or **0** if the municipality does not salt it (table below) |
| `route` | int | the municipality's own salting-route number (e.g. `301`), `-1` if none |
| `name` | text | street name, empty if the road has no name |
| `road_type` | text | OpenStreetMap road type: `residential`, `tertiary`, `secondary`, `motorway`, … |
| `oneway` | bool | `True` for a one-way street |
| `reverse_edge_id` | int | the arc driving the same street the other way, `-1` for one-way streets |
| `geometry` | text | the road's shape as WKT, `LINESTRING (lon lat, lon lat, …)`, drawn from `u` to `v` |
| `osm_edge_id` | text | id in the raw OpenStreetMap data – only for tracing an arc back to the source |

### `nodes` – one row per node

| column | type | meaning |
|---|---|---|
| `node_id` | int | id of the node, `0 … N-1` |
| `lon`, `lat` | float | position in degrees (WGS84, as in Google Maps) |
| `x_m`, `y_m` | float | position in **meters** (UTM zone 32N). The straight-line distance between two nodes is `hypot(x2-x1, y2-y1)`, handy for heuristics |
| `osm_node_id` | int | id in OpenStreetMap – only for tracing back |

### What the priorities mean

Aalborg Kommune divides its roads into four service classes
([source](https://www.aalborg.dk/mit-liv/trafik-og-parkering/trafik-og-veje/vejene-i-vintervejr/)):

| priority | roads | the municipality's goal |
|---|---|---|
| **1** | large through roads | salted *before* ice forms; kept passable around the clock |
| **2** | roads linking through roads and local roads | passable without major disruption, 02:00 – 18:00 |
| **3** | local roads, parking at schools and institutions | passable, some disruption; usually salted once a day |
| **4** | residential streets | only salted in exceptional cases |
| **0** | not in the municipal winter service | private roads, motorways and other state roads (salted by Vejdirektoratet), roads just outside the municipality |

Street length per priority (each street counted once):

| priority | 1 | 2 | 3 | 4 | 0 |
|---|---|---|---|---|---|
| km | 348 | 831 | 132 | 712 | 989 |

Arc length is about twice that, because a two-way street is two arcs.

`route` is the salting route the municipality actually uses: one truck drives one
route. There are 74 routes. They make good small test problems (*"salt every arc of
route 301"*). Route `0` is a state road; a few codes such as `99` are internal
to the municipality.

## 3. Loading the data

```python
import pandas as pd

edges = pd.read_csv("edges.csv")        # or pd.read_parquet("edges.parquet")
nodes = pd.read_csv("nodes.csv")

# arcs that must be salted, e.g. priority 1 and 2
required = edges[edges["priority"].isin([1, 2])]
print(len(required), "arcs,", required["length_m"].sum() / 1000, "km")
```

### A graph in plain Python (no libraries)

```python
# out_arcs[node] = list of edge_ids that start at that node
out_arcs = [[] for _ in range(len(nodes))]
for edge_id, u in zip(edges["edge_id"], edges["u"]):
    out_arcs[u].append(edge_id)

length = edges["length_m"].tolist()     # length[edge_id]
head   = edges["v"].tolist()            # head[edge_id] = node the arc ends at

for e in out_arcs[0]:
    print(f"from node 0 along arc {e} to node {head[e]} ({length[e]} m)")
```

### With networkx (optional)

```python
import networkx as nx

G = nx.MultiDiGraph()
for r in edges.itertuples():
    G.add_edge(r.u, r.v, key=r.edge_id, length=r.length_m, priority=r.priority)

dist = nx.shortest_path_length(G, source=0, target=500, weight="length")
```

## 4. Drawing maps and tours

`network_map.html` shows the whole network. With `roadmap.py` you can draw any part
of it, or your own tour, from Python:

```python
import roadmap

roadmap.save_map(edges, "map.html")                     # whole network -> open map.html
roadmap.show_map(edges[edges["route"] == 301])          # inside a Jupyter notebook

tour = [812, 813, 790, 791, 812]                        # edge_ids in driving order
roadmap.show_map(edges, tour=tour, tour_label="My greedy tour")

# draw only part of the network, but let the tour leave it:
roadmap.show_map(edges[edges["route"] == 301], tour=tour, network=edges)
```

In the map:

* **Colour by** switches between priority, speed limit, road type, one-way and salting route.
* The **Show priority** boxes hide or show priority classes.
* **Hover** a road to see its attributes, for both directions of a two-way street.
* A **tour** is drawn in black with arrows. Hover it to see in which step(s) each arc
  is driven. The panel shows the tour's length and time, and warns if the tour
  *jumps*, meaning an arc does not start where the previous one ended.

The map loads its background and the Leaflet library from the internet, so it needs
a connection. For the whole network the map is about 3 MB. Showing a subset keeps
notebooks small.

## 5. Notes for the Chinese Postman Problem

* **Which arcs must be salted** is your modelling choice, e.g. `priority in {1, 2}`.
* **Two-way streets.** The simplest model requires *both* arcs of a two-way street
  (one pass per direction). That is a *directed* CPP. A more realistic model says a
  narrow street is salted in one pass, so only *one* of the two arcs is required;
  `reverse_edge_id` tells you which arcs belong together. That is a *mixed* CPP.
* **Cost** can be `length_m` (distance) or `travel_time_s` (time). Driving an arc you
  have already salted ("deadheading") costs the same, but salts nothing new.
* **A depot.** Real routes start and end at a salt depot. Any node will do for the
  exercise, or you can pick the node closest to an address via `x_m` / `y_m`.
* **Checking a tour.** A valid tour is a list of edge_ids where each arc starts where
  the previous one ended (`u` of the next == `v` of the previous), and the last arc
  ends where the first started. `roadmap.show_map(..., tour=...)` checks this for you.

## 6. Where the data comes from, and its limitations

| Data | Source | Licence |
|---|---|---|
| Roads, names, speed limits, one-way streets | © OpenStreetMap contributors, downloaded via the Overpass API | ODbL – credit "© OpenStreetMap contributors" |
| Winter priorities and salting routes | Aalborg Kommune, winter-service map (KortInfo) | public map; credit Aalborg Kommune |

The download dates are in `dataset_info.json`. The priorities are joined onto the
OpenStreetMap roads by matching their shapes. Each road is compared every 5 m
with the municipality's map, which must be within 12 m and run the same
direction; street names break ties. This matched 97 % of the municipality's
salted km.

Limitations:

* **Speed limits.** About one third come from OpenStreetMap. The rest are estimated
  as the typical limit for that road type in Aalborg (usually 50 km/h in towns,
  80 km/h on rural roads).
* **Missing roads.** About 1 % of priority 1 – 3 roads have no OpenStreetMap road
  (mostly parking areas and harbour promenades). Some priority-4 gravel roads are
  missing too, as are the roads on the island **Egholm**, which is reached by ferry
  and not connected to the rest of the network.
* **Mixed priority.** Where the priority changes part-way along an arc, the arc gets
  the priority of most of its length.
* **Not ground truth.** Treat the data as realistic, not as the municipality's
  official plan.

---

### For teachers: rebuilding the dataset

The scripts in `road_net_data_for_students/` rebuild everything from scratch, about
5 minutes in total:

```
python 01_aalborg_osm_extraction.py   # newest OpenStreetMap roads   -> data/01_raw
python 02_aalborg_kortinfo_winter.py  # municipality's winter map    -> data/01_raw
python 03_join_winter_service.py      # match priorities onto roads  -> data/01_raw
python 04_make_student_dataset.py     # this folder                  -> data/02_student
```

`03_` also writes `aalborg_network_winter.gpkg` (open in QGIS) with the uncertain
matches for review, and `aalborg_join_report.json` with coverage statistics.
