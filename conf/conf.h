#define PORT 8888
#define SERVERADDR "127.0.0.1"

struct ClientInfo {
    int clientID;
    char name[30];
    int age;
    char nationalID[20];
    char mobileNum[15];
    char email[40];
    double balance;
};

struct Transaction {
    int fromAccount;
    int toAccount;
    double amount;
};