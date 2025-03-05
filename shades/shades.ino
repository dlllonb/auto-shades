#include <Servo.h>
#include <ArduinoBLE.h>

// HCI FINAL PROJECT

//pins
const int pingPin = 4; 
const int echoPin = 5;
const int servo1Pin = 7; 
const int servo2Pin = 6; 
const int ldrPin = A0;

//Bluetooth setup
//taken from class example
const char* name = "ShadeControl";
BLEService shadeService("19B10010-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic shadeCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

/* GLOBAL VARS */
//Speed of sound (mm/s)
const float c = 3.43e5;
//Distance threshold (mm)
const float distThresh = 200;

//Hysteresis buffer to prevent rapid toggling
//https://www.hwe.design/theories-concepts/hysteresis
//this was a really interesting concept -> imagine a thermometer toggling between heating and cooling 
// at 70 degrees. This buffer would allow the heating to change if the temperature was either one degree
// over or one degree under 70 (for example)
const int distBuffer = 10;

//the next two variables are used to make the distance sensor less sensitive to changes and noise
// we had issues where one swipe would be picked up as two and would not change the state of the shades
// after some testing we found 10 and 7 to be the best numbers to pick up our individual signals to the sensor
//Num samples for filtering
const int debounceNum = 10;
//Num of triggers needed to proceed
const int requiredTriggers = 7;

//5 second for light sensor after distance trigger
//this prevents light sensor from triggering the shades after the distance sensor is activated (for 5 seoncds)
// for the sake of time and the demo, we set this to 5 seconds in order to show you more rapidly how this works
// if this were a real product it would probably be much longer lol
const unsigned long lightBlockTime = 5000;
//1 second block for distance sensor
//this allows the servo to move into place before the distance sensor is able to be set off again
const unsigned long distanceBlockTime = 1000;
//5 second block for light sensor after BT trigger
//this prevents the light sensor from affecting the state of the shades input based on the BT sensor (for 5 seconds)
const unsigned long BTBlockTime = 5000;
//Light sensor threshold
const int lightThreshold = 600;

//bool to show whether shades are up or down
bool shadesUp = true;

//bool to control whether shades should be toggled
bool toggle = false;

// Servos
Servo servo1;
Servo servo2;

//last distance sensor trigger
unsigned long lastDistanceTrigger = 0;
//distance sensor blocking
unsigned long lastDistanceToggle = 0;
// last blocking light after BT trigger
unsigned long lastBluetoothTrigger = 0;

int upAngle1 = 65;
int downAngle1 = 120;
//2 has the distance sensor lol
int upAngle2 = 120;
int downAngle2 = 70;

void setup() {
  Serial.begin(9600);
  //servo pins
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo1.write(upAngle1);
  servo2.write(upAngle2);
  //dist pins
  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(pingPin, LOW);

  //BT setup
  //this was all taken from the BLE example from class
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1); //remains stuck until BLE connects
  }

  BLE.setLocalName(name); // set the local name peripheral advertises
  BLE.setAdvertisedService(shadeService); // set the UUID for the service this peripheral advertises:
  shadeService.addCharacteristic(shadeCharacteristic); // add the characteristics to the service, 1 for SHADES UP
  BLE.addService(shadeService); // add the service
  //start with shades down
  shadeCharacteristic.writeValue(0);
  BLE.advertise(); // start advertising
  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop() {
  //check BT input
  bluetooth();

  // check if distance sensor can activate (based on the 1 second block)
  if (millis() - lastDistanceToggle > distanceBlockTime) {
    distance();
  }

  // light sensor can only activate after 5 seconds since last distance OR BT trigger
  if ((millis() - lastDistanceTrigger > lightBlockTime) && (millis() - lastBluetoothTrigger > BTBlockTime)) {
    light();
  }

  servo();
}

void servo() {
  //if toggle is set, move servos instantly to specific position
  if (toggle) {
    toggle = false;
    shadesUp = !shadesUp;

    int targetAngle1 = shadesUp ? upAngle1 : downAngle1;
    int targetAngle2 = shadesUp ? upAngle2 : downAngle2;
    servo1.write(targetAngle1);
    servo2.write(targetAngle2);
    Serial.print("Shades Status: ");
    Serial.println(shadesUp ? "UP" : "DOWN");
  }
}

void bluetooth() {
  BLE.poll(); // poll for Bluetooth® Low Energy events

  if (shadeCharacteristic.written()) {
    // update Shade Status, either central has written to characteristic or state has changed
    int command = shadeCharacteristic.value();
    Serial.print("BT Received: ");
    Serial.println(command);

    if (command == 1 && !shadesUp) {
        //shades up
        toggle = true;
    } else if (command == 0 && shadesUp) {
        //shades down
        toggle = true;
    }
    // start the 5 second block for the light sensor to stop it from triggering
    lastBluetoothTrigger = millis();
  }
}

void distance() {
  int triggerCount = 0;
  float totalDistance = 0;

  //taking multiple readings for smoothing out
  for (int i = 0; i < debounceNum; i++) {
    // start with a clean signal
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    // send trigger signal
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    //return pulse duration in microseconds
    // if set to HIGH, pulseIn() waits for the pin to go from LOW to HIGH
    // stops timing when pin goes back LOW
    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 1.e-6 * c / 2;
    totalDistance += distance;

    //wait for 7 trigger counts to compute final distance used
    if (distance < distThresh) {
      triggerCount++;
    }

    //small delay between taking samples(it was quite fase)
    delay(5);
  }

  //compute average distance
  float avgDistance = totalDistance / debounceNum;
  //prints for debugging
  Serial.print("Avg distance: ");
  Serial.println(avgDistance);

  //use hysteresis
  //https://www.hwe.design/theories-concepts/hysteresis
  //if shades are down ...
  if (!shadesUp && avgDistance < (distThresh - distBuffer) && triggerCount >= requiredTriggers) {
    toggle = true;
    //start 5 second block for light sensor
    lastDistanceTrigger = millis();
    //start 1 second block for distance sensor
    lastDistanceToggle = millis();
  }
  //if shades are already up ...
  else if (shadesUp && avgDistance > (distThresh + distBuffer) && triggerCount >= requiredTriggers) {
    toggle = true;
    //start 5 second block for light sensor
    lastDistanceTrigger = millis();
    //start 1 second block for distance sensor
    lastDistanceToggle = millis();
  }
}

void light() {
  //light can only toggle when 5 second cool down is over
  int lightValue = analogRead(ldrPin);
  //prints for debugging
  Serial.print("Light value: ");
  Serial.println(lightValue);

  if (lightValue > lightThreshold && shadesUp) {
    //shades down
    toggle = true;
  } else if (lightValue < lightThreshold && !shadesUp) {
    //shades up
    toggle = true;
  }
}
