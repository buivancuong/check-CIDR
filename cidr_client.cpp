//
// Created by cuongbv on 02/05/2020.
//

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

#define PORT 6596

/** The main program run as a Socket Client (C)
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char const *argv[]) {
    int socketClient;       // The socket file description of the socket client side
    int readStatus;     // status when read response from a Socket Server
    struct sockaddr_in sockServerAddr{};  // The structure of a Socket Server address
    if (argc < 2) {
        std::cerr << "\"$ ./cidr_client help\" for usage." << std::endl;
        return 1;
    }
    // Handle command line arguments
    std::string command (argv[1]);      // The command will impact with a Socket Server side
    std::string param;      // The parameter will pass to a Socket Server side
    if (argc == 3) param = argv[2];
    else param = "";
    std::string fullCommand (command + " " + param);
    char *passingCommand = const_cast<char *>(fullCommand.c_str());

    char receiveBuffer[1024] = {0};     // The buffer which will contain a response from a Socket Server
    // Create the Socket Client
    if ((socketClient = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return 1;
    }

    // Assign parameters into Socket Server address structure
    sockServerAddr.sin_family = AF_INET;        // IPv4
    sockServerAddr.sin_port = htons(PORT);      // Encode port value
    if(inet_pton(AF_INET, "127.0.0.1", &sockServerAddr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 1;
    }

    // Construct a connection to a Socket Server side
    if (connect(socketClient, (struct sockaddr *)&sockServerAddr, sizeof(sockServerAddr)) < 0) {
        printf("\nConnection Failed \n");
        return 1;
    }

    // Send the request command to Socket Server side
    send(socketClient , passingCommand , strlen(passingCommand) , 0 );

    // Read and store the response from the Socket Server side
    readStatus = read(socketClient , receiveBuffer, 1024);
    if (readStatus < 0) {
        printf("Fail to read response from Socket server!\n");
        return 1;
    }

    // Print the response out to console
    printf("%s\n", receiveBuffer);

    // Close socket and return
    close(socketClient);
    return 0;
}