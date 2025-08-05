#pragma once

#include <thread>

#include <liblrn/rover-executor.hpp>
#include <liblrn/lrn-config.hpp>

namespace lrn {

class LRnManager
{
    std::thread rover_executor_thread;
    RoverExecutor rover_executor;
   
	bool running = false;

	LRnConfig config;

	void trigger_stop();
	void join();

public:
	/**
	* @brief 
	*/
	LRnManager() = default;
	bool load_configuration(const LRnConfig &cfg);

	void start();
	void stop();
};

} // namespace lrn