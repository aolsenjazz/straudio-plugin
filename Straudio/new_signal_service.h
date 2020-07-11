#include "shared_config.h"
#include "build_config.h"
#include "logger.h"
#include "rtc/rtc.hpp"
#include "domain.h"

#include <sstream>
#include <string>

class SignalService {

private:
	
	std::shared_ptr<rtc::WebSocket> _ws;
	
	std::function<void(std::string, std::string)> _errorCb;
	std::function<void(std::string)> _connectionCb;
	std::function<void(std::string, Room)> _roomCb;
	std::function<void()> _remoteDescriptionCb;
	std::function<void()> _iceCandidateCb;
	
	void onOpen() { _connectionCb("open"); }
	void onClose() { _connectionCb("closed"); }
	void onError(const std::string &error) { _errorCb("warn", error); }
	
	void onMessage(const std::variant<rtc::binary, std::string> &message) {
		if (auto* str_msg = std::get_if<std::string>(&message)) {
			auto j = nlohmann::json::parse(*str_msg);
			
			auto method = j["method"].get<std::string>();

			if (method == "createRoomResponse") onCreateRoomResponse(j);
			else if (method == "clientJoin") onClientJoin(j);
			else if (method == "clientLeave") onClientLeave(j);
			else if (method == "sendDescriptionResponse") onSendDescriptionResponse(j);
			else if (method == "sendCandidateResponse") onSendCandidateResponse(j);
			else if (method == "description") onRemoteDescription(j);
			else if (method == "candidate") onRemoteCandidate(j);
			else if (method == "rejoinResponse") onRejoinResponse(j);
			else Loggy::info("Unknown method: " + method);
			
		} else {
			Loggy::info("Received message is not a string. This shouldn't ever happen.");
		}
	}
	
	// For now, these are noop
	void onClientJoin(nlohmann::json j) { Loggy::info("Client joined room"); }
	void onClientLeave(nlohmann::json j) { Loggy::info("Client left room"); }
	void onRejoinResponse(nlohmann::json j) { Loggy::info("Rejoined room"); }
	
	void onCreateRoomResponse(nlohmann::json j) {
		bool success = j["success"].get<bool>();
		Room r(j["room"]);
		
		if (!success) _errorCb("critical", "Error creating room. This should never happen.");
		else _roomCb("open", r);
	}
	
	void onSendDescriptionResponse(nlohmann::json j) {/** This SHOULDN'T fail, but handle just in case */}
	void onSendCandidateResponse(nlohmann::json j) {/** This SHOULDN'T fail, but handle just in case */}
	void onRemoteDescription(nlohmann::json j) {
		std::string description = j["description"].get<std::string>();
		std::string type = j["type"].get<std::string>();
		
		_remoteDescriptionCb();
	}
	void onRemoteCandidate(nlohmann::json j) {
		std::string candidate = j["candidate"].get<std::string>();
		std::string mid = j["mid"].get<std::string>();
		
		_iceCandidateCb();
	}

public:
	
	std::string url;
	
	SignalService(std::function<void(std::string)> connectionCb,
				  std::function<void(std::string, Room)> roomCb,
				  std::function<void()> descriptionCb,
				  std::function<void()> candidateCb,
				  std::function<void(std::string, std::string)> errorCb) {
		_iceCandidateCb = candidateCb;
		_roomCb = roomCb;
		_connectionCb = connectionCb;
		_errorCb = errorCb;
		_remoteDescriptionCb = descriptionCb;
		
		std::ostringstream os;
		os << (USE_TLS ? "wss://" : "ws://") << SIGNAL_HOST << ":" << SIGNAL_PORT << "/";
		
		url = os.str();
		
		_ws = std::make_shared<rtc::WebSocket>();
		connect();
	}
	
	void connect() {
		_ws->onOpen(std::bind(&SignalService::onOpen, this));
		_ws->onMessage(std::bind(&SignalService::onMessage, this, std::placeholders::_1));
		_ws->onClosed(std::bind(&SignalService::onClose, this));
		_ws->onError(std::bind(&SignalService::onError, this, std::placeholders::_1));
		
		_ws->open(url);
	}
	
	void createRoom() {
		nlohmann::json j = {
			{"displayName", "Host"},
			{"method", "createRoom"}
		};
		
		Loggy::info("Sending createRoom request");
		_ws->send(j.dump());
	}
};
