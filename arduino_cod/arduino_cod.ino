
// Pines L298N 
#define ENA  5
#define IN1  6
#define IN2  8
#define ENB  9
#define IN3  10
#define IN4  11

// Pines HC-SR04 
#define TRIG 3
#define ECHO 4

// ── Velocidad base ────────────────────────────
#define SPEED_NORMAL 230  // 0-255
#define SPEED_TURN   250
#define DIST_ALERT   30    

// Variables
unsigned long lastSensor = 0;
const int SENSOR_INTERVAL = 100;  // medir cada 100ms

String currentCmd = "S";
bool obstacle = false;

//  SETUP

void setup() {
  Serial.begin(9600);  // comunicación con ESP32

  // L298N
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();

  // HC-SR04
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Señal de inicio al ESP32
  Serial.println("M:listo");
}


//  LOOP

void loop() {


  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "F" || cmd == "B" || cmd == "L" ||
        cmd == "R" || cmd == "S") {
      currentCmd = cmd;
      executeCmd(cmd);
    }
    
    else if (cmd.startsWith("SPEAK:")) {
    
    }
  }

  // Medir distancia periódicamente 
  if (millis() - lastSensor >= SENSOR_INTERVAL) {
    lastSensor = millis();
    int dist = measureDistance();

    Serial.println("D:" + String(dist));

    // detectar obstáculo
    bool newObstacle = (dist > 0 && dist < DIST_ALERT);
    
    if (newObstacle != obstacle) {
      obstacle = newObstacle;
      Serial.println("O:" + String(obstacle ? "1" : "0"));
    }

    // LÓGICA DE DETECCIÓN 
    if (obstacle) {
      if (currentCmd == "F") {
        avoidObstacle(); // Si iba adelante, ejecuta rutina para esquivar
      } 
      else if (currentCmd != "S") { 
        stopMotors();    // Si iba atrás o lados, frena inmediatamente por seguridad
        currentCmd = "S";
        Serial.println("M:Freno de emergencia - Obstáculo detectado");
      }
    }
  }
}


//  CONTROL DE MOTORES
void executeCmd(String cmd) {
  if      (cmd == "F") moveForward();
  else if (cmd == "B") moveBackward();
  else if (cmd == "L") turnLeft();
  else if (cmd == "R") turnRight();
  else if (cmd == "S") stopMotors();
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENA, SPEED_NORMAL);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENB, SPEED_NORMAL);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  analogWrite(ENA, SPEED_NORMAL);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENB, SPEED_NORMAL);
}

void turnLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(ENA, SPEED_TURN);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENB, SPEED_TURN);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENA, SPEED_TURN);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENB, SPEED_TURN);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}


//  ESQUIVAR OBSTÁCULO 

void avoidObstacle() {
  Serial.println("M:obstáculo, esquivando");

  stopMotors();
  delay(150);

  moveBackward();
  delay(400);
  stopMotors();
  delay(150);

  turnRight();
  delay(500);
  stopMotors();
  delay(150);

  Serial.println("M:reanudando");
  currentCmd = "F";
  moveForward();
}

//  HC-SR04

int measureDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 25000);  // timeout 25ms
  if (duration == 0) return 999;  // sin eco = lejos o error de conexión
  return duration * 0.034 / 2;   // convertir a cm
}

