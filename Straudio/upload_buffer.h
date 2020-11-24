#pragma once

#include "libsamplerate/src_config.h"
#include "libsamplerate/samplerate.h"

class UploadBuffer {
	
protected:
	
	std::function<void(int, int, int)> _onReadyCb;
	
	/**
	 The internal sample rate of the DAW. Potentially different from the _outputSampleRate.
	 */
	int inputSampleRate = 0;
	
public:

	int nChannels = 0;
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	UploadBuffer(std::function<void(int, int, int)> onReadyCb, int sampleR) {
		_onReadyCb = onReadyCb;
		inputSampleRate = sampleR;
	}
	
	virtual ~UploadBuffer() {

	}
	
	virtual void processBlock(iplug::sample** inputs, int nFrames, int nChans) = 0;
	virtual int bitDepth() = 0;
	virtual int outputSampleRate() = 0;
	virtual void upload() = 0;
	virtual void setInputSampleRate(int sampleRate) = 0;
};
