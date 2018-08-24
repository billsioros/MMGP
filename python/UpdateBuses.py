import pyodbc
from util import GetCredentials
import sys
from itertools import izip
from DBmanagement import DBManager as DBM
import os

cwd = os.getcwd()

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

# Select All Buses

sql = "Select BusCode, BusNumber, BusStudentSites     \
       From dbo.Bus"

cursor.execute(sql)
Buses = cursor.fetchall()

con.close()

# Create a new Database
DBManager = DBM(cwd + "/resources/data/MMGP_data.db", GoogleAPI_key, OpenAPI_key)


DBManager.InsertBus(Buses)
DBManager.Commit()

DBManager.Disconnect()
