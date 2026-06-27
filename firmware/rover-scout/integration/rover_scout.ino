#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "Doni Sw";
const char* password = "";

WebServer server(80);

#define IN1 18
#define IN2 19
#define IN3 16
#define IN4 17
#define ENA 21
#define ENB 22
#define TRIG_PIN 4
#define ECHO_PIN 5
#define BUZZER_PIN 25
#define DHT_PIN 14
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

int speedForward = 120;
int speedTurn = 180;
int distanceLimit = 30;
String command = "STOP";
unsigned long startTime = 0;
float currentTemp = 0;
float currentHum = 0;

void beepReverse();
void beepWarning();
void stopMotor();
void maju(int a, int b);
void mundur(int a, int b);
void kiri(int a, int b);
void kanan(int a, int b);
float getDistance();
void handleRoot();
void handleStatus();
void handleSettings();

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  stopMotor();
  dht.begin();

  Serial.println("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  startTime = millis();

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/settings", handleSettings);
  server.on("/forward",  []() { command = "FORWARD";  server.send(200, "text/plain", "OK"); });
  server.on("/backward", []() { command = "BACKWARD"; server.send(200, "text/plain", "OK"); });
  server.on("/left",     []() { command = "LEFT";     server.send(200, "text/plain", "OK"); });
  server.on("/right",    []() { command = "RIGHT";    server.send(200, "text/plain", "OK"); });
  server.on("/stop",     []() { command = "STOP";     server.send(200, "text/plain", "OK"); });
  server.on("/emergencystop", []() { command = "STOP"; server.send(200, "text/plain", "OK"); });

  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  server.handleClient();

  // DHT22 non-blocking, baca tiap 2 detik
  static unsigned long lastDHT = 0;
  if (millis() - lastDHT >= 2000) {
    lastDHT = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) currentTemp = t;
    if (!isnan(h)) currentHum = h;
    Serial.printf("[DHT22] Suhu: %.1f°C | Kelembaban: %.1f%%\n", currentTemp, currentHum);
  }

  float distance = getDistance();

  if (command == "FORWARD" && distance > 0 && distance <= distanceLimit) {
    Serial.printf("[WARN] Obstacle: %.1f cm\n", distance);
    stopMotor();
    beepWarning();
    return;
  }

  digitalWrite(BUZZER_PIN, LOW);
  if      (command == "FORWARD")  { maju(speedForward, speedForward); }
  else if (command == "BACKWARD") { mundur(speedForward, speedForward); beepReverse(); }
  else if (command == "LEFT")     { kiri(speedTurn, speedTurn); }
  else if (command == "RIGHT")    { kanan(speedTurn, speedTurn); }
  else                            { stopMotor(); }
}

void handleStatus() {
  float dist = getDistance();
  unsigned long uptime = (millis() - startTime) / 1000;
  int rssi = WiFi.RSSI();
  String distStatus = (dist > 0 && dist <= distanceLimit) ? "OBSTACLE" : "CLEAR";
  String ip = WiFi.localIP().toString();

  String json = "{";
  json += "\"distance\":" + String(dist, 1) + ",";
  json += "\"status\":\"" + distStatus + "\",";
  json += "\"uptime\":" + String(uptime) + ",";
  json += "\"rssi\":" + String(rssi) + ",";
  json += "\"command\":\"" + command + "\",";
  json += "\"speed\":" + String(speedForward) + ",";
  json += "\"distLimit\":" + String(distanceLimit) + ",";
  json += "\"ip\":\"" + ip + "\",";
  json += "\"temp\":" + String(currentTemp, 1) + ",";
  json += "\"humidity\":" + String(currentHum, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleSettings() {
  if (server.hasArg("speed")) {
    int s = server.arg("speed").toInt();
    if (s >= 90 && s <= 255) { speedForward = s; speedTurn = s; }
  }
  if (server.hasArg("distLimit")) {
    int d = server.arg("distLimit").toInt();
    if (d >= 10 && d <= 150) distanceLimit = d;
  }
  server.send(200, "text/plain", "OK");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>Rover Scout - AgroTitan AI</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0a0e14;color:#e6edf3;font-family:Arial,sans-serif;min-height:100vh}
.topbar{display:flex;align-items:center;justify-content:space-between;padding:10px 20px;background:#0d1117;border-bottom:1px solid #21262d;flex-wrap:wrap;gap:8px}
.topbar-left{display:flex;align-items:center;gap:12px}
.logo-box{display:flex;align-items:center;gap:8px}
.logo-ic{width:38px;height:38px;background:#1a3a1a;border-radius:8px;display:flex;align-items:center;justify-content:center;font-size:20px;border:1px solid #3fb95033}
.brand-name{font-size:16px;font-weight:700;letter-spacing:.5px}
.brand-name b{color:#3fb950}
.brand-sub{font-size:9px;color:#7d8590;letter-spacing:2px}
.divider{width:1px;height:30px;background:#21262d}
.page-title{font-size:14px;color:#7d8590;font-weight:400}
.topbar-right{display:flex;align-items:center;gap:16px;flex-wrap:wrap}
.top-stat{display:flex;align-items:center;gap:6px;font-size:12px}
.top-stat .ic{font-size:16px}
.top-stat .tv{font-weight:600}
.cg{color:#3fb950}.cb{color:#58a6ff}.cy{color:#d29922}.cr{color:#f85149}.co{color:#ff9966}.cp{color:#bc8cff}
.dot-pulse{width:8px;height:8px;background:#3fb950;border-radius:50%;animation:pulse 2s infinite;display:inline-block}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
.stats-row{display:grid;grid-template-columns:repeat(5,1fr);gap:10px;padding:14px 20px 10px}
.scard{background:#0d1117;border:1px solid #21262d;border-radius:10px;padding:12px 14px;display:flex;align-items:center;gap:10px}
.scard-ic{font-size:26px;flex-shrink:0}
.scard-info{flex:1}
.scard-lbl{font-size:10px;color:#7d8590;letter-spacing:.5px;margin-bottom:2px}
.scard-val{font-size:20px;font-weight:700;line-height:1}
.scard-sub{font-size:10px;color:#7d8590;margin-top:2px}
.scard-bar{height:3px;background:#21262d;border-radius:2px;margin-top:5px;width:100%}
.scard-bar-f{height:100%;border-radius:2px;background:#3fb950}
.main{display:grid;grid-template-columns:1fr 320px 260px;gap:10px;padding:0 20px 20px}
.cam-card{background:#0d1117;border:1px solid #21262d;border-radius:10px;overflow:hidden}
.card-header{display:flex;align-items:center;justify-content:space-between;padding:10px 14px;border-bottom:1px solid #21262d}
.card-title{font-size:11px;font-weight:600;letter-spacing:1px;color:#7d8590;display:flex;align-items:center;gap:6px}
.cam-body{aspect-ratio:16/9;background:#050810;display:flex;align-items:center;justify-content:center;position:relative}
.cam-txt{color:#2d3748;font-size:12px;text-align:center;line-height:2}
.rec-badge{position:absolute;top:8px;left:8px;background:#f8514933;color:#f85149;font-size:10px;font-weight:700;padding:3px 8px;border-radius:4px;border:1px solid #f8514966;display:flex;align-items:center;gap:4px}
.expand-btn{position:absolute;bottom:8px;right:8px;background:#00000066;color:#7d8590;padding:4px 8px;border-radius:4px;font-size:11px;cursor:pointer;border:1px solid #21262d}
.card{background:#0d1117;border:1px solid #21262d;border-radius:10px;overflow:hidden}
.card-body{padding:14px}
.status-row{margin-bottom:10px}
.status-lbl{font-size:10px;color:#7d8590;margin-bottom:2px}
.status-val{font-size:14px;font-weight:600}
.online-badge{background:#1a3a1a;color:#3fb950;font-size:10px;font-weight:600;padding:2px 8px;border-radius:10px;border:1px solid #3fb95033}
.logs-box{font-size:11px;font-family:monospace;max-height:130px;overflow-y:auto;line-height:1.7}
.ll{display:flex;gap:8px;margin-bottom:2px}
.lt{color:#3d444d;flex-shrink:0}.li{color:#58a6ff;flex-shrink:0}.lw{color:#d29922;flex-shrink:0}.lk{color:#3fb950;flex-shrink:0}
.right-col{display:flex;flex-direction:column;gap:10px}
.sl-row{margin-bottom:12px}
.sl-row:last-child{margin-bottom:0}
.sl-lbl{font-size:11px;color:#7d8590;display:flex;justify-content:space-between;margin-bottom:6px}
.sl-lbl span{color:#e6edf3;font-weight:700;font-size:13px}
input[type=range]{width:100%;accent-color:#3fb950;cursor:pointer;height:4px}
.sl-minmax{display:flex;justify-content:space-between;font-size:9px;color:#3d444d;margin-top:3px}
.dpad{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.dbtn{border-radius:8px;font-size:13px;font-weight:700;display:flex;flex-direction:column;align-items:center;justify-content:center;cursor:pointer;user-select:none;-webkit-user-select:none;transition:transform .1s,opacity .1s;border:1px solid #30363d;background:#161b22;color:#e6edf3;padding:14px 8px;gap:4px;letter-spacing:.5px}
.dbtn:active{transform:scale(.94);opacity:.8}
.dbtn .ic{font-size:18px}
.fwd,.bwd{background:#0f2a0f;border-color:#3fb95066;color:#3fb950}
.lft,.rgt{background:#0a1a30;border-color:#58a6ff66;color:#58a6ff}
.stp{background:#2a0a0a;border-color:#f8514966;color:#f85149}
.emp{background:transparent;border:none;pointer-events:none}
.qa-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.qa-btn{display:flex;align-items:center;justify-content:center;gap:6px;padding:10px 8px;border-radius:8px;border:1px solid #21262d;background:#161b22;color:#e6edf3;font-size:11px;font-weight:600;cursor:pointer;transition:background .15s;letter-spacing:.3px}
.qa-btn:hover{background:#21262d}
.qa-btn:active{transform:scale(.97)}
.qa-blue{border-color:#58a6ff44;color:#58a6ff;background:#0a1a30}
.qa-yellow{border-color:#d2992244;color:#d29922;background:#1a1500}
.qa-purple{border-color:#bc8cff44;color:#bc8cff;background:#1a0a2a}
.qa-red{border-color:#f8514944;color:#f85149;background:#2a0a0a}
.info-table{width:100%;font-size:11px}
.info-table tr td:first-child{color:#7d8590;padding:3px 0;width:60px}
.info-table tr td:last-child{color:#e6edf3;font-weight:500}
.obs-banner{display:none;background:#3a0a0a;border:1px solid #f8514966;color:#f85149;font-size:12px;font-weight:600;text-align:center;padding:8px 20px;margin:0 20px 8px;border-radius:8px}
/* DHT Alert */
.dht-alert{display:none;background:#1a1a0a;border:1px solid #d2992244;color:#d29922;font-size:11px;text-align:center;padding:5px;margin-top:6px;border-radius:6px}
@media(max-width:768px){
  .stats-row{grid-template-columns:1fr 1fr;padding:10px 12px 8px}
  .main{grid-template-columns:1fr;padding:0 12px 20px}
  .topbar{padding:10px 12px}
  .divider,.page-title{display:none}
  .obs-banner{margin:0 12px 8px}
  .dpad{gap:6px}
  .dbtn{padding:12px 6px}
}
@media(max-width:400px){
  .stats-row{grid-template-columns:1fr 1fr}
  .scard-val{font-size:16px}
}
</style>
</head>
<body>

<div class="topbar">
  <div class="topbar-left">
    <div class="logo-box">
      <div class="logo-ic">&#127807;</div>
      <div>
        <div class="brand-name">ROVER <b>SCOUT</b></div>
        <div class="brand-sub">AGROTITAN-AI</div>
      </div>
    </div>
    <div class="divider"></div>
    <div class="page-title">Rover Control Dashboard</div>
  </div>
  <div class="topbar-right">
    <div class="top-stat"><span class="ic">&#128246;</span><span class="tv cg">WiFi Connected</span></div>
    <div class="top-stat"><span class="ic">&#128267;</span><span class="tv">--</span></div>
    <div class="top-stat"><span class="ic">&#128336;</span><span class="tv" id="t-time">--:--:--</span></div>
  </div>
</div>

<div class="obs-banner" id="obs">&#9888; OBSTACLE TERDETEKSI! Rover dihentikan.</div>

<div class="stats-row">
  <div class="scard">
    <div class="scard-ic">&#128267;</div>
    <div class="scard-info">
      <div class="scard-lbl">Baterai</div>
      <div class="scard-val cg">--%</div>
      <div class="scard-sub">-- V</div>
      <div class="scard-bar"><div class="scard-bar-f" style="width:0%"></div></div>
    </div>
  </div>
  <div class="scard">
    <div class="scard-ic cb">&#128269;</div>
    <div class="scard-info">
      <div class="scard-lbl">Jarak</div>
      <div class="scard-val cb" id="s-dist">-- cm</div>
      <div class="scard-sub cg" id="s-dist-st">--</div>
    </div>
  </div>
  <div class="scard">
    <div class="scard-ic co">&#127777;</div>
    <div class="scard-info">
      <div class="scard-lbl">Suhu</div>
      <div class="scard-val co" id="s-temp">-- &#176;C</div>
      <div class="scard-sub">DHT22</div>
    </div>
  </div>
  <div class="scard">
    <div class="scard-ic cb">&#128167;</div>
    <div class="scard-info">
      <div class="scard-lbl">Kelembaban</div>
      <div class="scard-val cb" id="s-hum">--%</div>
      <div class="scard-sub">DHT22</div>
    </div>
  </div>
  <div class="scard">
    <div class="scard-ic cy">&#128246;</div>
    <div class="scard-info">
      <div class="scard-lbl">WiFi Signal</div>
      <div class="scard-val cy" id="s-rssi">-- dBm</div>
      <div class="scard-sub cg">Good</div>
    </div>
  </div>
</div>

<div class="main">

  <!-- COL 1: CAMERA + LOGS -->
  <div style="display:flex;flex-direction:column;gap:10px">
    <div class="cam-card">
      <div class="card-header">
        <div class="card-title"><div class="dot-pulse"></div>KAMERA LANGSUNG (ESP32-CAM)</div>
      </div>
      <div class="cam-body">
        <div class="cam-txt">[ Kamera ]<br>Stream belum aktif<br><span style="font-size:10px;color:#1a2030">Hubungkan ESP32-CAM</span></div>
        <div class="rec-badge"><div style="width:6px;height:6px;background:#f85149;border-radius:50%"></div> REC</div>
        <div class="expand-btn">&#8599; Perbesar</div>
      </div>
    </div>
    <div class="card">
      <div class="card-header">
        <div class="card-title"><div class="dot-pulse"></div>LOG SISTEM</div>
        <span style="font-size:11px;color:#58a6ff;cursor:pointer">Lihat Semua</span>
      </div>
      <div class="card-body">
        <div class="logs-box" id="logs">
          <div class="ll"><span class="lt">[--:--:--]</span><span class="li">INFO</span><span>Sistem dimulai</span></div>
        </div>
      </div>
    </div>
  </div>

  <!-- COL 2: STATUS + QUICK ACTIONS + ROVER INFO -->
  <div style="display:flex;flex-direction:column;gap:10px">
    <div class="card">
      <div class="card-header">
        <div class="card-title"><div class="dot-pulse"></div>STATUS ROVER</div>
        <span class="online-badge">ONLINE</span>
      </div>
      <div class="card-body">
        <div class="status-row">
          <div class="status-lbl">IP Address</div>
          <div class="status-val cg" id="st-ip">---.---.---.---</div>
        </div>
        <div class="status-row">
          <div class="status-lbl">Mode</div>
          <div class="status-val" id="st-mode">--</div>
        </div>
        <div class="status-row">
          <div class="status-lbl">Waktu Aktif</div>
          <div class="status-val" id="st-uptime">00:00:00</div>
        </div>
        <div class="status-row">
          <div class="status-lbl">Perintah</div>
          <div class="status-val cy" id="st-cmd">STOP</div>
        </div>
        <!-- DHT realtime di status panel -->
        <div style="border-top:1px solid #21262d;margin-top:10px;padding-top:10px">
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px">
            <div>
              <div class="status-lbl">Suhu</div>
              <div class="status-val co" id="st-temp">-- &#176;C</div>
            </div>
            <div>
              <div class="status-lbl">Kelembaban</div>
              <div class="status-val cb" id="st-hum">--%</div>
            </div>
          </div>
          <div class="dht-alert" id="dht-alert">&#9888; Sensor DHT22 tidak merespons</div>
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-header">
        <div class="card-title"><div class="dot-pulse"></div>AKSI CEPAT</div>
      </div>
      <div class="card-body">
        <div class="qa-grid">
          <div class="qa-btn qa-blue" onclick="addLog('INFO','Restart diminta...')">&#8635; Restart</div>
          <div class="qa-btn qa-yellow" onclick="addLog('INFO','Foto diambil')">&#128247; Foto</div>
          <div class="qa-btn qa-purple" onclick="downloadLogs()">&#11015; Unduh Log</div>
          <div class="qa-btn qa-red" onclick="emergencyStop()">&#9888; Darurat</div>
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-header">
        <div class="card-title">&#9899; INFO ROVER</div>
      </div>
      <div class="card-body">
        <table class="info-table">
          <tr><td>Nama</td><td>: Rover Scout</td></tr>
          <tr><td>Versi</td><td>: v1.0.0</td></tr>
          <tr><td>Board</td><td>: ESP32-S</td></tr>
          <tr><td>Driver</td><td>: L298N</td></tr>
          <tr><td>Kamera</td><td>: ESP32-CAM</td></tr>
          <tr><td>Sensor</td><td>: HC-SR04 + DHT22</td></tr>
        </table>
      </div>
    </div>
  </div>

  <!-- COL 3: SETTINGS + DPAD -->
  <div class="right-col">
    <div class="card">
      <div class="card-header">
        <div class="card-title">&#9881; PENGATURAN</div>
      </div>
      <div class="card-body">
        <div class="sl-row">
          <div class="sl-lbl">Kecepatan <span id="lbl-spd">120</span></div>
          <input type="range" min="90" max="255" value="120" id="sl-spd">
          <div class="sl-minmax"><span>90</span><span>255</span></div>
        </div>
        <div class="sl-row">
          <div class="sl-lbl">Jarak Stop (cm) <span id="lbl-dst">30</span></div>
          <input type="range" min="10" max="100" value="30" id="sl-dst">
          <div class="sl-minmax"><span>10</span><span>100</span></div>
        </div>
        <div class="sl-row" style="margin-bottom:0">
          <div class="sl-lbl">Kecepatan Belok <span id="lbl-trn">180</span></div>
          <input type="range" min="90" max="255" value="180" id="sl-trn">
          <div class="sl-minmax"><span>90</span><span>255</span></div>
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-header">
        <div class="card-title"><div class="dot-pulse"></div>KENDALI MANUAL</div>
      </div>
      <div class="card-body">
        <div class="dpad">
          <div class="dbtn emp"></div>
          <div class="dbtn fwd" id="btn-fwd"><div class="ic">&#9650;</div>MAJU</div>
          <div class="dbtn emp"></div>
          <div class="dbtn lft" id="btn-lft"><div class="ic">&#9668;</div>KIRI</div>
          <div class="dbtn stp" id="btn-stp"><div class="ic">&#9632;</div>STOP</div>
          <div class="dbtn rgt" id="btn-rgt"><div class="ic">&#9658;</div>KANAN</div>
          <div class="dbtn emp"></div>
          <div class="dbtn bwd" id="btn-bwd"><div class="ic">&#9660;</div>MUNDUR</div>
          <div class="dbtn emp"></div>
        </div>
      </div>
    </div>
  </div>

</div>

<script>
function pad(n){return String(n).padStart(2,"0");}
function now(){const d=new Date();return pad(d.getHours())+":"+pad(d.getMinutes())+":"+pad(d.getSeconds());}

function addLog(type,msg){
  const cls=type=="WARN"?"lw":type=="OK"?"lk":"li";
  const box=document.getElementById("logs");
  box.innerHTML+='<div class="ll"><span class="lt">['+now()+']</span><span class="'+cls+'">'+type+'</span><span>'+msg+'</span></div>';
  box.scrollTop=box.scrollHeight;
}

setInterval(()=>{document.getElementById("t-time").textContent=now();},1000);

function send(cmd){fetch("/"+cmd);addLog("INFO","CMD: "+cmd.toUpperCase());}

function hold(id,cmd){
  const b=document.getElementById(id);
  b.addEventListener("mousedown",()=>send(cmd));
  b.addEventListener("mouseup",()=>send("stop"));
  b.addEventListener("mouseleave",()=>send("stop"));
  b.addEventListener("touchstart",(e)=>{e.preventDefault();send(cmd);},{passive:false});
  b.addEventListener("touchend",()=>send("stop"));
}

hold("btn-fwd","forward");
hold("btn-bwd","backward");
hold("btn-lft","left");
hold("btn-rgt","right");
document.getElementById("btn-stp").addEventListener("click",()=>send("stop"));

function emergencyStop(){send("stop");addLog("WARN","EMERGENCY STOP diaktifkan!");}

function downloadLogs(){
  const txt=document.getElementById("logs").innerText;
  const a=document.createElement("a");
  a.href="data:text/plain;charset=utf-8,"+encodeURIComponent(txt);
  a.download="rover_log_"+now().replace(/:/g,"")+".txt";
  a.click();
  addLog("OK","Log diunduh");
}

function sendSettings(){
  fetch("/settings?speed="+document.getElementById("sl-spd").value+"&distLimit="+document.getElementById("sl-dst").value);
}

document.getElementById("sl-spd").oninput=function(){document.getElementById("lbl-spd").textContent=this.value;clearTimeout(this._t);this._t=setTimeout(sendSettings,500);};
document.getElementById("sl-dst").oninput=function(){document.getElementById("lbl-dst").textContent=this.value;clearTimeout(this._t);this._t=setTimeout(sendSettings,500);};
document.getElementById("sl-trn").oninput=function(){document.getElementById("lbl-trn").textContent=this.value;};

// Track apakah DHT pernah dapat data valid
let dhtOk = false;

function fetchStatus(){
  fetch("/status").then(r=>r.json()).then(d=>{
    // Distance
    document.getElementById("s-dist").textContent=d.distance.toFixed(1)+" cm";
    const ds=document.getElementById("s-dist-st");
    if(d.status==="OBSTACLE"){
      ds.textContent="TERHALANG"; ds.className="scard-sub cr";
      document.getElementById("obs").style.display="block";
    } else {
      ds.textContent="AMAN"; ds.className="scard-sub cg";
      document.getElementById("obs").style.display="none";
    }

    // DHT22 — cek validitas (0.0 saat belum dapat data pertama)
    if(d.temp > 0 || dhtOk){
      dhtOk = true;
      const tempStr = d.temp.toFixed(1)+" \u00b0C";
      const humStr  = d.humidity.toFixed(1)+"%";
      document.getElementById("s-temp").textContent = tempStr;
      document.getElementById("s-hum").textContent  = humStr;
      document.getElementById("st-temp").textContent = tempStr;
      document.getElementById("st-hum").textContent  = humStr;
      document.getElementById("dht-alert").style.display = "none";

      // Log jika suhu ekstrem
      if(d.temp > 35) addLog("WARN","Suhu tinggi: "+d.temp.toFixed(1)+"°C");
    } else {
      document.getElementById("dht-alert").style.display = "block";
    }

    // Status
    document.getElementById("s-rssi").textContent=d.rssi+" dBm";
    document.getElementById("st-ip").textContent=d.ip||"--";
    const ut=d.uptime;
    document.getElementById("st-uptime").textContent=pad(Math.floor(ut/3600))+":"+pad(Math.floor((ut%3600)/60))+":"+pad(ut%60);
    document.getElementById("st-mode").textContent=d.command==="STOP"?"IDLE":"MANUAL";
    document.getElementById("st-cmd").textContent=d.command;
    document.getElementById("lbl-spd").textContent=d.speed;
    document.getElementById("sl-spd").value=d.speed;
    document.getElementById("lbl-dst").textContent=d.distLimit;
    document.getElementById("sl-dst").value=d.distLimit;
  }).catch(()=>{});
}

setInterval(fetchStatus,1500);
fetchStatus();
addLog("INFO","WiFi Terhubung");
addLog("INFO","Dashboard dimuat");
addLog("INFO","DHT22 aktif - GPIO14");
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// ======================
// BUZZER
// ======================
void beepReverse() {
  digitalWrite(BUZZER_PIN, HIGH); delay(150);
  digitalWrite(BUZZER_PIN, LOW);  delay(600);
}
void beepWarning() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(80);
    digitalWrite(BUZZER_PIN, LOW);  delay(80);
  }
  delay(150);
}

// ======================
// SENSOR
// ======================
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1;
  return dur * 0.034 / 2;
}

// ======================
// MOTOR CONTROL
// ======================
void maju(int a, int b) {
  analogWrite(ENA,a); analogWrite(ENB,b);
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
}
void mundur(int a, int b) {
  analogWrite(ENA,a); analogWrite(ENB,b);
  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);
}
void kiri(int a, int b) {
  analogWrite(ENA,a); analogWrite(ENB,b);
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW);
}
void kanan(int a, int b) {
  analogWrite(ENA,a); analogWrite(ENB,b);
  digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
}
void stopMotor() {
  analogWrite(ENA,0); analogWrite(ENB,0);
  digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW); digitalWrite(IN4,LOW);
}