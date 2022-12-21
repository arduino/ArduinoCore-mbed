#pragma once

#include "WiFiNINA.h"
#include "FATFileSystem.h"

class SFU {
public:
	static int begin();
	static int download(const char* url);
	static int apply();
	static mbed::FATFileSystem& getFileSystem();
};