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

int action;

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

void Client::depositMoney(double amount){
    action = DEPOSIT;
    send(clientSocket, &action, sizeof(action), 0);

    send(clientSocket, &amount, sizeof(amount), 0);
}

void Client::withdrawMoney(double amount){
    action = WITHDRAW;
    send(clientSocket, &action, sizeof(action), 0);

    send(clientSocket, &amount, sizeof(amount), 0);
}

double Client::getAccountBalance(){
    action = BALANCE;
    send(clientSocket, &action, sizeof(action), 0);

    double balance;
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

    send(clientSocket, &clientID, sizeof(clientID), 0);
    send(clientSocket, &password, sizeof(password), 0);
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

int main() {
    cout << "Client" << endl;

    Client client;
    client.connectToServer(SERVERADDR, PORT);

    ClientInfo info;
    info.clientID = 7777;
    strcpy(info.name, "AhmedEssam");
    info.age = 23;
    strcpy(info.nationalID, "30010060100217");
    strcpy(info.mobileNum, "01111168909");
    strcpy(info.email, "ahmedessam222@gmail.com");
    info.balance = 5000.0;

    // client.sendClientInfo(info);
    // cout << "Sent client info" << endl;
    client.clientLogin(info.clientID, 1234);
    cout << "User Logged in" << endl;

    

    int action;
    while(true){
            cout << "Enter an action" << endl;
            cin >> action;
            switch (action){
                case LOGIN:
                    cout << "Client Attempting clientLogin()" << endl;
                    client.clientLogin(info.clientID, 1234);
                break;
                case REGISTER:
                    cout << "Client Attempting clientRegister()" << endl;
                    client.clientRegister(info.clientID, 1234, info);
                break;
                case DISPLAYINFO:
                    cout << "Client Attempting displayInfo()" << endl;
                    info = client.displayInfo();
                    printInfo(info);
                break;
                case BALANCE:
                    cout << "Client Attempting getAccountBalance()" << endl;
                    double balance;
                    balance = client.getAccountBalance();
                    cout << "Current Balance = " << balance << endl;
                break;
                case DEPOSIT:
                    cout << "Client Attempting depositMoney()" << endl;
                    client.depositMoney(7000);
                break;
                case WITHDRAW:
                    cout << "Client Attempting withdrawMoney()" << endl;
                    client.withdrawMoney(1000);
                break;
                case TRANSACTION:
                    cout << "Client Attempting receiveTransaction()" << endl;
                    Transaction transaction;
                    transaction.fromAccount = info.clientID;
                    transaction.toAccount = 5555;
                    transaction.amount = 3500;
                    client.sendTransaction(transaction);
                break;
                case UNDO:
                    cout << "Client Attempting undoTransaction()" << endl;
                    client.undoTransaction();
                break;
                case REDO:
                    cout << "Client Attempting redoTransaction()" << endl;
                    client.redoTransaction();
                break;
                default:
                    cout << "Invalid Action" << endl;
                break;
            }

        }

    return 0;
}