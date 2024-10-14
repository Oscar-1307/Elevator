#include <iostream>
#include <queue>
#include <vector>
#include <array>
#include <algorithm>
#include <queue>   //Structure for the queue of requests
#include <fstream> //Library used to read the file of requests

using namespace std;

const unsigned short MAX_TIME = 500;
unsigned int timeTotal = 0;

class TraditionalElevator
{
private:
    int currentFloor;     // Current position of the elevator
    int totalFloors;      // Total number of floors
    vector<int> requests; // A list to hold floor requests
    bool goingUp;         // Direction flag

public:
    unsigned int elevatorTime;
    TraditionalElevator() : currentFloor(0), totalFloors(0), goingUp(true), elevatorTime(0) {}
    TraditionalElevator(int floors) : currentFloor(0), totalFloors(floors), goingUp(true), elevatorTime(0) {}

    // Simulate pressing a floor button in the elevator
    void requestFloor(int floor)
    {
        if (floor >= 0 && floor < totalFloors)
        {
            requests.push_back(floor);
        }
    }

    // Move the elevator sequentially based on floor requests
    void moveElevator()
    {
        while (!requests.empty())
        {
            if (goingUp)
            {
                // Serve all floors above currentFloor
                for (int floor = currentFloor; floor < totalFloors; ++floor)
                {
                    if (serveFloor(floor))
                    {
                        currentFloor = floor;
                    }
                    elevatorTime++;
                }
                goingUp = false; // After going up, we switch direction to down
            }

            if (!goingUp)
            {
                // Serve all floors below currentFloor
                for (int floor = currentFloor - 1; floor >= 0; --floor)
                {
                    if (serveFloor(floor))
                    {
                        currentFloor = floor;
                    }
                    elevatorTime++;
                }
                goingUp = true; // After going down, we switch direction to up
            }
        }
    }

private:
    // Serve a particular floor if it was requested, and remove from the request list
    bool serveFloor(int floor)
    {
        auto it = find(requests.begin(), requests.end(), floor);
        if (it != requests.end())
        {
            cout << "Stopping at floor: " << floor << " at time " << timeTotal << endl;
            requests.erase(it); // Remove served floor
            return true;
        }
        return false;
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
    array<TraditionalElevator, 3> elevators;
    elevators[0] = TraditionalElevator(20);
    elevators[1] = TraditionalElevator(20);
    elevators[2] = TraditionalElevator(20);
    unsigned char numElevator = 0;
    queue requests = readFile();
    printQueue(requests);
    const unsigned short numberOfRequests = requests.size() / 2;
    cout << "The number of requests are " << numberOfRequests << endl;
    const unsigned short rate = MAX_TIME / numberOfRequests;
    while (!requests.empty()) // Sends all the requests to the elevators
    {
        if (timeTotal % rate == 0 && timeTotal >= rate)
        {
            const unsigned char startFloor = requests.front();
            requests.pop();
            const unsigned char destinationFloor = requests.front();
            requests.pop();
            elevators[numElevator].requestFloor(startFloor);
            elevators[numElevator].requestFloor(destinationFloor);
            elevators[numElevator].moveElevator();
            numElevator < 2 ? numElevator++ : numElevator = 0;
        }
        timeTotal++;
    }
    numElevator = 0;
    for (auto &elevator : elevators)
    {
        cout << "El elevador " << +numElevator << " hizo un tiempo total de: " << elevator.elevatorTime << endl;
        numElevator++;
    }
    return 0;
}
