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
#define GATE_OPEN_ANGLE 90

#define RELAY_ACTIVE_HIGH true

Servo gateServo;

unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 1000;

float waterLevelCm = 0.0;
String waterStatus = "UNKNOWN";

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RAIN_SWITCH_PIN, INPUT);

  gateServo.attach(SERVO_GATE_PIN);
  closeGate();
  deactivatePump();
  alertOff();

  Serial.println("=================================");
  Serial.println("AgroTitan-AI Fixed Irrigation Node");
  Serial.println("Simulation Started");
  Serial.println("=================================");
}

void loop() {
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();

    waterLevelCm = readWaterLevelCm();
    bool rainDetected = digitalRead(RAIN_SWITCH_PIN) == HIGH;

    processIrrigationLogic(waterLevelCm, rainDetected);
    printTelemetry(waterLevelCm, rainDetected);
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

void processIrrigationLogic(float level, bool rainDetected) {
  if (rainDetected) {
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
  } 
  else if (level >= MIN_LEVEL_CM && level <= MAX_LEVEL_CM) {
    waterStatus = "AIR_IDEAL";
    closeGate();
    deactivatePump();
    alertOff();
  } 
  else {
    waterStatus = "AIR_TERLALU_TINGGI";
    closeGate();
    deactivatePump();
    alertOn(true);
  }
}

void activatePump() {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(RELAY_PUMP_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PUMP_PIN, LOW);
  }

  digitalWrite(PUMP_LED_PIN, HIGH);
}

void deactivatePump() {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(RELAY_PUMP_PIN, LOW);
  } else {
    digitalWrite(RELAY_PUMP_PIN, HIGH);
  }

  digitalWrite(PUMP_LED_PIN, LOW);
}

void openGate() {
  gateServo.write(GATE_OPEN_ANGLE);
}

void closeGate() {
  gateServo.write(GATE_CLOSED_ANGLE);
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

void printTelemetry(float level, bool rainDetected) {
  String pumpStatus = digitalRead(PUMP_LED_PIN) == HIGH ? "ON" : "OFF";
  String gateStatus = gateServo.read() == GATE_OPEN_ANGLE ? "OPEN" : "CLOSED";

  Serial.println();
  Serial.println("===== TELEMETRY =====");
  Serial.print("Water Level : ");
  Serial.print(level, 2);
  Serial.println(" cm");

  Serial.print("Water Status: ");
  Serial.println(waterStatus);

  Serial.print("Rain Status : ");
  Serial.println(rainDetected ? "DETECTED" : "CLEAR");

  Serial.print("Pump Status : ");
  Serial.println(pumpStatus);

  Serial.print("Gate Status : ");
  Serial.println(gateStatus);

  Serial.println("=====================");
}