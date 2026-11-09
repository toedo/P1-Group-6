OSMIUM=./osmium-tool/build/osmium
DK_FULL_PBF=./tmp/denmark-latest.osm.pbf
AALBORG_GEOJSON=./tmp/aalborg.geojson
cd "$(dirname "$0")"

mkdir tmp
if test -e "$DK_FULL_PBF"; then
	echo "Denmark dataset is already downloaded"
else
	echo "Downloading Denmark dataset"
	wget https://download.geofabrik.de/europe/denmark-latest.osm.pbf -O $DK_FULL_PBF
fi

if test -e "$AALBORG_GEOJSON"; then
	echo "Aalborg geojson already found"
else
	echo "Finding Aalborg geojson"
	$OSMIUM export $DK_FULL_PBF -c osmium-config-export -o $AALBORG_GEOJSON --overwrite
	sed -i '2d' $AALBORG_GEOJSON # Deletes line 2 which is a geothing that isn't needed and doesn't play nice with the next command
fi
echo "Making a file of things only in the Aalborg geojson"
$OSMIUM extract $DK_FULL_PBF -c osmium-config-extract --overwrite
echo "Uncompressing"
$OSMIUM cat ./tmp/aalborg.osm.pbf -o ./aalborg.osm --overwrite