from DBmanagement import DBManager as DBM
import sys
import json
import os

cwd = os.getcwd()

fileName = sys.argv[1]
Database = sys.argv[2]

DBManager = DBM(Database)

with open(fileName, "r") as json_file:
    data = json.load(json_file)

students = data["students"]

Depot = DBManager.GetDepot()
Depot = Depot[0]

FullDistance = 0
FullDuration = 0

for i in range(len(students) - 1):
    student1 = students[i]
    student2 = students[i + 1]

    Distance, Duration = DBManager.CalculateDistance(student1["addressId"], student2["addressId"], student1["dayPart"])
    FullDistance += Distance
    FullDuration += Duration
    FullDuration += 20

if students[0]["dayPart"] == "Morning":
    Distance, Duration = DBManager.CalculateDistance(students[-1]["addressId"], Depot, students[-1]["dayPart"])
else:
    Distance, Duration = DBManager.CalculateDistance(Depot, students[0]["addressId"], students[0]["dayPart"])

FullDistance += Distance
FullDuration += Duration


print FullDistance
print FullDuration
