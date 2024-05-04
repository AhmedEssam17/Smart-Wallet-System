#define PORT 8888
#define SERVERADDR "127.0.0.1"

#define MAX_NAME_SIZE 30
#define MAX_NATIONAL_ID_SIZE 15
#define MAX_MOBILE_SIZE 11
#define MAX_EMAIL_SIZE 40
#define MAX_STRING_SIZE 50


struct ClientInfo {
    int clientID;
    std::string name;
    int age;
    std::string nationalID;
    std::string mobileNum;
    std::string email;
    double balance;
};

struct Transaction {
    std::string fromAccount;
    std::string toAccount;
    double amount;
};