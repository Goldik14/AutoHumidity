#define REMOTEXY_MODE__WIFI_CLOUD
#include <ESP8266WiFi.h>
#define REMOTEXY_WIFI_SSID "ТУТ_БЫЛ_МОЙ_ВАЙФАЙ"
#define REMOTEXY_WIFI_PASSWORD "ТУТ_БЫЛ_МОЙ_ПАРОЛЬ_ОТ_ВАЙФАЯ"
#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com"
#define REMOTEXY_CLOUD_PORT 6376
#define REMOTEXY_CLOUD_TOKEN "ТУТ_БЫЛ_МОЙ_ТОКЕН"
#include <RemoteXY.h>
#include <DHT.h>

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   
  { 255,0,0,16,0,59,0,19,0,0,0,65,117,116,111,72,117,109,105,100,
  105,116,121,0,31,1,106,200,1,1,4,0,67,11,67,40,10,78,2,26,
  2,67,58,67,40,10,78,2,26,2,68,11,79,40,40,1,8,36,68,58,
  79,40,40,1,8,36 };
  
struct {
  float value_01;
  float value_02;
  float onlineGraph_01_var1;
  float onlineGraph_02_var1;
  uint8_t connect_flag;

} RemoteXY;   
#pragma pack(pop)

void setup() 
{
  RemoteXY_Init (); 
  dht.begin();
}

void loop() 
{ 
  RemoteXY_Handler ();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  RemoteXY.value_01 = t;
  RemoteXY.value_02 = h;
  RemoteXY.onlineGraph_01_var1 = t;
  RemoteXY.onlineGraph_02_var1 = h;
}