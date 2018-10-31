import sqlite3
import os
from time import sleep
from hashlib import sha1 as sha1
import util
import csv

cwd = os.getcwd()


class DBManager:

    """ Connects to an sqlite3 database if it exists in current directory, or creates a new one
        Connection = current connection """
    def __init__(self, fileName, new=False, GoogleAPIKey=None, OpenAPIKey=None):
        self.Connection = None
        self.Cursor = None

        if new:
            if os.path.isfile(fileName):
                os.remove(fileName)
            
            print "Creating new Database..."
            self.CreateDatabase(fileName)
        else:
            if not os.path.isfile(fileName):
               raise IOError(fileName + " does not exist!")
                
        self.FileName = fileName
        self._GoogleAPIKey = GoogleAPIKey
        self._OpenAPIKey = OpenAPIKey
        self._MapsHandler = None
        
        self._Parser = None

        self.Connect(fileName)
        

    def CreateDatabase(self, fileName):  

        self.Connect(fileName)

        sql = "CREATE TABLE Student (                   \
                    StudentID       varchar(255),       \
                    LastName        varchar(255),       \
                    FirstName       varchar(255),       \
                    AddressID       varchar(255),       \
                    Level           varchar(255),       \
                    Class           varchar(255),       \
                    Phone           varchar(255),       \
                    Mobile          varchar(255),       \
                    OtherPhone1     varchar(255),       \
                    OtherPhone2     varchar(255),       \
                    Primary Key (StudentID),            \
                    Foreign Key (AddressID) References Address(AddressID)  )"
        self.Cursor.execute(sql)

        sql = "Create Table Address (                       \
                    AddressID           varchar(255),       \
                    Road                varchar(255),       \
                    Number              varchar(255),       \
                    ZipCode             varchar(255),       \
                    Prefecture          varchar(255),       \
                    Municipal           varchar(255),       \
                    Area                varchar(255),       \
                    GPS_X               decimal(20, 14),    \
                    GPS_Y               decimal(20, 14),    \
                    FullAddress         varchar(255),       \
                    TranslatedAddress   varchar(255),       \
                    FormattedAddress    varchar(255),       \
                    Primary Key (AddressID)         )"
        self.Cursor.execute(sql)

        sql = "Create Table Schedule (                  \
                    ScheduleID  varchar(255),           \
                    StudentID   varchar(255),           \
                    AddressID   varchar(255),           \
                    Monday          bit,                \
                    Tuesday         bit,                \
                    Wednesday       bit,                \
                    Thursday        bit,                \
                    Friday          bit,                \
                    DayPart         varchar(255),       \
                    FullNote        varchar(255),       \
                    Early           varchar(255),       \
                    Late            varchar(255),       \
                    Around          varchar(255),       \
                    Comment         text,               \
                    BusSchedule     varchar(255),       \
                    ScheduleOrder   int,                \
                    ScheduleTime    varchar(255),       \
                    Primary Key (ScheduleID),           \
                    Foreign Key (AddressID) References Address(AddressID),  \
                    Foreign Key (StudentID) References Student(StudentID)   )"
        self.Cursor.execute(sql)

        sql = "Create Table MorningDistance (               \
                    AddressID_1 varchar(255),               \
                    AddressID_2 varchar(255),               \
                    Duration    int,                        \
                    Distance    int,                        \
                    Primary Key (AddressID_1, AddressID_2),                  \
                    Foreign Key (AddressID_1) References Address(AddressID), \
                    Foreign Key (AddressID_2) References Address(AddressID)  )"
        self.Cursor.execute(sql)

        sql = "Create Table NoonDistance (                  \
                    AddressID_1 varchar(255),               \
                    AddressID_2 varchar(255),               \
                    Duration    int,                        \
                    Distance    int,                        \
                    Primary Key (AddressID_1, AddressID_2),                  \
                    Foreign Key (AddressID_1) References Address(AddressID), \
                    Foreign Key (AddressID_2) References Address(AddressID)  )"
        self.Cursor.execute(sql)

        sql = "Create Table StudyDistance (                 \
                    AddressID_1 varchar(255),               \
                    AddressID_2 varchar(255),               \
                    Duration    int,                        \
                    Distance    int,                        \
                    Primary Key (AddressID_1, AddressID_2),                  \
                    Foreign Key (AddressID_1) References Address(AddressID), \
                    Foreign Key (AddressID_2) References Address(AddressID)  )"            
        self.Cursor.execute(sql)

        sql = "Create Table Bus (               \
                    BusID       varchar(255),   \
                    Number      int,            \
                    Capacity    int,            \
                    Primary Key (BusID)         )"
        self.Cursor.execute(sql)   

        sql = "Create Table Depot (                     \
                AddressID           varchar(255),       \
                Road                varchar(255),       \
                Number              varchar(255),       \
                ZipCode             int,                \
                Prefecture          varchar(255),       \
                Municipal           varchar(255),       \
                Area                varchar(255),       \
                GPS_X               decimal(20, 14),    \
                GPS_Y               decimal(20, 14),    \
                FullAddress         varchar(255),       \
                TranslatedAddress   varchar(255),       \
                FormattedAddress    varchar(255),       \
                Primary Key (AddressID)         )"
        self.Cursor.execute(sql)

        self.Commit()
        self.Disconnect()

        return


    def DestroyDatabase(self):
        sql = "Drop Table Student"
        self.Cursor.execute(sql)

        sql = "Drop Table Address"
        self.Cursor.execute(sql)

        sql = "Drop Table Distance"
        self.Cursor.execute(sql)

        self.Connection.commit()
        self.Connection.close()

        os.remove(self.FileName)
        return


    def Connect(self, fileName):
        if not self.Connection:
            self.Connection = sqlite3.connect(fileName)
            self.Cursor = self.Connection.cursor()

            self.__InitRowFactory()


    def Disconnect(self):
        self.Connection.close()
        self.Connection = None
        self.Cursor = None
        self.MapsHandler = None
        self.FileName = None
        self.GoogleAPIKey = None
        self.OpenAPIKey = None


    """ Note To Self:
        Be sure to make Tables a double iterable of type (RowList, DayPart)"""
    def InsertStudent(self, Tables, overwrite=False, GeoFailsFile=None):
        requests = 0
        # Pull Addresses from Database
        Addresses = self.GetAddresses()
        ExistingStudents = self.GetStudents()

        # Delete the whole students table but not the addresses table                                       - [Update]
        # Check all the new students for previously found addresses                                         - [New, Update]
        # Insert those students with previously found address ID                                            - [New, Update]
        # Insert all students with new addresses (whether they have GPS coords or not)                      - [New, Update]
        # Delete any address that is not connected to a student after the new entries finish being inserted - [Update]

        self.Cursor.execute("Delete From Schedule")
        if overwrite:
            self.Cursor.execute("Delete From Address")
            self.Cursor.execute("Delete From Student")

        InsertedSchedules = dict()

        # Tables is list of lists of Rows of Data
        for RowList, DayPart in Tables:
            NoGPS = list()       

            # Insert All Records that already have GPS coordinates
            for Row in RowList:

                # Concatenate the Address to a single string and hash it
            
                FullAddress, TranslatedAddress = util.ConcatenateAddress(Row["Road"], Row["Num"], Row["ZipCode"], Row["Muni"], Row["Area"], Row["Prefec"], "GREECE")
                
                HashAddress = self.__Hash(TranslatedAddress)
                
                # If address has not been added to the database add it
                if Row["GPSX"] and Row["GPSY"]:
                    if not Addresses.has_key(HashAddress):
                        # Decimals must be turned to strings
                        GPSX = str(Row["GPSX"])
                        GPSY = str(Row["GPSY"])
                        
                        Addresses[HashAddress] = (GPSX, GPSY)
                        AddressList = [HashAddress, Row["Road"], Row["Num"], Row["ZipCode"], Row["Prefec"], Row["Muni"], Row["Area"], GPSX, GPSY, FullAddress, TranslatedAddress, None]

                        self.Cursor.execute("Insert Into Address    \
                                            Values (?,?,?,?,?,?,?,?,?,?,?,?)", AddressList)
                # If there is no GPS coordinates add address to GeoCoding list and geocode it after 
                # all other addresses have been inserted. This way if an address is already in the database
                # we do not have to geocode it. (Trying to reduce geocoding requests)
                else:
                    NoGPS.append((Row["ID"], Row["LastName"], Row["FirstName"], HashAddress, \
                    Row["Road"], Row["Num"], Row["ZipCode"], Row["Prefec"], Row["Muni"], Row["Area"], TranslatedAddress, FullAddress))

                # Add student to the database
                # Format Some Values

                if not ExistingStudents.has_key(Row["ID"]):
                    if Row["Class"]:
                        Row["Class"] = Row["Class"].replace('-', "")
                        Row["Class"].strip()
                        if Row["Class"] == "":
                            Row["Class"] = None

                    if Row["Phone"] != None:
                        Row["Phone"].strip(" ")
                        if Row["Phone"] == "":
                            Row["Phone"] = None

                    if Row["Mobile"] != None:
                        Row["Mobile"].strip(" ")
                        if Row["Mobile"] == "":
                            Row["Mobile"] = None

                    if Row["OtherPhone1"] != None:
                        Row["OtherPhone1"].strip(" ")
                        if Row["OtherPhone1"] == "":
                            Row["OtherPhone1"] = None

                    if Row["OtherPhone2"] != None:
                        Row["OtherPhone2"].strip(" ")
                        if Row["OtherPhone2"] == "":
                            Row["OtherPhone2"] = None

                    StudentList =  [Row["ID"], Row["LastName"], Row["FirstName"], HashAddress, Row["Level"], Row["Class"], 
                                    Row["Phone"], Row["Mobile"], Row["OtherPhone1"], Row["OtherPhone2"]]
                    
                    self.Cursor.execute("Insert Into Student    \
                                    Values (?,?,?,?,?,?,?,?,?,?)", StudentList)

                    ExistingStudents[Row["ID"]] = 1



                if not InsertedSchedules.has_key(Row["ScheduleID"]):
                    if Row["Notes"]:
                        self.__InitParser()
                        NotesDict = self._Parser.Parse(Row["Notes"])
                        Early = NotesDict["Early Pickup"]
                        Late = NotesDict["Late Pickup"]
                        Comment = NotesDict["Comments"]
                        Around = NotesDict["Around"]
                    else:
                        Early = "00.00"
                        Late = "23.59"
                        Comment = None
                        Around = None

                    if Row["ScheduleTime"]:
                        if "." in Row["ScheduleTime"]:
                            Row["ScheduleTime"] = Row["ScheduleTime"].replace(":", ".")
                        if "," in Row["ScheduleTime"]:
                            Row["ScheduleTime"] = Row["ScheduleTime"].replace(",", ".")

                        index = Row["ScheduleTime"].find(":")
                        if len(Row["ScheduleTime"][:index]) == 1:
                            Row["ScheduleTime"] = "0" + Row["ScheduleTime"]

                    else:
                        Row["ScheduleTime"] = None

                    ScheduleList = [Row["ScheduleID"], Row["ID"], HashAddress, Row["Mon"], Row["Tue"], Row["Wen"], Row["Thu"], Row["Fri"], DayPart,
                                    Row["Notes"], Early, Late, Around, Comment, 
                                    Row["BusSchedule"], Row["ScheduleOrder"], Row["ScheduleTime"]]

                    self.Cursor.execute("Insert Into Schedule   \
                                    Values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", ScheduleList)
                    
                    InsertedSchedules[Row["ScheduleID"]] = 1


            # Insert All Records that do not have GPS coordinates
            i = 0 # Geocoding per sec
          
            for ID, LastName, FirstName, HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, TranslatedAddress, FullAddress in NoGPS:

                if not TranslatedAddress:
                    continue

                # If address has not been added to the database, geocode it and add it
                if not Addresses.has_key(HashAddress):

                    self.__InitMapsHandler()
                    # Only 10 requests per sec
                    
                    if i == 10:
                        sleep(1) # Sleep 1 seconds for safety
                        i = 0
                    requests += 1
                    FormattedAddress, GPSX, GPSY = self._MapsHandler.Geocode(TranslatedAddress)
                    i += 1

                    valid = False
                    if FormattedAddress:
                        Addresses[HashAddress] = (GPSX, GPSY)
                    
                        # Find Error and Log it into a csv of your choosing
                        valid = self.__LogGeocodingError(ID, LastName, FirstName, FormattedAddress, TranslatedAddress, DayPart, GeoFailsFile)
                    else:
                        valid = self.__LogGeocodingError(ID, LastName, FirstName, "None", TranslatedAddress, DayPart, GeoFailsFile)
                        
                    if valid:
                        AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, \
                        FullAddress, TranslatedAddress, FormattedAddress]

                        self.Cursor.execute("Insert Into Address    \
                                            Values (?,?,?,?,?,?,?,?,?,?,?,?)", AddressList)

        self.__DiscardAddresses()


    def InsertBus(self, RowList):
        Buses = self.GetBuses()

        for Bus in RowList:
            ToAdd = [Bus["Code"], int(Bus["Number"]), Bus["Capacity"]]      

            if not Buses.has_key(Bus["Code"]):
                self.Cursor.execute("Insert Into Bus    \
                                        Values (?,?,?)", ToAdd)
            else:
                self.Cursor.execute("Update Bus     \
                                        Set Number = ?, Capacity = ?    \
                                        Where BusID = ?", ToAdd)


    def InsertDepot(self, RowList):

        for Road, Number, ZipCode, Prefecture, Municipal, Area in RowList:

            FullAddress, TranslatedAddress = util.ConcatenateAddress(Road, Number, ZipCode, Municipal, Area, Prefecture, "GREECE")
            HashAddress = self.__Hash(TranslatedAddress)
            self.__InitMapsHandler()
            FormattedAddress, GPSX, GPSY = self._MapsHandler.Geocode(TranslatedAddress)

            AddressList = [HashAddress, Road, Number, ZipCode, Prefecture, Municipal, Area, GPSX, GPSY,\
            FullAddress, TranslatedAddress, FormattedAddress]

            self.Cursor.execute("Insert Into Depot    \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?)", AddressList)


    def InsertSchedule(self, Schedule, SchedID=None, new=False):

        OldSchedules = self.GetSchedules()

        if new:
            if SchedID and not OldSchedules.has_key(SchedID):
                ScheduleID = SchedID
            else:
                import binascii

                digits = [4,2,2,2,6]

                found = True
                ScheduleID = ""

                while found:
                    ScheduleID = ""
                    for i in digits:
                        ScheduleID += binascii.b2a_hex(os.urandom(i)).upper()
                        ScheduleID += "-"
                    ScheduleID = ScheduleID[:-1]

                    if not OldSchedules.has_key(ScheduleID):
                        found = False
        else:
            ScheduleID = Schedule["ScheduleID"]

        OldAddresses = self.GetAddresses()

        if Schedule.has_key("Address"):
            FullAddress, TranslatedAddress = util.ConcatenateAddress(Schedule["Address"]["Road"], Schedule["Address"]["Number"],
            Schedule["Address"]["ZipCode"], Schedule["Address"]["Municipal"], None, None, "GREECE")
                
            AddressID = self.__Hash(TranslatedAddress)

            if not OldAddresses.has_key(AddressID):
                self.__InitMapsHandler()

                FormattedAddress, GPSX, GPSY = self._MapsHandler.Geocode(TranslatedAddress)

                AddressList = [AddressID, Schedule["Address"]["Road"], Schedule["Address"]["Number"], Schedule["Address"]["ZipCode"], 
                None, Schedule["Address"]["Municipal"], None, GPSX, GPSY,
                FullAddress, TranslatedAddress, FormattedAddress]

                self.Cursor.execute("Insert into Address    \
                                    Values (?,?,?,?,?,?,?,?,?,?,?,?)", AddressList)

        if not new:
            Keys = Schedule.keys()
            sql = "Update Schedule Set "
            for key in Keys:
                
                if key == "ScheduleID":
                    continue
                elif key == "Address":
                    if not OldAddresses.has_key(AddressID):
                        # that means address exists in Schedule, and addressID has been found and changed
                        sql += "AddressID = \"" + AddressID + "\", "
                elif key == "Days":
                    for day in Schedule[key].keys():
                        sql += day + " = " + str(Schedule[key][day]) + ", "
                elif key == "TemporalID":
                    continue
                else:
                    key = key.replace("\"", "")
                    key = key.replace("'", "")

                    sql += key + " = " + "\"" + Schedule[key] + "\", "

            sql = sql[:-2] 
            sql += " Where ScheduleID = \"" + ScheduleID + "\""
            print sql
            self.Cursor.execute(sql)

        else:
            allKeys = OldSchedules[ OldSchedules.keys()[0] ].keys()
            tmpSched = dict()
            
            for key in allKeys:
                if not Schedule.has_key(key):
                    tmpSched[key] = None
                else:
                    tmpSched[key] = Schedule[key]
                    
            Values = [ScheduleID, tmpSched["StudentID"], AddressID, 
                    tmpSched["Monday"], tmpSched["Tuesday"], tmpSched["Wednesday"], tmpSched["Thursday"], tmpSched["Friday"],
                    tmpSched["DayPart"], tmpSched["FullNote"], tmpSched["Early"], tmpSched["Late"], tmpSched["Around"], 
                    tmpSched["Comment"], tmpSched["BusSchedule"],
                    tmpSched["ScheduleOrder"], tmpSched["ScheduleTime"]]
            self.Cursor.execute("Insert into Schedule   \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", Values)

        return ScheduleID
            
        

    def InsertDistances(self, DayPart, direct=False, fileName=None):


        if DayPart != "Morning" and DayPart != "Noon" and DayPart != "Study":
            raise ValueError("Error: Non valid DayPart.")

        Table = DayPart + "Distance"
      
        self.Cursor.execute(\
                " Select Address.AddressID, Address.GPS_X, Address.GPS_Y    \
                From Address                                                \
                Where exists (  Select *                                    \
                                From Student, Schedule                      \
                                Where Schedule.AddressID = Address.AddressID and Student.StudentID = Schedule.StudentID \
                                and Schedule.DayPart = ?)", [DayPart])

        Addresses = self.Cursor.fetchall()

        Depot = self.GetDepot()

        Addresses.append(Depot)

        Origins = list()
        for Address in Addresses:
            Origins.append((Address["AddressID"], (Address["GPS_Y"], Address["GPS_X"])))

        self.__InitMapsHandler()
        Matrix = self._MapsHandler.DistanceMatrix(Origins)

        if not direct:
            if not fileName:
                "Error: No file was given, writing on \"tempDistances.tsv\""
                DatabaseDir = os.path.realpath(os.path.dirname(self.FileName))

                fileName = DatabaseDir + "/tempDistances.tsv"

            logcsv = open(fileName, "r")

            if logcsv.readline():
                logcsv.close()
                logcsv = open(fileName, "a+")
            else:
                logcsv.close()
                logcsv = open(fileName, "w+")
                logcsv.write("DayPart\tID1\tID2\tDuration\tDistance\n")
                
            for id1, id2, duration, distance in Matrix:
                logcsv.write(DayPart + "\t" + str(id1) + "\t" + str(id2) + "\t" + str(duration) + "\t" + str(distance) + "\n")
            logcsv.close()

        else:
            self.Cursor.execute("Delete From " + Table)
            for id1, id2, duration, distance in Matrix:
                # All previous distances will be overwritten              
                if not duration:
                    duration = 0
                if not distance:
                    distance = 0
                sql = "Insert into " + Table + " Values(\"" + str(id1) + "\", \"" + str(id2) + "\", "  + str(duration) + ", " + str(distance) + ")"
                self.Cursor.execute(sql)


    def InsertDistancesFromFile(self, DayPart, fileName):

        if DayPart != "Morning" and DayPart != "Noon" and DayPart != "Morning":
            raise ValueError("Error: Non valid DayPart.")

        if not os.path.isfile(fileName):
            print "Error: File does not exist. Returning..."
            return

        Table = DayPart + "Distance"


        with open(fileName) as distances:
            readCSV = csv.DictReader(distances, delimiter='\t')
            Rows = list()
            for row in readCSV:           
                ID1 = row["ID1"]
                ID2 = row["ID2"]
                Duration = row["Duration"]
                if row["Distance"]:
                    Distance = row["Distance"]
                else:
                    Distance = None
                Rows.append([ID1, ID2, Duration, Distance])

        
        for id1, id2, duration, distance in Rows:
            # All previous distances will be overwritten
            self.Cursor.execute("Delete From ?", [Table])
            sql = "Insert into " + Table + " Values('" + id1 + "', '" + id2 + "', "  + duration + ", " + distance + ")"
            self.Cursor.execute(sql)


    def Commit(self):
        if self.Connection:
            self.Connection.commit()


    def RollBack(self):
        if self.Connection:
            self.Connection.rollback()


    def GetAddresses(self):
        self.Connect(self.FileName)
        sql = " Select AddressID, GPS_X, GPS_Y, FullAddress \
                From Address"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        Addresses = dict()
        for Row in Rows:
            Addresses[Row["AddressID"]] = Row
        
        return Addresses


    def GetBuses(self):
        self.Connect(self.FileName)
        sql = " Select * From Bus"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        Buses = dict()
        for Row in Rows:
            Buses[Row["BusID"]] = Row

        return Buses


    def GetStudents(self):
        self.Connect(self.FileName)
        sql = "Select * From Student"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        Students = dict()
        for Row in Rows:
            Students[Row["StudentID"]] = Row

        return Students


    def GetSchedules(self):
        self.Connect(self.FileName)
        sql = "Select * From Schedule"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        Schedules = dict()
        for Row in Rows:
            Schedules[Row["ScheduleID"]] = Row

        return Schedules

    # Fix this!!
    def GetDistances(self, DayPart):
        self.Connect(self.FileName)

        if DayPart != "Morning" and DayPart != "Noon" and DayPart != "Study":
            raise ValueError("Error: Non valid DayPart.")

        sql = "Select * From " + DayPart + "Distance"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()
        
        Distances = {}
        for ID1, ID2, Duration, Distance in Rows:
            Distances[ID1] = {}
        
        for ID1, ID2, Duration, Distance in Rows:
            Distances[ID1][ID2] = (Duration, Distance)

        return Distances


    def GetDepot(self, AddressID=None):

        self.Connect(self.FileName)
        if AddressID:
            sql = "Select AddressID, GPS_X, GPS_Y, FullAddress From Depot Where AddressID = \"" + AddressID +"\""
            self.Cursor.execute(sql)
        else:
            self.Cursor.execute("Select AddressID, GPS_X, GPS_Y, FullAddress From Depot")

        Depot = self.Cursor.fetchone()
        return Depot


    def CalculateDistance(self, AddressID_1, AddressID_2, DayPart):
        self.Connect(self.FileName)

        sql = "Select Duration, Distance From " + DayPart + "Distance Where AddressID_1 = \"" + AddressID_1 + "\" and AddressID_2 = \"" + AddressID_2 + "\""

        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        if Rows:
            return (Rows[0]["Distance"], Rows[0]["Duration"])
        else:
            return []


    def Execute(self, sql):

        self.Cursor.execute(sql)

        Rows = self.Cursor.fetchall()

        return Rows


    def __InitRowFactory(self):
        def dict_factory(cursor, row):
            d = {}
            for idx, col in enumerate(cursor.description):
                d[col[0]] = row[idx]
            return d

        self.Connection.row_factory = dict_factory
        self.Cursor = self.Connection.cursor()


    def __ResetRowFactory(self):
        self.Disconnect()
        self.Connect(self.FileName)


    def __InitParser(self):
        if not self._Parser:
            self._Parser = util.Parser()
    

    def __InitMapsHandler(self):
        if not self._MapsHandler:
            self._MapsHandler = util.MapsHandler(GoogleAPIKey=self._GoogleAPIKey, OpenAPIKey=self._OpenAPIKey)

    
    def __DiscardAddresses(self):
        self.Connect(self.FileName)
        self.Cursor.execute("Select Address.AddressID   \
                            From Address                \
                            Where Not Exists            \
                                (Select * From Student  \
                                Where Student.AddressID = Address.AddressID)")
        
        Addresses = self.Cursor.fetchall()

        for Address in Addresses:
            self.Cursor.execute("Delete From Address Where AddressID = ?", [Address["AddressID"]])

        self.Cursor.execute("Select Address.AddressID   \
                            From Address                \
                            Where Not Exists            \
                                (Select * From Schedule  \
                                Where Schedule.AddressID = Address.AddressID)")
        
        Addresses = self.Cursor.fetchall()

        for Address in Addresses:
            self.Cursor.execute("Delete From Address Where AddressID = ?", [Address["AddressID"]])


    def __Hash(self, Address):
        return sha1(Address).hexdigest()


    def __LogGeocodingError(self, ID, LastName, FirstName, FormattedAddress, TranslatedAddress, DayPart, GeoFailsFile):
        valid = True 
        if util.CountNumbers(FormattedAddress) <= 5:
            if "&" not in FormattedAddress and " KAI " not in FormattedAddress:
                valid = False
                if GeoFailsFile:
                    LN = util.TranslateAddress(LastName)
                    FN = util.TranslateAddress(FirstName)
                    GeoFailsFile.write(str(ID) + "\t" + LN + "\t" + FN + "\t" + FormattedAddress + "\t"\
                    + TranslatedAddress + "\t" + DayPart + "\n")
        return valid