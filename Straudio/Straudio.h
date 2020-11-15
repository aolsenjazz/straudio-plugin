#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "web_services_manager.h"
#include "domain.h"
#include "logger_init.h"
#include "upload_buffer.h"

int kNumPrograms = 0;

enum EParams {
    kMonitor = 0,
    kNumParams
};

class Straudio final : public iplug::Plugin {

public:
	Straudio(const iplug::InstanceInfo& info);
	void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
	void OnReset() override;
	void OnUIOpen() override;
	void OnUIClose() override;
	void OnIdle() override;
	void OnActivate(bool active) override;
	void OnParamChange(int paramIdx) override;
	
private:
	void signalStateChange();
	void roomStateChange();
	void onError(std::string severity, std::string message);
	
	template <typename T>
	void sendData(T* data, size_t size);
	void onBufferReady(int sampleRate, int nChans, int bitDepth);
	
	void setRoomStatusMessage(std::string);
	
	std::unique_ptr<WebServicesManager> wsm = nullptr;
	std::unique_ptr<UploadBuffer> uploadBuffer = nullptr;
	
	std::shared_ptr<Room> room = Room::CLOSED_PTR();
	std::shared_ptr<std::string> signalState = std::make_shared<std::string>("closed");
	
	std::unique_ptr<std::string> roomMsg = std::make_unique<std::string>("Sanity");
	
};
