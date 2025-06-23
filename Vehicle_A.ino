#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Vishal";
const char* password = "Vishal123";

// MQTT Broker details
const char* mqtt_server = "test.mosquitto.org"; // e.g., "broker.hivemq.com"
const int mqtt_port = 1883; // Default MQTT port
const char* mqtt_user = ""; // If authentication is required
const char* mqtt_password = ""; // If authentication is required

// MQTT Topics
const char* mqtt_topic = "robot/car/control";

// Motor pins
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  
  delay(10);
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
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Control the robot car based on the received message
  if (message == "F") {
    moveForward();
  } else if (message == "B") {
    moveBackward();
  } else if (message == "L") {
    turnLeft();
  } else if (message == "R") {
    turnRight();
  } else if (message == "S") {
    stopCar();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("NodeMCUClient", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Initialize motor pins as output
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(D0,OUTPUT);
  pinMode(D5,OUTPUT);

  analogWrite(D0, 60);
  analogWrite(D5, 60);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Motor control functions
void moveForward() {
 digitalWrite(IN1, HIGH);  // Motor A forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  // Motor B forward
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);   // Motor A backward
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);   // Motor B backward
  digitalWrite(IN4, HIGH);
}


void turnLeft() {
  digitalWrite(IN1, LOW);   // Motor A stop or reverse
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);  // Motor B forward
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);  // Motor A forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);   // Motor B stop or reverse
  digitalWrite(IN4, HIGH);
  
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


  
 