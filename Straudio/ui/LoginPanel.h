#include <iostream>
#include <utility>
#include "http_client.h"
#include "preferences.h"
#include "logger.h"

#include "rtc/rtc.hpp"

#include <thread>

using namespace std::chrono_literals;


static void handle_message(const std::string & message) {
	printf(">>> %s\n", message.c_str());
}


template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }
class LoginPanel {

public:
	LoginPanel(iplug::igraphics::IGraphics* pGraphics, std::function<void()> loginCb) {
		loginSuccessCb = loginCb;
		auto onLoginPress = std::bind(&LoginPanel::onLoginPress, this, std::placeholders::_1);
		std::string authToken = Preferences::getAuth();
		
		const iplug::igraphics::IRECT root = pGraphics->GetBounds();
		
		// Login panel
		iplug::igraphics::IRECT loginSection = root.GetFromTLHC(300, 200);
		iplug::igraphics::IRECT emailRect = loginSection.GetGridCell(0, 0, 5, 1);
		iplug::igraphics::IRECT passwordRect = loginSection.GetGridCell(1, 0, 5, 1);
		iplug::igraphics::IRECT loginSubmitRect = loginSection.GetGridCell(2, 0, 5, 1);
		iplug::igraphics::IRECT errorFieldRect = loginSection.GetGridCell(3, 0, 5, 1);
		iplug::igraphics::IRECT authFieldRect = loginSection.GetGridCell(4, 0, 5, 1);
		
		emailField = new iplug::igraphics::IEditableTextControl(emailRect, "aolsenjazz@gmail.com");
		passwordField = new iplug::igraphics::IEditableTextControl(passwordRect, "TestPassword2020");
		errorField = new iplug::igraphics::ITextControl(errorFieldRect, "", iplug::igraphics::IText(12));
		authField = new iplug::igraphics::ITextControl(authFieldRect, authToken.c_str(), iplug::igraphics::IText(12));
		
		iplug::igraphics::IControl* loginSubmit = new iplug::igraphics::IVButtonControl(loginSubmitRect, onLoginPress);

		pGraphics->AttachControl(emailField);
		pGraphics->AttachControl(passwordField);
		pGraphics->AttachControl(loginSubmit);
		pGraphics->AttachControl(authField);
		pGraphics->AttachControl(errorField);
	}
	
private:
	iplug::igraphics::ITextControl* emailField;
	iplug::igraphics::ITextControl* passwordField;
	iplug::igraphics::ITextControl* authField;
	iplug::igraphics::ITextControl* errorField;
	
	
	
	void onLoginPress(iplug::igraphics::IControl* pCaller) {
//		std::string email = this->emailField->GetStr();
//		std::string password = this->passwordField->GetStr();
//
//		std::pair<bool, std::string> successAndText = HttpClient::login(email, password);
//
//		if (successAndText.first) {
//			Preferences::setAuth(successAndText.second);
//			this->errorField->SetStr("Success!");
//			this->authField->SetStr(successAndText.second.c_str());
//			this->loginSuccessCb();
//		} else {
//			this->errorField->SetStr(successAndText.second.c_str());
//		}
//
//		this->loginSuccessCb();
		
//		rtc::Configuration config;
//		config.iceServers.emplace_back("stun:stun4.l.google.com:19302");

		
//		auto ws = std::make_shared<rtc::WebSocket>();
//		
//		ws->onOpen([]() { std::cout << "websocket connected, signaling ready" << std::endl; });
//
//		ws->onClosed([]() { std::cout << "websocket closed" << std::endl; });
//
//		ws->onError([](const std::string &error) { std::cout << "websocket failed: " << error << std::endl; });
//
//		ws->onMessage([&](const std::variant<rtc::binary, std::string> &data) {
//			std::cout << "message\n";
//		});
//		
//		const std::string url = "ws://192.168.86.241:4444/";
//		ws->open(url);
//		
//		std::shared_ptr<rtc::PeerConnection> pc;
//		
//		std::cout << "waiting for signaling to be connected..." << std::endl;
//		while (!ws->isOpen()) {
//			if (ws->isClosed())
//				std::cout << "closed???? \n";
//			std::this_thread::sleep_for(100ms);
//			std::cout << "not done\n";
//		}
		
	}
	
	std::function<void()> loginSuccessCb;
};
