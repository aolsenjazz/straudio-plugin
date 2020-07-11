#pragma once

#include "rtc/rtc.hpp"
#include "logger.h"
#include "nlohmann/json.hpp"

class SignalAService {
	
private:
//	std::shared_ptr<rtc::WebSocket> ws;
	
	void onOpen() {
		Loggy::info("Socket open!!");
	}
	
	void onMessage(const std::variant<rtc::binary, std::string> &message) {
//		if (auto* str_msg = std::get_if<std::string>(&message)) {
//			auto j = nlohmann::json::parse(*str_msg);
//
//			auto method = j["method"].get<std::string>();
//
//			if (method == "createSuccess") {
//				Loggy::info("Room created!");
//			} else if (method == "clientDescription") {
//				std::string description = j["description"].get<std::string>();
//				std::string type = j["type"].get<std::string>();
//
//
//				descriptionCb(description, type);
//			} else if (method == "clientCandidate") {
//				std::string candidate = j["candidate"].get<std::string>();
//				std::string mid = j["mid"].get<std::string>();
//
//				candidateCb(candidate, mid);
//			}
//		} else {
//			// error, handle appropriately
//		}
	}
	
	void onClose() {
		Loggy::info("Closed!");
	}
	
	void onError(const std::string &error) {
		Loggy::info("Error in signalling websocket");
	}
	
	std::function<void(std::string, std::string)> descriptionCb;
	std::function<void(std::string, std::string)> candidateCb;
	
public:
	SignalAService(std::string url, std::function<void(std::string, std::string)> descCb, std::function<void(std::string, std::string)> candCb) {
//		descriptionCb = descCb;
//		candidateCb = candCb;
//
//		ws = std::make_shared<rtc::WebSocket>();
//
//		ws->onOpen(std::bind(&SignalAService::onOpen, this));
//		ws->onMessage(std::bind(&SignalAService::onMessage, this, std::placeholders::_1));
//		ws->onClosed(std::bind(&SignalAService::onClose, this));
//		ws->onError(std::bind(&SignalAService::onError, this, std::placeholders::_1));
//
//		ws->open(url);
	}
	
	void createRoom() {
//		nlohmann::json j;
//
//		j["method"] = "createRoom";
//		j["displayName"] = "hostboiiii";
//
//		ws->send(j.dump());
	}
	
	void setDescription(std::string type, std::string description) {
//		nlohmann::json j = {
//			{"type", type},
//			{"description", description},
//			{"method", "setDescription"}
//		};
//
//		Loggy::info("Setting local description: " + j.dump());
//		ws->send(j.dump());
	}
	
	void addCandidate(std::string type, std::string candidate, std::string mid) {
//		nlohmann::json j = {
//			{"type", type},
//			{"candidate", candidate},
//			{"mid", mid},
//			{"method", "addCandidate"}
//		};
//
//		Loggy::info("Sending local candidate: " + j.dump());
//		ws->send(j.dump());
	}
	
};
