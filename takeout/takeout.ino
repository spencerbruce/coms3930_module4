#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
  
// Conncet the port of the stepper motor driver
int outPorts[] = {14, 27, 26, 25};
Servo myservo;  // create servo object to control a servo

int posVal = 0;    // variable to store the servo position
int servoPin = 15; // Servo motor pin

String address= "LINK-TO-WEB-SERVER"; // for conneceting to the server

const char *ssid_Router     = "SSID"; //Enter the router name
const char *password_Router = "PWORD"; //Enter the router password

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(outPorts[i], OUTPUT);
  }
  myservo.setPeriodHertz(50);           // standard 50 hz servo
  myservo.attach(servoPin, 500, 2500);  // attaches the servo on servoPin to the servo object
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup start");
  WiFi.begin(ssid_Router, password_Router);
  Serial.println(String("Connecting to ")+ssid_Router);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup End");
  delay(2000);
  moveSteps(false, 3 * 64, 1); // quick movement to let me know that connection was successful 
  moveSteps(true, 3 * 64, 1);
}

void loop() {
  
  if((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(address);
 
    int httpCode = http.GET(); // start connection and send HTTP header
    if (httpCode == HTTP_CODE_OK) { 
        String response = http.getString();
        if (response.equals("false")) {
            // Do not run sculpture, perhaps sleep for a couple seconds
            delay(3000);
        }
        else if(response.equals("true")) {
            // Run sculpture
            moveSteps(false, 8 * 64, 3);
            delay(1000);
            for (posVal = 180; posVal >= 0; posVal -= 1) { // goes from 0 degrees to 180 degrees
              // in steps of 1 degree
              myservo.write(posVal);       // tell servo to go to position in variable 'pos'
              delay(10);                   // waits 15ms for the servo to reach the position
            }
            for (posVal = 0; posVal <= 180; posVal += 1) { // goes from 180 degrees to 0 degrees
              myservo.write(posVal);       // tell servo to go to position in variable 'pos'
              delay(10);                   // waits 15ms for the servo to reach the position
            }
            moveSteps(true, 4 * 64, 3);
            moveSteps(false, 3 * 64, 1);
            moveSteps(true, 3 * 64, 1);
            moveSteps(false, 3 * 64, 1);
            moveSteps(true, 7 * 64, 1);
            delay(1000);
        }
        Serial.println("Response was: " + response);
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(500); // sleep for half of a second
  }
}

//Suggestion: the motor turns precisely when the ms range is between 3 and 20
void moveSteps(bool dir, int steps, byte ms) {
  for (unsigned long i = 0; i < steps; i++) {
    moveOneStep(dir); // Rotate a step
    delay(constrain(ms,3,20));        // Control the speed
  }
}

void moveOneStep(bool dir) {
  // Define a variable, use four low bit to indicate the state of port
  static byte out = 0x01;
  // Decide the shift direction according to the rotation direction
  if (dir) {  // ring shift left
    out != 0x08 ? out = out << 1 : out = 0x01;
  }
  else {      // ring shift right
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }
  // Output singal to each port
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}

void moveAround(bool dir, int turns, byte ms){
  for(int i=0;i<turns;i++)
    moveSteps(dir,32*64,ms);
}
void moveAngle(bool dir, int angle, byte ms){
  moveSteps(dir,(angle*32*64/360),ms);
}
