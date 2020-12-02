#pragma once

class Resampler {
	
private:
	
	int _inputSampleRate;
	int _outputSampleRate;
	int _srcQuality;
	int _nChannels;
	
	/**
	 The struct passed to src_process(...)
	 */
	SRC_DATA _srcData;
	
	/**
	 Sample rate converter
	 http://www.mega-nerd.com/SRC/api.html
	 */
	SRC_STATE *_src = nullptr;
	
public:
	
	Resampler() {
		_srcData.end_of_input = 0;
	}
	
	~Resampler() {
		src_delete(_src);
	}
	
	// Should be called whenever the quality, sample rates, or nChannels changes
	void update(int inSr, int outSr, int nChans, int quality) {
		// delete the old src if exists, init new
		if (_src != nullptr) src_delete(_src);
		
		// initialize the sample rate converter
		int* error = new int;
		_src = src_new(quality, nChans, error);
		_srcData.src_ratio = outSr / (double) inSr;
		
		if (*error != 0) {
			PLOG_ERROR << *error << std::endl;
		}
		
		delete error;
		
		_inputSampleRate = inSr;
		_outputSampleRate = outSr;
		_nChannels = nChans;
		_srcQuality = quality;
	}
	
	// Resamples audio data of length readLength, writing it to write buffer. Returns the amount of data written
	int resample(float* writeBuffer, float* readBuffer, int readLength) {
		if (_inputSampleRate == _outputSampleRate) {
			// input and output sample rates are the same. just copy the buffer
			for (int i = 0; i < readLength; i++) {
				writeBuffer[i] = readBuffer[i];
			}
			
			return readLength;
		} else {
			// input and output sample rates differ; convert
			_srcData.data_out = writeBuffer;
			_srcData.data_in = readBuffer;
			_srcData.input_frames = readLength / _nChannels;
			_srcData.output_frames = readLength / _nChannels;
		
			int processError = src_process(_src, &_srcData);
			if (processError != 0) {
				PLOG_ERROR << processError << std::endl; // this should never happen
			}
			
			return _srcData.output_frames_gen * _nChannels;
		}
	}
};


