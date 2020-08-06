#pragma once

#include "ix_signal_service.h"
#include "peer_connection_manager.h"
#include "domain.h"

using namespace std::placeholders;

class WebServicesManager {
	
private:
	
	void onRemoteDescription(std::string sourceId, std::string type, std::string description) {
		PLOG_DEBUG << "Remote description received";
		
		pcm->setRemoteDescription(sourceId, type, description);
	}
	
	void onRemoteICECandidate(std::string sourceId, std::string mid, std::string candidate) {
		PLOG_DEBUG << "Remote ICE candidate received";
		
		pcm->addRemoteCandidate(sourceId, mid, candidate);
	}
	
	void _onLocalDescription(std::string targetId, std::string type, std::string description) {
		PLOG_DEBUG << "Sending local description";
		
		ss->sendDescription(targetId, type, description);
	}
	
	void _onLocalCandidate(std::string targetId, std::string mid, std::string candidate) {
		PLOG_DEBUG << "Sending local candidate";
		
		ss->sendCandidate(targetId, mid, candidate);
	}

public:
	
	std::unique_ptr<SignalService> ss;
	std::unique_ptr<PeerConnectionManager> pcm;
	
	WebServicesManager(std::shared_ptr<Room> roomPtr, std::shared_ptr<std::string> signalStatePtr,
					   std::function<void()> signalStateChangeCb,
					   std::function<void()> roomStateChangeCb,
					   std::function<void(std::string, std::string)> errorCb) {
		
		auto boundRemoteDesc = std::bind(&WebServicesManager::onRemoteDescription, this, _1, _2, _3);
		auto boundRemoteICE = std::bind(&WebServicesManager::onRemoteICECandidate, this, _1, _2, _3);
		auto boundCloseRtc = std::bind(&WebServicesManager::closeRtcConnection, this, _1);
		
		ss = std::make_unique<SignalService>(roomPtr, signalStatePtr,
											 signalStateChangeCb, roomStateChangeCb, errorCb,
											 boundRemoteDesc, boundRemoteICE, boundCloseRtc);
		
		auto boundLocalDesc = std::bind(&WebServicesManager::_onLocalDescription, this, _1, _2, _3);
		auto boundLocalCand = std::bind(&WebServicesManager::_onLocalCandidate, this, _1, _2, _3);
		
		pcm = std::make_unique<PeerConnectionManager>(boundLocalDesc, boundLocalCand);
	}
	
	template <typename T>
	void sendData(T* data, size_t size) {
		pcm->sendAudio(data, size);
	}
	
	void notifyBufferReset() {
		ss->notifyBufferReset();
	}
	
	void updateAudioSettings(int sampleRate, int nChannels, int bitDepth) {
		ss->updateAudioSettings(sampleRate, nChannels, bitDepth);
	}
	
	void closeRtcConnection(std::string clientId) {
		pcm->close(clientId);
	}
	
	void closePeerConnections() {
		pcm->reset();
	}
};
