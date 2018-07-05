import sqlite3
import os
from time import sleep
from hashlib import sha1 as sha1
import util
from googlemaps import geocoding, Client

class DBManager:

    """ Connects to an sqlite3 database if it exists in current directory, or creates a new one
        Connection = current connection """
    def __init__(self, fileName, APIKey=None):
        self.Connection = None
        self.Cursor = None

        if not os.path.isfile(fileName):
            print "Creating new Database..."
            self.CreateDatabase(fileName)

        self.FileName = fileName
        self.APIKey = APIKey
        self.GeoLocator = None
        self.GoogleClient = None

        self.Connect(fileName)
        

    def CreateDatabase(self, fileName):

        self.Connect(fileName)

        sql = "CREATE TABLE Student (               \
                    StudentID   varchar(255),       \
                    LastName    varchar(255),       \
                    FirstName   varchar(255),       \
                    AddressID   varchar(255),       \
                    Notes       text,               \
                    Level       varchar(255),       \
                    Class       varchar(255),       \
                    Monday      bit,                \
                    Tuesday     bit,                \
                    Wednesday   bit,                \
                    Thursday    bit,                \
                    Friday      bit,                \
                    DayPart     varchar(255),       \
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

        sql = "Create Table Distance (              \
                    AddressID_1 varchar(255),       \
                    AddressID_2 varchar(255),       \
                    Distance    varchar(255),       \
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
                
                HashAddress = self.Hash(FullAddress)
                
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
                StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]

                self.Cursor.execute("Insert Into Student     \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)


            # Insert All Records that do not have GPS coordinates
            i = 0 # Geocoding per sec

            
            for ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area, Notes,\
            Level, Class, Mon, Tue, Wen, Thu, Fri in NoGPS:

                # Concatenate the Address to a single string and hash it
                FullAddress = util.ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "GREECE")
                if not FullAddress:
                    continue

                HashAddress = self.Hash(FullAddress)

                # If address has not been added to the database, geocode it and add it
                if not Addresses.has_key(HashAddress):

                    self.InitGeolocator()
                    # Only 100 requests per sec
                    if i == 99:
                        sleep(4) # Sleep 4 seconds for safety
                        i = 0
                    FormattedAddress, GPSX, GPSY = self.Geocode(FullAddress)
                    i += 1

                    valid = True
                    if FormattedAddress:
                        Addresses[HashAddress] = (GPSX, GPSY)
                    
                        # Find Error and Log it into a csv of your choosing
                        valid = self.LogError(ID, FormattedAddress, FullAddress, DayPart, GeoFailsFile)
                    
                    if valid:
                        AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY,\
                        FullAddress, FormattedAddress]

                        self.Cursor.execute("Insert Into Address    \
                                            Values (?,?,?,?,?,?,?,?,?,?,?)", AddressList)

                # Add student to the database
                StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]

                self.Cursor.execute("Insert Into Student     \
                                Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)

        self.DiscardAddresses()


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


    def InsertDistances(self, distanceFunction):
        OldDistances = self.GetDistances()

        sql = "Select AddressID, GPS_X, GPS_Y From Address Where GPS_X Is Not Null and GPS_Y Is Not Null"

        self.Cursor.execute(sql)
        Addresses = self.Cursor.fetchall()
        Distances = list()
        for id1, x1, y1 in Addresses:
            for id2, x2, y2 in Addresses:
                # If we have calculated the address before dont calculate it again
                if OldDistances.has_key(id1):
                    flag = False
                    for OldID2, _D in OldDistances[id1]:                   
                        if OldID2 == id2:
                            # either update or keep old distance. For now continue
                            flag = True
                            break
                    if flag:
                        continue

                if id1 == id2:
                    distance = 0
                    Distances.append([id1, id2, distance])
                    continue
                distance = distanceFunction((x1, y1), (x2, y2))
                Distances.append([id1, id2, distance])
        
        for ToAdd in Distances:
            self.Cursor.execute("Insert into Distance     \
                                    Values(?,?,?)", ToAdd)


    def Commit(self):
        if self.Connection:
            self.Connection.commit()


    def RollBack(self):
        if self.Connection:
            self.Connection.rollback()


    def InitGeolocator(self):       
        if not self.GoogleClient:
            self.GoogleClient = Client(key=self.APIKey)


    def Geocode(self, Address):
        results = geocoding.geocode(self.GoogleClient, address=Address, region="GR")

        # If Weird Results Let Me Know and return
        try:
            if results[0]["geometry"]["location"]:
                locationData = results[0]["geometry"]["location"]
        except IndexError:
            try:
                print "First Error"
                print results[0]["geometry"]
            except IndexError:
                try:
                    print "Second Error"
                    print results[0]
                except IndexError:
                    print "Third Error"
                    print Address
                    print results
                    return

        address = results[0]["formatted_address"]
        return (address, locationData["lng"], locationData["lat"])


    def GetAddresses(self):
        self.Connect(self.FileName)
        sql = " Select AddressID, GPS_X, GPS_Y  \
                From Address"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        Addresses = dict()
        for ID, X, Y in Rows:
            Addresses[ID] = (X, Y)
        
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


    def GetDistances(self):
        self.Connect(self.FileName)
        sql = "Select * From Distance"
        self.Cursor.execute(sql)
        Rows = self.Cursor.fetchall()

        sql = "Select AddressID From Address"
        self.Cursor.execute(sql)
        Addresses = self.Cursor.fetchall()

        Distances = dict()
        for ad in Addresses:
            Distances[ad[0]] = list()
        
        for ID1, ID2, Distance in Rows:
            Distances[ID1].append((ID2, Distance))
        
        return Distances

    
    def DiscardAddresses(self):
        self.Connect(self.FileName)
        self.Cursor.execute("Select Address.AddressID   \
                            From Address                \
                            Where Not Exists            \
                                (Select * From Student  \
                                Where Student.AddressID = Address.AddressID)")
        
        Addresses = self.Cursor.fetchall()
        for ID in Addresses:
            self.Cursor.execute("Delete From Address Where AddressID = ?", ID)


    def Hash(self, Address):
        return sha1(Address).hexdigest()


    def LogError(self, ID, FormattedAddress, FullAddress, DayPart, GeoFailsFile):
        valid = True 
        if util.CountNumbers(FormattedAddress) <= 5:
            if "&" not in FormattedAddress and " KAI " not in FormattedAddress:
                valid = False
                if GeoFailsFile:
                    GeoFailsFile.write(str(ID) + "\t" + FormattedAddress + "\t"\
                    + FullAddress + "\t" + DayPart + "\n")
        return valid