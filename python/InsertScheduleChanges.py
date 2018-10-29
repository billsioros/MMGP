from DBmanagement import DBManager as DBM
import sys
import json
from util import GetCredentials
import os


jsonRequest = sys.argv[1]

data = json.loads(jsonRequest.decode("greek"))

Database = data["Database"]
Settings = data["Settings"]

ActiveCon, GoogleAPIKey, OpenAPIKey, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)


jsonFile = data["JSONFile"]
if os.path.isfile(jsonFile):
    fp = open(jsonFile)
    ScheduleChanges = json.load(fp)
    fp.close()
else:
    print "Nothing to insert"
    exit()

changes = ScheduleChanges["ScheduleChanges"]

DBManager = DBM(Database, GoogleAPIKey=GoogleAPIKey, OpenAPIKey=OpenAPIKey)

for schedule in changes:
    if schedule["TemporalID"] == "Existing":
        ID = DBManager.InsertSchedule(schedule)
    elif schedule["TemporalID"] == "New":
        ID = DBManager.InsertSchedule(schedule, schedule["ScheduleID"], new=True)
        schedule["ScheduleID"] = ID
    else:
        sql = "Delete From Schedule Where ScheduleID = \"" + schedule["ScheduleID"] + "\""
        DBManager.Execute(sql)
    

DBManager.Commit()

fp = open(jsonFile, "w+")
json.dump({ "ScheduleChanges": changes }, fp)
fp.close()