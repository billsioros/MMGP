import pyodbc
from util import GetCredentials, GetSetting, RowListToDict
import sys
from itertools import izip
from DBmanagement import DBManager as DBM
import os
import json


jsonRequest = sys.argv[1]

data = json.loads(jsonRequest)

Settings = data["Settings"]
Database = data["Database"]
Overwrite = data["Overwrite"]


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


StudentColumns = ["ID", "LastName", "FirstName", "Road", "Num", "ZipCode", "Prefec", "Muni", "Area", "Notes", "Level", "Class",\
      "ScheduleID", "BusSchedule", "ScheduleOrder", "ScheduleTime", "Mon", "Tue", "Wen", "Thu", "Fri",\
      "GPSX", "GPSY", "Phone", "Mobile", "OtherPhone1", "OtherPhone2"]

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
                  sched.SchCode,                \
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

      RowLists[key] = RowListToDict(RowLists[key], StudentColumns)
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


DBManager.InsertStudent(Tables, overwrite=Overwrite, GeoFailsFile=GeoFailsFile)
DBManager.Commit()

DBManager.Disconnect()
GeoFailsFile.close()