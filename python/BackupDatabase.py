from DBmanagement import DBManager as DBM
import sys
import json
from time import localtime, strftime
import os
from shutil import copyfile

datetime = strftime("%d-%m-%Y_%H-%M-%S", localtime())

fileName = sys.argv[1]

with open(fileName, "r") as json_file:
    data = json.load(json_file)

Settings = data["Settings"]
Database = data["Database"]


DatabaseDir = os.path.realpath(os.path.dirname(Database))
Backup = DatabaseDir + "/Backup_" + datetime + ".db"

copyfile(Database, Backup)