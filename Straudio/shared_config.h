#include "sago/platform_folders.h"

#define SIGNAL_PORT 4443
#define HTTP_PORT 3000

#define LOG_SIZE 10000
#define MAX_LOGS 1

#ifdef _WIN32
#define DATA_LOCATION (sago::getDataHome() + "\\Straudio")
#define LOG_LOCATION (DATA_LOCATION + "\\straudio.log")
#elif __APPLE__
#define DATA_LOCATION (sago::getDataHome() + "/Straudio")
#define LOG_LOCATION (DATA_LOCATION + "/straudio.log")
#endif
