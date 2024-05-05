import React, { useState } from 'react';
import axios from 'axios';

function Register({ onRegisterSuccess }) {
  const [clientID, setClientID] = useState('');
  const [password, setPassword] = useState('');
  const [clientInfo, setClientInfo] = useState({
    name: '',
    age: '',
    mobileNum: '',
    nationalID: '',
    email: '',
    balance: ''
  });

  const handleInputChange = (e) => {
    setClientInfo({ ...clientInfo, [e.target.name]: e.target.value });
  };

  const handleRegister = async () => {
    try {
      const response = await axios.post('http://localhost:5000/api/register', {
        clientID,
        password,
        clientInfo
      });
      onRegisterSuccess(response.data);  // Handle post-registration logic
    } catch (error) {
      console.error('Registration failed:', error);
      // Optionally, handle user feedback here
    }
  };

  return (
    <div className="container mt-5">
      <h2>Register</h2>
      <div className="mb-3">
        <input type="text" className="form-control" placeholder="Client ID" value={clientID} onChange={e => setClientID(e.target.value)} />
      </div>
      <div className="mb-3">
        <input type="password" className="form-control" placeholder="Password" value={password} onChange={e => setPassword(e.target.value)} />
      </div>
      {/* Repeat for other fields */}
      <div className="mb-3">
        <input type="text" className="form-control" name="name" placeholder="Name" value={clientInfo.name} onChange={handleInputChange} />
      </div>
      <div className="mb-3">
        <input type="text" className="form-control" name="age" placeholder="Age" value={clientInfo.age} onChange={handleInputChange} />
      </div>
      <div className="mb-3">
        <input type="text" className="form-control" name="mobileNum" placeholder="Mobile Number" value={clientInfo.mobileNum} onChange={handleInputChange} />
      </div>
      <div className="mb-3">
        <input type="text" className="form-control" name="nationalID" placeholder="National ID" value={clientInfo.nationalID} onChange={handleInputChange} />
      </div>
      <div className="mb-3">
        <input type="email" className="form-control" name="email" placeholder="Email" value={clientInfo.email} onChange={handleInputChange} />
      </div>
      <div className="mb-3">
        <input type="text" className="form-control" name="balance" placeholder="Initial Balance" value={clientInfo.balance} onChange={handleInputChange} />
      </div>
      <button className="btn btn-primary" onClick={handleRegister}>Register</button>
    </div>
  );
}

export default Register;