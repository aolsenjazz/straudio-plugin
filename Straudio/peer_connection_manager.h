#include "rtc/rtc.hpp"

using namespace std::placeholders;

class PeerConnectionManager {
	
private:
	std::function<void(std::string, std::string, std::string)> _localDescriptionCb;
	std::function<void(std::string, std::string, std::string)> _localCandidateCb;
	
	std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> _peerConnections;
	std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> _dataChannels;
	
	rtc::Configuration _config;
	std::mutex lock;
	
	void _onPCStateChange(rtc::PeerConnection::State state) { /** noop */ }
	void _onPCGatheringStateChange(rtc::PeerConnection::GatheringState state) { /** noop */ }
	
	void _initDataChannel(std::shared_ptr<rtc::DataChannel> dc, std::string sourceId) {
		PLOG_INFO << "DataChannel with Client[" << sourceId << "] open";
		
		dc->onClosed([sourceId]() { std::cout << "DataChannel from " << sourceId << " closed" << std::endl; });

		dc->onMessage([sourceId](const std::variant<rtc::binary, std::string> &message) {});

		_dataChannels.emplace(sourceId, dc);
	}
	
public:
	PeerConnectionManager(std::function<void(std::string, std::string, std::string)> localDescriptionCb,
						  std::function<void(std::string, std::string, std::string)> localCandidateCb) {
		_localDescriptionCb = localDescriptionCb;
		_localCandidateCb = localCandidateCb;
		
		_config.iceServers.emplace_back("stun:stun.l.google.com:19302");
	}
	
	~PeerConnectionManager() {
		reset();
	}
	
	std::shared_ptr<rtc::PeerConnection> createPeerConnection(std::string sourceId, std::string type, std::string desc) {
		auto pc = std::make_shared<rtc::PeerConnection>(_config);

		pc->onStateChange(std::bind(&PeerConnectionManager::_onPCStateChange, this, _1));
		pc->onGatheringStateChange(std::bind(&PeerConnectionManager::_onPCGatheringStateChange, this, _1));

		pc->onLocalDescription([&, sourceId](const rtc::Description &description) {
			_localDescriptionCb(sourceId, description.typeString(), std::string(description));
		});

		pc->onLocalCandidate([&, sourceId](const rtc::Candidate &candidate) {
			_localCandidateCb(sourceId, candidate.mid(), std::string(candidate));
		});

		pc->onDataChannel(std::bind(&PeerConnectionManager::_initDataChannel, this, _1, sourceId));

		_peerConnections.emplace(sourceId, pc);

		return pc;
	}
	
	void setRemoteDescription(std::string sourceId, std::string type, std::string description) {
		createPeerConnection(sourceId, type, description);
		
		std::shared_ptr<rtc::PeerConnection> pc;
		if (auto jt = _peerConnections.find(sourceId); jt != _peerConnections.end()) {
			pc = jt->second;
			pc->setRemoteDescription(rtc::Description(description, type));
		} else {
			PLOG_ERROR << "Tried to set remote description for nonexistent Client[" << sourceId << "]";
		}
	}
	
	void addRemoteCandidate(std::string sourceId, std::string mid, std::string candidate) {
		std::shared_ptr<rtc::PeerConnection> pc;
		if (auto jt = _peerConnections.find(sourceId); jt != _peerConnections.end()) {
			pc = jt->second;
			pc->addRemoteCandidate(rtc::Candidate(candidate, mid));
		} else {
			PLOG_ERROR << "Tried to set remote candidate for nonexistent Client[" << sourceId << "]";
		}
	}
	
	template <typename T>
	void sendAudio(T* data, size_t size) {
		lock.lock();
		for (const auto & [targetId, dc] : _dataChannels) {
			if (dc != nullptr && dc->isOpen()) {
				dc->send((rtc::byte*) data, size);
			} else {
				PLOG_WARNING << "Tried to send audio to nonexistent Client[" << targetId << "]";
			}
		}
		lock.unlock();
	}
	
	void close(std::string clientId) {
		lock.lock();
		if (auto it = _dataChannels.find(clientId); it != _dataChannels.end()) {
			std::shared_ptr<rtc::DataChannel> dc = it->second;
			dc->close();
			_dataChannels.erase(it);
			
			PLOG_INFO << "Client[" << clientId << "] DataChannel closed";
		}
		
		if (auto it = _peerConnections.find(clientId); it != _peerConnections.end()) {
			std::shared_ptr<rtc::PeerConnection> pc = it->second;
			pc->close();
			_peerConnections.erase(it);
			
			PLOG_INFO << "Client[" << clientId << "] PeerConnection closed";
		}
		lock.unlock();
	}
	
	void reset() {
		for (const auto & [key, dc] : _dataChannels) {
			dc->close();
		}
		
		for (const auto & [key, pc] : _peerConnections) {
			pc->close();
		}
		
		_dataChannels.clear();
		_peerConnections.clear();
	}
};
