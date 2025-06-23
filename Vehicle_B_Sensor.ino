#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Vishal";
const char* password = "Vishal123";

// MQTT Broker details
const char* mqtt_server = "broker.emqx.io"; // e.g., "broker.hivemq.com"
const int mqtt_port = 1883; // Default MQTT port
const char* mqtt_user = ""; // If authentication is required
const char* mqtt_password = ""; // If authentication is required

// MQTT Topics
const char* mqtt_topic_control = "robot/carB/control";
const char* mqtt_topic_temperature = "robot/carB/temperature";
const char* mqtt_topic_humidity = "robot/carB/humidity";
const char* mqtt_topic_obstacle = "robot/carB/obstacle"; // New topic for obstacle detection

// Motor pins
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4

// Ultrasonic Sensor pins
#define TRIGGER_PIN D7
#define ECHO_PIN D6

// DHT Sensor pins and type
#define DHTPIN D8     // Pin where the DHT sensor is connected
#define DHTTYPE DHT11 // DHT 11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

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
      client.subscribe(mqtt_topic_control);
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

  pinMode(D0, OUTPUT);
  pinMode(D5, OUTPUT);

  analogWrite(D0, 80);
  analogWrite(D5, 80);

  // Initialize Ultrasonic Sensor pins
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize DHT sensor
  dht.begin();

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

  // Read temperature and humidity data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish temperature and humidity data to MQTT topics
  char tempString[8];
  char humString[8];
  dtostrf(temperature, 1, 2, tempString);
  dtostrf(humidity, 1, 2, humString);

  client.publish(mqtt_topic_temperature, tempString);
  client.publish(mqtt_topic_humidity, humString);

  // Print data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C, Humidity: ");
  Serial.println(humidity);

  // Measure distance using ultrasonic sensor
  long distance = measureDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check for obstacle within 20 cm
  if (distance <= 20) {
    stopCar(); // Stop the car if obstacle is detected
    client.publish(mqtt_topic_obstacle, "Obstacle Detected"); // Send obstacle message
    Serial.println("Obstacle Detected!");
  }

  // Delay between readings
  delay(2000); // Adjust the delay as needed
  if (distance >= 20) {
    stopCar(); // Stop the car if obstacle is detected
    client.publish(mqtt_topic_obstacle, "Obstacle  NOT Detected"); // Send obstacle message
    Serial.println("Obstacle NOT  Detected!");
  }

}

// Function to measure distance using ultrasonic sensor
long measureDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration / 2) / 29.1; // Convert to cm
  return distance;
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