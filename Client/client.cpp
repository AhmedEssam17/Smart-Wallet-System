#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "../conf/conf.h"
#include <sstream>
using namespace std;

int action;

class Client {
public:
    Client();
    ~Client();

    void connectToServer(const string& serverAddress, int port);
    void sendClientInfo(const ClientInfo& info);
    void depositMoney(const int& clientID, int amount);
    void withdrawMoney(const int& clientID, int amount);
    int getAccountBalance(const int& clientID);
    void sendTransaction(const Transaction& transaction);
    void undoTransaction();
    void redoTransaction();
    void clientLogin(const int& clientID, const int& password);
    void clientRegister(const int& clientID, const int& password, const ClientInfo& info);\
    ClientInfo displayInfo();

private:
    int clientSocket;
    void initSocket();
    void closeSocket();
};

Client::Client() {
    initSocket();
}

Client::~Client() {
    closeSocket();
}

void Client::initSocket(){
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Error creating socket" << endl;
        exit(1);
    }
}

void Client::closeSocket(){
    close(clientSocket);
}

void Client::connectToServer(const string& serverAddress, int port) {
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress.c_str());

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error connecting to server" << endl;
        exit(1);
    }

    cout << "Connected to server" << endl;
}

void Client::sendClientInfo(const ClientInfo& clientInfo) {
    send(clientSocket, &clientInfo, sizeof(clientInfo), 0);
}

void Client::depositMoney(const int& clientID, int amount){
    action = DEPOSIT;
    send(clientSocket, &action, sizeof(action), 0);

    int networkAmount = htonl(amount);

    send(clientSocket, &networkAmount, sizeof(networkAmount), 0);
}

void Client::withdrawMoney(const int& clientID, int amount){
    action = WITHDRAW;
    send(clientSocket, &action, sizeof(action), 0);
    cout << "amount = "<< amount << endl;
    int networkAmount = htonl(amount);
    cout << "networkAmount = "<< networkAmount << endl;
    send(clientSocket, &networkAmount, sizeof(networkAmount), 0);
}

int Client::getAccountBalance(const int& clientID){
    action = BALANCE;
    send(clientSocket, &action, sizeof(action), 0);

    int balance;
    recv(clientSocket, &balance, sizeof(balance), 0);

    return balance;
}

void Client::sendTransaction(const Transaction& transaction) {
    action = TRANSACTION;
    send(clientSocket, &action, sizeof(action), 0);

    send(clientSocket, &transaction, sizeof(transaction), 0);
}

void Client::undoTransaction(){
    action = UNDO;
    send(clientSocket, &action, sizeof(action), 0);
}

void Client::redoTransaction(){
    action = REDO;
    send(clientSocket, &action, sizeof(action), 0);
}

void Client::clientLogin(const int& clientID, const int& password){
    action = LOGIN;
    send(clientSocket, &action, sizeof(action), 0);

    int networkClientID = htonl(clientID);
    int networkPassword = htonl(password);

    cout << "clientID " << clientID << endl;
    cout << "password " << password << endl;

    send(clientSocket, &networkClientID, sizeof(networkClientID), 0);
    send(clientSocket, &networkPassword, sizeof(networkPassword), 0);
}

void Client::clientRegister(const int& clientID, const int& password, const ClientInfo& info){
    action = REGISTER;
    send(clientSocket, &action, sizeof(action), 0);

    send(clientSocket, &clientID, sizeof(clientID), 0);
    send(clientSocket, &password, sizeof(password), 0);
    sendClientInfo(info);
}

ClientInfo Client::displayInfo(){
    action = DISPLAYINFO;
    send(clientSocket, &action, sizeof(action), 0);

    ClientInfo clientInfo;
    recv(clientSocket, &clientInfo, sizeof(clientInfo), 0);

    return clientInfo;
}

void printInfo(const ClientInfo& clientInfo){
    cout << "ID = " << clientInfo.clientID << endl;
    cout << "Name = " << clientInfo.name << endl;
    cout << "Age = " << clientInfo.age << endl;
    cout << "Mobile Num = " << clientInfo.mobileNum << endl;
    cout << "NationalID = " << clientInfo.nationalID << endl;
    cout << "Balance = " << clientInfo.balance << endl;
    cout << "email = " << clientInfo.email << endl;
}

// int main(int argc, char* argv[]) {
//     Client client;
//     client.connectToServer(SERVERADDR, PORT);

//     if (argc > 1) {
//         std::string command = argv[1];
//         if (command == "--login" && argc == 4) {
//             int clientID = std::stoi(argv[2]);
//             int password = std::stoi(argv[3]);
//             client.clientLogin(clientID, password);
//         } else if (command == "--register" && argc == 4) {
//             int clientID = std::stoi(argv[2]);
//             int password = std::stoi(argv[3]);
//             // Assuming clientInfo is passed somehow or hardcoded for simplicity
//             ClientInfo info = {/* initialize fields */};
//             client.clientRegister(clientID, password, info);
//         } else if (command == "--balance" && argc == 3) {
//             int clientID = std::stoi(argv[2]);
//             std::cout << client.getAccountBalance(clientID);
//         } else if (command == "--deposit" && argc == 4) {
//             int clientID = std::stoi(argv[2]);
//             double amount = std::stod(argv[3]);
//             client.depositMoney(clientID, amount);
//         } else if (command == "--withdraw" && argc == 4) {
//             int clientID = std::stoi(argv[2]);
//             double amount = std::stod(argv[3]);
//             client.withdrawMoney(clientID, amount);
//         }
//         // Add more commands as needed
//     }

//     return 0;
// }

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    Client client;
    client.connectToServer(SERVERADDR, PORT);

    while (true) {
        memset(buffer, 0, 1024);
        read(new_socket, buffer, 1024);
        std::string command(buffer);

        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "login") {
            int clientID, password;
            iss >> clientID >> password;
            cout << "Received command: " << cmd << endl;
            cout << "clientID: " << clientID << endl;
            cout << "password: " << password << endl;
            client.clientLogin(clientID, password);
        } else if (cmd == "register") {
            int clientID;
            int password;
            iss >> clientID >> password;

            std::string name, age, mobileNum, nationalID, email, balance;
            iss >> name >> age >> mobileNum >> nationalID >> email >> balance;

            ClientInfo info;
            info.clientID = clientID;
            strcpy(info.name, name.c_str());
            info.age = std::stoi(age);
            strcpy(info.mobileNum, mobileNum.c_str());
            strcpy(info.nationalID, nationalID.c_str());
            strcpy(info.email, email.c_str());
            info.balance = std::stoi(balance);

            client.clientRegister(clientID, password, info);
        } else if (cmd == "balance") {
            int clientID;
            iss >> clientID;
            cout << "Received command: " << cmd << endl;
            cout << "clientID: " << clientID << endl;
            int balance = client.getAccountBalance(clientID);
            cout << "After getAccountBalance >> Balance = " << balance << endl;
            std::string balanceStr = std::to_string(balance);
            send(new_socket, balanceStr.c_str(), balanceStr.size(), 0);
        } else if (cmd == "deposit") {
            int clientID;
            int amount;
            cout << "Received command: " << cmd << endl;
            iss >> clientID >> amount;
            cout << "clientID: " << clientID << endl;
            cout << "amount: " << amount << endl;
            client.depositMoney(clientID, amount);
        } else if (cmd == "withdraw") {
            int clientID;
            int amount;
            cout << "Received command: " << cmd << endl;
            iss >> clientID >> amount;
            cout << "clientID: " << clientID << endl;
            cout << "amount: " << amount << endl;
            client.withdrawMoney(clientID, amount);
        }
        // Add more commands as needed
    }

    return 0;
}
