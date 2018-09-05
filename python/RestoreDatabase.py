from DBmanagement import DBManager as DBM
import sys
import json
from time import localtime, strftime
import os
from shutil import copyfile

jsonRequest = sys.argv[1]

data = json.loads(jsonRequest)

Settings = data["Settings"]
Database = data["Database"]
Backup = data["Backup"]


if os.path.isfile(Database):
    os.remove(Database)

copyfile(Backup, Database)