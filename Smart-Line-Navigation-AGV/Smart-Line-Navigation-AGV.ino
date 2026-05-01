#include "HX711.h"

// ================= HX711 =================
#define DT 4
#define SCK 7

HX711 scale;
float calibration_factor = 1000;
float offset = 0;

// ================= PIN MOTOR =================
int IN1 = 8;
int IN2 = 9;

int IN3 = 10;
int IN4 = 12;

// ================= ENABLE =================
int ENA = 5;
int ENB = 6;

// ================= SENSOR IR =================
int IR_KIRI   = A2;
int IR_TENGAH = A1;
int IR_KANAN  = A3;

// ================= SPEED =================
int speedNormal = 80;     // 🔥 diturunkan
int speedPivot  = 150;

// ================= VAR =================
float berat = 0;
bool sudahTimbang = false;

// ================= SETUP =================
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(IR_KIRI, INPUT);
  pinMode(IR_TENGAH, INPUT);
  pinMode(IR_KANAN, INPUT);

  Serial.begin(9600);

  // HX711
  scale.begin(DT, SCK);
  delay(3000);

  scale.set_scale(calibration_factor);
  scale.tare(20);
  delay(500);

  offset = scale.get_units(20);

  Serial.println("=== ROBOT SORTING SIAP ===");
}

// ================= LOOP =================
void loop() {

  int kiri   = digitalRead(IR_KIRI);
  int tengah = digitalRead(IR_TENGAH);
  int kanan  = digitalRead(IR_KANAN);

  // ===== MONITOR RINGAN (biar tidak berat) =====
  float berat_monitor = scale.get_units(2) - offset;

  if (abs(berat_monitor) < 3) berat_monitor = 0;
  if (berat_monitor < 0) berat_monitor = 0;

  Serial.print("[MONITOR] Berat: ");
  Serial.print(berat_monitor);
  Serial.print(" g | IR: ");
  Serial.print(kiri);
  Serial.print("-");
  Serial.print(tengah);
  Serial.print("-");
  Serial.println(kanan);

  // ===== DETEKSI PERTIGAAN (LEBIH CEPAT) =====
  if (((kiri == HIGH && tengah == HIGH && kanan == HIGH) ||
       (kiri == HIGH && tengah == HIGH) ||
       (kanan == HIGH && tengah == HIGH)) && !sudahTimbang) {

    Serial.println(">>> CHECKPOINT TERDETEKSI");

    stopMotor();
    delay(300); // 🔥 dipercepat

    // ===== TIMBANG =====
    berat = scale.get_units(10) - offset;

    if (abs(berat) < 3) berat = 0;
    if (berat < 0) berat = 0;

    Serial.print(">>> HASIL TIMBANG: ");
    Serial.print(berat);
    Serial.println(" gram");

    // ===== SORTING =====
    if (berat < 100) {
      Serial.println("➡️ RINGAN → KIRI");
      belokKiri();
      delay(250); // 🔥 lebih presisi
    } 
    else if (berat < 150) {
      Serial.println("➡️ SEDANG → KANAN");
      belokKanan();
      delay(300);
    } 
    else {
      Serial.println("➡️ BERAT → MAJU");
      maju();
      delay(250);
    }

    sudahTimbang = true;
  }

  // ===== RESET SETELAH LEWAT =====
  if (kiri == LOW && tengah == LOW && kanan == LOW) {
    sudahTimbang = false;
  }

  // ===== LINE FOLLOWER NORMAL =====
  if (!sudahTimbang) {
    if (tengah == HIGH && kiri == LOW && kanan == LOW) {
      maju();
    } 
    else if (kiri == LOW && tengah == HIGH && kanan == HIGH) {
      belokKiri();
    } 
    else if (kanan == LOW && tengah == HIGH && kiri == HIGH) {
      belokKanan();
    } 
    else {
      stopMotor();
    }
  }

  delay(10); // 🔥 jauh lebih cepat
}

// ================= GERAK =================
void maju() {
  analogWrite(ENA, speedNormal);
  analogWrite(ENB, speedNormal);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void belokKiri() {
  analogWrite(ENA, speedPivot);
  analogWrite(ENB, speedPivot);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void belokKanan() {
  analogWrite(ENA, speedPivot);
  analogWrite(ENB, speedPivot);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

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