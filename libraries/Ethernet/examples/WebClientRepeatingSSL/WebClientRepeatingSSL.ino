/*
  Repeating TLS/SSL Ethernet Web client.

  Remeber to update the CA certificates using WiFiFirmwareUpdater sketch
  before using this sketch.

 */

#include <Ethernet.h>
#include <EthernetSSLClient.h>
#include <PortentaEthernet.h>

// initialize the library instance:
EthernetSSLClient client;

char server[] = "www.arduino.cc";
int port = 443;
// IPAddress server(64,131,82,241);

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10 * 1000; // delay between updates, in milliseconds

void setup()
{

    // start serial port:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin() == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("Ethernet cable is not connected.");
        }
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(ip, myDns);
        Serial.print("My IP address: ");
        Serial.println(Ethernet.localIP());
    } else {
        Serial.print("  DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
}

void loop()
{
    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only:
    if (client.available()) {
        char c = client.read();
        Serial.write(c);
    }

    // if ten seconds have passed since your last connection,
    // then connect again and send data:
    if (millis() - lastConnectionTime > postingInterval) {
        httpRequest();
    }
}

// this method makes a HTTP connection to the server:
void httpRequest()
{
    // close any connection before send a new request.
    // This will free the socket on the WiFi shield
    client.stop();

    // if there's a successful connection:
    if (client.connect(server, port)) {
        Serial.println("connecting...");
        // send the HTTP GET request:
        client.println("GET /latest.txt HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("User-Agent: arduino-ethernet");
        client.println("Accept: *");
        client.println("Connection: close");
        client.println();

        // note the time that the connection was made:
        lastConnectionTime = millis();
    } else {
        // if you couldn't make a connection:
        Serial.println("connection failed");
    }
}
