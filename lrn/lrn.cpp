#ifndef _WIN32
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
    #include <features.h>
    #ifndef __USE_GNU
        #define __MUSL__
    #endif
    #undef _GNU_SOURCE /* don't contaminate other includes unnecessarily */
#else // _GNU_SOURCE
    #include <features.h>
    #ifndef __USE_GNU
        #define __MUSL__
    #endif
#endif // _GNU_SOURCE
#endif // _WIN32

#include <iostream>
#include <fstream>
#include <csignal>
#include <atomic>
#include <string>
#include <filesystem>

#include <git.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp> // NOLINT misc-include-cleaner
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <fmt/format.h>

#include <liblrn/lrn.hpp>

namespace po = boost::program_options;
namespace logging = boost::log;
namespace fs = std::filesystem;


static std::atomic<bool> interrupt_execution {false};
static std::mutex exit_mutex;

[[noreturn]] void exit_process() {
    const std::unique_lock lck(exit_mutex);
    std::exit(1); // NOLINT concurrency-mt-unsafe
}

void signal_handler(int signal)
{
    if(signal == SIGINT && !interrupt_execution) {
        interrupt_execution = true;
        interrupt_execution.notify_all();
    } else {
        exit_process();
    }
}

std::string get_strerror() {
    const size_t buffer_size = 1024;
    std::array<char, buffer_size> buffer{};
#ifdef _WIN32
    errno_t error;
    error = strerror_s(buffer.data(), buffer_size, errno);
    if (error != 0) {
      strerror_s(buffer.data(), buffer_size, error);
    }
    const char* message = buffer.data();
#else
// XSI/POSIX version: MUSL, or GNU with _POSIX_C_SOURCE
#if (((defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || ( defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)) && ! _GNU_SOURCE) || defined(__MUSL__)
    strerror_r(errno, buffer.data(), buffer_size);
    const char* message = buffer.data();
#else
    // GNU version
    const char* message = strerror_r(errno, buffer.data(), buffer_size);
#endif
#endif  // _WIN32
    return { message };
}

int lrn_process(const boost::json::value &json_config) {
    int result = 0;
    try {
        lrn::LRnConfig config = boost::json::value_to<lrn::LRnConfig>(json_config);
        lrn::LRnManager manager;
        manager.load_configuration(config);
        manager.start();

        interrupt_execution.wait(false);

        manager.stop();
        result = 0;

    } catch(std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Failed to parse json file: {}", e.what());
        result = 1;
    }
    catch(...) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Exception of unknown type");
        result = 1;
    }
    return result;
}

int main(int argc, char* argv[]) {
    
    interrupt_execution = false;
    fs::path default_config_path = fs::current_path() / "nav.json";
    std::string config_file = "";
    boost::log::trivial::severity_level log_severity = logging::trivial::info;
    try{
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show version" )
        ("config,c", po::value < std::string >(&config_file)->default_value(default_config_path.string()), "Configuration file path")
        ("log_severity,l", po::value < boost::log::trivial::severity_level >(&log_severity)->default_value(log_severity), "Logging severity level: trace, debug, info (default), warning, error, fatal")
        ;
        
        po::variables_map po_vm;
        po::store(po::parse_command_line(argc, argv, desc), po_vm);

        if (po_vm.count("help") > 0) {
            std::cout << desc << "\n";
            return 1;
        }
        if(po_vm.count("version") > 0) {
            std::cout << fmt::format("{}{}", git_Describe(), git_AnyUncommittedChanges() ? "-dirty" : "")  << "\n";
            return 0;
        }
        
        po::notify(po_vm);

        logging::core::get()->set_filter
        (
            logging::trivial::severity >= log_severity // NOLINT misc-include-cleaner
        );

    } catch(std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Failed to parse options: {}", e.what());
        return 1;
    }
    catch(...) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Exception of unknown type");
        return 1;
    }

    
    boost::json::value json_config;
    fs::path config_path = config_file;

    try {
        // load configuration file
        std::ifstream config_file_stream(config_path);

        if(!config_file_stream.is_open()) {
            BOOST_LOG_TRIVIAL(fatal) << fmt::format("Failed to open file {}: {}", config_path.string(), get_strerror());
            return 1;
        }

        json_config = boost::json::parse(config_file_stream);

    } catch(std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Failed to parse json file: {}", e.what());
        return 1;
    }
    catch(...) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Exception of unknown type");
        return 1;
    }

    // Install a signal handler
    if(std::signal(SIGINT, signal_handler) == SIG_ERR) {
        BOOST_LOG_TRIVIAL(fatal) << fmt::format("Failed to install INT signal: {}", get_strerror());
        return 2; 
    }
    return lrn_process(json_config);
}