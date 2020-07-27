#include <iostream>
#include <utility>

class AudioPropagator {

public:
	static void propagateAudio(double** inputs, double** outputs, int nFrames, int nChans) {
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				outputs[c][s] = inputs[c][s];
			}
		}
	}

	static void propagateSilence(double** outputs, int nFrames, int nChans) {
		for (int s = 0; s < nFrames; s++) {
			for (int c = 0; c < nChans; c++) {
				outputs[c][s] = 0;
			}
		}
	}
};


