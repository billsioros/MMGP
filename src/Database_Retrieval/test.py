import pyodbc
import sqlite3
from DBManagement import DBManager as DBM
from geopy import geocoders
import os

con = pyodbc.connect(DRIVER='{SQL Server Native Client 11.0}', 
                 SERVER='DESKTOP-GIANNIS\SQLEXPRESS', 
                 DATABASE='maliaras',
                 Trusted_Connection='yes', autocommit=True)

con.setdecoding(pyodbc.SQL_CHAR, encoding='greek')
con.setdecoding(pyodbc.SQL_WCHAR, encoding='greek')

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
DBManager = DBM("MMGP_Data.db", "AnqIobiOKJjxYO1bwmdC4z04lX2kTfFEgKcKyj_9y-bTzjE1d6hoa_A2XNi-dd0R")
DBManager.Insert(OldYearMorning, "Morning")
DBManager.Insert(NewYearMorning, "Morning")
DBManager.Insert(OldYearNoon, "Noon")
DBManager.Insert(NewYearNoon, "Noon")
DBManager.Insert(OldYearStudy, "Study")
DBManager.Insert(NewYearStudy, "Study")

DBManager.Commit()

sql = "Select FullAddress From Address"
DBManager.Cursor.execute(sql)

Rows = DBManager.Cursor.fetchall()

i = 1
for row in Rows:
    print i
    print row[0]
    i += 1
        
