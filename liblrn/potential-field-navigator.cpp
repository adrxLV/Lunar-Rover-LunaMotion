#include <liblrn/potential-field-navigator.hpp>
#include <cmath>
#include <numbers>
#include <liblrn/rover.hpp>
#include <liblrn/potential-field-navigator-config.hpp>

namespace lrn {
// Utility function for distance
double dist(const Vec2& a, const Vec2& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

PotentialFieldNavigator::PotentialFieldNavigator(Rover& rover, PotentialFieldNavigatorConfig &cfg)
    : rover_(rover)
    , config(cfg)
    {}

void PotentialFieldNavigator::step() {
    using namespace std::numbers;

    const double K_ATT = config.K_ATT;
    const double K_REP = config.K_REP;
    const double RHO_0 = config.RHO_0;
    const double K_THETA = config.K_THETA;
    const double W_MAX = config.W_MAX;
    const double V_MAX = config.V_MAX;

    auto state = rover_.getState();
    Vec2 goal = rover_.getGoalPosition();
    auto sensors = rover_.readSensors();

    // Attractive force
    double dx_att = state.pos.x - goal.x;
    double dy_att = state.pos.y - goal.y;
    double fx_att = -K_ATT * dx_att;
    double fy_att = -K_ATT * dy_att;

    // Repulsive force
    double fx_rep = 0.0;
    double fy_rep = 0.0;
    double ir_sensors_delta = 30.0 * pi / 180.0;
    double ir_sensors_theta_start = 30.0 * pi / 180.0;

    for (size_t i = 0; i < 3; ++i) {
        if (sensors.ir[i]) {
            double rho = *sensors.ir[i];
            double sensor_theta = ir_sensors_theta_start - i * ir_sensors_delta;
            double obs_x = rho * std::cos(sensor_theta);
            double obs_y = rho * std::sin(sensor_theta);
            double factor = -K_REP * std::exp(-rho / RHO_0);
            if (rho > 1e-5) { // avoid division by zero
                fx_rep += factor * obs_x / rho;
                fy_rep += factor * obs_y / rho;
            }
        }
    }

    double fx = fx_att + fx_rep;
    double fy = fy_att + fy_rep;

    // Compute heading error
    double theta_d = std::atan2(fy, fx);
    double e_theta = theta_d - state.theta;
    while (e_theta > pi) e_theta -= 2 * pi;
    while (e_theta < -pi) e_theta += 2 * pi;

    // Angular control
    double omega = std::clamp(K_THETA * e_theta, -W_MAX, W_MAX);

    // Forward velocity control
    double v = V_MAX * std::cos(e_theta);
    if (v < 0) v = 0;

    if (dist(state.pos, goal) < 0.1) {
        v = 0;
        omega = 0;
    }

    //rover_.drive(v, omega);
}

} // namespace lrn
