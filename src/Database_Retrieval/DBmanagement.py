import sqlite3
import os
import unidecode
import hashlib


def CreateDatabase(fileName):

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
                FullAddress varchar(255)        \
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

def DestroyDatabase(fileName):
    con = sqlite3.connect(fileName)
    cursor = con.cursor()

    sql = "Drop Table Student"
    cursor.execute(sql)

    sql = "Drop Table Address"
    cursor.execute(sql)

    sql = "Drop Table Distance"
    cursor.execute(sql)

    con.commit()
    con.close()

    os.remove(fileName)
    return

def Connect(fileName):
    Connection = sqlite3.connect(fileName)
    return Connection

def Disconnect(Connection):
    Connection.close()

# RowList Components:
#   StCode
#   StLastName
#   StFirstName
#   SchAddress
#   SchAddressNumber
#   SchZipCode
#   PrefectureDescription
#   MunicipalDescription
#   AreaDescription
#   SchNotes
#   LevelDescription
#   ClassDescription
#   SchMonday
#   SchTuesday,
#   SchWednesday
#   SchThursday
#   SchFriday
#   SchGPS_X
#   SchGPS_Y

def InsertNew(Connection, RowList, DayPart):
    for ID, LastName, FirstName, Road, Num, ZipCode, Prefec, Muni, Area, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, GPSX, GPSY in RowList:
        # Concatenate the Address to a single string and hash it
        Address = ConcatenateAddress(Road, Num, ZipCode, Muni, Area, Prefec, "Greece")
        HashAddress = hashlib.sha1(Address)
        HashAddress = HashAddress.hexdigest()

        StudentList = [ID, LastName, FirstName, HashAddress, Notes, Level, Class, Mon, Tue, Wen, Thu, Fri, DayPart]
        Cursor = Connection.cursor()
        Cursor.execute("Inwert Into Student     \
                        Values (?,?,?,?,?,?,?,?,?,?,?,?,?)", StudentList)
        
        AddressList = [HashAddress, Road, Num, ZipCode, Prefec, Muni, Area, GPSX, GPSY, Address]



def ConcatenateAddress(Road, Num, ZipCode, Municipal, Area, Prefecture, Country):

    ResultAddress = ""
    if not Road:
        return ResultAddress
    
    ResultAddress += Road + " "
    if Num:
        ResultAddress += Num + " "
    ResultAddress += ", "
    if Area:
        ResultAddress += Area + " "
    if Municipal:
        ResultAddress += Municipal + " "
    ResultAddress += ", "
    if Prefec:
        ResultAddress += Prefec + " "
    if ZipCode:
        ResultAddress += ZipCode + " "
    ResultAddress += ", "
    if Country:
        ResultAddress += Country
    
    return ResultAddress

def TranslateAddress(Address):
    ResultAddress = unidecode.unidecode(Address)
    return ResultAddress
    

def HashAddress(Address):
    print "hi"
