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

GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)


constr =    "Driver=" + ServerType + ";" + \
            "Server=" + ServerName + ";" + \
            "Database=" + DatabaseName + ";"

if Username and Password:
      constr +=   ("UID=" + Username + ";" + \
                  "PWD=" + Password + ";")


# con = pyodbc.connect(constr, autocommit=True, timeout=120)

con = pyodbc.connect(   DRIVER=ServerType,
                        SERVER=ServerName,
                        Database=DatabaseName,
                        Trusted_Connection = 'yes',
                        autocommit=True)


# Select All Morning Students

cursor = con.cursor()

# Select All Buses

sql = "Select BusCode, BusNumber, BusStudentSites     \
       From dbo.Bus"

cursor.execute(sql)
Buses = cursor.fetchall()

con.close()

# Create a new Database
DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)


DBManager.InsertBus(Buses)
DBManager.Commit()

DBManager.Disconnect()
