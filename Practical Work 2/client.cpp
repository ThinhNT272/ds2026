/* client.cpp */
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;

#define PORT_NO 2702

void error(const string& msg) {
    cerr << msg << ": " << strerror(errno) << endl;
    exit(0);
}

int main(int argc, char *argv[]) {
    // Check arguments
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <Server_IP> <Filename>" << endl;
        exit(0);
    }

    string server_ip = argv[1];
    string filename = argv[2];

    // Open file to read (Binary mode)
    ifstream infile(filename, ios::binary);
    if (!infile.is_open()) {
        error("ERROR: Cannot open file " + filename);
    }

    // Create Socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Get Server Info
    struct hostent *server = gethostbyname(server_ip.c_str());
    if (server == NULL) {
        cerr << "ERROR, no such host" << endl;
        exit(0);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT_NO);

    // 3. Connect
    cout << "Connecting to " << server_ip << "..." << endl;
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    // Construct JSON string
    string json_payload = 
        "{"
        "\"jsonrpc\": \"2.0\", "
        "\"method\": \"upload\", "
        "\"params\": {\"filename\": \"" + filename + "\"}"
        "}";

    cout << "Sending JSON: " << json_payload << endl;
    write(sockfd, json_payload.c_str(), json_payload.length());
    
    vector<char> buffer(1024);
    int n = read(sockfd, buffer.data(), buffer.size());
    if (n < 0) error("ERROR reading from socket");

    string response(buffer.data(), n);
    cout << "Server Response: " << response << endl;

    if (response.find("\"result\": \"OK\"") == string::npos) {
        error("Server refused connection or returned error");
    }

    cout << "Server accepted. Sending file content..." << endl;
    
    // Read chunks from file and send
    while (!infile.eof()) {
        infile.read(buffer.data(), buffer.size());
        streamsize bytes_read = infile.gcount();
        
        if (bytes_read > 0) {
            write(sockfd, buffer.data(), bytes_read);
        }
    }

    cout << "Done! File sent successfully." << endl;
    
    // Close socket
    infile.close();
    close(sockfd);
    return 0;
}