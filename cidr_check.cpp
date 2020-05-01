//
// Created by cuongbv on 01/05/2020.
//

#include <fstream>
#include <bitset>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>

#define NUM_HASH_FUNCS 4
#define BF_SIZE 57707801
std::bitset<BF_SIZE> bloomFilter;
int numElement = 0;
std::set<int> maskLayer;

template <class Container>
void splitString(const std::string& str, Container& cont, char delim = ' ') {
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

std::vector<int> hashFunc(std::string hashValue) {
    std::vector<int> result;
    result.reserve(NUM_HASH_FUNCS);
    for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
        hashValue += std::to_string(i);
        std::hash<std::string> hasher;
        auto hashed = hasher(hashValue);
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
    std::bitset<32> cidrMask = ipToBitset(contCIDR[1]);
    maskLayer.insert(cidrMask.count());
    for (int i = 0; i < 32 - cidrMask.count(); ++i) {
        stableString.pop_back();
    }
    return stableString;
}

int main(int argc, const char **argv) {
    std::ifstream cidrFile;
    std::string cirdString;
    std::vector<std::string> cidrVector;
    cidrFile.open("./../ipvn.lst");
    if (cidrFile.fail()) {
        std::cerr << "Fail input CIDR file!" << std::endl;
        return 1;
    }
    while (!cidrFile.eof()) {
        std::getline(cidrFile, cirdString);
        if (cirdString.empty()) continue;
        if (cirdString.back() == '\r') cirdString.pop_back();
        cidrVector.push_back(cirdString);
    }
    cidrFile.close();

    for (const std::string& cidr : cidrVector) {
        std::string stableString = cidrToStableString(cidr);
        std::vector<int> hashIndex = hashFunc(stableString);
        for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
            bloomFilter[hashIndex[i]] = true;
        }
        ++numElement;
    }

    for (int layer : maskLayer) {
        std::string ip = ipToBitset("5.10.84.193").to_string();
        for (int i = 0; i < 32 - layer; ++i) {
            ip.pop_back();
        }
        std::vector<int> hashIndex = hashFunc(ip);
        bool isOn = true;
        for (int i = 0; i < NUM_HASH_FUNCS; ++i) {
            if (!bloomFilter[hashIndex[i]]) {
                isOn = false;
                break;
            }
        }
        if (isOn) {
            std::cout << "on" << std::endl;
            break;
        }
    }


    return 0;
}