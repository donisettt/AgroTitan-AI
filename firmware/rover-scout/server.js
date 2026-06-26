const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// In-memory storage (ganti dengan database untuk production)
let roverTelemetries = [];
let roverCommands = [];
let latestTelemetry = null;

// ==================== ROUTES ====================

// Health check
app.get('/api/health', (req, res) => {
  res.json({ status: 'OK', timestamp: new Date().toISOString() });
});

// Receive telemetry from rover
app.post('/api/rover/telemetries', (req, res) => {
  const telemetry = req.body;
  
  // Add server timestamp
  telemetry.server_timestamp = new Date().toISOString();
  telemetry.id = Date.now();
  
  // Store latest telemetry
  latestTelemetry = telemetry;
  
  // Store history (keep last 1000)
  roverTelemetries.unshift(telemetry);
  if (roverTelemetries.length > 1000) {
    roverTelemetries.pop();
  }
  
  // Broadcast to all connected clients via Socket.IO
  io.emit('telemetry_update', telemetry);
  
  console.log(`[TELEMETRY] Rover ${telemetry.rover_id} | Zone: ${telemetry.zone_id} | Status: ${telemetry.rover_status}`);
  
  res.status(201).json({ 
    status: 'success', 
    message: 'Telemetry received',
    data: telemetry 
  });
});

// Get latest telemetry
app.get('/api/rover/telemetry/latest', (req, res) => {
  if (!latestTelemetry) {
    return res.status(404).json({ status: 'error', message: 'No telemetry available' });
  }
  res.json(latestTelemetry);
});

// Get telemetry history
app.get('/api/rover/telemetries', (req, res) => {
  const limit = parseInt(req.query.limit) || 50;
  res.json(roverTelemetries.slice(0, limit));
});

// Send command to rover (simulated)
app.post('/api/rover/command', (req, res) => {
  const { command, params } = req.body;
  
  const commandLog = {
    id: Date.now(),
    command,
    params: params || {},
    timestamp: new Date().toISOString(),
    status: 'SENT'
  };
  
  roverCommands.unshift(commandLog);
  
  // Broadcast command to clients
  io.emit('command_update', commandLog);
  
  console.log(`[COMMAND] ${command} ${params ? JSON.stringify(params) : ''}`);
  
  res.json({ 
    status: 'success', 
    message: `Command ${command} sent to rover`,
    data: commandLog 
  });
});

// Get command history
app.get('/api/rover/commands', (req, res) => {
  const limit = parseInt(req.query.limit) || 50;
  res.json(roverCommands.slice(0, limit));
});

// ==================== SOCKET.IO ====================

io.on('connection', (socket) => {
  console.log(`[SOCKET] Client connected: ${socket.id}`);
  
  // Send latest telemetry on connection
  if (latestTelemetry) {
    socket.emit('telemetry_update', latestTelemetry);
  }
  
  socket.on('disconnect', () => {
    console.log(`[SOCKET] Client disconnected: ${socket.id}`);
  });
});

// ==================== START SERVER ====================

const PORT = process.env.PORT || 3000;

server.listen(PORT, () => {
  console.log('=========================================');
  console.log('AgroTitan-AI Web Dashboard');
  console.log(`Server running on http://localhost:${PORT}`);
  console.log(`Socket.IO ready for real-time updates`);
  console.log('=========================================');
});