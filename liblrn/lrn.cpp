#include <liblrn/lrn.hpp>

#include <thread>
#include <functional>

#include <boost/log/trivial.hpp>
#include <fmt/format.h>

namespace lrn {
    
bool LRnManager::load_configuration(const LRnConfig &cfg) {
    config = cfg;
    return true;
}

void LRnManager::start() {
    if(running) {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Ignored start because lrn is already running");
        return;
    }

    BOOST_LOG_TRIVIAL(info) << fmt::format("Starting Navigation...");
    running = true;
    rover_executor_thread = std::thread(&RoverExecutor::run, &rover_executor, std::ref(config.rover), std::ref(config.navigator));
    BOOST_LOG_TRIVIAL(info) << fmt::format("Starting Navigation...OK");
}

void LRnManager::stop() {
    if(!running) {
        return;
    }
    BOOST_LOG_TRIVIAL(info) << fmt::format("Stopping Navigation...");
    trigger_stop();
    join();
    BOOST_LOG_TRIVIAL(info) << fmt::format("Stopping Navigation...OK");
    running = false; 
}

void LRnManager::trigger_stop() {
    rover_executor.stop();
}

void LRnManager::join() {
    rover_executor_thread.join();
}

} // namespace lrn
