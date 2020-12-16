#include "Arduino.h"
#include "pinDefinitions.h"
#include "mbed.h"

using namespace std::chrono_literals;
using namespace std::chrono;

class Tone {
    mbed::DigitalOut   *pin;
    mbed::Timer        timer;
    mbed::Timeout      timeout;  // calls a callback once when a timeout expires
    mbed::Ticker       ticker;   // calls a callback repeatedly with a timeout
    uint32_t           frequency;
    uint32_t           duration;

public:
    Tone(PinName _pin, unsigned int frequency, unsigned long duration) : frequency(frequency), duration(duration)  {
    	pin = new mbed::DigitalOut(_pin);
    }

    ~Tone() {
        stop();
        timeout.detach();
        delete pin;
    }

    void start(void) {
        ticker.attach(mbed::callback(this, &Tone::toggle), 500000us / frequency );
        if (duration != 0) {
            start_timeout();
        }
    }

    void toggle() {
    	*pin = !*pin;
    }

    void stop(void) {
        ticker.detach();
        pin = 0;
    }

    void start_timeout(void) {
        timeout.attach(mbed::callback(this, &Tone::stop), duration * 1ms);
    }
};

Tone* active_tone = NULL;

void tone(uint8_t pin, unsigned int frequency, unsigned long duration) {
	if (active_tone) {
		delete active_tone;
	}
	Tone* t = new Tone(digitalPinToPinName(pin), frequency, duration);
	t->start();
	active_tone = t;
};

void noTone(uint8_t pin) {
	if (active_tone) {
		active_tone->stop();
		delete active_tone;
		active_tone = NULL;
	}
};