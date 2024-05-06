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
    void undoTransaction(int transactionID);
    void redoTransaction(int transactionID);
    void clientLogin(const int& clientID, const int& password);
    void clientRegister(const int& clientID, const int& password, const ClientInfo& info);
    ClientInfo displayInfo(const int& clientID);
    int receieveReponseFromServer();
    std::vector<TransactionTable> fetchActiveTransactions(const int& clientId);
    std::vector<TransactionTable> fetchUndoneTransactions(const int& clientId);
    std::vector<TransactionTable> parseTransactions(const std::string& data);

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

ClientInfo Client::displayInfo(const int& clientID){
    action = DISPLAYINFO;
    send(clientSocket, &action, sizeof(action), 0);

    ClientInfo clientInfo;
    recv(clientSocket, &clientInfo, sizeof(clientInfo), 0);

    return clientInfo;
}

// Function to request and receive active transactions
std::vector<TransactionTable> Client::fetchActiveTransactions(const int& clientId) {
    int action = ACTIVETRANSACTION;
    send(clientSocket, &action, sizeof(action), 0);
    send(clientSocket, &clientId, sizeof(clientId), 0);

    char buffer[4096];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string data(buffer);
    return parseTransactions(data);
}

std::vector<TransactionTable> Client::fetchUndoneTransactions(const int& clientId) {
    int action = UNDONETRANSACTION;
    send(clientSocket, &action, sizeof(action), 0);
    send(clientSocket, &clientId, sizeof(clientId), 0);

    char buffer[4096];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string data(buffer);
    return parseTransactions(data);
}

std::vector<TransactionTable> Client::parseTransactions(const std::string& data) {
    std::vector<TransactionTable> transactions;
    std::istringstream iss(data);
    std::string token;
    while (getline(iss, token, '{')) {
        getline(iss, token, '}');
        std::istringstream tokenStream(token);
        std::string key, value;
        TransactionTable tx;
        while (getline(tokenStream, key, ':')) {
            getline(tokenStream, value, ',');
            if (key.find("transactionID") != std::string::npos) {
                tx.transactionID = std::stoi(value);
            } else if (key.find("fromAccount") != std::string::npos) {
                tx.fromAccount = std::stoi(value);
            } else if (key.find("toAccount") != std::string::npos) {
                tx.toAccount = std::stoi(value);
            } else if (key.find("amount") != std::string::npos) {
                tx.amount = std::stoi(value);
            }
        }
        transactions.push_back(tx);
    }
    return transactions;
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

int Client::receieveReponseFromServer(){
    int response;
    recv(clientSocket, &response, sizeof(response), 0);
    cout << "response = "<< response << endl;
    return response;
}

void sendResponseToUI(int response, int nodeSocket){
    string responseString;
    if(response == 1){
        responseString = "success";
    }
    else{
        responseString = "failed";
    }
    cout << "responseString = " << responseString << endl;
    send(nodeSocket, responseString.c_str(), responseString.length(), 0);
}

#include <sstream>
#include <string>

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

void sendTransactionTableAsJson(int new_socket, const std::vector<TransactionTable>& transactions){
    std::ostringstream jsonStream;
    jsonStream << "[";
    for (size_t i = 0; i < transactions.size(); ++i) {
        const auto& tx = transactions[i];
        jsonStream << "{"
                    << "\"transactionID\": " << tx.transactionID << ", "
                   << "\"fromAccount\": " << tx.fromAccount << ", "
                   << "\"toAccount\": " << tx.toAccount << ", "
                   << "\"amount\": " << tx.amount
                   << "}";
        if (i < transactions.size() - 1) jsonStream << ", ";
    }
    jsonStream << "]";
    std::string data = jsonStream.str();
    send(new_socket, data.c_str(), data.length(), 0);
}

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
            cout << "Received command: " << cmd << endl;
            cout << "clientID: " << clientID << endl;
            cout << "password: " << password << endl;
            client.clientLogin(clientID, password);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "register") {
            int clientID;
            int password;
            iss >> clientID >> password;

            string name, age, mobileNum, nationalID, email, balance;
            iss >> name >> age >> mobileNum >> nationalID >> email >> balance;

            ClientInfo info;
            info.clientID = clientID;
            strcpy(info.name, name.c_str());
            
            try {
                info.age = stoi(age);
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid age: " << age << endl;
                continue; // Skip processing this line
            }

            strcpy(info.mobileNum, mobileNum.c_str());
            strcpy(info.nationalID, nationalID.c_str());
            strcpy(info.email, email.c_str());
            
            try {
                info.balance = stoi(balance);
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid balance: " << balance << endl;
                continue; // Skip processing this line
            }

            client.clientRegister(clientID, password, info);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "balance") {
            int clientID;
            iss >> clientID;
            cout << "Received command: " << cmd << endl;
            cout << "clientID: " << clientID << endl;
            int balance = client.getAccountBalance(clientID);
            // response = client.receieveReponseFromServer();
            cout << "After getAccountBalance >> Balance = " << balance << endl;
            string balanceStr = to_string(balance);
            send(new_socket, balanceStr.c_str(), balanceStr.size(), 0);
            // sendResponseToUI(response, new_socket);
        } else if (cmd == "deposit") {
            int clientID;
            int amount;
            cout << "Received command: " << cmd << endl;
            iss >> clientID >> amount;
            cout << "clientID: " << clientID << endl;
            cout << "amount: " << amount << endl;
            client.depositMoney(clientID, amount);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "withdraw") {
            int clientID;
            int amount;
            cout << "Received command: " << cmd << endl;
            iss >> clientID >> amount;
            cout << "clientID: " << clientID << endl;
            cout << "amount: " << amount << endl;
            client.withdrawMoney(clientID, amount);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "displayInfo") {
            int clientID;
            iss >> clientID;
            cout << "Received command: " << cmd << endl;
            cout << "clientID: " << clientID << endl;
            ClientInfo info = client.displayInfo(clientID);
            sendClientInfoAsJson(new_socket, info);
        } else if (cmd == "sendTransaction") {
            int fromClientID, toClientID, amount;
            iss >> fromClientID >> toClientID >> amount;
            cout << "Received command: " << cmd << endl;
            cout << "From ClientID: " << fromClientID << ", To ClientID: " << toClientID << ", Amount: " << amount << endl;
            Transaction transaction = {fromClientID, toClientID, amount};
            client.sendTransaction(transaction);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "undo") {
            int transactionID;
            iss >> transactionID;
            cout << "Received command: " << cmd << " for Transaction ID: " << transactionID << endl;
            client.undoTransaction(transactionID);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "redo") {
            int transactionID;
            iss >> transactionID;
            cout << "Received command: " << cmd << " for Transaction ID: " << transactionID << endl;
            client.redoTransaction(transactionID);
            response = client.receieveReponseFromServer();
            sendResponseToUI(response, new_socket);
        } else if (cmd == "fetchActiveTransactions") {
            int clientId;
            iss >> clientId;
            std::vector<TransactionTable> activeTransactions = client.fetchActiveTransactions(clientId);
            sendTransactionTableAsJson(new_socket, activeTransactions);
        } else if (cmd == "fetchUndoneTransactions") {
            int clientId;
            iss >> clientId;
            std::vector<TransactionTable> undoneTransactions = client.fetchUndoneTransactions(clientId);
            sendTransactionTableAsJson(new_socket, undoneTransactions);
        }
    }

    return 0;
}
