import React from 'react';
import { useNavigate } from 'react-router-dom';

function LandingPage() {
  const navigate = useNavigate();

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
      <h1 style={{ marginBottom: '20px' }}>Welcome to Smart Wallet</h1>
      <div style={{ display: 'flex', gap: '10px' }}>
        <button style={{ padding: '10px 20px' }} className="btn btn-primary" onClick={() => navigate('/login')}>Login</button>
        <button style={{ padding: '10px 20px' }} className="btn btn-primary" onClick={() => navigate('/register')}>Register</button>
      </div>
    </div>
  );
}

export default LandingPage;
