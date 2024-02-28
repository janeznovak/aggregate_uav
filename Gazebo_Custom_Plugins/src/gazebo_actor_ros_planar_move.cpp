// Copyright 2024 University of Turin
//  Heavily modified to work with actor and produce syntethic wearable data.
// Copyright 2019 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * \brief  Actor controller that uses a Pose message to move an entity on the xy plane.
 *
 * \author Giordano Scarso
 *
 * \date  26 January 2024
 */

#define elif else if
#include <gazebo/common/Events.hh>
#include <gazebo/physics/Model.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/physics/World.hh>
#include <gazebo_plugins/gazebo_actor_ros_planar_move.hpp>
#include <gazebo_ros/conversions/builtin_interfaces.hpp>
#include <gazebo_ros/conversions/geometry_msgs.hpp>
#include <gazebo_ros/node.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>
#ifdef IGN_PROFILER_ENABLE
#include <ignition/common/Profiler.hh>
#endif
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>

#include "std_msgs/msg/int8.hpp"
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <ignition/math/Pose3.hh>

#include <memory>
#include <string>
#include <random>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include "wearable_interfaces/msg/sensor.hpp"
#include "wearable_interfaces/msg/trigger.hpp"

namespace gazebo_plugins
{
class GazeboActorRosPlanarMovePrivate
{
public:
  /// Callback to be called at every simulation iteration.
  /// \param[in] _info Updated simulation info.
  void OnUpdate(const gazebo::common::UpdateInfo & _info);

  /// Callback when a velocity command is received.
  /// \param[in] _msg Twist command message.
  void OnCmdVel(const geometry_msgs::msg::Twist::SharedPtr _msg);

  void OnTargetPose(const geometry_msgs::msg::Pose::SharedPtr _msg);

  /// Callback when a velocity command is received.
  /// \param[in] _msg Twist command message.
  void OnAction(const std_msgs::msg::Int8::SharedPtr _msg);

  /// Update odometry.
  /// \param[in] _current_time Current simulation time
  void UpdateOdometry(const gazebo::common::Time & _current_time);

  /// Publish odometry transforms
  /// \param[in] _current_time Current simulation time
  void PublishOdometryTf(const gazebo::common::Time & _current_time);
  void PublishSensors();
  void PublishTriggers();

  void EnableCollision();

  void UpdateAnimation(std::string animation);

  void TriggerSos();
  template <typename T> void ResetArray(T array[], int size);
  template <typename T> T LimitValue(T value, T min, T max);

  /// A pointer to the GazeboROS node.
  gazebo_ros::Node::SharedPtr ros_node_;

  /// Subscriber to command velocities
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;

  /// Subscriber to command action
  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr action_sub_;

  rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr target_pose_sub_;

  /// Odometry publisher
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odometry_pub_;

  rclcpp::Publisher<wearable_interfaces::msg::Sensor>::SharedPtr sensors_pub_;

  rclcpp::Publisher<wearable_interfaces::msg::Trigger>::SharedPtr triggers_pub_;

  /// To broadcast TF
  std::shared_ptr<tf2_ros::TransformBroadcaster> transform_broadcaster_;

  /// Velocity received on command.
  geometry_msgs::msg::Twist target_cmd_vel_;

  /// Keep latest odometry message
  nav_msgs::msg::Odometry odom_;

  /// Pointer to world.
  gazebo::physics::WorldPtr world_;

  /// Pointer to model.
  gazebo::physics::ModelPtr model_;

  /// Pointer to actor.
  gazebo::physics::ActorPtr actor_;

  /// Connection to event called at every world iteration.
  gazebo::event::ConnectionPtr update_connection_;

  /// Protect variables accessed on callbacks.
  std::mutex lock_;

  /// Update period in seconds.
  double update_period_;

  /// Publish period in seconds.
  double publish_period_;

  double sensor_publish_threshold_;

  /// Publish period in seconds.
  double script_time_;

  double speed_factor_;
  double minX_;
  double minY_;
  double maxX_;
  double maxY_;

  /// Last update time.
  gazebo::common::Time last_update_time_;

  /// Last publish time.
  gazebo::common::Time last_publish_time_;

  /// Odometry frame ID
  std::string odometry_frame_;

  /// Robot base frame ID
  std::string robot_base_frame_;

  /// True to publish odometry messages.
  bool publish_odom_;

  /// True to publish odom-to-world transforms.
  bool publish_odom_tf_;

  ignition::math::Pose3d curr_pos_;
  ignition::math::Pose3d target_pos_;

  gazebo::physics::TrajectoryInfoPtr trajectoryInfo;

  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution<float> hrt{110.0, 8.0};
  std::normal_distribution<float> ox{95.0, 3.0};
  std::normal_distribution<float> temp{36.5, 1.0};
  int heart_rate_per_minute;
  float body_temperature_arr[10];
  int body_temperature_index;
  float body_temperature;
  int oxygenation_arr[10];
  int oxygenation_index;
  int oxygenation;
  bool random_movement_;
  bool automatic_trigger_;
  bool accidental_fall_sos_triggered;
  bool heart_rate_sos_triggered;
  bool body_temperature_sos_triggered;
  bool oxygenation_sos_triggered;
  bool follow_triggered;
  bool over_60;
  int priority;

  sdf::ElementPtr sdf_;
};

GazeboActorRosPlanarMove::GazeboActorRosPlanarMove()
: impl_(std::make_unique<GazeboActorRosPlanarMovePrivate>())
{
}

GazeboActorRosPlanarMove::~GazeboActorRosPlanarMove()
{
}

void GazeboActorRosPlanarMove::Load(gazebo::physics::ModelPtr _model, sdf::ElementPtr _sdf)
{
  std::srand(std::time(nullptr));
  impl_->model_ = _model;
  impl_->sdf_ = _sdf;
  impl_->actor_ = boost::dynamic_pointer_cast<gazebo::physics::Actor>(_model);
  impl_->world_ = _model->GetWorld();

  // values near the mean are the most likely
  // standard deviation affects the dispersion of generated values from the mean
  // Initialize ROS node
  impl_->ros_node_ = gazebo_ros::Node::Get(_sdf);

  // Get QoS profiles
  const gazebo_ros::QoS & qos = impl_->ros_node_->get_qos();

  // Odometry
  impl_->odometry_frame_ = _sdf->Get<std::string>("odometry_frame", "odom").first;
  impl_->robot_base_frame_ = _sdf->Get<std::string>("robot_base_frame", "actor_pose").first;

  // Update rate
  auto update_rate = _sdf->Get<double>("update_rate", 20.0).first;
  if (update_rate > 0.0) {
    impl_->update_period_ = 1.0 / update_rate;
  } else {
    impl_->update_period_ = 0.0;
  }
  impl_->last_update_time_ = impl_->world_->SimTime();

  // Update rate
  auto publish_rate = _sdf->Get<double>("publish_rate", 20.0).first;
  if (update_rate > 0.0) {
    impl_->publish_period_ = 1.0 / publish_rate;
  } else {
    impl_->publish_period_ = 0.0;
  }
  impl_->speed_factor_ = 0.5;
  impl_->last_publish_time_ = impl_->world_->SimTime();

  impl_->cmd_vel_sub_ = impl_->ros_node_->create_subscription<geometry_msgs::msg::Twist>(
    "cmd_vel", qos.get_subscription_qos("cmd_vel", rclcpp::QoS(1)),
    std::bind(&GazeboActorRosPlanarMovePrivate::OnCmdVel, impl_.get(), std::placeholders::_1));

  impl_->action_sub_ = impl_->ros_node_->create_subscription<std_msgs::msg::Int8>(
    "patient_action", qos.get_subscription_qos("patient_action", rclcpp::QoS(1)),
    std::bind(&GazeboActorRosPlanarMovePrivate::OnAction, impl_.get(), std::placeholders::_1));

  impl_->target_pose_sub_ = impl_->ros_node_->create_subscription<geometry_msgs::msg::Pose>(
    "target_pose", qos.get_subscription_qos("target_pose", rclcpp::QoS(1)),
    std::bind(&GazeboActorRosPlanarMovePrivate::OnTargetPose, impl_.get(), std::placeholders::_1));

  RCLCPP_INFO(
    impl_->ros_node_->get_logger(), "Subscribed to [%s]",
    impl_->cmd_vel_sub_->get_topic_name());

  // Advertise odometry topic
  impl_->publish_odom_ = _sdf->Get<bool>("publish_odom", true).first;
  if (impl_->publish_odom_) {
    impl_->odometry_pub_ = impl_->ros_node_->create_publisher<nav_msgs::msg::Odometry>(
      "odom", qos.get_publisher_qos("odom", rclcpp::QoS(1)));

    RCLCPP_INFO(
      impl_->ros_node_->get_logger(), "Advertise odometry on [%s]",
      impl_->odometry_pub_->get_topic_name());
  }

  impl_->sensors_pub_ = impl_->ros_node_->create_publisher<wearable_interfaces::msg::Sensor>(
      "sensors", qos.get_publisher_qos("sensors", rclcpp::QoS(1)));
  RCLCPP_INFO(
      impl_->ros_node_->get_logger(), "Advertise sensors on [%s]",
      impl_->sensors_pub_->get_topic_name());

  impl_->triggers_pub_ = impl_->ros_node_->create_publisher<wearable_interfaces::msg::Trigger>(
      "triggers", qos.get_publisher_qos("triggers", rclcpp::QoS(1)));
  RCLCPP_INFO(
      impl_->ros_node_->get_logger(), "Advertise triggers on [%s]",
      impl_->triggers_pub_->get_topic_name());

  // Broadcast TF
  impl_->publish_odom_tf_ = _sdf->Get<bool>("publish_odom_tf", true).first;
  if (impl_->publish_odom_tf_) {
    impl_->transform_broadcaster_ =
      std::make_shared<tf2_ros::TransformBroadcaster>(impl_->ros_node_);

    RCLCPP_INFO(
      impl_->ros_node_->get_logger(),
      "Publishing odom transforms between [%s] and [%s]", impl_->odometry_frame_.c_str(),
      impl_->robot_base_frame_.c_str());
  }

  auto covariance_x = _sdf->Get<double>("covariance_x", 0.00001).first;
  auto covariance_y = _sdf->Get<double>("covariance_y", 0.00001).first;
  auto covariance_yaw = _sdf->Get<double>("covariance_yaw", 0.001).first;

  // Set covariance
  impl_->odom_.pose.covariance[0] = covariance_x;
  impl_->odom_.pose.covariance[7] = covariance_y;
  impl_->odom_.pose.covariance[14] = 1000000000000.0;
  impl_->odom_.pose.covariance[21] = 1000000000000.0;
  impl_->odom_.pose.covariance[28] = 1000000000000.0;
  impl_->odom_.pose.covariance[35] = covariance_yaw;

  impl_->odom_.twist.covariance[0] = covariance_x;
  impl_->odom_.twist.covariance[7] = covariance_y;
  impl_->odom_.twist.covariance[14] = 1000000000000.0;
  impl_->odom_.twist.covariance[21] = 1000000000000.0;
  impl_->odom_.twist.covariance[28] = 1000000000000.0;
  impl_->odom_.twist.covariance[35] = covariance_yaw;
  impl_->target_pos_ = impl_->model_->WorldPose();

  // Set header
  impl_->odom_.header.frame_id = impl_->odometry_frame_;
  impl_->odom_.child_frame_id = impl_->robot_base_frame_;
  impl_->minX_ = _sdf->Get<double>("minX", 0.0).first;
  impl_->maxX_ = _sdf->Get<double>("maxX", 5.0).first;
  impl_->minY_ = _sdf->Get<double>("minY", 0.0).first;
  impl_->maxY_ = _sdf->Get<double>("maxY", 5.0).first;
  impl_->random_movement_ = _sdf->Get<bool>("random_movement", false).first;
  impl_->automatic_trigger_ = _sdf->Get<bool>("automatic_trigger", true).first;
  RCLCPP_INFO(impl_->ros_node_->get_logger(), "X:(%f %f), Y:(%f %f)",
  impl_->minX_, impl_->maxX_, impl_->minY_, impl_->maxY_);
  impl_->over_60 = true;
  impl_->sensor_publish_threshold_ = 0.0;
  impl_->priority = 0;
  this->Reset();

  // Listen to the update event (broadcast every simulation iteration)
  impl_->update_connection_ = gazebo::event::Events::ConnectWorldUpdateBegin(
    std::bind(&GazeboActorRosPlanarMovePrivate::OnUpdate, impl_.get(), std::placeholders::_1));
}

void GazeboActorRosPlanarMove::Reset()
{
  impl_->last_update_time_ = impl_->world_->SimTime();
  impl_->target_cmd_vel_.linear.x = 0;
  impl_->target_cmd_vel_.linear.y = 0;
  impl_->target_cmd_vel_.angular.z = 0;
  impl_->EnableCollision();
  impl_->trajectoryInfo.reset(new gazebo::physics::TrajectoryInfo());
  impl_->trajectoryInfo->type = "stand";
  impl_->trajectoryInfo->duration = 1.0;
  impl_->actor_->SetCustomTrajectory(impl_->trajectoryInfo);
  impl_->accidental_fall_sos_triggered = false;
  impl_->heart_rate_sos_triggered = false;
  impl_->body_temperature_sos_triggered = false;
  impl_->oxygenation_sos_triggered = false;
  impl_->follow_triggered = false;
  impl_->body_temperature_index = 0;
  impl_->body_temperature_index = 0;
  impl_->ResetArray<int>(impl_->oxygenation_arr, 10);
  impl_->ResetArray<float>(impl_->body_temperature_arr, 10);
}

void GazeboActorRosPlanarMovePrivate::OnUpdate(const gazebo::common::UpdateInfo & _info)
{
  double seconds_since_last_update = (_info.simTime - last_update_time_).Double();
  sensor_publish_threshold_ += seconds_since_last_update;
  std::lock_guard<std::mutex> scoped_lock(lock_);
#ifdef IGN_PROFILER_ENABLE
  IGN_PROFILE("GazeboActorRosPlanarMovePrivate::OnUpdate");
  IGN_PROFILE_BEGIN("fill ROS message");
#endif
  if (seconds_since_last_update >= update_period_) {
    heart_rate_per_minute = LimitValue<int>(std::round(hrt(gen)),50,200);

    body_temperature = LimitValue<float>(temp(gen),35.0,41.0);
    body_temperature = (float)((int)(body_temperature*100))/100;
    body_temperature_arr[body_temperature_index] = body_temperature;
    body_temperature_index = (body_temperature_index + 1) % 10;

    oxygenation = LimitValue<int>(std::round(ox(gen)),80,100);
    oxygenation_arr[oxygenation_index] = oxygenation;
    oxygenation_index = (oxygenation_index + 1) % 10;
    ignition::math::Pose3d pose = model_->WorldPose();
    auto diff = target_pos_.Pos() - pose.Pos();
    auto linear = ignition::math::Vector3d(0.0, 0.0, 0.0);
    auto angular = ignition::math::Vector3d(0.0, 0.0, 0.0);
    ignition::math::Angle yaw = atan2(diff.Y(), diff.X()) + 1.5707 - model_->WorldPose().Rot().Euler().Z();
    yaw.Normalize();
    if (diff.Length() > 0.3) {
      if (trajectoryInfo->type != "walking") {
        UpdateAnimation("walking");
      }
      linear = ignition::math::Vector3d( diff.X() * speed_factor_,
          diff.Y() * speed_factor_, 0.0);
      angular = ignition::math::Vector3d(0.0, 0.0, yaw.Radian() * 2.0);
    } else {
      if (random_movement_) {
        target_pos_ = ignition::math::Pose3d(
            minX_ + (static_cast<double>(std::rand())/RAND_MAX)*(maxX_-minX_),
            minY_ + (static_cast<double>(std::rand())/RAND_MAX)*(maxY_-minY_),
            1.0, 0.0, 0.0, 0.0);
      } elif(trajectoryInfo->type == "walking") {
        model_->ResetPhysicsStates();
        UpdateAnimation("stand");
      }
    }
    model_->SetLinearVel(linear);
    model_->SetAngularVel(angular);
    actor_->SetScriptTime(actor_->ScriptTime() + seconds_since_last_update);
    RCLCPP_DEBUG(ros_node_->get_logger(), "Actor pose [%f,%f,%f,%f]",
        actor_->WorldPose().Pos().X(),
        actor_->WorldPose().Pos().Y(),
        actor_->WorldPose().Pos().Z(),
        actor_->WorldPose().Rot().Yaw()
        );
    if (automatic_trigger_) {
      TriggerSos();
    }

    last_update_time_ = _info.simTime;
  }
  if (sensor_publish_threshold_ > 125.0) {
    PublishSensors();
    sensor_publish_threshold_ = 0.0;
  }
#ifdef IGN_PROFILER_ENABLE
  IGN_PROFILE_END();
#endif
  if (publish_odom_ || publish_odom_tf_) {
    double seconds_since_last_publish = (_info.simTime - last_publish_time_).Double();

    if (seconds_since_last_publish < publish_period_) {
      return;
    }

#ifdef IGN_PROFILER_ENABLE
    IGN_PROFILE_BEGIN("UpdateOdometry");
#endif
    UpdateOdometry(_info.simTime);
#ifdef IGN_PROFILER_ENABLE
    IGN_PROFILE_END();
#endif
    if (publish_odom_) {
#ifdef IGN_PROFILER_ENABLE
      IGN_PROFILE_BEGIN("publish odometry");
#endif
      odometry_pub_->publish(odom_);
#ifdef IGN_PROFILER_ENABLE
      IGN_PROFILE_END();
#endif
    }
    if (publish_odom_tf_) {
#ifdef IGN_PROFILER_ENABLE
      IGN_PROFILE_BEGIN("publish odometryTF");
#endif
      PublishOdometryTf(_info.simTime);
#ifdef IGN_PROFILER_ENABLE
      IGN_PROFILE_END();
#endif
    }

    last_publish_time_ = _info.simTime;
  }
}

void GazeboActorRosPlanarMovePrivate::UpdateAnimation(std::string animation)
{
  trajectoryInfo.reset(new gazebo::physics::TrajectoryInfo());
  trajectoryInfo->type = animation;
  trajectoryInfo->duration = 1.0;
  actor_->SetCustomTrajectory(trajectoryInfo);
}

void GazeboActorRosPlanarMovePrivate::OnCmdVel(const geometry_msgs::msg::Twist::SharedPtr _msg)
{
  std::lock_guard<std::mutex> scoped_lock(lock_);
  target_cmd_vel_ = *_msg;
}

void GazeboActorRosPlanarMovePrivate::OnTargetPose(const geometry_msgs::msg::Pose::SharedPtr _msg)
{
  std::lock_guard<std::mutex> scoped_lock(lock_);
  random_movement_ = false;
  auto value = *_msg;
  target_pos_ = ignition::math::Pose3d(
      value.position.x,
      value.position.y,
      value.position.z,
      0.0, 0.0, 0.0);
}

void GazeboActorRosPlanarMovePrivate::OnAction(const std_msgs::msg::Int8::SharedPtr _msg)
{
  std::lock_guard<std::mutex> scoped_lock(lock_);
  auto value = *_msg;
  RCLCPP_INFO(ros_node_->get_logger(), "Patient action received [%d]",
      value.data);
  if (value.data != 6) { random_movement_ = false; }
  switch(value.data) {
    case 1:
      RCLCPP_INFO(ros_node_->get_logger(), "Triggered accidental fall");
      target_pos_ = ignition::math::Pose3d(model_->WorldPose());
      UpdateAnimation("sitting");
      accidental_fall_sos_triggered = true;
      priority = 1;
      PublishTriggers();
      break;
    case 2:
      RCLCPP_INFO(ros_node_->get_logger(), "Triggered oxygenation warn");
      target_pos_ = ignition::math::Pose3d(model_->WorldPose());
      UpdateAnimation("sitting");
      oxygenation_sos_triggered = true;
      priority = 1;
      PublishTriggers();
      break;
    case 3:
      RCLCPP_INFO(ros_node_->get_logger(), "Triggered body_temperature warn");
      target_pos_ = ignition::math::Pose3d(model_->WorldPose());
      UpdateAnimation("sitting");
      body_temperature_sos_triggered = true;
      priority = 1;
      PublishTriggers();
      break;
    case 4:
      RCLCPP_INFO(ros_node_->get_logger(), "Triggered heart_rate warn");
      target_pos_ = ignition::math::Pose3d(model_->WorldPose());
      UpdateAnimation("sitting");
      heart_rate_sos_triggered = true;
      priority = 1;
      PublishTriggers();
      break;
    case 5:
      RCLCPP_INFO(ros_node_->get_logger(), "Triggered manual call");
      target_pos_ = ignition::math::Pose3d(model_->WorldPose());
      UpdateAnimation("talk_a");
      priority = 0;
      PublishTriggers();
      break;
    case 6:
      //Random movement after goal completition
      random_movement_ = true;
      break;
  }
}

void GazeboActorRosPlanarMovePrivate::UpdateOdometry(const gazebo::common::Time & _current_time)
{
  auto pose = model_->WorldPose();
  odom_.pose.pose = gazebo_ros::Convert<geometry_msgs::msg::Pose>(pose);

  // Get velocity in odom frame
  odom_.twist.twist.angular.z = model_->WorldAngularVel().Z();

  // Convert velocity to child_frame_id(aka base_footprint)
  auto linear = model_->WorldLinearVel();
  auto yaw = static_cast<float>(pose.Rot().Yaw());
  odom_.twist.twist.linear.x = cosf(yaw) * linear.X() + sinf(yaw) * linear.Y();
  odom_.twist.twist.linear.y = cosf(yaw) * linear.Y() - sinf(yaw) * linear.X();

  // Set timestamp
  odom_.header.stamp = gazebo_ros::Convert<builtin_interfaces::msg::Time>(_current_time);
}

void GazeboActorRosPlanarMovePrivate::PublishOdometryTf(const gazebo::common::Time & _current_time)
{
  geometry_msgs::msg::TransformStamped msg;
  msg.header.stamp = gazebo_ros::Convert<builtin_interfaces::msg::Time>(_current_time);
  msg.header.frame_id = odometry_frame_;
  msg.child_frame_id = robot_base_frame_;
  msg.transform = gazebo_ros::Convert<geometry_msgs::msg::Transform>(odom_.pose.pose);

  transform_broadcaster_->sendTransform(msg);
}

void GazeboActorRosPlanarMovePrivate::PublishSensors()
{
  auto now = std::chrono::system_clock::now();
  wearable_interfaces::msg::Sensor msg;
  msg.wearable_code = ros_node_->get_namespace();
  msg.heart_rate_per_minute = heart_rate_per_minute;
  msg.body_temperature = body_temperature;
  msg.oxygenation = oxygenation;
  msg.goal_status = -1;
  msg.goal_code = "";
  msg.unix_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count();
  sensors_pub_->publish(msg);
}

void GazeboActorRosPlanarMovePrivate::PublishTriggers()
{
  auto now = std::chrono::system_clock::now();
  wearable_interfaces::msg::Trigger msg;
  msg.wearable_code = ros_node_->get_namespace();
  msg.heart_rate_per_minute = heart_rate_per_minute;
  msg.body_temperature = body_temperature;
  msg.oxygenation = oxygenation;
  msg.accidental_fall_sos_triggered = accidental_fall_sos_triggered;
  msg.heart_rate_sos_triggered = heart_rate_sos_triggered;
  msg.body_temperature_sos_triggered = body_temperature_sos_triggered;
  msg.oxygenation_sos_triggered = oxygenation_sos_triggered;
  msg.follow_triggered = follow_triggered;
  msg.priority = priority;
  msg.unix_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count();
  triggers_pub_->publish(msg);
  //Reset values after publish
  accidental_fall_sos_triggered = false;
  heart_rate_sos_triggered = false;
  body_temperature_sos_triggered = false;
  oxygenation_sos_triggered = false;
  follow_triggered = false;
  priority = 0;
}

void GazeboActorRosPlanarMovePrivate::EnableCollision()
{
  // Map of collision scaling factors
  std::map<std::string, ignition::math::Vector3d> scaling;
  std::map<std::string, ignition::math::Pose3d> offsets;

  // Read in the collision scaling factors, if present
  if (sdf_->HasElement("scaling"))
  {
    auto elem = sdf_->GetElement("scaling");
    while (elem)
    {
      if (!elem->HasAttribute("collision"))
      {
        gzwarn << "Skipping element without collision attribute" << std::endl;
        elem = elem->GetNextElement("scaling");
        continue;
      }
      auto name = elem->Get<std::string>("collision");

      if (elem->HasAttribute("scale"))
      {
        auto scale = elem->Get<ignition::math::Vector3d>("scale");
        scaling[name] = scale;
      }

      if (elem->HasAttribute("pose"))
      {
        auto pose = elem->Get<ignition::math::Pose3d>("pose");
        offsets[name] = pose;
      }
      elem = elem->GetNextElement("scaling");
    }
  }

  for (const auto &link : actor_->GetLinks())
  {
    // Init the links, which in turn enables collisions
    link->Init();

    if (scaling.empty())
      continue;

    //Process all the collisions in all the links
    for (const auto &collision : link->GetCollisions())
    {
      auto name = collision->GetName();

      if (scaling.find(name) != scaling.end())
      {
        auto boxShape = boost::dynamic_pointer_cast<gazebo::physics::BoxShape>(
            collision->GetShape());

        // Make sure we have a box shape.
        if (boxShape)
          boxShape->SetSize(boxShape->Size() * scaling[name]);
      }

      if (offsets.find(name) != offsets.end())
      {
        collision->SetInitialRelativePose(
            offsets[name] + collision->InitialRelativePose());
      }
    }
  }
}
void GazeboActorRosPlanarMovePrivate::TriggerSos()
{
    if (over_60) {
      if (heart_rate_per_minute > 80 &&
          heart_rate_per_minute < 140) {
        heart_rate_sos_triggered = false;
      } else {
        heart_rate_sos_triggered = true;
      }
      if (heart_rate_per_minute > 70 &&
          heart_rate_per_minute <= 80) {
        priority = 2;
      } elif (heart_rate_per_minute  <= 70) {
        priority = 1;
      } elif (heart_rate_per_minute >= 140 &&
          heart_rate_per_minute < 160) {
        priority = 2;
      } elif (heart_rate_per_minute >= 160) {
        priority = 1;
      }
    }
    if(oxygenation_arr[10 - 1] != 0){
      auto sum_ox = 0;
      for(int i=0; i < 10; i++) {
        sum_ox += oxygenation_arr[i];
      }
      if(sum_ox/10 < 92) {
        oxygenation_sos_triggered = true;
        priority = 3;
        ResetArray<int>(oxygenation_arr, 10);
        oxygenation_index = 0;
      }
    }
    if(body_temperature_arr[10 - 1] != 0.0){
      auto sum_temp = 0.0;
      for (int i=0; i < 10; i++) {
        sum_temp += body_temperature_arr[i];
      }
      if(sum_temp/10 > 37.5) {
        body_temperature_sos_triggered = true;
        priority = 4;
        ResetArray<float>(body_temperature_arr, 10);
        body_temperature_index = 0;
      }
    }
    if (accidental_fall_sos_triggered ||
        body_temperature_sos_triggered ||
        follow_triggered ||
        heart_rate_sos_triggered ||
        oxygenation_sos_triggered) {
      random_movement_ = false;
      PublishTriggers();
    }
}

template <typename T>
void GazeboActorRosPlanarMovePrivate::ResetArray(T array[], int size)
{
  for (int i=0; i < size; i++) {
    array[i] = (T)0;
  }
}

template <typename T>
T GazeboActorRosPlanarMovePrivate::LimitValue(T value, T min, T max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

GZ_REGISTER_MODEL_PLUGIN(GazeboActorRosPlanarMove)
}  // namespace gazebo_plugins
