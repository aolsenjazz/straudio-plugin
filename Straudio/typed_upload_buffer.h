/**
 Custom buffers for sending PCM data to each client. Reuses the same buffer _buffer<T> to avoid uncessary allocation of memory.
 Each buffer contains a header consisting of metadata related to the PCM, and a body consisting of PCM data. Each message
 is structured as such:
 
 __________________
 | 8 bytes timestamp|
 |_________________|
 | 8 bytes dtype        | DTYPE_INT16 or DTYPE_FLOAT32
 |_________________|
 |0-8192 bytes PCM|
 |_________________|
 */

#pragma once

#include "upload_buffer.h"
#include <chrono>
#include <algorithm>

using namespace std::chrono;

template <typename T>
class TypedUploadBuffer : public UploadBuffer {
	
private:
	
	/**
	 Sent as second part of header. Informs clients of how to transform data
	 */
	const double DTYPE_INT16 = 0;
	
	/**
	Sent as second part of header. Informs clients of how to transform data
	*/
	const double DTYPE_FLOAT32 = 1;
	
	/**
	 20000 is an arbitrary size. As long as size is > 8200, shouldn't matter much
	 */
	T _buffer[20000];
	
	std::function<void(T*, size_t)> _submitFunc;
	
	int getTimestampIndex() {
		return 0;
	}
	
	/**
	 Returns the index of the data type portion of header
	 */
	int getDTypeIndex() {
		return getTimestampLength();
	}
	
	/**
	 Returns the first index in buffer at which PCM data starts
	 */
	int getBodyIndex() {
		return getHeaderSize() / sizeof(T);
	}
	
	/**
	 Returns size in bytes of header portion of buffer
	 */
	int getHeaderSize() {
		// "sizeof timestamp" plus "sizeof data type"
		return sizeof(double) + sizeof(double);
	}
	
	/**
	 Returns the length (in number of T) of the timestamp. E.g. for T=float32, this will return 2
	 */
	int getTimestampLength() {
		return sizeof(double) / sizeof(T);
	}
	
	/**
	 Returns the data type to send to the client.
	 */
	int getDTypeBufferVal() {
		if constexpr (std::is_same_v<T, short>) {
			return DTYPE_INT16;
		}
		
		return DTYPE_FLOAT32;
	}
	
public:
	
	TypedUploadBuffer(std::function<void(T*, size_t)> submitFunc, std::function<void(int, int, int)> onReadyCb, int sampleR)
	: UploadBuffer(onReadyCb, sampleR) {
		_submitFunc = submitFunc;
	}
	
	int bitDepth() {
		return sizeof(T) * 8;
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
		
		int bufferPos = getBodyIndex();
		int typeMult = dTypeMultiplier();
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				_buffer[bufferPos] = (inputs[c][s] > 1) ? typeMult : inputs[c][s] * typeMult;
				bufferPos++;
			}
		}
		
		// place the current time in MS at index 0
		double timeMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		double* timestampPtr = (double*) &_buffer[0];
		double* dtypePtr = (double*) &_buffer[getDTypeIndex()];
		*timestampPtr = timeMs;
		*dtypePtr = getDTypeBufferVal();

		_submitFunc(_buffer, nFrames * nChans * sizeof(T) + getHeaderSize());
	}
};
