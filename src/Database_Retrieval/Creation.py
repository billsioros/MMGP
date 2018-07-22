import pyodbc
import sqlite3
from DBManagement import DBManager as DBM
from util import GetCredentials
import os
import sys
import csv
from itertools import izip

fileName = sys.argv[1]
rowIndex = sys.argv[2]


GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(fileName, rowIndex)


con = pyodbc.connect(DRIVER=ServerType, 
                 SERVER=ServerName, 
                 DATABASE=DatabaseName,
                 Trusted_Connection='yes', autocommit=True)

con.setdecoding(pyodbc.SQL_CHAR, encoding='greek')
con.setdecoding(pyodbc.SQL_WCHAR, encoding='greek')

# Select All Morning Students

cursor = con.cursor() 

ogDbTables = ["dbo.SRP_Morning_Students_NewYear", "dbo.SRP_Morning_Students_OldYear", "dbo.SRP_Noon_Students_NewYear",
          "dbo.SRP_Noon_Students_OldYear", "dbo.SRP_Study_Students_NewYear", "dbo.SRP_Study_Students_OldYear"]
RowListKeys = ["Morning_NewYear", "Morning_OldYear", "Noon_NewYear", "Noon_OldYear", "Study_NewYear", "Study_OldYear"]
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

sql = "Select BusCode, BusNumber, BusStudentSites     \
       From dbo.Bus"

cursor.execute(sql)
Buses = cursor.fetchall()

con.close()

# Create a new Database
DBManager = DBM("MMGP_Data.db", GoogleAPI_key, OpenAPI_key)


DBManager.InsertBus(Buses)
DBManager.Commit()

GeoFailsFile = open("FormatFails.tsv", "w+")
GeoFailsFile.write("StudentID\tFormattedAddress\tFullAddress\tDayPart\n")

Tables = list()

for key in RowLists.keys():
      DayPart = key.replace("_NewYear", "")
      DayPart = DayPart.replace("_OldYear", "")
      Tables.append((RowLists[key], DayPart))


DBManager.InsertStudent(Tables, GeoFailsFile=GeoFailsFile)
DBManager.Commit()

DBManager.InsertDepot([["ERECHTHIOU", "6", "17455", "ATTIKIS", "ALIMOY", None]])
DBManager.Commit()

for DayPart in ["Morning", "Noon", "Study"]:
      DBManager.InsertDistances(DayPart, direct=True)


DBManager.Commit()

DBManager.Disconnect()
GeoFailsFile.close()
