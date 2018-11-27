#include "WiFi.h"
#include "WiFiSSLClient.h"
//#include "WiFiESP8266.h"
#include "WiFiODINW2.h"
#include "TLSSocket.h"
#include <MQTTClient.h>

mbed::DigitalOut myled(LED1);

const char CA_CERTIFICATES[] = "-----BEGIN CERTIFICATE-----\n"
                               "MIIEADCCAuigAwIBAgIBADANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEh\n"
                               "MB8GA1UEChMYVGhlIEdvIERhZGR5IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBE\n"
                               "YWRkeSBDbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTA0MDYyOTE3\n"
                               "MDYyMFoXDTM0MDYyOTE3MDYyMFowYzELMAkGA1UEBhMCVVMxITAfBgNVBAoTGFRo\n"
                               "ZSBHbyBEYWRkeSBHcm91cCwgSW5jLjExMC8GA1UECxMoR28gRGFkZHkgQ2xhc3Mg\n"
                               "MiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASAwDQYJKoZIhvcNAQEBBQADggEN\n"
                               "ADCCAQgCggEBAN6d1+pXGEmhW+vXX0iG6r7d/+TvZxz0ZWizV3GgXne77ZtJ6XCA\n"
                               "PVYYYwhv2vLM0D9/AlQiVBDYsoHUwHU9S3/Hd8M+eKsaA7Ugay9qK7HFiH7Eux6w\n"
                               "wdhFJ2+qN1j3hybX2C32qRe3H3I2TqYXP2WYktsqbl2i/ojgC95/5Y0V4evLOtXi\n"
                               "EqITLdiOr18SPaAIBQi2XKVlOARFmR6jYGB0xUGlcmIbYsUfb18aQr4CUWWoriMY\n"
                               "avx4A6lNf4DD+qta/KFApMoZFv6yyO9ecw3ud72a9nmYvLEHZ6IVDd2gWMZEewo+\n"
                               "YihfukEHU1jPEX44dMX4/7VpkI+EdOqXG68CAQOjgcAwgb0wHQYDVR0OBBYEFNLE\n"
                               "sNKR1EwRcbNhyz2h/t2oatTjMIGNBgNVHSMEgYUwgYKAFNLEsNKR1EwRcbNhyz2h\n"
                               "/t2oatTjoWekZTBjMQswCQYDVQQGEwJVUzEhMB8GA1UEChMYVGhlIEdvIERhZGR5\n"
                               "IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBEYWRkeSBDbGFzcyAyIENlcnRpZmlj\n"
                               "YXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQAD\n"
                               "ggEBADJL87LKPpH8EsahB4yOd6AzBhRckB4Y9wimPQoZ+YeAEW5p5JYXMP80kWNy\n"
                               "OO7MHAGjHZQopDH2esRU1/blMVgDoszOYtuURXO1v0XJJLXVggKtI3lpjbi2Tc7P\n"
                               "TMozI+gciKqdi0FuFskg5YmezTvacPd+mSYgFFQlq25zheabIZ0KbIIOqPjCDPoQ\n"
                               "HmyW74cNxA9hi63ugyuV+I6ShHI56yDqg+2DzZduCLzrTia2cyvk0/ZM/iZx4mER\n"
                               "dEr/VxqHD3VILs9RaRegAhJhldXRQLIQTO7ErBBDpqWeCtWVYpoNz4iCxTIM5Cuf\n"
                               "ReYNnyicsbkqWletNw+vHX/bvZ8=\n"
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
