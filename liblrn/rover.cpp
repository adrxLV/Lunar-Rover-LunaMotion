#include <liblrn/rover.hpp>

#include <istream>
#include <regex>
#include <numbers>

#include <boost/log/trivial.hpp>
#include <fmt/format.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace lrn {

Rover::Rover(const boost::posix_time::milliseconds read_deadline_expiry)
    : work_guard(boost::asio::make_work_guard(io_context))
    , read_deadline_timer_(io_context)
    , read_deadline_expiry_time(read_deadline_expiry)
    , read_interbytedeadline_timer_(io_context)
    , serial_port(io_context)
    , is_remote_enabled(false)
{
    io_context_thread = std::thread([this]() { io_context.run(); });
    data_pieces_regex = std::regex(lrn::data_regex.data());
}

Rover::~Rover()
{
    // This will cause the io_context run() call to return as soon as possible, abandoning unfinished operations and without permitting ready handlers to be dispatched. 
    io_context.stop();

    // alternative:
    // work.reset(); // Allow run() to exit.

    io_context_thread.join();

    if(motors_commands_thread.joinable()) {
        motors_commands_thread.join();
    }
}

void Rover::start_async_read() {
    async_read();
}

void Rover::stop_async_read() {
    cancel_signal_.emit(boost::asio::cancellation_type::partial);
}

void Rover::init(RoverConfig &cfg) {
    config = cfg;
    // Connect to/initialize sensors/actuators
    // Obtain handles

    std::string name = config.platform.port;
#ifdef _WIN32
    if (name.starts_with("COM")) {
        name = "\\\\.\\" + name;
    }
#else
    if (!name.starts_with("/")) {
        name.insert(0, "/dev/");
    }

#endif // WIN32

    is_remote_enabled = config.remote.has_value() && config.remote.value().enabled;
    if(is_remote_enabled) {
        // initialize the zmq context with a single IO thread
        context = zmq::context_t{ 1 };

        sensors_publisher = zmq::socket_t{ context, zmq::socket_type::pub };
        motors_subscriber = zmq::socket_t{ context, zmq::socket_type::sub };
    
        sensors_publisher.bind(fmt::format("tcp://*:{}", config.remote.value().port));
        motors_subscriber.bind(fmt::format("tcp://*:{}", config.remote.value().port+1));

        motors_subscriber.set(zmq::sockopt::subscribe, motors_setup_topic);
        motors_subscriber.set(zmq::sockopt::subscribe, motors_commands_topic);
        motors_subscriber.set(zmq::sockopt::subscribe, tilt_motor_command_topic);

        motors_commands_thread = std::thread([this]() { motor_commands_executor(); });
    }
    
    
    set_connection_parameters(name, 115200, boost::asio::serial_port_base::parity::none, boost::asio::serial_port_base::stop_bits::one);
    open_connection();
    start_async_read();
}

SensorReading Rover::readSensors() {
    return last_reading;
}

RobotState Rover::getState() {
    // Retrieve robot pose from odometry/state estimator
    RobotState state{/*pos=*/{0, 0}, /*theta=*/0};
    return state;
}

Vec2 Rover::getGoalPosition() {
    // Retrieve/set goal
    return {1, 1};
}

void Rover::drive(double v, double w) {
    // Transform to wheel velocities
    double vr = v + (config.wheelbase / 2.0) * w;
    double vl = v - (config.wheelbase / 2.0) * w;

    double wr = vr / config.wheel_radius;
    double wl = vl / config.wheel_radius;

    driveWheels(wl, wr);
}

double computeDutyFromOmega(double omega) {
    if(std::abs(omega) < 4.32) {
        return 0.0;
    }
    if(omega > 11.84) {
        return 255.0;
    }
    
    double result = 0.0443*std::pow(omega, 5.0) - 1.6146*std::pow(omega, 4.0) + 23.227*std::pow(omega, 3.0) - 162.59*std::pow(omega, 2.0) + 561.92*omega - 731.89;
    return result;
}


void Rover::driveWheels(double wl, double wr) {
    double ddl = computeDutyFromOmega(std::abs(wl));
    double ddr = computeDutyFromOmega(std::abs(wr));
    if(wl < 0.0) {
        ddl *= -1.0;
    }
    if(wr < 0.0) {
        ddr *= -1.0;
    }
    const auto dl = std::clamp(static_cast<int>(ddl), -255, 255);
    const auto dr =  std::clamp(static_cast<int>(ddr), -255, 255);

    std::string cmd = fmt::format("<ws,2,{},{}>\r\n", dl, dr);
    write(reinterpret_cast<uint8_t*>(cmd.data()), cmd.length());    
}

void Rover::tilt(double alpha) {
    double deg = std::clamp(alpha*180.0/std::numbers::pi, 15.0, 100.0);
    std::string cmd = fmt::format("<ss,1,{}>\r\n", static_cast<int>(deg));
    write(reinterpret_cast<uint8_t*>(cmd.data()), cmd.length());    
}

/* Finds in which interval the digital value belongs to
*   If it is lower than the starting value, -1 is returned
*   If is higher than the last value, the last value is returned
*   Else, returns the lower value of the interval to which it belongs to
*/
int find_interval(long int target) {
     //menor que o primeiro
     if (target < ir_lut_values[0].x)
         return -1; // Fora da tabela (abaixo)
     // maior que o último
     if (target >= ir_lut_values[LUT_SIZE-1].x)
         return LUT_SIZE-1;
     // Procura o long intervalo correto
     for (size_t i = 0; i < LUT_SIZE - 1; i++) {
         if (target >= ir_lut_values[i].x && target < ir_lut_values[i+1].x)
             return static_cast<int>(i);
     }
     return -1;
 }

 double computeDistance(double x) {
    return -2.1587 * x * x + 3.9694 * x + 0.3499;
}

void Rover::handle_ir_sensors_values(std::vector<double> values) {

    if(values.size() != last_reading.ir.size()) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("[rover]: IR expected {} values, but got {}", last_reading.ir.size(), values.size());
        return;
    }

    /*
     * (y - y0)/(x - x0) = m
     */

    for(size_t i = 0; i < 3; i++) {
        //if(values[i] >= 108.0) {
        //    last_reading.ir[i] = computeDistance(values[i]);
        //} else {
        //    last_reading.ir[i] = 0.8;
        //}

        long int item = find_interval(values[i]);
        if(item != -1) {
            long int dx = values[i] - ir_lut_values[item].x;
            long int dy = -ir_lut_values[item].m * dx;
            double dist = (ir_lut_values[item].mm + (dy>>10)) / 1000.0;
            last_reading.ir[i] = dist;
        } else {
            last_reading.ir[i] = 0.8;
        }
        
    }
    BOOST_LOG_TRIVIAL(warning) << fmt::format("[IR]: {}, {}, {}", last_reading.ir[0].value(), last_reading.ir[1].value(), last_reading.ir[2].value());
}

void Rover::handle_acc_sensors_values(std::vector<double> values) {

    if(values.size() != last_reading.acc.size()) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("[rover]: ACC expected {} values, but got {}", last_reading.acc.size(), values.size());
        return;
    }

    const double acc_x_g = values[0] / BMI323_ACCEL_SCALE_4G / 1000.0;
    const double acc_y_g = values[1] / BMI323_ACCEL_SCALE_4G / 1000.0;
    const double acc_z_g = values[2] / BMI323_ACCEL_SCALE_4G / 1000.0;

    last_reading.acc[0] = acc_x_g;
    last_reading.acc[1] = acc_y_g;
    last_reading.acc[2] = acc_z_g;    
}

void Rover::handle_gyro_sensors_values(std::vector<double> values) {

    if(values.size() != last_reading.gyro.size()) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("[rover]: GYRO expected {} values, but got {}", last_reading.gyro.size(), values.size());
        return;
    }

    double gyr_x_dps = values[0]/ BMI323_GYRO_SCALE_1000DPS;
    double gyr_y_dps = values[1]/ BMI323_GYRO_SCALE_1000DPS;
    double gyr_z_dps = values[2]/ BMI323_GYRO_SCALE_1000DPS;

    last_reading.gyro[0] = gyr_x_dps;
    last_reading.gyro[1] = gyr_y_dps;
    last_reading.gyro[2] = gyr_z_dps;
}

void Rover::handle_sensors_value(const std::string &sensor, std::vector<double> values) {

    if(sensor == "ir"){
        handle_ir_sensors_values(std::move(values));
    } else if(sensor == "acc"){
        handle_acc_sensors_values(std::move(values));
    } else if(sensor == "gyr"){
        handle_gyro_sensors_values(std::move(values));
    } else {
        BOOST_LOG_TRIVIAL(error) << fmt::format("[rover]: Unknown sensor '{}'", sensor);
    }
}

void Rover::handle_read_packet(boost::asio::streambuf &buffer, std::size_t /*bytes_transferred*/) {

    std::istream is(&buffer);
    std::string line;
    std::getline(is, line);
    
    std::sregex_iterator first_match = std::sregex_iterator(line.begin(), line.end(), data_pieces_regex);
    std::sregex_iterator last_match;

    
    auto current_match = first_match;
    bool has_readings = current_match != last_match;
    while(current_match != last_match) {
        std::smatch match = *current_match;
        // match[0] = whole match
        // match[1] = ir/acc/gyro
        // match[2] = count
        // match[3] = value (possibly comma-separated numbers)

        std::string sensor = match[1];
        int count = std::stoi(match[2]);
        std::string values_str = match[3];

        // Parse numbers after count into vector
        std::vector<double> values;
        std::stringstream ss(values_str);
        std::string item;
        while (std::getline(ss, item, ',')) {
            values.push_back(std::stod(item));
        }

        // Check if count matches number of parsed numbers
        if (values.size() == static_cast<std::size_t>(count)) {
            handle_sensors_value(sensor, std::move(values));
        } else {
            BOOST_LOG_TRIVIAL(warning) << "\nCount does NOT match number of values!\n";
        }
        ++current_match;
    }

    if(is_remote_enabled && has_readings) {
        json j;

        const auto sensorsReadings = readSensors();
        j = {
            {"ir", {sensorsReadings.ir[0], sensorsReadings.ir[1], sensorsReadings.ir[2]}},
            {"acc", {sensorsReadings.acc[0], sensorsReadings.acc[1], sensorsReadings.acc[2]}},
            {"gyro", {sensorsReadings.gyro[0], sensorsReadings.gyro[1], sensorsReadings.gyro[2]}}
        };

        std::string str = j.dump();
        zmq::message_t sensors_data(str);

        sensors_publisher.send(zmq::str_buffer("lunar-rover-sensors"), zmq::send_flags::sndmore);
        
        sensors_publisher.send(sensors_data, zmq::send_flags::none);
    }    
}

void Rover::handle_motors_command(const std::string& command) {
    json j = json::parse(command);

    if(!j.contains("v") || !j.contains("w")) {
        BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid command. Expected v and w";
        return;
    }

    const auto v_ = j["v"];
    const auto w_ = j["w"];

    if(!v_.is_number() || !w_.is_number() ) {
        BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid command. Expected v and w as numbers";
        return;
    }

    double v = v_.get<double>();
    double w = w_.get<double>();

    BOOST_LOG_TRIVIAL(trace) << fmt::format("Motor commands: v={}, w={}", v, w);

    drive(v, w);
}

void Rover::handle_tilt_motor_command(const std::string& command) {
    json j = json::parse(command);

    if(!j.contains("alpha")) {
        BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid command. Expected alpha";
        return;
    }

    const auto alpha_ = j["alpha"];

    if(!alpha_.is_number()) {
        BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid command. Expected alpha as number";
        return;
    }

    double alpha = alpha_.get<double>();

    BOOST_LOG_TRIVIAL(trace) << fmt::format("Tilt Motor command: alpha={}", alpha);

    tilt(alpha);
}

void Rover::handle_motors_setup(const std::string& command) {
    json j = json::parse(command);

    if(!j.contains("state")) {
        BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid setup command. Expected state";
        return;
    }

    const auto state_ = j["state"];
    bool ready = false;
    if(state_.is_boolean()) {
        ready = state_.get<bool>();
    } else if(state_.is_string()) {
        ready = state_.get<std::string>() == "run";
    }

    BOOST_LOG_TRIVIAL(trace) << fmt::format("Motor setup: run={}", ready);

    std::string cmd = fmt::format("<{},{}>\r\n", ready ? "start" : "stop", ready ? 100 : 0);
    write(reinterpret_cast<uint8_t*>(cmd.data()), cmd.length());
}

void Rover::motor_commands_executor() {
    while(!should_stop_) {
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result = zmq::recv_multipart(motors_subscriber, std::back_inserter(recv_msgs));
        if(!result) {
            BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Failed to receive data. Ending.";
            break;
        }

        if(*result != 2) {
            BOOST_LOG_TRIVIAL(error) << "[motor_commands_executor] Invalid data packet received. Dropping it.";
            continue;
        }
        const auto topic = recv_msgs[0].to_string_view();
        if(topic == motors_commands_topic) {
            handle_motors_command(recv_msgs[1].to_string());
        }else if(topic == tilt_motor_command_topic) {
            handle_tilt_motor_command(recv_msgs[1].to_string());
        } else if(topic == motors_setup_topic) {
            handle_motors_setup(recv_msgs[1].to_string());
        } else {
            BOOST_LOG_TRIVIAL(warning) << fmt::format("[motor_commands_executor] Unknown topic: {}", topic);
            continue;
        }
    }
}

void Rover::handle_read_error(boost::system::error_code error, std::size_t /*bytes_transferred*/) {
    BOOST_LOG_TRIVIAL(fatal) << fmt::format("[connection.async_read] Error reading: {}", error.what());
}

void Rover::set_connection_parameters(const std::string& port_name, std::uint32_t baudrate, boost::asio::serial_port_base::parity::type parity, boost::asio::serial_port_base::stop_bits::type stop_bits)
{
    port_name_ = port_name;
    baudrate_ = boost::asio::serial_port_base::baud_rate(baudrate);
    parity_ = boost::asio::serial_port_base::parity(parity);
    stop_bits_ = boost::asio::serial_port_base::stop_bits(stop_bits);
}
void Rover::open_connection()
{    
    constexpr auto character_size = 8U;
    serial_port.open(port_name_);

    serial_port.set_option(baudrate_);
    serial_port.set_option(boost::asio::serial_port_base::character_size(character_size));
    serial_port.set_option(parity_);
    serial_port.set_option(stop_bits_);
    serial_port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
}

void Rover::close_connection()
{
    if(serial_port.is_open()) {
        serial_port.close();
    }
}

void Rover::async_read()
{
    auto read_completion_handler = [this](
        // Result of operation.
        boost::system::error_code error,
        // Number of bytes copied into the buffers. If an error
        // occurred, this will be the number of bytes successfully
        // transferred prior to the error.
        std::size_t bytes_transferred
    ) {
        // completion handler
        if(error) {
            if(error != boost::asio::error::operation_aborted) {
                handle_read_error(error, bytes_transferred);
                BOOST_LOG_TRIVIAL(warning) << fmt::format("[serial-connection.async_read] Completion handler: {}", error.what());
            } else {
                BOOST_LOG_TRIVIAL(trace) << fmt::format("[serial-connection.async_read] read_completion_handler: Operation aborted");
            }
        } else {
            handle_read_packet(read_serial_buffer, bytes_transferred);
        }

        if(!error || error == boost::asio::error::operation_aborted) {
            async_read();
        }
    };

    boost::asio::async_read_until(
        serial_port,
        read_serial_buffer,
        "\r\n",
        read_completion_handler
    );
}

void Rover::write(uint8_t *data, std::size_t nr_bytes_to_write)
{
    boost::system::error_code erc;
    size_t bytes_transferred = boost::asio::write(serial_port, boost::asio::buffer(data, nr_bytes_to_write), erc);
    (void)bytes_transferred;
    if (erc) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("[write] Failed to send data to rover: {}", erc.what());
    }
}

void Rover::stop() {
    should_stop_ = true;
}
   
}