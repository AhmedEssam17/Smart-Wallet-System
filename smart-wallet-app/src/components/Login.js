import React, { useState } from 'react';
import axios from 'axios';

function Login({ onLoginSuccess }) {
  const [clientID, setClientID] = useState('');
  const [password, setPassword] = useState('');

  const handleLogin = async () => {
    try {
      const response = await axios.post('http://localhost:5000/api/login', { clientID, password });
      onLoginSuccess(response.data);
    } catch (error) {
      console.error('Login failed:', error);
    }
  };

  return (
    <div className="container mt-5">
      <h2>Login</h2>
      <div className="mb-3">
        <input
          type="text"
          className="form-control"
          placeholder="Client ID"
          value={clientID}
          onChange={e => setClientID(e.target.value)}
        />
      </div>
      <div className="mb-3">
        <input
          type="password"
          className="form-control"
          placeholder="Password"
          value={password}
          onChange={e => setPassword(e.target.value)}
        />
      </div>
      <button className="btn btn-primary" onClick={handleLogin}>Login</button>
    </div>
  );
}

export default Login;