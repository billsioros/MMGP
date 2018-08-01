from DBManagement import DBManager as dbm
import util
from itertools import izip
from ortools.constraint_solver import pywrapcp
from ortools.constraint_solver import routing_enums_pb2
import sys


class Vehicle():

    def __init__(self):
        self._capacity = 25

    @property
    def capacity(self):
        return self._capacity


class DataProblem():

    def __init__(self, credFile, rowIndex):
        self._vehicle = Vehicle()
        self._num_vehicles = 4

        GoogleAPI_key, OpenAPI_key, _ServerType, _ServerName, _DatabaseName = util.GetCredentials(credFile, rowIndex)
        DBManager = dbm("MMGP_Data.db", GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

        Addresses = DBManager.GetAddresses()
        Distances = DBManager.GetDistances("Study")
        Depot = DBManager.GetDepot()
        
        Locations = [Depot[0]]
        LocationNames = [Depot[-1]]
        LocationCoords = [(Depot[2], Depot[1])]
        i = 0
        for key in Distances.keys():
            if i == 10:
                break
            Locations.append(key)
            LocationNames.append(Addresses[key][-1])
            LocationCoords.append( (Addresses[key][1], Addresses[key][0] ))
            i += 1

        self._locations = Locations
        self._names = LocationNames
        self._coords = LocationCoords
        self._depot = 0

        self._demands = [0]
        for i in xrange(len(Locations)):
            self._demands.append(1)

    @property
    def vehicle(self):
        return self._vehicle
    
    @property
    def num_vehicles(self):
        return self._num_vehicles
    
    @property
    def locations(self):
        return self._locations

    @property
    def names(self):
        return self._names

    @property
    def coords(self):
        return self._coords

    @property
    def num_locations(self):
        return len(self._locations)

    @property
    def depot(self):
        return self._depot

    @property
    def demands(self):
        return self._demands


class CreateDistanceEvaluator(object):

    def __init__(self, data, credFile, rowIndex):
        self._distances = {}

        GoogleAPI_key, OpenAPI_key, _ServerType, _ServerName, _DatabaseName = util.GetCredentials(credFile, rowIndex)
        DBManager = dbm("MMGP_Data.db", GoogleAPIKey=GoogleAPI_key, OpenAPIKey=OpenAPI_key)

        Distances = DBManager.GetDistances("Study")

        self._distances = list()
        for key1 in Distances.keys():
            location = list()
            for key2 in Distances[key1].keys():
                location.append(Distances[key1][key2][0])
            self._distances.append(location)

    def distance_evaluator(self, from_node, to_node):
        try:
            return self._distances[from_node - 1][to_node - 1]
        except:
            print from_node, to_node
            exit()


class CreateDemandEvaluator(object):
    def __init__(self, data):
        self._demands = data.demands

    def demand_evaluator(self, from_node, to_node):
        del to_node
        return self._demands[from_node]


def add_capacity_contraints(routing, data, demand_evaluator):
    capacity = "Capacity"
    routing.AddDimension(demand_evaluator, 0, data.vehicle.capacity, True, capacity)


class ConsolePrinter():

    def __init__(self, data, routing, assignment, distance_evaluator):
        self._data = data
        self._routing = routing
        self._assignment = assignment
        self.distance_evaluator = distance_evaluator

    @property
    def data(self):
        return self._data
    
    @property
    def routing(self):
        return self._routing

    @property
    def assignment(self):
        return self._assignment
    
    def Print(self):
        total_dist = 0
        for vehicle_id in xrange(self.data.num_vehicles):
            index = self.routing.Start(vehicle_id)
            plan_output = "Route for vehicle {0}:\n".format(vehicle_id)
            route_dist = 0
            route_load = 0
            while not self.routing.IsEnd(index):
                node_index = self.routing.IndexToNode(index)
                next_node_index = self.routing.IndexToNode(
                    self.assignment.Value(self.routing.NextVar(index)))
                route_dist += self.distance_evaluator.distance_evaluator(
                    self.data.locations[node_index],
                    self.data.locations[next_node_index])
                route_load += self.data.demands[node_index]
                plan_output += ' {0} Load({1}) -> '.format(node_index, route_load)
                index = self.assignment.Value(self.routing.NextVar(index))

            node_index = self.routing.IndexToNode(index)
            total_dist += route_dist
            plan_output += ' {0} Load({1})\n'.format(node_index, route_load)
            plan_output += 'Distance of the route: {0}m\n'.format(route_dist)
            plan_output += 'Load of the route: {0}\n'.format(route_load)
            print plan_output 
        print 'Total Distance of all routes: {0}m'.format(total_dist)


def main(fileName, rowIndex):


    data = DataProblem(fileName, rowIndex)

    routing = pywrapcp.RoutingModel(data.num_locations, 1, data.depot)

    distance_evaluator = CreateDistanceEvaluator(data, fileName, rowIndex).distance_evaluator
    search_parameters = pywrapcp.RoutingModel.DefaultSearchParameters()
    routing.SetArcCostEvaluatorOfAllVehicles(distance_evaluator)

    assignment = routing.SolveWithParameters(search_parameters)
    order = list()
    if assignment:
        print "Total distance: " + str(assignment.ObjectiveValue()) + " seconds\n"
        index = routing.Start(0)
        route = ''       
        while not routing.IsEnd(index):
            route += str(data.names[ int(routing.IndexToNode(index)) ]) + ' -> '
            order.append(data.coords[routing.IndexToNode(index)])
            index = assignment.Value(routing.NextVar(index))            
        route += str(data.names[routing.IndexToNode(index)])
        print "Route:\n\n" + route
    else:
        print 'No Soluction found.'

    file = open("PlotDirs.csv", "w+")
    file.write("lat,lon\n")
    for loc in order:
        file.write(str(loc[0]) + "," + str(loc[1]) + "\n")
    file.close()

if __name__ == '__main__':
    fileName = ".\\src\\Database_Retrieval\\DataFiles\\Credentials.csv"#sys.argv[1]
    rowIndex = 0#sys.argv[2]
    main(fileName, rowIndex)