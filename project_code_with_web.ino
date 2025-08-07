#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// Sensor Pin Definitions
#define LDRPin 15
#define NightLight 2
#define DHTPin 4
#define waterlevelinfoPin 36
#define moisterPin 39
#define rainsensorPin 5
#define SoilwaterPump 18
#define TankwaterPump 19

// DHT Sensor
DHT dht11(DHTPin, DHT11);

// Wi-Fi Credentials
const char* ssid = "Jolly Amin";
const char* password = "jollyamin038";

// Web Server on port 80
WebServer server(80);

// Sensor variables
int waterlevel = 0;
int moisterlevel = 0;
int rainstatus = 0;
float temread = 0;
float humread = 0;
int ldrlightstatus = 0;
String SoilStatus;
int waterPercent;

void setup() {
  Serial.begin(9600);
  analogSetAttenuation(ADC_11db);
  pinMode(waterlevelinfoPin, INPUT);
  pinMode(LDRPin, INPUT);
  pinMode(NightLight, OUTPUT);
  pinMode(TankwaterPump, OUTPUT);
  pinMode(SoilwaterPump, OUTPUT);
  pinMode(moisterPin, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  // Define route
  server.on("/", handleWebPage);
  server.begin();
}

void loop() {
  server.handleClient();

  // Light Sensor
  ldrlightstatus = digitalRead(LDRPin);
  digitalWrite(NightLight, ldrlightstatus == LOW ? LOW : HIGH);

  // Temp & Humidity sensor
  temread = dht11.readTemperature();
  humread = dht11.readHumidity();
  Serial.print("Temp : ");
  Serial.print(temread);
  Serial.println(" °C\t");

  Serial.print("Hum : ");
  Serial.print(humread);
  Serial.println(" %");

  // Water level Sensor
  waterlevel = analogRead(waterlevelinfoPin);
  waterPercent = map(waterlevel, 0, 1630, 0, 100);
  Serial.print("Water Level: ");
  if(waterPercent <= 30){
    Serial.println("Low -> ");
    digitalWrite(TankwaterPump, HIGH);
  } else if(waterPercent <= 80){
    Serial.println("Medium -> ");
  } else if(waterPercent > 80){
    Serial.println("High -> ");
    digitalWrite(TankwaterPump, LOW);
  }
  Serial.println(waterlevel);
  
  //for error reading
  if(waterPercent>100){
    waterPercent = 100;
  }
  // Moister Sensor
  moisterlevel = analogRead(moisterPin);
   
  Serial.print("Moister Level: ");
  if (moisterlevel > 3000){
    Serial.print("Dry -> ");
    SoilStatus = "Dry";
    digitalWrite(SoilwaterPump, HIGH);
  } else if(moisterlevel > 1500){
    Serial.print("Moist -> ");
    SoilStatus = "Moist";
    digitalWrite(SoilwaterPump, LOW);
  } else if(moisterlevel > 800){
    Serial.print("Wet -> ");
    SoilStatus = "Wet";
  } else {
    Serial.print("Submerged -> ");
    SoilStatus = "submerged";
  }
  Serial.println(moisterlevel);

  // Rain Sensor
  rainstatus = digitalRead(rainsensorPin);
  if(rainstatus == LOW){
    Serial.println("Rain Detected!");
  }

  delay(500);
}

// Web page handler
void handleWebPage() {

  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;background:#f0f0f0;}div{background:white;padding:20px;margin:20px auto;width:300px;border-radius:10px;box-shadow:0 0 10px #ccc;}</style>";
  html += "</head><body><h2>ESP32 Sensor Dashboard</h2><div>";
  html += "<p><strong>Temp:</strong> " + String(temread) + "°C</p>";
  html += "<p><strong>Humidity:</strong> " + String(humread) + " %</p>";
  html += "<p><strong>Light status:</strong> " + String(ldrlightstatus == LOW ? "Night" : "Day") + "</p>";
  html += "<p><strong>Water Level:</strong> " + String(waterPercent) + " %</p>";
  html += "<p><strong>Soil Status:</strong> " + String(SoilStatus) + " </p>";
  html += "<p><strong>Rain Status:</strong> " + String((rainstatus == LOW) ? "Rain Detected" : "No Rain") + "</p>";
  html += "<p><strong>Soil Water Pump:</strong> " + String((digitalRead(SoilwaterPump) == HIGH) ? "ON" : "OFF") + "</p>";
  html += "<p><strong>Tank Water Pump:</strong> " + String((digitalRead(TankwaterPump) == HIGH) ? "ON" : "OFF") + "</p>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}
