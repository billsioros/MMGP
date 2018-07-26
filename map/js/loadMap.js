
map = new OpenLayers.Map("map");
map.addLayer(new OpenLayers.Layer.OSM());

map.setCenter(new OpenLayers.LonLat(23.727539, 37.883810)
.transform(
    new OpenLayers.Projection("EPSG:4326"),
    map.getProjectionObject()
), 12);

var markers = new OpenLayers.Layer.Markers( "Markers" );
map.addLayer(markers);

var menu = document.getElementById("menu");

function createli(name, value, text)
{
    var label = document.createElement("label");
    var radio = document.createElement("input");
    radio.type = "radio";
    radio.name = name;
    radio.value = value;

    label.appendChild(radio);

    label.appendChild(document.createTextNode(text));

    var li = document.createElement("li");
    li.appendChild(label);

    return li;
}

var ul = document.createElement("ul");

for (var b = 0; b < schedules.length; b++)
{
    ul.appendChild(createli("busButton", b, "Bus No." + b.toString()));
}

menu.appendChild(ul);

var radios = document.getElementsByName("busButton");
for (var r = 0; r < radios.length; r++)
{
    radios[r].onclick = function()
    {
        markers.clearMarkers();

        for (var s = 0; s < schedules[this.value].students.length; s++)
        {
            var student = schedules[this.value].students[s];

            markers.addMarker(new OpenLayers.Marker(new OpenLayers.LonLat(student.longitude, student.latitude)
            .transform(
            new OpenLayers.Projection("EPSG:4326"),
            map.getProjectionObject()
            )));
        }
    }
}
