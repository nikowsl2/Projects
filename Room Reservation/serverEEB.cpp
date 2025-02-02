#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <tuple>
#include <arpa/inet.h>
#include <cctype>

#define UDP_PORT 33908
#define SERVER_M_IP "127.0.0.1"
#define SERVER_M_UDP_PORT 34908
using namespace std;

// ScheduleKey design is from ChatGPT
struct ScheduleKey {
    string room;
    string day;
    string time;

    ScheduleKey(string room, string day, string time)
        : room(room), day(day), time(time) {}

    bool operator<(const ScheduleKey& other) const {
        return tie(room, day, time) <
               tie(other.room, other.day, other.time);
    }
};

map<ScheduleKey, bool> EEBRoomList;

int sendNotificationToServerM(int udp_sock);

void handleUDP(int udp_sock);

int runServerEEB();

string handleRoomRequest(const string& message);

void transformToLower(string& s);

void transformToUpper(string& s);

string handleRoomReservation(const string& message);

void removeSpace(string& s);

void formatDay(string& s);

string handleRoomAvailability(const string& message);

int sendAvailabilityResultToServerM(int udp_sock,const string& result);

int main() {
    runServerEEB();
    return 0;
}

int sendNotificationToServerM(int udp_sock) {
    const char* message = "BOOT:ServerEEB";
    sockaddr_in server_m_addr;

    memset(&server_m_addr, 0, sizeof(server_m_addr));
    server_m_addr.sin_family = AF_INET;
    server_m_addr.sin_port = htons(SERVER_M_UDP_PORT);
    server_m_addr.sin_addr.s_addr = inet_addr(SERVER_M_IP);

    if (sendto(udp_sock, message, strlen(message), 0, (sockaddr *)&server_m_addr, sizeof(server_m_addr)) < 0) {
        cerr << "Failed to send notification to Server M\n";
        return -1;
    }

    cout << "The Server EEB has informed the main server." << endl;
    return 0;
}

int sendAvailabilityResultToServerM(int udp_sock,const string& result) {
    string message = "Room request result from ServerEEB:\n" + result;
    sockaddr_in server_m_addr;

    memset(&server_m_addr, 0, sizeof(server_m_addr));
    server_m_addr.sin_family = AF_INET;
    server_m_addr.sin_port = htons(SERVER_M_UDP_PORT);
    server_m_addr.sin_addr.s_addr = inet_addr(SERVER_M_IP);

    if (sendto(udp_sock, message.c_str(), strlen(message.c_str()), 0, (sockaddr *)&server_m_addr, sizeof(server_m_addr)) < 0) {
        cerr << "Failed to send notification to Server M\n";
        return -1;
    }

    cout << "The Server EEB finished sending the response to the main server." << endl;
    return 0;
}

int runServerEEB() {
    int udp_sock;
    sockaddr_in udp_addr;

    // UDP socket
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == -1) {
        cerr << "Failed to create UDP socket\n";
        return -1;
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (::bind(udp_sock, (sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        cerr << "Failed to bind UDP socket to port " << UDP_PORT << "\n";
        return -1;
    }

    ifstream file("EEB.txt");
    if (!file.is_open()) {
        cerr << "Failed to open EEB.txt\n";
        close(udp_sock);
        return -1;
    }

    string line;
    while (getline(file, line)) {
        istringstream ss(line);
        string room, day, time;

        getline(ss, room, ',');
        getline(ss, day, ',');
        getline(ss, time, ',');

        room.erase(0, room.find_first_not_of(' '));
        room.erase(room.find_last_not_of(' ') + 1);
        day.erase(0, day.find_first_not_of(' '));
        day.erase(day.find_last_not_of(' ') + 1);
        time.erase(0, time.find_first_not_of(' '));
        time.erase(time.find_last_not_of(' ') + 1);

        EEBRoomList[ScheduleKey(room, day, time)] = true;
    }

    file.close();
    cout << "The Server EEB is up and running using UDP on port " << UDP_PORT << ".\n";

    if (sendNotificationToServerM(udp_sock) < 0) {
        close(udp_sock);
        return -1;
    }

    handleUDP(udp_sock);
    close(udp_sock);

    return 0;
}

void handleUDP(int udp_sock) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_len = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            string message = buffer;
            if (message.find("Room:") == 0) {
                string result = handleRoomRequest(message);
                sendAvailabilityResultToServerM(udp_sock,result);
            }
        }
        else if (recv_len < 0) {
            cerr << "Error receiving data: " << strerror(errno) << endl;
        }
    }
}

string handleRoomRequest(const string& message) {
    const size_t posOption = message.find("Option:");
    const size_t posUsername = message.find("Username:");
    string option = message.substr(posOption + 7, posUsername - (posOption + 8));
    transformToLower(option);
    string result;
    if (option == "reservation") {
        result = handleRoomReservation(message);
    }
    else if(option == "availability") {
        result = handleRoomAvailability(message);
    }
    return result;
}

string handleRoomReservation(const string& message) {
    cout << "The Server EEB received a reservation request from the main server." << endl;
    string result;
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
    formatDay(day);

    string time;
    time = message.substr(posTime + 5, posOption - (posTime + 6));
    removeSpace(time);
    transformToLower(time);

    string option;
    option = message.substr(posOption + 6, posUsername - (posOption + 8));
    transformToLower(option);

    string username = message.substr(posUsername + 9, posUsernameEnd - (posUsername + 10));

    roomNumber.erase(roomNumber.find_last_not_of(' ') + 1);
    day.erase(day.find_last_not_of(' ') + 1);
    time.erase(time.find_last_not_of(' ') + 1);
    bool roomExists = false;
    for (const auto& entry : EEBRoomList) {
        if (entry.first.room == roomNumber) {
            roomExists = true;
            break;
        }
    }

    if (!roomExists) {
        cout << "Cannot make a reservation. Not able to find the room " << roomNumber << ".\n";
        result += "Reservation result: " + roomNumber + " F0";
        return result += " for " + username;;
    }


    ScheduleKey key(roomNumber, day, time);
    auto it = EEBRoomList.find(key);
    if (it != EEBRoomList.end()) {
        if (it->second) {
            it->second = false;
            cout << "Successful reservation for " << roomNumber << " at " << time << " on " << day << ".\n";
            result += "Reservation result: " + roomNumber + " S0";
            return result += " for " + username;
        }
        cout << "Room " << roomNumber << " is not available at " << time << " on " << day << ".\n";
        result += "Reservation result: " + roomNumber + " F1";
        return result += " for " + username;

    }
    cout << "No information for Room " << roomNumber << " at " << time << " on " << day << ".\n";
    result += "Reservation result: " + roomNumber + " F2";
    return result += " for " + username ;
}




string handleRoomAvailability(const string& message) {
    cout << "The Server EEB received an availability request from the main server." << endl;
    string result;
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
    formatDay(day);

    string time;
    time = message.substr(posTime + 5, posOption - (posTime + 6));
    removeSpace(time);
    transformToLower(time);

    string option;
    option = message.substr(posOption + 7, posUsername - (posOption + 8));
    transformToLower(option);

    string username = message.substr(posUsername + 9, posUsernameEnd - (posUsername + 10));

    roomNumber.erase(roomNumber.find_last_not_of(' ') + 1);
    day.erase(day.find_last_not_of(' ') + 1);
    time.erase(time.find_last_not_of(' ') + 1);
    bool roomExists = false;
    for (const auto& entry : EEBRoomList) {
        if (entry.first.room == roomNumber) {
            roomExists = true;
            break;
        }
    }

    if (!roomExists) {
        cout << "Not able to find the room " << roomNumber << endl;
        result += "Not able to find the room " + roomNumber + ".\n";
        return result  + " for " + username;
    }

    if (day.empty() && time.empty()) {
        bool found = false;
        for (const auto& entry : EEBRoomList) {
            if (entry.first.room == roomNumber && entry.second) {
                cout << "Day: " << entry.first.day << ", Time: " << entry.first.time << " - Available" << endl;
                result += "Day: " + entry.first.day + ", Time: " + entry.first.time + " - Available\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No available times: room " << roomNumber << "." << endl;
            result += "No available times: room " + roomNumber + ".\n";
        }
        else {
            cout << "All the availability of the room "<< roomNumber<<" has been extracted." << endl;
        }
    }
    else if (!day.empty() && time.empty()) {
        bool found = false;
        for (const auto& entry : EEBRoomList) {
            if (entry.first.room == roomNumber && entry.first.day == day && entry.second) {
                cout << "Time: " << entry.first.time << " - Available" << endl;
                result += "Time: " + entry.first.time + " - Available\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No available times: room " << roomNumber << " on " << day << "." << endl;
            result += "No available times: room " + roomNumber + " on " + day + ".\n";
        }
        else {
            cout << "All the availability of the room "<< roomNumber << " on " << day <<" has been extracted." << endl;
            result += "Day of Week: " + day;
        }
    }
    else {
        ScheduleKey key(roomNumber, day, time);
        auto it = EEBRoomList.find(key);
        if (it != EEBRoomList.end() && it->second) {
            cout << "Room " << roomNumber <<" is available at "<< time <<" on "<< day << " day." << endl;
            result += "Room " + roomNumber + " is available at " + time + " on " + day +  " day.\n";
        } else {
            cout << "Room " << roomNumber << " is not available at " << time << " on " << day << "." << endl;
            result += "Room " + roomNumber + " is not available at " + time + " on " + day + ".\n";
        }
    }
    return result+ " for " + username;
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

void removeSpace(string& s) {
    string result;
    bool lastWasDigit = false;
    for (char c : s) {
        if (c != ' ') {
            if (isdigit(c)) {
                result += c;
                lastWasDigit = true;
            }
            else {
                if (lastWasDigit) {
                    result += ' ';
                    lastWasDigit = false;
                }
                result += c;
            }
        }
    }
    s = result;
}

void formatDay(string& s) {
    if (s.empty()) return;
    transformToLower(s);
    s[0] = toupper(static_cast<unsigned char>(s[0]));
}