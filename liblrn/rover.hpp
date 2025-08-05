#pragma once

#include <optional>
#include <thread>
#include <regex>
#include <atomic>
#include <array>
#include <liblrn/rover-config.hpp>

#include <boost/asio.hpp>
#include <boost/asio/serial_port_base.hpp>
#include <boost/system/detail/error_code.hpp>
#include <zmq.hpp>

namespace lrn {

struct Vec2 {
    double x, y;
};

struct SensorReading {
    std::array<std::optional<double>, 3> ir;
    std::array<std::optional<double>, 3> acc;
    std::array<std::optional<double>, 3> gyro;
    std::optional<double> ultrasonic;
};

struct RobotState {
    Vec2 pos;
    double theta;
};

enum DriveState {
    Idle = 0,
    Ready = 1,
};

constexpr std::size_t rover_packet_max = 256;
constexpr std::string_view data_regex = R"(<(ir|acc|gyr),([0-9]+),([\-?0-9\.,]+)>\r?\n?)";

constexpr std::string_view motors_setup_topic = "motors-setup";
constexpr std::string_view motors_commands_topic = "motors-commands";
constexpr std::string_view tilt_motor_command_topic = "tilt-motor-command";

constexpr auto BMI323_ACCEL_SCALE_4G = 8.19;
constexpr auto BMI323_GYRO_SCALE_1000DPS = 32.768;

typedef struct{
  long int x;    // ADC value for the corresponding distance  (mm)
  long int m;    // slope between current ADC value and the next one
  long int mm;   // distance in mm from the sensor
} LUT;

typedef struct{
  float x;    // ADC value for the corresponding distance  (mm)
  float m;    // slope between current ADC value and the next one
  float mm;   // distance in mm from the sensor
} FLUT;

constexpr size_t LUT_SIZE = 11;
constexpr LUT ir_lut_values[LUT_SIZE] = {
  {108, 17653, 800},  // slope: 17.2414
  {113,  7475, 700},  // slope: 7.2988
  {127, 10129, 600},  // slope: 9.8911
  {138,  4212, 500},  // slope: 4.1126
  {162,  2425, 400},  // slope: 2.3689
  {204,  1281, 300},  // slope: 1.2508
  {283,   463, 200},  // slope: 0.4528
  {504,   193, 100},  // slope: 0.1885
  {557,   190,  90},  // slope: 0.1861
  {610,   183,  80},  // slope: 0.1792
  {667,     0,  70}
};

//// Duty-Cycle	Distância (m)	Velocidade(m/s)
//constexpr FLUT v_lut_values[] = {
//    {0.15, 	0.81,	0.162},
//    {0.2, 	1.06,	0.212},
//    {0.25, 	1.23,	0.246},
//    {0.3, 	1.39,	0.278},
//    {0.35, 	1.52,	0.304},
//    {0.4, 	1.62,	0.324},
//    {0.45, 	1.72,	0.344},
//    {0.5, 	1.8,	0.36},
//    {0.55, 	1.88,	0.376},
//    {0.6, 	1.96,	0.392},
//    {0.65, 	1.98,	0.396},
//    {0.7, 	2.03,	0.406},
//    {0.75, 	2.11,	0.422},
//    {0.8, 	2.11,	0.422},
//    {0.85, 	2.13,	0.426},
//    {0.9, 	2.17,	0.434},
//    {0.95, 	2.18,	0.436},
//    {1	, 2.22,	0.444},
//};
 
class Rover {
    // Handles to wheels and sensors would be here.
    // For simulation or hardware, abstract as needed

    RoverConfig config;

protected:
    using rover_com_buffer = std::array<uint8_t, rover_packet_max>;

public:
    Rover(boost::posix_time::milliseconds read_deadline_expiry = boost::posix_time::milliseconds{200});
    ~Rover();

    void init(RoverConfig &cfg);
    void stop();
    SensorReading readSensors();

    RobotState getState();

    Vec2 getGoalPosition();

    void drive(double v, double w);
    void tilt(double theta);

    void driveWheels(double wl, double wr);

private:
    void set_connection_parameters(const std::string& port_name, std::uint32_t baudrate, boost::asio::serial_port_base::parity::type parity, boost::asio::serial_port_base::stop_bits::type stop_bits);
    void open_connection();
    void close_connection();
    void async_read();
    void write(std::uint8_t *data, std::size_t nr_bytes_to_write);

    void handle_read_packet(boost::asio::streambuf &buffer, std::size_t bytes_transferred);
    void handle_read_error(boost::system::error_code error, std::size_t bytes_transferred);

    void start_async_read();
    void stop_async_read();

    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;

    boost::asio::cancellation_signal cancel_signal_;
    boost::asio::deadline_timer read_deadline_timer_;
    const boost::posix_time::milliseconds read_deadline_expiry_time;

    std::thread io_context_thread;

    boost::system::error_code last_error;

    boost::asio::deadline_timer read_interbytedeadline_timer_;
    boost::posix_time::milliseconds read_interbytedeadline_expiry_time {10};

    boost::asio::streambuf read_serial_buffer;

    std::string port_name_;
    boost::asio::serial_port_base::baud_rate baudrate_;
    boost::asio::serial_port_base::parity parity_;
    boost::asio::serial_port_base::stop_bits stop_bits_;

    boost::asio::serial_port serial_port;

    std::regex data_pieces_regex;

    void handle_ir_sensors_values(std::vector<double> values);
    void handle_acc_sensors_values(std::vector<double> values);
    void handle_gyro_sensors_values(std::vector<double> values);
    void handle_sensors_value(const std::string &sensor, std::vector<double> values);

    // Remote API
    std::atomic<bool> should_stop_ {false};
    bool is_remote_enabled;
    zmq::context_t context;
    zmq::socket_t sensors_publisher;
    zmq::socket_t motors_subscriber;
    std::thread motors_commands_thread;
    void motor_commands_executor();
    void handle_motors_command(const std::string& command);
    void handle_tilt_motor_command(const std::string& command);
    void handle_motors_setup(const std::string& command);

    // Last sensors readings
    SensorReading last_reading;
};
   
}