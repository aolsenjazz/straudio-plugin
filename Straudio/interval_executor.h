#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

class IntervalExecutor {
	
private:
	std::function<void()> _workFunc;
	std::function<bool()> _runWhileFunc;
	
	int _intervalMs;
	bool _stop = false;
	bool _running = false;
	
public:
	IntervalExecutor(int intervalMs,
					 std::function<void()> workFunc,
					 std::function<bool()> runWhileFunc) {
		_intervalMs = intervalMs;
		_workFunc = workFunc;
		_runWhileFunc = runWhileFunc;
	}
	
	bool start() {
		_stop = false;
		if (_running) {
			return false;
		}
		_running = true;
		
		std::thread([&]() {
			while (_runWhileFunc()) {
				if (_stop) {
					_running = false;
					_stop = false;
					PLOG_DEBUG << "aborting work function";
					break;
				}
				
				PLOG_DEBUG << "executing work function";
				_workFunc();
				std::this_thread::sleep_for(std::chrono::milliseconds(_intervalMs));
			}
			
			_running = false;
		}).detach();
		
		return true;
	}
	
	void stop() {
		_stop = true;
	}
};
