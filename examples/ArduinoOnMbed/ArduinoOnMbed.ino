#include "WiFi.h"
#include "WiFiSSLClient.h"
//#include "WiFiESP8266.h"
#include "WiFiODINW2.h"
#include "TLSSocket.h"
#include <MQTTClient.h>

mbed::DigitalOut myled(LED1);

const char CA_CERTIFICATES[] = "-----BEGIN CERTIFICATE-----\n"
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
"-----END CERTIFICATE-----\n";


#define TEST_SSL

#ifndef TEST_SSL
WiFiClient net;
#else
WiFiSSLClient net;
#endif
MQTTClient client;

unsigned long lastMillis = 0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED2, OUTPUT);
  String a = "test";
  String b = a + 12;
  String c = a + "18";
  Serial.begin(115200);
  delay(100);
  Serial.println(a);
  Serial.println(b);
  Serial.println(c);
  a += "68";
  Serial.println(a);
  //delay(2000);
  Serial.println(USBRX);
  Serial.println(USBTX);
  printf("now\n\r");
  int ret = WiFi.begin("BCMI", "ArduinoccRulez");
  Serial.println("Wifibegin " + String(ret));

#ifndef TEST_SSL
  ret = net.connect("example.com", 80);
  Serial.println("after connect " + String(ret));
  net.print("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
  Serial.println("after get");
#else
  ret = net.connect("arduino.cc", 443);
  Serial.println("after connect " + String(ret));
  net.print("GET /asciilogo.txt HTTP/1.1\r\nHost: arduino.cc\r\n\r\n");
  Serial.println("after get");
#endif
}

void loop() {

  while (net.available()) {
    Serial.write(net.read());
  }

  // put your main code here, to run repeatedly:
  while (Serial.available()) {
    Serial.write(Serial.read());
  }
  wait(0.1);
  digitalWrite(LED2, HIGH);
  myled = 1;
  delay(1000);
  digitalWrite(LED2, LOW);
  myled = 0;
  delay(1000);
}
