from itertools import izip

class GreekDecoder:

    def __init__(self):
        self.LetterDictionary = dict()
        EngLetters = ['A', 'V', 'G', 'D', 'E', 'Z', 'I', 'TH', 'K', 'L', 'M', 'N', 'KS', 'O', 'P', 'R', 'S', 'T', 'Y', 'F', 'CH', 'PS']
        GrLetters = [\
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


class MunicipalConverter:
    
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
        "ATHINAION" : "ATHINA"}

    def Convert(self, Municipal):
        return self.Municipals[Municipal]


def ConvertGenitive(Genitive):
    Converter = MunicipalConverter()
    ResultNominative = Converter.Convert(Genitive)
    return ResultNominative
    

def ConcatenateAddress(Road, Num, ZipCode, Municipal, Area, Prefecture, Country):

    ResultAddress = ""
    if not Road:
        return ResultAddress
    
    Road = TranslateAddress(Road)
    ResultAddress += Road + " "

    if "&" not in Road and " KAI " not in Road:
        if Num:
            NewNum = ""
            Num = TranslateAddress(Num)
            for char in Num:
                if not char == ' ':
                    NewNum += char
            ResultAddress += NewNum + " "
    ResultAddress += ", "

    # if Area:
    #     Area = TranslateAddress(Area)
    #     ResultAddress += Area + " "

    if Municipal:# and not Area:       
        Municipal = TranslateAddress(Municipal)
        Municipal = ConvertGenitive(Municipal)
        ResultAddress += Municipal + " "

    
    if ZipCode:
        ResultAddress += ZipCode + " "  

    if Prefecture:
        Prefecture = TranslateAddress(Prefecture)
        if Prefecture == "PEIRAIOS":
            ResultAddress += ", "
            Prefecture = ConvertGenitive(Prefecture)
            ResultAddress += Prefecture + " "

    

    #ResultAddress += ", "

    #if Country:
        #ResultAddress += Country
    
    return ResultAddress


def TranslateAddress(Address):
    Decoder = GreekDecoder()
    ResultAddress = Decoder.Decode(Address)
    return ResultAddress


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
