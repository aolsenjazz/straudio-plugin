#pragma once

#include "upload_buffer.h"
#include <chrono>

using namespace std::chrono;

template <typename T>
class TypedUploadBuffer : public UploadBuffer {
	
private:
	
	T* _buffer = NULL;
	
	int dtypeMultiplier;
	
	std::function<void(T*, size_t)> _submitFunc;
	
	void setBufferLength() {
		if (sampleRate <= 48000) {
			bufferLength = std::max(batchSize, 512);
		} else if (sampleRate <= 96000) {
			bufferLength = std::max(batchSize, 1024);
		} else {
			bufferLength = std::max(batchSize, 4096);
		}
		
		int headerSize = sizeof(double) / sizeof(T);
		bufferLength = bufferLength * nChannels;
		bufferLength += headerSize;
		
		_buffer = new T[bufferLength];
	}
	
	int getFirstPcmIndex() {
		return sizeof(double) / sizeof(T);
	}
	
	int getBufferSize() {
		return (bufferLength - getHeaderLength()) * dTypeSize() + getHeaderSize();
	}
	
	int getHeaderSize() {
		return sizeof(double);
	}
	
	int getHeaderLength() {
		return sizeof(double) / sizeof(T);
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
		_collectedFrames = getFirstPcmIndex();
		
		sampleRate = sampleR;
		nChannels = nChans;
		batchSize = bSize;
		
		setBufferLength();
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
			setBufferLength();
		}
		
		if (bufferLength == 0) {
			return;
		}
	
		int typeMult = dTypeMultiplier();
		if (nFrames < batchSize) {
			// when looping in different DAWs:
			// ableton: shrink nFrames, process as usual
			// logic: drop if nFrames < batch size.
			// easy to just drop and doesnt affect UX
		} else {
			// Fill the buffer
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nChans; c++) {
					_buffer[_collectedFrames] = inputs[c][s] * typeMult;
					_collectedFrames++;
				}
			}
			
			// Upload once buffer is full
			if (_collectedFrames == bufferLength) {
				double d = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
				double* dptr = (double*) &_buffer[0];
				*dptr = d;
				
				
				
//				PLOG_INFO << ((double) _buffer[0]);
				
//				PLOG_INFO << d2;
				_submitFunc(_buffer, getBufferSize());
				_collectedFrames = getFirstPcmIndex();
			}
		}
	}
};
