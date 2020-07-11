class AudioInfoPanel {
	
public:
	AudioInfoPanel(iplug::igraphics::IGraphics* pGraphics) {
		const iplug::igraphics::IRECT root = pGraphics->GetBounds();

		iplug::igraphics::IRECT
			r = root.GetFromTRHC(300, 200),
			chansLabelRect = r.GetGridCell(0, 0, 5, 2),
			chansValRect = r.GetGridCell(0, 1, 5, 2),
			srLabelRect = r.GetGridCell(1, 0, 5, 2),
			srValRect = r.GetGridCell(1, 1, 5, 2),
			bsLabelRect = r.GetGridCell(2, 0, 5, 2),
			bsValRect = r.GetGridCell(2, 1, 5, 2);

		chansField = new iplug::igraphics::ITextControl(chansValRect, "", iplug::igraphics::IText(12));
		srField = new iplug::igraphics::ITextControl(srValRect, "", iplug::igraphics::IText(12));
		bsField = new iplug::igraphics::ITextControl(bsValRect, "", iplug::igraphics::IText(12));

		// Labels
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(chansLabelRect, "NChans: ", iplug::igraphics::IText(12)));
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(srLabelRect, "Sample Rate: ", iplug::igraphics::IText(12)));
		pGraphics->AttachControl(new iplug::igraphics::ITextControl(bsLabelRect, "Block size: ", iplug::igraphics::IText(12)));

		// Values
		pGraphics->AttachControl(chansField);
		pGraphics->AttachControl(srField);
		pGraphics->AttachControl(bsField);
	}
	
	void updateAudioInfo(int nChannels, int sampleRate, int blockSize) {
		chansField->SetStr(std::to_string(nChannels).c_str());
		srField->SetStr(std::to_string(sampleRate).c_str());
		bsField->SetStr(std::to_string(blockSize).c_str());
	}
	
private:
	iplug::igraphics::ITextControl* chansField;
	iplug::igraphics::ITextControl* srField;
	iplug::igraphics::ITextControl* bsField;
};

