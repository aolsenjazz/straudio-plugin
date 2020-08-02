#pragma once

#include "upload_buffer.h"

template <typename T>
class TypedUploadBuffer : public UploadBuffer {
	
private:
	
	T* _buffer = NULL;
	
	int dtypeMultiplier;
	
	std::function<void(T*, size_t)> _submitFunc;
	
	void setBufferSize() {
		if (sampleRate <= 48000) {
			bufferSize = std::max(batchSize, 512);
		} else if (sampleRate <= 96000) {
			bufferSize = std::max(batchSize, 1024);
		} else {
			bufferSize = std::max(batchSize, 4096);
		}
		
		_buffer = new T[bufferSize * nChannels];
	}
	
public:
	
	/**
	 nChannels and batchSize probably aren't known when this object is constructed, so these will probably be set in a later processBlock()
	 or updateSettings().
	 */
	TypedUploadBuffer(std::function<void(T*, size_t)> submitFunc, std::function<void(int, int, int)> onReadyCb,
					  int sampleR, int nChans = 0, int bSize = 0)
	: UploadBuffer(onReadyCb, sampleR) {
		_onReadyCb = onReadyCb;
		_submitFunc = submitFunc;
		
		sampleRate = sampleR;
		nChannels = nChans;
		batchSize = bSize;
		
		setBufferSize();
	}
	
	~TypedUploadBuffer() {
		if (_buffer != NULL) delete _buffer;
	}
	
	int bitDepth() {
		return sizeof(T) * 8;
	}
	
	int dTypeSize() {
		return sizeof(T);
	}
	
	/**
	 Useful for sending different datatypes to the server. E.g. Float32 = 1, int16 = SHRT_MAX
	 */
	int dTypeMultiplier() {
		if constexpr (std::is_same_v<T, short>) {
			return SHRT_MAX;
		}
		
		return 1;
	}
	
	void processBlock(iplug::sample** inputs, int nFrames, int nChans) {
		if (nChans != nChannels) {
			nChannels = nChans;
			_onReadyCb(sampleRate, nChannels, bitDepth());
		}
		
		if (batchSize == 0) {
			batchSize = nFrames;
			setBufferSize();
		}
		
		if (bufferSize == 0) {
			return;
		}
	
		int typeMult = dTypeMultiplier();
		bool uploadImmediately = nFrames < batchSize; // ableton dynamically reduces nFrames for loops
		if (uploadImmediately) {
			T tmpBuffer[nFrames];
			
			// fill the temporary buffer
			int tmpCollectedFrames = 0;
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nChans; c++) {
					tmpBuffer[tmpCollectedFrames] = inputs[c][s] * typeMult;
					tmpCollectedFrames++;
				}
			}
			
			_submitFunc(_buffer, bufferSize * nChannels * dTypeSize());
			
		} else {
			// Fill the buffer
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nChans; c++) {
					_buffer[_collectedFrames] = inputs[c][s] * typeMult;
					_collectedFrames++;
				}
			}
			
			// Upload once buffer is full
			if (_collectedFrames == bufferSize * nChannels) {
				_submitFunc(_buffer, bufferSize * nChannels * dTypeSize());
				_collectedFrames = 0;
			}
		}
	}
};
