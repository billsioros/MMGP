import pyodbc
import sqlite3
from DBManagement import DBManager as DBM
from harvesine import harvesine
import os

con = pyodbc.connect(DRIVER='{SQL Server Native Client 11.0}', 
                 SERVER='DESKTOP-GIANNIS\SQLEXPRESS', 
                 DATABASE='maliaras',
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
DBManager = DBM("MMGP_Data.db", "AIzaSyBRGHJf69r2tYhvmpJxdayyXfZorTfHu5g")

DBManager.InsertBus(Buses)
DBManager.Commit()

DBManager.InsertStudent(OldYearMorning, "Morning")
DBManager.InsertStudent(NewYearMorning, "Morning")
DBManager.InsertStudent(OldYearNoon, "Noon")
DBManager.InsertStudent(NewYearNoon, "Noon")
DBManager.InsertStudent(OldYearStudy, "Study")
DBManager.InsertStudent(NewYearStudy, "Study")
DBManager.Commit()

DBManager.InsertDistances(harvesine)
DBManager.Commit()

# sql = "Select FullAddress From Address"
# DBManager.Cursor.execute(sql)

# Rows = DBManager.Cursor.fetchall()

# i = 1
# for row in Rows:
#     print i
#     print row[0]
#     i += 1
        
