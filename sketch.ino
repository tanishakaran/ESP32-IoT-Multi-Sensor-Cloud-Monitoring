#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ---------- PIN DEFINITIONS ----------
#define DHTPIN 4
#define DHTTYPE DHT22

#define LDR_PIN 34
#define LED_PIN 5

#define TRIG_PIN 25
#define ECHO_PIN 26

#define RELAY_PIN 18

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ---------- WIFI ----------
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ---------- THINGSPEAK ----------
// We will add the API key here later.
String apiKey = "YOUR_API_KEY";

// ---------- OBJECTS ----------
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ---------- SETUP ----------
void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  // OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED initialization failed!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("IoT MONITOR");
  display.println("Initializing...");
  display.display();

  delay(2000);

  // Wi-Fi
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("--------------------------------");
  Serial.println("ESP32 IoT Cloud Monitoring");
  Serial.println("--------------------------------");
}

// ---------- LOOP ----------
void loop() {

  // ----- DHT22 -----
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("DHT22 reading failed!");

    delay(2000);
    return;
  }

  // ----- LDR -----
  int lightLevel = analogRead(LDR_PIN);

  // ----- ULTRASONIC -----
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  float distance = duration * 0.034 / 2;

  // ----- AUTOMATION -----

  // Dark → LED ON
  if (lightLevel < 2000) {

    digitalWrite(LED_PIN, HIGH);

  } else {

    digitalWrite(LED_PIN, LOW);
  }

  // Object close → Relay ON
  if (distance > 0 && distance < 20) {

    digitalWrite(RELAY_PIN, HIGH);

  } else {

    digitalWrite(RELAY_PIN, LOW);
  }

  // ----- SERIAL MONITOR -----

  Serial.println();
  Serial.println("========== IoT MONITOR ==========");

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    : ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Light Level : ");
  Serial.println(lightLevel);

  Serial.print("Distance    : ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("LED         : ");
  Serial.println(
    lightLevel < 2000 ? "ON" : "OFF"
  );

  Serial.print("Relay       : ");
  Serial.println(
    distance < 20 ? "ON" : "OFF"
  );

  Serial.println("================================");

  // ----- OLED -----

  display.clearDisplay();

  display.setCursor(0, 0);

  display.println("IoT MONITOR");

  display.print("T: ");
  display.print(temperature, 1);
  display.println(" C");

  display.print("H: ");
  display.print(humidity, 1);
  display.println(" %");

  display.print("L: ");
  display.println(lightLevel);

  display.print("D: ");
  display.print(distance, 1);
  display.println(" cm");

  display.print("Relay: ");
  display.println(
    distance < 20 ? "ON" : "OFF"
  );

  display.display();

  // ----- THINGSPEAK -----

  if (WiFi.status() == WL_CONNECTED &&
      apiKey != "YOUR_API_KEY") {

    HTTPClient http;

    String url =
      "http://api.thingspeak.com/update?api_key=" +
      apiKey +
      "&field1=" + String(temperature) +
      "&field2=" + String(humidity) +
      "&field3=" + String(lightLevel) +
      "&field4=" + String(distance) +
      "&field5=" + String(
        distance < 20 ? 1 : 0
      );

    http.begin(url);

    int httpResponseCode = http.GET();

    Serial.print("ThingSpeak Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }

  delay(15000);
}
