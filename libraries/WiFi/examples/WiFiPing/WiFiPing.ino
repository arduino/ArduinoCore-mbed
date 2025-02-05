/*
  Web ICMP Ping

  This sketch pings a device based on the IP address or the hostname
  using the WiFi module.

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the WiFi.begin() call accordingly.

  created 14 February 2024
  by paulvha
  modified 8 Jenuary 2025
  by fabik111

 */

#include <WiFi.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

/* -------------------------------------------------------------------------- */
void setup() {
/* -------------------------------------------------------------------------- */
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed.");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 3 seconds for connection:
    delay(3000);
  }

  printWifiStatus();
}

/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */

  // Ping IP
  const IPAddress remote_ip(140,82,121,4);
  Serial.print("Trying to ping github.com on IP: ");
  Serial.println(remote_ip);

  // using default ping count of 1
  int res = WiFi.ping(remote_ip);

  if (res > 0) {
    Serial.print("Ping response time: ");
    Serial.print(res);
    Serial.println(" ms");
  }
  else {
    Serial.println("Timeout on IP!");
  }

  // Ping Host
  const char* remote_host = "www.google.com";
  Serial.print("Trying to ping host: ");
  Serial.println(remote_host);

  int res1 = WiFi.ping(remote_host);

  if (res1 > 0) {
    Serial.print("Ping response time: ");
    Serial.print(res1);
    Serial.println(" ms");
  }
  else {
    Serial.println("Timeout on host!");
  }

  Serial.println();
  delay(5000);
}

/* -------------------------------------------------------------------------- */
void printWifiStatus() {
/* -------------------------------------------------------------------------- */
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
