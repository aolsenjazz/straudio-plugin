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
	static constexpr double DTYPE_INT16 = 0;
	
	/**
	Sent as second part of header. Informs clients of how to transform data
	*/
	static constexpr double DTYPE_FLOAT32 = 1;
	
	/**
	 192k = 1 second of audio at max sample rate. Also 100% arbitrary. See _swapBuffer1, _resampleOut for more
	 */
	static constexpr int BUFFER_SIZE = 16384;
	
	/**
	 If the _outputSampleRate == inputSampleRate, this is set to false. Also will be set to false on resampling error
	 */
	bool _doResample = true;
	
	/**
	 When important audio details are changed (sample rate, nChannels, etc), this is marked as true so that during the next run of processBlock(),
	 updates are made.
	 */
	bool _updateRequired = false;
	
	/**
	 Guards sensitive read/write operations in processBlock() and upload()
	 */
	std::mutex _bufferSwapMtx;
	
	/**
	 Guards against replacing the src while we're actively using it in upload()
	 */
	std::mutex _srcMtx;
	
	/**
	 Data is resampled to this rate if different from the DAW's internal sample rate
	 */
	int _outputSampleRate = 40000;

	/**
	 The struct used to resample audio data before transmitting to clients
	 */
	SRC_DATA _srcData;
	
	/**
	 Sample rate converter
	 http://www.mega-nerd.com/SRC/api.html
	 */
	SRC_STATE *_src = nullptr;
	
	/**
	 The audio thread is constantly writing audio data to one
	 of these two buffers. OnIdle(), swap which buffer is being written into while the other is resampled (if necessary) and transmitted to clients.
	 These array shouldn't be accessed directly; swapBuffer1 and swapBuffer2 pointers should be used
	 */
	float _swapBuffer1[BUFFER_SIZE];
	float _swapBuffer2[BUFFER_SIZE];
	
	/**
	 Used to track write process into the buffers in processBlock(), and to determine how much data to resample/transmit in upload().
	 These values are swapped in every OnIdle() like their accompanying buffers.
	 */
	int _swapBufferIndex1 = 0;
	int _swapBufferIndex2 = 0;
	
	/**
	 Don't bother uploading the contents of a buffer if it's all 0's. These bools track whether each buffer is silent
	 */
	bool _swapBuffer1Silent = true;
	bool _swapBuffer2Silent = true;
	
	/**
	 During every OnIdle(), this is set to !_swap. Changing this bool swap which buffer the audio thread is writing into, allowing for the
	 OnIdle thread to resample and transmit to clients concurrently
	 */
	bool _swap = false;
	
	/**
	 The destination to which resampled audio is written via src_process in _resampleData
	 */
	float _resampleOut[BUFFER_SIZE];
	
	/**
	 The buffer to which data is written before sending to the websocket. T is of type short of float32
	 */
	T _uploadBuffer[BUFFER_SIZE];
	
	/**
	 Called in upload() to transfer data to the websocket service.
	 */
	std::function<void(T*, size_t)> _submitFunc;
	
	/**
	 The index of the timestamp, stored as a double, in each message sent to client
	 */
	int _getTimestampIndex() {
		return 0;
	}
	
	/**
	 Returns the index of the data type portion of header
	 */
	int _getDTypeIndex() {
		return _getTimestampLength();
	}
	
	/**
	 Returns the first index in buffer at which PCM data starts
	 */
	int _getBodyIndex() {
		return _getHeaderSize() / sizeof(T);
	}
	
	/**
	 Returns size in bytes of header portion of buffer
	 */
	int _getHeaderSize() {
		// "sizeof timestamp" plus "sizeof data type"
		return sizeof(double) + sizeof(double);
	}
	
	/**
	 Returns the length (in number of T) of the timestamp. E.g. for T=float32, this will return 2
	 */
	int _getTimestampLength() {
		return sizeof(double) / sizeof(T);
	}
	
	/**
	 Returns the data type to send to the client.
	 */
	int _getDTypeBufferVal() {
		if constexpr (std::is_same_v<T, short>) {
			return DTYPE_INT16;
		}
		
		return DTYPE_FLOAT32;
	}
	
	/**
	 Useful for sending different datatypes to the server. E.g. Float32 = 1, int16 = SHRT_MAX
	 */
	int _dTypeMultiplier() {
		if constexpr (std::is_same_v<T, short>) {
			return SHRT_MAX;
		}
		
		return 1;
	}
	
	void _updateInternals(int nChans) {
		_updateRequired = false;
		
		if (inputSampleRate != outputSampleRate() || nChannels != nChans) {
			_srcMtx.lock();
			// delete the old src if exists, init new
			if (_src != nullptr) src_delete(_src);
			
			// initialize the sample rate converter
			int* error = new int;
			_src = src_new(SRC_SINC_BEST_QUALITY, nChans, error);
			if (*error != 0) _outputSampleRate = inputSampleRate;
			
			_srcMtx.unlock();
				
			_doResample = *error == 0;
			_srcData.src_ratio = outputSampleRate() / (double) inputSampleRate;
			nChannels = nChans;
			delete error;
		} else {
			_doResample = false;
		}
		
		_onReadyCb(outputSampleRate(), nChannels, bitDepth());
	}
	
	int resampleData(int nSamplesToResample, float *sourceBuffer) {
		_srcData.input_frames = nSamplesToResample / nChannels;
		_srcData.output_frames = nSamplesToResample / nChannels;
		_srcData.data_in = sourceBuffer;
	
		int processError = src_process(_src, &_srcData);
		if (processError != 0) {
			PLOG_ERROR << processError << std::endl;

			_doResample = false; // if resampling fails, stop resampling
			_outputSampleRate = inputSampleRate;
		}
		
		return _srcData.output_frames_gen * nChannels;
	}
	
	int copyToUploadBuffer(int startIndex, int nSamplesToCopy, float *sourceBuffer) {
		// copy resampled data into the upload buffer
		for (int j = 0; j < nSamplesToCopy; j++) {
			_uploadBuffer[j + startIndex] = (sourceBuffer[j] > 1) ? _dTypeMultiplier() : sourceBuffer[j] * _dTypeMultiplier();
		}
		
		return nSamplesToCopy * sizeof(T) + _getHeaderSize();
	}
	
public:
	
	TypedUploadBuffer(std::function<void(T*, size_t)> submitFunc, std::function<void(int, int, int)> onReadyCb, int sampleR)
	: UploadBuffer(onReadyCb, sampleR) {
		_submitFunc = submitFunc;
		
		_srcData.end_of_input = 0;
		_srcData.data_out = _resampleOut;
	}
	
	~TypedUploadBuffer() {
		src_delete(_src);
	}
	
	int bitDepth() {
		return sizeof(T) * 8;
	}
	
	int outputSampleRate() {
		return _outputSampleRate;
	}
	
	void setInputSampleRate(int sampleRate) {
		// the structure of this looks silly but is deliberate to account for race conditions
		if (inputSampleRate != sampleRate) {
			inputSampleRate = sampleRate;
			_updateRequired = true;
		}
		
		inputSampleRate = sampleRate;
	}
	
	void processBlock(iplug::sample** inputs, int nFrames, int nChans) {
		// audio settings change, init/reinit sample rate converter
		if (nChans != nChannels || _updateRequired) _updateInternals(nChans);
		
		_bufferSwapMtx.lock();
		int *relevantIndex = (_swap) ? &_swapBufferIndex2 : &_swapBufferIndex1;
		bool *isSilent = (_swap) ? &_swapBuffer2Silent : &_swapBuffer1Silent;
		
		// if we're about to go outside the swap buffers, reset their index to 0
		if (*relevantIndex >= BUFFER_SIZE - (nFrames * nChans)) *relevantIndex = 0;
		
		int index = *relevantIndex;
		*relevantIndex += (nFrames * nChans);
		
		float *buffer = (_swap) ? _swapBuffer2 : _swapBuffer1;
		_bufferSwapMtx.unlock();
		
		// just copy input to the upload buffer
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				if (inputs[c][s] != 0) {
					*isSilent = false;
				}

				buffer[index] = inputs[c][s] > 1 ? 1 : inputs[c][s];
				(index)++;
			}
		}
	}
	
	void upload() {
		// Do all race-condition-vulnerable reads/writes
		_bufferSwapMtx.lock();
		_swap = !_swap;
		
		int   *relevantIndex = (_swap) ? &_swapBufferIndex1  : &_swapBufferIndex2;
		bool  *isSilent      = (_swap) ? &_swapBuffer1Silent : &_swapBuffer2Silent;
		float *buffer        = (_swap) ? _swapBuffer1        : _swapBuffer2;
		int nSamples = *relevantIndex;
		*relevantIndex = 0;
		
		// if the buffer is all zeroes, or nothing was written, don't bother resampling and uploading
		if (nSamples == 0 || *isSilent == true) {
			_bufferSwapMtx.unlock();
			return;
		}
		
		*isSilent = true;
		
		_bufferSwapMtx.unlock();
		
		_srcMtx.lock();
		int nOutputSamples = (_doResample) ? resampleData(nSamples, buffer) : nSamples;
		float *buffToCopy  = (_doResample) ? _resampleOut : buffer;
		int dataToTransmit = copyToUploadBuffer(_getBodyIndex(), nOutputSamples, buffToCopy);
		_srcMtx.unlock();

		// add headers
		double* timestampPtr = (double*) &_uploadBuffer[0];
		double* dtypePtr = (double*) &_uploadBuffer[_getDTypeIndex()];
		*timestampPtr = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		*dtypePtr = _getDTypeBufferVal();

		_submitFunc(_uploadBuffer, dataToTransmit);
	}
};


