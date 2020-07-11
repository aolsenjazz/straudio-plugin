#include <iostream>
#include <random>
#include <thread>

class PcmGenerator {
	
public:
	template<typename Func>
	static void generate(Func cb, int sampleRate=44100, int nChans=2, int blockSize=512, int iterations=10) {
		// calculate ms interval to emit
		int intervalMs = (float) blockSize / (float) sampleRate * (float) 1000;
		std::cout << intervalMs;
		for (int i = 0; i < iterations; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
			
			double* data[nChans];
			for (int i = 0; i < nChans; i++) {
				double channel[blockSize];
				for (int j = 0; j < blockSize; j++) {
					channel[i] = static_cast<double> (rand()) / static_cast<double> (RAND_MAX);
				}
				data[i] = channel;
			}
			
			cb(data);
		}
	}
};
