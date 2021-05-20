#pragma once

#include "WiFiNINA.h"

class SFU {
public:
	static int begin();
	static int download(const char* url);
	static int apply();
};