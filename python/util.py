from itertools import izip
from googlemaps import geocoding as ggeo, distance_matrix as gd, directions, Client as gclient
from openrouteservice import distance_matrix as od, geocoding as ogeo, client as orclient
import time
import sys
import csv
import json

class GreekDecoder:

    def __init__(self, number=False):
        self.LetterDictionary = dict()
        self.isNumberDecoder = number
        EngLetters = ["'", 'A', 'V', 'G', 'D', 'E', 'Z', 'I', 'TH', 'K', 'L', 'M', 'N', 'KS', 'O', 'P', 'R', 'S', 'T', 'Y', 'F', 'CH', 'PS']

        if self.isNumberDecoder:
            EngLetters[2] = 'B'

        GrLetters = [\
            [u'\u0384'],\
            ['\x80', u'\u0391', u'\u0386'],\
            ['\x81', u'\u0392'],\
            ['\x82', u'\u0393'],\
            ['\x83', u'\u0394'],\
            ['\x84', u'\u0395', u'\u0388'],
            ['\x85', u'\u0396'],\
            ['\x86', '\x88', u'\u0397', u'\u0399', u'\u03aa', u'\u0389', u'\u038a'],\
            ['\x87', u'\u0398'],\
            ['\x89', u'\u039a'],\
            ['\x8a', u'\u039b'],\
            ['\x8b', u'\u039c'],\
            ['\x8c', u'\u039d'],\
            ['\x8d', u'\u039e'],\
            ['\x8e', '\x97', u'\u039f', u'\u03a9', u'\u038c', u'\u038f'],\
            ['\x8f', u'\u03a0'],\
            ['\x90', u'\u03a1'],\
            ['\x91', u'\u03a3'],\
            ['\x92', u'\u03a4'],\
            ['\x93', u'\u03a5', u'\u03ab', u'\u038e'],\
            ['\x94', u'\u03a6'],\
            ['\x95', u'\u03a7'],\
            ['\x96', u'\u03a8'] ]

        for Gr, En in izip(GrLetters, EngLetters):
            for letter in Gr:
                self.LetterDictionary[letter] = En

    def Decode(self, String):
        NewString = ""
        for Char in String:
            if self.LetterDictionary.has_key(Char):
                NewChar = self.LetterDictionary[Char]
            else:
                NewChar = Char
            NewString += NewChar
        return NewString


class GreekMunicipalConverter:
    
    def __init__(self):
        self.Municipals = {\
        "ALIMOY" : "ALIMOS",
        "ILIOYPOLEOS" : "ILIOYPOLI",
        "AGIOY DIMITRIOY" : "AGIOS DIMITRIOS",
        "YMITTOY" : "YMITTOS",
        "KALLITHEAS" : "KALLITHEA",
        "NEAS SMYRNIS" : "NEA SMIRNI",
        "PALAIOY FALIROY" : "PALAIO FALIRO",
        "PEIRAIOS" : "PEIRAIAS",
        "ATTIKIS" : "ATTIKI",
        "GLYFADAS" : "GLYFADA",
        "ARGYROYPOLEOS" : "ARGIROYPOLI",
        "VYRONOS" : "VYRONAS",
        "MOSCHATOY" : "MOSCHATO",
        "DAFNIS" : "DAFNI",
        "VARIS" : "VARI",
        "VOYLAS": "VOYLA",
        "ELLINIKOY" : "ELLINIKO",
        "TAYROY" : "TAYROS",
        "ATHINAION" : "ATHINA",
        "KALYVION THORIKOY": "KALYVIA",
        "VOYLIAGMENIS": "VOYLIAGMENI"}

    def Convert(self, Municipal):
        return self.Municipals[Municipal]


class MapsHandler:

    def __init__(self, GoogleAPIKey=None, OpenAPIKey=None, prefferedDistanceMatrix="Open"):
        self.GoogleAPIKey = GoogleAPIKey       
        self.OpenAPIKey = OpenAPIKey
        self.GoogleClient = gclient(key=self.GoogleAPIKey)
        self.OpenRouteClient = orclient.Client(key=self.OpenAPIKey)

        if prefferedDistanceMatrix != "Open" and prefferedDistanceMatrix != "Google":
            raise ValueError("prefferedDistanceMatrix should be \"Open\" or \"Google\"")
        elif prefferedDistanceMatrix == "Open":
            self.DistanceMatrix = self.__OpenDistanceMatrix
        elif prefferedDistanceMatrix == "Google":
            self.DistanceMatrix = self.__GoogleDistanceMatrix


    def Geocode(self, Address):

        results = ggeo.geocode(self.GoogleClient, address=Address, region="GR")


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
                    return [None, None, None]

        address = results[0]["formatted_address"]
        return (address, locationData["lng"], locationData["lat"])

    """locations : (Hash, (lat, lng)/AddressString ) """
    """Returns Distance Matrix of origins->destinations. Google has a limit on how many origins or destinations
       one can ask through a request, so: with n origins we have ceil(n/25)*n requests.
       We also wait 0.5s after every 100 requests. Google limits usage to 100 requests per second."""
    def __GoogleDistanceMatrix(self, locations):
        
        Destinations = dict()
        Destinations["Points"] = list()
        Destinations["IDs"] = list()

        for ID, Point in locations:
            Destinations["Points"].append(Point)
            Destinations["IDs"].append(ID)

        destinationsPerOrigin = 25

        Matrix = list()
        
        requests = 0
        originCount = 1
        print "Origins:" , len(locations)

        StartTime = time.time()
        OriginTimes = 0
        RequestTimes = 0

        for orID, orPoint in locations:

            OriginStartTime = time.time()
            print "Current Origin: ", originCount

            RestDestinationPoints = Destinations["Points"]
            RestDestinationIDs = Destinations["IDs"]

            while RestDestinationIDs:
                # Get first 25 destinations from Remaining Destinations
                DestinationPoints = RestDestinationPoints[0:destinationsPerOrigin]
                DestinationIDS = RestDestinationIDs[0:destinationsPerOrigin]
                # Remaining Destinations now start 25 indices to the right
                RestDestinationPoints = RestDestinationPoints[destinationsPerOrigin:]
                RestDestinationIDs = RestDestinationIDs[destinationsPerOrigin:]
       
                if (requests % 100) == 0:
                    time.sleep(0.5)
                RequestStartTime = time.time()

                results = gd.distance_matrix(self.GoogleClient, orPoint, DestinationPoints) 
                                                        #mode="driving",\
                                                        #avoid="tolls", units="metric", region="GR")

                RequestEndTime = time.time()
                RequestDuration = RequestEndTime - RequestStartTime
                RequestTimes += RequestDuration

                requests += 1

                # results["rows"] has always only one row since we give only one origin point
                row = results["rows"]

                for elem, deID in izip(row[0]["elements"], DestinationIDS):
                    Matrix.append([orID, deID, elem["duration"]["value"], elem["distance"]["value"]])

            OriginEndTime = time.time()   
            OriginDuration = OriginEndTime - OriginStartTime
            OriginTimes += OriginDuration         
            originCount += 1

        print "Requests: ", requests
        EndTime = time.time()
        WholeDuration = EndTime - StartTime
        (Mins, Secs) = SecondsToMinutes(WholeDuration)

        try:
            TimeLog = open("TimeLogs.txt", "a+")
            TimeLog.write("\n")
            full =  "Time elapsed for " + str(requests) + "requests (" + str(len(locations)) + "x" + str(len(locations)) + "):"\
            + str(Mins) + "min, " + str(Secs) + "sec"
            ori =  "Time per origin: " + str(OriginTimes / len(locations)) + " sec"
            req =  "Time per request: " + str(RequestTimes / requests) + " sec"
            print full
            print ori
            print req

            TimeLog.write(full + "\n")
            TimeLog.write(ori + "\n")
            TimeLog.write(req + "\n")
        except:
            print "Syntax error in TimeLogging"

        return Matrix


    def __OpenDistanceMatrix(self, locations):
        Locations = dict()
        Locations["IDs"] = list()
        Locations["Points"] = list()

        for ID, (y, x) in locations:
            Locations["IDs"].append(ID)
            Locations["Points"].append((x, y))


        locationsPerRequest = 2500

        originsPerRequest = int(locationsPerRequest / len(locations))

        currentOriginStart = 0

        RestOriginsIDs = Locations["IDs"]

        Matrix = list()

        requests = 0
        StartTime = time.time()
        RequestTimes = 0

        while RestOriginsIDs:

            OriginIDs = RestOriginsIDs[0:originsPerRequest]
            RestOriginsIDs = RestOriginsIDs[originsPerRequest:]

            sources = list()
            for j in range(currentOriginStart, currentOriginStart + originsPerRequest):
                if j <= len(Locations["Points"]) - 1:
                    sources.append(j)

            RestLocationIDs = Locations["IDs"]
            RestLocationPoints = Locations["Points"]
            
            while RestLocationPoints:
                Points = RestLocationPoints[0:locationsPerRequest]
                IDs = RestLocationIDs[0:locationsPerRequest]
                RestLocationPoints = RestLocationPoints[locationsPerRequest:]
                RestLocationIDs = RestLocationIDs[locationsPerRequest:]
                print sources

                RequestStartTime = time.time()

                results = od.distance_matrix(self.OpenRouteClient, Points, sources=sources,\
                 metrics=["duration","distance"])

                RequestEndTime = time.time()
                RequestDuration = RequestEndTime - RequestStartTime
                RequestTimes += RequestDuration

                requests += 1

                # durations is a double list
                for id1, originRowDuration, originRowDistance, in izip(OriginIDs, results["durations"], results["distances"]):
                    for id2, duration, distance in izip(IDs, originRowDuration, originRowDistance):
                        Matrix.append([id1, id2, duration, distance])

            
            currentOriginStart += originsPerRequest

        print "Requests: ", requests
        EndTime = time.time()
        WholeDuration = EndTime - StartTime
        (Mins, Secs) = SecondsToMinutes(WholeDuration)

        try:
            TimeLog = open("../data/TimeLogs.txt", "a+")
            TimeLog.write("\n")
            full =  "Time elapsed for " + str(requests) + " requests (" + str(len(locations))\
             + "x" + str(len(locations)) + "): " + str(Mins) + "min, " + str(Secs) + "sec"

            req =  "Time per request: " + str(RequestTimes / requests) + " sec"
            print full
            print req

            TimeLog.write(full + "\n")
            TimeLog.write(req + "\n")
        except:
            print "Syntax error in TimeLogging"

        return Matrix
        

    def Directions(self, origin, destination, waypoints):
        results = directions.directions(self.GoogleClient, origin, destination, waypoints=waypoints,\
         mode="driving", optimize_waypoints=True)
        return results
        

class Parser:

    def __init__(self):
        PossibleKeyWords = [\
        ["NORIT EPIV", "NORIT. EPIV.", "NORIT EPIV.", "NORIT. EPIV", "NORIT.EPIV", "NORIT.EPIV.", "NORITEPIV", "NORITERI EPIVIVASI"],\
        ["ARGOT EPIV", "ARGOT. EPIV.", "ARGOT EPIV.", "ARGOT. EPIV", "ARGOT.EPIV", "ARGOT.EPIV.", "ARGOTEPIV", "ARGOTERI EPIVIVASI"],\
        ["NORIT APOV", "NORIT. APOV.", "NORIT APOV.", "NORIT. APOV", "NORIT.APOV", "NORIT.APOV.", "NORITAPOV", "NORITERI APOVIVASI"],\
        ["ARGOT APOV", "ARGOT. APOV.", "ARGOT APOV.", "ARGOT. APOV", "ARGOT.APOV", "ARGOT.APOV.", "ARGOTAPOV", "ARGOTERI APOVIVASI"],\
        ["DIEYTHINSI", "DIEYTHINSI.", "DIEYTH", "DIEYTH."],\
        ["LOIPA", "LOIPA."],\
        ["PERIPOY", "PERIPOY."]
        ]

        ActualKeyWords = ["Early Pickup", "Late Pickup", "Early Drop", "Late Drop", "Address", "Comments", "Around"]

        self.KeyWords = dict()
        for actual, possible in izip(ActualKeyWords, PossibleKeyWords):
            self.KeyWords[actual] = possible


    def Parse(self, Note):
        Decoder = GreekDecoder()

        Note = Decoder.Decode(Note)

        Indices = dict()
        Keys = dict()
        Words = dict()
        for key in self.KeyWords.keys():
            for word in self.KeyWords[key]:
                index = Note.find(word)
                if index != -1:
                    Indices[key] = index
                    Keys[index] = key
                    Words[key] = word
                    break

        indiceList = list()
        for key in Indices.keys():
            indiceList.append(Indices[key])

        indiceList = sorted(indiceList)

        Data = dict()
        for i in range(0, len(indiceList)):
            if i == len(indiceList) - 1:
                Data[Keys[indiceList[i]]] = Note[indiceList[i] :]
            else:
                Data[Keys[indiceList[i]]] = Note[indiceList[i] : indiceList[i + 1]]

        for key in Data.keys():
            Data[key] = Data[key].replace(Words[key], "")
            Data[key] = Data[key].strip(" ")
            Data[key] = Data[key].strip(",")
            Data[key] = Data[key].strip(":")
            Data[key] = Data[key].strip(" ")
            if key != 'Comments' and key != 'Address':
                
                Data[key] = self.__FixHour(Data[key])

        for key in self.KeyWords:
            if key not in Data.keys():
                Data[key] = None

        return Data

    def __FixHour(self, hour):

        if ":" in hour:
            hour = hour.replace(":", ".")
        if "," in hour:
            hour = hour.replace(",", ".")
        
        return hour


def ConvertGenitive(Genitive):
    Converter = GreekMunicipalConverter()
    ResultNominative = Converter.Convert(Genitive)
    return ResultNominative
    

def ConcatenateAddress(Road, Num, ZipCode, Municipal, Area, Prefecture, Country):
    # print (Road, Num, ZipCode, Municipal, Area, Prefecture, Country)
    ResultAddress = ""
    TranslatedAddress = ""

    if not Road:
        return (ResultAddress, TranslatedAddress)
    
    TranslatedAddress += TranslateAddress(Road) + " "
    ResultAddress += Road + " "

    if "&" not in Road and " KAI " not in Road:
        if Num:
            NewNum = ""

            for char in Num:
                if char == '-':
                    break
                if char != ' ':
                    NewNum += char
            
            TranslatedAddress += TranslateNumber(NewNum)
            ResultAddress += NewNum

    ResultAddress += ", "
    TranslatedAddress += ", "

    if Municipal:    
        TrMunicipal = TranslateAddress(Municipal)
        TrMunicipal = ConvertGenitive(TrMunicipal)

        ResultAddress += Municipal + " "
        TranslatedAddress += TrMunicipal + " "

    
    if ZipCode:
        ResultAddress += ZipCode + " "  
        TranslatedAddress += ZipCode + " "

    if Prefecture:
        TrPrefecture = TranslateAddress(Prefecture)
        if TrPrefecture == "PEIRAIOS":
            TrPrefecture = ConvertGenitive(TrPrefecture)
            TranslatedAddress += TrPrefecture
            ResultAddress += ", " + Prefecture
    
    return (ResultAddress, TranslatedAddress)


def TranslateAddress(Address):
    Decoder = GreekDecoder()
    ResultAddress = Decoder.Decode(Address)
    return ResultAddress

def TranslateNumber(Number):
    Decoder = GreekDecoder(number=True)
    ResultNumber = Decoder.Decode(Number)
    return ResultNumber

def CountNumbers(String):
    if not isinstance(String, basestring):
        return -1

    Numbers = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']

    count = 0
    for c in String:
        if c in Numbers:
            count += 1
    
    return count


from math import radians, sin, cos, atan2, sqrt

def harvesine(u, v):
    # u and v are tuples
    # 0 is longitude 1 is latitude
    R = 6371
    lat1 = radians(u[1])
    lat2 = radians(v[1])
    Dlat = radians(v[1] - u[1])
    Dlon = radians(v[0] - u[0])

    a = sin(Dlat / 2) * sin(Dlat / 2) + cos(lat1) * cos(lat2) * sin(Dlon / 2) * sin(Dlon / 2)
    c = 2 * atan2(sqrt(a), sqrt(1 - a))

    d = R * c
    return d


def SecondsToMinutes(duration):
    Minutes = int(duration / 60)
    Seconds = int(duration % 60)
    return (Minutes, Seconds)
    

def GetCredentials(SettingsFile):
    GoogleAPI_key = None
    OpenAPI_key = None
    ServerType = None
    ServerName = None
    DatabaseName = None
    Username = None
    Password = None

    with open(SettingsFile, "r") as json_file:
        Settings = json.load(json_file)

    activeConnection = str(Settings["Connection"]["Active"])

    GoogleAPI_key = Settings["GoogleAPI_Key"]
    OpenAPI_key = Settings["OpenAPI_Key"]

    ServerType = Settings["Connection"][activeConnection]["Driver"]
    ServerName = Settings["Connection"][activeConnection]["Server"]
    DatabaseName = Settings["Connection"][activeConnection]["Database"]
    Username = Settings["Connection"][activeConnection]["Username"]
    Password = Settings["Connection"][activeConnection]["Password"]

    if not Username or not Password:
        Username = None
        Password = None

    return [activeConnection, GoogleAPI_key, OpenAPI_key,  ServerType, ServerName, DatabaseName, Username, Password]


def GetSetting(SettingsFile, Keys):

    with open(SettingsFile, "r") as json_file:
        Settings = json.load(json_file)

    SettingsToReturn = []
    
    if isinstance(Keys, list):
        for key in Keys:
            settingToReturn = dict()
            if isinstance(key, list):
                keylist = key
                settingToReturn = Settings[keylist[0]]
                keylist = keylist[1:]
                

                for subkey in keylist:
                    settingToReturn = settingToReturn[subkey]
            else:
                settingToReturn = Settings[key]

            SettingsToReturn.append(settingToReturn)
    else:
        SettingsToReturn.append(Settings[Keys])

    return SettingsToReturn


def RowListToDict(RowList, ColumnNames):
    results = []
    for row in RowList:
        d = {}
        for field, column in izip(row, ColumnNames):
            d[column] = field
        results.append(d)

    return results
