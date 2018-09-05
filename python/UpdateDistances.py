from util import GetCredentials
import sys
from DBmanagement import DBManager as DBM
import os
import json

jsonRequest = sys.argv[1]

data = json.loads(jsonRequest)

Settings = data["Settings"]
Database = data["Database"]
DayPart = data["DayPart"]
direct = data["direct"]

if data.has_key("fileName"):
    distancesFileName = data["fileName"]
else:
    distancesFileName = None



ActiveCon, GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)

# Get the Database
DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

DBManager.InsertDistances(DayPart, direct=direct, fileName=distancesFileName)

DBManager.Commit()

DBManager.Disconnect()
