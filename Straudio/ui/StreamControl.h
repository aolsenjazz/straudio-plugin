#include <iostream>
#include <utility>
#include <math.h>


TcpStreamClient client;

class StreamControl {

public:
	 StreamControl(iplug::igraphics::IGraphics* pGraphics,) {
		 
		 
		 const iplug::igraphics::IRECT root = pGraphics->GetBounds();
		 const iplug::igraphics::IRECT rect = root.GetFromBRHC(250, 100);
		 
		 pGraphics->AttachControl(new iplug::igraphics::IVToggleControl(rect, kStream, "Stream", iplug::igraphics::DEFAULT_STYLE, "OFF", "ON"));
	}
	
	
	
	void flushIfNecessary() {
		
	}
	
};


