// ---------------------------------------------------------------------------
// This example code was used to successfully communicate with 15 ultrasonic sensors. You can adjust
// the number of sensors in your project by changing SONAR_NUM and the number of NewPing objects in the
// "sonar" array. You also need to change the pins for each sensor for the NewPing objects. Each sensor
// is pinged at 33ms intervals. So, one cycle of all sensors takes 495ms (33 * 15 = 495ms). The results
// are sent to the "oneSensorCycle" function which currently just displays the distance data. Your project
// would normally process the sensor results in this function (for example, decide if a robot needs to
// turn and call the turn function). Keep in mind this example is event-driven. Your complete sketch needs
// to be written so there's no "delay" commands and the loop() cycles at faster than a 33ms rate. If other
// processes take longer than 33ms, you'll need to increase PING_INTERVAL so it doesn't get behind.
// ---------------------------------------------------------------------------
#include <NewPing.h>

#define SONAR_NUM     2 // Number of sensors.
#define MAX_DISTANCE 200 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
unsigned int cupsLeft[SONAR_NUM];   // How many cups are left of dog food
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(12, 11, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(10, 9, MAX_DISTANCE)
};

// motion sensor pins
int motionPin1 = 8;                   // pin for HC-SR501 motion sensor module
int motionPin2 = 7;                   // pin for HC-SR501 motion sensor module
unsigned long previousMillis = 0;
const long interval = 1000;           // interval at which to
bool lastMotionState1 = 0;             // keeps track of motion state, tracking edge detection
bool lastMotionState2 = 1;             // keeps track of motion state, tracking edge detection
bool motionState1 = 0;                // state of motion sensor 1
bool motionState2 = 0;                // state of motion sensor 2


void setup() {
  Serial.begin(115200);
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  // motion sensor setup
  pinMode(motionPin1, INPUT);
  pinMode(motionPin2, INPUT);
}

void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle();// Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  // Other code that *DOESN'T* analyze ping results can go here.
  if (Serial.available() > 0) {
    if (Serial.read() == 'r') {
      sendData();
    }
  }
  motionState1 = digitalRead(motionPin1);
  motionState2 = digitalRead(motionPin2);
  if (motionState1 && !lastMotionState1) {        // motion detected (edge rising)
    Serial.println("Motion1=1"); 
    lastMotionState1 = 1;
  }
  if (!motionState1 && lastMotionState1) {        // motion gone (edge falling)
    Serial.println("Motion1=0");
    lastMotionState1 = 0;
  }
  if (motionState2 && !lastMotionState2) {        // motion detected (edge rising)
    Serial.println("Motion2=1"); 
    lastMotionState2 = 1;
  }
  if (!motionState2 && lastMotionState2) {        // motion gone (edge falling)
    Serial.println("Motion2=0");
    lastMotionState2 = 0;    
  }
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    cupsLeft[i] = cm[i];
  }
}

void sendData() {
  Serial.print("foodLeft1=");
  Serial.println(cupsLeft[0]);
  Serial.print("foodLeft2=");
  Serial.println(cupsLeft[1]);

}

