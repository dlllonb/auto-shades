// glasses project code file!

// pins for distance sensor, change these to whatever pins you want
const int pingPin = 13; 
const int echoPin = 12;

// add other pins 

// for distance sensor 
const float c = 3.43e5;
long duration;

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
  
  if


}

void servos(){


}

void light(){
  
}

void distance(){

}

void bluetooth(){

}


