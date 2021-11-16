#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

#define DHTPIN 25
#define DHTTYPE DHT11

const char* ssid = "MEGACABLE-1A9G";
const char* password =  "URswM729";
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* clientid = "JerryESP32";
char jsonOutput[256];
const int disId = 123678;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(0x23);

void setup() {

  Serial.begin(115200);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 initialised"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Conectando a Broquer MQTT...");
    if (client.connect(clientid)) {
      Serial.println("connected");
      Serial.print("Return code: ");
      Serial.println(client.state());

    } else {

      Serial.print("conexion fallida ");
      Serial.print(client.state());
      delay(2000);

    }
  }

}

void loop() {
  int lec = analogRead(34);
  Serial.print("Read: ");
  Serial.println(lec);
  delay(1000);

  int gh = map(lec, 4095, 0, 0, 100);
  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("La Humedad es del: ");
  Serial.print(gh);
  Serial.println("%");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  Serial.print("Environment Humidity: ");
  Serial.print(h);
  Serial.print("% ,Temperature: ");
  Serial.print(t);
  Serial.println("°C ");

  if (lux <= 10 && lux >= 0) {
    Serial.print("No hay luz en el ambiente: ");
  }
  else if (lux <= 50) {
    Serial.print("Hay muy poca luz en el ambiente: ");
  }
  else if (lux <= 200) {
    Serial.print("El ambiente es de interiores oscuros: ");
  }
  else if (lux <= 400) {
    Serial.print("Interior con poca luz: ");
  }
  else if (lux <= 1000) {
    Serial.print("Hay luz en el ambiente interior: ");
  }
  else if (lux <= 5000) {
    Serial.print("Mucha luz en el interior: ");
  }
  else if (lux <= 10000) {
    Serial.print("Oscuro exterior: ");
  }
  else if (lux <= 30000) {
    Serial.print("Nublado exterior: ");
  }
  else if (lux <= 100000) {
    Serial.print("Luz de sol directa: ");
  }
  else {
    Serial.print("Error Cargando el sensor: ");
  }
  Serial.print(lux);
  Serial.println(" lx");

  const size_t CAPACITY = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
  object["Id"] = disId;
  object["GroundHumidity"] = gh;
  object["EnvironmentHumidity"] = h;
  object["Temperature"] = t;
  object["Lighting"] = lux;
  serializeJson(doc, jsonOutput);
  Serial.println(String(jsonOutput));
  client.loop();
  client.publish( "nodejs/mqtt/ifarm", jsonOutput);
  delay(60000);
}
