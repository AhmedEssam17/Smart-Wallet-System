#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "../conf/conf.h"
using namespace std;

class Client {
public:
    Client();
    ~Client();

    void connectToServer(const string& serverAddress, int port);
    void sendClientInfo(const ClientInfo& info);
    void depositMoney(double amount);
    void withdrawMoney(double amount);
    double getAccountBalance();
    void sendTransaction(const Transaction& transaction);
    void undoTransaction();
    void redoTransaction();

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

// void sendString(int clientSocket, const std::string& str) {
//     int len = str.size() + 1;
//     send(clientSocket, &len, sizeof(len), 0);
//     send(clientSocket, str.c_str(), len, 0);
// }

// void Client::sendClientInfo(const ClientInfo& info){
//     // Send ID
//     send(clientSocket, &info.clientID, sizeof(info.clientID), 0);

//     // Send name
//     sendString(clientSocket, info.name);

//     // Send age
//     send(clientSocket, &info.age, sizeof(info.age), 0);

//     // Send national ID
//     send(clientSocket, &info.nationalID, sizeof(info.nationalID), 0);

//     // Send mobile number
//     // sendString(clientSocket, info.mobileNum);
//     send(clientSocket, &info.mobileNum, sizeof(info.mobileNum), 0);

//     // Send email
//     // sendString(clientSocket, info.email);
// }

// std::vector<char> serializeClientInfo(const ClientInfo& clientInfo) {
//     std::vector<char> data(sizeof(ClientInfo));
//     memcpy(data.data(), &clientInfo, sizeof(ClientInfo));
//     return data;
// }

void Client::sendClientInfo(const ClientInfo& clientInfo) {
    send(clientSocket, &clientInfo, sizeof(clientInfo), 0);
}

void Client::depositMoney(double amount){

}

void Client::withdrawMoney(double amount){

}

double Client::getAccountBalance(){
    double balance = 100.0;
    return balance;
}

void Client::sendTransaction(const Transaction& transaction){

}

void Client::undoTransaction(){

}

void Client::redoTransaction(){

}

int main() {
    cout << "Client" << endl;

    Client client;
    client.connectToServer(SERVERADDR, PORT);

    ClientInfo info;
    info.clientID = 4444;
    strcpy(info.name, "Essam");
    info.age = 23;
    strcpy(info.nationalID, "30010060100217");
    strcpy(info.mobileNum, "01111168909");
    strcpy(info.email, "ahmedessam222@gmail.com");
    info.balance = 0.0;

    client.sendClientInfo(info);
    cout << "Sent client info" << endl;

    while(true){
        // Stay connected
    }

    return 0;
}