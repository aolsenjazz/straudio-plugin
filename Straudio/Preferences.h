#include <iostream>
#include <map>
#include "malvineous/cfgpath.h"
#include <fstream>

std::string DELIMITER = "=";

class Preferences {

public:
	static std::string getAuth() {
		return get("auth");
	};
	static void setAuth(std::string auth) {
		set("auth", auth);
	};
	
private:
	

	static std::map<std::string, std::string> getAllPrefs() {
		std::map<std::string, std::string> allPrefs;
		
		char cfgdir[256];
		get_user_config_file(cfgdir, sizeof(cfgdir), "straudio");
		
		std::string line;
		std::ifstream prefsFile(cfgdir);
		while (getline(prefsFile, line)) {
			std::string prefKey = line.substr(0, line.find(DELIMITER));
			std::string prefVal = line.substr(line.find(DELIMITER) + 1, line.length());
			
			allPrefs.insert({prefKey, prefVal});
		}
		
		prefsFile.close();
		
		return allPrefs;
	}

	static std::string get(std::string prefKey) {
		return getAllPrefs()[prefKey];
	}

	static void set(std::string key, std::string value) {
		std::map<std::string, std::string> allPrefs = getAllPrefs();
		allPrefs[key] = value;
		
		char cfgdir[256];
		get_user_config_file(cfgdir, sizeof(cfgdir), "straudio");

		std::ofstream prefsFile;
		prefsFile.open(cfgdir);

		std::map<std::string, std::string>::iterator it = allPrefs.begin();
		while (it != allPrefs.end()) {
			std::string key = it->first;
			std::string value = it->second;

			prefsFile << key << "=" << value << "\n";
			
			it++;
		}

		prefsFile.close();
	}
};
