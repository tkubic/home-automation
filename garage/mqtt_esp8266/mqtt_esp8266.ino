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

#define TERM_CHAR '\n'
#define DELIMITER ','

// Update these with values suitable for your network.

const char* ssid = "Kubic2";
const char* password = "lk971121";
const char* mqtt_server = "192.168.1.15";
char inChar; // Where to store the character read
char inString[10]; // Input string being read
byte myindex; // Index for serial input string
char data[6][10];
int counter;
String serialDataIn;
int inbyte;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  counter = 0;
  serialDataIn = String("");
  //data[0] = "garageDoor";
  //data[1] = "0";
  //data[2] = "garageBay1";
  //data[3] = "0";
  //data[4] = "garageBay2";
  //data[5] = "0";
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    //Serial.print((char)payload[i]);
  }
  //Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') 
  {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else 
  {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) 
    {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } 
    else 
    {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  if (Serial.available() > 0) 
  {
    inbyte = Serial.read();
    if(inbyte != TERM_CHAR || inbyte != DELIMITER)
    {
      inString[myindex] = inbyte;
      myindex = myindex + 1;
    } else if (inbyte = DELIMITER)
    {
      data[counter] == inString;
      Serial.println("DELIMIER found");
      Serial.println(data[counter]);
      memset(inString,0,sizeof(inString));
      counter++;
      Serial.println(counter);
      myindex = 0;                       // reset index
    } else if (inbyte = TERM_CHAR)
    {
      data[counter] == inString;
      Serial.println("Term char found!);
      Serial.print(data[0]);
      Serial.println(data[1]);
      Serial.print(data[2]);
      Serial.println(data[3]);
      Serial.print(data[4]);
      Serial.println(data[5]);
      memset(inString,0,sizeof(inString));
      counter = 0;
      publishMsg();
      myindex = 0;                       // reset index
    }
  }
  long now = millis();
  if (now - lastMsg > 5000) 
  {
    lastMsg = now;
    Serial.print("r");
  }
}

void publishMsg() 
{
  //char test1[] = "test1";
  //char test2[] = "test2";
  //client.publish(test1, test2);
  Serial.println("publishing data");
  Serial.print(data[0]);
  Serial.println(data[1]);
  Serial.print(data[2]);
  Serial.println(data[3]);
  Serial.print(data[4]);
  Serial.println(data[5]);
  client.publish(data[0], data[1]);
  client.publish(data[2], data[3]);
  client.publish(data[4], data[5]);
  //client.publish("test", "2");
}

