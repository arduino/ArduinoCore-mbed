#ifdef ARDUINO_PORTENTA_H7_M7
#ifndef __PORTENTA_SYSTEM__
#define __PORTENTA_SYSTEM__

#include "Arduino.h"

class Portenta_System {
public:
    // MUST be called before initializing any peripheral (eg. before Serial.begin())
    static void overdrive();
    static void downclock();
    static void getCpuStats(int samplingFrequency = SAMPLE_TIME_MS, voidFuncPtr printer = default_printer) {
        events::EventQueue *stats_queue = mbed::mbed_event_queue();
        int id = stats_queue->call_every(SAMPLE_TIME_MS, default_printer);
    }
private:
    static const int SAMPLE_TIME_MS = 1000;
    static void default_printer() {
        static uint64_t prev_idle_time = 0;
        mbed_stats_cpu_t stats;
        mbed_stats_cpu_get(&stats);

        // Calculate the percentage of CPU usage
        uint64_t diff_usec = (stats.idle_time - prev_idle_time);
        uint8_t idle = (diff_usec * 100) / (SAMPLE_TIME_MS*1000);
        uint8_t usage = 100 - ((diff_usec * 100) / (SAMPLE_TIME_MS*1000));
        prev_idle_time = stats.idle_time;

        printf("Time(us): Up: %lld", stats.uptime);
        printf("   Idle: %lld", stats.idle_time);
        printf("   Sleep: %lld", stats.sleep_time);
        printf("   DeepSleep: %lld\n", stats.deep_sleep_time);
        printf("Idle: %d%% Usage: %d%%\n\n", idle, usage);
    }
};

#endif
#endif