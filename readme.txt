 ## / ESP32 / FreeRTOS / Wi-Fi / DHT11-22 / Web log / ##

This project is powered by ESP32 which measures temperature and humidity using a DHT11 sensor.
The ESP32 runs tasks using FreeRTOS.
Within the program code for ESP32 it is necessary to set a WiFi password and set the URLs of the web server correctly.

The web application (runs on a web server of your choice, PHP required)
provides temperature and humidity logs (and public IP addresses from ESP32).
The default password is "admin" (change using the web interface).
The main idea of the Web application is to be as simple as possible (for implementation) and to send minimal traffic through the network.

