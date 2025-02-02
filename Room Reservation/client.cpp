#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define TCP_PORT 35908
bool guest = true;

string getUsername();

string getPassword();

int sendUserCredentialToServerM(int tcp_sock, const string& userName, const string& passWord);

int receiveAuthenticationResult(int tcp_sock);

string getInput(const string& prompt);

void handleRoomBooking(int tcp_sock, const string& username);

void handleRoomRequestResult(const string& result);

void handleGuestRoomBooking(int tcp_sock, const string& username);

void handleRoomReservationResult(const string& result);

void trim(string& str);

int main() {
    //TCP Setup
    sockaddr_in server_addr;
    const int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        cerr << "Failed to create TCP socket\n";
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // TCP connect
    if (connect(tcp_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Failed to connect to the server TCP: " << strerror(errno) << endl;
        close(tcp_sock);
        return -1;
    }

    // Child socket
    sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(tcp_sock, (sockaddr *)&local_addr, &addr_len) == -1) {
        cerr << "Failed to get socket name\n";
        close(tcp_sock);
        return -1;
    }

    cout << "Client is up and running." << endl;
    const string userName = getUsername();
    const string passWord = getPassword();

    // Sending credentials to serverM
    if (sendUserCredentialToServerM(tcp_sock, userName, passWord) != 0) {
        cerr << "Error sending credentials. Exiting...\n";
        close(tcp_sock);
        return -1;
    }

    cout << userName << " sent an authentication request to the main server." << endl;

    // Receive and process the authentication result
    int authResult = receiveAuthenticationResult(tcp_sock);
    if (authResult == 2) {
        cout << "Welcome member " << userName << "!" << endl;
        guest = false;
    }
    else if (authResult == 3) {
        cout << "Welcome guest " << userName << "!" << endl;
    }
    else if (authResult == 1) {
        cout << "Failed login: Password does not match." << endl;
        close(tcp_sock);
        return -1;
    }
    else if (authResult == 0) {
        cout << "Failed login: Username does not exist." << endl;
        close(tcp_sock);
        return -1;
    }
    else {
        cout << "Invalid server response. Please try again." << endl;
        close(tcp_sock);
        return -1;
    }

    // Handle guest or member request
    string continueBooking = "y";
    while (continueBooking == "y") {
        if (guest) {
            handleGuestRoomBooking(tcp_sock, userName);
        } else {
            handleRoomBooking(tcp_sock, userName);
        }
        cout << "-----Start a new request----- (y/n): ";
        getline(cin, continueBooking);
        trim(continueBooking);
    }
    close(tcp_sock);
    return 0;
}

string getUsername() {
    string username;
    while (username.empty()) {
        cout << "Please enter your username: ";
        getline(cin, username);
        if (username.empty()) {
            cout << "Username cannot be empty. Please try again.\n";
        }

        // Clear the error state of cin in case of an input error (from ChatGPT)
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    return username;
}

string getPassword() {
    string password;
    cout << "Please enter your password: ";
    getline(cin, password);

    // Clear the error state of cin in case of an input error (from ChatGPT)
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return password;
}


int sendUserCredentialToServerM(const int tcp_sock, const string& userName, const string& passWord) {
    const string message = "Credential: Username:" + userName + " " + "Password:" +passWord ;
    ssize_t bytes_sent = send(tcp_sock, message.c_str(), message.length(), 0);
    if (bytes_sent < 0) {
        cerr << "Failed to send Credential\n";
        return -1;
    }
    return 0;
}

int receiveAuthenticationResult(int tcp_sock) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = recv(tcp_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        cerr << "Failed to receive authentication result\n";
        return -1;
    }

    string response(buffer);
    // Authenticated
    if (response.find("Authenticated:2") != string::npos) {
        return 2;
    }
    // Not authenticated
    if (response.find("Authenticated:1") != string::npos) {
        return 1;
    }
    // Not authenticated
    if (response.find("Authenticated:0") != string::npos) {
        return 0;
    }
    // Guest
    if (response.find("Authenticated:3") != string::npos) {
        return 3;
    }
    return -1;
}

string getInput(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

void handleRoomBooking(int tcp_sock, const string& username) {
    string roomNumber = getInput("Please enter the room number: ");
    string day = getInput("Please enter the day: ");
    string time = getInput("Please enter the time: ");
    string option = getInput("Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation): ");

    string bookingRequest = "Room:" + roomNumber + " Day:" + day + " Time:" + time + " Option:" + option + " Username:" + username + " !Membe";

    ssize_t bytes_sent = send(tcp_sock, bookingRequest.c_str(), bookingRequest.length(), 0);
    if (bytes_sent < 0) {
        cerr << "Failed to send booking information\n";
    } else {
        cout << username << " sent a " << option <<" request to the main server."<<endl;
    }

    char buffer[1024];
    ssize_t bytes_received = recv(tcp_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        cout << "The client received the response from the main server using TCP over port "<< tcp_sock << "." << endl;
        string result = buffer;
        size_t pos = result.find("Reservation result:");
        if (pos != string::npos) {
            handleRoomReservationResult(result.substr(pos));
        }
        else {
            handleRoomRequestResult(result);
        }
    }
    else {
        cerr << "Failed to receive server response for booking.\n";
    }
}

void handleGuestRoomBooking(int tcp_sock, const string& username) {
    string roomNumber = getInput("Please enter the room number: ");
    string day = getInput("Please enter the day: ");
    string time = getInput("Please enter the time: ");
    string option = getInput("Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation): ");

    string bookingRequest = "Room:" + roomNumber + " Day:" + day + " Time:" + time + " Option:" + option + " Username:" + username + " !Guest";

    ssize_t bytes_sent = send(tcp_sock, bookingRequest.c_str(), bookingRequest.length(), 0);
    if (bytes_sent < 0) {
        cerr << "Failed to send booking information\n";
    } else {
        cout << username << " sent a " << option <<" request to the main server."<<endl;
    }

    char buffer[1024];
    ssize_t bytes_received = recv(tcp_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        cout << "The client received the response from the main server using TCP over port "<< tcp_sock << "." << endl;
        string result = buffer;
        size_t pos = result.find("Reservation result:");
        if (pos != string::npos) {
            handleRoomReservationResult(result.substr(pos));
        }
        else {
            handleRoomRequestResult(result);
        }
    }
    else {
        cerr << "Failed to receive server response for booking.\n";
    }
}

void handleRoomRequestResult(const string& result){
    stringstream ss(result);
    string line;
    string day;
    vector<string> times;
    bool dayFound = false;

    while (getline(ss, line)) {
        if (line.find("no") != string::npos || line.find("is available at") != string::npos || line.find("No") != string::npos|| line.find("not") != string::npos || line.find("Not") != string::npos) {
            cout << line << endl;
            cout <<endl;
            return;
        }

        if (line.find("Day:") != string::npos) {
            size_t start = line.find("Day:") + 5;
            size_t end = line.find(" - Available");
            if (start != string::npos && end != string::npos) {
                times.push_back(line.substr(start, end - start));
                trim(times.back());
            }
            trim(day);
        }

        else if (line.find("Time:") != string::npos) {
            size_t start = line.find("Time:") + 6;
            size_t end = line.find(" - Available");
            if (start != string::npos && end != string::npos) {
                times.push_back(line.substr(start, end - start));
                trim(times.back());
            }
        }

        else {
            trim(line);
            size_t dayIndex = line.find("Day of Week:");
            if (!line.empty() && !dayFound && dayIndex != string::npos) {
                day = line.substr(dayIndex + 12);
                trim(day);
                dayFound = true;
            }
        }
    }

    if (dayFound){
        cout << "The request room is available at the following time slots on " << day << ":\n";
    }
    else {
        cout << "The request room is available at the following time slots:\n";
    }
    for (const auto& time : times) {
        cout << "<" << time << ">\n";
    }
    cout << endl;
}

void handleRoomReservationResult(const string& result) {
    stringstream ss(result);
    string line;
    string roomNumber;
    while (getline(ss, line)) {
        string prefix = "Reservation result: ";
        size_t startPos = line.find(prefix);
        if (startPos != string::npos) {
            startPos += prefix.length();
            size_t endPos = line.find(' ', startPos);
            if (endPos != string::npos) {
                roomNumber = line.substr(startPos, endPos - startPos);
                if (line.find("S0") != string::npos) {
                    cout << "Congratulations! The reservation for Room " << roomNumber << " has been made." << endl;
                }
                else if (line.find("F0") != string::npos) {
                    cout << "Oops! Not able to find the room " << roomNumber << ".\n";
                }
                else if (line.find("F1") != string::npos || line.find("F2") != string::npos) {
                    cout << "Sorry! The requested room " << roomNumber << " is not available." << endl;
                }
                else {
                    cout << "An unknown error occurred." << endl;
                }
                cout << endl;
                return;
            }
        }
    }
}

void trim(string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) {
        str = "";
        return;
    }
    size_t last = str.find_last_not_of(' ');
    str = str.substr(first, (last - first + 1));
}
