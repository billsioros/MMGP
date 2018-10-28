from DBmanagement import DBManager as DBM
import sys
import json
from util import GetCredentials

jsonRequest = sys.argv[1]

data = json.loads(jsonRequest.decode("greek"))

Database = data["Database"]
StudentID = data["Student"]
Settings = data["Settings"]

ActiveCon, GoogleAPIKey, OpenAPIKey, ServerType, ServerName, DatabaseName, Username, Password = GetCredentials(Settings)

ExistingSchedules = data["Existing"]
NewSchedules = data["New"]
DeletedSchedules = data["Deleted"]

DBManager = DBM(Database, GoogleAPIKey=GoogleAPIKey, OpenAPIKey=OpenAPIKey)

for schedule in ExistingSchedules:
    DBManager.InsertSchedule(schedule, StudentID)

for schedule in NewSchedules:
    DBManager.InsertSchedule(schedule, StudentID, new=True)

for schedule in DeletedSchedules:
    sql = "Delete From Schedule Where ScheduleID = \"" + schedule + "\""
    DBManager.Execute(sql)

DBManager.Commit()