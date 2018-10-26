from DBmanagement import DBManager as DBM
import sys
import json
import os

jsonRequest = sys.argv[1]

data = json.loads(jsonRequest)

Database = data["Database"]
StudentID = data["Student"]

ExistingSchedules = data["Existing"]
NewSchedules = data["New"]

DBManager = DBM(Database)

for schedule in ExistingSchedules:
    print schedule