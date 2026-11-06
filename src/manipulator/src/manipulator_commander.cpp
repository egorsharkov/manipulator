#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

using namespace std::chrono_literals;

class Controller : public rclcpp :: Node
{
    public:
    using FJT = control_msgs::action::FollowJointTrajectory;
    using GoalHandleFJT = rclcpp_action::ClientGoalHandle<FJT>;

    Controller() : Node("controller")
    {
    client = rclcpp_action::create_client<FJT>(
        this, "/arm_controller/follow_joint_trajectory"
    );
    timer_ = create_wall_timer(1s, [this]() { send_goal(); });
    RCLCPP_INFO(this->get_logger(), "Manipulator commander started.");
    }
private:
    void send_goal()
    {
        if (!client ->wait_for_action_server(5s)) {
            RCLCPP_ERROR(this->get_logger(), "Action server not available");
            return;                      
        }
        auto goal = FJT::Goal();
        goal.trajectory.joint_names = {"rotate_base_joint", "shoulder_joint", "elbow_joint", "wrist_joint"};
        trajectory_msgs::msg::JointTrajectoryPoint p1 , p2 , p3;
        p1.positions = {1.5, 0.8 , 0.6, 0.3 };
        p1.time_from_start = rclcpp::Duration::from_seconds(3.0);

        p2.positions = { 0 ,0 , 0, 0 };
        p2.time_from_start = rclcpp::Duration::from_seconds(6.0);

        p3.positions = {-1.5 ,-0.8 , -0.6, -0.3 };
        p3.time_from_start = rclcpp::Duration::from_seconds(9.0);

        goal.trajectory.points = {p1, p2 , p3};
        auto options = rclcpp_action::Client<FJT>::SendGoalOptions();
        


    options.goal_response_callback =
      [this](const GoalHandleFJT::SharedPtr & handle) {
        if (!handle) {
          RCLCPP_ERROR(this->get_logger(), "Goal rejected");
        } else {
          RCLCPP_INFO(this->get_logger(), "Goal accepted");
        }
      };

    options.feedback_callback =
      [this](GoalHandleFJT::SharedPtr,
             const std::shared_ptr<const FJT::Feedback> feedback) {
        RCLCPP_INFO(this->get_logger(), "Feedback: %zu joints",
                    feedback->joint_names.size());
      };

    options.result_callback =
      [this](const GoalHandleFJT::WrappedResult & result) {
        switch (result.code) {
          case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(this->get_logger(), "Done!");
            break;
          case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(this->get_logger(), "Goal aborted");
            break;
          case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(this->get_logger(), "Goal canceled");
            break;
          default:
            RCLCPP_ERROR(this->get_logger(), "Unknown result code");
            break;
        }
        rclcpp::shutdown();  // задача выполнена, завершаем узел
      };
      client->async_send_goal(goal, options);
      RCLCPP_INFO(this->get_logger(), "Goal sent.");
    }
    rclcpp_action::Client<FJT>::SharedPtr client;
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