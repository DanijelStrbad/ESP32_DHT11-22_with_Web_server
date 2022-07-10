#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "DHT.h"

#define DHTPIN 18
#define DHTTYPE DHT11

#define LED_PIN 2

#define TEST 0

//#define SERVER_TIMER 900
//#define IP_SERVER_TIMER 900
#define SERVER_TIMER 300
#define IP_SERVER_TIMER 300

#define N_MEASUREMENTS_AVERAGE 10
#define N_MEASUREMENTS_ERR_MAX 10

const char* ssid = " . . . ";
const char* password = " . . . ";

const char* cert_web_log= \
"-----BEGIN CERTIFICATE-----\n" \
" . . . \n" \
" . . . \n" \
"ww==\n" \
"-----END CERTIFICATE-----\n";

TaskHandle_t xHandle = NULL;
SemaphoreHandle_t  xMutex = NULL;

float temp_c = 0.0;
float humid = 0.0;
float heat_index = 0.0;

float temp_c_now = 0.0;
float humid_now = 0.0;
float heat_index_now = 0.0;

int measured = 0;
int write_error = 0;


WebServer server(8123);

HTTPClient http;

/*IPAddress local_IP(192, 168, 8, 40);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);*/

DHT dht(DHTPIN, DHTTYPE);



void handleRoot() {
  digitalWrite(LED_PIN, 1);
  String root_ret = "DHT11:\n";
  root_ret += "  t = ";
  root_ret += temp_c_now;
  root_ret += " C\n";
  root_ret += " RH = ";
  root_ret += humid_now;
  root_ret += " %  (Relative humidity)\n";
  root_ret += " HI = ";
  root_ret += heat_index_now;
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
  int rw_index = 0, i, counter = 0, err_counter = 0;

  float temp_c_arr[10];
  float humid_arr[10];
  float heat_index_arr[10];

  while(1) {
    humid_now = dht.readHumidity();
    temp_c_now = dht.readTemperature(false);
    heat_index_now = dht.computeHeatIndex(temp_c_now, humid_now, false);

    if (isnan(humid_now) || isnan(temp_c_now)) {
      err_counter++;

      if(err_counter >= N_MEASUREMENTS_ERR_MAX/2) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        measured = 0;
        xSemaphoreGive(xMutex);
      } else if(err_counter >= N_MEASUREMENTS_ERR_MAX) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        write_error = 1;
        xSemaphoreGive(xMutex);
      }

      Serial.println(F("DHT sensor error"));
      for(i=0; i<3; i++) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

    } else {
      err_counter = 0;

      xSemaphoreTake(xMutex, portMAX_DELAY);
      write_error = 0;

      temp_c_arr[rw_index] = temp_c_now;
      humid_arr[rw_index] = humid_now;
      heat_index_arr[rw_index] = heat_index_now;

      rw_index++;
      rw_index = rw_index % N_MEASUREMENTS_AVERAGE;

      if(counter >= N_MEASUREMENTS_AVERAGE) {
        for(i=0; i<N_MEASUREMENTS_AVERAGE; i++) {
          temp_c += temp_c_arr[i];
          humid += humid_arr[i];
          heat_index += heat_index_arr[i];
        }
        temp_c /= N_MEASUREMENTS_AVERAGE;
        humid /= N_MEASUREMENTS_AVERAGE;
        heat_index /= N_MEASUREMENTS_AVERAGE;

        measured = 1;

      } else {
        counter++;

        if(TEST) {
          temp_c = temp_c_now;
          humid = humid_now;
          heat_index = heat_index_now;
          measured = 1;
        } else {
          measured = 0;
        }
      }

      xSemaphoreGive(xMutex);

      Serial.println(humid_now);
      Serial.println(temp_c_now);
      Serial.println(heat_index_now);
    }

    vTaskDelay(25000 / portTICK_PERIOD_MS);
  }
}


int connect_to_wifi() {
  int z = 0;
  if(WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, 1);
    Serial.println("Connection to WiFi lost");
    WiFi.disconnect();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    // WiFi.reconnect();
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi: ");
    while(WiFi.status() != WL_CONNECTED) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      Serial.print(".");
      z++;
      if(z>20) {
        Serial.println("");
        Serial.println("No WiFi");
        WiFi.disconnect();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        return 1;
        break;
      }
    }
  }
  digitalWrite(LED_PIN, 0);

  if(TEST == 1) {
    Serial.println("");
    Serial.print("Connected to: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  return 0;
}

void wifi_loop (void *parameters) {
  int measured_loc, write_error_loc, temp_c_loc, humid_loc, heat_index_loc;
  int httpCode;
  int timer = SERVER_TIMER + 1;
  int ip_timer = IP_SERVER_TIMER + 1;
  String httpResp = "/";
  String message = "/";
  while(1) {

    xSemaphoreTake(xMutex, portMAX_DELAY);
    measured_loc = measured;
    write_error_loc = write_error;
    temp_c_loc = temp_c;
    humid_loc = humid;
    heat_index_loc = heat_index;
    xSemaphoreGive(xMutex);

    if(timer >= SERVER_TIMER && measured_loc == 1 && write_error_loc == 0) {
      while(connect_to_wifi() == 1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }

      timer = 0;

      message.clear();
	    message += "http://localhost/esp_mess_save.php?pass=adminesp&esp_mess=";
      message += "t:";
      message += temp_c_loc;
      message += "C,rh:";
      message += humid_loc;
      message += ",hi:";
      message += heat_index_loc;
      message += "C";

      http.begin(message);
      httpCode = http.GET();
      Serial.println("HTTPS code");
      Serial.println(httpCode);
      Serial.println(http.getString());
      http.end();

      WiFi.disconnect();
      vTaskDelay(5000 / portTICK_PERIOD_MS);

    } else if(timer >= SERVER_TIMER && write_error_loc == 1) {
      while(connect_to_wifi() == 1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }

      timer = 0;

      message.clear();
      message += "http://localhost/esp_mess_save.php?pass=adminesp&esp_mess=";
      message += "sensor_error";

      http.begin(message);
      httpCode = http.GET();
      Serial.println("HTTPS code");
      Serial.println(httpCode);
      Serial.println(http.getString());
      http.end();

      WiFi.disconnect();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    
	  if(ip_timer >= IP_SERVER_TIMER){
      while(connect_to_wifi() == 1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }

      ip_timer = 0;
      
  	  message.clear();
      message += "http://api.ipify.org/";
      http.begin(message);
      //http.begin(message, cert_web_log);
      httpCode = http.GET();
  	  httpResp.clear();
  	  httpResp += http.getString();
      Serial.println("HTTPS code");
      Serial.println(httpCode);
      Serial.println(httpResp);
  	  http.end();
  	  
  	  message.clear();
  	  message += "http://localhost/esp_t_save.php?pass=adminesp&esp_mess=";
  	  message += httpResp;
  	  http.begin(message);
      httpCode = http.GET();
      Serial.println("HTTPS code");
      Serial.println(httpCode);
      Serial.println(http.getString());
      http.end();

      WiFi.disconnect();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    if(TEST == 1) {
      server.handleClient();
    }

    ip_timer++;
    timer++;
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

  if(TEST == 1) {
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
  
    if(MDNS.begin("esp32")) {
      Serial.println("MDNS responder started");
    }
  
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
  
    server.begin();
    Serial.println("HTTP server started");
  }

  dht.begin();


  delay(2);
  xMutex = xSemaphoreCreateMutex();
  
  xTaskCreate(measure_loop,   //Task function
              "measure_loop", //Name of task in task scheduler
              1024*5,         //Stack size
              NULL,           //Parameter send to function
              10,             //Priority
              &xHandle);      //task handler

  xTaskCreate(wifi_loop,      //Task function
              "wifi_loop",    //Name of task in task scheduler
              1024*5,         //Stack size
              NULL,           //Parameter send to function
              15,             //Priority
              &xHandle);      //task handler

  digitalWrite(LED_PIN, 0);
}

void loop(void) {
  delay(25000);
}
