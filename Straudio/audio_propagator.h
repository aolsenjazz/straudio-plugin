#include <iostream>
#include <utility>

class AudioPropagator {

public:
	static void propagateAudio(double** inputs, double** outputs, int nFrames, int nInChans, int nOutChans) {
		if (nInChans == 1) {
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nOutChans; c++) {
					outputs[c][s] = inputs[0][s];
				}
			}
		} else {
			for (int s = 0; s < nFrames; s++) {
				for (int c = 0; c < nOutChans; c++) {
					outputs[c][s] = inputs[c][s];
				}
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


