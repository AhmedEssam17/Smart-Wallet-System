#define PORT 8888
#define SERVERADDR "127.0.0.1"

#define MAX_NAME_SIZE 30
#define MAX_NATIONAL_ID_SIZE 15
#define MAX_MOBILE_SIZE 11
#define MAX_EMAIL_SIZE 40
#define MAX_STRING_SIZE 50


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