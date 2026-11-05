#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "trajectory_msgs/msg/joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

using namespace std::chrono_literals;

class Controller : public rclcpp :: Node
{
    public:
    Controller() : Node("controller")
    {
    traj_pub_ = this->create_publisher<trajectory_msgs::msg::JointTrajectory>(
        "/arm_controller/joint_trajectory", 10
    );
    timer_ = create_wall_timer(10s, [this]() { control(); });
    }
private:
    void control()
    {
        if (traj_pub_->get_subscription_count() == 0) {
            RCLCPP_WARN(get_logger(), "Waiting for arm_controller...");
            return;                      
        }
        auto traj = trajectory_msgs::msg::JointTrajectory();
        traj.joint_names = {"shoulder_joint", "elbow_joint", "wrist_joint"};
        trajectory_msgs::msg::JointTrajectoryPoint p1 , p2 , p3;
        p1.positions = {0.8 , 0.6, 0.3 };
        p1.time_from_start = rclcpp::Duration::from_seconds(3.0);

        p2.positions = {0 , 0, 0 };
        p2.time_from_start = rclcpp::Duration::from_seconds(6.0);

        p3.positions = {-0.8 , -0.6, -0.3 };
        p3.time_from_start = rclcpp::Duration::from_seconds(9.0);

        traj.points = {p1, p2 , p3};
        traj_pub_->publish(traj);
        RCLCPP_INFO(this->get_logger(), "trajectory sent");
    }
     rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr traj_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Controller>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}