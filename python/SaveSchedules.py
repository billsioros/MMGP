from DBmanagement import DBManager as DBM
import sys
import json
from util import GetCredentials
import os


jsonRequest = sys.argv[1]

keep = False
if len(sys.argv) > 2:
    if sys.argv[2] == "-k":
        keep = True
    else:
        raise ValueError("Wrong Argument:  '" + sys.argv[2] + "'")


data = json.loads(jsonRequest.decode("greek"))

Database = data["Database"]
Settings = data["Settings"]

ActiveCon, GoogleAPIKey, OpenAPIKey, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)

ExistingSchedules = data["Existing"]
NewSchedules = data["New"]
DeletedSchedules = data["Deleted"]

DBManager = DBM(Database, GoogleAPIKey=GoogleAPIKey, OpenAPIKey=OpenAPIKey)

for schedule in ExistingSchedules:
    ID = DBManager.InsertSchedule(schedule)
    schedule["TemporalID"] = "Existing"

for schedule in NewSchedules:
    # Here New schedule will always have "New#" on scheduleID
    ID = DBManager.InsertSchedule(schedule, new=True)
    schedule["ScheduleID"] = ID
    schedule["TemporalID"] = "New"

for schedule in DeletedSchedules:
    sql = "Delete From Schedule Where ScheduleID = \"" + schedule["ScheduleID"] + "\""
    DBManager.Execute(sql)
    schedule["TemporalID"] = "Deleted"

DBManager.Commit()

if keep:

    jsonFile = data["JSONFile"]

    if os.path.isfile(jsonFile):
        fp = open(jsonFile)
        ScheduleChanges = json.load(fp)
        fp.close()
    else:
        ScheduleChanges = {}
        ScheduleChanges["ScheduleChanges"] = []

    ScheduleChanges["ScheduleChanges"] += NewSchedules
    ScheduleChanges["ScheduleChanges"] += ExistingSchedules
    ScheduleChanges["ScheduleChanges"] += DeletedSchedules

    fp = open(jsonFile, "w+")
    json.dump(ScheduleChanges, fp)
    fp.close()
