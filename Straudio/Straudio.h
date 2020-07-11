#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "web_services_manager.h"
#include "domain.h"

const int kNumPrograms = 0;

enum EParams {
    kMonitor = 0,
	kStream = 1,
    kNumParams
};

class Straudio final : public iplug::Plugin {

public:
	Straudio(const iplug::InstanceInfo& info);
	void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
	void OnReset() override;
	void OnUIOpen() override;
	void OnUIClose() override;
	void OnRestoreState() override;
	void OnHostIdentified() override;
	void OnIdle() override;
	void OnActivate(bool active) override;
	
	~Straudio();
	
private:
	void onLoginSuccess();
	bool uiOpen = false;
	void initStream();
	void onConnectionChange(std::string connectionState);
	void onRoomChange(std::string roomState, Room r);
	void onError(std::string severity, std::string message);
	WebServicesManager* wsm;
};
