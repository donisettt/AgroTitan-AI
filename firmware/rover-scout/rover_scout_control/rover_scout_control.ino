/*
  AgroTitan-AI - Rover Scout Web Motor Control

  Board   : ESP32-S NodeMCU
  Driver  : L298N Motor Driver
  Control : WiFi Access Point + Web Server
  Status  : Motor movement test successful

  Kelompok 6 - TIF RP 23 CID A
*/

#include <WiFi.h>
#include <WebServer.h>

#define IN1 18
#define IN2 19
#define IN3 16
#define IN4 17

#define ENA 21
#define ENB 22

WebServer server(80);

// WiFi Access Point
const char* ssid = "AgroTitan-Rover";
const char* password = "12345678";

// Default setting
int currentSpeed = 130;
unsigned long moveEndTime = 0;
bool isMoving = false;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotor();

  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("AgroTitan Rover Web Control Started");
  Serial.print("WiFi Name: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("Open in browser: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/stop", handleStop);

  server.begin();

  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (isMoving && millis() >= moveEndTime) {
    stopMotor();
    isMoving = false;
    Serial.println("AUTO STOP");
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>AgroTitan Rover Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #101820;
      color: white;
      text-align: center;
      padding: 20px;
    }

    h1 {
      font-size: 26px;
      margin-bottom: 8px;
    }

    p {
      color: #cbd5e1;
    }

    .panel {
      max-width: 420px;
      margin: auto;
      background: #1e293b;
      padding: 20px;
      border-radius: 18px;
    }

    button {
      width: 120px;
      height: 55px;
      margin: 8px;
      font-size: 18px;
      border: none;
      border-radius: 12px;
      background: #22c55e;
      color: white;
      font-weight: bold;
    }

    button:active {
      transform: scale(0.96);
    }

    .stop {
      background: #ef4444;
      width: 260px;
    }

    .small {
      background: #3b82f6;
    }

    input {
      width: 80%;
      margin: 10px;
    }

    .value {
      font-size: 18px;
      font-weight: bold;
      color: #38bdf8;
    }
  </style>
</head>
<body>
  <div class="panel">
    <h1>AgroTitan-AI Rover</h1>
    <p>Web Control Panel</p>

    <div>
      <button onclick="move('forward')">MAJU</button>
    </div>

    <div>
      <button class="small" onclick="move('left')">KIRI</button>
      <button class="small" onclick="move('right')">KANAN</button>
    </div>

    <div>
      <button onclick="move('backward')">MUNDUR</button>
    </div>

    <div>
      <button class="stop" onclick="stopMotor()">STOP</button>
    </div>

    <hr>

    <p>Speed: <span id="speedValue" class="value">130</span></p>
    <input type="range" min="90" max="255" value="130" id="speed">

    <p>Durasi Gerak: <span id="durationValue" class="value">500</span> ms</p>
    <input type="range" min="100" max="2000" step="100" value="500" id="duration">
  </div>

  <script>
    const speed = document.getElementById('speed');
    const duration = document.getElementById('duration');

    speed.oninput = function() {
      document.getElementById('speedValue').innerText = this.value;
    }

    duration.oninput = function() {
      document.getElementById('durationValue').innerText = this.value;
    }

    function move(direction) {
      fetch(`/move?dir=${direction}&speed=${speed.value}&duration=${duration.value}`)
        .then(response => response.text())
        .then(data => console.log(data));
    }

    function stopMotor() {
      fetch('/stop')
        .then(response => response.text())
        .then(data => console.log(data));
    }
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleMove() {
  String direction = server.arg("dir");
  int speedValue = server.arg("speed").toInt();
  int durationValue = server.arg("duration").toInt();

  if (speedValue < 90) speedValue = 90;
  if (speedValue > 255) speedValue = 255;

  if (durationValue < 100) durationValue = 100;
  if (durationValue > 3000) durationValue = 3000;

  currentSpeed = speedValue;

  Serial.print("Command: ");
  Serial.print(direction);
  Serial.print(" | Speed: ");
  Serial.print(speedValue);
  Serial.print(" | Duration: ");
  Serial.println(durationValue);

  if (direction == "forward") {
    forward(speedValue);
  } else if (direction == "backward") {
    backward(speedValue);
  } else if (direction == "left") {
    turnLeft(speedValue);
  } else if (direction == "right") {
    turnRight(speedValue);
  } else {
    stopMotor();
    server.send(400, "text/plain", "Invalid direction");
    return;
  }

  isMoving = true;
  moveEndTime = millis() + durationValue;

  server.send(200, "text/plain", "OK");
}

void handleStop() {
  stopMotor();
  isMoving = false;
  Serial.println("STOP from web");
  server.send(200, "text/plain", "STOPPED");
}

void forward(int speedValue) {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward(int speedValue) {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft(int speedValue) {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight(int speedValue) {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotor() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}