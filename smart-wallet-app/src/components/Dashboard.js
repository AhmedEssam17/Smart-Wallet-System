import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { useNavigate } from 'react-router-dom';

function Dashboard() {
  const [clientID, setClientID] = useState(localStorage.getItem('clientID') || '');
  const [balance, setBalance] = useState(0);
  const [amount, setAmount] = useState(0);
  const navigate = useNavigate();
  const [recipientID, setRecipientID] = useState('');
  const [transactionAmount, setTransactionAmount] = useState('');
  const [clientInfo, setClientInfo] = useState({});

  useEffect(() => {
    if (!clientID) {
      console.error("No client ID found. Please log in.");
      navigate('/login'); // Redirect to Login page
    }
  }, [clientID, navigate]);

  const fetchClientInfo = async () => {
    try {
      const response = await axios.get(`http://localhost:5000/api/clientInfo/${clientID}`);
      setClientInfo(response.data);
    } catch (error) {
      console.error('Failed to fetch client info:', error);
    }
  };

  const fetchBalance = async () => {
    try {
      console.log("called")
      const response = await axios.get(`http://localhost:5000/api/balance/${clientID}`);
      setBalance(response.data.balance);
    } catch (error) {
      console.error('Error fetching balance:', error);
    }
  };

  const handleDeposit = async () => {
    try {
      const depositResponse = await axios.post('http://localhost:5000/api/deposit', { clientID, amount });
      if (depositResponse.data.status === "success") {
        await fetchBalance(); // Update balance after successful deposit
        setAmount('');
      } else {
        alert(depositResponse.data.message);
      }
    } catch (error) {
      console.error('Error during deposit:', error);
    }
  };

  const handleWithdraw = async () => {
    try {
      const withdrawResponse = await axios.post('http://localhost:5000/api/withdraw', { clientID, amount });
      if (withdrawResponse.data.status === "success") {
        await fetchBalance(); // Update balance after successful withdrawal
        setAmount('');
      } else {
        alert(withdrawResponse.data.message);
      }
    } catch (error) {
      console.error('Error withdrawing money:', error);
    }
  };

  const handleTransaction = async () => {
    try {
      const transactionResponse = await axios.post('http://localhost:5000/api/transaction', {
        fromClientID: clientID,
        toClientID: recipientID,
        amount: transactionAmount
      });
      if (transactionResponse.data.status === "success") {
        alert(transactionResponse.data.message);
        await fetchBalance(); // Update balance after successful transaction
        setTransactionAmount('');
        setRecipientID('');
      } else {
        alert(transactionResponse.data.message);
      }
    } catch (error) {
      console.error('Transaction failed:', error);
    }
  };

  return (
    <div className="container mt-5">
      <h1 style={{ textAlign: 'center' }}>Dashboard</h1>
      <div className="mb-3">
        <h2>Balance</h2>
        <button className="btn btn-info" onClick={fetchBalance}>Check Balance</button>
        <p>Balance: {balance}</p>
      </div>
      <div className="mb-3">
        <input type="number" className="form-control" placeholder="Amount" value={amount} onChange={e => setAmount(e.target.value)} />
        <button className="btn btn-success" onClick={handleDeposit}>Deposit Money</button>
        <button className="btn btn-danger" onClick={handleWithdraw}>Withdraw Money</button>
      </div>
      
      <div className="info-section">
        <h2>Client Info</h2>
        <button className="btn btn-info" onClick={fetchClientInfo}>Show My Info</button>
        <div>
          <p>Client ID: {clientInfo.clientID}</p>
          <p>Name: {clientInfo.name}</p>
          <p>Age: {clientInfo.age}</p>
          <p>Mobile Number: {clientInfo.mobileNum}</p>
          <p>National ID: {clientInfo.nationalID}</p>
          <p>Email: {clientInfo.email}</p>
        </div>
      </div>

      <div className="transaction-section">
        <h2>Transactions</h2>
        <input type="text" className="form-control" placeholder="Recipient ID" value={recipientID} onChange={e => setRecipientID(e.target.value)} />
        <input type="number" className="form-control" placeholder="Amount" value={transactionAmount} onChange={e => setTransactionAmount(e.target.value)} />
        <button className="btn btn-primary" onClick={handleTransaction}>Send Money</button>
        </div>
    </div>
    );
}
export default Dashboard;