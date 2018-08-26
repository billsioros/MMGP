from DBmanagement import DBManager as DBM
import sys
import json
import os

JSONFile = sys.argv[1]

with open(JSONFile, 'r') as jsonfile:
    data = json.load(jsonfile)

Database = data["Database"]
sql = data["sql"]

DBManager = DBM(Database)

Rows = DBManager.Execute(sql)

Rows = {"Rows": Rows}

with open(JSONFile, 'w+') as jsonfile:
    json.dump(Rows, jsonfile, sort_keys=True, indent=4)