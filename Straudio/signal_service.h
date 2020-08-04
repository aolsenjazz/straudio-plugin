#include "shared_config.h"
#include "build_config.h"
#include "rtc/rtc.hpp"
#include "domain.h"
#include "interval_executor.h"
#include "timeout_executor.h"

#include <chrono>

class SignalService {

private:
	
	std::shared_ptr<rtc::WebSocket> _ws;
	std::unique_ptr<IntervalExecutor> _reconnectExecutor;
	std::unique_ptr<TimeoutExecutor> _heartbeatExecutor = nullptr;
	
	std::shared_ptr<Room> room;
	std::shared_ptr<std::string> signalState;
	
	std::function<void(std::string, std::string)> _errorCb;
	std::function<void()> _signalStateChangeCb;
	std::function<void()> _roomStateChangeCb;
	std::function<void(std::string, std::string, std::string)> _remoteDescriptionCb;
	std::function<void(std::string, std::string, std::string)> _remoteCandidateCb;
	std::function<void(std::string)> _closeRtcCb;
	
	bool keepAlive = true;
	bool destructing = false;
	
	void _onOpen() {
		PLOG_DEBUG << "websocket open";
		_ws->setPingCallbacks(std::bind(&SignalService::onSendPing, this),
							  std::bind(&SignalService::onReceivePing, this));
		
		
		*signalState = "open";
		_signalStateChangeCb();
	}
	
	void onSendPing() {
		// this is useless rn
	}
	
	void onReceivePing() {
		PLOG_DEBUG << "pong received";
		
		if (_heartbeatExecutor != nullptr) {
			_heartbeatExecutor->interrupt();
		}
		
		_heartbeatExecutor.reset(new TimeoutExecutor(12000, std::bind(&SignalService::close, this)));
		_heartbeatExecutor->start();
	}
	
	void _onClose() {
		PLOG_DEBUG << "websocket closed";
		
		if (destructing) return; // if object is destructing, don't reconnect WS
		
		room->state = "closed";
		*signalState = "closed";
		_signalStateChangeCb();
		
		_reconnectExecutor->start();
	}
	
	void _onError(const std::string &error) {
		_errorCb("warn", error);
	}
	
	void _onMessage(const std::variant<rtc::binary, std::string> &message) {
		if (_heartbeatExecutor != nullptr) {
			_heartbeatExecutor->interrupt();
		}
		
		if (auto* str_msg = std::get_if<std::string>(&message)) {
			PLOG_VERBOSE << *str_msg;
			auto j = nlohmann::json::parse(*str_msg);
			
			auto method = j["method"].get<std::string>();

			if (method == "audioDetailsResponse")     _onAudioDetailsResponse(j);
			else if (method == "clientJoin")          _onClientJoin(j);
			else if (method == "clientLeave")         _onClientLeave(j);
			else if (method == "descriptionResponse") _onSendDescriptionResponse(j);
			else if (method == "candidateResponse")   _onSendCandidateResponse(j);
			else if (method == "description")         _onRemoteDescription(j);
			else if (method == "candidate")           _onRemoteCandidate(j);
			else if (method == "rejoinRoomResponse")  _onRejoinResponse(j);
			else if (method == "createRoomResponse")  _onCreateRoomResponse(j);
			else PLOG_INFO << "Unknown method: " << method;
			
		} else {
			PLOG_ERROR << "Recevied message is not a string. This shouldn't happen.";
		}
	}
	
	// For now, these are noop
	void _onClientJoin(nlohmann::json j) {
		std::string clientId = j["client"]["id"].get<std::string>();
		PLOG_INFO << "Client[" << clientId << "] joined room";
	}
	void _onClientLeave(nlohmann::json j) {
		std::string clientId = j["client"]["id"].get<std::string>();
		
		PLOG_INFO << "Client[" << clientId << "] left room";
		
		_closeRtcCb(clientId);
	}
	
	
	void _onCreateRoomResponse(nlohmann::json j) {
		bool success = j["success"].get<bool>();

		if (!success) {
			std::string errorString = j["error"].get<std::string>();
			
			PLOG_ERROR << "Error creating room: " << errorString;
		} else {
			*room = Room(j["room"]);
			_roomStateChangeCb();
		}
	}
	
	void _onAudioDetailsResponse(nlohmann::json j) {/** This SHOULDN'T fail, but handle just in case */}
	void _onSendDescriptionResponse(nlohmann::json j) {/** This SHOULDN'T fail, but handle just in case */}
	void _onSendCandidateResponse(nlohmann::json j) {/** This SHOULDN'T fail, but handle just in case */}
	void _onRemoteDescription(nlohmann::json j) {
		std::string sourceId = j["sourceId"].get<std::string>();
		std::string description = j["description"]["description"].get<std::string>();
		std::string type = j["description"]["type"].get<std::string>();
		
		_remoteDescriptionCb(sourceId, type, description);
	}
	void _onRemoteCandidate(nlohmann::json j) {
		std::string sourceId = j["sourceId"].get<std::string>();
		std::string candidate = j["candidate"]["candidate"].get<std::string>();
		std::string mid = j["candidate"]["mid"].get<std::string>();
		
		_remoteCandidateCb(sourceId, mid, candidate);
	}
	
	void _onRejoinResponse(nlohmann::json j) {
		
	}

public:
	
	std::string url;
	
	SignalService(std::shared_ptr<Room> r,
				  std::shared_ptr<std::string> sigState,
				  std::function<void()> signalStateCb,
				  std::function<void()> roomStateCb,
				  std::function<void(std::string, std::string)> errorCb,
				  std::function<void(std::string, std::string, std::string)> descriptionCb,
				  std::function<void(std::string, std::string, std::string)> candidateCb,
				  std::function<void(std::string)> closeRtcCb) {
		room = r;
		signalState = sigState;
		
		_remoteCandidateCb = candidateCb;
		_roomStateChangeCb = roomStateCb;
		_signalStateChangeCb = signalStateCb;
		_errorCb = errorCb;
		_remoteDescriptionCb = descriptionCb;
		_closeRtcCb = closeRtcCb;
		
		std::ostringstream os;
		os << (USE_TLS ? "wss://" : "ws://") << SIGNAL_HOST << ":" << SIGNAL_PORT << "/";
		url = os.str();
		
		rtc::WebSocket::Configuration config;
		config.disableTlsVerification = true;
		_ws = std::make_shared<rtc::WebSocket>(config);
		
		auto workFunc = std::bind(&SignalService::connect, this);
		auto runWhile = std::bind(&rtc::WebSocket::isClosed, _ws);
		_reconnectExecutor = std::make_unique<IntervalExecutor>(1000 , workFunc, runWhile);
	}
	
	~SignalService() {
		destructing = true;
		
		if (_heartbeatExecutor != nullptr) _heartbeatExecutor->interrupt();
		if (_reconnectExecutor != nullptr) _reconnectExecutor->stop();
		
		_ws->close();
	}
	
	void tryToConnect() {
		_reconnectExecutor->start();
	}
	
	void connect() {
		try {
			_ws->open(url);
			_ws->onOpen(std::bind(&SignalService::_onOpen, this));
			_ws->onMessage(std::bind(&SignalService::_onMessage, this, std::placeholders::_1));
			_ws->onClosed(std::bind(&SignalService::_onClose, this));
			_ws->onError(std::bind(&SignalService::_onError, this, std::placeholders::_1));
		} catch (const std::exception& e) {
			PLOG_ERROR << "Failed to open websocket: " << e.what();
		}
	}
	
	void close() {
		_ws->close();
	}
	
	bool isOpen() {
		return _ws->isOpen();
	}
	
	void createRoom(int sampleRate, int nChannels, int bitDepth) {
		nlohmann::json j = {
			{"displayName", "Host"},
			{"method", "createRoom"},
			{"sampleRate", sampleRate},
			{"nChannels", nChannels},
			{"bitDepth", bitDepth}
		};
		
		safeSend(j.dump());
	}
	
	void sendCandidate(std::string targetId, std::string mid, std::string candidate) {
		nlohmann::json j = {
			{"targetId", targetId},
			{"candidate", {
				{"mid", mid},
				{"candidate", candidate},
			}},
			{"method", "candidate"}
		};
		
		safeSend(j.dump());
	}
	
	void sendDescription(std::string targetId, std::string type, std::string description) {
		nlohmann::json j = {
			{"targetId", targetId},
			{"description", {
				{"type", type},
				{"description", description},
			}},
			{"method", "description"}
		};
		
		safeSend(j.dump());
	}
	
	void updateAudioSettings(int sampleRate, int nChannels, int bitDepth) {
		nlohmann::json j = {
			{"sampleRate", sampleRate},
			{"nChannels", nChannels},
			{"bitDepth", bitDepth},
			{"method", "audioDetails"}
		};
		
		safeSend(j.dump());
	}
	
	void notifyBufferReset() {
		nlohmann::json j = {
			{"method", "bufferReset"}
		};
		
		safeSend(j.dump());
	}
	
	void safeSend(std::string data) {
		if (_ws->readyState() == rtc::WebSocket::State::Open) {
			_ws->send(data);
		} else {
			PLOG_WARNING << "Tried to call ws->send() in incorrect State";
		}
	}
};
