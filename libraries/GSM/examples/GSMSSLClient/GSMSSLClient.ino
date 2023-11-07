/*
  GSMSSLlient

  This sketch connects to a website (https://ifconfig.me)
  using the Portenta CAT.M1/NB IoT GNSS Shield and TLS.
 
 */

#include <GSM.h>

#include "arduino_secrets.h" 
char pin[]      = SECRET_PIN;
char apn[]      = SECRET_APN;
char username[] = SECRET_USERNAME;
char pass[]     = SECRET_PASSWORD;

const char  server[] = "ifconfig.me";
const char* ip_address;
int port = 443;
GSMSSLClient client;

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  Serial.println("Starting Carrier Network registration");
  if(!GSM.begin(pin, apn, username, pass, CATM1, BAND_3 | BAND_20 | BAND_19)){
    Serial.println("The board was not able to register to the network...");
    // do nothing forevermore:
    while(1);
  }
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /ip HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("unable to connect to server");
  }
  
}

void loop() {

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }

}
