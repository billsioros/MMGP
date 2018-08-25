from util import GetCredentials
import sys
from DBmanagement import DBManager as DBM
import os

cwd = os.getcwd()

credsFileName = sys.argv[1]
rowIndex = sys.argv[2]
DayPart = sys.argv[3]
Database = sys.argv[4]

direct = False

if "-d" in sys.argv:
    direct = True

distancesFileName = None

if not direct:
    if len(sys.argv) > 5:
        distancesFileName = sys.argv[5]
    else:
        distancesFileName = None

GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(credsFileName, rowIndex)

# Get the Database
DBManager = DBM(Database, GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

DBManager.InsertDistances(DayPart, direct=direct, fileName=distancesFileName)

DBManager.Commit()

DBManager.Disconnect()
