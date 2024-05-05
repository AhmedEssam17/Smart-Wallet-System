import React, { useState } from 'react';
import axios from 'axios';

function Dashboard({ clientID }) {
    console.log(clientID)
  const [balance, setBalance] = useState(0);
  const [amount, setAmount] = useState(0);

  const fetchBalance = async () => {
    try {
      const response = await axios.get(`http://localhost:5000/api/balance/${clientID}`);
      setBalance(response.data.balance);
    } catch (error) {
      console.error('Error fetching balance:', error);
    }
  };

  const handleDeposit = async () => {
    try {
        console.log('amount is, ',amount)
      await axios.post('http://localhost:5000/api/deposit', { clientID, amount });
      fetchBalance();
      setAmount(0);
    } catch (error) {
      console.error('Error depositing money:', error);
    }
  };

  const handleWithdraw = async () => {
    try {
      await axios.post('http://localhost:5000/api/withdraw', { clientID, amount });
      fetchBalance();
      setAmount('');
    } catch (error) {
      console.error('Error withdrawing money:', error);
    }
  };

  return (
    <div className="container mt-5">
      <h1>Dashboard</h1>
      <div className="mb-3">
        <button className="btn btn-info" onClick={fetchBalance}>Check Balance</button>
        <p>Balance: {balance}</p>
      </div>
      <div className="mb-3">
        <input type="number" className="form-control" placeholder="Amount" value={amount} onChange={e => {
            console.log(e.target.value)
            return setAmount(e.target.value)
            }} />
      </div>
      <div className="mb-3">
        <button className="btn btn-success" onClick={handleDeposit}>Deposit Money</button>
        <button className="btn btn-danger" onClick={handleWithdraw}>Withdraw Money</button>
      </div>
    </div>
  );
}

export default Dashboard;