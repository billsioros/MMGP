import pyodbc
from util import GetCredentials
import sys
from itertools import izip
from DBmanagement import DBManager as DBM
import os
import json


fileName = sys.argv[1]


with open(fileName, "r") as json_file:
    data = json.load(json_file)

Settings = data["Settings"]
Database = data["Database"]

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



# Select All Morning Students

cursor = con.cursor()

ogDbTables = ["dbo.SRP_Morning_Students_NewYear", "dbo.SRP_Noon_Students_NewYear",
          "dbo.SRP_Study_Students_NewYear"]

# ogDbTables = ["dbo.SRP_Morning_Students_NewYear", "dbo.SRP_Morning_Students_OldYear", "dbo.SRP_Noon_Students_NewYear",
      #     "dbo.SRP_Noon_Students_OldYear", "dbo.SRP_Study_Students_NewYear", "dbo.SRP_Study_Students_OldYear"]

RowListKeys = ["Morning_NewYear", "Noon_NewYear", "Study_NewYear"]

# RowListKeys = ["Morning_NewYear", "Morning_OldYear", "Noon_NewYear", "Noon_OldYear", "Study_NewYear", "Study_OldYear"]
RowLists = dict()

for tableName, key in izip(ogDbTables, RowListKeys):
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
            Where stud.StCode = sched.StCode"

      cursor.execute(sql)
      RowLists[key] = cursor.fetchall()
      
# Select All Buses

con.close()


DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

DatabaseDir = os.path.realpath(os.path.dirname(Database))

GeoFailsFile = open(DatabaseDir + "/FormatFails.tsv", "w+")
GeoFailsFile.write("StudentID\tFormattedAddress\tFullAddress\tDayPart\n")

Tables = list()

for key in RowLists.keys():
      DayPart = key.replace("_NewYear", "")
      DayPart = DayPart.replace("_OldYear", "")
      Tables.append((RowLists[key], DayPart))


DBManager.InsertStudent(Tables, GeoFailsFile=GeoFailsFile)
DBManager.Commit()

DBManager.Disconnect()
GeoFailsFile.close()