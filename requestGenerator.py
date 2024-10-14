import random
import sys


def normalRequestGenerator(numberOfFloors):
    results = []
    startFloor = random.randint(1, numberOfFloors)
    results.append(startFloor)
    while True:
        destinationFloor = random.randint(1, numberOfFloors)
        if destinationFloor != startFloor:
            results.append(destinationFloor)
            return results


def createRequests(numberOfRequests, numberOfFloors):
    q1 = []
    numberOfRequests = numberOfRequests
    morning = int(numberOfRequests * 0.3)
    day = numberOfRequests - morning * 2
    for i in range(morning):
        if random.randint(1, 10) <= 7:
            q1.append(1)
            q1.append(random.randint(2, numberOfFloors))
        else:
            q1 += normalRequestGenerator(numberOfFloors)
    for i in range(day):
        q1 += normalRequestGenerator(numberOfFloors)
    for i in range(morning):
        if random.randint(1, 10) <= 7:
            first = random.randint(2, numberOfFloors)
            second = random.randint(1, first - 1)
            q1.append(first)
            q1.append(second)
        else:
            q1 += normalRequestGenerator(numberOfFloors)
    return bytes(q1)


requests = createRequests(int(sys.argv[1]), int(sys.argv[2]))
with open("requests/requests6.rq", "wb") as binary_file:
    binary_file.write(requests)
with open("requests/requests6.rq", "rb") as binary_file:
    print(binary_file.read())
