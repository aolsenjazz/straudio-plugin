#include "nlohmann/json.hpp"
#include "rtc/rtc.hpp"
#include "logger.h"

class DebugSocket {
	
private:
	std::shared_ptr<rtc::WebSocket> ws;

public:
	DebugSocket(std::string url) {
		ws = std::make_shared<rtc::WebSocket>();
		
		ws->onOpen([]() {
			Loggy::info("Socket open!!");
		});
		
		ws->onMessage([](const std::variant<rtc::binary, std::string> &message) {
			Loggy::info("Message!");
		});
		
		ws->onClosed([]() {
			Loggy::info("Closed!");
		});
		
		ws->onError([](const std::string &error) {
			std::cout << "websocket failed: " << error << std::endl;
		});
		
		ws->open(url);
	}
	
	void onLifecycleEvent(nlohmann::json data) {
		if (this->ws->isOpen()) this->ws->send(data.dump());
	}
	
	~DebugSocket() {
		
	}
	

};
