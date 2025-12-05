#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#define PORT 2702
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    // Create a socket: IPv4, TCP
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Define server addr
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    // Connect to Server
    cout << "Connecting to server" << endl;
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    cout << "Connect successful" << endl;

    string saveFileName;
    cout << "Enter file name: ";
    getline(cin, saveFileName);

    ofstream receivedFile(saveFileName.c_str(), ios::binary);

    // Receive file data
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    long totalBytes = 0;

    cout << "Receiving" << endl;
    
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        receivedFile.write(buffer, bytesReceived);
        totalBytes += bytesReceived;
    }

    cout << "Received" << endl;

    close(clientSocket);
    receivedFile.close();

    return 0;
}