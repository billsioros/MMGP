from util import GetCredentials
import sys
from DBmanagement import DBManager as DBM
import os
import json

fileName = sys.argv[1]


with open(fileName, "r") as json_file:
    data = json.load(json_file)

Credentials = data["Credentials"]
Database = data["Database"]
rowIndex = data["rowIndex"]
DayPart = data["DayPart"]
direct = data["direct"]

if data.has_key("fileName"):
    distancesFileName = data["fileName"]
else:
    distancesFileName = None



GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(Credentials, rowIndex)

# Get the Database
DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

DBManager.InsertDistances(DayPart, direct=direct, fileName=distancesFileName)

DBManager.Commit()

DBManager.Disconnect()
