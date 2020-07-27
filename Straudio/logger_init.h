#include "build_config.h"
#include "shared_config.h"
#include "sago/platform_folders.h"

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#include "plog/Init.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "plog/Appenders/RollingFileAppender.h"
#include "plog/Formatters/TxtFormatter.h"

static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(LOG_LOCATION.c_str(), LOG_SIZE, MAX_LOGS);
static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;

class LoggerInit {
	
public:
	static int init() {
		LoggerInit::createDataFolderIfNeeded();
		
		auto logLocation = LOG_LOCATION;
		plog::init(LOG_THRESHOLD, &fileAppender).addAppender(&consoleAppender);
		
		return 1337; // leet
	}
	
	static void createDataFolderIfNeeded() {
		auto dataLocation = DATA_LOCATION;
		
		struct stat info;
		
		if (stat(DATA_LOCATION.c_str(), &info) == 0) {
			return; // folder already exists
		}
		
		mode_t nMode = 0766;
		int nError = 0;
		#if defined(_WIN32)
			nError = _mkdir(dataLocation.c_str());
		#else
			nError = mkdir(dataLocation.c_str(), nMode);
		#endif
		
		if (nError == -1) {
			// Folder has already been created, ignore
		} else if (nError != 0) {
			throw;
		}
	}
	
	static int initializer;
	
};

int LoggerInit::initializer = LoggerInit::init();
