#define PORT 8888
#define SERVERADDR "127.0.0.1"

struct ClientInfo {
    int clientID;
    char name[30];
    int age;
    char nationalID[20];
    char mobileNum[15];
    char email[40];
    int balance;
};

struct Transaction {
    int fromAccount;
    int toAccount;
    int amount;
};

struct TransactionTable {
    int transactionID;
    int fromAccount;
    int toAccount;
    int amount;
};

enum ACTIONS { 
    LOGIN = 1,
    REGISTER,
    DISPLAYINFO,
    BALANCE,
    DEPOSIT,
    WITHDRAW,
    TRANSACTION,
    UNDO,
    REDO
};

ACTIONS actions;