from DBManagement import DBManager as DBM
import sys
import csv
from util import GetCredentials


fileName = sys.argv[1]
rowIndex = sys.argv[2]
dayPart = sys.argv[3]

API_key, ServerType, ServerName, DatabaseName = GetCredentials(fileName, rowIndex)

if dayPart != "Morning" and dayPart != "Noon" and dayPart != "Study":
    print "Not Valid DayPart"
    exit()


DBManager = DBM("MMGP_data.db", APIKey=API_key)
#DBManager.InsertDistancesToFile(dayPart)
DBManager.InsertDistancesFromFile(dayPart)
DBManager.Commit()


