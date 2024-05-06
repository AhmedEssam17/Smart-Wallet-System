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
        const response = data.toString();
        if (response === "success") {
            res.send({ status: "success", message: "Login successful" });
        } else {
            res.status(500).send({ status: "error", message: "Login failed" });
        }
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
        const response = data.toString();
        console.log("Response from C++ client:", response); // Log the raw response
        if (response === "success") {
          res.send({ status: "success", message: "Register successful" });
        } else {
          res.status(500).send({ status: "error", message: "Register failed" });
        }
      });
});

// Balance route
app.get('/api/balance/:clientID', (req, res) => {
    const { clientID } = req.params;
    console.log(`Request for balance received for clientID: ${clientID}`); // Log clientID
    client.write(`balance ${clientID}\n`);

    client.once('data', (data) => {
        res.send({ balance: data.toString() });
    });

    client.on('error', (error) => {
        console.error('TCP Error:', error);
        res.status(500).send('Error in TCP communication');
    });
});

// Deposit route
app.post('/api/deposit', (req, res) => {
    const { clientID, amount } = req.body;
    client.write(`deposit ${clientID} ${amount}\n`);

    client.once('data', (data) => {
        const response = data.toString();
        if (response === "success") {
            res.send({ status: "success", message: "Deposit successful" });
        } else {
            res.status(500).send({ status: "error", message: "Deposit failed" });
        }
    });
});



// Withdraw route
app.post('/api/withdraw', (req, res) => {
    const { clientID, amount } = req.body;
    client.write(`withdraw ${clientID} ${amount}\n`);

    client.once('data', (data) => {
        const response = data.toString();
        if (response === "success") {
            res.send({ status: "success", message: "Withdraw successful" });
        } else {
            res.status(500).send({ status: "error", message: "Withdraw failed" });
        }
    });
});

// Display Info Route
app.get('/api/clientInfo/:clientID', (req, res) => {
    const { clientID } = req.params;
    client.write(`displayInfo ${clientID}\n`);

    client.once('data', (data) => {
        const dataString = data.toString();
        try {
            const info = JSON.parse(dataString); // Directly parse the JSON string
            res.send(info);
        } catch (error) {
            console.error('Failed to parse client info:', error);
            res.status(500).send({ error: "Failed to parse client info" });
        }
    });
});

// Send Transaction Route
app.post('/api/transactions/', async (req, res) => {
    const { fromClientID, toClientID, amount } = req.body;
    // Format the data as a space-separated string
    client.write(`sendTransaction ${fromClientID} ${toClientID} ${amount}\n`);

    client.once('data', (data) => {
        const transactionId = data.toString().trim();
        if (transactionId !== "0") {
            res.send({ status: "success", message: "Transaction processed successfully", transactionId });
        } else {
            res.status(500).send({ status: "error", message: "Transaction processing failed" });
        }
    });
});

app.post('/api/transactions/undo', (req, res) => {
    const { undoId } = req.body;
    client.write(`undo ${undoId}\n`);

    client.once('data', (data) => {
        const response = data.toString().trim();
        if (response === "success") {
            res.send({ status: "success", message: "Undo successful", undoId: 0, redoId: undoId });
        } else {
            res.status(500).send({ status: "error", message: "Undo failed" });
        }
    });
});

app.post('/api/transactions/redo', async (req, res) => {
    const { redoId } = req.body;
    client.write(`redo ${redoId}\n`);

    client.once('data', (data) => {
        const response = data.toString();
        if (response === "success") {
            res.send({ status: "success", message: "Redo successful" });
        } else {
            res.status(500).send({ status: "error", message: "Redo failed" });
        }
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