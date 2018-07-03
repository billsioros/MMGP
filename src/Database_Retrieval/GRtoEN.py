from itertools import izip

class GreekDecoder:

    def __init__(self):
        self.LetterDictionary = dict()
        EngLetters = ['A', 'B', 'G', 'D', 'E', 'Z', 'I', 'TH', 'K', 'L', 'M', 'N', 'KS', 'O', 'P', 'R', 'S', 'T', 'Y', 'F', 'CH', 'PS']
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

