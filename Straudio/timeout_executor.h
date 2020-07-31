#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

class TimeoutExecutor {
	
private:
	std::function<void()> _workFunc;
	int _delayMs;
	
	std::shared_ptr<bool> _running = std::make_shared<bool>(false);
	std::shared_ptr<bool> _interrupt = std::make_shared<bool>(false);
	
public:
	TimeoutExecutor(int delayMs, std::function<void()> workFunc) {
		_delayMs = delayMs;
		_workFunc = workFunc;
	}
	
	bool start() {
		if (*_running) {
			return false;
		}
		*_running = true;
		
		std::thread([&]() {
			auto _int = _interrupt;
			auto isRunning = _running;
			auto wFunc = _workFunc;
			
			std::this_thread::sleep_for(std::chrono::milliseconds(_delayMs));
			
			if (*_int == true) {
				PLOG_DEBUG << "aborting timeout func";
				*isRunning = false;
			} else {
				PLOG_DEBUG << "executing timeout func";
				wFunc();
			}
			
			*isRunning = false;
		}).detach();
		
		return true;
	}
	
	void interrupt() {
		*_interrupt = true;
	}
};
