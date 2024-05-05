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
#include <vector>
#include <arpa/inet.h>
using namespace std;

sqlite3* db;

class Server {
public:
    Server();
    ~Server();

    void start();
    void stop();

    int getAccountBalance(const int& clientID);
    void depositMoney(const int& clientID, int amount);
    void withdrawMoney(const int& clientID, int amount);
    void processTransaction(const int& clientID, const Transaction& transaction);
    void undoTransaction(const int& clientID);
    void redoTransaction(const int& clientID);
    ClientInfo displayClientInfo(const int& clientID);

private:
    int serverSocket;
    std::mutex dbMutex;
    void initSocket();
    void closeSocket();
    void listenForConnections();
    void handleConnection(int clientSocket);
    void receiveClientInfo(int clientSocket);
    void receiveTransaction(int clientSocket, const int& clientID);
    int verifyClient(int clientSocket);
    int registerClient(int clientSocket);

    std::vector<Transaction> transactionHistory;

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
        std::thread clientThread(&Server::handleConnection, this, clientSocket);
        clientThread.detach(); // Detach the thread to let it run independently
        clientCount++;
        cout << "clientCount = " << clientCount << endl;
        // handleConnection(clientSocket);
    }
}

void storeClientInfo(const ClientInfo& info) {
    char* errMsg = nullptr;

    string sql = "INSERT INTO clients (clientID, name, age, nationalID, mobileNum, email, balance) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, info.clientID);
    sqlite3_bind_text(stmt, 2, info.name, strlen(info.name), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, info.age);
    sqlite3_bind_text(stmt, 4, info.nationalID, strlen(info.nationalID), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, info.mobileNum, strlen(info.mobileNum), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, info.email, strlen(info.email), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, info.balance);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Client's Data is stored in the Database Successfully" << endl;
    }

    sqlite3_finalize(stmt);
}

int Server::verifyClient(int clientSocket){
    int networkClientID;
    recv(clientSocket, &networkClientID, sizeof(networkClientID), 0);
    int clientID = ntohl(networkClientID);

    int networkPassword;
    recv(clientSocket, &networkPassword, sizeof(networkPassword), 0);
    int password = ntohl(networkPassword);

    // Query the client_passwords table
    string sql = "SELECT password FROM client_passwords WHERE clientID = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }
    sqlite3_bind_int(stmt, 1, clientID);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "ClientID hasn't registered yet" << endl;
        return 0;
    }
    int storedPassword = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (storedPassword != password) {
        cerr << "Invalid password" << endl;
        return 0;
    }

    cout << "Client Logged in Successfully" << endl;
    return clientID;
}

void Server::handleConnection(int clientSocket) {
    try {
        if (clientSocket < 0) {
            cerr << "Invalid client socket" << endl;
            return;
        }

        int action = 0;
        int networkAmount;
        int amount;
        ClientInfo clientInfo;

        while(true){
            action = 0;
            recv(clientSocket, &action, sizeof(action), 0);
            switch (action){
                case LOGIN:
                    cout << "Client Attempting verifyClient()" << endl;
                    clientInfo.clientID = verifyClient(clientSocket);
                break;
                case REGISTER:
                    cout << "Client Attempting receiveClientInfo()" << endl;
                    clientInfo.clientID = registerClient(clientSocket);
                break;
                case DISPLAYINFO:
                    cout << "Client Attempting displayClientInfo()" << endl;
                    clientInfo = displayClientInfo(clientInfo.clientID);
                    send(clientSocket, &clientInfo, sizeof(clientInfo), 0);
                break;
                case BALANCE:
                    cout << "Client Attempting getAccountBalance()" << endl;
                    clientInfo.balance = getAccountBalance(clientInfo.clientID);
                    send(clientSocket, &clientInfo.balance, sizeof(clientInfo.balance), 0);
                break;
                case DEPOSIT:
                    cout << "Client Attempting depositMoney()" << endl;
                    recv(clientSocket, &networkAmount, sizeof(networkAmount), 0);
                    cout << "networkAmount = "<< networkAmount << endl;
                    amount = ntohl(networkAmount);
                    cout << "amount = "<< amount << endl;
                    depositMoney(clientInfo.clientID, amount);
                break;
                case WITHDRAW:
                    cout << "Client Attempting withdrawMoney()" << endl;
                    recv(clientSocket, &networkAmount, sizeof(networkAmount), 0);
                    amount = ntohl(networkAmount);
                    withdrawMoney(clientInfo.clientID, amount);
                break;
                case TRANSACTION:
                    cout << "Client Attempting receiveTransaction()" << endl;
                    receiveTransaction(clientSocket, clientInfo.clientID);
                break;
                case UNDO:
                    cout << "Client Attempting undoTransaction()" << endl;
                    undoTransaction(clientInfo.clientID);
                break;
                case REDO:
                    cout << "Client Attempting redoTransaction()" << endl;
                    redoTransaction(clientInfo.clientID);
                break;
                default:
                    // cout << "Invalid Action" << endl;
                break;
            }
        }
        // Close the client socket
        close(clientSocket);
    } catch (const std::exception& e) {
        cerr << "Error in handling client connection: " << e.what() << endl;
    }
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

void Server::receiveClientInfo(int clientSocket) {
    ClientInfo info;
    recv(clientSocket, &info, sizeof(info), 0);
    storeClientInfo(info);
}

int Server::getAccountBalance(const int& clientID){
    int balance = 0.0;
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
        balance = sqlite3_column_int(stmt, 0);
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    return balance;
}

void Server::depositMoney(const int& clientID, int amount){

    cout << "Deposit amount = " << amount << endl;

    int currentBalance = getAccountBalance(clientID);
    int newBalance = currentBalance + amount;

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
    sqlite3_bind_int(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

void Server::withdrawMoney(const int& clientID, int amount){
    int currentBalance = getAccountBalance(clientID);
    int newBalance = currentBalance - amount;

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
    sqlite3_bind_int(stmt, 1, newBalance);
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
    // Check if the fromAccount is equal to clientID
    if (transaction.fromAccount != clientID) {
        cerr << "Invalid transaction: fromAccount does not match clientID" << endl;
        return;
    }

    // Check if the toAccount exists in the database
    bool toAccountExists = checkClientExists(transaction.toAccount);
    if (!toAccountExists) {
        cerr << "Invalid transaction: toAccount does not exist" << endl;
        return;
    }

    // Check if the amount required to be transferred is sufficient in the current user balance
    int balance = getAccountBalance(clientID);
    if (balance < transaction.amount) {
        cerr << "Invalid transaction: insufficient balance" << endl;
        return;
    }

    // Process the transaction
    // For example, update the balances in the database
    // Withdraw amount from the client's account
    withdrawMoney(clientID, transaction.amount);

    // Deposit amount to the recipient's account
    depositMoney(transaction.toAccount, transaction.amount);

    cout << "Transaction processed successfully" << endl;

    transactionHistory.push_back(transaction);
}

void Server::undoTransaction(const int& clientID){
    if (!transactionHistory.empty()) {
        // Undo the last transaction
        Transaction lastTransaction = transactionHistory.back();
        depositMoney(lastTransaction.fromAccount, lastTransaction.amount);
        withdrawMoney(lastTransaction.toAccount, lastTransaction.amount);

        // Remove the last transaction from history
        transactionHistory.pop_back();

        cout << "Transaction undone successfully" << endl;
    } else {
        cout << "No transaction to undo" << endl;
    }
}

void Server::redoTransaction(const int& clientID){

}

void Server::receiveTransaction(int clientSocket, const int& clientID) {
    Transaction transaction;
    recv(clientSocket, &transaction, sizeof(transaction), 0);
    processTransaction(clientID, transaction);
}

int Server::registerClient(int clientSocket){
    int clientID;
    recv(clientSocket, &clientID, sizeof(clientID), 0);
    int password;
    recv(clientSocket, &password, sizeof(password), 0);

    // Check if clientID already exists
    if (checkClientExists(clientID)){
        cout << "ClientID has already registered before" << endl;
        return 0;
    }

    // Insert into client_passwords table
    string sql = "INSERT INTO client_passwords (clientID, password) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }
    sqlite3_bind_int(stmt, 1, clientID);
    sqlite3_bind_int(stmt, 2, password);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }
    sqlite3_finalize(stmt);

    // Register the client info
    receiveClientInfo(clientSocket);
    cout << "Client Registered Successfully" << endl;
    return clientID;
}

ClientInfo Server::displayClientInfo(const int& clientID){
    ClientInfo info;

    // Construct the SQL query
    string sql = "SELECT * FROM clients WHERE clientID = ?;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return info;
    }

    // Bind parameters to the statement
    sqlite3_bind_int(stmt, 1, clientID);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "Client with ID " << clientID << " not found." << endl;
        sqlite3_finalize(stmt);
        return info;
    }

    // Read the client information from the query result
    int id = sqlite3_column_int(stmt, 1);
    const unsigned char* name = sqlite3_column_text(stmt, 2);
    int age = sqlite3_column_int(stmt, 3);
    const unsigned char* nationalID = sqlite3_column_text(stmt, 4);
    const unsigned char* mobileNum = sqlite3_column_text(stmt, 5);
    const unsigned char* email = sqlite3_column_text(stmt, 6);
    int balance = sqlite3_column_int(stmt, 7);

    // Print the client information
    cout << "Client ID: " << id << endl;
    cout << "Name: " << name << endl;
    cout << "Age: " << age << endl;
    cout << "National ID: " << nationalID << endl;
    cout << "Mobile Number: " << mobileNum << endl;
    cout << "Email: " << email << endl;
    cout << "Balance: " << balance << endl;

    // Copy the client information into the info struct
    info.clientID = id;
    strcpy(info.name, reinterpret_cast<const char*>(name));
    info.age = age;
    strcpy(info.nationalID, reinterpret_cast<const char*>(nationalID));
    strcpy(info.mobileNum, reinterpret_cast<const char*>(mobileNum));
    strcpy(info.email, reinterpret_cast<const char*>(email));
    info.balance = balance;

    // Finalize the statement
    sqlite3_finalize(stmt);

    return info;
}



void initDatabase(){

    int rc = sqlite3_open("wallet.db", &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* clientSql = "CREATE TABLE IF NOT EXISTS clients ("
                  "id INTEGER PRIMARY KEY,"
                  "clientID INTEGER UNIQUE,"
                  "name TEXT NOT NULL,"
                  "age INTEGER,"
                  "nationalID TEXT,"
                  "mobileNum TEXT,"
                  "email TEXT,"
                  "balance INTEGER"
                  ");";
    
    rc = sqlite3_exec(db, clientSql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* passSql = "CREATE TABLE IF NOT EXISTS client_passwords ("
                    "clientID INTEGER PRIMARY KEY,"
                    "password INTEGER"
                    ");";
    
    rc = sqlite3_exec(db, passSql, nullptr, nullptr, nullptr);
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