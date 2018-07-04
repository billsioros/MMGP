import gmplot
from numpy import mean
from DBManagement import DBManager as DBM


DBManager = DBM("MMGP_Data.db", "AIzaSyBRGHJf69r2tYhvmpJxdayyXfZorTfHu5g")
sql = "Select GPS_X, GPS_Y From Address"
DBManager.Cursor.execute(sql)
Rows = DBManager.Cursor.fetchall()


Lats = list()
Lons = list()
for Lat, Lon in Rows:
    Lats.append(Lat)
    Lons.append(Lons)

gmap = gmplot.GoogleMapPlotter(mean(Lats), mean(Lons), 13)
name = "Plot.html"
gmap.plot(Lats, Lons, 'red', edge_width=5)
gmap.draw(name)