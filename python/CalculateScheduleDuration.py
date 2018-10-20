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

def penalty(departureTime, depot, students):
    def duration(A, B):
        return -1

    def serviceTime(A):
        return 0 if A == depot else 20

    def timewindow(A):
        return (0, 86340)
    
    def partialCost(A, B):
        return serviceTime(A) + duration(A, B)

    def partialPenalty(arrivalTime, A, B):
        arrivalTime += partialCost(A, B)

        startOfService = max(arrivalTime, timewindow(B)[0])

        return max(0, startOfService + serviceTime(B) - timewindow(B)[1])

    arrivalTime = departureTime
    totalPenalty = partialPenalty(arrivalTime, depot, students[0])

    for j in xrange(0, len(students) - 1):
        totalPenalty += partialPenalty(arrivalTime, students[j], students[j + 1])

    totalPenalty += partialPenalty(arrivalTime, students[-1], depot)

    return totalPenalty
    