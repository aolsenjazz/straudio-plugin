class RingBuffer {

private:
	static constexpr int BUFFER_LENGTH = 32768;
	
	std::mutex _mtx;
	int _writePos = 0;
	int _readPos = 0;
	float _data[BUFFER_LENGTH];
	
public:
	bool hasDataAvailable() {
		return _writePos != _readPos;
	}
	
	void resetReadPosition() {
		_readPos = _writePos;
	}
	
	// TODO: might want to guard this with mutex?
	int dataAvailable() {
		if (_readPos == _writePos) return 0;
		
		return (_readPos < _writePos) ? _writePos - _readPos : BUFFER_LENGTH - _readPos + _writePos;
	}
	
	void read(float* writeBuffer, int numSamples) {
		int readPos = _readPos;
		
		for (int i = 0; i < numSamples; i++) {
			if (readPos == BUFFER_LENGTH) readPos = 0;
			
			writeBuffer[i] = _data[readPos];
			readPos++;
		}
		
		_readPos = readPos;
	}
	
	void write(double** inputs, int nFrames, int nChans) {
		int newWritePos = _writePos;
		
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				if (newWritePos == BUFFER_LENGTH) newWritePos = 0;
				
				_data[newWritePos++] = inputs[c][s] > 1 ? 1 : inputs[c][s];
			}
		}
		
		_writePos = newWritePos;
	}
};
