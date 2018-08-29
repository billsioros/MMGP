import pyodbc
from util import GetCredentials, GetSetting
import sys
import os
from itertools import izip
from DBmanagement import DBManager as DBM
import json

fileName = sys.argv[1]

with open(fileName, "r") as json_file:
    data = json.load(json_file)

Settings = data["Settings"]
Database = data["Database"]

Active = GetSetting(Settings, [["Current_Year", "Active"]])
Active = Active[0]
TableNames = GetSetting(Settings, [["Current_Year", Active, "Table_Names"]])
TableNames = TableNames[0]
ActiveCon, GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)


constr =    "Driver=" + ServerType + ";" + \
            "Server=" + ServerName + ";" + \
            "Database=" + DatabaseName + ";"

if ActiveCon == "Native" or ActiveCon == "Native-Laptop":
      con = pyodbc.connect(   DRIVER=ServerType,
                              SERVER=ServerName,
                              Database=DatabaseName,
                              Trusted_Connection = 'yes',
                              autocommit=True)
else:
      if Username and Password:
            constr +=   ("UID=" + Username + ";" + \
                        "PWD=" + Password + ";")
      else:
            print "Error: wrong username/password"
      con = pyodbc.connect(constr, autocommit=True, timeout=120)

print "Connected."
con.setdecoding(pyodbc.SQL_CHAR, encoding='greek')
con.setdecoding(pyodbc.SQL_WCHAR, encoding='greek')

# Select All Morning Students

cursor = con.cursor()


RowListKeys = []
for tableName in TableNames:
      name = tableName.replace("dbo.SRP_", "")
      name = name.replace("Students_", "")
      RowListKeys.append(name)

RowLists = dict()

for tableName, key in izip(TableNames, RowListKeys):
      sql = "Select                             \
                  sched.StCode,                 \
                  sched.StLastName,             \
                  sched.StFirstName,            \
                  sched.SchAddress,             \
                  sched.SchAddressNumber,       \
                  sched.SchZipCode,             \
                  sched.PrefectureDescription,  \
                  sched.MunicipalDescription,   \
                  sched.AreaDescription,        \
                  sched.SchNotes,               \
                  sched.LevelDescription,       \
                  sched.ClassDescription,       \
                  sched.ScheduleName,           \
                  sched.SchStudentOrder,        \
                  sched.SchStudentTime,         \
                  sched.SchMonday,              \
                  sched.SchTuesday,             \
                  sched.SchWednesday,           \
                  sched.SchThursday,            \
                  sched.SchFriday,              \
                  sched.SchGPS_X,               \
                  sched.SchGPS_Y,               \
                  stud.StContactPhone,          \
                  stud.StContactMobile,         \
                  stud.StOtherPhone1,           \
                  stud.StOtherPhone2            \
            From " + tableName + " as sched, dbo.Student as stud     \
            Where stud.StCode = sched.StCode \
            Order By sched.StLastName"

      cursor.execute(sql)
      RowLists[key] = cursor.fetchall()
      
# Select All Buses

sql = "Select BusCode, BusNumber, BusStudentSites     \
       From dbo.Bus"

cursor.execute(sql)
Buses = cursor.fetchall()

con.close()

# Create a new Database

DBManager = DBM(Database, new=True, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

print "Inserting Buses"
DBManager.InsertBus(Buses)
DBManager.Commit()
print "Buses Inserted"

DatabaseDir = os.path.realpath(os.path.dirname(Database))

GeoFailsFile = open(DatabaseDir + "/FormatFails.tsv", "w+")
GeoFailsFile.write("StudentID\tLastName\tFirstName\tFormattedAddress\tFullAddress\tDayPart\n")

Tables = list()

for key in RowLists.keys():
      DayPart = key.replace("_NewYear", "")
      DayPart = DayPart.replace("_OldYear", "")
      Tables.append((RowLists[key], DayPart))

print "Inserting Students"
DBManager.InsertStudent(Tables, overwrite=True, GeoFailsFile=GeoFailsFile)
DBManager.Commit()
print "Students Inserted"

DBManager.InsertDepot([["ERECHTHIOU", "6", "17455", "ATTIKIS", "ALIMOY", None]])
DBManager.Commit()

for DayPart in ["Morning", "Noon", "Study"]:
      print "Inserting " + DayPart + " Distances"
      DBManager.InsertDistances(DayPart, direct=True)
      print DayPart + " Distances Inserted"


DBManager.Commit()

DBManager.Disconnect()
GeoFailsFile.close()
