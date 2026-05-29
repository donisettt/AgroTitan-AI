#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>

#define WATER_LEVEL_PIN 34
#define SERVO_GATE_PIN 18
#define RELAY_PUMP_PIN 26
#define PUMP_LED_PIN 25
#define ALERT_LED_PIN 33
#define BUZZER_PIN 27
#define RAIN_SWITCH_PIN 32

#define MIN_LEVEL_CM 2.0
#define MAX_LEVEL_CM 5.0

#define GATE_CLOSED_ANGLE 0
#define GATE_OPEN_ANGLE 180

#define RELAY_ACTIVE_HIGH true

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* NODE_ID = "FIXED-NODE-01";

const char* MOCKAPI_TELEMETRY_URL =
  "https://6a199e03489e4715751a3fb6.mockapi.io/api/v1/fixed_node_telemetries";

Servo gateServo;

unsigned long lastReadTime = 0;
unsigned long lastApiPostTime = 0;

const unsigned long READ_INTERVAL = 1000;
const unsigned long API_POST_INTERVAL = 5000;

float waterLevelCm = 0.0;
String waterStatus = "UNKNOWN";
bool rainDetected = false;

String currentPumpStatus = "OFF";
String currentGateStatus = "CLOSED";

void connectWiFi();
float readWaterLevelCm();
void processIrrigationLogic(float level, bool isRainDetected);
void sendTelemetryToMockAPI();
String buildTelemetryPayload();

void activatePump();
void deactivatePump();
void openGate();
void closeGate();

void alertOn(bool withBuzzer);
void alertOff();

void printTelemetry(float level, bool isRainDetected);

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RAIN_SWITCH_PIN, INPUT);

  gateServo.setPeriodHertz(50);
  gateServo.attach(SERVO_GATE_PIN, 500, 2400);

  closeGate();
  deactivatePump();
  alertOff();

  connectWiFi();

  Serial.println("=================================");
  Serial.println("AgroTitan-AI Fixed Irrigation Node");
  Serial.println("REST API Simulation Started");
  Serial.println("=================================");
}

void loop() {
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();

    waterLevelCm = readWaterLevelCm();
    rainDetected = digitalRead(RAIN_SWITCH_PIN) == HIGH;

    processIrrigationLogic(waterLevelCm, rainDetected);
    printTelemetry(waterLevelCm, rainDetected);
  }

  if (millis() - lastApiPostTime >= API_POST_INTERVAL) {
    lastApiPostTime = millis();

    if (WiFi.status() == WL_CONNECTED) {
      sendTelemetryToMockAPI();
    } else {
      Serial.println("WiFi disconnected. Reconnecting...");
      connectWiFi();
    }
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed.");
  }
}

float readWaterLevelCm() {
  int total = 0;
  const int samples = 10;

  for (int i = 0; i < samples; i++) {
    total += analogRead(WATER_LEVEL_PIN);
    delay(5);
  }

  int analogValue = total / samples;
  float levelCm = (analogValue / 4095.0) * 10.0;

  return levelCm;
}

void processIrrigationLogic(float level, bool isRainDetected) {
  if (isRainDetected) {
    waterStatus = "HUJAN_TERDETEKSI";

    closeGate();
    deactivatePump();
    alertOn(false);

    return;
  }

  if (level < MIN_LEVEL_CM) {
    waterStatus = "AIR_RENDAH";

    openGate();
    activatePump();
    alertOn(false);
  } else if (level >= MIN_LEVEL_CM && level <= MAX_LEVEL_CM) {
    waterStatus = "AIR_IDEAL";

    closeGate();
    deactivatePump();
    alertOff();
  } else {
    waterStatus = "AIR_TERLALU_TINGGI";

    closeGate();
    deactivatePump();
    alertOn(true);
  }
}

void sendTelemetryToMockAPI() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  http.begin(client, MOCKAPI_TELEMETRY_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = buildTelemetryPayload();

  Serial.println();
  Serial.println("Sending telemetry to MockAPI...");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();

    Serial.println("Response:");
    Serial.println(response);
  } else {
    Serial.print("POST failed. Error: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
}

String buildTelemetryPayload() {
  String rainStatus = rainDetected ? "true" : "false";

  String payload = "{";
  payload += "\"node_id\":\"" + String(NODE_ID) + "\",";
  payload += "\"water_level\":" + String(waterLevelCm, 2) + ",";
  payload += "\"water_status\":\"" + waterStatus + "\",";
  payload += "\"pump_status\":\"" + currentPumpStatus + "\",";
  payload += "\"gate_status\":\"" + currentGateStatus + "\",";
  payload += "\"rain_status\":" + rainStatus + ",";
  payload += "\"timestamp\":\"" + String(millis()) + "\"";
  payload += "}";

  return payload;
}

void activatePump() {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(RELAY_PUMP_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PUMP_PIN, LOW);
  }

  digitalWrite(PUMP_LED_PIN, HIGH);
  currentPumpStatus = "ON";
}

void deactivatePump() {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(RELAY_PUMP_PIN, LOW);
  } else {
    digitalWrite(RELAY_PUMP_PIN, HIGH);
  }

  digitalWrite(PUMP_LED_PIN, LOW);
  currentPumpStatus = "OFF";
}

void openGate() {
  gateServo.write(GATE_OPEN_ANGLE);
  currentGateStatus = "OPEN";
}

void closeGate() {
  gateServo.write(GATE_CLOSED_ANGLE);
  currentGateStatus = "CLOSED";
}

void alertOn(bool withBuzzer) {
  digitalWrite(ALERT_LED_PIN, HIGH);

  if (withBuzzer) {
    tone(BUZZER_PIN, 1200);
  } else {
    noTone(BUZZER_PIN);
  }
}

void alertOff() {
  digitalWrite(ALERT_LED_PIN, LOW);
  noTone(BUZZER_PIN);
}

void printTelemetry(float level, bool isRainDetected) {
  Serial.println();
  Serial.println("===== TELEMETRY =====");

  Serial.print("Water Level : ");
  Serial.print(level, 2);
  Serial.println(" cm");

  Serial.print("Water Status: ");
  Serial.println(waterStatus);

  Serial.print("Rain Status : ");
  Serial.println(isRainDetected ? "DETECTED" : "CLEAR");

  Serial.print("Pump Status : ");
  Serial.println(currentPumpStatus);

  Serial.print("Gate Status : ");
  Serial.println(currentGateStatus);

  Serial.println("=====================");
}