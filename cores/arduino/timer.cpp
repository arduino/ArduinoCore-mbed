#include "Arduino.h"
#include "timer.h"
#include "mbed.h"

using namespace arduino;

struct _mbed_timer {
	mbed::Timer* obj;
};

ArduinoTimer::ArduinoTimer(void* _timer) {

	if (timer != NULL) {
		delete timer;
	}

	timer = new mbed_timer;
	timer->obj = (mbed::Timer*)_timer;
}

ArduinoTimer::~ArduinoTimer() {
	if (timer != NULL) {
		delete timer;
	}
}

void ArduinoTimer::start() {
  timer->obj->start();
}

void ArduinoTimer::stop() {
  timer->obj->stop();
}

void ArduinoTimer::reset() {
  timer->obj->reset();
}