#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#define PORT 2702
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    // Create a socket: IPv4, TCP
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Define server addr
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

    listen(serverSocket, 5);
    cout << "Server is listening on port " << PORT << endl;

    // Accept a connection
    sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLen);
    cout << "Connect successful" << endl;

    string fileName;
    ifstream fileToSend;

    while (true) {
        cout << "File name or patch: ";
        getline(cin, fileName);

        fileToSend.open(fileName.c_str(), ios::binary);

        if (fileToSend.is_open()) {
            cout << "Find the file: " << fileName << endl;
            break;
        } else {
            cerr << "Error: Cannot find the file!" << endl;
        }
    }

    // Send file data
    char buffer[BUFFER_SIZE];
    while (fileToSend) {
        fileToSend.read(buffer, BUFFER_SIZE);
        streamsize bytesRead = fileToSend.gcount();
        if (bytesRead > 0) {
            send(clientSocket, buffer, bytesRead, 0);
        }
    }

    cout << "Transfer sucessful" << endl;

    fileToSend.close();
    close(clientSocket);
    close(serverSocket);

    return 0;
}