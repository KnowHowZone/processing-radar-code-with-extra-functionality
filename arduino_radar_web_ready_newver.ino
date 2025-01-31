// Includes the Servo library
#include <Servo.h>

// Ultrasonic Sensor Pins
const int trigPin = 10;
const int echoPin = 11;

// Servo Motor Control
Servo myServo;

// Variables for distance measurement
long duration;
int distance;
const int maxReadings = 3;  // Number of samples for filtering
int distanceBuffer[maxReadings] = {0}; // Store readings for smoothing
int bufferIndex = 0;

// Angle tracking
int currentAngle = 15;

// Serial data storage
String dataString = "";

// Function prototypes
int calculateDistance();
void smoothMove(int from, int to, int stepDelay);
void scanMode();

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myServo.attach(12);
  Serial.begin(115200); // Increase baud rate for faster serial communication
  Serial.println("Ready"); // Indicate readiness
}

void loop() {
  if (Serial.available() > 0) {
    dataString = Serial.readStringUntil('\n');
    if (dataString == "SCAN") {
      currentAngle = 15;
      scanMode();
      Serial.println("SCAN COMPLETE"); // Signal completion
    }
  }
}

// **Main Scanning Function**
void scanMode() {
  // Forward Sweep
  smoothMove(currentAngle, 165, 8);

  // Reverse Sweep
  smoothMove(165, currentAngle, 8);
  
  // Increment angle for dynamic scanning
  currentAngle++;
}

// **Smooth Servo Movement for High-Speed Scanning**
void smoothMove(int from, int to, int stepDelay) {
  int stepSize = (from < to) ? 1 : -1;

  for (int i = from; i != to + stepSize; i += stepSize) {
    myServo.write(i);
    delayMicroseconds(stepDelay * 1000); // Reduced delay for fast response

    distance = calculateDistance();
    
    Serial.print(i);
    Serial.print(",");
    Serial.print(distance);
    Serial.println("."); // **Ensure fast serial output with no redundant delays**
  }
}

// **Ultrasonic Sensor Data Processing**
int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 25000); // Reduce timeout for fast response
  if (duration == 0) return -1; // No echo detected

  int rawDistance = duration * 0.034 / 2;
  if (rawDistance > 400) return -1; // Discard out-of-range values

  // **Moving Average Filter for Noise Reduction**
  distanceBuffer[bufferIndex] = rawDistance;
  bufferIndex = (bufferIndex + 1) % maxReadings;

  int sum = 0, count = 0;
  for (int i = 0; i < maxReadings; i++) {
    if (distanceBuffer[i] > 0) { // Ignore invalid readings
      sum += distanceBuffer[i];
      count++;
    }
  }
  return (count > 0) ? sum / count : -1;
}
