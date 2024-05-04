#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../conf/conf.h"
#include <sqlite3.h>
#include <thread>
#include <mutex>
using namespace std;

sqlite3* db;

class Server {
public:
    Server();
    ~Server();

    void start();
    void stop();

    double getAccountBalance(const int& clientID);
    void depositMoney(const int& clientID, double amount);
    void withdrawMoney(const int& clientID, double amount);
    void processTransaction(const int& clientID, const Transaction& transaction);
    void undoTransaction(const int& clientID);
    void redoTransaction(const int& clientID);
    void displayClientInfo(const int& clientID);

private:
    int serverSocket;
    std::mutex dbMutex;
    void initSocket();
    void closeSocket();
    void listenForConnections();
    void handleConnection(int clientSocket);
    ClientInfo receiveClientInfo(int clientSocket);

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
    serverAddr.sin_port = htons(PORT);

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

    int clientCount = 0;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cerr << "Error accepting client connection" << endl;
            continue;
        }

        // Handle the client connection in a separate thread or process
        // std::thread clientThread(&Server::handleConnection, this, clientSocket);
        // clientThread.detach(); // Detach the thread to let it run independently
        // clientCount++;
        // cout << "clientCount = " << clientCount << endl;
        handleConnection(clientSocket);
    }
}

void storeClientInfo(const ClientInfo& info) {
    char* errMsg = nullptr;

    // Construct the SQL query using placeholders (?)
    string sql = "INSERT INTO clients (clientID, name, age, nationalID, mobileNum, email, balance) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?);";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind parameters to the statement
    sqlite3_bind_int(stmt, 1, info.clientID);
    sqlite3_bind_text(stmt, 2, info.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, info.age);
    sqlite3_bind_text(stmt, 4, info.nationalID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, info.mobileNum.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, info.email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 7, info.balance);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Client's Data is stored in the Database Successfully" << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}


void Server::handleConnection(int clientSocket) {
    try {
        if (clientSocket < 0) {
            cerr << "Invalid client socket" << endl;
            return;
        }

        cout << "Inside handleConnection" << endl;
        // Receive client info
        ClientInfo clientInfo = receiveClientInfo(clientSocket);
        cout << "ClientInfo received Successfully" << endl;
        cout << "ID = " << clientInfo.clientID << endl;
        cout << "Name = " << clientInfo.name << endl;
        cout << "Age = " << clientInfo.age << endl;
        cout << "Mobile Num = " << clientInfo.mobileNum << endl;
        cout << "NationalID = " << clientInfo.nationalID << endl;
        cout << "email = " << clientInfo.email << endl;

        storeClientInfo(clientInfo);

        cout << endl;
        cout << endl;

        cout << "Attempting to getAccountBalance from DB" << endl;
        double balance = getAccountBalance(clientInfo.clientID);
        cout << "Current Balance in account = " << balance << endl;
        cout << endl;

        cout << "Attempting to Deposit money in wallet" << endl;
        depositMoney(clientInfo.clientID, 5000.0);
        balance = getAccountBalance(clientInfo.clientID);
        cout << "Current Balance in account = " << balance << endl;
        cout << endl;

        cout << "Attempting to withdraw money available in wallet" << endl;
        withdrawMoney(clientInfo.clientID, 2000.0);
        balance = getAccountBalance(clientInfo.clientID);
        cout << "Current Balance in account = " << balance << endl;
        cout << endl;

        cout << "Attempting to withdraw money more than there in wallet" << endl;
        withdrawMoney(clientInfo.clientID, 7000.0);
        balance = getAccountBalance(clientInfo.clientID);
        cout << "Current Balance in account = " << balance << endl;
        cout << endl;

        cout << "Attempting to displayClientInfo from DB" << endl;
        displayClientInfo(clientInfo.clientID);


        while(true){

        }
        // Close the client socket
        close(clientSocket);
    } catch (const std::exception& e) {
        cerr << "Error in handling client connection: " << e.what() << endl;
    }
}

template <typename T>
void receiveMember(int clientSocket, T& member, const char* errorMessage) {
    int bytesRecv = recv(clientSocket, &member, sizeof(member), 0);
    if (bytesRecv < 0) {
        cerr << errorMessage << endl;
        close(clientSocket);
        return;
    }
}

string receiveString(int clientSocket) {
    int len;
    receiveMember(clientSocket, len, "Error receiving string length");

    if (len >= MAX_STRING_SIZE) {
        cerr << "String length exceeds buffer size: "<< len << endl;
        close(clientSocket);
        return "";
    }

    char buffer[MAX_STRING_SIZE];
    receiveMember(clientSocket, buffer, "Error receiving string data");

    return std::string(buffer, len);
}

bool checkClientExists(const int& clientID) {
    char* errMsg = nullptr;
    string sql = "SELECT EXISTS(SELECT 1 FROM clients WHERE clientID = ? LIMIT 1);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    bool exists = sqlite3_column_int(stmt, 0) != 0;

    sqlite3_finalize(stmt);

    return exists;
}

ClientInfo Server::receiveClientInfo(int clientSocket){
    ClientInfo info;
    cout << "Inside receiveClientInfo" << endl;
    receiveMember(clientSocket, info.clientID, "Error receiving Client's ID");

    // // Check if the clientID already exists in the database
    // bool clientExists = checkClientExists(info.clientID);
    // if (clientExists) {
    //     cerr << "Client with ID " << info.clientID << " already exists." << endl;
    //     // Handle the situation, e.g., reject the client info or update the existing record
    //     // For now, let's just return an empty info
    //     return info;
    // }

    info.name = receiveString(clientSocket);
    receiveMember(clientSocket, info.age, "Error receiving Client's Age");
    info.nationalID = receiveString(clientSocket);
    info.mobileNum = receiveString(clientSocket);
    info.email = receiveString(clientSocket);
    info.balance = 0;
    cout << "End of receiveClientInfo" << endl;
    return info;
}

double Server::getAccountBalance(const int& clientID){
    double balance = 0.0;
    char* errMsg = nullptr;

    // Construct the SQL query to select the balance for the given clientID
    string sql = "SELECT balance FROM clients WHERE clientID = ?;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return balance;
    }

    // Bind parameters to the statement
    sqlite3_bind_int(stmt, 1, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 0);
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    return balance;
}

void Server::depositMoney(const int& clientID, double amount){
    double currentBalance = getAccountBalance(clientID);
    double newBalance = currentBalance + amount;

    char* errMsg = nullptr;

    // Construct the SQL query to update the balance for the given clientID
    string sql = "UPDATE clients SET balance = ? WHERE clientID = ?;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind parameters to the statement
    sqlite3_bind_double(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

void Server::withdrawMoney(const int& clientID, double amount){
    double currentBalance = getAccountBalance(clientID);
    double newBalance = currentBalance - amount;

    if (newBalance < 0) {
        cerr << "Insufficient balance" << endl;
        return;
    }

    char* errMsg = nullptr;

    // Construct the SQL query to update the balance for the given clientID
    string sql = "UPDATE clients SET balance = ? WHERE clientID = ?;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind parameters to the statement
    sqlite3_bind_double(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}


void Server::processTransaction(const int& clientID, const Transaction& transaction){

}

void Server::undoTransaction(const int& clientID){

}

void Server::redoTransaction(const int& clientID){

}

void Server::displayClientInfo(const int& clientID){
    // Construct the SQL query
    string sql = "SELECT * FROM clients WHERE clientID = ?;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind parameters to the statement
    sqlite3_bind_int(stmt, 1, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "Client with ID " << clientID << " not found." << endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Read the client information from the query result
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 2);
    int age = sqlite3_column_int(stmt, 3);
    const unsigned char* nationalID = sqlite3_column_text(stmt, 4);
    const unsigned char* mobileNum = sqlite3_column_text(stmt, 5);
    const unsigned char* email = sqlite3_column_text(stmt, 6);
    double balance = sqlite3_column_double(stmt, 7);

    // Print the client information
    cout << "Client ID: " << id << endl;
    cout << "Name: " << name << endl;
    cout << "Age: " << age << endl;
    cout << "National ID: " << nationalID << endl;
    cout << "Mobile Number: " << mobileNum << endl;
    cout << "Email: " << email << endl;
    cout << "Balance: " << balance << endl;

    // Finalize the statement
    sqlite3_finalize(stmt);
}


void initDatabase(){
    int rc = sqlite3_open("wallet.db", &db);

    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS clients ("
                  "id INTEGER PRIMARY KEY,"
                  "clientID INTEGER UNIQUE,"
                  "name TEXT NOT NULL,"
                  "age INTEGER,"
                  "nationalID TEXT,"
                  "mobileNum TEXT,"
                  "email TEXT,"
                  "balance REAL"
                  ");";
    
    rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }
}

int main() {

    cout << "Server" << endl;
    initDatabase();
    
    Server server;
    server.start();

    // Accept incoming connections
    while (true) {
        //TODO
    }

    server.stop();

    return 0;
}