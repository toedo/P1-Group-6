"""
roadmap.py – interactive map of the Aalborg road network (one self-contained HTML file).

Works with the student dataset (edges.csv / edges.parquet), the whole network or any
subset of it, and can draw your own tour on top.

    import pandas as pd, roadmap
    edges = pd.read_csv("edges.csv")

    roadmap.save_map(edges, path="map.html")                 # open map.html in a browser
    roadmap.show_map(edges)                                  # inside a Jupyter notebook

    small = edges[edges["route"] == 301]                     # any filtered DataFrame works
    roadmap.show_map(small)

    tour = [12, 13, 87, 88, 90]                              # edge_ids in the order you drive them
    roadmap.show_map(edges, tour=tour, tour_label="Greedy tour")
    roadmap.show_map(small, tour=tour, network=edges)        # subset as background, tour may leave it

In the map:
  * "Colour by" switches between priority, speed limit, road type, one-way and route
  * the check boxes hide/show priority classes
  * hover a road to see all its attributes (both directions of a two-way street)
  * a tour is drawn in black with arrows; hovering it shows in which step(s) each arc is driven

Only pandas and shapely are needed. The map itself loads Leaflet and the
background map from the internet when it is opened.
"""

import html
import json
from pathlib import Path

import numpy as np
import pandas as pd
import shapely

# ------------------------------------------------------------------
# Colours (one blue scale for ordered values, fixed categorical slots for types)
# ------------------------------------------------------------------
PRIORITY_STYLE = {   # priority: (colour, line width, label)
    1: ("#0d366b", 3.5, "1 – highest"),
    2: ("#256abf", 3.0, "2"),
    3: ("#5598e7", 2.5, "3"),
    4: ("#86b6ef", 2.0, "4 – lowest"),
    0: ("#c3c2b7", 1.2, "0 – not salted by the municipality"),
}
SPEED_BINS = [   # (upper limit km/h, colour, label) – one blue scale, light = slow
    (30, "#86b6ef", "≤ 30"), (40, "#5598e7", "40"), (50, "#2a78d6", "50"),
    (70, "#1c5cab", "60–70"), (999, "#0d366b", "≥ 80"),
]
ROAD_TYPE_GROUPS = [   # (label, OSM road types, colour) – fixed order; None = everything else
    ("Major (motorway – secondary)", ["motorway", "motorway_link", "trunk", "trunk_link", "primary",
                                      "primary_link", "secondary", "secondary_link"], "#2a78d6"),
    ("Tertiary", ["tertiary", "tertiary_link"], "#eb6834"),
    ("Unclassified (mostly rural)", ["unclassified"], "#1baf7a"),
    ("Residential & other", None, "#c3c2b7"),
]
ONEWAY_STYLE = {False: ("#c3c2b7", "Two-way"), True: ("#eb6834", "One-way")}
TOUR_COLOUR = "#0b0b0b"


# ------------------------------------------------------------------
# Data preparation
# ------------------------------------------------------------------
def _coords_latlng(nodes, edges):
    """List of [[lat, lng], ...] per edge, from WKT geometry or (fallback) node coordinates."""
    if "geometry" in edges.columns and edges["geometry"].notna().all():
        g = shapely.from_wkt(edges["geometry"].values)
        out = []
        for line in g:
            c = np.round(shapely.get_coordinates(line)[:, ::-1], 5)
            out.append(c.tolist())
        return out
    if nodes is None:
        raise ValueError("edges has no 'geometry' column – pass nodes=... so lines can be drawn")
    pos = nodes.set_index("node_id")[["lat", "lon"]]
    return [[pos.loc[u].round(5).tolist(), pos.loc[v].round(5).tolist()]
            for u, v in zip(edges["u"], edges["v"])]


def _road_group(t):
    for i, (_, members, _) in enumerate(ROAD_TYPE_GROUPS):
        if members and t in members:
            return i
    return len(ROAD_TYPE_GROUPS) - 1


def _arrow(latlngs, at=0.55, size_m=9.0):
    """Two short wing lines forming an arrowhead along a [lat, lng] polyline."""
    p = np.array(latlngs, dtype=float)
    lat0 = np.radians(p[:, 0].mean())
    xy = np.c_[p[:, 1] * 111320 * np.cos(lat0), p[:, 0] * 110540]
    seg = np.diff(xy, axis=0)
    seglen = np.hypot(seg[:, 0], seg[:, 1])
    if seglen.sum() == 0:
        return None
    target = at * seglen.sum()
    i = min(np.searchsorted(np.cumsum(seglen), target), len(seg) - 1)
    t = (target - (np.cumsum(seglen)[i] - seglen[i])) / max(seglen[i], 1e-9)
    tip = xy[i] + t * seg[i]
    d = seg[i] / max(seglen[i], 1e-9)
    wings = []
    for ang in (150, -150):
        a = np.radians(ang)
        r = np.array([d[0] * np.cos(a) - d[1] * np.sin(a), d[0] * np.sin(a) + d[1] * np.cos(a)])
        w = tip + size_m * r
        wings.append([[round(w[1] / 110540, 6), round(w[0] / (111320 * np.cos(lat0)), 6)],
                      [round(tip[1] / 110540, 6), round(tip[0] / (111320 * np.cos(lat0)), 6)]])
    return wings


def _prepare(edges, nodes, tour, network=None):
    e = edges.copy()
    for col, default in (("priority", 0), ("speed_kmh", 0), ("oneway", False), ("route", None),
                         ("name", None), ("road_type", "unknown"), ("reverse_edge_id", -1),
                         ("length_m", 0.0)):
        if col not in e.columns:
            e[col] = default
    e = e.reset_index(drop=True)
    e["reverse_edge_id"] = e["reverse_edge_id"].fillna(-1).astype(int)
    # route may be int (-1 = none) or text; turn it into text with None for "no route"
    e["route"] = [None if (pd.isna(r) or str(r) in ("-1", "")) else str(r) for r in e["route"]]
    present = set(e["edge_id"])
    has_twin = e["reverse_edge_id"].isin(present)
    # draw each physical street once: one-way arcs + the lower-id arc of each two-way pair
    piece = e[~has_twin | (e["edge_id"] < e["reverse_edge_id"])].reset_index(drop=True)
    speed_by_id = dict(zip(e["edge_id"], e["speed_kmh"]))

    coords = _coords_latlng(nodes, piece)
    names = list(pd.unique(piece["name"].fillna("").astype(str)))
    name_ix = {n: i for i, n in enumerate(names)}
    routes = sorted(pd.unique(e["route"].dropna().astype(str)), key=lambda r: (len(r), r))
    route_ix = {r: i for i, r in enumerate(routes)}
    types = list(pd.unique(piece["road_type"].fillna("unknown").astype(str)))
    type_ix = {t: i for i, t in enumerate(types)}

    props = []
    for r in piece.itertuples(index=False):
        rid = int(r.reverse_edge_id) if r.reverse_edge_id in present else -1
        back = int(speed_by_id[rid]) if rid >= 0 else -1
        props.append([
            name_ix[str(r.name) if pd.notna(r.name) else ""],
            type_ix[str(r.road_type) if pd.notna(r.road_type) else "unknown"],
            int(r.priority), int(r.speed_kmh), back, round(float(r.length_m), 1), int(bool(r.oneway)),
            route_ix[str(r.route)] if pd.notna(r.route) else -1, int(r.edge_id), rid,
        ])
    data = {
        "names": names, "types": types, "routes": routes,
        "type_group": [_road_group(t) for t in types],
        "coords": coords, "props": props,
    }

    if tour is not None and len(tour):
        tour = [int(t) for t in tour]
        lookup = e if network is None else pd.concat([e, network[~network["edge_id"].isin(present)]])
        by_id = lookup.set_index("edge_id")
        missing = [t for t in tour if t not in by_id.index]
        if missing:
            raise ValueError(f"tour contains edge_ids that are not in the table you passed: {missing[:10]}. "
                             "If you are drawing a subset, pass the full table too: network=edges")
        uniq = list(dict.fromkeys(tour))
        tcoords = _coords_latlng(nodes, by_id.loc[uniq].reset_index())
        steps = {}
        for k, t in enumerate(tour, start=1):
            steps.setdefault(t, []).append(k)
        t_len = by_id.loc[tour, "length_m"].sum()
        t_time = by_id.loc[tour, "travel_time_s"].sum() if "travel_time_s" in by_id.columns else None
        broken = sum(1 for a, b in zip(tour, tour[1:]) if by_id.at[a, "v"] != by_id.at[b, "u"]) \
            if {"u", "v"}.issubset(by_id.columns) else 0
        data["tour"] = {
            "coords": tcoords,
            "arrows": [_arrow(c) for c in tcoords],
            "info": [[int(t), steps[t], name_ix.get(str(by_id.at[t, "name"]), -1)
                      if pd.notna(by_id.at[t, "name"]) else -1] for t in uniq],
            "n_steps": len(tour), "km": round(float(t_len) / 1000, 2),
            "hours": None if t_time is None else round(float(t_time) / 3600, 2),
            "broken": int(broken),
        }
    return data


# ------------------------------------------------------------------
# HTML
# ------------------------------------------------------------------
_TEMPLATE = r"""<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<title>__TITLE__</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css">
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
<style>
  :root { --surface:#fcfcfb; --page:#f9f9f7; --ink:#0b0b0b; --ink2:#52514e; --hair:#e1e0d9; }
  html,body { margin:0; height:100%; font:14px/1.4 system-ui,-apple-system,"Segoe UI",sans-serif; color:var(--ink); background:var(--page); }
  #bar { display:flex; flex-wrap:wrap; gap:6px 18px; align-items:center; padding:8px 12px; background:var(--surface); border-bottom:1px solid var(--hair); }
  #bar h1 { font-size:15px; margin:0 8px 0 0; font-weight:600; }
  #bar label { color:var(--ink2); }
  #bar select { font:inherit; padding:2px 4px; }
  .chk { display:inline-flex; gap:10px; }
  #map { position:absolute; top:var(--barh,44px); bottom:0; left:0; right:0; background:var(--surface); }
  .panel { background:var(--surface); border:1px solid var(--hair); border-radius:6px; padding:8px 10px; box-shadow:0 1px 3px rgba(0,0,0,.08); }
  #info { min-width:230px; max-width:300px; }
  #info .muted, .legend .muted { color:var(--ink2); }
  #info table { border-collapse:collapse; } #info td { padding:1px 8px 1px 0; vertical-align:top; }
  #info td:first-child { color:var(--ink2); }
  .legend { line-height:1.6; }
  .legend .sw { display:inline-block; width:22px; height:0; border-top:4px solid; border-radius:2px; vertical-align:middle; margin-right:6px; }
  .legend .km { color:var(--ink2); margin-left:6px; font-variant-numeric:tabular-nums; }
  .legend b { display:block; margin-bottom:2px; }
</style></head>
<body>
<div id="bar">
  <h1>__TITLE__</h1>
  <label>Colour by
    <select id="mode">
      <option value="priority">Priority (salting)</option>
      <option value="speed">Speed limit</option>
      <option value="type">Road type</option>
      <option value="oneway">One-way / two-way</option>
      <option value="route">Salting route…</option>
    </select></label>
  <label id="routebox" style="display:none">Route <select id="route"></select></label>
  <span class="chk" id="prio"><span class="muted" style="color:var(--ink2)">Show priority</span></span>
</div>
<div id="map"></div>
<script>
const DATA = __DATA__;
const STYLE = __STYLE__;
const TOUR_LABEL = __TOUR_LABEL__;
let tourLayer = null;
document.documentElement.style.setProperty('--barh', document.getElementById('bar').offsetHeight + 'px');

const map = L.map('map', {preferCanvas:true, zoomSnap:0.25});
L.tileLayer('https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}{r}.png', {
  maxZoom: 20, subdomains:'abcd',
  attribution:'Roads © <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors · Map © <a href="https://carto.com/attributions">CARTO</a> · Winter service © Aalborg Kommune'
}).addTo(map);
const renderer = L.canvas({tolerance: 6, padding: 0.3});

// ---------- styling per mode
function speedBin(s){ for (let i=0;i<STYLE.speed.length;i++) if (s <= STYLE.speed[i][0]) return i; return STYLE.speed.length-1; }
function classOf(p, mode){
  if (mode==='priority') return String(p[2]);
  if (mode==='speed') return String(speedBin(p[3]));
  if (mode==='type') return String(DATA.type_group[p[1]]);
  if (mode==='oneway') return String(p[6]);
  if (mode==='route') return p[7] === selRoute ? 'sel' : (p[2] > 0 ? 'other' : 'none');
}
function styleOf(cls, mode){
  if (mode==='priority'){ const s = STYLE.priority[cls]; return {color:s[0], weight:s[1]}; }
  if (mode==='speed'){ return {color:STYLE.speed[+cls][1], weight:2.5}; }
  if (mode==='type'){ return {color:STYLE.type[+cls][2], weight: +cls===3 ? 1.4 : 2.6}; }
  if (mode==='oneway'){ const s = STYLE.oneway[cls]; return {color:s[0], weight: cls==='1'?2.8:1.6}; }
  if (mode==='route'){ return cls==='sel' ? {color:'#2a78d6', weight:4} : cls==='other' ? {color:'#c3c2b7', weight:1.6} : {color:'#e1e0d9', weight:1}; }
}
function legendItems(mode){
  if (mode==='priority') return ['1','2','3','4','0'].map(k=>[k, STYLE.priority[k][0], STYLE.priority[k][2]]);
  if (mode==='speed') return STYLE.speed.map((s,i)=>[String(i), s[1], s[2]+' km/h']);
  if (mode==='type') return STYLE.type.map((s,i)=>[String(i), s[2], s[0]]);
  if (mode==='oneway') return [['0', STYLE.oneway['0'][0], STYLE.oneway['0'][1]], ['1', STYLE.oneway['1'][0], STYLE.oneway['1'][1]]];
  if (mode==='route') return [['sel','#2a78d6','Route '+(DATA.routes[selRoute] ?? '–')], ['other','#c3c2b7','Other salted roads'], ['none','#e1e0d9','Not salted']];
}

// ---------- layers
const lines = DATA.coords.map((c,i) => {
  const l = L.polyline(c, {renderer, opacity:0.95, lineCap:'round', lineJoin:'round'});
  l._i = i;
  l.on('mouseover', e => { hover(i, l); }).on('mouseout', () => unhover(l));
  return l;
});
const group = L.layerGroup(lines).addTo(map);
let mode = 'priority', selRoute = -1;
const visible = {0:true,1:true,2:true,3:true,4:true};

function km(x){ return (x/1000).toLocaleString('en', {maximumFractionDigits:1}); }
function restyle(){
  const totals = {};
  lines.forEach(l => {
    const p = DATA.props[l._i];
    const show = visible[p[2]];
    if (show && !group.hasLayer(l)) group.addLayer(l);
    if (!show && group.hasLayer(l)) group.removeLayer(l);
    if (!show) return;
    const cls = classOf(p, mode);
    l._base = styleOf(cls, mode);
    l._base.opacity = DATA.tour ? 0.35 : 0.95;   // fade the network so a tour stands out
    l.setStyle(l._base);
    totals[cls] = (totals[cls]||0) + p[5];
  });
  if (mode==='route') lines.forEach(l => { if (DATA.props[l._i][7]===selRoute && group.hasLayer(l)) l.bringToFront(); });
  if (mode==='priority') [4,3,2,1].forEach(k => lines.forEach(l => { if (DATA.props[l._i][2]===k && group.hasLayer(l)) l.bringToFront(); }));
  if (tourLayer) tourLayer.eachLayer(x => x.bringToFront());
  legend.update(totals);
}

// ---------- hover info
const info = L.control({position:'topright'});
info.onAdd = function(){ this._div = L.DomUtil.create('div','panel'); this._div.id='info'; this.clear(); return this._div; };
info.clear = function(){ this._div.innerHTML = '<span class="muted">Hover a road to see its attributes.</span>' + tourSummary(); };
info.addTo(map);
function esc(s){ return String(s).replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c])); }
function hover(i, l){
  const p = DATA.props[i];
  l.setStyle({weight:(l._base?.weight||2)+3});
  const name = DATA.names[p[0]] || '<span class="muted">(no name)</span>';
  const speed = p[4] >= 0 && p[4] !== p[3] ? `${p[3]} → / ${p[4]} ← km/h` : `${p[3]} km/h`;
  const ids = p[9] >= 0 ? `${p[8]} (→) and ${p[9]} (←)` : `${p[8]} (one-way)`;
  info._div.innerHTML = `<b>${name}</b><table>
    <tr><td>Priority</td><td>${p[2] || '0 (not salted)'}</td></tr>
    <tr><td>Salting route</td><td>${p[7] >= 0 ? esc(DATA.routes[p[7]]) : '–'}</td></tr>
    <tr><td>Speed limit</td><td>${speed}</td></tr>
    <tr><td>Road type</td><td>${esc(DATA.types[p[1]])}</td></tr>
    <tr><td>Length</td><td>${p[5].toLocaleString('en')} m</td></tr>
    <tr><td>Direction</td><td>${p[6] ? 'one-way' : 'two-way'}</td></tr>
    <tr><td>edge_id</td><td>${ids}</td></tr></table>` + tourSummary();
}
function unhover(l){ if (l._base) l.setStyle(l._base); info.clear(); }

// ---------- legend
const legend = L.control({position:'bottomright'});
legend.onAdd = function(){ this._div = L.DomUtil.create('div','panel legend'); return this._div; };
legend.update = function(totals){
  const titles = {priority:'Winter priority', speed:'Speed limit', type:'Road type', oneway:'Direction', route:'Salting route'};
  let h = `<b>${titles[mode]}</b>`;
  legendItems(mode).forEach(([k,c,label]) => {
    h += `<div><span class="sw" style="border-color:${c}"></span>${esc(label)}<span class="km">${km(totals[k]||0)} km</span></div>`;
  });
  if (DATA.tour) h += `<div><span class="sw" style="border-color:#0b0b0b"></span>${esc(TOUR_LABEL)}</div>`;
  h += `<div class="muted" style="margin-top:4px;font-size:12px">km = street length, each street counted once</div>`;
  this._div.innerHTML = h;
};
legend.addTo(map);

// ---------- tour overlay
function tourSummary(){
  if (!DATA.tour) return '';
  const t = DATA.tour;
  return `<div style="margin-top:8px;border-top:1px solid var(--hair);padding-top:6px"><b>${esc(TOUR_LABEL)}</b><br>
    ${t.n_steps.toLocaleString('en')} arcs · ${t.km.toLocaleString('en')} km` + (t.hours!==null ? ` · ${t.hours} h at the speed limit` : '') +
    (t.broken ? `<br><span style="color:#d03238">⚠ ${t.broken} jump(s): an arc does not start where the previous one ended</span>` : '') + `</div>`;
}
if (DATA.tour){
  tourLayer = L.layerGroup().addTo(map);
  DATA.tour.coords.forEach((c, j) => {
    const [eid, steps, ni] = DATA.tour.info[j];
    const casing = L.polyline(c, {renderer, color:'#ffffff', weight:7, opacity:0.9, interactive:false});
    const line = L.polyline(c, {renderer, color:'#0b0b0b', weight:3.5, opacity:1});
    const shown = steps.length > 8 ? steps.slice(0,8).join(', ') + ', …' : steps.join(', ');
    line.bindTooltip(`<b>${ni>=0 ? esc(DATA.names[ni]) : 'edge'} </b> · edge_id ${eid}<br>driven in step ${shown}` + (steps.length>1 ? ` (${steps.length}×)` : ''), {sticky:true});
    tourLayer.addLayer(casing); tourLayer.addLayer(line);
    (DATA.tour.arrows[j] || []).forEach(w => tourLayer.addLayer(L.polyline(w, {renderer, color:'#0b0b0b', weight:2.5, interactive:false})));
  });
}

// ---------- controls
const prioBox = document.getElementById('prio');
[1,2,3,4,0].forEach(k => {
  const lab = document.createElement('label');
  lab.innerHTML = `<input type="checkbox" checked data-k="${k}"> ${k}`;
  lab.querySelector('input').addEventListener('change', ev => { visible[k] = ev.target.checked; restyle(); });
  prioBox.appendChild(lab);
});
const routeSel = document.getElementById('route');
DATA.routes.forEach((r,i) => { const o = document.createElement('option'); o.value=i; o.textContent=r; routeSel.appendChild(o); });
routeSel.addEventListener('change', () => {
  selRoute = +routeSel.value; restyle();
  const b = L.latLngBounds([]); lines.forEach(l => { if (DATA.props[l._i][7]===selRoute) b.extend(l.getBounds()); });
  if (b.isValid()) map.fitBounds(b, {padding:[30,30]});
});
document.getElementById('mode').addEventListener('change', ev => {
  mode = ev.target.value;
  document.getElementById('routebox').style.display = mode==='route' ? '' : 'none';
  if (mode==='route' && selRoute < 0 && DATA.routes.length){ selRoute = 0; routeSel.value = 0; routeSel.dispatchEvent(new Event('change')); return; }
  restyle();
});

// ---------- start
const all = L.latLngBounds([]); lines.forEach(l => all.extend(l.getBounds()));
if (tourLayer){ const tb = L.latLngBounds([]); tourLayer.eachLayer(x => tb.extend(x.getBounds())); map.fitBounds(tb, {padding:[30,30]}); }
else map.fitBounds(all, {padding:[20,20]});
restyle();
</script></body></html>
"""


def map_html(edges, nodes=None, tour=None, title="Aalborg road network", tour_label="Tour", network=None):
    """Return the map as an HTML string.

    edges       the arcs to draw (the whole table or any filtered part of it)
    nodes       only needed if edges has no 'geometry' column
    tour        list of edge_ids in driving order, drawn on top in black
    network     the full edges table, used to find tour arcs that are not in `edges`
    """
    data = _prepare(edges, nodes, tour, network)
    style = {
        "priority": {str(k): list(v) for k, v in PRIORITY_STYLE.items()},
        "speed": [list(b) for b in SPEED_BINS],
        "type": [[lab, None, col] for lab, _, col in ROAD_TYPE_GROUPS],
        "oneway": {str(int(k)): list(v) for k, v in ONEWAY_STYLE.items()},
    }
    return (_TEMPLATE
            .replace("__TITLE__", html.escape(title))
            .replace("__TOUR_LABEL__", json.dumps(tour_label))
            .replace("__STYLE__", json.dumps(style))
            .replace("__DATA__", json.dumps(data, separators=(",", ":"))))


def save_map(edges, path="map.html", nodes=None, tour=None, title="Aalborg road network", tour_label="Tour",
             network=None):
    """Write the map to an HTML file and return its path. Open the file in any browser.
    Arguments as in map_html()."""
    path = Path(path)
    path.write_text(map_html(edges, nodes, tour, title, tour_label, network), encoding="utf-8")
    return path


def show_map(edges, nodes=None, tour=None, title="Aalborg road network", tour_label="Tour", height=650,
             network=None):
    """Display the map inside a Jupyter notebook (works in JupyterLab, classic Jupyter and VS Code).

    Tip: for the whole network the map is a few MB; showing a subset keeps the notebook small."""
    import warnings
    from IPython.display import HTML
    page = map_html(edges, nodes, tour, title, tour_label, network)
    with warnings.catch_warnings():   # IPython suggests IFrame, but IFrame cannot embed the page itself
        warnings.simplefilter("ignore")
        return HTML(f'<iframe srcdoc="{html.escape(page, quote=True)}" '
                f'style="width:100%;height:{int(height)}px;border:1px solid #e1e0d9;border-radius:6px"></iframe>')
