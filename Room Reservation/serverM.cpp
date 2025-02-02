#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <map>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <cctype>

using namespace std;

#define BUFFER_SIZE 1024
#define TCP_PORT 35908
#define UDP_PORT 34908
#define DEST_UDP_IP "127.0.0.1"
#define SERVERC_UDP_PORT 31908
#define SERVEREEB_UDP_PORT 33908
#define SERVERRTH_UDP_PORT 32908

map<string, int> sessionToTcpSocketMap;

void sendEncryptedCredentials(int udp_sock, const string& message, const string& username);

string extractUsername(const string& message);

void handleTCP(int client_sock, int udp_sock);

void handleUDP(int udp_sock);

void handleBootNotification(const string& message);

void handleRoomRequestResult(int client_sock, const string& response);

void handleGuestRoomReservationRequest(int client_sock, const string& response);

void handleGuestAuthenticationRequest(int client_sock, const string& userName);

string handleCredential(const string& message);

string encryptCredential(const string& message);

char rotateChar(char c, int shift, char start, int count);

string encrypt(const string& input);

string getUserNameFromServerC (const string& message);

int checkAuthenticationResult(const string& message);

void printMap(const map<string, int>& sessionToTcpSocketMap);

void handleServerBootup(const string& message);

void handleRoomRequest(const string& message);

void transformToLower(string& s);

void transformToUpper(string& s);

void sendRoomRequest(int udp_sock, string& message);

void handleGuestRoomRequest(const string& message);

bool isMember(const string& data);

bool isGuest(const string& data);

bool isReservationRequest(const string& data);

int main() {
    int tcp_sock, udp_sock;
    struct sockaddr_in tcp_addr, udp_addr;

    // TCP socket
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock == -1) {
        cerr << "Failed to create TCP socket\n";
        return -1;
    }

    // UDP socket
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == -1) {
        cerr << "Failed to create UDP socket\n";
        return -1;
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(TCP_PORT);

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    int TCPResult = ::bind(tcp_sock, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
    if (TCPResult < 0) {
        cerr << "Failed to bind TCP socket\n";
        close(tcp_sock);
        return -1;
    }

    int UDPResult = ::bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr));
    if (UDPResult < 0) {
        cerr << "Failed to bind UDP socket\n";
        close(udp_sock);
        return -1;
    }

    // TCP listen
    if (listen(tcp_sock, 5) < 0) {
        cerr << "Failed to listen on TCP socket\n";
        return -1;
    }

    cout << "The main server is up and running.\n";

    // thread usage from ChatGPT
    thread udpThread(handleUDP, udp_sock);
    udpThread.detach();

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(tcp_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            cerr << "Error accepting TCP connection\n";
            continue;
        }
        cout << "TCP connection established\n";

        thread(handleTCP, client_sock, udp_sock).detach();
        printMap(sessionToTcpSocketMap);
    }

    close(tcp_sock);
    close(udp_sock);

    return 0;
}

void handleTCP(int client_sock, int udp_sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            string receivedData(buffer);
            string username;
            // Handle Credential from client
            if (receivedData.find("Credential:") == 0) {
                username = extractUsername(receivedData);
                string encryptUsername = encrypt(username);
                sessionToTcpSocketMap[username] = client_sock;
                cout << "The main server received the authentication for " << encryptUsername << " using TCP over port " <<TCP_PORT <<".\n";
                // Member authentication
                if (!receivedData.empty() && receivedData.back() != ':') {
                    string encryptedCredentials = handleCredential(receivedData);
                    sendEncryptedCredentials(udp_sock, encryptedCredentials, encryptUsername);
                }
                // Guest authentication
                else {
                    handleGuestAuthenticationRequest(client_sock,username);
                }
            }
            // Handle Room request
            else if (receivedData.find("Room:") == 0) {
                if (isMember(receivedData)) {
                    handleRoomRequest(receivedData);
                    sendRoomRequest(udp_sock, receivedData);
                }
                else if (isGuest(receivedData) && !isReservationRequest(receivedData)) {
                    handleGuestRoomRequest(receivedData);
                    sendRoomRequest(udp_sock, receivedData);
                }
                // Guest cannot make reservation
                else {
                    string message = "Permission denied. " + username + " cannot make a reservation.";
                    cout << message << endl;
                    handleGuestRoomReservationRequest(client_sock, message);
                }
            }
        }
        else if (recv_len == 0) {
            cout << "TCP connection closed by client\n";
            break;
        }
        else {
            cerr << "Error receiving data from TCP client: " << strerror(errno) << endl;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            else {
                break;
            }
        }
    }

}


string extractUsername(const string& message) {
    const size_t passwWordPos = message.find("Password:");
    const size_t userNamePos = message.find("Username:");
    string userName = message.substr(userNamePos+9,passwWordPos-userNamePos-10);
    return userName;
}

int checkAuthenticationResult(const string& message) {
    const string statusCode = message.substr(15, 1);
    if(statusCode == "2") {
        return 2;
    }
    if(statusCode == "1") {
        return 1;
    }
    if(statusCode == "0") {
        return 0;
    }
    return -1;
}

void printMap(const map<string, int>& sessionToTcpSocketMap) {
    for (const auto& pair : sessionToTcpSocketMap) {
        cout << "Session/User: " << pair.first << " => Socket Descriptor: " << pair.second << endl;
    }
}

void transformToLower(string& s) {
    for (char& c : s) {
        if (isalpha(c)) {
            c = tolower(c);
        }
    }
}

void transformToUpper(string& s) {
    for (char& c : s) {
        if (isalpha(c)) {
            c = toupper(c);
        }
    }
}

void handleRoomRequest(const string& message) {
    size_t posRoom = message.find("Room:");
    size_t posDay = message.find("Day:");
    size_t posTime = message.find("Time:");
    size_t posOption = message.find("Option:");
    size_t posUsername = message.find("Username:");
    size_t posUsernameEnd = message.find('!');

    string roomNumber;
    roomNumber = message.substr(posRoom + 5, posDay - (posRoom + 6));
    transformToUpper(roomNumber);

    string day;
    day = message.substr(posDay + 4, posTime - (posDay + 5));

    string time;
    time = message.substr(posTime + 5, posOption - (posTime + 6));

    string option;
    option = message.substr(posOption + 7, posUsername - (posOption + 8));
    transformToLower(option);

    string Username;
    Username = message.substr((posUsername)+9,posUsernameEnd - (posUsername + 10));
    string encryptUserName = encrypt(Username);

    cout << "The main server has received the " << option <<" request on Room "<< roomNumber <<" at " << time << " on " << day << " from " << encryptUserName << " using TCP over port " << TCP_PORT << endl;
}

void handleGuestRoomRequest(const string& message) {
    size_t posRoom = message.find("Room:");
    size_t posDay = message.find("Day:");
    size_t posTime = message.find("Time:");
    size_t posOption = message.find("Option:");
    size_t posUsername = message.find("Username:");
    size_t posUsernameEnd = message.find('!');

    string roomNumber;
    roomNumber = message.substr(posRoom + 5, posDay - (posRoom + 6));
    transformToUpper(roomNumber);

    string day;
    day = message.substr(posDay + 4, posTime - (posDay + 5));

    string time;
    time = message.substr(posTime + 5, posOption - (posTime + 6));

    string option;
    option = message.substr(posOption + 7, posUsername - (posOption + 8));
    transformToLower(option);

    string Username;
    Username = message.substr((posUsername)+9,posUsernameEnd - (posUsername + 10));
    string encryptUserName = encrypt(Username);

    cout << "The main server has received the " << option <<" request on Room "<< roomNumber <<" at " << time << " on " << day << " from " << encryptUserName << " using TCP over port " << TCP_PORT << endl;
}

void handleAuthenticationResult(int client_sock, const string& message) {
    const size_t posFor = message.find("for");
    const string username = message.substr(posFor + 4);
    const string encryptUsername = encrypt(username);
    const int isAuthenticated = checkAuthenticationResult(message);
    cout <<"The main server received the authentication result for "<< encryptUsername << " using UDP over port " << UDP_PORT <<".\n";

    string response;
    if (isAuthenticated == 2) {
        response = "Authenticated:2";
    }
    else if (isAuthenticated == 1){
        response = "Authenticated:1";
    }
    else if(isAuthenticated == 0) {
        response = "Authenticated:0";
    }

    ssize_t sent = send(client_sock, response.c_str(), response.length(), 0);
    if (sent < 0) {
        cerr << "Failed to send authentication result, error: " << strerror(errno) << endl;
    } else {
        cout << "The main server sent the authentication result to the client." << endl;
    }
}



void handleUDP(int udp_sock) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];
    int client_sock;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_len = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            string receivedData(buffer);
            // Handle authentication result
            if (receivedData.find("Authentication:") == 0) {
                string username = getUserNameFromServerC(receivedData); // Implement this function
                client_sock = sessionToTcpSocketMap[username];
                handleAuthenticationResult(client_sock, receivedData);
            }
            // Handle server boot up
            else if(receivedData.find("BOOT:") == 0) {
                handleBootNotification(receivedData);
            }
            // Handle room request result
            else if(receivedData.find("Room request result") == 0) {
                string username = getUserNameFromServerC(receivedData);
                client_sock = sessionToTcpSocketMap[username];
                handleRoomRequestResult(client_sock, receivedData);
            }
        }
    }
}

string getUserNameFromServerC (const string& message) {
    const size_t userNamePos = message.find("for ");
    string userName = message.substr(userNamePos+4);
    return userName;

}

void handleGuestAuthenticationRequest(int client_sock, const string& userName) {
    sessionToTcpSocketMap[userName] = client_sock;
    const string encryptUserName = encrypt(userName);
    cout << "The main server received the guest request for " << encryptUserName<< " using TCP over port " << TCP_PORT << ".\n";
    cout << "The main server accepts "<< encryptUserName << " as a guest.\n";

    const string response = "Authenticated:3";
    ssize_t sent = send(client_sock, response.c_str(), response.length(), 0);
    if (sent < 0) {
        cerr << "Failed to send authentication result, error: " << strerror(errno) << endl;
    }
    else {
        cout << "The main server sent the guest response to the client." << endl;
    }
}

void handleRoomRequestResult(int client_sock, const string& response) {
    string serverName;
    size_t serverPos = response.find("Server");
    if (serverPos != string::npos) {
        serverName = response.substr(serverPos + 6, 3);
    }
    cout << "The main server received the response from Server " << serverName <<" using UDP over port " << UDP_PORT <<".\n";

    string option;
    if (response.find("Reservation") != string::npos) {
        option = "reservation";
    }
    else if (response.find("request") != string::npos) {
        option = "availability";
    }

    ssize_t sent = send(client_sock, response.c_str(), response.length(), 0);
    if (sent < 0) {
        cerr << "Failed to send authentication result, error: " << strerror(errno) << endl;
    } else {
        cout << "The main server sent the " << option << " information to the client. "  << endl;
    }
}

void handleGuestRoomReservationRequest(int client_sock, const string& response) {
    ssize_t sent = send(client_sock, response.c_str(), response.length(), 0);
    if (sent < 0) {
        cerr << "Failed to send authentication result, error: " << strerror(errno) << endl;
    } else {
        cout << "The main server sent the error message to the client." << endl;
    }
}

void handleBootNotification(const string& message) {
    const string server = message.substr(11);
    cout <<"The main server has received the notification from Server " << server <<
                " using UDP over port " <<UDP_PORT <<endl;
}

string handleCredential(const string& message) {
    string encryptUP = encryptCredential(message);
    return encryptUP;
}

string encryptCredential(const string& message) {
    const size_t passwWordPos = message.find("Password:");
    const size_t userNamePos = message.find("Username:");
    const string userName = message.substr(userNamePos+9,passwWordPos-userNamePos-10);
    const string passWord = message.substr(passwWordPos+9);
    const string encryptedUserName = encrypt(userName);
    const string encryptedPassword = encrypt(passWord);
    string encryptedMessage = "Username: " + encryptedUserName + " PassWord: " + encryptedPassword;
    return encryptedMessage;
}


char rotateChar(const char c, const int shift, const char start, const int count) {
    return start + (c - start + shift) % count;
}

string encrypt(const string& input) {
    string output = input;
    for (size_t i = 0; i < input.length(); ++i) {
        const char c = input[i];
        const int shift = i + 1;
        // Lower case
        if (c >= 'a' && c <= 'z') {
            output[i] = rotateChar(c, shift, 'a', 26);
        }
        // Upper case
        else if (c >= 'A' && c <= 'Z') {
            output[i] = rotateChar(c, shift, 'A', 26);
        }
        // Numbers
        else if (c >= '0' && c <= '9') { // Digits
            output[i] = rotateChar(c, shift, '0', 10);
        }
        // Special characters and symbols remain the same
        else {
            output[i] = c;
        }
    }
    return output;
}

void sendEncryptedCredentials(const int udp_sock, const string& message, const string& username) {
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVERC_UDP_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(DEST_UDP_IP);

    ssize_t sent = sendto(udp_sock, message.c_str(), message.length(), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        cerr << "Failed to send encrypted credentials over UDP\n";
    } else {
        cout << "The main server forwarded the authentication for "<< username <<" using UDP over port " <<UDP_PORT <<".\n";
    }
}

void sendRoomRequest(int udp_sock, string& message) {
    struct sockaddr_in dest_addr;
    size_t posRoom = message.find("Room:");
    size_t posDay = message.find("Day:");

    string roomNumber = message.substr(posRoom + 5, posDay - (posRoom + 6));
    transformToUpper(roomNumber);
    const string serverName = roomNumber.substr(0,3);
    if(serverName == "EEB") {
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVEREEB_UDP_PORT);
        dest_addr.sin_addr.s_addr = inet_addr(DEST_UDP_IP);
    }
    else if(serverName == "RTH") {
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVERRTH_UDP_PORT);
        dest_addr.sin_addr.s_addr = inet_addr(DEST_UDP_IP);
    }
    else {
        cout << "Server for the requested building does not exist." << endl;
    }

    ssize_t sent = sendto(udp_sock, message.c_str(), message.length(), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        cerr << "Failed to send encrypted credentials over UDP\n";
    }
    else {
        cout << "The main server sent a request to Server " << serverName << endl;
    }
}

void handleServerBootup(const string& message) {
    const string server = message.substr(5);
    cout <<" The main server has received the notification from "<< server << "using UDP over port " << UDP_PORT << endl;
}

bool isMember(const string& data) {
    return data.find("Membe") != -1;
}

bool isGuest(const string& data) {
    return data.find("Guest") != -1;
}

bool isReservationRequest(const string& data) {
    return data.find("Option:r") != -1 || data.find("Option:R") != -1;
}