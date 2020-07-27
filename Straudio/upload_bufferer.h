#pragma once

class PcmUploadBuffer {
	
private:
	
	std::mutex updateLock;
	
	float* _streamBuffer = NULL;
	int _collectedFrames = 0; // _collected frames == total number samples buffered
	
	std::function<void(float*, size_t)> _dataSubmitFunc;
	std::function<void(int, int, int, int)> _onReadyCb;
	
	enum State {
		UPDATING,
		READY,
		UNINITIALIZED,
		MISCONFIGURED
	};
	
	State _state = UNINITIALIZED;
	
public:
	
	static int MIN_BUFFER_SIZE;
	static const int MAX_BUFFER_SIZE = 8192;
	
	int bufferSize = 0;
	int sampleRate = 0;
	int nChannels = 0;
	int batchSize = 0;
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	PcmUploadBuffer(std::function<void(float*, size_t)> dataSubmitFunc,
					std::function<void(int, int, int, int)> onReadyCb,
					int sampleR, int buffSize) {
		_onReadyCb = onReadyCb;
		_dataSubmitFunc = dataSubmitFunc;
		
		int parsedBSize = buffSize < 10 ? PcmUploadBuffer::buffSizeForParamVal(buffSize) : buffSize;
		bufferSize = parsedBSize;
		sampleRate = sampleR;
	}
	
	~PcmUploadBuffer() {
		if (_streamBuffer != NULL) delete _streamBuffer;
	}
	
	void processBlock(iplug::sample** inputs, int nFrames, int nChans) {
		if (nChans != nChannels || nFrames != batchSize || nFrames > bufferSize) {
			int bSize = nFrames > bufferSize ? std::max(nFrames, PcmUploadBuffer::MIN_BUFFER_SIZE) : bufferSize;
			updateSettings(sampleRate, nChans, nFrames,bSize, true);
		}
		
		if (_state != State::READY) return;
		
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				_streamBuffer[_collectedFrames] = inputs[c][s];
				_collectedFrames++;
			}
		}
		
		// we could run into an issue where if in-DAW buffer size is > upload buffer size, this would never trigger
		if (_collectedFrames == bufferSize * nChannels) {
			_dataSubmitFunc(_streamBuffer, bufferSize * nChannels * 4); // might be worth have a separate thread to consume?

			_collectedFrames = 0;
		}
	}
	
	void updateSettings(int sampleR, int nChans, int batchS, int buffSize, bool notifyOnReady) {
		int parsedBSize = buffSize < 10 ? PcmUploadBuffer::buffSizeForParamVal(buffSize) : buffSize;
		
		if (parsedBSize < batchS) {
			PLOG_INFO << "requested bufferSize < batchSize. adjusting...";
			parsedBSize = std::max(MIN_BUFFER_SIZE, batchS);
		}
		
		if (sampleR == 0 || nChans == 0 || batchS == 0) {
			PLOG_INFO << "One or more audio settings === 0. Ignoring update.";
			return;
		}
		
		updateLock.lock();
		_state = UPDATING;
		
 		if (sampleR == sampleRate && nChans == nChannels && batchS == batchSize && bufferSize == parsedBSize) {
			PLOG_INFO << "All audio settings are the same. Skipping update";
			updateLock.unlock();
			_state = READY;
			return;
		}
		
		PLOG_INFO << "Updating audio settings. SampleRate[" << sampleR << "] nChans["
		<< nChans << "] batchSize [" << batchS << "] buffSize[" << parsedBSize << "]";
		
		nChannels = nChans;
		sampleRate = sampleR;
		batchSize = batchS;
		bufferSize = parsedBSize;
		_collectedFrames = 0;
		
		if (_streamBuffer != NULL) delete _streamBuffer;
		_streamBuffer = new float[bufferSize * nChannels];
		
		if (nChannels != 0 && sampleRate != 0 && batchSize != 0) {
			if (notifyOnReady) _onReadyCb(sampleRate, nChannels, batchSize, bufferSize);
			_state = READY;
		} else {
			_state = MISCONFIGURED;
		}
		
		updateLock.unlock();
	}
	
	static int buffSizeForParamVal(int paramVal) {
		return PcmUploadBuffer::MIN_BUFFER_SIZE * pow(2, paramVal);
	}
	
	// god, do I forget how to find function inverses
	static int paramValForBuffSize(int buffSize) {
		switch(buffSize) {
			case 256:
				return 0;
			case 512:
				return 1;
			case 1024:
				return 2;
			case 2048:
				return 3;
			case 4096:
				return 4;
			case 8192:
				return 5;
			default:
				PLOG_ERROR << "Unsupported buffer size: [" << buffSize << "]";
				throw;
		}
	}
	
};

int PcmUploadBuffer::MIN_BUFFER_SIZE = 256;
