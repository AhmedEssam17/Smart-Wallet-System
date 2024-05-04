#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;

struct ClientInfo {
    int clientId;
    string name;
    int age;
    string nationalID;
    string mobileNum;
    string email;
};

struct Transaction {
    string fromAccount;
    string toAccount;
    double amount;
};

class Server {
public:
    Server();
    ~Server();

    void start();
    void stop();

    void receiveClientInfo(const int& clientID, const ClientInfo& info);
    void depositMoney(const int& clientID, double amount);
    void withdrawMoney(const int& clientID, double amount);
    double getAccountBalance(const int& clientID);
    void processTransaction(const int& clientID, const Transaction& transaction);
    void undoTransaction(const int& clientID);
    void redoTransaction(const int& clientID);
    void displayClientInfo(const int& clientID);

private:
    int serverSocket;
    void initSocket();
    void closeSocket();
    void listenForConnections();

    // map<string, ClientInfo> m_clientInfo;
    // map<string, double> m_accountBalances;
    // map<string, vector<Transaction>> m_transactions;
};

Server::Server() {
    initSocket();
}

Server::~Server() {
}

void Server::start() {
    listenForConnections();
}

void Server::stop() {
    close(serverSocket);
}

void Server::initSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error creating socket" << endl;
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error binding socket" << endl;
        exit(1);
    }
}

void Server::closeSocket() {
    close(serverSocket);
}

void Server::listenForConnections() {
    if (listen(serverSocket, 5) < 0) {
        cerr << "Error listening on socket" << endl;
        exit(1);
    }

    cout << "Server started. Listening on port 8888..." << endl;

    while (true) {
        // Accept incoming connections and handle them
    }
}

void Server::receiveClientInfo(const int& clientID, const ClientInfo& info){

}

void Server::depositMoney(const int& clientID, double amount){

}

void Server::withdrawMoney(const int& clientID, double amount){

}

double Server::getAccountBalance(const int& clientID){
    double balance = 100.0;
    return balance;
}

void Server::processTransaction(const int& clientID, const Transaction& transaction){

}

void Server::undoTransaction(const int& clientID){

}

void Server::redoTransaction(const int& clientID){

}

void Server::displayClientInfo(const int& clientID){

}


int main() {

    cout << "Server" << endl;
    
    Server server;
    server.start();

    // Accept incoming connections
    while (true) {
        //TODO
    }

    server.stop();

    return 0;
}