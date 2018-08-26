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

Credentials = data["Credentials"]
Database = data["Database"]
rowIndex = data["rowIndex"]

GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(fileName, rowIndex)

con = pyodbc.connect(DRIVER=ServerType,
                 SERVER=ServerName,
                 DATABASE=DatabaseName,
                 Trusted_Connection='yes', autocommit=True)

con.setdecoding(pyodbc.SQL_CHAR, encoding='greek')
con.setdecoding(pyodbc.SQL_WCHAR, encoding='greek')

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
