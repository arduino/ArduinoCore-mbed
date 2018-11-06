DigitalOut myled(LED1);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  myled = 1;
  wait(0.1);
  myled = 0;
  wait(0.1);
}
