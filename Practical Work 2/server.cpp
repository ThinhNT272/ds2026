/* server.cpp */
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

#define PORT_NO 2702

// Helper function to print error and exit
void error(const string& msg) {
    cerr << msg << ": " << strerror(errno) << endl;
    exit(1);
}

// Simple manual JSON parser
// Extracts value of a specific key from a JSON string
string parse_json_value(const string& json, const string& key) {
    string search_key = "\"" + key + "\":";
    size_t pos = json.find(search_key);
    if (pos == string::npos) return "";

    size_t start_quote = json.find("\"", pos + search_key.length());
    if (start_quote == string::npos) return "";

    size_t end_quote = json.find("\"", start_quote + 1);
    if (end_quote == string::npos) return "";

    return json.substr(start_quote + 1, end_quote - start_quote - 1);
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    
    vector<char> buffer(1024);

    //Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Initialize socket structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NO);

    // Bind
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // Listen
    listen(sockfd, 5);
    cout << "Server (C++ JSON-RPC) running on port " << PORT_NO << "..." << endl;

    // Accept connection
    cli_len = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (newsockfd < 0) error("ERROR on accept");

    // Read JSON Command
    fill(buffer.begin(), buffer.end(), 0);
    int n = read(newsockfd, buffer.data(), buffer.size() - 1);
    if (n < 0) error("ERROR reading from socket");

    string json_str(buffer.data());
    cout << "Received Command: " << json_str << endl;

    // Parse JSON
    string method = parse_json_value(json_str, "method");
    
    if (method == "upload") {
        string filename = parse_json_value(json_str, "filename");
        if (filename.empty()) {
            cerr << "Error: Filename not found in JSON!" << endl;
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        cout << "--> Preparing to receive file: " << filename << endl;

        string response = "{\"result\": \"OK\"}";
        write(newsockfd, response.c_str(), response.length());

        // Receive Binary Data
        ofstream outfile(filename, ios::binary);
        if (!outfile.is_open()) error("ERROR creating file");

        int total_bytes = 0;
        while (true) {
            int bytes_received = read(newsockfd, buffer.data(), buffer.size());
            if (bytes_received <= 0) break; 

            outfile.write(buffer.data(), bytes_received);
            total_bytes += bytes_received;
        }

        cout << "SUCCESS! Saved " << total_bytes << " bytes to " << filename << endl;
        outfile.close();

    } else {
        cerr << "Error: Unsupported method!" << endl;
    }

    // Close socket
    close(newsockfd);
    close(sockfd);
    return 0;
}