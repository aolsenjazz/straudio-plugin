#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "web_services_manager.h"
#include "upload_bufferer.h"
#include "domain.h"
#include "logger_init.h"

int kNumPrograms = 0;

enum EParams {
    kMonitor = 0,
	kStream = 1,
	kUploadBufferSize = 2,
    kNumParams
};

class Straudio final : public iplug::Plugin {

public:
	Straudio(const iplug::InstanceInfo& info);
	void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
	void OnReset() override;
	void OnUIOpen() override;
	void OnUIClose() override;
	void OnActivate(bool active) override;
	void OnParamChange(int paramIdx) override;
	
private:
	void signalStateChange();
	void roomStateChange();
	void onError(std::string severity, std::string message);
	
	void sendPcmData(float* data, size_t size);
	void onBufferReady(int sampleRate, int nChans, int batchSize, int bufferSize);
	
	std::unique_ptr<WebServicesManager> wsm;
	std::unique_ptr<PcmUploadBuffer> uploadBuffer;
	
	std::shared_ptr<Room> room = Room::CLOSED_PTR();
	std::shared_ptr<std::string> signalState = std::make_shared<std::string>("closed");
	
	std::unique_ptr<std::string> roomMsg = std::make_unique<std::string>("Creating room...");
	bool uiOpen;
};
