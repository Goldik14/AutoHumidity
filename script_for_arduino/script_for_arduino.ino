#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include "secrets.h"

WiFiClientSecure secured_client;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
const unsigned long BOT_MTBS = 5000;
unsigned long bot_lasttime = 0;
float temperature, humidity;
const float maxTemp = 30.0;
const float maxHum = 60.0, minHum = 35.0;
bool tempAlertSent = false;
bool humAlertSent = false;

#define DHTPIN D2 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

bool ledEnabled = true; // лента включена по умолчанию
int animationOffset = 0;
unsigned long lastAnimationTime = 0;
#define LED_PIN    D7
#define LED_COUNT  60
#define BRIGHTNESS 30
#define DOTS       4
#define GAP        8
#define SPEED      60
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define RELAY_IN D1

void handleNewMessages(int numNewMessages);
void SetColorLent();
void criticalTemperature();
void criticalHumidity();

void setup(){
  Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org");
  secured_client.setTrustAnchors(&cert);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  dht.begin();
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  pinMode(RELAY_IN, OUTPUT);
}

void loop(){
  if(millis()-bot_lasttime > BOT_MTBS){
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
    criticalTemperature();
    criticalHumidity();
  }
  SetColorLent();
}

void handleNewMessages(int numNewMessages){
  for (int i=0; i < numNewMessages; i++){
    if (bot.messages[i].chat_id == CHAT_ID){
      String text = bot.messages[i].text;
      if (text == "/start"){
        String startInfo = "👋 Привет! Я домашний бот на ESP.\n\n";
        startInfo += "Вот что я умею:\n";
        startInfo += "🌡 /temp — показать текущую температуру\n";
        startInfo += "💦 /humidity — показать влажность\n";
        startInfo += "💡 /ledon — включить LED ленту\n";
        startInfo += "💤 /ledoff — выключить LED ленту\n";
        bot.sendMessage(CHAT_ID, startInfo, "");
      }
      if (text == "/temp"){
        String temp = "🌡 Температуры комнаты в данный момент составляет: " + String(temperature) + "°С";
        bot.sendMessage(CHAT_ID, temp, "");
      }
      if (text == "/humidity"){
        String temp = "💦 Влажность комнаты в данный момент составляет: " + String(humidity) + "%";
        bot.sendMessage(CHAT_ID, temp, "");
      }
      if (text == "/ledon"){
        ledEnabled = true;
        bot.sendMessage(CHAT_ID, "💡 LED лента включена", "");
      }

      if (text == "/ledoff"){
        ledEnabled = false;
        strip.clear();
        strip.show();
        bot.sendMessage(CHAT_ID, "💤 LED лента выключена", "");
      }
      if(text == "/relayoff"){
        digitalWrite(RELAY_IN, HIGH);
        bot.sendMessage(CHAT_ID, "🔌 Реле выключено", "");
      }
      if(text == "/relayon"){
        digitalWrite(RELAY_IN, LOW);
        bot.sendMessage(CHAT_ID, "🔌 Реле включено", "");
      }
    }
  }
}

void SetColorLent() {
  if (!ledEnabled) return;  // если лента выключена — выходим

  if (millis() - lastAnimationTime < SPEED) return;
  lastAnimationTime = millis();

  strip.clear();

  uint32_t color;
  if (humidity > 60) color = strip.Color(128, 0, 128);       // 💜
  else if (humidity > 45) color = strip.Color(0, 255, 0);    // 💚
  else if (humidity > 35) color = strip.Color(255, 255, 0);  // 💛
  else color = strip.Color(255, 0, 0);                       // ❤️

  for (int start = -animationOffset; start < LED_COUNT; start += (DOTS + GAP)) {
    for (int i = 0; i < DOTS; i++) {
      int pos = start + i;
      if (pos >= 0 && pos < LED_COUNT) {
        int mirroredPos = (LED_COUNT - 1) - pos;
        strip.setPixelColor(mirroredPos, color);
      }
    }
  }

  strip.show();

  animationOffset++;
  if (animationOffset > LED_COUNT + DOTS + GAP) {
    animationOffset = 0;
  }
}

void criticalTemperature() {
  if (temperature >= maxTemp) {
    if (!tempAlertSent) {
      String crittemp = "🔥 Температура слишком высокая: " + String(temperature) + "°С\nПроветрите помещение!";
      bot.sendMessage(CHAT_ID, crittemp, "");
      tempAlertSent = true;
    }
  } else {
    tempAlertSent = false;
  }
}

void criticalHumidity() {
  if (humidity >= maxHum) {
    if (!humAlertSent) {
      String crithum = "💧 Слишком влажно: " + String(humidity) + "%\nПроветрите помещение!";
      bot.sendMessage(CHAT_ID, crithum, "");
      humAlertSent = true;
    }
  } else if (humidity <= minHum) {
    if (!humAlertSent) {
      String crithum = "🌵 Слишком сухо: " + String(humidity) + "%\nВключите увлажнитель!";
      bot.sendMessage(CHAT_ID, crithum, "");
      humAlertSent = true;
    }
  } else {
    humAlertSent = false; 
  }
}