#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <thread>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <sstream>
#include<fstream>

using namespace std;

string make_request(string host, string path) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;
    if ((status = getaddrinfo(host.c_str(), "80", &hints, &res)) != 0)
    {
        cerr << "getaddrinfo: " << host << " " << gai_strerror(status) << endl;
        exit(-1);
    }
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("Cannot create socket");
        exit(-1);
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Cannot Connect");
        exit(-1);
    }
    
    stringstream request;
    request << "GET " << path << " HTTP/1.0\r\n";
    request << "Host:  " << host << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    string req_str = request.str();

    int send_stat = send(sockfd, req_str.c_str(), req_str.size(), 0);
    if (send_stat == -1) {
        cout << "Send Error!" << endl;
    }

    stringstream response;
    char buf[256];
    int received_bytes = 0;
    while((received_bytes = recv(sockfd, buf, 256, 0)) != 0) {
        response.write(buf, received_bytes);
    }

    // cout << response.str() << endl;
    return response.str();
}

int main(int argc, char** argv) {

    string hostname = "";
    string filename = "index.html";

    if (argv[1] != NULL) {
        hostname = argv[1];
    }

    if (argv[2] != NULL) {
        filename = argv[2];
    }
	
	string portnum = "";
	bool file = false;
	int opt;

	// get argument


    // if http was found
    size_t http_found = hostname.find("http://", 0);
    if (http_found != string::npos) {
        // erase
        hostname.erase(0, 7); 

        // set port number
        portnum = "80";
    }

    // if https was found
    size_t https_found = hostname.find("https://", 0);
    if (https_found != string::npos) {
        // erase
        hostname.erase(0, 8);

        // set port number
        portnum = "443";
    }

    string path = "/";

    // remove paths if any and save as a string
    size_t path_found = hostname.find("/", 0);
    // if there is a path
    if (path_found != string::npos) {
        
        // remove and store the path
        int length = hostname.length() - path_found;
        path = hostname.substr(path_found, length);
        hostname.erase(path_found, length);

        // update file name if not specified
        if (file == false) {
            // find the name for the file
            size_t newFileNamePos = path.rfind("/");

            if (newFileNamePos != path.size() - 1) { // if path is not empty
                int newFileNameLength = path.length() - newFileNamePos;
                string newFileName = path.substr(newFileNamePos + 1, newFileNameLength);

                // change filename
                filename = newFileName;
            }
        }
    }

    // send request and store response
    string response = make_request(hostname, path);

    // get status code

    stringstream ss(response); // open file
    string statusCode;
    ss >> statusCode >> statusCode;

    cout << "Status code: " << statusCode << endl;

    string newHostname;
    // get new location if 300 code
    if(stoi(statusCode) >= 300 && stoi(statusCode) < 400) {
        // get the third line
        
        for (int i = 0; i < 3; i++) {
            getline(ss, newHostname);
        }

        // erase "location: "
        newHostname.erase(0, 10);
    }

    // check if chunked
    bool chunked = false;
    if (response.find("chunked") != string::npos) {
        chunked = true;
    }


    // remove header
    size_t headerIndex = response.find("\r\n\r\n");

    response.erase(0, headerIndex+4);

    // write string to file
    ofstream out(filename);
    out << response;
    out.close();

    cout << "File transfer complete." << endl;


    // handle redirections
    if ((stoi(statusCode) == 301) || (stoi(statusCode) == 302)) {

        // check to see if the final char is a "/"
        if ((newHostname.at(newHostname.size() - 2)) == '/') {
            // remove the last part
            newHostname = newHostname.substr(0, newHostname.size() - 2);
        }

        cout << "Redirecting to " << newHostname << endl;

        // if http was found
        size_t http_found = newHostname.find("http://", 0);
        if (http_found != string::npos) {
            // erase
            newHostname.erase(0, 7); 

            // set port number
            portnum = "80";
        }

        // if https was found
        size_t https_found = newHostname.find("https://", 0);
        if (https_found != string::npos) {
            // erase
            newHostname.erase(0, 8);

            // set port number
            portnum = "443";
        }

        string path = "/";

        // remove paths if any and save as a string
        size_t path_found = newHostname.find("/", 0);
        // if there is a path
        if (path_found != string::npos) {
            
            // remove and store the path
            int length = newHostname.length() - path_found;
            path = newHostname.substr(path_found, length);
            newHostname.erase(path_found, length);

            // update file name if not specified
            if (file == false) {
                // find the name for the file
                size_t newFileNamePos = path.rfind("/");

                if (newFileNamePos != path.size() - 1) { // if path is not empty
                    int newFileNameLength = path.length() - newFileNamePos;
                    string newFileName = path.substr(newFileNamePos + 1, newFileNameLength);

                    // change filename
                    filename = newFileName;
                }
            }
        }


        string newResponse = make_request(newHostname, path);

        // remove header
        size_t newHeaderIndex = newResponse.find("\r\n\r\n");

        newResponse.erase(0, newHeaderIndex+4);

        // write string to file
        ofstream out(filename);
        out << newResponse;
        out.close();

        // file transfer complete
        cout << "Redirected file transfer complete." << endl;
    }

    // handle error code
    if ((stoi(statusCode) >= 400)) {
        cout << "Error: " << statusCode << endl;
        return 1;
    }

    // if line 10 has chunked
    if (chunked) {
        cout << "Tranfer Encoding." << endl;

        // remove header
        size_t rnIndex;

        string decoded = "";

        rnIndex = response.find("\r\n");

        // erase (hex + \r\n)
        response.erase(0, rnIndex + 2);

        int body = 2;

        while ((rnIndex = response.find("\r\n")) != string::npos) {

            // store 0->rnIndex as a token
            string token = response.substr(0, rnIndex);

            // add token to decoder if it is a body
            if (body % 2 == 0) {
                decoded += token;
            }

            // delete body and \r\n
            response.erase(0, rnIndex+2);

            body++;

        }

        // update file
        remove(filename.c_str());
        ofstream ofs(filename);
        ofs << decoded;
        ofs.close();
    }


}