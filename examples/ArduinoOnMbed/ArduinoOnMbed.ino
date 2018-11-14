#include "WiFi.h"
#include "WiFiESP8266.h"

mbed::DigitalOut myled(LED1);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED2, OUTPUT);
  String a = "test";
  String b = a + 12;
  String c = a + "18";
  Serial.begin(921600);
  delay(100);
  Serial.println(a);
  Serial.println(b);
  Serial.println(c);
  a += "68";
  Serial.println(a);
//delay(2000);
  printf("now\n");
WiFi.begin("12132", "sfsfdf");
}

void loop() {
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
