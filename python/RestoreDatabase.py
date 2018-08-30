from DBmanagement import DBManager as DBM
import sys
import json
from time import localtime, strftime
import os
from shutil import copyfile

fileName = sys.argv[1]

with open(fileName, "r") as json_file:
    data = json.load(json_file)

Settings = data["Settings"]
Database = data["Database"]
Backup = data["Backup"]


if os.path.isfile(Database):
    os.remove(Database)

copyfile(Backup, Database)