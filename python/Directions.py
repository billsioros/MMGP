from DBManagement import DBManager as dbm
import sys
from util import GetCredentials, MapsHandler
import polyline
import gmplot
from numpy import mean
from itertools import izip

fileName = sys.argv[1]
rowIndex = sys.argv[2]
DayPart = sys.argv[3]

GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(fileName, rowIndex)

dbmanager = dbm("MMGP_data.db", GoogleAPI_key, OpenAPI_key)

dbmanager.Cursor.execute(\
                " Select Address.AddressID, Address.GPS_Y, Address.GPS_X    \
                From Address                                                \
                Where Address.ZipCode = '17455' and exists (  Select *      \
                                From Student                                \
                                Where Student.AddressID = Address.AddressID \
                                and Student.DayPart = ?)", [DayPart])

rows = dbmanager.Cursor.fetchall()

rows = rows[0:len(rows)/2]

mh = MapsHandler(GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

waypoints = list()
for id, y, x in rows:
    waypoints.append((y, x))


results = mh.Directions("Erechthiou 5, Alimos 17455", "Chrisostomou Smirnis 15, Agios Dimitrios 17341", waypoints)
line = results[0]["overview_polyline"]["points"]
line = polyline.decode(line)

lats = list()
lons = list()
file = open("PathToPlot.csv", "w+")
file.write("Lat,Lon\n")
for lat, lon in line:
    file.write(str(lat) + "," + str(lon) + "\n")
    lats.append(lat)
    lons.append(lon)

gmap = gmplot.GoogleMapPlotter(mean(lats), mean(lons), 13)
name = "DirectionsTest.html"
#gmap.plot(lats, lons, 'green', edge_width=5)


file = open("CoordsToPlot.csv", "w+")
file.write("Lat,Lon\n")
for _id, lat, lon in rows:
    file.write(str(lat) + "," + str(lon) + "\n")
    gmap.marker(lat, lon)
gmap.draw(name)
        