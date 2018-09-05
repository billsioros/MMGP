from DBmanagement import DBManager as DBM
import sys
import json
import os

cwd = os.getcwd()

jsonRequest = sys.argv[1]

data = json.loads(jsonRequest)

students = data["students"]
Database = data["Database"]

DBManager = DBM(Database)

Depot = DBManager.GetDepot()

FullDistance = 0
FullDuration = 0

for i in range(len(students) - 1):
    student1 = students[i]
    student2 = students[i + 1]

    results = DBManager.CalculateDistance(student1["addressId"], student2["addressId"], student1["dayPart"])


    if results:
        Distance = results[0]
        Duration = results[1]
    else:
        Distance = 0
        Duration = 0
        
    FullDistance += Distance
    FullDuration += Duration
    FullDuration += 20

if students[0]["dayPart"] == "Morning":
    results = DBManager.CalculateDistance(students[-1]["addressId"], Depot["AddressID"], students[-1]["dayPart"])
else:
    results = DBManager.CalculateDistance(Depot["AddressID"], students[0]["addressId"], students[0]["dayPart"])

if results:
    Distance = results[0]
    Duration = results[1]
else:
    Distance = 0
    Duration = 0

FullDistance += Distance
FullDuration += Duration

Result = {
    "Distance" : FullDistance, 
    "Duration" : FullDuration
}

print json.dumps(Result, sort_keys=True, indent=4)
