#pragma once

#include "new_signal_service.h"
#include "domain.h"

class WebServicesManager {
	
private:
	void onRemoteDescription() {
		Loggy::info("Remote description received");
	}
	
	void onRemoteICECandidate() {
		Loggy::info("Remote ice candidate received");
	}
	
	
public:
	
	SignalService* ss;
	
	WebServicesManager(std::function<void(std::string)> connectionCb,
					   std::function<void(std::string, Room)> roomCb,
					   std::function<void(std::string, std::string)> errorCb) {
		
		ss = new SignalService(connectionCb, roomCb,
							   std::bind(&WebServicesManager::onRemoteDescription, this),
							   std::bind(&WebServicesManager::onRemoteICECandidate, this), errorCb);
		
		
		
	}
	
};
