#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL50sQ2TLu1"
#define BLYNK_TEMPLATE_NAME "toets2"
#define BLYNK_AUTH_TOKEN "DgKLAOM5X34tRgVgCAkEmZAvbn5Jh9RM"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "embed2";
char pass[] = "weareincontrol";

const int btnDown = 6;      
const int btnUp   = 7;      
const int ketelLED = 8;

float gewensteTemp = 20.0;
float gemetenTemp = 0.0;

bool ketelAan = false;

unsigned long lastPressUp = 0;
unsigned long lastPressDown = 0;

BlynkTimer timer;

BLYNK_WRITE(V3) {
  gewensteTemp = param.asFloat();
  if (gewensteTemp < 0) gewensteTemp = 0;
  if (gewensteTemp > 30) gewensteTemp = 30;

  Serial.print("Blynk nieuwe gewenste temp: ");
  Serial.println(gewensteTemp);

  Blynk.virtualWrite(V1, gewensteTemp);
}

void meetTemperatuur() {
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println(" DHT11 niet gevonden... opnieuw proberen...");

    ketelAan = false;
    digitalWrite(ketelLED, LOW);

    Blynk.virtualWrite(V0, -1);  
    Blynk.virtualWrite(V2, 0);

    return;  
  }

  gemetenTemp = t;

  ketelAan = (gemetenTemp < gewensteTemp);
  digitalWrite(ketelLED, ketelAan ? HIGH : LOW);

  Serial.print("Gemeten: ");
  Serial.print(gemetenTemp);
  Serial.print(" °C | Gewenst: ");
  Serial.print(gewensteTemp);
  Serial.print(" °C | Ketel: ");
  Serial.println(ketelAan ? "AAN" : "UIT");

  Blynk.virtualWrite(V0, gemetenTemp);
  Blynk.virtualWrite(V1, gewensteTemp);
  Blynk.virtualWrite(V2, ketelAan ? 1 : 0);
}

void checkButtons() {
  if (digitalRead(btnDown) == LOW && millis() - lastPressDown > 250) {
    gewensteTemp -= 0.5;
    if (gewensteTemp < 0) gewensteTemp = 0;
    lastPressDown = millis();
    Blynk.virtualWrite(V1, gewensteTemp);
    Blynk.virtualWrite(V3, gewensteTemp);
  }

  if (digitalRead(btnUp) == LOW && millis() - lastPressUp > 250) {
    gewensteTemp += 0.5;
    if (gewensteTemp > 30) gewensteTemp = 30;
    lastPressUp = millis();
    Blynk.virtualWrite(V1, gewensteTemp);
    Blynk.virtualWrite(V3, gewensteTemp);
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(ketelLED, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  timer.setInterval(1000L, meetTemperatuur); 
  timer.setInterval(100L, checkButtons);      
}

void loop() {
  Blynk.run();
  timer.run();
}
