// ============================================
// AgroTitan-AI Dashboard JavaScript
// ============================================

const API_BASE = 'http://localhost:3000/api';

// ----- State -----
let socket = null;
let isConnected = false;
let telemetryHistory = [];

// ----- DOM References -----
const dom = {
  // Status
  connectionStatus: document.getElementById('connectionStatus'),
  roverStatus: document.getElementById('roverStatus'),
  currentZone: document.getElementById('currentZone'),
  temperature: document.getElementById('temperature'),
  humidity: document.getElementById('humidity'),
  obstacleDistance: document.getElementById('obstacleDistance'),
  plantStatus: document.getElementById('plantStatus'),
  
  // Image
  cameraImage: document.getElementById('cameraImage'),
  imagePlaceholder: document.getElementById('imagePlaceholder'),
  imageZone: document.getElementById('imageZone'),
  imageTime: document.getElementById('imageTime'),
  
  // Logs
  commandLogContainer: document.getElementById('commandLogContainer'),
  historyContainer: document.getElementById('historyContainer'),
  
  // Footer
  lastUpdate: document.getElementById('lastUpdate'),
  serverTime: document.getElementById('serverTime'),
  
  // Command Display
  cmdDisplay: document.getElementById('cmdDisplay'),
};

// ----- Socket.IO Connection -----
function connectSocket() {
  socket = io('http://localhost:3000', {
    reconnection: true,
    reconnectionAttempts: 5,
    reconnectionDelay: 2000,
  });

  socket.on('connect', () => {
    console.log('Socket connected');
    isConnected = true;
    updateConnectionStatus('connected', 'Terhubung');
  });

  socket.on('disconnect', () => {
    console.log('Socket disconnected');
    isConnected = false;
    updateConnectionStatus('disconnected', 'Terputus');
  });

  socket.on('connect_error', (err) => {
    console.warn('Socket connection error:', err);
    updateConnectionStatus('connecting', 'Menghubung...');
  });

  socket.on('telemetry_update', (data) => {
    console.log('Telemetry update:', data);
    updateDashboard(data);
  });

  socket.on('command_update', (data) => {
    console.log('Command update:', data);
    addCommandLog(data);
  });
}

// ----- Connection Status -----
function updateConnectionStatus(state, label) {
  const el = dom.connectionStatus;
  el.className = `status-indicator ${state}`;
  el.querySelector('span').textContent = label;
}

// ----- Update Dashboard -----
function updateDashboard(data) {
  // Rover Status
  const statusMap = {
    'IDLE': { class: 'status-idle', label: '🟡 IDLE' },
    'PATROL': { class: 'status-patrol', label: '🟢 PATROL' },
    'STOPPED_AT_ZONE': { class: 'status-zone', label: '🟠 AT ZONE' },
    'OBSTACLE_DETECTED': { class: 'status-obstacle', label: '🔴 OBSTACLE' },
  };
  const status = statusMap[data.rover_status] || { class: 'status-idle', label: data.rover_status || 'UNKNOWN' };
  dom.roverStatus.className = `stat-value ${status.class}`;
  dom.roverStatus.textContent = status.label;

  // Zone
  dom.currentZone.textContent = data.zone_id || '--';

  // Temperature
  dom.temperature.textContent = data.temperature ? `${data.temperature.toFixed(1)} °C` : '-- °C';

  // Humidity
  dom.humidity.textContent = data.humidity ? `${data.humidity.toFixed(1)} %` : '-- %';

  // Obstacle Distance
  dom.obstacleDistance.textContent = data.obstacle_distance > 0 ? `${data.obstacle_distance.toFixed(1)} cm` : '-- cm';

  // Plant Status
  const plantMap = {
    'NORMAL': { class: 'plant-normal', label: '🌿 Normal' },
    'PERLU_INSPEKSI': { class: 'plant-inspect', label: '⚠️ Perlu Inspeksi' },
    'UNKNOWN': { class: 'plant-unknown', label: '❓ Unknown' },
  };
  const plant = plantMap[data.plant_visual_status] || plantMap['UNKNOWN'];
  dom.plantStatus.className = `stat-value ${plant.class}`;
  dom.plantStatus.textContent = plant.label;

  // Image
  if (data.image_url) {
    dom.cameraImage.src = data.image_url || '';
    dom.cameraImage.className = 'visible';
    dom.imagePlaceholder.style.display = 'none';
    dom.imageZone.textContent = data.zone_id || '--';
    dom.imageTime.textContent = data.timestamp ? new Date(parseInt(data.timestamp)).toLocaleTimeString() : '--';
  }

  // Add to history
  addTelemetryHistory(data);

  // Last update
  dom.lastUpdate.textContent = `Last update: ${new Date().toLocaleTimeString()}`;
}

// ----- Telemetry History -----
function addTelemetryHistory(data) {
  const statusClassMap = {
    'IDLE': 'idle',
    'PATROL': 'patrol',
    'OBSTACLE_DETECTED': 'obstacle',
    'STOPPED_AT_ZONE': 'zone',
  };
  
  const statusClass = statusClassMap[data.rover_status] || 'idle';
  const statusLabel = data.rover_status || 'UNKNOWN';
  
  const item = document.createElement('div');
  item.className = 'history-item';
  item.innerHTML = `
    <span class="h-time">${new Date().toLocaleTimeString()}</span>
    <span class="h-zone">${data.zone_id || '--'}</span>
    <span class="h-status ${statusClass}">${statusLabel}</span>
    <span style="color: var(--text-muted); font-size: 10px; float: right;">
      ${data.temperature ? `${data.temperature.toFixed(1)}°C` : '--'} | 
      ${data.humidity ? `${data.humidity.toFixed(1)}%` : '--'}
    </span>
  `;
  
  dom.historyContainer.prepend(item);
  
  // Limit history items
  const items = dom.historyContainer.querySelectorAll('.history-item');
  if (items.length > 20) {
    items[items.length - 1].remove();
  }
  
  // Remove empty message
  const empty = dom.historyContainer.querySelector('.log-empty');
  if (empty) empty.remove();
}

// ----- Command Log -----
function addCommandLog(data) {
  const item = document.createElement('div');
  item.className = 'command-item';
  
  // Map command ke display name
  const cmdMap = {
    'FORWARD': '⬆ MAJU',
    'BACKWARD': '⬇ MUNDUR',
    'TURN_LEFT': '⬅ KIRI',
    'TURN_RIGHT': '➡ KANAN',
    'STOP': '⏹ STOP',
  };
  
  const displayName = cmdMap[data.command] || data.command;
  
  item.innerHTML = `
    <span class="cmd-name">${displayName}</span>
    <span class="cmd-time">${new Date(data.timestamp).toLocaleTimeString()}</span>
  `;
  
  dom.commandLogContainer.prepend(item);
  
  // Limit command logs
  const items = dom.commandLogContainer.querySelectorAll('.command-item');
  if (items.length > 20) {
    items[items.length - 1].remove();
  }
  
  // Remove empty message
  const empty = dom.commandLogContainer.querySelector('.log-empty');
  if (empty) empty.remove();
}

// ----- Update Command Display -----
function updateCommandDisplay(command) {
  const cmdMap = {
    'FORWARD': '↑ MAJU',
    'BACKWARD': '↓ MUNDUR',
    'TURN_LEFT': '← KIRI',
    'TURN_RIGHT': '→ KANAN',
    'STOP': '⏹ STOP',
  };
  
  const display = cmdMap[command] || command || 'STOP';
  dom.cmdDisplay.textContent = display;
  dom.cmdDisplay.className = 'cmd-text';
  if (command === 'STOP' || !command) {
    dom.cmdDisplay.classList.add('idle');
  }
}

// ----- Send Command -----
async function sendCommand(command, params = {}) {
  // Update display immediately
  updateCommandDisplay(command);
  
  try {
    const response = await fetch(`${API_BASE}/rover/command`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ command, params }),
    });
    
    const result = await response.json();
    console.log('Command sent:', command, result);
    
    if (result.status === 'success') {
      // Visual feedback pada tombol yang ditekan
      const btnMap = {
        'FORWARD': 'btnUp',
        'BACKWARD': 'btnDown',
        'TURN_LEFT': 'btnLeft',
        'TURN_RIGHT': 'btnRight',
      };
      
      const btnId = btnMap[command];
      if (btnId) {
        const btn = document.getElementById(btnId);
        if (btn) {
          btn.style.transform = 'scale(0.85)';
          setTimeout(() => { 
            btn.style.transform = ''; 
          }, 200);
        }
      }
    }
  } catch (error) {
    console.error('Failed to send command:', error);
  }
}

// ----- Load Initial Data -----
async function loadInitialData() {
  try {
    // Load latest telemetry
    const latestRes = await fetch(`${API_BASE}/rover/telemetry/latest`);
    if (latestRes.ok) {
      const data = await latestRes.json();
      updateDashboard(data);
    }
    
    // Load history
    const historyRes = await fetch(`${API_BASE}/rover/telemetries?limit=20`);
    if (historyRes.ok) {
      const history = await historyRes.json();
      history.forEach(item => addTelemetryHistory(item));
    }
    
    // Load commands
    const cmdRes = await fetch(`${API_BASE}/rover/commands?limit=10`);
    if (cmdRes.ok) {
      const commands = await cmdRes.json();
      commands.forEach(item => addCommandLog(item));
    }
  } catch (error) {
    console.error('Failed to load initial data:', error);
  }
}

// ----- Server Time -----
function updateServerTime() {
  const now = new Date();
  dom.serverTime.querySelector('span').textContent = now.toLocaleTimeString('id-ID');
}

// ----- Keyboard Shortcuts -----
document.addEventListener('keydown', (e) => {
  const key = e.key.toLowerCase();
  const commands = {
    'w': 'FORWARD',
    'arrowup': 'FORWARD',
    's': 'BACKWARD',
    'arrowdown': 'BACKWARD',
    'a': 'TURN_LEFT',
    'arrowleft': 'TURN_LEFT',
    'd': 'TURN_RIGHT',
    'arrowright': 'TURN_RIGHT',
    ' ': 'STOP',
  };
  
  if (key === ' ') {
    e.preventDefault();
    sendCommand('STOP');
    return;
  }
  
  if (commands[key]) {
    e.preventDefault();
    sendCommand(commands[key]);
  }
});

// ----- Init -----
console.log('AgroTitan-AI Joystick Dashboard Starting...');

// Start server time
updateServerTime();
setInterval(updateServerTime, 1000);

// Connect to WebSocket
connectSocket();

// Load initial data
loadInitialData();

// Auto-refresh via polling as fallback
setInterval(() => {
  if (!isConnected) {
    loadInitialData();
  }
}, 10000);

console.log('Dashboard ready!');
