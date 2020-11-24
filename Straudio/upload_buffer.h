#pragma once

#include "libsamplerate/src_config.h"
#include "libsamplerate/samplerate.h"

class UploadBuffer {
	
protected:
	
	std::function<void(int, int, int)> _onReadyCb;
	
public:

	int inputSampleRate = 0;
	int nChannels = 0;
	bool updateRequired = false;
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	UploadBuffer(std::function<void(int, int, int)> onReadyCb, int sampleR) {
		_onReadyCb = onReadyCb;
		inputSampleRate = sampleR;
	}
	
	void notifySettingsChange() {
		updateRequired = true;
	}
	
	virtual ~UploadBuffer() {

	}
	
	virtual void processBlock(iplug::sample** inputs, int nFrames, int nChans) = 0;
	virtual int bitDepth() = 0;
	virtual int outputSampleRate() = 0;
	virtual void upload() = 0;
};
