#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
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

#define LEDPIN D4
#define DHTPIN D2 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void handleNewMessages(int numNewMessages);

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
  pinMode(LEDPIN, OUTPUT);
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
        digitalWrite(LED_BUILTIN, LOW);
      }
      if (text == "/ledoff"){
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
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