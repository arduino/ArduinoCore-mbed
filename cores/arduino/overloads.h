void pinMode(PinName pinNumber, PinMode pinMode);
void digitalWrite(PinName pinNumber, PinStatus status);
PinStatus digitalRead(PinName pinNumber);
int analogRead(PinName pinNumber);
void analogWrite(PinName pinNumber, int value);

unsigned long pulseIn(PinName pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(PinName pin, uint8_t state, unsigned long timeout);

void shiftOut(PinName dataPin, PinName clockPin, BitOrder bitOrder, uint8_t val);
uint8_t shiftIn(PinName dataPin, PinName clockPin, BitOrder bitOrder);

void attachInterrupt(PinName interruptNumber, voidFuncPtr callback, PinStatus mode);
void attachInterruptParam(PinName interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param);
void detachInterrupt(PinName interruptNumber);