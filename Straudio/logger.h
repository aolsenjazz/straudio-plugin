#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

class Loggy {

public:
	static void info(std::string msg) {
#ifdef _WIN32
		OutputDebugString(msg.c_str());
#else
		std::cout << msg << std::endl;
#endif
	}
};
