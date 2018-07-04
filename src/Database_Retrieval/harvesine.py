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
