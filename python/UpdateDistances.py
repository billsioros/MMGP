from util import GetCredentials
import sys
from DBmanagement import DBManager as DBM

credsFileName = sys.argv[1]
rowIndex = sys.argv[2]
DayPart = sys.argv[3]

direct = False

if "-d" in sys.argv:
    direct = True

if not direct:
    if len(sys.argv) > 4:
        distancesFileName = sys.argv[4]
    else:
        distancesFileName = None

GoogleAPI_key, OpenAPI_key, ServerType, ServerName, DatabaseName = GetCredentials(credsFileName, rowIndex)

# Get the Database
DBManager = DBM("../data/MMGP_Data.db", GoogleAPI_key, OpenAPI_key)

DBManager.InsertDistances(DayPart, direct=direct, fileName=distancesFileName)

DBManager.Commit()

DBManager.Disconnect()
