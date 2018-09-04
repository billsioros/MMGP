import pyodbc
from util import GetCredentials, RowListToDict
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

# Select All Buses

sql = "Select BusCode, BusNumber, BusStudentSites     \
       From dbo.Bus"

cursor.execute(sql)
Buses = cursor.fetchall()

BusColumns = ["Code", "Number", "Capacity"]

Buses = RowListToDict(Buses, BusColumns)

con.close()

# Create a new Database
DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)


DBManager.InsertBus(Buses)
DBManager.Commit()

DBManager.Disconnect()
