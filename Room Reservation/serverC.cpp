#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <arpa/inet.h>
#include <thread>

#define UDP_PORT 31908
#define SERVER_M_IP "127.0.0.1"
#define SERVER_M_UDP_PORT 34908

using namespace std;
unordered_map<string, string> userCredentials;

void loadCredentials();

void handleUDP(int udp_sock);

int sendAuthenticationToServerM(int udp_sock, const char* message);

int sendNotificationToServerM(int udp_sock);

int runServerC();

char rotateChar(char c, int shift, char start, int count);

string decrypt(const string& input);

int main() {
    runServerC();
    return 0;
}

void loadCredentials() {
    ifstream file("member.txt");
    if (!file.is_open()) {
        cerr << "Failed to open member.txt\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != string::npos) {
            string username = line.substr(0, commaPos);
            string password = line.substr(commaPos + 2);
            userCredentials[username] = password;
        }
    }
    // cout << "Credentials loaded successfully.\n";
    file.close();
}

void handleUDP(int udp_sock) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_len = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            cout << "The Server C received an authentication request from the main server." << endl;

            string message = buffer;
            size_t userPos = message.find("Username: ");
            size_t passPos = message.find("PassWord: ");

            if (userPos != string::npos && passPos != string::npos) {
                string username = message.substr(userPos + 10, passPos - userPos - 11);
                string password = message.substr(passPos + 10);

                string decrptyUsername = decrypt(username);
                auto userIt = userCredentials.find(username);
                if (userIt == userCredentials.end()) {
                    cout << "Username does not exist." << endl;
                    string authenticationMessage = "Authentication:0 Username does not exist for " +decrptyUsername;
                    sendAuthenticationToServerM(udp_sock, authenticationMessage.c_str());
                }
                else if (userIt->second != password) {
                    cout << "Password does not match." << endl;
                    string authenticationMessage = "Authentication:1 Password does not match for " +decrptyUsername;
                    sendAuthenticationToServerM(udp_sock, authenticationMessage.c_str());
                }
                else {
                    cout << "Successful authentication." << endl;
                    string message2 = "Authentication:2 Successful authentication for " + decrptyUsername;
                    sendAuthenticationToServerM(udp_sock, message2.c_str());
                }
            }
        }
    }
}

int sendAuthenticationToServerM(int udp_sock, const char* message) {
    struct sockaddr_in server_m_addr;
    memset(&server_m_addr, 0, sizeof(server_m_addr));
    server_m_addr.sin_family = AF_INET;
    server_m_addr.sin_port = htons(SERVER_M_UDP_PORT);
    server_m_addr.sin_addr.s_addr = inet_addr(SERVER_M_IP);

    if (sendto(udp_sock, message, strlen(message), 0, (struct sockaddr *)&server_m_addr, sizeof(server_m_addr)) < 0) {
        cerr << "Failed to send notification to Server M\n";
        return -1;
    }

    cout << "The Server C finished sending the response to the main server." << endl;
    return 0;
}


int sendNotificationToServerM(int udp_sock) {
    const char* message = "BOOT:ServerC";
    struct sockaddr_in server_m_addr;

    memset(&server_m_addr, 0, sizeof(server_m_addr));
    server_m_addr.sin_family = AF_INET;
    server_m_addr.sin_port = htons(SERVER_M_UDP_PORT);
    server_m_addr.sin_addr.s_addr = inet_addr(SERVER_M_IP);

    if (sendto(udp_sock, message, strlen(message), 0, (struct sockaddr *)&server_m_addr, sizeof(server_m_addr)) < 0) {
        cerr << "Failed to send notification to Server M\n";
        return -1;
    }

    cout << "The Server C has informed the main server." << endl;
    return 0;
}

int runServerC() {
    loadCredentials();
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == -1) {
        cerr << "Failed to create UDP socket\n";
        return -1;
    }

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (::bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        cerr << "Failed to bind UDP socket to port " << UDP_PORT << "\n";
        close(udp_sock);
        return -1;
    }

    cout << "The Server C is up and running using UDP on port " << UDP_PORT << endl;

    if (sendNotificationToServerM(udp_sock) < 0) {
        close(udp_sock);
        return -1;
    }
    // Thread usage from ChatGPT
    thread udpThread(handleUDP, udp_sock);
    udpThread.join();
    return 0;
}


char rotateChar(const char c, const int shift, const char start, const int count) {
    int shifted = (c - start - shift) % count;
    if (shifted < 0) shifted += count;
    return start + shifted;
}


string decrypt(const string& input) {
    string output = input;
    for (size_t i = 0; i < input.length(); ++i) {
        const char c = input[i];
        const int shift = i + 1;
        // Lowercase letters
        if (c >= 'a' && c <= 'z') {
            output[i] = rotateChar(c, shift, 'a', 26);
        }
        // Uppercase letters
        else if (c >= 'A' && c <= 'Z') {
            output[i] = rotateChar(c, shift, 'A', 26);
        }
        // Numbers
        else if (c >= '0' && c <= '9') { // Digits
            output[i] = rotateChar(c, shift, '0', 10);
        }
        // Special characters are not encrypted
        else {
            output[i] = c;
        }
    }
    return output;
}
