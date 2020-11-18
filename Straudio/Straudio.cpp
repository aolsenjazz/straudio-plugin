#define _WINSOCKAPI_
#include <windows.h>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include "ui/LoginPanel.h"
#include "ui/AudioInfoPanel.h"
#include "web_services_manager.h"
#include "audio_propagator.h"
#include "idle_upload_buffer.h"

using namespace std::placeholders;

Straudio::Straudio(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(kNumParams, kNumPrograms)) {
	ix::initNetSystem();

	GetParam(kMonitor)->InitBool("Monitor", false);
	
	auto boundSigStateChange = std::bind(&Straudio::signalStateChange, this);
	auto boundRoomStateChange = std::bind(&Straudio::roomStateChange, this);
	auto boundOnError = std::bind(&Straudio::onError, this, _1, _2);
	wsm = std::make_unique<WebServicesManager>(room, signalState, boundSigStateChange,
											   boundRoomStateChange, boundOnError);
	
	auto boundSend = std::bind(&Straudio::sendData<short>, this, _1, _2);
	auto boundUpdateAudio = std::bind(&Straudio::onBufferReady, this, _1, _2, _3);
	uploadBuffer = std::make_unique<TypedUploadBuffer<short>>(boundSend, boundUpdateAudio, GetSampleRate());
	
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};
	mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics) {
		// Global Stuff
		pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_GRAY);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const iplug::igraphics::IRECT root = pGraphics->GetBounds();
		const iplug::igraphics::IRECT monitorCtrlRect = root.GetFromBRHC(250, 100);
		const iplug::igraphics::IRECT bitDepthRect = root.GetFromTRHC(250, 100);
		
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(root.GetMidVPadded(50), roomMsg->c_str(), iplug::igraphics::IText(50)), 69);
		
		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(monitorCtrlRect, kMonitor, "Monitor", iplug::igraphics::DEFAULT_STYLE, "Off", "On"));
	  };
}

void Straudio::OnUIOpen() {
	PLOG_DEBUG << "OnUiOpen";
}
void Straudio::OnUIClose() {
	PLOG_DEBUG << "OnUiClose";
}

void Straudio::OnIdle() {
	if (uploadBuffer != nullptr) {
		uploadBuffer->upload();
	}
}

void Straudio::OnActivate(bool active) {
	PLOG_DEBUG << "OnActivate() -> active = " << active;

	if (active && GetSampleRate() != uploadBuffer->inputSampleRate) {
		uploadBuffer->inputSampleRate = GetSampleRate();
		uploadBuffer->notifySettingsChange();
	}
}

void Straudio::OnReset() {
	PLOG_DEBUG << "OnReset()";

	wsm->notifyBufferReset();
	
	if (GetSampleRate() != uploadBuffer->inputSampleRate) {
		uploadBuffer->inputSampleRate = GetSampleRate();
		uploadBuffer->notifySettingsChange();
	}
}

void Straudio::onError(std::string severity, std::string message) {
	PLOG_ERROR << "Error[" << severity << "] " << message;
}

void Straudio::OnParamChange(int paramIdx) {
	switch(paramIdx) {
		
	}
}

void Straudio::signalStateChange() {
	PLOG_DEBUG << "Connection state change. State: " << *signalState;

	if (*signalState == "open") {
		if (uploadBuffer->nChannels == 0) {
			PLOG_ERROR << "SENDING 0 CHANNELS~~~" << std::endl;
		}
		
		wsm->ss->createRoom(uploadBuffer->outputSampleRate(), uploadBuffer->nChannels, uploadBuffer->bitDepth());
	} else {
		wsm->closePeerConnections(); // something happened with the connection. close peer connections
		setRoomStatusMessage("Disconnected...");
	}
}

void Straudio::roomStateChange() {
	PLOG_DEBUG << "Room state change. State:" << room->toString();

	std::string msg = "Creating room...";
	if (room->state == "open") {
		std::ostringstream os;
		os << "Room ID: " << room->rId;
		msg = os.str();
	}

	setRoomStatusMessage(msg);
}

void Straudio::onBufferReady(int sampleRate, int nChans, int bitDepth) {
	if (room->state == "open" && *signalState == "open") {
		PLOG_INFO << "Audio details changed. Updating server info...";
		if (nChans == 0) {
			PLOG_ERROR << "SENDING 0 CHANNELS~~~" << std::endl;
		}
		wsm->updateAudioSettings(sampleRate, nChans, bitDepth);
	} else {
		PLOG_DEBUG << "Tried to send audio details while room || signal != open. Ignoring...";
	}
}

void Straudio::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
    const int nChans = NOutChansConnected();

	if (GetParam(kMonitor)->Bool()) AudioPropagator::propagateAudio(inputs, outputs, nFrames, nChans);
	else AudioPropagator::propagateSilence(outputs, nFrames, nChans);

	uploadBuffer->processBlock(inputs, nFrames, nChans);
}

template <typename T>
void Straudio::sendData(T* data, size_t size) {
	wsm->sendData(data, size);
}

void Straudio::setRoomStatusMessage(std::string msg) {
	roomMsg.reset(new std::string(msg));

	if (GetUI()) {
		// refactor this nonsense (carefully, when you have the time)
		char const *formatted = roomMsg->c_str();
		((iplug::igraphics::ITextControl*) GetUI()->GetControlWithTag(69))->SetStr(formatted);
	}
}
