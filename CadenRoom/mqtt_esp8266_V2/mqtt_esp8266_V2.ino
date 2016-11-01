/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//DS18B20 setup
#define ONE_WIRE_BUS 4  // DS18B20 pin
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);

//DHT11 setup
#define DHTPIN 5     // GPIO 5 or D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE, 30); // 11 works fine for ESP8266
float humidity, temp_f;  // Values read from sensor
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 5000;              // interval at which to read sensor

//setup for 433mhz chip
RCSwitch mySwitch = RCSwitch();
const long outletSwitch[6][2] = {
  {4478268, 4478259},
  {4478412, 4478403},
  {4478732, 4478723},
  {4480268, 4480259},
  {4486412, 4486403}
};

// Update these with values suitable for your network.
const char* ssid = "Kubic2";
const char* password = "lk971121";
const char* mqtt_server = "192.168.1.15";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  dht.begin();           // initialize temperature sensor
  mySwitch.enableTransmit(2);       // set transmit pin to pin 2
  mySwitch.setPulseLength(187);     // set pulse length
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  mySwitch.setRepeatTransmit(15);    // optional set number of transmission repetitions
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //DS18B20.begin(); start up the DS18B20 library
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  mySwitch.send(outletSwitch[payload[0] - 1 - '0'][payload[1] - '0'], 24);
  Serial.println(outletSwitch[payload[0] - 1 - '0'][payload[1] - '0']);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("CadenESP8266")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("cadenSwitches");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;
    gettemperature();       // read sensor
    getDS18B20();           // read DS18B20 sensor and send temp to MQTT
  }
}

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also

  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp_f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    Serial.println(temp_f);
    Serial.println(humidity);
    char temp_f_char[10];
    char humidity_char[10];
    dtostrf(temp_f, 4, 1, temp_f_char);
    dtostrf(humidity, 4, 1, humidity_char);
    client.publish("cadenRoomTemp", temp_f_char);
    client.publish("cadenRoomHumid", humidity_char);
  }
}

void getDS18B20() {
  float temp;
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  temp = temp * 9 / 5 + 32;
  char temp_char[10];
  dtostrf(temp, 4, 1, temp_char);
  Serial.print("Temperature: ");
  Serial.println(temp);
  client.publish("cadenDS18B20", temp_char);
}

