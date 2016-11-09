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
#include <OneWire.h>
#include <DallasTemperature.h>

//DS18B20 setup
#define ONE_WIRE_BUS 4  // DS18B20 pin
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);

float humidity, temp_f;  // Values read from sensor
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 5000;              // interval at which to read sensor

// Update these with values suitable for your network.
const char* ssid = "temp_logger";
const char* password = "123456789";
const char* mqtt_server = "172.20.0.1";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
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
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("RyanTemps")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("Nothing");
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
    getTemps();           // read DS18B20 sensor and send temp to MQTT
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
  //client.publish("cadenDS18B20", temp_char);
}

void getTemps() {
  oneWire.reset_search();               // reset search
  uint8_t address[8];                   // placeholder for address
  uint8_t count = 0;                    // placeholder for sensor count

  if (oneWire.search(address))
  {
    DS18B20.requestTemperatures();        // get temperature of all sensors
  }
  do {
    count++;
    Serial.println(DS18B20.getTempF(address));
    Serial.println("  {");
    for (uint8_t i = 0; i < 8; i++)
    {
      Serial.print("0x");
      if (address[i] < 0x10) Serial.print("0");
      Serial.print(address[i], HEX);
      if (i < 7) Serial.print(", ");
    }
    Serial.println("  },");
  } while (oneWire.search(address));

  Serial.println("};");
  Serial.print("// nr devices found: ");
  Serial.println(count);
}

