// glasses project code file!

// pins for distance sensor, change these to whatever pins you want
const int pingPin = 13; 
const int echoPin = 12;

// add other pins 

// for distance sensor 
const float c = 3.43e5;
long duration;
float distThresh = 200; 

bool shades = false; // whether shades are up or down, possibly unneeded 
bool toggle = false; // whether the shades should be toggled

void setup() {
  // set up servo pins here 

  // set up distance sensor
  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(pingPin, LOW);

  // set up light sensor?

  // set up bluetooth?


  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  distance();
  light();
  bluetooth();

  if (toggle) {
    toggle = false;
    servos();
    shades = !shades;
  }

}

void servos(){

}

void distance(){
  // distance sensor can toggle the shade position
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  durationX = pulseIn(echoPin, HIGH);
  float distance = duration * 1.e-6 * c / 2;

  if (distance < distThresh) {
    toggle = true;
  }
}

void light(){
  // light sensor can toggle the shade position
}

void bluetooth(){
  // bluetooth input can toggle the shade position
}


