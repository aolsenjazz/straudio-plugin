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

#include "libsamplerate/src_config.h"

using namespace std::placeholders;

Straudio::Straudio(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(kNumParams, kNumPrograms)) {
	// initialize the websocket library
	ix::initNetSystem();

	GetParam(kMonitor)->InitBool("Monitor", false);
	GetParam(kBitDepth)->InitEnum("Bit Depth", 0, 2, "", iplug::IParam::kFlagsNone, "", "16 bit", "32 bit");
	GetParam(kOutputSampleRate)->InitEnum("Sample Rate", 0, 4, "", iplug::IParam::kFlagsNone, "", "44100", "48000", "88200", "96000");
	GetParam(kSrcQuality)->InitEnum("SRC Quality", 0, 3, "", iplug::IParam::kFlagsNone, "", "Highest", "Medium", "Fastest");
	
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
		const iplug::igraphics::IRECT monitorCtrlRect = root.GetGridCell(7, 3, 8, 4);
		const iplug::igraphics::IRECT bitDepthRect = root.GetGridCell(6, 3, 8, 4);
		const iplug::igraphics::IRECT sampleRateRect = root.GetGridCell(5, 3, 8, 4);
		const iplug::igraphics::IRECT srcRect = root.GetGridCell(4, 3, 8, 4);
		
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(root.GetMidVPadded(50), roomMsg->c_str(), iplug::igraphics::IText(50)), 69);
		
		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(monitorCtrlRect, kMonitor, "Monitor", iplug::igraphics::DEFAULT_STYLE, "Off", "On"));
		
		pGraphics->AttachControl(new iplug::igraphics::ICaptionControl(bitDepthRect, EParams::kBitDepth));
		pGraphics->AttachControl(new iplug::igraphics::ICaptionControl(sampleRateRect, EParams::kOutputSampleRate));
		pGraphics->AttachControl(new iplug::igraphics::ICaptionControl(srcRect, EParams::kSrcQuality));
	  };
}

void Straudio::OnUIOpen() {
	PLOG_DEBUG << "OnUiOpen";
}
void Straudio::OnUIClose() {
	PLOG_DEBUG << "OnUiClose";
}

void Straudio::OnIdle() {
	if (uploadBuffer != nullptr && !room->isEmpty()) {
		uploadBuffer->upload();
	}
}

void Straudio::OnActivate(bool active) {
	PLOG_DEBUG << "OnActivate() -> active = " << active;
	
	uploadBuffer->setInputSampleRate(GetSampleRate());
}

void Straudio::OnReset() {
	PLOG_DEBUG << "OnReset()";

	wsm->notifyBufferReset();
	uploadBuffer->setInputSampleRate(GetSampleRate());
}

void Straudio::onError(std::string severity, std::string message) {
	PLOG_ERROR << "Error[" << severity << "] " << message;
}

void Straudio::OnParamChange(int paramIdx) {
	switch(paramIdx) {
		case kBitDepth:
			setUploadBuffer(GetParam(paramIdx)->Int());
			break;
		case kOutputSampleRate:
			setOutputSampleRate(GetParam(paramIdx)->Int());
			break;
		case kSrcQuality:
			setSrcQuality(GetParam(paramIdx)->Int());
			break;
	}
}

void Straudio::signalStateChange() {
	PLOG_DEBUG << "Connection state change. State: " << *signalState;

	if (*signalState == "open") {
		wsm->ss->createRoom(uploadBuffer->getOutputSampleRate(), uploadBuffer->nChannels, uploadBuffer->bitDepth());
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
		wsm->updateAudioSettings(sampleRate, nChans, bitDepth);
	}
}

void Straudio::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
    const int nChans = NOutChansConnected();

	if (GetParam(kMonitor)->Bool()) AudioPropagator::propagateAudio(inputs, outputs, nFrames, nChans);
	else AudioPropagator::propagateSilence(outputs, nFrames, nChans);

	if (!room->isEmpty()) {
		uploadBuffer->processBlock(inputs, nFrames, nChans);
	}
}

void Straudio::setSrcQuality(int enumQualityValue) {
	int quality = 0;
	switch (enumQualityValue) {
		case 0:
			quality = SRC_SINC_BEST_QUALITY;
			break;
		case 1:
			quality = SRC_SINC_MEDIUM_QUALITY;
			break;
		case 2:
			quality = SRC_SINC_FASTEST;
		default:
			quality = SRC_SINC_FASTEST;
			break;
	}
	
	uploadBuffer->setSrcQuality(quality);
}

void Straudio::setOutputSampleRate(int enumSrValue) {
	int sr = 0;
	
	switch (enumSrValue) {
		case 0:
			sr = 40000;
			break;
		case 1:
			sr = 44100;
			break;
		case 2:
			sr = 48000;
			break;
		case 3:
			sr = 88200;
			break;
		case 4:
			sr = 96000;
			break;
		default:
			
			break;
	}
	
	uploadBuffer->setOutputSampleRate(sr);
}

void Straudio::setUploadBuffer(int enumVal) {
	auto boundUpdateAudio = std::bind(&Straudio::onBufferReady, this, _1, _2, _3);
	
	if (enumVal == 0) {
		auto boundSend = std::bind(&Straudio::sendData<short>, this, _1, _2);
		uploadBuffer.reset(new TypedUploadBuffer<short>(boundSend, boundUpdateAudio, GetSampleRate(), uploadBuffer->getOutputSampleRate(), uploadBuffer->getSrcQuality()));
	} else {
		auto boundSend = std::bind(&Straudio::sendData<float>, this, _1, _2);
		uploadBuffer.reset(new TypedUploadBuffer<float>(boundSend, boundUpdateAudio, GetSampleRate(), uploadBuffer->getOutputSampleRate(), uploadBuffer->getSrcQuality()));
	}
}

template <typename T>
void Straudio::sendData(T* data, size_t size) {
	wsm->sendData(data, size);
}

void Straudio::setRoomStatusMessage(std::string msg) {
	roomMsg.reset(new std::string(msg));

	if (GetUI()) {
		// TODO: refactor this nonsense (carefully, when you have the time)
		char const *formatted = roomMsg->c_str();
		((iplug::igraphics::ITextControl*) GetUI()->GetControlWithTag(69))->SetStr(formatted);
	}
}
