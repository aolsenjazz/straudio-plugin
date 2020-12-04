#pragma once

#include "libsamplerate/src_config.h"
#include "libsamplerate/samplerate.h"

class UploadBuffer {
	
protected:
	
	std::function<void(int, int)> _onReadyCb;
	int _inputSampleRate;
	
public:
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	UploadBuffer(std::function<void(int, int)> onReadyCb, int sampleR) {
		_inputSampleRate = sampleR;
		_onReadyCb = onReadyCb;
	}
	
	virtual ~UploadBuffer() {}
	
	// getters
	virtual int getBitDepth() = 0;
	virtual int getOutputSampleRate() = 0;
	virtual int getInputSampleRate() = 0;
	virtual int getSrcQuality() = 0;
	
	// setters
	virtual void setInputSampleRate(int sampleRate) = 0;
	virtual void setSrcQuality(int quality) = 0;
	virtual void setOutputSampleRate(int sampleRate) = 0;
	
	virtual void processBlock(iplug::sample** inputs, int nFrames, int nChans) = 0;
};
