import gmplot
from numpy import mean
from DBManagement import DBManager as DBM


DBManager = DBM("MMGP_Data.db", "AIzaSyBRGHJf69r2tYhvmpJxdayyXfZorTfHu5g")
sql = "Select GPS_X, GPS_Y, FullAddress, FormattedAddress From Address"
DBManager.Cursor.execute(sql)
Rows = DBManager.Cursor.fetchall()

csvfile = open("CoordsToPlot.tsv", "w+")
csvfile.write("lng\tlat\tFAddress\tAddress\n")


for Lat, Lon, Address, FAdress in Rows:
    csvfile.write(str(Lat) + "\t" + str(Lon) + "\t" + str(FAdress) + "\t" + str(Address) + "\n")


csvfile.close
