#define IN1 18
#define IN2 19
#define IN3 16
#define IN4 17

#define ENA 21
#define ENB 22

// HC-SR04
#define TRIG_PIN 4
#define ECHO_PIN 5

// Buzzer
#define BUZZER_PIN 25

// Batas deteksi
#define DISTANCE_LIMIT 15

void setup() {

  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  stopMotor();

  Serial.println("Rover Scout Started");
}

void loop() {

  float distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Ada halangan
  if (distance > 0 && distance <= DISTANCE_LIMIT) {

    Serial.println("OBJECT DETECTED!");

    stopMotor();

    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);

    digitalWrite(BUZZER_PIN, LOW);
    delay(300);

    return;
  }

  digitalWrite(BUZZER_PIN, LOW);

  // Pola patroli
  Serial.println("MAJU");
  maju(120, 120);
  delay(2000);

  Serial.println("STOP");
  stopMotor();
  delay(200);

  Serial.println("BELOK KANAN");
  kanan(180, 180);
  delay(700);

  Serial.println("STOP");
  stopMotor();
  delay(200);
}

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  float distance = duration * 0.034 / 2;

  return distance;
}

void maju(int speedA, int speedB) {

  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void kanan(int speedA, int speedB) {

  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);

  // roda kanan berhenti
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // roda kiri maju
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotor() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}