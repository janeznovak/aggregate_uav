#include <cstdio>
#include <string>
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/empty.h"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int8.hpp"
#include "nav_system_interfaces/msg/goal.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "irobot_create_msgs/action/undock.hpp"
#include "irobot_create_msgs/action/dock.hpp"
#include "irobot_create_msgs/action/navigate_to_position.hpp"
#include "irobot_create_msgs/msg/kidnap_status.hpp"
#include "irobot_create_msgs/msg/ir_intensity.hpp"
#include "irobot_create_msgs/msg/ir_intensity_vector.hpp"

#include <future>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;
using UndockAction = irobot_create_msgs::action::Undock;
using DockAction = irobot_create_msgs::action::Dock;
using KidnapMsg = irobot_create_msgs::msg::KidnapStatus;
using std::placeholders::_1;

using Undock = irobot_create_msgs::action::Undock;
using GoalHandleUndock = rclcpp_action::ClientGoalHandle<Undock>;
using Dock = irobot_create_msgs::action::Dock;
using GoalHandleDock = rclcpp_action::ClientGoalHandle<Dock>;

using NavigateToPosition = irobot_create_msgs::action::NavigateToPosition;
using Goal = nav_system_interfaces::msg::Goal;
using GoalHandleNavigateToPosition = rclcpp_action::ClientGoalHandle<NavigateToPosition>;
using IrIntensityVector = irobot_create_msgs::msg::IrIntensityVector;
using Odometry = nav_msgs::msg::Odometry;
using IrIntensity = irobot_create_msgs::msg::IrIntensity;

const std::string IR_INTENSITY_FRONT_CENTER_LEFT =  "ir_intensity_front_center_left";
const std::string IR_INTENSITY_FRONT_CENTER_RIGHT = "ir_intensity_front_center_right";
const std::string IR_INTENSITY_FRONT_LEFT =         "ir_intensity_front_left";
const std::string IR_INTENSITY_FRONT_RIGHT =        "ir_intensity_front_right";
const std::string IR_INTENSITY_LEFT =               "ir_intensity_left";
const std::string IR_INTENSITY_RIGHT =              "ir_intensity_right";
const std::string IR_INTENSITY_SIDE_LEFT =          "ir_intensity_side_left";

const size_t      IR_INTENSITY_FRONT_CENTER_LEFT_IDX =  0;
const size_t      IR_INTENSITY_FRONT_CENTER_RIGHT_IDX = 1;
const size_t      IR_INTENSITY_FRONT_LEFT_IDX =         2;
const size_t      IR_INTENSITY_FRONT_RIGHT_IDX =        3;
const size_t      IR_INTENSITY_LEFT_IDX =               4;
const size_t      IR_INTENSITY_RIGHT_IDX =              5;
const size_t      IR_INTENSITY_SIDE_LEFT_IDX =          6;

const double      ORIENTATION_GOAL_W = 1.0;

const int         ATTEMPTS_TO_EXEC_CORRECTION     = 1000;
const int         ATTEMPTS_TO_AVOID_OBSTACLE      = 50;
const double      DEFAULT_LINEAR_SPEED_OBSTACLE   = 0.2;
const double      DEFAULT_ANGULAR_SPEED_OBSTACLE  = 1.5;

const int16_t     IR_THRESHOLD = 15;

const int 	  GOAL_REACHED = 0;
const int 	  GOAL_ABORTED = 1;
const int 	  GOAL_FAILED  = 2;
const int 	  GOAL_RUNNING = 3;
const int 	  GOAL_UNKNOWN = 4;
const int 	  GOAL_NONE    = -1;

rclcpp::Node::SharedPtr  shared_node;

class MinimalNavigator : public rclcpp::Node
{
public:
  MinimalNavigator()
  : Node("minimal_navigator")
  {
    // parameters
    param_x = 0.0;
    param_y = 0.0;
    this->declare_parameter("x", 0.0);
    this->declare_parameter("y", 0.0);
    this->declare_parameter("z", 1.0);
    this->declare_parameter("namespace", "");

    // disable safety control
    //ovveride_safety_control();

    enable_abort_workaround = true;
    std_msgs::msg::Int8 msg;
    std::string ns = this->get_parameter("namespace").as_string();
    std::string dock, undock, cmd_vel, navigate, ir_intensity, ap_goal, ap_abort,
	    goal_state, odom;
    if(ns.length()) {
	    odom = "/"+ns+"/odom";
	    undock = "/"+ns+"/undock";
	    dock = "/"+ns+"/dock";
	    cmd_vel = "/"+ns+"/cmd_vel";
	    navigate = "/"+ns+"/navigate_to_position";
	    ir_intensity = "/"+ns+"/ir_intensity";
	    ap_goal = "/"+ns+"/ap_goal";
	    ap_abort = "/"+ns+"/ap_abort";
	    goal_state = "/"+ns+"/goal_state";
    } else {
	    odom = "/odom";
	    undock = "/undock";
	    dock = "/dock";
	    cmd_vel = "/cmd_vel";
	    navigate = "/navigate_to_position";
	    ir_intensity = "/ir_intensity";
	    ap_goal = "/ap_goal";
	    ap_abort = "/ap_abort";
	    goal_state = "/goal_state";
    }
    goal_subscription = this->create_subscription<Goal>(
	ap_goal, 10, std::bind(&MinimalNavigator::new_goal, this, _1));
    if (enable_abort_workaround) {
	    abort_subscription = this->create_subscription<Goal>(
			    ap_abort, 10,
			    std::bind(&MinimalNavigator::abort_goal_workaround, this, _1));
    } else {
	    abort_subscription = this->create_subscription<Goal>(
			    ap_abort, 10,
			    std::bind(&MinimalNavigator::abort_goal, this, _1));
    }

    // vel publisher
    vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      cmd_vel, rclcpp::SensorDataQoS().reliable());
    goal_state_pub = this->create_publisher<std_msgs::msg::Int8>(
      goal_state, 10);

    rclcpp::sleep_for(10s);

    rcl_action_client_options_t opts = rcl_action_client_get_default_options();
    client_navigate_ptr_ = rclcpp_action::create_client<NavigateToPosition>(
    this, navigate, nullptr, opts);
    client_dock_ptr_ = rclcpp_action::create_client<Dock>(this, dock);
    client_undock_ptr_ = rclcpp_action::create_client<Undock>(this, undock);
  
    // subscribe to ir sensors
    subscription_ = this->create_subscription<IrIntensityVector>(
      ir_intensity, rclcpp::SensorDataQoS(),
      std::bind(&MinimalNavigator::topic_ir_callback, this, _1));
    odom_subscription_ = this->create_subscription<Odometry>(
      odom, rclcpp::SensorDataQoS(), std::bind(&MinimalNavigator::topic_odom_callback, this, _1));
    abort_requested = false;
    msg.data = GOAL_NONE;
    goal_state_pub->publish(msg);
    }

private:
  bool abort_requested;
  bool enable_abort_workaround;
  rclcpp::TimerBase::SharedPtr timer_;
  std::shared_ptr<rclcpp::Publisher<geometry_msgs::msg::Twist, std::allocator<void>>> vel_pub_;
  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr goal_state_pub;
  std::vector<IrIntensity, std::allocator<IrIntensity>> vec_ir_sensor;
  geometry_msgs::msg::Twist next_message_twist;
  std::shared_ptr<rclcpp_action::Client<NavigateToPosition>> client_navigate_ptr_;
  std::shared_ptr<rclcpp_action::Client<Dock>> client_dock_ptr_;
  std::shared_ptr<rclcpp_action::Client<Undock>> client_undock_ptr_;
  std::shared_future<std::shared_ptr<GoalHandleNavigateToPosition>> nav_goal_handle;
  rclcpp::Subscription<Odometry>::SharedPtr odom_subscription_;

  void ovveride_safety_control()
  {
    std::string ns = this->get_parameter("namespace").as_string();
    std::string motion_control;
    if (ns.length()) {
	    motion_control = "/"+ns+"/motion_control";
    } else {
	    motion_control = "/motion_control";
    }
    auto parameters_client = std::make_shared<rclcpp::SyncParametersClient>(
		this, motion_control);
    while (!parameters_client->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
        rclcpp::shutdown();
      }
      RCLCPP_INFO(this->get_logger(), "service not available, waiting again...");
    }
    std::vector<rclcpp::Parameter> all_new_parameters{rclcpp::Parameter("safety_override", "full")};
    parameters_client->set_parameters(all_new_parameters);
  }

  void new_goal(const Goal & msg)
  {
	  this->param_x = msg.x;
	  this->param_y = msg.y;
	  if (msg.type.compare("GOAL") == 0) {
		  navigate_to_position(msg);
	  }
	  if (msg.type.compare("UNDOCK") == 0) {
		  handle_undock();
	  }
	  if (msg.type.compare("DOCK") == 0) {
		  handle_dock();
	  }
  }

  void abort_goal_workaround(const Goal & msg)
  {
    RCLCPP_INFO(this->get_logger(), "Cancelling current goal");
    abort_requested = true;
  }

  void abort_goal(const Goal & msg)
  {
    RCLCPP_INFO(this->get_logger(), "Cancelling current goal");
    try {
	    client_navigate_ptr_->async_cancel_goal(nav_goal_handle.get(), nullptr);
    } catch (rclcpp_action::exceptions::UnknownGoalHandleError const& ex) {
	    RCLCPP_ERROR(this->get_logger(), "Unknown goal handle");
    }
    RCLCPP_INFO(this->get_logger(), "Cancelled goal");
  }

  std::shared_future<std::shared_ptr<GoalHandleDock>> handle_dock()
  {
	  auto send_goal_options = rclcpp_action::Client<Dock>::SendGoalOptions();
	  send_goal_options.goal_response_callback = std::bind(
  		  &MinimalNavigator::goal_response_dock_callback, this, _1);
	  if (!client_dock_ptr_->wait_for_action_server()) {
		  RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
		  rclcpp::shutdown();
	  }
	  auto goal_msg = Dock::Goal();
	  auto action_handle = client_dock_ptr_->async_send_goal(goal_msg, send_goal_options);
	  RCLCPP_INFO(this->get_logger(), "...docking...");
	  return action_handle;
  }

  std::shared_future<std::shared_ptr<GoalHandleUndock>> handle_undock()
  {
	  auto send_goal_options = rclcpp_action::Client<Undock>::SendGoalOptions();
	  send_goal_options.goal_response_callback = std::bind(
			  &MinimalNavigator::goal_response_undock_callback, this, _1);
	  if (!client_undock_ptr_->wait_for_action_server()) {
		  RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
		  rclcpp::shutdown();
	  }
	  auto goal_msg = Undock::Goal();
	  auto client_handle = client_undock_ptr_->async_send_goal(goal_msg, send_goal_options);
	  RCLCPP_INFO(this->get_logger(), "...undocking...");
	  return client_handle;
  }

  void navigate_to_position(const Goal & msg)
  {
    RCLCPP_INFO(this->get_logger(), "...navigate to position goal...");
    std_msgs::msg::Int8 goal_state;
    auto send_goal_options = rclcpp_action::Client<NavigateToPosition>::SendGoalOptions();
    send_goal_options.result_callback = std::bind(&MinimalNavigator::result_callback_goal_position, this, _1);
    if (!client_navigate_ptr_->wait_for_action_server()) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
      rclcpp::shutdown();
    }

    // prepare goal msg
    auto goal_msg = NavigateToPosition::Goal();
    goal_msg.achieve_goal_heading = true;
    geometry_msgs::msg::PoseStamped goal_pose = geometry_msgs::msg::PoseStamped();
    geometry_msgs::msg::Pose pose = geometry_msgs::msg::Pose();
    geometry_msgs::msg::Point point = geometry_msgs::msg::Point();
    point.x = msg.x;
    point.y = msg.y;
    point.z = 0.0;
    geometry_msgs::msg::Quaternion quaternion;
    quaternion.x = 0.0;
    quaternion.y = 0.0;
    quaternion.z = msg.qz;
    quaternion.w = msg.qw;
    pose.set__orientation(quaternion);
    pose.set__position(point);
    goal_pose.set__pose(pose);
    goal_msg.set__goal_pose(goal_pose);

    // send goal
    nav_goal_handle = client_navigate_ptr_->async_send_goal(goal_msg, send_goal_options);
    goal_state.data = GOAL_RUNNING;
    goal_state_pub->publish(goal_state);
  }

  void goal_response_undock_callback(GoalHandleUndock::SharedPtr goal_handle)
  {
    //auto goal_handle = future.get();
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Undock goal was rejected by server");
    } else {
      RCLCPP_INFO(this->get_logger(), "Undock goal accepted by server, waiting for result");
    }
  }

  void goal_response_dock_callback(GoalHandleDock::SharedPtr goal_handle)
  {
    //auto goal_handle = future.get();
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Dock goal was rejected by server");
    } else {
      RCLCPP_INFO(this->get_logger(), "Dock goal accepted by server, waiting for result");
    }
  }

  void try_to_avoid_obstacle()
  {

    if (vec_ir_sensor.size() > 0)
    {
      std::string state_description;
      double linear_speed = DEFAULT_LINEAR_SPEED_OBSTACLE;
      double angular_speed = DEFAULT_ANGULAR_SPEED_OBSTACLE;

      double linear_x = 0.0;
      double linear_y = 0.0;
      double angular_z = 0.0;

      bool front_obstacle_presence = is_in_front_obstacle();
      bool left_obstacle_presence = is_in_left_obstacle();
      bool right_obstacle_presence = is_in_right_obstacle();

      if (!front_obstacle_presence && !left_obstacle_presence && !right_obstacle_presence) {
        state_description = "case 1 - no obstacle";
        linear_x = linear_speed;
        angular_z = 0;
      }
      else if (front_obstacle_presence && left_obstacle_presence && right_obstacle_presence) {
        state_description = "case 7 - front and left and right";
        
        linear_x = -linear_speed;
        angular_z = angular_speed; // Increase this angular speed for avoiding obstacle faster
      }
      else if (front_obstacle_presence && !left_obstacle_presence && !right_obstacle_presence) {
        state_description = "case 2 - front";
        linear_x = -0.1;
        angular_z = angular_speed;
      }      
      else if (!front_obstacle_presence && !left_obstacle_presence && right_obstacle_presence) {
        state_description = "case 3 - right";
        linear_x = 0.1;
        angular_z = angular_speed;
      }
      else if (!front_obstacle_presence && left_obstacle_presence && !right_obstacle_presence) {
        state_description = "case 4 - left";
        linear_x = 0.1;
        angular_z = -angular_speed;
      }
      else if (front_obstacle_presence && !left_obstacle_presence && right_obstacle_presence) {
        state_description = "case 5 - front and right";
        linear_x = linear_speed;
        angular_z = angular_speed;
      }
      else if (front_obstacle_presence && left_obstacle_presence && !right_obstacle_presence) {
        state_description = "case 6 - front and left";
        linear_x = linear_speed;
        angular_z = -angular_speed;
      }
      else if (!front_obstacle_presence && left_obstacle_presence && right_obstacle_presence) {
        state_description = "case 8 - left and right";
        linear_x = linear_speed;
        angular_z = angular_speed;
      }
      else
      {
        state_description = "unknown case";
        RCLCPP_INFO(this->get_logger(),
		    "front_obstacle_presence=%d, left_obstacle_presence=%d, right_obstacle_presence=%d",
		    front_obstacle_presence, left_obstacle_presence,
		    right_obstacle_presence);
      }

      RCLCPP_INFO(this->get_logger(), state_description.c_str());

      // prepare msg twist
      next_message_twist.linear.x = linear_x;
      next_message_twist.linear.y = linear_y;
      next_message_twist.angular.z = angular_z;

      RCLCPP_INFO(this->get_logger(), "move to: %s, %s", std::to_string(linear_x).c_str(), std::to_string(angular_z).c_str());

      // moving with msg for ATTEMPTS_TO_AVOID_OBSTACLE
      for (int i=0;i<ATTEMPTS_TO_AVOID_OBSTACLE;i++)
        {
          vel_pub_->publish(next_message_twist);
          // if there are no more obstacles, stop
          if (!is_in_front_obstacle() && !is_in_left_obstacle() && !is_in_right_obstacle())
          {
            RCLCPP_INFO(this->get_logger(), "no obstacles, stop");
            return;
          }
        }
    }
  }

  bool is_in_front_obstacle() { return vec_ir_sensor[IR_INTENSITY_FRONT_CENTER_LEFT_IDX].value > IR_THRESHOLD || vec_ir_sensor[IR_INTENSITY_FRONT_CENTER_RIGHT_IDX].value > IR_THRESHOLD || vec_ir_sensor[IR_INTENSITY_FRONT_LEFT_IDX].value > IR_THRESHOLD || vec_ir_sensor[IR_INTENSITY_FRONT_RIGHT_IDX].value > IR_THRESHOLD;}
  bool is_in_left_obstacle() { return vec_ir_sensor[IR_INTENSITY_LEFT_IDX].value > IR_THRESHOLD || vec_ir_sensor[IR_INTENSITY_SIDE_LEFT_IDX].value > IR_THRESHOLD;}
  bool is_in_right_obstacle() { return vec_ir_sensor[IR_INTENSITY_RIGHT_IDX].value > IR_THRESHOLD;}

  //result
  void result_callback_goal_position(const GoalHandleNavigateToPosition::WrappedResult & result)
  {
    std_msgs::msg::Int8 msg;
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        RCLCPP_INFO(get_logger(), "Success, position reached!!!");
	msg.data = GOAL_REACHED;
	goal_state_pub->publish(msg);
        break;
      case rclcpp_action::ResultCode::ABORTED:
	break;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_INFO(get_logger(), "Goal aborted by AP");
	msg.data = GOAL_ABORTED;
	goal_state_pub->publish(msg);
        break;
      default:
        RCLCPP_ERROR(get_logger(), "Unknown result code");
        RCLCPP_INFO(get_logger(), "Try to avoid obstacle");
	msg.data = GOAL_UNKNOWN;
	goal_state_pub->publish(msg);
        break;
    }
  }

  void topic_odom_callback(const Odometry::SharedPtr abort_msg)
  {
	  if (abort_requested) {
		  RCLCPP_INFO(this->get_logger(), "Aborting goal");
		  auto x = abort_msg->pose.pose.position.x;
		  auto y = abort_msg->pose.pose.position.y;
		  std_msgs::msg::Int8 msg;
		  auto send_goal_options = rclcpp_action::Client<NavigateToPosition>::SendGoalOptions();
		  if (!client_navigate_ptr_->wait_for_action_server()) {
			  RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
			  rclcpp::shutdown();
		  }

		  // prepare goal msg
		  auto goal_msg = NavigateToPosition::Goal();
		  goal_msg.achieve_goal_heading = false;
		  geometry_msgs::msg::PoseStamped goal_pose = geometry_msgs::msg::PoseStamped();
		  geometry_msgs::msg::Pose pose = geometry_msgs::msg::Pose();
		  geometry_msgs::msg::Point point = geometry_msgs::msg::Point();
		  point.x = x;
		  point.y = y;
		  point.z = 0.0;
		  geometry_msgs::msg::Quaternion quaternion;
		  quaternion.x = 0.0;
		  quaternion.y = 0.0;
		  quaternion.z = 0.0;
		  quaternion.w = ORIENTATION_GOAL_W;
		  pose.set__orientation(quaternion);
		  pose.set__position(point);
		  goal_pose.set__pose(pose);
		  goal_msg.set__goal_pose(goal_pose);

		  // send goal
		  nav_goal_handle = client_navigate_ptr_->async_send_goal(goal_msg, send_goal_options);
		  msg.data = GOAL_ABORTED;
		  goal_state_pub->publish(msg);
		  abort_requested = false;
	  }
  }

  void topic_ir_callback(const IrIntensityVector::SharedPtr msg)
    {
      bool some_obstacle_was_found = false;
      for (long unsigned int i=0; i<msg->readings.size(); i++)
      {
        vec_ir_sensor.push_back(msg->readings[i]);
        std::string position;
        switch (i)
        {
        case IR_INTENSITY_FRONT_CENTER_LEFT_IDX:
          position = IR_INTENSITY_FRONT_CENTER_LEFT;
          break;

          case IR_INTENSITY_FRONT_CENTER_RIGHT_IDX:
          position = IR_INTENSITY_FRONT_CENTER_RIGHT;
          break;

          case IR_INTENSITY_FRONT_LEFT_IDX:
          position = IR_INTENSITY_FRONT_LEFT;
          break;

          case IR_INTENSITY_FRONT_RIGHT_IDX:
          position = IR_INTENSITY_FRONT_RIGHT;
          break;

          case IR_INTENSITY_LEFT_IDX:
          position = IR_INTENSITY_LEFT;
          break;

          case IR_INTENSITY_RIGHT_IDX:
          position = IR_INTENSITY_RIGHT;
          break;

          case IR_INTENSITY_SIDE_LEFT_IDX:
          position = IR_INTENSITY_SIDE_LEFT;
          break;

        default:
          break;
        }
        if (msg->readings[i].value > IR_THRESHOLD)
        {
          some_obstacle_was_found = true;
          // RCLCPP_INFO(this->get_logger(),"I heard from '%s': '%d'", position.c_str(), msg->readings[i].value);
        }
      }
      if (some_obstacle_was_found)
      {
        // reset array
        vec_ir_sensor.clear();
        // populate array with IR sensor values
        for (long unsigned int i=0; i<msg->readings.size(); i++)
        {
          vec_ir_sensor.push_back(msg->readings[i]);
        }
      }
      else
      {
        // clear array and insert default values
        vec_ir_sensor.clear();
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
        vec_ir_sensor.push_back(IrIntensity());
      }
    }
    rclcpp::Subscription<IrIntensityVector>::SharedPtr subscription_;
    rclcpp::Subscription<Goal>::SharedPtr goal_subscription;
    rclcpp::Subscription<Goal>::SharedPtr abort_subscription;
    double param_x;
    double param_y;
};

int main(int argc, char * argv[])
{
  printf("hello world create3_poc_didattico package\n");

  rclcpp::init(argc, argv);
  shared_node = std::make_shared<MinimalNavigator>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(shared_node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
