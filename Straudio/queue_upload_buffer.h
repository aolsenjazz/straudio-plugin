/**
 Custom buffers for sending PCM data to each client. Reuses the same buffer _buffer<T> to avoid uncessary allocation of memory.
 Each buffer contains a header consisting of metadata related to the PCM, and a body consisting of PCM data. Each message
 is structured as such:
 
 __________________
 | 8 bytes timestamp| ms since epoch
 |_________________|
 | 8 bytes dtype        | DTYPE_INT16, DTYPE_FLOAT32, or SILENCE
 |_________________|
 | 8 bytes length       | number of samples in the body. only used to instruct clients how many samples of silence to write
 |_________________|
 |0-8192 bytes PCM|
 |_________________|
 
 */

#pragma once

#include "upload_buffer.h"
#include "resampler.h"
#include "ring_buffer.h"
#include <chrono>
#include <algorithm>

using namespace std::chrono;

template <typename T>
class TypedUploadBuffer : public UploadBuffer {
	
private:
	
	// Sent as second part of header. Informs clients of how to transform data
	static constexpr double DTYPE_INT16 = 0;
	static constexpr double DTYPE_FLOAT32 = 1;
	static constexpr double DTYPE_SILENCE = 2;
	
	// 32768 is completely arbitrary
	static constexpr int BUFFER_SIZE = 32768;

	// If the sample rate or nChans changes, tell the src it needs to update
	bool _updateRequired = false;	
	
	// True while program is alive. Tells the upload thread when to shut down
	bool _doUpload = true;
	
	int _nChannels = 0;
	int _outputSampleRate = 44100;
	int _srcQuality = SRC_SINC_BEST_QUALITY;
	
	// Locks for the audio upload thread
	std::mutex mtx;
	std::mutex shutdownMtx;
	std::condition_variable cond;
	
	// Buffers to write data into. Buffer usage follows the following sequential order:
	RingBuffer _buffer;
	float      _resampleOut[BUFFER_SIZE];
	T          _uploadBuffer[BUFFER_SIZE];
	
	// Resamples. if _updateRequired is true, updates itself
	Resampler _resampler;
	
	// Called in upload() to transfer data to the websocket service.
	std::function<void(T*, size_t)> _submitFunc;
	
	// Message index getters. Returns the element-wise position of each part of the message.
	int _getTimestampIndex()  { return 0; }
	int _getDTypeIndex()      { return _getTimestampLength(); }
	int _getDataLengthIndex() { return _getTimestampLength() + _getDTypeLength(); }
	int _getBodyIndex()       { return _getHeaderSize() / sizeof(T); }
	
	// Returns the size (in bytes) of the header
	int _getHeaderSize() { return (_getTimestampLength() + _getDTypeLength() + _getDataLengthLength()) * sizeof(T); }
	
	// Message length getters. Returns the length of each message in terms of type T
	int _getTimestampLength()  { return sizeof(double) / sizeof(T); }
	int _getDTypeLength()      { return sizeof(double) / sizeof(T); }
	int _getDataLengthLength() { return sizeof(double) / sizeof(T); }
	
	// Returns the type of T. This is the type of data transmitted to clients. Usually INT16
	int _getDTypeBufferVal() {
		if constexpr (std::is_same_v<T, short>) {
			return DTYPE_INT16;
		}
		return DTYPE_FLOAT32;
	}
	
	// If sending INT16 data, multiple native floats by SHRT_MAX
	int _dTypeMultiplier() {
		if constexpr (std::is_same_v<T, short>) {
			return SHRT_MAX;
		}
		
		return 1;
	}
	
	int _writeToUploadBuffer(int nSamples, float *buffToCopy) {
		for (int j = 0; j < nSamples; j++) {
			_uploadBuffer[j + _getBodyIndex()] = (buffToCopy[j] > 1) ? _dTypeMultiplier() : buffToCopy[j] * _dTypeMultiplier();
		}
	}
	
	void _uploadLoop() {
		while (_doUpload) {
			// wait until data becomes available
			std::unique_lock<std::mutex> lock{mtx};
			cond.wait(lock, [&] { return _buffer.hasDataAvailable() || !_doUpload; });
			if (!_doUpload) return; // if woken up because dying, don't process audio
			
			shutdownMtx.lock(); // don't shut down while we're trying to resample/upload
			
			if (_updateRequired) {
				_updateRequired = false;
				_resampler.update(getInputSampleRate(), _outputSampleRate, _nChannels, _srcQuality);
				_onReadyCb(getOutputSampleRate(), getNChannels(), getBitDepth());
				_buffer.resetReadPosition();
				shutdownMtx.unlock();
				continue;
			}
			
			// pull data from the buffer
			int nSamples = _buffer.dataAvailable();
			float data[nSamples];
			_buffer.read(data, nSamples);
			
			// resample
			int nOutputSamples = _resampler.resample(_resampleOut, data, nSamples);
			
			// TODO: if silent, handle appropriately
			_writeToUploadBuffer(nOutputSamples, _resampleOut);
			
			int dataToTransmit = nOutputSamples * sizeof(T) + _getHeaderSize();
			
			// add headers
			double* timestampPtr = (double*) &_uploadBuffer[0];
			*timestampPtr = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

			double* dtypePtr = (double*) &_uploadBuffer[_getDTypeIndex()];
			*dtypePtr = _getDTypeBufferVal();
	//		*dtypePtr = (bufferIsSilent) ? DTYPE_SILENCE : _getDTypeBufferVal();

			double* lengthPtr = (double*) &_uploadBuffer[_getDataLengthIndex()];
			*lengthPtr = nOutputSamples;

			_submitFunc(_uploadBuffer, dataToTransmit);
			
			shutdownMtx.unlock();
		}
	}
	
public:
	
	TypedUploadBuffer(std::function<void(T*, size_t)> submitFunc, std::function<void(int, int, int)> onReadyCb, int inSampleR, int outSampleR = 0, int srcQuality = 0)
	: UploadBuffer(onReadyCb, inSampleR) {
		_submitFunc = submitFunc;
		
		if (outSampleR != 0) setOutputSampleRate(outSampleR);
		if (srcQuality != 0) setSrcQuality(srcQuality);
		
		std::thread t(std::bind(&TypedUploadBuffer::_uploadLoop, this));
		t.detach();
	}
	
	~TypedUploadBuffer() {
		shutdownMtx.lock();
		_doUpload = false;
		cond.notify_all();
		shutdownMtx.unlock();
	}
	
	void processBlock(iplug::sample** inputs, int nFrames, int nChans) {
		if (_nChannels != nChans) {
			_nChannels = nChans;
			_onReadyCb(getOutputSampleRate(), _nChannels, getBitDepth());
		}
	
		_buffer.write(inputs, nFrames, nChans);
		cond.notify_all();
	}
	
	// getters
	int getBitDepth() { return sizeof(T) * 8; }
	int getOutputSampleRate() { return _outputSampleRate; }
	int getInputSampleRate() { return _inputSampleRate; }
	int getSrcQuality() { return _srcQuality; }
	int getNChannels() { return _nChannels; }
	
	// setters
	void setInputSampleRate(int sampleRate) {
		if (_inputSampleRate != sampleRate) {
			_inputSampleRate = sampleRate;
			_updateRequired = true;
		}
	}
	
	void setOutputSampleRate(int sampleRate) {
		if (_outputSampleRate != sampleRate) {
			_outputSampleRate = sampleRate;
			_updateRequired = true;
		}
	}
	
	void setSrcQuality(int quality) {
		if (_srcQuality != quality) {
			_srcQuality = quality;
			_updateRequired = true;
		}
	}
};


