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
#include <algorithm>
#include <cctype>
#include <string>
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
    void undoTransaction(int transactionID);
    void redoTransaction(int transactionID);
    void clientLogin(const int& clientID, const int& password);
    void clientRegister(const int& clientID, const int& password, const ClientInfo& info);
    ClientInfo displayInfo(const int& clientID);
    int receieveReponseFromServer();
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
    int networkAmount = htonl(amount);
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

void Client::undoTransaction(int transactionID) {
    action = UNDO;
    send(clientSocket, &action, sizeof(action), 0);
    send(clientSocket, &transactionID, sizeof(transactionID), 0);
}

void Client::redoTransaction(int transactionID) {
    action = REDO;
    send(clientSocket, &action, sizeof(action), 0);
    send(clientSocket, &transactionID, sizeof(transactionID), 0);
}

void Client::clientLogin(const int& clientID, const int& password){
    action = LOGIN;
    send(clientSocket, &action, sizeof(action), 0);

    int networkClientID = htonl(clientID);
    int networkPassword = htonl(password);

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

ClientInfo Client::displayInfo(const int& clientID){
    action = DISPLAYINFO;
    send(clientSocket, &action, sizeof(action), 0);

    ClientInfo clientInfo;
    recv(clientSocket, &clientInfo, sizeof(clientInfo), 0);

    return clientInfo;
}

int Client::receieveReponseFromServer(){
    int response;
    recv(clientSocket, &response, sizeof(response), 0);
    return response;
}

void sendResponseToUI(int response, int nodeSocket){
    string responseString;
    if(response != 0){
        responseString = "success";
    }
    else{
        responseString = "failed";
    }
    send(nodeSocket, responseString.c_str(), responseString.length(), 0);
}

void sendClientInfoAsJson(int new_socket, const ClientInfo& info) {
    std::ostringstream jsonStream;
    jsonStream << "{"
               << "\"clientID\": " << info.clientID << ", "
               << "\"name\": \"" << info.name << "\", "
               << "\"age\": " << info.age << ", "
               << "\"mobileNum\": \"" << info.mobileNum << "\", "
               << "\"nationalID\": \"" << info.nationalID << "\", "
               << "\"email\": \"" << info.email << "\", "
               << "\"balance\": " << info.balance
               << "}";

    std::string jsonString = jsonStream.str();
    send(new_socket, jsonString.c_str(), jsonString.length(), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

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

    int response = 0;

    while (true) {
        memset(buffer, 0, 1024);
        read(new_socket, buffer, 1024);
        string command(buffer);

        istringstream iss(command);
        string cmd;
        iss >> cmd;

        if (cmd == "login") {
            int clientID, password;
            iss >> clientID >> password;
            client.clientLogin(clientID, password);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } 
        else if (cmd == "register") {
            int clientID;
            int password;
            iss >> clientID >> password;
            string name, age, mobileNum, nationalID, email, balance;
            iss >> name >> age >> mobileNum >> nationalID >> email >> balance;

            ClientInfo info;
            info.clientID = clientID;
            strcpy(info.name, name.c_str());
            info.age = stoi(age);
            strcpy(info.mobileNum, mobileNum.c_str());
            strcpy(info.nationalID, nationalID.c_str());
            strcpy(info.email, email.c_str());
            info.balance = stoi(balance);

            client.clientRegister(clientID, password, info);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } 
        else if (cmd == "balance") {
            int clientID;
            iss >> clientID;
            int balance = client.getAccountBalance(clientID);
            string balanceStr = to_string(balance);
            send(new_socket, balanceStr.c_str(), balanceStr.size(), 0);
        } 
        else if (cmd == "deposit") {
            int clientID;
            int amount;
            iss >> clientID >> amount;
            client.depositMoney(clientID, amount);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } 
        else if (cmd == "withdraw") {
            int clientID;
            int amount;
            iss >> clientID >> amount;
            client.withdrawMoney(clientID, amount);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } 
        else if (cmd == "displayInfo") {
            int clientID;
            iss >> clientID;
            ClientInfo info = client.displayInfo(clientID);
            sendClientInfoAsJson(new_socket, info);
        } 
        else if (cmd == "sendTransaction") {
            int fromClientID, toClientID, amount;
            iss >> fromClientID >> toClientID >> amount;
            Transaction transaction = {fromClientID, toClientID, amount};
            client.sendTransaction(transaction);
            response = client.receieveReponseFromServer();
            string transId = to_string(response); 
            send(new_socket, transId.c_str(), transId.length(), 0);
        } 
        else if (cmd == "undo") {
            int transactionID;
            iss >> transactionID;
            client.undoTransaction(transactionID);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } 
        else if (cmd == "redo") {
            int transactionID;
            iss >> transactionID;
            client.redoTransaction(transactionID);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        }
    }

    return 0;
}
