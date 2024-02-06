#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <DHT.h>

#define SS_PIN 10       // Define the Slave Select (SS) pin
#define RST_PIN 9       // Define the Reset pin
#define BUZZER_PIN 8    // Define the pin for the buzzer
#define TRIGGER_PIN 3   // Ultrasonic sensor trigger pin
#define ECHO_PIN 2      // Ultrasonic sensor echo pin
#define SERVO_PIN 5     // Servo motor control pin
#define DC_MOTOR_PIN 6  // DC motor control pin
#define LDR_PIN A0      // LDR sensor analog pin
#define GAS_SENSOR_PIN 4 // Placeholder, replace with actual pin for gas sensor
#define FIRE_SENSOR_PIN 7 // Placeholder, replace with actual pin for fire detector
#define DHT_PIN 12      // DHT11 sensor data pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
String tagUID;


String authorizedUID = "23 e1 44 a9";//Replace the following line with the authorized UID in hexadecimal form of your choice

Servo servoMotor;
DHT dht(DHT_PIN, DHT11);

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  SPI.begin();         // Initiate SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522 RFID module
  dht.begin();          // Initialize DHT11 sensor

  pinMode(BUZZER_PIN, OUTPUT);  // Set the buzzer pin as an output
  digitalWrite(BUZZER_PIN, LOW);  // Ensure the buzzer is initially off

  pinMode(TRIGGER_PIN, OUTPUT); // Set the trigger pin as an output
  pinMode(ECHO_PIN, INPUT);     // Set the echo pin as an input

  servoMotor.attach(SERVO_PIN); // Attach the servo to its pin

  pinMode(DC_MOTOR_PIN, OUTPUT);  // Set the DC motor pin as an output
  analogWrite(DC_MOTOR_PIN, 255); // Start the DC motor at max speed

  pinMode(LDR_PIN, INPUT); // Set the LDR sensor pin as an input

  pinMode(GAS_SENSOR_PIN, INPUT); // Set the gas sensor pin as an input
  pinMode(FIRE_SENSOR_PIN, INPUT); // Set the fire sensor pin as an input

  Serial.println("System ready. Waiting for object detection...");
}

void loop() {
  // Check for object detection using ultrasonic sensor
  if (isObjectDetected()) {
    // Object detected, now scan RFID card for access
    Serial.println("Object detected. Please scan your RFID card...");

    // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID: ");
      tagUID = getTagUID(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println(tagUID);

      // Check if the scanned card is authorized
      if (isAuthorizedCard(tagUID)) {
        Serial.println("Access granted. Door opening...");

        activateBuzzer();  // Activate the buzzer when access is granted

        // Add your door opening logic here
        activateUltrasonicAndServo();
        delay(10000); // Keep the door open for 10 seconds

        // Say "Door Close" and deactivate the servo
        Serial.println("Door Close");
        deactivateUltrasonicAndServo();

        // Check if it's dark (LDR value below a threshold) and turn on the light
        if (isDark()) {
          Serial.println("It's dark. Turning on the light...");
          // Add your specific light control logic here
          // For example: digitalWrite(LIGHT_PIN, HIGH);
        }

        Serial.println("Waiting for object detection...");
      } else {
        Serial.println("Access denied. Unauthorized card.");
        Serial.println("Waiting for object detection...");
      }
    }
  }

  // Check for gas leakage
  if (digitalRead(GAS_SENSOR_PIN) == HIGH) {
    Serial.println("Gas leakage detected! Activating alarm...");
    activateAlarm();
    delay(5000);  // Alarm duration (adjust as needed)
    deactivateAlarm();
  }

  // Check for fire
  if (digitalRead(FIRE_SENSOR_PIN) == HIGH) {
    Serial.println("Fire detected! Activating alarm...");
    activateAlarm();
    delay(5000);  // Alarm duration (adjust as needed)
    deactivateAlarm();
  }
  
  // Read temperature and humidity from DHT11 sensor
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    // Adjust the DC motor speed based on temperature
    int motorSpeed = map(temperature, 0, 50, 0, 255); // Map temperature range (0-50°C) to motor speed (0-255)
    analogWrite(DC_MOTOR_PIN, motorSpeed);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  delay(2000); // Wait for 2 seconds before next reading
}

bool isObjectDetected() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.0343) / 2;

  if (distance >= 10 && distance <= 100) { // Adjust the distance range as needed
    Serial.println("Object detected at a valid distance.");
    return true;
  } else {
    return false;
  }
}

bool isAuthorizedCard(String tagUID) {
  return tagUID == authorizedUID;
}

String getTagUID(byte *uid, byte uidSize) {
  String tagUID = "";
  for (byte i = 0; i < uidSize; i++) {
    tagUID += (uid[i] < 0x10 ? " 0" : " ") + String(uid[i], HEX);
  }
   return tagUID.substring(1);  // Remove the leading space
}

void activateBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer
  delay(2000);  // Buzzer sound duration (adjust as needed)
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer
}

void activateUltrasonicAndServo() {

  servoMotor.write(180); // Rotate servo motor to 180 degrees
  Serial.println("Door Open");
}

void deactivateUltrasonicAndServo() {

  servoMotor.write(0); // Rotate servo motor back to 0 degrees (closed position)
}

bool isDark() {
  // Adjust the threshold value as needed based on LDR sensor characteristics
  const int LDR_THRESHOLD = 500;
  int ldrValue = analogRead(LDR_PIN);
  return ldrValue < LDR_THRESHOLD;
}

void activateAlarm() {
  digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer for the alarm
}

void deactivateAlarm() {
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer to end the alarm
}
