#pragma once

class UploadBuffer {
	
protected:
	
	int _collectedFrames = 0; // _collected frames == total number samples buffered
	
	std::function<void(int, int, int)> _onReadyCb;
	
public:

	int sampleRate = 0;
	int nChannels = 0;
	int batchSize = 0;
	
	/**
	 Computed
	 */
	int bufferSize = 0;
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	UploadBuffer(std::function<void(int, int, int)> onReadyCb, int sampleR) {
		_onReadyCb = onReadyCb;
		sampleRate = sampleR;
	}
	
	virtual ~UploadBuffer() {}
	
	virtual void processBlock(iplug::sample** inputs, int nFrames, int nChans) = 0;
	virtual int bitDepth() = 0;
};