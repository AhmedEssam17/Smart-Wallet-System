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
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

sqlite3* db;

class Server {
public:
    Server();
    ~Server();

    void start();
    void stop();

    int getAccountBalance(const int& clientID);
    int depositMoney(const int& clientID, int amount);
    int withdrawMoney(const int& clientID, int amount);
    int processTransaction(const int& clientID, const Transaction& transaction);
    int undoTransaction(int transactionID);
    int redoTransaction(int transactionID);
    ClientInfo displayClientInfo(const int& clientID);

private:
    int serverSocket;
    void initSocket();
    void closeSocket();
    void listenForConnections();
    void handleConnection(int clientSocket);
    int receiveClientInfo(int clientSocket);
    int receiveTransaction(int clientSocket, const int& clientID);
    int verifyClient(int clientSocket);
    int registerClient(int clientSocket);

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

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cerr << "Error accepting client connection" << endl;
            continue;
        }

        std::thread clientThread(&Server::handleConnection, this, clientSocket);
        clientThread.detach();
    }
}

int storeClientInfo(const ClientInfo& info) {
    char* errMsg = nullptr;

    string sql = "INSERT INTO clients (clientID, name, age, nationalID, mobileNum, email, balance) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return 0;
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
        return 0;
    } else {
        cout << "Client's Data is stored in the Database Successfully" << endl;
    }

    sqlite3_finalize(stmt);

    return 1;
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
        int response = 0;
        int transactionID;
        ClientInfo clientInfo;

        while(true){
            action = 0;
            recv(clientSocket, &action, sizeof(action), 0);
            switch (action){
                case LOGIN:
                    cout << "Attempting Login..." << endl;
                    clientInfo.clientID = verifyClient(clientSocket);
                    if(clientInfo.clientID == 0){
                        response = 0;
                    }
                    else{
                        response = 1;
                    }
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case REGISTER:
                    cout << "Attempting Registeration..." << endl;
                    clientInfo.clientID = registerClient(clientSocket);
                    if(clientInfo.clientID != 0){
                        response = 1;
                    }
                    else{
                        response = 0;
                    }
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case DISPLAYINFO:
                    cout << "Attempting Displaying Info..." << endl;
                    clientInfo = displayClientInfo(clientInfo.clientID);
                    send(clientSocket, &clientInfo, sizeof(clientInfo), 0);
                break;
                case BALANCE:
                    cout << "Attempting Get Balance..." << endl;
                    clientInfo.balance = getAccountBalance(clientInfo.clientID);
                    send(clientSocket, &clientInfo.balance, sizeof(clientInfo.balance), 0);
                break;
                case DEPOSIT:
                    cout << "Attempting Deposit Money..." << endl;
                    recv(clientSocket, &networkAmount, sizeof(networkAmount), 0);
                    amount = ntohl(networkAmount);
                    response = depositMoney(clientInfo.clientID, amount);
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case WITHDRAW:
                    cout << "Attempting Withdraw Money..." << endl;
                    recv(clientSocket, &networkAmount, sizeof(networkAmount), 0);
                    amount = ntohl(networkAmount);
                    response = withdrawMoney(clientInfo.clientID, amount);
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case TRANSACTION:
                    cout << "Attempting Transaction..." << endl;
                    response = receiveTransaction(clientSocket, clientInfo.clientID);
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case UNDO:
                    cout << "Attempting Undo Transaction..." << endl;
                    recv(clientSocket, &transactionID, sizeof(transactionID), 0);
                    response = undoTransaction(transactionID);
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                case REDO:
                    cout << "Attempting Redo Transaction..." << endl;
                    recv(clientSocket, &transactionID, sizeof(transactionID), 0);
                    response = redoTransaction(transactionID);
                    send(clientSocket, &response, sizeof(response), 0);
                break;
                default:
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

int Server::receiveClientInfo(int clientSocket) {
    ClientInfo info;
    recv(clientSocket, &info, sizeof(info), 0);
    return storeClientInfo(info);
}

int Server::getAccountBalance(const int& clientID){
    int balance = 0;
    char* errMsg = nullptr;

    string sql = "SELECT balance FROM clients WHERE clientID = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return balance;
    }

    sqlite3_bind_int(stmt, 1, clientID);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        balance = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return balance;
}

int Server::depositMoney(const int& clientID, int amount){

    int currentBalance = getAccountBalance(clientID);
    int newBalance = currentBalance + amount;

    char* errMsg = nullptr;

    string sql = "UPDATE clients SET balance = ? WHERE clientID = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

    sqlite3_bind_int(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, clientID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int Server::withdrawMoney(const int& clientID, int amount){
    int currentBalance = getAccountBalance(clientID);
    int newBalance = currentBalance - amount;

    if (newBalance < 0) {
        cerr << "Insufficient balance" << endl;
        return 0;
    }

    char* errMsg = nullptr;

    string sql = "UPDATE clients SET balance = ? WHERE clientID = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

    sqlite3_bind_int(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, clientID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL execute error: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int Server::processTransaction(const int& clientID, const Transaction& transaction){
    int depositResponse;
    int withdrawResponse;

    if (transaction.fromAccount != clientID) {
        cerr << "Invalid transaction: fromAccount does not match clientID" << endl;
        return 0;
    }


    bool toAccountExists = checkClientExists(transaction.toAccount);
    if (!toAccountExists) {
        cerr << "Invalid transaction: toAccount does not exist" << endl;
        return 0;
    }

    int balance = getAccountBalance(clientID);
    if (balance < transaction.amount) {
        cerr << "Invalid transaction: insufficient balance" << endl;
        return 0;
    }

    withdrawResponse = withdrawMoney(clientID, transaction.amount);
    depositResponse = depositMoney(transaction.toAccount, transaction.amount);

    if(depositResponse == 0 || withdrawResponse == 0){
        return 0;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    string sql = "INSERT INTO transactions (sender_id, recipient_id, amount) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transaction.fromAccount);
    sqlite3_bind_int(stmt, 2, transaction.toAccount);
    sqlite3_bind_int(stmt, 3, transaction.amount);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error inserting transaction into database: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    int transactionID = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

    cout << "Transaction processed successfully, ID: " << transactionID << endl;
    return transactionID;
}

int Server::undoTransaction(int transactionID) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    string sql = "SELECT sender_id, recipient_id, amount FROM transactions WHERE transaction_id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cerr << "Error retrieving transaction details: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    int senderId = sqlite3_column_int(stmt, 0);
    int recipientId = sqlite3_column_int(stmt, 1);
    int amount = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);

    if (depositMoney(senderId, amount) == 0 || withdrawMoney(recipientId, amount) == 0) {
        cerr << "Error reversing balances" << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return 0;
    }

    sql = "INSERT INTO undone_transactions SELECT * FROM transactions WHERE transaction_id = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error moving transaction to undone_transactions: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);

    sql = "DELETE FROM transactions WHERE transaction_id = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error deleting transaction: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

int Server::redoTransaction(int transactionID) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    string sql = "SELECT sender_id, recipient_id, amount FROM undone_transactions WHERE transaction_id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cerr << "Error retrieving transaction details: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    int senderId = sqlite3_column_int(stmt, 0);
    int recipientId = sqlite3_column_int(stmt, 1);
    int amount = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);

    if (withdrawMoney(senderId, amount) == 0 || depositMoney(recipientId, amount) == 0) {
        cerr << "Error reapplying balances" << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return 0;
    }

    sql = "INSERT INTO transactions SELECT * FROM undone_transactions WHERE transaction_id = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error moving transaction back to transactions: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);

    sql = "DELETE FROM undone_transactions WHERE transaction_id = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt,1, transactionID);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error deleting transaction from undone_transactions: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

int Server::receiveTransaction(int clientSocket, const int& clientID) {
    Transaction transaction;
    recv(clientSocket, &transaction, sizeof(transaction), 0);
    return processTransaction(clientID, transaction);
}

int Server::registerClient(int clientSocket){
    int clientID;
    recv(clientSocket, &clientID, sizeof(clientID), 0);
    int password;
    recv(clientSocket, &password, sizeof(password), 0);

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
    int response = receiveClientInfo(clientSocket);
    if(response == 0){
        clientID = 0;
        cout << "Client Registeration failed" << endl;
        return clientID;
    }
    cout << "Client Registered Successfully" << endl;
    return clientID;
}

ClientInfo Server::displayClientInfo(const int& clientID){
    ClientInfo info;
    info.clientID = 0;

    string sql = "SELECT * FROM clients WHERE clientID = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL prepare error: " << sqlite3_errmsg(db) << endl;
        return info;
    }

    sqlite3_bind_int(stmt, 1, clientID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "Client with ID " << clientID << " not found." << endl;
        sqlite3_finalize(stmt);
        return info;
    }

    int id = sqlite3_column_int(stmt, 1);
    const unsigned char* name = sqlite3_column_text(stmt, 2);
    int age = sqlite3_column_int(stmt, 3);
    const unsigned char* nationalID = sqlite3_column_text(stmt, 4);
    const unsigned char* mobileNum = sqlite3_column_text(stmt, 5);
    const unsigned char* email = sqlite3_column_text(stmt, 6);
    int balance = sqlite3_column_int(stmt, 7);

    info.clientID = id;
    strcpy(info.name, reinterpret_cast<const char*>(name));
    info.age = age;
    strcpy(info.nationalID, reinterpret_cast<const char*>(nationalID));
    strcpy(info.mobileNum, reinterpret_cast<const char*>(mobileNum));
    strcpy(info.email, reinterpret_cast<const char*>(email));
    info.balance = balance;

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

    const char* transactionSql = "CREATE TABLE IF NOT EXISTS transactions ("
                    "transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "sender_id INTEGER,"
                    "recipient_id INTEGER,"
                    "amount INTEGER,"
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
                    ");";
    
    rc = sqlite3_exec(db, transactionSql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* undoneTransactionSql = "CREATE TABLE IF NOT EXISTS undone_transactions ("
                    "transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "sender_id INTEGER,"
                    "recipient_id INTEGER,"
                    "amount INTEGER,"
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
                    ");";
    
    rc = sqlite3_exec(db, undoneTransactionSql, nullptr, nullptr, nullptr);
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
    while (true) {}

    server.stop();

    return 0;
}