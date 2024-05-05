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
    void clientLogin(const int& clientID, const int& password);

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

void Client::depositMoney(double amount){

}

void Client::withdrawMoney(double amount){

}

double Client::getAccountBalance(){
    double balance = 100.0;
    return balance;
}

void Client::sendTransaction(const Transaction& transaction) {
    send(clientSocket, &transaction, sizeof(transaction), 0);
}

void Client::undoTransaction(){

}

void Client::redoTransaction(){

}

void Client::clientLogin(const int& clientID, const int& password){
    send(clientSocket, &clientID, sizeof(clientID), 0);
    send(clientSocket, &password, sizeof(password), 0);
}

int main() {
    cout << "Client" << endl;

    Client client;
    client.connectToServer(SERVERADDR, PORT);

    ClientInfo info;
    info.clientID = 6666;
    strcpy(info.name, "Essam");
    info.age = 23;
    strcpy(info.nationalID, "30010060100217");
    strcpy(info.mobileNum, "01111168909");
    strcpy(info.email, "ahmedessam222@gmail.com");
    info.balance = 3000.0;

    // client.sendClientInfo(info);
    // cout << "Sent client info" << endl;
    client.clientLogin(info.clientID, 1234);
    cout << "User Logged in" << endl;

    while(true){
        // Stay connected
    }

    return 0;
}