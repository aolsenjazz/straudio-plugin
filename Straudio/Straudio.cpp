#define _WINSOCKAPI_
#include <windows.h>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include "ui/LoginPanel.h"
#include "ui/AudioInfoPanel.h"
#include "web_services_manager.h"
#include "audio_propagator.h"
#include "upload_bufferer.h"

using namespace std::placeholders;

Straudio::Straudio(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(kNumParams, kNumPrograms)) {
	GetParam(kMonitor)->InitBool("Monitor", false);
	GetParam(kStream)->InitBool("Stream", true);
	GetParam(kUploadBufferSize)->InitEnum("UploadBufferSize", 2, 6, "", iplug::IParam::kFlagsNone,
										  "", "256", "512", "1024", "2048", "4096", "8192");

	auto boundSigStateChange = std::bind(&Straudio::signalStateChange, this);
	auto boundRoomStateChange = std::bind(&Straudio::roomStateChange, this);
	auto boundOnError = std::bind(&Straudio::onError, this, _1, _2);
	wsm = std::make_unique<WebServicesManager>(room, signalState, boundSigStateChange,
											   boundRoomStateChange, boundOnError);
	
	auto boundSendPcm = std::bind(&Straudio::sendPcmData, this, _1, _2);
	auto boundUpdateAudio = std::bind(&Straudio::onBufferReady, this, _1, _2, _3, _4);
	uploadBuffer = std::make_unique<PcmUploadBuffer>(boundSendPcm, boundUpdateAudio, GetSampleRate(),
													 GetParam(kUploadBufferSize)->Int());
	
	Preferences::setAuth("bar");
	
	wsm->connectToSignalling();
	
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
		const iplug::igraphics::IRECT monitorCtrlRect = root.GetFromBRHC(250, 100);

		// Interface elements
//		LoginPanel loginPanel(pGraphics, std::bind(&Straudio::onLoginSuccess, this));
		
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(root.GetMidVPadded(50), roomMsg->c_str(), iplug::igraphics::IText(50)), 69);
//		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(streamCtrlRect, std::bind(&Straudio::initStream, this), "Stream", iplug::igraphics::DEFAULT_STYLE, "Off", "On", false));
		
		pGraphics->AttachControl(new iplug::igraphics::ICaptionControl(streamCtrlRect, kUploadBufferSize, iplug::igraphics::IText(24.f), iplug::igraphics::DEFAULT_FGCOLOR, false), 80085, "misccontrols");
		
		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(monitorCtrlRect, kMonitor, "Monitor", iplug::igraphics::DEFAULT_STYLE, "Off", "On"));
	  };
}

void Straudio::roomStateChange() {
	PLOG_INFO << "Room state change. State:" << room->toString();
	
	std::string msg = "Creating room...";
	if (room->state == "open") {
		std::ostringstream os;
		os << "Room ID: " << room->rId;
		msg = os.str();
	}
	
	roomMsg.reset(new std::string(msg));
	
	if (GetUI()) {
		// refactor this nonsense (carefully, when you have the time)
		char const *formatted = roomMsg->c_str();
		((iplug::igraphics::ITextControl*) GetUI()->GetControlWithTag(69))->SetStr(formatted);
	}
}

void Straudio::signalStateChange() {
	PLOG_INFO << "Connection state change. State: " << *signalState;
	
	if (*signalState == "open") {
		wsm->ss->createRoom(uploadBuffer->sampleRate, uploadBuffer->nChannels, uploadBuffer->batchSize);
	} else {
		wsm->closePeerConnections(); // something happened with the connection. close peer connections
	}
}

void Straudio::onError(std::string severity, std::string message) {
	PLOG_ERROR << "Error[" << severity << "] " << message;
}

void Straudio::sendPcmData(float* data, size_t size) {
	if (GetParam(kStream)->Bool()) wsm->sendPcmData(data, size);
}

void Straudio::onBufferReady(int sampleRate, int nChans, int batchSize, int bufferSize) {
	if (room->state == "open" && *signalState == "open") {
		PLOG_INFO << "Audio details changed. Updating server info...";
		wsm->updateAudioSettings(sampleRate, nChans, batchSize);
	} else {
		PLOG_DEBUG << "Tried to send audio details while room || signal != open. Ignoring...";
	}
}

void Straudio::OnParamChange(int paramIdx) {
	switch(paramIdx) {
		case kUploadBufferSize:
			int bufferSize = PcmUploadBuffer::buffSizeForParamVal(GetParam(paramIdx)->Int());
			if (uploadBuffer->bufferSize != bufferSize)
				uploadBuffer->updateSettings(uploadBuffer->sampleRate, uploadBuffer->nChannels,
											 uploadBuffer->batchSize, bufferSize, false);
			break;
	}
}

void Straudio::OnUIOpen() {}
void Straudio::OnUIClose() {}

void Straudio::OnReset() {
	PLOG_INFO << "OnReset()";
	uploadBuffer->updateSettings(GetSampleRate(), NOutChansConnected(),
								 uploadBuffer->batchSize, uploadBuffer->bufferSize, true);
}
	
void Straudio::OnActivate(bool active) {
	PLOG_INFO << "OnActivate() -> active = " << active;
	
	if (active) {
		uploadBuffer->updateSettings(GetSampleRate(), NOutChansConnected(),
									 uploadBuffer->batchSize, uploadBuffer->bufferSize, true);
	}
}

void Straudio::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
    const int nChans = NOutChansConnected();
	
	if (GetParam(kMonitor)->Bool()) AudioPropagator::propagateAudio(inputs, outputs, nFrames, nChans);
	else AudioPropagator::propagateSilence(outputs, nFrames, nChans);
	
	uploadBuffer->processBlock(inputs, nFrames, nChans);
}
