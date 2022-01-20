#include "Arduino.h"
#include "timer.h"
#include "mbed.h"

using namespace arduino;

struct _mbed_timer {
	mbed::Timer* obj;
};

ArduinoTimer::ArduinoTimer(void* _timer) {

	if (timer == NULL) {
		timer = new mbed_timer;
		timer->obj = NULL;
	}
	if (timer->obj == NULL) {
		timer->obj = (mbed::Timer*)_timer;
	}

}

void ArduinoTimer::start() {
  timer->obj->start();
}

void ArduinoTimer::stop() {
  timer->obj->stop();
}