#include <iostream>
#include <utility>

class MonitorControl {

public:
	MonitorControl(iplug::igraphics::IGraphics* pGraphics) {
		const iplug::igraphics::IRECT root = pGraphics->GetBounds();
		const iplug::igraphics::IRECT rect = root.GetFromBLHC(250, 100);

		pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(rect, kMonitor, "Monitor", iplug::igraphics::DEFAULT_STYLE, "OFF", "ON"));
	}
	
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


