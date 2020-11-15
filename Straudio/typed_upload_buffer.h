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
 
 Dtype will always be Float32 from now on. I'm keeping the capability for different data types just in case, but the whole system is more performant
 using Float32 because the web audio API is built to handle Float32 data.
 */

#pragma once

#include "upload_buffer.h"
#include <chrono>
#include <algorithm>

using namespace std::chrono;

template <typename T>
class TypedUploadBuffer : public UploadBuffer {
	
private:
	
	bool doResample = true;
	
	/**
	 Sent as second part of header. Informs clients of how to transform data
	 */
	const double DTYPE_INT16 = 0;
	
	/**
	Sent as second part of header. Informs clients of how to transform data
	*/
	const double DTYPE_FLOAT32 = 1;
	
	int bufferPos = 0;

	SRC_DATA srcData;
	
	/**
	 20000 is an arbitrary size. As long as size is > 8200, shouldn't matter much
	 */
	float _interleavedData[20000];
	float _resampleOut[20000];
	T _uploadBuffer[20000];
	
	std::function<void(T*, size_t)> _submitFunc;
	
	/**
	 USED IN process_block. Removed variable declarations because optimizations
	 */
	bool notSilent = false;
	int dataToTransmit = 0;
	int interleavedPos = 0;
	int processError = 0;
	int bodyIndex = getBodyIndex();
	int typeMult = dTypeMultiplier();
	/**
	 </USED IN process_block>
	 */
	
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
	
	void updateInternals(int nChans) {
		updateRequired = false;
		nChannels = nChans;
		
		// if we need to resample, init src
		if (inputSampleRate != outputSampleRate) {
			// cleanup old sample rate converter
			src_delete(src);
			
			// initialize a new sample rate converter
			int* error = new int;
			src = src_new(SRC_SINC_BEST_QUALITY, nChans, error);
			if (*error != 0) {
				doResample = false;
				outputSampleRate = inputSampleRate;
				return;
			}
			
			// update src data object
			srcData.src_ratio = outputSampleRate / (double) inputSampleRate;
			doResample = true;
			
			// clean up
			delete error;
		} else {
			doResample = false;
		}
		
		_onReadyCb(outputSampleRate, nChannels, bitDepth());
	}
	
public:
	
	TypedUploadBuffer(std::function<void(T*, size_t)> submitFunc, std::function<void(int, int, int)> onReadyCb, int sampleR)
	: UploadBuffer(onReadyCb, sampleR) {
		_submitFunc = submitFunc;
		
		srcData.end_of_input = 0;
		srcData.data_out = _resampleOut;
		srcData.data_in = _interleavedData;
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
		// audio settings change, init/reinit sample rate converter
		if (nChans != nChannels || updateRequired) {
			updateInternals(nChans);
		}
		
		// resample if necessary
		if (doResample) {

			// interleave channels if necessary
			interleavedPos = 0;
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nChans; c++) {
					if (inputs[c][s] != 0) {
						notSilent = true;
					}

					_interleavedData[interleavedPos] = inputs[c][s];
					interleavedPos++;
				}
			}

			srcData.input_frames = nFrames;
			srcData.output_frames = nFrames;

			processError = src_process(src, &srcData);
			if (processError != 0) {
				PLOG_ERROR << processError << std::endl;

				doResample = false; // if resampling fails, stop resampling
				outputSampleRate = inputSampleRate;
			}

			// copy resampled data into the upload buffer
			for (int j = 0; j < srcData.output_frames_gen * nChans; j++) {
				_uploadBuffer[j + bodyIndex] = (_resampleOut[j] > 1) ? typeMult : _resampleOut[j] * typeMult;
			}
			
			dataToTransmit = srcData.output_frames_gen * nChans * sizeof(T) + getHeaderSize();
		} else {
			// just copy input to the upload buffer
			bufferPos = bodyIndex;
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nChans; c++) {
					if (inputs[c][s] != 0) {
						notSilent = true;
					}
					
					_uploadBuffer[bufferPos] = (inputs[c][s] > 1) ? typeMult : inputs[c][s] * typeMult;
					bufferPos++;
				}
			}
			
			dataToTransmit = nFrames * nChans * sizeof(T) + getHeaderSize();
		}
		
		// add headers
		double* timestampPtr = (double*) &_uploadBuffer[0];
		double* dtypePtr = (double*) &_uploadBuffer[getDTypeIndex()]; // this will always be DTYPE_FLOAT32
		*timestampPtr = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		*dtypePtr = getDTypeBufferVal();
		
		if (notSilent) {
			_submitFunc(_uploadBuffer, dataToTransmit);
		}
	}
};
