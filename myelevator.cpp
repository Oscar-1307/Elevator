#include <algorithm> //Used for the sort algorithm
#include <array>     //This program uses the array objects
#include <fstream>   //Used to read the file of requests
#include <iostream>  //Used to display messages
#include <memory>    //Used for the shared pointers
#include <queue>     //Structure for the queue of requests
#include <thread>    //Used to make the threads to calculate the time in each elevator
#include <vector>    //Used to store the requests in an array of vectors

using namespace std;

const unsigned char NUM_ELEVATORS = 3;   // Number of elevatos
const unsigned char NUM_FLOORS = 20;     // Number of floors
const unsigned short CAPACITY = 750;     // Capacity in KG
const unsigned char TIME_TO_SERVE = 2;   // Time steps to deliver a person(s) to the destination
const unsigned char AVERAGE_WEIGHT = 75; // Average weight in each request
const unsigned short MAX_TIME = 500;     // Total time in the day

unsigned int timeTotal = 0;

// Represents the requests that the elevator are going to store
class Request
{
private:
    unsigned char startFloor;       // Save the first stop of the elevator
    unsigned char destinationFloor; // Save the destination of the elevator
    unsigned int timeStep;          // The time step the request was done
    bool reachStart;                // Used to check when the elevator takes a request

public:
    // Default constructor
    Request(unsigned char start, unsigned char destination, unsigned int time) : startFloor(start),
                                                                                 destinationFloor(destination),
                                                                                 timeStep(time),
                                                                                 reachStart(false) {}
    // Gets and sets for the elements in the object
    unsigned char getStartFloor()
    {
        return startFloor;
    }
    unsigned char getDestinationFloor()
    {
        return destinationFloor;
    }
    unsigned int getTimeStep()
    {
        return timeStep;
    }
    bool getReachStart()
    {
        return reachStart;
    }
    void setReachStart(bool condition)
    {
        reachStart = condition;
    }
};

// Represents the elevator itself
class Elevator
{
private:
    unsigned char ID;              // ID to identify the elevator, used more for logs porpuses
    unsigned char currentFloor;    // Stores the current floor of the elevator
    bool isGoingUp;                // Stores if the elevator is going up or down
    unsigned short capacityWeight; // Represents the capacity of the elevator
    unsigned short currentWeight;  // Represents the current weight of the elevator
    unsigned char pendingRequests; // Stores the number of requests left
    unsigned int elevatorTime;
    array<vector<shared_ptr<Request>>, NUM_FLOORS> requestsPerFloor; // Stores a shared pointer to every request in every floor

    // Function used to calculate the time of waiting for every request in a journey (start floor, current floor and destination floor)
    unsigned char stopsInTheJourney(const unsigned char startFloor, const unsigned char destinationFloor)
    {
        unsigned char time = 0;
        array<unsigned char, 3> floors = {startFloor, currentFloor, destinationFloor}; // Stores all the floors in the journey
        sort(floors.begin(), floors.end());                                            // Insertion sort alghorithm to organize the stops in the journey
        for (unsigned char i = floors[0]; i < floors[2]; i++)                          // Checks every stops in the route and returs the total time of stops
        {
            if (!requestsPerFloor[i].empty())
                time += TIME_TO_SERVE;
        }

        return time; // Returns the sum of the stops
    }

    // Checks the stops from floor A to floor B
    unsigned char stopsInTheRoute(const unsigned char startFloor, const unsigned char destinationFloor)
    {
        unsigned char time = 0;
        // Checks every stops in the route and returs the total time of stops, the for loop it is different depending of the order
        if (startFloor < destinationFloor)
        {
            for (unsigned char i = startFloor; i < destinationFloor; i++)
            {
                if (!requestsPerFloor[i].empty())
                    time += TIME_TO_SERVE; // Checks every stops in the route and returs the total time of stops
            }
        }
        else
        {
            for (unsigned char i = destinationFloor; i < startFloor; i++)
            {
                if (!requestsPerFloor[i].empty())
                    time += TIME_TO_SERVE;
            }
        }
        return time;
    }
    // Check the farest floor with a request and returns the floor, if there is no pending requests returns the current floor
    unsigned char farestFloorWithRequest()
    {
        if (isGoingUp)
        {
            for (unsigned char i = NUM_FLOORS - 1; i >= currentFloor; i--)
            {
                if (!requestsPerFloor[i].empty())
                    return i;
            }
        }
        else
        {
            for (unsigned char i = 0; i <= currentFloor; i++)
            {
                if (!requestsPerFloor[i].empty())
                    return i;
            }
        }
        return currentFloor;
    }
    // Default constructor for the class, used to create the threads
public:
    Elevator() : ID(0),
                 currentFloor(0),
                 isGoingUp(true),
                 capacityWeight(CAPACITY),
                 currentWeight(0),
                 pendingRequests(0),
                 elevatorTime(0) {}

    // Constructor for elevator, only assigns an ID
    Elevator(unsigned char id) : ID(id),
                                 currentFloor(0),
                                 isGoingUp(true),
                                 capacityWeight(CAPACITY),
                                 currentWeight(0),
                                 pendingRequests(0),
                                 elevatorTime(0)
    {
        for (auto &floor : requestsPerFloor)
        {
            floor.reserve(4);
        }
    }

    // Gets and sets for the elements in the object
    unsigned char getID()
    {
        return ID;
    }
    unsigned char getCurrentFloor()
    {
        return currentFloor;
    }
    bool getIsGoingUp()
    {
        return isGoingUp;
    }
    unsigned short getCapacityWeight()
    {
        return capacityWeight;
    }
    unsigned short getCurrentWeight()
    {
        return currentWeight;
    }
    unsigned char getPendingRequests()
    {
        return pendingRequests;
    }
    unsigned int getElevatorTime()
    {
        return elevatorTime;
    }

    // Method used to update the state of the elevator, first dispatch the current requests, subsequently changes the floor of the elevator
    void update()
    {
        // Only changes the state of the elevator if has pending requests
        if (pendingRequests > 0)
        {
            if (!requestsPerFloor[currentFloor].empty()) // Checks if the current floor has requests
            {
                auto it = requestsPerFloor[currentFloor].begin();  // Creates an iterator for the vector
                while (it != requestsPerFloor[currentFloor].end()) // The loops repites untill has checked all the requests.
                {
                    // First check if the request its for the start floor or the destination floor
                    if (currentFloor == (*it)->getStartFloor())
                    {
                        (*it)->setReachStart(true);                     // If is for the start floor stores that the elevator has passed the floor
                        currentWeight += AVERAGE_WEIGHT;                // Adds the average weight of the requests
                        if (it != requestsPerFloor[currentFloor].end()) //
                        {
                            // Since the requests are stored in an array, it is not efficient to delete the element directly by the subsequent memory ordering.
                            // Therefore, the last element is copied to the current position and the last element is deleted from the array.
                            *it = requestsPerFloor[currentFloor].back();
                            requestsPerFloor[currentFloor].pop_back();
                            continue;
                        }
                        else // If the request is the last one in the vector, simply remove the last element and end the loop.
                        {
                            requestsPerFloor[currentFloor].pop_back();
                            break;
                        }
                    }
                    else if (currentFloor == (*it)->getDestinationFloor() && (*it)->getReachStart())
                    {
                        const unsigned short timeOfRequest = timeTotal - (*it)->getTimeStep();
                        cout << "The elevator " << +ID << " has completed the request " << +(*it)->getStartFloor() << " to " << +(*it)->getDestinationFloor() << " with a time of: " << timeOfRequest << endl;
                        elevatorTime += timeOfRequest;
                        currentWeight -= AVERAGE_WEIGHT;
                        pendingRequests -= 1;                           // Every time the elevator dispatch a request, the counter is updated
                        if (it != requestsPerFloor[currentFloor].end()) //
                        {
                            // Since the requests are stored in an array, it is not efficient to delete the element directly by the subsequent memory ordering.
                            // Therefore, the last element is copied to the current position and the last element is deleted from the array.
                            *it = requestsPerFloor[currentFloor].back();
                            requestsPerFloor[currentFloor].pop_back();
                            continue;
                        }
                        else // If the request is the last one in the vector, simply remove the last element and end the loop.
                        {
                            requestsPerFloor[currentFloor].pop_back();
                            break;
                        }
                    }
                    it++;
                }
            }
            // Updates the floor of the elevator, in case the elevator has reached one limit of requests or floors, changes the direction
            if (pendingRequests != 0)
            {
                if (farestFloorWithRequest() == currentFloor)
                {
                    isGoingUp = !isGoingUp;
                }
                isGoingUp ? currentFloor++ : currentFloor--;
            }
        }
    }
    // Fuction used to calculate the estimate time for a request
    unsigned short estimateTime(Request req)
    {
        const unsigned char requestStartFloor = req.getStartFloor();
        const unsigned char requestDestinationFloor = req.getDestinationFloor();
        if (pendingRequests == 0) // If there is no requests left, simply returns the estimated time in travel
        {
            return abs(currentFloor - requestStartFloor) + abs(requestStartFloor - requestDestinationFloor);
        }
        else
        {
            if (currentWeight <= capacityWeight) // Checks if the elevator has space
            {
                // Checks if the request is on the way of the elevator (up or down). If thats the case, returns the cost plus the stops in the journey
                // Check if the request is going up, the elevator is going up and is on the way
                if (isGoingUp && requestDestinationFloor > requestStartFloor && requestStartFloor >= currentFloor)
                {
                    return requestDestinationFloor - currentFloor + stopsInTheJourney(requestStartFloor, requestDestinationFloor);
                }
                // Check if the request is going down, the elevator is going down and is on the way
                else if (!isGoingUp && requestDestinationFloor < requestStartFloor && requestStartFloor <= currentFloor)
                {
                    return currentFloor - requestDestinationFloor + stopsInTheJourney(requestStartFloor, requestDestinationFloor);
                }
            }
            // If no condition is fulfilled, returns the journey to the farest floor with a request, the return time and the stops.
            const unsigned char farFloor = farestFloorWithRequest();
            const unsigned char firstStops = stopsInTheRoute(currentFloor, farFloor);
            const unsigned char secondStops = stopsInTheRoute(farFloor, requestStartFloor) - firstStops;
            return abs(currentFloor - farFloor) + firstStops + abs(farFloor - requestStartFloor) + secondStops + abs(requestStartFloor - requestDestinationFloor);
        }
    }

    // Method used to assign a request to the elevator
    void assignRequest(Request request)
    {
        auto req = make_shared<Request>(request);
        requestsPerFloor[request.getStartFloor()].emplace_back(req);       // Adds the start floor to the requests
        requestsPerFloor[request.getDestinationFloor()].emplace_back(req); // Adds the destination floor to the requests
        // If the elevator has no requests changes the direction to the new request
        if (pendingRequests == 0)
        {
            if (request.getStartFloor() != currentFloor)
            {
                isGoingUp = request.getStartFloor() > currentFloor;
            }
            else
            {
                isGoingUp = request.getDestinationFloor() > currentFloor;
            }
        }
        pendingRequests += 1;
    }
};

// Represents the array of elevators, contains all the methods to control all elevators at the same time
class Building
{
private:
    array<Elevator, NUM_ELEVATORS> elevators; // Array that contains the elevators in the building

public:
    Building(unsigned char numElevators) // Initialize elevators with unique IDs
    {
        for (unsigned char i = 0; i < numElevators; i++)
        {
            elevators[i] = Elevator(i);
            cout << "The elevator " << +elevators[i].getID() << " has been created" << endl;
        }
    }

    void updateElevators() // Updates the state of all elevators
    {
        for (auto &elevator : elevators)
        {
            // If the elevator has no requests and it's a moment in the morning or afternoon, the elevator goes to an optimal place
            if (elevator.getPendingRequests() == 0)
            {
                if (timeTotal <= int(MAX_TIME * .3) && elevator.getCurrentFloor() != 0)
                    elevator.assignRequest(Request(elevator.getCurrentFloor(), 0, timeTotal));
                else if (timeTotal >= int(MAX_TIME * .7) && elevator.getCurrentFloor() != NUM_FLOORS - 1)
                    elevator.assignRequest(Request(elevator.getCurrentFloor(), NUM_FLOORS - 1, timeTotal));
            }
            elevator.update(); // Update the elevator´s state
        }
    }

    bool pendingRequests() // Checks if any elevator has a pending request
    {
        for (auto &elevator : elevators)
        {
            if (elevator.getPendingRequests() > 0)
                return true;
        }
        return false;
    }

    void elevatorTime() // Checks the total time of the elevator
    {
        for (auto &elevator : elevators)
        {
            cout << "El elevador " << +elevator.getID() << " hizo un tiempo total de: " << elevator.getElevatorTime() << endl;
        }
    }

    // Calculates the best elevator for the request. It uses multithreading for performace
    Elevator *findBestElevator(Request &req)
    {

        // Array to hold the threads
        array<thread, NUM_ELEVATORS> threads;
        // Array to store the results
        array<unsigned int, NUM_ELEVATORS> results;

        // Create threads for each object
        for (unsigned char i = 0; i < NUM_ELEVATORS; ++i)
        {
            threads[i] = thread([this, &results, i, req]()
                                {
                                    results[i] = elevators[i].estimateTime(req); // Call the function and store the result
                                });
        }

        // Join the threads to ensure all are finished
        for (thread &thread : threads)
        {
            thread.join();
        }
        // Compares all the results and returns the best elevator
        Elevator *bestElevator = &elevators[0];
        unsigned short bestCost = results[0];

        for (unsigned char i = 1; i < NUM_ELEVATORS; i++)
        {
            if (results[i] < bestCost)
            {
                bestCost = results[i];
                bestElevator = &elevators[i];
            }
        }

        cout << "The best elevator is: " << +bestElevator->getID() << ". With a cost of: " << bestCost << ". The elevator is in " << +bestElevator->getCurrentFloor() << endl;
        cout << "The request is from: " << +req.getStartFloor() << " to " << +req.getDestinationFloor() << endl;
        return bestElevator;
    }
};

// Function used to read the file with the requests
queue<unsigned char> readFile()
{
    queue<unsigned char> byteQueue;
    // Open the file in binary mode
    ifstream file("requests/requests5.rq", ios::binary);

    // Check if the file was successfully opened
    if (!file)
    {
        cout << "Error opening file!" << endl;
        file.close();
        return byteQueue;
    }

    // Read the file into the queue one byte at a time
    char byte;
    while (file.read(&byte, sizeof(byte)))
    {
        byteQueue.push(byte - 1);
    }

    // Close the file
    file.close();
    return byteQueue;
}

// Recives and print the queue
void printQueue(queue<unsigned char> q)
{
    cout << "Queue elements: ";
    while (!q.empty())
    {
        cout << +q.front() << " ";
        q.pop();
    }
    cout << endl;
}

int main()
{
    queue requests = readFile();
    printQueue(requests);
    const unsigned short numberOfRequests = requests.size() / 2;
    cout << "The number of requests are " << numberOfRequests << endl;
    const unsigned short rate = MAX_TIME / numberOfRequests;
    Building building(NUM_ELEVATORS);
    while (!requests.empty()) // Sends all the requests to the elevators
    {
        building.updateElevators();
        if (timeTotal % rate == 0 && timeTotal >= rate) // A probability of 40% of sending the request
        {
            const unsigned char startFloor = requests.front();
            requests.pop();
            const unsigned char destinationFloor = requests.front();
            requests.pop();
            Request request(startFloor, destinationFloor, timeTotal);
            Elevator *elevator = building.findBestElevator(request);
            elevator->assignRequest(request);
        }
        timeTotal++;
    }
    while (building.pendingRequests()) // Untill the last elevator serves a request, the elevators are updating
    {
        building.updateElevators();
        timeTotal++;
    }
    building.elevatorTime();
    return 0;
}