import sqlite3
import os
import hashlib
from geopy import geocoders
from GRtoEN import GreekDecoder

class DBManager:

    """ Connects to an sqlite3 database if it exists in current directory, or creates a new one
        Connection = current connection """
    def __init__(self, fileName, APIKey=None):
        if not os.path.isfile(fileName):
            print "Creating new Database..."
            self.CreateDatabase(fileName)
        self.Connection = None
        self.Cursor = None
        self.FileName = fileName
        self.APIKey = APIKey
        self.GeoLocator = None

        self.Connect(fileName)
        

    def CreateDatabase(self, fileName):

        if os.path.isfile(fileName):
            os.remove(fileName)
        con = sqlite3.connect(fileName)
        cursor = con.cursor()


        sql = "CREATE TABLE Student (               \
                    ID          varchar(255),       \
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

        cursor.execute(sql)

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
                    FullAddress varchar(255),        \
                    Primary Key (AddressID)         )"

        cursor.execute(sql)

        sql = "Create Table Distance (              \
                    AddressID_1 varchar(255),       \
                    AddressID_2 varchar(255),       \
                    Distance    varchar(255),       \
                    Primary Key (AddressID_1, AddressID_2),                  \
                    Foreign Key (AddressID_1) References Address(AddressID), \
                    Foreign Key (AddressID_2) References Address(AddressID)  )"

        cursor.execute(sql)
        con.commit()
        con.close()

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

    def Insert(self, RowList, DayPart):
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
                AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, FullAddress]
                self.Cursor.execute("Insert Into Address    \
                                    Values (?,?,?,?,?,?,?,?,?,?)", AddressList)
            
            # Add student to the database
            StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]
            self.Cursor.execute("Insert Into Student     \
                            Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)
            
        return

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
                address, (GPSX, GPSY) = self.GeoLocator.geocode(FullAddress)
                Addresses[HashAddress] = (GPSX, GPSY)
                AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, FullAddress]
                self.Cursor.execute("Insert Into Address    \
                                    Values (?,?,?,?,?,?,?,?,?,?)", AddressList)

            # Add student to the database
            StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]
            self.Cursor.execute("Insert Into Student     \
                            Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)


    def Commit(self):
        if self.Connection:
            self.Connection.commit()


    def RollBack(self):
        if self.Connection:
            self.Connection.rollback()


    def InitGeolocator(self):
        if not self.GeoLocator:
            self.GeoLocator = geocoders.Bing(api_key=self.APIKey, format_string="%s, Attiki Greece")


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
