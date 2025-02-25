// these can be any two digital pins I think
const int pingPin = 13;
const int echoPin = 12;

const float c = 3.43e5;
long duration;

void setup() {
    pinMode(pingPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(pingPin, LOW);
    Serial.begin(9600);
}

void loop() {
    // this part does the sensing, i think we can just deal with the 10 microsecond block
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    durationX = pulseIn(echoPin, HIGH);
    float distance = duration * 1.e-6 * c / 2;

    // i forget the units but I believe it's mm? 
    Serial.print("dist:");
    Serial.println(distanceX);
}
