import pyodbc
import sqlite3
import DBmanagement
from geopy import geocoders
import os

con = pyodbc.connect(DRIVER='{SQL Server Native Client 11.0}', 
                 SERVER='DESKTOP-GIANNIS\SQLEXPRESS', 
                 DATABASE='maliaras',
                 Trusted_Connection='yes', autocommit=True)

con.setdecoding(pyodbc.SQL_CHAR, encoding='greek')
con.setdecoding(pyodbc.SQL_WCHAR, encoding='greek')


# geolocator = geocoders.Bing(api_key="AnqIobiOKJjxYO1bwmdC4z04lX2kTfFEgKcKyj_9y-bTzjE1d6hoa_A2XNi-dd0R", format_string="%s, Attiki Greece")

# address, (lat, lon) = geolocator.geocode("Erechtheiou 6 Alimos, 17455")
# print (lat, lon)
# print address


# Select All Morning Students
sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Morning_Students_NewYear"

cursor = con.cursor().execute(sql)
NewYearMorning = cursor.fetchall()

sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Morning_Students_OldYear"

cursor = con.cursor().execute(sql)
OldYearMorning = cursor.fetchall()

# Select All Noon Students
sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Noon_Students_OldYear"

cursor = con.cursor().execute(sql)
OldYearNoon = cursor.fetchall()

sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Noon_Students_NewYear"

cursor = con.cursor().execute(sql)
NewYearNoon = cursor.fetchall()

sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Study_Students_NewYear"

cursor = con.cursor().execute(sql)
NewYearStudy = cursor.fetchall()

sql = "SELECT                       \
            StCode,                 \
            StLastName,             \
            StFirstName,            \
            SchAddress,             \
            SchAddressNumber,       \
            SchZipCode,             \
            PrefectureDescription,  \
            MunicipalDescription,   \
            AreaDescription,        \
            SchNotes,               \
            LevelDescription,       \
            ClassDescription,       \
            SchMonday,              \
            SchTuesday,             \
            SchWednesday,           \
            SchThursday,            \
            SchFriday,              \
            SchGPS_X,               \
            SchGPS_Y                \
      FROM dbo.SRP_Study_Students_OldYear"

cursor = con.cursor().execute(sql)
OldYearStudy = cursor.fetchall()

con.close()



# Create a new Database
DBmanagement.CreateDatabase("MMGP_Data.db")
Connection = DBmanagement.Connect("MMGP_Data.db")
Connection.row_factory = sqlite3.Row
Cursor = Connection.cursor()


for row in OldYearMorning:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Morning')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)

for row in OldYearNoon:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Noon')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)

for row in NewYearMorning:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Morning')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)

for row in NewYearNoon:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Noon')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)

for row in OldYearStudy:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Study')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)

for row in NewYearStudy:
    iterable = list()
    for i in range(len(row)):
        iterable.append(row[i])
    iterable.append(1.2)
    iterable.append(1.3)
    iterable.append('Study')
    cursor.execute("INSERT INTO Student     \
                    VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", iterable)


con.commit()

sql = "Select Address, AddressNum, ZipCode, Municipal, Area From Student"
cursor.execute(sql)

Rows = cursor.fetchall()

i = 0
for Address, Num, ZipCode, Municipal, Area in Rows:
    print i
    FullAddress = list()

    if not Address:
        continue
    else:
        FullAddress.append(Address)
        FullAddress.append(Num)
    if ZipCode:
        FullAddress.append(str(ZipCode))
    if Municipal and not Area:
        FullAddress.append(Municipal)
    if Area:
        FullAddress.append(Area)
    
    AddressString = ""
    for string in FullAddress:
        if string:
            AddressString += string + " " 

    print AddressString
    i += 1

con.close()

        
