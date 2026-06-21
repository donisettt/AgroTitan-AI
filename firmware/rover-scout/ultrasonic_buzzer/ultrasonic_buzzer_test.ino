/*
  AgroTitan-AI - HC-SR04 Obstacle Detection Test

  Board   : ESP32-S NodeMCU
  Sensor  : HC-SR04 Ultrasonic Sensor
  Output  : Active Buzzer
  Feature : Object detection with buzzer alert
  Status  : Test successful

  Kelompok 6 - TIF RP 23 CID A
*/

#define TRIG_PIN 26
#define ECHO_PIN 27
#define BUZZER_PIN 13

#define OBSTACLE_DISTANCE_CM 15

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("AgroTitan-AI HC-SR04 + Buzzer Test Started");
}

void loop() {
  float distance = readDistanceCM();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance <= OBSTACLE_DISTANCE_CM) {
    Serial.println("Object detected! Buzzer ON");
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(300);

}

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  float distance = duration * 0.0343 / 2;
  return distance;
}