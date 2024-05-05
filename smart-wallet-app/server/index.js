const net = require('net');
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
const port = 5000;

app.use(cors());
app.use(bodyParser.json());

// Setup TCP client
const client = new net.Socket();
client.connect(8080, '127.0.0.1', () => {
    console.log('Connected to C++ client');
});

client.on('data', (data) => {
    console.log('Received: ' + data.toString());
});

client.on('close', () => {
    console.log('Connection closed');
});

// Example of handling a login request
app.post('/api/login', (req, res) => {
    const { clientID, password } = req.body;
    client.write(`login ${clientID} ${password}\n`);

    // Setup event listener for receiving data
    client.once('data', (data) => {
        res.send(data.toString());
    });
});

// Additional routes for register, balance, deposit, withdraw, etc.
// Follow similar pattern as the login route
// Register route
app.post('/api/register', (req, res) => {
    const { clientID, password, clientInfo } = req.body;
    const infoString = `${clientInfo.name} ${clientInfo.age} ${clientInfo.mobileNum} ${clientInfo.nationalID} ${clientInfo.email} ${clientInfo.balance}`;
    client.write(`register ${clientID} ${password} ${infoString}\n`);

    client.once('data', (data) => {
        res.send(data.toString());
    });
});

// Balance route
app.get('/api/balance/:clientID', (req, res) => {
    const { clientID } = req.params;
    client.write(`balance ${clientID}\n`);

    client.once('data', (data) => {
        res.send({ balance: data.toString() });
    });
});

// Deposit route
app.post('/api/deposit', (req, res) => {
    const { clientID, amount } = req.body;
    client.write(`deposit ${clientID} ${amount}\n`);

    client.once('data', (data) => {
        res.send(data.toString());
    });
});

// Withdraw route
app.post('/api/withdraw', (req, res) => {
    const { clientID, amount } = req.body;
    client.write(`withdraw ${clientID} ${amount}\n`);

    client.once('data', (data) => {
        res.send(data.toString());
    });
});

// Additional error handling and server setup
client.on('error', (err) => {
    console.error('Error with C++ client connection:',err);
});

client.on('end', () => {
console.log('Disconnected from C++ client');
});

app.listen(port, () => {
    console.log(`Server running on port ${port}`);
});