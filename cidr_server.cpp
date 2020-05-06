//
// Created by cuongbv on 02/05/2020.
//

#include <bitset>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <map>

#define PORT 6596
#define NUM_HASH_FUNCS 4
#define BF_SIZE 57707801
std::bitset<BF_SIZE> bloomFilter;
int numElement = 0;
bool startState = false;
std::set<int> maskLayer;
std::map<std::string, std::pair<std::string, std::string> > stableCIDRMap;

template <class Container>
void splitString(const std::string& str, Container& cont, char delim = ' ') {
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

std::vector<int> hashFunc(std::string value) {
    std::vector<int> result;
    result.reserve(NUM_HASH_FUNCS);
    for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
        value += std::to_string(i);
        std::hash<std::string> hasher;
        auto hashed = hasher(value);
        result[i] = (int) (hashed % BF_SIZE);
    }
    return result;
}

std::bitset<32> ipToBitset(const std::string& ip) {
    std::vector<std::string> ipVector;
    splitString(ip, ipVector, '.');
    std::string ipBitString = std::bitset<8>(atoi(ipVector[0].c_str())).to_string()
                              + std::bitset<8>(atoi(ipVector[1].c_str())).to_string()
                              + std::bitset<8>(atoi(ipVector[2].c_str())).to_string()
                              + std::bitset<8>(atoi(ipVector[3].c_str())).to_string();
    return std::bitset<32>(ipBitString);
}

std::string cidrToStableString(const std::string& cidr) {
    std::vector<std::string> contCIDR;
    splitString(cidr, contCIDR, '/');
    if (contCIDR.size() != 2) {
        std::cerr << "Invalid CIDR!" << std::endl;
    }
    std::bitset<32> cidrNetwork = ipToBitset(contCIDR[0]);
    std::string stableString = cidrNetwork.to_string();
    std::bitset<32> netMask = ipToBitset(contCIDR[1]);
    maskLayer.insert(netMask.count());
    for (int i = 0; i < 32 - netMask.count(); ++i) {
        stableString.pop_back();
    }
    return stableString;
}

std::string showUsage() {
    std::stringstream stringstream;
    stringstream << "|*******************************************************|" << std::endl;
    stringstream << "|****************** Bloom Filter CIDR ******************|" << std::endl;
    stringstream << "|*******************************************************|" << std::endl;
    stringstream << "| Show Bloom filter info: $ ./cidr_client info          |" << std::endl;
    stringstream << "| Load new input file: $ ./cidr_client load <path>      |" << std::endl;
    stringstream << "| Add new value: $ ./cidr_client add <cidr> <location>  |" << std::endl;
    stringstream << "| Load check file: $ ./cidr_client check_file <path>    |" << std::endl;
    stringstream << "| Check new value: $ ./cidr_client check <ip_addr>      |" << std::endl;
    stringstream << "| Reset Bloom filter: $ ./cidr_client reset             |" << std::endl;
    stringstream << "|*******************************************************|";
    startState = false;
    return stringstream.str();
}

std::string showBFInfo() {
    std::stringstream stringstream;
    stringstream << "Bloom filter of CIDR information:" << std::endl;
    stringstream << "Number of hash functions: " << NUM_HASH_FUNCS << std::endl;
    stringstream << "Size of the Bloom filter: " << BF_SIZE << std::endl;
    stringstream << "Maximum storage capacity CIDR value: " << (int) (BF_SIZE / (NUM_HASH_FUNCS /  log(2))) << std::endl;
    stringstream << "False positive probability: " << (int)(pow(2, -NUM_HASH_FUNCS) * 100) << "%" << std::endl;
    stringstream << "Current number of CIDR values: " << numElement;
    startState = false;
    return stringstream.str();
}

std::string loadInputFile(const std::string& inputFilePath) {
    std::stringstream stringstream;
    std::ifstream cidrFile;
    std::string cird_locationString;
    std::vector<std::string> cidr_locationVector;
    cidrFile.open(inputFilePath);
    if (cidrFile.fail()) {
        stringstream << "Fail input CIDR file!";
        return stringstream.str();
    }
    while (!cidrFile.eof()) {
        std::getline(cidrFile, cird_locationString);
        if (cird_locationString.empty()) continue;
        if (cird_locationString.front() == '#') continue;
        if (cird_locationString.back() == '\r') cird_locationString.pop_back();
        cidr_locationVector.push_back(cird_locationString);
    }
    if (cidr_locationVector.empty()) {
        stringstream << "The input CIDR file is empty!";
        return stringstream.str();
    }
    cidrFile.close();

    for (const std::string& cidr_location : cidr_locationVector) {
        std::vector<std::string> cidr_locationContainer;
        splitString(cidr_location, cidr_locationContainer, ',');
        std::string cidr(cidr_locationContainer[0]);
        std::string location (cidr_locationContainer[1]);

        std::string stableString = cidrToStableString(cidr);
        std::vector<int> hashIndex = hashFunc(stableString);
        for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
            bloomFilter[hashIndex[i]] = true;
        }
        std::pair<std::string, std::string> cidr_locationPair = std::pair<std::string, std::string>(cidr, location);
        stableCIDRMap.insert(std::pair<std::string, std::pair<std::string, std::string> >(stableString, cidr_locationPair));

        ++numElement;
    }
    stringstream << "Complete load the new input file \"" << inputFilePath << "\" into the Bloom filter";
    startState = false;
    return stringstream.str();
}

std::string addValue(const std::string& inputCIDR, const std::string& cidrLocation) {
    std::stringstream stringstream;
    std::string stableString = cidrToStableString(inputCIDR);
    std::vector<int> hashIndex = hashFunc(stableString);
    for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
        bloomFilter[hashIndex[i]] = true;
    }
    std::pair<std::string, std::string> cidr_location = std::pair<std::string, std::string>(inputCIDR, cidrLocation);
    stableCIDRMap.insert(std::pair<std::string, std::pair<std::string, std::string> >(stableString, cidr_location));
    stringstream << "Complete add \"" << inputCIDR << "\" to the Bloom filter";
    startState = false;
    ++numElement;
    return stringstream.str();
}

std::string loadCheckFile(const std::string& checkFilePath) {
    std::stringstream stringstream;
    std::ifstream ipCheckFile;
    std::vector<std::string> ipCheckVector;
    std::string ipAddrString;
    // Load all of values in checking file into memory
    ipCheckFile.open(checkFilePath, std::ios::in);
    while (!ipCheckFile.eof()) {
        std::getline(ipCheckFile, ipAddrString);
        if (ipAddrString.empty()) continue;
        if (ipAddrString.back() == '\r') ipAddrString.pop_back();
        ipCheckVector.push_back(ipAddrString);
    }
    if (ipCheckVector.empty()) {
        stringstream << "The check file is empty!";
        return stringstream.str();
    }
    ipCheckFile.close();

    // Check each value with the Bloom filter
    for (const std::string& ipAddress : ipCheckVector) {
        for (int layer : maskLayer) {
            std::string ipBitString = ipToBitset(ipAddress).to_string();
            for (int i = 0; i < 32 - layer; ++i) ipBitString.pop_back();
            std::vector<int> hashIndex = hashFunc(ipBitString);
            bool isOn = true;
            for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
                if (!bloomFilter[hashIndex[i]]) {
                    isOn = false;
                    break;
                }
            }
            if (isOn) {
                if (stableCIDRMap.find(ipBitString) == stableCIDRMap.end()) {
                    stringstream << "False positive" << std::endl;
                    continue;
                }
                stringstream << "on " << stableCIDRMap[ipBitString].first << " of " << stableCIDRMap[ipBitString].second << std::endl;
                break;
            }
        }
    }
    stringstream << "Complete check values in \"" << checkFilePath << "\" with the Bloom filter";
    startState = false;
    return stringstream.str();
}

std::string checkValue(const std::string& ipAddress) {
    std::stringstream stringstream;
    for (int layer : maskLayer) {
        std::string ipBitString = ipToBitset(ipAddress).to_string();
        for (int i = 0; i < 32 - layer; ++i) {
            ipBitString.pop_back();
        }
        std::vector<int> hashIndex = hashFunc(ipBitString);
        bool isOn = true;
        for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
            if (!bloomFilter[hashIndex[i]]) {
                isOn = false;
                break;
            }
        }
        if (isOn) {
            if (stableCIDRMap.find(ipBitString) == stableCIDRMap.end()) {
                stringstream << "False positive" << std::endl;
                continue;
            }
            stringstream << "on " << stableCIDRMap[ipBitString].first << " of " << stableCIDRMap[ipBitString].second << std::endl;
            return stringstream.str();
        }
    }
    stringstream << "not on";
    return stringstream.str();
}

std::string restartBF() {
    std::stringstream stringstream;
    startState = true;
    bloomFilter.reset();
    stableCIDRMap.clear();
    stringstream << "The Bloom filter has been reset!";
    numElement = 0;
    return stringstream.str();
}

int main(int argc, char const *argv[]) {
    int socketServer;       // The socket file description of the socket server side
    int incomingClient;     // The socket file description of a socket client side
    int readStatus;         // status when read request from a Socket Client
    struct sockaddr_in sockServerAddr{};        // The structure of a Socket Server address
    int optionName = 1;     // OPTNAME param of setsockopt() call
    int addrLen = sizeof(sockServerAddr);

    // Creating the Socket Server
    if ((socketServer = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fail to create a Socket Server\n");
        exit(EXIT_FAILURE);
    }

    // Bind the Socket Server address with necessary parameter
    if (setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optionName, sizeof(optionName))) {
        perror("ERROR at Set Socket Server Option name\n");
        exit(EXIT_FAILURE);
    }
    sockServerAddr.sin_family = AF_INET;        // IPv4
    sockServerAddr.sin_addr.s_addr = INADDR_ANY;        // the loopback address
    sockServerAddr.sin_port = htons(PORT);      // Encode port value
    // Forcefully attaching socket to the port value on Device
    if (bind(socketServer, (struct sockaddr *)&sockServerAddr, sizeof(sockServerAddr)) < 0) {
        perror("Fail to Bind socket to the TCP port!\n");
        exit(EXIT_FAILURE);
    }

    // The infinity loop. The Socket Server will listen connect forever
    while (true) {
        char receiveBuffer[1024] = {0};     // The buffer which will contain a request command from a Socket Client

        // Listen the incoming request on assigned port
        if (listen(socketServer, 3) < 0) {
            std::cerr << "Could not listen on assign port " << PORT << std::endl;
            exit(EXIT_FAILURE);
        }

        // Intecept the incoming request from Socket Client side
        if ((incomingClient = accept(socketServer, (struct sockaddr *) &sockServerAddr, (socklen_t *) &addrLen)) < 0) {
            std::cerr << "Could not accept request from a socket client side" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Read and store the incoming request command from the Socket Client side
        readStatus = read(incomingClient, receiveBuffer, 1024);
        if (readStatus < 0) {
            std::cerr << "Fail to read request command from Socket client!" << std::endl;
            return 1;
        }

        printf("From client: %s\n", receiveBuffer);

        // Handle input command from Socket Client
        std::vector<std::string> requestCommandVector;
        std::string passingCommand (receiveBuffer);
        splitString(passingCommand, requestCommandVector);

        if (requestCommandVector[0] == "info") {
            std::string passingString = showBFInfo();
            send(incomingClient, passingString.c_str(), strlen(passingString.c_str()), 0);
            continue;
        }

        if (requestCommandVector[0] == "load") {
            if (requestCommandVector.size() > 1) {
                std::string response = loadInputFile(requestCommandVector[1]);
                send(incomingClient, response.c_str(), strlen(response.c_str()), 0);
                continue;
            }
            send(incomingClient, showUsage().c_str(), strlen(showUsage().c_str()), 0);
            continue;
        }

        if (requestCommandVector[0] == "add") {
            if (requestCommandVector.size() > 2) {
                std::string response = addValue(requestCommandVector[1], requestCommandVector[2]);
                send(incomingClient, response.c_str(), strlen(response.c_str()), 0);
                continue;
            }
            send(incomingClient, showUsage().c_str(), strlen(showUsage().c_str()), 0);
            continue;
        }

        if (requestCommandVector[0] == "check_file") {
            if (requestCommandVector.size() > 1) {
                std::string response = loadCheckFile(requestCommandVector[1]);
                send(incomingClient, response.c_str(), strlen(response.c_str()), 0);
                continue;
            }
            send(incomingClient, showUsage().c_str(), strlen(showUsage().c_str()), 0);
            continue;
        }

        if (requestCommandVector[0] == "check") {
            if (requestCommandVector.size() > 1) {
                std::string response = checkValue(requestCommandVector[1]);
                send(incomingClient, response.c_str(), strlen(response.c_str()), 0);
                continue;
            }
            send(incomingClient, showUsage().c_str(), strlen(showUsage().c_str()), 0);
            continue;
        }

        if (requestCommandVector[0] == "reset") {
            std::string response = restartBF();
            send(incomingClient, response.c_str(), strlen(response.c_str()), 0);
            continue;
        }

        // Default, show guide panel
        send(incomingClient, showUsage().c_str(), strlen(showUsage().c_str()), 0);
    }
    close(socketServer);
    return 0;
}
