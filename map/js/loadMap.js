
function createInput(type, name, value)
{
    var input   = document.createElement("input");

    input.type  = type;
    input.name  = name;
    input.value = value;

    return input;
}

function createLabel(content, text)
{
    var label = document.createElement("label");
    
    label.appendChild(content);
    label.appendChild(document.createTextNode(text));

    return label;
}

function createList(content, css)
{
    var list = document.createElement("ul");

    for (var cni = 0; cni < content.length; cni++)
    {
        var item = document.createElement("li");

        item.appendChild(content[cni]);

        list.appendChild(item);
    }

    list.classList.add(css);

    return list;
}

// Create map
map = new OpenLayers.Map("map");
map.addLayer(new OpenLayers.Layer.OSM());

map.setCenter(
    new OpenLayers.LonLat(23.727539, 37.883810).transform(
        new OpenLayers.Projection("EPSG:4326"),
        map.getProjectionObject()
    ),
12);

var markers = new OpenLayers.Layer.Markers("Markers");
map.addLayer(markers);

function plotStudents(schedules, sci, bsi)
{
    var _students = schedules[sci].buses[bsi].students;
            
    for (var sti = 0; sti < _students.length; sti++)
    {
        var student = _students[sti];

        markers.addMarker(
            new OpenLayers.Marker(
                new OpenLayers.LonLat(student.longitude, student.latitude).transform(
                    new OpenLayers.Projection("EPSG:4326"),
                    map.getProjectionObject()
                )
            )
        );
    }
}

// Create menu
var menu = document.getElementById("menu");

let _schedules = [];
for (let sci = 0; sci < schedules.length; sci++)
{
    var _buses = [];
    for (let bsi = 0; bsi < schedules[sci].buses.length; bsi++)
    {
        var radio = createInput("radio", "busButton", bsi);

        radio.onclick = function()
        {
            markers.clearMarkers();

            plotStudents(schedules, sci, bsi);
        }

        _buses.push(createLabel(radio, "Bus No." + radio.value));
    }

    var checkbox = createInput("checkbox", "scheduleButton", sci);
    var label    = createLabel(checkbox, "Schedule No." + checkbox.value);
    let list     = createList(_buses, "buses");

    checkbox.checked = true;
    if (sci)
    {
        checkbox.checked = false;
        list.classList.toggle("hidden");
    }

    checkbox.onclick = function()
    {        
        for (var sci = 0; sci < _schedules.length; sci++)
        {
            if (sci != this.value)
            {
                if (_schedules[sci].children[0].children[0].checked)
                {
                    _schedules[sci].children[0].children[0].checked = false;
                    _schedules[sci].children[1].classList.toggle("hidden");
                }
            }
        }
        
        list.classList.toggle("hidden");
    }

    var div = document.createElement("div");
    div.classList.add("container");

    div.appendChild(label);
    div.appendChild(list);

    _schedules.push(div);
}

menu.appendChild(createList(_schedules, "schedules"));
