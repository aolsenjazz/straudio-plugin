/**
 Circular buffer that allows for reasonably safe concurrent access. The number of input channels can be 0 < nInChans < nOutChans.
 The number of output chans is hardcoded to 2.
 */
class RingBuffer {

private:
	static constexpr int BUFFER_LENGTH = 32768;
	static constexpr int N_CHANNELS = 2;
	
	int _writePos = 0;
	int _readPos = 0;
	float _data[BUFFER_LENGTH];
	
public:

	// returns true if data is available to be read
	bool hasDataAvailable() {
		return _writePos != _readPos;
	}
	
	// Moves the read position up to the write position. Effectively chops off old data.
	void resetReadPosition() {
		_readPos = _writePos;
	}
	
	// Returns the amount of written data which has not yet been read
	int getNReadableSamples() {
		if (_readPos == _writePos) return 0;
		
		return (_readPos < _writePos) ? _writePos - _readPos : BUFFER_LENGTH - _readPos + _writePos;
	}
	
	// read the specified num of samples and put into the given writeBuffer. returns false if buffer is all 0
	bool read(float* writeBuffer, int numSamples) {
		int readPos = _readPos;
		bool hasSound = false;
		
		for (int i = 0; i < numSamples; i++) {
			if (readPos == BUFFER_LENGTH) readPos = 0;
			if (_data[readPos] != 0) hasSound = true;
			
			writeBuffer[i] = _data[readPos];
			readPos++;
		}
		
		_readPos = readPos;
		return hasSound;
	}
	
	// write the given number of samples where samples == nFrames * nChans
	void write(double** inputs, int nFrames, int nInChans) {
		int newWritePos = _writePos;
		
		if (nInChans == 1) {
			// copy the input channel to both outs
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < N_CHANNELS; c++) {
					if (newWritePos == BUFFER_LENGTH) newWritePos = 0;
					
					_data[newWritePos++] = inputs[c][s] > 1 ? 1 : inputs[0][s];
				}
			}
		} else {
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < N_CHANNELS; c++) {
					if (newWritePos == BUFFER_LENGTH) newWritePos = 0;
					
					_data[newWritePos++] = inputs[c][s] > 1 ? 1 : inputs[c][s];
				}
			}
		}
		
		_writePos = newWritePos;
	}
};
