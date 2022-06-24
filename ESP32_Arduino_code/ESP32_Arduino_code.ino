#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "DHT.h"

#define DHTPIN 18

#define DHTTYPE DHT11

#define LED_PIN 2

#define SERVER_TIMER 900

const char* ssid = " . . . ";
const char* password = " . . . ";


TaskHandle_t xHandle = NULL;

float temp_c = 0;
float humid = 0;
float heat_index = 0;
int measured = 0;

WebServer server(80);

HTTPClient http;

/*IPAddress local_IP(192, 168, 1, 15);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);*/

DHT dht(DHTPIN, DHTTYPE);



void handleRoot() {
  digitalWrite(LED_PIN, 1);
  String root_ret = "DHT11:\n";
  root_ret += "  t = ";
  root_ret += temp_c;
  root_ret += " C\n";
  root_ret += " RH = ";
  root_ret += humid;
  root_ret += " %  (Relative humidity)\n";
  root_ret += " HI = ";
  root_ret += heat_index;
  root_ret += " C  (Heat index)\n";
  server.send(200, "text/plain", root_ret);
  digitalWrite(LED_PIN, 0);
}

void handleNotFound() {
  digitalWrite(LED_PIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_PIN, 0);
}

void measure_loop (void *parameters) {
  int i;
  while(1) {
    humid = dht.readHumidity();
    temp_c = dht.readTemperature(false);
    heat_index = dht.computeHeatIndex(temp_c, humid, false);
    if (isnan(humid) || isnan(temp_c)) {
      measured = 0;
      Serial.println(F("DHT sensor error"));
      for(i=0; i<3; i++) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    } else {
      measured = 1;
    }
    Serial.println(humid);
    Serial.println(temp_c);
    Serial.println(heat_index);
    
    vTaskDelay(25000 / portTICK_PERIOD_MS);
  }
}

void wifi_loop (void *parameters) {
  int httpCode;
  int timer = SERVER_TIMER + 1;
  int z;
  String httpResp = "/";
  String message = "http://localhost/esp_mess_save.php?pass=admin&esp_mess=";
  while(1) {
    loop_start:
    timer++;
    z = 0;
    if(WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_PIN, 1);
      Serial.println("Connection to WiFi lost");
      WiFi.disconnect();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      WiFi.reconnect();
      Serial.println("Connecting to WiFi: ");
      while(WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");
        z++;
        if(z>20) {
          Serial.println("");
          goto loop_start;
        }
      }
      Serial.println("");
    }
    digitalWrite(LED_PIN, 0);
    
    if(timer >= SERVER_TIMER && !isnan(humid) && !isnan(temp_c) && measured) {
      timer = 0;
	  
      message.clear();
      message += "http://localhost/esp_mess_save.php?pass=admin&esp_mess=";
      message += "t:";
      message += temp_c;
      message += "C,rh:";
      message += humid;
      message += ",hi:";
      message += heat_index;
      message += "C";
	  
      http.begin(message);
      httpCode = http.GET();
      Serial.println("HTTP code");
      Serial.println(httpCode);
      Serial.println(http.getString());
      http.end();
	  
	  
      message.clear();
      message += "http://api.ipify.org/";
      http.begin(message);
      httpCode = http.GET();
      httpResp.clear();
      httpResp += http.getString();
      Serial.println("HTTP code");
      Serial.println(httpCode);
      Serial.println(httpResp);
      http.end();
  	  
      message.clear();
      message += "http://localhost/esp_t_save.php?pass=admin&esp_mess=";
      message += httpResp;
      http.begin(message);
      httpCode = http.GET();
      Serial.println("HTTP code");
      Serial.println(httpCode);
      Serial.println(http.getString());
      http.end();
    }
	
    server.handleClient();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup(void) {
  int z = 0;
  delay(5);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 1);
  Serial.begin(115200);

  /*if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }*/
  WiFi.mode(WIFI_STA);
  wifi_first:
  z = 0;
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi: ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  	z++;
  	if(z>20) {
        Serial.println("");
        WiFi.disconnect();
        delay(5000);
        goto wifi_first;
  	}
  }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  dht.begin();
  
  delay(2);
  xTaskCreate(measure_loop,   //Task function
              "measure_loop", //Name of task in task scheduler
              1024*5,         //Stack size
              NULL,           //Parameter send to function
              10,             //Priority
              &xHandle);      //task handler

  xTaskCreate(wifi_loop,   //Task function
              "wifi_loop", //Name of task in task scheduler
              1024*5,         //Stack size
              NULL,           //Parameter send to function
              15,             //Priority
              &xHandle);      //task handler

  digitalWrite(LED_PIN, 0);
}

void loop(void) {
  delay(25000);
}

