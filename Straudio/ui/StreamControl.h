//#include <iostream>
//#include <utility>
//#include <math.h>
//#include "../stream/TcpStreamClient.h"
//
//float* streamBuffer;
//int bufferSize;
//int uploadEveryNBatches;
//int batchCounter = 0;
//TcpStreamClient client;
//
//class StreamControl {
//
//public:
//	 StreamControl(iplug::igraphics::IGraphics* pGraphics,int nChannels, int batchSize, int sampleRate, int intervalMs=1000) {
//		 uploadEveryNBatches = floor(intervalMs / 1000 * sampleRate / batchSize);
//		 bufferSize = uploadEveryNBatches * batchSize * nChannels;
//		 streamBuffer = new float[bufferSize];
//		 
//		 const iplug::igraphics::IRECT root = pGraphics->GetBounds();
//		 const iplug::igraphics::IRECT rect = root.GetFromBRHC(250, 100);
//		 
//		 pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(rect, kStream, "Stream", iplug::igraphics::DEFAULT_STYLE, "OFF", "ON"));
//	}
//	
//	void onDataAvailable(iplug::sample** inputs, int nFrames, int nChans) {
//		int bufferPos = batchCounter * nFrames * nChans;
//		for (int s = 0; s < nFrames; s++) {
//			for (int c = 0; c < nChans; c++) {
//				streamBuffer[bufferPos] = inputs[c][s];
//				bufferPos++;
//			}
//		}
//		
//		if (batchCounter == uploadEveryNBatches - 1) {
////			if (fork()==0) {
//				// Child process
//				client.stream(streamBuffer, bufferSize);
//				
////				exit(-1);
////			} else {
//				// Main process
////			}
//			
//			batchCounter = 0;
//		} else {
//			batchCounter += 1;
//		}
//	}
//	
//	void flushIfNecessary() {
//		
//	}
//	
//};
//
//
