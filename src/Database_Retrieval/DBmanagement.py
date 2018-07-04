import sqlite3
import os
import hashlib
from geopy import geocoders
from GRtoEN import GreekDecoder
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

    def InsertStudent(self, RowList, DayPart):
        # Pull Addresses from Database
        Addresses = self.GetAddresses()
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
            FullAddress = ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "GREECE")
            FullAddress = TranslateAddress(FullAddress)
            HashAddress = Hash(FullAddress)
            
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
        for ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri in NoGPS:
            # Concatenate the Address to a single string and hash it
            FullAddress = ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "GREECE")
            if not FullAddress:
                continue
            FullAddress = TranslateAddress(FullAddress)
            HashAddress = Hash(FullAddress)

            # If address has not been added to the database, geocode it and add it
            if not Addresses.has_key(HashAddress):

                self.InitGeolocator()
                FormattedAddress, (GPSX, GPSY) = self.Geocode(FullAddress)

                Addresses[HashAddress] = (GPSX, GPSY)
                AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, FullAddress, FormattedAddress]
                self.Cursor.execute("Insert Into Address    \
                                    Values (?,?,?,?,?,?,?,?,?,?,?)", AddressList)

            # Add student to the database
            StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]
            self.Cursor.execute("Insert Into Student     \
                            Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)


    def InsertBus(self, RowList):
        for Code, Num, Capacity in RowList:
            Num = int(Num)
            ToAdd = [Code, Num, Capacity]
            self.Cursor.execute("Insert Into Bus    \
                                    Values (?,?,?)", ToAdd)


    def InsertDistances(self, distanceFunction):
        sql = "Select AddressID, GPS_X, GPS_Y From Address Where GPS_X Is Not Null and GPS_Y Is Not Null"

        self.Cursor.execute(sql)
        Addresses = self.Cursor.fetchall()
        Distances = list()
        for id1, x1, y1 in Addresses:
            for id2, x2, y2 in Addresses:
                if id1 == id2:
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
            self.GoogleClient = Client(key="AIzaSyBRGHJf69r2tYhvmpJxdayyXfZorTfHu5g")


    def Geocode(self, Address):
        results = geocoding.geocode(self.GoogleClient, address=Address, region="GR")
        locationData = results[0]["geometry"]["location"]
        address = results[0]["formatted_address"]
        return (address, (locationData["lat"], locationData["lng"]))


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



def ConcatenateAddress(Road, Num, ZipCode, Municipal, Area, Prefecture, Country):

    ResultAddress = ""
    if not Road:
        return ResultAddress
    
    ResultAddress += Road + " "
    if Num:
        NewNum = ""
        for char in Num:
            if not char == ' ':
                NewNum += char
        ResultAddress += NewNum + " "
    ResultAddress += ", "
    if Area:
        ResultAddress += Area + " "
    if Municipal:
        ResultAddress += Municipal + " "
    ResultAddress += ", "
    if Prefecture:
        Prefecture = TranslateAddress(Prefecture)
        if Prefecture == "ATTIKIS":
            Prefecture = "ATTIKI"
        elif Prefecture == "PEIRAIOS":
            Prefecture = "PEIRAIAS"
        ResultAddress += Prefecture + " "
    if ZipCode:
        ResultAddress += ZipCode + " "
    ResultAddress += ", "
    if Country:
        ResultAddress += Country
    
    return ResultAddress


def TranslateAddress(Address):
    Decoder = GreekDecoder()
    ResultAddress = Decoder.Decode(Address)
    return ResultAddress
    

def Hash(Address):
    return hashlib.sha1(Address).hexdigest()