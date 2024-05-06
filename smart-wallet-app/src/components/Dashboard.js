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
    const [transactions, setTransactions] = useState([]);

    const [activeTransactions, setActiveTransactions] = useState([]);
    const [undoneTransactions, setUndoneTransactions] = useState([]);

    const [transactionIdToModify, setTransactionIdToModify] = useState('');

    // useEffect(() => {
    //     const fetchActiveTransactions = async () => {
    //         const response = await fetch(`/api/transactions/active/${clientID}`);
    //         const data = await response.json();
    //         setActiveTransactions(data);
    //     };

    //     const fetchUndoneTransactions = async () => {
    //         const response = await fetch(`/api/transactions/undone/${clientID}`);
    //         const data = await response.json();
    //         setUndoneTransactions(data);
    //     };

    //     fetchActiveTransactions();
    //     fetchUndoneTransactions();

    //     // setInterval(async () => {
    //     //     // Fetch or recalculate active and undone transactions
    //     //     activeTransactions = await fetchActiveTransactions();
    //     //     undoneTransactions = await fetchUndoneTransactions();
    //     // }, 1000); // Update every 1000 milliseconds (1 second)
    // }, [clientID]);

  useEffect(() => {
    if (!clientID) {
      console.error("No client ID found. Please log in.");
      navigate('/login'); // Redirect to Login page
    }
  }, [clientID, navigate]);
  

    const fetchTransactions = async () => {
        const response = await axios.get(`http://localhost:5000/transactions/${clientID}`);
        setTransactions(response.data);
    };

    const handleUndo = async () => {
        if (!transactionIdToModify) {
            alert('Please enter a transaction ID.');
            return;
        }
        try {
            const undoResponse = await axios.post('http://localhost:5000/api/transactions/undo', { transactionIdToModify });
            if (undoResponse.data.status === "success") {
                alert(undoResponse.data.message); // Ensure you are accessing data.message
                setTransactionIdToModify(''); // Clear the input after action
            } else {
                alert(undoResponse.data.message);
            }
        } catch (error) {
            console.error('Error during undo:', error);
        }
    };
    
    const handleRedo = async () => {
        if (!transactionIdToModify) {
            alert('Please enter a transaction ID.');
            return;
        }
        try {
            const redoResponse = await axios.post('http://localhost:5000/api/transactions/redo', { transactionIdToModify });
            if (redoResponse.data.status === "success") {
                alert(redoResponse.data.message); // Ensure you are accessing data.message
                setTransactionIdToModify(''); // Clear the input after action
            } else {
                alert(redoResponse.data.message);
            }
        } catch (error) {
            console.error('Error during redo:', error);
        }
    };

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

        <div>
            {transactions.map((transaction) => (
                <div key={transaction.transactionID}>
                    <p>{`Transaction ID: ${transaction.transactionID}, Amount: ${transaction.amount}`}</p>
                    <button onClick={() => handleUndo(transaction.transactionID)}>Undo</button>
                    <button onClick={() => handleRedo(transaction.transactionID)}>Redo</button>
                </div>
            ))}
        </div>

        <div className="container mt-5">
            <div className="mb-3">
                <input
                    type="text"
                    className="form-control"
                    value={transactionIdToModify}
                    onChange={(e) => setTransactionIdToModify(e.target.value)}
                    placeholder="Enter Transaction ID"
                />
                <button className="btn btn-warning" onClick={handleUndo} style={{ margin: '10px' }}>Undo Transaction</button>
                <button className="btn btn-success" onClick={handleRedo} style={{ margin: '10px' }}>Redo Transaction</button>
            </div>
        </div>

        <div>
            <h2>Active Transactions</h2>
            {activeTransactions.map(tx => (
                <div key={tx.transactionID}>
                    <p>Transaction ID: {tx.transactionID}, RecipientID: {tx.toAccount}, Amount: {tx.amount}</p>
                </div>
            ))}

            <h2>Undone Transactions</h2>
            {undoneTransactions.map(tx => (
                <div key={tx.transaction_id}>
                    <p>Transaction ID: {tx.transactionID}, RecipientID: {tx.toAccount}, Amount: {tx.amount}</p>
                </div>
            ))}
        </div>
    </div>
    );
}
export default Dashboard;