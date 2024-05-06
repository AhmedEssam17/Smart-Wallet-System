# Smart Wallet System

## Server

1. Compile the server:
   ```bash
   cd Server
   g++ server.cpp -o ../exe/server -lsqlite3 -pthread

# Smart Wallet System
This README provides detailed instructions on how to set up and run the different components of the Smart Wallet System. The system is divided into four main parts: the C++ server, the C++ client, the Node.js server, and the React UI application. Follow these steps to get each component up and running.

#### Prerequisites
##### Before you begin, ensure you have the following installed:
#####   1) GCC Compiler for C++
#####   2) Node.js and npm
#####   3) SQLite3 (if interacting with a SQLite database)

#### Running the Project

##### C++ Server
Navigate to the Server directory within the Smart-Wallet-System and compile and run the server using the following commands:
```bash
cd Smart-Wallet-System/Server
g++ server.cpp -o ../exe/server -lsqlite3 -pthread
../exe/server > logs.txt
```
This will compile the server.cpp file and output the executable to the exe directory. The server will run and redirect its logs to logs.txt.

##### C++ Client
Open a new terminal and set up the C++ client by navigating to the Client directory, compiling, and running the client:
```bash
cd Smart-Wallet-System/Client
g++ -o ../exe/client client.cpp
../exe/client
```

##### C++ Server
Navigate to the Server directory within the Smart-Wallet-System and compile and run the server using the following commands:
```bash
cd Smart-Wallet-System/Server
g++ server.cpp -o ../exe/server -lsqlite3 -pthread
../exe/server > logs.txt
```
This compiles the client.cpp and runs the resulting executable, allowing it to communicate with the C++ server.

##### NodeJS Server
To start the Node.js server, navigate to the server directory under the smart-wallet-app and run the server using Node.js:
```bash
cd Smart-Wallet-System/smart-wallet-app/server
node index.js
```
This starts the Node.js server which acts as a middleware handling requests between the React UI and the C++ backend.

##### React UI App
Finally, to run the React application, navigate to the smart-wallet-app directory and start the application:
```bash
cd Smart-Wallet-System/smart-wallet-app
npm start
```
This command will start the React development server and open the application in your default web browser at http://localhost:3000.


#### Conclusion
By following these steps, you should have all components of the Smart Wallet System running. The system is now fully operational, and you can begin interacting with the UI to perform transactions and other account management tasks. Ensure all parts are running simultaneously in different terminals to maintain full functionality.
