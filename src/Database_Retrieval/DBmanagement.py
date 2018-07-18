import sqlite3
import os
from time import sleep
from hashlib import sha1 as sha1
import util
import csv


class DBManager:

    """ Connects to an sqlite3 database if it exists in current directory, or creates a new one
        Connection = current connection """
    def __init__(self, fileName, GoogleAPIKey=None, OpenAPIKey=None):
        self.Connection = None
        self.Cursor = None

        if not os.path.isfile(fileName):
            print "Creating new Database..."
            self.CreateDatabase(fileName)

        self.FileName = fileName
        self._GoogleAPIKey = GoogleAPIKey
        self._OpenAPIKey = OpenAPIKey
        self._MapsHandler = None
        
        self._Parser = None

        self.Connect(fileName)
        

    def CreateDatabase(self, fileName):  

        self.Connect(fileName)

        sql = "CREATE TABLE Student (               \
                    StudentID   varchar(255),       \
                    LastName    varchar(255),       \
                    FirstName   varchar(255),       \
                    AddressID   varchar(255),       \
                    Level       varchar(255),       \
                    Class       varchar(255),       \
                    Monday      bit,                \
                    Tuesday     bit,                \
                    Wednesday   bit,                \
                    Thursday    bit,                \
                    Friday      bit,                \
                    DayPart     varchar(255),       \
                    EarlyPickup varchar(255),       \
                    LatePickup  varchar(255),       \
                    EarlyDrop   varchar(255),       \
                    LateDrop    varchar(255),       \
                    Around      varchar(255),       \
                    AltAddress  varchar(255),       \
                    Comment     text,               \
                    Foreign Key (AddressID) References Address(AddressID)  )"
        self.Cursor.execute(sql)

        sql = "Create Table Address (               \
                    AddressID   varchar(255),       \
                    Road        varchar(255),       \
                    Number      varchar(255),       \
                    ZipCode     int,                \
                    Prefecture  varchar(255),       \
                    Municipal   varchar(255),       \
                    Area        varchar(255),       \
                    GPS_X       decimal(20, 14),    \
                    GPS_Y       decimal(20, 14),    \
                    FullAddress varchar(255),       \
                    FormattedAddress varchar(255),  \
                    Primary Key (AddressID)         )"
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

        sql = "Create Table Depot (             \
                AddressID   varchar(255),       \
                Road        varchar(255),       \
                Number      varchar(255),       \
                ZipCode     int,                \
                Prefecture  varchar(255),       \
                Municipal   varchar(255),       \
                Area        varchar(255),       \
                GPS_X       decimal(20, 14),    \
                GPS_Y       decimal(20, 14),    \
                FullAddress varchar(255),       \
                FormattedAddress varchar(255),  \
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


    def Disconnect(self):
        self.Connection.close()
        self.Connection = None
        self.Cursor = None
        self.MapsHandler = None
        self.FileName = None
        self.GoogleAPIKey = None
        self.OpenAPIKey = None

    """ RowList Components:
    StCode
    StLastName
    StFirstName
    SchAddress
    SchAddressNumber
    SchZipCode
    PrefectureDescription
    MunicipalDescription
    AreaDescription
    SchNotes
    LevelDescription
    ClassDescription
    SchMonday
    SchTuesday,
    SchWednesday
    SchThursday
    SchFriday
    SchGPS_X
    SchGPS_Y    
    """

    """ Note To Self:
        Be sure to make Tables a double iterable of type (RowList, DayPart)"""
    def InsertStudent(self, Tables, GeoFailsFile=None):
        # Pull Addresses from Database
        Addresses = self.GetAddresses()

        # Delete the whole students table but not the addresses table                                       - [Update]
        # Check all the new students for previously found addresses                                         - [New, Update]
        # Insert those students with previously found address ID                                            - [New, Update]
        # Insert all students with new addresses (whether they have GPS coords or not)                      - [New, Update]
        # Delete any address that is not connected to a student after the new entries finish being inserted - [Update]

        self.Cursor.execute("Delete From Student")

        # Tables is list of lists of Rows of Data
        for RowList, DayPart in Tables:
            NoGPS = list()

            # Insert All Records that already have GPS coordinates
            for ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area, Notes, Level, Class,\
            Mon, Tue, Wen, Thu, Fri, GPSX, GPSY in RowList:
                # If there are not coordinates add them to standby list
                if not GPSX or not GPSY:
                    NoGPS.append([ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area,\
                    Notes, Level, Class, Mon, Tue, Wen, Thu, Fri])

                    continue

                # Concatenate the Address to a single string and hash it
                FullAddress = util.ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "GREECE")
                
                HashAddress = self.__Hash(FullAddress)
                
                # If address has not been added to the database add it
                if not Addresses.has_key(HashAddress):
                    # Decimals must be turned to strings
                    GPSX = str(GPSX)
                    GPSY = str(GPSY)
                    
                    Addresses[HashAddress] = (GPSX, GPSY)
                    AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, FullAddress, None]

                    self.Cursor.execute("Insert Into Address    \
                                        Values (?,?,?,?,?,?,?,?,?,?,?)", AddressList)
                
                # Add student to the database
                if Notes:
                    self.__InitParser()
                    NotesDict = self._Parser.Parse(Notes)
                    EarlyPickup = NotesDict["Early Pickup"]
                    LatePickup = NotesDict["Late Pickup"]
                    EarlyDrop = NotesDict["Early Drop"]
                    LateDrop = NotesDict["Late Drop"]
                    AltAddress = NotesDict["Address"]
                    Comment = NotesDict["Comments"]
                    Around = NotesDict["Around"]
                else:
                    EarlyPickup = None
                    LatePickup = None
                    EarlyDrop = None
                    LateDrop = None
                    AltAddress = None
                    Comment = None
                    Around = None

                StudentList = [ID, LastName, FirstName, HashAddress, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart,\
                EarlyPickup, LatePickup, EarlyDrop, LateDrop, Around, AltAddress, Comment]
                
                self.Cursor.execute("Insert Into Student     \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)


            # Insert All Records that do not have GPS coordinates
            i = 0 # Geocoding per sec

            
            for ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area, Notes,\
            Level, Class, Mon, Tue, Wen, Thu, Fri in NoGPS:

                # Concatenate the Address to a single string and hash it
                FullAddress = util.ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "GREECE")
                if not FullAddress:
                    continue

                HashAddress = self.__Hash(FullAddress)

                # If address has not been added to the database, geocode it and add it
                if not Addresses.has_key(HashAddress):

                    self.__InitMapsHandler()
                    # Only 100 requests per sec
                    if i == 99:
                        sleep(4) # Sleep 4 seconds for safety
                        i = 0
                    FormattedAddress, GPSX, GPSY = self._MapsHandler.Geocode(FullAddress)
                    i += 1

                    valid = True
                    if FormattedAddress:
                        Addresses[HashAddress] = (GPSX, GPSY)
                    
                        # Find Error and Log it into a csv of your choosing
                        valid = self.__LogGeocodingError(ID, FormattedAddress, FullAddress, DayPart, GeoFailsFile)
                    
                    if valid:
                        AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY,\
                        FullAddress, FormattedAddress]

                        self.Cursor.execute("Insert Into Address    \
                                            Values (?,?,?,?,?,?,?,?,?,?,?)", AddressList)

                # Add student to the database
                if Notes:
                    self.__InitParser()
                    NotesDict = self._Parser.Parse(Notes)
                    EarlyPickup = NotesDict["Early Pickup"]
                    LatePickup = NotesDict["Late Pickup"]
                    EarlyDrop = NotesDict["Early Drop"]
                    LateDrop = NotesDict["Late Drop"]
                    AltAddress = NotesDict["Address"]
                    Comment = NotesDict["Comments"]
                    Around = NotesDict["Around"]
                else:
                    EarlyPickup = None
                    LatePickup = None
                    EarlyDrop = None
                    LateDrop = None
                    AltAddress = None
                    Comment = None
                    Around = None

                StudentList = [ID, LastName, FirstName, HashAddress, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart,\
                EarlyPickup, LatePickup, EarlyDrop, LateDrop, Around, AltAddress, Comment]
                
                self.Cursor.execute("Insert Into Student     \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)

        self.__DiscardAddresses()


    def InsertBus(self, RowList):
        Buses = self.GetBuses()
        for Code, Num, Capacity in RowList:
            
            Num = int(Num)                   
            if Code not in Buses:
                ToAdd = [Code, Num, Capacity]   
                self.Cursor.execute("Insert Into Bus    \
                                        Values (?,?,?)", ToAdd)
            else:
                ToAdd = [Num, Capacity, Code]
                self.Cursor.execute("Update Bus     \
                                        Set Number = ?, Capacity = ?    \
                                        Where BusID = ?", ToAdd)


    def InsertDepot(self, RowList):

        for Road, Number, ZipCode, Prefecture, Municipal, Area in RowList:

            FullAddress = util.ConcatenateAddress(Road, Number, ZipCode, Municipal, Area, Prefecture, "GREECE")
            HashAddress = self.__Hash(FullAddress)
            self.__InitMapsHandler()
            FormattedAddress, GPSX, GPSY = self._MapsHandler.Geocode(FullAddress)

            AddressList = [HashAddress, Road, Number, ZipCode, Prefecture, Municipal, Area, GPSX, GPSY,\
            FullAddress, FormattedAddress]

            self.Cursor.execute("Insert Into Depot    \
                                Values (?,?,?,?,?,?,?,?,?,?,?)", AddressList)


    def InsertDistances(self, DayPart, direct=False, fileName=None):


        if DayPart != "Morning" and DayPart != "Noon" and DayPart != "Study":
            raise ValueError("Error: Non valid DayPart.")

        Table = DayPart + "Distance"
      
        self.Cursor.execute(\
                " Select Address.AddressID, Address.GPS_X, Address.GPS_Y    \
                From Address                                                \
                Where exists (  Select *                                    \
                                From Student                                \
                                Where Student.AddressID = Address.AddressID \
                                and Student.DayPart = ?)", [DayPart])

        Addresses = self.Cursor.fetchall()

        Depot = self.GetDepot()
        Depot = (Depot[0], Depot[1], Depot[2])
        Addresses.append(Depot)

        Origins = list()
        for id1, x1, y1 in Addresses:
            Origins.append((id1, (y1, x1)))

        self.__InitMapsHandler()
        Matrix = self._MapsHandler.DistanceMatrix(Origins)
        print Table
        
        if not direct:
            if not fileName:
                "Error: No file was given, writing on \"tempDistances.tsv\""
                fileName = "temp.tsv"

            logcsv = open(fileName, "w+")
            logcsv.write("ID1\tID2\tDuration\tDistance\n")
            for id1, id2, duration, distance in Matrix:
                logcsv.write(str(id1) + "\t" + str(id2) + "\t" + str(duration) + "\t" + str(distance) + "\n")
            logcsv.close()

        else:
            for id1, id2, duration, distance in Matrix:
                # All previous distances will be overwritten
                self.Cursor.execute("Delete From " + Table)
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
        for ID, X, Y, FullAddress in Rows:
            Addresses[ID] = (X, Y, FullAddress)
        
        return Addresses


    def GetBuses(self):
        self.Connect(self.FileName)
        sql = " Select * From Bus"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()
        Buses = dict()
        for ID, Num, Capacity in Rows:
            Buses[ID] = (Num, Capacity)

        return Buses


    def GetStudents(self):
        self.Connect(self.FileName)
        sql = "Select * From Student"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()
        Students = list()
        for row in Rows:
            Students.append(row)

        return Students


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
        for ID in Addresses:
            self.Cursor.execute("Delete From Address Where AddressID = ?", ID)


    def __Hash(self, Address):
        return sha1(Address).hexdigest()


    def __LogGeocodingError(self, ID, FormattedAddress, FullAddress, DayPart, GeoFailsFile):
        valid = True 
        if util.CountNumbers(FormattedAddress) <= 5:
            if "&" not in FormattedAddress and " KAI " not in FormattedAddress:
                valid = False
                if GeoFailsFile:
                    GeoFailsFile.write(str(ID) + "\t" + FormattedAddress + "\t"\
                    + FullAddress + "\t" + DayPart + "\n")
        return valid