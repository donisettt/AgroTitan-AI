#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

#define LINE_LEFT_PIN 32
#define LINE_CENTER_PIN 33
#define LINE_RIGHT_PIN 25

#define START_BUTTON_PIN 13
#define STOP_BUTTON_PIN 16
#define MARKER_BUTTON_PIN 14

#define ULTRASONIC_TRIG_PIN 26
#define ULTRASONIC_ECHO_PIN 27

#define DHT_PIN 19
#define DHT_TYPE DHT22

#define MOTOR_LEFT_LED_PIN 21
#define MOTOR_RIGHT_LED_PIN 22
#define STATUS_LED_PIN 2
#define BUZZER_PIN 23

#define OBSTACLE_LIMIT_CM 10.0

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* ROVER_ID = "ROVER-01";

const char* MOCKAPI_ROVER_URL =
  "https://6a199e03489e4715751a3fb6.mockapi.io/api/v1/rover_telemetries";

DHT dht(DHT_PIN, DHT_TYPE);

enum RoverState {
  IDLE,
  PATROL,
  STOPPED_AT_ZONE,
  OBSTACLE
};

RoverState roverState = IDLE;

unsigned long lastTelemetryTime = 0;
unsigned long lastMarkerTime = 0;
unsigned long lastButtonTime = 0;

const unsigned long TELEMETRY_INTERVAL = 5000;
const unsigned long MARKER_DEBOUNCE = 1500;
const unsigned long BUTTON_DEBOUNCE = 150;

int currentZone = 0;

String roverStatus = "IDLE";
String movementAction = "STOP";
String imageUrl = "";
String plantVisualStatus = "UNKNOWN";

float temperature = 0.0;
float humidity = 0.0;
float obstacleDistance = -1.0;
bool obstacleDetected = false;

void connectWiFi();

void handleButtons();
void processRoverState();
void handleLineFollower();
void handleMarkerZone();

void moveForward();
void turnLeft();
void turnRight();
void stopRover();

void captureMockImage();
void readEnvironment();
float readObstacleDistanceCm();

void sendRoverTelemetry();
String buildRoverPayload();

bool isActiveLow(int pin);
String getZoneId();
String getStateName(RoverState state);

void printTelemetry();

void setup() {
  Serial.begin(115200);

  pinMode(LINE_LEFT_PIN, INPUT_PULLUP);
  pinMode(LINE_CENTER_PIN, INPUT_PULLUP);
  pinMode(LINE_RIGHT_PIN, INPUT_PULLUP);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MARKER_BUTTON_PIN, INPUT_PULLUP);

  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);

  pinMode(MOTOR_LEFT_LED_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  dht.begin();

  stopRover();
  digitalWrite(STATUS_LED_PIN, LOW);
  noTone(BUZZER_PIN);

  connectWiFi();

  Serial.println("=================================");
  Serial.println("AgroTitan-AI Rover Scout");
  Serial.println("REST API Wokwi Simulation Started");
  Serial.println("=================================");
}

void loop() {
  handleButtons();

  obstacleDistance = readObstacleDistanceCm();
  obstacleDetected = obstacleDistance > 0 && obstacleDistance < OBSTACLE_LIMIT_CM;

  if (obstacleDetected && roverState == PATROL) {
    roverState = OBSTACLE;
  }

  processRoverState();

  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = millis();

    readEnvironment();
    printTelemetry();

    if (WiFi.status() == WL_CONNECTED) {
      sendRoverTelemetry();
    } else {
      Serial.println("WiFi disconnected. Reconnecting...");
      connectWiFi();
    }
  }

  delay(100);
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

void handleButtons() {
  if (millis() - lastButtonTime < BUTTON_DEBOUNCE) {
    return;
  }

  if (isActiveLow(START_BUTTON_PIN)) {
    lastButtonTime = millis();

    roverState = PATROL;
    roverStatus = "PATROL";
    movementAction = "START_PATROL";
    digitalWrite(STATUS_LED_PIN, HIGH);

    Serial.println("[COMMAND] START_PATROL");
  }

  if (isActiveLow(STOP_BUTTON_PIN)) {
    lastButtonTime = millis();

    roverState = IDLE;
    roverStatus = "IDLE";
    movementAction = "STOP_ROVER";

    stopRover();
    digitalWrite(STATUS_LED_PIN, LOW);
    noTone(BUZZER_PIN);

    Serial.println("[COMMAND] STOP_ROVER");
  }
}

void processRoverState() {
  switch (roverState) {
    case IDLE:
      roverStatus = "IDLE";
      movementAction = "STOP";

      stopRover();
      digitalWrite(STATUS_LED_PIN, LOW);
      noTone(BUZZER_PIN);
      break;

    case PATROL:
      roverStatus = "PATROL";
      digitalWrite(STATUS_LED_PIN, HIGH);
      noTone(BUZZER_PIN);

      handleLineFollower();
      handleMarkerZone();
      break;

    case STOPPED_AT_ZONE:
      roverStatus = "STOPPED_AT_ZONE";
      movementAction = "CAPTURE_IMAGE";

      stopRover();
      readEnvironment();
      captureMockImage();
      printTelemetry();

      if (WiFi.status() == WL_CONNECTED) {
        sendRoverTelemetry();
      }

      delay(1000);

      roverState = PATROL;
      roverStatus = "PATROL";
      break;

    case OBSTACLE:
      roverStatus = "OBSTACLE_DETECTED";
      movementAction = "STOP";

      stopRover();
      digitalWrite(STATUS_LED_PIN, HIGH);
      tone(BUZZER_PIN, 1200);

      Serial.println("[ALERT] Obstacle detected. Rover stopped.");

      if (!obstacleDetected) {
        noTone(BUZZER_PIN);
        roverState = PATROL;
        roverStatus = "PATROL";
      }
      break;
  }
}

void handleLineFollower() {
  bool leftActive = isActiveLow(LINE_LEFT_PIN);
  bool centerActive = isActiveLow(LINE_CENTER_PIN);
  bool rightActive = isActiveLow(LINE_RIGHT_PIN);

  if (centerActive && !leftActive && !rightActive) {
    moveForward();
  } else if (leftActive && !centerActive) {
    turnLeft();
  } else if (rightActive && !centerActive) {
    turnRight();
  } else if (leftActive && centerActive && rightActive) {
    moveForward();
  } else {
    stopRover();
    movementAction = "LINE_LOST";
  }
}

void handleMarkerZone() {
  if (isActiveLow(MARKER_BUTTON_PIN) && millis() - lastMarkerTime > MARKER_DEBOUNCE) {
    lastMarkerTime = millis();

    currentZone++;

    if (currentZone > 3) {
      currentZone = 1;
    }

    Serial.print("[MARKER] Zone detected: ");
    Serial.println(getZoneId());

    roverState = STOPPED_AT_ZONE;
  }
}

void moveForward() {
  digitalWrite(MOTOR_LEFT_LED_PIN, HIGH);
  digitalWrite(MOTOR_RIGHT_LED_PIN, HIGH);

  movementAction = "FORWARD";
}

void turnLeft() {
  digitalWrite(MOTOR_LEFT_LED_PIN, LOW);
  digitalWrite(MOTOR_RIGHT_LED_PIN, HIGH);

  movementAction = "TURN_LEFT";
}

void turnRight() {
  digitalWrite(MOTOR_LEFT_LED_PIN, HIGH);
  digitalWrite(MOTOR_RIGHT_LED_PIN, LOW);

  movementAction = "TURN_RIGHT";
}

void stopRover() {
  digitalWrite(MOTOR_LEFT_LED_PIN, LOW);
  digitalWrite(MOTOR_RIGHT_LED_PIN, LOW);
}

void captureMockImage() {
  imageUrl = "/images/rover/" + getZoneId() + "_demo.jpg";

  if (currentZone == 2) {
    plantVisualStatus = "PERLU_INSPEKSI";
  } else {
    plantVisualStatus = "NORMAL";
  }

  Serial.print("[CAMERA] Mock image captured: ");
  Serial.println(imageUrl);

  Serial.print("[VISUAL] Plant status: ");
  Serial.println(plantVisualStatus);
}

void readEnvironment() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp)) {
    temperature = temp;
  }

  if (!isnan(hum)) {
    humidity = hum;
  }
}

float readObstacleDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

void sendRoverTelemetry() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  http.begin(client, MOCKAPI_ROVER_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = buildRoverPayload();

  Serial.println();
  Serial.println("Sending rover telemetry to MockAPI...");
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

String buildRoverPayload() {
  String obstacleStatus = obstacleDetected ? "true" : "false";

  String payload = "{";
  payload += "\"rover_id\":\"" + String(ROVER_ID) + "\",";
  payload += "\"zone_id\":\"" + getZoneId() + "\",";
  payload += "\"rover_status\":\"" + roverStatus + "\",";
  payload += "\"movement_action\":\"" + movementAction + "\",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"humidity\":" + String(humidity, 2) + ",";
  payload += "\"obstacle_distance\":" + String(obstacleDistance, 2) + ",";
  payload += "\"obstacle_status\":" + obstacleStatus + ",";
  payload += "\"image_url\":\"" + imageUrl + "\",";
  payload += "\"plant_visual_status\":\"" + plantVisualStatus + "\",";
  payload += "\"timestamp\":\"" + String(millis()) + "\"";
  payload += "}";

  return payload;
}

bool isActiveLow(int pin) {
  return digitalRead(pin) == LOW;
}

String getZoneId() {
  if (currentZone <= 0) {
    return "ZONE-00";
  }

  if (currentZone < 10) {
    return "ZONE-0" + String(currentZone);
  }

  return "ZONE-" + String(currentZone);
}

String getStateName(RoverState state) {
  switch (state) {
    case IDLE:
      return "IDLE";
    case PATROL:
      return "PATROL";
    case STOPPED_AT_ZONE:
      return "STOPPED_AT_ZONE";
    case OBSTACLE:
      return "OBSTACLE";
    default:
      return "UNKNOWN";
  }
}

void printTelemetry() {
  Serial.println();
  Serial.println("===== ROVER TELEMETRY =====");

  Serial.print("Rover ID        : ");
  Serial.println(ROVER_ID);

  Serial.print("Zone            : ");
  Serial.println(getZoneId());

  Serial.print("State           : ");
  Serial.println(getStateName(roverState));

  Serial.print("Rover Status    : ");
  Serial.println(roverStatus);

  Serial.print("Movement Action : ");
  Serial.println(movementAction);

  Serial.print("Temperature     : ");
  Serial.print(temperature, 2);
  Serial.println(" C");

  Serial.print("Humidity        : ");
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.print("Obstacle Dist   : ");
  Serial.print(obstacleDistance, 2);
  Serial.println(" cm");

  Serial.print("Obstacle Status : ");
  Serial.println(obstacleDetected ? "DETECTED" : "CLEAR");

  Serial.print("Image URL       : ");
  Serial.println(imageUrl);

  Serial.print("Plant Visual    : ");
  Serial.println(plantVisualStatus);

  Serial.println("===========================");
}