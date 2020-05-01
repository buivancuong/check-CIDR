#include <iostream>
#include <sstream>
#include <vector>

template <class Container>
void splitString(const std::string& str, Container& cont, char delim = ' ') {
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}


uint32_t IPToUInt(const std::string& ip) {
    int a, b, c, d;
    uint32_t addr = 0;

    if (sscanf(ip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
        return 0;

    addr = a << 24;
    addr |= b << 16;
    addr |= c << 8;
    addr |= d;
    return addr;
}

bool IsIPInRange(const std::string& ip, const std::string& network, const std::string& mask) {
    uint32_t ip_addr = IPToUInt(ip);
    uint32_t network_addr = IPToUInt(network);
    uint32_t mask_addr = IPToUInt(mask);

    uint32_t net_lower = (network_addr & mask_addr);
    uint32_t net_upper = (net_lower | (~mask_addr));

    return ip_addr >= net_lower && ip_addr <= net_upper;
}

int main (int argc, const char **argv) {
    std::string ip (argv[1]);
    std::string net_mask (argv[2]);
    std::vector<std::string> net_maskVector;
    splitString(net_mask, net_maskVector, '/');
    if (net_maskVector.size() != 2) {
        std::cerr << "Invalid input Network/Mask!" << std::endl;
        return 1;
    }
    if (IsIPInRange(ip, net_maskVector[0], net_maskVector[1])) std::cout << "in" << std::endl;
    else std::cout << "not in" << std::endl;
    return 0;
}
