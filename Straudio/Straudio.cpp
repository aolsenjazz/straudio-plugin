#define _WINSOCKAPI_
#include <windows.h>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <iostream>
#include <cstdint>
#include "HttpClient.h"
#include "ui/LoginPanel.h"
#include "ui/AudioInfoPanel.h"
#include "ui/MonitorControl.h"
#include "ui/StreamControl.h"
//#include "util/debug_socket.h"
#include "logger.h"
//#include "rtc/rtc.hpp"
#include "shared_config.h"
#include "build_config.h"
#include "web_services_manager.h"
#include "domain.h"

using namespace std::placeholders;

Straudio::Straudio(const iplug::InstanceInfo& info) : iplug::Plugin(info, iplug::MakeConfig(kNumParams, kNumPrograms)) {
	GetParam(kMonitor)->InitBool("Monitor", false);
	GetParam(kMonitor)->InitBool("Stream", false);

	wsm = new WebServicesManager(std::bind(&Straudio::onConnectionChange, this, _1),
								 std::bind(&Straudio::onRoomChange, this, _1, _2),
								 std::bind(&Straudio::onError, this, _1, _2));
	// Make services
//	ss = new SignalService("ws://192.168.86.241:4443/",
//						   std::bind(&Straudio::onClientDescription, this, std::placeholders::_1, std::placeholders::_2),
//						   std::bind(&Straudio::onClientCandidate, this, std::placeholders::_1, std::placeholders::_2));
	
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
	};
	mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics) {
		// Global Stuff
		pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_GRAY);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const iplug::igraphics::IRECT root = pGraphics->GetBounds();
		const iplug::igraphics::IRECT streamCtrlRect = root.GetFromBLHC(250, 100);

		// Interface elements
		LoginPanel loginPanel(pGraphics, std::bind(&Straudio::onLoginSuccess, this));
		
  		pGraphics->AttachControl(new iplug::igraphics::ITextControl(root.GetMidVPadded(50), "Hello iPlug 2!", iplug::igraphics::IText(50)));
		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(streamCtrlRect, std::bind(&Straudio::initStream, this), "Stream", iplug::igraphics::DEFAULT_STYLE, "Off", "On", false));
	  };
}

void Straudio::onRoomChange(std::string roomState, Room r) {
	Loggy::info("Room state change. New state: " + roomState);
	Loggy::info(r.toString());
}

void Straudio::onConnectionChange(std::string connectionState) {
	Loggy::info("Connection state change. New state: " + connectionState);
}

void Straudio::onError(std::string severity, std::string message) {
	Loggy::info("ERROR[" + severity + "] " + message);
}

//void Straudio::onClientDescription(std::string description, std::string type) {
//	pc->setRemoteDescription(rtc::Description(description, type));
//	Loggy::info("remote desc set");
//}

//void Straudio::onClientCandidate(std::string candidate, std::string mid) {
//	pc->addRemoteCandidate(rtc::Candidate(candidate, mid));
//	Loggy::info("remote cand added");
//}

void Straudio::initStream() {
	wsm->ss->createRoom();
	
//	rtc::Configuration config;
//	config.iceServers.emplace_back("stun:stun4.l.google.com:19302");
//	config.iceServers.emplace_back("stun:stun3.l.google.com:19302");
//	config.iceServers.emplace_back("stun:stun2.l.google.com:19302");
//	config.iceServers.emplace_back("stun:stun.l.google.com:19302");
//
//	ss->createRoom();
//	
//	pc = std::make_shared<rtc::PeerConnection>(config);
//
//	pc->onStateChange([&](rtc::PeerConnection::State state) {
//		std::cout << "State: " << state << std::endl;
//		if (state == rtc::PeerConnection::State::Connected) {
//			
//		}
//	});
//
//	pc->onGatheringStateChange(
//	    [](rtc::PeerConnection::GatheringState state) { std::cout << "Gathering State: " << state << std::endl; });
//
//	pc->onLocalDescription([&](const rtc::Description &description) {
//		std::cout << std::string(description);
//		ss->setDescription(description.typeString(), std::string(description));
//	});
//
//	pc->onLocalCandidate([&](const rtc::Candidate &candidate) {
//		ss->addCandidate("candidate", std::string(candidate), "data");
//	});
//
//	pc->onDataChannel([](std::shared_ptr<rtc::DataChannel> dc) {
//		std::cout << "DataChannel success dawg" << std::endl;
//	});
//	
//	auto dc = pc->createDataChannel("test");
//	
//	dc->onOpen([dc]() {
//		std::cout << "DataChannel from ";
//		dc->send("Hello from ");
//	});
//
//	dc->onClosed([]() { std::cout << "DataChannel from "; });
//
//	dc->onMessage([](const std::variant<rtc::binary, std::string> &message) {
//		std::cout << "Message from ";
//	});

}

void Straudio::onLoginSuccess() {
	// Nothing to do here.
}

void Straudio::OnUIOpen() {
	uiOpen = true;
}

void Straudio::OnUIClose() {
	
	uiOpen = false;
}

void Straudio::OnReset() {
//	if (uiOpen) audioInfoPanel->updateAudioInfo(NOutChansConnected(), GetSampleRate(), GetBlockSize());
}
	

void Straudio::OnRestoreState() {
	
}

void Straudio::OnHostIdentified() {
	
}
void Straudio::OnIdle() {
	
}
void Straudio::OnActivate(bool active) {
	
}

void Straudio::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
    const int nChans = NOutChansConnected();
	
//	if (GetParam(kMonitor)->Bool()) monitorControl->propagateAudio(inputs, outputs, nFrames, nChans);
//	else monitorControl->propagateSilence(outputs, nFrames, nChans);
	
	//if (GetParam(kStream)->Bool()) streamControl->onDataAvailable(inputs, nFrames, nChans);
	//else streamControl->flushIfNecessary();
	
}

Straudio::~Straudio() {
	
}
