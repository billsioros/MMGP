import pyodbc
import sqlite3
from DBManagement import DBManager as DBM
from util import GetCredentials
import os
import sys
import csv

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

cursor.execute(sql)
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

cursor.execute(sql)
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

cursor.execute(sql)
NewYearNoon = cursor.fetchall()

# Select All Study Students

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

cursor.execute(sql)
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

cursor.execute(sql)
OldYearStudy = cursor.fetchall()

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
Tables.append((OldYearMorning, "Morning"))
Tables.append((NewYearMorning, "Morning"))
Tables.append((OldYearNoon, "Noon"))
Tables.append((NewYearNoon, "Noon"))
Tables.append((OldYearStudy, "Study"))
Tables.append((NewYearStudy, "Study"))


DBManager.InsertStudent(Tables, GeoFailsFile=GeoFailsFile)
DBManager.Commit()

DBManager.Disconnect()

for DayPart in ["Morning", "Noon", "Study"]:
    DBManager.InsertDistances(DayPart, direct=True)


GeoFailsFile.close()
